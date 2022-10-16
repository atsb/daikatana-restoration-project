// ==========================================================================
//
//  File:
//  Contents:
//				skinny Worker
//  Author:
//
// ==========================================================================
#include "world.h"
#include "ai.h"
#include "ai_utils.h"
//#include "ai_move.h"// SCG[1/23/00]: not used
//#include "ai_weapons.h"// SCG[1/23/00]: not used
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"

/* ***************************** define types ****************************** */

#define MONSTERNAME			"monster_skinnyworker"

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_skinnyworker( userEntity_t *self );

int	SIDEKICK_FindFurthestNodeFrom (NodeTraverse *NodeT);
// ----------------------------------------------------------------------------
//
// Name:		WORKER_StartPain
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKINNYWORKER_StartPain(userEntity_t *self, userEntity_t *attacker, float kick, int damage)
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	_ASSERTE( attacker );
	if ( AI_IsAlive( self ) && attacker->flags & (FL_CLIENT + FL_MONSTER + FL_BOT) )
	{
		//	attacked by a player or bot so attack back
		if ( attacker->flags & (FL_CLIENT + FL_BOT) )
		{
			hook->ai_flags &= ~AI_IGNORE_PLAYER;
		}

		AI_EnemyAlert( self, attacker );

		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
		if ( !pCurrentGoal )
		{
			AI_AddNewGoal( self, GOALTYPE_IDLE );
			pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
		}

		GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
		if ( nGoalType != GOALTYPE_RUNAWAY )
		{
			AIDATA aiData;
			aiData.pEntity   = attacker;
			aiData.destPoint = self->s.origin;
			AI_AddNewGoal( self, GOALTYPE_RUNAWAY, &aiData );
			AI_AddNewTask( self, TASKTYPE_COWER, 1.0f );
		}
	}

	AI_StartPain(self,attacker,kick,damage);
}

// ----------------------------------------------------------------------------
// NSS[1/30/00]:
// Name:		WORKER_StartHide
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKINNYWORKER_StartHide( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	// NSS[1/30/00]:Making some er... changes... 
	if(hook)
	{
		float Rand = rnd();
		if(rnd() > 0.50)
		{
			if(Rand > 0.75f)
			{
				gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e1/Man_snifs.wav"),1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
			}
			else if(Rand > 0.45f)
			{
				gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e1/skinnydeath.wav"),1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
			}
			else
			{
				if(hook->type == TYPE_FATWORKER)
				{
					gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e1/Man_snifs.wav"),1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
				}
				else
				{
					gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e1/skinnydeath2.wav"),1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
				}
			}
		}
		AI_SetStateRunning( hook );
		self->nextthink	= gstate->time + 0.1f;
		AI_SetOkToAttackFlag( hook, FALSE );
		AI_SetTaskFinishTime( hook, 10.0f );
	}
}	

// ----------------------------------------------------------------------------
// NSS[1/30/00]:
// Name:		WORKER_Hide
// Description:Re-Wrote the hide function
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKINNYWORKER_Hide( userEntity_t *self )
{
	_ASSERTE( self );
	NodeTraverse				NodeT;
	playerHook_t *hook = AI_GetPlayerHook( self );
	if(hook)
	{
		if(hook->pNodeList)
		{
			
			// NSS[1/24/00]:Setup Start Point
			NodeT.origin				= self->enemy->s.origin;
			NodeT.NodeDepth				= 0;
			NodeT.CurrentDistance		= VectorXYDistance(self->s.origin,self->enemy->s.origin);
			NodeT.CurrentNodeIndex		= hook->pNodeList->nCurrentNodeIndex;
			NodeT.GreatestDistance		= 0.0f;
			NodeT.pNodeList				= hook->pNodeList;
			NodeT.MaximumNodeDepth		= 5;
			NodeT.pBestNode				= NULL;
			float fDistanceFrom			= VectorDistance(self->s.origin, self->enemy->s.origin);
			NodeT.MaximumDistance		= 312.0f + fDistanceFrom;
			
			// NSS[1/30/00]:Re-Using this nifty little function
			if(!SIDEKICK_FindFurthestNodeFrom (&NodeT))
			{
				// NSS[1/22/00]:If not then get the HideNode		
				if(!NodeT.pBestNode)
				{
					
					MAPNODE_PTR pRetreatNode = NODE_GetClosestCompleteHideNode( hook->pNodeList->pNodeHeader, self, self->enemy );
					if(!pRetreatNode)
					{
						//	nowhere to run to, hang out a bit
						AI_AddNewTaskAtFront( self, TASKTYPE_COWER );
						return;
					}
					NodeT.pBestNode = pRetreatNode;
				}		
			}
		}

		if(NodeT.pBestNode)
		{
			if(AI_IsCloseDistance2(self,VectorXYDistance(self->s.origin,NodeT.pBestNode->position)))
			{
				AI_AddNewTaskAtFront( self, TASKTYPE_COWER );
			}
			else
			{
				AI_AddNewTaskAtFront(self,TASKTYPE_MOVETOLOCATION, NodeT.pBestNode->position);
			}
		}
		else
		{
			AI_Dprintf("Sidekick Trying to Evade but finding no path or nodes to use!!!\n");
			AI_AddNewTaskAtFront( self, TASKTYPE_COWER );	
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		WORKER_StartBackWall
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKINNYWORKER_StartBackWall( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_SKINNYWORKER_BACKWALL );

	// watch out for enemy
	if ( AI_IsVisible(self, self->enemy) )
	{
		AI_RestartCurrentGoal( self );
		return;
	}

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );

	// search out for a nearest wall
//	CVector origin = self->s.origin;

	CVector forwardVector;
	YawToVector( self->s.angles.yaw, forwardVector );

	float fWidth = fabs(self->s.maxs.y - self->s.mins.y) * 0.6f;

	int bMoving = TRUE;
	CVector endPoint;
	endPoint.x = self->s.origin.x + (forwardVector.x * 64.0f);
	endPoint.y = self->s.origin.y + (forwardVector.y * 64.0f);
	endPoint.z = self->s.origin.z;
	tr = gstate->TraceLine_q2( self->s.origin, endPoint, self, MASK_SOLID );
	if ( tr.fraction < 1.0f )
	{
		float fLength = (tr.fraction * 64.0f) - fWidth;
		CVector destPoint;
		destPoint.x = self->s.origin.x + (forwardVector.x * fLength);
		destPoint.y = self->s.origin.y + (forwardVector.y * fLength);
		destPoint.z = self->s.origin.z;
		TASK_Set( pCurrentTask, destPoint );
	}
	else
	{
		// left side
		CVector leftVector( -forwardVector.y, forwardVector.x, 0.0f );
		endPoint.x = self->s.origin.x + (leftVector.x * 64.0f);
		endPoint.y = self->s.origin.y + (leftVector.y * 64.0f);
		endPoint.z = self->s.origin.z;
		tr = gstate->TraceLine_q2( self->s.origin, endPoint, self, MASK_SOLID );
		if ( tr.fraction < 1.0f )
		{
			float fLength = (tr.fraction * 64.0f) - fWidth;
			CVector destPoint;
			destPoint.x = self->s.origin.x + (leftVector.x * fLength);
			destPoint.y = self->s.origin.y + (leftVector.y * fLength);
			destPoint.z = self->s.origin.z;
			TASK_Set( pCurrentTask, destPoint );
		}
		else
		{
			// right side
			CVector rightVector( forwardVector.y, -forwardVector.x, 0.0f );
			endPoint.x = self->s.origin.x + (rightVector.x * 64.0f);
			endPoint.y = self->s.origin.y + (rightVector.y * 64.0f);
			endPoint.z = self->s.origin.z;
			tr = gstate->TraceLine_q2( self->s.origin, endPoint, self, MASK_SOLID );
			if ( tr.fraction < 1.0f )
			{
				float fLength = (tr.fraction * 64.0f) - fWidth;
				CVector destPoint;
				destPoint.x = self->s.origin.x + (rightVector.x * fLength);
				destPoint.y = self->s.origin.y + (rightVector.y * fLength);
				destPoint.z = self->s.origin.z;
				TASK_Set( pCurrentTask, destPoint );
			}
			else
			{
				TASK_Set( pCurrentTask, self->s.origin );
				bMoving = FALSE;
			}
		}
	}

	if ( bMoving )
	{
		AI_SetStateWalking( hook );
		if ( AI_StartMove( self ) == FALSE )
		{
			return;
		}
	}

	AI_Dprintf( "Starting TASKTYPE_SKINNYWORKER_BACKWALL.\n" );

	self->nextthink	= gstate->time + 0.1f;

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 4.0f );
}

// ----------------------------------------------------------------------------
//
// Name:		WORKER_BackWall
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKINNYWORKER_BackWall( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	// watch out for enemy
	if ( AI_IsVisible(self, self->enemy) )
	{
		AI_RestartCurrentGoal( self );
		return;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsExactDistance( hook, fXYDistance ) )
	{
		AI_RemoveCurrentTask( self, FALSE );
		return;
	}

	forward = pAIData->destPoint - self->s.origin;
	forward.Normalize();

	// facing away from the wall, which is the opposite of the forward vector
	CVector awayPoint;
	awayPoint.x = self->s.origin.x + (-forward.x * 32.0f);
	awayPoint.y = self->s.origin.y + (-forward.y * 32.0f);
	awayPoint.z = self->s.origin.z;
	AI_FaceTowardPoint( self, awayPoint );

	float fSpeed = AI_ComputeMovingSpeed( hook );
	if ( AI_IsCloseDistance2( self, fXYDistance ) )
	{
		fSpeed = fXYDistance * 10.0f;
	}
	float fOneFrameSpeed = fSpeed * 0.1f;
	CVector checkPoint;
	VectorMA( self->s.origin, forward, fOneFrameSpeed, checkPoint );
	tr = gstate->TraceLine_q2( self->s.origin, checkPoint, self, MASK_SOLID );
	if ( tr.fraction < 1.0f )
	{
		float fWidth = fabs(self->s.maxs.y - self->s.mins.y) * 0.6f;
		float fDistanceToWall = tr.fraction * fOneFrameSpeed;
		if ( fDistanceToWall < fWidth )
		{
			AI_RemoveCurrentTask( self, FALSE );
			return;
		}
	}

	// backup against the wall, which means face the opposite of moving direction 
	float fScale = FRAMES_ComputeFrameScale( hook );
	self->velocity = forward * (fSpeed * fScale);

	hook->last_origin = self->s.origin;

	// update the current node for this entity
	NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	node_find_cur_node( self, pNodeList );

	ai_frame_sounds( self );
}

// ----------------------------------------------------------------------------
//
// Name:		WORKER_StartLookOut
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKINNYWORKER_StartLookOut( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_SKINNYWORKER_LOOKOUT );

	// watch out for enemy
	if ( AI_IsVisible(self, self->enemy) )
	{
		AI_RestartCurrentGoal( self );
		return;
	}

	// play a look out animation
	if ( AI_StartSequence(self, "damba") == FALSE )
	{
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_SKINNYWORKER_LOOKOUT.\n" );

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1.0f );

	// set a time limit on cowering, after this amount it goes back to previous tasks
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData );
	pAIData->fValue = gstate->time + 90.0f;
}

// ----------------------------------------------------------------------------
//
// Name:		WORKER_LookOut
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKINNYWORKER_LookOut( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	// watch out for enemy
	if ( AI_IsVisible(self, self->enemy) )
	{
		AI_RestartCurrentGoal( self );
		return;
	}

	if ( AI_IsEndAnimation( self ) )
	{
		self->s.frameInfo.frameState = 0;

		// play another animation
		if ( rnd() > 0.75f )
		{
			if ( AI_StartSequence(self, "dambb") == FALSE )
			{
				return;
			}
		}
		else
		{
			if ( AI_StartSequence(self, "damba") == FALSE )
			{
				return;
			}
		}
	}

	float fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );
	if ( fDistance > hook->active_distance )
	{
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
		AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
		_ASSERTE( pAIData );
		if ( gstate->time > pAIData->fValue )
		{
			GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
			_ASSERTE( pCurrentGoal );
			AIDATA_PTR pGoalData = GOAL_GetData( pCurrentGoal );
			CVector destPoint = pGoalData->destPoint;

			AI_RemoveCurrentGoal( self );

			AI_AddNewGoal( self, GOALTYPE_MOVETOLOCATION, destPoint );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		WORKER_StartCower
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKINNYWORKER_StartCower( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_COWER );

	// play another animation
	if ( rnd() > 0.5f )
	{
		AI_ForceSequence(self, "gamba", FRAME_LOOP);
	}
	else
	{
		AI_ForceSequence(self, "gambc", FRAME_LOOP);
	}

	AI_Dprintf( "Starting TASKTYPE_SKINNYWORKER_COWER.\n" );

	self->delay = gstate->time + 10.0;
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1.0f );
}

// ----------------------------------------------------------------------------
//
// Name:		WORKER_Cower
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SKINNYWORKER_Cower( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData );

	// watch out for enemy
	float fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );
	if ((fDistance < 300.0f && AI_IsLineOfSight(self, self->enemy)) && self->delay < gstate->time)
	{
		AI_RestartCurrentGoal( self );
		//pAIData->fValue = gstate->time + 30.0f;
        return;
	}
	if ( fDistance > hook->active_distance )
	{
		if ( gstate->time > pAIData->fValue )
		{
			self->enemy = NULL;
            GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
			_ASSERTE( pCurrentGoal );
			AIDATA_PTR pGoalData = GOAL_GetData( pCurrentGoal );
			CVector destPoint = pGoalData->destPoint;

			AI_RemoveCurrentGoal( self );

			AI_AddNewGoal( self, GOALTYPE_MOVETOLOCATION, destPoint );
		}
	}
}


// ----------------------------------------------------------------------------
//
// Name:		monster_skinnyworker
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_skinnyworker(userEntity_t *self)
{
	playerHook_t *hook		= AI_InitMonster(self, TYPE_SKINNYWORKER);

	self->className			= MONSTERNAME;
	self->netname			= tongue_monsters[T_MONSTER_SKINNYWORKER];

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

//	hook->ai_flags			|= AI_IGNORE_PLAYER;

	hook->fnStartAttackFunc = NULL;
	hook->fnAttackFunc		= NULL;
    hook->fnStartCower		= SKINNYWORKER_StartCower;
	hook->fnCower			= SKINNYWORKER_Cower;

    hook->walk_speed		= 35;

	self->pain				= SKINNYWORKER_StartPain;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;


//	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );
	AIATTRIBUTE_SetInfo( self );// SCG[1/23/00]: pWeaponAttributes not used
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
void world_ai_skinnyworker_register_func()
{
	gstate->RegisterFunc("SKINNYWORKER_StartPain",SKINNYWORKER_StartPain);
	gstate->RegisterFunc("AI_ParseEpairs",AI_ParseEpairs);

}
