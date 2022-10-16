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

#define FLETCH_ATTACK_EVASIVE_CHANCE 0.3f
#define FLETCH_FAR_ATTACK_MIN_DISTANCE 400

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
void fletcher_ranged_attack( userEntity_t *self );
void fletcher_start_attack_seq( userEntity_t *self );

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */
DllExport void monster_fletcher( userEntity_t *self );

// ----------------------------------------------------------------------------
//
// Name:		AI_FletcherTakeCoverAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_FletcherTakeCoverAttack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

//	float dist = VectorDistance(self->s.origin, self->enemy->s.origin);// SCG[1/23/00]: not used

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) )
	{
		if ( ai_check_projectile_attack(self, self->enemy, (ai_weapon_t *) self->curWeapon, 10) )
		{
			ai_fire_curWeapon( self );
			hook->attack_finished = gstate->time + 2.0f;	//	won't attack for another 2 seconds
		}
		else
		{
			// Ach! We can't hit our enemy!
			AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP ); // sidestep
		}
	}

	if ( AI_IsEndAnimation( self ) )
	{
		AI_IsEnemyDead( self );
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_FletcherTakeCover
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_FletcherTakeCover( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	frameData_t *pSequence = hook->cur_sequence;
    if ( !pSequence || !strstr( pSequence->animation_name, "atak" ) || strstr( pSequence->animation_name, "amb" ) )
	{
		AI_ForceSequence( self, "ataka" );
		PATHLIST_KillPath(hook->pPathList);
	}

	if( AI_FletcherTakeCoverAttack( self ) == TRUE )
	{
		AI_ForceSequence( self, "amba" );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		fletcher_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void fletcher_set_attack_seq( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_FaceTowardPoint( self, self->enemy->s.origin );
	AI_ForceSequence( self, "ataka" );
}

// ----------------------------------------------------------------------------
//
// Name:		fletcher_set_far_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void fletcher_set_far_attack_seq( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_FaceTowardPoint( self, self->enemy->s.origin );
	AI_ForceSequence( self, "atakb" );
}

// ----------------------------------------------------------------------------
//
// Name:		fletcher_start_attack_seq
// Description: starts the fletcher attacking
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void fletcher_start_attack_seq( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( AI_IsEnemyTargetingMe( self, self->enemy ) && AI_CanMove( hook ) &&
		 rnd() <= FLETCH_ATTACK_EVASIVE_CHANCE )
	{
		AI_DoEvasiveAction( self );
	}
	else
	{
		if ( VectorDistance(self->s.origin, self->enemy->s.origin) < FLETCH_FAR_ATTACK_MIN_DISTANCE )
		{
			fletcher_set_attack_seq( self );
		}
		else
		{
			fletcher_set_far_attack_seq( self );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		fletcher_ranged_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void fletcher_ranged_attack( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	AI_ZeroVelocity(self);

	if( AI_IsFacingEnemy( self, self->enemy ) )
	{
		if( ai_check_projectile_attack(self, self->enemy, (ai_weapon_t *) self->curWeapon, 0) )
		{
			if( AI_IsReadyToAttack1( self ) )
				ai_fire_curWeapon( self );
		}
		else
		{
			// Ach! We can't hit our enemy!
			AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP ); // sidestep
		}
	}
	
	if( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance(self->s.origin, self->enemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible (self, self->enemy) )
		{
			if ( AI_ShouldFollow( self ) == TRUE )
				AI_RemoveCurrentTask( self );
			else
				// give up attacking the enemy
				AI_RemoveCurrentGoal( self );
		}
		else
			fletcher_start_attack_seq( self );
	}

	self->lastAIFrame = self->s.frame;

}

// ----------------------------------------------------------------------------
//
// Name:		fletcher_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void fletcher_begin_attack( userEntity_t *self )
{
	self->lastAIFrame = 0;
	fletcher_start_attack_seq( self );
	AI_ZeroVelocity(self);

}


// ----------------------------------------------------------------------------
//
// Name:		fletcher_start_pain
// Description: pain function for fletcher that selects front or back hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void fletcher_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,25);
}

// ----------------------------------------------------------------------------
//
// Name:		fletcher_start_die
// Description: die function for uzi gang member that may spawn uzi arm
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void fletcher_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                        int damage, CVector &destPoint )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	int notdeadyet = (self->deadflag == DEAD_NO);

	// call AI_StartDie, which does generic dying stuff
	AI_StartDie( self, inflictor, attacker, damage, destPoint );

	if( notdeadyet && self->deadflag == DEAD_DYING )
	{
		float dot = AI_GetDirDot( self, inflictor );
		if( dot > 0 )
			AI_ForceSequence( self, "diea" );
		else
			AI_ForceSequence( self, "dieb" );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		monster_fletcher
// Description:
//	spawns a fletcher during level load
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_fletcher( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_FLETCHER );

	self->className			= "monster_fletcher";
	self->netname			= tongue_monsters[T_MONSTER_FLETCHER]; // gay!

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

	hook->fnStartAttackFunc = fletcher_begin_attack;
	hook->fnAttackFunc		= fletcher_ranged_attack;
	hook->fnTakeCoverFunc	= AI_FletcherTakeCover;

	self->pain              = fletcher_start_pain;
	self->die               = fletcher_start_die;

	hook->run_speed			= 250;
	hook->walk_speed		= 50;
	hook->attack_speed		= 250;

	hook->dflags			|= DFL_RANGEDATTACK;
	hook->attack_dist		= 1000;

	self->health			= 200;
	hook->base_health		= 200;
	hook->pain_chance		= 20;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give fletcher a weapon
	///////////////////////////////////////////////////////////////////////////
	self->inventory = gstate->InventoryNew(MEM_MALLOC);

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
										  "magic arrow", 
										  magic_arrow_attack, 
										  ITF_PROJECTILE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}
	hook->nAttackMode = ATTACKMODE_NORMAL;
	hook->nAttackType = ATTACK_GROUND_RANGED;

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
void world_ai_fletcher_register_func()
{
	gstate->RegisterFunc("fletcher_start_pain",fletcher_start_pain);
	gstate->RegisterFunc("fletcher_start_die",fletcher_start_die);

	gstate->RegisterFunc("fletcher_begin_attack",fletcher_begin_attack);
	gstate->RegisterFunc("fletcher_ranged_attack",fletcher_ranged_attack);
	gstate->RegisterFunc("AI_FletcherTakeCover",AI_FletcherTakeCover);
}
