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
#include "ai_move.h"
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"

/* ***************************** define types ****************************** */
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_cyclops( userEntity_t *self );

///////////////////////////////////////////////////////////////////////////////////
// cyclops specific funcitons
///////////////////////////////////////////////////////////////////////////////////

void cyclops_attack( userEntity_t *self );


// ----------------------------------------------------------------------------
//
// Name:		cyclops_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void cyclops_set_attack_seq (userEntity_t *self)
{
    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {
	    playerHook_t *hook = AI_GetPlayerHook( self );

	    frameData_t *pSequence = FRAMES_GetSequence( self, "ataka" );
	    AI_ForceSequence(self, pSequence);
    }
    else
    {
        AI_FaceTowardPoint( self, self->enemy->s.origin );
    }
}

// ----------------------------------------------------------------------------
//
// Name:		cyclops_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void cyclops_attack(userEntity_t *self)
{
	playerHook_t	*hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) && AI_IsFacingEnemy( self, self->enemy ) )
	{
		ai_fire_curWeapon( self );
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible (self, self->enemy))
		{
			AI_RemoveCurrentTask( self );
			return;
		}
		else
		{
			cyclops_set_attack_seq(self);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		cyclops_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void cyclops_begin_attack(userEntity_t *self)
{
	self->velocity.Zero();

	cyclops_set_attack_seq(self);

}

// ----------------------------------------------------------------------------
//
// Name:		monster_cyclops
// Description:
//				spawns a skeleton during level load
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_cyclops(userEntity_t *self)
{
	playerHook_t *hook	= AI_InitMonster(self, TYPE_CYCLOPS);

	self->className		= "monster_cyclops";
	self->netname		= tongue_monsters[T_MONSTER_CYCLOPS];

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

	gstate->SetSize(self, -32.0, -32.0, -24.0, 32.0, 32.0, 64.0);

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	hook->fnStartAttackFunc = cyclops_begin_attack;
	hook->fnAttackFunc		= cyclops_attack;

	hook->run_speed			= 250;
	hook->walk_speed		= 100;
	hook->attack_speed		= 250;

	hook->attack_dist		= 128;

	self->health			= 200;
	hook->base_health		= 200;
	hook->pain_chance		= 10;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give cyclops a weapon
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
										  "punch", 
										  melee_punch, 
										  ITF_TRACE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}

	hook->nAttackType = ATTACK_GROUND_MELEE;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	AI_DetermineMovingEnvironment( self );

	gstate->LinkEntity(self);
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

