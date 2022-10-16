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
#include <stdlib.h>
#include "dk_std.h"
#include "world.h"
#include "ai_utils.h"
#include "ai_move.h"
#include "ai_frames.h"
#include "ai.h"
#include "actorlist.h"
#include "nodelist.h"
#include "GrphPath.h"
#include "ai_func.h"
#include "ai_weapons.h"
#include "collect.h"
#include "action.h"
#include "spawn.h"
#include "Sidekick.h"
#include "ai_info.h"
#include "SequenceMap.h"
#include "ai_think.h"
#include "ai_save.h"



/* ***************************** define types **************************** */

#define SIDESTEP_DISTANCE	96.0f

/* ***************************** Local Variables **************************** */

/* ***************************** Local Functions **************************** */
int Check_Que(userEntity_t *other,int Type, float Offset);
int SIDEKICK_SendMessage(userEntity_t *other, int command, int An_Int, float A_Float, float B_Float, userEntity_t *sender,int Flag);
int AI_HandleGettingOffPlatformAndTrain( userEntity_t *self );

int AI_HasTaskInQue( userEntity_t *self, TASKTYPE SearchTask );

float Random();

int	AI_IsDirectPath( userEntity_t *self, CVector &destPoint );

void AI_FaceTowardPoint( userEntity_t *self, CVector &destPoint );

int AI_FindNextPathCorner( userEntity_t *self );

userEntity_t * FindEntityByClassName(userEntity_t *self, char *classname, float Distance);

void AI_Pain( userEntity_t *self );
void AI_StartPain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );

void AI_Die( userEntity_t *self );
void AI_StartDie( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &destPoint );

void AI_StartIdle( userEntity_t *self );
void AI_Idle( userEntity_t *self );

void AI_JumpUp( userEntity_t *self );
void AI_StartJumpUp( userEntity_t *self );

void AI_JumpForward( userEntity_t *self );
void AI_StartJumpForward( userEntity_t *self );

void AI_StartAttack( userEntity_t *self );
void AI_Attack( userEntity_t *self );

int AI_Move( userEntity_t *self );

void AI_StartChase( userEntity_t *self );
void AI_Chase( userEntity_t *self );

void AI_Freeze( userEntity_t *self );

// following not yet implemented
void AI_StartPatrol( userEntity_t *self );
void AI_Patrol( userEntity_t *self );

void AI_StartWander( userEntity_t *self );
void AI_Wander( userEntity_t *self );

void AI_StartMoveToLocation( userEntity_t *self );
void AI_MoveToLocation( userEntity_t *self );

void AI_StartGoingAroundObstacle( userEntity_t *self );
void AI_GoingAroundObstacle( userEntity_t *self );


int AI_StartFly( userEntity_t *self );
int AI_Fly( userEntity_t *self );

void AI_StartFlyToLocation( userEntity_t *self );
void AI_FlyToLocation( userEntity_t *self );

void AI_StartSwoopAttack( userEntity_t *self );
void AI_SwoopAttack( userEntity_t *self );

void AI_StartTowardIntermediatePoint( userEntity_t *self );
void AI_TowardIntermediatePoint( userEntity_t *self );

void AI_StartAirRangedAttack( userEntity_t *self );
void AI_AirRangedAttack( userEntity_t *self );

void AI_StartAirMeleeAttack( userEntity_t *self );
void AI_AirMeleeAttack( userEntity_t *self );

int AI_StartSwim( userEntity_t *self );
int AI_SwimTowardPoint( userEntity_t *self, CVector &destPoint, int bMovingTowardNode );
int AI_Swim( userEntity_t *self );
void AI_StartSwimToLocation( userEntity_t *self );
void AI_SwimToLocation( userEntity_t *self );

void AI_StartFaceAngle( userEntity_t *self );
void AI_FaceAngle( userEntity_t *self );

void AI_StartWait( userEntity_t *self );
void AI_Wait( userEntity_t *self );


void AI_StartBackupAllAttributes( userEntity_t *self );
void AI_StartRestoreAllAttributes( userEntity_t *self );
void AI_StartModifyTurnAttribute( userEntity_t *self );
void AI_StartModifyRunAttribute( userEntity_t *self );

void AI_StartPlayAnimation(userEntity_t *self);
void AI_StartSetIdleAnimation(userEntity_t *self);

void AI_StartUsingWalkSpeed( userEntity_t *self );
void AI_StartUsingRunSpeed( userEntity_t *self );

int AI_IsTrainStopped( userEntity_t *pTrain );
void AI_StartActivateSwitch( userEntity_t *self );
void AI_StartWaitForTrainToCome( userEntity_t *self );
void AI_WaitForTrainToCome( userEntity_t *self );
void AI_StartWaitForTrainToStop( userEntity_t *self );
void AI_WaitForTrainToStop( userEntity_t *self );

void AI_StartMoveToEntity( userEntity_t *self );
void AI_MoveToEntity( userEntity_t *self );

int AI_HandleTrainUse( userEntity_t *self, NODEHEADER_PTR pNodeHeader,
					   MAPNODE_PTR pCurrentNode, MAPNODE_PTR pNextNode );
int AI_HandleDoorUse( userEntity_t *self, NODEHEADER_PTR pNodeHeader,
					  MAPNODE_PTR pCurrentNode, MAPNODE_PTR pNextNode );
int AI_HandleLadderUse( userEntity_t *self, NODEHEADER_PTR pNodeHeader,
   						MAPNODE_PTR pCurrentNode, MAPNODE_PTR pNextNode );
int AI_HandleUse( userEntity_t *self );

userEntity_t *AI_GetSwitchOnTrain( userEntity_t *self );
userEntity_t *AI_GetClosestSwitch( userEntity_t *pTrain, CVector &currentPosition );

void AI_StartTakeCover( userEntity_t *self );
void AI_TakeCover( userEntity_t *self );

void AI_SetSpawnValue( userEntity_t *self, int nSpawnValue );

void AI_StartGoingAroundEntity( userEntity_t *self );
void AI_GoingAroundEntity( userEntity_t *self );

void AI_RemoveIdleScriptGoal(userEntity_t *self);

int AI_IsWaterUnit( userEntity_t *self );
int AI_IsInWater( userEntity_t *self );

int AI_IsFacingTowardPoint( userEntity_t *self, CVector &point );
int AI_StartMoveBackwards( userEntity_t *self );

void AI_FollowWalking( userEntity_t *self );
void AI_FollowRunning( userEntity_t *self );
void AI_FollowSidekickWalking( userEntity_t *self );
void AI_FollowSidekickRunning( userEntity_t *self );

float sin_tbl[] = {0.017,0.515,0.874,0.999,0.857,0.484,-0.017,-0.515,-0.874,-0.999,-0.857,-0.484};
float cos_tbl[] = {0.999,0.857,0.484,-0.017,-0.515,-0.874,-0.999,-0.857,-0.484,0.017,0.515,0.874};
int Check_Que(userEntity_t *other,int Type, float Offset);
int SideKick_TalkAmbient(userEntity_t *self, enum Sidekick_Talk VoiceType, int OverRide_Time);

/* ***************************** Global Variables *************************** */


//*******************************************************************************
//							AI functions
//*******************************************************************************

//*******************************************************************************
//						helper functions
//*******************************************************************************

int AI_IsOnTrain( userEntity_t *self );








void AI_SelectAnimationPerWeaponType( userEntity_t *self, char *pszAnimation )
{
	weaponInfo_t *pWeaponInfo = (weaponInfo_t *)self->winfo;
	if ( !pWeaponInfo )
	{
		return;
	}

	int nStrLength = strlen( pszAnimation );
	switch( pWeaponInfo->nWeaponType )
	{
		case WEAPONTYPE_GLOVE:
		{
			break;
		}
		case WEAPONTYPE_PISTOL:
		{
			pszAnimation[nStrLength] = 'a';
			pszAnimation[nStrLength+1] = '\0';
			break;
		}
		case WEAPONTYPE_RIFLE:
		{
			pszAnimation[nStrLength] = 'b';
			pszAnimation[nStrLength+1] = '\0';
			break;
		}
		case WEAPONTYPE_SHOULDER:
		{
			pszAnimation[nStrLength] = 'b';//'c';
			pszAnimation[nStrLength+1] = '\0';
			break;
		}
		default:
			break;
	}
}


// ----------------------------------------------------------------------------
// NSS[12/7/99]:
// Name:        AI_GibLimit
// Description: Determine if we should gib or not.
// Input:userEntity_t *self,float damage
// Output:
// Note:This will take the gib_damage->value and will determine if we should explode.
// ----------------------------------------------------------------------------
int AI_GibLimit(userEntity_t *self, float damage)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	if(hook)
	{
		if((( damage >= (gib_damage_point->value * hook->base_health)) && !AI_IsAlive(self)) || self->health < (-0.50*hook->base_health))
			return TRUE;

	}
	return FALSE;
}


// ----------------------------------------------------------------------------

void AI_SelectAmbientAnimation( userEntity_t *self, char *pszAnimation )

{
	_ASSERTE( self );
	_ASSERTE( pszAnimation );

	playerHook_t *hook = AI_GetPlayerHook( self );
	if ( hook->type == TYPE_BOT || AI_IsSidekick( hook ) || hook->type == TYPE_CLIENT )
	{
		//if(hook->bCarryingMikiko)
		//	strcpy( pszAnimation, "amba" );
		if( AI_IsCrouching( hook ))
			strcpy( pszAnimation, "camb" );
		else if( hook->bCarryingMikiko )
			strcpy( pszAnimation, "amba" );
		else
			strcpy( pszAnimation, "aamb" );
        AI_SelectAnimationPerWeaponType( self, pszAnimation );
	}
	else
	{
		frameData_t *pSequence;
		float chanceleft = 1.0f;
		char animchar = 'a';

		// start with amba
        strcpy( pszAnimation, "amba" );

		pSequence = FRAMES_GetSequence( self, pszAnimation );

		// check to see if the chance for this sequence indicates that
		// we should advance to the next one instead
		while ( pSequence && rnd() > pSequence->fAnimChance / chanceleft && animchar < 'z' )
		{
			// remove the chance of this animation from the total chance
			chanceleft -= pSequence->fAnimChance;

			// cap chance remaining to 0.1%
			if ( chanceleft < 0.0001f )
				chanceleft = 0.0001f;

			animchar++;
			pszAnimation[3] = animchar;

			// update current sequence
			pSequence = FRAMES_GetSequence( self, pszAnimation );

			if(!pSequence) // No sequence there?
			{
				// whoops, backtrack one
				animchar--;
				pszAnimation[3] = animchar;
			}
		}
	}
}

// ----------------------------------------------------------------------------

void AI_SelectCrouchingAnimation( userEntity_t *self, char *pszAnimation )
{
	_ASSERTE( self );
	_ASSERTE( pszAnimation );

	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->type == TYPE_BOT || AI_IsSidekick( hook ) || hook->type == TYPE_CLIENT )
	{
		strcpy( pszAnimation, "camb" );
        AI_SelectAnimationPerWeaponType( self, pszAnimation );
	}
	else
	{
        strcpy( pszAnimation, "camb" );

        frameData_t *pSequence = NULL;
        int nRandom = rand() % 4;
        if ( nRandom == 3 )
        {
            pSequence = FRAMES_GetSequence( self, "cambd" );
            if ( !pSequence )
            {
                nRandom = 2;
            }
        }
        if ( nRandom == 2 )
        {
            pSequence = FRAMES_GetSequence( self, "cambc" );
            if ( !pSequence )
            {
                nRandom = 1;
            }
        }
        if ( nRandom == 1 )
        {
            pSequence = FRAMES_GetSequence( self, "cambb" );
        }

        if ( pSequence )
        {
            strcpy( pszAnimation, pSequence->animation_name );
        }
	}
}

// ----------------------------------------------------------------------------

void AI_SelectCrouchMovingAnimation( userEntity_t *self, char *pszAnimation )
{
	_ASSERTE( self );
	_ASSERTE( pszAnimation );

	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->type == TYPE_BOT || AI_IsSidekick( hook ) || hook->type == TYPE_CLIENT )
	{
		strcpy( pszAnimation, "cwalk" );
        AI_SelectAnimationPerWeaponType( self, pszAnimation );
	}
	else
	{
		strcpy( pszAnimation, "cwalka" );
	}
}


// ----------------------------------------------------------------------------

void AI_SelectWalkingAnimation( userEntity_t *self, char *pszAnimation )
{
	_ASSERTE( self );
	_ASSERTE( pszAnimation );

	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->type == TYPE_BOT || AI_IsSidekick( hook ) || hook->type == TYPE_CLIENT )
	{
		if(hook)
		{
			if(hook->bCarryingMikiko)
			{
				strcpy( pszAnimation, "walka" );
			}
			else
			{
				strcpy( pszAnimation, "walk" );
			}
		}
		if(self->waterlevel > 2)
		{
			strcpy( pszAnimation, "swim" );
		}
		else
		{
			AI_SelectAnimationPerWeaponType( self, pszAnimation );
		}
	}
	else
	{
		strcpy( pszAnimation, "walka" );
	}
}

// ----------------------------------------------------------------------------

void AI_SelectRunningAnimation( userEntity_t *self, char *pszAnimation )
{
	_ASSERTE( self );
	_ASSERTE( pszAnimation );

	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->type == TYPE_BOT || AI_IsSidekick( hook ) || hook->type == TYPE_CLIENT )
	{
		if(hook)
		{
			if(hook->bCarryingMikiko)
			{
				strcpy( pszAnimation, "runa" );
			}
			else
			{
				strcpy( pszAnimation, "run" );
			}
		}
		if(self->waterlevel > 2)
		{
			strcpy( pszAnimation, "swim" );
		}
        else
		{
			AI_SelectAnimationPerWeaponType( self, pszAnimation );
		}
	}
	else
	{
		strcpy( pszAnimation, "runa" );
	}
}

// ----------------------------------------------------------------------------

void AI_SelectJumpingAnimation( userEntity_t *self, char *pszAnimation )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->type == TYPE_BOT || AI_IsSidekick( hook ) || hook->type == TYPE_CLIENT )
	{
		if(hook)
		{
			if(hook->bCarryingMikiko)
			{
				strcpy( pszAnimation, "jumpa" );
			}
			else
			{
				strcpy( pszAnimation, "bjump" );
			}
			AI_SelectAnimationPerWeaponType( self, pszAnimation );
		}
	}
	else
	{
		strcpy( pszAnimation, "runa" );
	}
}


// ----------------------------------------------------------------------------

int AI_IsChaseVisible( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	int bEnemyVisible = FALSE;
	if ( AI_IsRangeAttack(hook) )
	{
		bEnemyVisible = AI_IsCompletelyVisible( self, self->enemy );
		if ( bEnemyVisible == COMPLETELY_VISIBLE )
		{
			bEnemyVisible = TRUE;
		}
        else
        {
            bEnemyVisible = FALSE;
        }
	}
	else
	{
		bEnemyVisible = AI_IsVisible(self, self->enemy);
	}

	return bEnemyVisible;
}

// ----------------------------------------------------------------------------

__inline float AI_ComputeAttackDistance( playerHook_t *hook )
{
	_ASSERTE( hook );
	float fAttackDistance = hook->attack_dist;
	if ( hook->dflags & DFL_JUMPATTACK && rnd() <= ( hook->jump_chance * 0.01f ) )
	{
		fAttackDistance = hook->jump_attack_dist;
	}

	return fAttackDistance;
}

// ----------------------------------------------------------------------------

__inline void AI_SetTurnRate( userEntity_t *self, CVector &destPoint )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	// to make sure that a flying unit never gets caught in a loop, modify the y turning rate
	if ( AI_WillItBeCaughtInLoop( self, destPoint ) )
	{
		float fTurnRateAdjuster = AI_FindTurnRateAdjuster( self, destPoint );

		self->ang_speed.y = self->ang_speed.y + fTurnRateAdjuster;
	}
}


// ----------------------------------------------------------------------------

__inline float AI_ComputeAirDistance( userEntity_t *pAttacker, userEntity_t *pAttackee )
{
	_ASSERTE( pAttacker && pAttackee );

	CVector attackerPoint( pAttacker->s.origin.x, pAttacker->s.origin.y, pAttacker->s.origin.z + pAttacker->s.mins.z );

	float fZOffset = fabs(pAttacker->s.mins.z);
	CVector attackeePoint( pAttackee->s.origin.x, pAttackee->s.origin.y, pAttackee->s.origin.z + pAttackee->s.maxs.z + fZOffset );

	float fDistance = VectorDistance( attackerPoint, attackeePoint );
	return fDistance;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsStraightPath
// Description:
// Input:
// Output:
// Note:
//
// ---------------------------------------------------------------------------
int AI_IsStraightPath( userEntity_t *self, userEntity_t *enemy )
{
	_ASSERTE( self && enemy );

	if ( AI_IsFlyingUnit( self ) || AI_IsInWater( self ) )
	{
		return TRUE;
	}

    if ( !AI_IsLineOfSight( self, enemy ) )
    {
        return FALSE;
    }

    if ( AI_IsEntityBelow( self, enemy ) )
    {
        return FALSE;
    }

	// the ground unit from here onwards
	if ( !self->groundEntity && self->waterlevel < 3 )
	{
		int bEnemyVisible = AI_IsVisible(self, enemy);
		float fZDistance = VectorZDistance( self->s.origin, enemy->s.origin );
		if ( bEnemyVisible && (enemy->s.origin.z < self->s.origin.z || fZDistance < 32.0f) && fZDistance < 512.0f )
		{
			// Ai units should be able to jump down in this case
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	playerHook_t *hook = AI_GetPlayerHook( self );
//	playerHook_t *ehook = AI_GetPlayerHook( enemy );// SCG[1/23/00]: not used

	float fDistance = VectorDistance( self->s.origin, enemy->s.origin );
	if ( AI_IsWithinAttackDistance( self, fDistance ) )
	{
		if (  hook->pPathList->pPath )
		{
			PATHLIST_KillPath(hook->pPathList);	
		}
		return TRUE;
	}

	if ( !AI_IsFlyingUnit( self ) && !AI_IsInWater( self ) && !AI_IsRangeAttack(hook) )
	{
		int bEnemyVisible = AI_IsVisible(self, enemy);
		float fZDistance = VectorZDistance(self->s.origin, enemy->s.origin);
		if ( bEnemyVisible && (enemy->s.origin.z < self->s.origin.z) && fZDistance < 512.0f )
		{
			// Ai units should be able to jump down in this case
			return TRUE;
		}
		else
		{ 
			/*if ( (hook->pPathList->pPath && hook->pPathList->nPathLength > 3) || fZDistance > MAX_JUMP_HEIGHT )
			{
				return FALSE;
			}
			else
			{*/
				if ( !hook->pPathList->pPath && fZDistance <= MAX_JUMP_HEIGHT && 
					! AI_IsGroundBelowBetween( self, self->s.origin, enemy->s.origin ) )
				{
					return FALSE;
				}
				else
				//if ( fZDistance > 256.0f )
				{
					return TRUE;
				}
			//}
#if 0
	/*
			else
			{
				// find shortest path from current node to goal node
				NODELIST_PTR pNodeList = hook->pNodeList;
				_ASSERTE( pNodeList );
				NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
				_ASSERTE( pNodeHeader );

				MAPNODE_PTR pNode1 = NODE_GetClosestNode( pNodeHeader, self->s.origin, 512.0f );
				_ASSERTE( pNode1 );
				MAPNODE_PTR pNode2 = NODE_GetClosestNode( pNodeHeader, enemy->s.origin, 512.0f );
				_ASSERTE( pNode2 );

				if ( pNode1 != pNode2 )
				{
					int				nPathLength = 0;
					PATHNODE_PTR	pPath = NULL;

					pPath = PATH_ComputePath(self, pNodeHeader, pNode1, pNode2, nPathLength);

					PATHNODE_PTR pPrevNode = pPath;
					PATHNODE_PTR pCurrentNode = pPrevNode->next_node;
					while ( pCurrentNode )
					{
						MAPNODE_PTR pPrevMapNode = NODE_GetNode( pNodeHeader, pPrevNode->nNodeIndex );
						MAPNODE_PTR pCurrentMapNode = NODE_GetNode( pNodeHeader, pCurrentNode->nNodeIndex );
						if ( fabs(pPrevMapNode->position.z - pCurrentMapNode->position.z) > 128.0f )
						{
							return FALSE;
						}

						pPrevNode = pCurrentNode;
						pCurrentNode = pCurrentNode->next_node;
					}

					path_kill( pPath );
				}
			}
		*/
#endif 0
		}
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsOnLadder
// Description:
// Input:
// Output:
// Note:
//
// ---------------------------------------------------------------------------
__inline int AI_IsOnLadder( userEntity_t *self, MAPNODE_PTR pNextNode )
{
	_ASSERTE( self );

//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	if ( pNextNode && (pNextNode->node_type & NODETYPE_LADDER) )
	{
		float fXYDistance = VectorXYDistance( self->s.origin, pNextNode->position );
//		float fZDistance = VectorZDistance( self->s.origin, pNextNode->position );// SCG[1/23/00]: not used
		if ( AI_IsCloseDistance2( self, fXYDistance ) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsUsingTrain
// Description:
// Input:
// Output:
// Note:
//
// ---------------------------------------------------------------------------
__inline static
int AI_IsUsingTrain( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->pPathList->pPath )
	{
		NODELIST_PTR pNodeList = hook->pNodeList;
		_ASSERTE( pNodeList );
		NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
		_ASSERTE( pNodeHeader );

		MAPNODE_PTR pCurrentNode = NODE_GetClosestNode(self);
		if ( !pCurrentNode )
		{
			if ( pNodeList->nCurrentNodeIndex != -1 )
			{
				pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
			}
		}
		
		if ( pCurrentNode )
		{
			PATHNODE_PTR pPathNode = hook->pPathList->pPath;
			if ( pCurrentNode->nIndex == pPathNode->nNodeIndex )
			{
				pPathNode = pPathNode->next_node;		
			}

			MAPNODE_PTR pNextNode = NULL;
			if ( pPathNode )
			{
				pNextNode = NODE_GetNode( pNodeHeader, pPathNode->nNodeIndex );
			}

			if ( ( (pCurrentNode->node_type & NODETYPE_TRAIN) && 
				   (pNextNode && (pNextNode->node_type & NODETYPE_TRAIN)) ) ||
				 ( AI_IsOnTrain( self ) && pNextNode && (pNextNode->node_type & NODETYPE_TRAIN)	)
			   )	
			{
				return TRUE;
			}
		}
	}
	else
	{

	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsNextPathTrain
// Description:
// Input:
// Output:
// Note:
//
// ---------------------------------------------------------------------------
__inline static
int AI_IsNextPathTrain( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->pPathList->pPath )
	{
		NODELIST_PTR pNodeList = hook->pNodeList;
		_ASSERTE( pNodeList );
		NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
		_ASSERTE( pNodeHeader );

		MAPNODE_PTR pCurrentNode = NODE_GetClosestNode(self);
		if ( !pCurrentNode )
		{
			if ( pNodeList->nCurrentNodeIndex != -1 )
			{
				pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
			}
		}
		if ( pCurrentNode )
		{
			PATHNODE_PTR pPathNode = hook->pPathList->pPath;
			if ( pCurrentNode->nIndex == pPathNode->nNodeIndex )
			{
				pPathNode = pPathNode->next_node;		
			}

			MAPNODE_PTR pNextNode = NULL;
			if ( pPathNode )
			{
				pNextNode = NODE_GetNode( pNodeHeader, pPathNode->nNodeIndex );
			}

			if ( pNextNode && (pNextNode->node_type & NODETYPE_TRAIN) )
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsOnTrain
// Description:
// Input:
// Output:
// Note:
//
// ---------------------------------------------------------------------------
int AI_IsOnTrain( userEntity_t *self )
{
	_ASSERTE( self );
	if ( self->groundEntity )
	{
		userEntity_t *pGroundEntity = self->groundEntity;
		_ASSERTE( pGroundEntity );

		if ( _stricmp( pGroundEntity->className, "func_train" ) == 0 )
		{
			return TRUE;
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsOnSameTrain
// Description:
// Input:
// Output:
// Note:
//
// ---------------------------------------------------------------------------
__inline static
int AI_IsOnSameTrain( userEntity_t *pUnit1, userEntity_t *pUnit2 )
{
	_ASSERTE( pUnit1 && pUnit2 );

	if ( pUnit1->groundEntity && pUnit2->groundEntity &&
		 pUnit1->groundEntity == pUnit2->groundEntity )
	{
		if ( _stricmp( pUnit1->groundEntity->className, "func_train" ) == 0 )
		{
			return TRUE;
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsOnMovingTrain
// Description:
// Input:
// Output:
// Note:
//
// ---------------------------------------------------------------------------
int AI_IsOnMovingTrain( userEntity_t *self )
{
	_ASSERTE( self );

	if ( self->groundEntity )
	{
		userEntity_t *pGroundEntity = self->groundEntity;
		if ( _stricmp( pGroundEntity->className, "func_train" ) == 0 && 
			 AI_IsTrainStopped( pGroundEntity ) == FALSE )
		{
			return TRUE;
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsTrainStopped
// Description:
// Input:
// Output:
// Note:
//
// ---------------------------------------------------------------------------
__inline static
int AI_IsTrainStopped( userEntity_t *pTrain )
{
	_ASSERTE( pTrain );
	if ( pTrain->velocity.Length() == 0.0f )
	{
		return TRUE;
	}
		
	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsTrainAtLocation
// Description:
// Input:
// Output:
// Note:
//
// ---------------------------------------------------------------------------
__inline static
int AI_IsTrainAtLocation( userEntity_t *pTrain, CVector &position )
{
	_ASSERTE( pTrain );
	if ( AI_IsTrainStopped( pTrain ) )
	{
		float fDistance = VectorDistance( pTrain->s.origin, position );
		if ( fDistance < 64.0f )
		{
			return TRUE;
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_GetEndNodeTrainNodes
// Description:
// Input:
// Output:
// Note:
//
// ---------------------------------------------------------------------------
__inline static
void AI_GetEndNodeTrainNodes( NODEHEADER_PTR pNodeHeader, PATHNODE_PTR pFirstTrainPathNode, 
							  MAPNODE_PTR &pLastTrainNode, MAPNODE_PTR &pAfterTrainNode )
{
	_ASSERTE( pNodeHeader );

	pLastTrainNode = NULL;
	pAfterTrainNode = NULL;
	PATHNODE_PTR path = pFirstTrainPathNode;
	while ( path )
	{
		MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, path->nNodeIndex );
		if ( !(pNode->node_type & NODETYPE_TRAIN) )
		{
			pAfterTrainNode = pNode;
			break;
		}
		pLastTrainNode = pNode;
	
		path = path->next_node;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_BackupAllAttributes
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void AI_BackupAllAttributes( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	hook->savedAttribute.turnSpeed = self->ang_speed;
	hook->savedAttribute.fRunSpeed = hook->run_speed;
	hook->savedAttribute.fWalkSpeed = hook->walk_speed;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_RestoreAllAttributes
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void AI_RestoreAllAttributes( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	self->ang_speed = hook->savedAttribute.turnSpeed;
	hook->run_speed = hook->savedAttribute.fRunSpeed;
	hook->walk_speed = hook->savedAttribute.fWalkSpeed;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ModifyTurnAttribute
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void AI_ModifyTurnAttribute( userEntity_t *self, CVector &turnSpeed )
{
	_ASSERTE( self );
	self->ang_speed.Set( turnSpeed.pitch, turnSpeed.yaw, turnSpeed.roll );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ModifyRunAttribute
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void AI_ModifyRunAttribute( userEntity_t *self, float fNewRunSpeed )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	hook->run_speed = fNewRunSpeed;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ModifWalkAttribute
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void AI_ModifyWalkAttribute( userEntity_t *self, float fNewWalkSpeed )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	hook->walk_speed = fNewWalkSpeed;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_GetCurrentGoalStack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
DllExport __inline GOALSTACK_PTR AI_GetCurrentGoalStack( playerHook_t *hook )
{
	_ASSERTE( hook );
	if ( !hook->pGoals )
	{
		hook->pGoals = GOALSTACK_Allocate();
	}
	
	GOALSTACK_PTR pGoalStack = hook->pGoals;
	if ( hook->bInScriptMode )
	{
		if ( !hook->pScriptGoals )
		{
			hook->pScriptGoals = GOALSTACK_Allocate();
		}

		pGoalStack = hook->pScriptGoals;
	}

	return pGoalStack;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ComputeSideStepPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static
int AI_ComputeSideStepPoint( userEntity_t *self, float fSideStepDistance, CVector &sideStepPoint )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	int bSuccess = TRUE;
	if ( AI_IsFlyingUnit( self ) || AI_IsInWater( self ) )
	{
		hook->strafe_dir = rand() % (STRAFE_DOWNRIGHT+1);
		
		CVector vectorTowardPoint;
		CVector angleTowardPoint;
		
		vectorTowardPoint.Subtract( self->s.origin, self->enemy->s.origin );
		vectorTowardPoint.Normalize();
		
		VectorToAngles( vectorTowardPoint, angleTowardPoint );
		
		angleTowardPoint.x -= 40.0f;
		switch ( hook->strafe_dir )
		{
			case STRAFE_LEFT:
			{
				angleTowardPoint.y += 45.0f;
				break;
			}
			case STRAFE_RIGHT:
			{
				angleTowardPoint.y -= 45.0f;
				break;
			}
			case STRAFE_UPLEFT:
			{
				angleTowardPoint.x -= 10.0f;
				angleTowardPoint.y += 45.0f;
				break;
			}
			case STRAFE_UPRIGHT:
			{
				angleTowardPoint.x -= 10.0f;
				angleTowardPoint.y -= 45.0f;
				break;
			}
			case STRAFE_DOWNLEFT:
			{
				angleTowardPoint.x += 25.0f;
				angleTowardPoint.y += 45.0f;
				break;
			}
			case STRAFE_DOWNRIGHT:
			{
				angleTowardPoint.x += 25.0f;
				angleTowardPoint.y -= 45.0f;
				break;
			}
			default:
			{
				break;
			}
		}
		
		angleTowardPoint.y = AngleMod( angleTowardPoint.y );
		
		CVector facingDir, right, up;
		angleTowardPoint.AngleToVectors( facingDir, right, up );
		facingDir.Normalize();

		float fHalfAttackDistance = hook->attack_dist * 0.5f;
		VectorMA( self->enemy->s.origin, facingDir, fHalfAttackDistance, sideStepPoint );
	}
	else
	{
		fSideStepDistance -= 44.0f;

		CVector vector;
		YawToVector( self->s.angles.yaw, vector );

		CVector orthogonalVector( -vector.y, vector.x, 0.0f );
		if ( rnd() > 0.5f )
		{
			orthogonalVector.Set( vector.y, -vector.x, 0.0f );
		}

		VectorMA( self->s.origin, orthogonalVector, fSideStepDistance, sideStepPoint );

		tr = gstate->TraceLine_q2( self->s.origin, sideStepPoint, self, MASK_SOLID );
		if ( tr.fraction < 1.0f )
		{
			// try the other side
			CVector oppositeVector( -orthogonalVector.x, -orthogonalVector.y, 0.0f );

			VectorMA( self->s.origin, oppositeVector, fSideStepDistance, sideStepPoint );

			tr = gstate->TraceLine_q2( self->s.origin, sideStepPoint, self, MASK_SOLID );
			if ( tr.fraction < 1.0f )
			{
				float fDistance = (fSideStepDistance * tr.fraction) - 16.0f;
				
				VectorMA( self->s.origin, oppositeVector, fDistance, sideStepPoint );
			}
			else
			{
				sideStepPoint.Set( self->s.origin );
				bSuccess = FALSE;
			}
		}

		if ( bSuccess == TRUE )
		{
			if ( AI_IsGroundBelowBetween( self, self->s.origin, sideStepPoint ) == FALSE )
			{
				sideStepPoint.Set( self->s.origin );
				bSuccess = FALSE;
			}
		}
	}

	return bSuccess;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ComputeChaseSideStepPoint
// Description:
// Input:
//				int nDirection		=> left or right step
//									   1 = left, 2 = right
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_ComputeChaseSideStepPoint( userEntity_t *self, userEntity_t *pEnemy, int nDirection, CVector &sideStepPoint )
{
	_ASSERTE( self && pEnemy );
	playerHook_t *hook = AI_GetPlayerHook( self );

	int bSuccess = TRUE;
	if ( AI_IsFlyingUnit( self ) || AI_IsInWater( self ) )
	{
		hook->strafe_dir = rand() % (STRAFE_DOWNRIGHT+1);
		if ( nDirection == LEFT )
		{
			if ( (hook->strafe_dir % 2) != 0 )
			{
				hook->strafe_dir = (hook->strafe_dir + 1) % (STRAFE_DOWNRIGHT+1);
			}	
		}
		else
		if ( nDirection == RIGHT )
		{
			if ( (hook->strafe_dir % 2) == 0 )
			{
				hook->strafe_dir = (hook->strafe_dir + 1) % (STRAFE_DOWNRIGHT+1);
			}	
		}

		CVector vectorTowardPoint;
		CVector angleTowardPoint;
		vectorTowardPoint.Subtract( self->s.origin, self->enemy->s.origin );
		vectorTowardPoint.Normalize();
		
		VectorToAngles( vectorTowardPoint, angleTowardPoint );
		
		angleTowardPoint.x -= -40.0f;
		switch ( hook->strafe_dir )
		{
			case STRAFE_LEFT:
			{
				angleTowardPoint.y += 45.0f;
				break;
			}
			case STRAFE_RIGHT:
			{
				angleTowardPoint.y -= 45.0f;
				break;
			}
			case STRAFE_UPLEFT:
			{
				angleTowardPoint.x -= 10.0f;
				angleTowardPoint.y += 45.0f;
				break;
			}
			case STRAFE_UPRIGHT:
			{
				angleTowardPoint.x -= 10.0f;
				angleTowardPoint.y -= 45.0f;
				break;
			}
			case STRAFE_DOWNLEFT:
			{
				angleTowardPoint.x += 25.0f;
				angleTowardPoint.y += 45.0f;
				break;
			}
			case STRAFE_DOWNRIGHT:
			{
				angleTowardPoint.x += 25.0f;
				angleTowardPoint.y -= 45.0f;
				break;
			}
			default:
			{
				break;
			}
		}
		
		angleTowardPoint.y = AngleMod( angleTowardPoint.y );
		
		CVector facingDir, right, up;
		angleTowardPoint.AngleToVectors( facingDir, right, up );
		facingDir.Normalize();

		float fHalfAttackDistance = hook->attack_dist * 0.5f;
		VectorMA( self->enemy->s.origin, facingDir, fHalfAttackDistance, sideStepPoint );
	}
	else
	{
		float fSpeed = AI_ComputeMovingSpeed( hook );
        float fSideStepDistance = fSpeed * 0.126f;
        if ( fSideStepDistance < 30.0f )
        {
            fSideStepDistance = 30.0f;
        }

		CVector vector( pEnemy->s.origin.x - self->s.origin.x, pEnemy->s.origin.y - self->s.origin.y, 0.0f );
		vector.Normalize();

		CVector orthogonalVector( vector.y, -vector.x, 0.0f );

		if ( nDirection == LEFT )
		{
			orthogonalVector.Set( -vector.y, vector.x, 0.0f );
		}

		VectorMA( self->s.origin, orthogonalVector, fSideStepDistance, sideStepPoint );

		tr = gstate->TraceLine_q2( self->s.origin, sideStepPoint, self, MASK_SOLID );
		if ( tr.fraction < 1.0f )
		{
			float fHalfWidth = (self->s.maxs.x - self->s.mins.x) * 0.5f;
            float fDistance = (fSideStepDistance * tr.fraction) - fHalfWidth;
			VectorMA( self->s.origin, orthogonalVector, fDistance, sideStepPoint );
		}

		float fXYDistance = VectorXYDistance( self->s.origin, sideStepPoint );
		if ( AI_IsExactDistance( hook, fXYDistance ) )
		{
			bSuccess = FALSE;
		}
		else
		{
			// check if there is a ground underneath
			CVector end( sideStepPoint.x, sideStepPoint.y, sideStepPoint.z - CHECK_GROUND_DISTANCE );
			
			tr = gstate->TraceLine_q2( sideStepPoint, end, self, MASK_SOLID );
			if ( tr.fraction >= 1.0f || tr.plane.normal.z < 0.9f )
			{
				// no ground, do not move
				sideStepPoint = self->s.origin;

				bSuccess = FALSE;
			}
		}
	}

	return bSuccess;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_UpdatePitchTowardEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_UpdatePitchTowardEnemy( userEntity_t *self )
{
	_ASSERTE( self );
	if ( self->enemy )
	{
		CVector temp = self->enemy->s.origin - self->s.origin;
		temp.Normalize();
		
		CVector angleVector;
		VectorToAngles(temp, angleVector);
		angleVector.pitch = AngleMod( angleVector.pitch );
		if ( angleVector.pitch > 60.0f && angleVector.pitch < 180.0f )
		{
			angleVector.pitch = 60.0f;
		}
		else
		if ( angleVector.pitch > 270.0f && angleVector.pitch < 300.0f )
		{
			angleVector.pitch = 300.0f;
		}

		self->ideal_ang.pitch = angleVector.pitch;
		com->ChangePitch(self);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_OrientToFloor
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_OrientToFloor( userEntity_t *self )
{
	_ASSERTE( self );

	CVector forwardVector;
	self->s.angles.AngleToForwardVector( forwardVector );

	// left side
    CVector leftVector;
	AI_LeftVector( forwardVector, leftVector );

	// right side
	CVector rightVector;
    AI_RightVector( forwardVector, rightVector );

	CVector upperLeftVector = forwardVector + leftVector;
	CVector upperRightVector = forwardVector + rightVector;

	CVector upperLeft, upperRight, lowerRight;
	upperLeft.x = self->s.origin.x + (upperLeftVector.x * 8.0f);
	upperLeft.y = self->s.origin.y + (upperLeftVector.y * 8.0f);
	upperLeft.z = self->s.origin.z + 32.0f;

	upperRight.x = self->s.origin.x + (upperRightVector.x * 8.0f);
	upperRight.y = self->s.origin.y + (upperRightVector.y * 8.0f);
	upperRight.z = self->s.origin.z + 32.0f;

	lowerRight.x = self->s.origin.x + (-upperLeftVector.x * 8.0f);
	lowerRight.y = self->s.origin.y + (-upperLeftVector.y * 8.0f);
	lowerRight.z = self->s.origin.z + 32.0f;

	// now compute the height from each points
	CVector tempPoint = upperRight;
	tempPoint.z -= 64.0f;
	tr = gstate->TraceLine_q2( upperRight, tempPoint, self, MASK_SOLID );
	
	CVector upperRightFloor = upperRight;
	upperRightFloor.z = upperRight.z - (64.0f * tr.fraction); 

	tempPoint = lowerRight;
	tempPoint.z -= 64.0f;
	tr = gstate->TraceLine_q2( lowerRight, tempPoint, self, MASK_SOLID );
	CVector lowerRightFloor = lowerRight;
	lowerRightFloor.z = lowerRight.z - (64.0f * tr.fraction);

	// pitch
	CVector tempVector = upperRightFloor - lowerRightFloor;
	tempVector.Normalize();

	CVector tempAngle;
	VectorToAngles( tempVector, tempAngle );

	tempAngle.pitch = AngleMod( tempAngle.pitch );
	if ( tempAngle.pitch > 45.0f && tempAngle.pitch < 180.0f )
	{
		tempAngle.pitch = 45.0f;
	}
	else
	if ( tempAngle.pitch > 270.0f && tempAngle.pitch < 315.0f )
	{
		tempAngle.pitch = 315.0f;
	}

	self->ideal_ang.pitch = tempAngle.pitch;

	com->ChangePitch(self);

	// now do the roll
	tempPoint = upperLeft;
	tempPoint.z -= 64.0f;
	tr = gstate->TraceLine_q2( upperLeft, tempPoint, self, MASK_SOLID );
	CVector upperLeftFloor = upperLeft;
	upperLeftFloor.z = upperLeft.z - (64.0f * tr.fraction);

	tempVector = upperRightFloor - upperLeftFloor;
	tempVector.Normalize();

	VectorToAngles( tempVector, tempAngle );

	tempAngle.roll = AngleMod( tempAngle.roll );
	if ( tempAngle.roll > 45.0f && tempAngle.roll < 180.0f )
	{
		tempAngle.roll = 45.0f;
	}
	else
	if ( tempAngle.roll > 270.0f && tempAngle.roll < 315.0f )
	{
		tempAngle.roll = 315.0f;
	}

	self->ideal_ang.roll = tempAngle.roll;

	com->ChangeRoll(self);
}


// ----------------------------------------------------------------------------
//
// Name:		AI_HandleEvasiveActionDuringChase
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_HandleEvasiveActionDuringChase( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *pEnemy = self->enemy;
	_ASSERTE( pEnemy );

	float fDistance = VectorDistance( self->s.origin, pEnemy->s.origin );
	if ( fDistance > (2 * AI_ComputeAttackDistance(hook)) )
	{
		if ( AI_IsEnemyTargetingMe( self, pEnemy ) && rnd() > 0.2f )
		{
			AI_AddNewTaskAtFront( self, TASKTYPE_CHASEEVADE );
			return TRUE;
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_RestartCurrentGoal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_RestartCurrentGoal( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

 	// restart the current goal
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	if ( pCurrentGoal )
	{
		GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
		if ( nGoalType != GOALTYPE_SCRIPTACTION && 
             nGoalType != GOALTYPE_WHENUSED_SCRIPTACTION &&
             nGoalType != GOALTYPE_SIDEKICK_WAITHERE &&
             nGoalType != GOALTYPE_SIDEKICK_FOLLOWOWNER &&
	         nGoalType != GOALTYPE_SIDEKICK_ATTACK &&
             nGoalType != GOALTYPE_SIDEKICK_STOPGOINGFURTHER &&
             nGoalType != GOALTYPE_SIDEKICK_TELEPORTANDCOMENEAR )
		{
			GOAL_ClearTasks( pCurrentGoal );
			AI_AddInitialTasksToGoal( self, pCurrentGoal, TRUE );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_EnemyAlert
// Description:
//				Alert all monsters within a visible distance that a enemy has 
//				been sighted
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_EnemyAlert( userEntity_t *self, userEntity_t *pEnemy )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( pEnemy);
	// SCG[10/29/99]: make sure we only do this for clients, monsters, and bots...
	if( !( pEnemy->flags & ( FL_CLIENT | FL_MONSTER | FL_BOT )) )
	{
		return;
	}

	
	AI_SetEnemy( self, pEnemy );

	//NSS[11/18/99]:If player has Wraith Orb don't alert other monsters
	if(hook)
	{
		if(hook->items & IT_WRAITHORB)
			return;
	}
	
	if ( AI_IsAlive( pEnemy ) )
    {
        userEntity_t *pMonster = alist_FirstEntity( monster_list );

	    while ( pMonster )
	    {
			// SCG[11/14/99]: We only want to do this for monsters...
			if( pMonster->flags & FL_MONSTER )
			{
				if ( self != pMonster && !pMonster->enemy &&
					 ( gstate->inPVS(self->s.origin, pMonster->s.origin) || 
					   gstate->inPVS(pMonster->s.origin, pEnemy->s.origin) )
				   )
				{
					playerHook_t *pMonsterHook = AI_GetPlayerHook( self );

					int bAlertedEnemy = FALSE;
					float fDistance = VectorDistance(pMonster->s.origin, self->s.origin);
					//if ( fDistance < 256.0f )
					//{
						// they are right next to each other
					//	bAlertedEnemy = TRUE;
					//	AI_SetEnemy( pMonster, pEnemy );
					//}
					//else
					if ( fDistance < pMonsterHook->speak )
					{
						if ( AI_IsLineOfSight( self, pMonster ) || AI_IsLineOfSight( pMonster, pEnemy ) )
						{
							bAlertedEnemy = TRUE;
							AI_SetEnemy( pMonster, pEnemy );
						}
					}

					if ( bAlertedEnemy == FALSE )
					{
						// now do it from the enemy's point
						fDistance = VectorDistance(pMonster->s.origin, pEnemy->s.origin);
						//if ( fDistance < 256.0f )
						//{
							// they are right next to each other
						//	bAlertedEnemy = TRUE;
						//	AI_SetEnemy( pMonster, pEnemy );
						//}
						//else
						if ( fDistance < pMonsterHook->speak )
						{
							if ( AI_IsLineOfSight( self, pMonster ) || AI_IsLineOfSight( pMonster, pEnemy ) )
							{
								bAlertedEnemy = TRUE;
								AI_SetEnemy( pMonster, pEnemy );
							}
						}
					}
				}
			}

		    pMonster = alist_NextEntity( monster_list );
	    }
    }
}

// ----------------------------------------------------------------------------
//
// Name:		AI_EnemyAlertNearbyMonsters
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_EnemyAlertNearbyMonsters( userEntity_t *self, userEntity_t *pEnemy )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	
	playerHook_t *phook = AI_GetPlayerHook( pEnemy);
	//NSS[11/18/99]:If player has Wraith Orb don't alert other monsters
	if(phook)
	{
		if(phook->items & IT_WRAITHORB)
			return;
	}
	
	userEntity_t *pMonster = alist_FirstEntity( monster_list );
	while ( pMonster )
	{
		// SCG[11/14/99]: We only want to do this for monsters...
		if( pMonster->flags & FL_MONSTER )
		{
			if ( AI_IsAlive( pMonster ) && self != pMonster )
			{
				playerHook_t *pMonsterHook = AI_GetPlayerHook( pMonster );

				_ASSERTE( pMonsterHook != NULL );
				if( pMonsterHook )
				{
					float fDistance = VectorDistance(pMonster->s.origin, self->s.origin);

					if ( fDistance < pMonsterHook->speak )
					{
						tr = gstate->TraceLine_q2( self->s.origin, pMonster->s.origin, self, MASK_SOLID );
						//Make sure we are clear and that we are not set to Ignore Player
						if ( tr.fraction >= 1.0f && !(pMonsterHook->ai_flags & AI_IGNORE_PLAYER) )
						{
							AI_SetEnemy( pMonster, pEnemy );
						}
					}
				}
			}
		}

		pMonster = alist_NextEntity( monster_list );
	}
}


// ----------------------------------------------------------------------------
//
// Name:		AI_IsInFOV
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsInFOV( userEntity_t *self, userEntity_t *targ )
{
    playerHook_t *hook = AI_GetPlayerHook( self );
	float fHalfFOV = hook->fov * 0.5f;
    
    CVector dir = targ->s.origin - self->s.origin;
	dir.Normalize();
	CVector angleTowardClient;
	VectorToAngles( dir, angleTowardClient );
	
	float fYawDiff = fabs( self->s.angles.yaw - angleTowardClient.yaw );
	if( fYawDiff > 180 )
	{
		fYawDiff -= 360;
	}
	if( fYawDiff < -180 )
	{
		fYawDiff += 360;
	}
	if ( fYawDiff <= fHalfFOV || fYawDiff >= (360.0f - fHalfFOV) )
	{
        return TRUE;
    }

	return	FALSE;
}

// ----------------------------------------------------------------------------
// NSS[1/3/00]:
// Name:		AI_IsVisible
// Description:
//              Test whether a target is within fov and a line of sight
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsVisible( userEntity_t *self, userEntity_t *targ )
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	if ( !AI_IsAlive( targ ) || !AI_IsAlive( self ) )
	{
		return FALSE;
	}

	CVector	org, end;

	org.Add( self->s.origin, self->view_ofs );

	end.Add( targ->absmin, targ->absmax );
	end.Multiply( 0.5 );
	end.Add( targ->view_ofs );
	playerHook_t *pTargetHook = AI_GetPlayerHook( targ );
	//NSS[11/6/99]:Check to see if it even has a hook)
	if(pTargetHook)
	{
		//	use Potentially Visible Set to quickly exclude some entities
		tr = gstate->TraceLine_q2( org, end, self, CONTENTS_SOLID|CONTENTS_NOSHOOT);//MASK_SOLID );
		if( targ->flags & (FL_BOT|FL_CLIENT) )
		{
			if ( (!(pTargetHook->items & IT_WRAITHORB) || hook->ai_flags&AI_SEENPLAYER)&& tr.fraction >= 1.0f)//gstate->inPVS(org, end) )// NSS[1/3/00]:
			{
				if ( tr.fraction >= 1.0f )
				{
					if ( AI_IsInFOV( self, targ ) || AI_IsSidekick(hook))
					{
						return TRUE;
					}
					else
					{
						float fXYDistance = VectorXYDistance( self->s.origin, targ->s.origin );
						// SCG[11/2/99]: We need to check something else, this is fucked.  
						// SCG[11/2/99]: pTargetHook->fSoundOutDistance only gets set 
						// SCG[11/2/99]: when the player is moving. So if the player is not
						// SCG[11/2/99]: moving, the owner will never get set.  
						// SCG[11/2/99]: Besides, it looks like this is the only place 
						// SCG[11/2/99]: pTargetHook->fSoundOutDistance is checked...
		//                if ( fXYDistance < pTargetHook->fSoundOutDistance )
						if ( fXYDistance < 256.0 )
						{
							return TRUE;
						}
					}
				}
			}
		}
		else if (tr.fraction >=1.0f)
		{
			return TRUE;
		}
	}
	return	FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsLineOfSight
// Description:
//              Just test whether there is a line of sight to the target
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsLineOfSight( userEntity_t *self, CVector &destPoint )
{
	if ( !self )
	{
		return FALSE;
	}

	CVector	org, end;

	// test top and bottom to see if there is a line of sight
    org = self->s.origin;
    org.z = self->absmax.z - 4.0f;
    end = destPoint;

	//	use Potentially Visible Set to quickly exclude some entities
	if ( gstate->inPVS(org, end) )
	{
		tr = gstate->TraceLine_q2( org, end, self, MASK_SOLID );
		if ( tr.fraction >= 1.0 && !tr.startsolid && !tr.allsolid )
		{
			return TRUE;
		}
	}
    else
    {
        org = self->s.origin;
        org.z = self->absmin.z + 4.0f;
        end = destPoint;
	    if ( gstate->inPVS(org, end) )
	    {
		    tr = gstate->TraceLine_q2( org, end, self, MASK_SOLID );
		    if ( tr.fraction >= 1.0 && !tr.startsolid && !tr.allsolid )
		    {
			    return TRUE;
		    }
	    }
    }

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsLineOfSight
// Description:
//              Just test whether there is a line of sight to the target
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsLineOfSight( userEntity_t *self, userEntity_t *targ )
{
	if ( !targ || !self )
	{
		return FALSE;
	}

	CVector	org, end;

	// test top and bottom to see if there is a line of sight
    org = self->s.origin;
    org.z = self->absmax.z - 4.0f;
    end = targ->s.origin;
    end.z = targ->absmax.z - 4.0f;

	//	use Potentially Visible Set to quickly exclude some entities
	if ( gstate->inPVS(org, end) )
	{
		tr = gstate->TraceLine_q2( org, end, self, MASK_SOLID );
		if ( tr.fraction >= 1.0 && !tr.startsolid && !tr.allsolid )
		{
			return TRUE;
		}
	}
    else
    {
        end = targ->s.origin;
        end.z = targ->absmin.z + 4.0f;
	    if ( gstate->inPVS(org, end) )
	    {
		    tr = gstate->TraceLine_q2( org, end, self, MASK_SOLID );
		    if ( tr.fraction >= 1.0 && !tr.startsolid && !tr.allsolid )
		    {
			    return TRUE;
		    }
	    }
    }

	// reverse test
    org = targ->s.origin;
    org.z = targ->absmax.z - 4.0f;
    end = self->s.origin;
    end.z = self->absmax.z - 4.0f;

	//	use Potentially Visible Set to quickly exclude some entities
	if ( gstate->inPVS(org, end) )
	{
		tr = gstate->TraceLine_q2( org, end, self, MASK_SOLID );
		if ( tr.fraction >= 1.0 && !tr.startsolid && !tr.allsolid )
		{
			return TRUE;
		}
	}
    else
    {
        end = self->s.origin;
        end.z = self->absmin.z + 4.0f;
	    if ( gstate->inPVS(org, end) )
	    {
		    tr = gstate->TraceLine_q2( org, end, self, MASK_SOLID );
		    if ( tr.fraction >= 1.0 && !tr.startsolid && !tr.allsolid )
		    {
			    return TRUE;
		    }
	    }
    }
    
	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsCompletelyVisible
// Description:
//				This function tests visibility from both sides to make sure that
//				the target is completely visible
//				This function is usually called while chasing, to make sure there
//				is a clear attack sight from a ranged weapon
// Input:
// Output:
//				0	=> not visible
//				1	=> completely visible
//				2	=> left side blocked
//				3	=> right side blocked
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsCompletelyVisible( userEntity_t *self, CVector &destPoint, float fWidthFactor /* = 0.6f */)
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	float fSelfHalfHeight = (self->s.maxs.z - self->s.mins.z) * 0.4;

	CVector org( self->s.origin.x, self->s.origin.y, self->s.origin.z + fSelfHalfHeight );
	CVector end = destPoint;

	float fHalfWidth = (self->s.maxs.x - self->s.mins.x) * fWidthFactor;
	
	// do two traces from both sides to make sure that the hide node is a good hiding place
	CVector directionVector;
	directionVector.x = destPoint.x - self->s.origin.x;
	directionVector.y = destPoint.y - self->s.origin.y;
	directionVector.z = destPoint.z - self->s.origin.z;
	directionVector.Normalize();
	
	// left side
    CVector leftVector;
    AI_LeftVector( directionVector, leftVector );

	// right side
    CVector rightVector;
    AI_RightVector( directionVector, rightVector );

	CVector leftPos;
	VectorMA( org, leftVector, fHalfWidth, leftPos );

	CVector rightPos;
	VectorMA( org, rightVector, fHalfWidth, rightPos );

	int nVisibility = NOT_VISIBLE;
    int bLeftSideVisible = FALSE;
    int bRightSideVisible = FALSE;

    //	use Potentially Visible Set to quickly exclude some entities
	if ( gstate->inPVS(org, end) || gstate->inPVS(leftPos, end) || gstate->inPVS(rightPos, end) )
	{
		trace_t leftTrace = gstate->TraceLine_q2( leftPos, end, self, MASK_SOLID );
		if ( leftTrace.fraction >= 1.0f && !leftTrace.startsolid && !leftTrace.allsolid )
		{
			bLeftSideVisible = TRUE;
        }

        trace_t rightTrace = gstate->TraceLine_q2( rightPos, end, self, MASK_SOLID );
		if ( rightTrace.fraction >= 1.0f && !rightTrace.startsolid && !rightTrace.allsolid )
		{
		    bRightSideVisible = TRUE;
        }
        
        if ( bLeftSideVisible == TRUE && bRightSideVisible == TRUE )
        {
            nVisibility = COMPLETELY_VISIBLE;
        }
        else
        {
            if ( bLeftSideVisible == TRUE )
            {
                nVisibility = LEFTSIDE_VISIBLE;
            }
            else
            if ( bRightSideVisible == TRUE )
            {
                nVisibility = RIGHTSIDE_VISIBLE;
            }
        }
	}

	return nVisibility;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsCompletelyVisible
// Description:
//				This function tests visibility from both sides to make sure that
//				the target is completely visible
//				This function is usually called while chasing, to make sure there
//				is a clear attack sight from a ranged weapon
// Input:
// Output:
//				0	=> not visible
//				1	=> completely visible
//				2	=> left side blocked
//				3	=> right side blocked
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsCompletelyVisible( userEntity_t *self, userEntity_t *targ, float fWidthFactor /* = 0.6f */)
{
	if ( !targ || !self )
	{
		return NOT_VISIBLE;
	}

	playerHook_t *pTargetHook = AI_GetPlayerHook( targ );
	
	//NSS[11/7/99]:
	//First check to see if what we are targeting has a hook
	//Then if so check to see if they have the Wraith Orb value set.
	if(pTargetHook)
	{
		if ( pTargetHook->items & IT_WRAITHORB )
		{
			return NOT_VISIBLE;
		}
    }
	else
	{
		//No hook means no target (and bad shit has happened)
		return NOT_VISIBLE;
	}

	float fTargetHalfHeight = (targ->s.maxs.z - targ->s.mins.z) * 0.4f;
	CVector end( targ->s.origin.x, targ->s.origin.y, targ->s.origin.z + fTargetHalfHeight );

	return AI_IsCompletelyVisible( self, end, fWidthFactor );

}

// ----------------------------------------------------------------------------
//
// Name:		AI_DetectPits
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define MAX_PIT_LENGTH				128.0f
#define PIT_DETECTION_INCREMENT		32.0f

int	AI_DetectPits( userEntity_t *self, CVector &endPoint, float fStartDistance, float fEndDistance )
{
	_ASSERTE( self );

	int bFurtherTest = FALSE;

	CVector org( self->s.origin.x, self->s.origin.y, self->s.origin.z + self->view_ofs.z );

	CVector destPoint( endPoint );

	float fIncrement = 0.0f;
	float fDistance = VectorDistance( self->s.origin, endPoint );
	while ( fIncrement < fDistance )
	{
		tr = gstate->TraceLine_q2( org, destPoint, NULL, MASK_SOLID );
		if ( tr.fraction >= 1.0f && !tr.ent && 
			 tr.plane.normal.z >= 0.1f && tr.plane.normal.z <= 0.7f )
		{
			bFurtherTest = TRUE;
			break;
		}

		fIncrement += PIT_DETECTION_INCREMENT;
		destPoint.AddZ( fIncrement );
	}

	if ( bFurtherTest )
	{
		playerHook_t *hook = AI_GetPlayerHook( self );

		CVector start, end;

		float fLowestPoint = self->s.origin.z - 24.0f;
		float fHighestPoint = fLowestPoint;

		VectorMA( self->s.origin, forward, fStartDistance, start );
		end = start;
		end.z -= MAX_PIT_LENGTH;

		tr = gstate->TraceLine_q2( start, end, self, MASK_SOLID );
		if ( (tr.fraction * MAX_PIT_LENGTH) > 24.0f && (tr.fraction * MAX_PIT_LENGTH) < 60.0f )
		{
			// going down, check for a low pit 

			CVector forward_add = forward;
			forward_add.Multiply( PIT_DETECTION_INCREMENT );

			float fRemainingDistance = fEndDistance - fStartDistance;
			while ( fRemainingDistance > 0.0f )
			{
				start += forward_add;
				end = start;
				end.z -= MAX_PIT_LENGTH;

				tr = gstate->TraceLine_q2( start, end, self, MASK_SOLID );
				if ( tr.fraction < 1.0f )
				{
					float fIntersectPoint = start.z - (tr.fraction * MAX_PIT_LENGTH);
					if ( fIntersectPoint < fLowestPoint )
					{
						fLowestPoint = fIntersectPoint;
					}

					if ( fIntersectPoint > fHighestPoint )
					{
						fHighestPoint = fIntersectPoint;
					}
				}

				fRemainingDistance -= PIT_DETECTION_INCREMENT;
			}

			float fMaxJumpHeight = ai_max_height(self, hook->upward_vel);
			if ( (fHighestPoint - fLowestPoint) > fMaxJumpHeight )
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsDirectPath
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int	AI_IsDirectPath( userEntity_t *self, CVector &destPoint )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	if ( destPoint.z > self->s.origin.z )
	{
		// destination is higher than the source
		// can jump over to get to the dest point

		float fDistance = VectorDistance( self->s.origin, destPoint );
		if ( AI_DetectPits( self, destPoint, 32.0f, fDistance ) == TRUE )
		{
			return FALSE;
		}
	}
	else
	{
		float fDistance = VectorDistance( self->s.origin, destPoint );
		if ( AI_DetectPits( self, destPoint, 32.0f, fDistance ) == TRUE )
		{
			return FALSE;
		}
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_FaceTowardPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_FaceTowardPoint( userEntity_t *self, CVector &destPoint )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( /*(hook->nTargetCounter % 3) == 0 && */ hook->dflags & DFL_ORIENTTOFLOOR )
	{
		AI_OrientToFloor( self );
	}

	CVector dir = destPoint - self->s.origin;
	dir.Normalize();

	self->ideal_ang.yaw = AngleMod( VectorToYaw(dir) );
	com->ChangeYaw(self);
}

// ----------------------------------------------------------------------------
//
// Name:		AI_RemoveFirstPathIfPossible
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static 
void AI_RemoveFirstPathIfPossible( userEntity_t *self, NODEHEADER_PTR pNodeHeader, CVector &destPoint )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	PATHNODE_PTR pFirstPathNode = hook->pPathList->pPath;
	PATHNODE_PTR pSecondPathNode = hook->pPathList->pPath->next_node;

	MAPNODE_PTR pFirstNode = NODE_GetNode( pNodeHeader, pFirstPathNode->nNodeIndex );
	_ASSERTE( pFirstNode );

	if ( !pSecondPathNode )
	{
	    float fXYDistance = VectorXYDistance( self->s.origin, pFirstNode->position );
	    float fZDistance = VectorZDistance( self->s.origin, pFirstNode->position );
	    if ( AI_IsCloseDistance2( self, fXYDistance ) && fZDistance < 32.0f )
	    {
		    PATHLIST_DeleteFirstInPath( hook->pPathList );
		    return;
	    }

	    MAPNODE_PTR pFirstNode = NODE_GetNode( pNodeHeader, pFirstPathNode->nNodeIndex );
	    _ASSERTE( pFirstNode );

		CVector vector1 = self->s.origin - pFirstNode->position;
		CVector vector2 = destPoint - pFirstNode->position;

		float fAngle = ComputeAngle2D( vector1, vector2 );
		if ( fAngle < 90.0f || fAngle > 270.0f )
		{
			PATHLIST_DeleteFirstInPath( hook->pPathList );
		}
		return;
	}

	MAPNODE_PTR pSecondNode = NODE_GetNode( pNodeHeader, pSecondPathNode->nNodeIndex );
	_ASSERTE( pSecondNode );

	int bCheckForDeletion = FALSE;
	float fXYDistance = VectorXYDistance( self->s.origin, pFirstNode->position );
	float fZDistance = VectorZDistance( self->s.origin, pFirstNode->position );
	if ( AI_IsCloseDistance2( self, fXYDistance ) && fZDistance < 32.0f )
	{
		PATHLIST_DeleteFirstInPath( hook->pPathList );
		return;
	}
    else
    {
        bCheckForDeletion = TRUE;
    }

	if ( bCheckForDeletion )
	{
		if ( (pFirstNode->node_type & NODETYPE_JUMPFORWARD) ||
			 (pFirstNode->node_type & NODETYPE_SHOTCYCLERJUMP) ||
			 (pFirstNode->node_type & NODETYPE_JUMPUP) )
		{
			if ( !AI_IsGroundBelowBetween( self, pFirstNode->position, pSecondNode->position ) )
			{
				bCheckForDeletion = FALSE;
			}
		}
	}
				
	if ( bCheckForDeletion )
	{
		CVector vector1 = self->s.origin - pFirstNode->position;
		CVector vector2 = pSecondNode->position - pFirstNode->position;

		float fAngle = ComputeAngle2D( vector1, vector2 );
		if ( fAngle < 90.0f || fAngle > 270.0f )
		{
			PATHLIST_DeleteFirstInPath( hook->pPathList );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_FindPathToPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_FindPathToPoint( userEntity_t *self, CVector &destPoint )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !AI_CanPath( hook ) )
	{
		return FALSE;
	}

#ifdef _DEBUG
	int time1 = gstate->Sys_Milliseconds();
#endif _DEBUG

	    // get rid of old path
	    PATHLIST_KillPath( hook->pPathList );

	    MAPNODE_PTR pCurrentNode = NULL;
	    MAPNODE_PTR pGoalNode = NULL;
	    int nNodeType;
	    NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeaderAndType( self, nNodeType );
	    if ( !pNodeHeader )
	    {
		    return FALSE;	
	    }

	    NODELIST_PTR pNodeList = hook->pNodeList;
	    _ASSERTE( pNodeList );
	    if ( pNodeList->nCurrentNodeIndex != -1 )
	    {
		    pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
	    }
		float fDistance = 0.0f;
		if(pCurrentNode)
		{
			fDistance = VectorDistance(pCurrentNode->position,self->s.origin);
		}

	    if ( !pCurrentNode || fDistance > 256.0f )
	    {
		    pCurrentNode = NODE_GetClosestNode(self);
		    if ( !pCurrentNode )
		    {
			    return FALSE;
		    }
	    }

	    // make sure the goal node is the same type of node as the start node
	    pGoalNode = NODE_FindClosestNode( pNodeHeader, nNodeType, destPoint );
	    if ( !pGoalNode )
	    {
		    return FALSE;
	    }

		//		int nPathLength = PATH_ComputePath(self, pNodeHeader, pCurrentNode, pGoalNode, hook->pPathList);
		PATH_ComputePath(self, pNodeHeader, pCurrentNode, pGoalNode, hook->pPathList);// SCG[1/23/00]: nPathLength not used
	    if ( !hook->pPathList->pPath )
	    {
    #ifdef _DEBUG
		    int timeDiff = gstate->Sys_Milliseconds() - time1;
		    AI_Dprintf( "NO PATH: %s at %s - %d ms\n", self->className, com->vtos(self->s.origin), timeDiff );
    #endif _DEBUG
		    return FALSE;
	    }
	hook->nMoveCounter = 0;

#ifdef _DEBUG
	int timeDiff = gstate->Sys_Milliseconds() - time1;
//	gstate->Con_Dprintf( "Path took = %d miliseconds. Path length = %d.\n", timeDiff, nPathLength );
#endif _DEBUG

	// see if the first node is needed
	AI_RemoveFirstPathIfPossible( self, pNodeHeader, destPoint );
	
	return (int)hook->pPathList->pPath;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_FindPathToEntity
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_FindPathToEntity( userEntity_t *self, userEntity_t *destEntity, int bTestCanPath /* = TRUE */ )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !destEntity || destEntity->movetype == MOVETYPE_NOCLIP )
	{
		return 0;
	}
	
	if ( bTestCanPath && !AI_CanPath( hook ) )
	{
		return FALSE;
	}

#ifdef _DEBUG
	int time1 = gstate->Sys_Milliseconds();
#endif _DEBUG

	// get rid of old path
	if( hook->pPathList != NULL )
	{
		PATHLIST_KillPath( hook->pPathList );
	}
	else
	{
		return FALSE;
	}

	MAPNODE_PTR pCurrentNode = NULL;
	MAPNODE_PTR pGoalNode = NULL;

	int nNodeType;
	NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeaderAndType( self, nNodeType );
	if ( !pNodeHeader )
	{
		return FALSE;
	}
	NODELIST_PTR pNodeList = hook->pNodeList;
	if ( !pNodeList )
	{
		return FALSE;
	}
	if ( pNodeList->nCurrentNodeIndex != -1 && AI_IsOnGround(self) )
	{
		pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
	}
	if ( !pCurrentNode )
	{
		pCurrentNode = NODE_GetClosestNode(self);
		if ( !pCurrentNode )
		{
			return FALSE;
		}
	}

	// make sure the goal node is the same type of node as the start node
	playerHook_t *pGoalHook = AI_GetPlayerHook( destEntity );

	int nGoalNodeType;
	NODEHEADER_PTR pGoalNodeHeader = NODE_GetNodeHeaderAndType( destEntity, nGoalNodeType );
	if ( !pGoalNodeHeader )
	{
		return FALSE;
	}
	if(!pGoalHook)
		return FALSE;

	NODELIST_PTR pGoalNodeList = NULL;
	if((destEntity->s.flags & (FL_CLIENT|FL_BOT|FL_MONSTER)) && AI_IsAlive(destEntity))
	{
		pGoalNodeList = pGoalHook->pNodeList;
	}

	if( pGoalNodeList && AI_IsSidekick(hook) && ((nNodeType == 2 && nGoalNodeType == 1) || (nNodeType == 1 && nGoalNodeType == 2)))
	{
		
		pGoalNode = NODE_GetNode( pGoalNodeHeader, pGoalNodeList->nCurrentNodeIndex );
	}
	else if ( pGoalNodeList && (nNodeType & nGoalNodeType) && pGoalNodeList->nCurrentNodeIndex != -1 &&
         (AI_IsOnGround(self)||self->waterlevel > 2) )
	{
		pGoalNode = NODE_GetNode( pGoalNodeHeader, pGoalNodeList->nCurrentNodeIndex );
	}
	if ( !pGoalNode || !pGoalNodeList)
	{
		pGoalNode = NODE_FindClosestNode( pNodeHeader, nNodeType, destEntity->s.origin );
		if ( !pGoalNode )
		{
			return FALSE;
		}
	}

//	int nPathLength = PATH_ComputePath(self, pNodeHeader, pCurrentNode, pGoalNode, hook->pPathList);
	PATH_ComputePath(self, pNodeHeader, pCurrentNode, pGoalNode, hook->pPathList);// SCG[1/23/00]: nPathLength not used
	if ( !hook->pPathList->pPath )
	{
#ifdef _DEBUG
		int timeDiff = gstate->Sys_Milliseconds() - time1;
		AI_Dprintf( "NO PATH: %s at %s - %d ms\n", self->className, com->vtos(self->s.origin), timeDiff );
#endif _DEBUG
		return FALSE;
	}
	hook->nMoveCounter = 0;

#ifdef _DEBUG
	int timeDiff = gstate->Sys_Milliseconds() - time1;
	//gstate->Con_Dprintf( "Path took = %d miliseconds. Path length = %d.\n", timeDiff, nPathLength );
#endif _DEBUG

	// see if the first node is needed
	AI_RemoveFirstPathIfPossible( self, pNodeHeader, destEntity->s.origin );

	return (int)hook->pPathList->pPath;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_FindPathToNode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_FindPathToNode( userEntity_t *self, MAPNODE_PTR pDestNode )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !pDestNode )
	{
		return 0;
	}

	if ( !AI_CanPath( hook ) )
	{
		return FALSE;
	}

#ifdef _DEBUG
	int time1 = gstate->Sys_Milliseconds();
#endif _DEBUG

	// get rid of old path
	PATHLIST_KillPath( hook->pPathList );

	MAPNODE_PTR pCurrentNode = NULL;
	MAPNODE_PTR pGoalNode = NULL;

	int nNodeType;
	NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeaderAndType( self, nNodeType );
	if ( !pNodeHeader )
	{
		return FALSE;
	}
	NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	if ( pNodeList->nCurrentNodeIndex != -1 )
	{
		pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
	}
	if ( !pCurrentNode )
	{
		pCurrentNode = NODE_GetClosestNode(self);
		if ( !pCurrentNode )
		{
			return FALSE;
		}
	}

	if ( pDestNode->node_type & nNodeType )
	{
		pGoalNode = pDestNode;
	}
	if ( !pGoalNode )
	{
		pGoalNode = NODE_FindClosestNode( pNodeHeader, nNodeType, pDestNode->position );
		if ( !pGoalNode )
		{
			return FALSE;
		}
	}

//	int nPathLength = PATH_ComputePath(self, pNodeHeader, pCurrentNode, pGoalNode, hook->pPathList);
	PATH_ComputePath(self, pNodeHeader, pCurrentNode, pGoalNode, hook->pPathList);// SCG[1/23/00]: nPathLength not used
	if ( !hook->pPathList->pPath )
	{
#ifdef _DEBUG
		int timeDiff = gstate->Sys_Milliseconds() - time1;
		AI_Dprintf( "NO PATH: %s at %s - %d ms\n", self->className, com->vtos(self->s.origin), timeDiff );
#endif _DEBUG
		return FALSE;
	}
	hook->nMoveCounter = 0;

#ifdef _DEBUG
	int timeDiff = gstate->Sys_Milliseconds() - time1;
	//gstate->Con_Dprintf( "Path took = %d miliseconds. Path length = %d.\n", timeDiff, nPathLength );
#endif _DEBUG

	// see if the first node is needed
	AI_RemoveFirstPathIfPossible( self, pNodeHeader, pDestNode->position );

	return (int)hook->pPathList->pPath;
}




#ifdef _DEBUG
void AI_Drop_Markers (CVector &Target, float delay);
#endif



// ----------------------------------------------------------------------------
// NSS[2/9/00]:
// Name:		AI_NextPathAroundEntity
// Description:Looks One more step into the future and determines if we can go ahead
// and move forward.
// Input:userEntity_t *self,CVector &Origin, userEntity_t *obstacleEnt,CVector &destPoint
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void AI_NextPathAroundEntity(userEntity_t *self,CVector &Origin,CVector &destPoint)
{
	
	playerHook_t *hook = AI_GetPlayerHook( self );
	float fThreeFrameSpeed = AI_ComputeMovingSpeed( hook ) * 0.2f;
	CVector Dir,Destination;

	Dir = destPoint - Origin;
	Dir.Normalize();
	Destination = Origin + (Dir*fThreeFrameSpeed);

	tr = gstate->TraceBox_q2( self->s.origin, self->s.mins,self->s.maxs,Destination, self,MASK_MONSTERSOLID );
	if(tr.fraction >= 1.0f)
	{
		AI_AddNewTask(self,TASKTYPE_MOVETOLOCATION,Destination);
	}
	else
	{
		MAPNODE_PTR Node = NODE_GetClosestNode(self,Destination);
		if(Node)
		{
			tr = gstate->TraceBox_q2( self->s.origin, self->s.mins,self->s.maxs,Node->position, self,MASK_MONSTERSOLID );
			float fDistance1 = VectorXYDistance(Node->position,destPoint);
			float fDistance2 = VectorXYDistance(Origin,destPoint);
			if(tr.fraction >= 1.0f && (fDistance1 < fDistance2) )
			{
				AI_AddNewTask(self,TASKTYPE_MOVETOLOCATION,Node->position);
			}
		}
	}
}

int AI_HasGoalInQueue( userEntity_t *self, GOALTYPE SearchGoal );
// ----------------------------------------------------------------------------
// NSS[2/9/00]:
// Name:		AI_FindPathAroundEntity
// Description:This will find out if we need to path around an entity and will
// set our new task to be TASKTYPE_GOINGAROUNDENTITY
// Input:userEntity_t *self, userEntity_t *obstacleEnt, CVector &destPoint
// Output:TRUE/FALSE  (True = We found a path and setup the new task)
// Note:
// ----------------------------------------------------------------------------
int AI_FindPathAroundEntity( userEntity_t *self, userEntity_t *obstacleEnt, CVector &destPoint )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if(!hook)
		return FALSE;
	// only have one going around obstacle or an entity
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASKTYPE nTaskType = GOALSTACK_GetCurrentTaskType( pGoalStack );
	if ( nTaskType == TASKTYPE_GOINGAROUNDENTITY || nTaskType == TASKTYPE_GOINGAROUNDOBSTACLE )
    {
        AI_RemoveCurrentTaskWithoutStartingNextTask( self );
        // there might be two in a row
	    if ( nTaskType == TASKTYPE_GOINGAROUNDENTITY || nTaskType == TASKTYPE_GOINGAROUNDOBSTACLE )
        {
            AI_RemoveCurrentTaskWithoutStartingNextTask( self );
        }
    }
	
	CVector leftVector,rightVector,vectorFromEntity, DestinationRight,DestinationLeft,Dir,Mins,Maxs;
	float fDistance, LeftFraction,RightFraction,OriginDistance,Mul;
	Mins = self->s.mins*0.5;
	Maxs = self->s.maxs*0.5;
	
	
	Mul = 4.0f;
	while(Mul > 1.0)
	{
		OriginDistance = VectorXYDistance(self->s.origin,destPoint) + (self->s.maxs.x*Mul);
		// NSS[2/9/00]:Get a decent range to scan from their bounding boxes
		fDistance = (self->s.maxs.x) + (obstacleEnt->s.maxs.x*Mul);

 		
		// NSS[2/9/00]:	Get the vector to the entity
		vectorFromEntity		=  obstacleEnt->s.origin - self->s.origin;

		vectorFromEntity.Normalize();

		/*if(self->movetype != MOVETYPE_FLY)
		{
			vectorFromEntity.z		= 0.0f;
		}*/

		
		// NSS[2/9/00]:Find both the left and right vector
		AI_LeftVector( vectorFromEntity, leftVector );
		AI_RightVector( vectorFromEntity, rightVector );

		DestinationLeft = self->s.origin + (leftVector * fDistance);
		//tr = gstate->TraceBox_q2( self->s.origin, Mins,Maxs,DestinationLeft, self,MASK_MONSTERSOLID );
		tr = gstate->TraceLine_q2( self->s.origin, DestinationRight, self,MASK_MONSTERSOLID );
		LeftFraction = tr.fraction ;
	//	if(AI_IsSidekick(hook))
		//	AI_Drop_Markers (DestinationLeft,0.1);
		
		DestinationRight = self->s.origin + (rightVector * fDistance);
		tr = gstate->TraceBox_q2( self->s.origin,Mins,Maxs,DestinationRight, self,MASK_MONSTERSOLID );
		//tr = gstate->TraceLine_q2( self->s.origin, DestinationRight, self,MASK_MONSTERSOLID );
		RightFraction = tr.fraction;
		
	//	if(AI_IsSidekick(hook))
		//	AI_Drop_Markers (DestinationRight,0.1);
		
		if(RightFraction >= 1.0 || LeftFraction >= 1.0)
		{
			float DistancefromLeft,DistancefromRight;
			
			DistancefromLeft = DistancefromRight = 10000.0f;
			if(RightFraction >= 1.0)
			{
				DistancefromRight = VectorXYDistance(DestinationRight,destPoint);
				tr = gstate->TraceLine_q2( destPoint, DestinationRight, NULL,MASK_MONSTERSOLID );
				RightFraction = tr.fraction;
			}
			if(LeftFraction >= 1.0)
			{
				DistancefromLeft = VectorXYDistance(DestinationLeft,destPoint);
				tr = gstate->TraceLine_q2( destPoint, DestinationLeft,NULL,MASK_MONSTERSOLID );
				LeftFraction = tr.fraction;
			}

			if(DistancefromLeft < DistancefromRight && LeftFraction >=1.0f  && DistancefromLeft < OriginDistance)
			{
				// NSS[2/12/00]:Make sure this is a valid place to go.
				if ( AI_IsGroundBelowBetween( self, self->s.origin, DestinationRight ) )
				{
					tr = gstate->TraceBox_q2( DestinationLeft, self->s.mins,self->s.maxs,DestinationLeft, self,MASK_MONSTERSOLID );
					DistancefromLeft = VectorDistance(self->s.origin,DestinationLeft);
					if(!AI_HasGoalInQueue( self, GOALTYPE_MOVETOLOCATION ) && Mul > 1.75 && !tr.startsolid && tr.fraction >=1.0f && !AI_IsCloseDistance2(self,DistancefromLeft))
					{
						AI_AddNewGoal(self,GOALTYPE_MOVETOLOCATION,DestinationLeft);
						AI_NextPathAroundEntity(self,DestinationLeft,destPoint);
					}
					else
					{
						AI_SetVelocity(self,leftVector,128.0f);
					}
					return TRUE;
				}
			}
			else if(RightFraction >= 1.0f && DistancefromRight < OriginDistance)
			{
				// NSS[2/12/00]:Make sure this is a valid place to go.
				if ( AI_IsGroundBelowBetween( self, self->s.origin, DestinationRight ) )
				{				
					tr = gstate->TraceBox_q2( DestinationRight, self->s.mins,self->s.maxs,DestinationRight, self,MASK_MONSTERSOLID );
					DistancefromRight = VectorDistance(self->s.origin,DestinationRight);
					if(!AI_HasGoalInQueue( self, GOALTYPE_MOVETOLOCATION ) && Mul > 1.75 && !tr.startsolid && tr.fraction >=1.0f &&  !AI_IsCloseDistance2(self,DistancefromRight))
					{
						AI_AddNewGoal(self,GOALTYPE_MOVETOLOCATION,DestinationRight);
						AI_NextPathAroundEntity(self,DestinationRight,destPoint);
					}
					else
					{
						AI_SetVelocity(self,rightVector,128.0f);
					}
					return TRUE;
				}
			}
		}
		Mul -= 0.25;
	}
	if(AI_IsSidekick(hook))
	{
		if(SIDEKICK_HasTaskInQue(self,TASKTYPE_SIDEKICK_EVADE))
		{
			AI_SetStateRunning(hook);
			AI_AddNewTaskAtFront( self, TASKTYPE_MOVEAWAY, obstacleEnt );
		}
		else if(!(obstacleEnt->flags & FL_BOT))
		{
			Dir = destPoint;
			Dir.Normalize();
			AI_FindNewCourse(self,&Dir);
			//AI_StopEntity(self);
		}
	}
	return FALSE;
}


/*
// ----------------------------------------------------------------------------
//
// Name:		AI_FindPathAroundEntity
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_FindPathAroundEntity( userEntity_t *self, userEntity_t *obstacleEnt, CVector &destPoint )
{
	_ASSERTE( self );
    _ASSERTE( obstacleEnt );

	playerHook_t *hook = AI_GetPlayerHook( self );

	// only have one going around obstacle or an entity
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASKTYPE nTaskType = GOALSTACK_GetCurrentTaskType( pGoalStack );
	if ( nTaskType == TASKTYPE_GOINGAROUNDENTITY || nTaskType == TASKTYPE_GOINGAROUNDOBSTACLE )
    {
        AI_RemoveCurrentTaskWithoutStartingNextTask( self );

        // there might be two in a row
	    if ( nTaskType == TASKTYPE_GOINGAROUNDENTITY || nTaskType == TASKTYPE_GOINGAROUNDOBSTACLE )
        {
            AI_RemoveCurrentTaskWithoutStartingNextTask( self );
        }
    }

	float fAroundDistance = (obstacleEnt->s.maxs.x - obstacleEnt->s.mins.x) * 1.3f;
    if ( fAroundDistance == 0.0f )
    {
		fAroundDistance = (obstacleEnt->absmax.x - obstacleEnt->absmin.x) * 1.3f;
    }
    float fOneFrameSpeed = AI_ComputeMovingSpeed( hook ) * 0.1f;
    fAroundDistance += fOneFrameSpeed;

	float fCheckDist = fAroundDistance * 1.2f;

	CVector destVector = destPoint - self->s.origin;
    destVector.z = 0.0f;
    destVector.Normalize();

    CVector vectorToEntity = obstacleEnt->s.origin - self->s.origin;
	vectorToEntity.z = 0.0f;
	vectorToEntity.Normalize();

    // if the entity that we are trying to go around is at the side, then
    // just adjust the moving vector a little to make it go around instead
    // of computing points around the entity
    float fAngle = ComputeAngle2D( destVector, vectorToEntity );
    if ( fAngle >= 30.0f && fAngle <= 330.0f )
    {
	    //	scale speed based on current frame's move_scale
        float fSpeed = AI_ComputeMovingSpeed( hook );

        float fTurnAngle = 360.0f - (60.0f - fAngle);
        if ( fAngle > 180.0f )
        {
            fTurnAngle = 60.0f - (360.0f - fAngle);
        }
        
        RotateVector2D( destVector, fTurnAngle );
        destVector.Normalize();

	    if ( AI_IsGap( self, self->s.origin, destVector, fOneFrameSpeed ) )
        {
            AI_StopEntity( self );
            return FALSE;
        }

	    float fScale = FRAMES_ComputeFrameScale( hook );
	    AI_SetVelocity( self, destVector, (fSpeed * fScale) );

	    if ( AI_HandleGettingStuck( self, destPoint, fSpeed ) )
	    {
		    return FALSE;
	    }

	    hook->last_origin = self->s.origin;
		//As long as we are not a flying or water unit we can do this.
	    if( !AI_IsFlyingUnit( self ) && !AI_IsInWater( self ) )
			AI_UpdateCurrentNode( self );
        return TRUE;
    }
    	
   
	int nWhichSide = 0;		// 0 => no side, 0x00000001 => left side free, 0x00000002 => right side free
	
	// check left side
	CVector pos;
	VectorMA( obstacleEnt->s.origin, leftVector, fCheckDist, pos );
    CVector groundPoint;
    if ( AI_FindGroundPoint( self, pos, groundPoint ) )
    {
	    tr = gstate->TraceLine_q2( self->s.origin, groundPoint, self,MASK_MONSTERSOLID );
	    if ( tr.fraction >= 1.0f && AI_IsGroundBelowBetween( self, self->s.origin, groundPoint ) )
	    {
		    // left side free
		    nWhichSide |= 0x00000001;	
	    }
    }
    	
	// check right side
	VectorMA( obstacleEnt->s.origin, rightVector, fCheckDist, pos );
	if ( AI_FindGroundPoint( self, pos, groundPoint ) )
    {
        tr = gstate->TraceLine_q2( self->s.origin, groundPoint, self,MASK_MONSTERSOLID );
	    if ( tr.fraction >= 1.0f && AI_IsGroundBelowBetween( self, self->s.origin, groundPoint ) )
	    {
		    // right side free
		    nWhichSide |= 0x00000002;	
	    }
    }

	if ( (nWhichSide & 0x00000001) && (nWhichSide & 0x00000002) )
	{
		if ( rnd() <= 0.5f )
		{
			nWhichSide = 0x00000001;
		}
		else
		{
			nWhichSide = 0x00000002;
		}
	}

	CVector nextPos;
	if ( nWhichSide & 0x00000001 )
	{
		// left side free
		VectorMA( self->s.origin, leftVector, fAroundDistance, pos );
        AI_FindGroundPoint( self, pos, groundPoint );
        pos = groundPoint;
		VectorMA( obstacleEnt->s.origin, leftVector, fAroundDistance, nextPos );
        AI_FindGroundPoint( self, nextPos, groundPoint );
        nextPos = groundPoint;
	}
	else
	if ( nWhichSide & 0x00000002 )
	{
		// right side free
		VectorMA( self->s.origin, rightVector, fAroundDistance, pos );
        AI_FindGroundPoint( self, pos, groundPoint );
        pos = groundPoint;
		VectorMA( obstacleEnt->s.origin, rightVector, fAroundDistance, nextPos );
        AI_FindGroundPoint( self, nextPos, groundPoint );
        nextPos = groundPoint;
	}
	else
	{
		// both sides blocked, wait for the entity to move out of the way
		if(obstacleEnt->flags & FL_MONSTER)
		{
			AI_AddNewTaskAtFront( self, TASKTYPE_WAITFORNOCOLLISION, obstacleEnt->s.origin );
			return TRUE;
		}
	    return FALSE;
	}		

	if ( nWhichSide != 0x00000000 )
	{
        
		tr = gstate->TraceBox_q2( self->s.origin, self->s.mins, self->s.maxs, nextPos, self, MASK_MONSTERSOLID );
		//tr = gstate->TraceLine_q2( self->s.origin, nextPos, self, MASK_MONSTERSOLID );
		if(tr.fraction >= 1.0f || strstr(tr.ent->className,"deco") || strstr(tr.ent->className,"button"))
		{
			AIDATA aiData;
			aiData.destPoint = nextPos;
			aiData.pEntity = obstacleEnt;
			AI_AddNewTaskAtFront( self, TASKTYPE_GOINGAROUNDENTITY, &aiData );
			
			aiData.destPoint = pos;
			AI_AddNewTaskAtFront( self, TASKTYPE_GOINGAROUNDENTITY, &aiData );
		}
		else
		{
			if(AI_IsSidekick(hook))
			{
				MAPNODE_PTR Node = NODE_GetClosestNode(self,obstacleEnt->s.origin);
				if(Node)
				{
					destPoint = Node->position;
				}			
				else
				{
					if(SIDEKICK_HasTaskInQue(self,TASKTYPE_SIDEKICK_COMEHERE))
					{
						AI_RestartCurrentGoal(self);
					}
					else
					{
						float Time = 0.75+rnd();
						AI_AddNewTaskAtFront(self,TASKTYPE_SIDEKICK_STOP, Time);
					}
				}
			}
			else
			{
				AI_AddNewTaskAtFront( self, TASKTYPE_WAITFORNOCOLLISION, obstacleEnt->s.origin );
			}
			return TRUE;			
		}
	}
	else
	{
		AIDATA aiData;
        aiData.destPoint = pos;
        aiData.pEntity = obstacleEnt;
        AI_AddNewTaskAtFront( self, TASKTYPE_GOINGAROUNDENTITY, &aiData );
	}

	return TRUE;
}

*/

// ----------------------------------------------------------------------------
//
// Name:		AI_GoAroundObstacle
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_GoAroundObstacle( userEntity_t *self )
{
	_ASSERTE( self );

	float fAroundDistance = (self->s.maxs.x - self->s.mins.x) * 1.25f;
	float fCheckDist = fAroundDistance * 1.2f;

	CVector directionVector = self->velocity;
	directionVector.Normalize();
	
	// left side
    CVector leftVector;
    AI_LeftVector( directionVector, leftVector );

	// right side
    CVector rightVector;
    AI_RightVector( directionVector, rightVector );

	int nWhichSide = 0;		// 0 => no side, 0x00000001 => left side free, 0x00000002 => right side free
	
	// check left side
	CVector pos;
	VectorMA( self->s.origin, leftVector, fCheckDist, pos );
	tr = gstate->TraceLine_q2( self->s.origin, pos, self, MASK_SOLID | CONTENTS_MONSTER );
	if ( tr.fraction >= 1.0f )
	{
		// left side free
		nWhichSide |= 0x00000001;	
	}
	
	// check right side
	VectorMA( self->s.origin, rightVector, fCheckDist, pos );
	tr = gstate->TraceLine_q2( self->s.origin, pos, self, MASK_SOLID | CONTENTS_MONSTER );
	if ( tr.fraction >= 1.0f )
	{
		// right side free
		nWhichSide |= 0x00000002;	
	}

	if ( (nWhichSide & 0x00000001) && (nWhichSide & 0x00000002) )
	{
		if ( rnd() <= 0.5f )
		{
			nWhichSide = 0x00000001;
		}
		else
		{
			nWhichSide = 0x00000002;
		}
	}

	if ( nWhichSide & 0x00000001 )
	{
		VectorMA( self->s.origin, leftVector, fAroundDistance, pos );
	}
	else
	if ( nWhichSide & 0x00000002 )
	{
		VectorMA( self->s.origin, rightVector, fAroundDistance, pos );
	}

	// only have one going around obstacle or an entity
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASKTYPE nTaskType = GOALSTACK_GetCurrentTaskType( pGoalStack );
	if ( nTaskType == TASKTYPE_GOINGAROUNDENTITY || nTaskType == TASKTYPE_GOINGAROUNDOBSTACLE )
    {
        AI_RemoveCurrentTaskWithoutStartingNextTask( self );

        // there might be two in a row
	    if ( nTaskType == TASKTYPE_GOINGAROUNDENTITY || nTaskType == TASKTYPE_GOINGAROUNDOBSTACLE )
        {
            AI_RemoveCurrentTaskWithoutStartingNextTask( self );
        }
    }

	if ( nWhichSide != 0x00000000 )
	{
		CVector nextPos;
		VectorMA( pos, directionVector, fCheckDist, nextPos );
		
		AI_AddNewTaskAtFront( self, TASKTYPE_GOINGAROUNDOBSTACLE, nextPos );
		AI_AddNewTaskAtFront( self, TASKTYPE_GOINGAROUNDOBSTACLE, pos );
	}
	else
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_GOINGAROUNDOBSTACLE, pos );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_FindNearClientOrSidekick
//
// Description:	finds the first client or bot within hook->active_dist units
//				of self.  Only entities with an opposing alignment are 
//				considered.
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *AI_FindNearClientOrSidekick( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	int alignment = hook->dflags & (DFL_BERSERK + DFL_EVIL + DFL_NEUTRAL + DFL_GOOD);
	int Found = 0;
	int Notarget = 0;
	float ClosestDistance = 0.0f;
	float ClosestClientDistance = 0.0f;//NSS[11/30/99]:Really for Co-Op... if we have say 2 clients and 1 sidekick
	userEntity_t *BadPerson,*Client;
	
	// NSS[1/11/00]:No enemies when cinematics are playing
	if(gstate->bCinematicPlaying)
	{
		return NULL;
	}
	//	neutral entities never attack first
	if (alignment == DFL_NEUTRAL)
	{
		return	NULL;
	}
	BadPerson = Client = NULL;
	userEntity_t *head = alist_FirstEntity( client_list );
	while ( head != NULL )
	{
		if(head->flags & FL_NOTARGET)
			Notarget = 1;
		// is this a client, bot or monster?
		if ( AI_IsAlive( head ) && !(head->flags & FL_NOTARGET) )
		{
			playerHook_t *hook = AI_GetPlayerHook( head );

			if (hook && !(hook->ai_flags & AI_IGNORE_PLAYER) && head->flags & (FL_CLIENT|FL_BOT) )
            {
				float fDistance = VectorDistance( self->s.origin, head->s.origin );
			    
				//NSS[11/30/99]:If we have a visible client let's chalk it up (also this is for Co-Op mode...to be able to find the closest client)
				int Visible = AI_IsVisible(self,head);
				if(head->flags & FL_CLIENT && Visible )
				{
					if(ClosestClientDistance < fDistance)
					{
						ClosestClientDistance = fDistance;
						Client = head;
					}
				}
				//NSS[11/6/99]:
				//If we have been hit and registered and enemy then check for an either or case
				//Handle Sidekicks and Other monsters
				if(self->enemy)
				{
					if ( fDistance < hook->active_distance || Visible )
					{
						if(!(head->flags & FL_CLIENT))//NSS[11/30/99]:Clients don't matter here
						{
							if(ClosestDistance < fDistance)
							{
								ClosestDistance = fDistance;
								BadPerson = head;
								Found = 1;
							}
						}
					}	
				}
				else//otherwise use the standard method
				{
					if ( fDistance < hook->active_distance || Visible )
					{
						if(!(head->flags & FL_CLIENT))
						{	
							if(ClosestDistance < fDistance && Visible)
							{
								ClosestDistance = fDistance;
								BadPerson = head;
								Found = 1;
							}
						}
					}
				}
		    }
        }

		head = alist_NextEntity( client_list );
	}
	
	//NSS[11/24/99]:If we have found a client and a sidekick let's figure out who we are gonna shoot at.
	if( Client != NULL && BadPerson != NULL)
	{
		//NSS[11/24/99]:As long as the client is alive.
		if(Client->deadflag == DEAD_NO)
		{
			float cDist = VectorDistance( self->s.origin, Client->s.origin );
			float bDist = VectorDistance( self->s.origin, BadPerson->s.origin );
			//If the client(player) is 250 units away more than the sidekick spotted attack the client
			if(bDist > cDist -150)
			{
				if(BadPerson->enemy && rnd() < 0.25) //
				{
					if(!(BadPerson->enemy == self))
						BadPerson = Client;	
				}
				else
				{
					BadPerson = Client;
				}
			}
		}
	}
	else if( Client != NULL)
	{
			BadPerson	= Client;
			Found		= 1;
	}
	if(Found && BadPerson!=NULL && AI_IsVisible(self,BadPerson) && !Notarget)
		return BadPerson;
	else
		return NULL;
}



// ----------------------------------------------------------------------------
//
// Name:		AI_FindNearestMonsterBot
// Description:	returns the closest monster or bot within hook->active_dist
//				units of self.  Only entities with an opposing alignment are
//				considered.
//
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *AI_FindNearestMonsterBot( userEntity_t *self )
{
	_ASSERTE( self );

	userEntity_t	*head;
	userEntity_t	*enemy;
	int				dist, enemy_d, alignment;
	playerHook_t	*hook = AI_GetPlayerHook( self );

	//	find alignment of searching creature
	alignment = hook->dflags & (DFL_BERSERK + DFL_EVIL + DFL_NEUTRAL + DFL_GOOD);
	
	//	neutral entities never attack unless provoked
	if (alignment == DFL_NEUTRAL)
	{
		return FALSE;
	}


	enemy = NULL;
	enemy_d = hook->active_distance;
	
	head = alist_FirstEntity( monster_list );
    while ( head )
	{
		if (head == self)
		{
			continue;
		}

		//	make sure target is alive and is targettable
		if ( AI_IsAlive( head ) && !(head->flags & FL_NOTARGET) )
		{
			playerHook_t *head_hook = AI_GetPlayerHook( head );

			if ((alignment & DFL_EVIL && head_hook->dflags & DFL_GOOD) ||
				(alignment & DFL_GOOD && head_hook->dflags & DFL_EVIL) || alignment & DFL_BERSERK)
			{
				dist = VectorDistance (self->s.origin, head->s.origin);

				//	only check visibility if enemy is closer than closest enemy
				if (dist < enemy_d)
				{
					if ( AI_IsVisible( self, head ) )
					{
						enemy = head;
						enemy_d = dist;
					}
				}
			}
		}

        head = alist_NextEntity( monster_list );
	}

	return	enemy;
}

//*******************************************************************************
//					individual AI functions
//*******************************************************************************

// ----------------------------------------------------------------------------
//
// Name:		AI_KilledEnemy
// Description:
//			monster's current enemy was killed
//
//			pop off everything having to do with it
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_KilledEnemy( userEntity_t *self )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	if ( self->enemy && self->enemy->flags & FL_CLIENT )
	{
		//	player's view will follow enemy
		self->enemy->enemy = self;

		AI_Dprintf( "Killed %s.\n", self->enemy->className );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ShouldFollow
// Description:
//	determines if a bot should attack, path to, or continue following
//	an enemy based on how far away the enemy is, and whether or not
//	the bot has a ranged weapon and is close enough to attack
// Input:
// Output:
// Note:	ATTACKMODE_SNIPING - entity will not follow unless its enemy
//								 is very close (within 2 nodes)
//
//
// ----------------------------------------------------------------------------
int	AI_ShouldFollow( userEntity_t *self )
{
	_ASSERTE( self );

	playerHook_t *hook = AI_GetPlayerHook( self );
	if(!self->enemy)
		return FALSE;
//	playerHook_t *ehook = AI_GetPlayerHook( self->enemy );// SCG[1/23/00]: not used
	int				bShouldFollow = TRUE;
//	int				nPathLength = 0;// SCG[1/23/00]: not used
//	PATHNODE_PTR	pPath = NULL;// SCG[1/23/00]: not used

	if ( hook->nAttackMode == ATTACKMODE_SNIPING )
	{
		bShouldFollow = FALSE;
	}
	else
	{
		int bEnemyAtHigherGround = FALSE;
		if ( self->enemy->s.origin.z > self->s.origin.z )
		{
			bEnemyAtHigherGround = TRUE;
		}

		float fZDistance = VectorZDistance( self->s.origin, self->enemy->s.origin );
		
		if ( ( AI_IsOnMovingTrain( self ) && !(fZDistance < (MAX_JUMP_HEIGHT*2) && bEnemyAtHigherGround == FALSE) ) ||
			 ( AI_IsOnMovingTrain( self->enemy ) && AI_IsNextPathTrain( self ) )
		   )
		{
			bShouldFollow = FALSE;
		}
	}

	return bShouldFollow;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_Pain
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Pain( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	ai_frame_sounds( self );
	
	AI_ZeroVelocity(self);

    if ( AI_IsEndAnimation(self) )
	{	
		AI_RemoveCurrentTask( self );
	}

	if ( !hook->cur_sequence || !strstr(hook->cur_sequence->animation_name, "hit") )
    {
        AI_RemoveCurrentTask( self );
    }

}



// ----------------------------------------------------------------------------
// NSS[11/13/99]:
// Name:			AI_ThinkFadeFast
// Description:	For Gibbing monsters 
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void AI_ThinkFadeFast(userEntity_t * self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	self->s.renderfx |= (SPR_ALPHACHANNEL);
	if(self->groundEntity)
		self->velocity.Zero();
	self->avelocity.Zero();
	self->ang_speed.Zero();
	if(hook)
	{
		if ( hook->pGoals )
		{
			GOALSTACK_Delete( hook->pGoals );
			hook->pGoals = NULL;
		}
		if ( hook->pScriptGoals )
		{
			GOALSTACK_Delete( hook->pScriptGoals );
			hook->pScriptGoals = NULL;
		}
	}
	RELIABLE_UNTRACK(self); //NSS[11/8/99]:Added this to remove track entities in case they were attached
	UNIQUEID_Remove( hook->szScriptName );
	free( hook->szScriptName );
	hook->szScriptName = NULL;

	alist_remove( self );
	self->remove( self );
}

// ----------------------------------------------------------------------------
// NSS[11/13/99]:
// Name:			AI_ThinkFade
// Description:	For monsters in no violence mode
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void AI_ThinkFade(userEntity_t * self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	self->s.renderfx |= (SPR_ALPHACHANNEL);
	if(self->groundEntity || self->waterlevel >= 2)
		self->velocity.Zero();
	self->avelocity.Zero();
	self->ang_speed.Zero();
	if(self->s.alpha < 0.10f)
	{
		if(hook)
		{
			if ( hook->pGoals )
			{
				GOALSTACK_Delete( hook->pGoals );
				hook->pGoals = NULL;
			}
			if ( hook->pScriptGoals )
			{
				GOALSTACK_Delete( hook->pScriptGoals );
				hook->pScriptGoals = NULL;
			}
		}

		RELIABLE_UNTRACK(self); //NSS[11/8/99]:Added this to remove track entities in case they were attached
		UNIQUEID_Remove( hook->szScriptName );
		free( hook->szScriptName );
		hook->szScriptName = NULL;
		alist_remove( self );
		self->remove( self );
	}
	else
	{
		CVector C1,C2;
		self->s.alpha *= 0.92f;
		C1.Set(0.0,0.0,0.0);
		C2.Set(0.85,0.45,0.15);
		self->s.color.Interpolate(C1,C2,self->s.alpha);
	}

	self->nextthink = gstate->time + 0.1f;
}


// ----------------------------------------------------------------------------
// NSS[11/15/99]:
// Name:		AI_HolyShitIdied
// Description:Handle Exp and the like... 
// Input:userEntity_t *self, userEntity_t *attacker
// Output:NA
// Note:This function rocks!!!!
// ----------------------------------------------------------------------------
void AI_HolyShitIdied(userEntity_t *self, userEntity_t *attacker, userEntity_t *inflictor)
{
	if (self->svflags & SVF_AWARDEDEXP)
	{
		return;
	}

	if(attacker)
	{
		if (!inflictor || (inflictor && stricmp(inflictor->className,"weapon_daikatana")))
			award_exps(attacker,self,0);	//NSS[11/23/99]:  award experience points after monster dies
	}
	else
		award_exps(gstate->attacker,self,0);

	// NSS[3/7/00]:Set the flag to award EXP.
	self->svflags |= SVF_AWARDEDEXP;

	// SCG[3/8/00]: Increment monstercount 
	playerHook_t *pHook = ( playerHook_t * )self->userHook;

	if( pHook != NULL )
	{
		if( pHook->type != TYPE_GOLDFISH && pHook->type != TYPE_SEAGULL )
		{
			if( pHook->type == TYPE_PROTOPOD )
			{
				if( !( pHook->nFlags & 0x0001 /*PROTOPOD_HATCHED*/ ) )
				{
					gstate->numMonstersKilled++;
				}
			}
			else
			{
				gstate->numMonstersKilled++;
			}
		}
	}
	self->flags &= ~SFL_TARGETABLE;
}



// ----------------------------------------------------------------------------
// NSS[3/31/00]:
// Name:		AI_DeadPain
// Description: Another function for death stuff... (Ayiieeee!)
// Input:userEntity_t *self, userEntity_t *attacker, float kick, int damage 
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void AI_DeadPain(userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );

	if ( !(self->fragtype & FRAGTYPE_NEVERGIB) && AI_GibLimit(self,damage)&& ((self->fragtype & FRAGTYPE_ALWAYSGIB) || self->deadflag == DEAD_DEAD))
	{
		// only go into here once when the thing dies.
		if (!(self->svflags & SVF_DEADMONSTER))
		{
			AI_DoDeathTarget( self );
			AI_DoSpawnName( self );
		}

		if(sv_violence->value == 0 )
		{
			if ( hook->pRespawnData )
			{
				self->think = SPAWN_RespawnThink;
				AI_SetNextThinkTime( self, 5.0f );
				AI_StartGibFest(self,attacker);
			}
			else
			{

				self->deadflag	 = DEAD_PUNK;
				
				self->pain		= NULL;
				self->use		= NULL;
				self->die		= NULL;
				self->prethink	= NULL;
				self->postthink = NULL;
				self->think		= AI_ThinkFadeFast;
				self->nextthink = gstate->time + 0.1f;
				self->solid		= SOLID_NOT;
				gstate->LinkEntity( self );
				AI_StartGibFest(self,attacker);
			}
		}
	}
}

// ----------------------------------------------------------------------------
//NSS[11/15/99]:
// Name:		AI_StartPain
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartPain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	
    if ( pCurrentTask && TASK_GetType( pCurrentTask ) == TASKTYPE_WAITONLADDERFORNOCOLLISION )
    {
        self->movetype = MOVETYPE_WALK;
    }

    if(AI_IsAlive(self))
	{
		AI_SetNextThinkTime( self, 0.1f );
		//AI_SetOkToAttackFlag( hook, FALSE );
		//AI_SetTaskFinishTime( hook, 3.0f );
		//AI_SetMovingCounter( hook, 0 );
	}



    if ( !AI_IsAlive( self ) )
    {
	    if ( !(self->fragtype & FRAGTYPE_NEVERGIB) && AI_GibLimit(self,damage)&& ((self->fragtype & FRAGTYPE_ALWAYSGIB) || self->deadflag == DEAD_DEAD))
		{
			// only go into here once when the thing dies.
			if (!(self->svflags & SVF_DEADMONSTER))
			{
				AI_DoDeathTarget( self );
				AI_DoSpawnName( self );
			}

			if(sv_violence->value == 0 )
			{
				if ( hook->pRespawnData )
				{
					self->think = SPAWN_RespawnThink;
					AI_SetNextThinkTime( self, 5.0f );
					AI_StartGibFest(self,attacker);
				}
				else
				{

					self->deadflag	 = DEAD_PUNK;
					
					self->pain		= AI_DeadPain;
					self->use		= NULL;
					self->die		= NULL;
					self->prethink	= NULL;
					self->postthink = NULL;
					self->think		= AI_ThinkFadeFast;
					self->nextthink = gstate->time + 0.1f;
					self->solid		= SOLID_NOT;
					gstate->LinkEntity( self );
					AI_StartGibFest(self,attacker);
				}
			}
			else
			{
				self->avelocity.Zero();
				self->ang_speed.Zero();
				AI_SetDeathBoundingBox(self);
				gstate->LinkEntity( self );
				self->pain			= AI_DeadPain;
				self->use			= NULL;
				self->die			= NULL;
				self->prethink		= NULL;
				self->postthink		= NULL;

				self->think			= AI_ThinkFade;
				self->nextthink		= gstate->time + 0.10f;
			}
	    }
		AI_HolyShitIdied(self, attacker,attacker);
		return;
    }

	// still in a pain function from last hit?
	if ( hook->pain_finished >= gstate->time || self == attacker )
	{
		return;
	}

	TASKTYPE nTaskType = GOALSTACK_GetCurrentTaskType( pGoalStack );

	pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	
	AIDATA_PTR pAIData = NULL;

	if( pCurrentTask )
	{
		pAIData = TASK_GetData( pCurrentTask );
	}

	////////////////////////////////////////////////////////////////
	//	make monsters fight one another
	////////////////////////////////////////////////////////////////
	
	_ASSERTE( attacker );
	//attacker = gstate->attacker;
	if ( attacker->flags & (FL_CLIENT + FL_MONSTER + FL_BOT) )
	{
		int new_enemy = FALSE;
		playerHook_t *ahook = AI_GetPlayerHook( attacker );

		if ( self->flags & FL_MONSTER )
		{
			//	attacked by a player or bot so attack back
			if ( attacker->flags & (FL_CLIENT + FL_BOT) )
			{
				hook->ai_flags &= ~AI_IGNORE_PLAYER;
				new_enemy = TRUE;
			}
			//	berserk, so attack anything 
			else 
			if ( hook->dflags & DFL_BERSERK )
			{
				new_enemy = TRUE;
			}
		}
		else 
		if ( self->flags & FL_BOT )
		{
			if ( nTaskType != TASKTYPE_RETREATTOOWNER )
			{
				if ( attacker->flags & FL_MONSTER )
				{
					new_enemy = TRUE;
				}
				else
				//	bots only attack entities with opposite alignment
				if ( hook->dflags & DFL_EVIL && (ahook->dflags & (DFL_GOOD | DFL_NEUTRAL)) )
				{
					new_enemy = TRUE;
				}
				else 
				if ( hook->dflags & DFL_GOOD &&	(ahook->dflags & (DFL_EVIL | DFL_NEUTRAL)) )
				{
					new_enemy = TRUE;
				}
				//	or anything if they are berserk
				else 
				if ( hook->dflags & DFL_BERSERK )
				{
					new_enemy = TRUE;
				}
			}
		}

		if ( !ahook->pNodeList )
		{
			new_enemy = FALSE;
		}
		else 
		if ( ahook->pNodeList->nCurrentNodeIndex < 0 )
		{
			new_enemy = FALSE;
		}

		if ( new_enemy && AI_IsAlive(self))
		{
			// new enemy sighted, attack!
			AI_EnemyAlert( self, attacker );

			if ( hook->fnStartAttackFunc )
			{
				
				// do not attack anymore if in an attack state
				GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
				if ( !pCurrentGoal || GOAL_GetType( pCurrentGoal ) != GOALTYPE_KILLENEMY || self->enemy != attacker )
				{
					// NSS[1/6/00]:If we get hit by the player or the bot let's rock!
					AI_SetOkToAttackFlag( hook, TRUE );

					hook->ai_flags |= AI_SEENPLAYER;
					hook->active_distance = 5000.0f;
					self->enemy = attacker;
					if(hook->type == TYPE_BUBOID)
					{
						// NSS[2/6/00]:Make sure we get rid of the cloud... this means we were hit while coming up from the ground.
						RELIABLE_UNTRACK(self);
					}
					AI_AddNewGoal( self, GOALTYPE_KILLENEMY, attacker );
                    return;
				}
			}
		}
	}
	
	// NSS[2/12/00]:Test to see what is hurting us
	if(!(attacker->flags & (FL_BOT|FL_CLIENT|FL_MONSTER)))
	{
		// NSS[2/12/00]:If it is a door smashing us let's try and get out of its way
		if(attacker->className && strstr(attacker->className,"door"))
		{
			CVector dir;
			self->s.angles.AngleToForwardVector( dir );
			dir.Negate();
			AI_FindNewCourse(self, &dir);
		}
	}
	//	bots and Superfly/Mikiko never go into pain frames
	if ( !(self->flags & FL_BOT) && damage > 0 )
	{
		if ( hook->nAttackMode == ATTACKMODE_SNIPING )
		{
			GOALSTACK_ClearAllGoals( pGoalStack );
			AI_AddNewGoal( self, GOALTYPE_SNIPE );
            return;
		}
		else
		{
   			int nRandomValue = (int)(Random() * 99.9);
			if ( nRandomValue < hook->pain_chance && hook->pain_finished < gstate->time)
			{
				//AI_Dprintf( "Starting TASKTYPE_PAIN.\n" );
				ai_frame_sounds( self );
				AI_ForceSequence( self, "hita" );

				AI_AddNewTaskAtFront( self, TASKTYPE_PAIN );
				hook->pain_finished = gstate->time + (self->s.frameInfo.endFrame - self->s.frameInfo.startFrame) * self->s.frameInfo.frameTime;
			}
		}
		

	}
	playerHook_t *ahook = AI_GetPlayerHook( attacker );
	//  attacked by another monster, attack back if
	//	not of the same type
	if ( attacker->flags & FL_MONSTER && hook->type != ahook->type )
	{
		GOALTYPE nGoalType = GOALSTACK_GetCurrentGoalType( pGoalStack );
		//NSS[11/6/99]:Added to make monsters attack other monsters that hit them
		if(nGoalType == GOALTYPE_IDLE)
			AI_AddNewGoal(self,GOALTYPE_KILLENEMY,FALSE);
		self->enemy = attacker;
	}
}


// ----------------------------------------------------------------------------
// NSS[1/22/00]:
// Name:		ai_generic_pain_handler
// Description:  userEntity_t *self, userEntity_t *attacker, float kick, int damage 
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_generic_pain_handler( userEntity_t *self, userEntity_t *attacker, float kick, int damage, int limit )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_StartPain( self, attacker, kick, damage );

	if(hook)
	{
		int nRandomValue = (int)(Random() * 99.9);
		if( AI_IsAlive( self ) && hook->pain_finished < gstate->time && nRandomValue < hook->pain_chance)
		{
			
			if( damage >= limit )
			{
				if(!AI_ForceSequence( self, "hita" ))
				{
					return;
				}
			}
			//else
			//{
			//	if(!AI_ForceSequence( self, "hitb" ))
			///	{
			//		if(!AI_ForceSequence( self, "hita" ))
			//		{
			//			return;
			//		}
			///	}
			//}
			hook->pain_finished = gstate->time + (self->s.frameInfo.endFrame - self->s.frameInfo.startFrame) * self->s.frameInfo.frameTime;
			AI_AddNewTaskAtFront(self,TASKTYPE_PAIN);
			AI_ZeroVelocity(self);
		}
	}
}




// ----------------------------------------------------------------------------
//NSS[11/15/99]:
// Name:		AI_StartBodyPain
// Description:
//				called when a dead body is shot
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartBodyPain( userEntity_t *self, userEntity_t *other, float kick, int damage )
{
    CVector vecGibDir;
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_Dprintf( "Starting StartBodyPain.\n" );

	if ( self->fragtype & FRAGTYPE_NEVERGIB )
	{
		// No gibs for you!
		return;
	}

	//	only gib a body if massive damage is done
	if ( ( AI_GibLimit(self,damage) || (self->fragtype & FRAGTYPE_ALWAYSGIB)))
	{	
		if(sv_violence==0)
		{
			//////////////////////////////////////////////////////////////
			//	major damage, so gib
			//////////////////////////////////////////////////////////////

			self->deadflag		= DEAD_DEAD;
			self->pain			= NULL;
			self->think			= AI_ThinkFadeFast;
			self->nextthink		= gstate->time + 0.1f;
			self->solid			= SOLID_NOT;	
			gstate->LinkEntity( self );
			AI_StartGibFest(self,other);
		}
		else
		{
			self->think		= AI_ThinkFade;
			self->pain		= AI_DeadPain;
			self->nextthink = gstate->time + 0.1f;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_Die
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Die( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if(AI_Determine_Room_Height(self, 30, 1)<= 10.0f || self->groundEntity || self->waterlevel >= 2)
		self->velocity.Zero();
	self->avelocity.Zero();
	self->ang_speed.Zero();
	if ( hook->cur_sequence && !strstr( hook->cur_sequence->animation_name, "die" ) )
	{
		// waiting for the current sequence to finish
		if ( self->s.frameInfo.frameState & FRSTATE_LAST ||
			 self->s.frameInfo.frameState & FRSTATE_STOPPED )
		{
			AI_ForceSequence( self, "diea" );
		}
	}
	else
	{

		ai_frame_sounds( self );

		if ( self->s.frameInfo.frameState & FRSTATE_LAST ||
			 self->s.frameInfo.frameState & FRSTATE_STOPPED )
		{
			if ( self->flags & FL_POSSESSED )
			{
				// FIXME: put the possesser back in their own body
				// this does it right now, but it's just a hack
				//camera_stop (hook->demon);
			}

			if ( hook->pRespawnData )
			{
				self->think = SPAWN_RespawnThink;
				AI_SetNextThinkTime( self, 5.0f );
			}
			else
			{
				self->deadflag	= DEAD_DEAD;
				
				if(sv_violence->value==0)
				{
					self->pain		= AI_DeadPain;
					self->die		= NULL;
					self->prethink	= NULL;
					self->postthink = NULL;

					if ( hook->pGoals )
					{
						GOALSTACK_Delete( hook->pGoals );
						hook->pGoals = NULL;
					}
					if ( hook->pScriptGoals )
					{
						GOALSTACK_Delete( hook->pScriptGoals );
						hook->pScriptGoals = NULL;
					}
					UNIQUEID_Remove( hook->szScriptName );
					free( hook->szScriptName );
					hook->szScriptName = NULL;
					alist_remove( self );
					self->fragtype |= FRAGTYPE_ALWAYSGIB;

					RELIABLE_UNTRACK(self); //NSS[11/8/99]:Added this to remove track entities in case they were attached

					//NSS[11/15/99]:
					AI_SetDeathBoundingBox(self);
					gstate->LinkEntity( self );
					self->think		= AI_ThinkFade;
					self->nextthink = gstate->time + 3.0f;

				}
				else
				{
					self->pain		= AI_DeadPain;
					self->die		= NULL;
					self->prethink	= NULL;
					self->postthink = NULL;
					self->solid     = SOLID_NOT;


					if ( hook->pGoals )
					{
						GOALSTACK_Delete( hook->pGoals );
						hook->pGoals = NULL;
					}
					if ( hook->pScriptGoals )
					{
						GOALSTACK_Delete( hook->pScriptGoals );
						hook->pScriptGoals = NULL;
					}

					UNIQUEID_Remove( hook->szScriptName );
					free( hook->szScriptName );
					hook->szScriptName = NULL;

					RELIABLE_UNTRACK(self); //NSS[11/8/99]:Added this to remove track entities in case they were attached

					//NSS[11/15/99]:
					AI_SetDeathBoundingBox(self);
					gstate->LinkEntity( self );
					self->think		= AI_ThinkFade;
					self->nextthink = gstate->time + 0.1f;
				}
			}

			gstate->LinkEntity( self );

			AI_Dprintf( "Finished TASKTYPE_DIE.\n" );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_DoGib
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_DoGib( userEntity_t *self )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	//////////////////////////////////////////////////////////////
	//	major damage, so gib
	//////////////////////////////////////////////////////////////

	self->deadflag	 = DEAD_DEAD;
	self->s.renderfx = RF_NODRAW;

	// don't gib if violence set or creature is ungibbable
	if ( sv_violence->value > 0 || (self->fragtype & FRAGTYPE_NEVERGIB) )
	{
		return;
	}

	AI_StartGibFest(self,self);
}


// ----------------------------------------------------------------------------
//NSS[11/15/99]:
// Name:		AI_DeathThink
// Description:
// Input:
// Output:
// Note:THIS FUNCTION GETS CALLED AFTER A SMALL DELAY HAS BEEN SET ON THE MONSTER
//
// ----------------------------------------------------------------------------
void AI_DeathThink(userEntity_t *self)
{
	self->deadflag	= DEAD_DEAD;
	//If we are done dying then let's set our bounding box to something small
    if(!self->groundEntity)
		self->movetype		= MOVETYPE_BOUNCE;
	else
		self->movetype		= MOVETYPE_NONE;

	
	self->avelocity.Zero();
	ai_frame_sounds( self );
	if(AI_IsEndAnimation( self ) || self->hacks < gstate->time)
	{
		AI_SetDeathBoundingBox(self);
		gstate->LinkEntity( self );
		self->think		= AI_ThinkFade;
		self->nextthink = gstate->time + 3.0f;
	}
	else
	{
		self->nextthink = gstate->time + 0.1f;
	}

}


// ----------------------------------------------------------------------------
//NSS[11/15/99]:
// Name:		AI_StartDie
// Description:
// Input:
// Output:
// Note:THIS FUNCTION GETS CALLED RIGHT AS THE MONSTER IS GOING TO DIE(BEFORE GIB)
//
// ----------------------------------------------------------------------------
void AI_StartDie( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
				  int damage, CVector &destPoint )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if(AI_Determine_Room_Height(self, 30, 1)<= 10.0f)
		self->velocity.Zero();
	self->avelocity.Zero();
	self->ang_speed.Zero();
	
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    if ( pCurrentTask && TASK_GetType( pCurrentTask ) == TASKTYPE_WAITONLADDERFORNOCOLLISION )
    {
        self->movetype = MOVETYPE_WALK;
    }

	AI_SetNextThinkTime( self, 0.1f );
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 5.0f );
	AI_SetMovingCounter( hook, 0 );

	AI_Dprintf( "Starting TASKTYPE_DIE.\n" );

	// new enemy sighted, attack!
	AI_EnemyAlert( self, attacker );    

	AI_HolyShitIdied(self, attacker, inflictor);

	// only go into here once when the thing dies.
	if (!(self->svflags & SVF_DEADMONSTER))
	{
		AI_DoDeathTarget( self );
        AI_DoSpawnName( self );
	}
    
	self->svflags -= (self->svflags & SVF_MONSTER);
	self->svflags |= SVF_DEADMONSTER;
    
    //gstate->Con_Printf ("damage = %d / gib_damage_point = %d\n",damage,(int)gib_damage_point->value);
	
	
	int bGib = !(self->fragtype & FRAGTYPE_NEVERGIB) &&
	           ( AI_GibLimit(self,damage) || (self->fragtype & FRAGTYPE_ALWAYSGIB));
	if ( !bGib || sv_violence->value > 0 ) // don't gib if violence set
    {
        if (self->deadflag == DEAD_NO)
        {
		    //////////////////////////////////////////////////////////////
		    //	not too much damage, so just die
		    //////////////////////////////////////////////////////////////
		    if ( AI_IsFlyingUnit( self ) || AI_IsInWater( self ) )
		    {
			    self->avelocity.Zero();
			    self->s.angles.Zero();
			    hook->dflags &= ~(DFL_FACEENEMY | DFL_FACEPITCH | DFL_FACEOWNER | DFL_FACEENEMY);
			    hook->dflags |= DFL_FACENONE;
		    }

			self->view_ofs.Set( 0.0f, 0.0f, -8.0f );
		    self->deadflag		= DEAD_DYING;
			self->takedamage	= DAMAGE_YES;
			self->pain			= AI_DeadPain;
			self->use			= NULL;
			self->die			= NULL;
			self->prethink		= NULL;
			self->postthink		= NULL;


			//NSS[11/18/99]:Check to see if we are on the ground
			if(!self->groundEntity)
				self->movetype		= MOVETYPE_BOUNCE;
			else
				self->movetype		= MOVETYPE_NONE;
			self->gravity		= 2.0f;

		    
			AI_AddNewTaskAtFront( self, TASKTYPE_DIE );
			self->think			= AI_DeathThink;
			self->nextthink		= gstate->time + 0.10f;
			self->hacks			= gstate->time + 4.0f;
			
			// die task need to be inserted since the die task gets triggered from outside 
		    // the AI framework
		    
		    AI_ForceSequence( self, "diea" );

		    ai_frame_sounds( self );
		    
			gstate->LinkEntity( self );
        }
    


	}
	else
    if ( bGib )
	{
		
		if(sv_violence->value == 0)
		{
			//////////////////////////////////////////////////////////////
			//	major damage, so gib
			//////////////////////////////////////////////////////////////

 			//gstate->Con_Printf ("self->fragtype = %d.\n",self->fragtype);

			CVector gibVector( 0.0f, 0.0f, self->s.maxs.z * 0.5f );
  
			
			// prepare to throw out the little gibs away from the body
			gibVector.Set( Random() * 16 - 8, Random() * 16 - 8, Random() * 16 - 8 );
        
			AI_StartGibFest(self, inflictor);

			if ( hook->pRespawnData )
			{
				self->think = SPAWN_RespawnThink;
				AI_SetNextThinkTime( self, 5.0f );
			}
			else
			{
				self->deadflag	 = DEAD_DEAD;
				
			//	FIXME: need to remove entity, but what if another monster is
			//	tracking it as an enemy?
				//if ( damage >= (int)gib_damage_point->value && !(hook->dflags & DFL_MECHANICAL) )
				{
					self->pain		= NULL;
					self->use		= NULL;
					self->die		= NULL;
					self->prethink	= NULL;
					self->postthink = NULL;
					self->think			= AI_ThinkFadeFast;
					self->nextthink		= gstate->time + 0.1f;
					self->solid			= SOLID_NOT;
					gstate->LinkEntity( self );
				}

			}
		}
	}


}

// ----------------------------------------------------------------------------
//
// Name:		AI_ChooseWanderGoal
// Description:	Chooses a random wander goal for wandering during 
//				GOALTYPE_FOLLOW
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

#define	WANDER_GOAL_DIST	(REACHED_GOAL_DIST + 16.0)

void AI_ChooseWanderGoal( userEntity_t *self )
{
	_ASSERTE( self );

	playerHook_t *hook = AI_GetPlayerHook( self );
	
	CVector	ang, end, down;
	float	yaw_add, base_yaw_add = 0;
	int		tries = 0, failed = 0;

	CVector	mins(-16, -16, -24);
	CVector	maxs(16, 16, 16);

AI_ChooseWanderGoal_retry:

	ang.Set( 0.0f, self->s.angles.yaw, 0.0f );

	if ( rnd() < 0.25f )
	{
		yaw_add = rnd() * 15 + 30 + base_yaw_add;
		if ( rnd() < 0.5 )
		{
			yaw_add *= -1.0;
		}

		ang.y = AngleMod( ang.y + yaw_add );
	}

	ang.AngleToVectors( forward, right, up );
	VectorMA( self->s.origin, forward, WANDER_GOAL_DIST, end );

	tr = gstate->TraceBox_q2( self->s.origin, self->s.mins, self->s.maxs, end, self, MASK_SOLID );
	if (tr.fraction == 1.0)
	{
		down.Set( end );
		down.AddZ( -24 );
		tr = gstate->TraceBox_q2( end, self->s.mins, self->s.maxs, down, self, MASK_SOLID );
	}
	else
	{ 
		failed = true;
	}

	if ( tr.fraction == 1.0 || failed )
	{
		tries++;

		if (tries > 4)
		{
//			playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

			//	picked a bad wander spot, so just give up this goal
			GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
//			GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
			GOALSTACK_GetCurrentGoal( pGoalStack );// SCG[1/23/00]: pCurrentGoal not used
			AI_RemoveCurrentGoal( self );

			return;
		}
		else
		{
			base_yaw_add += 90;
			goto AI_ChooseWanderGoal_retry;
		}
	}

	AI_AddNewTaskAtFront( self, TASKTYPE_MOVETOLOCATION, end );
//	com->SpawnMarker (self, end, "models/global/dv_goal.dkm", 2.0);
}

// ----------------------------------------------------------------------------
//
// Name:		AI_RandomWander
// Description:	
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_RandomWander( userEntity_t *self )
{
	_ASSERTE( self );

	playerHook_t *hook = AI_GetPlayerHook( self );

	float fDistance = VectorDistance(self->s.origin, hook->owner->s.origin);
	int chance = rand () % 100;
	
	if ( (hook->owner->velocity.Length() > 0 && fDistance > AI_GetRunFollowDistance(hook)) ||
		(chance >= 95))
	{
		//	stop wandering, go to stand
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
//		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		GOALSTACK_GetCurrentGoal( pGoalStack );// SCG[1/23/00]: pCurrentGoal not used
		AI_RemoveCurrentGoal(self);
	}
	else
	{
		AI_ChooseWanderGoal(self);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartRandomWander
// Description:	Starts a TASKTYPE_RANDOMWANDER, which chooses a random spot
//				near self to move to, then spawns a TASKTYPE_MOVETOLOCATION
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartRandomWander( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_SetNextThinkTime( self, 0.1f );
	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, -1.0f );
	AI_SetMovingCounter( hook, 0 );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_RANDOMWANDER );

	AI_Dprintf( "Starting TASKTYPE_RANDOMWANDER.\n" );

	AI_SetStateWalking( hook );

	AI_ChooseWanderGoal(self);

}

// ----------------------------------------------------------------------------
//
// Name:		AI_RunAway
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_RunAway( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if(hook)
	{
		if(hook->pPathList && hook->pPathList->nPathLength > 1)
		{
			AI_Move( self );
		}
		else
		if ( !hook->pPathList->pPath )
		{
			//	set up a path consisting of one node
			MAPNODE_PTR pRetreatNode = NODE_FurthestFrom( hook->pNodeList, self->enemy->s.origin );
			if ( !pRetreatNode )
			{
				//	nowhere to run to, start cower
				AI_AddNewTaskAtFront( self, TASKTYPE_WANDER );
				return;
			}

			PATHLIST_AddNodeToPath( hook->pPathList, pRetreatNode );

			float fDistance = VectorDistance( self->s.origin, pRetreatNode->position );
			float fSpeed = AI_ComputeMovingSpeed( hook );
			float fTime = (fDistance / fSpeed) + 1.0f;
			AI_SetTaskFinishTime( hook, fTime );

			AI_EnemyAlertNearbyMonsters( self, self->enemy );

			AI_Move( self );
		}
		else if(hook->pPathList->nPathLength <= 1)
		{
			if(!AI_HasTaskInQue(self,TASKTYPE_COWER))
			{
				AI_RemoveCurrentTask(self,TASKTYPE_COWER,self,FALSE);
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartRunAway
// Description:	Starts a TASKTYPE_RUNAWAY, which chooses the furthest node
//				that is linked to self's current node as the destination
//				node
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartRunAway( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_RUNAWAY );

	//	delete any previous path
	PATHLIST_KillPath( hook->pPathList );

	// set up a path consisting of one node
	MAPNODE_PTR pRetreatNode = NODE_FurthestFrom(hook->pNodeList, self->enemy->s.origin);
	if ( !pRetreatNode )
	{
		//	nowhere to run to, start cowering
		AI_RemoveCurrentTask( self, TASKTYPE_COWER );

		return;
	}

	AI_SetStateRunning( hook );
	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_RUNAWAY )
		{
			return;
		}
	}

	AI_Dprintf( "Starting TASKTYPE_RUNAWAY.\n" );

	PATHLIST_AddNodeToPath(hook->pPathList, pRetreatNode);

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );

	float fDistance = VectorDistance( self->s.origin, pRetreatNode->position );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 1.0f;
	AI_SetTaskFinishTime( hook, fTime );

	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_BriefCover
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_BriefCover( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	if ( AI_MoveTowardPoint( self, pAIData->destPoint, FALSE ) )
    {
        AI_RemoveCurrentTask( self );
    }
}

// ----------------------------------------------------------------------------
//
// Name:		AI_FindBriefCoverPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define BRIEF_COVER_DISTANCE    64.0f

int AI_FindBriefCoverPoint( userEntity_t *self, CVector &coverPoint )
{
    _ASSERTE( self );

    userEntity_t *pEnemy = self->enemy;
    if(!AI_IsAlive( pEnemy ))
	{
		AI_RemoveCurrentTask(self,FALSE);
		return FALSE;
	}

    CVector forwardVector = pEnemy->s.origin - self->s.origin;
    forwardVector.Normalize();

    CVector destPoint;

    CVector oppositeVector( -forwardVector.x, -forwardVector.y, 0.0f );
    VectorMA( self->s.origin, oppositeVector, BRIEF_COVER_DISTANCE, destPoint );

    CVector groundPoint;
    if ( AI_FindGroundPoint( self, destPoint, groundPoint ) )
    {
	    tr = gstate->TraceLine_q2( self->s.origin, groundPoint, self, MASK_SOLID|CONTENTS_MONSTER );
	    if ( tr.fraction >= 1.0f && AI_IsGroundBelowBetween( self, self->s.origin, groundPoint ) )
	    {
            coverPoint = groundPoint;
            return TRUE;
        }
    }

	// left side
    CVector leftVector;
	AI_LeftVector( forwardVector, leftVector );

    VectorMA( self->s.origin, leftVector, BRIEF_COVER_DISTANCE, destPoint );
    if ( AI_FindGroundPoint( self, destPoint, groundPoint ) )
    {
	    tr = gstate->TraceLine_q2( self->s.origin, groundPoint, self, MASK_SOLID|CONTENTS_MONSTER );
	    if ( tr.fraction >= 1.0f && AI_IsGroundBelowBetween( self, self->s.origin, groundPoint ) )
	    {
            coverPoint = groundPoint;
            return TRUE;
        }
    }

	// right side
	CVector rightVector;
    AI_RightVector( forwardVector, rightVector );
    VectorMA( self->s.origin, rightVector, BRIEF_COVER_DISTANCE, destPoint );
    if ( AI_FindGroundPoint( self, destPoint, groundPoint ) )
    {
	    tr = gstate->TraceLine_q2( self->s.origin, groundPoint, self, MASK_SOLID|CONTENTS_MONSTER );
	    if ( tr.fraction >= 1.0f && AI_IsGroundBelowBetween( self, self->s.origin, groundPoint ) )
	    {
            coverPoint = groundPoint;
            return TRUE;
        }
    }

    return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartBriefCover
// Description:	
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartBriefCover( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_BRIEFCOVER );

	//	delete any previous path
	PATHLIST_KillPath( hook->pPathList );

    CVector coverPoint;
    if ( !AI_FindBriefCoverPoint( self, coverPoint ) )
    {
		//	nowhere to run to, start cowering
		AI_RemoveCurrentTask( self, TASKTYPE_COWER );

		return;
	}

	AI_SetStateRunning( hook );
	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_BRIEFCOVER )
		{
			return;
		}
	}

    TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	TASK_Set( pCurrentTask, coverPoint );

	AI_Dprintf( "Starting TASKTYPE_BRIEFCOVER.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );

	float fDistance = VectorDistance( self->s.origin, coverPoint );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 1.0f;
	AI_SetTaskFinishTime( hook, fTime );

	AI_SetMovingCounter( hook, 0 );

    AI_BriefCover( self );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_FindMoveAwayPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define MOVEAWAY_DISTANCE			96.0f
#define CLOSER_MOVEAWAY_DISTANCE	32.0f

__inline static int AI_TestMoveAwayPoint( userEntity_t *self, CVector &mins, CVector &maxs, CVector &vector, CVector &moveAwayPoint )
{
	CVector endPoint;
	VectorMA( self->s.origin, vector, MOVEAWAY_DISTANCE, endPoint );
	if ( AI_IsGroundBelowBetween( self, self->s.origin, endPoint ) )
	{
		tr = gstate->TraceBox_q2( self->s.origin, mins, maxs, endPoint, self, MASK_SOLID );
		if ( tr.fraction >= 1.0f )
		{
			moveAwayPoint = endPoint;
			return TRUE;
		}
	}
	else
	{
		VectorMA( self->s.origin, vector, CLOSER_MOVEAWAY_DISTANCE, endPoint );
		if ( AI_IsGroundBelowBetween( self, self->s.origin, endPoint ) )
		{
			tr = gstate->TraceBox_q2( self->s.origin, mins, maxs, endPoint, self, MASK_SOLID );
			if ( tr.fraction >= 1.0f )
			{
				moveAwayPoint = endPoint;
				return TRUE;
			}
		}
	}

	return FALSE;
}

/*int AI_FindMoveAwayPoint( userEntity_t *self, userEntity_t *pEntity, CVector &moveAwayPoint )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	_ASSERTE( pEntity );

	CVector mins(self->s.mins.x + 4.0f, self->s.mins.y + 4.0f, self->s.mins.z + 16.0f );
	CVector maxs(self->s.maxs.x - 4.0f, self->s.maxs.y - 4.0f, self->s.maxs.z - 4.0f );

	CVector vector = self->s.origin - pEntity->s.origin;
	vector.z = 0.0f;
	vector.Normalize();

	CVector angle;
	VectorToAngles( vector, angle );
	CVector originalAngle = angle;

	if ( AI_TestMoveAwayPoint( self, mins, maxs, vector, moveAwayPoint ) == TRUE )
	{
		return TRUE;
	}

	angle.yaw = AngleMod( angle.yaw + 45.0f );
	angle.AngleToForwardVector( vector );
	if ( AI_TestMoveAwayPoint( self, mins, maxs, vector, moveAwayPoint ) == TRUE )
	{
		return TRUE;
	}

	angle.yaw = AngleMod( angle.yaw + 45.0f );
	angle.AngleToForwardVector( vector );
	if ( AI_TestMoveAwayPoint( self, mins, maxs, vector, moveAwayPoint ) == TRUE )
	{
		return TRUE;
	}

	angle.yaw = AngleMod( angle.yaw + 45.0f );
	angle.AngleToForwardVector( vector );
	if ( AI_TestMoveAwayPoint( self, mins, maxs, vector, moveAwayPoint ) == TRUE )
	{
		return TRUE;
	}

	angle = originalAngle;
	angle.yaw = AngleMod( angle.yaw - 45.0f );
	angle.AngleToForwardVector( vector );
	if ( AI_TestMoveAwayPoint( self, mins, maxs, vector, moveAwayPoint ) == TRUE )
	{
		return TRUE;
	}

	angle.yaw = AngleMod( angle.yaw - 45.0f );
	angle.AngleToForwardVector( vector );
	if ( AI_TestMoveAwayPoint( self, mins, maxs, vector, moveAwayPoint ) == TRUE )
	{
		return TRUE;
	}

	angle.yaw = AngleMod( angle.yaw - 45.0f );
	angle.AngleToForwardVector( vector );
	if ( AI_TestMoveAwayPoint( self, mins, maxs, vector, moveAwayPoint ) == TRUE )
	{
		return TRUE;
	}

	return FALSE;
}*/

int AI_FindMoveAwayPoint(userEntity_t *self, userEntity_t *pEntity, CVector &clearPoint )
{
	playerHook_t *hook	= AI_GetPlayerHook(self);
	// search the eight directions around the self to find a spawn point
	float delta_angle	= 0.0f;
	float angle_check	= 0.0f;
	float distance		= 64.0f;
	float OtherSpeed	= pEntity->velocity.Length();
	
	float Fstage,Lstage;
	CVector fromPoint = self->s.origin;

	CVector vector,Dir,bottomPoint,MidPoint,endPoint,angles,mins,maxs;

	// NSS[3/13/00]:Get the vector away from the thing hitting us.
	Dir = self->s.origin - pEntity->s.origin;
	Dir.Normalize();

	
	if(OtherSpeed > 75)
	{
		AI_SetStateRunning(hook);
		Fstage = 96.0f;
		Lstage = 128.0f;
	}
	else
	{
		AI_SetStateWalking(hook);
		Fstage = 48.0f;
		Lstage = 96.0f;
	}
	// NSS[3/14/00]:First stage you always use the nodes to find an away point.
	//if not possible then we go into a little more complicated code.
	CVector AwayPoint = self->s.origin + (Dir * Fstage);
	MAPNODE_PTR Node = 	NODE_GetClosestNode( self, AwayPoint);
	AwayPoint = self->s.origin + (Dir * Lstage);
	// NSS[3/14/00]:Get the node closest to us.
	if(Node)
	{
		MAPNODE_PTR fNode = NULL;
		float MaxDistance,TestDistance; 
		
		MaxDistance = 1024.0f;
		

		if(Node->nNumLinks)
		{
			for(int i=0; i < Node->nNumLinks ; i++)
			{
				// update the current node for this entity
				NODELIST_PTR pNodeList = hook->pNodeList;
				_ASSERTE( pNodeList );
				NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
				
				//get the node of this index.
				MAPNODE_PTR tNode =  NODE_GetNode( pNodeHeader, Node->aLinks[i].nIndex );
				//as long as it is valid.
				if(tNode != NULL)
				{
					TestDistance = VectorDistance(tNode->position, AwayPoint);
					if(TestDistance < MaxDistance)
					{
						fNode = tNode;
						MaxDistance = TestDistance;
					}
				}
			}
		}
		if(fNode != NULL)
		{
			clearPoint = fNode->position;
			//AI_RemoveCurrentTask(self,TASKTYPE_MOVETOLOCATION,fNode->position);
			return TRUE;
		}
	}

	
	// NSS[3/14/00]: If we make it here then we are more than likely far from nodes, so let's just try and make do with what we have.
	mins = self->s.mins * 0.50f;
	maxs = self->s.maxs * 0.50f;
	
	// NSS[3/13/00]:Get the vector away from the thing hitting us.
	Dir = self->s.origin - pEntity->s.origin;
	Dir.Normalize();
	VectorToAngles(Dir,angles);
	
	delta_angle = angles.yaw;
	angles.AngleToForwardVector( vector );

	while(angle_check < 360)
	{
		while(distance > 8.0f)
		{
			MidPoint = fromPoint + (vector*distance);

			tr = gstate->TraceBox_q2(self->s.origin , mins, maxs, MidPoint, self, self->clipmask );

			if ( tr.fraction >= 1.0f && !tr.allsolid && !tr.startsolid && AI_IsGroundBelowBetween(self,self->s.origin,MidPoint,32.0f))
			{
				//If we made it then cool.
				//clearPoint = Node->position;
				clearPoint = MidPoint;
				return TRUE;
			}
			distance -= 8.0f;
		}
		
		// NSS[3/13/00]:Handle wrap around
		if(delta_angle >= 360)
			delta_angle = 0.0f;
		delta_angle += 15.0f;

		angle_check += 15.0f;
		angles.yaw = delta_angle;
		angles.AngleToForwardVector( vector );

		//reset the distance for the next angle
		distance = 64.0f;
	}
	// NSS[3/13/00]:Last resort... 
	MidPoint = self->s.origin + (Dir*48.0f);
	Node = 	NODE_GetClosestNode( self, MidPoint);
	if(Node)
	{
		clearPoint = Node->position;
		return TRUE;
	}
	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_MoveAway
// Description:	function to get NPC's to move away from the player
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_MoveAway( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData );

	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsCloseDistance2(self,fXYDistance) && fZDistance < 32.0f )
	{
		
		AI_RemoveCurrentTask( self );
		return;
	}

	AI_MoveTowardPoint( self, pAIData->destPoint, FALSE );
}
int AI_ComputeBestAwayYawPoint(userEntity_t *self, CVector &flyAwayPoint, int Distance, int Resolution);

// ----------------------------------------------------------------------------
//
// Name:		AI_StartMoveAway
// Description:	Starts a TASKTYPE_MOVEAWAY, which chooses the closest node
//				that is linked to self's current node as the destination
//				node
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartMoveAway( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_MOVEAWAY );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData );

	userEntity_t *pOtherEntity = pAIData->pEntity;
	if ( !pOtherEntity )
	{
		AI_RemoveCurrentTask( self );
		return;
	}
	
	if ( !AI_IsAlive( pOtherEntity ) )
	{
		// the player is dead..  superfly should just stand there
		return;
	}

	CVector moveAwayPoint;
	int bSuccess = AI_FindMoveAwayPoint( self, pOtherEntity, moveAwayPoint );
	if ( bSuccess )
	{
		//if ( AI_StartMoveBackwards( self ) == FALSE )
        if ( AI_StartMove( self ) == FALSE )
		{
			TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
            _ASSERTE( pCurrentTask );
			if ( TASK_GetType( pCurrentTask ) != TASKTYPE_MOVEAWAY )
			{
				return;
			}
		}

		// store moving direction direction
        self->movedir = moveAwayPoint - self->s.origin;
        self->movedir.Normalize();

        TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		TASK_Set( pCurrentTask, moveAwayPoint );

		AI_Dprintf( "Starting TASKTYPE_MOVEAWAY.\n" );

		AI_SetOkToAttackFlag( hook, FALSE );
		AI_SetTaskFinishTime( hook, 2.0f );
		AI_SetMovingCounter( hook, 0 );

		AI_SetNextThinkTime( self, 0.1f );
	}
	else
	{
		AI_RemoveCurrentTask( self );
	}
}
// ----------------------------------------------------------------------------
//
// Name:		AI_StartIdleSettings
// Description: Adjust the entity's settings appropriately for starting to idle.
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartIdleSettings( userEntity_t *self ) 
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if(AI_IsSidekick(hook) && (!self->enemy || !AI_IsAlive(self->enemy) || !hook->bInAttackMode))
	{
		AI_ZeroVelocity( self );
		if(self->enemy && !AI_IsAlive(self))
			self->enemy = NULL;
	}
	else if(!AI_IsSidekick(hook))
	{
		AI_ZeroVelocity( self );	
		self->enemy = NULL;
	}

	AI_SetStateIdle( hook );

    AI_Dprintf( "Starting TASKTYPE_IDLE.\n" );

	AI_SetNextThinkTime( self, 0.1f );
	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, -1.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//NSS[12/4/99]:
// Name:		AI_StartIdle
// Description:
// Input:
// Output:
// Note:Added a trap to make sure we don't start the ambient animations until the
// end of the animation.  Also, added a trap for the initial start of ambient mode.
//
// ----------------------------------------------------------------------------
void AI_StartIdle( userEntity_t *self ) 
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    hook->nFollowing = FOLLOWING_NONE;
        
    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    //_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_IDLE );

	if ( hook->fnStartIdle )
    {
        hook->fnStartIdle( self );
    }
    else
    {
	    char szAnimation[16];
        if ( AI_IsCrouching( hook ) )
        {
            AI_SelectCrouchingAnimation( self, szAnimation );
            frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
            if ( pSequence && AI_IsEndAnimation(self))
            {
                if(self->movetype == MOVETYPE_FLY || self->movetype == MOVETYPE_HOVER || self->movetype == MOVETYPE_SWIM)
				{
					AI_ForceSequence(self, szAnimation, FRAME_LOOP );
				}
				else
				{
					AI_ForceSequence(self, szAnimation, pSequence->flags );
				}
				
            }
        }
        else
        {
            AI_SelectAmbientAnimation( self, szAnimation );
            frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
            if( pSequence != NULL)
			{
				if ( (AI_IsEndAnimation(self)||self->s.frame==0||hook->bInScriptMode) && !(self->flags & FL_CINEMATIC))
				{
					if(self->movetype == MOVETYPE_FLY || self->movetype == MOVETYPE_HOVER || self->movetype == MOVETYPE_SWIM)
					{
						AI_ForceSequence(self, szAnimation, FRAME_LOOP );
					}
					else
					{
						AI_ForceSequence(self, szAnimation, pSequence->flags );
					}
				}
			}
		}

		// Not finishing a sequence, so this creature may proceed to the settings stage.
		AI_StartIdleSettings(self);
    }
}	




// ----------------------------------------------------------------------------
//
// Name:		AI_RestartIdle
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_RestartIdle( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	//_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_IDLE );

	AI_ZeroVelocity( self );

	if ( hook->fnStartIdle )
    {
        hook->fnStartIdle( self );
    }
	else
	{
		char szAnimation[16];
		AI_SelectAmbientAnimation( self, szAnimation );
		frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
		if ( pSequence )
		{
			if ( AI_StartSequence(self, pSequence, pSequence->flags) == FALSE )
			{
				return;
			}
		}
	}

	AI_SetNextThinkTime( self, 0.1f );
}	
int AI_CheckForEnemy( userEntity_t *self );
// ----------------------------------------------------------------------------
//
// Name:		AI_Idle
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Idle( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	ai_ambient_sound( self );
	//NSS[11/16/99]:
	//If we are a monster and we have seen the player
	//and we have made it back to IDLE mode.. we will
	//"wander"/look around 
	if(hook)
	{
		NODELIST_PTR pNodeList = hook->pNodeList;
		if(pNodeList != NULL)
		{
			if( (self->flags&FL_MONSTER && ((hook->ai_flags & AI_SEENPLAYER) || gstate->GetCvar("sv_aiwander"))) && self->movetype != MOVETYPE_NONE)
			{
				if(hook->type == TYPE_COLUMN && !(hook->nFlags & 0x02))
				{
					if(AI_IsEndAnimation(self))
					{
						// restart idle
						AI_RestartIdle( self );
					}
					return;
				}
				if(self->enemy)
				{
					if(hook->type == TYPE_COLUMN && !(hook->nFlags & 0x02))
					{
						
					}
					//If there are no more players to kill let's just roam around
					if(!AI_IsAlive(self->enemy) && !AI_CheckForEnemy( self ))
						hook->ai_flags |= AI_IGNORE_PLAYER;
				}
				AI_AddNewGoal(self,GOALTYPE_WANDER);
				return;
			}
		}
	}
	
	
	if(self->flags & FL_CINEMATIC)
	{
		if(hook && hook->cur_sequence && hook->cur_sequence->animation_name && (strstr(hook->cur_sequence->animation_name,"walk") || strstr(hook->cur_sequence->animation_name,"run")))
		{
			AI_ForceSequence( self, "amba", FRAME_LOOP );
		}
	}
	if ( !AI_IsAIDisabled() && !(self->flags & FL_BOT) || hook->bInScriptMode)
	{
		// do an ambient standing sequence
		// FIXME: won't search for targets while doing this...
		// may be a problem for long ambients, may need a special
		// transition think for this
		if ( self->s.frameInfo.frameState & FRSTATE_LAST  )
		{
			// restart idle
			AI_RestartIdle( self );
		}
	}
	else
	{
	    SIDEKICK_HandleIdle( self );
    }
}

// ----------------------------------------------------------------------------
//
// Name:		AI_JumpUp
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_JumpUp( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	if(self->delay < gstate->time)
	{
		
		if ( self->groundEntity)
		{
			if(AI_IsSidekick(hook))
			{
				if(!Check_Que(self,LANDING,5.0f))
				{
					SideKick_TalkAmbient(self, LANDING, 1);
				}
			}
			AI_UpdateCurrentNode( self );
			AI_RemoveCurrentTask( self );
			return;
		}

		ai_frame_sounds( self );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartJumpUp
// Description:
//	bot is trying to jump up onto a low obstruction
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartJumpUp( userEntity_t *self )
{
	_ASSERTE( self );
 	playerHook_t *hook = AI_GetPlayerHook( self );

	CVector Dir;

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_JUMPUP );

	char szAnimation[16];
	AI_SelectJumpingAnimation( self, szAnimation );
	AI_ForceSequence( self, szAnimation );

	if(AI_IsSidekick(hook))
	{
		if(!Check_Que(self,JUMP,5.0f))
		{
			SideKick_TalkAmbient(self, JUMP, 1);
		}
	}
	AI_Dprintf( "Starting TASKTYPE_JUMPUP.\n" );

	self->s.angles.AngleToForwardVector(Dir);

	CVector vector( Dir.x, Dir.y, 1.0f );
	
	AI_SetVelocity( self, vector, hook->upward_vel*1.15);

	self->velocity.x *= 0.15f;
	self->velocity.y *= 0.15f;
	//self->velocity.z *= 0.75f;
	
	//hook->ai_flags |= AI_JUMP;
	//self->groundEntity = NULL;


	AI_SetNextThinkTime( self, 0.1f );
	self->delay = gstate->time + 0.20;
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 5.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_JumpForward
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_JumpForward( userEntity_t *self )
{
	_ASSERTE( self );
	if(self->delay < gstate->time)
	{
		if ( self->groundEntity )
		{
			AI_UpdateCurrentNode( self );

			AI_RemoveCurrentTask( self );
			return;
		}
	}

	ai_frame_sounds( self );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartJumpForward
// Description:
//	bot is trying to jump up onto a low obstruction
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartJumpForward( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_JUMPFORWARD );

	char szAnimation[16];
	AI_SelectJumpingAnimation( self, szAnimation );
	AI_ForceSequence(self,szAnimation,FRAME_ONCE);
	AI_Dprintf( "Starting TASKTYPE_JUMPFORWARD.\n" );

	AI_SetVelocity( self, forward, hook->forward_vel * 2.0 );
	self->velocity.z = hook->upward_vel*0.75;

	self->delay = gstate->time + 0.20;

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1 );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartAttack( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	hook->nSpecificAttackMode = 0;

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_ATTACK );

	if ( !self->enemy )
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		AIDATA_PTR pAIData = GOAL_GetData(pCurrentGoal);
		self->enemy = pAIData->pEntity;
	}

	if ( !AI_IsAlive( self->enemy ) )
	{
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
		if ( nGoalType == GOALTYPE_KILLENEMY )
		{
			GOAL_Satisfied( pCurrentGoal );
		}

		AI_RemoveCurrentTask( self );
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_ATTACK.\n" );

	AI_SetStateAttacking(hook);

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 30.0f );
	AI_SetMovingCounter( hook, 0 );

	if ( hook->fnStartAttackFunc )
	{
		hook->fnStartAttackFunc( self );
	}
}
void CAMBOT_FoundPlayer( userEntity_t *self, userEntity_t *pPlayer );
// ----------------------------------------------------------------------------
//
// Name:		AI_Attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Attack( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *pEnemy = self->enemy;
	if ( !AI_IsAlive( pEnemy ) && (hook->ai_flags&AI_SEENPLAYER))
	{
		/*GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
		if ( nGoalType == GOALTYPE_KILLENEMY )
		{
			GOAL_Satisfied( pCurrentGoal );
		}

		AI_RemoveCurrentTask( self );*/
		NODELIST_PTR pNodeList = hook->pNodeList;
		if(pNodeList != NULL)
		{
			AI_AddNewGoal(self,GOALTYPE_WANDER);
		}
		else
		{
			AI_RemoveCurrentTask(self,FALSE);
		}
		//AI_RemoveCurrentTask(self,TASKTYPE_WANDER,self);
		return;
	}

	_ASSERTE( AI_IsAlive( self->enemy ) );

    int bEnemyVisible = AI_IsChaseVisible( self );

	_ASSERTE( AI_IsAlive( self->enemy ) );

    if ( AI_IsSidekick( hook ) )
    {
        // extend the time
		AI_SetTaskFinishTime( hook, 1.0f );
    }
    else
    {
        if ( bEnemyVisible )
	    {
		    // alert enemy's presense every few seconds
	        if ( ((int)gstate->time % 10) == 0 )
	        {
                AI_EnemyAlert( self, pEnemy );
            }

            if ( AI_IsEntityAbove( self, pEnemy ) )
            {
                CVector freePoint;
                if ( AI_FindCollisionFreePoint( self, freePoint ) )
                {
                    AI_MoveTowardPoint( self, freePoint );
                    return;
                }
            }
            else
            if ( AI_IsEntityBelow( self, pEnemy ) )
            {
                // since enemy is below, find a point respective of the enemy
                CVector freePoint;
                if ( AI_FindCollisionFreePoint( pEnemy, freePoint ) )
                {
                    AI_MoveTowardPoint( self, freePoint );
                    return;
                }
            }

            // extend the time
		    AI_SetTaskFinishTime( hook, 10.0f );
	    }
	    else
	    {
		    AI_RemoveCurrentTask( self );
		    return;
	    }
    }

	_ASSERTE( AI_IsAlive( self->enemy ) );
    if ( hook->fnAttackFunc && self->enemy )
	{
		hook->fnAttackFunc( self );
	}
	else if(hook->type == TYPE_CAMBOT)
	{
		CAMBOT_FoundPlayer( self, self->enemy );
	}

}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartMoveForWalkingUnit
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_StartMoveForWalkingUnit( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( AI_IsGroundUnit(self) )
    {
		if ( AI_IsCrouching( hook ) )
        {
			char szAnimation[16];
			AI_SelectCrouchMovingAnimation( self, szAnimation );
            //if ( !hook->cur_sequence) //|| _stricmp( hook->cur_sequence->animation_name, szAnimation ) != 0 )
            //{
			    if ( AI_ForceSequence(self, szAnimation, FRAME_LOOP) == FALSE )
			    {
                    return FALSE;
                }
            //}
        }
        else
        if ( AI_IsStateRunning( hook ) || AI_IsStateAttacking( hook ) )
		{
			char szAnimation[16];
			AI_SelectRunningAnimation( self, szAnimation );
			// NSS[2/16/00]:Swim Stuff
			if(AI_IsSidekick(hook) && self->waterlevel > 2)
				strcpy(szAnimation,"swim");

            if ( !hook->cur_sequence || _stricmp( hook->cur_sequence->animation_name, szAnimation ) != 0 )
            {
				if( !hook->cur_sequence || !strstr( hook->cur_sequence->animation_name, "hit" ))
				{
					if ( AI_ForceSequence(self, szAnimation, FRAME_LOOP) == FALSE )
					{
						if ( AI_ForceSequence(self, "runa", FRAME_LOOP) == FALSE )
						{
							return FALSE;
						}
					}
				}
				else
				{
					if ( AI_ForceSequence(self, szAnimation, FRAME_LOOP) == FALSE  )
					{
						if ( AI_ForceSequence(self, "walka", FRAME_LOOP) == FALSE )
						{
							return FALSE;
						}
					}
				}
		    }
			else if( hook->cur_sequence && hook->cur_sequence->animation_name)
			{
				AI_ForceSequence(self,hook->cur_sequence->animation_name, FRAME_LOOP);
			}
        }
		else
		{
			char szAnimation[16];
			AI_SelectWalkingAnimation( self, szAnimation );
			// NSS[2/16/00]:Swim Stuff
			if(AI_IsSidekick(hook) && self->waterlevel > 2)
				strcpy(szAnimation,"swim");
            if ( !hook->cur_sequence || _stricmp( hook->cur_sequence->animation_name, szAnimation ) != 0 )
            {
                if ( AI_ForceSequence(self, szAnimation, FRAME_LOOP) == FALSE )
			    {
				    if ( AI_ForceSequence(self, "walka", FRAME_LOOP) == FALSE )
				    {
					    return FALSE;
				    }
			    }
		    }
        }
    }

    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartMove
// Description:
// Input:
// Output:
//				TRUE	=> started the move animation sequence and moving
//				FALSE	=> not moving due to transition, current sequence need to
//							to be completed, etc.
//
// Note:	if walk == TRUE, then walking frames will be started, otherwise
//			running frames will start.  This has no effect on flying creatures
//
// ----------------------------------------------------------------------------
int AI_StartMove( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    
	if ( hook->pMovingAnimation )
	{
//		if ( AI_StartSequence(self, hook->pMovingAnimation, FRAME_LOOP) == FALSE )
		if ( AI_ForceSequence(self, hook->pMovingAnimation, FRAME_LOOP) == FALSE )
		{
			return FALSE;
		}
	}
	else
	{
		if ( AI_IsFlyingUnit( self ) )
		{
			if ( AI_ForceSequence(self, "flya", FRAME_LOOP) == FALSE )
			{
				return FALSE;
			}
		}
		else 
		if ( AI_IsInWater( self ) )
		{
			if ( AI_IsGroundUnit(self) )
			{
				if ( !AI_StartMoveForWalkingUnit( self ) )
				{
					return FALSE;
				}
			}
			else
			{
				if(AI_IsEndAnimation(self) || self->s.frame == 0)
				{
					if(AI_IsSidekick(hook))
					{
						AI_ForceSequence(self, "swim", FRAME_LOOP);
					}
					else
					if ( AI_ForceSequence(self, "swima", FRAME_LOOP) == FALSE )
					{
						return FALSE;
					}
				}
			}
		}
		else
		{
			if ( !AI_StartMoveForWalkingUnit( self ) )
			{
				AI_StartMoveForWalkingUnit( self );
			}
		}
	}
	hook->nMoveCounter = 0;
	hook->nWaitCounter = 0;

	// this is needed so that the code will not detect it has not moved 
	// in the first update
	CVector org( self->s.origin.x + 10.f, self->s.origin.y + 10.0f, self->s.origin.z );
	hook->last_origin.Set( org );

	// get rid of old path if one exists
	if(hook->pPathList)
		PATHLIST_KillPath( hook->pPathList );

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartMoveBackwards
// Description:
// Input:
// Output:
//				TRUE	=> started the move animation sequence and moving
//				FALSE	=> not moving due to transition, current sequence need to
//							to be completed, etc.
//
// Note:	if walk == TRUE, then walking frames will be started, otherwise
//			running frames will start.  This has no effect on flying creatures
//
// ----------------------------------------------------------------------------
int AI_StartMoveBackwards( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->pMovingAnimation )
	{
		if ( AI_StartSequenceInReverse(self, hook->pMovingAnimation, FRAME_LOOP) == FALSE )
		{
			return FALSE;
		}
	}
	else
	{
		if ( AI_IsFlyingUnit( self ) )
		{
			if ( AI_StartSequenceInReverse(self, "flya", FRAME_LOOP) == FALSE )
			{
				return FALSE;
			}
		}
        else
        if ( AI_IsWaterUnit( self ) )
        {
			if ( AI_StartSequenceInReverse(self, "swima", FRAME_LOOP) == FALSE )
			{
				return FALSE;
			}
        }
		else
		{
			if ( AI_IsCrouching( hook ) )
            {
				char szAnimation[16];
				AI_SelectCrouchMovingAnimation( self, szAnimation );
				if ( AI_ForceSequenceInReverse(self, szAnimation, FRAME_LOOP) == FALSE )
				{
                    return FALSE;
                }
            }
            else
			if ( AI_IsStateRunning( hook ) || AI_IsStateAttacking( hook ) )
			{
				char szAnimation[16];
				AI_SelectRunningAnimation( self, szAnimation );
				if ( AI_ForceSequenceInReverse(self, szAnimation, FRAME_LOOP) == FALSE )
				{
					if ( AI_ForceSequenceInReverse(self, "walka", FRAME_LOOP) == FALSE )
					{
						return FALSE;
					}
				}
			}
			else
			{
				char szAnimation[16];
				AI_SelectWalkingAnimation( self, szAnimation );
				if ( AI_ForceSequenceInReverse(self, szAnimation, FRAME_LOOP) == FALSE )
				{
					if ( AI_ForceSequenceInReverse(self, "walka", FRAME_LOOP) == FALSE )
					{
						return FALSE;
					}
				}
			}
		}
	}

	hook->nMoveCounter = 0;
	hook->nWaitCounter = 0;

	// this is needed so that the code will not detect it has not moved 
	// in the first update
	CVector org( self->s.origin.x + 10.f, self->s.origin.y + 10.0f, self->s.origin.z );
	hook->last_origin.Set( org );

	// get rid of old path
	PATHLIST_KillPath( hook->pPathList );

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ComputeAlongWallVector
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ComputeAlongWallVector( userEntity_t *self, CVector &forward, CVector &normal, CVector &dir )
{
	//NSS[12/1/99]:La la... 
	AI_HandleWallCollision(  self, forward, normal, dir );
}



// ----------------------------------------------------------------------------
//
// Name:		AI_ComputeAlongWallVectorForAir
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ComputeAlongWallVectorForAir( userEntity_t *self, CVector &forward, CVector &normal, CVector &dir )
{
	_ASSERTE( self );

	forward.Normalize();

    //	rotate right of wall normal
	CVector newdir;
	if ( normal.x == 0.0f && normal.y == 0.0f )
	{
		newdir.Set( -forward.y, forward.x, 0.0f );
	}
	else
	{
		newdir.Set( normal.y, -normal.x, 0.0f );
	}
	newdir.Normalize();

	// get cos of angle between goal direction and vector along wall
	float dot = DotProduct( forward, newdir );
	if ( dot > 0.0f )
	{
		dir.x = normal.y;
		dir.y = -normal.x;
	}
	else
	{
		dir.x = -normal.y;
		dir.y = normal.x;
	}

    float fAbsNormalX = fabs(normal.x);
    float fAbsNormalY = fabs(normal.y);
	float fAbsNormalZ = fabs(normal.z);
    if ( fAbsNormalZ > fAbsNormalX && fAbsNormalZ > fAbsNormalY )
    {
        dir.Set( 0.0f, 0.0f, normal.z );
    }

    dir.Normalize ();

}


// ----------------------------------------------------------------------------
//
// Name:		ai_ChooseLedge
// Description:
//			chooses the best ledge to jump to based on where goal is
//			
//			searches the ledge list set up by ai_find_ledge_list
//			if a ledge's z is within +/- 24 units of the goal's z
//			then that ledge is chosen, otherwise, a ledge with z within +/- 24
//			units of self's z is chosen
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define	LEDGE_ABOVE	32
#define	LEDGE_BELOW	256
#define	LEDGE_INC	32

int	ai_ChooseLedge( userEntity_t *self, CVector &goal_org, float start_dist )
{
	_ASSERTE( self );

	int			chose_second_ledge, found_ledge;
	float		goal_z, self_z, start_z, ledge_z;
	float		goal_dist, ledge_dist;
	float		tempLedgeDist, check_dist;
	CVector		org;

	start_z = self->s.origin.z + LEDGE_ABOVE - 24;  // subtract 24 to account for distance from bot's origin to floor
	goal_z = goal_org.z - 24;
	self_z = self->s.origin.z - 24;
	
	// find dist in xy plane
	goal_dist = VectorXYDistance( self->s.origin, goal_org );
	
	chose_second_ledge = FALSE;
	ledge_dist = 8192.0;
	found_ledge = ledge_list.num_ledges + 1;

	for ( int i = 0; i <= ledge_list.num_ledges; i++ )
	{
		ledge_z = start_z - (float) ledge_list.ledge_height [i];

		// is this ledge near to the goal?
		if ( fabs(ledge_z - goal_z) <= 24.0 )
		{
			// record this ledge's xy and z distance
			tempLedgeDist = start_dist + i * LEDGE_INC;
			check_dist = fabs( tempLedgeDist - goal_dist );

			if ( check_dist < ledge_dist && check_dist > 16 )
			{
				//	this is the best ledge so far based on xy 
				//	and z distances, so record it
				ledge_data.ledge_z_dist = ledge_z - self_z;	
				ledge_data.ledge_xy_dist = tempLedgeDist;
				ledge_dist = tempLedgeDist;
				
				found_ledge = i;
			}
		}

		if ( fabs(ledge_z - self_z) <= 24.0 && !chose_second_ledge )
		{
			ledge_data.ledge_z_dist = ledge_z - self_z;
			ledge_data.ledge_xy_dist = start_dist + i * LEDGE_INC;
			chose_second_ledge = TRUE;
		}
	}

#ifdef _DEBUG
	if ( self->flags & FL_DEBUG )
	{
		if (found_ledge <= ledge_list.num_ledges || chose_second_ledge)
		{
			VectorMA( self->s.origin, forward, ledge_data.ledge_xy_dist, org );
			org.AddZ( ledge_data.ledge_z_dist );
	
			com->SpawnMarker( self, org, debug_models [DEBUG_JUMPTARG], 2.0 );
		}
	}
#endif
	if ( found_ledge <= ledge_list.num_ledges )
	{
		return	TRUE;
	}
	else 
	if ( chose_second_ledge )
	{
		return	TRUE;
	}

	return	FALSE;
}


int AI_CheckBlockingDirection( userEntity_t *self, CVector &destPoint )
{
	_ASSERTE( self );

	int nEntityBlockedSide = 0;		// 0 => no side, 0x00000001 => left side blocked, 0x00000002 => right side blocked

	float fWidth = self->s.maxs.x - self->s.mins.x;

	CVector directionVector;
	directionVector.x = destPoint.x - self->s.origin.x;
	directionVector.y = destPoint.y - self->s.origin.y;
	directionVector.z = 0.0f;
	directionVector.Normalize();

	//
	//	 \		  |		   /
	//	   \	  |		 /
	//		 \	  |	   /
	//		   \  |	 /
	//			 \ /
	
	// check right side
	CVector rightVector( directionVector );
	RotateVector2D( rightVector, 315.0f );
	
	CVector endPoint;
	VectorMA( self->s.origin, rightVector, fWidth, endPoint );
	tr = gstate->TraceLine_q2( self->s.origin, endPoint, self, MASK_SOLID | CONTENTS_MONSTER );
	if ( tr.fraction < 1.0f )
	{
		if ( tr.ent && (tr.ent->flags & (FL_MONSTER|FL_CLIENT|FL_BOT)) )
		{
			// blocked right by another entity
			nEntityBlockedSide |= 0x00000002;
		}
		else
		{
			return TER_WALL_RIGHT;
		}
	}

	// check left side
    CVector leftVector;
    AI_LeftVector( rightVector, leftVector );

	VectorMA( self->s.origin, leftVector, fWidth, endPoint );
	tr = gstate->TraceLine_q2( self->s.origin, endPoint, self, MASK_SOLID | CONTENTS_MONSTER );
	if ( tr.fraction < 1.0f )
	{
		if ( tr.ent && (tr.ent->flags & (FL_MONSTER|FL_CLIENT|FL_BOT)) )
		{
			// blocked left by another entity
			nEntityBlockedSide |= 0x00000001;
		}
		else
		{
			return TER_WALL_LEFT;
		}
	}

	if ( nEntityBlockedSide )
	{
		if ( (nEntityBlockedSide & 0x00000001) && (nEntityBlockedSide & 0x00000002) )
		{
			// both sides are blocked by entities
			return BLOCKED_BOTH_BY_ENTITY;
		}
		else
		if ( nEntityBlockedSide & 0x00000001 )
		{
			return BLOCKED_LEFT_BY_ENTITY;
		}
		else
		if ( nEntityBlockedSide & 0x00000002 )
		{
			return BLOCKED_RIGHT_BY_ENTITY;
		}
	}

	return TER_CLEAR;
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		AI_CheckBlockingDirectionFlyUnits
// Description: This will check above below and left & right
// Input:userEntity_t *self, CVector &destPoint
// Output:int value
// Note: This code is still in testing.
//
// ---------------------------------------------------------------------------
int AI_CheckBlockingDirectionFlyUnits( userEntity_t *self, CVector &destPoint )
{
	_ASSERTE( self );

	CVector Dir,target_angles,delta_angles,delta;
	//Let's create a distance for both our width and hieght
	//Also let's add a little bit of "extra" width in there to make sure we don't fucking hit something
	
	float fWidthx = (self->s.maxs.x*1.15) - (self->s.mins.x*1.15);
	float fWidthz = (self->s.maxs.z*1.15) - (self->s.mins.z*1.15);
	
	// 0 => no side, 0x00000001 => left side blocked, 0x00000002 => right side blocked
	// 0x00000004 => above is blocked,0x00000008 => Below is blocked
	
	int nEntityBlockedSide = 0;		

	
	//Get the initial direction of travel
	Dir = destPoint - self->s.origin;
	
	Dir.Normalize();
	
	
	//Get the angles from the direction of travel
	VectorToAngles(Dir,target_angles);

	delta_angles = target_angles;
	//Let's create a vector to the right
	delta_angles.yaw += 90;

	
	delta_angles.AngleToForwardVector(delta);
	
	CVector endPoint;
	VectorMA( self->s.origin, delta, fWidthx, endPoint );
	tr = gstate->TraceLine_q2( self->s.origin, endPoint, self, MASK_SOLID | CONTENTS_MONSTER );
	if ( tr.fraction < 1.0f )
	{
		if ( tr.ent && (tr.ent->flags & (FL_MONSTER|FL_CLIENT|FL_BOT)) )
		{
			// blocked right by another entity
			nEntityBlockedSide |= 0x00000002;
		}
		else
		{
			return TER_WALL_RIGHT;
		}
	}

	delta_angles = target_angles;
	//Let's create a vector to the left
	delta_angles.yaw += 90;
	
	delta_angles.AngleToForwardVector(delta);

	VectorMA( self->s.origin, delta, fWidthx, endPoint );
	tr = gstate->TraceLine_q2( self->s.origin, endPoint, self, MASK_SOLID | CONTENTS_MONSTER );
	if ( tr.fraction < 1.0f )
	{
		if ( tr.ent && (tr.ent->flags & (FL_MONSTER|FL_CLIENT|FL_BOT)) )
		{
			// blocked left by another entity
			nEntityBlockedSide |= 0x00000001;
		}
		else
		{
			return TER_WALL_LEFT;
		}
	}

	delta_angles = target_angles;
	//Let's create a vector above us
    // ISP: 10-12-99 let's check straight up which is 90 degrees
	delta_angles.pitch = -90;
	
	delta_angles.AngleToForwardVector(delta);

	VectorMA( self->s.origin, delta, fWidthz, endPoint );
	tr = gstate->TraceLine_q2( self->s.origin, endPoint, self, MASK_SOLID | CONTENTS_MONSTER );
	if ( tr.fraction < 1.0f )
	{
		if ( tr.ent && (tr.ent->flags & (FL_MONSTER|FL_CLIENT|FL_BOT)) )
		{
			// blocked left by another entity
			nEntityBlockedSide |= 0x00000004;
		}
		else
		{
			return TER_HIGH_OBS;
		}
	}


	delta_angles = target_angles;
	//Let's create a vector Below us
    // ISP: 10-12-99 let's check straight down which is 90 degrees
	delta_angles.pitch = 90;
	
	delta_angles.AngleToForwardVector(delta);

	VectorMA( self->s.origin, delta, fWidthz, endPoint );
	tr = gstate->TraceLine_q2( self->s.origin, endPoint, self, MASK_SOLID | CONTENTS_MONSTER );
	if ( tr.fraction < 1.0f )
	{
		if ( tr.ent && (tr.ent->flags & (FL_MONSTER|FL_CLIENT|FL_BOT)) )
		{
			// blocked left by another entity
			nEntityBlockedSide |= 0x00000008;
		}
		else
		{
			return TER_LOW_OBS;
		}
	}
	
	if ( nEntityBlockedSide )
	{
		if ( (nEntityBlockedSide & 0x00000001) && (nEntityBlockedSide & 0x00000002) && (nEntityBlockedSide & 0x00000004) && (nEntityBlockedSide & 0x00000008))
		{
			// both sides are blocked by entities
			return BLOCKED_BOTH_BY_ENTITY;
		}
		else
		if ( nEntityBlockedSide & 0x00000001 )
		{
			return BLOCKED_LEFT_BY_ENTITY;
		}
		else
		if ( nEntityBlockedSide & 0x00000002 )
		{
			return BLOCKED_RIGHT_BY_ENTITY;
		}
		else
		if ( nEntityBlockedSide & 0x00000004 )
		{
			return BLOCKED_ABOVE_BY_ENTITY;
		}
		else
		if ( nEntityBlockedSide & 0x00000008 )
		{
			return BLOCKED_BELOW_BY_ENTITY;
		}

	}

	return TER_CLEAR;
}



// ----------------------------------------------------------------------------
//
// Name:		AI_IsGap
// Description:
// Input:
// Output:
//				TRUE	=> if found a gap
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsGap( userEntity_t *self, CVector &startPoint, CVector &direction, float fSpeed )
{
	_ASSERTE( self );

	CVector nextPoint;
	VectorMA( self->s.origin, direction, fSpeed, nextPoint );
	if ( !AI_IsGroundBelowBetween( self, self->s.origin, nextPoint ) )
	{
		return TRUE;
	}
	return FALSE;
}

// ----------------------------------------------------------------------------
// NSS[2/14/00]:
// Name:		AI_IsGap
// Description:
// Input:
// Output:
//				TRUE	=> if found a gap
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsGap( userEntity_t *self, CVector &destPoint, float fDistance )
{
	_ASSERTE( self );

	CVector vector = destPoint - self->s.origin;
	vector.Normalize();
	CVector nextPoint;
	// NSS[2/14/00]:This Mins, Maxs check is to make sure that we are traversing
	// a distance greater than that of our bounding box's distance, if we are not,
	// then let's change the distance to be equal to that of 1/2 our bounding box's distance.
	// this was causing a problem when AI would slow down and would find a gap like 1/10 the distance
	// of their bounding box in a floor or something.  Removing this check will cause ground units to get
	// stuck where there are small 'gaps' in the floor.
	CVector Mins,Maxs;
	float mDistance,kDistance,BoundingDistance,MaxDistance;
	Mins = self->s.mins;
	Maxs = self->s.maxs;
	
	kDistance			= VectorXYDistance(self->s.origin, destPoint);
	BoundingDistance	= VectorXYDistance(Mins,Maxs);
	MaxDistance			= BoundingDistance;
	
	BoundingDistance	*= 0.25;
	mDistance			= fDistance;
	
	if(kDistance < mDistance)
	{
		nextPoint = self->s.origin + (vector * mDistance);
		if ( AI_IsGroundBelowBetween( self, self->s.origin, nextPoint ) )
		{
			return FALSE;
		}
	}
	while(kDistance > mDistance)
	{
		nextPoint = self->s.origin + (vector * mDistance);
		if ( AI_IsGroundBelowBetween( self, self->s.origin, nextPoint ) )
		{
			return FALSE;
		}
		mDistance += BoundingDistance;
		if(MaxDistance < mDistance)
		{
			return TRUE;
		}
	}
	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_HandleGap
// Description:
// Input:
// Output:
//				TRUE	=> if found gap	and handled
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int AI_HandleGap( userEntity_t *self, CVector &destPoint, float fSpeed, float fDistance, float fZDistance )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	if ( AI_IsGap( self, destPoint, fDistance ) )
	{
//		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
//		if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) != TASKTYPE_GOINGAROUNDOBSTACLE )
//		{
//			AI_FindPathToPoint( self, destPoint );
//			if ( hook->pPathList->pPath )
//			{
//				AI_AddNewTaskAtFront( self, TASKTYPE_GOINGAROUNDOBSTACLE, destPoint );
//			}
//			else
//			{
//				// did not find another ledge to jump to, basically can not reach the destination,
//				// give up current goal or task
//				AI_RemoveCurrentGoal( self );
//			}
//		}

		return TRUE;
	}
	return FALSE;

/*	
	int bGapFound = FALSE;

	//-------------------------------------------------------------------------
	// Handle ledge detection
	//-------------------------------------------------------------------------
	if ( AI_IsGap( self, destPoint, fDistance ) )
	{
		// stop all motion
		AI_ZeroVelocity( self );
		hook->last_origin.Set( self->s.origin );

		if ( hook->fnStartJumpForward )
		{
			float dist = fSpeed * 0.1f;
			if ( self->velocity.Length() > 0 )
			{
				dist = fSpeed * 0.1f;
			}
			if ( dist < 32.0f )
			{
				dist = 32.0f;
			}

			if ( ai_find_ledge_list(self, dist) )
			{
				// choose ledge
				int i = 0;
				//	FIXME:	added self->goalentity check for demo hack
				GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
				GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );

				if ( GOAL_GetType( pCurrentGoal ) == GOALTYPE_FOLLOW )
				{
					i = ai_ChooseLedge(self, hook->owner->s.origin, 64);
				}
				else 
				if (self->goalentity)
				{
					i = ai_ChooseLedge(self, destPoint, 64);
				}

				if ( i && !ai_jump_obstructed(self, (float) i, hook->upward_vel) )
				{
					hook->forward_vel = ai_jump_vel(self, ledge_data.ledge_xy_dist, ledge_data.ledge_z_dist, hook->upward_vel);
					AI_AddNewTaskAtFront( self, TASKTYPE_JUMPFORWARD );

					bGapFound = TRUE;
				}
			}

			if ( !bGapFound )
			{
				// did not find a ledge to jump to, test if the AI can jump down
				if ( (destPoint.z < self->s.origin.z) && fZDistance < 512.0f )
				{
					tr = gstate->TraceLine_q2( self->s.origin, destPoint, self, MASK_SOLID );
					if ( tr.fraction >= 1.0f )
					{
						hook->forward_vel = ai_jump_vel(self, fDistance, fZDistance, hook->upward_vel);
						AI_AddNewTaskAtFront( self, TASKTYPE_JUMPFORWARD );

						bGapFound = TRUE;
					}
				}
			}
		}

		if ( !bGapFound )
		{
			GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
			if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) != TASKTYPE_GOINGAROUNDOBSTACLE )
			{
				AI_FindPathToPoint( self, destPoint );
				if ( hook->pPathList->pPath )
				{
					AI_AddNewTaskAtFront( self, TASKTYPE_GOINGAROUNDOBSTACLE, destPoint );
				}
				else
				{
					// did not find another ledge to jump to, basically can not reach the destination,
					// give up current goal or task
					AI_RemoveCurrentGoal( self );
				}

				AI_ZeroVelocity( self );

				bGapFound = TRUE;
			}
		}
	}

	return bGapFound;
*/
}	



int AI_4CastTrace(CVector &Origin, float x, float z, CVector &Destination,userEntity_t *self, unsigned long Mask);
int AI_HandleWallMoveAwayPoints(userEntity_t *self, CVector Dir);
// ----------------------------------------------------------------------------
//
// Name:		AI_HandleGroundObstacle
// Description:
// Input:
// Output:
//				TRUE	=> if found obstacle and handled
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int AI_HandleGroundObstacle( userEntity_t *self, float fSpeed, int bMovingTowardNode )
{
	float dist = fSpeed * 0.1f;
	if ( self->velocity.Length() > 0 )
	{
		dist = fSpeed * 0.125f;
	}
	int nWallType = ai_terrain_type( self, forward, dist ); 
	int COLLISION = FALSE;
	switch ( nWallType )
	{
		case TER_OBSTRUCTED:
		{
			AI_Dprintf( "Strafing along the wall.\n" );

			CVector dir;
			self->s.angles.AngleToForwardVector( dir );
			AI_FindNewCourse(self, &dir);
			//AI_ComputeAlongWallVector( self, forward, terrain.wall_normal, dir );
			forward = dir;
			COLLISION = TRUE;
			break;
		}
		case TER_WALL:
		case TER_WALL_PARTIAL:
		{
			AI_Dprintf( "Wall in front.\n" );
			
			CVector dir;
			self->s.angles.AngleToForwardVector( dir );
			AI_FindNewCourse(self, &dir);
			//AI_ComputeAlongWallVector( self, forward, terrain.wall_normal, dir );
			forward = dir;
			COLLISION = TRUE;
			break;
		}
		case TER_HIGH_OBS:
		{
			// duck under
			break;
		}
		case TER_WALL_RIGHT:
		{
			AI_Dprintf( "Right side Blocked.\n" );

			// right side is blocked
			// rotate the wall normal to the left so that we can move around this obstacle
			CVector dir;	
			AI_LeftVector(forward,dir);
			//AI_ComputeAlongWallVector( self, forward, terrain.wall_normal, dir );
			dir.Normalize();
			//if(!AI_HandleWallMoveAwayPoints(self,dir))
			//{
				AI_SetVelocity(self,dir,128.0f);
			//}
			COLLISION = TRUE;
			break;
		}
		case TER_WALL_LEFT:
		{
			AI_Dprintf( "Left side Blocked.\n" );
			
			// left side is blocked
			// rotate the wall normal to the right so that we can move around this obstacle
			CVector dir;
			AI_RightVector(forward,dir);
			//AI_ComputeAlongWallVector( self, forward, terrain.wall_normal, dir );
			dir.Normalize();
			//if(!AI_HandleWallMoveAwayPoints(self,dir))
			//{
			AI_SetVelocity(self,dir,128.0f);
			//}
			COLLISION = TRUE;
			break;
		}		
		case TER_LOW_OBS:
		{
			AI_Dprintf( "Detected low obstruction.\n" );
            
			if ( bMovingTowardNode )
            {
                // jump up and over
			    AI_AddNewTaskAtFront( self, TASKTYPE_JUMPUP );
            }
            else
            {
			    CVector dir;
				self->s.angles.AngleToForwardVector( dir );
				AI_FindNewCourse(self, &dir);
				//AI_ComputeAlongWallVector( self, forward, terrain.wall_normal, dir );
			    //forward = dir;
				//COLLISION = TRUE;
            }

			break;
		}

		default:
		{
			break;
		}
	}

	return COLLISION;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_HandleOffGround
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_HandleOffGround( userEntity_t *self, CVector &destPoint, float fSpeed )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	// forward is direction to goal, but entity should not face it
	forward = destPoint - self->s.origin;
	forward.Normalize();

	float dist = fSpeed * 0.125f;
	if ( dist < 32.0f )
	{
		dist = 32.0f;
	}
	int nWallType = ai_terrain_type( self, forward, dist );
	switch ( nWallType )
	{
		case TER_OBSTRUCTED:
		{
			CVector dir;
			AI_ComputeAlongWallVector( self, forward, terrain.wall_normal, dir );
			forward = dir;
			forward.z = -1.0f;
			forward.Normalize();
			break;
		}
		case TER_WALL:
		case TER_WALL_PARTIAL:
		{
			CVector dir;
			AI_ComputeAlongWallVector( self, forward, terrain.wall_normal, dir );
			forward = dir;
			forward.z = -1.0f;
			forward.Normalize();
			break;
		}
		
		case TER_HIGH_OBS:
		{
			CVector dir;
			AI_ComputeAlongWallVector( self, forward, terrain.wall_normal, dir );
			forward = dir;
			forward.z = -1.0f;
			forward.Normalize();
			break;
		}
		case TER_WALL_RIGHT:
		{
			// right side is blocked
			// rotate the wall normal to the left so that we can move around this obstacle
			CVector dir( -forward.y, forward.x, 0.0f );	
			dir.Normalize();
			forward = dir;
			forward.z = -1.0f;
			forward.Normalize();
			break;
		}
		case TER_WALL_LEFT:
		{
			// left side is blocked
			// rotate the wall normal to the right so that we can move around this obstacle
			CVector dir( forward.y, -forward.x, 0.0f );
			dir.Normalize();
			forward = dir;
			forward.z = -1.0f;
			forward.Normalize();
			break;
		}		

		default:
		{
			break;
		}
	}

	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	AI_SetVelocity( self, forward, (fSpeed * fScale) );
//	if ( !AI_IsGroundBelowBetween( self, self->s.origin, destPoint ) )
//	{
		self->velocity.z -= 80.0f;
//	}
	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_HandleDirectPath
// Description:
// Input:
// Output:
//				TRUE	=> if no direct path and handled
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int AI_HandleDirectPath( userEntity_t *self, CVector &destPoint, int bMovingTowardNode )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !bMovingTowardNode && !AI_IsDirectPath( self, destPoint ) )
	{
		AI_FindPathToPoint( self, destPoint );
		if ( hook->pPathList->pPath )
		{
			AI_AddNewTaskAtFront( self, TASKTYPE_GOINGAROUNDOBSTACLE, destPoint );
		}
		else
		{
			if ( AI_CanPath( hook ) )
			{
				AI_RemoveCurrentGoal( self );
			}
		}

		return TRUE;
	}

	return FALSE;
}


// ----------------------------------------------------------------------------
// NSS[2/6/00]:
// Name:		AI_HandleCollisionWithEntities
// Description:
// Input:
// Output:
//				TRUE	=> if collision with entity and handled
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int AI_HandleCollisionWithEntities( userEntity_t *self, CVector &destPoint, float fSpeed )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	float fHalfWidth = fabs(self->s.maxs.x - self->s.mins.x) * 0.5f;

	// check collision with other entities
	float fOneFrameSpeed = (fSpeed * 0.125f);

	CVector checkPoint;
	CVector Dir;
	
	
	//VectorMA( self->s.origin, forward, fOneFrameSpeed + fHalfWidth, checkPoint );


	Dir = forward*(fOneFrameSpeed+fHalfWidth);
	checkPoint = Dir + self->s.origin;
	

	CVector mins = self->s.mins;
    mins.z += 8.0f;
    CVector maxs = self->s.maxs;

	tr = gstate->TraceBox_q2( self->s.origin, mins, maxs, checkPoint, self, CONTENTS_MONSTER );
	if(tr.fraction < 1.0f)
	{
		if(tr.ent)
		{
			if(tr.ent != self)
			{
				if(self->enemy && !AI_IsSidekick(hook))
				{
					if(self->enemy == tr.ent)
					{
						//AI_RemoveCurrentTask(self,FALSE);
						return FALSE;
					}
				}
				if(AI_IsSidekick(hook))
				{
					if(hook->owner)
					{
						if(tr.ent == hook->owner )
						{
							AI_RemoveCurrentTask(self,FALSE);
						}
						else if (tr.ent->flags & FL_BOT)
						{
							return FALSE;
						}
					}
				}
				if(!AI_FindPathAroundEntity( self, tr.ent, destPoint ))
				{
					if(AI_IsSidekick(hook))
					{
						if(rnd() > 0.50f)
						{
							if(rnd() > 0.50)
							{
								AI_FindNewCourse( self , &Dir);
							}
							else
							{
								AI_AddNewTaskAtFront(self,TASKTYPE_SIDEKICK_STOP);
							}
						}
						else
						{
							AI_RestartCurrentGoal(self);
						}
					}
					else
					{
						AI_FindNewCourse( self , &Dir);
					}
					hook->skill = 0;
					return FALSE;
				}
				return TRUE;
			}
		}
	}
/*
    
	if ( tr.fraction < 1.0f )
	{
		if ( tr.ent && _stricmp( tr.ent->className, "worldspawn" ) != 0 )
		{
			if ( tr.ent->flags & (FL_MONSTER|FL_CLIENT|FL_BOT) )
			{
				userEntity_t *pCollidedEnt = tr.ent;
                
                if ( pCollidedEnt == self->enemy )
                {
                    return FALSE;
                }

                if ( pCollidedEnt->velocity.Length() > 75.0f && rnd() < 0.35)
                {
				    AI_AddNewTaskAtFront( self, TASKTYPE_WAITFORNOCOLLISION, destPoint );
                    return TRUE;
                }
                else
                {
                    // see if the collided entity is on top of the destPoint
                    float fCollidedEntDistToDestPoint = VectorXYDistance( pCollidedEnt->s.origin, destPoint );
				    float fCollidedEntZDistance = VectorZDistance( pCollidedEnt->s.origin, destPoint );
				    if ( (fCollidedEntDistToDestPoint <= (fHalfWidth *3.0f) && fCollidedEntZDistance < 32.0f) )
                    {
			            GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
			            TASKTYPE nTaskType = GOALSTACK_GetCurrentTaskType( pGoalStack );

                        // if the current destination is the next point in the path list remove it
                        if ( nTaskType == TASKTYPE_PATHFOLLOW )
                        {
                            AI_AddNewTaskAtFront( self, TASKTYPE_WAITFORNOCOLLISION, destPoint );
                            return TRUE;
                        }
                        else
                        {
                            if ( hook->pPathList->pPath && hook->pPathList->pPath->nNodeIndex >= 0 )
                            {
								if(AI_IsInSmallSpace(self) || hook->skill > 1)    //Check to see if we are in a small space //NSS[12/1/99]:
								{
									AI_FindNewCourse( self , &Dir);  //If in a small corridor or the like let's just turn around and try going out the way we came in. //NSS[12/1/99]:
									hook->skill = 0;
								}
								else
								{
									//NSS[12/1/99]:semi-hack, but since we are no longer using the skill var in hook.. what the hey..
									hook->skill++;
									MAPNODE_PTR pNode = NODE_GetNode( pGroundNodes, hook->pPathList->pPath->nNodeIndex );
									float fDistance = VectorDistance( destPoint, pNode->position );
									if ( fDistance < 32.0f )
									{
										PATHLIST_DeleteFirstInPath( hook->pPathList );
										AI_FindNewCourse( self , &Dir);
										hook->skill = 0;
										return TRUE;
//										return AI_FindPathAroundEntity( self, pCollidedEnt, destPoint );
										//return TRUE;
									}
								}
                            }
                        }
                    }

			        GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
			        TASKTYPE nTaskType = GOALSTACK_GetCurrentTaskType( pGoalStack );
			        if ( nTaskType == TASKTYPE_RANDOMWANDER || 
                         nTaskType == TASKTYPE_PRISONER_WANDER ||
                         nTaskType == TASKTYPE_SIDEKICK_RANDOMWALK )
			        {
                        AI_RemoveCurrentTask( self );
                        return TRUE;
                    }
                    
					if(AI_IsInSmallSpace(self))    //Check to see if we are in a small space //NSS[12/1/99]:
					{
						AI_FindNewCourse( self , &Dir);  //If in a small corridor or the like let's just turn around and try going out the way we came in. //NSS[12/1/99]:
						hook->skill = 0;
						return TRUE;
					}
					else
					{
						AI_FindNewCourse( self , &Dir);
						hook->skill = 0;
						return TRUE;
						//return AI_FindPathAroundEntity( self, pCollidedEnt, destPoint );
					}
			    }
            }
			else
			if ( _stricmp( tr.ent->className, "func_door" ) == 0 || _stricmp( tr.ent->className, "func_door_rotate" )==0)
			{
			
			    tr = gstate->TraceLine_q2(self->s.origin,checkPoint,self, MASK_SOLID);
				if(tr.fraction < 1.0f)
				{
					if(AI_IsSidekick(hook) && !SIDEKICK_HasTaskInQue(self,TASKTYPE_SIDEKICK_ATTACK))
					{
						AI_AddNewTaskAtFront( self, TASKTYPE_WAIT, 1.0f );
					}
					else
					{
						AI_FindNewCourse( self , &Dir);
					}
				}
				else
				{
					return FALSE;
				}
			}
			else
			if ( _stricmp( tr.ent->className, "func_plat" ) == 0 )
			{
				// collided with door, just wait a bit
				AI_AddNewTaskAtFront( self, TASKTYPE_WAIT, 1.0f );
			}
			else
			{
				// must have collided with other obstacles, try to go around
				
				CVector dir;
				AI_ComputeAlongWallVector( self, forward, tr.plane.normal, dir );
				forward = dir;
				forward.Normalize();

				if ( forward.Length() == 0.0f )
                {
                    forward = destPoint - self->s.origin;
                    forward.Normalize();
                }
                // make sure there is no gap in the direction where we are moving to
                if ( AI_IsGap( self, self->s.origin, forward, fOneFrameSpeed ) )
                {
                    AI_FindNewCourse( self , &Dir);  //If in a small corridor or the like let's just turn around and try going out the way we came in. //NSS[12/1/99]:
					hook->skill = 0;
					//AI_StopEntity( self );
                    return FALSE;
                }

                float fScale = FRAMES_ComputeFrameScale( hook );
				
				AI_SetVelocity( self, forward, (fSpeed * fScale) );
            }

			return TRUE;
		}
	}*/

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_HandleGettingStuck
// Description:
// Input:
// Output:
//				TRUE	=> if detected a stuck condition and handled
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int AI_HandleGettingStuck( userEntity_t *self, CVector &destPoint, float fSpeed )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	float fScale = FRAMES_ComputeFrameScale( hook );
	
	// check collision with other entities
	float fOneFrameSpeed = fSpeed * 0.125f;
	CVector checkPoint;
	VectorMA( self->s.origin, forward, fOneFrameSpeed, checkPoint );

	CVector org = self->s.origin;
	float fMoveDistance = hook->last_origin.Distance(org);
	if ( fMoveDistance < (self->velocity.Length() * 0.025f) )
	{
        int bOnFlatSurface = AI_IsOnFlatSurface( self );
		if ( !bOnFlatSurface )
		{
			// if not on a flat surface try to move slightly to left or right
			// to free itself
			CVector vectorTowardPoint;
			CVector angleTowardPoint;
			vectorTowardPoint = destPoint - self->s.origin;
			vectorTowardPoint.Normalize();
			
			VectorToAngles( vectorTowardPoint, angleTowardPoint );
			if ( rnd() > 0.5f )
			{
				angleTowardPoint.yaw += 45.0f;
			}
			else
			{
				angleTowardPoint.yaw -= 45.0f;
			}
			
			angleTowardPoint.y = AngleMod( angleTowardPoint.y );
            angleTowardPoint.z = 10.0f;
			
			CVector facingDir, right, up;
			angleTowardPoint.AngleToVectors( facingDir, right, up );
			facingDir.Normalize();

			forward = facingDir;

            AI_SetVelocity( self, forward, (fSpeed * fScale * 0.5f) );

			hook->last_origin = self->s.origin;

			if (!AI_IsFlyingUnit( self ) && !AI_IsInWater( self ))
			{
				AI_UpdateCurrentNode( self );
			}

			return FALSE;
		}

		// check blocks on the sides
		int nBlockDirection = AI_CheckBlockingDirection( self, checkPoint );
		switch ( nBlockDirection )
		{
			case BLOCKED_BOTH_BY_ENTITY:
			{
				if ( tr.ent )
				{
			        GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
			        TASKTYPE nTaskType = GOALSTACK_GetCurrentTaskType( pGoalStack );
			        if ( nTaskType == TASKTYPE_RANDOMWANDER || 
                         nTaskType == TASKTYPE_PRISONER_WANDER ||
                         nTaskType == TASKTYPE_SIDEKICK_RANDOMWALK )
			        {
                        AI_RemoveCurrentTask( self );
                    }
                    else
                    {
    					AI_GoAroundObstacle( self );
                    }

					return TRUE;
				}
				break;
			}

			case BLOCKED_LEFT_BY_ENTITY:
			case TER_WALL_LEFT:
			{
				CVector dir( forward.y, -forward.x, 0.0f );
				dir.Normalize();
				forward = dir;
				
				if ( AI_IsGap( self, self->s.origin, forward, fOneFrameSpeed ) )
                {
                    AI_StopEntity( self );
                }
                else
                {
                    AI_SetVelocity( self, forward, (fSpeed * fScale) );
                }
				break;
			}
			case BLOCKED_RIGHT_BY_ENTITY:
			case TER_WALL_RIGHT:
			{
				CVector dir( -forward.y, forward.x, 0.0f );
				dir.Normalize();
				forward = dir;

				if ( AI_IsGap( self, self->s.origin, forward, fOneFrameSpeed ) )
                {
                    AI_StopEntity( self );
                }
                else
                {
                    AI_SetVelocity( self, forward, (fSpeed * fScale) );
                }
				break;
			}
			case TER_CLEAR:
			{
				break;
			}
		}

		hook->nWaitCounter++;
		if ( (hook->nWaitCounter > 2 && fMoveDistance < 1.0f) || hook->nWaitCounter > 20 )
		{
			GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
			TASKTYPE nTaskType = GOALSTACK_GetCurrentTaskType( pGoalStack );
			if ( nTaskType == TASKTYPE_RANDOMWANDER || 
                 nTaskType == TASKTYPE_PRISONER_WANDER ||
                 nTaskType == TASKTYPE_SIDEKICK_RANDOMWALK )
			{
				AI_RemoveCurrentTask( self );
    			hook->nWaitCounter = 0;
				return TRUE;
			}
			else
			if ( nTaskType != TASKTYPE_GOINGAROUNDOBSTACLE )
			{
				AI_FindPathToPoint( self, destPoint );
				if ( hook->pPathList->pPath )
				{
					AI_AddNewTaskAtFront( self, TASKTYPE_GOINGAROUNDOBSTACLE, destPoint );
				}
				else
				{
					AI_RemoveCurrentTask( self );
				}
				hook->nWaitCounter = 0;
                return TRUE;
			}
			else
			{
				AI_RemoveCurrentTask( self );

				NODELIST_PTR pNodeList = hook->pNodeList;
				_ASSERTE( pNodeList );
				NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
				_ASSERTE( pNodeHeader );
				MAPNODE_PTR pClosestNode = NODE_GetClosestNode(self);
				if ( !pClosestNode )
				{
					if ( pNodeList->nCurrentNodeIndex != -1 )
					{
						pClosestNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
					}
				}
				
				if ( pClosestNode )
				{
					AI_Dprintf("Adding MOVETOLOCATION from handle getting stuck.\n");
					AI_AddNewTaskAtFront( self, TASKTYPE_MOVETOLOCATION, pClosestNode->position );
				}

				hook->nWaitCounter = 0;
                return TRUE;
			}
		}
	} 
	else
	{
		// moved correctly
		hook->nWaitCounter = 0;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_HopTowardPoint
// Description:
//				hop straight toward a point
// Input:
// Output:
//				TRUE	=> if reached the point
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int AI_HopTowardPoint( userEntity_t *self, CVector &destPoint, int bMovingTowardNode )
{
	_ASSERTE( self );

	// if this unit is going outside of its bounding box, then terminate current goal
	if ( AI_IsInsideBoundingBox( self ) == FALSE )
	{
		AI_RemoveCurrentGoal( self );
		return FALSE;
	}

	playerHook_t *hook = AI_GetPlayerHook( self );
	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return FALSE;
	}
	
	float fXYDistance = VectorXYDistance( self->s.origin, destPoint );
	float fZDistance = VectorZDistance( self->s.origin, destPoint );
	if ( AI_IsCloseDistance2( self, fXYDistance ) && fZDistance < 32.0f )
	{
		return TRUE;
	}

	AI_FaceTowardPoint( self, destPoint );

	float fSpeed = AI_ComputeMovingSpeed( hook );

	if ( !self->groundEntity && self->waterlevel < 3 )
	{
		if ( AI_IsCloseDistance2( self, fXYDistance ) )
		{
			return TRUE;
		}

		AI_HandleOffGround( self, destPoint, fSpeed );

		return FALSE;
	}

	if ( AI_HandleDirectPath( self, destPoint, bMovingTowardNode ) )
	{
		return FALSE;
	}

	forward = destPoint - self->s.origin;
	forward.Normalize();

	if ( AI_HandleCollisionWithEntities( self, destPoint, fSpeed ) )
	{
		return FALSE;
	}

	if ( AI_HandleGroundObstacle( self, fSpeed, bMovingTowardNode ) )
	{
		return FALSE;
	}

	if ( !bMovingTowardNode && AI_IsGap( self, self->s.origin, forward, fSpeed * 0.125f ) )
    {
        AI_StopEntity( self );
        return FALSE;
    }

    //	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	AI_SetVelocity( self, forward, (fSpeed * fScale) );

	if ( AI_HandleGettingStuck( self, destPoint, fSpeed ) )
	{
		return FALSE;
	}

	hook->last_origin = self->s.origin;

	AI_UpdateCurrentNode( self );

	ai_frame_sounds( self );
	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_TrackMoveTowardPoint
// Description:
//				move straight toward a point
// Input:
// Output:
//				TRUE	=> if reached the point
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int AI_TrackMoveTowardPoint( userEntity_t *self, CVector &destPoint )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	float fXYDistance = VectorXYDistance( self->s.origin, destPoint );
	float fZDistance = VectorZDistance( self->s.origin, destPoint );
	if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
	{
		return TRUE;
	}

	forward = destPoint - self->s.origin;
	forward.Normalize();

	float fSpeed = hook->attack_speed;
	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	AI_SetVelocity( self, forward, (fSpeed * fScale) );

	hook->last_origin = self->s.origin;

	ai_frame_sounds( self );
	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_OneLastStep
// Description:
//				take the last step
// Input:
// Output:
//				TRUE	=> if reached the point
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int AI_OneLastStep( userEntity_t *self, CVector &destPoint, float fSpeed,
					int bFaceTowardDest /* = TRUE */ )
{
	_ASSERTE( self );
	
	// if this unit is going outside of its bounding box, then terminate current goal
	if ( AI_IsInsideBoundingBox( self ) == FALSE )
	{
		AI_RemoveCurrentGoal( self );
		return FALSE;
	}

	playerHook_t *hook = AI_GetPlayerHook( self );
	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return FALSE;
	}
	
	// we can just check the x,y distance since this function assumes that the 
	// dest point is visible from self
	float fXYDistance = VectorXYDistance( self->s.origin, destPoint );
	float fZDistance = VectorZDistance( self->s.origin, destPoint );
	if ( AI_IsExactDistance( hook, fXYDistance ) && fZDistance < 32.0f )
	{
		AI_UpdateCurrentNode( self );
        return TRUE;
	}

	if ( bFaceTowardDest )
	{
        AI_FaceTowardPoint( self, destPoint );
	}

    if ( !self->groundEntity )
    {
		if ( AI_IsExactDistance( hook, fXYDistance ) )
		{
			AI_UpdateCurrentNode( self );
            return TRUE;
		}

        if ( self->waterlevel > 0 && self->waterlevel < 3 )
	    {
            float fHeight = 8.0f + ((float)self->waterlevel * 8.0f);
            if ( fZDistance > fHeight  )
            {
                // must be stuck trying to go out of the water, try to jump
	            self->velocity.z = hook->upward_vel;
                self->groundEntity = NULL;
            }
            else
            {
                forward = destPoint - self->s.origin;
                forward.Normalize();

                //	scale speed based on current frame's move_scale
	            float fScale = FRAMES_ComputeFrameScale( hook );
	            AI_SetVelocity( self, forward, (fSpeed * fScale) );
            }

            AI_UpdateCurrentNode( self );

            return FALSE;
        }
        else
        {
            AI_HandleOffGround( self, destPoint, fSpeed );
        }

		return FALSE;
	}

    if ( self->groundEntity && self->waterlevel == 2 )
    {
        if ( fZDistance > 24.0f  )
        {
            // must be stuck trying to go out of the water, try to jump
	        self->velocity.z = hook->upward_vel;
            self->groundEntity = NULL;
            AI_UpdateCurrentNode( self );
            return FALSE;
        }
    }

	forward = destPoint - self->s.origin;

	// should never be in water, but just in case
	int bOnFlatSurface = AI_IsOnFlatSurface( self );
	if ( self->waterlevel < 3 && bOnFlatSurface && self->movetype != MOVETYPE_HOP )
	{
		forward.z = 0.0f;
	}
	forward.Normalize();

	if ( AI_HandleCollisionWithEntities( self, destPoint, fSpeed ) )
	{
		AI_UpdateCurrentNode( self );
        return FALSE;
	}

	if ( AI_HandleGroundObstacle( self, fSpeed, FALSE ) )
	{
		AI_UpdateCurrentNode( self );
        return FALSE;
	}

	float fOneFrameDistance = fSpeed * 0.125f;
    if ( AI_IsGap( self, destPoint, fOneFrameDistance ) )
	{
		AI_StopEntity( self );
        return FALSE;
	}

	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	AI_SetVelocity( self, forward, (fSpeed * fScale) );

	if ( AI_HandleGettingStuck( self, destPoint, fSpeed ) )
	{
		AI_UpdateCurrentNode( self );
        return FALSE;
	}

	hook->last_origin = self->s.origin;

	AI_UpdateCurrentNode( self );

    AI_HandleCrouching( self );

	ai_frame_sounds( self );
	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_MoveTowardPoint
// Description:
//				move straight toward a point
// Input:
// Output:
//				TRUE	=> if reached the point
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int AI_MoveTowardPoint( userEntity_t *self, CVector &destPoint, 
						int bMovingTowardNode /* = FALSE */, int bFaceTowardDest /* = TRUE */ )
{
	_ASSERTE( self );
	
	if ( AI_IsFlyingUnit( self ) )
	{
		return AI_FlyTowardPoint( self, destPoint, bMovingTowardNode );
	}

	if ( AI_IsInWater( self ) )
    {
        return AI_SwimTowardPoint( self, destPoint, bMovingTowardNode );
    }

    if ( self->movetype == MOVETYPE_HOP )
	{
		return AI_HopTowardPoint( self, destPoint, bMovingTowardNode );
	}

	// if this unit is going outside of its bounding box, then terminate current goal
	if ( AI_IsInsideBoundingBox( self ) == FALSE )
	{
		AI_RemoveCurrentGoal( self );
		return FALSE;
	}

	playerHook_t *hook = AI_GetPlayerHook( self );
	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return FALSE;
	}
	
	// we can just check the x,y distance since this function assumes that the 
	// dest point is visible from self
	float fXYDistance = VectorXYDistance( self->s.origin, destPoint );
	float fZDistance = VectorZDistance( self->s.origin, destPoint );
	if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
	{
		AI_UpdateCurrentNode( self );
        return TRUE;
	}

	if ( bFaceTowardDest )
	{
        AI_FaceTowardPoint( self, destPoint );
	}

	float fSpeed = AI_ComputeMovingSpeed( hook );

    if ( !self->groundEntity)
    {
		if ( AI_IsCloseDistance2(self, fXYDistance ) )
		{
			AI_UpdateCurrentNode( self );
            return TRUE;
		}

        if ( self->waterlevel > 0 && self->waterlevel < 3 )
	    {
            float fHeight = 8.0f + ((float)self->waterlevel * 8.0f);
            if ( fZDistance > fHeight  )
            {
                // must be stuck trying to go out of the water, try to jump
	            //self->velocity.z = hook->upward_vel;
                //self->groundEntity = NULL;
            }
            else
            {
                //forward = destPoint - self->s.origin;
                //forward.Normalize();

                //	scale speed based on current frame's move_scale
	            //float fScale = FRAMES_ComputeFrameScale( hook );
	            //AI_SetVelocity( self, forward, (fSpeed * fScale) );
            }  
			if(!(hook->dflags & DFL_AMPHIBIOUS))
			{
				AI_UpdateCurrentNode( self );
				return FALSE;
			}
        }
        else
        {
			AI_HandleOffGround( self, destPoint, fSpeed );
			return FALSE;
        }
	}

    if ( self->groundEntity && self->waterlevel == 2 && ! (hook->dflags & DFL_AMPHIBIOUS) )
    {
        if ( fZDistance > 24.0f  )
        {
            // must be stuck trying to go out of the water, try to jump
	        self->velocity.z = hook->upward_vel;
            self->groundEntity = NULL;
            AI_UpdateCurrentNode( self );
            return FALSE;
        }
    }

    userEntity_t *pAboveEntity = AI_IsEntityAbove( self );
    if ( pAboveEntity )
    {
        CVector freePoint;
        if ( AI_FindCollisionFreePoint( self, freePoint ) )
        {
            destPoint = freePoint;
            bMovingTowardNode = FALSE;
        }
    }

    userEntity_t *pBelowEntity = AI_IsEntityBelow( self );
    if ( pBelowEntity )
    {
        // since enemy is below, find a point respective of the enemy
        CVector freePoint;
        if ( AI_FindCollisionFreePoint( pBelowEntity, freePoint ) )
        {
        	forward = freePoint - self->s.origin;
            forward.Normalize();

	        float fDistance = VectorDistance( self->s.origin, freePoint );
	        float fZDistance = VectorZDistance( self->s.origin, freePoint );

	        // compute needed forward velocity to reach the point
	        float fUpwardVelocity = hook->upward_vel * 0.5f;
	        float fForwardVelocity = ai_jump_vel(self, fDistance, fZDistance, fUpwardVelocity);

	        if ( freePoint.z > self->s.origin.z )
	        {
		        fUpwardVelocity += fZDistance;
	        }
	        self->velocity = forward * fForwardVelocity;
	        self->velocity.z = fUpwardVelocity;

	        self->groundEntity = NULL;
            return FALSE;
        }
    }

	if ( AI_HandleDirectPath( self, destPoint, bMovingTowardNode ) )
	{
		AI_UpdateCurrentNode( self );
        return FALSE;
	}

	forward = destPoint - self->s.origin;

	if ( self->movetype == MOVETYPE_WHEEL )
	{
		if ( AI_WillItBeCaughtInLoop( self, destPoint ) == FALSE ) 
		{
			CVector angleTowardPoint;
			VectorToAngles( forward, angleTowardPoint );
			
			CVector facingAngle = self->s.angles;
			if ( AI_IsSameAngle2D( self, facingAngle, angleTowardPoint ) == FALSE )
			{
				CVector facingDir;
				YawToVector( facingAngle.y, facingDir );

				forward.Set( facingDir.x, facingDir.y, 0.0f );
			}
		}
		else
		{
			AI_UpdateCurrentNode( self );
            return FALSE;
		}
	}

	// should never be in water, but just in case
	int bOnFlatSurface = AI_IsOnFlatSurface( self );
	if ( self->waterlevel < 3 && bOnFlatSurface && self->movetype != MOVETYPE_HOP )
	{
		forward.z = 0.0f;
	}
	forward.Normalize();

	if ( AI_HandleCollisionWithEntities( self, destPoint, fSpeed ) )
	{
		AI_UpdateCurrentNode( self );
        return FALSE;
	}
	if ( AI_HandleGroundObstacle( self, fSpeed, bMovingTowardNode ) )
	{
		AI_UpdateCurrentNode( self );
        return FALSE;
	}
	//NSS[11/27/99]:
	//float fOneFrameDistance = fSpeed * 0.125f;
	float fOneFrameDistance = fSpeed * 0.125f;
    if ( !bMovingTowardNode && AI_IsGap( self, destPoint, fOneFrameDistance ) && pBelowEntity == NULL && !(self->flags & FL_CINEMATIC))
	{
		AI_StopEntity( self );
        return FALSE;
	}

	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	AI_SetVelocity( self, forward, (fSpeed * fScale) );

	if ( AI_HandleGettingStuck( self, destPoint, fSpeed ) )
	{
		AI_UpdateCurrentNode( self );
        return FALSE;
	}

	hook->last_origin = self->s.origin;

	AI_UpdateCurrentNode( self );

    AI_HandleCrouching( self );

	ai_frame_sounds( self );
	
	if(!bMovingTowardNode)
		return TRUE;
	else
		return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_MoveTowardExactPoint
// Description:
//				move straight toward a point
// Input:
// Output:
//				TRUE	=> if reached the point
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int AI_MoveTowardExactPoint( userEntity_t *self, CVector &destPoint, 
							 int bMovingTowardNode /* = FALSE */, int bFaceTowardDest /* = TRUE */ )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return FALSE;
	}
	
	// we can just check the x,y distance since this function assumes that the 
	// dest point is visible from self
	float fXYDistance = VectorXYDistance( self->s.origin, destPoint );
	float fZDistance = VectorZDistance( self->s.origin, destPoint );
	if ( AI_IsExactDistance( hook, fXYDistance ) && fZDistance < 32.0f )
	{
		PATHLIST_KillPath(hook->pPathList);
		return TRUE;
	}

	float fSpeed = AI_ComputeMovingSpeed( hook );
	if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
	{
		fSpeed = fXYDistance * 10.0f;
	}

	forward = destPoint - self->s.origin;

	// should never be in water, but just in case
	int bOnFlatSurface = AI_IsOnFlatSurface( self );
	if ( self->waterlevel < 3 && bOnFlatSurface && self->movetype != MOVETYPE_HOP )
	{
		forward.z = 0.0f;
	}
	forward.Normalize();

   if ( !bMovingTowardNode && AI_IsGap( self, self->s.origin, forward, fSpeed * 0.125f ) )
   {
       AI_StopEntity( self );
       return FALSE;
   }

	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	AI_SetVelocity( self, forward, (fSpeed * fScale) );

	AI_HandleGroundObstacle(self,fSpeed,bMovingTowardNode);
	AI_HandleCollisionWithEntities(self,destPoint,fSpeed);

	hook->last_origin = self->s.origin;

	AI_UpdateCurrentNode( self );

	ai_frame_sounds( self );
	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_UseLadder
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_UseLadder( userEntity_t *self, MAPNODE_PTR pNode )
{
	_ASSERTE( self );
	_ASSERTE( pNode );
	playerHook_t *hook = AI_GetPlayerHook( self );

	float fSpeed = AI_ComputeMovingSpeed( hook );
	if ( pNode->node_type & NODETYPE_LADDER )
	{
		// we can just check the x,y distance since this function assumes that the 
		// dest point is visible from self
		float fXYDistance = VectorXYDistance( self->s.origin, pNode->position );
		float fZDistance = VectorZDistance( self->s.origin, pNode->position );

		int bXYExactDistance = AI_IsExactDistance( hook, fXYDistance );
		int bZExactDistance = AI_IsExactDistance( hook, fZDistance );
		if ( bXYExactDistance && bZExactDistance )
		{
			return TRUE;
		}

#define LADDERMOVEMENT_VERTICAL	0
#define LADDERMOVEMENT_LATERAL	1
		int nLadderMovement = LADDERMOVEMENT_VERTICAL;
		if ( bXYExactDistance )
		{
			if ( AI_IsCloseDistance2(self, fZDistance) )
			{
				// vertical movement
				fSpeed = fZDistance * 10.0f;
			}
		}
		else
		{
			if ( AI_IsCloseDistance2(self, fXYDistance ) )
			{
				// lateral movement
				nLadderMovement = LADDERMOVEMENT_LATERAL;
				fSpeed = fXYDistance * 10.0f;
			}
		}

		if ( nLadderMovement == LADDERMOVEMENT_VERTICAL )
		{
			NODEDATA_PTR pNodeData = NODE_GetData( pNode );
			forward.x = pNodeData->vector.x * 0.1f;
			forward.y = pNodeData->vector.y * 0.1f;
			if ( self->s.origin.z < pNode->position.z )
			{
				forward.z = 1.0f;
			}
			else
			{
				forward.z = -1.0f;
			}
			forward.Normalize();
		}
		else
		{
			_ASSERTE( nLadderMovement == LADDERMOVEMENT_LATERAL );
			forward = pNode->position - self->s.origin;
			forward.Normalize();
		}
	}

	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	AI_SetVelocity( self, forward, (fSpeed * fScale) );

	hook->last_origin = self->s.origin;

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_Move
// Description:
//				move using the node system
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_Move( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return FALSE;
	}

	if ( AI_IsFlyingUnit( self ) )
	{
		return AI_Fly( self );
	}
    else
    if ( AI_IsInWater( self ) )
    {
        return AI_Swim( self );
    }
	else if (self->movetype == MOVETYPE_TRACK)
	{
		return AI_TrackMove( self);
	}

	if ( hook->pPathList->pPath && hook->pPathList->pPath->nNodeIndex >= 0 )
	{
		_ASSERTE( pGroundNodes );

		
		MAPNODE_PTR pNode = NODE_GetNode( pGroundNodes, hook->pPathList->pPath->nNodeIndex );
		
		if(pNode)
		{
			CVector destPoint = pNode->position;
			if ( AI_MoveTowardPoint( self, destPoint, TRUE ) == TRUE )
			{
				// we're already at this node, so delete the first one
				PATHLIST_DeleteFirstInPath( hook->pPathList );

				if ( hook->pPathList->pPath )
				{
					NODELIST_PTR pNodeList = hook->pNodeList;
					_ASSERTE( pNodeList );
					NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
					_ASSERTE( pNodeHeader );

					MAPNODE_PTR pNextNode = NODE_GetNode( pNodeHeader, hook->pPathList->pPath->nNodeIndex );
					if ( AI_HandleUse( self, pNodeHeader, pNode, pNextNode ) )
					{
						return TRUE;
					}
				}
				// NSS[1/20/00]:Added for the case in which we have reached the appropriate distance, have deleted
				// the current node, and there are no more nodes in the path to traverse.
				if(!AI_Move( self ))
				{
					GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
					if(pGoalStack)
					{
						GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );

						if ( GOAL_GetType( pCurrentGoal ) == GOALTYPE_MOVETOLOCATION &&	 GOAL_GetNumTasks( pCurrentGoal ) == 1 )
						{
								AI_HandleUse( self );
								GOAL_Satisfied( pCurrentGoal );
						}
					}
					self->velocity.Zero();
					if(AI_IsSidekick(hook) && SIDEKICK_HasTaskInQue(self,TASKTYPE_SIDEKICK_PICKUPITEM))
						return FALSE;
					AI_RemoveCurrentTask( self );					//AI_RemoveCurrentTask(self,FALSE);
				}
			}
		}
		else
		{
			AI_Dprintf("pNode returned a NULL value! Skipping move to node section!<nss>\n");
		}

		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_TrackMove
// Description:
//				move using the node system
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_TrackMove( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	PATHNODE_PTR pCurrentPath = hook->pPathList->pPath;
	if ( pCurrentPath && pCurrentPath->nNodeIndex >= 0 && pCurrentPath != hook->pPathList->pPathEnd )
	{
		_ASSERTE( pTrackNodes );

		MAPNODE_PTR pNode = NODE_GetNode( pTrackNodes, pCurrentPath->nNodeIndex );
		if ( AI_TrackMoveTowardPoint( self, pNode->position ) == TRUE )
		{
			// we're already at this node, so delete the first one
			PATHLIST_DeleteFirstInPath( hook->pPathList );

			AI_TrackMove( self );
		}

		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartFly
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_StartFly( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	_ASSERTE( AI_IsFlyingUnit( self ) );
	AI_ForceSequence(self, "flya", FRAME_LOOP);
	
	hook->nMoveCounter = 0;

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_FlyTowardPoint
// Description:
//				move straight toward a point
// Input:
// Output:
//				TRUE	=> if reached the point
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
__inline static
int AI_CheckLeftBottom( userEntity_t *self, CVector &dir, float fWidth )
{
	_ASSERTE( self );
	float fTwiceWidth = 2.0f * fWidth;

	CVector leftOrthogonalVector( -dir.y, dir.x, 0.0f );
	leftOrthogonalVector.Normalize();

	CVector leftPoint;
	VectorMA( self->s.origin, leftOrthogonalVector, fTwiceWidth, leftPoint );

	tr = gstate->TraceLine_q2( self->s.origin, leftPoint, self, MASK_SOLID );
	if ( tr.fraction == 1.0f )
	{
		CVector bottomPoint( leftPoint.x, leftPoint.y, leftPoint.z + (-1.0 * fTwiceWidth) );

		tr = gstate->TraceLine_q2( leftPoint, bottomPoint, self, MASK_SOLID );
		if ( tr.fraction >= 0.6f )
		{
			return TRUE;
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------

__inline static
int AI_CheckRightBottom( userEntity_t *self, CVector &dir, float fWidth )
{
	_ASSERTE( self );
	float fTwiceWidth = 2.0f * fWidth;

	CVector rightOrthogonalVector( dir.y, -dir.x, 0.0f );
	rightOrthogonalVector.Normalize();

	CVector rightPoint;
	VectorMA( self->s.origin, rightOrthogonalVector, fTwiceWidth, rightPoint );
	tr = gstate->TraceLine_q2( self->s.origin, rightPoint, self, MASK_SOLID );
	if ( tr.fraction == 1.0f )
	{
		CVector bottomPoint( rightPoint.x, rightPoint.y, rightPoint.z + (-1.0 * fTwiceWidth) );

		tr = gstate->TraceLine_q2( rightPoint, bottomPoint, self, MASK_SOLID );
		if ( tr.fraction >= 0.6f )
		{
			return TRUE;
		}
	}

	return FALSE;
}
int AI_HasTaskInQue( userEntity_t *self, TASKTYPE SearchTask );
// ----------------------------------------------------------------------------
//
// Name:		AI_HandleAirCollisionWithEntities
// Description:
// Input:
// Output:
//				TRUE	=> if collision with entity and handled
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int AI_HandleAirCollisionWithEntities( userEntity_t *self, CVector &destPoint, float fSpeed )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	float fHalfWidth = fabs(self->s.maxs.x - self->s.mins.x) * 0.5f;

	// check collision with other entities
	float fOneFrameSpeed = (fSpeed * 0.125f);

	CVector checkPoint;
	VectorMA( self->s.origin, forward, fOneFrameSpeed + fHalfWidth, checkPoint );

	CVector mins = self->s.mins;
    mins.z += 16.0f;
    CVector maxs = self->s.maxs;

    tr = gstate->TraceBox_q2( self->s.origin, mins, maxs, checkPoint, self, CONTENTS_MONSTER );
	if ( tr.fraction < 1.0f )
	{
		if ( tr.ent && _stricmp( tr.ent->className, "worldspawn" ) != 0 )
		{
			if ( tr.ent->flags & (FL_MONSTER|FL_CLIENT|FL_BOT) )
			{
				userEntity_t *pCollidedEnt = tr.ent;
                
                if ( pCollidedEnt == self->enemy )
                {
                    return FALSE;
                }

                if ( pCollidedEnt->velocity.Length() > 0.0f )
                {
				    AI_AddNewTaskAtFront( self, TASKTYPE_WAITFORNOCOLLISION, destPoint );
                }
                else
                {
                    // see if the collided entity is on top of the destPoint
                    float fCollidedEntDistToDestPoint = VectorXYDistance( pCollidedEnt->s.origin, destPoint );
				    float fCollidedEntZDistance = VectorZDistance( pCollidedEnt->s.origin, destPoint );
				    if ( (fCollidedEntDistToDestPoint <= (fHalfWidth *3.0f) && fCollidedEntZDistance < 32.0f) )
                    {
                        // if the current destination is the next point in the path list remove it
                        if ( hook->pPathList->pPath && hook->pPathList->pPath->nNodeIndex >= 0 )
                        {
		                    MAPNODE_PTR pNode = NODE_GetNode( pGroundNodes, hook->pPathList->pPath->nNodeIndex );
		                    float fDistance = VectorDistance( destPoint, pNode->position );
                            if ( fDistance < 32.0f )
                            {
                                PATHLIST_DeleteFirstInPath( hook->pPathList );
                            }
                        }
                    }

                    return AI_FindPathAroundEntity( self, pCollidedEnt, destPoint );
			    }
            }
			else
			if ( _stricmp( tr.ent->className, "func_door" ) == 0 )
			{
				AI_AddNewTaskAtFront( self, TASKTYPE_WAIT, 1.0f );
			}
			else
			{
				// must have collided with other obstacles, try to go around
				if(!AI_FindPathAroundEntity(self,tr.ent,destPoint) && AI_HasTaskInQue( self, TASKTYPE_MOVETOLOCATION ) <= 3)
				{
					CVector dir;
					forward = dir;
					AI_FindNewCourse( self, &dir);
					forward = dir;
					forward.Normalize();
					float fScale = FRAMES_ComputeFrameScale( hook );
					AI_SetVelocity( self, forward, (fSpeed * fScale) );
				}
			}

			return TRUE;
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_HandleAirUnitGettingStuck
// Description:
// Input:
// Output:
//				TRUE	=> if detected a stuck condition and handled
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int AI_HandleAirUnitGettingStuck( userEntity_t *self, CVector &destPoint, float fSpeed )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	//float fScale = FRAMES_ComputeFrameScale( hook );
	float fScale = 0.5f;
	
	// check collision with other entities
	float fOneFrameSpeed = fSpeed * 0.1f;
	CVector checkPoint;
	VectorMA( self->s.origin, forward, fOneFrameSpeed, checkPoint );

	CVector org( self->s.origin );
	float fMoveDistance = hook->last_origin.Distance(org);
	if ( fMoveDistance < (self->velocity.Length() * 0.025f) )
	{
		// check blocks on the sides
		int nBlockDirection = AI_CheckBlockingDirectionFlyUnits( self, checkPoint );

//	    float dist = fSpeed * 0.1f;
//	    if ( self->velocity.Length() > 0 )
//	    {
//		    dist = fSpeed * 0.125f;
//	    }
//        int nBlockDirection = AI_CheckAirTerrain( self, forward, dist );
		
		switch ( nBlockDirection )
		{
			case BLOCKED_BOTH_BY_ENTITY:
			{
				if ( tr.ent )
				{
					AI_GoAroundObstacle( self );
					return TRUE;
				}
				break;
			}

			case BLOCKED_LEFT_BY_ENTITY:
			case TER_WALL_LEFT:
			{
				CVector dir;
				AI_RightVector( forward, dir );
				AI_SetVelocity( self, dir, (fSpeed) );
				break;
			}
			case BLOCKED_RIGHT_BY_ENTITY:
			case TER_WALL_RIGHT:
			{
				CVector dir;
				AI_LeftVector( forward, dir );
				AI_SetVelocity( self, dir, (fSpeed) );
				break;
			}
			case BLOCKED_ABOVE_BY_ENTITY:
			case TER_HIGH_OBS:
			{
				CVector dir(0.0,0.0, -1.0f);
				dir.Normalize();
				forward = dir;

				AI_SetVelocity( self, forward, (fSpeed * fScale) );
				break;
			}
			case BLOCKED_BELOW_BY_ENTITY:
			case TER_LOW_OBS:
			{
				if(self->movetype != MOVETYPE_SWIM &&!(hook->dflags & DFL_AMPHIBIOUS))
				{
					CVector dir(0.0f,0.0f, 1.0 );
					dir.Normalize();
					forward = dir;
					AI_SetVelocity( self, forward, (fSpeed * fScale) );
				}
				break;
			}
			case TER_CLEAR:
			{
				break;
			}
		}

		hook->nWaitCounter++;
		if ( (hook->nWaitCounter > 2 && fMoveDistance < 1.0f) || hook->nWaitCounter > 20 )
		{
			

			NODELIST_PTR pNodeList = hook->pNodeList;
			_ASSERTE( pNodeList );
			NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
			_ASSERTE( pNodeHeader );
			MAPNODE_PTR pClosestNode = NODE_GetClosestNode(self);
			if ( !pClosestNode )
			{
				if ( pNodeList->nCurrentNodeIndex != -1 )
				{
					pClosestNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
				}
			}
			
			if ( pClosestNode )
			{
				AI_Dprintf("Adding MOVETOLOCATION from Air Unit handle getting stuck.\n");
				AI_RemoveCurrentTask( self, TASKTYPE_MOVETOLOCATION, pClosestNode->position );
			}
			else
			{
				AI_RemoveCurrentTask( self );
			}

			return TRUE;
		}
	} 
	else
	{
		// moved correctly
		hook->nWaitCounter = 0;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_HandleAirObstacle
// Description:
// Input:
// Output:
//				TRUE	=> if found obstacle and handled
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int AI_HandleAirObstacle( userEntity_t *self, float fSpeed, int bMovingTowardNode )
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	float dist = fSpeed * 0.1f;

	int COLLISION = FALSE;
	if ( self->velocity.Length() > 0 )
	{
		dist = fSpeed * 0.125f;
	}

	int nWallType = AI_CheckAirTerrain( self, forward, dist );
	switch ( nWallType )
	{
		case TER_OBSTRUCTED:
		{
			AI_Dprintf( "(Air Unit)Strafing along the wall.\n" );

			CVector dir;
			self->s.angles.AngleToForwardVector( dir );
			if(self->movetype != MOVETYPE_SWIM || ((hook->dflags & DFL_AMPHIBIOUS) && self->waterlevel > 2))
			{
				AI_FindNewCourse(self, &dir);
			}
			else
			{
				forward = dir;
				AI_ComputeAlongWallVectorForAir( self, forward, terrain.wall_normal, dir );
				if ( dir.Length() > 0.0f )
				{
    				if(dir.x == 0.0 && dir.y == 0.0 && dir.z == 1.0 && (hook->dflags & DFL_AMPHIBIOUS))
					{
						forward.Set( forward.x, forward.y, dir.z );
					}
					else
					{
						forward.Set( dir.x, dir.y, dir.z );
					}
				}
				COLLISION = TRUE;
			}
			break;
		}
		case TER_WALL:
		{
			AI_Dprintf( "Partial wall blocked.\n" );

			AI_Dprintf( "(Air Unit)Strafing along the wall.\n" );

			CVector dir;
			self->s.angles.AngleToForwardVector( dir );
			if(self->movetype != MOVETYPE_SWIM  || ((hook->dflags & DFL_AMPHIBIOUS) && self->waterlevel > 2))
			{
				AI_FindNewCourse(self, &dir);
			}
			else
			{
				forward = dir;
				AI_ComputeAlongWallVectorForAir( self, forward, terrain.wall_normal, dir );
				if ( dir.Length() > 0.0f )
				{
    				forward.Set( dir.x, dir.y, 0.0f);
				}
				COLLISION = TRUE;
			}
			break;
		}
		case TER_WALL_RIGHT:
		{
			AI_Dprintf( "Right side Blocked.\n" );

			// right side is blocked
			// rotate the wall normal to the left so that we can move around this obstacle
			CVector dir;
			if ( terrain.wall_normal.y == 0.0f && terrain.wall_normal.x == 0.0f )
			{
				dir.Set( -forward.y, forward.x, 0.0f );	
			}
			else
			{
				dir.Set( terrain.wall_normal.y, -terrain.wall_normal.x, 0.0f );
			}
			COLLISION = TRUE;
			forward.Set( dir.x, dir.y, 0.0f );
			break;
		}
		case TER_WALL_LEFT:
		{
			AI_Dprintf( "Left side Blocked.\n" );
			
			// left side is blocked
			// rotate the wall normal to the right so that we can move around this obstacle
			CVector dir;
			if ( terrain.wall_normal.y == 0.0f && terrain.wall_normal.x == 0.0f )
			{
				dir.Set( forward.y, -forward.x, 0.0f );
			}
			else
			{
				dir.Set( -terrain.wall_normal.y, terrain.wall_normal.x, 0.0f );
			}
			COLLISION = TRUE;
			forward.Set( dir.x, dir.y, 0.0f );
			break;
		}		
		case TER_HIGH_OBS:
		{
			AI_Dprintf( "High obstruction blocked.\n" );

            forward.Set( 0.0f, 0.0f, -1.0f );
			COLLISION = TRUE;
			break;
		}
		case TER_LOW_OBS:
		{
			AI_Dprintf( "Detected low obstruction.\n" );

			forward.Set( 0.0f, 0.0f, 1.0f );
			COLLISION = TRUE;
			break;
		}
		case TER_FAR_WALL:
		{
			break;
		}

		default:
		{
			break;
		}
	}

	return COLLISION;
}

// ----------------------------------------------------------------------------

void AI_HandleCollisionWithVerticalSurface( userEntity_t *self, CVector &facingDir, float fWidth )
{
	_ASSERTE( self );

	// see if left or the right side is free
	if ( AI_CheckLeftBottom( self, facingDir, fWidth ) == TRUE )
	{
		facingDir.x = -forward.y;
		facingDir.y = forward.x;
		facingDir.z = 0.0f;
	}
	else
	if ( AI_CheckRightBottom( self, facingDir, fWidth ) == TRUE )
	{
		facingDir.x = forward.y;
		facingDir.y = -forward.x;
		facingDir.z = 0.0f;
	}
	else
	{
		facingDir.z = 0.0f;
	}
}
#ifdef _DEBUG
void AI_Drop_Markers (CVector &Target, float delay);
#endif

int AI_FlyTowardPoint( userEntity_t *self, CVector &destPoint, int bMovingTowardNode, int bFaceDestination )
{
	_ASSERTE( self );

	playerHook_t *hook = AI_GetPlayerHook( self );
	
	//Any special movetypes get handled here <nss>
	switch(hook->type)
	{
		case TYPE_SL_SKEET:
		{
			return AI_FlyTowardPoint2( self, destPoint, 0.30f);
		}
		case TYPE_TH_SKEET:
		{
			return AI_FlyTowardPoint2( self, destPoint, 0.05f);
		}
		case TYPE_SEAGULL:
		{
			return SEAGULL_FlyTowardPoint( self, destPoint);		
		}
		case TYPE_DOOMBAT:
		{
			return AI_FlyTowardPoint2( self, destPoint, 0.25f);
		}
		case TYPE_GRIFFON:
		{	
			return AI_FlyTowardPoint2( self, destPoint, 0.55f);
		}
		default:	
		{
			break;
		}
	
	}
	if ( AI_IsInsideBoundingBox( self ) == FALSE )
	{
		AI_RemoveCurrentGoal( self );
		return FALSE;
	}

	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return FALSE;
	}

	if ( self->movetype == MOVETYPE_HOVER || !bFaceDestination )
	{
		_ASSERTE( AI_IsFlyingUnit( self ) );
		if ( self->enemy && AI_IsVisible( self, self->enemy ) )
		{
			AI_FaceTowardPoint( self, self->enemy->s.origin );
			AI_UpdatePitchTowardEnemy( self );
		}
		else
		{
			AI_SetTurnRate( self, destPoint );
			AI_OrientTowardPoint( self, destPoint );
			self->s.angles.pitch = 0.0f;
		}
	}
	else
	{
		_ASSERTE( self->movetype == MOVETYPE_FLY || self->movetype == MOVETYPE_BOUNCEMISSILE);

		AI_SetTurnRate( self, destPoint );
		AI_OrientTowardPoint( self, destPoint );
	}

	float fDistance = VectorDistance( self->s.origin, destPoint );
	if ( AI_IsCloseDistance2(self, fDistance ) )
	{
		return TRUE;
	}
	
	float fSpeed = AI_ComputeMovingSpeed( hook );
/*
	if ( AI_IsCloseDistance( hook, fDistance ) )
	{
		fSpeed = fDistance * 10.0f;
	}
*/
 
	CVector vectorTowardPoint;
	CVector angleTowardPoint;
	vectorTowardPoint = destPoint - self->s.origin;
	vectorTowardPoint.Normalize();


	if ( self->movetype == MOVETYPE_HOVER || !bFaceDestination )
	{
		_ASSERTE( AI_IsFlyingUnit( self ) );

		forward = vectorTowardPoint;
	}
	else
	{
		//Vector to point is set here
		_ASSERTE( self->movetype == MOVETYPE_FLY || self->movetype == MOVETYPE_BOUNCEMISSILE );

		VectorToAngles( vectorTowardPoint, angleTowardPoint );
		CVector angleVector = self->s.angles;
		CVector right, up;
		if ( AI_IsSameAngle3D( self, angleVector, angleTowardPoint ) == TRUE )
		{
			forward = vectorTowardPoint;
		}
		else
		{
			angleVector.AngleToForwardVector( forward );
			forward.Normalize();
		}
	}
		
	if ( AI_HandleAirCollisionWithEntities( self, destPoint, fSpeed ) )
	{
		return FALSE;
	}

	if ( AI_HandleAirObstacle( self, fSpeed, bMovingTowardNode ) )
	{
		return FALSE;
	}

	forward.Normalize();

    //	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	AI_SetVelocity( self, forward, (fSpeed * fScale) );

	if ( AI_HandleAirUnitGettingStuck( self, destPoint, fSpeed ) )
	{
		return FALSE;
	}

	hook->last_origin = self->s.origin;

	ai_frame_sounds( self );

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_Fly
// Description:
//				move using the node system
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_Fly( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return FALSE;
	}

	if ( hook->pPathList->pPath && hook->pPathList->pPath->nNodeIndex >= 0 )
	{
		_ASSERTE( pAirNodes );
		MAPNODE_PTR pNode = NODE_GetNode( pAirNodes, hook->pPathList->pPath->nNodeIndex );
		if ( AI_FlyTowardPoint( self, pNode->position, TRUE ) == TRUE )
		{
			// we're already at this node, so delete the first one
			PATHLIST_DeleteFirstInPath(hook->pPathList);
		}

		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartChaseAttackStrafing
// Description:	Entity is within optimal distance for best weapon, so attack
//				without going forward
// Input:
// Output:
// Note:
//				for ECTS demo entity just stands and attacks
//
// ----------------------------------------------------------------------------
void AI_StartChaseAttackStrafing(userEntity_t *self)
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_CHASEATTACKSTRAFING );

	if ( AI_StartSequence( self, "ataka" ) == FALSE )
	{
		return;
	}

	int nDirection = RIGHT;
	if ( rnd() > 0.5f )
	{
		nDirection = LEFT;
	}

	CVector sideStepPoint;
	int bSuccess = AI_ComputeChaseSideStepPoint( self, self->enemy, nDirection, sideStepPoint );
	if ( bSuccess )
	{
		AI_SetStateRunning( hook );
		if ( AI_StartMove( self ) == FALSE )
		{
			TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
            _ASSERTE( pCurrentTask );
			if ( TASK_GetType( pCurrentTask ) != TASKTYPE_CHASEATTACKSTRAFING )
			{
				return;
			}
		}

		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		TASK_Set( pCurrentTask, sideStepPoint );

		AI_Dprintf( "Starting TASKTYPE_CHASEATTACKSTRAFING.\n" );

		AI_SetOkToAttackFlag( hook, FALSE );
		AI_SetTaskFinishTime( hook, 2.0f );
		AI_SetMovingCounter( hook, 0 );

		AI_SetNextThinkTime( self, 0.1f );
	}
	else
	{
		AI_RemoveCurrentTask( self, TASKTYPE_CHARGETOWARDENEMY );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ChaseAttackStrafing
//
// Description:	entity is within optimal distance of target, and is strafing
//				to avoid fire
//				
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ChaseAttackStrafing(userEntity_t *self)
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	float fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );

	int bEnemyVisible = FALSE;
	if ( AI_IsRangeAttack(hook) )
	{
		bEnemyVisible = AI_IsCompletelyVisible( self, self->enemy );
		if ( bEnemyVisible == COMPLETELY_VISIBLE )
		{
			bEnemyVisible = TRUE;
		}
        else
        {
            bEnemyVisible = FALSE;
        }
	}
	else
	{
		bEnemyVisible = AI_IsVisible(self, self->enemy);
	}

	if ( !AI_IsAlive( self->enemy ) )
	{
		AI_RemoveCurrentGoal (self);
		return;
	}

	if ( !bEnemyVisible || !AI_IsWithinAttackDistance( self, fDistance ) || !hook->fnChasingAttack( self ) )
	{
		AI_RemoveCurrentTask (self);
		return;
	}

	AI_SetNextThinkTime( self, 0.1f );
	if ( !bEnemyVisible )
	{
		AI_SetTaskFinishTime( hook, 30.0f );
	}			

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	AI_MoveTowardPoint( self, pAIData->destPoint, FALSE );

	hook->fnChasingAttack( self );

}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartChaseAttack
// Description:	Entity is starting an attack in which they can follow their
//				enemy while attacking
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartChaseAttack( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_CHASEATTACK );

	AI_Dprintf( "Starting TASKTYPE_CHASEATTACK.\n" );

	if ( AI_IsFlyingUnit( self ) )
	{
	    if ( AI_StartFly( self ) == FALSE )
        {
		    TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
            _ASSERTE( pCurrentTask );
		    if ( TASK_GetType( pCurrentTask ) != TASKTYPE_CHASEATTACK )
		    {
			    return;
		    }
        }
	}
    else
    if ( AI_IsInWater( self ) )
    {
	    if ( AI_StartSwim( self ) == FALSE )
        {
		    TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
            _ASSERTE( pCurrentTask );
		    if ( TASK_GetType( pCurrentTask ) != TASKTYPE_CHASEATTACK )
		    {
			    return;
		    }
        }
    }
	else
	{
		//	path to enemy 
        userEntity_t *pEnemy = self->enemy;
		if ( !AI_FindPathToEntity( self, pEnemy ) && AI_CanPath( hook ) )
		{
			// do not try another path for 2 seconds
			AI_SetNextPathTime( hook, 2.0f );

            float fXYDistance = VectorXYDistance( self->s.origin, pEnemy->s.origin );
            float fZDistance = VectorZDistance( self->s.origin, pEnemy->s.origin );
            if ( !AI_IsOkToMoveStraight( self, pEnemy->s.origin, fXYDistance, fZDistance ) )
            {
                AI_RemoveCurrentGoal( self );
            }

			return;
		}

		AI_SetStateRunning( hook );
		if ( AI_StartMove( self ) == FALSE )
		{
			TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
            _ASSERTE( pCurrentTask );
			if ( TASK_GetType( pCurrentTask ) != TASKTYPE_CHASEATTACK )
			{
				return;
			}
		}
	}

	AI_SetNextThinkTime( self, 0.1f );
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 10.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ChaseAttack
//
// Description:	entity is chasing after an enemy, possibly on a path, and
//				attacking while moving
//				
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ChaseAttack( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *pEnemy = self->enemy;
    _ASSERTE( pEnemy );

    int bEnemyVisible = FALSE;
	if ( AI_IsRangeAttack(hook) )
	{
		bEnemyVisible = AI_IsCompletelyVisible( self, pEnemy );
		if ( bEnemyVisible == COMPLETELY_VISIBLE )
		{
			bEnemyVisible = TRUE;
		}
        else
        {
            bEnemyVisible = FALSE;
        }
	}
	else
	{
		bEnemyVisible = AI_IsVisible(self, pEnemy);
	}

	if ( !bEnemyVisible )
	{
		AI_SetTaskFinishTime( hook, 10.0f );
	}

	float fDistance = VectorDistance( self->s.origin, pEnemy->s.origin );
	if ( AI_IsFlyingUnit( self ) || AI_IsInWater( self ) )
	{
		// in air
        if ( fDistance > hook->active_distance || !AI_IsAlive( pEnemy ) )
		{
			//	the enemy is out of range or dead, give up chasing
			AI_RemoveCurrentGoal( self );
			return;
		}
		else
		{
			//	call attack function
			if ( hook->fnChasingAttack )
			{
				hook->fnChasingAttack( self );
			}
			AI_MoveTowardPoint( self, pEnemy->s.origin, FALSE );
		}
	}
	else
	{
		// on ground
        if ( (fDistance > hook->active_distance && !hook->pPathList->pPath) || !AI_IsAlive( pEnemy ) )
		{
			// the enemy is out of range or dead, give up chasing
			AI_RemoveCurrentGoal( self );

			return;
		}
		else
		{
			if ( AI_HandleUse( self ) )
			{
				if ( bEnemyVisible )
				{
					//	call attack function
					if ( hook->fnChasingAttack )
					{
						hook->fnChasingAttack( self );
					}
				}
			}
			else
			{
				if ( !AI_IsPathToEntityClose( self, pEnemy ) )
				{
					//	path to enemy if no path exists or if time to repath
					if ( !AI_FindPathToEntity( self, pEnemy ) && AI_CanPath( hook ) )
					{
						// do not try another path for 5 seconds
						AI_SetNextPathTime( hook, 5.0f );
						
                        float fXYDistance = VectorXYDistance( self->s.origin, pEnemy->s.origin );
                        float fZDistance = VectorZDistance( self->s.origin, pEnemy->s.origin );
                        if ( !AI_IsOkToMoveStraight( self, pEnemy->s.origin, fXYDistance, fZDistance ) )
                        {
                            AI_RemoveCurrentGoal( self );
                        }
						return;
					}
				}

				if ( bEnemyVisible )
				{
					//	call attack function
					int nCanAttack = hook->fnChasingAttack( self );

					if ( nCanAttack && AI_IsWithinAttackDistance( self, fDistance ) )
					{
						AI_AddNewTaskAtFront (self, TASKTYPE_CHASEATTACKSTRAFING);
						return;
					}
				}

				if ( AI_Move( self ) == FALSE)
				{
					PATHLIST_KillPath (hook->pPathList);
				}
			}
					
			hook->nMoveCounter++;
		}
	}

}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartChase
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartChase( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	float fDistance = 0.0f;

	if( hook->type == TYPE_BATTLEBOAR )
	{
		AI_ForceSequence( self, "rev" );
		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e1/m_bboarrev.wav"), 
			1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	}

	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	if ( !self->enemy )
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		AIDATA_PTR pAIData = GOAL_GetData(pCurrentGoal);
		self->enemy = pAIData->pEntity;
	}

	// NSS[2/22/00]:
	/*if(self->enemy )
	{
		fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );
		if(AI_IsWithinAttackDistance( self, fDistance ) && AI_IsChaseVisible( self ))
		{
			
			if(!AI_IsSidekick(hook))
			{
				AI_RemoveCurrentTask( self );
				return;		
			}
		}
	}
	else
	{
		AI_RemoveCurrentTask( self );
		return;	
	}*/

	if ( AI_IsFlyingUnit( self ) )
	{
	    if ( AI_StartFly( self ) == FALSE )
        {
		    TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
            _ASSERTE( pCurrentTask );
		    if ( TASK_GetType( pCurrentTask ) != TASKTYPE_CHASE )
		    {
			    return;
		    }
        }
	}
    else
    if ( AI_IsInWater( self ) )
    {
	    if ( AI_StartSwim( self ) == FALSE )
        {
		    TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
            _ASSERTE( pCurrentTask );
		    if ( TASK_GetType( pCurrentTask ) != TASKTYPE_CHASE )
		    {
			    return;
		    }
        }
    }
	else
	{
		// ground units
		//	path to enemy 

    	userEntity_t *pEnemy = self->enemy;
        float fXYDistance = VectorXYDistance( self->s.origin, pEnemy->s.origin );
        float fZDistance = VectorZDistance( self->s.origin, pEnemy->s.origin );
        if ( !AI_FindPathToEntity( self, pEnemy ) && AI_CanPath( hook ) &&
             !AI_IsOkToMoveStraight( self, pEnemy->s.origin, fXYDistance, fZDistance ) )
		{
			if ( AI_IsLineOfSight( self, pEnemy ) )
            {
                // take brief cover
                AI_AddNewTaskAtFront( self, TASKTYPE_BRIEFCOVER, pEnemy );
            }
            else
            {
                // do not try another path for 5 seconds
			    AI_SetNextPathTime( hook, 5.0f );

			    //	no path was found, so do not do a chasing attack
			    AI_RemoveCurrentGoal( self );
            }

			return;
		}

		if( hook->type == TYPE_COLUMN && fDistance < 500.0f ) // the column only runs when more than 500 away
		{
			AI_SetStateWalking( hook );
		}
		else
		{
			AI_SetStateRunning( hook );
		}

		if ( AI_StartMove( self ) == FALSE )
		{
			TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
            _ASSERTE( pCurrentTask );
			if ( TASK_GetType( pCurrentTask ) != TASKTYPE_CHASE )
			{
				return;
			}
		}
	}

    AI_Dprintf( "Starting TASKTYPE_CHASE.\n" );

	AI_SetNextThinkTime( self, 0.1f );
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 20.0f );
	AI_SetMovingCounter( hook, 0 );

	//AI_Chase( self );
}

int psyclaw_chasethink(userEntity_t *self);
// ----------------------------------------------------------------------------
//
// Name:		AI_Special_Chase_Think
// Description:This function deals with special case monsters that have unique
// functions to call within AI_Chase.  
// Input:userEntity_t *self, playerHook_t *hook
// Output:int(1/0) 
// ----------------------------------------------------------------------------
int AI_Special_Chase_Think( userEntity_t *self, playerHook_t *hook )
{

	switch(hook->type)
	{
		case TYPE_SL_SKEET:
		{
			SKEETER_Chase( self );
			return 1;
		}
		case TYPE_TH_SKEET:
		{
			THUNDERSKEET_Chase( self );
			return 1;
		}
		case TYPE_BUBOID:
		{
			if( BUBOID_ChaseThink( self ) )
				return 1;
			break;
		}
		case TYPE_PSYCLAW:
		{
			if(psyclaw_chasethink(self))
				return 1;
			break;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_Check_For_Life
// Description:Checks to see if the AI is alive
// Input:userEntity_t *self, playerHook_t *hook, userEntity_t *pEnemy
// Output:int(1/0) 
// Note:
//
// ----------------------------------------------------------------------------

int AI_Check_For_Life( userEntity_t *self, playerHook_t *hook, userEntity_t *pEnemy)
{
	if ( !AI_IsAlive( pEnemy ) )
	{
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
		if ( nGoalType == GOALTYPE_KILLENEMY )
		{
			GOAL_Satisfied( pCurrentGoal );
		}

		AI_RemoveCurrentTask( self );
		return 1;
	}
	return 0;
}


// ----------------------------------------------------------------------------
//
// Name:		AI_TrackChase
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_TrackChase( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	_ASSERTE( self->movetype == MOVETYPE_TRACK );

	userEntity_t *pEnemy = self->enemy;

    //NSS[11/27/99]:IF we have no enemy let's not try to chase after it no?
	if(!pEnemy)
	{
		AI_RemoveCurrentTask( self );
		return;
	}
	float fDistance = VectorDistance( self->s.origin, pEnemy->s.origin );
	if ( AI_IsWithinAttackDistance( self, fDistance ) )
	{
		AI_RemoveCurrentTask( self );
		return;
	}
	else
	{
		if ( AI_IsVisible( self, pEnemy ) )
		{
			AI_FaceTowardPoint( self, pEnemy->s.origin );
			AI_UpdatePitchTowardEnemy( self );
			if ( !hook->pPathList->pPath || (hook->nMoveCounter % 100) == 50 )
			{
				if ( !AI_FindPathToEntity( self, self->enemy ) && AI_CanPath( hook ) )
				{
					// do not try another path for 5 seconds
					AI_SetNextPathTime( hook, 5.0f );
					return;
				}
			}

			if ( AI_TrackMove( self ) == FALSE )
			{
				PATHLIST_KillPath( hook->pPathList );
			}
			hook->nMoveCounter++;
		}
		else
		{
			hook->nMoveCounter = 0;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_Chase
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Chase( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	userEntity_t *pEnemy = self->enemy;
	


	if(self->enemy)
	{
		if(!AI_IsStateRunning(hook))
		{
			AI_SetStateRunning(hook);
		}
		//NSS[11/23/99]:IF we have seen the player and the player is dead
		if(!AI_IsAlive(self->enemy) && (hook->ai_flags&AI_SEENPLAYER))
		{
			AI_RemoveCurrentTask(self,TASKTYPE_WANDER);
			return;
		}
	}
	// NSS[12/7/99]:Fixing some animation prob's
	if(AI_IsEndAnimation(self))
	{
		//if(hook && hook->cur_sequence && hook->cur_sequence->animation_name)
		//	AI_ForceSequence(self,hook->cur_sequence->animation_name,FRAME_LOOP);
		//else
		//{
			if ( AI_IsFlyingUnit( self ) )
			{
				AI_StartFly( self );
			}
			else
			if ( AI_IsInWater( self ) )
			{
			   AI_StartSwim( self );
			}		
			else
			{
				AI_StartMove( self );
			}
		//}

	}
    if(pEnemy == NULL)
	{
		AI_RemoveCurrentTask(self,FALSE);
		return;
	}

	if ( AI_Special_Chase_Think( self, hook ) || AI_Check_For_Life( self, hook, pEnemy) )//<nss>
	{
		return;
	}

	float fDistance = VectorDistance( self->s.origin, pEnemy->s.origin );
	if ( (fDistance > hook->active_distance && !hook->pPathList->pPath) && !(hook->ai_flags&AI_SEENPLAYER))
	{
		// the enemy is out of range or dead, give up chasing
		AI_RemoveCurrentGoal( self );
		return;
	}

	if ( (hook->ai_flags & AI_SNIPE) )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	if ( self->movetype == MOVETYPE_TRACK )
	{
        AI_TrackChase( self );
        return;
    }

	int bEnemyVisible = FALSE;
	if ( AI_IsRangeAttack(hook) )
	{
        bEnemyVisible = AI_IsChaseVisible( self );
    }
	else
	{
		bEnemyVisible = AI_IsVisible(self, pEnemy);
	}

	if ( bEnemyVisible || (hook->ai_flags & AI_SEENPLAYER) )
	{
		// extend the time
		AI_SetTaskFinishTime( hook, 15.0f );
	}

	// the column walks when less than 500 away
	if ( hook->type == TYPE_COLUMN && fDistance < 500.0f && !AI_IsStateWalking( hook ))
	{
		AI_SetStateWalking( hook );
	}

	// the column runs when more than 500 away
	if ( hook->type == TYPE_COLUMN && fDistance > 500.0f && !AI_IsStateRunning( hook ))
	{
		AI_SetStateRunning( hook );
	}

	if ( AI_IsFlyingUnit( self ) || AI_IsInWater( self ) )
	{
		// in air and we can see the target
        if ( bEnemyVisible )
		{
			if ( AI_IsWithinAttackDistance( self, fDistance ) )
			{
				AI_RemoveCurrentTask( self );
				return;
			}
			else
			{
				AI_MoveTowardPoint( self, pEnemy->s.origin, FALSE );
			}
		}
		else  //we can't see the target and must now use the nodes.
		{
			// test to see if the last path point is close to the enemy's position
			if ( !AI_IsPathToEntityClose( self, pEnemy ) && !AI_FindPathToEntity( self, pEnemy ) && AI_CanPath( hook ) )
			{
				// do not try another path for 1 seconds
				AI_SetNextPathTime( hook, 1.0f );

                float fXYDistance = VectorXYDistance( self->s.origin, pEnemy->s.origin );
                float fZDistance = VectorZDistance( self->s.origin, pEnemy->s.origin );
                if ( !AI_IsOkToMoveStraight( self, pEnemy->s.origin, fXYDistance, fZDistance ) )
                {
                    AI_Dprintf("No Path and can't move straight.  Removing Goal!!!\n");
					#pragma message("TODO NSS: AI_IsOkToMoveStraight fails when it should not. For Now go into Wander mode.")
					AI_RemoveCurrentGoal( self );
					AI_AddNewGoal(self,GOALTYPE_WANDER);
                }
				return;
			}

			if ( AI_Move( self ) == FALSE )
			{
				PATHLIST_KillPath( hook->pPathList );
			}
			
			hook->nMoveCounter++;
		}
	}
	else
	{
		// on ground
        if ( bEnemyVisible && AI_IsWithinAttackDistance( self, fDistance ) )
		{
			AI_RemoveCurrentTask( self );
			return;
		}

		AI_OrientToFloor(self);
		float fXYDistance = VectorXYDistance( self->s.origin, pEnemy->s.origin );
		float fZDistance = VectorZDistance( self->s.origin, pEnemy->s.origin );
		CVector Destination,Dir,Mins,Maxs;
		Dir = pEnemy->s.origin - self->s.origin;
		Dir.Normalize();
		Mins = self->s.mins;
		Maxs = self->s.maxs;
		Mins.z = Maxs.z = 0.0f;
		Destination = self->s.origin + (VectorDistance(Mins,Maxs) * Dir);
		if ( AI_IsOkToMoveStraight( self, Destination, fXYDistance, fZDistance ) )
		{
			AI_MoveTowardPoint( self, pEnemy->s.origin, FALSE );
			hook->nMoveCounter = 0;
		}
		else
		{
			if ( AI_ShouldFollow( self ) )
			{
				if ( !AI_HandleUse( self ) )//<nss> marker.
				{
					// test to see if the last path point is close to the enemy's position
					if ( !AI_IsPathToEntityClose( self, pEnemy ) && 
                         !AI_FindPathToEntity( self, pEnemy ) && AI_CanPath( hook ) )
					{
			           // if ( AI_IsLineOfSight( self, pEnemy ) )
                       // {
                            // take brief cover
                            //AI_AddNewTaskAtFront( self, TASKTYPE_BRIEFCOVER, pEnemy );
                       // }
                       // else
                       // {
						if(!AI_MoveTowardPoint( self, pEnemy->s.origin, FALSE ))
						{
						// do not try another path for 5 seconds
						    AI_SetNextPathTime( hook, 5.0f );

                            float fXYDistance = VectorXYDistance( self->s.origin, pEnemy->s.origin );
                            float fZDistance = VectorZDistance( self->s.origin, pEnemy->s.origin );
                            if ( !AI_IsOkToMoveStraight( self, pEnemy->s.origin, fXYDistance, fZDistance ) )
                            {
                                AI_RemoveCurrentGoal( self );
                            }
							return;
						}

						
					}
					else
					if ( AI_Move( self ) == FALSE )
					{
						PATHLIST_KillPath( hook->pPathList );
					}
					
					hook->nMoveCounter++;
				}
			}
		}
	}

}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartFreeze
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
DllExport void AI_StartFreeze( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_Dprintf( "Starting TASKTYPE_FREEZE.\n" );

	AI_ZeroVelocity( self );

	if ( AI_StartSequence( self, "amba" ) == FALSE )
	{
		return;
	}

	AI_AddNewTaskAtFront( self, TASKTYPE_FREEZE );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetTaskFinishTime( hook, -1.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_Freeze
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Freeze( userEntity_t *self )
{
}

// ----------------------------------------------------------------------------
//
// Name:		AI_EndFreeze
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
DllExport void AI_EndFreeze( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FREEZE )
	{
		AI_Dprintf( "Ending TASKTYPE_FREEZE.\n" );
		AI_RemoveCurrentTask( self );
		return;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartPatrol
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartPatrol( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *target = NULL;

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_PATROL );

	if ( !self->target )
	{
		// no target, so find the nearest monster_path_corner
		target = com->FindClosestEntity( self, "monster_path_corner" );
		if ( !target )
		{
			GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
			GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
			if ( nGoalType == GOALTYPE_PATROL )
			{
				GOAL_Satisfied( pCurrentGoal );
			}

			AI_RemoveCurrentTask( self );
			return;
		}
	}
	else
	{
		// find the entity with the matching targetname
		target = com->FindTarget( self->target );
	}
	
	if ( !target )
	{
		com->Error( " AI_StartPatrol: Invalid Target!\n" );
	}

	AI_Dprintf( "Starting TASKTYPE_PATROL.\n" );

	self->target = target->targetname;

	AI_SetStateWalking( hook );
	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_PATROL )
		{
			return;
		}
	}

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );

	float fDistance = AI_ComputeDistanceToPoint( self, target->s.origin );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 2.0f;
	AI_SetTaskFinishTime( hook, fTime );
	AI_SetMovingCounter( hook, 0 );

}

// ----------------------------------------------------------------------------
//
// Name:		AI_FindNextPathCorner
// Description:
//			finds the closest monster_path_corner
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_FindNextPathCorner( userEntity_t *self )
{
	_ASSERTE( self );
	userEntity_t *pCurrentCorner = com->FindTarget( self->target );
	_ASSERTE( pCurrentCorner );
	mpathHook_t	 *pCornerHook = (mpathHook_t *)pCurrentCorner->userHook;
	_ASSERTE( pCornerHook );
	userEntity_t *pNextCorner = NULL;

	if ( pCurrentCorner )
	{
		ai_debug_print( self, "no corner\n" );
		return	FALSE;
	}
	if ( !pCornerHook )
	{
		ai_debug_print( self, "no hook\n" );
		return	FALSE;
	}

	// find the number of targets on the path corner that we have reached
	int nNumTargets = 0;
	for ( int i = 0; i < 4 ; i++ )
	{
		if ( pCornerHook->target[i] != NULL )
		{
			nNumTargets++;
		}
	}

	int nTargetIndex = 0;
	if ( nNumTargets > 1 )
	{
		nTargetIndex = (int)(Random() * (float)nNumTargets);
	}

	// find the entity with the matching target index
	pNextCorner = com->FindTarget( pCornerHook->target[nTargetIndex] );
	if ( !pNextCorner )
	{
		return FALSE;
	}

	self->target = pNextCorner->targetname;

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_Patrol
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Patrol( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	ai_wander_sound( self );

	userEntity_t *target = com->FindTarget( self->target );
	_ASSERTE( target );

	ai_frame_sounds( self );

	float fXYDistance = VectorXYDistance( self->s.origin, target->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, target->s.origin );
	if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
	{
		if ( !AI_FindNextPathCorner( self ) )
		{
			ai_debug_print( self, "ai_path_wander: couldn't find monster_path_corner\n" );

			GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
			GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
			GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
			if ( nGoalType == GOALTYPE_PATROL )
			{
				GOAL_Satisfied( pCurrentGoal );
			}

			AI_RemoveCurrentTask( self );
			return;
		}
	}

	target = com->FindTarget( self->target );
    if ( target )
    {
	    AI_MoveTowardPoint( self, target->s.origin, TRUE );

      	float fDistance = AI_ComputeDistanceToPoint( self, target->s.origin );
	    float fSpeed = AI_ComputeMovingSpeed( hook );
	    float fTime = (fDistance / fSpeed) + 2.0f;
	    AI_SetTaskFinishTime( hook, fTime );
    }
}


int AI_IsStateIdle( playerHook_t *hook )
{
	_ASSERTE( hook );
	return (hook->stateFlags & SFL_IDLE);
}
int AI_IsStateWalking( playerHook_t *hook )
{
	_ASSERTE( hook );
	return (hook->stateFlags & SFL_WALKING);
}
int AI_IsStateSlowWalking( playerHook_t *hook )
{
    return (hook->stateFlags & SFL_SLOWWALKING);
}
int AI_IsStateRunning( playerHook_t *hook )
{
	_ASSERTE( hook );
	return (hook->stateFlags & SFL_RUNNING);
}
int AI_IsStateAttacking( playerHook_t *hook )
{
	_ASSERTE( hook );
	return (hook->stateFlags & SFL_ATTACKING);
}
int AI_IsStateCrouching( playerHook_t *hook )
{
	_ASSERTE( hook );
	return (hook->stateFlags & SFL_CROUCHING);
}
int AI_IsStateMoving( playerHook_t *hook )
{
    return ( hook->stateFlags & (SFL_WALKING|SFL_SLOWWALKING|SFL_RUNNING|SFL_ATTACKING) );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_SetStateIdle
// Description:	sets hook->stateFlags so that walk sequences and walk speed will
//				be used
// Input:
// Output:
// Note:		
//
// ----------------------------------------------------------------------------
void AI_SetStateIdle( playerHook_t *hook )
{
	_ASSERTE( hook );

    if ( AI_IsStateCrouching( hook ) )
    {
        hook->stateFlags = SFL_IDLE | SFL_CROUCHING;
    }
    else
    {
        hook->stateFlags = SFL_IDLE;
    }
}

// ----------------------------------------------------------------------------
//
// Name:		AI_SetStateSlowWalking
// Description:	sets hook->stateFlags so that walk sequences and walk speed will
//				be used
// Input:
// Output:
// Note:		
//
// ----------------------------------------------------------------------------
void AI_SetStateSlowWalking( playerHook_t *hook )
{
	_ASSERTE( hook );

    if ( AI_IsStateCrouching( hook ) )
    {
    	hook->stateFlags = SFL_SLOWWALKING | SFL_CROUCHING;
    }
    else
    {
        hook->stateFlags = SFL_SLOWWALKING;
    }

	hook->pMovingAnimation = NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_SetStateWalking
// Description:	sets hook->stateFlags so that walk sequences and walk speed will
//				be used
// Input:
// Output:
// Note:		
//
// ----------------------------------------------------------------------------
void AI_SetStateWalking( playerHook_t *hook )
{
	_ASSERTE( hook );

    if ( AI_IsStateCrouching( hook ) )
    {
    	hook->stateFlags = SFL_WALKING | SFL_CROUCHING;
    }
    else
    {
        hook->stateFlags = SFL_WALKING;
    }

	hook->pMovingAnimation = NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_SetStateAttacking
// Description:	setAI_SetStateAttackings hook->stateFlags so that run sequences and attack speed will
//				be used
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_SetStateAttacking( playerHook_t *hook )
{
	_ASSERTE( hook );

    if ( AI_IsStateCrouching( hook ) )
    {
    	hook->stateFlags = SFL_ATTACKING | SFL_CROUCHING;
    }
    else
    {
        hook->stateFlags = SFL_ATTACKING;
    }
	hook->pMovingAnimation = NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_SetStateRunning
// Description:	sets hook->stateFlags so that run sequences and run speed will
//				be used
// Input:
// Output:
// Note:		
//
// ----------------------------------------------------------------------------
void AI_SetStateRunning( playerHook_t *hook )
{
	_ASSERTE( hook );

    if ( AI_IsStateCrouching( hook ) )
    {
    	hook->stateFlags = SFL_RUNNING | SFL_CROUCHING;
    }
    else
    {
        hook->stateFlags = SFL_RUNNING;
    }

	hook->pMovingAnimation = NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_SetStateCrouching
// Description:	
// Input:
// Output:
// Note:		
//
// ----------------------------------------------------------------------------
void AI_SetStateCrouching( playerHook_t *hook )
{
	_ASSERTE( hook );

	hook->stateFlags |= SFL_CROUCHING;

	hook->pMovingAnimation = NULL;
}

void AI_ClearStateCrouching( playerHook_t *hook )
{
    _ASSERTE( hook  );

    hook->stateFlags &= ~SFL_CROUCHING;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_MoveWhileFollowing
// Description:	
// Input:
// Output:
// Note:		
//
// ----------------------------------------------------------------------------
__inline static int AI_MoveWhileFollowing( userEntity_t *self, userEntity_t *pTarget )
{
	_ASSERTE( self );
    _ASSERTE( pTarget );
	playerHook_t *hook = AI_GetPlayerHook( self );

	float fXYDistance = VectorXYDistance( self->s.origin, pTarget->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pTarget->s.origin );

	// NSS[2/23/00]:If we have an enemy (which will reset our path) and we are trying to follow the player
	// then let's get a more valid path to the player.
	if(AI_IsSidekick(hook))
	{
		if ( !AI_FindPathToEntity( self, pTarget ) && AI_CanPath( hook ) )

		{
			// do not try another path for 2 seconds
			AI_SetNextPathTime( hook, 2.0f );
            if ( !AI_IsOkToMoveStraight( self, pTarget->s.origin, fXYDistance, fZDistance ) )
            {
                GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
                GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
                GOAL_ClearTasks( pCurrentGoal );
                AI_AddNewTaskAtFront( self, TASKTYPE_IDLE );
            }

			return FALSE;
		}
	}
	
	if ( ( hook->pPathList != NULL ) && hook->pPathList->nPathLength == 0 &&
         AI_IsLineOfSight(self, pTarget) && 
		 AI_IsOkToMoveStraight( self, pTarget->s.origin, fXYDistance, fZDistance ) )
	{
		AI_MoveTowardPoint( self, pTarget->s.origin, FALSE );
		hook->nMoveCounter = 0;
	}
	else
	{
		if ( !AI_HandleUse( self ) )
		{
			if ( !AI_IsPathToEntityClose( self, pTarget ) )
			{
				if ( !AI_FindPathToEntity( self, pTarget ) && AI_CanPath( hook ) )
				{
					// do not try another path for 2 seconds
					AI_SetNextPathTime( hook, 2.0f );
                    if ( !AI_IsOkToMoveStraight( self, pTarget->s.origin, fXYDistance, fZDistance ) )
                    {
                    	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
                        GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
                        GOAL_ClearTasks( pCurrentGoal );
                        AI_AddNewTaskAtFront( self, TASKTYPE_IDLE );
                    }

					return FALSE;
				}
			}

			AI_Move( self );

			hook->nMoveCounter++;
		}
	}

    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartFollowWalking
// Description:
// Input:
// Output:
// Note:	walk == TRUE starts walking frames
//
// ----------------------------------------------------------------------------
void AI_StartFollowWalking( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *pOwner = hook->owner;
	if ( !pOwner )
	{
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		GOAL_Satisfied( pCurrentGoal );
		AI_RemoveCurrentGoal( self );
		return;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
//	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FOLLOWWALKING );

	float fXYDistance = VectorXYDistance( self->s.origin, pOwner->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pOwner->s.origin );
	if ( (fXYDistance > AI_GetRunFollowDistance(hook) || fZDistance > MAX_JUMP_HEIGHT) && !AI_IsCrouching(hook) )
		//	far away, so start running towards owner
	{
		AI_RemoveCurrentTask( self, TASKTYPE_FOLLOWRUNNING, pOwner );
		return;
	}
	if ( fXYDistance < AI_GetWalkFollowDistance(hook) && fZDistance < MAX_JUMP_HEIGHT )
	{
		//	go back to TASKTYPE_IDLE
		AI_RemoveCurrentTask(self);
        SIDEKICK_ResetAmbientTimes( self );

		return;
	}
	CVector Origin;
	Origin = pOwner->s.origin;

	if ( !AI_FindPathToEntity( self, pOwner, FALSE ))
	{
		SIDEKICK_FindClearSpotFrom( self, pOwner->s.origin , Origin );
	}


    if( !AI_IsOkToMoveStraight( self,Origin, fXYDistance, fZDistance ) && !AI_FindPathToPoint( self,Origin) && !AI_FindPathToEntity( self, pOwner, FALSE ))
    {
        GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
        GOAL_ClearTasks( pCurrentGoal );
        AI_AddNewTask( self, TASKTYPE_IDLE );
        SIDEKICK_ResetAmbientTimes( self );
        return;
    }

	if ( AI_HandleUse( self ) )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_FOLLOWWALKING )
		{
			return;
		}
	}

	AI_SetStateWalking( hook );
	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_FOLLOWWALKING )
		{
			return;
		}
	}

	AI_Dprintf( "Starting TASKTYPE_FOLLOWWALKING.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, -1.0f );
	AI_SetMovingCounter( hook, 0 );

	AI_FindPathToEntity( self, pOwner, FALSE );
    AI_FollowWalking( self );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_FollowWalking
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_FollowWalking( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	// NSS[12/20/99]:Trap for non-started walking animations
	if((!AI_IsStateWalking(hook) || !strstr( hook->cur_sequence->animation_name, "walk" )) && AI_IsSidekick(hook))
	{
		AI_SetStateWalking(hook);
		if(!SIDEKICK_StartAnimation(self, "walk"))
			return;
	}	
	
	// NSS[12/20/99]:Don't do this too much
	if ( hook->nMoveCounter > 8)
	{
		AI_RemoveCurrentTask(self,FALSE);
		return;
	}
 
	userEntity_t *pOwner = hook->owner;
	if ( !pOwner || !AI_IsAlive(pOwner))
	{
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		GOAL_Satisfied( pCurrentGoal );
		AI_RemoveCurrentGoal( self );
		return;
	}
	if ( !pOwner->groundEntity && pOwner->waterlevel < 3 )
	{
		CVector Next;
		Next	= pOwner->s.origin;
		Next.z	+= pOwner->s.mins.z - 16.0f;
		tr = gstate->TraceLine_q2( pOwner->s.origin, Next, pOwner, MASK_SOLID );
		if(tr.fraction >= 1.0f)
		{
			AI_AddNewTaskAtFront(self,TASKTYPE_SIDEKICK_STOP,0.50f);
			AI_ZeroVelocity(self);
			return;
		}
	}

	float fXYDistance = VectorXYDistance( self->s.origin, pOwner->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pOwner->s.origin );
	if ( (fXYDistance > AI_GetRunFollowDistance(hook) || fZDistance > MAX_JUMP_HEIGHT) && !AI_IsCrouching(hook))
	{
		AI_RemoveCurrentTask( self, TASKTYPE_FOLLOWRUNNING, pOwner );
		return;
	}

	if ( fXYDistance < AI_GetWalkFollowDistance(hook) && 
         fZDistance < MAX_JUMP_HEIGHT && 
         AI_IsVisible( self, pOwner ) )
	{
		//	go back to TASKTYPE_IDLE
		AI_RemoveCurrentTask(self);
        SIDEKICK_ResetAmbientTimes( self );

		return;
	}
	else
	{
		if ( !AI_MoveWhileFollowing( self, pOwner ) )
        {
            return;
        }
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartFollowRunning
// Description:
// Input:
// Output:
// Note:	walk == TRUE starts walking frames
//
// ----------------------------------------------------------------------------
void AI_StartFollowRunning( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *pOwner = hook->owner;
	if ( !pOwner )
	{
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		GOAL_Satisfied( pCurrentGoal );
		AI_RemoveCurrentGoal( self );
		return;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	//_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FOLLOWRUNNING );

	float fXYDistance = VectorXYDistance( self->s.origin, pOwner->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pOwner->s.origin );
	if ( fXYDistance < AI_GetRunFollowDistance(hook) && fZDistance < MAX_JUMP_HEIGHT )
	{
		//	close enough to start walking towards owner
		AI_RemoveCurrentTask( self, TASKTYPE_FOLLOWWALKING, pOwner );
		return;
	}
	if ( fXYDistance < AI_GetWalkFollowDistance(hook) && fZDistance < MAX_JUMP_HEIGHT )
	{
		//	go back to TASKTYPE_IDLE
		AI_RemoveCurrentTask(self);
        SIDEKICK_ResetAmbientTimes( self );

		return;
	}

	if ( !AI_FindPathToEntity( self, pOwner, FALSE ) &&
         !AI_IsOkToMoveStraight( self, pOwner->s.origin, fXYDistance, fZDistance ) )
	{
        GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
        GOAL_ClearTasks( pCurrentGoal );
        if(AI_IsSidekick(hook))
		{
			// NSS[2/25/00]:Chill out for 3 seconds.
			AI_AddNewTaskAtFront( self, TASKTYPE_SIDEKICK_STOP, 1.0f );
			return;
		}
		else
		{
			AI_AddNewTaskAtFront( self, TASKTYPE_WANDER );
		}
        AI_StartWander(self);
		SIDEKICK_ResetAmbientTimes( self );

        return;
    }
	if ( AI_HandleUse( self ) )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_FOLLOWRUNNING )
		{
			return;
		}
	}

	AI_SetStateRunning( hook );
	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_FOLLOWRUNNING )
		{
			return;
		}
	}

	AI_Dprintf( "Starting TASKTYPE_FOLLOWRUNNING.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, -1.0f );
	AI_SetMovingCounter( hook, 0 );

	AI_FindPathToEntity( self, pOwner, FALSE );
	AI_FollowRunning( self );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_FollowRunning
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_FollowRunning( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	// NSS[12/20/99]:Trap for non-started running animations
	if(!AI_IsStateRunning(hook) || AI_IsSidekick(hook))
	{
		AI_SetStateRunning(hook);
		if(self->waterlevel > 2)
		{
			if(!strstr( hook->cur_sequence->animation_name, "swim" ))
			{
				if(!SIDEKICK_StartAnimation(self, "swim"))
					return;
			}
		}
		else if(!strstr( hook->cur_sequence->animation_name, "walk" ))
		{
			if(!SIDEKICK_StartAnimation(self, "run"))
				return;	
		}
	}	
	
	// NSS[1/4/00]:
	if(AI_IsEndAnimation(self))
	{
		AI_ForceSequence(self,hook->cur_sequence,FRAME_LOOP);
	}

	userEntity_t *pOwner = hook->owner;
	if ( !pOwner )
	{
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		GOAL_Satisfied( pCurrentGoal );
		AI_RemoveCurrentGoal( self );
		return;
	}
	if ( !pOwner->groundEntity && pOwner->waterlevel < 3 )
	{
		CVector Next;
		Next	= pOwner->s.origin;
		Next.z	+= pOwner->s.mins.z - 16.0f;
		tr = gstate->TraceLine_q2( pOwner->s.origin, Next, pOwner, MASK_SOLID );
		if(tr.fraction >= 1.0f)
		{
			AI_AddNewTaskAtFront(self,TASKTYPE_SIDEKICK_STOP,0.50f);
			AI_ZeroVelocity(self);
			return;
		}
	}

	float fXYDistance = VectorXYDistance( self->s.origin, pOwner->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pOwner->s.origin );
	if ( fXYDistance < AI_GetRunFollowDistance(hook) && fZDistance < MAX_JUMP_HEIGHT )
	{
		//	close enough to start walking towards owner
		AI_RemoveCurrentTask( self, TASKTYPE_FOLLOWSIDEKICKWALKING, pOwner );
		return;
	}

//	AI_FaceTowardPoint( self, pOwner->s.origin );

	if ( fXYDistance < AI_GetWalkFollowDistance(hook) && 
         fZDistance < MAX_JUMP_HEIGHT &&
         AI_IsVisible( self, pOwner ) )
	{
		//	go back to TASKTYPE_IDLE
		AI_RemoveCurrentTask(self);
        SIDEKICK_ResetAmbientTimes( self );
		return;
	}
	else
	{
         if ( !AI_MoveWhileFollowing( self, pOwner ) )
        {
            return;
        }
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartFollowSidekickWalking
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartFollowSidekickWalking( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FOLLOWSIDEKICKWALKING );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	_ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	userEntity_t *pSidekick = pAIData->pEntity;
	if ( !AI_IsAlive( pSidekick ) )
	{
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		GOAL_Satisfied( pCurrentGoal );
		AI_RemoveCurrentGoal( self );
		return;
	}

	float fRunFollowDistance = (AI_GetRunFollowDistance(hook) * 0.5f);
    float fWalkFollowDistance = (AI_GetWalkFollowDistance(hook) * 0.5f);

    // first check the owner (player) that this entity is following
	userEntity_t *pOwner = hook->owner;     // owner of a sidekick should be the player
	if ( pOwner )
	{
        float fXYDistance = VectorXYDistance( self->s.origin, pOwner->s.origin );
	    float fZDistance = VectorZDistance( self->s.origin, pOwner->s.origin );
	    if ( fXYDistance < fWalkFollowDistance && fZDistance < MAX_JUMP_HEIGHT )
	    {
		    //	go back to TASKTYPE_IDLE
		    AI_RemoveCurrentTask(self);
            SIDEKICK_ResetAmbientTimes( self );

		    return;
	    }
    }
    else
    {
        AI_RemoveCurrentTask( self );
    }

	// check distance with the sidekick that this entity is following
    float fXYDistance = VectorXYDistance( self->s.origin, pSidekick->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pSidekick->s.origin );
	if ( (fXYDistance > fRunFollowDistance || fZDistance > MAX_JUMP_HEIGHT) && !AI_IsCrouching(hook) )
	{
		AI_RemoveCurrentTask( self, TASKTYPE_FOLLOWRUNNING, pSidekick );
		return;
	}
	// NSS[2/21/00]:Test to see how they react when letting them go past this point
	/*if ( fXYDistance < fWalkFollowDistance && fZDistance < MAX_JUMP_HEIGHT )
	{
		//	go back to TASKTYPE_IDLE
		AI_RemoveCurrentTask(self);
        SIDEKICK_ResetAmbientTimes( self );

		return;
	}*/

	if ( !AI_FindPathToEntity( self, pSidekick, FALSE ) &&
         !AI_IsOkToMoveStraight( self, pSidekick->s.origin, fXYDistance, fZDistance ) )
	{
        GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
        GOAL_ClearTasks( pCurrentGoal );
        AI_AddNewTaskAtFront( self, TASKTYPE_IDLE );
        SIDEKICK_ResetAmbientTimes( self );

        return;
    }
	if ( AI_HandleUse( self ) )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_FOLLOWSIDEKICKWALKING )
		{
			return;
		}
	}

	AI_SetStateWalking( hook );
	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_FOLLOWSIDEKICKWALKING )
		{
			return;
		}
	}

	AI_Dprintf( "Starting TASKTYPE_FOLLOWSIDEKICKWALKING.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, -1.0f );
	AI_SetMovingCounter( hook, 0 );

	AI_FindPathToEntity( self, pSidekick, FALSE );
    AI_FollowSidekickWalking( self );

}

// ----------------------------------------------------------------------------
//
// Name:		AI_FollowSidekickWalking
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_FollowSidekickWalking( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if(!AI_IsAlive(self))
		return;

	// NSS[12/20/99]:Trap for non-started walking animations
	if(!AI_IsStateWalking(hook) ||  AI_IsSidekick(hook))
	{

		AI_SetStateWalking(hook);
		if(self->waterlevel > 2)
		{
			if(!strstr( hook->cur_sequence->animation_name, "swim" ))
			{
				if(!SIDEKICK_StartAnimation(self, "swim"))
					return;
			}
		}
		else if(!strstr( hook->cur_sequence->animation_name, "walk" ))
		{
			if(!SIDEKICK_StartAnimation(self, "walk"))
				return;	
		}
	}	

	// NSS[1/4/00]:
	if(AI_IsEndAnimation(self))
	{
		AI_ForceSequence(self,hook->cur_sequence,FRAME_LOOP);
	}


    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
//	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FOLLOWSIDEKICKWALKING );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	_ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );

	userEntity_t *pSidekick = pAIData->pEntity;
	if ( !AI_IsAlive( pSidekick ) )
	{
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		GOAL_Satisfied( pCurrentGoal );
		AI_RemoveCurrentGoal( self );
		return;
	}
	if ( !self->groundEntity && pSidekick->waterlevel < 3 )
	{
		return;
	}

	float fRunFollowDistance = (AI_GetRunFollowDistance(hook) * 0.5f);
    float fWalkFollowDistance = (AI_GetWalkFollowDistance(hook));

    // first check the owner (player) that this entity is following
	userEntity_t *pOwner = hook->owner;     // owner of a sidekick should be the player
	if ( pOwner )
	{
		// owner is moving, so follow him
		float fXYDistanceToOwner = VectorXYDistance( self->s.origin, pOwner->s.origin );
        float fZDistanceToOwner = VectorZDistance( self->s.origin, pOwner->s.origin );
        float fXYDistanceToSidekick = VectorXYDistance( self->s.origin, pSidekick->s.origin );
//        float fZDistanceToSidekick = VectorZDistance( self->s.origin, pSidekick->s.origin );// SCG[1/23/00]: not used
        if ( fXYDistanceToOwner < fXYDistanceToSidekick && fZDistanceToOwner < MAX_JUMP_HEIGHT )
        {
		    if ( fXYDistanceToOwner > fRunFollowDistance )
		    {
			    AI_RemoveCurrentTask( self, TASKTYPE_FOLLOWRUNNING, pOwner );
			    return;
		    }
		    else 
		    if ( fXYDistanceToOwner > fWalkFollowDistance )
		    {
			    AI_RemoveCurrentTask( self, TASKTYPE_FOLLOWWALKING, pOwner );
			    return;
		    }
        }

	    if ( fXYDistanceToOwner < fWalkFollowDistance && fZDistanceToOwner < MAX_JUMP_HEIGHT )
	    {
		    //	go back to TASKTYPE_IDLE
		    AI_RemoveCurrentTask( self );
            SIDEKICK_ResetAmbientTimes( self );
		    return;
	    }
    }
    else
    {
        AI_RemoveCurrentTask( self );
    }

    // now check distance with the sidekick that this entity is following
    float fXYDistance = VectorXYDistance( self->s.origin, pSidekick->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pSidekick->s.origin );
	if ( fXYDistance > fRunFollowDistance || fZDistance > MAX_JUMP_HEIGHT )
	{
		AI_RemoveCurrentTask( self, TASKTYPE_FOLLOWSIDEKICKRUNNING, pSidekick );
		return;
	}

//	AI_FaceTowardPoint( self, pSidekick->s.origin );

	if ( fXYDistance < fWalkFollowDistance && fZDistance < MAX_JUMP_HEIGHT )
	{
		userEntity_t *pOwner = hook->owner;     // owner of a sidekick should be the player
		if ( pOwner )
		{
	        float fDistance = VectorDistance( self->s.origin, pOwner->s.origin );
	        if ( fDistance > fRunFollowDistance )
	        {
		        CVector point;
                if ( SIDEKICK_FindNearUnobstructedPoint( self, point ) )
                {
                    AI_RemoveCurrentTask( self, TASKTYPE_MOVETOLOCATION, point );
                }
                else
                {
                    AI_RemoveCurrentTask( self );
                }
	        }
	        else 
	        if ( fDistance > fWalkFollowDistance )
	        {
		        CVector point;
                if ( SIDEKICK_FindNearUnobstructedPoint( self, point ) )
                {
                    AI_RemoveCurrentTask( self, TASKTYPE_WALKTOLOCATION, point );
                }
                else
                {
                    AI_RemoveCurrentTask( self );
                }
	        }
            else
            {
                // go back to idle
                AI_RemoveCurrentTask( self );
                SIDEKICK_ResetAmbientTimes( self );
            }
        }
        else
        {
    	    //	go back to TASKTYPE_IDLE
		    AI_RemoveCurrentTask( self );
            SIDEKICK_ResetAmbientTimes( self );
        }

		return;
	}
	else if(self->groundEntity) // NSS[2/6/00]:If we actually are touching the ground.
	{
		if ( !AI_MoveWhileFollowing( self, pSidekick ) )
        {
            return;
        }
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartFollowSidekickRunning
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartFollowSidekickRunning( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FOLLOWSIDEKICKRUNNING );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	_ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	userEntity_t *pSidekick = pAIData->pEntity;
	if ( !AI_IsAlive( pSidekick ) )
	{
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		GOAL_Satisfied( pCurrentGoal );
		AI_RemoveCurrentGoal( self );
		return;
	}

	float fRunFollowDistance = (AI_GetRunFollowDistance(hook) * 0.5f);
    float fWalkFollowDistance = (AI_GetWalkFollowDistance(hook) * 0.5f);

    // first check the owner (player) that this entity is following
	userEntity_t *pOwner = hook->owner;     // owner of a sidekick should be the player
	if ( pOwner )
	{
        float fXYDistance = VectorXYDistance( self->s.origin, pOwner->s.origin );
	    float fZDistance = VectorZDistance( self->s.origin, pOwner->s.origin );
	    if ( fXYDistance < fRunFollowDistance && fZDistance < MAX_JUMP_HEIGHT )
	    {
		    //	close enough to start walking towards owner
		    AI_RemoveCurrentTask( self, TASKTYPE_FOLLOWWALKING, pOwner );
		    return;
	    }
        else
	    if ( fXYDistance < fWalkFollowDistance && fZDistance < MAX_JUMP_HEIGHT )
	    {
		    //	go back to TASKTYPE_IDLE
		    AI_RemoveCurrentTask(self);
            SIDEKICK_ResetAmbientTimes( self );
		    return;
	    }
    }
    else
    {
        AI_RemoveCurrentTask( self );
    }

	// check the sidekick that this entity is following
    float fXYDistance = VectorXYDistance( self->s.origin, pSidekick->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pSidekick->s.origin );
	if ( fXYDistance < fRunFollowDistance && fZDistance < MAX_JUMP_HEIGHT )
	{
		//	close enough to start walking towards owner
		AI_RemoveCurrentTask( self, TASKTYPE_FOLLOWSIDEKICKWALKING, pSidekick );
		return;
	}
    else
	if ( fXYDistance < fWalkFollowDistance && fZDistance < MAX_JUMP_HEIGHT )
	{
		//	go back to TASKTYPE_IDLE
		AI_RemoveCurrentTask(self);
        SIDEKICK_ResetAmbientTimes( self );
		return;
	}

	if ( !AI_FindPathToEntity( self, pSidekick, FALSE ) &&
         !AI_IsOkToMoveStraight( self, pSidekick->s.origin, fXYDistance, fZDistance ) )
	{
        GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
        GOAL_ClearTasks( pCurrentGoal );
        AI_AddNewTaskAtFront( self, TASKTYPE_IDLE );
        SIDEKICK_ResetAmbientTimes( self );

        return;
    }
	if ( AI_HandleUse( self ) )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_FOLLOWSIDEKICKRUNNING )
		{
			return;
		}
	}

	AI_SetStateRunning( hook );
	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_FOLLOWSIDEKICKRUNNING )
		{
			return;
		}
	}

	AI_Dprintf( "Starting TASKTYPE_FOLLOWSIDEKICKRUNNING.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, -1.0f );
	AI_SetMovingCounter( hook, 0 );

	AI_FindPathToEntity( self, pSidekick, FALSE );
    AI_FollowSidekickRunning( self );

}

// ----------------------------------------------------------------------------
//
// Name:		AI_FollowSidekickRunning
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_FollowSidekickRunning( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	// NSS[12/20/99]:Trap for non-started running animations
	if((!AI_IsStateRunning(hook) || !strstr( hook->cur_sequence->animation_name, "run" )) && AI_IsSidekick(hook))
	{
		AI_SetStateRunning(hook);
		if(!SIDEKICK_StartAnimation(self, "run"))
			return;
	}	

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	_ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	userEntity_t *pSidekick = pAIData->pEntity;
	if ( !AI_IsAlive( pSidekick ) )
	{
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		GOAL_Satisfied( pCurrentGoal );
		AI_RemoveCurrentGoal( self );
		return;
	}
	if ( !pSidekick->groundEntity && pSidekick->waterlevel < 3 )
	{
		return;
	}

	float fRunFollowDistance = (AI_GetRunFollowDistance(hook) * 0.5f);
    float fWalkFollowDistance = (AI_GetWalkFollowDistance(hook) * 0.5f);

    // now check the owner (player) that this entity is following
	userEntity_t *pOwner = hook->owner;     // owner of a sidekick should be the player
	if ( pOwner )
	{
		// owner is moving, so follow him
		float fXYDistanceToOwner = VectorXYDistance( self->s.origin, pOwner->s.origin );
        float fZDistanceToOwner = VectorZDistance( self->s.origin, pOwner->s.origin );
        float fXYDistanceToSidekick = VectorXYDistance( self->s.origin, pSidekick->s.origin );
//        float fZDistanceToSidekick = VectorZDistance( self->s.origin, pSidekick->s.origin );// SCG[1/23/00]: not used
        if ( fXYDistanceToOwner < fXYDistanceToSidekick && fZDistanceToOwner < MAX_JUMP_HEIGHT )
        {
		    if ( fXYDistanceToOwner > fRunFollowDistance )
		    {
			    AI_RemoveCurrentTask( self, TASKTYPE_FOLLOWRUNNING, pOwner );
			    return;
		    }
		    else 
		    if ( fXYDistanceToOwner > fWalkFollowDistance )
		    {
			    AI_RemoveCurrentTask( self, TASKTYPE_FOLLOWWALKING, pOwner );
			    return;
		    }
        }

	    if ( fXYDistanceToOwner < fRunFollowDistance && fZDistanceToOwner < MAX_JUMP_HEIGHT )
	    {
		    //	close enough to start walking towards owner
		    AI_RemoveCurrentTask( self, TASKTYPE_FOLLOWWALKING, pOwner );
		    return;
	    }
        else
	    if ( fXYDistanceToOwner < fWalkFollowDistance && fZDistanceToOwner < MAX_JUMP_HEIGHT )
	    {
		    //	go back to TASKTYPE_IDLE
		    AI_RemoveCurrentTask(self);
            SIDEKICK_ResetAmbientTimes( self );
		    return;
	    }
    }
    else
    {
        AI_RemoveCurrentTask( self );
    }

    // now check for the sidekick this entity is following
	float fXYDistance = VectorXYDistance( self->s.origin, pSidekick->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pSidekick->s.origin );
	if ( fXYDistance < fRunFollowDistance && fZDistance < MAX_JUMP_HEIGHT )
	{
		//	close enough to start walking towards owner
		AI_RemoveCurrentTask( self, TASKTYPE_FOLLOWSIDEKICKWALKING, pSidekick );
		return;
	}

//	AI_FaceTowardPoint( self, pSidekick->s.origin );

	if ( fXYDistance < fWalkFollowDistance && fZDistance < MAX_JUMP_HEIGHT )
	{
		//	go back to TASKTYPE_IDLE
		AI_RemoveCurrentTask(self);
        SIDEKICK_ResetAmbientTimes( self );
		return;
	}
	else if(self->groundEntity || self->waterlevel > 2) // NSS[2/6/00]:If we actually are touching the ground. And if we are not in water
	{
		
		
		if ( !AI_MoveWhileFollowing( self, pSidekick ) )
        {
            return;
        }
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_FindPathNextWanderNode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR AI_FindPathNextWanderNode( userEntity_t *self, NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pCurrentNode )
{
	_ASSERTE( self );
	_ASSERTE( pNodeHeader );

	if ( !pCurrentNode || pCurrentNode->nNumLinks == 0 )
	{
		return NULL;
	}

	playerHook_t *hook = AI_GetPlayerHook( self );
	NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );

	// find a random node that is within a certain distance from the starting position
	int nNumLinks = 0;
	int aNodeIndices[MAX_NODE_LINKS];
	//if(pCurrentNode->nNumLinks >= 2)
	//{
		for ( int i = 0; i < pCurrentNode->nNumLinks; i++ )
		{
			MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, pCurrentNode->aLinks[i].nIndex );

			if(pNode)
			{
				float fDistanceFromStartPosition = VectorDistance( hook->startPosition, pNode->position );

				// choose a node that is in the field of view (~180 degrees)
				CVector vector, angles;
				vector = pNode->position - self->s.origin;
				VectorToAngles( vector, angles );
				float fAngleDiff = fabs(self->s.angles.yaw - angles.yaw);

				float fXYDistance = VectorXYDistance( self->s.origin, pNode->position );
				float fZDistance = VectorZDistance( self->s.origin, pNode->position );

				if ( !(AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f) &&
					 (fDistanceFromStartPosition < hook->active_distance) && 
					 (fAngleDiff <= 90.0f || fAngleDiff >= 270.0f) )
				{
					aNodeIndices[nNumLinks] = pCurrentNode->aLinks[i].nIndex;
					nNumLinks++;
				}
			}
		}
	//}
	//else
	//{
	//	nNumLinks = pCurrentNode->nNumLinks - 1;
	//}
	MAPNODE_PTR pDestNode = NULL;
	if ( nNumLinks > 0 )
	{
   		int nTargetNodeIndex;
		//if(pCurrentNode->nNumLinks >= 2 && nNumLinks > 1)
		nTargetNodeIndex = aNodeIndices[(rand() % nNumLinks)];
		//else
		//	nTargetNodeIndex = pCurrentNode->aLinks[(rand() % nNumLinks)].nIndex;

		pDestNode = NODE_GetNode( pNodeHeader, nTargetNodeIndex );
		_ASSERTE( pDestNode );
	}
	else
	{
		if ( pCurrentNode->nNumLinks > 0 )
		{
   			int nLinkIndex = (rand() % pCurrentNode->nNumLinks);

			pDestNode = NODE_GetNode( pNodeHeader, pCurrentNode->aLinks[nLinkIndex].nIndex );
			if(!pDestNode)
				return NULL;
		}
	}

	// get rid of old path
	PATHLIST_KillPath(hook->pPathList);
	PATHLIST_AddNodeToPath(hook->pPathList, pDestNode);

	return pDestNode;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartWander
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartWander( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
//	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_WANDER );

	if(AI_IsSidekick(hook))
	{
		AI_RemoveCurrentTask(self,FALSE);
		return;
	}

	AI_SetStateWalking( hook );
	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_WANDER )
		{
			return;
		}
	}

	// update the current node for this entity
	NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	MAPNODE_PTR pCurrentNode = NODE_GetClosestNode(self);
	if ( !pCurrentNode )
	{
		if ( pNodeList->nCurrentNodeIndex != -1 )
		{
			pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
		}
	}
	if ( !pCurrentNode )
	{
		
		AI_Dprintf("No closest node was found while trying to start a wander for %s at %s\n", self->className, com->vtos(self->s.origin));

		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
		if ( nGoalType == GOALTYPE_WANDER || nGoalType == GOALTYPE_RANDOMWANDER )
		{
			AI_RemoveCurrentGoal( self );
		}
        else
        {
            AI_RemoveCurrentTask( self );
        }
        return;
	}
    _ASSERTE( pCurrentNode );

	MAPNODE_PTR pNode = AI_FindPathNextWanderNode( self, pNodeHeader, pCurrentNode );
	if ( !pNode )
	{
		AI_Dprintf( "Did not find the next wander node!!!!.\n" );

		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
		if ( nGoalType == GOALTYPE_WANDER )
		{
			GOAL_Satisfied( pCurrentGoal );
		}
		AI_RemoveCurrentTask( self );
		return;
	}
    PATHLIST_KillPath( hook->pPathList );
	PATHLIST_AddNodeToPath( hook->pPathList, pNode );

	AI_Dprintf( "Starting TASKTYPE_WANDER.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );

	float fDistance = VectorDistance( self->s.origin, pNode->position );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 1.0f;
	AI_SetTaskFinishTime( hook, fTime );

	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_Wander
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Wander( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	frameData_t *pSequence;
	char szAnimation[16];

	if(AI_IsSidekick(hook))
	{
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		if(pGoalStack->pTopOfStack->nGoalType == GOALTYPE_WANDER)
		{
			AI_RemoveCurrentGoal(self);
		}
		else
		{
			AI_RemoveCurrentTask(self,FALSE);
		}
		return;
	}


	ai_wander_sound( self );
	// NSS[2/16/00]:Cleaned this crap up... 
	if(AI_IsEndAnimation(self))
	{
		if(hook && AI_IsGroundUnit(self))
		{
			AI_SetStateWalking(hook);
			AI_SelectWalkingAnimation( self, szAnimation );
			pSequence = FRAMES_GetSequence( self, szAnimation );
			if( pSequence != NULL)
			{
				AI_ForceSequence(self, pSequence, FRAME_LOOP );
			}
		}
		else if(hook && AI_IsWaterUnit(self))
		{
			pSequence = FRAMES_GetSequence( self, "swima" );
			if( pSequence != NULL)
			{
				AI_ForceSequence(self, pSequence, FRAME_LOOP );
			}
		}
		else if(hook && AI_IsFlyingUnit(self))
		{
			pSequence = FRAMES_GetSequence( self, "flya" );
			if( pSequence != NULL)
			{
				AI_ForceSequence(self, pSequence, FRAME_LOOP );
			}
		}
	}

	if( hook->pPathList == NULL )
	{
		AI_RemoveCurrentTask( self );
	}
	else if ( hook->pPathList->pPath == NULL )
	{
		NODELIST_PTR pNodeList = hook->pNodeList;
		_ASSERTE( pNodeList );
		NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
		_ASSERTE( pNodeHeader );

        MAPNODE_PTR pDestNode = NULL;
        MAPNODE_PTR pClosestNode = NODE_GetClosestNode(self);
        if ( pClosestNode )
        {
		    pDestNode = AI_FindPathNextWanderNode( self, pNodeHeader, pClosestNode );
        }

		if ( !pDestNode )
		{
			AI_Dprintf( "Did not find the next wander node!!!!.\n" );

			GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
			GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
			GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
			if ( nGoalType == GOALTYPE_WANDER )
			{
				GOAL_Satisfied( pCurrentGoal );
			}

			AI_RemoveCurrentTask( self );
			return;
		}

		PATHLIST_AddNodeToPath( hook->pPathList, pDestNode );

	    float fDistance = VectorDistance( self->s.origin, pDestNode->position );
	    float fSpeed = AI_ComputeMovingSpeed( hook );
	    float fTime = (fDistance / fSpeed) + 1.0f;
	    AI_SetTaskFinishTime( hook, fTime );
        AI_Move( self );
	}
	else
	{
		AI_Move( self );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartMoveToLocation
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartMoveToLocation( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_MOVETOLOCATION );

	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_MOVETOLOCATION )
		{
			return;
		}
	}

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	_ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	AI_Dprintf( "Starting TASKTYPE_MOVETOLOCATION.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fDistance = AI_ComputeDistanceToPoint( self, pAIData->destPoint );
	float fTime = (fDistance / fSpeed) + 3.0f;
	AI_SetTaskFinishTime( hook, fTime );
	AI_SetMovingCounter( hook, 0 );
	//AI_Drop_Markers(pAIData->destPoint,5.0f);
}

// ----------------------------------------------------------------------------
//
// Name:		AI_MoveToLocation
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_MoveToLocation( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	if(AI_IsEndAnimation(self))
	{
		if(hook)
		{
			if(hook->cur_sequence && (strstr(hook->cur_sequence->animation_name,"walk")||strstr(hook->cur_sequence->animation_name,"run")))
			{
				AI_ForceSequence(self,hook->cur_sequence,FRAME_LOOP);
			}
		}	
	}

	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	if ( (AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f))
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		if ( (GOAL_GetType( pCurrentGoal ) == GOALTYPE_MOVETOLOCATION || GOAL_GetType( pCurrentGoal ) == GOALTYPE_GENERALUSE)&&
			 GOAL_GetNumTasks( pCurrentGoal ) == 1 )
		{
			AIDATA_PTR pAIData = GOAL_GetData( pCurrentGoal );

			float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
			float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
			if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
			{
				// NSS[12/9/99]:In case there is anything we have not used but should use.
				AI_HandleUse( self );
				GOAL_Satisfied( pCurrentGoal );
			}
		}
		self->velocity.Zero();
		AI_RemoveCurrentTask( self );
		return;
	}
	//if(AI_IsSidekick(hook))
	//	AI_Drop_Markers(pAIData->destPoint, 0.1);
	if ( AI_IsOkToMoveStraight( self, pAIData->destPoint, fXYDistance, fZDistance ) )
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
					// NSS[2/21/00]:If our goal is to move to a point that has no path to it then let's not.
					if(hook->pGoals && hook->pGoals->pTopOfStack && hook->pGoals->pTopOfStack->nGoalType == GOALTYPE_MOVETOLOCATION)
					{
						AI_RemoveCurrentGoal( self );
					}
					else
					{
						AI_RestartCurrentGoal( self );
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartWalkToLocation
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartWalkToLocation( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_WALKTOLOCATION );

	AI_SetStateWalking( hook );
    if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_WALKTOLOCATION )
		{
			return;
		}
	}

	AI_Dprintf( "Starting TASKTYPE_WALKTOLOCATION.\n" );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	_ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	float fDistance = AI_ComputeDistanceToPoint( self, pAIData->destPoint );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 2.0f;
	AI_SetTaskFinishTime( hook, fTime );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_WalkToLocation
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_WalkToLocation( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	if ( AI_IsOkToMoveStraight( self, pAIData->destPoint, fXYDistance, fZDistance ) )
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
// Name:		AI_StartMoveToExactLocation
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartMoveToExactLocation( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_MOVETOEXACTLOCATION );

	if ( AI_StartMove( self ) == FALSE )
	{
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_MOVETOEXACTLOCATION.\n" );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	_ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	if(pAIData->pEntity)
		pAIData->destPoint = pAIData->pEntity->s.origin;
	float fDistance = AI_ComputeDistanceToPoint( self, pAIData->destPoint );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 2.0f;
	AI_SetTaskFinishTime( hook, fTime );
	AI_SetMovingCounter( hook, 0 );
}
int SIDEKICK_HasPickedUpItem( userEntity_t *self, int UseIt);
// ----------------------------------------------------------------------------
// NSS[2/7/00]:
// Name:		AI_MoveToExactLocation
// Description:Slowly approaches the exact location.
// Input:userEntity *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void AI_MoveToExactLocation( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	// NSS[1/28/00]:For Sidekicks picking shit up.
	if(self->flags & FL_BOT)
	{
		if(SIDEKICK_HasPickedUpItem(self,0))
		{
			return;
		}
	}

	if ( AI_IsExactDistance( hook, fXYDistance ) && fZDistance < 32.0f )
	{
	    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	    if ( GOAL_GetType( pCurrentGoal ) == GOALTYPE_MOVETOEXACTLOCATION &&
		     GOAL_GetNumTasks( pCurrentGoal ) == 1 )
	    {
		    AIDATA_PTR pAIData = GOAL_GetData( pCurrentGoal );

		    float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
		    float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
		    if ( AI_IsExactDistance( hook, fXYDistance ) && fZDistance < 32.0f )
		    {
			    GOAL_Satisfied( pCurrentGoal );
		    }
	    }
		if(self->flags & FL_BOT)
		{
			SIDEKICK_HasPickedUpItem(self,1);
		}
		AI_RemoveCurrentTask( self );
		
		pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
		
		while(pCurrentTask->nTaskType == TASKTYPE_MOVETOEXACTLOCATION)
		{
			AI_RemoveCurrentTask( self );
			pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
		}
		return;
	}
	if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
	{
		char szAnimation[16];
		float fSpeed = fXYDistance * 10.0f;

		//	scale speed based on current frame's move_scale
		float fScale = FRAMES_ComputeFrameScale( hook );

		forward = pAIData->destPoint - self->s.origin;
		forward.Normalize();
		//AI_SetVelocity( self, forward, (fSpeed * fScale) );
		if(fScale < 32.0f)
		{
			if(!strstr(hook->cur_sequence->animation_name,"walk"))
			{
			   AI_SelectWalkingAnimation( self, szAnimation );
				frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
				if( pSequence != NULL)
				{
					AI_ForceSequence(self, szAnimation, pSequence->flags );
				}
			}
			fScale = 32.0f;
		}
		AI_FaceTowardPoint(self,pAIData->destPoint);
		AI_SetVelocity( self, forward, fScale);

		hook->last_origin = self->s.origin;

		AI_UpdateCurrentNode( self );

		ai_frame_sounds( self );
		return;
	}
// NSS[1/27/00]:
/*	if(!AI_MoveTowardExactPoint( self, pAIData->destPoint, FALSE ))
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
*/
	if ( AI_IsOkToMoveStraight( self, pAIData->destPoint, fXYDistance, fZDistance ) )
	{
		AI_MoveTowardExactPoint( self, pAIData->destPoint, FALSE );
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
// Name:		AI_StartFlyToLocation
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartFlyToLocation( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FLYTOLOCATION );

	if ( AI_StartFly( self ) == FALSE )
    {
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_FLYTOLOCATION )
		{
			return;
		}
    }

	AI_Dprintf( "Starting TASKTYPE_FLYTOLOCATION.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
    _ASSERTE( pAIData );

	float fDistance = AI_ComputeDistanceToPoint( self, pAIData->destPoint );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 2.0f;
	AI_SetTaskFinishTime( hook, fTime );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_FlyToLocation
// Description:
//				move using the node system
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_FlyToLocation( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	float fDistance = VectorDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsCloseDistance2(self, fDistance ) )
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		if ( GOAL_GetType( pCurrentGoal ) == GOALTYPE_FLYTOLOCATION &&
			 GOAL_GetNumTasks( pCurrentGoal ) == 1 )
		{
			AIDATA_PTR pAIData = GOAL_GetData( pCurrentGoal );

			float fDistance = VectorDistance( self->s.origin, pAIData->destPoint );
			if ( AI_IsCloseDistance2(self, fDistance ) )
			{
				GOAL_Satisfied( pCurrentGoal );
			}
		}

		AI_RemoveCurrentTask( self );
		return;
	}

	tr = gstate->TraceLine_q2( self->s.origin, pAIData->destPoint, self, MASK_SOLID );
	if ( tr.fraction == 1.0 )
	{
		AI_FlyTowardPoint( self, pAIData->destPoint, FALSE );
	}
	else
	{
		AI_Fly( self );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartMoveToEntity
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartMoveToEntity( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_MOVETOENTITY );

	AI_SetStateWalking( hook );
	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_MOVETOENTITY )
		{
			return;
		}
	}

	AI_Dprintf( "Starting TASKTYPE_MOVETOENTITY.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	userEntity_t *pDestEntity = pAIData->pEntity;
	_ASSERTE( pDestEntity );
	float fDistance = AI_ComputeDistanceToPoint( self, pDestEntity->s.origin );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 2.0f;
	AI_SetTaskFinishTime( hook, fTime );

	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_MoveToEntity
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_MoveToEntity( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	userEntity_t *pDestEntity = pAIData->pEntity;
	_ASSERTE( pDestEntity );
	
	CVector destPoint( pDestEntity->s.origin );
	if ( destPoint.Length() == 0 )
	{
		destPoint.x = (pDestEntity->absmax.x + pDestEntity->absmin.x) * 0.5f;
		destPoint.y = (pDestEntity->absmax.y + pDestEntity->absmin.y) * 0.5f;
		destPoint.z = (pDestEntity->absmax.z + pDestEntity->absmin.z) * 0.5f;
	}

	float fHalfSelfWidth = (self->s.maxs.x - self->s.mins.x) * 0.5f;
	float fHalfDestEntityWidth = (pDestEntity->s.maxs.x - pDestEntity->s.mins.x) * 0.5f; 

	float fOneFrameSpeed = AI_ComputeMovingSpeed( hook ) * 0.1f;
    float fDistance = VectorDistance( self->s.origin, destPoint );
	if ( fDistance < (fHalfSelfWidth + fHalfDestEntityWidth + fOneFrameSpeed + 32.0f) )
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		if ( GOAL_GetType( pCurrentGoal ) == GOALTYPE_MOVETOENTITY && 
			 GOAL_GetNumTasks( pCurrentGoal ) == 1 )
		{
			AIDATA_PTR pAIData = GOAL_GetData( pCurrentGoal );
			pDestEntity = pAIData->pEntity;
			_ASSERTE( pDestEntity );
			
			destPoint.Set( pDestEntity->s.origin );
			if ( destPoint.Length() == 0 )
			{
				destPoint.x = (pDestEntity->absmax.x + pDestEntity->absmin.x) * 0.5f;
				destPoint.y = (pDestEntity->absmax.y + pDestEntity->absmin.y) * 0.5f;
				destPoint.z = (pDestEntity->absmax.z + pDestEntity->absmin.z) * 0.5f;
			}

			fDistance = VectorDistance( self->s.origin, destPoint );
        	if ( fDistance < (fHalfSelfWidth + fHalfDestEntityWidth + fOneFrameSpeed + 32.0f) )
			{
				GOAL_Satisfied( pCurrentGoal );
			}
		}

		AI_RemoveCurrentTask( self );
		return;
	}

	float fXYDistance = VectorXYDistance( self->s.origin, destPoint );
	float fZDistance = VectorZDistance( self->s.origin, destPoint );
	if ( AI_IsOkToMoveStraight( self, destPoint, fXYDistance, fZDistance ) )
	{
		AI_MoveTowardPoint( self, destPoint, FALSE );
	}
	else
	{
		if ( hook->pPathList->pPath )
		{
			if ( AI_Move( self ) == FALSE )
			{
				if ( !AI_FindPathToPoint( self, destPoint ) )
				{
					AI_RestartCurrentGoal( self );
				}
			}
		}
		else
		{
			if ( !AI_FindPathToPoint( self, destPoint ) )
			{
				AI_RestartCurrentGoal( self );
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartMoveToOwner
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartMoveToOwner( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_MOVETOOWNER );

	userEntity_t *pOwner = hook->owner;
    
	if(!AI_IsAlive(pOwner))
	{
		AI_RemoveCurrentGoal(self);
		return;
	}
    
	if ( (pOwner->flags & FL_CLIENT) )
    {
        AI_SetStateRunning( hook );
    }
    else
    {
        AI_SetStateWalking( hook );
    }

	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_MOVETOOWNER )
		{
			return;
		}
	}

	AI_Dprintf( "Starting TASKTYPE_MOVETOOWNER.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );

	float fDistance = AI_ComputeDistanceToPoint( self, pOwner->s.origin );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 2.0f;
	AI_SetTaskFinishTime( hook, fTime );

	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_MoveToOwner
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_MoveToOwner( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	AI_GetCurrentGoalStack( hook );// SCG[1/23/00]: pGoalStack not used

	userEntity_t *pOwner = hook->owner;
    _ASSERTE( AI_IsAlive(pOwner) );

	float fXYDistance = VectorXYDistance( self->s.origin, pOwner->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pOwner->s.origin );
    if ( fXYDistance < AI_GetRunFollowDistance(hook) )
    {
		AI_RemoveCurrentTask( self );
		return;
    }

	if ( AI_IsOkToMoveStraight( self, pOwner->s.origin, fXYDistance, fZDistance ) )
	{
		AI_MoveTowardPoint( self, pOwner->s.origin, FALSE );
	}
	else
	{
		if ( hook->pPathList->pPath )
		{
			if ( AI_Move( self ) == FALSE )
			{
				if ( !AI_FindPathToEntity( self, pOwner ) && AI_CanPath( hook ) )
				{
                    if ( !AI_IsOkToMoveStraight( self, pOwner->s.origin, fXYDistance, fZDistance ) )
                    {
                        AI_RemoveCurrentGoal( self );
                    }
                    else
                    {
    				    AI_RestartCurrentGoal( self );
                    }
				}
			}
		}
		else
		{
			if ( !AI_FindPathToEntity( self, pOwner ) && AI_CanPath( hook ) )
			{
                if ( !AI_IsOkToMoveStraight( self, pOwner->s.origin, fXYDistance, fZDistance ) )
                {
                    AI_RemoveCurrentGoal( self );
                }
                else
                {
				    AI_RestartCurrentGoal( self );
                }
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartGoingAroundObstacle
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartGoingAroundObstacle( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_GOINGAROUNDOBSTACLE );

	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_GOINGAROUNDOBSTACLE )
		{
			return;
		}
	}

	if ( !hook->pPathList->pPath )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
		AI_FindPathToPoint( self, pAIData->destPoint );
		if ( !hook->pPathList->pPath )
		{
			AI_RestartCurrentGoal( self );
			return;
		}
	}

	float fDistance = 0;
	CVector currentPos( self->s.origin );

	NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( self );
	PATHNODE_PTR pPath = hook->pPathList->pPath;
	while ( pPath )
	{
		MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, pPath->nNodeIndex );
		fDistance += VectorXYDistance( currentPos, pNode->position );
		
		currentPos.Set( pNode->position );

		pPath = pPath->next_node;
	}

	AI_Dprintf( "Starting TASKTYPE_GOINGAROUNDOBSTACLE.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );

	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 1.0f;
	AI_SetTaskFinishTime( hook, fTime );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_GoingAroundObstacle
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_GoingAroundObstacle( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );

	// if currently following something then, stop if close enough
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	if ( GOAL_GetType( pCurrentGoal ) == GOALTYPE_FOLLOW &&
		 fXYDistance < AI_GetWalkFollowDistance(hook) && fZDistance < MAX_JUMP_HEIGHT )
	{
		AI_RemoveCurrentTask( self );
		return;		
	}

	if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	if ( !AI_HandleUse( self ) )
	{
		if ( AI_Move( self ) == FALSE )
		{
			AI_RemoveCurrentTask( self );
			return;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartFlyingAroundObstacle
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartFlyingAroundObstacle( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FLYINGAROUNDOBSTACLE );

	if ( AI_StartFly( self ) == FALSE )
    {
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_FLYINGAROUNDOBSTACLE )
		{
			return;
		}
    }

	if ( !hook->pPathList->pPath )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
		AI_FindPathToPoint( self, pAIData->destPoint );
		if ( !hook->pPathList->pPath )
		{
			AI_RestartCurrentGoal( self );
            return;
		}
	}

	AI_Dprintf( "Starting TASKTYPE_FLYINGAROUNDOBSTACLE.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );

	float fDistance = 0.0f;
    CVector currentPos = self->s.origin;
	NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( self );
	PATHNODE_PTR pPath = hook->pPathList->pPath;
	while ( pPath )
	{
		MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, pPath->nNodeIndex );
		fDistance += VectorDistance( currentPos, pNode->position );
		
		currentPos = pNode->position;

		pPath = pPath->next_node;
	}
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 2.0f;
	AI_SetTaskFinishTime( hook, fTime );

	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_FlyingAroundObstacle
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_FlyingAroundObstacle( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	if ( !AI_HandleUse( self ) )
	{
		if ( AI_Fly( self ) == FALSE )
		{
			AI_RemoveCurrentTask( self );
			return;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartSwoopAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartSwoopAttack( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_SWOOPATTACK );

	AI_SetStateAttacking( hook );
	if ( AI_StartFly( self ) == FALSE )
    {
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_SWOOPATTACK )
		{
			return;
		}
    }

	AI_Dprintf( "Starting TASKTYPE_SWOOPATTACK.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );

	AI_SetTaskFinishTime( hook, 20.0f );
	AI_SetMovingCounter( hook, 0 );
}


// ----------------------------------------------------------------------------
//
// Name:		AI_ComputeIntermediatePoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define INTERMEDIATE_POINT_DISTANCE		600.0f

__inline static
int AI_IsClearToPoint( userEntity_t *self, CVector &startPoint, CVector &directionVector, 
					   float fDistance, CVector &destPoint )
{
	_ASSERTE( self );

	VectorMA( startPoint, directionVector, fDistance, destPoint );

	tr = gstate->TraceBox_q2( startPoint, self->s.mins, self->s.maxs, destPoint, self, MASK_SOLID );
	return tr.fraction;
}

__inline static
int AI_IsEnoughRoomToTurn( userEntity_t *self, CVector &facingDir, CVector &destPoint, float fTurningDiameter )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	int nWhichSideClear = 0;

	CVector tempPoint;
	// check left side for enough room for turning
	CVector leftOrthogonalVector( -facingDir.y, facingDir.x, 0.0f );
	if ( AI_IsClearToPoint( self, destPoint, leftOrthogonalVector, fTurningDiameter, tempPoint ) >= 0.6f )
	{
		nWhichSideClear = LEFT_SIDE_CLEAR;	
	}

	CVector rightOrthogonalVector( facingDir.y, -facingDir.x, 0.0f );
	if ( AI_IsClearToPoint( self, destPoint, rightOrthogonalVector, fTurningDiameter, tempPoint ) >= 0.6f )
	{
		nWhichSideClear |= RIGHT_SIDE_CLEAR;
	}

	if ( (nWhichSideClear & LEFT_SIDE_CLEAR) && (nWhichSideClear & RIGHT_SIDE_CLEAR) )
	{
		return TRUE;
	}

	return FALSE;
}

void AI_ComputeIntermediatePoint( userEntity_t *self, CVector &intermediatePoint )
{
	_ASSERTE( self );

	CVector angleVector( -30, self->s.angles.yaw, self->s.angles.roll );

	float fSavedYaw = self->s.angles.yaw;

	float fTurningDiameter = AI_ComputeTurningDiameter( self );
	float fTurningRadius = fTurningDiameter * 0.5f;
	float fCheckDistance = INTERMEDIATE_POINT_DISTANCE + fTurningRadius;

	CVector facingDir;
	AngleToVectors( angleVector, facingDir );
	facingDir.Normalize();

	CVector destPoint;
	if ( AI_IsClearToPoint( self, self->s.origin, facingDir, fCheckDistance, destPoint ) == 1.0 )
	{
		if ( AI_IsEnoughRoomToTurn( self, facingDir, destPoint, fTurningDiameter ) == TRUE )
		{
			VectorMA( self->s.origin, facingDir, INTERMEDIATE_POINT_DISTANCE, intermediatePoint );
			return;
		}
	}

	angleVector.y = AngleMod( fSavedYaw + 45.0f );
	AngleToVectors( angleVector, facingDir );
	facingDir.Normalize();
	if ( AI_IsClearToPoint( self, self->s.origin, facingDir, fCheckDistance, destPoint ) == 1.0 )
	{
		if ( AI_IsEnoughRoomToTurn( self, facingDir, destPoint, fTurningDiameter ) == TRUE )
		{
			VectorMA( self->s.origin, facingDir, INTERMEDIATE_POINT_DISTANCE, intermediatePoint );
			return;
		}
	}

	angleVector.y = AngleMod( fSavedYaw - 45.0f );
	AngleToVectors( angleVector, facingDir );
	facingDir.Normalize();
	if ( AI_IsClearToPoint( self, self->s.origin, facingDir, fCheckDistance, destPoint ) == 1.0 )
	{
		if ( AI_IsEnoughRoomToTurn( self, facingDir, destPoint, fTurningDiameter ) == TRUE )
		{
			VectorMA( self->s.origin, facingDir, INTERMEDIATE_POINT_DISTANCE, intermediatePoint );
			return;
		}
	}

	angleVector.y = AngleMod( fSavedYaw + 90.0f );
	AngleToVectors( angleVector, facingDir );
	facingDir.Normalize();
	if ( AI_IsClearToPoint( self, self->s.origin, facingDir, fCheckDistance, destPoint ) == 1.0 )
	{
		if ( AI_IsEnoughRoomToTurn( self, facingDir, destPoint, fTurningDiameter ) == TRUE )
		{
			VectorMA( self->s.origin, facingDir, INTERMEDIATE_POINT_DISTANCE, intermediatePoint );
			return;
		}
	}

	angleVector.y = AngleMod( fSavedYaw - 90.0f );
	AngleToVectors( angleVector, facingDir );
	facingDir.Normalize();
	if ( AI_IsClearToPoint( self, self->s.origin, facingDir, fCheckDistance, destPoint ) == 1.0 )
	{
		if ( AI_IsEnoughRoomToTurn( self, facingDir, destPoint, fTurningDiameter ) == TRUE )
		{
			VectorMA( self->s.origin, facingDir, INTERMEDIATE_POINT_DISTANCE, intermediatePoint );
			return;
		}
	}

	angleVector.y = AngleMod( fSavedYaw + 180.0f );
	AngleToVectors( angleVector, facingDir );
	facingDir.Normalize();
	if ( AI_IsClearToPoint( self, self->s.origin, facingDir, fCheckDistance, destPoint ) == 1.0 )
	{
		if ( AI_IsEnoughRoomToTurn( self, facingDir, destPoint, fTurningDiameter ) == TRUE )
		{
			VectorMA( self->s.origin, facingDir, INTERMEDIATE_POINT_DISTANCE, intermediatePoint );
			return;
		}
	}

/*
	int FAILED_TO_FIND_INTERMEDIATE_POINT = FALSE;
	_ASSERTE( FAILED_TO_FIND_INTERMEDIATE_POINT );
*/

	AI_Dprintf( "FAILED TO FIND INTERMEDIATE POINT.\n" );
}

void AI_ComputeSecondIntermediatePoint( userEntity_t *self, CVector &intermediatePoint )
{
	_ASSERTE( self );

	CVector vectorTowardPoint;
	CVector angleTowardPoint;
	vectorTowardPoint = self->enemy->s.origin - self->s.origin;
	vectorTowardPoint.Normalize();
	VectorToAngles( vectorTowardPoint, angleTowardPoint );
	
	angleTowardPoint.x += 5.0f;

	int nRandom = rand() % (STRAFE_DOWNRIGHT+1);
	switch ( nRandom )
	{
		case STRAFE_LEFT:
		{
			angleTowardPoint.y += 60.0f;
			break;
		}
		case STRAFE_RIGHT:
		{
			angleTowardPoint.y -= 60.0f;
			break;
		}
		case STRAFE_UPLEFT:
		case STRAFE_DOWNLEFT:
		{
			angleTowardPoint.x += 20.0f;
			angleTowardPoint.y += 60.0f;
			break;
		}
		case STRAFE_UPRIGHT:
		case STRAFE_DOWNRIGHT:
		{
			angleTowardPoint.x += 20.0f;
			angleTowardPoint.y -= 60.0f;
			break;
		}
	}
	
	angleTowardPoint.y = AngleMod( angleTowardPoint.y );
	
	CVector facingDir, right, up;
	angleTowardPoint.AngleToVectors( facingDir, right, up );
	facingDir.Normalize();

	float fHalfDistance = INTERMEDIATE_POINT_DISTANCE * 0.5f;
	VectorMA( self->s.origin, facingDir, fHalfDistance, intermediatePoint );

	tr = gstate->TraceLine_q2( self->s.origin, intermediatePoint, self, MASK_SOLID );
	if ( tr.fraction < 1.0f )
	{
		float fDistance = (fHalfDistance * tr.fraction) - 16.0f;
		VectorMA( self->s.origin, facingDir, fDistance, intermediatePoint );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_SwoopAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_SwoopAttack( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	CVector attackerPoint( self->s.origin.x, self->s.origin.y, self->s.origin.z + self->s.mins.z );

	//float fZOffset = fabs(self->s.mins.z);
	CVector attackeePoint( self->enemy->s.origin.x, self->enemy->s.origin.y, self->enemy->s.origin.z + self->enemy->s.maxs.z );

	float fDistance = VectorDistance( attackerPoint, attackeePoint );
	if ( fDistance > hook->active_distance )
	{
		// the enemy is out of range, give up chasing
		AI_RemoveCurrentGoal( self );
		return;
	}
	else
	{
		if ( AI_IsWithinAttackDistance( self, fDistance ) )
		{
			// attack the enemy, should be a "punch"
			ai_fire_curWeapon( self );

			// compute the intermediate point and
			CVector intermediatePoint;
			AI_ComputeIntermediatePoint( self, intermediatePoint );
			AI_AddNewTaskAtFront( self, TASKTYPE_TOWARDINTERMEDIATEPOINT, intermediatePoint );
            return;
		}
		else
		{
			if ( AI_IsVisible(self, self->enemy) )
			{
                AI_SetTaskFinishTime( hook, 20.0f );
				AI_FlyTowardPoint( self, attackeePoint, FALSE );
			}
			else
			{
				AI_Fly( self );
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartAirRangedAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartAirRangedAttack( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_AIRRANGEDATTACK );

	AI_SetStateAttacking( hook );
	if ( AI_StartFly( self ) == FALSE )
    {
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_AIRRANGEDATTACK )
		{
			return;
		}
    }

	AI_Dprintf( "Starting TASKTYPE_AIRRANGEDATTACK.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 15.0f );
	AI_SetMovingCounter( hook, 0 );

}

// ----------------------------------------------------------------------------
//
// Name:		AI_AirRangedAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_AirRangedAttack( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *pEnemy = self->enemy;
    _ASSERTE( pEnemy );

    CVector attackerPoint = self->s.origin;
	CVector attackeePoint = pEnemy->s.origin;

	float fDistance = VectorDistance( attackerPoint, attackeePoint );
	if ( fDistance > hook->active_distance )
	{
		// the enemy is out of range, give up chasing
		AI_RemoveCurrentGoal( self );
		return;
	}
	else
	{
		if ( AI_IsVisible(self, pEnemy) )
		{
			if ( AI_IsWithinAttackDistance( self, fDistance ) )
			{
				GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
				GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
				AIDATA_PTR pAIData = GOAL_GetData(pCurrentGoal);
				AI_AddNewTaskAtFront( self, TASKTYPE_ATTACK, pAIData->pEntity );

				return;
			}
			else
			{
                AI_SetTaskFinishTime( hook, 15.0f );
				AI_FlyTowardPoint( self, attackeePoint, FALSE );
			}

			hook->nMoveCounter = 0;
		}
		else
		{
			if ( !AI_IsPathToEntityClose( self, pEnemy ) )
			{
				if ( !AI_FindPathToEntity( self, pEnemy ) && AI_CanPath( hook ) )
				{
					// do not try another path for 5 seconds
					AI_SetNextPathTime( hook, 5.0f );

					//	no path was found, so give up
					AI_RemoveCurrentGoal( self );

					return;
				}
			}

			if ( AI_Fly( self ) == FALSE )
			{
				PATHLIST_KillPath( hook->pPathList );
			}
			
			hook->nMoveCounter++;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartAirMeleeAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartAirMeleeAttack( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_AIRMELEEATTACK );

	AI_SetStateAttacking( hook );
	if ( AI_StartFly( self ) == FALSE )
    {
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_AIRMELEEATTACK )
		{
			return;
		}
    }

	AI_Dprintf( "Starting TASKTYPE_AIRMELEEATTACK.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 15.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AirMeleeAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_AirMeleeAttack( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	CVector attackerPoint( self->s.origin.x, self->s.origin.y, self->s.origin.z + self->s.mins.z );
	float fZOffset = fabs(self->s.mins.z);

	CVector attackeePoint( self->enemy->s.origin.x, self->enemy->s.origin.y, self->enemy->s.origin.z + self->enemy->s.maxs.z + fZOffset );

	float fDistance = VectorDistance( attackerPoint, attackeePoint );
	if ( fDistance > hook->active_distance )
	{
		// the enemy is out of range, give up chasing
		AI_RemoveCurrentGoal( self );
		return;
	}
	else
	{
		if ( AI_IsVisible(self, self->enemy) )
		{
			if ( AI_IsWithinAttackDistance( self, fDistance ) )
			{
				AI_ZeroVelocity( self );

				GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
				GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
				AIDATA_PTR pAIData = GOAL_GetData(pCurrentGoal);
				AI_AddNewTaskAtFront( self, TASKTYPE_ATTACK, pAIData->pEntity );
                return;
			}
			else
			{
                AI_SetTaskFinishTime( hook, 15.0f );
				if ( (hook->type == TYPE_SL_SKEET) || AI_HandleEvasiveActionDuringChase( self ) == FALSE )
				{
					AI_FlyTowardPoint( self, attackeePoint, FALSE );
				}
			}

			hook->nMoveCounter = 0;
		}
		else
		{
			if ( !AI_IsPathToEntityClose( self, self->enemy ) )
			{
				if ( !AI_FindPathToEntity( self, self->enemy ) && AI_CanPath( hook ) )
				{
					// do not try another path for 5 seconds
					AI_SetNextPathTime( hook, 5.0f );

					//	no path was found, so give up
					AI_RemoveCurrentGoal (self);

					return;
				}
			}

			if ( AI_Fly( self ) == FALSE )
			{
				PATHLIST_KillPath( hook->pPathList );
			}
			
			hook->nMoveCounter++;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartTowardIntermediatePoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartTowardIntermediatePoint( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_TOWARDINTERMEDIATEPOINT );

	if ( AI_StartFly( self ) == FALSE )
    {
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_TOWARDINTERMEDIATEPOINT )
		{
			return;
		}
    }

	AI_Dprintf( "Starting TASKTYPE_TOWARDINTERMEDIATEPOINT.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );
    _ASSERTE( pAIData );
    float fDistance = VectorDistance( self->s.origin, pAIData->destPoint );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 2.0f;
	AI_SetTaskFinishTime( hook, fTime );

	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_TowardIntermediatePoint
// Description:
//				move using the node system
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_TowardIntermediatePoint( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );
	
	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
	{
		AI_RemoveCurrentTask( self );

		// make the AI unit move more interesting by having it move to some other
		if ( rnd() > 0.5f )
		{
			// intermediate point, rather than straight toward the enemy
			CVector intermediatePoint;
			AI_ComputeSecondIntermediatePoint( self, intermediatePoint );
			AI_AddNewTaskAtFront( self, TASKTYPE_TOWARDINTERMEDIATEPOINT2, intermediatePoint );
		}

		return;
	}

	AI_FlyTowardPoint( self, pAIData->destPoint, FALSE );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartTowardIntermediatePoint2
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartTowardIntermediatePoint2( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_TOWARDINTERMEDIATEPOINT2 );

	if ( AI_StartFly( self ) == FALSE )
    {
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_TOWARDINTERMEDIATEPOINT2 )
		{
			return;
		}
    }

	AI_Dprintf( "Starting TASKTYPE_TOWARDINTERMEDIATEPOINT2.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );
    
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );
    _ASSERTE( pAIData );
    float fDistance = VectorDistance( self->s.origin, pAIData->destPoint );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 2.0f;
	AI_SetTaskFinishTime( hook, fTime );

	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_TowardIntermediatePoint2
// Description:
//				move using the node system
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_TowardIntermediatePoint2( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );
	
	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
	{
		AI_RemoveCurrentTask( self );

		return;
	}

	AI_FlyTowardPoint( self, pAIData->destPoint, FALSE );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartSwim
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_StartSwim( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    if ( AI_IsGroundUnit(self) )
    {
        AI_StartMoveForWalkingUnit( self );
    }
    else
    {
	    if ( AI_ForceSequence( self, "swima", FRAME_LOOP ) == FALSE )
	    {
		    AI_ForceSequence( self, "walka", FRAME_LOOP );
	    }
    }

	hook->nMoveCounter = 0;

	return TRUE;
}
void HandleAirObstructions2(userEntity_t *self, CVector &Dest);
// ----------------------------------------------------------------------------
//
// Name:		AI_SwimTowardPoint
// Description:
//				move straight toward a point
// Input:
// Output:
//				TRUE	=> if reached the point
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int AI_SwimTowardPoint( userEntity_t *self, CVector &destPoint, int bMovingTowardNode )
{
	_ASSERTE( self );

	// if this unit is going outside of its bounding box, then terminate current goal
	if ( AI_IsInsideBoundingBox( self ) == FALSE )
	{
		AI_RemoveCurrentGoal( self );
		return FALSE;
	}

	playerHook_t *hook = AI_GetPlayerHook( self );
	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return FALSE;
	}



	float fSpeed = AI_ComputeMovingSpeed( hook );
	fSpeed = hook->run_speed;

	float fDistance = VectorDistance( self->s.origin, destPoint );
	if ( AI_IsCloseDistance2(self, fDistance ) )
	{
		AI_UpdateCurrentNode( self );
		return TRUE;
	}
	
 	CVector vectorTowardPoint;
	CVector angleTowardPoint;
	vectorTowardPoint = destPoint - self->s.origin;
	vectorTowardPoint.Normalize();
	VectorToAngles( vectorTowardPoint, angleTowardPoint ); 


	if ( AI_IsGroundUnit(self) )
    {

		angleTowardPoint.AngleToForwardVector( forward );
        forward.Normalize();

		if ( bMovingTowardNode && self->waterlevel < 3 )
        {
            // must be stuck trying to go out of the water, try to jump
	        self->velocity.z = hook->upward_vel;
            AI_UpdateCurrentNode( self );
            return FALSE;
        }
    }
    else
    {
        CVector angleVector = self->s.angles;
	    CVector right, up;
	    if ( AI_IsSameAngle3D( self, angleVector, angleTowardPoint ) == TRUE )
	    {
		    forward = vectorTowardPoint;
	    }
	    else
	    {
		    angleVector.AngleToForwardVector( forward );
		    forward.Normalize();
	    }
    }

	float turnrate = 0.0005f * self->velocity.Length();
	if(hook->dflags & DFL_AMPHIBIOUS)
	{
		turnrate = 0.05 * self->velocity.Length();
	}
	AI_FlyTowardPoint2(self,destPoint, turnrate);
		
	if ( AI_IsWaterUnit(self) && self->waterlevel < 3 )
    {
		if(!(hook->dflags & DFL_AMPHIBIOUS))
		{
			forward.z = 0.0f;
		}
    }
	
/*	if ( AI_HandleAirCollisionWithEntities( self, destPoint, fSpeed ) )
	{
		return FALSE;
	}
    

	if ( AI_HandleAirObstacle( self, fSpeed, bMovingTowardNode ) )
	{
		//HandleAirObstructions2(self,forward);
		return FALSE;
	}


    forward.Normalize();

	if(hook->dflags&DFL_AMPHIBIOUS)
	{
		if(AI_HandleUse(self))
		{
			return TRUE;
		}
	}
	
	if ( AI_IsWaterUnit(self) && self->waterlevel < 3 )
    {
		if(!(hook->dflags & DFL_AMPHIBIOUS))
		{
			forward.z = 0.0f;
		}
    }	
	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
    AI_SetVelocity( self, forward, (fSpeed * fScale) );

	if ( AI_HandleAirUnitGettingStuck( self, destPoint, fSpeed ) )
	{
		return FALSE;
	}

	hook->last_origin = self->s.origin;

	ai_frame_sounds( self );*/

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_Swim
// Description:
//				move using the node system
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_Swim( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	if(!hook)
	{
		return FALSE;
	}

	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return FALSE;
	}



	if ( hook->pPathList->pPath && hook->pPathList->pPath->nNodeIndex >= 0 )
	{
		_ASSERTE( pGroundNodes );
		MAPNODE_PTR pNode = NODE_GetNode( pGroundNodes, hook->pPathList->pPath->nNodeIndex );

		if(hook->dflags&DFL_AMPHIBIOUS)
		{
			if(AI_HandleUse(self))
			{
				return TRUE;
			}
		}
		if ( AI_SwimTowardPoint( self, pNode->position, TRUE ) == TRUE )
		{
			// we're already at this node, so delete the first one
			PATHLIST_DeleteFirstInPath(hook->pPathList);
		}

		return TRUE;
	}
	else
	{
		hook->fNextPathTime = 0.0f;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartSwimToLocation
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartSwimToLocation( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_SWIMTOLOCATION );

	AI_Dprintf( "Starting TASKTYPE_SWIMTOLOCATION.\n" );

	AI_StartSwim( self );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
    _ASSERTE( pAIData );
	float fDistance = AI_ComputeDistanceToPoint( self, pAIData->destPoint );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 2.0f;
	AI_SetTaskFinishTime( hook, fTime );

	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_SwimToLocation
// Description:
//				move using the node system
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_SwimToLocation( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	float fDistance = VectorDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsCloseDistance2(self, fDistance ) )
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		if ( GOAL_GetType( pCurrentGoal ) == GOALTYPE_SWIMTOLOCATION && 
			 GOAL_GetNumTasks( pCurrentGoal ) == 1 )
		{
			GOAL_Satisfied( pCurrentGoal );
		}

		AI_RemoveCurrentTask( self );
		return;
	}

	tr = gstate->TraceLine_q2( self->s.origin, pAIData->destPoint, self, MASK_SOLID );
	if ( tr.fraction == 1.0 )
	{
		AI_SwimTowardPoint( self, pAIData->destPoint, FALSE );
	}
	else
	{
		AI_Swim( self );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartFaceAngle
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartFaceAngle( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FACEANGLE );

	char szAnimation[16];
	AI_SelectAmbientAnimation( self, szAnimation );
	AI_ForceSequence( self, szAnimation, FRAME_LOOP );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	self->ideal_ang = pAIData->destPoint;

	com->ChangeYaw(self);
	
	AI_SetNextThinkTime( self, 0.1f );

	AI_Dprintf( "Starting TASKTYPE_FACEANGLE.\n" );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, 5.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_FaceAngle
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_FaceAngle( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->dflags & DFL_ORIENTTOFLOOR )
	{
		AI_OrientToFloor( self );
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );

//	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	TASK_GetData(pCurrentTask);// SCG[1/23/00]: pAIData not used
	
	float yDiff = fabs(self->s.angles.yaw - self->ideal_ang.yaw);
	//float zDiff = fabs(self->s.angles.roll - self->ideal_ang.roll);

	float yMinDiff = self->ang_speed.yaw * 0.1f;
	//float zMinDiff = self->ang_speed.roll * 0.1f;

	int bSameAngle = FALSE;
	if ( hook->dflags & DFL_FACEPITCH )
	{
		float xDiff = fabs(self->s.angles.pitch - self->ideal_ang.pitch);
		float xMinDiff = self->ang_speed.pitch * 0.1f;
		if ( (xDiff <= xMinDiff || xDiff >= (360.0f-xMinDiff)) && 
			 (yDiff <= yMinDiff || yDiff >= (360.0f-yMinDiff))
		   )
		{
			bSameAngle = TRUE;
		}
	}
	else
	{
		if ( yDiff <= yMinDiff || yDiff >= (360.0f-yMinDiff) )
		{
			bSameAngle = TRUE;
		}
	}

	if ( bSameAngle )
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		if ( GOAL_GetType( pCurrentGoal ) == GOALTYPE_FACEANGLE )
		{
			GOAL_Satisfied( pCurrentGoal );
		}

		AI_RemoveCurrentTask( self );
		return;
	}
	
	com->ChangeYaw(self);
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartWait
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartWait( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_WAIT );

	char szAnimation[16];
	AI_SelectAmbientAnimation( self, szAnimation );
	AI_ForceSequence( self, szAnimation, FRAME_LOOP );

	AI_Dprintf( "Starting TASKTYPE_WAIT.\n" );

	AI_ZeroVelocity( self );

	AI_SetNextThinkTime( self, 0.1f );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, pAIData->fValue );
	AI_SetMovingCounter( hook, 0 );

}

// ----------------------------------------------------------------------------
//
// Name:		AI_Wait
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Wait( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );

//	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	TASK_GetData(pCurrentTask);// SCG[1/23/00]: pAIData not used

	if ( hook->fTaskFinishTime >= 0.0f && gstate->time > hook->fTaskFinishTime )
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		if ( GOAL_GetType( pCurrentGoal ) == GOALTYPE_WAIT )
		{
			GOAL_Satisfied( pCurrentGoal );
		}

		AI_RemoveCurrentTask( self );
		return;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartWaitForNoCollision
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartWaitForNoCollision( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	_ASSERTE( TASK_GetType( pCurrentTask ) == TASKTYPE_WAITFORNOCOLLISION );

	if(AI_IsGroundUnit(self) || AI_IsWaterUnit(self))
	{
		char szAnimation[16];
		AI_SelectAmbientAnimation( self, szAnimation );
		AI_ForceSequence( self, szAnimation, FRAME_LOOP );
	}

	AI_Dprintf( "Starting TASKTYPE_WAITFORNOCOLLISION.\n" );

	AI_ZeroVelocity( self );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, 1.0f + (1.0f * rnd()) );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_WaitForNoCollision
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_WaitForNoCollision( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->destPoint );

	forward = pAIData->destPoint - self->s.origin;
	forward.Normalize();

	// check collision with other entities
	float fHalfWidth = fabs(self->s.maxs.x - self->s.mins.x) * 0.5f;

	float fSpeed = AI_ComputeMovingSpeed( hook );

	// check collision with other entities
	float fOneFrameSpeed = (fSpeed * 0.125f);
	CVector checkPoint;
	VectorMA( self->s.origin, forward, fOneFrameSpeed + fHalfWidth, checkPoint );

	CVector mins = self->s.mins;
    mins.z += 16.0f;
    CVector maxs = self->s.maxs;

	hook->nMoveCounter ++;

    tr = gstate->TraceBox_q2( self->s.origin, mins, maxs, checkPoint, self, MASK_SOLID|CONTENTS_MONSTER );
	if ( tr.fraction >= 1.0f || _stricmp(tr.ent->className,"worldspawn") == 0)
	{
		AI_RemoveCurrentTask( self );
	}
    else
    {
        if(hook->nMoveCounter > 40)
		{
			AI_RemoveCurrentTask( self );
		}
		AI_SetTaskFinishTime( hook, 1.0f );
    }
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartStay
// Description:	starts NPC waiting at a location until owner gives them a new
//				goal
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartStay( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_STAY );

	char szAnimation[16];
	AI_SelectAmbientAnimation( self, szAnimation );
	if ( AI_StartSequence( self, szAnimation, FRAME_LOOP ) == FALSE )
	{
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_STAY.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_ZeroVelocity( self );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, -1.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_Stay
// Description:	NPC is waiting at a location until owner gives them a new
//				goal
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Stay( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	//	NPC will not break out of this task unless attacked (which should
	//	be handled in pain functions) or specifically given a new goal
	//	by their owner

    if ( AI_IsSidekick( hook ) )
    {
        // track owner
        SIDEKICK_TrackOwner( self );

        if(!AI_IsCrouching(hook))
			SIDEKICK_HandleAmbients( self );
    }

	return;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartBackupAllAttributes
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartBackupAllAttributes( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_BACKUPALLATTRIBUTES );

	AI_Dprintf( "Starting TASKTYPE_BACKUPALLATTRIBUTES.\n" );

	AI_BackupAllAttributes( self );

	// this function good for only once, so remove this goal
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	GOAL_Satisfied( pCurrentGoal );
	AI_RemoveCurrentGoal( self );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartRestoreAllAttributes
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartRestoreAllAttributes( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_RESTOREALLATTRIBUTES );

	AI_Dprintf( "Starting TASKTYPE_RESTOREALLATTRIBUTES.\n" );

	AI_RestoreAllAttributes( self );

	AI_SetTaskFinishTime( hook, -1.0f );

	// this function good for only once, so remove this goal
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	GOAL_Satisfied( pCurrentGoal );
	AI_RemoveCurrentGoal( self );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartModifyTurnAttribute
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartModifyTurnAttribute( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	AIDATA_PTR pAIData = GOAL_GetData(pCurrentGoal);
	AI_ModifyTurnAttribute( self, pAIData->destPoint );

	AI_Dprintf( "Starting TASKTYPE_MODIFYTURNATTRIBUTE.\n" );

	AI_SetTaskFinishTime( hook, -1.0f );

	// this function good for only once, so remove this goal
	GOAL_Satisfied( pCurrentGoal );
	AI_RemoveCurrentGoal( self );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartModifyRunAttribute
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartModifyRunAttribute( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	AIDATA_PTR pAIData = GOAL_GetData(pCurrentGoal);
	AI_ModifyRunAttribute( self, pAIData->fValue );

	AI_Dprintf( "Starting TASKTYPE_MODIFYRUNATTRIBUTE.\n" );

	// this function good for only once, so remove this goal
	GOAL_Satisfied( pCurrentGoal );
	AI_RemoveCurrentGoal( self );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartModifyWalkAttribute
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartModifyWalkAttribute( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	AIDATA_PTR pAIData = GOAL_GetData(pCurrentGoal);
	AI_ModifyWalkAttribute( self, pAIData->fValue );

	AI_Dprintf( "Starting TASKTYPE_MODIFYWALKATTRIBUTE.\n" );

	AI_SetTaskFinishTime( hook, -1.0f );

	// this function good for only once, so remove this goal
	GOAL_Satisfied( pCurrentGoal );
	AI_RemoveCurrentGoal( self );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_PlayAnimation
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_PlayAnimation( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

    // get the goal.
    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

    frameData_t *pSequence = pAIData->pAnimSequence;
    _ASSERTE( pSequence );
    if ( AI_IsEndAnimation(self) )
    {
        GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
        if ( nGoalType == GOALTYPE_PLAYANIMATION && GOAL_GetNumTasks( pCurrentGoal ) == 1 )
        {
            //remove this goal, we are done with it.
            GOAL_Satisfied( pCurrentGoal );
        }

        AI_RemoveCurrentTask( self );
    }
	self->velocity.Zero();
}

// ----------------------------------------------------------------------------
//
// Name:        AI_StartPlayAnimation
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartPlayAnimation( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    //make sure we have a play animation task in our goal.
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_PLAYANIMATION );

    // get the goal.
    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

    frameData_t *pSequence = pAIData->pAnimSequence;
    _ASSERTE( pSequence );

	// NSS[12/9/99]:FORCE the sequence
	AI_ForceSequence(self,pSequence->animation_name,FRAME_ONCE);
    AI_Dprintf( "Starting TASKTYPE_PLAYANIMATION.\n" );

	AI_SetOkToAttackFlag( hook, FALSE );

	float fTime = AI_ComputeAnimationTime( pSequence );
    AI_SetTaskFinishTime( hook, fTime + 1.0f );

	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_SetIdleAnimation
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_SetIdleAnimation( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

    // get the goal.
    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

    int bEndAnimation = FALSE;
    if ( hook->cur_sequence == pAIData->pAnimSequence )
    {
	    if ( self->s.frameInfo.frameState & (FRSTATE_STOPPED | FRSTATE_LAST) )
	    {
            bEndAnimation = TRUE;	
        }
        
    }
    else
    {
        bEndAnimation = TRUE;
    }

    if ( bEndAnimation == TRUE )
    {
        GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
        if ( nGoalType == GOALTYPE_SETIDLEANIMATION && GOAL_GetNumTasks( pCurrentGoal ) == 1 )
        {
            //remove this goal, we are done with it.
            GOAL_Satisfied( pCurrentGoal );
        }

        AI_RemoveCurrentTask( self );
    }
}

// ----------------------------------------------------------------------------
//
// Name:        AI_StartSetIdleAnimation
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartSetIdleAnimation( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    //make sure we have a set idle animation task in our goal.
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SETIDLEANIMATION );

    //get the goal.
    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );

    //get the goal data.
    AIDATA_PTR pAIData = GOAL_GetData(pCurrentGoal);

    //set our idle animation info
    if ( AI_StartSequence( self, pAIData->pAnimSequence, FRAME_LOOP ) == FALSE )
	{
		return;
	}

    AI_Dprintf( "Starting TASKTYPE_SETIDLEANIMATION.\n" );

/*
	//AI_SetIdleAnimation(self, pAIData->animation.type, pAIData->animation.number);
*/
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartUsingWalkSpeed
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartUsingWalkSpeed( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_STARTUSINGWALKSPEED );

	gstate->Con_Dprintf( "Starting TASKTYPE_STARTUSINGWALKSPEED.\n" );

	AI_SetStateWalking( hook );

	// this function good for only once, so remove this goal
//	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	GOALSTACK_GetCurrentGoal( pGoalStack );// SCG[1/23/00]: pCurrentGoal not used
	AI_RemoveCurrentGoal( self );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartUsingRunSpeed
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartUsingRunSpeed( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_STARTUSINGRUNSPEED );

	gstate->Con_Dprintf( "Starting TASKTYPE_STARTUSINGRUNSPEED.\n" );

	AI_SetStateRunning( hook );

	// this function good for only once, so remove this goal
//	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	GOALSTACK_GetCurrentGoal( pGoalStack );// SCG[1/23/00]: pCurrentGoal not used
	AI_RemoveCurrentGoal( self );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartActivateSwitch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartActivateSwitch( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_ACTIVATESWITCH );

	gstate->Con_Dprintf( "Starting TASKTYPE_ACTIVATESWITCH.\n" );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	userEntity_t *pSwitch = pAIData->pEntity;
	_ASSERTE( pSwitch );
	pSwitch->use( pSwitch, self, self );

	CVector destPoint( pSwitch->s.origin );
	if ( destPoint.Length() == 0 )
	{
		destPoint.x = (pSwitch->absmax.x + pSwitch->absmin.x) * 0.5f;
		destPoint.y = (pSwitch->absmax.y + pSwitch->absmin.y) * 0.5f;
		destPoint.z = (pSwitch->absmax.z + pSwitch->absmin.z) * 0.5f;
	}
	AI_FaceTowardPoint( self, destPoint );
	
	AI_ZeroVelocity( self );

	// this function good for only once, so remove this task
	AI_RemoveCurrentTask( self );
}


// ----------------------------------------------------------------------------
//
// Name:		AI_StartWaitForTrainToCome
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartWaitForTrainToCome( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_WAITFORTRAINTOCOME );

	// make the entity just stand
	AI_ZeroVelocity( self );

	char szAnimation[16];
	AI_SelectAmbientAnimation( self, szAnimation );
	if ( AI_StartSequence( self, szAnimation, FRAME_LOOP ) == FALSE )
	{
		return;
	}

	gstate->Con_Dprintf( "Starting TASKTYPE_WAITFORTRAINTOCOME.\n" );

	hook->nWaitCounter = 0;

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, 20.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartWaitForTrainToCome
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_WaitForTrainToCome( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	int nNodeIndex = pAIData->nValue;

	MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, nNodeIndex );
	_ASSERTE( pNode && (pNode->node_type & NODETYPE_TRAIN) );

	AI_FaceTowardPoint( self, pNode->position );

	userEntity_t *pTrain = com->FindNodeTarget( NODE_GetTarget(pNode) );
	_ASSERTE( pTrain );

	if ( AI_IsTrainAtLocation( pTrain, pNode->position ) )
	{
		hook->nWaitCounter = 0;
		AI_RemoveCurrentTask( self );
		return;
	}
	else
	{
		hook->nWaitCounter++;
		if ( hook->nWaitCounter > 50 && AI_IsTrainStopped( pTrain ) == TRUE )
		{
			AI_RestartCurrentGoal( self );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartWaitForTrainToStop
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartWaitForTrainToStop( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_WAITFORTRAINTOSTOP );

	// make the entity just stand
	AI_ZeroVelocity( self );
	char szAnimation[16];
	AI_SelectAmbientAnimation( self, szAnimation );
	if ( AI_StartSequence( self, szAnimation, FRAME_LOOP ) == FALSE )
	{
		return;
	}

	gstate->Con_Dprintf( "Starting TASKTYPE_WAITFORTRAINTOSTOP.\n" );

	hook->nWaitCounter = 0;

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, 30.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_WaitForTrainToStop
// Description:
// Input:
// Output:
// Note:
//				The entity is on a train
//
// ----------------------------------------------------------------------------
void AI_WaitForTrainToStop( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	userEntity_t *pTrain = pAIData->pEntity;

	if(pTrain)
	{
		if ( AI_IsTrainStopped( pTrain ) )
		{
				AI_UpdateCurrentNode( self );
				AI_RemoveCurrentTask( self );
				AI_HandleGettingOffPlatformAndTrain( self );
				return;
		}
	}
}


/*void AI_WaitForTrainToStop( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	int nNodeIndex = pAIData->nValue;

	MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, nNodeIndex );
	_ASSERTE( pNode && (pNode->node_type & NODETYPE_TRAIN) );

	userEntity_t *pTrain = com->FindNodeTarget( NODE_GetTarget(pNode) );
	_ASSERTE( pTrain );

	if ( AI_IsTrainStopped( pTrain ) )
	{
	//	float fDistance = VectorDistance(self->s.origin,pNode->position);
	//	if ( AI_IsTrainAtLocation( pTrain, pNode->position ) || AI_IsCloseDistance2(self,fDistance))
	//	{
			AI_UpdateCurrentNode( self );
			AI_RemoveCurrentTask( self );

			AI_HandleGettingOffPlatformAndTrain( self );
			return;
	//	}
	//	else
	//	{
#if 0
/*
			userEntity_t *pSwitch = AI_GetClosestSwitch( pTrain, self->s.origin );
			if ( pSwitch )
			{
				pSwitch->use( pSwitch, self, self );
			}
*/
/*#endif 0
	//	}
	}
	else
	{
		hook->nWaitCounter++;
		if ( hook->nWaitCounter > 50 )
		{
			AI_RestartCurrentGoal( self );
		}
	}
}*/

// ----------------------------------------------------------------------------
//
// Name:		AI_StartStand
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartStand( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_STAND );

	AI_ZeroVelocity( self );

	char szAnimation[16];
	AI_SelectAmbientAnimation( self, szAnimation );
	if ( AI_StartSequence( self, szAnimation, FRAME_LOOP ) == FALSE )
	{
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_STAND.\n" );

	// this function good for only once, so remove this task
	AI_RemoveCurrentTask( self );
}	


// ----------------------------------------------------------------------------
// NSS[1/3/00]:
// Name:		AI_StartTakeCover_Attack
// Description:This function deals with the actual attacking within the whole
// take cover scheme of things.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartTakeCover_Attack(userEntity_t *self)
{
	if(self->enemy)
	{
		AI_FaceTowardPoint( self, self->enemy->s.origin );
	}
}


// ----------------------------------------------------------------------------
// NSS[1/3/00]:
// Name:		AI_TakeCover_Attack
// Description: The act of the attack sequence
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void AI_TakeCover_Attack(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	if(self->enemy)
	{
		AI_FaceTowardPoint( self, self->enemy->s.origin );
	}
	if(hook->fnTakeCoverFunc)
	{
		hook->fnTakeCoverFunc( self );
	}

	if(hook->attack_finished > gstate->time)
	{
		AI_RemoveCurrentTask(self,FALSE);
	}

}


// ----------------------------------------------------------------------------
//
// Name:		AI_StartTakeCover
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartTakeCover( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_TAKECOVER );

	if ( !hook->fnTakeCoverFunc )
	{
		AI_RemoveCurrentGoal( self );
		return;
	}

	hook->nAttackMode = ATTACKMODE_TAKECOVER;
    AI_SetStateAttacking( hook );

	AI_Dprintf( "Starting TASKTYPE_TAKECOVER.\n" );

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1.0f );
	AI_SetMovingCounter( hook, 0 );
}	




// ----------------------------------------------------------------------------
//
// Name:		AI_TakeCover
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_TakeCover( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *pEnemy = self->enemy;
    if ( !AI_IsAlive( pEnemy ) )
	{
		// find enemy
		pEnemy = hook->fnFindTarget( self );
		if ( AI_IsAlive( pEnemy ) )
		{
			self->enemy = pEnemy;
		}
		else
		{
			return;
		}
	}
    pEnemy = self->enemy;
    _ASSERTE( AI_IsAlive( pEnemy ) );
	
    NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( self );
	_ASSERTE( pNodeHeader );

    frameData_t *pSequence = hook->cur_sequence;

	int bEnemyVisible = AI_IsCompletelyVisible( self, pEnemy );

	// is the AI just standing or just finished attacking
	int bAttackSequence = FALSE;
    if ( pSequence && strstr( pSequence->animation_name, "atak" ) )
    {
        bAttackSequence = TRUE;
    }

	if ( bEnemyVisible == COMPLETELY_VISIBLE && hook->attack_finished < gstate->time)
	{
		PATHLIST_KillPath(hook->pPathList);
		// fire a shot
        AI_AddNewTaskAtFront(self,TASKTYPE_TAKECOVER_ATTACK);
		// NSS[1/3/00]:Made a task out of this.
		//hook->fnTakeCoverFunc( self );
		return;
	}
	else
	{
		// wait a little bit before attacking
		if ( (hook->nTargetCounter % 40) == 0 && rnd() > 0.5f )
		{
            if ( !AI_FindPathToEntity( self, pEnemy ) && AI_CanPath( hook ) )
			{
                // do not try another path for 2.5 seconds
			    AI_SetNextPathTime( hook, 2.5f );
			}

			if ( hook->pPathList->pPath )
			{
				if ( hook->pPathList->nPathLength < 5 || 
					 !pSequence || !strstr( pSequence->animation_name, "run" ) )
				{
					// start the run sequence
					char szAnimation[16];
					AI_SelectRunningAnimation( self, szAnimation );
					AI_ForceSequence( self, szAnimation, FRAME_LOOP );
				}
				else
				{
					PATHLIST_KillPath(hook->pPathList);
				}
			}
		}
        //else
        //{
        //    AI_FaceTowardPoint( self, pEnemy->s.origin );
        //}
	}

    pSequence = hook->cur_sequence;



	if ( bEnemyVisible != NOT_VISIBLE && 
		 ( !bAttackSequence || 
		   (bAttackSequence && 
		   (self->s.frameInfo.frameState == FRSTATE_LAST || self->s.frameInfo.frameState & FRSTATE_STOPPED))
		 )
	   )
	{
		// hide
        if ( !hook->pPathList->pPath )
		{
			MAPNODE_PTR pNode = NODE_GetClosestCompleteHideNode( pNodeHeader, self, pEnemy );
			if ( AI_FindPathToNode( self, pNode ) == 0 && pNode )
			{
				float fXYDistance = VectorXYDistance( self->s.origin, pNode->position );
				float fZDistance = VectorZDistance( self->s.origin, pNode->position );
				if ( !AI_IsCloseDistance2(self, fXYDistance ) || fZDistance >= 32.0f )
				{
					PATHLIST_AddNodeToPath(hook->pPathList, pNode);	
				}
			}
		}

		if ( hook->pPathList->pPath && 
			 (!pSequence || !strstr( pSequence->animation_name, "run" )) )
		{
			// start the run sequence
			char szAnimation[16];
			AI_SelectRunningAnimation( self, szAnimation );
			AI_ForceSequence( self, szAnimation, FRAME_LOOP );
		}
	}

	if ( hook->pPathList->pPath && hook->pPathList->pPath->nNodeIndex >= 0 )
	{
		if ( !pSequence || !strstr( pSequence->animation_name, "run" ) )
		{
			// start the run sequence
			char szAnimation[16];
			AI_SelectRunningAnimation( self, szAnimation );
			AI_ForceSequence( self, szAnimation, FRAME_LOOP );
		}

		if ( !AI_HandleUse( self ) )
		{
			if(hook->pPathList && hook->pPathList->pPath)
			{
				MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, hook->pPathList->pPath->nNodeIndex );

				if ( AI_MoveTowardPoint( self, pNode->position, TRUE ) == TRUE )
				{
					// we're already at this node, so delete the first one
					PATHLIST_DeleteFirstInPath( hook->pPathList );

					if ( !hook->pPathList->pPath )
					{
						AI_ZeroVelocity( self );
						char szAnimation[16];
						AI_SelectAmbientAnimation( self, szAnimation );
						AI_ForceSequence( self, szAnimation, FRAME_LOOP );
					}
					else
					{
						NODELIST_PTR pNodeList = hook->pNodeList;
						_ASSERTE( pNodeList );
						NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
						_ASSERTE( pNodeHeader );

						MAPNODE_PTR pNextNode = NODE_GetNode( pNodeHeader, hook->pPathList->pPath->nNodeIndex );
						if ( AI_HandleUse( self, pNodeHeader, pNode, pNextNode ) )
						{
							return;
						}
					}
				}
			}
		}
	}
	else
	{
		if ( pSequence && strstr( pSequence->animation_name, "run" ) )
		{
			AI_ZeroVelocity( self );
			char szAnimation[16];
			AI_SelectAmbientAnimation( self, szAnimation );
			AI_ForceSequence( self, szAnimation, FRAME_LOOP );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_GetNextPathCorner
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *AI_GetNextPathCorner( userEntity_t *pPathCorner )
{
	_ASSERTE( pPathCorner );
	mpathHook_t *pathCornerHook = (mpathHook_t *)pPathCorner->userHook;
	_ASSERTE( pathCornerHook );

	int nNumTargets = 0;

	// find the number of targets on the path corner that we have reached
	while ( pathCornerHook->target[nNumTargets] != NULL )
	{
		nNumTargets++;
	}
	
	int nTargetNum = 0;
	if ( nNumTargets > 1 )
	{
		nTargetNum = (int)(rand() % nNumTargets);
	}

	// find the entity with the matching target name
	userEntity_t *pNextPathCorner = NULL;
	if ( pathCornerHook->target[nTargetNum] && strlen( pathCornerHook->target[nTargetNum] ) > 0 )
	{
		pNextPathCorner = com->FindTarget( pathCornerHook->target[nTargetNum] );
	}
	return pNextPathCorner;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartPathFollow
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartPathFollow( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_PATHFOLLOW );

	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_PATHFOLLOW )
		{
			return;
		}
	}

	// IMPORTANT:
	// if this is a brand new task, the AIDATA for the goal should be blank,
	// if this is a returning task after a pathcorner action is finished for instance, then
	//		the AIDATA should contain previous pathcorner

	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = GOAL_GetData(pCurrentGoal);
	userEntity_t *pPathCorner = pAIData->pEntity;
	if ( pPathCorner )
	{
		userEntity_t *pNextPathCorner = AI_GetNextPathCorner( pPathCorner );
		if ( pNextPathCorner )
		{
			if ( !AI_IsLineOfSight( self, pPathCorner ) )
            {
                AI_Dprintf( "No line of sight to path corner!" );
                AI_AddNewGoal( self, GOALTYPE_MOVETOENTITY, pPathCorner );
                return;
            }

            TASK_Set( pCurrentTask, pNextPathCorner );
		}
		else
		{
			//gstate->Con_Dprintf( "Monster %s PATHFOLLOW failed due to no more pathcorners at %s.\n", 
			//					 self->className, com->vtos(self->s.origin) );
			
			GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
			GOAL_Satisfied( pCurrentGoal );
			AI_RemoveCurrentGoal( self );
			return;
		}
	}
	else
	{
		if ( self->target )
		{
			userEntity_t *pNextPathCorner = com->FindTarget( self->target );
			if ( pNextPathCorner )
			{
			    if ( !AI_IsVisible( self, pNextPathCorner ) )
				//if ( !AI_IsLineOfSight( self, pNextPathCorner ) )
                {
                    AI_Dprintf( "No line of sight to path corner!" );
                    AI_AddNewGoal( self, GOALTYPE_MOVETOENTITY, pNextPathCorner );
                    return;
                }

				TASK_Set( pCurrentTask, pNextPathCorner );
			}
			else if( gstate->nLevelTransitionType == 3 )
			{
				AI_SetNextThinkTime( self, 3.0f );
				return;
			}
			else
			{
				gstate->Con_Dprintf( "Monster %s PATHFOLLOW failed due to no more pathcorners at %s.\n", 
									 self->className, com->vtos(self->s.origin) );
				
				GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
				GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
				if ( nGoalType == GOALTYPE_PATHFOLLOW )
				{
					GOAL_Satisfied( pCurrentGoal );
				}
				AI_RemoveCurrentTask( self );
				return;
			}
		}
		else
		{
			// no target, so find the nearest monster_path_corner
			pPathCorner = com->FindClosestEntity( self, "monster_path_corner" );
			if ( pPathCorner )
			{
			    if ( !AI_IsLineOfSight( self, pPathCorner ) )
                {
                    AI_Dprintf( "No line of sight to path corner!" );
                    AI_AddNewGoal( self, GOALTYPE_MOVETOENTITY, pPathCorner );
                    return;
                }

				float fXYDistance = VectorXYDistance( self->s.origin, pPathCorner->s.origin );
				float fZDistance = VectorZDistance( self->s.origin, pPathCorner->s.origin );
				if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
				{
					// find the next one and go to it
					userEntity_t *pNextPathCorner = AI_GetNextPathCorner( pPathCorner );
					if ( pNextPathCorner )
					{
			            if ( !AI_IsLineOfSight( self, pNextPathCorner ) )
                        {
                            AI_Dprintf( "No line of sight to path corner!" );
                            AI_AddNewGoal( self, GOALTYPE_MOVETOENTITY, pNextPathCorner );
                            return;
                        }
						
                        TASK_Set( pCurrentTask, pNextPathCorner );
					}
					else
					{
						gstate->Con_Dprintf( "Monster %s PATHFOLLOW failed due to no more pathcorners at %s.\n", 
											 self->className, com->vtos(self->s.origin) );
						
						GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
						GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
						if ( nGoalType == GOALTYPE_PATHFOLLOW )
						{
							GOAL_Satisfied( pCurrentGoal );
						}
						AI_RemoveCurrentTask( self );
						return;
					}
				}
				else
				{
					TASK_Set( pCurrentTask, pPathCorner );
				}
			}
			else
			{
				gstate->Con_Dprintf( "Monster %s wanting PATHFOLLOW but no path corners defined at %s.\n", 
									 self->className, com->vtos(self->s.origin) );
				
				GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
				GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
				if ( nGoalType == GOALTYPE_PATHFOLLOW )
				{
					GOAL_Satisfied( pCurrentGoal );
				}
				AI_RemoveCurrentTask( self );
				return;
			}
		}
	}

	AI_Dprintf( "Starting TASKTYPE_PATHFOLLOW.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );

	pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	pAIData = TASK_GetData(pCurrentTask);
	pPathCorner = pAIData->pEntity;
	_ASSERTE( pPathCorner );
    float fDistance = AI_ComputeDistanceToPoint( self, pPathCorner->s.origin );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 2.0f;
	AI_SetTaskFinishTime( hook, fTime );

	AI_SetMovingCounter( hook, 0 );

}


bool AI_ActionFaceAngle(userEntity_t *self, userEntity_t *pPathCorner, CVector *facingAngle )
// turn the entity to the specified angle/direction
{
  bool bFound = FALSE;
  float fAngle = 0.0f;
  int i = 0;


  while (pPathCorner->epair[i].key)
  {
    if ( _stricmp(pPathCorner->epair[i].key, "angle") == 0 )
	{
	  if ( stricmp(pPathCorner->epair[i].value, "") )
	  {
	    fAngle = (float)atoi( pPathCorner->epair[i].value );
		bFound = true;
		break;
	  }
	}
	i++;
  }

  if ( bFound )
  {
	facingAngle->x = self->s.angles.x;
	facingAngle->y = fAngle;
	facingAngle->z = self->s.angles.z;
  }

  return (bFound);
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddTasksToScriptAction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_AddTasksToScriptAction( userEntity_t *self, CScriptAction *pScriptAction )
{
	_ASSERTE( self );
	_ASSERTE( pScriptAction );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( pGoalStack );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
	_ASSERTE( pCurrentGoal );

	TASK_PTR pFirstTask = NULL;

	CPtrList *pActionList = pScriptAction->GetActionList();
	POSITION pos = pActionList->GetHeadPosition();
	while ( pos )
	{
		CAction *pAction = (CAction*)pActionList->GetNext( pos );
		int nActionType = pAction->GetActionType();
		
		TASK_PTR pTask = NULL;
		switch ( nActionType )
		{
			case ACTIONTYPE_ANIMATE:
			{
				// two parameters
				CParameter *pParameter0 = pAction->GetParameter( 0 );
				_ASSERTE( pParameter0 );
				CParameter *pParameter1 = pAction->GetParameter( 1 );
				if ( pParameter1 )
				{
					pTask = AI_AddNewTask( self,
										   TASKTYPE_ACTION_PLAYANIMATION, 
										   pParameter0->GetString(), 
										   pParameter1->GetValue() );
				}
				else
				{
					pTask = AI_AddNewTask( self, 
										   TASKTYPE_ACTION_PLAYANIMATION, 
										   pParameter0->GetString() );
				}

				break;
			}

			case ACTIONTYPE_ANIMATEPARTIAL:
			{
				// two parameters
				CParameter *pParameter0 = pAction->GetParameter( 0 );
				_ASSERTE( pParameter0 );
				CParameter *pParameter1 = pAction->GetParameter( 1 );
				_ASSERTE( pParameter1 );
				CParameter *pParameter2 = pAction->GetParameter( 2 );
				if ( pParameter2 )
				{
					pTask = AI_AddNewTask( self, 
										   TASKTYPE_ACTION_PLAYPARTIALANIMATION,
										   pParameter0->GetString(), 
										   pParameter1->GetValue(),
										   pParameter2->GetValue() );
				}
				else
				{
					pTask = AI_AddNewTask( self, 
										   TASKTYPE_ACTION_PLAYPARTIALANIMATION,
										   pParameter0->GetString(),
										   pParameter1->GetValue(),
										   0 );
				}

				break;
			}
		
			case ACTIONTYPE_SOUND:
			{
				pTask = AI_AddNewTask( self, TASKTYPE_ACTION_PLAYSOUND, pAction );
				break;
			}

			case ACTIONTYPE_SPAWN:
			{
				pTask = AI_AddNewTask( self, TASKTYPE_ACTION_SPAWN, pAction );

				break;
			}
			case ACTIONTYPE_MOVETO:
			{
				// three parameters
				CParameter *pParameter0 = pAction->GetParameter( 0 );
				_ASSERTE( pParameter0 );
				CParameter *pParameter1 = pAction->GetParameter( 1 );
				_ASSERTE( pParameter1 );
				CParameter *pParameter2 = pAction->GetParameter( 2 );
				_ASSERTE( pParameter2 );

				CVector destPoint( pParameter0->GetValue(), pParameter1->GetValue(), pParameter2->GetValue() );
				pTask = AI_AddNewTask( self, TASKTYPE_MOVETOLOCATION, destPoint );

				break;
			}
			case ACTIONTYPE_FACEANGLE:
			{
				// three parameters
				CParameter *pParameter0 = pAction->GetParameter( 0 );
				_ASSERTE( pParameter0 );
				CParameter *pParameter1 = pAction->GetParameter( 1 );
				_ASSERTE( pParameter1 );
				CParameter *pParameter2 = pAction->GetParameter( 2 );
				_ASSERTE( pParameter2 );

				CVector angle( pParameter0->GetValue(), pParameter1->GetValue(), pParameter2->GetValue() );
				pTask = AI_AddNewTask( self, TASKTYPE_FACEANGLE, angle );

				break;
			}
			case ACTIONTYPE_USE:
			{
				CParameter *pParameter = pAction->GetParameter( 0 );
				_ASSERTE( pParameter );

				_ASSERTE( strlen( pParameter->GetString() ) > 0 );
				userEntity_t *pUseEntity = UNIQUEID_Lookup( pParameter->GetString() );
				if ( AI_IsAlive( pUseEntity ) )
				{
					pTask = AI_AddNewTask( self, TASKTYPE_USE, pUseEntity );
				}
				else
				{
					gstate->Con_Dprintf( "Uniqueid %s not found\n", pParameter->GetString() );
				}

				break;
			}
			case ACTIONTYPE_SENDMESSAGE:
			{
				pTask = AI_AddNewTask( self, TASKTYPE_ACTION_SENDMESSAGE, pAction );
				break;
			}
			case ACTIONTYPE_SETMOVINGANIMATION:
			{
				pTask = AI_AddNewTask( self, TASKTYPE_ACTION_SETMOVINGANIMATION, pAction );
				break;
			}
			case ACTIONTYPE_SETSTATE:
			{
				pTask = AI_AddNewTask( self, TASKTYPE_ACTION_SETSTATE, pAction );
				break;
			}
			case ACTIONTYPE_DIE:
			{
				// two parameters
				CParameter *pParameter0 = pAction->GetParameter( 0 );
				_ASSERTE( pParameter0 );
				CParameter *pParameter1 = pAction->GetParameter( 1 );
				_ASSERTE( pParameter1 );
				
				_ASSERTE( strlen( pParameter0->GetString() ) > 0 );
				userEntity_t *pDieEntity = UNIQUEID_Lookup( pParameter0->GetString() );
				if ( AI_IsAlive( pDieEntity ) )
				{
					pTask = AI_AddNewTask( pDieEntity, TASKTYPE_ACTION_DIE, pParameter1->GetString() );
				}
				else
				{
					gstate->Con_Dprintf( "Uniqueid %s not found\n", pParameter1->GetString() );
				}
				break;
			}
			case ACTIONTYPE_CALL:
			{
				pTask = AI_AddNewTask( self, TASKTYPE_ACTION_CALL, pAction );
				
				break;
			}
			case ACTIONTYPE_RANDOMSCRIPT:
			{
				pTask = AI_AddNewTask( self, TASKTYPE_ACTION_RANDOMSCRIPT, pAction );

				break;
			}
			case ACTIONTYPE_STREAMSOUND:
			{
				pTask = AI_AddNewTask( self, TASKTYPE_ACTION_STREAMSOUND, pAction );
				
				break;
			}
			case ACTIONTYPE_SENDURGENTMESSAGE:
			{
				pTask = AI_AddNewTask( self, TASKTYPE_ACTION_SENDURGENTMESSAGE, pAction );
				break;
			}
			case ACTIONTYPE_COMENEAR:
			{
				pTask = AI_AddNewTask( self, TASKTYPE_ACTION_COMENEAR, pAction );
				break;
			}
			case ACTIONTYPE_REMOVE:
			{
				pTask = AI_AddNewTask( self, TASKTYPE_ACTION_REMOVE, pAction );
				break;
			}
			case ACTIONTYPE_LOOKAT:
			{
				pTask = AI_AddNewTask( self, TASKTYPE_ACTION_LOOKAT, pAction );
				break;
			}
            case ACTIONTYPE_STOPLOOK:
            {
                pTask = AI_AddNewTask( self, TASKTYPE_ACTION_STOPLOOK, pAction );
                break;
            }
            case ACTIONTYPE_ATTACK:
            {
                pTask = AI_AddNewTask( self, TASKTYPE_ACTION_ATTACK, pAction );
                break;
            }
            case ACTIONTYPE_PRINT:
            {
                pTask = AI_AddNewTask( self, TASKTYPE_ACTION_PRINT, pAction );
                break;
            }

			default:
			{
				_ASSERTE( FALSE );
				break;
			}
		}
	}

	// get the first task again just in case the previous animation has not finished
	pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
	_ASSERTE( pCurrentGoal );
	pFirstTask = GOAL_GetCurrentTask( pCurrentGoal );
	if ( pFirstTask )
	{
		AI_StartTask( self, pFirstTask );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewScriptActionGoal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_AddNewScriptActionGoal( userEntity_t *self, char *szScriptAction, int bUseUniqueId /* = TRUE */)
{
	_ASSERTE( self );

	CScriptAction *pScriptAction = SCRIPTACTION_Lookup( szScriptAction );
	if ( pScriptAction )
	{
		userEntity_t *pEntity = self;
		
		if ( bUseUniqueId )
		{
			if ( strlen( pScriptAction->GetUniqueID() ) > 0 )
			{
				// find the entity that this script belongs to
				pEntity = UNIQUEID_Lookup( pScriptAction->GetUniqueID() );
				if ( !AI_IsAlive( pEntity ))
				{
					gstate->Con_Dprintf( "Uniqueid: %s not found\n", pScriptAction->GetUniqueID() );
					return FALSE;
				}
			}
		}
		if ( !pEntity )
		{
			pEntity = self;
		}

		if ( _stricmp( pEntity->className, "player" ) == 0 )
		{
#ifdef WIN32
			_RPT1( _CRT_WARN, "Trying to add script %s to player.", szScriptAction );
#endif			
			return FALSE;
		}


		playerHook_t *hook = AI_GetPlayerHook( pEntity );

		// NSS[2/4/00]:No hook... no go! No TaskThinking... no GO!
		//if(!hook || (pEntity->think != AI_TaskThink))
		if(!hook)
		{
			gstate->Con_Dprintf( "Entity has been freed or has no hook, ignoring script action!\n");
			return FALSE;
		}
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		
		if(pGoalStack)
		{
			GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
			if(pCurrentGoal)
			{
				if(pCurrentGoal->nGoalType == GOALTYPE_SCRIPTACTION)
				{
					GOALSTACK_ClearAllGoals( pGoalStack );
					if( hook->szScriptName )
					{
						free( hook->szScriptName );
					}

					hook->szScriptName = _strdup( szScriptAction );
				}
			}
		}

		AIDATA aiData;
		aiData.pString		= szScriptAction;
		aiData.nValue		= pScriptAction->GetLoopCount();
		GOAL_PTR pCurrentGoal = AI_AddNewGoal( pEntity, GOALTYPE_SCRIPTACTION, &aiData );
		_ASSERTE( pCurrentGoal );

		AI_AddTasksToScriptAction( pEntity, pScriptAction );

		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddScriptActionGoal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_AddScriptActionGoal( userEntity_t *self, char *szScriptAction, 
							int bUseUniqueId /* = TRUE */, int bRestart /* = FALSE */ )
{
	_ASSERTE( self );

	CScriptAction *pScriptAction = SCRIPTACTION_Lookup( szScriptAction );
	if ( pScriptAction )
	{
		if ( bRestart == FALSE && pScriptAction->GetType() == SCRIPTTYPE_LOOP )
		{
			return AI_AddNewScriptActionGoal( self, szScriptAction, bUseUniqueId );
		}

		userEntity_t *pEntity = self;
		
		if ( bUseUniqueId )
		{
			// find the entity that this script belongs to
			if ( strlen( pScriptAction->GetUniqueID() ) > 0 )
			{
				pEntity = UNIQUEID_Lookup( pScriptAction->GetUniqueID() );
				if ( !AI_IsAlive( pEntity ) )
				{
					gstate->Con_Dprintf( "Uniqueid: %s not found\n", pScriptAction->GetUniqueID() );
					return FALSE;
				}
			}
		}
		if ( !pEntity )
		{
			pEntity = self;
		}

		if ( _stricmp( pEntity->className, "player" ) == 0 )
		{
#ifdef WIN32
			_RPT1( _CRT_WARN, "Trying to add script %s to player.", szScriptAction );
#endif			
			return FALSE;
		}

		playerHook_t *hook = AI_GetPlayerHook( pEntity );
		//NSS[11/8/99]:Let's check to see if we even have a hook!!!
		if(!hook)
			return FALSE;
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		_ASSERTE( pGoalStack );
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
		if ( !pCurrentGoal || GOAL_GetType( pCurrentGoal ) != GOALTYPE_SCRIPTACTION )
		{
			AIDATA aiData;
			aiData.pString = szScriptAction;
			aiData.nValue = pScriptAction->GetLoopCount();
			pCurrentGoal = AI_AddNewGoal( pEntity, GOALTYPE_SCRIPTACTION, &aiData );
			_ASSERTE( pCurrentGoal );
		}
	/*	else if(pGoalStack && GOAL_GetType( pCurrentGoal ) == GOALTYPE_SCRIPTACTION)
		{
			// NSS[2/3/00]:If we are currently doing a script but need to do another one kill all goals and do this script!
			GOALSTACK_ClearAllGoals( pGoalStack );
			return AI_AddNewScriptActionGoal( self, szScriptAction, bUseUniqueId);
		}*/
		
		AI_AddTasksToScriptAction( pEntity, pScriptAction );

		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ProcessPathCornerAction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_ProcessPathCornerAction( userEntity_t *self, userEntity_t *pPathCorner)
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	
	_ASSERTE( pPathCorner );
	mpathHook_t *chook = (mpathHook_t *) pPathCorner->userHook;
	_ASSERTE( chook );

    GOALSTACK_PTR pGoalStack = NULL;
	GOAL_PTR pCurrentGoal = NULL;
    CVector cvFacingAngle;

	int bAction = FALSE;

	// handle the script action first
	if ( chook->szScriptName )
	{
		bAction = AI_AddScriptActionGoal( self, chook->szScriptName );
	}
	if( chook->pathtarget != NULL)
	{
		userEntity_t *Thing	= NULL;
		userEntity_t *ent = gstate->FirstEntity();
		// NSS[3/12/00]:Added pathtarget use capabilities.
		//Process through all the entities on the map
		while(ent)
		{
			//Make sure the entity is the right classname
			if(ent->targetname != NULL && _stricmp(ent->targetname, chook->pathtarget)==0)
			{
				//now use the mutha' fucka!
				if(ent->use != NULL)
				{
					ent->use(ent,self,self);
				}
			}
			ent = gstate->NextEntity(ent);
		}	
		chook->pathtarget = NULL;

	}
	return bAction;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_PathFollow
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_PathFollow( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	userEntity_t *pPathCorner = pAIData->pEntity;

	// SCG[2/14/00]: defer it if we are in a save game
	if( pPathCorner == NULL )
	{
		if( gstate->nLevelTransitionType == 3 )
		{
			AI_RestartCurrentGoal( self );
		}
		else
		{
			AI_RemoveCurrentGoal( self );
		}

		return;
	}

	float fXYDistance = VectorXYDistance( self->s.origin, pPathCorner->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pPathCorner->s.origin );
	if ( (AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f) || (fXYDistance < 20.0f && fZDistance < 32.0f) )
	{
		// reached a path node, see if there is an action associated with it

		// remember this path corner as visited
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		GOAL_Set( pCurrentGoal, pPathCorner );

		if ( !AI_ProcessPathCornerAction( self, pPathCorner ) )
		{
			// find the next one and go to it
			userEntity_t *pNextPathCorner = AI_GetNextPathCorner( pPathCorner );
			if ( pNextPathCorner )
			{
				TASK_Set( pCurrentTask, pNextPathCorner );

                float fDistance = AI_ComputeDistanceToPoint( self, pNextPathCorner->s.origin );
	            float fSpeed = AI_ComputeMovingSpeed( hook );
	            float fTime = (fDistance / fSpeed) + 2.0f;
	            AI_SetTaskFinishTime( hook, fTime );
			}
			else
			{
				gstate->Con_Dprintf( "Monster %s PATHFOLLOW failed due to no more pathcorners at %s.\n", 
									 self->className, com->vtos(self->s.origin) );
				
				GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
				//NSS[11/17/99]:This should handle monsters being spawned with no initial setting of IDLE
				if(pGoalStack->nNumGoals <= 1)
					AI_AddNewGoalAtBack(self,GOALTYPE_IDLE);
				GOAL_Satisfied( pCurrentGoal );
				AI_RemoveCurrentGoal( self );
				return;
			}
		}

		return;
	}
	//Yet ANOTHER temp hack to be able to do flying monster corner path
	//following... 
	if( ( hook->type == TYPE_DOOMBAT ) || ( hook->type == TYPE_GRIFFON ) || hook->type == TYPE_HARPY || hook->type == TYPE_CHAINGANG || hook->type == TYPE_SL_SKEET|| hook->type == TYPE_DRAGON)
	{
		//New Fly Type... 
		AI_FlyTowardPoint2( self, pPathCorner->s.origin, 0.10f);
	}
	else
	{
		AI_MoveTowardPoint( self, pPathCorner->s.origin, TRUE );
	}

}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartDodge
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartDodge( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_DODGE );

	int bSuccess = TRUE;
	int bFindAlternateSpot = TRUE;

	MAPNODE_PTR pDodgeNode = NULL;
	NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( self );
	if ( pNodeHeader )
	{
		if ( rnd() > 0.5f )
		{
			if ( rnd() < 0.2f )
			{
				pDodgeNode = NODE_GetClosestVisibleNode( pNodeHeader, self, self->enemy );
				if ( pDodgeNode )
				{
					AI_FindPathToNode( self, pDodgeNode );
					bFindAlternateSpot = FALSE;
				}
			}
			else
			{
				MAPNODE_PTR pHideNodes[MAX_CLOSEST_HIDENODES];
				int nNumHideNodes = NODE_GetClosestHideNodes( pNodeHeader, self, self->enemy, pHideNodes );
				if ( nNumHideNodes > 0 )
				{
					int nDodgeNodeIndex = (int) (rand() % nNumHideNodes);
					pDodgeNode = pHideNodes[nDodgeNodeIndex];
					if ( pDodgeNode )
					{
						AI_FindPathToNode( self, pDodgeNode );
						if ( hook->pPathList->nPathLength < 4 )
						{
							bFindAlternateSpot = FALSE;
						}
						else
						{
							PATHLIST_KillPath (hook->pPathList);
						}
					}
				}
			}
		}
	}

	float fMaxTime = 5.0f;
	CVector dodgePoint;
	if ( bFindAlternateSpot == TRUE )
	{
		float fSideStepDistance = SIDESTEP_DISTANCE+32.0f;
		bSuccess = AI_ComputeSideStepPoint( self, fSideStepDistance, dodgePoint );

		fMaxTime = 2.0f;
	}
	else
	{
		if ( pDodgeNode )
		{
			dodgePoint = pDodgeNode->position;
		}
		else
		{
			// set to current position as the dodge point
			dodgePoint = self->s.origin;
            bSuccess = FALSE;
		}
	}

	if ( !bSuccess )
    {
        AI_RemoveCurrentTask( self );
        return;
    }
    else
	{
		AI_SetStateRunning( hook );
		if ( AI_StartMove( self ) == FALSE )
		{
			TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
            _ASSERTE( pCurrentTask );
			if ( TASK_GetType( pCurrentTask ) != TASKTYPE_DODGE )
			{
				return;
			}
		}
	}

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	TASK_Set( pCurrentTask, dodgePoint );

	AI_Dprintf( "Starting TASKTYPE_DODGE.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, fMaxTime );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_FlyDodge
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_FlyDodge( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
	{
		AI_RestartCurrentGoal( self );
		return;
	}

	tr = gstate->TraceLine_q2( self->s.origin, pAIData->destPoint, self, MASK_SOLID );
	if ( tr.fraction >= 1.0f )
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
// Name:		AI_Dodge
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Dodge( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	if ( AI_IsFlyingUnit( self ) || AI_IsInWater( self ) )
	{
		AI_FlyDodge( self );
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	
	if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
	{
		AI_RestartCurrentGoal( self );
		return;
	}
	
	if ( AI_IsOkToMoveStraight( self, pAIData->destPoint, fXYDistance, fZDistance ) )
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
// Name:		AI_StartHide
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartHide( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_HIDE );

	int bFoundHidingPlace = FALSE;
	NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( self );
	if ( pNodeHeader )
	{
		MAPNODE_PTR pHideNode = NULL;
		MAPNODE_PTR pHideNodes[MAX_CLOSEST_HIDENODES];
		int nNumHideNodes = NODE_GetClosestHideNodes( pNodeHeader, self, self->enemy, pHideNodes );
		if ( nNumHideNodes > 0 )
		{
			int nHideNodeIndex = (int) (rand() % nNumHideNodes);
			pHideNode = pHideNodes[nHideNodeIndex];
			if ( pHideNode )
			{
				AI_FindPathToNode( self, pHideNode );
                bFoundHidingPlace = TRUE;
			}
		}

		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( pHideNode )
		{
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

    if ( bFoundHidingPlace )
    {	
        AI_SetStateRunning( hook );
	    if ( AI_StartMove( self ) == FALSE )
	    {
		    TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
            _ASSERTE( pCurrentTask );
		    if ( TASK_GetType( pCurrentTask ) != TASKTYPE_HIDE )
		    {
			    return;
		    }
	    }
    }

	AI_Dprintf( "Starting TASKTYPE_HIDE.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
    _ASSERTE( pAIData );
	float fDistance = AI_ComputeDistanceToPoint( self, pAIData->destPoint );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 2.0f;
	AI_SetTaskFinishTime( hook, fTime );

	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_FlyHide
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_FlyHide( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
	{
		AI_RestartCurrentGoal( self );
		return;
	}

	tr = gstate->TraceLine_q2( self->s.origin, pAIData->destPoint, self, MASK_SOLID );
	if ( tr.fraction >= 1.0f )
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
// Name:		AI_Hide
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Hide( userEntity_t *self )
{
	_ASSERTE( self );
	if ( AI_IsFlyingUnit( self ) || AI_IsInWater( self ) )
	{
		AI_FlyHide( self );
	}

	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
	{
		AI_RestartCurrentGoal( self );
		return;
	}

	if ( AI_IsOkToMoveStraight( self, pAIData->destPoint, fXYDistance, fZDistance ) )
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
// Name:		AI_StartSnipe
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartSnipe( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_SNIPE );

	if ( hook->cur_sequence && strstr( hook->cur_sequence->animation_name, "run" ) )
	{
		char szAnimation[16];
		AI_SelectAmbientAnimation( self, szAnimation );
		if ( AI_StartSequence( self, szAnimation, FRAME_LOOP ) == FALSE )
		{
			return;
		}
	}

	AI_Dprintf( "Starting TASKTYPE_SNIPE.\n" );

	hook->nWaitCounter = 0;

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, -1.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_Snipe
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Snipe( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !AI_CanMove( hook ) )
    {
		// find a target
		userEntity_t *pEnemy = hook->fnFindTarget( self );
		if ( AI_IsAlive( pEnemy ) )
		{
			AI_AddNewGoal( self, GOALTYPE_KILLENEMY, pEnemy );
            return;
		}
    }
    else
    {
        MAPNODE_PTR pClosestNode = NODE_GetClosestNode(self);
	    if ( pClosestNode )
	    {
		    if ( pClosestNode->node_type & NODETYPE_SNIPE )
		    {
			    // find a target
			    userEntity_t *pEnemy = hook->fnFindTarget( self );
			    if ( AI_IsAlive( pEnemy ) )
			    {
				    AI_AddNewGoal( self, GOALTYPE_KILLENEMY, pEnemy );
                    return;
			    }
		    }
		    else
		    {
			    hook->nWaitCounter++;

			    if ( hook->nWaitCounter > 20 && rnd() < 0.25f )
			    {
				    // currently not at a snipe node, so goto a closest snipe node
				    MAPNODE_PTR pSnipeNodes[MAX_CLOSEST_SNIPENODES];
				    int nNumSnipeNodes = NODE_GetClosestSnipeNodes( self, pSnipeNodes );
				    if ( nNumSnipeNodes > 0 )
				    {
					    int nSnipeNodeIndex = (int) (rand() % nNumSnipeNodes);
					    MAPNODE_PTR pSnipeNode = pSnipeNodes[nSnipeNodeIndex];
					    if ( pSnipeNode )
					    {
						    AI_AddNewTaskAtFront( self, TASKTYPE_MOVETOSNIPE, pSnipeNode->position );
					    }
				    }
			    }
		    }
	    }
    }
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartSideStep
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartSideStep( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_SIDESTEP );

	CVector sideStepPoint;
	int bSuccess = AI_ComputeSideStepPoint( self, SIDESTEP_DISTANCE, sideStepPoint );
    if ( !bSuccess )
    {
        AI_RemoveCurrentTask( self );
        return;
    }
    else
	{
		AI_SetStateRunning( hook );
		if ( AI_StartMove( self ) == FALSE )
		{
			TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
            _ASSERTE( pCurrentTask );
			if ( TASK_GetType( pCurrentTask ) != TASKTYPE_SIDESTEP )
			{
				return;
			}
		}
	}

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	TASK_Set( pCurrentTask, sideStepPoint );

	AI_Dprintf( "Starting TASKTYPE_SIDESTEP.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 2.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_SideStep
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_SideStep( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
	{
		AI_RestartCurrentGoal( self );
		return;
	}

	AI_MoveTowardPoint( self, pAIData->destPoint, FALSE );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartGoingAroundEntity
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartGoingAroundEntity( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_GOINGAROUNDENTITY );

	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_GOINGAROUNDENTITY )
		{
			return;
		}
	}

	AI_Dprintf( "Starting TASKTYPE_GOINGAROUNDENTITY.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	if(!pCurrentTask)
		return;
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	float fDistance = VectorDistance( self->s.origin, pAIData->destPoint );
    float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 1.0f;

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, fTime );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_GoingAroundEntity
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_GoingAroundEntity( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
	{
		AI_RemoveCurrentTask( self );
		return;
	}
	if(AI_IsSidekick(hook) && pAIData->pEntity && (pAIData->pEntity->flags & FL_BOT))
	{
		fXYDistance = VectorXYDistance( self->s.origin, pAIData->pEntity->s.origin );
		fZDistance = VectorZDistance( self->s.origin, pAIData->pEntity->s.origin );
		if ( !AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
		{
			AI_RemoveCurrentTask( self );
			return;
		}	
	}

	//tr = gstate->TraceLine_q2( self->s.origin, pAIData->destPoint, self, MASK_SOLID );
	tr = gstate->TraceBox_q2(self->s.origin,self->s.mins, self->s.maxs,pAIData->destPoint,self,MASK_MONSTERSOLID);
	float fSpeed = AI_ComputeMovingSpeed( hook );

	// forward is direction to goal, but entity should not face it
	forward = pAIData->destPoint - self->s.origin;
	forward.Normalize();

	if ( AI_IsGap( self, forward, fSpeed * 0.125f ) )
	{
		AI_StopEntity( self );
        AI_RemoveCurrentTask( self );
		return;
	}
	
	if(tr.fraction >= 1.0f)
	{
		AI_MoveTowardPoint( self, pAIData->destPoint, FALSE );
	}
	else
	{
		
		if(tr.ent && (tr.ent->flags & FL_MONSTER))
		{
			CVector NewDir;
			if(!AI_FindPathAroundEntity(self,tr.ent,NewDir))
			{
				MAPNODE_PTR Node = NODE_GetClosestNode(self,pAIData->destPoint);
				if(Node)
				{
					fXYDistance = VectorXYDistance( self->s.origin, tr.ent->s.origin );
					if (!AI_IsCloseDistance2(self, fXYDistance ))
					{
						AI_MoveTowardPoint(self, Node->position,TRUE);
						return;
					}
				}
				AI_RemoveCurrentTask( self ,FALSE);
			}
			else
			{
				AI_MoveTowardPoint(self, NewDir,TRUE);
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartMoveToSnipe
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartMoveToSnipe( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_MOVETOSNIPE );

	AI_SetStateRunning( hook );
	if ( AI_StartMove( self ) == FALSE )
	{
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		_ASSERTE( pGoalStack );
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_MOVETOSNIPE )
		{
			return;
		}
	}

	AI_Dprintf( "Starting TASKTYPE_MOVETOSNIPE.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
    _ASSERTE( pAIData );
	float fDistance = AI_ComputeDistanceToPoint( self, pAIData->destPoint );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 2.0f;
	AI_SetTaskFinishTime( hook, fTime );

	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_MoveToSnipe
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_MoveToSnipe( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( AI_IsEnemyLookingAtMe( self, self->enemy ) && rnd() > 0.5f )
	{
		// go back to hiding place
		AI_RemoveCurrentTask( self, TASKTYPE_DODGE );
		return;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	if ( AI_IsOkToMoveStraight( self, pAIData->destPoint, fXYDistance, fZDistance ) )
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
// Name:		AI_StartStrafe
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartStrafe( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( pGoalStack );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_STRAFE );

	CVector strafePoint;
	int bSuccess = AI_ComputeSideStepPoint( self, 80.0f, strafePoint );
    if ( !bSuccess )
    {
        AI_RemoveCurrentTask( self );
        return;
    }
    else
	{
		AI_SetStateRunning( hook );
		if ( AI_StartMove( self ) == FALSE )
		{
			TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
            _ASSERTE( pCurrentTask );
			if ( TASK_GetType( pCurrentTask ) != TASKTYPE_STRAFE )
			{
				return;
			}
		}
	}

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	TASK_Set( pCurrentTask, strafePoint );

	hook->nMoveCounter = 0;

	AI_Dprintf( "Starting TASKTYPE_STRAFE.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 2.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_Strafe
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Strafe( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	hook->nMoveCounter++;
	if ( hook->nMoveCounter > 10 )
	{
		AI_RestartCurrentGoal( self );
		return;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
	{
		AI_RestartCurrentGoal( self );
		return;
	}

	float fSpeed = AI_ComputeMovingSpeed( hook );

	// forward is direction to goal, but entity should not face it
	forward = pAIData->destPoint - self->s.origin;
	forward.Normalize();

	if ( AI_IsGap( self, forward, fSpeed * 0.125f ) )
	{
		AI_StopEntity( self );
        AI_RemoveCurrentTask( self );
		return;
	}

    //	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	AI_SetVelocity( self, forward, (fSpeed * fScale) );

	// make sure the AI does not get stuck for a long time
	CVector org = self->s.origin;
	float fMoveDistance = hook->last_origin.Distance(org);
	if ( fMoveDistance < (self->velocity.Length() * 0.025f) )
	{
		hook->nWaitCounter++;
		if ( (hook->nWaitCounter > 3 && fMoveDistance < 1.0f) || hook->nWaitCounter > 10 )
		{
			AI_RestartCurrentGoal( self );
		}
	}
	else
	{
		hook->nWaitCounter = 0;
	}

	hook->last_origin = self->s.origin;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartChaseEvade
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartChaseEvade( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_CHASEEVADE );

	AI_SetStateRunning( hook );
	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_CHASEEVADE )
		{
			return;
		}
	}

	userEntity_t *pEnemy = self->enemy;
	_ASSERTE( pEnemy );

	CVector directionVector;
	directionVector.x = pEnemy->s.origin.x - self->s.origin.x;
	directionVector.y = pEnemy->s.origin.y - self->s.origin.y;
	directionVector.z = pEnemy->s.origin.z - self->s.origin.z;
	directionVector.Normalize();

	//
	//	 \		  |		   /
	//	   \	  |		 /
	//		 \	  |	   /
	//		   \  |	 /
	//			 \ /
	CVector evadeVector;
	if ( rnd() > 0.5f )
	{
		evadeVector.Set( directionVector.x, directionVector.y, 0.0f );
		RotateVector2D( evadeVector, 335.0f );
	}
	else
	{
		evadeVector.Set( directionVector.x, directionVector.y, 0.0f );
		RotateVector2D( evadeVector, 25.0f );
	}

	if ( AI_IsFlyingUnit( self ) || AI_IsInWater( self ) )
	{
		// very the z height to make even more a difficult target
		if ( rnd() > 0.5f )
		{
			evadeVector.z = directionVector.z + 0.2f;
		}
		else
		{
			evadeVector.z = directionVector.z - 0.2f;
		}
	}
	evadeVector.Normalize();

	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fHalfSpeed = fSpeed * 0.5f;

	CVector evadePoint;
	VectorMA( self->s.origin, evadeVector, fHalfSpeed, evadePoint );

	tr = gstate->TraceLine_q2( self->s.origin, evadePoint, self, MASK_SOLID );
	if ( tr.fraction < 1.0f )
	{
		float fDistance = (fHalfSpeed * tr.fraction) - 16.0f;
		VectorMA( self->s.origin, evadeVector, fDistance, evadePoint );
	}

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	TASK_Set( pCurrentTask, evadePoint );

	hook->nMoveCounter = 0;
	 
	AI_Dprintf( "Starting TASKTYPE_CHASEEVADE.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 5.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ChaseEvade
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ChaseEvade( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	hook->nMoveCounter++;
	if ( hook->nMoveCounter > 10 )
	{
		AI_RestartCurrentGoal( self );
		return;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
	{
		AI_RestartCurrentGoal( self );
		return;
	}

	AI_MoveTowardPoint( self, pAIData->destPoint, FALSE );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartTakeOff
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartTakeOff( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_TAKEOFF );

	AI_Dprintf( "Starting TASKTYPE_TAKEOFF.\n" );

	frameData_t *pSequence = FRAMES_GetSequence( self, "flya" );
	if ( AI_StartSequence(self, pSequence) == FALSE )
	{
		return;
	}

	{
		if ( hook->nAttackType == ATTACK_GROUND_RANGED )
		{
			hook->nAttackType = ATTACK_AIR_RANGED;
		}
		else
		if ( hook->nAttackType == ATTACK_GROUND_MELEE )
		{
			hook->nAttackType = ATTACK_AIR_MELEE;
		}

		self->movetype = hook->nOriginalMoveType;
	}
		 
	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 2.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_TakeOff
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_TakeOff( userEntity_t *self )
{
	_ASSERTE( self );
	if ( self->s.frameInfo.frameState & (FRSTATE_STOPPED | FRSTATE_LAST) )
	{	
		AI_RemoveCurrentTask( self );
		return;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartDrop
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartDrop( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_DROP );

	AI_Dprintf( "Starting TASKTYPE_DROP.\n" );

	frameData_t *pSequence = FRAMES_GetSequence( self, "specialb" );
	if ( AI_StartSequence(self, pSequence) == FALSE )
	{
		return;
	}
	 
	{		
		hook->nOriginalMoveType = self->movetype;
		if ( hook->nAttackType == ATTACK_AIR_RANGED )
		{
			hook->nAttackType = ATTACK_GROUND_RANGED;
		}
		else
		if ( hook->nAttackType == ATTACK_AIR_MELEE )
		{
			hook->nAttackType = ATTACK_GROUND_MELEE;
		}

		self->movetype = MOVETYPE_WALK;
	}

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, 2.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_Drop
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Drop( userEntity_t *self )
{
	_ASSERTE( self );
	if ( self->s.frameInfo.frameState & (FRSTATE_STOPPED | FRSTATE_LAST) )
	{	
		AI_RemoveCurrentTask( self );
		return;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartChaseSideStepLeft
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartChaseSideStepLeft( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_CHASESIDESTEPLEFT );

	CVector sideStepPoint;
	int bSuccess = AI_ComputeChaseSideStepPoint( self, self->enemy, LEFT, sideStepPoint );
	if ( bSuccess )
	{
		AI_SetStateRunning( hook );
		if ( AI_StartMove( self ) == FALSE )
		{
			TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
            _ASSERTE( pCurrentTask );
			if ( TASK_GetType( pCurrentTask ) != TASKTYPE_CHASESIDESTEPLEFT )
			{
				return;
			}
		}

		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		TASK_Set( pCurrentTask, sideStepPoint );

		AI_Dprintf( "Starting TASKTYPE_CHASESIDESTEPLEFT.\n" );

		AI_SetOkToAttackFlag( hook, FALSE );
		AI_SetTaskFinishTime( hook, 1.0f );
		AI_SetMovingCounter( hook, 0 );

		AI_SetNextThinkTime( self, 0.1f );
	}
	else
	{
		AI_RemoveCurrentTask( self, TASKTYPE_CHARGETOWARDENEMY );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ChaseSideStepLeft
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ChaseSideStepLeft( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
    if ( AI_IsChaseVisible( self ) )
    {
        AI_RemoveCurrentTask( self, FALSE );
        return;
    }

	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsCloseDistance2(self, fXYDistance ) )
	{
		if ( AI_OneLastStep( self, pAIData->destPoint, fXYDistance * 10.0f, FALSE ) == TRUE )
        {
            AI_RemoveCurrentTask( self, FALSE );
        }
	}
    else
    {
    	if ( AI_MoveTowardPoint( self, pAIData->destPoint, FALSE ) == TRUE )
        {
            AI_RemoveCurrentTask( self, FALSE );
        }
    }
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartChaseSideStepRight
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartChaseSideStepRight( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_CHASESIDESTEPRIGHT );

	CVector sideStepPoint;
	int bSuccess = AI_ComputeChaseSideStepPoint( self, self->enemy, RIGHT, sideStepPoint );
	if ( bSuccess )
	{
		AI_SetStateRunning( hook );
		if ( AI_StartMove( self ) == FALSE )
		{
			TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
            _ASSERTE( pCurrentTask );
			if ( TASK_GetType( pCurrentTask ) != TASKTYPE_CHASESIDESTEPRIGHT )
			{
				return;
			}
		}

		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		TASK_Set( pCurrentTask, sideStepPoint );

		AI_Dprintf( "Starting TASKTYPE_CHASESIDESTEPRIGHT.\n" );

		AI_SetOkToAttackFlag( hook, FALSE );
		AI_SetTaskFinishTime( hook, 1.0f );
		AI_SetMovingCounter( hook, 0 );

		AI_SetNextThinkTime( self, 0.1f );
	}
	else
	{
		AI_RemoveCurrentTask( self, TASKTYPE_CHARGETOWARDENEMY );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ChaseSideStepRight
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ChaseSideStepRight( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
    if ( AI_IsChaseVisible( self ) )
    {
        AI_RemoveCurrentTask( self, FALSE );
        return;
    }

	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsCloseDistance2(self, fXYDistance ) )
	{
		if ( AI_OneLastStep( self, pAIData->destPoint, fXYDistance * 10.0f, FALSE ) == TRUE )
        {
            AI_RemoveCurrentTask( self, FALSE );
        }
	}
    else
    {
    	if ( AI_MoveTowardPoint( self, pAIData->destPoint, FALSE ) == TRUE )
        {
            AI_RemoveCurrentTask( self, FALSE );
        }
    }
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartChargeTowardEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartChargeTowardEnemy( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_CHARGETOWARDENEMY );

	userEntity_t *pEnemy = self->enemy;
	_ASSERTE( pEnemy );

	if ( AI_StartMove( self ) == FALSE )
	{
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		_ASSERTE( pGoalStack );
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_CHARGETOWARDENEMY )
		{
			return;
		}
	}

	float fXYDistance = VectorXYDistance( self->s.origin, pEnemy->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pEnemy->s.origin );
	if ( AI_IsOkToMoveStraight( self, pEnemy->s.origin, fXYDistance, fZDistance ) )
	{
		AI_MoveTowardPoint( self, pEnemy->s.origin, FALSE );
	}
	else
	{
		if ( !AI_FindPathToPoint( self, pEnemy->s.origin ) )
		{
			AI_RestartCurrentGoal( self );
		}
	}

	AI_Dprintf( "Starting TASKTYPE_CHARGETOWARDENEMY.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, 1.0f );		// charge for a second only
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ChargeTowardEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ChargeTowardEnemy( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *pEnemy = self->enemy;
	_ASSERTE( pEnemy );

	float fXYDistance = VectorXYDistance( self->s.origin, pEnemy->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pEnemy->s.origin );
	if ( AI_IsWithinAttackDistance( self, fXYDistance ) && fZDistance < 48.0f )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	if ( AI_IsOkToMoveStraight( self, pEnemy->s.origin, fXYDistance, fZDistance ) )
	{
		AI_MoveTowardPoint( self, pEnemy->s.origin, FALSE );
	}
	else
	{
		if ( !AI_HandleUse( self ) )
		{
			if ( hook->pPathList->pPath )
			{
				if ( AI_Move( self ) == FALSE )
				{
					if ( !AI_FindPathToPoint( self, pEnemy->s.origin ) )
					{
						AI_RestartCurrentGoal( self );
					}
				}					
			}
			else
			{
				if ( !AI_FindPathToPoint( self, pEnemy->s.origin ) )
				{
					AI_RestartCurrentGoal( self );
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartRetreatToOwner
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartRetreatToOwner( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_RETREATTOOWNER );

	userEntity_t *pOwner = hook->owner;
	_ASSERTE( pOwner );

	self->enemy = NULL;

	AI_SetStateRunning( hook );
	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_RETREATTOOWNER )
		{
			return;
		}
	}

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	TASK_Set( pCurrentTask, pOwner->s.origin );

	AI_Dprintf( "Starting TASKTYPE_RETERATTOOWNER.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );

	AI_SetTaskFinishTime( hook, -1.0f );

	AI_SetMovingCounter( hook, 0 );

    AI_RetreatToOwner( self );

}

// ----------------------------------------------------------------------------
//
// Name:		AI_RetreatToOwner
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_RetreatToOwner( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	userEntity_t *pOwner = hook->owner;
	_ASSERTE( pOwner );

	float fXYDistance = VectorXYDistance( self->s.origin, pOwner->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pOwner->s.origin );
	if ( fXYDistance < AI_GetWalkFollowDistance(hook) && fZDistance < MAX_JUMP_HEIGHT )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	if ( AI_IsOkToMoveStraight( self, pOwner->s.origin, fXYDistance, fZDistance ) )
	{
		AI_MoveTowardPoint( self, pOwner->s.origin, FALSE );
	}
	else
	{
		if ( !AI_HandleUse( self ) )
		{
			if ( hook->pPathList->pPath )
			{
				if ( AI_Move( self ) == FALSE )
				{
					if ( !AI_FindPathToPoint( self, pOwner->s.origin ) )
					{
						AI_RestartCurrentGoal( self );
					}
				}					
			}
			else
			{
				if ( !AI_FindPathToPoint( self, pOwner->s.origin ) )
				{
					AI_RestartCurrentGoal( self );
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartSequenceTransition
// Description:
//				start a transition animation sequence
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartSequenceTransition( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	// start the transition animation
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	// see if the current animation has ended
	if ( AI_IsEndAnimation( self ) )
	{
		AI_StartTransition( self, pAIData->pAnimSequence );
		return;
	}

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );
    float fAnimationTime = AI_ComputeAnimationTime( pAIData->pAnimSequence );
	AI_SetTaskFinishTime( hook, fAnimationTime + 1.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_SequenceTransition
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

__inline static void AI_ContinueAttack( userEntity_t *self )
{
	_ASSERTE( self );
	
    if ( AI_IsAlive( self ) )
    {
        playerHook_t *hook = AI_GetPlayerHook( self );

	    // make sure the current sequence is an attack sequence
	    // attack animations are named, "ataka", "atakb", "atakc", etc
	    if ( strlen( hook->cur_sequence->animation_name ) == 5 && 
		     strstr( hook->cur_sequence->animation_name, "atak" )
	       )
	    {
		    // do the attack sequence
		    if( hook->fnAttackFunc && self->enemy )
		    {
			    hook->fnAttackFunc( self );
		    }
	    }
    }
}

void AI_SequenceTransition( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( self->s.frameInfo.frameState & FRSTATE_TRANSITION )
	{
		if ( AI_IsEndAnimation( self ) )
		{
			self->s.frameInfo.frameState = 0;
			AI_RemoveCurrentTask( self );
			return;
		}
	}
	else
	{
		// wait for the current sequence to finish
		if ( AI_IsEndAnimation( self ) )
		{
			// start the transition animation
			GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
			TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
            _ASSERTE( pCurrentTask );
			AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

			AI_StartTransition( self, pAIData->pAnimSequence );
			return;
		}

		AI_ContinueAttack( self );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartFinishCurrentSequence
// Description:
//				let the current sequence finish out to the end
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartFinishCurrentSequence( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FINISHCURRENTSEQUENCE );

	if ( strstr( hook->cur_sequence->animation_name, "run" ) || 
		 strstr( hook->cur_sequence->animation_name, "walk" ) )
	{
		// check for no ground condition
		float fSpeed = AI_ComputeMovingSpeed( hook ) * 0.75f;

		CVector nextPoint;
		VectorMA( self->s.origin, self->movedir, fSpeed, nextPoint );
		if ( AI_IsGap( self, nextPoint, fSpeed * 0.2f ) )
        {
            AI_StopEntity( self );
            AI_RemoveCurrentTask( self );
            return;
        }

		//AngleToVectors( self->s.angles, forward );
		float fScale = FRAMES_ComputeFrameScale( hook );
		AI_SetVelocity( self, self->movedir, (fSpeed * fScale) );
	}

	if ( AI_IsEndAnimation( self ) )
	{
		self->s.frameInfo.frameState = 0;
		AI_RemoveCurrentTask( self );
		return;
	}

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );
    float fAnimationTime = AI_ComputeAnimationTime( hook->cur_sequence );
	AI_SetTaskFinishTime( hook, fAnimationTime + 0.2f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_FinishCurrentSequence
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_FinishCurrentSequence( userEntity_t *self )
{
	_ASSERTE( self );
	
	// in case the current animation is a movement animation (e.g. run or walk),
	// have it continue to move in the current facing direction
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !hook->cur_sequence)
	{
		AI_RemoveCurrentTask(self,FALSE);
		return;
	}

	if ( strstr( hook->cur_sequence->animation_name, "run" ) || 
		 strstr( hook->cur_sequence->animation_name, "walk" ) )
	{
		// check for no ground condition
		float fSpeed = AI_ComputeMovingSpeed( hook ) * 0.75f;

		CVector nextPoint;
		VectorMA( self->s.origin, self->movedir, fSpeed, nextPoint );
		if ( AI_IsGap( self, nextPoint, fSpeed * 0.2f ) )
        {
            AI_StopEntity( self );
            AI_RemoveCurrentTask( self, FALSE );
            return;
        }

		//AngleToVectors( self->s.angles, forward );
		float fScale = FRAMES_ComputeFrameScale( hook );
		AI_SetVelocity( self, self->movedir, (fSpeed * fScale) );
	}
	
	if ( AI_IsEndAnimation( self ) )
	{
		self->s.frameInfo.frameState = 0;
		AI_RemoveCurrentTask( self , FALSE);
		return;
	}
 
	AI_ContinueAttack( self );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_JumpTowardPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_JumpTowardPoint( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	if(AI_IsEndAnimation(self))
	{
		char szAnimation[16];
		Com_sprintf(szAnimation,sizeof(szAnimation),hook->cur_sequence->animation_name);
		AI_ForceSequence(self, szAnimation);	
	}

	float fDistance = VectorDistance( self->s.origin, pAIData->destPoint );
	if ( self->groundEntity )
	{
		if(AI_IsSidekick(hook))
		{
			if(!Check_Que(self,LANDING,5.0f))
			{
				SideKick_TalkAmbient(self, LANDING, 1);
			}
		}
		AI_UpdateCurrentNode( self );

		AI_RemoveCurrentTask( self );
		return;
	}
	else
	{
		AI_FaceTowardPoint( self, pAIData->destPoint );
	}

	ai_frame_sounds( self );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartJumpTowardPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartJumpTowardPoint( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_JUMPTOWARDPOINT );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	// NSS[2/6/00]:Check pointers
	if(!hook)
	{
		return;
	}
	
	if ( self->groundEntity )
	{
		int bVisible = AI_IsCompletelyVisible( self, pAIData->destPoint );
		if ( bVisible != COMPLETELY_VISIBLE )
		{
			AI_AddNewTaskAtFront( self, TASKTYPE_MOVEUNTILVISIBLE, pAIData->destPoint );
			return;
		}
	}

	// check if the destination point is obstructed by another entity
    if ( AI_IsPointObstructedByEntity( self, pAIData->destPoint ) )
    {
        AI_AddNewTaskAtFront( self, TASKTYPE_WAITUNTILNOOBSTRUCTION, pAIData->destPoint );
        return;
    }

    // NSS[2/6/00]:
	if(AI_IsSidekick(hook)) 
	{
		userEntity_t *pOtherSidekick = SIDEKICK_GetOtherSidekick( self );
		if(pOtherSidekick)
		{
			if(SIDEKICK_HasTaskInQue(pOtherSidekick,TASKTYPE_JUMPTOWARDPOINT))
			{
				if(!SIDEKICK_HasTaskInQue(pOtherSidekick,TASKTYPE_WAIT) && !SIDEKICK_HasTaskInQue(pOtherSidekick,TASKTYPE_WAITUNTILNOOBSTRUCTION))
				{
					AI_AddNewTaskAtFront( self, TASKTYPE_WAIT, 0.50f + (rnd() * 2.0f));
					return;
				}
			}
		}
		if(!Check_Que(self,JUMP,2.0f))
		{
			SideKick_TalkAmbient(self, JUMP, 0);
		}
	}
	
	char szAnimation[16];
	
	AI_SelectRunningAnimation( self, szAnimation );
	
	AI_ForceSequence(self, szAnimation);

	AI_FaceTowardPoint( self, pAIData->destPoint );

	forward = pAIData->destPoint - self->s.origin;
	forward.Normalize();

	float fDistance = VectorDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );

	// compute needed forward velocity to reach the point
	float fUpwardVelocity = hook->upward_vel;
	float fForwardVelocity = ai_jump_vel(self, fDistance, fZDistance, fUpwardVelocity);

	if ( pAIData->destPoint.z > self->s.origin.z )
	{
		fUpwardVelocity += fZDistance;
	}

	//fUpwardVelocity = fUpwardVelocity * (fDistance/(hook->run_speed*0.75));
	
	self->velocity = forward * fForwardVelocity;
	
	CVector Dir,Destination;

	//Dir = pAIData->destPoint - self->s.origin;
	//Dir.Normalize();
/*	Dir.y		= 0.0f;
	Dir.x		= 0.0f;
	Dir.z		= 1.0f;
	//3 frames ahead
	float fSpeed = fUpwardVelocity *0.30;
	Destination = self->s.origin + (Dir*fSpeed);
	tr = gstate->TraceLine_q2( self->s.origin, Dir, self, MASK_SOLID);

	if(tr.fraction < 1.0 && fDistance < 128.0f)
	{
		self->velocity.z = fUpwardVelocity*tr.fraction;
		self->groundEntity	= NULL;
		//self->s.origin.z	+= 10.0f*tr.fraction;
	}
	else
	{*/
	float fSpeed = fUpwardVelocity *0.30;
	Destination = self->s.origin + (Dir*fSpeed);
	self->velocity.z = fUpwardVelocity;
	self->groundEntity	= NULL;
		//self->s.origin.z	+= 10.0f;
	//}
	



	AI_Dprintf( "Starting TASKTYPE_JUMPTOWARDPOINT.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, -1 );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ShotCyclerJump
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ShotCyclerJump( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	float fDistance = VectorDistance( self->s.origin, pAIData->destPoint );
	if ( self->groundEntity || fDistance < 32.0f )
	{
		AI_RemoveCurrentTask( self );
		return;
	}
	else
	{
		// adjust direction to land on the designated spot
		CVector selfVelocity = self->velocity;
		float fZVelocity = selfVelocity.z;

		self->velocity.z = 0.0f;
		float fVelocity = selfVelocity.Length();
		if ( fVelocity > 0.0f )
		{
			forward = pAIData->destPoint - self->s.origin;
			forward.Normalize();

			AI_SetVelocity( self, forward, fVelocity );
			self->velocity.z = fZVelocity;
		}

		AI_FaceTowardPoint( self, pAIData->destPoint );
	}

	ai_frame_sounds( self );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartShotCyclerJump
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartShotCyclerJump( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_SHOTCYCLERJUMP );

	// switch weapon to shotcycler
	weapon_t *pWeapon = (weapon_t *)gstate->InventoryFindItem( self->inventory, "weapon_shotcycler" );
	_ASSERTE( pWeapon );
	self->curWeapon = (userInventory_t *)pWeapon;

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	char szAnimation[16];
	AI_SelectJumpingAnimation( self, szAnimation );
	if ( AI_StartSequence(self, szAnimation) == FALSE )
	{
		return;
	}

	AI_FaceTowardPoint( self, pAIData->destPoint );

	forward = pAIData->destPoint - self->s.origin;
	forward.Normalize();

	float fDistance = VectorDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );

	// compute needed forward velocity to reach the point
	float fForwardVelocity = hook->run_speed;
	AI_SetVelocity( self, forward, fForwardVelocity );
	self->velocity.z = hook->upward_vel;

	self->groundEntity = NULL;

	// fire the shotcyler straight down
	CVector vectorTowardGround( 0.0f, 0.0f, -1.0f );
	CVector angleTowardGround;
	VectorToAngles( vectorTowardGround, angleTowardGround );

	CVector forwardAngle;
	VectorToAngles( forward, forwardAngle );
	if( self->client )
	{
		self->client->v_angle		= forwardAngle;
		self->client->v_angle.pitch = angleTowardGround.pitch;
	}

	hook->weapon_fired			= self->curWeapon;
	self->curWeapon->use( self->curWeapon, self );

	AI_Dprintf( "Starting TASKTYPE_SHOTCYCLERJUMP.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, 3.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_MoveUntilVislbie
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_MoveUntilVisible( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !self->groundEntity )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	int bVisible = AI_IsCompletelyVisible( self, pAIData->destPoint );
	if ( bVisible == COMPLETELY_VISIBLE )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	// make this unit move forward a just a bit since touch() expects a unit to be moving
	CVector ang = self->s.angles;
	ang.x = 0;
	ang.z = 0;
	ang.AngleToForwardVector( forward );

	float fOneFrameSpeed = AI_ComputeMovingSpeed( hook ) * 0.125f;
    if ( AI_IsGap( self, self->s.origin, forward, fOneFrameSpeed ) )
    {
        AI_StopEntity( self );
        AI_RemoveCurrentTask( self );
        return;
    }

    ai_frame_sounds( self );

	AI_SetVelocity( self, forward, hook->walk_speed );

}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartMoveUntilVisible
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartMoveUntilVisible( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_MOVEUNTILVISIBLE );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);


	// make this unit move forward a just a bit since touch() expects a unit to be moving
	CVector ang = self->s.angles;
	ang.x = 0;
	ang.z = 0;
	ang.AngleToForwardVector( forward );

	float fOneFrameSpeed = AI_ComputeMovingSpeed( hook ) * 0.125f;
    if ( AI_IsGap( self, self->s.origin, forward, fOneFrameSpeed ) )
    {
        AI_StopEntity( self );
        AI_RemoveCurrentTask( self );
        return;
    }

	char szAnimation[16];
	AI_SelectWalkingAnimation( self, szAnimation );
	AI_ForceSequence(self, szAnimation, FRAME_LOOP);

	AI_SetVelocity( self, forward, hook->walk_speed );

	AI_Dprintf( "Starting TASKTYPE_MOVEUNTILVISIBLE.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, 2.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_HandleGettingOffPlatformAndTrain
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_HandleGettingOffPlatformAndTrain( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	MAPNODE_PTR pCurrentNode = NULL;

	int nNodeType;
	NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeaderAndType( self, nNodeType );
	if ( !pNodeHeader )
	{
		return FALSE;
	}
	NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	if ( pNodeList->nCurrentNodeIndex != -1 )
	{
		pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
	}
	if ( !pCurrentNode )
	{
		pCurrentNode = NODE_GetClosestNode(self);
		if ( !pCurrentNode )
		{
			return FALSE;
		}
	}

	MAPNODE_PTR pNextNode = NULL;
	for ( int i = 0; i < pCurrentNode->nNumLinks; i++ )
	{
		MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, pCurrentNode->aLinks[i].nIndex );

		float fZDistance = VectorZDistance( self->s.origin, pNode->position );
		if ( fZDistance < MAX_JUMP_HEIGHT )
		{
			pNextNode = pNode;
		}
	}

	if ( !pNextNode )
	{
		return FALSE;
	}

	AI_SetStateRunning( hook );
	AI_AddNewTaskAtFront( self, TASKTYPE_MOVETOLOCATION, pNextNode->position );

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_UsePlatform
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_UsePlatform( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	userEntity_t *pPlatform = pAIData->pEntity;
	_ASSERTE( pPlatform );
	doorHook_t *pPlatformHook = (doorHook_t *)pPlatform->userHook;
	_ASSERTE( pPlatformHook );

	if ( pPlatformHook->state == STATE_TOP )
	{
		AI_UpdateCurrentNode( self );
		AI_RemoveCurrentTask( self );

		AI_HandleGettingOffPlatformAndTrain( self );

		return;
	}

	ai_frame_sounds( self );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartUsePlatform
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartUsePlatform( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_USEPLATFORM );

	char szAnimation[16];
	AI_SelectAmbientAnimation( self, szAnimation );
	if ( AI_StartSequence(self, szAnimation, FRAME_LOOP) == FALSE )
	{
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_USEPLATFORM.\n" );

	// make this unit move forward a just a bit since touch() expects a unit to be moving
	CVector ang = self->s.angles;
	ang.x = 0;
	ang.z = 0;
	ang.AngleToForwardVector( forward );
	AI_SetVelocity( self, forward, hook->run_speed );

	// since ai units do not touch other entities in the world automatically, we have to 
	// use them manually, this includes doors that are opened automatically for a player
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	userEntity_t *pPlatform = pAIData->pEntity;
	_ASSERTE( pPlatform );
	door_use( pPlatform, self, self );

	AI_ZeroVelocity( self );
	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, 5.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_MoveDown
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_MoveDown( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	float fDistance = VectorDistance( self->s.origin, pAIData->destPoint );
//	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );// SCG[1/23/00]: not used
	if ( AI_IsCloseDistance2(self, fDistance ) )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	self->groundEntity = NULL;

	forward = pAIData->destPoint - self->s.origin;
	forward.Normalize();
	float fSpeed = hook->run_speed;
	float fScale = FRAMES_ComputeFrameScale( hook );
	AI_SetVelocity( self, forward, (fSpeed * fScale) );

	hook->last_origin = self->s.origin;

	AI_UpdateCurrentNode( self );

	ai_frame_sounds( self );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartMoveDown
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartMoveDown( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_MOVEDOWN );

	char szAnimation[16];
	AI_SelectRunningAnimation( self, szAnimation );
	if ( AI_StartSequence(self, szAnimation, FRAME_LOOP) == FALSE )
	{
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_MOVEDOWN.\n" );

	self->groundEntity = NULL;

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	forward = pAIData->destPoint - self->s.origin;
	forward.Normalize();

	float fSpeed = hook->run_speed;
	float fScale = FRAMES_ComputeFrameScale( hook );
	AI_SetVelocity( self, forward, (fSpeed * fScale) );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, 3.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_Use
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Use( userEntity_t *self )
{
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartUse
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartUse( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	_ASSERTE( TASK_GetType( pCurrentTask ) == TASKTYPE_USE );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pEntity );

	if ( pAIData->pEntity->use )
	{
		pAIData->pEntity->use( pAIData->pEntity, self, self );
	}

	AI_SetNextThinkTime( self, 0.1f );

	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	if ( GOAL_GetType( pCurrentGoal ) == GOALTYPE_USE )
	{
		GOAL_Satisfied( pCurrentGoal );
	}

	AI_RemoveCurrentTask( self, FALSE );

}

// ----------------------------------------------------------------------------
//
// Name:		AI_Cower
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_Cower( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->fnCower )
	{
		hook->fnCower( self );
	}
	else
	{
		AI_RemoveCurrentTask(self,TASKTYPE_WANDER,self);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartStartCower
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartCower( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	_ASSERTE( TASK_GetType( pCurrentTask ) == TASKTYPE_COWER );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, 10.0f );
	AI_SetMovingCounter( hook, 0 );

	if ( hook->fnStartCower )
	{
		hook->fnStartCower( self );
	}
	else
	{
		// no cowering function, just do ambients
		char szAnimation[16];
		AI_SelectAmbientAnimation( self, szAnimation );
		AI_ForceSequence(self, szAnimation, FRAME_LOOP);
	}

	AI_SetNextThinkTime( self, 0.1f );

}

// ----------------------------------------------------------------------------
//
// Name:		AI_HandleGettingOffLadder
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_HandleGettingOffLadder( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	_ASSERTE( pCurrentGoal );
	AIDATA_PTR pAIData = GOAL_GetData( pCurrentGoal );

	CVector destPoint;
	if ( pAIData->pEntity )
	{
		destPoint = pAIData->pEntity->s.origin;
		AI_FindPathToEntity( self, pAIData->pEntity );
	}
	else
	if ( pAIData->destPoint.Length() > 0.0f )
	{
		destPoint = pAIData->destPoint;
		AI_FindPathToPoint( self, pAIData->destPoint );
	}
	else
	{
		// try the current task
		TASK_PTR pCurrentTask = GOAL_GetCurrentTask( pCurrentGoal );
		pAIData = TASK_GetData( pCurrentTask );
		if ( pAIData->pEntity )
		{
			destPoint = pAIData->pEntity->s.origin;
			AI_FindPathToEntity( self, pAIData->pEntity );
		}
		else
		if ( pAIData->destPoint.Length() > 0.0f )
		{
			destPoint = pAIData->destPoint;
			AI_FindPathToPoint( self, pAIData->destPoint );
		}
		else
		{
			PATHLIST_KillPath(hook->pPathList);	
		}
	}

	if ( hook->pPathList->pPath )
	{
		NODELIST_PTR pNodeList = hook->pNodeList;
		_ASSERTE( pNodeList );
		NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
		_ASSERTE( pNodeHeader );

		PATHNODE_PTR pFirstPathNode = hook->pPathList->pPath;
		PATHNODE_PTR pSecondPathNode = hook->pPathList->pPath->next_node;

		MAPNODE_PTR pFirstNode = NODE_GetNode( pNodeHeader, pFirstPathNode->nNodeIndex );
		_ASSERTE( pFirstNode );

		float fDistance = VectorDistance( self->s.origin, pFirstNode->position );
		if ( !AI_IsCloseDistance2(self, fDistance ) )
        {
			CVector mins( -12.0f, -12.0f, -12.0f );
			CVector maxs( 12.0f, 12.0f, 12.0f );
			CVector viewPos = self->s.origin;
			viewPos.z += 24.0f;
			tr = gstate->TraceBox_q2( viewPos, mins, maxs, pFirstNode->position, self, MASK_SOLID );
			if ( tr.fraction < 1.0f && tr.ent && _stricmp( tr.ent->className, "worldspawn" ) != 0 )
			{
				// dest entity is blocking the way, wait for it to move out of the way
				AI_AddNewTaskAtFront( self, TASKTYPE_WAITONLADDERFORNOCOLLISION, destPoint );
				return TRUE;
			}
			else
			if ( !AI_IsGroundBelowBetween( self, self->s.origin, pFirstNode->position ) )
			{
				// make this entity jump forward
				AI_AddNewTaskAtFront( self, TASKTYPE_JUMPTOWARDPOINT, pFirstNode->position );
				return TRUE;
			}
        }
        else
		if ( pFirstNode->node_type & NODETYPE_LADDER ) 
		{
			MAPNODE_PTR pSecondNode = NULL;
			if ( pSecondPathNode )
			{
				pSecondNode = NODE_GetNode( pNodeHeader, pSecondPathNode->nNodeIndex );
			}

			MAPNODE_PTR pNextNode = pSecondNode;
			if ( pNextNode && !(pNextNode->node_type & NODETYPE_LADDER) )
			{
				float fDistance = VectorDistance( pNextNode->position, destPoint );

				CVector mins( -12.0f, -12.0f, -12.0f );
				CVector maxs( 12.0f, 12.0f, 12.0f );
				CVector viewPos = self->s.origin;
				viewPos.z += 24.0f;
				tr = gstate->TraceBox_q2( viewPos, mins, maxs, pNextNode->position, self, MASK_SOLID );
				if ( tr.fraction < 1.0f && tr.ent && _stricmp( tr.ent->className, "worldspawn" ) != 0 )
				{
					// dest entity is blocking the way, wait for it to move out of the way
					AI_AddNewTaskAtFront( self, TASKTYPE_WAITONLADDERFORNOCOLLISION, destPoint );
					return TRUE;
				}
				else
				if ( !AI_IsGroundBelowBetween( self, self->s.origin, pNextNode->position ))
				{
					// make this entity jump forward
					AI_AddNewTaskAtFront( self, TASKTYPE_JUMPTOWARDPOINT, pNextNode->position );
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_UpLadder
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_UpLadder( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pVoid );

	MAPNODE_PTR pNode = (MAPNODE_PTR)pAIData->pVoid;
	_ASSERTE( pNode );

	float fDistance = VectorDistance( self->s.origin, pNode->position );
	if(AI_IsSidekick(hook))
	{
		
		if(AI_IsInSmallSpace(self))
		{
			float fDifference = fabs(pNode->position.z - self->s.origin.z );

			if (fDifference > -1.0f && fDifference < 1.0f || (self->s.origin.z - 12) >= pNode->position.z  )
			{
				AI_UpdateCurrentNode( self );
				AI_RemoveCurrentTask( self );
				AI_HandleGettingOffLadder( self );
				return;
			}		
		}
		else
		{
			if ( (self->s.origin.z - 12) >= pNode->position.z )
			{
				AI_UpdateCurrentNode( self );
				AI_RemoveCurrentTask( self );
				AI_HandleGettingOffLadder( self );
				return;
			}
		}
	}
	else
	{
		if ( (self->s.origin.z - 16) >= pNode->position.z )
		{
			AI_UpdateCurrentNode( self );
			if(!AI_IsSidekick(hook))
			{
				AI_SetOkToAttackFlag( hook, TRUE );
			}
			AI_RemoveCurrentTask( self );
			AI_HandleGettingOffLadder( self );
			return;
		}
	
	}
	float fSpeed = AI_ComputeMovingSpeed( hook );
	if(!AI_IsSidekick(hook) && fSpeed <300.0f)
	{
		fSpeed = 300.0f;
	}

	NODEDATA_PTR pNodeData = NODE_GetData( pNode );
	CVector facingPoint;
	VectorMA( self->s.origin, pNodeData->vector, 32.0f, facingPoint );
	AI_FaceTowardPoint( self, facingPoint );

	forward = pNode->position - self->s.origin;
    forward.Normalize();
	float xyDistance = VectorXYDistance(self->s.origin,pNode->position);
	if(xyDistance > 10.0f)
	{
		if(AI_IsSidekick(hook))
		{
			forward.x = pNodeData->vector.x * 0.1f;
			forward.y = pNodeData->vector.y * 0.1f;
		}
		else
		{
			forward.x = pNodeData->vector.x * 0.2f;
			forward.y = pNodeData->vector.y * 0.2f;	
		}
	}
	forward.Normalize();
	forward.z = 1.0f;
	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	CVector TraceDestination;
	TraceDestination = self->s.origin + (forward * (fSpeed *0.1));
	tr = gstate->TraceLine_q2( self->s.origin, TraceDestination, self, MASK_MONSTERSOLID );
	if(tr.fraction < 0.65f)
	{
		AI_RemoveCurrentTask(self,FALSE);
		AI_HandleGettingOffLadder( self );

	}
	if(AI_IsSidekick(hook))
	{
		AI_SetVelocity( self, forward, (fSpeed * 0.50) );
	}
	else
	{
		AI_SetVelocity( self, forward, fSpeed);
	}

	hook->last_origin = self->s.origin;
	if(AI_IsEndAnimation(self))
	{
		if(AI_IsSidekick(hook))
		{
			AI_ForceSequence( self, "ladderup" );
			
		}
		else
		{
			AI_ForceSequence( self, "walka" );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartUpLadder
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartUpLadder( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	_ASSERTE( TASK_GetType( pCurrentTask ) == TASKTYPE_UPLADDER );

	AI_SetStateRunning( hook );
	if(AI_IsSidekick(hook))
	{
		AI_ForceSequence( self, "ladderup" );
		
	}
	else
	{
		AI_ForceSequence( self, "walka" );
	}

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pVoid );
	MAPNODE_PTR pNode = (MAPNODE_PTR)pAIData->pVoid;
	_ASSERTE( pNode );
	float fDistance = VectorDistance( self->s.origin, pNode->position );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / (fSpeed*0.25f) ) + 1.0f;
	AI_SetTaskFinishTime( hook, -1 );
	if(!AI_IsSidekick(hook))
	{
		AI_SetOkToAttackFlag( hook, FALSE );
	}
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_DownLadder
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_DownLadder( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pVoid );

	MAPNODE_PTR pNode = (MAPNODE_PTR)pAIData->pVoid;
	_ASSERTE( pNode );

	float fDistance = VectorDistance( self->s.origin, pNode->position );
	if (self->s.origin.z <= pNode->position.z + 4.0f)
	{
		AI_UpdateCurrentNode( self );

		AI_RemoveCurrentTask( self );
		AI_HandleGettingOffLadder( self );
		AI_AddNewTaskAtFront(self,TASKTYPE_WAIT,0.5f);
		return;
	}
	float fSpeed = AI_ComputeMovingSpeed( hook );
	if ( AI_IsCloseDistance2(self, fDistance ) )
	{
		fSpeed = fDistance * 10.0f;
	}

	NODEDATA_PTR pNodeData = NODE_GetData( pNode );

	//CVector facingPoint;
	//VectorMA( self->s.origin, pNodeData->vector, 32.0f, facingPoint );
	//AI_FaceTowardPoint( self, facingPoint );

	forward = pNode->position - self->s.origin;
    forward.Normalize();
//	forward.x = pNodeData->vector.x * 0.1f;
//	forward.y = pNodeData->vector.y * 0.1f;
//	forward.z = -1.0f;
//	forward.Normalize();

	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	AI_SetVelocity( self, forward, (fSpeed * 0.25) );

	hook->last_origin = self->s.origin;

	if(AI_IsEndAnimation(self))
	{
		AI_ForceSequence( self, "ladderdn" );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartDownLadder
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartDownLadder( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	_ASSERTE( TASK_GetType( pCurrentTask ) == TASKTYPE_DOWNLADDER );

	AI_SetStateRunning( hook );
	AI_ForceSequence( self, "ladderdn" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pVoid );
	MAPNODE_PTR pNode = (MAPNODE_PTR)pAIData->pVoid;
	_ASSERTE( pNode );
	//float fDistance = VectorDistance( self->s.origin, pNode->position );
	//float fSpeed = AI_ComputeMovingSpeed( hook );
	//float fTime = (fDistance / fSpeed*0.10) + 1.0f;
	AI_SetTaskFinishTime( hook, -1 );
	AI_ZeroVelocity(self);
	CVector Dir,Angles;
	float Delta_Angles;
	Angles.Set(0.0f,0.0f,0.0f);
	
	Delta_Angles = 0.0f;
	
	while(Delta_Angles < 360.0f)
	{
		Angles.yaw = Delta_Angles;
		Angles.AngleToForwardVector(Dir);
		Dir = pNode->position + (Dir * 32.0f);
		tr = gstate->TraceBox_q2( pNode->position, self->s.mins,self->s.maxs,Dir, self, MASK_SOLID | CONTENTS_MONSTER );
		if(tr.fraction < 1.0f && (tr.contents & CONTENTS_LADDER))
		{
			Dir.Set(0.0f,0.0f,0.0f);
			Dir = tr.plane.normal;
			Dir.Negate();
			VectorToAngles(Dir,Angles);
			self->s.angles.yaw = Angles.yaw;
			break;
		}
		Delta_Angles += 15.0f;
		
		// NSS[3/6/00]:Just in case we have ladder nodes but no ladder... uhhhhg!  HAHAHAHAHAHAHAHAHAHAAHAH! <heh>
		if(Delta_Angles >= 360)
		{
			Dir = pNode->position - self->s.origin;
			Dir.Normalize();
			VectorToAngles(Dir,Angles);
			
			if((self->s.angles.yaw - Angles.yaw) < 110)
			{
				Dir = self->s.origin - pNode->position;
				Dir.Normalize();
				VectorToAngles(Dir,Angles);
			}
			self->s.angles.yaw = Angles.yaw;
		}
	}
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartWaitOnLadderForNoCollision
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartWaitOnLadderForNoCollision( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	_ASSERTE( TASK_GetType( pCurrentTask ) == TASKTYPE_WAITONLADDERFORNOCOLLISION );

	char szAnimation[16];
	AI_SelectAmbientAnimation( self, szAnimation );
	if ( AI_StartSequence( self, szAnimation, FRAME_LOOP ) == FALSE )
	{
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_WAITONLADDERFORNOCOLLISION.\n" );

	AI_ZeroVelocity( self );

	// hack to have this unit sit here 

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, 2.0f );
	AI_SetMovingCounter( hook, 0 );
	
}

// ----------------------------------------------------------------------------
//
// Name:		AI_WaitOnLadderForNoCollision
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_WaitOnLadderForNoCollision( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	// NSS[3/6/00]:Try to stay put damn you!  heh.
	AI_ZeroVelocity( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->destPoint );

	CVector mins( -12.0f, -12.0f, -12.0f );
	CVector maxs( 12.0f, 12.0f, 12.0f );
	CVector viewPos = self->s.origin;
	viewPos.z += 24.0f;
	tr = gstate->TraceBox_q2( viewPos, mins, maxs, pAIData->destPoint, self, MASK_SOLID | CONTENTS_MONSTER );
	if ( tr.fraction >= 1.0f )
	{
		AI_UpdateCurrentNode( self );

		AI_RemoveCurrentTask( self );

		AI_HandleGettingOffLadder( self );
        return;
	}

    AI_SetTaskFinishTime( hook, 1.0f );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartPlaySoundToEnd
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartPlaySoundToEnd( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	_ASSERTE( TASK_GetType( pCurrentTask ) == TASKTYPE_PLAYSOUNDTOEND );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

    // play sound
    if ( strstr( pAIData->pString, "mp3" ) )
    {
        gstate->StartMP3( pAIData->pString, 2, 1.0, 1 );
    }
    else
    if ( strstr( pAIData->pString, "wav" ) )
    {
        gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex(pAIData->pString), 1.0f, 
                                  hook->fMinAttenuation, hook->fMaxAttenuation );
    }

	AI_Dprintf( "Starting TASKTYPE_PLAYSOUNDTOEND.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, pAIData->fValue );
	AI_SetMovingCounter( hook, 0 );

}

// ----------------------------------------------------------------------------
//
// Name:		AI_PlaySoundToEnd
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_PlaySoundToEnd( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	if ( gstate->time > hook->fTaskFinishTime )
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		if ( GOAL_GetType( pCurrentGoal ) == GOALTYPE_PLAYSOUNDTOEND )
		{
			GOAL_Satisfied( pCurrentGoal );
		}

		AI_RemoveCurrentTask( self );
		return;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartTeleport
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartTeleport( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_TELEPORT );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	_ASSERTE( TASK_GetType( pCurrentTask ) == TASKTYPE_TELEPORT );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
    
    self->s.origin = pAIData->destPoint;

	AI_RemoveCurrentTask( self );
}

void AI_Teleport( userEntity_t *self )
{
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartPlayAnimationAndSound
// Description:
//              starts both a sound and an animation, this task will end when 
//              the animation stops
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartPlayAnimationAndSound( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_PLAYANIMATIONANDSOUND );

    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	frameData_t *pSequence = pAIData->pAnimSequence;
    if ( pSequence )
    {
	    AI_ForceSequence( self, pSequence, pSequence->flags );
    }

    if ( pAIData->pString )
    {
        if ( strstr( pAIData->pString, "mp3" ) )
        {
            gstate->StartMP3( pAIData->pString, 2, 1.0, 1 );
        }
        else
        if ( strstr( pAIData->pString, "wav" ) )
        {
    	    gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex(pAIData->pString), 
                                      1.0f, hook->fMinAttenuation, hook->fMaxAttenuation );
        }
    }

    AI_Dprintf( "Starting TASKTYPE_PLAYANIMATIONANDSOUND.\n" );
    
	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );

    if ( pSequence )
    {
	    float fTime = AI_ComputeAnimationTime( pSequence );
        AI_SetTaskFinishTime( hook, fTime + 1.0f );
	}
    else
    {
        AI_SetTaskFinishTime( hook, 1.0f );
    }

    AI_SetMovingCounter( hook, 0 );

}

void AI_PlayAnimationAndSound( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

    // get the goal.
    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

    int bEndAnimation = FALSE;
    if ( hook->cur_sequence == pAIData->pAnimSequence )
    {
	    if ( self->s.frameInfo.frameState & (FRSTATE_STOPPED | FRSTATE_LAST) )
	    {
            bEndAnimation = TRUE;	
        }
    }
    else
    {
        bEndAnimation = TRUE;
    }

    if ( bEndAnimation == TRUE )
    {
        AI_RemoveCurrentTask( self );
    }

}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartPlaySound
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartPlaySound( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_PLAYSOUND );

    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

    if ( pAIData->pString )
    {
        if ( strstr( pAIData->pString, "mp3" ) )
        {
            gstate->StartMP3( pAIData->pString, 2, 1.0, 1 );
        }
        else
        if ( strstr( pAIData->pString, "wav" ) )
        {
	        gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex(pAIData->pString), 
                                      1.0f, hook->fMinAttenuation, hook->fMaxAttenuation );
        }
    }

    AI_Dprintf( "Starting TASKTYPE_PLAYSOUND.\n" );
    
	AI_SetNextThinkTime( self, 0.1f );

	AI_RemoveCurrentTask( self );

}

void AI_PlaySound( userEntity_t *self )
{
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartMoveToEntityUntilVisible
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartMoveToEntityUntilVisible( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_MOVETOENTITYUNTILVISIBLE );

	AI_SetStateRunning( hook );
	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_MOVETOENTITYUNTILVISIBLE )
		{
			return;
		}
	}

	AI_Dprintf( "Starting TASKTYPE_MOVETOENTITYUNTILVISIBLE.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	userEntity_t *pDestEntity = pAIData->pEntity;
	_ASSERTE( pDestEntity );
	float fDistance = AI_ComputeDistanceToPoint( self, pDestEntity->s.origin );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 2.0f;
	AI_SetTaskFinishTime( hook, fTime );

	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_MoveToEntityUntilVisible
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_MoveToEntityUntilVisible( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	userEntity_t *pDestEntity = pAIData->pEntity;
	_ASSERTE( pDestEntity );
	
	CVector destPoint( pDestEntity->s.origin );
	if ( destPoint.Length() == 0 )
	{
		destPoint.x = (pDestEntity->absmax.x + pDestEntity->absmin.x) * 0.5f;
		destPoint.y = (pDestEntity->absmax.y + pDestEntity->absmin.y) * 0.5f;
		destPoint.z = (pDestEntity->absmax.z + pDestEntity->absmin.z) * 0.5f;
	}

	float fXYDistance = VectorXYDistance( self->s.origin, destPoint );
	float fZDistance = VectorZDistance( self->s.origin, destPoint );
	if ( ( fXYDistance <= 512.0f && AI_IsCompletelyVisible( self, pDestEntity ) ) ||
         ( fXYDistance < 256.0f && fZDistance < MAX_JUMP_HEIGHT ) )
    {
		AI_RemoveCurrentTask( self );
		return;
	}

	if ( AI_IsOkToMoveStraight( self, destPoint, fXYDistance, fZDistance ) )
	{
		AI_MoveTowardPoint( self, destPoint, FALSE );
	}
	else
	{
		if ( hook->pPathList->pPath )
		{
			if ( AI_Move( self ) == FALSE )
			{
				if ( !AI_FindPathToPoint( self, destPoint ) )
				{
					AI_RestartCurrentGoal( self );
				}
			}
		}
		else
		{
			if ( !AI_FindPathToPoint( self, destPoint ) )
			{
				AI_RestartCurrentGoal( self );
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartActionSpawn
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ActionSpawn( userEntity_t *self ) {}
void AI_StartActionSpawn( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	_ASSERTE( TASK_GetType( pCurrentTask ) == TASKTYPE_ACTION_SPAWN );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pAction );

	CAction *pAction = pAIData->pAction;

	AI_RemoveCurrentTask( self, FALSE );

	// nine parameters
	CParameter *pParameter0 = pAction->GetParameter( 0 );
	_ASSERTE( pParameter0 );
	CParameter *pParameter1 = pAction->GetParameter( 1 );
	_ASSERTE( pParameter1 );
	CParameter *pParameter2 = pAction->GetParameter( 2 );
	_ASSERTE( pParameter2 );
	CParameter *pParameter3 = pAction->GetParameter( 3 );
	_ASSERTE( pParameter3 );
	CParameter *pParameter4 = pAction->GetParameter( 4 );
	_ASSERTE( pParameter4 );
	CParameter *pParameter5 = pAction->GetParameter( 5 );
	_ASSERTE( pParameter5 );
	CParameter *pParameter6 = pAction->GetParameter( 6 );
	_ASSERTE( pParameter6 );

	CParameter *pParameter7 = pAction->GetParameter( 7 );
	CVector spawnPoint( pParameter2->GetValue(), pParameter3->GetValue(), pParameter4->GetValue() );
	CVector facingAngle( 0.0f, pParameter5->GetValue(), 0.0f );
	if ( pParameter7 )
	{
    	CParameter *pParameter8 = pAction->GetParameter( 8 );
        if ( pParameter8 )
        {
		    SPAWN_AI( self, pParameter0->GetString(), pParameter1->GetString(), spawnPoint, facingAngle, 
                      pParameter6->GetValue(), pParameter7->GetString(), pParameter8->GetString() );
	    }
        else
        {
		    SPAWN_AI( self, pParameter0->GetString(), pParameter1->GetString(), spawnPoint, facingAngle, 
                      pParameter6->GetValue(), pParameter7->GetString() );
        }
    }
	else
	{
		SPAWN_AI( self, pParameter0->GetString(), pParameter1->GetString(), spawnPoint, facingAngle, pParameter6->GetValue() );
	}

	// SCG[3/6/00]: Increase the monster count
	IncrementMonsterCount();
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
void AI_ActionSetMovingAnimation( userEntity_t *self ) {}
void AI_StartActionSetMovingAnimation( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	_ASSERTE( TASK_GetType( pCurrentTask ) == TASKTYPE_ACTION_SETMOVINGANIMATION );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pAction );

	CAction *pAction = pAIData->pAction;

	// one parameters
	CParameter *pParameter = pAction->GetParameter( 0 );
	_ASSERTE( pParameter );
	
	hook->pMovingAnimation = FRAMES_GetSequence( self, pParameter->GetString() );
	
	AI_RemoveCurrentTask( self, FALSE );
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
void AI_ActionSetState( userEntity_t *self ) {}
void AI_StartActionSetState( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *phook = AI_GetPlayerHook( self );
	playerHook_t *hook;

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( phook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	_ASSERTE( TASK_GetType( pCurrentTask ) == TASKTYPE_ACTION_SETSTATE );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pAction );

	CAction *pAction = pAIData->pAction;

	// two parameters
	CParameter *pParameter0 = pAction->GetParameter( 0 ); //Name of entity to call 
	_ASSERTE( pParameter0 );
	CParameter *pParameter1 = pAction->GetParameter( 1 ); //Action to do upon entity
	_ASSERTE( pParameter1 );
	
	_ASSERTE( strlen( pParameter0->GetString() ) > 0 );
	
	userEntity_t *pEntity = UNIQUEID_Lookup( pParameter0->GetString() );
	if ( !AI_IsAlive( pEntity ) || (_stricmp(pEntity->className,"freed") == 0) )
	{
		gstate->Con_Dprintf( "Uniqueid: %s not found\n", pParameter0->GetString() );
		AI_RemoveCurrentTask( self );
		return;
	}
	
	hook = AI_GetPlayerHook( pEntity );

	// NSS[2/3/00]:Check the POINTERS!!!!!!!
	if(!hook)
	{
		AI_RemoveCurrentTask( self );
	}

	int nFlag = 0;
	if ( _stricmp( pParameter1->GetString(), "ignore_player" ) == 0 )
	{	
		
		hook->ai_flags |= AI_IGNORE_PLAYER;

		// do not attack anymore if in an attack state
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		_ASSERTE( pGoalStack );
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
		if(pCurrentGoal)
		{
			GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
			if ( nGoalType == GOALTYPE_KILLENEMY )
			{
				self->enemy = NULL;
				GOAL_Satisfied( pCurrentGoal );
				AI_RemoveCurrentGoal( pEntity );
			}
	
			if ( AI_IsSidekick( hook ) )
			{
				if ( nGoalType == GOALTYPE_FOLLOW )
				{
					AI_AddNewGoal( pEntity, GOALTYPE_IDLE );
				}
			}
		}
	}
	if ( _stricmp( pParameter1->GetString(), "aggressive" ) == 0 )
	{
		hook->ai_flags &= ~AI_IGNORE_PLAYER;

		if ( AI_IsSidekick( hook ) )
		{
			GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
			_ASSERTE( pGoalStack );
			GOALSTACK_ClearAllGoals( pGoalStack );

			SIDEKICK_FindOwner( pEntity );
		}
	}
	if ( _stricmp( pParameter1->GetString(), "pathfollow" ) == 0 )
	{
		nFlag |= SPAWN_PATHFOLLOW;
		
		// get the target if available
		CParameter *pParameter2 = pAction->GetParameter( 2 );
		if ( pParameter2 )
		{
			pEntity->target = pParameter2->GetString();
		}
		//If we are the same entity running and being effected by the script
		//
		if(pEntity == self)
		{
			AI_RemoveCurrentTask(self);		
			AI_SetSpawnValue( pEntity, nFlag );
			AI_StartNextTask( pEntity );
			return;
		}
		else
		{
			AI_SetSpawnValue( pEntity, nFlag );
			AI_StartNextTask( pEntity );
		}
	}
	AI_RemoveCurrentTask( self );
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
void AI_ActionDie( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	_ASSERTE( hook->cur_sequence );
	
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pString );

	if ( _stricmp( hook->cur_sequence->animation_name, pAIData->pString ) != 0 )
	{
		// waiting for the current sequence to finish
		if ( self->s.frameInfo.frameState & FRSTATE_LAST ||
			 self->s.frameInfo.frameState & FRSTATE_STOPPED )
		{
			AI_ForceSequence( self, pAIData->pString );
		}
	}
	else
	{
		ai_frame_sounds( self );

		if ( self->s.frameInfo.frameState & FRSTATE_LAST ||
			 self->s.frameInfo.frameState & FRSTATE_STOPPED )
		{
			if ( hook->pRespawnData )
			{
				self->think = SPAWN_RespawnThink;
				AI_SetNextThinkTime( self, 5.0f );
			}
			else
			{
				self->pain = AI_StartBodyPain;

				if ( hook->pGoals )
				{
					hook->pGoals = GOALSTACK_Delete( hook->pGoals );
				}
				if ( hook->pScriptGoals )
				{
					hook->pScriptGoals = GOALSTACK_Delete( hook->pScriptGoals );
				}

                
                // set up fake damage
                gstate->damage_inflicted = hook->base_health;
                gstate->damage_vector.Set(1.0,-1.0,1.0);  

				alist_remove( self );
                
                self->pain( self, NULL, 0, gstate->damage_inflicted);

			}

			gstate->LinkEntity( self );

			AI_Dprintf( "Finished TASKTYPE_ACTION_DIE.\n" );
		}
	}
}

void AI_StartActionDie( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	_ASSERTE( TASK_GetType( pCurrentTask ) == TASKTYPE_ACTION_DIE );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pString );

	//////////////////////////////////////////////////////////////
	//	not too much damage, so just die
	//////////////////////////////////////////////////////////////
	if ( AI_IsFlyingUnit( self ) || AI_IsInWater( self ) )
	{
		self->avelocity.Zero();
		self->s.angles.Zero();
		hook->dflags &= ~(DFL_FACEENEMY | DFL_FACEPITCH | DFL_FACEOWNER | DFL_FACEENEMY);
		hook->dflags |= DFL_FACENONE;
	}

	self->view_ofs.Set( 0.0f, 0.0f, -8.0f );
	self->deadflag		= DEAD_DYING;
	self->takedamage	= DAMAGE_YES;
	//self->health		= 32767;

	self->movetype		= MOVETYPE_TOSS;
	gstate->LinkEntity(self);


	if ( AI_StartSequence( self, pAIData->pString ) == FALSE )
	{
        gstate->Con_Printf("sequence %s failed\n",pAIData->pString);

		AI_RemoveCurrentTask( self );
		return;
	}

	ai_frame_sounds( self );

	self->think = AI_DeathThink;
	self->hacks	= gstate->time + 4.0f;
	self->delay = gstate->time + 2.0f;
	self->nextthink = gstate->time + 1.0f;

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 3.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_ActionPlayAnimation
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ActionPlayAnimation( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( pGoalStack );
    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	if ( _stricmp( self->className, "monster_cryotech" ) == 0 )
	{
		if ( _stricmp( hook->cur_sequence->animation_name, "bambb" ) == 0 )
		{
			cryotech_bambb( self );
		}
		else
		if ( _stricmp( hook->cur_sequence->animation_name, "bambc" ) == 0 )
		{
			cryotech_bambc( self );
		}
	}

	if ( pAIData->fValue > 0.0f )
	{
		if ( gstate->time > hook->fTaskFinishTime )
		{
			AI_RemoveCurrentTask( self );
			return;
		}
	}
	else
	{
		if ( AI_IsEndAnimation( self ) )
		{
			AI_RemoveCurrentTask( self );
			return;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:        AI_StartActionPlayAnimation
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartActionPlayAnimation(userEntity_t *self) 
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_ACTION_PLAYANIMATION );

    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	frameData_t *pSequence = FRAMES_GetSequence( self, pAIData->pString );
	if ( !pSequence )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	
	if ( pAIData->fValue > 0.0f )
	{
	    if ( AI_StartSequence( self, pSequence, FRAME_LOOP ) == FALSE )
	    {
		    return;
	    }
	}
	else
	{
	    if ( AI_StartSequence( self, pSequence ) == FALSE )
	    {
		    return;
	    }
	}

    AI_Dprintf( "Starting TASKTYPE_ACTION_PLAYANIMATION.\n" );

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, pAIData->fValue );
	//AI_SetTaskFinishTime( hook, -1.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_ActionPlayPartialAnimation
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ActionPlayPartialAnimation( userEntity_t *self )
{
	_ASSERTE( self );

	if ( AI_IsEndAnimation( self ) )
	{
		AI_RemoveCurrentTask( self );
		return;
	}
}

// ----------------------------------------------------------------------------
//
// Name:        AI_StartActionPlayPartialAnimation
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartActionPlayPartialAnimation(userEntity_t *self) 
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_ACTION_PLAYPARTIALANIMATION );

    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	frameData_t *pSequence = FRAMES_GetSequence( self, pAIData->pString );
	if ( !pSequence )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	if ( pAIData->fValue > 0.0f )
	{
		if ( AI_StartSequence( self, pSequence, pAIData->nValue, pAIData->fValue, FRAME_ONCE ) == FALSE )
		{
			return;
		}
	}
	else
	{
		if ( AI_StartSequence( self, pSequence, pAIData->nValue, -1, FRAME_ONCE ) == FALSE )
		{
			return;
		}
	}

    AI_Dprintf( "Starting TASKTYPE_ACTION_PLAYPARTIALANIMATION.\n" );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_ActionPlaySound
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ActionPlaySound( userEntity_t *self )
{
	_ASSERTE( self );

}

// ----------------------------------------------------------------------------
//
// Name:        AI_StartActionPlaySound
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartActionPlaySound(userEntity_t *self) 
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_ACTION_PLAYSOUND );

    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pAction );

	CAction *pAction = pAIData->pAction;

	userEntity_t *pEntity = self;
	// two parameters
	CParameter *pParameter0 = pAction->GetParameter( 0 );
	_ASSERTE( pParameter0 );
	CParameter *pParameter1 = pAction->GetParameter( 1 );
	if ( pParameter1 )
	{
		_ASSERTE( strlen( pParameter1->GetString() ) > 0 );
		pEntity = UNIQUEID_Lookup( pParameter1->GetString() );
		if ( !AI_IsAlive( pEntity ) )
		{
			gstate->Con_Dprintf( "Uniqueid: %s not found\n", pParameter1->GetString() );
			AI_RemoveCurrentTask( self );
			return;
		}
	}

    if ( strstr( pAIData->pString, "mp3" ) )
    {
        gstate->StartMP3( pParameter0->GetString(), 2, 1.0, 1 );
    }
    else
    if ( strstr( pAIData->pString, "wav" ) )
    {
	    gstate->StartEntitySound(pEntity, CHAN_AUTO, gstate->SoundIndex(pParameter0->GetString()), 
                                 1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
    }

    AI_Dprintf( "Starting TASKTYPE_ACTION_PLAYSOUND.\n" );

	AI_RemoveCurrentTask( self );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_ActionStreamSound
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ActionStreamSound( userEntity_t *self )
{
	_ASSERTE( self );

}

// ----------------------------------------------------------------------------
//
// Name:        AI_StartActionStreamSound
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartActionStreamSound(userEntity_t *self) 
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_ACTION_STREAMSOUND );

    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pAction );

	CAction *pAction = pAIData->pAction;

	userEntity_t *pEntity = self;
	// two parameters
	CParameter *pParameter0 = pAction->GetParameter( 0 );
	_ASSERTE( pParameter0 );
	CParameter *pParameter1 = pAction->GetParameter( 1 );
	if ( pParameter1 )
	{
		_ASSERTE( strlen( pParameter1->GetString() ) > 0 );
		pEntity = UNIQUEID_Lookup( pParameter1->GetString() );
		if ( !AI_IsAlive( pEntity ) )
		{
			gstate->Con_Dprintf( "Uniqueid: %s not found\n", pParameter1->GetString() );
			AI_RemoveCurrentTask( self );
			return;
		}
	}

	gstate->StreamedSound( pParameter0->GetString(), 1.0f );

    AI_Dprintf( "Starting TASKTYPE_ACTION_STREAMSOUND.\n" );

	AI_RemoveCurrentTask( self );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_ActionSendMessage
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ActionSendMessage( userEntity_t *self )
{
	_ASSERTE( self );
	AI_RemoveCurrentTask( self );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_StartActionSendMessage
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartActionSendMessage(userEntity_t *self) 
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_ACTION_SENDMESSAGE );

    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pAction );

	CAction *pAction = pAIData->pAction;

	// two parameters
	CParameter *pParameter0 = pAction->GetParameter( 0 );
	_ASSERTE( pParameter0 );
	CParameter *pParameter1 = pAction->GetParameter( 1 );
	_ASSERTE( pParameter1 );

	_ASSERTE( strlen( pParameter0->GetString() ) > 0 );
	userEntity_t *pMessageEntity = UNIQUEID_Lookup( pParameter0->GetString() );
	if ( AI_IsAlive( pMessageEntity ) )
	{
		GOALSTACK_RemoveCurrentTask( pGoalStack );

		AI_AddNewScriptActionGoal( pMessageEntity, pParameter1->GetString() );

		AI_StartNextTask( self );
	}
	else
	{
		gstate->Con_Dprintf( "Uniqueid %s not found\n", pParameter0->GetString() );
	}

    AI_Dprintf( "Starting TASKTYPE_ACTION_SENDMESSAGE.\n" );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_ActionSendUrgentMessage
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ActionSendUrgentMessage( userEntity_t *self )
{
	_ASSERTE( self );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_StartActionSendMessage
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartActionSendUrgentMessage(userEntity_t *self) 
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_ACTION_SENDURGENTMESSAGE );

    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pAction );

	CAction *pAction = pAIData->pAction;

	// two parameters
	CParameter *pParameter0 = pAction->GetParameter( 0 );
	_ASSERTE( pParameter0 );
	CParameter *pParameter1 = pAction->GetParameter( 1 );
	_ASSERTE( pParameter1 );

	_ASSERTE( strlen( pParameter0->GetString() ) > 0 );
	userEntity_t *pMessageEntity = UNIQUEID_Lookup( pParameter0->GetString() );
	if ( AI_IsAlive( pMessageEntity ) )
	{
		GOALSTACK_RemoveCurrentTask( pGoalStack );

		AI_AddNewScriptActionGoal( pMessageEntity, pParameter1->GetString() );

		AI_StartNextTask( self );
	}
	else
	{
		gstate->Con_Dprintf( "Uniqueid %s not found\n", pParameter0->GetString() );
	}

    AI_Dprintf( "Starting TASKTYPE_ACTION_SENDURGENTMESSAGE.\n" );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_ActionCall
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ActionCall( userEntity_t *self )
{
	_ASSERTE( self );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_StartActionCall
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartActionCall(userEntity_t *self) 
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_ACTION_CALL );

    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pAction );

	CAction *pAction = pAIData->pAction;

	// one parameters
	CParameter *pParameter0 = pAction->GetParameter( 0 );
	_ASSERTE( pParameter0 );

	GOALSTACK_RemoveCurrentTask( pGoalStack );

	AI_AddNewScriptActionGoal( self, pParameter0->GetString(), FALSE );
				
	AI_StartNextTask( self );

    AI_Dprintf( "Starting TASKTYPE_ACTION_CALL.\n" );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_ActionRandomScript
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ActionRandomScript( userEntity_t *self )
{
	_ASSERTE( self );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_StartActionRandomScript
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartActionRandomScript(userEntity_t *self) 
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_ACTION_RANDOMSCRIPT );

    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pAction );

	CAction *pAction = pAIData->pAction;

	int nNumParameters = pAction->GetNumParameters();
	int nIndex = (int)(rand() % nNumParameters);
	
	CParameter *pParameter = pAction->GetParameter( nIndex );
	
	GOALSTACK_RemoveCurrentTask( pGoalStack );

	AI_AddNewScriptActionGoal( self, pParameter->GetString() );

	AI_StartNextTask( self );

    AI_Dprintf( "Starting TASKTYPE_ACTION_RANDOMSCRIPT.\n" );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_ActionComeNear
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ActionComeNear( userEntity_t *self )
{
	_ASSERTE( self );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_StartActionComeNear
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartActionComeNear(userEntity_t *self) 
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_ACTION_COMENEAR );

    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pAction );

	CAction *pAction = pAIData->pAction;

	// two parameters
	CParameter *pParameter0 = pAction->GetParameter( 0 );
	_ASSERTE( pParameter0 );
	CParameter *pParameter1 = pAction->GetParameter( 1 );
	_ASSERTE( pParameter1 );

	_ASSERTE( strlen( pParameter0->GetString() ) > 0 );
	userEntity_t *pFirstEntity = UNIQUEID_Lookup( pParameter0->GetString() );
	int bFirstEntityAlive = AI_IsAlive( pFirstEntity );
	if ( !bFirstEntityAlive )
	{
		gstate->Con_Dprintf( "Uniqueid: %s not found\n", pParameter0->GetString() );
	}

	_ASSERTE( strlen( pParameter1->GetString() ) > 0 );
	userEntity_t *pSecondEntity = UNIQUEID_Lookup( pParameter1->GetString() );
	int bSecondEntityAlive = AI_IsAlive( pSecondEntity );
	if ( !bSecondEntityAlive )
	{
		gstate->Con_Dprintf( "Uniqueid: %s not found\n", pParameter1->GetString() );
	}

	GOALSTACK_RemoveCurrentTask( pGoalStack );

	if ( bFirstEntityAlive && bSecondEntityAlive )
	{
		AI_AddNewTaskAtFront( pSecondEntity, TASKTYPE_MOVETOENTITY, pFirstEntity );
	}

	AI_StartNextTask( self );

    AI_Dprintf( "Starting TASKTYPE_ACTION_COMENEAR.\n" );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_ActionRemove
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ActionRemove( userEntity_t *self )
{
	_ASSERTE( self );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_StartActionRemove
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartActionRemove( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_ACTION_REMOVE );

    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pAction );

	CAction *pAction = pAIData->pAction;

	// one parameters
	CParameter *pParameter0 = pAction->GetParameter( 0 );
	_ASSERTE( pParameter0 );

	_ASSERTE( strlen( pParameter0->GetString() ) > 0 );
	userEntity_t *pEntity = UNIQUEID_Lookup( pParameter0->GetString() );
	if ( !AI_IsAlive( pEntity ) )
	{
		gstate->Con_Dprintf( "Uniqueid: %s not found\n", pParameter0->GetString() );
		if ( self != pEntity )
		{
			AI_RemoveCurrentTask( self );
		}
		return;
	}
	
	playerHook_t *pEntityHook = (playerHook_t*)pEntity->userHook;
	_ASSERTE( pEntityHook );
	
	// FIXME: ISP 
	// Need to way to implement a better way to remove an entity from the game
	self->svflags -= (self->svflags & SVF_MONSTER);
	self->svflags |= SVF_DEADMONSTER;
    
	pEntity->s.renderfx = RF_NODRAW;
	pEntity->deadflag	= DEAD_DEAD;
	
	alist_remove( self );

	pEntity->pain		= NULL;
	pEntity->use		= NULL;
	pEntity->die		= NULL;
	pEntity->think		= NULL;
	pEntity->prethink	= NULL;
	pEntity->postthink = NULL;

	if ( pEntityHook->pGoals )
	{
		pEntityHook->pGoals = GOALSTACK_Delete( pEntityHook->pGoals );
	}
	if ( pEntityHook->pScriptGoals )
	{
		pEntityHook->pScriptGoals = GOALSTACK_Delete( pEntityHook->pScriptGoals );
	}

    AI_Dprintf( "Starting TASKTYPE_ACTION_REMOVE.\n" );

	if ( self != pEntity )
	{
		AI_RemoveCurrentTask( self );
	}

	pEntity->remove( pEntity );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_ActionLookAt
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ActionLookAt( userEntity_t *self )
{
	_ASSERTE( self );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_StartActionLookAt
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartActionLookAt( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_ACTION_LOOKAT );

    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pAction );

	CAction *pAction = pAIData->pAction;

	// two parameters
	CParameter *pParameter0 = pAction->GetParameter( 0 );
	_ASSERTE( pParameter0 );
	CParameter *pParameter1 = pAction->GetParameter( 1 );
	_ASSERTE( pParameter1 );

	_ASSERTE( strlen( pParameter0->GetString() ) > 0 );
	userEntity_t *pFirstEntity = UNIQUEID_Lookup( pParameter0->GetString() );
	int bFirstEntityAlive = AI_IsAlive( pFirstEntity );
	if ( !bFirstEntityAlive )
	{
		gstate->Con_Dprintf( "Uniqueid: %s not found\n", pParameter0->GetString() );
	}

	_ASSERTE( strlen( pParameter1->GetString() ) > 0 );
	userEntity_t *pSecondEntity = UNIQUEID_Lookup( pParameter1->GetString() );
	int bSecondEntityAlive = AI_IsAlive( pSecondEntity );
	if ( !bSecondEntityAlive )
	{
		gstate->Con_Dprintf( "Uniqueid: %s not found\n", pParameter1->GetString() );
	}

	if ( bFirstEntityAlive && bSecondEntityAlive )
	{
		playerHook_t *pFirstHook = (playerHook_t*)pFirstEntity->userHook;
		_ASSERTE( pFirstHook );

		pFirstHook->pEntityToLookAt = pSecondEntity;
	}

	AI_RemoveCurrentTask( self );

    AI_Dprintf( "Starting TASKTYPE_ACTION_LOOKAT.\n" );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_ActionStopLook
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ActionStopLook( userEntity_t *self )
{
	_ASSERTE( self );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_StartActionStopLook
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartActionStopLook( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_ACTION_STOPLOOK );

    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pAction );

	CAction *pAction = pAIData->pAction;

	// one parameter
	CParameter *pParameter0 = pAction->GetParameter( 0 );
	_ASSERTE( pParameter0 );

	_ASSERTE( strlen( pParameter0->GetString() ) > 0 );
	userEntity_t *pFirstEntity = UNIQUEID_Lookup( pParameter0->GetString() );
	if ( AI_IsAlive( pFirstEntity ) )
	{
		playerHook_t *pFirstHook = (playerHook_t*)pFirstEntity->userHook;
		_ASSERTE( pFirstHook );

		pFirstHook->pEntityToLookAt = NULL;
	}
	else
	{
		gstate->Con_Dprintf( "Uniqueid: %s not found\n", pParameter0->GetString() );
	}

	AI_RemoveCurrentTask( self );

    AI_Dprintf( "Starting TASKTYPE_ACTION_STOPLOOK.\n" );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_ActionAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ActionAttack( userEntity_t *self )
{
	_ASSERTE( self );
    playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_ACTION_ATTACK );

    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

    userEntity_t *pEnemy = self->enemy;

    if ( AI_IsAlive( pEnemy ) )
    {
	    CVector destPoint = pEnemy->s.origin;
	    if ( destPoint.Length() == 0.0f )
	    {
		    // hack to place correct origins on bmodels and func_*
            // so that it sill fire at the correct coord
            destPoint.x = (pEnemy->absmax.x + pEnemy->absmin.x) * 0.5f;
		    destPoint.y = (pEnemy->absmax.y + pEnemy->absmin.y) * 0.5f;
		    destPoint.z = (pEnemy->absmax.z + pEnemy->absmin.z) * 0.5f;
	    }

        AI_FaceTowardPoint( self, destPoint );

        if ( !strstr( hook->cur_sequence->animation_name, "atak" ) )
        {
            if ( AI_IsFacingTowardPoint( self, destPoint ) )
            {
                AI_ForceSequence( self, pAIData->pAnimSequence );
            }
        }
        else
        {
	        AI_PlayAttackSounds( self );

	        if ( AI_IsReadyToAttack1( self ) )
	        {
		        if ( pAIData->pString )
                {
                    self->curWeapon = gstate->InventoryFindItem( self->inventory, pAIData->pString );
                }

                if ( pEnemy->s.origin.Length() == 0.0f )
                {
                    pEnemy->s.origin = destPoint;
                    ai_fire_curWeapon( self );
                    pEnemy->s.origin = vec3_origin;
                }
                else
                {
                    ai_fire_curWeapon( self );
                }
	        }

	        if ( AI_IsEndAnimation( self ) )
	        {
		        self->enemy = NULL;
                AI_RemoveCurrentTask( self );
	        }
        }
    }
    else
    {
   		self->enemy = NULL;
        AI_RemoveCurrentTask( self );
    }
}

// ----------------------------------------------------------------------------
//
// Name:        AI_StartActionAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartActionAttack( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_ACTION_ATTACK );

    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pAction );

	CAction *pAction = pAIData->pAction;

	// two parameters
	CParameter *pParameter0 = pAction->GetParameter( 0 );
	_ASSERTE( pParameter0 );
	CParameter *pParameter1 = pAction->GetParameter( 1 );
	_ASSERTE( pParameter1 );

	_ASSERTE( strlen( pParameter0->GetString() ) > 0 );
	userEntity_t *pFirstEntity = UNIQUEID_Lookup( pParameter0->GetString() );
	int bFirstEntityAlive = AI_IsAlive( pFirstEntity );
	if ( !bFirstEntityAlive )
	{
        gstate->Con_Dprintf( "Uniqueid: %s not found\n", pParameter0->GetString() );
        AI_RemoveCurrentTask( self );
        return;
    }

	self->enemy = pFirstEntity;

    _ASSERTE( strlen( pParameter1->GetString() ) > 0 );

    pAIData->pAnimSequence = FRAMES_GetSequence( self, pParameter1->GetString() );

    // weapon name
    CParameter *pParameter2 = pAction->GetParameter( 2 );
    if ( pParameter2 )
    {
        pAIData->pString = pParameter2->GetString();
    }
    else
    {
        pAIData->pString = NULL;
    }

    AI_Dprintf( "Starting TASKTYPE_ACTION_ATTACK.\n" );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_ActionPrint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ActionPrint( userEntity_t *self )
{
	_ASSERTE( self );
}

// ----------------------------------------------------------------------------
//
// Name:        AI_StartActionPrint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartActionPrint( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_ACTION_PRINT );

    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->pAction );

	CAction *pAction = pAIData->pAction;

	// one parameter
	CParameter *pParameter0 = pAction->GetParameter( 0 );
	_ASSERTE( pParameter0 );

	_ASSERTE( strlen( pParameter0->GetText() ) > 0 );

	gstate->Con_Dprintf( "%s\n", pParameter0->GetText() );

	AI_RemoveCurrentTask( self );

    AI_Dprintf( "Starting TASKTYPE_ACTION_PRINT.\n" );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartWaitUntilNoObstruction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartWaitUntilNoObstruction( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	_ASSERTE( TASK_GetType( pCurrentTask ) == TASKTYPE_WAITUNTILNOOBSTRUCTION );

	char szAnimation[16];
	AI_SelectAmbientAnimation( self, szAnimation );
	AI_ForceSequence( self, szAnimation, FRAME_LOOP );

	AI_Dprintf( "Starting TASKTYPE_WAITUNTILNOOBSTRUCTION.\n" );

	AI_ZeroVelocity( self );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, 1.0f );
	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_WaitUntilNoObstruction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_WaitUntilNoObstruction( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData->destPoint );

    if ( !AI_IsPointObstructedByEntity( self, pAIData->destPoint ) )
    {
        AI_RemoveCurrentTask( self );
        return;
    }

    AI_SetTaskFinishTime( hook, 1.0f );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartMoveToRetreatNode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartMoveToRetreatNode( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_MOVETORETREATNODE );

	AI_SetStateRunning( hook );
	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_MOVETORETREATNODE )
		{
			return;
		}
	}

	//	delete any previous path
	PATHLIST_KillPath( hook->pPathList );

	//	set up a path consisting of one node
	MAPNODE_PTR pRetreatNode = NODE_GetClosestRetreatNode( self );
	if ( !pRetreatNode )
	{
		//	nowhere to run to retreat
		AI_RemoveCurrentTask( self );
		return;
	}

	AI_FindPathToPoint( self, pRetreatNode->position );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	TASK_Set( pCurrentTask, pRetreatNode->position );

	AI_Dprintf( "Starting TASKTYPE_MOVETORETREATNODE.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );

	float fDistance = VectorDistance( self->s.origin, pRetreatNode->position );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 2.0f;
	AI_SetTaskFinishTime( hook, fTime );

	AI_SetMovingCounter( hook, 0 );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_MoveToRetreatNode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_MoveToRetreatNode( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
    _ASSERTE( pAIData->destPoint.Length() > 0.0f );    	

	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	if ( fXYDistance < AI_GetWalkFollowDistance(hook) && fZDistance < MAX_JUMP_HEIGHT )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	if ( AI_IsOkToMoveStraight( self, pAIData->destPoint, fXYDistance, fZDistance ) )
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
// Name:		AI_StartStopEntity
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartStopEntityTask( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_STOPENTITY );

	if ( !strstr( hook->cur_sequence->animation_name, "amb" ) && AI_IsEndAnimation(self) )
    {
	    char szAnimation[16];
	    AI_SelectAmbientAnimation( self, szAnimation );
	    AI_ForceSequence( self, szAnimation, FRAME_LOOP );
    }
    self->s.frameInfo.frameState = 0;
	AI_ZeroVelocity( self );
    PATHLIST_KillPath(hook->pPathList);
    AI_UpdateCurrentNode( self );
    AI_Dprintf( "Starting TASKTYPE_STOPENTITY.\n" );
}

void AI_StopEntityTask( userEntity_t *self )
{
	AI_ZeroVelocity( self );
	AI_RemoveCurrentTask( self );
}


// ----------------------------------------------------------------------------
//
// Name:		AI_HandleHiding
// Description:
//				test if there are any hide nodes, if not just runaway
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_HandleHiding( userEntity_t *self, userEntity_t *pAttacker )
{
	_ASSERTE( self );
	_ASSERTE( pAttacker );

	NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( self );
	MAPNODE_PTR pHideNode = NODE_GetClosestPartialHideNode( pNodeHeader, self, pAttacker );
	if ( pHideNode )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_HIDE );
	}
	else
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_RUNAWAY, pAttacker );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_HandleSelfPreservation
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_HandleSelfPreservation( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( (self->flags & FL_BOT) )
	{
		if ( hook->owner && self->enemy )
		{
			float fDistance = VectorDistance( self->s.origin, hook->owner->s.origin );
			if ( fDistance > 512.0f && !AI_IsVisible( self, hook->owner ) )
			{
				GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
				GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
				GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
				if ( nGoalType == GOALTYPE_KILLENEMY )
				{
					AI_RemoveCurrentGoal( self );
					AI_AddNewTaskAtFront( self, TASKTYPE_RETREATTOOWNER );
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

//*****************************************************************************
//
//*****************************************************************************






// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewTaskAtFront
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
DllExport TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	if(!pCurrentGoal)
		return NULL;


	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType );
	GOAL_AddTaskAtFront( pCurrentGoal, pNewTask );
	AI_StartTask( self, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewTaskAtFront
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, AIDATA_PTR aiData )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
    
    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
	if (!pCurrentGoal)
		return NULL;
    _ASSERTE(pCurrentGoal);

    TASK_PTR pNewTask = TASK_Allocate(nNewTaskType, aiData);
    GOAL_AddTaskAtFront(pCurrentGoal, pNewTask);
    AI_StartTask(self, pNewTask);

    return pNewTask;
}

TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, userEntity_t *pEntity )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	_ASSERTE( pEntity );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
//	_ASSERTE( pCurrentGoal );
	// SCG[12/4/99]: 
	if( pCurrentGoal == NULL )
	{
		return NULL;
	}

	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, pEntity );
	GOAL_AddTaskAtFront( pCurrentGoal, pNewTask );
	AI_StartTask( self, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewTaskAtFront
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, const CVector &position )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
//	_ASSERTE( pCurrentGoal );
	// SCG[12/4/99]: 
	if( pCurrentGoal == NULL )
	{
		return NULL;
	}

	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, position );
	GOAL_AddTaskAtFront( pCurrentGoal, pNewTask );
	AI_StartTask( self, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewTaskAtFront
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, const float fValue )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
//	_ASSERTE( pCurrentGoal );
	// SCG[12/4/99]: 
	if( pCurrentGoal == NULL )
	{
		return NULL;
	}


	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, fValue );
	GOAL_AddTaskAtFront( pCurrentGoal, pNewTask );
	AI_StartTask( self, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewTaskAtFront
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, const int nValue )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
//	_ASSERTE( pCurrentGoal );
	// SCG[12/4/99]: 
	if( pCurrentGoal == NULL )
	{
		return NULL;
	}


	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, nValue );
	GOAL_AddTaskAtFront( pCurrentGoal, pNewTask );
	AI_StartTask( self, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewTaskAtFront
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, frameData_t *pNewSequence )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	_ASSERTE( pNewSequence );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
//	_ASSERTE( pCurrentGoal );
	// SCG[12/4/99]: 
	if( pCurrentGoal == NULL )
	{
		return NULL;
	}


	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, pNewSequence );
	GOAL_AddTaskAtFront( pCurrentGoal, pNewTask );
	AI_StartTask( self, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewTaskAtFront
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, CAction *pAction )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	_ASSERTE( pAction );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
//	_ASSERTE( pCurrentGoal );
	// SCG[12/4/99]: 
	if( pCurrentGoal == NULL )
	{
		return NULL;
	}


	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, pAction );
	GOAL_AddTaskAtFront( pCurrentGoal, pNewTask );
	AI_StartTask( self, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewTaskAtFront
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, void *pVoid )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	_ASSERTE( pVoid );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
//	_ASSERTE( pCurrentGoal );
	// SCG[12/4/99]: 
	if( pCurrentGoal == NULL )
	{
		return NULL;
	}


	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, pVoid );
	GOAL_AddTaskAtFront( pCurrentGoal, pNewTask );
	AI_StartTask( self, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewTaskAtFront
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, char *szString )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	_ASSERTE( szString );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
//	_ASSERTE( pCurrentGoal );
	// SCG[12/4/99]: 
	if( pCurrentGoal == NULL )
	{
		return NULL;
	}


	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, szString );
	GOAL_AddTaskAtFront( pCurrentGoal, pNewTask );
	AI_StartTask( self, pNewTask );

	return pNewTask;
}

TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, char *szString, const float fNewValue )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	_ASSERTE( szString );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
//	_ASSERTE( pCurrentGoal );
	// SCG[12/4/99]: 
	if( pCurrentGoal == NULL )
	{
		return NULL;
	}


	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, szString, fNewValue );
	GOAL_AddTaskAtFront( pCurrentGoal, pNewTask );
	AI_StartTask( self, pNewTask );

	return pNewTask;
}

TASK_PTR AI_AddNewTaskAtFront( userEntity_t *self, TASKTYPE nNewTaskType, char *szString, const float fValue1, const float fValue2 )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	_ASSERTE( szString );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
//	_ASSERTE( pCurrentGoal );
	// SCG[12/4/99]: 
	if( pCurrentGoal == NULL )
	{
		return NULL;
	}


	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, szString, fValue1, fValue2 );
	GOAL_AddTaskAtFront( pCurrentGoal, pNewTask );
	AI_StartTask( self, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	_ASSERTE( pCurrentGoal );

	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType );
	GOAL_AddTask( pCurrentGoal, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewTaskAtFront
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, AIDATA_PTR aiData )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
    
    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE(pCurrentGoal);

    TASK_PTR pNewTask = TASK_Allocate(nNewTaskType, aiData);
    GOAL_AddTask(pCurrentGoal, pNewTask);

    return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, userEntity_t *pEntity )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	_ASSERTE( pEntity );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	_ASSERTE( pCurrentGoal );

	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, pEntity );
	GOAL_AddTask( pCurrentGoal, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		Ai_AddNewTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, const CVector &position )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	_ASSERTE( pCurrentGoal );

	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, position );
	GOAL_AddTask( pCurrentGoal, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		Ai_AddNewTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, const float fValue )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	_ASSERTE( pCurrentGoal );

	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, fValue );
	GOAL_AddTask( pCurrentGoal, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		Ai_AddNewTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, const int nValue )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	_ASSERTE( pCurrentGoal );

	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, nValue );
	GOAL_AddTask( pCurrentGoal, pNewTask );

	return pNewTask;
}

TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, frameData_t *pNewSequence )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	_ASSERTE( pCurrentGoal );

	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, pNewSequence );
	GOAL_AddTask( pCurrentGoal, pNewTask );

	return pNewTask;
}

TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, char *szString )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	_ASSERTE( pCurrentGoal );

	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, szString );
	GOAL_AddTask( pCurrentGoal, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, CAction *pAction )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	_ASSERTE( pAction );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	_ASSERTE( pCurrentGoal );

	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, pAction );
	GOAL_AddTask( pCurrentGoal, pNewTask );

	return pNewTask;
}


TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, char *szString, const float fNewValue )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	_ASSERTE( pCurrentGoal );

	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, szString, fNewValue );
	GOAL_AddTask( pCurrentGoal, pNewTask );

	return pNewTask;
}

TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, char *szString, const float fValue1, const float fValue2 )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	_ASSERTE( pCurrentGoal );

	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, szString, fValue1, fValue2 );
	GOAL_AddTask( pCurrentGoal, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTask( userEntity_t *self, TASKTYPE nNewTaskType, void *pVoid )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	_ASSERTE( pVoid );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	_ASSERTE( pCurrentGoal );

	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, pVoid );
	GOAL_AddTask( pCurrentGoal, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTask( GOAL_PTR pGoal, TASKTYPE nNewTaskType )
{
	_ASSERTE( pGoal );
	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType );
	GOAL_AddTask( pGoal, pNewTask );

	return pNewTask;
}

TASK_PTR AI_AddNewTask( GOAL_PTR pGoal, TASKTYPE nNewTaskType, AIDATA_PTR aiData) 
{
    _ASSERTE( pGoal );

	TASK_PTR pNewTask = TASK_Allocate(nNewTaskType, aiData);
    if (pNewTask == NULL) return NULL;

    GOAL_AddTask(pGoal, pNewTask);

    return pNewTask;
}


// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTask( GOAL_PTR pGoal, TASKTYPE nNewTaskType, userEntity_t *pEntity )
{
	_ASSERTE( pGoal );
	_ASSERTE( pEntity );
	
	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, pEntity );
	GOAL_AddTask( pGoal, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTask( GOAL_PTR pGoal, TASKTYPE nNewTaskType, CAction *pAction )
{
	_ASSERTE( pGoal );
	_ASSERTE( pAction );
	
	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, pAction );
	GOAL_AddTask( pGoal, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTask( GOAL_PTR pGoal, TASKTYPE nNewTaskType, const CVector &position )
{
	_ASSERTE( pGoal );
	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, position );
	GOAL_AddTask( pGoal, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTask( GOAL_PTR pGoal, TASKTYPE nNewTaskType, const float fValue )
{
	_ASSERTE( pGoal );
	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, fValue );
	GOAL_AddTask( pGoal, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR AI_AddNewTask( GOAL_PTR pGoal, TASKTYPE nNewTaskType, const int nValue )
{
	_ASSERTE( pGoal );
	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, nValue );
	GOAL_AddTask( pGoal, pNewTask );

	return pNewTask;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_RemoveCurrentTaskWithoutStartingNextTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_RemoveCurrentTaskWithoutStartingNextTask( userEntity_t *self, int bFinishCurrentSequence )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOALSTACK_RemoveCurrentTask( pGoalStack );

	// see if current animation has completed
/*	if ( bFinishCurrentSequence && hook->cur_sequence && 
		 hook->type != TYPE_CLIENT && hook->type != TYPE_BOT && 
		 !AI_IsSidekick( hook ) &&
		 self->s.frameInfo.frameState != 0 && !AI_IsEndAnimation( self )
	   )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_FINISHCURRENTSEQUENCE );
		return;
	}*/
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddStartNextTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartNextTask( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( pGoalStack );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	if ( pCurrentTask )
	{
		AI_StartTask( self, pCurrentTask );
	}
	else
	{
		// there are no more tasks for this goal, determine if the goal is 
		// satisfied, if not then start the process over
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		if ( pCurrentGoal )
		{
			// if this is part of a script actions, check if the goal is satified
			GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
			if ( nGoalType == GOALTYPE_SCRIPTACTION || nGoalType == GOALTYPE_WHENUSED_SCRIPTACTION )
			{
				AIDATA_PTR pAIData = GOAL_GetData( pCurrentGoal );
				if ( pAIData->nValue >= 0 )
				{
					pAIData->nValue--;
					if ( pAIData->nValue <= 0 )
					{
						GOAL_Satisfied( pCurrentGoal );
					}
				}

				if ( GOAL_IsSatisfied( pCurrentGoal ) == FALSE )
				{
					// have not finished looping,
					// start the process all over again
					AI_AddScriptActionGoal( self, pAIData->pString, TRUE, TRUE );
				}
				else
				{
					AI_RemoveCurrentGoal( self );
				}
			}
			else
            if ( nGoalType == GOALTYPE_GENERALUSE )
            {
                AI_RemoveCurrentGoal( self );
            }
            else
			{
                if ( GOAL_IsSatisfied( pCurrentGoal ) == FALSE )
				{
					// start the process all over again
					AI_AddInitialTasksToGoal( self, pCurrentGoal );
				}
				else
				{
					AI_RemoveCurrentGoal( self );
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_RemoveCurrentTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_RemoveCurrentTask( userEntity_t *self, int bFinishCurrentSequence )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    AI_SetOkToAttackFlag( hook, TRUE );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOALSTACK_RemoveCurrentTask( pGoalStack );

	// see if current animation has completed
/*	if ( bFinishCurrentSequence && hook->cur_sequence && 
		 hook->type != TYPE_CLIENT && hook->type != TYPE_BOT && 
		 !AI_IsSidekick( hook ) &&
		 self->s.frameInfo.frameState != 0 && !AI_IsEndAnimation( self )
	   )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_FINISHCURRENTSEQUENCE );
		return;
	}*/

	AI_StartNextTask( self );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_RemoveCurrentTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_RemoveCurrentTask( userEntity_t *self, TASKTYPE nNewTaskType, int bFinishCurrentSequence )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_SetOkToAttackFlag( hook, TRUE );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOALSTACK_RemoveCurrentTask( pGoalStack );

	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	_ASSERTE( pCurrentGoal );

	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType );
	GOAL_AddTaskAtFront( pCurrentGoal, pNewTask );

	// see if current animation has completed
/*	if ( bFinishCurrentSequence && hook->cur_sequence && 
		 hook->type != TYPE_CLIENT && hook->type != TYPE_BOT && 
		 !AI_IsSidekick( hook ) &&
		 self->s.frameInfo.frameState != 0 && !AI_IsEndAnimation( self )
	   )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_FINISHCURRENTSEQUENCE );
		return;
	}*/

	AI_StartTask( self, pNewTask );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_RemoveCurrentTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_RemoveCurrentTask( userEntity_t *self, TASKTYPE nNewTaskType, 
						   userEntity_t *pEntity, int bFinishCurrentSequence )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_SetOkToAttackFlag( hook, TRUE );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOALSTACK_RemoveCurrentTask( pGoalStack );

	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	_ASSERTE( pCurrentGoal );

	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, pEntity );
	//NSS[11/23/99]:Not the front... the current position.
	GOAL_AddTaskAtFront( pCurrentGoal, pNewTask );
	//GOAL_AddTask(pCurrentGoal,pNewTask);
	
	//NSS[11/23/99]:Do NOT add another task ontop of another task like this... this would be bad.
	// see if current animation has completed
	/*if ( bFinishCurrentSequence && hook->cur_sequence && 
		 hook->type != TYPE_CLIENT && hook->type != TYPE_BOT && 
		 !AI_IsSidekick( hook ) &&
		 self->s.frameInfo.frameState != 0 && !AI_IsEndAnimation( self )
	   )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_FINISHCURRENTSEQUENCE );
		return;
	}*/

	AI_StartTask( self, pNewTask );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_RemoveCurrentTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_RemoveCurrentTask( userEntity_t *self, TASKTYPE nNewTaskType, 
						   const CVector &position, int bFinishCurrentSequence )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_SetOkToAttackFlag( hook, TRUE );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOALSTACK_RemoveCurrentTask( pGoalStack );

	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	_ASSERTE( pCurrentGoal );

	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, position );
	GOAL_AddTaskAtFront( pCurrentGoal, pNewTask );

	// see if current animation has completed
/*	if ( bFinishCurrentSequence && hook->cur_sequence && 
		 hook->type != TYPE_CLIENT && hook->type != TYPE_BOT && 
		 !AI_IsSidekick( hook ) &&
		 self->s.frameInfo.frameState != 0 && !AI_IsEndAnimation( self )
	   )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_FINISHCURRENTSEQUENCE );
		return;
	}*/

	AI_StartTask( self, pNewTask );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_RemoveCurrentTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_RemoveCurrentTask( userEntity_t *self, TASKTYPE nNewTaskType, 
						   const float fValue, int bFinishCurrentSequence )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_SetOkToAttackFlag( hook, TRUE );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOALSTACK_RemoveCurrentTask( pGoalStack );

	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	_ASSERTE( pCurrentGoal );

	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, fValue );
	GOAL_AddTaskAtFront( pCurrentGoal, pNewTask );

	// see if current animation has completed
/*	if ( bFinishCurrentSequence && hook->cur_sequence && 
		 hook->type != TYPE_CLIENT && hook->type != TYPE_BOT && 
		 !AI_IsSidekick( hook ) &&
		 self->s.frameInfo.frameState != 0 && !AI_IsEndAnimation( self )
	   )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_FINISHCURRENTSEQUENCE );
		return;
	}*/

	AI_StartTask( self, pNewTask );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_RemoveCurrentTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_RemoveCurrentTask( userEntity_t *self, TASKTYPE nNewTaskType, 
						   const int nValue, int bFinishCurrentSequence )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_SetOkToAttackFlag( hook, TRUE );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOALSTACK_RemoveCurrentTask( pGoalStack );

	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	_ASSERTE( pCurrentGoal );

	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, nValue );
	GOAL_AddTaskAtFront( pCurrentGoal, pNewTask );

	// see if current animation has completed
/*	if ( bFinishCurrentSequence && hook->cur_sequence && 
		 hook->type != TYPE_CLIENT && hook->type != TYPE_BOT && 
		 !AI_IsSidekick( hook ) &&
		 self->s.frameInfo.frameState != 0 && !AI_IsEndAnimation( self )
	   )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_FINISHCURRENTSEQUENCE );
		return;
	}*/

	AI_StartTask( self, pNewTask );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_RemoveCurrentTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_RemoveCurrentTask( userEntity_t *self, TASKTYPE nNewTaskType, 
						   frameData_t *pFrameData, int bFinishCurrentSequence )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_SetOkToAttackFlag( hook, TRUE );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOALSTACK_RemoveCurrentTask( pGoalStack );

	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	_ASSERTE( pCurrentGoal );

	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, pFrameData );
	GOAL_AddTaskAtFront( pCurrentGoal, pNewTask );

	// see if current animation has completed
/*	if ( bFinishCurrentSequence && hook->cur_sequence && 
		 hook->type != TYPE_CLIENT && hook->type != TYPE_BOT && 
		 !AI_IsSidekick( hook ) &&
		 self->s.frameInfo.frameState != 0 && !AI_IsEndAnimation( self )
	   )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_FINISHCURRENTSEQUENCE );
		return;
	}*/

	AI_StartTask( self, pNewTask );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_RemoveCurrentTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_RemoveCurrentTask( userEntity_t *self, TASKTYPE nNewTaskType, 
						   char *szString, int bFinishCurrentSequence )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_SetOkToAttackFlag( hook, TRUE );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOALSTACK_RemoveCurrentTask( pGoalStack );

	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	_ASSERTE( pCurrentGoal );

	TASK_PTR pNewTask = TASK_Allocate( nNewTaskType, szString );
	GOAL_AddTaskAtFront( pCurrentGoal, pNewTask );

	// see if current animation has completed
/*	if ( bFinishCurrentSequence && hook->cur_sequence && 
		 hook->type != TYPE_CLIENT && hook->type != TYPE_BOT && 
		 !AI_IsSidekick( hook ) &&
		 self->s.frameInfo.frameState != 0 && !AI_IsEndAnimation( self )
	   )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_FINISHCURRENTSEQUENCE );
		return;
	}*/

	AI_StartTask( self, pNewTask );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewGoal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR AI_AddNewGoal( userEntity_t *self, GOALTYPE nNewGoalType )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	// clear the tasks for the current goal before adding a new goal
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	if ( pCurrentGoal )
	{
		GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
		if ( nGoalType != GOALTYPE_SCRIPTACTION && nGoalType != GOALTYPE_WHENUSED_SCRIPTACTION )
		{
			GOAL_ClearTasks( pCurrentGoal );
		}
	}

	GOAL_PTR pNewGoal = GOAL_Allocate( nNewGoalType );
	if ( !pNewGoal )
	{
		return NULL;
	}

	GOALSTACK_Add( pGoalStack, pNewGoal );

	AI_AddInitialTasksToGoal( self, pNewGoal );

	return pNewGoal;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewGoal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR AI_AddNewGoal( userEntity_t *self, GOALTYPE nNewGoalType, AIDATA_PTR pAIData )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	// clear the tasks for the current goal before adding a new goal
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	if ( pCurrentGoal )
	{
		GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
		if ( nGoalType != GOALTYPE_SCRIPTACTION && nGoalType != GOALTYPE_WHENUSED_SCRIPTACTION )
		{
			GOAL_ClearTasks( pCurrentGoal );
		}
	}

	GOAL_PTR pNewGoal = GOAL_Allocate( nNewGoalType, pAIData );
	if ( !pNewGoal )
	{
		return NULL;
	}

	GOALSTACK_Add( pGoalStack, pNewGoal );

	AI_AddInitialTasksToGoal( self, pNewGoal );

	return pNewGoal;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewGoal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR AI_AddNewGoal( userEntity_t *self, GOALTYPE nNewGoalType, userEntity_t *pEntity )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	_ASSERTE( pEntity );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );

    if ( nNewGoalType == GOALTYPE_KILLENEMY && 
         AI_IsSidekick( hook ) && !SIDEKICK_IsOkToFight( self ) )
    {
        if ( GOAL_GetType( pCurrentGoal ) == GOALTYPE_KILLENEMY )
        {
            AI_RemoveCurrentGoal( self );
        }

        // try to evade from fighting
		TASK_PTR pCurrentTask = GOAL_GetCurrentTask( pCurrentGoal );
		TASKTYPE nTaskType = TASK_GetType( pCurrentTask );
        if ( nTaskType != TASKTYPE_SIDEKICK_EVADE && AI_IsLineOfSight( self, pEntity ) )
        {
            AI_SetEnemy( self, pEntity );
            AI_AddNewTaskAtFront( self, TASKTYPE_SIDEKICK_EVADE, pEntity );
        }
        return NULL;
    }

	// clear the tasks for the current goal before adding a new goal
	if ( pCurrentGoal )
	{
		GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );

        // check for duplicate kill enemy goals
        if ( nNewGoalType == GOALTYPE_KILLENEMY )
        {
		    AIDATA_PTR pAIData = GOAL_GetData( pCurrentGoal );
		    if ( pAIData->pEntity == pEntity )
		    {
                return NULL;
            }
        }

		if ( nGoalType != GOALTYPE_SCRIPTACTION && nGoalType != GOALTYPE_WHENUSED_SCRIPTACTION )
		{
			GOAL_ClearTasks( pCurrentGoal );
		}
	}

	GOAL_PTR pNewGoal = GOAL_Allocate( nNewGoalType, pEntity );
	if ( !pNewGoal )
	{
		return NULL;
	}

	GOALSTACK_Add( pGoalStack, pNewGoal );

	AI_AddInitialTasksToGoal( self, pNewGoal );

	return pNewGoal;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewGoal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR AI_AddNewGoal( userEntity_t *self, GOALTYPE nNewGoalType, const CVector &position )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	// clear the tasks for the current goal before adding a new goal
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	if ( pCurrentGoal )
	{
		GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
		if ( nGoalType != GOALTYPE_SCRIPTACTION && nGoalType != GOALTYPE_WHENUSED_SCRIPTACTION )
		{
			GOAL_ClearTasks( pCurrentGoal );
		}
	}

	GOAL_PTR pNewGoal = GOAL_Allocate( nNewGoalType, position );
	if ( !pNewGoal )
	{
		return NULL;
	}

	GOALSTACK_Add( pGoalStack, pNewGoal );

	AI_AddInitialTasksToGoal( self, pNewGoal );

	return pNewGoal;
}

// ---------------------------------------------------------------------------- 
//
// Name:		AI_AddNewGoal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR AI_AddNewGoal( userEntity_t *self, GOALTYPE nNewGoalType, const float fValue )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	// clear the tasks for the current goal before adding a new goal
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	if ( pCurrentGoal )
	{
		GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
		if ( nGoalType != GOALTYPE_SCRIPTACTION && nGoalType != GOALTYPE_WHENUSED_SCRIPTACTION )
		{
			GOAL_ClearTasks( pCurrentGoal );
		}
	}

	GOAL_PTR pNewGoal = GOAL_Allocate( nNewGoalType, fValue );
	if ( !pNewGoal )
	{
		return NULL;
	}

	GOALSTACK_Add( pGoalStack, pNewGoal );

	AI_AddInitialTasksToGoal( self, pNewGoal );

	return pNewGoal;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewGoal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR AI_AddNewGoal( userEntity_t *self, GOALTYPE nNewGoalType, const int nValue )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	// clear the tasks for the current goal before adding a new goal
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	if ( pCurrentGoal )
	{
		GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
		if ( nGoalType != GOALTYPE_SCRIPTACTION && nGoalType != GOALTYPE_WHENUSED_SCRIPTACTION )
		{
			GOAL_ClearTasks( pCurrentGoal );
		}
	}

	GOAL_PTR pNewGoal = GOAL_Allocate( nNewGoalType, nValue );
	if ( !pNewGoal )
	{
		return NULL;
	}

	GOALSTACK_Add( pGoalStack, pNewGoal );

	AI_AddInitialTasksToGoal( self, pNewGoal );

	return pNewGoal;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewGoalAtBack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR AI_AddNewGoalAtBack( userEntity_t *self, GOALTYPE nNewGoalType )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
		
	GOAL_PTR pNewGoal = GOAL_Allocate( nNewGoalType );
	if ( !pNewGoal )
	{
		return NULL;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOALSTACK_AddAtBack( pGoalStack, pNewGoal );

	AI_AddInitialTasksToGoal( self, pNewGoal, FALSE );

	return pNewGoal;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewGoalAtBack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR AI_AddNewGoalAtBack( userEntity_t *self, GOALTYPE nNewGoalType, userEntity_t *pEntity )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOAL_PTR pNewGoal = GOAL_Allocate( nNewGoalType, pEntity );
	if ( !pNewGoal )
	{
		return NULL;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOALSTACK_AddAtBack( pGoalStack, pNewGoal );

	AI_AddInitialTasksToGoal( self, pNewGoal, FALSE );

	return pNewGoal;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewGoalAtBack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR AI_AddNewGoalAtBack( userEntity_t *self, GOALTYPE nNewGoalType, const CVector &position )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOAL_PTR pNewGoal = GOAL_Allocate( nNewGoalType, position );
	if ( !pNewGoal )
	{
		return NULL;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOALSTACK_AddAtBack( pGoalStack, pNewGoal );
	
	AI_AddInitialTasksToGoal( self, pNewGoal, FALSE );

	return pNewGoal;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewGoalAtBack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR AI_AddNewGoalAtBack( userEntity_t *self, GOALTYPE nNewGoalType, const float fValue )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOAL_PTR pNewGoal = GOAL_Allocate( nNewGoalType, fValue );
	if ( !pNewGoal )
	{
		return NULL;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOALSTACK_AddAtBack( pGoalStack, pNewGoal );

	AI_AddInitialTasksToGoal( self, pNewGoal, FALSE );

	return pNewGoal;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewGoalAtBack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR AI_AddNewGoalAtBack( userEntity_t *self, GOALTYPE nNewGoalType, const int nValue )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOAL_PTR pNewGoal = GOAL_Allocate( nNewGoalType, nValue );
	if ( !pNewGoal )
	{
		return NULL;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOALSTACK_AddAtBack( pGoalStack, pNewGoal );

	AI_AddInitialTasksToGoal( self, pNewGoal, FALSE );

	return pNewGoal;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_RemoveCurrentGoal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_RemoveCurrentGoal( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    AI_SetOkToAttackFlag( hook, TRUE );

	// reset the task finish time
	AI_SetTaskFinishTime( hook, -1.0f );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOALSTACK_RemoveCurrentGoal( pGoalStack );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	if ( pCurrentTask )
	{
		AI_StartTask( self, pCurrentTask );
	}
	else
	{
		// there are no more tasks for this goal, determine if the goal is 
		// satisfied, if not then start the process over
 		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		if ( pCurrentGoal )
		{
			// if this is part of a script actions, check if the goal is satified
			GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
			if ( nGoalType == GOALTYPE_SCRIPTACTION || nGoalType == GOALTYPE_WHENUSED_SCRIPTACTION )
			{
				AIDATA_PTR pAIData = GOAL_GetData( pCurrentGoal );
				if ( pAIData->nValue >= 0 )
				{
					pAIData->nValue--;
					if ( pAIData->nValue <= 0 )
					{
						GOAL_Satisfied( pCurrentGoal );
					}
				}

				if ( GOAL_IsSatisfied( pCurrentGoal ) == FALSE )
				{
					// have not finished looping,
					// start the process all over again
					AI_AddScriptActionGoal( self, pAIData->pString, TRUE, TRUE );
				}
				else
				{
					AI_RemoveCurrentGoal( self );
				}
			}
			else
			{
				if ( GOAL_IsSatisfied( pCurrentGoal ) == FALSE )
				{
					// start the process all over again
					AI_AddInitialTasksToGoal( self, pCurrentGoal );

					pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
					if ( pCurrentGoal )
					{
						TASK_PTR pTask = GOAL_GetCurrentTask( pCurrentGoal );
						if ( !pTask )
						{
							AI_RemoveCurrentGoal( self );
						}
					}
				}
				else
				{
					AI_RemoveCurrentGoal( self );
				}
			}
		}
        else
        {
			//NSS[11/18/99]:
			//If in script mode do not go into IDLE animations
			//Otherwise go into IDLE if we have no Goals
			if(hook)
			{
				if(hook->bInScriptMode)
					return;
			}
				
			AI_AddNewGoal(self,GOALTYPE_IDLE);
			pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
			//if ( pCurrentTask )
			//{
			//	AI_StartTask( self, pCurrentTask );
			//}
        }
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AdjustAngles
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_AdjustAngles( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	CVector	ang_speed_save = self->ang_speed;

	//	if standing, this is an ambient turn, so do it slowly
	//if ( hook->th_state == TH_STAND )
	{
		self->ang_speed.Multiply( gstate->frametime * (rnd() * 0.8 + 0.1) );
	}
/*
	else
	{
		self->ang_speed.Multiply( gstate->frametime );
	}
*/

	com->ChangeYaw(self);

	if ( hook->dflags & DFL_FACEPITCH )
	{
		com->ChangePitch(self);
	}

	com->ChangeRoll(self);

	self->ang_speed = ang_speed_save;
}

//---------------------------------------------------------------------------
// clientSmoke()
//---------------------------------------------------------------------------
void clientSmoke(CVector &pos,float scale,float amount,float spread,float maxvel)
{
	gstate->WriteByte(SVC_TEMP_ENTITY);
	gstate->WriteByte(TE_SMOKE);
	gstate->WritePosition(pos);
	gstate->WriteFloat(scale);
	gstate->WriteFloat(amount);
	gstate->WriteFloat(spread);
	gstate->WriteFloat(maxvel);
	gstate->MultiCast(pos,MULTICAST_PVS);
}

// ----------------------------------------------------------------------------
//
// Name:		AI_UpdateHeadAngles
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_UpdateHeadAngles( userEntity_t *self )
{
// SCG[12/7/99]: 	if ( !self || !(self->s.renderfx & RF_CLUSTERBASED))
	if ( !self )
	{
		return FALSE;
	}
	userEntity_t *pClient = NULL;
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *head = alist_FirstEntity(client_list);
	while ( head )
	{
		if ( AI_IsAlive( head ) )
        {
            float fDistance = VectorDistance( self->s.origin, head->s.origin );
		    if ( fDistance < hook->active_distance )
		    {
			    if ( AI_IsVisible( self, head ) )
			    {
				    //amw : data is hacked.. numClusters holds the entity number
				    // for the monster to look at
				    self->s.numClusters = head->s.number;
				    return self->s.numClusters;
			    }
		    }
		    else
		    {
			    float fCameraDistance = 0;

			    playerHook_t *pPlayerHook = AI_GetPlayerHook( head );
			    // NSS[12/7/99]:Added check for the playerhook
				if(pPlayerHook)
				{
					userEntity_t *pCamera = pPlayerHook->camera;
					if ( pCamera )
					{
						fCameraDistance = VectorDistance(self->s.origin, pCamera->s.origin);
						if ( fCameraDistance < hook->active_distance )
						{
							if (AI_IsVisible( self, head ))
							{
								//amw : data is hacked.. numClusters holds the entity number
								// for the monster to look at
								self->s.numClusters = head->s.number;
								return self->s.numClusters;
							}
						}
					}
				}
		    }
        }

		head = alist_NextEntity (client_list);
	}

	// don't look at anyone
	self->s.numClusters = 0;
	return self->s.numClusters;
}


// ----------------------------------------------------------------------------
//
// Name:		AI_ProcessInitialAIState
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ProcessInitialAIState( userEntity_t *self, char *value )
{
	_ASSERTE( self );
	if ( _stricmp( value, "pathfollow" ) == 0 )
	{
		if ( _stricmp( self->className, "monster_cambot" ) == 0 )
		{
			AI_AddNewGoal( self, GOALTYPE_CAMBOT_PATHFOLLOW );
		}
		else
		{
			AI_AddNewGoal( self, GOALTYPE_PATHFOLLOW );
		}
	}
	else
	if ( _stricmp( value, "buboidcoffin" ) == 0 )
	{
		AI_AddNewGoal( self, GOALTYPE_BUBOID_GETOUTOFCOFFIN );
	}
//    /*
//    if ( _stricmp( value, "work_a") == 0) 
//    {
//		if ( _stricmp( self->className, "monster_skinnyworker" ) == 0 )
//		{
//          AI_AddNewGoal( self, GOALTYPE_SKINNYWORKER_WORK_A);
//        }
//    }
//    */
}

// ----------------------------------------------------------------------------
//
// Name:		AI_SetSpawnValue
// Description:
// Input:
// Output:
// Note:
//				also handles the default script
//
// ----------------------------------------------------------------------------
void AI_SetSpawnValue( userEntity_t *self, int nSpawnValue )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( nSpawnValue & SPAWN_WANDER )
	{
		hook->nSpawnValue |= SPAWN_WANDER;

//		if ( hook->type == TYPE_PRISONER )
//		{
//			AI_AddNewGoal( self, GOALTYPE_PRISONER_WANDER );
//		}
//		else
		{
			AI_AddNewGoal( self, GOALTYPE_WANDER );
		}
	}
	if ( nSpawnValue & SPAWN_PATHFOLLOW )
	{
		hook->nSpawnValue |= SPAWN_PATHFOLLOW;

		if ( _stricmp( self->className, "monster_cambot" ) == 0 )
		{
			AI_AddNewGoal( self, GOALTYPE_CAMBOT_PATHFOLLOW );
		}
		else
		{
			AI_AddNewGoal( self, GOALTYPE_PATHFOLLOW );
		}
	}
	if ( nSpawnValue & SPAWN_NODE_WANDER )
	{
		hook->nSpawnValue |= SPAWN_NODE_WANDER;
		AI_AddNewGoal( self, GOALTYPE_WANDER );
	}
	if ( nSpawnValue & SPAWN_RANDOM_WANDER )
	{
		hook->nSpawnValue |= SPAWN_RANDOM_WANDER;
//		if ( hook->type == TYPE_PRISONER )
//		{
//			AI_AddNewGoal( self, GOALTYPE_PRISONER_WANDER );
//		}
//		else
		{
			AI_AddNewGoal( self, GOALTYPE_RANDOMWANDER );
		}
	}
	
	if ( nSpawnValue & SPAWN_IGNORE_PLAYER )
	{
		hook->nSpawnValue |= SPAWN_IGNORE_PLAYER;
		hook->ai_flags |= AI_IGNORE_PLAYER;
	}

	if ( nSpawnValue & SPAWN_SNIPE )
	{
		hook->nSpawnValue |= SPAWN_SNIPE;
		hook->ai_flags |= AI_SNIPE;
		
		hook->nAttackMode = ATTACKMODE_SNIPING;
		AI_AddNewGoal( self, GOALTYPE_SNIPE );
	}

	if ( nSpawnValue & SPAWN_DO_NOT_FLY )
	{
		if ( self->movetype == MOVETYPE_FLY || self->movetype == MOVETYPE_HOVER )
        {
            self->movetype = MOVETYPE_WALK;
        }

        hook->nSpawnValue |= SPAWN_DO_NOT_FLY;
	}

	if ( nSpawnValue & SPAWN_DO_NOT_MOVE )
	{
		hook->nSpawnValue |= SPAWN_DO_NOT_MOVE;
	}

	// SCG[11/8/99]: Deathtarget flag removed.  Added deathtarget to userEntity_t
/*
    if ( nSpawnValue & SPAWN_DEATHTARGET )
    {
        hook->nSpawnValue |= SPAWN_DEATHTARGET;
    }
*/

	if ( nSpawnValue & SPAWN_TAKECOVER )
	{
		hook->nSpawnValue |= SPAWN_TAKECOVER;
		AI_AddNewGoal( self, GOALTYPE_TAKECOVER );
	}

    if ( nSpawnValue & SPAWN_ALWAYSGIB )
    {
		hook->nSpawnValue |= SPAWN_ALWAYSGIB;
		self->fragtype    |= FRAGTYPE_ALWAYSGIB;
    }

	if ( nSpawnValue & SPAWN_CEILING )
	{
		hook->nSpawnValue |= SPAWN_CEILING;
	}
	
	if ( hook->szScriptName && !AI_HasGoalInQueue(self,GOALTYPE_SCRIPTACTION))
	{
		AI_AddScriptActionGoal( self, hook->szScriptName );
	}

}

// ----------------------------------------------------------------------------
//
// Name:		AI_ProcessSpawnFlag
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ProcessSpawnFlag( userEntity_t *self, char *value )
{
	_ASSERTE( self );
	int nSpawnValue = -1;
	if ( _stricmp(value, "") != 0 )
	{
		nSpawnValue = atoi( value );
	}
	_ASSERTE( nSpawnValue != -1 );

	AI_SetSpawnValue( self, nSpawnValue );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ProcessBounds
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ProcessBoundingBox( userEntity_t *self, char *value )
{
	_ASSERTE( self );
	CVector mins, maxs;
	sscanf( value, " %f %f %f %f %f %f", &mins.x, &mins.y, &mins.z, &maxs.x, &maxs.y, &maxs.z );

	playerHook_t *hook = AI_GetPlayerHook( self );
	AI_SetBoundedBox( hook, mins, maxs );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ParseEpairs
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ParseEpairs( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	self->think		= AI_TaskThink;
	AI_SetNextThinkTime( self, 0.2f );

	if (self->className)
	{
		ai_debug_print (self, "%s\n", self->className);
	}

	if (self->epair)
	{
	    int i = 0;
	    while ( self->epair[i].key )
	    {
		    if ( _stricmp(self->epair[i].key, "aistate") == 0 )
		    {
			    AI_ProcessInitialAIState( self, self->epair[i].value );
		    }
		    if ( _stricmp( self->epair[i].key, "spawnflags" ) == 0 )
		    {
			    AI_ProcessSpawnFlag( self, self->epair[i].value );
		    }
		    if ( _stricmp( self->epair[i].key, "BoundingBox" ) == 0 )
		    {
			    AI_ProcessBoundingBox( self, self->epair[i].value );
		    }
		    if ( _stricmp( self->epair[i].key, "target" ) == 0 )
		    {
			    self->target = self->epair[i].value;
		    }
		    if ( _stricmp( self->epair[i].key, "deathtarget" ) == 0 )
		    {
			    self->deathtarget = self->epair[i].value;
		    }
		    if ( _stricmp( self->epair[i].key, "SightRange" ) == 0 )
		    {
			    hook->active_distance = atoi( self->epair[i].value );
		    }
		    if ( _stricmp( self->epair[i].key, "spawnname" ) == 0 )
		    {
			    self->spawnname = self->epair[i].value;
		    }

		    if ( _stricmp( self->epair[i].key, "default_aiscript" ) == 0 )
		    {
			    // set up a default script to play
			    hook->szScriptName = strdup( self->epair[i].value );

			    AI_AddScriptActionGoal( self, hook->szScriptName );
		    }

		    i++;
	    }
    }

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
    if ( !pCurrentGoal )
    {
        pCurrentGoal = AI_AddNewGoal( self, GOALTYPE_IDLE );        
    }
}

// ----------------------------------------------------------------------------
//
// Name:		AI_InitMonster
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
playerHook_t *AI_InitMonster( userEntity_t *self, int type )
{
	_ASSERTE( self );
	playerHook_t *hook = NULL;

	if ( self->pMapAnimationToSequence )
	{
		delete self->pMapAnimationToSequence;
		self->pMapAnimationToSequence = NULL;
	}
	
    SEQUENCEMAP_AllocSequenceMap( self );
	
	//NSS[11/16/99]:
	self->nSidekickFlag = 0;

	////////////////////////////////
	// set up standard entity values
	////////////////////////////////
	if ( self->epair )
	{
		for ( int i = 0; self->epair[i].key != NULL; i++ )
		{
			if ( !stricmp(self->epair[i].key, "target") )
			{
				self->target = self->epair[i].value;
			}
			// load alternate skin
			else 
            if ( !stricmp(self->epair[i].key, "skin") ) 
			{
				self->s.skinnum = gstate->ImageIndex( self->epair[i].value );
				self->s.effects2 |= EF2_IMAGESKIN;
			}
			else if(!_stricmp(self->epair[i].key,"itemspawnflags"))
			{
				self->nSidekickFlag = atol(self->epair[i].value);	
			}
		}
	}

	self->s.flags	= SFL_TARGETABLE;
	self->solid		= SOLID_SLIDEBOX;
	self->movetype	= MOVETYPE_WALK;
	
	//NSS[11/22/99]:Added to prevent bouncey bouncy ontop of bbox things.
	self->s.iflags |= IF_SV_SENDBBOX;
	
	if ( type == TYPE_BOT )
	{
		self->flags = ( FL_BOT | FL_PUSHABLE );
		self->clipmask  = MASK_PLAYERSOLID;
	}
	else 
	if ( type == TYPE_SUPERFLY || type == TYPE_MIKIKO || type == TYPE_MIKIKOFLY)
	{
		self->flags = ( FL_BOT | FL_PUSHABLE );
		self->clipmask  = MASK_NPCSOLID;
	}
	else
	{
		self->flags = FL_MONSTER;// + FL_PUSHABLE;
		// mdm99.05.24 - this was causing the player to clip into monsters
		//self->clipmask  = MASK_SOLID;
		self->clipmask  = MASK_MONSTERSOLID;
	}

	//	MUST DO |= HERE OR SVF_NEWSPAWN FLAG WILL BE CLEARED!!!
	self->deadflag		= DEAD_NO;
	self->svflags		|= ( SVF_NOSPLASH | SVF_NOTHUD | SVF_MONSTER );
	self->takedamage	= DAMAGE_YES;
	self->health		= 100;
	self->inventory		= NULL;

	self->viewheight = 22.0;

	//	turn rates for each axis in degrees per second
	self->ang_speed.Set( 90, 360, 90 );

	self->ideal_ang = self->s.angles;
//	self->max_speed = 1.0;
	self->input_entity		= self;
	
	self->elasticity		= 0.5;	// half the bounce of a normal MOVETYPE_BOUNCE
	self->mass				= 1.0;	// same mass as a player
    self->delay             = 0.0f;

	if ( !self->className )
	{
		self->className = "monster_default";
	}
	if ( !self->netname )
	{
		self->netname = "default monster";
	}

	self->view_ofs.Set(0.0, 0.0, 22.0);

	self->s.modelindex	= 0;
	//gstate->SetSize(self, -16.0, -16.0, -24.0, 16.0, 16.0, 32.0);
	//gstate->SetOrigin2(self, self->s.origin);

	////////////////////////////////
	// initialize user hook
	////////////////////////////////
	hook = (playerHook_t *) self->userHook;
	if ( hook )
	{
		if ( hook->pRespawnData )
		{
			// cek[8-2-00]: mismatch
			//gstate->X_Free( hook->pRespawnData );
			delete hook->pRespawnData;
			hook->pRespawnData = NULL;
		}
		gstate->X_Free( hook );
		self->userHook = NULL;
	}
	self->userHook = ( playerHook_t * ) gstate->X_Malloc( sizeof( playerHook_t ), MEM_TAG_HOOK );
	if ( !self->userHook )
	{
		com->Error( "MEM Allocation failed" );	
	}	 
	// SCG[10/23/99]: clear this shit out
	memset( self->userHook, 0, sizeof( playerHook_t ) );

	hook = AI_GetPlayerHook( self );

	
	
	
	self->flags							|= RF_MINLIGHT;
	hook->owner							= NULL;
	hook->dflags						= DFL_EVIL;
	hook->goalFlags						= GMASK_MONSTERDEFAULT;
	hook->szScriptName					= NULL;
	self->s.frameInfo.frameTime			= FRAMETIME_FPS20;
	self->s.frameInfo.frameInc			= 1;
	self->s.frameInfo.next_frameTime	= gstate->time + self->s.frameInfo.frameTime;	

	hook->last_origin = self->s.origin + CVector(128, 128, 128);

	////////////////////////////////////////////////////////////////
	// map node stuffffff
	////////////////////////////////////////////////////////////////

	hook->attack_dist		= 250.0;
	hook->jump_attack_dist	= hook->attack_dist;
	hook->attack_finished	= 0;

	hook->upward_vel		= 270.0;
	hook->run_speed			= 125;
	hook->walk_speed		= 25;
	hook->attack_speed		= 125;
	hook->max_jump_dist		= ai_max_jump_dist (hook->run_speed, hook->upward_vel);
	hook->jump_chance		= 0;

	hook->nTargetCounter	= 0;
	hook->nTargetFrequency	= 3;	//	check every 3 frames

	ai_setfov( self, 180.0 );

	// create a list that points to the master nodelist header
	hook->pPathList = PATHLIST_Create ();


	////////////////////////////////////////////////////////////////
	// init goal
	////////////////////////////////////////////////////////////////
	self->goalentity				= NULL;


	////////////////////////////////////////////////////////////////
	// init cluster hierarchy
	////////////////////////////////////////////////////////////////

	clusterDef_t *pDef = &self->s.cDef[0];

	pDef->clusterIdx = 0;
	pDef->angles.Set(0.0f,0.0f,0.0f);
	pDef++;
	pDef->clusterIdx = 0;
	pDef->angles.Set(0.0f,0.0f,0.0f);
	pDef++;
	pDef->clusterIdx = 0;
	pDef->angles.Set(0.0f,0.0f,0.0f);

	self->s.numClusters = 0;

    // Logic[5/11/99]: Cut and paste boo-boo?
/*
	////////////////////////////////////////////////////////////////
	// init cluster hierarchy
	////////////////////////////////////////////////////////////////

	pDef = &self->s.cDef[0];

	pDef->clusterIdx = 0;
	pDef->angles.Set(0.0f,0.0f,0.0f);
	pDef++;
	pDef->clusterIdx = 0;
	pDef->angles.Set(0.0f,0.0f,0.0f);
	pDef++;
	pDef->clusterIdx = 0;
	pDef->angles.Set(0.0f,0.0f,0.0f);

	self->s.numClusters = 0;
*/
	///////////////////////////////////////////////////////////////
	// set up thinks
	///////////////////////////////////////////////////////////////

	hook->begin_jump				= NULL;
	hook->begin_stand				= NULL;
	hook->begin_follow				= NULL;
	hook->begin_path_follow			= NULL;
	hook->begin_time_stamp_follow	= NULL;
	hook->begin_node_wander			= NULL;
	hook->begin_wander				= NULL;
	hook->begin_attack				= NULL;
	hook->begin_seek				= NULL;
	hook->wander_sound				= NULL;
	hook->sight_sound				= NULL;
	hook->begin_turn				= NULL;
	hook->begin_path_wander			= NULL;
	hook->begin_action				= NULL;
	hook->begin_transition			= NULL;
	hook->begin_node_retreat		= NULL;
	hook->find_target				= NULL;
	hook->sight						= NULL;
	
	hook->special_think				= NULL;
	hook->continue_think			= NULL;

	hook->fnAttackFunc		= NULL;
	hook->fnStartAttackFunc	= NULL;
	hook->fnTakeCoverFunc	= NULL;

	hook->fnFindTarget		= AI_FindNearClientOrSidekick;

	hook->fnStartJumpUp		= AI_StartJumpUp;
	hook->fnJumpUp			= AI_JumpUp;
	hook->fnStartJumpForward = AI_StartJumpForward;
	hook->fnJumpForward		= AI_JumpForward;
	
	hook->fnStartCower		= NULL;
	hook->fnCower			= NULL;
    hook->fnStartIdle       = NULL;

	hook->fnInAttackRange   = NULL;

	self->die				= AI_StartDie;
	self->pain				= AI_StartPain;

	self->prethink			= AI_Update;
	self->think				= AI_TaskThink;
	self->postthink			= AI_PostThink;

	hook->think_time		= THINK_TIME;

	hook->back_away_dist	= 128;
	hook->active_distance	= 2000;


	///////////////////////////////////////////////////////////////
	//	set dflags
	///////////////////////////////////////////////////////////////
/*
	if ( !ai_disable )
	{
		hook->dflags |= (DFL_CANSTRAFE + DFL_EVIL + DFL_TIMEFOLLOW);
	}
	else
	{
		hook->dflags |= (DFL_CANSTRAFE + DFL_GOOD + DFL_TIMEFOLLOW);
	}
*/

	hook->sound_time	= gstate->time + Random() * 10.0;

	// save the monster's type
	hook->type			= type;
	hook->pain_chance	= 75;

	// malloc up the space for our frame data
	//hook->frames		= new(HOOK_MALLOC) sequenceTable_t;

	//	get unique ID for this entity
	hook->ID			= com->GenID ();
	hook->ai_flags		|= AI_ALWAYS_USENODES;

	hook->pGoals		= GOALSTACK_Allocate();
	
	hook->bInScriptMode = FALSE;
	hook->pScriptGoals	= GOALSTACK_Allocate();

	hook->nMoveCounter	= 0;
	// store the original position for this monster
	hook->startPosition = self->s.origin;

	hook->nAttackType	= ATTACK_GROUND_MELEE;
	hook->nAttackMode	= ATTACKMODE_NORMAL;

	hook->nMovementState	= MOVEMENT_GROUND;
	hook->nWaitCounter		= 0;

	hook->nCanUseFlag		= CANUSE_ALL;
	hook->bBounded			= FALSE;
	hook->boundedBox.mins.Set( -4096, -4096, -4096 );
	hook->boundedBox.maxs.Set( 4096, 4096, 4096 );
	hook->nSpawnValue		 = 0;
	
	hook->fTenUpdatesDistance = 0.0f;
	hook->nMovingCounter	 = 0;
	hook->fTaskFinishTime	 = gstate->time + 30.0f;
	hook->bOkToAttack		 = TRUE;

	hook->fAttackTime		 = -1.0f;
	hook->fNextPathTime		 = 0.0f;

	hook->bInTransition		= FALSE;
	hook->nTransitionIndex	= 0;
	hook->fTransitionTime	= 0.0f;
	hook->pTransitions		= NULL;

	hook->ai_flags			|= AI_CANDODGE;
	hook->nSpecificAttackMode = SPECIFICATTACKMODE_NONE;

	// SCG[12/15/99]: Ok for debug but not for release
#ifdef _DEBUG
	if ( strstr( gstate->mapName, "zoo" ) )
	{
		AI_DisableAI();
	}
#endif _DEBUG

    hook->fSoundOutDistance = 0.0f;
    hook->ai_flags |= AI_ATTACK;
    
    hook->pWantItem     = NULL;
    hook->bInAttackMode = FALSE;

    hook->fMinAttenuation   = ATTN_NORM_MIN;
    hook->fMaxAttenuation   = ATTN_NORM_MAX;

//    hook->nNumTasksInOneUpdate  = 0;
//    hook->nCurrentTaskType      = TASKTYPE_NONE;

	// SCG[11/24/99]: Save game stuff
	self->save = AI_SaveMonsterPlayerHook;
	self->load = AI_LoadMonsterPlayerHook;

    alist_add( self );

	return hook;
}




// ****************************************************************************
//						Script specific functions
// ****************************************************************************

// ----------------------------------------------------------------------------
//
// Name:		AI_RemoveCurrentScriptGoal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_RemoveCurrentScriptGoal( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = hook->pScriptGoals;

	GOALSTACK_RemoveCurrentGoal( pGoalStack );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	if ( pCurrentTask )
	{
		AI_StartTask( self, pCurrentTask );
	}
	else
	{
		// there are no more tasks for this goal, determine if the goal is 
		// satisfied, if not then start the process over
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		if ( pCurrentGoal && GOAL_IsSatisfied( pCurrentGoal ) == FALSE )
		{
			// start the process all over again
			AI_AddInitialTasksToGoal( self, pCurrentGoal );
		}
	}
}


void AI_RemoveIdleScriptGoal(userEntity_t *self) 
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if (!hook->pScriptGoals) 
	{
		return;
	}

    //get the number of goals on the stack.
    int num_goals = GOALSTACK_GetNumGoals(hook->pScriptGoals);
    if (num_goals <= 1) 
	{
		return;
	}

    //get the top goal.
    GOAL_PTR top_goal = GOALSTACK_GetCurrentGoal(hook->pScriptGoals);
    if (top_goal == NULL) 
	{
		return;
	}

    //check if the goal is an idle goal
    GOALTYPE goal_type = GOAL_GetType(top_goal);
    if (goal_type != GOALTYPE_IDLE) 
	{
		return;
	}

    //remove the top goal.
//    GOALSTACK_RemoveCurrentGoal(hook->pScriptGoals);
	AI_RemoveCurrentGoal (self);
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewScriptGoalAtBack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR AI_AddNewScriptGoalAtBack(userEntity_t *self, GOALTYPE nNewGoalType, AIDATA_PTR aiData) 
{
    _ASSERTE( self );
    AI_SetNextThinkTime( self, 0.1f );

	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !hook->pScriptGoals )
	{
		hook->pScriptGoals = GOALSTACK_Allocate();
	}

	GOALSTACK_PTR pGoalStack = hook->pScriptGoals;

	GOAL_PTR pNewGoal = GOAL_Allocate( nNewGoalType, aiData );
	if ( !pNewGoal )
	{
		return NULL;
	}

	GOALSTACK_AddAtBack( pGoalStack, pNewGoal );
	
	AI_AddInitialTasksToGoal( self, pNewGoal, FALSE );

    //make sure we dont have an idle at the top of our script stack.
    AI_RemoveIdleScriptGoal(self);

	return pNewGoal;
}

GOAL_PTR AI_AddNewScriptGoalAtBack( userEntity_t *self, GOALTYPE nNewGoalType, userEntity_t *pEntity )
{
    _ASSERTE( self );
    AI_SetNextThinkTime( self, 0.1f );

	playerHook_t *hook = AI_GetPlayerHook( self );
	if ( !hook->pScriptGoals )
	{
		hook->pScriptGoals = GOALSTACK_Allocate();
	}

	GOALSTACK_PTR pGoalStack = hook->pScriptGoals;

	GOAL_PTR pNewGoal = GOAL_Allocate( nNewGoalType, pEntity );
	if ( !pNewGoal )
	{
		return NULL;
	}

	GOALSTACK_AddAtBack( pGoalStack, pNewGoal );
	
	AI_AddInitialTasksToGoal( self, pNewGoal, FALSE );

    //make sure we dont have an idle at the top of our script stack.
    AI_RemoveIdleScriptGoal(self);

	return pNewGoal;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewScriptGoalAtBack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR AI_AddNewScriptGoalAtBack(userEntity_t *self, GOALTYPE nNewGoalType) 
{
	_ASSERTE( self );
    AI_SetNextThinkTime( self, 0.1f );

	playerHook_t *hook = AI_GetPlayerHook( self );
	if (!hook->pScriptGoals) 
	{
		hook->pScriptGoals = GOALSTACK_Allocate();
	}

	GOALSTACK_PTR pGoalStack = hook->pScriptGoals;

	GOAL_PTR pNewGoal = GOAL_Allocate(nNewGoalType);
	if (!pNewGoal) 
	{
		return NULL;
	}

	GOALSTACK_AddAtBack(pGoalStack, pNewGoal);
	
	AI_AddInitialTasksToGoal(self, pNewGoal, FALSE);

    //make sure we dont have an idle at the top of our script stack.
    AI_RemoveIdleScriptGoal(self);

	return pNewGoal;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewScriptGoalAtBack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR AI_AddNewScriptGoalAtBack( userEntity_t *self, GOALTYPE nNewGoalType, const CVector &position )
{
    _ASSERTE( self );
    AI_SetNextThinkTime( self, 0.1f );

	playerHook_t *hook = AI_GetPlayerHook( self );
	if ( !hook->pScriptGoals )
	{
		hook->pScriptGoals = GOALSTACK_Allocate();
	}

	GOALSTACK_PTR pGoalStack = hook->pScriptGoals;

	GOAL_PTR pNewGoal = GOAL_Allocate( nNewGoalType, position );
	if ( !pNewGoal )
	{
		return NULL;
	}

	GOALSTACK_AddAtBack( pGoalStack, pNewGoal );
	
	AI_AddInitialTasksToGoal( self, pNewGoal, FALSE );

    //make sure we dont have an idle at the top of our script stack.
    AI_RemoveIdleScriptGoal(self);

    return pNewGoal;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewScriptGoalAtBack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR AI_AddNewScriptGoalAtBack( userEntity_t *self, GOALTYPE nNewGoalType, const float fValue )
{
    _ASSERTE( self );
    AI_SetNextThinkTime( self, 0.1f );

	playerHook_t *hook = AI_GetPlayerHook( self );
	if ( !hook->pScriptGoals )
	{
		hook->pScriptGoals = GOALSTACK_Allocate();
	}

	GOALSTACK_PTR pGoalStack = hook->pScriptGoals;

	GOAL_PTR pNewGoal = GOAL_Allocate( nNewGoalType, fValue );
	if ( !pNewGoal )
	{
		return NULL;
	}

	GOALSTACK_AddAtBack( pGoalStack, pNewGoal );
	
	AI_AddInitialTasksToGoal( self, pNewGoal, FALSE );

    //make sure we dont have an idle at the top of our script stack.
    AI_RemoveIdleScriptGoal(self);

    return pNewGoal;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddNewScriptGoalAtBack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR AI_AddNewScriptGoalAtBack( userEntity_t *self, GOALTYPE nNewGoalType, const int nValue )
{
    _ASSERTE( self );
    AI_SetNextThinkTime( self, 0.1f );

	playerHook_t *hook = AI_GetPlayerHook( self );
	if ( !hook->pScriptGoals )
	{
		hook->pScriptGoals = GOALSTACK_Allocate();
	}

	GOALSTACK_PTR pGoalStack = hook->pScriptGoals;

	GOAL_PTR pNewGoal = GOAL_Allocate( nNewGoalType, nValue );
	if ( !pNewGoal )
	{
		return NULL;
	}

	GOALSTACK_AddAtBack( pGoalStack, pNewGoal );
	
	AI_AddInitialTasksToGoal( self, pNewGoal, FALSE );

    //make sure we dont have an idle at the top of our script stack.
    AI_RemoveIdleScriptGoal(self);

    return pNewGoal;
}

// ****************************************************************************
//						End Script specific functions
// ****************************************************************************



// ----------------------------------------------------------------------------
//
// Name:		AI_GetClosestSwitch
// Description:
// Input:
// Output:
// Note:
//
// ---------------------------------------------------------------------------
userEntity_t *AI_GetClosestSwitch( userEntity_t *pTrain, CVector &currentPosition )
{
	_ASSERTE( pTrain );
	userEntity_t *pSwitch = NULL;

	// func_train and func_nodetrain are handled differently
	
	if ( !stricmp(pTrain->className, "func_train") )
	{
		if ( pTrain->targetname )
		{
			// find a button that 
			pSwitch = com->FindClosestEntityPt( currentPosition, "func_button", pTrain->targetname );
		}
	}
// SCG[2/13/00]: 
/*
	else 
	if ( !stricmp(pTrain->className, "func_nodetrain") )
	{
		// not finished yet

	}
*/
	return (pSwitch);
}

// ----------------------------------------------------------------------------
//
// Name:		AI_GetSwitchOnTrain
// Description:
// Input:
// Output:
// Note:
//
// ---------------------------------------------------------------------------
userEntity_t *AI_GetSwitchOnTrain( userEntity_t *self )
{
	_ASSERTE( self );
	userEntity_t *pSwitch = NULL, *child = NULL, *closest = NULL;

	// func_train and func_nodetrain are handled differently
	
	// on func_train's, we don't know if a certain button will take us to a desired location,
	// all we know is that it will activate the train.. so that is the button that will be
	// returned

	if ( !stricmp(self->className, "func_train") )
	{
		// get a pointer to the child list
		child = self->teamchain;

		if ( !child )
		{
			return (NULL);
		}

		// while we have valid pointers
		while ( child )
		{
			// find the button on the train that is within sight of the rider
			if ( !stricmp( child->className, "func_button" ) )
			{
				// does this button target the train?
				if ( self->targetname && child->target && !stricmp ( self->targetname, child->target ) )
				{
					pSwitch = child;
					break;
				}
			}
			// get next child
			child = child->teamchain;
		}
	}
// SCG[2/13/00]: 
/*
	else 
	if ( !stricmp (self->className, "func_nodetrain") )
	{
		// not finished yet

	}
*/
	return (pSwitch);
}


int SIDEKICK_SynchActions(userEntity_t *self, float Value);



// ----------------------------------------------------------------------------
// NSS[12/5/99]:
// Name:		AI_HandleTrainUse
// Description:
// Input:
// Output:
// Note:
//
// ---------------------------------------------------------------------------
int AI_HandleTrainUse( userEntity_t *self, NODEHEADER_PTR pNodeHeader,
					   MAPNODE_PTR pCurrentNode, MAPNODE_PTR pNextNode )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *pSwitchForTrain;

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( pGoalStack );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );

	userEntity_t *pOwner = hook->owner;
	int bRetValue = FALSE;
	PATHNODE_PTR pPathNode = hook->pPathList->pPath;

	if ( (pCurrentNode->node_type & (NODETYPE_TRAIN | NODETYPE_ELEVATOR)) &&
		 pNextNode && (pNextNode->node_type & (NODETYPE_TRAIN | NODETYPE_ELEVATOR)) )
	{
		userEntity_t *pTrain = com->FindNodeTarget( NODE_GetTarget(pCurrentNode) );
		if(!pTrain)
		{

			AI_Dprintf("Node does not target a train/ elevator... fixeme Kelly.\n");
			return FALSE;
		}
		// NSS[2/14/00]:Try to prevent a cluster fuck from happening
		if(AI_IsSidekick(hook))
		{
			userEntity_t *pOtherSidekick = SIDEKICK_GetOtherSidekick( self );
			playerHook_t *oHook = AI_GetPlayerHook(pOtherSidekick);
			if(pOtherSidekick && AI_IsSidekick(oHook) &&hook->type == TYPE_MIKIKO)
			{
				float fDistanceO = VectorDistance(self->s.origin,pOtherSidekick->s.origin);
				if((fDistanceO < 128.0f && AI_IsVisible(self,pOtherSidekick))||(!AI_IsOnTrain( pOtherSidekick ) && !AI_IsTrainStopped(pTrain)))
				{
					AI_AddNewTask( self, TASKTYPE_WAIT, 2.25f );
					return TRUE;
				}
			}
		}

		// NSS[12/5/99]:Handle that special case where the player leaves the sidekicks behind
		// and they want to follow, but the elevator has already left.
		if ( AI_IsSidekick(hook) && pOwner && pCurrentGoal && GOAL_GetType( pCurrentGoal ) == GOALTYPE_FOLLOW )
		{		
			if ( AI_IsOnTrain( pOwner ) && !AI_IsOnTrain( self ) && !AI_IsTrainStopped(pTrain))
			{
				MAPNODE_PTR pEndTrainNode = NULL;
				MAPNODE_PTR pAfterTrainNode = NULL;
				AI_GetEndNodeTrainNodes( pNodeHeader, pPathNode, pEndTrainNode, pAfterTrainNode );
				AI_AddNewTaskAtFront( self, TASKTYPE_WAIT,1.0f );
				return TRUE;
			}	
			float OZDistance = VectorZDistance(pOwner->s.origin, self->s.origin);
			if(OZDistance < 64.0f)
			{
				return FALSE;
			}
		}

		if ( AI_IsOnTrain( self ) )
		{
			// need to use the train to get to the destination
			userEntity_t *pTrain = com->FindNodeTarget( NODE_GetTarget(pCurrentNode) );
			
			if ( !pTrain )
			{
				AI_Dprintf("Node does not target a train/ elevator... fixeme Kelly.\n");
				return FALSE;
			}

			pSwitchForTrain = AI_GetSwitchOnTrain( pTrain );
			if(!pSwitchForTrain) //There is no switch on the train like there is SUPPOSED to be because people always change their design and never let anyone know... soo...
			{
				pSwitchForTrain = AI_GetClosestSwitch( pTrain, self->s.origin );
				if(!pSwitchForTrain)
				{
					AI_Dprintf("There is no switch on the train nor is there a switch close to me and the elevator/train node... report this bug to Noel and Kelly.\n");
				}
			}
			if ( pSwitchForTrain )
			{
				MAPNODE_PTR pEndTrainNode = NULL;
				MAPNODE_PTR pAfterTrainNode = NULL;
				AI_GetEndNodeTrainNodes( pNodeHeader, pPathNode, pEndTrainNode, pAfterTrainNode );
				_ASSERTE( pEndTrainNode );

				GOAL_PTR pCurrentGoal = AI_AddNewGoal( self, GOALTYPE_GENERALUSE );
				AI_AddNewTask( self, TASKTYPE_WAIT, 0.25f );
				AI_AddNewTask( self, TASKTYPE_ACTIVATESWITCH, pSwitchForTrain );
				AI_AddNewTask( self, TASKTYPE_WAIT, 3.0f );
				AI_AddNewTask( self, TASKTYPE_WAITFORTRAINTOSTOP,pTrain);
				AI_StartWait(self);
				//AI_StartActivateSwitch( self );
				bRetValue = TRUE;
			}
		}
		else if(AI_IsTrainStopped(pTrain))
		{
			
			MAPNODE_PTR pEndTrainNode = NULL;
			MAPNODE_PTR pAfterTrainNode = NULL;
			AI_GetEndNodeTrainNodes( pNodeHeader, pPathNode, pEndTrainNode, pAfterTrainNode );
			//_ASSERTE( pEndTrainNode );

			if(AI_IsTrainStopped(pTrain))//Is the train moving?
			{
				pSwitchForTrain = AI_GetClosestSwitch( pTrain, self->s.origin );
				if(!pSwitchForTrain)
				{
					AI_Dprintf("There is no switch on the train nor is there a switch close to me and the elevator/train node... report this bug to Noel and Kelly if there is supposed to be one.\n");
					// NSS[12/5/99]:It may be automatic... so wait.
					AI_AddNewTaskAtFront( self, TASKTYPE_WAIT, 1.0f );
					return FALSE;
				}

				// NSS[12/5/99]:Get on the train only if it is stopped and is next to us
				if(VectorDistance(self->s.origin,pTrain->s.origin) < 64.0f)
				{
					AI_AddNewTask( self, TASKTYPE_ACTIVATESWITCH, pSwitchForTrain );
					AI_AddNewTask( self, TASKTYPE_WAITFORTRAINTOCOME, pEndTrainNode->nIndex );
					bRetValue = TRUE;
				}
				else
				{
					// NSS[12/5/99]:Otherwise get the bastard to come back.
					if ( pSwitchForTrain )
					{
						GOAL_PTR pCurrentGoal = AI_AddNewGoal( self, GOALTYPE_GENERALUSE );

						AI_AddNewTask( self, TASKTYPE_WAIT, 0.25f );
						AI_AddNewTask( self, TASKTYPE_ACTIVATESWITCH, pSwitchForTrain );

						MAPNODE_PTR pNearNode = NULL;
						for ( int i = 0; i < pCurrentNode->nNumLinks; i++ )
						{
							MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, pCurrentNode->aLinks[i].nIndex );
							float fXYDistance = VectorXYDistance( self->s.origin, pNode->position );
							if ( fXYDistance < 256.0f && !(pNode->node_type&(NODETYPE_TRAIN | NODETYPE_ELEVATOR)))
							{
								pNearNode = pNode;
							}
						}
						AI_AddNewTask( self, TASKTYPE_MOVETOLOCATION, pNearNode->position );
						AI_AddNewTask( self, TASKTYPE_WAIT, 3.0f );
						AI_AddNewTask( self, TASKTYPE_WAITFORTRAINTOSTOP,pTrain);
						AI_StartWait(self);
						bRetValue = TRUE;
					}
				}

			}
			else
			{
				GOAL_PTR pCurrentGoal = AI_AddNewGoal( self, GOALTYPE_GENERALUSE );
				// find a point to move to and wait
				MAPNODE_PTR pNearNode = NULL;
				for ( int i = 0; i < pCurrentNode->nNumLinks; i++ )
				{
					MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, pCurrentNode->aLinks[i].nIndex );
					float fXYDistance = VectorXYDistance( self->s.origin, pNode->position );
					if ( fXYDistance < 256.0f && !(pNode->node_type&(NODETYPE_TRAIN | NODETYPE_ELEVATOR)))
					{
						pNearNode = pNode;
					}
				}
				_ASSERTE( pNearNode );
				AI_AddNewTask( self, TASKTYPE_WAIT, 0.25f );
				AI_AddNewTask( self, TASKTYPE_MOVETOLOCATION, pNearNode->position );
				AI_AddNewTask( self, TASKTYPE_WAITFORTRAINTOCOME, pEndTrainNode->nIndex );
				bRetValue = TRUE;
			}
		}
	}
	else if ( pNextNode && (pNextNode->node_type & NODETYPE_TRAIN))//We are not on the train yet.
	{
	
		
		userEntity_t *pTrain = com->FindNodeTarget( NODE_GetTarget(pNextNode) );
		MAPNODE_PTR pEndTrainNode = NULL;
		MAPNODE_PTR pAfterTrainNode = NULL;
		AI_GetEndNodeTrainNodes( pNodeHeader, pPathNode, pEndTrainNode, pAfterTrainNode );
		if(!pTrain)
		{

			AI_Dprintf("Node does not target a train/ elevator... fixeme Kelly.\n");
			return FALSE;
		}
		// NSS[2/14/00]:Try to prevent a cluster fuck from happening
		if(AI_IsSidekick(hook))
		{
			userEntity_t *pOtherSidekick = SIDEKICK_GetOtherSidekick( self );
			playerHook_t *oHook = AI_GetPlayerHook(pOtherSidekick);
			if(pOtherSidekick && AI_IsSidekick(oHook) && hook->type == TYPE_MIKIKO)
			{
				float fDistanceO	= VectorDistance(self->s.origin,pOtherSidekick->s.origin);
				float fDistanceT	= VectorDistance(self->s.origin,pTrain->s.origin);
				float fDsitanceTO	= VectorDistance(pOtherSidekick->s.origin,pTrain->s.origin);
				if((fDistanceO < 128.0f && AI_IsVisible(self,pOtherSidekick) && fDistanceT > fDsitanceTO)||(AI_IsOnTrain( pOtherSidekick ) && !AI_IsTrainStopped(pTrain)))
				{
					AI_AddNewTask( self, TASKTYPE_WAIT, 2.25f );
					return TRUE;
				}
			}
		}

		// NSS[12/5/99]:Handle that special case where the player leaves the sidekicks behind
		// and they want to follow, but the elevator has already left.
		if (AI_IsSidekick(hook) &&  pOwner && pCurrentGoal && GOAL_GetType( pCurrentGoal ) == GOALTYPE_FOLLOW )
		{		
			if ( AI_IsOnTrain( pOwner ) && !AI_IsOnTrain( self ) && !AI_IsTrainStopped(pTrain))
			{
				MAPNODE_PTR pEndTrainNode = NULL;
				MAPNODE_PTR pAfterTrainNode = NULL;
				AI_GetEndNodeTrainNodes( pNodeHeader, pPathNode, pEndTrainNode, pAfterTrainNode );
				AI_AddNewTaskAtFront( self, TASKTYPE_WAIT,1.0f );
				return TRUE;
			}	
			float OZDistance = VectorZDistance(pOwner->s.origin, self->s.origin);
			if(OZDistance < 64.0f)
			{
				return FALSE;
			}

		}

		CVector Bottom_Point = pNextNode->position;
		Bottom_Point.z -= 64.0f;
		tr = gstate->TraceLine_q2( pNextNode->position, Bottom_Point, NULL, MASK_SOLID );
		//Is the train moving and is it close to the node we are going to?
		if(AI_IsTrainStopped(pTrain) && (tr.fraction >= 1.0f && !tr.startsolid) )
		{
			// NSS[12/5/99]:See if there is a closeby switch to bring it to us.
			pSwitchForTrain = AI_GetClosestSwitch( pTrain, self->s.origin );
			if(!pSwitchForTrain)
			{
				AI_Dprintf("There is no switch on the train nor is there a switch close to me and the elevator/train node... report this bug to Noel and Kelly if there is supposed to be one.\n");
				// NSS[12/5/99]:It may be automatic... so wait.
				AI_AddNewTaskAtFront( self, TASKTYPE_WAIT, 1.0f );
				return FALSE;
			}
			// NSS[12/5/99]:Otherwise get the bastard to come back.
			if ( pSwitchForTrain )
			{
				GOAL_PTR pCurrentGoal = AI_AddNewGoal( self, GOALTYPE_GENERALUSE );

				AI_AddNewTask( self, TASKTYPE_WAIT, 0.25f );
				AI_AddNewTask( self, TASKTYPE_ACTIVATESWITCH, pSwitchForTrain );

				MAPNODE_PTR pNearNode = NULL;
				for ( int i = 0; i < pCurrentNode->nNumLinks; i++ )
				{
					MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, pCurrentNode->aLinks[i].nIndex );
					float fXYDistance = VectorXYDistance( self->s.origin, pNode->position );
					if ( fXYDistance < 256.0f && !(pNode->node_type&(NODETYPE_TRAIN | NODETYPE_ELEVATOR)))
					{
						pNearNode = pNode;
					}
				}
				AI_AddNewTask( self, TASKTYPE_MOVETOLOCATION, pNearNode->position );
				AI_AddNewTask( self, TASKTYPE_WAIT, 3.0f );
				AI_AddNewTask( self, TASKTYPE_WAITFORTRAINTOSTOP,pTrain);
				AI_StartWait(self);
				bRetValue = TRUE;
			}
		}
		else if (!AI_IsTrainStopped(pTrain))
		{
			AI_AddNewTaskAtFront( self, TASKTYPE_WAIT, 0.25f );
		}
		else if((tr.fraction < 1.0f && !tr.startsolid && tr.ent && tr.ent->className && strstr(tr.ent->className,"train")) && AI_IsTrainStopped(pTrain))
		{
			AI_AddNewTaskAtFront( self, TASKTYPE_MOVETOLOCATION, pNextNode->position );
		}
		else
		{
			AI_Dprintf("uhhh... er.. what now?\n");
		}
	}
	return bRetValue;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsDoorOpen
// Description:
// Input:
// Output:
// Note:
//
// ---------------------------------------------------------------------------
__inline static int AI_IsDoorOpen( userEntity_t *self )
{
	_ASSERTE( self );

	int bOpen = FALSE;
    if(self)
	{
		if ( strstr( self->className, "func_door" ) )
		{
    		doorHook_t *pDoorHook = (doorHook_t *)self->userHook;
			if ( pDoorHook->state == STATE_TOP || pDoorHook->state == STATE_UP )
			{
				bOpen = TRUE;
			}
		}
		else
		if ( _stricmp( self->className, "func_wall" ) == 0 )
		{
			if ( self->solid == SOLID_NOT )
			{
				bOpen = TRUE;
			}				
		}
	}

	return bOpen;
}

// ----------------------------------------------------------------------------
// NSS[12/20/99]:
// Name:		Door_FindNodeTarget
// Description:Nice hack... since we have switches with the same name we must trace
// to make sure we can reach the target.
// Input:const char *name
// Output:userEntity_t * target
// Note:
// ---------------------------------------------------------------------------
userEntity_t *Door_FindNodeTarget (userEntity_t *self, const char *name)
{
	if ( name )
	{
		userEntity_t	*head;
		
		head = gstate->FirstEntity();

		while (head)
		{
			if (head->nodeTargetName)
			{
				if (!stricmp(head->nodeTargetName, name))
				{
					if(gstate->inPVS(self->s.origin, head->s.origin))
						return(head);
				}
			}

			head = gstate->NextEntity(head);
		}
	}

	return NULL;
}


// ----------------------------------------------------------------------------
// NSS[12/20/99]:
// Name:		AI_HandleDoorUse
// Description:Function to let AI handle opening doors.
// Input:userEntity_t *self, NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pCurrentNode, MAPNODE_PTR pNextNode
// Output:int (T/F)
// Note:
// ---------------------------------------------------------------------------
int AI_HandleDoorUse( userEntity_t *self, NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pCurrentNode, MAPNODE_PTR pNextNode )
{
	_ASSERTE( self && pNodeHeader && pCurrentNode );
	if ( !self || !pNodeHeader || !pCurrentNode || !pNextNode )
	{
		return FALSE;
	}
	
	playerHook_t *hook = AI_GetPlayerHook( self );

	int bRetValue = FALSE;

	if(AI_IsSidekick(hook))
	{
		// NSS[12/20/99]:If we are not the closest then let the one in front do all the door handling.
		if(!SIDEKICK_WhoIsClosest(self,pCurrentNode))
			return FALSE;
	}

	// NSS[12/20/99]:Handle Doors with buttons here
	if ( pCurrentNode->node_type & NODETYPE_DOORBUTTON || ( pNextNode && (pNextNode->node_type & NODETYPE_DOORBUTTON) ) )
	{
		MAPNODE_PTR pNode = pCurrentNode;
		if ( NODE_GetTarget( pCurrentNode ) == NULL )
		{
			pNode = pNextNode;
		}

		userEntity_t *pSwitch = com->FindNodeTarget( NODE_GetTarget(pNode) );// NSS[3/10/00]:
		
		if(!pSwitch)
		{
			AI_Dprintf("%s:Door has no target name associated with it! Fix me Kelly!\n",self->className);
			AI_RemoveCurrentTask(self,FALSE);
			return FALSE;
		}
		_ASSERTE( pSwitch );
		if(strstr(pSwitch->className,"door"))
		{
			// NSS[2/17/00]:Because some people have NO clue about nodes and how they work I am just going to handle it when DUMB people set node types of Door Button to target the door instead of the button that is supposed to open the door.
			if(!AI_IsDoorOpen( pSwitch ))
			{
				pSwitch->use( pSwitch, self, self );
				if(AI_IsSidekick(hook))
				{
					AI_AddNewTaskAtFront(self,TASKTYPE_SIDEKICK_STOP,2.5f);
					AI_ZeroVelocity(self);
				}
				else
				{
					AI_AddNewTaskAtFront(self,TASKTYPE_WAIT,0.5f);
					AI_AddNewTaskAtFront(self,TASKTYPE_MOVETOLOCATION,pNextNode->position);
				}
				return TRUE;
			}
			else
			{
				return FALSE;
			}
			
		}
		userEntity_t *pDoor = com->FindTarget( pSwitch->target );
		if ( pDoor && AI_IsDoorOpen( pDoor ) == FALSE && !AI_HasGoalInQueue(self,GOALTYPE_GENERALUSE))
		{
    		// NSS[3/10/00]:A few ..ehum... modifications.
			CVector Destination;
			Destination =  (pSwitch->absmax + pSwitch->absmin) * 0.50f;
			MAPNODE_PTR Node		=	NODE_GetClosestNode( self, Destination);
			//MAPNODE_PTR FinalNode	=	NODE_GetNode( pNodeHeader, pNextNode->next_node->nNodeIndex );
			if(Node && AI_FindPathToPoint( self, Node->position))
			{
    			CVector FinalPoint,Dir;
				Dir = pNextNode->position - pCurrentNode->position;
				Dir.Normalize();
				FinalPoint = pNextNode->position + (128.0f * Dir);
				MAPNODE_PTR FinalNode = NODE_GetClosestNode( self, FinalPoint);

				// NSS[3/10/00]:If we have a final node then let's go!
				if(FinalNode && FinalNode != pNextNode )
				{
					AI_SetStateRunning(hook);
					// NSS[3/10/00]:Add the tasks!
					GOAL_PTR pCurrentGoal = AI_AddNewGoal( self, GOALTYPE_GENERALUSE );
					AI_AddNewTask( self, TASKTYPE_MOVETOLOCATION, Node->position );
					AI_AddNewTask( self, TASKTYPE_ACTIVATESWITCH, pSwitch );
					AI_AddNewTask( self, TASKTYPE_MOVETOLOCATION, FinalNode->position );
					AI_StartMoveToLocation( self );
				}
				else
				{
					AI_Dprintf("Problem with node connections and Door With button... get in touch with Noel\n");
					return FALSE;
				}
				return TRUE;
			}
			else if (AI_IsSidekick(hook))
			{
				if(!Check_Que(self,CANTREACH,1.5f))
				{
					SideKick_TalkAmbient(self, CANTREACH,1);
					SIDEKICK_SendMessage(self, sCom_Speak,NO,gstate->time+1.5f,0.0f,self,COM_Read);
				}				
			}
			//AI_StartMoveToLocation( self );
			
		}
	}
	else// NSS[12/20/99]:Handle regular Doors.
	if ( pCurrentNode->node_type & NODETYPE_DOOR && pNextNode )
	{
		tr = gstate->TraceLine_q2( self->s.origin, pNextNode->position, self, MASK_SOLID );
		if ( tr.fraction < 1.0f )
		{
			userEntity_t *pDoor = com->FindNodeTarget( NODE_GetTarget(pCurrentNode) );
			if( !pDoor )
			{
				AI_Dprintf("%s:Node is door node, but no target! Fix me Kelly!\n",self->className);
				AI_RemoveCurrentTask(self,FALSE);
				return FALSE;
			}
			else if (pDoor && !(pDoor->targetname) && !(pDoor->keyname))
			{
				float Distance = VectorDistance(self->s.origin, pCurrentNode->position);
				if ( !AI_IsDoorOpen( pDoor ) && ( Distance < 48.0f) )
				{
					// open the door
					pDoor->use( pDoor, self, self );
					if(AI_IsSidekick(hook))
					{
						AI_AddNewTaskAtFront(self,TASKTYPE_SIDEKICK_STOP,2.5f);
						AI_ZeroVelocity(self);
					}
					else
					{
						AI_AddNewTaskAtFront(self,TASKTYPE_WAIT,0.5f);
						AI_AddNewTaskAtFront(self,TASKTYPE_MOVETOLOCATION,pNextNode->position);
					}
				}
			}
		}
	}
	else
	{
		if ( pNextNode && (pNextNode->node_type & NODETYPE_DOOR) )
		{
			userEntity_t *pDoor = com->FindNodeTarget( NODE_GetTarget(pNextNode) );
			if(!pDoor)
			{
				AI_Dprintf("%s:Node is door node, but no target! Fix me Kelly!\n",self->className);
				AI_RemoveCurrentTask(self,FALSE);
				return FALSE;
			}
			else if (pDoor && !(pDoor->targetname))
			{
				float Distance = VectorDistance(self->s.origin, pNextNode->position);
				if (!(pDoor->keyname) && !AI_IsDoorOpen( pDoor ) && Distance< 48.0f )
				{
					// open the door
					pDoor->use( pDoor, self, self );
					if(AI_IsSidekick(hook))
					{
						AI_AddNewTaskAtFront(self,TASKTYPE_SIDEKICK_STOP,2.5f);
						AI_ZeroVelocity(self);
					}
					else
					{
						AI_AddNewTaskAtFront(self,TASKTYPE_WAIT,1.0f);
					}
				}
			}
		}
	}
	
	return bRetValue;
}

// ----------------------------------------------------------------------------
// NSS[12/8/99]:
// Name:		AI_IsCloseDistance3
// Description:Special case for jumping
// Input:userEntity_t *self, float fDistance
// Output:NA
// Note:
// ----------------------------------------------------------------------------
int AI_IsCloseDistance3(userEntity_t *self, float fDistance )
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	float fSpeed = AI_ComputeMovingSpeed( hook );

	float fSpeedPerFrame = fSpeed * 0.175f;
	if ( (fDistance - ( (self->s.maxs.x - self->s.mins.x)) ) < fSpeedPerFrame )
	{
		return TRUE;
	}
	return FALSE;

}

// ----------------------------------------------------------------------------
//
// Name:		AI_HandleLadderUse
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_HandleLadderUse( userEntity_t *self, NODEHEADER_PTR pNodeHeader, 
						MAPNODE_PTR pCurrentNode, MAPNODE_PTR pNextNode )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !pNextNode )
	{
		return FALSE;
	}

	int bRetValue = FALSE;

	if ( !(pCurrentNode->node_type & NODETYPE_LADDER) && 
		 (pNextNode->node_type & NODETYPE_LADDER) )
	{
		if ( AI_IsGroundBelowBetween( self, pCurrentNode->position, pNextNode->position ) )
		{
	        float fXYDistance = VectorXYDistance( self->s.origin, pCurrentNode->position );
	        float fZDistance = VectorZDistance( self->s.origin, pCurrentNode->position );
			//if ( AI_IsExactDistance( hook, fXYDistance ) && fZDistance < 32.0f )
			//{
			//	AI_AddNewTaskAtFront( self, TASKTYPE_JUMPTOWARDPOINT, pNextNode->position );
			//	bRetValue = TRUE;
			//}
			//else
			if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
			{
				GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
				if(pGoalStack)
				{
					TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
					if(pCurrentTask)
					{
						if(AI_IsSidekick(hook))
						{
							userEntity_t *pOtherSidekick = SIDEKICK_GetOtherSidekick( self );
							playerHook_t *oHook = AI_GetPlayerHook(pOtherSidekick);
							
							if(pOtherSidekick && AI_IsSidekick(oHook))
							{
								float ODistance = VectorDistance(pNextNode->position,pOtherSidekick->s.origin);
								float MDistance = VectorDistance(pNextNode->position,self->s.origin);
								if(ODistance < MDistance)
								{
									AI_AddNewTaskAtFront(self,TASKTYPE_WAIT,1.0f);
									return TRUE;
								}
							}
						}
						if(pCurrentTask->nTaskType != TASKTYPE_MOVETOEXACTLOCATION)
						{
							AI_AddNewTaskAtFront( self, TASKTYPE_MOVETOEXACTLOCATION, pNextNode->position );
							bRetValue = TRUE;
						}
					}
				}
			}
		}	
	}
	else
	if ( (pCurrentNode->node_type & NODETYPE_LADDER) && 
		 (pNextNode->node_type & NODETYPE_LADDER) )
	{
		float fXYDistance = VectorXYDistance( self->s.origin, pCurrentNode->position );
		float fZDistance = VectorZDistance( self->s.origin, pCurrentNode->position );
		if ( AI_IsExactDistance( hook, fXYDistance ) && fZDistance < 32.0f )
		//if(AI_IsCloseDistance3(self,fXYDistance )&& fZDistance < 32.0f )
		{
			GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
			TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
			_ASSERTE( pCurrentTask );
			AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);			
			
			// NSS[2/15/00]:If we are a sidekick.. let's make sure that we don't hog the ladder.
			if(AI_IsSidekick(hook))
			{
				userEntity_t *pOtherSidekick = SIDEKICK_GetOtherSidekick( self );
				playerHook_t *oHook = AI_GetPlayerHook(pOtherSidekick);
				
				if(pOtherSidekick && AI_IsSidekick(oHook))
				{
					if(SIDEKICK_HasTaskInQue(self,TASKTYPE_UPLADDER) || SIDEKICK_HasTaskInQue(self,TASKTYPE_DOWNLADDER))
					{
						AI_AddNewTaskAtFront(self,TASKTYPE_WAIT,1.0f);
						return TRUE;
					}
				}
			}
			if ( pNextNode->position.z > pCurrentNode->position.z )
			{
				//if(pAIData->destPoint.z >= pNextNode->position.z)
				//{
					AI_AddNewTaskAtFront( self, TASKTYPE_UPLADDER, (void*)pNextNode );
				//}
				//else
				//{
					
				//	return FALSE;
				//}

			}
			else
			{
				AI_AddNewTaskAtFront( self, TASKTYPE_DOWNLADDER, (void*)pNextNode );
			}
			bRetValue = TRUE;
		}
		else
		if ( AI_IsCloseDistance3(self, fXYDistance ) && fZDistance < 32.0f )
		{
			
			// NSS[2/15/00]:If we are a sidekick.. let's make sure that we don't hog the ladder.
			if(AI_IsSidekick(hook))
			{
				userEntity_t *pOtherSidekick = SIDEKICK_GetOtherSidekick( self );
				playerHook_t *oHook = AI_GetPlayerHook(pOtherSidekick);
				
				if(pOtherSidekick && AI_IsSidekick(oHook))
				{
					if(SIDEKICK_HasTaskInQue(self,TASKTYPE_UPLADDER) || SIDEKICK_HasTaskInQue(self,TASKTYPE_DOWNLADDER))
					{
						AI_AddNewTaskAtFront(self,TASKTYPE_WAIT,1.0f);
						return TRUE;
					}
				}
			}
			AI_AddNewTaskAtFront( self, TASKTYPE_MOVETOEXACTLOCATION, pCurrentNode->position );
			bRetValue = TRUE;
		}
		else// NSS[2/7/00]:Check for 'overshoot'
		if(pNextNode->position.z < pCurrentNode->position.z)
		{
			if(self->s.origin.z < pCurrentNode->position.z)
			{
				AI_AddNewTaskAtFront( self, TASKTYPE_DOWNLADDER, (void*)pNextNode );
			}
		}
	}

//
//	int bRetValue = FALSE;
//	if ( AI_IsOnLadder( self, pCurrentNode ) )
//	{
//		bRetValue = TRUE;
//		if ( AI_UseLadder( self, pCurrentNode ) )
//		{
//			// we're already at this node, so delete the first one
//			PATHLIST_DeleteFirstInPath( hook->pPathList );
//		}
//	}
//
	return bRetValue;
}




// ----------------------------------------------------------------------------
//
// Name:		AI_HandleJumping
// Description:
// Input:
// Output:
// Note:
//
// ---------------------------------------------------------------------------
int AI_HandleJumping( userEntity_t *self, NODEHEADER_PTR pNodeHeader, 
					  MAPNODE_PTR pCurrentNode, MAPNODE_PTR pNextNode )
{
	_ASSERTE( self && pNodeHeader && pCurrentNode );
	if ( !self || !pNodeHeader || !pCurrentNode || !pNextNode )
	{
		return FALSE;
	}

	playerHook_t *hook = AI_GetPlayerHook( self );

	int bRetValue = FALSE;
	if ( pCurrentNode && (pNextNode->node_type & NODETYPE_SHOTCYCLERJUMP) && (self->flags & FL_BOT) )
	{
		float fDistance = VectorDistance( self->s.origin, pCurrentNode->position );
		if ( AI_IsCloseDistance2(self, fDistance ) && 
			 !AI_IsGroundBelowBetween( self, pCurrentNode->position, pNextNode->position ) )
		{
			// determine the type of jump needed
			int bOkToShotcyclerJump = FALSE;
			if ( !AI_IsWeaponAvailable( self, "weapon_shotcycler" ) )
			{
				if ( BOT_PickupWeapon( self, "weapon_shotcycler" ) )
				{
					bRetValue = TRUE;
				}
			}
			else
			{
				bOkToShotcyclerJump = TRUE;
			}

			if ( bOkToShotcyclerJump )
			{
				// could jump using a shotcycler
				AI_AddNewTaskAtFront( self, TASKTYPE_SHOTCYCLERJUMP, pNextNode->position );
				bRetValue = TRUE;
			}
		}
	}
	else
	if ( (pCurrentNode->node_type & NODETYPE_JUMPFORWARD) && pNextNode )
	{
		CVector HalfDistance = (pCurrentNode->position + pNextNode->position) * 0.50f;
		// NSS[2/6/00]:Keep the same altitude just half the XY plane
		HalfDistance.z = pNextNode->position.z;
		
		if ( !AI_IsGroundBelowBetween( self, pCurrentNode->position, HalfDistance, 72.0f ) )
		{
			float fDistance = VectorDistance( self->s.origin, pCurrentNode->position );
			//float fSpeed = AI_ComputeMovingSpeed( hook );
			//float fSpeedPerFrame = fSpeed * 0.055f;
			
			if ( fDistance < self->s.maxs.x)
			{
				// make this entity jump forward
				AI_AddNewTaskAtFront( self, TASKTYPE_JUMPTOWARDPOINT, pNextNode->position );
				bRetValue = TRUE;
			}
			else
			if ( AI_IsCloseDistance3(self, fDistance ) )
			{
				AI_AddNewTaskAtFront( self, TASKTYPE_MOVETOEXACTLOCATION, pCurrentNode->position );
				bRetValue = TRUE;
			}
		}
	}
	else
	if ( (pCurrentNode->node_type & NODETYPE_JUMPUP) && pNextNode )
	{
		
		if( (pNextNode->position.z > (pCurrentNode->position.z + 16.0f)) && (self->s.origin.z < pNextNode->position.z))
		{
			AI_AddNewTaskAtFront( self, TASKTYPE_JUMPUP, pNextNode->position );
			bRetValue = TRUE;
		}
	}

	return bRetValue;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_HandlePlatformUse
// Description:
// Input:
// Output:
// Note:
//
// ---------------------------------------------------------------------------
int AI_HandlePlatformUse( userEntity_t *self, NODEHEADER_PTR pNodeHeader, 
						  MAPNODE_PTR pCurrentNode, MAPNODE_PTR pNextNode )
{
	_ASSERTE( self && pNodeHeader && pCurrentNode );
	if ( !self || !pNodeHeader || !pCurrentNode || !pNextNode )
	{
		return FALSE;
	}

	playerHook_t *hook = AI_GetPlayerHook( self );

	int bRetValue = FALSE;
	// NSS[2/14/00]:Added the tail end to this 'logic'  make sure we are not already ON a platform node.
	if ( pNextNode->node_type & NODETYPE_PLATFORM && !(pCurrentNode->node_type & NODETYPE_PLATFORM))
	{
		CVector bottomPoint = pNextNode->position;
		bottomPoint.z -= MAX_JUMP_HEIGHT;
		tr = gstate->TraceLine_q2(pNextNode->position, bottomPoint, NULL, MASK_SOLID);
		if ( !(tr.fraction < 1.0f && tr.ent && _stricmp( tr.ent->className, "func_plat" ) == 0) )
		{
			// platform must be at top, wait for it to come down
			AI_AddNewTaskAtFront( self, TASKTYPE_WAIT, 1.0f );
			bRetValue = TRUE;
		}
	}
	else
	if ( (pCurrentNode->node_type & NODETYPE_PLATFORM) && pNextNode )
	{
		float fNodeZDistance = VectorZDistance( pCurrentNode->position, pNextNode->position );
		if ( fNodeZDistance > MAX_JUMP_HEIGHT )
		{
			CVector bottomPoint = pCurrentNode->position;
			bottomPoint.z -= MAX_JUMP_HEIGHT;

			tr = gstate->TraceLine_q2(pCurrentNode->position, bottomPoint, self, MASK_SOLID);
			if ( tr.fraction < 1.0f && tr.ent && _stricmp( tr.ent->className, "func_plat" ) == 0 )
			{
				float fDistance = VectorDistance( self->s.origin, pCurrentNode->position );
				if ( AI_IsExactDistance( hook, fDistance ) )
				{
					AI_AddNewTaskAtFront( self, TASKTYPE_USEPLATFORM, tr.ent );
					bRetValue = TRUE;
				}
				else
				{
					AI_AddNewTaskAtFront( self, TASKTYPE_MOVETOEXACTLOCATION, pCurrentNode->position );
					bRetValue = TRUE;
				}
			}
			else
			{
				// platform must be at top, wait for it to come down
				AI_AddNewTaskAtFront( self, TASKTYPE_WAIT, 1.0f );

				// find a point to move to and wait
				MAPNODE_PTR pNearNode = NULL;
				for ( int i = 0; i < pCurrentNode->nNumLinks; i++ )
				{
					MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, pCurrentNode->aLinks[i].nIndex );
					float fZDistance = VectorZDistance( self->s.origin, pNode->position );
					if ( fZDistance < 32.0f )
					{
						pNearNode = pNode;
					}
				}
				if(pNearNode)
				{
					AI_AddNewTaskAtFront( self, TASKTYPE_MOVETOLOCATION, pNearNode->position );
					bRetValue = TRUE;
				}

			}
		}
	}

	return bRetValue;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_HandleEmptyPlatform
// Description:
// Input:
// Output:
// Note:
//
// ---------------------------------------------------------------------------
int AI_HandleEmptyPlatform( userEntity_t *self, NODEHEADER_PTR pNodeHeader, 
							MAPNODE_PTR pCurrentNode, MAPNODE_PTR pNextNode )
{
	_ASSERTE( self && pNodeHeader && pCurrentNode );
	if ( !self || !pNodeHeader || !pCurrentNode || !pNextNode )
	{
		return FALSE;
	}

//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	if ( !(pCurrentNode->node_type & NODETYPE_PLATFORM) && 
		 (pNextNode->node_type & NODETYPE_PLATFORM) )
	{
//		float fXYDistance = VectorXYDistance( pCurrentNode->position, pNextNode->position );// SCG[1/23/00]: not used
//		float fZDistance = VectorZDistance( pCurrentNode->position, pNextNode->position );// SCG[1/23/00]: not used
		if ( pNextNode->position.z < pCurrentNode->position.z )
		{
			CVector bottomPoint = pCurrentNode->position;
			bottomPoint.z -= MAX_JUMP_HEIGHT;
			tr = gstate->TraceLine_q2(pCurrentNode->position, bottomPoint, self, MASK_SOLID);
			if ( tr.fraction >= 1.0f )
			{
				// platform must be at top, wait for it to come down
				AI_AddNewTaskAtFront( self, TASKTYPE_JUMPTOWARDPOINT, pNextNode->position );
				return TRUE;
			}
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_HandleEmptyTrain
// Description:
// Input:
// Output:
// Note:
//
// ---------------------------------------------------------------------------
int AI_HandleEmptyTrain( userEntity_t *self, NODEHEADER_PTR pNodeHeader, 
							MAPNODE_PTR pCurrentNode, MAPNODE_PTR pNextNode )
{
	_ASSERTE( self && pNodeHeader && pCurrentNode );
	if ( !self || !pNodeHeader || !pCurrentNode || !pNextNode )
	{
		return FALSE;
	}

//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	if ( (pCurrentNode->node_type & (NODETYPE_TRAIN | NODETYPE_ELEVATOR)) && 
		 (pNextNode->node_type & (NODETYPE_TRAIN | NODETYPE_ELEVATOR)) )
	{
//		float fXYDistance = VectorXYDistance( pCurrentNode->position, pNextNode->position );
//		float fZDistance = VectorZDistance( pCurrentNode->position, pNextNode->position );
		if ( pNextNode->position.z < pCurrentNode->position.z )
		{
			CVector bottomPoint = pCurrentNode->position;
			bottomPoint.z -= MAX_JUMP_HEIGHT;
			tr = gstate->TraceLine_q2(pCurrentNode->position, bottomPoint, self, MASK_SOLID);
			if ( tr.fraction >= 1.0f )
			{
				// platform must be at top, wait for it to come down
				AI_AddNewTaskAtFront( self, TASKTYPE_JUMPTOWARDPOINT, pNextNode->position );
				return TRUE;
			}
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_HandleUse
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_HandleUse( userEntity_t *self, NODEHEADER_PTR pNodeHeader, 
				  MAPNODE_PTR pCurrentNode, MAPNODE_PTR pNextNode )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	//Preset this value to FALSE
	int bRetValue = FALSE;
	
	// NSS[2/12/00]:Flag which allows AI to use these things..
	//hook->dflags  |= DFL_CANUSELADDER|DFL_CANUSEDOOR;
	
	
	// NSS[12/20/99]:How about we don't go into this function unless we need to?
	if ( (pCurrentNode->node_type & NODETYPE_DOOR) || ( pNextNode && (pNextNode->node_type & NODETYPE_DOOR) ) || (pCurrentNode->node_type & NODETYPE_DOORBUTTON) || ( pNextNode && (pNextNode->node_type & NODETYPE_DOORBUTTON) ))
	{
		if(hook->dflags & DFL_CANUSEDOOR)
		{
			bRetValue =  AI_HandleDoorUse( self, pNodeHeader, pCurrentNode, pNextNode );
		}
	}
		
		
	//Below are all WALK type use traps... no flying units will check for these conditions;
	if(self->movetype != MOVETYPE_FLY || self->movetype != MOVETYPE_HOVER)
	{
		if ( bRetValue == FALSE )
		{
			bRetValue = AI_HandleJumping( self, pNodeHeader, pCurrentNode, pNextNode );
		}
		
		if(hook->dflags & DFL_CANUSELADDER)
		{
			if ( bRetValue == FALSE )
			{
				bRetValue = AI_HandlePlatformUse( self, pNodeHeader, pCurrentNode, pNextNode );
			}

			if ( bRetValue == FALSE )
			{
				
				bRetValue = AI_HandleTrainUse( self, pNodeHeader, pCurrentNode, pNextNode );
			}

			if ( bRetValue == FALSE )
			{	
					bRetValue = AI_HandleLadderUse( self, pNodeHeader, pCurrentNode, pNextNode );
			}

			if ( bRetValue == FALSE )
			{
				bRetValue = AI_HandleEmptyPlatform( self, pNodeHeader, pCurrentNode, pNextNode );
			}
		}
	}

    if ( bRetValue == FALSE && AI_IsSidekick( hook ) )
    {
        if ( bRetValue == FALSE && AI_IsSuperfly( hook ) )
        {
            bRetValue = SIDEKICK_HandleSuperflyTalk( self, pCurrentNode );
        }
        if ( bRetValue == FALSE && AI_IsMikiko( hook ) )
        {
            bRetValue = SIDEKICK_HandleMikikoTalk( self, pCurrentNode );
        }
    }

	return bRetValue;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_HandleUse
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_HandleUse( userEntity_t *self )
{
 	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	// NSS[2/12/00]:Checking the pointer... 
	if(!hook)
		return FALSE;
	if ( ( hook->pPathList == NULL ) || ( hook->pPathList->pPath == NULL ))
	{
		return FALSE;
	}

	NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	MAPNODE_PTR pCurrentNode = NULL;
	if ( pNodeList->nCurrentNodeIndex >= 0 )
	{
		pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
	}
	if ( !pCurrentNode )
	{
		pCurrentNode = NODE_GetClosestNode(self);
	}

	if ( !pCurrentNode )
	{
		return FALSE;
	}
	int bRetValue = FALSE;
	float fDistance = (VectorDistance( self->s.origin, pCurrentNode->position )*0.85);
	if ( AI_IsCloseDistance2(self, fDistance ) )
	{
		PATHNODE_PTR pCurrentPathNode = hook->pPathList->pPath;
		_ASSERTE( pCurrentPathNode );
		MAPNODE_PTR pCurrentMapNode = NODE_GetNode( pNodeHeader, pCurrentPathNode->nNodeIndex );
		_ASSERTE( pCurrentMapNode );//<nss> marker

		MAPNODE_PTR pNextNode = NULL;
		if ( pCurrentNode == pCurrentMapNode )
		{
			if ( pCurrentPathNode->next_node )
			{
				pNextNode = NODE_GetNode( pNodeHeader, pCurrentPathNode->next_node->nNodeIndex );
			}
		}
		else
		{
			pNextNode = pCurrentMapNode;
		}

		if ( pNextNode )
		{
			if ( AI_HandleUse( self, pGroundNodes, pCurrentNode, pNextNode ) )
			{
				bRetValue = TRUE;
			}
		}
	}
	else
	{
		PATHNODE_PTR pCurrentPathNode = hook->pPathList->pPath;
		_ASSERTE( pCurrentPathNode );
		MAPNODE_PTR pCurrentMapNode = NODE_GetNode( pNodeHeader, pCurrentPathNode->nNodeIndex );
		_ASSERTE( pCurrentMapNode );//<nss> marker
		MAPNODE_PTR pNextNode = NULL;
		if ( pCurrentNode == pCurrentMapNode )
		{
			if ( pCurrentPathNode->next_node )
			{
				pNextNode = NODE_GetNode( pNodeHeader, pCurrentPathNode->next_node->nNodeIndex );
			}
		}
		else
		{
			pNextNode = pCurrentMapNode;
		}
		if(hook->dflags & DFL_CANUSELADDER)
		{
			if(self->s.origin.z < pCurrentNode->position.z)
			{
				
				if ( pNextNode )
				{
					if ((pCurrentNode->node_type & NODETYPE_LADDER) && (pNextNode->node_type & NODETYPE_LADDER))
					{
						AI_AddNewTaskAtFront( self, TASKTYPE_DOWNLADDER, (void*)pNextNode );
						bRetValue = TRUE;
					}
				}
			}
		}
	}

	return bRetValue;
}

// ----------------------------------------------------------------------------
//
// Name:        AI_ComputeMovementState
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline MOVEMENTSTATE AI_ComputeMovementState( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	if ( !self->groundEntity )
	{
		MAPNODE_PTR pNode = NODE_GetClosestNode(self);
		
		if ( (pNode->node_type & NODETYPE_LADDER) )
		{
			hook->nMovementState = MOVEMENT_LADDER;
		}
		else
		{
			hook->nMovementState = MOVEMENT_AIR;
		}
	}
	else
	{
		userEntity_t *pGroundEntity = self->groundEntity;
		if ( _stricmp( pGroundEntity->className, "func_train" ) == 0 )
		{
			hook->nMovementState = MOVEMENT_TRAIN;
		}
		else
		if ( _stricmp( pGroundEntity->className, "func_plat" ) == 0 )
		{
			hook->nMovementState = MOVEMENT_PLATFORM;
		}
		else
		if ( _stricmp( pGroundEntity->className, "worldspawn" ) == 0 )
		{
			hook->nMovementState = MOVEMENT_GROUND;
		}
		else
		{		
			if ( hook->pPathList->pPath )
			{
				MAPNODE_PTR pCurrentNode = NODE_GetClosestNode(self);
				if ( !pCurrentNode )
				{
					if ( pNodeList->nCurrentNodeIndex != -1 )
					{
						pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
					}
				}
				
				if ( pCurrentNode )
				{
					MAPNODE_PTR pNextNode = NODE_GetNode( pNodeHeader, hook->pPathList->pPath->nNodeIndex );
					if ( pCurrentNode != pNextNode )
					{
						float fXYDistance = VectorXYDistance( self->s.origin, pCurrentNode->position );
						float fZDistance = VectorZDistance( pNextNode->position, pCurrentNode->position );
						if ( fZDistance > 16.0f && fXYDistance < 32.0f && (pCurrentNode->node_type & NODETYPE_LADDER) && 
							 (pNextNode && (pNextNode->node_type & NODETYPE_LADDER)) )
						{
							hook->nMovementState = MOVEMENT_LADDER;
						}
					}
					else
					{
						float fZDistance = VectorZDistance( pCurrentNode->position, self->s.origin );
						if ( (pCurrentNode->node_type & NODETYPE_LADDER) && fZDistance > 16.0f )
						{
							hook->nMovementState = MOVEMENT_LADDER;
						}
					}
				}
			}
		}
	}

	return hook->nMovementState;
}

// ----------------------------------------------------------------------------
//
// Name:        AI_DetermineMovingEnvironment
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_DetermineMovingEnvironment( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	gstate->LinkEntity( self );

	hook->nOriginalMoveType = self->movetype;

// SCG[11/3/99]: why the fuck is this shit here if it does nothing?!?!?!?!
/*
	// get the bot's water level for this frame
	ai_water_level( self );

	if ( self->waterlevel == 3 )
	{
	}
	else
	{
        // this was used for monsters that could both walk on the ground and take off flying (ie: griffon)
        // we'll need to handle this some other way, since monsters could be placed closed to the ground, or
        // in cages/caves that will ALWAYS be flying (ie: deathsphere)
        //

		if ( AI_IsFlyingUnit( self ) )
		{

*/
/*
			CVector end( self->s.origin );
			end.AddZ( -48.0f );

			tr = gstate->TraceLine_q2(self->s.origin, end, self, MASK_SOLID);
			if ( tr.fraction < 1.0f )
			{
				if ( hook->nAttackType == ATTACK_AIR_RANGED )
				{
					hook->nAttackType = ATTACK_GROUND_RANGED;
				}
				else
				if ( hook->nAttackType == ATTACK_AIR_MELEE )
				{
					hook->nAttackType = ATTACK_GROUND_MELEE;
				}

				self->movetype = MOVETYPE_WALK;  // 4.28 dsn  deathspheres turned into walking monsters?
			}
			else
			{
			}
*/
/*
		}
	}
*/

	AI_InitNodeList( self );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_CanClientHear
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *AI_CanClientHear( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *head = alist_FirstEntity( client_list );
	while ( head != NULL )
	{
		// is this a client, bot or monster?
		//NSS[12/1/99]:Made a small change...
		if(head->flags & FL_CLIENT)
		{
			float fDistance = VectorDistance( self->s.origin, head->s.origin );
			if ( fDistance < hook->active_distance )//&& gstate->inPHS( self->s.origin, head->s.origin ) )
			{
				return head;
			}
		}

		head = alist_NextEntity( client_list );
	}

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_AddWhenUsedScriptGoal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_AddWhenUsedScriptGoal( userEntity_t *self, char *szScriptAction, int bUseUniqueId /* = TRUE */ )
{
	_ASSERTE( self );

	CScriptAction *pScriptAction = SCRIPTACTION_Lookup( szScriptAction );
	if ( pScriptAction )
	{
		userEntity_t *pEntity = self;
		
		if ( bUseUniqueId )
		{
			// find the entity that this script belongs to
			if ( strlen( pScriptAction->GetUniqueID() ) > 0 )
			{
				pEntity = UNIQUEID_Lookup( pScriptAction->GetUniqueID() );
				if ( !AI_IsAlive( pEntity ) )
				{
					gstate->Con_Dprintf( "Uniqueid: %s not found\n", pScriptAction->GetUniqueID() );
					return FALSE;
				}
			}
		}
		if ( !pEntity )
		{
			pEntity = self;
		}

		playerHook_t *hook = AI_GetPlayerHook( self );

		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		_ASSERTE( pGoalStack );
		AIDATA aiData;
		aiData.pString = szScriptAction;
		aiData.nValue = pScriptAction->GetLoopCount();
		GOAL_PTR pCurrentGoal = AI_AddNewGoal( pEntity, GOALTYPE_WHENUSED_SCRIPTACTION, &aiData );
		_ASSERTE( pCurrentGoal );

		AI_AddTasksToScriptAction( pEntity, pScriptAction );

		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_WhenUsedByPlayer
// Description:
//				Executes predefined commands when used by the player
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_WhenUsedByPlayer( userEntity_t *self, userEntity_t *other, userEntity_t *activator )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	CWhenUsedCommand *pWhenUsedCommand = hook->pWhenUsedCommand;
	if ( !pWhenUsedCommand )
	{
		return;
	}

    // determine if this object can be 'use'd from the last use 
    if (gstate->time > pWhenUsedCommand->GetUseTime())
    {
      // set next time available for 'use'
      pWhenUsedCommand->SetUseTime(gstate->time + pWhenUsedCommand->GetUseDelay());
    }
    else
      return; // unable to 'use' yet... still delaying. 


	CParameter *pParameter = NULL;
	int nNumActions = pWhenUsedCommand->GetNumActions();
	if ( pWhenUsedCommand->GetNumUsed() >= nNumActions )
	{
		// do idle actions
		CAction *pIdleAction = pWhenUsedCommand->GetIdleAction();
		if ( !pIdleAction )
		{
			return;
		}

		int nNumParameters = pIdleAction->GetNumParameters();
		int nIndex = (int)(rand() % nNumParameters);
		
		pParameter = pIdleAction->GetParameter( nIndex );
	}
	else
	{
		CAction *pCurrentAction = pWhenUsedCommand->GetCurrentAction();
		if ( !pCurrentAction )
		{
			return;
		}

		pWhenUsedCommand->UsedOnce();

		pParameter = pCurrentAction->GetParameter( 0 );
	}
	_ASSERTE( pParameter );
	
	AI_AddWhenUsedScriptGoal( self, pParameter->GetString() );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_DoLevelStartScript
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_DoLevelStartScript()
{
	CScriptAction *pLevelStartScript = ACTION_GetLevelStartScript();
	if ( pLevelStartScript )
	{
		CPtrList *pActionList = pLevelStartScript->GetActionList();
		POSITION pos = pActionList->GetHeadPosition();
		while ( pos )
		{
			CAction *pAction = (CAction*)pActionList->GetNext( pos );
			int nActionType = pAction->GetActionType();
			
			_ASSERTE( nActionType == ACTIONTYPE_SENDMESSAGE );

			// two parameters
			CParameter *pParameter0 = pAction->GetParameter( 0 );
			_ASSERTE( pParameter0 );
			CParameter *pParameter1 = pAction->GetParameter( 1 );
			_ASSERTE( pParameter1 );

			_ASSERTE( strlen( pParameter0->GetString() ) > 0 );
			userEntity_t *pMessageEntity = UNIQUEID_Lookup( pParameter0->GetString() );
			if ( AI_IsAlive( pMessageEntity ) )
			{
				AI_AddScriptActionGoal( pMessageEntity, pParameter1->GetString() );
			}
			else
			{
				gstate->Con_Dprintf( "Uniqueid %s not found\n", pParameter0->GetString() );
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_PlayAttackSounds
// Description:
// Input:
// Output:
// Note: 
// ----------------------------------------------------------------------------
int AI_PlayAttackSounds( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( self->s.frameInfo.frameState & FRSTATE_PLAYSOUND1 )
	{
		self->s.frameInfo.frameState -= FRSTATE_PLAYSOUND1;

		if ( hook->sound1 && !AI_IsSoundDisabled() )
		{
			gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex(hook->sound1), 
                                      1.0f, hook->fMinAttenuation, hook->fMaxAttenuation );
		}
		return 1;
	}
	
	if (self->s.frameInfo.frameState & FRSTATE_PLAYSOUND2)
	{
		self->s.frameInfo.frameState -= FRSTATE_PLAYSOUND2;
		
		if ( hook->sound2 && !AI_IsSoundDisabled() )
		{
			gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex(hook->sound2), 
                                      1.0f, hook->fMinAttenuation, hook->fMaxAttenuation );
		}

		return 2;
	}

	return 0;
}	

// ----------------------------------------------------------------------------
//	following functions are defined as inline in release build
//	the compile time was too long in debug build, so they are defined here
// ----------------------------------------------------------------------------

//#ifdef _DEBUG

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float Random()
{
   	return (float)rand() / (float)RAND_MAX;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_PitchTowardPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_PitchTowardPoint( userEntity_t *self, CVector &destPoint )
{
	_ASSERTE( self );

	CVector dir = destPoint - self->s.origin;
	dir.Normalize();
	CVector newIdealAngle;
	VectorToAngles( dir, newIdealAngle );
	self->ideal_ang.pitch = newIdealAngle.pitch;
    if ( self->ideal_ang.pitch > 45.0f && self->ideal_ang.pitch < 315.0f )
    {
        if ( self->ideal_ang.pitch < 180.0f )
        {
            self->ideal_ang.pitch = 45.0f;
        }
        else
        {
            self->ideal_ang.pitch = 315.0f;
        }
    }

	com->ChangePitch(self);
}

// ----------------------------------------------------------------------------
//
// Name:		AI_OrientTowardPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_OrientTowardPoint( userEntity_t *self, CVector &destPoint )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	CVector dir = destPoint - self->s.origin;
	dir.Normalize();

	CVector newIdealAngle;
	VectorToAngles( dir, newIdealAngle );

	self->ideal_ang = newIdealAngle;
	com->ChangeYaw(self);
	com->ChangePitch(self);
}

// ----------------------------------------------------------------------------

int AI_IsWithinAttackDistance( userEntity_t *self, float fDistance, userEntity_t *enemy )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->fnInAttackRange )
	{
		return hook->fnInAttackRange( self, fDistance, enemy );
	}
	else
	{
		if ( (hook->dflags & DFL_JUMPATTACK) && fDistance < hook->jump_attack_dist &&
			 rnd() <= ( hook->jump_chance * 0.01f ) )
		{
			return TRUE;
		}

		ai_weapon_t	*pWeapon = (ai_weapon_t *) self->curWeapon;
		if ( pWeapon && fDistance < pWeapon->distance )
		{
			return TRUE;
		}

		return FALSE;
	}
}

int AI_IsWithinAttackDistance( userEntity_t *self, userEntity_t *enemy )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	float fDistance = VectorDistance( self->s.origin, enemy->s.origin );

	if ( hook->fnInAttackRange )
	{
		return hook->fnInAttackRange( self, fDistance, enemy );
	}
	else
	{
		if ( (hook->dflags & DFL_JUMPATTACK) && fDistance < hook->jump_attack_dist &&
			 rnd() <= ( hook->jump_chance * 0.01f ) )
		{
			return TRUE;
		}

		ai_weapon_t	*pWeapon = (ai_weapon_t *) self->curWeapon;
		if ( pWeapon && fDistance < pWeapon->distance )
		{
			return TRUE;
		}

		return FALSE;
	}
}

void AI_SetNextPathTime( playerHook_t *hook, float fTime )
{
	_ASSERTE( hook );
	hook->fNextPathTime = gstate->time + fTime;
}

int AI_CanPath( playerHook_t *hook )
{
	_ASSERTE( hook );
	if ( gstate->time > hook->fNextPathTime )
	{
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------

int AI_CanMove( playerHook_t *hook )
{
	_ASSERTE( hook );
	if ( hook->nSpawnValue & SPAWN_DO_NOT_MOVE )
	{
		return FALSE;
	}

	return TRUE;
}

// ----------------------------------------------------------------------------

void AI_SetTaskFinishTime( playerHook_t *hook, float fTime )
{
	_ASSERTE( hook );

	if ( fTime >= 0.0f )
	{
		hook->fTaskFinishTime = gstate->time + fTime;
	}
	else
	{
		hook->fTaskFinishTime = fTime;
	}
}

int AI_IsOverTaskFinishTime( playerHook_t *hook )
{
	_ASSERTE( hook );

	// note: if the value is < 0, then the task has no time limit
	if ( hook->fTaskFinishTime > 0.0f && gstate->time > hook->fTaskFinishTime )
	{
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------

void AI_SetOkToAttackFlag( playerHook_t *hook, int bFlag )
{
	_ASSERTE( hook );
	hook->bOkToAttack = bFlag;
}

int AI_IsOkToAttack( playerHook_t *hook )
{
	_ASSERTE( hook );
	return hook->bOkToAttack;
}

// ----------------------------------------------------------------------------

float AI_ComputeMovingSpeed( playerHook_t *hook )
{
	_ASSERTE( hook );

	float fSpeed = hook->walk_speed;
	if ( hook->pMovingAnimation )
	{
		_ASSERTE( hook->cur_sequence );
        if ( strstr( hook->cur_sequence->animation_name, "run" ) )
		{
			fSpeed = hook->run_speed;
		}
		else
		if ( strstr( hook->cur_sequence->animation_name, "walk" ) )
		{
			fSpeed = hook->walk_speed;
		}
	}
	else
	{
		if ( AI_IsStateSlowWalking( hook ) )
        {
            fSpeed = hook->walk_speed * 0.5f;
        }
        else
        if ( AI_IsStateRunning( hook ) )
		{
			fSpeed = hook->run_speed;
		}
		else 
		if ( AI_IsStateAttacking( hook ) )
		{
			fSpeed = hook->attack_speed;
		}
        else
        if ( AI_IsStateCrouching( hook ) )
        {
            // half of walking speed while crouching
            fSpeed = hook->walk_speed * 0.5f;
        }
	}

	return fSpeed;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsEnemyLookingAtMe
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsEnemyLookingAtMe( userEntity_t *self, userEntity_t *pEnemy )
{
	_ASSERTE( self && pEnemy );
//	playerHook_t *pEnemyHook = AI_GetPlayerHook( pEnemy );// SCG[1/23/00]: not used

	CVector dir = self->s.origin - pEnemy->s.origin;
	dir.Normalize();
	CVector angleTowardSelf;
	VectorToAngles( dir, angleTowardSelf );
	
	float fEnemyTowardSelfX = AngleMod(angleTowardSelf.x);
	float fEnemyTowardSelfY = AngleMod(angleTowardSelf.y);
	float fEnemyFacingAngleX = AngleMod(pEnemy->s.angles[PITCH]);
	float fEnemyFacingAngleY = AngleMod(pEnemy->s.angles[YAW]);

	float fYawDiff = fabs( fEnemyTowardSelfY - fEnemyFacingAngleY );
	float fPitchDiff = fabs( fEnemyTowardSelfX - fEnemyFacingAngleX );
	if ( (fYawDiff < 15.0f || fYawDiff > 345.0f) && (fPitchDiff < 30.0f || fPitchDiff > 330.0f) )
	{
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsFacingEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsFacingEnemy( userEntity_t *self, userEntity_t *pEnemy, float maxyaw, float maxpitch, float maxdist )
{
	_ASSERTE( self );
    _ASSERTE( pEnemy );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	// replace default (invalid) distance
	if ( maxdist == -1.0f )
	{
		ai_weapon_t	*pWeapon = (ai_weapon_t *) self->curWeapon;
		if ( pWeapon )
		{
			maxdist = pWeapon->distance;
		}
		else
		{
			maxdist = 256.0f;
		}
	}

	CVector dir = pEnemy->s.origin - self->s.origin;
	dir.Normalize();
	CVector angleTowardEnemy;
	VectorToAngles( dir, angleTowardEnemy );
	
	float fTowardEnemyY = AngleMod(angleTowardEnemy.y);
	float fFacingAngleY = AngleMod(self->s.angles.yaw);

	float fYawDiff = fabs( fTowardEnemyY - fFacingAngleY );
	if ( fYawDiff < maxyaw || fYawDiff > (360.0f - maxyaw) )
    {
	    float fTowardEnemyX = AngleMod(angleTowardEnemy.x);
	    float fFacingAngleX = AngleMod(self->s.angles.pitch);
	    float fPitchDiff = fabs( fTowardEnemyX - fFacingAngleX );
        if ( fPitchDiff < maxpitch || fPitchDiff > (360.0f - maxpitch) )
	    {
		    return TRUE;
	    }
        else
        {
            float fDistance = VectorDistance( self->s.origin, pEnemy->s.origin );
            if ( fDistance < maxdist )
            {
                return TRUE;
            }
        }
    }

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsFacingTowardPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsFacingTowardPoint( userEntity_t *self, CVector &point )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	CVector dir = point - self->s.origin;
	dir.Normalize();
	CVector angleTowardPoint;
	VectorToAngles( dir, angleTowardPoint );
	
	float fTowardPointY = AngleMod(angleTowardPoint.y);
	float fFacingAngleY = AngleMod(self->s.angles.yaw);

	float fYawDiff = fabs( fTowardPointY - fFacingAngleY );
	if ( fYawDiff < 10.0f || fYawDiff > 350.0f )
    {
        return TRUE;
    }

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsEnemyTargetingMe
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsEnemyTargetingMe( userEntity_t *self, userEntity_t *pEnemy )
{
	_ASSERTE( self && pEnemy );
	if ( pEnemy->flags & FL_CLIENT )
	{
		playerHook_t *pEnemyHook = AI_GetPlayerHook( pEnemy );
		if ( pEnemyHook->autoAim.ent == self )
		{
			return TRUE;	
		}
	}
	else
	{
		return AI_IsFacingEnemy( self, pEnemy );
	}
	return FALSE;
}

// NSS[3/6/00]:create an array of 5 different sight sounds that can play at the same time
// this is 5 sounds of different monster types.
static SIGHT_CACHE S_Cache[5];

// ----------------------------------------------------------------------------
// NSS[3/6/00]:
// Name:		AI_CanPlaySightSound
// Description:Will determine if the current entity can play its sight sound
// Input:userEntity_t *self (the entity wanting to play the sight sound)
// Output:TRUE/FALSE
// Note:
// ----------------------------------------------------------------------------
int AI_CanPlaySightSound(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	int i;
	// NSS[3/6/00]:First check to make sure another type of our same type has not already spoken
	for(i = 0; i < 5; i++)
	{
		if(S_Cache[i].Time < gstate->time && S_Cache[i].type == hook->type)
		{
			S_Cache[i].Time = gstate->time + 4.0f;
			S_Cache[i].type = hook->type;
			return TRUE;
		}
		else if (S_Cache[i].type == hook->type)
		{
			return FALSE;
		}
	}
	// NSS[3/6/00]:If we have made it this far then check to see if we have a valid spot to say something.
	for( i = 0; i < 5; i++)
	{
		if(S_Cache[i].Time < gstate->time)
		{
			// NSS[3/6/00]:
			if(S_Cache[i].type != hook->type)
			{
				S_Cache[i].Time = gstate->time + 4.0f;
				S_Cache[i].type = hook->type;
				return TRUE;
			}
		}
	}
	return FALSE;
}


// ----------------------------------------------------------------------------
//
// Name:		AI_SetEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_SetEnemy( userEntity_t *self, userEntity_t *enemy )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    int bEnemyAlive = AI_IsAlive( enemy );
    userEntity_t *pSelfEnemy = self->enemy;
    if ( self->enemy != enemy )
    {
		// actually set enemy to the entity provided
        self->enemy = enemy;

        if ( bEnemyAlive )
        {
            AI_SetOkToAttackFlag( hook, TRUE );
        }
    }

    if ( AI_IsAlive( self ) && !AI_IsAlive( pSelfEnemy ) && bEnemyAlive && 	!(hook->ai_flags & AI_IGNORE_PLAYER))
    {
    	// make sure this  entity can path to the enemy
		AI_SetNextPathTime( hook, 0.0f );

		AI_SetOkToAttackFlag( hook, TRUE );	
       
		// NSS[3/6/00]:We have been alerted... we can see all hear all... 
		hook->ai_flags |= AI_SEENPLAYER;
		hook->active_distance = 4000.0f;
		self->enemy = enemy;
		self->view_ofs.Set(0.0f, 0.0f, 0.0f);
		
		if(AI_CanPlaySightSound(self))
		{
			// play the sighted sound
      		ai_sight_sound( self );
		}
		//AI_EnemyAlert( self, enemy );
    }
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
void AI_SetMovementState( playerHook_t *hook, MOVEMENTSTATE nNewState )
{
	_ASSERTE( hook );
	hook->nMovementState = nNewState;
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
MOVEMENTSTATE AI_GetMovementState( playerHook_t *hook )
{
	_ASSERTE( hook );
	return hook->nMovementState;
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
void AI_SetCanUseFlag( playerHook_t *hook, int nUseFlag )
{
	_ASSERTE( hook );
	hook->nCanUseFlag |= nUseFlag;	
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
void AI_SetBoundedBox( playerHook_t *hook, const CVector &mins, const CVector &maxs )
{
	_ASSERTE( hook );
	hook->bBounded = TRUE;

	hook->boundedBox.mins = mins;
	hook->boundedBox.maxs = maxs;
}

// ----------------------------------------------------------------------------

#define PITCH_PER_FRAME( e )	(gstate->frametime * (e)->ang_speed.pitch)
#define YAW_PER_FRAME( e )		(gstate->frametime * (e)->ang_speed.yaw)
#define ROLL_PER_FRAME( e )		(gstate->frametime * (e)->ang_speed.roll)

int AI_IsSameAngle2D( userEntity_t *self, CVector &a1, CVector &a2 )
{
	_ASSERTE( self );

	float y = YAW_PER_FRAME(self);
	float yDiff = fabs(a1.y - a2.y);
	if ( (yDiff < y || yDiff > (360.0f-y)) )
	{
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------

int AI_IsSameAngle3D( userEntity_t *self, CVector &a1, CVector &a2 )
{
	_ASSERTE( self );
	float x = PITCH_PER_FRAME(self);
	float y = YAW_PER_FRAME(self);
	float z = ROLL_PER_FRAME(self);
	float xDiff = fabs(a1.x - a2.x);
	float yDiff = fabs(a1.y - a2.y);
	float zDiff = fabs(a1.z - a2.z);
	if ( (xDiff < x || xDiff > (360.0f-x)) &&
		 (yDiff < y || yDiff > (360.0f-y)) &&
		 (zDiff < z || zDiff > (360.0f-z)) )
	{
		return TRUE;
	}

	return FALSE;
}


// ----------------------------------------------------------------------------

int AI_IsGroundUnit( userEntity_t *self )
{
    if ( self->movetype == MOVETYPE_WALK || self->movetype == MOVETYPE_WHEEL ||
         self->movetype == MOVETYPE_HOP )
    {
        return TRUE;
    }

    return FALSE;
}

int AI_IsOnGround( userEntity_t *self )
{
    if ( AI_IsGroundUnit( self ) && self->groundEntity && self->waterlevel < 3 )
	{
        return TRUE;
    }

    return FALSE;
}

int AI_IsFlyingUnit( userEntity_t *self )
{
	_ASSERTE( self );

	if ( self->movetype == MOVETYPE_FLY || self->movetype == MOVETYPE_HOVER || self->movetype == MOVETYPE_BOUNCEMISSILE)
	{
		playerHook_t *hook = AI_GetPlayerHook( self );
		if ( !(hook->nSpawnValue & SPAWN_DO_NOT_FLY) )
		{
			return TRUE;
		}
	}

	return FALSE;
}


// ----------------------------------------------------------------------------

int AI_IsInAir( userEntity_t *self )
{
	_ASSERTE( self );
	if ( (self->movetype == MOVETYPE_FLY || self->movetype == MOVETYPE_HOVER) && 
         self->groundEntity == NULL )
	{
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------

int AI_IsWaterUnit( userEntity_t *self )
{
	_ASSERTE( self );

	if ( self->movetype == MOVETYPE_SWIM )
	{
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------

int AI_IsInWater( userEntity_t *self )
{
	_ASSERTE( self );
	if ( self->movetype == MOVETYPE_SWIM || self->waterlevel == 3 ||
        (self->movetype == MOVETYPE_WALK && !self->groundEntity && self->waterlevel > 1) )
	{
		return TRUE;
    }

	return FALSE;
}

// ----------------------------------------------------------------------------

int AI_IsInsideBoundingBox( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	CVector currentPosition( self->s.origin );

	if ( currentPosition.x >= hook->boundedBox.mins.x && currentPosition.x <= hook->boundedBox.maxs.x &&
		 currentPosition.y >= hook->boundedBox.mins.y && currentPosition.y <= hook->boundedBox.maxs.y &&
		 currentPosition.z >= hook->boundedBox.mins.z && currentPosition.z <= hook->boundedBox.maxs.z )
	{
		return TRUE;
	}

	return FALSE;
}


// ----------------------------------------------------------------------------
//
// Name:		AI_GetClosestNode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

MAPNODE_PTR AI_GetClosestNode( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( self );
	if ( !pNodeHeader )
	{
		return NULL;
	}

    NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );  

	MAPNODE_PTR pCurrentNode = NULL;

	if ( pNodeList )
	{
		if ( pNodeList->nCurrentNodeIndex != -1 )
		{
			pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
		}

		if ( !pCurrentNode )
		{
			pCurrentNode = NODE_GetClosestNode( self );
		}
	}

    return pCurrentNode;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_HandleCrouching
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_HandleCrouching( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	MAPNODE_PTR pCurrentNode = AI_GetClosestNode( self );
    if ( pCurrentNode )
    {
        if ( AI_IsCrouching( hook ) )
        {
            if ( !(pCurrentNode->node_type & NODETYPE_CROUCH) )
            {
                AI_EndCrouching( self );
            }
        }
        else
        {
            if ( pCurrentNode->node_type & NODETYPE_CROUCH )
            {
                AI_StartCrouching( self );
            }
        } 
    }

    return FALSE;

}

// ----------------------------------------------------------------------------

int AI_IsCrouching( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
    
    return AI_IsStateCrouching( hook );
}

// ----------------------------------------------------------------------------

int AI_IsCrouching( playerHook_t *hook )
{
    return AI_IsStateCrouching( hook );
}

// ----------------------------------------------------------------------------

void AI_StartCrouching( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_SetStateCrouching( hook );

    self->s.maxs.z   = 4.0f;
	self->viewheight = -2.0f;

	char szAnimation[16];
    if ( AI_IsStateIdle( hook ) )
    {
		AI_SelectCrouchingAnimation( self, szAnimation );
    }
    else
    {
        // must be moving
		AI_SelectCrouchMovingAnimation( self, szAnimation );
    }

	AI_ForceSequence( self, szAnimation, FRAME_LOOP );
	AI_SetStateWalking(hook);
}

// ----------------------------------------------------------------------------

void AI_EndCrouching( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    AI_ClearStateCrouching( hook );

	self->s.maxs.z   = 32.0f;
	self->viewheight = 22.0f;

	tr = gstate->TraceBox_q2( self->s.origin, self->s.mins, self->s.maxs, self->s.origin, self, MASK_SOLID );
	if ( tr.fraction < 1.0f )
	{
        // can not get up yet
        self->s.maxs.z = 4.0f;
        self->viewheight = -2.0f;
    }
    else
    {
	    char szAnimation[16];
        if ( AI_IsStateIdle( hook ) )
        {
		    AI_SelectAmbientAnimation( self, szAnimation );
        }
        else
        if ( AI_IsStateWalking( hook ) || AI_IsStateSlowWalking( hook ) )
        {
            AI_SelectWalkingAnimation( self, szAnimation );
        }
        else
        if ( AI_IsStateRunning( hook ) )
        {
		    AI_SelectRunningAnimation( self, szAnimation );
        }

        AI_ForceSequence( self, szAnimation, FRAME_LOOP );
    }
}


// ----------------------------------------------------------------------------
//
// Name:		AI_IsCloseDistance
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsCloseDistance( playerHook_t *hook, float fDistance )
{
	_ASSERTE( hook );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fSpeedPerFrame = fSpeed * 0.125f;
	if ( fDistance  < fSpeedPerFrame )
	{
		return TRUE;
	}
	return FALSE;
}


// ----------------------------------------------------------------------------
// NSS[12/8/99]:
// Name:		AI_IsCloseDistance2
// Description:A little more accurate distance checker.
// Input:userEntity_t *self, float fDistance
// Output:NA
// Note:
// ----------------------------------------------------------------------------
int AI_IsCloseDistance2(userEntity_t *self, float fDistance )
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	float fSpeed = AI_ComputeMovingSpeed( hook );

	if(hook->nAttackMode == ATTACKMODE_TAKECOVER)
	{
		if(fDistance <= 20.0f)
		{
			return TRUE;
		}
	}
	else
	{
		float Mul;
		if(fSpeed > 175)
			Mul = 0.1;
		else
			Mul = 0.2;
		float fSpeedPerFrame = fSpeed * Mul;
		if ( fDistance < fSpeedPerFrame)
		{
			return TRUE;
		}
	}
	return FALSE;

}





// ----------------------------------------------------------------------------
//NSS[12/1/99]:
// Name:		AI_IsSideKickCloseToItem
// Description:New function only for sidekicks to get things.
// Input:playerHook_t *hook, float fDistance 
// Output:NA
// Note:
// ----------------------------------------------------------------------------
int AI_IsSideKickCloseToItem( playerHook_t *hook, float fDistance )
{
	_ASSERTE( hook );
	float fSpeed = AI_ComputeMovingSpeed( hook );


	float fSpeedPerFrame = fSpeed * 0.40f;
	if ( fDistance < fSpeedPerFrame )
	{
		return TRUE;
	}

	return FALSE;
}


// ----------------------------------------------------------------------------
//
// Name:		AI_IsExactDistance
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsExactDistance( playerHook_t *hook, float fDistance )
{
	_ASSERTE( hook );
	float fSpeed = AI_ComputeMovingSpeed( hook );

	float fSpeedPerFrame = fSpeed * 0.015f;
	if ( (int)fDistance <= (int)fSpeedPerFrame )
	{
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------

int AI_IsOnFlatSurface( userEntity_t *self )
{
	_ASSERTE( self );
	CVector angleVector( 0, self->s.angles.yaw, 0 );

	CVector facingDir;
	AngleToVectors( angleVector, facingDir );
	facingDir.Normalize();

	CVector forwardPoint;
	VectorMA( self->s.origin, facingDir, 48.0f, forwardPoint );

	CVector end( forwardPoint.x, forwardPoint.y, forwardPoint.z );
	//CVector end( forwardPoint.x, forwardPoint.y, forwardPoint.z - 32.0f );

	tr = gstate->TraceLine_q2( self->s.origin, end, self, MASK_SOLID );
	if ( tr.fraction < 1.0 && tr.plane.normal.z >= 0.9999f )
	{
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------


int AI_IsPathToEntityClose( userEntity_t *self, userEntity_t *destEntity )
{
	_ASSERTE( self && destEntity );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !hook->pPathList->pPath )
	{
		return FALSE;
	}

	NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	// test to see if the first node is visible 
    MAPNODE_PTR pFirstNode = NODE_GetNode( pNodeHeader, hook->pPathList->pPath->nNodeIndex );
	if ( !pFirstNode )
    {
        return FALSE;
    }

	if ( gstate->inPVS(self->s.origin, pFirstNode->position) )
	{
		tr = gstate->TraceLine_q2( self->s.origin, pFirstNode->position, self, MASK_SOLID );
		if ( tr.fraction < 1.0f )
		{
			return FALSE;
		}
	}
    else
    {
        return FALSE;
    }

    if ( hook->pPathList->pPathEnd && (hook->nMoveCounter % 5) == 0 )
	{
		MAPNODE_PTR pLastNode = NODE_GetNode( pNodeHeader, hook->pPathList->pPathEnd->nNodeIndex );
		if ( pLastNode )
		{
			float fDistance = VectorDistance( destEntity->s.origin, pLastNode->position );
			//????????
			//Hard code distance checking?  What about other monsters?
			if ( fDistance > 768.0f )
			{
				return FALSE;
			}
		}
		else
		{
			AI_Dprintf("LastNode Undefined!--->AI_IsPathToEntityClose\n");
			return FALSE;
		}
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsReadyToAttack1
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsReadyToAttack1( userEntity_t *self )
{
	_ASSERTE( self );
	if ( self->s.frameInfo.frameState & FRSTATE_PLAYATTACK1 )
	{
		self->s.frameInfo.frameState -= FRSTATE_PLAYATTACK1;
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsReadyToAttack2
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsReadyToAttack2( userEntity_t *self )
{
	_ASSERTE( self );
	if ( self->s.frameInfo.frameState & FRSTATE_PLAYATTACK2 )
	{
		self->s.frameInfo.frameState -= FRSTATE_PLAYATTACK2;
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsEndAnimation
// Description:
// Input:
// Output:
// Note:Only call this function once per attack function cycle
//
// ----------------------------------------------------------------------------
int AI_IsEndAnimation( userEntity_t *self )
{
	_ASSERTE( self );
	if ( self->s.frameInfo.frameState & FRSTATE_LAST )
	{
		self->s.frameInfo.frameState -= FRSTATE_LAST;
		return TRUE;
	}

	if ( self->s.frameInfo.frameState & FRSTATE_STOPPED )
	{
		self->s.frameInfo.frameState -= FRSTATE_STOPPED;
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsEnemyDead
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_IsEnemyDead( userEntity_t *self )
{
	_ASSERTE( self );

	if ( !AI_IsAlive( self->enemy ) )
	{
		playerHook_t *hook = AI_GetPlayerHook( self );

		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		if ( pCurrentGoal )
		{
			GOAL_Satisfied( pCurrentGoal );
			AI_RemoveCurrentGoal( self );
		}

		AI_KilledEnemy( self );
		
		return TRUE;
	}

	return FALSE;
}




//#endif _DEBUG

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
void world_ai_func_register_func()
{
	gstate->RegisterFunc("AI_ParseEpairs",AI_ParseEpairs);
	gstate->RegisterFunc("AI_WhenUsedByPlayer",AI_WhenUsedByPlayer);
	gstate->RegisterFunc("AI_StartBodyPain",AI_StartBodyPain);
	gstate->RegisterFunc("AI_StartDie",AI_StartDie);
	gstate->RegisterFunc("AI_Pain",AI_Pain);
	gstate->RegisterFunc("AI_Die",AI_Die);
	gstate->RegisterFunc("AI_StartIdle",AI_StartIdle);
	gstate->RegisterFunc("AI_Idle",AI_Idle);
	gstate->RegisterFunc("AI_JumpUp",AI_JumpUp);
	gstate->RegisterFunc("AI_StartJumpUp",AI_StartJumpUp);
	gstate->RegisterFunc("AI_JumpForward",AI_JumpForward);
	gstate->RegisterFunc("AI_StartJumpForward",AI_StartJumpForward);
	gstate->RegisterFunc("AI_StartAttack",AI_StartAttack);
	gstate->RegisterFunc("AI_Attack",AI_Attack);
	gstate->RegisterFunc("AI_StartChase",AI_StartChase);
	gstate->RegisterFunc("AI_Chase",AI_Chase);
	gstate->RegisterFunc("AI_Freeze",AI_Freeze);
	gstate->RegisterFunc("AI_StartPatrol",AI_StartPatrol);
	gstate->RegisterFunc("AI_Patrol",AI_Patrol);
	gstate->RegisterFunc("AI_StartWander",AI_StartWander);
	gstate->RegisterFunc("AI_Wander",AI_Wander);
	gstate->RegisterFunc("AI_StartMoveToLocation",AI_StartMoveToLocation);
	gstate->RegisterFunc("AI_MoveToLocation",AI_MoveToLocation);
	gstate->RegisterFunc("AI_StartGoingAroundObstacle",AI_StartGoingAroundObstacle);
	gstate->RegisterFunc("AI_GoingAroundObstacle",AI_GoingAroundObstacle);
	gstate->RegisterFunc("AI_StartFlyToLocation",AI_StartFlyToLocation);
	gstate->RegisterFunc("AI_FlyToLocation",AI_FlyToLocation);
	gstate->RegisterFunc("AI_StartSwoopAttack",AI_StartSwoopAttack);
	gstate->RegisterFunc("AI_SwoopAttack",AI_SwoopAttack);
	gstate->RegisterFunc("AI_StartTowardIntermediatePoint",AI_StartTowardIntermediatePoint);
	gstate->RegisterFunc("AI_TowardIntermediatePoint",AI_TowardIntermediatePoint);
	gstate->RegisterFunc("AI_StartAirRangedAttack",AI_StartAirRangedAttack);
	gstate->RegisterFunc("AI_AirRangedAttack",AI_AirRangedAttack);
	gstate->RegisterFunc("AI_StartAirMeleeAttack",AI_StartAirMeleeAttack);
	gstate->RegisterFunc("AI_AirMeleeAttack",AI_AirMeleeAttack);
	gstate->RegisterFunc("AI_StartSwimToLocation",AI_StartSwimToLocation);
	gstate->RegisterFunc("AI_SwimToLocation",AI_SwimToLocation);
	gstate->RegisterFunc("AI_StartFaceAngle",AI_StartFaceAngle);
	gstate->RegisterFunc("AI_FaceAngle",AI_FaceAngle);
	gstate->RegisterFunc("AI_StartWait",AI_StartWait);
	gstate->RegisterFunc("AI_Wait",AI_Wait);
	gstate->RegisterFunc("AI_StartBackupAllAttributes",AI_StartBackupAllAttributes);
	gstate->RegisterFunc("AI_StartRestoreAllAttributes",AI_StartRestoreAllAttributes);
	gstate->RegisterFunc("AI_StartModifyTurnAttribute",AI_StartModifyTurnAttribute);
	gstate->RegisterFunc("AI_StartModifyRunAttribute",AI_StartModifyRunAttribute);
	gstate->RegisterFunc("AI_StartPlayAnimation",AI_StartPlayAnimation);
	gstate->RegisterFunc("AI_StartSetIdleAnimation",AI_StartSetIdleAnimation);
	gstate->RegisterFunc("AI_StartUsingWalkSpeed",AI_StartUsingWalkSpeed);
	gstate->RegisterFunc("AI_StartUsingRunSpeed",AI_StartUsingRunSpeed);
	gstate->RegisterFunc("AI_StartActivateSwitch",AI_StartActivateSwitch);
	gstate->RegisterFunc("AI_StartWaitForTrainToCome",AI_StartWaitForTrainToCome);
	gstate->RegisterFunc("AI_WaitForTrainToCome",AI_WaitForTrainToCome);
	gstate->RegisterFunc("AI_StartWaitForTrainToStop",AI_StartWaitForTrainToStop);
	gstate->RegisterFunc("AI_WaitForTrainToStop",AI_WaitForTrainToStop);
	gstate->RegisterFunc("AI_StartMoveToEntity",AI_StartMoveToEntity);
	gstate->RegisterFunc("AI_MoveToEntity",AI_MoveToEntity);
	gstate->RegisterFunc("AI_StartTakeCover",AI_StartTakeCover);
	gstate->RegisterFunc("AI_TakeCover",AI_TakeCover);
	gstate->RegisterFunc("AI_StartGoingAroundEntity",AI_StartGoingAroundEntity);
	gstate->RegisterFunc("AI_GoingAroundEntity",AI_GoingAroundEntity);
	gstate->RegisterFunc("AI_RemoveIdleScriptGoal",AI_RemoveIdleScriptGoal);
	gstate->RegisterFunc("AI_FollowWalking",AI_FollowWalking);
	gstate->RegisterFunc("AI_FollowRunning",AI_FollowRunning);
	gstate->RegisterFunc("AI_FollowSidekickWalking",AI_FollowSidekickWalking);
	gstate->RegisterFunc("AI_FollowSidekickRunning",AI_FollowSidekickRunning);
	gstate->RegisterFunc("AI_StartPain",AI_StartPain);
	gstate->RegisterFunc("AI_FindNearClientOrSidekick",AI_FindNearClientOrSidekick);
	gstate->RegisterFunc("AI_ThinkFadeFast",AI_ThinkFadeFast);
	gstate->RegisterFunc("AI_ThinkFade",AI_ThinkFade);
	gstate->RegisterFunc("AI_DeadPain",AI_DeadPain);
}


