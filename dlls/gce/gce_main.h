#pragma once

extern cin_gce_export_t *cin;

//our pointer to the server state structure
extern serverState_t *server;

//
//commands that are called in this dll.
//

void gce_hello(struct edict_s *);

//saves the current camera position and angles to the end of the current sequence.
void gce_save_position(struct edict_s *);
void gce_seq_select_segment(struct edict_s *);
void gce_seq_select_point(struct edict_s *);
void gce_seq_time(struct edict_s *);
void gce_seq_have_start_fov_toggle(struct edict_s *);
void gce_seq_have_end_fov_toggle(struct edict_s *);
void gce_seq_start_fov(struct edict_s *);
void gce_seq_end_fov(struct edict_s *);
void gce_seq_have_start_timescale_toggle(struct edict_s *);
void gce_seq_have_end_timescale_toggle(struct edict_s *);
void gce_seq_start_timescale(struct edict_s *);
void gce_seq_end_timescale(struct edict_s *);
void gce_seq_have_start_color_toggle(struct edict_s *);
void gce_seq_have_end_color_toggle(struct edict_s *);
void gce_seq_start_color(struct edict_s *);
void gce_seq_end_color(struct edict_s *);
void gce_seq_edit_point_loc(struct edict_s *);
void gce_seq_edit_point_angles(struct edict_s *);
void gce_seq_vel_start(struct edict_s *);
void gce_seq_vel_end(struct edict_s *);
void gce_seq_save(struct edict_s *);
void gce_seq_edit_point_save(struct edict_s *);
void gce_seq_edit_point_abort(struct edict_s *);
void gce_seq_delete_point(struct edict_s *);


void gce_hud(struct edict_s *);
void gce_hud_field(struct edict_s *);
void gce_hud_cursor(struct edict_s *);


void gce_set_position(struct edict_s *);


void gce_script_name(struct edict_s *);
void gce_script_load(struct edict_s *);
void gce_script_save(struct edict_s *);
void gce_script_add_shot(struct edict_s *);
void gce_script_edit_camera(struct edict_s *);
void gce_script_edit_sounds(struct edict_s *);
void gce_script_select_shot(struct edict_s *);
void gce_script_shot_sky(struct edict_s *);
void gce_script_shot_time_start(struct edict_s *);
void gce_script_shot_time_end(struct edict_s *);
void gce_script_shot_start_fov(struct edict_s *);
void gce_script_shot_have_start_fov_toggle(struct edict_s *);
void gce_script_shot_delete_selected(struct edict_s *);
void gce_script_shot_insert(struct edict_s *);
void gce_script_shot_camera_target(struct edict_s *);
void gce_script_shot_camera_target_entity(struct edict_s *);
void gce_script_shot_end_flag(struct edict_s *);
void gce_script_shot_end_entity(struct edict_s *);
void gce_script_edit_entities(struct edict_s *);
void gce_script_play_shot(struct edict_s *);
void gce_script_cancel_shot(struct edict_s *);
void gce_script_play_all_shots(struct edict_s *);
void gce_script_play_from_current_shot(struct edict_s *);
void gce_script_copy_shot(struct edict_s *);
void gce_script_paste_shot(struct edict_s *);

void gce_sound_save(struct edict_s *);
void gce_sound_add(struct edict_s *);
void gce_sound_delete(struct edict_s *);
void gce_sound_name(struct edict_s *);
void gce_sound_loop(struct edict_s *);
void gce_sound_time(struct edict_s *);
void gce_sound_channel(struct edict_s *);

void gce_entities_save(struct edict_s *);
void gce_entities_add(struct edict_s *);
void gce_entities_delete(struct edict_s *);
void gce_entities_add_task(struct edict_s *);
void gce_entities_task_time(struct edict_s *);
void gce_entities_task_insert(struct edict_s *);
void gce_entities_task_delete(struct edict_s *);
void gce_entities_task_make_moveto(struct edict_s *);
void gce_entities_task_make_teleport(struct edict_s *);
void gce_entities_task_make_headscript(struct edict_s *);
void gce_entities_task_make_play_animation(struct edict_s *);
void gce_entities_task_make_set_idle_animation(struct edict_s *);
void gce_entities_task_make_play_sound(struct edict_s *);
void gce_entities_task_make_turn(struct edict_s *);
void gce_entities_task_make_moveandturn(struct edict_s *);
void gce_entities_task_make_wait(struct edict_s *);
void gce_entities_task_make_attribute_backup(struct edict_s *);
void gce_entities_task_make_attribute_restore(struct edict_s *);
void gce_entities_task_make_attribute_yawspeed(struct edict_s *);
void gce_entities_task_make_attribute_runspeed(struct edict_s *);
void gce_entities_task_make_attribute_walkspeed(struct edict_s *);
void gce_entities_task_make_start_running(struct edict_s *);
void gce_entities_task_make_start_walking(struct edict_s *);
void gce_entities_task_attribute(struct edict_s *);
void gce_entities_task_make_trigger_brush_use(struct edict_s *);
void gce_entities_task_make_save(struct edict_s *);
void gce_entities_task_make_abort(struct edict_s *);
void gce_entities_set_entity_id(struct edict_s *);
void gce_entities_set_entity_name(struct edict_s *);

void gce_entities_task_make_spawn_entity(struct edict_s *);
void gce_entities_task_make_remove_entity(struct edict_s *);
void gce_entities_task_make_clear_goals(struct edict_s *);

void gce_headscript_start(struct edict_s *);
void gce_headscript_save(struct edict_s *);
void gce_headscript_abort(struct edict_s *);

void gce_position_save(struct edict_s *);
void gce_position_abort(struct edict_s *);

void gce_toggle_first_person_edit(struct edict_s *);

//
// Import functions sent to the exe.
//

void GCE_HUD();
bool HUDActive();


//
//types that are used in multiple source files.
//

//colors
typedef enum {
    DC_WHITE,
    DC_GREY,
    DC_BLACK,
    DC_RED,
    DC_PEACH,
} draw_color;

typedef struct autoAim_s {
   float xofs,yofs;
   entity_state_t *ent;
   short item;
} autoAim_t;





