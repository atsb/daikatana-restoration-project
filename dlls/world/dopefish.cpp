// ==========================================================================
//
//  File:dopefish.cpp
//  Contents:The Dopefish!
//  Author:NSS
//
// ==========================================================================
#include "world.h"
#include "ai_common.h"
#include "ai_utils.h"
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

DllExport void fish_dopefish( userEntity_t *self );
DllExport void e_dopefish( userEntity_t *self );
void FISH_Think(userEntity_t *self);

// ----------------------------------------------------------------------------
// NSS[3/14/00]:
// Name:		dopefish_set_attack_seq
// Description:Set it
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void dopefish_set_attack_seq (userEntity_t *self)
{
	frameData_t *pSequence = NULL;
	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	if ( AI_IsWithinAttackDistance( self, dist ) )
	{
		pSequence = FRAMES_GetSequence( self, "atak" );
		AI_ForceSequence(self, pSequence);
	}
}
void AI_BloodCloud(userEntity_t *self, userEntity_t *Attacker);


// ----------------------------------------------------------------------------
// NSS[3/14/00]:
// Name:		dopefish_attack
// Description:Munch it
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void dopefish_attack(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	if ( AI_IsWithinAttackDistance( self, dist ) )
	{
		AI_FaceTowardPoint( self, self->enemy->s.origin );

		AI_PlayAttackSounds( self );

		if ( AI_IsReadyToAttack1( self ) )
		{
			float health = 100.0f;
			if(self->enemy)
				health = self->enemy->health;
			ai_fire_curWeapon( self );
			if(self->enemy && health > self->enemy->health)
				AI_BloodCloud(self->enemy,self);
		}

	}
	if ( AI_IsEndAnimation( self ))
	{
		AI_RemoveCurrentTask( self );
	}
	
}

// ----------------------------------------------------------------------------
// NSS[3/14/00]:
// Name:		dopefish_begin_attack
// Description:Get it.
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void dopefish_begin_attack (userEntity_t *self)
{
	dopefish_set_attack_seq (self);
}
int AI_HasGoalInQueue( userEntity_t *self, GOALTYPE SearchGoal );
// ----------------------------------------------------------------------------
// NSS[3/14/00]:
// Name:		DOPEFISH_Think
// Description:Think dopefish think!
// Input: userEntity_t *self
// Output:NA
// Note:NA
// ----------------------------------------------------------------------------
void DOPEFISH_Think(userEntity_t *self)
{
	
	playerHook_t *hook = AI_GetPlayerHook(self);
	// NSS[3/14/00]:Deal with attacking only in a specific range
	if((self->enemy && hook) || hook->owner)
	{
		float Dist;
		if(hook->owner == NULL)
		{
			hook->owner = self->enemy;
		}
		if(hook->owner)
		{
			Dist = VectorDistance(hook->owner->s.origin,self->s.origin);
			if(Dist < 150.0f && rnd() < 0.50f)
			{
				AI_SetOkToAttackFlag( hook, TRUE );
				hook->ai_flags		&= ~AI_IGNORE_PLAYER;
			}
			else if( Dist > 175.0f )
			{
				AI_SetOkToAttackFlag( hook, FALSE );
				hook->ai_flags		|= AI_IGNORE_PLAYER;
				if(AI_HasGoalInQueue(self,GOALTYPE_KILLENEMY))
					AI_RemoveCurrentGoal(self);
			}
		}
	}
	AI_TaskThink(self);
	if(self->waterlevel < 3 && self->velocity.z > 0.0f)
	{
		self->velocity.z = 0.0f;
	}
}

// ----------------------------------------------------------------------------
// NSS[3/14/00]:
// Name:		DOPEFISH_ParseEpairs
// Description:Setup the dopefish
// Input:userEntity_t *self 
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void DOPEFISH_ParseEpairs( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook(self);

	AI_InitNodeList( self );
	Terrain_Node_Change(self, MOVETYPE_SWIM);
	AI_ParseEpairs(self);
	
	if(hook)
	{
		hook->ai_flags	&= ~AI_IGNORE_PLAYER;
	}
	
	
	self->think		=	DOPEFISH_Think;
	
	self->nextthink =	gstate->time + 0.10;
}





// ----------------------------------------------------------------------------
// NSS[3/6/00]:
// Name:        fish_dopefish
// Description: The DOPE fish!
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void fish_dopefish( userEntity_t *self )
{
	CVector	temp, ang;
	
	AI_InitMonster(self, TYPE_DOPEFISH);
	playerHook_t *hook  = AI_GetPlayerHook(self);
	self->className     = "e_dopefish";// Must be the same name as in the AIDATA.CSV file <nss>
	self->netname       = tongue_monsters[T_MONSTER_DOPEFISH];

	char *szModelName   = AIATTRIBUTE_GetModelName( self->className );
	_ASSERTE( szModelName );
	
	self->s.modelindex  = gstate->ModelIndex( szModelName );
	if ( !ai_get_sequences(self) )
	{
		char *szCSVFileName = AIATTRIBUTE_GetCSVFileName( self->className );
        if ( szCSVFileName )
        {
            FRAMEDATA_ReadFile( szCSVFileName, self );
        }
	}
	hook->fnStartAttackFunc = dopefish_begin_attack;
	hook->fnAttackFunc		= dopefish_attack;
	ai_register_sounds(self);
    //Get attributes from the CSV file
	AIATTRIBUTE_SetInfo( self );

	///////////////////////////////////////////////////////////////////////////
	//	give dopefish a weapon
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

	self->movetype  = MOVETYPE_SWIM;

	self->solid     = SOLID_BBOX;
	self->flags     |= FL_MONSTER;
 
	self->think     = DOPEFISH_ParseEpairs;
	self->prethink  = FISH_Update;
	

	//Need this bad boy...
	//Not only does it set the moving environment, but it also sets up node paths.. 
	AI_DetermineMovingEnvironment( self );
	self->fragtype  = FRAGTYPE_ALWAYSGIB;
	gstate->LinkEntity( self );
	AI_SetOkToAttackFlag( hook, FALSE );
	hook->ai_flags |= AI_CANSWIM;
	self->nextthink = gstate->time + 0.5 + rnd () * 0.5;

}
// NSS[3/16/00]:My bad... yuk!
void e_dopefish(userEntity_t *self)
{
	fish_dopefish( self );
}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_dopefish_register_func()
{
	gstate->RegisterFunc("DOPEFISH_ParseEpairs",DOPEFISH_ParseEpairs);
	gstate->RegisterFunc("FISH_Update",FISH_Update);
	gstate->RegisterFunc("DOPEFISH_Think",DOPEFISH_Think);
	gstate->RegisterFunc("dopefish_begin_attack",dopefish_begin_attack);
	gstate->RegisterFunc("dopefish_attack",dopefish_attack);

}
