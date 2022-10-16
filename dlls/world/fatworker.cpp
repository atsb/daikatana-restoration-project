// ==========================================================================
//
//  File:
//  Contents:
//				fat worker
//  Author:
//
// ==========================================================================
#include "world.h"
#include "ai.h"
#include "ai_utils.h"
//#include "ai_move.h"		// SCG[1/23/00]: not used
//#include "ai_weapons.h"	// SCG[1/23/00]: not used
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"

/* ***************************** define types ****************************** */

#define MONSTERNAME			"monster_fatworker"

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_fatworker( userEntity_t *self );



// ----------------------------------------------------------------------------
//
// Name:		WORKER_StartPain
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void FATWORKER_StartPain(userEntity_t *self, userEntity_t *attacker, float kick, int damage)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	// still in a pain function from last hit?
	if ( hook->pain_finished >= gstate->time || self->deadflag != DEAD_NO )
	{
		AI_StartPain(self,attacker,kick,damage);
		return;
	}

	// retreat to a safe location
	_ASSERTE( attacker );
	if ( attacker->flags & (FL_CLIENT + FL_MONSTER + FL_BOT) )
	{
		AI_EnemyAlert( self, attacker );

		//AI_AddNewGoal( self, GOALTYPE_SKINNYWORKER_HIDE );
	}
	AI_StartPain(self,attacker,kick,damage);
}

// ----------------------------------------------------------------------------
//
// Name:		WORKER_StartHide
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void FATWORKER_StartHide( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	//_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_SKINNYWORKER_HIDE );

	if(rnd() < 0.75)
	{
		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e1/fart4.wav"), 1.0f, ATTN_NORM_MIN,ATTN_NORM_MAX);
	}
	
	AI_SetStateRunning( hook );
	if ( AI_StartMove( self ) == FALSE )
	{
		return;
	}

//	int bFindAlternateSpot = TRUE;	// SCG[1/23/00]: not referenced
	NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( self );
	if ( pNodeHeader )
	{
		MAPNODE_PTR pHideNode = NULL;
		MAPNODE_PTR pHideNodes[MAX_CLOSEST_HIDENODES];
		int nNumHideNodes = NODE_GetClosestHideNodes( pNodeHeader, self, self->enemy, pHideNodes );
		
		float fMinAngle = 180.0f;
		for ( int i = 0; i < nNumHideNodes; i++ )
		{
			MAPNODE_PTR pNode = pHideNodes[i];

			CVector vector1 = pNode->position - self->s.origin;
			vector1.Normalize();
			CVector vector2 = self->enemy->s.origin - self->s.origin;
			vector2.Normalize();

			float fAngle = ComputeAngle2D( vector1, vector2 );
			float fAngleDiff = fabs(180.0f - fAngle);
			if ( fAngleDiff < fMinAngle )
			{
				fMinAngle = fAngleDiff;
				pHideNode = pNode;
			}
		}

		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
		if ( pHideNode )
		{
			AI_FindPathToNode( self, pHideNode );
		
			TASK_Set( pCurrentTask, pHideNode->position );
		}
		else
		{
			// set to current position as the hide point
			TASK_Set( pCurrentTask, self->s.origin );
		}
	}
	else
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	//AI_Dprintf( "Starting TASKTYPE_SKINNYWORKER_HIDE.\n" );

	self->nextthink	= gstate->time + 0.1f;

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 15.0f );
}	

// ----------------------------------------------------------------------------
//
// Name:		WORKER_Hide
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void FATWORKER_Hide( userEntity_t *self )
{
	_ASSERTE( self );

	playerHook_t *hook = AI_GetPlayerHook( self );
//	userEntity_t *pEnemy = self->enemy;	// SCG[1/23/00]: not referenced

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	float fDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsCloseDistance2( self, fDistance ) && fZDistance < 32.0f )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	tr = gstate->TraceLine_q2( self->s.origin, pAIData->destPoint, self, MASK_SOLID );
	if ( (tr.fraction >= 1.0f && fZDistance < MAX_JUMP_HEIGHT) || 
		 (tr.fraction >= 0.8f && ((1.2f-tr.fraction) * (fDistance+fZDistance)) < 32.0f) )
	{
		AI_MoveTowardPoint( self, pAIData->destPoint, FALSE );
	}
	else
	{
		if ( !AI_HandleUse( self ) )
		{
			if ( hook->pPathList->pPath )
			{
				if ( AI_Move( self ) == FALSE )
				{
					if ( !AI_FindPathToPoint( self, pAIData->destPoint ) )
					{
						AI_RestartCurrentGoal( self );
					}
				}					
			}
			else
			{
				if ( !AI_FindPathToPoint( self, pAIData->destPoint ) )
				{
					AI_RestartCurrentGoal( self );
				}
			}
		}
	}
}


// ----------------------------------------------------------------------------
//
// Name:		monster_fatworker
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_fatworker(userEntity_t *self)
{
	playerHook_t *hook		= AI_InitMonster(self, TYPE_FATWORKER);

	self->className			= MONSTERNAME;
	self->netname			= tongue_monsters[T_MONSTER_FATWORKER];

	char *szModelName = AIATTRIBUTE_GetModelName( self->className );
    _ASSERTE( szModelName );
    self->s.modelindex = gstate->ModelIndex( szModelName );

	if ( !ai_get_sequences(self) )
	{
		char *szCSVFileName = AIATTRIBUTE_GetCSVFileName( self->className );
        _ASSERTE( szCSVFileName );
        FRAMEDATA_ReadFile( szCSVFileName, self );
	}
    ai_register_sounds( self );

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	hook->fnStartAttackFunc = NULL;
	hook->fnAttackFunc		= NULL;

    hook->walk_speed		= 35;
	self->fragtype			|= FRAGTYPE_ALWAYSGIB;
	self->pain				= FATWORKER_StartPain;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;

//	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );
	AIATTRIBUTE_SetInfo( self );

	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;

	AI_DetermineMovingEnvironment( self );

    gstate->LinkEntity( self );

	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

}


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_fatworker_register_func()
{
	gstate->RegisterFunc("FATWORKER_StartPain",FATWORKER_StartPain);
}
