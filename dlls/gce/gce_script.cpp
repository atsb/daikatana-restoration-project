#include "p_user.h"
#include "keys.h"
#include "ref.h"
#include <ctype.h>

#include "dk_buffer.h"
#include "dk_array.h"

#include "dk_cin_types.h"
#include "dk_gce_interface.h"
#include "dk_point.h"

#include "dk_gce_spline.h"
#include "dk_gce_script.h"

#include "gce_main.h"
#include "gce_sequence.h"
#include "gce_script.h"
#include "gce_hud.h"
#include "gce_sound.h"
#include "gce_entities.h"
#include "gce_entity_save.h"

void FastForwardScript(int shot_num);

//the script we are currently editing.
static CCinematicScript *script = NULL;
//the shot we are editing in the script
static int selected_shot_number = -1;

//true if the sequence module is modifying the sequence of the selected shot.
static bool modifying_sequence = false;
//true if we are modifying one of our shot's sounds in the sound module.
static bool modifying_sounds = false;
//true if we are modifying one of our shot's array of entities.
static bool modifying_entities = false;

CScriptShot *current_shot;

#define script_header() {if (cin == NULL) return; if (script == NULL) script = new CCinematicScript();}

void ScriptModified() {
    script_header();

    //send our script to the exe for playback operations.
    cin->UpdateScript(*script);
}

const CCinematicScript *ScriptCurrent() {
    if (script == NULL) script = new CCinematicScript();

    return script;
}

int ScriptCurrentShot() {
    return selected_shot_number;
}

CScriptShot *SelectedShot() {
    if (script == NULL) return NULL;
    if (selected_shot_number < 0 || selected_shot_number >= script->NumShots()) return NULL;

    return script->GetShot(selected_shot_number);
}

void ScriptSelectShot(int num) {
    script_header();

    selected_shot_number = num;

    //try to move the entities to updated positions
    if (selected_shot_number >= 0) {
        //fast forward the entities positions to the start of the selected shot.
        FastForwardScript(selected_shot_number);

        //get the selected shot.
        CScriptShot *shot = SelectedShot();
        if (shot != NULL) {
            //get the sequence from the shot.
            const CSplineSequence *sequence = shot->Sequence();
            if (sequence != NULL) {
                //generate entities to use to draw the camera spline for this shot.
                SequenceSet(sequence);
            }
        }
    }
}

void ScriptSelectShot() {
    script_header();

    //check our arguments.
    if (server->GetArgc() != 2) return;

    const char *arg = server->GetArgv(1);

    //get the number of the shot that was entered.
    int shot_num = atoi(arg);

    if (shot_num < 0 || shot_num >= script->NumShots() || arg[0] < '0' || arg[0] > '9') {
        //invalid shot number 
        return;
    }

    ScriptSelectShot(shot_num);
}

void ScriptSetName() {
    script_header();

    //check the number of arguments.
    if (server->GetArgc() != 2) return;

    //get the argument.
    const char *arg = server->GetArgv(1);

    //check to make sure the name is valid.
    if (arg == NULL || arg[0] == '\0') {
        return;
    }

    //set the name of our script.
    script->Name(arg);
}

void ScriptLoad() {
    script_header();

    //check our parameters.
    if (server->GetArgc() != 2) return;

    const char *script_name = server->GetArgv(1);

    //have the exe load the script.
    const CCinematicScript *newscript = cin->LoadScript(script_name);
    if (newscript == NULL) return;

    //copy the script.
    *script = *newscript;

    //send the loaded script back to the exe to be put down.
    cin->DeleteScript(newscript);
}

void ScriptAddShot() {
    script_header();

    script->AddShot();
}

void ScriptEditCamera() {
    script_header();

    //select the shot
    ScriptSelectShot();
    if (SelectedShot() == NULL) return;

    //get the sequence.
    const CSplineSequence *sequence = SelectedShot()->Sequence();

    //send the sequence to the sequence editing module.
    SequenceSet(sequence);

    modifying_sequence = true;

    //set the sequence header text.
    buffer256 buffer("Current Shot: %d", ScriptCurrentShot());
    SequenceHeader(buffer);

    //set the callback function that will restore script mode when the sequence editing is done.
    SequenceDoneFunction(ScriptSequenceModified);

    //change hud modes to the sequence module.
    HUDModeSequence();
}

void ScriptEditSounds() {
    script_header();

    //select the shot.
    ScriptSelectShot();
    if (SelectedShot() == NULL) return;

    //send the shot to the sound editing module.
    SoundEdit(SelectedShot()->Sounds());

    modifying_sounds = true;

    //change hud mode.
    HUDModeSounds();
}

void ScriptEditEntities() {
    script_header();

    //select the shot.
    ScriptSelectShot();
    if (SelectedShot() == NULL) return;

    //send the entities from the shot to the entity editing module.
    EntitiesEdit(SelectedShot()->Entities());

    modifying_entities = true;

    //change the hud mode
    HUDModeEntities();
}

void ScriptSequenceModified(const CSplineSequence *sequence) {
    //check if we have a selected shot.
    if (SelectedShot() == NULL) return;
    if (modifying_sequence == false) return;

    //copy the data from our sequence into our selected shot.
    SelectedShot()->SetSequence(sequence);

    modifying_sequence = false;

    //change hud modes
    HUDModeScript();
}

void ScriptSoundModified(const array<CScriptSound> &sounds) {
    //check if we have a shot selected
    if (SelectedShot() == NULL) return;
    if (modifying_sounds == false) return;

    //copy the data to our selected shot.
    SelectedShot()->SetSounds(sounds);

    modifying_sounds = false;

    //change hud modes
    HUDModeScript();
}

void ScriptEntitiesModified(const array<CScriptEntity> &entities) {
    //check if we have a shot selected.
    if (SelectedShot() == NULL) return;
    if (modifying_entities == false) return;

    SelectedShot()->SetEntities(entities);

    modifying_entities = false;

    //change back to script mode.
    HUDModeScript();
}

void ScriptShotSky() {
    script_header();
    
	//make sure we have a shot selected.
    CScriptShot *shot = SelectedShot();
    if (shot == NULL) return;

    //get the parameters.
    if (server->GetArgc() != 2) return;

    const char *arg = server->GetArgv(1);
    if (arg == NULL) return;

    int sky = atoi(arg);
    if (sky < 1) return;

    shot->Sky(sky);
}
                             

void ScriptShotExtraTimeStart() {
    script_header();
    //make sure we have a shot selected.
    CScriptShot *shot = SelectedShot();
    if (shot == NULL) return;

    //get the parameters.
    if (server->GetArgc() != 2) return;

    const char *arg = server->GetArgv(1);
    if (arg == NULL) return;

    float time = atof(arg);
    if (time < 0.0f) return;

    shot->ExtraTimeStart(time);
}
                             
void ScriptShotExtraTimeEnd() {
    script_header();
    //make sure we have a shot selected.
    CScriptShot *shot = SelectedShot();
    if (shot == NULL) return;

    //get the parameters.
    if (server->GetArgc() != 2) return;

    const char *arg = server->GetArgv(1);
    if (arg == NULL) return;

    float time = atof(arg);
    if (time < 0.0f) return;

    shot->ExtraTimeEnd(time);
}

void ScriptShotStartFOV() {
    script_header();

    //make sure we have a shot selected.
    CScriptShot *shot = SelectedShot();
    if (shot == NULL) return;

    //get the parameters.
    if (server->GetArgc() != 2) return;

    const char *arg = server->GetArgv(1);
    if (arg == NULL) return;

    float fov = atof(arg);

    bound_min(fov, SCRIPT_FOV_MIN);
    bound_max(fov, SCRIPT_FOV_MAX);

    shot->StartFOV(fov);
}

void ScriptShotHaveStartFOVToggle() {
    script_header();

    //make sure we have a shot selected.
    CScriptShot *shot = SelectedShot();
    if (shot == NULL) return;

    shot->HaveStartFOV(!shot->HaveStartFOV());
}

void ScriptShotInsert() {
    script_header();

    //get the parameter.
    if (server->GetArgc() != 2) return;

    const char *arg = server->GetArgv(1);
    if (arg == NULL || arg[0] == '\0') return;

    int position = atoi(arg);

    //check if this is a valid position.
    if (position < 0 || position > script->NumShots()) return;

    //add a new shot to the script at that position.
    script->InsertShot(position);
}

void ScriptShotDeleteSelected() {
    script_header();

    //make sure we have a shot selected.
    CScriptShot *shot = SelectedShot();
    if (shot == NULL) return;

    //delete the shot from the script.
    script->DeleteShot(shot);

    ScriptSelectShot(-1);
}

void ScriptShotCameraTarget() {
    script_header();

    //check the arguments.
    if (server->GetArgc() != 2) return;
    if (!isdigit(server->GetArgv(1)[0])) return;

    int arg = atoi(server->GetArgv(1));
    if (arg < CT_SEQUENCE || arg > CT_ENTITY) return;

    //make sure we have a shot selected.
    CScriptShot *shot = SelectedShot();
    if (shot == NULL) return;

    //set the camera target type.
    shot->CameraTarget((camera_target)arg);
}

void ScriptShotCameraTargetEntity() {
    script_header();

    //check the arguments.
    if (server->GetArgc() != 2) return;
    
    //get the name that was passed in.
    const char *name = server->GetArgv(1);

    //get the selected shot.
    CScriptShot *shot = SelectedShot();
    if (shot == NULL) return;

    //set the camera target entity.
    shot->EntityCameraTarget(name);
}

void ScriptShotEndFlag() {
    script_header();
    
    //check the arguments.
    if (server->GetArgc() != 2) return;
    if (!isdigit(server->GetArgv(1)[0])) return;

    int arg = atoi(server->GetArgv(1));
    if (arg < SE_SEQUENCE || arg > SE_ENTITY) return;

    //get the selected shot.
    CScriptShot *shot = SelectedShot();
    if (shot == NULL) return;

    //set the end flag.
    shot->EndFlag((shot_end)arg);
}

void ScriptShotEndEntity() {
    script_header();

    //check the arguments.
    if (server->GetArgc() != 2) return;

    //get the name that was passed in.
    const char *name = server->GetArgv(1);

    //get the selected shot.
    CScriptShot *shot = SelectedShot();
    if (shot == NULL) return;

    //set the entity name.
    shot->EntityShotEnd(name);
}


void FastForwardScript(int shot_num) {
    script_header();

    //first restore whatever save info we have.
    RestoreEntityStates();

    //save states for everyone in the script.
    SaveEntityStates(ScriptCurrent());

    //go through the script shots up to the one we are going to.
    for (int i = 0; i < shot_num; i++) {
        //get the shot.
        const CScriptShot *shot = script->Shot(i);
        if (shot == NULL) continue;

        //tell the shot to fast forward to it's end.
        FastForwardEntities(shot);
    }
}

void gce_set_position(struct edict_s *) {
    script_header();
    if (server == NULL) return;

    ScriptModified();

    //move the entities to their initial positions.
    FastForwardScript(0);

    //send the command to the exe to play the script.
    server->CBuf_AddText("cin_play\n");
}

void ScriptPlayCurrentShot() {
    script_header();
    if (server == NULL) return;

    if (ScriptCurrentShot() < 0) return;

    FastForwardScript(ScriptCurrentShot());

    //send the script to the exe.
    ScriptModified();

    //play the script starting at this shot.
    buffer64 command("cin_play_shot %d\n", ScriptCurrentShot());
    server->CBuf_AddText(command);
}

void ScriptPlayCancelShot()
{
    script_header();
    if (server == NULL) return;

	server->CBuf_AddText("cin_cancel_shots");
}

void ScriptPlayAllShots()
{
    script_header();
    if (server == NULL) return;

	int num_shots = script->NumShots();

	// SCG[8/26/99]: update all the shots
	for( int i = 0; i < num_shots; i++ )
	{
		ScriptSelectShot( i );

	    if (ScriptCurrentShot() < 0) return;

	    FastForwardScript(ScriptCurrentShot());

		//send the script to the exe.
		ScriptModified();
	}

	//play the script starting at this shot.
	buffer64 command( "cin_play_all_shots %d\n", num_shots );
	server->CBuf_AddText(command);
}

void ScriptPlayFromCurrentShot()
{
    script_header();
    if (server == NULL) return;

	int current_shot = ScriptCurrentShot();
    if ( current_shot < 0) return;

	int num_shots = script->NumShots();
    if ( num_shots < 1) return;

	// SCG[8/26/99]: update all the shots
	for( int i = ScriptCurrentShot(); i < num_shots; i++ )
	{
		ScriptSelectShot( i );

	    if (ScriptCurrentShot() < 0) return;

	    FastForwardScript(ScriptCurrentShot());

		//send the script to the exe.
		ScriptModified();
	}

	//play the script starting at this shot.
	buffer64 command( "cin_play_from_current_shot %d %d\n", current_shot, num_shots );
	server->CBuf_AddText(command);
}

void ScriptCopyShot()
{
    script_header();
    if (server == NULL) return;

    if (ScriptCurrentShot() < 0) return;

	current_shot = SelectedShot();
}

void ScriptPasteShot()
{
    script_header();
    if (server == NULL) return;

    if (ScriptCurrentShot() < 0) return;

    CScriptShot *shot = SelectedShot();

	*shot = *current_shot;
}
