#include "p_user.h"

#include "dk_cin_types.h"
#include "dk_gce_interface.h"

#include "dk_point.h"
#include "dk_buffer.h"

#include "gce_main.h"
#include "gce_position.h"
#include "gce_hud.h"
#include "gce_entities.h"

#include "dk_gce_spline.h"
#include "dk_gce_headbob.h"

//true if we are doing script recording.
static bool recording = false;
//when we are recording, this is the last time we recorded the position.
static int recording_last_time;

//the current head script we are making.
static CEntityHeadScript headscript;

//returns true if we are recording a headscript.
bool HeadscriptRecording() {
    return recording;
}

//called once per frame when a script is recording.
void HeadscriptFrame() {
    if (recording == false) return;

    //get the current time.
    int time = GetTickCount();

    //get the elapsed time since the previous time.
    int elapsed = time - recording_last_time;

    //check if enough time has passed.
    if (elapsed < HEADSCRIPT_RESOLUTION) {
        //wait some more
        return;
    }

    //get the current angles of the camera.
    CVector camera_angles, camera_position;
    cin->LastCameraPos(camera_position, camera_angles);

    //add the camera angles to our head script.
    headscript.Add(camera_angles);

    //update our last time variable.
    if (elapsed < HEADSCRIPT_RESOLUTION * 2) {
        //if the elapsed time is not totally rediculous, just add our script resolution to it.
        recording_last_time += HEADSCRIPT_RESOLUTION;
    }
    else {
        //just use the current time as the last time.
        recording_last_time = time;
    }
}

//returns the headscript that was recorded.
const CEntityHeadScript &HeadscriptRecorded() {
    return headscript;
}

void HeadscriptStart() {
    //reset our headscript var.
    headscript.Reset();

    //get the current time.
    recording_last_time = GetTickCount();

    //set our flag.
    recording = true;

    //hide the mouse cursor.
    HUDShowCursor();
}

void HeadscriptSave() {
    recording = false;

    HUDShowCursor();

    EntitiesTaskHeadScriptSave();
}

void HeadscriptAbort() {
    recording = false;

    HUDShowCursor();

    EntitiesTaskHeadScriptAbort();
}


