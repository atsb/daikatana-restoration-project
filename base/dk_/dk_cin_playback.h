// #pragma once

#ifndef _DK_CIN_PLAYBACK_H_
#define _DK_CIN_PLAYBACK_H_

//
//functions used by playback and gce.dll to get information they need from
//the server.
//

bool EntityPosition(const char *entity_name, const char *id, CVector &pos, CVector &angles);

void TeleportEntity(const char *entity_name, const char *id, CVector &pos, CVector &angles, bool use_angles);

void SpawnHiroActor();
void RemoveHiroActor();

//returns the current horizontal fov that should be used in the current point in the script.
float ScriptFOVCurrent();

float ScriptTimescaleCurrent();

bool ScriptColorFading();
float ScriptRCurrent();
float ScriptGCurrent();
float ScriptBCurrent();
float ScriptACurrent();

#endif // _DK_CIN_PLAYBACK_H_
