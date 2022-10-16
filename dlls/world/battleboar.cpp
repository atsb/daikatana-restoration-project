// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#include "world.h"
#include "ai_utils.h"
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "ai_aim.h"
#include "MonsterSound.h"

/* ***************************** define types ****************************** */

#define MONSTERNAME			"monster_battleboar"

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_battleboar (userEntity_t *self);

static int Battleboar_IsClearShot( userEntity_t *self, userEntity_t *pEnemy )
{
	ai_weapon_t	*pWeapon = (ai_weapon_t *) self->curWeapon;
    if ( !pWeapon )
    {
        return FALSE;
    }
    
	self->s.angles.AngleToVectors(forward, right, up);

	CVector weaponOrigin = self->s.origin;
	CVector enemyOrigin = pEnemy->s.origin;

	weaponOrigin.z += 15.0;

	enemyOrigin.z += 12.0;
	if( W_IsTargetCrouching( pEnemy ) )
		enemyOrigin.z -= 32.0;

    tr = gstate->TraceLine_q2( weaponOrigin, enemyOrigin, self, MASK_SHOT );


	if( tr.ent && AI_IsAlive( tr.ent ) && tr.ent == pEnemy )
		return TRUE;
	else
		return FALSE;

}

// ----------------------------------------------------------------------------
//
// Name:		battleboar_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//			ataka => guns
//			atakb => missile
//
// ----------------------------------------------------------------------------
void battleboar_set_attack_seq (userEntity_t *self)
{
	if ( AI_IsFacingEnemy(self, self->enemy, 10.0f, 25.0f) )
	{
//		playerHook_t *hook = AI_GetPlayerHook( self );	// SCG[1/23/00]: not used

		int i = 0;
		frameData_t *pSequence = FRAMES_GetSequence( self, "ataka" );
		float fDistance = VectorDistance(self->s.origin, self->enemy->s.origin);
		if ( fDistance > 120.0f )
		{
			if ( rnd() > 0.15f )
			{
				pSequence = FRAMES_GetSequence( self, "atakb" );
				i = 1;
			}
		}
		AI_ForceSequence( self, pSequence );

		switch ( i )
		{
			case 0:
			{
				self->curWeapon = gstate->InventoryFindItem(self->inventory, "Boargun");
				break;
			}
			case 1:
			{
				//	pick the rocket from inventory
				self->curWeapon = gstate->InventoryFindItem(self->inventory, "BoarRocket");
				break;
			}
			default:
			{
				break;
			}
		}
	}
	else
	{
    	AI_FaceTowardPoint( self, self->enemy->s.origin );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		battleboar_ranged_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void battleboar_ranged_attack( userEntity_t *self )
{
	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if( Battleboar_IsClearShot( self, self->enemy ) )
	{
		if ( AI_IsReadyToAttack1( self ) && AI_IsFacingEnemy( self, self->enemy ) )
		{
			ai_fire_curWeapon( self );
		}

		if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
		{
//			playerHook_t *hook = AI_GetPlayerHook( self );	// SCG[1/23/00]: not used

			float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
			if ( !AI_IsWithinAttackDistance( self, dist ) || !AI_IsVisible(self, self->enemy) )
			{
				AI_RemoveCurrentTask( self );
				return;
			}
			else
			{
				battleboar_set_attack_seq (self);
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		battleboar_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void battleboar_begin_attack( userEntity_t *self )
{
	battleboar_set_attack_seq( self );
}


// ----------------------------------------------------------------------------
//
// Name:		battleboar_start_pain
// Description: pain function for battleboar that selects front or back hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void battleboar_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_StartPain( self, attacker, kick, damage );

	if( AI_IsAlive( self ) && hook->pain_finished < gstate->time )
	{
		switch( rand() % 3 )
		{
			case 0:
				AI_ForceSequence( self, "hita" );
				break;
			case 1:
				AI_ForceSequence( self, "hitc" );
				break;
			case 2:
				AI_ForceSequence( self, "hitb" );
				break;
		}
		hook->pain_finished = gstate->time + (self->s.frameInfo.endFrame - self->s.frameInfo.startFrame) * self->s.frameInfo.frameTime;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		battleboar_start_die
// Description: die function for battleboar
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void battleboar_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                     int damage, CVector &destPoint )
{
	int notdeadyet = (self->deadflag == DEAD_NO);

	// call AI_StartDie, which does generic dying stuff
	AI_StartDie( self, inflictor, attacker, damage, destPoint );

    if ( notdeadyet && self->deadflag == DEAD_DYING )
	{ // pick a different sequence for different deaths
		if ( rnd() < 0.5f )
		{
			AI_ForceSequence( self, "diea" ); // fall over slowly
		}
		else
		{
			AI_ForceSequence( self, "dieb" ); // collapse
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		monster_battleboar
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_battleboar( userEntity_t *self )
{
	playerHook_t *hook	= AI_InitMonster(self, TYPE_BATTLEBOAR);

	self->className		= MONSTERNAME;
	self->netname		= tongue_monsters[T_MONSTER_BATTLEBOAR];

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

	self->movetype			= MOVETYPE_WHEEL;
    
    self->fragtype          |= FRAGTYPE_NOBLOOD;   // don't leave blood upon death
    self->fragtype          |= FRAGTYPE_ROBOTIC;   // apply robotic gib parts

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////
	
	hook->fnStartAttackFunc = battleboar_begin_attack;
	hook->fnAttackFunc		= battleboar_ranged_attack;

	hook->dflags			|= (DFL_RANGEDATTACK);// + DFL_ORIENTTOFLOOR);
	hook->dflags			-= (DFL_CANSTRAFE);
	hook->dflags			|= DFL_ORIENTTOFLOOR;

//	self->prethink = battleboar_prethink;

	hook->think_time		= THINK_TIME;

	self->pain				= battleboar_start_pain;
	hook->pain_chance		= 30;
	self->die				= battleboar_start_die;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;

	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );

	///////////////////////////////////////////////////////////////////////////
	//	give monster its weapon
	///////////////////////////////////////////////////////////////////////////
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
										  "Boargun", 
										  boargun_fire, 
										  ITF_BULLET);
		gstate->InventoryAddItem (self, self->inventory, self->curWeapon);
		self->curWeapon = ai_init_weapon( self, 
										  pWeaponAttributes[1].fBaseDamage, 
										  pWeaponAttributes[1].fRandomDamage,
										  pWeaponAttributes[1].fSpreadX,
										  pWeaponAttributes[1].fSpreadZ,
										  pWeaponAttributes[1].fSpeed,
										  pWeaponAttributes[1].fDistance,
										  pWeaponAttributes[1].offset, 
										  "BoarRocket", 
										  boar_rocket_attack, 
										  ITF_PROJECTILE | ITF_EXPLOSIVE);
		gstate->InventoryAddItem (self, self->inventory, self->curWeapon);
	}

	hook->pain_chance		= 25;

	hook->nAttackType = ATTACK_GROUND_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	AI_DetermineMovingEnvironment( self );

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
void world_ai_battleboar_register_func()
{
	gstate->RegisterFunc( "battleboar_start_pain", battleboar_start_pain );
	gstate->RegisterFunc( "battleboar_start_die", battleboar_start_die );

	gstate->RegisterFunc( "AI_ParseEpairs", AI_ParseEpairs );

	gstate->RegisterFunc( "battleboar_begin_attack", battleboar_begin_attack );
	gstate->RegisterFunc( "battleboar_ranged_attack", battleboar_ranged_attack );
}
