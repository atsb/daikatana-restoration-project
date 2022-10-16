#include "p_user.h"
#include "keys.h"

#include "ref.h"
#include "dk_array.h"

#include "dk_cin_types.h"
#include "dk_gce_interface.h"
#include "dk_point.h"
#include "dk_gce_spline.h"
#include "dk_gce_script.h"
#include "dk_gce_entities.h"
#include "dk_buffer.h"
#include "dk_misc.h"

#include "gce_main.h"
#include "gce_sequence.h"
#include "gce_script.h"
#include "gce_input.h"
#include "gce_fields.h"
#include "gce_hud.h"
#include "gce_sound.h"
#include "gce_entities.h"
#include "gce_position.h"
#include "gce_headscript.h"

void MouseDownScript(int x, int y);
void MouseDownSounds(int x, int y);
void MouseDownEntities(int x, int y);
void MouseDownHeadScript(int x, int y);

int HUD_HeadScriptKeyEvent(int key);

//our different hud modes.
typedef enum {
    HM_SEQUENCE,    //info about the current sequence is displayed.
    HM_SCRIPT,      //info about the entire current script is displayed
    HM_SOUNDS,      //info about the sounds in the current shot is displayed.
    HM_ENTITIES,    //info about the different entities in a shot.
    HM_POSITION,    //mode where we can move around and save a world position and facing direction.
    HM_HEADSCRIPT,  //sub-editor for head-script task recording

    HM_NUM_MODES,
} hud_mode;

extern field_description field_desc_sequence[];
extern field_description field_desc_script[];
extern field_description field_desc_sounds[];
extern field_description field_desc_entities[];
extern field_description field_desc_position[];
extern field_description field_desc_headscript[];

//pointers to the arrays of field descriptions for each mode.
field_description *field_descriptions[HM_NUM_MODES] = {
    field_desc_sequence,
    field_desc_script,
    field_desc_sounds,
    field_desc_entities,
    field_desc_position,
    field_desc_headscript,
};

void HUD_Sequence();
void HUD_Script();
void HUD_Sounds();
void HUD_Entities();
void HUD_Position();
void HUD_HeadScript();

void HUD_DrawHilight();
void HUD_DrawTargetBox();

bool hud_on = false;

//these functions return true or false to say whether the given field in can be used at this time.
bool field_mask_sequence(int field);
bool field_mask_script(int field);
bool field_mask_sounds(int field);
bool field_mask_entities(int field);
bool field_mask_position(int field);
bool field_mask_headscript(int field);

//these functions start the edit process for the given field.
void edit_start_sequence(int field);
void edit_start_script(int field);
void edit_start_sounds(int field);
void edit_start_entities(int field);
void edit_start_position(int field);
void edit_start_headscript(int field);

//these functions end the edit process for the given field.
void edit_end_sequence(int field);
void edit_end_script(int field);
void edit_end_sounds(int field);
void edit_end_entities(int field);
void edit_end_position(int field);
void edit_end_headscript(int field);


extern int num_fields_sequence;
extern int num_fields_script;
extern int num_fields_sounds;
extern int num_fields_entities;
extern int num_fields_position;
extern int num_fields_headscript;

//info for all our hud modes.
hud_info info[HM_NUM_MODES] = {
    {num_fields_sequence, -1, field_mask_sequence, edit_start_sequence, edit_end_sequence},
    {num_fields_script, -1, field_mask_script, edit_start_script, edit_end_script},
    {num_fields_sounds, -1, field_mask_sounds, edit_start_sounds, edit_end_sounds},
    {num_fields_entities, -1, field_mask_entities, edit_start_entities, edit_end_entities},
    {num_fields_position, -1, field_mask_position, edit_start_position, edit_end_position},
    {num_fields_headscript, -1, field_mask_headscript, edit_start_headscript, edit_end_headscript},
};

//our current mode.
hud_mode hud_cur_mode = HM_SCRIPT;
static bool hud_show_cursor = true;
static bool hud_show_nodes = true;

void HUDShowCursor() {
    //check if we are in the head script recording mode.
    if (hud_cur_mode == HM_HEADSCRIPT && HeadscriptRecording() == true) {
        InputMouseHide();
        return;
    }

    if (hud_on == true && hud_show_cursor == true) {
        InputMouseShow();
    }
    else {
        InputMouseHide();
    }
}

void HUDToggleCursor() {
    hud_show_cursor = !hud_show_cursor;

    HUDShowCursor();
}

void HUDToggleNodes()
{
	hud_show_nodes = !hud_show_nodes;
}

void HUDModeSequence() {
    hud_cur_mode = HM_SEQUENCE;
}

void HUDModeSounds() {
    hud_cur_mode = HM_SOUNDS;
}

void HUDModeScript() {
    hud_cur_mode = HM_SCRIPT;
}

void HUDModeEntities() {
    hud_cur_mode = HM_ENTITIES;
}

void HUDModePosition() {
    hud_cur_mode = HM_POSITION;
}

void HUDModeHeadScript() {
    hud_cur_mode = HM_HEADSCRIPT;
}

// here's a spot for doing any per-frame maintenance
void GCE_HUD_PerFrameUpdate()
{
    PositionUpdate();
}

//draws the hud.
void GCE_HUD() {
    if (hud_on == false) return;

	refdef_t *rd = cin->GetRefDef();

    HUDDrawBox(0, 0, rd->width, 66, CVector(0.0, 0.0, 0.0));

    // update any per-frame stuff
    GCE_HUD_PerFrameUpdate();

    if (hud_cur_mode == HM_SEQUENCE) {
        HUD_Sequence();
    }
    else if (hud_cur_mode == HM_SCRIPT) {
        HUD_Script();
    }
    else if (hud_cur_mode == HM_SOUNDS) {
        HUD_Sounds();
    }
    else if (hud_cur_mode == HM_ENTITIES) {
        HUD_Entities();
    }
    else if (hud_cur_mode == HM_POSITION) {
        HUD_Position();
    }
    else if (hud_cur_mode == HM_HEADSCRIPT) {
        HUD_HeadScript();
    }

	HUD_DrawTargetBox();

    //draw a box around the input field the mouse is over.
    HUD_DrawHilight();

    InputMouseDraw();
}

bool HUDActive() {
    return hud_on;
}

//turns the hud on
void GCE_HUD_On() {
    hud_on = true;

    HUDShowCursor();
}

void GCE_HUD_Off() {
    hud_on = false;

    HUDShowCursor();
}

void GCE_HUD_Toggle() {
    hud_on = !hud_on;

    HUDShowCursor();
}

field_description *HilightField(int &field_index) {
    //get the location of the mouse cursor.
    int x, y;
    InputMousePos(x, y);

    //go through the fields of the current mode.
    hud_info &info = ::info[hud_cur_mode];

    for (field_index = 0; field_index < info.fields; field_index++) {
        //get the coordinates of the field.
        int left, top, right, bottom;
        FieldBox(field_descriptions[hud_cur_mode][field_index], left, top, right, bottom);

        //check if the mouse is inside this field.
        if (x >= left && x <= right && y >= top && y <= bottom) {
            //the mouse is in this field.
            //check if the field is active.
            if (info.field_mask(field_index) == true) {
                return &field_descriptions[hud_cur_mode][field_index];
            }
        }
    }

    return NULL;
}

refdef_t	*rd;
void HUD_DrawTargetBox()
{
	int			left, top, right, bottom;
	
	if( rd == NULL )
	{
		rd = cin->GetRefDef();
	}

	left = ( rd->width / 2 )  - 32;
	right = ( rd->width / 2 )  - 32;
	top = ( ( rd->height / 2 )  - 8 ) + 60;		// SCG[8/20/99]: + 60 for the letter box
	bottom = ( ( rd->height / 2 )  + 8 ) + 60;	// SCG[8/20/99]: + 60 for the letter box

    HUDDrawEmptyBox(left, top, right, bottom, CVector(1.0, 1.0, 1.0));

	left = ( rd->width / 2 )  + 32;
	right = ( rd->width / 2 )  + 32;

    HUDDrawEmptyBox(left, top, right, bottom, CVector(1.0, 1.0, 1.0));
}

void HUD_DrawHilight() {
    //check if the mouse cursor is visible.
    if (MouseVisible() == false) return;
    //check if we are editing a field
    if (FieldEditing() == true) return;

    //get the field that is under the mouse.
    int field_index;
    field_description *field = HilightField(field_index);
    if (field == NULL) return;

    //draw the field.
    FieldDrawBox(*field, CVector(1.0, 1.0, 1.0));
}

//choices in the script shot camera target field.
char *choices_entity_default[] = {
    "Default",
    "Entity ",
};

//main dll entry point that allows us to add entities to be drawn on the client each frame.
void AddEntities() {
    if (hud_on == false) return;

    if ((hud_cur_mode == HM_SEQUENCE) ||

        //(hud_cur_mode == HM_SCRIPT && ScriptCurrentShot() != -1) || 
        (hud_cur_mode == HM_POSITION && EditingSequencePoint() == true)
       ) 

    {
        SequenceAddEntities();
    }
    else if (hud_cur_mode == HM_ENTITIES)
    {
        if (-1!=HUD_GetSelectedEntity())
            EntitiesAddEntityPath(HUD_GetSelectedEntity());
    }
}

//returns true if we are editing a field in the hud.
bool HUDEditing() {
    return FieldEditing();
}

//returns 1 if the key was handled.
int HUDKeyEvent(int key) {
    //check if the current hud mode wants this key event.
    if (hud_cur_mode == HM_HEADSCRIPT) {
        if (HUD_HeadScriptKeyEvent(key) == 1) return 1;
    }

    //let the current field have the key event.
    return FieldKeyEvent(key, info[hud_cur_mode]);
}

void SetCamera(CVector &viewpos, CVector &viewangles) {
    //check our mode.
    if (hud_cur_mode == HM_SEQUENCE) {
        SequenceSetCamera(viewpos, viewangles);
    }
    else if (hud_cur_mode == HM_POSITION) {
        PositionSetCamera(viewpos, viewangles);
    }
}

void HUD_MouseDown() {
    //check if we are recording a head script.
    if (hud_cur_mode == HM_HEADSCRIPT && HeadscriptRecording() == true) {
        MouseDownHeadScript(0, 0);
        return;
    }

    //check if the mouse cursor is visible.
    if (MouseVisible() == false) return;
    //check if we are editing a field
    if (FieldEditing() == true) return;

    //get the field that is under the mouse.
    int field_index;
    field_description *field = HilightField(field_index);
    if (field == NULL) {
        //get the current mouse position.
        int x, y;
        InputMousePos(x, y);

        //check for special click tests in our current mode.
        if (hud_cur_mode == HM_SCRIPT) {
            MouseDownScript(x, y);
        }
        else if (hud_cur_mode == HM_SOUNDS) {
            MouseDownSounds(x, y);
        }
        else if (hud_cur_mode == HM_ENTITIES) {
            MouseDownEntities(x, y);
        }
        return;   
    }

    //check if the field was a button.
    if (field->type == FT_BUTTON || field->type == FT_RADIO) {
        //call the edit_end function.
        info[hud_cur_mode].edit_end(field_index);
    }
    else {
        info[hud_cur_mode].cur_field = field_index;
        //start editing that field.
        info[hud_cur_mode].edit_start(field_index);
    }
}

void HUDFieldCycle() {
    if (hud_on == false) return;

    FieldCycle(info[hud_cur_mode]);
}

void HUDDrawBox(int left, int top, int right, int bottom, CVector& rgbColor) {
    if (left > right) return;
    if (top > bottom) return;

    //get the ref export structure.
    refexport_t *re = cin->GetRefExport();
    if (re == NULL) return;

//    re->DrawStretchPic(left, top, right - left + 1, bottom - top + 1, pic_name, RESOURCE_GLOBAL);
	// consolidation change: SCG 3-11-99
	DRAWSTRUCT drawStruct;
	drawStruct.nFlags = DSFLAG_COLOR | DSFLAG_FLAT;
    drawStruct.pImage = NULL;
	drawStruct.nLeft = left;				// SCG[3/30/99]: XPos
	drawStruct.nTop = top;					// SCG[3/30/99]: YPos
	drawStruct.nRight = right - left + 1;	// SCG[3/30/99]: Width
	drawStruct.nBottom = bottom - top + 1;	// SCG[3/30/99]: Height
	drawStruct.rgbColor = rgbColor;			// SCG[3/30/99]: Color
	re->DrawPic( drawStruct );
}

void HUDDrawEmptyBox(int left, int top, int right, int bottom, CVector& rgbColor) {
    //draw the 4 edges.
    HUDDrawBox(left, top, left, bottom, rgbColor);
    HUDDrawBox(left, top, right, top, rgbColor);
    HUDDrawBox(left, bottom, right, bottom, rgbColor);
    HUDDrawBox(right, top, right, bottom, rgbColor);
}

