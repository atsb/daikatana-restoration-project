#ifndef _AI_THINK_H
#define _AI_THINK_H


void AI_StartIdle( userEntity_t *self );
void AI_StartWander( userEntity_t *self );
void AI_StartAttack( userEntity_t *self );
void AI_StartChase( userEntity_t *self );
void AI_StartChaseAttack( userEntity_t *self );
void AI_StartChaseAttackStrafing( userEntity_t *self );
void AI_StartFollowWalking( userEntity_t *self );
void AI_StartFollowRunning( userEntity_t *self );
void AI_StartFollowSidekickWalking( userEntity_t *self );
void AI_StartFollowSidekickRunning( userEntity_t *self );
void AI_StartHide( userEntity_t *self );
void AI_StartSnipe( userEntity_t *self );
void AI_StartRunAway( userEntity_t *self );
void AI_StartMoveAway( userEntity_t *self );
void AI_StartWaitForTrainToCome( userEntity_t *self );
void AI_StartWaitForTrainToStop( userEntity_t *self );
void AI_StartPatrol( userEntity_t *self );
void AI_StartMoveToLocation( userEntity_t *self );
void AI_StartMoveToExactLocation( userEntity_t *self );
void AI_StartWalkToLocation( userEntity_t *self );
void AI_StartGoingAroundObstacle( userEntity_t *self );
void AI_StartFlyToLocation( userEntity_t *self );
void AI_StartSwoopAttack( userEntity_t *self );
void AI_StartTowardIntermediatePoint( userEntity_t *self );
void AI_StartTowardIntermediatePoint2( userEntity_t *self );
void AI_StartAirRangedAttack( userEntity_t *self );
void AI_StartAirMeleeAttack( userEntity_t *self );
void AI_StartSwimToLocation( userEntity_t *self );
void AI_StartFaceAngle( userEntity_t *self );
void AI_StartWait( userEntity_t *self );
void AI_StartWaitForNoCollision( userEntity_t *self );
void AI_StartBackupAllAttributes( userEntity_t *self );
void AI_StartRestoreAllAttributes( userEntity_t *self );
void AI_StartModifyTurnAttribute( userEntity_t *self );
void AI_StartModifyRunAttribute( userEntity_t *self );
void AI_StartPlayAnimation( userEntity_t *self );
void AI_StartSetIdleAnimation( userEntity_t *self );
void AI_StartRandomWander( userEntity_t *self );
void AI_StartUsingWalkSpeed( userEntity_t *self );
void AI_StartUsingRunSpeed( userEntity_t *self );
void AI_StartActivateSwitch( userEntity_t *self );
void AI_StartMoveToEntity( userEntity_t *self );
void AI_StartMoveToOwner( userEntity_t *self );
void AI_StartStand( userEntity_t *self );
void AI_StartTakeCover( userEntity_t *self );
void AI_StartStay( userEntity_t *self );
void AI_StartFlyingAroundObstacle( userEntity_t *self );
void AI_StartPathFollow( userEntity_t *self );
void AI_StartDodge( userEntity_t *self );
void AI_StartSideStep( userEntity_t *self );
void AI_StartGoingAroundEntity( userEntity_t *self );
void AI_StartMoveToSnipe( userEntity_t *self );
void AI_StartStrafe( userEntity_t *self );
void AI_StartChaseEvade( userEntity_t *self );
void AI_StartTakeOff( userEntity_t *self );
void AI_StartDrop( userEntity_t *self );
void AI_StartChaseSideStepLeft( userEntity_t *self );
void AI_StartChaseSideStepRight( userEntity_t *self );
void AI_StartChargeTowardEnemy( userEntity_t *self );
void AI_StartRetreatToOwner( userEntity_t *self );
void AI_StartSequenceTransition( userEntity_t *self );
void AI_StartFinishCurrentSequence( userEntity_t *self );
void AI_StartJumpTowardPoint( userEntity_t *self );
void AI_StartShotCyclerJump( userEntity_t *self );
void AI_StartMoveUntilVisible( userEntity_t *self );
void AI_StartUsePlatform( userEntity_t *self );
void AI_StartMoveDown( userEntity_t *self );
void AI_StartUse( userEntity_t *self );
void AI_StartCower( userEntity_t *self );
void AI_StartUpLadder( userEntity_t *self );
void AI_StartDownLadder( userEntity_t *self );
void AI_StartWaitOnLadderForNoCollision( userEntity_t *self );
void AI_StartPlaySoundToEnd( userEntity_t *self );
void AI_StartActionPlayAnimation( userEntity_t *self );
void AI_StartActionPlayPartialAnimation( userEntity_t *self );
void AI_StartActionPlaySound( userEntity_t *self );
void AI_StartActionSpawn( userEntity_t *self );
void AI_StartActionSetMovingAnimation( userEntity_t *self );
void AI_StartActionSetState( userEntity_t *self );
void AI_StartActionDie( userEntity_t *self );
void AI_StartActionStreamSound( userEntity_t *self );
void AI_StartActionSendMessage( userEntity_t *self );
void AI_StartActionSendUrgentMessage( userEntity_t *self );
void AI_StartActionCall( userEntity_t *self );
void AI_StartActionRandomScript( userEntity_t *self );
void AI_StartActionComeNear( userEntity_t *self );
void AI_StartActionRemove( userEntity_t *self );
void AI_StartActionLookAt( userEntity_t *self );
void AI_StartActionStopLook( userEntity_t *self );
void AI_StartActionAttack( userEntity_t *self );
void AI_StartActionPrint( userEntity_t *self );
void AI_StartWaitUntilNoObstruction( userEntity_t *self );
void AI_StartTeleport( userEntity_t *self );
void AI_StartMoveToRetreatNode( userEntity_t *self );

void AI_StartTestMapRoam( userEntity_t *self );

void AI_StartBackupAllAttributes( userEntity_t *self );
void AI_StartRestoreAllAttributes( userEntity_t *self );
void AI_StartModifyTurnAttribute( userEntity_t *self );
void AI_StartModifyRunAttribute( userEntity_t *self );
void AI_StartModifyWalkAttribute( userEntity_t *self );
void AI_StartBriefCover( userEntity_t *self );



void AI_Idle( userEntity_t *self );
void AI_Wander( userEntity_t *self );
void AI_Attack( userEntity_t *self );
void AI_Chase( userEntity_t *self );
void AI_ChaseAttack( userEntity_t *self );
void AI_ChaseAttackStrafing( userEntity_t *self );
void AI_FollowWalking( userEntity_t *self );
void AI_FollowRunning( userEntity_t *self );
void AI_FollowSidekickWalking( userEntity_t *self );
void AI_FollowSidekickRunning( userEntity_t *self );
void AI_Hide( userEntity_t *self );
void AI_Snipe( userEntity_t *self );
void AI_RunAway( userEntity_t *self );
void AI_WaitForTrainToCome( userEntity_t *self );
void AI_WaitForTrainToStop( userEntity_t *self );
void AI_Fly_Patrol( userEntity_t *self );
void AI_Patrol( userEntity_t *self );
void AI_Pain( userEntity_t *self );
void AI_Die( userEntity_t *self );
void AI_Freeze( userEntity_t *self );
void AI_MoveToLocation( userEntity_t *self );
void AI_MoveToExactLocation( userEntity_t *self );
void AI_WalkToLocation( userEntity_t *self );
void AI_GoingAroundObstacle( userEntity_t *self );
void AI_FlyToLocation( userEntity_t *self );
void AI_SwoopAttack( userEntity_t *self );
void AI_TowardIntermediatePoint( userEntity_t *self );
void AI_TowardIntermediatePoint2( userEntity_t *self );
void AI_AirRangedAttack( userEntity_t *self );
void AI_AirMeleeAttack( userEntity_t *self );
void AI_SwimToLocation( userEntity_t *self );
void AI_FaceAngle( userEntity_t *self );
void AI_Wait( userEntity_t *self );
void AI_WaitForNoCollision( userEntity_t *self );
void AI_PlayAnimation( userEntity_t *self );
void AI_SetIdleAnimation( userEntity_t *self );
void AI_RandomWander( userEntity_t *self );
void AI_MoveToEntity( userEntity_t *self );
void AI_MoveToOwner( userEntity_t *self );
void AI_TakeCover( userEntity_t *self );
void AI_Stay( userEntity_t *self );
void AI_FlyingAroundObstacle( userEntity_t *self );
void AI_PathFollow( userEntity_t *self );
void AI_Dodge( userEntity_t *self );
void AI_SideStep( userEntity_t *self );
void AI_GoingAroundEntity( userEntity_t *self );
void AI_MoveToSnipe( userEntity_t *self );
void AI_Strafe( userEntity_t *self );
void AI_ChaseEvade( userEntity_t *self );
void AI_TakeOff( userEntity_t *self );
void AI_Drop( userEntity_t *self );
void AI_ChaseSideStepLeft( userEntity_t *self );
void AI_ChaseSideStepRight( userEntity_t *self );
void AI_ChargeTowardEnemy( userEntity_t *self );
void AI_RetreatToOwner( userEntity_t *self );
void AI_SequenceTransition( userEntity_t *self );
void AI_FinishCurrentSequence( userEntity_t *self );
void AI_JumpTowardPoint( userEntity_t *self );
void AI_ShotCyclerJump( userEntity_t *self );
void AI_MoveUntilVisible( userEntity_t *self );
void AI_UsePlatform( userEntity_t *self );
void AI_MoveDown( userEntity_t *self );
void AI_Use( userEntity_t *self );
void AI_Cower( userEntity_t *self );
void AI_UpLadder( userEntity_t *self );
void AI_DownLadder( userEntity_t *self );
void AI_WaitOnLadderForNoCollision( userEntity_t *self );
void AI_PlaySoundToEnd( userEntity_t *self );
void AI_MoveAway( userEntity_t *self );
void AI_ActionPlayAnimation( userEntity_t *self );
void AI_ActionPlayPartialAnimation( userEntity_t *self );
void AI_ActionPlaySound( userEntity_t *self );
void AI_ActionSpawn( userEntity_t *self );
void AI_ActionSetMovingAnimation( userEntity_t *self );
void AI_ActionSetState( userEntity_t *self );
void AI_ActionDie( userEntity_t *self );
void AI_ActionStreamSound( userEntity_t *self );
void AI_ActionSendMessage( userEntity_t *self );
void AI_ActionSendUrgentMessage( userEntity_t *self );
void AI_ActionCall( userEntity_t *self );
void AI_ActionRandomScript( userEntity_t *self );
void AI_ActionComeNear( userEntity_t *self );
void AI_ActionRemove( userEntity_t *self );
void AI_ActionLookAt( userEntity_t *self );
void AI_ActionStopLook( userEntity_t *self );
void AI_ActionAttack( userEntity_t *self );
void AI_ActionPrint( userEntity_t *self );
void AI_WaitUntilNoObstruction( userEntity_t *self );
void AI_Teleport( userEntity_t *self );
void AI_StartPlayAnimationAndSound( userEntity_t *self );
void AI_PlayAnimationAndSound( userEntity_t *self );
void AI_StartPlaySound( userEntity_t *self );
void AI_PlaySound( userEntity_t *self );
void AI_StartMoveToEntityUntilVisible( userEntity_t *self );
void AI_MoveToEntityUntilVisible( userEntity_t *self );
void AI_MoveToRetreatNode( userEntity_t *self );
void AI_BriefCover( userEntity_t *self );
void AI_StartStopEntityTask( userEntity_t *self );
void AI_StopEntityTask( userEntity_t *self );

void AI_TestMapRoam( userEntity_t *self );


void AI_StartTakeCover_Attack(userEntity_t *self);
void AI_TakeCover_Attack(userEntity_t *self);


//-----------------------------------------------------------------------------
//		monster specific AI handlers
//-----------------------------------------------------------------------------

// defined in inmater.cpp
userEntity_t *inmater_find_prisoner (userEntity_t *self);
int INMATER_AddKillPrisonerGoal( userEntity_t *self );
void INMATER_StartGotoPrisoner( userEntity_t *self );
void INMATER_GotoPrisoner( userEntity_t *self );
void INMATER_StartWaitAtPrisoner( userEntity_t *self );
void INMATER_WaitAtPrisoner( userEntity_t *self );
// SCG[2/13/00]: void INMATER_StartKillPrisoner( userEntity_t *self );
// SCG[2/13/00]: void INMATER_KillPrisoner( userEntity_t *self );

// defined in prisoner.cpp
void PRISONER_Wander(userEntity_t *self);
void PRISONER_StartWander(userEntity_t *self);
void PRISONER_Cower(userEntity_t *self);
void PRISONER_StartCower(userEntity_t *self);
// SCG[2/13/00]: void PRISONER_InmaterHasItInMe(userEntity_t *self);
// SCG[2/13/00]: void PRISONER_StartInmaterHasItInMe(userEntity_t *self);

// defined in sludgeminion.cpp
void SLUDGEMINION_Scoop( userEntity_t *self );
void SLUDGEMINION_StartScoop( userEntity_t *self );
void SLUDGEMINION_Dump( userEntity_t *self );
void SLUDGEMINION_StartDump( userEntity_t *self );

// defined in buboid.cpp
void BUBOID_StartGetOutOfCoffin( userEntity_t *self );
void BUBOID_GetOutOfCoffin( userEntity_t *self );
void BUBOID_StartMelt( userEntity_t *self );
void BUBOID_Melt( userEntity_t *self );
void BUBOID_StartMelted( userEntity_t *self );
void BUBOID_Melted( userEntity_t *self );
void BUBOID_StartUnmelt( userEntity_t *self );
void BUBOID_Unmelt( userEntity_t *self );
bool BUBOID_ChaseThink( userEntity_t *self );
void BUBOID_StartResurrectWait(userEntity_t *self);
void BUBOID_ResurrectWait(userEntity_t *self);
void BUBOID_StartResurrect(userEntity_t *self);
void BUBOID_Resurrect(userEntity_t *self);

void COLUMN_StartAwakenWait(userEntity_t *self);
void COLUMN_AwakenWait(userEntity_t *self);
void COLUMN_StartAwaken(userEntity_t *self);
void COLUMN_Awaken(userEntity_t *self);

// defined in rotworm.cpp
void ROTWORM_StartJumpSpring( userEntity_t *self );
void ROTWORM_JumpSpring( userEntity_t *self );
void ROTWORM_StartJumpFly( userEntity_t *self );
void ROTWORM_JumpFly( userEntity_t *self );
void ROTWORM_StartJumpBite( userEntity_t *self );
void ROTWORM_JumpBite( userEntity_t *self );

// defined in cambot.cpp
void CAMBOT_StartPathFollow( userEntity_t *self );
void CAMBOT_PathFollow( userEntity_t *self );
void CAMBOT_StartFollowPlayer( userEntity_t *self );
void CAMBOT_FollowPlayer( userEntity_t *self );

// defined in labworker.cpp
//void LABWORKER_StartWork( userEntity_t *self );
//void LABWORKER_Work( userEntity_t *self );

//defined in seagull.cpp <nss>
int SEAGULL_FlyTowardPoint( userEntity_t *self, CVector &Destination);

//defined in chaingang.cpp <nss>
//<nss>
void CHAINGANG_Attack_Think( userEntity_t *self );//<nss>
void CHAINGANG_StartChase( userEntity_t *self);//<nss>
void CHAINGANG_Chase( userEntity_t *self);//<nss>
void CHAINGANG_StartGround( userEntity_t *self );//<nss>
void CHAINGANG_Ground( userEntity_t *self );//<nss>
void CHAINGANG_StartAir( userEntity_t *self );
void CHAINGANG_Air( userEntity_t *self );//<nss>
void CHAINGANG_Begin_Attack( userEntity_t *self );//<nss>
void CHAINGANG_Attack( userEntity_t *self );//<nss>
void CHAINGANG_StartInitiateAir( userEntity_t *self );//<nss>
void CHAINGANG_InitiateAir( userEntity_t *self );//<nss>
void CHAINGANG_StartInitiateGround( userEntity_t *self );//<nss>
void CHAINGANG_InitiateGround( userEntity_t *self );//<nss>
void CHAINGANG_MoveToLocation (userEntity_t *self);//<nss>
void CHAINGANG_StartMoveToLocation( userEntity_t *self );//<nss>
void CHAINGANG_StartDodge( userEntity_t *self); //<nss>
void CHAINGANG_Dodge( userEntity_t *self);//<nss>

//defined in froginator.cpp<nss>
void FROG_Start_Attack_Think( userEntity_t *self );//<nss>
void FROG_Attack_Think( userEntity_t *self );//<nss>
void FROG_Start_Jump( userEntity_t *self );//<nss>
void FROG_Jump( userEntity_t *self );//<nss>
void FROG_Begin_Attack( userEntity_t *self );//<nss>
void FROG_Attack(userEntity_t *self);//<nss>
void FROG_Start_Chase( userEntity_t *self );//<nss>
void FROG_Chase( userEntity_t *self );//<nss>

//defined in griffon.cpp<nss>
void GRIFFON_Start_Attack_Think( userEntity_t *self );//<nss>
void GRIFFON_Attack_Think( userEntity_t *self );//<nss>
void GRIFFON_Begin_Attack( userEntity_t *self );//<nss>
void GRIFFON_Attack(userEntity_t *self);//<nss>
void GRIFFON_Jump(userEntity_t *self);//<nss>
void GRIFFON_Start_Jump(userEntity_t *self);//<nss>
void GRIFFON_StartChase( userEntity_t *self );//<nss>
void GRIFFON_Chase( userEntity_t *self );//<nss>
void GRIFFON_StartFlyAway( userEntity_t *self );//<nss>
void GRIFFON_FlyAway( userEntity_t *self );//<nss>
void GRIFFON_StartGround( userEntity_t *self );//<nss>
void GRIFFON_Ground( userEntity_t *self );//<nss>
void GRIFFON_StartLand( userEntity_t *self );//<nss>
void GRIFFON_Land( userEntity_t *self );//<nss>

//void Harpy_Attack_Think( userEntity_t *self );
//void Harpy_Start_Attack_Think( userEntity_t *self );
//void HARPY_CheckFly( userEntity_t *self );

//defined in harpy.cpp
void HARPY_Attack_Think( userEntity_t *self );
void HARPY_StartChase( userEntity_t *self);
void HARPY_Chase( userEntity_t *self);
void HARPY_StartGround( userEntity_t *self );
void HARPY_Ground( userEntity_t *self );
void HARPY_StartAir( userEntity_t *self );
void HARPY_Air( userEntity_t *self );
void HARPY_Begin_Attack( userEntity_t *self );
void HARPY_Attack( userEntity_t *self );
void HARPY_StartInitiateAir( userEntity_t *self );
void HARPY_InitiateAir( userEntity_t *self );
void HARPY_StartInitiateGround( userEntity_t *self );
void HARPY_InitiateGround( userEntity_t *self );
void HARPY_MoveToLocation (userEntity_t *self);
void HARPY_StartMoveToLocation( userEntity_t *self );
void HARPY_StartDodge( userEntity_t *self); //<nss>
void HARPY_Dodge( userEntity_t *self);//<nss>

// defined in medusa.cpp
void MEDUSA_StartChase( userEntity_t *self );
void MEDUSA_Chase( userEntity_t *self );
void MEDUSA_StartRattle( userEntity_t *self );
void MEDUSA_Rattle( userEntity_t *self );
void MEDUSA_StartGaze( userEntity_t *self );
void MEDUSA_Gaze( userEntity_t *self );
void MEDUSA_StartStopGazing( userEntity_t *self );
void MEDUSA_StopGazing( userEntity_t *self );

// defined in doombat.cpp <nss>
void DOOMBAT_StartHover( userEntity_t *self );//<nss>
void DOOMBAT_Hover( userEntity_t *self );//<nss>
void DOOMBAT_Start_Attack_Think( userEntity_t *self );//<nss>
void DOOMBAT_Attack_Think( userEntity_t *self );//<nss>
void DOOMBAT_StartKamakazi( userEntity_t *self );//<nss>
void DOOMBAT_Kamakazi( userEntity_t *self );//<nss>
void DOOMBAT_StartChase( userEntity_t *self );//<nss>
void DOOMBAT_Chase( userEntity_t *self );//<nss>
void DOOMBAT_StartFlyAway( userEntity_t *self );//<nss>
void DOOMBAT_Attack( userEntity_t *self );//<nss>
void DOOMBAT_Begin_Attack( userEntity_t *self );//<nss>
void DOOMBAT_FlyAway( userEntity_t *self );//<nss>

//Dragon stuff
void DRAGON_StartHover( userEntity_t *self );//<nss>
void DRAGON_Hover( userEntity_t *self );//<nss>
void DRAGON_Attack_Think( userEntity_t *self );//<nss>
void DRAGON_StartFlyAway( userEntity_t *self );//<nss>
void DRAGON_FlyAway( userEntity_t *self );//<nss>
void DRAGON_BeginAttack( userEntity_t *self );//<nss>
void DRAGON_Attack( userEntity_t *self );//<nss>

//Ghost stuff
void Ghost_StartSpiral_Upward(userEntity_t *self);
void Ghost_Spiral_Upward(userEntity_t *self);
void Ghost_Begin_Death(userEntity_t *self);
void Ghost_Die(userEntity_t *self);
void Ghost_Begin_Attack(userEntity_t *self);
void Ghost_Attack(userEntity_t *self);
void Ghost_Chase(userEntity_t *self);
void Ghost_Start_WakeUp(userEntity_t *self);
void Ghost_WakeUp(userEntity_t *self);

void THUNDERSKEET_Chase( userEntity_t *self );
void THUNDERSKEET_StartHover( userEntity_t *self );
void THUNDERSKEET_Hover( userEntity_t *self );
void THUNDERSKEET_StartDartTowardEnemy( userEntity_t *self );
void THUNDERSKEET_DartTowardEnemy( userEntity_t *self );
void THUNDERSKEET_StartFlyAway( userEntity_t *self );
void THUNDERSKEET_FlyAway( userEntity_t *self );
void THUNDERSKEET_Begin_Attack( userEntity_t *self );//NSS[10/26/99]:
void THUNDERSKEET_Attack( userEntity_t *self );//NSS[10/26/99]:

// defined in skeeter.cpp
void SKEETER_Chase( userEntity_t *self );
void SKEETER_StartHover( userEntity_t *self );
void SKEETER_Hover( userEntity_t *self );
void SKEETER_StartDartTowardEnemy( userEntity_t *self );
void SKEETER_DartTowardEnemy( userEntity_t *self );
void SKEETER_StartFlyAway( userEntity_t *self );
void SKEETER_FlyAway( userEntity_t *self );
void SKEETER_StartPreHatch( userEntity_t *self );
void SKEETER_PreHatch( userEntity_t *self );
void SKEETER_StartHatch( userEntity_t *self );
void SKEETER_Hatch( userEntity_t *self );
void SKEETER_Begin_Attack( userEntity_t *self );//NSS[10/26/99]:
void SKEETER_Attack( userEntity_t *self );//NSS[10/26/99]:

// defined in deathsphere.cpp
void DEATHSPHERE_StartChargeWeapon( userEntity_t *self );
void DEATHSPHERE_ChargeWeapon( userEntity_t *self );
void DEATHSPHERE_StartMove(userEntity_t *self);
void DEATHSPHERE_Move( userEntity_t *self );

// defined in protopod.cpp
void PROTOPOD_StartWaitToHatch(userEntity_t* self);
void PROTOPOD_WaitToHatch(userEntity_t* self);
void PROTOPOD_StartHatch(userEntity_t* self);
void PROTOPOD_Hatch(userEntity_t* self);

// defined in lycanthir.cpp
void LYCANTHIR_StartResurrectWait(userEntity_t *self);
void LYCANTHIR_ResurrectWait(userEntity_t *self);
void LYCANTHIR_StartResurrect(userEntity_t *self);
void LYCANTHIR_Resurrect(userEntity_t *self);

void SKINNYWORKER_StartHide( userEntity_t *self );
void SKINNYWORKER_Hide( userEntity_t *self );
void SKINNYWORKER_StartBackWall( userEntity_t *self );
void SKINNYWORKER_BackWall( userEntity_t *self );
void SKINNYWORKER_StartLookOut( userEntity_t *self );
void SKINNYWORKER_LookOut( userEntity_t *self );
void SKINNYWORKER_StartCower( userEntity_t *self );
void SKINNYWORKER_Cower( userEntity_t *self );

// defined in mishimaguard.cpp
void MISHIMAGUARD_StartReload( userEntity_t *self );
void MISHIMAGUARD_Reload( userEntity_t *self );

// defined bot.cpp
void bot_spawn( userEntity_t *self );
void BOT_Die( userEntity_t *self );
void BOT_StartRoam( userEntity_t *self );
void BOT_Roam( userEntity_t *self );
void BOT_StartPickupItem( userEntity_t *self );
void BOT_PickupItem( userEntity_t *self );
void BOT_StartEngageEnemy( userEntity_t *self );
void BOT_EngageEnemy( userEntity_t *self );
void BOT_StartChaseAttack( userEntity_t *self );
void BOT_ChaseAttack( userEntity_t *self );

void BOT_StartStrafeLeft( userEntity_t *self );
void BOT_StrafeLeft( userEntity_t *self );
void BOT_StartStrafeRight( userEntity_t *self );
void BOT_StrafeRight( userEntity_t *self );
void BOT_StartChargeTowardEnemy( userEntity_t *self );
void BOT_ChargeTowardEnemy( userEntity_t *self );

int BOT_IsWeaponAvailable( userEntity_t *self, char *szWeaponName );
int BOT_PickupWeapon( userEntity_t *self, char *szWeaponName );

userEntity_t *BOT_GetSpawnPoint();
MAPNODE_PTR BOT_FindPathNextWanderNode( userEntity_t *self, NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pCurrentNode );
int BOT_MoveTowardPoint( userEntity_t *self, CVector &destPoint, int bMovingTowardNode );

// defined in sidekick.cpp
void SIDEKICK_Die( userEntity_t *self );
void SIDEKICK_StartPickupItem( userEntity_t *self );
void SIDEKICK_PickupItem( userEntity_t *self );
void SIDEKICK_StartEvade( userEntity_t *self );
void SIDEKICK_Evade( userEntity_t *self );
void SIDEKICK_StartRandomWalk( userEntity_t *self );
void SIDEKICK_RandomWalk( userEntity_t *self );
void SIDEKICK_StartLookAround( userEntity_t *self );
void SIDEKICK_LookAround( userEntity_t *self );
void SIDEKICK_StartLookUp( userEntity_t *self );
void SIDEKICK_LookUp( userEntity_t *self );
void SIDEKICK_StartLookDown( userEntity_t *self );
void SIDEKICK_LookDown( userEntity_t *self );
void SIDEKICK_StartSaySomething( userEntity_t *self );
void SIDEKICK_SaySomething( userEntity_t *self );
void SIDEKICK_StartKickSomething( userEntity_t *self );
void SIDEKICK_KickSomething( userEntity_t *self );
void SIDEKICK_StartWhistle( userEntity_t *self );
void SIDEKICK_Whistle( userEntity_t *self );
void SIDEKICK_StartAnimate( userEntity_t *self );
void SIDEKICK_Animate( userEntity_t *self );

void SIDEKICK_StartStopGoingFurther( userEntity_t *self );
void SIDEKICK_StopGoingFurther( userEntity_t *self );
void SIDEKICK_StartTeleportAndComeNear( userEntity_t *self );
void SIDEKICK_TeleportAndComeNear( userEntity_t *self );

void SIDEKICK_StartComeHere( userEntity_t *self );
void SIDEKICK_ComeHere( userEntity_t *self );
void SIDEKICK_StartTeleport( userEntity_t *self );
void SIDEKICK_Teleport( userEntity_t *self );

void SIDEKICK_StartStop(userEntity_t *self);

// NSS[5/19/00]:Added hosportal capabilities for the sidekicks.
void SIDEKICK_StartUseItem( userEntity_t *self );
void SIDEKICK_UseItem(userEntity_t *self);


// defined in NavTest.cpp
void AI_StartTestMapRoam( userEntity_t *self );
void AI_TestMapRoam( userEntity_t *self );

// defined in cryotech.cpp
void cryotech_bambb( userEntity_t *self );
void cryotech_bambc( userEntity_t *self );

void SMALLSPIDER_StartRunAway( userEntity_t *self );
void SMALLSPIDER_RunAway( userEntity_t *self );

//Wyndrax specific task functions	<nss>
void WYNDRAX_Start_Find_WispMaster(userEntity_t *self); //Search for Wisp Master Init
void WYNDRAX_Find_WispMaster(userEntity_t *self);		//If everything is cool add new goal to move to Wispmaster
void WYNDRAX_Start_Collect_Wisps(userEntity_t *self);	//Once we have completed the goal let's recharge, do setup for this here
void WYNDRAX_Collect_Wisps(userEntity_t *self);			//Once we have completely recharged let's fight!
void WYNDRAX_Start_Charge_Up( userEntity_t *self );		//This will find the Charge Unit (Named WyndraxCharge) 
void WYNDRAX_Charge_Up(userEntity_t *self);				//This will get a charge from it.
// NSS[12/13/99]:
void NHARRE_Teleport(userEntity_t *self);				//What actually teleports Nharre
void NHARRE_StartTeleport(userEntity_t *self);			//Find the teleporters if we already haven't done so and setup animations etc..

// NSS[1/15/00]:Kage's special Attacks
void KAGE_StartSmokeScreen(userEntity_t *self);
void KAGE_SmokeScreen(userEntity_t *self);
void KAGE_StartCreateProtectors(userEntity_t *self);
void KAGE_CreateProtectors(userEntity_t *self);
void KAGE_Start_ChargeHealth(userEntity_t *self);
void KAGE_ChargeHealth(userEntity_t *self);
void KAGE_StartWait(userEntity_t *self);
void KAGE_Wait(userEntity_t *self);
void KAGE_StartReturn(userEntity_t *self);
void KAGE_Return(userEntity_t *self);

// NSS[2/25/00]:
void PSYCLAW_JumpUp(userEntity_t *self);
void PSYCLAW_StartJumpUp( userEntity_t *self );

void AI_ClearExecutedTasks( playerHook_t *hook );
void AI_IncrementExecutedTask( playerHook_t *hook, TASKTYPE nTaskType );
int AI_GetNumExecutedTask( playerHook_t *hook, TASKTYPE nTaskType );

#endif _AI_THINK_H

