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
#include "nodelist.h"
//#include "ai_move.h"	// SCG[1/23/00]: not used
#include "ai_weapons.h"
#include "ai_frames.h"
#include "ai.h"
#include "actorlist.h"
#include "ai_func.h"
#include "MonsterSound.h"
#include "ai_utils.h"

/* ***************************** define types ****************************** */

#define BACKAWAY_DISTANCE	72.0f
#define HEIGHT_FROM_PLAYER	72.0f

#define MIN_DISTANCE_AWAY	72.0f
#define MAX_DISTANCE_AWAY	192.0f


#define STRAIGHT		0
#define LEFT			1
#define RIGHT			2

/* ***************************** Local Variables *************************** */
static int nMovingDirection = STRAIGHT;
static int nAlarmSound;
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
extern float sin_tbl[];
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_cambot (userEntity_t *self);
int AI_HasTaskInQue( userEntity_t *self, TASKTYPE SearchTask );
void CAMBOT_SoundAlarm( userEntity_t *self );
void CAMBOT_AlertNearbyMonsters( userEntity_t *self, userEntity_t *pEnemy );
int AI_HasGoalInQueue( userEntity_t *self, GOALTYPE SearchGoal );
// ----------------------------------------------------------------------------
//
// Name:		deathsphere_moveup
// Description:Function which prevents the deathsphere from slamming against the ceiling.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------

void cambot_moveup(userEntity_t *self)
{
		CVector Axis;
		Axis.Set(0.0f,0.0f,1.0f);

		Axis = self->s.origin + (Axis * (96.0f + 128.0f*rnd()));

		MAPNODE_PTR Node = 	NODE_GetClosestNode( self, Axis);
		if(Node)
		{
			AI_AddNewTaskAtFront( self, TASKTYPE_MOVETOLOCATION, Node->position );
		}
}


// ----------------------------------------------------------------------------
//
// Name:		deathsphere_movedown
// Description:Function which prevents the deathsphere from slamming against the ceiling.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------

void cambot_movedown(userEntity_t *self)
{
		CVector Axis,Target_Angles,flyAwayPoint;

		//Initialize the flyaway point to be ourself in case we don't find one
		flyAwayPoint = self->s.origin;
	

		Target_Angles	= self->s.angles;
		Target_Angles.x = 0.0f;
		Target_Angles.y = 0.0f;
		Target_Angles.z	= -90.0f;

		Target_Angles.AngleToForwardVector(Axis);
	
		Axis = self->s.origin + (Axis * (96.0f + 128.0f*rnd()));

		MAPNODE_PTR Node = 	NODE_GetClosestNode( self, Axis);
		if(Node)
		{
			AI_AddNewTaskAtFront( self, TASKTYPE_MOVETOLOCATION, Node->position );
		}
}


// ----------------------------------------------------------------------------
//
// Name:		CAMBOT_IsFacingEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CAMBOT_Think(userEntity_t *self)
{
	
	playerHook_t *hook = AI_GetPlayerHook( self );

	if(hook)
	{
		if(self->enemy == NULL && hook->owner && AI_IsAlive(hook->owner))
		{
			self->enemy = hook->owner;
		}

		//Get the goal pointers and task pointers
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		GOAL_PTR pGoal = pGoalStack->pTopOfStack;
		//NSS[11/16/99]:If we have seend an emeny but gone back into IDLE lets go into WANDER now
		if(!pGoal || pGoal->nGoalType == GOALTYPE_IDLE && hook->base_vita && self->enemy && !AI_IsAlive(self->enemy))
		{
			AI_AddNewGoal(self,GOALTYPE_WANDER);
			self->nextthink = gstate->time + 0.1f;
			return;
		}
		
		if(self->enemy && hook->base_vita==0)
			hook->base_vita = 1;
	}
	
	AI_TaskThink( self);
	
	if(AI_Determine_Room_Height( self, 300.0f, 2) < 32.0f) //Check distance to floor
	{
		cambot_movedown( self );
	}
	else if(AI_Determine_Room_Height( self, 300.0f, 1) < 32.0f)//Check Distance to Ceiling
	{
		cambot_moveup( self );
	}

	if(hook)
	{
		//Add a sin wave to its flight	
		if(hook->acro_boost >= 12)
		{	
			hook->acro_boost = 0;
		}
		self->velocity.z += 15*(sin_tbl[(int)hook->acro_boost]);
		hook->acro_boost += 1;
		//The hover sound
		if(hook->acro_boost == 1)
		{
			if(self->deadflag != DEAD_DEAD||self->deadflag != DEAD_DYING)
			{
				if(hook->stateFlags & SFL_WALKING)
				{
					gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/e_roomtoned.wav"),0.65f, ATTN_NORM_MIN, ATTN_NORM_MAX );
				}
				else
				{
					gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/e_roomtonee.wav"),0.75f, ATTN_NORM_MIN, ATTN_NORM_MAX );
				}
			}
			else
			{
				gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/e_roomtonee.wav"),0.0f, 0, 0 );
			}
		}
		//The gibberish sounds
		if( (hook->acro_boost==5))
		{
			float Limit = 0.65f;
			if(hook->stateFlags & SFL_RUNNING)
			{
				Limit = 0.30f;
			}
			if(rnd() > Limit)
			{
				char off = ((int) 9.0f * rnd()) + 97;
				char sound[64];
				Com_sprintf(sound,sizeof(sound),"global/e_cntrltone%c.wav",off);
				AI_Dprintf("Sound = %s\n",sound);
				gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex(sound),0.65f, ATTN_NORM_MIN, ATTN_NORM_MAX );
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		CAMBOT_IsFacingEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int CAMBOT_IsFacingTarget( userEntity_t *self, userEntity_t *pTarget )
{
	if(pTarget)
	{
		CVector dir = pTarget->s.origin - self->s.origin;
		dir.Normalize();
		CVector angleTowardTarget;
		VectorToAngles( dir, angleTowardTarget );
		
		//float fTowardTargetX = AngleMod(angleTowardTarget.x);
		float fTowardTargetY = AngleMod(angleTowardTarget.y);
		//float fFacingAngleX = AngleMod(self->s.angles.pitch);
		float fFacingAngleY = AngleMod(self->s.angles.yaw);

		float fYawDiff = fabs( fTowardTargetY - fFacingAngleY );
		//float fPitchDiff = fabs( fTowardTargetX - fFacingAngleX );
		if ( (fYawDiff < 75.0f || fYawDiff > 285.0f) )//&& (fPitchDiff < 45.0f || fPitchDiff > 315.0f) )
		{
			return TRUE;
		}
	}
	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		CAMBOT_IsVisible
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int CAMBOT_IsVisible( userEntity_t *self, userEntity_t *targ )
{
	_ASSERTE( self && targ );

	float fSelfHalfHeight = (self->s.maxs.z - self->s.mins.z) * 0.6;
	float fTargetHalfHeight = (targ->s.maxs.z - targ->s.mins.z) * 0.6;

	
	// NSS[2/17/00]:If the enemy has wraith orb and we have not seen him then let's
	//early out.
	playerHook_t *phook = AI_GetPlayerHook( targ );
	playerHook_t *hook	= AI_GetPlayerHook( self );

	if(phook && hook)
	{
		if((phook->items & IT_WRAITHORB) && !(hook->ai_flags&AI_SEENPLAYER))
			return	FALSE;
	}
	
	CVector org( self->s.origin.x, self->s.origin.y, self->s.origin.z + fSelfHalfHeight );
	CVector end( targ->s.origin.x, targ->s.origin.y, targ->s.origin.z + fTargetHalfHeight );

	//	use Potentially Visible Set to quickly exclude some entities
	if ( Node_IsWithinRange(org, end, 0, 0) )
	{
		tr = gstate->TraceLine_q2( org, end, self, MASK_SOLID );
		if ( tr.fraction >= 1.0f && CAMBOT_IsFacingTarget( self, targ ) )
		{
			float fHalfWidth = (self->s.maxs.x - self->s.mins.x) * 0.6f;
			
			// do two traces from both sides to make sure that the hide node is a good hiding place
			CVector directionVector;
			directionVector.x = targ->s.origin.x - self->s.origin.x;
			directionVector.y = targ->s.origin.y - self->s.origin.y;
			directionVector.z = 0.0f;
			directionVector.Normalize();
			
			// left side
			CVector leftVector( -directionVector.y, directionVector.x, 0.0f );

			// right side
			CVector rightVector( directionVector.y, -directionVector.x, 0.0f );

			CVector leftPos;
			VectorMA( org, leftVector, fHalfWidth, leftPos );

			CVector rightPos;
			VectorMA( org, rightVector, fHalfWidth, rightPos );

			trace_t leftTrace = gstate->TraceLine_q2( leftPos, end, self, MASK_SOLID );
			if ( leftTrace.fraction >= 1.0f )
			{
				trace_t rightTrace = gstate->TraceLine_q2( rightPos, end, self, MASK_SOLID );
				if ( rightTrace.fraction >= 1.0f )
				{
					return TRUE;
				}
			}
		}
	}

	return	FALSE;
}

// ----------------------------------------------------------------------------
// NSS[2/17/00]:
// Name:		CAMBOT_FoundPlayer
// Description:Cambot sets itself up to deal with the player and also alerts 
// other monsters.
// Input:userEntity_t *self, userEntity_t *pPlayer 
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void CAMBOT_FoundPlayer( userEntity_t *self, userEntity_t *pPlayer )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	if(!AI_HasGoalInQueue(self,GOALTYPE_CAMBOT_FOLLOWPLAYER))
	{
		if(hook)
		{
			hook->ai_flags  |= AI_SEENPLAYER;
		}
		else
		{
			return;
		}

		// a new enemy found !!!
		AI_SetEnemy( self, pPlayer );
		AI_EnemyAlert( self, pPlayer );
		self->enemy = pPlayer;
		hook->owner = pPlayer;

		// sound alarm and place a spot light on enemy and alert all AI's within a 1024 radius
		CAMBOT_SoundAlarm( self );
		//CAMBOT_AlertNearbyMonsters( self, pPlayer );
		AI_AddNewGoal( self, GOALTYPE_CAMBOT_FOLLOWPLAYER, pPlayer );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		CAMBOT_FindNearPlayer
//
// Description:	finds the first client or bot within hook->active_dist units
//				of self.  Only entities with an opposing alignment are 
//				considered.
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *CAMBOT_FindNearPlayer( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *head = alist_FirstEntity( client_list );
	
	
	self->think				= CAMBOT_Think;
	if(hook->ai_flags & AI_IGNORE_PLAYER)
		return NULL;

	while ( head != NULL )
	{
		if ( AI_IsAlive( head ) && head->flags & (FL_CLIENT | FL_BOT) )
		{
			float fDistance = VectorDistance( self->s.origin, head->s.origin );
			if ( fDistance < hook->active_distance && CAMBOT_IsVisible(self, head) )
			{
		        if ( head != self->enemy && head->deadflag == DEAD_NO)
		        {
						//Found a player go red
						hook->active_distance	=	5000.0f;
						hook->ai_flags			|=	AI_SEENPLAYER;
						hook->power_boost		=	1;
						CAMBOT_FoundPlayer( self, head );
		        }

				return head;
			}
		}

		head = alist_NextEntity( client_list );
	}
	//Didn't find a player go yellow
	hook->power_boost = 0;
	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		CAMBOT_IsPathToEntityClose
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline int CAMBOT_IsPathToEntityClose( userEntity_t *self, userEntity_t *destEntity )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if(!destEntity)
		return FALSE;
	if(hook && hook->pPathList)
	{
		if ( !hook->pPathList->pPath && !AI_IsVisible(self,destEntity))
		{
			AI_FindPathToEntity(self,destEntity,FALSE);
			if(!hook->pPathList || !hook->pPathList->pPath)
				return FALSE;
		}

		if ( hook->pPathList->pPathEnd && (hook->nMoveCounter % 5) == 0 )
		{
			NODELIST_PTR pNodeList = hook->pNodeList;
			_ASSERTE( pNodeList );
			NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
			_ASSERTE( pNodeHeader );
			
			MAPNODE_PTR pLastNode = NODE_GetNode( pNodeHeader, hook->pPathList->pPathEnd->nNodeIndex );

			float fDistance = VectorXYDistance( destEntity->s.origin, pLastNode->position );
			if ( fDistance > 1024.0f )
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		CAMBOT_FindBackAwayPointFromEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static 
int CAMBOT_FindBackAwayPointFromEnemy( userEntity_t *self, userEntity_t *enemy, CVector &backAwayPoint )
{
	CVector mins( -8, -8, -8 );
	CVector maxs( 8, 8, 8 );

	// search backwards, then left and right directions for a clear path
	CVector direction = self->s.origin - enemy->s.origin;
	direction.z = 0.0f;
	float fZDistance = VectorZDistance( self->s.origin, enemy->s.origin );
	direction.Normalize();
	if ( fZDistance < HEIGHT_FROM_PLAYER )
	{
		direction.z = 0.2f;
	}
	direction.Normalize();

	backAwayPoint.x = self->s.origin.x + direction.x * BACKAWAY_DISTANCE;
	backAwayPoint.y = self->s.origin.y + direction.y * BACKAWAY_DISTANCE;
	backAwayPoint.z = self->s.origin.z + direction.z * BACKAWAY_DISTANCE;
	tr = gstate->TraceBox_q2( self->s.origin, mins, maxs, backAwayPoint, self, MASK_SOLID );
	if (tr.fraction >= 1.0)
	{
		nMovingDirection = STRAIGHT;
		return TRUE;
	}
	else
	{
		int nPrevMovingDirection = nMovingDirection;

		playerHook_t *hook = AI_GetPlayerHook( self );
		float fSpeed = AI_ComputeMovingSpeed( hook ) * 0.15f;

		CVector rightVector = direction;
		CVector leftVector = direction;

		RotateVector2D( leftVector, 300.0f );
		CVector leftBackAwayPoint;
		leftBackAwayPoint.x = self->s.origin.x + leftVector.x * 1024.0f;
		leftBackAwayPoint.y = self->s.origin.y + leftVector.y * 1024.0f;
		leftBackAwayPoint.z = self->s.origin.z + leftVector.z * 1024.0f;
		trace_t leftTrace = gstate->TraceBox_q2( self->s.origin, mins, maxs, leftBackAwayPoint, self, MASK_SOLID );
		float fLeftClearDistance = leftTrace.fraction * 1024.0f;

		RotateVector2D( rightVector, 60.0f );
		CVector rightBackAwayPoint;
		rightBackAwayPoint.x = self->s.origin.x + rightVector.x * 1024.0f;
		rightBackAwayPoint.y = self->s.origin.y + rightVector.y * 1024.0f;
		rightBackAwayPoint.z = self->s.origin.z + rightVector.z * 1024.0f;
		trace_t rightTrace = gstate->TraceBox_q2( self->s.origin, mins, maxs, rightBackAwayPoint, self, MASK_SOLID );
		float fRightClearDistance = rightTrace.fraction * 1024.0f;
		
		int bFound = FALSE;
		if ( fLeftClearDistance >= BACKAWAY_DISTANCE && fRightClearDistance >= BACKAWAY_DISTANCE )
		{
			bFound = TRUE;

			if ( fLeftClearDistance >= fRightClearDistance )
			{
				backAwayPoint.x = self->s.origin.x + (leftVector.x * fSpeed);
				backAwayPoint.y = self->s.origin.y + (leftVector.y * fSpeed);
				backAwayPoint.z = self->s.origin.z + (leftVector.z * fSpeed);
				nMovingDirection = LEFT;
			}
			else
			{
				backAwayPoint.x = self->s.origin.x + (rightVector.x * fSpeed);
				backAwayPoint.y = self->s.origin.y + (rightVector.y * fSpeed);
				backAwayPoint.z = self->s.origin.z + (rightVector.z * fSpeed);
				nMovingDirection = RIGHT;
			}
		}
		else
		{
			if ( fLeftClearDistance >= BACKAWAY_DISTANCE )
			{
				bFound = TRUE;
				backAwayPoint.x = self->s.origin.x + (leftVector.x * fSpeed);
				backAwayPoint.y = self->s.origin.y + (leftVector.y * fSpeed);
				backAwayPoint.z = self->s.origin.z + (leftVector.z * fSpeed);

				nMovingDirection = LEFT;
			}
			else
			if ( fRightClearDistance >= BACKAWAY_DISTANCE )
			{
				bFound = TRUE;
				backAwayPoint.x = self->s.origin.x + (rightVector.x * fSpeed);
				backAwayPoint.y = self->s.origin.y + (rightVector.y * fSpeed);
				backAwayPoint.z = self->s.origin.z + (rightVector.z * fSpeed);

				nMovingDirection = RIGHT;
			}
		}

		if ( !bFound )
		{
			CVector leftAddedVector, rightAddedVector;

			leftAddedVector = leftVector + leftTrace.plane.normal;
			leftAddedVector.Normalize();
			leftBackAwayPoint.x = self->s.origin.x + leftAddedVector.x * 1024.0f;
			leftBackAwayPoint.y = self->s.origin.y + leftAddedVector.y * 1024.0f;
			leftBackAwayPoint.z = self->s.origin.z + leftAddedVector.z * 1024.0f;
			leftTrace = gstate->TraceBox_q2( self->s.origin, mins, maxs, leftBackAwayPoint, self, MASK_SOLID );
			float fLeftClearDistance = leftTrace.fraction * 1024.0f;

			rightAddedVector = rightVector + rightTrace.plane.normal;
			rightAddedVector.Normalize();
			rightBackAwayPoint.x = self->s.origin.x + rightAddedVector.x * 1024.0f;
			rightBackAwayPoint.y = self->s.origin.y + rightAddedVector.y * 1024.0f;
			rightBackAwayPoint.z = self->s.origin.z + rightAddedVector.z * 1024.0f;
			rightTrace = gstate->TraceBox_q2( self->s.origin, mins, maxs, rightBackAwayPoint, self, MASK_SOLID );
			float fRightClearDistance = rightTrace.fraction * 1024.0f;

			if ( fLeftClearDistance >= BACKAWAY_DISTANCE && fRightClearDistance >= BACKAWAY_DISTANCE )
			{
				bFound = TRUE;

				if ( fLeftClearDistance >= fRightClearDistance )
				{
					backAwayPoint.x = self->s.origin.x + (leftVector.x * fSpeed);
					backAwayPoint.y = self->s.origin.y + (leftVector.y * fSpeed);
					backAwayPoint.z = self->s.origin.z + (leftVector.z * fSpeed);

					nMovingDirection = LEFT;
				}
				else
				{
					backAwayPoint.x = self->s.origin.x + (rightVector.x * fSpeed);
					backAwayPoint.y = self->s.origin.y + (rightVector.y * fSpeed);
					backAwayPoint.z = self->s.origin.z + (rightVector.z * fSpeed);

					nMovingDirection = RIGHT;
				}
			}
			else
			{
				if ( fLeftClearDistance >= BACKAWAY_DISTANCE )
				{
					bFound = TRUE;
					backAwayPoint.x = self->s.origin.x + (leftVector.x * fSpeed);
					backAwayPoint.y = self->s.origin.y + (leftVector.y * fSpeed);
					backAwayPoint.z = self->s.origin.z + (leftVector.z * fSpeed);

					nMovingDirection = LEFT;
				}
				else
				if ( fRightClearDistance >= BACKAWAY_DISTANCE )
				{
					bFound = TRUE;
					backAwayPoint.x = self->s.origin.x + (rightVector.x * fSpeed);
					backAwayPoint.y = self->s.origin.y + (rightVector.y * fSpeed);
					backAwayPoint.z = self->s.origin.z + (rightVector.z * fSpeed);

					nMovingDirection = RIGHT;
				}
			}
		}

		if ( nPrevMovingDirection != STRAIGHT && nPrevMovingDirection != nMovingDirection )
		{
			nMovingDirection = nPrevMovingDirection;
			bFound = FALSE;
		}

		return bFound;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		CAMBOT_StartPain
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CAMBOT_StartPain(userEntity_t *self, userEntity_t *attacker, float kick, int damage)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	if(pGoalStack)
	{
		if( ( pGoalStack->pTopOfStack != NULL ) && !AI_HasGoalInQueue(self,GOALTYPE_CAMBOT_FOLLOWPLAYER))
		{

			if(hook)
			{
				CAMBOT_FoundPlayer(self, attacker );
				//We are in alert... let's change the color to red... (semi hack)
				hook->power_boost = 1;
			}
		}
	}
	AI_StartPain(self,attacker,kick,damage);
}

// ----------------------------------------------------------------------------
//
// Name:		CAMBOT_FlyTowardEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CAMBOT_FlyTowardEnemy( userEntity_t *self, userEntity_t *enemy )
{

	float fDistance = VectorXYDistance( self->s.origin, self->enemy->s.origin );

	if(!enemy)
	{
		return;
	}

	if ( fDistance < MIN_DISTANCE_AWAY )
	{
		// back away
		CVector destPoint;
		if ( CAMBOT_FindBackAwayPointFromEnemy( self, enemy, destPoint ) )
		{
			// make sure the destPoint is visible to the enemy
			//if ( gstate->inPVS(enemy->s.origin, destPoint) )
			if(Node_IsWithinRange(enemy->s.origin, destPoint,0, 0))
			{
				tr = gstate->TraceLine_q2( enemy->s.origin, destPoint, enemy, MASK_SOLID );
				if ( tr.fraction >= 1.0 )
				{
					AI_FlyTowardPoint2( self,destPoint , 0.10 );
					//AI_FlyTowardPoint( self, destPoint, FALSE, FALSE );
				}
			}
		}
	}
	else
	if ( fDistance >= MIN_DISTANCE_AWAY && fDistance <= MAX_DISTANCE_AWAY )
	{
		// move around a bit so that this will not become just a sitting target
		if ( rnd() > 0.75f && AI_IsEnemyTargetingMe( self, enemy ) )
		{
			
			CVector DodgePoint = self->s.origin;
			CVector Axis;

			Axis.x = 1;
			Axis.y = 1;
			Axis.z = 1;

			if(AI_ComputeFlyAwayPoint( self, DodgePoint, 250, 10, &Axis )==1)
			{
				MAPNODE_PTR NodeA = NODE_FindClosestNode( pAirNodes, NODETYPE_AIR, DodgePoint );
				if(NodeA)
				{
					AI_AddNewTaskAtFront( self, TASKTYPE_MOVETOLOCATION, NodeA->position );
				}
			}
			//If we can't find a flyaway point don't do a damned thing.
		}
	}
	else
	if ( fDistance > MAX_DISTANCE_AWAY )
	{	
		CVector destPoint = enemy->s.origin;
		destPoint.z += HEIGHT_FROM_PLAYER;
		AI_FlyTowardPoint2( self,destPoint , 0.10 );
			
	}
}

// ----------------------------------------------------------------------------
//
// Name:		CAMBOT_SoundAlarm
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CAMBOT_SoundAlarm( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	// play an alarm sound
	gstate->StartEntitySound(self,CHAN_AUTO,nAlarmSound,0.85f,hook->fMinAttenuation, hook->fMaxAttenuation);
}

// ----------------------------------------------------------------------------
//
// Name:		CAMBOT_AlertNearbyMonsters
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CAMBOT_AlertNearbyMonsters( userEntity_t *self, userEntity_t *pEnemy )
{
	float fMinDistance = 1024.0f;
	userEntity_t *pMonster = alist_FirstEntity( monster_list );
	while ( pMonster )
	{
		if ( self != pMonster && pMonster->flags & FL_MONSTER )
		{
			float fDistance = VectorDistance(self->s.origin, pEnemy->s.origin);
			if ( fDistance < fMinDistance && gstate->inPHS( self->s.origin, pMonster->s.origin ) )
			{
				AI_SetEnemy( pMonster, pEnemy );
				AI_AddNewGoal( pMonster, GOALTYPE_KILLENEMY, pEnemy );
			}
		}

		pMonster = alist_NextEntity( monster_list );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		CAMBOT_StartFollowPlayer
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CAMBOT_StartFollowPlayer( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FREEZE )
	{
		return;
	}

	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) != TASKTYPE_CAMBOT_FOLLOWPLAYER )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_CAMBOT_FOLLOWPLAYER );
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_CAMBOT_FOLLOWPLAYER.\n" );

	AI_SetStateAttacking( hook );
	AI_StartMove( self );

	self->nextthink	= gstate->time + 0.1f;

	
	// do not try to find the enemy in the main loop, it will be handled in the AI_CambotPathFollow()
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1.0f );
	pAIData->fValue = 0.0f;
	hook->power_boost = 1;
}

// ----------------------------------------------------------------------------
//
// Name:		CAMBOT_FollowPlayer
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CAMBOT_FollowPlayer( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_ZeroVelocity( self );
	if(self->enemy)
	{
		if(self->enemy->deadflag != DEAD_NO)
		{
			AI_RemoveCurrentTask(self,FALSE);
			return;
		}
	}
	else
	{
		AI_RemoveCurrentTask(self,FALSE);
		return;
	}

	if ( AI_IsVisible(self, self->enemy) )
	{
		hook->power_boost = 1;
		AI_FaceTowardPoint( self, self->enemy->s.origin );
		CAMBOT_FlyTowardEnemy( self, self->enemy );

	}
	else
	{
		if(!AI_HasTaskInQue(self,TASKTYPE_CHASE) && self->enemy)
			AI_AddNewTaskAtFront(self,TASKTYPE_CHASE,self->enemy);
		else
		{
			// test to see if the last path point is close to the enemy's position
			if ( !CAMBOT_IsPathToEntityClose( self, self->enemy ) )
			{
				AI_FindPathToEntity( self, self->enemy );
				if ( !hook->pPathList->pPath && AI_CanPath( hook ) )
				{
					AI_SetOkToAttackFlag( hook, TRUE );
					AI_RemoveCurrentGoal( self );

					// do not try another path for 5 seconds
					AI_SetNextPathTime( hook, 2.0f );
					//	no path was found, so give up
					return;
				}
			}
			// NSS[2/24/00]:Make sure we don't slam into anything.
			CVector Dir;
			Dir = self->velocity;
			Dir.Normalize();
			Dir.Multiply((self->velocity.Length()*0.3));		
			tr = gstate->TraceBox_q2( self->s.origin,self->s.mins,self->s.maxs,Dir, self, MASK_MONSTERSOLID );
			if(tr.fraction < 1.0)
			{
				self->velocity.Zero();
			}
			else
			{
				if ( AI_Fly( self ) == FALSE )
				{
					PATHLIST_KillPath( hook->pPathList );
				}
				hook->nMoveCounter++;
			}
			
			
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		CAMBOT_StartPathFollow
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CAMBOT_StartPathFollow( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FREEZE )
	{
		return;
	}

	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) != TASKTYPE_CAMBOT_PATHFOLLOW )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_CAMBOT_PATHFOLLOW );
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_CAMBOT_PATHFOLLOW.\n" );

	AI_SetStateWalking( hook );
	AI_StartMove( self );

	// no target, so find the nearest monster_path_corner
	userEntity_t *pPathCorner = com->FindClosestEntity( self, "monster_path_corner" );
	if ( pPathCorner )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );

		float fDistance = VectorXYDistance( self->s.origin, pPathCorner->s.origin );
		float fZDistance = VectorZDistance( self->s.origin, pPathCorner->s.origin );
		if ( AI_IsCloseDistance2( self, fDistance ) && fZDistance < 32.0f )
		{
			// find the next one and go to it
			userEntity_t *pNextPathCorner = AI_GetNextPathCorner( pPathCorner );
			if ( pNextPathCorner )
			{
				TASK_Set( pCurrentTask, pNextPathCorner );
			}
		}
		else
		{
			TASK_Set( pCurrentTask, pPathCorner );
		}
	}
	else
	{
		AI_RemoveCurrentGoal(self);
		return;
	}

	self->nextthink	= gstate->time + 0.1f;

	// do not try to find the enemy in the main loop, it will be handled in the AI_CambotPathFollow()
	AI_SetOkToAttackFlag( hook, TRUE );
	self->enemy = NULL;
	AI_SetTaskFinishTime( hook, -1.0f );
}

// ----------------------------------------------------------------------------
//
// Name:		CAMBOT_PathFollow
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CAMBOT_PathFollow( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	userEntity_t *pPathCorner = pAIData->pEntity;
	if(!pPathCorner)
		AI_RemoveCurrentTask(self,FALSE);

	float fDistance = VectorXYDistance( self->s.origin, pPathCorner->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pPathCorner->s.origin );
	// find a target
	userEntity_t *pEnemy = hook->fnFindTarget( self );
	if ( pEnemy && !(hook->ai_flags & AI_IGNORE_PLAYER))
	{
			CAMBOT_FoundPlayer( self, pEnemy );
			return;
	}

	if ( AI_IsCloseDistance2( self, fDistance ) && fZDistance < 32.0f )
	{
		// find the next one and go to it
		userEntity_t *pNextPathCorner = AI_GetNextPathCorner( pPathCorner );
		if ( pNextPathCorner )
		{
			TASK_Set( pCurrentTask, pNextPathCorner );
		}

		return;
	}


	AI_MoveTowardPoint( self, pPathCorner->s.origin, FALSE );
	AI_FaceTowardPoint( self, pPathCorner->s.origin);
	self->s.angles.pitch = 0.0f;
}


// ----------------------------------------------------------------------------
//
// Name:		CAMBOT_ParseEpairs
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CAMBOT_ParseEpairs( userEntity_t *self )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );
	AI_InitNodeList( self );
	Terrain_Node_Change(self, self->movetype );
	AI_ParseEpairs(self);
	self->think = CAMBOT_Think;
}

// ----------------------------------------------------------------------------
//
// Name:		monster_cambot
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_cambot( userEntity_t *self )
{
	playerHook_t *hook	= AI_InitMonster( self, TYPE_CAMBOT );

	self->className		= "monster_cambot";
	self->netname		= tongue_monsters[T_MONSTER_CAMBOT];

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

	nAlarmSound = gstate->SoundIndex("e1/m_cambotalarm.wav");

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	//	turn rates for each axis in degrees per second
	self->ang_speed.Set( 90, 90, 90 );

	

    self->fragtype          |= FRAGTYPE_NOBLOOD;   // don't leave blood upon death
    self->fragtype          |= FRAGTYPE_ROBOTIC;   // apply robotic gib parts
    self->fragtype          |= FRAGTYPE_ALWAYSGIB;

	hook->dflags			|= DFL_MECHANICAL | DFL_FACEPITCH;

	self->pain				= CAMBOT_StartPain;
	
	hook->bOkToAttack		= FALSE;
	
	hook->fnFindTarget		= CAMBOT_FindNearPlayer;
	
	self->think				= CAMBOT_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;

	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );

	AI_DetermineMovingEnvironment( self );

	// give cambot his little flashlight
	self->inventory = gstate->InventoryNew (MEM_MALLOC);

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
										  "light", 
										  light_fire, 
										  ITF_TRACE);
		gstate->InventoryAddItem (self, self->inventory, self->curWeapon);
	}

	// turn on the cambot light
	self->curWeapon->use( self->curWeapon, self );
	self->movetype			= MOVETYPE_HOVER;
    gstate->LinkEntity( self );

	self->view_ofs.Set(0.0f, 0.0f, 0.0f);
	//Make light white
	hook->power_boost		= 0;
	hook->attack_boost_time	= 0;

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
void world_ai_cambot_register_func()
{
	gstate->RegisterFunc( "CAMBOT_Think", CAMBOT_Think );
	gstate->RegisterFunc( "CAMBOT_ParseEpairs", CAMBOT_ParseEpairs );
	gstate->RegisterFunc( "CAMBOT_FindNearPlayer", CAMBOT_FindNearPlayer );
	gstate->RegisterFunc( "cambot_moveup", cambot_moveup);
	gstate->RegisterFunc( "cambot_movedown", cambot_movedown);
	gstate->RegisterFunc( "CAMBOT_SoundAlarm", CAMBOT_SoundAlarm);
	gstate->RegisterFunc( "CAMBOT_StartFollowPlayer", CAMBOT_StartFollowPlayer);
	gstate->RegisterFunc( "CAMBOT_FollowPlayer", CAMBOT_FollowPlayer);
	gstate->RegisterFunc( "CAMBOT_StartPathFollow", CAMBOT_StartPathFollow);
	gstate->RegisterFunc( "CAMBOT_PathFollow", CAMBOT_PathFollow);
	gstate->RegisterFunc( "CAMBOT_StartPain", CAMBOT_StartPain);
}
