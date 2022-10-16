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
#define SKEL_CHASE_MIN_ATTACK_MOVEMENT_DIST 40.0f

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
int skel_check_range( userEntity_t *self, float dist, userEntity_t *enemy );
void skeleton_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );
void skeleton_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &destPoint );
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_skeleton (userEntity_t *self);

// ----------------------------------------------------------------------------
//
// Name:		skel_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//				ataka - atake, atakg	=> stand attack
//				atakf & atakh			=> running attack
//
// ----------------------------------------------------------------------------
void skel_set_attack_seq( userEntity_t *self )
{
    playerHook_t *hook = AI_GetPlayerHook( self );

	hook->nAttackMode = ATTACKMODE_NORMAL;

    if ( AI_IsFacingEnemy(self, self->enemy, 1.0f, 2.0f))
    {
		frameData_t *pSequence;
		int attackRnd = rand() % 6;

		switch( attackRnd )
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
			case 3:
				pSequence = FRAMES_GetSequence( self, "atakd" );
				break;
			case 4:
				pSequence = FRAMES_GetSequence( self, "atake" );
				break;
			case 5:
				pSequence = FRAMES_GetSequence( self, "atakg" );
				break;
			default:
				pSequence = FRAMES_GetSequence( self, "ataka" );
				break;
		}	
	    AI_ForceSequence(self, pSequence);
    }
    else
        AI_FaceTowardPoint( self, self->enemy->s.origin );
}

// ----------------------------------------------------------------------------
//
// Name:		skel_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void skel_begin_attack( userEntity_t *self )
{
	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	playerHook_t *hook = AI_GetPlayerHook( self );

	if( dist < ((ai_weapon_t *)self->curWeapon)->distance )
	{
		// melee attack
//		if ( AI_IsEnemyTargetingMe( self, self->enemy ) && rnd() > 0.75f )
//			AI_DoEvasiveAction( self );
//		else
//		{
			AI_ZeroVelocity( self );
			skel_set_attack_seq (self);
//		}
	}
	else
	{
		// chase attack
		if( strcmp( hook->cur_sequence->animation_name, "atakf" ) )
			AI_StartSequence( self, "atakf", FRAME_LOOP );

		hook->nAttackMode = ATTACKMODE_CHASE;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		skel_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void skel_attack (userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	if( hook->nAttackMode == ATTACKMODE_CHASE )
	{
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );

		if ( dist < SKEL_CHASE_MIN_ATTACK_MOVEMENT_DIST )
		{
			AI_ZeroVelocity( self );
		}
		else
		{
			AI_SetStateRunning(hook);
			if(!AI_MoveTowardPoint( self, self->enemy->s.origin, FALSE ))
			{
				AI_RestartCurrentGoal(self);
				return;
			}
		}
	}

	if(strstr(hook->cur_sequence->animation_name,"atak"))
	{
		AI_PlayAttackSounds( self );

		if( (AI_IsReadyToAttack1( self ) || AI_IsReadyToAttack2( self )) && AI_IsFacingEnemy(self, self->enemy, 8.0f, 10.0f))
			ai_fire_curWeapon( self );
	}		
	if( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance(self->s.origin, self->enemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		{
			AI_RemoveCurrentTask( self );
			return;
		}
		else
		{
			skel_set_attack_seq (self);
		}
	}
}


// ----------------------------------------------------------------------------
//
// Name:		skeleton_start_pain
// Description: pain function for skeleton that selects front or back hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void skeleton_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,35);
}

// ----------------------------------------------------------------------------
//
// Name:		skeleton_start_die
// Description: die function for uzi gang member that may spawn uzi arm
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void skeleton_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                        int damage, CVector &destPoint )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	int notdeadyet = (self->deadflag == DEAD_NO);

	// call AI_StartDie, which does generic dying stuff
	AI_StartDie( self, inflictor, attacker, damage, destPoint );

	if( notdeadyet && self->deadflag == DEAD_DYING )
	{
		float dot = AI_GetDirDot( self, inflictor );
		if( fabs( dot ) < 0.3f )
			AI_ForceSequence( self, "dieb" );
		else
		{
			if( dot > 0 )
				AI_ForceSequence( self, "diea" );
			else
				AI_ForceSequence( self, "died" );
		}
	}
}


// ----------------------------------------------------------------------------
//
// Name:		skel_check_range
// Description: Checks attack range.
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

int skel_check_range( userEntity_t *self, float dist, userEntity_t *enemy )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	return (dist < hook->attack_dist);
}


// ----------------------------------------------------------------------------
//
// Name:		monster_skeleton
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_skeleton(userEntity_t *self)
{
	playerHook_t * hook		= AI_InitMonster(self, TYPE_SKELETON);

	self->className			= "monster_skeleton";
	self->netname			= tongue_monsters[T_MONSTER_SKELETON];

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

    self->fragtype          |= FRAGTYPE_NOBLOOD | FRAGTYPE_BONE;   // don't leave blood upon death and push out some boners
	hook->run_speed			= 250;
	hook->walk_speed		= 50;
	hook->attack_speed		= 250;

	hook->attack_dist		= 200;

	self->health			= 200;
	hook->base_health		= 200;
	hook->pain_chance		= 20;
	
	hook->fnStartAttackFunc = skel_begin_attack;
	hook->fnAttackFunc		= skel_attack;
	hook->fnInAttackRange   = skel_check_range;

	self->pain              = skeleton_start_pain;
	self->die               = skeleton_start_die;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	//////////////////////////////////////////////
	//	give skel its melee weapon
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
										  "swing", 
										  melee_swing, 
										  ITF_TRACE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}
	hook->nAttackType = ATTACK_GROUND_MELEE;
	hook->nAttackMode = ATTACKMODE_NORMAL;

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
void world_ai_skeleton_register_func()
{
	gstate->RegisterFunc("skeleton_start_pain",skeleton_start_pain);
	gstate->RegisterFunc("skeleton_start_die",skeleton_start_die);

	gstate->RegisterFunc("skel_begin_attack",skel_begin_attack);
	gstate->RegisterFunc("skel_attack",skel_attack);
	gstate->RegisterFunc("skel_check_range",skel_check_range);
}
