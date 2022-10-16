#include "dk_misc.h"

#include "ref.h"

#include "dk_cin_types.h"
#include "dk_buffer.h"
#include "dk_array.h"
#include "dk_point.h"

#include "dk_gce_spline.h"
#include "dk_gce_script.h"
#include "dk_gce_entities.h"
#include "dk_cin_world_interface.h"
#include "dk_gce_headbob.h"

#include "sound.h"
#include "client.h"

//
// system playback entry points.
//

bool fLoopingASound = false;
CScriptSound loopingSound;
void CinPlaybackSound(const CScriptSound *sound) {

    if (sound->Name() == NULL || sound->Name()[0] == '\0') return;

	cvar_t	*s_cine = Cvar_Get("s_cine","0.7",CVAR_ARCHIVE);

    buffer128 buffer;

	buffer.Set("sounds/%s", sound->Name());

	int channel = 0;

	if( sound->Loop() == true )
	{
		fLoopingASound = true;
		loopingSound = *sound;
		channel = CHAN_LOOP;
	}
	else
	{
		fLoopingASound = false;
	}

	if( strstr( buffer, ".mp3" ) )
	{
		channel = sound->Channel();
		S_StartMP3( buffer, channel, s_cine->value, ( fLoopingASound == true ) ? 0 : 1 );
	}
	else
	{
		S_StartSoundQuake2(CVector(0,0,0), 1, channel,  S_RegisterSound(sound->Name()), 1.0f, ATTN_NORM, 0.0f);
	}
        
}

void CinStopLoopingSound(CScriptSound *sound) {

    S_StartSoundQuake2(CVector(0,0,0), 1, CHAN_LOOP,  S_RegisterSound(sound->Name()), 0.0f, ATTN_NORM, 0.0f);

}

extern cin_world_import_t world_import;
extern bool got_world_import;
void TeleportEntity(const char *entity_name, const char *id, CVector &pos, CVector &angles, bool use_angles);
void CinPlaybackHeadScript(const char *entity_name, const char *id, const CScriptEntityTask *task, int start_time, int cur_time);

void CinPlaybackEntityTask(const char *entity_name, const CScriptEntityTask *task, int start_time, int end_time) {
    if (got_world_import == false) return;

    //check the task type.
    switch (task->Type()) {
		case TT_SPAWN_ENTITY:
			if( world_import.SpawnEntity == NULL ) return;
			world_import.SpawnEntity(entity_name, task->UniqueID(), task->Destination(), task->Direction());
			break;
		case TT_REMOVE_ENTITY:
			if( world_import.RemoveEntity == NULL ) return;
			world_import.RemoveEntity(entity_name, task->UniqueID());
			break;
		case TT_CLEAR_GOALS:
			if( world_import.ClearGoals == NULL ) return;
			world_import.ClearGoals(entity_name, task->UniqueID());
			break;
        case TT_MOVE_TO:
            if (world_import.QueueMoveTo == NULL) return;
            world_import.QueueMoveTo(entity_name, task->UniqueID(), task->Destination(), task->AnimationName() );
            break;
        case TT_TURN:
            if (world_import.QueueTurn == NULL) return;
            world_import.QueueTurn(entity_name, task->UniqueID(), task->Direction());
            break;
        case TT_MOVE_AND_TURN:
            if (world_import.QueueMoveAndTurn == NULL) return;
            world_import.QueueMoveAndTurn(entity_name, task->UniqueID(), task->Destination(), task->Direction());
            break;
        case TT_TELEPORT:
		{
            CVector taskDestination = task->Destination();
			CVector taskDirection = task->Direction();
			TeleportEntity(entity_name, task->UniqueID(), taskDestination, taskDirection, true);
            break;
        }
		case TT_WAIT:
            if (world_import.QueueWait == NULL) return;
            world_import.QueueWait(entity_name, task->UniqueID(), task->AttributeValue());
            break;
        case TT_ATTRIBUTE_BACKUP:
            if (world_import.AttributesBackup == NULL) return;
            world_import.AttributesBackup(entity_name, task->UniqueID());
            break;
        case TT_ATTRIBUTE_RESTORE:
            if (world_import.AttributesRestore == NULL) return;
            world_import.AttributesRestore(entity_name, task->UniqueID());
            break;
        case TT_ATTRIBUTE_RUN_SPEED:
            if (world_import.AttributesRunSpeed == NULL) return;
            world_import.AttributesRunSpeed(entity_name, task->UniqueID(), task->AttributeValue());
            break;
        case TT_ATTRIBUTE_WALK_SPEED:
            if (world_import.AttributesWalkSpeed == NULL) return;
            world_import.AttributesWalkSpeed(entity_name, task->UniqueID(), task->AttributeValue());
            break;
        case TT_ATTRIBUTE_YAW_SPEED:
            if (world_import.AttributesTurnSpeed == NULL) return;
            world_import.AttributesTurnSpeed(entity_name, task->UniqueID(), task->AttributeValue());
            break;
        case TT_START_RUNNING:
            if (world_import.QueueStartRunning == NULL) return;
            world_import.QueueStartRunning(entity_name, task->UniqueID());
            break;
        case TT_START_WALKING:
            if (world_import.QueueStartWalking == NULL) return;
            world_import.QueueStartWalking(entity_name, task->UniqueID(), task->AnimationName() );
            break;
        case TT_TRIGGER_BRUSH_USE:
            if (world_import.QueueTriggerBrushUse == NULL) return;
            world_import.QueueTriggerBrushUse(entity_name, task->UniqueID(), task->UseEntityName());
            break;
        case TT_HEAD_SCRIPT:
            //call our head script playback function.
            CinPlaybackHeadScript(entity_name, task->UniqueID(), task, start_time, end_time);
            break;
        case TT_PLAY_ANIMATION:
            if (world_import.QueueAnimation == NULL) return;
            world_import.QueueAnimation(entity_name, task->UniqueID(), task->AnimationName());
            break;
        case TT_SET_IDLE_ANIMATION:
            if (world_import.QueueSetIdleAnimation == NULL) return;
            world_import.QueueSetIdleAnimation(entity_name, task->UniqueID(), task->AnimationName());
            break;
        case TT_PLAY_SOUND:
            if (world_import.QueuePlaySound == NULL) return;
            world_import.QueuePlaySound(entity_name, task->UniqueID(), task->SoundName(), task->SoundDuration());
            break;
    }
}

void ShutdownCinematicEntities()
{
}

bool EntityPosition(const char *entity_name, const char *id, CVector &pos, CVector &angles) {
    if (got_world_import == false) return false;

    //get the entity's index from the server.
    int index;
    if (world_import.EntityIndex == NULL) return false;
    if (world_import.EntityIndex(entity_name, id, index) == false) return false;
    if (index < 0 || index >= MAX_EDICTS) return false;

    //find the client entity at that index.
    centity_t *entity = &cl_entities[index];

    //we have to check to make check if this entity has ever been sent down to the client.
    //if not, then we must go back to the server to get the position of the entity.
    if (entity->serverframe == 0) {
        //the entity has never been sent down from the server.  go back to the server.
        CVector position, direction;
        if (world_import.GetEntityPosition == NULL) return false;
        if (world_import.GetEntityPosition(entity_name, id, position, direction) == false) return false;

        //copy the data over to our variables.
        pos = position;
        angles = direction;

        return true;
    }

    //get the lerp'd info of the entity.
    pos = entity->lerp_origin;
    angles = entity->lerp_angles;

    return true;
}

bool EntityDone(const char *entity_name, const char *id) {
    if (got_world_import == false) return true;

    //ask world.dll if the entity's queue is empty.
    if (world_import.DoneWithTasks == NULL) return true;
    return world_import.DoneWithTasks(entity_name, id);
}

void TeleportEntity(const char *entity_name, const char *id, CVector &pos, CVector &angles, bool use_angles) {
    if (entity_name == NULL) return;
    if (got_world_import == false) return;

    //get the entity's index from the server.
    int index;
    if (world_import.EntityIndex == NULL) return;
    if (world_import.EntityIndex(entity_name, id, index) == false) return;
    if (index < 0 || index >= MAX_EDICTS) return;

    //find the client entity at that index.
    centity_t *entity = &cl_entities[index];

    //set the position (current, previous, and lerped)
	entity->current.origin		= pos;
	entity->current.old_origin	= pos;
	entity->prev.origin			= pos;
	entity->prev.old_origin		= pos;
	entity->lerp_origin			= pos;

    if (use_angles == true) 
	{
        //set the angles (current, previous, and lerped)
		entity->current.angles = angles;
		entity->prev.angles = angles;
		entity->lerp_angles = angles;
    }

    //move the entity in world.dll.
    if (world_import.TeleportEntity == NULL) return;
    world_import.TeleportEntity(entity_name, id, pos, angles, use_angles);
}

void CinPlaybackHeadScript(const char *entity_name, const char *id, const CScriptEntityTask *task, int start_time, int cur_time) {
    if (entity_name == NULL || entity_name[0] == '\0') return;
    if (task == NULL) return;
    if (task->Type() != TT_HEAD_SCRIPT) return;

    //get the head script
    const CEntityHeadScript *headscript = task->HeadScript();
    if (headscript == NULL) return;

    //compute the time offset we will use into our head script.
    float script_time = (cur_time - start_time) / 1000.0f;
    
    //get the angles from the head script at the computed time.
    CVector angle;
    headscript->Angle(script_time, angle);

    //set the entity's angle.
    if (world_import.SetEntityFacing == NULL) return;
    world_import.SetEntityFacing(entity_name, id, angle);
}

bool EntityAttributes(const char *entity_name, const char *id, entity_attributes &attributes) {
    if (entity_name == NULL) return false;
    if (got_world_import == false) return false;

    //make the call to world.dll
    if (world_import.EntityAttributes == NULL) return false;
    return world_import.EntityAttributes(entity_name, id, attributes);
}

cin_entity_type GetEntityType(const char *entity_name, const char *id) {
    if (entity_name == NULL) return CET_NONE;
    if (got_world_import == false) return CET_NONE;

    //make the call to world.dll
    if (world_import.GetEntityType == NULL) return CET_NONE;
    return world_import.GetEntityType(entity_name, id );
}

void SpawnHiroActor() {
    if (got_world_import == false) return;

    //get the local client's entity number.
    int entity_num = cl.playernum + 1;

    //send this entity number to the server so it can spawn the hiro actor.
    if (world_import.SpawnHiroActor == NULL) return;
    world_import.SpawnHiroActor(entity_num);
}

void RemoveHiroActor() {
    if (got_world_import == false) return;

    //tell the server to swap out our actor.
    if (world_import.RemoveHiroActor == NULL) return;
    world_import.RemoveHiroActor();
}

//
// functions that track the script playback fov.
//

//the current initial fov.
static float script_fov_initial = SCRIPT_FOV_DEFAULT;
//the current destination fov.
static float script_fov_dest = SCRIPT_FOV_DEFAULT;
//the fov that should be used now in the script.
static float script_fov_current = SCRIPT_FOV_DEFAULT;

//resets the fov tracking state.
void ScriptFOVInit(float fov) {
    bound_min(fov, SCRIPT_FOV_MIN);
    bound_max(fov, SCRIPT_FOV_MAX);

    script_fov_initial = fov;
    script_fov_dest = fov;
    script_fov_current = fov;
}

//sets a new destination fov, the previous destination fov is used as the new initial fov.
void ScriptFOVDest(float fov) {
    bound_min(fov, SCRIPT_FOV_MIN);
    bound_max(fov, SCRIPT_FOV_MAX);

    //move our current dest to be the new initial fov.
    script_fov_initial = script_fov_dest;

    //save the new dest fov.
    script_fov_dest = fov;

    //make the current fov the initial fov.
    script_fov_current = script_fov_initial;
}

//reports the current progress, from 0.0 to 1.0, of the fov animation.
//the new current fov is calculated.
void ScriptFOVProgress(float percent) {
    //bounds check the given percent.
    bound_min(percent, 0.0f);
    bound_max(percent, 1.0f);

    //compute the new current fov.
    script_fov_current = script_fov_initial + percent * (script_fov_dest - script_fov_initial);
}

//if a sequence segment has no start or end fov settings, it should call this function when it is started.
//Then its calls to ScriptFOVProgress will not change the current fov.
void ScriptFOVDestNone() {
    //set the current initial fov to the last dest, and leave the current dest.
    script_fov_initial = script_fov_dest;
}

float ScriptFOVCurrent() {
    return script_fov_current;
}


//
// functions that track the script playback timescale.
//

//the current initial timescale.
static float script_timescale_initial = SCRIPT_TIMESCALE_DEFAULT;
//the current destination timescale.
static float script_timescale_dest = SCRIPT_TIMESCALE_DEFAULT;
//the timescale that should be used now in the script.
static float script_timescale_current = SCRIPT_TIMESCALE_DEFAULT;

//resets the timescale tracking state.
void ScriptTimescaleInit(float timescale) {
    bound_min(timescale, SCRIPT_TIMESCALE_MIN);
    bound_max(timescale, SCRIPT_TIMESCALE_MAX);

    script_timescale_initial = timescale;
    script_timescale_dest = timescale;
    script_timescale_current = timescale;
}

//sets a new destination timescale, the previous destination timescale is used as the new initial timescale.
void ScriptTimescaleDest(float timescale) {
    bound_min(timescale, SCRIPT_TIMESCALE_MIN);
    bound_max(timescale, SCRIPT_TIMESCALE_MAX);

    //move our current dest to be the new initial timescale.
    script_timescale_initial = script_timescale_dest;

    //save the new dest timescale.
    script_timescale_dest = timescale;

    //make the current timescale the initial timescale.
    script_timescale_current = script_timescale_initial;
}

//reports the current progress, from 0.0 to 1.0, of the timescale animation.
//the new current timescale is calculated.
void ScriptTimescaleProgress(float percent) {
    //bounds check the given percent.
    bound_min(percent, 0.0f);
    bound_max(percent, 1.0f);

    //compute the new current timescale.
    script_timescale_current = script_timescale_initial + percent * (script_timescale_dest - script_timescale_initial);
}

//if a sequence segment has no start or end timescale settings, it should call this function when it is started.
//Then its calls to ScriptTimescaleProgress will not change the current timescale.
void ScriptTimescaleDestNone() {
    //set the current initial timescale to the last dest, and leave the current dest.
    script_timescale_initial = script_timescale_dest;
}

float ScriptTimescaleCurrent() {
    return script_timescale_current;
}


//
// functions that track the script playback fade color.
//

//the current initial color.
static float script_r_initial = SCRIPT_R_DEFAULT;
static float script_g_initial = SCRIPT_G_DEFAULT;
static float script_b_initial = SCRIPT_B_DEFAULT;
static float script_a_initial = SCRIPT_A_DEFAULT;

//the current destination color.
static float script_r_dest = SCRIPT_R_DEFAULT;
static float script_g_dest = SCRIPT_G_DEFAULT;
static float script_b_dest = SCRIPT_B_DEFAULT;
static float script_a_dest = SCRIPT_A_DEFAULT;
//the color that should be used now in the script.
static float script_r_current = SCRIPT_R_DEFAULT;
static float script_g_current = SCRIPT_G_DEFAULT;
static float script_b_current = SCRIPT_B_DEFAULT;
static float script_a_current = SCRIPT_A_DEFAULT;

//resets the color tracking state.
void ScriptColorInit(float r, float g, float b, float a) {
    bound_min(r, SCRIPT_COLOR_MIN);
    bound_max(r, SCRIPT_COLOR_MAX);

    bound_min(g, SCRIPT_COLOR_MIN);
    bound_max(g, SCRIPT_COLOR_MAX);

    bound_min(b, SCRIPT_COLOR_MIN);
    bound_max(b, SCRIPT_COLOR_MAX);

    bound_min(a, SCRIPT_COLOR_MIN);
    bound_max(a, SCRIPT_COLOR_MAX);

    script_r_initial = r;
    script_g_initial = g;
    script_b_initial = b;
    script_a_initial = a;
    
    script_r_dest = r;
    script_g_dest = g;
    script_b_dest = b;
    script_a_dest = a;
    
    script_r_current = r;
    script_g_current = g;
    script_b_current = b;
    script_a_current = a;
}

//sets a new destination color, the previous destination color is used as the new initial color.
void ScriptColorDest(float r, float g, float b, float a) {
    bound_min(r, SCRIPT_COLOR_MIN);
    bound_max(r, SCRIPT_COLOR_MAX);

    bound_min(g, SCRIPT_COLOR_MIN);
    bound_max(g, SCRIPT_COLOR_MAX);

    bound_min(b, SCRIPT_COLOR_MIN);
    bound_max(b, SCRIPT_COLOR_MAX);

    bound_min(a, SCRIPT_COLOR_MIN);
    bound_max(a, SCRIPT_COLOR_MAX);

    //move our current dest colors to be the new initial colors.
    script_r_initial = script_r_dest;
    script_g_initial = script_g_dest;
    script_b_initial = script_b_dest;
    script_a_initial = script_a_dest;

    //save the new dest colors.
    script_r_dest = r;
    script_g_dest = g;
    script_b_dest = b;
    script_a_dest = a;

    //make the current colors the initial colors.
    script_r_current = script_r_initial;
    script_g_current = script_g_initial;
    script_b_current = script_b_initial;
    script_a_current = script_a_initial;
}

//reports the current progress, from 0.0 to 1.0, of the color animation.
//the new current color is calculated.
void ScriptColorProgress(float percent) {
    //bounds check the given percent.
    bound_min(percent, 0.0f);
    bound_max(percent, 1.0f);

    //compute the new current color.
    script_r_current = script_r_initial + percent * (script_r_dest - script_r_initial);
    script_g_current = script_g_initial + percent * (script_g_dest - script_g_initial);
    script_b_current = script_b_initial + percent * (script_b_dest - script_b_initial);
    script_a_current = script_a_initial + percent * (script_a_dest - script_a_initial);
}

//if a sequence segment has no start or end color settings, it should call this function when it is started.
//Then its calls to ScriptColorProgress will not change the current color.
void ScriptColorDestNone() {
    //set the current initial color to the last dest, and leave the current dest.
    script_r_initial = script_r_dest;
    script_g_initial = script_g_dest;
    script_b_initial = script_b_dest;
    script_a_initial = script_a_dest;
}

float ScriptRCurrent() {
    return script_r_current;
}

float ScriptGCurrent() {
    return script_g_current;
}

float ScriptBCurrent() {
    return script_b_current;
}

float ScriptACurrent() {
    return script_a_current;
}

bool ScriptColorFading() {
    return ((script_r_current != SCRIPT_R_DEFAULT) ||
            (script_g_current != SCRIPT_G_DEFAULT) ||
            (script_b_current != SCRIPT_B_DEFAULT) ||
            (script_a_current != SCRIPT_A_DEFAULT));
}

//
// class playback functions
//
void CScriptEntity::PlaybackReset() {
    next_task = 0;

    saved_task_num = -1;
}

void CScriptShot::PlaybackReset() {
    next_sound = 0;
    wait_entity_done = false;
    wait_entity_done_time = 0;

    // set the specified sky
	char cmd[16];

	Com_sprintf(cmd, sizeof(cmd),"sky 0 0 %d\n", Sky());
    Cbuf_AddText(cmd);

    //initialize the fov.
    sequence->PlaybackReset();

    //reset all of our entities.
    for (int i = 0; i < entities.Num(); i++) {
        entities.Item(i)->PlaybackReset();
    }

    //check if we have an initial fov.
    if (have_start_fov == true) {
        //register the fov.
        ScriptFOVInit(StartFOV());
    }
    
    //check if we have an initial timescale.
    if (have_start_timescale == true) {
        //register the timescale.
        ScriptTimescaleInit(StartTimescale());
    }
    
    //check if we have an initial color.
    if (have_start_color == true) {
        //register the color.
        ScriptColorInit(StartR(), StartG(), StartB(), StartA());
    }
}

void CSplineSequence::PlaybackReset() {
    //reset playback_last_segment so that we know we haven't dealt with 
    //any of the segment playback vars.
    playback_last_segment = -1;
}

void CSplineSequence::Playback(int sequence_time) {
    //make sure the sequence time is positive.
    if (sequence_time < 0) {
        //we are in playback during the shot start pause.
        return;
    }

    //process the FOV information.
    //compute the time in seconds.
    float time_seconds = sequence_time / 1000.0f;

    //check the segment that the time falls in.
    int32 cur_segment = Segment(time_seconds);

    //process all segments after the last one processed, up to the current segment.
    for (int32 segment = playback_last_segment + 1; segment <= cur_segment; segment++) {
        //check if the segment has a start fov.
        if (HaveStartFOV(segment) == true) {
            //re-initialize the fov scripting module.
            ScriptFOVInit(StartFOV(segment));
        }

        //check if the segment has an end fov.
        if (HaveEndFOV(segment) == true) {
            //give the end fov to the fov scripting module as the destination fov.
            ScriptFOVDest(EndFOV(segment));
        }

        //check if the segment has no fov settings.
        if (HaveStartFOV(segment) == false && HaveEndFOV(segment) == false) {
            ScriptFOVDestNone();
        }
        
        //check if the segment has a start timescale.
        if (HaveStartTimescale(segment) == true) {
            //re-initialize the timescale scripting module.
            ScriptTimescaleInit(StartTimescale(segment));
        }

        //check if the segment has an end timescale.
        if (HaveEndTimescale(segment) == true) {
            //give the end timescale to the timescale scripting module as the destination timescale.
            ScriptTimescaleDest(EndTimescale(segment));
        }

        //check if the segment has no timescale settings.
        if (HaveStartTimescale(segment) == false && HaveEndTimescale(segment) == false) {
            ScriptTimescaleDestNone();
        }
        
        //check if the segment has a start color.
        if (HaveStartColor(segment) == true) {
            //re-initialize the color scripting module.
            ScriptColorInit(StartR(segment), StartG(segment), StartB(segment), StartA(segment));
        }

        //check if the segment has an end color.
        if (HaveEndColor(segment) == true) {
            //give the end color to the color scripting module as the destination color.
            ScriptColorDest(EndR(segment), EndG(segment), EndB(segment), EndA(segment));
        }

        //check if the segment has no color settings.
        if (HaveStartColor(segment) == false && HaveEndColor(segment) == false) {
            ScriptColorDestNone();
        }
    }

    //remember that we processed the current segment.
    playback_last_segment = cur_segment;

    //calculate the percentage that we have gone through the given segment.
    //time_seconds has been modified by Segment() to be the time offset within the current segment.
    float percent = time_seconds / Time(cur_segment);

    //give the percent to the fov scripting module.
    ScriptFOVProgress(percent);
    
    //give the percent to the timescale scripting module.
    ScriptTimescaleProgress(percent);
    
    //give the percent to the color scripting module.
    ScriptColorProgress(percent);
}


bool CanQueueTask(task_type type) {
    //cant queue teleports
    if (type == TT_TELEPORT) return false;

    //cant queue head scripts.
    if (type == TT_HEAD_SCRIPT) return false;

    //by default we can queue anything else.
    return true;
}

bool ExtendedPlaybackTask(task_type type) {
    //we have to step through every moment of head scripts.
    if (type == TT_HEAD_SCRIPT) return true;

    //all other tasks can be executed instantly.
    return false;
}

bool CinPlaybackTaskDone(const CScriptEntityTask *task, int start_time, int cur_time) {
    //get the task type.
    task_type type = task->Type();

    if (type != TT_HEAD_SCRIPT) return true;

    //get the head script from the task.
    const CEntityHeadScript *headscript = task->HeadScript();
    if (headscript == NULL) return true;

    //compute the time that the script will end.
    float script_done_time = start_time / 1000.0f + headscript->Length();

    //compare the computed time with the time passed in.
    if (script_done_time * 1000 < cur_time) {
        //the script is done.
        return true;
    }

    //we have not passed the ending time for the head script.
    return false;
}

void CScriptEntity::Playback(int end_time) {
    bound_min(next_task, 0);

    //go through the list of tasks and queue up each one
    for (;;) {
        //check if we are done with all the tasks.
        if (next_task >= tasks.Num()) break;

        //get the task.
        const CScriptEntityTask *task = tasks.Item(next_task);
        if (task == NULL) return;

        //check the time
        if (task->Time() * 1000 >= end_time) {
            //not time for this task.
            break;
        }

        //the time for the current task is upon us.

        //check if we can queue the task or if we must wait until the entity finishes it's current
        //tasks before executing this task.
        if (CanQueueTask(task->Type()) == true) {
            //if we haven't already, record the time we started this task.
            if (saved_task_num != next_task) {
                //remember the time that we finished this task, which is the time we start the next task.
                current_task_start_time = end_time;
                saved_task_num = next_task;
            }
            //queue this task.
            CinPlaybackEntityTask(name, task, current_task_start_time, end_time);
        }
        else {
            //check if the entity has tasks in its queue.
            if (EntityDone(name, uniqueid) == false) {
                //we must wait until the entity's task queue is empty.
                break;
            }

            //if we haven't already, record the time we started this task.
            if (saved_task_num != next_task) {
                //remember the time that we finished this task, which is the time we start the next task.
                current_task_start_time = end_time;
                saved_task_num = next_task;
            }

            //ok to execute this task now.
            CinPlaybackEntityTask(name, task, current_task_start_time, end_time);

            //check if this task type requires us to step through it's execution over time.
            if (ExtendedPlaybackTask(task->Type()) == true) {
                //check if we can go on to the next task.
                if (CinPlaybackTaskDone(task, current_task_start_time, end_time) == false) {
                    //we must keep running this task.
                    break;
                }
            }
        }

        //go to our next task.
        next_task++;
    }
}

bool CScriptEntity::Done() const {
    //check if we have queued all of our tasks yet.
    if (next_task < tasks.Num()) {
        //haven't even queued the tasks yet.
        return false;
    }

    //all the tasks are queued, find out if the entity has completed them.
    return EntityDone(name, uniqueid);
}

int CScriptShot::Playback(int end_time) {
    //look for events that should trigger in this time period.

    //play the sounds.
    for (;;) {
        //get the next sound we are going to play.
        const CScriptSound *sound = sounds.Item(next_sound);
        if (sound == NULL) break;

        //see if this sound is played after our end time.
        if ((sound->Time() * 1000) >= end_time) {
            //this sound is not up yet.
            break;
        }

        //we should play this sound.
        CinPlaybackSound(sound);

        //advance our sound number
        next_sound++;
    }

    //go through each of our entities.
    for (int ent = 0; ent < entities.Num(); ent++) {
        entities.Item(ent)->Playback(end_time);
    }

    //process the current fov in the sequence.
    sequence->Playback(end_time - extra_time_start);

    //check if the script is over.
    if (end_flag == SE_ENTITY) {
        //find the entity.
        const CScriptEntity *ent = entities.Find(entity_shot_end, CScriptEntity::Find);

        if (ent != NULL) {
            //the shot ends when the entity whose name we have is done with all of it's goals.
            //check if we already found out that the entity is done and are now waiting for the end wait time.
            if (wait_entity_done == true) {
                //the entity is done with it's tasks, check if the end wait time is done.
                if (end_time > wait_entity_done_time + extra_time_end * 1000) {
                    //the shot is over.
                    return 0;
                }

                //the shot is not over.
                return -1;
            }
            else {
                //check if the entity is done with it's tasks
                if (ent->Done() == true) {
                    //the entity finished.  
                    wait_entity_done = true;
                    wait_entity_done_time = end_time;

                    //check if we have extra wait time for the end.
                    if (extra_time_end <= 0.0f) {
                        //no extra time.
                        return 0;
                    }
                }

                return -1;
            }
        }
    }

    //either the shot ends when the time is up, or we dont have a valid entity to wait for.
    //get the ending time of the script based on the sequence time.
    int shot_time = (extra_time_start + sequence->TotalTime() + extra_time_end) * 1000;

    if (end_time > shot_time) {
        //its over.
        
        // stop any looping sound
        if (fLoopingASound)
            CinStopLoopingSound(&loopingSound);

        return end_time - shot_time;
    }

    //the shot is not over.
    return -1;
}

void YawPitch(CVector &dir, float &yaw, float &pitch) {
    //get the shadow of the vector.
    CVector shadow(dir.x, dir.y, 0.0f);

    //normalize the shadow.
    float len = shadow.Normalize();

    if (len < 0.001f) {
        //the vector is pointing almost straight up, or it is a zero vector.
        yaw = 0.0f;
        pitch = 90.0f;
        return;
    }

    //compute the yaw based on the normalized shadow vector.
    yaw = RAD2DEG * float(atan2(shadow.y, shadow.x));

    //compute the pitch based on the shadow length and the z of the dir.
    pitch = -RAD2DEG * float(atan2(dir.z, len));
}

void CScriptShot::PlaybackCamera(CVector &pos, CVector &dir, float time) {
    //if our sequence has 
    //compute the time we will give to the sequence
    float seq_time;

    if (time < extra_time_start) {
        //get the initial position of the sequence.
        seq_time = 0.0f;
    }
    else if (time < extra_time_start + sequence->TotalTime()) {
        seq_time = time - extra_time_start;
    }
    else {
        //get the ending position of the sequence.
        seq_time = sequence->TotalTime();
    }

    sequence->Pos(seq_time, pos);
    sequence->Dir(seq_time, dir);

    if (got_world_import == false) return;

    //check if we are supposed to look at an entity.
    if (target_flag == CT_ENTITY) {
        //find the entity's position.
        CVector entity, angles;
        if (EntityPosition(entity_camera_target, entity_camera_target, entity, angles) == false) return;

        //get the vector from the camera to the entity's location.
        CVector to_entity;
		to_entity = entity - pos;
		to_entity.Normalize();
        //to_entity.sub(entity, pos);
        //to_entity.normalize();

        //get a yaw/pitch from the vector.
        float yaw, pitch;
        YawPitch(to_entity, yaw, pitch);

        //put the yaw/pitch and a roll of 0 into dir.
        dir.x = pitch;
        dir.y = yaw;
        dir.z = 0.0f;
    }
}


void CCinematicScript::PlaybackReset(int start_shot, int end_shot) {
    cur_shot = start_shot;
    this->end_shot = end_shot;
    shot_time = 0;

    bound_min(cur_shot, 0);
    bound_max(cur_shot, NumShots() - 1);
    bound_min(this->end_shot, cur_shot);
    bound_max(this->end_shot, NumShots() - 1);

    //set the default fov
    if (have_camera_info == true) {
        ScriptFOVInit(SCRIPT_FOV_DEFAULT);
    }

    //set the default timescale
    if (have_camera_info == true) {
        ScriptTimescaleInit(SCRIPT_TIMESCALE_DEFAULT);
    }

    //set the default color
    if (have_camera_info == true) {
        ScriptColorInit(SCRIPT_R_DEFAULT, SCRIPT_G_DEFAULT, SCRIPT_B_DEFAULT, SCRIPT_A_DEFAULT);
    }

    //initialize the shot's playback variables.
    if (shots.Item(cur_shot) != NULL) {
        shots.Item(cur_shot)->PlaybackReset();
    }
}

void CCinematicScript::PlaybackTimeElapsed(int ms) {
    if (cur_shot < 0 || cur_shot > end_shot) return;

    //add the time to our total for the shot so far.
    shot_time += ms;

    //tell the current shot to advance.
    ms = shots.Item(cur_shot)->Playback(shot_time);

    //check if we have any time remaining.
    if (ms >= 0) {
        //advance to the next shot.
        cur_shot++;

        shot_time = 0;

        //initialize the shot's playback variables.
        if (shots.Item(cur_shot) != NULL) {
            shots.Item(cur_shot)->PlaybackReset();
        }

        PlaybackTimeElapsed(ms);
    }
}

bool CCinematicScript::PlaybackDone() {
    if (cur_shot > end_shot) return true;

    return false;
}

void CCinematicScript::PlaybackCamera(CVector &pos, CVector &dir) {
    if (cur_shot < 0 || cur_shot > end_shot) return;

    float time = shot_time * 0.001f;

    //ask the current shot for the camera info.
    shots.Item(cur_shot)->PlaybackCamera(pos, dir, time);
}

bool CCinematicScript::PlaybackMovesClient() {
    //go through each shot.
    for (int s = 0; s < NumShots(); s++) {
        //get the shot.
        const CScriptShot *shot = Shot(s);
        if (shot == NULL) continue;

        //get the entities in the shot.
        const array<CScriptEntity> &entities = shot->Entities();

        //look through all the entities.
        for (int i = 0; i < entities.Num(); i++) {
            //get the entity.
            const CScriptEntity *entity = entities.Item(i);
            if (entity == NULL) continue;

            //check the entity's name.
            if (stricmp(entity->Name(), "hiro") == 0) {
                //hiro is part of the script.
                return true;
            }
        }
    }

    //no shots control hiro.
    return false;
}

void CCinematicScript::RegsterCinematicSounds()
{
    for( int nShotNum = 0; nShotNum < NumShots(); nShotNum++ )
	{
        const CScriptShot *pShot = Shot( nShotNum );
		if( pShot == NULL )
		{
			continue;
		}

        const array<CScriptSound> &sounds = pShot->Sounds();
		for( int nSoundNum = 0; nSoundNum < sounds.Num(); nSoundNum++ )
		{
			const CScriptSound *pSound = sounds.Item( nSoundNum );
			if( pSound == NULL )
			{
				continue;
			}
			const char *pFileName = pSound->Name();
			if( ( pFileName[0] != NULL ) && ( strstr( pFileName, ".mp3" ) == NULL ) )
			{
				S_RegisterSound( pSound->Name() );
			}
		}
	}

	S_EndRegistration( 0 );
}

bool CCinematicScript::HaveCameraInfo() const {
    return have_camera_info;    
}
