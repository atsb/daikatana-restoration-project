// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#include "world.h"
#include "ai.h"
#include "ai_utils.h"
#include "ai_move.h"
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"
//#include "actorlist.h"// SCG[1/23/00]: not used
#include "ai_common.h"


#define TO_FLY_HEIGHT 350.0f
#define T0_LAND_HEIGHT 250.0f

//#define SPEW_HARPY_MESSAGES 1

/* ***************************** define types ****************************** */
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */

void HARPY_Start_Die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &destPoint );

/* **************************** Global Variables *************************** */
//extern float sin_tbl[];// SCG[1/23/00]: not used
//static int DeltaH;// SCG[1/23/00]: not used

/* **************************** Global Functions *************************** */
//void AddTrackLight(userEntity_t *ent,float size,float r,float g,float b,int fxflags);// SCG[1/23/00]: not used
void AI_ParseEpairs2( userEntity_t *self );
void AI_Chase( userEntity_t *self );
int AI_StartFly( userEntity_t *self );

/* **************************** Extern Functions *************************** */

void AI_StartDie( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &destPoint );

/* ******************************* exports ********************************* */

DllExport void monster_harpy(userEntity_t *self);



// ----------------------------------------------------------------------------
//
// Name:		HARPY_Think
// Description:All internal pre-thinking stuff goes here or we can hook the
// pre-think function.  Either way it works.  In-Line think related handlers
// should go here.  Anything that needs to be thought of before all thinking 
// should be hooked through the prethink function.
// Input:self
// Output:<nothing>
// Note:
// 
// ----------------------------------------------------------------------------
void HARPY_Think(userEntity_t *self)
{
#if SPEW_HARPY_MESSAGES
	AI_Dprintf("HARPY_Think\n");
#endif
	AI_TaskThink(self);

	/*if(DeltaH >= 12)
	{	
		DeltaH = 0;
	}
	self->s.origin.z += 2.0*(sin_tbl[DeltaH]);
	DeltaH += 1;*/
}


// ----------------------------------------------------------------------------
//
// Name:		HARPY_Attack_Think
// Description:Determines what tasks the HARPY will do during its attack
// Input:self
// Output:<nothing>
// Note:
// 
// ----------------------------------------------------------------------------
void HARPY_Attack_Think( userEntity_t *self )
{
#if SPEW_HARPY_MESSAGES
	AI_Dprintf("HARPY_Attack_Think\n");	
#endif

	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );
//	GOAL_PTR pGoal = pGoalStack->pTopOfStack;// SCG[1/23/00]: not used
	
	if ( !self->enemy )
	{
		self->enemy = pAIData->pEntity;
	}

	//simple trap for AI_Think

	self->think = HARPY_Think;

	self->curWeapon = gstate->InventoryFindItem (self->inventory, "magic arrow");
	
	AI_SetOkToAttackFlag( hook, TRUE );
	// Go into Generic Path Finding Mode  We always fly to the player if we cannot see him!
	AI_AddNewTaskAtFront( self, TASKTYPE_HARPY_ATTACK, self );
	AI_AddNewTaskAtFront( self, TASKTYPE_HARPY_CHASE, self );

	//Setup next think time
	self->nextthink = gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
//
// Name:		HARPY_DetermineMovementMode
// Description:Function to determine what movement mode the Chaingunner should 
// be in.
// Input: userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
int HARPY_DetermineMovementMode(userEntity_t *self)
{
#if SPEW_HARPY_MESSAGES
	AI_Dprintf("HARPY_DetermineMovementMode\n");
#endif

	frameData_t *pSequence = NULL;
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	float Player  = AI_Determine_Room_Height( self->enemy, 500, 0);
	float Monster = AI_Determine_Room_Height( self, 500, 0);
	
	if(self->movetype == MOVETYPE_FLY)
	{
		
		//Has the player gone into a room that has a low ceiling?
		if((Player < T0_LAND_HEIGHT)&&(self->movetype == MOVETYPE_FLY))
		{
			//AI_Dprintf("Check for Landing!\n");
			float Floor = AI_Determine_Room_Height( self, 500, 1);
			CVector NewV = self->s.origin;
			NewV.z	= (self->s.origin.z - Floor)+50;

			MAPNODE_PTR NodeA = NODE_FindClosestNode( pGroundNodes, NODETYPE_GROUND, NewV );
			if(NodeA)
			{
//				GOAL_PTR pGoal = pGoalStack->pTopOfStack;// SCG[1/23/00]: not used
				CVector Position = NodeA->position;
				Position.z += 85.0f;
				AI_AddNewTaskAtFront( self, TASKTYPE_HARPY_GROUND);
				AI_AddNewTaskAtFront( self, TASKTYPE_HARPY_INITIATE_GROUND );
				AI_AddNewTaskAtFront( self, TASKTYPE_HARPY_MOVETOLOCATION, Position );
				AI_SetOkToAttackFlag( hook, FALSE );
				//We are still in the air keep the air animations.
				pSequence = FRAMES_GetSequence( self, "flya" );
				AI_ForceSequence( self, pSequence );
				return 1;
			}
		}
	}
	else
	{
		//Test to see if we need to go back into flight
		if ( Player > TO_FLY_HEIGHT  && Monster > TO_FLY_HEIGHT)
		{
//			GOAL_PTR pGoal = pGoalStack->pTopOfStack;// SCG[1/23/00]: not used
			AI_AddNewTaskAtFront( self, TASKTYPE_HARPY_AIR);
			AI_AddNewTaskAtFront( self, TASKTYPE_HARPY_INITIATE_AIR );
			AI_SetOkToAttackFlag( hook, FALSE );
			return 1;
		}	
	}
	return 0;
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
void HARPY_StartMoveToLocation( userEntity_t *self )
{
	AI_Dprintf("HARPY_StartMoveToLocation\n");

	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );


	
	if(!AI_StartSequence(self, "runa", FRAME_LOOP))
		return;

	// this is needed so that the code will not detect it has not moved 
	// in the first update
	CVector org( self->s.origin.x + 10.f, self->s.origin.y + 10.0f, self->s.origin.z );
	hook->last_origin.Set( org );

	// get rid of old path
	PATHLIST_KillPath( hook->pPathList );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	_ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	float fDistance = AI_ComputeDistanceToPoint( self, pAIData->destPoint );

#if SPEW_HARPY_MESSAGES
	AI_Dprintf( "Starting TASKTYPE_HARPY_MOVETOLOCATION.\n" );
#endif

	self->nextthink	= gstate->time + 0.1f;

	AI_SetOkToAttackFlag( hook, TRUE );

	float fSpeed = AI_ComputeMovingSpeed( hook );
	
	float fTime = (fDistance / fSpeed) + 2.0f;
	
	AI_SetTaskFinishTime( hook, fTime );
	
	AI_SetMovingCounter( hook, 0 );
}


void HARPY_MoveToLocation (userEntity_t *self)
{
#if SPEW_HARPY_MESSAGES
	AI_Dprintf("HARPY_MoveToLocation\n");
#endif

	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );

	AI_FaceTowardPoint(self, pAIData->destPoint);
	
	//Check to see if we are close to our current Goal
	if ( AI_IsCloseDistance2( self, fXYDistance ) && fZDistance < 32.0f )
	{
		
		//Handle removing the Goal once you are done with the movement
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		
		if ( GOAL_GetType( pCurrentGoal ) == GOALTYPE_MOVETOLOCATION && GOAL_GetNumTasks( pCurrentGoal ) == 1 )
		{
			AIDATA_PTR pAIData = GOAL_GetData( pCurrentGoal );

			float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
			float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
			if ( AI_IsCloseDistance2( self, fXYDistance ) && fZDistance < 32.0f )
			{
				GOAL_Satisfied( pCurrentGoal );
			}
		}
		AI_RemoveCurrentTask( self );
		return;
	}
	
	//Go do the movement if we are still far away from the destination
	if ( AI_IsOkToMoveStraight( self, pAIData->destPoint, fXYDistance, fZDistance ) )
	{
		//Comment:<nss>
		//We have checked to see if we can move straight without nodes...
		//send our entity structure, the destination, and a FALSE that tells the AI_FlyTowardPoint we are moving node 'free'.
		AI_FlyTowardPoint( self, pAIData->destPoint, FALSE , TRUE);
	}
	else
	{
		if ( !AI_HandleUse( self ) )
		{
			if ( hook->pPathList->pPath )
			{
				if ( AI_Fly( self ) == FALSE )
				{
					if ( !AI_FindPathToPoint( self, pAIData->destPoint ) )
					{
						AI_RestartCurrentGoal( self );
					}
				}					
			}
			else
			{
				AI_Dprintf("No path to point!\n");
				if ( !AI_FindPathToPoint( self, pAIData->destPoint ) )
				{
					//AI_Dprintf("Restarting Goal!\n");
					AI_RestartCurrentGoal( self );
				}
			}
		}
	}

	CVector dir = pAIData->destPoint - self->s.origin;
	dir.Normalize();
	self->s.angles.yaw = VectorToYaw(dir);
	self->s.angles.pitch = 0.0f;

}



// ----------------------------------------------------------------------------
//
// Name:		HARPY_StartDodge
// Description:Sets up a good point to go to or removes the task if no point can 
// be found at that position
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_StartDodge( userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	CVector DodgePoint = self->s.origin;
	CVector Axis;

	if(AI_StartSequence(self, "runa", FRAME_LOOP))
	{
		self->nextthink	= gstate->time + 0.1f;
//		return;
	}

	Axis.x = 1;
	Axis.y = 1;
	Axis.z = 0;//Set the Z axis (height) to 0 so we get an XY only destination
	if(AI_ComputeAwayPoint( self, DodgePoint, 250, 10, &Axis )==0)
	{
		AI_RemoveCurrentTask(self,FALSE);
	}
	else
	{
		MAPNODE_PTR NodeG = NODE_FindClosestNode( pGroundNodes, NODETYPE_GROUND, DodgePoint );
		
		if(NodeG)
		{
			pAIData->destPoint = NodeG->position;
			AI_Dprintf("Dropped Marker!\n");
			//AI_Drop_Marker (self, pAIData->destPoint);
		}
		else
		{
			AI_Dprintf("No node close or DodgePoint invalid!\n");
			AI_RemoveCurrentTask(self,FALSE);
		}
	}
}


// ----------------------------------------------------------------------------
//
// Name:		HARPY_Dodge
// Description:Calls standard AI_Dodge
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_Dodge( userEntity_t *self)
{
	AI_Dodge( self );
}


// ----------------------------------------------------------------------------
//
// Name:		HARPY_StartChase
// Description:The Setup for the chase thinking for Miss Harpy
// Input: userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_StartChase( userEntity_t *self)
{
	AI_Dprintf("HARPY_StartChase\n");

	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !AI_CanMove( hook ) )
	{
		self->nextthink	= gstate->time + 0.1f;
		AI_RemoveCurrentTask( self );
		return;
	}
	if(self->movetype == MOVETYPE_FLY)
	{
		if(!AI_StartSequence(self, "flya", FRAME_LOOP))
		{
			self->nextthink	= gstate->time + 0.1f;
			return;
		}
	}
	else
	{
		if(!AI_StartSequence(self, "runa", FRAME_LOOP))
		{
			self->nextthink	= gstate->time + 0.1f;
			return;
		}
	}
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	if ( !self->enemy )
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		AIDATA_PTR pAIData = GOAL_GetData(pCurrentGoal);
		self->enemy = pAIData->pEntity;
	}

//	float fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );// SCG[1/23/00]: not used

	if(self->movetype == MOVETYPE_FLY)
	{
		AI_StartFly( self );		
	}
	else
	{
    	userEntity_t *pEnemy = self->enemy;
        float fXYDistance = VectorXYDistance( self->s.origin, pEnemy->s.origin );
        float fZDistance = VectorZDistance( self->s.origin, pEnemy->s.origin );
        if ( !AI_FindPathToEntity( self, pEnemy, FALSE ) &&
             !AI_IsOkToMoveStraight( self, pEnemy->s.origin, fXYDistance, fZDistance ) )
		{
			// do not try another path for 2 seconds
			AI_SetNextPathTime( hook, 2.0f );

			//	no path was found, so do not do a chasing attack
			AI_RemoveCurrentGoal( self );
			self->nextthink	= gstate->time + 0.1f;
			return;
		}

        AI_SetStateRunning( hook );
		if ( AI_StartMove( self ) == FALSE )
		{
			TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
            _ASSERTE( pCurrentTask );
			if ( TASK_GetType( pCurrentTask ) != TASKTYPE_CHASE )
			{
				self->nextthink	= gstate->time + 0.1f;
				return;
			}
		}	
	}

	self->nextthink	= gstate->time + 0.1f;

	AI_SetOkToAttackFlag( hook, FALSE );

	AI_SetTaskFinishTime( hook, 10.0f );
	//Set the movingcounter
	hook->nMovingCounter = 0;
}



// ----------------------------------------------------------------------------
//
// Name:		HARPY_Chase
// Description:The chase thinking for Mr. Chaingunner
// Input: userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_Chase( userEntity_t *self)
{
#if SPEW_HARPY_MESSAGES
	AI_Dprintf("HARPY_Chase\n");
#endif

	playerHook_t *hook = AI_GetPlayerHook( self );
//	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	AI_GetCurrentGoalStack( hook );// SCG[1/23/00]: pGoalStack not used

	//Check for landing every 3rd pass
	if(HARPY_DetermineMovementMode(self))
	{
		self->nextthink	= gstate->time + 0.1f;
		return;
	}

	//Until we are halfway in range we have a 15% chance of going into attack mode.  Once
	// we get within 1/2 our gun's distance we should then start fucking up the player.
	float fDist = VectorDistance( self->s.origin, self->enemy->s.origin );
	ai_weapon_t *weapon = (ai_weapon_t *) self->curWeapon;
	//if ( (( fDist <= weapon->distance) && (rnd() < 0.15f)) || (fDist <= weapon->distance/2) ) 
	
	if ( fDist <= weapon->distance && AI_IsLineOfSight( self, self->enemy ) ) 
	{
		AI_RemoveCurrentTask( self );
		self->nextthink	= gstate->time + 0.1f;
		return;
	}

	//Let's do all that Chase mumbo-jumbo
	AI_Chase(self);

	//Increment the moving counter which is used for resolution control on the DetermineMovement Mode and also for the "bouncing" effect.
	hook->nMovingCounter++;
	if(hook->nMovingCounter > 360)
	{
		AI_RemoveCurrentTask(self,FALSE);
		self->nextthink	= gstate->time + 0.1f;
		return;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		HARPY_Set_Attack_Seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_Set_Attack_Seq( userEntity_t *self )
{
#if SPEW_HARPY_MESSAGES
	AI_Dprintf("HARPY_Set_Attack_Seq\n");
#endif

    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {
//	    playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	    frameData_t *pSequence = NULL;
	    if ( self->movetype == MOVETYPE_FLY )
	    {
		    pSequence = FRAMES_GetSequence( self, "atakb" );
	    }
	    else
	    {
		    pSequence = FRAMES_GetSequence( self, "ataka" );
	    }

        AI_ForceSequence( self, pSequence );
    }
    else
    {
        AI_FaceTowardPoint( self, self->enemy->s.origin );
        AI_UpdatePitchTowardEnemy( self );
    }
}

// ----------------------------------------------------------------------------
//
// Name:		HARPY_DetermineAttackMode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_DetermineAttackMode( userEntity_t *self )
{
#if SPEW_HARPY_MESSAGES
	AI_Dprintf("HARPY_DetermineAttackMode\n");
#endif

	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->nAttackMode == ATTACKMODE_NORMAL )
	{
		if ( rnd() < 0.25f )
		{
			hook->nAttackMode = ATTACKMODE_SWOOPSTRAFE;
			hook->strafe_dir = rand() % (STRAFE_RIGHT+1);
		}
	}
	else
	if ( hook->nAttackMode == ATTACKMODE_SWOOPSTRAFE )
	{
	}
}

// ----------------------------------------------------------------------------
//
// Name:		HARPY_ModifyStrafeDirectionAfterCollision
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_ModifyStrafeDirectionAfterCollision( userEntity_t *self )
{
#if SPEW_HARPY_MESSAGES
	AI_Dprintf("HARPY_ModifyStrafeDirectionAfterCollision\n");
#endif

	playerHook_t *hook = AI_GetPlayerHook( self );

	switch ( hook->strafe_dir )
	{
		case STRAFE_LEFT:
		{
			hook->strafe_dir = STRAFE_RIGHT;
			break;
		}
		case STRAFE_RIGHT:
		{
			hook->strafe_dir = STRAFE_LEFT;
			break;
		}
		case STRAFE_UPLEFT:
		{
			hook->strafe_dir = STRAFE_DOWNRIGHT;
			break;
		}
		case STRAFE_UPRIGHT:
		{
			hook->strafe_dir = STRAFE_DOWNLEFT;
			break;
		}
		case STRAFE_DOWNLEFT:
		{
			hook->strafe_dir = STRAFE_UPRIGHT;
			break;
		}
		case STRAFE_DOWNRIGHT:
		{
			hook->strafe_dir = STRAFE_UPLEFT;
			break;
		}
		default:
		{
			break;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		HARPY_HandleCollision
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_HandleCollision( userEntity_t *self, float fSpeed, CVector &forwardDirection )
{
#if SPEW_HARPY_MESSAGES
	AI_Dprintf("HARPY_HandleCollision\n");
#endif

//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	float fDist = fSpeed / 10.0f;
	if ( self->velocity.Length() > 0 )
	{
		fDist = fSpeed / 5.0f;
	}
	if ( fDist < 32.0f )
	{
		fDist = 32.0f;
	}

	int nWallType = AI_CheckAirTerrain( self, forwardDirection, fDist );
	switch ( nWallType )
	{
		case TER_OBSTRUCTED:
		{
			CVector dir;
			AI_ComputeAlongWallVector( self, forwardDirection, terrain.wall_normal, dir );
			forward = dir;
			
			HARPY_ModifyStrafeDirectionAfterCollision( self );
			break;
		}
		case TER_WALL:
		{
			CVector dir;
			AI_ComputeAlongWallVector( self, forwardDirection, terrain.wall_normal, dir );
			forwardDirection = dir;

			HARPY_ModifyStrafeDirectionAfterCollision( self );
			break;
		}
		
		case TER_HIGH_OBS:
		{
			
			// if it is a ceiling then, glide along it
			if ( terrain.wall_normal.z < -0.7f )
			{
				forwardDirection.z = 0.0f;
				forwardDirection.Normalize();
			}
			else
			{
				// duck under
				forwardDirection.x = 0.0f;
				forwardDirection.y = 0.0f;
				forwardDirection.z = -1.0f;
			}
			break;
		}
		case TER_WALL_RIGHT:
		{
			// right side is blocked
			// rotate the wall normal to the left so that we can move around this 
			// obstacle
			CVector dir( terrain.wall_normal.y, -terrain.wall_normal.x, forwardDirection.z );
			dir.Normalize();
			forwardDirection = dir;
			
			HARPY_ModifyStrafeDirectionAfterCollision( self );
			break;
		}
		case TER_WALL_LEFT:
		{
			// left side is blocked
			// rotate the wall normal to the right so that we can move around this 
			// obstacle
			CVector dir( -terrain.wall_normal.y, terrain.wall_normal.x, forwardDirection.z );
			dir.Normalize();
			forwardDirection = dir;
			
			HARPY_ModifyStrafeDirectionAfterCollision( self );
			break;
		}		
		case TER_LOW_OBS:
		{
			// go up

			if ( terrain.wall_normal.z > 0.7f )
			{
				forwardDirection.z = 0.0f;
				forwardDirection.Normalize();
			}
			else
			{
				forwardDirection.x = 0.0f;
				forwardDirection.y = 0.0f;
				forwardDirection.z = 1.0f;
			}
			break;
		}

		default:
		{
			break;
		}
	}
}


// ----------------------------------------------------------------------------
// <nss>
// Name:		HARPY_StartInitiateAir
// Description:Setup for the Ground Sequence.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_StartInitiateAir( userEntity_t *self )
{
	AI_Dprintf("HARPY_StartInitiateAir\n");

//	frameData_t *pSequence = NULL;// SCG[1/23/00]: not used
	//pSequence = FRAMES_GetSequence( self, "drop" );
	AI_ForceSequenceInReverse( self, "drop", FRAME_ONCE );
}


// ----------------------------------------------------------------------------
// <nss>
// Name:		HARPY_InitiateAir
// Description:Setup for the Ground Sequence.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_InitiateAir( userEntity_t *self )
{
#if SPEW_HARPY_MESSAGES
	AI_Dprintf("HARPY_InitiateAir\n");
#endif

	//AI_Dprintf("Init Air frame:%d\n",self->s.frame);
	if( self->s.frame <= 191)
	{
		AI_RemoveCurrentTask(self,FALSE);
	}
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		HARPY_StartAir
// Description:Setup for the Ground Sequence.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_StartAir( userEntity_t *self )
{
	AI_Dprintf("HARPY_StartAir\n");

	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );


	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );

	self->s.angles.pitch = 0;
	self->s.angles.roll = 0;
	
	AI_SetOkToAttackFlag( hook, FALSE );
	self->velocity.Zero();
	self->velocity.z = 275;
	
	AI_SetTaskFinishTime( hook, -1 );
	pAIData->fValue = gstate->time;

	//Setup movement type
	//Attach smoke and light for jetpack
	hook->nAttackType = ATTACK_AIR_RANGED;
	Terrain_Node_Change(self, MOVETYPE_FLY);
	
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		HARPY_Air
// Description:This function will make the HARPY member ignite his jetpack 
// and will get him ready for Air attacks.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_Air( userEntity_t *self )
{
	AI_Dprintf("HARPY_Air\n");

	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );

	//AI_Dprintf("frame:%d\n",self->s.frame);

	if((pAIData->fValue + 0.75f) < gstate->time)
	{
		//AI_Dprintf("Air Mode Activated!\n");

		AI_RemoveCurrentTask(self,FALSE);
		AI_SetOkToAttackFlag( hook, TRUE );
	}
	self->nextthink	= gstate->time + 0.1f;
}


// ----------------------------------------------------------------------------
// <nss>
// Name:		HARPY_StartInitiateAir
// Description:Setup for the Ground Sequence.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_StartInitiateGround( userEntity_t *self )
{
#if SPEW_HARPY_MESSAGES
	AI_Dprintf("HARPY_StartInitiateGround\n");
#endif

	frameData_t *pSequence = NULL;
	pSequence = FRAMES_GetSequence( self, "drop" );
	AI_ForceSequence( self, pSequence );
}


// ----------------------------------------------------------------------------
// <nss>
// Name:		HARPY_InitiateAir
// Description:Setup for the Ground Sequence.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_InitiateGround( userEntity_t *self )
{
	AI_Dprintf("HARPY_InitiateGround\n");

	//AI_Dprintf("Init Ground frame:%d\n",self->s.frame);
	if(self->s.frame >= 209)
	{
		AI_RemoveCurrentTask(self,FALSE);
	}
}


// ----------------------------------------------------------------------------
// <nss>
// Name:		HARPY_StartGround
// Description:Setup for the Ground Sequence.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_StartGround( userEntity_t *self )
{
#if SPEW_HARPY_MESSAGES
	AI_Dprintf("HARPY_StartGround\n");
#endif

	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );
	
	self->s.angles.pitch = 0;
	self->s.angles.roll = 0;

	self->velocity.Zero();
	self->velocity.z = 35;

	AI_SetOkToAttackFlag( hook, FALSE );
	
	pAIData->fValue = gstate->time;

	AI_SetTaskFinishTime( hook, -1 );

}

// ----------------------------------------------------------------------------
// <nss>
// Name:		HARPY_Ground
// Description:This function will land the HARPY and get him ready for Ground
// attacks.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_Ground( userEntity_t *self )
{
	AI_Dprintf("HARPY_Ground\n");

	userEntity_t *attacker = self->enemy;
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	self->s.angles.pitch = 0;
	self->s.angles.roll = 0;

	//Once we have reached landing range and our flya animation has reached the last sequence
	//kill the ground task, set our move type to walk, and run the landing sequence.
	if( pAIData->fValue + 0.50f < gstate->time)
	{
		hook->nAttackType = ATTACK_GROUND_RANGED;
		Terrain_Node_Change(self, MOVETYPE_WALK);
		//AI_Dprintf("Ground Mode Activated!\n");
		AI_RemoveCurrentTask(self,FALSE);
		AI_SetOkToAttackFlag( hook, TRUE );
		return;
	}
}



// ----------------------------------------------------------------------------
//
// Name:		HARPY_UpdateAttackMovement
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_UpdateAttackMovement( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );	
	userEntity_t *enemy = self->enemy;

	if(self->movetype == MOVETYPE_HOVER)
	{
		if ( hook->nAttackMode == ATTACKMODE_NORMAL )
		{
			if ( hook->strafe_time <= gstate->time )
			{
				hook->strafe_time = gstate->time + (rnd() * (640 / hook->attack_speed)) + 1.5f;
				hook->strafe_dir = rand() % (STRAFE_DOWNRIGHT+1);

				HARPY_DetermineAttackMode( self );
			}
			
			CVector vectorTowardPoint;
			CVector angleTowardPoint;
			vectorTowardPoint = self->s.origin - enemy->s.origin;
			vectorTowardPoint.Normalize();
			VectorToAngles( vectorTowardPoint, angleTowardPoint );
			
			angleTowardPoint.x = -40.0f;
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
					angleTowardPoint.x += -10.0f;
					angleTowardPoint.y += 45.0f;
					break;
				}
				case STRAFE_UPRIGHT:
				{
					angleTowardPoint.x += -10.0f;
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
			CVector destPoint;
			VectorMA( enemy->s.origin, facingDir, fHalfAttackDistance, destPoint );

			CVector movingVector;
			vectorTowardPoint = destPoint - self->s.origin;
			vectorTowardPoint.Normalize();

			float fSpeed = hook->run_speed;
			HARPY_HandleCollision( self, fSpeed, vectorTowardPoint );
			
			vectorTowardPoint.Normalize();
			// scale speed based on current frame's move_scale
			float fScale = FRAMES_ComputeFrameScale( hook );
// SCG[1/23/00]: not used
/*
			if(fScale > 2)
				int PoopY = 1;
			if(fSpeed > 500)
				int PoopH = 1;
*/
			self->velocity = vectorTowardPoint * (fSpeed * fScale);
		}
		else
		if ( hook->nAttackMode == ATTACKMODE_SWOOPSTRAFE )
		{
			CVector vectorTowardPoint;
			CVector angleTowardPoint;
			vectorTowardPoint = self->s.origin - enemy->s.origin;
			vectorTowardPoint.Normalize();
			VectorToAngles( vectorTowardPoint, angleTowardPoint );
			
			angleTowardPoint.x = -15.0f;
			switch ( hook->strafe_dir )
			{
				case STRAFE_LEFT:
				{
					angleTowardPoint.y += 90.0f;
					break;
				}
				case STRAFE_RIGHT:
				{
					angleTowardPoint.y -= 90.0f;
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

			float fCloseAttackDistance = hook->attack_dist * 0.20f;
			CVector destPoint;
			VectorMA( enemy->s.origin, facingDir, fCloseAttackDistance, destPoint );

			CVector movingVector;
			vectorTowardPoint = destPoint - self->s.origin;
			vectorTowardPoint.Normalize();

			float fSpeed = (hook->run_speed * 1.3f);
			HARPY_HandleCollision( self, fSpeed, vectorTowardPoint );

			// scale speed based on current frame's move_scale
			float fScale = FRAMES_ComputeFrameScale( hook );
// SCG[1/23/00]: not used
/*
			if(fScale > 2)
				int PoopY = 1;
			if(fSpeed > 500)
				int PoopH = 1;
*/
			self->velocity = vectorTowardPoint * (fSpeed * fScale);
			vectorTowardPoint.Normalize();
			float fDistance = VectorDistance( self->s.origin, destPoint );
			if ( fDistance < (fCloseAttackDistance+25.0f) )
			{
				hook->nAttackMode = ATTACKMODE_NORMAL;
				hook->strafe_time = gstate->time + 3.0f;
			}
		}
	}
	else
	{
		//Ground section to update different moving techniques
	}
}



// ----------------------------------------------------------------------------
//
// Name:		HARPY_Begin_Attack
// Description: Start Attack sequence
// Input: userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_Begin_Attack( userEntity_t *self )
{
#if SPEW_HARPY_MESSAGES
	AI_Dprintf("HARPY_Begin_Attack\n");
#endif

	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );

	hook->nTargetFrequency = 32.0f;

   	//AI_ForceSequence( self, pSequence ); 

	AI_SetTaskFinishTime( hook, -1 );
	pAIData->nValue = 0;
	AI_ZeroVelocity( self );
	AI_SetOkToAttackFlag( hook, FALSE );
	
}

// ----------------------------------------------------------------------------
//
// Name:		HARPY_Attack
// Description:Standard Attack function for Chaingang member
// Input:userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_Attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );

	if( !self->enemy || AI_IsEnemyDead( self ) )
		return;

	AI_FaceTowardPoint( self, self->enemy->s.origin );
	int End = AI_IsEndAnimation( self );

	if(End)
		AI_SetOkToAttackFlag( hook, TRUE );
	if (AI_CanMove( hook ) )
	{
		//if we are in the air
		if ( AI_IsInAir( self ))
			AI_UpdatePitchTowardEnemy( self );
		//Update for Attack movement
		//Also checks for the air ground transition
		HARPY_UpdateAttackMovement( self );

		if(self->velocity.x > 1000.0f)
		{
			CVector Dir;
			Dir = self->velocity;
			Dir.Normalize();
			Dir = Dir * hook->run_speed;
			self->velocity = Dir;
		}
	}

	if( End && pAIData->nValue == 1 && self->movetype == MOVETYPE_WALK )
	{
			
			
			if(HARPY_DetermineMovementMode(self))
			{
				AI_RemoveCurrentTask(self,TRUE);
				return;
			}
			AI_AddNewTaskAtFront( self, TASKTYPE_HARPY_DODGE, self );
			AI_SetOkToAttackFlag( hook, FALSE );
			//AI_AddNewTask( self, TASKTYPE_HARPY_ATTACK);

			return;
	}	

	if ( End && !AI_IsEnemyDead( self ) )
	{
		float fDist = VectorDistance(self->enemy->s.origin, self->s.origin);
		if ( !AI_IsWithinAttackDistance( self, fDist ) || !AI_IsVisible( self, self->enemy ))
		{
			AI_RemoveCurrentTask( self );
			return;
		}
		else
		{
			HARPY_Set_Attack_Seq(self);
			pAIData->nValue = 1;
		}

	}
	
	int face, ready; 
	if( self->enemy && !AI_IsEnemyDead( self ) )
	{
		face = AI_IsFacingEnemy( self, self->enemy );
		ready = AI_IsReadyToAttack1( self );
		if( ready && face )
		{
			AI_PlayAttackSounds( self );
				
			if( !AI_DetectForLikeMonsters( self ) )
			{
				if(hook->attack_boost_time > gstate->time)
					AI_RemoveCurrentTask(self,FALSE);
				ai_fire_curWeapon( self );
				hook->attack_boost_time = gstate->time + 1.0f;
			}
			else
			{
				AI_RemoveCurrentTask( self, TASKTYPE_HARPY_DODGE, self );
				AI_AddNewTask( self, TASKTYPE_HARPY_ATTACK);
				return;
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		HARPY_Start_Pain
// Description: pain function for skeleton that selects front or back hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_Start_Pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,25);
}

// ----------------------------------------------------------------------------
//
// Name:		HARPY_Start_Die
// Description: die handler for chain gang member
// Input:userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
// int damage, CVector &destPoint 
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void HARPY_Start_Die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                          int damage, CVector &destPoint )
{
#if SPEW_HARPY_MESSAGES
	AI_Dprintf("HARPY_Start_Die\n");
#endif

	int notdeadyet = (self->deadflag == DEAD_NO);

	// call AI_StartDie, which does generic dying stuff
	AI_StartDie( self, inflictor, attacker, damage, destPoint );

    if ( notdeadyet && self->deadflag == DEAD_DYING )
	{ // pick a different sequence for different deaths
		AI_ForceSequence( self, "diea" );
	}
}


// ----------------------------------------------------------------------------
//
// Name:		monster_harpy
// Description:
//	creates the superfly entity and sets him up to smack down
//	everything that gets in his way.
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_harpy( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_HARPY );

	self->className			= "monster_harpy";
	self->netname			= tongue_monsters[T_MONSTER_HARPY];

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
	//	properties
	///////////////////////////////////////////////////////////////////////////
	hook->pain_chance		= 20;

	///////////////////////////////////////////////////////////////////////////
	//	CSV Config
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
										  "magic arrow", 
										  magic_arrow_attack, 
										  ITF_PROJECTILE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}
	
	//Setup standard function pointers.
	hook->fnStartAttackFunc = HARPY_Begin_Attack;
	hook->fnAttackFunc		= HARPY_Attack;

	self->die               = HARPY_Start_Die;
	self->pain				= HARPY_Start_Pain;
	self->think				= AI_ParseEpairs2;
	self->nextthink			= gstate->time + 0.2;
	
	//Setup Movetypes and attack types
	self->movetype			= MOVETYPE_FLY;
	hook->nAttackType		= ATTACK_AIR_RANGED;
	hook->nAttackMode		= ATTACKMODE_NORMAL;

	hook->strafe_time = gstate->time;
	hook->strafe_dir  = STRAFE_LEFT;


	// NSS[2/11/00]:Always gib
	self->fragtype     |= FRAGTYPE_ALWAYSGIB;

	hook->pain_chance		= 10;

	AI_DetermineMovingEnvironment( self );

	gstate->LinkEntity(self);

	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);


}


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_harpy_register_func()
{
	gstate->RegisterFunc("HARPY_Think",HARPY_Think);
	gstate->RegisterFunc("HARPY_Start_Pain",HARPY_Start_Pain);
	gstate->RegisterFunc("HARPY_Start_Die",HARPY_Start_Die);

	gstate->RegisterFunc("HARPY_Begin_Attack",HARPY_Begin_Attack);
	gstate->RegisterFunc("HARPY_Attack",HARPY_Attack);

}
