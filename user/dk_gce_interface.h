//#pragma once

#ifndef _DK_GCE_INTERFACE_H
#define _DK_GCE_INTERFACE_H

class CVector;
class CSplineSequence;
class CCinematicScript;
class cin_gce_import_t;
class cin_gce_export_t;
class entity_attributes;

#include "ref.h"

//the latest version of the script save files.
#define SCRIPT_FILE_VERSION 15
//changes made for each version
//
//14- Added sky index per shot
//13- Added sound duration for each queued sound
//12- Added playing of sounds to entity task queue
//11- Added timescale support and sound looping
//10- Added FOV vars to CScriptShot.
//9 - Created CSplineSequenceSegmentData object to hold info about each segment in the spline sequence.
//8 - Added animation name to entity task object.
//7 - Added support for headscripts by adding headscript memeber to task class.
//6 - Rearranged spline data into separate CSplineSequenceComponent objects for position and angle data.
//5 - Added attribute value to CScriptEntityTask
//4 - fixed task destination points to remove camera offset.
//3 - Added entity name fields for camera target and shot completion data to CScriptShot
//2 - Added entity array to CScriptShot
//1 - Added sound array to CScriptShot



//this structure is used to export functions from the exe to gce.dll
//we keep this structure separate so that we do not have to have our
//types included where the rest of the files would need to include
//our header files.

class cin_gce_export_t {
  public:
    //returns a version number
    int (*GCE_Version)();

    //gives the exe the import structure with function that can be called in gce.dll
    void (*GCE_Import)(cin_gce_import_t &import);

    //gets the last position and angles of the camera.
    void (*LastCameraPos)(CVector &pos, CVector &angles);

    //the playback module in the exe will copy the given script and use
    //it in subsequent playback operations.
    void (*UpdateScript)(const CCinematicScript &sequence);

    //draws text on the screen
    void (*DrawString)(int x, int y, const char *str);

    //adds an entity to the client's entity list.
    void (*V_AddEntity)(entity_t *ent);

    refexport_t *(*GetRefExport)();

    //loads a script, using the pack file routines.  The returned script
    //should not be modified in any way in the dll, and should be passed to 
    //DeleteScript when it is no longer needed.
    const CCinematicScript *(*LoadScript)(const char *name);
    void (*DeleteScript)(const CCinematicScript *script);

    //computes vectors that correspond to the given angles.
    void (*AngleVectors)(CVector &angles, CVector &forward, CVector &right, CVector &up);

    //for accessing information about entities.
    bool (*EntityPosition)(const char *entity_name, const char *id, CVector &location, CVector &angles);

    //for moving a monster type entity.
    void (*TeleportEntity)(const char *entity_name, const char *id, CVector &location, CVector &angles, bool use_angles);

    //gets the entity's modifyable attributes.
    bool (*EntityAttributes)(const char *entity_name, const char *id, entity_attributes &attributes);

    //gets the type of the script entity.
    cin_entity_type (*GetEntityType)(const char *entity_name, const char *id);

    // gets the dimensions of the current window
    void (*GetVidDims)(float& width, float& height);

	refdef_t *(*GetRefDef)();
	
	void (*WorldToScreen)( float *worldpos, int32 *sx, int32 *sy );

	beam_t		*(*beam_AllocBeam) (void);
	beamSeg_t	*(*beam_AllocSeg) (void);
	int			(*beam_AddGCESplines) (entity_t *entList, int numEnts, int selectedSeg, float r=1.0f, float g=1.0f, float b=1.0f);

	// SCG[4/8/99]: Added for cine scriptor functionality
	int *(*Key_KeybindingToKeynum) (char *);
	int (*Key_IsDown) (int);

#ifdef DEBUG_MEMORY
	void*	(*X_Malloc) (size_t size, MEM_TAG tag, char* file, int line);
#else
	void*	(*X_Malloc) (size_t size, MEM_TAG tag);
#endif
	void	(*X_Free) (void* mem);
};

//this structure is used by gce.dll to pass a set of function pointers back to the exe.
class cin_gce_import_t {
  public:
    cin_gce_import_t();

    //called after the world draws, lets the dll draw text and 2-d graphics to the screen.
    void (*GCE_HUD)();

    //returns true if the hud is being displayed.
    bool (*HUDActive)();

    void (*AddEntities)();

    //returns 1 is we have a currently active edit field that wants our keyboard input.
    int (*InputActive)();

    //returns 1 if the dll made use of the keystroke.
    int (*InputKeyEvent)(int keydown, int keyup, bool shift_down);

    //returns 1 if the dll made use of the mouse movement info.
    int (*InputMouseMove)(int dx, int dy);

    //gives the dll a chance to control the camera position/angles.
    void (*SetCamera)(CVector &pos, CVector &angles);

    //fast forwards the current script to the beginning of the given shot number
    void (*FastForwardScript)(int shot_num);
};

#endif // _DK_GCE_INTERFACE_H