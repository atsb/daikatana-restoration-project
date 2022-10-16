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
#include "ai.h"
#include "ai_func.h"
#include "MonsterSound.h"

/* ***************************** define types ****************************** */

// distance at which column attacks
#define COLUMN_ATTACK_DIST 200.0f
// distance at which column awakens
#define COLUMN_AWAKEN_DIST 256.0f
// amount of damage for a heavy hit
#define COLUMN_HEAVY_HIT_DAMAGE 20

// set once column starts waiting to wake up
#define COLUMN_WAITING 0x1
// set once column wakes up
#define COLUMN_AWAKE 0x2
// set once column experiences pain
#define COLUMN_PAIN 0x8

// mask for waking flags -- this is not a flag, it is a bitmask
#define COLUMN_NOTASLEEP 0x3

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */

void column_attack( userEntity_t *self );
void column_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &destPoint );
void column_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );
int column_check_range( userEntity_t *self, float dist, userEntity_t *enemy );

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

void COLUMN_StartAwakenWait(userEntity_t *self);
void COLUMN_AwakenWait(userEntity_t *self);
void COLUMN_StartAwaken(userEntity_t *self);
void COLUMN_Awaken(userEntity_t *self);

/* ******************************* exports ********************************* */
DllExport void monster_column( userEntity_t *self );


// ----------------------------------------------------------------------------
//
// Name:		COLUMN_StartAwakenWait
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void COLUMN_StartAwakenWait(userEntity_t *self)
{
	if( !self->enemy || !AI_IsVisible( self, self->enemy ) ||
	     VectorXYDistance (self->s.origin, self->enemy->s.origin) > COLUMN_AWAKEN_DIST )
	{
		playerHook_t *hook = AI_GetPlayerHook( self );

		// we shouldn't do pain until we move either
		hook->pain_chance = 0;

		// we can't move until we see something
		self->movetype = MOVETYPE_NONE;

		// get stuck in first awakening frame
		frameData_t *pSequence = FRAMES_GetSequence( self, "awaken" );
		AI_ForceSequence( self, pSequence );
		self->s.frameInfo.frameFlags |= FRAME_STATIC;

		AI_SetTaskFinishTime( hook, -1.0f );

		AI_Dprintf( "Starting TASKTYPE_COLUMN_AWAKEN_WAIT.\n" );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		COLUMN_AwakenWait
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void COLUMN_AwakenWait(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( self->enemy && AI_IsVisible( self, self->enemy ) &&
	     VectorXYDistance (self->s.origin, self->enemy->s.origin) <= COLUMN_AWAKEN_DIST )
	{
		// we're done, now restore supplanted idle goal to bottom of stack
		AI_AddNewGoalAtBack( self, GOALTYPE_IDLE );
		AI_RemoveCurrentGoal( self );
	}
	else
	{
		// wait
		AI_SetTaskFinishTime( hook, -1.0f );
	}
}


// ----------------------------------------------------------------------------
//
// Name:		COLUMN_StartAwaken
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void COLUMN_StartAwaken(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	// see if we haven't awakened already
	if( !hook->pain_chance && !(hook->nFlags & COLUMN_AWAKE) )
	{
		// Really OK to awaken?
		if ( self->enemy && AI_IsVisible( self, self->enemy ) &&
		     VectorXYDistance (self->s.origin, self->enemy->s.origin) <= COLUMN_AWAKEN_DIST )
		{
			// restore movement type
			self->movetype = hook->nOriginalMoveType;

			// restore sequence if FRAME_STATIC is set
			if ( self->s.frameInfo.frameFlags & FRAME_STATIC )
			{
				frameData_t *pSequence = FRAMES_GetSequence( self, "awaken" );
				AI_ForceSequence( self, pSequence );
			}

			AI_SetTaskFinishTime( hook, 10.0f );

			AI_Dprintf( "Starting TASKTYPE_COLUMN_AWAKEN.\n" );
		}
		else
		{
			// forget it
			AI_RemoveCurrentGoal( self );
		}
	}
	else
	{
		// we don't actually need to awaken...
		AI_RemoveCurrentTask( self );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		COLUMN_Awaken
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void COLUMN_Awaken(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->pain_chance > 1 )
	{
		AI_RemoveCurrentTask( self );
	}

	// The part of AI_CheckForEnemy() which alerts self to enemies
	// unfortunately restarts this task very frequently.
	// So here the flags are set to COLUMN_AWAKE, which indicates it's awake
	hook->nFlags = COLUMN_AWAKE;

	ai_frame_sounds( self );

	// End of animation?
	if ( AI_IsEndAnimation( self ) )
	{
		// OK to take pain now
		hook->pain_chance = 20;

		AI_RemoveCurrentTask( self );
	}
}


// ----------------------------------------------------------------------------
//
// Name:		column_check_range
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

int column_check_range( userEntity_t *self, float dist, userEntity_t *enemy )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	return dist < COLUMN_ATTACK_DIST;
}


// ----------------------------------------------------------------------------
//
// Name:		column_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void column_set_attack_seq( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	hook->nAttackMode = ATTACKMODE_NORMAL;

    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {
	    playerHook_t *hook = AI_GetPlayerHook( self );

		float dist = VectorDistance(self->s.origin, self->enemy->s.origin);

		if ( dist < ((ai_weapon_t *)self->curWeapon)->distance )
		{
			frameData_t *pSequence = FRAMES_GetSequence( self, "ataka" );
			AI_ForceSequence( self, pSequence );

			AI_ZeroVelocity( self );
		}
		else
		{
			frameData_t *pSequence = FRAMES_GetSequence( self, "atakb" );
			AI_ForceSequence( self, pSequence );

			hook->nAttackMode = ATTACKMODE_CHASE;
		}
    }
}

// ----------------------------------------------------------------------------
//
// Name:		column_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void column_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if(!AI_IsAlive(self))
	{
		AI_AddNewTaskAtFront(self,TASKTYPE_DIE,FALSE);
		return;
	}
	if(self->movetype == MOVETYPE_NONE)
		self->movetype = MOVETYPE_WALK;
	if ( hook->nAttackMode == ATTACKMODE_CHASE )
	{
		AI_MoveTowardPoint( self, self->enemy->s.origin, FALSE );
	}
	else
	{
		AI_FaceTowardPoint( self, self->enemy->s.origin );
	}

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) && AI_IsFacingEnemy( self, self->enemy ) )
	{
		ai_fire_curWeapon( self );
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance(self->s.origin, self->enemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		{
			AI_RemoveCurrentTask( self );
		}
		else
		{
			column_set_attack_seq (self);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		column_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void column_begin_attack( userEntity_t *self )
{
	column_set_attack_seq (self);
}


// ----------------------------------------------------------------------------
//
// Name:	column_start_idle
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void column_start_idle( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !(hook->nFlags & COLUMN_NOTASLEEP) )
	{
		// set the goal to awakening
		AI_AddNewGoal( self, GOALTYPE_COLUMN_AWAKEN );

		// now don't do this again
		hook->nFlags = COLUMN_WAITING;
	}

	AI_StartIdleSettings( self );
}


// ----------------------------------------------------------------------------
//
// Name:	column_start_die
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void column_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &destPoint )
{
	// Is it the hammer?
	if( (inflictor && inflictor->inventory && // only an entity with inventory will have a hammer
	     inflictor->curWeapon && inflictor->curWeapon->name &&
	     !_stricmp( inflictor->curWeapon->name, "weapon_hammer" )) || !_stricmp( inflictor->className, "hammer_controller" ))
	{
		// call AI_StartDie, which does generic dying stuff
		//AI_AddNewTaskAtFront(self,TASKTYPE_DIE,FALSE);
		AI_StartDie( self, inflictor, attacker, damage, destPoint );
		if(AI_IsAlive(self))
		{
			AI_Die(self);
		}
	}
	else
	{
		// don't die on me

		// set health to its minimum
		self->health = 1.0;
	}
}


// ----------------------------------------------------------------------------
//
// Name:	column_start_pain
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void column_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	// Is it the hammer?
	if( attacker && attacker->curWeapon && attacker->curWeapon->name &&
		!strcmp( attacker->curWeapon->name, "weapon_hammer" ) )
	{
		// always do pain the first time or on a heavy hit
		if ( !( hook->nFlags & COLUMN_PAIN) || damage > 90 )
		{
			hook->pain_chance = 100;
		}
		else
		{
			// Quake?
			if ( self->flags & FL_FREEZE )
			{
				hook->pain_chance = 0;
			}
		}

		AI_StartPain( self, attacker, kick, damage );

		// make sure we do nothing if self is dead
		if( AI_IsAlive( self ) )
		{
			if ( !(hook->nFlags & COLUMN_PAIN) || damage > 90 || (self->flags & FL_FREEZE) )
			{
				hook->nFlags |= COLUMN_PAIN;
				// revert the pain chance to normal
				hook->pain_chance = 20;
			}

			// no, it's not OK to attack if we already are
			if ( GOALSTACK_GetCurrentTaskType( AI_GetCurrentGoalStack( hook ) ) == TASKTYPE_ATTACK )
			{
				AI_SetOkToAttackFlag( hook, FALSE );
			}

			// In pain?
			if( hook->pain_finished < gstate->time )
			{
				// switch up pain sequence based on damage
				if( damage < COLUMN_HEAVY_HIT_DAMAGE )
					AI_ForceSequence( self, "hita" );
				else
					AI_ForceSequence( self, "hitb" );
				hook->pain_finished = gstate->time + (self->s.frameInfo.endFrame - self->s.frameInfo.startFrame) * self->s.frameInfo.frameTime;
			}

			if ( self->flags & FL_FREEZE )
			{
				GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

				// force stillness for the next 0.2 seconds
				AI_ForceSequence( self, "amba" );
				AI_StopCurrentSequence( self );
				AI_SetOkToAttackFlag( hook, FALSE );
				if ( pGoalStack && GOALSTACK_GetCurrentTaskType( pGoalStack ) != TASKTYPE_IDLE )
				{
					AI_AddNewTaskAtFront( self, TASKTYPE_IDLE );
				}
				AI_SetTaskFinishTime( hook, 0.2f );
			}
		}
	}
	else
	{
		// give ourselves a damage rebate, self can only be damaged by the hammer
		self->health += damage;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		monster_column
// Description:
//	spawns a column during level load
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_column( userEntity_t *self )
{
	playerHook_t *hook = AI_InitMonster( self, TYPE_COLUMN );

	self->className		= "monster_column";
	self->netname		= tongue_monsters[T_MONSTER_COLUMN];

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
	///////////////////////////////////////////////////////////

	// "robotic" is used so it doesn't draw blood -- no gibs are produced anyway
	self->fragtype     |= FRAGTYPE_NEVERGIB | FRAGTYPE_NOBLOOD | FRAGTYPE_ROBOTIC;
	hook->run_speed		= 200;
	hook->walk_speed	= 100;
	hook->attack_speed	= 200;
	hook->attack_dist	= 108;
	hook->max_jump_dist = ai_max_jump_dist( hook->run_speed, hook->upward_vel );

	self->health		= 1000;
	hook->base_health	= 1000;
	hook->pain_chance	= 20;
	
	hook->fnStartAttackFunc = column_begin_attack;
	hook->fnAttackFunc		= column_attack;

	hook->fnInAttackRange = column_check_range;
	hook->fnStartIdle     = column_start_idle;

	self->pain				= column_start_pain;
	self->die               = column_start_die;

	self->think			= AI_ParseEpairs;
	self->nextthink		= gstate->time + 0.2;

	self->s.render_scale.Set(1.3, 1.3, 1.3 );

	//////////////////////////////////////////////
	//	give column its melee weapon
	//////////////////////////////////////////////
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
										  ITF_TRACE);
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}

	hook->nFlags = 0;

	hook->nAttackType = ATTACK_GROUND_MELEE;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	hook->nOriginalMoveType = self->movetype;

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
void world_ai_column_register_func()
{
	gstate->RegisterFunc("column_begin_attack",column_begin_attack);
	gstate->RegisterFunc("column_attack",column_attack);
	gstate->RegisterFunc("column_check_range",column_check_range);
	gstate->RegisterFunc("column_start_idle",column_start_idle);
	gstate->RegisterFunc("column_start_pain",column_start_pain);
}

