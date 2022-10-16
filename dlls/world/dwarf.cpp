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
#define DWARF_CHASE_MIN_ATTACK_MOVEMENT_DIST 40.0f
#define DWARF_CHASE_MIN_DISTANCE 100.0f
#define DWARF_CHASE_MAX_DISTANCE 250.0f
#define DWARF_CHASE_PROBABILITY 0.7f
#define DWARF_AXE_THROW_PROBABILITY 0.6f

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
void dwarf_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &destPoint );
void dwarf_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
void AI_StartPain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );
void AI_StartDie( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &destPoint );

/* ******************************* exports ********************************* */

DllExport	void	monster_dwarf (userEntity_t *self);


///////////////////////////////////////////////////////////////////////////////////
// dwarf specific funcitons
///////////////////////////////////////////////////////////////////////////////////

void	dwarf_attack (userEntity_t *self);

// ----------------------------------------------------------------------------
//
// Name:		AI_DwarfTakeCoverAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_DwarfTakeCoverAttack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if( AI_IsReadyToAttack1( self ) )
	{
		self->curWeapon = gstate->InventoryFindItem(self->inventory, "throwing axe");
//		float dist = VectorDistance(self->s.origin, self->enemy->s.origin);// SCG[1/23/00]: not used

		if( ai_check_projectile_attack(self, self->enemy, (ai_weapon_t *) self->curWeapon, 10) )
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
// Name:		AI_DwarfTakeCover
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_DwarfTakeCover( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	frameData_t *pSequence = hook->cur_sequence;
    if ( !pSequence || !strstr( pSequence->animation_name, "atak" ) || strstr( pSequence->animation_name, "amb" ) )
	{
		AI_ForceSequence( self, "atakd" );
		PATHLIST_KillPath(hook->pPathList);
	}

	if ( AI_DwarfTakeCoverAttack( self ) == TRUE )
	{
		AI_ForceSequence( self, "amba" );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		dwarf_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void dwarf_set_attack_seq( userEntity_t *self )
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
			if ( dist > DWARF_CHASE_MIN_DISTANCE &&
				 dist < DWARF_CHASE_MAX_DISTANCE && // if distance is OK
				 rnd() < DWARF_CHASE_PROBABILITY ) // and chance is correct
			{
				// chase (running chop)
				pSequence = FRAMES_GetSequence( self, "atakc" );
				AI_ForceSequence(self, pSequence);

				hook->nAttackMode = ATTACKMODE_CHASE;

				self->curWeapon = gstate->InventoryFindItem(self->inventory, "punch");
			}
			else
			{
				// ranged attack (axe throw)
				pSequence = FRAMES_GetSequence( self, "atakd" );
				AI_ForceSequence(self, pSequence); 

				hook->nAttackMode = ATTACKMODE_RANGED;

				self->curWeapon = gstate->InventoryFindItem(self->inventory, "throwing axe");
			}
	    }
	    else
	    {
			// melee (axe chop)
			pSequence = FRAMES_GetSequence( self, "ataka" );
			AI_ForceSequence(self, pSequence);

			hook->nAttackMode = ATTACKMODE_MELEE;

			self->curWeapon = gstate->InventoryFindItem(self->inventory, "punch");
	    }
    }
}


// ----------------------------------------------------------------------------
//
// Name:		dwarf_melee_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void dwarf_melee_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	if( hook->nAttackMode == ATTACKMODE_CHASE )
	{
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );

		if ( dist < DWARF_CHASE_MIN_ATTACK_MOVEMENT_DIST )
		{
			AI_ZeroVelocity( self );
		}
		else
		{
			AI_MoveTowardPoint( self, self->enemy->s.origin, FALSE );
		}
	}

	AI_PlayAttackSounds( self );

	if( AI_IsFacingEnemy( self, self->enemy ) && hook->nAttackMode != ATTACKMODE_NORMAL )
	{
		if( AI_IsReadyToAttack1( self ) )
			ai_fire_curWeapon( self );
	}
	
	if (AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible (self, self->enemy) )
		{
			self->curWeapon = gstate->InventoryFindItem(self->inventory, "throwing axe");
			AI_RemoveCurrentTask( self );
			return;
		}
		else
			dwarf_set_attack_seq( self );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		dwarf_ranged_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void dwarf_ranged_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

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
			dwarf_set_attack_seq (self);	// dwarves aren't going to do evasive action
		AI_SetOkToAttackFlag( hook, TRUE);
	}

	self->nextthink			= gstate->time + 1.0;
}


// ----------------------------------------------------------------------------
//
// Name:		dwarf_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void dwarf_attack (userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->nAttackMode == ATTACKMODE_RANGED )
		dwarf_ranged_attack(self);
	else
		dwarf_melee_attack(self);

	self->lastAIFrame = self->s.frame;
}

// ----------------------------------------------------------------------------
//
// Name:		dwarf_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void dwarf_begin_attack(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	self->lastAIFrame = 0;

	AI_ZeroVelocity( self );

	dwarf_set_attack_seq (self);
	
	AI_SetOkToAttackFlag( hook, FALSE);
}

// ----------------------------------------------------------------------------
//
// Name:		dwarf_start_pain
// Description: pain function for dwarf that selects front or back hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void dwarf_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,15);
}

// ----------------------------------------------------------------------------
//
// Name:		dwarf_start_die
// Description: die function for dwarf
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void dwarf_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
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
// Name:		dwarf_check_range
// Description: range check function for dwarf
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int dwarf_check_range(userEntity_t *self, float dist, userEntity_t *enemy)
{
	self->curWeapon = gstate->InventoryFindItem(self->inventory, "punch");

	if ( dist < ((ai_weapon_t *)self->curWeapon)->distance )
		return TRUE;

	if ( rnd() < DWARF_AXE_THROW_PROBABILITY )
	{
		self->curWeapon = gstate->InventoryFindItem(self->inventory, "throwing axe");
		if ( dist < ((ai_weapon_t *)self->curWeapon)->distance )
			return TRUE;
	}

	return ( dist > DWARF_CHASE_MIN_DISTANCE && dist < DWARF_CHASE_MAX_DISTANCE &&
	         rnd() < DWARF_AXE_THROW_PROBABILITY );
}


// ----------------------------------------------------------------------------
//
// Name:		monster_dwarf
// Description:
//				spawns a skeleton during level load
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_dwarf(userEntity_t *self)
{
	playerHook_t *hook	= AI_InitMonster(self, TYPE_DWARF);

	self->className		= "monster_dwarf";
	self->netname		= tongue_monsters[T_MONSTER_DWARF];

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
	//	give dwarf a weapon
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
										  "throwing axe", 
										  throwing_axe_attack, 
										  ITF_PROJECTILE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}

	hook->nAttackType = ATTACK_GROUND_RANGED;  //ATTACK_GROUND_MELEE;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	hook->dflags			|= DFL_RANGEDATTACK;

	hook->fnStartAttackFunc = dwarf_begin_attack;
	hook->fnAttackFunc		= dwarf_attack;
	hook->fnTakeCoverFunc	= AI_DwarfTakeCover;

	hook->fnInAttackRange   = dwarf_check_range;

	hook->pain_chance		= 20;

	self->pain				= dwarf_start_pain;
	self->die				= dwarf_start_die;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

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
void world_ai_dwarf_register_func()
{
	gstate->RegisterFunc("dwarf_start_pain",dwarf_start_pain);
	gstate->RegisterFunc("dwarf_start_die",dwarf_start_die);

	gstate->RegisterFunc("dwarf_begin_attack",dwarf_begin_attack);
	gstate->RegisterFunc("dwarf_attack",dwarf_attack);
	gstate->RegisterFunc("AI_DwarfTakeCover",AI_DwarfTakeCover);
	gstate->RegisterFunc("dwarf_check_range",dwarf_check_range);
}
