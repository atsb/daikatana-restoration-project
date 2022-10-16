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

#define MONSTERNAME			"monster_ragemaster"
#define MODELNAME			"models/e1/m_ragemaster.dkm"
#define CSV_NAME			"sounds/e1/m_ragemaster.csv"

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_ragemaster (userEntity_t *self);

////////////////////////////////////////////////////////////////////////////////
//	globals
////////////////////////////////////////////////////////////////////////////////

void ragemaster_attack (userEntity_t *self);

// ----------------------------------------------------------------------------
//
// Name:		ragemaster_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//				ataka	=> attack from the right side
//				atakb	=> attack from right followed by attack from left
//				atakc	=> attack from both sides at same time
//				atakd	=> attack while walking
//				atake	=> wacking attack 
//
// ----------------------------------------------------------------------------
void ragemaster_set_attack_seq (userEntity_t *self)
{
	_ASSERTE( self );

    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {

	    float dist = VectorDistance( self->s.origin, self->enemy->s.origin );
	    if ( AI_IsWithinAttackDistance( self, dist ) )
	    {
		    frameData_t *pSequence = FRAMES_GetSequence( self, "atake" );
		    AI_ForceSequence(self, pSequence);
		    ragemaster_attack (self);
		}
	    else
	    {
		    AI_RemoveCurrentTask( self );
		    return;
        }
    }
    else
    {
        AI_FaceTowardPoint( self, self->enemy->s.origin );
    }
}


// ----------------------------------------------------------------------------
//
// Name:		ragemaster_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ragemaster_attack (userEntity_t *self)
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook(self);
	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {
	    if(hook && hook->cur_sequence && hook->cur_sequence->animation_name && strstr(hook->cur_sequence->animation_name,"atak"))
		{
			if ( AI_IsReadyToAttack1( self ))
			{
   				ai_fire_curWeapon( self );
			}
			if ( AI_IsReadyToAttack2( self ) )
			{
				ai_fire_curWeapon( self );
			}
		}
    }
    	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
		_ASSERTE( hook );
		
		float dist = VectorDistance(self->s.origin, self->enemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		{
			AI_RemoveCurrentTask( self );
			return;
		}
		else
		{
			ragemaster_set_attack_seq (self);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		ragemaster_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ragemaster_begin_attack (userEntity_t *self)
{
	AI_ZeroVelocity( self );
	self->curWeapon = gstate->InventoryFindItem(self->inventory, "punch");
	ragemaster_set_attack_seq(self);

}

// ----------------------------------------------------------------------------
//
// Name:	ragemaster_start_pain
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ragemaster_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,65);
}

///////////////////////////////////////////////////////////////////////////////
//
//  monster_ragemaster
//
///////////////////////////////////////////////////////////////////////////////
void monster_ragemaster (userEntity_t *self)
{
	_ASSERTE( self );

	playerHook_t *hook	= AI_InitMonster (self, TYPE_RAGEMASTER);

	self->className		= MONSTERNAME;
	self->netname		= tongue_monsters[T_MONSTER_RAGEMASTER];

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

	gstate->SetSize(self, -24.0, -24.0, -24.0, 24.0, 24.0, 64.0);

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	hook->fnStartAttackFunc = ragemaster_begin_attack;
	hook->fnAttackFunc		= ragemaster_attack;
	self->pain				= ragemaster_start_pain;

	hook->dflags			|= DFL_MECHANICAL;

	hook->upward_vel		= 100.0;
	hook->max_jump_dist		= ai_max_jump_dist( hook->run_speed, hook->upward_vel );

	hook->run_speed			= 100;
	hook->walk_speed		= 50;
	hook->attack_speed		= 100;

	hook->attack_dist		= 100;

	self->health			= 200;
	hook->base_health		= 200;
	hook->pain_chance		= 20;
	self->mass = 2.0;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

    self->fragtype          |= FRAGTYPE_NOBLOOD;   // don't leave blood upon death
    self->fragtype          |= FRAGTYPE_ROBOTIC;   // apply robotic gib parts

	self->viewheight =	32;

	///////////////////////////////////////////////////////////////////////////
	//	give ragemaster a weapon
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
		gstate->InventoryAddItem(self, self->inventory, self->curWeapon);	
	}

	hook->pain_chance		= 5;

	hook->nAttackType = ATTACK_GROUND_MELEE;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	hook->ai_flags	  &= ~AI_CANDODGE;
	// NSS[2/12/00]:Flag which allows AI to use these things..
	// NSS[3/15/00]:Commented out... most maps the Ragemaster cannot fit through the doors the mappers created... thus the best solution is to not allow them to use the doors.
	//hook->dflags  |= DFL_CANUSEDOOR;

	AI_DetermineMovingEnvironment( self );

    gstate->LinkEntity( self );

	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_ragemaster_register_func()
{
	gstate->RegisterFunc("ragemaster_begin_attack",ragemaster_begin_attack);
	gstate->RegisterFunc("ragemaster_attack",ragemaster_attack);
	gstate->RegisterFunc("ragemaster_start_pain",ragemaster_start_pain);
	gstate->RegisterFunc("AI_ParseEpairs",AI_ParseEpairs);

}