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
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_sdiver (userEntity_t *self);

// ----------------------------------------------------------------------------
//
// Name:		sdiver_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sdiver_set_attack_seq( userEntity_t *self )
{
    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {
	    playerHook_t *hook = AI_GetPlayerHook( self );

	    frameData_t *pSequence;
		switch( rand() % 3 )
		{
			case 0:
				pSequence = FRAMES_GetSequence( self, "ataka" );
				break;
			case 1:
				pSequence = FRAMES_GetSequence( self, "atakb" );
				break;				
			case 2:
				pSequence = FRAMES_GetSequence( self, "atakc" );
				break;
		}
	    AI_ForceSequence(self, pSequence);
    }
}

// ----------------------------------------------------------------------------
//
// Name:		sdiver_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sdiver_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	AI_ZeroVelocity( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) && AI_IsFacingEnemy( self, self->enemy ) )
	{
		ai_fire_curWeapon( self );
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		{
			AI_RemoveCurrentTask( self );

			return;
		}
		else
		{
			sdiver_set_attack_seq (self);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		sdiver_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sdiver_begin_attack( userEntity_t *self )
{
	sdiver_set_attack_seq( self );
}


// ----------------------------------------------------------------------------
//
// Name:		sdiver_start_pain
// Description: pain function for sdiver that selects front or back hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sdiver_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_StartPain( self, attacker, kick, damage );

	if( AI_IsAlive( self ) && hook->pain_finished >= gstate->time )
	{
		if( damage >= 35 )
			AI_ForceSequence( self, "hita" );
		else
			AI_ForceSequence( self, "hitb" );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		sdiver_start_die
// Description: die function for sdiver
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sdiver_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
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
// Name:		monster_sdiver
// Description:
//				spawns a sdivereton during level load
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_sdiver( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_SDIVER );

	self->className			= "monster_sdiver";
	self->netname			= tongue_monsters[T_MONSTER_SEALDIVER];

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

	hook->upward_vel	= 270.0;
	hook->run_speed		= 250;
	hook->walk_speed	= 100;
	hook->attack_speed	= 250;
	hook->max_jump_dist = ai_max_jump_dist( hook->run_speed, hook->upward_vel );

	hook->attack_dist	= 80;

	self->health		= 200;
	hook->base_health	= 200;
	hook->pain_chance	= 10;
	self->mass			= 2.0;

	hook->fnStartAttackFunc = sdiver_begin_attack;
	hook->fnAttackFunc		= sdiver_attack;

	self->die				= sdiver_start_die;
	self->pain				= sdiver_start_pain;
	hook->pain_chance		= 20;

	self->think			= AI_ParseEpairs;
	self->nextthink		= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give sdiver a weapon
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
										  "swing", 
										  melee_swing, 
										  ITF_TRACE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}
	hook->nAttackType = ATTACK_GROUND_MELEE;
	hook->nAttackMode = ATTACKMODE_NORMAL;

    AI_DetermineMovingEnvironment( self );

	gstate->LinkEntity(self);
}


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_sdiver_register_func()
{
	gstate->RegisterFunc("sdiver_start_pain",sdiver_start_pain);
	gstate->RegisterFunc("sdiver_start_die",sdiver_start_die);
}
