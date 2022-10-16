// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#include "world.h"
#include "nodelist.h"
//#include "ai_move.h"// SCG[1/23/00]: not used
#include "ai_weapons.h"
#include "ai_frames.h"
#include "ai_utils.h"
#include "nodelist.h"
#include "ai_common.h"
#include "ai_func.h"
#include "MonsterSound.h"

/* ***************************** define types ****************************** */

#define BPRIS_HEAVY_HIT_DAMAGE 40
#define BPRIS_ATTACK_EVASIVE_CHANCE 0.3f
#define BPRIS_MIN_COLLAPSING_DEATH_DAMAGE 20
#define BPRIS_COLLAPSING_DEATH_CHANCE 0.3f
#define BPRIS_MIN_ROCKTHROW_DIST 350.0f
#define BPRIS_MAX_ROCKTHROW_DIST 500.0f
#define BPRIS_ROCKTHROW_CHANCE 0.1f

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */

void blackpris_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );
void blackpris_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                          int damage, CVector &destPoint );
int blackpris_check_range(userEntity_t *self, float dist, userEntity_t *enemy);

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* **************************** Extern Functions *************************** */

void AI_StartPain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );
void AI_StartDie( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &destPoint );

/* ******************************* exports ********************************* */

DllExport void monster_blackprisoner (userEntity_t *self);

// ----------------------------------------------------------------------------
//
// Name:		blackpris_check_range
// Description: Checks given distance to determine if it's in range.
// Input:
// Output:
// Note:        The enemy pointer will usually be NULL.
//
// ----------------------------------------------------------------------------
int blackpris_check_range(userEntity_t *self, float dist, userEntity_t *enemy)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( dist <= hook->attack_dist )
	{
		return TRUE;
	}

	if ( dist >= BPRIS_MIN_ROCKTHROW_DIST && dist <= BPRIS_MAX_ROCKTHROW_DIST && rnd() < BPRIS_ROCKTHROW_CHANCE)
	{
		 return TRUE;
	}

	return FALSE;
}


// ----------------------------------------------------------------------------
//
// Name:		blackpris_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void blackpris_set_attack_seq( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	float dist = VectorDistance( self->s.origin, self->enemy->s.origin );

	if ( dist < (BPRIS_MIN_ROCKTHROW_DIST + hook->attack_dist) * 0.5f )
	{
		float chance = rnd();

		// pick punch
		self->curWeapon = gstate->InventoryFindItem( self->inventory, "punch" );

		// pick a melee attack and play it
		if(chance < 0.33f)
		{
			AI_ForceSequence( self, FRAMES_GetSequence( self, "ataka" ) );
		}
		else if(chance < 0.666f)
		{
			AI_ForceSequence( self, FRAMES_GetSequence( self, "atakb" ) );
		}
		else
		{
			AI_ForceSequence( self, FRAMES_GetSequence( self, "atakc" ) );
		}
	}
	else
	{
		// pick rock throw
		self->curWeapon = gstate->InventoryFindItem( self->inventory, "rock throw" );

		// set ranged attack sequence
		AI_ForceSequence( self, FRAMES_GetSequence( self, "atakd" ) );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		blackpris_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void blackpris_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_ZeroVelocity( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if ( (AI_IsReadyToAttack1( self ) || AI_IsReadyToAttack2( self )) && AI_IsFacingEnemy( self, self->enemy ) )
	{
		ai_fire_curWeapon( self );
	}

	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );

		// Can we melee attack?
		if ( dist > hook->attack_dist || !AI_IsVisible(self, self->enemy) )
		{
			// stop attacking for now
			AI_AddNewTaskAtFront(self,TASKTYPE_CHASE);
			AI_StartNextTask( self );
		}
		else
		{
			if ( AI_IsEnemyTargetingMe( self, self->enemy ) && AI_CanMove( hook ) &&
			     rnd() < BPRIS_ATTACK_EVASIVE_CHANCE )
			{
				AI_DoEvasiveAction( self );
			}
			else
			{
				blackpris_set_attack_seq( self );
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		blackpris_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void blackpris_begin_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	hook->attack_finished = gstate->time + 1.0;

	blackpris_set_attack_seq( self );
}


// ----------------------------------------------------------------------------
//
// Name:		blackpris_start_pain
// Description: pain function for black prisoner that selects light or heavy hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void blackpris_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,35);
}

// ----------------------------------------------------------------------------
//
// Name:		blackpris_start_die
// Description: die handler for black prisoner, selects falling or collapsing death
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void blackpris_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                          int damage, CVector &destPoint )
{
	int notdeadyet = (self->deadflag == DEAD_NO);

	// call AI_StartDie, which does generic dying stuff
	AI_StartDie( self, inflictor, attacker, damage, destPoint );

    if ( notdeadyet && self->deadflag == DEAD_DYING )
	{ // pick a different sequence for different deaths
		if ( damage < BPRIS_MIN_COLLAPSING_DEATH_DAMAGE || rnd() < BPRIS_COLLAPSING_DEATH_CHANCE )
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
// Name:		monster_blackprisoner
// Description: init function
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_blackprisoner (userEntity_t *self)
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_BLACKPRIS );

	self->className			= "monster_blackprisoner";
	self->netname			= tongue_monsters[T_MONSTER_PRISONER_BLACK];

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
	///////////////////////////////////////////////////////////

	hook->upward_vel		= 270.0;
	hook->run_speed			= 250;
	hook->walk_speed		= 150;
	hook->attack_speed		= 250;
	hook->max_jump_dist		= ai_max_jump_dist( hook->run_speed, hook->upward_vel );

	hook->attack_dist       = 80;

	self->health			= 200;
	hook->base_health		= 200;
	hook->pain_chance		= 10;
	

	hook->fnStartAttackFunc = blackpris_begin_attack;
	hook->fnAttackFunc		= blackpris_attack;

	hook->fnInAttackRange   = blackpris_check_range;

	self->pain              = blackpris_start_pain;
	self->die               = blackpris_start_die;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give blackpris a weapon
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
										  "rock throw", 
										  rock_throw, 
										  ITF_PROJECTILE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}

	hook->nAttackType = ATTACK_GROUND_MELEE;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	AI_DetermineMovingEnvironment( self );
	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;

	gstate->LinkEntity(self);
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
void world_ai_blackpris_register_func()
{
	gstate->RegisterFunc("blackpris_start_pain",blackpris_start_pain);
	gstate->RegisterFunc("blackpris_start_die",blackpris_start_die);

	gstate->RegisterFunc("blackpris_begin_attack",blackpris_begin_attack);
	gstate->RegisterFunc("blackpris_attack",blackpris_attack);
	gstate->RegisterFunc("blackpris_check_range",blackpris_check_range);
}
