#pragma once

#include "dk_array.h"

class CScriptEntity;

//returns the array of entities we are editing..
const array<CScriptEntity> &EntitiesCurrent();

//copies the array of entities so they can be edited.
void EntitiesEdit(const array<CScriptEntity> &entities);


void EntitiesSave();
void EntitiesAddID();
void EntitiesAdd();
void EntitiesDelete();
void EntitiesAddTask();
void EntitiesTaskTime();
void EntitiesTaskInsert();
void EntitiesTaskDelete();
void EntitiesTaskMakeMoveTo();
void EntitiesTaskMakeTeleport();
void EntitiesTaskMakeHeadScript();
void EntitiesTaskMakePlayAnimation();
void EntitiesTaskMakeSetIdleAnimation();
void EntitiesTaskMakePlaySound();
void EntitiesTaskMakeTurn();
void EntitiesTaskMakeMoveAndTurn();
void EntitiesTaskMakeWait();
void EntitiesTaskMakeAttributeBackup();
void EntitiesTaskMakeAttributeRestore();
void EntitiesTaskMakeAttributeYawSpeed();
void EntitiesTaskMakeAttributeRunSpeed();
void EntitiesTaskMakeAttributeWalkSpeed();
void EntitiesTaskMakeStartRunning();
void EntitiesTaskMakeStartWalking();
void EntitiesTaskAttribute();
void EntitiesTaskTriggerBrushUse();
void EntitiesSetEntityID();
void EntitiesSetEntityName();

void EntitiesTaskPositionSave();
void EntitiesTaskPositionAbort();

void EntitiesTaskHeadScriptSave();
void EntitiesTaskHeadScriptAbort();

void EntitiesAddEntityPath(int);

void EntitiesMakeSpawnEntity();
void EntitiesMakeRemoveEntity();

void EntitiesMakeClearGoals();
