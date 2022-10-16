// ==========================================================================
//
//  File:
//  Contents:
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

/* ***************************** define types ****************************** */

// distance at which female gang member attacks
#define FEMGANG_ATTACK_DIST 200.0f

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport	void	monster_femgang (userEntity_t *self);


// ----------------------------------------------------------------------------
//
// Name:		femgang_check_range
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

int femgang_check_range( userEntity_t *self, float dist, userEntity_t *enemy )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );

	return dist < FEMGANG_ATTACK_DIST;
}


// ----------------------------------------------------------------------------
//
// Name:		femgang_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void femgang_set_attack_seq( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	hook->nAttackMode = ATTACKMODE_NORMAL;

    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {
	    playerHook_t *hook = AI_GetPlayerHook( self );

		float dist = VectorDistance(self->s.origin, self->enemy->s.origin);

		if ( dist < ((ai_weapon_t *)self->curWeapon)->distance )
		{
			frameData_t *pSequence = FRAMES_GetSequence( self, "ataka" );
			AI_ForceSequence( self, pSequence );

			AI_ZeroVelocity( self );
		}
		else
		{
			frameData_t *pSequence = FRAMES_GetSequence( self, "atakb" );
			AI_ForceSequence( self, pSequence );

			hook->nAttackMode = ATTACKMODE_CHASE;
		}
    }
}

// ----------------------------------------------------------------------------
//
// Name:		femgang_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void femgang_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->nAttackMode == ATTACKMODE_CHASE )
	{
		AI_MoveTowardPoint( self, self->enemy->s.origin, FALSE );
	}
	else
	{
		AI_ZeroVelocity( self );

		AI_FaceTowardPoint( self, self->enemy->s.origin );
	}

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) && AI_IsFacingEnemy( self, self->enemy ) )
	{
		ai_fire_curWeapon( self );
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible (self, self->enemy))
		{
			AI_RemoveCurrentTask( self );
		}
		else
		{
			femgang_set_attack_seq (self);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		femgang_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void femgang_begin_attack( userEntity_t *self )
{
	if ( AI_IsEnemyTargetingMe( self, self->enemy ) && rnd() > 0.5f )
	{
		AI_DoEvasiveAction( self );
	}
	else
	{
//		playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

		femgang_set_attack_seq(self);
	}
}

// ----------------------------------------------------------------------------
//
// Name:        femgang_start_idle
// Description: Start idle function that selects between two seqs
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void femgang_start_idle( userEntity_t *self )
{
    frameData_t *pSequence;
	float chance = rnd();

	if ( chance < 0.85 )
	{
		pSequence = FRAMES_GetSequence( self, "amba" );
	}
	else
	{
		// play ambb occasionally
		pSequence = FRAMES_GetSequence( self, "ambb" );
	}

    if ( pSequence )
    {
        if ( AI_StartSequence(self, pSequence, pSequence->flags) == FALSE )
		{
			return;
		}
    }

	AI_StartIdleSettings( self );
}

// ----------------------------------------------------------------------------
//
// Name:		femgang_start_pain
// Description: pain function for femgang that selects front or back hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void femgang_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,35);
}

// ----------------------------------------------------------------------------
//
// Name:		femgang_start_die
// Description: die function for femgang
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void femgang_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                     int damage, CVector &destPoint )
{
	int notdeadyet = (self->deadflag == DEAD_NO);

	// call AI_StartDie, which does generic dying stuff
	AI_StartDie( self, inflictor, attacker, damage, destPoint );

    if ( notdeadyet && self->deadflag == DEAD_DYING )
	{ // pick a different sequence for different deaths
		if ( rnd() < 0.5f )
		{
			AI_ForceSequence( self, "diea" ); // fall over slowly
		}
		else
		{
			AI_ForceSequence( self, "dieb" ); // collapse
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		monster_femgang
// Description: spawns a female gang member during level load
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_femgang (userEntity_t *self)
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_FEMGANG );

	self->className			= "monster_femgang";
	self->netname			= tongue_monsters[T_MONSTER_FEMGANG];

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

	gstate->SetSize( self, -16.0, -16.0, -24.0, 16.0, 16.0, 32.0 );

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	////////////////	///////////////////////////////////////////

	hook->upward_vel		= 270.0;
	hook->run_speed			= 250;
	hook->walk_speed		= 100;
	hook->attack_speed		= 250;
	hook->max_jump_dist		= ai_max_jump_dist( hook->run_speed, hook->upward_vel );

	hook->attack_dist		= 80;

	self->health			= 200;
	hook->base_health		= 200;
	hook->pain_chance		= 10;

	hook->fnStartIdle       = femgang_start_idle;
	self->die				= femgang_start_die;
	self->pain				= femgang_start_pain;
	hook->pain_chance		= 20;

	hook->fnStartAttackFunc = femgang_begin_attack;
	hook->fnAttackFunc		= femgang_attack;

	hook->fnInAttackRange   = femgang_check_range;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give femgang a weapon
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
	hook->nAttackType = ATTACK_GROUND_MELEE;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	AI_DetermineMovingEnvironment( self );
	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;
	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

	gstate->LinkEntity(self);
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
void world_ai_femgang_register_func()
{
	gstate->RegisterFunc("femgang_start_pain",femgang_start_pain);
	gstate->RegisterFunc("femgang_start_die",femgang_start_die);

	gstate->RegisterFunc("femgang_start_idle",femgang_start_idle);
	gstate->RegisterFunc("femgang_begin_attack",femgang_begin_attack);
	gstate->RegisterFunc("femgang_attack",femgang_attack);
	gstate->RegisterFunc("femgang_check_range",femgang_check_range);
}
