// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#include "world.h"
#include "nodelist.h"
//#include "ai_move.h"// SCG[1/23/00]: not used
#include "ai_weapons.h"
#include "ai_frames.h"
#include "ai.h"
#include "ai_func.h"
#include "MonsterSound.h"
#include "ai_utils.h"


/* ***************************** define types ****************************** */
#define PALADIN_CHASE_MIN_ATTACK_MOVEMENT_DIST 40.0f
#define PALADIN_CHASE_MIN_DISTANCE 100.0f
#define PALADIN_CHASE_MAX_DISTANCE 250.0f
#define PALADIN_CHASE_PROBABILITY 0.7f
#define PALADIN_AXE_THROW_PROBABILITY 0.6f

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
void knight1_attack( userEntity_t *self );

/* **************************** Global Variables *************************** */

/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */
DllExport void monster_knight1( userEntity_t *self );


// ----------------------------------------------------------------------------
//
// Name:		knight1_check_range
// Description: range check function for paladin
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int knight1_check_range(userEntity_t *self, float dist, userEntity_t *enemy)
{
	self->curWeapon = gstate->InventoryFindItem(self->inventory, "swing");

	if ( dist < ((ai_weapon_t *)self->curWeapon)->distance )
		return TRUE;

	if ( rnd() < PALADIN_AXE_THROW_PROBABILITY )
	{
		self->curWeapon = gstate->InventoryFindItem(self->inventory, "FlameSword");
		if ( dist < ((ai_weapon_t *)self->curWeapon)->distance )
			return TRUE;
	}

	return ( dist > PALADIN_CHASE_MIN_DISTANCE && dist < PALADIN_CHASE_MAX_DISTANCE &&
	         rnd() < PALADIN_AXE_THROW_PROBABILITY );
}

// ----------------------------------------------------------------------------
//
// Name:		knight1_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void knight1_set_attack_seq( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	hook->nAttackMode = ATTACKMODE_NORMAL;

    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {
	    playerHook_t *hook = AI_GetPlayerHook( self );

	    frameData_t *pSequence = NULL;
	    float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
		AI_Dprintf( "%g\n", dist );
	    if ( dist > 80 )
	    {
			// ranged attack (axe throw)
			pSequence = FRAMES_GetSequence( self, "atakb" );
			AI_ForceSequence(self, pSequence); 

			hook->nAttackMode = ATTACKMODE_RANGED;

			self->curWeapon = gstate->InventoryFindItem(self->inventory, "FlameSword");
	    }
	    else
	    {
			// melee (axe chop)
			pSequence = FRAMES_GetSequence( self, "ataka" );
			AI_ForceSequence(self, pSequence);

			hook->nAttackMode = ATTACKMODE_MELEE;

			self->curWeapon = gstate->InventoryFindItem(self->inventory, "swing");
	    }
    }
}


// ----------------------------------------------------------------------------
//
// Name:		knight1_melee_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void knight1_melee_attack( userEntity_t *self )
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
			self->curWeapon = gstate->InventoryFindItem(self->inventory, "throwing axe");
			AI_RemoveCurrentTask( self );
			return;
		}
		else
			knight1_set_attack_seq( self );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		knight1_ranged_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void knight1_ranged_attack( userEntity_t *self )
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
			knight1_set_attack_seq (self);	// dwarves aren't going to do evasive action
	}

	self->nextthink			= gstate->time + 1.0;
}


// ----------------------------------------------------------------------------
//
// Name:		knight1_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void knight1_attack (userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->nAttackMode == ATTACKMODE_RANGED )
		knight1_ranged_attack(self);
	else
		knight1_melee_attack(self);

	self->lastAIFrame = self->s.frame;
}

// ----------------------------------------------------------------------------
//
// Name:		knight1_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void knight1_begin_attack(userEntity_t *self)
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	self->lastAIFrame = 0;

	AI_ZeroVelocity( self );

	knight1_set_attack_seq (self);
}

// ----------------------------------------------------------------------------
//
// Name:		knight1_start_pain
// Description: pain function for knight1 that selects front or back hit
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void knight1_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	ai_generic_pain_handler( self, attacker, kick,damage,35);
}

// ----------------------------------------------------------------------------
//
// Name:		knight1_start_die
// Description: die function for knight1
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void knight1_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
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
//NSS[11/3/99]:
// Name:Knight1_Think
// Description:Think trap for knight before AI_Think
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void KNIGHT1_Think(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	if(self->enemy)
	{
		if(AI_IsVisible(self,self->enemy) && hook->acro_boost==0)
		{
			//Attach the Sprite Light&Glow and fire stuff 
			trackInfo_t tinfo;

			// clear this variable
			memset(&tinfo, 0, sizeof(tinfo));

			tinfo.ent=self;
			tinfo.srcent=self;
			tinfo.fru.Zero();
			tinfo.lightColor.x = 0.95;//R
			tinfo.lightColor.y = 0.25;//G
			tinfo.lightColor.z = 0.15;//B
			tinfo.lightSize= 125.00;	
			
			tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX;
			tinfo.fxflags = TEF_LIGHT|TEF_FLAMESWORD|TEF_FX_ONLY;
			com->trackEntity(&tinfo,MULTICAST_PVS);
			hook->acro_boost = 1;
		}
		else if (!(AI_IsVisible(self,self->enemy)) && hook->acro_boost == 1)
		{
			RELIABLE_UNTRACK(self);
			hook->acro_boost = 0;
		}
	}

	AI_TaskThink( self);
}


// ----------------------------------------------------------------------------
//NSS[11/3/99]:
// Name:Knight1_ParseEpairs
// Description:Trap to set the think sequence
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void Knight1_ParseEpairs(userEntity_t *self)
{
	AI_ParseEpairs(self);
	self->think = KNIGHT1_Think;
	self->nextthink = gstate->time + 0.1f;
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
void monster_knight1( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_KNIGHT1 );

	self->className			= "monster_knight1";
	self->netname			= tongue_monsters[T_MONSTER_KNIGHT1];

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

	self->health			= 200;
	hook->base_health		= 200;
	hook->pain_chance		= 5;
	self->mass				= 2.0;

	hook->fnInAttackRange   = knight1_check_range;
	hook->fnStartAttackFunc = knight1_begin_attack;
	hook->fnAttackFunc		= knight1_attack;

	self->think				= Knight1_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give knight1 a weapon
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
										  "swing", 
										  melee_swing, 
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
										  "FlameSword", 
										  FlameSword_attack, 
										  ITF_PROJECTILE | ITF_EXPLOSIVE);
		gstate->InventoryAddItem (self, self->inventory, self->curWeapon);
	}

	hook->dflags			|= DFL_RANGEDATTACK;

	hook->nAttackType = ATTACK_GROUND_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	AI_DetermineMovingEnvironment( self );
	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;

	hook->pain_chance		= 5;

	gstate->LinkEntity(self);

	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);



}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_paladin_register_func()
{
	gstate->RegisterFunc("knight1_check_range",knight1_check_range);
	gstate->RegisterFunc("knight1_begin_attack",knight1_begin_attack);
	gstate->RegisterFunc("knight1_attack",knight1_attack);
	gstate->RegisterFunc("Knight1_ParseEpairs",Knight1_ParseEpairs);
	gstate->RegisterFunc("KNIGHT1_Think",KNIGHT1_Think);

}


#if 0
// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void knight1_set_attack_seq( userEntity_t *self )
{
    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {
	    playerHook_t *hook = AI_GetPlayerHook( self );

	    frameData_t *pSequence = FRAMES_GetSequence( self, "ataka" );
	    AI_ForceSequence( self, pSequence );
    }
    else
    {
        AI_FaceTowardPoint( self, self->enemy->s.origin );
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
void knight1_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) && AI_IsFacingEnemy( self, self->enemy ) )
	{
		ai_fire_curWeapon( self );
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible (self, self->enemy) )
		{
			AI_RemoveCurrentTask( self );

			return;
		}
		else
		{
			knight1_set_attack_seq( self );
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
void knight1_begin_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	self->velocity.Zero();

	hook->attack_finished = gstate->time + 1.0;

	knight1_set_attack_seq( self );
}


// ----------------------------------------------------------------------------
//
// Name:        knight1_start_idle
// Description: Start idle function that selects between two seqs
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void knight1_start_idle( userEntity_t *self )
{
    frameData_t *pSequence;
	float chance = rnd();

	if ( chance < 0.9 )
	{
		pSequence = FRAMES_GetSequence( self, "amba" );
	}
	else
	{
		// play ambb occasionally
		pSequence = FRAMES_GetSequence( self, "ambb" );
	}

    if ( pSequence )
    {
        if ( AI_StartSequence(self, pSequence, pSequence->flags) == FALSE )
		{
			return;
		}
    }

	AI_StartIdleSettings( self );
}
#endif 0

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

