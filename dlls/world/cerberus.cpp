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
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_cerberus (userEntity_t *self);
//DllExport void monster_cerberus (userEntity_t *self);


///////////////////////////////////////////////////////////////////////////////////
// cerberus specific funcitons
///////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
//
// Name:		cerberus_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void cerberus_set_attack_seq(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	hook->nAttackMode = ATTACKMODE_NORMAL;

	if ( AI_IsFacingEnemy( self, self->enemy ) )
	{
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );

		if ( dist > hook->attack_dist )
		{
			// do a jump attack
			hook->nAttackMode = ATTACKMODE_JUMP;
			frameData_t *pSequence = FRAMES_GetSequence( self, "jumpa" );
			AI_ForceSequence(self, pSequence);
		}
		else
		{
			// normal melee attack
			self->velocity.Zero();

			frameData_t *pSequence = FRAMES_GetSequence( self, rnd() < 0.5f ? "ataka" : "atakb" );
			AI_ForceSequence(self, pSequence);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		cerberus_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void cerberus_attack(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->nAttackMode == ATTACKMODE_JUMP )
	{
		if ( self->s.frameInfo.frameState & FRSTATE_PLAYSOUND1 )
		{
			CVector jump_ang, vel;

			//	jump on this frame
			self->groundEntity = NULL;

			jump_ang = self->s.angles;
			jump_ang.AngleToVectors(forward, right, up);

			vel = forward * hook->run_speed;
			self->velocity = vel;
			self->velocity.z = hook->upward_vel;

			self->s.frameInfo.frameState -= FRSTATE_PLAYSOUND1;

			if ( hook->sound1 && !AI_IsSoundDisabled() )
			{
				gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex(hook->sound1), 0.85f, hook->fMinAttenuation, hook->fMaxAttenuation);
			}
		}

		if ( AI_IsReadyToAttack1( self ) )
		{
			ai_fire_curWeapon( self );
		}
	}
	else
	{
		AI_FaceTowardPoint( self, self->enemy->s.origin );

		AI_PlayAttackSounds( self );

		if ( AI_IsReadyToAttack1( self ) && AI_IsFacingEnemy( self, self->enemy ) )
		{
			ai_fire_curWeapon( self );
		}
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
			cerberus_set_attack_seq (self);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		cerberus_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void cerberus_begin_attack(userEntity_t *self)
{
	cerberus_set_attack_seq (self);
}

// ----------------------------------------------------------------------------
//
// Name:		monster_cerberus
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_cerberus(userEntity_t *self)
{
	playerHook_t *hook	= AI_InitMonster( self, TYPE_CERBERUS );

	self->className		= "monster_cerberus";
	self->netname		= tongue_monsters[T_MONSTER_CERBERUS];

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

	hook->fnStartAttackFunc = cerberus_begin_attack;
	hook->fnAttackFunc		= cerberus_attack;
	hook->dflags			|= DFL_JUMPATTACK;

	hook->run_speed			= 250;
	hook->walk_speed		= 50;
	hook->attack_speed		= 250;
	hook->max_jump_dist		= ai_max_jump_dist( hook->run_speed, hook->upward_vel );

	hook->jump_chance		= 40;
	hook->attack_dist		= 96;
	hook->jump_attack_dist	= 150;

	self->health			= 300;
	hook->base_health		= 300;
	hook->pain_chance		= 10;
	
	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give cerberus a weapon
	///////////////////////////////////////////////////////////////////////////
 	self->inventory = gstate->InventoryNew( MEM_MALLOC );

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
										  ITF_TRACE);
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}

	hook->nAttackType = ATTACK_GROUND_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	AI_DetermineMovingEnvironment( self );
	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;

   	gstate->LinkEntity(self);
	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

	// NSS[6/7/00]:Mark this monster as a boss
	hook->dflags |= DFL_ISBOSS;

}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_cerberus_register_func()
{
	gstate->RegisterFunc("cerberus_begin_attack",cerberus_begin_attack);
	gstate->RegisterFunc("cerberus_attack",cerberus_attack);
}