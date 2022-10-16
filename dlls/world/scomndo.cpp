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
void sealcommando_ranged_attack( userEntity_t *self );

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */
DllExport void monster_sealcommando( userEntity_t *self );

// ----------------------------------------------------------------------------
//
// Name:		AI_SealCommandoTakeCoverAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_SealCommandoTakeCoverAttack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_ZeroVelocity( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

//	float dist = VectorDistance(self->s.origin, self->enemy->s.origin);// SCG[1/23/00]: not used

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) )
	{
		if ( ai_check_projectile_attack(self, self->enemy, (ai_weapon_t *) self->curWeapon, 10) )
		{
			ai_fire_curWeapon( self );
			
		}
		else
		{
			// Ach! We can't hit our enemy!
			AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP ); // sidestep

			hook->attack_finished = gstate->time + 2.0;	//	won't attack for another 2 seconds

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
		AI_IsEnemyDead( self );
		AI_SetAttackFinished(self);
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_SealCommandoTakeCover
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_SealCommandoTakeCover( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	frameData_t *pSequence = hook->cur_sequence;
    if ( !pSequence || !strstr( pSequence->animation_name, "atak" ) || strstr( pSequence->animation_name, "amb" ) )
	{
		AI_ForceSequence( self, "ataka" );
		PATHLIST_KillPath(hook->pPathList);
	}

	if( AI_SealCommandoTakeCoverAttack( self ) == TRUE )
	{
		AI_ForceSequence( self, "amba" );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		sealcommando_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sealcommando_set_attack_seq( userEntity_t *self )
{
    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {
//	    playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	    frameData_t *pSequence; 

		if( rand() % 2 )
			pSequence = FRAMES_GetSequence( self, "ataka" );
		else
			pSequence = FRAMES_GetSequence( self, "atakb" );

	    AI_ForceSequence( self, pSequence );
    }
}

// ----------------------------------------------------------------------------
//
// Name:		sealcommando_ranged_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sealcommando_ranged_attack( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_ZeroVelocity( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) && AI_IsFacingEnemy( self, self->enemy ) )
	{
		if ( ai_check_projectile_attack(self, self->enemy, (ai_weapon_t *) self->curWeapon, 0) )
		{
			ai_fire_curWeapon( self );
		}
		else
		{
			// Ach! We can't hit our enemy!
			AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP ); // sidestep
		}
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		RELIABLE_UNTRACK(self);
		
		float dist = VectorDistance(self->s.origin, self->enemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		{
			if ( AI_ShouldFollow( self ) == TRUE )
			{
				
				AI_RemoveCurrentTask( self );
			}
			else
			{
				// give up attacking the enemy
				AI_RemoveCurrentGoal( self );
			}
		}
		else
		{
			sealcommando_set_attack_seq( self );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		sealcommando_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sealcommando_begin_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	self->velocity.Zero();

	sealcommando_set_attack_seq( self );

	hook->attack_finished = gstate->time + (self->s.frameInfo.endFrame - self->s.frameInfo.endFrame) * 0.1;
}


// ----------------------------------------------------------------------------
//
// Name:		sealcommando_start_pain
// Description: pain function for sealcommando that selects front or back hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sealcommando_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,45);
}

// ----------------------------------------------------------------------------
//
// Name:		sealcommando_start_die
// Description: die function for sealcommando
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sealcommando_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
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
// Name:		monster_sealcommando
// Description:
//	spawns a skeleton during level load
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_sealcommando( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_SEALCOMMANDO );

	self->className			= "monster_sealcommando";
	self->netname			= tongue_monsters[T_MONSTER_SEALCOMMANDO];

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

	hook->fnStartAttackFunc = sealcommando_begin_attack;
	hook->fnAttackFunc		= sealcommando_ranged_attack;
	hook->fnTakeCoverFunc	= AI_SealCommandoTakeCover;

	hook->run_speed			= 250;
	hook->walk_speed		= 50;
	hook->attack_speed		= 250;

	hook->dflags			|= DFL_RANGEDATTACK;

	hook->attack_dist		= 768;

	self->health			= 200;
	hook->base_health		= 200;
	hook->pain_chance		= 10;
	self->mass				= 2.0;

	self->die				= sealcommando_start_die;
	self->pain				= sealcommando_start_pain;
	hook->pain_chance		= 20;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give sealcommando a weapon
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
										  "chaingun", 
										  chaingun_fire, 
										  ITF_BULLET );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}
	hook->nAttackType = ATTACK_GROUND_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	hook->strafe_time = gstate->time;
	hook->strafe_dir  = STRAFE_LEFT;

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
void world_ai_scomndo_register_func()
{
	gstate->RegisterFunc("sealcommando_start_pain",sealcommando_start_pain);
	gstate->RegisterFunc("sealcommando_start_die",sealcommando_start_die);

	gstate->RegisterFunc("sealcommando_begin_attack",sealcommando_begin_attack);
	gstate->RegisterFunc("sealcommando_ranged_attack",sealcommando_ranged_attack);
	gstate->RegisterFunc("AI_SealCommandoTakeCover",AI_SealCommandoTakeCover);
}
