// ==========================================================================
//
//  File:     rocketmp.cpp
//  Contents: Uzi gang member AI.
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
#include "ai.h"
//#include "actorlist.h"// SCG[1/23/00]: not used
//#include "spawn.h"// SCG[1/23/00]: not used

/* ***************************** define types ****************************** */

// minimum distance for doing a chasing atack
#define ROCKETMP_MIN_CHASE_ATTACK_DISTANCE 120.0f
// the minimum speed the enemy must have to give chase from melee range, squared
#define ROCKETMP_CHASE_MIN_ENEMY_XY_SPEED_SQUARED 200.0f
// the range of the rocket mp's rocket
#define ROCKETMP_ROCKET_RANGE 1000.0f

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */

void rocketmp_set_attack_seq( userEntity_t *self );
void rocketmp_do_chase_attack_seq( userEntity_t *self );
void rocketmp_do_run_seq( userEntity_t *self );
int rocketmp_chasing_attack( userEntity_t *self );
void rocketmp_begin_attack( userEntity_t *self );
void rocketmp_attack( userEntity_t *self );
void rocketmp_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );
//void rocketmp_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
//                        int damage, CVector &destPoint );// SCG[1/23/00]: not used
int rocketmp_check_range( userEntity_t *self, float dist, userEntity_t *enemy );

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* **************************** Extern Functions *************************** */

void AI_StartDie( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &destPoint );
void AI_StartPain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );

/* ******************************* exports ********************************* */

DllExport void monster_rocketmp( userEntity_t *self );

// ----------------------------------------------------------------------------
//
// Name:		AI_RocketMpTakeCoverAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_RocketMpTakeCoverAttack( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_ZeroVelocity( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

//	float dist = VectorDistance(self->s.origin, self->enemy->s.origin);// SCG[1/23/00]: not used

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) )
	{
		self->curWeapon = gstate->InventoryFindItem(self->inventory, "rocket_left");
		ai_fire_curWeapon( self );
		self->curWeapon = gstate->InventoryFindItem(self->inventory, "rocket_right");
	}
	if ( AI_IsReadyToAttack2( self ) )
	{
		if ( ai_check_projectile_attack(self, self->enemy, (ai_weapon_t *) self->curWeapon, 10) )
		{
			self->curWeapon = gstate->InventoryFindItem(self->inventory, "rocket_right");
			ai_fire_curWeapon( self );
			self->curWeapon = gstate->InventoryFindItem(self->inventory, "rocket_left");
			
		}
		else
		{
			// Ach! We can't hit our enemy!
			AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP ); // sidestep
		}
	}
	
	if ( AI_IsEndAnimation( self ) )
	{
		AI_SetAttackFinished(self);
		AI_IsEnemyDead( self );
		return TRUE;
	}

	return FALSE;
}


int rocketmp_check_range( userEntity_t *self, float dist, userEntity_t *enemy )
{
	// only in range if the enemy is stopped
	float enemymovementdot;

	if ( !self->enemy )
	{
		return FALSE;
	}

	// check if enemy moving toward us
	self->s.angles.AngleToVectors( forward, right, up );
	enemymovementdot = DotProduct( forward, self->enemy->velocity );

	if ( dist < ROCKETMP_ROCKET_RANGE && // if in range and either
	     ( dist < ROCKETMP_MIN_CHASE_ATTACK_DISTANCE || // close
	       enemymovementdot < -1.0f || // if enemy is moving toward us
	       self->enemy->velocity.x * self->enemy->velocity.x +
	       self->enemy->velocity.y * self->enemy->velocity.y <
	       ROCKETMP_CHASE_MIN_ENEMY_XY_SPEED_SQUARED ) ) // or speed is under minimum
	{
		 return TRUE; // attack
	}

	return FALSE; // must chase
}


// ----------------------------------------------------------------------------
//
// Name:		AI_RocketMpTakeCover
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_RocketMpTakeCover( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	frameData_t *pSequence = hook->cur_sequence;
    if ( !pSequence || !strstr( pSequence->animation_name, "atak" ) || strstr( pSequence->animation_name, "amb" ) )
	{
		AI_ForceSequence( self, "ataka" );
		PATHLIST_KillPath(hook->pPathList);
	}

	if( AI_RocketMpTakeCoverAttack( self ) == TRUE )
	{
		AI_ForceSequence( self, "amba" );
	}
}


// ----------------------------------------------------------------------------
// NSS[3/10/00]:This code makes me ill... *puke*
// Name:		rocketmp_set_attack_seq
// Description: sets uzi gang member in attack sequence
// Input:
// Output:
// Note:
// ----------------------------------------------------------------------------
void rocketmp_set_attack_seq( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_Dprintf( "rocketmp_set_attack_seq\n" );

	self->curWeapon = gstate->InventoryFindItem(self->inventory, "rocket_left");

	if(hook->nSpawnValue & SPAWN_DO_NOT_MOVE)
	{
		hook->nAttackMode = ATTACKMODE_RANGED;
		AI_ForceSequence( self, "ataka" );	
		return;
	}
		
	float dist = VectorDistance(self->s.origin, self->enemy->s.origin);

	if ( dist < ROCKETMP_MIN_CHASE_ATTACK_DISTANCE && rnd() < 0.50f) // and speed is over minimum
	{
		// swing knife
		hook->nAttackMode = ATTACKMODE_MELEE;
		AI_ForceSequence( self, "atakb" );
	}
	else 
	{
		if(dist > ROCKETMP_MIN_CHASE_ATTACK_DISTANCE )
		{
			hook->nAttackMode = ATTACKMODE_RANGED;
			AI_ForceSequence( self, "ataka" );
		}
		else
		{
			
			AI_AddNewTaskAtFront(self,TASKTYPE_CHASEEVADE);
			//CVector Dir,Destination;
			//Dir = self->enemy->s.origin - self->s.origin;
			//Dir.Normalize();
			//AI_FindNewCourse( self, &Dir);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		rocketmp_do_chase_attack_seq
// Description: Start/transition self to use chase (running) attack sequence.
// Input:
// Output:
// Note:
// ----------------------------------------------------------------------------
void rocketmp_do_chase_attack_seq( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	frameData_t *pSequence;

	AI_Dprintf( "rocketmp_do_chase_attack_seq\n" );

	if ( hook->attack_finished <= gstate->time )
	{
		hook->nAttackMode = ATTACKMODE_CHASE;

		// launch rockets
		pSequence = FRAMES_GetSequence( self, "atakc" );
	}
	else
	{
		hook->nAttackMode = ATTACKMODE_MELEE;

		// swing knife
		pSequence = FRAMES_GetSequence( self, "atakb" );
	}

	self->curWeapon = gstate->InventoryFindItem(self->inventory, "rocket_left");

	AI_ForceSequence( self, pSequence ); // start the desired sequence over again
}

// ----------------------------------------------------------------------------
//
// Name:		rocketmp_do_run_seq
// Description: Start/transition self to use chase (running) attack sequence.
// Input:
// Output:
// Note:
// ----------------------------------------------------------------------------
void rocketmp_do_run_seq( userEntity_t *self )
{
	AI_Dprintf( "rocketmp_do_run_seq\n" );

	frameData_t *pSequence = FRAMES_GetSequence( self, "runa" );

	AI_ForceSequence( self, pSequence ); // start the current sequence over again
}

// ----------------------------------------------------------------------------
//
// Name:		rocketmp_chasing_attack
// Description: Attack enemy while chasing him/her.
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_HasTaskInQue( userEntity_t *self, TASKTYPE SearchTask );
int rocketmp_chasing_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	float dist;

	AI_Dprintf( "rocketmp_chasing_attack\n" );

	if ( AI_IsEnemyDead( self ) )
		return 0;

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	dist = VectorDistance(self->s.origin, self->enemy->s.origin);

	// see if we can attack
	if ( AI_IsWithinAttackDistance( self, dist ) && com->Visible( self, self->enemy ) )
	{
		int atend = AI_IsEndAnimation( self );

		// eligible to attack while chasing, so check distance at end of animation
		if ( atend && dist < ROCKETMP_MIN_CHASE_ATTACK_DISTANCE)
		{
			// back to normal attacking
			hook->nAttackMode = ATTACKMODE_NORMAL;

			if(!AI_HasTaskInQue(self,TASKTYPE_ATTACK))
			{
				// not close enough to attack while chasing, so stop and attack normally
				AI_RemoveCurrentTask( self, TASKTYPE_ATTACK,self,FALSE);
			}
			else
			{
				AI_RemoveCurrentTask(self);
			}
		}
		else
		{
			if ( hook->nAttackMode != ATTACKMODE_CHASE &&
				 hook->nAttackMode != ATTACKMODE_MELEE )
			{
				rocketmp_do_chase_attack_seq( self );
			}

			AI_PlayAttackSounds( self );

			if ( AI_IsFacingEnemy( self, self->enemy ) )
			{
				if( hook->nAttackMode == ATTACKMODE_CHASE )
				{
					// fire if ready to attack
					if ( AI_IsReadyToAttack1( self ) )
					{
						self->curWeapon = gstate->InventoryFindItem(self->inventory, "rocket_left");
						ai_fire_curWeapon( self );
						self->curWeapon = gstate->InventoryFindItem(self->inventory, "rocket_right");
						hook->attack_finished = gstate->time + 2.0f;	//	won't attack for another 2 seconds
					}
					if ( AI_IsReadyToAttack2( self ) )
					{
						self->curWeapon = gstate->InventoryFindItem(self->inventory, "rocket_right");
						ai_fire_curWeapon( self );
						self->curWeapon = gstate->InventoryFindItem(self->inventory, "rocket_left");
						hook->attack_finished = gstate->time + 2.0f;	//	won't attack for another 2 seconds
					}
				}
				else
				{
					// fire off punch weapon if ready to attack
					if ( AI_IsReadyToAttack1( self ) )
					{
						self->curWeapon = gstate->InventoryFindItem(self->inventory, "punch");
						ai_fire_curWeapon( self );
						self->curWeapon = gstate->InventoryFindItem(self->inventory, "rocket_left");
					}
				}
			}

			// check for end of animation
			if ( atend )
			{
				// run chasing attack again
				rocketmp_do_chase_attack_seq( self );
			}
		}
	}
	else
	{
		// we've lost him

		// go back to running
		if ( hook->nAttackMode != ATTACKMODE_NORMAL )
		{
			AI_SetStateRunning( hook );
		}

		// back to normal attack mode
		hook->nAttackMode = ATTACKMODE_NORMAL;

		if ( AI_ShouldFollow( self ) != TRUE )
		{
			// give up attacking the enemy completely
			AI_RemoveCurrentGoal( self );
		}
	}

	return 0; // never within strafing distance
}

// ----------------------------------------------------------------------------
//
// Name:		rocketmp_begin_attack
// Description: setup for attack function
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void rocketmp_begin_attack( userEntity_t *self )
{
	AI_Dprintf( "rocketmp_begin_attack\n" );

	rocketmp_set_attack_seq( self );
}

// ----------------------------------------------------------------------------
//
// Name:		rocketmp_attack
// Description: attack function for ranged or melee chase
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void rocketmp_attack( userEntity_t *self )
{
	AI_Dprintf( "rocketmp_attack\n" );

	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_ZeroVelocity( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	if( hook->nAttackMode == ATTACKMODE_CHASE || hook->nAttackMode == ATTACKMODE_MELEE )
	{
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );

		if ( dist < hook->attack_dist )
		{
			AI_ZeroVelocity( self );
		}
		else
		{
			AI_MoveTowardPoint( self, self->enemy->s.origin, FALSE );
		}
	}

	AI_PlayAttackSounds( self );

	if ( hook->nAttackMode == ATTACKMODE_RANGED && AI_IsFacingEnemy( self, self->enemy ) )
	{
		if ( AI_IsReadyToAttack1( self ) )
		{
			self->curWeapon = gstate->InventoryFindItem(self->inventory, "rocket_left");
			ai_fire_curWeapon( self );
			self->curWeapon = gstate->InventoryFindItem(self->inventory, "rocket_right");
		}
		if ( AI_IsReadyToAttack2( self ) )
		{
			if ( ai_check_projectile_attack(self, self->enemy, (ai_weapon_t *) self->curWeapon, 10) )
			{
				self->curWeapon = gstate->InventoryFindItem(self->inventory, "rocket_right");
				ai_fire_curWeapon( self );
				self->curWeapon = gstate->InventoryFindItem(self->inventory, "rocket_left");
				hook->attack_finished = gstate->time + 2.0f;	//	won't attack for another 2 seconds
			}
			else
			{
				// Ach! We can't hit our enemy!
				AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP ); // sidestep
			}
		}
	}
	else
	{
		if ( hook->nAttackMode == ATTACKMODE_MELEE && AI_IsFacingEnemy( self, self->enemy ) &&
			AI_IsReadyToAttack1( self ) )
		{
			self->curWeapon = gstate->InventoryFindItem(self->inventory, "punch");
			ai_fire_curWeapon( self );
			self->curWeapon = gstate->InventoryFindItem(self->inventory, "rocket_left");
		}
	}

	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		/*float dist = VectorDistance( self->s.origin, self->enemy->s.origin );
		if ( dist > ROCKETMP_MIN_CHASE_ATTACK_DISTANCE || !com->Visible( self, self->enemy ) )
		{
			if ( AI_ShouldFollow( self ) == TRUE )
			{
				AI_RemoveCurrentTask( self );
			}
			else
			{
				// give up attacking the enemy
				AI_RemoveCurrentGoal( self );
			}
		}
		else
		{
			// set up the attack again
			rocketmp_set_attack_seq( self );
		}*/
		AI_RemoveCurrentTask(self,FALSE);
	}
}


// ----------------------------------------------------------------------------
//
// Name:		rocketmp_start_pain
// Description: pain function for uzi gang member that selects light or heavy hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void rocketmp_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,25);
}

// ----------------------------------------------------------------------------
//
// Name:		monster_rocketmp
// Description: init function
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_rocketmp( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_ROCKETMP );
	self->className			= "monster_rocketmp";
	self->netname			= tongue_monsters[T_MONSTER_ROCKETMP];

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

	gstate->SetSize(self, -16.0, -16.0, -24.0, 16.0, 16.0, 32.0);

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	hook->fnStartAttackFunc = rocketmp_begin_attack;
	hook->fnAttackFunc		= rocketmp_attack;
	hook->fnTakeCoverFunc	= AI_RocketMpTakeCover;

	hook->fnChasingAttack	= rocketmp_chasing_attack;
	//hook->fnInAttackRange   = rocketmp_check_range;

	hook->run_speed			= 250;
	hook->walk_speed		= 50;
	hook->attack_speed		= 250;

	hook->dflags			|= DFL_RANGEDATTACK;

	hook->attack_dist		= 10000;

	self->health			= 200;
	hook->base_health		= 200;
	hook->pain_chance		= 20;
	self->mass				= 2.0;

	self->pain              = rocketmp_start_pain;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give rocketmp a weapon
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

		self->curWeapon = ai_init_weapon( self, 
										  pWeaponAttributes[1].fBaseDamage, 
										  pWeaponAttributes[1].fRandomDamage,
										  pWeaponAttributes[1].fSpreadX,
										  pWeaponAttributes[1].fSpreadZ,
										  pWeaponAttributes[1].fSpeed,
										  pWeaponAttributes[1].fDistance,
										  pWeaponAttributes[1].offset, 
										  "rocket_left", 
										  mp_rocket_left_attack, 
										  ITF_PROJECTILE | ITF_EXPLOSIVE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );

		self->curWeapon = ai_init_weapon( self, 
										  pWeaponAttributes[2].fBaseDamage, 
										  pWeaponAttributes[2].fRandomDamage,
										  pWeaponAttributes[2].fSpreadX,
										  pWeaponAttributes[2].fSpreadZ,
										  pWeaponAttributes[2].fSpeed,
										  pWeaponAttributes[2].fDistance,
										  pWeaponAttributes[2].offset, 
										  "rocket_right", 
										  mp_rocket_right_attack, 
										  ITF_PROJECTILE | ITF_EXPLOSIVE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}
	hook->nAttackType = ATTACK_GROUND_CHASING;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	hook->strafe_time = gstate->time;
	hook->strafe_dir  = STRAFE_LEFT;

	AI_DetermineMovingEnvironment( self );
	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;

	gstate->LinkEntity(self);

	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_rocketmp_register_func()
{
	gstate->RegisterFunc("rocketmp_begin_attack",rocketmp_begin_attack);
	gstate->RegisterFunc("rocketmp_attack",rocketmp_attack);

	gstate->RegisterFunc("AI_RocketMpTakeCover",AI_RocketMpTakeCover);
	gstate->RegisterFunc("rocketmp_chasing_attack",rocketmp_chasing_attack);
	gstate->RegisterFunc("rocketmp_start_pain",rocketmp_start_pain);
}
