// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#include "world.h"
#include "ai_utils.h"
//#include "ai_move.h"
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_common.h"
#include "ai_func.h"
#include "MonsterSound.h"

/* ***************************** define types ****************************** */
#define PLAGUE_RAT_POISON_CHANCE 0.2f

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */

void plague_rat_melee_attack( userEntity_t *self );
void plague_rat_jump_attack (userEntity_t *self);
void plague_rat_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );
void plague_rat_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                           int damage, CVector &destPoint );

/* **************************** Global Variables *************************** */

/* **************************** Global Functions *************************** */
void monster_rat( userEntity_t *self, playerHook_t *hook );

/* ******************************* exports ********************************* */
DllExport void monster_plague_rat( userEntity_t *self );


// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void plague_rat_set_attack_seq( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if( self->waterlevel )
	{ 
		hook->nAttackType = ATTACK_WATER_MELEE;
		hook->nAttackMode = ATTACKMODE_NORMAL;
		AI_DetermineMovingEnvironment( self );
	}
	else
	{
		hook->nAttackType = ATTACK_GROUND_MELEE;
		hook->nAttackMode = ATTACKMODE_NORMAL;
		AI_DetermineMovingEnvironment( self );
	}

    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {
	    frameData_t *pSequence = NULL;
	    float dist = VectorDistance (self->s.origin, self->enemy->s.origin);

		self->curWeapon = gstate->InventoryFindItem(self->inventory, "punch");

	    if ( AI_IsWithinAttackDistance( self, dist ) )
	    {
		    //int i = (int)(rand() % hook->sequence_max[SEQUENCE_ATAK]);
		    pSequence = FRAMES_GetSequence( self, "ataka" );
		    AI_ForceSequence( self, pSequence );

			if ( rnd() < PLAGUE_RAT_POISON_CHANCE )
			{
				self->curWeapon = gstate->InventoryFindItem(self->inventory, "poison bite mild");
			}

		    plague_rat_melee_attack (self);
	    }
	    else
	    {
		    pSequence = FRAMES_GetSequence( self, "atakb" );
		    AI_ForceSequence(self, pSequence);

		    plague_rat_jump_attack (self);
	    }
    }
    else
    {
        AI_FaceTowardPoint( self, self->enemy->s.origin );
    }
}

// ----------------------------------------------------------------------------
//
// Name:		plague_rat_DoEvasiveAction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void plague_rat_DoEvasiveAction( userEntity_t *self )
{
	if ( rnd() > 0.5f )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP );
	}
	else
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_STRAFE );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		plague_rat_jump_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void plague_rat_jump_attack (userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	AI_FaceTowardPoint( self, self->enemy->s.origin );

	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	if ( self->s.frameInfo.frameState & FRSTATE_PLAYSOUND1 )
	{
		CVector jump_ang, vel;

		//	jump on this frame
		//NSS[11/14/99]:
		//self->groundEntity = NULL;

		jump_ang = self->s.angles;
		jump_ang.AngleToVectors(forward, right, up);

		vel = forward * hook->run_speed * 1.5;
		self->velocity = vel;
		self->velocity.z = hook->upward_vel;

		self->s.frameInfo.frameState -= FRSTATE_PLAYSOUND1;

		if ( hook->sound1 && !AI_IsSoundDisabled() )
		{
			gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex(hook->sound1), 0.85f, hook->fMinAttenuation, hook->fMaxAttenuation);
		}
	}
	
	if (self->s.frameInfo.frameState & FRSTATE_PLAYSOUND2)
	{
		//	attack on this frame
		self->s.frameInfo.frameState -= FRSTATE_PLAYSOUND2;
				
		ai_fire_curWeapon( self );
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		{
			AI_RemoveCurrentTask( self );

			return;
		}
		else
		{
			if ( rnd() > 0.5f )
			{
				plague_rat_DoEvasiveAction( self );
			}
			else
			{
				plague_rat_set_attack_seq (self);
			}
		}
	}
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
void plague_rat_melee_attack( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) && AI_IsFacingEnemy( self, self->enemy ) )
	{
		ai_fire_curWeapon( self );
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		{
			AI_RemoveCurrentTask( self );

			return;
		}
		else
		{
			if ( rnd() > 0.5f )
			{
				plague_rat_DoEvasiveAction( self );
			}
			else
			{
				plague_rat_set_attack_seq( self );
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		plague_rat_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void plague_rat_attack(userEntity_t *self)
{
//	playerHook_t *hook = AI_GetPlayerHook( self );

	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	if ( AI_IsWithinAttackDistance( self, dist ) )
	{
		plague_rat_melee_attack (self);
	}
	else
	{
		plague_rat_jump_attack (self);
	}
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
void plague_rat_begin_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	self->velocity.Zero();

	hook->attack_finished = gstate->time + 1.0;

	plague_rat_set_attack_seq( self );
}


// ----------------------------------------------------------------------------
//
// Name:		plague_rat_start_pain
// Description: pain function for plague rat that selects from 2 hit sequences
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void plague_rat_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,35);
}


// ----------------------------------------------------------------------------
//
// Name:		plague_rat_start_die
// Description: die function for plague rat that selects from 2 hit sequences
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void plague_rat_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                           int damage, CVector &destPoint )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );
	int notdeadyet = (self->deadflag == DEAD_NO);

	// call AI_StartDie, which does generic dying stuff
	AI_StartDie( self, inflictor, attacker, damage, destPoint );

	if( notdeadyet && self->deadflag == DEAD_DYING )
	{
		if( rand() & 1 )
			AI_ForceSequence( self, "diea" );
		else
			AI_ForceSequence( self, "dieb" );
	}
}


// ----------------------------------------------------------------------------
//
// Name:        monster_plague_rat
// Description: Spawn function for plague rat.
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_plague_rat( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_PLAGUERAT );

	self->className			= "monster_plague_rat";
	self->netname			= tongue_monsters[T_MONSTER_PLAGUERAT];

	monster_rat( self, hook );
}


void monster_rat( userEntity_t *self, playerHook_t *hook )
{
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

	gstate->SetSize (self, -4.0, -4.0, -24.0, 4.0, 4.0, 8.0);

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	////////////////	///////////////////////////////////////////

	hook->upward_vel		= 270.0;
	hook->run_speed			= 250;
	hook->walk_speed		= 150;
	hook->attack_speed		= 250;
	hook->max_jump_dist		= ai_max_jump_dist( hook->run_speed, hook->upward_vel );

	hook->dflags			|= DFL_JUMPATTACK;
	hook->dflags			|= DFL_AMPHIBIOUS;

	hook->jump_chance		= 30;
	hook->attack_dist		= 40;
	hook->jump_attack_dist	= 120;

	self->health			= 75;
	hook->base_health		= 75;
	hook->pain_chance		= 25;
	self->mass				= 2.0;

	hook->fnStartAttackFunc = plague_rat_begin_attack;
	hook->fnAttackFunc		= plague_rat_attack;

	self->pain              = plague_rat_start_pain;
	self->die               = plague_rat_start_die;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give plague_rat a weapon
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
										  "poison bite mild",
										  poison_bite_mild,
										  ITF_TRACE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}
	hook->nAttackType = ATTACK_GROUND_MELEE;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	AI_DetermineMovingEnvironment( self );

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
void world_ai_prat_register_func()
{
	gstate->RegisterFunc("plague_rat_start_pain",plague_rat_start_pain);
	gstate->RegisterFunc("plague_rat_start_die",plague_rat_start_die);

	gstate->RegisterFunc("plague_rat_begin_attack",plague_rat_begin_attack);
	gstate->RegisterFunc("plague_rat_attack",plague_rat_attack);
}
