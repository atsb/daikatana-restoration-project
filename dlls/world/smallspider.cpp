// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#include "world.h"
#include "ai_common.h"
#include "ai_utils.h"
//#include "ai_move.h"// SCG[1/23/00]: not used
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"
//#include "spawn.h"// SCG[1/23/00]: not used
#include "ai.h"
#include "action.h"
//#include "actorlist.h"// SCG[1/23/00]: not used

/* ***************************** define types ****************************** */
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
void smallspider_jump_attack(userEntity_t *self);
void smallspider_melee_attack(userEntity_t *self);

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_smallspider (userEntity_t *self);

// ----------------------------------------------------------------------------
//
// Name:		SMALLSPIDER_RunAway
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SMALLSPIDER_RunAway( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
//	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);// SCG[1/23/00]: not used

	if ( hook->fTaskFinishTime >= 0.0f && gstate->time > hook->fTaskFinishTime )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	userEntity_t *pEnemy = self->enemy;
    if ( !AI_IsAlive( pEnemy ) )
    {
        AI_RemoveCurrentTask( self );
        return;
    }

	AI_Move( self );

	if ( !hook->pPathList->pPath )
	{
		//	set up a path consisting of one node
		MAPNODE_PTR pRetreatNode = NODE_FurthestFrom( hook->pNodeList, pEnemy->s.origin );
		if ( !pRetreatNode )
		{
			//	nowhere to run to, start cower
			AI_AddNewTaskAtFront( self, TASKTYPE_COWER );
			return;
		}

		PATHLIST_AddNodeToPath( hook->pPathList, pRetreatNode );

	    float fDistance = VectorDistance( self->s.origin, pRetreatNode->position );
	    float fSpeed = AI_ComputeMovingSpeed( hook );
	    float fTime = (fDistance / fSpeed) + 1.0f;
	    AI_SetTaskFinishTime( hook, fTime );

        AI_Move( self );
	}

}

// ----------------------------------------------------------------------------
//
// Name:		SMALLSPIDER_StartRunAway
// Description:	
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SMALLSPIDER_StartRunAway( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_SMALLSPIDER_RUNAWAY );

	userEntity_t *pEnemy = self->enemy;
    if ( !AI_IsAlive( pEnemy ) )
    {
        AI_RemoveCurrentTask( self );
        return;
    }

    AI_SetStateRunning( hook );
	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_SMALLSPIDER_RUNAWAY )
		{
			return;
		}
	}

	//	delete any previous path
	PATHLIST_KillPath( hook->pPathList );

	//	set up a path consisting of one node
	MAPNODE_PTR pRetreatNode = NODE_FurthestFrom(hook->pNodeList, pEnemy->s.origin);
	if ( !pRetreatNode )
	{
		//	nowhere to run to, start cowering
		AI_RemoveCurrentTask( self, TASKTYPE_COWER );

		return;
	}

	AI_Dprintf( "Starting TASKTYPE_SMALLSPIDER_RUNAWAY.\n" );

	PATHLIST_AddNodeToPath(hook->pPathList, pRetreatNode);

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	AI_SetTaskFinishTime( hook, pAIData->fValue );

	AI_SetMovingCounter( hook, 0 );

    AI_Move( self );
}

// ----------------------------------------------------------------------------
//NSS[11/14/99]:
// Name:		SMALLSPIDER_StartPain
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SMALLSPIDER_StartPain(userEntity_t *self, userEntity_t *attacker, float kick, int damage)
{
	_ASSERTE( self );
	//NSS[11/14/99]:
	// new enemy sighted, attack!
	AI_EnemyAlert( self, attacker );
	// runaway for a few seconds
    AI_AddNewTaskAtFront( self, TASKTYPE_SMALLSPIDER_RUNAWAY, rnd()*5.0f + 2.0f );
	AI_StartPain(self,attacker,kick,damage);
}

// ----------------------------------------------------------------------------
//NSS[11/14/99]:
// Name:		spider_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void smallspider_set_attack_seq (userEntity_t *self)
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	frameData_t *pSequence = NULL;
	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	if ( AI_IsWithinAttackDistance( self, dist ) )
	{
		pSequence = FRAMES_GetSequence( self, "ataka" );
		AI_ForceSequence(self, pSequence);
		self->s.frameInfo.frameInc = 2;

		smallspider_melee_attack (self);
	}
	else
	{
		pSequence = FRAMES_GetSequence( self, "jumpa" );
		AI_ForceSequence(self, pSequence);

		smallspider_jump_attack (self);
	}
}

// ----------------------------------------------------------------------------
//NSS[11/14/99]:
// Name:		smallspider_jump_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void smallspider_jump_attack (userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	AI_FaceTowardPoint( self, self->enemy->s.origin );

	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	if ( self->s.frameInfo.frameState & FRSTATE_PLAYSOUND1 )
	{
		CVector jump_ang, vel;

		//	jump on this frame
		//NSS[11/14/99]:HUH?!  Why would someone want to do this!?
		//self->groundEntity = NULL;

		jump_ang = self->s.angles;
		jump_ang.AngleToVectors(forward, right, up);

		vel = forward * hook->run_speed * 1.5;
		self->velocity = vel;
		self->velocity.z = hook->upward_vel;

		self->s.frameInfo.frameState -= FRSTATE_PLAYSOUND1;

		if ( hook->sound1 && !AI_IsSoundDisabled() )
		{
			gstate->StartEntitySound (self, CHAN_AUTO, gstate->SoundIndex(hook->sound1), 0.65f, hook->fMinAttenuation, hook->fMaxAttenuation);
		}
	}
	
	if (self->s.frameInfo.frameState & FRSTATE_PLAYSOUND2)
	{
		//	attack on this frame
		self->s.frameInfo.frameState -= FRSTATE_PLAYSOUND2;
				
		//ai_fire_curWeapon( self );
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		{
			AI_RemoveCurrentTask( self );

			return;
		}
		else
		{
			if ( rnd() > 0.5f )
			{
				AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP );
			}
			else
			{
				smallspider_set_attack_seq (self);
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		smallspider_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void smallspider_melee_attack(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) )
	{
		ai_fire_curWeapon( self );
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		{
			AI_RemoveCurrentTask( self );

			return;
		}
		else
		{
			if ( rnd() > 0.5f && AI_CanMove( hook ) )
			{
				AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP );
			}
			else
			{
				smallspider_set_attack_seq (self);
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		smallspider_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void smallspider_attack(userEntity_t *self)
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	if ( AI_IsWithinAttackDistance( self, dist ) )
	{
		smallspider_melee_attack (self);
	}
	else
	{
		smallspider_jump_attack (self);
	}
}


// ----------------------------------------------------------------------------
//
// Name:		smallspider_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void smallspider_begin_attack (userEntity_t *self)
{
	self->velocity.Zero();

	smallspider_set_attack_seq (self);
}

// ----------------------------------------------------------------------------
//
// Name:		smallspider_start_pain
// Description: pain function for smallspider that selects front or back hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void smallspider_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,15);
}


// ----------------------------------------------------------------------------
// NSS[3/9/00]:
// Name:		SMALLSPIDER_Activate
// Description: Will set the small spiders into violent mode!
// Input:userEntity_t *self, userEntity_t *other, userEntity_t *activator
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void SMALLSPIDER_Activate(userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	if(hook)
	{
		// remove the ignore player flag
		hook->ai_flags &= ~AI_IGNORE_PLAYER;

		//If we don't have a find enemy function then get outta here!
		if(!hook->fnFindTarget)
			return;
		
		//Find an enemy
		userEntity_t *pEnemy = hook->fnFindTarget( self );
		
		//If found then let's rock and roll!
		if(pEnemy)
		{
			AI_SetEnemy( self, pEnemy );
		}
	}
}


// ----------------------------------------------------------------------------
//NSS[11/14/99]:
// Name:		monster_smallspider
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_smallspider(userEntity_t *self)
{
	playerHook_t *hook		= AI_InitMonster(self, TYPE_SPIDER);

	self->className			= "monster_smallspider";
	self->netname			= tongue_monsters[T_MONSTER_SMALLSPIDER];

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

	///////////////////////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////////////////////

	hook->fnStartAttackFunc = smallspider_begin_attack;
	hook->fnAttackFunc		= smallspider_attack;

	hook->dflags			|= DFL_JUMPATTACK;

	self->pain				= SMALLSPIDER_StartPain;
    self->think				= AI_ParseEpairs;

	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give smallspider a weapon
	///////////////////////////////////////////////////////////////////////////
	self->inventory = gstate->InventoryNew (MEM_MALLOC);

	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );
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
										  ITF_TRACE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}
	hook->nAttackType = ATTACK_GROUND_MELEE;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	self->use		  = SMALLSPIDER_Activate;

	AI_DetermineMovingEnvironment( self );

	gstate->LinkEntity(self);
	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

	
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



///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_smallspider_register_func()
{
	gstate->RegisterFunc("SMALLSPIDER_StartPain",SMALLSPIDER_StartPain);
	gstate->RegisterFunc("smallspider_begin_attack",smallspider_begin_attack);
	gstate->RegisterFunc("smallspider_attack",smallspider_attack);
	gstate->RegisterFunc("SMALLSPIDER_Activate",SMALLSPIDER_Activate);
}
