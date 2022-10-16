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
//#include "ai_move.h"	// SCG[1/23/00]: not used
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

DllExport void monster_stavros(userEntity_t *self);

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
void stavros_set_attack_seq( userEntity_t *self )
{
    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {
//	    playerHook_t *hook = AI_GetPlayerHook( self );	// SCG[1/23/00]: not referenced

	    frameData_t *pSequence = FRAMES_GetSequence( self, "ataka" );
	    AI_ForceSequence( self, pSequence );
    }
    else
    {
        AI_FaceTowardPoint( self, self->enemy->s.origin );
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
void stavros_attack( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );	// SCG[1/23/00]: not referenced

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) && AI_IsFacingEnemy( self, self->enemy ) )
	{
		ai_fire_curWeapon( self );
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible (self, self->enemy) )
		{
			AI_RemoveCurrentTask( self );
		}
		else
		{
			stavros_set_attack_seq( self );
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
void stavros_begin_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	self->velocity.Zero();

	hook->attack_finished = gstate->time + 1.0;

	stavros_set_attack_seq( self );
}

// ----------------------------------------------------------------------------
//
// Name:		stavros_start_pain
// Description: pain function for stavros
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void stavros_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	if(self->health <=0)
	{
		int poop = 1;
	}
	ai_generic_pain_handler( self, attacker, kick,damage,35);
}

///////////////////////////////////////////////////////////////////////////////
// NSS[6/2/00]:
// stavros_think
// Quick Hack
///////////////////////////////////////////////////////////////////////////////
void stavros_think(userEntity_t *self)
{
	AI_TaskThink(self);
}

///////////////////////////////////////////////////////////////////////////////
// NSS[6/2/00]:
//  stavros_parseEpairs
// Quick Hack
///////////////////////////////////////////////////////////////////////////////
void stavros_parseEpairs(userEntity_t *self)
{
	AI_ParseEpairs(self);
	self->think = stavros_think;
	self->nextthink = gstate->time + 0.1f;
}


// ----------------------------------------------------------------------------
//
// Name:		monster_stavros
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_stavros( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_STAVROS );

	self->className			= "monster_stavros";
	self->netname			= tongue_monsters[T_MONSTER_STAVROS];

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

	self->movetype			= MOVETYPE_WALK;
	hook->dflags			|= DFL_FACEPITCH;

	hook->fnStartAttackFunc = stavros_begin_attack;
	hook->fnAttackFunc		= stavros_attack;
	self->pain				= stavros_start_pain;

	hook->run_speed			= 250;
	hook->walk_speed		= 120;
	hook->attack_speed		= 160;
	hook->pain_chance		= 20;

	self->think				= stavros_parseEpairs;
	self->nextthink			= gstate->time + 0.2;

	///////////////////////////////////////////////////////////////////////////
	//
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
										  "stave", 
										  stave_attack, 
										  ITF_TRACE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}
	hook->nAttackType = ATTACK_AIR_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	AI_DetermineMovingEnvironment( self );
	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;

	hook->pain_chance		= 1;

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
void world_ai_stavros_register_func()
{
	gstate->RegisterFunc("stavros_begin_attack",stavros_begin_attack);
	gstate->RegisterFunc("stavros_attack",stavros_attack);
	gstate->RegisterFunc("stavros_start_pain",stavros_start_pain);
	gstate->RegisterFunc("stavros_think",stavros_think);
	gstate->RegisterFunc("stavros_parseEpairs",stavros_parseEpairs);
}
