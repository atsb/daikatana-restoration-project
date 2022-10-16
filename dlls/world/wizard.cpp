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
#include "ai_move.h"
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"


/* ***************************** define types ****************************** */

#define NHARRE_SUMMON_CHANCE 0.27f
#define NHARRE_TELEPORT_DISTANCE 200.0f

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_wizard(userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void wizard_set_attack_seq( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	// choose whether to teleport, idle, or summon

	float dist = VectorDistance( self->s.origin, self->enemy->s.origin );

	if ( dist < NHARRE_TELEPORT_DISTANCE )
	{
		// teleport
		hook->nAttackMode = ATTACKMODE_TAKECOVER;

		frameData_t *pSequence = FRAMES_GetSequence( self, "deathb" );
		AI_ForceSequence( self, pSequence );
	}
	else if ( rnd() > NHARRE_SUMMON_CHANCE )
	{
		// idle
		hook->nAttackMode = ATTACKMODE_NORMAL;

		frameData_t *pSequence = FRAMES_GetSequence( self, "amba" );
		AI_ForceSequence( self, pSequence );
	}
	else
	{
		// summon
		hook->nAttackMode = ATTACKMODE_RANGED;

		frameData_t *pSequence = FRAMES_GetSequence( self, "atakb" );
		AI_ForceSequence( self, pSequence );
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
void wizard_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->nAttackMode == ATTACKMODE_NORMAL )
	{
		ai_frame_sounds( self );

		if ( AI_IsEndAnimation( self ) )
		{
			wizard_set_attack_seq( self );
		}
	}
	else if ( hook->nAttackMode == ATTACKMODE_TAKECOVER )
	{
		ai_frame_sounds( self );

		if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
		{
			// teleport to a different location

			// set a new sequence
			wizard_set_attack_seq( self );
		}
	}
	else if ( hook->nAttackMode == ATTACKMODE_RANGED )
	{
		AI_FaceTowardPoint( self, self->enemy->s.origin );

		AI_PlayAttackSounds( self );

		if ( AI_IsReadyToAttack1( self ) && AI_IsFacingEnemy( self, self->enemy ) )
		{
			// "fire" our summoning "weapon"
			ai_fire_curWeapon( self );
		}

		if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
		{
			wizard_set_attack_seq( self );
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
void wizard_begin_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	self->velocity.Zero();

	hook->attack_finished = gstate->time + 1.0;

	wizard_set_attack_seq( self );
}


// ----------------------------------------------------------------------------
//
// Name:		monster_wizard
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_wizard( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_WIZARD );

	self->className			= "monster_wizard";
	self->netname			= tongue_monsters[T_MONSTER_WIZARD];

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

	hook->fnStartAttackFunc = wizard_begin_attack;
	hook->fnAttackFunc		= wizard_attack;

	hook->run_speed			= 250;
	hook->walk_speed		= 120;
	hook->attack_speed		= 160;
	hook->pain_chance		= 20;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;

	// the wizard doesn't really move, he just teleports
	self->movetype          = MOVETYPE_NONE;

	//////////////////////////////////////////////
	//////////////////////////////////////////////
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
//										  "nharre summon", 
//										  nharre_summon, 
										  ITF_TRACE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}

	hook->nAttackType = ATTACK_GROUND_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;

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
