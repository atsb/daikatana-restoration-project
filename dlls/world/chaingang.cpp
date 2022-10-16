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

/* ***************************** define types ****************************** */
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */

void CHAINGANG_Start_Die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &destPoint );
void CHAINGANG_FX( userEntity_t *self);

/* **************************** Global Variables *************************** */
//extern float sin_tbl[];// SCG[1/23/00]: not used 
/* **************************** Global Functions *************************** */
//void AddTrackLight(userEntity_t *ent,float size,float r,float g,float b,int fxflags);// SCG[1/23/00]: not used 
//void AI_ParseEpairs2( userEntity_t *self );// SCG[1/23/00]: not used 
void AI_Chase( userEntity_t *self );
int AI_StartFly( userEntity_t *self );
//static float DeltaH;// SCG[1/23/00]: not used 
//static int CH_Start;

/* **************************** Extern Functions *************************** */

void AI_StartDie( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &destPoint );

/* ******************************* exports ********************************* */

DllExport void monster_chaingang(userEntity_t *self);



// ----------------------------------------------------------------------------
//
// Name:		CHAINGANG_Think
// Description:All internal pre-thinking stuff goes here or we can hook the
// pre-think function.  Either way it works.  In-Line think related handlers
// should go here.  Anything that needs to be thought of before all thinking 
// should be hooked through the prethink function.
// Input:self
// Output:<nothing>
// Note:
// 
// ----------------------------------------------------------------------------
void CHAINGANG_Think(userEntity_t *self)
{
// SCG[1/23/00]: not used
//	if(self->velocity.x > 1000.0f)
//		int poop = 1;

	AI_TaskThink(self);
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	
	AI_Dprintf("vX:%f vY:%f vZ:%x\n",self->velocity.x,self->velocity.y,self->velocity.z);

// SCG[1/23/00]: not used
//	if(self->velocity.x > 1000.0f)
//		int poop = 1;
	
	if(self->movetype == MOVETYPE_HOVER)
	{
		if(!AI_IsAlive(self))
		{
			self->gravity=1.5f;
			self->movetype = MOVETYPE_BOUNCE;
		}
		/*else
		{
			if(hook->acro_boost_time > 12.0f)
			{	
				hook->acro_boost_time = 0.0f;
			}
			self->velocity.z += 20.0*(sin_tbl[(int)hook->acro_boost_time]);
			hook->acro_boost_time +=1.0;
		}*/
	}
	self->nextthink = gstate->time + 0.1f;
}




// ----------------------------------------------------------------------------
//
// Name:		CHAINGANG_Think_Start
// Description: This function is setup for the very very start.  This is ONLY
// setup this way so the track entities for the Jet Stream will initialize 
// properly.
// 
// Input:self
// Output:<nothing>
// Note:
// 
// ----------------------------------------------------------------------------
void CHAINGANG_Think_Start(userEntity_t *self)
{
	if(self->movetype != MOVETYPE_WALK)
	{
		//Setup the Chaingang jetpack FX
		CHAINGANG_FX( self );
	}
	self->think = CHAINGANG_Think;
	self->nextthink = gstate->time + 0.1f;
}



int AI_HasTaskInQue( userEntity_t *self, TASKTYPE SearchTask );
int AI_HasGoalInQueue( userEntity_t *self, GOALTYPE SearchGoal );
// ----------------------------------------------------------------------------
//
// Name:		CHAINGANG_Attack_Think
// Description:Determines what tasks the CHAINGANG will do during its attack
// Input:self
// Output:<nothing>
// Note:
// 
// ----------------------------------------------------------------------------
void CHAINGANG_Attack_Think( userEntity_t *self )
{
	
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

	self->think = CHAINGANG_Think;

	self->curWeapon = gstate->InventoryFindItem (self->inventory, "chaingun");
	
	AI_SetOkToAttackFlag( hook, TRUE );
	//Generic stuff so far, there will be changes to this
	if ( AI_IsLineOfSight( self, self->enemy ) && !AI_HasTaskInQue(self,TASKTYPE_CHAINGANG_CHASE) )
	{
		AI_RemoveCurrentTask(self,TASKTYPE_CHAINGANG_CHASE,self, FALSE);
	}
	else if( !AI_HasTaskInQue(self,TASKTYPE_CHASE))
	{
		AI_RemoveCurrentTask(self,TASKTYPE_CHASE,self,FALSE);
	}
	//Setup next think time
	self->nextthink = gstate->time + 0.1f;

}

// ----------------------------------------------------------------------------
//
// Name:		CHAINGANG_DetermineMovementMode
// Description:Function to determine what movement mode the Chaingunner should 
// be in.
// Input: userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
int CHAINGANG_DetermineMovementMode(userEntity_t *self)
{
	frameData_t *pSequence = NULL;
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	float Player  = AI_Determine_Room_Height( self->enemy, 500, 0);
	float Monster = AI_Determine_Room_Height( self, 500, 0);
	
	if(self->movetype == MOVETYPE_HOVER)
	{
		
		//Has the player gone into a room that has a low ceiling?
		if((Player < T0_LAND_HEIGHT)&&(self->movetype == MOVETYPE_HOVER) && rnd() > 0.45f)
		{
			AI_Dprintf("Landing Mode!\n");
			float Floor = AI_Determine_Room_Height( self, 500, 1);
			
			if(!AI_Checkbelow_For_Liquid( self,15))
			{
			
				CVector NewV = self->s.origin;
				//Adjust above the floor
				NewV.z	= (self->s.origin.z - Floor)+50;

				MAPNODE_PTR NodeA = NODE_FindClosestNodeForLanding( pGroundNodes, NODETYPE_GROUND, NewV );
				
				if(NodeA)
				{
					GOAL_PTR pGoal = pGoalStack->pTopOfStack;
					GOAL_ClearTasks( pGoal );
					//AI_RemoveCurrentTask(self,TASKTYPE_CHAINGANG_INITIATE_GROUND,self, FALSE);
					CVector NodePOS = NodeA->position;
					NodePOS.z +=100;
					AI_AddNewTask(self, TASKTYPE_CHAINGANG_MOVETOLOCATION, NodePOS );
					AI_AddNewTask( self, TASKTYPE_CHAINGANG_INITIATE_GROUND);
					AI_AddNewTask( self, TASKTYPE_CHAINGANG_GROUND);
					AI_SetOkToAttackFlag( hook, FALSE );
					//We are still in the air keep the air animations.
					pSequence = FRAMES_GetSequence( self, "flya" );
					AI_ForceSequence( self, pSequence );
					//AI_Drop_Marker (self, NodePOS);
					return 1;
				}
			}
		}
	}
	else
	{
		//Test to see if we need to go back into flight
		if ( Player > TO_FLY_HEIGHT  && Monster > TO_FLY_HEIGHT && rnd() > 0.45f)
		{
			//First find the closest Air node to me
			//MAPNODE_PTR NodeAir = NODE_FindClosestNode( pAirNodes, NODETYPE_AIR, self->s.origin );
			//if(NodeAir)
			//{
				//Now find the closest Ground node to the Air Node.
			//	MAPNODE_PTR NodeG = NODE_FindClosestNode( pGroundNodes, NODETYPE_GROUND, NodeAir->position );
				//As long as there are no fuck ups... we should be off to find that ground node and then take off!				
			//	if(NodeG)
			//	{
					AI_Dprintf("To the AIR!\n");
					//GOAL_PTR pGoal = pGoalStack->pTopOfStack;
					//GOAL_ClearTasks( pGoal );
					//AI_AddNewTask( self, TASKTYPE_MOVETOLOCATION, NodeG->position );
					AI_RemoveCurrentTask(self,TASKTYPE_CHAINGANG_INITIATE_AIR,self, FALSE);
					AI_AddNewTask( self, TASKTYPE_CHAINGANG_AIR);
					AI_SetOkToAttackFlag( hook, FALSE );
					return 1;
			//	}
			//}
		}	
	}
	return 0;
}



// ----------------------------------------------------------------------------
//
// Name:		CHAINGANG_StartMoveToLocation
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_StartMoveToLocation( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

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


	AI_Dprintf( "Starting TASKTYPE_CHAINGANG_MOVETOLOCATION.\n" );

	self->nextthink	= gstate->time + 0.1f;

	AI_SetOkToAttackFlag( hook, TRUE );

	float fSpeed = AI_ComputeMovingSpeed( hook );
	
	float fTime = (fDistance / fSpeed) + 2.0f;
	
	AI_SetTaskFinishTime( hook, fTime );
	
	AI_SetMovingCounter( hook, 0 );
}


// ----------------------------------------------------------------------------
//
// Name:		CHAINGANG_MoveToLocation
// Description:Pretty much the same as
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_MoveToLocation (userEntity_t *self)
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	

	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );

	AI_Dprintf("Ch_MTOLOC FunctioN!\n");
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
		AI_Dprintf("We have finished our move-to location function!\n");
		AI_RemoveCurrentTask( self );
		return;
	}
	
	//Go do the movement if we are still far away from the destination
	//if ( AI_IsOkToMoveStraight( self, pAIData->destPoint, fXYDistance, fZDistance ) )
	//{
		//Comment:<nss>
		//We have checked to see if we can move straight without nodes...
		//send our entity structure, the destination, and a FALSE that tells the AI_FlyTowardPoint we are moving node 'free'.
	
	AI_MoveTowardPoint( self, pAIData->destPoint, FALSE, TRUE);
	
	//}
	/*else
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
					AI_Dprintf("Restarting Goal!\n");
					AI_RestartCurrentGoal( self );
				}
			}
		}
	}*/
	
	
	CVector dir = pAIData->destPoint - self->s.origin;
	dir.Normalize();
	self->s.angles.yaw = VectorToYaw(dir);	
	self->s.angles.pitch = 0.0f;

}


// ----------------------------------------------------------------------------
//
// Name:		CHAINGANG_StartDodge
// Description:Sets up a good point to go to or removes the task if no point can 
// be found at that position
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_StartDodge( userEntity_t *self)
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
			//AI_Dprintf("Dropped Maker!\n");
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
// Name:		CHAINGANG_Dodge
// Description:Calls standard AI_Dodge
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_Dodge( userEntity_t *self)
{
	AI_Dodge( self );
}





// ----------------------------------------------------------------------------
//
// Name:		CHAINGANG_StartChase
// Description:The Setup for the chase thinking for Mr. Chaingunner
// Input: userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_StartChase( userEntity_t *self)
{

	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !AI_CanMove( hook ) )
	{
		self->nextthink	= gstate->time + 0.1f;
		AI_RemoveCurrentTask( self );
		return;
	}
	if(self->movetype == MOVETYPE_HOVER)
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

	if(self->movetype == MOVETYPE_HOVER)
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
// Name:		CHAINGANG_Chase
// Description:The chase thinking for Mr. Chaingunner
// Input: userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_Chase( userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	//Check for landing every 3rd pass
	if(CHAINGANG_DetermineMovementMode(self))
	//if(CHAINGANG_DetermineMovementMode(self) && (hook->nMovingCounter % 3))
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
		GOAL_PTR pGoal = pGoalStack->pTopOfStack;
		GOAL_ClearTasks( pGoal );
		
		AI_AddNewTask( self, TASKTYPE_CHAINGANG_ATTACK );
		self->nextthink	= gstate->time + 0.1f;
		return;
	}
	

	if((AI_IsInWater(self->enemy)||AI_IsInWater(self)) && self->movetype == MOVETYPE_HOVER)
	{
		AI_RemoveCurrentTask(self,TASKTYPE_WANDER,self, FALSE);
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
// Name:		CHAINGANG_Set_Attack_Seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_Set_Attack_Seq( userEntity_t *self )
{
    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {
//	    playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	    frameData_t *pSequence = NULL;
	    if ( self->movetype == MOVETYPE_HOVER )
	    {
		    pSequence = FRAMES_GetSequence( self, "atakf" );
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
// Name:		CHAINGANG_DetermineAttackMode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_DetermineAttackMode( userEntity_t *self )
{
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
// Name:		CHAINGANG_ModifyStrafeDirectionAfterCollision
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_ModifyStrafeDirectionAfterCollision( userEntity_t *self )
{
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
// Name:		CHAINGANG_HandleCollision
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_HandleCollision( userEntity_t *self, float fSpeed, CVector &forwardDirection )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	float fDist = fSpeed  * 0.1f;
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
			
			CHAINGANG_ModifyStrafeDirectionAfterCollision( self );
			break;
		}
		case TER_WALL:
		{
			CVector dir;
			AI_ComputeAlongWallVector( self, forwardDirection, terrain.wall_normal, dir );
			forwardDirection = dir;

			CHAINGANG_ModifyStrafeDirectionAfterCollision( self );
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
			
			CHAINGANG_ModifyStrafeDirectionAfterCollision( self );
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
			
			CHAINGANG_ModifyStrafeDirectionAfterCollision( self );
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
// Name:		CHAINGANG_StartInitiateAir
// Description:Setup for the Ground Sequence.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_StartInitiateAir( userEntity_t *self )
{
	frameData_t *pSequence = NULL;
	pSequence = FRAMES_GetSequence( self, "flyb" );
	AI_ForceSequence( self, pSequence );
	
}


// ----------------------------------------------------------------------------
// <nss>
// Name:		CHAINGANG_InitiateAir
// Description:Setup for the Ground Sequence.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_InitiateAir( userEntity_t *self )
{
	//AI_Dprintf("!!frame:%d\n",self->s.frame);
	
	if(self->s.frame >= 293 && self->s.frame <= 294)
	{
		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e4/m_chgangjetsrta.wav"),0.75f, ATTN_NORM_MIN, ATTN_NORM_MAX );
	}
	if( self->s.frame >= 299)
	{
		AI_RemoveCurrentTask(self,FALSE);
	}
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		CHAINGANG_StartAir
// Description:Setup for the Ground Sequence.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_StartAir( userEntity_t *self )
{
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
	
	CVector SmokeandShit = self->s.origin;
	
	SmokeandShit.z +=25.0;
	//Fireblast FX
	gstate->WriteByte(SVC_TEMP_ENTITY);
	gstate->WriteByte(TE_EXPLOSION_SMALL2);
	gstate->WritePosition(SmokeandShit);
	gstate->MultiCast(SmokeandShit,MULTICAST_PVS);
	
	// Smoke FX
	CVector color;
	color.Set( 1, 1, 1 );
    SmokeandShit.x +=15;
	gstate->particle (TE_PARTICLE_RISE, SmokeandShit, up, 6, color, 7, PARTICLE_SMOKE|PARTICLE_RANDOM_SPREAD);
    SmokeandShit = self->s.origin;
	SmokeandShit.y +=15;
	gstate->particle (TE_PARTICLE_RISE, SmokeandShit, up, 4, color, 7, PARTICLE_SMOKE|PARTICLE_RANDOM_SPREAD);
    SmokeandShit = self->s.origin;
	SmokeandShit.x -=15;
	gstate->particle (TE_PARTICLE_RISE, SmokeandShit, up, 8, color, 7, PARTICLE_SMOKE|PARTICLE_RANDOM_SPREAD);
    SmokeandShit = self->s.origin;
	SmokeandShit.y -=15;
	gstate->particle (TE_PARTICLE_RISE, SmokeandShit, up, 5, color, 7, PARTICLE_SMOKE|PARTICLE_RANDOM_SPREAD);


	//Setup movement type
	//Attach smoke and light for jetpack
	hook->nAttackType = ATTACK_AIR_RANGED;
	Terrain_Node_Change(self, MOVETYPE_HOVER);
	CHAINGANG_FX( self );

	
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		CHAINGANG_Air
// Description:This function will make the CHAINGANG member ignite his jetpack 
// and will get him ready for Air attacks.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_Air( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );

	if((pAIData->fValue + 0.75f) < gstate->time)
	{
		AI_Dprintf("Air Mode Activated!\n");

		AI_RemoveCurrentTask(self,FALSE);
		
		AI_SetOkToAttackFlag( hook, TRUE );
		AI_StartSequence(self, "flya", FRAME_LOOP);
		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e4/m_chgangflya.wav"),0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );

	}
	self->nextthink	= gstate->time + 0.1f;
}


// ----------------------------------------------------------------------------
// <nss>
// Name:		CHAINGANG_StartInitiateGround
// Description:Setup for the Ground Sequence.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_StartInitiateGround( userEntity_t *self )
{
	frameData_t *pSequence = NULL;
	pSequence = FRAMES_GetSequence( self, "flyc" );
	AI_ForceSequence( self, pSequence );
	gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e4/m_chgangjetland.wav"),0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );
}


// ----------------------------------------------------------------------------
// <nss>
// Name:	CHAINGANG_InitiateGround
// Description:Setup for the Ground Sequence.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_InitiateGround( userEntity_t *self )
{

	//AI_Dprintf("@@frame:%d\n",self->s.frame);
	
	if(self->s.frame >=316)
	{
		
		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e4/m_chgangjetland.wav"),0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );
		AI_RemoveCurrentTask(self,FALSE);
	}
}


// ----------------------------------------------------------------------------
// <nss>
// Name:		CHAINGANG_StartGround
// Description:Setup for the Ground Sequence.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_StartGround( userEntity_t *self )
{

	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );


	CVector LandSpot,DumpSpot;

	DumpSpot = self->s.origin;
	DumpSpot.z -= 5000.0f;

	tr = gstate->TraceLine_q2( self->s.origin, DumpSpot, self, self->clipmask );
	if(tr.fraction < 1.0f)
	{
		LandSpot = self->s.origin;
		LandSpot.z -= tr.fraction * 5000.0f;
	}
	else
	{
		AI_RemoveCurrentTask(self);
	}
	
	MAPNODE_PTR Node = 	NODE_FindClosestNode( pGroundNodes, NODETYPE_GROUND, LandSpot );

	pAIData->destPoint = Node->position;
	
	self->s.angles.pitch = 0;
	self->s.angles.roll = 0;

	self->velocity.Zero();
	self->velocity.z = 35;

	
	AI_SetOkToAttackFlag( hook, FALSE );
	
	pAIData->fValue = gstate->time;

	AI_SetTaskFinishTime( hook, -1 );

	CVector SmokeandShit = self->s.origin;
	
	SmokeandShit = self->s.origin;
	// Smoke FX
	CVector color;
	color.Set( 1, 1, 1 );
    SmokeandShit.x +=7;
	gstate->particle (TE_PARTICLE_RISE, SmokeandShit, up, 6, color, 7, PARTICLE_SMOKE|PARTICLE_RANDOM_SPREAD);
    SmokeandShit = self->s.origin;
	SmokeandShit.y +=7;
	gstate->particle (TE_PARTICLE_RISE, SmokeandShit, up, 4, color, 7, PARTICLE_SMOKE|PARTICLE_RANDOM_SPREAD);
    SmokeandShit = self->s.origin;
	SmokeandShit.x -=7;
	gstate->particle (TE_PARTICLE_RISE, SmokeandShit, up, 8, color, 7, PARTICLE_SMOKE|PARTICLE_RANDOM_SPREAD);
    SmokeandShit = self->s.origin;
	SmokeandShit.y -=7;
	gstate->particle (TE_PARTICLE_RISE, SmokeandShit, up, 5, color, 7, PARTICLE_SMOKE|PARTICLE_RANDOM_SPREAD);

	
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		CHAINGANG_Ground
// Description:This function will land the CHAINGANG and get him ready for Ground
// attacks.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_Ground( userEntity_t *self )
{

	userEntity_t *attacker = self->enemy;
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	self->s.angles.pitch = 0;
	self->s.angles.roll = 0;

	//Once we have reached landing range and our hovera animation has reached the last sequence
	//kill the ground task, set our move type to walk, and run the landing sequence.
	if( pAIData->fValue + 0.50f < gstate->time)
	{
		hook->nAttackType = ATTACK_GROUND_RANGED;
		Terrain_Node_Change(self, MOVETYPE_WALK);
		AI_Dprintf("Ground Mode Activated!\n");
		AI_RemoveCurrentTask(self,FALSE);
		AI_SetOkToAttackFlag( hook, TRUE );
		return;
	}
}



// ----------------------------------------------------------------------------
//
// Name:		CHAINGANG_UpdateAttackMovement
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_UpdateAttackMovement( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
//	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );	
	AI_GetCurrentGoalStack( hook );	// SCG[1/23/00]: pGoalStack not used
	userEntity_t *enemy = self->enemy;

	if(self->movetype == MOVETYPE_HOVER)
	{
		if ( hook->nAttackMode == ATTACKMODE_NORMAL )
		{
			if ( hook->strafe_time <= gstate->time )
			{
				hook->strafe_time = gstate->time + (rnd() * (640 / hook->attack_speed)) + 1.5f;
				hook->strafe_dir = rand() % (STRAFE_DOWNRIGHT+1);

				CHAINGANG_DetermineAttackMode( self );
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
			CHAINGANG_HandleCollision( self, fSpeed, vectorTowardPoint );
			
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
			CHAINGANG_HandleCollision( self, fSpeed, vectorTowardPoint );

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
// Name:		CHAINGANG_Begin_Attack
// Description: Start Attack sequence
// Input: userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_Begin_Attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );
	frameData_t *pSequence = NULL;
	if ( self->movetype == MOVETYPE_HOVER )
	{
		//Ready Weapon for Air
		pSequence = FRAMES_GetSequence( self, "transb" );
	}
	else
	{
		//Ready Weapon for Ground
		pSequence = FRAMES_GetSequence( self, "transd" );
	}
	hook->nTargetFrequency = 32.0f;
    AI_ForceSequence( self, pSequence );
	AI_SetTaskFinishTime( hook, -1 );
	pAIData->nValue = 0;
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_ZeroVelocity( self );
	
}

// ----------------------------------------------------------------------------
//
// Name:		CHAINGANG_Attack
// Description:Standard Attack function for Chaingang member
// Input:userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_Attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );

	if( !self->enemy || AI_IsEnemyDead( self ) )
		return;

	AI_FaceTowardPoint( self, self->enemy->s.origin );
	int End = AI_IsEndAnimation( self );

	if((AI_IsInWater(self->enemy)||AI_IsInWater(self)) && self->movetype == MOVETYPE_HOVER)
	{
		AI_RemoveCurrentTask(self,TASKTYPE_WANDER,self, FALSE);
		return;
	}
	if (AI_CanMove( hook ) )
	{
		//if we are in the air
		if ( AI_IsInAir( self ))
			AI_UpdatePitchTowardEnemy( self );
		//Update for Attack movement
		//Also checks for the air ground transition
// SCG[1/23/00]: not used 
//		if(self->velocity.x > 1000.0f)
//			int poop = 1;

		CHAINGANG_UpdateAttackMovement( self );
		if(self->velocity.x > 1000.0f)
		{
			CVector Dir;
			Dir = self->velocity;
			Dir.Normalize();
			Dir = Dir * hook->run_speed;
			self->velocity = Dir;
		}
	}

	if( End && pAIData->nValue == 1 && self->movetype == MOVETYPE_WALK)
	{
			if(CHAINGANG_DetermineMovementMode(self))
			{
				AI_RemoveCurrentTask(self,TRUE);
				return;
			}
			AI_RemoveCurrentTask( self, TASKTYPE_CHAINGANG_DODGE, self, TRUE );
			AI_SetOkToAttackFlag( hook, FALSE );
			//AI_AddNewTask( self, TASKTYPE_CHAINGANG_ATTACK);
			return;
	}	

	if ( End && !AI_IsEnemyDead( self ) )
	{
		float fDist = VectorDistance(self->enemy->s.origin, self->s.origin);
		if ( !AI_IsWithinAttackDistance( self, fDist ) || !com->Visible(self, self->enemy))
		{
			AI_RemoveCurrentTask( self );
			return;
		}
		else
		{
			CHAINGANG_Set_Attack_Seq(self);
			pAIData->nValue = 1;
			AI_PlayAttackSounds( self );
			//gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("sounds/e4/m_chgangataka.wav"), 1.0f, 500, 1024);
		}

	}
	int P = AI_IsReadyToAttack1( self ) & AI_IsFacingEnemy( self, self->enemy ); 

	if ( (P || self->hacks_int > 0) && pAIData->nValue == 1)
	{
		if(!AI_DetectForLikeMonsters( self ))
		{
			
			int Da = self->hacks_int;
			if( Da % 3)
			{
				
				ai_fire_curWeapon( self );
			}
			if( Da % 7)
			{
				AI_PlayAttackSounds( self );
			}
			self->hacks_int++;
			if(self->hacks_int > 22)
				self->hacks_int = 0;
		}
		else
		{
			AI_RemoveCurrentTask( self, TASKTYPE_CHAINGANG_DODGE, self, FALSE );
			AI_AddNewTask( self, TASKTYPE_CHAINGANG_ATTACK);
			self->hacks_int = 0;
			pAIData->nValue = 0;
			return;
		}
	}



}

// ----------------------------------------------------------------------------
//
// Name:		CHAINGANG_Start_Die
// Description: die handler for chain gang member
// Input:userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
// int damage, CVector &destPoint 
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_Start_Die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                          int damage, CVector &destPoint )
{
	int notdeadyet = (self->deadflag == DEAD_NO);

	// call AI_StartDie, which does generic dying stuff
	AI_StartDie( self, inflictor, attacker, damage, destPoint );

    if ( notdeadyet && self->deadflag == DEAD_DYING )
	{ // pick a different sequence for different deaths
		float chance = rnd();

		if ( chance > 0.33f )
		{
			AI_ForceSequence( self, "diea" ); // fall over
		}
		else if ( chance > 0.666f )
		{
			AI_ForceSequence( self, "dieb" ); // collapse to the left side
		}
		else
		{
			AI_ForceSequence( self, "diec" ); // fall over
		}
	}
}


// ----------------------------------------------------------------------------
//
// Name:		CHAINGANG_FX_Think
// Description: This is the think function for the smoke
// It is also a real good example of rotating objects around an entity.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_FX_Think( userEntity_t *self, float x, float y, float z )
{

	CVector N_Origin,fAng,forward,right,up, my_forward,my_right,my_up;

	N_Origin = self->owner->s.origin;

	N_Origin.x = x;
	N_Origin.y = y;
	N_Origin.z = z;

	fAng = self->owner->s.angles;

	fAng.AngleToVectors(forward, right, up);
	
	//right = -right;

	// change matrix orientation
	my_forward.x = forward.x; 
	my_forward.y = right.x;  
	my_forward.z = up.x;
	my_right.x = forward.y; 
	my_right.y = right.y; 
	my_right.z = up.y;
	my_up.x = forward.z; 
	my_up.y = right.z; 
	my_up.z = up.z;

	self->s.origin.x = DotProduct( N_Origin, my_forward) + self->owner->s.origin.x;
	self->s.origin.y = DotProduct( N_Origin, my_right ) + self->owner->s.origin.y;
	self->s.origin.z = DotProduct( N_Origin, my_up ) + self->owner->s.origin.z;
	
	
	self->s.angles = self->owner->s.angles;
	self->nextthink = gstate->time + 0.01f;

	//If the Chaingunner dies or goes into walk mode let's "turn off" the jets.
	if( !AI_IsAlive(self->owner) || (self->owner->movetype == MOVETYPE_WALK))
	{
		RELIABLE_UNTRACK(self);
		self->remove (self);
	}


}



// ----------------------------------------------------------------------------
//
// Name:		CHAINGANG_FX_Smoke1
// Description: This is the think function for the smoke1
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_FX_Smoke1( userEntity_t *self)
{

		CHAINGANG_FX_Think( self, -10.0, 0.0, 18.5 );
}

// ----------------------------------------------------------------------------
//
// Name:		CHAINGANG_FX_Light
// Description: This is the think function for the red light behind the Chaingang
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_FX_Light(userEntity_t *self)
{
	CHAINGANG_FX_Think( self, -40.0, 0.5, 20.5 );
}

// ----------------------------------------------------------------------------
//
// Name:		CHAINGANG_FX
// Description: This sets up the smoke trail and muzzle flash for the flying
// sequence of the Flying Chaingunner.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_FX( userEntity_t *self)
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used 

	///////////////////////////////////////////////////////////////////////////
	//	Special Effects for Hovering										 //
	///////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////
	///////////////(Smoke  I)////////////////////
	/////////////////////////////////////////////
	userEntity_t	*temp;
	CVector			Goal, N_Origin, dir, ang, p_org;

	temp = gstate->SpawnEntity ();

	temp->owner = self;
	temp->solid = SOLID_NOT;
	temp->movetype = MOVETYPE_NONE;
	temp->className = "drop_sprite";

	temp->s.renderfx = RF_TRACKENT;

	temp->s.modelindex = gstate->ModelIndex ("models/global/e2_firea.sp2");
	
	gstate->SetSize(temp, -1, -1, -1, 1, 1, 1);
	temp->s.render_scale.Set( -0.01, -0.01, -0.01);
	temp->s.frame = 0;
	
	N_Origin = self->s.origin;

	gstate->SetOrigin2( temp, N_Origin );
	temp->s.angles.roll = 180.0f;

	temp->think		= CHAINGANG_FX_Smoke1;


	trackInfo_t tinfo;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=temp;
    tinfo.srcent=temp;
    tinfo.fru.Zero();
    //Flags to tell what settings we have changed
	tinfo.flags = TEF_FXFLAGS|TEF_SRCINDEX;
	//Flags to tell what FX to add to the track ent
	tinfo.fxflags = TEF_JETTRAIL;
    
	com->trackEntity(&tinfo,MULTICAST_ALL);

	gstate->LinkEntity(temp);
	temp->nextthink = gstate->time + 0.01f;

	/////////////////////////////////////////////
	///////////////(Red Light)///////////////////
	/////////////////////////////////////////////
	temp = gstate->SpawnEntity ();

	temp->owner = self;
	temp->solid = SOLID_NOT;
	temp->movetype = MOVETYPE_NONE;
	temp->className = "drop_sprite";
	temp->s.effects |= EF_DIMLIGHT;
	temp->s.color.x = 0.50f; // R
	temp->s.color.y = 0.25f; // G
	temp->s.color.z = 0.25f; // B

	temp->s.modelindex = gstate->ModelIndex ("models/global/e2_firea.sp2");
	
	gstate->SetSize(temp, -1, -1, -1, 1, 1, 1);
	temp->s.render_scale.Set( -0.001, -0.001, -0.001);
	temp->s.frame = 0;
	temp->s.alpha = 0.0;

	N_Origin = self->s.origin;
	
	gstate->SetOrigin2( temp, N_Origin );
	
	temp->think		= CHAINGANG_FX_Light;
	gstate->LinkEntity(temp);
	temp->nextthink = gstate->time + 0.01f;

	
}

// ----------------------------------------------------------------------------
//
// Name:		CHAINGANG_ParseEpairs
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CHAINGANG_ParseEpairs( userEntity_t *self )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used 
	AI_InitNodeList( self );
	Terrain_Node_Change(self, self->movetype );
	AI_ParseEpairs(self);

	self->think = CHAINGANG_Think_Start;
	self->nextthink	= gstate->time + 2.0f;
}


// ----------------------------------------------------------------------------
//
// Name:		monster_chaingang
// Description:
//	creates the superfly entity and sets him up to smack down
//	everything that gets in his way.
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_chaingang( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_CHAINGANG );

	self->className			= "monster_chaingang";
	self->netname			= tongue_monsters[T_MONSTER_CHAINGANG];

	char *szModelName = AIATTRIBUTE_GetModelName( self->className );
    _ASSERTE( szModelName );
    self->s.modelindex = gstate->ModelIndex( szModelName );
	self->s.modelindex3 = gstate->ModelIndex("models/global/me_mflash.dkm");

	if ( !ai_get_sequences(self) )
	{
		char *szCSVFileName = AIATTRIBUTE_GetCSVFileName( self->className );
        _ASSERTE( szCSVFileName );
        FRAMEDATA_ReadFile( szCSVFileName, self );
	}
	ai_register_sounds(self);

	///////////////////////////////////////////////////////////////////////////
	//	give chaingunner his chaingun
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
										  "chaingun", 
										  chaingun_fire, 
										  ITF_BULLET );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}
	
	//Setup standard function pointers.
	hook->fnStartAttackFunc = CHAINGANG_Begin_Attack;
	hook->fnAttackFunc		= CHAINGANG_Attack;

	self->die               = CHAINGANG_Start_Die;
	self->think				= CHAINGANG_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;
	
	//Setup Movetypes and attack types
	if(AI_Determine_Room_Height( self, 500, 1)<100)
		self->movetype			= MOVETYPE_WALK;
	else
		self->movetype			= MOVETYPE_HOVER;
	hook->nAttackType = ATTACK_AIR_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	hook->strafe_time = gstate->time;
	hook->strafe_dir  = STRAFE_LEFT;


	AI_DetermineMovingEnvironment( self );

	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;


	gstate->LinkEntity(self);

	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

	// NSS[2/22/00]:Cache all of these sounds..
	gstate->SoundIndex("e4/m_chgangjetland.wav");
	gstate->SoundIndex("e4/m_chgangjetsrta.wav");
	gstate->SoundIndex("e4/m_chgangflya.wav");
//	gstate->SoundIndex("e4/m_chgangland.wav"); KRH[3/01/00] Does not exist.
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

#if 0
#endif 0


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_chaingang_register_func()
{
	gstate->RegisterFunc("CHAINGANG_Think",CHAINGANG_Think);
	gstate->RegisterFunc("CHAINGANG_FX_Smoke1",CHAINGANG_FX_Smoke1);
	gstate->RegisterFunc("CHAINGANG_FX_Light",CHAINGANG_FX_Light);
	gstate->RegisterFunc("CHAINGANG_Think_Start",CHAINGANG_Think_Start);
	gstate->RegisterFunc("CHAINGANG_ParseEpairs",CHAINGANG_ParseEpairs);
	gstate->RegisterFunc("CHAINGANG_Start_Die",CHAINGANG_Start_Die);

	gstate->RegisterFunc("CHAINGANG_Begin_Attack",CHAINGANG_Begin_Attack);
	gstate->RegisterFunc("CHAINGANG_Attack",CHAINGANG_Attack);
}
