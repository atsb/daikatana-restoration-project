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
#include <stdlib.h>
//#include "dk_std.h"
#include "world.h"
#include "ai_utils.h"
//#include "ai_move.h"
//#include "ai_frames.h"
#include "ai.h"
#include "actorlist.h"
#include "nodelist.h"
//#include "GrphPath.h"
#include "ai_func.h"
//#include "ai_weapons.h"

#include "world.h"

//*****************************************************************************
//								Test code
//*****************************************************************************

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_SetTakeCover( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	int bSuccess = FALSE;
	if ( _stricmp( gstate->mapName, "aitest" ) == 0 || _stricmp( gstate->mapName, "deathtest" ) == 0 )
	{
		NODE_DisableLayingNodes();

		userEntity_t *pClient = alist_FirstEntity(client_list);
		if ( pClient )
		{
			GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( hook->pGoals );
			if ( pCurrentGoal == NULL )
			{
				pCurrentGoal = AI_AddNewGoal( self, GOALTYPE_IDLE );
			}

			// set client to enemy
			self->enemy = pClient;

			AI_AddNewTaskAtFront( self, TASKTYPE_TAKECOVER );

			bSuccess = TRUE;
		}
	}

	return bSuccess;
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
void ECTS_TakeCoverThink( userEntity_t *self )
{
	int bSuccess = FALSE;
	
	bSuccess = AI_SetTakeCover( self );

	if ( bSuccess )
	{
		self->think			= AI_TaskThink;
		self->nextthink		= gstate->time + 0.2;
	}
	else
	{
		self->think			= ECTS_TakeCoverThink;
		self->nextthink		= gstate->time + 1.0;
	}
}

#ifdef _DEBUG// SCG[12/9/99]: 
// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_TestTrain()
{
/*
	static int nTestNum = 0;
	static CVector pos1( -672, 768, 24 );
	static CVector pos2( -672, 768, -616 );

	if ( _stricmp( gstate->mapName, "insootrain" ) == 0 )
	{
		NODE_DisableLayingNodes();

		userEntity_t *pMonster = com->FindEntity( "monster_lycanthir" );
		_ASSERTE( pMonster );
		playerHook_t *hook = AI_GetPlayerHook( pMonster );
		_ASSERTE( hook );

		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) != TASKTYPE_WAITFORTRAINTOSTOP )
		{
			if ( nTestNum == 0)
			{
				AI_AddNewGoal( pMonster, GOALTYPE_MOVETOLOCATION, pos1 );
			}
			else
			if ( nTestNum == 1 )
			{
				AI_AddNewGoal( pMonster, GOALTYPE_MOVETOLOCATION, pos2 );
			}

			if ( nTestNum == 0 )
			{
				nTestNum = 1;
			}
			else
			{
				nTestNum = 0;
			}
		}
	}
*/
	static int nTestNum = 0;

	static CVector pos1( 672, 608, 24 );
	static CVector pos2( 165, -672, 1016 );

	if ( _stricmp( gstate->mapName, "aitest" ) == 0 )
	{
		NODE_DisableLayingNodes();

		userEntity_t *pMonster = com->FindEntity( "monster_blackprisoner" );
		_ASSERTE( pMonster );
		playerHook_t *hook = AI_GetPlayerHook( pMonster );
		_ASSERTE( hook );

		if ( pMonster->deadflag == DEAD_NO )
		{
			AI_SetStateRunning( hook );

			GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
			if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) != TASKTYPE_WAITFORTRAINTOSTOP )
			{
				if ( nTestNum == 0)
				{
					//set the entity's position.
					pMonster->s.origin = pos2;
					pMonster->s.origin.z += 0.125;
					pMonster->groundEntity = NULL;
					gstate->LinkEntity(pMonster);

					AI_AddNewGoal( pMonster, GOALTYPE_MOVETOLOCATION, pos1 );
				}
				else
				if ( nTestNum == 1 )
				{
					//set the entity's position.
					pMonster->s.origin = pos1;
					pMonster->s.origin.z += 0.125;
					pMonster->groundEntity = NULL;
					gstate->LinkEntity(pMonster);

					AI_AddNewGoal( pMonster, GOALTYPE_MOVETOLOCATION, pos2 );
				}

				if ( nTestNum == 0 )
				{
					nTestNum = 1;
				}
				else
				{
					nTestNum = 0;
				}
			}
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
int AI_TestTakeCover()
{
	int bSuccess = FALSE;
	if ( _stricmp( gstate->mapName, "aitest" ) == 0 )
	{
		NODE_DisableLayingNodes();

		userEntity_t *pClient = alist_FirstEntity(client_list);
		if ( pClient )
		{
			userEntity_t *pMonster = com->FindEntity( "monster_rocketgang" );
			if ( pMonster && pMonster->deadflag == DEAD_NO )
			{
				playerHook_t *hook = AI_GetPlayerHook( pMonster );

				GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( hook->pGoals );
				if ( pCurrentGoal == NULL )
				{
					pCurrentGoal = AI_AddNewGoal( pMonster, GOALTYPE_IDLE );
				}

				// set client to enemy
				pMonster->enemy = pClient;

				AI_AddNewTaskAtFront( pMonster, TASKTYPE_TAKECOVER );

				bSuccess = TRUE;
			}
		}
	}
	else
	if ( _stricmp( gstate->mapName, "deathtest" ) == 0 )
	{
		NODE_DisableLayingNodes();

		userEntity_t *pClient = alist_FirstEntity(client_list);
		if ( pClient )
		{
			userEntity_t *pMonster = com->FindEntity( "monster_sealcaptain" );
			if ( pMonster && pMonster->deadflag == DEAD_NO )
			{
				playerHook_t *hook = AI_GetPlayerHook( pMonster );

				GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( hook->pGoals );
				if ( pCurrentGoal == NULL )
				{
					pCurrentGoal = AI_AddNewGoal( pMonster, GOALTYPE_IDLE );
				}

				// set client to enemy
				pMonster->enemy = pClient;

				AI_AddNewTaskAtFront( pMonster, TASKTYPE_TAKECOVER );

				bSuccess = TRUE;
			}
		}
	}

	return bSuccess;
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
void AI_TestDoor()
{

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
void AI_TestLadder()
{
	static CVector pos1( 704, 896, -40 );
	static CVector pos2( 1344, 896, -424 );

	if ( _stricmp( gstate->mapName, "laddertest" ) == 0 )
	{
		NODE_DisableLayingNodes();

		userEntity_t *pMonster = com->FindEntity( "monster_lycanthir" );
		_ASSERTE( pMonster );
		playerHook_t *hook = AI_GetPlayerHook( pMonster );
		_ASSERTE( hook );

		if ( pMonster->deadflag == DEAD_NO )
		{
			AI_SetStateRunning( hook );

			//set the entity's position.
			pMonster->s.origin = pos2;
			pMonster->s.origin.z += 0.125;
			pMonster->groundEntity = NULL;
			gstate->LinkEntity(pMonster);

			AI_AddNewGoal( pMonster, GOALTYPE_MOVETOLOCATION, pos1 );
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
void AI_TestBoar()
{
	static nCounter = 0;
	if ( _stricmp( gstate->mapName, "battle1" ) == 0 )
	{
		static CVector pos1( -81, 200, 88 );
		static CVector pos2( 192, -1344, 88 );

		NODE_DisableLayingNodes();

		userEntity_t *pClient = alist_FirstEntity(client_list);
		if ( pClient )
		{
			userEntity_t *pMonster = com->FindEntity( "monster_battleboar" );
			if ( pMonster && pMonster->deadflag == DEAD_NO )
			{
				playerHook_t *hook = AI_GetPlayerHook( pMonster );

				GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( hook->pGoals );
				if ( pCurrentGoal == NULL )
				{
					pCurrentGoal = AI_AddNewGoal( pMonster, GOALTYPE_IDLE );
				}

				hook->ai_flags |= AI_IGNORE_PLAYER;

				if ( nCounter == 0 )
				{
					AI_AddNewGoal( pMonster, GOALTYPE_MOVETOLOCATION, pos1 );
				}
				else
				{
					AI_AddNewGoal( pMonster, GOALTYPE_MOVETOLOCATION, pos2 );
				}

				nCounter = ~nCounter;
			}
		}
	}
	else
	if ( _stricmp( gstate->mapName, "battle2" ) == 0 )
	{
		static CVector pos3( -33, 192, 113 );
		static CVector pos4( 192, -1344, 120 );

		NODE_DisableLayingNodes();

		userEntity_t *pClient = alist_FirstEntity(client_list);
		if ( pClient )
		{
			userEntity_t *pMonster = com->FindEntity( "monster_battleboar" );
			//userEntity_t *pMonster = com->FindEntity( "monster_lycanthir" );
			if ( pMonster && pMonster->deadflag == DEAD_NO )
			{
				playerHook_t *hook = AI_GetPlayerHook( pMonster );

				GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( hook->pGoals );
				if ( pCurrentGoal == NULL )
				{
					pCurrentGoal = AI_AddNewGoal( pMonster, GOALTYPE_IDLE );
				}

				hook->ai_flags |= AI_IGNORE_PLAYER;

				if ( nCounter == 0 )
				{
					AI_AddNewGoal( pMonster, GOALTYPE_MOVETOLOCATION, pos3 );
				}
				else
				{
					AI_AddNewGoal( pMonster, GOALTYPE_MOVETOLOCATION, pos4 );
				}

				nCounter = ~nCounter;
			}
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
#endif //_DEBUG// SCG[12/9/99]: 

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_test_register_func()
{
	gstate->RegisterFunc("ECTS_TakeCoverThink",ECTS_TakeCoverThink);
}
