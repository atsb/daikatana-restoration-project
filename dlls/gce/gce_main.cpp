//  dll interface crap
#include "dk_dll.h"
#include "p_user.h"

#include "dk_cin_types.h"
#include "dk_gce_interface.h"
#include "dk_point.h"

#include "gce_main.h"
#include "gce_sequence.h"
#include "gce_hud.h"
#include "gce_input.h"
#include "gce_script.h"

#include "dk_gce_spline.h"
#include "dk_gce_script.h"

extern "C" __declspec(dllexport) int dll_Entry(HINSTANCE hParent, DWORD dwReasonForCall, PVOID pvData);

char	*dll_Description = "GCE.DLL.  Game Cinematic Editor.\n";

//the server state.
serverState_t *server = NULL;
cin_gce_export_t *cin = NULL;
memory_import_t	memmgr;

///////////////////////////////////////////////////////////////////////////////
//  DllMain
//
//  standard DLL startup/exit routine.  We don't have to do anything in here
//  because Daikatana calls dll_Entry at appropriate times.
///////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain (HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}

cin_gce_import_t::cin_gce_import_t() {
    GCE_HUD = NULL;
    HUDActive = NULL;
    AddEntities = NULL;
    InputActive = NULL;
    InputKeyEvent = NULL;
    InputMouseMove = NULL;
    SetCamera = NULL;
    FastForwardScript = NULL;
}

void dll_ServerLoad(serverState_t *state) {
    server = state;

	if (server->GetCvar ("dedicated"))
		return;

    //register our commands for script editing.
    server->AddCommand("gce_hello", gce_hello);

    server->AddCommand("gce_save_position", gce_save_position);
    server->AddCommand("gce_seq_select_segment", gce_seq_select_segment);
    server->AddCommand("gce_seq_select_point", gce_seq_select_point);
    server->AddCommand("gce_seq_edit_point_loc", gce_seq_edit_point_loc);
    server->AddCommand("gce_seq_edit_point_angles", gce_seq_edit_point_angles);
    server->AddCommand("gce_seq_time", gce_seq_time);
    server->AddCommand("gce_seq_start_fov", gce_seq_start_fov);
    server->AddCommand("gce_seq_end_fov", gce_seq_end_fov);
    server->AddCommand("gce_seq_have_start_fov_toggle", gce_seq_have_start_fov_toggle);
    server->AddCommand("gce_seq_have_end_fov_toggle", gce_seq_have_end_fov_toggle);
    server->AddCommand("gce_seq_start_timescale", gce_seq_start_timescale);
    server->AddCommand("gce_seq_end_timescale", gce_seq_end_timescale);
    server->AddCommand("gce_seq_have_start_timescale_toggle", gce_seq_have_start_timescale_toggle);
    server->AddCommand("gce_seq_have_end_timescale_toggle", gce_seq_have_end_timescale_toggle);
    server->AddCommand("gce_seq_have_start_color_toggle", gce_seq_have_start_color_toggle);
    server->AddCommand("gce_seq_have_end_color_toggle", gce_seq_have_end_color_toggle);
    server->AddCommand("gce_seq_start_color", gce_seq_start_color);
    server->AddCommand("gce_seq_end_color", gce_seq_end_color);
    server->AddCommand("gce_seq_vel_start", gce_seq_vel_start);
    server->AddCommand("gce_seq_vel_end", gce_seq_vel_end);
    server->AddCommand("gce_seq_save", gce_seq_save);
    server->AddCommand("gce_seq_edit_point_save", gce_seq_edit_point_save);
    server->AddCommand("gce_seq_edit_point_abort", gce_seq_edit_point_abort);
    server->AddCommand("gce_seq_delete_point", gce_seq_delete_point);

    server->AddCommand("gce_script_name", gce_script_name);
    server->AddCommand("gce_script_load", gce_script_load);
    server->AddCommand("gce_script_save", gce_script_save);
    server->AddCommand("gce_script_add_shot", gce_script_add_shot);
    server->AddCommand("gce_script_edit_camera", gce_script_edit_camera);
    server->AddCommand("gce_script_edit_sounds", gce_script_edit_sounds);
    server->AddCommand("gce_script_select_shot", gce_script_select_shot);
    server->AddCommand("gce_script_shot_sky", gce_script_shot_sky);
    server->AddCommand("gce_script_shot_time_start", gce_script_shot_time_start);
    server->AddCommand("gce_script_shot_have_start_fov_toggle", gce_script_shot_have_start_fov_toggle);
    server->AddCommand("gce_script_shot_time_end", gce_script_shot_time_end);
    server->AddCommand("gce_script_shot_start_fov", gce_script_shot_start_fov);
    server->AddCommand("gce_script_shot_delete_selected", gce_script_shot_delete_selected);
    server->AddCommand("gce_script_shot_insert", gce_script_shot_insert);
    server->AddCommand("gce_script_shot_camera_target", gce_script_shot_camera_target);
    server->AddCommand("gce_script_shot_camera_target_entity", gce_script_shot_camera_target_entity);
    server->AddCommand("gce_script_shot_end_flag", gce_script_shot_end_flag);
    server->AddCommand("gce_script_shot_end_entity", gce_script_shot_end_entity);
    server->AddCommand("gce_script_edit_entities", gce_script_edit_entities);
    server->AddCommand("gce_script_cancel_shot", gce_script_cancel_shot);
    server->AddCommand("gce_script_play_shot", gce_script_play_shot);
    server->AddCommand("gce_script_play_all_shots", gce_script_play_all_shots);
    server->AddCommand("gce_script_play_from_current_shot", gce_script_play_from_current_shot);
    server->AddCommand("gce_script_copy_shot", gce_script_copy_shot);
    server->AddCommand("gce_script_paste_shot", gce_script_paste_shot);

    server->AddCommand("gce_sound_save", gce_sound_save);
    server->AddCommand("gce_sound_add", gce_sound_add);
    server->AddCommand("gce_sound_delete", gce_sound_delete);
    server->AddCommand("gce_sound_name", gce_sound_name);
    server->AddCommand("gce_sound_loop", gce_sound_loop);
    server->AddCommand("gce_sound_time", gce_sound_time);
    server->AddCommand("gce_sound_channel", gce_sound_channel);

    server->AddCommand("gce_entities_save", gce_entities_save);
    server->AddCommand("gce_entities_add", gce_entities_add);
    server->AddCommand("gce_entities_delete", gce_entities_delete);
    server->AddCommand("gce_entities_add_task", gce_entities_add_task);
    server->AddCommand("gce_entities_task_time", gce_entities_task_time);
    server->AddCommand("gce_entities_task_insert", gce_entities_task_insert);
    server->AddCommand("gce_entities_task_delete", gce_entities_task_delete);
    server->AddCommand("gce_entities_task_make_moveto", gce_entities_task_make_moveto);
    server->AddCommand("gce_entities_task_make_teleport", gce_entities_task_make_teleport);
    server->AddCommand("gce_entities_task_make_headscript", gce_entities_task_make_headscript);
    server->AddCommand("gce_entities_task_make_play_animation", gce_entities_task_make_play_animation);
    server->AddCommand("gce_entities_task_make_set_idle_animation", gce_entities_task_make_set_idle_animation);
    server->AddCommand("gce_entities_task_make_play_sound", gce_entities_task_make_play_sound);
    server->AddCommand("gce_entities_task_make_turn", gce_entities_task_make_turn);
    server->AddCommand("gce_entities_task_make_moveandturn", gce_entities_task_make_moveandturn);
    server->AddCommand("gce_entities_task_make_wait", gce_entities_task_make_wait);
    server->AddCommand("gce_entities_task_make_attribute_backup", gce_entities_task_make_attribute_backup);
    server->AddCommand("gce_entities_task_make_attribute_restore", gce_entities_task_make_attribute_restore);
    server->AddCommand("gce_entities_task_make_attribute_yawspeed", gce_entities_task_make_attribute_yawspeed);
    server->AddCommand("gce_entities_task_make_attribute_runspeed", gce_entities_task_make_attribute_runspeed);
    server->AddCommand("gce_entities_task_make_attribute_walkspeed", gce_entities_task_make_attribute_walkspeed);
    server->AddCommand("gce_entities_task_make_start_running", gce_entities_task_make_start_running);
    server->AddCommand("gce_entities_task_make_start_walking", gce_entities_task_make_start_walking);
    server->AddCommand("gce_entities_task_attribute", gce_entities_task_attribute);
    server->AddCommand("gce_entities_task_make_trigger_brush_use", gce_entities_task_make_trigger_brush_use);
    server->AddCommand("gce_entities_task_make_save", gce_entities_task_make_save);
    server->AddCommand("gce_entities_task_make_spawn_entity", gce_entities_task_make_spawn_entity);
    server->AddCommand("gce_entities_task_make_remove_entity", gce_entities_task_make_remove_entity);
    server->AddCommand("gce_entities_set_entity_id", gce_entities_set_entity_id);
    server->AddCommand("gce_entities_set_entity_name", gce_entities_set_entity_name);
    server->AddCommand("gce_entities_task_make_clear_goals", gce_entities_task_make_clear_goals);


    server->AddCommand("gce_position_save", gce_position_save);
    server->AddCommand("gce_position_abort", gce_position_abort);

    server->AddCommand("gce_headscript_save", gce_headscript_save);
    server->AddCommand("gce_headscript_start", gce_headscript_start);
    server->AddCommand("gce_headscript_abort", gce_headscript_abort);

    server->AddCommand("gce_hud", gce_hud);
    server->AddCommand("gce_hud_field", gce_hud_field);
    server->AddCommand("gce_hud_cursor", gce_hud_cursor);

    server->AddCommand("gce_set_position", gce_set_position);
    server->AddCommand("gce_toggle_first_person_edit", gce_toggle_first_person_edit);

    //get the gce export functions.
    cin = (cin_gce_export_t *)server->CIN_GCE_Interface();

    //check the version number of the exe's gce export.
    if (cin->GCE_Version() != sizeof(CSplineSequence) + sizeof(CCinematicScript)) {
        //bad cin.
        cin = NULL;
        return;
    }

	memmgr.X_Malloc = cin->X_Malloc;
	memmgr.X_Free = cin->X_Free;

    //make an import structure to give to the exe.
    cin_gce_import_t import;
    import.GCE_HUD = GCE_HUD;
    import.HUDActive = HUDActive;
    import.AddEntities = AddEntities;
    import.InputActive = InputActive;
    import.InputKeyEvent = InputKeyEvent;
    import.InputMouseMove = InputMouseMove;
    import.SetCamera = SetCamera;
    import.FastForwardScript = FastForwardScript;

    //send the import stuct to the exe.
    cin->GCE_Import(import);
}

///////////////////////////////////////////////////////////////////////////////
// dll_Version
//
// returns TRUE if size == IONSTORM_DLL_INTERFACE_VERSION
///////////////////////////////////////////////////////////////////////////////

int dll_Version(int size) {
    if (size == IONSTORM_DLL_INTERFACE_VERSION) {
        return  TRUE;
    }
    else {
        return  FALSE;
    }
}

void dll_LevelLoad() {

}

void dll_ServerInit() 
{
//	g_cvarLogOutput = server->cvar( "LogOutput", "1", CVAR_ARCHIVE );

//	cvar_t* cvarLogFilePath;
//	cvarLogFilePath = server->cvar( "LogFilePath", "", CVAR_ARCHIVE );
//	DKLOG_Initialize( cvarLogFilePath->string );

//	DKLOG_Write( LOGTYPE_SCRIPT, 0.0, "dll_ServerInit()" );
}

void dll_ServerKill() {
	if (server->GetCvar ("dedicated"))
		return;

    if (cin == NULL) return;

    //give the exe NULL pointers to all the gce functions so it doesnt try to call them after the dll is unloaded.
    cin_gce_import_t import;

    cin->GCE_Import(import);

    //DKLOG_Close();
}

void dll_LevelExit() 
{
}

int dll_Entry(HINSTANCE hParent, DWORD dwReasonForCall, PVOID pvData) {
    int size;

    switch (dwReasonForCall) {
        case QDLL_VERSION:
            size = *(int *)pvData;
            return  dll_Version (size);

        case QDLL_QUERY:
            *(userEpair_t **)pvData = NULL;
            return  TRUE;

        case QDLL_DESCRIPTION:
            *(char **)pvData = dll_Description;
            return  TRUE;

        case QDLL_SERVER_INIT:
            dll_ServerInit ();
            return  TRUE;

        case QDLL_SERVER_KILL:
            dll_ServerKill ();
            return  TRUE;

        case QDLL_SERVER_LOAD:
            dll_ServerLoad ((serverState_t *) pvData);
            return  TRUE;

        case QDLL_LEVEL_LOAD:
            dll_LevelLoad ();
            return  TRUE;

        case QDLL_LEVEL_EXIT:
            dll_LevelExit ();
            return  TRUE;
    }

    return  FALSE;
}





