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

void pipe_rat_melee_attack( userEntity_t *self );
void pipe_rat_jump_attack (userEntity_t *self);

/* **************************** Global Variables *************************** */

/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */
DllExport void monster_piperat( userEntity_t *self );


// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void pipe_rat_set_attack_seq( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if( self->waterlevel )
	{ 
		hook->nAttackType = ATTACK_WATER_MELEE;
		hook->nAttackMode = ATTACKMODE_NORMAL;
		AI_DetermineMovingEnvironment( self );
	}
	else
	{
		hook->nAttackType = ATTACK_GROUND_MELEE;
		hook->nAttackMode = ATTACKMODE_NORMAL;
		AI_DetermineMovingEnvironment( self );
	}

    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {
	    frameData_t *pSequence = NULL;
	    float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	    if ( AI_IsWithinAttackDistance( self, dist ) )
	    {
		    //int i = (int)(rand() % hook->sequence_max[SEQUENCE_ATAK]);
		    pSequence = FRAMES_GetSequence( self, "ataka" );
		    AI_ForceSequence( self, pSequence );

		    pipe_rat_melee_attack (self);
	    }
	    else
	    {
		    pSequence = FRAMES_GetSequence( self, "jumpa" );
		    AI_ForceSequence(self, pSequence);

		    pipe_rat_jump_attack (self);
	    }
    }
}

// ----------------------------------------------------------------------------
//
// Name:		pipe_rat_DoEvasiveAction
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void pipe_rat_DoEvasiveAction( userEntity_t *self )
{
	if ( rnd() > 0.5f )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP );
	}
	else
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_STRAFE );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		pipe_rat_jump_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void pipe_rat_jump_attack (userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	AI_FaceTowardPoint( self, self->enemy->s.origin );

	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	if ( self->s.frameInfo.frameState & FRSTATE_PLAYSOUND1 )
	{
		CVector jump_ang, vel;

		//	jump on this frame
		//NSS[11/14/99]:
		//self->groundEntity = NULL;

		jump_ang = self->s.angles;
		jump_ang.AngleToVectors(forward, right, up);

		vel = forward * hook->run_speed * 1.5;
		self->velocity = vel;
		self->velocity.z = hook->upward_vel;

		self->s.frameInfo.frameState -= FRSTATE_PLAYSOUND1;

		if ( hook->sound1 && !AI_IsSoundDisabled() )
		{
			gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex(hook->sound1), 0.85f, hook->fMinAttenuation, hook->fMaxAttenuation);
		}
	}
	
	if (self->s.frameInfo.frameState & FRSTATE_PLAYSOUND2)
	{
		//	attack on this frame
		self->s.frameInfo.frameState -= FRSTATE_PLAYSOUND2;
				
		ai_fire_curWeapon( self );
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		{
			AI_RemoveCurrentTask( self );

			return;
		}
		else
		{
			if ( rnd() > 0.5f )
			{
				pipe_rat_DoEvasiveAction( self );
			}
			else
			{
				pipe_rat_set_attack_seq (self);
			}
		}
	}
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
void pipe_rat_melee_attack( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_ZeroVelocity( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) && AI_IsFacingEnemy( self, self->enemy ) )
	{
		ai_fire_curWeapon( self );
	}
	
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
			if ( rnd() > 0.5f )
			{
				pipe_rat_DoEvasiveAction( self );
			}
			else
			{
				pipe_rat_set_attack_seq( self );
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		pipe_rat_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void pipe_rat_attack(userEntity_t *self)
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	if ( AI_IsWithinAttackDistance( self, dist ) )
	{
		pipe_rat_melee_attack (self);
	}
	else
	{
		pipe_rat_jump_attack (self);
	}
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
void pipe_rat_begin_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	hook->attack_finished = gstate->time + 1.0;

	pipe_rat_set_attack_seq( self );
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
void monster_piperat( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_PIPERAT );

	self->className			= "monster_piperat";
	self->netname			= tongue_monsters[T_MONSTER_PIPERAT];

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

	gstate->SetSize (self, -8.0, -8.0, -24.0, 8.0, 8.0, 8.0);

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	////////////////	///////////////////////////////////////////

	hook->upward_vel		= 270.0;
	hook->run_speed			= 250;
	hook->walk_speed		= 150;
	hook->attack_speed		= 250;
	hook->max_jump_dist		= ai_max_jump_dist( hook->run_speed, hook->upward_vel );

	hook->dflags			|= DFL_JUMPATTACK;
	hook->dflags			|= DFL_AMPHIBIOUS;

	hook->jump_chance		= 30;
	hook->attack_dist		= 40;
	hook->jump_attack_dist	= 120;

	self->health			= 75;
	hook->base_health		= 75;
	hook->pain_chance		= 10;
	self->mass				= 2.0;

	hook->fnStartAttackFunc = pipe_rat_begin_attack;
	hook->fnAttackFunc		= pipe_rat_attack;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give pipe_rat a weapon
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
	}
	hook->nAttackType = ATTACK_GROUND_MELEE;
	hook->nAttackMode = ATTACKMODE_NORMAL;

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
void world_ai_piperat_register_func()
{
	gstate->RegisterFunc("pipe_rat_begin_attack",pipe_rat_begin_attack);
	gstate->RegisterFunc("pipe_rat_attack",pipe_rat_attack);
}

