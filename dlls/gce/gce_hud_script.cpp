#include "p_user.h"
#include "ref.h"

#include "dk_cin_types.h"
#include "dk_gce_interface.h"
#include "dk_point.h"

#include "dk_gce_script.h"
#include "dk_gce_spline.h"

#include "gce_main.h"
#include "gce_fields.h"
#include "gce_script.h"
#include "gce_hud.h"

typedef enum {
    FS_SAVE,
    FS_LOAD,
    FS_NAME,
    FS_ADD_SHOT,
    FS_PLAY_SHOT,
	FS_PLAY_ALL_SHOTS,
	FS_PLAY_FROM_CURRENT,
	FS_CANCEL_SHOT,
    FS_INSERT_SHOT,
    FS_DELETE_SHOT,

    FS_SCROLL_BEGIN,
    FS_SCROLL_MID,
    FS_SCROLL_END,

    FS_SCROLL_LEFT,
    FS_SCROLL_RIGHT,

	FS_SHOT_SKY,

    FS_SHOT_EXTRA_START,
    FS_SHOT_EXTRA_END,

    FS_SHOT_START_FOV_RADIO,
    FS_SHOT_START_FOV,

    FS_SHOT_CAMERA_TARGET,
    FS_SHOT_ENTITY_TARGET,

    FS_SHOT_END_FLAG,
    FS_SHOT_END_ENTITY,

    FS_NUM_FIELDS,
} fields_script;

int num_fields_script = FS_NUM_FIELDS;

field_description field_desc_script[] = {
    {FT_BUTTON, 12, "Save Script"},
    {FT_STRING, 32, "Load Script: "},
    {FT_STRING, 32, "Current Script: "},
    {FT_BUTTON, 10, "Add"},
    {FT_BUTTON, 8, "Play"},
    {FT_BUTTON, 10, "Play All"},
    {FT_BUTTON, 10, "Play From"},
    {FT_BUTTON, 8, "Cancel"},
    {FT_BUTTON, 4, "Ins"},
    {FT_BUTTON, 4, "Del"},

    {FT_BUTTON, 4, "<<"},
    {FT_BUTTON, 4, "[]"},
    {FT_BUTTON, 4, ">>"},

    {FT_BUTTON, 2, "<"},
    {FT_BUTTON, 2, ">"},

    {FT_INT, 6, "Shot sky: "},
	
	{FT_FLOAT, 6, "Shot start pause: "},
    {FT_FLOAT, 6, "Shot end pause: "},

    {FT_RADIO, 0, "Have Start FOV"},
    {FT_FLOAT, 6, "Start FOV: "},

    {FT_LIST, 8, "Camera Target: "},
    {FT_STRING, 16, "Target Entity: "},

    {FT_LIST, 8, "Shot End Type: "},
    {FT_STRING, 16, "Wait for Entity: "},
};

extern char *choices_entity_default[];

//list descriptors for the script shot camera target field.
list_field_description shot_camera_target = {
    choices_entity_default, 2
};

//list descriptor sfor the script shot end flag.
list_field_description shot_end_flag = {
    choices_entity_default, 2
};


typedef struct {
    //the extra time for the shot.
    float extra_start, extra_end;
    //the time for the actual sequence.
    float seq_time;
    //the total time for the whole shot.
    float total;
    //info about how we are going to draw the shot in the script viewer.
    int left, right;
} shot_info;

static shot_info script_shots[200];
static int script_first_shot = 0;
static int script_last_shot = 0;
static int script_num_shots = 0;

bool field_mask_script(int field) {
    //get the current script.
    const CCinematicScript *script = ScriptCurrent();
    if (script == NULL) return false;

    if (field == FS_ADD_SHOT) return true;
    if (field == FS_NAME) return true;

    if (field == FS_LOAD) return true;

    if (field == FS_SAVE) {
        //check if the name of our script is valid.
        const char *name = script->Name();
        if (name[0] == '\0' || name[0] == ' ') {
            //invalid name.
            return false;
        }

        return true;
    }

	// SCG[8/26/99]: check play all shots button
	if( script_num_shots )
	{
		if (field == FS_PLAY_ALL_SHOTS) return true;
	}

	// SCG[11/7/99]: check play from current
	if( script_num_shots )
	{
		if (field == FS_PLAY_FROM_CURRENT) return true;
	}

    if (script_num_shots > 0 )
	{
		if (field == FS_SCROLL_BEGIN) return true;
		if (field == FS_SCROLL_MID) return true;
		if (field == FS_SCROLL_END) return true;
	}

    //check the scroll buttons.
    if (script_first_shot > 0) {
        if (field == FS_SCROLL_LEFT) return true;
    }
    if (script_last_shot < script_num_shots - 1) {
        if (field == FS_SCROLL_RIGHT) return true;
    }

    //get the shot that is selected.
    int selected_shot = ScriptCurrentShot();

    if (selected_shot != -1) {
        //get the selected shot.
        const CScriptShot *shot = script->Shot(selected_shot);
        if (shot != NULL) {
   			if (field == FS_SHOT_SKY) 
				return true;

			if (field == FS_SHOT_EXTRA_START) return true;
            if (field == FS_SHOT_EXTRA_END) return true;

            if (field == FS_SHOT_START_FOV_RADIO) return true;

            if (shot->HaveStartFOV() == true) {
                if (field == FS_SHOT_START_FOV) return true;
            }

            if (field == FS_PLAY_SHOT) return true;
            if (field == FS_CANCEL_SHOT) return true;
            if (field == FS_INSERT_SHOT) return true;
            if (field == FS_DELETE_SHOT) return true;

            if (field == FS_SHOT_CAMERA_TARGET) return true;

            if (shot->CameraTarget() == CT_ENTITY) {
                if (field == FS_SHOT_ENTITY_TARGET) return true;
            }

            if (field == FS_SHOT_END_FLAG) return true;

            if (shot->EndFlag() == SE_ENTITY) {
                if (field == FS_SHOT_END_ENTITY) return true;
            }
        }
    }

    return false;
}

void edit_start_script(int field) {
    //get the current script.
    const CCinematicScript *script = ScriptCurrent();
    if (script == NULL) return;

    //get the selected shot.
    const CScriptShot *shot = script->Shot(ScriptCurrentShot());

    if (field == FS_ADD_SHOT) {
        FieldEdit(field_desc_script[field]);
    }
    else if (field == FS_NAME) {
        FieldEdit(field_desc_script[field], script->Name());
    }
    else if (field == FS_LOAD) {
        FieldEdit(field_desc_script[field], "");
    }
    else if (field == FS_SAVE) {
        FieldEdit(field_desc_script[field]);
    }
    else if (field == FS_SCROLL_BEGIN) {
        FieldEdit(field_desc_script[field]);
    }
    else if (field == FS_SCROLL_MID) {
        FieldEdit(field_desc_script[field]);
    }
    else if (field == FS_SCROLL_END) {
        FieldEdit(field_desc_script[field]);
    }
    else if (field == FS_SCROLL_LEFT) {
        FieldEdit(field_desc_script[field]);
    }
    else if (field == FS_SCROLL_RIGHT) {
        FieldEdit(field_desc_script[field]);
    }
    else if (field == FS_PLAY_ALL_SHOTS) {
        FieldEdit(field_desc_script[field]);
    }
    else if (field == FS_PLAY_FROM_CURRENT) {
        FieldEdit(field_desc_script[field]);
    }
    else if (shot != NULL) {
        if (field == FS_SHOT_SKY) {
            FieldEdit(field_desc_script[field], shot->Sky());
        }
        else if (field == FS_SHOT_EXTRA_START) {
            FieldEdit(field_desc_script[field], shot->ExtraTimeStart());
        }
        else if (field == FS_SHOT_EXTRA_END) {
            FieldEdit(field_desc_script[field], shot->ExtraTimeEnd());
        }
        else if (field == FS_SHOT_START_FOV_RADIO) {
            FieldEdit(field_desc_script[field], shot->HaveStartFOV());
        }
        else if (field == FS_SHOT_START_FOV) {
            FieldEdit(field_desc_script[field], shot->StartFOV());
        }
        else if (field == FS_PLAY_SHOT) {
            FieldEdit(field_desc_script[field]);
        }
        else if (field == FS_CANCEL_SHOT) {
            FieldEdit(field_desc_script[field]);
        }
        else if (field == FS_INSERT_SHOT) {
            FieldEdit(field_desc_script[field]);
        }
        else if (field == FS_DELETE_SHOT) {
            FieldEdit(field_desc_script[field]);
        }
        else if (field == FS_SHOT_CAMERA_TARGET) {
            shot_camera_target.selected = shot->CameraTarget();
            FieldEdit(field_desc_script[field], shot_camera_target);
        }
        else if (field == FS_SHOT_ENTITY_TARGET) {
            if (shot->CameraTarget() == CT_ENTITY) {
                FieldEdit(field_desc_script[field], shot->EntityCameraTarget());
            }
        }
        else if (field == FS_SHOT_END_FLAG) {
            shot_end_flag.selected = shot->EndFlag();
            FieldEdit(field_desc_script[field], shot_end_flag);
        }
        else if (field == FS_SHOT_END_ENTITY) {
            if (shot->EndFlag() == SE_ENTITY) {
                FieldEdit(field_desc_script[field], shot->EntityShotEnd());
            }
        }
    }
}

void edit_end_script(int field) {
    buffer128 buffer;

    //get the current script.
    const CCinematicScript *script = ScriptCurrent();
    if (script == NULL) return;

    //get the selected shot.
    const CScriptShot *shot = script->Shot(ScriptCurrentShot());

    //make the command string.
    if (field == FS_ADD_SHOT) {
        buffer.Set("gce_script_add_shot\n");
    }
    else if (field == FS_NAME) {
        buffer.Set("gce_script_name \"%s\"\n", FieldEditText());
    }
    else if (field == FS_LOAD) {
        buffer.Set("gce_script_load \"%s\"\n", FieldEditText());
    }
    else if (field == FS_PLAY_SHOT) {
        buffer.Set("gce_script_play_shot\n");
    }
    else if (field == FS_CANCEL_SHOT) {
        buffer.Set("gce_script_cancel_shot\n");
    }
	else if( field == FS_PLAY_ALL_SHOTS )
	{
        buffer.Set("gce_script_play_all_shots\n");
	}
	else if( field == FS_PLAY_FROM_CURRENT )
	{
        buffer.Set("gce_script_play_from_current_shot %d\n", ScriptCurrentShot());
	}
    else if (field == FS_INSERT_SHOT) {
        if (shot != NULL) {
            buffer.Set("gce_script_shot_insert %d\n", ScriptCurrentShot());
        }
    }
    else if (field == FS_DELETE_SHOT) {
        buffer.Set("gce_script_shot_delete_selected\n");
    }
    else if (field == FS_SAVE) {
        buffer.Set("gce_script_save\n");
    }
    else if (field == FS_SCROLL_BEGIN) {
        script_first_shot = 0;
    }
    else if (field == FS_SCROLL_MID) {
        script_first_shot = ( int ) ( script->NumShots() ) / 2;
    }
    else if (field == FS_SCROLL_END) {
        script_first_shot = script->NumShots() - 1;
    }
    else if (field == FS_SCROLL_LEFT) {
        script_first_shot--;
    }
    else if (field == FS_SCROLL_RIGHT) {
        script_first_shot++;
    }
    else if (field == FS_SHOT_SKY) {
        //get the value that was entered.
        int sky = 1;
        sscanf(FieldEditText(), "%d", &sky);

        //check the value
        if (sky < 1) return;

        //change the value.
        buffer.Set("gce_script_shot_sky %d\n", sky);
    }
    else if (field == FS_SHOT_EXTRA_START || field == FS_SHOT_EXTRA_END) {
        //get the value that was entered.
        float time = 0.0f;
        sscanf(FieldEditText(), "%f", &time);

        //check the value
        if (time < 0.0f) return;

        //change the value.
        if (field == FS_SHOT_EXTRA_START) {
            buffer.Set("gce_script_shot_time_start %.2f\n", time);
        }
        else {
            buffer.Set("gce_script_shot_time_end %.2f\n", time);
        }
    }
    else if (field == FS_SHOT_START_FOV_RADIO) {
        buffer.Set("gce_script_shot_have_start_fov_toggle\n");
    }
    else if (field == FS_SHOT_START_FOV) {
        //get the value that was entered.
        float fov = 0.0f;
        sscanf(FieldEditText(), "%f", &fov);

        //change the value.
        buffer.Set("gce_script_shot_start_fov %.2f\n", fov);
    }
    else if (field == FS_SHOT_CAMERA_TARGET) {
        buffer.Set("gce_script_shot_camera_target %d\n", FieldListChoiceSelected());
    }
    else if (field == FS_SHOT_ENTITY_TARGET) {
        buffer.Set("gce_script_shot_camera_target_entity \"%s\"\n", FieldEditText());
    }
    else if (field == FS_SHOT_END_FLAG) {
        buffer.Set("gce_script_shot_end_flag %d\n", FieldListChoiceSelected());
    }
    else if (field == FS_SHOT_END_ENTITY) {
        buffer.Set("gce_script_shot_end_entity \"%s\"\n", FieldEditText());
    }

    //send the command.
    if (buffer) server->CBuf_AddText(buffer);
}

//info for all the shots we display 
static int script_shot_num_top;
static int script_camera_row_top;
static int script_camera_row_bottom;
static int script_sounds_row_top;
static int script_sounds_row_bottom;
static int script_entities_row_top;
static int script_entities_row_bottom;

#define ROW_HEIGHT 15
#define SCRIPT_LEFT 10
#define SCRIPT_COL0 110

void HUD_Script() {
    buffer128 buffer;
    int y = 100;

    //get the current script.
    const CCinematicScript *script = ScriptCurrent();
    if (script == NULL) return;

    //draw the save script button.
    FieldDraw(field_desc_script[FS_SAVE], 20, 10);

    //draw the load script field.
//    FieldDraw(field_desc_script[FS_LOAD], 220, 70, "");
    FieldDraw(field_desc_script[FS_LOAD], 130, 10, "");

    //draw the script name.
    FieldDraw(field_desc_script[FS_NAME], 20, 30, script->Name()); y += 10;

    //draw the add sequence button.
    FieldDraw(field_desc_script[FS_ADD_SHOT], 20, 45);

    //draw the play all button.
    FieldDraw(field_desc_script[FS_PLAY_ALL_SHOTS], 160, 45 );

    //draw the play from button.
    FieldDraw(field_desc_script[FS_PLAY_FROM_CURRENT], 255, 45 );

    //draw the play button.
    FieldDraw(field_desc_script[FS_PLAY_SHOT], 348, 45);

    //draw the cancel button.
    FieldDraw(field_desc_script[FS_CANCEL_SHOT], 427, 45);

    //draw the insert button.
    FieldDraw(field_desc_script[FS_INSERT_SHOT], 505, 45); y += ROW_HEIGHT;

    //draw the delete button.
    FieldDraw(field_desc_script[FS_DELETE_SHOT], 550, 45); y += ROW_HEIGHT;

    //get the number of shots out of the 
    script_num_shots = script->NumShots();
    buffer.Set("Num shots: %d", script_num_shots);
    cin->DrawString(SCRIPT_LEFT + 49, 70, buffer); y += 10;
 
    //if we have no shots, we do not draw our shot display grid.
    if (script_num_shots < 1) return;

    //get info about all the shots.
    bound_max(script_num_shots, 200);
    for (int i = 0; i < script_num_shots; i++) {
        //get the shot.
        const CScriptShot *shot = script->Shot(i);
        if (shot == NULL) {
            script_shots[i].extra_start = 0.0f;
            script_shots[i].extra_end = 0.0f;
            script_shots[i].seq_time = 0.0f;
            continue;
        }

        script_shots[i].extra_start = shot->ExtraTimeStart();
        script_shots[i].extra_end = shot->ExtraTimeEnd();

        //get the sequence itself.
        const CSplineSequence *sequence = shot->Sequence();
        if (sequence == NULL) {
            script_shots[i].seq_time = 0.0f;
            continue;
        }

        script_shots[i].seq_time = sequence->TotalTime();
    }

    //verify we have a valid possible value for the first shot.
    if (script_first_shot >= script_num_shots || script_first_shot < 0) {
        //just reset it.
        script_first_shot = 0;
    }

    //compute the times for each shot.
    for (i = 0; i < script_num_shots; i++) {
        script_shots[i].total = script_shots[i].extra_start + script_shots[i].extra_end + script_shots[i].seq_time;
    }
    
    script_shots[script_first_shot].left = SCRIPT_COL0;
    script_shots[script_first_shot].right = script_shots[script_first_shot].left + script_shots[script_first_shot].total * 10;
    bound_min(script_shots[script_first_shot].right, script_shots[script_first_shot].left + 20)

    //compute the total amount of time is taken up by our script, and the last shot we will display.
    for (i = script_first_shot + 1; i < script_num_shots; i++) {
        //a cvar that controls the scale of the display of the shots
        static cvar_t *shot_display_scale = server->cvar("gce_shot_display_scale", "1", CVAR_ARCHIVE);

        //do error checking on the value.
        float display_scale;
        if (shot_display_scale != NULL) {
            display_scale = shot_display_scale->value;
        }
        else {
            display_scale = 1.0f;
        }
        bound_min(display_scale, 0.1f);
        bound_max(display_scale, 2.0f);

        script_shots[i].left = script_shots[i - 1].right;
        script_shots[i].right = script_shots[i].left + script_shots[i].total * 10 * display_scale;

        bound_min(script_shots[i].right, script_shots[i].left + 35);

        //check if all the shots are too long to display.
        if (script_shots[i].right > 620) {
            break;
        }
    }

    script_last_shot = i - 1;

	// SCG[10/18/99]: Draw the begin / end buttons
	FieldDraw(field_desc_script[FS_SCROLL_BEGIN], SCRIPT_COL0, y + 5);

    //draw our scroll buttons to see other shots off the edge.
    if (script_first_shot > 0) {
        //draw the left scroll.
        FieldDraw(field_desc_script[FS_SCROLL_LEFT], SCRIPT_COL0 + 45, y + 5);
    }

	FieldDraw(field_desc_script[FS_SCROLL_MID], SCRIPT_COL0 + 75, y + 5);

    if (script_last_shot < script_num_shots - 1) {
        FieldDraw(field_desc_script[FS_SCROLL_RIGHT], SCRIPT_COL0 + 120, y + 5);
    }

	FieldDraw(field_desc_script[FS_SCROLL_END], SCRIPT_COL0 + 150, y + 5);

//    y += 20;
    y += 248;

    //compute the y coordinates for the different rows.
    script_shot_num_top = y;
    y += ROW_HEIGHT;
    script_camera_row_top = y;
    script_camera_row_bottom = y + ROW_HEIGHT;
    script_sounds_row_top = script_camera_row_bottom;
    script_sounds_row_bottom = script_camera_row_bottom + ROW_HEIGHT;
    script_entities_row_top = script_sounds_row_bottom;
    script_entities_row_bottom = script_entities_row_top + ROW_HEIGHT;

    int script_rows_bottom = script_entities_row_bottom;

    //get the selected shot number
    int selected_shot = ScriptCurrentShot();

    //put a background behind our column labels
    HUDDrawBox(script_shots[script_first_shot].left, script_shot_num_top, script_shots[script_last_shot].right, script_camera_row_top, CVector(0.0, 0.0, 0.0));

    //put a background behind our row labels.
    HUDDrawBox(SCRIPT_LEFT, script_camera_row_top, script_shots[script_first_shot].left, script_rows_bottom, CVector(0.0, 0.0, 0.0));

    //if we have a shot selected and it is visible, put a hilight where the shot number will go.
    if (selected_shot >= script_first_shot && selected_shot <= script_last_shot) {
        HUDDrawBox(script_shots[selected_shot].left, script_shot_num_top, script_shots[selected_shot].right, script_camera_row_top, CVector(1.0, 1.0, 1.0));
    }

    //draw the numbers for the shots.
    for (i = script_first_shot; i <= script_last_shot; i++) {
        buffer.Set("%d", i);

        //compute the x coordinate for the text.
        int x = script_shots[i].left + 0.5f * (script_shots[i].right - script_shots[i].left - strlen(buffer) * 8 );
        cin->DrawString(x, 4 + script_shot_num_top, buffer);
    }

    //draw the row labels.
    cin->DrawString(20, 4 + script_camera_row_top, "Camera");
    cin->DrawString(20, 4 + script_sounds_row_top, "Sounds");
    cin->DrawString(20, 4 + script_entities_row_top, "Entities");

    //draw our grid to show the shots in the script.
    //draw vertical lines on left of each shot column.
    for (i = script_first_shot; i <= script_last_shot; i++) {
        HUDDrawBox(script_shots[i].left, script_shot_num_top, script_shots[i].left, script_rows_bottom, CVector(0.5, 0.5, 0.5));
    }
    //draw vertical line on right of last column.
    HUDDrawBox(script_shots[script_last_shot].right, script_shot_num_top, script_shots[script_last_shot].right, script_rows_bottom, CVector(0.5, 0.5, 0.5));
    //draw vertical line on left of row labels.
    HUDDrawBox(SCRIPT_LEFT, script_camera_row_top, SCRIPT_LEFT, script_rows_bottom, CVector(0.5, 0.5, 0.5));

    //draw the horizontal lines.
    HUDDrawBox(SCRIPT_COL0, script_shot_num_top, script_shots[script_last_shot].right, script_shot_num_top, CVector(0.5, 0.5, 0.5));
    HUDDrawBox(SCRIPT_LEFT, script_camera_row_top, script_shots[script_last_shot].right, script_camera_row_top, CVector(0.5, 0.5, 0.5));
    HUDDrawBox(SCRIPT_LEFT, script_sounds_row_top, script_shots[script_last_shot].right, script_sounds_row_top, CVector(0.5, 0.5, 0.5));
    HUDDrawBox(SCRIPT_LEFT, script_entities_row_top, script_shots[script_last_shot].right, script_entities_row_top, CVector(0.5, 0.5, 0.5));
    HUDDrawBox(SCRIPT_LEFT, script_rows_bottom, script_shots[script_last_shot].right, script_rows_bottom, CVector(0.5, 0.5, 0.5));

    y = 300;

    //check if we have a shot selected.
    if (selected_shot != -1) {
        //get the selected shot.
        const CScriptShot *shot = script->Shot(selected_shot);
        if (shot != NULL) {
			// display the sky number
            FieldDraw(field_desc_script[FS_SHOT_SKY], SCRIPT_LEFT + 67, y, shot->Sky()); y += 10;

            //display the start/end extra time fields.
			FieldDraw(field_desc_script[FS_SHOT_EXTRA_START], SCRIPT_LEFT, y, shot->ExtraTimeStart()); y += 10;
            FieldDraw(field_desc_script[FS_SHOT_EXTRA_END], SCRIPT_LEFT + 18, y, shot->ExtraTimeEnd()); y += 10;

            //display the total time for the whole shot.
            buffer.Set("Shot total time: %.2f", shot->TotalTime());
            cin->DrawString(SCRIPT_LEFT, 80, buffer); 

            //draw the camera target type.
            shot_camera_target.selected = shot->CameraTarget();
            FieldDraw(field_desc_script[FS_SHOT_CAMERA_TARGET], SCRIPT_LEFT + 22, y, shot_camera_target); y += 10;

            if (shot->CameraTarget() == CT_ENTITY) {
                FieldDraw(field_desc_script[FS_SHOT_ENTITY_TARGET], SCRIPT_LEFT + 22, y, shot->EntityCameraTarget()); y += 10;
            }

            //draw shot end flag type.
            shot_end_flag.selected = shot->EndFlag();
            FieldDraw(field_desc_script[FS_SHOT_END_FLAG], SCRIPT_LEFT + 27, y, shot_end_flag); y += 10;

            if (shot->EndFlag() == SE_ENTITY) {
                FieldDraw(field_desc_script[FS_SHOT_END_ENTITY], SCRIPT_LEFT + 9, y, shot->EntityShotEnd()); y += 10;
            }

			y += 6;

            //draw shot FOV fields.
            FieldDraw(field_desc_script[FS_SHOT_START_FOV_RADIO], SCRIPT_LEFT, y, shot->HaveStartFOV());
            if (shot->HaveStartFOV() == true) {
                FieldDraw(field_desc_script[FS_SHOT_START_FOV], SCRIPT_LEFT + 180, y, shot->StartFOV());
            }

        }
    }
}

void MouseDownScript(int x, int y) {
    buffer64 buffer;

    //check if we clicked on the row labels and we have a currently selected shot..
    if (x >= SCRIPT_LEFT && x < SCRIPT_COL0 && ScriptCurrentShot() != -1) {
        //check which row we clicked on.
        if (y >= script_camera_row_top && y <= script_camera_row_bottom) {
            //we clicked on the camera.
            buffer.Set("gce_script_edit_camera %d\n", ScriptCurrentShot());
            server->CBuf_AddText(buffer);
        }
        else if (y >= script_sounds_row_top && y <= script_sounds_row_bottom) {
            //we clicked on the camera.
            buffer.Set("gce_script_edit_sounds %d\n", ScriptCurrentShot());
            server->CBuf_AddText(buffer);
        }
        else if (y >= script_entities_row_top && y <= script_entities_row_bottom) {
            //we clicked on the entities.
            buffer.Set("gce_script_edit_entities %d\n", ScriptCurrentShot());
            server->CBuf_AddText(buffer);
        }
    }

    //check which column, if any was clicked.
    for (int i = script_first_shot; i <= script_last_shot; i++) {
        if (x >= script_shots[i].left && x <= script_shots[i].right) {
            //we clicked this column, check what row we are on.
            if (y >= script_shot_num_top && y < script_camera_row_top) {
                //we clicked on the shot number.
                buffer.Set("gce_script_select_shot %d\n", i);
                server->CBuf_AddText(buffer);
            }
            else if (y >= script_camera_row_top && y <= script_camera_row_bottom) {
                //we clicked on the camera.
                buffer.Set("gce_script_edit_camera %d\n", i);
                server->CBuf_AddText(buffer);
            }
            else if (y >= script_sounds_row_top && y <= script_sounds_row_bottom) {
                //we clicked on the camera.
                buffer.Set("gce_script_edit_sounds %d\n", i);
                server->CBuf_AddText(buffer);
            }
            else if (y >= script_entities_row_top && y <= script_entities_row_bottom) {
                //we clicked on the entities.
                buffer.Set("gce_script_edit_entities %d\n", i);
                server->CBuf_AddText(buffer);
            }
            return;
        }
    }
}

