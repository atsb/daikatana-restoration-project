#ifndef DK_NEURAL_AI_H
#define DK_NEURAL_AI_H

#ifdef __cplusplus
extern "C" {
#endif

#define DK_NEURAL_INPUT_COUNT       302
#define DK_NEURAL_HIDDEN_1         3891
#define DK_NEURAL_HIDDEN_2          980
#define DK_NEURAL_ACTION_COUNT        7
#define DK_NEURAL_PARAMETER_COUNT 5000000UL
#define DK_NEURAL_MODEL_VERSION       1

    typedef enum
    {
        DK_NEURAL_ACTION_INVALID = 0,
        DK_NEURAL_ACTION_FOLLOW_OWNER = 1,
        DK_NEURAL_ACTION_ATTACK_TARGET = 2,
        DK_NEURAL_ACTION_RETREAT = 3,
        DK_NEURAL_ACTION_TAKE_COVER = 4,
        DK_NEURAL_ACTION_FLANK_LEFT = 5,
        DK_NEURAL_ACTION_FLANK_RIGHT = 6,
        DK_NEURAL_ACTION_HOLD = 7
    } dk_neural_action_t;

    typedef struct
    {
        float health_percent;
        float armor_percent;
        float speed;
        float velocity_x;
        float velocity_y;
        float velocity_z;
        float forward_x;
        float forward_y;
        float forward_z;
        float water_level;
        float on_ground;
        float enemy_present;
        float enemy_distance;
        float ammo_fraction;
        float weapon_ion;
        float weapon_shotcycler;
        float weapon_bolter;
        float weapon_other;
        float ai_stay;
        float ai_noattack;
        float ai_sidekickkill;
        float role_mikiko;
        float role_superfly;
        float current_task;
        float current_goal;
        float last_action;
        float movement_blocked;
        float weapon_ready;
    } dk_neural_self_state_t;

    typedef struct
    {
        float alive;
        float health_percent;
        float distance;
        float relative_forward;
        float relative_right;
        float relative_up;
        float under_attack;
        float target_self;
        float enemy_present;
        float enemy_distance;
        float in_pvs;
        float in_phs;
        float visible;
        float speed;
        float velocity_x;
        float velocity_y;
        float velocity_z;
        float danger_score;
        float critical;
        float formation_distance;
    } dk_neural_owner_state_t;

    typedef struct
    {
        float present;
        float health_percent;
        float distance;
        float relative_forward;
        float relative_right;
        float relative_up;
        float visible;
        float targeting_self;
        float targeting_owner;
        float in_pvs;
        float in_phs;
        float speed;
        float velocity_x;
        float velocity_y;
        float velocity_z;
        float type;
        float class_hash;
        float attack_range;
        float attack_type;
        float attack_mode;
        float threat_score;
        float critical;
        float easy_target;
        float close_range;
        float owner_threat;
        float self_threat;
        float movement_pressure;
        /* Compact temporal/context summary. Same slot/ABI as the old reserved float. */
        float context_summary;
    } dk_neural_enemy_state_t;

    typedef struct
    {
        float present;
        float health_percent;
        float distance;
        float relative_forward;
        float relative_right;
        float relative_up;
        float in_pvs;
        float in_phs;
        float visible;
        float speed;
        float velocity_x;
        float velocity_y;
        float velocity_z;
        float role_mikiko;
        float role_superfly;
        float distance_pressure;
        float health_pressure;
        float cooperation_need;
        float formation_quality;
        float threat_pressure;
    } dk_neural_squad_state_t;

    typedef struct
    {
        float present;
        float health_percent;
        float distance;
        float relative_forward;
        float relative_right;
        float relative_up;
        float visible;
        float targeting_self;
        float targeting_owner;
        float type;
        float class_hash;
        float speed;
        float in_pvs;
        float in_phs;
        float threat_score;
        float close_range;
    } dk_neural_threat_state_t;

    typedef struct
    {
        float horizontal[8][4];
        float floor_distance[8];
        float ceiling_distance[8];
    } dk_neural_geometry_state_t;

    typedef struct
    {
        float self_contents[8];
        float sample_contents[8];
        float owner_in_pvs;
        float enemy_in_pvs;
        float squad_in_pvs;
        float owner_in_phs;
        float enemy_in_phs;
        float squad_in_phs;
        float local_pvs_samples[4];
        float local_phs_samples[4];
    } dk_neural_bsp_state_t;

    typedef struct
    {
        dk_neural_self_state_t self;
        dk_neural_owner_state_t owner;
        dk_neural_enemy_state_t enemy;
        dk_neural_squad_state_t squad;
        dk_neural_threat_state_t threats[8];
        dk_neural_geometry_state_t geometry;
        dk_neural_bsp_state_t bsp;
    } dk_neural_tactical_state_t;

    void DKNeuralAI_Init(void);
    void DKNeuralAI_Shutdown(void);

#if defined(_MSC_VER)
    __declspec(dllexport)
#endif
        int DKNeuralAI_RunTacticalState(const dk_neural_tactical_state_t* state, int* action);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
static_assert(sizeof(dk_neural_tactical_state_t) == DK_NEURAL_INPUT_COUNT * sizeof(float),
    "Daikatana neural tactical state size changed");
#endif

#endif
