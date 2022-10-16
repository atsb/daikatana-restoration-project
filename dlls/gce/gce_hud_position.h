//#include "p_user.h"

#include "gce_main.h"
#include "gce_hud.h"
#include "gce_fields.h"

typedef enum {
    FP_SAVE,
    FP_ABORT,

    FP_NUM_FIELDS,
} fields_position;

int num_fields_entities = FP_NUM_FIELDS;


field_description field_desc_position[] = {
    {FT_BUTTON, 10, "Save"},
    {FT_BUTTON, 10, "Abort"},
};

