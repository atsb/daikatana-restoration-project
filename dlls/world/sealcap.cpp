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
#define SEALCAP_CHASE_MIN_DISTANCE 300.0f
#define SEALCAP_CHASE_MAX_DISTANCE 300.0f
#define SEALCAP_CHASE_PROBABILITY 1.0f

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
void sealcaptain_ranged_attack( userEntity_t *self );

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */
DllExport void monster_sealcaptain( userEntity_t *self );

// ----------------------------------------------------------------------------
//
// Name:		AI_SealCaptainTakeCoverAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_SealCaptainTakeCoverAttack( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

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
// Name:		AI_SealCaptainTakeCover
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_SealCaptainTakeCover( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	frameData_t *pSequence = hook->cur_sequence;
    if ( !pSequence || !strstr( pSequence->animation_name, "atak" ) || strstr( pSequence->animation_name, "amb" ) )
	{
		AI_ForceSequence( self, "ataka" );
		PATHLIST_KillPath(hook->pPathList);
	}

	if( AI_SealCaptainTakeCoverAttack( self ) == TRUE )
	{
		AI_ForceSequence( self, "amba" );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		sealcaptain_chase_attack_seq_active
// Description: Determines if the chase attack sequence is currently running.
// Input:
// Output:
// Note:
// ----------------------------------------------------------------------------
int sealcaptain_chase_attack_seq_active( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	frameData_t *pSequence = FRAMES_GetSequence( self, "atakb" );

	return ( hook->cur_sequence && hook->cur_sequence == pSequence );
}

// ----------------------------------------------------------------------------
//
// Name:		sealcaptain_do_chase_attack_seq
// Description: Start/transition self to use chase (running) attack sequence.
// Input:
// Output:
// Note:
// ----------------------------------------------------------------------------
void sealcaptain_do_chase_attack_seq( userEntity_t *self )
{
	frameData_t *pSequence = FRAMES_GetSequence( self, "atakb" );

	AI_ForceSequence( self, pSequence ); // start the current sequence over again
}

// ----------------------------------------------------------------------------
//
// Name:		sealcaptain_do_run_seq
// Description: Start/transition self to use chase (running) attack sequence.
// Input:
// Output:
// Note:
// ----------------------------------------------------------------------------
void sealcaptain_do_run_seq( userEntity_t *self )
{
	frameData_t *pSequence = FRAMES_GetSequence( self, "runa" );

	AI_ForceSequence( self, pSequence ); // start the current sequence over again
}

// ----------------------------------------------------------------------------
//
// Name:		sealcaptain_chasing_attack
// Description: Attack enemy while chasing him/her.
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int sealcaptain_chasing_attack( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	float dist;

	if ( AI_IsEnemyDead( self ) )
	{
		return 0;
	}

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	// see if we can switch to attacking
	dist = VectorDistance(self->s.origin, self->enemy->s.origin);

	// check whether to end attack
	if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible( self, self->enemy ) )
	{
		// we've lost him

		if ( AI_ShouldFollow( self ) == TRUE )
		{
			// give up current attack
			AI_RemoveCurrentTask( self );
		}
		else
		{
			// give up attacking the enemy completely
			AI_RemoveCurrentGoal( self );
		}
	}
	else
	{
		int atend = AI_IsEndAnimation( self );

		// eligible to attack while chasing
		if ( atend && dist < SEALCAP_CHASE_MIN_DISTANCE )
		{
			// not close enough to attack while chasing, so stop and attack normally
			AI_AddNewTaskAtFront( self, TASKTYPE_ATTACK );
		}
		else
		{
			if ( sealcaptain_chase_attack_seq_active( self ) )
			{
				AI_PlayAttackSounds( self );

				// fire if ready to attack
				if ( AI_IsFacingEnemy( self, self->enemy ) && 
                     (AI_IsReadyToAttack1( self ) || AI_IsReadyToAttack2( self )) )
				{
					ai_fire_curWeapon( self );
				}
			}
			else
			{
				sealcaptain_do_chase_attack_seq( self );
			}

			// check for end of animation
			if ( atend )
			{
				sealcaptain_do_chase_attack_seq( self );
			}
		}
	}

	return 0; // never within strafing distance
}

// ----------------------------------------------------------------------------
//
// Name:		sealcaptain_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sealcaptain_set_attack_seq( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );
//	frameData_t *pSequence = NULL;// SCG[1/23/00]: not used

    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {
//	    playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	    AI_ForceSequence( self, "ataka" );
    }
    else
    {
        AI_FaceTowardPoint( self, self->enemy->s.origin );
    }

}

// ----------------------------------------------------------------------------
//
// Name:		sealcaptain_ranged_attack
// Description:
// Input:
// Output:
// Note:`
//
// ----------------------------------------------------------------------------
void sealcaptain_ranged_attack( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_ZeroVelocity( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) && AI_IsFacingEnemy( self, self->enemy ) )
	{
		//ai_fire_playerWeapon(self, self->s.origin + self->view_ofs, self->enemy->s.origin + self->enemy->view_ofs, 0, 0, 5);
 		ai_fire_curWeapon( self );
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
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

			return;
		}
		else
		{
			sealcaptain_set_attack_seq( self );
		}
	}

}

// ----------------------------------------------------------------------------
//
// Name:		sealcaptain_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sealcaptain_begin_attack( userEntity_t *self )
{
	sealcaptain_set_attack_seq( self );
}


// ----------------------------------------------------------------------------
// NSS[1/22/00]:
// Name:		sealcaptain_start_pain
// Description: pain function for sealcaptain that selects front or back hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sealcaptain_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,35);
}

// ----------------------------------------------------------------------------
//
// Name:		sealcaptain_start_die
// Description: die function for sealcaptain
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sealcaptain_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
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
// Name:		monster_sealcaptain
// Description:
//	spawns a skeleton during level load
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_sealcaptain( userEntity_t *self )
{
//	weaponInfo_t	*winfo;
	playerHook_t *hook		= AI_InitMonster( self, TYPE_SEALCAPTAIN );

	self->className			= "monster_sealcaptain";
	self->netname			= tongue_monsters[T_MONSTER_SEALCAPTAIN];

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

	hook->fnStartAttackFunc = sealcaptain_begin_attack;
	hook->fnAttackFunc		= sealcaptain_ranged_attack;
	hook->fnTakeCoverFunc	= AI_SealCaptainTakeCover;
	hook->fnChasingAttack	= sealcaptain_chasing_attack;

	hook->run_speed			= 250;
	hook->walk_speed		= 50;
	hook->attack_speed		= 250;

	hook->dflags			|= DFL_RANGEDATTACK;

	hook->attack_dist		= 1000;

	self->health			= 200;
	hook->base_health		= 200;
	hook->pain_chance		= 10;
	self->mass				= 2.0;

	self->die				= sealcaptain_start_die;
	self->pain				= sealcaptain_start_pain;
	hook->pain_chance		= 20;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give sealcaptain a weapon
	///////////////////////////////////////////////////////////////////////////

	self->inventory = gstate->InventoryNew (MEM_MALLOC);

//	winfo = com->FindRegisteredWeapon ("weapon_ripgun");
//	if (winfo)
//	{
//		winfo->give_func (self, winfo->initialAmmo);
//		self->curWeapon = (userInventory_t *) gstate->InventoryFindItem (self->inventory, "weapon_ripgun");
//		gstate->InventoryAddItem (self, self->inventory, self->curWeapon);
//	}

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
										  "shotgun", 
										  shotgun_fire, 
										  ITF_BULLET );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}
	hook->nAttackType = ATTACK_GROUND_CHASING;
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
void world_ai_sealcap_register_func()
{
	gstate->RegisterFunc("sealcaptain_start_pain",sealcaptain_start_pain);
	gstate->RegisterFunc("sealcaptain_start_die",sealcaptain_start_die);

	gstate->RegisterFunc("sealcaptain_begin_attack",sealcaptain_begin_attack);
	gstate->RegisterFunc("sealcaptain_ranged_attack",sealcaptain_ranged_attack);
	gstate->RegisterFunc("AI_SealCaptainTakeCover",AI_SealCaptainTakeCover);
	gstate->RegisterFunc("sealcaptain_chasing_attack",sealcaptain_chasing_attack);
}

