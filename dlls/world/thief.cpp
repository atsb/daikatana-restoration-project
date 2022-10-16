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
//#define THIEF_CHASE_MIN_ATTACK_MOVEMENT_DIST 40.0f// SCG[1/23/00]: not used
#define THIEF_CHASE_MIN_DISTANCE 100.0f
#define THIEF_CHASE_MAX_DISTANCE 250.0f
//#define THIEF_CHASE_PROBABILITY 0.7f// SCG[1/23/00]: not used
#define THIEF_AXE_THROW_PROBABILITY 0.6f

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
void thief_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &destPoint );
void thief_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
void AI_StartPain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );
void AI_StartDie( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &destPoint );

/* ******************************* exports ********************************* */

DllExport	void	monster_thief (userEntity_t *self);


///////////////////////////////////////////////////////////////////////////////////
// thief specific funcitons
///////////////////////////////////////////////////////////////////////////////////

void	thief_attack (userEntity_t *self);

// ----------------------------------------------------------------------------
//
// Name:		AI_ThiefTakeCoverAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_ThiefTakeCoverAttack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

//	float dist = VectorDistance(self->s.origin, self->enemy->s.origin);// SCG[1/23/00]: not used

	AI_PlayAttackSounds( self );

	if( AI_IsReadyToAttack1( self ) )
	{
		if ( ai_check_projectile_attack(self, self->enemy, (ai_weapon_t *) self->curWeapon, 10) )
		{
			self->curWeapon = gstate->InventoryFindItem(self->inventory, "throwing knife");
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
// Name:		AI_ThiefTakeCover
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ThiefTakeCover( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	frameData_t *pSequence = hook->cur_sequence;
    if ( !pSequence || !strstr( pSequence->animation_name, "atak" ) || strstr( pSequence->animation_name, "amb" ) )
	{
		AI_ForceSequence( self, "ataka" );
		PATHLIST_KillPath(hook->pPathList);
	}

	if( AI_ThiefTakeCoverAttack( self ) == TRUE )
	{
		AI_ForceSequence( self, "amba" );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		thief_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void thief_set_attack_seq( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	hook->nAttackMode = ATTACKMODE_NORMAL;

    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {
	    playerHook_t *hook = AI_GetPlayerHook( self );

	    frameData_t *pSequence = NULL;
	    float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	    if ( dist > 80 )
	    {
			// ranged attack (knife throw)
			pSequence = FRAMES_GetSequence( self, "ataka" );
			AI_ForceSequence(self, pSequence); 

			hook->nAttackMode = ATTACKMODE_RANGED;

			self->curWeapon = gstate->InventoryFindItem(self->inventory, "throwing knife");
	    }
	    else
	    {
			// melee (knife chop)
			pSequence = FRAMES_GetSequence( self, "atakb" );
			AI_ForceSequence(self, pSequence);

			hook->nAttackMode = ATTACKMODE_MELEE;

			self->curWeapon = gstate->InventoryFindItem(self->inventory, "punch");
	    }
    }
}


// ----------------------------------------------------------------------------
//
// Name:		thief_melee_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void thief_melee_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if( AI_IsFacingEnemy( self, self->enemy ) && hook->nAttackMode != ATTACKMODE_NORMAL )
	{
		if( AI_IsReadyToAttack1( self ) )
			ai_fire_curWeapon( self );
	}
	
	if ( (hook->nAttackMode == ATTACKMODE_NORMAL) || AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible (self, self->enemy) )
		{
			self->curWeapon = gstate->InventoryFindItem(self->inventory, "throwing knife");
			AI_RemoveCurrentTask( self );
			return;
		}
		else
			thief_set_attack_seq( self );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		thief_ranged_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void thief_ranged_attack( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if( AI_IsFacingEnemy( self, self->enemy ) )
	{
		if( AI_IsReadyToAttack1( self ) )
		{
			if( ai_check_projectile_attack(self, self->enemy, (ai_weapon_t *) self->curWeapon, 0) )
			{
				ai_fire_curWeapon( self );
			}
			else
			{
				// Ach! We can't hit our enemy!
				AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP ); // sidestep
			}
		}
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance(self->s.origin, self->enemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible (self, self->enemy) )
		{
			if ( AI_ShouldFollow( self ) == TRUE )
				AI_RemoveCurrentTask( self );
			else
				AI_RemoveCurrentGoal( self );	// give up attacking the enemy
		}
		else
			thief_set_attack_seq (self);	// dwarves aren't going to do evasive action
	}

	self->nextthink			= gstate->time + 1.0;
}


// ----------------------------------------------------------------------------
//
// Name:		thief_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void thief_attack (userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->nAttackMode == ATTACKMODE_RANGED )
		thief_ranged_attack(self);
	else
		thief_melee_attack(self);

	self->lastAIFrame = self->s.frame;
}

// ----------------------------------------------------------------------------
//
// Name:		thief_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void thief_begin_attack(userEntity_t *self)
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	self->lastAIFrame = 0;

	AI_ZeroVelocity( self );

	thief_set_attack_seq (self);
}

// ----------------------------------------------------------------------------
//
// Name:		thief_start_pain
// Description: pain function for thief that selects front or back hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void thief_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,35);
}

// ----------------------------------------------------------------------------
//
// Name:		thief_start_die
// Description: die function for thief
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void thief_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                        int damage, CVector &destPoint )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	int notdeadyet = (self->deadflag == DEAD_NO);

	// call AI_StartDie, which does generic dying stuff
	AI_StartDie( self, inflictor, attacker, damage, destPoint );

	if( notdeadyet && self->deadflag == DEAD_DYING )
	{
		if( rand() % 2 )
			AI_ForceSequence( self, "diea" );
		else
			AI_ForceSequence( self, "dieb" );
	}
}


// ----------------------------------------------------------------------------
//
// Name:		thief_check_range
// Description: range check function for thief
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int thief_check_range(userEntity_t *self, float dist, userEntity_t *enemy)
{
	self->curWeapon = gstate->InventoryFindItem(self->inventory, "punch");

	if ( dist < ((ai_weapon_t *)self->curWeapon)->distance )
		return TRUE;

	if ( rnd() < THIEF_AXE_THROW_PROBABILITY )
	{
		self->curWeapon = gstate->InventoryFindItem(self->inventory, "throwing knife");
		if ( dist < ((ai_weapon_t *)self->curWeapon)->distance )
			return TRUE;
	}

	return ( dist > THIEF_CHASE_MIN_DISTANCE && dist < THIEF_CHASE_MAX_DISTANCE &&
	         rnd() < THIEF_AXE_THROW_PROBABILITY );
}

// ----------------------------------------------------------------------------
//
// Name:		monster_thief
// Description:
//				spawns a skeleton during level load
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_thief(userEntity_t *self)
{
	playerHook_t *hook	= AI_InitMonster(self, TYPE_THIEF);

	self->className		= "monster_thief";
	self->netname		= tongue_monsters[T_MONSTER_THIEF];

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

	///////////////////////////////////////////////////////////////////////////
	//	give thief a weapon
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
										  ITF_TRACE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );

		self->curWeapon = ai_init_weapon( self, 
										  pWeaponAttributes[1].fBaseDamage, 
										  pWeaponAttributes[1].fRandomDamage,
										  pWeaponAttributes[1].fSpreadX,
										  pWeaponAttributes[1].fSpreadZ,
										  pWeaponAttributes[1].fSpeed,
										  pWeaponAttributes[1].fDistance,
										  pWeaponAttributes[1].offset, 
										  "throwing knife", 
										  throwing_knife_attack, 
										  ITF_PROJECTILE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}

	hook->nAttackType = ATTACK_GROUND_RANGED;  //ATTACK_GROUND_MELEE;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	hook->dflags			|= DFL_RANGEDATTACK;

	hook->fnStartAttackFunc = thief_begin_attack;
	hook->fnAttackFunc		= thief_attack;
	hook->fnTakeCoverFunc	= AI_ThiefTakeCover;

	hook->fnInAttackRange   = thief_check_range;

	hook->pain_chance		= 30;

	self->pain				= thief_start_pain;
	self->die				= thief_start_die;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	AI_DetermineMovingEnvironment( self );
	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;

	gstate->LinkEntity(self);
	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_thief_register_func()
{
	gstate->RegisterFunc("thief_begin_attack",thief_begin_attack);
	gstate->RegisterFunc("thief_attack",thief_attack);
	gstate->RegisterFunc("AI_ThiefTakeCover",AI_ThiefTakeCover);
	gstate->RegisterFunc("thief_check_range",thief_check_range);
	gstate->RegisterFunc("thief_start_pain",thief_start_pain);
	gstate->RegisterFunc("thief_start_die",thief_start_die);
}