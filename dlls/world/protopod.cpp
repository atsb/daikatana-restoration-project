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
//#include "ai_weapons.h"// SCG[1/23/00]: not used
#include "ai_frames.h"
#include "ai.h"
#include "ai_func.h"
#include "MonsterSound.h"
#include "ai_utils.h"
//#include "actorlist.h"// SCG[1/23/00]: not used
#include "spawn.h"

/* ***************************** define types ****************************** */
#define MONSTERNAME			"monster_protopod"

#define PROTOPOD_CLOSED		0x0000
#define PROTOPOD_HATCHED	0x0001

#define PROTOPOD_MAYCHAPES_HATCH_DISTANCE 512.0f
#define PROTOPOD_MAYCHAPES_HATCH_CHANCE 0.05f
#define PROTOPOD_HATCH_DISTANCE 200.0f

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_protopod (userEntity_t *self);

// ----------------------------------------------------------------------------
//
// Name:		PROTOPOD_SpawnSkeeter
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void PROTOPOD_SpawnSkeeter(userEntity_t* self)
{
	// spawn a monster of class hook->monsterclass
	//NSS[11/21/99]:We don't use this to spawn a SIMPLE entity...
	//userEntity_t *pNewEntity = com->SpawnDynamicEntity( self, "monster_slaughterskeet", true );
	userEntity_t *pNewEntity = gstate->SpawnEntity();
    if ( pNewEntity )
    {
		//NSS[11/21/99]:Set the origin and give a small offset
		pNewEntity->s.origin	= self->s.origin;
		pNewEntity->s.origin.z	+= 10.0f;
		
		//NSS[11/21/99]:THIS IS WHAT TURNS THE ENTITY INTO A SKEET!
		SPAWN_CallInitFunction( pNewEntity, "monster_slaughterskeet" );
		
		
		// since this is a brand new entity without any goals, add some
		AI_AddNewGoal( pNewEntity, GOALTYPE_IDLE );
		if ( self->enemy )
		{
			AI_AddNewGoal( pNewEntity, GOALTYPE_KILLENEMY, self->enemy );
		}
		AI_AddNewGoal( pNewEntity, GOALTYPE_SKEETER_HATCH );
	}

}


// ----------------------------------------------------------------------------
//
// Name:		PROTOPOD_StartWaitToHatch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void PROTOPOD_StartWaitToHatch(userEntity_t* self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	// if we already hatched, then leave
	if ( hook->nFlags & PROTOPOD_HATCHED )
	{
		AI_RemoveCurrentGoal( self );
		return;
	}

	frameData_t *pSequence = FRAMES_GetSequence( self, "amba" );

	if ( pSequence )
	{
		AI_StartSequence(self, pSequence, pSequence->flags);
	}

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, -1 );

	self->nextthink	= gstate->time + 0.1f;

	AI_Dprintf( "Starting TASKTYPE_PROTOPOD_WAIT_TO_HATCH.\n" );
}

//NSS[11/13/99]:
void Protpod_Hatch_It(userEntity_t *self)
{
	// we're done
	AI_RemoveCurrentTask( self, FALSE );
	self->think = AI_TaskThink;
	self->nextthink = 0.1f;
}

// ----------------------------------------------------------------------------
//
// Name:		PROTOPOD_WaitToHatch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void PROTOPOD_WaitToHatch(userEntity_t* self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_SetTaskFinishTime( hook, -1 );

	// Is the enemy visible nearby?
	//NSS[11/6/99]:Make sure you are still alive before you try doing things!
	// NSS[12/12/99]:Added health < basehealth no spawn
	// NSS[2/1/00]:Removed health < basehalth no spawn
	if(AI_IsAlive(self))
	{
		if ( self->enemy && AI_IsVisible( self, self->enemy ) && self->enemy->movetype != MOVETYPE_NOCLIP &&
			 (VectorXYDistance (self->s.origin, self->enemy->s.origin) <= PROTOPOD_HATCH_DISTANCE ||
			  (rnd() < PROTOPOD_MAYCHAPES_HATCH_CHANCE &&
			   VectorXYDistance (self->s.origin, self->enemy->s.origin) <= PROTOPOD_MAYCHAPES_HATCH_DISTANCE)))
		{
 			self->think		= Protpod_Hatch_It;
			self->nextthink = gstate->time + (0.50f + rnd()*3.0f);
		}
	}
}


// ----------------------------------------------------------------------------
//
// Name:		PROTOPOD_StartHatch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void PROTOPOD_StartHatch(userEntity_t* self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FREEZE )
	{
		return;
	}

	// if we already hatched, then leave
	if ( hook->nFlags & PROTOPOD_HATCHED )
	{
		return;
	}

	// set hatch flag
	hook->nFlags = PROTOPOD_HATCHED;

	// skeeter is gone, we don't weight much alone
	self->mass = .5;

	AI_ForceSequence( self, FRAMES_GetSequence( self, "hatcha" ) ); // urgent, so must be forced

	// spawn a skeeter for hatching
	PROTOPOD_SpawnSkeeter(self);

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1 );

	self->nextthink	= gstate->time + 0.1f;

	AI_Dprintf( "Starting TASKTYPE_PROTOPOD_HATCH.\n" );
}

// ----------------------------------------------------------------------------
// NSS[2/5/00]:
// Name:		PROTOPOD_Hatch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void PROTOPOD_Hatch(userEntity_t* self)
{
	ai_frame_sounds(self);
	
	if ( AI_IsEndAnimation( self ) )
	{
		gstate->SetSize(self, -8.0, -8.0, -2.0, 8.0, 8.0, 2.0);
		//NSS[11/7/99]:Once you are done stop thinking.
		//YOu don't need to be used again... wait until your pain function is triggered.
		//They need to also blow up real easy.
		self->health	= 1.0f;
		self->think		= NULL;
		self->postthink = NULL;
		self->prethink	= NULL;
		self->flags		&= ~FL_MONSTER;	
		AI_RemoveCurrentTask(self,FALSE);
	}		
}

int AI_CheckForEnemy( userEntity_t *self );
void protopod_start_idle( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	// if we already hatched, then leave
	if ( hook->nFlags & PROTOPOD_HATCHED )
	{
		return;
	}

	AI_StartSequence( self, "amba" );

	AI_StartIdleSettings( self );

	
	if(AI_CheckForEnemy(self))
	{
		AI_AddNewGoal( self, GOALTYPE_PROTOPOD_HATCH );
	}
}


// ----------------------------------------------------------------------------
//
// Name:		protopod_hatch_now
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void protopod_hatch_now(userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	// if we already hatched, then forget about it
	if ( hook->nFlags & PROTOPOD_HATCHED )
	{
		return;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	if( pCurrentGoal == NULL )
	{
		AI_AddNewGoal( self, GOALTYPE_IDLE );
	}
	//AI_AddNewGoal( self, GOALTYPE_PROTOPOD_HATCH );

	AI_AddNewTask( self, TASKTYPE_PROTOPOD_HATCH );
	PROTOPOD_StartHatch(self);

}

// ----------------------------------------------------------------------------
//
// Name:		monster_protopod
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_protopod( userEntity_t *self )
{
	playerHook_t *hook	= AI_InitMonster( self, TYPE_PROTOPOD );

	self->className		= MONSTERNAME;
	self->netname		= tongue_monsters[T_MONSTER_PROTOPOD];

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

	gstate->SetSize(self, -8.0, -8.0, -8.0, 8.0, 8.0, 8.0);

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	//	turn rates for each axis in degrees per second
	self->ang_speed.Set( 90, 90, 90 );

	self->movetype			= MOVETYPE_TOSS;

    self->fragtype          |= FRAGTYPE_NOBLOOD;   // don't leave blood upon death
    self->fragtype          |= FRAGTYPE_ROBOTIC;   // apply robotic gib parts
    self->fragtype          |= FRAGTYPE_ALWAYSGIB; // gib into protobits

	hook->active_distance	= 1000;
	hook->attack_dist		= 64;

	self->health			= 200;
	hook->base_health		= 200;
	hook->pain_chance		= 0; // no chance of pain sequence playing
	self->mass				= 2.0;

	self->s.render_scale.Set(2.0,2.0,2.0);

	hook->fnAttackFunc		= PROTOPOD_WaitToHatch;
	hook->fnStartAttackFunc = PROTOPOD_StartWaitToHatch;
	hook->fnStartIdle       = protopod_start_idle;

	hook->nFlags			= PROTOPOD_CLOSED;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;

	self->use               = protopod_hatch_now;

	AIATTRIBUTE_SetInfo( self );

	AI_DetermineMovingEnvironment( self );

    gstate->LinkEntity( self );

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
void world_ai_protopod_register_func()
{
	gstate->RegisterFunc("protopod_hatch_now",protopod_hatch_now);

	gstate->RegisterFunc("PROTOPOD_WaitToHatch",PROTOPOD_WaitToHatch);
	gstate->RegisterFunc("PROTOPOD_StartWaitToHatch",PROTOPOD_StartWaitToHatch);
	gstate->RegisterFunc("protopod_start_idle",protopod_start_idle);
	gstate->RegisterFunc("AI_ParseEpairs",AI_ParseEpairs);
	gstate->RegisterFunc("PROTOPOD_Hatch",PROTOPOD_Hatch);
	gstate->RegisterFunc("PROTOPOD_StartHatch",PROTOPOD_StartHatch);
	gstate->RegisterFunc("Protpod_Hatch_It",Protpod_Hatch_It);
	gstate->RegisterFunc("PROTOPOD_SpawnSkeeter",PROTOPOD_SpawnSkeeter);
}
