#include "p_user.h"
#include "dk_system.h"

#include "dk_cin_types.h"
#include "dk_gce_interface.h"
#include "dk_point.h"

#include "dk_buffer.h"

#include "gce_main.h"
#include "gce_position.h"
#include "gce_input.h"

//the prompt we display when we draw the hud
static buffer128 hud_prompt;
static buffer128 hud_prompt_angles_off;
static buffer128 hud_prompt_angles_yawpitch;
static buffer128 hud_prompt_angles_roll;

//commands we run when we save/abort the process
static buffer256 command_save;
static buffer256 command_abort;

//when we save, the position and direction are saved here.
static CVector saved_position;
static CVector saved_angles;

//storage for other modules to use to tell position where to put something
static CVector position_pos_start(0,0,0);
static CVector position_angles_start(0,0,0);

static CVector position_pos_current(0,0,0);
static CVector position_angles_current(0,0,0);

//storage for updating data given by other modules
static bool position_pos_update = false;    // true if this module will update pos
static bool position_angles_update = false; // true if this module will update angles
static PositionUpdateCall position_update_func = NULL;

// set this to edit from the point of view of the selected object
static bool position_first_person_edit = false;

typedef enum
{
    MOUSE_ANGLE_CONTROL_OFF,
    MOUSE_ANGLE_CONTROL_YAWPITCH,
    MOUSE_ANGLE_CONTROL_ROLL
} MouseAngleControlState;

static MouseAngleControlState mouse_angle_control_state = MOUSE_ANGLE_CONTROL_OFF;
static bool mouse_flipy = false;

#define position_header() if (server == NULL || cin == NULL) return;

//sets the string that will be used as a prompt in the position mode.
void PositionPrompt(const char *string) {
    hud_prompt_angles_off = string;
    hud_prompt_angles_yawpitch = string;
    hud_prompt_angles_roll = string;
    hud_prompt = string;
}

void PositionPromptAnglesOff(const char *string) {
    hud_prompt_angles_off = string;
    hud_prompt = string;
}

void PositionPromptAnglesYawPitch(const char *string) {
    hud_prompt_angles_yawpitch = string;
}

void PositionPromptAnglesRoll(const char *string) {
    hud_prompt_angles_roll = string;
}

//sets the command that is executed when the user wants to save/abort their position
void PositionSaveCommand(const char *command) {
    command_save = command;
}

void PositionAbortCommand(const char *command) {
    command_abort = command;
}

//gets the prompt...
const char *PositionPrompt() {
    return hud_prompt;
}

void PositionSave() {
    position_header();

    position_angles_update = false;
    position_pos_update = false;

    //get the last camera position from the exe.
    cin->LastCameraPos(saved_position, saved_angles);

    //run our save command.
    server->CBuf_AddText(command_save);
}

void PositionAbort() {
    position_header();

    position_angles_update = false;
    position_pos_update = false;

    //run our abort command.
    server->CBuf_AddText(command_abort);
}

const CVector &PositionLoc() {
    return saved_position;
}

const CVector &PositionAngles() {
    return saved_angles;
}

const CVector PositionLocStart() {
    return position_pos_start;
}

const CVector PositionAnglesStart() {
    return position_angles_start;
}

const CVector PositionLocCurrent() {
    return position_pos_current;
}

const CVector PositionAnglesCurrent() {
    return position_angles_current;
}

void PositionSetCamera(CVector &viewpos, CVector &viewangles) {
    position_header();

    if (true==position_first_person_edit)
    {
        viewpos = position_pos_current;
        viewangles = position_angles_current;
    }
}

void PositionSetFirstPersonEdit(bool fVal)
{
    position_first_person_edit = fVal;
}

bool PositionGetFirstPersonEdit()
{
    return position_first_person_edit;
}

bool PositionGetMouseEnabled()
{
    return (MOUSE_ANGLE_CONTROL_OFF==mouse_angle_control_state);
}

void PositionCycleMouseAngleControl()
{
    if (false==position_angles_update)
        return;
    
    if (MOUSE_ANGLE_CONTROL_OFF == mouse_angle_control_state)
    {
        mouse_angle_control_state = MOUSE_ANGLE_CONTROL_YAWPITCH;
        InputMouseHideNoMLook();
        hud_prompt = hud_prompt_angles_yawpitch;
        
        cvar_t *m_pitch = server->cvar("m_pitch", "0", 0);
        
        if (m_pitch->value<0.0f)
            mouse_flipy = true;
        else
            mouse_flipy = false;
    }
    else if (MOUSE_ANGLE_CONTROL_YAWPITCH == mouse_angle_control_state)
    {
        mouse_angle_control_state = MOUSE_ANGLE_CONTROL_ROLL;
        InputMouseHideNoMLook();
        
        hud_prompt = hud_prompt_angles_roll;
    }
    else
    {
        mouse_angle_control_state = MOUSE_ANGLE_CONTROL_OFF;
        InputMouseShow();

        hud_prompt = hud_prompt_angles_off;
    }
}

void PositionUpdateObjects(CVector startPos, bool fUpdatePos, CVector startAngles, bool fUpdateAngles)
{
    position_pos_start = startPos;
    position_angles_start = startAngles;
    
    position_pos_current = startPos;
    position_angles_current = startAngles;

    position_pos_update = fUpdatePos;
    position_angles_update = fUpdateAngles;
}

void PositionUpdateFunc(PositionUpdateCall pFunc)
{
    position_update_func = pFunc;
}

void PositionUpdate()
{
    CVector forward, right, up;
    
    if (position_pos_update||position_angles_update) {
        
        // allow angular updates in both modes
        position_angles_current.AngleToVectors(forward, right, up);

        if (MOUSE_ANGLE_CONTROL_YAWPITCH == mouse_angle_control_state)
        {
            // update the yaw and pitch with the mouse deltas
            int dx, dy;

            InputMouseDeltaPos(dx, dy);
            
            if (mouse_flipy)
                position_angles_current.pitch -= dy/4;
            else
                position_angles_current.pitch += dy/4;

            position_angles_current.yaw -= dx/4;
            
            // rangelimit
            position_angles_current.pitch = min(max(position_angles_current.pitch, -90.0f), 90.0f);
            while (position_angles_current.yaw>180.0f) position_angles_current.yaw-=360.0f;
            while (position_angles_current.yaw<-180.0f) position_angles_current.yaw+=360.0f;
        }
        else if (MOUSE_ANGLE_CONTROL_ROLL == mouse_angle_control_state)
        {
            // update the roll mouse deltas
            int dx, dy;

            InputMouseDeltaPos(dx, dy);
            
            position_angles_current.roll += dx/4;
        }

        // allow positional updates only in editing_point_loc mode
        if (position_pos_update) {
            
            if (true == InputForward()) 
                position_pos_current += 4*forward;
            if (true == InputBack())    
                position_pos_current -= 4*forward;
            if (true == InputRight())   
                position_pos_current += 4*right;
            if (true == InputLeft())    
                position_pos_current -= 4*right;
            if (true == InputUp())      
                position_pos_current += 4*up;
            if (true == InputDown())    
                position_pos_current -= 4*up;
        }
    }

    // now call our callback, if any
    if (position_update_func)
        position_update_func();
}


