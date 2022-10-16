// ==========================================================================
//
//  File:Griffon "To fly or not to fly!"
//  Contents:The brain of the nifty griffon monster!
//  Author:N.S.S.
//
// ==========================================================================
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
#include "ai_common.h"



/* ***************************** define types ****************************** */
#define HOVER_DISTANCE			70.0f
#define COLLISION_TIME			2
#define FLY_AWAY_DISTANCE		250.0f


/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
void AI_OrientToFloor( userEntity_t *self );
//void AI_StartChase( userEntity_t *self );// SCG[1/23/00]: not used
void AI_Chase( userEntity_t *self );
int AI_StartFly( userEntity_t *self );
int AI_HandleCollisionWithEntities( userEntity_t *self, CVector &destPoint, float fSpeed );
int AI_HandleGroundObstacle( userEntity_t *self, float fSpeed, int bMovingTowardNode );
int AI_HandleGettingStuck( userEntity_t *self, CVector &destPoint, float fSpeed );

/* ******************************* exports ********************************* */
DllExport void monster_griffon( userEntity_t *self );


// ----------------------------------------------------------------------------
//
// Name:		Griffon_Think
// Description:All internal pre-thinking stuff goes here
// Input:self
// Output:NA
// Note: 
// I put a hook in the griffon think (instead of standard AI_TASKTHINK) for several
// reasons, the first is that it is easier to debug when you have multiple entities,
// the second is that there are some things you might need to do before the AI_TASKTHINK
// ----------------------------------------------------------------------------
void Griffon_Think(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( pGoalStack );
	GOALTYPE nGoalType = GOALSTACK_GetCurrentGoalType( pGoalStack );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	if(nGoalType != GOALTYPE_KILLENEMY && self->movetype == MOVETYPE_WALK)
	{
		AI_ForceSequence(self,"flya",FRAME_LOOP);
		Terrain_Node_Change(self, MOVETYPE_FLY);
		//self->s.origin.z += 20;
		self->nextthink = gstate->time + 0.1f;
		AI_RemoveCurrentTask(self,FALSE);
		return;
	}
	if(pCurrentTask)
	{
		if(self->deadflag != DEAD_NO && pCurrentTask->nTaskType != TASKTYPE_DIE)
		{
			AI_RemoveCurrentTask(self,TASKTYPE_DIE, self, FALSE);
		}
	}
	AI_TaskThink(self);
	if(self->movetype != MOVETYPE_FLY)
		AI_Dprintf("NOT FLYING!\n");
	if(self->velocity.z < 0.0f)
		AI_Dprintf("Velocity is negative!\n");
}


// ----------------------------------------------------------------------------
//
// Name:		GRIFFON_StartPain
// Description:Sets up the pain sequences for the Griffon
// Input:self
// Output:Nothing
// Note: 
// This function intercepts the normal AI Startpain function and figures out if
// the Griffon is flying or not.  If so do the fly pain animations if not then
// do the walking pain animations.
// ----------------------------------------------------------------------------
void GRIFFON_StartPain(userEntity_t *self, userEntity_t *attacker, float kick, int damage)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if(hook)
	{
		if((hook->pain_finished < gstate->time) && rnd() < 0.15f)
		{
			if (self->movetype == MOVETYPE_FLY)
			{
				AI_ForceSequence(self,"hita");
			}
			else
			{
				AI_ForceSequence(self,"hitb");
			}
			hook->pain_finished = gstate->time + (self->s.frameInfo.endFrame - self->s.frameInfo.startFrame) * self->s.frameInfo.frameTime;
			if(pGoalStack)
			{
				if(pGoalStack->pTopOfStack)
				{
					GOAL_ClearTasks( pGoalStack->pTopOfStack );
				}
			}
		}
	}

	AI_StartPain(self,attacker,kick,damage);
}



// ----------------------------------------------------------------------------
//
// Name:		GRIFFON_State_Change
// Description:States which change on the GRIFFON.
// Input:self
// Output:will return the current health
// Note: 
// This function should be called from the main loop of any task that could
// potentially run for longer than a second.  It will change any state of the
// monster which is directly related to the condition of the monster.
// ----------------------------------------------------------------------------
float GRIFFON_State_Change(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	//Get the % of health loss
	float P_health_loss = 100*(self->health/hook->base_health);

	if( P_health_loss <= (self->health*0.50f))
	{
		//Put in something based off of health loss.
	}
	return P_health_loss;
}


// ----------------------------------------------------------------------------
//
// Name:		GRIFFON_Start_Attack_Think
// Description:Initialize the GRIFFON think states
// Input:self
// Output:<nothing>
// Note:
// ----------------------------------------------------------------------------
void GRIFFON_Start_Attack_Think( userEntity_t *self )
{
	//Nothing needs to be initialized as of yet
}

// ----------------------------------------------------------------------------
//
// Name:		GRIFFON_Attack_Think
// Description:Determines what tasks the Griffon will do during its attack
// Input:self
// Output:<nothing>
// Note:
// 
// ----------------------------------------------------------------------------
void GRIFFON_Attack_Think( userEntity_t *self )
{
	
	//Attack states are no yet implemented.  So far all choices are based off of health and
	//random values.
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );
	GOAL_PTR pGoal = pGoalStack->pTopOfStack;
	
	if ( !self->enemy )
	{
		self->enemy = pAIData->pEntity;
	}

	//Debug the Griffon's think sequence
	//simple trap for AI_Think
	self->think = Griffon_Think;
	self->curWeapon = gstate->InventoryFindItem (self->inventory, "punch");
	
	if ( AI_IsLineOfSight( self, self->enemy ) )
	{
		AI_RemoveCurrentTask(self,TASKTYPE_GRIFFON_CHASE,self, FALSE);
	}
	else // Go into Generic Path Finding Mode  We always fly to the player if we cannot see him!
	{
		PATHLIST_KillPath( hook->pPathList );
		GOAL_ClearTasks( pGoal );
		hook->nAttackType = ATTACK_AIR_SWOOP_MELEE;
		Terrain_Node_Change(self, MOVETYPE_FLY);
		AI_RemoveCurrentTask(self,TASKTYPE_GRIFFON_CHASE, self, FALSE);
		AI_Dprintf("Attack Think Fly Activate!\n");
	}
	//Setup next think time
	self->nextthink = gstate->time + 0.1f;

}


// ----------------------------------------------------------------------------
// <nss>
// Name:		GRIFFON_StartLand
// Description:Begin the animations for landing
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void GRIFFON_StartLand( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_ForceSequence(self, "drop", FRAME_ONCE);
	gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e2/m_griffondrop.wav"), 0.85f, hook->fMinAttenuation, hook->fMaxAttenuation);
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1 );

}

// ----------------------------------------------------------------------------
// <nss>
// Name:		GRIFFON_Land
// Description:Once animations are complete change terrain type
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void GRIFFON_Land( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	if(AI_IsEndAnimation( self ))
	{
		AI_SetOkToAttackFlag( hook, TRUE );
		Terrain_Node_Change(self, MOVETYPE_WALK);
		AI_RemoveCurrentTask(self,FALSE);
	}
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		GRIFFON_StartGround
// Description:Setup for the Ground Sequence.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void GRIFFON_StartGround( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );
	
	MAPNODE_PTR Node = 	NODE_FindClosestNode( pGroundNodes, NODETYPE_GROUND, self->s.origin );

	if(Node)
	{
		pAIData->destPoint = Node->position;
	
		AI_ForceSequence(self, "flya", FRAME_LOOP);

		self->s.angles.pitch = 0;
		self->s.angles.roll = 0;
	
		AI_SetOkToAttackFlag( hook, FALSE );

		AI_SetTaskFinishTime( hook, -1 );
	}
	else
	{
		AI_Dprintf("No Nodes!  Something is fucked!\n");
		AI_RemoveCurrentTask(self);
	}
}
// ----------------------------------------------------------------------------
// <nss>
// Name:		GRIFFON_Ground
// Description:This function will land the Griffon and get him ready for Ground
// attacks.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void GRIFFON_Ground( userEntity_t *self )
{
	userEntity_t *attacker = self->enemy;
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	float fDistance = VectorDistance( self->s.origin, pAIData->destPoint );

	pAIData->fValue		= 0.45f;

	//Check to see if we are stuck.
	if(AI_FLY_Debug_Origin(self))
	{
		pAIData->nValue++;
	}
	
	AI_FlyTowardPoint2( self, pAIData->destPoint, pAIData->fValue);

	self->s.angles.pitch = 0;
	self->s.angles.roll = 0;

	//Once we have reached landing range and our hovera animation has reached the last sequence
	//kill the ground task, set our move type to walk, and run the landing sequence.
	if( ( fDistance  <= 100 ) || pAIData->nValue > COLLISION_TIME+5)
	{
		AI_Dprintf("Adding Land Task!\n");
		AI_RemoveCurrentTask(self,TASKTYPE_GRIFFON_LAND,self,FALSE);
	}
	//Set our last origin to help with collision detection
	hook->last_origin = self->s.origin;

}



// ----------------------------------------------------------------------------
// <nss>
// Name:		GRIFFON_StartChase
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GRIFFON_StartChase( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if ( AI_IsLineOfSight( self, self->enemy ) )
	{
		AI_SetOkToAttackFlag( hook, TRUE );
		if(self->movetype == MOVETYPE_FLY)
		{
			AI_ForceSequence(self, "flya", FRAME_LOOP);
		}
		else
		{
			AI_ForceSequence(self, "runa", FRAME_LOOP);
		}

		//If we don't have a pointer to the enemy let's exit out
		if ( !self->enemy )
		{
			AI_RemoveCurrentTask(self,FALSE);
			return;
		}

		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
		AIDATA_PTR pAIDataT = TASK_GetData(pCurrentTask);

		hook->nMoveCounter = 0;
		//Trap to check and see if we are stuck
		pAIDataT->nValue = 0;


		AI_SetOkToAttackFlag( hook, FALSE );
		//Never give up this task! Er.. unless we just can't see the player... :)
		AI_SetTaskFinishTime( hook, -1 );
		//AI_SetMovingCounter( hook, 0 );
		
		self->nextthink	= gstate->time + 0.1f;
		//AI_Dprintf("Start Chase!!\n");
	}
	else
	{
		playerHook_t *hook = AI_GetPlayerHook( self );

		if ( !AI_CanMove( hook ) )
		{
			AI_RemoveCurrentTask( self );
			return;
		}
		if(self->movetype == MOVETYPE_FLY)
		{
			AI_ForceSequence(self, "flya", FRAME_LOOP);
		}
		else
		{
			AI_ForceSequence(self, "runa", FRAME_LOOP);
		}
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

		if ( !self->enemy )
		{
			GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
			AIDATA_PTR pAIData = GOAL_GetData(pCurrentGoal);
			self->enemy = pAIData->pEntity;
		}

//		float fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );// SCG[1/23/00]: not used

		if(self->movetype == MOVETYPE_FLY)
		{
			AI_ForceSequence(self,"flya",FALSE);
		}
		else
		{
			AI_ForceSequence(self,"runa",FALSE);
		}

		self->nextthink	= gstate->time + 0.1f;

		AI_SetOkToAttackFlag( hook, FALSE );

		AI_SetTaskFinishTime( hook, 10.0f );
		AI_SetMovingCounter( hook, 0 );
	}

}


// ----------------------------------------------------------------------------
// <nss>
// Name:		GRIFFON_Chase
// Description: Chase code for both ground and air while enemy is visible.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void GRIFFON_Chase( userEntity_t *self )
{
	
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	userEntity_t *pEnemy = self->enemy;
	float fDistance = VectorDistance( self->s.origin, pEnemy->s.origin );

	
//	float P_health_loss = GRIFFON_State_Change(self);	// SCG[1/23/00]: not used
	
	if(self->movetype == MOVETYPE_BOUNCE)
	{
		self->movetype = MOVETYPE_WALK;
	}
	//enemy is above or below us by 100 units and
	//we are in walk mode let's fly to the enemy!
	if(((self->enemy->s.origin.z > self->s.origin.z+150)||(self->enemy->s.origin.z < self->s.origin.z-150))&&(self->movetype == MOVETYPE_WALK))
	{
		//AI_Dprintf("Player is above or below me, let's fly to him!\n");
		GOAL_PTR pGoal = pGoalStack->pTopOfStack;
		GOAL_ClearTasks( pGoal );
		hook->nAttackType = ATTACK_AIR_SWOOP_MELEE;
		Terrain_Node_Change(self, MOVETYPE_FLY);
		//self->s.origin.z += 80;
		AI_RemoveCurrentTask(self,FALSE);
		AI_Dprintf("Chase Fly Activate!\n");
		return;
	}

	//Check for Water contents
	if((AI_IsInWater(self->enemy)||AI_IsInWater(self)) && self->movetype == MOVETYPE_FLY)
	{
		AI_RemoveCurrentTask(self,TASKTYPE_GRIFFON_FLYAWAY,self, FALSE);
		//Sometimes make noises because you are pissed at the player beeing under the water!
		if(rnd() > 0.5f)
		{
			gstate->StartEntitySound(self, CHAN_AUTO,gstate->SoundIndex("e2/m_griffonsight.wav"), 0.85f, hook->fMinAttenuation, hook->fMaxAttenuation);
		}
		return;//this is all we need to do, player is in water.
	}

	if(self->movetype == MOVETYPE_FLY)
	{
		//This is for that peachy special case if we chose to put him in a room that has a low ceiling
		//KAHDUH!  DON'T PUT A HUGE FREAKING MONSTER THAT FLIES IN A FUCKING SMALL LOW CEILING ROOM!
		float Monster = AI_Determine_Room_Height( self, 500, 0);
		if(Monster <=250)
		{
			if( ! AI_Checkbelow_For_Liquid( self, 8) )
			{
				
				AI_RemoveCurrentTask(self,TASKTYPE_GRIFFON_GROUND,self, FALSE);
				//self->s.origin.z += 20;
				return;

			}
		}
	}

	if ( AI_IsLineOfSight( self, self->enemy ) )
	{
		//Close enough to bust their heads yet?
		ai_weapon_t *weapon = (ai_weapon_t *) self->curWeapon;
		
		if (fDistance <= weapon->distance + 35)
		{
			//AI_Dprintf("Remove Chase!!\n");
			AI_SetOkToAttackFlag( hook, TRUE );
			AI_RemoveCurrentTask(self,TASKTYPE_GRIFFON_ATTACK,self, FALSE);
			return;
		}
		else
		{
			if( (self->movetype == MOVETYPE_WALK) && (fDistance < 300) && (fDistance > 150) && (rnd() < 0.25f))
			{
				AI_AddNewTaskAtFront( self, TASKTYPE_GRIFFON_JUMP);
				return;
			}
			
			pAIData->destPoint = self->enemy->s.origin;  //Set the origin
			
			if(self->movetype == MOVETYPE_FLY)
			{
				pAIData->destPoint.z += weapon->distance-10; //Set the origin offest (will change from monster to monster that is flying)
			}


			//Set the resolution for % change in degrees for smooth turn
			//this value works well for the Griffon
			pAIData->fValue		= 0.05f;



			float fSpeed = AI_ComputeMovingSpeed( hook );
			
			if(self->movetype == MOVETYPE_FLY)
			{
			
				//if ( (AI_HandleAirCollisionWithEntities( self, pAIData->destPoint, fSpeed )) || (AI_HandleAirObstacle( self, fSpeed, FALSE )) || (AI_HandleAirUnitGettingStuck( self, pAIData->destPoint, fSpeed )))
				//{
				//	return;	
				//}

			}
			else
			{
				if( AI_HandleCollisionWithEntities(self, pAIData->destPoint, fSpeed) || AI_HandleGroundObstacle( self, fSpeed, FALSE ) || AI_HandleGettingStuck( self, pAIData->destPoint, fSpeed ) )
				{
					return;
				}
			}

			if(AI_IsEndAnimation(self))
			{
				if(hook->cur_sequence && hook->cur_sequence->animation_name)
				{
					AI_ForceSequence(self,hook->cur_sequence->animation_name,FRAME_LOOP);
				}
			}
				
				
			AI_FlyTowardPoint2( self, pAIData->destPoint, pAIData->fValue);

			if(self->movetype == MOVETYPE_FLY)
			{
				CVector vel = self->velocity;
				vel.Normalize();
				vel = (vel * hook->run_speed * 1.15);
				self->velocity = vel;
			}
			else
			{
				AI_OrientToFloor( self );
			}

			
			//Check to see if we haven't moved...we could be stuck on something.
			if(AI_FLY_Debug_Origin( self ))
			{
				pAIData->nValue++;
			}

			CVector facingDir = self->velocity;
			facingDir.Normalize();

			if( ( pAIData->nValue > COLLISION_TIME) && (self->movetype == MOVETYPE_FLY) )
			{
				AI_Dprintf( "Collision time timed out.\n");			
				AI_RemoveCurrentTask(self,TASKTYPE_GRIFFON_FLYAWAY,self, FALSE);
				return;
			}
		}
	}
	else
	{ 
		if(AI_IsLineOfSight( self, self->enemy )&& (self->movetype != MOVETYPE_WALK))
		{
			AI_RemoveCurrentTask( self );
		}
		else
		{
			AI_Chase(self);
		}
	}

	//Set our last origin to help with collision detection
	hook->last_origin = self->s.origin;
	

}


// ----------------------------------------------------------------------------
//<nss>
// Name:		GRIFFON_StartFlyAway
// Description: Begin to fly away to a new random point
// Input:userEntity_t *self
// Output:<none>
// Notes:
// Axis--> This vector is to tell the AI_ComputeFlyAwayPoint what axis it should
// search.  DO NOT SET THE VALUE ABOVE 1 OR IT WILL MULTIPLY THE RESULTANT VECTOR
// FOR THAT AXIS BY ANY VALUE GREATER THAN 1.  To eliminate an axis, just set that
// axis to the value of 0. Or to increase velocity go ahead an move it above 1.0
// flyAwaPoint-->the Vector which will have the new location stored in
// ----------------------------------------------------------------------------
void GRIFFON_StartFlyAway( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	//Make sure we are flying!
	if(self->movetype != MOVETYPE_FLY)
	{
		AI_RemoveCurrentTask( self, FALSE );
		return;
	}

	CVector flyAwayPoint = self->s.origin;
	CVector Axis;

	Axis.x = 1;
	Axis.y = 1;
	Axis.z = 1;
	if(AI_ComputeFlyAwayPoint( self, flyAwayPoint, FLY_AWAY_DISTANCE, 15, &Axis )==0)
	{
		
		Axis.x = 1;
		Axis.y = 1;
		Axis.z = 0;
		if(AI_ComputeFlyAwayPoint( self, flyAwayPoint, FLY_AWAY_DISTANCE, 15, &Axis )==0)
		{
		
			Axis.x = 1;
			Axis.y = 1;
			Axis.z = -1;
			if(AI_ComputeFlyAwayPoint( self, flyAwayPoint, FLY_AWAY_DISTANCE, 15, &Axis )==0)
			{

				//AI_Dprintf( "No point found setting my new origin to above enemy's head.\n");
				flyAwayPoint = self->enemy->s.origin;
				flyAwayPoint.z += HOVER_DISTANCE;
			}
		}
	}

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );
	
	MAPNODE_PTR Node = 	NODE_GetClosestNode( self, flyAwayPoint);

	if(!Node)
	{
		AI_Dprintf("BAD THINGS ARE HAPPENING!!!Flyaway with no Node system initialized!\n");
		AI_RemoveCurrentTask(self,FALSE);
		return;
	}
	pAIData->destPoint = Node->position;

	//Setup things for fly_to_point function
	
	pAIData->nValue = 0;

	AI_SetOkToAttackFlag( hook, FALSE );
	
	float fDistance = VectorDistance( self->s.origin, flyAwayPoint);
	float fSpeed = AI_ComputeMovingSpeed( hook );
	
	AI_SetTaskFinishTime( hook,(fDistance/fSpeed));

	self->nextthink	= gstate->time + 0.1f;
}


// ----------------------------------------------------------------------------
//
// Name:		GRIFFON_FlyAway
// Description:
// Input:
// Output:
// Note:Need to figure out how to get the Griffon to make sure that it is not
// going to fly away to some weird point that will cause it to get stuck
// ----------------------------------------------------------------------------
void GRIFFON_FlyAway( userEntity_t *self )
{
	userEntity_t *attacker = self->enemy;
	playerHook_t *hook = AI_GetPlayerHook( self );

//	int bEnemyVisible = AI_IsLineOfSight( self, self->enemy );// SCG[1/23/00]: not used

	//Get the % of health loss
//	float P_health_loss = GRIFFON_State_Change(self);// SCG[1/23/00]: not used


	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	float fDistance = VectorDistance( self->s.origin, pAIData->destPoint );

	pAIData->fValue		= 0.05f;

	//AI_Dprintf("Flyaway!!\n");
	//Check to see if we are stuck.
	if(AI_FLY_Debug_Origin(self))
	{
		pAIData->nValue++;
	}

	if(AI_FlyTowardPoint2( self, pAIData->destPoint, pAIData->fValue)|| pAIData->nValue > COLLISION_TIME)
	{
		//AI_Dprintf( "Collision Occured(ing) or reached destination.\n");
		int bEnemyVisible = AI_IsLineOfSight( self, self->enemy );
		if(bEnemyVisible)
		{
			AI_RemoveCurrentTask(self,TASKTYPE_GRIFFON_FLYAWAY,self, FALSE);
		}
		else
		{
			AI_RemoveCurrentTask(self,TASKTYPE_CHASE,self, FALSE);
		}
	}
	if( ( fDistance <= HOVER_DISTANCE + 20) )
	{
		//AI_Dprintf( "Removing FlyAway Task.\n");
		AI_SetOkToAttackFlag( hook, TRUE );
		AI_RemoveCurrentTask(self,FALSE);
	}

	//Set our last origin to help with collision detection
	hook->last_origin = self->s.origin;

}

// ----------------------------------------------------------------------------
//
// Name:		GRIFFON_Start_Jump
// Description:Start the jump sequence for the Griffon
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void GRIFFON_Start_Jump(userEntity_t *self)
{
	CVector			vel;

	playerHook_t	*hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	
	CVector dir = self->enemy->s.origin - self->s.origin;
	dir.Normalize();

	self->s.angles.yaw = VectorToYaw(dir);
	
	//Store our old velocity
	//pAIData->destPoint = self->velocity;
	self->movetype = MOVETYPE_BOUNCE;
	
	vel = self->enemy->s.origin - self->s.origin;
	vel.Normalize();

	vel = vel * (hook->run_speed * 1.65);
	
	self->velocity = vel;
	self->velocity.z = (hook->upward_vel*1.1);
	
	if(self->velocity.z < 0.0f)
		AI_Dprintf("Velocity is negative!\n");
	pAIData->destPoint = self->enemy->s.origin;
	AI_ForceSequence(self, "jumpa", FRAME_LOOP);

	//Set the Time marker to end the jump sequence.
	pAIData->fValue = gstate->time; 

	AI_SetTaskFinishTime( hook, -1 );
	self->s.origin.z +=10.0;
	self->nextthink	= gstate->time + 0.1f;
	AI_SetOkToAttackFlag( hook, FALSE );

}

// ----------------------------------------------------------------------------
//
// Name:		GRIFFON_Jump
// Description:
// Input:
// Output:
// Note:This might need some work... 
//
// ----------------------------------------------------------------------------
void GRIFFON_Jump(userEntity_t *self)
{
	float fDistance;
	CVector vel;
	
	playerHook_t	*hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	//If our upward velocity is real close to nothing then we 
	//should be pretty much done with our jump

	fDistance = VectorDistance( self->s.origin, pAIData->destPoint );
	float Player_Distance = VectorDistance(self->s.origin, self->enemy->s.origin);

	if( ((pAIData->fValue + 1.0f) <= gstate->time) || (fDistance <= 100.0f))
	{
		self->movetype = MOVETYPE_WALK;
		if(Player_Distance <= 150.0f)
		{
			gstate->StartEntitySound(self, CHAN_AUTO,gstate->SoundIndex("e2/m_griffonataka.wav"), 0.85f, ATTN_NORM_MIN,ATTN_NORM_MAX);
			CVector Dir = self->enemy->s.origin - self->s.origin;
			Dir.Normalize();
			AI_Dprintf("Do Dammage!\n");
			com->Damage(self->enemy,self,self,self->enemy->s.origin,Dir,15.0,DAMAGE_INERTIAL);
			//ai_fire_curWeapon( self );
		}
		else if(rnd() > 0.30f)
		{
			gstate->StartEntitySound(self, CHAN_AUTO,gstate->SoundIndex("e2/m_griffonsight.wav"), 0.85f, ATTN_NORM_MIN,ATTN_NORM_MAX);
		}
		//ai_fire_curWeapon( self );
		AI_RemoveCurrentTask( self, FALSE );
		self->movetype = MOVETYPE_WALK;
	}

}
// ----------------------------------------------------------------------------
//
// Name:		GRIFFON_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GRIFFON_Begin_Attack( userEntity_t *self )
{

	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );

	pAIData->nValue = 0;


	if(self->movetype == MOVETYPE_FLY)
	{
		AI_ForceSequence(self, "atakd", FRAME_LOOP);
	}
	else
	{
		AI_ZeroVelocity(self);
		if(rnd() > 0.50f)
		{
			AI_ForceSequence( self, "atakb", FRAME_LOOP );
		}
		else
		{
			AI_ForceSequence( self, "ataka", FRAME_LOOP );
		}
	}
	AI_SetTaskFinishTime( hook, -1 );
	AI_SetOkToAttackFlag( hook, FALSE );
}





// ----------------------------------------------------------------------------
//
// Name:		griffon_attack
// Description:  The attack sequence for both Air and Ground
// Input: userEntity_t *self
// Output: na
// Note:
//
// ----------------------------------------------------------------------------
void GRIFFON_Attack(userEntity_t *self)
{

	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
//	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );// SCG[1/23/00]: not used

	
	AI_FaceTowardPoint( self, self->enemy->s.origin );

	self->ideal_ang.roll = 0;
	com->ChangeRoll(self);

//	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);// SCG[1/23/00]: not used
	
//	ai_weapon_t *weapon = (ai_weapon_t *) self->curWeapon;// SCG[1/23/00]: not used
		
	//When in the air the attack sequence looks good already so go ahead, when ready and claw
	if ( self->movetype == MOVETYPE_FLY && (AI_IsReadyToAttack1( self ) || AI_IsReadyToAttack2( self )) )
	{
		ai_fire_curWeapon( self );
		AI_PlayAttackSounds( self );

		if( ! AI_Checkbelow_For_Liquid( self, 8) )
		{
			AI_RemoveCurrentTask(self,TASKTYPE_GRIFFON_GROUND,self, FALSE);
			//self->s.origin.z += 60;
		}
	}
	else 
    if ( (self->movetype == MOVETYPE_WALK) && (AI_IsReadyToAttack1( self ) || AI_IsReadyToAttack2( self )) )
	{
		ai_fire_curWeapon( self );
		AI_PlayAttackSounds( self );
	}

	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
			AI_RemoveCurrentTask(self, FALSE);
			AI_SetOkToAttackFlag( hook, TRUE);
	}
	//if (self->movetype == MOVETYPE_WALK)
	//{
	//	if( dist > weapon->distance)
	//	{
	//		AI_SetOkToAttackFlag( hook, TRUE);
	//		AI_RemoveCurrentTask(self,FALSE);
	//	}
	//}
}



// ----------------------------------------------------------------------------
// Coder: NSS
// Name:		monster_griffon
// Description: Spawns a monster of type griffon upon level load
// Input:userEntity_t *self
// Output:NA
// Note:NA
// ----------------------------------------------------------------------------
void monster_griffon( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_GRIFFON );

	self->className			= "monster_griffon";
	self->netname			= tongue_monsters[T_MONSTER_GRIFFON];

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
	//	give griffon a weapon and setup its information (size,bounding region,etc..)
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
	//Setup some stuff that I am making sure get set right
	self->movetype			= MOVETYPE_FLY;
	
	hook->dflags			|= DFL_JUMPATTACK;
	hook->dflags			|= DFL_FACEPITCH;

	hook->fnStartAttackFunc = GRIFFON_Begin_Attack;
	hook->fnAttackFunc		= GRIFFON_Attack;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load
	self->pain				= GRIFFON_StartPain;

	hook->pain_chance		= 10;

	self->elasticity		= 2.0f;
	
	hook->nAttackType = ATTACK_AIR_SWOOP_MELEE;
	hook->nAttackMode = ATTACKMODE_NORMAL;
	hook->pMovingAnimation  = NULL;
	self->fragtype     |= FRAGTYPE_ALWAYSGIB;
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
void world_ai_griffon_register_func()
{
	gstate->RegisterFunc("Griffon_Think",Griffon_Think);
	gstate->RegisterFunc("GRIFFON_StartPain",GRIFFON_StartPain);
	gstate->RegisterFunc("GRIFFON_Begin_Attack",GRIFFON_Begin_Attack);
	gstate->RegisterFunc("GRIFFON_Attack",GRIFFON_Attack);
}
