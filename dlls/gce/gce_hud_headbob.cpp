#include "p_user.h"

#include "dk_cin_types.h"
#include "dk_point.h"
#include "dk_buffer.h"
#include "dk_array.h"

#include "dk_gce_interface.h"
#include "dk_gce_spline.h"
#include "dk_gce_headbob.h"

#include "gce_main.h"
#include "gce_hud.h"
#include "gce_fields.h"
#include "gce_position.h"
#include "gce_headscript.h"

typedef enum {
    FH_START,
    FH_ABORT,

    FH_NUM_FIELDS,
} fields_headscript;

int num_fields_headscript = FH_NUM_FIELDS;


field_description field_desc_headscript[] = {
    {FT_BUTTON, 10, "Start"},
    {FT_BUTTON, 10, "Abort"},
};

bool field_mask_headscript(int field) {
    if (HeadscriptRecording() == false) {
        if (field == FH_START) return true;
        if (field == FH_ABORT) return true;
    }
    return false;
}

void edit_start_headscript(int field) {
    if (field == FH_START) FieldEdit(field_desc_headscript[field]);
    if (field == FH_ABORT) FieldEdit(field_desc_headscript[field]);
}

void edit_end_headscript(int field) {
    buffer128 buffer;

    if (field == FH_START) {
        server->CBuf_AddText("gce_headscript_start\n");
    }
    if (field == FH_ABORT) {
        server->CBuf_AddText("gce_headscript_abort\n");
    }
}


void HUD_HeadScript() {
    if (cin == NULL) return;

    //check if we are already recording.
    if (HeadscriptRecording() == true) {
        //tell the editor that time has elapsed.
        HeadscriptFrame();

        //draw a message to tell the user how to end recording.
        cin->DrawString(100, 100, "Click the left mouse button to end recording.");
        cin->DrawString(100, 120, "Press any key to abort the recording.");
    }
    else {
        //print some instructions.
        cin->DrawString(50, 100, "Press Start to begin recording.");

        //draw our buttons.
        FieldDraw(field_desc_headscript[FH_START], 50, 120);
        FieldDraw(field_desc_headscript[FH_ABORT], 50, 140);
    }
}

void MouseDownHeadScript(int x, int y) {
    if (HeadscriptRecording() == false) return;

    //save the recorded script.
    server->CBuf_AddText("gce_headscript_save\n");
}

int HUD_HeadScriptKeyEvent(int key) {
    if (HeadscriptRecording() == false) return 0;

    //abort the recording.
    server->CBuf_AddText("gce_headscript_abort\n");

    return 1;
}