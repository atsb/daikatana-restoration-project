#include "p_user.h"

#include "dk_buffer.h"

#include "dk_cin_types.h"
#include "dk_gce_interface.h"

#include "gce_main.h"
#include "gce_hud.h"
#include "gce_fields.h"
#include "gce_position.h"

typedef enum {
    FP_SAVE,
    FP_ABORT,

    FP_NUM_FIELDS,
} fields_position;

int num_fields_position = FP_NUM_FIELDS;

field_description field_desc_position[] = {
    {FT_BUTTON, 10, "Save"},
    {FT_BUTTON, 10, "Abort"},
};

bool field_mask_position(int field) {
    if (false==PositionGetMouseEnabled())
        return false;

    if (field == FP_SAVE) 
        return true;

    if (field == FP_ABORT) 
        return true;
    
    return false;
}

void edit_start_position(int field) {
    if (false==PositionGetMouseEnabled())
        return;
    
    if (field == FP_SAVE)
        FieldEdit(field_desc_position[field]);
    
    if (field == FP_ABORT)
        FieldEdit(field_desc_position[field]);
}

void edit_end_position(int field) {
    buffer128 buffer;

    if (false==PositionGetMouseEnabled())
        return;
    
    if (field == FP_SAVE) {
        server->CBuf_AddText("gce_position_save\n");
    }
    else if (field == FP_ABORT) {
        server->CBuf_AddText("gce_position_abort\n");
    }
}


void HUD_Position() {
    //draw the prompt.
    cin->DrawString(50, 100, PositionPrompt());

    //draw our buttons.
    if (true==PositionGetMouseEnabled())
    {
        FieldDraw(field_desc_position[FP_SAVE], 50, 120);
        FieldDraw(field_desc_position[FP_ABORT], 50, 140);
    }
}


