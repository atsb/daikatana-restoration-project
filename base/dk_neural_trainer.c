#include "dk_neural_ai.h"
#include "dk_neural_training_format.h"

#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DK_NEURAL_TRAINER_DEFAULT_EPOCHS 10
#define DK_NEURAL_TRAINER_DEFAULT_LEARNING_RATE 0.00005f
#define DK_NEURAL_TRAINER_DEFAULT_LABEL_SMOOTHING 0.05f
#define DK_NEURAL_TRAINER_DEFAULT_MAX_GRADIENT 1.0f
#define DK_NEURAL_TRAINER_PROGRESS_SAMPLES 10000ULL
#define DK_NEURAL_TRAINER_CHECKPOINT_EPOCHS 1
#define DK_NEURAL_TRAINER_MAX_CLASS_WEIGHT 4.0f
#define DK_NEURAL_TRAINER_VALIDATION_MODULUS 10ULL

#define DK_NEURAL_MODEL_MAGIC ((uint32_t)0x354B4444U)

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
} dk_neural_trainer_model_t;

typedef struct
{
    const char *input_path;
    const char *output_path;
    int epochs;
    float learning_rate;
    float label_smoothing;
    int resume;
    uint64_t max_samples;
    uint64_t validation_modulus;
} dk_neural_trainer_options_t;

typedef struct
{
    uint64_t total_samples;
    uint64_t action_counts[DK_NEURAL_ACTION_COUNT];
    uint64_t training_samples;
    uint64_t validation_samples;
} dk_neural_dataset_info_t;

static size_t DKNeuralTrainer_WeightCount(void)
{
    return (size_t)DK_NEURAL_PARAMETER_COUNT;
}

static float DKNeuralTrainer_Clamp(float value, float minimum, float maximum)
{
    if (value < minimum)
        return minimum;
    if (value > maximum)
        return maximum;
    return value;
}

static float DKNeuralTrainer_Relu(float value)
{
    return value > 0.0f ? value : 0.0f;
}

static float DKNeuralTrainer_ReluDerivative(float value)
{
    return value > 0.0f ? 1.0f : 0.0f;
}

static uint32_t DKNeuralTrainer_RandomStep(uint32_t *seed)
{
    *seed = *seed * 1664525U + 1013904223U;
    return *seed;
}

static float DKNeuralTrainer_RandomSigned(uint32_t *seed, float scale)
{
    uint32_t value = DKNeuralTrainer_RandomStep(seed);
    float normalized = (float)(value & 0x00FFFFFFU) / 16777215.0f;
    return (normalized * 2.0f - 1.0f) * scale;
}

static void DKNeuralTrainer_AssignWeightViews(dk_neural_trainer_model_t *model)
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

static void DKNeuralTrainer_FreeModel(dk_neural_trainer_model_t *model)
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

static int DKNeuralTrainer_InitializeModel(dk_neural_trainer_model_t *model)
{
    size_t index;
    uint32_t seed = 0x4B445441U;

    memset(model, 0, sizeof(*model));
    model->parameters = (float *)calloc(DKNeuralTrainer_WeightCount(), sizeof(float));
    model->hidden1_activation = (float *)calloc(DK_NEURAL_HIDDEN_1, sizeof(float));
    model->hidden2_activation = (float *)calloc(DK_NEURAL_HIDDEN_2, sizeof(float));
    model->hidden1_delta = (float *)calloc(DK_NEURAL_HIDDEN_1, sizeof(float));
    model->hidden2_delta = (float *)calloc(DK_NEURAL_HIDDEN_2, sizeof(float));

    if (!model->parameters || !model->hidden1_activation || !model->hidden2_activation ||
        !model->hidden1_delta || !model->hidden2_delta)
    {
        DKNeuralTrainer_FreeModel(model);
        return 0;
    }

    DKNeuralTrainer_AssignWeightViews(model);

    for (index = 0; index < (size_t)DK_NEURAL_INPUT_COUNT * DK_NEURAL_HIDDEN_1; ++index)
        model->input_to_hidden1[index] = DKNeuralTrainer_RandomSigned(&seed, 0.08f);

    for (index = 0; index < (size_t)DK_NEURAL_HIDDEN_1 * DK_NEURAL_HIDDEN_2; ++index)
        model->hidden1_to_hidden2[index] = DKNeuralTrainer_RandomSigned(&seed, 0.04f);

    for (index = 0; index < (size_t)DK_NEURAL_HIDDEN_2 * DK_NEURAL_ACTION_COUNT; ++index)
        model->hidden2_to_output[index] = DKNeuralTrainer_RandomSigned(&seed, 0.04f);

    return 1;
}

static int DKNeuralTrainer_LoadModel(const char *path, dk_neural_trainer_model_t *model)
{
    FILE *file;
    dk_neural_model_file_header_t header;

    file = fopen(path, "rb");
    if (!file)
        return 0;

    if (fread(&header, sizeof(header), 1, file) != 1 ||
        header.magic != DK_NEURAL_MODEL_MAGIC ||
        header.version != DK_NEURAL_MODEL_VERSION ||
        header.input_count != DK_NEURAL_INPUT_COUNT ||
        header.hidden1_count != DK_NEURAL_HIDDEN_1 ||
        header.hidden2_count != DK_NEURAL_HIDDEN_2 ||
        header.output_count != DK_NEURAL_ACTION_COUNT ||
        header.parameter_count != DK_NEURAL_PARAMETER_COUNT)
    {
        fclose(file);
        return 0;
    }

    if (fread(model->parameters, sizeof(float), DKNeuralTrainer_WeightCount(), file) != DKNeuralTrainer_WeightCount())
    {
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}

static int DKNeuralTrainer_SaveModel(const char *path, const dk_neural_trainer_model_t *model)
{
    char temporary_path[4096];
    FILE *file;
    dk_neural_model_file_header_t header;

    if (snprintf(temporary_path, sizeof(temporary_path), "%s.tmp", path) < 0)
        return 0;

    file = fopen(temporary_path, "wb");
    if (!file)
    {
        fprintf(stderr, "Unable to create '%s': %s\n", temporary_path, strerror(errno));
        return 0;
    }

    memset(&header, 0, sizeof(header));
    header.magic = DK_NEURAL_MODEL_MAGIC;
    header.version = DK_NEURAL_MODEL_VERSION;
    header.input_count = DK_NEURAL_INPUT_COUNT;
    header.hidden1_count = DK_NEURAL_HIDDEN_1;
    header.hidden2_count = DK_NEURAL_HIDDEN_2;
    header.output_count = DK_NEURAL_ACTION_COUNT;
    header.parameter_count = DK_NEURAL_PARAMETER_COUNT;

    if (fwrite(&header, sizeof(header), 1, file) != 1 ||
        fwrite(model->parameters, sizeof(float), DKNeuralTrainer_WeightCount(), file) != DKNeuralTrainer_WeightCount())
    {
        fclose(file);
        remove(temporary_path);
        return 0;
    }

    if (fclose(file) != 0)
    {
        remove(temporary_path);
        return 0;
    }

    remove(path);
    if (rename(temporary_path, path) != 0)
    {
        remove(temporary_path);
        return 0;
    }

    return 1;
}

static int DKNeuralTrainer_ReadTrainingHeader(FILE *file, dk_neural_training_file_header_t *header)
{
    if (fread(header, sizeof(*header), 1, file) != 1)
        return 0;

    if (header->magic != DK_NEURAL_TRAINING_MAGIC ||
        header->version != DK_NEURAL_TRAINING_VERSION ||
        header->input_count != DK_NEURAL_INPUT_COUNT ||
        header->action_count != DK_NEURAL_ACTION_COUNT ||
        header->record_size != DK_NEURAL_TRAINING_RECORD_SIZE)
        return 0;

    return 1;
}

static int DKNeuralTrainer_ScanDataset(const dk_neural_trainer_options_t *options,
    dk_neural_dataset_info_t *info)
{
    FILE *file;
    dk_neural_training_file_header_t header;
    float input[DK_NEURAL_INPUT_COUNT];
    uint32_t action;
    uint64_t sampleIndex;
    uint64_t maximumSamples;

    memset(info, 0, sizeof(*info));
    file = fopen(options->input_path, "rb");
    if (!file)
    {
        fprintf(stderr, "Unable to open training data '%s': %s\n", options->input_path, strerror(errno));
        return 0;
    }

    if (!DKNeuralTrainer_ReadTrainingHeader(file, &header))
    {
        fprintf(stderr, "Invalid Daikatana tactical training header in '%s'.\n", options->input_path);
        fclose(file);
        return 0;
    }

    maximumSamples = options->max_samples;
    sampleIndex = 0;

    while ((maximumSamples == 0 || sampleIndex < maximumSamples) &&
           fread(input, sizeof(float), DK_NEURAL_INPUT_COUNT, file) == DK_NEURAL_INPUT_COUNT &&
           fread(&action, sizeof(action), 1, file) == 1)
    {
        if (action >= DK_NEURAL_ACTION_FOLLOW_OWNER && action <= DK_NEURAL_ACTION_HOLD)
        {
            ++info->total_samples;
            ++info->action_counts[action - 1];
            if (options->validation_modulus > 0 &&
                (sampleIndex % options->validation_modulus) == 0)
                ++info->validation_samples;
            else
                ++info->training_samples;
        }
        ++sampleIndex;
    }

    fclose(file);
    return info->total_samples > 0 ? 1 : 0;
}

static void DKNeuralTrainer_RunNetwork(dk_neural_trainer_model_t *model,
    const float *input, float *logits)
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
        model->hidden1_activation[i] = DKNeuralTrainer_Relu(sum);
    }

    for (i = 0; i < DK_NEURAL_HIDDEN_2; ++i)
    {
        float sum = model->hidden2_bias[i];
        const float *weights = model->hidden1_to_hidden2 + (size_t)i * DK_NEURAL_HIDDEN_1;
        for (j = 0; j < DK_NEURAL_HIDDEN_1; ++j)
            sum += weights[j] * model->hidden1_activation[j];
        model->hidden2_activation[i] = DKNeuralTrainer_Relu(sum);
    }

    for (action = 0; action < DK_NEURAL_ACTION_COUNT; ++action)
    {
        float sum = model->output_bias[action];
        const float *weights = model->hidden2_to_output + (size_t)action * DK_NEURAL_HIDDEN_2;
        for (i = 0; i < DK_NEURAL_HIDDEN_2; ++i)
            sum += weights[i] * model->hidden2_activation[i];
        logits[action] = sum;
    }
}

static int DKNeuralTrainer_Predict(dk_neural_trainer_model_t *model, const float *input)
{
    float logits[DK_NEURAL_ACTION_COUNT];
    float maximum;
    int action;
    int bestAction = DK_NEURAL_ACTION_FOLLOW_OWNER;

    DKNeuralTrainer_RunNetwork(model, input, logits);
    maximum = logits[0];
    for (action = 1; action < DK_NEURAL_ACTION_COUNT; ++action)
        if (logits[action] > maximum)
        {
            maximum = logits[action];
            bestAction = action + 1;
        }

    return bestAction;
}

static float DKNeuralTrainer_ComputeClassWeight(const dk_neural_dataset_info_t *info, int action)
{
    float weight;

    if (action < DK_NEURAL_ACTION_FOLLOW_OWNER || action > DK_NEURAL_ACTION_HOLD ||
        info->action_counts[action - 1] == 0)
        return 1.0f;

    weight = (float)info->total_samples /
        ((float)DK_NEURAL_ACTION_COUNT * (float)info->action_counts[action - 1]);
    return DKNeuralTrainer_Clamp(weight, 0.25f, DK_NEURAL_TRAINER_MAX_CLASS_WEIGHT);
}

static float DKNeuralTrainer_TrainSample(dk_neural_trainer_model_t *model,
    const float *input, int targetAction, float learningRate, float labelSmoothing, float classWeight)
{
    float logits[DK_NEURAL_ACTION_COUNT];
    float probabilities[DK_NEURAL_ACTION_COUNT];
    float outputDelta[DK_NEURAL_ACTION_COUNT];
    float maximumLogit;
    float probabilitySum;
    float targetProbability;
    float gradientScale;
    float loss;
    int action;
    int i;
    int j;

    DKNeuralTrainer_RunNetwork(model, input, logits);

    maximumLogit = logits[0];
    for (action = 1; action < DK_NEURAL_ACTION_COUNT; ++action)
        if (logits[action] > maximumLogit)
            maximumLogit = logits[action];

    probabilitySum = 0.0f;
    for (action = 0; action < DK_NEURAL_ACTION_COUNT; ++action)
    {
        probabilities[action] = (float)exp((double)DKNeuralTrainer_Clamp(
            logits[action] - maximumLogit, -40.0f, 0.0f));
        probabilitySum += probabilities[action];
    }

    if (probabilitySum <= 0.0f)
        return 0.0f;

    for (action = 0; action < DK_NEURAL_ACTION_COUNT; ++action)
        probabilities[action] /= probabilitySum;

    targetProbability = 1.0f - labelSmoothing;
    for (action = 0; action < DK_NEURAL_ACTION_COUNT; ++action)
    {
        float target = action == targetAction - 1 ? targetProbability :
            labelSmoothing / (float)(DK_NEURAL_ACTION_COUNT - 1);
        outputDelta[action] = (probabilities[action] - target) * classWeight;
    }

    loss = -logf(DKNeuralTrainer_Clamp(probabilities[targetAction - 1], 0.0000001f, 1.0f));

    for (i = 0; i < DK_NEURAL_HIDDEN_2; ++i)
    {
        float delta = 0.0f;
        for (action = 0; action < DK_NEURAL_ACTION_COUNT; ++action)
            delta += model->hidden2_to_output[(size_t)action * DK_NEURAL_HIDDEN_2 + i] * outputDelta[action];
        model->hidden2_delta[i] = DKNeuralTrainer_Clamp(
            delta * DKNeuralTrainer_ReluDerivative(model->hidden2_activation[i]),
            -DK_NEURAL_TRAINER_DEFAULT_MAX_GRADIENT, DK_NEURAL_TRAINER_DEFAULT_MAX_GRADIENT);
    }

    for (i = 0; i < DK_NEURAL_HIDDEN_1; ++i)
    {
        float delta = 0.0f;
        for (j = 0; j < DK_NEURAL_HIDDEN_2; ++j)
            delta += model->hidden1_to_hidden2[(size_t)j * DK_NEURAL_HIDDEN_1 + i] * model->hidden2_delta[j];
        model->hidden1_delta[i] = DKNeuralTrainer_Clamp(
            delta * DKNeuralTrainer_ReluDerivative(model->hidden1_activation[i]),
            -DK_NEURAL_TRAINER_DEFAULT_MAX_GRADIENT, DK_NEURAL_TRAINER_DEFAULT_MAX_GRADIENT);
    }

    gradientScale = learningRate;
    for (action = 0; action < DK_NEURAL_ACTION_COUNT; ++action)
    {
        float error = outputDelta[action];
        float *weights = model->hidden2_to_output + (size_t)action * DK_NEURAL_HIDDEN_2;
        for (i = 0; i < DK_NEURAL_HIDDEN_2; ++i)
            weights[i] -= gradientScale * error * model->hidden2_activation[i];
        model->output_bias[action] -= gradientScale * error;
    }

    for (i = 0; i < DK_NEURAL_HIDDEN_2; ++i)
    {
        float *weights = model->hidden1_to_hidden2 + (size_t)i * DK_NEURAL_HIDDEN_1;
        float delta = model->hidden2_delta[i];
        for (j = 0; j < DK_NEURAL_HIDDEN_1; ++j)
            weights[j] -= gradientScale * delta * model->hidden1_activation[j];
        model->hidden2_bias[i] -= gradientScale * delta;
    }

    for (i = 0; i < DK_NEURAL_HIDDEN_1; ++i)
    {
        float *weights = model->input_to_hidden1 + (size_t)i * DK_NEURAL_INPUT_COUNT;
        float delta = model->hidden1_delta[i];
        for (j = 0; j < DK_NEURAL_INPUT_COUNT; ++j)
            weights[j] -= gradientScale * delta * input[j];
        model->hidden1_bias[i] -= gradientScale * delta;
    }

    return loss;
}

static int DKNeuralTrainer_TrainEpoch(const dk_neural_trainer_options_t *options,
    const dk_neural_dataset_info_t *info, dk_neural_trainer_model_t *model,
    int epoch, uint64_t *trainedThisEpoch, double *lossSum)
{
    FILE *file;
    dk_neural_training_file_header_t header;
    float input[DK_NEURAL_INPUT_COUNT];
    uint32_t action;
    uint64_t sampleIndex = 0;
    uint64_t processed = 0;
    uint64_t lastProgress = 0;
    float classWeights[DK_NEURAL_ACTION_COUNT];
    clock_t startClock = clock();

    file = fopen(options->input_path, "rb");
    if (!file)
        return 0;

    if (!DKNeuralTrainer_ReadTrainingHeader(file, &header))
    {
        fclose(file);
        return 0;
    }

    for (action = 1; action <= DK_NEURAL_ACTION_COUNT; ++action)
        classWeights[action - 1] = DKNeuralTrainer_ComputeClassWeight(info, action);

    *trainedThisEpoch = 0;
    *lossSum = 0.0;

    while ((options->max_samples == 0 || sampleIndex < options->max_samples) &&
           fread(input, sizeof(float), DK_NEURAL_INPUT_COUNT, file) == DK_NEURAL_INPUT_COUNT &&
           fread(&action, sizeof(action), 1, file) == 1)
    {
        if (action >= DK_NEURAL_ACTION_FOLLOW_OWNER && action <= DK_NEURAL_ACTION_HOLD)
        {
            if (options->validation_modulus == 0 ||
                (sampleIndex % options->validation_modulus) != 0)
            {
                *lossSum += (double)DKNeuralTrainer_TrainSample(
                    model, input, (int)action, options->learning_rate,
                    options->label_smoothing, classWeights[action - 1]);
                ++(*trainedThisEpoch);
                ++processed;
            }
        }

        ++sampleIndex;
        if (processed - lastProgress >= DK_NEURAL_TRAINER_PROGRESS_SAMPLES)
        {
            double seconds = (double)(clock() - startClock) / (double)CLOCKS_PER_SEC;
            double averageLoss = processed > 0 ? *lossSum / (double)processed : 0.0;
            printf("Epoch %d: %llu/%llu training samples, loss %.6f, %.1f samples/sec\n",
                epoch,
                (unsigned long long)processed,
                (unsigned long long)info->training_samples,
                averageLoss,
                seconds > 0.0 ? (double)processed / seconds : 0.0);
            fflush(stdout);
            lastProgress = processed;
        }
    }

    fclose(file);
    return *trainedThisEpoch > 0 ? 1 : 0;
}

static int DKNeuralTrainer_Evaluate(const dk_neural_trainer_options_t *options,
    dk_neural_trainer_model_t *model, const dk_neural_dataset_info_t *info,
    uint64_t *correct, uint64_t *samples, double *lossSum)
{
    FILE *file;
    dk_neural_training_file_header_t header;
    float input[DK_NEURAL_INPUT_COUNT];
    uint32_t action;
    uint64_t sampleIndex = 0;

    *correct = 0;
    *samples = 0;
    *lossSum = 0.0;

    if (options->validation_modulus == 0)
        return 1;

    file = fopen(options->input_path, "rb");
    if (!file)
        return 0;
    if (!DKNeuralTrainer_ReadTrainingHeader(file, &header))
    {
        fclose(file);
        return 0;
    }

    while ((options->max_samples == 0 || sampleIndex < options->max_samples) &&
           fread(input, sizeof(float), DK_NEURAL_INPUT_COUNT, file) == DK_NEURAL_INPUT_COUNT &&
           fread(&action, sizeof(action), 1, file) == 1)
    {
        if (action >= DK_NEURAL_ACTION_FOLLOW_OWNER && action <= DK_NEURAL_ACTION_HOLD &&
            (sampleIndex % options->validation_modulus) == 0)
        {
            int prediction = DKNeuralTrainer_Predict(model, input);
            if (prediction == (int)action)
                ++(*correct);
            ++(*samples);
        }
        ++sampleIndex;
    }

    fclose(file);
    (void)info;
    (void)lossSum;
    return 1;
}

static void DKNeuralTrainer_PrintUsage(const char *programName)
{
    printf("Daikatana tactical decision trainer (pure C)\n\n");
    printf("Usage:\n");
    printf("  %s --input <dataset.dkt> --output <model.bin> [options]\n\n", programName);
    printf("Options:\n");
    printf("  --epochs <n>              Training epochs (default: %d)\n", DK_NEURAL_TRAINER_DEFAULT_EPOCHS);
    printf("  --learning-rate <value>   SGD learning rate (default: %.8f)\n", DK_NEURAL_TRAINER_DEFAULT_LEARNING_RATE);
    printf("  --label-smoothing <value> Label smoothing (default: %.3f)\n", DK_NEURAL_TRAINER_DEFAULT_LABEL_SMOOTHING);
    printf("  --resume                  Continue from an existing .bin file\n");
    printf("  --max-samples <n>         Limit samples read per epoch (default: all)\n");
    printf("  --no-validation           Disable the deterministic 1-in-10 validation split\n");
    printf("\nThe trainer uses one CPU thread, no external libraries, and no Python runtime.\n");
}

static int DKNeuralTrainer_ParseArguments(int argc, char **argv, dk_neural_trainer_options_t *options)
{
    int index;

    memset(options, 0, sizeof(*options));
    options->epochs = DK_NEURAL_TRAINER_DEFAULT_EPOCHS;
    options->learning_rate = DK_NEURAL_TRAINER_DEFAULT_LEARNING_RATE;
    options->label_smoothing = DK_NEURAL_TRAINER_DEFAULT_LABEL_SMOOTHING;
    options->validation_modulus = DK_NEURAL_TRAINER_VALIDATION_MODULUS;

    for (index = 1; index < argc; ++index)
    {
        if (strcmp(argv[index], "--input") == 0 && index + 1 < argc)
            options->input_path = argv[++index];
        else if (strcmp(argv[index], "--output") == 0 && index + 1 < argc)
            options->output_path = argv[++index];
        else if (strcmp(argv[index], "--epochs") == 0 && index + 1 < argc)
            options->epochs = atoi(argv[++index]);
        else if (strcmp(argv[index], "--learning-rate") == 0 && index + 1 < argc)
            options->learning_rate = (float)atof(argv[++index]);
        else if (strcmp(argv[index], "--label-smoothing") == 0 && index + 1 < argc)
            options->label_smoothing = (float)atof(argv[++index]);
        else if (strcmp(argv[index], "--resume") == 0)
            options->resume = 1;
        else if (strcmp(argv[index], "--max-samples") == 0 && index + 1 < argc)
            options->max_samples = (uint64_t)strtoull(argv[++index], NULL, 10);
        else if (strcmp(argv[index], "--no-validation") == 0)
            options->validation_modulus = 0;
        else if (strcmp(argv[index], "--help") == 0 || strcmp(argv[index], "-h") == 0)
            return 0;
        else
        {
            fprintf(stderr, "Unknown or incomplete option: %s\n", argv[index]);
            return -1;
        }
    }

    if (!options->input_path || !options->output_path || options->epochs < 1 ||
        options->learning_rate <= 0.0f || options->label_smoothing < 0.0f || options->label_smoothing >= 1.0f)
        return -1;

    return 1;
}

int main(int argc, char **argv)
{
    dk_neural_trainer_options_t options;
    dk_neural_dataset_info_t info;
    dk_neural_trainer_model_t model;
    int parseResult;
    int epoch;

    parseResult = DKNeuralTrainer_ParseArguments(argc, argv, &options);
    if (parseResult <= 0)
    {
        DKNeuralTrainer_PrintUsage(argv[0]);
        return parseResult == 0 ? 0 : 2;
    }

    if (DK_NEURAL_PARAMETER_COUNT !=
        ((size_t)DK_NEURAL_INPUT_COUNT * DK_NEURAL_HIDDEN_1 +
         DK_NEURAL_HIDDEN_1 +
         (size_t)DK_NEURAL_HIDDEN_1 * DK_NEURAL_HIDDEN_2 +
         DK_NEURAL_HIDDEN_2 +
         (size_t)DK_NEURAL_HIDDEN_2 * DK_NEURAL_ACTION_COUNT +
         DK_NEURAL_ACTION_COUNT))
    {
        fprintf(stderr, "5M model topology mismatch.\n");
        return 3;
    }

    if (!DKNeuralTrainer_ScanDataset(&options, &info))
        return 4;

    printf("Daikatana 5M tactical trainer\n");
    printf("Input:  %s\n", options.input_path);
    printf("Output: %s\n", options.output_path);
    printf("Samples: %llu (train %llu / validation %llu)\n",
        (unsigned long long)info.total_samples,
        (unsigned long long)info.training_samples,
        (unsigned long long)info.validation_samples);
    printf("Topology: %d -> %d -> %d -> %d = %lu parameters\n",
        DK_NEURAL_INPUT_COUNT, DK_NEURAL_HIDDEN_1, DK_NEURAL_HIDDEN_2,
        DK_NEURAL_ACTION_COUNT, DK_NEURAL_PARAMETER_COUNT);
    printf("Learning rate: %.8f, label smoothing: %.3f, epochs: %d\n",
        options.learning_rate, options.label_smoothing, options.epochs);
    printf("CPU mode: single-threaded / standard C runtime only\n");

    if (!DKNeuralTrainer_InitializeModel(&model))
    {
        fprintf(stderr, "Unable to allocate the 5M model (about 20 MB of weights plus scratch buffers).\n");
        return 5;
    }

    if (options.resume)
    {
        if (DKNeuralTrainer_LoadModel(options.output_path, &model))
            printf("Resuming from existing trained weights.\n");
        else
            printf("Existing model was not usable; starting from deterministic bootstrap weights.\n");
    }
    else
    {
        printf("Starting from deterministic bootstrap weights.\n");
    }

    for (epoch = 1; epoch <= options.epochs; ++epoch)
    {
        uint64_t trainedThisEpoch;
        double lossSum;
        uint64_t correct;
        uint64_t validationSamples;
        double validationLoss;

        if (!DKNeuralTrainer_TrainEpoch(&options, &info, &model, epoch, &trainedThisEpoch, &lossSum))
        {
            fprintf(stderr, "Training failed during epoch %d.\n", epoch);
            DKNeuralTrainer_FreeModel(&model);
            return 6;
        }

        validationLoss = 0.0;
        if (!DKNeuralTrainer_Evaluate(&options, &model, &info, &correct, &validationSamples, &validationLoss))
        {
            fprintf(stderr, "Validation failed during epoch %d.\n", epoch);
            DKNeuralTrainer_FreeModel(&model);
            return 7;
        }

        printf("Epoch %d complete: trained=%llu loss=%.6f",
            epoch,
            (unsigned long long)trainedThisEpoch,
            trainedThisEpoch > 0 ? lossSum / (double)trainedThisEpoch : 0.0);
        if (validationSamples > 0)
        {
            printf(", validation=%llu/%llu (%.2f%%)",
                (unsigned long long)correct,
                (unsigned long long)validationSamples,
                100.0 * (double)correct / (double)validationSamples);
        }
        printf("\n");

        if ((epoch % DK_NEURAL_TRAINER_CHECKPOINT_EPOCHS) == 0 &&
            !DKNeuralTrainer_SaveModel(options.output_path, &model))
        {
            fprintf(stderr, "Unable to save checkpoint '%s'.\n", options.output_path);
            DKNeuralTrainer_FreeModel(&model);
            return 8;
        }
    }

    printf("Training complete. Model saved to %s\n", options.output_path);
    DKNeuralTrainer_FreeModel(&model);
    return 0;
}
