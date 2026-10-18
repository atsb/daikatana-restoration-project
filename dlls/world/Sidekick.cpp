// ==========================================================================
//
//  File:       Sidekick.cpp
//  Contents:
//  Author:
//
// ==========================================================================

#if _MSC_VER
#include <crtdbg.h>
#endif
#include "world.h"
#include "ai_move.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "actorlist.h"
#include "ai_func.h"
#include "MonsterSound.h"
#include "ai.h"
#include "spawn.h"
#include "Sidekick.h"
#include "ai_info.h"
#include "SequenceMap.h"
#include "ai_utils.h"
#include "items.h"
#include "ai_weapons.h"
#include "weapondefs.h"
#include "ai_think.h"
#include "csv.h"
#include "dk_io.h"
#include "chasecam.h"
#include "client.h"
#include "dk_system.h"
#include "dk_neural_ai.h"
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

/* Original secret-door interaction supplied by DOOR.CPP. */
extern void secret_use(userEntity_t* self, userEntity_t* other, userEntity_t* activator);
extern userEntity_t* AI_GetSwitchOnTrain(userEntity_t* self);
extern userEntity_t* AI_GetClosestSwitch(userEntity_t* pTrain, CVector& currentPosition);

/* ***************************** define types ****************************** */

//#define USE_STRAFE
#define DO_AMBIENTS

#define HIRO_CLASSNAME		"Hiro"
#define MIKIKO_CLASSNAME	"Mikiko"
#define SUPERFLY_CLASSNAME	"Superfly"
#define MIKIKOFLY_CLASSNAME	"MikikoFly"

#define SUPERFLY_CARRYING_MIKIKO_MODEL  "models/global/m_mikiko.dkm"

#define SIDEKICK_MELEE_ATTACK_DISTANCE	128.0f
#define SIDEKICK_WIDTH_FACTOR   0.75f
#define PROJECTILE_EFFECTIVE_DISTANCE   512.0f

#ifdef _DEBUG
void AI_Drop_Markers(CVector& Target, float delay);
#endif
/* ***************************** Local Variables *************************** */

// NSS[2/24/00]:Wait timers for commands.
float Superflywait, MikikoWait;
const static float fAngleAwayFromOwner = 75.0f;
const static float fNormalDistanceFromOwner = 96.0f;
const static float fCloseDistanceToOwner = 65.0f;
const static float fOneHalfFrameDistance = 60.0f;

static float SuperFlyWaitIdle;				// NSS[3/4/00]:Variable used to determine how long the sidkick has been waiting in one place.
static float MikikoWaitIdle;

static float SuperflyLastTalk;
static float MikikoLastTalk;

static DEATHTYPE sidekick_deathtype = NONE;
static int bInGodMode = FALSE;


// ---------------------------------------------------------------------------
// Smart sidekick navigation / combat memory.
// Keep this state outside playerHook_t so savegame/network ABI remains intact.
// ---------------------------------------------------------------------------
#define SIDEKICK_SMART_TRAIL_SIZE            24
#define SIDEKICK_SMART_FAILED_SIZE            8
#define SIDEKICK_SMART_TRAIL_INTERVAL      0.20f
#define SIDEKICK_SMART_FAILED_TIME         10.0f
#define SIDEKICK_SMART_RECOVER_COOLDOWN     0.75f
#define SIDEKICK_SMART_MAX_TRAIL_TESTS       10
#define SIDEKICK_SMART_MAX_NODE_TESTS        12
#define SIDEKICK_SMART_MAX_BREADCRUMB_TESTS  18
#define SIDEKICK_SMART_BREADCRUMB_DISTANCE   512.0f
#define SIDEKICK_SMART_BREADCRUMB_MIN_DIST    96.0f
#define SIDEKICK_SMART_STUCK_TIME             0.45f
#define SIDEKICK_SMART_STUCK_DISTANCE         2.0f
#define SIDEKICK_SMART_JUMP_RETRY_TIME        1.50f
#define SIDEKICK_TIGHT_FOLLOW_DISTANCE       56.0f
#define SIDEKICK_TIGHT_WALK_FOLLOW_DISTANCE  64.0f
#define SIDEKICK_TIGHT_RUN_FOLLOW_DISTANCE  176.0f
#define SIDEKICK_SMART_MAX_SAFE_DROP        256.0f
#define SIDEKICK_SMART_DROP_LOOKAHEAD       256.0f
#define SIDEKICK_SMART_LEDGE_LOOKAHEAD       160.0f
#define SIDEKICK_SMART_LEDGE_SAMPLE_STEP      32.0f
#define SIDEKICK_SMART_MIN_LEDGE_HEIGHT       20.0f
#define SIDEKICK_SMART_LEDGE_JUMP_FORWARD_DELAY 0.08f
#define SIDEKICK_SMART_OWNER_LADDER_RADIUS       80.0f
#define SIDEKICK_SMART_OWNER_LADDER_Z_RANGE     768.0f

#define SIDEKICK_ADVANCED_MAX_ROUTE_ACTIONS   24
#define SIDEKICK_ADVANCED_ROUTE_REJOIN         1
#define SIDEKICK_ADVANCED_ROUTE_EXPLORE        2
#define SIDEKICK_ADVANCED_REPLAN_INTERVAL      1.0f
#define SIDEKICK_ADVANCED_REJOIN_TRIGGER      512.0f
#define SIDEKICK_ADVANCED_REJOIN_MAX_DISTANCE 1536.0f
#define SIDEKICK_ADVANCED_REJOIN_DONE_DISTANCE 192.0f

typedef struct
{
	int nodeIndex;
	int nodeType;
	int actionKind;
	userEntity_t* controller;
	userEntity_t* target;
	CVector nodePoint;
	CVector controllerPoint;
	CVector targetPoint;
} sidekick_advanced_route_action_t;

typedef struct
{
	userEntity_t* self;
	CVector ownerTrail[SIDEKICK_SMART_TRAIL_SIZE];
	float ownerTrailTime[SIDEKICK_SMART_TRAIL_SIZE];
	int ownerTrailCount;
	int ownerTrailHead;
	CVector failedPoints[SIDEKICK_SMART_FAILED_SIZE];
	float failedPointUntil[SIDEKICK_SMART_FAILED_SIZE];
	int failedPointCount;
	int failedPointHead;
	float nextTrailTime;
	float recoveryCooldown;
	int recoveryAttempts;
	CVector recoveryPoint;
	qboolean recoveryPointActive;
	CVector jumpTarget;
	CVector jumpStart;
	float jumpStartTime;
	qboolean jumpActive;
	float nextNeuralDecisionTime;
	int lastNeuralAction;
	int lastNeuralTarget;
	char lastNeuralActionName[32];
	CVector lastOrigin;
	float lastOriginTime;
	float stuckSince;
	float jumpBlockedUntil;
	qboolean ledgeJumpForwardPending;
	CVector ledgeJumpDirection;
	float ledgeJumpForwardVelocity;
	float ledgeJumpForwardAt;
	/* Neural/Advanced runtime memory. Kept outside playerHook_t to preserve ABI. */
	qboolean neuralEnhancementsSuspended;
	float lastOwnerSampleTime;
	CVector lastOwnerOrigin;
	float lastEnemySeenTime;
	int lastEnemyNumber;
	CVector lastEnemyOrigin;
	float lastInteractionTime;
	int lastInteractionNumber;
	/* Advanced Reasoning only: global-level exploration state. */
	qboolean advancedExploreActive;
	int advancedExploreTargetNumber;
	CVector advancedExploreTargetPoint;
	float advancedExploreCooldown;
	float advancedExploreStartTime;
	int advancedExploreSecretCount;
	/* Advanced route planning: deterministic execution path for rejoin/exploration. */
	qboolean advancedRouteActive;
	int advancedRouteMode;
	CVector advancedRouteGoalPoint;
	float advancedRouteStartTime;
	float advancedRouteNextReplanTime;
	int advancedRouteActionCount;
	sidekick_advanced_route_action_t advancedRouteActions[SIDEKICK_ADVANCED_MAX_ROUTE_ACTIONS];
	int advancedRouteNextAction;
} sidekick_smart_state_t;

static sidekick_smart_state_t sidekickSmartStates[4];

/* Neural script/cinematic gate is defined later, but all smart helpers may be
   invoked from original task execution and must be able to see the gate. */
static qboolean SIDEKICK_NeuralScriptEventOwnsAI(userEntity_t* self);
static qboolean SIDEKICK_NeuralHasScriptCriticalTask(userEntity_t* self);

static sidekick_smart_state_t* SIDEKICK_SmartGetState(userEntity_t* self)
{
	int i;
	int freeSlot = -1;

	for (i = 0; i < 4; i++)
	{
		if (sidekickSmartStates[i].self == self)
			return &sidekickSmartStates[i];

		if (sidekickSmartStates[i].self == NULL && freeSlot == -1)
			freeSlot = i;
	}

	if (freeSlot == -1)
		freeSlot = 0;

	memset(&sidekickSmartStates[freeSlot], 0, sizeof(sidekickSmartStates[freeSlot]));
	sidekickSmartStates[freeSlot].self = self;
	sidekickSmartStates[freeSlot].lastEnemyNumber = -1;
	sidekickSmartStates[freeSlot].lastInteractionNumber = -1;
	sidekickSmartStates[freeSlot].advancedExploreTargetNumber = -1;
	return &sidekickSmartStates[freeSlot];
}

static void SIDEKICK_SmartQueueLookAround(userEntity_t* self)
{
	if (!self)
		return;
	if (SIDEKICK_NeuralScriptEventOwnsAI(self))
		return;

	if (!SIDEKICK_HasTaskInQue(self, TASKTYPE_SIDEKICK_LOOKAROUND))
		AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_LOOKAROUND);
}

static qboolean SIDEKICK_SmartFindSafeDropPoint(
	userEntity_t* self,
	const CVector& destination,
	CVector& outPoint)
{
	playerHook_t* hook;
	CVector direction;
	float targetDistance;
	int sampleIndex;

	if (!self)
		return FALSE;

	hook = AI_GetPlayerHook(self);
	if (!hook || !AI_IsSidekick(hook) || !self->groundEntity)
		return FALSE;

	if (destination.z > self->s.origin.z - 32.0f)
		return FALSE;

	direction = destination - self->s.origin;
	direction.z = 0.0f;
	targetDistance = direction.Length();
	if (targetDistance < 16.0f)
		return FALSE;

	direction.Normalize();

	for (sampleIndex = 0; sampleIndex < 8; ++sampleIndex)
	{
		float forwardDistance = 24.0f + (sampleIndex * 32.0f);
		CVector edgePoint;
		CVector movementPoint;
		CVector traceStart;
		CVector traceEnd;
		CVector landingPoint;
		trace_t horizontalTrace;
		trace_t downTrace;
		int contents;
		float dropDistance;

		if (forwardDistance > SIDEKICK_SMART_DROP_LOOKAHEAD ||
			forwardDistance > targetDistance + 32.0f)
		{
			break;
		}

		edgePoint = self->s.origin + (direction * forwardDistance);
		edgePoint.z = self->s.origin.z;

		horizontalTrace = gstate->TraceBox_q2(
			self->s.origin,
			self->s.mins,
			self->s.maxs,
			edgePoint,
			self,
			MASK_SOLID);

		if (horizontalTrace.startsolid ||
			horizontalTrace.allsolid ||
			horizontalTrace.fraction < 0.99f)
		{
			continue;
		}

		traceStart = edgePoint;
		traceStart.z += 24.0f;
		traceEnd = edgePoint;
		traceEnd.z -= SIDEKICK_SMART_MAX_SAFE_DROP;

		downTrace = gstate->TraceLine_q2(traceStart, traceEnd, self, MASK_SOLID);

		if (downTrace.startsolid ||
			downTrace.allsolid ||
			downTrace.fraction >= 1.0f ||
			downTrace.plane.normal.z < 0.7f)
		{
			continue;
		}

		landingPoint = downTrace.endpos;
		dropDistance = self->s.origin.z - landingPoint.z;
		if (dropDistance < 32.0f || dropDistance > SIDEKICK_SMART_MAX_SAFE_DROP)
			continue;

		contents = gstate->PointContents(landingPoint);
		if (contents & (CONTENTS_LAVA | CONTENTS_SLIME))
			continue;

		movementPoint = edgePoint + (direction * 24.0f);
		movementPoint.z = self->s.origin.z;
		horizontalTrace = gstate->TraceBox_q2(
			self->s.origin,
			self->s.mins,
			self->s.maxs,
			movementPoint,
			self,
			MASK_SOLID);

		if (horizontalTrace.startsolid ||
			horizontalTrace.allsolid ||
			horizontalTrace.fraction < 0.99f)
		{
			continue;
		}

		outPoint = movementPoint;
		return TRUE;
	}

	return FALSE;
}


void SIDEKICK_SmartJumpStarted(userEntity_t* self, const CVector& target);

static MAPNODE_PTR SIDEKICK_SmartFindOwnerLadderNode(userEntity_t* self)
{
	playerHook_t* hook;
	userEntity_t* owner;
	MAPNODE_PTR bestNode = NULL;
	float bestScore = 99999999.0f;
	int i;

	if (!self || !pGroundNodes || !pGroundNodes->pNodes)
		return NULL;

	hook = AI_GetPlayerHook(self);
	if (!hook || !AI_IsSidekick(hook) || !hook->owner)
		return NULL;

	owner = hook->owner;
	/* Only force a ladder route when the player is actually above us. */
	if (owner->s.origin.z <= self->s.origin.z + 48.0f)
		return NULL;

	for (i = 0; i < pGroundNodes->nNumNodes; ++i)
	{
		MAPNODE_PTR node = &pGroundNodes->pNodes[i];
		float xyDistance;
		float zDistance;
		float score;

		if (!(node->node_type & NODETYPE_LADDER))
			continue;

		xyDistance = VectorXYDistance(node->position, owner->s.origin);
		zDistance = (float)fabs(node->position.z - owner->s.origin.z);
		/* A player climbing a long ladder can be far above the sidekick.
		 * XY alignment is the strong signal while the owner is off the ground. */
		if (xyDistance > SIDEKICK_SMART_OWNER_LADDER_RADIUS)
			continue;
		if (owner->groundEntity && zDistance > SIDEKICK_SMART_OWNER_LADDER_Z_RANGE)
			continue;

		score = xyDistance + (zDistance * 0.10f);
		if (score < bestScore)
		{
			bestScore = score;
			bestNode = node;
		}
	}

	return bestNode;
}

static qboolean SIDEKICK_SmartFollowOwnerLadder(userEntity_t* self)
{
	playerHook_t* hook;
	GOALSTACK_PTR goalStack;
	TASK_PTR currentTask;
	MAPNODE_PTR ownerLadderNode;
	MAPNODE_PTR targetLadderNode;
	PATHNODE_PTR pathNode;
	int foundLadderInPath = FALSE;
	int pathChecks = 0;

	if (!self)
		return FALSE;
	if (SIDEKICK_NeuralScriptEventOwnsAI(self) ||
		SIDEKICK_NeuralHasScriptCriticalTask(self))
		return FALSE;

	hook = AI_GetPlayerHook(self);
	if (!hook || !AI_IsSidekick(hook) || !hook->owner ||
		!AI_IsAlive(hook->owner) || !(hook->dflags & DFL_CANUSELADDER))
		return FALSE;

	ownerLadderNode = SIDEKICK_SmartFindOwnerLadderNode(self);
	if (!ownerLadderNode)
		return FALSE;

	/* Never rebuild a route that already knows it must traverse a ladder. */
	if (hook->pPathList && hook->pPathList->pPath)
	{
		pathNode = hook->pPathList->pPath;
		while (pathNode && pathChecks < 16)
		{
			MAPNODE_PTR pathMapNode = NODE_GetNode(pGroundNodes, pathNode->nNodeIndex);
			if (pathMapNode && (pathMapNode->node_type & NODETYPE_LADDER))
			{
				foundLadderInPath = TRUE;
				break;
			}
			pathNode = pathNode->next_node;
			++pathChecks;
		}

		if (foundLadderInPath)
		{
			/* Let the original AI create the exact ladder task when the sidekick
			 * reaches the transition node.  If it is not at a ladder transition
			 * yet, normal path movement must continue toward it. */
			return AI_HandleUse(self) ? TRUE : FALSE;
		}
	}

	/* Target the ladder node adjacent to the player, not an intermediate rung.
	 * The normal path then contains the complete ladder transition and the
	 * original AI climbs the ladder until it reaches the player's level. */
	targetLadderNode = ownerLadderNode;
	if (!AI_FindPathToNode(self, targetLadderNode))
		return FALSE;

	goalStack = AI_GetCurrentGoalStack(hook);
	currentTask = goalStack ? GOALSTACK_GetCurrentTask(goalStack) : NULL;
	if (currentTask && TASK_GetType(currentTask) == TASKTYPE_MOVETOLOCATION)
		AI_RemoveCurrentTask(self);

	AI_AddNewTaskAtFront(self, TASKTYPE_MOVETOLOCATION, targetLadderNode->position);
	return TRUE;
}

static qboolean SIDEKICK_SmartHandleLadder(userEntity_t* self)
{
	playerHook_t* hook;
	GOALSTACK_PTR goalStack;
	TASK_PTR currentTask;

	if (!self)
		return FALSE;

	hook = AI_GetPlayerHook(self);
	if (!hook || !(hook->dflags & DFL_CANUSELADDER))
		return FALSE;

	if (SIDEKICK_SmartFollowOwnerLadder(self))
		return TRUE;

	if (!hook->pPathList || !hook->pPathList->pPath)
		return FALSE;

	goalStack = AI_GetCurrentGoalStack(hook);
	currentTask = goalStack ? GOALSTACK_GetCurrentTask(goalStack) : NULL;
	if (SIDEKICK_NeuralScriptEventOwnsAI(self))
	{
		/* A script may intentionally be using a ladder. Do not invent a new
		   ladder operation; let the original task system own it. */
		if (!currentTask)
			return FALSE;
		return TASK_GetType(currentTask) == TASKTYPE_UPLADDER ||
			TASK_GetType(currentTask) == TASKTYPE_DOWNLADDER ||
			TASK_GetType(currentTask) == TASKTYPE_WAITONLADDERFORNOCOLLISION;
	}
	if (currentTask)
	{
		TASKTYPE taskType = TASK_GetType(currentTask);
		if (taskType == TASKTYPE_UPLADDER ||
			taskType == TASKTYPE_DOWNLADDER ||
			taskType == TASKTYPE_WAITONLADDERFORNOCOLLISION)
		{
			return TRUE;
		}
	}

	/*
	 * The original AI already knows how to use ladder nodes, including
	 * alignment, sidekick ladder arbitration, collision disabling and the
	 * ladder animations.  The smart movement layer can otherwise bypass
	 * AI_HandleUse() when it decides to move directly toward a destination.
	 * Give the original ladder system first refusal whenever a path exists.
	 */
	if (AI_HandleUse(self))
		return TRUE;

	goalStack = AI_GetCurrentGoalStack(hook);
	currentTask = goalStack ? GOALSTACK_GetCurrentTask(goalStack) : NULL;
	if (!currentTask)
		return FALSE;

	return TASK_GetType(currentTask) == TASKTYPE_UPLADDER ||
		TASK_GetType(currentTask) == TASKTYPE_DOWNLADDER ||
		TASK_GetType(currentTask) == TASKTYPE_WAITONLADDERFORNOCOLLISION;
}

static qboolean SIDEKICK_SmartFindJumpLedge(
	userEntity_t* self,
	const CVector& destination,
	CVector& outPoint)
{
	playerHook_t* hook;
	CVector direction;
	float jumpDistance;
	float rise;
	int ledgeIndex;

	if (!self || !self->groundEntity)
		return FALSE;

	hook = AI_GetPlayerHook(self);
	if (!hook || !AI_IsSidekick(hook))
		return FALSE;

	/* Use the original Daikatana ledge detector.  It was specifically built
	 * for the low-obstruction case that a sidekick can jump but cannot walk. */
	direction = destination - self->s.origin;
	direction.z = 0.0f;
	if (direction.Length() < 32.0f)
		return FALSE;
	direction.Normalize();
	forward = direction;

	if (!ai_find_ledge_list(self, 32.0f))
		return FALSE;

	{
		CVector ledgeDestination = destination;
		ledgeIndex = ai_ChooseLedge(self, ledgeDestination, 32.0f);
	}
	if (!ledgeIndex)
		return FALSE;

	if (ai_jump_obstructed(self, (float)ledge_data.ledge_xy_dist,
		hook->upward_vel))
		return FALSE;

	jumpDistance = ledge_data.ledge_xy_dist;
	rise = ledge_data.ledge_z_dist;
	if (jumpDistance < 24.0f || jumpDistance > hook->max_jump_dist + 16.0f)
		return FALSE;
	if (rise < SIDEKICK_SMART_MIN_LEDGE_HEIGHT ||
		rise > ai_max_height(self, hook->upward_vel) + 16.0f)
		return FALSE;

	outPoint = self->s.origin + (direction * jumpDistance);
	outPoint.z = self->s.origin.z + rise;
	return TRUE;
}

static qboolean SIDEKICK_SmartJumpToLedge(
	userEntity_t* self,
	const CVector& destination)
{
	playerHook_t* hook;
	sidekick_smart_state_t* state;
	CVector landingPoint;
	CVector direction;
	float jumpDistance;
	float rise;
	float forwardVelocity;

	if (SIDEKICK_NeuralScriptEventOwnsAI(self))
		return FALSE;
	if (!SIDEKICK_SmartFindJumpLedge(self, destination, landingPoint))
		return FALSE;

	hook = AI_GetPlayerHook(self);
	if (!hook)
		return FALSE;

	direction = landingPoint - self->s.origin;
	direction.z = 0.0f;
	jumpDistance = direction.Length();
	if (jumpDistance < 1.0f)
		return FALSE;
	direction.Normalize();
	rise = landingPoint.z - self->s.origin.z;
	forwardVelocity = ai_jump_vel(self, jumpDistance, rise, hook->upward_vel);
	if (forwardVelocity <= 0.0f)
		return FALSE;

	{
		char szAnimation[16];
		AI_SelectJumpingAnimation(self, szAnimation);
		AI_ForceSequence(self, szAnimation, FRAME_ONCE);
	}

	/* Clear the lip vertically first, then add horizontal velocity shortly
	 * after launch.  This is the human "jump, then press forward" motion. */
	self->velocity.x = 0.0f;
	self->velocity.y = 0.0f;
	self->velocity.z = hook->upward_vel;
	self->groundEntity = NULL;

	if (hook->pPathList)
		PATHLIST_KillPath(hook->pPathList);

	SIDEKICK_SmartJumpStarted(self, landingPoint);
	state = SIDEKICK_SmartGetState(self);
	state->ledgeJumpForwardPending = TRUE;
	state->ledgeJumpDirection = direction;
	state->ledgeJumpForwardVelocity = forwardVelocity;
	state->ledgeJumpForwardAt = gstate->time + SIDEKICK_SMART_LEDGE_JUMP_FORWARD_DELAY;
	state->jumpTarget = landingPoint;
	state->jumpActive = TRUE;
	state->jumpStartTime = gstate->time;
	return TRUE;
}

int SIDEKICK_GetTightFollowPoint(userEntity_t* self, userEntity_t* target, CVector& outPoint)
{
	CVector direction;
	float length;

	if (!self || !target)
		return FALSE;

	if (target->s.origin.z < self->s.origin.z - 32.0f &&
		SIDEKICK_SmartFindSafeDropPoint(self, target->s.origin, outPoint))
	{
		return 2;
	}

	direction = target->s.angles;
	direction.AngleToForwardVector(direction);
	direction.z = 0.0f;
	length = direction.Length();

	if (length < 1.0f)
	{
		direction = target->velocity;
		direction.z = 0.0f;
		length = direction.Length();
	}

	if (length < 1.0f)
	{
		direction = self->s.origin - target->s.origin;
		direction.z = 0.0f;
		length = direction.Length();
	}

	if (length < 1.0f)
		return FALSE;

	direction.Normalize();
	outPoint = target->s.origin - (direction * SIDEKICK_TIGHT_FOLLOW_DISTANCE);
	outPoint.z = target->s.origin.z;
	return 1;
}

static void SIDEKICK_SmartResetState(userEntity_t* self)
{
	int i;

	for (i = 0; i < 4; i++)
	{
		if (sidekickSmartStates[i].self == self)
		{
			memset(&sidekickSmartStates[i], 0, sizeof(sidekickSmartStates[i]));
			return;
		}
	}
}

static void SIDEKICK_SmartRememberOwner(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	userEntity_t* owner;
	sidekick_smart_state_t* state;
	int index;
	float moved;

	if (!hook || !AI_IsSidekick(hook) || !hook->owner || !AI_IsAlive(hook->owner))
		return;

	owner = hook->owner;
	state = SIDEKICK_SmartGetState(self);

	if (gstate->time < state->nextTrailTime)
		return;

	state->nextTrailTime = gstate->time + SIDEKICK_SMART_TRAIL_INTERVAL;

	if (state->ownerTrailCount > 0)
	{
		index = (state->ownerTrailHead - 1 + SIDEKICK_SMART_TRAIL_SIZE) %
			SIDEKICK_SMART_TRAIL_SIZE;
		moved = VectorXYDistance(state->ownerTrail[index], owner->s.origin);

		if (moved < 20.0f &&
			fabs(owner->s.origin.z - state->ownerTrail[index].z) < 24.0f)
		{
			return;
		}

		if (moved > 768.0f ||
			fabs(owner->s.origin.z - state->ownerTrail[index].z) > 384.0f)
		{
			state->ownerTrailCount = 0;
			state->ownerTrailHead = 0;
		}
	}

	state->ownerTrail[state->ownerTrailHead] = owner->s.origin;
	state->ownerTrailTime[state->ownerTrailHead] = gstate->time;
	state->ownerTrailHead =
		(state->ownerTrailHead + 1) % SIDEKICK_SMART_TRAIL_SIZE;

	if (state->ownerTrailCount < SIDEKICK_SMART_TRAIL_SIZE)
		state->ownerTrailCount++;
}

static qboolean SIDEKICK_SmartNearFailedPoint(userEntity_t* self, const CVector& point)
{
	sidekick_smart_state_t* state = SIDEKICK_SmartGetState(self);
	int i;

	for (i = 0; i < state->failedPointCount; i++)
	{
		if (state->failedPointUntil[i] > gstate->time &&
			VectorXYDistance(state->failedPoints[i], point) < 72.0f &&
			fabs(state->failedPoints[i].z - point.z) < 56.0f)
		{
			return TRUE;
		}
	}

	return FALSE;
}

static void SIDEKICK_SmartRememberFailedPoint(userEntity_t* self, const CVector& point)
{
	sidekick_smart_state_t* state = SIDEKICK_SmartGetState(self);
	int i;

	if (SIDEKICK_SmartNearFailedPoint(self, point))
		return;

	if (state->failedPointCount < SIDEKICK_SMART_FAILED_SIZE)
	{
		i = state->failedPointCount++;
	}
	else
	{
		i = state->failedPointHead;
		state->failedPointHead =
			(state->failedPointHead + 1) % SIDEKICK_SMART_FAILED_SIZE;
	}

	state->failedPoints[i] = point;
	state->failedPointUntil[i] = gstate->time + SIDEKICK_SMART_FAILED_TIME;
}

static qboolean SIDEKICK_SmartPathCandidate(userEntity_t* self, const CVector& point)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	float xyDistance;
	float zDistance;
	CVector testPoint = point;

	if (!hook || SIDEKICK_SmartNearFailedPoint(self, point))
		return FALSE;

	if (AI_IsPointObstructedByEntity(self, testPoint))
		return FALSE;

	xyDistance = VectorXYDistance(self->s.origin, point);
	zDistance = VectorZDistance(self->s.origin, point);

	if (xyDistance < 40.0f && fabs(zDistance) < 24.0f)
		return FALSE;

	{
		CVector safeDropPoint;
		if (SIDEKICK_SmartFindSafeDropPoint(self, testPoint, safeDropPoint))
			return TRUE;
	}

	if (AI_IsOkToMoveStraight(self, testPoint, xyDistance, zDistance))
		return TRUE;

	if (hook->pPathList && AI_FindPathToPoint(self, testPoint))
		return TRUE;

	return FALSE;
}

static qboolean SIDEKICK_SmartFindBreadcrumbPoint(
	userEntity_t* self, userEntity_t* owner, CVector& outPoint)
{
	sidekick_smart_state_t* state;
	int i;
	int index;
	float distance;

	if (!self || !owner || !AI_IsAlive(owner))
		return FALSE;

	state = SIDEKICK_SmartGetState(self);

	/*
	 * The newest reachable breadcrumb is the best recovery route.
	 * Trying the trail backwards lets a sidekick retrace the player's path
	 * through a corridor when the current owner position is not reachable.
	 */
	for (i = 0;
		i < state->ownerTrailCount &&
		i < SIDEKICK_SMART_MAX_BREADCRUMB_TESTS;
		++i)
	{
		index = (state->ownerTrailHead - 1 - i + SIDEKICK_SMART_TRAIL_SIZE) %
			SIDEKICK_SMART_TRAIL_SIZE;

		if (state->ownerTrailTime[index] <= 0.0f ||
			gstate->time - state->ownerTrailTime[index] > SIDEKICK_SMART_FAILED_TIME)
		{
			continue;
		}

		distance = VectorXYDistance(self->s.origin, state->ownerTrail[index]);
		if (distance < SIDEKICK_SMART_BREADCRUMB_MIN_DIST)
			continue;

		if (!SIDEKICK_SmartPathCandidate(self, state->ownerTrail[index]))
			continue;

		outPoint = state->ownerTrail[index];
		return TRUE;
	}

	return FALSE;
}

static qboolean SIDEKICK_SmartFindRecoveryPoint(userEntity_t* self, CVector& outPoint)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	sidekick_smart_state_t* state = SIDEKICK_SmartGetState(self);
	userEntity_t* owner;
	CVector candidates[64];
	MAPNODE_PTR nodeCandidates[32];
	int count = 0;
	int nodeCount = 0;
	int i;
	int j;
	int k;
	int index;
	int best = -1;
	float bestScore = 99999999.0f;
	float score;
	MAPNODE_PTR currentNode = NULL;
	NODEHEADER_PTR nodeHeader = NULL;
	CVector downwardRoute;

	if (!hook || !AI_IsSidekick(hook))
		return FALSE;

	owner = hook->owner;

	if (owner && AI_IsAlive(owner) &&
		SIDEKICK_SmartFindSafeDropPoint(self, owner->s.origin, downwardRoute))
	{
		candidates[count++] = downwardRoute;
	}

	for (i = 0;
		i < state->ownerTrailCount &&
		i < SIDEKICK_SMART_MAX_TRAIL_TESTS &&
		count < 64;
		++i)
	{
		index = (state->ownerTrailHead - 1 - i + SIDEKICK_SMART_TRAIL_SIZE) %
			SIDEKICK_SMART_TRAIL_SIZE;

		if (VectorXYDistance(self->s.origin, state->ownerTrail[index]) > 64.0f)
			candidates[count++] = state->ownerTrail[index];
	}

	if (hook->pNodeList)
	{
		nodeHeader = hook->pNodeList->pNodeHeader;
		currentNode = NODE_GetClosestNode(self);

		if (!currentNode &&
			hook->pNodeList->nCurrentNodeIndex != -1 &&
			nodeHeader)
		{
			currentNode = NODE_GetNode(
				nodeHeader, hook->pNodeList->nCurrentNodeIndex);
		}
	}

	if (currentNode && nodeHeader)
	{
		nodeCandidates[nodeCount++] = currentNode;

		for (i = 0;
			i < currentNode->nNumLinks &&
			nodeCount < 32 &&
			i < SIDEKICK_SMART_MAX_NODE_TESTS;
			++i)
		{
			MAPNODE_PTR node =
				NODE_GetNode(nodeHeader, currentNode->aLinks[i].nIndex);

			if (node)
				nodeCandidates[nodeCount++] = node;
		}

		{
			int firstHopCount = nodeCount;

			for (i = 1;
				i < firstHopCount &&
				i < 1 + SIDEKICK_SMART_MAX_NODE_TESTS;
				++i)
			{
				MAPNODE_PTR node = nodeCandidates[i];

				for (j = 0;
					node &&
					j < node->nNumLinks &&
					nodeCount < 32 &&
					j < SIDEKICK_SMART_MAX_NODE_TESTS;
					++j)
				{
					MAPNODE_PTR nextNode =
						NODE_GetNode(nodeHeader, node->aLinks[j].nIndex);

					if (!nextNode)
						continue;

					for (k = 0; k < nodeCount; ++k)
					{
						if (nodeCandidates[k] == nextNode)
							break;
					}

					if (k == nodeCount)
						nodeCandidates[nodeCount++] = nextNode;
				}
			}
		}

		for (i = 0; i < nodeCount && count < 64; ++i)
			candidates[count++] = nodeCandidates[i]->position;
	}

	if (owner && AI_IsAlive(owner))
	{
		playerHook_t* ownerHook = AI_GetPlayerHook(owner);

		if (ownerHook &&
			ownerHook->pNodeList &&
			ownerHook->pNodeList->pNodeHeader &&
			ownerHook->pNodeList->nCurrentNodeIndex != -1 &&
			count < 64)
		{
			MAPNODE_PTR ownerNode =
				NODE_GetNode(ownerHook->pNodeList->pNodeHeader,
					ownerHook->pNodeList->nCurrentNodeIndex);

			if (ownerNode)
				candidates[count++] = ownerNode->position;
		}

		if (count < 64)
			candidates[count++] = owner->s.origin;
	}

	for (i = 0; i < 8 && count < 64; ++i)
	{
		CVector angle = self->s.angles;
		CVector direction;

		angle.yaw = AngleMod(angle.yaw + (float)(i * 45));
		angle.pitch = 0.0f;
		angle.roll = 0.0f;
		angle.AngleToForwardVector(direction);
		direction.z = 0.0f;

		if (direction.Length() < 1.0f)
			continue;

		direction.Normalize();
		VectorMA(self->s.origin, direction, 96.0f, candidates[count]);
		candidates[count].z = self->s.origin.z;
		++count;
	}

	for (i = 0; i < count; ++i)
	{
		for (j = 0; j < i; ++j)
		{
			if (VectorXYDistance(candidates[j], candidates[i]) < 8.0f &&
				fabs(candidates[j].z - candidates[i].z) < 16.0f)
			{
				break;
			}
		}

		if (j != i)
			continue;

		if (!SIDEKICK_SmartPathCandidate(self, candidates[i]))
			continue;

		score = VectorXYDistance(self->s.origin, candidates[i]);

		if (owner && AI_IsAlive(owner))
		{
			score += VectorXYDistance(owner->s.origin, candidates[i]) * 0.18f;

			if (candidates[i].z < self->s.origin.z - 40.0f &&
				candidates[i].z <= owner->s.origin.z + 64.0f)
			{
				score -= 180.0f;
			}

			if (VectorXYDistance(owner->s.origin, candidates[i]) < 192.0f)
				score -= 70.0f;
		}

		if (fabs(candidates[i].z - self->s.origin.z) > 40.0f)
			score += 40.0f;

		if (candidates[i].z < self->s.origin.z - 40.0f)
			score -= 80.0f;

		if (AI_IsOkToMoveStraight(self, candidates[i],
			VectorXYDistance(self->s.origin, candidates[i]),
			VectorZDistance(self->s.origin, candidates[i])))
		{
			score -= 20.0f;
		}

		if (score < bestScore)
		{
			bestScore = score;
			best = i;
		}
	}

	if (best < 0)
		return FALSE;

	outPoint = candidates[best];

	if (!AI_IsOkToMoveStraight(self, outPoint,
		VectorXYDistance(self->s.origin, outPoint),
		VectorZDistance(self->s.origin, outPoint)))
	{
		if (!AI_FindPathToPoint(self, outPoint))
			return FALSE;
	}

	return TRUE;
}

int SIDEKICK_SmartRecoverMovement(userEntity_t* self, const CVector& failedPoint)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	sidekick_smart_state_t* state;
	CVector recovery;
	GOALSTACK_PTR goalStack;
	TASK_PTR currentTask;
	TASKTYPE taskType;

	if (!hook || !AI_IsSidekick(hook))
		return FALSE;
	if (SIDEKICK_NeuralScriptEventOwnsAI(self))
		return FALSE;

	state = SIDEKICK_SmartGetState(self);
	SIDEKICK_SmartRememberFailedPoint(self, failedPoint);

	if (gstate->time < state->recoveryCooldown)
		return FALSE;

	if (!SIDEKICK_SmartFindRecoveryPoint(self, recovery))
	{
		state->recoveryCooldown = gstate->time + 0.50f;
		return FALSE;
	}

	goalStack = AI_GetCurrentGoalStack(hook);
	currentTask = goalStack ? GOALSTACK_GetCurrentTask(goalStack) : NULL;

	if (currentTask)
	{
		taskType = TASK_GetType(currentTask);

		if (taskType == TASKTYPE_MOVETOLOCATION ||
			taskType == TASKTYPE_MOVETOEXACTLOCATION ||
			taskType == TASKTYPE_GOINGAROUNDOBSTACLE)
		{
			AI_RemoveCurrentTask(self);
		}
	}

	state->recoveryPoint = recovery;
	state->recoveryPointActive = TRUE;
	state->recoveryAttempts++;
	state->recoveryCooldown =
		gstate->time + SIDEKICK_SMART_RECOVER_COOLDOWN;

	AI_AddNewTaskAtFront(self, TASKTYPE_MOVETOLOCATION, recovery);
	return TRUE;
}

void SIDEKICK_SmartJumpStarted(userEntity_t* self, const CVector& target)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	sidekick_smart_state_t* state;

	if (!hook || !AI_IsSidekick(hook))
		return;

	state = SIDEKICK_SmartGetState(self);
	state->ledgeJumpForwardPending = FALSE;
	state->jumpTarget = target;
	state->jumpStart = self->s.origin;
	state->jumpStartTime = gstate->time;
	state->jumpActive = TRUE;
	state->jumpBlockedUntil = gstate->time + 0.80f;
}

qboolean SIDEKICK_SmartJumpShouldAvoid(userEntity_t* self, const CVector& target)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	sidekick_smart_state_t* state;
	float xyDistance;
	float verticalDelta;
	float maxJumpHeight;

	if (!hook || !AI_IsSidekick(hook))
		return FALSE;
	if (SIDEKICK_NeuralScriptEventOwnsAI(self))
		return FALSE;

	state = SIDEKICK_SmartGetState(self);

	/* A sidekick must never start a second jump while already airborne. */
	if (!self->groundEntity)
		return TRUE;

	if (state->jumpBlockedUntil > gstate->time)
		return TRUE;

	if (SIDEKICK_SmartNearFailedPoint(self, target))
		return TRUE;

	xyDistance = VectorXYDistance(self->s.origin, target);
	verticalDelta = target.z - self->s.origin.z;
	maxJumpHeight = ai_max_height(self, hook->upward_vel);

	if (verticalDelta < -32.0f)
	{
		CVector safeDrop;

		if (SIDEKICK_SmartFindSafeDropPoint(self, target, safeDrop))
		{
			state->jumpBlockedUntil =
				gstate->time + SIDEKICK_SMART_JUMP_RETRY_TIME;
			return TRUE;
		}
	}

	if (verticalDelta > (maxJumpHeight + 16.0f) ||
		xyDistance > (hook->max_jump_dist + 32.0f))
	{
		state->jumpBlockedUntil =
			gstate->time + SIDEKICK_SMART_JUMP_RETRY_TIME;
		return TRUE;
	}

	return FALSE;
}

void SIDEKICK_SmartJumpFinished(userEntity_t* self, const CVector& target)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	sidekick_smart_state_t* state;
	float xyDistance;
	float zDelta;

	if (!hook || !AI_IsSidekick(hook))
		return;

	state = SIDEKICK_SmartGetState(self);

	if (!state->jumpActive)
		return;

	state->jumpActive = FALSE;
	state->ledgeJumpForwardPending = FALSE;

	xyDistance = VectorXYDistance(self->s.origin, target);
	zDelta = target.z - self->s.origin.z;

	if (xyDistance > 72.0f || zDelta > 48.0f)
	{
		SIDEKICK_SmartRememberFailedPoint(self, target);
		SIDEKICK_SmartRecoverMovement(self, target);
		return;
	}

	state->recoveryAttempts = 0;
	state->recoveryPointActive = FALSE;
	state->jumpBlockedUntil = 0.0f;
}

void SIDEKICK_SmartThink(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	sidekick_smart_state_t* state;
	GOALSTACK_PTR goalStack;
	GOAL_PTR currentGoal;
	TASK_PTR currentTask;
	float moved;

	if (!hook || !AI_IsSidekick(hook))
		return;
	if (SIDEKICK_NeuralScriptEventOwnsAI(self))
	{
		state = SIDEKICK_SmartGetState(self);
		state->ledgeJumpForwardPending = FALSE;
		return;
	}

	state = SIDEKICK_SmartGetState(self);
	SIDEKICK_SmartRememberOwner(self);

	/* Human-style traversal for the low-ledge case: launch first, then drive
	 * forward once airborne.  This prevents a forward-jump from repeatedly
	 * colliding with the lip when the ledge is just above walk-over height. */
	if (state->ledgeJumpForwardPending)
	{
		if (SIDEKICK_NeuralScriptEventOwnsAI(self) ||
			self->groundEntity || !state->jumpActive)
		{
			state->ledgeJumpForwardPending = FALSE;
		}
		else if (gstate->time >= state->ledgeJumpForwardAt)
		{
			self->velocity.x = state->ledgeJumpDirection.x *
				state->ledgeJumpForwardVelocity;
			self->velocity.y = state->ledgeJumpDirection.y *
				state->ledgeJumpForwardVelocity;
			state->ledgeJumpForwardPending = FALSE;
		}
	}

	if (state->jumpActive &&
		(self->groundEntity ||
			gstate->time - state->jumpStartTime > 2.0f))
	{
		SIDEKICK_SmartJumpFinished(self, state->jumpTarget);
	}

	if (state->recoveryPointActive &&
		VectorXYDistance(self->s.origin, state->recoveryPoint) < 48.0f &&
		fabs(self->s.origin.z - state->recoveryPoint.z) < 48.0f)
	{
		state->recoveryPointActive = FALSE;
		state->recoveryAttempts = 0;
	}

	if (state->lastOriginTime > 0.0f)
	{
		moved = VectorXYDistance(state->lastOrigin, self->s.origin);
		goalStack = AI_GetCurrentGoalStack(hook);
		currentGoal = goalStack ? GOALSTACK_GetCurrentGoal(goalStack) : NULL;
		currentTask = goalStack ? GOALSTACK_GetCurrentTask(goalStack) : NULL;

		if (self->groundEntity &&
			self->velocity.Length() > 20.0f &&
			hook->owner &&
			AI_IsAlive(hook->owner) &&
			currentGoal &&
			GOAL_GetType(currentGoal) == GOALTYPE_FOLLOW &&
			currentTask &&
			moved < SIDEKICK_SMART_STUCK_DISTANCE)
		{
			if (state->stuckSince <= 0.0f)
				state->stuckSince = gstate->time;
			else if (gstate->time - state->stuckSince >=
				SIDEKICK_SMART_STUCK_TIME)
			{
				state->stuckSince = gstate->time;
				SIDEKICK_SmartRecoverMovement(self, hook->owner->s.origin);
			}
		}
		else
		{
			state->stuckSince = 0.0f;
		}
	}

	state->lastOrigin = self->s.origin;
	state->lastOriginTime = gstate->time;
}

// ----------------------------------------------------------------------------
// Neural sidekick decision layer.
// The decision engine receives a fixed-size, engine-derived state snapshot.
// Movement, navigation, collision, weapons, animation and mission scripting
// remain authoritative in the existing deterministic AI.
// ----------------------------------------------------------------------------

typedef int (*sidekick_neural_decision_fn)(const dk_neural_tactical_state_t* state, int* action);

static sidekick_neural_decision_fn sidekickNeuralDecision = NULL;
static qboolean sidekickNeuralLookupAttempted = FALSE;

static const char* sidekickNeuralActionName(int action)
{
	switch (action)
	{
	case DK_NEURAL_ACTION_FOLLOW_OWNER:  return "FOLLOW_OWNER";
	case DK_NEURAL_ACTION_ATTACK_TARGET: return "ATTACK_TARGET";
	case DK_NEURAL_ACTION_RETREAT:      return "RETREAT";
	case DK_NEURAL_ACTION_TAKE_COVER:   return "TAKE_COVER";
	case DK_NEURAL_ACTION_FLANK_LEFT:   return "FLANK_LEFT";
	case DK_NEURAL_ACTION_FLANK_RIGHT:  return "FLANK_RIGHT";
	case DK_NEURAL_ACTION_HOLD:         return "HOLD";
	}
	return "INVALID";
}

static float SIDEKICK_NeuralClamp(float value, float minimum, float maximum)
{
	if (value < minimum)
		return minimum;
	if (value > maximum)
		return maximum;
	return value;
}

static float SIDEKICK_NeuralNormalize(float value, float scale)
{
	if (scale <= 0.0f)
		return 0.0f;
	return SIDEKICK_NeuralClamp(value / scale, -1.0f, 1.0f);
}

static float SIDEKICK_NeuralNormalizeDistance(float distance)
{
	if (distance < 0.0f)
		return 1.0f;
	return SIDEKICK_NeuralClamp(distance / 1024.0f, 0.0f, 1.0f);
}

static float SIDEKICK_NeuralRelativeForward(const CVector& origin, const CVector& target, const CVector& forward)
{
	return SIDEKICK_NeuralNormalize(DotProduct(target - origin, forward), 1024.0f);
}

static float SIDEKICK_NeuralRelativeRight(const CVector& origin, const CVector& target, const CVector& right)
{
	return SIDEKICK_NeuralNormalize(DotProduct(target - origin, right), 1024.0f);
}

static float SIDEKICK_NeuralRelativeUp(const CVector& origin, const CVector& target, const CVector& up)
{
	return SIDEKICK_NeuralNormalize((target.z - origin.z), 512.0f);
}

static float SIDEKICK_NeuralClassHash(const char* className)
{
	unsigned int hash = 2166136261U;
	const unsigned char* text = (const unsigned char*)className;

	if (!text)
		return 0.0f;

	while (*text)
	{
		hash ^= (unsigned int)*text++;
		hash *= 16777619U;
	}

	return (float)(hash & 0x00FFFFFFU) / 16777215.0f;
}

static float SIDEKICK_NeuralHealthPercent(userEntity_t* entity, playerHook_t* hook)
{
	if (!entity || !hook || hook->base_health <= 0.0f)
		return 0.0f;

	return SIDEKICK_NeuralClamp(entity->health / hook->base_health, 0.0f, 1.0f);
}

static void SIDEKICK_NeuralBuildDirections(const CVector& forward, const CVector& right, CVector* directions)
{
	static const float diagonal = 0.70710678f;

	directions[0] = forward;
	directions[1] = (forward + right) * diagonal;
	directions[2] = right;
	directions[3] = (-forward + right) * diagonal;
	directions[4] = -forward;
	directions[5] = (-forward - right) * diagonal;
	directions[6] = -right;
	directions[7] = (forward - right) * diagonal;
}

static void SIDEKICK_NeuralTracePoint(userEntity_t* self, const CVector& start, const CVector& end, trace_t* trace)
{
	CVector traceStart;
	CVector traceEnd;
	CVector zeroMins;
	CVector zeroMaxs;

	traceStart.Set(start);
	traceEnd.Set(end);
	zeroMins.Set(0.0f, 0.0f, 0.0f);
	zeroMaxs.Set(0.0f, 0.0f, 0.0f);
	*trace = gstate->TraceBox_q2(traceStart, zeroMins, zeroMaxs, traceEnd, self, MASK_SOLID);
}

static void SIDEKICK_NeuralBuildGeometry(userEntity_t* self, dk_neural_tactical_state_t* state,
	const CVector& forward, const CVector& right)
{
	CVector directions[8];
	int directionIndex;

	if (!self || !state)
		return;

	SIDEKICK_NeuralBuildDirections(forward, right, directions);

	for (directionIndex = 0; directionIndex < 8; ++directionIndex)
	{
		CVector endPoint = self->s.origin + directions[directionIndex] * 192.0f;
		trace_t trace;
		float wallNormal = 0.0f;
		float wallVerticality = 0.0f;
		float hitEntity = 0.0f;

		trace = gstate->TraceBox_q2(self->s.origin, self->s.mins, self->s.maxs,
			endPoint, self, self->clipmask);
		if (trace.fraction < 1.0f)
		{
			wallNormal = SIDEKICK_NeuralClamp(-DotProduct(directions[directionIndex], trace.plane.normal), 0.0f, 1.0f);
			wallVerticality = 1.0f - SIDEKICK_NeuralClamp((float)fabs(trace.plane.normal.z), 0.0f, 1.0f);
			if (trace.ent && trace.ent != self)
				hitEntity = 1.0f;
		}

		state->geometry.horizontal[directionIndex][0] = SIDEKICK_NeuralClamp(trace.fraction, 0.0f, 1.0f);
		state->geometry.horizontal[directionIndex][1] = SIDEKICK_NeuralClamp((trace.fraction * 192.0f) / 192.0f, 0.0f, 1.0f);
		state->geometry.horizontal[directionIndex][2] = wallVerticality;
		state->geometry.horizontal[directionIndex][3] = hitEntity * wallNormal;
	}

	for (directionIndex = 0; directionIndex < 8; ++directionIndex)
	{
		CVector samplePoint = self->s.origin + directions[directionIndex] * 64.0f;
		CVector floorStart = samplePoint;
		CVector floorEnd = samplePoint;
		CVector ceilingStart = samplePoint;
		CVector ceilingEnd = samplePoint;
		trace_t floorTrace;
		trace_t ceilingTrace;

		floorStart.z += 16.0f;
		floorEnd.z -= 384.0f;
		ceilingEnd.z += 256.0f;

		SIDEKICK_NeuralTracePoint(self, floorStart, floorEnd, &floorTrace);
		SIDEKICK_NeuralTracePoint(self, ceilingStart, ceilingEnd, &ceilingTrace);

		state->geometry.floor_distance[directionIndex] = SIDEKICK_NeuralClamp(floorTrace.fraction, 0.0f, 1.0f);
		state->geometry.ceiling_distance[directionIndex] = SIDEKICK_NeuralClamp(ceilingTrace.fraction, 0.0f, 1.0f);
	}
}

static float SIDEKICK_NeuralContentBit(int contents, int bit)
{
	return (contents & bit) ? 1.0f : 0.0f;
}

static void SIDEKICK_NeuralBuildBspState(userEntity_t* self, userEntity_t* owner,
	userEntity_t* enemy, userEntity_t* other, dk_neural_tactical_state_t* state,
	const CVector& forward, const CVector& right)
{
	static const int contentBits[8] =
	{
		CONTENTS_SOLID,
		CONTENTS_WINDOW,
		CONTENTS_LAVA,
		CONTENTS_SLIME,
		CONTENTS_WATER,
		CONTENTS_LADDER,
		CONTENTS_PLAYERCLIP,
		CONTENTS_MONSTERCLIP
	};
	CVector directions[8];
	int selfContents;
	int directionIndex;

	if (!self || !state)
		return;

	SIDEKICK_NeuralBuildDirections(forward, right, directions);
	selfContents = gstate->PointContents(self->s.origin);

	for (directionIndex = 0; directionIndex < 8; ++directionIndex)
		state->bsp.self_contents[directionIndex] = SIDEKICK_NeuralContentBit(selfContents, contentBits[directionIndex]);

	for (directionIndex = 0; directionIndex < 8; ++directionIndex)
	{
		CVector sample = self->s.origin + directions[directionIndex] * 96.0f;
		int sampleContents = gstate->PointContents(sample);
		state->bsp.sample_contents[directionIndex] =
			SIDEKICK_NeuralContentBit(sampleContents, contentBits[directionIndex]);
	}

	state->bsp.owner_in_pvs = owner && AI_IsAlive(owner) && gstate->inPVS(self->s.origin, owner->s.origin) ? 1.0f : 0.0f;
	state->bsp.enemy_in_pvs = enemy && AI_IsAlive(enemy) && gstate->inPVS(self->s.origin, enemy->s.origin) ? 1.0f : 0.0f;
	state->bsp.squad_in_pvs = other && AI_IsAlive(other) && gstate->inPVS(self->s.origin, other->s.origin) ? 1.0f : 0.0f;
	state->bsp.owner_in_phs = owner && AI_IsAlive(owner) && gstate->inPHS(self->s.origin, owner->s.origin) ? 1.0f : 0.0f;
	state->bsp.enemy_in_phs = enemy && AI_IsAlive(enemy) && gstate->inPHS(self->s.origin, enemy->s.origin) ? 1.0f : 0.0f;
	state->bsp.squad_in_phs = other && AI_IsAlive(other) && gstate->inPHS(self->s.origin, other->s.origin) ? 1.0f : 0.0f;

	for (directionIndex = 0; directionIndex < 4; ++directionIndex)
	{
		CVector sample = self->s.origin + directions[directionIndex * 2] * 192.0f;
		state->bsp.local_pvs_samples[directionIndex] = gstate->inPVS(self->s.origin, sample) ? 1.0f : 0.0f;
		state->bsp.local_phs_samples[directionIndex] = gstate->inPHS(self->s.origin, sample) ? 1.0f : 0.0f;
	}
}

static sidekick_neural_decision_fn SIDEKICK_NeuralGetDecisionFunction(void)
{
#if defined(_MSC_VER)
	HMODULE module;

	if (sidekickNeuralLookupAttempted)
		return sidekickNeuralDecision;

	sidekickNeuralLookupAttempted = TRUE;
	module = GetModuleHandle(NULL);
	if (!module)
		return NULL;

	sidekickNeuralDecision = (sidekick_neural_decision_fn)GetProcAddress(
		module, "DKNeuralAI_RunTacticalState");
	if (!sidekickNeuralDecision)
	{
		gstate->Con_Dprintf("Neural AI: tactical decision entry point not found in daikatana.exe.\n");
	}
	return sidekickNeuralDecision;
#else
	return NULL;
#endif
}

static void SIDEKICK_NeuralFillThreat(userEntity_t* self, userEntity_t* owner,
	userEntity_t* threat, float distance, dk_neural_threat_state_t* output,
	const CVector& forward, const CVector& right, const CVector& up)
{
	playerHook_t* threatHook;
	float healthPercent = 0.0f;
	float threatScore;

	memset(output, 0, sizeof(*output));
	if (!self || !threat || !output)
		return;

	threatHook = AI_GetPlayerHook(threat);
	if (threatHook && threatHook->base_health > 0.0f)
		healthPercent = SIDEKICK_NeuralClamp(threat->health / threatHook->base_health, 0.0f, 1.0f);

	output->present = 1.0f;
	output->health_percent = healthPercent;
	output->distance = SIDEKICK_NeuralNormalizeDistance(distance);
	output->relative_forward = SIDEKICK_NeuralRelativeForward(self->s.origin, threat->s.origin, forward);
	output->relative_right = SIDEKICK_NeuralRelativeRight(self->s.origin, threat->s.origin, right);
	output->relative_up = SIDEKICK_NeuralRelativeUp(self->s.origin, threat->s.origin, up);
	output->visible = AI_IsVisible(self, threat) ? 1.0f : 0.0f;
	output->targeting_self = threat->enemy == self ? 1.0f : 0.0f;
	output->targeting_owner = owner && threat->enemy == owner ? 1.0f : 0.0f;
	output->type = threatHook ? SIDEKICK_NeuralNormalize((float)threatHook->type, 128.0f) : 0.0f;
	output->class_hash = SIDEKICK_NeuralClassHash(threat->className);
	output->speed = SIDEKICK_NeuralNormalize(threat->velocity.Length(), 600.0f);
	output->in_pvs = gstate->inPVS(self->s.origin, threat->s.origin) ? 1.0f : 0.0f;
	output->in_phs = gstate->inPHS(self->s.origin, threat->s.origin) ? 1.0f : 0.0f;
	output->close_range = 1.0f - output->distance;

	threatScore = output->targeting_self * 0.45f + output->targeting_owner * 0.65f;
	threatScore += output->visible * 0.25f;
	threatScore += output->close_range * 0.40f;
	output->threat_score = SIDEKICK_NeuralClamp(threatScore, 0.0f, 1.0f);
}

static int SIDEKICK_NeuralCountNearbyThreats(userEntity_t* self, userEntity_t** threats,
	float* distances, int maxThreats)
{
	userEntity_t* entity;
	int count = 0;
	int i;

	if (!self || !threats || !distances || maxThreats <= 0)
		return 0;

	for (entity = gstate->FirstEntity(); entity; entity = gstate->NextEntity(entity))
	{
		float distance;
		int insertIndex;
		int farthestIndex;

		if (entity == self || !(entity->flags & FL_MONSTER) || !AI_IsAlive(entity))
			continue;
		if (entity->flags & FL_BOT)
		{
			playerHook_t* entityHook = AI_GetPlayerHook(entity);
			if (entityHook && AI_IsSidekick(entityHook))
				continue;
		}

		distance = VectorDistance(self->s.origin, entity->s.origin);
		if (count < maxThreats)
		{
			threats[count] = entity;
			distances[count] = distance;
			count++;
			continue;
		}

		farthestIndex = 0;
		for (i = 1; i < count; ++i)
		{
			if (distances[i] > distances[farthestIndex])
				farthestIndex = i;
		}
		if (distance >= distances[farthestIndex])
			continue;

		insertIndex = farthestIndex;
		threats[insertIndex] = entity;
		distances[insertIndex] = distance;
	}

	return count;
}

static void SIDEKICK_NeuralBuildState(userEntity_t* self, dk_neural_tactical_state_t* state)
{
	playerHook_t* selfHook;
	userEntity_t* owner;
	userEntity_t* enemy;
	userEntity_t* other;
	playerHook_t* ownerHook;
	playerHook_t* enemyHook;
	playerHook_t* otherHook;
	weapon_t* weapon;
	GOALSTACK_PTR goalStack;
	GOAL_PTR currentGoal;
	sidekick_smart_state_t* smartState;
	userEntity_t* threats[8];
	float threatDistances[8];
	CVector forward;
	CVector right;
	CVector up;
	int threatCount;
	int i;
	const char* weaponName = "none";

	if (!self || !state)
		return;

	memset(state, 0, sizeof(*state));
	selfHook = AI_GetPlayerHook(self);
	if (!selfHook)
		return;

	owner = selfHook->owner;
	enemy = self->enemy && AI_IsAlive(self->enemy) ? self->enemy : NULL;
	other = SIDEKICK_GetOtherSidekick(self);
	weapon = (weapon_t*)self->curWeapon;
	smartState = SIDEKICK_SmartGetState(self);
	goalStack = AI_GetCurrentGoalStack(selfHook);
	currentGoal = goalStack ? GOALSTACK_GetCurrentGoal(goalStack) : NULL;

	self->s.angles.AngleToVectors(forward, right, up);
	state->self.health_percent = SIDEKICK_NeuralHealthPercent(self, selfHook);
	state->self.armor_percent = SIDEKICK_NeuralClamp(self->armor_val / 200.0f, 0.0f, 1.0f);
	state->self.speed = SIDEKICK_NeuralNormalize(self->velocity.Length(), 600.0f);
	state->self.velocity_x = SIDEKICK_NeuralNormalize(self->velocity.x, 600.0f);
	state->self.velocity_y = SIDEKICK_NeuralNormalize(self->velocity.y, 600.0f);
	state->self.velocity_z = SIDEKICK_NeuralNormalize(self->velocity.z, 600.0f);
	state->self.forward_x = forward.x;
	state->self.forward_y = forward.y;
	state->self.forward_z = forward.z;
	state->self.water_level = SIDEKICK_NeuralClamp((float)self->waterlevel / 3.0f, 0.0f, 1.0f);
	state->self.on_ground = self->groundEntity ? 1.0f : 0.0f;
	state->self.enemy_present = enemy ? 1.0f : 0.0f;
	state->self.enemy_distance = enemy ? SIDEKICK_NeuralNormalizeDistance(VectorDistance(self->s.origin, enemy->s.origin)) : 1.0f;

	if (weapon)
	{
		if (weapon->winfo && weapon->winfo->weaponName)
			weaponName = weapon->winfo->weaponName;
		if (weapon->ammo)
			state->self.ammo_fraction = SIDEKICK_NeuralClamp((float)weapon->ammo->count / 100.0f, 0.0f, 1.0f);
	}
	state->self.weapon_ion = !_stricmp(weaponName, "Ion Blaster") || !_stricmp(weaponName, "weapon_ionblaster") ? 1.0f : 0.0f;
	state->self.weapon_shotcycler = !_stricmp(weaponName, "Shotcycler") || !_stricmp(weaponName, "weapon_shotcycler") ? 1.0f : 0.0f;
	state->self.weapon_bolter = !_stricmp(weaponName, "Bolter") || !_stricmp(weaponName, "weapon_bolter") ? 1.0f : 0.0f;
	state->self.weapon_other = 1.0f - SIDEKICK_NeuralClamp(
		state->self.weapon_ion + state->self.weapon_shotcycler + state->self.weapon_bolter, 0.0f, 1.0f);
	state->self.ai_stay = (selfHook->ai_flags & AI_STAY) ? 1.0f : 0.0f;
	state->self.ai_noattack = (selfHook->ai_flags & AI_NOATTACK) ? 1.0f : 0.0f;
	state->self.ai_sidekickkill = (selfHook->ai_flags & AI_SIDEKICKKILL) ? 1.0f : 0.0f;
	state->self.role_mikiko = AI_IsMikiko(selfHook) ? 1.0f : 0.0f;
	state->self.role_superfly = AI_IsSuperfly(selfHook) ? 1.0f : 0.0f;
	state->self.current_task = goalStack ? SIDEKICK_NeuralNormalize((float)GOALSTACK_GetCurrentTaskType(goalStack), 128.0f) : 0.0f;
	state->self.current_goal = currentGoal ? SIDEKICK_NeuralNormalize((float)GOAL_GetType(currentGoal), 128.0f) : 0.0f;
	state->self.last_action = smartState ? SIDEKICK_NeuralNormalize((float)smartState->lastNeuralAction, 7.0f) : 0.0f;
	state->self.weapon_ready = selfHook->attack_finished <= gstate->time ? 1.0f : 0.0f;

	ownerHook = owner ? AI_GetPlayerHook(owner) : NULL;
	state->owner.alive = owner && AI_IsAlive(owner) ? 1.0f : 0.0f;
	state->owner.health_percent = owner && ownerHook ? SIDEKICK_NeuralHealthPercent(owner, ownerHook) : 0.0f;
	if (owner && AI_IsAlive(owner))
	{
		float ownerDistance = VectorDistance(self->s.origin, owner->s.origin);
		state->owner.distance = SIDEKICK_NeuralNormalizeDistance(ownerDistance);
		state->owner.relative_forward = SIDEKICK_NeuralRelativeForward(self->s.origin, owner->s.origin, forward);
		state->owner.relative_right = SIDEKICK_NeuralRelativeRight(self->s.origin, owner->s.origin, right);
		state->owner.relative_up = SIDEKICK_NeuralRelativeUp(self->s.origin, owner->s.origin, up);
		state->owner.under_attack = owner->enemy && AI_IsAlive(owner->enemy) ? 1.0f : 0.0f;
		state->owner.target_self = owner->enemy == self ? 1.0f : 0.0f;
		state->owner.enemy_present = owner->enemy && AI_IsAlive(owner->enemy) ? 1.0f : 0.0f;
		state->owner.enemy_distance = owner->enemy && AI_IsAlive(owner->enemy) ?
			SIDEKICK_NeuralNormalizeDistance(VectorDistance(owner->s.origin, owner->enemy->s.origin)) : 1.0f;
		state->owner.in_pvs = gstate->inPVS(self->s.origin, owner->s.origin) ? 1.0f : 0.0f;
		state->owner.in_phs = gstate->inPHS(self->s.origin, owner->s.origin) ? 1.0f : 0.0f;
		state->owner.visible = AI_IsVisible(self, owner) ? 1.0f : 0.0f;
		state->owner.speed = SIDEKICK_NeuralNormalize(owner->velocity.Length(), 600.0f);
		state->owner.velocity_x = SIDEKICK_NeuralNormalize(owner->velocity.x, 600.0f);
		state->owner.velocity_y = SIDEKICK_NeuralNormalize(owner->velocity.y, 600.0f);
		state->owner.velocity_z = SIDEKICK_NeuralNormalize(owner->velocity.z, 600.0f);
		state->owner.danger_score = SIDEKICK_NeuralClamp(
			state->owner.under_attack * 0.60f + (1.0f - state->owner.health_percent) * 0.40f, 0.0f, 1.0f);
		state->owner.critical = state->owner.health_percent < 0.20f ? 1.0f : 0.0f;
		state->owner.formation_distance = state->owner.distance;
	}

	enemyHook = enemy ? AI_GetPlayerHook(enemy) : NULL;
	state->enemy.present = enemy ? 1.0f : 0.0f;
	if (enemy)
	{
		float enemyDistance = VectorDistance(self->s.origin, enemy->s.origin);
		float enemyHealthPercent = enemyHook ? SIDEKICK_NeuralHealthPercent(enemy, enemyHook) : 0.0f;
		state->enemy.health_percent = enemyHealthPercent;
		state->enemy.distance = SIDEKICK_NeuralNormalizeDistance(enemyDistance);
		state->enemy.relative_forward = SIDEKICK_NeuralRelativeForward(self->s.origin, enemy->s.origin, forward);
		state->enemy.relative_right = SIDEKICK_NeuralRelativeRight(self->s.origin, enemy->s.origin, right);
		state->enemy.relative_up = SIDEKICK_NeuralRelativeUp(self->s.origin, enemy->s.origin, up);
		state->enemy.visible = AI_IsVisible(self, enemy) ? 1.0f : 0.0f;
		state->enemy.targeting_self = enemy->enemy == self ? 1.0f : 0.0f;
		state->enemy.targeting_owner = owner && enemy->enemy == owner ? 1.0f : 0.0f;
		state->enemy.in_pvs = gstate->inPVS(self->s.origin, enemy->s.origin) ? 1.0f : 0.0f;
		state->enemy.in_phs = gstate->inPHS(self->s.origin, enemy->s.origin) ? 1.0f : 0.0f;
		state->enemy.speed = SIDEKICK_NeuralNormalize(enemy->velocity.Length(), 600.0f);
		state->enemy.velocity_x = SIDEKICK_NeuralNormalize(enemy->velocity.x, 600.0f);
		state->enemy.velocity_y = SIDEKICK_NeuralNormalize(enemy->velocity.y, 600.0f);
		state->enemy.velocity_z = SIDEKICK_NeuralNormalize(enemy->velocity.z, 600.0f);
		state->enemy.type = enemyHook ? SIDEKICK_NeuralNormalize((float)enemyHook->type, 128.0f) : 0.0f;
		state->enemy.class_hash = SIDEKICK_NeuralClassHash(enemy->className);
		state->enemy.attack_range = enemyHook ? SIDEKICK_NeuralNormalize(enemyHook->attack_dist, 1024.0f) : 0.0f;
		state->enemy.attack_type = enemyHook ? SIDEKICK_NeuralNormalize((float)enemyHook->nAttackType, 16.0f) : 0.0f;
		state->enemy.attack_mode = enemyHook ? SIDEKICK_NeuralNormalize((float)enemyHook->nAttackMode, 8.0f) : 0.0f;
		state->enemy.threat_score = SIDEKICK_NeuralClamp(
			state->enemy.targeting_self * 0.45f + state->enemy.targeting_owner * 0.65f +
			state->enemy.visible * 0.20f + (1.0f - state->enemy.distance) * 0.30f,
			0.0f, 1.0f);
		state->enemy.critical = enemyHealthPercent < 0.20f ? 1.0f : 0.0f;
		state->enemy.easy_target = state->enemy.visible * (1.0f - enemyHealthPercent);
		state->enemy.close_range = 1.0f - state->enemy.distance;
		state->enemy.owner_threat = state->enemy.targeting_owner;
		state->enemy.self_threat = state->enemy.targeting_self;
		state->enemy.movement_pressure = state->enemy.speed * state->enemy.close_range;
	}

	otherHook = other ? AI_GetPlayerHook(other) : NULL;
	if (other && otherHook && AI_IsAlive(other))
	{
		state->squad.present = 1.0f;
		state->squad.health_percent = SIDEKICK_NeuralHealthPercent(other, otherHook);
		state->squad.distance = SIDEKICK_NeuralNormalizeDistance(VectorDistance(self->s.origin, other->s.origin));
		state->squad.relative_forward = SIDEKICK_NeuralRelativeForward(self->s.origin, other->s.origin, forward);
		state->squad.relative_right = SIDEKICK_NeuralRelativeRight(self->s.origin, other->s.origin, right);
		state->squad.relative_up = SIDEKICK_NeuralRelativeUp(self->s.origin, other->s.origin, up);
		state->squad.in_pvs = gstate->inPVS(self->s.origin, other->s.origin) ? 1.0f : 0.0f;
		state->squad.in_phs = gstate->inPHS(self->s.origin, other->s.origin) ? 1.0f : 0.0f;
		state->squad.visible = AI_IsVisible(self, other) ? 1.0f : 0.0f;
		state->squad.speed = SIDEKICK_NeuralNormalize(other->velocity.Length(), 600.0f);
		state->squad.velocity_x = SIDEKICK_NeuralNormalize(other->velocity.x, 600.0f);
		state->squad.velocity_y = SIDEKICK_NeuralNormalize(other->velocity.y, 600.0f);
		state->squad.velocity_z = SIDEKICK_NeuralNormalize(other->velocity.z, 600.0f);
		state->squad.role_mikiko = AI_IsMikiko(otherHook) ? 1.0f : 0.0f;
		state->squad.role_superfly = AI_IsSuperfly(otherHook) ? 1.0f : 0.0f;
		state->squad.distance_pressure = state->squad.distance;
		state->squad.health_pressure = 1.0f - state->squad.health_percent;
		state->squad.cooperation_need = (state->enemy.targeting_self + state->enemy.targeting_owner) * 0.5f;
		state->squad.formation_quality = 1.0f - state->squad.distance;
		state->squad.threat_pressure = state->enemy.threat_score;
	}

	threatCount = SIDEKICK_NeuralCountNearbyThreats(self, threats, threatDistances, 8);
	for (i = 0; i < threatCount; ++i)
		SIDEKICK_NeuralFillThreat(self, owner, threats[i], threatDistances[i], &state->threats[i], forward, right, up);

	SIDEKICK_NeuralBuildGeometry(self, state, forward, right);
	SIDEKICK_NeuralBuildBspState(self, owner, enemy, other, state, forward, right);

	/* Compact short-term memory in the existing reserved input slot. */
	if (owner && AI_IsAlive(owner))
	{
		smartState->lastOwnerOrigin = owner->s.origin;
		smartState->lastOwnerSampleTime = gstate->time;
	}
	if (enemy)
	{
		smartState->lastEnemyOrigin = enemy->s.origin;
		smartState->lastEnemySeenTime = gstate->time;
		smartState->lastEnemyNumber = enemy->s.number;
	}
	{
		unsigned int contextFlags = 0;
		if (owner && AI_IsAlive(owner) && owner->velocity.Length() > 20.0f) contextFlags |= 1U << 0;
		if (owner && AI_IsAlive(owner) && VectorDistance(self->s.origin, owner->s.origin) > 512.0f) contextFlags |= 1U << 1;
		if (owner && AI_IsAlive(owner) && !state->owner.in_pvs) contextFlags |= 1U << 2;
		if (smartState->lastEnemySeenTime > 0.0f && gstate->time - smartState->lastEnemySeenTime < 4.0f) contextFlags |= 1U << 3;
		if (smartState->lastInteractionTime > 0.0f && gstate->time - smartState->lastInteractionTime < 4.0f) contextFlags |= 1U << 4;
		if (smartState->advancedRouteActive) contextFlags |= 1U << 5;
		if (smartState->advancedExploreActive) contextFlags |= 1U << 6;
		if (smartState->neuralEnhancementsSuspended) contextFlags |= 1U << 7;
		if (other && otherHook && AI_IsAlive(other)) contextFlags |= 1U << 8;
		if (other && otherHook && AI_IsAlive(other) && other->enemy && AI_IsAlive(other->enemy)) contextFlags |= 1U << 9;
		state->enemy.context_summary = (float)contextFlags / 1023.0f;
	}

	state->self.movement_blocked = state->geometry.horizontal[0][0] < 0.35f ? 1.0f : 0.0f;
}


static int SIDEKICK_NeuralIsAmbientTask(TASKTYPE taskType);
static qboolean SIDEKICK_NeuralHasScriptCriticalTask(userEntity_t* self);
static qboolean SIDEKICK_NeuralTaskIsScriptCritical(TASKTYPE taskType);

#define SIDEKICK_ADVANCED_MAX_SECRETS              64
#define SIDEKICK_ADVANCED_MAX_INTERACTABLES      256
#define SIDEKICK_ADVANCED_MAX_ACTION_NODES       256

#define SIDEKICK_ADVANCED_ACTION_NONE             0
#define SIDEKICK_ADVANCED_ACTION_SWITCH          1
#define SIDEKICK_ADVANCED_ACTION_DOORBUTTON      2
#define SIDEKICK_ADVANCED_ACTION_DOOR            3
#define SIDEKICK_ADVANCED_ACTION_PLATFORM        4
#define SIDEKICK_ADVANCED_ACTION_TRAIN           5
#define SIDEKICK_ADVANCED_ACTION_ELEVATOR        6
#define SIDEKICK_ADVANCED_ACTION_LADDER          7
#define SIDEKICK_ADVANCED_ACTION_JUMP            8
#define SIDEKICK_ADVANCED_ACTION_TELEPORT        9
#define SIDEKICK_ADVANCED_ACTION_CROUCH          10
#define SIDEKICK_ADVANCED_OWNER_RADIUS           768.0f
#define SIDEKICK_ADVANCED_MAX_DISTANCE           960.0f
#define SIDEKICK_ADVANCED_REJOIN_DISTANCE        896.0f
#define SIDEKICK_ADVANCED_SCAN_INTERVAL            1.0f
#define SIDEKICK_ADVANCED_COOLDOWN                 4.0f
#define SIDEKICK_ADVANCED_OWNER_SPEED             24.0f

/*
 * Advanced Reasoning receives a semantic view of the whole loaded level.
 * The current 5M tactical model deliberately keeps its 302-float ABI, so we
 * do not copy raw cmodel_t BSP memory into that tensor. Instead the planner
 * has the complete ground navigation graph/path table, special node flags,
 * level interactables and secrets, with live BSP-backed collision/content/PVS
 * queries remaining authoritative at execution time.
 */
typedef struct
{
	char mapName[64];
	int subMap;
	NODEHEADER_PTR groundNodes;
	int groundNodeCount;
	int specialNodeCounts[12];
	userEntity_t* secrets[SIDEKICK_ADVANCED_MAX_SECRETS];
	CVector secretPoints[SIDEKICK_ADVANCED_MAX_SECRETS];
	int secretCount;
	userEntity_t* interactables[SIDEKICK_ADVANCED_MAX_INTERACTABLES];
	CVector interactablePoints[SIDEKICK_ADVANCED_MAX_INTERACTABLES];
	int interactableCount;
	int actionNodeCount;
	MAPNODE_PTR actionNodes[SIDEKICK_ADVANCED_MAX_ACTION_NODES];
	int actionKinds[SIDEKICK_ADVANCED_MAX_ACTION_NODES];
	userEntity_t* actionControllers[SIDEKICK_ADVANCED_MAX_ACTION_NODES];
	userEntity_t* actionTargets[SIDEKICK_ADVANCED_MAX_ACTION_NODES];
	float lastRefreshTime;
	float lastLevelTime;
	qboolean valid;
} sidekick_advanced_level_t;

static sidekick_advanced_level_t sidekickAdvancedLevel;

static void SIDEKICK_AdvancedResetLevelKnowledge(void)
{
	memset(&sidekickAdvancedLevel, 0, sizeof(sidekickAdvancedLevel));
	if (gstate)
	{
		strncpy(sidekickAdvancedLevel.mapName, gstate->mapName, sizeof(sidekickAdvancedLevel.mapName) - 1);
		sidekickAdvancedLevel.mapName[sizeof(sidekickAdvancedLevel.mapName) - 1] = '\0';
		sidekickAdvancedLevel.subMap = gstate->subMap;
	}
}

static int SIDEKICK_AdvancedNodeSpecialIndex(int nodeType)
{
	if (nodeType & NODETYPE_SWITCH) return 0;
	if (nodeType & NODETYPE_PLATFORM) return 1;
	if (nodeType & NODETYPE_TRAIN) return 2;
	if (nodeType & NODETYPE_ELEVATOR) return 3;
	if (nodeType & NODETYPE_DOOR) return 4;
	if (nodeType & NODETYPE_JUMPUP) return 5;
	if (nodeType & NODETYPE_JUMPFORWARD) return 6;
	if (nodeType & NODETYPE_TELEPORTER) return 7;
	if (nodeType & NODETYPE_LADDER) return 8;
	if (nodeType & NODETYPE_DOORBUTTON) return 9;
	if (nodeType & NODETYPE_SHOTCYCLERJUMP) return 10;
	if (nodeType & NODETYPE_CROUCH) return 11;
	return -1;
}

static CVector SIDEKICK_AdvancedEntityCenter(userEntity_t* entity)
{
	CVector point;
	if (!entity)
		return CVector(0.0f, 0.0f, 0.0f);
	point.x = (entity->absmin.x + entity->absmax.x) * 0.5f;
	point.y = (entity->absmin.y + entity->absmax.y) * 0.5f;
	point.z = (entity->absmin.z + entity->absmax.z) * 0.5f;
	return point;
}

static qboolean SIDEKICK_AdvancedIsSecretDoor(userEntity_t* entity)
{
	return entity && entity->use == secret_use;
}

static userEntity_t* SIDEKICK_AdvancedResolveNodeTarget(MAPNODE_PTR node)
{
	userEntity_t* entity = NULL;

	if (!node || !node->target)
		return NULL;

	entity = com->FindNodeTarget(node->target);
	if (!entity)
		entity = com->FindTarget(node->target);
	return entity;
}

static userEntity_t* SIDEKICK_AdvancedFindButtonForMover(userEntity_t* mover, const CVector& point)
{
	userEntity_t* best = NULL;
	userEntity_t* entity;
	float bestDistance = 99999999.0f;

	if (!mover)
		return NULL;

	for (entity = gstate->FirstEntity(); entity; entity = gstate->NextEntity(entity))
	{
		float distance;

		if (!entity->className || _stricmp(entity->className, "func_button"))
			continue;
		if (!entity->use || !entity->target || !mover->targetname)
			continue;
		if (_stricmp(entity->target, mover->targetname))
			continue;

		distance = VectorDistance(entity->s.origin, point);
		if (distance < bestDistance)
		{
			bestDistance = distance;
			best = entity;
		}
	}

	return best;
}

static userEntity_t* SIDEKICK_AdvancedFindPlatformAtNode(MAPNODE_PTR node)
{
	CVector start;
	CVector end;
	trace_t platformTrace;

	if (!node)
		return NULL;

	start = node->position;
	start.z += 24.0f;
	end = node->position;
	end.z -= MAX_JUMP_HEIGHT;
	platformTrace = gstate->TraceLine_q2(start, end, NULL, MASK_SOLID);

	if (platformTrace.fraction < 1.0f && platformTrace.ent &&
		platformTrace.ent->className &&
		!_stricmp(platformTrace.ent->className, "func_plat"))
		return platformTrace.ent;

	return NULL;
}

static int SIDEKICK_AdvancedResolveNodeAction(
	MAPNODE_PTR node,
	userEntity_t** controller,
	userEntity_t** target)
{
	int action = SIDEKICK_ADVANCED_ACTION_NONE;
	userEntity_t* resolvedTarget = NULL;
	userEntity_t* resolvedController = NULL;

	if (controller)
		*controller = NULL;
	if (target)
		*target = NULL;
	if (!node)
		return SIDEKICK_ADVANCED_ACTION_NONE;

	if (node->node_type & NODETYPE_SWITCH)
	{
		resolvedController = SIDEKICK_AdvancedResolveNodeTarget(node);
		if (resolvedController)
			resolvedTarget = resolvedController->target ? com->FindTarget(resolvedController->target) : NULL;
		action = SIDEKICK_ADVANCED_ACTION_SWITCH;
	}
	else if (node->node_type & NODETYPE_DOORBUTTON)
	{
		resolvedController = SIDEKICK_AdvancedResolveNodeTarget(node);
		if (resolvedController)
			resolvedTarget = resolvedController->target ? com->FindTarget(resolvedController->target) : NULL;
		action = SIDEKICK_ADVANCED_ACTION_DOORBUTTON;
	}
	else if (node->node_type & NODETYPE_DOOR)
	{
		resolvedTarget = SIDEKICK_AdvancedResolveNodeTarget(node);
		action = SIDEKICK_ADVANCED_ACTION_DOOR;
	}
	else if (node->node_type & (NODETYPE_TRAIN | NODETYPE_ELEVATOR))
	{
		resolvedTarget = SIDEKICK_AdvancedResolveNodeTarget(node);
		if (resolvedTarget)
		{
			resolvedController = AI_GetSwitchOnTrain(resolvedTarget);
			if (!resolvedController)
				resolvedController = AI_GetClosestSwitch(resolvedTarget, node->position);
		}
		action = (node->node_type & NODETYPE_ELEVATOR) ?
			SIDEKICK_ADVANCED_ACTION_ELEVATOR : SIDEKICK_ADVANCED_ACTION_TRAIN;
	}
	else if (node->node_type & NODETYPE_PLATFORM)
	{
		resolvedTarget = SIDEKICK_AdvancedFindPlatformAtNode(node);
		if (resolvedTarget)
			resolvedController = SIDEKICK_AdvancedFindButtonForMover(resolvedTarget, node->position);
		action = SIDEKICK_ADVANCED_ACTION_PLATFORM;
	}
	else if (node->node_type & NODETYPE_LADDER)
	{
		action = SIDEKICK_ADVANCED_ACTION_LADDER;
	}
	else if (node->node_type & (NODETYPE_JUMPUP | NODETYPE_JUMPFORWARD | NODETYPE_SHOTCYCLERJUMP))
	{
		action = SIDEKICK_ADVANCED_ACTION_JUMP;
	}
	else if (node->node_type & NODETYPE_TELEPORTER)
	{
		action = SIDEKICK_ADVANCED_ACTION_TELEPORT;
	}
	else if (node->node_type & NODETYPE_CROUCH)
	{
		action = SIDEKICK_ADVANCED_ACTION_CROUCH;
	}

	if (controller)
		*controller = resolvedController;
	if (target)
		*target = resolvedTarget;

	return action;
}

static void SIDEKICK_AdvancedBuildRoutePlan(userEntity_t* self)
{
	sidekick_smart_state_t* state;
	playerHook_t* hook;
	PATHNODE_PTR pathNode;
	int i;

	if (!self)
		return;

	state = SIDEKICK_SmartGetState(self);
	hook = AI_GetPlayerHook(self);
	state->advancedRouteActionCount = 0;
	state->advancedRouteNextAction = 0;

	if (!hook || !hook->pPathList || !hook->pPathList->pPath || !sidekickAdvancedLevel.valid)
		return;

	pathNode = hook->pPathList->pPath;
	while (pathNode && state->advancedRouteActionCount < SIDEKICK_ADVANCED_MAX_ROUTE_ACTIONS)
	{
		MAPNODE_PTR node = NODE_GetNode(sidekickAdvancedLevel.groundNodes, pathNode->nNodeIndex);
		userEntity_t* controller = NULL;
		userEntity_t* target = NULL;
		int action;

		if (!node)
		{
			pathNode = pathNode->next_node;
			continue;
		}

		action = SIDEKICK_AdvancedResolveNodeAction(node, &controller, &target);
		if (action != SIDEKICK_ADVANCED_ACTION_NONE)
		{
			int index = state->advancedRouteActionCount++;
			state->advancedRouteActions[index].nodeIndex = node->nIndex;
			state->advancedRouteActions[index].nodeType = node->node_type;
			state->advancedRouteActions[index].actionKind = action;
			state->advancedRouteActions[index].controller = controller;
			state->advancedRouteActions[index].target = target;
			state->advancedRouteActions[index].nodePoint = node->position;
			state->advancedRouteActions[index].controllerPoint = controller ? SIDEKICK_AdvancedEntityCenter(controller) : node->position;
			state->advancedRouteActions[index].targetPoint = target ? SIDEKICK_AdvancedEntityCenter(target) : node->position;
		}

		pathNode = pathNode->next_node;
	}

	/* Advance the index past actions which are already behind the sidekick. */
	for (i = 0; i < state->advancedRouteActionCount; ++i)
	{
		if (VectorDistance(self->s.origin, state->advancedRouteActions[i].nodePoint) > 96.0f)
			break;
	}
	state->advancedRouteNextAction = i < state->advancedRouteActionCount ? i : state->advancedRouteActionCount;
	if (state->advancedRouteNextAction < state->advancedRouteActionCount)
	{
		sidekick_advanced_route_action_t* nextAction = &state->advancedRouteActions[state->advancedRouteNextAction];
		if (nextAction->controller &&
			VectorDistance(self->s.origin, nextAction->controllerPoint) < 128.0f)
		{
			state->lastInteractionTime = gstate->time;
			state->lastInteractionNumber = nextAction->controller->s.number;
		}
	}
}

static qboolean SIDEKICK_AdvancedCanRejoin(userEntity_t* self)
{
	playerHook_t* hook;

	if (!self || (gstate->GetCvar("dk_advanced_reasoning") <= 0.0f &&
		gstate->GetCvar("dk_neural_ai") <= 0.0f))
		return FALSE;

	hook = AI_GetPlayerHook(self);
	if (!hook || !AI_IsSidekick(hook) || (hook->ai_flags & AI_STAY) || hook->bCarryingMikiko)
		return FALSE;
	if (!hook->owner || !AI_IsAlive(hook->owner))
		return FALSE;
	if (gstate->bCinematicPlaying || (self->enemy && AI_IsAlive(self->enemy)))
		return FALSE;
	if (SIDEKICK_NeuralHasScriptCriticalTask(self))
		return FALSE;

	return TRUE;
}

static qboolean SIDEKICK_AdvancedStartRoute(
	userEntity_t* self,
	const CVector& point,
	int mode)
{
	CVector mutablePoint;
	sidekick_smart_state_t* state;
	playerHook_t* hook;

	if (!self || !SIDEKICK_AdvancedCanRejoin(self))
		return FALSE;

	hook = AI_GetPlayerHook(self);
	if (!hook)
		return FALSE;

	mutablePoint = point;
	if (!AI_FindPathToPoint(self, mutablePoint))
		return FALSE;

	state = SIDEKICK_SmartGetState(self);
	state->advancedRouteActive = TRUE;
	state->advancedRouteMode = mode;
	state->advancedRouteGoalPoint = point;
	state->advancedRouteStartTime = gstate->time;
	state->advancedRouteNextReplanTime = gstate->time + SIDEKICK_ADVANCED_REPLAN_INTERVAL;
	SIDEKICK_AdvancedBuildRoutePlan(self);

	{
		GOALSTACK_PTR goalStack = AI_GetCurrentGoalStack(hook);
		TASK_PTR currentTask = goalStack ? GOALSTACK_GetCurrentTask(goalStack) : NULL;
		if (currentTask && TASK_GetType(currentTask) == TASKTYPE_MOVETOLOCATION)
			AI_RemoveCurrentTask(self);
	}

	AI_AddNewTaskAtFront(self, TASKTYPE_MOVETOLOCATION, point);
	return TRUE;
}

static void SIDEKICK_AdvancedCancelRoute(userEntity_t* self, qboolean returnToOwner)
{
	sidekick_smart_state_t* state;
	playerHook_t* hook;
	GOALSTACK_PTR goalStack;
	TASK_PTR task;

	if (!self)
		return;

	state = SIDEKICK_SmartGetState(self);
	hook = AI_GetPlayerHook(self);
	state->advancedRouteActive = FALSE;
	state->advancedRouteMode = 0;
	state->advancedRouteActionCount = 0;
	state->advancedRouteNextAction = 0;

	if (!hook)
		return;

	goalStack = AI_GetCurrentGoalStack(hook);
	task = goalStack ? GOALSTACK_GetCurrentTask(goalStack) : NULL;
	if (task && TASK_GetType(task) == TASKTYPE_MOVETOLOCATION)
		AI_RemoveCurrentTask(self);

	if (returnToOwner && hook->owner && AI_IsAlive(hook->owner) &&
		!SIDEKICK_NeuralHasScriptCriticalTask(self))
		AI_AddNewTaskAtFront(self, TASKTYPE_MOVETOOWNER);
}

static qboolean SIDEKICK_AdvancedRouteNeedsReplan(userEntity_t* self)
{
	sidekick_smart_state_t* state;
	playerHook_t* hook;
	CVector target;

	if (!self)
		return FALSE;

	state = SIDEKICK_SmartGetState(self);
	hook = AI_GetPlayerHook(self);
	if (!state->advancedRouteActive || !hook || !hook->owner || !AI_IsAlive(hook->owner))
		return FALSE;
	if (gstate->time < state->advancedRouteNextReplanTime)
		return FALSE;

	target = hook->owner->s.origin;
	if (state->advancedRouteMode == SIDEKICK_ADVANCED_ROUTE_EXPLORE)
		return FALSE;

	state->advancedRouteNextReplanTime = gstate->time + SIDEKICK_ADVANCED_REPLAN_INTERVAL;
	return VectorDistance(target, state->advancedRouteGoalPoint) > 160.0f;
}

static void SIDEKICK_AdvancedRefreshLevelKnowledge(void)
{
	userEntity_t* entity;
	int i;

	if (gstate->GetCvar("dk_advanced_reasoning") <= 0.0f &&
		gstate->GetCvar("dk_neural_ai") <= 0.0f)
		return;

	if (sidekickAdvancedLevel.valid &&
		!_stricmp(sidekickAdvancedLevel.mapName, gstate->mapName) &&
		sidekickAdvancedLevel.subMap == gstate->subMap &&
		sidekickAdvancedLevel.groundNodes == pGroundNodes &&
		gstate->time >= sidekickAdvancedLevel.lastLevelTime &&
		gstate->time - sidekickAdvancedLevel.lastRefreshTime < SIDEKICK_ADVANCED_SCAN_INTERVAL)
		return;

	if (!sidekickAdvancedLevel.valid ||
		_stricmp(sidekickAdvancedLevel.mapName, gstate->mapName) ||
		sidekickAdvancedLevel.subMap != gstate->subMap ||
		sidekickAdvancedLevel.groundNodes != pGroundNodes ||
		gstate->time < sidekickAdvancedLevel.lastLevelTime)
		SIDEKICK_AdvancedResetLevelKnowledge();

	sidekickAdvancedLevel.groundNodes = pGroundNodes;
	sidekickAdvancedLevel.groundNodeCount = pGroundNodes ? pGroundNodes->nNumNodes : 0;
	sidekickAdvancedLevel.secretCount = 0;
	sidekickAdvancedLevel.interactableCount = 0;
	sidekickAdvancedLevel.actionNodeCount = 0;
	memset(sidekickAdvancedLevel.specialNodeCounts, 0, sizeof(sidekickAdvancedLevel.specialNodeCounts));

	/* Full navigation graph metadata. */
	if (pGroundNodes && pGroundNodes->pNodes)
	{
		for (i = 0; i < pGroundNodes->nNumNodes; ++i)
		{
			MAPNODE_PTR node = &pGroundNodes->pNodes[i];
			int specialIndex = SIDEKICK_AdvancedNodeSpecialIndex(node->node_type);
			if (specialIndex >= 0)
				sidekickAdvancedLevel.specialNodeCounts[specialIndex]++;
			if (SIDEKICK_AdvancedResolveNodeAction(node, NULL, NULL) != SIDEKICK_ADVANCED_ACTION_NONE &&
				sidekickAdvancedLevel.actionNodeCount < SIDEKICK_ADVANCED_MAX_ACTION_NODES)
			{
				userEntity_t* controller = NULL;
				userEntity_t* target = NULL;
				int action = SIDEKICK_AdvancedResolveNodeAction(node, &controller, &target);
				int index = sidekickAdvancedLevel.actionNodeCount++;
				sidekickAdvancedLevel.actionNodes[index] = node;
				sidekickAdvancedLevel.actionKinds[index] = action;
				sidekickAdvancedLevel.actionControllers[index] = controller;
				sidekickAdvancedLevel.actionTargets[index] = target;
			}
		}
	}

	/* Full level-entity scan for secrets and usable world objects. */
	for (entity = gstate->FirstEntity(); entity; entity = gstate->NextEntity(entity))
	{
		qboolean isSecret;
		qboolean isSecretDoor;
		CVector point;

		isSecret = entity->className && !_stricmp(entity->className, "trigger_secret");
		isSecretDoor = SIDEKICK_AdvancedIsSecretDoor(entity);

		if (isSecret && entity->solid != SOLID_NOT && sidekickAdvancedLevel.secretCount < SIDEKICK_ADVANCED_MAX_SECRETS)
		{
			point = SIDEKICK_AdvancedEntityCenter(entity);
			if (!(gstate->PointContents(point) & (CONTENTS_LAVA | CONTENTS_SLIME)))
			{
				sidekickAdvancedLevel.secrets[sidekickAdvancedLevel.secretCount] = entity;
				sidekickAdvancedLevel.secretPoints[sidekickAdvancedLevel.secretCount] = point;
				sidekickAdvancedLevel.secretCount++;
			}
		}

		if ((isSecret || isSecretDoor || entity->use) &&
			!(entity->flags & FL_MONSTER) && !(entity->flags & FL_CLIENT) &&
			sidekickAdvancedLevel.interactableCount < SIDEKICK_ADVANCED_MAX_INTERACTABLES)
		{
			point = SIDEKICK_AdvancedEntityCenter(entity);
			sidekickAdvancedLevel.interactables[sidekickAdvancedLevel.interactableCount] = entity;
			sidekickAdvancedLevel.interactablePoints[sidekickAdvancedLevel.interactableCount] = point;
			sidekickAdvancedLevel.interactableCount++;
		}
	}

	sidekickAdvancedLevel.lastRefreshTime = gstate->time;
	sidekickAdvancedLevel.lastLevelTime = gstate->time;
	sidekickAdvancedLevel.valid = TRUE;
}

static float SIDEKICK_AdvancedGraphDistance(MAPNODE_PTR startNode, MAPNODE_PTR endNode)
{
	int currentIndex;
	int endIndex;
	float distance = 0.0f;
	int safety;

	if (!startNode || !endNode || !sidekickAdvancedLevel.groundNodes)
		return -1.0f;
	if (startNode == endNode)
		return 0.0f;

	/* The node path table contains the first hop of the shortest route. */
	if (!sidekickAdvancedLevel.groundNodes->bUsePathTable ||
		!sidekickAdvancedLevel.groundNodes->pPathTable ||
		sidekickAdvancedLevel.groundNodes->nNumPathTableNodes < sidekickAdvancedLevel.groundNodeCount)
		return VectorDistance(startNode->position, endNode->position);

	currentIndex = startNode->nIndex;
	endIndex = endNode->nIndex;

	for (safety = 0; safety < sidekickAdvancedLevel.groundNodeCount; ++safety)
	{
		short nextIndex;
		MAPNODE_PTR nextNode;

		if (currentIndex == endIndex)
			return distance;
		if (currentIndex < 0 || currentIndex >= sidekickAdvancedLevel.groundNodeCount)
			return -1.0f;

		nextIndex = sidekickAdvancedLevel.groundNodes->pPathTable[currentIndex][endIndex];
		if (nextIndex < 0 || nextIndex >= sidekickAdvancedLevel.groundNodeCount || nextIndex == currentIndex)
			return -1.0f;

		nextNode = &sidekickAdvancedLevel.groundNodes->pNodes[nextIndex];
		distance += VectorDistance(sidekickAdvancedLevel.groundNodes->pNodes[currentIndex].position, nextNode->position);
		currentIndex = nextIndex;
	}

	return -1.0f;
}

static qboolean SIDEKICK_AdvancedCanExplore(userEntity_t* self)
{
	playerHook_t* hook;
	GOALSTACK_PTR goalStack;
	GOAL_PTR goal;
	TASK_PTR task;
	GOALTYPE goalType;
	TASKTYPE taskType;

	if (!self || (gstate->GetCvar("dk_advanced_reasoning") <= 0.0f &&
		gstate->GetCvar("dk_neural_ai") <= 0.0f))
		return FALSE;
	hook = AI_GetPlayerHook(self);
	if (!hook || !AI_IsSidekick(hook) || (hook->ai_flags & AI_STAY) || hook->bCarryingMikiko)
		return FALSE;
	if (gstate->bCinematicPlaying || (self->enemy && AI_IsAlive(self->enemy)))
		return FALSE;
	if (!hook->owner || !AI_IsAlive(hook->owner))
		return FALSE;
	if (hook->owner->velocity.Length() > SIDEKICK_ADVANCED_OWNER_SPEED)
		return FALSE;
	if (VectorDistance(self->s.origin, hook->owner->s.origin) > SIDEKICK_ADVANCED_OWNER_RADIUS)
		return FALSE;
	if (SIDEKICK_NeuralHasScriptCriticalTask(self))
		return FALSE;

	goalStack = AI_GetCurrentGoalStack(hook);
	goal = goalStack ? GOALSTACK_GetCurrentGoal(goalStack) : NULL;
	task = goal ? GOAL_GetCurrentTask(goal) : NULL;
	goalType = goal ? GOAL_GetType(goal) : GOALTYPE_NONE;
	taskType = task ? TASK_GetType(task) : TASKTYPE_IDLE;

	if (goalType != GOALTYPE_NONE && goalType != GOALTYPE_IDLE && goalType != GOALTYPE_FOLLOW &&
		goalType != GOALTYPE_SIDEKICK_FOLLOWOWNER)
		return FALSE;

	if (task && !SIDEKICK_NeuralIsAmbientTask(taskType) && taskType != TASKTYPE_IDLE &&
		taskType != TASKTYPE_FOLLOWRUNNING && taskType != TASKTYPE_FOLLOWWALKING &&
		taskType != TASKTYPE_MOVETOOWNER && taskType != TASKTYPE_SIDEKICK_STOP)
		return FALSE;

	return TRUE;
}

static qboolean SIDEKICK_AdvancedFindSecretTarget(userEntity_t* self, CVector& outPoint, int& outNumber)
{
	playerHook_t* hook;
	MAPNODE_PTR startNode;
	float bestScore = 99999999.0f;
	int best = -1;
	int i;

	if (!SIDEKICK_AdvancedCanExplore(self) || !sidekickAdvancedLevel.groundNodes)
		return FALSE;
	hook = AI_GetPlayerHook(self);
	startNode = NODE_GetClosestNode(self);
	if (!hook || !startNode)
		return FALSE;

	for (i = 0; i < sidekickAdvancedLevel.secretCount; ++i)
	{
		userEntity_t* secret = sidekickAdvancedLevel.secrets[i];
		userEntity_t* other = SIDEKICK_GetOtherSidekick(self);
		if (other)
		{
			sidekick_smart_state_t* otherState = SIDEKICK_SmartGetState(other);
			if (otherState->advancedExploreActive && otherState->advancedExploreTargetNumber == secret->s.number)
				continue;
		}
		MAPNODE_PTR targetNode;
		CVector point;
		float ownerDistance;
		float selfDistance;
		float graphDistance;
		float score;

		if (!secret || secret->solid == SOLID_NOT)
			continue;
		point = sidekickAdvancedLevel.secretPoints[i];
		ownerDistance = VectorDistance(hook->owner->s.origin, point);
		selfDistance = VectorDistance(self->s.origin, point);
		if (ownerDistance > SIDEKICK_ADVANCED_OWNER_RADIUS || selfDistance > SIDEKICK_ADVANCED_MAX_DISTANCE)
			continue;
		if (gstate->PointContents(point) & (CONTENTS_LAVA | CONTENTS_SLIME))
			continue;

		targetNode = NODE_FindClosestNode(sidekickAdvancedLevel.groundNodes, NODETYPE_GROUND, point);
		if (!targetNode)
			continue;
		graphDistance = SIDEKICK_AdvancedGraphDistance(startNode, targetNode);
		if (graphDistance < 0.0f)
			continue;

		score = graphDistance + ownerDistance * 0.75f + selfDistance * 0.15f;
		if (score < bestScore)
		{
			bestScore = score;
			best = i;
		}
	}

	if (best < 0)
		return FALSE;

	outPoint = sidekickAdvancedLevel.secretPoints[best];
	outNumber = sidekickAdvancedLevel.secrets[best]->s.number;
	return TRUE;
}

static void SIDEKICK_AdvancedCancelExplore(userEntity_t* self, qboolean returnToOwner)
{
	playerHook_t* hook;
	sidekick_smart_state_t* state;

	if (!self)
		return;
	hook = AI_GetPlayerHook(self);
	if (!hook)
		return;
	state = SIDEKICK_SmartGetState(self);
	state->advancedExploreActive = FALSE;
	state->advancedExploreTargetNumber = -1;
	state->advancedExploreCooldown = gstate->time + SIDEKICK_ADVANCED_COOLDOWN;

	/* Exploration uses the same deterministic route planner as rejoining. */
	SIDEKICK_AdvancedCancelRoute(self, returnToOwner);
}

static void SIDEKICK_AdvancedStartExplore(userEntity_t* self, const CVector& point, int entityNumber)
{
	sidekick_smart_state_t* state;

	if (!self || !SIDEKICK_AdvancedCanExplore(self))
		return;

	if (!SIDEKICK_AdvancedStartRoute(self, point, SIDEKICK_ADVANCED_ROUTE_EXPLORE))
		return;

	state = SIDEKICK_SmartGetState(self);
	state->advancedExploreActive = TRUE;
	state->advancedExploreTargetNumber = entityNumber;
	state->advancedExploreTargetPoint = point;
	state->advancedExploreStartTime = gstate->time;
	state->advancedExploreSecretCount = gstate->numSecretsFound;
	state->advancedExploreCooldown = gstate->time + SIDEKICK_ADVANCED_COOLDOWN;
}

static void SIDEKICK_SmartCombatInterrupt(userEntity_t* self)
{
	playerHook_t* hook;
	sidekick_smart_state_t* state;
	GOALSTACK_PTR goalStack;
	GOAL_PTR goal;
	TASK_PTR task;
	TASKTYPE taskType;

	if (!self || SIDEKICK_NeuralScriptEventOwnsAI(self))
		return;
	hook = AI_GetPlayerHook(self);
	if (!hook || !AI_IsSidekick(hook) || !self->enemy || !AI_IsAlive(self->enemy))
		return;
	if (hook->ai_flags & (AI_STAY | AI_NOATTACK))
		return;
	if (!SIDEKICK_IsOkToFight(self) || SIDEKICK_NeuralHasScriptCriticalTask(self))
		return;

	state = SIDEKICK_SmartGetState(self);
	/* Combat outranks autonomous rejoin/exploration. */
	if (state->advancedExploreActive)
		SIDEKICK_AdvancedCancelExplore(self, FALSE);
	else if (state->advancedRouteActive)
		SIDEKICK_AdvancedCancelRoute(self, FALSE);

	goalStack = AI_GetCurrentGoalStack(hook);
	goal = goalStack ? GOALSTACK_GetCurrentGoal(goalStack) : NULL;
	task = goal ? GOAL_GetCurrentTask(goal) : NULL;
	taskType = task ? TASK_GetType(task) : TASKTYPE_IDLE;

	if (task && SIDEKICK_NeuralTaskIsScriptCritical(taskType))
		return;

	/* Only interrupt autonomous movement/ambient work.  Original scripted
	 * traversal remains authoritative. */
	if (task && !(SIDEKICK_NeuralIsAmbientTask(taskType) ||
		taskType == TASKTYPE_FOLLOWRUNNING ||
		taskType == TASKTYPE_FOLLOWWALKING ||
		taskType == TASKTYPE_MOVETOOWNER ||
		taskType == TASKTYPE_MOVETOLOCATION ||
		taskType == TASKTYPE_WAITFORNOCOLLISION ||
		taskType == TASKTYPE_SIDEKICK_STOP ||
		taskType == TASKTYPE_IDLE))
		return;

	hook->ai_flags |= AI_SIDEKICKKILL;
	if (!SIDEKICK_HasTaskInQue(self, TASKTYPE_SIDEKICK_ATTACK))
		AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_ATTACK);
}

static void SIDEKICK_AdvancedThink(userEntity_t* self)
{
	playerHook_t* hook;
	sidekick_smart_state_t* state;
	CVector targetPoint;
	int targetNumber;
	float ownerDistance;

	if (!self || (gstate->GetCvar("dk_advanced_reasoning") <= 0.0f &&
		gstate->GetCvar("dk_neural_ai") <= 0.0f))
		return;
	if (SIDEKICK_NeuralScriptEventOwnsAI(self))
	{
		state = SIDEKICK_SmartGetState(self);
		state->neuralEnhancementsSuspended = TRUE;
		state->advancedExploreActive = FALSE;
		state->advancedRouteActive = FALSE;
		state->advancedRouteMode = 0;
		state->advancedRouteActionCount = 0;
		state->advancedRouteNextAction = 0;
		return;
	}
	hook = AI_GetPlayerHook(self);
	if (!hook || !AI_IsSidekick(hook))
		return;
	state = SIDEKICK_SmartGetState(self);
	state->neuralEnhancementsSuspended = FALSE;

	SIDEKICK_AdvancedRefreshLevelKnowledge();

	/* A deterministic route is sacred: the neural layer must not replace it. */
	if (state->advancedRouteActive && !state->advancedExploreActive)
	{
		if (!SIDEKICK_AdvancedCanRejoin(self))
		{
			SIDEKICK_AdvancedCancelRoute(self, TRUE);
			return;
		}

		ownerDistance = VectorDistance(self->s.origin, hook->owner->s.origin);
		if (ownerDistance <= SIDEKICK_ADVANCED_REJOIN_DONE_DISTANCE &&
			!SIDEKICK_NeuralHasScriptCriticalTask(self))
		{
			SIDEKICK_AdvancedCancelRoute(self, FALSE);
			return;
		}

		if (SIDEKICK_AdvancedRouteNeedsReplan(self))
		{
			if (!SIDEKICK_AdvancedStartRoute(self, hook->owner->s.origin, SIDEKICK_ADVANCED_ROUTE_REJOIN))
				state->advancedRouteNextReplanTime = gstate->time + 1.0f;
		}
		return;
	}

	if (state->advancedExploreActive)
	{
		if (gstate->numSecretsFound > state->advancedExploreSecretCount ||
			!SIDEKICK_AdvancedCanExplore(self) ||
			VectorDistance(self->s.origin, hook->owner->s.origin) > SIDEKICK_ADVANCED_REJOIN_DISTANCE)
		{
			SIDEKICK_AdvancedCancelExplore(self, TRUE);
		}
		return;
	}

	/* First priority when Advanced Reasoning is free: never remain stranded. */
	if (SIDEKICK_AdvancedCanRejoin(self))
	{
		ownerDistance = VectorDistance(self->s.origin, hook->owner->s.origin);
		if (ownerDistance > SIDEKICK_ADVANCED_REJOIN_TRIGGER ||
			fabs(self->s.origin.z - hook->owner->s.origin.z) > 128.0f)
		{
			if (ownerDistance <= SIDEKICK_ADVANCED_REJOIN_MAX_DISTANCE &&
				SIDEKICK_AdvancedStartRoute(self, hook->owner->s.origin, SIDEKICK_ADVANCED_ROUTE_REJOIN))
			{
				return;
			}
		}
	}

	if (gstate->numSecretsFound >= gstate->numSecrets ||
		gstate->time < state->advancedExploreCooldown ||
		!SIDEKICK_AdvancedCanExplore(self))
		return;

	if (SIDEKICK_AdvancedFindSecretTarget(self, targetPoint, targetNumber))
		SIDEKICK_AdvancedStartExplore(self, targetPoint, targetNumber);
}

static qboolean SIDEKICK_NeuralShouldHold(userEntity_t* self)
{
	playerHook_t* hook;
	userEntity_t* owner;

	if (!self)
		return FALSE;

	if (gstate->GetCvar("dk_neural_ai") <= 0.0f &&
		gstate->GetCvar("dk_advanced_reasoning") <= 0.0f)
		return FALSE;

	hook = AI_GetPlayerHook(self);
	if (!hook || !AI_IsSidekick(hook))
		return FALSE;

	if (SIDEKICK_SmartGetState(self)->advancedExploreActive ||
		SIDEKICK_SmartGetState(self)->advancedRouteActive)
		return FALSE;

	owner = hook->owner;
	if (!owner || !AI_IsAlive(owner))
		return FALSE;

	if (self->enemy && AI_IsAlive(self->enemy))
		return FALSE;

	/* Do not invent movement when the player is stationary and already close. */
	if (VectorDistance(self->s.origin, owner->s.origin) > 256.0f)
		return FALSE;

	if (owner->velocity.Length() > 20.0f)
		return FALSE;

	return TRUE;
}

static qboolean SIDEKICK_NeuralShouldSuppressAmbients(userEntity_t* self)
{
	if (SIDEKICK_NeuralScriptEventOwnsAI(self))
		return TRUE;

	playerHook_t* hook;
	userEntity_t* owner;
	float distance;

	if (!self)
		return FALSE;

	hook = AI_GetPlayerHook(self);
	if (!hook || !AI_IsSidekick(hook))
		return FALSE;

	owner = hook->owner;
	if (!owner || !AI_IsAlive(owner))
		return FALSE;

	if (self->enemy && AI_IsAlive(self->enemy))
		return FALSE;

	if (owner->velocity.Length() > 20.0f)
		return FALSE;

	distance = VectorDistance(self->s.origin, owner->s.origin);
	return distance <= 512.0f;
}

static int SIDEKICK_NeuralIsAmbientTask(TASKTYPE taskType)
{
	switch (taskType)
	{
	case TASKTYPE_SIDEKICK_RANDOMWALK:
	case TASKTYPE_SIDEKICK_LOOKAROUND:
	case TASKTYPE_SIDEKICK_LOOKUP:
	case TASKTYPE_SIDEKICK_LOOKDOWN:
	case TASKTYPE_SIDEKICK_SAYSOMETHING:
	case TASKTYPE_SIDEKICK_KICKSOMETHING:
	case TASKTYPE_SIDEKICK_WHISTLE:
	case TASKTYPE_SIDEKICK_ANIMATE:
		return TRUE;
	default:
		return FALSE;
	}
}

/*
 * Script/cinematic execution owns the sidekick completely. Check the
 * dedicated script stack directly because ai_think.cpp can populate it one
 * frame before bInScriptMode becomes TRUE.
 */
static qboolean SIDEKICK_NeuralScriptEventOwnsAI(userEntity_t* self)
{
	playerHook_t* hook;
	GOAL_PTR scriptGoal;
	TASK_PTR scriptTask;

	if (!self || !gstate)
		return FALSE;
	if (gstate->bCinematicPlaying || (self->flags & FL_CINEMATIC))
		return TRUE;

	hook = AI_GetPlayerHook(self);
	if (!hook)
		return FALSE;
	if (hook->bInScriptMode)
		return TRUE;
	if (!hook->pScriptGoals)
		return FALSE;

	scriptGoal = GOALSTACK_GetCurrentGoal(hook->pScriptGoals);
	if (!scriptGoal)
		return FALSE;
	scriptTask = GOAL_GetCurrentTask(scriptGoal);
	if (scriptTask)
		return TRUE;

	/* A live non-idle script goal remains engine-owned while the original AI
	   decides whether to populate or remove its next task. */
	return GOAL_GetType(scriptGoal) != GOALTYPE_NONE &&
		GOAL_GetType(scriptGoal) != GOALTYPE_IDLE;
}

/*
 * Script/mission tasks are an engine contract.  The neural layer may improve
 * tactical choices around them, but it must never pre-empt a task whose
 * completion can be required for level progression.  In particular this
 * protects USEITEM/EXACTLOCATION/SEQUENCE/TELEPORT/LADDER work and the
 * original AI_HandleUse() path used for switches, levers, buttons, movers
 * and other level mechanics.
 */
static qboolean SIDEKICK_NeuralTaskIsScriptCritical(TASKTYPE taskType)
{
	switch (taskType)
	{
	case TASKTYPE_MOVETOEXACTLOCATION:
	case TASKTYPE_MOVETOENTITYUNTILVISIBLE:
	case TASKTYPE_BACKUPALLATTRIBUTES:
	case TASKTYPE_RESTOREALLATTRIBUTES:
	case TASKTYPE_MODIFYTURNATTRIBUTE:
	case TASKTYPE_MODIFYRUNATTRIBUTE:
	case TASKTYPE_MODIFYWALKATTRIBUTE:
	case TASKTYPE_SEQUENCETRANSITION:
	case TASKTYPE_ACTIVATESWITCH:
	case TASKTYPE_USE:
	case TASKTYPE_USEPLATFORM:
	case TASKTYPE_SWITCH:
	case TASKTYPE_JUMPUP:
	case TASKTYPE_JUMPFORWARD:
	case TASKTYPE_JUMPTOWARDPOINT:
	case TASKTYPE_SHOTCYCLERJUMP:
	case TASKTYPE_WAITFORTRAINTOCOME:
	case TASKTYPE_WAITFORTRAINTOSTOP:
	case TASKTYPE_WAITFORPLATFORMTOCOME:
	case TASKTYPE_WAITFORPLATFORMTOSTOP:
	case TASKTYPE_MOVEINTOPLATFORM:
	case TASKTYPE_MOVEOUTOFPLATFORM:
	case TASKTYPE_WAITFORDOORTOOPEN:
	case TASKTYPE_WAITFORDOORTOCLOSE:
	case TASKTYPE_SIDEKICK_USEITEM:
	case TASKTYPE_SIDEKICK_PICKUPITEM:
	case TASKTYPE_SIDEKICK_COMEHERE:
	case TASKTYPE_SIDEKICK_TELEPORT:
	case TASKTYPE_SIDEKICK_TELEPORTANDCOMENEAR:
	case TASKTYPE_SIDEKICK_DIE:
	case TASKTYPE_PLAYANIMATION:
	case TASKTYPE_SETIDLEANIMATION:
	case TASKTYPE_PLAYANIMATIONANDSOUND:
	case TASKTYPE_PLAYSOUND:
	case TASKTYPE_FINISHCURRENTSEQUENCE:
	case TASKTYPE_UPLADDER:
	case TASKTYPE_DOWNLADDER:
	case TASKTYPE_WAITONLADDERFORNOCOLLISION:
	case TASKTYPE_WAITFORNOCOLLISION:
	case TASKTYPE_ACTION_PLAYANIMATION:
	case TASKTYPE_ACTION_PLAYPARTIALANIMATION:
	case TASKTYPE_ACTION_PLAYSOUND:
	case TASKTYPE_ACTION_SPAWN:
	case TASKTYPE_ACTION_SETMOVINGANIMATION:
	case TASKTYPE_ACTION_SETSTATE:
	case TASKTYPE_ACTION_DIE:
	case TASKTYPE_ACTION_STREAMSOUND:
	case TASKTYPE_ACTION_SENDMESSAGE:
	case TASKTYPE_ACTION_SENDURGENTMESSAGE:
	case TASKTYPE_ACTION_CALL:
	case TASKTYPE_ACTION_RANDOMSCRIPT:
	case TASKTYPE_ACTION_COMENEAR:
	case TASKTYPE_ACTION_REMOVE:
	case TASKTYPE_ACTION_LOOKAT:
	case TASKTYPE_ACTION_STOPLOOK:
	case TASKTYPE_ACTION_ATTACK:
	case TASKTYPE_ACTION_PRINT:
	case TASKTYPE_WAITUNTILNOOBSTRUCTION:
	case TASKTYPE_MOVETORETREATNODE:
	case TASKTYPE_STOPENTITY:
		return TRUE;
	default:
		return FALSE;
	}
}

static qboolean SIDEKICK_NeuralHasScriptCriticalTask(userEntity_t* self)
{
	playerHook_t* hook;
	GOALSTACK_PTR goalStack;
	GOAL_PTR currentGoal;
	TASK_PTR currentTask;

	if (!self)
		return FALSE;
	if (SIDEKICK_NeuralScriptEventOwnsAI(self))
		return TRUE;

	hook = AI_GetPlayerHook(self);
	if (!hook)
		return FALSE;

	goalStack = AI_GetCurrentGoalStack(hook);
	currentGoal = goalStack ? GOALSTACK_GetCurrentGoal(goalStack) : NULL;
	currentTask = goalStack ? GOALSTACK_GetCurrentTask(goalStack) : NULL;

	if (currentGoal)
	{
		GOALTYPE goalType = GOAL_GetType(currentGoal);
		if (goalType == GOALTYPE_SCRIPTACTION ||
			goalType == GOALTYPE_WHENUSED_SCRIPTACTION ||
			goalType == GOALTYPE_BACKUPALLATTRIBUTES ||
			goalType == GOALTYPE_RESTOREALLATTRIBUTES ||
			goalType == GOALTYPE_MODIFYTURNATTRIBUTE ||
			goalType == GOALTYPE_MODIFYRUNATTRIBUTE ||
			goalType == GOALTYPE_MODIFYWALKATTRIBUTE ||
			goalType == GOALTYPE_STAY ||
			goalType == GOALTYPE_SIDEKICK_WAITHERE ||
			goalType == GOALTYPE_SIDEKICK_TELEPORTANDCOMENEAR ||
			goalType == GOALTYPE_USE ||
			goalType == GOALTYPE_GENERALUSE ||
			goalType == GOALTYPE_MOVETOEXACTLOCATION ||
			goalType == GOALTYPE_MOVETOENTITY ||
			goalType == GOALTYPE_PLAYANIMATION ||
			goalType == GOALTYPE_SETIDLEANIMATION ||
			goalType == GOALTYPE_PLAYSOUNDTOEND)
			return TRUE;
	}

	return currentTask ? SIDEKICK_NeuralTaskIsScriptCritical(TASK_GetType(currentTask)) : FALSE;
}

static void SIDEKICK_NeuralEnforceIdle(userEntity_t* self)
{
	playerHook_t* hook;
	GOALSTACK_PTR goalStack;
	GOAL_PTR currentGoal;
	TASK_PTR currentTask;

	if (SIDEKICK_NeuralScriptEventOwnsAI(self))
		return;
	if (!SIDEKICK_NeuralShouldHold(self))
		return;

	hook = AI_GetPlayerHook(self);
	if (!hook)
		return;

	goalStack = AI_GetCurrentGoalStack(hook);
	currentGoal = goalStack ? GOALSTACK_GetCurrentGoal(goalStack) : NULL;

	if (currentGoal)
	{
		GOALTYPE goalType = GOAL_GetType(currentGoal);
		currentTask = GOAL_GetCurrentTask(currentGoal);

		if (SIDEKICK_NeuralIsAmbientTask(
			currentTask ? TASK_GetType(currentTask) : TASKTYPE_IDLE) ||
			(currentTask &&
				(TASK_GetType(currentTask) == TASKTYPE_FOLLOWRUNNING ||
					TASK_GetType(currentTask) == TASKTYPE_FOLLOWWALKING ||
					TASK_GetType(currentTask) == TASKTYPE_MOVETOOWNER)))
		{
			GOAL_ClearTasks(currentGoal);

			if (SIDEKICK_NeuralShouldHold(self))
				AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_STOP);
			else
				AI_AddNewTaskAtFront(self, TASKTYPE_MOVETOOWNER);
		}
		else if (SIDEKICK_NeuralShouldHold(self) &&
			(goalType == GOALTYPE_FOLLOW || goalType == GOALTYPE_IDLE) &&
			(!currentTask ||
				SIDEKICK_NeuralIsAmbientTask(TASK_GetType(currentTask)) ||
				TASK_GetType(currentTask) == TASKTYPE_FOLLOWRUNNING ||
				TASK_GetType(currentTask) == TASKTYPE_FOLLOWWALKING ||
				TASK_GetType(currentTask) == TASKTYPE_MOVETOOWNER))
		{
			if (!currentTask || TASK_GetType(currentTask) != TASKTYPE_SIDEKICK_STOP)
			{
				GOAL_ClearTasks(currentGoal);
				AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_STOP);
			}
		}
	}

	AI_ZeroVelocity(self);
}

static void SIDEKICK_NeuralApplyAction(userEntity_t* self, int action)
{
	playerHook_t* hook;
	userEntity_t* owner;

	if (!self)
		return;
	if (SIDEKICK_NeuralScriptEventOwnsAI(self))
		return;

	hook = AI_GetPlayerHook(self);
	if (!hook || !AI_IsSidekick(hook))
		return;

	/* Never pre-empt a mission/level-critical original-AI task. */
	if (SIDEKICK_NeuralHasScriptCriticalTask(self))
		return;

	/* Player-issued stay/no-attack orders remain authoritative. */
	if (hook->ai_flags & AI_STAY)
		return;
	if ((hook->ai_flags & AI_NOATTACK) && action == DK_NEURAL_ACTION_ATTACK_TARGET)
		return;

	owner = hook->owner;

	switch (action)
	{
	case DK_NEURAL_ACTION_ATTACK_TARGET:
		if (self->enemy && AI_IsAlive(self->enemy) && SIDEKICK_IsOkToFight(self))
		{
			hook->ai_flags |= AI_SIDEKICKKILL;
			if (!SIDEKICK_HasTaskInQue(self, TASKTYPE_SIDEKICK_ATTACK))
				AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_ATTACK);
		}
		break;

	case DK_NEURAL_ACTION_FLANK_LEFT:
		if (self->enemy && AI_IsAlive(self->enemy) &&
			!SIDEKICK_HasTaskInQue(self, TASKTYPE_CHASESIDESTEPLEFT))
			AI_AddNewTaskAtFront(self, TASKTYPE_CHASESIDESTEPLEFT);
		break;

	case DK_NEURAL_ACTION_FLANK_RIGHT:
		if (self->enemy && AI_IsAlive(self->enemy) &&
			!SIDEKICK_HasTaskInQue(self, TASKTYPE_CHASESIDESTEPRIGHT))
			AI_AddNewTaskAtFront(self, TASKTYPE_CHASESIDESTEPRIGHT);
		break;

	case DK_NEURAL_ACTION_RETREAT:
		if (self->enemy && AI_IsAlive(self->enemy))
		{
			if (!SIDEKICK_HasTaskInQue(self, TASKTYPE_MOVEAWAY))
				AI_AddNewTaskAtFront(self, TASKTYPE_MOVEAWAY, self->enemy);
		}
		else if (owner && AI_IsAlive(owner) && !SIDEKICK_HasTaskInQue(self, TASKTYPE_MOVETOOWNER))
		{
			AI_AddNewTaskAtFront(self, TASKTYPE_MOVETOOWNER);
		}
		break;

	case DK_NEURAL_ACTION_FOLLOW_OWNER:
		if (owner && AI_IsAlive(owner))
		{
			/* Refresh the deterministic path whenever follow is selected.  This
			 * prevents a stale MOVETOOWNER path from repeatedly driving into an
			 * old wall after the player has changed rooms/height. */
			AI_FindPathToPoint(self, owner->s.origin);
			if (!SIDEKICK_HasTaskInQue(self, TASKTYPE_MOVETOOWNER))
				AI_AddNewTaskAtFront(self, TASKTYPE_MOVETOOWNER);
		}
		break;

	case DK_NEURAL_ACTION_TAKE_COVER:
		if (self->enemy && AI_IsAlive(self->enemy))
		{
			if (!SIDEKICK_HasTaskInQue(self, TASKTYPE_MOVEAWAY))
				AI_AddNewTaskAtFront(self, TASKTYPE_MOVEAWAY, self->enemy);
		}
		else if (owner && AI_IsAlive(owner) && !SIDEKICK_HasTaskInQue(self, TASKTYPE_MOVETOOWNER))
		{
			AI_AddNewTaskAtFront(self, TASKTYPE_MOVETOOWNER);
		}
		break;

	case DK_NEURAL_ACTION_HOLD:
		if (!SIDEKICK_HasTaskInQue(self, TASKTYPE_SIDEKICK_STOP))
			AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_STOP);
		break;
	}
}

static void SIDEKICK_NeuralThink(userEntity_t* self)
{
	playerHook_t* hook;
	sidekick_smart_state_t* state;
	sidekick_neural_decision_fn decisionFn;
	dk_neural_tactical_state_t tacticalState;
	int action;

	if (!self)
		return;
	if (SIDEKICK_NeuralScriptEventOwnsAI(self))
		return;

	hook = AI_GetPlayerHook(self);
	if (!hook || !AI_IsSidekick(hook))
		return;
	if (!AI_IsMikiko(hook) && !AI_IsSuperfly(hook))
		return;
	if (gstate->GetCvar("dk_neural_ai") <= 0.0f &&
		gstate->GetCvar("dk_advanced_reasoning") <= 0.0f)
		return;
	if (hook->ai_flags & AI_STAY)
		return;
	if (gstate->bCinematicPlaying)
		return;

	/* Original AI/script tasks have priority over neural tactical choices. */
	if (SIDEKICK_NeuralHasScriptCriticalTask(self))
		return;

	state = SIDEKICK_SmartGetState(self);
	if (state->advancedExploreActive || state->advancedRouteActive)
		return;
	if (gstate->time < state->nextNeuralDecisionTime)
		return;

	decisionFn = SIDEKICK_NeuralGetDecisionFunction();
	if (!decisionFn)
	{
		state->nextNeuralDecisionTime = gstate->time + 5.0f;
		return;
	}

	SIDEKICK_NeuralBuildState(self, &tacticalState);
	if (SIDEKICK_NeuralShouldHold(self))
	{
		action = DK_NEURAL_ACTION_HOLD;
	}
	else if (hook->owner && AI_IsAlive(hook->owner) &&
		!self->enemy &&
		(VectorDistance(self->s.origin, hook->owner->s.origin) > 384.0f ||
			hook->owner->velocity.Length() > 80.0f))
	{
		/* The player is the squad's anchor.  Neural exploration is allowed only
		 * while the sidekick remains reasonably close; once separation grows,
		 * following/rejoining is mandatory. */
		action = DK_NEURAL_ACTION_FOLLOW_OWNER;
	}
	else if (!decisionFn(&tacticalState, &action) || action < DK_NEURAL_ACTION_FOLLOW_OWNER || action > DK_NEURAL_ACTION_HOLD)
	{
		state->nextNeuralDecisionTime = gstate->time + 2.0f;
		return;
	}

	state->lastNeuralAction = action;
	state->lastNeuralTarget = self->enemy ? self->enemy->s.number : -1;
	Com_sprintf(state->lastNeuralActionName, sizeof(state->lastNeuralActionName), "%s",
		sidekickNeuralActionName(action));

	{
		float interval = gstate->GetCvar("dk_neural_sidekick_interval");
		if (interval < 0.25f)
			interval = 0.25f;
		if (interval > 10.0f)
			interval = 10.0f;
		state->nextNeuralDecisionTime = gstate->time + interval;
	}

	if (gstate->GetCvar("dk_neural_debug") > 0.0f)
	{
		gstate->Con_Printf("Tactical AI: %s -> ACTION=%s\n",
			AI_IsMikiko(hook) ? "Mikiko" : "Superfly", sidekickNeuralActionName(action));
	}

	SIDEKICK_NeuralApplyAction(self, action);
}

// ----------------------------------------------------------------------------

#define E1_DISRUPTOR    0
#define E1_IONBLASTER	1
#define E1_SHOTCYCLER	2
#define E1_NUMWEAPONS   3

#define E2_DISCUS       0
#define E2_VENOMOUS     1
#define E2_TRIDENT		2
#define E2_NUMWEAPONS   3

#define E3_SILVERCLAW   0
#define E3_BOLTER       1
#define E3_BALLISTA     2
#define E3_NUMWEAPONS   3

#define E4_GLOCK        0
#define E4_RIPGUN       1
#define E4_SLUGGER      2
#define E4_NUMWEAPONS   3


// NSS[12/5/99]:Maximum nuimber of messages to hold in the que 
#define MAX_SIDEKICK_MESSAGES 128


//NSS[12/4/99]:Communication buffers for sidekicks
SideKickCom_t SuperflyCom[MAX_SIDEKICK_MESSAGES];
SideKickCom_t MikikoCom[MAX_SIDEKICK_MESSAGES];






//NSS[12/4/99]:Had to create a cross reference table because someone had no fore sight and obviously didn't understand naming conventions.
SideMp3_t SuperFlyVoices[] =
{
	{4,0,HEALTH_RESPONSE1,	"sid_s_01.mp3"},
	{4,0,HEALTH_RESPONSE2,	"sid_s_02.mp3"},
	{4,0,HEALTH_RESPONSE3,	"sid_s_03.mp3"},
	{4,0,HEALTH_RESPONSE4,	"sid_s_04.mp3"},
	{4,0,HEALTH_RESPONSE5,	"sid_s_05.mp3"},
	{4,0,HIRO_HEALTH_G50,		"sid_s_06.mp3"},
	{4,0,HIRO_HEALTH_L50,		"sid_s_07.mp3"},
	{4,0,HIRO_HEALTH_L15,		"sid_s_08.mp3"},
	{4,0,IMPATIENT,			"sid_s_09.mp3"},
	{4,0,IMPATIENT,			"sid_s_10.mp3"},
	{8,0,IMPATIENT,			"sid_s_amblong_bustskulls1.mp3"},
#ifndef DAIKATANA_DEMO
	{8,0,IMPATIENT,			"sid_s_amblong_geez1.mp3"},
	{8,0,IMPATIENT,			"sid_s_amblong_gottahurry1.mp3"},
	{8,0,IMPATIENT,			"sid_s_amblong_heylets1.mp3"},
	{8,0,IMPATIENT,			"sid_s_amblong_onefoot1.mp3"},
	{8,0,IMPATIENT,			"sid_s_amblong_pizza1.mp3"},
	{20,0,IMPATIENT,			"sid_s_amblong_snore1.mp3"},
	{20,0,IMPATIENT,			"sid_s_amblong_snore2.mp3"},
	{8,0,IMPATIENT,			"sid_s_amblong_thisplace1.mp3"},
	{8,0,IMPATIENT,			"sid_s_amblong_whistle1.mp3"},
#endif
	{4,0,LOOKING_UP,			"sid_s_11.mp3"},
	{4,0,LOOKING_UP,			"sid_s_12.mp3"},
	{4,0,LOOKING_UP,			"sid_s_amb_hahahmm1.mp3"},
#ifndef DAIKATANA_DEMO
	{4,0,LOOKING_UP,			"sid_s_amb_hello1.mp3"},
	{4,0,LOOKING_UP,			"sid_s_amb_hello2.mp3"},
	{4,0,LOOKING_UP,			"sid_s_amb_heyhiro1.mp3"},
#endif
	{4,0,LOOKING_SIDEWAYS,	"sid_s_13.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_s_14.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_s_amb_checkthat1.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_s_amb_cracka1.mp3"},
#ifndef DAIKATANA_DEMO
	{4,0,LOOKING_SIDEWAYS,	"sid_s_amb_hatching1.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_s_amb_whassup1.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_s_amb_whassup2.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_s_amb_whassup3.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_s_amb_thatlooks1.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_s_amb_thatsgross1.mp3"},
#endif
	{6,0,RANDOM_WALK,			"sid_s_amb_gonnawander1.mp3"},
	{6,0,RANDOM_WALK,			"sid_s_amb_heywhats1.mp3"},
	{6,0,RANDOM_WALK,			"sid_s_amb_hirolisten1.mp3"},
	{6,0,RANDOM_WALK,			"sid_s_amb_placerocks1.mp3"},
#ifndef DAIKATANA_DEMO
	{6,0,RANDOM_WALK,			"sid_s_amb_somethings1.mp3"},
	{6,0,RANDOM_WALK,			"sid_s_amb_wanderin1.mp3"},
	{6,0,RANDOM_WALK,			"sid_s_amb_whatdid1.mp3"},
	{6,0,RANDOM_WALK,			"sid_s_amb_whatsgoin1.mp3"},
	{6,0,RANDOM_WALK,			"sid_s_amb_youhear1.mp3"},
#endif
	{4,0,HITBY_HIRO,			"sid_s_15.mp3"},
	{6,0,HITBY_HIRO,			"sid_s_hiroshot_assdust1.mp3"},
	{6,0,HITBY_HIRO,			"sid_s_hiroshot_bustyou1.mp3"},
	{6,0,HITBY_HIRO,			"sid_s_hiroshot_idiotpile1.mp3"},
#ifndef DAIKATANA_DEMO
	{6,0,HITBY_HIRO,			"sid_s_hiroshot_jacking1.mp3"},
	{6,0,HITBY_HIRO,			"sid_s_hiroshot_knobby1.mp3"},
	{6,0,HITBY_HIRO,			"sid_s_hiroshot_punchteeth1.mp3"},
	{6,0,HITBY_HIRO,			"sid_s_hiroshot_smackdown1.mp3"},
	{6,0,HITBY_HIRO,			"sid_s_hiroshot_stupidload1.mp3"},
	{6,0,HITBY_HIRO,			"sid_s_hiroshot_takeeyes1.mp3"},
	{6,0,HITBY_HIRO,			"sid_s_hiroshot_watchit1.mp3"},
#endif
	{4,0,HITBY_SIDEKICK,		"sid_s_16.mp3"},
	{4,0,PUSHEDBY_HIRO,		"sid_s_push_youlike1.mp3"},
	{4,0,PUSHEDBY_HIRO,		"sid_s_push_asktomove1.mp3"},
	{4,0,PUSHEDBY_HIRO,		"sid_s_push_getoffme1.mp3"},
#ifndef DAIKATANA_DEMO
	{4,0,PUSHEDBY_HIRO,		"sid_s_push_heyman1.mp3"},
	{4,0,PUSHEDBY_HIRO,		"sid_s_push_heythats1.mp3"},
#endif
	{4,0,STARTING_ATTACK,		"sid_s_17.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_18.mp3"},
	{4,0,STARTING_ATTACK,		"Super_32.mp3"},
	{4,0,STARTING_ATTACK,		"Super_34.mp3"},
#ifndef DAIKATANA_DEMO
	{4,0,STARTING_ATTACK,		"Super_37.mp3"},
	{4,0,STARTING_ATTACK,		"Super_41.mp3"},
	{4,0,STARTING_ATTACK,		"Super_50.mp3"},
	{4,0,STARTING_ATTACK,		"Super_51.mp3"},
	{4,0,STARTING_ATTACK,		"Super_54.mp3"},
	{4,0,STARTING_ATTACK,		"Super_56.mp3"},
	{4,0,STARTING_ATTACK,		"Super_59.mp3"},
	{4,0,STARTING_ATTACK,		"Super_60.mp3"},
	{4,0,STARTING_ATTACK,		"e1_super_31.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_blanket1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_daddytime1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_gulpit1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_jackedup1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_lemmebust1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_maddaddy1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_pity1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_punktime1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_ruin1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_tearcrap1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_timefor1.mp3"},
#endif
	{8,0,HIRO_DIES,			"sid_s_hirodies_asscapped1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_bitchslapped1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_blowndown1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_busted1.mp3"},
#ifndef DAIKATANA_DEMO
	{8,0,HIRO_DIES,			"sid_s_hirodies_dead1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_demolished1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_goodbye1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_goodbye2.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_goodbye3.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_hiroisdead1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_jawbounce1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_nutskicked1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_skullsplit1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_sucks1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_tongue1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_wander1.mp3"},
#endif
	{4,0,NO_AMMO,				"sid_s_19.mp3"},
	{4,0,NO_AMMO,				"sid_s_20.mp3"},
	{4,0,LOW_HEALTH,			"sid_s_21.mp3"},
	{4,0,LOW_HEALTH,			"sid_s_22.mp3"},
	{4,0,START_CROUCH,		"sid_s_23.mp3"},
	{4,0,START_CROUCH,		"sid_s_24.mp3"},
	{4,0,LOOK_4_HIRO,			"sid_s_25.mp3"},
	{4,0,LOOK_4_HIRO,			"sid_s_26.mp3"},
	{4,0,RUN_AWAY,			"sid_s_27.mp3"},
	{4,0,RUN_AWAY,			"sid_s_28.mp3"},
	{4,0,RUN_AWAY,			"sid_s_hiroruns_backhere1.mp3"},
#ifndef DAIKATANA_DEMO
	{4,0,RUN_AWAY,			"sid_s_hiroruns_hottie1.mp3"},
	{4,0,RUN_AWAY,			"sid_s_hiroruns_laterass1.mp3"},
	{4,0,RUN_AWAY,			"sid_s_hiroruns_runafter1.mp3"},
#endif
	{4,0,QUESTION_HEALTH,		"sid_s_q_01a.mp3"},
	{4,0,ANSWER_1,			"sid_s_29.mp3"},
	{4,0,QUESTION_1,			"sid_s_30.mp3"},
	{4,0,ANSWER_2,			"sid_s_31.mp3"},
	{4,0,PICKUP_ITEM,			"sid_s_32.mp3"},
	{4,0,EVADE,				"death3.wav"},
	{4,0,EVADE,				"death7.wav"},
	{4,0,JUMP,				"jump1.wav"},
	{4,0,JUMP,				"jump2.wav"},
	{4,0,JUMP,				"jump3.wav"},
	{4,0,JUMP,				"jump4.wav"},
	{4,0,JUMP,				"jump5.wav"},
	{4,0,LANDING,				"Land1.wav"},
	{4,0,LANDING,				"Land2.wav"},
	{4,0,LANDING,				"Land3.wav"},
	{4,0,LANDING,				"Land4.wav"},
	{4,0,LANDING,				"Land5.wav"},
	{4,0,NO,					"cmd_su_no_01.mp3"},
	{4,0,NO,					"cmd_su_no_02.mp3"},
	{4,0,NO,					"cmd_su_no_03.mp3"},
	{4,0,CANTREACH,			"cmd_su_pu_01_n2.mp3"},
	{4,0,CANTREACH,			"cmd_su_pu_02_n2.mp3"},
	{4,0,CANTREACH,			"cmd_su_pu_03_n2.mp3"},
	{4,0,NO_TALK,""}
};


SideMp3_t MikikoVoices[] =
{
	{4,0,HEALTH_RESPONSE1,	"sid_m_01.mp3"},
	{4,0,HEALTH_RESPONSE2,	"sid_m_02.mp3"},
	{4,0,HEALTH_RESPONSE3,	"sid_m_03.mp3"},
	{4,0,HEALTH_RESPONSE4,	"sid_m_04.mp3"},
	{4,0,HEALTH_RESPONSE5,	"sid_m_05.mp3"},
	{4,0,HIRO_HEALTH_G50,		"sid_m_06.mp3"},
	{4,0,HIRO_HEALTH_L50,		"sid_m_07.mp3"},
	{4,0,HIRO_HEALTH_L15,		"sid_m_08.mp3"},
	{8,0,IMPATIENT,			"sid_m_09.mp3"},
	{8,0,IMPATIENT,			"sid_m_10.mp3"},
	{8,0,IMPATIENT,			"sid_m_amblong_getgoing1.mp3"},
	{8,0,IMPATIENT,			"sid_m_amblong_gettingold1.mp3"},
#ifndef DAIKATANA_DEMO
	{8,0,IMPATIENT,			"sid_m_amblong_hungry1.mp3"},
	{8,0,IMPATIENT,			"sid_m_amblong_letsmove1.mp3"},
	{8,0,IMPATIENT,			"sid_m_amblong_movealong1.mp3"},
	{8,0,IMPATIENT,			"sid_m_amblong_moving1.mp3"},
	{8,0,IMPATIENT,			"sid_m_amblong_notime1.mp3"},
	{8,0,IMPATIENT,			"sid_m_amblong_thirsty1.mp3"},
	{8,0,IMPATIENT,			"sid_m_amblong_waiting1.mp3"},
#endif
	{4,0,LOOKING_UP,			"sid_m_11.mp3"},
	{4,0,LOOKING_UP,			"sid_m_12.mp3"},
	{4,0,LOOKING_UP,			"sid_m_amb_hearsometh1.mp3"},
	{4,0,LOOKING_UP,			"sid_m_amb_smellthat1.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_m_13.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_m_14.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_m_amb_clockthose1.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_m_amb_feelingshabs1.mp3"},
#ifndef DAIKATANA_DEMO
	{4,0,LOOKING_SIDEWAYS,	"sid_m_amb_hello1.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_m_amb_heyhiro1.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_m_amb_hmm2.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_m_amb_hmm3.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_m_amb_whatshappen1.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_m_amb_whatare1.mp3"},
#endif
	{4,0,RANDOM_WALK,			"sid_m_amb_hmm1.mp3"},
	{4,0,RANDOM_WALK,			"sid_m_amb_interesting1.mp3"},
	{4,0,RANDOM_WALK,			"sid_m_amb_somethingsover1.mp3"},
	{4,0,RANDOM_WALK,			"sid_m_amb_nice1.mp3"},
#ifndef DAIKATANA_DEMO
	{4,0,RANDOM_WALK,			"sid_m_amb_smellinteresting1.mp3"},
	{4,0,RANDOM_WALK,			"sid_m_amb_somethingsmells1.mp3"},
	{4,0,RANDOM_WALK,			"sid_m_amb_whatis1.mp3"},
	{4,0,RANDOM_WALK,			"sid_m_amb_whatsover1.mp3"},
#endif
	{4,0,HITBY_HIRO,			"sid_m_15.mp3"},
	{4,0,HITBY_HIRO,			"sid_m_hiroshot_bunyip1.mp3"},
	{4,0,HITBY_HIRO,			"sid_m_hiroshot_chucku1.mp3"},
	{4,0,HITBY_HIRO,			"sid_m_hiroshot_hobbit1.mp3"},
#ifndef DAIKATANA_DEMO
	{4,0,HITBY_HIRO,			"sid_m_hiroshot_jackedup1.mp3"},
	{4,0,HITBY_HIRO,			"sid_m_hiroshot_mansteak1.mp3"},
	{4,0,HITBY_HIRO,			"sid_m_hiroshot_nooselick1.mp3"},
	{4,0,HITBY_HIRO,			"sid_m_hiroshot_nubmunch1.mp3"},
	{4,0,HITBY_HIRO,			"sid_m_hiroshot_whatdoyou1.mp3"},
#endif
	{4,0,HITBY_SIDEKICK,		"sid_m_16.mp3"},
	{4,0,PUSHEDBY_HIRO,		"sid_m_push_stoppushing1.mp3"},
	{4,0,PUSHEDBY_HIRO,		"sid_m_push_watchit1.mp3"},
	{4,0,PUSHEDBY_HIRO,		"sid_m_push_watchwhere1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_m_17.mp3"},
	{4,0,STARTING_ATTACK,		"sid_m_18.mp3"},
	{4,0,STARTING_ATTACK,		"sid_m_atk_ampedup1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_m_atk_completed1.mp3"},
#ifndef DAIKATANA_DEMO
	{4,0,STARTING_ATTACK,		"sid_m_atk_igotthis1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_m_atk_lemmeatem1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_m_atk_skullcrack1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_m_atk_suckitdown1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_m_atk_tearitdown1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_m_atk_timefora1.mp3"},
#endif
	{4,0,NO_AMMO,				"sid_m_19.mp3"},
	{4,0,NO_AMMO,				"sid_m_20.mp3"},
	{4,0,LOW_HEALTH,			"sid_m_21.mp3"},
	{4,0,LOW_HEALTH,			"sid_m_22.mp3"},
	{4,0,START_CROUCH,		"sid_m_23.mp3"},
	{4,0,START_CROUCH,		"sid_m_24.mp3"},
	{4,0,LOOK_4_HIRO,			"sid_m_25.mp3"},
	{4,0,LOOK_4_HIRO,			"sid_m_26.mp3"},
	{4,0,RUN_AWAY,			"sid_m_27.mp3"},
	{4,0,RUN_AWAY,			"sid_m_28.mp3"},
	{4,0,RUN_AWAY,			"sid_m_hiroruns_alone1.mp3"},
#ifndef DAIKATANA_DEMO
	{4,0,RUN_AWAY,			"sid_m_hiroruns_getback1.mp3"},
	{4,0,RUN_AWAY,			"sid_m_hiroruns_manbeast1.mp3"},
	{4,0,RUN_AWAY,			"sid_m_hiroruns_runningaway1.mp3"},
#endif
	{4,0,PICKUP_ITEM,			"sid_m_29.mp3"},
	{4,0,PICKUP_ITEM,			"sid_m_30.mp3"},
	{4,0,QUESTION_HEALTH,		"sid_m_q_01.mp3"},
	{4,0,QUESTION_1,			"sid_m_q_02.mp3"},
	{4,0,QUESTION_2,			"sid_m_q_04.mp3"},
	{4,0,ANSWER_1,			"sid_m_a_03.mp3"},
	{4,0,ANSWER_2,			"sid_m_a_05.mp3"},
	{4,0,EVADE,				"death2.wav"},
	{4,0,EVADE,				"death8.wav"},
	{4,0,EVADE,				"death7.wav"},
	{4,0,JUMP,				"jump1.wav"},
	{4,0,JUMP,				"jump2.wav"},
	{4,0,JUMP,				"jump3.wav"},
	{4,0,JUMP,				"jump4.wav"},
	{4,0,JUMP,				"jump5.wav"},
	{4,0,LANDING,				"Land1.wav"},
	{4,0,LANDING,				"Land2.wav"},
	{4,0,LANDING,				"Land3.wav"},
	{4,0,LANDING,				"Land4.wav"},
	{4,0,LANDING,				"Land5.wav"},
	{4,0,NO,					"cmd_mi_no_01.mp3"},
	{4,0,NO,					"cmd_mi_no_03.mp3"},
	{4,0,CANTREACH,			"cmd_mi_pu_01_n2.mp3"},
	{4,0,CANTREACH,			"cmd_mi_pu_02_n2.mp3"},
	{4,0,CANTREACH,			"cmd_mi_pu_03_n2.mp3"},
	{4,0,NO_TALK,""}
};


// store the best weapon to use against each monsters
typedef struct sidekickAttackInfo
{
	int nFirstWeaponChoice;
	int nSecondWeaponChoice;
	int nThirdWeaponChoice;

} sidekickAttackInfo_t;

static sidekickAttackInfo_t aSidekickCombatData[MAX_MONSTERS_IN_LIST] =
{
	{ 0,   1,   2 },		//	*EMPTY*				= 0
	{ 0,   1,   2 },		//	TYPE_CLIENT			= 1		
	{ 0,   1,   2 },		//	TYPE_MIKIKO			= 2
	{ 0,   1,   2 },		//	TYPE_SUPERFLY		= 3
	{ 0,   1,   2 },		//	TYPE_BOT			= 4	
	{ 0,   1,   2 },		//	*EMPTY*				= 5
	{ 0,   1,   2 },		//	*EMPTY*				= 6
	{ 0,   1,   2 },		//	*EMPTY*				= 7
	{ 0,   1,   2 },		//	*EMPTY*				= 8
	{ 0,   1,   2 },		//	*EMPTY*				= 9
	//Weapon selection scheme
	// NSS[12/11/99]: Date of last update
	//  Episode 1
	{ 0,   1,   2 },		//	TYPE_ROBOCROC		= 10	
	{ 1,   1,   2 },		//	TYPE_SL_SKEET		= 11
	{ 1,   1,   2 },		//	TYPE_TH_SKEET		= 12
	{ 0,   1,   2 },		//	TYPE_FROGINATOR		= 13
	{ 0,   1,   2 },		//	TYPE_VERMIN			= 14
	{ 0,   1,   2 },        //	TYPE_SLUDGEMINION	= 15
	{ 0,   1,   2 },		//  TYPE_SURGEON		= 16
	{ 0,   1,   2 },		//	TYPE_INMATER		= 17
	{ 0,   1,   2 },		//	TYPE_PRISONER		= 18
	{ 0,   1,   2 },		//	TYPE_BATTLEBOAR		= 19
	{ 0,   1,   2 },		//	TYPE_RAGEMASTER		= 20
	{ 0,   1,   2 },		//	TYPE_TRACKATTACK	= 21
	{ 0,   1,   2 },        //  TYPE_MISHIMAGUARD   = 22
	{ 1,   1,   2 },		//	TYPE_LASERGAT		= 23
	{ 1,   1,   2 },		//	TYPE_CAMBOT			= 24
	{ 0,   1,   2 },		//	TYPE_PSYCLAW		= 25
	{ 1,   1,   2 },		//	TYPE_DEATHSPHERE	= 26
	{ 0,   1,   2 },		//	TYPE_PROTOPOD		= 27
	{ 0,   0,   0 },		//	TYPE_SKINNYWORKER	= 28
	{ 0,   0,   0 },		//	TYPE_FATWORKER	    = 29

	//  Episode 2 // NSS[12/11/99]:Only Two weapons
	{ 0,   1,   2 },		//	TYPE_FERRYMAN		= 30	
	{ 0,   1,   2 },		//	TYPE_SKELETON		= 31
	{ 0,   1,   2 },		//	TYPE_SPIDER			= 32
	{ 0,   1,   2 },		//	TYPE_THIEF			= 33
	{ 0,   1,   2 },		//	TYPE_SATYR			= 34
	{ 0,   1,   2 },		//	TYPE_CENTURION		= 35
	{ 0,   1,   2 },		//	TYPE_HARPY			= 36
	{ 0,   0,   0 },		//	TYPE_COLUMN			= 37
	{ 0,   1,   2 },		//	TYPE_SIREN			= 38
	{ 0,   1,   2 },		//	TYPE_GRIFFON		= 39
	{ 0,   1,   2 },		//	TYPE_CERBERUS		= 40
	{ 0,   1,   2 },		//	TYPE_MEDUSA			= 41
	{ 0,   1,   2 },		//	TYPE_CYCLOPS		= 42
	{ 0,   1,   2 },		//	TYPE_MINOTAUR		= 43
	{ 0,   1,   2 },		//	TYPE_SMALLSPIDER    = 44
	{ 0,   1,   2 },		//	TYPE_KMINOS			= 45
	{ 0,   1,   2 },		//	*EMPTY*				= 46
	{ 0,   1,   2 },		//	*EMPTY*				= 47
	{ 0,   1,   2 },		//	*EMPTY*				= 48
	{ 0,   1,   2 },		//	*EMPTY*				= 49

	//  Episode 3
	{ 0,   1,   2 },		//	TYPE_DRAGONEGG		= 50	
	{ 0,   1,   2 },		//	TYPE_PRIEST			= 51
	{ 0,   1,   1 },		//	TYPE_ROTWORM		= 52
	{ 0,   1,   1 },		//	TYPE_PLAGUERAT		= 53
	{ 0,   1,   2 },		//	TYPE_BUBOID			= 54
	{ 0,   1,   1 },		//	TYPE_DOOMBAT		= 55
	{ 0,   1,   2 },		//	TYPE_FLETCHER		= 56
	{ 0,   1,   2 },		//	TYPE_DWARF			= 57
	{ 0,   0,   0 },		//	TYPE_LYCANTHIR		= 58
	{ 0,   1,   2 },		//	TYPE_CELESTRIL		= 59
	{ 0,   1,   2 },		//	TYPE_WYNDRAX		= 60
	{ 0,   1,   2 },		//	TYPE_BABYDRAGON		= 61
	{ 0,   1,   2 },		//	TYPE_NHARRE			= 62
	{ 0,   1,   2 },		//	TYPE_GARROTH		= 63
	{ 0,   1,   2 },		//	TYPE_KNIGHT1		= 64
	{ 0,   1,   2 },		//	TYPE_KNIGHT2		= 65
	{ 0,   1,   2 },		//	TYPE_WIZARD			= 66
	{ 0,   1,   2 },		//	TYPE_DRAGON			= 67
	{ 0,   1,   2 },		//	TYPE_STAVROS		= 68
	{ 0,   1,   2 },		//	*EMPTY*				= 69

	//  Episode 4
	{ 0,   1,   2 },		//	TYPE_SEAGULL		= 70	
	{ 0,   1,   2 },		//	TYPE_GOLDFISH		= 71
	{ 0,   1,   2 },		//	TYPE_SHARK			= 72
	{ 0,   1,   2 },		//	TYPE_SQUID			= 73
	{ 0,   1,   2 },		//	TYPE_BLACKPRIS		= 74
	{ 0,   1,   2 },		//	TYPE_WHITEPRIS		= 75
	{ 0,   1,   2 },		//	TYPE_LABMONKEY		= 76
	{ 0,   1,   2 },		//	TYPE_FEMGANG		= 77
	{ 0,   1,   2 },		//	TYPE_PIPEGANG		= 78
	{ 0,   1,   2 },		//	TYPE_ROCKETGANG 	= 79
	{ 0,   1,   2 },		//	TYPE_UZIGANG		= 80
	{ 0,   1,   2 },		//	TYPE_CHAINGANG		= 81
	{ 0,   1,   2 },		//	TYPE_SDIVER			= 82
	{ 0,   1,   2 },		//	TYPE_CRYOTECH		= 83
	{ 0,   1,   2 },		//	TYPE_NAVYSEAL		= 84
	{ 0,   1,   2 },		//	TYPE_SEALGIRL		= 85
	{ 0,   1,   2 },		//	TYPE_SEALCOMMANDO	= 86
	{ 0,   1,   2 },		//	TYPE_SEALCAPTAIN	= 87
	{ 0,   1,   2 },		//	TYPE_ROCKETMP		= 88
	{ 0,   1,   2 },		//	TYPE_KAGE			= 89
};


//#define INDEX_COMMAND_YES           0
//#define INDEX_COMMAND_NO            1
//#define INDEX_COMMAND_STAY          2
//#define INDEX_COMMAND_FOLLOW        3
//#define INDEX_COMMAND_ATTACK        4
//#define INDEX_COMMAND_NOATTACK      5
//#define INDEX_COMMAND_COMEHERE      6
//#define INDEX_COMMAND_PICKUP        7
//#define INDEX_COMMAND_LAST          8

#define SIDEKICK_COMMAND_YES            1
#define SIDEKICK_COMMAND_NO             2
#define SIDEKICK_COMMAND_STAY           3
#define SIDEKICK_COMMAND_FOLLOW         4
#define SIDEKICK_COMMAND_ATTACK         5
#define SIDEKICK_COMMAND_NOATTACK       6
#define SIDEKICK_COMMAND_COMEHERE       7
#define SIDEKICK_COMMAND_PICKUP         8
#define SIDEKICK_COMMAND_PICKUP_BAD     9
#define SIDEKICK_COMMAND_PICKUP_NO		 10
#define SIDEKICK_COMMAND_COMMAND       11
#define SIDEKICK_COMMAND_LAST          12

/*
static char *aSidekickCommandString[SIDEKICK_COMMAND_LAST][2] =
{
		// command    filename substring
	{ "",         ""          },
	{ "yes",      "yes"       },
	{ "no",       "no"        },
	{ "stay",     "stay"      },
	{ "follow",   "follow"    },
	{ "attack",   "attack"    },
	{ "noattack", "noattack", },
	{ "comehere", "come"      },
	{ "pickup",   "get"       },
	{ "pickup",   "get_bad"   },
	{ "pickup",   "get_no"    },
	{ "",         "command"   }
};
*/
static char* aSidekickCommandString[SIDEKICK_COMMAND_LAST][2] =
{
	// command    filename substring
{ "",					""          },
{ "yes",				"yes"       },
{ "no",					"no"        },
{ "sidekick_stay",		"stay"      },
{ "follow",				"follow"    },
{ "sidekick_attack",	"attack"    },
{ "sidekick_backoff",	"noattack", },
{ "sidekick_come",		"come"      },
{ "sidekick_get",		"get"       },
{ "pickup",   "get_bad"   },
{ "pickup",   "get_no"    },
{ "",         "command"   }
};

#define MAX_NUMBER_AMBIENTS     8

typedef struct ambientInfo
{
	float fWaitTime;
	float fRandomFactor;

} AMBIENT_INFO, * AMBIENT_INFO_PTR;

static AMBIENT_INFO aAmbientInfo[] =
{
	{ 30.0f,  0.05f },         // TASKTYPE_SIDEKICK_RANDOMWALK,
	{ 25.0f,  0.05f },         // TASKTYPE_SIDEKICK_LOOKAROUND,
	{ 20.0f,  0.05f },         // TASKTYPE_SIDEKICK_LOOKUP,
	{ 20.0f,  0.05f },         // TASKTYPE_SIDEKICK_LOOKDOWN,
	{ 25.0f,  0.05f },         // TASKTYPE_SIDEKICK_SAYSOMETHING,
	{ 40.0f,  0.05f },         // TASKTYPE_SIDEKICK_KICKSOMETHING,
	{ 30.0f,  0.05f },         // TASKTYPE_SIDEKICK_WHISTLE,
	{ 50.0f,  0.05f },         // TASKTYPE_SIDEKICK_ANIMATE,

};

//static int nNumAmbients = sizeof(aAmbientInfo) / sizeof(aAmbientInfo[0]);

static float aNextAmbientTimes[2][MAX_NUMBER_AMBIENTS] =
{
	{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },

};


// ****************************************************************************
// stuff to notify the player of the sidekicks' fighting status!
#define MK_NO_UPDATE			0x0f
#define MK_NO_WEAPON			0
#define MK_LOW_AMMO				1		//
#define MK_GOOD_AMMO			2		
#define MK_ROCKIN_AMMO			3

#define SF_NO_UPDATE			0xf0
#define SF_NO_WEAPON			0
#define SF_LOW_AMMO				16
#define SF_GOOD_AMMO			32
#define SF_ROCKIN_AMMO			48

static byte saveFlags = 0x55;			// 01010101


void SIDEKICK_UpdateWeaponStatus(userEntity_t* player, byte mk_flags, byte sf_flags)
{
	if (coop->value || deathmatch->value)
		return;

	if ((mk_flags | sf_flags) == 0xff)
		return;

	byte flags = 0;
	if (mk_flags != MK_NO_UPDATE)
	{
		flags |= mk_flags;
	}
	else
	{
		flags |= saveFlags & 0x0f;
	}

	if (sf_flags != SF_NO_UPDATE)
	{
		flags |= sf_flags;
	}
	else
	{
		flags |= saveFlags & 0xf0;
	}

	if (flags != saveFlags)
	{
		if (!gstate->gi->TestUnicast(player, true))
			return;

		AI_Dprintf("Update Sidekick Ammo Status!\n");
		saveFlags = flags;
		gstate->WriteByte(SVC_SIDEKICK_WEAPON_STATUS);
		gstate->WriteByte(flags);
		gstate->UniCast(player, true);
	}
}
/* ***************************** Local Functions *************************** */

int SIDEKICK_HandleFollowing(userEntity_t* self);
userEntity_t* SIDEKICK_FindClosestEnemy(userEntity_t* self);
void SIDEKICK_FreeInventory(userEntity_t* self);
void SIDEKICK_DropInventory(userEntity_t* self);
int AI_HasGoalInQueue(userEntity_t* self, GOALTYPE SearchGoal);
void SIDEKICK_InitWeapons(userEntity_t* self);
static void SIDEKICK_Init(userEntity_t* self, int nType);
void SIDEKICK_PreThink(userEntity_t* self);
void SIDEKICK_PostThink(userEntity_t* self);
void SIDEKICK_Remove(userEntity_t* self);
void SIDEKICK_StartDie(userEntity_t* self, userEntity_t* inflictor,
	userEntity_t* attacker, int damage, CVector& point);
void SIDEKICK_Pain(userEntity_t* self, userEntity_t* other, float kick, int damage);
void SIDEKICK_Attack(userEntity_t* self);
void SIDEKICK_BeginAttack(userEntity_t* self);
void SIDEKICK_Start(userEntity_t* self, int nType);
userEntity_t* SIDEKICK_Spawn(userEntity_t* self, int nType);
int SIDEKICK_FindClearSpotFrom(userEntity_t* self, CVector& fromPoint, CVector& clearPoint);

int SIDEKICK_AttackOwnerFacingEnemy(userEntity_t* self);

userEntity_t* SIDEKICK_FindEnemy(userEntity_t* self);
void SIDEKICK_FindOwnerThink(userEntity_t* self);

void SIDEKICK_CommandAcknowledge(userEntity_t* self, int command_flag, qboolean PVS_required = false);
void PlaySidekickMP3(userEntity_t* self, char* sound);

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* ******************************* exports ********************************* */
DllExport void SIDEKICK_Alert(userEntity_t* owner, userEntity_t* target);
DllExport void info_superfly_start(userEntity_t* self);
DllExport void info_mikikofly_start(userEntity_t* self); //NSS[11/30/99]:Added for mikikofly mesh mess
DllExport void info_mikiko_start(userEntity_t* self);
DllExport void trigger_mikiko_spawn(userEntity_t* self);
DllExport void trigger_superfly_spawn(userEntity_t* self);

void SIDEKICK_Think(userEntity_t* self);

void SIDEKICK_PermissionGiven(userEntity_t* self);
void SIDEKICK_PermissionDenied(userEntity_t* self);

void SIDEKICK_PickedUpItem(userEntity_t* self, const char* szItemName);

userInventory_t* SIDEKICK_ChooseBestWeaponForE1(userEntity_t* self, userEntity_t* pEnemy = NULL);
userInventory_t* SIDEKICK_ChooseBestWeaponForE2(userEntity_t* self, userEntity_t* pEnemy = NULL);
userInventory_t* SIDEKICK_ChooseBestWeaponForE3(userEntity_t* self, userEntity_t* pEnemy = NULL);
userInventory_t* SIDEKICK_ChooseBestWeaponForE4(userEntity_t* self, userEntity_t* pEnemy = NULL);

int SIDEKICK_MoveTowardPoint(userEntity_t* self, CVector& destPoint,
	int bMovingTowardNode = FALSE, int bFaceTowardDest = TRUE);
int SIDEKICK_MoveTowardPointWhileAttacking(userEntity_t* self, CVector& destPoint,
	int bMovingTowardNode = FALSE, int bFaceTowardDest = TRUE);
int SIDEKICK_ReadAmbientInfo(userEntity_t* self, const char* szFileName);

/*
void FetchSidekickEntData( edict_t *pClientEnt, edict_t *pSidekickEnt, int nType );
*/

int SideKick_TalkAmbient(userEntity_t* self, enum Sidekick_Talk VoiceType, int OverRideTime);
int Check_Que(userEntity_t* other, int Type, float Offset);
int SIDEKICK_SendMessage(userEntity_t* other, int command, int An_Int, float A_Float, float B_Float, userEntity_t* sender, int Flag);

// NSS[1/19/00]:Some defines for functions
int SIDEKICK_IsClearShot(userEntity_t* self, userEntity_t* pEntity, CVector& OrgStart, CVector& OrgEnd);
static qboolean SIDEKICK_IsSquadmateBlockingShot(userEntity_t* self, userEntity_t* pEnemy);
int SIDEKICK_HasTaskInQue(userEntity_t* self, TASKTYPE Task);
int SIDEKICK_OwnerIsTooCloseToEnemy(userEntity_t* self);
int SIDEKICK_FindGoodShootingSpot(userEntity_t* self, userEntity_t* enemy, CVector* Position);

void FetchSidekickEntData(edict_t* pClientEnt, edict_t* pSidekickEnt, int nType);
float SIDEKICK_DeterminePathLength(userEntity_t* self, userEntity_t* target);

// ----------------------------------------------------------------------------
//								LOCAL functions
// ----------------------------------------------------------------------------
#define BOUND_MIN(val,min)		if (val < min) val = min
void AI_SaveSidekick(FILE* f, struct edict_s* ent)
{
	AI_SavePlayerHook(f, ent);

	// SCG[2/24/00]: copy persistant data
	playerHook_t* pHook = AI_GetPlayerHook(ent);
	client_persistant_t* pPersistant = &ent->client->pers;
	pPersistant->nHealth = ent->health;
	pPersistant->record = ent->record;
	pPersistant->nArmorVal = ent->armor_val;
	pPersistant->nArmorAbs = ent->armor_abs;
	pPersistant->nTeam = ent->team;
	pPersistant->nItems = pHook->items;
	pPersistant->nExpFlags = pHook->exp_flags;
	pPersistant->fInvulnerabilityTime = pHook->invulnerability_time;
	pPersistant->fEnvirosuitTime = pHook->envirosuit_time;
	pPersistant->fWraithorbTime = pHook->wraithorb_time;
	pPersistant->fBasePower = pHook->base_power;
	pPersistant->fBaseAttack = pHook->base_attack;
	pPersistant->fBaseSpeed = pHook->base_speed;
	pPersistant->fBaseAcro = pHook->base_acro;
	pPersistant->fBaseVita = pHook->base_vita;
	pPersistant->fBoostPower = pHook->power_boost;
	pPersistant->fBoostAttack = pHook->attack_boost;
	pPersistant->fBoostSpeed = pHook->speed_boost;
	pPersistant->fBoostAcro = pHook->acro_boost;
	pPersistant->fBoostVita = pHook->vita_boost;
	pPersistant->fTimePower = pHook->power_boost_time;
	pPersistant->fTimeAttack = pHook->attack_boost_time;
	pPersistant->fTimeSpeed = pHook->speed_boost_time;
	pPersistant->fTimeAcro = pHook->acro_boost_time;
	pPersistant->fTimeVita = pHook->vita_boost_time;
	pPersistant->vAngles = ent->s.angles;
	pPersistant->nAIFlags = pHook->ai_flags;
	BOUND_MIN(pPersistant->fTimePower, 0);
	BOUND_MIN(pPersistant->fTimeAttack, 0);
	BOUND_MIN(pPersistant->fTimeSpeed, 0);
	BOUND_MIN(pPersistant->fTimeAcro, 0);
	BOUND_MIN(pPersistant->fTimeVita, 0);
	BOUND_MIN(pPersistant->fInvulnerabilityTime, 0);
	BOUND_MIN(pPersistant->fEnvirosuitTime, 0);
	BOUND_MIN(pPersistant->fWraithorbTime, 0);

	com->SavePersInventory(ent);
	memcpy(&ent->client->pers, pPersistant, sizeof(client_persistant_t));

	// SCG[2/24/00]: save client
	com->AI_SaveHook(f, ent->client, sizeof(gclient_s));
}

#define	SIDEKICK_PERS_MIKIKO	2
#define	SIDEKICK_PERS_SUPERFLY	3

void SidekickLoadThink(userEntity_t* self)
{
	playerHook_t* pHook = AI_GetPlayerHook(self);
	userEntity_t* pPlayer = NULL;

	unsigned int	nSidekickType;

	if (pHook->type == TYPE_SUPERFLY)
	{
		nSidekickType = SIDEKICK_PERS_SUPERFLY;
	}
	else if (pHook->type == TYPE_MIKIKO)
	{
		nSidekickType = SIDEKICK_PERS_MIKIKO;
	}

	userEntity_t* pEntity = alist_FirstEntity(client_list);
	while (pEntity != NULL)
	{
		if (pEntity->className != NULL)
		{
			if (_stricmp(pEntity->className, "player") == 0)
			{
				pPlayer = pEntity;
				break;
			}
		}

		pEntity = alist_NextEntity(client_list);
	}

	if ((pHook != NULL) && (pPlayer != NULL))
	{
		pHook->owner = pPlayer;
		if (nSidekickType == SIDEKICK_PERS_MIKIKO)
		{
			if (pPlayer->client->pers.pPersMikiko)
			{
				memcpy(&self->client->pers, pPlayer->client->pers.pPersMikiko, sizeof(client_persistant_t));
			}
			else
			{
				pPlayer->client->pers.pPersMikiko = &self->client->pers;
			}
		}
		else
		{
			if (pPlayer->client->pers.pPersSuperfly)
			{
				memcpy(&self->client->pers, pPlayer->client->pers.pPersSuperfly, sizeof(client_persistant_t));
			}
			else
			{
				pPlayer->client->pers.pPersSuperfly = &self->client->pers;
			}
		}
		FetchSidekickEntData(pPlayer, self, nSidekickType);

		AI_AddNewGoal(self, GOALTYPE_FOLLOW, pHook->owner);

		self->think = SIDEKICK_FindOwnerThink;
		self->nextthink = gstate->time + 0.1f;
	}
	else
	{
		self->think = SidekickLoadThink;
		self->nextthink = gstate->time + 1.0f;
	}
}

void AI_LoadSidekick(FILE* f, struct edict_s* ent)
{
	userEntity_t* pPlayer = NULL;
	userEntity_t* pSuperfly = AIINFO_GetSuperfly();
	userEntity_t* pMikiko = AIINFO_GetMikiko();
	unsigned int	nSidekickType;

	AI_LoadPlayerHook(f, ent);

	// SCG[2/6/00]: Allocate memory for the client
	gclient_t* client = (gclient_t*)gstate->X_Malloc(sizeof(gclient_t), MEM_TAG_TEMP);
	com->AI_LoadHook(f, client, sizeof(gclient_s));

	ent->s.origin.z += 8;

	playerHook_t* pLoadHook = AI_GetPlayerHook(ent);

	ent->userHook = NULL;

	if ((pLoadHook->type == TYPE_SUPERFLY) || (pLoadHook->type == TYPE_MIKIKOFLY))
	{
		if (pLoadHook->bCarryingMikiko)
		{
			SIDEKICK_Start(ent, TYPE_MIKIKOFLY);
		}
		else
		{
			SIDEKICK_Start(ent, TYPE_SUPERFLY);
		}
		nSidekickType = SIDEKICK_PERS_SUPERFLY;
	}
	else if (pLoadHook->type == TYPE_MIKIKO)
	{
		SIDEKICK_Start(ent, TYPE_MIKIKO);
		nSidekickType = SIDEKICK_PERS_MIKIKO;
	}

	// NSS[3/3/00]:Prevents the 'phatom weapons' from appearing.
	ent->s.modelindex2 = 0;

	ent->client = client;

	playerHook_t* pHook = AI_GetPlayerHook(ent);

	client_persistant_t* pPersistant = &ent->client->pers;
	pHook->stateFlags = pLoadHook->stateFlags;
	pHook->items = pPersistant->nItems;
	pHook->exp_flags = pPersistant->nExpFlags;
	pHook->invulnerability_time = pPersistant->fInvulnerabilityTime;
	pHook->envirosuit_time = pPersistant->fEnvirosuitTime;
	pHook->wraithorb_time = pPersistant->fWraithorbTime;
	pHook->base_power = pPersistant->fBasePower;
	pHook->base_attack = pPersistant->fBaseAttack;
	pHook->base_speed = pPersistant->fBaseSpeed;
	pHook->base_acro = pPersistant->fBaseAcro;
	pHook->base_vita = pPersistant->fBaseVita;
	pHook->power_boost = pPersistant->fBoostPower;
	pHook->attack_boost = pPersistant->fBoostAttack;
	pHook->speed_boost = pPersistant->fBoostSpeed;
	pHook->acro_boost = pPersistant->fBoostAcro;
	pHook->vita_boost = pPersistant->fBoostVita;
	pHook->power_boost_time = pPersistant->fTimePower;
	pHook->attack_boost_time = pPersistant->fTimeAttack;
	pHook->speed_boost_time = pPersistant->fTimeSpeed;
	pHook->acro_boost_time = pPersistant->fTimeAcro;
	pHook->vita_boost_time = pPersistant->fTimeVita;
	ent->s.angles = pPersistant->vAngles;
	pHook->ai_flags = pPersistant->nAIFlags;
	ent->armor_val = pPersistant->nArmorVal;
	ent->armor_abs = pPersistant->nArmorAbs;

	gstate->X_Free(pLoadHook);

	ent->think = SidekickLoadThink;
	ent->nextthink = gstate->time + 1.0f;

}

void SIDEKICK_SetGodMode()
{
	bInGodMode = TRUE;

	userEntity_t* pFirstSidekick = AIINFO_GetFirstSidekick();
	if (pFirstSidekick)
	{
		pFirstSidekick->takedamage = DAMAGE_NO;
	}

	userEntity_t* pSecondSidekick = AIINFO_GetSecondSidekick();
	if (pSecondSidekick)
	{
		pSecondSidekick->takedamage = DAMAGE_NO;
	}

	gstate->Con_Dprintf("Sidekick(s) in god mode.");
}

void SIDEKICK_ClearGodMode()
{
	bInGodMode = FALSE;

	userEntity_t* pFirstSidekick = AIINFO_GetFirstSidekick();
	if (pFirstSidekick)
	{
		pFirstSidekick->takedamage = DAMAGE_YES;
	}

	userEntity_t* pSecondSidekick = AIINFO_GetSecondSidekick();
	if (pSecondSidekick)
	{
		pSecondSidekick->takedamage = DAMAGE_YES;
	}

	gstate->Con_Dprintf("Sidekick(s) no longer in god mode.");
}

int SIDEKICK_IsInGodMode()
{
	return bInGodMode;
}

void SIDEKICK_ToggleGodMode()
{
	if (SIDEKICK_IsInGodMode())
	{
		SIDEKICK_ClearGodMode();
	}
	else
	{
		SIDEKICK_SetGodMode();
	}
}

// ----------------------------------------------------------------------------

int SIDEKICK_IsCarryingMikiko(playerHook_t* hook)
{
	return hook->bCarryingMikiko;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_IsStaying
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsStaying(playerHook_t* hook)
{
	if (hook->ai_flags & AI_STAY)
	{
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_SetAttackTime
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SetAttackTime(playerHook_t* hook, float fTime)
{
	hook->fAttackTime = fTime;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_IsAboutToFire
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsAboutToFire(playerHook_t* hook)
{
	if (gstate->time >= hook->attack_finished || gstate->time >= hook->attack_finished + 0.2f)
	{
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_IsSpecialWeapon
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsSpecialWeapon(weapon_t* pWeapon)
{
	_ASSERTE(pWeapon);

	int nCurrentEpisode = GetCurrentEpisode();
	if (nCurrentEpisode == 2 &&
		(_stricmp(pWeapon->name, "weapon_discus") == 0 ||
			_stricmp(pWeapon->name, "weapon_venomous") == 0)
		)
	{
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_CanUseWeapon
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_CanUseWeapon(weapon_t* pWeapon)
{
	_ASSERTE(pWeapon);
	if (pWeapon->ammo->count > 0 || SIDEKICK_IsSpecialWeapon(pWeapon))
	{
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:        AI_GetCurrentWeaponRange
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float AI_GetCurrentWeaponRange(userEntity_t* self)
{
	weapon_t* pWeapon = (weapon_t*)self->curWeapon;

	if (pWeapon == NULL)
	{
		return 0;
	}

	float fRange = pWeapon->winfo->range;

	if (pWeapon && self->enemy)
	{
		int nCurrentEpisode = GetCurrentEpisode();
		if (nCurrentEpisode == 2 &&
			self->health > 50 && _stricmp(pWeapon->name, "weapon_discus") == 0)
		{
			float fDistance = VectorDistance(self->s.origin, self->enemy->s.origin);
			if (fDistance <= SIDEKICK_MELEE_ATTACK_DISTANCE)
			{
				fRange = SIDEKICK_MELEE_ATTACK_DISTANCE - 4.0f;
			}
		}
		else
			if (nCurrentEpisode == 2 && pWeapon->ammo->count == 0 &&
				_stricmp(pWeapon->name, "weapon_venomous") == 0)
			{
				float fDistance = VectorDistance(self->s.origin, self->enemy->s.origin);
				if (fDistance <= SIDEKICK_MELEE_ATTACK_DISTANCE || pWeapon->ammo->count == 0)
				{
					fRange = SIDEKICK_MELEE_ATTACK_DISTANCE - 4.0f;
				}
			}
	}
	fRange = pWeapon->winfo->range;

	return fRange;

}


// ----------------------------------------------------------------------------
// NSS[12/11/99]:
// Name:		SIDEKICK_IsFacingEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsFacingEnemy(userEntity_t* self, userEntity_t* pEnemy, float maxdist, float maxyaw, float maxpitch)
{
	_ASSERTE(self);
	_ASSERTE(pEnemy);
	//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

		// replace default (invalid) distance
	if (maxdist == -1.0f)
	{
		weapon_t* pWeapon = (weapon_t*)self->curWeapon;
		weaponInfo_t* winfo = (weaponInfo_t*)pWeapon->winfo;
		if (winfo)
		{
			maxdist = winfo->range;
		}
		else
		{
			maxdist = 256.0f;
		}
	}

	CVector dir = pEnemy->s.origin - self->s.origin;
	dir.Normalize();
	CVector angleTowardEnemy;
	VectorToAngles(dir, angleTowardEnemy);

	float fTowardEnemyY = AngleMod(angleTowardEnemy.y);
	float fFacingAngleY = AngleMod(self->s.angles.yaw);

	float fYawDiff = fabs(fTowardEnemyY - fFacingAngleY);
	if (fYawDiff < maxyaw || fYawDiff >(360.0f - maxyaw))
	{
		float fTowardEnemyX = AngleMod(angleTowardEnemy.x);
		float fFacingAngleX = AngleMod(self->s.angles.pitch);
		float fPitchDiff = fabs(fTowardEnemyX - fFacingAngleX);
		float fDistance = VectorDistance(self->s.origin, pEnemy->s.origin);
		if ((fPitchDiff < maxpitch || fPitchDiff >(360.0f - maxpitch)) && fDistance < maxdist)
		{
			return TRUE;
		}
	}
	return FALSE;
}


// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_GetAmbientTimes
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float* SIDEKICK_GetNextAmbientTimes(userEntity_t* self)
{
	float* pNextAmbientTimes = NULL;
	if (AIINFO_GetFirstSidekick() == self)
	{
		pNextAmbientTimes = aNextAmbientTimes[0];
	}
	else
	{
		pNextAmbientTimes = aNextAmbientTimes[1];
	}

	return pNextAmbientTimes;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_SetNextAmbientTime
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SetNextAmbientTime(userEntity_t* self, TASKTYPE nTaskType)
{
	_ASSERTE(nTaskType >= TASKTYPE_SIDEKICK_RANDOMWALK && nTaskType <= TASKTYPE_SIDEKICK_ANIMATE);

	int nIndex = nTaskType - TASKTYPE_SIDEKICK_RANDOMWALK;

	float* pNextAmbientTimes = SIDEKICK_GetNextAmbientTimes(self);

	pNextAmbientTimes[nIndex] = gstate->time + aAmbientInfo[nIndex].fWaitTime;

}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_IsRandomWalkTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float SIDEKICK_GetRandomWalkDistance(playerHook_t* hook)
{
	return hook->follow_dist * 2.0f;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_IsRandomWalkTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsRandomWalkTask(TASKTYPE nTaskType)
{
	if (nTaskType == TASKTYPE_SIDEKICK_RANDOMWALK)
	{
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
// NSS[2/1/00]:
// Name:        SIDEKICK_IsAmbientTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsAmbientTask(TASKTYPE nTaskType)
{
	switch (nTaskType)
	{
	case TASKTYPE_SIDEKICK_RANDOMWALK:
	case TASKTYPE_SIDEKICK_LOOKAROUND:
	case TASKTYPE_SIDEKICK_LOOKUP:
	case TASKTYPE_SIDEKICK_LOOKDOWN:
	case TASKTYPE_SIDEKICK_SAYSOMETHING:
	case TASKTYPE_SIDEKICK_KICKSOMETHING:
	case TASKTYPE_SIDEKICK_WHISTLE:
	case TASKTYPE_SIDEKICK_ANIMATE:
	{
		return TRUE;
	}
	}
	return FALSE;
}


// ----------------------------------------------------------------------------
// NSS[1/13/00]:
// Name:        SIDEKICK_CheckMonsterType
// Description: Checks the current weapon and the enemy and makes sure that the
// two are compatible.  
// Input: userEntity_t *self, userEntity_t *monster
// Output:TRUE or FALSE
// Note:
// ----------------------------------------------------------------------------
int SIDEKICK_CheckMonsterType(userEntity_t* self, userEntity_t* monster)
{
	_ASSERTE(self);
	//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	//	userEntity_t *client_entity = &gstate->g_edicts[1];	// SCG[1/23/00]: not used
	weapon_t* pWeapon = (weapon_t*)self->curWeapon;

	// NSS[1/13/00]:Specific monsters to run from when we have a melee weapon
	if (pWeapon)
	{
		if ((!_stricmp(pWeapon->name, "weapon_silverclaw")) || (!_stricmp(pWeapon->name, "weapon_disruptor")))
		{
			if (self->enemy)
			{
				playerHook_t* mhook = AI_GetPlayerHook(self->enemy);
				if (!mhook)
				{
					return TRUE;
				}
				switch (mhook->type)
				{
				case TYPE_SLUDGEMINION:
				case TYPE_INMATER:
				case TYPE_RAGEMASTER:
				case TYPE_DEATHSPHERE:
				case TYPE_BATTLEBOAR:
				case TYPE_PSYCLAW:
				case TYPE_SL_SKEET:
				case TYPE_TH_SKEET:
				case TYPE_LASERGAT:
				case TYPE_DOOMBAT:
				case TYPE_DRAGON:
				case TYPE_GARROTH:
				case TYPE_NHARRE:
				case TYPE_STAVROS:
				case TYPE_KNIGHT1:
				case TYPE_WYNDRAX:
				case TYPE_KNIGHT2:
				case TYPE_COLUMN:// NSS[1/13/00]:Always run from Columns
				case TYPE_CERBERUS:
				case TYPE_MEDUSA:
				{
					return FALSE;
				}
				}
			}
		}

		// NSS[1/13/00]:Always run from Columns
		if (self->enemy)
		{
			playerHook_t* mhook = AI_GetPlayerHook(self->enemy);
			if (mhook && (mhook->type == TYPE_COLUMN || mhook->type == TYPE_MEDUSA || mhook->type == TYPE_CERBERUS))
				return FALSE;

		}

	}
	// NSS[2/1/00]:We don't have a weapon or we made it through the loop, let's use regular processing
	return TRUE;
}



// ----------------------------------------------------------------------------
// NSS[1/13/00]:
// Name:        SIDEKICK_IsOkToFight
// Description:Determine if the entity has enough health to survive a fight
// Input:userEntity_t *self 
// Output:TRUE or FALSE
// Note:
// ----------------------------------------------------------------------------
int SIDEKICK_IsOkToFight(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);
	weapon_t* pWeapon = (weapon_t*)self->curWeapon;

	if (self->enemy && (self->enemy->flags & FL_EXPLOSIVE))
		return FALSE;

	if (!self->enemy && !(hook->ai_flags & AI_SIDEKICKKILL))
	{
		return FALSE;
	}

	if ((hook->ai_flags & AI_NOATTACK) || self->health < 15 || SIDEKICK_IsCarryingMikiko(hook) || !pWeapon)
	{
		return FALSE;
	}
	if (!SIDEKICK_CanUseWeapon(pWeapon))
	{
		return FALSE;
	}


	return SIDEKICK_CheckMonsterType(self, self->enemy);
}



// ----------------------------------------------------------------------------

int SIDEKICK_IsInAttackMode(playerHook_t* hook)
{

	return hook->bInAttackMode;
}

// ----------------------------------------------------------------------------
// NSS[12/6/99]:
// Name:        SIDEKICK_SetAttackMode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SetAttackMode(userEntity_t* self, userEntity_t* pEnemy)
{
	_ASSERTE(self);
	_ASSERTE(pEnemy);

	playerHook_t* hook = AI_GetPlayerHook(self);


	if (hook)
	{
		// NSS[1/19/00]:Set our enemy;
		self->enemy = pEnemy;

		// NSS[1/16/00]:Make sure we have the right kind of weapon
		SIDEKICK_ChooseBestWeapon(self);


		// NSS[12/15/99]:Make sure we are 'ok' to fight.
		if (!hook->bCarryingMikiko && !SIDEKICK_HasTaskInQue(self, TASKTYPE_SIDEKICK_ATTACK) && SIDEKICK_IsOkToFight(self))
		{
			AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_ATTACK);
		}
		//else if (!SIDEKICK_IsOkToFight( self ) && !SIDEKICK_IsEvading(self))
		else if (!SIDEKICK_IsOkToFight(self) && !SIDEKICK_HasTaskInQue(self, TASKTYPE_SIDEKICK_EVADE))
		{
			AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_EVADE, self->enemy);
		}
	}
}

// ----------------------------------------------------------------------------

void SIDEKICK_ClearAttackMode(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	int bOkToResetGoal = TRUE;

	// if the owner is too far away, run toward the owner always keep a close 
	// distance to owner
	userEntity_t* pOwner = hook->owner;
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
	if (pCurrentGoal)
	{
		TASK_PTR pCurrentTask = GOAL_GetCurrentTask(pCurrentGoal);
		if (pCurrentTask)
		{
			TASKTYPE nTaskType = TASK_GetType(pCurrentTask);
			if (nTaskType == TASKTYPE_SIDEKICK_EVADE)
			{
				bOkToResetGoal = FALSE;
			}
		}
	}

	if (bOkToResetGoal && AI_IsAlive(pOwner) && !SIDEKICK_IsStaying(hook))
	{
		GOALTYPE nGoalType = GOAL_GetType(pCurrentGoal);
		if (nGoalType == GOALTYPE_FOLLOW)
		{
			GOAL_ClearTasks(pCurrentGoal);
			AI_AddInitialTasksToGoal(self, pCurrentGoal, TRUE);
		}
		else
		{
			GOALSTACK_ClearAllGoals(pGoalStack);
			AI_AddNewGoal(self, GOALTYPE_FOLLOW, pOwner);
		}
	}

	SIDEKICK_ResetAmbientTimes(self);
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_ComputeAttackAnimationTime
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float SIDEKICK_ComputeAttackAnimationTime(userEntity_t* self)
{
	weaponInfo_t* winfo = (weaponInfo_t*)self->winfo;
	_ASSERTE(winfo);

	playerHook_t* hook = AI_GetPlayerHook(self);

	float fFrameTime = 0.03f;
	float fAnimTime = winfoNumFrames(ANIM_SHOOT_STD) * fFrameTime + 0.1f;
	if (hook->attack_boost)
	{
		fFrameTime = fFrameTime / ((hook->attack_boost + 1) * 0.3);
		fAnimTime = (winfoNumFrames(ANIM_SHOOT_STD) * fFrameTime) + 0.1f;
	}

	return fAnimTime;
}


// ----------------------------------------------------------------------------
// NSS[12/14/99]:
// Name:        SIDEKICK_InAttackRange
// Description:Are we within range?
// Input:userEntity_t *self, float fDistance, userEntity_t *enemy
// Output:TRUE/FALSE
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_InAttackRange(userEntity_t* self, float fDistance, userEntity_t* enemy)
{

	float fWeaponRange = AI_GetCurrentWeaponRange(self);

	if (!enemy)
	{
		if (self->enemy)
		{
			enemy = self->enemy;
		}
		else
		{
			return TRUE;
		}
	}

	if ((fWeaponRange > fDistance || fDistance < 94.0f) && SIDEKICK_IsClearShot(self, enemy, self->s.origin, enemy->s.origin))
		return TRUE;

	return FALSE;
	// hook->fnInAttackRange( self, fDistance, enemy )
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_ComputeAnimationTime
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float SIDEKICK_ComputeAnimationTime(playerHook_t* hook)
{
	_ASSERTE(hook);

	if (!hook->cur_sequence)
	{
		return 0.0f;
	}

	frameData_t* pCurrentSequence = hook->cur_sequence;
	_ASSERTE(pCurrentSequence);

	float fTime = (pCurrentSequence->last - pCurrentSequence->first + 1) * 0.1f;

	return fTime;
}


// ----------------------------------------------------------------------------
// NSS[12/16/99]:
// Name:		Sidekick_Touch
// Description: A touch function for the Sidekicks to keep them in check with one another.
// Input:	userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf
// Output: NA
// Note:
//
// ----------------------------------------------------------------------------
void Sidekick_Touch(userEntity_t* self, userEntity_t* other, cplane_t* plane, csurface_t* surf)
{
	if (other)
	{
		playerHook_t* ohook = AI_GetPlayerHook(other);
		if (ohook)
		{
			if ((ohook->type == TYPE_MIKIKO || ohook->type == TYPE_SUPERFLY) && (self->velocity.Length() == 0.0f))
			{
				SIDEKICK_HandleMoveAway(self, other);
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_Init
// Description:
// Input:
// Output:
// Note:
//				self->s.origin should be set before calling this function
//
// ----------------------------------------------------------------------------
void SIDEKICK_Init(userEntity_t* self, int nType)
{
	_ASSERTE(self);
	int MikikoFly = 0;
	playerHook_t* hook = AI_GetPlayerHook(self);
	//	set the size
	gstate->SetSize(self, -16.0f, -16.0f, -24.0f, 16.0f, 16.0f, 32.0f);
	Superflywait = 0.0f;	// NSS[3/4/00]:These two are used to determine the time between commands.
	MikikoWait = 0.0f;
	SuperFlyWaitIdle = 0.0f;	// NSS[3/4/00]:These two are used to determine the time in which they are in an IDLE state.
	MikikoWaitIdle = 0.0f;

	SuperflyLastTalk = gstate->time + 10.0f;
	MikikoLastTalk = gstate->time + 10.0f;

	memset(SuperflyCom, 0, sizeof(SuperflyCom));
	memset(SuperflyCom, 0, sizeof(MikikoCom));

	if (nType == TYPE_MIKIKO)
	{
		self->className = MIKIKO_CLASSNAME;

		self->netname = tongue_world[T_MIKIKO_IBIHARA];
		self->scriptname = MIKIKO_CLASSNAME;
	}
	else
		if (nType == TYPE_SUPERFLY || nType == TYPE_MIKIKOFLY)
		{
			//NSS[11/30/99]:


			if (nType == TYPE_MIKIKOFLY)
			{
				MikikoFly = 1;
				hook->type = TYPE_SUPERFLY;			//Not my idea, but hey some of the code was already there why not use it.
				self->className = SUPERFLY_CLASSNAME;
			}
			else
			{
				self->className = SUPERFLY_CLASSNAME;
			}

			self->netname = tongue_world[T_SUPERFLY_JOHNSON];
			self->scriptname = SUPERFLY_CLASSNAME;
		}
		else
			if (nType == TYPE_CLIENT)
			{
				self->className = HIRO_CLASSNAME;

				self->netname = tongue_world[T_HIRO_MIYAMOTO];
				self->scriptname = HIRO_CLASSNAME;
			}

	self->save = AI_SaveSidekick;
	self->load = AI_LoadSidekick;

	// init model information
	/////////////////////////////////////////////////////////
	//	make sure we're drawing the models
	/////////////////////////////////////////////////////////
	self->s.renderfx &= ~RF_NODRAW;

	char* szModelName;
	if (MikikoFly)
	{
		szModelName = AIATTRIBUTE_GetModelName(MIKIKOFLY_CLASSNAME);
	}
	else
	{
		szModelName = AIATTRIBUTE_GetModelName(self->className);
	}

	_ASSERTE(szModelName);
	self->s.modelindex = gstate->ModelIndex(szModelName);
	self->modelName = szModelName;

	// initialize the animation frame data
	if (!ai_get_sequences(self))
	{
		char* szCSVFileName = AIATTRIBUTE_GetCSVFileName(self->className);
		_ASSERTE(szCSVFileName);
		FRAMEDATA_ReadFile(szCSVFileName, self);
	}

	// amw: right now zero means don't look at anyone
	self->s.numClusters = 0;

	// init animation info
	self->s.frameInfo.frameInc = 1;
	self->s.frameInfo.frameTime = FRAMETIME_FPS20;
	self->s.frameInfo.next_frameTime = gstate->time + self->s.frameInfo.frameTime;


	//	server and physics
	self->movetype = MOVETYPE_WALK;
	self->solid = SOLID_BBOX;
	//	self->clipmask			= MASK_NPCSOLID;
	self->clipmask = MASK_MONSTERSOLID;
	self->flags |= FL_BOT;// | FL_PUSHABLE;
	self->elasticity = 1.0;
	self->mass = 300.0;
	self->max_speed = SIDEKICK_RUNSPEED;
	self->svflags |= SVF_MONSTER;
	self->svflags &= ~SVF_DEADMONSTER;
	self->s.effects = EF_GIB;				// this makes server send client info to the clients //???
	// SCG[12/7/99]: 	self->s.renderfx		= RF_CLUSTERBASED;		// makes this model draw with cluster based animation
	self->s.iflags = IF_SV_SENDBBOX;		// send the real BBOX size down to client for prediction	

	self->ang_speed.Set(90.0f, 90.0f, 90.0f);
	AI_ZeroVelocity(self);

	self->s.effects = 0;
	self->s.origin.z += 0.125;		// make sure off ground

	//	set up skins
	self->s.skinnum = 0;
	self->s.frame = 0;

	self->viewheight = 22.0f;
	self->health = ATTR_HEALTH;
	self->takedamage = DAMAGE_AIM;
	self->deadflag = DEAD_NO;

	//	thinks
	self->prethink = SIDEKICK_PreThink;
	self->postthink = SIDEKICK_PostThink;
	self->pain = SIDEKICK_Pain;
	self->die = SIDEKICK_StartDie;

	self->goalentity = self;
	self->enemy = NULL;

	self->PickedUpItem = SIDEKICK_PickedUpItem;

	self->touch = Sidekick_Touch;

	ai_setfov(self, 270.0f);


	hook->dflags &= ~DFL_LLAMA;	// SCG[1/5/00]: 

	hook->upward_vel = 125.0;
	hook->forward_vel = 350.0;
	hook->max_jump_dist = ai_max_jump_dist(hook->forward_vel, hook->upward_vel);

	hook->attack_dist = 2000.0;
	hook->run_speed = SIDEKICK_RUNSPEED;
	hook->walk_speed = SIDEKICK_WALKSPEED;
	hook->attack_speed = SIDEKICK_RUNSPEED;
	hook->jump_chance = 0;
	hook->back_away_dist = 0;
	hook->active_distance = 8192;

	hook->think_time = THINK_TIME;

	hook->fnStartAttackFunc = NULL;//SIDEKICK_BeginAttack;
	hook->fnAttackFunc = NULL;//SIDEKICK_Attack;

	hook->bOkToAttack = TRUE;
	hook->nTargetFrequency = 5;

	hook->bInTransition = FALSE;
	hook->nTransitionIndex = 0;
	hook->fTransitionTime = 0.0f;
	hook->pTransitions = NULL;

	// misc hook info
	hook->fnFindTarget = NULL;//SIDEKICK_FindEnemy;
	hook->dflags &= ~(DFL_EVIL | DFL_NEUTRAL | DFL_BERSERK);
	hook->dflags = DFL_RANGEDATTACK + DFL_CANSTRAFE + DFL_GOOD;

	hook->fov_desired = 90.0;
	hook->fov_rate = 0.0;

	// initialize timer info
	hook->strafe_time = gstate->time;
	hook->strafe_dir = STRAFE_LEFT;

	//	poison stuff
	hook->poison_time = 0;
	hook->poison_next_damage_time = 0;
	hook->poison_damage = 0;
	hook->poison_interval = 0;

	//	power ups
	hook->invulnerability_time = 0;
	hook->envirosuit_time = 0;
	hook->wraithorb_time = 0;
	hook->oxylung_time = 0;

	//	temporary attribute boosts
	hook->power_boost_time = 0;
	hook->attack_boost_time = 0;
	hook->speed_boost_time = 0;
	hook->acro_boost_time = 0;
	hook->vita_boost_time = 0;

	//	other timer fields
	hook->pain_finished = 0;
	hook->drown_time = gstate->time + 12.0;
	hook->attack_finished = 0;
	hook->jump_time = 0;
	hook->sound_time = 0;
	hook->path_time = 0;
	hook->think_time = 0.1;
	hook->shake_time = 0;
	hook->ambient_time = 0;

	//	clear flags
	hook->exp_flags = 0;

	//	Nelno:	never finished implementing this...
	hook->fatigue = 0;

	hook->fnInAttackRange = SIDEKICK_InAttackRange;

	// now initialize inventory
	if (self->inventory)
	{
		//	client already has an inventory, so free it and set ptr to NULL
		SIDEKICK_FreeInventory(self);
	}

	self->inventory = gstate->InventoryNew(MEM_MALLOC);

	hook->follow_dist = SIDEKICK_TIGHT_RUN_FOLLOW_DISTANCE;
	hook->walk_follow_dist = SIDEKICK_TIGHT_WALK_FOLLOW_DISTANCE;

	self->think = SIDEKICK_FindOwnerThink;		//	waits for owner to connect
	AI_SetNextThinkTime(self, 0.2f);

	hook->fSoundOutDistance = 0.0f;

	hook->nAttackMode = ATTACKMODE_NORMAL;

	hook->pWantItem = NULL;
	hook->bInAttackMode = FALSE;
	hook->bAttackPlayer = FALSE;
	hook->bCarryingMikiko = MikikoFly;
	hook->bJustFired = 0;
	hook->type = nType;		//NSS[11/30/99]:Added

	hook->dflags |= DFL_CANUSELADDER | DFL_CANUSEDOOR;// NSS[2/12/00]:They can use both

	AIATTRIBUTE_SetInfo(self);

	// update client on sidekick's info
	if (hook->type == TYPE_SUPERFLY || hook->type == TYPE_MIKIKOFLY)
	{
		com->Sidekick_Update(self, SIDEKICK_SUPERFLY, SIDEKICK_STATUS_HEALTH, 100 * (self->health / hook->base_health)); // update health
	}
	else
	{
		com->Sidekick_Update(self, SIDEKICK_MIKIKO, SIDEKICK_STATUS_HEALTH, 100 * (self->health / hook->base_health)); // update health
	}

	SIDEKICK_FindOwner(self);

	//NSS[11/27/99]:Flag which will allow the sidekicks to swim around and follow you into water until they start taking damage.
	hook->ai_flags |= AI_CANSWIM;
	hook->ai_flags |= AI_FOLLOW;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_InitWeapons
// Description:
// Input:
// Output:
// Note:
//			Inits all variables related to client weapons
//
// ----------------------------------------------------------------------------
void SIDEKICK_InitWeapons(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	self->curWeapon = NULL;

	hook->fxFrameNum = 0;
	hook->fxFrameFunc = NULL;

	hook->last_button0 = 0;
	//	hook->viewmodel_start_frame = 0;
	//	hook->viewmodel_end_frame	= 0;
	//	hook->viewmodel_frame_time	= 0;

	hook->weapon_fired = NULL;
	hook->weapon_next = NULL;

	// clear out armor also
	self->armor_val = 0;
	self->armor_abs = 0.0f;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_FreeWeapon
// Description:
// Input:
// Output:
// Note:
//			untracks the client's weapon and removes the entity associated with it
//
// ----------------------------------------------------------------------------
void SIDEKICK_FreeWeapon(userEntity_t* self)
{
	self->curWeapon = NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_InitDefaultWeapon
// Description:
// Input:
// Output:
// Note:
//			Give client default weapon
//
// ----------------------------------------------------------------------------
void SIDEKICK_InitDefaultWeapon(userEntity_t* self)
{
	_ASSERTE(self);
	// give client a weapon if deathmatch
	_ASSERTE(self->inventory);

	weaponInfo_t* winfo = NULL;
	playerHook_t* hook = AI_GetPlayerHook(self);

	int nCurrentEpisode = GetCurrentEpisode();
	switch (nCurrentEpisode)
	{
	case 1:
	{

		if (hook && !hook->bCarryingMikiko)
		{
			winfo = com->FindRegisteredWeapon("weapon_disruptor");
			if (winfo)
			{
				winfo->give_func(self, winfo->initialAmmo);
				winfo->select_func(self);
			}

			// NSS[1/10/00]:Added server var to allow sidekicks to have all weapons upon start
			if (gstate->GetCvar("sv_sidekickweapons"))
			{
				winfo = com->FindRegisteredWeapon("weapon_ionblaster");
				if (winfo)
				{
					winfo->give_func(self, winfo->initialAmmo);
					winfo->select_func(self);
				}
				winfo = com->FindRegisteredWeapon("weapon_shotcycler");
				if (winfo)
				{
					winfo->give_func(self, winfo->initialAmmo);
					winfo->select_func(self);
				}
			}

		}
		break;
	}

	case 2:
	{

		// NSS[1/10/00]:Added server var to allow sidekicks to have all weapons upon start
		if (gstate->GetCvar("sv_sidekickweapons"))
		{
			if (hook && !hook->bCarryingMikiko)
			{
				winfo = com->FindRegisteredWeapon("weapon_discus");
				if (winfo)
				{
					winfo->give_func(self, winfo->initialAmmo);
					winfo->select_func(self);
				}

				winfo = com->FindRegisteredWeapon("weapon_venomous");
				if (winfo)
				{
					winfo->give_func(self, winfo->initialAmmo);
					winfo->select_func(self);
				}

				winfo = com->FindRegisteredWeapon("weapon_trident");
				if (winfo)
				{
					winfo->give_func(self, winfo->initialAmmo);
					winfo->select_func(self);
				}

			}
		}
		break;
	}
	case 3:
	{
		// NSS[1/10/00]:Added server var to allow sidekicks to have all weapons upon start
		if (hook && !hook->bCarryingMikiko && gstate->GetCvar("sv_sidekickweapons"))
		{
			winfo = com->FindRegisteredWeapon("weapon_silverclaw");
			if (winfo)
			{
				winfo->give_func(self, winfo->initialAmmo);
				winfo->select_func(self);
			}

			winfo = com->FindRegisteredWeapon("weapon_bolter");
			if (winfo)
			{
				winfo->give_func(self, winfo->initialAmmo);
				winfo->select_func(self);
			}
		}
		break;
	}
	case 4:
	{
		// NSS[1/10/00]:Added server var to allow sidekicks to have all weapons upon start
		if (gstate->GetCvar("sv_sidekickweapons"))
		{
			if (hook && !hook->bCarryingMikiko)
			{
				winfo = com->FindRegisteredWeapon("weapon_glock");
				winfo->give_func(self, winfo->initialAmmo);
				winfo->select_func(self);
				winfo = com->FindRegisteredWeapon("weapon_ripgun");
				winfo->give_func(self, winfo->initialAmmo);
				winfo->select_func(self);
				winfo = com->FindRegisteredWeapon("weapon_slugger");
				winfo->give_func(self, winfo->initialAmmo);
				winfo->select_func(self);

			}
		}
		break;
	}
	default:
	{
		break;
	}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_FreeInventory
// Description:
// Input:
// Output:
// Note:
//			Frees inventory when a player restarts (single player) or respawns 
//			(coop and deathmatch)
//
// ----------------------------------------------------------------------------
void SIDEKICK_FreeInventory(userEntity_t* self)
{
	_ASSERTE(self);

	if (!self->inventory)
	{
		return;
	}

	gstate->InventoryFree(self->inventory);
	self->inventory = NULL;

	//	make sure there are no pointers to weapons left
	SIDEKICK_InitWeapons(self);

}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_DropInventory
// Description:
// Input:
// Output:
// Note:
//			makes client drop their weapon and ammo in deathmatch
//
// ----------------------------------------------------------------------------
void SIDEKICK_DropInventory(userEntity_t* self)
{
	_ASSERTE(self);

	playerHook_t* hook = AI_GetPlayerHook(self);
	_ASSERTE(hook);

	if (!self->inventory)
	{
		return;
	}

	weaponInfo_t* winfo = (weaponInfo_t*)self->winfo;
	//	drop the client's current weapon with current amount of ammo
	if (winfo)
	{
		// no model should be drawn on/with the client model
		self->s.modelindex2 = 0;

		weapon_t* pWeapon = (weapon_t*)self->curWeapon;
		if (pWeapon && pWeapon->ammo->count > 0 && !(winfo->flags & WFL_NODROP))
		{
			userEntity_t* pNewWeapon = gstate->SpawnEntity();
			pNewWeapon->className = winfo->weaponName;
			pNewWeapon->netname = winfo->weaponNetName;
			pNewWeapon->spawnflags = (DROPPED_ITEM | DROPPED_PLAYER_ITEM);

			//	set origin
			pNewWeapon->s.origin = self->s.origin;
			//	give some velocity
			pNewWeapon->velocity.x = rnd() * 300.0f - 200.0f;
			pNewWeapon->velocity.y = rnd() * 300.0f - 200.0f;
			pNewWeapon->velocity.z = rnd() * 200.0f + 200.0f;

			winfo->spawn_func(winfo, pNewWeapon, -1.0f, NULL);

			//	find out how much ammo player has for this weapon,
			//	and store it in the weapon pickup item
			pNewWeapon->hacks_int = pWeapon->ammo->count;
		}
	}

	//	clear everything having to do with weapons
	SIDEKICK_FreeWeapon(self);
	SIDEKICK_InitWeapons(self);

}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_InitStats
// Description:
// Input:
// Output:
// Note:
//			Inits deathmatch statistics
//
// ----------------------------------------------------------------------------
void SIDEKICK_InitStats(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);
	_ASSERTE(hook);

	hook->killed = 0;
	hook->died = 0;
	hook->fired = 0;
	hook->hit = 0;

}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_FreeNodeSystem
// Description:
// Input:
// Output:
// Note:
//	frees data associated with node system
//
// ----------------------------------------------------------------------------
void SIDEKICK_FreeNodeSystem(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	_ASSERTE(hook);

	hook->pNodeList = AI_DeleteNodeList(hook->pNodeList);

	//	by default, players should not have either of these...
	PATHLIST_KillPath(hook->pPathList);
	hook->pPathList = PATHLIST_Destroy(hook->pPathList);

}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_CheckPowerUps
// Description:
//	called each frame before the physics are run
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_CheckPowerUps(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	_ASSERTE(hook);

	int	recalc = FALSE;

	// exp_flags - set for each item when picked up and cleared when the item's expire message is sent
	// items - set when an item is picked up and cleared when the item becomes inactive
	// xx_time - set when an item is picked up to number of seconds till item expires

	////////////////////////////////////////
	// poison
	////////////////////////////////////////
	if (hook->exp_flags & EXP_POISON && hook->poison_time <= -3.0)
	{
		//	poison expiration sound and message
		gstate->StartEntitySound(self, CHAN_VOICE, gstate->SoundIndex("artifacts/poisonfade.wav"),
			1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
		sidekick_deathtype = POISON;
		//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_POISON_EXPIRE]);
		hook->exp_flags &= ~EXP_POISON;
	}
	if (hook->items & IT_POISON)
	{
		//	do damage
		if (hook->poison_next_damage_time <= 0)
		{
			com->Damage(self, self, self, zero_vector, zero_vector, hook->poison_damage, DAMAGE_POISON | DAMAGE_NO_BLOOD);
			hook->poison_next_damage_time = hook->poison_interval;
		}
		//	poison expire
		if (hook->poison_time < 0)
		{
			hook->items &= ~IT_POISON;
		}
		else
		{
			hook->poison_time -= 0.1f;
			//		hook->poison_interval -= 0.1;
			hook->poison_next_damage_time -= 0.1f;
		}
	}

	////////////////////////////////////////
	// envirosuit
	////////////////////////////////////////
	if (hook->envirosuit_time <= gstate->time + 3.0 && (hook->exp_flags & EXP_ENVIROSUIT))
	{
		//	envirosuit expiration sound and message
		gstate->StartEntitySound(self, CHAN_VOICE, gstate->SoundIndex("artifacts/envirosuit/envirosuitfade.wav"),
			1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
		//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_ENVIROSUIT_EXPIRE]);
		hook->exp_flags = hook->exp_flags - (hook->exp_flags & EXP_ENVIROSUIT);
	}
	if (hook->items & IT_ENVIROSUIT)
	{
		//	envirosuit expires
		if (hook->envirosuit_time < gstate->time)
		{
			hook->items -= (hook->items & IT_ENVIROSUIT);
		}
	}

	////////////////////////////////////////
	// megashield
	////////////////////////////////////////
	if (hook->invulnerability_time <= gstate->time + 3.0 && (hook->exp_flags & EXP_MEGASHIELD))
	{
		//	megashield expiration sound and message
		//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_MEGASHIELD_EXPIRE]);
		gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("artifacts/megashield/megafade.wav"),
			1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
		hook->exp_flags = hook->exp_flags - (hook->exp_flags & EXP_MEGASHIELD);
	}
	if (hook->items & IT_MEGASHIELD)
	{
		//	megashield expires
		if (hook->invulnerability_time < gstate->time)
		{
			hook->items -= (hook->items & IT_MEGASHIELD);
		}
	}
	/////////////////////////////////////////////
	// stat boosters
	/////////////////////////////////////////////
	if (hook->items & IT_POWERBOOST)
	{
		if (hook->power_boost_time <= gstate->time + 5.0 && hook->exp_flags & EXP_POWERBOOST)
		{
			//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_BOOST_POWER_EXPIRE]);
			hook->exp_flags -= (hook->exp_flags & (EXP_POWERBOOST));
		}

		if (hook->power_boost_time <= gstate->time)
		{
			hook->items -= (hook->items & IT_POWERBOOST);
			hook->power_boost = 0; //hook->base_power;
			recalc = TRUE;
		}
	}
	if (hook->items & IT_ATTACKBOOST)
	{
		if (hook->attack_boost_time <= gstate->time + 5.0 && hook->exp_flags & EXP_ATTACKBOOST)
		{
			//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_BOOST_ATTACK_EXPIRE]);
			hook->exp_flags -= (hook->exp_flags & (EXP_ATTACKBOOST));
		}

		if (hook->attack_boost_time <= gstate->time)
		{
			hook->items -= (hook->items & IT_ATTACKBOOST);
			hook->attack_boost = 0; //hook->base_attack;
			recalc = TRUE;
		}
	}
	if (hook->items & IT_SPEEDBOOST)
	{
		if (hook->speed_boost_time <= gstate->time + 5.0 && hook->exp_flags & EXP_SPEEDBOOST)
		{
			//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_BOOST_SPEED_EXPIRE]);
			hook->exp_flags -= (hook->exp_flags & (EXP_SPEEDBOOST));
		}

		if (hook->speed_boost_time <= gstate->time)
		{
			hook->items -= (hook->items & IT_SPEEDBOOST);
			hook->speed_boost = 0; //hook->base_speed;
			recalc = TRUE;
		}
	}
	if (hook->items & IT_ACROBOOST)
	{
		if (hook->acro_boost_time <= gstate->time + 5.0 && hook->exp_flags & EXP_ACROBOOST)
		{
			//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_BOOST_ACRO_EXPIRE]);
			hook->exp_flags -= (hook->exp_flags & (EXP_ACROBOOST));
		}

		if (hook->acro_boost_time <= gstate->time)
		{
			hook->items -= (hook->items & IT_ACROBOOST);
			hook->acro_boost = 0; //hook->base_acro;
			recalc = TRUE;
		}
	}
	if (hook->items & IT_VITABOOST)
	{
		if (hook->vita_boost_time <= gstate->time + 5.0 && hook->exp_flags & EXP_VITABOOST)
		{
			//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_BOOST_VITA_EXPIRE]);
			hook->exp_flags -= (hook->exp_flags & (EXP_VITABOOST));
		}

		if (hook->vita_boost_time <= gstate->time)
		{
			hook->items -= (hook->items & IT_VITABOOST);
			hook->vita_boost = 0; //hook->base_vita;
			recalc = TRUE;
		}
	}

	if (recalc)
	{
		com->CalcBoosts(self);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_PreThink
// Description:
// Input:
// Output:
// Note:
//	called each frame before the physics are run
//
// ----------------------------------------------------------------------------
void SIDEKICK_PreThink(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);

	client_deathtype = NONE;

	//if(!(self->flags & FL_HCHILD))
	//	self->flags |= FL_HCHILD;

	if (hook)
		AI_ClearExecutedTasks(hook);

	/////////////////////////////////////////////////////////////////
	//	check to see if client on a warp path
	//	frame updates don't happen in a warp
	/////////////////////////////////////////////////////////////////

	if (self->flags & FL_INWARP)
	{
		return;
	}

	//	animate the input entity's models
	com->FrameUpdate(self);

	//	fire player weapons that are based on frames
	/*if ( hook->fxFrameFunc )
	{
		hook->fxFrameFunc( self );
	}*/

	if (!AI_IsAlive(self)) // 10.6 changed to 'NOT alive'
	{
		return;
	}

	hook->last_origin = self->s.origin;

	// determine if the player/bot has left the ground between nodes
	// used for determining when to link nodes back
	if (self->groundEntity == NULL)
	{
		hook->bOffGround = TRUE;
	}

}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_NitroDeath
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
// Logic[5/10/99]: E3 Nitro Hack
void SIDEKICK_NitroDeath(userEntity_t* self)
{
	com->Damage(self, self, self, zero_vector, zero_vector, 32000.0f, DAMAGE_LAVA | DAMAGE_NO_BLOOD);
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_PostThink
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void SIDEKICK_KeepGroundedUpright(userEntity_t* self)
{
	playerHook_t* hook;

	if (!self || !self->groundEntity)
		return;

	hook = AI_GetPlayerHook(self);
	if (!hook || !AI_IsSidekick(hook))
		return;

	self->s.angles.pitch = 0.0f;
	self->s.angles.roll = 0.0f;
	self->ideal_ang.pitch = 0.0f;
	self->ideal_ang.roll = 0.0f;
}

void SIDEKICK_PostThink(userEntity_t* self)
{
	_ASSERTE(self);
	//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used


	if (self->flags & FL_INWARP)
	{
		return;
	}

	if (!AI_IsAlive(self)) // 10.6 changed to 'NOT alive'
	{
		return;
	}

	SIDEKICK_KeepGroundedUpright(self);

	//SIDEKICK_CheckPowerUps( self );

  // Logic[5/10/99]: E3 NITRO HACK
	if (self->s.renderfx & RF_IN_NITRO)
	{
		if (self->think != SIDEKICK_NitroDeath)
		{
			self->think = SIDEKICK_NitroDeath;
			AI_SetNextThinkTime(self, 3.0f);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_FindOwner
// Description:
// Input:
// Output:
// Note:
//				waits until a player logs on, then assigns that player as owner 
//              this sidekick
//
// ----------------------------------------------------------------------------
void SIDEKICK_FindOwner(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	AI_SetOwner(self, NULL);

	int nGoalType = GOALTYPE_NONE;
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
	if (pCurrentGoal)
	{
		GOALTYPE nGoalType = GOAL_GetType(pCurrentGoal);
	}

	userEntity_t* head = alist_FirstEntity(client_list);
	while (head != NULL)
	{
		// NSS[12/5/99]:Making changes for Coop.. find the first Client
		//if ( AI_IsAlive( head ) && _stricmp( head->className, "player" ) == 0 )
		if (AI_IsAlive(head) && (head->flags & FL_CLIENT))
		{
			float fDistance = VectorDistance(self->s.origin, head->s.origin);
			if ((fDistance < hook->active_distance && AI_IsVisible(self, head)) ||
				(AI_IsLineOfSight(self, head) && fDistance < 256.0f))
			{
				AI_SetOwner(self, head);

				if (!SIDEKICK_IsStaying(hook))
				{
					if (nGoalType == GOALTYPE_FOLLOW)
					{
						AI_AddInitialTasksToGoal(self, pCurrentGoal, TRUE);
					}
					else
					{
						AI_AddNewGoal(self, GOALTYPE_FOLLOW, hook->owner);
					}
				}
			}
		}

		head = alist_NextEntity(client_list);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_FindOwnerThink
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_FindOwnerThink(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	// NSS[2/10/00]:Make sure we have some form of a sequence first!
	if (!hook->cur_sequence)
	{
		// NSS[2/10/00]:We got no sequence thus force one down the hole!
		char szAnimation[16];
		AI_SelectAmbientAnimation(self, szAnimation);
		frameData_t* pSequence = FRAMES_GetSequence(self, szAnimation);
		AI_ForceSequence(self, pSequence, FRAME_LOOP);
	}
	SIDEKICK_FindOwner(self);




	if (hook->owner == NULL)
	{
		self->think = SIDEKICK_FindOwnerThink;
	}
	else
	{
		SIDEKICK_InitDefaultWeapon(self);

		self->think = SIDEKICK_Think;
	}

	AI_SetNextThinkTime(self, 0.1f);
}

// ----------------------------------------------------------------------------
// NSS[3/1/00]:
// Name:		SIDEKICK_CanPathToOrIsCloseToEnemy
// Description: This will determine all of the possible variables determining
// if the enemy is reachable, shootable, and visible.  
// Input:userEntity_t self, userEntity_t *enemy
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_CanPathToOrIsCloseToEnemy(userEntity_t* self, userEntity_t* enemy)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	int response = FALSE;
	int path = AI_FindPathToEntity(self, enemy, TRUE);
	float xyDistance = VectorXYDistance(self->s.origin, enemy->s.origin);
	float zDistance = VectorZDistance(self->s.origin, enemy->s.origin);
	float fDistance = SIDEKICK_DeterminePathLength(self, enemy);
	if (hook->pPathList && hook->pPathList->nPathLength && fDistance < 1024.0f && fDistance != -1)
	{
		response = TRUE;
	}
	else if (hook->pPathList && hook->pPathList->nPathLength && fDistance != -1 && zDistance < 64.0f && xyDistance < 256.0f || zDistance < 64.0f && xyDistance < 256.0f && AI_IsVisible(self, enemy))
	{
		response = TRUE;
	}
	else if ((enemy->movetype == MOVETYPE_FLY || enemy->movetype == MOVETYPE_HOVER || enemy->movetype == MOVETYPE_SWIM || enemy->movetype == MOVETYPE_NONE) && zDistance < 1024.0f && AI_IsVisible(self, enemy) && SIDEKICK_IsClearShot(self, enemy, self->s.origin, enemy->s.origin))
	{
		response = TRUE;
	}
	return response;
}

// ----------------------------------------------------------------------------
// NSS[3/1/00]:
// Name:		SIDEKICK_CanPathToOrIsCloseToEnemy
// Description: This will determine all of the possible variables determining
// if the enemy is reachable, shootable, and visible.  
// Input:userEntity_t self, userEntity_t *enemy
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_CanPathToOrIsCloseToOwner(userEntity_t* self, userEntity_t* owner)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	int response = FALSE;
	int path;
	float xyDistance;
	float zDistance;
	float fDistance;

	if (!hook || !owner)
		return FALSE;

	if (AI_IsSidekick(hook))
	{
		CVector followPoint;
		int followPointType = SIDEKICK_GetTightFollowPoint(self, owner, followPoint);

		if (followPointType == 2)
			return TRUE;

		if (followPointType == 1 &&
			AI_IsOkToMoveStraight(self, followPoint,
				VectorXYDistance(self->s.origin, followPoint),
				VectorZDistance(self->s.origin, followPoint)))
		{
			return TRUE;
		}
	}

	path = AI_FindPathToEntity(self, owner, TRUE);
	xyDistance = VectorXYDistance(self->s.origin, owner->s.origin);
	zDistance = VectorZDistance(self->s.origin, owner->s.origin);
	fDistance = SIDEKICK_DeterminePathLength(self, owner);
	if (hook->pPathList && hook->pPathList->nPathLength > 2 && fDistance != -1)
	{
		response = TRUE;
	}
	else if (hook->pPathList && hook->pPathList->nPathLength && fDistance != -1 && zDistance < 64.0f && xyDistance < 256.0f || zDistance < 64.0f && xyDistance < 256.0f && AI_IsVisible(self, owner))
	{
		response = TRUE;
	}
	return response;
}
// ----------------------------------------------------------------------------
// NSS[5/19/00]:
// Name:  SIDEKICK_IsOkToMoveStraight
// Description:This is a hack so that we do not change the fundamental ai_isoktomovestraight
// function but we can make changes here for various minor functions which need 'special' attention.
// Input:userEntity_t *self, CVector &destPoint, float fXYDistance, float fZDistance
// Output:TRUE/FALSE
// ----------------------------------------------------------------------------
int SIDEKICK_IsOkToMoveStraight(userEntity_t* self, CVector& destPoint, float fXYDistance, float fZDistance)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	if (AI_IsFlyingUnit(self) || AI_IsInWater(self) || hook->ai_flags & AI_DROWNING || self->flags & FL_CINEMATIC)
	{
		if (AI_IsLineOfSight(self, destPoint))
		{
			return TRUE;
		}
	}
	else
	{
		CVector Mins = self->s.mins;
		CVector QuarterDest, Dir;
		float fSpeed = self->velocity.Length() * 0.125;
		Dir = destPoint - self->s.origin;
		Dir.Normalize();
		QuarterDest = self->s.origin + (Dir * fSpeed);
		Mins.z += 8.0f;
		tr = gstate->TraceBox_q2(self->s.origin, Mins, self->s.maxs, QuarterDest, self, MASK_SOLID);
		if (hook->ai_flags & AI_ALWAYS_USENODES)
		{
			if ((tr.fraction >= 1.0f && fZDistance < MAX_JUMP_HEIGHT) ||
				(tr.fraction >= 0.8f && ((1.2f - tr.fraction) * (fXYDistance + fZDistance)) < 32.0f))
			{
				if (AI_IsGroundBelowBetween(self, self->s.origin, destPoint))
				{
					return TRUE;
				}
			}
		}
		else
		{
			if ((tr.fraction >= 1.0f && fZDistance < MAX_JUMP_HEIGHT) ||
				(tr.fraction >= 0.8f && ((1.2f - tr.fraction) * (fXYDistance + fZDistance)) < 32.0f))
			{
				if (AI_IsGroundBelowBetween(self, self->s.origin, destPoint))
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}


// ----------------------------------------------------------------------------
// <nss>
// Name:		Markers_Think
// Description:Think function for Markers
// Input:userEntity_t *self
// Output:<nothing>
// Note:  
// Created:<nss>
// ----------------------------------------------------------------------------
void SIDEKICK_Markers_Think(userEntity_t* self)
{
	if (self->delay < gstate->time)
		self->remove(self);
	self->nextthink = gstate->time + 0.01f;
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		AI_Drop_Markers
// Description:Drops a Marker at the CVector Target origin and leaves it for
// 20 seconds
// Input:CVector Target
// Output:<nothing>
// Note:  
// Created:<nss>
// ----------------------------------------------------------------------------
void SIDEKICK_Drop_Markers(CVector& Target, float delay)
{
	if (gstate->GetCvar("ai_debug_info") != 0.0f)
	{
		userEntity_t* Flare;

		Flare = gstate->SpawnEntity();

		Flare->owner = Flare;
		Flare->solid = SOLID_NOT;
		Flare->movetype = MOVETYPE_NONE;
		Flare->className = "drop_sprite";

		Flare->s.renderfx |= (SPR_ALPHACHANNEL);

		Flare->s.modelindex = gstate->ModelIndex("models/global/e_flare4x.sp2");

		Flare->s.render_scale.Set(.50, .50, .50);
		Flare->s.alpha = 0.50f;
		Flare->s.color.Set(1.0f, 0.50f, 0.50f);
		Flare->delay = gstate->time + delay;
		Flare->think = SIDEKICK_Markers_Think;

		gstate->SetSize(Flare, -2, -2, -4, 2, 2, 4);
		gstate->SetOrigin2(Flare, Target);
		gstate->LinkEntity(Flare);

		Flare->s.frame = 0;
		Flare->nextthink = gstate->time + 0.01f;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_ParseCommand
// Description:
// Input:
// Output:
// Note:
//	searches through the passed string for meaningful commands
//	if a valid command is found, it's bit is set in commandFlag
//	commandFlag is returned
//
//	this does a lot of string compares, so it should not be used except
//	when the player issues a command from the console or a key binding
//
// ----------------------------------------------------------------------------
//#define SIDEKICK_COMMAND_YES            0x00000001
//#define SIDEKICK_COMMAND_NO             0x00000002
//#define SIDEKICK_COMMAND_STAY           0x00000004
//#define SIDEKICK_COMMAND_FOLLOW         0x00000008
//#define SIDEKICK_COMMAND_ATTACK         0x00000010
//#define SIDEKICK_COMMAND_NOATTACK       0x00000020
//#define SIDEKICK_COMMAND_COMEHERE       0x00000040
//#define SIDEKICK_COMMAND_PICKUP         0x00000080
int FindClientInPVS(userEntity_t* self);
// NSS[2/24/00]:Hack hack hack... don't want to use any more values in the userEntity or hook structure... got WAY too many as it is.
void SIDEKICK_ParseCommand(userEntity_t* self, char* str)
{
	playerHook_t* hook = AI_GetPlayerHook(self);

	if (!hook || !hook->owner || hook->bAttackPlayer == TRUE)
		return;

	unsigned long nCommandType = 0;


	// NSS[2/26/00]:Prevents too many commands coming at once.
	if (AI_IsSuperfly(hook) && gstate->time > Superflywait)
	{
		Superflywait = gstate->time + 1.10f;
	}
	else if (AI_IsMikiko(hook) && gstate->time > MikikoWait)
	{
		MikikoWait = gstate->time + 1.10f;
	}
	else
	{
		return;
	}

	userEntity_t* pOwner = hook->owner;
	if (!AI_IsAlive(pOwner) || self->groundEntity == NULL)
		return;


	// NSS[2/5/00]:If sidekicks cannot see you and we are not in developer mode then don't respond or do a damned thing!
	if (!FindClientInPVS(self) && !gstate->GetCvar("developer"))
	{
		return;
	}

	if (_stricmp(str, aSidekickCommandString[SIDEKICK_COMMAND_YES][0]) == 0)
	{
		nCommandType = SIDEKICK_COMMAND_YES;
	}
	else
		if (_stricmp(str, aSidekickCommandString[SIDEKICK_COMMAND_NO][0]) == 0)
		{
			nCommandType = SIDEKICK_COMMAND_NO;
		}
		else
			if (_stricmp(str, aSidekickCommandString[SIDEKICK_COMMAND_STAY][0]) == 0)
			{
				nCommandType = SIDEKICK_COMMAND_STAY;
			}
			else
				if (_stricmp(str, aSidekickCommandString[SIDEKICK_COMMAND_FOLLOW][0]) == 0)
				{
					nCommandType = SIDEKICK_COMMAND_FOLLOW;
				}
				else
					if (_stricmp(str, aSidekickCommandString[SIDEKICK_COMMAND_ATTACK][0]) == 0)
					{
						nCommandType = SIDEKICK_COMMAND_ATTACK;
					}
					else
						if (_stricmp(str, aSidekickCommandString[SIDEKICK_COMMAND_NOATTACK][0]) == 0)
						{
							nCommandType = SIDEKICK_COMMAND_NOATTACK;
						}
						else
							if (_stricmp(str, aSidekickCommandString[SIDEKICK_COMMAND_COMEHERE][0]) == 0)
							{
								nCommandType = SIDEKICK_COMMAND_COMEHERE;
							}
							else
								if (_stricmp(str, aSidekickCommandString[SIDEKICK_COMMAND_PICKUP][0]) == 0)
								{
									nCommandType = SIDEKICK_COMMAND_PICKUP;
								}


	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
	_ASSERTE(pCurrentGoal);
	GOALTYPE nGoalType = GOAL_GetType(pCurrentGoal);

	if (nCommandType == SIDEKICK_COMMAND_YES)
	{
		SIDEKICK_PermissionGiven(self);
	}
	else
		if (nCommandType == SIDEKICK_COMMAND_NO)
		{
			SIDEKICK_PermissionDenied(self);
		}
		else
			if (nCommandType == SIDEKICK_COMMAND_STAY)
			{
				hook->ai_flags &= ~AI_FOLLOW;
				hook->ai_flags |= AI_STAY;

				if (nGoalType == GOALTYPE_STAY)
				{
					AI_AddInitialTasksToGoal(self, pCurrentGoal, TRUE);
				}
				else
				{
					//	get rid of all goals
					GOALSTACK_ClearAllGoals(pGoalStack);
					AI_AddNewGoal(self, GOALTYPE_STAY);
				}
				SIDEKICK_CommandAcknowledge(self, SIDEKICK_COMMAND_STAY); // respond to given command
			}
			else
				if (nCommandType == SIDEKICK_COMMAND_FOLLOW)
				{
					hook->ai_flags &= ~AI_STAY;
					hook->ai_flags |= AI_FOLLOW;

					if (nGoalType == GOALTYPE_FOLLOW)
					{
						AI_AddInitialTasksToGoal(self, pCurrentGoal, TRUE);
					}
					else
					{
						//	get rid of all goals
						GOALSTACK_ClearAllGoals(pGoalStack);
						AI_AddNewGoal(self, GOALTYPE_FOLLOW, hook->owner);
					}

				}
				else
					if (nCommandType == SIDEKICK_COMMAND_ATTACK)
					{
						hook->ai_flags &= ~AI_NOATTACK;
						hook->ai_flags |= AI_ATTACK;
						hook->ai_flags |= AI_SIDEKICKKILL;


						if (hook->bCarryingMikiko)
						{
							SideKick_TalkAmbient(self, NO, 1);
							SIDEKICK_SendMessage(self, sCom_Speak, NO, gstate->time + 2.0f, 0.0f, self, COM_Read);
							return;
						}

						if (SIDEKICK_AttackOwnerFacingEnemy(self) && SIDEKICK_CheckMonsterType(self, self->enemy))
						{
							// NSS[2/29/00]:If we can kill and is ok to fight
							if (SIDEKICK_IsOkToFight(self) && SIDEKICK_CanPathToOrIsCloseToEnemy(self, self->enemy))
							{
								hook->ai_flags |= AI_SIDEKICKKILL;
								if (!SIDEKICK_HasTaskInQue(self, TASKTYPE_SIDEKICK_ATTACK))
								{
									AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_ATTACK);
								}
								return;
							}
							else
							{
								SideKick_TalkAmbient(self, NO, 1);
								hook->ai_flags &= ~AI_SIDEKICKKILL;
								self->enemy = NULL;
								SIDEKICK_SendMessage(self, sCom_Speak, NO, gstate->time + 1.0f, 0.0f, self, COM_Read);
								return;
							}
						}
						else
						{
							hook->ai_flags &= ~AI_SIDEKICKKILL;
						}

						SIDEKICK_CommandAcknowledge(self, SIDEKICK_COMMAND_ATTACK); // respond to given command
					}
					else
						if (nCommandType == SIDEKICK_COMMAND_NOATTACK)
						{
							hook->ai_flags &= ~AI_ATTACK;
							hook->ai_flags |= AI_NOATTACK;

							SIDEKICK_CommandAcknowledge(self, SIDEKICK_COMMAND_NOATTACK); // respond to given command

							// NSS[12/7/99]:Adding the backoff stuff
							if (hook->owner)
							{
								// try to evade from fighting
								SIDEKICK_HandleMoveAway(self, hook->owner);
								hook->ai_flags &= ~AI_STAY;
							}
						}
						else
							if (nCommandType == SIDEKICK_COMMAND_COMEHERE)
							{

								hook->ai_flags &= ~AI_STAY;
								hook->ai_flags |= AI_FOLLOW;

								hook->ai_flags |= AI_COMEHERE;

								// find a point in directly front of the owner
								userEntity_t* pOwner = hook->owner;
								if (pOwner)
								{
									if (!AI_IsAlive(pOwner) || (pOwner->deadflag != DEAD_NO))
										return;

								}
								else// NSS[12/7/99]:No owner get out.
								{
									return;
								}
								CVector vector;
								pOwner->s.angles.AngleToForwardVector(vector);

								CVector position;
								VectorMA(pOwner->s.origin, vector, 64.0f, position);

								CVector groundPoint;
								if (SIDEKICK_FindClearSpotFrom(self, pOwner->s.origin, groundPoint))
								{
									if (!AI_FindPathToPoint(self, groundPoint))
									{
										float fZDistance = VectorZDistance(self->s.origin, groundPoint);
										float fXYDistance = VectorXYDistance(self->s.origin, groundPoint);
										if (!AI_IsOkToMoveStraight(self, groundPoint, fXYDistance, fZDistance))
										{
											if (!Check_Que(self, CANTREACH, 1.5f))
											{
												SideKick_TalkAmbient(self, CANTREACH, 1);
												SIDEKICK_SendMessage(self, sCom_Speak, NO, gstate->time + 1.5f, 0.0f, self, COM_Read);
											}
											return;
										}
									}
#ifdef _DEBUG
									AI_Drop_Markers(groundPoint, 2.0f);
#endif
									if (nGoalType == GOALTYPE_STAY)
									{
										if (hook->owner)
										{
											GOALSTACK_ClearAllGoals(pGoalStack);
											AI_AddNewGoal(self, GOALTYPE_FOLLOW, hook->owner);
											GOALSTACK_PTR pGoalStackT = AI_GetCurrentGoalStack(hook);// NSS[3/10/00]:This is a double check
											GOAL_PTR pCurrentGoalT = GOALSTACK_GetCurrentGoal(pGoalStackT);
											if (pCurrentGoalT->nGoalType == GOALTYPE_FOLLOW)
												AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_COMEHERE, groundPoint);
										}
									}
									else
									{
										GOAL_ClearTasks(pCurrentGoal);
										AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_COMEHERE, groundPoint);
									}
								}
								else
								{
									if (!Check_Que(self, CANTREACH, 3.0f))
									{
										SideKick_TalkAmbient(self, CANTREACH, 1);
										SIDEKICK_SendMessage(self, sCom_Speak, NO, gstate->time + 3.0f, 0.0f, self, COM_Read);
										return;
									}
								}

								// NSS[2/9/00]:Checking health and soon to be ammo if low health or ammo say so and exit out.(move to seperate function)
								if (self->health < hook->base_health * 0.20)
								{
									if (!Check_Que(self, LOW_HEALTH, 70.0f))
									{
										SideKick_TalkAmbient(self, HEALTH_RESPONSE5, 1);
										SIDEKICK_SendMessage(self, sCom_Speak, LOW_HEALTH, gstate->time + 15.0f, 0.0f, self, COM_Read);
										return;
									}
								}
								else if (AI_IsMikiko(hook))// NSS[2/29/00]:Otherwise check for no ammo...
								{
									if (!(saveFlags & MK_GOOD_AMMO) && !(saveFlags & MK_ROCKIN_AMMO))
									{
										if (!Check_Que(self, NO_AMMO, 70.0f))
										{
											SideKick_TalkAmbient(self, NO_AMMO, 1);
											SIDEKICK_SendMessage(self, sCom_Speak, NO_AMMO, gstate->time + 15.0f, 0.0f, self, COM_Read);
											return;
										}
									}
								}
								else if (AI_IsSuperfly(hook) && !hook->bCarryingMikiko)
								{
									if (!(saveFlags & SF_GOOD_AMMO) && !(saveFlags & SF_ROCKIN_AMMO))
									{
										if (!Check_Que(self, NO_AMMO, 70.0f))
										{
											SideKick_TalkAmbient(self, NO_AMMO, 1);
											SIDEKICK_SendMessage(self, sCom_Speak, NO_AMMO, gstate->time + 15.0f, 0.0f, self, COM_Read);
											return;
										}
									}
								}
								SIDEKICK_CommandAcknowledge(self, SIDEKICK_COMMAND_COMEHERE); // respond to given command
							}
							else
								if (nCommandType == SIDEKICK_COMMAND_PICKUP)
								{

									userEntity_t* pItem = ITEM_GetTargetedItem(self);
									if (pItem)
									{
										int ret = SIDEKICK_CanPickupItem(self, pItem, TRUE);
										if (ret == TRUE)
										{
											/*TASK_PTR pTask = GOAL_GetCurrentTask( pCurrentGoal );
											AIDATA_PTR pAIData = TASK_GetData( pTask );
											TASKTYPE nTaskType = TASK_GetType( pTask );
											if ( !(nTaskType == TASKTYPE_SIDEKICK_PICKUPITEM && pAIData->pEntity == pItem) )
											{
												AI_AddNewTaskAtFront( self, TASKTYPE_SIDEKICK_PICKUPITEM, pItem );
											}*/
											float fZDistance = VectorZDistance(self->s.origin, pItem->s.origin);
											float fXYDistance = VectorXYDistance(self->s.origin, pItem->s.origin);

											if (!AI_FindPathToPoint(self, pItem->s.origin) || (fXYDistance < 64.0f && fZDistance <= 32.0f))
											{
												if (!SIDEKICK_IsOkToMoveStraight(self, pItem->s.origin, fXYDistance, fZDistance))
												{
													if (!Check_Que(self, CANTREACH, 1.5f))
													{
														SideKick_TalkAmbient(self, CANTREACH, 1);
														SIDEKICK_SendMessage(self, sCom_Speak, NO, gstate->time + 1.5f, 0.0f, self, COM_Read);
													}
													return;
												}
											}
											if (!AI_HasGoalInQueue(self, GOALTYPE_PICKUPITEM))
											{
												if (strstr(pItem->className, "misc_"))
												{
													AI_AddNewGoal(self, GOALTYPE_USEHOSPORTAL, pItem);
												}
												else
												{
													AI_AddNewGoal(self, GOALTYPE_PICKUPITEM, pItem);
												}
											}
											SIDEKICK_CommandAcknowledge(self, SIDEKICK_COMMAND_PICKUP); // respond to given command
										}
										else if (ret == FALSE)
										{
											SIDEKICK_CommandAcknowledge(self, SIDEKICK_COMMAND_PICKUP_NO); // respond to given command
										}
										else
										{
											if (!Check_Que(self, CANTREACH, 1.5f))
											{
												SideKick_TalkAmbient(self, CANTREACH, 1);
												SIDEKICK_SendMessage(self, sCom_Speak, CANTREACH, gstate->time + 1.5f, 0.0f, self, COM_Read);
												return;
											}
										}
									}
									else
									{
										if (!Check_Que(self, NO, 1.5f))
										{
											SideKick_TalkAmbient(self, NO, 1);
											SIDEKICK_SendMessage(self, sCom_Speak, CANTREACH, gstate->time + 1.5f, 0.0f, self, COM_Read);
											return;
										}
									}
								}
}




void SIDEKICK_CommandAcknowledge(userEntity_t* self, int command_type, qboolean PVS_required)
// play a sound in response to a command given (which will also show subtitle text)
//NSS[12/2/99]:
// file name format: [initial]_cmd_[command abbrev.]_[#].mp3
//
{
	char buf[256];
	char filepath[256];
	playerHook_t* hook = AI_GetPlayerHook(self);

	if (hook == NULL)
	{
		return;
	}
	if (hook->owner == NULL)
	{
		SIDEKICK_FindOwner(self);
	}
	if (hook->owner == NULL)
	{
		return;
	}

	if (Check_Que(self, NO_TALK, 1.0f))
	{
		return;
	}
	if (!command_type)
		return;

	if (PVS_required)                                            // owner/sidekick must be in PVS?
		if (!gstate->inPVS(self->s.origin, hook->owner->s.origin))
			return;                                                  // not so, so don't speak


	if (AI_IsMikiko(hook))
		strcpy(filepath, "sounds/voices/mikiko/cmd_mi_");
	else
		if (AI_IsSuperfly(hook))
			strcpy(filepath, "sounds/voices/superfly/cmd_su_");

	//NSS[12/2/99]:Nice reference
	//#define SIDEKICK_COMMAND_YES            1
	//#define SIDEKICK_COMMAND_NO             2
	//#define SIDEKICK_COMMAND_STAY           3
	//#define SIDEKICK_COMMAND_FOLLOW         4
	//#define SIDEKICK_COMMAND_ATTACK         5
	//#define SIDEKICK_COMMAND_NOATTACK       6
	//#define SIDEKICK_COMMAND_COMEHERE       7
	//#define SIDEKICK_COMMAND_PICKUP         8
	//#define SIDEKICK_COMMAND_PICKUP_BAD     9
	//#define SIDEKICK_COMMAND_PICKUP_NO	  10
	//#define SIDEKICK_COMMAND_COMMAND		  11
	//#define SIDEKICK_COMMAND_LAST			  12

	//NSS[12/2/99]:Setup the commands to play
	switch (command_type)
	{
	case SIDEKICK_COMMAND_COMEHERE:
	{
		Com_sprintf(buf, sizeof(buf), "%sco_0%d.mp3", filepath, (int)(rnd() * 3) + 1);
		break;
	}
	case SIDEKICK_COMMAND_STAY:
	{
		Com_sprintf(buf, sizeof(buf), "%sst_0%d.mp3", filepath, (int)(rnd() * 3) + 1);
		break;
	}
	case SIDEKICK_COMMAND_ATTACK:
	{
		Com_sprintf(buf, sizeof(buf), "%sat_0%d.mp3", filepath, (int)(rnd() * 3) + 1);
		break;
	}
	case SIDEKICK_COMMAND_NOATTACK:
	{
		Com_sprintf(buf, sizeof(buf), "%sba_0%d.mp3", filepath, (int)(rnd() * 3) + 1);
		break;
	}
	case SIDEKICK_COMMAND_PICKUP:
	{
		Com_sprintf(buf, sizeof(buf), "%spu_0%d.mp3", filepath, (int)(rnd() * 3) + 1);
		break;
	}
	case SIDEKICK_COMMAND_PICKUP_NO:
	{
		Com_sprintf(buf, sizeof(buf), "%spu_0%d_n1.mp3", filepath, (int)(rnd() * 3) + 1);
		break;
	}
	case SIDEKICK_COMMAND_YES:
	{
		Com_sprintf(buf, sizeof(buf), "%sye_0%d.mp3", filepath, (int)(rnd() * 3) + 1);
		break;
	}
	case SIDEKICK_COMMAND_NO:
	{

		Com_sprintf(buf, sizeof(buf), "%sno_0%d.mp3", filepath, (int)(rnd() * 2) + 2);
		break;
	}

	default://NSS[12/2/99]:Got a command not in here?  Let's not play it.
	{
		buf[0] = NULL;
		break;
	}
	}
	//NSS[12/2/99]:Play it!
	if (buf[0] != NULL)
		PlaySidekickMP3(self, buf);
	//gstate->StartMP3(buf, 2, 0.65, 1 );

}


// ----------------------------------------------------------------------------
// NSS[1/31/00]:
// Name:		SIDEKICK_Die
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_Die(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);
	userEntity_t* client_entity = &gstate->g_edicts[1];
	ai_frame_sounds(self);

	frameData_t* pCurrentSequence = hook->cur_sequence;


	if (pCurrentSequence && !strstr(pCurrentSequence->animation_name, "die"))
	{
		AI_ForceSequence(self, "diea");
		self->deadflag = DEAD_DYING;
	}
	else if (!pCurrentSequence)
	{
		self->deadflag = DEAD_DEAD;
		self->die = NULL;
		self->prethink = NULL;
		self->postthink = NULL;
		self->think = NULL;

		if (hook->pGoals)
		{
			GOALSTACK_Delete(hook->pGoals);
			hook->pGoals = NULL;
		}
		if (hook->pScriptGoals)
		{
			GOALSTACK_Delete(hook->pScriptGoals);
			hook->pScriptGoals = NULL;
		}

		if (self->waterlevel > 2)
		{
			int sound_index = rand() % 2;
			sound_index += gstate->SoundIndex("hiro/waterland4.wav");

			gstate->StartEntitySound(self, CHAN_AUTO, sound_index, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
		}

		UNIQUEID_Remove(hook->szScriptName);
		free(hook->szScriptName);
		hook->szScriptName = NULL;

		alist_remove(self);

		gstate->LinkEntity(self);

		AI_Dprintf("Finished TASKTYPE_SIDEKICK_DIE.\n");

		if ((client_entity->inuse != 0) && (client_entity->deadflag == DEAD_NO))
		{
			AlertClientOfSidekickDeath(false); // game over  (false == not gibbed)
		}
	}

	if (self->deadflag == DEAD_DYING)
	{

		if (!strstr(pCurrentSequence->animation_name, "die") ||
			(self->s.frameInfo.frameState & FRSTATE_LAST ||
				self->s.frameInfo.frameState & FRSTATE_STOPPED))
		{
			self->deadflag = DEAD_DEAD;
			self->die = NULL;
			self->prethink = NULL;
			self->postthink = NULL;
			self->think = NULL;

			if (hook->pGoals)
			{
				GOALSTACK_Delete(hook->pGoals);
				hook->pGoals = NULL;
			}
			if (hook->pScriptGoals)
			{
				GOALSTACK_Delete(hook->pScriptGoals);
				hook->pScriptGoals = NULL;
			}

			if (self->waterlevel > 2)
			{
				int sound_index = rand() % 2;
				sound_index += gstate->SoundIndex("hiro/waterland4.wav");

				gstate->StartEntitySound(self, CHAN_AUTO, sound_index, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
			}

			UNIQUEID_Remove(hook->szScriptName);
			free(hook->szScriptName);
			hook->szScriptName = NULL;

			alist_remove(self);

			gstate->LinkEntity(self);

			AI_Dprintf("Finished TASKTYPE_SIDEKICK_DIE.\n");

			if ((client_entity->inuse != 0) && (client_entity->deadflag == DEAD_NO))
			{
				AlertClientOfSidekickDeath(false); // game over  (false == not gibbed)
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_StartDie
// Description:
//			called from com->Damage when a player is killed
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartDie(userEntity_t* self, userEntity_t* inflictor,
	userEntity_t* attacker, int damage, CVector& point)
{
	userEntity_t* client_entity = &gstate->g_edicts[1];
	playerHook_t* phook = AI_GetPlayerHook(client_entity);

	playerHook_t* hook = AI_GetPlayerHook(self);
	if (hook && AI_IsSidekick(hook) && sv_sidekick_invincible && sv_sidekick_invincible->value > 0.0f)
	{
		self->takedamage = DAMAGE_NO;
		return;
	}
	userEntity_t* pCameraTarget = NULL;

	int Chan = 0;

	if (hook && AI_IsSidekick(hook))
	{
		Chan = 4;

	}
	else
	{
		Chan = 3;
	}

	gstate->StopMP3(Chan, NULL);

	if (hook && hook->owner)
	{
		char deathmsg[80];
		Com_sprintf(deathmsg, sizeof(deathmsg), "%s%s", com->ProperNameOf(self), tongue_deathmsg_self[NONE]);
		gstate->centerprint(hook->owner, 2.0, deathmsg);
	}

	if (self->deadflag == DEAD_NO)
	{
		SIDEKICK_DropInventory(self);
	}

	// NSS[1/31/00]:Episode3 water death
	if ((self->watertype & CONTENTS_WATER) && attacker == self && inflictor == self && (GetCurrentEpisode() == 3))
	{
		self->flags |= FL_FREEZE;
		self->s.color.Set(0.0, 0.0, 0.65);
		self->s.alpha = 0.75;
		self->s.renderfx |= SPR_ALPHACHANNEL;
	}


	// self->solid			= SOLID_NOT;    //	do not clip any longer!
	self->solid = SOLID_BBOX;
	self->groundEntity = NULL;
	self->movetype = MOVETYPE_BOUNCE;
	self->svflags |= SVF_DEADMONSTER;	//&&& AMW 7/17/98 - as per quake2
	self->armor_val = 0;
	self->armor_abs = 0;

	// lock the angles of the entity so he can't rotate or spin
	self->s.angles.x = 0.0f;
	self->s.angles.z = 0.0f;


	if ((client_entity->inuse != 0) && (client_entity->deadflag == DEAD_NO))
	{
		if ((phook != NULL) && (phook->camera))  // if previous camera in use, stop it
		{
			camera_stop(client_entity, false);
		}

		camera_start(client_entity, false);	// prepare camera
	}

	if (self->deadflag == DEAD_NO)
	{
		if (self->waterlevel > 2)
		{
			SIDEKICK_PlaySound(self, "waterdeath.wav");
		}
		else
			if (self->health < -40)
			{
				SIDEKICK_PlaySound(self, "udeath.wav");
			}
			else
			{
				switch (rand() % 4)
				{
				case 0: SIDEKICK_PlaySound(self, "death1.wav"); break;
				case 1: SIDEKICK_PlaySound(self, "death2.wav"); break;
				case 2: SIDEKICK_PlaySound(self, "death3.wav"); break;
				case 3: SIDEKICK_PlaySound(self, "death4.wav"); break;
				}
			}

		if (!(self->flags & FL_FREEZE) && check_for_gib(self, damage) && sv_violence == 0)
		{
			//	make sure we're not drawing the player model
			self->s.renderfx |= RF_NODRAW;

			// dismember this dead bastard
			//NSS[11/23/99]:Violence stuff (0 = violence anything else = none)
			//if(sv_violence == 0)
			//{
			pCameraTarget = gib_client_die(self, inflictor);
			//}

			//If we are not dead yet
			if ((client_entity->inuse != 0) && (client_entity->deadflag == DEAD_NO))
			{
				if (pCameraTarget)
				{
					camera_set_target(phook->camera, pCameraTarget);
					camera_mode(client_entity, CAMERAMODE_GIBCAM, false);
				}
			}

			self->die = NULL;
			self->prethink = NULL;
			self->postthink = NULL;
			self->think = NULL;
			self->deadflag = DEAD_DEAD;

			if (hook->pGoals)
			{
				GOALSTACK_Delete(hook->pGoals);
				hook->pGoals = NULL;
			}
			if (hook->pScriptGoals)
			{
				GOALSTACK_Delete(hook->pScriptGoals);
				hook->pScriptGoals = NULL;
			}

			UNIQUEID_Remove(hook->szScriptName);
			free(hook->szScriptName);
			hook->szScriptName = NULL;

			alist_remove(self);
			self->remove(self);          // remove if gibbed
			gstate->LinkEntity(self);

			//			if(client_entity->deadflag == DEAD_NO)
			//			{
			//				AlertClientOfSidekickDeath(false); // game over (true == gibbed)
			//			}

			return;
		}
		else
		{
			// set up frozen state
			if (self->flags & FL_FREEZE)
			{
				AI_StopCurrentSequence(self); // stop animation dead

				self->solid = SOLID_BBOX;    // make the player dead but solid as a rock
				self->movetype = MOVETYPE_BOUNCE; // get pushy since we're frozen
				self->deadflag = DEAD_FROZEN;
			}
			else
			{
				AI_ForceSequence(self, "diea");

				// die task need to be inserted since the die task gets triggered from outside 
				// the AI framework
				GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
				GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
				if (!pCurrentGoal)
				{
					AI_AddNewGoal(self, GOALTYPE_IDLE);
				}
				AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_DIE);
			}

			pCameraTarget = self; // set the camera to this entity

			self->deadflag = DEAD_DYING;
		}

		//if we are not dead yet.
		if ((client_entity->inuse != 0) && (client_entity->deadflag == DEAD_NO))
		{
			camera_set_target(phook->camera, pCameraTarget);
			camera_mode(client_entity, CAMERAMODE_GIBCAM, false);
		}
	}
	else
		if (self->deadflag == DEAD_FROZEN)
		{
			self->solid = SOLID_BBOX;       // frozen solid
			self->movetype = MOVETYPE_BOUNCE; // push technology

			pCameraTarget = self;

			//NSS[11/23/99]:IF we are not dead yet.
			if ((client_entity->inuse != 0) && (client_entity->deadflag == DEAD_NO))
			{
				camera_set_target(phook->camera, pCameraTarget);
				camera_mode(client_entity, CAMERAMODE_GIBCAM, false);
			}

			self->die = NULL;
			self->prethink = NULL;
			self->postthink = NULL;
			self->think = NULL;
			self->deadflag = DEAD_DEAD;

			if (hook->pGoals)
			{
				GOALSTACK_Delete(hook->pGoals);
				hook->pGoals = NULL;
			}
			if (hook->pScriptGoals)
			{
				GOALSTACK_Delete(hook->pScriptGoals);
				hook->pScriptGoals = NULL;
			}

			UNIQUEID_Remove(hook->szScriptName);
			free(hook->szScriptName);
			hook->szScriptName = NULL;

			alist_remove(self);
			gstate->LinkEntity(self);

		}
	//NSS[11/30/99]:There can only be one!  :P
	if ((client_entity->inuse != 0) && (client_entity->deadflag == DEAD_NO))
	{
		AlertClientOfSidekickDeath(false); // game over  (false == not gibbed)
	}

	AI_SetNextThinkTime(self, 0.1f);
	AI_SetOkToAttackFlag(hook, FALSE);
	AI_SetTaskFinishTime(hook, 5.0f);
}








// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_Pain
// Description:
//			initializes the client data structures
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_Pain(userEntity_t* self, userEntity_t* pAttacker, float kick, int damage)
{
	_ASSERTE(self);

	char soundname[MAX_QPATH];

	playerHook_t* hook = AI_GetPlayerHook(self);
	userEntity_t* pCameraTarget = NULL;
	userEntity_t* pOtherSidekick = SIDEKICK_GetOtherSidekick(self);
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask(pGoalStack);


	// NSS[12/5/99]:Check to see if Hiro is the one doing the smacking... make sure we will still be alive to speak.
	if (hook && ((self->health - damage) > 0))
	{
		// NSS[12/6/99]:First thing we do is clear any command to stay... if something is busting us down let's kick ass!
		if (pAttacker == hook->owner)
		{
			if (rnd() < 0.50 || damage > 10.0f)
			{
				if (!Check_Que(self, HITBY_HIRO, 3.0f))
				{
					SideKick_TalkAmbient(self, HITBY_HIRO, 1);
					SIDEKICK_SendMessage(self, sCom_Speak, HITBY_HIRO, gstate->time + 3, 0.0f, self, COM_Read);
				}
			}
		}
		else if (pAttacker == pOtherSidekick)
		{
			if (rnd() < 0.50 || damage > 10.0f)
			{
				if (!Check_Que(self, HITBY_SIDEKICK, 3.0f))
				{
					SideKick_TalkAmbient(self, HITBY_SIDEKICK, 1);
					SIDEKICK_SendMessage(self, sCom_Speak, HITBY_SIDEKICK, gstate->time + 3, 0.0f, self, COM_Read);
				}

			}
		}
		else// NSS[12/7/99]:A monster is hitting us... 
		{
			if (SIDEKICK_HasTaskInQue(self, TASKTYPE_SIDEKICK_EVADE))
			{
				if (pCurrentTask)
				{
					if (pCurrentTask->nTaskType == TASKTYPE_SIDEKICK_STOP)
					{
						AI_RemoveCurrentTask(self, FALSE);
					}
				}
			}
		}
		// NSS[2/12/00]:Test to see what is hurting us
		if (!(pAttacker->flags & (FL_BOT | FL_CLIENT | FL_MONSTER)))
		{
			// NSS[2/12/00]:If it is a door smashing us let's try and get out of its way
			if (pAttacker->className && (strstr(pAttacker->className, "door") || strstr(pAttacker->className, "train")))
			{
				if (!SIDEKICK_HasTaskInQue(self, TASKTYPE_MOVEAWAY))
				{
					AI_AddNewTaskAtFront(self, TASKTYPE_MOVEAWAY, pAttacker);
				}
				else
				{
					CVector dir;
					self->s.angles.AngleToForwardVector(dir);
					dir.Negate();
					AI_FindNewCourse(self, &dir);
				}
			}
		}
	}

	if (pCurrentTask)
	{
		if (TASK_GetType(pCurrentTask) == TASKTYPE_WAITONLADDERFORNOCOLLISION)
		{
			self->movetype = MOVETYPE_WALK;
		}

		if (TASK_GetType(pCurrentTask) != TASKTYPE_SIDEKICK_ATTACK)
		{
			if (pAttacker)
			{
				if ((pAttacker->flags & FL_MONSTER) && !hook->bCarryingMikiko && !SIDEKICK_HasTaskInQue(self, TASKTYPE_SIDEKICK_ATTACK) && SIDEKICK_IsOkToFight(self) && SIDEKICK_CheckMonsterType(self, pAttacker))
				{
					AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_ATTACK);
				}
			}
		}
	}

	// NSS[12/6/99]:Remove any stay or no attack command.
	hook->ai_flags &= ~AI_STAY;
	hook->ai_flags &= ~AI_NOATTACK;
	hook->ai_flags |= AI_ATTACK;
	hook->ai_flags |= AI_FOLLOW;

	weapon_t* pWeapon = (weapon_t*)self->curWeapon;

	if (pWeapon)
	{
		if (SIDEKICK_CanUseWeapon(pWeapon))
		{
			hook->bInAttackMode = TRUE;
		}
	}

	AI_SetNextThinkTime(self, 0.1f);
	AI_SetOkToAttackFlag(hook, FALSE);
	AI_SetTaskFinishTime(hook, 3.0f);
	AI_SetMovingCounter(hook, 0);

	if (self->health < 0.0f)
	{
		if (!(self->flags & FL_FREEZE) && check_for_gib(self, damage)) // able to gib?// NSS[3/8/00]:Medusa was gibbing us and this was the problem added the check for FL_FREEZE
		{

			userEntity_t* client_entity = &gstate->g_edicts[1];
			playerHook_t* phook = AI_GetPlayerHook(client_entity);


			if ((client_entity->inuse != 0) && (client_entity->deadflag == DEAD_NO))
			{
				if (phook->camera)  // if previous camera in use, stop it
					camera_stop(client_entity, false);

				camera_start(client_entity, false);	// prepare camera
			}


			self->s.renderfx = RF_NODRAW;

			// dismember this dead bastard
			pCameraTarget = gib_client_die(self, pAttacker);

			if (client_entity->inuse != 0)
			{
				camera_set_target(phook->camera, pCameraTarget);
				camera_mode(client_entity, CAMERAMODE_GIBCAM, false);
			}

			if (hook->pRespawnData)
			{
				self->think = SPAWN_RespawnThink;
				AI_SetNextThinkTime(self, 5.0f);
			}
			else
			{
				self->deadflag = DEAD_DEAD;
				self->pain = NULL;
				self->use = NULL;
				self->die = NULL;
				self->think = NULL;
				self->prethink = NULL;
				self->postthink = NULL;

				if (hook->pGoals)
				{
					GOALSTACK_Delete(hook->pGoals);
					hook->pGoals = NULL;
				}

				if (hook->pScriptGoals)
				{
					GOALSTACK_Delete(hook->pScriptGoals);
					hook->pScriptGoals = NULL;
				}

				UNIQUEID_Remove(hook->szScriptName);
				free(hook->szScriptName);
				hook->szScriptName = NULL;

				alist_remove(self);
				self->remove(self);

				if ((client_entity->inuse != 0) && (client_entity->deadflag == DEAD_NO))
				{
					AlertClientOfSidekickDeath(false); // game over  (true == gibbed)
				}
			}
		}

		return;
	}
	else
	{
		// play PAIN sounds
		// NSS[1/31/00]:Added water and lava stuff
		if ((self->watertype & CONTENTS_WATER) && pAttacker == self && (GetCurrentEpisode() == 3))
		{
			if (rnd() < 0.15)
			{
				Com_sprintf(soundname, sizeof(soundname), "icehurt%c.wav", ('1' + (rand() % 2)));
				SIDEKICK_PlaySound(self, soundname);
				if (self->s.color.x <= 0.0f || self->s.color.y <= 0.0f)
				{
					self->s.color.x = 1.0f;
					self->s.color.y = 1.0f;
					self->s.color.z = 0.25f;
				}
				if (self->s.color.z < 1.0)
				{
					self->s.color.z += 0.10;
					if (self->s.color.x > 0.10)
					{
						self->s.color.x -= 0.10;
						self->s.color.y -= 0.10;
					}
				}
			}


		}
		else if ((self->watertype & CONTENTS_WATER) && pAttacker == self)
		{
			if (rnd() < 0.15)
			{
				if (hook->type == TYPE_MIKIKO)
					Com_sprintf(soundname, sizeof(soundname), "waterchoke%c.wav", ('1' + ((rand() % 2) + 1)));
				else
					Com_sprintf(soundname, sizeof(soundname), "waterchoke%c.wav", ('1' + ((rand() % 2) + 2)));
				SIDEKICK_PlaySound(self, soundname);
			}
		}
		else if ((self->watertype & (CONTENTS_SLIME | CONTENTS_LAVA)) && pAttacker == self)
		{
			if (rnd() < 0.15)
			{
				Com_sprintf(soundname, sizeof(soundname), "death%c.wav", ('1' + ((rand() % 3) + 5)));
				SIDEKICK_PlaySound(self, soundname);
			}
		}
		else
		{
			Com_sprintf(soundname, sizeof(soundname), "pain%c.wav", ('1' + (rand() % 4)));
			SIDEKICK_PlaySound(self, soundname);
		}



	}

	// still in a pain function from last hit?
	if (hook->pain_finished >= gstate->time || self == pAttacker)
	{
		return;
	}

	// only attack monsters and do not attack player and other sidekicks
	_ASSERTE(pAttacker);
	if (pAttacker->flags & FL_MONSTER)
	{
		int new_enemy = FALSE;
		playerHook_t* ahook = AI_GetPlayerHook(pAttacker);

		if (pAttacker->flags & FL_MONSTER)
		{
			new_enemy = TRUE;
		}
		else
			//	bots only attack entities with opposite alignment
			if (hook->dflags & DFL_EVIL && (ahook->dflags & (DFL_GOOD | DFL_NEUTRAL)))
			{
				new_enemy = TRUE;
			}
			else
				if (hook->dflags & DFL_GOOD && (ahook->dflags & (DFL_EVIL | DFL_NEUTRAL)))
				{
					new_enemy = TRUE;
				}
		//	or anything if they are berserk
				else
					if (hook->dflags & DFL_BERSERK)
					{
						new_enemy = TRUE;
					}

		if (!ahook->pNodeList)
		{
			new_enemy = FALSE;
		}
		else
			if (ahook->pNodeList->nCurrentNodeIndex < 0)
			{
				new_enemy = FALSE;
			}

		if (new_enemy)
		{
			SIDEKICK_SetAttackMode(self, pAttacker);
		}
	}

	int nRandomValue = (int)(Random() * 99.9);
	if (nRandomValue < hook->pain_chance)
	{
		char szAnimation[16];
		strcpy(szAnimation, "hit");
		AI_SelectAnimationPerWeaponType(self, szAnimation);
		weaponInfo_t* pWeaponInfo = (weaponInfo_t*)self->winfo;
		if (pWeaponInfo)
		{
			if (pWeaponInfo->nWeaponType == WEAPONTYPE_GLOVE)
			{
				int nStrLength = strlen(szAnimation);
				szAnimation[nStrLength] = 'a';
				szAnimation[nStrLength + 1] = '\0';
			}
		}
		else
		{
			int nStrLength = strlen(szAnimation);
			szAnimation[nStrLength] = 'a';
			szAnimation[nStrLength + 1] = '\0';
		}
		frameData_t* pSequence = FRAMES_GetSequence(self, szAnimation);
		if (pSequence)
		{
			AI_ForceSequence(self, pSequence, FRAME_ONCE);

			AI_Dprintf("Starting TASKTYPE_PAIN.\n");
			ai_frame_sounds(self);
			AI_AddNewTaskAtFront(self, TASKTYPE_PAIN);
		}
	}
	hook->pain_finished = gstate->time + (self->s.frameInfo.endFrame - self->s.frameInfo.startFrame) * self->s.frameInfo.frameTime;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SetAttackSequence
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SetAttackSequence(userEntity_t* self)
{
	_ASSERTE(self);

	if (self->curWeapon)
	{
		//		playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
		char szAnimation[16];
		AI_SelectRunningAnimation(self, szAnimation);
		AI_ForceSequence(self, szAnimation, FRAME_LOOP);
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_IsGap
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsGap(userEntity_t* self, CVector& forwardVector)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	float fSpeed = AI_ComputeMovingSpeed(hook);
	float fDistance = fSpeed * 0.2f;

	CVector nextPoint;
	VectorMA(self->s.origin, forwardVector, fDistance, nextPoint);
	if (!AI_IsGroundBelowBetween(self, self->s.origin, nextPoint))
	{
		return TRUE;
	}
	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_MoveTowardPoint
// Description:
//				move straight toward a point
// Input:
// Output:
//				TRUE	=> if reached the point
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_MoveTowardPoint(userEntity_t* self, CVector& destPoint,
	int bMovingTowardNode /* = FALSE */, int bFaceTowardDest /* = TRUE */)
{
	_ASSERTE(self);

	if (AI_IsInWater(self))
	{
		return AI_SwimTowardPoint(self, destPoint, bMovingTowardNode);
	}

	// if this unit is going outside of its bounding box, then terminate current goal
	if (AI_IsInsideBoundingBox(self) == FALSE)
	{
		AI_RemoveCurrentGoal(self);
		return FALSE;
	}

	playerHook_t* hook = AI_GetPlayerHook(self);
	if (SIDEKICK_SmartHandleLadder(self))
		return FALSE;

	if (!AI_CanMove(hook))
	{
		AI_RemoveCurrentTask(self);
		return FALSE;
	}

	// we can just check the x,y distance since this function assumes that the 
	// dest point is visible from self
	float fXYDistance = VectorXYDistance(self->s.origin, destPoint);
	float fZDistance = VectorZDistance(self->s.origin, destPoint);
	if (AI_IsCloseDistance2(self, fXYDistance) && fZDistance < 32.0f)
	{
		AI_UpdateCurrentNode(self);
		return TRUE;
	}

	if (bFaceTowardDest)
	{
		AI_FaceTowardPoint(self, destPoint);
	}

	float fSpeed = AI_ComputeMovingSpeed(hook);

	if (!self->groundEntity)
	{
		if (AI_IsCloseDistance2(self, fXYDistance))
		{
			AI_UpdateCurrentNode(self);
			return TRUE;
		}

		if (self->waterlevel > 0 && self->waterlevel < 3)
		{
			float fHeight = 8.0f + ((float)self->waterlevel * 8.0f);
			if (fZDistance > fHeight)
			{
				// must be stuck trying to go out of the water, try to jump
				self->velocity.z = hook->upward_vel * 0.50;
			}
			else
			{
				forward = destPoint - self->s.origin;
				forward.Normalize();

				//	scale speed based on current frame's move_scale
				float fScale = FRAMES_ComputeFrameScale(hook);
				AI_SetVelocity(self, forward, (fSpeed * fScale));
			}

			AI_UpdateCurrentNode(self);

			return FALSE;
		}
		else
		{
			AI_HandleOffGround(self, destPoint, fSpeed);
		}

		return FALSE;
	}

	if (self->groundEntity && self->waterlevel == 2)
	{
		if (fZDistance > 24.0f)
		{
			// must be stuck trying to go out of the water, try to jump
			self->velocity.z = hook->upward_vel * 0.5f;
			self->groundEntity = NULL;
			AI_UpdateCurrentNode(self);
			return FALSE;
		}
	}

	forward = destPoint - self->s.origin;

	// should never be in water, but just in case
	int bOnFlatSurface = AI_IsOnFlatSurface(self);
	if (self->waterlevel < 3 && bOnFlatSurface && self->movetype != MOVETYPE_HOP)
	{
		forward.z = 0.0f;
	}
	forward.Normalize();

	if (AI_HandleCollisionWithEntities(self, destPoint, fSpeed))
	{
		AI_UpdateCurrentNode(self);
		return FALSE;
	}

	if (SIDEKICK_SmartJumpToLedge(self, destPoint))
	{
		AI_UpdateCurrentNode(self);
		return FALSE;
	}

	if (AI_HandleGroundObstacle(self, fSpeed, bMovingTowardNode))
	{
		AI_UpdateCurrentNode(self);
		return FALSE;
	}

	float fOneFrameDistance = fSpeed * 0.125f;
	if (!bMovingTowardNode && AI_IsGap(self, destPoint, fOneFrameDistance))
	{
		if (SIDEKICK_SmartRecoverMovement(self, destPoint))
			return FALSE;
		AI_StopEntity(self);
		return FALSE;
	}

	/* Final hull safety check: if the smart layer is about to drive directly
	 * into solid world geometry, hand movement back to the deterministic path
	 * system instead of pushing the sidekick into the wall. */
	{
		CVector safetyEnd = self->s.origin + (forward * 24.0f);
		trace_t safetyTrace = gstate->TraceBox_q2(
			self->s.origin, self->s.mins, self->s.maxs, safetyEnd, self, MASK_SOLID);
		if (safetyTrace.startsolid || safetyTrace.allsolid || safetyTrace.fraction < 0.20f)
		{
			if (AI_FindPathToPoint(self, destPoint) && hook->pPathList && hook->pPathList->pPath)
			{
				AI_Move(self);
			}
			else
			{
				SIDEKICK_SmartRecoverMovement(self, destPoint);
			}
			AI_UpdateCurrentNode(self);
			return FALSE;
		}
	}

	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale(hook);
	AI_SetVelocity(self, forward, (fSpeed * fScale));

	if (AI_HandleGettingStuck(self, destPoint, fSpeed))
	{
		AI_UpdateCurrentNode(self);
		return FALSE;
	}

	hook->last_origin = self->s.origin;

	AI_UpdateCurrentNode(self);

	AI_HandleCrouching(self);

	ai_frame_sounds(self);
	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_MoveTowardPointWhileAttacking
// Description:
//				move straight toward a point
// Input:
// Output:
//				TRUE	=> if reached the point
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_MoveTowardPointWhileAttacking(userEntity_t* self, CVector& destPoint,
	int bMovingTowardNode /* = FALSE */, int bFaceTowardDest /* = TRUE */)
{
	_ASSERTE(self);

	if (AI_IsInWater(self))
	{
		return AI_SwimTowardPoint(self, destPoint, bMovingTowardNode);
	}

	playerHook_t* hook = AI_GetPlayerHook(self);
	if (SIDEKICK_SmartHandleLadder(self))
		return FALSE;

	// we can just check the x,y distance since this function assumes that the 
	// dest point is visible from self
	float fXYDistance = VectorXYDistance(self->s.origin, destPoint);
	float fZDistance = VectorZDistance(self->s.origin, destPoint);
	if (AI_IsCloseDistance2(self, fXYDistance) && fZDistance < 32.0f)
	{
		AI_UpdateCurrentNode(self);
		return TRUE;
	}

	userEntity_t* pEnemy = self->enemy;
	if (AI_IsAlive(pEnemy) && AI_IsLineOfSight(self, pEnemy))
	{
		SIDEKICK_FaceTowardPoint(self, pEnemy->s.origin);
	}
	else
	{
		if (bFaceTowardDest)
		{
			SIDEKICK_FaceTowardPoint(self, destPoint);
		}
	}

	float fSpeed = AI_ComputeMovingSpeed(hook);

	if (!self->groundEntity && self->waterlevel < 3)
	{
		if (AI_IsCloseDistance2(self, fXYDistance))
		{
			AI_UpdateCurrentNode(self);
			return TRUE;
		}

		return FALSE;
	}

	forward = destPoint - self->s.origin;

	// should never be in water, but just in case
	int bOnFlatSurface = AI_IsOnFlatSurface(self);
	if (self->waterlevel < 3 && bOnFlatSurface)
	{
		forward.z = 0.0f;
	}
	forward.Normalize();

	if (SIDEKICK_SmartJumpToLedge(self, destPoint))
	{
		AI_UpdateCurrentNode(self);
		return FALSE;
	}

	if (AI_HandleGroundObstacle(self, fSpeed, bMovingTowardNode))
	{
		AI_UpdateCurrentNode(self);
		return FALSE;
	}

	/*if ( SIDEKICK_IsGap( self, forward ) )
	{
		AI_StopEntity( self );
		return FALSE;
	}*/

	if (!bMovingTowardNode && AI_HandleGap(self, destPoint, fSpeed, fXYDistance, fZDistance))
	{
		AI_UpdateCurrentNode(self);
		return FALSE;
	}

	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale(hook);
	AI_SetVelocity(self, forward, (fSpeed * fScale));

	//	if ( AI_HandleGettingStuck( self, destPoint, fSpeed ) )
	//	{
	//		return FALSE;
	//	}

	hook->last_origin = self->s.origin;

	AI_UpdateCurrentNode(self);

	AI_HandleCrouching(self);

	ai_frame_sounds(self);

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_MoveWhileAttacking
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_MoveWhileAttacking(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	if (AI_IsInWater(self))
	{
		return AI_Swim(self);
	}

	if (hook->pPathList->pPath && hook->pPathList->pPath->nNodeIndex >= 0)
	{
		_ASSERTE(pGroundNodes);

		MAPNODE_PTR pNode = NODE_GetNode(pGroundNodes, hook->pPathList->pPath->nNodeIndex);
		_ASSERTE(pNode);

		CVector destPoint = pNode->position;
		if (SIDEKICK_MoveTowardPointWhileAttacking(self, destPoint, TRUE) == TRUE)
		{
			// we're already at this node, so delete the first one
			PATHLIST_DeleteFirstInPath(hook->pPathList);

			if (hook->pPathList->pPath)
			{
				MAPNODE_PTR pNextNode = NODE_GetNode(pGroundNodes, hook->pPathList->pPath->nNodeIndex);
				if (AI_HandleUse(self, pGroundNodes, pNode, pNextNode))
				{
					return TRUE;
				}
			}

			SIDEKICK_MoveWhileAttacking(self);
		}

		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_ModifyStrafeDirectionAfterCollision
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_ModifyStrafeDirectionAfterCollision(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);

	switch (hook->strafe_dir)
	{
	case STRAFE_LEFT:
	{
		hook->strafe_dir = STRAFE_RIGHT;
		break;
	}
	case STRAFE_RIGHT:
	{
		hook->strafe_dir = STRAFE_LEFT;
		break;
	}
	case STRAFE_UPLEFT:
	{
		hook->strafe_dir = STRAFE_DOWNRIGHT;
		break;
	}
	case STRAFE_UPRIGHT:
	{
		hook->strafe_dir = STRAFE_DOWNLEFT;
		break;
	}
	case STRAFE_DOWNLEFT:
	{
		hook->strafe_dir = STRAFE_UPRIGHT;
		break;
	}
	case STRAFE_DOWNRIGHT:
	{
		hook->strafe_dir = STRAFE_UPLEFT;
		break;
	}
	default:
	{
		break;
	}
	}

	hook->strafe_time = gstate->time + 3.0f;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_HandleCollision
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_HandleCollision(userEntity_t* self, float fSpeed, CVector& forwardDirection)
{
	if (SIDEKICK_IsGap(self, forwardDirection))
	{
		SIDEKICK_ModifyStrafeDirectionAfterCollision(self);
		return;
	}

	playerHook_t* hook = AI_GetPlayerHook(self);

	float fDist = fSpeed * 0.1f;
	if (self->velocity.Length() > 0)
	{
		fDist = fSpeed * 0.2f;
	}
	if (fDist < 32.0f)
	{
		fDist = 32.0f;
	}

	int nWallType = AI_CheckAirTerrain(self, forwardDirection, fDist);
	switch (nWallType)
	{
	case TER_OBSTRUCTED:
	{
		AI_Dprintf("Strafing along the wall.\n");

		CVector dir;
		AI_ComputeAlongWallVector(self, forwardDirection, terrain.wall_normal, dir);
		forwardDirection = dir;

		SIDEKICK_ModifyStrafeDirectionAfterCollision(self);
		break;
	}
	case TER_WALL:
	{
		AI_Dprintf("Strafing along the wall.\n");

		CVector dir;
		AI_ComputeAlongWallVector(self, forwardDirection, terrain.wall_normal, dir);
		forwardDirection = dir;

		SIDEKICK_ModifyStrafeDirectionAfterCollision(self);
		break;
	}

	case TER_HIGH_OBS:
	{
		gstate->Con_Dprintf("Detected high obstruction.\n");

		// if it is a ceiling then, glide along it
		if (terrain.wall_normal.z < -0.7f)
		{
			forwardDirection.z = 0.0f;
			forwardDirection.Normalize();
		}
		else
		{
			// duck under
			forwardDirection.x = 0.0f;
			forwardDirection.y = 0.0f;
			forwardDirection.z = -1.0f;
		}
		break;
	}
	case TER_WALL_RIGHT:
	{
		gstate->Con_Dprintf("Right side Blocked.\n");

		// right side is blocked
		// rotate the wall normal to the left so that we can move around this 
		// obstacle
		CVector dir(terrain.wall_normal.y, -terrain.wall_normal.x, forwardDirection.z);
		dir.Normalize();
		forwardDirection = dir;

		SIDEKICK_ModifyStrafeDirectionAfterCollision(self);
		break;
	}
	case TER_WALL_LEFT:
	{
		gstate->Con_Dprintf("Left side Blocked.\n");

		// left side is blocked
		// rotate the wall normal to the right so that we can move around this 
		// obstacle
		CVector dir(-terrain.wall_normal.y, terrain.wall_normal.x, forwardDirection.z);
		dir.Normalize();
		forwardDirection = dir;

		SIDEKICK_ModifyStrafeDirectionAfterCollision(self);
		break;
	}
	case TER_LOW_OBS:
	{
		// go up
		gstate->Con_Dprintf("Detected low obstruction.\n");

		if (terrain.wall_normal.z > 0.7f)
		{
			forwardDirection.z = 0.0f;
			forwardDirection.Normalize();
		}
		else
		{
			forwardDirection.x = 0.0f;
			forwardDirection.y = 0.0f;
			forwardDirection.z = 1.0f;
		}
		break;
	}

	default:
	{
		break;
	}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_HandleCollisionWhileAttacking
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_HandleCollisionWhileAttacking(userEntity_t* self, userEntity_t* pEnemy,
	float fSpeed, CVector& forwardDirection)
{
	_ASSERTE(self);
	_ASSERTE(pEnemy);

	SIDEKICK_HandleCollision(self, fSpeed, forwardDirection);
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SelectAnimationWhileAttacking
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SelectAnimationWhileAttacking(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);

	if (self->velocity.Length() > 0.0f)
	{
		frameData_t* pCurrentSequence = hook->cur_sequence;
		if (!strstr(pCurrentSequence->animation_name, "run"))
		{
			char szAnimation[16];
			AI_SelectRunningAnimation(self, szAnimation);
			AI_ForceSequence(self, szAnimation, FRAME_LOOP);
		}
	}
	else
	{
		float fWeaponRange = AI_GetCurrentWeaponRange(self);
		if (fWeaponRange >= SIDEKICK_MELEE_ATTACK_DISTANCE && AI_IsEndAnimation(self))
		{
			frameData_t* pCurrentSequence = hook->cur_sequence;
			weapon_t* pWeapon = (weapon_t*)self->curWeapon;
			//if ( !strstr( pCurrentSequence->animation_name, "amb" ) && _stricmp( pWeapon->name, "weapon_discus" ) )
			//{
			char szAnimation[16];
			//AI_SelectAnimationPerWeaponType( userEntity_t *self, char *pszAnimation )
//				sprintf(szAnimation,"atak");
			Com_sprintf(szAnimation, sizeof(szAnimation), "atak");
			AI_SelectAnimationPerWeaponType(self, szAnimation);

			AI_ForceSequence(self, szAnimation, FRAME_LOOP);
			//}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_StickCloseToPlayer
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_StickCloseToPlayer(userEntity_t* self)
{
	userEntity_t* pEnemy = self->enemy;
	_ASSERTE(AI_IsAlive(pEnemy));

	playerHook_t* hook = AI_GetPlayerHook(self);
	userEntity_t* pOwner = hook->owner;

	float fDistance = VectorDistance(self->s.origin, pOwner->s.origin);
	float fSpeed = AI_ComputeMovingSpeed(hook);

	// make this sidekick stay at certain angle from the player relative to where
	// the enemy and player is located and at a certain distance from the player
	CVector ownerVectorTowardEnemy;
	CVector ownerAngleTowardEnemy;
	ownerVectorTowardEnemy = pEnemy->s.origin - pOwner->s.origin;
	ownerVectorTowardEnemy.Normalize();
	VectorToAngles(ownerVectorTowardEnemy, ownerAngleTowardEnemy);
	ownerAngleTowardEnemy.pitch = 0.0f;   // pitch should be zero, a sidekick should be
	// at about the same plane as the player

	float fAngle = fAngleAwayFromOwner;
	int nWhichSide = AI_WhichSide(pOwner->s.origin, pEnemy->s.origin, self->s.origin);
	if (nWhichSide == ON_RIGHT)
	{
		fAngle = -fAngleAwayFromOwner;
	}
	ownerAngleTowardEnemy.y = AngleMod(ownerAngleTowardEnemy.y + fAngle);

	CVector forwardVectorFromOwner;
	ownerAngleTowardEnemy.AngleToForwardVector(forwardVectorFromOwner);

	CVector destPoint;
	VectorMA(pOwner->s.origin, forwardVectorFromOwner, fNormalDistanceFromOwner, destPoint);

	tr = gstate->TraceLine_q2(self->s.origin, destPoint, self, MASK_SOLID);
	if (tr.fraction < 1.0f)
	{
		float fDistanceToDestFromSelf = VectorDistance(self->s.origin, destPoint);
		CVector forwardVectorFromSelf = destPoint - self->s.origin;
		forwardVectorFromSelf.Normalize();

		float fDistanceToCollidedPoint = (fDistanceToDestFromSelf * tr.fraction) - 32.0f;
		VectorMA(self->s.origin, forwardVectorFromSelf, fDistanceToCollidedPoint, destPoint);

		float fDistanceToDestFromOwner = VectorDistance(pOwner->s.origin, destPoint);
		if (fDistanceToDestFromOwner < fCloseDistanceToOwner)
		{
			// do not move
			destPoint = self->s.origin;
		}
	}

	CVector groundPoint;
	CVector movingVector;
	if (AI_FindGroundPoint(self, destPoint, groundPoint))
	{
		movingVector = groundPoint - self->s.origin;
		movingVector.Normalize();
	}
	else
	{
		// do not move
	}

	float fDistanceToDestPoint = VectorDistance(self->s.origin, groundPoint);
	if (fDistanceToDestPoint < fOneHalfFrameDistance)
	{
		movingVector = pOwner->velocity;
		movingVector.Normalize();
	}

	if (movingVector.Length() > 0.0f)
	{
		SIDEKICK_HandleCollision(self, fSpeed, movingVector);

		if (SIDEKICK_IsGap(self, movingVector))
		{
			AI_StopEntity(self);
			return FALSE;
		}
		else
		{
			// scale speed based on current frame's move_scale
			float fScale = FRAMES_ComputeFrameScale(hook);
			AI_SetVelocity(self, movingVector, (fSpeed * fScale));
		}
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_MoveCloserTowardEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_MoveCloserTowardEnemy(userEntity_t* self, userEntity_t* pEnemy)
{
	playerHook_t* hook = AI_GetPlayerHook(self);

	_ASSERTE(pEnemy);

	float fSpeed = AI_ComputeMovingSpeed(hook);

	CVector vectorTowardEnemy;
	CVector angleTowardEnemy;
	vectorTowardEnemy = pEnemy->s.origin - self->s.origin;
	vectorTowardEnemy.Normalize();
	VectorToAngles(vectorTowardEnemy, angleTowardEnemy);

	float fYAngle = 45.0f;
	if (rnd() > 0.8f)
	{
		fYAngle += 15.0f;
	}

	switch (hook->strafe_dir)
	{
	case STRAFE_LEFT:
	{
		angleTowardEnemy.y += fYAngle;
		break;
	}
	case STRAFE_RIGHT:
	{
		angleTowardEnemy.y -= fYAngle;
		break;
	}
	case STRAFE_UPLEFT:
	{
		angleTowardEnemy.y += fYAngle;
		break;
	}
	case STRAFE_UPRIGHT:
	{
		angleTowardEnemy.y -= fYAngle;
		break;
	}
	case STRAFE_DOWNLEFT:
	{
		angleTowardEnemy.y += fYAngle;
		break;
	}
	case STRAFE_DOWNRIGHT:
	{
		angleTowardEnemy.y -= fYAngle;
		break;
	}
	default:
	{
		break;
	}
	}

	angleTowardEnemy.y = AngleMod(angleTowardEnemy.y);

	CVector forwardVector;
	angleTowardEnemy.AngleToForwardVector(forwardVector);
	forwardVector.Normalize();

	SIDEKICK_HandleCollisionWhileAttacking(self, pEnemy, fSpeed, forwardVector);

	if (SIDEKICK_IsGap(self, forwardVector))
	{
		AI_StopEntity(self);
		return TRUE;
	}

	// scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale(hook);
	AI_SetVelocity(self, forwardVector, (fSpeed * fScale));

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_MoveTowardEntity
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_MoveTowardEntity(userEntity_t* self, userEntity_t* pEntity)
{
	_ASSERTE(self);
	_ASSERTE(pEntity);

	playerHook_t* hook = AI_GetPlayerHook(self);

	float fXYDistance = VectorXYDistance(self->s.origin, pEntity->s.origin);
	float fZDistance = VectorZDistance(self->s.origin, pEntity->s.origin);
	if (AI_IsOkToMoveStraight(self, pEntity->s.origin, fXYDistance, fZDistance))
	{
		SIDEKICK_MoveTowardPointWhileAttacking(self, pEntity->s.origin, FALSE);
	}
	else
	{
		// move toward owner until can no longer see the enemy
		if (!AI_IsPathToEntityClose(self, pEntity))
		{
			if (!AI_FindPathToEntity(self, pEntity))
			{
				return;
			}
		}

		if (SIDEKICK_MoveWhileAttacking(self) == FALSE)
		{
			PATHLIST_KillPath(hook->pPathList);
		}
	}

	hook->nMoveCounter++;

	SIDEKICK_SelectAnimationWhileAttacking(self);
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_UpdateMeleeAttackMovement
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_UpdateMeleeAttackMovement(userEntity_t* self)
{
	userEntity_t* pEnemy = self->enemy;
	playerHook_t* hook = AI_GetPlayerHook(self);
	userEntity_t* pOwner = hook->owner;



	if (!AI_IsAlive(pEnemy))
	{
		return FALSE;
	}

	AI_SetStateRunning(hook);

	SIDEKICK_SelectAnimationWhileAttacking(self);

	// if the owner is hidden from the enemy then move closer to him
	float fDistanceToOwner = fNormalDistanceFromOwner;
	int bCanOwnerSeeEnemy = AI_IsLineOfSight(pOwner, pEnemy);
	if (!bCanOwnerSeeEnemy)
	{
		fDistanceToOwner = fCloseDistanceToOwner;
	}

	int bCanSeeOwner = AI_IsLineOfSight(self, pOwner);
	int bCanSeeEnemy = AI_IsLineOfSight(self, pEnemy);

	if (!bCanSeeOwner && AI_IsAlive(self))
	{
		SIDEKICK_MoveTowardEntity(self, pOwner);
		return TRUE;
	}

	// this sidekick can see the owner now

	if (!bCanOwnerSeeEnemy && bCanSeeEnemy)
	{
		SIDEKICK_MoveTowardEntity(self, pOwner);
		return TRUE;
	}

	if (!SIDEKICK_IsOwnerClose(self))
	{
		if (SIDEKICK_StickCloseToPlayer(self) == FALSE)
		{
			return FALSE;
		}
	}
	else
	{
		float fDistance = VectorDistance(self->s.origin, pEnemy->s.origin);
		if (fDistance < AI_GetCurrentWeaponRange(self))
		{
			AI_ZeroVelocity(self);
			// NSS[12/14/99]:Give them a little movement
			if (AI_IsEnemyTargetingMe(self, self->enemy) && (rnd() < 0.25))
			{
				AI_AddNewTaskAtFront(self, TASKTYPE_SIDESTEP);
			}

		}
		else
		{
			// SCG[10/20/99]: this was causing a problem
			if (SIDEKICK_MoveTowardPointWhileAttacking(self, pEnemy->s.origin) == FALSE)
			{
				return FALSE;
			}
		}
	}


	return TRUE;
}

static qboolean SIDEKICK_IsEntityBlockingShot(
	userEntity_t* self,
	userEntity_t* pEnemy,
	userEntity_t* blocker)
{
	CVector toEnemy;
	CVector toBlocker;
	CVector direction;
	CVector closest;
	float enemyDistance;
	float blockerAlong;
	float blockerLateral;

	if (!self || !pEnemy || !blocker || blocker == pEnemy || !AI_IsAlive(blocker))
		return FALSE;

	if (blocker == self)
		return FALSE;

	toEnemy = pEnemy->s.origin - self->s.origin;
	enemyDistance = toEnemy.Length();
	if (enemyDistance < 64.0f)
		return FALSE;

	direction = toEnemy;
	direction.Normalize();
	toBlocker = blocker->s.origin - self->s.origin;
	blockerAlong = DotProduct(toBlocker, direction);

	if (blockerAlong < 24.0f || blockerAlong > enemyDistance - 24.0f)
		return FALSE;

	closest = direction * blockerAlong;
	blockerLateral = (toBlocker - closest).Length();
	if (blockerLateral > 56.0f)
		return FALSE;

	return AI_IsVisible(self, blocker) && AI_IsVisible(self, pEnemy);
}

static qboolean SIDEKICK_IsSquadmateBlockingShot(
	userEntity_t* self,
	userEntity_t* pEnemy)
{
	playerHook_t* hook;
	userEntity_t* owner;
	userEntity_t* other;

	if (!self || !pEnemy)
		return FALSE;

	hook = AI_GetPlayerHook(self);
	if (!hook)
		return FALSE;

	owner = hook->owner;
	if (owner && SIDEKICK_IsEntityBlockingShot(self, pEnemy, owner))
		return TRUE;

	other = SIDEKICK_GetOtherSidekick(self);
	if (other && SIDEKICK_IsEntityBlockingShot(self, pEnemy, other))
		return TRUE;

	return FALSE;
}

static qboolean SIDEKICK_IsOwnerBlockingShot(
	userEntity_t* self,
	userEntity_t* pEnemy)
{
	playerHook_t* hook;

	if (!self || !pEnemy)
		return FALSE;

	hook = AI_GetPlayerHook(self);
	if (!hook || !hook->owner)
		return FALSE;

	return SIDEKICK_IsEntityBlockingShot(self, pEnemy, hook->owner);
}

static void SIDEKICK_SelectOwnerBypassDirection(userEntity_t* self, userEntity_t* pEnemy)
{
	playerHook_t* hook;
	CVector forward;
	CVector right;
	CVector up;
	userEntity_t* blocker;
	CVector toBlocker;
	float blockerRight;

	if (!self || !pEnemy)
		return;

	hook = AI_GetPlayerHook(self);
	if (!hook)
		return;

	blocker = NULL;
	if (hook->owner &&
		SIDEKICK_IsEntityBlockingShot(self, pEnemy, hook->owner))
	{
		blocker = hook->owner;
	}
	else
	{
		userEntity_t* other = SIDEKICK_GetOtherSidekick(self);
		if (other && SIDEKICK_IsEntityBlockingShot(self, pEnemy, other))
			blocker = other;
	}

	if (!blocker)
		return;

	self->s.angles.AngleToVectors(forward, right, up);
	toBlocker = blocker->s.origin - self->s.origin;
	blockerRight = DotProduct(toBlocker, right);

	if (fabs(blockerRight) < 8.0f)
		hook->strafe_dir = (rnd() < 0.5f) ? STRAFE_LEFT : STRAFE_RIGHT;
	else if (blockerRight > 0.0f)
		hook->strafe_dir = STRAFE_LEFT;
	else
		hook->strafe_dir = STRAFE_RIGHT;

	hook->strafe_time = gstate->time + 1.5f;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_ShouldStrafe
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define STRAFE_CHECK_DISTANCE   128.0f

int SIDEKICK_ShouldStrafe(userEntity_t* self, userEntity_t* pEnemy)
{
	CVector vectorTowardEnemy;
	CVector angleTowardEnemy;
	vectorTowardEnemy = pEnemy->s.origin - self->s.origin;
	vectorTowardEnemy.Normalize();
	VectorToAngles(vectorTowardEnemy, angleTowardEnemy);

	float fYAngle = 85.0f;
	float fDistance = VectorDistance(self->s.origin, pEnemy->s.origin);
	float fRange = AI_GetCurrentWeaponRange(self);
	if (fDistance < fRange * 0.35f)
	{
		fYAngle = 95.0f;
	}
	else
		if (fDistance > fRange * 0.75f)
		{
			fYAngle = 45.0f;
		}

	angleTowardEnemy.y = fYAngle;

	CVector forwardVector;
	angleTowardEnemy.AngleToForwardVector(forwardVector);
	forwardVector.Normalize();

	CVector mins(self->s.mins.x + 8.0f, self->s.mins.y + 8.0f, self->s.mins.z + 16.0f);
	CVector maxs(self->s.maxs.x - 8.0f, self->s.maxs.y - 8.0f, self->s.maxs.z);

	int bCanStrafe = TRUE;
	CVector endPoint;
	VectorMA(self->s.origin, forwardVector, STRAFE_CHECK_DISTANCE, endPoint);
	if (AI_IsGroundBelowBetween(self, self->s.origin, endPoint))
	{
		tr = gstate->TraceBox_q2(self->s.origin, mins, maxs, endPoint, self, MASK_SOLID);
		if (tr.fraction < 1.0f)
		{
			bCanStrafe = FALSE;
		}
	}
	else
	{
		bCanStrafe = FALSE;
	}

	if (bCanStrafe == TRUE)
	{
		float fOppositeAngle = 360.0f - fYAngle;
		CVector oppositeAngle = angleTowardEnemy;
		oppositeAngle.y = fOppositeAngle;

		CVector oppositeVector;
		oppositeAngle.AngleToForwardVector(oppositeVector);
		oppositeVector.Normalize();

		//	    endPoint;// SCG[1/23/00]: not used
		VectorMA(self->s.origin, oppositeVector, STRAFE_CHECK_DISTANCE, endPoint);
		if (AI_IsGroundBelowBetween(self, self->s.origin, endPoint))
		{
			tr = gstate->TraceBox_q2(self->s.origin, mins, maxs, endPoint, self, MASK_SOLID);
			if (tr.fraction >= 1.0f)
			{
				bCanStrafe = FALSE;
			}
		}
		else
		{
			bCanStrafe = FALSE;
		}
	}

	return bCanStrafe;

}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_AttackWhileStrafing
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_AttackWhileStrafing(userEntity_t* self, userEntity_t* pEnemy)
{
	_ASSERTE(self);
	_ASSERTE(pEnemy);

	if (self->groundEntity == NULL && self->waterlevel < 3)
	{
		return TRUE;
	}

	playerHook_t* hook = AI_GetPlayerHook(self);

	int bCanSeeEnemy = AI_IsCompletelyVisible(self, pEnemy, SIDEKICK_WIDTH_FACTOR);
	if (bCanSeeEnemy == NOT_VISIBLE)
	{
		// end this state
		SIDEKICK_SetAttackTime(hook, gstate->time);
		return TRUE;
	}

	float fSpeed = AI_ComputeMovingSpeed(hook);

	CVector vectorTowardEnemy;
	CVector angleTowardEnemy;
	vectorTowardEnemy = pEnemy->s.origin - self->s.origin;
	vectorTowardEnemy.Normalize();
	VectorToAngles(vectorTowardEnemy, angleTowardEnemy);

	float fYAngle = 85.0f;
	float fDistance = VectorDistance(self->s.origin, pEnemy->s.origin);
	float fRange = AI_GetCurrentWeaponRange(self);
	if (fDistance < fRange * 0.35f)
	{
		fYAngle = 95.0f;
	}
	else
		if (fDistance > fRange * 0.75f)
		{
			fYAngle = 45.0f;
		}

	if (rnd() > 0.8f)
	{
		fYAngle += 15.0f;
	}

	switch (hook->strafe_dir)
	{
	case STRAFE_LEFT:
	{
		angleTowardEnemy.y += fYAngle;
		break;
	}
	case STRAFE_RIGHT:
	{
		angleTowardEnemy.y -= fYAngle;
		break;
	}
	case STRAFE_UPLEFT:
	{
		angleTowardEnemy.y += fYAngle;
		break;
	}
	case STRAFE_UPRIGHT:
	{
		angleTowardEnemy.y -= fYAngle;
		break;
	}
	case STRAFE_DOWNLEFT:
	{
		angleTowardEnemy.y += fYAngle;
		break;
	}
	case STRAFE_DOWNRIGHT:
	{
		angleTowardEnemy.y -= fYAngle;
		break;
	}
	default:
	{
		break;
	}
	}

	angleTowardEnemy.y = AngleMod(angleTowardEnemy.y);

	CVector forwardVector;
	angleTowardEnemy.AngleToForwardVector(forwardVector);
	forwardVector.Normalize();

	SIDEKICK_HandleCollisionWhileAttacking(self, pEnemy, fSpeed, forwardVector);

	if (SIDEKICK_IsGap(self, forwardVector))
	{
		AI_StopEntity(self);
		return TRUE;
	}

	// scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale(hook);
	AI_SetVelocity(self, forwardVector, (fSpeed * fScale));

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_TakeCover
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_TakeCover(userEntity_t* self, userEntity_t* pEnemy)
{
	_ASSERTE(self);
	_ASSERTE(pEnemy);
	playerHook_t* hook = AI_GetPlayerHook(self);

	SIDEKICK_MoveWhileAttacking(self);

	if (!hook->pPathList->pPath)
	{
		//	set up a path consisting of one node
		MAPNODE_PTR pRetreatNode = NODE_FurthestFrom(hook->pNodeList, pEnemy->s.origin);
		if (!pRetreatNode)
		{
			// no longer in backaway
			SIDEKICK_SetAttackTime(hook, gstate->time);
			return TRUE;
		}

		PATHLIST_AddNodeToPath(hook->pPathList, pRetreatNode);

		SIDEKICK_MoveWhileAttacking(self);
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_StayStill
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_StayStill(userEntity_t* self, userEntity_t* pEnemy)
{
	_ASSERTE(self);
	_ASSERTE(pEnemy);
	playerHook_t* hook = AI_GetPlayerHook(self);

	AI_ZeroVelocity(self);

	int bCanSeeEnemy = AI_IsCompletelyVisible(self, pEnemy, SIDEKICK_WIDTH_FACTOR);
	if (bCanSeeEnemy != NOT_VISIBLE)
	{
		// end this state
		SIDEKICK_SetAttackTime(hook, gstate->time);
		return TRUE;
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_BackAway
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_BackAway(userEntity_t* self, userEntity_t* pEnemy)
{
	_ASSERTE(self);
	_ASSERTE(pEnemy);
	playerHook_t* hook = AI_GetPlayerHook(self);

	int bCanSeeEnemy = AI_IsCompletelyVisible(self, pEnemy, SIDEKICK_WIDTH_FACTOR);
	if (bCanSeeEnemy == NOT_VISIBLE)
	{
		// no longer in backaway
		SIDEKICK_SetAttackTime(hook, gstate->time);
		return TRUE;
	}

	SIDEKICK_MoveWhileAttacking(self);

	if (!hook->pPathList->pPath)
	{
		//	set up a path consisting of one node
		MAPNODE_PTR pRetreatNode = NODE_FurthestFrom(hook->pNodeList, pEnemy->s.origin);
		if (!pRetreatNode)
		{
			// no longer in backaway
			SIDEKICK_SetAttackTime(hook, gstate->time);
			return TRUE;
		}

		PATHLIST_AddNodeToPath(hook->pPathList, pRetreatNode);

		SIDEKICK_MoveWhileAttacking(self);
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_ShouldCharge
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_ShouldCharge(userEntity_t* self, userEntity_t* pEnemy)
{
	_ASSERTE(self);

	if (!AI_IsAlive(pEnemy))
	{
		return FALSE;
	}

	int bCanSeeEnemy = AI_IsCompletelyVisible(self, pEnemy, SIDEKICK_WIDTH_FACTOR);
	if (bCanSeeEnemy == NOT_VISIBLE)
	{
		return TRUE;
	}

	int bShouldCharge = TRUE;
	float fDistance = VectorDistance(self->s.origin, pEnemy->s.origin);
	float fRange = AI_GetCurrentWeaponRange(self);
	if (fRange < SIDEKICK_MELEE_ATTACK_DISTANCE)
	{
		bShouldCharge = FALSE;
	}
	else
	{
		if (fDistance < fRange * 0.5f)
		{
			bShouldCharge = FALSE;
		}
	}

	return bShouldCharge;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_Charge
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_Charge(userEntity_t* self, userEntity_t* pEnemy)
{
	_ASSERTE(self);
	_ASSERTE(pEnemy);

	playerHook_t* hook = AI_GetPlayerHook(self);

	if (!SIDEKICK_ShouldCharge(self, pEnemy))
	{
		// no longer in charge mode
		SIDEKICK_SetAttackTime(hook, gstate->time);
		return TRUE;
	}

	int bCanSeeEnemy = AI_IsCompletelyVisible(self, pEnemy, SIDEKICK_WIDTH_FACTOR);
	if (bCanSeeEnemy == COMPLETELY_VISIBLE)
	{
		SIDEKICK_AttackWhileStrafing(self, pEnemy);
		return TRUE;
	}

	float fXYDistance = VectorXYDistance(self->s.origin, pEnemy->s.origin);
	float fZDistance = VectorZDistance(self->s.origin, pEnemy->s.origin);
	if (AI_IsOkToMoveStraight(self, pEnemy->s.origin, fXYDistance, fZDistance))
	{
		SIDEKICK_MoveTowardPointWhileAttacking(self, pEnemy->s.origin, FALSE);
	}
	else
	{
		// move toward owner until can no longer see the enemy
		if (!AI_IsPathToEntityClose(self, pEnemy))
		{
			if (!AI_FindPathToEntity(self, pEnemy))
			{
				SIDEKICK_SetAttackTime(hook, gstate->time);
				return FALSE;
			}
		}

		if (SIDEKICK_MoveWhileAttacking(self) == FALSE)
		{
			PATHLIST_KillPath(hook->pPathList);
		}
	}

	hook->nMoveCounter++;

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_Jump
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_Jump(userEntity_t* self, userEntity_t* pEnemy)
{
	_ASSERTE(self);
	_ASSERTE(pEnemy);

	if (self->groundEntity != NULL)
	{
		playerHook_t* hook = AI_GetPlayerHook(self);

		char szAnimation[16];
		AI_SelectJumpingAnimation(self, szAnimation);
		AI_ForceSequence(self, szAnimation);

		self->velocity.z += hook->upward_vel;

		self->groundEntity = NULL;

		SIDEKICK_SetAttackTime(hook, gstate->time);
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_UpdateRangeAttackMovement
// Description:
// Input:
// Output:
// Note:
//              try to stay close to the owner
//
// ----------------------------------------------------------------------------
#ifdef USE_STRAFE

int SIDEKICK_UpdateRangeAttackMovement(userEntity_t* self)
{
	userEntity_t* pEnemy = self->enemy;
	if (!pEnemy)
	{
		return FALSE;
	}

	AI_ZeroVelocity(self);

	playerHook_t* hook = AI_GetPlayerHook(self);

	float fSpeed = AI_ComputeMovingSpeed(hook);

	CVector vectorTowardEnemy;
	CVector angleTowardEnemy;
	vectorTowardEnemy = pEnemy->s.origin - self->s.origin;
	vectorTowardEnemy.Normalize();
	VectorToAngles(vectorTowardEnemy, angleTowardEnemy);

	float fYAngle = 85.0f;
	float fDistance = VectorDistance(self->s.origin, pEnemy->s.origin);
	float fRange = AI_GetCurrentWeaponRange(self);
	if (fDistance < fRange * 0.25f)
	{
		fYAngle = 95.0f;
	}

	if (rnd() > 0.8f)
	{
		fYAngle += 15.0f;
	}

	switch (hook->strafe_dir)
	{
	case STRAFE_LEFT:
	{
		angleTowardEnemy.y += fYAngle;
		break;
	}
	case STRAFE_RIGHT:
	{
		angleTowardEnemy.y -= fYAngle;
		break;
	}
	case STRAFE_UPLEFT:
	{
		angleTowardEnemy.y += fYAngle;
		break;
	}
	case STRAFE_UPRIGHT:
	{
		angleTowardEnemy.y -= fYAngle;
		break;
	}
	case STRAFE_DOWNLEFT:
	{
		angleTowardEnemy.y += fYAngle;
		break;
	}
	case STRAFE_DOWNRIGHT:
	{
		angleTowardEnemy.y -= fYAngle;
		break;
	}
	default:
	{
		break;
	}
	}

	angleTowardEnemy.y = AngleMod(angleTowardEnemy.y);

	CVector forwardVector;
	angleTowardEnemy.AngleToForwardVector(forwardVector);
	forwardVector.Normalize();

	SIDEKICK_HandleCollisionWhileAttacking(self, pEnemy, fSpeed, forwardVector);

	if (SIDEKICK_IsGap(self, forwardVector))
	{
		AI_StopEntity(self);
		return FALSE;
	}

	// scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale(hook);
	AI_SetVelocity(self, forwardVector, (fSpeed * fScale));

	return TRUE;
}

#else USE_STRAFE

int SIDEKICK_UpdateRangeAttackMovement(userEntity_t* self)
{
	userEntity_t* pEnemy = self->enemy;
	if (!AI_IsAlive(pEnemy))
	{
		return FALSE;
	}

	// AI_ZeroVelocity( self );

	playerHook_t* hook = AI_GetPlayerHook(self);
	userEntity_t* pOwner = hook->owner;

	// if the owner is hidden from the enemy then move closer to him
// SCG[1/23/00]: not used
//    float fDistanceToOwner = fNormalDistanceFromOwner;
//    int bCanOwnerSeeEnemy = AI_IsLineOfSight( pOwner, pEnemy );
//    if ( !bCanOwnerSeeEnemy )
//    {
//        fDistanceToOwner = fCloseDistanceToOwner;
//    }

	SIDEKICK_MoveTowardEntity(self, pOwner);

	SIDEKICK_SelectAnimationWhileAttacking(self);

	return TRUE;
}
#endif USE_STRAFE



// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_UpdateAttackMovement
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_UpdateAttackMovement(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);
	//	userEntity_t *client_entity = &gstate->g_edicts[1];	// SCG[1/23/00]: not used

	userEntity_t* pOwner = hook->owner;

	weapon_t* pWeapon = (weapon_t*)self->curWeapon;
	if (!pWeapon || !SIDEKICK_CanUseWeapon(pWeapon))
	{
		SIDEKICK_ChooseBestWeapon(self);
		pWeapon = (weapon_t*)self->curWeapon;
		if (!pWeapon || !SIDEKICK_CanUseWeapon(pWeapon))
		{
			// try to evade from fighting
			GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
			GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
			GOAL_ClearTasks(pCurrentGoal);
			AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_EVADE, self->enemy);
			return FALSE;
		}
	}

	float fRange = AI_GetCurrentWeaponRange(self);
	if (fRange <= 156.0f)
	{
		if (SIDEKICK_UpdateMeleeAttackMovement(self) == FALSE)
		{
			return FALSE;
		}
	}
	else
	{
		// ranged attack weapon
		if (SIDEKICK_UpdateRangeAttackMovement(self) == FALSE)
		{
			return FALSE;
		}
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
// NSS[1/20/00]:
// Name:		SIDEKICK_IsClearShot
// Description: Will determine if the Sidekick has a clear shot or not.
// Input: userEntity_t *self, userEntity_t *pEntity
// Output: TURE or FALSE
// Note:
// ----------------------------------------------------------------------------
int SIDEKICK_IsClearShot(userEntity_t* self, userEntity_t* pEntity, CVector& OrgStart, CVector& OrgEnd)
{
	CVector Delta_Origin, Destination;
	int Hit = 0;
	int single_trace = 0;
	float x = 5;
	float z = 5;


	// NSS[1/20/00]:Had to make this a little more modular for other purposes
	Delta_Origin.Set(OrgStart.x, OrgStart.y, OrgStart.z);
	Destination.Set(OrgEnd.x, OrgEnd.y, OrgEnd.z);

	weapon_t* pWeapon = (weapon_t*)self->curWeapon;

	int nCurrentEpisode = GetCurrentEpisode();

	if (pWeapon)
	{

		float fWeaponRange = AI_GetCurrentWeaponRange(self);
		float fDistance = VectorXYDistance(Delta_Origin, Destination);

		// NSS[1/20/00]:Make sure we can hit it first.
		if (fWeaponRange < fDistance)
		{
			return FALSE;
		}

		switch (nCurrentEpisode)
		{
		case 1:
		{
			//Reference
			//gstate->SetSize (ionBlast, -8, -8, -8, 8, 8, 8);
			if (_stricmp(pWeapon->name, "weapon_disruptor") == 0)
			{
				single_trace = 1;
			}
			else if (_stricmp(pWeapon->name, "weapon_ionblaster") == 0)
			{
				x = 16;
				z = 16;
			}
			break;
		}
		case 2:
		{
			//Reference
			//gstate->SetSize(discus,-5,-2,-5,5,2,5);
			if (_stricmp(pWeapon->name, "weapon_discus") == 0)
			{
				x = 8;
				z = 8;
			}
			else
				if (_stricmp(pWeapon->name, "weapon_venomous") == 0)
				{
					x = 16;
					z = 16;
				}
				else
					if (_stricmp(pWeapon->name, "weapon_trident") == 0)
					{
						x = 16;
						z = 16;
					}
			break;
		}
		case 3:
		{
			//Reference
			//gstate->SetSize(bolt,-4,-4,-4,4,4,4);
			if (_stricmp(pWeapon->name, "weapon_silverclaw") == 0)
			{
				single_trace = 1;
			}
			else if (_stricmp(pWeapon->name, "weapon_bolter") == 0)
			{
				x = 16;
				z = 16;
			}

			break;
		}
		case 4:
		{
			if (_stricmp(pWeapon->name, "weapon_glock") == 0)
			{
				single_trace = 1;
			}
			break;
		}
		}


		if (!single_trace)
		{
			Delta_Origin.x += x;
			Delta_Origin.z += z;
			tr = gstate->TraceLine_q2(Delta_Origin, Destination, self, MASK_MONSTERSOLID);
			if (tr.fraction < 1.0f && tr.ent != pEntity)
			{
				Hit++;
			}
			Delta_Origin.x -= x;
			Delta_Origin.z += z;
			tr = gstate->TraceLine_q2(Delta_Origin, Destination, self, MASK_MONSTERSOLID);
			if (tr.fraction < 1.0f && tr.ent != pEntity)
			{
				Hit++;
			}
			Delta_Origin.x += x;
			Delta_Origin.z -= z;
			tr = gstate->TraceLine_q2(Delta_Origin, Destination, self, MASK_MONSTERSOLID);
			if (tr.fraction < 1.0f && tr.ent != pEntity)
			{
				Hit++;
			}
			Delta_Origin.x -= x;
			Delta_Origin.z -= z;
			tr = gstate->TraceLine_q2(Delta_Origin, Destination, self, MASK_MONSTERSOLID);
			if (tr.fraction < 1.0f && tr.ent != pEntity)
			{
				Hit++;
			}
		}
		else
		{
			tr = gstate->TraceLine_q2(Delta_Origin, Destination, self, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_CLEAR | CONTENTS_DEADMONSTER);
			if (tr.fraction < 1.0f && tr.ent != pEntity)
			{
				Hit++;
			}
		}

		if (!Hit)
		{
			return TRUE;
		}
	}
	return FALSE;
}





// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_IsOkToUseRangedWeapon
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsOkToUseRangeWeapon(userEntity_t* self)
{
	_ASSERTE(self);
	//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	userEntity_t* pEnemy = self->enemy;
	_ASSERTE(pEnemy);

	int bOkToUseRangeWeapon = FALSE;

	//	float fDistance = VectorDistance( self->s.origin, pEnemy->s.origin );// SCG[1/23/00]: not used

	userInventory_t* pCurrentWeapon = self->curWeapon;
	_ASSERTE(pCurrentWeapon);

	int nCurrentEpisode = GetCurrentEpisode();
	switch (nCurrentEpisode)
	{
	case 1:
	{
		if (_stricmp(pCurrentWeapon->name, "weapon_ionblaster") == 0)
		{
			bOkToUseRangeWeapon = TRUE;
		}
		else if (_stricmp(pCurrentWeapon->name, "weapon_shotcycler") == 0)
		{
			bOkToUseRangeWeapon = TRUE;
		}
		break;
	}
	case 2:
	{
		bOkToUseRangeWeapon = TRUE;
		break;
	}
	case 3:
	{
		if ((_stricmp(pCurrentWeapon->name, "weapon_bolter") == 0 ||
			_stricmp(pCurrentWeapon->name, "weapon_ballista") == 0))
		{
			bOkToUseRangeWeapon = TRUE;
		}
		break;
	}
	case 4:
	{
		bOkToUseRangeWeapon = TRUE;
		break;
	}
	default:
	{
		break;
	}
	}

	return bOkToUseRangeWeapon;

}
// ----------------------------------------------------------------------------
// NSS[12/15/99]:
// Name:		SIDEKICK_SelectBestAttackAnimation
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SelectBestAttackAnimation(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	weapon_t* pWeapon = (weapon_t*)self->curWeapon;
	AI_Dprintf("SideKickAFrame:%d\n", self->s.frame);
	if (hook)
	{
		if (_stricmp(pWeapon->name, "weapon_discus") == 0 && self->velocity.Length() < 10.0f)
		{
			if ((!hook->cur_sequence || !strstr(hook->cur_sequence->animation_name, "atak") || AI_IsEndAnimation(self)) && pWeapon->ammo->count > 0 || AI_IsJustFired(hook))
			{
				frameData_t* pSequence = FRAMES_GetSequence(self, "atak");
				AI_ForceSequence(self, pSequence);
			}
		}
		else
		{
			SIDEKICK_SelectAnimationWhileAttacking(self);
		}
	}
}


// ----------------------------------------------------------------------------
// NSS[12/16/99]:
// Name:		SIDEKICK_RipgunThink
// Description: A solution to the chaingun problem and sidekicks...
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_RipgunThink(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	weapon_t* pWeapon = (weapon_t*)self->curWeapon;
	if (pWeapon)
	{


		if (_stricmp(pWeapon->name, "weapon_ripgun") == 0 || _stricmp(pWeapon->name, "weapon_shotcycler") == 0)
		{
			if (hook)
			{
				self->nextthink = gstate->time + 0.10f;
				if (hook->attack_finished < gstate->time)
				{
					self->think = SIDEKICK_Think;
				}
				else
				{
					ai_fire_curWeapon(self);
				}
				return;
			}
		}
	}
	self->think = SIDEKICK_Think;
	self->nextthink = gstate->time + 0.10f;
}




// ----------------------------------------------------------------------------
// NSS[12/15/99]:
// Name:		SIDEKICK_UseCurrentWeapon
// Description: This function actually uses the current weapon the sidekick has chosen
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_UseCurrentWeapon(userEntity_t* self)
{
	_ASSERTE(self);
	_ASSERTE(self->enemy);
	playerHook_t* hook = AI_GetPlayerHook(self);

	userEntity_t* pEnemy = self->enemy;

	if (self->enemy == NULL || hook == NULL)
	{
		return;
	}

	if (!SIDEKICK_IsFacingEnemy(self, pEnemy, -1.0f, 90.0f, 90.0f))
	{
		AI_FaceTowardPoint(self, pEnemy->s.origin);
		//	return;
	}


	weapon_t* pWeapon = (weapon_t*)self->curWeapon;
	if (pWeapon &&
		AI_GetCurrentWeaponRange(self) > SIDEKICK_MELEE_ATTACK_DISTANCE &&
		SIDEKICK_IsSquadmateBlockingShot(self, pEnemy))
	{
		return;
	}

	if (pWeapon)
	{
		if (SIDEKICK_CanUseWeapon(pWeapon))
		{
			float fWeaponRange = AI_GetCurrentWeaponRange(self);
			float fDistance = VectorDistance(self->s.origin, pEnemy->s.origin);

			// NSS[1/17/00]:Call the frame functions if required.
			if (hook->fxFrameFunc)
			{
				hook->fxFrameFunc(self);
			}

			if (fWeaponRange <= 124.0f)
			{
				if (fDistance <= fWeaponRange)
				{
					if (!hook->cur_sequence ||
						!strstr(hook->cur_sequence->animation_name, "atak") ||
						AI_IsEndAnimation(self))
					{
						frameData_t* pSequence = FRAMES_GetSequence(self, "atak");
						AI_ForceSequence(self, pSequence);
					}
					else
					{


						if (SIDEKICK_IsClearShot(self, pEnemy, self->s.origin, pEnemy->s.origin) &&
							(AI_IsReadyToAttack1(self) || AI_IsReadyToAttack2(self)))
						{
							CVector vectorTowardEnemy = pEnemy->s.origin - self->s.origin;

							vectorTowardEnemy.Normalize();

							CVector angleTowardEnemy;
							VectorToAngles(vectorTowardEnemy, angleTowardEnemy);

							self->client->v_angle = self->s.angles;
							self->client->v_angle.pitch = angleTowardEnemy.pitch;
							self->client->v_angle.roll = angleTowardEnemy.roll;

							hook->weapon_fired = self->curWeapon;
							// NSS[12/9/99]:So far not many good sounds for Mikiko's battle cries... 
							if (!Check_Que(self, STARTING_ATTACK, 10.0f))
							{
								SideKick_TalkAmbient(self, STARTING_ATTACK, 1);
								SIDEKICK_SendMessage(self, sCom_Speak, STARTING_ATTACK, gstate->time + 10.0f, 0.0f, self, COM_Read);
							}

							// NSS[12/20/99]:If we made it this far we should be pretty still.
							AI_ZeroVelocity(self);

							ai_fire_curWeapon(self);

							AI_SetJustFired(hook);

						}
						else if (AI_IsEndAnimation(self))
						{
							AI_RemoveCurrentTask(self, FALSE);
						}
					}
				}
			}
			else
			{
				// using ranged weapon
				if (gstate->time >= hook->attack_finished)
				{
					if (fDistance <= fWeaponRange)
					{
						if (SIDEKICK_IsClearShot(self, pEnemy, self->s.origin, pEnemy->s.origin))
						{
							if ((AI_IsReadyToAttack1(self) || AI_IsReadyToAttack2(self)) && pWeapon->ammo->count > 0)
							{
								CVector vectorTowardEnemy = pEnemy->s.origin - self->s.origin;
								vectorTowardEnemy.Normalize();

								CVector angleTowardEnemy;
								VectorToAngles(vectorTowardEnemy, angleTowardEnemy);

								self->client->v_angle = angleTowardEnemy;
								self->client->v_angle.pitch = angleTowardEnemy.pitch;
								self->client->v_angle.roll = angleTowardEnemy.roll;

								hook->weapon_fired = self->curWeapon;

								if (!Check_Que(self, STARTING_ATTACK, 10.0f))
								{
									SideKick_TalkAmbient(self, STARTING_ATTACK, 1);
									SIDEKICK_SendMessage(self, sCom_Speak, STARTING_ATTACK, gstate->time + 10.0f, 0.0f, self, COM_Read);
								}
								ai_fire_curWeapon(self);

								// NSS[12/20/99]:If we made it this far we should be pretty still.
								AI_ZeroVelocity(self);

								if (_stricmp(pWeapon->name, "weapon_ripgun") == 0)
								{
									self->think = SIDEKICK_RipgunThink;
									hook->attack_finished = gstate->time + 0.50;
									self->nextthink = gstate->time + 0.10f;
								}
								else if (_stricmp(pWeapon->name, "weapon_shotcycler") == 0)
								{
									self->think = SIDEKICK_RipgunThink;
									hook->attack_finished = gstate->time + 0.15;
									self->nextthink = gstate->time + 0.10f;
								}
								else
								{
									hook->attack_finished = gstate->time + SIDEKICK_ComputeAttackAnimationTime(self);
									AI_RemoveCurrentTask(self, FALSE);
								}

								AI_SetJustFired(hook);
							}

							if (pWeapon->ammo->count > 0)
							{
								SIDEKICK_SelectBestAttackAnimation(self);
							}
						}
					}
				}
			}
		}
	}
}



// ----------------------------------------------------------------------------
// NSS[1/17/00]:
// Name:		SIDEKICK_FindGoodShootingSpot
// Description: Will attempt to find a good shooting spot..if so it will move there.
// Input:userEntity_t *self, userEntity_t *enemy, CVector *Position
// Output: TRUE --> Spot found, FALSE---> Nope... just can't find one.
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_FindGoodShootingSpot(userEntity_t* self, userEntity_t* enemy, CVector* Position)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	weapon_t* pWeapon;
	CVector origin;
	CVector enemyOrigin = enemy->s.origin;
	CVector baseDir;
	CVector baseAngles;
	CVector ownerDir = vec3_origin;
	float range = 200.0f;
	float offsets[12] = { -170.0f, -135.0f, -100.0f, -65.0f, -30.0f, 15.0f,
		55.0f, 90.0f, 125.0f, 160.0f, 200.0f, 240.0f };
	int i;
	int best = -1;
	float bestScore = 99999999.0f;
	CVector bestPoint;
	userEntity_t* owner;

	if (!hook || !enemy)
		return FALSE;
	pWeapon = (weapon_t*)self->curWeapon;
	if (pWeapon)
	{
		float weaponRange = AI_GetCurrentWeaponRange(self);
		if (weaponRange < range)
			range = weaponRange * 0.85f;
	}
	owner = hook->owner;
	if (range < 200.0f)
		origin = enemyOrigin;
	else if (owner && AI_IsAlive(owner))
		origin = owner->s.origin;
	else
		origin = self->s.origin;

	baseDir = enemyOrigin - origin;
	baseDir.z = 0.0f;
	if (baseDir.Length() < 1.0f)
		baseDir = self->s.origin - enemyOrigin;
	baseDir.z = 0.0f;
	if (baseDir.Length() < 1.0f)
		baseDir.Set(1.0f, 0.0f, 0.0f);
	baseDir.Normalize();
	VectorToAngles(baseDir, baseAngles);

	if (owner && AI_IsAlive(owner))
	{
		ownerDir = owner->s.origin - enemyOrigin;
		ownerDir.z = 0.0f;
		ownerDir.Normalize();
	}

	for (i = 0; i < 12; i++)
	{
		CVector angles = baseAngles;
		CVector dir;
		CVector candidate;
		CVector fromEnemy;
		float score;
		float lateral;
		float behind;
		angles.yaw = AngleMod(baseAngles.yaw + offsets[i]);
		angles.pitch = 0.0f;
		angles.roll = 0.0f;
		angles.AngleToForwardVector(dir);
		dir.z = 0.0f;
		dir.Normalize();
		VectorMA(origin, dir, range, candidate);
		candidate.z += 16.0f;

		if (AI_IsPointObstructedByEntity(self, candidate))
			continue;
		if (!SIDEKICK_IsClearShot(self, enemy, candidate, enemyOrigin))
			continue;
		if (!AI_IsOkToMoveStraight(self, candidate,
			VectorXYDistance(self->s.origin, candidate), VectorZDistance(self->s.origin, candidate)))
		{
			if (!AI_FindPathToPoint(self, candidate))
				continue;
		}

		score = VectorXYDistance(self->s.origin, candidate);
		if (owner && AI_IsAlive(owner))
			score += VectorXYDistance(owner->s.origin, candidate) * 0.18f;
		fromEnemy = candidate - enemyOrigin;
		fromEnemy.z = 0.0f;
		fromEnemy.Normalize();
		lateral = fabs(ownerDir.x * fromEnemy.y - ownerDir.y * fromEnemy.x);
		behind = -(ownerDir.x * fromEnemy.x + ownerDir.y * fromEnemy.y);
		score -= lateral * 65.0f;
		if (behind > 0.15f)
			score -= 45.0f;
		if (fabs(candidate.z - enemyOrigin.z) > MAX_JUMP_HEIGHT)
			score += 120.0f;

		if (score < bestScore)
		{
			bestScore = score;
			best = i;
			bestPoint = candidate;
		}
	}

	if (best < 0)
		return FALSE;
	if (!AI_IsOkToMoveStraight(self, bestPoint,
		VectorXYDistance(self->s.origin, bestPoint), VectorZDistance(self->s.origin, bestPoint)))
	{
		if (!AI_FindPathToPoint(self, bestPoint))
			return FALSE;
	}
	*Position = bestPoint;
	return TRUE;
}


// ----------------------------------------------------------------------------
// NSS[1/17/00]:
// Name:		SIDEKICK_IsOwnerGoingThisWay
// Description:Determines if the owner is going in the general direction of the 
// monster we are targeting
// Input:userEntity_t *self
// Output:TRUE FALSE
// Note:
// ----------------------------------------------------------------------------
int SIDEKICK_IsOwnerGoingThisWay(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	userEntity_t* owner = hook->owner;
	// NSS[1/17/00]:Store this for later.
	float FOV = hook->fov;

	if (owner)
	{
		hook->fov = 90.0f;
		if (AI_IsInFOV(self, self->enemy) && self->velocity.Length() > 20.0f)
		{
			hook->fov = FOV;
			return TRUE;
		}

	}
	return FALSE;
}

// ----------------------------------------------------------------------------
// NSS[12/13/99]:
// Name:		SIDEKICK_Attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_Attack(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);
	CVector Dir, Angles;
	weapon_t* pWeapon = (weapon_t*)self->curWeapon;


	if (!self->enemy || !AI_IsAlive(self->enemy))
	{
		self->enemy = NULL;
		// killed the enemy, no longer in attack mode
		SIDEKICK_ClearAttackMode(self);
		// NSS[12/13/99]: Now this is a task... finally.
		AI_RemoveCurrentTask(self, FALSE);
		return;
	}

	Dir = self->enemy->s.origin - self->s.origin;
	Dir.Normalize();

	VectorToAngles(Dir, Angles);

	self->s.angles.yaw = Angles.yaw;

	// NSS[12/13/99]:Moved this out of the think process
	SIDEKICK_ChooseBestWeapon(self);

	if (SIDEKICK_IsOkToFight(self))
	{
		if (hook)
		{
			userEntity_t* owner = hook->owner;

			if (!SIDEKICK_IsClearShot(self, self->enemy, self->s.origin, self->enemy->s.origin))
			{
				if (SIDEKICK_FindGoodShootingSpot(self, self->enemy, &Dir))
				{
					AI_SetStateRunning(hook);
					float fDistance = VectorXYDistance(self->s.origin, Dir);
					float fZDistance = VectorZDistance(self->s.origin, Dir);
					if (fDistance < 48.0f && AI_IsOkToMoveStraight(self, Dir, fDistance, fZDistance))
					{
						if (!AI_MoveTowardPoint(self, self->enemy->s.origin, FALSE))
						{
							AI_AddNewTaskAtFront(self, TASKTYPE_MOVETOLOCATION, Dir);
						}
					}
					else
					{
						AI_AddNewTaskAtFront(self, TASKTYPE_MOVETOLOCATION, Dir);
					}
					return;
				}
				else
				{
					// Failed attack position: look around and allow the attack task
					// to retry using the smarter shooting-position search.
					SIDEKICK_SmartRememberFailedPoint(self, self->s.origin);
					SIDEKICK_SmartQueueLookAround(self);
				}
				return;
			}

			// NSS[1/20/00]:Always running when there is fighting going on.
			AI_SetStateRunning(hook);


			//if ( SIDEKICK_UpdateAttackMovement( self ) == FALSE)
			//{
			//	return;
			//}

			// NSS[12/20/99]:Actually do something with the weapon now.
			SIDEKICK_UseCurrentWeapon(self);
		}
		else
		{
			AI_Dprintf("Problemo's no hook and sidekick is in attack mode!!!!\n");
		}
	}
	else
	{
		self->enemy = NULL;
		AI_RemoveCurrentTask(self, FALSE);
		//AI_Dprintf("Problemo's no enemy and sidekick is in attack mode!!!!\n");
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_BeginAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_BeginAttack(userEntity_t* self)
{
	_ASSERTE(self);
	_ASSERTE(self->enemy);
	playerHook_t* hook = AI_GetPlayerHook(self);

	AI_SetStateAttacking(hook);

	// clear all tasks when beginning to attack
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
	GOAL_ClearTasks(pCurrentGoal);

#ifdef USE_STRAFE
	hook->strafe_dir = rand() % (STRAFE_DOWNRIGHT + 1);
#endif USE_STRAFE

	// NSS[12/5/99]:Code to make sidekicks scream that battle cry they scream so well.
	if (!Check_Que(self, STARTING_ATTACK, 15.0f) && rnd() < 0.35f)
	{
		if (SideKick_TalkAmbient(self, STARTING_ATTACK, 0))
			SIDEKICK_SendMessage(self, sCom_Speak, STARTING_ATTACK, gstate->time + 15.0f, 0.0f, self, COM_Read);
	}

	SIDEKICK_ChooseBestWeapon(self);
	SIDEKICK_SetAttackSequence(self);
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_Start
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_Start(userEntity_t* self, int nType)
{
	SIDEKICK_SmartResetState(self);
	_ASSERTE(self);

	// cek[1-19-00] prevent sidekick spawning in coop mode
	if (coop->value || deathmatch->value)
	{
		gstate->RemoveEntity(self);
		return;
	}

	playerHook_t* hook = AI_InitMonster(self, nType);
	self->s.flags &= ~SFL_TARGETABLE;
	_ASSERTE(hook);

	// SCG[10/25/99]: called specifically since only sidekicks and real clients 
	// SCG[10/25/99]: should be referencing the client structure..  This 
	// SCG[10/25/99]: takes place of the code Nelno wrote that alloc'd a client
	// SCG[10/25/99]: for every entity (monsters, worldspawn, etc...)
	self->client = (gclient_t*)gstate->X_Malloc(sizeof(gclient_t), MEM_TAG_TEMP);
	memset(self->client, 0, sizeof(gclient_t));

	gstate->LinkEntity(self);

	AI_DetermineMovingEnvironment(self);

	SIDEKICK_Init(self, nType);
	if (nType != TYPE_CLIENT && sv_sidekick_invincible && sv_sidekick_invincible->value > 0.0f)
		self->takedamage = DAMAGE_NO;
	SIDEKICK_InitWeapons(self);

	char szAnimation[16];
	AI_SelectAmbientAnimation(self, szAnimation);
	frameData_t* pSequence = FRAMES_GetSequence(self, szAnimation);
	AI_StartSequence(self, pSequence, FRAME_LOOP);

	gstate->LinkEntity(self);

	int nNumSidekicks = AIINFO_GetNumSidekicks();
	if (nNumSidekicks == 1)
	{
		SIDEKICK_ReadAmbientInfo(self, "SidekickAmbient.csv");
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_Spawn
// Description:
// Input:
// Output:
// Note:
//          do not spawn more than one kind of sidekick at a time
//
// ----------------------------------------------------------------------------
userEntity_t* SIDEKICK_Spawn(userEntity_t* self, int nType)
{
	// cek[1-19-00] prevent sidekick spawning in coop mode
	if (coop->value)
		return NULL;

	if ((nType == TYPE_SUPERFLY || nType == TYPE_MIKIKO || nType == TYPE_MIKIKOFLY) &&
		sv_no_sidekicks && sv_no_sidekicks->value > 0.0f)
		return NULL;

	if (nType == TYPE_SUPERFLY)
	{
		userEntity_t* pSuperFly = AIINFO_GetSuperfly();
		if (pSuperFly)
		{
			com->Warning("Superfly already exists!");
			return NULL;
		}
	}
	else
		if (nType == TYPE_MIKIKOFLY)
		{
			userEntity_t* pSuperFly = AIINFO_GetMikikofly();
			if (pSuperFly)
			{
				com->Warning("Mikikofly already exists!");
				return NULL;
			}
		}
		else
			if (nType == TYPE_MIKIKO)
			{
				userEntity_t* pMikiko = AIINFO_GetMikiko();
				if (pMikiko)
				{
					com->Warning("Mikiko already exists!");
					return NULL;
				}
			}
	CVector spawnPoint;
	if (!SPAWN_FindNearSpawnPoint(self, spawnPoint))
	{
		com->Warning("Did not find an appropriate spawn point.\n");
		return NULL;
	}

	userEntity_t* pSidekick = gstate->SpawnEntity();
	pSidekick->s.origin = spawnPoint;
	SIDEKICK_Start(pSidekick, nType);

	return pSidekick;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_Spawn
// Description:
// Input:
// Output:
// Note:
//          do not spawn more than one kind of sidekick at a time
//
// ----------------------------------------------------------------------------
userEntity_t* SIDEKICK_Spawn(int nType, const CVector& origin, const CVector& angle)
{
	// cek[1-19-00] prevent sidekick spawning in coop mode
	if (coop->value)
		return NULL;

	if ((nType == TYPE_SUPERFLY || nType == TYPE_MIKIKO || nType == TYPE_MIKIKOFLY) &&
		sv_no_sidekicks && sv_no_sidekicks->value > 0.0f)
		return NULL;

	if (nType == TYPE_SUPERFLY)
	{
		userEntity_t* pSuperFly = AIINFO_GetSuperfly();
		if (pSuperFly)
		{
			com->Warning("Superfly already exists!");
			return NULL;
		}
	}
	else
		if (nType == TYPE_MIKIKOFLY)
		{
			userEntity_t* pSuperFly = AIINFO_GetMikikofly();
			if (pSuperFly)
			{
				com->Warning("Mikikofly already exists!");
				return NULL;
			}
		}
		else
			if (nType == TYPE_MIKIKO)
			{
				userEntity_t* pMikiko = AIINFO_GetMikiko();
				if (pMikiko)
				{
					com->Warning("Mikiko already exists!");
					return NULL;
				}
			}

	userEntity_t* pSidekick = gstate->SpawnEntity();
	pSidekick->s.origin = origin;
	pSidekick->s.angles = angle;
	pSidekick->ideal_ang = angle;

	SIDEKICK_Start(pSidekick, nType);

	return pSidekick;
}

// ----------------------------------------------------------------------------
//							GLOBAL functions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_Alert
// Description:
// Input:
// Output:
// Note:
//	called by damage function when a client attacks a monster or bot
//
//	FIXME:	make this a general function that alerts all entities on the
//			same team.
//
// ----------------------------------------------------------------------------
void SIDEKICK_Alert(userEntity_t* owner, userEntity_t* target)
{
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_PlaySound
// Description:
//      Stub routine for difference player class sounds until the
//      faster, cheaper, cleaner method is ready to go
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_PlaySound(userEntity_t* self, const char* soundfile)
{
	char buf[256];

	_ASSERTE(self);

	playerHook_t* hook = AI_GetPlayerHook(self);

	_ASSERTE(hook);

	if (AI_IsMikiko(hook))  // mikiko sound?
	{
		//		sprintf(buf,"mikiko/%s",soundfile);
		Com_sprintf(buf, sizeof(buf), "mikiko/%s", soundfile);
	}
	else if (AI_IsSuperfly(hook)) // superfly sound?
	{
		//		sprintf(buf,"superfly/%s",soundfile);
		Com_sprintf(buf, sizeof(buf), "superfly/%s", soundfile);
	}
	else
		//		sprintf(buf,"hiro/%s",soundfile);
		Com_sprintf(buf, sizeof(buf), "hiro/%s", soundfile);

	gstate->StartEntitySound(self, CHAN_BODY, gstate->SoundIndex(buf), 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
}

// ----------------------------------------------------------------------------
// NSS[12/10/99]:
// Name:		SIDEKICK_SelectWeapon
// Description: Generic Weapon selection code based off of a simple rating system for monsters
// Input:userEntity_t *self, userEntity_t *pEnemy, weapon_t **aWeapon, ammo_t **aAmmo
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
userInventory_t* SIDEKICK_SelectWeapon(userEntity_t* self, userEntity_t* pEnemy, weapon_t** aWeapon, ammo_t** aAmmo)
{
	_ASSERTE(aWeapon);
	_ASSERTE(aAmmo);

	weapon_t* pWeapon = NULL;
	playerHook_t* hook = AI_GetPlayerHook(self);
	// NSS[2/14/00]:MikikoFly does not use weapons!
	if (hook->type == TYPE_MIKIKOFLY)
	{
		return NULL;
	}
	int nFirstChoice, nSecondChoice, nThirdChoice;

	if (pEnemy && AI_IsAlive(pEnemy))
	{
		playerHook_t* pEnemyHook = AI_GetPlayerHook(pEnemy);
		if (pEnemyHook)
		{
			nFirstChoice = aSidekickCombatData[pEnemyHook->type].nFirstWeaponChoice;
			nSecondChoice = aSidekickCombatData[pEnemyHook->type].nSecondWeaponChoice;
			nThirdChoice = aSidekickCombatData[pEnemyHook->type].nThirdWeaponChoice;
		}
		else// NSS[1/20/00]:Not a valid monster get standard selection
		{
			nFirstChoice = 0;
			nSecondChoice = 1;
			nThirdChoice = 2;
		}
	}
	else // NSS[1/20/00]:No enemy, get the standard selection
	{
		nFirstChoice = 0;
		nSecondChoice = 1;
		nThirdChoice = 2;
	}
	// NSS[3/1/00]:Make sure this is a valid number
	if (nFirstChoice > 2)
		nFirstChoice = 0;
	if (nSecondChoice > 2)
		nSecondChoice = 1;
	if (nThirdChoice > 2)
		nThirdChoice = 2;


	weapon_t* pWeapon1 = aWeapon[nFirstChoice];
	weapon_t* pWeapon2 = aWeapon[nSecondChoice];
	weapon_t* pWeapon3 = aWeapon[nThirdChoice];

	if (!pWeapon1 || !pWeapon1->ammo || !SIDEKICK_CanUseWeapon(pWeapon1) || pWeapon2 || pWeapon3)
	{
		if (!pWeapon2 || !pWeapon2->ammo || !SIDEKICK_CanUseWeapon(pWeapon2) || pWeapon3)
		{
			if (!pWeapon3 || !pWeapon3->ammo || !SIDEKICK_CanUseWeapon(pWeapon3))
			{
				if (!pWeapon2 || !pWeapon2->ammo || !SIDEKICK_CanUseWeapon(pWeapon2))
				{
					if (!pWeapon1 || !pWeapon1->ammo || !SIDEKICK_CanUseWeapon(pWeapon1))
					{
						pWeapon = NULL;
					}
					else
					{
						pWeapon = pWeapon1;
					}
				}
				else
				{
					pWeapon = pWeapon2;
				}
			}
			else
			{
				pWeapon = pWeapon3;
			}
		}
		else
		{
			pWeapon = pWeapon2;
		}
	}
	else
	{
		pWeapon = pWeapon1;
	}

	return (userInventory_t*)pWeapon;
}

// ----------------------------------------------------------------------------
// NSS[12/10/99]:
// Name:		SIDEKICK_OwnerIsTooCloseToEnemy
// Description: For weapon selection...don't want to select a weapon that is explosive
// if the owner is right next to the monster.
// Input: userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------

int SIDEKICK_OwnerIsTooCloseToEnemy(userEntity_t* self)
{
	float fDistance;
	playerHook_t* hook = AI_GetPlayerHook(self);

	if (hook)
	{
		if (hook->owner && self->enemy)
		{
			fDistance = VectorDistance(self->enemy->s.origin, hook->owner->s.origin);
			if (fDistance < 156.0f)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

// ----------------------------------------------------------------------------
// NSS[12/10/99]:
// Name:		SIDEKICK_IsTooCloseToOwner
// Description: For weapon selection...don't want to select a weapon that is explosive
// if the owner is right next to the monster.
// Input: userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsTooCloseToOwner(userEntity_t* self)
{
	float fDistance;
	playerHook_t* hook = AI_GetPlayerHook(self);

	if (hook)
	{
		if (hook->owner)
		{
			fDistance = VectorDistance(hook->owner->s.origin, self->s.origin);
			if (fDistance < 96.0f)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}


// ----------------------------------------------------------------------------
// NSS[12/10/99]:
// Name:		SIDEKICK_IsTooCloseToEnemy
// Description: For weapon selection...don't want to select a weapon that is explosive
// if the owner is right next to the monster.
// Input: userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsTooCloseToEnemy(userEntity_t* self)
{
	float fDistance;
	if (self->enemy)
	{
		fDistance = VectorDistance(self->enemy->s.origin, self->s.origin);
		if (fDistance < 156.0f)
		{
			return TRUE;
		}
	}
	return FALSE;
}


// ----------------------------------------------------------------------------
// NSS[12/10/99]:
// Name:		SIDEKICK_IsTooFarFromEnemy
// Description: For weapon selection...don't want to select a weapon that spreads too much at a distance
// Input: userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsTooFarFromEnemy(userEntity_t* self)
{
	float fDistance;
	playerHook_t* hook = AI_GetPlayerHook(self);

	if (hook)
	{
		if (hook->owner && self->enemy)
		{
			fDistance = VectorDistance(self->enemy->s.origin, self->s.origin);
			if (fDistance > 400.0f)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

// ----------------------------------------------------------------------------

userInventory_t* SIDEKICK_ChooseBestWeaponForE1(userEntity_t* self, userEntity_t* pEnemy /* = NULL */)
{
	weapon_t* aWeapon[E1_NUMWEAPONS];
	ammo_t* aAmmo[E1_NUMWEAPONS];


	//	clear the ammo and weapons arrays
	memset(aWeapon, 0x00, E1_NUMWEAPONS * sizeof(weapon_t*));
	memset(aAmmo, 0x00, E1_NUMWEAPONS * sizeof(ammo_t*));

	///////////////////////////////////////////////////////////
	//	determine which weapons we have
	///////////////////////////////////////////////////////////

	//	search through the whole inventory list in one pass
	_ASSERTE(self->inventory);
	userInventory_t* inv = gstate->InventoryFirstItem(self->inventory);
	while (inv)
	{
		if (inv->flags & ITF_WEAPON)
		{
			if (inv->name)
			{
				//	look for weapons
				if (_stricmp(inv->name, "weapon_disruptor") == 0)
				{
					aWeapon[E1_DISRUPTOR] = (weapon_t*)inv;

					// disruptor glove does not need ammo
					if (aWeapon[E1_DISRUPTOR]->ammo)
					{
						aWeapon[E1_DISRUPTOR]->ammo->count = 1;
					}
				}
				else
					if (_stricmp(inv->name, "weapon_ionblaster") == 0)
					{
						aWeapon[E1_IONBLASTER] = (weapon_t*)inv;
					}
					else
						if (_stricmp(inv->name, "weapon_shotcycler") == 0 && !SIDEKICK_IsTooCloseToOwner(self) && !SIDEKICK_IsTooFarFromEnemy(self))
						{
							aWeapon[E1_SHOTCYCLER] = (weapon_t*)inv;
						}
						else
							if (_stricmp(inv->name, "ammo_disruptor") == 0)
							{
								aAmmo[E1_DISRUPTOR] = (ammo_t*)inv;
							}
							else
								if (_stricmp(inv->name, "ammo_ionpack") == 0)
								{
									aAmmo[E1_IONBLASTER] = (ammo_t*)inv;
								}
								else
									if (_stricmp(inv->name, "ammo_shells") == 0)
									{
										aAmmo[E1_SHOTCYCLER] = (ammo_t*)inv;
									}
			}
		}
		inv = gstate->InventoryNextItem(self->inventory);
	}

	if (aAmmo[E1_DISRUPTOR])
	{

	}

	userInventory_t* pWeapon = SIDEKICK_SelectWeapon(self, pEnemy, aWeapon, aAmmo);

	return pWeapon;
}

// ----------------------------------------------------------------------------

userInventory_t* SIDEKICK_ChooseBestWeaponForE2(userEntity_t* self, userEntity_t* pEnemy /* = NULL */)
{
	weapon_t* aWeapon[E1_NUMWEAPONS];
	ammo_t* aAmmo[E1_NUMWEAPONS];

	//	clear the ammo and weapons arrays
	memset(aWeapon, 0x00, E1_NUMWEAPONS * sizeof(weapon_t*));
	memset(aAmmo, 0x00, E1_NUMWEAPONS * sizeof(ammo_t*));

	///////////////////////////////////////////////////////////
	//	determine which weapons we have
	///////////////////////////////////////////////////////////

	//	search through the whole inventory list in one pass
	_ASSERTE(self->inventory);
	userInventory_t* inv = gstate->InventoryFirstItem(self->inventory);
	while (inv)
	{
		// SCG[11/5/99]: save some cycles, lets only do this for weapons..
		if (inv->flags & ITF_WEAPON)
		{
			if (inv->name)
			{
				//	look for weapons
				if (_stricmp(inv->name, "weapon_discus") == 0)
				{
					aWeapon[E2_DISCUS] = (weapon_t*)inv;
				}
				else
					if (_stricmp(inv->name, "weapon_venomous") == 0 && !SIDEKICK_OwnerIsTooCloseToEnemy(self))
					{
						aWeapon[E2_VENOMOUS] = (weapon_t*)inv;
					}
					else
						if (_stricmp(inv->name, "weapon_trident") == 0 && !SIDEKICK_OwnerIsTooCloseToEnemy(self))
						{
							aWeapon[E2_TRIDENT] = (weapon_t*)inv;
						}
						else
							if (_stricmp(inv->name, "ammo_discus") == 0)
							{
								aAmmo[E2_DISCUS] = (ammo_t*)inv;
							}
							else
								if (_stricmp(inv->name, "ammo_venomous") == 0)
								{
									aAmmo[E2_VENOMOUS] = (ammo_t*)inv;
								}
								else
									if (_stricmp(inv->name, "ammo_trident") == 0)
									{
										aAmmo[E2_TRIDENT] = (ammo_t*)inv;
									}
			}
		}

		inv = gstate->InventoryNextItem(self->inventory);
	}

	userInventory_t* pWeapon = SIDEKICK_SelectWeapon(self, pEnemy, aWeapon, aAmmo);

	return pWeapon;
}

// ----------------------------------------------------------------------------

userInventory_t* SIDEKICK_ChooseBestWeaponForE3(userEntity_t* self, userEntity_t* pEnemy /* = NULL */)
{
	weapon_t* aWeapon[E3_NUMWEAPONS];
	ammo_t* aAmmo[E3_NUMWEAPONS];

	//	clear the ammo and weapons arrays
	memset(aWeapon, 0x00, E3_NUMWEAPONS * sizeof(weapon_t*));
	memset(aAmmo, 0x00, E1_NUMWEAPONS * sizeof(ammo_t*));

	///////////////////////////////////////////////////////////
	//	determine which weapons we have
	///////////////////////////////////////////////////////////

	//	search through the whole inventory list in one pass
	_ASSERTE(self->inventory);
	userInventory_t* inv = gstate->InventoryFirstItem(self->inventory);
	while (inv)
	{
		if (inv->flags & ITF_WEAPON)
		{
			if (inv->name)
			{
				//	look for weapons
				if (_stricmp(inv->name, "weapon_silverclaw") == 0)
				{
					aWeapon[E3_SILVERCLAW] = (weapon_t*)inv;

					// silverclaw does not need ammo
					if (aWeapon[E3_SILVERCLAW]->ammo)
					{
						aWeapon[E3_SILVERCLAW]->ammo->count = 1;
					}
				}
				else
					if (_stricmp(inv->name, "weapon_bolter") == 0)
					{
						aWeapon[E3_BOLTER] = (weapon_t*)inv;
					}
					else // NSS[12/20/99]:Added distance check for both the owner and self to make sure we don't bust ourself up.
						if (_stricmp(inv->name, "weapon_ballista") == 0 && !SIDEKICK_OwnerIsTooCloseToEnemy(self) && !SIDEKICK_IsTooCloseToEnemy(self))
						{
							aWeapon[E3_BALLISTA] = (weapon_t*)inv;
						}
						else
							if (_stricmp(inv->name, "ammo_bolts") == 0)
							{
								aAmmo[E3_BOLTER] = (ammo_t*)inv;
							}
							else
								if (_stricmp(inv->name, "ammo_ballista") == 0)
								{
									aAmmo[E3_BALLISTA] = (ammo_t*)inv;
								}
			}
		}
		inv = gstate->InventoryNextItem(self->inventory);
	}

	userInventory_t* pWeapon = SIDEKICK_SelectWeapon(self, pEnemy, aWeapon, aAmmo);

	return pWeapon;
}

// ----------------------------------------------------------------------------

userInventory_t* SIDEKICK_ChooseBestWeaponForE4(userEntity_t* self, userEntity_t* pEnemy /* = NULL */)
{
	weapon_t* aWeapon[E4_NUMWEAPONS];
	ammo_t* aAmmo[E4_NUMWEAPONS];

	//	clear the ammo and weapons arrays
	memset(aWeapon, 0x00, E4_NUMWEAPONS * sizeof(weapon_t*));
	memset(aAmmo, 0x00, E1_NUMWEAPONS * sizeof(ammo_t*));

	///////////////////////////////////////////////////////////
	//	determine which weapons we have
	///////////////////////////////////////////////////////////

	//	search through the whole inventory list in one pass
	_ASSERTE(self->inventory);
	userInventory_t* inv = gstate->InventoryFirstItem(self->inventory);
	while (inv)
	{
		if (inv->flags & ITF_WEAPON)
		{
			if (inv->name)
			{
				//	look for weapons
				if (_stricmp(inv->name, "weapon_glock") == 0)
				{
					aWeapon[E4_GLOCK] = (weapon_t*)inv;
				}
				else
					if (_stricmp(inv->name, "weapon_ripgun") == 0)
					{
						aWeapon[E4_RIPGUN] = (weapon_t*)inv;
					}
					else
						if (_stricmp(inv->name, "weapon_slugger") == 0 && !SIDEKICK_IsTooCloseToOwner(self) && !SIDEKICK_IsTooFarFromEnemy(self))
						{
							aWeapon[E4_SLUGGER] = (weapon_t*)inv;
						}
						else
							if (_stricmp(inv->name, "ammo_bullets") == 0)
							{
								aAmmo[E4_GLOCK] = (ammo_t*)inv;
							}
							else
								if (_stricmp(inv->name, "ammo_ripgun") == 0)
								{
									aAmmo[E4_RIPGUN] = (ammo_t*)inv;
								}
								else
									if (_stricmp(inv->name, "ammo_slugger") == 0)
									{
										aAmmo[E4_SLUGGER] = (ammo_t*)inv;
									}
			}
		}

		inv = gstate->InventoryNextItem(self->inventory);
	}

	userInventory_t* pWeapon = SIDEKICK_SelectWeapon(self, pEnemy, aWeapon, aAmmo);

	return pWeapon;
}

// ----------------------------------------------------------------------------

void SIDEKICK_ChooseBestWeapon(userEntity_t* self)
{
	_ASSERTE(self);
	//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	userEntity_t* pEnemy = self->enemy;

	userInventory_t* pCurrentWeapon = NULL;
	int nCurrentEpisode = GetCurrentEpisode();
	switch (nCurrentEpisode)
	{
	case 1:
	{
		pCurrentWeapon = SIDEKICK_ChooseBestWeaponForE1(self, pEnemy);
		break;
	}
	case 2:
	{
		pCurrentWeapon = SIDEKICK_ChooseBestWeaponForE2(self, pEnemy);
		break;
	}
	case 3:
	{
		pCurrentWeapon = SIDEKICK_ChooseBestWeaponForE3(self, pEnemy);
		break;
	}
	case 4:
	{
		pCurrentWeapon = SIDEKICK_ChooseBestWeaponForE4(self, pEnemy);
		break;
	}
	default:
	{
		pCurrentWeapon = NULL;
		break;
	}
	}

	if (pCurrentWeapon)
	{
		if ((self->curWeapon != pCurrentWeapon) || (self->winfo == NULL))
		{
			ai_weapon_t* pAIWeapon = (ai_weapon_t*)pCurrentWeapon;

			weapon_t* pWeapon = &(pAIWeapon->weapon);
			weaponInfo_t* winfo = pWeapon->winfo;
			_ASSERTE(winfo);

			winfo->select_func(self);

			self->curWeapon = pCurrentWeapon;
		}
	}
	else
	{
		self->curWeapon = NULL;
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_ChooseMeleeWeapon
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_ChooseMeleeWeapon(userEntity_t* self)
{
	_ASSERTE(self);
	//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	_ASSERTE(self->inventory);

	userInventory_t* pCurrentWeapon = NULL;
	int nCurrentEpisode = GetCurrentEpisode();
	switch (nCurrentEpisode)
	{
	case 1:
	{
		pCurrentWeapon = gstate->InventoryFindItem(self->inventory, "weapon_disruptor");
		break;
	}
	case 2:
	{
		pCurrentWeapon = gstate->InventoryFindItem(self->inventory, "weapon_discus");
		break;
	}
	case 3:
	{
		pCurrentWeapon = gstate->InventoryFindItem(self->inventory, "weapon_silverclaw");
		break;
	}
	case 4:
	{
		pCurrentWeapon = gstate->InventoryFindItem(self->inventory, "weapon_glock");
		break;
	}
	default:
	{
		pCurrentWeapon = gstate->InventoryFindItem(self->inventory, "weapon_disruptor");
		break;
	}
	}

	if (pCurrentWeapon)
	{
		ai_weapon_t* pAIWeapon = (ai_weapon_t*)pCurrentWeapon;

		weapon_t* pWeapon = &(pAIWeapon->weapon);
		weaponInfo_t* winfo = pWeapon->winfo;
		_ASSERTE(winfo);

		winfo->select_func(self);

		self->curWeapon = pCurrentWeapon;

		return TRUE;
	}
	else
	{
		self->curWeapon = NULL;
	}

	return FALSE;
}




// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_CanPickupWeapon
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_CanPickupWeapon(userEntity_t* pItem)
{
	if (!ITEM_IsWeapon(pItem))
	{
		return FALSE;
	}

	if (pItem->spawnflags & DROPPED_PLAYER_ITEM)
	{
		return FALSE;
	}

	int bRetValue = FALSE;


	int nCurrentEpisode = GetCurrentEpisode();
	switch (nCurrentEpisode)
	{
	case 1:
	{
		if (_stricmp(pItem->className, "weapon_disruptor") == 0 ||
			_stricmp(pItem->className, "weapon_ionblaster") == 0 ||
			_stricmp(pItem->className, "weapon_shotcycler") == 0)
		{

			bRetValue = TRUE;
		}
		break;
	}
	case 2:
	{
		if (_stricmp(pItem->className, "weapon_discus") == 0 ||
			_stricmp(pItem->className, "weapon_venomous") == 0)
		{
			bRetValue = TRUE;
		}
		break;
	}
	case 3:
	{
		if (_stricmp(pItem->className, "weapon_silverclaw") == 0 ||
			_stricmp(pItem->className, "weapon_bolter") == 0 ||
			_stricmp(pItem->className, "weapon_ballista") == 0)
		{
			bRetValue = TRUE;
		}
		break;
	}
	case 4:
	{
		if (_stricmp(pItem->className, "weapon_glock") == 0 ||
			_stricmp(pItem->className, "weapon_ripgun") == 0 ||
			_stricmp(pItem->className, "weapon_slugger") == 0)
		{
			bRetValue = TRUE;
		}
		break;
	}
	default:
	{
		bRetValue = TRUE;
		break;
	}
	}
	return bRetValue;
}

// ----------------------------------------------------------------------------
// NSS[1/31/00]:
// Name:        SIDEKICK_CanPickupAmmo
// Description:This will first determine if we can pickup that type of ammo and then will
// make sure that we aren't already full in our inventory.
// Input:userEntity_t *pItem, userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_CanPickupAmmo(userEntity_t* pItem, userEntity_t* self)
{
	int bRetValue = TRUE;
	int nCurrentEpisode = GetCurrentEpisode();
	char Buffer[64];


	if (!ITEM_IsAmmo(pItem))
	{
		return FALSE;
	}

	switch (nCurrentEpisode)
	{
	case 1:
	{
		if (_stricmp(pItem->className, "ammo_ionpack") == 0)
		{
			Com_sprintf(Buffer, sizeof(Buffer), "weapon_ionblaster");
		}
		else if (_stricmp(pItem->className, "ammo_shells") == 0)
		{
			Com_sprintf(Buffer, sizeof(Buffer), "weapon_shotcycler");
		}
		else
		{
			bRetValue = FALSE;
		}
		break;
	}
	case 2:
	{
		if (_stricmp(pItem->className, "ammo_venomous") == 0)
		{
			Com_sprintf(Buffer, sizeof(Buffer), "weapon_venomous");
		}
		else if (_stricmp(pItem->className, "ammo_trident") == 0)
		{
			Com_sprintf(Buffer, sizeof(Buffer), "weapon_trident");
		}
		else
		{
			bRetValue = FALSE;
		}

		break;
	}
	case 3:
	{
		if (_stricmp(pItem->className, "ammo_bolts") == 0)
		{
			Com_sprintf(Buffer, sizeof(Buffer), "weapon_bolter");
		}
		else if (_stricmp(pItem->className, "ammo_ballista") == 0)
		{
			Com_sprintf(Buffer, sizeof(Buffer), "weapon_ballista");
		}
		else
		{
			bRetValue = FALSE;
		}

		break;
	}
	case 4:
	{
		if (_stricmp(pItem->className, "ammo_bullets") == 0)
		{
			Com_sprintf(Buffer, sizeof(Buffer), "weapon_glock");
		}
		else if (_stricmp(pItem->className, "ammo_ripgun") == 0)
		{
			Com_sprintf(Buffer, sizeof(Buffer), "weapon_ripgun");
		}
		else if (_stricmp(pItem->className, "ammo_slugger") == 0)
		{
			Com_sprintf(Buffer, sizeof(Buffer), "weapon_slugger");
		}
		else
		{
			bRetValue = FALSE;
		}
		break;
	}
	default:
	{
		bRetValue = FALSE;
		break;
	}
	}

	// NSS[1/31/00]:If we can actually use this weapon
	if (bRetValue)
	{
		weapon_t* pWeapon = (weapon_t*)gstate->InventoryFindItem(self->inventory, Buffer);
		if (pWeapon)
		{
			// NSS[1/31/00]:As long as we aren't maxed out.
			if (pWeapon->ammo->count >= pWeapon->winfo->ammo_max)
			{
				bRetValue = FALSE;
			}
		}
		else
		{
			bRetValue = FALSE;
		}
	}

	return bRetValue;
}

void SIDEKICK_GiveEpisode1TestWeapons(userEntity_t* self)
{
	static const char* weaponNames[] =
	{
		"weapon_disruptor",
		"weapon_ionblaster",
		"weapon_shotcycler"
	};
	int weaponIndex;

	if (!self || !self->inventory)
		return;

	for (weaponIndex = 0; weaponIndex < 3; ++weaponIndex)
	{
		char weaponName[64];
		weaponInfo_t* weaponInfo;

		strcpy(weaponName, weaponNames[weaponIndex]);
		weaponInfo = com->FindRegisteredWeapon(weaponName);

		if (weaponInfo && weaponInfo->give_func)
			weaponInfo->give_func(self, weaponInfo->initialAmmo);
	}

	SIDEKICK_ChooseBestWeapon(self);
}

void SIDEKICK_SpawnBothWithEpisode1TestWeapons(userEntity_t* self)
{
	userEntity_t* firstSidekick;
	userEntity_t* secondSidekick;

	if (!self)
		return;

	SIDEKICK_SpawnSuperfly2(self);
	SIDEKICK_SpawnMikiko2(self);

	firstSidekick = AIINFO_GetFirstSidekick();
	secondSidekick = AIINFO_GetSecondSidekick();

	if (firstSidekick)
		SIDEKICK_GiveEpisode1TestWeapons(firstSidekick);

	if (secondSidekick)
		SIDEKICK_GiveEpisode1TestWeapons(secondSidekick);
}

// ----------------------------------------------------------------------------
// NSS[12/14/99]:
// Name:		SIDEKICK_DetermineNeedWeapon
// Description: Will determine if the sidekick can pickup a specific special weapon type.
// Input: userEntity_t *self, char *Weapon_Name
// Output:TRUE/FALSE
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_DetermineNeedWeapon(userEntity_t* self, char* Weapon_Name)
{
	userInventory_t* inv = gstate->InventoryFirstItem(self->inventory);

	weapon_t* item = (weapon_t*)gstate->InventoryFindItem(self->inventory, Weapon_Name);
	if (!item)
		return TRUE;

	if (item->ammo->count >= item->winfo->ammo_max)
		return FALSE;

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_CanPickupItem
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_CanPickupItem(userEntity_t* self, userEntity_t* pItem, int bForcePickup /* = FALSE */)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	_ASSERTE(AI_IsSidekick(hook));

	int RetValue = FALSE;
	if (!ITEM_IsItem(pItem))
	{
		return FALSE;
	}
	float XYDistance = VectorXYDistance(self->s.origin, pItem->s.origin);
	float ZDistance = VectorZDistance(self->s.origin, pItem->s.origin);

	// NSS[2/29/00]:If we cannot find a path to the item.
	if (!AI_FindPathToEntity(self, pItem, TRUE))
	{
		if (!AI_IsVisible(self, pItem))
		{
			return -1;
		}
		else if (!(XYDistance < 255.0f && ZDistance < 32.0f))
		{
			return -1;
		}

	}

	if (pItem->nSidekickFlag & MIKIKO_NO_PICKUP && hook->type == TYPE_MIKIKO && !bForcePickup)
	{
		return FALSE;
	}
	else if (pItem->nSidekickFlag & SUPERFLY_NO_PICKUP && (hook->type == TYPE_SUPERFLY || hook->type == TYPE_MIKIKOFLY) && !bForcePickup)
	{
		return FALSE;
	}

	if (strstr(pItem->className, "item_health"))
	{
		if (self->health < (hook->base_health - (hook->base_health * .05)))
		{
			RetValue = TRUE;
		}
	}
	else
		if (_stricmp(pItem->className, "item_goldensoul") == 0)
		{
			if (self->health <= 100 || bForcePickup)
			{
				RetValue = TRUE;
			}
		}
		else
			if (strstr(pItem->className, "weapon"))
			{
				// all weapons should be flagged anyway, if not
				// NSS[2/14/00]:MikikoFly does not use weapons!
				if (hook->type == TYPE_MIKIKOFLY)
				{
					return FALSE;
				}
				// NSS[3/28/00]:Do a small filter to make sure we need this weapon type.
				if (!SIDEKICK_DetermineNeedWeapon(self, pItem->className))
					return FALSE;

				RetValue = SIDEKICK_CanPickupWeapon(pItem);
			}
			else
				if (strstr(pItem->className, "ammo"))
				{
					// NSS[2/14/00]:MikikoFly does not use weapons!
					if (hook->type == TYPE_MIKIKOFLY)
					{
						return FALSE;
					}

					RetValue = SIDEKICK_CanPickupAmmo(pItem, self);
				}
				else
					if (strstr(pItem->className, "armor"))
					{
						if ((self->health * ((float)pItem->armor_abs / 100.0)) > (self->armor_val * self->armor_abs))
						{
							RetValue = TRUE;
						}
					}
					else
						if (_stricmp(pItem->className, "item_power_boost") == 0 ||
							_stricmp(pItem->className, "item_acro_boost") == 0 ||
							_stricmp(pItem->className, "item_attack_boost") == 0 ||
							_stricmp(pItem->className, "item_speed_boost") == 0 ||
							_stricmp(pItem->className, "item_vita_boost") == 0)
						{
							// no power ups for
							return FALSE;
						}
						else if (((!_stricmp(pItem->className, "misc_lifewater")) || (!_stricmp(pItem->className, "misc_hosportal")) || (!_stricmp(pItem->className, "misc_healthtree"))))
						{
							if (!_stricmp(pItem->className, "misc_healthtree"))
							{
								healthtreeHook_t* ihook = (healthtreeHook_t*)pItem->userHook;
								if (ihook && hook)
								{
									// NSS[5/24/00]:As long as we have fruit and low health then we can still use this item.
									if (ihook->current_fruit && (self->health < (hook->base_health - (hook->base_health * .05))))
									{
										RetValue = TRUE;
									}
								}
							}
							else
							{
								hosportalHook_t* ihook = (hosportalHook_t*)pItem->userHook;
								if (ihook && hook)
								{
									// NSS[5/19/00]:If the hosportal has some juice left in it and we need health then let's do it.
									if (ihook->current_juice > 0 && (self->health < (hook->base_health - (hook->base_health * .05))))
									{
										RetValue = TRUE;
									}
								}
							}
						}

	return RetValue;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnHiro
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t* SIDEKICK_SpawnHiro(userEntity_t* self)
{
	return SIDEKICK_Spawn(self, TYPE_CLIENT);
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnSuperfly
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t* SIDEKICK_SpawnSuperfly(userEntity_t* self)
{
	return SIDEKICK_Spawn(self, TYPE_SUPERFLY);
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnMikiko
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t* SIDEKICK_SpawnMikiko(userEntity_t* self)
{
	return SIDEKICK_Spawn(self, TYPE_MIKIKO);
}


// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnMikikoFly
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t* SIDEKICK_SpawnMikikoFly(userEntity_t* self)
{
	return SIDEKICK_Spawn(self, TYPE_MIKIKOFLY);
}


// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnHiro2
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SpawnHiro2(userEntity_t* self)
{
	SIDEKICK_Spawn(self, TYPE_CLIENT);
}


// ----------------------------------------------------------------------------
// NSS[2/15/00]:
// Name:		SIDEKICK_TeleportSetup
// Description:Setups the teleport ihnformation then assigns the task
// Input:userEntity_t *self (where self is the player), MONSTER_TYPE Type
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_TeleportSetup(userEntity_t* self, MONSTER_TYPE Type)
{
	userEntity_t* Sidekick;
	playerHook_t* hook;

	if (Type == TYPE_SUPERFLY || Type == TYPE_MIKIKOFLY)
	{
		Sidekick = AIINFO_GetSuperfly();
	}
	else
	{
		Sidekick = AIINFO_GetMikiko();
	}

	if (Sidekick)
	{
		hook = AI_GetPlayerHook(Sidekick);
		if (hook->type == Type)
		{
			AI_AddNewTaskAtFront(Sidekick, TASKTYPE_SIDEKICK_TELEPORT, self->s.origin);
		}
	}
}


// ----------------------------------------------------------------------------
// NSS[2/15/00]:
// Name:		SIDEKICK_TeleportSuperfly
// Description:Teleports Superfly close to the player
// Input:userEntity_t *self (where self is the player)
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_TeleportSuperfly(userEntity_t* self)
{
	SIDEKICK_TeleportSetup(self, TYPE_SUPERFLY);
}

// ----------------------------------------------------------------------------
// NSS[2/15/00]:
// Name:		SIDEKICK_TeleportMikiko
// Description:Teleports Mikikofly close to the player
// Input:userEntity_t *self (where self is the player)
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_TeleportMikiko(userEntity_t* self)
{
	SIDEKICK_TeleportSetup(self, TYPE_MIKIKO);

}

// ----------------------------------------------------------------------------
// NSS[2/15/00]:
// Name:		SIDEKICK_TeleportMikikoFly
// Description:Teleports Mikikofly close to the player
// Input:userEntity_t *self (where self is the player)
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_TeleportMikikoFly(userEntity_t* self)
{
	SIDEKICK_TeleportSetup(self, TYPE_MIKIKOFLY);
}




// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnSuperfly2
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SpawnSuperfly2(userEntity_t* self)
{
	SIDEKICK_Spawn(self, TYPE_SUPERFLY);
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnMikiko2
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SpawnMikiko2(userEntity_t* self)
{
	SIDEKICK_Spawn(self, TYPE_MIKIKO);

}

// ----------------------------------------------------------------------------
// NSS[12/16/99]:
// Name:		SIDEKICK_SpawnMikikoFly2
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SpawnMikikoFly2(userEntity_t* self)
{
	SIDEKICK_Spawn(self, TYPE_MIKIKOFLY);
}
// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnHiro
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t* SIDEKICK_SpawnHiro(const CVector& origin, const CVector& angle)
{
	return SIDEKICK_Spawn(TYPE_CLIENT, origin, angle);
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnSuperfly
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t* SIDEKICK_SpawnSuperfly(const CVector& origin, const CVector& angle)
{
	return SIDEKICK_Spawn(TYPE_SUPERFLY, origin, angle);
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnMikiko
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t* SIDEKICK_SpawnMikiko(const CVector& origin, const CVector& angle)
{
	return SIDEKICK_Spawn(TYPE_MIKIKO, origin, angle);
}



// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnMikikoFly
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t* SIDEKICK_SpawnMikikoFly(const CVector& origin, const CVector& angle)
{
	return SIDEKICK_Spawn(TYPE_MIKIKOFLY, origin, angle);
}



// ----------------------------------------------------------------------------
//
// Name:		info_mikikofly_start
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void info_mikikofly_start(userEntity_t* self)
{
	if (sv_no_sidekicks && sv_no_sidekicks->value > 0.0f)
		return;
	// SCG[2/11/00]: the model still needs to be registered, though!!!
	char* szModelName = AIATTRIBUTE_GetModelName(SUPERFLY_CLASSNAME);
	if (szModelName == NULL)
	{
		gstate->Error("Could not get superfly model info");
	}
	gstate->ModelIndex(szModelName);

	for (int i = 0; self->epair[i].key; i++)
	{
		if (!stricmp(self->epair[i].key, "targetname"))
			self->targetname = self->epair[i].value;
	}

	// SCG[2/6/00]: do not spawn on level transitions.( that is taken care of in dll_putclientinserver )
	// SCG[2/6/00]: this will allow us leave sidekicks behind on certain levels.
	if ((gstate->nLevelTransitionType == 1) && (self->targetname == NULL))
	{
		return;
	}

	if (self->targetname)
	{
		if (strcmp(gstate->game->spawnpoint, self->targetname))
		{
			return;
		}
	}

	// SCG[1/28/00]: Create a new entity for the sidekick
	userEntity_t* pSidekickEntity = gstate->SpawnEntity();
	if (pSidekickEntity == NULL)
	{
		return;
	}

	pSidekickEntity->s.origin = self->s.origin;
	pSidekickEntity->s.angles = self->s.angles;

	SIDEKICK_Start(pSidekickEntity, TYPE_MIKIKOFLY);

	gstate->Con_Printf("info_superfly_start\n");
}

// ----------------------------------------------------------------------------
//
// Name:		info_superfly_start
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void info_superfly_start(userEntity_t* self)
{
	if (sv_no_sidekicks && sv_no_sidekicks->value > 0.0f)
		return;
	// SCG[2/11/00]: the model still needs to be registered, though!!!
	char* szModelName = AIATTRIBUTE_GetModelName(SUPERFLY_CLASSNAME);
	if (szModelName == NULL)
	{
		gstate->Error("Could not get superfly model info");
	}
	gstate->ModelIndex(szModelName);

	for (int i = 0; self->epair[i].key; i++)
	{
		if (!stricmp(self->epair[i].key, "targetname"))
			self->targetname = self->epair[i].value;
	}

	// SCG[2/6/00]: do not spawn on level transitions.( that is taken care of in dll_putclientinserver )
	// SCG[2/6/00]: this will allow us leave sidekicks behind on certain levels.
	if ((gstate->nLevelTransitionType == 1) && (self->targetname == NULL))
	{
		return;
	}

	if (self->targetname)
	{
		if (strcmp(gstate->game->spawnpoint, self->targetname))
		{
			return;
		}
	}

	// SCG[1/28/00]: Create a new entity for the sidekick
	userEntity_t* pSidekickEntity = gstate->SpawnEntity();
	if (pSidekickEntity == NULL)
	{
		return;
	}

	pSidekickEntity->s.origin = self->s.origin;
	pSidekickEntity->s.angles = self->s.angles;

	SIDEKICK_Start(pSidekickEntity, TYPE_SUPERFLY);

	gstate->Con_Printf("info_superfly_start\n");
}

// ----------------------------------------------------------------------------
//
// Name:		info_mikiko_start
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void info_mikiko_start(userEntity_t* self)
{
	if (sv_no_sidekicks && sv_no_sidekicks->value > 0.0f)
		return;
	// SCG[2/11/00]: the model still needs to be registered, though!!!
	char* szModelName = AIATTRIBUTE_GetModelName(MIKIKO_CLASSNAME);
	if (szModelName == NULL)
	{
		gstate->Error("Could not get superfly model info");
	}
	gstate->ModelIndex(szModelName);

	for (int i = 0; self->epair[i].key; i++)
	{
		if (!stricmp(self->epair[i].key, "targetname"))
			self->targetname = self->epair[i].value;
	}

	// SCG[2/6/00]: do not spawn on level transitions.( that is taken care of in dll_putclientinserver )
	// SCG[2/6/00]: this will allow us leave sidekicks behind on certain levels.
	if ((gstate->nLevelTransitionType == 1) && (self->targetname == NULL))
	{
		return;
	}

	if (self->targetname)
	{
		if (strcmp(gstate->game->spawnpoint, self->targetname))
		{
			return;
		}
	}

	// SCG[1/28/00]: Create a new entity for the sidekick
	userEntity_t* pSidekickEntity = gstate->SpawnEntity();
	if (pSidekickEntity == NULL)
	{
		return;
	}

	pSidekickEntity->s.origin = self->s.origin;
	pSidekickEntity->s.angles = self->s.angles;

	SIDEKICK_Start(pSidekickEntity, TYPE_MIKIKO);

	gstate->Con_Printf("info_mikiko_start\n");
}

// ----------------------------------------------------------------------------
// NSS[1/26/00]:
// Name:		SIDEKICK_IsNeedHealth
// Description:Do we need health?
// Input:userEntity_t *self
// Output:TRUE/FALSE
// ----------------------------------------------------------------------------
int SIDEKICK_IsNeedHealth(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	if (hook)
	{
		if (self->health < hook->base_health * 0.50)
		{
			return TRUE;
		}
	}
	return FALSE;
}



// ----------------------------------------------------------------------------
// NSS[2/29/00]:
// Name:		SIDEKICK_DetermineNeededItem
// Description:Determines if the Sidekick Needs something
// Input:userEntity_t *self
// Output:NA
// Note: I have made modifications to this... by far this is not how I would handle
// doing this whole item thang....
// ----------------------------------------------------------------------------
userEntity_t* SIDEKICK_DetermineNeededItem(userEntity_t* self)
{
	_ASSERTE(self->inventory);

	userEntity_t* pNeedItem = NULL;

	if (SIDEKICK_IsNeedHealth(self) == TRUE)
	{
		pNeedItem = SIDEKICK_GetClosestVisibleItem(self, NULL, 0.0f, ITEMTYPE_HEALTH);
	}

	if (!pNeedItem)
	{
		pNeedItem = SIDEKICK_GetClosestVisibleWeaponNotInInventory(self);
	}

	if (!pNeedItem)
	{
		pNeedItem = SIDEKICK_GetClosestVisibleAmmoNotInInventory(self);
	}

	// NSS[2/29/00]:Check to make sure we can get to it.
	if (AI_FindPathToEntity(self, pNeedItem, TRUE) && SIDEKICK_DeterminePathLength(self, pNeedItem) < 256.0f)
		return pNeedItem;
	else
		return NULL;
}


// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_DetermineWantItem
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t* SIDEKICK_DetermineWantItem(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);

	userEntity_t* pWantItem = NULL;
	if (self->health < hook->base_health)
	{
		// find a goldensoul
		pWantItem = SIDEKICK_GetClosestVisibleItem(self, NULL, 0.0f, ITEMTYPE_GOLDENSOUL);
		//pWantItem = SIDEKICK_GetClosestVisibleGoldenSoul( self );
		if (pWantItem)
		{
			return pWantItem;
		}
	}

	if (self->health < hook->base_health)
	{
		pWantItem = SIDEKICK_GetClosestVisibleItem(self, NULL, 0.0f, ITEMTYPE_HEALTH);
		//pWantItem = SIDEKICK_GetClosestVisibleHealth( self );
		if (pWantItem)
		{
			return pWantItem;
		}
	}

	if (self->armor_val <= 75)
	{
		// find a nearest armor	
		pWantItem = SIDEKICK_GetClosestVisibleItem(self, NULL, 0.0f, ITEMTYPE_ARMOR);
		//pWantItem = SIDEKICK_GetClosestVisibleArmor( self );
		if (pWantItem)
		{
			return pWantItem;
		}
	}
	else
		if (self->armor_val <= 150)
		{

			pWantItem = SIDEKICK_GetClosestVisibleItem(self, "item_plasteel_armor", 0.0f, ITEMTYPE_ARMOR);
			//pWantItem = SIDEKICK_GetClosestVisibleArmor( self, "item_plasteel_armor" );

			if (pWantItem)
			{
				return pWantItem;
			}
		}

	// look for stat boost
	if (!(hook->items & IT_POWERBOOST))
	{

		pWantItem = SIDEKICK_GetClosestVisibleItem(self, "item_power_boost", 0.0f, ITEMTYPE_STATBOOST);
		//pWantItem = SIDEKICK_GetClosestVisibleStatBoost( self, "item_power_boost" );
		if (pWantItem)
		{
			return pWantItem;
		}
	}
	if (!(hook->items & IT_ATTACKBOOST))
	{
		pWantItem = SIDEKICK_GetClosestVisibleItem(self, "item_attack_boost", 0.0f, ITEMTYPE_STATBOOST);
		//pWantItem = SIDEKICK_GetClosestVisibleStatBoost( self, "item_attack_boost" );
		if (pWantItem)
		{
			return pWantItem;
		}
	}
	if (!(hook->items & IT_SPEEDBOOST))
	{
		pWantItem = SIDEKICK_GetClosestVisibleItem(self, "item_speed_boost", 0.0f, ITEMTYPE_STATBOOST);
		//pWantItem = SIDEKICK_GetClosestVisibleStatBoost( self, "item_speed_boost" );
		if (pWantItem)
		{
			return pWantItem;
		}
	}
	if (!(hook->items & IT_ACROBOOST))
	{
		pWantItem = SIDEKICK_GetClosestVisibleItem(self, "item_acro_boost", 0.0f, ITEMTYPE_STATBOOST);
		//pWantItem = SIDEKICK_GetClosestVisibleStatBoost( self, "item_acro_boost" );
		if (pWantItem)
		{
			return pWantItem;
		}
	}
	if (!(hook->items & IT_VITABOOST))
	{
		pWantItem = SIDEKICK_GetClosestVisibleItem(self, "item_vita_boost", 0.0f, ITEMTYPE_STATBOOST);
		//pWantItem = SIDEKICK_GetClosestVisibleStatBoost( self, "item_vita_boost" );
		if (pWantItem)
		{
			return pWantItem;
		}
	}

	// now look for weapons
	_ASSERTE(self->inventory);

	int nCurrentEpisode = GetCurrentEpisode();
	switch (nCurrentEpisode)
	{
	case 1:
	{
		weapon_t* pWeapon = (weapon_t*)gstate->InventoryFindItem(self->inventory, "weapon_shotcycler");
		if (!pWeapon)
		{
			// find a closest shotcyler
			pWantItem = SIDEKICK_GetClosestVisibleItem(self, "weapon_shotcycler", 0.0f, ITEMTYPE_WEAPONS);
			//pWantItem = SIDEKICK_GetClosestVisibleWeapon( self, "weapon_shotcycler" );
			if (pWantItem)
			{
				return pWantItem;
			}
		}
		pWeapon = (weapon_t*)gstate->InventoryFindItem(self->inventory, "weapon_ionblaster");
		if (!pWeapon)
		{
			// find a closest ionblaster
			pWantItem = SIDEKICK_GetClosestVisibleItem(self, "weapon_ionblaster", 0.0f, ITEMTYPE_WEAPONS);
			//pWantItem = SIDEKICK_GetClosestVisibleWeapon( self, "weapon_ionblaster" );
			if (pWantItem)
			{
				return pWantItem;
			}
		}

		break;
	}
	case 2:
	{
		weapon_t* pWeapon = (weapon_t*)gstate->InventoryFindItem(self->inventory, "weapon_discus");
		if (!pWeapon)
		{
			// find a closest shotcyler
			pWantItem = SIDEKICK_GetClosestVisibleItem(self, "weapon_discus", 0.0f, ITEMTYPE_WEAPONS);
			if (pWantItem)
			{
				return pWantItem;
			}
		}
		pWeapon = (weapon_t*)gstate->InventoryFindItem(self->inventory, "weapon_venomous");
		if (!pWeapon)
		{
			// find a closest ionblaster
			pWantItem = SIDEKICK_GetClosestVisibleItem(self, "weapon_venomous", 0.0f, ITEMTYPE_WEAPONS);
			if (pWantItem)
			{
				return pWantItem;
			}
		}

		break;
	}
	case 3:
	{
		weapon_t* pWeapon = (weapon_t*)gstate->InventoryFindItem(self->inventory, "weapon_silverclaw");
		if (!pWeapon)
		{
			// find a closest shotcyler
			pWantItem = SIDEKICK_GetClosestVisibleItem(self, "weapon_silverclaw", 0.0f, ITEMTYPE_WEAPONS);
			if (pWantItem)
			{
				return pWantItem;
			}
		}
		pWeapon = (weapon_t*)gstate->InventoryFindItem(self->inventory, "weapon_bolter");
		if (!pWeapon)
		{
			// find a closest ionblaster
			pWantItem = SIDEKICK_GetClosestVisibleItem(self, "weapon_bolter", 0.0f, ITEMTYPE_WEAPONS);
			if (pWantItem)
			{
				return pWantItem;
			}
		}
		pWeapon = (weapon_t*)gstate->InventoryFindItem(self->inventory, "weapon_ballista");
		if (!pWeapon)
		{
			// find a closest ionblaster
			pWantItem = SIDEKICK_GetClosestVisibleItem(self, "weapon_ballista", 0.0f, ITEMTYPE_WEAPONS);
			if (pWantItem)
			{
				return pWantItem;
			}
		}
		break;
	}
	case 4:
	{
		weapon_t* pWeapon = (weapon_t*)gstate->InventoryFindItem(self->inventory, "weapon_glock");
		if (!pWeapon)
		{
			// find a closest shotcyler
			pWantItem = SIDEKICK_GetClosestVisibleItem(self, "weapon_glock", 0.0f, ITEMTYPE_WEAPONS);
			if (pWantItem)
			{
				return pWantItem;
			}
		}
		pWeapon = (weapon_t*)gstate->InventoryFindItem(self->inventory, "weapon_slugger");
		if (!pWeapon)
		{
			// find a closest ionblaster
			pWantItem = SIDEKICK_GetClosestVisibleItem(self, "weapon_slugger", 0.0f, ITEMTYPE_WEAPONS);
			if (pWantItem)
			{
				return pWantItem;
			}
		}
		pWeapon = (weapon_t*)gstate->InventoryFindItem(self->inventory, "weapon_ripgun");
		if (!pWeapon)
		{
			// find a closest ionblaster
			pWantItem = SIDEKICK_GetClosestVisibleItem(self, "weapon_ripgun", 0.0f, ITEMTYPE_WEAPONS);
			if (pWantItem)
			{
				return pWantItem;
			}
		}
		break;
	}

	default:
		break;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_IsWeaponAvailable
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsWeaponAvailable(userEntity_t* self, char* szWeaponName)
{
	_ASSERTE(self);

	if (!self->inventory)
	{
		return FALSE;
	}

	weapon_t* pWeapon = (weapon_t*)gstate->InventoryFindItem(self->inventory, szWeaponName);
	if (!pWeapon)
	{
		return FALSE;
	}

	return TRUE;
}

//NSS[12/1/99]:Only for sidekicks
int AI_IsSideKickCloseToItem(playerHook_t* hook, float fDistance);
#define HEALTH	1
#define AMMO	2
#define ARMOR	3
// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_StartPickupItem
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartPickupItem(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	_ASSERTE(GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_PICKUPITEM);

	AI_SetStateRunning(hook);
	if (AI_StartMove(self) == FALSE)
	{
		return;
	}

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask(pGoalStack);
	_ASSERTE(pCurrentTask);
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	userEntity_t* pItem = pAIData->pEntity;
	_ASSERTE(pItem);

	// compute the time that may take to get there
	float fDistance = 0;
	if (!AI_FindPathToPoint(self, pItem->s.origin))
	{
		/*		CVector currentPos = self->s.origin;

				NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( self );
				PATHNODE_PTR pPath = hook->pPathList->pPath;
				while ( pPath )
				{
					MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, pPath->nNodeIndex );
					fDistance += VectorXYDistance( currentPos, pNode->position );

					currentPos = pNode->position;

					pPath = pPath->next_node;
				}*/
	}
	// NSS[3/6/00]:Setup a quick check to make sure that we still need the item as we approach the item.
	pAIData->nValue = 0;
	if (strstr(pItem->className, "health"))
		pAIData->nValue = HEALTH;
	if (strstr(pItem->className, "ammo"))
		pAIData->nValue = AMMO;
	if (strstr(pItem->className, "armor"))
		pAIData->nValue = ARMOR;

	AI_Dprintf("Starting TASKTYPE_SIDEKICK_PICKUPITEM.\n");

	AI_SetNextThinkTime(self, 0.1f);

	AI_SetOkToAttackFlag(hook, TRUE);
	AI_SetTaskFinishTime(hook, -1);



}






// ----------------------------------------------------------------------------
// NSS[12/16/99]:
// Name:		SideKick_SpecialMoveTowardPoint
// Description: Specific Move function for specific circumstances, one being picking up items
// Input: userEntity_t *self, CVector &dstpoint, int bMovingTowardNode, int bFaceTowardDest
// Output:T/F
// Note:NA
// ----------------------------------------------------------------------------
int SideKick_SpecialMoveTowardPoint(userEntity_t* self, CVector& destPoint,
	int bMovingTowardNode /* = FALSE */, int bFaceTowardDest /* = TRUE */)
{
	_ASSERTE(self);

	playerHook_t* hook = AI_GetPlayerHook(self);
	if (!AI_CanMove(hook))
	{
		AI_RemoveCurrentTask(self);
		return FALSE;
	}

	// we can just check the x,y distance since this function assumes that the 
	// dest point is visible from self
	float fXYDistance = VectorXYDistance(self->s.origin, destPoint);
	float fZDistance = VectorZDistance(self->s.origin, destPoint);
	if (fXYDistance < ((self->s.maxs.x - self->s.mins.x) * 0.50f) && fZDistance < 32.0f)
	{

		AI_UpdateCurrentNode(self);
		return TRUE;
	}

	if (bFaceTowardDest)
	{
		AI_FaceTowardPoint(self, destPoint);
	}

	float fSpeed = AI_ComputeMovingSpeed(hook);

	if (!self->groundEntity)
	{
		if (AI_IsCloseDistance2(self, fXYDistance))
		{
			AI_UpdateCurrentNode(self);
			return TRUE;
		}

		if (self->waterlevel > 0 && self->waterlevel < 3)
		{
			float fHeight = 8.0f + ((float)self->waterlevel * 8.0f);
			if (fZDistance > fHeight)
			{
				// must be stuck trying to go out of the water, try to jump
				self->velocity.z = hook->upward_vel * 0.50f;
			}
			else
			{
				forward = destPoint - self->s.origin;
				forward.Normalize();

				//	scale speed based on current frame's move_scale
				float fScale = FRAMES_ComputeFrameScale(hook);
				AI_SetVelocity(self, forward, (fSpeed * fScale));
			}

			AI_UpdateCurrentNode(self);

			return FALSE;
		}
		else
		{
			AI_HandleOffGround(self, destPoint, fSpeed);
		}

		return FALSE;
	}

	if (self->groundEntity && self->waterlevel == 2)
	{
		if (fZDistance > 24.0f)
		{
			// must be stuck trying to go out of the water, try to jump
			self->velocity.z = hook->upward_vel * 0.25f;
			AI_UpdateCurrentNode(self);
			return FALSE;
		}
	}

	userEntity_t* pAboveEntity = AI_IsEntityAbove(self);
	if (pAboveEntity)
	{
		CVector freePoint;
		if (AI_FindCollisionFreePoint(self, freePoint))
		{
			destPoint = freePoint;
			bMovingTowardNode = FALSE;
		}
	}

	userEntity_t* pBelowEntity = AI_IsEntityBelow(self);
	if (pBelowEntity)
	{
		// since enemy is below, find a point respective of the enemy
		CVector freePoint;
		if (AI_FindCollisionFreePoint(pBelowEntity, freePoint))
		{
			forward = freePoint - self->s.origin;
			forward.Normalize();

			float fDistance = VectorDistance(self->s.origin, freePoint);
			float fZDistance = VectorZDistance(self->s.origin, freePoint);

			// compute needed forward velocity to reach the point
			float fUpwardVelocity = hook->upward_vel * 0.5f;
			float fForwardVelocity = ai_jump_vel(self, fDistance, fZDistance, fUpwardVelocity);

			if (freePoint.z > self->s.origin.z)
			{
				fUpwardVelocity += fZDistance;
			}
			self->velocity = forward * fForwardVelocity;
			self->velocity.z = fUpwardVelocity;

			self->groundEntity = NULL;
			return FALSE;
		}
	}

	if (AI_HandleDirectPath(self, destPoint, bMovingTowardNode))
	{
		AI_UpdateCurrentNode(self);
		return FALSE;
	}

	forward = destPoint - self->s.origin;

	// should never be in water, but just in case
	int bOnFlatSurface = AI_IsOnFlatSurface(self);

	forward.Normalize();

	if (AI_HandleCollisionWithEntities(self, destPoint, fSpeed))
	{
		AI_UpdateCurrentNode(self);
		return FALSE;
	}

	if (AI_HandleGroundObstacle(self, fSpeed, bMovingTowardNode))
	{
		AI_UpdateCurrentNode(self);
		return FALSE;
	}

	float fOneFrameDistance = fSpeed * 0.125f;

	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale(hook);
	AI_SetVelocity(self, forward, (fSpeed * fScale));

	if (AI_HandleGettingStuck(self, destPoint, fSpeed))
	{
		AI_UpdateCurrentNode(self);
		return FALSE;
	}

	hook->last_origin = self->s.origin;

	AI_UpdateCurrentNode(self);

	AI_HandleCrouching(self);

	ai_frame_sounds(self);
	return FALSE;
}


void SIDEKICK_RemoveItem(userEntity_t* Item);



// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// NSS[12/14/99]:
// Name:		SIDEKICK_DetermineNeedAmmo
// Description: Function that will determine if we still need ammo.
// Input: userEntity_t *self, char *Ammo_Name
// Output:TRUE/FALSE
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_DetermineNeedAmmo(userEntity_t* self, char* Ammo_Name)
{
	userInventory_t* inv = gstate->InventoryFirstItem(self->inventory);
	ammo_t* Ammo_Ptr;
	while (inv)
	{
		if (inv->name)
		{
			if (_stricmp(inv->name, Ammo_Name) == 0)
			{
				Ammo_Ptr = (ammo_t*)inv;
				if (Ammo_Ptr->winfo && Ammo_Ptr->winfo->ammo_max)
				{
					if (Ammo_Ptr->winfo->ammo_max > Ammo_Ptr->count)
						return TRUE;
				}
				return FALSE;
			}
		}
		inv = gstate->InventoryNextItem(self->inventory);
	}
	return FALSE;
}
// ----------------------------------------------------------------------------
// NSS[12/14/99]:
// Name:		SIDEKICK_PickupItem
// Description: General Pickup for Sidekicks
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_PickupItem(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask(pGoalStack);
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	userEntity_t* pItem = pAIData->pEntity;
	_ASSERTE(pItem);

	int bItemVisible = ITEM_IsVisible(pItem);
	if (!bItemVisible)
	{
		AI_RemoveCurrentTask(self);
		return;
	}

	float fXYDistance = VectorXYDistance(self->s.origin, pItem->s.origin);
	float fZDistance = VectorZDistance(self->s.origin, pItem->s.origin);
	CVector destPoint;
	destPoint = pItem->s.origin;
	destPoint.z += 16.0f;
	switch (pAIData->nValue)
	{
	case HEALTH:
	{
		// NSS[3/6/00]:We might have run over some health on the way, check to make sure we still need it.
		if (self->health >= hook->base_health)
		{
			AI_RemoveCurrentGoal(self);
			return;
		}
		break;
	}
	case AMMO:
	{
		if (pItem && pItem->className && !SIDEKICK_DetermineNeedAmmo(self, pItem->className))
		{
			AI_RemoveCurrentGoal(self);
			return;
		}
		break;
	}
	case ARMOR:
	{
		// NSS[3/6/00]:Again, we might have run over some armor on our way, if we don't need it give up this task.
		if (self->armor_val >= pItem->health)
		{
			AI_RemoveCurrentGoal(self);
			return;
		}
		break;
	}
	}

	if (AI_IsSideKickCloseToItem(hook, fXYDistance) && fZDistance < 32.0f && AI_IsOkToMoveStraight(self, destPoint, fXYDistance, fZDistance))
	{

		//if(AI_IsOkToMoveStraight( self,pItem->s.origin, fXYDistance, fZDistance ))
		//{
		AI_AddNewTaskAtFront(self, TASKTYPE_MOVETOEXACTLOCATION, pItem);
		//}
		//else
		//{
			// move the dest point a little further away from the item so 
			// that it will move through the item
//			AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
//			if(pAIData->destPoint && pAIData->nValue)
//			{
		//		float fDistance = VectorDistance( self->s.origin, pAIData->destPoint );
				// NSS[2/14/99]:Check for Walls
//				tr = gstate->TraceBox_q2( self->s.origin,self->s.mins,self->s.maxs, pAIData->destPoint, self, MASK_SOLID);
		//		if ( fDistance < 38.0f || (fDistance < ((self->s.maxs.x - self->s.mins.x)) && tr.fraction < 1.0f))
		//		{
		//			AI_RemoveCurrentTask(self,FALSE);
		//			if(!Check_Que(self,PICKUP_ITEM,80.0f))
		//			{
		//				SideKick_TalkAmbient(self, PICKUP_ITEM,1);
		//				pItem->touch(pItem, self, NULL, NULL);
		//				SIDEKICK_SendMessage(self, sCom_Speak,PICKUP_ITEM,gstate->time,0.0f,self,COM_Read);
		//			}
		//			return;
		//		}

		//	}
/*			else
			{
				CVector forwardVector;
				float fSpeed = AI_ComputeMovingSpeed( hook );
				float fOneFrameSpeed = fSpeed * 0.04f;

				forwardVector = pItem->s.origin - self->s.origin;
				forwardVector.Normalize();

				pAIData->destPoint	= (forwardVector* fOneFrameSpeed) + pItem->s.origin;
				pAIData->nValue		= 1;  //NSS[12/2/99]:Marker that we have stored our destination for this task.
			}*/

			//			SideKick_SpecialMoveTowardPoint( self, pAIData->destPoint, FALSE, TRUE );
			//
			///			AI_SetTaskFinishTime( hook, 1.0f );
			//	        return;
			//		}
	}
	else
	{
		int bMoved = FALSE;
		if (hook->pPathList->nPathLength == 0)
		{
			tr = gstate->TraceLine_q2(self->s.origin, pItem->s.origin, self, MASK_SOLID);
			if ((tr.fraction >= 0.65f && fZDistance < MAX_JUMP_HEIGHT) ||
				(tr.fraction >= 0.8f && ((1.2f - tr.fraction) * (fXYDistance + fZDistance)) < 32.0f))
			{
				AI_MoveTowardPoint(self, pItem->s.origin, FALSE);
				bMoved = TRUE;
			}
		}
		if (!bMoved)
		{
			if (hook->pPathList->pPath)
			{
				if (!AI_HandleUse(self))
				{
					if (AI_Move(self) == FALSE)
					{
						if (AI_CanPath(hook) && !AI_FindPathToPoint(self, pItem->s.origin))
						{
							AI_MoveTowardPoint(self, pItem->s.origin, FALSE);
							bMoved = TRUE;
							//AI_RestartCurrentGoal( self );
						}
					}
				}
			}
			else
			{
				if (AI_CanPath(hook) && !AI_FindPathToPoint(self, pItem->s.origin))
				{
					AI_MoveTowardPoint(self, pItem->s.origin, FALSE);
					bMoved = TRUE;
					//AI_RestartCurrentGoal( self );
				}
			}
		}
	}
}






#define SIDEKICK_USE_NOTHING	0
#define SIDEKICK_USE_HOSPORTAL	1
#define SIDEKICK_USE_HEALTHTREE 2


// ----------------------------------------------------------------------------
// NSS[5/19/00]:
// Name:		SIDEKICK_StartUseItem
// Description: Setup everything before we go over to use a hosportal.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartUseItem(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);

	AI_SetStateRunning(hook);
	if (AI_StartMove(self) == FALSE)
	{
		return;
	}

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask(pGoalStack);
	_ASSERTE(pCurrentTask);
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	userEntity_t* pItem = pAIData->pEntity;
	_ASSERTE(pItem);

	// compute the time that may take to get there
	float fDistance = 0;

	AI_FindPathToPoint(self, pItem->s.origin);

	AI_Dprintf("Starting TASKTYPE_SIDEKICK_USEITEM.\n");

	AI_SetNextThinkTime(self, 0.1f);

	AI_SetOkToAttackFlag(hook, TRUE);
	AI_SetTaskFinishTime(hook, -1);


	// NSS[5/24/00]:This here is what sets up our actions through this particular task... and there you have it.
	if (_stricmp(pAIData->pEntity->className, "misc_hosportal") == 0 || _stricmp(pAIData->pEntity->className, "misc_lifewater") == 0)
	{
		pAIData->nValue = SIDEKICK_USE_HOSPORTAL;
	}
	else if (_stricmp(pAIData->pEntity->className, "misc_healthtree") == 0)
	{
		pAIData->nValue = SIDEKICK_USE_HEALTHTREE;
	}
	else
	{
		pAIData->nValue = SIDEKICK_USE_NOTHING;
	}
}


// ----------------------------------------------------------------------------
// NSS[5/19/00]:
// Name:		SIDEKICK_UseItem
// Description: Move to the item and also use it
// Input:userEntity_t *self
// Output:NA
// Note:If you are looking through this code I feel for you... no really.. I do.
//
// ----------------------------------------------------------------------------
void SIDEKICK_UseItem(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask(pGoalStack);
	_ASSERTE(pCurrentTask);
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	userEntity_t* pItem = pAIData->pEntity;
	_ASSERTE(pItem);


	float XYDistance, ZDistance;
	XYDistance = VectorXYDistance(self->s.origin, pItem->s.origin);
	ZDistance = VectorZDistance(self->s.origin, pItem->s.origin);

	// NSS[5/19/00]:The case where we are actually close enough to use the hosportal.
	if (XYDistance < 65.0f && ZDistance <= 32.0f)
	{
		if (pAIData->nValue == SIDEKICK_USE_HOSPORTAL)
		{
			// NSS[3/6/00]:Setup a quick check to make sure that we still need the item as we approach the item.
			hosportalHook_t* ihook = (hosportalHook_t*)pItem->userHook;
			// NSS[5/19/00]:As long as we have enough juice to charge ourself up then lets hang out a bit and keep using it.
			if (ihook->current_juice > 0 && (self->health < (hook->base_health - (hook->base_health * 0.025))))
			{

				if (pItem->use && pAIData->fValue < gstate->time)
				{
					pItem->use(pItem, self, self);
					pAIData->fValue = gstate->time + 2.0f;
					if (pAIData->nValue == 0)
					{
						char szAnimation[16];
						AI_SelectAmbientAnimation(self, szAnimation);
						AI_ForceSequence(self, szAnimation, FRAME_LOOP);
					}
					pAIData->nValue = 1;
				}
				else if (pItem->use && pAIData->fValue > gstate->time)
				{
					//We are just waiting to be charged up to MAX HP.
					//What the hell... just make sure we look like we are using it.
					AI_FaceTowardPoint(self, pItem->s.origin);
				}
				else
				{
					AI_Dprintf("Major problem, the hosportal does not have a use function!!\n");
				}
			}
			else
			{
				AI_RemoveCurrentGoal(self);
			}
		}
		else if (pAIData->nValue == SIDEKICK_USE_HEALTHTREE)
		{
			healthtreeHook_t* ihook = (healthtreeHook_t*)pItem->userHook;
			if (ihook->current_fruit && (self->health < (hook->base_health - (hook->base_health * 0.025))))
			{
				if (gstate->time >= ihook->ignore_timer && pItem->use)
				{
					pItem->use(pItem, self, self);
				}
				else
				{
					AI_FaceTowardPoint(self, pItem->s.origin);
				}
			}
			else
			{
				AI_RemoveCurrentGoal(self);
			}

		}
		else if (pAIData->nValue == SIDEKICK_USE_NOTHING)
		{
			AI_Dprintf("The entity being pointed at has no code to deal with it!\n Removing this goal.");
			AI_RemoveCurrentGoal(self);
		}
	}
	else
	{
		int bMoved = FALSE;
		if (hook->pPathList->nPathLength == 0)
		{
			tr = gstate->TraceLine_q2(self->s.origin, pItem->s.origin, self, MASK_SOLID);
			if ((tr.fraction >= 0.65f && ZDistance < MAX_JUMP_HEIGHT) ||
				(tr.fraction >= 0.8f && ((1.2f - tr.fraction) * (XYDistance + ZDistance)) < 32.0f))
			{
				AI_MoveTowardPoint(self, pItem->s.origin, FALSE);
				bMoved = TRUE;
			}
		}
		if (!bMoved)
		{
			if (hook->pPathList->pPath)
			{
				if (!AI_HandleUse(self))
				{
					if (AI_Move(self) == FALSE)
					{
						if (AI_CanPath(hook) && !AI_FindPathToPoint(self, pItem->s.origin))
						{
							AI_MoveTowardPoint(self, pItem->s.origin, FALSE);
							bMoved = TRUE;
						}
					}
				}
			}
			else
			{
				if (AI_CanPath(hook) && !AI_FindPathToPoint(self, pItem->s.origin))
				{
					AI_MoveTowardPoint(self, pItem->s.origin, FALSE);
					bMoved = TRUE;
				}
			}
		}
	}
}




// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_PermissionDenied
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_PermissionDenied(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	if (hook->pWantItem)
	{
		ITEM_SetNoPickup(self, hook->pWantItem);
		hook->pWantItem = NULL;

		SIDEKICK_CommandAcknowledge(self, SIDEKICK_COMMAND_NO); // respond to given command
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_PermissionGiven
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_PermissionGiven(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	if (hook->pWantItem)
	{
		AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_PICKUPITEM, hook->pWantItem);
		SIDEKICK_CommandAcknowledge(self, SIDEKICK_COMMAND_YES); // respond to given command
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_AskPermissionForItem
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsOkToAskPermissionForItem(userEntity_t* self, userEntity_t* pItem)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	//NSS[11/30/99]:Added Mikikofly here
	if (hook->type == TYPE_SUPERFLY || hook->type == TYPE_MIKIKOFLY)
	{
		if (pItem->nSidekickFlag & SUPERFLY_ASKED_PERMISSION)
		{
			return FALSE;
		}
	}
	else
		if (hook->type == TYPE_MIKIKO)
		{
			if (pItem->nSidekickFlag & MIKIKO_ASKED_PERMISSION)
			{
				return FALSE;
			}
		}

	return TRUE;
}

// ----------------------------------------------------------------------------

void SIDEKICK_SetAskedPermissionForItem(userEntity_t* self, userEntity_t* pItem)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	if (hook->type == TYPE_SUPERFLY || hook->type == TYPE_MIKIKOFLY)
	{
		pItem->nSidekickFlag |= SUPERFLY_ASKED_PERMISSION;
	}
	else
		if (hook->type == TYPE_MIKIKO)
		{
			pItem->nSidekickFlag |= MIKIKO_ASKED_PERMISSION;
		}
}

// ----------------------------------------------------------------------------

void SIDEKICK_AskPermissionForItem(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	_ASSERTE(hook->pWantItem);
	if (SIDEKICK_IsOkToAskPermissionForItem(self, hook->pWantItem))
	{
		AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_PICKUPITEM, hook->pWantItem);
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_HandlePickupItems
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_HandlePickupItems(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);

	if (self->hacks < gstate->time && !SIDEKICK_IsStaying(hook))
	{
		hook->pWantItem = SIDEKICK_DetermineNeededItem(self);
		self->hacks = gstate->time + 1.0f;
		if (!hook->pWantItem)
		{
			hook->pWantItem = SIDEKICK_DetermineWantItem(self);
		}

		if (hook->pWantItem)
		{
			if (AI_FindPathToEntity(self, hook->pWantItem, TRUE) && SIDEKICK_DeterminePathLength(self, hook->pWantItem) < 256.0f)
			{
				SIDEKICK_AskPermissionForItem(self);
				return TRUE;
			}
		}
	}
	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_PickedUpItem
// Description:
//				sidekick a bot picks up a new item, this function is called
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_PickedUpItem(userEntity_t* self, const char* szItemName)
{
	if (_strnicmp(szItemName, "weapon", 6) == 0)
	{
		SIDEKICK_ChooseBestWeapon(self);
	}
	else
		if (_stricmp(szItemName, "item_plasteel_armor") == 0 ||
			_stricmp(szItemName, "item_chromatic_armor") == 0)
		{
		}
		else
			if (_stricmp(szItemName, "item_power_boost") == 0 ||
				_stricmp(szItemName, "item_acro_boost") == 0 ||
				_stricmp(szItemName, "item_attack_boost") == 0 ||
				_stricmp(szItemName, "item_speed_boost") == 0)
			{
			}
			else
				if (_stricmp(szItemName, "item_vita_boost") == 0)
				{
				}
				else
					if (_strnicmp(szItemName, "item_health", 11) == 0)
					{
					}
					else
						if (_stricmp(szItemName, "item_goldensoul") == 0)
						{
						}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_IsOwnerTargetingMe
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsOwnerTargetingMe(userEntity_t* self, userEntity_t* pOwner)
{
	_ASSERTE(AI_IsAlive(pOwner));

	CVector dir = self->s.origin - pOwner->s.origin;
	dir.Normalize();
	CVector angleTowardSelf;
	VectorToAngles(dir, angleTowardSelf);

	float fTowardSelfY = AngleMod(angleTowardSelf.y);
	float fFacingAngleY = AngleMod(pOwner->s.angles.yaw);

	float fYawDiff = fabs(fTowardSelfY - fFacingAngleY);
	if (fYawDiff < 10.0f || fYawDiff > 350.0f)
	{
		float fFacingAngleX = AngleMod(pOwner->s.angles.pitch);
		float fTowardSelfX = AngleMod(angleTowardSelf.x);
		float fPitchDiff = fabs(fTowardSelfX - fFacingAngleX);
		if (fPitchDiff < 30.0f || fPitchDiff > 330.0f)
		{
			CVector facingVector;
			pOwner->s.angles.AngleToForwardVector(facingVector);

			CVector testPoint;
			VectorMA(pOwner->s.origin, facingVector, 1000.0f, testPoint);
			tr = gstate->TraceLine_q2(pOwner->s.origin, testPoint, pOwner, MASK_SOLID);
			if (tr.fraction < 1.0 && tr.ent == self)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
// ----------------------------------------------------------------------------
// NSS[12/12/99]:
// Name:        SIDEKICK_EnemyGoalFilter
// Description:This will determine if the enemy iis in an agressive mode or not.
// Input: userEntity_t *ent
// Output:int (yes/no) 1/0
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_EnemyGoalFilter(userEntity_t* ent)
{
	playerHook_t* Mhook;
	Mhook = AI_GetPlayerHook(ent);
	int Valid = 0;
	// NSS[12/11/99]:Monster has to be in attack mode in order for Sidekicks to really react to them.
	if (Mhook)
	{
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(Mhook);
		if (pGoalStack)
		{
			GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
			if (pCurrentGoal)
			{
				GOALTYPE nGoalType = GOAL_GetType(pCurrentGoal);
				switch (nGoalType)
				{
				case GOALTYPE_KILLENEMY:
				case GOALTYPE_COLUMN_AWAKEN:
				case GOALTYPE_BUBOID_GETOUTOFCOFFIN:
				case GOALTYPE_BUBOID_MELT:
				case GOALTYPE_ROTWORM_DROPFROMCEILING:
				case GOALTYPE_ROTWORM_JUMPATTACK:
				case GOALTYPE_PROTOPOD_HATCH:
				case GOALTYPE_MEDUSA_GAZE:
				case GOALTYPE_SKEETER_HATCH:
				case GOALTYPE_LYCANTHIR_RESURRECT:
				case GOALTYPE_BUBOID_RESURRECT:
				case GOALTYPE_WYNDRAX_RECHARGE:
				case GOALTYPE_WYNDRAX_POWERUP:
				case GOALTYPE_TAKECOVER:
				case GOALTYPE_WANDER:
				case GOALTYPE_CAMBOT_FOLLOWPLAYER:
				{
					Valid = 1;
				}
				}
			}
		}
	}
	return Valid;
}


// ----------------------------------------------------------------------------
// NSS[12/12/99]:
// Name:        SIDEKICK_WhoIsClosest
// Description:Function to determine who should be the 'controller' for doors etc..
// Input:userEntity_t *self,MAPNODE_PTR pCurrentNode
// Output:T/F
// Note:NA
// ----------------------------------------------------------------------------
int SIDEKICK_WhoIsClosest(userEntity_t* self, MAPNODE_PTR pCurrentNode)
{
	float fDistanceMe;
	float fDistanceThem;

	userEntity_t* pOtherSidekick = SIDEKICK_GetOtherSidekick(self);
	if (pOtherSidekick != NULL)
	{
		fDistanceMe = VectorDistance(self->s.origin, pCurrentNode->position);
		fDistanceThem = VectorDistance(pOtherSidekick->s.origin, pCurrentNode->position);
		if (fDistanceMe > fDistanceThem)
			return FALSE;
	}
	return TRUE;
}



// ----------------------------------------------------------------------------
// NSS[12/12/99]:
// Name:        SIDEKICK_AttackOwnerFacingEnemy
// Description:This is a new part of the attack command for the sidekicks.  It will
// trace out from the owner's view and find the enemy the owner is pointing at and 
// attack it.
// Input:userEntity_t *self
// Output:TRUE/FALSE (True-->going to attack/False-->can't find enemy)
// Note:NA
// ----------------------------------------------------------------------------
int SIDEKICK_AttackOwnerFacingEnemy(userEntity_t* self)
{

	playerHook_t* hook = AI_GetPlayerHook(self);
	if (hook)
	{
		if (hook->owner)
		{
			CVector Dir, Destination, Start;
			Start = hook->owner->s.origin;
			Start.z += 22.0f;
			// NSS[2/4/00]:Get 
			hook->owner->client->ps.viewangles.AngleToForwardVector(Dir);
			//hook->owner->s.angles.AngleToForwardVector(Dir);
			Destination = Start + 800 * Dir;
			tr = gstate->TraceLine_q2(Start, Destination, hook->owner, MASK_SHOT);
			// NSS[2/4/00]:if we have hit something considered to be a monster solid and it thinks then kill it!

			SIDEKICK_Drop_Markers((Start + (Dir * (800 * tr.fraction))), 1.0f);
			if (tr.fraction < 1.0f && (tr.ent->flags & FL_MONSTER))
			{
				self->enemy = tr.ent;
				if (SIDEKICK_IsOkToFight(self))
				{
					return TRUE;
				}
				else
				{
					self->enemy = NULL;
				}
			}
		}
	}
	return FALSE;
}

// ----------------------------------------------------------------------------
// NSS[2/17/00]:
// Name:        SIDEKICK_DeterminePathLength
// Description:Adding this for mid-textures that can be seen through.  This will
// check the path length every time and determine how far we have to travel in order
// to get to our destination.
// Input:userEntity_t *self
// Output:userEntity_t *self
// Note:NA
// ----------------------------------------------------------------------------
float SIDEKICK_DeterminePathLength(userEntity_t* self, userEntity_t* target)
{
	// NSS[2/17/00]:Make it some stupid distance
	float distance = -1;
	playerHook_t* hook = AI_GetPlayerHook(self);
	if (hook)
	{
		if (hook->pPathList)
		{
			// update the current node for this entity
			NODELIST_PTR pNodeList = hook->pNodeList;
			if (pNodeList)
			{
				NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
				if (pNodeHeader)
				{
					if (hook->pPathList->pPath && hook->pPathList->pPath->next_node)
					{
						PATHNODE_PTR CurrentPath = hook->pPathList->pPath;
						if (CurrentPath)
						{
							MAPNODE_PTR pCurrentNode = NODE_GetNode(pNodeHeader, CurrentPath->nNodeIndex);
							if (pCurrentNode)
							{
								MAPNODE_PTR pNextNode;

								while (CurrentPath)
								{
									pNextNode = NULL;
									if (CurrentPath->next_node)
									{
										pNextNode = NODE_GetNode(pNodeHeader, CurrentPath->next_node->nNodeIndex);
									}
									if (pNextNode != NULL)
									{
										// NSS[3/1/00]:Added this per John request to determine if our path is blocked by something
										tr = gstate->TraceLine_q2(pCurrentNode->position, pNextNode->position, NULL, MASK_SOLID);
										if (tr.fraction < 1.0f && tr.ent)
										{
											if (strstr(tr.ent->className, "door"))
											{
												if (!(pCurrentNode->node_type & (NODETYPE_DOORBUTTON | NODETYPE_DOOR) || (pNextNode && (pNextNode->node_type & (NODETYPE_DOORBUTTON | NODETYPE_DOOR)))))
												{
													return -1;
												}
											}
											else
											{
												return -1;
											}
										}
										distance += VectorDistance(pCurrentNode->position, pNextNode->position);
									}
									else
									{
										// NSS[3/11/00]:Finally trace to the final target from the end Node to make sure we can reach it.
										tr = gstate->TraceLine_q2(pCurrentNode->position, target->s.origin, NULL, MASK_SOLID);
										if (tr.fraction < 1.0f)
										{
											return -1;
										}
									}
									pCurrentNode = pNextNode;
									CurrentPath = CurrentPath->next_node;
								}
							}
						}
					}
					else if (hook->pPathList->pPath && !hook->pPathList->pPath->next_node)
					{
						MAPNODE_PTR pCurrentNode = NODE_GetNode(pNodeHeader, hook->pPathList->pPath->nNodeIndex);
						distance += VectorDistance(self->s.origin, pCurrentNode->position);
					}
				}
			}
		}
	}
	return distance;
}


// ----------------------------------------------------------------------------
// NSS[12/12/99]:
// Name:        SIDEKICK_FindEnemy_DeepLoop
// Description:Had to add a second layer to do processing after I get the enemy
// Input:userEntity_t *self
// Output:userEntity_t *self
// Note:NA
// ----------------------------------------------------------------------------
userEntity_t* SIDEKICK_FindEnemy_DeepLoop(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* Mhook;
	playerHook_t* hook = AI_GetPlayerHook(self);
	userEntity_t* pOwner = hook->owner;
	userEntity_t* Owner_Enemy, * SideKick_Enemy, * Self_Enemy, * SideKick, * ent;
	float fOwnerDis, fSideDis, fSelfDis, fDistance, wDistance;
	CVector	position;

	fOwnerDis = fSideDis = fSelfDis = 9999;

	Owner_Enemy = SideKick_Enemy = Self_Enemy = SideKick = ent = NULL;

	userEntity_t* pOtherSidekick = SIDEKICK_GetOtherSidekick(self);



	//userEntity_t *pMonster = alist_FirstEntity( monster_list );
	userEntity_t* pMonster = gstate->FirstEntity();

	//for (vm = gstate->FirstEntity (); vm && stricmp (vm->className, "viewthing"); vm = gstate->NextEntity (vm));
	weapon_t* pWeapon = (weapon_t*)self->curWeapon;

	if (pWeapon && pWeapon->winfo)
		wDistance = pWeapon->winfo->range;
	else
		wDistance = 0.0f;

	//Parse through all monsters
	while (pMonster)
	{
		if (pMonster->flags & FL_MONSTER)
		{
			// NSS[12/2/99]:FirstCheck to see if the monster has an enemy.  We are reactional and not action oriented A.I. creatures. Then check to make
			// sure that the AI is in an Offensive mode, then check to make sure the offensive mode is adversely going to effect someone in the party.
			if (pMonster->enemy && AI_IsAlive(pMonster) && SIDEKICK_EnemyGoalFilter(pMonster))
			{
				fDistance = VectorDistance(self->s.origin, pMonster->s.origin);

				if ((SIDEKICK_IsClearShot(self, pMonster, self->s.origin, pMonster->s.origin) || !pWeapon || wDistance < fDistance) && fDistance < 1048.0f)
				{
					// NSS[1/22/00]:Make sure we don't see any really bad monsters... and if we do let's run!
					if (!SIDEKICK_CheckMonsterType(self, pMonster) && (AI_IsVisible(self, pMonster) || AI_IsVisible(pOwner, pMonster)))
					{
						return pMonster;
					}

					// NSS[12/2/99]:Priority #1 Take care of ourself
					if (pMonster->enemy == self)
					{
						if (fDistance < fSelfDis)
						{
							Self_Enemy = pMonster;
							fSelfDis = fDistance;
						}
					}
					else// NSS[12/2/99]:Priority #2 Look to see if either the player or the other sidekick is in trouble
						if (pMonster->enemy->flags & FL_CLIENT)
						{
							if (fDistance < fOwnerDis)
							{
								Owner_Enemy = pMonster;
								fOwnerDis = fDistance;
								pOwner->enemy = pMonster;
							}
						}
						else// NSS[12/2/99]:Part of Priority #2
						{
							Mhook = AI_GetPlayerHook(pMonster->enemy);
							if (Mhook)
							{
								if ((Mhook->type == TYPE_MIKIKO || Mhook->type == TYPE_SUPERFLY) && (Mhook->type != hook->type))
								{
									if (fDistance < fSideDis)
									{
										// NSS[12/2/99]:Get the pointer to the monster and the other sidekick
										SideKick = pMonster->enemy;
										SideKick_Enemy = pMonster;
										fSideDis = fDistance;

									}
								}
							}
						}
				}
			}
		}
		else if (pMonster->flags & FL_EXPLOSIVE)// NSS[1/28/00]:For sunflare, C4, Venomous
		{
			fDistance = VectorDistance(self->s.origin, pMonster->s.origin);
			if (fDistance < 256.0f)
			{
				AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_EVADE, pMonster);
				return pMonster;
			}
		}

		// NSS[12/2/99]:Continue through the list of monsters
		pMonster = gstate->NextEntity(pMonster);
		//pMonster = alist_NextEntity( monster_list );
	}


	// NSS[12/20/99]:Let's worry about ourself first.
	if (Self_Enemy != NULL)
	{
		ent = Self_Enemy;
		fDistance = VectorDistance(self->s.origin, Self_Enemy->s.origin);
		if (!SIDEKICK_IsClearShot(self, ent, self->s.origin, ent->s.origin))
		{
			if (SIDEKICK_FindGoodShootingSpot(self, ent, &position))
			{
				if (Owner_Enemy)
				{
					// NSS[3/11/00]:If it is a monster to run from or 
					if (!SIDEKICK_CheckMonsterType(self, Owner_Enemy) || (fDistance > VectorDistance(self->s.origin, Owner_Enemy->s.origin)))
					{
						return Owner_Enemy;
					}
				}
				if (SIDEKICK_CanPathToOrIsCloseToEnemy(self, Self_Enemy))
				{
					return ent;
				}
			}
			else if (Owner_Enemy)
			{
				if (!SIDEKICK_CheckMonsterType(self, Owner_Enemy))
				{
					return Owner_Enemy;
				}
			}
			// NSS[2/27/00]:Did not find a thing reset ent
			ent = NULL;
		}
		else if (Owner_Enemy == NULL && SideKick_Enemy == NULL)
		{
			return ent;
		}
	}

	// NSS[12/2/99]:Check to see if other sidekick is in trouble
	if (SideKick_Enemy != NULL)
	{
		// NSS[12/2/99]:Let's not be selfish, if we have more than 50 health and our buddy is in trouble let's help out.
		if ((SideKick->health < self->health && self->health > 50.0f) || Self_Enemy == NULL)
		{
			ent = SideKick_Enemy;
			if (!SIDEKICK_IsClearShot(self, ent, self->s.origin, ent->s.origin))
			{
				if (SIDEKICK_FindGoodShootingSpot(self, ent, &position))
				{
					return ent;
				}
			}
			else
			{
				return ent;
			}
		}
	}

	// NSS[12/20/99]: Finally let's help the player out.
	if (Owner_Enemy != NULL)
	{
		ent = Owner_Enemy;
		if (!SIDEKICK_IsClearShot(self, ent, self->s.origin, ent->s.origin))
		{
			if (SIDEKICK_FindGoodShootingSpot(self, ent, &position))
			{
				return ent;
			}
		}
		else
		{
			return ent;
		}
	}

	if (!pWeapon && ent)
	{
		return ent;
	}
	return NULL;
}



// ----------------------------------------------------------------------------
// NSS[12/12/99]:
// Name:        SIDEKICK_FindEnemy
// Description:This is a new and improved version of find enemy for the sidekicks
// Input:userEntity_t *self
// Output:userEntity_t *self
// Note:NA
// ----------------------------------------------------------------------------
userEntity_t* SIDEKICK_FindEnemy(userEntity_t* self)
{
	userEntity_t* enemy;
	//playerHook_t *hook;

	enemy = NULL;
	enemy = SIDEKICK_FindEnemy_DeepLoop(self);

	if (enemy && !(enemy->flags & FL_EXPLOSIVE))
	{
		//hook = AI_GetPlayerHook(enemy);
		//if(hook)
		//{
		if (!SIDEKICK_CanPathToOrIsCloseToEnemy(self, enemy))
		{
			enemy = NULL;
		}
		/*else
		if(enemy->movetype == MOVETYPE_WALK || enemy->movetype == MOVETYPE_HOP)
		{
			float zDistance = VectorZDistance(self->s.origin, enemy->s.origin);
			// NSS[2/3/00]:Set it to make the OkToFight call.
			self->enemy = enemy;
			if(zDistance > 156.0f && !SIDEKICK_IsOkToFight(self))
			{
				enemy		= NULL;
			}
			// NSS[2/3/00]:Reset it... let the exterior calls figure this one out.
			self->enemy = NULL;
		}*/
		//}
	}
	// NSS[3/8/00]:Sidekicks are not affraid of the player's missiles.
	if (enemy && (enemy->flags & FL_EXPLOSIVE) && enemy->movetype == MOVETYPE_FLYMISSILE)
		return NULL;
	else
		return enemy;
}


// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_FindClosestEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define MONSTER_DETECTION_CHANCE    0.1f

userEntity_t* SIDEKICK_FindClosestEnemy(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);
	userEntity_t* pOwner = hook->owner;
	if (!pOwner)
	{
		return NULL;
	}

	float fClosestDistance = 1024.0f;
	userEntity_t* pClosestEnemy = NULL;

	float fClosestDetectedDistance = 4096.0f;
	userEntity_t* pDetectedEnemy = NULL;

	userEntity_t* pMonster = alist_FirstEntity(monster_list);
	while (pMonster)
	{
		if (pMonster->enemy && AI_IsAlive(pMonster) && AI_IsLineOfSight(self, pMonster) &&
			(pMonster->enemy == pOwner || pMonster->enemy == self))
		{
			float fXYDistance = VectorXYDistance(self->s.origin, pMonster->s.origin);
			float fZDistance = VectorZDistance(self->s.origin, pMonster->s.origin);
			if (fXYDistance < fClosestDistance && fZDistance < 256.0f)
			{
				fClosestDistance = fXYDistance;
				pClosestEnemy = pMonster;
			}
			else
			{
				float fDistance = VectorDistance(self->s.origin, pMonster->s.origin);
				if (fDistance < fClosestDetectedDistance && rnd() < MONSTER_DETECTION_CHANCE)
				{
					pDetectedEnemy = pMonster;
					fClosestDetectedDistance = fDistance;
				}
			}
		}

		pMonster = alist_NextEntity(monster_list);
	}

	if (!pClosestEnemy)
	{
		pClosestEnemy = pDetectedEnemy;
	}

	return pClosestEnemy;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_TestRandomWalkPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define RANDOMWALK_DISTANCE			128.0f
#define CLOSER_RANDOMWALK_DISTANCE	96.0f

int SIDEKICK_TestRandomWalkPoint(userEntity_t* self, CVector& mins, CVector& maxs, CVector& vector, CVector& randomWalkPoint)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	userEntity_t* pOwner = hook->owner;
	_ASSERTE(pOwner);

	CVector endPoint;
	VectorMA(self->s.origin, vector, RANDOMWALK_DISTANCE, endPoint);
	float fDistance = VectorDistance(pOwner->s.origin, endPoint);
	if (fDistance < SIDEKICK_GetRandomWalkDistance(hook) &&
		AI_IsGroundBelowBetween(self, self->s.origin, endPoint))
	{
		tr = gstate->TraceBox_q2(self->s.origin, mins, maxs, endPoint, self, MASK_SOLID | CONTENTS_MONSTER);
		if (tr.fraction >= 1.0f)
		{
			VectorMA(self->s.origin, vector, RANDOMWALK_DISTANCE - 24.0f, randomWalkPoint);
			_ASSERTE(randomWalkPoint.Length() > 0.0f);
			return TRUE;
		}
	}
	else
	{
		VectorMA(self->s.origin, vector, CLOSER_RANDOMWALK_DISTANCE, endPoint);
		float fDistance = VectorDistance(pOwner->s.origin, endPoint);
		if (fDistance < SIDEKICK_GetRandomWalkDistance(hook) &&
			AI_IsGroundBelowBetween(self, self->s.origin, endPoint))
		{
			tr = gstate->TraceBox_q2(self->s.origin, mins, maxs, endPoint, self, MASK_SOLID | CONTENTS_MONSTER);
			if (tr.fraction >= 1.0f)
			{
				VectorMA(self->s.origin, vector, CLOSER_RANDOMWALK_DISTANCE - 24.0f, randomWalkPoint);
				_ASSERTE(randomWalkPoint.Length() > 0.0f);
				return TRUE;
			}
		}
	}

	return FALSE;
}

int SIDEKICK_FindRandomWalkPoint(userEntity_t* self, CVector& randomWalkPoint)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	userEntity_t* pOwner = hook->owner;
	_ASSERTE(pOwner);

	CVector mins(self->s.mins.x + 8.0f, self->s.mins.y + 8.0f, self->s.mins.z + 16.0f);
	CVector maxs(self->s.maxs.x - 8.0f, self->s.maxs.y - 8.0f, self->s.maxs.z);

	CVector vector = self->s.origin - pOwner->s.origin;
	vector.z = 0.0f;
	vector.Normalize();

	CVector angleTowardOwner;
	VectorToAngles(vector, angleTowardOwner);
	angleTowardOwner.pitch = 1.0f;

	int bFacingOwner = FALSE;
	CVector angle = self->s.angles;
	float fDistance = VectorDistance(self->s.origin, pOwner->s.origin);
	if (fDistance < RANDOMWALK_DISTANCE)
	{
		float fYAngleDiff = AngleMod(self->s.angles.yaw - angleTowardOwner.yaw);
		if (fYAngleDiff < 45.0f || fYAngleDiff > 315.0f)
		{
			bFacingOwner = TRUE;
			angle = angleTowardOwner;
		}
	}

	float fYAngle = angle.yaw;
	int nRandom = rand() % 5;
	if (nRandom < 4)
	{
		fYAngle = angle.yaw;
		if (nRandom == 0)
		{
			fYAngle = 45.0f;
		}
		else
			if (nRandom == 1)
			{
				fYAngle = 90.0f;
			}
			else
				if (nRandom == 2)
				{
					fYAngle = 315.0f;
				}
				else
					if (nRandom == 3)
					{
						fYAngle = 270.0f;
					}
	}
	else
	{
		nRandom = rand() % 8;
		fYAngle = (float)nRandom * 45.0f;

		if (fYAngle == 0.0f && bFacingOwner == TRUE)
		{
			if (rnd() > 0.5f)
			{
				fYAngle = 45.0f;
			}
			else
			{
				fYAngle = 315.0f;
			}
		}
	}

	float fRunFollowDistance = AI_GetRunFollowDistance(hook);
	if (fDistance >= fRunFollowDistance - 64.0f && pOwner->deadflag == DEAD_NO)
	{
		float fNewYAngle = AngleMod(angle.yaw + fYAngle);
		float fYAngleDiff = AngleMod(fNewYAngle - angleTowardOwner.yaw);
		if (fYAngleDiff > 150.0f && fYAngleDiff < 210.0f)
		{
			if (rnd() > 0.5f)
			{
				fYAngle += 135.0f;
			}
			else
			{
				fYAngle += 225.0f;
			}
		}
	}

	angle.yaw = AngleMod(angle.yaw + fYAngle);
	angle.AngleToForwardVector(vector);

	return SIDEKICK_TestRandomWalkPoint(self, mins, maxs, vector, randomWalkPoint);
}


int SIDEKICK_FindRandomWalkPointUsingNodes(userEntity_t* self, CVector& randomWalkPoint)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	userEntity_t* pOwner = hook->owner;
	_ASSERTE(pOwner);

	// update the current node for this entity
	NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE(pNodeList);
	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE(pNodeHeader);

	MAPNODE_PTR pCurrentNode = NODE_GetClosestNode(self);
	if (!pCurrentNode)
	{
		if (pNodeList->nCurrentNodeIndex != -1)
		{
			pCurrentNode = NODE_GetNode(pNodeHeader, pNodeList->nCurrentNodeIndex);
		}
	}

	int bFound = FALSE;
	if (pCurrentNode)
	{
		// find a random node that is within a certain distance from the starting position
		int nNumLinks = 0;
		int aNodeIndices[MAX_NODE_LINKS];
		for (int i = 0; i < pCurrentNode->nNumLinks; i++)
		{
			MAPNODE_PTR pNode = NODE_GetNode(pNodeHeader, pCurrentNode->aLinks[i].nIndex);

			float fDistance = VectorDistance(pOwner->s.origin, pNode->position);
			if (fDistance < SIDEKICK_GetRandomWalkDistance(hook))
			{
				aNodeIndices[nNumLinks] = pCurrentNode->aLinks[i].nIndex;
				nNumLinks++;
			}
		}

		MAPNODE_PTR pDestNode = NULL;
		if (nNumLinks > 0)
		{
			srand((unsigned)time(NULL));
			int nTargetNodeIndex = aNodeIndices[(rand() % nNumLinks)];

			pDestNode = NODE_GetNode(pNodeHeader, nTargetNodeIndex);
			_ASSERTE(pDestNode);

			randomWalkPoint = pDestNode->position;

			bFound = TRUE;
		}
	}

	return bFound;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_ComputeRandomWalkPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_ComputeRandomWalkPoint(userEntity_t* self, CVector& randomWalkPoint)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	userEntity_t* pOwner = hook->owner;
	_ASSERTE(pOwner);

	if (!SIDEKICK_FindRandomWalkPoint(self, randomWalkPoint))
	{
		return FALSE;
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StartRandomWalk
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartRandomWalk(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	_ASSERTE(GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_RANDOMWALK);

	CVector position;
	if (SIDEKICK_ComputeRandomWalkPoint(self, position) == FALSE)
	{
		AI_RemoveCurrentTask(self);
		return;
	}
	_ASSERTE(position.Length() > 0.0f);

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask(pGoalStack);
	_ASSERTE(pCurrentTask);
	TASK_Set(pCurrentTask, position);

	AI_SetStateSlowWalking(hook);
	if (AI_StartMove(self) == FALSE)
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask(pGoalStack);
		if (TASK_GetType(pCurrentTask) != TASKTYPE_SIDEKICK_RANDOMWALK)
		{
			return;
		}
	}

	AI_Dprintf("Starting TASKTYPE_SIDEKICK_RANDOMWALK.\n");

	AI_SetOkToAttackFlag(hook, TRUE);

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE(pAIData);

	_ASSERTE(pAIData->destPoint.Length() > 0.0f);

	float fDistance = VectorDistance(self->s.origin, pAIData->destPoint);
	float fSpeed = AI_ComputeMovingSpeed(hook);
	float fTime = (fDistance / fSpeed) + 1.0f;
	AI_SetTaskFinishTime(hook, fTime);

	AI_SetMovingCounter(hook, 0);

	SIDEKICK_RandomWalk(self);
}

void SIDEKICK_RandomWalk(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask(pGoalStack);
	_ASSERTE(pCurrentTask);
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE(pAIData);

	float fXYDistance = VectorXYDistance(self->s.origin, pAIData->destPoint);
	float fZDistance = VectorZDistance(self->s.origin, pAIData->destPoint);
	if (AI_IsCloseDistance2(self, fXYDistance) && fZDistance < 32.0f)
	{
		AI_RemoveCurrentTask(self);
		SIDEKICK_SetNextAmbientTime(self, TASKTYPE_SIDEKICK_RANDOMWALK);
		return;
	}

	AI_MoveTowardPoint(self, pAIData->destPoint, TRUE, TRUE);

}



// ----------------------------------------------------------------------------
// NSS[12/20/99]:
// Name:		SIDEKICK_StartAnimation
// Description:Generic StartAnimation Function for Sidekicks, will select the right
// animation based off of the current weapon being held.
// Input:userEntity_t *self, char *Buff (starting letters of desired animation)
// Output:0 = failed, 1 = Success
// Note:
// ----------------------------------------------------------------------------
int SIDEKICK_StartAnimation(userEntity_t* self, char* Buff)
{

	char szAnimation[16];
	playerHook_t* hook = AI_GetPlayerHook(self);

	if (AI_IsEndAnimation(self) || self->s.frame == 0)
	{
		// NSS[12/20/99]:Trap for crouching.
		if (AI_IsCrouching(hook))
		{
			//			sprintf(szAnimation,"cwalk");
			Com_sprintf(szAnimation, sizeof(szAnimation), "cwalk");
		}
		else
		{
			if (self->waterlevel > 2)
			{
				strcpy(szAnimation, "swim");
			}
			else
			{
				strcpy(szAnimation, Buff);
			}
		}

		AI_SelectAnimationPerWeaponType(self, szAnimation);
		frameData_t* pSequence = FRAMES_GetSequence(self, szAnimation);
		if (!pSequence)
		{
			AI_RemoveCurrentTask(self);
			return 0;
		}

		AI_ForceSequence(self, pSequence, FRAME_LOOP);
	}
	return 1;
}


// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StartLookAround
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartLookAround(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	_ASSERTE(GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_LOOKAROUND);

	char szAnimation[16];
	strcpy(szAnimation, "lookaround");
	AI_SelectAnimationPerWeaponType(self, szAnimation);
	frameData_t* pSequence = FRAMES_GetSequence(self, szAnimation);
	if (!pSequence)
	{
		AI_RemoveCurrentTask(self);
		return;
	}

	if (AI_StartSequence(self, pSequence, FRAME_ONCE) == FALSE)
	{
		return;
	}

	AI_Dprintf("Starting TASKTYPE_SIDEKICK_LOOKAROUND.\n");

	AI_SetOkToAttackFlag(hook, TRUE);

	float fAnimationTime = AI_ComputeAnimationTime(pSequence);
	AI_SetTaskFinishTime(hook, fAnimationTime);

	AI_SetNextThinkTime(self, 0.1f);

}

void SIDEKICK_LookAround(userEntity_t* self)
{
	_ASSERTE(self);

	if (AI_IsEndAnimation(self))
	{
		AI_RemoveCurrentTask(self);
		SIDEKICK_SetNextAmbientTime(self, TASKTYPE_SIDEKICK_LOOKAROUND);
		return;
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StartLookUp
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartLookUp(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	_ASSERTE(GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_LOOKUP);

	char szAnimation[16];
	strcpy(szAnimation, "lookup");
	AI_SelectAnimationPerWeaponType(self, szAnimation);
	frameData_t* pSequence = FRAMES_GetSequence(self, szAnimation);
	if (!pSequence)
	{
		AI_RemoveCurrentTask(self);
		return;
	}

	if (AI_StartSequence(self, pSequence, FRAME_ONCE) == FALSE)
	{
		return;
	}

	AI_Dprintf("Starting TASKTYPE_SIDEKICK_LOOKUP.\n");

	AI_SetOkToAttackFlag(hook, TRUE);

	float fAnimationTime = AI_ComputeAnimationTime(pSequence);
	AI_SetTaskFinishTime(hook, fAnimationTime);

	AI_SetNextThinkTime(self, 0.1f);
}

void SIDEKICK_LookUp(userEntity_t* self)
{
	_ASSERTE(self);

	if (AI_IsEndAnimation(self))
	{
		AI_RemoveCurrentTask(self);
		SIDEKICK_SetNextAmbientTime(self, TASKTYPE_SIDEKICK_LOOKUP);
		return;
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StartLookDown
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartLookDown(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	_ASSERTE(GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_LOOKDOWN);

	char szAnimation[16];
	strcpy(szAnimation, "lookdown");
	AI_SelectAnimationPerWeaponType(self, szAnimation);
	frameData_t* pSequence = FRAMES_GetSequence(self, szAnimation);
	if (!pSequence)
	{
		AI_RemoveCurrentTask(self);
		return;
	}

	if (AI_StartSequence(self, pSequence, FRAME_ONCE) == FALSE)
	{
		return;
	}

	AI_Dprintf("Starting TASKTYPE_SIDEKICK_LOOKDOWN.\n");

	AI_SetOkToAttackFlag(hook, TRUE);

	float fAnimationTime = AI_ComputeAnimationTime(pSequence);
	AI_SetTaskFinishTime(hook, fAnimationTime);

	AI_SetNextThinkTime(self, 0.1f);
}

void SIDEKICK_LookDown(userEntity_t* self)
{
	_ASSERTE(self);

	if (AI_IsEndAnimation(self))
	{
		AI_RemoveCurrentTask(self);
		SIDEKICK_SetNextAmbientTime(self, TASKTYPE_SIDEKICK_LOOKDOWN);
		return;
	}
}

// ----------------------------------------------------------------------------
// NSS[12/12/99]:
// Name:        SIDEKICK_StartSaySomething
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartSaySomething(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);
	float distance;

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	_ASSERTE(GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_SAYSOMETHING);

	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
	_ASSERTE(pCurrentGoal);
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask(pGoalStack);
	_ASSERTE(pCurrentTask);
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	if (pAIData->pString)
	{
		if (strstr(pAIData->pString, "mp3"))
		{
			// NSS[12/12/99]:Added a cheesy attenuation hack for MP3 stuff.
			distance = 1.0f;
			if (hook)
			{
				userEntity_t* owner = hook->owner;
				if (owner)
				{
					float fDistance = VectorDistance(self->s.origin, owner->s.origin);
					if (fDistance > 512.0f)
					{
						if (fDistance > 1500.0f)
						{
							distance = 0.10f;
						}
						else
						{
							distance = (1500 - fDistance) / 1000.0f;
						}
						// NSS[12/9/99]:Limit the min.
						if (distance < 0.10f)
							distance = 0.10f;
					}
				}
			}
			gstate->StartMP3(pAIData->pString, 2, distance, 1, TRUE, NULL);
		}
		else
			if (strstr(pAIData->pString, "wav"))
			{
				gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex(pAIData->pString),
					1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
			}
	}

	AI_Dprintf("Starting TASKTYPE_SIDEKICK_SAYSOMETHING.\n");

	AI_SetNextThinkTime(self, 0.1f);

	AI_RemoveCurrentTask(self);
}



// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_SaySomething
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SaySomething(userEntity_t* self)
{
	_ASSERTE(self);

}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StartKickSomething
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartKickSomething(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	_ASSERTE(GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_KICKSOMETHING);

	char szAnimation[16];
	strcpy(szAnimation, "aataka");
	AI_SelectAnimationPerWeaponType(self, szAnimation);
	frameData_t* pSequence = FRAMES_GetSequence(self, szAnimation);
	if (!pSequence)
	{
		AI_RemoveCurrentTask(self);
		return;
	}

	if (AI_StartSequence(self, pSequence, FRAME_ONCE) == FALSE)
	{
		return;
	}

	AI_Dprintf("Starting TASKTYPE_SIDEKICK_KICKSOMETHING.\n");

	AI_SetOkToAttackFlag(hook, TRUE);

	float fAnimationTime = AI_ComputeAnimationTime(pSequence);
	AI_SetTaskFinishTime(hook, fAnimationTime);

	AI_SetNextThinkTime(self, 0.1f);
}

void SIDEKICK_KickSomething(userEntity_t* self)
{
	_ASSERTE(self);

	if (AI_IsEndAnimation(self))
	{
		AI_RemoveCurrentTask(self);
		SIDEKICK_SetNextAmbientTime(self, TASKTYPE_SIDEKICK_KICKSOMETHING);
		return;
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StartWhistle
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartWhistle(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	_ASSERTE(GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_WHISTLE);

	char szAnimation[16];
	strcpy(szAnimation, "aamb");
	AI_SelectAnimationPerWeaponType(self, szAnimation);
	frameData_t* pSequence = FRAMES_GetSequence(self, szAnimation);
	if (!pSequence)
	{
		AI_RemoveCurrentTask(self);
		return;
	}

	if (AI_StartSequence(self, pSequence, FRAME_ONCE) == FALSE)
	{
		return;
	}

	AI_Dprintf("Starting TASKTYPE_SIDEKICK_WHISTLE.\n");

	AI_SetOkToAttackFlag(hook, TRUE);

	float fAnimationTime = AI_ComputeAnimationTime(pSequence);
	AI_SetTaskFinishTime(hook, fAnimationTime);

	AI_SetNextThinkTime(self, 0.1f);
}

void SIDEKICK_Whistle(userEntity_t* self)
{
	_ASSERTE(self);

	if (AI_IsEndAnimation(self))
	{
		AI_RemoveCurrentTask(self);
		SIDEKICK_SetNextAmbientTime(self, TASKTYPE_SIDEKICK_WHISTLE);
		return;
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StartAnimate
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartAnimate(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	_ASSERTE(GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_ANIMATE);

	char szAnimation[16];
	strcpy(szAnimation, "aamb");
	AI_SelectAnimationPerWeaponType(self, szAnimation);
	frameData_t* pSequence = FRAMES_GetSequence(self, szAnimation);
	if (!pSequence)
	{
		AI_RemoveCurrentTask(self);
		return;
	}

	if (AI_StartSequence(self, pSequence, FRAME_ONCE) == FALSE)
	{
		return;
	}

	AI_Dprintf("Starting TASKTYPE_SIDEKICK_ANIMATE.\n");

	AI_SetOkToAttackFlag(hook, TRUE);

	float fAnimationTime = AI_ComputeAnimationTime(pSequence);
	AI_SetTaskFinishTime(hook, fAnimationTime);

	AI_SetNextThinkTime(self, 0.1f);
}

void SIDEKICK_Animate(userEntity_t* self)
{
	_ASSERTE(self);

	if (AI_IsEndAnimation(self))
	{
		AI_RemoveCurrentTask(self);
		SIDEKICK_SetNextAmbientTime(self, TASKTYPE_SIDEKICK_ANIMATE);
		return;
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StartStopGoingFurther
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartStopGoingFurther(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	_ASSERTE(pGoalStack);
	_ASSERTE(GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_STOPGOINGFURTHER);

	char szAnimation[16];
	AI_SelectAmbientAnimation(self, szAnimation);
	if (AI_StartSequence(self, szAnimation, FRAME_LOOP) == FALSE)
	{
		return;
	}

	AI_Dprintf("Starting TASKTYPE_SIDEKICK_STOPGOINGFURTHER.\n");

	AI_ZeroVelocity(self);

	AI_SetNextThinkTime(self, 0.1f);

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask(pGoalStack);
	_ASSERTE(pCurrentTask);
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	AI_SetOkToAttackFlag(hook, FALSE);
	AI_SetTaskFinishTime(hook, -1.0f);
	AI_SetMovingCounter(hook, 0);
}

void SIDEKICK_StopGoingFurther(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);

	frameData_t* pCurrentSequence = hook->cur_sequence;
	if (!strstr(pCurrentSequence->animation_name, "amb"))
	{
		char szAnimation[16];
		AI_SelectAmbientAnimation(self, szAnimation);
		AI_ForceSequence(self, szAnimation, FRAME_LOOP);
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StartTeleportAndComeNear
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartTeleportAndComeNear(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	AI_SetOkToAttackFlag(hook, TRUE);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	_ASSERTE(pGoalStack);
	_ASSERTE(GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_TELEPORTANDCOMENEAR);

	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
	_ASSERTE(pCurrentGoal);
	GOALTYPE nGoalType = GOAL_GetType(pCurrentGoal);
	if (nGoalType == GOALTYPE_SIDEKICK_TELEPORTANDCOMENEAR)
	{
		GOAL_Satisfied(pCurrentGoal);
		AI_RemoveCurrentGoal(self);
	}

}

void SIDEKICK_TeleportAndComeNear(userEntity_t* self)
{
}
int AI_StartMoveForWalkingUnit(userEntity_t* self);

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StartComeHere
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartComeHere(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	_ASSERTE(GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_COMEHERE);

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask(pGoalStack);
	_ASSERTE(pCurrentTask);
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	float fDistance = AI_ComputeDistanceToPoint(self, pAIData->destPoint);
	float fRunFollowDistance = AI_GetRunFollowDistance(hook);
	float fWalkFollowDistance = AI_GetWalkFollowDistance(hook);

	//if ( fDistance > fRunFollowDistance && !AI_IsCrouching(hook))
	//{
	//    AI_SetStateRunning( hook );
	//}
	//else
	//{
	AI_SetStateWalking(hook);
	//}

	if (AI_StartMove(self) == FALSE)
	{
		if (TASK_GetType(pCurrentTask) != TASKTYPE_SIDEKICK_COMEHERE)
		{
			return;
		}
	}


	AI_Dprintf("Starting TASKTYPE_SIDEKICK_COMEHERE.\n");

	AI_SetNextThinkTime(self, 0.1f);

	AI_SetOkToAttackFlag(hook, TRUE);
	float fSpeed = AI_ComputeMovingSpeed(hook);
	float fTime = (fDistance / fSpeed) + 2.0f;
	AI_SetTaskFinishTime(hook, fTime);
	AI_SetMovingCounter(hook, 0);

}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_ComeHere
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_ComeHere(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask(pGoalStack);
	_ASSERTE(pCurrentTask);
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	if (hook && hook->owner && AI_IsAlive(hook->owner))
		pAIData->destPoint = hook->owner->s.origin;

	float fXYDistance = VectorXYDistance(self->s.origin, pAIData->destPoint);
	float fZDistance = VectorZDistance(self->s.origin, pAIData->destPoint);
	if ((AI_IsCloseDistance2(self, fXYDistance) || fXYDistance < 32.0f) && fZDistance <= 32.0f)
	{
		AI_RemoveCurrentTask(self);
		return;
	}

	if (AI_IsEndAnimation(self))
	{
		float fDistance = AI_ComputeDistanceToPoint(self, pAIData->destPoint);
		float fRunFollowDistance = AI_GetRunFollowDistance(hook);
		float fWalkFollowDistance = AI_GetWalkFollowDistance(hook);

		if (fDistance > fRunFollowDistance && !AI_IsCrouching(hook))
		{
			AI_SetStateRunning(hook);
		}
		else
		{
			AI_SetStateWalking(hook);
		}

		AI_StartMoveForWalkingUnit(self);
	}

	if (AI_IsOkToMoveStraight(self, pAIData->destPoint, fXYDistance, fZDistance))
	{
		AI_MoveTowardPoint(self, pAIData->destPoint, FALSE);
	}
	else
	{
		if (!AI_HandleUse(self))
		{
			if (hook->pPathList->pPath)
			{
				if (AI_Move(self) == FALSE)
				{
					if (!AI_FindPathToPoint(self, pAIData->destPoint))
					{
						AI_RestartCurrentGoal(self);
					}
				}
			}
			else
			{
				if (!AI_FindPathToPoint(self, pAIData->destPoint))
				{
					AI_RestartCurrentGoal(self);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
// NSS[1/27/00]:
// Name:		SIDEKICK_FindClearSpotFrom
// Description: Finds a clear point from the point fromPoint... er.. yeah.
// Input:userEntity_t *self,CVector &fromPoint, CVector &clearPoint
// Output:TRUE/FALSE
// Note:
// ----------------------------------------------------------------------------
#define CLEAR_POINT_DISTANCE    64.0f

int SIDEKICK_FindClearSpotFrom(userEntity_t* self, CVector& fromPoint, CVector& clearPoint)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	userEntity_t* owner = hook->owner;
	// search the eight directions around the self to find a spawn point
	float delta_angle = 0.0f;
	float distance = 128.0f;
	CVector angle = self->s.angles;
	angle.pitch = -5.0f;


	CVector vector, Dir, bottomPoint, MidPoint;
	angle.AngleToForwardVector(vector);
	CVector endPoint;

	tr = gstate->TraceBox_q2(fromPoint, self->s.mins, self->s.maxs, fromPoint, self, self->clipmask);

	if (tr.fraction >= 1.0f && !tr.allsolid && !tr.startsolid)
	{
		clearPoint = fromPoint;
		//If we made it then cool.
		clearPoint.z += 6.0f;
		return TRUE;
	}


	while (delta_angle < 360)
	{
		while (distance > 32.0f)
		{
			VectorMA(fromPoint, vector, distance, endPoint);
			endPoint.z += 2.0f;
			//if ( AI_IsGroundBelowBetween( self, fromPoint, endPoint,32.0 ) ||  (owner && owner->waterlevel > 2))
			//{
			endPoint.z += 6.0f;
			bottomPoint = endPoint;
			//Get a new midway point
			Dir = endPoint - fromPoint;
			Dir.Normalize();
			MidPoint = fromPoint + (Dir * (distance * 0.75f));
			//trace from mid to new destination
			tr = gstate->TraceBox_q2(MidPoint, self->s.mins, self->s.maxs, endPoint, self, self->clipmask);

			if (tr.fraction >= 1.0f && !tr.allsolid && !tr.startsolid)
			{
				//If we made it then cool.
				clearPoint = endPoint;
				return TRUE;
			}
			if (tr.ent)
			{
				AI_Dprintf("Collision With:%s", tr.ent->className);
			}
			//}
			//slowly decrement the distance
			distance -= 8.0f;
		}
		delta_angle += 45.0f;
		angle.yaw = AngleMod(angle.yaw + delta_angle);
		angle.AngleToForwardVector(vector);

		//reset the distance for the next angle
		distance = 128.0f;
	}

	return FALSE;
}


// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_StartTeleport
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartTeleport(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	AI_SetOkToAttackFlag(hook, TRUE);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	_ASSERTE(GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_TELEPORT);

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask(pGoalStack);
	_ASSERTE(pCurrentTask);
	_ASSERTE(TASK_GetType(pCurrentTask) == TASKTYPE_SIDEKICK_TELEPORT);

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	CVector clearPoint;
	SIDEKICK_FindClearSpotFrom(self, pAIData->destPoint, clearPoint);

	self->s.origin = clearPoint;

	gstate->LinkEntity(self);

	AI_RemoveCurrentTask(self);
}

void SIDEKICK_Teleport(userEntity_t* self)
{
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_HandlePlayerSelectingSidekick
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_HandlePlayerSelectingSidekick(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);

	if ((hook->nTargetCounter % 2) == 0)
	{
		// if there are two sidekicks, make the one that the player is pointing
		// at more noticeable than the other one, 
		// this is to differentiate which one the player is communicating with
		int bFullBright = FALSE;
		userEntity_t* pOwner = hook->owner;
		if (AI_IsAlive(pOwner))
		{
			float fDistance = VectorDistance(self->s.origin, pOwner->s.origin);
			if (fDistance < 512.0f && SIDEKICK_IsOwnerTargetingMe(self, pOwner))
			{
				AIINFO_SetLastSelectedSidekick(self);
				bFullBright = TRUE;
			}
		}

		if (bFullBright)
		{
			self->s.renderfx |= RF_FULLBRIGHT;
		}
		else
		{
			self->s.renderfx &= ~RF_FULLBRIGHT;
		}
	}

}

// ----------------------------------------------------------------------------
// NSS[1/31/00]:
// Name:        PlaySidekickMP3
// Description: New Mp3 Playing routine to deal with the ultimate Cheese hack
// Input:userEntity_t *self, int Index
// Output:NA
// Note:See the defined structure information in sidekick.h and also the defines 
// at the top of this file.  
// ----------------------------------------------------------------------------
void PlaySidekickMP3(userEntity_t* self, char* sound)
{
	int Chan;
	float distance = 1.0f;
	userEntity_t* Sidekick;

	if (sound != NULL)
	{
		if (strstr(sound, "superfly") || strstr(sound, "mikikofly"))
		{
			Chan = 4;
			Sidekick = AIINFO_GetSuperfly();
			// NSS[3/12/00]:Keep a marker on the last time they have spoken
			SuperflyLastTalk = gstate->time + 10.0f;
		}
		else
		{
			Chan = 3;
			Sidekick = AIINFO_GetMikiko();
			// NSS[3/12/00]:Keep a marker on the last time they have spoken
			MikikoLastTalk = gstate->time + 10.0f;
		}
	}
	else
	{
		return;
	}

	playerHook_t* hook = AI_GetPlayerHook(Sidekick);
	// NSS[2/17/00]:Testy testy
	if (!AI_IsSidekick(hook))
		return;
	// NSS[12/9/99]:Get the distance from the owner to give the effect of 'distance' in the volume since mp3's don't play sounds based off of their origin.
	if (hook != NULL)
	{
		userEntity_t* owner = hook->owner;
		if (owner != NULL)
		{
			// NSS[3/10/00]:If the owner is not alive then we want to here the sounds
			if (AI_IsAlive(owner))
			{
				float fDistance = VectorDistance(Sidekick->s.origin, owner->s.origin);
				if (fDistance > 164.0f)
				{
					if (fDistance >= 800.0f)
					{
						if (!gstate->inPHS(Sidekick->s.origin, hook->owner->s.origin))
							return;
						else
							distance = 0.10f;
					}
					else
					{
						// NSS[2/12/00]:If we are far away and cannot see owner then we cannot hear the owner
						if (fDistance > 300 && !gstate->inPHS(Sidekick->s.origin, hook->owner->s.origin))
							return;
						else
							distance = (1000 - fDistance) / 1000.0f;
					}


					// NSS[12/9/99]:Limit the min.
					if (distance < 0.10f)
						distance = 0.10f;
				}
			}
		}
		else
		{
			return;
		}
	}

	if (!gstate->bCinematicPlaying)
	{
		gstate->StopMP3(Chan, NULL);
		gstate->StartMP3(sound, Chan, distance, 1, TRUE, NULL);
	}
}




// ----------------------------------------------------------------------------
// NSS[1/31/00]:
// Name:        PlaySidekickVoice
// Description: Had to rewrite this again... this is a litle more modular now.
// Input:userEntity_t *self, int Index
// Output:NA
// Note:See the defined structure information in sidekick.h and also the defines 
// at the top of this file.  
// ----------------------------------------------------------------------------
void PlaySidekickVoice(userEntity_t* self, int Index, int VoiceType)
{
	char Buffer[32];
	char Sound[128];
	int Chan;
	int MP3 = 0;
	float distance = 1.0f;
	SideMp3_t VoiceIndex;
	playerHook_t* hook = AI_GetPlayerHook(self);

	// NSS[12/9/99]:Get the distance from the owner to give the effect of 'distance' in the volume since mp3's don't play sounds based off of their origin.
	if (hook)
	{
		userEntity_t* owner = hook->owner;
		if (owner)
		{
			float fDistance = VectorDistance(self->s.origin, owner->s.origin);
			if (fDistance > 164.0f)
			{
				if (fDistance >= 800.0f)
				{
					if (!gstate->inPHS(self->s.origin, hook->owner->s.origin))
						return;
					else
						distance = 0.30f;
				}
				else
				{
					// NSS[2/12/00]:If we are far away and cannot see owner then we cannot hear the owner
					if (fDistance > 300 && !gstate->inPHS(self->s.origin, hook->owner->s.origin))
						return;
					else
						distance = (1000 - fDistance) / 1000.0f;
				}


				// NSS[12/9/99]:Limit the min.
				if (distance < 0.30f)
					distance = 0.30f;

			}
		}
	}


	if (hook->type == TYPE_MIKIKO)
	{
		//NSS[12/4/99]:Setup the index reference
		VoiceIndex = MikikoVoices[Index];
		// NSS[1/31/00]:Added Wav capability
		if (strstr(VoiceIndex.File, ".wav"))
			Com_sprintf(Buffer, sizeof(Buffer), "mikiko/");
		else
		{
			MP3 = 1;
			Com_sprintf(Buffer, sizeof(Buffer), "sounds/voices/mikiko/");
		}
		Chan = 2;
		// NSS[3/12/00]:Keep a marker on the last time they have spoken
		MikikoLastTalk = gstate->time + VoiceIndex.delay;
	}
	else
	{
		//NSS[12/4/99]:Setup the index reference
		VoiceIndex = SuperFlyVoices[Index];
		// NSS[1/31/00]:Added Wav capability
		if (strstr(VoiceIndex.File, ".wav"))
			Com_sprintf(Buffer, sizeof(Buffer), "superfly/");
		else
		{
			MP3 = 1;
			Com_sprintf(Buffer, sizeof(Buffer), "sounds/voices/superfly/");
		}
		Chan = 3;
		// NSS[3/12/00]:Keep a marker on the last time they have spoken
		SuperflyLastTalk = gstate->time + VoiceIndex.delay;
	}

	//NSS[12/4/99]:Combine the two
	Com_sprintf(Sound, sizeof(Sound), "%s%s", Buffer, VoiceIndex.File);

	if (Sound[0] != NULL && !gstate->bCinematicPlaying)		//NSS[12/3/99]:If we have a sound and a cinematic is not playing then go for it.
	{
		if (MP3)
		{
			gstate->StopMP3(Chan, NULL);
			gstate->StartMP3(Sound, Chan, distance, 1, TRUE, NULL);
		}
		else
		{
			gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex(Sound), 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
		}
		VoiceIndex.last_time_used = gstate->time;			// NSS[12/4/99]:Keep track of the last time we played this sound.
	}
}

// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        SideKick_TalkAmbient
// Description: Uhh.. a section of ambient stuff to say... no real method to 
// this madness..
// Input:userEntity_t *self, int Tasktype
// Output:Returns the Index played.
// Note:Parse through either Mikiko or Superfly's ambient Voice sounds and 
// and determine which one will be best to play.
// OverRideTime specifies that they are answering a question or need to say something
// ----------------------------------------------------------------------------
static SideMp3_t List[10];					//NSS[12/4/99]:Create 5 slots to possibly be able to fill
static int Indexes[10];
int SideKick_TalkAmbient(userEntity_t* self, enum Sidekick_Talk VoiceType, int OverRide_Time)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	SideMp3_t* VoiceIndex;
	int i = 0;
	int index_found = 0;				//NSS[12/4/99]:How many we have found of this type
	int Best_Index = 0;				//NSS[12/4/99]:Best index found.
	float Last_Time = 0;				//NSS[12/4/99]:Keep track of time... 

	memset(&List, 0, sizeof(List));
	memset(&Indexes, 0, sizeof(Indexes));

	if (hook->type == TYPE_MIKIKO)		//NSS[12/4/99]:Get the pointer to the Voices
	{
		VoiceIndex = MikikoVoices;
	}
	else
	{
		VoiceIndex = SuperFlyVoices;
	}

	//NSS[12/4/99]:Run through the voices until we find the one that matches the Voice Type
	while (VoiceIndex[i].SoundID != 0)
	{
		if (VoiceIndex[i].SoundID == VoiceType)
		{
			if (index_found < 10)
			{
				List[index_found] = VoiceIndex[i];
				Indexes[index_found] = i;
				index_found++;
			}
			else
			{
				break;//Theoretically we should never get here unless someone jacks with the lists
			}
		}
		i++;
	}

	//NSS[12/4/99]:We have accumulated the Array of possible things to say now process it.
	switch (VoiceType)
	{
		// NSS[2/2/00]:Randomly choose one depending on how many there are.
		/*case EVADE:
		case STARTING_ATTACK:
		case NO:
		case CANTREACH:
		case IMPATIENT:
		case JUMP:
		case LANDING:
		{
			int Number = rnd()*index_found;
			Best_Index = Indexes[Number];
			break;
		}*/
	case HEALTH_RESPONSE1:
	case HEALTH_RESPONSE2:
	case HEALTH_RESPONSE3:
	case HEALTH_RESPONSE4:
	case HEALTH_RESPONSE5:
	{
		Best_Index = Indexes[index_found - 1];
		break;
	}
	default:
	{
		int Number = rnd() * index_found;
		Best_Index = Indexes[Number];
		break;
	}
	/*default:
	{
		for(i = 0; i < index_found; i++)
		{
			if(List[i].last_time_used >= Last_Time)
			{
				Best_Index = Indexes[i];						// NSS[12/4/99]:
				Last_Time = List[Best_Index].last_time_used;	// NSS[12/4/99]:
			}
		}
	}*/
	}

	// NSS[12/4/99]:Now play it.. nice and simple.
	PlaySidekickVoice(self, Best_Index, VoiceType);




	return Best_Index;
}

// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        SIDEKICK_SendMessage
// Description:This allows the sidekicks to talk to one another
// Input:	userEntity_t *other, int command, int An_Int, float A_Float
// Output:
// Note:
//Other --> person getting the message.
// ----------------------------------------------------------------------------
int SIDEKICK_SendMessage(userEntity_t* other, int command, int An_Int, float A_Float, float B_Float, userEntity_t* sender, int Flag)
{
	int i;
	float time;
	SideKickCom_t* Kick_Com = NULL;
	SideKickCom_t* ParseCom = NULL;
	playerHook_t* hook = AI_GetPlayerHook(other);

	if (hook == NULL)
	{
		return 0;
	}

	if (hook->type == TYPE_MIKIKO)
	{
		ParseCom = MikikoCom;
	}
	else
	{
		ParseCom = SuperflyCom;
	}

	// NSS[12/4/99]:Get our point of reference
	time = gstate->time;

	// NSS[3/12/00]:First check to see if we have this message in the Queue...
	for (i = 0; i < MAX_SIDEKICK_MESSAGES;i++)
	{
		if (ParseCom[i].aInt == An_Int && ParseCom[i].aFloat > gstate->time)
		{
			return 0;
		}
		else if (ParseCom[i].aInt == An_Int && ParseCom[i].aFloat < gstate->time)
		{
			Kick_Com = &ParseCom[i];
			break;
		}
	}

	// NSS[3/12/00]:Next if we have made it this far, find an open spot.
	if (Kick_Com == NULL)
	{
		for (i = 0; i < MAX_SIDEKICK_MESSAGES;i++)
		{
			if (ParseCom[i].aFloat < gstate->time)
			{
				Kick_Com = &ParseCom[i];
				break;
			}
		}
	}


	if (!Kick_Com)
	{
		return 0;
	}
	else
	{
		Kick_Com->bFloat = B_Float;
		Kick_Com->aFloat = A_Float;
		Kick_Com->aInt = An_Int;
		Kick_Com->command = command;
		Kick_Com->Location = sender->s.origin;
		Kick_Com->flags = Flag;
		Kick_Com->TimeStamp = gstate->time;
	}
	return 1;
}



// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        Check_Que
// Description:This will check the message Que and determine if a message has been sent.
// in too short a period of time ago... 
// Input:userEntity_t *Other,int Type, float Offset
// Output:Int (Number of occurances found
// Note:
// If the offset has a value it will find messages of the same type and determine
// if it has been long enough to play it again.
// ----------------------------------------------------------------------------
int Check_Que(userEntity_t* other, int Type, float Offset)
{
	int i;
	int Number = 0;
	SideKickCom_t* Kick_Com;
	SideKickCom_t* Kick_Com_It;
	playerHook_t* hook = AI_GetPlayerHook(other);
	if (!hook)
		return 1;
	if (hook->type == TYPE_MIKIKO)
	{
		Kick_Com = MikikoCom;
		// NSS[3/12/00]:If superfly has just said something let's not talk...
		if (SuperflyLastTalk > gstate->time && Type != NO_TALK && Type != JUMP && Type != LANDING)
			return 1;
	}
	else
	{
		Kick_Com = SuperflyCom;
		if (MikikoLastTalk > gstate->time && Type != NO_TALK && Type != JUMP && Type != LANDING)
			return 1;
	}

	for (i = 0; i < MAX_SIDEKICK_MESSAGES; i++)
	{
		Kick_Com_It = &Kick_Com[i];
		if (Type != SPECIAL_TALK)
		{
			if (Offset)
			{
				// NSS[1/31/00]:Check to see if a message has been in the Queue for longer than Offset clicks.
				if (Kick_Com_It->aInt == Type && ((Kick_Com_It->aFloat + Offset) > gstate->time))
				{
					Number++;
				}
			}
			else
			{
				// NSS[12/5/99]:Generic Check
				if (Kick_Com_It->aInt == Type)
				{
					Number++;
				}
			}
			// NSS[1/31/00]:Generally, if we have something in here that is fairly new let's not speak at all.
			if (Kick_Com_It->TimeStamp <= gstate->time)
			{
				if ((gstate->time - Kick_Com_It->TimeStamp) < 4.0f)
				{
					Number++;
				}
			}
		}
		else// NSS[1/31/00]:Handle Sidekick's special talking
		{
			if (Kick_Com_It->aFloat + Offset > gstate->time)
			{
				Number++;
			}
		}
	}

	return Number;
}


// ----------------------------------------------------------------------------
// NSS[12/16/99]:
// Name:        SIDEKICK_FaceSomethingReal
// Description: Turns to face something that is something other than a wall
// Input: userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_FaceSomethingReal(userEntity_t* self)
{
	CVector Dir, Angles;

	float Roll_Max = 15.0f;
	float Pitch_Max = 20.0f;

	playerHook_t* hook = AI_GetPlayerHook(self);

	AngleToVectors(self->s.angles, Dir);
	Dir.Normalize();

	Dir = Dir * 128.0f;
	tr = gstate->TraceLine_q2(self->s.origin, Dir, self, MASK_SOLID);

	if (!SIDEKICK_HasTaskInQue(self, TASKTYPE_SIDEKICK_ATTACK))
	{
		//This means we are facing a wall...or some other very un-interesting cock let's not do this.
		if (tr.fraction < 1.0f && !SIDEKICK_HasTaskInQue(self, TASKTYPE_SIDEKICK_ATTACK))
		{
			// NSS[12/16/99]:If we have an owner let's face him.
			if (hook->owner)
			{
				Dir = hook->owner->s.origin - self->s.origin;
				Dir.Normalize();
				VectorToAngles(Dir, Angles);

				if (Angles.roll > Roll_Max || Angles.roll < (-1 * Roll_Max))
				{
					if (Angles.roll > 0)
						Angles.roll = Roll_Max;
					else
						Angles.roll = (-1 * Roll_Max);
				}
				if (Angles.pitch > Pitch_Max || Angles.pitch > (-1 * Pitch_Max))
				{
					if (Angles.pitch > 0)
						Angles.pitch = Pitch_Max;
					else
						Angles.pitch = (-1 * Pitch_Max);
				}

				self->s.angles.yaw = Angles.yaw;

			}
			else
			{
				self->s.angles.yaw += 15.0f;
			}
		}
	}
}

// ----------------------------------------------------------------------------
// NSS[12/6/99]:
// Name:        SIDEKICK_HandleAmbientTasks
// Description: Handles ambient tasks... kahduh... 
// Input: userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_HandleAmbientTasks(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
	userEntity_t* pOtherSidekick = SIDEKICK_GetOtherSidekick(self);
	int bDoneSidekickThink = FALSE;

	if (pCurrentGoal)
	{
		TASK_PTR pCurrentTask = GOAL_GetCurrentTask(pCurrentGoal);
		if (pCurrentTask)
		{
			CVector lastPosition = self->s.origin;
			TASKTYPE nCurrentTaskType = TASK_GetType(pCurrentTask);
			if (SIDEKICK_IsAmbientTask(nCurrentTaskType) &&
				SIDEKICK_HandleFollowing(self) == FALSE)
			{
				bDoneSidekickThink = TRUE;
				switch (nCurrentTaskType)
				{
				case TASKTYPE_SIDEKICK_RANDOMWALK:
				{
					SIDEKICK_RandomWalk(self);
					AI_SetNextThinkTime(self, 0.1f);

					if (!Check_Que(self, RANDOM_WALK, 15.0f) && (rnd() < 0.35f) && hook->owner && AI_IsAlive(hook->owner))
					{
						if (SideKick_TalkAmbient(self, RANDOM_WALK, 1))
						{
							// NSS[12/6/99]:We send a message to both sidekicks so we are not saying the same shit at the same time.
							SIDEKICK_SendMessage(self, sCom_Speak, RANDOM_WALK, gstate->time + 5.0f, 0.0f, self, COM_Read);
							if (pOtherSidekick)
							{
								SIDEKICK_SendMessage(pOtherSidekick, sCom_Speak, RANDOM_WALK, gstate->time + 10.0f, 0.0f, self, COM_Read);
							}
						}
					}
					else
					{
						SIDEKICK_SendMessage(self, sCom_Speak, RANDOM_WALK, gstate->time + 15.0f, 0.0f, self, COM_Read);
					}
					break;
				}
				case TASKTYPE_SIDEKICK_LOOKAROUND:
				{
					if (!Check_Que(self, LOOKING_SIDEWAYS, 30.0f) && (rnd() < 0.15f) && hook->owner && AI_IsAlive(hook->owner))
					{
						SIDEKICK_FaceSomethingReal(self);
						self->velocity.Zero();
						if (rnd() < 0.25f)
						{
							if (SideKick_TalkAmbient(self, LOOKING_SIDEWAYS, 1))
							{
								// NSS[12/6/99]:We send a message to both sidekicks so we are not saying the same shit at the same time.
								SIDEKICK_SendMessage(self, sCom_Speak, LOOKING_SIDEWAYS, gstate->time + 30.0f, 0.0f, self, COM_Read);
								if (pOtherSidekick)
								{
									SIDEKICK_SendMessage(pOtherSidekick, sCom_Speak, LOOKING_SIDEWAYS, gstate->time + 30.0f, 0.0f, self, COM_Read);
								}
							}
						}
						else
						{
							SIDEKICK_SendMessage(self, sCom_Speak, LOOKING_SIDEWAYS, gstate->time + 30.0f, 0.0f, self, COM_Read);
						}
					}
					SIDEKICK_LookAround(self);
					AI_SetNextThinkTime(self, 0.1f);
					break;
				}
				case TASKTYPE_SIDEKICK_LOOKUP:
				{
					SIDEKICK_FaceSomethingReal(self);
					self->velocity.Zero();
					if (!Check_Que(self, LOOKING_UP, 30.0f) && (rnd() < 0.15f) && hook->owner && AI_IsAlive(hook->owner))
					{
						if (SideKick_TalkAmbient(self, LOOKING_UP, 0))
						{
							// NSS[12/6/99]:We send a message to both sidekicks so we are not saying the same shit at the same time.
							SIDEKICK_SendMessage(self, sCom_Speak, LOOKING_UP, gstate->time + 30.0f, 0.0f, self, COM_Read);
							if (pOtherSidekick)
							{
								SIDEKICK_SendMessage(pOtherSidekick, sCom_Speak, LOOKING_UP, gstate->time + 30.0f, 0.0f, self, COM_Read);
							}
						}
					}
					SIDEKICK_LookUp(self);
					AI_SetNextThinkTime(self, 0.1f);
					break;
				}
				case TASKTYPE_SIDEKICK_LOOKDOWN:
				{
					SIDEKICK_FaceSomethingReal(self);
					self->velocity.Zero();
					SIDEKICK_LookDown(self);
					AI_SetNextThinkTime(self, 0.1f);
					break;
				}
				case TASKTYPE_SIDEKICK_SAYSOMETHING:
				{

					SIDEKICK_SaySomething(self);
					self->velocity.Zero();
					AI_SetNextThinkTime(self, 0.1f);
					break;
				}
				case TASKTYPE_SIDEKICK_KICKSOMETHING:
				{
					SIDEKICK_FaceSomethingReal(self);
					self->velocity.Zero();
					SIDEKICK_KickSomething(self);
					AI_SetNextThinkTime(self, 0.1f);
					break;
				}
				case TASKTYPE_SIDEKICK_WHISTLE:
				{
					SIDEKICK_Whistle(self);
					self->velocity.Zero();
					AI_SetNextThinkTime(self, 0.1f);
					break;
				}
				case TASKTYPE_SIDEKICK_ANIMATE:
				{
					SIDEKICK_FaceSomethingReal(self);
					self->velocity.Zero();
					SIDEKICK_Animate(self);
					AI_SetNextThinkTime(self, 0.1f);
					break;
				}
				default:
				{
					_ASSERTE(FALSE);
					break;
				}
				}

				AI_HandleTaskFinishTime(self, nCurrentTaskType, lastPosition);
			}
			else
			{

			}
		}
	}

	return bDoneSidekickThink;
}


// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        SIDEKICK_CheckMessage
// Description:Does what it sounds like... checks the messages for sidekick type self.
// Input:	userEntity_t *self
// Output:	NA
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_CheckMessage(userEntity_t* self)
{
	enum Sidekick_Talk Message_Type;
	SideKickCom_t* Kick_Com;
	SideKickCom_t* Kick_Com_It;
	SideMp3_t* Voices;
	playerHook_t* hook = AI_GetPlayerHook(self);

	int i, Index;
	float Delay = 0.0f;
	Index = -1;

	if (hook->type == TYPE_MIKIKO)
	{
		Kick_Com = MikikoCom;
		Voices = MikikoVoices;
	}
	else
	{
		Kick_Com = SuperflyCom;
		Voices = SuperFlyVoices;
	}
	Message_Type = NO_TALK;
	for (i = 0;i < MAX_SIDEKICK_MESSAGES;i++)
	{
		Kick_Com_It = &Kick_Com[i];
		// NSS[12/4/99]:Unread message.
		if (Kick_Com_It->flags == COM_Unread)
		{
			switch (Kick_Com_It->command)
			{
			case sCom_Speak:
			{
				if (Kick_Com_It->aFloat < gstate->time)
				{
					switch (Kick_Com_It->aInt)
					{
					case QUESTION_HEALTH:
					{
						if (self->health >= 80)
							Message_Type = HEALTH_RESPONSE1;
						else if (self->health < 80 && self->health >= 60)
							Message_Type = HEALTH_RESPONSE2;
						else if (self->health < 60 && self->health >= 40)
							Message_Type = HEALTH_RESPONSE3;
						else if (self->health < 40 && self->health >= 20)
							Message_Type = HEALTH_RESPONSE4;
						else
							Message_Type = HEALTH_RESPONSE5;
						break;
					}
					case QUESTION_1:
					{
						Message_Type = ANSWER_1;
						break;
					}
					case QUESTION_2:
					{
						Message_Type = ANSWER_2;
						break;
					}
					// NSS[12/5/99]:Sync Commands go below this point
					case COM_WAIT:
					{
						AI_AddNewTaskAtFront(self, TASKTYPE_WAIT, Kick_Com_It->aFloat);
						break;
					}
					default:
					{
						Message_Type = (Sidekick_Talk)Kick_Com_It->aInt;
					}
					}
					// NSS[12/5/99]:If we have a message to say then do so.
					if (Message_Type)
					{
						Index = SideKick_TalkAmbient(self, Message_Type, 1);
					}
					// NSS[12/5/99]:Mark the message as being read.
					Kick_Com_It->flags = COM_Read;
					// NSS[3/12/00]: Break out of this loop so we don't continue to say things.
					break;
				}
			}
			}
		}

		// NSS[3/12/00]:If we played something let's push back anything else waiting to be played.		
		if (Index != -1)
		{
			Delay = Voices[Index].delay;
			// NSS[3/12/00]:Now delay everything else based off of the delay of the message just sent.
			for (i = 0;i < MAX_SIDEKICK_MESSAGES;i++)
			{
				Kick_Com_It = &Kick_Com[i];
				// NSS[12/4/99]:Unread message.
				if (Kick_Com_It->flags == COM_Unread)
				{
					if (Kick_Com_It->aFloat <= (gstate->time + Delay))
					{
						// NSS[3/12/00]:Add the time the MP3 will take to play plus 2 seconds for safety purposes.
						Kick_Com_It->aFloat = gstate->time + Delay + 2.0f;
					}
				}
			}
		}

		if (Kick_Com_It->flags == COM_Read)
		{
			// NSS[12/5/99]:Message timed out and should be removed
			if ((gstate->time - Kick_Com_It->aFloat) > 240.0f)
			{
				Kick_Com_It->bFloat = 0.0f;
				Kick_Com_It->aFloat = 0.0f;
				Kick_Com_It->aInt = 0;
				Kick_Com_It->command = 0;
				Kick_Com_It->Location = NULL;
				Kick_Com_It->flags = 0;
				Kick_Com_It->TimeStamp = 0.0f;
			}

		}
	}
}






//void SideKick_TalkAmbient(userEntity_t *self, int VoiceType);
// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        SIDEKICK_ChitChat
// Description: The main segment for conversation between Mikiko and SuperFly
// Input: userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_ChitChat(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	userEntity_t* pOtherSidekick = SIDEKICK_GetOtherSidekick(self);

	if (self && AI_IsVisible(self, hook->owner))
	{
		if (self->hacks * 0.25 < gstate->time) //We have this look at the time thing a tad bit earlier.  We might not have to have this
		{
			if (pOtherSidekick)
			{
				// NSS[12/4/99]:First one to check is the Health question. Added in that they have to be close to one another
				if (pOtherSidekick->health < 100)
					if (!Check_Que(pOtherSidekick, QUESTION_HEALTH, 1000.0f))
						if (VectorDistance(self->s.origin, pOtherSidekick->s.origin) < 175.0f)
							if (!Check_Que(self, LOW_HEALTH, 60.0f))
								if (!Check_Que(self, QUESTION_HEALTH, 70.0f))
									if (rnd() < 0.25f)
									{
										if (SIDEKICK_SendMessage(pOtherSidekick, sCom_Speak, QUESTION_HEALTH, (gstate->time + 2.0f), pOtherSidekick->health, self, COM_Unread))
										{
											SideKick_TalkAmbient(self, QUESTION_HEALTH, 1);
											SIDEKICK_SendMessage(self, sCom_Speak, QUESTION_HEALTH, gstate->time + 180.0f, 0.0f, self, COM_Read);
										}
										return;
									}
									else
									{
										SIDEKICK_SendMessage(self, sCom_Speak, QUESTION_HEALTH, gstate->time + 180, 0.0f, self, COM_Read);
									}

				// NSS[12/5/99]:Really far away conversation doesn't take place too often.
				float RandomNumber = rnd();
				if (hook->owner)
				{
					if (VectorDistance(self->s.origin, hook->owner->s.origin) > 1000.0f && VectorDistance(self->s.origin, pOtherSidekick->s.origin) < 200.0f && AI_IsVisible(self, hook->owner))
					{
						if (!(Check_Que(pOtherSidekick, QUESTION_1, 512.0f)) && !(Check_Que(self, QUESTION_1, 512.0f) && (RandomNumber < 0.03f)))
						{
							if (SIDEKICK_SendMessage(pOtherSidekick, sCom_Speak, QUESTION_1, (gstate->time + 4.0f), 500.0f, self, COM_Unread))
							{
								SideKick_TalkAmbient(self, QUESTION_1, 1);
								SIDEKICK_SendMessage(self, sCom_Speak, QUESTION_1, gstate->time + 512.0f, 0.0f, self, COM_Read);
								return;
							}
						}
						else if (!Check_Que(self, QUESTION_1, 512))
						{
							SIDEKICK_SendMessage(self, sCom_Speak, QUESTION_1, gstate->time + 512.0f, 500.0f, self, COM_Read);
						}

						if (!(Check_Que(pOtherSidekick, QUESTION_2, 512)) && !(Check_Que(pOtherSidekick, QUESTION_1, 512)) && !(Check_Que(self, QUESTION_1, 512)) && hook->type == TYPE_MIKIKO && (RandomNumber < 0.03f))
						{
							if (SIDEKICK_SendMessage(pOtherSidekick, sCom_Speak, QUESTION_2, (gstate->time + 4.0), 500.0f, self, COM_Unread))
							{
								SideKick_TalkAmbient(self, QUESTION_2, 1);
								SIDEKICK_SendMessage(self, sCom_Speak, QUESTION_2, gstate->time + 512.0f, 500.0f, self, COM_Read);
								return;
							}
						}
						else if (hook->type == TYPE_MIKIKO && !Check_Que(self, QUESTION_2, 512.0f))
						{
							SIDEKICK_SendMessage(self, sCom_Speak, QUESTION_2, gstate->time + 512, 0.0f, self, COM_Read);
						}
					}
				}
			}
			// NSS[12/5/99]:Sidekicks talking to Hiro about health goes here.
			if (hook->owner)
			{
				// NSS[12/5/99]:Health related conversation goes here...
				if (hook->owner->health < 100 && (rnd() < 0.10f))
				{

				}
			}
		}
	}
}


// ----------------------------------------------------------------------------
// NSS[12/9/99]:
// Name:        SIDEKICK_HandleDeadOwner
// Description:Handle when the owner dies... 
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_HandleDeadOwner(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);

	if (hook)
	{
		if (hook->owner)
		{
			userEntity_t* other = SIDEKICK_GetOtherSidekick(self);
			if (!AI_IsAlive(hook->owner) && AI_IsSuperfly(hook))
			{
				if (other && !AI_IsAlive(other))
					return 0;
				if (!Check_Que(self, HIRO_DIES, 0.0f))
				{
					SideKick_TalkAmbient(self, HIRO_DIES, 1);
					SIDEKICK_SendMessage(self, sCom_Speak, HIRO_DIES, gstate->time + 10000.0f, 0.0f, self, COM_Read);
				}
			}
		}
	}
	return 0;
}


// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        SIDEKICK_StartStop
// Description: Heh... always wanted to create a function named this. 
// Input:	userEntity_t *self
// Output:  na
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_StartStop(userEntity_t* self)
{
	char szAnimation[16];
	playerHook_t* hook = AI_GetPlayerHook(self);
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask(pGoalStack);
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);


	AI_SelectAmbientAnimation(self, szAnimation);
	frameData_t* pSequence = FRAMES_GetSequence(self, szAnimation);
	if (hook)
	{
		if (pSequence != NULL && hook->cur_sequence && hook->cur_sequence->animation_name && !strstr(hook->cur_sequence->animation_name, "amb"))
		{
			AI_ForceSequence(self, szAnimation, pSequence->flags);
		}
		else if (hook->cur_sequence && hook->cur_sequence->animation_name && strstr(hook->cur_sequence->animation_name, "amb") && AI_IsEndAnimation(self))
		{
			AI_ForceSequence(self, szAnimation, pSequence->flags);
		}
	}
	// NSS[12/13/99]:Hang out for a few seconds;
	// NSS[2/25/00]:If we pass in a time to wait...then wait that time frame otherwise it is a quick wait.
	if (pAIData->fValue && ((pAIData->fValue + gstate->time) < (gstate->time + 30.0f)))
	{
		pAIData->fValue += gstate->time;
	}
	else
	{
		pAIData->fValue = gstate->time + 0.25f;
	}
	pAIData->nValue = 0;

	// NSS[12/13/99]:Let's STOP
	self->velocity.Zero();

	AI_SetTaskFinishTime(hook, -1);

}


// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        SIDEKICK_Stop
// Description: Just stop ok?  
// Input:	userEntity_t *self
// Output:  na
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_Stop(userEntity_t* self)
{
	char szAnimation[16];
	float fDistance;
	float oDistance;
	playerHook_t* mHook;// NSS[1/23/00]:hook for our enemy
	playerHook_t* hook = AI_GetPlayerHook(self);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask(pGoalStack);
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	self->enemy = SIDEKICK_FindEnemy(self);
	// NSS[12/13/99]:Let's STOP
	self->velocity.Zero();


	if (self->enemy)
	{
		// NSS[1/17/00]:Handle getting out of the Stop function while attacking.
		if (SIDEKICK_IsOkToFight(self))
		{
			oDistance = VectorDistance(self->s.origin, hook->owner->s.origin);
			if (AI_IsVisible(self, self->enemy) || oDistance > fNormalDistanceFromOwner)
			{
				AI_RemoveCurrentTask(self, FALSE);
				return;
			}
		}
		else
		{
			mHook = AI_GetPlayerHook(self->enemy);
			fDistance = VectorDistance(self->s.origin, self->enemy->s.origin);
			if (fDistance < mHook->active_distance && AI_IsVisible(self, self->enemy) && self->enemy->enemy == self)
			{
				AI_RemoveCurrentTask(self, FALSE);
				return;
			}
		}
		AI_FaceTowardPoint(self, self->enemy->s.origin);
	}

	if (AI_IsEndAnimation(self))
	{
		AI_SelectAmbientAnimation(self, szAnimation);
		frameData_t* pSequence = FRAMES_GetSequence(self, szAnimation);
		if (pSequence != NULL)
		{
			AI_ForceSequence(self, szAnimation, pSequence->flags);
		}
	}

	// NSS[12/13/99]:End of our second.. .let's re-evaluate the situation.
	if (gstate->time > pAIData->fValue)
	{
		if (self->enemy && pAIData->nValue < 10 && !SIDEKICK_IsOkToFight(self))
		{
			fDistance = VectorDistance(self->s.origin, self->enemy->s.origin);
			if (fDistance > 500.0f && AI_IsVisible(self, self->enemy))// NSS[12/13/99]:Keep hangin'
			{
				pAIData->fValue = gstate->time + 0.25f;
				pAIData->nValue++;
				return;
			}
		}
		AI_RemoveCurrentTask(self);
		return;
	}
}


// ----------------------------------------------------------------------------
// NSS[1/19/00]:
// Name:        SIDEKICK_HasPickedUpItem
// Description: Determine if the Sidekick has picked up an item only if it has
// TASKTYPE_SIDEKICK_PICKUPITEM in its Que.
// Input:	userEntity_t *self
// Output:  TRUE/FALSE
// Note:
// ----------------------------------------------------------------------------
int SIDEKICK_HasPickedUpItem(userEntity_t* self, int UseIt)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	userEntity_t* owner = hook->owner;

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);

	TASK_PTR Task;

	// NSS[1/16/00]:Get the next task to be executed
	if (pCurrentGoal->pTasks->pStartList)
	{
		Task = pCurrentGoal->pTasks->pStartList;
		while (Task)
		{
			if (Task->nTaskType == TASKTYPE_SIDEKICK_PICKUPITEM)
			{
				AIDATA_PTR pAIData = TASK_GetData(Task);
				userEntity_t* pItem = pAIData->pEntity;
				// NSS[1/29/00]:For objects like health and such... 
				// NSS[1/29/00]:This is a hack because the sidekicks were standing on health(exact location) and not triggering it... don't have the time to figure out why the trigger shit is fucked.
				if (UseIt && pItem && pItem->touch)
				{
					tr = gstate->TraceLineTrigger(self->s.origin, self->s.origin, self, MASK_ALL, SVF_ITEM);
					if (tr.ent && tr.ent->flags & FL_ITEM)
						pItem->touch(pItem, self, NULL, NULL);
				}
				else if (!pItem || !ITEM_IsVisible(pItem))
				{
					AI_RemoveCurrentTask(self);
					return TRUE;
				}
				return FALSE;
			}
			else
			{
				if (Task->pNext)
				{
					Task = Task->pNext;
				}
				else
				{
					Task = NULL;
					return FALSE;
				}
			}
		}
	}

	return FALSE;
}



// ----------------------------------------------------------------------------
// NSS[1/19/00]:
// Name:        SIDEKICK_HasTaskInQue
// Description: Determine if the Sidekick has 'Task' in its Que
// Input:	userEntity_t *self
// Output:  TRUE/FALSE
// Note:
// ----------------------------------------------------------------------------
int SIDEKICK_HasTaskInQue(userEntity_t* self, TASKTYPE SearchTask)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	userEntity_t* owner = hook->owner;

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	if (!pGoalStack)
		return FALSE;
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
	if (!pCurrentGoal || !pCurrentGoal->pTasks)
		return FALSE;


	TASK_PTR Task;

	// SCG[2/10/00]: Early out if we can.  Also prevents from accessing NULL pointers
	if (
		(pGoalStack->nNumGoals == 0) ||
		(pCurrentGoal == NULL) ||
		(pCurrentGoal->pTasks == NULL))
	{
		return FALSE;
	}

	// NSS[1/16/00]:Get the next task to be executed
	if (pCurrentGoal->pTasks->pStartList)
	{
		Task = pCurrentGoal->pTasks->pStartList;
		while (Task)
		{
			if (Task->nTaskType == SearchTask)
			{
				return TRUE;
			}
			else
			{
				if (Task->pNext)
				{
					Task = Task->pNext;
				}
				else
				{
					Task = NULL;
					return FALSE;
				}
			}
		}
	}

	return FALSE;
}



// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        SIDEKICK_Process_Environment
// Description: Scan for unfriendlies and deal with them
// Input:	userEntity_t *self
// Output:  na
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_Process_Environment(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	userEntity_t* owner = hook->owner;

	float fDistance = 0.0f;

	if (owner)
	{
		fDistance = VectorXYDistance(self->s.origin, owner->s.origin);
	}

	// NSS[12/11/99]: find an enemy that is close to the owner every so often
	userEntity_t* pEnemy = NULL;
	hook->nTargetCounter++;

	// NSS[1/20/00]:
	SIDEKICK_ChooseBestWeapon(self);

	// NSS[1/13/00]:Find the most threatening enemy.
	self->enemy = SIDEKICK_FindEnemy(self);

	if (self->enemy)
	{
		// NSS[1/16/00]:Make sure we have the right kind of weapon
		SIDEKICK_ChooseBestWeapon(self);

		// NSS[12/15/99]:Make sure we are 'ok' to fight.
		if (!SIDEKICK_IsStaying(hook) && !hook->bCarryingMikiko && !SIDEKICK_HasTaskInQue(self, TASKTYPE_SIDEKICK_ATTACK) && SIDEKICK_IsOkToFight(self))
		{
			AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_ATTACK);
		}
		else if ((!SIDEKICK_IsStaying(hook) && !SIDEKICK_IsOkToFight(self) && !SIDEKICK_HasTaskInQue(self, TASKTYPE_SIDEKICK_EVADE)) && AI_IsVisible(self, self->enemy))
		{
			AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_EVADE, self->enemy);
		}
	}

	// NSS[12/20/99]:If we don't have an enemy targeting us or a 'friendly' then let's see if we need to process any
	// dead-owner related stuff.
	if (!self->enemy)
	{
		// NSS[12/9/99]:
		SIDEKICK_HandleDeadOwner(self);
	}

}

// ----------------------------------------------------------------------------
// NSS[1/3/00]:
// Name:		SideKick_IsVisible
// Description:Test whether a target is within fov and a line of sight
// Input:userEntity_t *self, userEntity_t *targ
// Output:TRUE/FALSE
// Note:
//
// ----------------------------------------------------------------------------
int SideKick_IsEnemyVisible(userEntity_t* self, userEntity_t* targ)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	if (!AI_IsAlive(targ) || !AI_IsAlive(self))
	{
		return FALSE;
	}

	CVector	org, end;

	org.Add(self->s.origin, self->view_ofs);
	end.Add(targ->absmin, targ->absmax);
	end.Multiply(0.5);
	end.Add(targ->view_ofs);
	playerHook_t* pTargetHook = AI_GetPlayerHook(targ);

	//NSS[11/6/99]:Check to see if it even has a hook)
	if (pTargetHook)
	{
		//	use Potentially Visible Set to quickly exclude some entities
		tr = gstate->TraceLine_q2(org, end, self, MASK_SOLID);
		if (tr.fraction >= 1.0f)
		{
			return TRUE;
		}
	}
	return	FALSE;
}


// ----------------------------------------------------------------------------
// NSS[3/2/00]:
// Name:        SIDEKICK_HandleManaSkull
// Description: This handles the manaskull thang!
// Input:	playerHook_t *hook
// Output:  NADA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_HandleManaSkull(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	if (hook)
	{
		if (hook->items & IT_MANASKULL)
		{
			if (hook->invulnerability_time <= 3.0 && (hook->exp_flags & EXP_MANASKULL))
			{
				hook->exp_flags &= ~EXP_MANASKULL;
				gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("artifacts/manaskullfade.wav"), 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
			}

			if (hook->invulnerability_time < 0)
			{
				hook->items &= ~IT_MANASKULL;
			}
			else
				hook->invulnerability_time -= 0.1;
		}
	}
}

// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        SIDEKICK_HandleWraith
// Description: This handles the Wraith mode for Sidekicks
// Input:	userEntity_t *self
// Output:  NADA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_HandleWraith(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	if (hook)
	{
		if (hook->items & IT_WRAITHORB)
		{
			userEntity_t* pWeapon = NULL;

			self->s.renderfx |= RF_TRANSLUCENT;

			// make the clients weapons translucent also
			if (self->client && self->client->ps.weapon)
			{
				pWeapon = (userEntity_t*)self->client->ps.weapon;
				pWeapon->s.renderfx |= RF_TRANSLUCENT;
			}
			//	FIXME:	make wratthorb work
			if (hook->wraithorb_time >= 0)
			{
				float speed = self->velocity.Length();

				// set translucency for player based on velocity
				// set to correct player model
				if (speed < 1.0)
				{
					if (self->s.alpha > 0.20)	// as we're standing still, go invisible
						self->s.alpha -= 0.03;
				}
				else
				{
					if (self->s.alpha < 0.20)	// when moving, go more visible
						self->s.alpha += 0.03;
					else if ((self->s.alpha > 0.20) && (self->s.alpha <= 1.0))
						self->s.alpha -= 0.02;
				}

				if (pWeapon)
					pWeapon->s.alpha = self->s.alpha;

				// shift into predator mode if we're translucent enough
				if (self->s.alpha < 0.22)
				{
					self->s.renderfx |= RF_PREDATOR;
					if (pWeapon)
						pWeapon->s.renderfx |= RF_PREDATOR;
				}
				hook->wraithorb_time -= 0.1;
			}
			else
			{
				// bring translucency back up...
				if (self->s.alpha < 1.0f)
					self->s.alpha += 0.02;

				if (self->s.alpha > 1.0f)
					self->s.alpha = 1.0f;

				if (pWeapon)
					pWeapon->s.alpha = self->s.alpha;

				if (self->s.alpha > 0.22)
				{
					self->s.renderfx &= (~RF_PREDATOR);
					if (pWeapon)
						pWeapon->s.renderfx &= (~RF_PREDATOR);
				}

				//	wraithorb expires when translucency is back to full
				if (1.0f == self->s.alpha)
				{
					hook->items = hook->items - (hook->items & IT_WRAITHORB);
					self->s.renderfx &= ~RF_TRANSLUCENT;
					self->s.renderfx &= ~RF_PREDATOR;
					if (pWeapon)
					{
						pWeapon->s.renderfx &= ~RF_TRANSLUCENT;
						pWeapon->s.renderfx &= ~RF_PREDATOR;
					}
					hook->exp_flags -= (hook->exp_flags & IT_WRAITHORB);
				}
				// added this so the warning plays before it is a moot point, independent of time setting -KRH
				if ((0.3f < self->s.alpha) && (0.32 >= self->s.alpha))
				{
					//	wraithorb expiration sound and message
					gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("artifacts/wraithorburnout.wav"), 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
// NSS[2/28/00]:
// Name:        SIDEKICK_GetAmmoCount
// Description: Gets the current Sidekicks ammo count
// Input:	userEntity_t *self
// Output:  NA
// ----------------------------------------------------------------------------
void SIDEKICK_GetAmmoCount(userEntity_t* self, byte* flag)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	if (hook && hook->owner)
	{
		weapon_t* pWeapon = (weapon_t*)self->curWeapon;
		if (pWeapon && pWeapon->winfo)
		{
			weaponInfo_t* winfo = pWeapon->winfo;

			switch ((int)winfo->EntityType)
			{
			case WEAPON_DISCUS:
			{
				if (AI_IsMikiko(hook))
					*flag = MK_ROCKIN_AMMO;
				else
					*flag = SF_ROCKIN_AMMO;
			}
			return;

			case WEAPON_DISRUPTOR:
			case WEAPON_SILVERCLAW:
			{
				if (AI_IsMikiko(hook))
					*flag = MK_LOW_AMMO;
				else
					*flag = SF_LOW_AMMO;
			}
			return;
			case WEAPON_VENEMOUS:
			{
				if (!pWeapon->ammo->count)
				{
					if (AI_IsMikiko(hook))
						*flag = MK_LOW_AMMO;
					else
						*flag = SF_LOW_AMMO;
				}
			}
			break;

			default:
				break;
			};

			float Ammo, Ammo_Max;
			Ammo_Max = pWeapon->winfo->ammo_max;
			Ammo = pWeapon->ammo->count;
			if (AI_IsMikiko(hook))
			{
				*flag = MK_NO_WEAPON;
				if (Ammo >= Ammo_Max * 0.75f)
				{
					*flag = MK_ROCKIN_AMMO;
				}
				else if (Ammo >= Ammo_Max * 0.35f)
				{
					*flag = MK_GOOD_AMMO;
				}
				else if (Ammo != 0.0f)
				{
					*flag = MK_LOW_AMMO;
				}
			}
			else
			{
				*flag = SF_NO_WEAPON;
				if (Ammo >= Ammo_Max * 0.75f)
				{
					*flag = SF_ROCKIN_AMMO;
				}
				else if (Ammo >= Ammo_Max * 0.40f)
				{
					*flag = SF_GOOD_AMMO;
				}
				else if (Ammo != 0.0f)
				{
					*flag = SF_LOW_AMMO;
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
// NSS[2/28/00]:
// Name:        SIDEKICK_CheckAmmoCount
// Description: Checks the Ammo for the sidekick and updates the hud.
// Input:	userEntity_t *self
// Output:  na
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_CheckAmmoCount(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	playerHook_t* ohook;

	byte flag_mk, flag_sf;

	flag_mk = MK_NO_UPDATE;
	flag_sf = SF_NO_UPDATE;

	userEntity_t* other = SIDEKICK_GetOtherSidekick(self);

	// NSS[2/28/00]:Superfly gets makiko's shiat!
	if (AI_IsSuperfly(hook) && other)
	{
		ohook = AI_GetPlayerHook(other);
		if (ohook && AI_IsSidekick(ohook))
		{
			SIDEKICK_GetAmmoCount(other, &flag_mk);
		}
		SIDEKICK_GetAmmoCount(self, &flag_sf);
	}
	else if (AI_IsSuperfly(hook))
	{
		SIDEKICK_GetAmmoCount(self, &flag_sf);
	}
	else
	{
		SIDEKICK_GetAmmoCount(self, &flag_mk);
	}

	// NSS[2/28/00]:Pak'em up and ship em' off!
	SIDEKICK_UpdateWeaponStatus(hook->owner, flag_mk, flag_sf);
}



// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        SIDEKICK_EndGameHandler
// Description: This function will take care of the sidekicks for good.
// this function is only called from Kage's code and will only be called if Kage
// has been activated.
// Input:	nothing
// Output:  can of kickass
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_EndGameHandler(void)
{
	//	#ifndef Debug
	userEntity_t* Superfly, * Mikiko;

	Superfly = AIINFO_GetSuperfly();
	Mikiko = AIINFO_GetMikiko();

	gstate->RemoveEntity(Superfly);
	gstate->RemoveEntity(Mikiko);

	//	#endif 
}



// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        SIDEKICK_Think
// Description: Sidekicks version 6.66
// Input:	userEntity_t *satan
// Output:  can of kickass
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_Think(userEntity_t* self)
{

	playerHook_t* hook = AI_GetPlayerHook(self);
	playerHook_t* ohook;
	CVector Mins;

	if (hook && AI_IsSidekick(hook))
	{
		if (sv_no_sidekicks && sv_no_sidekicks->value > 0.0f)
		{
			SIDEKICK_Remove(self);
			return;
		}

		if (sv_sidekick_invincible && sv_sidekick_invincible->value > 0.0f)
			self->takedamage = DAMAGE_NO;
	}

	// NSS[3/23/00]:Hack for being frozen by medusa.
	if (!AI_IsAlive(self))
	{
		self->nextthink = gstate->time + 1.0f;
		return;
	}
	// NSS[3/14/00]:HACK HACK HACK HACK HACK HACK HACK HACK!!!!!!!!
	// NSS[3/14/00]:This is here because the physics engine is still completely fucked.
	// Sidekicks sometimes(rarely) get stuck into one another...this will allow superfly
	// the ability to get unstuck from mikiko if this happens.
	if (((int)(gstate->time * 10)) % 10)
	{
		Mins = self->s.mins;
		Mins.z *= 0.65f;
		tr = gstate->TraceBox_q2(self->s.origin, Mins, self->s.maxs, self->s.origin, self, self->clipmask);
		if (tr.ent && (tr.ent->flags & (FL_CLIENT | FL_BOT | FL_MONSTER)) && tr.ent != self)
		{
			if (!(self->svflags & SVF_DEADMONSTER))
			{
				self->svflags |= SVF_DEADMONSTER;
			}
		}
		else if ((self->svflags & SVF_DEADMONSTER))
		{
			self->svflags &= ~SVF_DEADMONSTER;
		}
	}


	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
	// NSS[3/6/00]:Once again... the million'th hack to handle things that don't get restored properly from a load/save game.
	if (!pCurrentGoal)
	{
		if (hook && hook->owner && AI_IsAlive(hook->owner))
			AI_AddNewGoal(self, GOALTYPE_FOLLOW, hook->owner);
		else
			AI_AddNewGoal(self, GOALTYPE_IDLE);

		self->nextthink = gstate->time + 0.10f;
		return;
	}

	TASK_PTR pCurrentTask = GOAL_GetCurrentTask(pCurrentGoal);

	SIDEKICK_SmartThink(self);

	userEntity_t* client_entity = &gstate->g_edicts[1];

	CVector AttackRate;

	// NSS[2/4/00]:Set the rate of attack based on the skill level
	AttackRate.Set(1.0f, 2.0f, 5.0f);

	// NSS[2/28/00]:If both sidekicks are around let the big boy handle the ammo thang!
	userEntity_t* other = SIDEKICK_GetOtherSidekick(self);
	// NSS[2/28/00]:Updates the status of the Sidekick's ammo
	if (AI_IsMikiko(hook) && other)
	{
		ohook = AI_GetPlayerHook(other);
		if (AI_IsSidekick(ohook))
			SIDEKICK_CheckAmmoCount(self);
	}
	else
	{
		SIDEKICK_CheckAmmoCount(self);
	}

	// NSS[2/9/00]:Handle Wraith Mode
	SIDEKICK_HandleWraith(self);

	// NSS[3/2/00]:Handle The Mana Skull thing!
	SIDEKICK_HandleManaSkull(self);

	// NSS[1/31/00]:Restore color for the freezing water thing that will never be used... *sigh*
	if ((hook->pain_finished <= gstate->time) && (GetCurrentEpisode() == 3) && self->s.color.z > 0.0f && !(self->watertype & CONTENTS_WATER))
	{
		self->s.color.z -= 0.10f;
		if (self->s.color.z < 0.20f)
		{
			self->s.color.Set(0.0f, 0.0f, 0.0f);
		}
	}

	// NSS[12/11/99]:If we are not alive and we have reached this point
	// NSS[12/11/99]:We should go ahead and call our die function and then return out.
	if (!AI_IsAlive(self))
	{
		SIDEKICK_Die(self);
		self->nextthink = gstate->time + 0.1f;
		return;
	}
	// NSS[1/28/00]:Don't do a damned thing when cinematic's are playing
	if (gstate->bCinematicPlaying)
	{
		self->nextthink = gstate->time + 0.1f;
		return;
	}

	// NSS[12/9/99]:
	SIDEKICK_HandlePlayerSelectingSidekick(self);


	// NSS[12/5/99]:
	SIDEKICK_CheckMessage(self);

	// NSS[2/4/00]:If we are in search and destroy mode and our enemy is dead then reset
	if ((hook->ai_flags & AI_SIDEKICKKILL) && self->enemy && !(AI_IsAlive(self->enemy)))
	{
		self->enemy = NULL;
		hook->ai_flags &= ~AI_SIDEKICKKILL;
	}
	else if ((hook->ai_flags & AI_SIDEKICKKILL) && !self->enemy)
	{
		self->enemy = NULL;
		hook->ai_flags &= ~AI_SIDEKICKKILL;
	}

	/* Combat is an immediate interrupt for autonomous smart movement. */
	SIDEKICK_SmartCombatInterrupt(self);

	/* Advanced Reasoning may use global level knowledge when the original AI is free. */
	SIDEKICK_AdvancedThink(self);

	/* Optional neural tactical layer. Player commands and engine mechanics remain authoritative. */
	SIDEKICK_NeuralThink(self);
	SIDEKICK_NeuralEnforceIdle(self);
	pGoalStack = AI_GetCurrentGoalStack(hook);
	pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
	pCurrentTask = pCurrentGoal ? GOAL_GetCurrentTask(pCurrentGoal) : NULL;

	if (SIDEKICK_HasTaskInQue(self, TASKTYPE_SIDEKICK_EVADE) || (self->enemy && !SIDEKICK_IsOkToFight(self)))
	{
		AI_SetStateRunning(hook);
	}

	// NSS[2/16/00]:
	if (self->waterlevel > 2)
	{
		if (hook->cur_sequence && hook->cur_sequence->animation_name && !strstr(hook->cur_sequence->animation_name, "swim"))
			AI_ForceSequence(self, "swim", FRAME_LOOP);
	}

	// NSS[3/4/00]:If we are moving around then 
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// NSS[3/4/00]:Check to see if we are idle or not
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (pCurrentTask && hook->owner && AI_IsAlive(hook->owner))
	{
		switch (pCurrentTask->nTaskType)
		{
		case TASKTYPE_SIDEKICK_LOOKAROUND:
		case TASKTYPE_SIDEKICK_LOOKUP:
		case TASKTYPE_SIDEKICK_LOOKDOWN:
		case TASKTYPE_IDLE:
		{
			float WaitTime;
			if (AI_IsSuperfly(hook))
				WaitTime = SuperFlyWaitIdle;
			else
				WaitTime = MikikoWaitIdle;

			// NSS[3/4/00]:Every 5th second let's check to see if we are able to play the impatient sounds.
			if (gstate->time > (WaitTime + 15.0f))
			{
				if (!Check_Que(self, IMPATIENT, 40.0f))
				{
					// NSS[3/4/00]:25% chance they will say something about sitting around.
					if (rnd() < 0.35f)
					{
						SideKick_TalkAmbient(self, IMPATIENT, 1);
						if (AI_IsSuperfly(hook))
							SuperFlyWaitIdle = gstate->time + 10 + (10 * rnd());
						if (AI_IsMikiko(hook))
							MikikoWaitIdle = gstate->time + 10 + (10 * rnd());

					}
					SIDEKICK_SendMessage(self, sCom_Speak, IMPATIENT, (gstate->time + 40.0f) + (rnd() * 60.0f), 0.0f, self, COM_Read);
				}
			}
			// NSS[3/13/00]:
			SIDEKICK_ChitChat(self);
			break;
		}
		default:
		{
			if (AI_IsSuperfly(hook))
				SuperFlyWaitIdle = gstate->time;
			if (AI_IsMikiko(hook))
				MikikoWaitIdle = gstate->time;
		}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// NSS[12/11/99]:Now process the real thinking/ following/ attacking from here down.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// NSS[12/13/99]:Sidekick specific task processing.
	if (pCurrentTask)
	{
		switch (pCurrentTask->nTaskType)
		{
		case TASKTYPE_SIDEKICK_ATTACK:
		{
			SIDEKICK_Attack(self);

			if (!(hook->ai_flags & AI_SIDEKICKKILL) || (self->enemy && !(AI_IsAlive(self->enemy))))
			{
				// NSS[2/4/00]:Clamper
				if (!((int)(gstate->time * 10) % (int)AI_GetSkill(AttackRate)))
				{
					SIDEKICK_Process_Environment(self);
				}
			}
			break;
		}
		case TASKTYPE_SIDEKICK_STOP:
		{
			// NSS[12/13/99]:We are finished with all major tasks, let's look around a bit.
			SIDEKICK_Stop(self);
			break;
		}
		case TASKTYPE_CHASE:
		{
			if (SideKick_IsEnemyVisible(self, self->enemy))
				AI_TaskThink(self);
			else
				AI_RemoveCurrentTask(self, FALSE);
			break;
		}
		case TASKTYPE_MOVETOLOCATION:
		case TASKTYPE_WAITFORNOCOLLISION:
		{
			if (SIDEKICK_HasTaskInQue(self, TASKTYPE_SIDEKICK_ATTACK))
			{
				if (self->enemy)
				{
					if (AI_IsAlive(self->enemy))
					{
						if (SIDEKICK_IsClearShot(self, self->enemy, self->s.origin, self->enemy->s.origin))
						{
							AI_RemoveCurrentTask(self, FALSE);
							break;
						}
					}
					else
					{
						self->enemy = NULL;
						AI_RemoveCurrentTask(self, FALSE);
					}
				}
			}
			AI_TaskThink(self);
			break;
		}
		case TASKTYPE_SIDEKICK_EVADE:
		{
			if (self->enemy && SIDEKICK_IsOkToFight(self))
			{
				AI_RemoveCurrentTask(self, FALSE);
				break;
			}
			AI_TaskThink(self);
			break;
		}
		case TASKTYPE_WAIT:
		{
			if (strstr(hook->cur_sequence->animation_name, "run") || strstr(hook->cur_sequence->animation_name, "walk"))
			{
				char szAnimation[16];
				AI_SelectAmbientAnimation(self, szAnimation);
				AI_ForceSequence(self, szAnimation, FRAME_LOOP);
			}
		}
		case TASKTYPE_SIDEKICK_RANDOMWALK:
		case TASKTYPE_SIDEKICK_LOOKAROUND:
		case TASKTYPE_SIDEKICK_LOOKUP:
		case TASKTYPE_SIDEKICK_LOOKDOWN:
		case TASKTYPE_SIDEKICK_SAYSOMETHING:
		case TASKTYPE_SIDEKICK_KICKSOMETHING:
		case TASKTYPE_SIDEKICK_WHISTLE:
		case TASKTYPE_SIDEKICK_ANIMATE:
		case TASKTYPE_IDLE:
		case TASKTYPE_FOLLOWRUNNING:
		case TASKTYPE_FOLLOWWALKING:
		case TASKTYPE_FINISHCURRENTSEQUENCE:
		case TASKTYPE_SIDEKICK_COMEHERE:
		{

			if (hook->cur_sequence)
			{
				if (strstr(hook->cur_sequence->animation_name, "atak") || strstr(hook->cur_sequence->animation_name, "jump"))
				{
					char szAnimation[16];
					AI_SelectAmbientAnimation(self, szAnimation);
					AI_ForceSequence(self, szAnimation, FRAME_LOOP);
				}
			}
			// NSS[12/13/99]:We are finished with all major tasks, let's look around a bit.
			if (!(hook->ai_flags & AI_SIDEKICKKILL) || (self->enemy && !(AI_IsAlive(self->enemy))))
			{
				// NSS[2/4/00]:Clamper
				if (!((int)(gstate->time * 10) % (int)AI_GetSkill(AttackRate)))
				{
					SIDEKICK_Process_Environment(self);
				}
			}
			else if ((hook->ai_flags & AI_SIDEKICKKILL) && AI_IsAlive(self->enemy))
			{
				if (!SIDEKICK_HasTaskInQue(self, TASKTYPE_SIDEKICK_ATTACK))
				{
					AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_ATTACK);
				}
			}

			// NSS[12/13/99]:handle ambient tasks, if we have one then break otherwise continue on to AI_Think
			if (SIDEKICK_HandleAmbientTasks(self))
			{
				break;
			}
		}
		default:
		{

			//Handle Generic AI Tasks here
			AI_TaskThink(self);
			break;
		}
		}
	}
	else
	{
		if (hook->owner)
		{
			AI_AddNewTask(self, TASKTYPE_FOLLOWRUNNING, hook->owner);
		}
		AI_AddNewTask(self, TASKTYPE_IDLE);
	}
	// NSS[12/13/99]:We always have a next think time!
	AI_SetNextThinkTime(self, 0.1f);
	// NSS[12/13/99]:If this happens then someone is an ass!
	_ASSERTE(self->nextthink > gstate->time);

}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_GetOtherSidekick
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t* SIDEKICK_GetOtherSidekick(userEntity_t* self)
{
	userEntity_t* pOtherSidekick = NULL;
	int nNumSidekicks = AIINFO_GetNumSidekicks();
	if (nNumSidekicks == 2)
	{
		userEntity_t* pFirstSidekick = AIINFO_GetFirstSidekick();
		_ASSERTE(pFirstSidekick);
		userEntity_t* pSecondSidekick = AIINFO_GetSecondSidekick();
		if (!pSecondSidekick || !(pSecondSidekick->flags & FL_BOT))
			return NULL;

		pOtherSidekick = pSecondSidekick;
		if (self == pSecondSidekick)
		{
			pOtherSidekick = pFirstSidekick;
		}
	}

	// NSS[3/14/00]:Make sure we are a sidekick
	if (pOtherSidekick && !(pOtherSidekick->flags & FL_BOT))
	{
		return NULL;
	}
	else
	{
		return pOtherSidekick;
	}

}


// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_CanDoRandomWalk
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_CanDoRandomWalk(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	if (AI_IsOnTrain(self) || SIDEKICK_IsStaying(hook))
	{
		return FALSE;
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_HandleAmbients
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_HandleAmbients(userEntity_t* self)
{
#ifdef DO_AMBIENTS

	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	if (SIDEKICK_NeuralShouldSuppressAmbients(self))
		return FALSE;

	// set the owner to the goal data
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
	if (pCurrentGoal)
	{
		GOALTYPE nGoalType = GOAL_GetType(pCurrentGoal);
		if (nGoalType == GOALTYPE_SIDEKICK_STOPGOINGFURTHER ||
			nGoalType == GOALTYPE_SIDEKICK_TELEPORTANDCOMENEAR)
		{
			if (self->velocity.Length() <= 180.0f && (strstr(hook->cur_sequence->animation_name, "walk") || strstr(hook->cur_sequence->animation_name, "run")))
			{
				if (!strstr(hook->cur_sequence->animation_name, "hit"))
				{
					char szAnimation[16];
					AI_SelectAmbientAnimation(self, szAnimation);
					AI_ForceSequence(self, szAnimation, FRAME_LOOP);
				}
				else if (AI_IsEndAnimation(self))
				{
					char szAnimation[16];
					AI_SelectAmbientAnimation(self, szAnimation);
					AI_ForceSequence(self, szAnimation, FRAME_LOOP);
				}
				AI_ZeroVelocity(self);
			}
			return FALSE;
		}
	}

	float* pNextAmbientTimes = SIDEKICK_GetNextAmbientTimes(self);

	int nNumAmbients = 0;
	TASKTYPE aAmbientTasks[MAX_NUMBER_AMBIENTS];
	for (int i = 0; i < MAX_NUMBER_AMBIENTS; i++)
	{
		if (gstate->time > pNextAmbientTimes[i] &&
			rnd() < aAmbientInfo[i].fRandomFactor)
		{
			aAmbientTasks[nNumAmbients++] = (TASKTYPE)(TASKTYPE_SIDEKICK_RANDOMWALK + i);
		}
	}

	if (nNumAmbients > 0)
	{
		int nRandom = rand() % nNumAmbients;

		if (aAmbientTasks[nRandom] == TASKTYPE_SIDEKICK_RANDOMWALK)
		{
			if (SIDEKICK_CanDoRandomWalk(self))
			{
				int nNumRandomWalk = (rand() % 3) + 1;
				for (i = 0; i < nNumRandomWalk; i++)
				{
					AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_RANDOMWALK);
				}
			}
		}
		else
		{
			AI_AddNewTaskAtFront(self, aAmbientTasks[nRandom]);
		}
		return TRUE;
	}

#endif DO_AMBIENTS
	//NSS[12/3/99]:If we are in ambients and we are still using the walking frame... let's not.
	if (self->velocity.Length() <= 180.0f && (strstr(hook->cur_sequence->animation_name, "walk") || strstr(hook->cur_sequence->animation_name, "run")))
	{
		if (!strstr(hook->cur_sequence->animation_name, "hit"))
		{
			char szAnimation[16];
			AI_SelectAmbientAnimation(self, szAnimation);
			AI_ForceSequence(self, szAnimation, FRAME_LOOP);
		}
		else if (AI_IsEndAnimation(self))
		{
			char szAnimation[16];
			AI_SelectAmbientAnimation(self, szAnimation);
			AI_ForceSequence(self, szAnimation, FRAME_LOOP);
		}
		AI_ZeroVelocity(self);
	}
	else if (self->velocity.Length() <= 48.0f && strstr(hook->cur_sequence->animation_name, "amb"))
	{
		AI_ZeroVelocity(self);
	}
	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_DetermineFollowing
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_DetermineFollowing(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	userEntity_t* pOwner = hook->owner;
	_ASSERTE(pOwner);

	userEntity_t* pOtherSidekick = SIDEKICK_GetOtherSidekick(self);
	// NSS[2/10/00]:Test
	//hook->nFollowing = FOLLOWING_PLAYER;
	//return hook->nFollowing;

	int nNumSidekicks = AIINFO_GetNumSidekicks();
	if (nNumSidekicks == 2)
	{
		if (!pOtherSidekick)
		{
			hook->nFollowing = FOLLOWING_PLAYER;
			return hook->nFollowing;
		}
		if (AI_IsSuperfly(hook) && SIDEKICK_IsCarryingMikiko(hook))
		{
			hook->nFollowing = FOLLOWING_PLAYER;
		}
		else
		{
			playerHook_t* pOtherSidekickHook = AI_GetPlayerHook(pOtherSidekick);
			if (pOtherSidekickHook)
			{
				if (pOtherSidekickHook->nFollowing == FOLLOWING_NONE)
				{
					// make the sidekick that is closer to the player follow first
					float fThisSidekickDistance = VectorDistance(pOwner->s.origin, self->s.origin);
					float fOtherSidekickDistance = VectorDistance(pOwner->s.origin, pOtherSidekick->s.origin);
					if (fThisSidekickDistance < fOtherSidekickDistance)
					{
						hook->nFollowing = FOLLOWING_PLAYER;
					}
					else
					{
						hook->nFollowing = FOLLOWING_SIDEKICK;
					}
				}
				else
					if (pOtherSidekickHook->nFollowing == FOLLOWING_PLAYER)
					{
						hook->nFollowing = FOLLOWING_SIDEKICK;
					}
					else
						if (pOtherSidekickHook->nFollowing == FOLLOWING_SIDEKICK)
						{
							hook->nFollowing = FOLLOWING_PLAYER;
						}
			}
		}
	}
	else
	{
		hook->nFollowing = FOLLOWING_PLAYER;
	}
	return hook->nFollowing;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_HandleFollowing
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_HandleFollowing(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);


	if (hook == NULL || SIDEKICK_IsStaying(hook) || (self->enemy && AI_IsAlive(self->enemy)) || !(hook->ai_flags & AI_FOLLOW))
	{
		return FALSE;
	}

	userEntity_t* pOwner = hook->owner;
	if (pOwner == NULL)
		return FALSE;

	hook->nFollowing = FOLLOWING_NONE;

	// set the owner to the goal data
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
	_ASSERTE(pCurrentGoal);
	GOAL_Set(pCurrentGoal, pOwner);

	TASK_PTR pCurrentTask = GOAL_GetCurrentTask(pCurrentGoal);
	_ASSERTE(pCurrentTask);
	TASKTYPE nTaskType = TASK_GetType(pCurrentTask);

	if (!AI_IsAlive(pOwner))
		return FALSE;

	if (AI_IsOnTrain(pOwner) && !AI_IsOnTrain(self) && AI_IsOnMovingTrain(pOwner))
	{
		float fZDistance = VectorZDistance(self->s.origin, pOwner->s.origin);
		if (fZDistance < MAX_JUMP_HEIGHT)
		{
			if (SIDEKICK_IsAmbientTask(nTaskType))
			{
				AI_RemoveCurrentTask(self, TASKTYPE_MOVETOOWNER, pOwner);
			}
			else
			{
				AI_AddNewTaskAtFront(self, TASKTYPE_MOVETOOWNER, pOwner);
			}
			return TRUE;
		}
	}

	CVector lastPlayerMovedPosition = CLIENT_GetLastPlayerMovedPosition();
	float fRunFollowDistance, fWalkFollowDistance, fDistance, fPlayerMovedDistance, OwnerVelocity;
	int nFollowing = SIDEKICK_DetermineFollowing(self);

	fRunFollowDistance = AI_GetRunFollowDistance(hook);
	fWalkFollowDistance = AI_GetWalkFollowDistance(hook);

	lastPlayerMovedPosition = CLIENT_GetLastPlayerMovedPosition();
	fPlayerMovedDistance = VectorDistance(pOwner->input_entity->s.origin, lastPlayerMovedPosition);

	OwnerVelocity = pOwner->velocity.Length();

	if (nFollowing == FOLLOWING_PLAYER)
	{
		lastPlayerMovedPosition = CLIENT_GetLastPlayerMovedPosition();
		fDistance = VectorDistance(self->s.origin, pOwner->s.origin);
	}
	else
	{
		userEntity_t* ent = SIDEKICK_GetOtherSidekick(self);
		if (ent)
		{
			pOwner = ent;
			fDistance = VectorDistance(self->s.origin, pOwner->s.origin);
		}
	}
	int IsOwnerVisible = AI_IsVisible(self, pOwner);

	if (nFollowing == FOLLOWING_PLAYER &&
		fDistance > SIDEKICK_SMART_BREADCRUMB_DISTANCE &&
		!IsOwnerVisible &&
		!SIDEKICK_HasTaskInQue(self, TASKTYPE_MOVETOLOCATION))
	{
		CVector breadcrumb;
		if (SIDEKICK_SmartFindBreadcrumbPoint(self, pOwner, breadcrumb))
		{
			AI_AddNewTaskAtFront(self, TASKTYPE_MOVETOLOCATION, breadcrumb);
			return TRUE;
		}
	}

	if (!SIDEKICK_CanPathToOrIsCloseToOwner(self, pOwner))
	{
		SIDEKICK_SmartRecoverMovement(self, pOwner->s.origin);
		return TRUE;
	}

	if ((!IsOwnerVisible || fPlayerMovedDistance > 64.0f || OwnerVelocity > 20.0f || fDistance > (fWalkFollowDistance * 2.0f)) && pOwner->deadflag == DEAD_NO)
	{
		// if ( !SIDEKICK_IsAmbientTask( nTaskType ) )
		 //{
		GOALTYPE nGoalType = GOAL_GetType(pCurrentGoal);



		if ((fDistance > fRunFollowDistance || SIDEKICK_HasTaskInQue(self, TASKTYPE_SIDEKICK_ATTACK)) && !AI_IsCrouching(hook))
		{
			if (nFollowing == FOLLOWING_PLAYER)
			{
				if (nGoalType == GOALTYPE_FOLLOW)
				{
					GOAL_ClearTasks(pCurrentGoal);
				}
				AI_AddNewTaskAtFront(self, TASKTYPE_FOLLOWRUNNING, pOwner);
				//AI_ForceSequence(self, "runa");
				return TRUE;
			}
			else
				if (nFollowing == FOLLOWING_SIDEKICK && !AI_IsCrouching(hook))
				{
					userEntity_t* pOtherSidekick = SIDEKICK_GetOtherSidekick(self);
					_ASSERTE(pOtherSidekick);

					if (nGoalType == GOALTYPE_FOLLOW)
					{
						GOAL_ClearTasks(pCurrentGoal);
					}
					AI_AddNewTaskAtFront(self, TASKTYPE_FOLLOWSIDEKICKRUNNING, pOtherSidekick);
					return TRUE;
				}
		}
		else
			if (fDistance > fWalkFollowDistance || !IsOwnerVisible)
			{
				if (nFollowing == FOLLOWING_PLAYER)
				{
					if (nGoalType == GOALTYPE_FOLLOW)
					{
						GOAL_ClearTasks(pCurrentGoal);
					}
					AI_AddNewTaskAtFront(self, TASKTYPE_FOLLOWWALKING, pOwner);
					return TRUE;
				}
				else
					if (nFollowing == FOLLOWING_SIDEKICK)
					{
						userEntity_t* pOtherSidekick = SIDEKICK_GetOtherSidekick(self);
						_ASSERTE(pOtherSidekick);

						if (nGoalType == GOALTYPE_FOLLOW)
						{
							GOAL_ClearTasks(pCurrentGoal);
						}
						AI_AddNewTaskAtFront(self, TASKTYPE_FOLLOWSIDEKICKWALKING, pOtherSidekick);
						return TRUE;
					}
				//}
			}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_TrackOwner
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_TrackOwner(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	userEntity_t* pOwner = hook->owner;
	if (pOwner && pOwner->deadflag == DEAD_NO && !self->enemy)
	{
		self->ideal_ang.pitch = 0.0f;
		com->ChangePitch(self);

		CVector dir = pOwner->s.origin - self->s.origin;
		dir.Normalize();
		CVector angleTowardOwner;
		VectorToAngles(dir, angleTowardOwner);

		float fTowardOwnerY = AngleMod(angleTowardOwner.y);
		float fFacingAngleY = AngleMod(self->s.angles.yaw);

		float fYawDiff = fabs(fTowardOwnerY - fFacingAngleY);
		if (fYawDiff > 60.0f && fYawDiff < 300.0f)
		{
			AI_FaceTowardPoint(self, pOwner->s.origin);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_ResetAmbientTimes
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_ResetAmbientTimes(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	if (AI_IsSidekick(hook))
	{
		// reset the next ambient times
		float* pNextAmbientTimes = SIDEKICK_GetNextAmbientTimes(self);

		for (int i = 0; i < MAX_NUMBER_AMBIENTS; i++)
		{
			pNextAmbientTimes[i] = gstate->time + aAmbientInfo[i].fWaitTime;
		}
	}

}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_HandleIdle
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_HandleIdle(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	hook->nFollowing = FOLLOWING_NONE;

	if (AI_IsSidekick(hook))
	{
		if ((hook->ai_flags & AI_IGNORE_PLAYER))
		{
			return;
		}

		if (!self->groundEntity && self->waterlevel < 3)
		{
			return;
		}

		if (!hook->owner)
		{
			SIDEKICK_FindOwner(self);
		}

		userEntity_t* pOwner = hook->owner;
		if (pOwner && AI_IsAlive(pOwner))
		{

			if (SIDEKICK_HandleFollowing(self) == TRUE)
			{
				return;
			}

			// track owner
			SIDEKICK_TrackOwner(self);

			if (SIDEKICK_HandlePickupItems(self) == FALSE)
			{
				// NSS[2/3/00]:Don't do ambient things if crouching
				if (!hook->bCarryingMikiko && !AI_IsCrouching(hook))
				{
					SIDEKICK_HandleAmbients(self);
				}
				else
				{
					// NSS[12/6/99]:As long as we aren't in the middle of a pain frame.					
					if (self->velocity.Length() <= 48.0f && (strstr(hook->cur_sequence->animation_name, "walk") || strstr(hook->cur_sequence->animation_name, "run") || strstr(hook->cur_sequence->animation_name, "amb")))
					{

						AI_ZeroVelocity(self);
						// NSS[2/3/00]:Don't do ambient things if crouching
						if (!strstr(hook->cur_sequence->animation_name, "hit") && !AI_IsCrouching(hook))
						{
							char szAnimation[16];
							AI_SelectAmbientAnimation(self, szAnimation);
							AI_ForceSequence(self, szAnimation, FRAME_LOOP);
						}
						else if (AI_IsEndAnimation(self) && !AI_IsCrouching(hook))
						{
							char szAnimation[16];
							AI_SelectAmbientAnimation(self, szAnimation);
							AI_ForceSequence(self, szAnimation, FRAME_LOOP);
						}

					}
				}
			}
		}
		else
		{
			self->ideal_ang.pitch = 0.0f;
			com->ChangePitch(self);
			// NSS[2/3/00]:Don't do ambient things if crouching
			if (!AI_IsCrouching(hook))
				SIDEKICK_HandleAmbients(self);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_FindNearUnobstructedPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define BETWEEN_SIDEKICK_DISTANCE   64.0f

int SIDEKICK_FindNearUnobstructedPoint(userEntity_t* self, CVector& point)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	userEntity_t* pOwner = hook->owner;
	if (!pOwner)
	{
		return FALSE;
	}

	userEntity_t* pOtherSidekick = SIDEKICK_GetOtherSidekick(self);
	if (!pOtherSidekick)
	{
		return FALSE;
	}

	CVector twoPoints[2];
	int nNumFreePoints = 0;

	CVector vectorTowardOwner = pOwner->s.origin - pOtherSidekick->s.origin;
	vectorTowardOwner.Normalize();

	CVector angleTowardOwner;
	VectorToAngles(vectorTowardOwner, angleTowardOwner);

	CVector angle = angleTowardOwner;
	angle.pitch = -5.0f;

	CVector endPoint, vector;

	angle.yaw = AngleMod(angleTowardOwner.yaw + 60.0f);
	angle.AngleToForwardVector(vector);
	endPoint.x = pOtherSidekick->s.origin.x + (vector.x * BETWEEN_SIDEKICK_DISTANCE);
	endPoint.y = pOtherSidekick->s.origin.y + (vector.y * BETWEEN_SIDEKICK_DISTANCE);
	endPoint.z = pOtherSidekick->s.origin.z + (vector.z * BETWEEN_SIDEKICK_DISTANCE);
	CVector bottomPoint = endPoint;
	bottomPoint.z -= 1.0f;
	tr = gstate->TraceBox_q2(endPoint, self->s.mins, self->s.maxs, bottomPoint, self, self->clipmask);
	if (tr.fraction >= 1.0f && !tr.allsolid && !tr.startsolid && !tr.ent)
	{
		bottomPoint = endPoint;
		bottomPoint.z -= 100.0f;
		tr = gstate->TraceLine_q2(endPoint, bottomPoint, self, MASK_SOLID);
		if (tr.fraction < 1.0f)
		{
			twoPoints[nNumFreePoints] = endPoint;
			twoPoints[nNumFreePoints].z -= 100.0f * tr.fraction;
			twoPoints[nNumFreePoints].z += 24.0f;

			nNumFreePoints++;
		}
	}

	angle = angleTowardOwner;
	angle.pitch -= 5.0f;
	angle.yaw = AngleMod(angleTowardOwner.yaw - 60.0f);
	angle.AngleToForwardVector(vector);
	endPoint.x = pOtherSidekick->s.origin.x + (vector.x * BETWEEN_SIDEKICK_DISTANCE);
	endPoint.y = pOtherSidekick->s.origin.y + (vector.y * BETWEEN_SIDEKICK_DISTANCE);
	endPoint.z = pOtherSidekick->s.origin.z + (vector.z * BETWEEN_SIDEKICK_DISTANCE);
	bottomPoint = endPoint;
	bottomPoint.z -= 1.0f;
	tr = gstate->TraceBox_q2(endPoint, self->s.mins, self->s.maxs, bottomPoint, self, self->clipmask);
	if (tr.fraction >= 1.0f && !tr.allsolid && !tr.startsolid && !tr.ent)
	{
		bottomPoint = endPoint;
		bottomPoint.z -= 100.0f;
		tr = gstate->TraceLine_q2(endPoint, bottomPoint, self, MASK_SOLID);
		if (tr.fraction < 1.0f)
		{
			twoPoints[nNumFreePoints] = endPoint;
			twoPoints[nNumFreePoints].z -= 100.0f * tr.fraction;
			twoPoints[nNumFreePoints].z += 24.0f;

			nNumFreePoints++;
		}
	}

	int bRetValue = FALSE;
	if (nNumFreePoints == 1)
	{
		point = twoPoints[0];
		bRetValue = TRUE;
	}
	else
		if (nNumFreePoints == 2)
		{
			// find the closer of the two from the owner
			float fDistance1 = VectorDistance(pOwner->s.origin, twoPoints[0]);
			float fDistance2 = VectorDistance(pOwner->s.origin, twoPoints[1]);
			if (fDistance1 < fDistance2)
			{
				point = twoPoints[0];
			}
			else
			{
				point = twoPoints[1];
			}
			bRetValue = TRUE;
		}

	return bRetValue;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_FaceTowardPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_FaceTowardPoint(userEntity_t* self, CVector& destPoint)
{
	_ASSERTE(self);

	CVector dir = destPoint - self->s.origin;
	dir.Normalize();
	CVector newIdealAngle;
	VectorToAngles(dir, newIdealAngle);

	self->ideal_ang = newIdealAngle;
	self->ideal_ang.pitch = 0.0f;

	//	if ( newIdealAngle.pitch > 5.0f && newIdealAngle.pitch < 180.0f )
	//    {
	//        newIdealAngle.pitch *= 0.5f;
	//    }
	//    else
	//    if ( newIdealAngle.pitch >= 180.0f && newIdealAngle.pitch < 355.0f )
	//    {
	//        float fTempPitch = 360.0f - newIdealAngle.pitch;
	//        fTempPitch *= 0.5f;
	//        newIdealAngle.pitch = 360.0f - fTempPitch;
	//    }
	//    self->ideal_ang = newIdealAngle;
	//    if ( self->ideal_ang.pitch > 45.0f && self->ideal_ang.pitch < 315.0f )
	//    {
	//        if ( self->ideal_ang.pitch < 180.0f )
	//        {
	//            self->ideal_ang.pitch = 45.0f;
	//        }
	//        else
	//        {
	//            self->ideal_ang.pitch = 315.0f;
	//        }
	//    }
	//
	//	com->ChangePitch( self );

	com->ChangeYaw(self);
}

// ----------------------------------------------------------------------------
// NSS[2/8/00]:
// Name:        SIDEKICK_HandleMoveAway
// Description: Handles the sidekicks moveing away from one another.
// Input:userEntity_t *self(the one moving away), userEntity_t *other (the one colliding with us)
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_HandleMoveAway(userEntity_t* self, userEntity_t* other)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	playerHook_t* ohook = AI_GetPlayerHook(other);

	//First we move out of their way.. if possible.
	//if (/*!SIDEKICK_IsStaying( hook )*/ && (other->flags & (FL_CLIENT|FL_BOT)) )
	if ((other->flags & (FL_CLIENT | FL_BOT)))
	{
		//if((other->flags & FL_BOT))// NSS[2/10/00]:Handle Small Space areas.
		//{
		//	if(AI_IsInSmallSpace(self) && rnd() < 0.50 && !SIDEKICK_HasTaskInQue(self,TASKTYPE_MOVEAWAY))
		//	{
		//		return;
		//	}
		//}
		if (!SIDEKICK_HasTaskInQue(self, TASKTYPE_MOVEAWAY))// && other->flags & FL_CLIENT)
		{
			if (other->flags & FL_CLIENT)
			{
				if (!Check_Que(self, PUSHEDBY_HIRO, 60.0f))
				{
					SideKick_TalkAmbient(self, PUSHEDBY_HIRO, 1);
					SIDEKICK_SendMessage(self, sCom_Speak, PUSHEDBY_HIRO, gstate->time + 60.0f, 0.0f, self, COM_Read);
					return;
				}
			}
			AI_AddNewTaskAtFront(self, TASKTYPE_MOVEAWAY, other);
		}
	}
	else if (other->flags & FL_BOT)
	{
		//Second if we are unable to then we need to tell them to stop 
		if (!SIDEKICK_HasTaskInQue(self, TASKTYPE_SIDEKICK_STOP))
		{
			AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_STOP, other);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_IsOwnerClose
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsOwnerClose(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);

	userEntity_t* pOwner = hook->owner;
	_ASSERTE(pOwner);

	float fRunFollowDistance = AI_GetRunFollowDistance(hook);

	float fDistance = VectorDistance(self->s.origin, pOwner->s.origin);
	if (fDistance > (fRunFollowDistance * 4.0f) ||
		(fDistance > (fRunFollowDistance * 2.0f) && !AI_IsLineOfSight(self, pOwner))
		)
	{
		return FALSE;
	}

	return TRUE;
}


// ----------------------------------------------------------------------------
// NSS[1/23/00]: New Function to find a good retreat Node
// Name:		SIDEKICK_FindFurthestNodeFrom
// Description:A majorly re-worked function similar to NODE_FindFurthestFrom but
// greatly enhanced.  This function searches multiple layers within the node system
// to determine the best Node to retreat to.  This function DOES use recursion.
// Input: NodeTraverse *NodeT
// Output: TRUE/FALSE and NodeT->pCurrentNode is the best destination if returned True.
// Note: NodeTraverse Struct layout
// NODELIST_PTR pNodeList --> node list to parse through
// CVector &Origin		  --> Origin to Determine distance between nodes.
// int NodeIndex		  --> current Node we are looking at.
// float Distance		  --> current Distance away
// int NodeDepth          --> current NodeDepth
// float MaximumDistance  --> Maximum Distance to traverse from the origin
// float CurrentDistance  --> Current distance traversed from the origin
// ----------------------------------------------------------------------------
int	SIDEKICK_FindFurthestNodeFrom(NodeTraverse* NodeT)
{
	int i;
	float fDistance;
	float CurrentDistance;
	MAPNODE_PTR pBestLinkNode, pLinkNode;
	NODEHEADER_PTR pNodeHeader;

	// NSS[1/24/00]:Check our nodelist and current index before doing anything
	if (NodeT->pNodeList == NULL || (NodeT->pNodeList->nCurrentNodeIndex == -1))
	{
		return FALSE;
	}

	// NSS[1/24/00]:Setup all node pointers and the header
	pBestLinkNode = NULL;
	pNodeHeader = NodeT->pNodeList->pNodeHeader;
	NodeT->pCurrentNode = &(pNodeHeader->pNodes[NodeT->CurrentNodeIndex]);

	// NSS[1/24/00]:Make sure we are traversing a valid node
	if (!NodeT->pCurrentNode)
	{
		return FALSE;
	}

	if ((NodeT->NodeDepth >= NodeT->MaximumNodeDepth) || (NodeT->CurrentDistance >= NodeT->MaximumDistance))
	{
		return TRUE;
	}

	for (i = 0; i < NodeT->pCurrentNode->nNumLinks; i++)
	{
		pLinkNode = &(pNodeHeader->pNodes[NodeT->pCurrentNode->aLinks[i].nIndex]);

		if (pLinkNode->nNumLinks > 1)
		{
			fDistance = VectorXYDistance(pLinkNode->position, NodeT->origin);
			if (fDistance > NodeT->CurrentDistance)
			{
				if (NodeT->CurrentDistance > NodeT->GreatestDistance)
				{
					NodeT->GreatestDistance = NodeT->CurrentDistance;
					NodeT->pBestNode = pLinkNode;
				}

				// NSS[2/1/00]:Store this itterations currentdistance value
				CurrentDistance = NodeT->CurrentDistance;
				// NSS[2/1/00]:Setup the next itterations CurrentDistance, index, and depth;
				NodeT->CurrentDistance = fDistance;
				NodeT->CurrentNodeIndex = pLinkNode->nIndex;
				NodeT->NodeDepth++;
				if (SIDEKICK_FindFurthestNodeFrom(NodeT))
				{
					return TRUE;
				}
				else
				{
					NodeT->CurrentDistance = CurrentDistance;
					NodeT->CurrentNodeIndex = NodeT->pCurrentNode->nIndex;
					NodeT->NodeDepth--;
				}
			}
		}
		else//Handle single linked nodes
		{
			fDistance = NodeT->CurrentDistance + VectorXYDistance(NodeT->pCurrentNode->position, NodeT->origin);
			if (fDistance > NodeT->MaximumDistance)
			{
				NodeT->CurrentDistance = fDistance;
				return TRUE;
			}
		}
	}
	return FALSE;
}

// ----------------------------------------------------------------------------
// NSS[1/23/00]:Revamped version of Sidekick's evade function
// Name:		SIDEKICK_StartEvade
// Description:Run away!
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_StartEvade(userEntity_t* self)
{
	_ASSERTE(self);

	NodeTraverse	NodeT;

	int				Trap = 0;

	playerHook_t* hook = AI_GetPlayerHook(self);
	// NSS[1/23/00]:Check our hook
	if (!hook)
	{
		AI_Dprintf("Problemo's no hook and Sidekick is trying to evade!!\n");
		AI_RemoveCurrentTask(self, FALSE);
	}




	AI_SetStateRunning(hook);


	// NSS[1/23/00]:Check our enemy pointer
	//If we don't have an enemy get out.  If our enemy is dead and doesn't have the explosive flag set get out.  Otherwise if 
	//our enemy is dead but has the explosive flag set then get the fuck outta there.
	if (!self->enemy || (!AI_IsAlive(self->enemy) && !(self->enemy->flags & FL_EXPLOSIVE)))
	{
		AI_RemoveCurrentTask(self);
		return;
	}

	// NSS[1/31/00]:Added evade sounds to the sidekicks.
	// NSS[2/23/00]:We should move out of the way from missiles but not scream about them... 
	if (!Check_Que(self, EVADE, 15.0f) && !(self->enemy->flags & FL_CLIENT) && !strstr(self->enemy->className, "projectile_missile"))
	{
		SideKick_TalkAmbient(self, EVADE, 1);
		SIDEKICK_SendMessage(self, sCom_Speak, EVADE, gstate->time + 5.0, 0.0f, self, COM_Read);
	}

	if (hook->pNodeList)
	{

		// NSS[1/24/00]:Setup Start Point
		NodeT.origin = self->enemy->s.origin;
		NodeT.NodeDepth = 0;
		NodeT.CurrentDistance = VectorXYDistance(self->s.origin, self->enemy->s.origin);
		NodeT.CurrentNodeIndex = hook->pNodeList->nCurrentNodeIndex;
		NodeT.GreatestDistance = 0.0f;
		NodeT.pNodeList = hook->pNodeList;
		NodeT.MaximumNodeDepth = 5;
		NodeT.pBestNode = NULL;
		if (self->enemy->flags & FL_EXPLOSIVE)
		{
			NodeT.MaximumDistance = 256.0f;
		}
		else
		{
			float fDistanceFrom = VectorDistance(self->s.origin, self->enemy->s.origin);
			NodeT.MaximumDistance = 700.0f + fDistanceFrom;
		}

		if (!SIDEKICK_FindFurthestNodeFrom(&NodeT))
		{
			// NSS[1/22/00]:If not then get the HideNode

			if (!NodeT.pBestNode)
			{

				MAPNODE_PTR pRetreatNode = NODE_GetClosestCompleteHideNode(hook->pNodeList->pNodeHeader, self, self->enemy);
				if (!pRetreatNode)
				{
					Trap = 1;
				}
				else
				{
					NodeT.pBestNode = pRetreatNode;
				}
			}

		}
	}

	if (NodeT.pBestNode)
	{
		if (AI_IsCloseDistance2(self, VectorXYDistance(self->s.origin, NodeT.pBestNode->position)))
		{
			Trap = 1;
		}
	}
	else
	{
		AI_Dprintf("Sidekick Trying to Evade but finding no path or nodes to use!!!\n");
		Trap = 1;
	}
	// NSS[2/1/00]:This trap prevents jitter in animations due to the fact that the sidekick might
	// get in a situation where they just can't run anywhere else...if so don't run
	if (!Trap)
	{
		frameData_t* pCurrentSequence = hook->cur_sequence;
		if (!strstr(pCurrentSequence->animation_name, "run"))
		{
			char szAnimation[16];
			AI_SelectRunningAnimation(self, szAnimation);
			AI_ForceSequence(self, szAnimation, FRAME_LOOP);
		}
	}

	AI_SetNextThinkTime(self, 0.1f);
	AI_SetOkToAttackFlag(hook, FALSE);
	AI_SetTaskFinishTime(hook, 2.0f);
	AI_SetMovingCounter(hook, 0);
}


// ----------------------------------------------------------------------------
// NSS[1/23/00]:Revamped SidekickEvade function
// Name:		SIDEKICK_Evade
// Description:Main loop for finding the next node to traverse towards.
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_Evade(userEntity_t* self)
{
	NodeTraverse				NodeT;
	playerHook_t* hook = AI_GetPlayerHook(self);
	float fDistance = 0.0f;

	if (!hook)
	{
		AI_Dprintf("Problemo's no hook and Sidekick is trying to evade!!\n");
		AI_RemoveCurrentTask(self, FALSE);
		return;
	}

	// NSS[1/23/00]:Check our enemy
	if (self->enemy)
	{
		fDistance = VectorDistance(self->s.origin, self->enemy->s.origin);
	}
	else
	{
		AI_RemoveCurrentTask(self, FALSE);
		return;
	}


	AI_SetStateRunning(hook);
	if (fDistance > 700.0f && !AI_IsVisible(self, self->enemy))
	{
		AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_STOP);
	}

	if (AI_IsEndAnimation(self))
	{
		char szAnimation[16];
		AI_SelectRunningAnimation(self, szAnimation);
		AI_ForceSequence(self, szAnimation, FRAME_LOOP);
	}

	if (hook->pNodeList)
	{

		// NSS[1/24/00]:Setup Start Point
		NodeT.origin = self->enemy->s.origin;
		NodeT.NodeDepth = 0;
		NodeT.CurrentDistance = VectorXYDistance(self->s.origin, self->enemy->s.origin);
		NodeT.CurrentNodeIndex = hook->pNodeList->nCurrentNodeIndex;
		NodeT.GreatestDistance = 0.0f;
		NodeT.pNodeList = hook->pNodeList;
		NodeT.MaximumNodeDepth = 5;
		NodeT.pBestNode = NULL;
		if (self->enemy->flags & FL_EXPLOSIVE)
		{
			NodeT.MaximumDistance = 256.0f;
		}
		else
		{
			float fDistanceFrom = VectorDistance(self->s.origin, self->enemy->s.origin);
			NodeT.MaximumDistance = 700.0f + fDistanceFrom;
		}

		if (!SIDEKICK_FindFurthestNodeFrom(&NodeT))
		{
			// NSS[1/22/00]:If not then get the HideNode	
			if (!NodeT.pBestNode)
			{
				MAPNODE_PTR pRetreatNode = NODE_GetClosestCompleteHideNode(hook->pNodeList->pNodeHeader, self, self->enemy);
				if (!pRetreatNode)
				{
					if (hook->owner)
					{
						// NSS[2/7/00]:Run home to da-da
						if (!AI_IsCrouching(hook))
						{
							AI_AddNewTaskAtFront(self, TASKTYPE_FOLLOWRUNNING, hook->owner);
						}
						else
						{
							AI_AddNewTaskAtFront(self, TASKTYPE_FOLLOWWALKING, hook->owner);
						}

					}
					else
					{
						//	nowhere to run to, hang out a bit and get the shit kicked out of us... 
						AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_STOP, hook->owner->s.origin);
					}
					return;
				}
				NodeT.pBestNode = pRetreatNode;
			}

		}
	}

	if (NodeT.pBestNode)
	{
		if (AI_IsCloseDistance2(self, VectorXYDistance(self->s.origin, NodeT.pBestNode->position)))
		{
			if (hook->owner)
			{
				// NSS[2/7/00]:Run home to da-da
				if (!AI_IsCrouching(hook))
					AI_AddNewTaskAtFront(self, TASKTYPE_FOLLOWRUNNING, hook->owner);
				else
					AI_AddNewTaskAtFront(self, TASKTYPE_FOLLOWWALKING, hook->owner);
			}
			else
			{
				//	nowhere to run to, hang out a bit and get the shit kicked out of us... 
				AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_STOP, hook->owner->s.origin);
			}

		}
		else
		{
			if (hook->owner)
			{
				float oDistance;
				oDistance = VectorDistance(self->s.origin, hook->owner->s.origin);
				if (oDistance > 768.0f)
				{
					// NSS[2/7/00]:Run home to da-da
					if (!AI_IsCrouching(hook))
						AI_AddNewTaskAtFront(self, TASKTYPE_FOLLOWRUNNING, hook->owner);
					else
						AI_AddNewTaskAtFront(self, TASKTYPE_FOLLOWWALKING, hook->owner);
				}
				else
				{
					AI_AddNewTaskAtFront(self, TASKTYPE_MOVETOLOCATION, NodeT.pBestNode->position);
				}
			}
			else
			{
				AI_AddNewTaskAtFront(self, TASKTYPE_MOVETOLOCATION, NodeT.pBestNode->position);
			}
		}
	}
	else
	{
		AI_Dprintf("Sidekick Trying to Evade but finding no path or nodes to use!!!\n");
		AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_STOP);
	}
	AI_SetTaskFinishTime(hook, 1.0f);
}


// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_DoStopGoingFurther
// Description:
//              int nOrder => 1 = first time
//                            2 = second time
//                              this var is used so that sound is played only once
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_DoStopGoingFurther(userEntity_t* self, CVector& position,
	char* szSound, char* szAnimation, int nOrder /* = 1 */)
{
	_ASSERTE(self);

	if (!AI_IsAlive(self))
	{
		return FALSE;
	}

	playerHook_t* hook = AI_GetPlayerHook(self);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	if (GOALSTACK_GetCurrentGoalType(pGoalStack) == GOALTYPE_SIDEKICK_STOPGOINGFURTHER)
	{
		return FALSE;
	}

	AI_AddNewGoal(self, GOALTYPE_SIDEKICK_STOPGOINGFURTHER);

	TASKTYPE nTaskType = TASKTYPE_NONE;
	if (szSound && strlen(szSound) > 0 && szAnimation && strlen(szAnimation) > 0 && nOrder == 1)
	{
		AIDATA aiData;
		frameData_t* pSequence = FRAMES_GetSequence(self, szAnimation);
		if (pSequence)
		{
			aiData.pAnimSequence = pSequence;
		}
		aiData.pString = szSound;

		AI_AddNewTask(self, TASKTYPE_PLAYANIMATIONANDSOUND, &aiData);

		nTaskType = TASKTYPE_PLAYANIMATIONANDSOUND;
	}
	else
	{
		if (szAnimation && strlen(szAnimation) > 0)
		{
			frameData_t* pSequence = FRAMES_GetSequence(self, szAnimation);
			AI_AddNewTask(self, TASKTYPE_PLAYANIMATION, pSequence);

			if (nTaskType == TASKTYPE_NONE)
			{
				nTaskType = TASKTYPE_PLAYANIMATION;
			}
		}
		if (szSound && strlen(szSound) > 0 && nOrder == 1)
		{
			PlaySidekickMP3(self, szSound);//AI_AddNewTask( self, TASKTYPE_PLAYSOUND, szSound );

			//if ( nTaskType == TASKTYPE_NONE )
			//{
			//  nTaskType = TASKTYPE_PLAYSOUND;
			//}
		}
	}

	if (position.Length() > 0.0f)
	{
		AI_SetStateRunning(hook);
		AI_AddNewTask(self, TASKTYPE_MOVETOLOCATION, position);

		if (nTaskType == TASKTYPE_NONE)
		{
			nTaskType = TASKTYPE_MOVETOLOCATION;
		}
	}

	AI_AddNewTask(self, TASKTYPE_SIDEKICK_STOPGOINGFURTHER);

	if (nTaskType == TASKTYPE_PLAYANIMATIONANDSOUND)
	{
		AI_StartPlayAnimationAndSound(self);
	}
	else
		if (nTaskType == TASKTYPE_PLAYANIMATION)
		{
			AI_StartPlayAnimation(self);
		}
		else
			if (nTaskType == TASKTYPE_MOVETOLOCATION)
			{
				AI_StartMoveToLocation(self);
			}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_DoStopGoingFurther
// Description:
//              int nOrder => 1 = first time
//                            2 = second time
//                              this var is used so that sound is played only once
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_DoStopGoingFurther(userEntity_t* self, MAPNODE_PTR pCurrentNode, int nOrder /* = 1 */)
{
	if (!AI_IsAlive(self) || !pCurrentNode)
	{
		return FALSE;
	}

	CVector position = pCurrentNode->node_data.vector;
	char* szSound = pCurrentNode->target;
	char* szAnimation = pCurrentNode->targetname;

	return SIDEKICK_DoStopGoingFurther(self, position, szSound, szAnimation, nOrder);

}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_DoTeleportAndComeNear
// Description:
//              int nOrder => 1 = first time
//                            2 = second time
//                              this var is used so that sound is played only once
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_DoTeleportAndComeNear(userEntity_t* self, CVector& position,
	char* szSound, char* szAnimation, int nOrder /* = 1 */)
{
	_ASSERTE(self);

	if (!AI_IsAlive(self))
	{
		return FALSE;
	}
	playerHook_t* hook = AI_GetPlayerHook(self);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	if (GOALSTACK_GetCurrentGoalType(pGoalStack) == GOALTYPE_SIDEKICK_TELEPORTANDCOMENEAR)
	{
		return FALSE;
	}

	AI_AddNewGoal(self, GOALTYPE_SIDEKICK_TELEPORTANDCOMENEAR);

	TASKTYPE nTaskType = TASKTYPE_NONE;

	userEntity_t* pSecondSidekick = AIINFO_GetSecondSidekick();
	if (self == pSecondSidekick)
	{
		AI_AddNewTask(self, TASKTYPE_WAIT, 1.0f);
		nTaskType = TASKTYPE_WAIT;
	}

	if (position.Length() > 0.0f)
	{
		AI_AddNewTask(self, TASKTYPE_SIDEKICK_TELEPORT, position);

		if (nTaskType == TASKTYPE_NONE)
		{
			nTaskType = TASKTYPE_SIDEKICK_TELEPORT;
		}
	}

	if (position.Length() > 0.0f)
	{
		userEntity_t* pOwner = hook->owner;
		if (AI_IsAlive(pOwner))
		{
			AI_AddNewTask(self, TASKTYPE_MOVETOENTITYUNTILVISIBLE, pOwner);

			if (nTaskType == TASKTYPE_NONE)
			{
				nTaskType = TASKTYPE_MOVETOENTITYUNTILVISIBLE;
			}
		}
	}

	if (szSound && strlen(szSound) > 0 && szAnimation && strlen(szAnimation) > 0 && nOrder == 1)
	{
		AIDATA aiData;
		frameData_t* pSequence = FRAMES_GetSequence(self, szAnimation);
		if (pSequence)
		{
			aiData.pAnimSequence = pSequence;
		}
		aiData.pString = szSound;

		AI_AddNewTask(self, TASKTYPE_PLAYANIMATIONANDSOUND, &aiData);

		if (nTaskType == TASKTYPE_NONE)
		{
			nTaskType = TASKTYPE_PLAYANIMATIONANDSOUND;
		}
	}
	else
	{
		if (szAnimation && strlen(szAnimation) > 0)
		{
			frameData_t* pSequence = FRAMES_GetSequence(self, szAnimation);
			AI_AddNewTask(self, TASKTYPE_PLAYANIMATION, pSequence);

			if (nTaskType == TASKTYPE_NONE)
			{
				nTaskType = TASKTYPE_PLAYANIMATION;
			}
		}
		if (szSound && strlen(szSound) > 0 && nOrder == 1)
		{
			AI_AddNewTask(self, TASKTYPE_PLAYSOUND, szSound);

			if (nTaskType == TASKTYPE_NONE)
			{
				nTaskType = TASKTYPE_PLAYSOUND;
			}
		}
	}

	AI_AddNewTask(self, TASKTYPE_SIDEKICK_TELEPORTANDCOMENEAR);

	if (nTaskType == TASKTYPE_WAIT)
	{
		AI_StartWait(self);
	}
	else
		if (nTaskType == TASKTYPE_SIDEKICK_TELEPORT)
		{
			SIDEKICK_StartTeleport(self);
		}
		else
			if (nTaskType == TASKTYPE_MOVETOENTITYUNTILVISIBLE)
			{
				AI_StartMoveToEntityUntilVisible(self);
			}
			else
				if (nTaskType == TASKTYPE_PLAYANIMATION)
				{
					AI_StartPlayAnimation(self);
				}
				else
					if (nTaskType == TASKTYPE_PLAYSOUND)
					{
						AI_StartPlaySound(self);
					}
					else
						if (nTaskType == TASKTYPE_MOVETOLOCATION)
						{
							AI_StartMoveToLocation(self);
						}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_DoTeleportAndComeNear
// Description:
//              int nOrder => 1 = first time
//                            2 = second time
//                              this var is used so that sound is played only once
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_DoTeleportAndComeNear(userEntity_t* self, MAPNODE_PTR pCurrentNode, int nOrder /* = 1 */)
{
	if (!AI_IsAlive(self) || !pCurrentNode)
	{
		return FALSE;
	}

	CVector position = pCurrentNode->node_data.vector;
	char* szSound = pCurrentNode->target;
	char* szAnimation = pCurrentNode->targetname;

	return SIDEKICK_DoTeleportAndComeNear(self, position, szSound, szAnimation, nOrder);

}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_HandleSuperflyTalk
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_HandleSuperflyTalk(userEntity_t* self, MAPNODE_PTR pCurrentNode)
{
	_ASSERTE(self);
	_ASSERTE(pCurrentNode);

	if (!self || !pCurrentNode)
	{
		return FALSE;
	}

	playerHook_t* hook = AI_GetPlayerHook(self);
	_ASSERTE(AI_IsSidekick(hook));

	if (!AI_IsSuperfly(hook))
	{
		return FALSE;
	}

	int bRetValue = FALSE;

	userEntity_t* owner = hook->owner;

	if (owner && (pCurrentNode->node_type & NODETYPE_SUPERFLYTALK))
	{
		float fDistance = VectorDistance(self->s.origin, owner->s.origin);
		if (!Check_Que(self, NO_TALK, 1.0f) && fDistance < 800.0f && AI_IsVisible(self, owner))
		{
			fDistance = VectorDistance(self->s.origin, pCurrentNode->position);
			if (AI_IsCloseDistance2(self, fDistance) && NODE_IsTriggered(pCurrentNode) == FALSE)
			{
				NODE_SetTriggered(pCurrentNode);
				if (pCurrentNode->target)
				{
					AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_SAYSOMETHING, pCurrentNode->target);
				}

				bRetValue = TRUE;
			}
		}
	}

	return bRetValue;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_HandleMikikoTalk
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_HandleMikikoTalk(userEntity_t* self, MAPNODE_PTR pCurrentNode)
{
	_ASSERTE(self);
	_ASSERTE(pCurrentNode);

	if (!self || !pCurrentNode)
	{
		return FALSE;
	}

	playerHook_t* hook = AI_GetPlayerHook(self);
	_ASSERTE(AI_IsSidekick(hook));

	if (!AI_IsMikiko(hook))
	{
		return FALSE;
	}

	// NSS[3/12/00]:If we can't talk then let's not do a damned thing.
	if (Check_Que(self, NO_TALK, 1.0f))
	{
		return FALSE;
	}

	int bRetValue = FALSE;
	userEntity_t* owner = hook->owner;
	if (owner && (pCurrentNode->node_type & NODETYPE_MIKIKOTALK))
	{
		float fDistance = VectorDistance(self->s.origin, owner->s.origin);
		if (!Check_Que(self, SPECIAL_TALK, 5.0f) && fDistance < 800.0f && AI_IsVisible(self, owner))
		{
			fDistance = VectorDistance(self->s.origin, pCurrentNode->position);
			if (AI_IsCloseDistance2(self, fDistance) && NODE_IsTriggered(pCurrentNode) == FALSE)
			{
				NODE_SetTriggered(pCurrentNode);
				if (pCurrentNode->target)
				{
					AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_SAYSOMETHING, pCurrentNode->target);
				}

				bRetValue = TRUE;
			}
		}
	}
	return bRetValue;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_ReadAmbientInfo
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_ReadAmbientInfo(userEntity_t* self, const char* szFileName)
{
	int bSuccess = TRUE;

	CCSVFile* pCsvFile = NULL;
	int nRetCode = CSV_OpenFile(szFileName, &pCsvFile);
	if (nRetCode == DKIO_ERROR_NONE)
	{
		int nNumSequences = 0;

		char szLine[2048];
		char szElement[64];
		// skip first line
		CSV_GetNextLine(pCsvFile, szLine);

		int i = 0;
		while (CSV_GetNextLine(pCsvFile, szLine) != EOF)
		{
			int nRetValue = CSV_GetFirstElement(pCsvFile, szLine, szElement);
			if (nRetValue > 0)
			{
				float fValue = 0.0f;
				if ((nRetValue = CSV_GetNextElement(pCsvFile, szLine, szElement)) != EOLN)
				{
					int nRetValue = sscanf(szElement, "%f", &fValue);
					if (nRetValue > 0)
					{
						aAmbientInfo[i].fWaitTime = fValue;
					}
				}

				if ((nRetValue = CSV_GetNextElement(pCsvFile, szLine, szElement)) != EOLN)
				{
					int nRetValue = sscanf(szElement, "%f", &fValue);
					if (nRetValue > 0)
					{
						aAmbientInfo[i].fRandomFactor = fValue;
					}
				}

				i++;
			}

		}

		CSV_CloseFile(pCsvFile);
	}
	else
	{
		bSuccess = FALSE;
	}

	return bSuccess;
}


// ----------------------------------------------------------------------------
//                      SIDEKICK attacking player code
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_AttackPlayerUpdateMeleeAttackMovement
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_AttackPlayerUpdateMeleeAttackMovement(userEntity_t* self)
{
	userEntity_t* pEnemy = self->enemy;
	if (!AI_IsAlive(pEnemy))
	{
		return FALSE;
	}

	playerHook_t* hook = AI_GetPlayerHook(self);

	int bCanSeeEnemy = AI_IsLineOfSight(self, pEnemy);

	float fDistance = VectorDistance(self->s.origin, pEnemy->s.origin);
	if (fDistance < AI_GetCurrentWeaponRange(self))
	{
		AI_ZeroVelocity(self);
	}
	else
	{
		SIDEKICK_MoveTowardPointWhileAttacking(self, pEnemy->s.origin);
	}

	SIDEKICK_SelectAnimationWhileAttacking(self);

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_AttackPlayerUpdateRangeAttackMovement
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_AttackPlayerUpdateRangeAttackMovement(userEntity_t* self)
{
	userEntity_t* pEnemy = self->enemy;
	if (!AI_IsAlive(pEnemy))
	{
		return FALSE;
	}

	playerHook_t* hook = AI_GetPlayerHook(self);

	int nPrevAttackMode = hook->nAttackMode;

	int bOwnerBlockingShot = SIDEKICK_IsSquadmateBlockingShot(self, pEnemy);
	int bCanSeeEnemy = AI_IsCompletelyVisible(self, pEnemy, SIDEKICK_WIDTH_FACTOR);

	if (bOwnerBlockingShot)
	{
		if (hook->nAttackMode != ATTACKMODE_SIDEKICK_STRAFE)
		{
			SIDEKICK_SelectOwnerBypassDirection(self, pEnemy);
		}
		hook->nAttackMode = ATTACKMODE_SIDEKICK_STRAFE;
		if (hook->nAttackMode != nPrevAttackMode)
			PATHLIST_KillPath(hook->pPathList);
		SIDEKICK_AttackWhileStrafing(self, pEnemy);
		SIDEKICK_SelectAnimationWhileAttacking(self);
		return TRUE;
	}
	if (gstate->time > hook->fAttackTime)
	{
		if (SIDEKICK_IsAboutToFire(hook))
		{
			if (bCanSeeEnemy == NOT_VISIBLE)
			{
				hook->nAttackMode = ATTACKMODE_SIDEKICK_CHARGE;
				SIDEKICK_SetAttackTime(hook, gstate->time + (rnd() * 2.0f) + 2.0f);
			}
			else
			{
				if (rnd() > 0.5f)
				{
					if (SIDEKICK_ShouldStrafe(self, pEnemy))
					{
						hook->nAttackMode = ATTACKMODE_SIDEKICK_STRAFE;
						SIDEKICK_SetAttackTime(hook, gstate->time + (rnd() * 4.0f) + 4.0f);
					}
					else
					{
						hook->nAttackMode = ATTACKMODE_SIDEKICK_BACKAWAY;
						SIDEKICK_SetAttackTime(hook, gstate->time + (rnd() * 4.0f) + 3.0f);
					}
				}
				else
				{
					hook->nAttackMode = ATTACKMODE_SIDEKICK_STAYSTILL;
					SIDEKICK_SetAttackTime(hook, gstate->time + 0.5f);
				}
			}
		}
		else
			if (AI_IsJustFired(hook))
			{
				if (rnd() > 0.5f && bCanSeeEnemy == NOT_VISIBLE)
				{
					if (rnd() > 0.5f && SIDEKICK_ShouldCharge(self, pEnemy))
					{
						hook->nAttackMode = ATTACKMODE_SIDEKICK_CHARGE;
						SIDEKICK_SetAttackTime(hook, gstate->time + (rnd() * 2.0f) + 2.0f);
					}
					else
					{
						if (SIDEKICK_ShouldStrafe(self, pEnemy))
						{
							hook->nAttackMode = ATTACKMODE_SIDEKICK_STRAFE;
							SIDEKICK_SetAttackTime(hook, gstate->time + (rnd() * 4.0f) + 4.0f);
						}
						else
						{
							hook->nAttackMode = ATTACKMODE_SIDEKICK_BACKAWAY;
							SIDEKICK_SetAttackTime(hook, gstate->time + (rnd() * 4.0f) + 3.0f);
						}
					}
				}
				else
				{
					if (rnd() > 0.5f)
					{
						hook->nAttackMode = ATTACKMODE_SIDEKICK_BACKAWAY;
						SIDEKICK_SetAttackTime(hook, gstate->time + (rnd() * 4.0f) + 3.0f);
					}
					else
					{
						hook->nAttackMode = ATTACKMODE_SIDEKICK_TAKECOVER;
						SIDEKICK_SetAttackTime(hook, gstate->time + (rnd() * 4.0f) + 5.0f);
					}
				}
			}
			else
			{
				if (bCanSeeEnemy == NOT_VISIBLE)
				{
					hook->nAttackMode = ATTACKMODE_SIDEKICK_STAYSTILL;
					SIDEKICK_SetAttackTime(hook, gstate->time + 0.5f);
				}
				else
				{
					int nRandom = rand() % 4;
					switch (nRandom)
					{
					case 0:
					{
						hook->nAttackMode = ATTACKMODE_SIDEKICK_TAKECOVER;
						SIDEKICK_SetAttackTime(hook, gstate->time + (rnd() * 4.0f) + 7.0f);
						break;
					}
					case 1:
					{
						if (SIDEKICK_ShouldStrafe(self, pEnemy))
						{
							hook->nAttackMode = ATTACKMODE_SIDEKICK_STRAFE;
							SIDEKICK_SetAttackTime(hook, gstate->time + (rnd() * 4.0f) + 4.0f);
						}
						else
						{
							hook->nAttackMode = ATTACKMODE_SIDEKICK_BACKAWAY;
							SIDEKICK_SetAttackTime(hook, gstate->time + (rnd() * 4.0f) + 3.0f);
						}
						break;
					}
					case 2:
					{
						hook->nAttackMode = ATTACKMODE_SIDEKICK_BACKAWAY;
						SIDEKICK_SetAttackTime(hook, gstate->time + (rnd() * 4.0f) + 3.0f);
						break;
					}
					case 3:
					{
						if (SIDEKICK_ShouldCharge(self, pEnemy))
						{
							hook->nAttackMode = ATTACKMODE_SIDEKICK_CHARGE;
							SIDEKICK_SetAttackTime(hook, gstate->time + (rnd() * 2.0f) + 2.0f);
						}
						else
						{
							hook->nAttackMode = ATTACKMODE_SIDEKICK_BACKAWAY;
							SIDEKICK_SetAttackTime(hook, gstate->time + (rnd() * 4.0f) + 3.0f);
						}
						break;
					}
					default:
					{
						break;
					}
					}
				}
			}
	}

	if (hook->nAttackMode != nPrevAttackMode)
	{
		PATHLIST_KillPath(hook->pPathList);
	}

	if (hook->nAttackMode == ATTACKMODE_SIDEKICK_STRAFE)
	{
		SIDEKICK_AttackWhileStrafing(self, pEnemy);
	}
	else
		if (hook->nAttackMode == ATTACKMODE_SIDEKICK_TAKECOVER)
		{
			SIDEKICK_TakeCover(self, pEnemy);
		}
		else
			if (hook->nAttackMode == ATTACKMODE_SIDEKICK_STAYSTILL)
			{
				SIDEKICK_StayStill(self, pEnemy);
			}
			else
				if (hook->nAttackMode == ATTACKMODE_SIDEKICK_BACKAWAY)
				{
					SIDEKICK_BackAway(self, pEnemy);
				}
				else
					if (hook->nAttackMode == ATTACKMODE_SIDEKICK_CHARGE)
					{
						SIDEKICK_Charge(self, pEnemy);
					}
					else
					{
						_ASSERTE(FALSE);
					}

	playerHook_t* pEnemyHook = AI_GetPlayerHook(pEnemy);
	if (pEnemy->flags & FL_CLIENT)
	{
		pEnemyHook = AI_GetPlayerHook(pEnemy->input_entity);
	}
	if (bCanSeeEnemy != NOT_VISIBLE && AI_IsJustFired(pEnemyHook))
	{
		if (pEnemy->flags & FL_CLIENT)
		{
			if (pEnemyHook->autoAim.ent == self)
			{
				SIDEKICK_Jump(self, pEnemy);
			}
		}
		else
		{
			if (rnd() < 0.25f || AI_IsEnemyLookingAtMe(self, pEnemy))
			{
				SIDEKICK_Jump(self, pEnemy);
			}
		}
	}

	SIDEKICK_SelectAnimationWhileAttacking(self);

	return TRUE;

}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_AttackPlayerUpdateAttackMovement
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_AttackPlayerUpdateAttackMovement(userEntity_t* self)
{
	_ASSERTE(self);
	//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	userEntity_t* pEnemy = self->enemy;
	_ASSERTE(AI_IsAlive(pEnemy));

	float fRange = AI_GetCurrentWeaponRange(self);
	if (fRange < SIDEKICK_MELEE_ATTACK_DISTANCE)
	{
		if (SIDEKICK_AttackPlayerUpdateMeleeAttackMovement(self) == FALSE)
		{
			return FALSE;
		}
	}
	else
	{
		// ranged attack weapon
		if (SIDEKICK_AttackPlayerUpdateRangeAttackMovement(self) == FALSE)
		{
			return FALSE;
		}
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_AttackPlayer
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_AttackPlayer(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	userEntity_t* pEnemy = self->enemy;
	if (!AI_IsAlive(pEnemy))
	{
		// killed the enemy, no longer in attack mode
		return;
	}

	AI_FaceTowardPoint(self, pEnemy->s.origin);

	int bOkToFight = TRUE;
	if (SIDEKICK_IsOkToFight(self))
	{
		weapon_t* pWeapon = (weapon_t*)self->curWeapon;
		if (!pWeapon || !SIDEKICK_CanUseWeapon(pWeapon))
		{
			SIDEKICK_ChooseBestWeapon(self);
			pWeapon = (weapon_t*)self->curWeapon;
			if (!pWeapon || !SIDEKICK_CanUseWeapon(pWeapon))
			{
				bOkToFight = FALSE;
			}
		}
	}
	else
	{
		bOkToFight = FALSE;
	}

	if (bOkToFight == FALSE)
	{
		// try to evade from fighting
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);

		//if(GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_SIDEKICK_EVADE))
		//{

		//}
		//else
		//{
		GOAL_ClearTasks(pCurrentGoal);
		AI_AddNewTaskAtFront(self, TASKTYPE_SIDEKICK_EVADE, pEnemy);
		return;
		//}
	}

	if (SIDEKICK_AttackPlayerUpdateAttackMovement(self) == FALSE)
	{
		return;
	}

	if (AI_GetCurrentWeaponRange(self) >= SIDEKICK_MELEE_ATTACK_DISTANCE &&
		SIDEKICK_IsSquadmateBlockingShot(self, pEnemy))
	{
		return;
	}

	SIDEKICK_UseCurrentWeapon(self);
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_AttackPlayerThink
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_AttackPlayerThink(userEntity_t* self)
{
	_ASSERTE(self);

	if (!AI_IsAlive(self))
	{
		AI_TaskThink(self);
		return;
	}

	playerHook_t* hook = AI_GetPlayerHook(self);

	userEntity_t* pEnemy = self->enemy;
	if (!AI_IsAlive(pEnemy))
	{
		// killed the enemy, no longer in attack mode
		AI_ZeroVelocity(self);
		SIDEKICK_SelectAnimationWhileAttacking(self);
		return;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
	if (!pCurrentGoal)
	{
		pCurrentGoal = AI_AddNewGoal(self, GOALTYPE_GENERALUSE);
	}
	_ASSERTE(pCurrentGoal);

	AI_DecreaseJustFired(hook);

	GOALTYPE nGoalType = GOAL_GetType(pCurrentGoal);
	if (nGoalType == GOALTYPE_GENERALUSE && GOAL_GetNumTasks(pCurrentGoal) > 0)
	{
		AI_FaceTowardPoint(self, pEnemy->s.origin);

		if (AI_GetCurrentWeaponRange(self) >= SIDEKICK_MELEE_ATTACK_DISTANCE &&
			SIDEKICK_IsSquadmateBlockingShot(self, pEnemy))
		{
			SIDEKICK_SelectOwnerBypassDirection(self, pEnemy);
			hook->nAttackMode = ATTACKMODE_SIDEKICK_STRAFE;
			SIDEKICK_AttackWhileStrafing(self, pEnemy);
		}
		else
		{
			SIDEKICK_UseCurrentWeapon(self);
		}

		AI_TaskThink(self);
	}
	else
	{
		SIDEKICK_AttackPlayer(self);
		AI_SetNextThinkTime(self, 0.1f);
	}

	_ASSERTE(self->nextthink > gstate->time);

	if (AI_IsJustFired(hook))
	{
		// end this state
		SIDEKICK_SetAttackTime(hook, gstate->time);
	}

}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_SetAttackPlayer
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SetAttackPlayer(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);

	if (!AI_IsAlive(self) || SIDEKICK_IsCarryingMikiko(hook))
	{
		return;
	}

	userEntity_t* pPlayer = AIINFO_GetPlayer();
	_ASSERTE(AI_IsAlive(pPlayer));

	AI_ClearTeam(self);

	hook->owner = NULL;
	hook->bAttackPlayer = TRUE;
	self->enemy = pPlayer;

	self->think = SIDEKICK_AttackPlayerThink;

	AI_SetStateAttacking(hook);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	GOALSTACK_ClearAllGoals(pGoalStack);
	AI_AddNewGoal(self, GOALTYPE_GENERALUSE);

	hook->strafe_dir = rand() % (STRAFE_DOWNRIGHT + 1);

	SIDEKICK_ChooseBestWeapon(self);
	SIDEKICK_SetAttackSequence(self);

	AI_SetNextThinkTime(self, 0.1f);

}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StopAttackPlayer
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StopAttackPlayer(userEntity_t* self)
{
	_ASSERTE(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	GOALSTACK_ClearAllGoals(pGoalStack);

	hook->bAttackPlayer = TRUE;
	self->enemy = NULL;

	SIDEKICK_FindOwner(self);

	if (hook->owner)
	{
		self->think = SIDEKICK_Think;
	}
	else
	{
		self->think = SIDEKICK_StopAttackPlayer;
	}

	AI_SetNextThinkTime(self, 0.1f);
}

// ----------------------------------------------------------------------------
//                          Test functions
// ----------------------------------------------------------------------------
void SIDEKICK_TestAttack(userEntity_t* self)
{
	userEntity_t* pSidekick = AIINFO_GetFirstSidekick();

	// find enemy, then attack
	userEntity_t* pFirstMonster = alist_FirstEntity(monster_list);
	if (pFirstMonster)
	{
		SIDEKICK_SetAttackMode(self, pFirstMonster);
	}
}

void SIDEKICK_TestAttackPlayer(userEntity_t* self)
{
	userEntity_t* pSidekick = AIINFO_GetFirstSidekick();

	if (pSidekick)
	{
		SIDEKICK_SetAttackPlayer(pSidekick);
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_DisableCommanding
// Description: 
// Note:        
//              
// ----------------------------------------------------------------------------
void SIDEKICK_DisableCommanding(userEntity_t* self)
{
	userEntity_t* pMikiko, * pSuperfly, * pLastSelectedSidekick;

	pMikiko = AIINFO_GetMikiko();
	pSuperfly = AIINFO_GetSuperfly();

	pLastSelectedSidekick = AIINFO_GetLastSelectedSidekick();

	if (pLastSelectedSidekick == pMikiko)
	{
		com->Sidekick_Update(self, TYPE_MIKIKO, SIDEKICK_STATUS_DEACTIVATE, 0);
	}
	else if (pLastSelectedSidekick == pSuperfly)
	{
		com->Sidekick_Update(self, TYPE_SUPERFLY, SIDEKICK_STATUS_DEACTIVATE, 0);
	}

}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_SwapCommanding
// Description: Swap command control between sidekicks
// Note:        If Mikiko is the current sidekick, swap control to Superfly
//              (and vice versa)
// ----------------------------------------------------------------------------
void Inventory_SetMode(userEntity_t* self, int nMode, qboolean bPerformAction);
void SIDEKICK_SetCommanding(userEntity_t* self, userEntity_t* pSK, int mode)
{
	if (mode == INV_MODE_SIDEKICK1)		// select superfly
	{
		Inventory_SetMode(self, INV_MODE_SIDEKICK1, FALSE);// SCG[1/24/00]: set the mode for the global inventory status
		AIINFO_SetLastSelectedSidekick(pSK);  // he's in command now
		com->Sidekick_Update(self, TYPE_SUPERFLY, SIDEKICK_STATUS_COMMANDING, 0); // update client
		SIDEKICK_CommandAcknowledge(pSK, SIDEKICK_COMMAND_COMMAND, true); // respond to given command

		gstate->CommandSelectorNext(self);
		gstate->CommandSelectorPrev(self);
	}
	else if (mode == INV_MODE_SIDEKICK2)	// select mikiko
	{
		Inventory_SetMode(self, INV_MODE_SIDEKICK2, FALSE);// SCG[1/24/00]: set the mode for the global inventory status
		AIINFO_SetLastSelectedSidekick(pSK);      // she's in command now
		com->Sidekick_Update(self, TYPE_MIKIKO, SIDEKICK_STATUS_COMMANDING, 0); // update client
		SIDEKICK_CommandAcknowledge(pSK, SIDEKICK_COMMAND_COMMAND, true); // respond to given command

		gstate->CommandSelectorNext(self);
		gstate->CommandSelectorPrev(self);
	}
}

void SIDEKICK_SwapCommanding(userEntity_t* self)
{
	// if mikiko is last selected sidekick,
	//	 if superfly exists and is alive,
	//     set superfly to last selected sidekick
	//     send status message to client of swap
	//   end
	// else
	//   if superfly is the last selected sidekick
	//     if mikiko exists and is alive
	//       set mikiko to last selected sidekick
	//       send status message to client of swap
	//     end
	//   else
	//     no sidekicks exist to swap between

	if (gstate->GetCvar("paused") > 0.0f)
		return;

	userEntity_t* pMikiko, * pSuperfly, * pLastSelectedSidekick;

	pMikiko = AIINFO_GetMikiko();														 // grab 'er
	pSuperfly = AIINFO_GetSuperfly();                          // grab 'im

	pLastSelectedSidekick = AIINFO_GetLastSelectedSidekick();	 // grab last in command

	if (!pLastSelectedSidekick) // no sidekick in command?  should have been set earlier.. 
	{
		// just to be sure, check for available sidekick and set as last selected

		if (pMikiko && AI_IsAlive(pMikiko))
		{
			/*
						Inventory_SetMode( self, INV_MODE_SIDEKICK2, FALSE );// SCG[1/24/00]: set the mode for the global inventory status
						AIINFO_SetLastSelectedSidekick(pMikiko);      // she's in command now
						com->Sidekick_Update(self,TYPE_MIKIKO,SIDEKICK_STATUS_COMMANDING,0); // update client
						SIDEKICK_CommandAcknowledge(pMikiko,SIDEKICK_COMMAND_COMMAND,true); // respond to given command

						gstate->CommandSelectorNext( self );
						gstate->CommandSelectorPrev( self );*/
			SIDEKICK_SetCommanding(self, pMikiko, INV_MODE_SIDEKICK2);
		}
		else if (pSuperfly && AI_IsAlive(pSuperfly))
		{
			/*
						Inventory_SetMode( self, INV_MODE_SIDEKICK1, FALSE );// SCG[1/24/00]: set the mode for the global inventory status
						AIINFO_SetLastSelectedSidekick(pSuperfly);  // he's in command now
						com->Sidekick_Update(self,TYPE_SUPERFLY,SIDEKICK_STATUS_COMMANDING,0); // update client
						SIDEKICK_CommandAcknowledge(pSuperfly,SIDEKICK_COMMAND_COMMAND,true); // respond to given command

						gstate->CommandSelectorNext( self );
						gstate->CommandSelectorPrev( self );*/
			SIDEKICK_SetCommanding(self, pSuperfly, INV_MODE_SIDEKICK1);
		}
	}
	else
	{
		if (pMikiko && pMikiko == pLastSelectedSidekick)  // mikiko exists and is last selected?
		{
			if (pSuperfly && AI_IsAlive(pSuperfly))        // superfly exists and is alive?
			{
				/*
								Inventory_SetMode( self, INV_MODE_SIDEKICK1, FALSE );// SCG[1/24/00]: set the mode for the global inventory status
								AIINFO_SetLastSelectedSidekick(pSuperfly);    // set superfly as commanding sidekick
								com->Sidekick_Update(self,TYPE_SUPERFLY,SIDEKICK_STATUS_COMMANDING,0); // update client
								SIDEKICK_CommandAcknowledge(pSuperfly,SIDEKICK_COMMAND_COMMAND,true); // respond to given command

								gstate->CommandSelectorNext( self );
								gstate->CommandSelectorPrev( self );*/
				SIDEKICK_SetCommanding(self, pSuperfly, INV_MODE_SIDEKICK1);
			}
			else
			{
				// there is no superfly but we still have to activate the sidekick command HUD just as normal
/*
				Inventory_SetMode( self, INV_MODE_SIDEKICK2, FALSE );// SCG[1/24/00]: set the mode for the global inventory status
				AIINFO_SetLastSelectedSidekick(pMikiko);      // she's in command now
				com->Sidekick_Update(self,TYPE_MIKIKO,SIDEKICK_STATUS_COMMANDING,0); // update client
				SIDEKICK_CommandAcknowledge(pMikiko,SIDEKICK_COMMAND_COMMAND,true); // respond to given command

				gstate->CommandSelectorNext( self );
				gstate->CommandSelectorPrev( self );*/
				SIDEKICK_SetCommanding(self, pMikiko, INV_MODE_SIDEKICK2);
			}
		}
		else
		{
			if (pSuperfly && pSuperfly == pLastSelectedSidekick) // superfly is last selected?
			{
				if (pMikiko && AI_IsAlive(pMikiko))               // mikiko exists and alive?
				{
					/*
										Inventory_SetMode( self, INV_MODE_SIDEKICK2, FALSE );// SCG[1/24/00]: set the mode for the global inventory status
										AIINFO_SetLastSelectedSidekick(pMikiko);         // set mikiko as commanding sidekick
										com->Sidekick_Update(self,TYPE_MIKIKO,SIDEKICK_STATUS_COMMANDING,0); // update client
										SIDEKICK_CommandAcknowledge(pMikiko,SIDEKICK_COMMAND_COMMAND,true); // respond to given command

										gstate->CommandSelectorNext( self );
										gstate->CommandSelectorPrev( self );*/
					SIDEKICK_SetCommanding(self, pMikiko, INV_MODE_SIDEKICK2);
				}
				else
				{
					// there is no mikiko but we still have to activate the sidekick command HUD just as normal
					SIDEKICK_SetCommanding(self, pSuperfly, INV_MODE_SIDEKICK1);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_Remove
// Description: Removes sidkekick and performs cleanup
// Note:		Utility function for SIDEKICK_ShutdownSuperfly and SIDEKICK_ShutdownMikiko
//
// ----------------------------------------------------------------------------
void SIDEKICK_Remove(userEntity_t* self)
{
	SIDEKICK_SmartResetState(self);
	playerHook_t* hook = AI_GetPlayerHook(self);

	self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_BOUNCE;

	self->die = NULL;
	self->prethink = NULL;
	self->postthink = NULL;
	self->think = NULL;

	if (hook->pGoals)
	{
		GOALSTACK_Delete(hook->pGoals);
		hook->pGoals = NULL;
	}

	if (hook->pScriptGoals)
	{
		GOALSTACK_Delete(hook->pScriptGoals);
		hook->pScriptGoals = NULL;
	}

	UNIQUEID_Remove(hook->szScriptName);
	free(hook->szScriptName);
	hook->szScriptName = NULL;

	alist_remove(self);
	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_ShutdownSuperfly
// Description: Removes Superfly and performs cleanup
//
// ----------------------------------------------------------------------------
void SIDEKICK_ShutdownSuperfly()
{
	userEntity_t* pSuperfly = AIINFO_GetSuperfly();

	_ASSERTE(pSuperfly);
	if (pSuperfly != NULL)
	{
		SIDEKICK_Remove(pSuperfly);
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_ShutdownMikiko
// Description: Removes Mikiko and performs cleanup
//
// ----------------------------------------------------------------------------
void SIDEKICK_ShutdownMikiko()
{
	userEntity_t* pMikiko = AIINFO_GetMikiko();

	_ASSERTE(pMikiko);
	if (pMikiko != NULL)
	{
		SIDEKICK_Remove(pMikiko);
	}
}

// ----------------------------------------------------------------------------
//
// Name:        trigger_superfly_spawn_use
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void trigger_superfly_spawn_use(userEntity_t* self, userEntity_t* other, userEntity_t* activator)
{
	SIDEKICK_Spawn(TYPE_SUPERFLY, self->s.origin, self->s.angles);

	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//
// Name:        trigger_mikiko_spawn_use
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void trigger_mikiko_spawn_use(userEntity_t* self, userEntity_t* other, userEntity_t* activator)
{
	SIDEKICK_Spawn(TYPE_MIKIKO, self->s.origin, self->s.angles);

	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//
// Name:        trigger_superfly_spawn
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void trigger_superfly_spawn(userEntity_t* self)
{
	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->use = trigger_superfly_spawn_use;
	gstate->SetModel(self, self->modelName);
	self->modelName = NULL;
	gstate->LinkEntity(self);

	CVector origin;
	origin.x = self->absmin.x + (self->absmax.x - self->absmin.x);
	origin.y = self->absmin.y + (self->absmax.y - self->absmin.y);
	origin.z = self->absmin.z + (self->absmax.z - self->absmin.z);
	self->s.origin = origin;
}

// ----------------------------------------------------------------------------
//
// Name:        trigger_mikiko_spawn
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void trigger_mikiko_spawn(userEntity_t* self)
{
	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->use = trigger_mikiko_spawn_use;
	gstate->SetModel(self, self->modelName);
	self->modelName = NULL;
	gstate->LinkEntity(self);

	CVector origin;
	origin.x = self->absmin.x + (self->absmax.x - self->absmin.x);
	origin.y = self->absmin.y + (self->absmax.y - self->absmin.y);
	origin.z = self->absmin.z + (self->absmax.z - self->absmin.z);
	self->s.origin = origin;
}

// ----------------------------------------------------------------------------
// NSS[3/6/00]:
// Name:        SIDEKICK_Stats
// Description: Displays the satistics for the sidekick.
// Input:userEntity_t *self(sidekick)
// Output:
// ----------------------------------------------------------------------------
void SIDEKICK_Stats(userEntity_t* self)
{
	playerHook_t* hook = AI_GetPlayerHook(self);
	if (hook)
	{
		weapon_t* pWeapon = (weapon_t*)self->curWeapon;
		if (pWeapon && pWeapon->winfo)
		{
			float Ammo, Ammo_Max;
			Ammo_Max = pWeapon->winfo->ammo_max;
			Ammo = pWeapon->ammo->count;
			AI_Dprintf("Health:%f Armor:%f Weapon:%s Ammo:%d AmmoMax:%d\n", self->health, self->armor_val, pWeapon->name, Ammo, Ammo_Max);
		}
		else
		{
			AI_Dprintf("Health:%f Armor:%f\n", self->health, self->armor_val);
		}
	}
}

// ----------------------------------------------------------------------------
// NSS[3/6/00]:
// Name:        SIDEKICK_Stats_SF
// Description: Get the statistics for superfly/mikikofly
// Input:userEntity_t *self(client)
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_Stats_SF(userEntity_t* self)
{
	userEntity_t* ent = AIINFO_GetSuperfly();
	if (!ent)
		ent = AIINFO_GetMikikofly();
	if (ent)
	{
		SIDEKICK_Stats(ent);
	}
}


// ----------------------------------------------------------------------------
// NSS[3/6/00]:
// Name:        SIDEKICK_Stats_MK
// Description: Get the statistics for mikiko
// Input:userEntity_t *self(client)
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_Stats_MK(userEntity_t* self)
{
	userEntity_t* ent = AIINFO_GetMikiko();
	if (ent)
	{
		SIDEKICK_Stats(ent);
	}
}


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_sidekick_register_func()
{
	gstate->RegisterFunc("SIDEKICK_FindOwnerThink", SIDEKICK_FindOwnerThink);
	gstate->RegisterFunc("SIDEKICK_NitroDeath", SIDEKICK_NitroDeath);
	gstate->RegisterFunc("SIDEKICK_Think", SIDEKICK_Think);
	gstate->RegisterFunc("SIDEKICK_AttackPlayerThink", SIDEKICK_AttackPlayerThink);
	gstate->RegisterFunc("SIDEKICK_StopAttackPlayer", SIDEKICK_StopAttackPlayer);
	gstate->RegisterFunc("SIDEKICK_Pain", SIDEKICK_Pain);
	gstate->RegisterFunc("SIDEKICK_StartDie", SIDEKICK_StartDie);
	gstate->RegisterFunc("SIDEKICK_PreThink", SIDEKICK_PreThink);
	gstate->RegisterFunc("SIDEKICK_PostThink", SIDEKICK_PostThink);
	gstate->RegisterFunc("SIDEKICK_InAttackRange", SIDEKICK_InAttackRange);
	gstate->RegisterFunc("AI_SaveSidekick", AI_SaveSidekick);
	//	gstate->RegisterFunc("AI_LoadSidekick",AI_LoadSidekick);
	gstate->RegisterFunc("AI_LoadSidekick", AI_LoadSidekick);
	gstate->RegisterFunc("Sidekick_Touch", Sidekick_Touch);
	gstate->RegisterFunc("SIDEKICK_RipgunThink", SIDEKICK_RipgunThink);

	gstate->RegisterFunc("trigger_superfly_spawn_use", trigger_superfly_spawn_use);
	gstate->RegisterFunc("trigger_mikiko_spawn_use", trigger_mikiko_spawn_use);
}
