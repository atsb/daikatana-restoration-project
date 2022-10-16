#include "p_user.h"

#include "dk_array.h"

#include "dk_cin_types.h"
#include "dk_gce_interface.h"
#include "gce_main.h"
#include "dk_point.h"

#include "dk_gce_spline.h"

#include "gce_sequence.h"
#include "gce_hud.h"
#include "gce_script.h"
#include "gce_input.h"
#include "gce_sound.h"
#include "gce_entities.h"
#include "gce_position.h"
#include "gce_headscript.h"

void gce_hello(struct edict_s *) {
    server->Con_Printf("Hello from GCE.DLL\n");
}

//saves the current camera position and angles to the end of the current sequence.
void gce_save_position(struct edict_s *) {
    SequenceAddPoint();
}

void gce_seq_select_segment(struct edict_s *) {
    SequenceSelectSegment();
}

void gce_seq_select_point(struct edict_s *) {
    SequenceSelectPoint();
}

void gce_seq_time(struct edict_s *) {
    SequenceTime();
}

void gce_seq_have_start_fov_toggle(struct edict_s *) {
    SequenceHaveStartFOVToggle();
}

void gce_seq_have_end_fov_toggle(struct edict_s *) {
    SequenceHaveEndFOVToggle();
}

void gce_seq_start_fov(struct edict_s *) {
    SequenceStartFOV();
}

void gce_seq_end_fov(struct edict_s *) {
    SequenceEndFOV();
}

void gce_seq_have_start_timescale_toggle(struct edict_s *) {
    SequenceHaveStartTimescaleToggle();
}

void gce_seq_have_end_timescale_toggle(struct edict_s *) {
    SequenceHaveEndTimescaleToggle();
}

void gce_seq_start_timescale(struct edict_s *) {
    SequenceStartTimescale();
}

void gce_seq_end_timescale(struct edict_s *) {
    SequenceEndTimescale();
}


void gce_seq_have_start_color_toggle(struct edict_s *) {
    SequenceHaveStartColorToggle();
}

void gce_seq_have_end_color_toggle(struct edict_s *) {
    SequenceHaveEndColorToggle();
}

void gce_seq_start_color(struct edict_s *) {
    SequenceStartColor();
}

void gce_seq_end_color(struct edict_s *) {
    SequenceEndColor();
}

void gce_seq_edit_point_loc(struct edict_s *) {
    SequenceEditPointLoc();
}

void gce_seq_edit_point_angles(struct edict_s *) {
    SequenceEditPointAngles();
}

void gce_seq_vel_start(struct edict_s *) {
    SequenceVelStart();
}

void gce_seq_vel_end(struct edict_s *) {
    SequenceVelEnd();
}

void gce_seq_save(struct edict_s *) {
    SequenceSave();
}

void gce_seq_edit_point_save(struct edict_s *) {
    SequenceEditPointSave();
}

void gce_seq_edit_point_abort(struct edict_s *) {
    SequenceEditPointAbort();
}

void gce_seq_delete_point(struct edict_s *) {
    SequenceDeletePoint();
}

void gce_hud(struct edict_s *) {
    int argc = server->GetArgc();

    //check the arguments.
    if (argc == 2 && stricmp(server->GetArgv(1), "on") == 0) {
        GCE_HUD_On();
    }
    else if (argc == 2 && stricmp(server->GetArgv(1), "off") == 0) {
        GCE_HUD_Off();
    }
    else if (argc == 2 && stricmp(server->GetArgv(1), "toggle") == 0) {
        GCE_HUD_Toggle();
    }
    else if (argc == 1) {
        GCE_HUD_Toggle();
    }
    else if (argc == 2 && stricmp(server->GetArgv(1), "seq_show") == 0) {
        SequenceShow();
    }
    else if (argc == 2 && stricmp(server->GetArgv(1), "seq_hide") == 0) {
        SequenceHide();
    }
}

void gce_hud_field(struct edict_s *) {
    HUDFieldCycle();
}

void gce_hud_cursor(struct edict_s *) {
    HUDToggleCursor();
}

void gce_script_name(struct edict_s *) {
    ScriptSetName();
}

void gce_script_load(struct edict_s *) {
    ScriptLoad();
}

void gce_script_save(struct edict_s *) {
    ScriptSave();
}

void gce_script_add_shot(struct edict_s *) {
    ScriptAddShot();
}

void gce_script_edit_camera(struct edict_s *) {
    ScriptEditCamera();
}

void gce_script_edit_sounds(struct edict_s *) {
    ScriptEditSounds();
}

void gce_script_edit_entities(struct edict_s *) {
    ScriptEditEntities();
}

void gce_script_select_shot(struct edict_s *) {
    ScriptSelectShot();
}

void gce_script_shot_sky(struct edict_s *) {
    ScriptShotSky();
}

void gce_script_shot_time_start(struct edict_s *) {
    ScriptShotExtraTimeStart();
}

void gce_script_shot_time_end(struct edict_s *) {
    ScriptShotExtraTimeEnd();
}

void gce_script_shot_start_fov(struct edict_s *) {
    ScriptShotStartFOV();
}

void gce_script_shot_have_start_fov_toggle(struct edict_s *) {
    ScriptShotHaveStartFOVToggle();
}

void gce_script_shot_delete_selected(struct edict_s *) {
    ScriptShotDeleteSelected();
}

void gce_script_shot_insert(struct edict_s *) {
    ScriptShotInsert();
}

void gce_script_shot_camera_target(struct edict_s *) {
    ScriptShotCameraTarget();
}

void gce_script_shot_camera_target_entity(struct edict_s *) {
    ScriptShotCameraTargetEntity();
}

void gce_script_shot_end_flag(struct edict_s *) {
    ScriptShotEndFlag();
}

void gce_script_shot_end_entity(struct edict_s *) {
    ScriptShotEndEntity();
}

void gce_script_play_shot(struct edict_s *) {
    ScriptPlayCurrentShot();
}
void gce_script_cancel_shot(struct edict_s *) {
    ScriptPlayCancelShot();
}

void gce_script_play_all_shots(struct edict_s *) {
    ScriptPlayAllShots();
}

void gce_script_play_from_current_shot(struct edict_s *) {
    ScriptPlayFromCurrentShot();
}

void gce_script_copy_shot(struct edict_s *)
{
	ScriptCopyShot();
}

void gce_script_paste_shot(struct edict_s *)
{
	ScriptPasteShot();
}

void gce_sound_save(struct edict_s *) {
    SoundSave();
}

void gce_sound_add(struct edict_s *) {
    SoundAdd();
}

void gce_sound_delete(struct edict_s *) {
    SoundDelete();
}

void gce_sound_name(struct edict_s *) {
    SoundSetName();
}

void gce_sound_loop(struct edict_s *) {
    SoundSetLoop();
}

void gce_sound_time(struct edict_s *) {
    SoundSetTime();
}

void gce_sound_channel(struct edict_s *) {
    SoundSetChannel();
}

void gce_entities_save(struct edict_s *) {
    EntitiesSave();
}

void gce_entities_add(struct edict_s *) {
    EntitiesAdd();
}

void gce_entities_delete(struct edict_s *) {
    EntitiesDelete();
}

void gce_entities_add_task(struct edict_s *) {
    EntitiesAddTask();
}

void gce_entities_task_time(struct edict_s *) {
    EntitiesTaskTime();
}

void gce_entities_task_insert(struct edict_s *) {
    EntitiesTaskInsert();
}

void gce_entities_task_delete(struct edict_s *) {
    EntitiesTaskDelete();
}

void gce_entities_task_make_moveto(struct edict_s *) {
    EntitiesTaskMakeMoveTo();
}

void gce_entities_task_make_teleport(struct edict_s *) {
    EntitiesTaskMakeTeleport();
}

void gce_entities_task_make_headscript(struct edict_s *) {
    EntitiesTaskMakeHeadScript();
}

void gce_entities_task_make_play_animation(struct edict_s *) {
    EntitiesTaskMakePlayAnimation();
}

void gce_entities_task_make_set_idle_animation(struct edict_s *) {
    EntitiesTaskMakeSetIdleAnimation();
}

void gce_entities_task_make_play_sound(struct edict_s *) {
    EntitiesTaskMakePlaySound();
}

void gce_entities_task_make_turn(struct edict_s *) {
    EntitiesTaskMakeTurn();
}

void gce_entities_task_make_moveandturn(struct edict_s *) {
    EntitiesTaskMakeMoveAndTurn();
}

void gce_entities_task_make_wait(struct edict_s *) {
    EntitiesTaskMakeWait();
}

void gce_entities_task_make_attribute_backup(struct edict_s *) {
    EntitiesTaskMakeAttributeBackup();
}

void gce_entities_task_make_attribute_restore(struct edict_s *) {
    EntitiesTaskMakeAttributeRestore();
}

void gce_entities_task_make_attribute_yawspeed(struct edict_s *) {
    EntitiesTaskMakeAttributeYawSpeed();
}

void gce_entities_task_make_attribute_runspeed(struct edict_s *) {
    EntitiesTaskMakeAttributeRunSpeed();
}

void gce_entities_task_make_attribute_walkspeed(struct edict_s *) {
    EntitiesTaskMakeAttributeWalkSpeed();
}

void gce_entities_task_make_start_running(struct edict_s *) {
    EntitiesTaskMakeStartRunning();
}

void gce_entities_task_make_start_walking(struct edict_s *) {
    EntitiesTaskMakeStartWalking();
}

void gce_entities_task_attribute(struct edict_s *) {
    EntitiesTaskAttribute();
}

void gce_entities_set_entity_id(struct edict_s *) {
    EntitiesSetEntityID();
}

void gce_entities_set_entity_name(struct edict_s *) {
    EntitiesSetEntityName();
}

void gce_entities_task_make_trigger_brush_use(struct edict_s *) {
    EntitiesTaskTriggerBrushUse();
}

void gce_entities_task_make_save(struct edict_s *) {
    EntitiesTaskPositionSave();
}

void gce_entities_task_make_abort(struct edict_s *) {
    EntitiesTaskPositionAbort();
}

void gce_entities_task_make_spawn_entity(struct edict_s *) {
	EntitiesMakeSpawnEntity();
}

void gce_entities_task_make_remove_entity(struct edict_s *) {
	EntitiesMakeRemoveEntity();
}

void gce_entities_task_make_clear_goals(struct edict_s *) {
	EntitiesMakeClearGoals();
}

void gce_position_save(struct edict_s *) {
    PositionSave();
}

void gce_position_abort(struct edict_s *) {
    PositionAbort();
}

void gce_headscript_start(struct edict_s *) {
    HeadscriptStart();
}

void gce_headscript_save(struct edict_s *) {
    HeadscriptSave();
}

void gce_headscript_abort(struct edict_s *) {
    HeadscriptAbort();
}

void gce_toggle_first_person_edit(struct edict_s *) {
    bool val = PositionGetFirstPersonEdit();

    val = !val;

    PositionSetFirstPersonEdit(val);
    SequenceSetFirstPersonEdit(val);
}
