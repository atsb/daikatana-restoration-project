// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#if _MSC_VER
#include <crtdbg.h>
#endif
#include "world.h"
#include "ai_common.h"
#include "ai_utils.h"
//#include "ai_move.h"// SCG[1/23/00]: not used
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"
//#include "ai_aim.h"// SCG[1/23/00]: not used

/* ***************************** define types ****************************** */

#define MONSTERNAME			"monster_venomvermin"

const float VERMIN_MELEE_DISTANCE   = 40.0f;
const float VERMIN_JUMP_DISTANCE    = 192.0f;
const float VERMIN_RANGED_DISTANCE  = 400.0f;

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
void vermin_ranged_attack(userEntity_t *self);
void vermin_melee_attack(userEntity_t *self);
void vermin_jump_attack(userEntity_t *self);

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* ******************************* exports ********************************* */
DllExport void monster_venomvermin(userEntity_t *self);


///////////////////////////////////////////////////////////////////////////////////
// vermin specific functions
///////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
//
// Name:        vermin_DetermineAttackMode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static int vermin_DetermineAttackMode( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

    if ( !AI_IsAlive( self->enemy ) )
    {
        hook->nAttackMode = ATTACKMODE_NORMAL;
    }
    else
    {
	    float fDist = VectorDistance( self->s.origin, self->enemy->s.origin );
	    if ( fDist <= VERMIN_MELEE_DISTANCE )
	    {
		    hook->nAttackMode = ATTACKMODE_MELEE;
	    }
	    else
	    if ( fDist <= VERMIN_JUMP_DISTANCE )
	    {
		    hook->nAttackMode = ATTACKMODE_JUMP;
	    }
	    else
	    if ( fDist <= VERMIN_RANGED_DISTANCE )
	    {
		    hook->nAttackMode = ATTACKMODE_RANGED;
	    }
    }

    return hook->nAttackMode;
}


// ----------------------------------------------------------------------------
//
// Name:		vermin_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void vermin_set_attack_seq( userEntity_t *self )
{
	int nAttackMode = vermin_DetermineAttackMode( self );
	
	float dist = VectorDistance( self->s.origin, self->enemy->s.origin );
	AI_Dprintf( "%g\n", dist );

	AI_FaceTowardPoint( self, self->enemy->s.origin );
	
	if( AI_IsFacingEnemy(self, self->enemy, 5.0f, 10.0f) )
	{
		switch ( nAttackMode )
		{
			case ATTACKMODE_MELEE:
			{
				AI_StartSequence( self, "ataka" );
				self->curWeapon = gstate->InventoryFindItem( self->inventory, "poison bite mild" );
				break;
			}
			case ATTACKMODE_JUMP:
			{
				AI_StartSequence( self, "runa" );
				self->curWeapon = gstate->InventoryFindItem( self->inventory, "poison bite mild" );
				break;
			}
			case ATTACKMODE_RANGED:
			{
				AI_StartSequence( self, "atakc" );
				self->curWeapon = gstate->InventoryFindItem( self->inventory, "rocket" );
				break;
			}
			default:
			{
				break;
			}
		}
	}
	else
		//AI_ForceSequence( self, "amba" );
		AI_StartSequence( self, "amba" );
}

// ----------------------------------------------------------------------------

// SCG[1/23/00]: not used
/*
static float vermin_jump_direction( userEntity_t *self )
{
	//	targets movement vector
	CVector move_dir = self->enemy->velocity;
	move_dir.Normalize ();
	
	//	get vector to target
	CVector dir = self->enemy->s.origin - self->s.origin;
	dir.Normalize();

	float dp = DotProduct( dir, move_dir );

	return dp;
}
*/

// ----------------------------------------------------------------------------
//
// Name:		vermin_jump_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void vermin_jump_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if( AI_IsReadyToAttack1( self ) )
	{
		CVector jump_ang, vel;

		//	give jumping velocity to vermin here
		self->groundEntity = NULL;

		jump_ang = self->s.angles;
		jump_ang.AngleToVectors(forward, right, up);

		vel = forward * hook->run_speed * 1.5;
		self->velocity = vel;
		self->velocity.z = hook->upward_vel * 0.5f;

        hook->nSpecificAttackMode = SPECIFICATTACKMODE_JUMP;

		if( hook->cur_sequence && !strstr( hook->cur_sequence->animation_name, "atakd" ) )
			AI_StartSequence( self, "atakd" );
	}

	if( AI_IsFacingEnemy( self, self->enemy ) && AI_IsReadyToAttack2( self ) )
	{
		ai_fire_curWeapon( self );
    }

	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );

		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		{
			AI_RemoveCurrentTask( self );
		}
		else
		{
			vermin_set_attack_seq( self );
		}
	}

}

// ----------------------------------------------------------------------------
//
// Name:		vermin_ranged_attack
// Description:
//				FIXME: shoot out a floating, growing
//				poison bubble
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void vermin_ranged_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	if ( strstr( hook->cur_sequence->animation_name, "atak" ) )
	{
		AI_PlayAttackSounds( self );

		if( AI_IsFacingEnemy( self, self->enemy ) && AI_IsReadyToAttack1( self ) )
		{
			CVector viewOffset( 0, 0, 24.0f );
			ai_fire_curWeapon( self );

			hook->fAttackTime = gstate->time + 1.5f;
		}
    }

	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance(self->s.origin, self->enemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		{
			AI_RemoveCurrentTask( self );
		}
		else
		{
			if ( AI_IsEnemyTargetingMe( self, self->enemy ) && rnd() > 0.75f && AI_CanMove( hook ) )
			{
				AI_DoEvasiveAction( self );
			}
			else
			{
				vermin_set_attack_seq(self);
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		vermin_melee_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void vermin_melee_attack( userEntity_t *self )
{
	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) )
	{
		ai_fire_curWeapon( self );
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
//		playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

        float dist = VectorDistance(self->s.origin, self->enemy->s.origin); 
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		{
			AI_RemoveCurrentTask( self );
		}
		else
		{
			vermin_set_attack_seq( self );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		vermin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void vermin_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( gstate->time > hook->fAttackTime )
	{
		float fDist = VectorDistance( self->s.origin, self->enemy->s.origin );
		if ( fDist <= VERMIN_MELEE_DISTANCE )
		{
			if ( hook->nAttackMode != ATTACKMODE_MELEE )
			{
				vermin_set_attack_seq( self );
			}		
			vermin_melee_attack( self );
		}
		else
		if ( fDist <= VERMIN_JUMP_DISTANCE )
		{
			if ( hook->nAttackMode != ATTACKMODE_JUMP )
			{
				vermin_set_attack_seq( self );
			}		
			vermin_jump_attack( self );
		}
		else
		if ( fDist <= VERMIN_RANGED_DISTANCE )
		{
			if ( hook->nAttackMode != ATTACKMODE_RANGED )
			{
				vermin_set_attack_seq( self );
			}		
			vermin_ranged_attack( self );
		}
		else
		{
			AI_RemoveCurrentTask( self );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		vermin_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void vermin_begin_attack( userEntity_t *self )
{
	AI_Dprintf( "begin_attack\n" );

	if ( AI_IsEnemyTargetingMe( self, self->enemy ) && rnd() > 0.75f )
	{
		AI_DoEvasiveAction( self );
	}
	else
	{
		self->velocity.Zero();

		vermin_set_attack_seq( self );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		vermin_start_pain
// Description: pain function for vermin
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void vermin_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,35);
}

// ----------------------------------------------------------------------------
//
// Name:		monster_vermin
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_venomvermin( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_VERMIN );

	self->className			= MONSTERNAME;
	self->netname			= "VenomVermin XP5";

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

	//gstate->ModelIndex("models/e1/me_bubble.dkm");

	gstate->SetSize(self, -16.0, -16.0, -24.0, 16.0, 16.0, 0.0);

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	hook->fnStartAttackFunc = vermin_begin_attack;
	hook->fnAttackFunc		= vermin_attack;
	self->pain				= vermin_start_pain;

	hook->run_speed			= 160;
	hook->walk_speed		= 60;
	hook->attack_speed		= 320;
	hook->attack_dist		= VERMIN_RANGED_DISTANCE;
	hook->upward_vel		= 250;

	self->health			= 60;
	hook->base_health		= 60;
	hook->pain_chance		= 50;
	hook->dflags			|= DFL_JUMPATTACK;
	hook->dflags			|= DFL_RANGEDATTACK;
	hook->jump_chance		= 100;
	

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;

    self->fragtype          |= FRAGTYPE_NOBLOOD;   // don't leave blood upon death
    self->fragtype          |= FRAGTYPE_ROBOTIC;   // apply robotic gib parts
	
	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );

	//////////////////////////////////////////////
	//	set up monster's inventory and weapons
	//////////////////////////////////////////////
	
	self->inventory = gstate->InventoryNew(MEM_MALLOC);

	if ( pWeaponAttributes )
	{
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
										  ITF_TRACE);
		gstate->InventoryAddItem(self, self->inventory, self->curWeapon);

		self->curWeapon = ai_init_weapon( self, 
										  pWeaponAttributes[2].fBaseDamage, 
										  pWeaponAttributes[2].fRandomDamage,
										  pWeaponAttributes[2].fSpreadX,
										  pWeaponAttributes[2].fSpreadZ,
										  pWeaponAttributes[2].fSpeed,
										  pWeaponAttributes[2].fDistance,
										  pWeaponAttributes[2].offset, 
										  "rocket", 
										  vermin_rocket_attack, 
										  ITF_PROJECTILE | ITF_EXPLOSIVE);
		gstate->InventoryAddItem(self, self->inventory, self->curWeapon);
	}

	hook->nAttackType = ATTACK_GROUND_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	AI_DetermineMovingEnvironment( self );

    gstate->LinkEntity( self );
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
void world_ai_venomvermin_register_func()
{
	gstate->RegisterFunc("vermin_begin_attack",vermin_begin_attack);
	gstate->RegisterFunc("vermin_attack",vermin_attack);
	gstate->RegisterFunc("vermin_start_pain",vermin_start_pain);
	gstate->RegisterFunc("AI_ParseEpairs",AI_ParseEpairs);
}

