// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#include "world.h"
#include "ai_utils.h"
//#include "ai_move.h"// SCG[1/23/00]: not used
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_common.h"
#include "ai_func.h"
#include "MonsterSound.h"

/* ***************************** define types ****************************** */
#define SATYR_CHASE_MIN_ATTACK_MOVEMENT_DIST 40.0f
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
int satyr_check_range( userEntity_t *self, float dist, userEntity_t *enemy );
void satyr_far_melee( userEntity_t *self );
void satyr_set_attack_seq( userEntity_t *self );

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */
DllExport void monster_satyr( userEntity_t *self );

///////////////////////////////////////////////////////////////////////////////////
// satyr specific funcitons
///////////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------------
//
// Name:		satyr_roar
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void satyr_roar( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	// killed the player, so roar
	frameData_t *pSequence = FRAMES_GetSequence( self, "ambc" );
	AI_ForceSequence(self, pSequence);
}

// ----------------------------------------------------------------------------
//
// Name:		satyr_roar_killed_enemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void satyr_roar_killed_enemy( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	// killed the player, so roar
	frameData_t *pSequence = FRAMES_GetSequence( self, "ambc" );
	AI_ForceSequence(self, pSequence);
}


// ----------------------------------------------------------------------------
//
// Name:		satyr_far_melee
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void satyr_far_melee( userEntity_t *self )
{
	AI_Dprintf( "far_melee\n" );

//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	if ( AI_IsReadyToAttack2( self ) )
    {
		ai_fire_curWeapon( self );
	}

	// attack without any transitions
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
			satyr_set_attack_seq( self );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		satyr_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void satyr_begin_attack( userEntity_t *self )
{
	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	playerHook_t *hook = AI_GetPlayerHook( self );

	if( dist < ((ai_weapon_t *)self->curWeapon)->distance )
	{
		// melee attack
		self->velocity.Zero();
		satyr_set_attack_seq( self );
		self->lastAIFrame = 0;
	}
	else
	{
		// chase attack
		if( strcmp( hook->cur_sequence->animation_name, "atakc" ) )
			AI_StartSequence( self, "atakc", FRAME_LOOP );
		hook->nAttackMode = ATTACKMODE_CHASE;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		satyr_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void satyr_set_attack_seq( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_Dprintf( "\n" );

	hook->nAttackMode = ATTACKMODE_NORMAL;

	if( rand() % 2 )
	{
		if( !strcmp( hook->cur_sequence->animation_name, "atakb" ) )
			AI_ForceSequence( self, "transa" );
		AI_StartSequence(self, "ataka" );
	}
	else
	{
		if( !strcmp( hook->cur_sequence->animation_name, "ataka" ) )
			AI_ForceSequence( self, "transb" );
		AI_StartSequence(self, "atakb" );
	}

}

// ----------------------------------------------------------------------------
//
// Name:		satyr_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void satyr_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	if( hook->nAttackMode == ATTACKMODE_CHASE )
	{
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );

		if ( dist < SATYR_CHASE_MIN_ATTACK_MOVEMENT_DIST )
		{
			AI_ZeroVelocity( self );
		}
		else
		{
			AI_MoveTowardPoint( self, self->enemy->s.origin, FALSE );
		}
	}

	AI_PlayAttackSounds( self );

	if( AI_IsFacingEnemy(self, self->enemy, 5.0f))
	{
		if( AI_IsReadyToAttack1( self ) || AI_IsReadyToAttack2( self ) )
			ai_fire_curWeapon( self );
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance(self->s.origin, self->enemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		{
        	AI_RemoveCurrentTask( self );
		}
        else
			satyr_set_attack_seq( self );
	}

	self->lastAIFrame = self->s.frame;
}


// ----------------------------------------------------------------------------
// NSS[1/22/00]:
// Name:		satyr_start_pain
// Description: pain function for satyr that selects front or back hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void satyr_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,35);
}


// ----------------------------------------------------------------------------
//
// Name:		satyr_check_range
// Description: Checks attack range.
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

int satyr_check_range( userEntity_t *self, float dist, userEntity_t *enemy )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	return (dist < hook->attack_dist);
}


// ----------------------------------------------------------------------------
//
// Name:		monster_satyr
// Description:
//	spawns a satyr during level load
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_satyr( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_SATYR );
	
	self->className			= "monster_satyr";
	self->netname			= tongue_monsters[T_MONSTER_SATYR];

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

	hook->run_speed			= 250;
	hook->walk_speed		= 120;
	hook->attack_dist		= 250;
	hook->pain_chance		= 20;

	self->health			= 200;
	hook->base_health		= 200;
	self->mass				= 2.0;

	hook->fnStartAttackFunc = satyr_begin_attack;
	hook->fnAttackFunc		= satyr_attack;
	hook->fnInAttackRange   = satyr_check_range;

	self->pain				= satyr_start_pain;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.1;

	self->s.render_scale.Set(1.3, 1.3, 1.3 );

	//////////////////////////////////////////////
	//	give satyr its melee weapon
	//////////////////////////////////////////////
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

	hook->pain_chance		= 5;

	AI_DetermineMovingEnvironment( self );

	gstate->LinkEntity(self);

	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

}


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_satyr_register_func()
{
	gstate->RegisterFunc("satyr_start_pain",satyr_start_pain);
	gstate->RegisterFunc("satyr_begin_attack",satyr_begin_attack);
	gstate->RegisterFunc("satyr_attack",satyr_attack);
	gstate->RegisterFunc("satyr_check_range",satyr_check_range);
}
