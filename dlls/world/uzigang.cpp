// ==========================================================================
//
//  File:     uzigang.cpp
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

#define UZIGANG_MIN_CHASE_ATTACK_DISTANCE 200.0f
#define UZIGANG_BLOWN_AWAY_MIN_DAMAGE 10
#define UZIGANG_BLOWN_AWAY_ANGLECOS 0.707f
#define UZIGANG_HEAVY_HIT_DAMAGE 50

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */

int uzigang_chase_attack_seq_active( userEntity_t *self );
void uzigang_set_attack_seq( userEntity_t *self );
void uzigang_do_chase_attack_seq( userEntity_t *self );
int uzigang_chasing_attack( userEntity_t *self );
void uzigang_begin_attack( userEntity_t *self );
void uzigang_ranged_attack( userEntity_t *self );
void uzigang_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );
void uzigang_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                        int damage, CVector &destPoint );

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* **************************** Extern Functions *************************** */

void AI_StartDie( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &destPoint );
void AI_StartPain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );

/* ******************************* exports ********************************* */

DllExport void monster_uzigang( userEntity_t *self );


// ----------------------------------------------------------------------------
//
// Name:		uzigang_chase_attack_seq_active
// Description: Determines if the chase attack sequence is currently running.
// Input:
// Output:
// Note:
// ----------------------------------------------------------------------------
int uzigang_chase_attack_seq_active( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	frameData_t *pSequence = FRAMES_GetSequence( self, "atakb" );

	return ( hook->cur_sequence && hook->cur_sequence == pSequence );
}

// ----------------------------------------------------------------------------
//
// Name:		uzigang_set_attack_seq
// Description: sets uzi gang member in attack sequence
// Input:
// Output:
// Note:
// ----------------------------------------------------------------------------
void uzigang_set_attack_seq( userEntity_t *self )
{
    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {
//	    playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	    frameData_t *pSequence = FRAMES_GetSequence( self, "ataka" );

		AI_ForceSequence( self, pSequence );
    }
}

// ----------------------------------------------------------------------------
//
// Name:		uzigang_do_chase_attack_seq
// Description: Start/transition self to use chase (running) attack sequence.
// Input:
// Output:
// Note:
// ----------------------------------------------------------------------------
void uzigang_do_chase_attack_seq( userEntity_t *self )
{
	float dist = VectorDistance(self->s.origin, self->enemy->s.origin);

	// attack instead of chasing?
	if ( dist < UZIGANG_MIN_CHASE_ATTACK_DISTANCE )
	{
		// not close enough to attack while chasing, so stop and attack normally
		AI_AddNewTaskAtFront( self, TASKTYPE_ATTACK );
	}
	else
	{
		frameData_t *pSequence = FRAMES_GetSequence( self, "atakb" );

		AI_ForceSequence( self, pSequence ); // start the current sequence over again
	}
}


// ----------------------------------------------------------------------------
//
// Name:		uzigang_chasing_attack
// Description: Attack enemy while chasing him/her.
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int uzigang_chasing_attack( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
//	float dist;// SCG[1/23/00]: not used

	if ( AI_IsEnemyDead( self ) )
	{
		return 0;
	}

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	// see if we can switch to attacking
//	dist = VectorDistance(self->s.origin, self->enemy->s.origin);// SCG[1/23/00]: not used

	// check whether to end attack
	if ( !AI_IsVisible( self, self->enemy ) )
	{
		// we've lost him

		if ( AI_ShouldFollow( self ) == TRUE )
		{
			// give up current attack
			AI_RemoveCurrentTask( self );
		}
		else
		{
			// give up attacking the enemy completely
			AI_RemoveCurrentGoal( self );
		}
	}
	else
	{
		if ( uzigang_chase_attack_seq_active( self ) )
		{
			AI_PlayAttackSounds( self );

			// fire if ready to attack
			if ( (AI_IsReadyToAttack1( self ) || AI_IsReadyToAttack2( self )) &&
				 ai_check_projectile_attack(self, self->enemy, (ai_weapon_t *) self->curWeapon, 10))
			{
				ai_fire_curWeapon( self );
			}
		}
		else
		{
			uzigang_do_chase_attack_seq( self );
		}

		// check for end of animation
		if ( AI_IsEndAnimation( self ) )
		{
			// run chasing attack
			uzigang_do_chase_attack_seq( self );
		}
	}

	return 0; // never within strafing distance
}

// ----------------------------------------------------------------------------
//
// Name:		uzigang_begin_attack
// Description: setup for attack function
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void uzigang_begin_attack( userEntity_t *self )
{
	uzigang_set_attack_seq( self );
}

// ----------------------------------------------------------------------------
//
// Name:		uzigang_ranged_attack
// Description: attack function for uzi gang member
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void uzigang_ranged_attack( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_ZeroVelocity( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) || AI_IsReadyToAttack2( self ) )
	{
		if( ai_check_projectile_attack(self, self->enemy, (ai_weapon_t *) self->curWeapon, 10) )
		{
			ai_fire_curWeapon( self );
		}
		else
		{
			// Ach! We can't hit our enemy!
			AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP ); // sidestep
		}
	}

	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );

		if ( AI_IsWithinAttackDistance( self, dist ) || !AI_IsVisible( self, self->enemy ) )
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
			uzigang_set_attack_seq( self );
		}
	}
}


// ----------------------------------------------------------------------------
//
// Name:		uzigang_start_pain
// Description: pain function for uzi gang member that selects light or heavy hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void uzigang_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,UZIGANG_HEAVY_HIT_DAMAGE);
}

// ----------------------------------------------------------------------------
//
// Name:		uzigang_start_die
// Description: die function for uzi gang member that may spawn uzi arm
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void uzigang_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                        int damage, CVector &destPoint )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	int notdeadyet = (self->deadflag == DEAD_NO);

	// call AI_StartDie, which does generic dying stuff
	AI_StartDie( self, inflictor, attacker, damage, destPoint );

	// pick a different sequence than "diea" if we aren't spawning
	if ( notdeadyet && self->deadflag == DEAD_DYING )
	{
		// see from the dot product whether we're blown away or just plain old dead
		if ( damage > UZIGANG_BLOWN_AWAY_MIN_DAMAGE &&
			 AI_GetDirDot( self, inflictor ) > UZIGANG_BLOWN_AWAY_ANGLECOS )
		{
			AI_ForceSequence( self, "diea" ); // jump back (blown away from in front)
		}
		else
		{
			AI_ForceSequence( self, "dieb" ); // crumple
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		monster_uzigang
// Description: init function
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_uzigang( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_UZIGANG );
	self->className			= "monster_uzigang";
	self->netname			= tongue_monsters[T_MONSTER_UZIGANG];

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

	hook->fnStartAttackFunc = uzigang_begin_attack;
	hook->fnAttackFunc		= uzigang_ranged_attack;

	hook->fnChasingAttack	= uzigang_chasing_attack;

	hook->run_speed			= 250;
	hook->walk_speed		= 50;
	hook->attack_speed		= 250;

	hook->dflags			|= DFL_RANGEDATTACK;

	hook->attack_dist		= 5000;

	self->health			= 200;
	hook->base_health		= 200;
	hook->pain_chance		= 20;
	self->mass				= 2.0;

	self->pain              = uzigang_start_pain;
	self->die               = uzigang_start_die;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give uzigang a weapon
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
										  uzigun_fire, 
										  ITF_BULLET );
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
void world_ai_uzigang_register_func()
{
	gstate->RegisterFunc("uzigang_start_pain",uzigang_start_pain);
	gstate->RegisterFunc("uzigang_start_die",uzigang_start_die);

	gstate->RegisterFunc("uzigang_begin_attack",uzigang_begin_attack);
	gstate->RegisterFunc("uzigang_ranged_attack",uzigang_ranged_attack);
	gstate->RegisterFunc("uzigang_chasing_attack",uzigang_chasing_attack);
}
