// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#if _MSC_VER
#include <crtdbg.h>
#endif
#include "world.h"
#include "ai.h"
#include "ai_utils.h"
//#include "ai_move.h"// SCG[1/23/00]: not used
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"
//#include "actorlist.h"// SCG[1/23/00]: not used

/* ***************************** define types ****************************** */

#define MONSTERNAME			"monster_slaughterskeet"

#define AMPLITUDE_ADJUSTER		10		// higher number equals higher amplitude

#define HOVER_DISTANCE			178.0f
#define FLY_AWAY_DISTANCE		512.0f

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
//extern float sin_tbl[];
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_slaughterskeet(userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
//
// Name:		SKEETER_Think
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKEETER_Think(userEntity_t *self)
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_TaskThink(self);
	
	//if(hook->acro_boost_time > 12.0f)
	//{	
	//	hook->acro_boost_time = 0.0f;
	//}
	//self->velocity.z += 55.0*(sin_tbl[(int)hook->acro_boost_time]);
	//hook->acro_boost_time +=3.3;

}


// ----------------------------------------------------------------------------
//
// Name:		SKEETER_ComputeFlyAwayPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKEETER_ComputeFlyAwayPoint( userEntity_t *self, CVector &flyAwayPoint )
{
	//NSS[10/25/99]: added new flyaway
	CVector Axis;
	//Initialize the flyaway point to be ourself
	flyAwayPoint = self->s.origin;
//	Axis;// SCG[1/23/00]: not used

	Axis.x = 1;
	Axis.y = 0;
	Axis.z = 1;
	if(AI_ComputeFlyAwayPoint( self, flyAwayPoint, FLY_AWAY_DISTANCE, 12, &Axis )==0)
	{
		
		Axis.x = 0;
		Axis.y = 1;
		Axis.z = 1;
		if(AI_ComputeFlyAwayPoint( self, flyAwayPoint, FLY_AWAY_DISTANCE, 12, &Axis )==0)
		{
			AI_Dprintf( "No point found setting my new origin to above enemy's head.\n");
			flyAwayPoint = self->enemy->s.origin;
			flyAwayPoint.z += HOVER_DISTANCE;
		}
	}
	
	MAPNODE_PTR Node = 	NODE_GetClosestNode( self, flyAwayPoint);

	if(!Node)
	{
		AI_Dprintf("BAD THINGS ARE HAPPENING!!!Flyaway with no Node system initialized!\n");
		AI_RemoveCurrentTask(self,FALSE);
		return;
	}

	flyAwayPoint = Node->position;
}


// ----------------------------------------------------------------------------
//
// Name:		SKEETER_Chase
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKEETER_Chase( userEntity_t *self )
{
    self->think = SKEETER_Think;
	
	if ( self->enemy->waterlevel == 3 )
    {
		// the enemy is out of range or dead, give up chasing
		AI_RemoveCurrentGoal( self );
		return;
    }

	playerHook_t *hook = AI_GetPlayerHook( self );

	float fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );
	if ( (fDistance > hook->active_distance && !hook->pPathList->pPath) || self->enemy->deadflag != DEAD_NO )
	{
		// the enemy is out of range or dead, give up chasing
		AI_RemoveCurrentGoal( self );
		return;
	}

	int bEnemyVisible = AI_IsVisible( self, self->enemy );
	if ( bEnemyVisible )
	{
		if ( fDistance < HOVER_DISTANCE )
		{
			AI_RemoveCurrentTask( self, FALSE );
			return;
		}
		else
		{
			CVector destPoint = self->enemy->s.origin;
			destPoint.z += 24.0f;
			
			//NSS[10/25/99]:Added new flyTowardpoint code.
			AI_FlyTowardPoint2( self,destPoint, 0.15f);
		}

		// extend the time
		AI_SetTaskFinishTime( hook, 10.0f );
	}
	else
	{
		// test to see if the last path point is close to the enemy's position
		if ( !AI_IsPathToEntityClose( self, self->enemy ) )
		{
			AI_FindPathToEntity( self, self->enemy );
			if ( !hook->pPathList->pPath && AI_CanPath( hook ) )
			{
				// do not try another path for 5 seconds
				AI_SetNextPathTime( hook, 5.0f );

				//	no path was found, so give up
				AI_RemoveCurrentGoal (self);
				return;
			}
		}

		if ( AI_Fly( self ) == FALSE )
			PATHLIST_KillPath( hook->pPathList );
		
		hook->nMoveCounter++;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SKEETER_StartHover
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKEETER_StartHover( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	TASK_Set( pCurrentTask, gstate->time + 0.3f );

	AI_ZeroVelocity( self );

	AI_Dprintf( "Starting TASKTYPE_SKEETER_HOVER.\n" );

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 1.0f );

	self->nextthink	= gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
//
// Name:		SKEETER_Hover
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKEETER_Hover( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	if ( gstate->time > pAIData->fValue )
		AI_RemoveCurrentTask( self, FALSE );
}

// ----------------------------------------------------------------------------
//
// Name:		SKEETER_StartDartTowardEnemy
// Description:
//				move straight toward the enemy
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKEETER_StartDartTowardEnemy( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	AI_StartMove( self );

	float fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );
	if ( AI_IsWithinAttackDistance( self, fDistance ) )
	{
		AI_RemoveCurrentTask( self, FALSE );
		return;
	}		

	AI_FaceTowardPoint( self, self->enemy->s.origin );
	AI_UpdatePitchTowardEnemy( self );
	
	float fSpeed = AI_ComputeMovingSpeed( hook ) * 1.5f;

	CVector destPoint = self->enemy->s.origin;
	destPoint.z += 24.0f;
	CVector vectorTowardPoint = destPoint - self->s.origin;
	vectorTowardPoint.Normalize();

	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	self->velocity = vectorTowardPoint * (fSpeed * fScale);

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 3.0f );

	self->nextthink	= gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
//
// Name:		SKEETER_DartTowardEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKEETER_DartTowardEnemy( userEntity_t *self )
{
	_ASSERTE( self );
	_ASSERTE( self->enemy );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	float fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );
	if ( AI_IsWithinAttackDistance( self, fDistance - 32.0f ) )
	{
		AI_RemoveCurrentTask( self, FALSE );
		return;
	}		

	AI_FaceTowardPoint( self, self->enemy->s.origin );
	AI_UpdatePitchTowardEnemy( self );
	
	float fSpeed = AI_ComputeMovingSpeed( hook ) * 1.5f;

	CVector destPoint = self->enemy->s.origin;
	destPoint.z += 24.0f;
	CVector vectorTowardPoint = destPoint - self->s.origin;
	vectorTowardPoint.Normalize();

	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	self->velocity = vectorTowardPoint * (fSpeed * fScale);

	CVector org = self->s.origin;
	float fMoveDistance = hook->last_origin.Distance(org);
	if ( fMoveDistance < (self->velocity.Length() * 0.025f) )
	{
		if ( hook->nWaitCounter > 1 )
		{
			AI_RemoveCurrentTask( self, FALSE );
			return;
		}
		hook->nWaitCounter++;
	}
	else
		hook->nWaitCounter = 0;

	hook->last_origin = self->s.origin;
	ai_frame_sounds( self );
}

// ----------------------------------------------------------------------------
//
// Name:		SKEETER_StartFlyAway
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKEETER_StartFlyAway( userEntity_t *self )
{
	// find a point to fly away
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_StartMove( self );

	CVector flyAwayPoint;
	SKEETER_ComputeFlyAwayPoint( self, flyAwayPoint );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );
	pAIData->destPoint = flyAwayPoint;

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 3.0f );
	AI_Dprintf("Started Flyaway!!!\n");
	self->nextthink	= gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
//
// Name:		SKEETER_FlyAway
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKEETER_FlyAway( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );


	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	float fDistance = VectorDistance( self->s.origin, pAIData->destPoint );
	//NSS[10/25/99]:added new fly function
	AI_FlyTowardPoint2( self,pAIData->destPoint, 0.15f);
	if ( fDistance <= 64.0f )
	{
		AI_RemoveCurrentTask( self, FALSE );
		AI_SetOkToAttackFlag( hook, TRUE );
	}		
}

// ----------------------------------------------------------------------------
//
// Name:		SKEETER_StartPreHatch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKEETER_StartPreHatch( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_Dprintf( "Starting TASKTYPE_SKEETER_PREHATCH.\n" );

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1.0f );

	self->nextthink	= gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
//
// Name:		SKEETER_PreHatch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKEETER_PreHatch( userEntity_t *self )
{
	AI_RemoveCurrentTask( self );
}

// ----------------------------------------------------------------------------
//
// Name:		SKEETER_StartHatch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKEETER_StartHatch( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FREEZE )
		return;

	AI_ForceSequence( self, "speciala" ); // urgent, so must be forced

	AI_Dprintf( "Starting TASKTYPE_SKEETER_HATCH.\n" );

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1.0f );

	self->nextthink	= gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
//
// Name:		SKEETER_Hatch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKEETER_Hatch( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( strstr( hook->cur_sequence->animation_name, "special" ) )
	{
		if ( AI_IsEndAnimation( self ) )
		{
			GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
			GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
			if ( GOAL_GetType( pCurrentGoal ) == GOALTYPE_SKEETER_HATCH )
			{
				GOAL_Satisfied( pCurrentGoal );
			}

			AI_RemoveCurrentTask( self );
		}
		else
			self->s.origin.z += 4.0f;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		skeeter_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void skeeter_set_attack_seq( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );FRAMES_ComputeFrameScale
	AI_ForceSequence( self, "ataka" );
}

// ----------------------------------------------------------------------------
//
// Name:		skeeter_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKEETER_Attack( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );FRAMES_ComputeFrameScale
	AI_FaceTowardPoint( self, self->enemy->s.origin );
	AI_PlayAttackSounds( self );
	AI_Dprintf("Frame:%d \n",self->s.frame);

	
	if ( AI_IsReadyToAttack1( self ) )
	{
		if(!AI_DetectForLikeMonsters( self ))
		{
			ai_fire_curWeapon( self );
		}
		else
		{
			AI_RemoveCurrentTask( self, TASKTYPE_DODGE, self, FALSE );
			AI_AddNewTask(self, TASKTYPE_SKEETER_DARTTOWARDENEMY);
			AI_AddNewTask( self, TASKTYPE_SKEETER_ATTACK);
		}
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		AI_RemoveCurrentTask( self,TASKTYPE_SKEETER_FLYAWAY,self, FALSE );
	}
}

// ----------------------------------------------------------------------------
////NSS[10/25/99]:Modified
// Name:		skeeter_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKEETER_Begin_Attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	skeeter_set_attack_seq( self );
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1 );
	//Immediately face towards the enemy
	CVector dir = self->enemy->s.origin - self->s.origin;
	dir.Normalize();
	self->s.angles.yaw = VectorToYaw(dir);

	self->velocity.Zero();
}

// ----------------------------------------------------------------------------
// NSS[1/21/00]:
// Name:		SKEETER_ParseEpairs
// Description:Trap to watch skeeters think
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SKEETER_ParseEpairs(userEntity_t *self)
{
	AI_ParseEpairs(self);
	self->think = SKEETER_Think;
	self->nextthink = gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
////NSS[10/25/99]:Modified
// Name:		monster_slaughterskeet
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_slaughterskeet( userEntity_t *self )
{
	playerHook_t *hook = AI_InitMonster( self, TYPE_SL_SKEET );

	self->className		= MONSTERNAME;
	self->netname		= tongue_monsters[T_MONSTER_SLAUGHTERSKEET];

	char *szModelName = AIATTRIBUTE_GetModelName( self->className );
    _ASSERTE( szModelName );
    self->s.modelindex = gstate->ModelIndex( szModelName );

	if ( !ai_get_sequences(self) )
	{
		char *szCSVFileName = AIATTRIBUTE_GetCSVFileName( self->className );
        _ASSERTE( szCSVFileName );
        FRAMEDATA_ReadFile( szCSVFileName, self );
	}
	ai_register_sounds(self);


	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	self->movetype			= MOVETYPE_FLY;

    self->fragtype          |= FRAGTYPE_NOBLOOD;   // don't leave blood upon death
    self->fragtype          |= FRAGTYPE_ROBOTIC;   // apply robotic gib parts
	self->fragtype			|= FRAGTYPE_ALWAYSGIB;  // Always blow into small chunks

	hook->dflags			|= DFL_FACEPITCH;


	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );

	//////////////////////////////////////////////
	//////////////////////////////////////////////
	self->inventory = gstate->InventoryNew(MEM_MALLOC);

	if ( pWeaponAttributes )
	{
		self->curWeapon = ai_init_weapon( self, 
										  pWeaponAttributes[0].fBaseDamage, 
										  pWeaponAttributes[0].fRandomDamage,
										  pWeaponAttributes[0].fSpreadX,
										  pWeaponAttributes[0].fSpreadZ,
										  pWeaponAttributes[0].fSpeed,
										  pWeaponAttributes[0].fDistance,
										  pWeaponAttributes[0].offset, 
										  "punch", 
										  melee_punch, 
										  ITF_TRACE);
		gstate->InventoryAddItem(self, self->inventory, self->curWeapon);
	}

	hook->fnStartAttackFunc = SKEETER_Begin_Attack;
	hook->fnAttackFunc		= SKEETER_Attack;

	hook->nAttackType = ATTACK_AIR_MELEE;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	hook->strafe_time = gstate->time;
	hook->strafe_dir  = STRAFE_LEFT;

	self->think				= SKEETER_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;

	if(hook->active_distance < 600)
	{
		hook->active_distance = 4000.0f;
	}

	AI_DetermineMovingEnvironment( self );

    gstate->LinkEntity( self );

	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

	// NSS[6/7/00]:Reduce their speed on lower skill levels.
	CVector Speed;
	Speed.Set(0.55f,0.75f,1.0f);
	hook->run_speed		= hook->run_speed * AI_GetSkill(Speed);
	hook->walk_speed	= hook->walk_speed * AI_GetSkill(Speed);

}


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_skeeter_register_func()
{
	gstate->RegisterFunc("SKEETER_ParseEpairs",SKEETER_ParseEpairs);
	gstate->RegisterFunc("SKEETER_Begin_Attack",SKEETER_Begin_Attack);
	gstate->RegisterFunc("SKEETER_Attack",SKEETER_Attack);

	gstate->RegisterFunc("SKEETER_Think",SKEETER_Think);
	gstate->RegisterFunc("SKEETER_Chase",SKEETER_Chase);
	gstate->RegisterFunc("SKEETER_StartHover",SKEETER_StartHover);
	gstate->RegisterFunc("SKEETER_Hover",SKEETER_Hover);
	gstate->RegisterFunc("SKEETER_StartDartTowardEnemy",SKEETER_StartDartTowardEnemy);
	gstate->RegisterFunc("SKEETER_DartTowardEnemy",SKEETER_DartTowardEnemy);
	gstate->RegisterFunc("SKEETER_StartFlyAway",SKEETER_StartFlyAway);
	gstate->RegisterFunc("SKEETER_FlyAway",SKEETER_FlyAway);
	gstate->RegisterFunc("SKEETER_StartPreHatch",SKEETER_StartPreHatch);
	gstate->RegisterFunc("SKEETER_PreHatch",SKEETER_PreHatch);
	gstate->RegisterFunc("SKEETER_StartHatch",SKEETER_StartHatch);
	gstate->RegisterFunc("SKEETER_Hatch",SKEETER_Hatch);
	gstate->RegisterFunc("skeeter_set_attack_seq",skeeter_set_attack_seq);
}

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note: 
//
// ----------------------------------------------------------------------------

