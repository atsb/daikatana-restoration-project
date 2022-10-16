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
#include <stdlib.h>
//#include "dk_std.h"fDamage
#include "world.h"
#include "ai_utils.h"
#include "ai.h"
#include "ai_think.h"
#include "ai_func.h"
#include "ai_frames.h"
#include "Sidekick.h"
#include "ai_move.h"


#define CHECK_STACKOVERFLOW

bool AI_IsTakeCoverType( playerHook_t *hook );

// ----------------------------------------------------------------------------

#ifdef CHECK_STACKOVERFLOW
void AI_OrientToFloor( userEntity_t *self );
void AI_ClearExecutedTasks( playerHook_t *hook )
{
    hook->taskTracker.Clear();
}

void AI_IncrementExecutedTask( playerHook_t *hook, TASKTYPE nTaskType )
{
    hook->taskTracker.AddTask( nTaskType );
}

int AI_GetNumExecutedTask( playerHook_t *hook, TASKTYPE nTaskType )
{
    _ASSERTE( nTaskType >= 0 && nTaskType < TASKTYPE_MAX );

    return hook->taskTracker.GetNumForTaskType( nTaskType );
}

#else

void AI_ClearExecutedTasks( playerHook_t *hook )
{
}

void AI_IncrementExecutedTask( playerHook_t *hook, TASKTYPE nTaskType )
{
}

int AI_GetNumExecutedTask( playerHook_t *hook, TASKTYPE nTaskType )
{
    return 0;
}

#endif CHECK_STACKOVERFLOW


// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Name:		AI_StartTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartTask( userEntity_t *self, TASK_PTR pTask )
{
	_ASSERTE( self );
	_ASSERTE( pTask );
    playerHook_t *hook = AI_GetPlayerHook( self );

    TASKTYPE nCurrentTaskType = TASK_GetType( pTask );

#ifdef CHECK_STACKOVERFLOW
    AI_IncrementExecutedTask( hook, nCurrentTaskType );
    if ( AI_GetNumExecutedTask( hook, nCurrentTaskType ) > 5 )
    {
        // too many calls in a single update, stackoverflow is imminent, 
        // so remove current goal
        AI_RemoveCurrentGoal( self );
        return;
    }
#endif CHECK_STACKOVERFLOW

    // check if this entity has a valid goal
    int bStartTask = TRUE;
    if ( AI_IsAlive( self ) )
    {
        GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
        if ( pGoalStack )
        {
            GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
            if ( !pCurrentGoal )
            {
                bStartTask = FALSE;
            }
        }
        else
        {
            bStartTask = FALSE;
        }
    }
    else
    {
        bStartTask = FALSE;
    }

    if ( bStartTask == FALSE && !hook->bInScriptMode)
    {
        return;
    }

	if( gstate->bCinematicPlaying && !hook->bInScriptMode )
	{
		return;
	}

	switch ( nCurrentTaskType )
	{
		case TASKTYPE_IDLE:
		{
			AI_StartIdle( self );
			break;
		}
		case TASKTYPE_WANDER:
		{
			AI_StartWander( self );
			break;
		}
		case TASKTYPE_ATTACK:
		{
			AI_StartAttack( self );
			break;
		}
		case TASKTYPE_MOVE:
		{
			break;
		}
		case TASKTYPE_CHASE:
		{
	
			AI_StartChase( self );
			break;
		}
		case TASKTYPE_CHASEATTACK:
		{
			AI_StartChaseAttack( self );
			break;
		}
		case TASKTYPE_CHASEATTACKSTRAFING:
		{
			AI_StartChaseAttackStrafing( self );
			break;
		}
		case TASKTYPE_FOLLOWWALKING:
		{
			AI_StartFollowWalking( self );
			break;
		}
		case TASKTYPE_FOLLOWRUNNING:
		{
			AI_StartFollowRunning( self );
			break;
		}
        case TASKTYPE_FOLLOWSIDEKICKWALKING:
        {
            AI_StartFollowSidekickWalking( self );
            break;
        }
        case TASKTYPE_FOLLOWSIDEKICKRUNNING:
        {
            AI_StartFollowSidekickRunning( self );
            break;
        }
		case TASKTYPE_JUMPUP:
		{
			playerHook_t *hook = AI_GetPlayerHook( self );
			if ( hook->fnStartJumpUp )
			{
				hook->fnStartJumpUp( self );
			}
			break;
		}
		case TASKTYPE_JUMPFORWARD:
		{
			playerHook_t *hook = AI_GetPlayerHook( self );
			if ( hook->fnStartJumpForward )
			{
				hook->fnStartJumpForward( self );
			}
			break;
		}
		case TASKTYPE_SWITCH:
		{
			break;
		}
		case TASKTYPE_HIDE:
		{
			AI_StartHide( self );
			break;
		}
		case TASKTYPE_SNIPE:
		{
			AI_StartSnipe( self );
			break;
		}
		case TASKTYPE_AMBUSH:
		{
			break;
		}
		case TASKTYPE_RUNAWAY:
		{
			AI_StartRunAway( self );
			break;
		}
        case TASKTYPE_BRIEFCOVER:
        {
            AI_StartBriefCover( self );
            break;
        }
		case TASKTYPE_MOVEAWAY:
		{
			AI_StartMoveAway( self );
			break;
		}
		case TASKTYPE_WAITFORTRAINTOCOME:
		{
			AI_StartWaitForTrainToCome( self );
			break;
		}
		case TASKTYPE_WAITFORTRAINTOSTOP:
		{
			AI_StartWaitForTrainToStop( self );
			break;
		}
		case TASKTYPE_WAITFORPLATFORMTOCOME:
		{
			break;
		}
		case TASKTYPE_WAITFORPLATFORMTOSTOP:
		{
			break;
		}
		case TASKTYPE_WAITFORDOORTOOPEN:
		{
			break;
		}
		case TASKTYPE_WAITFORDOORTOCLOSE:
		{
			break;
		}
		case TASKTYPE_MOVEINTOPLATFORM:
		{
			break;
		}
		case TASKTYPE_MOVEOUTOFPLATFORM:
		{
			break;
		}
		case TASKTYPE_PATROL:
		{
			AI_StartPatrol( self );
			break;
		}
		case TASKTYPE_PAIN:
		{
			break;
		}
		case TASKTYPE_DIE:
		{
			break;
		}
		case TASKTYPE_FREEZE:
		{
			break;
		}
		case TASKTYPE_MOVETOLOCATION:
		{
			AI_StartMoveToLocation( self );
			break;
		}
		case TASKTYPE_MOVETOEXACTLOCATION:
		{
			AI_StartMoveToExactLocation( self );
			break;
		}
        case TASKTYPE_WALKTOLOCATION:
        {
            AI_StartWalkToLocation( self );
            break;
        }
		case TASKTYPE_GOINGAROUNDOBSTACLE:
		{
			AI_StartGoingAroundObstacle( self );
			break;
		}
		case TASKTYPE_FLYTOLOCATION:
		{
			AI_StartFlyToLocation( self );
			break;
		}
		case TASKTYPE_SWOOPATTACK:
		{
			AI_StartSwoopAttack( self );
			break;
		}
		case TASKTYPE_TOWARDINTERMEDIATEPOINT:
		{
			AI_StartTowardIntermediatePoint( self );
			break;
		}
		case TASKTYPE_TOWARDINTERMEDIATEPOINT2:
		{
			AI_StartTowardIntermediatePoint2( self );
			break;
		}
		case TASKTYPE_AIRRANGEDATTACK:
		{
			AI_StartAirRangedAttack( self );
			break;
		}
		case TASKTYPE_AIRMELEEATTACK:
		{
			AI_StartAirMeleeAttack( self );
			break;
		}
		case TASKTYPE_SWIMTOLOCATION:
		{
			AI_StartSwimToLocation( self );
			break;
		}
		case TASKTYPE_FACEANGLE:
		{
			AI_StartFaceAngle( self );
			break;
		}
		case TASKTYPE_WAIT:
		{
			AI_StartWait( self );
			break;
		}
		case TASKTYPE_WAITFORNOCOLLISION:
		{
			AI_StartWaitForNoCollision( self );
			break;
		}
		case TASKTYPE_BACKUPALLATTRIBUTES:
		{
			AI_StartBackupAllAttributes( self );
			break;
		}
		case TASKTYPE_RESTOREALLATTRIBUTES:
		{
			AI_StartRestoreAllAttributes( self );
			break;
		}
		case TASKTYPE_MODIFYTURNATTRIBUTE:
		{
			AI_StartModifyTurnAttribute( self );
			break;
		}
		case TASKTYPE_MODIFYRUNATTRIBUTE:
		{
			AI_StartModifyRunAttribute( self );
			break;
		}
		case TASKTYPE_PLAYANIMATION:
		{
			AI_StartPlayAnimation( self );
			break;
		}
		case TASKTYPE_SETIDLEANIMATION:
		{
			AI_StartSetIdleAnimation( self );
			break;
		}
		case TASKTYPE_RANDOMWANDER:
		{
			AI_StartRandomWander (self);
			break;
		}
		case TASKTYPE_STARTUSINGWALKSPEED:
		{
			AI_StartUsingWalkSpeed( self );
			break;
		}
		case TASKTYPE_STARTUSINGRUNSPEED:
		{
			AI_StartUsingRunSpeed( self );
			break;
		}
		case TASKTYPE_ACTIVATESWITCH:
		{
			AI_StartActivateSwitch( self );
			break;
		}
		case TASKTYPE_MOVETOENTITY:
		{
			AI_StartMoveToEntity( self );
			break;
		}
        case TASKTYPE_MOVETOOWNER:
        {
            AI_StartMoveToOwner( self );
            break;
        }
		case TASKTYPE_STAND:
		{
			AI_StartStand( self );
			break;
		}
		case TASKTYPE_TAKECOVER:
		{
			AI_StartTakeCover( self );
			break;
		}
		case TASKTYPE_TAKECOVER_ATTACK:
		{
			AI_StartTakeCover_Attack(self);
			break;
		}
		case TASKTYPE_STAY:
		{
			AI_StartStay( self );
			break;
		}
		case TASKTYPE_FLYINGAROUNDOBSTACLE:
		{
			AI_StartFlyingAroundObstacle( self );
			break;
		}
		case TASKTYPE_PATHFOLLOW:
		{
			AI_StartPathFollow( self );
			break;
		}
		case TASKTYPE_DODGE:
		{
			AI_StartDodge( self );
			break;
		}
		case TASKTYPE_SIDESTEP:
		{
			AI_StartSideStep( self );
			break;
		}
		case TASKTYPE_GOINGAROUNDENTITY:
		{
			AI_StartGoingAroundEntity( self );
			break;
		}
		case TASKTYPE_MOVETOSNIPE:
		{
			AI_StartMoveToSnipe( self );
			break;
		}
		case TASKTYPE_STRAFE:
		{
			AI_StartStrafe( self );
			break;
		}
		case TASKTYPE_CHASEEVADE:
		{
			AI_StartChaseEvade( self );
			break;
		}
		case TASKTYPE_TAKEOFF:
		{
			AI_StartTakeOff( self );
			break;
		}
		case TASKTYPE_DROP:
		{
			AI_StartDrop( self );
			break;
		}
		case TASKTYPE_CHASESIDESTEPLEFT:
		{
			AI_StartChaseSideStepLeft( self );
			break;
		}
		case TASKTYPE_CHASESIDESTEPRIGHT:
		{
			AI_StartChaseSideStepRight( self );
			break;
		}
		case TASKTYPE_CHARGETOWARDENEMY:
		{
			AI_StartChargeTowardEnemy( self );
			break;
		}
		case TASKTYPE_RETREATTOOWNER:
		{
			AI_StartRetreatToOwner( self );
			break;
		}
		case TASKTYPE_SEQUENCETRANSITION:
		{
			AI_StartSequenceTransition( self );
			break;
		}
		case TASKTYPE_FINISHCURRENTSEQUENCE:
		{
			AI_StartFinishCurrentSequence( self );
			break;
		}
		case TASKTYPE_JUMPTOWARDPOINT:
		{
			AI_StartJumpTowardPoint( self );
			break;
		}
		case TASKTYPE_SHOTCYCLERJUMP:
		{
			AI_StartShotCyclerJump( self );
			break;
		}
		case TASKTYPE_MOVEUNTILVISIBLE:
		{
			AI_StartMoveUntilVisible( self );
			break;
		}
		case TASKTYPE_USEPLATFORM:
		{
			AI_StartUsePlatform( self );
			break;
		}
		case TASKTYPE_MOVEDOWN:
		{
			AI_StartMoveDown( self );
			break;
		}
		case TASKTYPE_USE:
		{
			AI_StartUse( self );
			break;
		}
		case TASKTYPE_COWER:
		{
			AI_StartCower( self );
			break;
		}
		case TASKTYPE_UPLADDER:
		{
			AI_StartUpLadder( self );
			break;
		}
		case TASKTYPE_DOWNLADDER:
		{
			AI_StartDownLadder( self );
			break;
		}
		case TASKTYPE_WAITONLADDERFORNOCOLLISION:
		{
			AI_StartWaitOnLadderForNoCollision( self );
			break;
		}
        case TASKTYPE_PLAYSOUNDTOEND:
        {
            AI_StartPlaySoundToEnd( self );
            break;
        }
        case TASKTYPE_TELEPORT:
        {
            AI_StartTeleport( self );
            break;
        }
        case TASKTYPE_PLAYANIMATIONANDSOUND:
        {
            AI_StartPlayAnimationAndSound( self );
            break;
        }
        case TASKTYPE_PLAYSOUND:
        {
            AI_StartPlaySound( self );
            break;
        }
        case TASKTYPE_MOVETOENTITYUNTILVISIBLE:
        {
            AI_StartMoveToEntityUntilVisible( self );
            break;
        }

		case TASKTYPE_ACTION_PLAYANIMATION:
		{
			AI_StartActionPlayAnimation( self );
			break;
		}
		case TASKTYPE_ACTION_PLAYPARTIALANIMATION:
		{
			AI_StartActionPlayPartialAnimation( self );
			break;
		}
		case TASKTYPE_ACTION_PLAYSOUND:
		{
			AI_StartActionPlaySound( self );
			break;
		}
		case TASKTYPE_ACTION_SPAWN:
		{
			AI_StartActionSpawn( self );
			break;
		}
		case TASKTYPE_ACTION_SETMOVINGANIMATION:
		{
			AI_StartActionSetMovingAnimation( self );
			break;
		}
		case TASKTYPE_ACTION_SETSTATE:
		{
			AI_StartActionSetState( self );
			break;
		}
		case TASKTYPE_ACTION_DIE:
		{
			AI_StartActionDie( self );
			break;
		}
		case TASKTYPE_ACTION_STREAMSOUND:
		{
			AI_StartActionStreamSound( self );
			break;
		}
		case TASKTYPE_ACTION_SENDMESSAGE:
		{
			AI_StartActionSendMessage( self );
			break;
		}
		case TASKTYPE_ACTION_SENDURGENTMESSAGE:
		{
			AI_StartActionSendUrgentMessage( self );
			break;
		}
		case TASKTYPE_ACTION_CALL:
		{
			AI_StartActionCall( self );
			break;
		}
		case TASKTYPE_ACTION_RANDOMSCRIPT:
		{
			AI_StartActionRandomScript( self );
			break;
		}
		case TASKTYPE_ACTION_COMENEAR:
		{
			AI_StartActionComeNear( self );
			break;
		}
		case TASKTYPE_ACTION_REMOVE:
		{
			AI_StartActionRemove( self );
			break;
		}
		case TASKTYPE_ACTION_LOOKAT:
		{
			AI_StartActionLookAt( self );
			break;
		}
		case TASKTYPE_ACTION_STOPLOOK:
		{
			AI_StartActionStopLook( self );
			break;
		}
        case TASKTYPE_ACTION_ATTACK:
        {
            AI_StartActionAttack( self );
            break;
        }
        case TASKTYPE_ACTION_PRINT:
        {
            AI_StartActionPrint( self );
            break;
        }
        case TASKTYPE_WAITUNTILNOOBSTRUCTION:
        {
            AI_StartWaitUntilNoObstruction( self );
            break;
        }
        case TASKTYPE_MOVETORETREATNODE:
        {
            AI_StartMoveToRetreatNode( self );
            break;
        }
        case TASKTYPE_STOPENTITY:
        {
            AI_StartStopEntityTask( self );
            break;
        }

		// monster specific tasks should be placed here, everything else above
		case TASKTYPE_SPECIAL:
		{
			break;
		}
		case TASKTYPE_INMATER_GOTOPRISONER:
		{
			INMATER_StartGotoPrisoner( self );
			break;
		}
		case TASKTYPE_INMATER_WAITATPRISONER:
		{
			INMATER_StartWaitAtPrisoner( self );
			break;
		}
/*
		case TASKTYPE_INMATER_KILLPRISONER:
		{
			INMATER_StartKillPrisoner( self );
			break;
		}
*/
		case TASKTYPE_PRISONER_WANDER:
		{
			PRISONER_StartWander( self );
			break;
		}
		case TASKTYPE_PRISONER_COWER:
		{
			PRISONER_StartCower( self );
			break;
		}
/*
		case TASKTYPE_PRISONER_INMATERHASITINME:
		{
			PRISONER_StartInmaterHasItInMe( self );
			break;
		}
*/
		case TASKTYPE_SLUDGEMINION_SCOOP:
		{
			SLUDGEMINION_StartScoop( self );
			break;
		}
		case TASKTYPE_SLUDGEMINION_DUMP:
		{
			SLUDGEMINION_StartDump( self );
			break;
		}
		case TASKTYPE_BUBOID_GETOUTOFCOFFIN:
		{
			BUBOID_StartGetOutOfCoffin( self );
			break;
		}
		case TASKTYPE_BUBOID_MELT:
		{
			BUBOID_StartMelt( self );
			break;
		}
		case TASKTYPE_BUBOID_MELTED:
		{
			BUBOID_StartMelted( self );
			break;
		}
		case TASKTYPE_BUBOID_UNMELT:
		{
			BUBOID_StartUnmelt( self );
			break;
		}
		case TASKTYPE_CAMBOT_PATHFOLLOW://<nss>
		{
			CAMBOT_StartPathFollow( self );
			break;
		}
		case TASKTYPE_CAMBOT_FOLLOWPLAYER://<nss>
		{
			CAMBOT_StartFollowPlayer( self );
			break;
		}
		case TASKTYPE_THUNDERSKEET_HOVER://<nss>
		{
			THUNDERSKEET_StartHover( self );
			break;
		}
		case TASKTYPE_THUNDERSKEET_DARTTOWARDENEMY://<nss>
		{
			THUNDERSKEET_StartDartTowardEnemy( self );
			break;
		}
		case TASKTYPE_THUNDERSKEET_ATTACK://<nss>
		{
			THUNDERSKEET_Begin_Attack( self );
			break;
		}
		case TASKTYPE_SKEETER_HOVER://<nss>
		{
			SKEETER_StartHover( self );
			break;
		}
		case TASKTYPE_SKEETER_DARTTOWARDENEMY://<nss>
		{
			SKEETER_StartDartTowardEnemy( self );
			break;
		}
		case TASKTYPE_SKEETER_ATTACK://<nss>
		{
			SKEETER_Begin_Attack( self );
			break;
		}
		case TASKTYPE_PSYCLAW_JUMPUP:
		{
			PSYCLAW_StartJumpUp( self );
			break;
		}
		
		///////////////////FROG///////////////////
		//   We are messy little coders no?     //
		//////////////////////////////////////////
		case TASKTYPE_FROG_ATTACK_THINK://<nss>
		{
			FROG_Start_Attack_Think( self );
			break;
		}
		case TASKTYPE_FROG_CHASE://<nss>
		{
			FROG_Start_Chase( self );
			break;
		}
		case TASKTYPE_FROG_JUMP://<nss>
		{
			FROG_Start_Jump( self );
			break;
		}
		case TASKTYPE_FROG_ATTACK://<nss>
		{
			FROG_Begin_Attack( self );
			break;
		}
		////////////////CHAINGANG/////////////////
		//////////////////////////////////////////
		case TASKTYPE_CHAINGANG_GROUND://<nss>
		{
			CHAINGANG_StartGround( self );
			break;
		}
		case TASKTYPE_CHAINGANG_INITIATE_AIR://<nss>
		{
			CHAINGANG_StartInitiateAir( self );
			break;
		}
		case TASKTYPE_CHAINGANG_INITIATE_GROUND://<nss>
		{
			CHAINGANG_StartInitiateGround( self );
			break;
		}
		case TASKTYPE_CHAINGANG_AIR://<nss>
		{
			CHAINGANG_StartAir( self );
			break;
		}
		case TASKTYPE_CHAINGANG_ATTACK://<nss>
		{
			CHAINGANG_Begin_Attack( self );
			break;
		}
		case TASKTYPE_CHAINGANG_CHASE://<nss>
		{
			CHAINGANG_StartChase( self );
			break;
		}
		case TASKTYPE_CHAINGANG_MOVETOLOCATION://<nss>
		{
			CHAINGANG_StartMoveToLocation( self );
			break;
		}
		case TASKTYPE_CHAINGANG_DODGE://<nss>
		{
			CHAINGANG_StartDodge( self );
			break;
		}
		/////////////////GRIFFON//////////////////
		//Jesus we need to clean this area up!  //
		//////////////////////////////////////////
		case TASKTYPE_GRIFFON_ATTACK_THINK://<nss>
		{
			GRIFFON_Start_Attack_Think( self );
			break;
		}
		case TASKTYPE_GRIFFON_CHASE://<nss>
		{
			GRIFFON_StartChase( self );
			break;
		}
		case TASKTYPE_GRIFFON_JUMP://<nss>
		{
			GRIFFON_Start_Jump( self );
			break;
		}
		case TASKTYPE_GRIFFON_ATTACK://<nss>
		{
			GRIFFON_Begin_Attack( self );
			break;
		}
		case TASKTYPE_GRIFFON_FLYAWAY://<nss>
		{
			GRIFFON_StartFlyAway( self );
			break;
		}
		case TASKTYPE_GRIFFON_GROUND://<nss>
		{
			GRIFFON_StartGround( self );
			break;
		}
		case TASKTYPE_GRIFFON_LAND://<nss>
		{
			GRIFFON_StartLand( self );
			break;
		}

		/////////////////HARPY//////////////////
		//////////////////////////////////////////
		case TASKTYPE_HARPY_GROUND://<nss>
		{
			HARPY_StartGround( self );
			break;
		}
		case TASKTYPE_HARPY_INITIATE_AIR://<nss>
		{
			HARPY_StartInitiateAir( self );
			break;
		}
		case TASKTYPE_HARPY_INITIATE_GROUND://<nss>
		{
			HARPY_StartInitiateGround( self );
			break;
		}
		case TASKTYPE_HARPY_AIR://<nss>
		{
			HARPY_StartAir( self );
			break;
		}
		case TASKTYPE_HARPY_ATTACK://<nss>
		{
			HARPY_Begin_Attack( self );
			break;
		}
		case TASKTYPE_HARPY_CHASE://<nss>
		{
			HARPY_StartChase( self );
			break;
		}
		case TASKTYPE_HARPY_MOVETOLOCATION://<nss>
		{
			HARPY_StartMoveToLocation( self );
			break;
		}
		case TASKTYPE_HARPY_DODGE://<nss>
		{
			HARPY_StartDodge( self );
			break;
		}

		/////////////////MEDUSA/////////////////
		case TASKTYPE_MEDUSA_CHASE://<nss>
		{
			MEDUSA_StartChase( self );
			break;
		}
		case TASKTYPE_MEDUSA_RATTLE://<nss>
		{
			MEDUSA_StartRattle( self );
			break;
		}
		case TASKTYPE_MEDUSA_GAZE://<nss>
		{
			MEDUSA_StartGaze( self );
			break;
		}
		case TASKTYPE_MEDUSA_STOPGAZING://<nss>
		{
			MEDUSA_StartStopGazing( self );
			break;
		}

		/////////////////DRAGON//////////////////
		case TASKTYPE_DRAGON_HOVER://<nss>
		{
			DRAGON_StartHover( self );
			break;
		}
		case TASKTYPE_DRAGON_ATTACK_THINK://<nss>
		{
			break;
		}
		case TASKTYPE_DRAGON_ATTACK://<nss>
		{
			DRAGON_BeginAttack( self );
			break;
		}
		case TASKTYPE_DRAGON_FLYAWAY://<nss>
		{
			DRAGON_StartFlyAway( self );
			break;
		}
		////////////////GHOST///////////////////
		case TASKTYPE_GHOST_WAKEUP:
		{
			Ghost_Start_WakeUp( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_GHOST_CHASE:
		{
			AI_StartChase( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_GHOST_SPIRAL:
		{
			Ghost_StartSpiral_Upward( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_GHOST_ATTACK:
		{
			Ghost_Begin_Attack( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_GHOST_DEATH:
		{
			Ghost_Begin_Death( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		/////////////////DOOMBAT//////////////////
		case TASKTYPE_DOOMBAT_HOVER://<nss>
		{
			DOOMBAT_StartHover( self );
			break;
		}
		case TASKTYPE_DOOMBAT_CHASE://<nss>
		{
			DOOMBAT_StartChase( self );
			break;
		}
		case TASKTYPE_DOOMBAT_ATTACK_THINK://<nss>
		{
			DOOMBAT_Start_Attack_Think( self );
			break;
		}
		case TASKTYPE_DOOMBAT_KAMAKAZI://<nss>
		{
			DOOMBAT_StartKamakazi( self );
			break;
		}
		case TASKTYPE_DOOMBAT_ATTACK://<nss>
		{
			DOOMBAT_Begin_Attack( self );
			break;
		}
		case TASKTYPE_DOOMBAT_FLYAWAY://<nss>
		{
			DOOMBAT_StartFlyAway( self );
			break;
		}
		/////////////////NHARRE//////////////////
		case TASKTYPE_NHARRE_TELEPORT:// NSS[12/13/99]:
		{
			NHARRE_StartTeleport(self);
			break;
		}
		/////////////////WYNDRAX//////////////////
		case TASKTYPE_WYNDRAX_CHARGE_UP://<nss>
		{
			WYNDRAX_Start_Charge_Up( self );
			break;
		}
		case TASKTYPE_WYNDRAX_FIND_WISPMASTER://<nss>
		{
			WYNDRAX_Start_Find_WispMaster( self );
			break;
		}
		case TASKTYPE_WYNDRAX_COLLECT_WISPS://<nss>
		{
			WYNDRAX_Start_Collect_Wisps( self );
			break;
		}
		case TASKTYPE_THUNDERSKEET_FLYAWAY://<nss>
		{
			THUNDERSKEET_StartFlyAway( self );
			break;
		}
		case TASKTYPE_SKEETER_FLYAWAY://<nss>
		{
			SKEETER_StartFlyAway( self );
			break;
		}
		case TASKTYPE_SKEETER_PREHATCH://<nss>
		{
			SKEETER_StartPreHatch( self );
			break;
		}
		case TASKTYPE_SKEETER_HATCH:
		{
			SKEETER_StartHatch( self );
			break;
		}
		case TASKTYPE_LYCANTHIR_RESURRECT_WAIT:
		{
			LYCANTHIR_StartResurrectWait( self );
			break;
		}
		case TASKTYPE_LYCANTHIR_RESURRECT:
		{
			LYCANTHIR_StartResurrect( self );
			break;
		}
		case TASKTYPE_BUBOID_RESURRECT_WAIT:
		{
			BUBOID_StartResurrectWait( self );
			break;
		}
		case TASKTYPE_BUBOID_RESURRECT:
		{
			BUBOID_StartResurrect( self );
			break;
		}
		case TASKTYPE_COLUMN_AWAKEN_WAIT:
		{
			COLUMN_StartAwakenWait( self );
			break;
		}
		case TASKTYPE_COLUMN_AWAKEN:
		{
			COLUMN_StartAwaken( self );
			break;
		}
		case TASKTYPE_ROTWORM_JUMPSPRING://<nss>
		{
			ROTWORM_StartJumpSpring( self );
			break;
		}
		case TASKTYPE_ROTWORM_JUMPFLY:
		{
			ROTWORM_StartJumpFly( self );
			break;
		}
		case TASKTYPE_ROTWORM_JUMPBITE://<nss>
		{
			ROTWORM_StartJumpBite( self );
			break;
		}
		case TASKTYPE_DEATHSPHERE_MOVETOLOCATION:
		{
			DEATHSPHERE_StartMove( self );
			break;
		}
		case TASKTYPE_DEATHSPHERE_CHARGEWEAPON:
		{
			DEATHSPHERE_StartChargeWeapon( self );
			break;
		}
		case TASKTYPE_PROTOPOD_WAIT_TO_HATCH:
		{
			PROTOPOD_StartWaitToHatch(self);
			break;
		}
		case TASKTYPE_PROTOPOD_HATCH:
		{
			PROTOPOD_StartHatch(self);
			break;
		}
		case TASKTYPE_SKINNYWORKER_HIDE:
		{
			SKINNYWORKER_StartHide( self );
			break;
		}
		case TASKTYPE_SKINNYWORKER_BACKWALL:
		{
			SKINNYWORKER_StartBackWall( self );
			break;
		}
		case TASKTYPE_SKINNYWORKER_LOOKOUT:
		{
			SKINNYWORKER_StartLookOut( self );
			break;
		}


		case TASKTYPE_MISHIMAGUARD_RELOAD :
		{
			MISHIMAGUARD_StartReload( self );
			break;
		}
        case TASKTYPE_SMALLSPIDER_RUNAWAY:
        {
            SMALLSPIDER_StartRunAway( self );
            break;
        }


		case TASKTYPE_BOT_DIE:
		{
			break;
		}
		case TASKTYPE_BOT_ROAM:
		{
			BOT_StartRoam( self );
			break;
		}
		case TASKTYPE_BOT_PICKUPITEM:
		{
			BOT_StartPickupItem( self );
			break;
		}
		case TASKTYPE_BOT_ENGAGEENEMY:
		{
			BOT_StartEngageEnemy( self );
			break;
		}
		case TASKTYPE_BOT_CHASEATTACK:
		{
			BOT_StartChaseAttack( self );
			break;
		}
		case TASKTYPE_BOT_STRAFELEFT:
		{
			BOT_StartStrafeLeft( self );
			break;
		}
		case TASKTYPE_BOT_STRAFERIGHT:
		{
			BOT_StartStrafeRight( self );
			break;
		}
		case TASKTYPE_BOT_CHARGETOWARDENEMY:
		{
			BOT_StartChargeTowardEnemy( self );
			break;
		}

		case TASKTYPE_SIDEKICK_DIE:
		{
			break;
		}
        case TASKTYPE_SIDEKICK_PICKUPITEM://<nss>
        {
            SIDEKICK_StartPickupItem( self );
            break;
        }
		case TASKTYPE_SIDEKICK_USEITEM: //<nss>
		{
			SIDEKICK_StartUseItem(self);
			break;
		}
        case TASKTYPE_SIDEKICK_EVADE://<nss>
        {
            SIDEKICK_StartEvade( self );
            break;
        }
        case TASKTYPE_SIDEKICK_RANDOMWALK://<nss>
		{
			SIDEKICK_StartRandomWalk( self );
			break;
		}
        case TASKTYPE_SIDEKICK_LOOKAROUND:
		{
			SIDEKICK_StartLookAround( self );
			break;
		}
        case TASKTYPE_SIDEKICK_LOOKUP:
		{
			SIDEKICK_StartLookUp( self );
			break;
		}
        case TASKTYPE_SIDEKICK_LOOKDOWN:
		{
			SIDEKICK_StartLookDown( self );
			break;
		}
        case TASKTYPE_SIDEKICK_SAYSOMETHING:
		{
			SIDEKICK_StartSaySomething( self );
			break;
		}
        case TASKTYPE_SIDEKICK_KICKSOMETHING:
		{
			SIDEKICK_StartKickSomething( self );
			break;
		}
        case TASKTYPE_SIDEKICK_WHISTLE:
		{
			SIDEKICK_StartWhistle( self );
			break;
		}
        case TASKTYPE_SIDEKICK_ANIMATE:
		{
			SIDEKICK_StartAnimate( self );
			break;
		}
        case TASKTYPE_SIDEKICK_STOPGOINGFURTHER:
        {
            SIDEKICK_StartStopGoingFurther( self );
            break;
        }
        case TASKTYPE_SIDEKICK_TELEPORTANDCOMENEAR:
        {
            SIDEKICK_StartTeleportAndComeNear( self );
            break;
        }
        case TASKTYPE_SIDEKICK_COMEHERE:
        {
            SIDEKICK_StartComeHere( self );
            break;
        }
        case TASKTYPE_SIDEKICK_TELEPORT:
        {
            SIDEKICK_StartTeleport( self );
            break;
        }
		case TASKTYPE_SIDEKICK_STOP://<nss>
		{
			SIDEKICK_StartStop(self);
			break;
		}
		case TASKTYPE_KAGE_SMOKESCREEN:// NSS[1/15/00]:
		{
			KAGE_StartSmokeScreen(self);
			break;
		}
		case TASKTYPE_KAGE_WAIT:// NSS[2/19/00]:
		{
			KAGE_StartWait(self);
			break;
		}
		case TASKTYPE_KAGE_RETURN:// NSS[2/19/00]:
		{
			KAGE_StartReturn(self);
			break;
		}
		case TASKTYPE_KAGE_CREATEPROTECTORS:// NSS[1/15/00]:
		{
			KAGE_StartCreateProtectors(self);
			break;
		}
		case TASKTYPE_KAGE_CHARGEHEALTH:// NSS[1/15/00]:
		{
			KAGE_Start_ChargeHealth(self);
			break;
		}	
		case TASKTYPE_TESTMAP_ROAM:
		{
			AI_StartTestMapRoam( self );
			break;
		}
		default:
		{
			break;
		}
	}

    if ( self->nextthink <= gstate->time )
    {
        AI_SetNextThinkTime( self, 0.1f );
    }

}
int SIDEKICK_HandleFollowing( userEntity_t *self );
int ITEM_IsVisible( userEntity_t *self );
void AdjustHealth_BasedOn_NOC(userEntity_t *self);
// ----------------------------------------------------------------------------
//
// Name:		AI_AddInitialTasksToGoal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_AddInitialTasksToGoal( userEntity_t *self, GOAL_PTR pGoal, int bStart )
{
	_ASSERTE( self );
	_ASSERTE( pGoal );

	playerHook_t *hook = AI_GetPlayerHook( self );
			
	if ( pGoal && GOAL_GetType( pGoal ) != GOALTYPE_SCRIPTACTION )
	{
		GOAL_ClearTasks( pGoal );
	}

	GOALTYPE nGoalType = GOAL_GetType( pGoal );
	AIDATA_PTR pAIData = GOAL_GetData( pGoal );


	TASK_PTR pNewTask = NULL;
	switch ( nGoalType )
	{
		case GOALTYPE_IDLE:
		{
			pNewTask = AI_AddNewTask( pGoal, TASKTYPE_IDLE );
			if ( bStart )
			{
				AI_StartIdle( self );
			}
			break;
		}
		case GOALTYPE_KILLENEMY:
		{
			userEntity_t *pEnemy = pAIData->pEntity;
			
			if ( !AI_IsAlive( pEnemy ) )
			{
				GOAL_Satisfied( pGoal );
				AI_RemoveCurrentGoal( self );
				break;
			}

            // NSS[6/7/00]:This will adjust a boss's health based on the number of clients connected when the boss goes into attack mode for the first time.
			AdjustHealth_BasedOn_NOC(self);

			// see if the AI can move to chase the target if necessary
            if ( !AI_CanMove( hook ) && !AI_IsVisible(self, pEnemy) && hook->type != TYPE_ROCKETMP)
            {
				GOAL_Satisfied( pGoal );
				AI_RemoveCurrentGoal( self );
				break;
            }
			//NSS[11/6/99]:
			//_ASSERTE( pEnemy->solid != SOLID_NOT );
			
			AI_SetEnemy( self, (userEntity_t *)pEnemy );
			AI_EnemyAlertNearbyMonsters( self, self->enemy );

			// for immobile creatures, just run the attack task, except for
			// the rotworm and protopod, which drop from the ceiling and hatch
			if ( self->movetype == MOVETYPE_NONE && hook->type != TYPE_PROTOPOD &&
			     hook->type != TYPE_COLUMN && hook->type != TYPE_ROTWORM && AI_IsAlive(self) )
            {
				AI_AddNewTask( pGoal, TASKTYPE_ATTACK, pAIData->pEntity );
				if ( bStart )
				{
					AI_StartAttack( self );
				}
                return;
            }

            if ( AI_IsSidekick( hook ) )
            {
				// sidekick should not add GOALTYPE_KILLENEMY
                return;
            }
            else
            if ( hook->type == TYPE_DEATHSPHERE )
			{
				AI_AddNewTask( pGoal, TASKTYPE_CHASE, pAIData->pEntity );

				AI_AddNewTask( pGoal, TASKTYPE_DEATHSPHERE_CHARGEWEAPON );  

				AI_AddNewTask( pGoal, TASKTYPE_ATTACK, pAIData->pEntity );

				if ( bStart )
				{
					AI_StartChase( self );
				}
			}
			else
			if ( hook->type == TYPE_DOOMBAT )//<nss>
			{
				pAIData->nValue = bStart;
				//New cycle lets setup the tasks!
				AI_AddNewTask( pGoal, TASKTYPE_DOOMBAT_ATTACK_THINK, pAIData->pEntity );
			}
			else
			if ( hook->type == TYPE_DRAGON )//<nss>
			{
				pAIData->nValue = bStart;
				//New cycle lets setup the tasks!
				AI_AddNewTask( pGoal, TASKTYPE_DRAGON_ATTACK_THINK, pAIData->pEntity );
			}
			else
			if ( hook->type == TYPE_GHOST )//<nss>
			{
				pAIData->nValue = bStart;
				AI_AddNewTask( pGoal, TASKTYPE_GHOST_WAKEUP);
				//AI_AddNewTask( pGoal, TASKTYPE_GHOST_SPIRAL);
				AI_AddNewTask( pGoal, TASKTYPE_GHOST_CHASE, pAIData->pEntity );
				AI_AddNewTask( pGoal, TASKTYPE_GHOST_ATTACK, pAIData->pEntity );
				if ( bStart )
				{
					Ghost_Start_WakeUp(self);
				}

			}
			else 
			if ( hook->type == TYPE_FROGINATOR )//<nss>
			{
				pAIData->nValue = bStart;
				//New cycle lets setup the think task!
				AI_AddNewTask( pGoal, TASKTYPE_FROG_ATTACK_THINK, pAIData->pEntity );
			}
			else
			if ( hook->type == TYPE_GRIFFON )//<nss>
			{
				pAIData->nValue = bStart;
				//New cycle lets setup the tasks!
				AI_AddNewTask( pGoal, TASKTYPE_GRIFFON_ATTACK_THINK, pAIData->pEntity );
			}
			else
			if ( hook->type == TYPE_CHAINGANG )//<nss>
			{
				pAIData->nValue = bStart;
				//New cycle lets setup the tasks!
				AI_AddNewTask( pGoal, TASKTYPE_CHAINGANG_ATTACK_THINK, pAIData->pEntity );
			}
			else
			if ( hook->type == TYPE_HARPY )
			{
				pAIData->nValue = bStart;
				//New cycle lets setup the tasks!
//				AI_AddNewTask( pGoal, TASKTYPE_HARPY_ATTACK_THINK, pAIData->pEntity );
				AI_AddNewTask( self, TASKTYPE_HARPY_CHASE, pAIData->pEntity );
				AI_AddNewTask( self, TASKTYPE_HARPY_ATTACK, pAIData->pEntity );
			}
			else
			if ( hook->type == TYPE_TH_SKEET )
			{
				AI_AddNewTask( pGoal, TASKTYPE_CHASE, pAIData->pEntity );
				AI_AddNewTask( pGoal, TASKTYPE_THUNDERSKEET_ATTACK, pAIData->pEntity );
				AI_AddNewTask( pGoal, TASKTYPE_THUNDERSKEET_HOVER, pAIData->pEntity );
			}
			else
			if ( hook->type == TYPE_SL_SKEET )
			{
				AI_AddNewTask( pGoal, TASKTYPE_CHASE, pAIData->pEntity );
				AI_AddNewTask( pGoal, TASKTYPE_SKEETER_DARTTOWARDENEMY, pAIData->pEntity );
				AI_AddNewTask( pGoal, TASKTYPE_SKEETER_ATTACK, pAIData->pEntity );
			}
			else
			if (hook->type == TYPE_PROTOPOD)
			{
				AI_AddNewTask(pGoal,TASKTYPE_PROTOPOD_WAIT_TO_HATCH);
				AI_AddNewTask(pGoal,TASKTYPE_PROTOPOD_HATCH);
			}
			else
			if ( hook->type == TYPE_COLUMN )
			{
//				userEntity_t *pEnemy = pAIData->pEntity;// SCG[1/23/00]: not used

				// columns need to awaken before doing anything else
				AI_AddNewTask( pGoal, TASKTYPE_COLUMN_AWAKEN );

				// then they can chase and attack
				AI_AddNewTask( pGoal, TASKTYPE_CHASE, pAIData->pEntity );
				AI_AddNewTask( pGoal, TASKTYPE_ATTACK, pAIData->pEntity );

				if ( bStart )
				{
					COLUMN_StartAwaken( self );
				}
			}
			else
			if ( hook->type == TYPE_MEDUSA )
			{
				pAIData->nValue = bStart;
				AI_AddNewTask( self, TASKTYPE_MEDUSA_CHASE, pAIData->pEntity );
				AI_AddNewTask( self, TASKTYPE_ATTACK, pAIData->pEntity );
			}
			else 
			if( AI_IsTakeCoverType( hook ) && (hook->nSpawnValue & SPAWN_TAKECOVER) )
			{
				AI_RemoveCurrentGoal( self );
				AI_AddNewGoal( self, GOALTYPE_TAKECOVER );
			}
            else
			{

				if ( AI_IsFlyingUnit( self ) && 
                     (hook->nAttackType == ATTACK_AIR_SWOOP_MELEE || 
					  hook->nAttackType == ATTACK_AIR_SWOOP_RANGED) )
				{
					AI_AddNewTask( pGoal, TASKTYPE_SWOOPATTACK, pAIData->pEntity );
					
					if ( bStart )
					{
						AI_StartSwoopAttack( self );
					}
				}
				else
				if ( AI_IsFlyingUnit( self ) && hook->nAttackType == ATTACK_AIR_RANGED )
				{
					AI_AddNewTask( pGoal, TASKTYPE_AIRRANGEDATTACK, pAIData->pEntity );
					if ( bStart )
					{
						AI_StartAirRangedAttack( self );
					}
				}
				else
				if ( AI_IsFlyingUnit( self ) && hook->nAttackType == ATTACK_AIR_MELEE )
				{
					AI_AddNewTask( pGoal, TASKTYPE_AIRMELEEATTACK, pAIData->pEntity );
					
					if ( bStart )
					{
						AI_StartAirMeleeAttack( self );
					}
				}
				else
				if ( hook->nAttackType == ATTACK_GROUND_CHASING )
				{
					if ( AI_CanMove( hook ) )
					{
						AI_AddNewTask( pGoal, TASKTYPE_CHASEATTACK, pAIData->pEntity );
						
						if ( bStart )
						{
							AI_StartChaseAttack( self );
						}
					}
					else
					{
						AI_AddNewTask( pGoal, TASKTYPE_ATTACK, pAIData->pEntity );
						
						if ( bStart )
						{
							AI_StartAttack( self );
						}
					
					}
				}
				else
				{
					// no chasing in snipe mode
					if ( (hook->ai_flags & AI_SNIPE) && !AI_CanMove( hook ) )
					{
						AI_AddNewTask( pGoal, TASKTYPE_ATTACK, pAIData->pEntity );
						if ( bStart )
						{
							AI_StartAttack( self );
						}
					}
					else
					{
						userEntity_t *pEnemy = pAIData->pEntity;
						float fDistance = VectorDistance( self->s.origin, pEnemy->s.origin );
						// NSS[2/22/00]:This has to be here to prevent bad things from happening... i.e. recursive bullshit.
						if ( !AI_IsWithinAttackDistance( self, fDistance ) || !AI_IsChaseVisible( self ) )
						{
							pNewTask = AI_AddNewTask( pGoal, TASKTYPE_CHASE, pAIData->pEntity );
							AI_AddNewTask( pGoal, TASKTYPE_ATTACK, pAIData->pEntity );
							if ( bStart )
							{
								AI_StartChase( self );
							}
						}
						else
						{
							AI_AddNewTask( pGoal, TASKTYPE_ATTACK, pAIData->pEntity );
							if ( bStart )
							{
								AI_StartAttack( self );
							}
						}
					}
				}
			}

			break;
		}
		case GOALTYPE_WANDER:
		{
			if ( AI_CanMove( hook ) )
			{

				pNewTask = AI_AddNewTask( pGoal, TASKTYPE_WANDER );
				if ( bStart )
				{
					AI_StartWander( self );
				}
			}
			else
			{
				AI_RemoveCurrentGoal( self );
			}
			break;
		}
		case GOALTYPE_PATROL:
		{
			if ( AI_CanMove( hook ) )
			{
				
				pNewTask = AI_AddNewTask( pGoal, TASKTYPE_PATROL );
				if ( bStart )
				{
					AI_StartPatrol( self );
				}
			}
			else
			{
				AI_RemoveCurrentGoal( self );
			}

			break;
		}
		case GOALTYPE_HIDE:
		{
			break;
		}
		case GOALTYPE_PICKUPITEM:
		{
			if(pAIData->pEntity && pAIData->pEntity->flags & FL_ITEM)
			{
				int bItemVisible = ITEM_IsVisible( pAIData->pEntity );// NSS[3/14/00]:This determines if the item still exists.
				if (bItemVisible)
				{
					AI_AddNewTask( self, TASKTYPE_SIDEKICK_PICKUPITEM, pAIData->pEntity );
					if ( bStart )
					{
						SIDEKICK_StartPickupItem( self );
					}
					break;
				}
			}
			// NSS[3/14/00]:If this happens then the item is gone.
			AI_RemoveCurrentGoal(self);
			break;
		}
		case GOALTYPE_USEHOSPORTAL:// NSS[5/19/00]:Added so sidekicks can now use hosportals
		{
			if(pAIData->pEntity)
			{
				int bItemVisible = ITEM_IsVisible( pAIData->pEntity );
				if (bItemVisible)
				{
					AI_AddNewTask( self, TASKTYPE_SIDEKICK_USEITEM, pAIData->pEntity );
					if ( bStart )
					{
						SIDEKICK_StartUseItem( self );
					}
					break;
				}
			}
			// NSS[3/14/00]:If this happens then the item being told to pickup does not exist or someone is using this goaltype wrong.
			AI_RemoveCurrentGoal(self);
			break;
		}
		case GOALTYPE_MOVETOLOCATION:
		{
			if ( AI_CanMove( hook ) )
			{
				pNewTask = AI_AddNewTask( pGoal, TASKTYPE_MOVETOLOCATION, pAIData->destPoint );
				if ( bStart )
				{
					AI_StartMoveToLocation( self );
				}
			}
			else
			{
				AI_RemoveCurrentGoal( self );
			}

			break;
		}
		case GOALTYPE_AMBUSH:
		{
			AI_SetNextThinkTime( self, 0.1f );
            break;
		}
		case GOALTYPE_RUNAWAY:
		{
			if ( AI_CanMove( hook ) )
			{
				AI_SetEnemy( self, (userEntity_t *)pAIData->pEntity );

				pNewTask = AI_AddNewTask( pGoal, TASKTYPE_RUNAWAY, (userEntity_t *) pAIData->pEntity );
				if ( bStart )
				{
					AI_StartRunAway (self);
				}
			}
			else
			{
				AI_RemoveCurrentGoal( self );
			}
			break;
		}
		case GOALTYPE_SNIPE:
		{
			if ( AI_CanMove( hook ) )
			{
				AI_AddNewTask( pGoal, TASKTYPE_HIDE );
				AI_AddNewTask( pGoal, TASKTYPE_SNIPE );
				if ( bStart )
				{
					AI_StartHide( self );
				}
			}
            else
            {
				AI_AddNewTask( pGoal, TASKTYPE_SNIPE );
				if ( bStart )
				{
					AI_StartSnipe( self );
				}
            }
			break;
		}
		case GOALTYPE_FOLLOW:
		{
			if ( AI_CanMove( hook ) && AI_IsAlive( pAIData->pEntity ) )
			{
				playerHook_t *hook = AI_GetPlayerHook( self );
                if ( !hook->owner )
                {
				    AI_SetOwner( self, pAIData->pEntity );
                }

//				self->goalentity = hook->owner;

				if ( AI_IsSidekick( hook ) )
                {
                    int nFollowing = SIDEKICK_DetermineFollowing( self );
                }
                if ( hook->nFollowing == FOLLOWING_PLAYER && hook->owner->deadflag == DEAD_NO)
				{
					// set the owner to the goal data
					GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
					GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
					TASK_PTR pCurrentTask = GOAL_GetCurrentTask( pCurrentGoal );
					if(AI_IsSidekick(hook) &&  pCurrentTask == NULL)
					{
						AI_AddNewTaskAtFront(self,TASKTYPE_IDLE);
					}
					if(!AI_IsSidekick(hook))
					{
						float fXYDistance = VectorXYDistance( self->s.origin, hook->owner->s.origin );
						if ( fXYDistance > AI_GetRunFollowDistance(hook) )
						{
							if(!AI_IsCrouching(hook))
								AI_AddNewTask( self, TASKTYPE_FOLLOWRUNNING, hook->owner );
							else
								AI_AddNewTask( self, TASKTYPE_FOLLOWWALKING, hook->owner );
							AI_AddNewTask( self, TASKTYPE_IDLE );
							if ( bStart )
							{
								AI_StartFollowRunning( self );
							}
						}
						else 
						if ( fXYDistance > AI_GetWalkFollowDistance(hook) )
						{
		
							AI_AddNewTask( self, TASKTYPE_FOLLOWWALKING, hook->owner );
							AI_AddNewTask( self, TASKTYPE_IDLE );
							if ( bStart )
							{
								AI_StartFollowWalking( self );
							}
						}
						else
						{
							pNewTask = AI_AddNewTask( pGoal, TASKTYPE_IDLE );
							if ( bStart )
							{
								AI_StartIdle (self);
							}
						}
					}
					else
					{
						if ( !SIDEKICK_HandleFollowing( self ) )
						{
							pNewTask = AI_AddNewTask( pGoal, TASKTYPE_IDLE );
							if ( bStart )
							{
								AI_StartIdle (self);
							}
						}
					}
				}
				else
                if ( hook->nFollowing == FOLLOWING_SIDEKICK && hook->owner->deadflag == DEAD_NO)
                {
                    userEntity_t *pOtherSidekick = SIDEKICK_GetOtherSidekick( self );
                    _ASSERTE( pOtherSidekick );
                
					float fXYDistance = VectorXYDistance( self->s.origin, pOtherSidekick->s.origin );
					if ( fXYDistance > AI_GetRunFollowDistance(hook) )
					{
						AI_AddNewTask( self, TASKTYPE_FOLLOWSIDEKICKRUNNING, pOtherSidekick );
						AI_AddNewTask( self, TASKTYPE_IDLE );
						if ( bStart )
						{
							AI_StartFollowSidekickRunning( self );
						}
					}
					else 
					if ( fXYDistance > AI_GetWalkFollowDistance(hook) )
					{
						AI_AddNewTask( self, TASKTYPE_FOLLOWSIDEKICKWALKING, pOtherSidekick );
						AI_AddNewTask( self, TASKTYPE_IDLE );
						if ( bStart )
						{
							AI_StartFollowSidekickWalking( self );
						}
					}
					else
					{
						pNewTask = AI_AddNewTask( pGoal, TASKTYPE_IDLE );
						if ( bStart )
						{
							AI_StartIdle (self);
						}
					}
                }
                else
				{
					//	follow starts with TASKTYPE_IDLE, which it always returns
					//	to when close enough to owner
					pNewTask = AI_AddNewTask (pGoal, TASKTYPE_IDLE);
					if ( bStart )
					{
						AI_StartIdle (self);
					}
				}
			}
			else
			{
				AI_RemoveCurrentGoal( self );
			}
			break;
		}
		case GOALTYPE_FLYTOLOCATION:
		{
			if ( AI_CanMove( hook ) )
			{
				pNewTask = AI_AddNewTask( pGoal, TASKTYPE_FLYTOLOCATION, pAIData->destPoint );
				if ( bStart )
				{
					AI_StartFlyToLocation( self );
				}
			}
			else
			{
				AI_RemoveCurrentGoal( self );
			}
			break;
		}
		case GOALTYPE_SWIMTOLOCATION:
		{
			if ( AI_CanMove( hook ) )
			{
				pNewTask = AI_AddNewTask( pGoal, TASKTYPE_SWIMTOLOCATION, pAIData->destPoint );
				if ( bStart )
				{
					AI_StartSwimToLocation( self );
				}
			}
			else
			{
				AI_RemoveCurrentGoal( self );
			}
			break;
		}
		case GOALTYPE_FACEANGLE:
		{
			pNewTask = AI_AddNewTask( pGoal, TASKTYPE_FACEANGLE, pAIData->destPoint );
			if ( bStart )
			{
				AI_StartFaceAngle( self );
			}
			break;
		}
		case GOALTYPE_WAIT:
		{
			pNewTask = AI_AddNewTask( pGoal, TASKTYPE_WAIT, pAIData->fValue );
			if ( bStart )
			{
				AI_StartWait( self );
			}
			break;
		}
		case GOALTYPE_BACKUPALLATTRIBUTES:
		{
			pNewTask = AI_AddNewTask( pGoal, TASKTYPE_BACKUPALLATTRIBUTES );
			if ( bStart )
			{
				AI_StartBackupAllAttributes( self );
			}
			break;
		}
		case GOALTYPE_RESTOREALLATTRIBUTES:
		{
			pNewTask = AI_AddNewTask( pGoal, TASKTYPE_RESTOREALLATTRIBUTES );
			if ( bStart )
			{
				AI_StartRestoreAllAttributes( self );
			}
			break;
		}
		case GOALTYPE_MODIFYTURNATTRIBUTE:
		{
			pNewTask = AI_AddNewTask( pGoal, TASKTYPE_MODIFYTURNATTRIBUTE, pAIData->destPoint );
			if ( bStart )
			{
				AI_StartModifyTurnAttribute( self );
			}
			break;
		}
		case GOALTYPE_MODIFYRUNATTRIBUTE:
		{
			pNewTask = AI_AddNewTask( pGoal, TASKTYPE_MODIFYRUNATTRIBUTE, pAIData->fValue );
			if ( bStart )
			{
				AI_StartModifyRunAttribute( self );
			}
			break;
		}
		case GOALTYPE_MODIFYWALKATTRIBUTE:
		{
			pNewTask = AI_AddNewTask( pGoal, TASKTYPE_MODIFYWALKATTRIBUTE, pAIData->fValue );
			if ( bStart )
			{
				AI_StartModifyWalkAttribute( self );
			}
			break;
		}
		case GOALTYPE_PLAYANIMATION:
		{
			pNewTask = AI_AddNewTask(pGoal, TASKTYPE_PLAYANIMATION, pAIData);
			if ( bStart )
			{
				AI_StartPlayAnimation( self );
			}
			break;
		}
		case GOALTYPE_SETIDLEANIMATION:
		{
			pNewTask = AI_AddNewTask(pGoal, TASKTYPE_SETIDLEANIMATION, pAIData);
			if ( bStart )
			{
				AI_StartSetIdleAnimation( self );
			}
			break;
		}
		case GOALTYPE_RANDOMWANDER:
		{
			if ( AI_CanMove( hook ) )
			{
				pNewTask = AI_AddNewTask( pGoal, TASKTYPE_RANDOMWANDER, pAIData->fValue );
				if (bStart)
				{
					AI_StartRandomWander( self );
				}
			}
			else
			{
				AI_RemoveCurrentGoal( self );
			}
			break;
		}
		case GOALTYPE_STARTUSINGWALKSPEED:
		{
			pNewTask = AI_AddNewTask( pGoal, TASKTYPE_STARTUSINGWALKSPEED );
			if ( bStart )
			{
				AI_StartUsingWalkSpeed( self );
			}
			break;
		}
		case GOALTYPE_STARTUSINGRUNSPEED:
		{
			pNewTask = AI_AddNewTask( pGoal, TASKTYPE_STARTUSINGRUNSPEED );
			if ( bStart )
			{
				AI_StartUsingRunSpeed( self );
			}
			break;
		}
		case GOALTYPE_MOVETOENTITY:
		{
			if ( AI_CanMove( hook ) )
			{
				pNewTask = AI_AddNewTask( pGoal, TASKTYPE_MOVETOENTITY, pAIData->pEntity );
				if ( bStart )
				{
					AI_StartMoveToEntity( self );
				}
			}
			else
			{
				AI_RemoveCurrentGoal( self );
			}
			break;
		}
        case GOALTYPE_MOVETOEXACTLOCATION:
        {
			if ( AI_CanMove( hook ) )
			{
				pNewTask = AI_AddNewTask( pGoal, TASKTYPE_MOVETOEXACTLOCATION, pAIData->destPoint );
				if ( bStart )
				{
					AI_StartMoveToExactLocation( self );
				}
			}
			else
			{
				AI_RemoveCurrentGoal( self );
			}
			break;
        }

		case GOALTYPE_STAY:
		{
			pNewTask = AI_AddNewTask( pGoal, TASKTYPE_STAY );
			if ( bStart )
			{
				AI_StartStay( self );
			}
			break;
		}
		case GOALTYPE_PATHFOLLOW:
		{
			// get rid of all previous path follow goals
			GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
			GOALSTACK_RemoveAllGoalsOfTypeExceptCurrentGoal(pGoalStack, GOALTYPE_PATHFOLLOW);
			if ( AI_CanMove( hook ) )
			{
				pNewTask = AI_AddNewTask( pGoal, TASKTYPE_PATHFOLLOW );
				if ( bStart )
				{
					AI_StartPathFollow( self );
				}
			}
			else
			{
				AI_RemoveCurrentGoal( self );
			}
			break;
		}
		case GOALTYPE_TAKECOVER:
		{
			pNewTask = AI_AddNewTask( pGoal, TASKTYPE_TAKECOVER );
			if ( bStart )
			{
				AI_StartTakeCover( self );
			}
			break;
	    }
		case GOALTYPE_USE:
		{
			pNewTask = AI_AddNewTask( pGoal, TASKTYPE_USE, pAIData->pEntity );
			if ( bStart )
			{
				AI_StartUse( self );
			}
			break;
		}
		case GOALTYPE_SCRIPTACTION:
		case GOALTYPE_WHENUSED_SCRIPTACTION:
		{
			// IMPORTANT: do not add any tasks here

			AI_SetNextThinkTime( self, 0.1f );
            break;
		}
		case GOALTYPE_GENERALUSE:
		{
			// IMPORTANT: Do not add any tasks here

            AI_SetNextThinkTime( self, 0.1f );
			break;
		}
        case GOALTYPE_PLAYSOUNDTOEND:
        {
			pNewTask = AI_AddNewTask( pGoal, TASKTYPE_PLAYSOUNDTOEND, pAIData );
			if ( bStart )
			{
				AI_StartPlaySoundToEnd( self );
			}
			break;
        }

		// monster specific goals should be place here, everything else above

/*
		case GOALTYPE_INMATER_KILLPRISONER:
		{
			userEntity_t *pPrisoner = self->enemy;
			if ( !(pPrisoner && _stricmp( pPrisoner->className, "monster_prisoner" ) == 0) )
			{
				pPrisoner = inmater_find_prisoner(self);
			}

			if ( pPrisoner )
			{ 
				GOAL_Set( pGoal, nGoalType, pPrisoner );
				AI_AddNewTask( pGoal, TASKTYPE_INMATER_GOTOPRISONER, pPrisoner );
				AI_AddNewTask( pGoal, TASKTYPE_INMATER_WAITATPRISONER, 1.5f );
				AI_AddNewTask( pGoal, TASKTYPE_INMATER_KILLPRISONER, pPrisoner );
				if ( bStart )
				{
					INMATER_StartGotoPrisoner( self );
				}
			}
			else
			{
				AI_RemoveCurrentGoal( self );
			}

			break;
		}
*/
		case GOALTYPE_PRISONER_WANDER:
		{
			AI_AddNewTask( pGoal, TASKTYPE_PRISONER_WANDER );
			if ( bStart )
			{
				PRISONER_StartWander( self );
			}
			break;
		}
		case GOALTYPE_BUBOID_GETOUTOFCOFFIN:
		{
			AI_AddNewTask( pGoal, TASKTYPE_BUBOID_GETOUTOFCOFFIN );
			if ( bStart )
			{
				BUBOID_StartGetOutOfCoffin( self );
			}
			break;
		}
		case GOALTYPE_BUBOID_MELT:
		{
			AI_AddNewTask( pGoal, TASKTYPE_BUBOID_MELT );
			AI_AddNewTask( pGoal, TASKTYPE_BUBOID_MELTED );
			AI_AddNewTask( pGoal, TASKTYPE_BUBOID_UNMELT );

			if ( bStart )
			{
				BUBOID_StartMelt( self );
			}
			break;
		}
		case GOALTYPE_CAMBOT_PATHFOLLOW:
		{
			AI_AddNewTask( pGoal, TASKTYPE_CAMBOT_PATHFOLLOW );
			if ( bStart )
			{
				CAMBOT_StartPathFollow( self );
			}
			break;
		}
		case GOALTYPE_CAMBOT_FOLLOWPLAYER:
		{
			AI_AddNewTask( pGoal, TASKTYPE_CAMBOT_FOLLOWPLAYER, pAIData->pEntity );
			if ( bStart )
			{
				CAMBOT_StartFollowPlayer( self );
			}
			break;
		}
		case GOALTYPE_PROTOPOD_HATCH:
		{
			AI_AddNewTask( pGoal, TASKTYPE_PROTOPOD_WAIT_TO_HATCH );
			AI_AddNewTask( pGoal, TASKTYPE_PROTOPOD_HATCH );
			if ( bStart )
			{
				PROTOPOD_StartWaitToHatch( self );
			}
			break;
		}

		case GOALTYPE_MEDUSA_GAZE:
		{
			AI_AddNewTask( self, TASKTYPE_MEDUSA_RATTLE );
			if ( bStart )
			{
				MEDUSA_StartRattle( self );
			}
			AI_AddNewTask( pGoal, TASKTYPE_MEDUSA_GAZE );
			AI_AddNewTask( pGoal, TASKTYPE_MEDUSA_STOPGAZING );
			break;
		}

		case GOALTYPE_SKEETER_HATCH:
		{
			AI_AddNewTask( self, TASKTYPE_SKEETER_PREHATCH );
			if ( bStart )
			{
				SKEETER_StartPreHatch( self );
			}
			AI_AddNewTask( pGoal, TASKTYPE_SKEETER_HATCH );
			break;
		}

		case GOALTYPE_LYCANTHIR_RESURRECT:
		{
			AI_AddNewTask( self, TASKTYPE_LYCANTHIR_RESURRECT_WAIT );
			AI_AddNewTask( pGoal, TASKTYPE_LYCANTHIR_RESURRECT );

			if ( bStart )
			{
				LYCANTHIR_StartResurrectWait( self );
			}
			break;
		}
		case GOALTYPE_BUBOID_RESURRECT:
		{
			AI_AddNewTask( self, TASKTYPE_BUBOID_RESURRECT_WAIT );
			AI_AddNewTask( pGoal, TASKTYPE_BUBOID_RESURRECT );

			if ( bStart )
			{
				BUBOID_StartResurrectWait( self );
			}
			break;
		}
		case GOALTYPE_COLUMN_AWAKEN:
		{
			AI_AddNewTask( pGoal, TASKTYPE_COLUMN_AWAKEN_WAIT );

			if ( bStart )
			{
				COLUMN_StartAwakenWait( self );
			}
			break;
		}
		case GOALTYPE_ROTWORM_JUMPATTACK:
		{
			AI_AddNewTask( pGoal, TASKTYPE_ROTWORM_JUMPFLY );
			if ( bStart )
			{
				ROTWORM_JumpFly( self );
			}
			break;
		}
		case GOALTYPE_WYNDRAX_POWERUP:
		{
			AI_AddNewTask( self, TASKTYPE_WYNDRAX_CHARGE_UP );
			if ( bStart )
			{
				WYNDRAX_Start_Charge_Up( self );
			}
			break;		
		}
		case GOALTYPE_WYNDRAX_RECHARGE:
		{
			AI_AddNewTask( self, TASKTYPE_WYNDRAX_FIND_WISPMASTER );
			AI_AddNewTask( self, TASKTYPE_WYNDRAX_COLLECT_WISPS );
			if ( bStart )
			{
				WYNDRAX_Start_Find_WispMaster( self );
			}
			break;
		}
		case GOALTYPE_SKINNYWORKER_HIDE:
		{
			AI_AddNewTask( self, TASKTYPE_SKINNYWORKER_HIDE );
			AI_AddNewTask( self, TASKTYPE_SKINNYWORKER_BACKWALL );
			AI_AddNewTask( self, TASKTYPE_SKINNYWORKER_LOOKOUT );

			if ( bStart )
			{
				SKINNYWORKER_StartHide( self );
			}
			break;
		}

		case GOALTYPE_SIDEKICK_WAITHERE:
		{
			AI_SetNextThinkTime( self, 0.1f );
            break;
		}
		case GOALTYPE_SIDEKICK_FOLLOWOWNER:
		{
			AI_SetNextThinkTime( self, 0.1f );
            break;
		}
		case GOALTYPE_SIDEKICK_ATTACK:
		{
			AI_SetNextThinkTime( self, 0.1f );
            break;
		}
        case GOALTYPE_SIDEKICK_STOPGOINGFURTHER:
        {
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }
        case GOALTYPE_SIDEKICK_TELEPORTANDCOMENEAR:
        {
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }
		
		case GOALTYPE_BOT_ROAM:
		{
			AI_AddNewTask( self, TASKTYPE_BOT_ROAM );
			if ( bStart )
			{
				BOT_StartRoam( self );
			}
			break;
		}
		case GOALTYPE_BOT_PICKUPITEM:
		{
			AI_AddNewTask( self, TASKTYPE_BOT_PICKUPITEM, pAIData->pEntity );
			if ( bStart )
			{
				BOT_StartPickupItem( self );
			}
			break;
		}
		case GOALTYPE_BOT_ENGAGEENEMY:
		{
			int bEnemyVisible = AI_IsCompletelyVisible( self, self->enemy );
			if ( bEnemyVisible == NOT_VISIBLE )
			{
				AI_AddNewTask( self, TASKTYPE_BOT_CHASEATTACK );
				AI_AddNewTask( self, TASKTYPE_BOT_ENGAGEENEMY );

				if ( bStart )
				{
					BOT_StartChaseAttack( self );
				}
			}
			else
			{
				AI_AddNewTask( self, TASKTYPE_BOT_ENGAGEENEMY );
				if ( bStart )
				{
					BOT_StartEngageEnemy( self );
				}		
			}
				
			break;
		}

		case GOALTYPE_TESTMAP_ROAM:
		{
			AI_AddNewTask( self, TASKTYPE_TESTMAP_ROAM );
			if ( bStart )
			{
				AI_StartTestMapRoam( self );
			}
			break;
		}
		// NSS[1/15/00]:
		case GOALTYPE_KAGE_SPARKUP:
		{
			AI_AddNewTask(self,TASKTYPE_KAGE_CREATEPROTECTORS);
			AI_AddNewTask(self,TASKTYPE_KAGE_CHARGEHEALTH);
			if( bStart)
			{
				KAGE_StartCreateProtectors(self);
			}
			break;
		}
		case GOALTYPE_KAGE_YINYANG:
		{
			AI_AddNewTask(self,TASKTYPE_KAGE_SMOKESCREEN);
			AI_AddNewTask(self,TASKTYPE_KAGE_WAIT);
			AI_AddNewTask(self,TASKTYPE_KAGE_RETURN);
			if( bStart)
			{
				KAGE_StartSmokeScreen(self);
			}		
		}
		default:
		{
			AI_SetNextThinkTime( self, 0.1f );
            break;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_CanTarget
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static int AI_CanTarget( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	int bCanTarget = TRUE;
	if ( (hook->ai_flags & AI_IGNORE_PLAYER) || (hook->ai_flags & AI_SNIPE) )
	{
		bCanTarget = FALSE;
	}
	
	return bCanTarget;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_DoSpecialEffects
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_DoSpecialEffects( userEntity_t *self )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
}


// ----------------------------------------------------------------------------
//
// Name:		AI_CheckForEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_CheckForEnemy( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if(gstate->bCinematicPlaying)
		return FALSE;
	if(!hook)
		return FALSE;
	if(AI_IsSidekick(hook))
		return FALSE;
	hook->nTargetCounter++;
	if ( (hook->nTargetCounter % hook->nTargetFrequency) == 0 && AI_CanAttack( self ) && AI_IsAlive( self ) )
	{
		//	check for a target
			
		if(!hook->fnFindTarget)
			return FALSE;
		userEntity_t *pEnemy = hook->fnFindTarget( self );

		int WaterLevel;
		//NSS[11/27/99]:For monsters that are not Amphibious nor can swim and are trying to chase a player that is in water
		
		if(pEnemy)
		{
			if(pEnemy->s.maxs.z >= (self->s.maxs.z + 8))
			{
				WaterLevel = 1;
			}
			else if (pEnemy->s.maxs.z >= (self->s.maxs.z + 16))
			{
				WaterLevel = 2;
			}
			else
			{
				WaterLevel = 3;
			}

			if(!(hook->dflags & DFL_AMPHIBIOUS) && pEnemy->waterlevel > WaterLevel && !(hook->ai_flags & AI_CANSWIM))
			{
				return FALSE;
			}
		}

		
		if ( AI_CanTarget( self ) && AI_IsOkToAttack( hook ) && pEnemy )
		{
			// do self preservation actions

			// if this is a sidekick, then stay close to the owner, espcially when
			// the sidekick is fighting

			if ( AI_HandleSelfPreservation( self ) )
			{
				AI_SetNextThinkTime( self, 0.1f );
                return TRUE;
			}

			// NSS[1/30/00]:Adding the freakin hide crud'ola for the only 2 monsters in the game that are supposed to run...geesh
			if((hook->type == TYPE_SKINNYWORKER || hook->type == TYPE_FATWORKER) && AI_IsVisible(self,pEnemy) && AI_IsAlive(pEnemy))
			{
				self->enemy = pEnemy;
				if(!SIDEKICK_HasTaskInQue(self,TASKTYPE_SKINNYWORKER_HIDE))
				{
					AI_AddNewTaskAtFront(self, TASKTYPE_SKINNYWORKER_HIDE);
				}
			}

			if ( hook->fnStartAttackFunc && AI_IsAlive( pEnemy ) )
			{
                if ( pEnemy != self->enemy )
				{
					// enemy sighted, attack!
					AI_EnemyAlert( self, pEnemy );
					AI_AddNewGoal( self, GOALTYPE_KILLENEMY, pEnemy );
					AI_SetNextThinkTime( self, 0.1f );
					return TRUE;
				}
				else
				{
					if ( AI_IsStraightPath( self, pEnemy ) == TRUE && !AI_IsOnMovingTrain( pEnemy ) )
					{
						GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
						GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
						if ( pCurrentGoal )
                        {
                            GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
							if ( nGoalType == GOALTYPE_KILLENEMY )
							{
								AI_RestartCurrentGoal( self );
                                return FALSE;
							}
							else
							{
 								AI_AddNewGoal( self, GOALTYPE_KILLENEMY, pEnemy );
							}
                        }
                        else
                        {
                            AI_AddNewGoal( self, GOALTYPE_KILLENEMY, pEnemy );
                        }

						AI_SetNextThinkTime( self, 0.1f );
						return TRUE;
					}
				}
            }
		}
	}

    return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_HandleTaskFinishTime
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_HandleTaskFinishTime( userEntity_t *self, TASKTYPE nCurrentTaskType, const CVector &lastPosition )
{
	_ASSERTE( self );

	playerHook_t *hook = AI_GetPlayerHook( self );

	// the following code makes sure that a monster does not get stuck for a long time
	if ( AI_IsAlive( self ) && self->movetype != MOVETYPE_NONE )
	{
		float fDistanceTraveled = VectorDistance( self->s.origin, lastPosition );
        float fVelocity = self->velocity.Length();
		if ( fVelocity > 0.0f && fDistanceTraveled > 0.0f )
		{
			hook->fTenUpdatesDistance += fDistanceTraveled;

			hook->nMovingCounter++;
			if ( (hook->nMovingCounter % 10) == 0 )
			{
				float fEnoughMoveDistance = fVelocity * 0.125f;

				if ( hook->fTenUpdatesDistance < fEnoughMoveDistance )
				{
					AI_RestartCurrentGoal( self );
				}
				else
				{
					hook->fTenUpdatesDistance = 0.0f;
				}
			}
		}
		else
		{
			hook->nMovingCounter = 0;
            hook->fTenUpdatesDistance = 0.0f;
		}

		if ( AI_IsOverTaskFinishTime( hook ) )
		{
			if ( nCurrentTaskType == TASKTYPE_CHASE )
			{
				AI_RemoveCurrentGoal( self );
			}
			else
			{
				AI_RemoveCurrentTask( self, FALSE );
				return;
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_CheckWaterDamage
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define WATER_LEVEL_FEET	1
#define WATER_LEVEL_BODY	2
#define WATER_LEVEL_HEAD	3

#define WATER_DAMAGE_LAVA	5
#define WATER_DAMAGE_NITRO	50
#define WATER_DAMAGE_SLIME	2
MAPNODE_PTR NODE_FindClosestGroundNode( userEntity_t *self );
void AI_CheckWaterDamage( userEntity_t *self )
{
	float			fDamage = 0;
	playerHook_t	*hook = AI_GetPlayerHook( self );

	if(!AI_IsAlive(self))
		return;
	if( self->movetype != MOVETYPE_SWIM ) // SCG[11/1/99]: Non water creatures in water
	{
		// SCG[11/1/99]: Do drowning damage
		if( self->waterlevel == WATER_LEVEL_HEAD && self->watertype & CONTENTS_WATER )
		{
			//NSS[11/27/99]:For monsters spawned in water that shouldn't be.  Arrg?
			if(hook->drown_time == 0.0f)
				hook->drown_time = gstate->time;
			fDamage = ( gstate->time - hook->drown_time ) * 0.05;
			if( ( hook->drown_time <= gstate->time ) && ( hook->pain_finished < gstate->time ) )
			{
				com->Damage( 
					self, 
					self, 
					self, 
					zero_vector,
					zero_vector, 
					fDamage, 
					DAMAGE_DROWN | DAMAGE_NO_BLOOD );
			}
			if(!(hook->ai_flags & AI_DROWNING) && (hook->ai_flags & AI_CANSWIM))
			{
				MAPNODE_PTR Node	=	NODE_FindClosestGroundNode( self);
				AI_SetStateRunning(hook);
				if(Node)
				{
					AI_AddNewGoal(self,GOALTYPE_MOVETOLOCATION,Node->position);
				}
				hook->ai_flags		|=	AI_DROWNING;	
			}
		}
		else if( self->waterlevel > 0 )	// SCG[11/1/99]: Do non-drowning damage from lava, slime, etc.
		{
			if( self->watertype & ( CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_NITRO) )
			{
				if( self->watertype & CONTENTS_LAVA )
				{
					fDamage = self->waterlevel * WATER_DAMAGE_LAVA;
				}
				else if ( self->watertype & CONTENTS_NITRO && !(self->fragtype & FRAGTYPE_ROBOTIC))
				{
					fDamage = self->waterlevel * WATER_DAMAGE_NITRO;
				}
				else if( self->watertype & CONTENTS_SLIME )
				{
					fDamage = self->waterlevel * WATER_DAMAGE_SLIME;
				}

				com->Damage( 
					self, 
					self, 
					self, 
					zero_vector,
					zero_vector, 
					fDamage, 
					DAMAGE_DROWN | DAMAGE_NO_BLOOD );
			}
		}
		else //NSS[11/27/99]:Otherwise set the drown_time to 10 seconds past the current state time.
		{
			hook->drown_time = gstate->time + 10.0f;
			hook->ai_flags		&=	~AI_DROWNING;
		}
	}
	else	// SCG[11/1/99]: Water creatures out of water
	{
		if( self->waterlevel == 0 )
		{
			if( ( hook->drown_time <= gstate->time ) && ( hook->pain_finished < gstate->time ) )
			{
				com->Damage( 
					self, 
					self, 
					self, 
					zero_vector,
					zero_vector, 
					( gstate->time - hook->drown_time ) * 0.75, 
					DAMAGE_DROWN | DAMAGE_NO_BLOOD );
			}
		}
		else //NSS[11/27/99]:Otherwise set the drown_time to 10 seconds past the current state time.
		{
			hook->drown_time = gstate->time + 10.0f;
		}
	}

}

int AI_HasTaskInQue( userEntity_t *self, TASKTYPE SearchTask );

// ----------------------------------------------------------------------------
// NSS[1/4/00]:
// Name:		AI_TaskThink
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_TaskThink( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	int bScriptActionMode = FALSE;


	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( pGoalStack );
	GOALTYPE nGoalType = GOALSTACK_GetCurrentGoalType( pGoalStack );
	if ( nGoalType == GOALTYPE_SCRIPTACTION || nGoalType == GOALTYPE_WHENUSED_SCRIPTACTION ||
		 nGoalType == GOALTYPE_PATHFOLLOW || nGoalType == GOALTYPE_CAMBOT_PATHFOLLOW)
	{
		bScriptActionMode = TRUE;
	}
	
	// NSS[2/23/00]:
	if(!AI_IsAlive(self) && !AI_HasTaskInQue(self,TASKTYPE_DIE))
	{
		AI_AddNewTaskAtFront(self,TASKTYPE_DIE);
	}
	
	
	//NSS[11/16/99]:Trap to see if we have seen an "enemy" or not
	if(self->enemy && !(hook->ai_flags&AI_SEENPLAYER) && self->flags&FL_MONSTER && nGoalType == GOALTYPE_KILLENEMY)
	{
		if(AI_IsAlive(self->enemy) && (self->enemy->flags & (FL_CLIENT|FL_BOT)))
			hook->ai_flags |= AI_SEENPLAYER;
	}
	
	
	if ( !bScriptActionMode && !(self->flags & FL_BOT) && !(hook->ai_flags&AI_SEENPLAYER) && ai_client_near(self) == FALSE && AI_IsAlive(self))
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		// NSS[2/14/00]:This zero velocity NEEDS to be here.  This is in the event that they were moving in a 'frictionless' environment(ie. AIR/WATER)
		// They would keep moving at that vector until they collided with something.  Not good.  Thus, we STOP until the player gets within range.
		AI_ZeroVelocity(self);

		if ( nGoalType == GOALTYPE_KILLENEMY )
		{
			GOAL_Satisfied( pCurrentGoal );
		}
		// save the last goal
        if ( GOALSTACK_GetNumGoals( pGoalStack ) > 1 )
        {
            GOAL_Satisfied( pCurrentGoal );
			AI_RemoveCurrentGoal( self );
        }
		AI_SetNextThinkTime( self, 0.1f );//2.5f;
		return;
	}
	
	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FREEZE )
	{
		return;
	}

	if ( AI_IsAIDisabled() && !(self->flags & FL_BOT) )
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( hook->pGoals );
		if ( pCurrentGoal == NULL )
		{
			AI_SetSpawnValue( self, hook->nSpawnValue );

			pCurrentGoal = GOALSTACK_GetCurrentGoal( hook->pGoals );
			if ( pCurrentGoal == NULL )
			{
				pCurrentGoal = AI_AddNewGoal( self, GOALTYPE_IDLE );
			}
		}

		TASKTYPE nTaskType = GOALSTACK_GetCurrentTaskType( hook->pGoals );
		if ( nTaskType == TASKTYPE_PAIN )
		{
			AI_Pain( self );
			AI_SetNextThinkTime( self, 0.1f );
		}
		else
		{
			while ( GOALSTACK_GetCurrentTaskType( hook->pGoals ) != TASKTYPE_IDLE )
			{
				AI_RemoveCurrentTask( self );
			}

			AI_Idle( self );
			AI_SetNextThinkTime( self, 0.2f );
		}

		return;
	}
	

		
	TASK_PTR pCurrentTask = NULL;
	GOAL_PTR pScriptGoal = NULL;
	if( hook->pScriptGoals != NULL )
	{
		pScriptGoal = GOALSTACK_GetCurrentGoal( hook->pScriptGoals );
	}

	GOAL_PTR pCurrentGoal = NULL;
	if ( pScriptGoal )
	{
		GOALTYPE nGoalType = GOAL_GetType( pScriptGoal );
		if ( nGoalType == GOALTYPE_IDLE && ai_scriptrunning->value == 0 )
		{
			AI_RemoveCurrentScriptGoal(self);
			AI_SetNextThinkTime( self, 0.1f );
			return;
		}
		// do special effect as needed
		AI_DoSpecialEffects( self );

		pCurrentTask = GOAL_GetCurrentTask( pScriptGoal );
		// NSS[12/13/99]:If we have no task then obviously we are done with our Goal.
		if(!pCurrentTask)
		{
			AI_RemoveCurrentScriptGoal(self);
			AI_SetNextThinkTime( self, 0.1f );
			return;
		}

		if ( hook->bInScriptMode == FALSE )
		{
			hook->bInScriptMode = TRUE;
			AI_StartTask( self, pCurrentTask );
            //adam: we quit out of the think now because sometimes running AI_StartTask will
            //actually remove the current goal (pScriptGoal) and task (pCurrentTask).
			AI_SetNextThinkTime( self, 0.1f );
			return;
		}
	}
	else
	{
		// NSS[2/19/00]:Had to to this because pathfollow mode creatures were never getting a chance to finish their new goaltype_killenemy
		/*if ( AI_CheckForEnemy( self ) )
		{
				return;
		}*/
		if(!AI_IsSidekick(hook))
			AI_CheckForEnemy( self );

		// do special effect as needed
		AI_DoSpecialEffects( self );

		pCurrentGoal = GOALSTACK_GetCurrentGoal( hook->pGoals );
		if ( pCurrentGoal )
		{
			if ( GOALSTACK_GetNumGoals( hook->pGoals ) > 1 &&
				 GOAL_GetType( pCurrentGoal ) == GOALTYPE_IDLE )
			{
				AI_RemoveCurrentGoal( self );
				pCurrentGoal = GOALSTACK_GetCurrentGoal( hook->pGoals );
			}
		}

		if ( !pCurrentGoal )
		{
			self->enemy = NULL;
            AI_SetSpawnValue( self, hook->nSpawnValue );
			
			pCurrentGoal = GOALSTACK_GetCurrentGoal( hook->pGoals );
			if ( !pCurrentGoal )
			{
				pCurrentGoal = AI_AddNewGoal( self, GOALTYPE_IDLE );
			}
		}
        _ASSERTE( pCurrentGoal );

#pragma message( "// SCG[11/16/99]: Fix this!" )
		if(pCurrentGoal->pTasks && ( ( void *) pCurrentGoal->pTasks != ( void * ) 0xdddddddd ) )
			pCurrentTask = GOAL_GetCurrentTask( pCurrentGoal );
		else
			return;
		if ( !pCurrentTask )
		{
			AI_RestartCurrentGoal( self );
//            AI_StartNextTask( self );
			pCurrentGoal = GOALSTACK_GetCurrentGoal( hook->pGoals );
			_ASSERTE( pCurrentGoal );
			pCurrentTask = GOAL_GetCurrentTask( pCurrentGoal );
            if ( !pCurrentTask )
            {
                // just remove this damn goal
                AI_RemoveCurrentGoal( self );
			    pCurrentGoal = GOALSTACK_GetCurrentGoal( hook->pGoals );
			    if ( !pCurrentGoal )
			    {
				    pCurrentGoal = AI_AddNewGoal( self, GOALTYPE_IDLE );
			    }
        		pCurrentTask = GOAL_GetCurrentTask( pCurrentGoal );
            }
		}
		_ASSERTE( pCurrentTask );

		if ( hook->bInScriptMode == TRUE )
		{
			
			AI_StartTask( self, pCurrentTask );
			
			hook->bInScriptMode = FALSE;

            //adam: same as above, we quit out of the think now because sometimes running AI_StartTask will
            //actually remove the current goal (pScriptGoal) and task (pCurrentTask).
			AI_SetNextThinkTime( self, 0.1f );
			return;
		}
	}
	
	// NSS[1/11/00]:Switch to prevent monsters from doing things while in cinematic mode.
	if(gstate->bCinematicPlaying && !hook->bInScriptMode && !(self->flags & FL_CINEMATIC))
	{
		self->s.renderfx	|= RF_NODRAW;
		self->svflags		|= SVF_DEADMONSTER;
		self->nextthink		= gstate->time + 0.1f;
		return;
	}
	else if(!gstate->bCinematicPlaying && self->s.renderfx & RF_NODRAW  && AI_IsAlive(self) && !(self->flags & FL_BOT))
	{
		
		if(self->s.renderfx & RF_NODRAW)
		{
			self->s.renderfx	&= ~RF_NODRAW;
			
			if(hook->type == TYPE_CAMBOT)// NSS[3/20/00]:A nice special hack.. 
			{
				if(self->curWeapon && self->curWeapon->use)
				{
					self->curWeapon->use( self->curWeapon, self );
				}
			}
		}
		if(self->svflags	& SVF_DEADMONSTER)
		{
			self->svflags		&= ~SVF_DEADMONSTER;
		}
		// NSS[3/12/00]:Blow myself up if I am inside something.
		tr = gstate->TraceBox_q2( self->s.origin, self->s.mins, self->s.maxs, self->s.origin, self, MASK_MONSTERSOLID );
		if(tr.startsolid && tr.ent->flags & (FL_CLIENT|FL_BOT) && tr.ent != self)
		{
			com->Damage(self,self, self, self->s.origin, zero_vector, 1000.0f, DAMAGE_INERTIAL);
		}
	}

    TASKTYPE nCurrentTaskType = TASK_GetType( pCurrentTask );
	switch ( nCurrentTaskType )
	{
		case TASKTYPE_IDLE:
		{
			AI_Idle( self );
			AI_SetNextThinkTime( self, 0.2f );
			break;
		}
		case TASKTYPE_WANDER:
		{
			if(pCurrentGoal->nGoalType == GOALTYPE_WANDER && self->enemy && AI_IsVisible(self,self->enemy) && !(hook->ai_flags & AI_IGNORE_PLAYER))
			{
				AI_RemoveCurrentGoal(self);
			}
			else
			{
				AI_Wander( self );
			}
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_ATTACK:
		{
			AI_Attack( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_MOVE:
		{
			break;
		}
		case TASKTYPE_CHASE:
		{
			AI_Chase( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_CHASEATTACK:
		{
			AI_ChaseAttack( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_CHASEATTACKSTRAFING:
		{
			AI_ChaseAttackStrafing( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_FOLLOWWALKING:
		{
			AI_FollowWalking( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_FOLLOWRUNNING:
		{
			AI_FollowRunning( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
        case TASKTYPE_FOLLOWSIDEKICKWALKING:
        {
            AI_FollowSidekickWalking( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }
        case TASKTYPE_FOLLOWSIDEKICKRUNNING:
        {
            AI_FollowSidekickRunning( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }
		case TASKTYPE_JUMPUP:
		{
			if ( hook->fnJumpUp )
			{
				hook->fnJumpUp( self );
				AI_SetNextThinkTime( self, 0.1f );
			}
			break;
		}
		case TASKTYPE_JUMPFORWARD:
		{
			if ( hook->fnJumpForward )
			{
				hook->fnJumpForward( self );
				AI_SetNextThinkTime( self, 0.1f );
			}
			break;
		}
		case TASKTYPE_SWITCH:
		{
			break;
		}
		case TASKTYPE_HIDE:
		{
			AI_Hide( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_SNIPE:
		{
			AI_Snipe( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_AMBUSH:
		{
			break;
		}
		case TASKTYPE_RUNAWAY:
		{
			AI_RunAway( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
        case TASKTYPE_BRIEFCOVER:
        {
            AI_BriefCover( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }
		case TASKTYPE_WAITFORTRAINTOCOME:
		{
			AI_WaitForTrainToCome( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_WAITFORTRAINTOSTOP:
		{
			AI_WaitForTrainToStop( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_WAITFORPLATFORMTOCOME:
		{
			break;
		}
		case TASKTYPE_WAITFORPLATFORMTOSTOP:
		{
			break;
		}
		case TASKTYPE_WAITFORDOORTOOPEN:
		{
			break;
		}
		case TASKTYPE_WAITFORDOORTOCLOSE:
		{
			break;
		}
		case TASKTYPE_MOVEINTOPLATFORM:
		{
			break;
		}
		case TASKTYPE_MOVEOUTOFPLATFORM:
		{
			break;
		}
		case TASKTYPE_PATROL:
		{
			if( hook->type == TYPE_DOOMBAT || hook->type == TYPE_GRIFFON || hook->type == TYPE_HARPY )
			{
				AI_Fly_Patrol( self );
			}
			else
			{
				AI_Patrol( self );
			}
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_PAIN:
		{
			AI_Pain( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_DIE:
		{
			AI_Die( self );
			if ( self->nextthink <= gstate->time )
			{
				AI_SetNextThinkTime( self, 0.1f );
			}
			break;
		}
		case TASKTYPE_FREEZE:
		{
			AI_Freeze( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_MOVETOLOCATION:
		{

			AI_MoveToLocation( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_MOVETOEXACTLOCATION:
		{
			AI_MoveToExactLocation( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
        case TASKTYPE_WALKTOLOCATION:
        {
            AI_WalkToLocation( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }
		case TASKTYPE_GOINGAROUNDOBSTACLE:
		{
			AI_GoingAroundObstacle( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_FLYTOLOCATION:
		{
			AI_FlyToLocation( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_SWOOPATTACK:
		{
			AI_SwoopAttack( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_TOWARDINTERMEDIATEPOINT:
		{
			AI_TowardIntermediatePoint( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_TOWARDINTERMEDIATEPOINT2:
		{
			AI_TowardIntermediatePoint2( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_AIRRANGEDATTACK:
		{
			AI_AirRangedAttack( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_AIRMELEEATTACK:
		{
			AI_AirMeleeAttack( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_SWIMTOLOCATION:
		{
			AI_SwimToLocation( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_FACEANGLE:
		{
			AI_FaceAngle( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_WAIT:
		{
			AI_Wait( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_WAITFORNOCOLLISION:
		{
			AI_WaitForNoCollision( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_BACKUPALLATTRIBUTES:
		{
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_RESTOREALLATTRIBUTES:
		{
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_MODIFYTURNATTRIBUTE:
		{
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_MODIFYRUNATTRIBUTE:
		{
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_MODIFYWALKATTRIBUTE:
		{
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_PLAYANIMATION:
		{
            AI_PlayAnimation( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_SETIDLEANIMATION:
		{
            AI_SetIdleAnimation( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_RANDOMWANDER:
		{
			AI_RandomWander( self );
			AI_SetNextThinkTime( self, 0.1f );
            break;
		}
		case TASKTYPE_STARTUSINGWALKSPEED:
        {
			AI_SetNextThinkTime( self, 0.1f );
            break;
        }
		case TASKTYPE_STARTUSINGRUNSPEED:
		{
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_ACTIVATESWITCH:
		{
			break;
		}
		case TASKTYPE_MOVETOENTITY:
		{
			AI_MoveToEntity( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
        case TASKTYPE_MOVETOOWNER:
        {
            AI_MoveToOwner( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }
		case TASKTYPE_STAND:
		{
			break;
		}				
		case TASKTYPE_TAKECOVER:
		{
			AI_TakeCover( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_TAKECOVER_ATTACK:
		{
			AI_TakeCover_Attack( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;		
		}
		case TASKTYPE_STAY:
		{
			AI_Stay( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_FLYINGAROUNDOBSTACLE:
		{
			AI_FlyingAroundObstacle( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_PATHFOLLOW:
		{
			AI_PathFollow( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_DODGE:
		{
			AI_Dodge( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_SIDESTEP:
		{
			AI_SideStep( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_GOINGAROUNDENTITY:
		{
			AI_GoingAroundEntity( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_MOVETOSNIPE:
		{
			AI_MoveToSnipe( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_STRAFE:
		{
			AI_Strafe( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_CHASEEVADE:
		{
			AI_ChaseEvade( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_TAKEOFF:
		{
			AI_TakeOff( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_DROP:
		{
			AI_Drop( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_CHASESIDESTEPLEFT:
		{
			AI_ChaseSideStepLeft( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_CHASESIDESTEPRIGHT:
		{
			AI_ChaseSideStepRight( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_CHARGETOWARDENEMY:
		{
			AI_ChargeTowardEnemy( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_RETREATTOOWNER:
		{
			AI_RetreatToOwner( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_SEQUENCETRANSITION:
		{
			AI_SequenceTransition( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_FINISHCURRENTSEQUENCE:
		{
			AI_FinishCurrentSequence( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_JUMPTOWARDPOINT:
		{
			AI_JumpTowardPoint( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_SHOTCYCLERJUMP:
		{
			AI_ShotCyclerJump( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_MOVEUNTILVISIBLE:
		{
			AI_MoveUntilVisible( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_USEPLATFORM:
		{
			AI_UsePlatform( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_MOVEDOWN:
		{
			AI_MoveDown( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_USE:
		{
			AI_Use( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_COWER:
		{
			AI_Cower( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_UPLADDER:
		{
			AI_UpLadder( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_DOWNLADDER:
		{
			AI_DownLadder( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_WAITONLADDERFORNOCOLLISION:
		{
			AI_WaitOnLadderForNoCollision( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
        case TASKTYPE_PLAYSOUNDTOEND:
        {
            AI_PlaySoundToEnd( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }
        case TASKTYPE_TELEPORT:
        {
            AI_Teleport( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }
        case TASKTYPE_PLAYANIMATIONANDSOUND:
        {
            AI_PlayAnimationAndSound( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }
        case TASKTYPE_PLAYSOUND:
        {
            AI_PlaySound( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }
        case TASKTYPE_MOVETOENTITYUNTILVISIBLE:
        {
            AI_MoveToEntityUntilVisible( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }

		case TASKTYPE_MOVEAWAY:
		{
			AI_MoveAway( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}

		case TASKTYPE_ACTION_PLAYANIMATION:
		{
			AI_ActionPlayAnimation( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_ACTION_PLAYPARTIALANIMATION:
		{
			AI_ActionPlayPartialAnimation( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_ACTION_PLAYSOUND:
		{
			AI_ActionPlaySound( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_ACTION_SPAWN:
		{
			AI_ActionSpawn( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_ACTION_SETMOVINGANIMATION:
		{
			AI_ActionSetMovingAnimation( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_ACTION_SETSTATE:
		{
			AI_ActionSetState( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_ACTION_DIE:
		{
			AI_ActionDie( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_ACTION_STREAMSOUND:
		{
			AI_ActionStreamSound( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_ACTION_SENDMESSAGE:
		{
			AI_ActionSendMessage( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_ACTION_SENDURGENTMESSAGE:
		{
			AI_ActionSendUrgentMessage( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_ACTION_CALL:
		{
			AI_ActionCall( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}			
		case TASKTYPE_ACTION_RANDOMSCRIPT:
		{
			AI_ActionRandomScript( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_ACTION_COMENEAR:
		{
			AI_ActionComeNear( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_ACTION_REMOVE:
		{
			AI_ActionRemove( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_ACTION_LOOKAT:
		{
			AI_ActionLookAt( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_ACTION_STOPLOOK:
		{
			AI_ActionStopLook( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
        case TASKTYPE_ACTION_ATTACK:
        {
            AI_ActionAttack( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }
        case TASKTYPE_ACTION_PRINT:
        {
            AI_ActionPrint( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }
        case TASKTYPE_WAITUNTILNOOBSTRUCTION:
        {
            AI_WaitUntilNoObstruction( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }
        case TASKTYPE_MOVETORETREATNODE:
        {
            AI_MoveToRetreatNode( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }
        case TASKTYPE_STOPENTITY:
        {
            AI_StopEntityTask( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }

		// monster specific tasks should be placed here
		case TASKTYPE_SPECIAL:
		{
			break;
		}
		case TASKTYPE_INMATER_GOTOPRISONER:
		{
			INMATER_GotoPrisoner( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_INMATER_WAITATPRISONER:
		{
			INMATER_WaitAtPrisoner( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
/*
		case TASKTYPE_INMATER_KILLPRISONER:
		{
			INMATER_KillPrisoner( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
*/
		case TASKTYPE_BUBOID_GETOUTOFCOFFIN:
		{
			BUBOID_GetOutOfCoffin( self  );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_BUBOID_MELT:
		{
			BUBOID_Melt( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_BUBOID_MELTED:
		{
			BUBOID_Melted( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_BUBOID_UNMELT:
		{
			BUBOID_Unmelt( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_PRISONER_WANDER:
		{
			PRISONER_Wander( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_PRISONER_COWER:
		{
			PRISONER_Cower( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
/*
		case TASKTYPE_PRISONER_INMATERHASITINME:
		{
			PRISONER_InmaterHasItInMe( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
*/
		case TASKTYPE_SLUDGEMINION_SCOOP:
		{
			SLUDGEMINION_Scoop( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_SLUDGEMINION_DUMP:
		{
			SLUDGEMINION_Dump( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_CAMBOT_PATHFOLLOW:
		{
			CAMBOT_PathFollow( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_CAMBOT_FOLLOWPLAYER:
		{
			CAMBOT_FollowPlayer( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
//		case TASKTYPE_LABWORKER_WORK:
//		{
//			LABWORKER_Work( self );
//			AI_SetNextThinkTime( self, 0.1f );
//			break;
//		}
		case TASKTYPE_THUNDERSKEET_HOVER:
		{
			THUNDERSKEET_Hover( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_THUNDERSKEET_DARTTOWARDENEMY:
		{
			THUNDERSKEET_DartTowardEnemy( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_THUNDERSKEET_ATTACK:
		{
			THUNDERSKEET_Attack( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_SKEETER_HOVER:
		{
			SKEETER_Hover( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_SKEETER_DARTTOWARDENEMY:
		{
			SKEETER_DartTowardEnemy( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_SKEETER_ATTACK:
		{
			SKEETER_Attack( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		////////////////HARPY/////////////////////
		case TASKTYPE_HARPY_ATTACK_THINK:
		{
			HARPY_Attack_Think( self );
			break;
		}
		case TASKTYPE_HARPY_CHASE:
		{
			HARPY_Chase( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_HARPY_INITIATE_AIR:
		{
			HARPY_InitiateAir( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_HARPY_INITIATE_GROUND:
		{
			HARPY_InitiateGround( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_HARPY_AIR:
		{
			HARPY_Air( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_HARPY_GROUND:
		{
			HARPY_Ground( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_HARPY_ATTACK:
		{
			HARPY_Attack( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_HARPY_MOVETOLOCATION:
		{
			HARPY_MoveToLocation (self);
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_HARPY_DODGE:
		{
			HARPY_Dodge(self);
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}

		/////////////////MEDUSA/////////////////
		case TASKTYPE_MEDUSA_CHASE:
		{
			MEDUSA_Chase( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_MEDUSA_RATTLE:
		{
			MEDUSA_Rattle( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_MEDUSA_GAZE:
		{
			MEDUSA_Gaze( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_MEDUSA_STOPGAZING:
		{
			MEDUSA_StopGazing( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_PSYCLAW_JUMPUP:
		{
			PSYCLAW_JumpUp( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;		
		}
		////////////////FROGINATOR/////////////////
		case TASKTYPE_FROG_ATTACK_THINK:
		{
			FROG_Attack_Think( self );//next think gets set within this function
			break;
		}
		case TASKTYPE_FROG_JUMP:
		{
			FROG_Jump( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_FROG_ATTACK:
		{
			FROG_Attack( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_FROG_CHASE:
		{
			FROG_Chase( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		///////////////CHAINGANG///////////////////
		case TASKTYPE_CHAINGANG_ATTACK_THINK:
		{
			CHAINGANG_Attack_Think( self );
			break;
		}
		case TASKTYPE_CHAINGANG_CHASE:
		{
			CHAINGANG_Chase( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_CHAINGANG_INITIATE_AIR:
		{
			CHAINGANG_InitiateAir( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_CHAINGANG_INITIATE_GROUND:
		{
			CHAINGANG_InitiateGround( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_CHAINGANG_AIR:
		{
			CHAINGANG_Air( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_CHAINGANG_GROUND:
		{
			CHAINGANG_Ground( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_CHAINGANG_ATTACK:
		{
			CHAINGANG_Attack( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_CHAINGANG_MOVETOLOCATION:
		{
			CHAINGANG_MoveToLocation (self);
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_CHAINGANG_DODGE:
		{
			CHAINGANG_Dodge(self);
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		////////////////GRIFFON///////////////////
		case TASKTYPE_GRIFFON_ATTACK_THINK:
		{
			GRIFFON_Attack_Think( self );
			break;
		}
		case TASKTYPE_GRIFFON_CHASE:
		{
			GRIFFON_Chase( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_GRIFFON_FLYAWAY:
		{
			GRIFFON_FlyAway( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_GRIFFON_JUMP:
		{
			GRIFFON_Jump( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_GRIFFON_ATTACK:
		{
			GRIFFON_Attack( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_GRIFFON_GROUND:
		{
			GRIFFON_Ground( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_GRIFFON_LAND:
		{
			GRIFFON_Land( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		////////////////DRAGON///////////////////
		case TASKTYPE_DRAGON_HOVER:
		{
			DRAGON_Hover( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_DRAGON_ATTACK_THINK:
		{
			DRAGON_Attack_Think( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_DRAGON_ATTACK:
		{
			DRAGON_Attack( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_DRAGON_FLYAWAY:
		{
			DRAGON_FlyAway( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		////////////////GHOST///////////////////
		case TASKTYPE_GHOST_WAKEUP:
		{
			Ghost_WakeUp(self);
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_GHOST_CHASE:
		{
			Ghost_Chase( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_GHOST_SPIRAL:
		{
			Ghost_Spiral_Upward( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_GHOST_ATTACK:
		{
			Ghost_Attack( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_GHOST_DEATH:
		{
			Ghost_Die( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		////////////////DOOMBAT///////////////////
		case TASKTYPE_DOOMBAT_HOVER:
		{
			DOOMBAT_Hover( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_DOOMBAT_CHASE:
		{
			DOOMBAT_Chase( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_DOOMBAT_ATTACK_THINK:
		{
			DOOMBAT_Attack_Think( self );
			
			break;
		}
		case TASKTYPE_DOOMBAT_KAMAKAZI:
		{
			DOOMBAT_Kamakazi( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_DOOMBAT_ATTACK:
		{
			DOOMBAT_Attack( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_DOOMBAT_FLYAWAY:
		{
			DOOMBAT_FlyAway( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		////////////////NHARRE////////////////////
		case TASKTYPE_NHARRE_TELEPORT:
		{
			NHARRE_Teleport(self);
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		////////////////WYNDRAX///////////////////
		case TASKTYPE_WYNDRAX_CHARGE_UP:
		{
			WYNDRAX_Charge_Up( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_WYNDRAX_FIND_WISPMASTER:
		{
			WYNDRAX_Find_WispMaster( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_WYNDRAX_COLLECT_WISPS:
		{
			WYNDRAX_Collect_Wisps( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}

		////////////////THUNDERSKEET///////////////////
		case TASKTYPE_THUNDERSKEET_FLYAWAY:
		{
			THUNDERSKEET_FlyAway( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}

		////////////////SKEETER///////////////////
		case TASKTYPE_SKEETER_FLYAWAY:
		{
			SKEETER_FlyAway( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_SKEETER_PREHATCH:
		{
			SKEETER_PreHatch( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_SKEETER_HATCH:
		{
			SKEETER_Hatch( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_LYCANTHIR_RESURRECT_WAIT:
		{
			LYCANTHIR_ResurrectWait( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_LYCANTHIR_RESURRECT:
		{
			LYCANTHIR_Resurrect( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_BUBOID_RESURRECT_WAIT:
		{
			BUBOID_ResurrectWait( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_BUBOID_RESURRECT:
		{
			BUBOID_Resurrect( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_COLUMN_AWAKEN_WAIT:
		{
			COLUMN_AwakenWait( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_COLUMN_AWAKEN:
		{
			COLUMN_Awaken( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_ROTWORM_JUMPSPRING:
		{
			ROTWORM_JumpSpring( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_ROTWORM_JUMPFLY:
		{
			ROTWORM_JumpFly( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_ROTWORM_JUMPBITE:
		{
			ROTWORM_JumpBite( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_DEATHSPHERE_MOVETOLOCATION:
		{
			DEATHSPHERE_Move( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_DEATHSPHERE_CHARGEWEAPON:
		{
			DEATHSPHERE_ChargeWeapon( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_PROTOPOD_WAIT_TO_HATCH:
		{
			PROTOPOD_WaitToHatch(self);
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_PROTOPOD_HATCH:
		{
			PROTOPOD_Hatch(self);
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
       
		case TASKTYPE_SKINNYWORKER_HIDE:
		{
			SKINNYWORKER_Hide( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_SKINNYWORKER_BACKWALL:
		{
			SKINNYWORKER_BackWall( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_SKINNYWORKER_LOOKOUT:
		{
			SKINNYWORKER_LookOut( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}

        case TASKTYPE_MISHIMAGUARD_RELOAD :
        {
            MISHIMAGUARD_Reload( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }
        case TASKTYPE_SMALLSPIDER_RUNAWAY:
        {
            SMALLSPIDER_RunAway( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }

		case TASKTYPE_BOT_DIE:
		{
			BOT_Die( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_BOT_ROAM:
		{
			BOT_Roam( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_BOT_ENGAGEENEMY:
		{
			BOT_EngageEnemy( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}

		case TASKTYPE_SIDEKICK_DIE:
		{
			SIDEKICK_Die( self );
			AI_SetNextThinkTime( self, 0.1f );
            break;
		}
        case TASKTYPE_SIDEKICK_PICKUPITEM:
        {
            SIDEKICK_PickupItem( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }
		case TASKTYPE_SIDEKICK_USEITEM: //<nss>
		{
			SIDEKICK_UseItem(self);
            AI_SetNextThinkTime( self, 0.1f );
			break;
		}
        case TASKTYPE_SIDEKICK_EVADE:
        {
            SIDEKICK_Evade( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }

        case TASKTYPE_SIDEKICK_STOPGOINGFURTHER:
        {
            SIDEKICK_StopGoingFurther( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }
        case TASKTYPE_SIDEKICK_TELEPORTANDCOMENEAR:
        {
            SIDEKICK_TeleportAndComeNear( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }
        case TASKTYPE_SIDEKICK_COMEHERE:
        {
            SIDEKICK_ComeHere( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }
        case TASKTYPE_SIDEKICK_TELEPORT:
        {
            SIDEKICK_Teleport( self );
            AI_SetNextThinkTime( self, 0.1f );
            break;
        }

		case TASKTYPE_TESTMAP_ROAM:
		{
			AI_TestMapRoam( self );
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
        case TASKTYPE_SIDEKICK_RANDOMWALK:
		{
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_KAGE_SMOKESCREEN:// NSS[1/15/00]:
		{
			KAGE_SmokeScreen(self);
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_KAGE_WAIT:// NSS[1/15/00]:
		{
			KAGE_Wait(self);
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_KAGE_RETURN:// NSS[1/15/00]:
		{
			KAGE_Return(self);
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_KAGE_CREATEPROTECTORS:// NSS[1/15/00]:
		{
			KAGE_CreateProtectors(self);
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		case TASKTYPE_KAGE_CHARGEHEALTH:// NSS[1/15/00]:
		{
			KAGE_ChargeHealth(self);
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
		default:
		{
			//NSS[12/2/99]:We need this here because sidekicks have their own
			//semi-switch for tasks and need to have this set.
			AI_SetNextThinkTime( self, 0.1f );
			break;
		}
	}
	//NSS[11/2/99]: If we are freed don't worry about Finishtimes etc..
	if(!(_stricmp(self->className,"freed")))
		return;
	AI_HandleTaskFinishTime( self, nCurrentTaskType, hook->last_origin );

	hook->last_origin = self->s.origin;

	_ASSERTE( self->nextthink > gstate->time );


	//NSS[11/28/99]:Last thing we check for is if we shouldn't move or not.
	//If not Zero out our velocity before exiting TaskThink.  It is ok for it 
	//to exit before the switch statement due to the fact that the velocity is
	//not set prior to hitting the switch statement from hell.
	if(hook->nSpawnValue & SPAWN_DO_NOT_MOVE)
	{
		self->velocity.Zero();
	}


	//NSS[11/30/99]:And finally.. the very last thing we want to do is water damage...this clears up a lot of problems.
	AI_CheckWaterDamage( self );
	

	// NSS[3/13/00]:If we are not a flying unit nor a sidekick then let's make sure we are facing the ground.
	if(AI_IsGroundUnit( self ) && !AI_IsSidekick(hook) && self->velocity.Length() > 0.0f)
	{
		AI_OrientToFloor(self);
	}

}

void NitroDeath(userEntity_t *self)
{
    com->Damage(self, self, self, zero_vector, zero_vector, 32000, DAMAGE_LAVA | DAMAGE_NO_BLOOD);
}

// ----------------------------------------------------------------------------
//
// Name:		AI_Update
// Description:
//			handles some things that must be done every frame
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define FREEZE_FRAME_DECREMENT			(0.01)
#define FREEZE_INTERVAL					(2.0)
#define FREEZE_DAMAGE					(5.0)
void AI_Update( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	// SCG[11/24/99]: Handle NULL
	if( hook == NULL )
	{
		return;
	}

    AI_ClearExecutedTasks( hook );

	if ( self->flags & FL_INWARP )
	{
		return;
	}

	// determine if the player/bot has left the ground between nodes
	// used for determining when to link nodes back

	//	Q2KILL
	if ( !self->groundEntity )
	{
		hook->bOffGround = TRUE;
	}

	// get the bot's water level for this frame
	ai_water_level( self );

	// do damage to bot if in water/lava/slime
	ai_water_damage( self );
    if (self->s.renderfx & RF_IN_NITRO)
    {
        self->think = NitroDeath;
        AI_SetNextThinkTime( self, 3.0f );
    }

	com->FrameUpdate( self );

	///////////////////////////////////////////////////////////////////////////
	//	turning done here for much smoother turning rates
	//	ang_speed is in degree's per second, so scale ang_speed for each frame
	//	based on gstate->frametime
	///////////////////////////////////////////////////////////////////////////
	
	AI_AdjustAngles(self);
/*
	//	don't turn if DFL_FACENONE or jumping
	if ( !(hook->dflags & DFL_FACENONE) && !(hook->ai_flags & AI_JUMP) )
	{
		ai_adjust_angles(self);
	}
*/

	//if ( hook->dflags & DFL_ORIENTTOFLOOR )
	//{
	//	ai_orient_to_floor(self);
	//}

	///////////////////////////////////////////////////////////////////////////
	//	poisoning
	///////////////////////////////////////////////////////////////////////////

	if ( hook->items & IT_POISON && hook->type != TYPE_MEDUSA )
	{
		//	do damage
		if (hook->poison_next_damage_time <= 0)
		{
			com->Damage (self, self, self, zero_vector, zero_vector, hook->poison_damage, DAMAGE_POISON | DAMAGE_NO_BLOOD);
			hook->poison_next_damage_time = hook->poison_interval;
		}
		//	poison expire
		if (hook->poison_time <= 0)
		{
			hook->items &= ~IT_POISON;
		}
		else
		{
			hook->poison_time -= 0.1f;
//			hook->poison_interval -= 0.1;
			hook->poison_next_damage_time -= 0.1f;
		}
	}


	if (gstate->episode == 4)
	{
		float &freezeLevel = hook->freezeLevel;
		if (freezeLevel > 0)
		{
			if (gstate->time > hook->freezeTime)
			{
				// clamp it so we can't have any insanity...
				if (freezeLevel > 1) freezeLevel = 1;

				// blue flash so the client knows he's freezing...
				com->FlashClient(self, 0.4 * freezeLevel,0.2 ,0.2 ,1.0 , 0.05f );

				gstate->damage_inflicted = 0;

				// do some damage
				float freeze = freezeLevel * FREEZE_DAMAGE;
				com->Damage (self, self, self, zero_vector, zero_vector, freeze, DAMAGE_NO_BLOOD|DAMAGE_IGNORE_ARMOR|DAMAGE_COLD );

//				gstate->Con_Dprintf("Freezelevel: %f - dmg: %f\n",freezeLevel,gstate->damage_inflicted);
				hook->freezeTime = gstate->time + FREEZE_INTERVAL;
			}

			// set up the color
			float r,g,b;
			r=g= 0.7-0.6*freezeLevel;
			if (r < 0.3) {r=g=0.3;}
			b = 0.4 + freezeLevel;
			if (b > .8) b = .8;
			self->s.color.Set(r,g,b);

			// update the info
			if (!( (gstate->episode == 3) && (self->input_entity->waterlevel > 1) ))
				freezeLevel -= FREEZE_FRAME_DECREMENT;
		}
		else
			freezeLevel = 0;

		if ((freezeLevel == 0) && (self->s.color.Length() > 0))
		{
			self->s.color.Set(0,0,0);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_PostThink
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
bool AI_IsTakeCoverType( playerHook_t *hook )
{
	if( !hook )
		return false;

	switch( hook->type )
	{
		case TYPE_MISHIMAGUARD:
		case TYPE_THIEF:
		case TYPE_DWARF:
		case TYPE_FLETCHER:
		case TYPE_ROCKETGANG:
		case TYPE_SEALGIRL:
		case TYPE_SEALCOMMANDO:
		case TYPE_SEALCAPTAIN:
		case TYPE_ROCKETMP:
			return true;
	}

	return false;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_PostThink
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_PostThink( userEntity_t *self )
{
}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_think_register_func()
{
	gstate->RegisterFunc("NitroDeath",NitroDeath);
	gstate->RegisterFunc("AI_TaskThink",AI_TaskThink);
	gstate->RegisterFunc("AI_Update",AI_Update);
	gstate->RegisterFunc("AI_PostThink",AI_PostThink);
}
