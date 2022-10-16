///////////////////////////////////////////////////////////////////////////////
//
//  Surgeon
//
///////////////////////////////////////////////////////////////////////////////

#include "world.h"
#include "ai.h"
#include "ai_utils.h"
//#include "ai_move.h"// SCG[1/23/00]: not used
//#include "ai_weapons.h"// SCG[1/23/00]: not used
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"
#include "ai_think.h"

/* ***************************** define types ****************************** */
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_surgeon( userEntity_t *self );

// ----------------------------------------------------------------------------
//
// Name:		WORKER_StartPain
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void surgeon_StartPain(userEntity_t *self, userEntity_t *attacker, float kick, int damage)
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	_ASSERTE( attacker );
	if ( AI_IsAlive( self ) && attacker->flags & (FL_CLIENT + FL_MONSTER + FL_BOT) )
	{
		//	attacked by a player or bot so attack back
		if ( attacker->flags & (FL_CLIENT + FL_BOT) )
		{
			hook->ai_flags &= ~AI_IGNORE_PLAYER;
		}

		AI_EnemyAlert( self, attacker );

		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
		if ( !pCurrentGoal )
		{
			AI_AddNewGoal( self, GOALTYPE_IDLE );
			pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
		}
        _ASSERTE( pCurrentGoal );

		TASK_PTR pCurrentTask = GOAL_GetCurrentTask( pCurrentGoal );
		TASKTYPE nTaskType = TASK_GetType( pCurrentTask );
		if ( nTaskType == TASKTYPE_COWER )
		{
			// cower for 15 seconds more
			AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
			_ASSERTE( pAIData );
			pAIData->fValue = gstate->time + 15.0f;
		}
		else
		{
			AI_AddNewTaskAtFront( self, TASKTYPE_COWER, 15.0f );
		}
	}
	AI_StartPain(self,attacker,kick,damage);
}

///////////////////////////////////////////////////////////////////////////////
//
//  monster_surgeon
//
///////////////////////////////////////////////////////////////////////////////
void monster_surgeon(userEntity_t *self)
{
	playerHook_t *hook		= AI_InitMonster(self, TYPE_SKINNYWORKER);

	self->className			= "monster_surgeon";
	self->netname			= tongue_monsters[T_MONSTER_SURGEON];

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

	hook->ai_flags			|= AI_IGNORE_PLAYER;

	hook->fnStartAttackFunc = NULL;
	hook->fnAttackFunc		= NULL;
    hook->fnStartCower		= SKINNYWORKER_StartCower;
	hook->fnCower			= SKINNYWORKER_Cower;
    
    hook->walk_speed		= 35;

	self->pain				= surgeon_StartPain;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;


//	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );
	AIATTRIBUTE_SetInfo( self );// SCG[1/23/00]: pWeaponAttributes not used

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
void world_ai_surgeon_register_func()
{
	gstate->RegisterFunc("surgeon_StartPain",surgeon_StartPain);
	gstate->RegisterFunc("SKINNYWORKER_StartCower",SKINNYWORKER_StartCower);
	gstate->RegisterFunc("SKINNYWORKER_Cower",SKINNYWORKER_Cower);
	gstate->RegisterFunc("AI_ParseEpairs",AI_ParseEpairs);
}
