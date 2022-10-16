// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#include "world.h"
#include "ai.h"
#include "ai_utils.h"
//#include "ai_move.h"// SCG[1/23/00]: not used
//#include "ai_weapons.h"// SCG[1/23/00]: not used
#include "ai_frames.h"
#include "nodelist.h"
#include "actorlist.h"
#include "ai_func.h"
#include "MonsterSound.h"

/* ***************************** define types ****************************** */



#define	PRISONER_SPEED			200
#define	WANDER_GOAL_DIST		REACHED_GOAL_DIST

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_prisoner (userEntity_t *self);
DllExport void monster_prisonerb (userEntity_t *self);
//DllExport void func_prisoner_respawn (userEntity_t *self);

void AI_PrisonerSpawnThink( userEntity_t *self );

////////////////////////////////////////////////////////////////////////////////
// frame data
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// prisoner specific funcitons
///////////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------------
//
// Name:		prisoner_find_target
// Description:
//				searches the entity list for targets and items
//				returns true if an item or targetable (enemy) entity was found
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
// SCG[1/23/00]: not used
/*
static int prisoner_find_target(userEntity_t *self)
{
	userEntity_t	*head;
	userEntity_t	*enemy;
	int				dist, enemy_d;
	playerHook_t	*hook = AI_GetPlayerHook( self );

	enemy = NULL;
	enemy_d = 150.0;
    head = alist_FirstEntity( client_list );
    while ( head )
	{
		if (head->health > 0 && !(head->flags & FL_NOTARGET))
		{
			if (head->deadflag == DEAD_NO)
			{
				dist = com->Visible (self, head);

				if (dist && dist < enemy_d)
				{
					enemy = head;
					enemy_d = dist;
				}
			}
		}

        head = alist_NextEntity( client_list );
	}

	if (enemy && enemy_d < 150.0)
	{
		if (hook->sight_sound) 
		{
			hook->sight_sound (self);
		}

		self->enemy = enemy;

		return	TRUE;
	}

	return	FALSE;
}
*/
// ----------------------------------------------------------------------------
//
// Name:		AI_PrisonerChooseWanderPoint
// Description:	Chooses a random wander goal for wandering during 
//				GOALTYPE_FOLLOW
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_PrisonerChooseWanderPoint( userEntity_t *self, CVector &wanderPoint )
{
	int bFound = FALSE;

	CVector end = self->s.origin + forward * WANDER_GOAL_DIST;
	tr = gstate->TraceBox_q2 (self->s.origin, self->s.mins, self->s.maxs, end, self, self->clipmask);
	if ( tr.fraction >= 1.0f )
	{
		bFound = TRUE;
	}

	if ( bFound == FALSE )
	{
		CVector facingAngle;
		float fAngleIncrement = 45.0f;
		for ( int i = 0; i < 3; i++ )
		{
			// rotate right
			facingAngle = self->s.angles;
			facingAngle.y += fAngleIncrement;
			facingAngle.y = AngleMod(facingAngle.y);

			facingAngle.AngleToVectors(forward, right, up);
			end = self->s.origin + forward * WANDER_GOAL_DIST;
			tr = gstate->TraceBox_q2 (self->s.origin, self->s.mins, self->s.maxs, end, self, self->clipmask);
			if ( tr.fraction >= 1.0f )
			{
				bFound = TRUE;
				break;
			}

			// rotate left
			facingAngle = self->s.angles;
			facingAngle.y -= fAngleIncrement;
			facingAngle.y = AngleMod(facingAngle.y);

			facingAngle.AngleToVectors(forward, right, up);
			end = self->s.origin + forward * WANDER_GOAL_DIST;
			tr = gstate->TraceBox_q2 (self->s.origin, self->s.mins, self->s.maxs, end, self, self->clipmask);
			if ( tr.fraction >= 1.0f )
			{
				bFound = TRUE;
				break;
			}

			fAngleIncrement += 45.0f;
		}

		if ( bFound == FALSE )
		{
			facingAngle = self->s.angles;
			facingAngle.y += 180.0f;
			facingAngle.y = AngleMod(facingAngle.y);

			facingAngle.AngleToVectors(forward, right, up);
			end = self->s.origin + forward * WANDER_GOAL_DIST;
			tr = gstate->TraceBox_q2 (self->s.origin, self->s.mins, self->s.maxs, end, self, self->clipmask);
			if ( tr.fraction < 1.0f )
			{
				float fDistance = (WANDER_GOAL_DIST * tr.fraction) - 16.0f;
				end = self->s.origin + forward * fDistance;
			}
		}
	}

	wanderPoint = end;
}

// ----------------------------------------------------------------------------
//
// Name:		PRISONER_Wander
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void PRISONER_Wander(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	float fDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsCloseDistance2( self , fDistance ) && fZDistance < 32.0f )
	{
		CVector wanderPoint;
		AI_PrisonerChooseWanderPoint( self, wanderPoint );
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
		TASK_Set( pCurrentTask, wanderPoint );

		AI_SetTaskFinishTime( hook, 5.0f );

		return;
	}

	AI_MoveTowardPoint( self, pAIData->destPoint, FALSE );

	self->nextthink	= gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
//
// Name:		PRISONER_StartWander
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void PRISONER_StartWander(userEntity_t *self)
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FREEZE )
	{
		return;
	}

	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) != TASKTYPE_PRISONER_WANDER )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_PRISONER_WANDER );
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_PRISONER_WANDER.\n" );

	AI_SetStateWalking( hook );
	if ( AI_StartMove( self ) == FALSE )
	{
		return;
	}

	CVector wanderPoint;
	AI_PrisonerChooseWanderPoint( self, wanderPoint );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	TASK_Set( pCurrentTask, wanderPoint );

	self->nextthink = gstate->time + 0.1f;

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 5.0f );
}

// ----------------------------------------------------------------------------
//
// Name:		PRISONER_Cower
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void PRISONER_Cower(userEntity_t *self)
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	float dist = VectorDistance(self->enemy->s.origin, self->s.origin);
	if ( dist > 200.0f )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	self->nextthink	= gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
//
// Name:		PRISONER_StartCower
// Description:
//				prisoner cowers and faces his enemy
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void PRISONER_StartCower(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) != TASKTYPE_PRISONER_COWER )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_PRISONER_COWER );
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_PRISONER_COWER.\n" );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_ZeroVelocity( self );

	frameData_t *pSequence = FRAMES_GetSequence( self, "amba" );
	if ( AI_StartSequence(self, pSequence, FRAME_LOOP) == FALSE )
	{
		return;
	}

	//ai_set_sequence_sounds (self, self->s.frame + 1, "monsters/prisoner/prisonertalk1.wav", -1, NULL);

	self->nextthink	= gstate->time + 0.1f;

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 10.0f );
}

// ----------------------------------------------------------------------------
//
// Name:		PRISONER_InmaterHasItInMe
// Description:
//				special think function for when an inmater is gibbing a prisoner
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
/*
void PRISONER_InmaterHasItInMe(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	self->deadflag	= DEAD_DEAD;

	gstate->LinkEntity( self );

	self->pain		= NULL;
	self->use		= NULL;
	self->die		= NULL;
	self->think		= NULL;
	self->prethink	= NULL;
	self->postthink = NULL;

	if ( hook->pGoals )
	{
		GOALSTACK_Delete( hook->pGoals );
		hook->pGoals = NULL;
	}
	if ( hook->pScriptGoals )
	{
		GOALSTACK_Delete( hook->pScriptGoals );
		hook->pScriptGoals = NULL;
	}

	AI_DoGib( self );

	alist_remove( self );
}
*/

// ----------------------------------------------------------------------------
//
// Name:		PRISONER_StartInmaterHasItInMe
// Description:
//				special think function for when an inmater is gibbing a prisoner
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
/*
void PRISONER_StartInmaterHasItInMe(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) != TASKTYPE_PRISONER_INMATERHASITINME )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_PRISONER_INMATERHASITINME );
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_PRISONER_INMATERHASITINME.\n" );

	self->view_ofs.Set( 0, 0, -8.0);
	self->deadflag		= DEAD_DYING;
	self->takedamage	= DAMAGE_NO;
	self->solid			= SOLID_NOT;
	self->movetype		= MOVETYPE_BOUNCE;

	if ( self->target )
	{
		// prisoner should target func_prisoner_respawn
		userEntity_t *ent = com->FindTarget( self->target );
		if ( ent )
		{
			// set up the think function to spawn a new prisoner
			ent->think     = AI_PrisonerSpawnThink;
			ent->nextthink = gstate->time + ent->delay;
		}
	}

	AI_ZeroVelocity( self );

	self->nextthink	= gstate->time + 1.0f;

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 10.0f );
}
*/
// ----------------------------------------------------------------------------
//
// Name:		monster_prisoner
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_prisoner(userEntity_t *self)
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_PRISONER );

	self->className			= "monster_prisoner";
	self->netname			= "prisoner";

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

	gstate->SetSize( self, -16.0, -16.0, -24.0, 16.0, 16.0, 32.0);

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	//hook->begin_wander		= prisoner_begin_wander;	
	//hook->begin_stand		= prisoner_begin_stand;
	//hook->begin_follow		= prisoner_begin_cower;
	//hook->special_think		= prisoner_holy_shit_the_inmater_has_it_in_me;

	hook->fnStartAttackFunc = NULL;
	hook->fnAttackFunc		= NULL;

	hook->run_speed			= 25;
	hook->walk_speed		= 25;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up at 0.1 after map load


	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );
	_ASSERTE( pWeaponAttributes );

	AI_DetermineMovingEnvironment( self );

    gstate->LinkEntity( self );
}


// ----------------------------------------------------------------------------
//
// Name:		monster_prisoner
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_prisonerb(userEntity_t *self)
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_PRISONER );

	self->className			= "monster_prisonerb";
	self->netname			= "prisonerb";

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

	gstate->SetSize( self, -16.0, -16.0, -24.0, 16.0, 16.0, 32.0);

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	hook->fnStartAttackFunc = NULL;
	hook->fnAttackFunc		= NULL;

	hook->run_speed			= 25;
	hook->walk_speed		= 25;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up at 0.1 after map load


	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );
	_ASSERTE( pWeaponAttributes );

	AI_DetermineMovingEnvironment( self );

    gstate->LinkEntity( self );
}

// ----------------------------------------------------------------------------
//
// Name:		monster_spawnprisoner
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_spawnprisoner(userEntity_t *self)
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_PRISONER );

	self->className			= "monster_prisoner";
	self->netname			= tongue_monsters[T_MONSTER_PRISONER];

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

	gstate->SetSize( self, -16.0, -16.0, -24.0, 16.0, 16.0, 32.0);

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	hook->fnStartAttackFunc = NULL;
	hook->fnAttackFunc		= NULL;

	hook->run_speed			= 25;
	hook->walk_speed		= 25;

	self->spawnflags		= self->spawnflags | SPAWN_WANDER;

	self->think				= AI_TaskThink;
	self->nextthink			= gstate->time + 0.2;	// node links are set up at 0.1 after map load

	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );
	_ASSERTE( pWeaponAttributes );

	AI_AddNewGoal( self, GOALTYPE_PRISONER_WANDER );

	AI_DetermineMovingEnvironment( self );
	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;

    gstate->LinkEntity( self );
	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

}

// ----------------------------------------------------------------------------
//
// Name:		AI_PrisonerSpawnThink
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_PrisonerSpawnThink( userEntity_t *self )
{
	AI_Dprintf( "AI_PrisonerSpawnThink\n" );

	userEntity_t *pNewPrisoner = gstate->SpawnEntity();
	if ( pNewPrisoner )
	{
		pNewPrisoner->target = self->targetname;
		pNewPrisoner->s.origin = self->s.origin;
		pNewPrisoner->s.angles = self->s.angles;

		monster_spawnprisoner( pNewPrisoner );
	}

	self->nextthink = gstate->time + self->delay;
	self->think = NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		func_prisoner_respawn
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
/*
void func_prisoner_respawn(userEntity_t *self)
{
	self->delay = 1.0;

	for	( int i = 0; self->epair[i].key != NULL; i++ )
	{
		if ( !stricmp(self->epair[i].key, "targetname") )
		{
			self->targetname = self->epair[i].value;
		}
		else 
		if ( !stricmp(self->epair[i].key, "time") )
		{
			self->delay = atof(self->epair[i].value);
		}
	}

	self->think		= AI_PrisonerSpawnThink;
	self->nextthink = gstate->time + self->delay;

	self->modelName = NULL;
}
*/

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_prisoner_register_func()
{
	gstate->RegisterFunc("AI_PrisonerSpawnThink",AI_PrisonerSpawnThink);
	gstate->RegisterFunc("AI_ParseEpairs",AI_ParseEpairs);


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

































#ifdef	OLDAI
/*
=============
prisoner_wander_sound

exits if sound_time is not up
otherwise, % chance a sound will be played
=============
*/

void	prisoner_wander_sound (userEntity_t *self)
{
	playerHook_t	*hook = AI_GetPlayerHook( self );
	float			r;

	if (hook->sound_time >= gstate->time)
		return;
/*
	// play a sound
	r = rnd ();
	if (r < 0.5)
		gstate->sound (self, CHAN_AUTO, "monsters/prisoner/prisonerwander1.wav", 255, ATTN_NORM);
	else
		gstate->sound (self, CHAN_AUTO, "monsters/prisoner/prisonerwander2.wav", 255, ATTN_NORM);
*/
	hook->sound_time = gstate->time + 10.0 + rnd () * 30.0;
}

///////////////////////////////
//	prisoner_sight_sound
//
///////////////////////////////

void	prisoner_sight_sound (userEntity_t *self)
{
/*
	if (rnd () < 0.5)
		gstate->sound (self, CHAN_AUTO, "monsters/prisoner/prisonersight1.wav", 255.0, ATTN_NORM);
	else
		gstate->sound (self, CHAN_AUTO, "monsters/prisoner/prisonersight2.wav", 255.0, ATTN_NORM);
*/
}
#endif



#if 0
#endif 0

