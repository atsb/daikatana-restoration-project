#ifndef	AI_H
#define	AI_H

#include "action.h"

//-----------------------------------------------------------------------------

TASK_PTR TASK_Allocate();
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType );
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, AIDATA_PTR aiData) ;
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, userEntity_t *pEntity );
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, CAction *pAction );
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, const int nValue );
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, const CVector &position );
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, const float fValue );
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, frameData_t *pNewSequence );
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, char *szString );
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, void *pVoid );
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, char *szString, const float fNewValue );
TASK_PTR TASK_Allocate( TASKTYPE nNewTaskType, char *szString, const float fValue1, const float fValue2 );
TASK_PTR TASK_Delete( TASK_PTR pTask );
void TASK_Set( TASK_PTR pTask, userEntity_t *pEntity );
void TASK_Set( TASK_PTR pTask, CAction *pAction );
void TASK_Set( TASK_PTR pTask, const int nValue );
void TASK_Set( TASK_PTR pTask, const CVector &position );
void TASK_Set( TASK_PTR pTask, const float fValue );
void TASK_Set( TASK_PTR pTask, void *pVoid );
void TASK_Set( TASK_PTR pTask, TASKTYPE nNewTaskType, userEntity_t *pEntity );
void TASK_Set( TASK_PTR pTask, TASKTYPE nNewTaskType, const int nValue );
void TASK_Set( TASK_PTR pTask, TASKTYPE nNewTaskType, const CVector &position );
void TASK_Set( TASK_PTR pTask, TASKTYPE nNewTaskType, const float fValue );
void TASK_Set( TASK_PTR pTask, TASKTYPE nNewTaskType, char *szString );
TASKTYPE TASK_GetType( TASK_PTR pTask );
AIDATA_PTR TASK_GetData( TASK_PTR pTask );

//-----------------------------------------------------------------------------

TASKQUEUE_PTR TASKQUEUE_Allocate();
TASKQUEUE_PTR TASKQUEUE_Delete( TASKQUEUE_PTR pTaskQueue );
void TASKQUEUE_AddAtFront( TASKQUEUE_PTR pTaskQueue, TASK_PTR pTask );
void TASKQUEUE_Add( TASKQUEUE_PTR pTaskQueue, TASK_PTR pTask );
TASK_PTR TASKQUEUE_GetCurrentTask( TASKQUEUE_PTR pTaskQueue );
void TASKQUEUE_RemoveCurrentTask( TASKQUEUE_PTR	pTaskQueue );
int TASKQUEUE_GetNumTasks( TASKQUEUE_PTR pTaskQueue );
void TASKQUEUE_ClearTasks( TASKQUEUE_PTR pTaskQueue );

//-----------------------------------------------------------------------------

GOAL_PTR GOAL_Allocate();
GOAL_PTR GOAL_Allocate( GOALTYPE nNewGoalType );
GOAL_PTR GOAL_Allocate( GOALTYPE nNewGoalType, AIDATA_PTR aiData );
GOAL_PTR GOAL_Allocate( GOALTYPE nNewGoalType, userEntity_t *pEntity );
GOAL_PTR GOAL_Allocate( GOALTYPE nNewGoalType, CAction *pAction );
GOAL_PTR GOAL_Allocate( GOALTYPE nNewGoalType, const int nValue );
GOAL_PTR GOAL_Allocate( GOALTYPE nNewGoalType, const CVector &position );
GOAL_PTR GOAL_Allocate( GOALTYPE nNewGoalType, const float fValue );
GOAL_PTR GOAL_Allocate( GOALTYPE nNewGoalType, frameData_t *pNewSequence );
GOAL_PTR GOAL_Allocate( GOALTYPE nNewGoalType, char *szString );
GOAL_PTR GOAL_Delete( GOAL_PTR pGoal );
void GOAL_Satisfied( GOAL_PTR pGoal );
int GOAL_IsSatisfied( GOAL_PTR pGoal );
void GOAL_Set( GOAL_PTR pGoal, userEntity_t *pEntity );
void GOAL_Set( GOAL_PTR pGoal, CAction *pAction );
void GOAL_Set( GOAL_PTR pGoal, const int nValue );
void GOAL_Set( GOAL_PTR pGoal, const CVector &position );
void GOAL_Set( GOAL_PTR pGoal, const float fValue );
void GOAL_Set( GOAL_PTR pGoal, char *szString );
void GOAL_Set( GOAL_PTR pGoal, GOALTYPE nNewGoalType, userEntity_t *pEntity );
void GOAL_Set( GOAL_PTR pGoal, GOALTYPE nNewGoalType, const int nValue );
void GOAL_Set( GOAL_PTR pGoal, GOALTYPE nNewGoalType, const CVector &position );
void GOAL_Set( GOAL_PTR pGoal, GOALTYPE nNewGoalType, const float fValue );
void GOAL_Set( GOAL_PTR pGoal, GOALTYPE nNewGoalType, char *szString );
void GOAL_AddTaskAtFront( GOAL_PTR pGoal, TASK_PTR pTask );
void GOAL_AddTask( GOAL_PTR pGoal, TASK_PTR pTask );
TASK_PTR GOAL_GetCurrentTask( GOAL_PTR pGoal );
void GOAL_RemoveCurrentTask( GOAL_PTR pGoal );
GOALTYPE GOAL_GetType( GOAL_PTR pGoal );
GOALTYPE GOAL_GetTypeForString (char *str);
AIDATA_PTR GOAL_GetData( GOAL_PTR pGoal );
TASKQUEUE_PTR GOAL_GetTasks( GOAL_PTR pGoal );
void GOAL_ClearTasks( GOAL_PTR pGoal );
int GOAL_GetNumTasks( GOAL_PTR pGoal );

//-----------------------------------------------------------------------------

GOALSTACK_PTR GOALSTACK_Allocate();
GOALSTACK_PTR GOALSTACK_Delete( GOALSTACK_PTR pGoalStack );
void GOALSTACK_ClearAllGoals( GOALSTACK_PTR pGoalStack );
void GOALSTACK_Add( GOALSTACK_PTR pGoalStack, GOAL_PTR pGoal );
void GOALSTACK_AddAtBack( GOALSTACK_PTR pGoalStack, GOAL_PTR pGoal );
GOAL_PTR GOALSTACK_GetCurrentGoal( GOALSTACK_PTR pGoalStack );
GOALTYPE GOALSTACK_GetCurrentGoalType( GOALSTACK_PTR pGoalStack );
DllExport TASK_PTR GOALSTACK_GetCurrentTask( GOALSTACK_PTR pGoalStack );
DllExport TASKTYPE GOALSTACK_GetCurrentTaskType( GOALSTACK_PTR pGoalStack );
void GOALSTACK_RemoveCurrentGoal( GOALSTACK_PTR pGoalStack );
void GOALSTACK_RemoveCurrentTask( GOALSTACK_PTR pGoalStack );
int GOALSTACK_GetNumGoals( GOALSTACK_PTR pGoalStack );
void GOALSTACK_RemoveAllGoalsOfType( GOALSTACK_PTR pGoalStack, int nGoalType );
void GOALSTACK_RemoveAllGoalsOfTypeExceptCurrentGoal( GOALSTACK_PTR pGoalStack, int nGoalType );



#if 0

#else _DEBUG

#include "ai_common.h"

#endif 0

//-----------------------------------------------------------------------------

void hiro_SpawnScriptActor(userEntity_t *self, const CVector &origin, const CVector &angles);
void hiro_SpawnScriptDummy(const CVector &origin, const CVector &angles);

void hiro_RemoveScriptActor(userEntity_t *actor);
void hiro_RemoveScriptDummy(userEntity_t *actor);


#endif