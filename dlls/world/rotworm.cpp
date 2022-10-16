// ==========================================================================
//
//  File:     rotworm.cpp
//  Contents:
//  Author:
//
// ==========================================================================
#include "world.h"
#include "ai_common.h"
#include "ai_utils.h"
//#include "ai_move.h"	// SCG[1/23/00]: not used
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"
#include "ai.h"

/* ***************************** define types ****************************** */

// the probablity of a long-range attack being a jump attack
// this multiplied by jump_chance gives the chance of a jump attack on a given think
#define ROTWORM_JUMP_ATTACK_CHANCE 0.25f
#define ROTWORM_SPIT_ATTACK_CHANCE 0.25f
// The distance threshold, within which the rotworm will drop from the ceiling
//#define ROTWORM_DROP_DISTANCE 256.0f	// SCG[1/23/00]: not referenced

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */

void rotworm_start_attack( userEntity_t *self );
void rotworm_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &destPoint );

void ROTWORM_StartJumpSpring( userEntity_t *self );
void ROTWORM_JumpSpring( userEntity_t *self );
void ROTWORM_StartJumpFly( userEntity_t *self );
void ROTWORM_JumpFly( userEntity_t *self );
void ROTWORM_StartJumpBite( userEntity_t *self );
void ROTWORM_JumpBite( userEntity_t *self );

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
float AI_Determine_Room_Height( userEntity_t *self, int Max_Mid, int type);
int AI_CheckForEnemy( userEntity_t *self );

/* ******************************* exports ********************************* */

DllExport void monster_rotworm(userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
void ROTWORM_Think( userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	if(hook)
	{
		//The first thing we check for is if we are on the ceiling.
		if(hook->nSpawnValue & SPAWN_CEILING)
		{
			if( AI_CheckForEnemy( self ) && self->enemy )
			{
				if(VectorDistance(self->s.origin,self->enemy->s.origin) < hook->active_distance)
				{
					self->movetype = MOVETYPE_WALK;
					hook->nSpawnValue	&= ~ SPAWN_CEILING;
				}
			}
			self->nextthink = gstate->time + 0.3f;
			return;
		}

		//Get the goal pointers and task pointers
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		GOAL_PTR pGoal = pGoalStack->pTopOfStack;
		
		//NSS[11/19/99]:Check to see if we have a Goal then check for the type
		if(pGoal)
		{
			if(pGoal->nGoalType == GOALTYPE_KILLENEMY && self->enemy)
			{
				TASK_PTR pCurrentTask = GOAL_GetCurrentTask( pGoal );		
				//NSS[11/19/99]:If we have a current task continue on to seek what we seek
				if(pCurrentTask)
				{
					//NSS[11/19/99]:Are we chasing?  If so let's see if we want to jump or spit
					if(pCurrentTask->nTaskType == TASKTYPE_CHASE)
					{
						float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
						if(self->enemy)
						{
							if ( dist > 200 && AI_IsVisible( self, self->enemy ) && rnd() > ROTWORM_JUMP_ATTACK_CHANCE )
							{
								// Jump attack!
								AI_AddNewGoal( self, GOALTYPE_ROTWORM_JUMPATTACK );
							}
							else if ( AI_IsVisible( self, self->enemy ) && rnd() > ROTWORM_SPIT_ATTACK_CHANCE)
							{
								self->curWeapon = gstate->InventoryFindItem(self->inventory, "venom spit");
								AI_AddNewTask(self,TASKTYPE_ATTACK);
							}
						}
					}
				}
			}
		}
	}
	//NSS[11/19/99]:If we have made it this far, let's continue on with our tasks
	AI_TaskThink(self);
}

// SCG[1/23/00]: not referenced
//void AI_OrientToFloor( userEntity_t *self );

// ----------------------------------------------------------------------------
//NSS[11/21/99]:
// Name: rotworm_set_attack_seq
// Description: sets up attack sequence for rotworm
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void rotworm_set_attack_seq( userEntity_t *self )
{
	// zero out pitch
	self->ideal_ang.pitch = 0.0f;
	self->s.angles.pitch = 0.0f;

//	playerHook_t *hook = AI_GetPlayerHook( self );
	frameData_t *pSequence = NULL;
	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);

    AI_FaceTowardPoint( self, self->enemy->s.origin );

	
	if(dist < 60.0f || rnd () < 0.25f)
	{
		// set current weapon to bite
		self->curWeapon = gstate->InventoryFindItem(self->inventory, "poison bite mild");
		// set bite sequence
		pSequence = FRAMES_GetSequence( self, "ataka" );
		AI_ForceSequence(self, pSequence);

	}
	else
	{
		self->curWeapon = gstate->InventoryFindItem(self->inventory, "venom spit");

		pSequence = FRAMES_GetSequence( self, "atakc" );
		AI_ForceSequence(self, pSequence);
	}

}


// ----------------------------------------------------------------------------
//NSS[11/21/99]:
// Name:        rotworm_start_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void rotworm_start_attack( userEntity_t *self )
{
//	float dist = VectorDistance( self->s.origin, self->enemy->s.origin );
	playerHook_t *hook = AI_GetPlayerHook( self );

	rotworm_set_attack_seq( self );
	AI_SetOkToAttackFlag( hook, FALSE );
}


// ----------------------------------------------------------------------------
//NSS[11/21/99]:
// Name: rotworm_attack
// Description: attack function for rotworm
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void rotworm_attack( userEntity_t *self )
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
		AI_RemoveCurrentTask(self,FALSE);
		AI_SetOkToAttackFlag( hook, TRUE);
	}
}


void rotworm_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                        int damage, CVector &destPoint )
{
	// zero out pitch
	self->ideal_ang.pitch = 0.0f;
	self->s.angles.pitch = 0.0f;

	AI_StartDie( self, inflictor, attacker, damage, destPoint );
}




void ROTWORM_StartJumpFly( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	CVector			vel;

	CVector dir = self->enemy->s.origin - self->s.origin;
	dir.Normalize();

	// set fly sequence
	frameData_t *pSequence = FRAMES_GetSequence( self, "atakb" );
	AI_ForceSequence( self, pSequence,FRAME_ONCE );

	self->s.angles.yaw = VectorToYaw(dir);
	
	vel = self->enemy->s.origin - self->s.origin;
	vel.Normalize();

	vel = vel * (hook->run_speed * 1.95);
	
	self->velocity = vel;
	self->velocity.z = (hook->upward_vel*1.3);

	pAIData->destPoint = self->enemy->s.origin;
	AI_StartSequence(self, "jumpa", FRAME_LOOP);

	AI_SetOkToAttackFlag( hook, FALSE );

	AI_SetTaskFinishTime( hook, 5.0f );
	self->delay = gstate->time + 3.0f;
	AI_Dprintf( "Starting TASKTYPE_ROTWORM_JUMPFLY.\n" );
}

void ROTWORM_JumpFly( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	ai_frame_sounds( self );

	
	float fDistance = VectorDistance(self->s.origin,pAIData->destPoint);
	
	AI_Dprintf("vX:%f vY:%f vZ:%f\n",self->velocity.x,self->velocity.y,self->velocity.z);

	if(fDistance < 32.0f||self->delay < gstate->time)
	{
		// Hit ground?
		if ( self->groundEntity )
		{
			// Landed on enemy?
			if ( self->groundEntity == self->enemy )
			{
				// drop to jump bite
				AI_RemoveCurrentTask( self );
			}
			else
			{
				playerHook_t *hook = AI_GetPlayerHook( self );

				AI_UpdateCurrentNode( self );
				AI_SetOkToAttackFlag( hook, TRUE );

				// goal complete
				GOAL_Satisfied( GOALSTACK_GetCurrentGoal( AI_GetCurrentGoalStack( hook ) ) );
				AI_RemoveCurrentGoal( self );
			}
		}	
	}
}

void ROTWORM_StartJumpSpring( userEntity_t *self )
{
	ROTWORM_StartJumpFly( self );
}

void ROTWORM_JumpSpring( userEntity_t *self )
{
	ROTWORM_JumpFly( self );
}

void ROTWORM_StartJumpBite( userEntity_t *self )
{
	frameData_t *pSequence = FRAMES_GetSequence( self, "ataka" );
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_ForceSequence(self, pSequence);

	self->curWeapon = gstate->InventoryFindItem(self->inventory, "poison bite mild");

	AI_SetTaskFinishTime( hook, 10.0f );

	AI_Dprintf( "Starting TASKTYPE_ROTWORM_JUMPBITE.\n" );
}

void ROTWORM_JumpBite( userEntity_t *self )
{
    AI_FaceTowardPoint( self, self->enemy->s.origin );

	ai_frame_sounds( self );

	if ( AI_IsReadyToAttack1( self ) )
	{
		ai_fire_curWeapon( self );
	}

	if ( AI_IsEndAnimation( self ) )
	{
		playerHook_t *hook = AI_GetPlayerHook( self );

		// goal complete
		GOAL_Satisfied( GOALSTACK_GetCurrentGoal( AI_GetCurrentGoalStack( hook ) ) );
		AI_RemoveCurrentGoal( self );
	}
}


// ----------------------------------------------------------------------------
//
// Name:		rotworm_ParseEpairs
// Description: Pre_think, Post_Parse for Rotworm
// Input:    userEntity_t *self
// Output: NA
// Note:This is how it should have been setup.
//
// ----------------------------------------------------------------------------
void rotworm_ParseEpairs( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_ParseEpairs(self);
	
	//NSS[11/19/99]:Deal with SPAWN CELING Bullshit
	if(hook->nSpawnValue & SPAWN_CEILING)
	{
		//Get the distance from the ceiling
		float Distance = AI_Determine_Room_Height( self, 10000, 2);	
		//If we are further than 84 units then position ourself closer to the ceiling
		if(Distance > (self->s.maxs.z - self->s.mins.z) )
		{
			Distance -= (self->s.maxs.z - self->s.mins.z);
			self->s.origin.z += Distance;
		}
		self->movetype = MOVETYPE_NONE;
		self->s.angles.pitch = 90.0f;

	}
	self->think			= ROTWORM_Think;
	self->nextthink		= gstate->time + 0.1f;
}



// ----------------------------------------------------------------------------
//
// Name:		monster_rotworm
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_rotworm( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_ROTWORM );

	self->className			= "monster_rotworm";
	self->netname			= tongue_monsters[T_MONSTER_ROTWORM];

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

	gstate->SetSize (self, -16.0, -16.0, -24.0, 16.0, 16.0, 32.0);

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	//	turn rates for each axis in degrees per second
	self->ang_speed.Set( 90, 180, 90 );

	//////////////////////////////////////////////
	//////////////////////////////////////////////
	self->inventory = gstate->InventoryNew (MEM_MALLOC);

	hook->dflags			|= DFL_RANGEDATTACK;
	hook->dflags			|= DFL_JUMPATTACK;

	hook->fnStartAttackFunc = rotworm_start_attack;
	hook->fnAttackFunc		= rotworm_attack;

	self->think				= rotworm_ParseEpairs;
	self->nextthink			= gstate->time + 0.1;

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
										  "poison bite mild",
										  poison_bite_mild,
										  ITF_TRACE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );

		self->curWeapon = ai_init_weapon( self,
										  pWeaponAttributes[1].fBaseDamage,
										  pWeaponAttributes[1].fRandomDamage,
										  pWeaponAttributes[1].fSpreadX,
										  pWeaponAttributes[1].fSpreadZ,
										  pWeaponAttributes[1].fSpeed,
										  pWeaponAttributes[1].fDistance,
										  pWeaponAttributes[1].offset,
										  "venom spit",
										  venom_spit,
										  ITF_PROJECTILE | ITF_NOLEAD ); // don't lead our shots with the spit
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}

	self->movetype = MOVETYPE_WALK;

	hook->nAttackType = ATTACK_GROUND_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;
	hook->nOriginalMoveType = self->movetype;

	AI_DetermineMovingEnvironment( self );

	//NSS[11/19/99]:DO NOT REMOVE THIS...THIS IS NEEDED DUE TO THE FACT THAT ROTWORMS HANG FROM THE CEILING!
	self->view_ofs.Set(0.0f, 0.0f, 0.0f);
	
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
void world_ai_rotworm_register_func()
{
	gstate->RegisterFunc("rotworm_start_die",rotworm_start_die);
	gstate->RegisterFunc("ROTWORM_Think",ROTWORM_Think);
	gstate->RegisterFunc("rotworm_start_attack",rotworm_start_attack);
	gstate->RegisterFunc("rotworm_attack",rotworm_attack);
}
