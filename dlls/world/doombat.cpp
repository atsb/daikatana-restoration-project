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
#include "ai_common.h"


#define AMPLITUDE_ADJUSTER				30		// higher number equals higher amplitude

#define HOVER_DISTANCE					80.0f
#define DOOMBAT_FLY_AWAY_DISTANCE		1000.0f
#define COLLISION_TIME					2


//Define the Attack States
#define DB_MELEE_ATTACK 0
#define DB_RANGE_ATTACK 1
#define DB_KAMAKAZI_ATTACK 2


/* ***************************** define types ****************************** */
/* ***************************** Local Variables *************************** */

/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
extern float sin_tbl[];
/* **************************** Global Functions *************************** */
//void AI_StartChase( userEntity_t *self );// SCG[1/23/00]: not used
void AI_Chase( userEntity_t *self );
int AI_StartFly( userEntity_t *self );
//int AI_HandleAirCollisionWithEntities( userEntity_t *self, CVector &destPoint, float fSpeed );// SCG[1/23/00]: not used
//int AI_HandleAirObstacle( userEntity_t *self, float fSpeed, int bMovingTowardNode );// SCG[1/23/00]: not used
//int AI_HandleAirUnitGettingStuck( userEntity_t *self, CVector &destPoint, float fSpeed );// SCG[1/23/00]: not used
int AI_ComputeFlyAwayPoint2( userEntity_t *self, CVector &flyAwayPoint, int Distance, int Resolution, CVector &Axis);
//int AI_CheckBlockingDirectionFlyUnits( userEntity_t *self, CVector &destPoint );// SCG[1/23/00]: not used
//void AI_Drop_Markers (CVector &Target, float delay);// SCG[1/23/00]: not used
/* ******************************* exports ********************************* */

DllExport void monster_doombat (userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////////
//	doombat specific funcitons
///////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
//
// Name:		DOOMBAT_StartIdle
// Description:Idle Sequence for Doombat
// Input:self
// Output:NA
// Note: 
// ----------------------------------------------------------------------------
void DOOMBAT_StartIdle(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

    frameData_t *pSequence;

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, -1.0f );
	AI_SetMovingCounter( hook, 0 );

	if(hook->acro_boost > 0.0f)
	{
		AI_Dprintf( "Starting GoalType Wander due to the fact that we are probably in the air.\n" );
		pSequence = FRAMES_GetSequence( self, "flya" );
		AI_AddNewGoal(self,GOALTYPE_WANDER);
		return;
	}
	else
	{
		pSequence = FRAMES_GetSequence( self, "amba" );
	}

    if ( pSequence )
    {
        AI_StartSequence(self, pSequence, pSequence->flags);
    }

    AI_ZeroVelocity( self );

	self->enemy = NULL;
	AI_SetNextThinkTime( self, 0.1f );

	AI_SetStateIdle( hook );

    AI_Dprintf( "Starting TASKTYPE_IDLE.\n" );
}

// ----------------------------------------------------------------------------
//
// Name:		DOOMBAT_Think
// Description:Think function for Doombat, this is a good debug break point to.
// see what is happening within AI_THINK.
// Input:self
// Output:NA
// Note: 
// ----------------------------------------------------------------------------
void DOOMBAT_Think(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );


	AI_TaskThink(self);
	
	if(!AI_IsAlive(self))
	{
		self->gravity=1.5f;
		self->movetype = MOVETYPE_BOUNCE;
	}
	else
	{
		if(hook->acro_boost_time > 12.0f)
		{	
			hook->acro_boost_time = 0.0f;
		}
		self->velocity.z += 50.0*(sin_tbl[(int)hook->acro_boost_time]);
		hook->acro_boost_time +=2.5;
	}
}

void DOOMBAT_Touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	CVector Dir;
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fThreeFrameSpeed = fSpeed * 0.60f;
	
	if(plane)
	{
		Dir = plane->normal;
		Dir = Dir * fThreeFrameSpeed;
		self->velocity += Dir;
	}
	
	//AI_Dprintf("I am touching something!\n");
}


// ----------------------------------------------------------------------------
//
// Name:		DOOMBAT_State_Change
// Description:States which change on the Doombat.
// Input:self
// Output:will return the current health
// Note: 
// This function should be called from the main loop of any task that could
// potentially run for longer than a second.  It will change any state of the
// monster which is directly related to the condition of the monster.
// ----------------------------------------------------------------------------
float Doombat_State_Change(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	//Get the % of health loss
	float P_health_loss = 100*(self->health/hook->base_health);

	if( P_health_loss <= (self->health*0.50f))
	{
		//we are messed up, but we are pissed!  So move a little quicker
		//This value will probably end up being a % value based on the initial speed.
		hook->run_speed			*= 1.35;
		hook->walk_speed		*= 1.35;
	}
	return P_health_loss;
}

// ----------------------------------------------------------------------------
//
// Name:		DOOMBAT_Start_Attack_Think
// Description:Initialize the Doombat think states
// Input:self
// Output:<nothing>
// Note:
// ----------------------------------------------------------------------------
void DOOMBAT_Start_Attack_Think( userEntity_t *self )
{
	//Nothing needs to be initialized as of yet
}

// ----------------------------------------------------------------------------
//
// Name:		DOOMBAT_Attack_Think
// Description:Determines what tasks the Doombat will do during its attack
// Input:self
// Output:<nothing>
// Note:
// 
// ----------------------------------------------------------------------------
void DOOMBAT_Attack_Think( userEntity_t *self )
{
	
	//Attack states are no yet implemented.  So far all choices are based off of health and
	//random values.
	playerHook_t *hook = AI_GetPlayerHook( self );

	//Get the % of health loss
	float P_health_loss = Doombat_State_Change(self);
	
	//Go ahead and remove the DOOMBAT_Attack_Think task from the set of tasks
	//We do this because we know that by the time this code is finished we will
	//have a new set of tasks assigned.
	
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );
	GOAL_PTR pGoal = pGoalStack->pTopOfStack;
	
	if ( !self->enemy )
	{
		self->enemy = pAIData->pEntity;
	}
	//Dumb temp flag to tell me that I have seen the player so if I go back to idle state I can reset my goaltype to wander
	hook->acro_boost = 1.0f;
	//Temp hack... set the DOOMBAT_Think after it has initialized.	
	self->think = DOOMBAT_Think;

	self->curWeapon = gstate->InventoryFindItem (self->inventory, "punch");

	if ( AI_IsLineOfSight( self, self->enemy ) )
	{

	//If we are below the hurt marker and we are not too dumb, then do a ranged attack
	//If the random factor kicks in we do a swoop attack
		
		if( ( P_health_loss <= (hook->base_health * 0.50f) ) && ( rnd() < .25f ) )//FIREBALLS AND AVOID
		{

			self->curWeapon = gstate->InventoryFindItem (self->inventory, "fireball");
			AI_Dprintf( "Started Long Range Attack Mode.\n" );
			//Remove the think and add the flyaway in its place.
			GOAL_ClearTasks(pGoal);
			AI_AddNewTask(self, TASKTYPE_DOOMBAT_FLYAWAY);
			AI_AddNewTask(self, TASKTYPE_DOOMBAT_HOVER);
			AI_AddNewTask(self, TASKTYPE_DOOMBAT_ATTACK);
		}
		else
		{
				AI_Dprintf( "Going into Regular Attack Mode.\n" );
				self->curWeapon = gstate->InventoryFindItem (self->inventory, "punch");
				GOAL_ClearTasks(pGoal);
				AI_AddNewTask( self, TASKTYPE_DOOMBAT_CHASE);
				AI_AddNewTask( self, TASKTYPE_DOOMBAT_ATTACK);
				AI_AddNewTask( self, TASKTYPE_DOOMBAT_FLYAWAY);
		}
	}
	else // Go into Path Finding Mode
	{
		//Use generic flying code to get us in sight of the player.
		AI_RemoveCurrentTask(self,TASKTYPE_DOOMBAT_CHASE,self);
	}
	//Setup next think time
	self->nextthink = gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
//
// Name:		DOOMBAT_StartHover
// Description: Hover sequence intialization.  Just before Long Ranged Attack
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void DOOMBAT_StartHover( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	//Alert the player the we are gonna spit fire giz at them!
	gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e3/e_firespitb.wav"), 
                              1.0f, hook->fMinAttenuation, hook->fMaxAttenuation );
	
	//Calculate a 1/4 of the way point to the player
	pAIData->destPoint.x = (self->enemy->s.origin.x - self->s.origin.x)/4;
	pAIData->destPoint.y = (self->enemy->s.origin.y - self->s.origin.y)/4;
	//pAIData->destPoint.z = (self->enemy->s.origin.z - self->s.origin.z)/4;

	CVector vel = pAIData->destPoint;
	vel.Normalize();
	//Go at 1/4 the speed
	self->velocity = vel * (hook->run_speed*1/4);
	AI_SetTaskFinishTime( hook, -1 );
	self->nextthink	= gstate->time + 0.1f;
	pAIData->fValue = gstate->time;
	AI_SetOkToAttackFlag( hook, FALSE );
}

// ----------------------------------------------------------------------------
//
// Name:		DOOMBAT_Hover
// Description: Hover sequence intialization.  Just before Long Ranged Attack
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void DOOMBAT_Hover( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	AI_FaceTowardPoint(self,self->enemy->s.origin);

	if ( (pAIData->fValue + .65f) <= gstate->time)
	{
		AI_RemoveCurrentTask( self );
		return;
	}

}	


// ----------------------------------------------------------------------------
//
// Name:		DOOMBAT_StartKamakazi
// Description:
//				move straight toward the enemy
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void DOOMBAT_StartKamakazi( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	AI_SetTaskFinishTime( hook, -1 );

}

// ----------------------------------------------------------------------------
//
// Name:		Doombat_Kamakazi
// Description:
// Input:
// Output:
// Notes:We still need to make this a little more robust and realistic looking
// as it is now, the Kamakazi works and does dammage upon arrival.
// ----------------------------------------------------------------------------
void DOOMBAT_Kamakazi( userEntity_t *self )
{
	ai_fire_curWeapon( self );
	AI_RemoveCurrentTask( self, TRUE );
	AI_Suicide( self, 1000 );
	//New function for the Doombat to explode on the player
	//and do dammage... can be used for any creature... >:)
	return;
}


// ----------------------------------------------------------------------------
// <nss>
// Name:		DOOMBAT_StartChase
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void DOOMBAT_StartChase( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	//Check for Kamakazi mode.
	if(hook->run_speed >= 340.0f)
	{
		AI_ForceSequence(self, "flyc", FRAME_LOOP);
	}
	else
	{
		AI_ForceSequence(self, "flya", FRAME_LOOP);
	}

	if ( AI_IsLineOfSight( self, self->enemy ) )
	{

		if ( !AI_CanMove( hook ) )
		{
			AI_RemoveCurrentTask( self );
			return;
		}

		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

		//If we don't have a pointer to the enemy let's set one up
		if ( !self->enemy )
		{
			GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
			AIDATA_PTR pAIData = GOAL_GetData(pCurrentGoal);
			self->enemy = pAIData->pEntity;
		}

		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
		AIDATA_PTR pAIDataT = TASK_GetData(pCurrentTask);

		
		
		hook->nMoveCounter = 0;
		
		//Trap to check and see if we are stuck
		pAIDataT->nValue = 0;

		self->nextthink	= gstate->time + 0.1f;

		AI_SetOkToAttackFlag( hook, FALSE );

		//Never give up this task! Er.. unless we just can't see the player... :)
		AI_SetTaskFinishTime( hook, -1 );
		AI_SetMovingCounter( hook, 0 );
	}
	else
	{
		playerHook_t *hook = AI_GetPlayerHook( self );

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
		AI_StartFly( self );		
		self->nextthink	= gstate->time + 0.1f;
		AI_SetOkToAttackFlag( hook, FALSE );
		AI_SetTaskFinishTime( hook, 10.0f );
		AI_SetMovingCounter( hook, 0 );
		AI_Chase( self );
	}
}


// ----------------------------------------------------------------------------
// <nss>
// Name:		DOOMBAT_Chase
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void DOOMBAT_Chase( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	if(AI_IsEndAnimation(self))
	{
			AI_ForceSequence(self, "flya", FRAME_LOOP);
	}


	if ( AI_IsLineOfSight( self, self->enemy ) )
	{

	
		//Get the % of health loss
//		float P_health_loss = Doombat_State_Change(self);	
		Doombat_State_Change(self);	// SCG[1/23/00]: P_health_loss not used
		
		float fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );
		
		//Can we see the enemy?
		int bEnemyVisible = AI_IsVisible( self, self->enemy );
		if ( bEnemyVisible )
		{

			//Close enough to bust their heads yet?
			if (fDistance < HOVER_DISTANCE)
			{
				AI_RemoveCurrentTask( self, FALSE );
				return;
			}
			else
			{


				pAIData->destPoint = self->enemy->s.origin;  //Set the origin
				
				pAIData->destPoint.z += HOVER_DISTANCE; //Set the origin offest (will change from monster to monster)

				//Set the resolution for % change in degrees for smooth turn
				//this value works well for the Doombat
				CVector TurnRate;
				TurnRate.Set(0.10,0.20,0.35);
		
				AI_FlyTowardPoint2( self, pAIData->destPoint, AI_GetSkill(TurnRate));

	
				//Check to see if we haven't moved...we could be stuck on something.
				if(AI_FLY_Debug_Origin(self))
				{
					pAIData->nValue++;
				}

			}

			// extend the time
			AI_SetTaskFinishTime( hook,-1);
		}
		else
		{
			AI_RemoveCurrentTask( self, FALSE );
		}

		//Set our last origin to help with collision detection
		hook->last_origin = self->s.origin;
	}
	else
	{
		AI_Chase(self);
	}
}


// ----------------------------------------------------------------------------
//<nss>
// Name:		DOOMBAT_StartFlyAway
// Description: Begin to fly away to a new random point
// Input:userEntity_t *self
// Output:<none>
// Notes:
// Axis--> This vector is to tell the AI_ComputeFlyAwayPoint what axis it should
// search.  DO NOT SET THE VALUE ABOVE 1 OR IT WILL MULTIPLY THE RESULTANT VECOTOR
// FOR THAT AXIS BY ANY VALUE GREATER THAN 1.  To eliminate an axis, just set that
// axis to the value of 0.
// flyAwaPoint-->the Vector which will have the new location stored in
// ----------------------------------------------------------------------------
void DOOMBAT_StartFlyAway( userEntity_t *self )
{
	CVector flyAwayPoint;
	CVector Axis;
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_DOOMBAT_FLYAWAY )
		{
			return;
		}
	}
	//Special case move around objects
	if(pAIData->destPoint.x || pAIData->destPoint.y || pAIData->destPoint.z)
	{
		Axis = pAIData->destPoint;
		AI_ComputeFlyAwayPoint2( self, flyAwayPoint, DOOMBAT_FLY_AWAY_DISTANCE/2, 12, Axis );
		pAIData->destPoint = flyAwayPoint;
	}
	else
	{
		flyAwayPoint = self->s.origin;
//		Axis;// SCG[1/23/00]: not used

		Axis.x = 1;
		Axis.y = 0;
		Axis.z = 1;
		if(AI_ComputeFlyAwayPoint( self, flyAwayPoint, DOOMBAT_FLY_AWAY_DISTANCE, 12, &Axis )==0)
		{
			
			Axis.x = 0;
			Axis.y = 1;
			Axis.z = 1;
			if(AI_ComputeFlyAwayPoint( self, flyAwayPoint, DOOMBAT_FLY_AWAY_DISTANCE, 12, &Axis )==0)
			{
				AI_Dprintf( "No point found setting my new origin to above enemy's head.\n");
				flyAwayPoint = self->enemy->s.origin;
				flyAwayPoint.z += HOVER_DISTANCE;
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
	}

	//Setup things for fly_to_point function
	pAIData->cDYaw = pAIData->gDYaw = pAIData->nValue = 0;

	AI_SetOkToAttackFlag( hook, FALSE );
	
	
	AI_SetTaskFinishTime( hook, -1 );

	self->nextthink	= gstate->time + 0.1f;
}



// ----------------------------------------------------------------------------
//
// Name:		DOOMBAT_FlyAway
// Description:
// Input:
// Output:
// Note:Need to figure out how to get the doombat to make sure that it is not
// going to fly away to some weird point that will cause it to get stuck
// ----------------------------------------------------------------------------
void DOOMBAT_FlyAway( userEntity_t *self )
{
	userEntity_t *attacker = self->enemy;
	playerHook_t *hook = AI_GetPlayerHook( self );

	//Get the % of health loss
//	float P_health_loss = Doombat_State_Change(self);
	Doombat_State_Change(self);// SCG[1/23/00]: P_health_loss not used


	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	float fDistance = VectorDistance( self->s.origin, pAIData->destPoint );
	// NSS[12/13/99]:More than likely this is an invalid point
	if(pAIData->destPoint.Length() == 0.0f)
	{
		AI_RemoveCurrentTask(self,FALSE);
	}

	//Check to see if we are stuck.
	if(AI_FLY_Debug_Origin(self))
	{
		pAIData->nValue++;
	}

	pAIData->fValue		= 0.15f;
	
//	float fSpeed = AI_ComputeMovingSpeed( hook );
	AI_ComputeMovingSpeed( hook );// SCG[1/23/00]: fSpeed not used
	
	AI_FlyTowardPoint2( self, pAIData->destPoint, pAIData->fValue);

	if( ( fDistance <= HOVER_DISTANCE ) )
	{
		AI_Dprintf( "Removing FlyAway Task.\n");
		AI_SetOkToAttackFlag( hook, TRUE );
		AI_RemoveCurrentTask( self, FALSE );
	}

	//Set our last origin to help with collision detection
	hook->last_origin = self->s.origin;

}

// ----------------------------------------------------------------------------
//
// Name:		DOOMBAT_Begin_Attack
// Description:  Start the attack sequence... set all variables/frame animations here
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void DOOMBAT_Begin_Attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	if(!strstr(self->curWeapon->name,"fireball"))
	{
		if(!AI_StartSequence(self, "atakb", FRAME_LOOP))
			return;
	}
	else
	{
		if(!AI_StartSequence(self, "ataka", FRAME_LOOP))
			return;
	}
	AI_SetOkToAttackFlag( hook, FALSE );

	AI_SetTaskFinishTime( hook, -1 );
	self->nextthink	= gstate->time + 0.1f;
}


// ----------------------------------------------------------------------------
//
// Name:		DOOMBAT_Attack
// Description: Duh... generic attack
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void DOOMBAT_Attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	self->velocity.Zero();
	AI_FaceTowardPoint(self,self->enemy->s.origin);
	if ( AI_IsReadyToAttack1( self ) )
	{
		AI_PlayAttackSounds( self );
		AI_Dprintf("Claw or Fire!\n");
		ai_fire_curWeapon( self );
	}
	else if (AI_IsEndAnimation(self))
	{
		AI_RemoveCurrentTask( self, FALSE );
		AI_SetOkToAttackFlag( hook, TRUE );
	}
}




// ----------------------------------------------------------------------------
//
// Name:		monster_doombat
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_doombat( userEntity_t *self )
{
	
	playerHook_t *hook		= AI_InitMonster( self, TYPE_DOOMBAT );
	CVector Orig;
	
	self->className			= "monster_doombat";
	self->netname			= tongue_monsters[T_MONSTER_DOOMBAT];

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

	//	turn rates for each axis in degrees per second
	

	self->movetype			 = MOVETYPE_FLY;

	hook->dflags			|= DFL_FACEPITCH;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2f;

	//Get all information from the CSV file.
	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );

	//////////////////////////////////////////////
	//////////////////////////////////////////////
	self->inventory = gstate->InventoryNew(MEM_MALLOC);

	
	// Mannually setting up the weapon dammage for the fireball.
	pWeaponAttributes[1].fSpeed = 400;

	if ( pWeaponAttributes )
	{
		//Claws
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
		//Fireball
		self->curWeapon = ai_init_weapon( self, 
										  pWeaponAttributes[1].fBaseDamage, 
										  pWeaponAttributes[1].fRandomDamage,
										  pWeaponAttributes[1].fSpreadX,
										  pWeaponAttributes[1].fSpreadZ,
										  pWeaponAttributes[1].fSpeed,
										  pWeaponAttributes[1].fDistance,
										  pWeaponAttributes[1].offset, 
										  "fireball", 
										  fireball_attack, 
										  ITF_PROJECTILE | ITF_EXPLOSIVE);
		gstate->InventoryAddItem (self, self->inventory, self->curWeapon);
	}
	//This is ONLY to make changes to the monster during the creation process
	//Once all of these below values have been "OK'd" send them to Kelly to be
	//inserted into the CSV file.
	hook->fnStartAttackFunc		= DOOMBAT_Begin_Attack;
	hook->fnAttackFunc			= DOOMBAT_Attack;


	//hook->nAttackType = ATTACK_AIR_RANGED;
	//hook->nAttackMode = ATTACKMODE_NORMAL;

	hook->strafe_time = gstate->time;
	hook->strafe_dir  = STRAFE_LEFT;

	AI_DetermineMovingEnvironment( self );
	
//	hook->fnStartIdle = DOOMBAT_StartIdle;

	hook->acro_boost = 0.0f;
	hook->acro_boost_time = 0.0f;

	//Temp setting for active distance.. trying to figure out why bullshit cock lick goes into fuck nut IDLE mode while chasing!!!!
	
	hook->active_distance = 10000.0f;
	self->view_ofs.Set(0.0f, 0.0f, 0.0f);

	self->touch = DOOMBAT_Touch;
	
	gstate->LinkEntity( self );
	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

	// NSS[6/7/00]:Reduce their speed on lower skill levels.
	CVector Speed;
	Speed.Set(0.35f,0.65f,1.0f);
	hook->run_speed		= hook->run_speed * AI_GetSkill(Speed);
	hook->walk_speed	= hook->walk_speed * AI_GetSkill(Speed);
}


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_doombat_register_func()
{
	gstate->RegisterFunc("DOOMBAT_Touch",DOOMBAT_Touch);
	gstate->RegisterFunc("DOOMBAT_Think",DOOMBAT_Think);

	gstate->RegisterFunc("DOOMBAT_Begin_Attack",DOOMBAT_Begin_Attack);
	gstate->RegisterFunc("DOOMBAT_Attack",DOOMBAT_Attack);
}
