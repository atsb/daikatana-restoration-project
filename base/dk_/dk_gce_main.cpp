#include "ref.h"

#include "dk_misc.h"
#include "dk_array.h"
#include "dk_point.h"

#include "dk_cin_types.h"
#include "dk_gce_main.h"
#include "dk_gce_spline.h"
#include "dk_gce_interface.h"
#include "dk_cin_world_interface.h"
#include "dk_cin_playback.h"
#include "dk_gce_script.h"
#include "dk_beams.h"
#include "dk_screen_coord.h"

#include "dk_shared.h"
#include "qcommon.h"
#include "client.h"

//our private export functions
int GCE_Version();
void GCE_Import(cin_gce_import_t &import);
void LastCameraPos(CVector &pos, CVector &angles);
void UpdateScript(const CCinematicScript &script);
void DrawString(int x, int y, const char *str);
refexport_t *GetRefExport();
const CCinematicScript *LoadScript(const char *name);
void DeleteScript(const CCinematicScript *script);
void GCE_AngleVectors(CVector &angles, CVector &forward, CVector &right, CVector &up);
bool EntityAttributes(const char *entity_name, const char *id, entity_attributes &attributes);
cin_entity_type GetEntityType(const char *entity_name, const char *id);
void GetVidDims(float& width, float& height);
int *Key_KeybindingToKeynum (char *str);
int Key_IsDown(int key);
refdef_t *GCE_RefDef();
void GCE_WorldToScreen( float *worldpos, int32 *sx, int32 *sy );


void CIN_Toggle();
void cin_play_shot();
void cin_play_all_shots();
void cin_play_from_current_shot();
void cin_cancel_shots();
void cin_load_and_play();

cvar_t *gce_set_position;

//our private interface to gce.dll
cin_gce_export_t gce_export;
cin_gce_import_t gce_import;

//our private interface to world.dll
cin_world_import_t world_import;
bool got_world_import = false;

void GCE_Init() {
    gce_export.GCE_Version = GCE_Version;
    gce_export.GCE_Import = GCE_Import;
    gce_export.LastCameraPos = LastCameraPos;
    gce_export.UpdateScript = UpdateScript;
    gce_export.DrawString = DrawString;
    gce_export.V_AddEntity = V_AddEntity;
    gce_export.GetRefExport = GetRefExport;
    gce_export.LoadScript = LoadScript;
    gce_export.DeleteScript = DeleteScript;
    gce_export.AngleVectors = GCE_AngleVectors;
    gce_export.EntityPosition = EntityPosition;
    gce_export.TeleportEntity = TeleportEntity;
    gce_export.EntityAttributes = EntityAttributes;
    gce_export.GetEntityType = GetEntityType;
    gce_export.GetVidDims = GetVidDims;
    gce_export.GetRefDef = GCE_RefDef;
    gce_export.WorldToScreen = GCE_WorldToScreen;

	gce_export.beam_AllocBeam = beam_AllocBeam;
	gce_export.beam_AllocSeg = beam_AllocSeg;
	gce_export.beam_AddGCESplines = beam_AddGCESplines;

	gce_export.Key_KeybindingToKeynum = Key_KeybindingToKeynum;
	gce_export.Key_IsDown = Key_IsDown;

	gce_export.X_Malloc = X_Malloc;
	gce_export.X_Free = X_Free;

    //register our commands for script playback.
    Cmd_AddCommand("cin_play", CIN_Toggle);
    Cmd_AddCommand("cin_play_shot", cin_play_shot);
    Cmd_AddCommand("cin_play_all_shots", cin_play_all_shots);
    Cmd_AddCommand("cin_play_from_current_shot", cin_play_from_current_shot);
    Cmd_AddCommand("cin_cancel_shots", cin_cancel_shots);

    Cmd_AddCommand("cin_load_and_play", cin_load_and_play);
}

cin_gce_import_t::cin_gce_import_t() {
    GCE_HUD = NULL;
    AddEntities = NULL;
    InputActive = NULL;
    InputKeyEvent = NULL;
    InputMouseMove = NULL;
    SetCamera = NULL;
}

refexport_t *GetRefExport() {
    return &re;
}

refdef_t *GCE_RefDef()
{
	return &cl.refdef;
}

void GCE_WorldToScreen( float *worldpos, int32 *sx, int32 *sy )
{
	ComputeScreenCoordinates(
		cl.refdef.width,
		cl.refdef.height,
		cl.refdef.x,
		cl.refdef.y,
		cl.refdef.fov_x,
		cl.refdef.fov_y,
		cl.refdef.vieworg,
		cl.refdef.viewangles,
		worldpos,
		sx,
		sy);
}

void *CIN_GCE_Interface() {
    return &gce_export;
}

void CIN_WorldPlayback(void *funcs) {
    //the pointer passed in is type cin_world_import_t
    world_import = *(cin_world_import_t *)funcs;

    got_world_import = true;
}

void GCE_Import(cin_gce_import_t &import) {
    gce_import = import;
}

void CIN_GCE_HUD() {
    if (gce_import.GCE_HUD != NULL) {
        try {
            gce_import.GCE_HUD();
        }
        catch (...) {
            gce_import.GCE_HUD = NULL;
        }
    }
}

void CIN_GCE_AddEntities() {
    if (gce_import.AddEntities != NULL) {
        try {
            gce_import.AddEntities();
        }
        catch (...) {
            gce_import.AddEntities = NULL;
        }
    }
}

//our script
CCinematicScript script;

static CVector gce_last_camera_pos;
static CVector gce_last_camera_angle;

//true if we are playing a script
bool playback_running = false;
//true if we have swapped out the player entity with the hiro actor bot.
bool playback_using_hiro_actor = false;
//true if a script that is currently playing controls the camera.
bool playback_running_with_camera = false;

static int	scripts_waiting;
static int	current_waiting_script;

// used to clear all the memory before unloading
void CIN_Reset()
{
	script.Reset();		
}

void CIN_Toggle() {
    if (CIN_Running() == 1) {
        CIN_Stop(NULL);
    }
    else {
        CIN_Start(NULL, 0, script.NumShots() - 1);
    }
}

void cin_play_shot() {
    if (GetArgc() != 2) return;

    //get the shot number passed in.
    int shot = atoi(GetArgv(1));
    if (shot < 0) return;

{buffer256 buffer("Playing shot %d\n", shot); OutputDebugStringA(buffer);}

    if (CIN_Running() == 1) {
        CIN_Stop(NULL);
    }
    else {
        //start the script.
        CIN_Start(NULL, shot, shot);

        //fast forward the script to the shot we are playing.
        if (gce_import.FastForwardScript != NULL) {
            gce_import.FastForwardScript(shot);
        }
    }
}

void cin_cancel_shots()
{
	CIN_Stop(NULL);
}

void cin_play_all_shots()
{
	scripts_waiting = atoi( GetArgv( 1 ) );
	current_waiting_script = 0;

	CIN_Start(NULL, 0, scripts_waiting );
}

void cin_play_from_current_shot()
{
    if (GetArgc() != 3) return;

    //get the shot number passed in.
    int shot = atoi( GetArgv( 1 ) );
    if( shot < 0 ) return;

	int num_shots = atoi( GetArgv( 2 ) );
    if( num_shots < 1 ) return;

	CIN_Start(NULL, shot, num_shots );
}

void cin_load_and_play() {
    //check our parameters.
    if (GetArgc() != 2) return;

    //get the script name passed in.
    const char *script_name = GetArgv(1);
    if (script_name == NULL || script_name[0] == '\0') return;

    const CCinematicScript *script = LoadScript(script_name);
    if (script == NULL) return;

    //make this script the current script.
    UpdateScript(*script);

    //delete the script we loaded.
    delete script;

    //start the playback.
    CIN_Start(script_name, 0, ::script.NumShots() - 1);
}

void GCE_ClientFrame() {
    //save the position/angles of the camera in case we need it.
    gce_last_camera_pos = cl.refdef.vieworg;
    gce_last_camera_angle = cl.refdef.viewangles;

    CIN_Play();
}

void GCE_SetCamera() {
    //check if we have a cinematic running.
    if (playback_running_with_camera == true) {
        CIN_CamLocation(cl.refdef.vieworg, cl.refdef.viewangles);

        //set the view settings.
        cl.refdef.fov_x = ScriptFOVCurrent();

/*
		if( world_import.SetClientFOV != NULL  )
		{
			world_import.SetClientFOV( ScriptFOVCurrent() );
		}
*/        
        // only override the existing values if we're actually doing something
        if (ScriptColorFading())
        {
            cl.refdef.blend[0] = ScriptRCurrent()/SCRIPT_COLOR_MAX;
            cl.refdef.blend[1] = ScriptGCurrent()/SCRIPT_COLOR_MAX;
            cl.refdef.blend[2] = ScriptBCurrent()/SCRIPT_COLOR_MAX;
            cl.refdef.blend[3] = ScriptACurrent()/SCRIPT_COLOR_MAX;
        }
        
        return;
    }

    //check if gce.dll wants to modify the camera for some reason.
    if (gce_import.SetCamera != NULL) {
        try {
            gce_import.SetCamera(cl.refdef.vieworg, cl.refdef.viewangles);
        }
        catch (...) {
            gce_import.SetCamera = NULL;
        }
    }
}

static int cin_last_tick;
static bool cin_changed_timescale;

void IN_DeactivateMouse ();
void IN_ActivateMouse ();
void ScriptColorInit(float r, float g, float b, float a);
void ScriptColorDest(float r, float g, float b, float a);
void SCR_TileClear();
extern float cl_gameover_time;
void CIN_Start(const char *name, int start_shot, int end_shot) {
    if (playback_running == true) return;

	if( cl_gameover_time > 0.0 )
	{
		return;
	}

//#ifdef _DEBUG
	cvar_t *sv_cinematics = Cvar_Get( "sv_cinematics", "0", 0 );

	if( sv_cinematics->value == 0 )
	{
		CIN_RemoveCinematicEntities();
		CIN_Stop( name );
		return;
	}
//#endif //_DEBUG
	cl.refdef.blend[0] = 0.0f;
	cl.refdef.blend[1] = 0.0f;
	cl.refdef.blend[2] = 0.0f;
	cl.refdef.blend[3] = 1.0f;

	IN_DeactivateMouse();

	// SCG[11/17/99]: set the server state
	if( world_import.SetServerState != NULL )
	{
		world_import.SetServerState( TRUE, name );
	}

{buffer256 buffer("Starting script playback\n"); OutputDebugStringA(buffer);}

    // Holster the current weapon so we don't hear its sounds playing
//	Cbuf_AddText("holster\n");

    cin_last_tick = GetTickCount();

    cin_changed_timescale = false;

    playback_running = true;

    script.PlaybackReset(start_shot, end_shot);

    //assume by default that we are not going to use the player actor.
    playback_using_hiro_actor = false;

    Cvar_SetValue("ai_scriptrunning", 1.0f);

    //check if the script has camera info.
    if (script.HaveCameraInfo() == true) 
	{
        //set our flag.
		playback_running_with_camera = true;

        //turn on our special camera mode.
        Cbuf_AddText("cam_toggle cin_on\n");

		SpawnHiroActor();
		playback_using_hiro_actor = true;
    }

	script.RegsterCinematicSounds();

	cl.frame.playerstate.rdflags |= RDF_CINESCREENHACK;
}

void CIN_RemoveCinematicEntities()
{
	if( world_import.RemoveAllEntities != NULL )
	{
		world_import.RemoveAllEntities();
	}
}

void CIN_StopAllSounds()
{
	for( int i = 2; i < 8; i++ )
	{
		S_StopMP3( i );
	}
}

void CIN_Stop(const char *name) {
    if (playback_running == false) return;

	IN_ActivateMouse();

	// SCG[11/17/99]: set the server state
	if( world_import.SetServerState != NULL )
	{
		world_import.SetServerState( FALSE, NULL );
	}

    //set our flag.
    playback_running = false;

    Cvar_SetValue("ai_scriptrunning", 0.0f);

	RemoveHiroActor();

/*
    //check if we had the hiro actor spawned.
    if (playback_using_hiro_actor == true) {
        //remove the actor and replace it with the normal client entity.
        RemoveHiroActor();
    }
*/

    //check if the script had camera info.
    if (playback_running_with_camera == true) {
        //turn off our special camera mode.
        Cbuf_AddText("cam_toggle cin_off\n");

        playback_running_with_camera = false;
    }

    // bring the selected weapon back out
//    Cbuf_AddText("holster\n");

    // reset the timescale if we changed it
    if (cin_changed_timescale)
        Cvar_SetValue("timescale", 1.0f);

	CIN_StopAllSounds();

	SCR_SubtitlePrint( NULL, 0.0 );
/*
	cvar_t	*fov = Cvar_Get( "fov", 0, 0 );
	float	fFOV = fov->value;

	if( world_import.SetClientFOV != NULL  )
	{
		world_import.SetClientFOV( fFOV );
	}
*/
}

int CIN_Running() {
    if (playback_running == true) { 
        return 1;
    }
    else {
        return 0;
    }
}

int CIN_RunningWithView() {
    //check if gce has it's hud up.
    if (gce_import.HUDActive != NULL) {
        try {
            if (gce_import.HUDActive() == true) return 1;
        }
        catch (...) {
            gce_import.HUDActive = NULL;
        }
    }

    //check if a cinematic is running at all.
    if (CIN_Running() == 0) return 0;

    //check if a cinematic is running that controls the camera.
    if (playback_running_with_camera == true) return 1;

    return 0;
}

//advances the positions of all active cinematic seqences.
void CIN_Play() {
extern	cvar_t	*cl_paused;
    //check if a cinematic is running.
    if (CIN_Running() == 0) return;

    //check if the game is paused.
//    static cvar_t *cl_paused = Cvar_Get("paused", "0", 1);
    if (cl_paused->value != 0) return;

    //get the current time.
    int cin_cur_tick = GetTickCount();

    //get the elapsed time.
    int time = (cin_cur_tick - cin_last_tick)*ScriptTimescaleCurrent();
    bound_max(time, 100);

    //send the time to the script.

    if (1!=ScriptTimescaleCurrent())
    {
        cin_changed_timescale = true;
        Cvar_SetValue("timescale", ScriptTimescaleCurrent());
    }

    script.PlaybackTimeElapsed(time);

    //remember the current tick for next time.
    cin_last_tick = cin_cur_tick;

    //check if the cinematic is over.
    if (script.PlaybackDone() == true) {
        CIN_Stop(NULL);
    }
}

void CIN_CamLocation(CVector &fpos, CVector &angles) {
    //true if we have ever set the camera.  If we have set the camera at least once,
    //we will always return the most recent camera position.
    static bool set_camera_once = false;

    //The last reported camera position and direction.
    //We make these static so that in case we try to get the location after the cinematic is
    //over we can return the most recent information.
    static CVector pos, dir;

    //check if we have never returned a camera position.
    if (set_camera_once == false) {
        //we have never returned a camera position, so if the current script doesn't have
        //camera info, dont return anything.
        if (playback_running_with_camera == false) {
            return;
        }
    }

    //either we have previously set the camera position, or we have not but have a script running
    //that has camera information available.

    //we are going to return a camera position.  Check if the current script has camera data.
    if (playback_running_with_camera == true) {
        //get the camera position from the script.
        script.PlaybackCamera(pos, dir);

        //remember that we returned a camera position.
        set_camera_once = true;
    }
    //else, use whatever camera data we most recently put into our static pos and dir.

    //copy our most recent camera data into the return value vectors.
    fpos = pos;
    angles = dir;
}

int GCE_Version() {
    return sizeof(CSplineSequence) + sizeof(CCinematicScript);
}

void LastCameraPos(CVector &pos, CVector &angles) {
    pos = gce_last_camera_pos;
    angles = gce_last_camera_angle;
}

void UpdateScript(const CCinematicScript &script) {
    //copy the data over to our sequence.
    ::script = script;
}

void DrawString(int x, int y, const char *str) {
    if (str == NULL) return;

	DRAWSTRUCT	drawStruct;

// changed by yokoyama
#ifdef	JPN	// JPN
		extern void *con_font;
		
		drawStruct.nFlags = DSFLAG_SCALE;
		drawStruct.szString = str;
		drawStruct.nXPos = x;
		drawStruct.nYPos = y;
		drawStruct.pImage = con_font;
		drawStruct.fScaleX = viddef.width / 640.0f;
		drawStruct.fScaleY = viddef.height / 480.0f;
		re.DrawString( drawStruct );
#else	// JPN

	drawStruct.nFlags = DSFLAG_BLEND;
	drawStruct.szString = str;
	drawStruct.nXPos = x;
	drawStruct.nYPos = y;
	drawStruct.pImage = NULL;
	re.DrawString( drawStruct );

#endif	// JPN
// Encompass MarkMa 040599
}

void GetVidDims(float& width, float& height)
{
    width = viddef.width;
    height = viddef.height;
}

int CIN_GCE_CurrentlyEditing() {
    if (gce_import.InputActive != NULL) {
        try {
            return gce_import.InputActive();
        }
        catch (...) {
            gce_import.InputActive = NULL;
        }
    }

    return 0;
}

int CIN_GCE_KeyEvent(int keydown, int keyup, int shift_down) {
    if (gce_import.InputKeyEvent != NULL) {
        try {
            return gce_import.InputKeyEvent(keydown, keyup, shift_down != 0);
        }
        catch (...) {
            gce_import.InputKeyEvent = NULL;
        }
    }

    return 0;
}

int CIN_GCE_MouseMove(int dx, int dy) {
    if (gce_import.InputMouseMove != NULL) {
        try {
            return gce_import.InputMouseMove(dx, dy);
        }
        catch (...) {
            gce_import.InputMouseMove = NULL;
        }
    }

    return 0;
}

void GCE_AngleVectors(CVector &angles, CVector &forward, CVector &right, CVector &up) {
    AngleToVectors(angles, forward, right, up);
}

///////////////////////////////////////////////////////////////////////////////
//	converted to use beamList
//	these routines just return NULL in gce.dll, which is why there is now
//	a dk_gce_spline and a gce_spline file.  That sucks, but I didn't see 
//	any other way without 
///////////////////////////////////////////////////////////////////////////////

//Makes as many entities as are needed to represent the sequence position splines.
//Uses the given function for allocating entities.  reference_entity is used
//to initialize data memebers in the entity that are not actually needed by the function
//to position the laser entities.  max_angle (in degrees) affects the smoothness of the curve by specifying
//the maximum allowed angle between adjacent entities.  max_length specifies the maximum length of
//any entity
int CSplineSequence::MakeCurveSegments (float max_angle, float max_length) 
{
	beam_t		*beam;
	beamSeg_t	*beamSeg, *lastSeg = NULL;
	CVector		beamNormal;

	beam = beam_AllocBeam ();
	if (beam == NULL)
		return	0;

	//	set up basic info for the beam
	beam->alpha			= 1.0;
	beam->startRadius	= 1.0;
	beam->endRadius		= 1.0;
	beam->texIndex		= BEAM_TEX_LASER;
	beam->texIndex2		= BEAM_TEX_NONE;
	beam->rgbColor.Set( 1.0, 1.0f, 1.0f );

    //the number of entities we have actually used to make the curve.
    int num_entities = 0;

    //check the parameters passed in.
    bound_min(max_angle, 1.0f);
    bound_max(max_angle, 80.0f);

    bound_min(max_length, 1.0f);

    //square the length.
    max_length = max_length * max_length;

    //compute the cos of the tolerance angle that was passed in.
    float angle_tolerance = cos(max_angle / 180.0f * PI);

    //move along the spline and construct lasers to approximate the path.
    //get the initial velocity direction.
    CVector begin_vel;
    CVector begin_pos;
    float begin_time = 0.0f;
    int begin_seg = 0;

    Pos(begin_time, begin_pos);
    Vel(begin_time, begin_vel);

	beam->beamStart = begin_pos;

    //compute the step length, based on the time of the current segment.
    float step = segment_data.Item(begin_seg).time / 50;
    bound_min(step, 0.01f);

    for (float time = 0.0f; time <= TotalTime(); time += step) {
        //get the position at this time.
        CVector pos;
        Pos(time, pos);

        //get the distance since the last position.
        CVector line;
        line = pos - begin_pos;
        float distance = line.DotProduct(line);

        //check if we have moved far enough to allow a new entity.
        if (distance < 1.0f) {
            //we havent move far enough in the world.
            continue;
        }

        //we use this copy of the current time because our call to Segment below will modify the value
        //we pass it.
        float segment_time = time;

        //get the segment number for the current time.
        int cur_seg = Segment(segment_time);

        //get the velocity vector at this time
        CVector vel;
        Vel(time, vel);

        //check if we should make a laser from the last point to the current point.
        if ((vel.DotProduct(begin_vel) > angle_tolerance) &&   //the spline is still on a straight course.
            (distance < max_length) &&                  //we haven't gone too far since the end of the last piece.
            (cur_seg == begin_seg))                     //we are still on the same segment.
        {
            //we should go farther before making the next laser.
            continue;
        }

        //the angles are far enough apart, make a marker.
        //get the begin and ending time we use to find positions from the spline.
        float time_start = begin_time;
        float time_end = time;
        if (time_end - time_start < 0.005f) {
            time_start = time_end - 0.005f;
        }

        //save the current data for next time.
        begin_pos = pos;
        begin_vel = vel;
        begin_time = time;
        begin_seg = cur_seg;

        //recompute our step.
        step = segment_data.Item(begin_seg).time / 50;
        bound_min(step, 0.01f);

        //	add a segment to the beam
        beamSeg = MakeCurveSegment (time_start, time_end, begin_seg, beam);
		if (beamSeg)
		{
			beam->numSegs++;
			if (lastSeg == NULL)
				//	first segment in beam list, so point list to it
				beam->firstSeg = beamSeg;
			else
				lastSeg->nextSeg = beamSeg;
			lastSeg = beamSeg;
		}

		num_entities++;
    }

    //get the end point of the spline.
    CVector end_pos;
    Pos(TotalTime(), end_pos);

    //get the distance since the end of the last laser.
    CVector line;
    line = end_pos - begin_pos;
    float distance = line.DotProduct(line);

	beam->beamEnd = end_pos;
	beamNormal = beam->beamEnd - beam->beamStart;
	beam->beamLength = beamNormal.Normalize();

    //if the last marker we placed was too far back, make one last marker.
    if (distance > 1.0f) {
        //make an entity from the previous time to the ending time of the sequence.
        beamSeg = MakeCurveSegment (begin_time, TotalTime(), begin_seg, beam);
		if (beamSeg)
		{
			beam->numSegs++;
			lastSeg->nextSeg = beamSeg;
		}

        num_entities++;
    }

    return num_entities;
}

beamSeg_t *CSplineSequence::MakeCurveSegment (float time_start, float time_end, int segment_num, beam_t *beam) 
{
	beamSeg_t	*beamSeg;

	beamSeg = beam_AllocSeg ();
	if (beamSeg == NULL)
		return	NULL;

    //get the initial and ending positions of the laser.
    CVector loc, endloc;
    Pos(time_start, loc);
    Pos(time_end, endloc);

    //position the entity.
    beamSeg->segStart = loc;
    beamSeg->segEnd = endloc;

	beamSeg->segNormal = beamSeg->segEnd - beamSeg->segStart;
	beamSeg->segNormal.Normalize();

	return	beamSeg;
}



