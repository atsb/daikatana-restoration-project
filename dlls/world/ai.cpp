// ========================================================================== 
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================

#include "world.h"

/* ***************************** define types **************************** */

//	NOTE!!	If you add a GOALTYPE here, also add the string equivalent for it
//			to GOALTYPE_STR in below!!!

char *GOALTYPE_STR [] =
{
	"GOALTYPE_IDLE",
	"GOALTYPE_KILLENEMY",
	"GOALTYPE_WANDER",
	"GOALTYPE_PATROL",
	"GOALTYPE_HIDE",
	"GOALTYPE_PICKUPITEM",
	"GOALTYPE_MOVETOLOCATION",
	"GOALTYPE_AMBUSH",
	"GOALTYPE_RUNAWAY",
	"GOALTYPE_SNIPE",
	"GOALTYPE_FOLLOW",
	"GOALTYPE_FLYTOLOCATION",
	"GOALTYPE_SWIMTOLOCATION",
	"GOALTYPE_FACEANGLE",
	"GOALTYPE_WAIT",
	"GOALTYPE_BACKUPALLATTRIBUTES",
	"GOALTYPE_RESTOREALLATTRIBUTES",
	"GOALTYPE_MODIFYTURNATTRIBUTE",
	"GOALTYPE_MODIFYRUNATTRIBUTE",

    "GOALTYPE_PLAYANIMATION",
    "GOALTYPE_SETIDLEANIMATION",

	"GOALTYPE_RANDOMWANDER",

	"GOALTYPE_STARTUSINGWALKSPEED",
	"GOALTYPE_STARTUSINGRUNSPEED",
	"GOALTYPE_MOVETOENTITY",

	"GOALTYPE_STAY",				//	for sidekicks: wait at current location until given a new follow mode
	"GOALTYPE_PATHFOLLOW",

	// monster specific goal should be placed here
	"GOALTYPE_INMATER_KILLPRISONER",

	"GOALTYPE_PRISONER_WANDER",

	"GOALTYPE_BUBOID_GETOUTOFCOFFIN",
	"GOALTYPE_BUBOID_MELT",

	"GOALTYPE_ROTWORM_DROPFROMCEILING",
	"GOALTYPE_ROTWORM_JUMPATTACK",

	"GOALTYPE_CAMBOT_PATHFOLLOW",
	"GOALTYPE_CAMBOT_FOLLOWPLAYER",

	"GOALTYPE_PROTOPOD_HATCH",
	
	"GOALTYPE_MEDUSA_GAZE",

	"GOALTYPE_SKEETER_HATCH",

	"GOALTYPE_LYCANTHIR_RESURRECT",

//	"GOALTYPE_SKINNYWORKER_WORK_A",

	NULL
};

/* ***************************** Local Variables **************************** */


/* ***************************** Local Functions **************************** */

/* ***************************** Global Variables *************************** */


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#include "ai_common.h"

//*******************************************************************************
//		TASK and GOAL managing functions 
//*******************************************************************************

// ----------------------------------------------------------------------------
//
// Name:		TASK_Allocate
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR TASK_Allocate()
{
//	TASK_PTR pTask = (TASK_PTR)gstate->X_Malloc(sizeof(TASK),MEM_TAG_AI);
	TASK_PTR pTask = (TASK_PTR)malloc(sizeof(TASK));
	memset(pTask,0,sizeof(TASK));
//	TASK_PTR pTask = new TASK;
//	if ( !pTask )
//	{
//		return NULL;
//	}

	pTask->nTaskType		= TASKTYPE_IDLE;
	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.pEntity = NULL;
	pTask->pNext			= NULL;
	pTask->taskData.pString	= NULL;

	return pTask;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Allocate
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType )
{
//	TASK_PTR pTask = (TASK_PTR)gstate->X_Malloc(sizeof(TASK),MEM_TAG_AI);
	TASK_PTR pTask = (TASK_PTR)malloc(sizeof(TASK));
	memset(pTask,0,sizeof(TASK));
//	if ( !pTask )
//	{
//		return NULL;
//	}

	pTask->nTaskType		= nNewTaskType;
	pTask->taskData.pString	= NULL;
	pTask->pNext			= NULL;

//	memset( &pTask->taskData, 0, sizeof(AIDATA) );

	return pTask;
}

TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, AIDATA_PTR aiData) 
{
    if (aiData == NULL) 
	{
		return NULL;
	}

    //allocate a new task.
//	TASK_PTR pTask = (TASK_PTR)gstate->X_Malloc(sizeof(TASK),MEM_TAG_AI);
	TASK_PTR pTask = (TASK_PTR)malloc(sizeof(TASK));
	memset(pTask,0,sizeof(TASK));
//    if ( !pTask )
//    {
//        return NULL;
//    }

    //set the task type.
    pTask->nTaskType		= nNewTaskType;
	pTask->taskData.pString	= NULL;
    pTask->pNext			= NULL;

     //copy the data.
    memcpy(&pTask->taskData, aiData, sizeof(AIDATA));

    return pTask;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Allocate
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, userEntity_t *pEntity )
{
//	TASK_PTR pTask = (TASK_PTR)gstate->X_Malloc(sizeof(TASK),MEM_TAG_AI);
	TASK_PTR pTask = (TASK_PTR)malloc(sizeof(TASK));
	memset(pTask,0,sizeof(TASK));
//	if ( !pTask )
//	{
//		return NULL;
//	}
	_ASSERTE( pEntity );

	pTask->nTaskType		= nNewTaskType;

//	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.pEntity = pEntity;
	pTask->taskData.pString	= NULL;
	pTask->pNext			= NULL;

	return pTask;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Allocate
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, CAction *pAction )
{
//	TASK_PTR pTask = (TASK_PTR)gstate->X_Malloc(sizeof(TASK),MEM_TAG_AI);
	TASK_PTR pTask = (TASK_PTR)malloc(sizeof(TASK));
	memset(pTask,0,sizeof(TASK));
//	if ( !pTask )
//	{
//		return NULL;
//	}
	_ASSERTE( pAction );

	pTask->nTaskType		= nNewTaskType;

//	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.pString	= NULL;
	pTask->taskData.pAction = pAction;
	pTask->pNext			= NULL;

	return pTask;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Allocate
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, const int nValue )
{
//	TASK_PTR pTask = (TASK_PTR)gstate->X_Malloc(sizeof(TASK),MEM_TAG_AI);
	TASK_PTR pTask = (TASK_PTR)malloc(sizeof(TASK));
	memset(pTask,0,sizeof(TASK));
//	if ( !pTask )
//	{
//		return NULL;
//	}

	pTask->nTaskType			  = nNewTaskType;
	
//	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.nValue	= nValue;
	pTask->taskData.pString	= NULL;
	pTask->pNext			= NULL;

	return pTask;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Allocate
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, const CVector &position )
{
//	TASK_PTR pTask = (TASK_PTR)gstate->X_Malloc(sizeof(TASK),MEM_TAG_AI);
	TASK_PTR pTask = (TASK_PTR)malloc(sizeof(TASK));
	memset(pTask,0,sizeof(TASK));
//	if ( !pTask )
//	{
//		return NULL;
//	}

	pTask->nTaskType			= nNewTaskType;
	
//	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.pString		= NULL;
	pTask->taskData.destPoint	= position;
	pTask->pNext				= NULL;
	pTask->taskData.pEntity		= NULL;

	return pTask;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Allocate
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, const float fValue )
{
//	TASK_PTR pTask = (TASK_PTR)gstate->X_Malloc(sizeof(TASK),MEM_TAG_AI);
	TASK_PTR pTask = (TASK_PTR)malloc(sizeof(TASK));
	memset(pTask,0,sizeof(TASK));
//	if ( !pTask )
//	{
//		return NULL;
//	}

	pTask->nTaskType	   = nNewTaskType;
	
//	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.pString	= NULL;
	pTask->taskData.fValue	= fValue;
	pTask->pNext			= NULL;

	return pTask;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Allocate
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, frameData_t *pNewSequence )
{
//	TASK_PTR pTask = (TASK_PTR)gstate->X_Malloc(sizeof(TASK),MEM_TAG_AI);
	TASK_PTR pTask = (TASK_PTR)malloc(sizeof(TASK));
	memset(pTask,0,sizeof(TASK));
//	if ( !pTask )
//	{
//		return NULL;
//	}

	pTask->nTaskType				= nNewTaskType;

//	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.pString			= NULL;
	pTask->taskData.pAnimSequence	= pNewSequence;
	pTask->pNext					= NULL;

	return pTask;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Allocate
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, char *szString )
{
//	TASK_PTR pTask = (TASK_PTR)gstate->X_Malloc(sizeof(TASK),MEM_TAG_AI);
	TASK_PTR pTask = (TASK_PTR)malloc(sizeof(TASK));
	memset(pTask,0,sizeof(TASK));
//	if ( !pTask )
//	{
//		return NULL;
//	}

	pTask->nTaskType		= nNewTaskType;

//	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.pString	= szString;
	pTask->pNext			= NULL;

	return pTask;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Allocate
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, void *pVoid )
{
//	TASK_PTR pTask = (TASK_PTR)gstate->X_Malloc(sizeof(TASK),MEM_TAG_AI);
	TASK_PTR pTask = (TASK_PTR)malloc(sizeof(TASK));
	memset(pTask,0,sizeof(TASK));
//	if ( !pTask )
//	{
//		return NULL;
//	}

	pTask->nTaskType		= nNewTaskType;

//	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.pString	= NULL;
	pTask->taskData.pVoid	= pVoid;
	pTask->pNext			= NULL;

	return pTask;
}

TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, char *szString, const float fValue )
{
//	TASK_PTR pTask = (TASK_PTR)gstate->X_Malloc(sizeof(TASK),MEM_TAG_AI);
	TASK_PTR pTask = (TASK_PTR)malloc(sizeof(TASK));
	memset(pTask,0,sizeof(TASK));
//	if ( !pTask )
//	{
//		return NULL;
//	}

	pTask->nTaskType		= nNewTaskType;

//	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.pString	= szString;
	pTask->taskData.fValue	= fValue;
	pTask->pNext			= NULL;

	return pTask;
}

TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, char *szString, const float fValue1, const float fValue2 )
{
//	TASK_PTR pTask = (TASK_PTR)gstate->X_Malloc(sizeof(TASK),MEM_TAG_AI);
	TASK_PTR pTask = (TASK_PTR)malloc(sizeof(TASK));
	memset(pTask,0,sizeof(TASK));
//	if ( !pTask )
//	{
//		return NULL;
//	}

	pTask->nTaskType		= nNewTaskType;

//	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.pString	= szString;
	pTask->taskData.nValue	= (int)fValue1;
	pTask->taskData.fValue	= fValue2;
	pTask->pNext			= NULL;

	return pTask;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Delete
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR TASK_Delete( TASK_PTR pTask )
{
	//delete pTask;
//	gstate->X_Free(pTask);
	free(pTask);

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void TASK_Set( TASK_PTR pTask, userEntity_t *pEntity )
{
	_ASSERTE( pTask );
	_ASSERTE( pEntity );

	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.pEntity = pEntity;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void TASK_Set( TASK_PTR pTask, CAction *pAction )
{
	_ASSERTE( pTask );
	_ASSERTE( pAction );

	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.pAction = pAction;
}


// ----------------------------------------------------------------------------
//
// Name:		TASK_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void TASK_Set( TASK_PTR pTask, const int nValue )
{
	_ASSERTE( pTask );

	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.nValue = nValue;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void TASK_Set( TASK_PTR pTask, const CVector &position )
{
	_ASSERTE( pTask );

	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.destPoint = position;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void TASK_Set( TASK_PTR pTask, const float fValue )
{
	_ASSERTE( pTask );

	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.fValue = fValue;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void TASK_Set( TASK_PTR pTask, char *szString )
{
	_ASSERTE( pTask );

	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.pString = szString;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void TASK_Set( TASK_PTR pTask, void *pVoid )
{
	_ASSERTE( pTask );

	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.pVoid = pVoid;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void TASK_Set( TASK_PTR pTask, TASKTYPE nNewTaskType, userEntity_t *pEntity )
{
	_ASSERTE( pTask );
	_ASSERTE( pEntity );

	pTask->nTaskType		= nNewTaskType;
	
	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.pEntity = pEntity;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void TASK_Set( TASK_PTR pTask, TASKTYPE nNewTaskType, const int nValue )
{
	_ASSERTE( pTask );

	pTask->nTaskType			  = nNewTaskType;
	
	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.nValue = nValue;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void TASK_Set( TASK_PTR pTask, TASKTYPE nNewTaskType, const CVector &position )
{
	_ASSERTE( pTask );

	pTask->nTaskType		  = nNewTaskType;
	
	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.destPoint = position;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void TASK_Set( TASK_PTR pTask, TASKTYPE nNewTaskType, const float fValue )
{
	_ASSERTE( pTask );

	pTask->nTaskType	   = nNewTaskType;
	
	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.fValue = fValue;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void TASK_Set( TASK_PTR pTask, TASKTYPE nNewTaskType, char *szString )
{
	_ASSERTE( pTask );

	pTask->nTaskType	   = nNewTaskType;
	
	memset( &pTask->taskData, 0, sizeof(AIDATA) );
	pTask->taskData.pString = szString;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_GetType
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASKTYPE TASK_GetType( TASK_PTR pTask )
{
	_ASSERTE( pTask );

	return pTask->nTaskType;
}

// ----------------------------------------------------------------------------
//
// Name:		TASK_GetData
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
AIDATA_PTR TASK_GetData( TASK_PTR pTask )
{
	_ASSERTE( pTask );

	return &(pTask->taskData);
}

//-----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//
// Name:		TASKQUEUE_Allocate
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASKQUEUE_PTR TASKQUEUE_Allocate()
{
//	TASKQUEUE_PTR pTaskQueue = (TASKQUEUE_PTR)gstate->X_Malloc(sizeof(TASKQUEUE),MEM_TAG_AI);
	TASKQUEUE_PTR pTaskQueue = (TASKQUEUE_PTR)malloc(sizeof(TASKQUEUE));
//	TASKQUEUE_PTR pTaskQueue = new TASKQUEUE;
//	if ( !pTaskQueue )
//	{
//		return NULL;
//	}

	pTaskQueue->nNumTasks  = 0;
	pTaskQueue->pStartList = NULL;
	pTaskQueue->pEndList   = NULL;	

	return pTaskQueue;
}

// ----------------------------------------------------------------------------
//
// Name:		TASKQUEUE_Delete
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASKQUEUE_PTR TASKQUEUE_Delete( TASKQUEUE_PTR pTaskQueue )
{
	TASK_PTR pTask = pTaskQueue->pStartList;
	while ( pTask != NULL )
	{
		TASK_PTR pNextTask = pTask->pNext;

		TASK_Delete( pTask );

		pTask = pNextTask;
	}

	pTaskQueue->pStartList = NULL;
	pTaskQueue->pEndList = NULL;

	//delete pTaskQueue;
//	gstate->X_Free(pTaskQueue);
	free(pTaskQueue);

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		TASKQUEUE_AddAtFront
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void TASKQUEUE_AddAtFront( TASKQUEUE_PTR pTaskQueue, TASK_PTR pTask )
{
	pTask->pNext = pTaskQueue->pStartList;
	pTaskQueue->pStartList = pTask;

	pTaskQueue->nNumTasks++;
}

// ----------------------------------------------------------------------------
//
// Name:		TASKQUEUE_Add
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void TASKQUEUE_Add( TASKQUEUE_PTR pTaskQueue, TASK_PTR pTask )
{
	if ( pTaskQueue->pStartList == NULL )
	{
		pTaskQueue->pStartList = pTask;
		pTaskQueue->pEndList = pTask;

	}
	else
	{
		// NSS[2/22/00]:Oh mother of god... these two lines were reversed...uh... aahahah...	
		if(pTaskQueue->pEndList)
		{
			pTaskQueue->pEndList->pNext = pTask;
			pTaskQueue->pEndList		= pTask;			
		}
		else
		{
			pTaskQueue->pStartList = pTask;
			pTaskQueue->pEndList = pTask;
		}
		
	}

	pTaskQueue->nNumTasks++;
}

// ----------------------------------------------------------------------------
//
// Name:		TASKQUEUE_GetCurrentTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR TASKQUEUE_GetCurrentTask( TASKQUEUE_PTR pTaskQueue )
{
	_ASSERTE( pTaskQueue );
	return pTaskQueue->pStartList;
}

// ----------------------------------------------------------------------------
//
// Name:		TASKQUEUE_RemoveCurrentTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void TASKQUEUE_RemoveCurrentTask( TASKQUEUE_PTR	pTaskQueue )
{
	if ( pTaskQueue && pTaskQueue->pStartList )
	{
		TASK_PTR pNextTask = pTaskQueue->pStartList->pNext;

		//delete pTaskQueue->pStartList;
//		gstate->X_Free(pTaskQueue->pStartList);
		free(pTaskQueue->pStartList);
		pTaskQueue->pStartList = NULL;

		pTaskQueue->pStartList = pNextTask;

		pTaskQueue->nNumTasks--;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		TASKQUEUE_GetNumTasks
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int TASKQUEUE_GetNumTasks( TASKQUEUE_PTR pTaskQueue )
{
	return pTaskQueue->nNumTasks;
}

// ----------------------------------------------------------------------------
//
// Name:		TASKQUEUE_ClearTasks
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void TASKQUEUE_ClearTasks( TASKQUEUE_PTR pTaskQueue )
{
	if ( !pTaskQueue)
	{
		return;
	}

	TASK_PTR pTask = pTaskQueue->pStartList;
	while ( pTask != NULL )
	{
		TASK_PTR pNextTask = pTask->pNext;

		TASK_Delete( pTask );
		//NSS[11/17/99]:Woops did we forget to do something?
		if(!pNextTask)
		{
			//NSS[11/17/99]:HOW ABOUT NULL OUT WHAT WE JUST CLEARED!!!!!!!!!
			pTaskQueue->pStartList = NULL;
			pTaskQueue->pEndList = NULL;
			pTaskQueue->nNumTasks = 0;
			return;
		}
		pTask = pNextTask;
	}

	pTaskQueue->pStartList = NULL;
	pTaskQueue->pEndList = NULL;
	pTaskQueue->nNumTasks = 0;
}

//-----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Name:		GOAL_Allocate
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR GOAL_Allocate()
{
//	GOAL_PTR pGoal = (GOAL_PTR)gstate->X_Malloc(sizeof(GOAL),MEM_TAG_AI);
	GOAL_PTR pGoal = (GOAL_PTR)malloc(sizeof(GOAL));
//	GOAL_PTR pGoal = new GOAL;
//	if ( !pGoal )
//	{
//		return NULL;
//	}
	memset( &pGoal->goalData, 0, sizeof(AIDATA) );

	pGoal->bFinished				= FALSE;
	pGoal->nGoalType				= GOALTYPE_IDLE;
	pGoal->goalData.pString			= NULL;
	pGoal->goalData.pEntity			= NULL;
	pGoal->goalData.destPoint		= NULL;
	pGoal->goalData.pAction			= NULL;
	pGoal->goalData.pAnimSequence	= NULL;
	pGoal->pTasks					= TASKQUEUE_Allocate();
	if ( !pGoal->pTasks )
	{
		return NULL;
	}

	pGoal->pNext	 = NULL;

	return pGoal;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_Allocate
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR GOAL_Allocate( GOALTYPE nNewGoalType )
{
//	GOAL_PTR pGoal = (GOAL_PTR)gstate->X_Malloc(sizeof(GOAL),MEM_TAG_AI);
	GOAL_PTR pGoal = (GOAL_PTR)malloc(sizeof(GOAL));
//	if ( !pGoal )
//	{
//		return NULL;
//	}

	pGoal->bFinished	  = FALSE;
	pGoal->nGoalType	  = nNewGoalType;
	pGoal->pTasks		  = TASKQUEUE_Allocate();
	if ( !pGoal->pTasks )
	{
		return NULL;
	}
	
	memset( &pGoal->goalData, 0, sizeof(AIDATA) );
	pGoal->pNext	 = NULL;

	return pGoal;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_Allocate
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR GOAL_Allocate( GOALTYPE nNewGoalType, AIDATA_PTR aiData ) 
{
//	GOAL_PTR pGoal = (GOAL_PTR)gstate->X_Malloc(sizeof(GOAL),MEM_TAG_AI);
	GOAL_PTR pGoal = (GOAL_PTR)malloc(sizeof(GOAL));
//	if ( !pGoal )
//	{
//		return NULL;
//	}

	pGoal->bFinished	  = FALSE;
	pGoal->nGoalType	  = nNewGoalType;

    //copy the data over.
    memcpy(&pGoal->goalData, aiData, sizeof(AIDATA));

	pGoal->pTasks		  = TASKQUEUE_Allocate();
	if ( !pGoal->pTasks )
	{
		return NULL;
	}
	pGoal->pNext	 = NULL;

	return pGoal;
}

GOAL_PTR GOAL_Allocate( GOALTYPE nNewGoalType, userEntity_t *pEntity )
{
//	GOAL_PTR pGoal = (GOAL_PTR)gstate->X_Malloc(sizeof(GOAL),MEM_TAG_AI);
	GOAL_PTR pGoal = (GOAL_PTR)malloc(sizeof(GOAL));
//	if ( !pGoal )
//	{
//		return NULL;
//	}

	pGoal->bFinished		= FALSE;
	pGoal->nGoalType		= nNewGoalType;
	
	memset( &pGoal->goalData, 0, sizeof(AIDATA) );
	pGoal->goalData.pEntity = pEntity;
	pGoal->pTasks			= TASKQUEUE_Allocate();
	if ( !pGoal->pTasks )
	{
		return NULL;
	}
	pGoal->pNext	 = NULL;

	return pGoal;
}

GOAL_PTR GOAL_Allocate( GOALTYPE nNewGoalType, CAction *pAction )
{
//	GOAL_PTR pGoal = (GOAL_PTR)gstate->X_Malloc(sizeof(GOAL),MEM_TAG_AI);
	GOAL_PTR pGoal = (GOAL_PTR)malloc(sizeof(GOAL));
//	if ( !pGoal )
//	{
//		return NULL;
//	}

	pGoal->bFinished		= FALSE;
	pGoal->nGoalType		= nNewGoalType;
	
	memset( &pGoal->goalData, 0, sizeof(AIDATA) );
	pGoal->goalData.pAction = pAction;
	pGoal->pTasks			= TASKQUEUE_Allocate();
	if ( !pGoal->pTasks )
	{
		return NULL;
	}
	pGoal->pNext	 = NULL;

	return pGoal;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_Allocate
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR GOAL_Allocate( GOALTYPE nNewGoalType, const int nValue )
{
//	GOAL_PTR pGoal = (GOAL_PTR)gstate->X_Malloc(sizeof(GOAL),MEM_TAG_AI);
	GOAL_PTR pGoal = (GOAL_PTR)malloc(sizeof(GOAL));
//	if ( !pGoal )
//	{
//		return NULL;
//	}

	pGoal->bFinished			  = FALSE;
	pGoal->nGoalType			  = nNewGoalType;
	
	memset( &pGoal->goalData, 0, sizeof(AIDATA) );
	pGoal->goalData.nValue	= nValue;
	pGoal->pTasks		  	= TASKQUEUE_Allocate();
	if ( !pGoal->pTasks )
	{
		return NULL;
	}
	pGoal->pNext	 = NULL;

	return pGoal;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_Allocate
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR GOAL_Allocate( GOALTYPE nNewGoalType, const CVector &position )
{
//	GOAL_PTR pGoal = (GOAL_PTR)gstate->X_Malloc(sizeof(GOAL),MEM_TAG_AI);
	GOAL_PTR pGoal = (GOAL_PTR)malloc(sizeof(GOAL));
//	if ( !pGoal )
//	{
//		return NULL;
//	}

	pGoal->bFinished		  = FALSE;
	pGoal->nGoalType		  = nNewGoalType;
	
	memset( &pGoal->goalData, 0, sizeof(AIDATA) );
	pGoal->goalData.destPoint = position;
	pGoal->pTasks			  = TASKQUEUE_Allocate();
	if ( !pGoal->pTasks )
	{
		return NULL;
	}
	pGoal->pNext	 = NULL;

	return pGoal;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_Allocate
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR GOAL_Allocate( GOALTYPE nNewGoalType, const float fValue )
{
//	GOAL_PTR pGoal = (GOAL_PTR)gstate->X_Malloc(sizeof(GOAL),MEM_TAG_AI);
	GOAL_PTR pGoal = (GOAL_PTR)malloc(sizeof(GOAL));
//	if ( !pGoal )
//	{
//		return NULL;
//	}

	pGoal->bFinished	   = FALSE;
	pGoal->nGoalType	   = nNewGoalType;
	
	memset( &pGoal->goalData, 0, sizeof(AIDATA) );
	pGoal->goalData.fValue = fValue;
	pGoal->pTasks		   = TASKQUEUE_Allocate();
	if ( !pGoal->pTasks )
	{
		return NULL;
	}
	pGoal->pNext	 = NULL;

	return pGoal;
}

GOAL_PTR GOAL_Allocate( GOALTYPE nNewGoalType, frameData_t *pNewSequence )
{
//	GOAL_PTR pGoal = (GOAL_PTR)gstate->X_Malloc(sizeof(GOAL),MEM_TAG_AI);
	GOAL_PTR pGoal = (GOAL_PTR)malloc(sizeof(GOAL));
//	if ( !pGoal )
//	{
//		return NULL;
//	}

	pGoal->bFinished			= FALSE;
	pGoal->nGoalType			= nNewGoalType;
	pGoal->goalData.pAnimSequence = pNewSequence;
	pGoal->pTasks				= TASKQUEUE_Allocate();
	if ( !pGoal->pTasks )
	{
		return NULL;
	}
	pGoal->pNext	 = NULL;

	return pGoal;
}

GOAL_PTR GOAL_Allocate( GOALTYPE nNewGoalType, char *szString )
{
//	GOAL_PTR pGoal = (GOAL_PTR)gstate->X_Malloc(sizeof(GOAL),MEM_TAG_AI);
	GOAL_PTR pGoal = (GOAL_PTR)malloc(sizeof(GOAL));
//	if ( !pGoal )
//	{
//		return NULL;
//	}

	pGoal->bFinished		= FALSE;
	pGoal->nGoalType		= nNewGoalType;
	pGoal->goalData.pString = szString;
	pGoal->pTasks			= TASKQUEUE_Allocate();
	if ( !pGoal->pTasks )
	{
		return NULL;
	}
	pGoal->pNext	 = NULL;

	return pGoal;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_Delete
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR GOAL_Delete( GOAL_PTR pGoal )
{
	TASKQUEUE_Delete( pGoal->pTasks );

	pGoal->pTasks = NULL;
//	gstate->X_Free(pGoal);
	free(pGoal);
	//delete pGoal;

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_Satisfied
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOAL_Satisfied( GOAL_PTR pGoal )
{
	pGoal->bFinished = TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_IsSatisfied
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int GOAL_IsSatisfied( GOAL_PTR pGoal )
{
	return pGoal->bFinished;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOAL_Set( GOAL_PTR pGoal, userEntity_t *pEntity )
{
	_ASSERTE( pGoal );

	memset( &pGoal->goalData, 0, sizeof(AIDATA) );
	pGoal->goalData.pEntity = pEntity;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOAL_Set( GOAL_PTR pGoal, CAction *pAction )
{
	_ASSERTE( pGoal );

	memset( &pGoal->goalData, 0, sizeof(AIDATA) );
	pGoal->goalData.pAction = pAction;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOAL_Set( GOAL_PTR pGoal, const int nValue )
{
	_ASSERTE( pGoal );

	memset( &pGoal->goalData, 0, sizeof(AIDATA) );
	pGoal->goalData.nValue = nValue;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOAL_Set( GOAL_PTR pGoal, const CVector &position )
{
	_ASSERTE( pGoal );
	
	memset( &pGoal->goalData, 0, sizeof(AIDATA) );
	pGoal->goalData.destPoint = position;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOAL_Set( GOAL_PTR pGoal, const float fValue )
{
	_ASSERTE( pGoal );

	memset( &pGoal->goalData, 0, sizeof(AIDATA) );
	pGoal->goalData.fValue = fValue;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOAL_Set( GOAL_PTR pGoal, char *szString )
{
	_ASSERTE( pGoal );

	memset( &pGoal->goalData, 0, sizeof(AIDATA) );
	pGoal->goalData.pString = szString;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOAL_Set( GOAL_PTR pGoal, GOALTYPE nNewGoalType, userEntity_t *pEntity )
{
	_ASSERTE( pGoal );

	pGoal->nGoalType		= nNewGoalType;

	memset( &pGoal->goalData, 0, sizeof(AIDATA) );
	pGoal->goalData.pEntity = pEntity;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOAL_Set( GOAL_PTR pGoal, GOALTYPE nNewGoalType, const int nValue )
{
	_ASSERTE( pGoal );

	pGoal->nGoalType			  = nNewGoalType;
	
	memset( &pGoal->goalData, 0, sizeof(AIDATA) );
	pGoal->goalData.nValue = nValue;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOAL_Set( GOAL_PTR pGoal, GOALTYPE nNewGoalType, const CVector &position )
{
	_ASSERTE( pGoal );

	pGoal->nGoalType		  = nNewGoalType;
	
	memset( &pGoal->goalData, 0, sizeof(AIDATA) );
	pGoal->goalData.destPoint = position;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOAL_Set( GOAL_PTR pGoal, GOALTYPE nNewGoalType, const float fValue )
{
	_ASSERTE( pGoal );

	pGoal->nGoalType	   = nNewGoalType;

	memset( &pGoal->goalData, 0, sizeof(AIDATA) );
	pGoal->goalData.fValue = fValue;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_Set
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOAL_Set( GOAL_PTR pGoal, GOALTYPE nNewGoalType, char *szString )
{
	_ASSERTE( pGoal );

	pGoal->nGoalType	   = nNewGoalType;

	memset( &pGoal->goalData, 0, sizeof(AIDATA) );
	pGoal->goalData.pString = szString;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_AddTaskAtFront
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOAL_AddTaskAtFront( GOAL_PTR pGoal, TASK_PTR pTask )
{
    if (pGoal == NULL) return;

	TASKQUEUE_AddAtFront( pGoal->pTasks, pTask );
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_AddTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOAL_AddTask( GOAL_PTR pGoal, TASK_PTR pTask )
{
	_ASSERTE( pGoal );

	TASKQUEUE_Add( pGoal->pTasks, pTask );
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_GetCurrentTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASK_PTR GOAL_GetCurrentTask( GOAL_PTR pGoal )
{
	return TASKQUEUE_GetCurrentTask( pGoal->pTasks );
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_RemoveCurrentTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOAL_RemoveCurrentTask( GOAL_PTR pGoal )
{
	TASKQUEUE_RemoveCurrentTask( pGoal->pTasks );
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_GetType
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOALTYPE GOAL_GetType( GOAL_PTR pGoal )
{
	return pGoal->nGoalType;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_GetTypeForString
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOALTYPE GOAL_GetTypeForString (char *str)
{
	int		i;

	for (i = 0; GOALTYPE_STR [i] != NULL; i++)
	{
		if (!stricmp (GOALTYPE_STR [i], str))
			return	(GOALTYPE) i;
	}

	return	(GOALTYPE) -1;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_GetData
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
AIDATA_PTR GOAL_GetData( GOAL_PTR pGoal )
{
	return &(pGoal->goalData);
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_GetTasks
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
TASKQUEUE_PTR GOAL_GetTasks( GOAL_PTR pGoal )
{
	// NSS[2/9/00]:Make sure this is valid first
	if(pGoal)
		return pGoal->pTasks;
	else
		return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_ClearTasks
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOAL_ClearTasks( GOAL_PTR pGoal )
{
	if ( pGoal )
	{
		TASKQUEUE_ClearTasks( pGoal->pTasks );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		GOAL_GetNumTasks
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int GOAL_GetNumTasks( GOAL_PTR pGoal )
{
	if ( pGoal )
	{
		return TASKQUEUE_GetNumTasks( pGoal->pTasks );
	}

	return 0;
}

//-----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Name:		GOALSTACK_Allocate
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOALSTACK_PTR GOALSTACK_Allocate()
{
//	GOALSTACK_PTR pGoalStack = (GOALSTACK_PTR)gstate->X_Malloc(sizeof(GOALSTACK),MEM_TAG_AI);
	GOALSTACK_PTR pGoalStack = (GOALSTACK_PTR)malloc(sizeof(GOALSTACK));
//	GOALSTACK_PTR pGoalStack = new GOALSTACK;
//	if ( !pGoalStack )
//	{
//		return NULL;
//	}

	pGoalStack->nNumGoals   = 0;
	pGoalStack->pTopOfStack = NULL;

	return pGoalStack;
}

// ----------------------------------------------------------------------------
//
// Name:		GOALSTACK_Delete
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOALSTACK_PTR GOALSTACK_Delete( GOALSTACK_PTR pGoalStack )
{
	GOAL_PTR pGoal = pGoalStack->pTopOfStack;
	while ( pGoal != NULL )
	{
		if(&pGoal->pNext)
		{
			GOAL_PTR pNextGoal = pGoal->pNext;

			GOAL_Delete( pGoal );

			pGoal = pNextGoal;
		}
	}

	pGoalStack->pTopOfStack = NULL;

//	gstate->X_Free(pGoalStack);
	free(pGoalStack);
	//delete pGoalStack;

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		GOALSTACK_ClearAllGoals
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOALSTACK_ClearAllGoals( GOALSTACK_PTR pGoalStack )
{
	GOAL_PTR pGoal = pGoalStack->pTopOfStack;
	while ( pGoal != NULL )
	{
		GOAL_PTR pNextGoal = pGoal->pNext;

		GOAL_Delete( pGoal );

		pGoal = pNextGoal;
	}

	pGoalStack->nNumGoals = 0;
	pGoalStack->pTopOfStack = NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		GOALSTACK_Add
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOALSTACK_Add( GOALSTACK_PTR pGoalStack, GOAL_PTR pGoal )
{
	_ASSERTE( pGoal );

	pGoal->pNext = pGoalStack->pTopOfStack;
	pGoalStack->pTopOfStack = pGoal;

	pGoalStack->nNumGoals++;
}

// ----------------------------------------------------------------------------
//
// Name:		GOALSTACK_AddAtBack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOALSTACK_AddAtBack( GOALSTACK_PTR pGoalStack, GOAL_PTR pGoal )
{
	_ASSERTE( pGoal );

	if ( pGoalStack->pTopOfStack )
	{
		GOAL_PTR pLastGoal = pGoalStack->pTopOfStack;
		GOAL_PTR pCurrentGoal = pGoalStack->pTopOfStack;
		while ( pCurrentGoal )
		{
			pLastGoal = pCurrentGoal;
			pCurrentGoal = pCurrentGoal->pNext;
		}
			
		pGoal->pNext = NULL;
		pLastGoal->pNext = pGoal;
		pGoalStack->nNumGoals++;
	}
	else
	{
		GOALSTACK_Add( pGoalStack, pGoal );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		GOALSTACK_GetCurrentGoal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOAL_PTR GOALSTACK_GetCurrentGoal( GOALSTACK_PTR pGoalStack )
{
	// NSS[2/9/00]:Make sure this is valid first
	if(pGoalStack)
		return pGoalStack->pTopOfStack;
	else
		return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		GOALSTACK_GetCurrentGoalType
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
GOALTYPE GOALSTACK_GetCurrentGoalType( GOALSTACK_PTR pGoalStack )
{
	if ( pGoalStack )
	{
		GOAL_PTR pGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		if ( pGoal )
		{
			return GOAL_GetType( pGoal );
		}
	}

	return GOALTYPE_NONE;
}

// ----------------------------------------------------------------------------
//
// Name:		GOALSTACK_GetCurrentTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
DllExport TASK_PTR GOALSTACK_GetCurrentTask( GOALSTACK_PTR pGoalStack )
{
//	if ( pGoalStack )
// SCG[11/16/99]: 
	if ( pGoalStack && pGoalStack->nNumGoals != 0 )
	{
		GOAL_PTR pCurrentGoal = pGoalStack->pTopOfStack;
		if ( pCurrentGoal )
		{
			return GOAL_GetCurrentTask( pCurrentGoal );
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		GOALSTACK_GetCurrentTaskType
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
DllExport TASKTYPE GOALSTACK_GetCurrentTaskType( GOALSTACK_PTR pGoalStack )
{
	if ( pGoalStack )
	{
		TASK_PTR pTask = GOALSTACK_GetCurrentTask( pGoalStack );
		if ( pTask )
		{
			return TASK_GetType( pTask );
		}
	}

	return TASKTYPE_IDLE;
}

// ----------------------------------------------------------------------------
//
// Name:		GOALSTACK_RemoveCurrentGoal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOALSTACK_RemoveCurrentGoal( GOALSTACK_PTR pGoalStack )
{
	if ( pGoalStack && pGoalStack->pTopOfStack )
	{
		GOAL_PTR pNextGoal = pGoalStack->pTopOfStack->pNext;
		pGoalStack->pTopOfStack = GOAL_Delete( pGoalStack->pTopOfStack );
		if(pNextGoal)
		{
			pGoalStack->pTopOfStack = pNextGoal;
		}
		pGoalStack->nNumGoals--;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		GOALSTACK_RemoveCurrentTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOALSTACK_RemoveCurrentTask( GOALSTACK_PTR pGoalStack )
{
	GOAL_PTR pCurrentGoal = pGoalStack->pTopOfStack;
	// NSS[2/9/00]:Make sure this is valid first
	if(pCurrentGoal)
	{
		GOAL_RemoveCurrentTask( pCurrentGoal );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		GOALSTACK_GetNumGoals
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int GOALSTACK_GetNumGoals( GOALSTACK_PTR pGoalStack )
{
	// NSS[2/9/00]:Make sure this is valid first
	if(pGoalStack)
		return pGoalStack->nNumGoals;
	else
		return 0;
}

// ---------------------------------------------------------------------------- 
//
// Name:		GOALSTACK_RemoveAllGoalsOfType
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOALSTACK_RemoveAllGoalsOfType( GOALSTACK_PTR pGoalStack, int nGoalType )
{
	if ( pGoalStack && pGoalStack->pTopOfStack )
	{
		// clear the tasks for the current goal before adding a new goal
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		GOAL_PTR pPrevGoal = NULL;
		while ( pCurrentGoal )
		{
			if ( pCurrentGoal->nGoalType == nGoalType )
			{
				// remove this
				GOAL_PTR pTempGoal = pCurrentGoal;

                if ( pPrevGoal )
                {
				    pPrevGoal->pNext = pCurrentGoal->pNext;
                }

				pCurrentGoal = pCurrentGoal->pNext;

				GOAL_Delete( pTempGoal );
				pGoalStack->nNumGoals--;
			}
			else
			{
				pPrevGoal = pCurrentGoal;
				pCurrentGoal = pCurrentGoal->pNext;
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		GOALSTACK_RemoveAllGoalsOfTypeExceptCurrentGoal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void GOALSTACK_RemoveAllGoalsOfTypeExceptCurrentGoal( GOALSTACK_PTR pGoalStack, int nGoalType )
{
	if ( pGoalStack && pGoalStack->pTopOfStack )
	{
		// clear the tasks for the current goal before adding a new goal
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		GOAL_PTR pPrevGoal = pCurrentGoal;
		pCurrentGoal = pCurrentGoal->pNext;
		while ( pCurrentGoal )
		{
			if ( pCurrentGoal->nGoalType == nGoalType )
			{
				// remove this
				GOAL_PTR pTempGoal = pCurrentGoal;
				pPrevGoal->pNext = pCurrentGoal->pNext;
				pCurrentGoal = pCurrentGoal->pNext;

				GOAL_Delete( pTempGoal );
				pGoalStack->nNumGoals--;
			}
			else
			{
				pPrevGoal = pCurrentGoal;
				pCurrentGoal = pCurrentGoal->pNext;
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
