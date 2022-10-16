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
#include "ai.h"

/* ***************************** define types ****************************** */

// the minimum speed the enemy must have to give chase from melee range, squared
#define LYCANTHIR_CHASE_MIN_ENEMY_XY_SPEED_SQUARED 100.0f
// the probability that that will happen
#define LYCANTHIR_CHASE_PROBABILITY_MELEE 0.6f
// minimum distance for moving while chasing
#define LYCANTHIR_MIN_CHASE_ATTACK_MOVEMENT_DIST 60.0f

#define LYCANTHIR_JUMP_CHANCE 0.2f

// chance of doing an evasive action immediately after an attack
#define LYCANTHIR_JUMP_ATTACK_EVASIVE_CHANCE 0.1f
#define LYCANTHIR_CHASE_ATTACK_EVASIVE_CHANCE 0.1f
#define LYCANTHIR_MELEE_ATTACK_EVASIVE_CHANCE 0.2f

#define LYCANTHIR_RESURRECTION_DELAY 5.0f

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
void lycanthir_select_attack( userEntity_t *self );
void lycanthir_melee_attack( userEntity_t *self );
void lycanthir_chase_attack( userEntity_t *self );
void lycanthir_jump_attack( userEntity_t *self );
void lycanthir_set_attack_seq( userEntity_t *self );
void lycanthir_set_melee_attack_seq( userEntity_t *self );
void lycanthir_set_chase_attack_seq( userEntity_t *self );
void lycanthir_set_jump_attack_seq( userEntity_t *self );
void lycanthir_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );
void lycanthir_die_maybe( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                          int damage, CVector &destPoint );

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

void LYCANTHIR_StartResurrectWait(userEntity_t *self);
void LYCANTHIR_ResurrectWait(userEntity_t *self);
void LYCANTHIR_StartResurrect(userEntity_t *self);
void LYCANTHIR_Resurrect(userEntity_t *self);

/* ******************************* exports ********************************* */
DllExport void monster_lycanthir( userEntity_t *self );


// ----------------------------------------------------------------------------
//
// Name:        lycanthir_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void lycanthir_begin_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	lycanthir_select_attack( self );

	if ( hook->nAttackMode == ATTACKMODE_NORMAL )
	{
		// my mistake
		AI_AddNewTaskAtFront(self,TASKTYPE_CHASE);
		AI_StartNextTask( self );
	}
	else
	{
		lycanthir_set_attack_seq( self );
		hook->attack_finished = gstate->time + 1.0;
	}
}


// ----------------------------------------------------------------------------
//
// Name:        lycanthir_select_attack
// Description: picks an attack to use (melee, chase, jump)
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void lycanthir_select_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);

	// Within melee distance?
	if ( dist < hook->attack_dist )
	{
		float enemymovementdot;

		// check if enemy moving toward us
		self->s.angles.AngleToVectors( forward, right, up );
		enemymovementdot = DotProduct( forward, self->enemy->velocity );

		if ( enemymovementdot > -1.0f && // if enemy isn't moving toward us
		     self->enemy->velocity.x * self->enemy->velocity.x +
			 self->enemy->velocity.y * self->enemy->velocity.y >
			 LYCANTHIR_CHASE_MIN_ENEMY_XY_SPEED_SQUARED && // and speed is over minimum
		     rnd() < LYCANTHIR_CHASE_PROBABILITY_MELEE ) // and chance is correct
		{
			// set a chase attack
			hook->nAttackMode = ATTACKMODE_CHASE;
		}
		else
		{
			// do a melee attack
			hook->nAttackMode = ATTACKMODE_MELEE;
		}
	}
	else
	{
		// jump if distance and chance indicates, otherwise chase
		if ( dist < hook->max_jump_dist && rnd() < LYCANTHIR_JUMP_CHANCE )
		{
			hook->nAttackMode = ATTACKMODE_JUMP;
		}
		else
		{
			hook->nAttackMode = ATTACKMODE_CHASE;
		}
	}
}


// ----------------------------------------------------------------------------
//
// Name:        lycanthir_set_attack_seq
// Description: sets up sequence for whatever attack the lycanthir is doing
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void lycanthir_set_attack_seq( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->nAttackMode == ATTACKMODE_JUMP )
	{
		lycanthir_set_jump_attack_seq( self );
	}
	else if ( hook->nAttackMode == ATTACKMODE_CHASE )
	{
		lycanthir_set_chase_attack_seq( self );
	}
	else
	{
		lycanthir_set_melee_attack_seq( self );
	}
}


// ----------------------------------------------------------------------------
//
// Name:        lycanthir_set_melee_attack_seq
// Description: sets up attack while standing in place
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void lycanthir_set_melee_attack_seq( userEntity_t *self )
{
//    playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	frameData_t *pSequence = NULL;
	float chance = rnd();

	// randomly select a melee attack
	if ( chance < 0.33f )
	{
		pSequence = FRAMES_GetSequence( self, "ataka" );
	}
	else if ( chance < 0.666f )
	{
		pSequence = FRAMES_GetSequence( self, "atakb" );
	}
	else
	{
		pSequence = FRAMES_GetSequence( self, "atakc" );
	}
	
	AI_ForceSequence( self, pSequence );
}


// ----------------------------------------------------------------------------
//
// Name:        lycanthir_set_chase_attack_seq
// Description: sets up attack while chasing
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void lycanthir_set_chase_attack_seq( userEntity_t *self )
{
	AI_ForceSequence( self, FRAMES_GetSequence( self, "atakd" ) );
}


// ----------------------------------------------------------------------------
//
// Name:        lycanthir_set_jump_attack_seq
// Description: sets up attack while chasing
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void lycanthir_set_jump_attack_seq( userEntity_t *self )
{
	AI_ForceSequence( self, FRAMES_GetSequence( self, "jumpb" ) );
}


// ----------------------------------------------------------------------------
//
// Name:		lycanthir_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void lycanthir_attack(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( self->enemy )
	{
		// Run the attack function appropriate for our attack mode
		if ( hook->nAttackMode == ATTACKMODE_JUMP )
		{
			lycanthir_jump_attack( self );
		}
		else if ( hook->nAttackMode == ATTACKMODE_CHASE )
		{
			lycanthir_chase_attack( self );
		}
		else
		{
			lycanthir_melee_attack( self );
		}
	}
}


// ----------------------------------------------------------------------------
//
// Name:		lycanthir_jump_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void lycanthir_jump_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	AI_FaceTowardPoint( self, self->enemy->s.origin );

//	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);// SCG[1/23/00]: not used
	if ( self->s.frameInfo.frameState & FRSTATE_PLAYSOUND1 )
	{
		CVector jump_ang, vel;

		//	jump on this frame
		//NSS[11/14/99]: This is a No! No!
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
		AI_AddNewTaskAtFront(self,TASKTYPE_CHASE);
		AI_StartNextTask( self );
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
void lycanthir_chase_attack( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if( (AI_IsReadyToAttack1( self ) || AI_IsReadyToAttack2( self )) && AI_IsFacingEnemy( self, self->enemy ) )
		ai_fire_curWeapon( self );

	// NSS[2/11/00]:
	if(self->enemy)
	{
		float zDistance = VectorZDistance(self->s.origin, self->enemy->s.origin);
		if(zDistance > 64.0f)
		{
			AI_RestartCurrentGoal(self);
			return;
		}
	}

	
	// don't move if well inside melee range (but move toward enemy if not)
	if ( dist < LYCANTHIR_MIN_CHASE_ATTACK_MOVEMENT_DIST )
	{
		AI_ZeroVelocity( self );
	}
	else
	{
		AI_MoveTowardPoint( self, self->enemy->s.origin, FALSE );
	}

	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		if ( rnd() < LYCANTHIR_CHASE_ATTACK_EVASIVE_CHANCE )
		{
			AI_DoEvasiveAction( self );
		}
		else
		{
			AI_AddNewTaskAtFront(self,TASKTYPE_CHASE);
			AI_StartNextTask( self );
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
void lycanthir_melee_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_ZeroVelocity( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if( (AI_IsReadyToAttack1( self ) || AI_IsReadyToAttack2( self )) && AI_IsFacingEnemy( self, self->enemy ) )
		ai_fire_curWeapon( self );
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );
		if ( dist > hook->attack_dist || !com->Visible(self, self->enemy) )
		{
			AI_AddNewTaskAtFront(self,TASKTYPE_CHASE);
			AI_StartNextTask( self );
			return;
		}
		else
		{
			if ( rnd() < LYCANTHIR_MELEE_ATTACK_EVASIVE_CHANCE )
			{
				AI_DoEvasiveAction( self );
			}
			else
			{
				lycanthir_set_attack_seq( self );
			}
		}
	}
}


// ----------------------------------------------------------------------------
//
// Name:        lycanthir_start_pain
// Description: "Pain of the Lycanthir"
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void lycanthir_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALTYPE goaltype = GOALSTACK_GetCurrentGoalType( AI_GetCurrentGoalStack( hook ) );

	// Is it the (gasp) silverclaw?
	if( attacker && (attacker->flags & (FL_CLIENT|FL_BOT)) &&
	    attacker->curWeapon && attacker->curWeapon->name &&
		!strcmp( attacker->curWeapon->name, "weapon_silverclaw" ) )
	{
		// do pain unless self is playing dead
		if( goaltype != GOALTYPE_LYCANTHIR_RESURRECT )
		{
			ai_generic_pain_handler( self, attacker, kick,damage,35);
		}
	}
	else
	{
		// check whether seeking resurrect goal
		if ( goaltype == GOALTYPE_LYCANTHIR_RESURRECT )
		{
			// give ourselves a damage rebate, we can't be damaged while in resurrection mode
			self->health = 1.0;
		}
		else
		{
			ai_generic_pain_handler( self, attacker, kick,damage,35);
		}
	}
}


// ----------------------------------------------------------------------------
//
// Name:        lycanthir_die_maybe
// Description: die function for lycanthir... maybe it'll die, maybe it won't
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void lycanthir_die_maybe( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                          int damage, CVector &destPoint )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	// Is it the silverclaw?
	if( (inflictor && inflictor->inventory && // only an entity with inventory will have a silverclaw
	     inflictor->curWeapon && inflictor->curWeapon->name &&
	     !strcmp( inflictor->curWeapon->name, "weapon_silverclaw" )) )
	{
		int notdeadyet = (self->deadflag == DEAD_NO);

		damage = 32000; // big gib (should gib always anyhow, but let's be clear)
		if(hook)
		{
			gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("e3/m_lycanthirhowla.wav"), 0.65f, hook->fMinAttenuation, hook->fMaxAttenuation);
		}
		
		// call AI_StartDie, which does generic dying stuff
		AI_StartDie( self, inflictor, attacker, damage, destPoint );

		if ( notdeadyet && self->deadflag == DEAD_DYING )
		{
			AI_ForceSequence( self, "dieb" ); // collapse
		}
	}
	else
	{ // fake own death
		GOALTYPE goaltype = GOALSTACK_GetCurrentGoalType( AI_GetCurrentGoalStack( hook ) );

		// check our goal and add resurrect goal if necessary
		if ( goaltype != GOALTYPE_LYCANTHIR_RESURRECT )
		{
			// start resurrection goal
			AI_AddNewGoal( self, GOALTYPE_LYCANTHIR_RESURRECT );

			// notify other enemies
			AI_EnemyAlert( self, attacker );
		}

		// set health to its minimum
		self->health = 1.0;
	}
}


// ----------------------------------------------------------------------------
//
// Name:        LYCANTHIR_StartResurrectWait
// Description: function called when resurrect wait task starts
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void LYCANTHIR_StartResurrectWait(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_Dprintf( "Starting TASKTYPE_LYCANTHIR_RESURRECT_WAIT.\n" );

	// play death sequence
	AI_ForceSequence( self, "diea" );

	if(hook)
	{
		// not OK to attack while resurrecting
		AI_SetOkToAttackFlag( hook, FALSE );
		hook->fnStartAttackFunc = NULL;

		AI_ZeroVelocity( self );
		self->avelocity.Zero();

		self->movetype = MOVETYPE_TOSS;

		hook->force_rate	= self->s.mins;
		hook->ground_org	= self->s.maxs;
		
		self->svflags |= SVF_DEADMONSTER;

		AI_SetDeathBoundingBox(self);
		
		

		//NSS[11/16/99]:Time respawn based off of skill level.
		//0 = 3x :---->x
		//1 = 2x :---->y
		//2 = 1x :---->z
		CVector Skill_Values;
		Skill_Values.Set(LYCANTHIR_RESURRECTION_DELAY * 3,LYCANTHIR_RESURRECTION_DELAY * 2,LYCANTHIR_RESURRECTION_DELAY);
		self->hacks = gstate->time + AI_GetSkill(Skill_Values ); 
		AI_SetTaskFinishTime( hook, -1 );

		gstate->LinkEntity(self);
	}
}


// ----------------------------------------------------------------------------
//
// Name:        LYCANTHIR_StartResurrectWait
// Description: function called for resurrect wait task
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void LYCANTHIR_ResurrectWait(userEntity_t *self)
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_ZeroVelocity( self );

	self->avelocity.Zero();

	ai_frame_sounds( self );


	if(self->hacks < gstate->time)
	{
		tr = gstate->TraceBox_q2( self->s.origin, self->s.mins, self->s.maxs, self->s.origin, self, MASK_MONSTERSOLID );
		if(tr.startsolid && tr.ent->flags & (FL_CLIENT|FL_BOT|FL_MONSTER) && tr.ent != self)
		{
			self->hacks = gstate->time + 1.0f;
		}
		else
		{
			AI_RemoveCurrentTask(self,FALSE);
		}
	}

}


// ----------------------------------------------------------------------------
//
// Name:        LYCANTHIR_StartResurrectWait
// Description: function called when resurrect task starts
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void LYCANTHIR_StartResurrect(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_ForceSequenceInReverse( self, "diea", FRAME_ONCE );



	if(hook)
	{
		self->svflags &= ~SVF_DEADMONSTER;
		gstate->SetSize(self,hook->force_rate.x,hook->force_rate.y,hook->force_rate.z,hook->ground_org.x,hook->ground_org.y,hook->ground_org.z);
		self->s.mins = hook->force_rate;
		self->s.maxs = hook->ground_org;
		hook->ground_org.Set(0.0f,0.0f,0.0f);
		hook->force_rate.Set(0.0f,0.0f,0.0f);
		gstate->LinkEntity(self);
	}


	AI_SetTaskFinishTime( hook, 10.0f );
	AI_Dprintf( "Starting TASKTYPE_LYCANTHIR_RESURRECT.\n" );
}


// ----------------------------------------------------------------------------
//
// Name:        LYCANTHIR_Resurrect
// Description: function called for resurrect task
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void LYCANTHIR_Resurrect(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_ZeroVelocity( self );

	// Are we back on our feet?
	if ( AI_IsEndAnimation( self ) )
	{
//		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( AI_GetCurrentGoalStack( hook ) );
		GOALSTACK_GetCurrentGoal( AI_GetCurrentGoalStack( hook ) );// SCG[1/23/00]: pCurrentGoal not used

		// set health to a healthy number
		self->health = hook->base_health;

		// OK to attack again
		AI_SetOkToAttackFlag( hook, TRUE );
		hook->fnStartAttackFunc = lycanthir_begin_attack;

		// move as normal
	    self->movetype = hook->nOriginalMoveType;

		// Goal complete, now go right back to killing enemy!
		AI_RemoveCurrentGoal( self );
	}
}


// ----------------------------------------------------------------------------
//
// Name:        monster_lycanthir
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_lycanthir( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_LYCANTHIR );

	self->className			= "monster_lycanthir";
	self->netname			= tongue_monsters[T_MONSTER_LYCANTHIR];

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

	hook->dflags			|= DFL_JUMPATTACK;
	hook->attack_dist		= 80;
	hook->jump_attack_dist	= 1000;
	hook->jump_chance       = 100;  // can always chase-attack from the jump attack distance

	self->fragtype          |= FRAGTYPE_ALWAYSGIB; // always gib (if killed with the silverclaw)
	self->health			= 200;
	hook->base_health		= 200;
	hook->pain_chance		= 10;
	self->mass				= 2.0;

	hook->fnStartAttackFunc = lycanthir_begin_attack;
	hook->fnAttackFunc		= lycanthir_attack;

	self->pain              = lycanthir_start_pain;
	self->die               = lycanthir_die_maybe;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give lycanthir a weapon
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
	hook->nOriginalMoveType = self->movetype;

	hook->pain_chance		= 15;

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
void world_ai_lycanthir_register_func()
{
	gstate->RegisterFunc("lycanthir_start_pain",lycanthir_start_pain);
	gstate->RegisterFunc("lycanthir_die_maybe",lycanthir_die_maybe);

	gstate->RegisterFunc("lycanthir_begin_attack",lycanthir_begin_attack);
	gstate->RegisterFunc("lycanthir_attack",lycanthir_attack);
}
