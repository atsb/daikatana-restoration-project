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

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
void centurion_set_attack_seq( userEntity_t *self );
void centurion_melee_attack (userEntity_t *self);
void centurion_ranged_attack (userEntity_t *self);

/* **************************** Global Variables *************************** */

/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */
DllExport void monster_centurion( userEntity_t *self );


// ----------------------------------------------------------------------------
//
// Name:		centurion_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void centurion_begin_attack( userEntity_t *self )
{
	if ( AI_IsEnemyTargetingMe( self, self->enemy ) && rnd() > 0.75f )
	{
		AI_DoEvasiveAction( self );
	}
	else
	{
		centurion_set_attack_seq( self );
	}
}


// ----------------------------------------------------------------------------
//
// Name:		centurion_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void centurion_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->nAttackMode == ATTACKMODE_RANGED )
	{
		centurion_ranged_attack(self);
	}
	else
	{
		centurion_melee_attack(self);
	}
}


// ----------------------------------------------------------------------------
//
// Name:		centurion_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//				ataka	=> melee attack
//				atakb	=> ranged attack, throws a spear
//
// ----------------------------------------------------------------------------
void centurion_set_attack_seq( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	hook->nAttackMode = ATTACKMODE_NORMAL;

	if ( gstate->time > hook->fAttackTime )
	{
		if ( AI_IsFacingEnemy( self, self->enemy ) )
        {
            frameData_t *pSequence = NULL;
		    float dist = VectorDistance (self->s.origin, self->enemy->s.origin);

		    self->curWeapon = gstate->InventoryFindItem(self->inventory, "punch");

		    if ( dist < ((ai_weapon_t *) self->curWeapon)->distance )
		    {
			    pSequence = FRAMES_GetSequence( self, "ataka" );
			    AI_ForceSequence(self, pSequence);
				hook->nAttackMode = ATTACKMODE_MELEE;
		    }
		    else
		    {
			    pSequence = FRAMES_GetSequence( self, "atakb" );
			    AI_ForceSequence(self, pSequence);

			    //	pick the spear from inventory
			    self->curWeapon = gstate->InventoryFindItem(self->inventory, "spear");
				hook->nAttackMode = ATTACKMODE_RANGED;
		    }
	    }
    }
}

// ----------------------------------------------------------------------------
//
// Name:		centurion_ranged_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void centurion_ranged_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	if ( hook->nAttackMode == ATTACKMODE_RANGED )
    {
	    AI_PlayAttackSounds( self );

	    if ( AI_IsReadyToAttack1( self ) && AI_IsFacingEnemy( self, self->enemy ) )
	    {
		    if ( ai_check_projectile_attack(self, self->enemy, (ai_weapon_t *) self->curWeapon, 0) )
		    {
			    ai_fire_curWeapon( self );
			    hook->fAttackTime = gstate->time + 0.75f;
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

			hook->nAttackMode = ATTACKMODE_NORMAL;

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
			    if ( AI_IsEnemyTargetingMe( self, self->enemy ) && rnd() > 0.75f && AI_CanMove( hook ) )
			    {
				    AI_DoEvasiveAction( self );
			    }
			    else
			    {
				    centurion_set_attack_seq( self );
			    }
		    }
	    }
    }
	else
	{
		centurion_set_attack_seq( self );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		centurion_melee_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void centurion_melee_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	if ( hook->nAttackMode == ATTACKMODE_MELEE )
    {
	    AI_PlayAttackSounds( self );

	    if ( AI_IsReadyToAttack1( self ) && AI_IsFacingEnemy( self, self->enemy ) )
	    {
		    ai_fire_curWeapon( self );
	    }
	    
	    if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	    {
		    float dist = VectorDistance( self->s.origin, self->enemy->s.origin );

			hook->nAttackMode = ATTACKMODE_NORMAL;

		    if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible (self, self->enemy) )
		    {
			    AI_RemoveCurrentTask( self );

			    return;
		    }
		    else
		    {
			    if ( rnd() > 0.75f )
			    {
				    AI_DoEvasiveAction( self );
			    }
			    else
			    {
				    centurion_set_attack_seq( self );
			    }
		    }
	    }
    }
	else
	{
		centurion_set_attack_seq( self );
	}

}


// ----------------------------------------------------------------------------
//
// Name:		monster_centurion
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_centurion( userEntity_t *self )
{
	playerHook_t *hook	= AI_InitMonster( self, TYPE_CENTURION );

	self->className		= "monster_centurion";
	self->netname		= tongue_monsters[T_MONSTER_CENTURION];

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

	hook->upward_vel		= 270.0;
	hook->run_speed			= 250;
	hook->walk_speed		= 150;
	hook->attack_speed		= 250;
	hook->max_jump_dist		= ai_max_jump_dist( hook->run_speed, hook->upward_vel );

	hook->dflags			|= DFL_RANGEDATTACK;
	hook->attack_dist		= 1000;

	self->health			= 200;
	hook->base_health		= 200;
	hook->pain_chance		= 30;
	

	hook->fnStartAttackFunc = centurion_begin_attack;
	hook->fnAttackFunc		= centurion_attack;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give centurion a weapon
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
										  "spear", 
										  spear_attack, 
										  ITF_PROJECTILE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );

		self->curWeapon = ai_init_weapon( self, 
										  pWeaponAttributes[1].fBaseDamage, 
										  pWeaponAttributes[1].fRandomDamage,
										  pWeaponAttributes[1].fSpreadX,
										  pWeaponAttributes[1].fSpreadZ,
										  pWeaponAttributes[1].fSpeed,
										  pWeaponAttributes[1].fDistance,
										  pWeaponAttributes[1].offset, 
										  "punch", 
										  melee_punch, 
										  ITF_TRACE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}

	hook->nAttackType = ATTACK_GROUND_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	AI_DetermineMovingEnvironment( self );
	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;

    gstate->LinkEntity( self );
	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_centurion_register_func()
{
	gstate->RegisterFunc("centurion_begin_attack",centurion_begin_attack);
	gstate->RegisterFunc("centurion_attack",centurion_attack);
}
