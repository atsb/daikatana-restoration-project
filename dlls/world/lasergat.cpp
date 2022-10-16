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
#include "nodelist.h"
//#include "ai_move.h"// SCG[1/23/00]: not used
#include "ai_weapons.h"
#include "ai_frames.h"
#include "ai_common.h"
#include "ai_func.h"
#include "MonsterSound.h"
#include "ai_utils.h"

/* ***************************** define types ****************************** */

#define MONSTERNAME			"monster_lasergat"

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_lasergat (userEntity_t *self);
void entAnimate(userEntity_t *ent,short first,short last,unsigned short frameflags,float frametime);

///////////////////////////////////////////////////////////////////////////////////
// lasergat specific funcitons
///////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
//
// Name:		lasergat_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//				ataka	=> ranged attack
//				atakb	=> melee attack
//
// ----------------------------------------------------------------------------
static void lasergat_set_attack_seq(userEntity_t *self)
{
	frameData_t *pSequence = FRAMES_GetSequence( self, "shoota" );
	AI_StartSequence( self, pSequence );
}


// ----------------------------------------------------------------------------
// NSS[2/26/00]:
// Name:		LASERGAT_Turn
// Description: The lasergat turning to face the player.
// Input: userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void LASERGAT_Turn(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	AIDATA_PTR pAIData = AI_GetAIDATA(self);
	if(pAIData)
	{
		AI_StopCurrentSequence( self );
		entAnimate(self,1,1,FRAME_ONCE,0.10);
		self->ang_speed.yaw = 10.0f;
		if(self->hacks < gstate->time)
		{
			gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("e1/m_lazergatservo.wav"),0.35f, 356.0f, 512.0f);
			self->hacks = gstate->time + 0.40;
		}
		AI_FaceTowardPoint( self, self->enemy->s.origin );
		AI_UpdatePitchTowardEnemy( self );
	}
}


// ----------------------------------------------------------------------------
// NSS[2/26/00]:
// Name:		lasergat_attack
// Description: The lasergat attack thing!
// Input: userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void lasergat_attack( userEntity_t *self )
{
	

	if(self->enemy && !AI_IsEnemyDead( self ))
	{
		// NSS[2/26/00]:makesure we are facing the enemy
		if( AI_IsFacingEnemy(self, self->enemy, 1.0f, 2.0f) )
		{
			
			if( AI_IsReadyToAttack1( self ) || AI_IsReadyToAttack2( self ) )
			{
				AI_PlayAttackSounds( self );
				self->curWeapon = gstate->InventoryFindItem(self->inventory, "laser");
				//gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("e1/m_lasergatshoota.wav"),1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
				ai_fire_curWeapon( self );
			}
		}
		else
		{
			LASERGAT_Turn(self);
		}

		// NSS[2/26/00]:Deal with the end of animation stuff
		if ( AI_IsEndAnimation( self ))
		{
			float fDistance = VectorDistance(self->s.origin,self->enemy->s.origin);
			if(!AI_IsWithinAttackDistance(self,fDistance,self->enemy))
			{
				AI_RemoveCurrentTask( self );
			}
			else
			{
				lasergat_set_attack_seq (self);
			}
		}
	}
	else
	{
		AI_RemoveCurrentTask( self );
	}
}

// ----------------------------------------------------------------------------
// NSS[2/26/00]:
// Name:		lasergat_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void lasergat_begin_attack (userEntity_t *self)
{
	AIDATA_PTR pAIData = AI_GetAIDATA(self);
	if(pAIData)
	{
		if( AI_IsFacingEnemy(self, self->enemy, 1.0f, 2.0f) )
		{
			lasergat_set_attack_seq (self);
		}
		else
		{
			LASERGAT_Turn(self);
		}
	}
}

// ----------------------------------------------------------------------------
// NSS[12/20/99]:
// Name:		monster_lasergat
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_lasergat( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_LASERGAT );

	self->className			= MONSTERNAME;
	self->netname			= tongue_monsters[T_MONSTER_LASERGAT];

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

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	self->movetype			= MOVETYPE_NONE;

    self->fragtype          |= FRAGTYPE_NOBLOOD;   // don't leave blood upon death
    self->fragtype          |= FRAGTYPE_ROBOTIC;   // apply robotic gib parts
    self->fragtype          |= FRAGTYPE_ALWAYSGIB;  

	hook->dflags			|= DFL_MECHANICAL;
	hook->dflags			|= DFL_FACEPITCH;

	hook->fnStartAttackFunc = lasergat_begin_attack;
	hook->fnAttackFunc		= lasergat_attack;

	hook->dflags			|= DFL_RANGEDATTACK;
	hook->bOkToAttack		= FALSE;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;

	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );

	self->inventory = gstate->InventoryNew (MEM_MALLOC);

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
										  "laser", 
										  lasergat_fire, 
										  ITF_PROJECTILE );
		gstate->InventoryAddItem (self, self->inventory, self->curWeapon);
	}

	hook->nAttackType = ATTACK_GROUND_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	AI_DetermineMovingEnvironment( self );
	self->view_ofs.Set(0.0f, 0.0f, 0.0f);

    gstate->LinkEntity( self );
	// NSS[2/26/00]:Make sure these two sounds are cached
	gstate->SoundIndex("e1/m_lasergatshoota.wav");
	gstate->SoundIndex("e1/m_lazergatservo.wav");

	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_lasergat_register_func()
{
	gstate->RegisterFunc( "lasergat_begin_attack", lasergat_begin_attack );
	gstate->RegisterFunc( "lasergat_attack", lasergat_attack );
	gstate->RegisterFunc( "AI_ParseEpairs", AI_ParseEpairs );
}