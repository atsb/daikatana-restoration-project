#ifndef DK_NEURAL_TRAINING_FORMAT_H
#define DK_NEURAL_TRAINING_FORMAT_H

#include <stdint.h>

#define DK_NEURAL_TRAINING_MAGIC      ((uint32_t)0x444B5444U)
#define DK_NEURAL_TRAINING_VERSION    1U
#define DK_NEURAL_TRAINING_RECORD_SIZE (DK_NEURAL_INPUT_COUNT * (uint32_t)sizeof(float) + (uint32_t)sizeof(uint32_t))

typedef struct
{
    uint32_t magic;
    uint32_t version;
    uint32_t input_count;
    uint32_t action_count;
    uint32_t record_size;
    uint32_t reserved;
    uint64_t sample_count;
} dk_neural_training_file_header_t;

#endif

#ifdef __cplusplus
static_assert(sizeof(dk_neural_training_file_header_t) == 32,
    "Daikatana neural training file header size changed");
#endif
