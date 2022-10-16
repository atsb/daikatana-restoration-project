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
void spider_jump_attack(userEntity_t *self);
void spider_melee_attack(userEntity_t *self);

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_spider (userEntity_t *self);

// ----------------------------------------------------------------------------
//
// Name:		spider_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void spider_set_attack_seq (userEntity_t *self)
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	frameData_t *pSequence = NULL;
	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	if ( AI_IsWithinAttackDistance( self, dist ) )
	{
		pSequence = FRAMES_GetSequence( self, "ataka" );
		AI_ForceSequence(self, pSequence);
		self->s.frameInfo.frameInc = 2;

		spider_melee_attack (self);
	}
	else
	{
		pSequence = FRAMES_GetSequence( self, "jumpa" );
		AI_ForceSequence(self, pSequence);

		spider_jump_attack (self);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		spider_jump_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void spider_jump_attack (userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	AI_FaceTowardPoint( self, self->enemy->s.origin );

	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	if ( self->s.frameInfo.frameState & FRSTATE_PLAYSOUND1 )
	{
		CVector jump_ang, vel;

		//	jump on this frame
		self->groundEntity = NULL;

		jump_ang = self->s.angles;
		jump_ang.AngleToVectors(forward, right, up);

		vel = forward * hook->run_speed * 1.5;
		self->velocity = vel;
		self->velocity.z = hook->upward_vel;

		self->s.frameInfo.frameState -= FRSTATE_PLAYSOUND1;
	}
	
	if (self->s.frameInfo.frameState & FRSTATE_PLAYSOUND2)
	{
		//	attack on this frame
		self->s.frameInfo.frameState -= FRSTATE_PLAYSOUND2;
				
		ai_fire_curWeapon( self );
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible (self, self->enemy))
		{
			AI_RemoveCurrentTask( self );

			return;
		}
		else
		{
			spider_set_attack_seq (self);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		spider_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void spider_melee_attack(userEntity_t *self)
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	
	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) )
	{
		ai_fire_curWeapon( self );
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		{
			AI_RemoveCurrentTask( self );

			return;
		}
		else
		{
			spider_set_attack_seq (self);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		spider_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void spider_attack(userEntity_t *self)
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	if ( AI_IsWithinAttackDistance( self, dist ) )
	{
		spider_melee_attack (self);
	}
	else
	{
		spider_jump_attack (self);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		spider_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void spider_begin_attack (userEntity_t *self)
{
	self->velocity.Zero();

	spider_set_attack_seq (self);
}

// ----------------------------------------------------------------------------
//
// Name:		spider_start_pain
// Description: pain function for spider that selects front or back hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void spider_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	AI_StartPain( self, attacker, kick, damage );
}

// ----------------------------------------------------------------------------
//
// Name:		monster_spider
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_spider(userEntity_t *self)
{
	playerHook_t *hook		= AI_InitMonster(self, TYPE_SPIDER);

	self->className			= "monster_spider";
	self->netname			= tongue_monsters[T_MONSTER_SPIDER];

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
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////////////////////

	hook->fnStartAttackFunc = spider_begin_attack;
	hook->fnAttackFunc		= spider_attack;
	self->pain				= spider_start_pain;

	hook->dflags			|= DFL_JUMPATTACK;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give spider a weapon
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
void world_ai_spider_register_func()
{
	gstate->RegisterFunc("spider_begin_attack",spider_begin_attack);
	gstate->RegisterFunc("spider_attack",spider_attack);
	gstate->RegisterFunc("spider_start_pain",spider_start_pain);
}

