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

/* ***************************** define types ****************************** */

#define ROCKETGANG_BLOWN_AWAY_ANGLECOS 0.707f
#define ROCKETGANG_HEAVY_HIT_DAMAGE 50
#define ROCKETGANG_KNEELING_DIST 500

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */

void rocketgang_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );
void rocketgang_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                           int damage, CVector &destPoint );
void rocketgang_ranged_attack (userEntity_t *self);

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* **************************** Extern Functions *************************** */

void AI_StartDie( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &destPoint );
void AI_StartPain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );

/* ******************************* exports ********************************* */

DllExport void monster_rocketdude (userEntity_t *self);

// ----------------------------------------------------------------------------
//
// Name:		AI_RocketGangTakeCoverAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_RocketGangTakeCoverAttack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_ZeroVelocity( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

//	float dist = VectorDistance(self->s.origin, self->enemy->s.origin);// SCG[1/23/00]: not used

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) )
	{
		if ( ai_check_projectile_attack(self, self->enemy, (ai_weapon_t *) self->curWeapon, ROCKETGANG_KNEELING_DIST) )
		{
			ai_fire_curWeapon( self );

			
		}
		else
		{
			// Ach! We can't hit our enemy!
			AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP ); // sidestep

			//	skip kick-back part of firing sequence
			if ( hook->cur_seq_num == 0 )
			{
				self->s.frame += 12;
			}
			else
			{
				self->s.frame += 21;
			}
		}
	}
	
	if ( AI_IsEndAnimation( self ) )
	{
		AI_SetAttackFinished(self);
		AI_IsEnemyDead( self );
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_RocketGangTakeCover
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_RocketGangTakeCover( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	frameData_t *pSequence = hook->cur_sequence;
    if ( !pSequence || !strstr( pSequence->animation_name, "atak" ) || strstr( pSequence->animation_name, "amb" ) )
    {
		AI_ForceSequence( self, "ataka" );
		PATHLIST_KillPath(hook->pPathList);
	}

	if ( AI_RocketGangTakeCoverAttack( self ) == TRUE )
	{
		AI_ForceSequence( self, "amba" );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		rocketgang_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//				ataka	=> stand fire rocket
//				atakb	=> kneel fire rocket
//
// ----------------------------------------------------------------------------
void rocketgang_set_attack_seq( userEntity_t *self )
{
  	frameData_t *pSequence = NULL;
	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	if ( dist < ROCKETGANG_KNEELING_DIST )
	{
		//	pick the rocket from inventory
		self->curWeapon = gstate->InventoryFindItem (self->inventory, "rocket");

		pSequence = FRAMES_GetSequence( self, "ataka" );
		AI_ForceSequence(self, pSequence);
	}
	else
	{
		//	pick the kneeling rocket from inventory
		self->curWeapon = gstate->InventoryFindItem (self->inventory, "rocket kneeling");

		pSequence = FRAMES_GetSequence( self, "atakb" );
		AI_ForceSequence(self, pSequence);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		rocketgang_ranged_attack
// Description: ranged attack function, called by rocketgang_attack()
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void rocketgang_ranged_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_ZeroVelocity( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if( AI_IsFacingEnemy( self, self->enemy ))
	{
		if( AI_IsReadyToAttack1( self ) )
		{
			ai_fire_curWeapon( self );
		}
		hook->attack_finished = gstate->time + 2.0;	//	won't attack for another 2 seconds
	}
	
	if ( (AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self )) || !hook->cur_sequence )
	{
		//NSS[12/4/99]:Allow the dude another chance to bust open some player scum.
		AI_SetOkToAttackFlag( hook, TRUE );
		
		float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible (self, self->enemy) )
		{

			if ( AI_ShouldFollow( self ) == TRUE )
			{
				AI_RemoveCurrentTask( self );
			}
			else
			{
				AI_RemoveCurrentGoal( self );
			}
		}
		else
		{

			if ( AI_IsEnemyTargetingMe( self, self->enemy ) && rnd() > 0.2f && AI_CanMove( hook ) )
			{
				AI_DoEvasiveAction( self );
			}
			else
			{
				AI_RemoveCurrentTask(self,FALSE);
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		rocketgang_attack
// Description: top-level attack function for rocket gang member
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void rocketgang_attack( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	rocketgang_ranged_attack( self );
	self->lastAIFrame = self->s.frame;
}

// ----------------------------------------------------------------------------
//
// Name:		rocketgang_begin_attack
// Description: setup for attack function
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void rocketgang_begin_attack(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( self->enemy );

	self->lastAIFrame = 0;

	
	if ( AI_IsEnemyTargetingMe( self, self->enemy ) && rnd() > 0.5f && AI_CanMove(hook))
	{
		AI_DoEvasiveAction( self );
	}
	else
	{
//		playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

		rocketgang_set_attack_seq (self);
	}
}


// ----------------------------------------------------------------------------
//
// Name:		rocketgang_start_pain
// Description: pain handler for rocket gang member
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void rocketgang_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,ROCKETGANG_HEAVY_HIT_DAMAGE);
}

// ----------------------------------------------------------------------------
//
// Name:		rocketgang_start_die
// Description: die handler for rocket gang member
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void rocketgang_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                           int damage, CVector &destPoint )
{
	int notdeadyet = (self->deadflag == DEAD_NO);

	// call AI_StartDie, which does generic dying stuff
	AI_StartDie( self, inflictor, attacker, damage, destPoint );

    if ( notdeadyet && self->deadflag == DEAD_DYING )
	{ // pick a different sequence for different deaths
		// see from the dot porduct whether we're blown away or just plain old dead
		if ( AI_GetDirDot( self, inflictor ) > ROCKETGANG_BLOWN_AWAY_ANGLECOS )
			AI_ForceSequence( self, "dieb" ); // jump back (blown away from in front)
		else
			AI_ForceSequence( self, "diea" ); // crumple
	}
}

// ----------------------------------------------------------------------------
//
// Name:		monster_rocketgang
// Description: init function
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_rocketdude( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_ROCKETGANG );

	self->className			= "monster_rocketdude";
	self->netname			= tongue_monsters[T_MONSTER_ROCKETGANG];

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

	hook->fnStartAttackFunc = rocketgang_begin_attack;
	hook->fnAttackFunc		= rocketgang_attack;
	hook->fnTakeCoverFunc	= AI_RocketGangTakeCover;

	hook->run_speed			= 250;
	hook->walk_speed		= 50;
	hook->attack_speed		= 250;

	hook->dflags			|= DFL_RANGEDATTACK;

	hook->attack_dist		= 1000;

	self->health			= 200;
	hook->base_health		= 200;
	hook->pain_chance		= 10;
	self->mass				= 2.0;

	self->pain              = rocketgang_start_pain;
	self->die               = rocketgang_start_die;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give rocketgang a weapon
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
										  "rocket", 
										  rocket_attack, 
										  ITF_PROJECTILE | ITF_EXPLOSIVE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );

		self->curWeapon = ai_init_weapon( self, 
										  pWeaponAttributes[1].fBaseDamage, 
										  pWeaponAttributes[1].fRandomDamage,
										  pWeaponAttributes[1].fSpreadX,
										  pWeaponAttributes[1].fSpreadZ,
										  pWeaponAttributes[1].fSpeed, 
										  pWeaponAttributes[1].fDistance,
										  pWeaponAttributes[1].offset, 
										  "rocket kneeling", 
										  rocket_attack, 
										  ITF_PROJECTILE | ITF_EXPLOSIVE | ITF_SPLASH );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}
	hook->nAttackType = ATTACK_GROUND_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	hook->strafe_time = gstate->time;
	hook->strafe_dir  = STRAFE_LEFT;

	AI_DetermineMovingEnvironment( self );
	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSELADDER|DFL_CANUSEDOOR;

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
void world_ai_rocketgang_register_func()
{
	gstate->RegisterFunc("rocketgang_start_pain",rocketgang_start_pain);
	gstate->RegisterFunc("rocketgang_start_die",rocketgang_start_die);

	gstate->RegisterFunc("rocketgang_begin_attack",rocketgang_begin_attack);
	gstate->RegisterFunc("rocketgang_attack",rocketgang_attack);
	gstate->RegisterFunc("AI_RocketGangTakeCover",AI_RocketGangTakeCover);
}
