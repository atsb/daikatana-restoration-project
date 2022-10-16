// ==========================================================================
//
//  File:		NavTest
//  Contents:
//				Test a map's navigation by spawning a monster and have it 
//				roam around
//  Author:
//
// ==========================================================================
// Logic[5/25/99]: Bad, bad, bad  #include <windows.h>
#include "dk_system.h" //use this instead, please!

#include "world.h"
#include "ai_common.h"
#include "ai_utils.h"
//#include "ai_move.h"// SCG[1/23/00]: not used
//#include "ai_weapons.h"// SCG[1/23/00]: not used
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"
#include "ai.h"
#include "collect.h"
#include "spawn.h"
#include "ai_think.h"

/* ***************************** define types ****************************** */

class CNodeVisitPerAI
{
private:
	userEntity_t *pAI;
	
	int nNumNodes;
	int *pNodeVisitCounter;

public:
	CNodeVisitPerAI( userEntity_t *pEntity = NULL );
	~CNodeVisitPerAI();

//	void* CNodeVisitPerAI::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* CNodeVisitPerAI::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  CNodeVisitPerAI::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CNodeVisitPerAI::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }
	
	userEntity_t *GetAI()		{ return pAI; }
	int FindLeastVisitedNodeIndex();
	void NoPathToNodeIndex( int nNodeIndex );

	void NodeVisited( int nNodeIndex );
};

/* ***************************** Local Variables *************************** */

static CPtrList *pNodeVisitCounterList;

/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

DllExport void monster_bot( userEntity_t *self );

/* ******************************* exports ********************************* */


// ----------------------------------------------------------------------------
//						CNodeVisitPerAI functions
// ----------------------------------------------------------------------------

CNodeVisitPerAI::CNodeVisitPerAI( userEntity_t *pEntity )
{
	pAI = pEntity;
	
	if ( pAI )
	{
		NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( pAI );
		_ASSERTE( pNodeHeader );

		nNumNodes = pNodeHeader->nNumNodes;

		pNodeVisitCounter = (int*)gstate->X_Malloc(sizeof(int)*nNumNodes,MEM_TAG_NODE);
//		pNodeVisitCounter = new int[nNumNodes];
//		memset( pNodeVisitCounter, 0, sizeof(int) * nNumNodes );
	}
	else
	{
		nNumNodes = 0;
		pNodeVisitCounter = NULL;
	}
}
// ----------------------------------------------------------------------------

CNodeVisitPerAI::~CNodeVisitPerAI()
{
	if ( pNodeVisitCounter )
	{
		gstate->X_Free(pNodeVisitCounter);
		//delete pNodeVisitCounter;
	}
}

// ----------------------------------------------------------------------------

int CNodeVisitPerAI::FindLeastVisitedNodeIndex()
{
	_ASSERTE( pNodeVisitCounter );

	// find a least visited node
	int nMin = 1000000000;
	int nLeastVisitedNodeIndex = -1;
	for ( int i = 0; i < nNumNodes; i++ )
	{
		if ( pNodeVisitCounter[i] < nMin )
		{
			nMin = pNodeVisitCounter[i];
			nLeastVisitedNodeIndex = i;
		}
	}
	
	return nLeastVisitedNodeIndex;
}

// ----------------------------------------------------------------------------

void CNodeVisitPerAI::NoPathToNodeIndex( int nNodeIndex )
{
	_ASSERTE( pNodeVisitCounter );
	_ASSERTE( nNodeIndex >= 0 && nNodeIndex < nNumNodes );

	pNodeVisitCounter[nNodeIndex] = 1000;
}

// ----------------------------------------------------------------------------

void CNodeVisitPerAI::NodeVisited( int nNodeIndex )
{
	_ASSERTE( pNodeVisitCounter );
	_ASSERTE( nNodeIndex >= 0 && nNodeIndex < nNumNodes );

	pNodeVisitCounter[nNodeIndex]++;
}

// ----------------------------------------------------------------------------
//					NAVTEST functions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Name:		NAVTEST_Init
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NAVTEST_Init()
{
	pNodeVisitCounterList = new CPtrList;
}

// ----------------------------------------------------------------------------
//
// Name:		NAVTEST_End
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NAVTEST_End()
{
	if ( pNodeVisitCounterList )
    {
        POSITION pos = pNodeVisitCounterList->GetHeadPosition();
        while (pos)
        {
		    CNodeVisitPerAI *pNodeVisitPerAI = (CNodeVisitPerAI*)pNodeVisitCounterList->GetNext(pos);
		    _ASSERTE( pNodeVisitPerAI );

		    delete pNodeVisitPerAI;
			pNodeVisitPerAI = NULL;
	    }

		delete pNodeVisitCounterList;
        pNodeVisitCounterList = NULL;
    }
}

// ----------------------------------------------------------------------------
//
// Name:		NAVTEST_AddToNodeVisitList
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NAVTEST_AddToNodeVisitList( userEntity_t *pEntity )
{
	_ASSERTE( pEntity );
	_ASSERTE( pNodeVisitCounterList );

	CNodeVisitPerAI *pNodeVisitPerAI = new CNodeVisitPerAI( pEntity );
	_ASSERTE( pNodeVisitPerAI );

	pNodeVisitCounterList->AddTail( pNodeVisitPerAI );

}

// ----------------------------------------------------------------------------
//
// Name:		NAVTEST_FindLeastVisitedNode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int NAVTEST_FindLeastVisitedNode( userEntity_t *pEntity )
{
	_ASSERTE( pEntity );
	_ASSERTE( pNodeVisitCounterList );

	NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( pEntity );
	_ASSERTE( pNodeHeader );

	POSITION pos = pNodeVisitCounterList->GetHeadPosition();
    while (pos)
    {
		CNodeVisitPerAI *pNodeVisitPerAI = (CNodeVisitPerAI*)pNodeVisitCounterList->GetNext(pos);
		_ASSERTE( pNodeVisitPerAI );

		if ( pNodeVisitPerAI->GetAI() == pEntity )
		{
			int nLeastVisitedNodeIndex = pNodeVisitPerAI->FindLeastVisitedNodeIndex();
			return nLeastVisitedNodeIndex;
		}
	}

	return -1;
}

// ----------------------------------------------------------------------------
//
// Name:		NAVTEST_NoPathToNodeIndex
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NAVTEST_NoPathToNodeIndex( userEntity_t *pEntity, int nNodeIndex )
{
	_ASSERTE( pEntity );
	_ASSERTE( pNodeVisitCounterList );

	NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( pEntity );
	_ASSERTE( pNodeHeader );

	POSITION pos = pNodeVisitCounterList->GetHeadPosition();
    while (pos)
    {
		CNodeVisitPerAI *pNodeVisitPerAI = (CNodeVisitPerAI*)pNodeVisitCounterList->GetNext(pos);
		_ASSERTE( pNodeVisitPerAI );

		if ( pNodeVisitPerAI->GetAI() == pEntity )
		{
			pNodeVisitPerAI->NoPathToNodeIndex( nNodeIndex );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		NAVTEST_NodeVisited
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NAVTEST_NodeVisited( userEntity_t *pEntity, int nNodeIndex )
{
	_ASSERTE( pEntity );
	_ASSERTE( pNodeVisitCounterList );

	NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( pEntity );
	_ASSERTE( pNodeHeader );

	POSITION pos = pNodeVisitCounterList->GetHeadPosition();
    while (pos)
    {
		CNodeVisitPerAI *pNodeVisitPerAI = (CNodeVisitPerAI*)pNodeVisitCounterList->GetNext(pos);
		_ASSERTE( pNodeVisitPerAI );

		if ( pNodeVisitPerAI->GetAI() == pEntity )
		{
			pNodeVisitPerAI->NodeVisited( nNodeIndex );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		NAVTEST_Start
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NAVTEST_Start( userEntity_t *self, const char *szAIClassName )
{
	char szClassName[32];
	if ( szAIClassName == NULL )
	{
		strcpy( szClassName, "monster_bot" );
	}
	else
	{
		strcpy( szClassName, szAIClassName );
	}

	userEntity_t *pMonster = gstate->SpawnEntity();

	// look for place to put player
	CVector spawnPoint;
	int bFoundSpot = SPAWN_FindNearSpawnPoint( self, spawnPoint );
	if ( !bFoundSpot )
	{
		com->Warning( "No info_player_start spawn points." );
		return;
	}
	pMonster->s.origin = spawnPoint;
	pMonster->s.angles = self->s.angles;

	playerHook_t *hook = NULL;
	if ( _stricmp( szClassName, "monster_bot" ) == 0 )
	{
		monster_bot( pMonster );
		
		hook = (playerHook_t*)pMonster->userHook;
		_ASSERTE( hook );
	}
	else
	{
		char szModelName[64];
		int nType = GetMonsterInfo( szClassName, szModelName );
		if ( nType < 0 )
		{
			com->Warning( "Monster %s was not found", szClassName );
			return;
		}

		hook					= AI_InitMonster( pMonster, nType );
		pMonster->className		= szClassName;

		pMonster->s.modelindex = gstate->ModelIndex( szModelName );

		////////////////////////////////////////////////////////////////
		// set up animation stuff
		////////////////////////////////////////////////////////////////
		if ( !ai_get_sequences( pMonster ) )
		{
			char szName[32];
			char szCsvName[64];
			int nCharCount = 0;
			for ( int i = 11; i < 64; i++ )
			{
				if ( szModelName[i] == '.' )
				{
					szName[nCharCount] = '\0';
					break;
				}
				szName[nCharCount++] = szModelName[i];
			}
			sprintf( szCsvName, "sounds/%s.csv", szName );

			FRAMEDATA_ReadFile( szCsvName, pMonster );
		}
	}
	gstate->LinkEntity( pMonster );

	hook->fnStartAttackFunc = NULL;
	hook->fnAttackFunc		= NULL;

	hook->nSpawnValue		|= SPAWN_IGNORE_PLAYER;
	hook->ai_flags			|= AI_IGNORE_PLAYER;

	pMonster->spawnflags	= pMonster->spawnflags | SPAWN_WANDER;

	pMonster->think			= AI_TaskThink;
	pMonster->nextthink		= gstate->time + 0.2;	// node links are set up at 0.1 after map load

	AIATTRIBUTE_SetInfo( pMonster );

	NAVTEST_AddToNodeVisitList( pMonster );

	AI_AddNewGoal( pMonster, GOALTYPE_TESTMAP_ROAM );

}

// ----------------------------------------------------------------------------
//
// Name:		NAVTEST_FindNextRoamNode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void NAVTEST_FindNextRoamNode( userEntity_t *self )
{
 	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	if ( hook->nMoveCounter > 100 )
	{
		int nLeastVisitedNodeIndex = NAVTEST_FindLeastVisitedNode( self );
		if ( nLeastVisitedNodeIndex )
		{
			MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, nLeastVisitedNodeIndex );
			if ( !AI_FindPathToPoint( self, pNode->position ) )
			{
				NAVTEST_NoPathToNodeIndex( self, nLeastVisitedNodeIndex );
			}
		}
		
		hook->nMoveCounter = 0;
	}
	else
	{
		MAPNODE_PTR pCurrentNode = NODE_GetClosestNode(self);
		if ( !pCurrentNode && pNodeList->nCurrentNodeIndex != -1 )
		{
			pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
		}

		MAPNODE_PTR pNextNode = BOT_FindPathNextWanderNode( self, pNodeHeader, pCurrentNode );
		if ( !pNextNode )
		{
			AI_Dprintf( "Did not find the next wander node!!!!.\n" );
			AI_AddNewGoal( self, GOALTYPE_IDLE );

			return;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartTestMapRoam
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartTestMapRoam( userEntity_t *self )
{
 	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_TESTMAP_ROAM );

	AI_SetStateRunning(hook);
	if ( AI_StartMove(self) == FALSE )
	{
		return;
	}

	NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	MAPNODE_PTR pCurrentNode = NODE_GetClosestNode(self);
	if ( !pCurrentNode && pNodeList->nCurrentNodeIndex != -1 )
	{
		pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
	}

	MAPNODE_PTR pNode = BOT_FindPathNextWanderNode( self, pNodeHeader, pCurrentNode );
	if ( !pNode )
	{
		AI_Dprintf( "Did not find the next wander node!!!!.\n" );
		AI_AddNewGoal( self, GOALTYPE_IDLE );
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_TESTMAP_ROAM.\n" );

	self->nextthink	= gstate->time + 0.1f;

	hook->nMoveCounter = 0;	// used to keep track of how long the bot is in a roam task

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1.0f );
}	

// ----------------------------------------------------------------------------
//
// Name:		AI_TestMapRoam
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_TestMapRoam( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( self );
	_ASSERTE( pNodeHeader );

	hook->nMoveCounter++;

	if ( hook->pPathList->pPath )
	{
		MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, hook->pPathList->pPath->nNodeIndex );
		if ( !AI_HandleUse( self )  )
		{
			if ( AI_MoveTowardPoint( self, pNode->position, TRUE ) == TRUE )
			{
				NAVTEST_NodeVisited( self, pNode->nIndex );
				
				// we're already at this node, so delete the first one
				PATHLIST_DeleteFirstInPath( hook->pPathList );

				NAVTEST_FindNextRoamNode( self );
				if ( hook->pPathList->pPath )
				{
					MAPNODE_PTR pNextNode = NODE_GetNode( pNodeHeader, hook->pPathList->pPath->nNodeIndex );
					if ( AI_HandleUse( self, pNodeHeader, pNode, pNextNode ) )
					{
						return;
					}
				}

				AI_TestMapRoam( self );
			}			
		}
	}
	else
	{
		NAVTEST_FindNextRoamNode( self );
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
