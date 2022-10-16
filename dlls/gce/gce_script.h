#pragma once

#include "dk_array.h"

class CScriptSound;
class CCinematicScript;
class CScriptEntity;

//returns a pointer to the script being edited.
const CCinematicScript *ScriptCurrent();
//returns the index of the shot that is selected in the current script.
int ScriptCurrentShot();

//called when the sequence selected into the sequence module has been modified.
void ScriptSequenceModified(const CSplineSequence *sequence);
//called when the sound module is done modifying it's array of sounds.
void ScriptSoundModified(const array<CScriptSound> &sounds);
//called when the entities module is done modifying it's array of entities.
void ScriptEntitiesModified(const array<CScriptEntity> &entities);


//fast forwards the current shot to the given shot number.
void FastForwardScript(int shot_num);


//
//command handlers
//
void ScriptSetName();

void ScriptLoad();
void ScriptSave();

void ScriptAddShot();
void ScriptEditCamera();
void ScriptEditSounds();
void ScriptEditEntities();

void ScriptSelectShot();

void ScriptShotSky();
void ScriptShotExtraTimeStart();
void ScriptShotExtraTimeEnd();
void ScriptShotStartFOV();
void ScriptShotHaveStartFOVToggle();
void ScriptShotDeleteSelected();
void ScriptShotInsert();
void ScriptShotCameraTarget();
void ScriptShotCameraTargetEntity();
void ScriptShotEndFlag();
void ScriptShotEndEntity();
void ScriptPlayCurrentShot();
void ScriptPlayCancelShot();
void ScriptPlayAllShots();
void ScriptPlayFromCurrentShot();
void ScriptCopyShot();
void ScriptPasteShot();



