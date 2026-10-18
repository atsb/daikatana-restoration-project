#include "dk_neural_ai.h"
#include "dk_neural_training_format.h"
#include "qcommon.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <atomic>

#if defined(_WIN32)
#include <windows.h>
#endif

#define DK_NEURAL_MODEL_MAGIC ((uint32_t)0x354B4444U)
#define DK_NEURAL_MAX_MODEL_PATH (MAX_OSPATH * 2)
#define DK_NEURAL_HEALTH_CRITICAL 0.20f
#define DK_NEURAL_HEALTH_LOW 0.40f
#define DK_NEURAL_HEALTH_SAFE 0.65f
#define DK_NEURAL_OVERWHELMING_THREATS 3.0f
#define DK_NEURAL_RULE_SCORE_HARD 1000.0f

#define DK_NEURAL_RULE(condition, action, weight) \
    do { if (condition) scores[(action) - 1] += (weight); } while (0)

typedef struct
{
    uint32_t magic;
    uint32_t version;
    uint32_t input_count;
    uint32_t hidden1_count;
    uint32_t hidden2_count;
    uint32_t output_count;
    uint32_t parameter_count;
} dk_neural_model_file_header_t;

static_assert((size_t)DK_NEURAL_INPUT_COUNT * DK_NEURAL_HIDDEN_1 +
              DK_NEURAL_HIDDEN_1 +
              (size_t)DK_NEURAL_HIDDEN_1 * DK_NEURAL_HIDDEN_2 +
              DK_NEURAL_HIDDEN_2 +
              (size_t)DK_NEURAL_HIDDEN_2 * DK_NEURAL_ACTION_COUNT +
              DK_NEURAL_ACTION_COUNT == DK_NEURAL_PARAMETER_COUNT,
    "5M tactical model parameter count changed");
static_assert(sizeof(dk_neural_model_file_header_t) == 28,
    "5M tactical model file header size changed");

typedef struct
{
    float *parameters;
    float *input_to_hidden1;
    float *hidden1_bias;
    float *hidden1_to_hidden2;
    float *hidden2_bias;
    float *hidden2_to_output;
    float *output_bias;
    float *hidden1_activation;
    float *hidden2_activation;
    float *hidden1_delta;
    float *hidden2_delta;
    qboolean allocated;
} dk_neural_model_t;

typedef struct
{
    float selfHealth;
    float selfArmor;
    float ownerAlive;
    float enemyPresent;
    float ownerHealth;
    float ownerDistance;
    float ownerDanger;
    float enemyDistance;
    float enemyHealth;
    float enemyThreat;
    float threatCount;
    float selfThreatCount;
    float ownerThreatCount;
    float squadPresent;
    float squadHealth;
    float squadDistance;
    float squadThreat;
    float ammo;
    float weaponReady;
    float hasKnownWeapon;
    float forwardOpen;
    float backwardOpen;
    float leftOpen;
    float rightOpen;
    float coverQuality;
    float dropAhead;
    float dropBehind;
    float dropLeft;
    float dropRight;
    float lowHeadroom;
    float narrowSpace;
    float forwardHazard;
    float backwardHazard;
    float leftHazard;
    float rightHazard;
    float currentHazard;
    float enemyVisible;
    float enemyInPVS;
    float enemyInPHS;
    float ownerVisible;
    float ownerInPVS;
    float ownerInPHS;
    float squadVisible;
    float squadInPVS;
    float squadInPHS;
    float enemyEasyTarget;
    float enemyTargetingSelf;
    float enemyTargetingOwner;
    float enemyCritical;
    float ownerCritical;
    float ownerUnderAttack;
    float movementBlocked;
    float selfOnGround;
    float selfInWater;
    float lastAction;
    float currentTask;
    float currentGoal;
    float aiStay;
    float aiNoAttack;
    float ionBlaster;
    float shotcycler;
    float bolter;
    float contextSummary;
} dk_neural_facts_t;

static cvar_t *dk_neural_ai;
static cvar_t *dk_advanced_reasoning;
static cvar_t *dk_neural_weights;
static cvar_t *dk_neural_debug;
static cvar_t *dk_neural_record_samples;
static cvar_t *dk_neural_dataset;

static dk_neural_model_t dk_neural_models[2];
static std::atomic<int> dk_neural_active_model(0);
static std::atomic<int> dk_neural_trained_weights_loaded(0);

static char dk_neural_resolved_weights_path[DK_NEURAL_MAX_MODEL_PATH];
static qboolean dk_neural_weight_load_attempted;
static qboolean dk_neural_initialized;
static FILE *dk_neural_dataset_file;
static uint64_t dk_neural_dataset_sample_count;
static char dk_neural_resolved_dataset_path[DK_NEURAL_MAX_MODEL_PATH];

static const char *DKNeuralAI_ActionName(int action)
{
    switch (action)
    {
        case DK_NEURAL_ACTION_FOLLOW_OWNER:  return "FOLLOW_OWNER";
        case DK_NEURAL_ACTION_ATTACK_TARGET: return "ATTACK_TARGET";
        case DK_NEURAL_ACTION_RETREAT:      return "RETREAT";
        case DK_NEURAL_ACTION_TAKE_COVER:   return "TAKE_COVER";
        case DK_NEURAL_ACTION_FLANK_LEFT:   return "FLANK_LEFT";
        case DK_NEURAL_ACTION_FLANK_RIGHT:  return "FLANK_RIGHT";
        case DK_NEURAL_ACTION_HOLD:         return "HOLD";
    }
    return "INVALID";
}

static float DKNeuralAI_Clamp(float value, float minimum, float maximum)
{
    if (value < minimum)
        return minimum;
    if (value > maximum)
        return maximum;
    return value;
}

static float DKNeuralAI_Relu(float value)
{
    return value > 0.0f ? value : 0.0f;
}

static float DKNeuralAI_ReluDerivative(float value)
{
    return value > 0.0f ? 1.0f : 0.0f;
}

static size_t DKNeuralAI_WeightCount(void)
{
    return (size_t)DK_NEURAL_PARAMETER_COUNT;
}

static float *DKNeuralAI_AllocWeights(void)
{
    return (float *)calloc(DKNeuralAI_WeightCount(), sizeof(float));
}

static uint32_t DKNeuralAI_RandomStep(uint32_t *seed)
{
    *seed = *seed * 1664525U + 1013904223U;
    return *seed;
}

static float DKNeuralAI_RandomSigned(uint32_t *seed, float scale)
{
    uint32_t value = DKNeuralAI_RandomStep(seed);
    float normalized = (float)(value & 0x00FFFFFFU) / 16777215.0f;
    return (normalized * 2.0f - 1.0f) * scale;
}

static void DKNeuralAI_AssignWeightViews(dk_neural_model_t *model)
{
    size_t offset = 0;

    model->input_to_hidden1 = model->parameters + offset;
    offset += (size_t)DK_NEURAL_INPUT_COUNT * DK_NEURAL_HIDDEN_1;

    model->hidden1_bias = model->parameters + offset;
    offset += DK_NEURAL_HIDDEN_1;

    model->hidden1_to_hidden2 = model->parameters + offset;
    offset += (size_t)DK_NEURAL_HIDDEN_1 * DK_NEURAL_HIDDEN_2;

    model->hidden2_bias = model->parameters + offset;
    offset += DK_NEURAL_HIDDEN_2;

    model->hidden2_to_output = model->parameters + offset;
    offset += (size_t)DK_NEURAL_HIDDEN_2 * DK_NEURAL_ACTION_COUNT;

    model->output_bias = model->parameters + offset;
}

static void DKNeuralAI_FreeModelBuffer(dk_neural_model_t *model)
{
    if (!model)
        return;

    free(model->parameters);
    free(model->hidden1_activation);
    free(model->hidden2_activation);
    free(model->hidden1_delta);
    free(model->hidden2_delta);
    memset(model, 0, sizeof(*model));
}

static void DKNeuralAI_FreeModels(void)
{
    DKNeuralAI_FreeModelBuffer(&dk_neural_models[0]);
    DKNeuralAI_FreeModelBuffer(&dk_neural_models[1]);
    dk_neural_active_model.store(0, std::memory_order_release);
    dk_neural_trained_weights_loaded.store(0, std::memory_order_release);
    dk_neural_weight_load_attempted = FALSE;
}

static void DKNeuralAI_InitializeModelBuffer(dk_neural_model_t *model)
{
    size_t i;
    uint32_t seed = 0x4B445441U;
    float inputScale;
    float hiddenScale;

    if (!model || model->allocated)
        return;

    memset(model, 0, sizeof(*model));
    model->parameters = DKNeuralAI_AllocWeights();
    model->hidden1_activation = (float *)calloc(DK_NEURAL_HIDDEN_1, sizeof(float));
    model->hidden2_activation = (float *)calloc(DK_NEURAL_HIDDEN_2, sizeof(float));
    model->hidden1_delta = (float *)calloc(DK_NEURAL_HIDDEN_1, sizeof(float));
    model->hidden2_delta = (float *)calloc(DK_NEURAL_HIDDEN_2, sizeof(float));

    if (!model->parameters || !model->hidden1_activation || !model->hidden2_activation ||
        !model->hidden1_delta || !model->hidden2_delta)
    {
        DKNeuralAI_FreeModelBuffer(model);
        return;
    }

    DKNeuralAI_AssignWeightViews(model);

    inputScale = 0.08f;
    hiddenScale = 0.04f;

    for (i = 0; i < (size_t)DK_NEURAL_INPUT_COUNT * DK_NEURAL_HIDDEN_1; ++i)
        model->input_to_hidden1[i] = DKNeuralAI_RandomSigned(&seed, inputScale);

    for (i = 0; i < (size_t)DK_NEURAL_HIDDEN_1 * DK_NEURAL_HIDDEN_2; ++i)
        model->hidden1_to_hidden2[i] = DKNeuralAI_RandomSigned(&seed, hiddenScale);

    for (i = 0; i < (size_t)DK_NEURAL_HIDDEN_2 * DK_NEURAL_ACTION_COUNT; ++i)
        model->hidden2_to_output[i] = DKNeuralAI_RandomSigned(&seed, hiddenScale);

    model->allocated = TRUE;
}

static void DKNeuralAI_InitializeModels(void)
{
    DKNeuralAI_InitializeModelBuffer(&dk_neural_models[0]);
    DKNeuralAI_InitializeModelBuffer(&dk_neural_models[1]);

    if (!dk_neural_models[0].allocated || !dk_neural_models[1].allocated)
    {
        DKNeuralAI_FreeModels();
        Com_Printf("DaikatanaAI: unable to allocate model buffers.\n");
        return;
    }

    Com_Printf("DaikatanaAI: model initialized.\n");
}

static qboolean DKNeuralAI_IsAbsolutePath(const char *path)
{
    if (!path || !path[0])
        return FALSE;

    if (((path[0] >= 'A') && (path[0] <= 'Z')) ||
        ((path[0] >= 'a') && (path[0] <= 'z')))
    {
        return path[1] == ':' && (path[2] == '/' || path[2] == '\\');
    }

    return path[0] == '/' || path[0] == '\\';
}

static void DKNeuralAI_ResolveWeightsPath(void)
{
    const char *configured = dk_neural_weights ? dk_neural_weights->string : "neural/models/DaikatanaAI.dkai";

    if (DKNeuralAI_IsAbsolutePath(configured))
        Com_sprintf(dk_neural_resolved_weights_path, sizeof(dk_neural_resolved_weights_path), "%s", configured);
    else
        Com_sprintf(dk_neural_resolved_weights_path, sizeof(dk_neural_resolved_weights_path), "%s/%s", FS_Gamedir(), configured);
}

static qboolean DKNeuralAI_LoadWeightsFromPath(const char *path, int modelIndex)
{
    FILE *file;
    dk_neural_model_file_header_t header;
    dk_neural_model_t *model;

    if (!path || !path[0] || modelIndex < 0 || modelIndex > 1)
        return FALSE;

    model = &dk_neural_models[modelIndex];
    if (!model->allocated)
        return FALSE;

    file = fopen(path, "rb");
    if (!file)
        return FALSE;

    if (fread(&header, sizeof(header), 1, file) != 1)
    {
        fclose(file);
        return FALSE;
    }

    if (header.magic != DK_NEURAL_MODEL_MAGIC ||
        header.version != DK_NEURAL_MODEL_VERSION ||
        header.input_count != DK_NEURAL_INPUT_COUNT ||
        header.hidden1_count != DK_NEURAL_HIDDEN_1 ||
        header.hidden2_count != DK_NEURAL_HIDDEN_2 ||
        header.output_count != DK_NEURAL_ACTION_COUNT ||
        header.parameter_count != DK_NEURAL_PARAMETER_COUNT)
    {
        fclose(file);
        Com_Printf("DaikatanaAI: invalid model header: %s\n", path);
        return FALSE;
    }

    if (fread(model->parameters, sizeof(float), DKNeuralAI_WeightCount(), file) != DKNeuralAI_WeightCount())
    {
        fclose(file);
        Com_Printf("DaikatanaAI: incomplete model: %s\n", path);
        return FALSE;
    }

    fclose(file);
    return TRUE;
}

static qboolean DKNeuralAI_SaveWeightsToPath(const char *path)
{
    FILE *file;
    char temporaryPath[DK_NEURAL_MAX_MODEL_PATH + 8];
    dk_neural_model_file_header_t header;
    int activeIndex;
    dk_neural_model_t *model;

    if (!path || !path[0])
        return FALSE;

    activeIndex = dk_neural_active_model.load(std::memory_order_acquire);
    model = &dk_neural_models[activeIndex];
    if (!model->allocated)
        return FALSE;

    Com_sprintf(temporaryPath, sizeof(temporaryPath), "%s.tmp", path);

    file = fopen(temporaryPath, "wb");
    if (!file)
        return FALSE;

    header.magic = DK_NEURAL_MODEL_MAGIC;
    header.version = DK_NEURAL_MODEL_VERSION;
    header.input_count = DK_NEURAL_INPUT_COUNT;
    header.hidden1_count = DK_NEURAL_HIDDEN_1;
    header.hidden2_count = DK_NEURAL_HIDDEN_2;
    header.output_count = DK_NEURAL_ACTION_COUNT;
    header.parameter_count = DK_NEURAL_PARAMETER_COUNT;

    if (fwrite(&header, sizeof(header), 1, file) != 1 ||
        fwrite(model->parameters, sizeof(float), DKNeuralAI_WeightCount(), file) != DKNeuralAI_WeightCount())
    {
        fclose(file);
        remove(temporaryPath);
        return FALSE;
    }

    fclose(file);
    remove(path);
    if (rename(temporaryPath, path) != 0)
    {
        remove(temporaryPath);
        return FALSE;
    }

    return TRUE;
}

static void DKNeuralAI_NormalizeInput(const dk_neural_tactical_state_t *state, float *output)
{
    int i;
    const float *input = (const float *)state;

    for (i = 0; i < DK_NEURAL_INPUT_COUNT; ++i)
        output[i] = DKNeuralAI_Clamp(input[i], -1.0f, 1.0f);
}

static void DKNeuralAI_BuildFacts(const dk_neural_tactical_state_t *state, dk_neural_facts_t *facts)
{
    int i;

    memset(facts, 0, sizeof(*facts));

    facts->ownerAlive = state->owner.alive;
    facts->enemyPresent = state->enemy.present;
    facts->selfHealth = DKNeuralAI_Clamp(state->self.health_percent, 0.0f, 1.0f);
    facts->selfArmor = DKNeuralAI_Clamp(state->self.armor_percent, 0.0f, 1.0f);
    facts->ownerHealth = DKNeuralAI_Clamp(state->owner.health_percent, 0.0f, 1.0f);
    facts->ownerDistance = DKNeuralAI_Clamp(state->owner.distance, 0.0f, 1.0f);
    facts->ownerDanger = DKNeuralAI_Clamp(
        state->owner.danger_score + state->owner.target_self * 0.25f, 0.0f, 1.0f);
    facts->enemyDistance = DKNeuralAI_Clamp(state->enemy.distance, 0.0f, 1.0f);
    facts->enemyHealth = DKNeuralAI_Clamp(state->enemy.health_percent, 0.0f, 1.0f);
    facts->enemyThreat = DKNeuralAI_Clamp(state->enemy.threat_score, 0.0f, 1.0f);
    facts->squadPresent = state->squad.present;
    facts->squadHealth = DKNeuralAI_Clamp(state->squad.health_percent, 0.0f, 1.0f);
    facts->squadDistance = DKNeuralAI_Clamp(state->squad.distance, 0.0f, 1.0f);
    facts->squadThreat = DKNeuralAI_Clamp(state->squad.threat_pressure, 0.0f, 1.0f);
    facts->ammo = DKNeuralAI_Clamp(state->self.ammo_fraction, 0.0f, 1.0f);
    facts->weaponReady = DKNeuralAI_Clamp(state->self.weapon_ready, 0.0f, 1.0f);
    facts->hasKnownWeapon = DKNeuralAI_Clamp(
        state->self.weapon_ion + state->self.weapon_shotcycler + state->self.weapon_bolter, 0.0f, 1.0f);
    facts->forwardOpen = DKNeuralAI_Clamp(state->geometry.horizontal[0][0], 0.0f, 1.0f);
    facts->rightOpen = DKNeuralAI_Clamp(state->geometry.horizontal[2][0], 0.0f, 1.0f);
    facts->backwardOpen = DKNeuralAI_Clamp(state->geometry.horizontal[4][0], 0.0f, 1.0f);
    facts->leftOpen = DKNeuralAI_Clamp(state->geometry.horizontal[6][0], 0.0f, 1.0f);
    facts->dropAhead = state->geometry.floor_distance[0] > 0.75f ? 1.0f : 0.0f;
    facts->dropBehind = state->geometry.floor_distance[4] > 0.75f ? 1.0f : 0.0f;
    facts->dropRight = state->geometry.floor_distance[2] > 0.75f ? 1.0f : 0.0f;
    facts->dropLeft = state->geometry.floor_distance[6] > 0.75f ? 1.0f : 0.0f;
    facts->lowHeadroom = state->geometry.ceiling_distance[0] < 0.12f ? 1.0f : 0.0f;
    facts->narrowSpace = (facts->leftOpen < 0.35f && facts->rightOpen < 0.35f) ? 1.0f : 0.0f;
    facts->currentHazard = DKNeuralAI_Clamp(
        state->bsp.self_contents[2] * 0.85f + state->bsp.self_contents[3] * 0.85f,
        0.0f, 1.0f);
    facts->forwardHazard = DKNeuralAI_Clamp(
        state->bsp.sample_contents[2] * 0.75f + state->bsp.sample_contents[3] * 0.75f + facts->dropAhead * 0.60f,
        0.0f, 1.0f);
    facts->backwardHazard = DKNeuralAI_Clamp(
        state->bsp.self_contents[2] * 0.35f + state->bsp.self_contents[3] * 0.35f + facts->dropBehind * 0.60f,
        0.0f, 1.0f);
    facts->leftHazard = DKNeuralAI_Clamp(facts->dropLeft * 0.75f, 0.0f, 1.0f);
    facts->rightHazard = DKNeuralAI_Clamp(facts->dropRight * 0.75f, 0.0f, 1.0f);
    facts->coverQuality = DKNeuralAI_Clamp(
        (1.0f - facts->forwardOpen) * 0.30f +
        (1.0f - facts->rightOpen) * 0.20f +
        (1.0f - facts->leftOpen) * 0.20f +
        (1.0f - facts->backwardOpen) * 0.10f +
        (state->enemy.visible < 0.5f ? 0.20f : 0.0f),
        0.0f, 1.0f);

    for (i = 0; i < 8; ++i)
    {
        facts->threatCount += state->threats[i].present;
        facts->selfThreatCount += state->threats[i].targeting_self;
        facts->ownerThreatCount += state->threats[i].targeting_owner;
    }

    if (facts->enemyPresent > 0.5f && facts->threatCount < 1.0f)
        facts->threatCount = 1.0f;
    if (facts->enemyTargetingSelf > 0.5f && facts->selfThreatCount < 1.0f)
        facts->selfThreatCount = 1.0f;
    if (facts->enemyTargetingOwner > 0.5f && facts->ownerThreatCount < 1.0f)
        facts->ownerThreatCount = 1.0f;

    facts->enemyVisible = state->enemy.visible;
    facts->enemyInPVS = state->enemy.in_pvs;
    facts->enemyInPHS = state->enemy.in_phs;
    facts->ownerVisible = state->owner.visible;
    facts->ownerInPVS = state->owner.in_pvs;
    facts->ownerInPHS = state->owner.in_phs;
    facts->squadVisible = state->squad.visible;
    facts->squadInPVS = state->squad.in_pvs;
    facts->squadInPHS = state->squad.in_phs;
    facts->enemyEasyTarget = state->enemy.easy_target;
    facts->enemyTargetingSelf = state->enemy.targeting_self;
    facts->enemyTargetingOwner = state->enemy.targeting_owner;
    facts->enemyCritical = state->enemy.critical;
    facts->ownerCritical = state->owner.critical;
    facts->ownerUnderAttack = state->owner.under_attack;
    facts->movementBlocked = state->self.movement_blocked;
    facts->selfOnGround = state->self.on_ground;
    facts->selfInWater = state->self.water_level;
    facts->lastAction = DKNeuralAI_Clamp(state->self.last_action, 0.0f, 1.0f);
    facts->currentTask = DKNeuralAI_Clamp(state->self.current_task, 0.0f, 1.0f);
    facts->currentGoal = DKNeuralAI_Clamp(state->self.current_goal, 0.0f, 1.0f);
    facts->aiStay = state->self.ai_stay;
    facts->aiNoAttack = state->self.ai_noattack;
    facts->ionBlaster = state->self.weapon_ion;
    facts->shotcycler = state->self.weapon_shotcycler;
    facts->bolter = state->self.weapon_bolter;
    facts->contextSummary = DKNeuralAI_Clamp(state->enemy.context_summary, 0.0f, 1.0f);
}

static int DKNeuralAI_AbsoluteRuleAction(const dk_neural_facts_t *facts)
{
    unsigned int contextFlags;

    if (facts->aiStay > 0.5f)
        return DK_NEURAL_ACTION_HOLD;

    /* Context bit 1 means the owner is beyond the normal follow radius.
       Let the deterministic world-model route solve the hard traversal;
       this fallback keeps neural-only recovery pointed at the owner. */
    contextFlags = (unsigned int)(facts->contextSummary * 1023.0f + 0.5f);
    if ((contextFlags & (1U << 1)) && facts->enemyPresent < 0.5f &&
        facts->ownerAlive > 0.5f)
        return DK_NEURAL_ACTION_FOLLOW_OWNER;

    if (facts->currentHazard > 0.75f)
    {
        if (facts->backwardOpen > 0.20f && facts->backwardHazard < 0.50f)
            return DK_NEURAL_ACTION_RETREAT;
        if (facts->leftOpen > 0.20f && facts->leftHazard < 0.50f)
            return DK_NEURAL_ACTION_FLANK_LEFT;
        if (facts->rightOpen > 0.20f && facts->rightHazard < 0.50f)
            return DK_NEURAL_ACTION_FLANK_RIGHT;
    }

    if (facts->aiNoAttack > 0.5f && facts->enemyTargetingSelf > 0.5f)
    {
        if (facts->backwardOpen > 0.20f)
            return DK_NEURAL_ACTION_RETREAT;
        if (facts->coverQuality > 0.50f)
            return DK_NEURAL_ACTION_TAKE_COVER;
    }

    if (facts->ownerCritical > 0.5f && facts->ownerUnderAttack > 0.5f &&
        facts->enemyTargetingOwner > 0.5f && facts->selfHealth > 0.35f &&
        facts->weaponReady > 0.5f && facts->ammo > 0.05f && facts->aiNoAttack < 0.5f)
    {
        return DK_NEURAL_ACTION_ATTACK_TARGET;
    }

    if (facts->selfHealth < 0.10f)
    {
        if (facts->coverQuality > 0.60f && facts->enemyThreat > 0.40f)
            return DK_NEURAL_ACTION_TAKE_COVER;
        if (facts->backwardOpen > 0.20f && facts->backwardHazard < 0.50f)
            return DK_NEURAL_ACTION_RETREAT;
    }

    if (facts->selfHealth < DK_NEURAL_HEALTH_CRITICAL && facts->threatCount >= 3.0f)
    {
        if (facts->backwardOpen > 0.20f && facts->backwardHazard < 0.50f)
            return DK_NEURAL_ACTION_RETREAT;
        if (facts->coverQuality > 0.50f)
            return DK_NEURAL_ACTION_TAKE_COVER;
    }

    if (facts->dropAhead > 0.5f && facts->movementBlocked > 0.5f)
    {
        if (facts->leftOpen > 0.40f && facts->leftHazard < 0.50f)
            return DK_NEURAL_ACTION_FLANK_LEFT;
        if (facts->rightOpen > 0.40f && facts->rightHazard < 0.50f)
            return DK_NEURAL_ACTION_FLANK_RIGHT;
        if (facts->backwardOpen > 0.20f && facts->backwardHazard < 0.50f)
            return DK_NEURAL_ACTION_RETREAT;
    }

    if (facts->forwardOpen < 0.20f && facts->leftOpen > 0.55f && facts->rightOpen < 0.20f &&
        facts->leftHazard < 0.50f && facts->threatCount > 0.0f)
        return DK_NEURAL_ACTION_FLANK_LEFT;

    if (facts->forwardOpen < 0.20f && facts->rightOpen > 0.55f && facts->leftOpen < 0.20f &&
        facts->rightHazard < 0.50f && facts->threatCount > 0.0f)
        return DK_NEURAL_ACTION_FLANK_RIGHT;

    if (facts->enemyEasyTarget > 0.75f && facts->selfHealth > DK_NEURAL_HEALTH_SAFE &&
        facts->weaponReady > 0.5f && facts->ammo > 0.10f && facts->aiNoAttack < 0.5f)
        return DK_NEURAL_ACTION_ATTACK_TARGET;

    if (facts->enemyPresent < 0.5f && facts->ownerAlive > 0.5f && facts->ownerDistance > 0.30f)
        return DK_NEURAL_ACTION_FOLLOW_OWNER;

    return DK_NEURAL_ACTION_INVALID;
}

static void DKNeuralAI_AddRuleScores(const dk_neural_tactical_state_t *state,
    const dk_neural_facts_t *facts, float *scores)
{
    memset(scores, 0, sizeof(float) * DK_NEURAL_ACTION_COUNT);

    DK_NEURAL_RULE(facts->selfHealth < 0.20f, DK_NEURAL_ACTION_RETREAT, 7.0f);
    DK_NEURAL_RULE(facts->selfHealth < 0.30f, DK_NEURAL_ACTION_RETREAT, 4.0f);
    DK_NEURAL_RULE(facts->selfHealth < 0.40f, DK_NEURAL_ACTION_TAKE_COVER, 3.0f);
    DK_NEURAL_RULE(facts->selfArmor < 0.20f && facts->selfThreatCount > 0.0f, DK_NEURAL_ACTION_TAKE_COVER, 2.5f);
    DK_NEURAL_RULE(facts->selfHealth > 0.70f && facts->selfArmor > 0.40f, DK_NEURAL_ACTION_ATTACK_TARGET, 2.0f);
    DK_NEURAL_RULE(facts->selfHealth > 0.80f, DK_NEURAL_ACTION_ATTACK_TARGET, 1.5f);
    DK_NEURAL_RULE(facts->ammo < 0.05f, DK_NEURAL_ACTION_RETREAT, 3.0f);
    DK_NEURAL_RULE(facts->ammo < 0.15f, DK_NEURAL_ACTION_FOLLOW_OWNER, 1.5f);
    DK_NEURAL_RULE(facts->ammo > 0.40f, DK_NEURAL_ACTION_ATTACK_TARGET, 1.0f);
    DK_NEURAL_RULE(facts->weaponReady > 0.5f, DK_NEURAL_ACTION_ATTACK_TARGET, 1.0f);
    DK_NEURAL_RULE(facts->weaponReady < 0.5f && facts->enemyThreat > 0.50f, DK_NEURAL_ACTION_TAKE_COVER, 1.5f);
    DK_NEURAL_RULE(facts->selfInWater > 0.75f && facts->enemyThreat > 0.40f, DK_NEURAL_ACTION_RETREAT, 1.5f);
    DK_NEURAL_RULE(facts->currentHazard > 0.25f, DK_NEURAL_ACTION_RETREAT, 3.0f);
    DK_NEURAL_RULE(facts->selfOnGround < 0.5f, DK_NEURAL_ACTION_HOLD, 0.5f);

    DK_NEURAL_RULE(facts->ownerDanger > 0.75f, DK_NEURAL_ACTION_ATTACK_TARGET, 5.0f);
    DK_NEURAL_RULE(facts->ownerDanger > 0.50f, DK_NEURAL_ACTION_ATTACK_TARGET, 3.0f);
    DK_NEURAL_RULE(facts->ownerCritical > 0.5f && facts->ownerUnderAttack > 0.5f, DK_NEURAL_ACTION_ATTACK_TARGET, 4.5f);
    DK_NEURAL_RULE(facts->ownerUnderAttack > 0.5f, DK_NEURAL_ACTION_FOLLOW_OWNER, 2.0f);
    DK_NEURAL_RULE(facts->ownerDistance > 0.65f && facts->ownerUnderAttack < 0.5f, DK_NEURAL_ACTION_FOLLOW_OWNER, 4.0f);
    DK_NEURAL_RULE(facts->ownerDistance > 0.35f && facts->ownerUnderAttack < 0.5f, DK_NEURAL_ACTION_FOLLOW_OWNER, 1.8f);
    DK_NEURAL_RULE(facts->ownerDistance < 0.18f && facts->ownerDanger < 0.30f, DK_NEURAL_ACTION_HOLD, 1.5f);
    DK_NEURAL_RULE(facts->ownerHealth < 0.30f && facts->ownerUnderAttack < 0.5f, DK_NEURAL_ACTION_FOLLOW_OWNER, 1.2f);
    DK_NEURAL_RULE(facts->ownerVisible < 0.5f && facts->ownerInPHS > 0.5f, DK_NEURAL_ACTION_FOLLOW_OWNER, 1.0f);
    DK_NEURAL_RULE(facts->ownerVisible > 0.5f && facts->ownerDistance < 0.20f, DK_NEURAL_ACTION_HOLD, 1.0f);

    DK_NEURAL_RULE(facts->enemyThreat > 0.70f, DK_NEURAL_ACTION_TAKE_COVER, 2.8f);
    DK_NEURAL_RULE(facts->enemyThreat > 0.50f, DK_NEURAL_ACTION_ATTACK_TARGET, 1.5f);
    DK_NEURAL_RULE(facts->enemyTargetingSelf > 0.5f, DK_NEURAL_ACTION_ATTACK_TARGET, 2.0f);
    DK_NEURAL_RULE(facts->enemyTargetingOwner > 0.5f, DK_NEURAL_ACTION_ATTACK_TARGET, 3.0f);
    DK_NEURAL_RULE(facts->enemyVisible > 0.5f, DK_NEURAL_ACTION_ATTACK_TARGET, 2.0f);
    DK_NEURAL_RULE(facts->enemyEasyTarget > 0.50f, DK_NEURAL_ACTION_ATTACK_TARGET, 2.0f);
    DK_NEURAL_RULE(facts->enemyEasyTarget > 0.80f, DK_NEURAL_ACTION_ATTACK_TARGET, 1.5f);
    DK_NEURAL_RULE(facts->enemyHealth < 0.30f, DK_NEURAL_ACTION_ATTACK_TARGET, 1.5f);
    DK_NEURAL_RULE(facts->enemyDistance < 0.15f, DK_NEURAL_ACTION_ATTACK_TARGET, 1.0f);
    DK_NEURAL_RULE(facts->enemyDistance < 0.30f && facts->weaponReady > 0.5f, DK_NEURAL_ACTION_ATTACK_TARGET, 1.5f);
    DK_NEURAL_RULE(facts->enemyDistance > 0.80f, DK_NEURAL_ACTION_HOLD, 0.8f);
    DK_NEURAL_RULE(facts->enemyVisible < 0.5f, DK_NEURAL_ACTION_ATTACK_TARGET, -1.8f);
    DK_NEURAL_RULE(facts->enemyInPVS < 0.5f, DK_NEURAL_ACTION_ATTACK_TARGET, -0.8f);
    DK_NEURAL_RULE(facts->enemyInPHS < 0.5f && facts->enemyVisible < 0.5f, DK_NEURAL_ACTION_ATTACK_TARGET, -0.8f);
    DK_NEURAL_RULE(facts->threatCount >= 3.0f, DK_NEURAL_ACTION_ATTACK_TARGET, -2.0f);
    DK_NEURAL_RULE(facts->threatCount >= 4.0f, DK_NEURAL_ACTION_RETREAT, 3.5f);
    DK_NEURAL_RULE(facts->selfThreatCount >= 2.0f, DK_NEURAL_ACTION_TAKE_COVER, 2.5f);
    DK_NEURAL_RULE(facts->ownerThreatCount >= 2.0f, DK_NEURAL_ACTION_ATTACK_TARGET, 3.5f);

    DK_NEURAL_RULE(facts->coverQuality > 0.75f && facts->enemyVisible > 0.5f, DK_NEURAL_ACTION_TAKE_COVER, 3.0f);
    DK_NEURAL_RULE(facts->coverQuality > 0.60f && facts->enemyThreat > 0.40f, DK_NEURAL_ACTION_TAKE_COVER, 2.0f);
    DK_NEURAL_RULE(facts->coverQuality > 0.85f && facts->selfHealth < 0.50f, DK_NEURAL_ACTION_TAKE_COVER, 2.5f);
    DK_NEURAL_RULE(facts->coverQuality < 0.20f, DK_NEURAL_ACTION_TAKE_COVER, -1.5f);
    DK_NEURAL_RULE(facts->enemyVisible > 0.5f && facts->coverQuality < 0.30f, DK_NEURAL_ACTION_ATTACK_TARGET, 1.0f);

    DK_NEURAL_RULE(facts->leftOpen > 0.65f && facts->leftHazard < 0.50f, DK_NEURAL_ACTION_FLANK_LEFT, 2.0f);
    DK_NEURAL_RULE(facts->rightOpen > 0.65f && facts->rightHazard < 0.50f, DK_NEURAL_ACTION_FLANK_RIGHT, 2.0f);
    DK_NEURAL_RULE(facts->enemyVisible < 0.5f && facts->leftOpen > 0.55f && facts->enemyThreat > 0.30f, DK_NEURAL_ACTION_FLANK_LEFT, 1.5f);
    DK_NEURAL_RULE(facts->enemyVisible < 0.5f && facts->rightOpen > 0.55f && facts->enemyThreat > 0.30f, DK_NEURAL_ACTION_FLANK_RIGHT, 1.5f);
    DK_NEURAL_RULE(facts->selfHealth > 0.60f && facts->threatCount <= 2.0f && facts->leftOpen > 0.45f, DK_NEURAL_ACTION_FLANK_LEFT, 1.8f);
    DK_NEURAL_RULE(facts->selfHealth > 0.60f && facts->threatCount <= 2.0f && facts->rightOpen > 0.45f, DK_NEURAL_ACTION_FLANK_RIGHT, 1.8f);
    DK_NEURAL_RULE(facts->selfHealth < 0.35f, DK_NEURAL_ACTION_FLANK_LEFT, -3.0f);
    DK_NEURAL_RULE(facts->selfHealth < 0.35f, DK_NEURAL_ACTION_FLANK_RIGHT, -3.0f);
    DK_NEURAL_RULE(facts->threatCount >= 3.0f, DK_NEURAL_ACTION_FLANK_LEFT, -2.0f);
    DK_NEURAL_RULE(facts->threatCount >= 3.0f, DK_NEURAL_ACTION_FLANK_RIGHT, -2.0f);
    DK_NEURAL_RULE(facts->dropLeft > 0.5f, DK_NEURAL_ACTION_FLANK_LEFT, -8.0f);
    DK_NEURAL_RULE(facts->dropRight > 0.5f, DK_NEURAL_ACTION_FLANK_RIGHT, -8.0f);
    DK_NEURAL_RULE(facts->leftHazard > 0.5f, DK_NEURAL_ACTION_FLANK_LEFT, -8.0f);
    DK_NEURAL_RULE(facts->rightHazard > 0.5f, DK_NEURAL_ACTION_FLANK_RIGHT, -8.0f);
    DK_NEURAL_RULE(facts->forwardOpen < 0.20f && facts->leftOpen > 0.50f && facts->rightOpen < 0.25f,
        DK_NEURAL_ACTION_FLANK_LEFT, 4.0f);
    DK_NEURAL_RULE(facts->forwardOpen < 0.20f && facts->rightOpen > 0.50f && facts->leftOpen < 0.25f,
        DK_NEURAL_ACTION_FLANK_RIGHT, 4.0f);
    DK_NEURAL_RULE(facts->leftOpen < 0.20f, DK_NEURAL_ACTION_FLANK_LEFT, -7.0f);
    DK_NEURAL_RULE(facts->rightOpen < 0.20f, DK_NEURAL_ACTION_FLANK_RIGHT, -7.0f);

    DK_NEURAL_RULE(facts->forwardOpen < 0.20f && facts->backwardOpen > 0.40f,
        DK_NEURAL_ACTION_RETREAT, 2.0f);
    DK_NEURAL_RULE(facts->forwardOpen < 0.20f && facts->backwardOpen < 0.20f && facts->leftOpen < 0.20f && facts->rightOpen < 0.20f,
        DK_NEURAL_ACTION_HOLD, 3.0f);
    DK_NEURAL_RULE(facts->movementBlocked > 0.5f && facts->backwardOpen > 0.50f,
        DK_NEURAL_ACTION_RETREAT, 2.5f);
    DK_NEURAL_RULE(facts->movementBlocked > 0.5f && facts->leftOpen > 0.50f,
        DK_NEURAL_ACTION_FLANK_LEFT, 2.0f);
    DK_NEURAL_RULE(facts->movementBlocked > 0.5f && facts->rightOpen > 0.50f,
        DK_NEURAL_ACTION_FLANK_RIGHT, 2.0f);
    DK_NEURAL_RULE(facts->dropAhead > 0.5f, DK_NEURAL_ACTION_ATTACK_TARGET, -2.5f);
    DK_NEURAL_RULE(facts->dropAhead > 0.5f, DK_NEURAL_ACTION_FOLLOW_OWNER, -1.5f);
    DK_NEURAL_RULE(facts->dropAhead > 0.5f && facts->backwardOpen > 0.30f,
        DK_NEURAL_ACTION_RETREAT, 4.0f);
    DK_NEURAL_RULE(facts->dropBehind > 0.5f, DK_NEURAL_ACTION_RETREAT, -4.0f);
    DK_NEURAL_RULE(facts->lowHeadroom > 0.5f && facts->enemyDistance > 0.30f,
        DK_NEURAL_ACTION_TAKE_COVER, 1.0f);
    DK_NEURAL_RULE(facts->narrowSpace > 0.5f && facts->threatCount >= 3.0f,
        DK_NEURAL_ACTION_HOLD, 1.5f);
    DK_NEURAL_RULE(facts->narrowSpace > 0.5f && facts->enemyVisible > 0.5f,
        DK_NEURAL_ACTION_ATTACK_TARGET, 1.0f);

    DK_NEURAL_RULE(facts->forwardHazard > 0.50f, DK_NEURAL_ACTION_RETREAT, 3.0f);
    DK_NEURAL_RULE(facts->forwardHazard > 0.50f, DK_NEURAL_ACTION_ATTACK_TARGET, -3.0f);
    DK_NEURAL_RULE(facts->forwardHazard > 0.50f && facts->leftHazard < 0.30f && facts->leftOpen > 0.30f,
        DK_NEURAL_ACTION_FLANK_LEFT, 3.0f);
    DK_NEURAL_RULE(facts->forwardHazard > 0.50f && facts->rightHazard < 0.30f && facts->rightOpen > 0.30f,
        DK_NEURAL_ACTION_FLANK_RIGHT, 3.0f);
    DK_NEURAL_RULE(facts->backwardOpen < 0.20f, DK_NEURAL_ACTION_RETREAT, -7.0f);
    DK_NEURAL_RULE(facts->backwardHazard > 0.50f, DK_NEURAL_ACTION_RETREAT, -8.0f);

    DK_NEURAL_RULE(facts->ionBlaster > 0.5f && facts->enemyDistance > 0.25f && facts->enemyDistance < 0.75f,
        DK_NEURAL_ACTION_ATTACK_TARGET, 1.4f);
    DK_NEURAL_RULE(facts->shotcycler > 0.5f && facts->enemyDistance < 0.45f,
        DK_NEURAL_ACTION_ATTACK_TARGET, 1.8f);
    DK_NEURAL_RULE(facts->bolter > 0.5f && facts->enemyDistance > 0.15f,
        DK_NEURAL_ACTION_ATTACK_TARGET, 1.4f);
    DK_NEURAL_RULE(facts->hasKnownWeapon < 0.5f, DK_NEURAL_ACTION_ATTACK_TARGET, -2.0f);
    DK_NEURAL_RULE(facts->ammo < 0.10f && facts->enemyVisible > 0.5f, DK_NEURAL_ACTION_ATTACK_TARGET, -2.0f);
    DK_NEURAL_RULE(facts->ammo > 0.50f && facts->weaponReady > 0.5f, DK_NEURAL_ACTION_ATTACK_TARGET, 1.5f);

    DK_NEURAL_RULE(facts->squadHealth < 0.30f && facts->squadDistance < 0.45f,
        DK_NEURAL_ACTION_FOLLOW_OWNER, 1.5f);
    DK_NEURAL_RULE(facts->squadThreat > 0.70f, DK_NEURAL_ACTION_ATTACK_TARGET, 2.0f);
    DK_NEURAL_RULE(facts->squadDistance > 0.60f && facts->ownerDanger < 0.50f,
        DK_NEURAL_ACTION_FOLLOW_OWNER, 2.0f);
    DK_NEURAL_RULE(facts->squadDistance < 0.20f && facts->ownerDistance < 0.20f,
        DK_NEURAL_ACTION_HOLD, 1.0f);
    DK_NEURAL_RULE(facts->squadVisible < 0.5f && facts->squadInPHS > 0.5f,
        DK_NEURAL_ACTION_FOLLOW_OWNER, 0.8f);

    DK_NEURAL_RULE(facts->aiNoAttack > 0.5f, DK_NEURAL_ACTION_ATTACK_TARGET, -DK_NEURAL_RULE_SCORE_HARD);
    DK_NEURAL_RULE(facts->aiStay > 0.5f, DK_NEURAL_ACTION_HOLD, DK_NEURAL_RULE_SCORE_HARD);
    DK_NEURAL_RULE(facts->lastAction >= 0.40f && facts->lastAction < 0.60f && facts->leftOpen > 0.40f,
        DK_NEURAL_ACTION_FLANK_LEFT, 0.5f);
    DK_NEURAL_RULE(facts->lastAction >= 0.25f && facts->lastAction < 0.40f && facts->backwardOpen > 0.30f,
        DK_NEURAL_ACTION_RETREAT, 0.8f);
    DK_NEURAL_RULE(facts->lastAction > 0.10f && facts->lastAction < 0.25f && facts->enemyVisible > 0.5f,
        DK_NEURAL_ACTION_ATTACK_TARGET, 0.6f);

    DK_NEURAL_RULE(facts->enemyPresent < 0.5f && facts->ownerDistance > 0.50f,
        DK_NEURAL_ACTION_FOLLOW_OWNER, 2.5f);
    DK_NEURAL_RULE(facts->enemyPresent < 0.5f && facts->ownerDistance < 0.20f,
        DK_NEURAL_ACTION_HOLD, 1.8f);
    DK_NEURAL_RULE(facts->enemyPresent < 0.5f && facts->squadDistance < 0.20f,
        DK_NEURAL_ACTION_HOLD, 1.0f);
    DK_NEURAL_RULE(facts->enemyPresent < 0.5f && facts->ownerInPVS < 0.5f && facts->ownerInPHS < 0.5f,
        DK_NEURAL_ACTION_FOLLOW_OWNER, 1.5f);
    DK_NEURAL_RULE(facts->enemyPresent > 0.5f && facts->enemyVisible > 0.5f && facts->enemyDistance < 0.12f,
        DK_NEURAL_ACTION_ATTACK_TARGET, 1.0f);
    DK_NEURAL_RULE(facts->enemyPresent > 0.5f && facts->enemyVisible > 0.5f && facts->enemyDistance > 0.65f,
        DK_NEURAL_ACTION_HOLD, 1.0f);
    DK_NEURAL_RULE(facts->enemyTargetingSelf > 0.5f && facts->coverQuality > 0.55f,
        DK_NEURAL_ACTION_TAKE_COVER, 2.0f);
    DK_NEURAL_RULE(facts->enemyTargetingSelf > 0.5f && facts->backwardOpen > 0.60f && facts->selfHealth < 0.50f,
        DK_NEURAL_ACTION_RETREAT, 2.5f);
    DK_NEURAL_RULE(facts->enemyTargetingOwner > 0.5f && facts->enemyVisible > 0.5f,
        DK_NEURAL_ACTION_ATTACK_TARGET, 2.0f);
    DK_NEURAL_RULE(facts->enemyTargetingOwner > 0.5f && facts->enemyVisible < 0.5f && facts->leftOpen > 0.50f,
        DK_NEURAL_ACTION_FLANK_LEFT, 1.0f);
    DK_NEURAL_RULE(facts->enemyTargetingOwner > 0.5f && facts->enemyVisible < 0.5f && facts->rightOpen > 0.50f,
        DK_NEURAL_ACTION_FLANK_RIGHT, 1.0f);
    DK_NEURAL_RULE(facts->ownerDistance > 0.75f && facts->ownerVisible < 0.5f && facts->enemyPresent > 0.5f,
        DK_NEURAL_ACTION_FOLLOW_OWNER, 1.5f);
    DK_NEURAL_RULE(facts->ownerDistance < 0.25f && facts->ownerDanger < 0.25f && facts->enemyPresent < 0.5f,
        DK_NEURAL_ACTION_HOLD, 1.5f);
    DK_NEURAL_RULE(facts->squadPresent > 0.5f && facts->squadDistance > 0.75f,
        DK_NEURAL_ACTION_FOLLOW_OWNER, 1.0f);
    DK_NEURAL_RULE(facts->squadPresent > 0.5f && facts->squadHealth < 0.20f && facts->ownerDanger < 0.50f,
        DK_NEURAL_ACTION_FOLLOW_OWNER, 1.2f);
    DK_NEURAL_RULE(facts->squadPresent > 0.5f && facts->squadVisible > 0.5f && facts->squadThreat > 0.50f,
        DK_NEURAL_ACTION_ATTACK_TARGET, 1.0f);
    DK_NEURAL_RULE(facts->lowHeadroom > 0.5f && facts->narrowSpace > 0.5f,
        DK_NEURAL_ACTION_HOLD, 1.0f);
    DK_NEURAL_RULE(facts->dropAhead > 0.5f && facts->leftOpen > 0.50f && facts->leftHazard < 0.30f,
        DK_NEURAL_ACTION_FLANK_LEFT, 2.0f);
    DK_NEURAL_RULE(facts->dropAhead > 0.5f && facts->rightOpen > 0.50f && facts->rightHazard < 0.30f,
        DK_NEURAL_ACTION_FLANK_RIGHT, 2.0f);
    DK_NEURAL_RULE(facts->dropAhead > 0.5f && facts->leftOpen < 0.25f && facts->rightOpen < 0.25f && facts->backwardOpen > 0.40f,
        DK_NEURAL_ACTION_RETREAT, 3.5f);
    DK_NEURAL_RULE(facts->forwardOpen < 0.35f && facts->leftOpen < 0.35f && facts->rightOpen < 0.35f && facts->backwardOpen > 0.50f,
        DK_NEURAL_ACTION_RETREAT, 2.0f);
    DK_NEURAL_RULE(facts->leftOpen > 0.80f && facts->rightOpen < 0.30f && facts->enemyPresent > 0.5f,
        DK_NEURAL_ACTION_FLANK_LEFT, 1.5f);
    DK_NEURAL_RULE(facts->rightOpen > 0.80f && facts->leftOpen < 0.30f && facts->enemyPresent > 0.5f,
        DK_NEURAL_ACTION_FLANK_RIGHT, 1.5f);
    DK_NEURAL_RULE(facts->enemyInPVS < 0.5f && facts->enemyInPHS > 0.5f && facts->leftOpen > facts->rightOpen,
        DK_NEURAL_ACTION_FLANK_LEFT, 0.8f);
    DK_NEURAL_RULE(facts->enemyInPVS < 0.5f && facts->enemyInPHS > 0.5f && facts->rightOpen > facts->leftOpen,
        DK_NEURAL_ACTION_FLANK_RIGHT, 0.8f);
    DK_NEURAL_RULE(facts->enemyInPVS < 0.5f && facts->enemyInPHS < 0.5f && facts->ownerDanger < 0.30f,
        DK_NEURAL_ACTION_FOLLOW_OWNER, 1.0f);
    DK_NEURAL_RULE(facts->currentGoal > 0.80f && facts->ownerDanger < 0.40f,
        DK_NEURAL_ACTION_HOLD, 0.7f);
    DK_NEURAL_RULE(facts->currentTask > 0.80f && facts->enemyPresent < 0.5f,
        DK_NEURAL_ACTION_FOLLOW_OWNER, 0.7f);
    DK_NEURAL_RULE(facts->lastAction > 0.80f && facts->ownerDistance > 0.40f,
        DK_NEURAL_ACTION_HOLD, 0.4f);
    DK_NEURAL_RULE(facts->hasKnownWeapon > 0.5f && facts->ammo > 0.70f && facts->enemyVisible > 0.5f,
        DK_NEURAL_ACTION_ATTACK_TARGET, 1.0f);

    if (facts->ownerDanger > 0.70f)
    {
        scores[DK_NEURAL_ACTION_ATTACK_TARGET - 1] += 2.0f;
        scores[DK_NEURAL_ACTION_HOLD - 1] -= 2.0f;
        scores[DK_NEURAL_ACTION_FLANK_LEFT - 1] -= 1.5f;
        scores[DK_NEURAL_ACTION_FLANK_RIGHT - 1] -= 1.5f;
    }

    if (facts->selfHealth < DK_NEURAL_HEALTH_CRITICAL)
    {
        scores[DK_NEURAL_ACTION_RETREAT - 1] += 3.0f;
        scores[DK_NEURAL_ACTION_FLANK_LEFT - 1] -= 2.0f;
        scores[DK_NEURAL_ACTION_FLANK_RIGHT - 1] -= 2.0f;
    }

    if (facts->threatCount >= DK_NEURAL_OVERWHELMING_THREATS)
    {
        scores[DK_NEURAL_ACTION_RETREAT - 1] += 2.5f;
        scores[DK_NEURAL_ACTION_TAKE_COVER - 1] += 1.5f;
        scores[DK_NEURAL_ACTION_ATTACK_TARGET - 1] -= 1.0f;
    }

    if (state->bsp.self_contents[0] > 0.5f)
        scores[DK_NEURAL_ACTION_HOLD - 1] -= 0.5f;
}

static int DKNeuralAI_ComputeExpertAction(const dk_neural_tactical_state_t *state, float *scores)
{
    dk_neural_facts_t facts;
    int absoluteAction;
    int action;
    int bestAction;

    DKNeuralAI_BuildFacts(state, &facts);
    absoluteAction = DKNeuralAI_AbsoluteRuleAction(&facts);
    DKNeuralAI_AddRuleScores(state, &facts, scores);

    if (absoluteAction != DK_NEURAL_ACTION_INVALID)
        return absoluteAction;

    bestAction = DK_NEURAL_ACTION_FOLLOW_OWNER;
    for (action = DK_NEURAL_ACTION_ATTACK_TARGET; action <= DK_NEURAL_ACTION_HOLD; ++action)
    {
        if (scores[action - 1] > scores[bestAction - 1])
            bestAction = action;
    }

    return bestAction;
}

static void DKNeuralAI_RunNetwork(dk_neural_model_t *model, const float *input, float *scores)
{
    int i;
    int j;
    int action;

    for (i = 0; i < DK_NEURAL_HIDDEN_1; ++i)
    {
        float sum = model->hidden1_bias[i];
        const float *weights = model->input_to_hidden1 + (size_t)i * DK_NEURAL_INPUT_COUNT;
        for (j = 0; j < DK_NEURAL_INPUT_COUNT; ++j)
            sum += weights[j] * input[j];
        model->hidden1_activation[i] = DKNeuralAI_Relu(sum);
    }

    for (i = 0; i < DK_NEURAL_HIDDEN_2; ++i)
    {
        float sum = model->hidden2_bias[i];
        const float *weights = model->hidden1_to_hidden2 + (size_t)i * DK_NEURAL_HIDDEN_1;
        for (j = 0; j < DK_NEURAL_HIDDEN_1; ++j)
            sum += weights[j] * model->hidden1_activation[j];
        model->hidden2_activation[i] = DKNeuralAI_Relu(sum);
    }

    for (action = 0; action < DK_NEURAL_ACTION_COUNT; ++action)
    {
        float sum = model->output_bias[action];
        const float *weights = model->hidden2_to_output + (size_t)action * DK_NEURAL_HIDDEN_2;
        for (i = 0; i < DK_NEURAL_HIDDEN_2; ++i)
            sum += weights[i] * model->hidden2_activation[i];
        scores[action] = sum;
    }
}

static int DKNeuralAI_BestNeuralAction(const float *scores)
{
    int action;
    int bestAction = DK_NEURAL_ACTION_FOLLOW_OWNER;

    for (action = DK_NEURAL_ACTION_ATTACK_TARGET; action <= DK_NEURAL_ACTION_HOLD; ++action)
    {
        if (scores[action - 1] > scores[bestAction - 1])
            bestAction = action;
    }

    return bestAction;
}

static int DKNeuralAI_SelectNeuralAction(const dk_neural_tactical_state_t *state, const float *expertScores)
{
    float input[DK_NEURAL_INPUT_COUNT];
    float neuralScores[DK_NEURAL_ACTION_COUNT];
    float probabilities[DK_NEURAL_ACTION_COUNT];
    float maximumScore;
    float sum;
    float expertBias;
    int action;
    int bestAction;
    int modelIndex;
    dk_neural_model_t *model;

    DKNeuralAI_NormalizeInput(state, input);

    modelIndex = dk_neural_active_model.load(std::memory_order_acquire);
    model = &dk_neural_models[modelIndex];
    DKNeuralAI_RunNetwork(model, input, neuralScores);

    maximumScore = neuralScores[0];
    for (action = 1; action < DK_NEURAL_ACTION_COUNT; ++action)
        if (neuralScores[action] > maximumScore)
            maximumScore = neuralScores[action];

    sum = 0.0f;
    for (action = 0; action < DK_NEURAL_ACTION_COUNT; ++action)
    {
        probabilities[action] = (float)exp((double)DKNeuralAI_Clamp(neuralScores[action] - maximumScore, -40.0f, 0.0f));
        sum += probabilities[action];
    }

    if (sum > 0.0f)
        for (action = 0; action < DK_NEURAL_ACTION_COUNT; ++action)
            probabilities[action] /= sum;

    bestAction = DKNeuralAI_BestNeuralAction(probabilities);

    expertBias = expertScores[bestAction - 1] * 0.025f;
    for (action = 1; action <= DK_NEURAL_ACTION_COUNT; ++action)
    {
        float candidate = probabilities[action - 1] + expertScores[action - 1] * 0.025f;
        float best = probabilities[bestAction - 1] + expertBias;
        if (candidate > best)
        {
            bestAction = action;
            expertBias = expertScores[action - 1] * 0.025f;
        }
    }

    return bestAction;
}

static int DKNeuralAI_SelectAction(const dk_neural_tactical_state_t *state)
{
    float expertScores[DK_NEURAL_ACTION_COUNT];
    int expertAction;
    int absoluteAction;
    dk_neural_facts_t facts;

    expertAction = DKNeuralAI_ComputeExpertAction(state, expertScores);
    DKNeuralAI_BuildFacts(state, &facts);
    absoluteAction = DKNeuralAI_AbsoluteRuleAction(&facts);

    if (absoluteAction != DK_NEURAL_ACTION_INVALID)
        return absoluteAction;

    if (!dk_neural_ai || dk_neural_ai->value <= 0.0f ||
        !dk_neural_trained_weights_loaded.load(std::memory_order_acquire))
        return expertAction;

    return DKNeuralAI_SelectNeuralAction(state, expertScores);
}

static void DKNeuralAI_ResolveDatasetPath(void)
{
    const char *configured = dk_neural_dataset ? dk_neural_dataset->string :
        "neural/models/DaikatanaAI.dkt";
    const char *extension;

    /*
     * Keep the live training-dataset format at .dkt.  Older builds used .dkd;
     * an archived dk_neural_dataset cvar can otherwise silently recreate the
     * old extension even though the current default is .dkt.
     */
    extension = strrchr(configured, '.');
    if (extension && !stricmp(extension, ".dkd"))
    {
        char migrated[DK_NEURAL_MAX_MODEL_PATH];

        Com_sprintf(migrated, sizeof(migrated), "%s", configured);
        migrated[(size_t)(extension - configured)] = '.';
        migrated[(size_t)(extension - configured) + 1] = 'd';
        migrated[(size_t)(extension - configured) + 2] = 'k';
        migrated[(size_t)(extension - configured) + 3] = 't';
        migrated[(size_t)(extension - configured) + 4] = '\0';

        configured = migrated;
        if (dk_neural_dataset && stricmp(dk_neural_dataset->string, migrated))
            Cvar_Set("dk_neural_dataset", migrated);
    }

    if (DKNeuralAI_IsAbsolutePath(configured))
        Com_sprintf(dk_neural_resolved_dataset_path, sizeof(dk_neural_resolved_dataset_path), "%s", configured);
    else
        Com_sprintf(dk_neural_resolved_dataset_path, sizeof(dk_neural_resolved_dataset_path), "%s/%s", FS_Gamedir(), configured);
}

static qboolean DKNeuralAI_OpenDatasetFile(void)
{
    FILE *file;
    dk_neural_training_file_header_t header;
    qboolean newFile = FALSE;

    if (dk_neural_dataset_file)
        return TRUE;

    DKNeuralAI_ResolveDatasetPath();

    file = fopen(dk_neural_resolved_dataset_path, "rb");
    if (!file)
    {
        newFile = TRUE;
    }
    else
    {
        if (fread(&header, sizeof(header), 1, file) != 1 ||
            header.magic != DK_NEURAL_TRAINING_MAGIC ||
            header.version != DK_NEURAL_TRAINING_VERSION ||
            header.input_count != DK_NEURAL_INPUT_COUNT ||
            header.action_count != DK_NEURAL_ACTION_COUNT ||
            header.record_size != DK_NEURAL_TRAINING_RECORD_SIZE)
        {
            fclose(file);
            Com_Printf("DaikatanaAI: invalid training dataset, refusing to append: %s\n",
                dk_neural_resolved_dataset_path);
            return FALSE;
        }
        fclose(file);
    }

    dk_neural_dataset_file = fopen(dk_neural_resolved_dataset_path, newFile ? "wb" : "ab");
    if (!dk_neural_dataset_file)
    {
        Com_Printf("DaikatanaAI: unable to open training dataset: %s\n",
            dk_neural_resolved_dataset_path);
        return FALSE;
    }

    setvbuf(dk_neural_dataset_file, NULL, _IOFBF, 64 * 1024);

    if (newFile)
    {
        memset(&header, 0, sizeof(header));
        header.magic = DK_NEURAL_TRAINING_MAGIC;
        header.version = DK_NEURAL_TRAINING_VERSION;
        header.input_count = DK_NEURAL_INPUT_COUNT;
        header.action_count = DK_NEURAL_ACTION_COUNT;
        header.record_size = DK_NEURAL_TRAINING_RECORD_SIZE;
        header.reserved = 0;
        header.sample_count = 0;
        if (fwrite(&header, sizeof(header), 1, dk_neural_dataset_file) != 1)
        {
            fclose(dk_neural_dataset_file);
            dk_neural_dataset_file = NULL;
            return FALSE;
        }
    }

    dk_neural_dataset_sample_count = 0;
    return TRUE;
}

static void DKNeuralAI_RecordTrainingSample(const dk_neural_tactical_state_t *state, int action)
{
    float normalizedState[DK_NEURAL_INPUT_COUNT];
    uint32_t storedAction;

    if (!dk_neural_record_samples || dk_neural_record_samples->value <= 0.0f ||
        action < DK_NEURAL_ACTION_FOLLOW_OWNER || action > DK_NEURAL_ACTION_HOLD)
        return;

    if (!DKNeuralAI_OpenDatasetFile())
        return;

    DKNeuralAI_NormalizeInput(state, normalizedState);
    storedAction = (uint32_t)action;

    if (fwrite(normalizedState, sizeof(float), DK_NEURAL_INPUT_COUNT, dk_neural_dataset_file) != DK_NEURAL_INPUT_COUNT ||
        fwrite(&storedAction, sizeof(storedAction), 1, dk_neural_dataset_file) != 1)
    {
        Com_Printf("DaikatanaAI: training dataset write failed: %s\n",
            dk_neural_resolved_dataset_path);
        fclose(dk_neural_dataset_file);
        dk_neural_dataset_file = NULL;
        return;
    }

    ++dk_neural_dataset_sample_count;
    if ((dk_neural_dataset_sample_count & 63ULL) == 0)
        fflush(dk_neural_dataset_file);
}

static void DKNeuralAI_CloseDatasetFile(void)
{
    if (!dk_neural_dataset_file)
        return;

    fflush(dk_neural_dataset_file);
    fclose(dk_neural_dataset_file);
    dk_neural_dataset_file = NULL;
}

static void DKNeuralAI_ResetDataset_f(void)
{
    DKNeuralAI_CloseDatasetFile();
    DKNeuralAI_ResolveDatasetPath();
    remove(dk_neural_resolved_dataset_path);
    Com_Printf("DaikatanaAI: training dataset reset: %s\n", dk_neural_resolved_dataset_path);
}

static void DKNeuralAI_LoadCurrentWeights(void)
{
    int activeIndex;
    int shadowIndex;

    dk_neural_weight_load_attempted = TRUE;
    DKNeuralAI_ResolveWeightsPath();
    activeIndex = dk_neural_active_model.load(std::memory_order_acquire);
    shadowIndex = activeIndex ^ 1;

    dk_neural_trained_weights_loaded.store(0, std::memory_order_release);
    if (!DKNeuralAI_LoadWeightsFromPath(dk_neural_resolved_weights_path, activeIndex))
        return;

    memcpy(dk_neural_models[shadowIndex].parameters, dk_neural_models[activeIndex].parameters,
        DKNeuralAI_WeightCount() * sizeof(float));
    dk_neural_trained_weights_loaded.store(1, std::memory_order_release);
    Com_Printf("DaikatanaAI: loaded trained weights: %s\n", dk_neural_resolved_weights_path);
}

static void DKNeuralAI_Status_f(void)
{
    Com_Printf("DaikatanaAI: rules=advanced %s, neural %s, weights=%s, parameters=%lu\n",
        dk_advanced_reasoning && dk_advanced_reasoning->value > 0.0f ? "ON" : "OFF",
        dk_neural_ai && dk_neural_ai->value > 0.0f ? "ON" : "OFF",
        dk_neural_trained_weights_loaded.load(std::memory_order_acquire) ? "LOADED" : "BOOTSTRAP",
        DK_NEURAL_PARAMETER_COUNT);
    Com_Printf("DaikatanaAI: dataset recording=%s, session samples=%llu, file=%s\n",
        dk_neural_record_samples && dk_neural_record_samples->value > 0.0f ? "ON" : "OFF",
        (unsigned long long)dk_neural_dataset_sample_count,
        dk_neural_resolved_dataset_path);
}

static void DKNeuralAI_Load_f(void)
{
    DKNeuralAI_LoadCurrentWeights();
    dk_neural_weight_load_attempted = TRUE;
}

static void DKNeuralAI_Save_f(void)
{
    if (DKNeuralAI_SaveWeightsToPath(dk_neural_resolved_weights_path))
        Com_Printf("DaikatanaAI: saved weights: %s\n", dk_neural_resolved_weights_path);
    else
        Com_Printf("DaikatanaAI: unable to save weights: %s\n", dk_neural_resolved_weights_path);
}

static void DKNeuralAI_Reload_f(void)
{
    DKNeuralAI_CloseDatasetFile();
    DKNeuralAI_FreeModels();
    DKNeuralAI_InitializeModels();
    if (dk_neural_models[0].allocated && dk_neural_models[1].allocated)
        DKNeuralAI_LoadCurrentWeights();
}

static void DKNeuralAI_Reset_f(void)
{
    DKNeuralAI_ResolveWeightsPath();
    {
        char temporaryPath[DK_NEURAL_MAX_MODEL_PATH + 8];
        Com_sprintf(temporaryPath, sizeof(temporaryPath), "%s.tmp", dk_neural_resolved_weights_path);
        remove(dk_neural_resolved_weights_path);
        remove(temporaryPath);
    }
    dk_neural_trained_weights_loaded.store(0, std::memory_order_release);
    DKNeuralAI_FreeModels();
    DKNeuralAI_InitializeModels();
    Com_Printf("DaikatanaAI: model reset to untrained bootstrap weights.\n");
}

static void DKNeuralAI_Test_f(void)
{
    dk_neural_tactical_state_t state;
    float scores[DK_NEURAL_ACTION_COUNT];
    int action;

    memset(&state, 0, sizeof(state));
    state.self.health_percent = 0.85f;
    state.self.armor_percent = 0.75f;
    state.self.ammo_fraction = 0.80f;
    state.self.weapon_ready = 1.0f;
    state.owner.alive = 1.0f;
    state.owner.health_percent = 0.75f;
    state.owner.distance = 0.10f;
    state.enemy.present = 1.0f;
    state.enemy.health_percent = 0.25f;
    state.enemy.distance = 0.18f;
    state.enemy.visible = 1.0f;
    state.enemy.easy_target = 0.75f;
    state.enemy.targeting_self = 1.0f;
    state.geometry.horizontal[0][0] = 0.80f;
    state.geometry.horizontal[2][0] = 0.80f;
    state.geometry.horizontal[4][0] = 0.80f;
    state.geometry.horizontal[6][0] = 0.80f;

    action = DKNeuralAI_ComputeExpertAction(&state, scores);
    Com_Printf("Advanced Reasoning test: ACTION=%s\n", DKNeuralAI_ActionName(action));
}

void DKNeuralAI_Init(void)
{
    if (dk_neural_initialized)
        return;

    dk_neural_ai = Cvar_Get("dk_neural_ai", "0", CVAR_ARCHIVE);
    dk_advanced_reasoning = Cvar_Get("dk_advanced_reasoning", "0", CVAR_ARCHIVE);
    dk_neural_weights = Cvar_Get("dk_neural_weights", "neural/models/DaikatanaAI.dkai", CVAR_ARCHIVE);
    dk_neural_debug = Cvar_Get("dk_neural_debug", "0", CVAR_ARCHIVE);
    dk_neural_record_samples = Cvar_Get("dk_neural_record_samples", "1", CVAR_ARCHIVE);
    dk_neural_dataset = Cvar_Get("dk_neural_dataset", "neural/models/DaikatanaAI.dkt", CVAR_ARCHIVE);

    Cmd_AddCommand("dk_neural_status", DKNeuralAI_Status_f);
    Cmd_AddCommand("dk_neural_load", DKNeuralAI_Load_f);
    Cmd_AddCommand("dk_neural_save", DKNeuralAI_Save_f);
    Cmd_AddCommand("dk_neural_reload", DKNeuralAI_Reload_f);
    Cmd_AddCommand("dk_neural_reset", DKNeuralAI_Reset_f);
    Cmd_AddCommand("dk_neural_test", DKNeuralAI_Test_f);
    Cmd_AddCommand("dk_neural_dataset_reset", DKNeuralAI_ResetDataset_f);

    DKNeuralAI_ResolveWeightsPath();
    DKNeuralAI_InitializeModels();
    if (!dk_neural_models[0].allocated || !dk_neural_models[1].allocated)
        return;

    dk_neural_initialized = TRUE;
    DKNeuralAI_LoadCurrentWeights();
}

void DKNeuralAI_Shutdown(void)
{
    if (!dk_neural_initialized)
        return;

    DKNeuralAI_CloseDatasetFile();
    DKNeuralAI_FreeModels();
    dk_neural_initialized = FALSE;
}

#if defined(__GNUC__) && !defined(_MSC_VER)
__attribute__((used))
#endif
int DKNeuralAI_RunTacticalState(const dk_neural_tactical_state_t *state, int *action)
{
    float expertScores[DK_NEURAL_ACTION_COUNT];
    int expertAction;

    if (!state || !action)
        return 0;

    if (!dk_neural_initialized)
        DKNeuralAI_Init();

    if (!dk_neural_initialized)
        return 0;

    if (!dk_neural_weight_load_attempted)
    {
        dk_neural_weight_load_attempted = TRUE;
        DKNeuralAI_LoadCurrentWeights();
    }

    expertAction = DKNeuralAI_ComputeExpertAction(state, expertScores);
    if ((dk_advanced_reasoning && dk_advanced_reasoning->value > 0.0f) ||
        (dk_neural_ai && dk_neural_ai->value > 0.0f))
        DKNeuralAI_RecordTrainingSample(state, expertAction);
    *action = DKNeuralAI_SelectAction(state);

    if (dk_neural_debug && dk_neural_debug->value > 0.0f)
    {
        Com_Printf("Tactical decision: expert=%s selected=%s advanced=%d neural=%d trained=%d\n",
            DKNeuralAI_ActionName(expertAction), DKNeuralAI_ActionName(*action),
            dk_advanced_reasoning ? (int)dk_advanced_reasoning->value : 0,
            dk_neural_ai ? (int)dk_neural_ai->value : 0,
            dk_neural_trained_weights_loaded.load(std::memory_order_acquire));
    }

    return *action >= DK_NEURAL_ACTION_FOLLOW_OWNER && *action <= DK_NEURAL_ACTION_HOLD ? 1 : 0;
}
