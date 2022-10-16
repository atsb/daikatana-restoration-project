#ifndef _AI_FUNC_H
#define _AI_FUNC_H


#if _MSC_VER
//	for C++ classes that will be used in other DLLs
#ifndef	DllExport_Plus
#define DllExport_Plus __declspec(dllexport)
#endif
//	for functions the Quake engine calls (not in C++)
#ifndef	DllExport
#define DllExport	extern "C"	__declspec( dllexport )
#endif
#else
// unix 
#define DLLExport_Plus
#define DllExport extern "C"
#endif

#define NOT_VISIBLE			0
#define	COMPLETELY_VISIBLE	1
#define LEFTSIDE_VISIBLE	2
#define RIGHTSIDE_VISIBLE	3

typedef struct _weaponAttribute
{
	CVector offset;
	float fBaseDamage;
	float fRandomDamage;
	float fSpreadX;
	float fSpreadZ;
	float fSpeed;
	float fDistance;
	
} WEAPON_ATTRIBUTE, *WEAPON_ATTRIBUTE_PTR;

typedef struct _SightCache
{
	float		Time;	//Last time played
	char		type;	//Type of monster that played it.
} SIGHT_CACHE, *SIGHT_CACHE_PTR;


// NSS[1/24/00]:Created for the new node traversing function
typedef struct
{
	int				CurrentNodeIndex;
	int				NodeDepth;
	int				MaximumNodeDepth;
	float			CurrentDistance;
	float			GreatestDistance;
	float			MaximumDistance;
	MAPNODE_PTR		pCurrentNode;
	MAPNODE_PTR		pBestNode;
	NODELIST_PTR	pNodeList;
	CVector			origin;
} NodeTraverse;

#define	LEFT		1
#define	RIGHT		2

#define MAX_JUMP_HEIGHT		    48.0f



//NSS[11/6/99]:
float AI_GetSkill(CVector Skill_Values );
//NSS[11/6/99]:
void AI_SetDeathBoundingBox(userEntity_t *self);

void AI_SetAttackFinished(userEntity_t *self);
// NSS[2/26/00]:
AIDATA *AI_GetAIDATA(userEntity_t *self);
// NSS[2/7/00]:
int Node_IsWithinRange(CVector &position1, CVector &position2, int traceit, int NodeType);
// NSS[3/10/00]:
void AI_SetInitialThinkTime(userEntity_t *self);
//----------------------------------------------------------------------------- 
int SIDEKICK_HasTaskInQue( userEntity_t *self, TASKTYPE SearchTask );
int	SIDEKICK_FindFurthestNodeFrom (NodeTraverse *NodeT);

void AI_SelectAnimationPerWeaponType( userEntity_t *self, char *pszAnimation );
int AI_IsGap( userEntity_t *self, CVector &startPoint, CVector &direction, float fSpeed );
int AI_IsGap( userEntity_t *self, CVector &destPoint, float fDistance );

int AI_IsInAir( userEntity_t *self );
int AI_FlyTowardPoint( userEntity_t *self, CVector &destPoint, int bMovingTowardNode, int bFaceDestination );
int AI_Fly( userEntity_t *self );

int AI_IsInWater( userEntity_t *self );
int AI_SwimTowardPoint( userEntity_t *self, CVector &destPoint, int bMovingTowardNode );
int AI_Swim( userEntity_t *self );

void AI_WhenUsedByPlayer( userEntity_t *self, userEntity_t *other, userEntity_t *activator );
//int AI_IsOkToMoveStraight( userEntity_t *self, CVector &destPoint, 
//						   float fXYDistance, float fZDistance );
int AI_AddNewScriptActionGoal( userEntity_t *self, char *szScriptAction, int bUseUniqueId = TRUE );
int AI_AddScriptActionGoal( userEntity_t *self, char *szScriptAction, int bUseUniqueId = TRUE, int bRestart = FALSE );
int AI_AddWhenUsedScriptGoal( userEntity_t *self, char *szScriptAction, int bUseUniqueId = TRUE );

void AI_SelectAmbientAnimation( userEntity_t *self, char *pszAnimation );
void AI_SelectRunningAnimation( userEntity_t *self, char *pszAnimation );
void AI_SelectJumpingAnimation( userEntity_t *self, char *pszAnimation );

DllExport void AI_StartFreeze( userEntity_t *self );
DllExport void AI_EndFreeze( userEntity_t *self );

void AI_EnemyAlert( userEntity_t *self, userEntity_t *pEnemy );

void AI_UpdatePitchTowardEnemy( userEntity_t *self );
void AI_HandleCollisionWithVerticalSurface( userEntity_t *self, CVector &facingDir, float fWidth );

int AI_HandleGap( userEntity_t *self, CVector &destPoint, float fSpeed, float fDistance, float fZDistance );
int AI_HandleGroundObstacle( userEntity_t *self, float fSpeed, int bMovingTowardNode );
int AI_HandleOffGround( userEntity_t *self, CVector &destPoint, float fSpeed );
int AI_HandleDirectPath( userEntity_t *self, CVector &destPoint, int bMovingTowardNode );
int AI_HandleCollisionWithEntities( userEntity_t *self, CVector &destPoint, float fSpeed );
int AI_HandleGettingStuck( userEntity_t *self, CVector &destPoint, float fSpeed );

int AI_ComputeChaseSideStepPoint( userEntity_t *self, userEntity_t *pEnemy, int nDirection, CVector &sideStepPoint );

userEntity_t *AI_GetNextPathCorner( userEntity_t *pPathCorner );
void AI_KilledEnemy( userEntity_t *self );
int	AI_ShouldFollow( userEntity_t *self );

void AI_StartMoveToEntity( userEntity_t *self );

void AI_DoGib( userEntity_t *self );

void AI_Die( userEntity_t *self );
int AI_FindPathToPoint( userEntity_t *self, CVector &point );
int AI_FindPathToEntity( userEntity_t *self, userEntity_t *destEntity, int bTestCanPath = TRUE );
int AI_FindPathToNode( userEntity_t *self, MAPNODE_PTR pDestNode );
int AI_FindPathAroundEntity( userEntity_t *self, userEntity_t *obstacleEnt, CVector &destPoint );

void AI_StartPain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );
int AI_StartMove( userEntity_t *self );

int AI_FlyTowardPoint( userEntity_t *self, CVector &destPoint, int bMovingTowardNode, int bFaceDestination = TRUE );
int AI_Fly( userEntity_t *self );

int AI_OneLastStep( userEntity_t *self, CVector &destPoint, float fSpeed, int bFaceTowardDest /* = TRUE */ );
int AI_MoveTowardPoint( userEntity_t *self, CVector &point, int bMovingTowardNode = FALSE, int bFaceTowardDest = TRUE );
int AI_MoveTowardExactPoint( userEntity_t *self, CVector &destPoint, int bMovingTowardNode = FALSE, int bFaceTowardDest = TRUE );

int AI_TrackMove( userEntity_t *self );
int AI_Move( userEntity_t *self );

int AI_HandleUse( userEntity_t *self, NODEHEADER_PTR pNodeHeader, 
				  MAPNODE_PTR pCurrentNode, MAPNODE_PTR pNextNode );
int AI_HandleUse( userEntity_t *self );

int AI_IsInFOV( userEntity_t *self, userEntity_t *targ );
int AI_IsVisible( userEntity_t *self, userEntity_t *targ );
int AI_IsLineOfSight( userEntity_t *self, CVector &destPoint );
int AI_IsLineOfSight( userEntity_t *self, userEntity_t *targ );
void AI_FaceTowardPoint( userEntity_t *self, CVector &point );
void AI_AdjustAngles( userEntity_t *self );
userEntity_t *AI_FindNearClientOrSidekick( userEntity_t *self );
userEntity_t *AI_FindNearestMonsterBot (userEntity_t *self);

void AI_HandleTaskFinishTime( userEntity_t *self, TASKTYPE nCurrentTaskType, const CVector &lastPosition );
void AI_Update( userEntity_t *self );
void AI_TaskThink( userEntity_t *self );
void AI_PostThink( userEntity_t *self );
playerHook_t *AI_InitMonster( userEntity_t *self, int type );

void AI_StartNextTask( userEntity_t *self );

void ai_generic_pain_handler( userEntity_t *self, userEntity_t *attacker, float kick, int damage, int limit );

//void AI_SetIdleAnimation(userEntity_t *self, int animation_type, int animation_number);

GOAL_PTR AI_AddNewGoal( userEntity_t *self, GOALTYPE nNewGoalType );
GOAL_PTR AI_AddNewGoal( userEntity_t *self, GOALTYPE nNewGoalType, AIDATA_PTR pAIData );
GOAL_PTR AI_AddNewGoal( userEntity_t *self, GOALTYPE nNewGoalType, userEntity_t *pData );
GOAL_PTR AI_AddNewGoal( userEntity_t *self, GOALTYPE nNewGoalType, const CVector &position );
GOAL_PTR AI_AddNewGoal( userEntity_t *self, GOALTYPE nNewGoalType, const float fValue );
GOAL_PTR AI_AddNewGoal( userEntity_t *self, GOALTYPE nNewGoalType, const int nValue );
GOAL_PTR AI_AddNewGoalAtBack( userEntity_t *self, GOALTYPE nNewGoalType );
GOAL_PTR AI_AddNewGoalAtBack( userEntity_t *self, GOALTYPE nNewGoalType, userEntity_t *pData );
GOAL_PTR AI_AddNewGoalAtBack( userEntity_t *self, GOALTYPE nNewGoalType, const CVector &position );
GOAL_PTR AI_AddNewGoalAtBack( userEntity_t *self, GOALTYPE nNewGoalType, const float fValue );
GOAL_PTR AI_AddNewGoalAtBack( userEntity_t *self, GOALTYPE nNewGoalType, const int nValue );

DllExport TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType );
TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, AIDATA_PTR aiData);
TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, userEntity_t *pData );
TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, const CVector &position );
TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, const float fValue );
TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, const int nValue );
TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, frameData_t *pNewSequence );
TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, char *szString );
TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, char *szString, const float fNewValue );
TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, char *szString, const float fValue1, const float fValue2 );
TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, CAction *pAction );
TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, void *pVoid );

TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType );
TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, AIDATA_PTR aiData );
TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, userEntity_t *pData );
TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, const CVector &position );
TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, const float fValue );
TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, const int nValue );
TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, frameData_t *pNewSequence );
TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, char *szString );
TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, CAction *pAction );
TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, char *szString, const float fNewValue );
TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, char *szString, const float fValue1, const float fValue2 );
TASK_PTR AI_AddNewTask( GOAL_PTR pGoal, TASKTYPE nNewTaskType );
TASK_PTR AI_AddNewTask( GOAL_PTR pGoal, TASKTYPE nNewTaskType, AIDATA_PTR aiData);
TASK_PTR AI_AddNewTask( GOAL_PTR pGoal, TASKTYPE nNewTaskType, userEntity_t *pData );
TASK_PTR AI_AddNewTask( GOAL_PTR pGoal, TASKTYPE nNewTaskType, CAction *pAction );
TASK_PTR AI_AddNewTask( GOAL_PTR pGoal, TASKTYPE nNewTaskType, const CVector &position );
TASK_PTR AI_AddNewTask( GOAL_PTR pGoal, TASKTYPE nNewTaskType, const float fValue );
TASK_PTR AI_AddNewTask( GOAL_PTR pGoal, TASKTYPE nNewTaskType, const int nValue );

void AI_RestartCurrentGoal( userEntity_t *self );
void AI_RemoveCurrentGoal( userEntity_t *self );
DllExport GOALSTACK_PTR AI_GetCurrentGoalStack( playerHook_t *hook );

void AI_StartTask( userEntity_t *self, TASK_PTR pTask );
void AI_AddInitialTasksToGoal( userEntity_t *self, GOAL_PTR pGoal, int bStart = TRUE );

void AI_RemoveCurrentTaskWithoutStartingNextTask( userEntity_t *self, int bFinishCurrentSequence = TRUE );
void AI_RemoveCurrentTask( userEntity_t *self, int bFinishCurrentSequence = TRUE );
void AI_RemoveCurrentTask( userEntity_t *self, TASKTYPE nNewTaskType, int bFinishCurrentSequence = TRUE );
void AI_RemoveCurrentTask( userEntity_t *self, TASKTYPE nNewTaskType, 
						   userEntity_t *pEntity, int bFinishCurrentSequence = TRUE );
void AI_RemoveCurrentTask( userEntity_t *self, TASKTYPE nNewTaskType, 
						   const CVector &position, int bFinishCurrentSequence = TRUE );
void AI_RemoveCurrentTask( userEntity_t *self, TASKTYPE nNewTaskType, 
						   const float fValue, int bFinishCurrentSequence = TRUE );
void AI_RemoveCurrentTask( userEntity_t *self, TASKTYPE nNewTaskType, 
						   const int nValue, int bFinishCurrentSequence = TRUE );
void AI_RemoveCurrentTask( userEntity_t *self, TASKTYPE nNewTaskType, 
						   frameData_t *pFrameData, int bFinishCurrentSequence = TRUE );
void AI_RemoveCurrentTask( userEntity_t *self, TASKTYPE nNewTaskType, 
						   char *szString, int bFinishCurrentSequence = TRUE );

void AI_ComputeAlongWallVector( userEntity_t *self, CVector &forward, CVector &normal, CVector &dir );

int AI_IsStateIdle( playerHook_t *hook );
int AI_IsStateSlowWalking( playerHook_t *hook );
int AI_IsStateWalking( playerHook_t *hook );
int AI_IsStateRunning( playerHook_t *hook );
int AI_IsStateAttacking( playerHook_t *hook );
int AI_IsStateCrouching( playerHook_t *hook );
int AI_IsStateMoving( playerHook_t *hook );
void AI_SetStateIdle( playerHook_t *hook );
void AI_SetStateSlowWalking( playerHook_t *hook );
void AI_SetStateWalking( playerHook_t *hook );
void AI_SetStateRunning( playerHook_t *hook );
void AI_SetStateAttacking( playerHook_t *hook );
void AI_SetStateCrouching( playerHook_t *hook );
void AI_ClearStateCrouching( playerHook_t *hook );

// scripting functions
GOAL_PTR AI_AddNewScriptGoalAtBack(userEntity_t *self, GOALTYPE nNewGoalType);
GOAL_PTR AI_AddNewScriptGoalAtBack(userEntity_t *self, GOALTYPE nNewGoalType, AIDATA_PTR aiData);
GOAL_PTR AI_AddNewScriptGoalAtBack( userEntity_t *self, GOALTYPE nNewGoalType, userEntity_t *pEntity );
GOAL_PTR AI_AddNewScriptGoalAtBack( userEntity_t *self, GOALTYPE nNewGoalType, const CVector &position );
GOAL_PTR AI_AddNewScriptGoalAtBack( userEntity_t *self, GOALTYPE nNewGoalType, const float fValue );
GOAL_PTR AI_AddNewScriptGoalAtBack( userEntity_t *self, GOALTYPE nNewGoalType, const int nValue );

void AI_RemoveCurrentScriptGoal( userEntity_t *self );

//-----------------------------------------------------------------------------

int AI_IsChaseVisible( userEntity_t *self );
void AI_EnemyAlertNearbyMonsters( userEntity_t *self, userEntity_t *pEnemy );
userEntity_t *AI_CanClientHear( userEntity_t *self );

void AI_ParseEpairs( userEntity_t *self );
void AI_DetermineMovingEnvironment( userEntity_t *self );

//-----------------------------------------------------------------------------

void AI_Fly_Patrol( userEntity_t *self );
#ifdef _DEBUG// SCG[12/9/99]: 
void AI_TestTrain();
int AI_TestTakeCover();
void AI_TestDoor();
void AI_TestLadder();
void AI_TestBoar();

void ECTS_TakeCoverThink( userEntity_t *self );
#endif //_DEBUG// SCG[12/9/99]: 

int AI_IsOnTrain( userEntity_t *self );
int AI_IsOnMovingTrain( userEntity_t *self );

int AI_IsOnLadder( userEntity_t *self, MAPNODE_PTR pNextNode );
int AI_UseLadder( userEntity_t *self, MAPNODE_PTR pNode );

int	ai_ChooseLedge( userEntity_t *self, CVector &goal_org, float start_dist );
int AI_CheckBlockingDirection( userEntity_t *self, CVector &destPoint );
void AI_GoAroundObstacle( userEntity_t *self );
int	AI_IsDirectPath( userEntity_t *self, CVector &destPoint );
MAPNODE_PTR AI_FindPathNextWanderNode( userEntity_t *self, NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pCurrentNode );
int AI_IsCompletelyVisible( userEntity_t *self, CVector &destPoint, float fWidthFactor = 0.5f );
int AI_IsCompletelyVisible( userEntity_t *self, userEntity_t *targ, float fWidthFactor = 0.5f );
int AI_HandleSelfPreservation( userEntity_t *self );
int AI_IsStraightPath( userEntity_t *self, userEntity_t *enemy );
int AI_IsOnMovingTrain( userEntity_t *self );

void AI_Idle( userEntity_t *self );
void AI_Hide( userEntity_t *self );
void AI_WaitForTrainToCome( userEntity_t *self );
void AI_WaitForTrainToStop( userEntity_t *self );
void AI_MoveToLocation( userEntity_t *self );
void AI_GoingAroundObstacle( userEntity_t *self );
void AI_SwimToLocation( userEntity_t *self );
void AI_FaceAngle( userEntity_t *self );
void AI_Wait( userEntity_t *self );
void AI_MoveToEntity( userEntity_t *self );
void AI_TakeCover( userEntity_t *self );
void AI_Dodge( userEntity_t *self );
void AI_SideStep( userEntity_t *self );
void AI_GoingAroundEntity( userEntity_t *self );
void AI_SequenceTransition( userEntity_t *self );
void AI_FinishCurrentSequence( userEntity_t *self );

void AI_ChaseSideStepRight( userEntity_t *self );
void AI_ChaseSideStepLeft( userEntity_t *self );
void AI_ChargeTowardEnemy( userEntity_t *self );

void AI_StartJumpTowardPoint( userEntity_t *self );
void AI_JumpTowardPoint( userEntity_t *self );
void AI_StartShotCyclerJump( userEntity_t *self );
void AI_ShotCyclerJump( userEntity_t *self );
void AI_StartMoveUntilVisible( userEntity_t *self );
void AI_MoveUntilVisible( userEntity_t *self );
void AI_StartUsePlatform( userEntity_t *self );
void AI_UsePlatform( userEntity_t *self );
void AI_StartMoveDown( userEntity_t *self );
void AI_MoveDown( userEntity_t *self );

void AI_StartMoveToExactLocation( userEntity_t *self );
void AI_MoveToExactLocation( userEntity_t *self );

void AI_StartPain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );
void AI_StartDie( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &destPoint );
void AI_StartIdleSettings( userEntity_t *self );

void AI_StartUpLadder( userEntity_t *self );
void AI_UpLadder( userEntity_t *self );
void AI_StartDownLadder( userEntity_t *self );
void AI_DownLadder( userEntity_t *self );
void AI_StartWaitOnLadderForNoCollision( userEntity_t *self );
void AI_WaitOnLadderForNoCollision( userEntity_t *self );

void AI_DoLevelStartScript();

// crouching related stuff
int		AI_IsCrouching( userEntity_t *self );
int		AI_IsCrouching( playerHook_t *hook );
void	AI_StartCrouching( userEntity_t *self );
void	AI_EndCrouching( userEntity_t *self );
int		AI_HandleCrouching( userEntity_t *self );

void AI_SetSpawnValue( userEntity_t *self, int nSpawnValue );
int AI_UpdateHeadAngles( userEntity_t *self );

//-----------------------------------------------------------------------------
//		monster specific AI handlers  (nss)
//-----------------------------------------------------------------------------
int		AI_FLY_Debug_Origin(userEntity_t *self);						//Stupid Origin hack that might change if I have the time.
int		AI_Checkbelow_For_Liquid(userEntity_t *self,int Resolution);	//Checking for valid ground point below(flying/land units only!)
#ifdef _DEBUG
void	AI_Drop_Marker (userEntity_t *self, CVector &Target);			//Drop a 'flare' at the Target point, good debug tool(self managing)
#endif
void	Terrain_Node_Change(userEntity_t *self, movetype_t MoveType);   //Change from Ground to air/air to ground function(must do this if you are going to change movetype!)
void	FISH_ParseEpairs( userEntity_t *self );							//Use this function only with the passive Fish first think function.
void    FISH_Update( userEntity_t *self);								//All fish use this as their pre-think function it eventually calls AI_UPDATE()
float	AI_Determine_Room_Height( userEntity_t *self, int Max_Mid, int type);//Will determine the height of the room the entity passed is in(up to 2*Max_Mid).
float   AI_TraceZLimit( userEntity_t *self, float maxdist );            //Calculates z (vertical) limit of entity.
float	AI_DetectForLikeMonsters( userEntity_t *self);					//Will check to make sure we are not aiming at out buddies
int		AI_ComputeAwayPoint( userEntity_t *self, CVector &AwayPoint, int Distance, int Resolution, CVector *Axis); 
//-----------------------------------------------------------------------------

// defined in AI_File.cpp
int AIATTRIBUTE_ReadFile( const char *szFileName );
void AIATTRIBUTE_Destroy();
char *AIATTRIBUTE_GetModelName( const char *szClassName );
char *AIATTRIBUTE_GetCSVFileName( const char *szClassName );
WEAPON_ATTRIBUTE_PTR AIATTRIBUTE_SetInfo( userEntity_t *self );

int AISOUNDS_ReadFile( const char *szFileName );
int AI_PlayAttackSounds( userEntity_t *self );

//#ifdef _DEBUG

float Random();
void AI_PitchTowardPoint( userEntity_t *self, CVector &destPoint );
void AI_OrientTowardPoint( userEntity_t *self, CVector &destPoint );
int AI_IsWithinAttackDistance( userEntity_t *self, float fDistance, userEntity_t *enemy = NULL );
int AI_IsWithinAttackDistance( userEntity_t *self, userEntity_t *enemy );
void AI_SetNextPathTime( playerHook_t *hook, float fTime );
int AI_CanPath( playerHook_t *hook );
int AI_CanMove( playerHook_t *hook );
void AI_SetTaskFinishTime( playerHook_t *hook, float fTime );
int AI_IsOverTaskFinishTime( playerHook_t *hook );
void AI_SetOkToAttackFlag( playerHook_t *hook, int bFlag );
int AI_IsOkToAttack( playerHook_t *hook );
float AI_ComputeMovingSpeed( playerHook_t *hook );
int AI_IsEnemyLookingAtMe( userEntity_t *self, userEntity_t *pEnemy );
int AI_IsFacingEnemy( userEntity_t *self, userEntity_t *pEnemy, float maxyaw = 5.0f, float maxpitch = 45.0f, float maxdist = -1.0f );
int AI_IsEnemyTargetingMe( userEntity_t *self, userEntity_t *pEnemy );
void AI_SetEnemy( userEntity_t *self, userEntity_t *enemy );
void AI_SetMovementState( playerHook_t *hook, MOVEMENTSTATE nNewState );
MOVEMENTSTATE AI_GetMovementState( playerHook_t *hook );
void AI_SetCanUseFlag( playerHook_t *hook, int nUseFlag );
void AI_SetBoundedBox( playerHook_t *hook, const CVector &mins, const CVector &maxs );
int AI_IsSameAngle2D( userEntity_t *self, CVector &a1, CVector &a2 );
int AI_IsSameAngle3D( userEntity_t *self, CVector &a1, CVector &a2 );

int AI_IsGroundUnit( userEntity_t *self );
int AI_IsOnGround( userEntity_t *self );

int AI_IsFlyingUnit( userEntity_t *self );
int AI_IsInAir( userEntity_t *self );
int AI_IsInsideBoundingBox( userEntity_t *self );
int AI_IsCloseDistance( playerHook_t *hook, float fDistance );
int AI_IsCloseDistance2( userEntity_t *self, float fDistance );
int AI_IsExactDistance( playerHook_t *hook, float fDistance );
int AI_IsOnFlatSurface( userEntity_t *self );
int AI_IsPathToEntityClose( userEntity_t *self, userEntity_t *destEntity );
int AI_IsReadyToAttack1( userEntity_t *self );
int AI_IsReadyToAttack2( userEntity_t *self );
int AI_IsEndAnimation( userEntity_t *self );
int AI_IsEnemyDead( userEntity_t *self );

int AI_ComputeFlyAwayPoint( userEntity_t *self, CVector &flyAwayPoint, int Distance, int Resolution, CVector *Axis);
int AI_FlyTowardPoint2( userEntity_t *self, CVector &destPoint, float TurnRate );


//#else _DEBUG
//
//#include "nodelist.h"
//#include "ai_frames.h"
//
////-----------------------------------------------------------------------------
//// inline functions defined below
////-----------------------------------------------------------------------------
////
////#if _MSC_VER
////#include <crtdbg.h>
////#endif
////#include "dk_point.h"
////#include "nodelist.h"
//
//// ----------------------------------------------------------------------------
////
//// Name:
//// Description:
//// Input:
//// Output:
//// Note:
////
//// ----------------------------------------------------------------------------
//__inline float Random()
//{
//   	return (float)rand() / (float)RAND_MAX;
//}
//
//// ----------------------------------------------------------------------------
////
//// Name:		AI_PitchTowardPoint
//// Description:
//// Input:
//// Output:
//// Note:
////
//// ----------------------------------------------------------------------------
//__inline void AI_PitchTowardPoint( userEntity_t *self, CVector &destPoint )
//{
//	_ASSERTE( self );
//
//	CVector dir = destPoint - self->s.origin;
//	dir.Normalize();
//	CVector newIdealAngle;
//	VectorToAngles( dir, newIdealAngle );
//	self->ideal_ang.pitch = newIdealAngle.pitch;
//	com->ChangePitch(self);
//}
//
//// ----------------------------------------------------------------------------
////
//// Name:		AI_OrientTowardPoint
//// Description:
//// Input:
//// Output:
//// Note:
////
//// ----------------------------------------------------------------------------
//__inline void AI_OrientTowardPoint( userEntity_t *self, CVector &destPoint )
//{
//	_ASSERTE( self );
//	playerHook_t *hook = (playerHook_t *) self->userHook;
//	_ASSERTE( hook );
//
//	CVector dir = destPoint - self->s.origin;
//	dir.Normalize();
//
//	CVector newIdealAngle;
//	VectorToAngles( dir, newIdealAngle );
//
//	self->ideal_ang = newIdealAngle;
//}
//
//// ----------------------------------------------------------------------------
//
//__inline void AI_SetNextPathTime( playerHook_t *hook, float fTime )
//{
//	_ASSERTE( hook );
//	hook->fNextPathTime = fTime;
//}
//
//__inline int AI_CanPath( playerHook_t *hook )
//{
//	_ASSERTE( hook );
//	if ( gstate->time > hook->fNextPathTime )
//	{
//		return TRUE;
//	}
//
//	return FALSE;
//}
//
//// ----------------------------------------------------------------------------
//
//__inline int AI_CanMove( playerHook_t *hook )
//{
//	_ASSERTE( hook );
//	if ( hook->nSpawnValue & SPAWN_DO_NOT_MOVE )
//	{
//		return FALSE;
//	}
//
//	return TRUE;
//}
//
//// ----------------------------------------------------------------------------
//
//__inline void AI_SetTaskFinishTime( playerHook_t *hook, float fTime )
//{
//	_ASSERTE( hook );
//
//	if ( fTime > 0.0f )
//	{
//		hook->fTaskFinishTime = gstate->time + fTime;
//	}
//	else
//	{
//		hook->fTaskFinishTime = fTime;
//	}
//}
//
//__inline int AI_IsOverTaskFinishTime( playerHook_t *hook )
//{
//	_ASSERTE( hook );
//
//	// note: if the value is < 0, then the task has no time limit
//	if ( hook->fTaskFinishTime > 0.0f && gstate->time > hook->fTaskFinishTime )
//	{
//		return TRUE;
//	}
//
//	return FALSE;
//}
//
//// ----------------------------------------------------------------------------
//
//__inline void AI_SetOkToAttackFlag( playerHook_t *hook, int bFlag )
//{
//	_ASSERTE( hook );
//	hook->bOkToAttack = bFlag;
//}
//
//__inline int AI_IsOkToAttack( playerHook_t *hook )
//{
//	_ASSERTE( hook );
//	return hook->bOkToAttack;
//}
//
//// ----------------------------------------------------------------------------
//
//__inline float AI_ComputeMovingSpeed( playerHook_t *hook )
//{
//	_ASSERTE( hook );
//
//	float fSpeed = hook->walk_speed;
//	if ( hook->stateFlags & SFL_RUNNING )
//	{
//		fSpeed = hook->run_speed;
//	}
//	else 
//	if ( hook->stateFlags & SFL_ATTACKING )
//	{
//		fSpeed = hook->attack_speed;
//	}
//
//	return fSpeed;
//}
//
//// ----------------------------------------------------------------------------
////
//// Name:		AI_IsEnemyLookingAtMe
//// Description:
//// Input:
//// Output:
//// Note:
////
//// ----------------------------------------------------------------------------
//__inline int AI_IsEnemyLookingAtMe( userEntity_t *self, userEntity_t *pEnemy )
//{
//	_ASSERTE( self && pEnemy );
//	playerHook_t *pEnemyHook = (playerHook_t *)pEnemy->userHook;
//	_ASSERTE( pEnemyHook );
//
//	CVector dir = self->s.origin - pEnemy->s.origin;
//	dir.Normalize();
//	CVector angleTowardSelf;
//	VectorToAngles( dir, angleTowardSelf );
//	
//	float fEnemyTowardSelfX = AngleMod(angleTowardSelf.x);
//	float fEnemyTowardSelfY = AngleMod(angleTowardSelf.y);
//	float fEnemyFacingAngleX = AngleMod(pEnemy->s.angles[PITCH]);
//	float fEnemyFacingAngleY = AngleMod(pEnemy->s.angles[YAW]);
//
//	float fYawDiff = fabs( fEnemyTowardSelfY - fEnemyFacingAngleY );
//	float fPitchDiff = fabs( fEnemyTowardSelfX - fEnemyFacingAngleX );
//	if ( (fYawDiff < 15.0f || fYawDiff > 345.0f) && (fPitchDiff < 30.0f || fPitchDiff > 330.0f) )
//	{
//		return TRUE;
//	}
//
//	return FALSE;
//}
//
//// ----------------------------------------------------------------------------
////
//// Name:		AI_IsFacingEnemy
//// Description:
//// Input:
//// Output:
//// Note:
////
//// ----------------------------------------------------------------------------
//__inline int AI_IsFacingEnemy( userEntity_t *self, userEntity_t *pEnemy )
//{
//	_ASSERTE( self && pEnemy );
//	playerHook_t *hook = (playerHook_t *)self->userHook;
//	_ASSERTE( hook );
//
//	CVector dir = pEnemy->s.origin - self->s.origin;
//	dir.Normalize();
//	CVector angleTowardEnemy;
//	VectorToAngles( dir, angleTowardEnemy );
//	
//	float fTowardEnemyX = AngleMod(angleTowardEnemy.x);
//	float fTowardEnemyY = AngleMod(angleTowardEnemy.y);
//	float fFacingAngleX = AngleMod(self->s.angles.pitch);
//	float fFacingAngleY = AngleMod(self->s.angles.yaw);
//
//	float fYawDiff = fabs( fTowardEnemyY - fFacingAngleY );
//	float fPitchDiff = fabs( fTowardEnemyX - fFacingAngleX );
//	if ( (fYawDiff < 15.0f || fYawDiff > 345.0f) && (fPitchDiff < 30.0f || fPitchDiff > 330.0f) )
//	{
//		return TRUE;
//	}
//
//	return FALSE;
//}
//
//// ----------------------------------------------------------------------------
////
//// Name:		AI_IsEnemyTargetingMe
//// Description:
//// Input:
//// Output:
//// Note:
////
//// ----------------------------------------------------------------------------
//__inline int AI_IsEnemyTargetingMe( userEntity_t *self, userEntity_t *pEnemy )
//{
//	_ASSERTE( self && pEnemy );
//	if ( pEnemy->flags & FL_CLIENT )
//	{
//		playerHook_t *pEnemyHook = (playerHook_t *)pEnemy->userHook;
//		_ASSERTE( pEnemyHook );
//		if ( pEnemyHook->autoAim.ent == self )
//		{
//			return TRUE;	
//		}
//	}
//	else
//	{
//		return AI_IsFacingEnemy( self, pEnemy );
//	}
//	return FALSE;
//}
//
//
//// ----------------------------------------------------------------------------
////
//// Name:		AI_SetEnemy
//// Description:
//// Input:
//// Output:
//// Note:
////
//// ----------------------------------------------------------------------------
//__inline void AI_SetEnemy( userEntity_t *self, userEntity_t *enemy )
//{
//	_ASSERTE( self );
//	self->enemy = enemy;
////	self->goalentity = enemy;
//
//	// make sure this  entity can path to the enemy
//	playerHook_t *hook = (playerHook_t *)self->userHook;
//	_ASSERTE( hook );
//	AI_SetNextPathTime( hook, 0.0f );
//}
//
//// ----------------------------------------------------------------------------
////
//// Name:        AI_SetMovementState
//// Description:
//// Input:
//// Output:
//// Note:
////
//// ----------------------------------------------------------------------------
//__inline void AI_SetMovementState( playerHook_t *hook, MOVEMENTSTATE nNewState )
//{
//	_ASSERTE( hook );
//	hook->nMovementState = nNewState;
//}
//
//// ----------------------------------------------------------------------------
////
//// Name:        AI_GetMovementState
//// Description:
//// Input:
//// Output:
//// Note:
////
//// ----------------------------------------------------------------------------
//__inline MOVEMENTSTATE AI_GetMovementState( playerHook_t *hook )
//{
//	_ASSERTE( hook );
//	return hook->nMovementState;
//}
//
//
//// ----------------------------------------------------------------------------
////
//// Name:        AI_SetCanUseFlag
//// Description:
//// Input:
//// Output:
//// Note:
////
//// ----------------------------------------------------------------------------
//__inline void AI_SetCanUseFlag( playerHook_t *hook, int nUseFlag )
//{
//	_ASSERTE( hook );
//	hook->nCanUseFlag |= nUseFlag;	
//}
//
//// ----------------------------------------------------------------------------
////
//// Name:        AI_SetBoundedBox
//// Description:
//// Input:
//// Output:
//// Note:
////
//// ----------------------------------------------------------------------------
//__inline void AI_SetBoundedBox( playerHook_t *hook, const CVector &mins, const CVector &maxs )
//{
//	_ASSERTE( hook );
//	hook->bBounded = TRUE;
//
//	hook->boundedBox.mins = mins;
//	hook->boundedBox.maxs = maxs;
//}
//
//// ----------------------------------------------------------------------------
//
//#define PITCH_PER_FRAME( e )	(gstate->frametime * (e)->ang_speed.pitch)
//#define YAW_PER_FRAME( e )		(gstate->frametime * (e)->ang_speed.yaw)
//#define ROLL_PER_FRAME( e )		(gstate->frametime * (e)->ang_speed.roll)
//
//__inline int AI_IsSameAngle2D( userEntity_t *self, CVector &a1, CVector &a2 )
//{
//	_ASSERTE( self );
//
//	float y = YAW_PER_FRAME(self);
//	float yDiff = fabs(a1.y - a2.y);
//	if ( (yDiff < y || yDiff > (360.0f-y)) )
//	{
//		return TRUE;
//	}
//
//	return FALSE;
//}
//
//// ----------------------------------------------------------------------------
//
//__inline int AI_IsSameAngle3D( userEntity_t *self, CVector &a1, CVector &a2 )
//{
//	_ASSERTE( self );
//	float x = PITCH_PER_FRAME(self);
//	float y = YAW_PER_FRAME(self);
//	float z = ROLL_PER_FRAME(self);
//	float xDiff = fabs(a1.x - a2.x);
//	float yDiff = fabs(a1.y - a2.y);
//	float zDiff = fabs(a1.z - a2.z);
//	if ( (xDiff < x || xDiff > (360.0f-x)) &&
//		 (yDiff < y || yDiff > (360.0f-y)) &&
//		 (zDiff < z || zDiff > (360.0f-z)) )
//	{
//		return TRUE;
//	}
//
//	return FALSE;
//}
//
//
//// ----------------------------------------------------------------------------
//
//__inline int AI_IsFlyingUnit( userEntity_t *self )
//{
//	_ASSERTE( self );
//
//	if ( self->movetype == MOVETYPE_FLY || self->movetype == MOVETYPE_HOVER )
//	{
//		playerHook_t *hook = (playerHook_t *) self->userHook;
//		_ASSERTE( hook );
//		if ( !(hook->nSpawnValue & SPAWN_DO_NOT_FLY) )
//		{
//			return TRUE;
//		}
//	}
//
//	return FALSE;
//}
//
//// ----------------------------------------------------------------------------
//
//__inline int AI_IsInAir( userEntity_t *self )
//{
//	_ASSERTE( self );
//	if ( self->movetype == MOVETYPE_FLY || self->movetype == MOVETYPE_HOVER )
//	{
//		return TRUE;
//	}
//
//	return FALSE;
//}
//
//// ----------------------------------------------------------------------------
//
//__inline int AI_IsInsideBoundingBox( userEntity_t *self )
//{
//	_ASSERTE( self );
//	playerHook_t *hook = (playerHook_t *) self->userHook;
//	_ASSERTE( hook );
//
//	CVector currentPosition( self->s.origin );
//
//	if ( currentPosition.x >= hook->boundedBox.mins.x && currentPosition.x <= hook->boundedBox.maxs.x &&
//		 currentPosition.y >= hook->boundedBox.mins.y && currentPosition.y <= hook->boundedBox.maxs.y &&
//		 currentPosition.z >= hook->boundedBox.mins.z && currentPosition.z <= hook->boundedBox.maxs.z )
//	{
//		return TRUE;
//	}
//
//	return FALSE;
//}
//
//// ----------------------------------------------------------------------------
////
//// Name:        AI_IsCloseDistance
//// Description:
//// Input:
//// Output:
//// Note:
////
//// ----------------------------------------------------------------------------
//__inline int AI_IsCloseDistance( playerHook_t *hook, float fDistance )
//{
//	_ASSERTE( hook );
//	float fSpeed = AI_ComputeMovingSpeed( hook );
//
//	float fSpeedPerFrame = fSpeed * 0.125f;
//	if ( fDistance < fSpeedPerFrame )
//	{
//		return TRUE;
//	}
//
//	return FALSE;
//}
//
//// ----------------------------------------------------------------------------
////
//// Name:        AI_IsExactDistance
//// Description:
//// Input:
//// Output:
//// Note:
////
//// ----------------------------------------------------------------------------
//__inline int AI_IsExactDistance( playerHook_t *hook, float fDistance )
//{
//	_ASSERTE( hook );
//	float fSpeed = AI_ComputeMovingSpeed( hook );
//
//	float fSpeedPerFrame = fSpeed * 0.03f;
//	if ( fDistance < fSpeedPerFrame )
//	{
//		return TRUE;
//	}
//
//	return FALSE;
//}
//
//// ----------------------------------------------------------------------------
//
//__inline int AI_IsOnFlatSurface( userEntity_t *self )
//{
//	_ASSERTE( self );
//	CVector angleVector( 0, self->s.angles[YAW], 0 );
//
//	CVector facingDir;
//	AngleToVectors( angleVector, facingDir );
//	facingDir.Normalize();
//
//	CVector forwardPoint;
//	VectorMA( self->s.origin, facingDir, 32.0f, forwardPoint );
//
//	CVector end( forwardPoint.x, forwardPoint.y, forwardPoint.z - 32.0f );
//
//	tr = gstate->TraceLine_q2( self->s.origin, end, self, MASK_SOLID );
//	if ( tr.fraction < 1.0 && tr.plane.normal.z >= 0.9999f )
//	{
//		return TRUE;
//	}
//
//	return FALSE;
//}
//
//
//__inline int AI_IsPathToEntityClose( userEntity_t *self, userEntity_t *destEntity )
//{
//	_ASSERTE( self && destEntity );
//
//	playerHook_t *hook = (playerHook_t *)self->userHook;
//	_ASSERTE( hook );
//
//	if ( !hook->pPathList->pPath )
//	{
//		return FALSE;
//	}
//
//	if ( hook->pPathList->pPathEnd && (hook->nMoveCounter % 5) == 0 )
//	{
//		NODELIST_PTR pNodeList = hook->pNodeList;
//		_ASSERTE( pNodeList );
//		NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
//		_ASSERTE( pNodeHeader );
//		
//		MAPNODE_PTR pLastNode = NODE_GetNode( pNodeHeader, hook->pPathList->pPathEnd->nNodeIndex );
//
//		float fDistance = VectorDistance( destEntity->s.origin, pLastNode->position );
//		if ( fDistance > 768.0f )
//		{
//			return FALSE;
//		}
//	}
//
//	return TRUE;
//}
//
//// ----------------------------------------------------------------------------
////
//// Name:		AI_IsReadyToAttack1
//// Description:
//// Input:
//// Output:
//// Note:
////
//// ----------------------------------------------------------------------------
//__inline int AI_IsReadyToAttack1( userEntity_t *self )
//{
//	_ASSERTE( self );
//	if ( self->s.frameInfo.frameState & FRSTATE_PLAYATTACK1 )
//	{
//		self->s.frameInfo.frameState -= FRSTATE_PLAYATTACK1;
//		return TRUE;
//	}
//
//	return FALSE;
//}
//
//// ----------------------------------------------------------------------------
////
//// Name:		AI_IsReadyToAttack2
//// Description:
//// Input:
//// Output:
//// Note:
////
//// ----------------------------------------------------------------------------
//__inline int AI_IsReadyToAttack2( userEntity_t *self )
//{
//	_ASSERTE( self );
//	if ( self->s.frameInfo.frameState & FRSTATE_PLAYATTACK2 )
//	{
//		self->s.frameInfo.frameState -= FRSTATE_PLAYATTACK2;
//		return TRUE;
//	}
//
//	return FALSE;
//}
//
//// ----------------------------------------------------------------------------
////
//// Name:		AI_IsEndAnimation
//// Description:
//// Input:
//// Output:
//// Note:
////
//// ----------------------------------------------------------------------------
//__inline int AI_IsEndAnimation( userEntity_t *self )
//{
//	_ASSERTE( self );
//	if ( self->s.frameInfo.frameState & FRSTATE_LAST )
//	{
//		self->s.frameInfo.frameState -= FRSTATE_LAST;
//		return TRUE;
//	}
//
//	if ( self->s.frameInfo.frameState & FRSTATE_STOPPED )
//	{
//		self->s.frameInfo.frameState -= FRSTATE_STOPPED;
//		return TRUE;
//	}
//
//	return FALSE;
//}
//
//// ----------------------------------------------------------------------------
////
//// Name:		AI_IsEnemyDead
//// Description:
//// Input:
//// Output:
//// Note:
////
//// ----------------------------------------------------------------------------
//__inline int AI_IsEnemyDead( userEntity_t *self )
//{
//	_ASSERTE( self );
//
//	if ( self->enemy->deadflag != DEAD_NO )
//	{
//		AI_RemoveCurrentGoal( self );
//		AI_KilledEnemy( self );
//		
//		return TRUE;
//	}
//
//	return FALSE;
//}
//
//__inline int AI_IsInWater( userEntity_t *self )
//{
//	_ASSERTE( self );
//	if ( self->movetype == MOVETYPE_SWIM )
//	{
//			return TRUE;
//	}
//
//	return FALSE;
//}
//
//#endif _DEBUG


#endif _AI_FUNC_H
