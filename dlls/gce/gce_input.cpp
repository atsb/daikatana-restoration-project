#include "p_user.h"
#include "keys.h"

#include "dk_cin_types.h"
#include "dk_gce_interface.h"

#include "gce_main.h"
#include "gce_hud.h"
#include "gce_sequence.h"
#include "gce_fields.h"
#include "gce_input.h"
#include "gce_headscript.h"
#include "gce_position.h"

#include "dk_misc.h"

//true if we are displaying the mouse cursor.
static bool display_cursor = false;

//true if we are allowing mlook
static bool allow_mlook = true;

//the position of the cursor.
static int mouse_x, mouse_y;
static int mouse_dx, mouse_dy;

// command key init state
static bool key_commands_inited = false;

// command key codes
static int  forward_keynum[2],
            back_keynum[2],
            right_keynum[2],
            left_keynum[2],
            up_keynum[2],
            down_keynum[2],
            xform_keynum[2],
            angle_keynum[2],
			select_node_keynum[2],
			freelook_keynum[2],
			node_toggle_keynum[2],
			copy_shot_keynum[2],
			paste_shot_keynum[2];

// command key states
static bool forward_keystate		= false, 
            back_keystate			= false,
            right_keystate			= false, 
            left_keystate			= false,
            up_keystate				= false, 
            down_keystate			= false,
			xform_keystate			= false,
			angle_keystate			= false,
			select_node_keystate	= false,
			freelook_keystate		= false;

typedef enum {
    HM_SEQUENCE,    //info about the current sequence is displayed.
    HM_SCRIPT,      //info about the entire current script is displayed
    HM_SOUNDS,      //info about the sounds in the current shot is displayed.
    HM_ENTITIES,    //info about the different entities in a shot.
    HM_POSITION,    //mode where we can move around and save a world position and facing direction.
    HM_HEADSCRIPT,  //sub-editor for head-script task recording

    HM_NUM_MODES,
} hud_mode;

void SequenceSelectPointFromTargetBox();

extern serverState_t *server;
extern hud_mode hud_cur_mode;

int INPUT_Sequence( int keydown, int keyup, bool shift_down )
{
    int fUsedKey = 1;

	if( keydown == xform_keynum[0] || keydown == xform_keynum[1] )
	{
		server->CBuf_AddText("gce_seq_edit_point_loc\n");
	}
	else if( keydown == angle_keynum[0]  || keydown == angle_keynum[1] )
	{
		server->CBuf_AddText("gce_seq_edit_point_angles\n");
	}
	else if( keydown == select_node_keynum[0] || keydown == select_node_keynum[1] )
	{
		SequenceSelectPointFromTargetBox();
	}
	else
	{
		fUsedKey = 0;
	}

	return fUsedKey;
}

int INPUT_Script( int keydown, int keyup, bool shift_down )
{
    int fUsedKey = 1;

	if( FieldEditing() == true ) return 0;

	if( keydown == copy_shot_keynum[0] || keydown == copy_shot_keynum[1] )
	{
		server->CBuf_AddText("gce_script_copy_shot\n");
	}
	else if( keydown == paste_shot_keynum[0] || keydown == paste_shot_keynum[1] )
	{
		server->CBuf_AddText("gce_script_paste_shot\n");
	}
	else
	{
		fUsedKey = 0;
	}

	return fUsedKey;
}

int INPUT_Sounds( int keydown, int keyup, bool shift_down )
{
    int fUsedKey = 0;

	return fUsedKey;
}

int INPUT_Entities( int keydown, int keyup, bool shift_down )
{
    int fUsedKey = 0;

	return fUsedKey;
}

int INPUT_Position( int keydown, int keyup, bool shift_down )
{
    int fUsedKey = 0;

	return fUsedKey;
}

int INPUT_HeadScript( int keydown, int keyup, bool shift_down )
{
    int fUsedKey = 0;

	return fUsedKey;
}

int InputKeyEvent(int keydown, int keyup, bool shift_down) {
    int fUsedKey = 0;

	if( !HUDActive() )
	{
		return fUsedKey;
	}

    //check if the cursor is showing and this is a mouse event.
    if (K_MOUSE1 == keydown) {
        //send the mouse click to the hud if the mouse is visible or we are recording a head script..
        if (MouseVisible() == true || HeadscriptRecording() == true) {
            //tell the hud that the mouse was clicked.
            HUD_MouseDown();
            return 1;
        }

        return 0;
    }
    else if (K_MOUSE2 == keydown)
    {
        if (EditingSequencePoint())
		{
            PositionCycleMouseAngleControl();
	        return 1;
		}
    }

    //check for shift on certain keys
    if (shift_down == true) {
        if (keydown == '-') {
            keydown = '_';
        }
    }

	// do we know our command codes?  ugh there are 2...
//    if (false == key_commands_inited)
    {
        memcpy(forward_keynum,     cin->Key_KeybindingToKeynum("+forward"), 2*sizeof(int));
        memcpy(back_keynum,        cin->Key_KeybindingToKeynum("+back"), 2*sizeof(int));
        memcpy(right_keynum,       cin->Key_KeybindingToKeynum("+moveright"), 2*sizeof(int));
        memcpy(left_keynum,        cin->Key_KeybindingToKeynum("+moveleft"), 2*sizeof(int));
        memcpy(up_keynum,          cin->Key_KeybindingToKeynum("+moveup"), 2*sizeof(int));
        memcpy(down_keynum,        cin->Key_KeybindingToKeynum("+movedown"), 2*sizeof(int));
        memcpy(xform_keynum,       cin->Key_KeybindingToKeynum("gce_adjust_transform"), 2*sizeof(int));
        memcpy(angle_keynum,       cin->Key_KeybindingToKeynum("gce_adjust_angle"), 2*sizeof(int));
        memcpy(select_node_keynum, cin->Key_KeybindingToKeynum("gce_select_node_in_targetbox"), 2*sizeof(int));
        memcpy(freelook_keynum,    cin->Key_KeybindingToKeynum("gce_freelook_toggle"), 2*sizeof(int));
        memcpy(node_toggle_keynum, cin->Key_KeybindingToKeynum("gce_node_toggle"), 2*sizeof(int));
        memcpy(copy_shot_keynum, cin->Key_KeybindingToKeynum("gce_copy_shot"), 2*sizeof(int));
        memcpy(paste_shot_keynum, cin->Key_KeybindingToKeynum("gce_paste_shot"), 2*sizeof(int));
		
//        key_commands_inited = true;
    }


	if( keydown != -1 )
	{
 		if( keydown == freelook_keynum[0] || keydown == freelook_keynum[1] )
		{
			if( !MouseVisible() && allow_mlook == true )
			{
				InputMouseShow();
			}
			else
			{
				InputMouseHide();
			}
			fUsedKey = 1;
		}
		else if( keydown == node_toggle_keynum[0] || keydown == node_toggle_keynum[1] )
		{
			HUDToggleNodes();
	        fUsedKey = 1;
		}
		else if( hud_cur_mode == HM_SEQUENCE )
		{
			INPUT_Sequence( keydown, keyup, shift_down );
		}
		else if( hud_cur_mode == HM_SCRIPT )
		{
			INPUT_Script( keydown, keyup, shift_down );
		}
		else if( hud_cur_mode == HM_SOUNDS )
		{ 
			INPUT_Sounds( keydown, keyup, shift_down );
		}
		else if( hud_cur_mode == HM_ENTITIES )
		{
			INPUT_Entities( keydown, keyup, shift_down );
		}
		else if( hud_cur_mode == HM_POSITION )
		{
			INPUT_Position( keydown, keyup, shift_down );
		}
		else if( hud_cur_mode == HM_HEADSCRIPT )
		{
			INPUT_HeadScript( keydown, keyup, shift_down );
		}

	}

	if (EditingSequencePointLoc())
    {
        fUsedKey = 1;

        if (-1 != keydown)
        {
            if (keydown == forward_keynum[0] || keydown == forward_keynum[1])
                forward_keystate = true;
            else if (keydown == back_keynum[0] || keydown == back_keynum[1])
                back_keystate = true;
            else if (keydown == right_keynum[0] || keydown == right_keynum[1])
                right_keystate = true;
            else if (keydown == left_keynum[0] || keydown == left_keynum[1])
                left_keystate = true;
            else if (keydown == up_keynum[0] || keydown == up_keynum[1])
                up_keystate = true;
            else if (keydown == down_keynum[0] || keydown == down_keynum[1])
                down_keystate = true;
            else
                fUsedKey = 0;
        }

        if (-1 != keyup)
        {
            if (keyup == forward_keynum[0] || keyup == forward_keynum[1])
                forward_keystate = false;
            else if (keyup == back_keynum[0] || keyup == back_keynum[1])
                back_keystate = false;
            else if (keyup == right_keynum[0] || keyup == right_keynum[1])
                right_keystate = false;
            else if (keyup == left_keynum[0] || keyup == left_keynum[1])
                left_keystate = false;
            else if (keyup == up_keynum[0] || keyup == up_keynum[1])
                up_keystate = false;
            else if (keyup == down_keynum[0] || keyup == down_keynum[1])
                down_keystate = false;
            else
                fUsedKey = 0;
        }
    }

    //check if we are editing a field in the hud.
    if (keydown != -1 && HUDEditing() == true) 
        return HUDKeyEvent(keydown);

    return fUsedKey;
}

int InputActive() {
    //check if we are editing a field.
    if (FieldEditing() == true) return 1;

    return 0;
}

void InputMouseHide() {
    display_cursor = false;
    allow_mlook = true;
}

void InputMouseShow() {
    display_cursor = true;
    allow_mlook = false;
}

void InputMouseHideNoMLook() {
    display_cursor = false;
    allow_mlook = false;
}

bool MouseVisible() {
    return display_cursor;
}

// return 0 for mlook and 1 for no mlook
int InputMouseMove(int dx, int dy) {

    // always update the deltas
    mouse_dx = dx;
    mouse_dy = dy;

    if (display_cursor == false)
    {
        if (true == allow_mlook)
            return 0;
        else
            return 1;
    }

    //add the displacement to our position.
    mouse_x += mouse_dx;
    mouse_y += mouse_dy;

    // get the window dimensions
    float width, height;

    cin->GetVidDims(width, height);

    //keep the cursor on the screen.
    bound_min(mouse_x, 0);
    bound_min(mouse_y, 0);
    bound_max(mouse_x, width);
    bound_max(mouse_y, height);

    return 1;
}

//the bitmaps we use for the mouse.
static char *cursor_bitmaps[] = {
    "/pics/interface/cursor_01.tga",
    "/pics/interface/cursor_02.tga",
    "/pics/interface/cursor_03.tga",
    "/pics/interface/cursor_04.tga",
    "/pics/interface/cursor_05.tga",
    "/pics/interface/cursor_06.tga",
    "/pics/interface/cursor_07.tga",
    "/pics/interface/cursor_08.tga",
    "/pics/interface/cursor_09.tga",
};

#define num_bitmaps (sizeof(cursor_bitmaps)/sizeof(char *))
#define frame_time (75)
#define anim_length (num_bitmaps * frame_time)

static int cursor_offset_x = 6;
static int cursor_offset_y = 8;

//draws the mouse cursor at its current position
void InputMouseDraw() {
    if (display_cursor == false) return;

    //get the ref export structure.
    refexport_t *re = cin->GetRefExport();
    if (re == NULL) return;

    //the frame we draw.
    int frame = (GetTickCount() % anim_length) / frame_time;

    //register the image.
	int	w, h;

    //draw the image.
	DRAWSTRUCT drawStruct;
    drawStruct.pImage = re->RegisterPic( cursor_bitmaps[ frame ], &w, &h, RESOURCE_GLOBAL );
	drawStruct.nFlags = DSFLAG_BLEND;
	drawStruct.nXPos = mouse_x - cursor_offset_x;
	drawStruct.nYPos = mouse_y - cursor_offset_y;
	re->DrawPic( drawStruct );
}

void InputMousePos(int &x, int &y) {
    x = mouse_x;
    y = mouse_y;
}

void InputMouseDeltaPos(int &dx, int &dy)
{
    dx = mouse_dx;
    dy = mouse_dy;
}

bool InputForward()
{
    return forward_keystate;
}

bool InputBack()
{
    return back_keystate;
}

bool InputRight()
{
    return right_keystate;
}
bool InputLeft()
{
    return left_keystate;
}
bool InputUp()
{
    return up_keystate;
}
bool InputDown()
{
    return down_keystate;
}

