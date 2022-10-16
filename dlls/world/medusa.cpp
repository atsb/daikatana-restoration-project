// ==========================================================================
//
//  File:     medusa.cpp
//  Contents: AI functions for Medusa.
//  Author:
//
//  Task List Description:
//  Medusa will be slithering around in a very dark area and has two main
//  attacks: (1) a biting attack using her entire head and (2) a stone gaze
//
//  Before Medusa does her stone gaze, she will make a really loud rattling
//  sound, then two superbright alpha-blended sprites will appear where her
//  eyes are (need a poly tag to anchor these).  If the player is looking at
//  her during this 4 second period, he will turn to stone and the game is
//  over...
//
//  Notes:
//
//  GOALTYPE_MEDUSA_GAZE -- gazing sequence
//  --------------------
//
//  TASKTYPE_MEDUSA_RATTLE:
//  She rattles her tail for a few seconds then goes into Gaze.
//
//  TASKTYPE_MEDUSA_GAZE:
//  She directly faces the player at all times and plays a gaze sound for a
//  few seconds, then goes into Bite if the player is within range or Slither
//  if not.
//
//  TASKTYPE_MEDUSA_STOPGAZING:
//  She directly faces the player at all times and plays a gaze sound for a
//  few seconds, then goes back into her previous goal (normally KILLENEMY)
//
// ==========================================================================
#include "world.h"
#include "ai_utils.h"
//#include "ai_move.h"// SCG[1/23/00]: not used
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_common.h"
#include "ai_func.h"
#include "MonsterSound.h"
#include "ai.h"


/* ***************************** define types ****************************** */

// length of gaze rattle
#define MEDUSA_RATTLE_DURATION 3.0f
// length of gaze stare
#define MEDUSA_GAZE_DURATION 3.0f
// minimum amount of time between gazes
#define MEDUSA_GAZE_MIN_INTERVAL 5.0f
// the color to set a turned-to-stone enemy to
#define STONE_COLOR CVector( 5.0, 5.0, 5.0 )
// the yaw of Medusa's gaze field of view
#define MEDUSA_GAZE_FOV_YAW 35.0f
// the pitch of Medusa's gaze field of view
#define MEDUSA_GAZE_FOV_PITCH 35.0f
// the pitch of Medusa's enemys' gazing field of view
#define MEDUSA_ENEMY_FOV_PITCH 35.0f
// the chance of Medusa spitting
#define MEDUSA_SPIT_CHANCE 0.5f
// the chance of Medusa gazing
#define MEDUSA_GAZE_CHANCE 0.05f
// the distance Medusa can spit
#define MEDUSA_SPIT_DIST 350.0f
// the distance Medusa can gaze
#define MEDUSA_GAZE_DIST 1250.0f

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */

int medusa_check_range( userEntity_t *self, float dist, userEntity_t *enemy );
void medusa_start_attack( userEntity_t *self );
void medusa_attack( userEntity_t *self );
void medusa_set_attack_seq( userEntity_t *self );
void medusa_stone_enemy( userEntity_t *self, userEntity_t *enemy );
int medusa_eye_contact( userEntity_t *self, userEntity_t *enemy );
int medusa_within_ranged_attack_dist( float dist );

/* **************************** Global Variables *************************** */

/* **************************** Global Functions *************************** */

void MEDUSA_StartChase(userEntity_t *self);
void MEDUSA_Chase(userEntity_t *self);
void MEDUSA_StartRattle(userEntity_t *self);
void MEDUSA_Rattle(userEntity_t *self);
void MEDUSA_StartGaze(userEntity_t *self);
void MEDUSA_Gaze(userEntity_t *self);
void MEDUSA_StartStopGazing(userEntity_t *self);
void MEDUSA_StopGazing(userEntity_t *self);

int AI_Check_For_Life( userEntity_t *self, playerHook_t *hook, userEntity_t *pEnemy);

/* ******************************* exports ********************************* */
DllExport void monster_medusa( userEntity_t *self );


// ----------------------------------------------------------------------------
//
// Name:		medusa_check_range
// Description: Checks given distance to determine if it's in range.
// Input:
// Output:
// Note:        The enemy pointer will usually be NULL.
//
// ----------------------------------------------------------------------------
int medusa_check_range(userEntity_t *self, float dist, userEntity_t *enemy)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	// In melee distance?
	if ( dist <= hook->attack_dist )
	{
		return TRUE;
	}

	// In ranged attack distance?
	if ( dist <= MEDUSA_SPIT_DIST && rnd() < MEDUSA_SPIT_CHANCE )
	{
		 return TRUE;
	}
	else
	{
		// In gaze attack distance?
		if ( dist <= MEDUSA_GAZE_DIST && rnd() < MEDUSA_GAZE_CHANCE )
		{
			 return TRUE;
		}
	}

	return FALSE;
}


// Answers the question "Can this distance be reached by the preferred ranged attack?"
int medusa_within_ranged_attack_dist(float dist)
{
	return dist < MEDUSA_SPIT_DIST;
}



// ----------------------------------------------------------------------------
//
// Name:        medusa_start_attack
// Description: Medusa attack start-task function
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void medusa_start_attack( userEntity_t *self )
{
// SCG[1/23/00]: not used	float dist = VectorDistance(self->s.origin, self->enemy->s.origin);
	playerHook_t *hook = AI_GetPlayerHook( self );

	self->velocity.Zero();

	AI_SetTaskFinishTime( hook, 15.0f );

	medusa_set_attack_seq( self );
	

	AI_SetOkToAttackFlag( hook, FALSE );
	

}

// ----------------------------------------------------------------------------
//
// Name:        medusa_set_attack_seq
// Description: sets current sequence to Medusa's attack sequence
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void medusa_set_attack_seq( userEntity_t *self )
{
	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
	playerHook_t *hook = AI_GetPlayerHook( self );
	frameData_t *pSequence;

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	// Within range of melee weapon?
	if ( dist <= hook->attack_dist )
	{
		RELIABLE_UNTRACK(self);
		// bite
		hook->nAttackMode = ATTACKMODE_MELEE;

		// set weapon to bite attack
		self->curWeapon = gstate->InventoryFindItem(self->inventory, "poison bite mild");

		pSequence = FRAMES_GetSequence( self, "ataka" );
		AI_ForceSequence( self, pSequence );
	}
	else
	{
		// choose whether to gaze or perform a ranged attack
		hook->nAttackMode = ATTACKMODE_RANGED;

		// In spitting distance or within gaze chance?
		if ( dist > MEDUSA_SPIT_DIST || rnd() < MEDUSA_GAZE_CHANCE )
		{

			//NSS[11/12/99]:
			//Medusa's eyes.
			//Attach the Sprite Light&Glow and fire stuff 
			trackInfo_t tinfo;

			// clear this variable
			memset(&tinfo, 0, sizeof(tinfo));
			tinfo.fru.Zero();
			tinfo.ent			= self;
			tinfo.srcent		= self;
			tinfo.lightColor.x	= -1.0;//R
			tinfo.lightColor.y	= 0.95;//G
			tinfo.lightColor.z	= -1.0;//B
			tinfo.lightSize		= 100.00;	
			tinfo.scale			= 0.75;
			tinfo.modelindex	= gstate->ModelIndex("models/global/e_flgreen.sp2");
			tinfo.scale2		= 0.75;
			tinfo.modelindex2	= gstate->ModelIndex("models/global/e_flgreen.sp2");

//			sprintf(tinfo.HardPoint_Name,"eye1");
//			sprintf(tinfo.HardPoint_Name2,"eye2");
			Com_sprintf(tinfo.HardPoint_Name,sizeof(tinfo.HardPoint_Name),"eye1");
			Com_sprintf(tinfo.HardPoint_Name2,sizeof(tinfo.HardPoint_Name2),"eye2");
			
			tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_HARDPOINT|TEF_MODELINDEX|TEF_SCALE|TEF_HARDPOINT2|TEF_MODELINDEX2|TEF_SCALE2;
			tinfo.fxflags = TEF_LIGHT|TEF_FX_ONLY|TEF_MODEL|TEF_ATTACHMODEL|TEF_MODEL2;
			tinfo.renderfx = SPR_ALPHACHANNEL;
			
			com->trackEntity(&tinfo,MULTICAST_PVS);

			// add gaze goal
			AI_AddNewGoal( self, GOALTYPE_MEDUSA_GAZE );
		}
		else
		{
			RELIABLE_UNTRACK(self);
			// do ranged attack
			self->curWeapon = gstate->InventoryFindItem(self->inventory, "medusa spit");

			pSequence = FRAMES_GetSequence( self, "atakb" );
			AI_ForceSequence( self, pSequence );
		}
	}
}


// ----------------------------------------------------------------------------
//
// Name:        medusa_attack
// Description: Medusa attack task function
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void medusa_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	
	if ( AI_IsReadyToAttack1( self ) )
	{
		// make sure we face enemy for melee attack
		if ( hook->nAttackMode != ATTACKMODE_MELEE || AI_IsFacingEnemy( self, self->enemy ) )
		{
			ai_fire_curWeapon( self );
		}
	}

	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );
		AI_SetOkToAttackFlag( hook, TRUE );
		if ( !AI_IsWithinAttackDistance( self, dist ) || !AI_IsVisible(self, self->enemy) )
		{
		    AI_SetOkToAttackFlag( hook, TRUE );
			AI_AddNewTaskAtFront( self, TASKTYPE_MEDUSA_CHASE );
		}
		else
		{
			medusa_set_attack_seq( self );
		}
	}
}


// ----------------------------------------------------------------------------
//
// Name:        MEDUSA_GetLineOfSight
// Description: Medusa visibility-seeking task function
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void MEDUSA_StartChase(userEntity_t *self)
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_MEDUSA_CHASE );

	if ( !self->enemy )
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		AIDATA_PTR pAIData = GOAL_GetData(pCurrentGoal);
		self->enemy = pAIData->pEntity;
	}

	float fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );
	if ( AI_IsWithinAttackDistance( self, fDistance ) && AI_IsChaseVisible( self ) )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

    userEntity_t *pEnemy = self->enemy;
    float fXYDistance = VectorXYDistance( self->s.origin, pEnemy->s.origin );
    float fZDistance = VectorZDistance( self->s.origin, pEnemy->s.origin );
    if ( !AI_FindPathToEntity( self, pEnemy ) && AI_CanPath( hook ) &&
         !AI_IsOkToMoveStraight( self, pEnemy->s.origin, fXYDistance, fZDistance ) )
	{
		// do not try another path for 5 seconds
		AI_SetNextPathTime( hook, 5.0f );

		//	no path was found, so move to the retreat node
		AI_RemoveCurrentTask( self );
		AI_AddNewTaskAtFront( self, TASKTYPE_MOVETORETREATNODE );

		return;
	}

    AI_SetStateRunning( hook );
	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
        _ASSERTE( pCurrentTask );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_MEDUSA_CHASE )
		{
			return;
		}
	}

	MEDUSA_Chase( self );

    AI_Dprintf( "Starting TASKTYPE_MEDUSA_CHASE.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, FALSE );

	AI_SetTaskFinishTime( hook, 10.0f );
	AI_SetMovingCounter( hook, 0 );

	
}


// ----------------------------------------------------------------------------
//
// Name:        MEDUSA_GetLineOfSight
// Description: Medusa visibility-seeking task function
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void MEDUSA_Chase(userEntity_t *self)
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	userEntity_t *pEnemy = self->enemy;
    _ASSERTE( pEnemy );
	
	if ( AI_Check_For_Life( self, hook, pEnemy) )//<nss>
	{
		return;
	}

	float fDistance = VectorDistance( self->s.origin, pEnemy->s.origin );
	if ( (fDistance > hook->active_distance && !hook->pPathList->pPath) )
	{
		// the enemy is out of range or dead, give up
		AI_RemoveCurrentGoal( self );
		return;
	}

	int bEnemyVisible = FALSE;
	if ( hook->dflags & DFL_RANGEDATTACK )
	{
		bEnemyVisible = AI_IsCompletelyVisible( self, pEnemy );

		if ( bEnemyVisible == LEFTSIDE_VISIBLE )
		{
	        CVector sideStepPoint;
            int bSuccess = AI_ComputeChaseSideStepPoint( self, pEnemy, RIGHT, sideStepPoint );
	        if ( bSuccess )
	        {
			    // side step right
			    AI_AddNewTaskAtFront( self, TASKTYPE_CHASESIDESTEPLEFT );
			    return;
		    }
        }
		else
		if ( bEnemyVisible == RIGHTSIDE_VISIBLE )
		{
	        CVector sideStepPoint;
            int bSuccess = AI_ComputeChaseSideStepPoint( self, pEnemy, LEFT, sideStepPoint );
	        if ( bSuccess )
	        {
			    // side step left
			    AI_AddNewTaskAtFront( self, TASKTYPE_CHASESIDESTEPRIGHT );
			    return;
		    }
        }
	}
	else
	{
		bEnemyVisible = AI_IsVisible(self, pEnemy);
	}

	// on ground
    if ( bEnemyVisible && AI_IsWithinAttackDistance( self, fDistance ) )
	{
		
		AI_RemoveCurrentTask( self );
		return;
	}

	float fXYDistance = VectorXYDistance( self->s.origin, pEnemy->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pEnemy->s.origin );
	if ( AI_IsOkToMoveStraight( self, pEnemy->s.origin, fXYDistance, fZDistance ) )
	{
		AI_MoveTowardPoint( self, pEnemy->s.origin, FALSE );
		hook->nMoveCounter = 0;
	}
	else
	{
		if ( AI_ShouldFollow( self ) )
		{
			if ( !AI_HandleUse( self ) )//<nss> marker.
			{
				// Retreat for the special case of being on different levels of the lair.
				if ( !bEnemyVisible && pEnemy->s.origin.z > self->s.origin.z + 150.0f &&
				     self->s.origin.z < -1890.0f && pEnemy->s.origin.z > -1890.0f )
				{
					AI_RemoveCurrentTask( self );
					AI_AddNewTaskAtFront( self, TASKTYPE_MOVETORETREATNODE );
				}

				// test to see if the last path point is close to the enemy's position
				if ( !AI_IsPathToEntityClose( self, pEnemy ) )
				{
					if ( !AI_FindPathToEntity( self, pEnemy ) && AI_CanPath( hook ) )
					{
						// do not try another path for 5 seconds
						AI_SetNextPathTime( hook, 5.0f );

                        float fXYDistance = VectorXYDistance( self->s.origin, pEnemy->s.origin );
                        float fZDistance = VectorZDistance( self->s.origin, pEnemy->s.origin );
                        if ( !AI_IsOkToMoveStraight( self, pEnemy->s.origin, fXYDistance, fZDistance ) )
                        {
							//	no path was found, so move to the retreat node
							AI_RemoveCurrentTask( self );
							AI_AddNewTaskAtFront( self, TASKTYPE_MOVETORETREATNODE );
                        }

						return;
					}
				}

				if ( AI_Move( self ) == FALSE )
				{
					PATHLIST_KillPath( hook->pPathList );
				}
				
				hook->nMoveCounter++;
			}
		}
	}

	if ( bEnemyVisible )
	{
		// extend the time
		AI_SetTaskFinishTime( hook, 10.0f );
	}
}


// ----------------------------------------------------------------------------
//
// Name:        MEDUSA_StartRattle
// Description: Medusa rattle start-task function
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void MEDUSA_StartRattle(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	frameData_t *pSequence;

	AI_ZeroVelocity( self );
	AI_FaceTowardPoint( self, self->enemy->s.origin );

	// force get up into rattle sequence
	pSequence = FRAMES_GetSequence( self, "atakc" );
	AI_ForceSequence( self, pSequence );


	// set task duration
	AI_SetTaskFinishTime( hook, MEDUSA_RATTLE_DURATION );

	// not OK to attack or to take pain during gaze goal
	AI_SetOkToAttackFlag( hook, FALSE );
	hook->pain_finished = gstate->time + 4 * (MEDUSA_RATTLE_DURATION + MEDUSA_GAZE_DURATION);

	AI_Dprintf( "Starting TASKTYPE_MEDUSA_RATTLE.\n" );
}

// ----------------------------------------------------------------------------
// NSS[3/10/00]:
// Name:        MEDUSA_Rattle
// Description: Medusa rattle task function
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void MEDUSA_Rattle(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	ai_frame_sounds( self );

	if((int)gstate->time % 2)
	{
		CVector Color;
		Color.Set(0.65,0.65f,0.65);
		com->FlashClient(self->enemy, 0.35f,Color.x ,Color.y ,Color.z , 1.0f );	
		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/we_gravela.wav"),1.0f, hook->fMinAttenuation, hook->fMaxAttenuation );
	}
	
	// play gaze loop once done getting up
	if ( AI_IsEndAnimation( self ) )
	{
		frameData_t *pSequence = FRAMES_GetSequence( self, "atakd" );
		AI_ForceSequence( self, pSequence );
	}
}

// ----------------------------------------------------------------------------
//
// Name:        MEDUSA_StartGaze
// Description: Medusa gaze start-task function
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void MEDUSA_StartGaze(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	// play gaze loop
	frameData_t *pSequence = FRAMES_GetSequence( self, "atakd" );
	AI_ForceSequence( self, pSequence );

	// set task duration
	AI_SetTaskFinishTime( hook, MEDUSA_GAZE_DURATION );

	AI_Dprintf( "Starting TASKTYPE_MEDUSA_GAZE.\n" );

	AI_SetOkToAttackFlag( hook, FALSE );
}

// ----------------------------------------------------------------------------
//
// Name:        MEDUSA_Gaze
// Description: Medusa gaze task function
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void MEDUSA_Gaze(userEntity_t *self)
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	ai_frame_sounds( self );

	if(self->enemy && self->enemy->flags & FL_CLIENT)
	{
		CVector Color;
		Color.Set(0.65,0.65f,0.65);
		com->FlashClient(self->enemy, 0.35f,Color.x ,Color.y ,Color.z , 1.0f );
	}
	
	// keep playing gaze loop
	if ( AI_IsEndAnimation( self ) )
	{
		frameData_t *pSequence = FRAMES_GetSequence( self, "atakd" );
		AI_ForceSequence( self, pSequence );
		RELIABLE_UNTRACK(self);
	}

	// turn enemy to stone if they peek
	if ( self->enemy && medusa_eye_contact( self, self->enemy ) )
	{
		// Stop LOOKING AT MEEE!!!
		medusa_stone_enemy( self, self->enemy );
		RELIABLE_UNTRACK(self);
	}

}


// ----------------------------------------------------------------------------
//
// Name:        medusa_eye_contact
// Description: evaluates whether Medusa is making eye contact with an enemy
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

int medusa_eye_contact( userEntity_t *self, userEntity_t *enemy )
{
	_ASSERTE( self && enemy );
	playerHook_t *pEnemyHook = AI_GetPlayerHook( enemy );
	CVector dir = self->s.origin - enemy->s.origin;
	CVector angleTowardSelf, angleTowardEnemy;

	if ( !AI_IsVisible( self, self->enemy ) )
		return FALSE;

	// normalize direction to self vector
	dir.Normalize();

	VectorToAngles( dir, angleTowardSelf );
	VectorToAngles( -dir, angleTowardEnemy );

	float fEnemyTowardSelfX = AngleMod(angleTowardSelf.x);
	float fEnemyTowardSelfY = AngleMod(angleTowardSelf.y);
	float fEnemyFacingAngleX = AngleMod(enemy->s.angles[PITCH]);
	float fEnemyFacingAngleY = AngleMod(enemy->s.angles[YAW]);

	float fYawDiff = fabs( fEnemyTowardSelfY - fEnemyFacingAngleY );
	float fPitchDiff = fabs( fEnemyTowardSelfX - fEnemyFacingAngleX );

	// check whether view angles are within tolerance
	if ( (fYawDiff < pEnemyHook->fov * 0.5f || fYawDiff > (360.0f - pEnemyHook->fov * 0.5f)) &&
	     (fPitchDiff < MEDUSA_ENEMY_FOV_PITCH || fPitchDiff > (360.0f - MEDUSA_ENEMY_FOV_PITCH)) )
	{
		// OK, they're facing us... now are we facing them?
		float fSelfTowardEnemyX = AngleMod(angleTowardEnemy.x);
		float fSelfTowardEnemyY = AngleMod(angleTowardEnemy.y);
		float fSelfFacingAngleX = AngleMod(self->s.angles[PITCH]);
		float fSelfFacingAngleY = AngleMod(self->s.angles[YAW]);

		fYawDiff = fabs( fSelfTowardEnemyY - fSelfFacingAngleY );
		fPitchDiff = fabs( fSelfTowardEnemyX - fSelfFacingAngleX );

		if ( (fYawDiff < MEDUSA_GAZE_FOV_YAW || fYawDiff > (360.0f - MEDUSA_GAZE_FOV_YAW)) &&
			 (fPitchDiff < MEDUSA_GAZE_FOV_PITCH || fPitchDiff > (360.0f - MEDUSA_GAZE_FOV_PITCH)) )
		{
			if(self->enemy->flags & FL_CLIENT)
			{
				CVector Color;
				Color.Set(0.65,0.65f,0.65);
				com->FlashClient(self->enemy, 0.35f,Color.x ,Color.y ,Color.z , 1.0f );
			}
			return TRUE;
		}
	}

	return FALSE;
}


// sunrise
void medusa_stone_enemy( userEntity_t *self, userEntity_t *enemy )
{
	// give the enemy a gray color (or grey colour if we're in Canada)
	enemy->s.color = STONE_COLOR;
	enemy->s.alpha = 0.70;

	// freeze damage the enemy to death
	com->Damage( enemy, self, self, CVector(0, 0, 0), CVector(0, 0, 0),
	            100000, DAMAGE_FREEZE | DAMAGE_NO_BLOOD);
	
	if(gstate->damage_inflicted)
	{
		self->enemy->flags |= FL_FREEZE;
	}
}


// ----------------------------------------------------------------------------
//
// Name:        MEDUSA_StartStopGazing
// Description: Medusa stop gazing start-task function
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void MEDUSA_StartStopGazing(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	// play set down animation
	frameData_t *pSequence = FRAMES_GetSequence( self, "atake" );
	AI_ForceSequence( self, pSequence );

	AI_SetTaskFinishTime( hook, 10.0f );

	AI_Dprintf( "Starting TASKTYPE_MEDUSA_STOPGAZING.\n" );
}

// ----------------------------------------------------------------------------
//
// Name:        MEDUSA_StopGazing
// Description: Medusa stop gazing task function
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void MEDUSA_StopGazing(userEntity_t *self)
{
	ai_frame_sounds( self );

	// check for end of set down animation
	if ( AI_IsEndAnimation( self ) )
	{
		// gaze goal done

		playerHook_t *hook = AI_GetPlayerHook( self );
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( AI_GetCurrentGoalStack( hook ) );

		// OK to attack and take pain now
		AI_SetOkToAttackFlag( hook, TRUE );
		hook->pain_finished = gstate->time;

		// Goal complete, now go right back to killing enemy!
		GOAL_Satisfied( pCurrentGoal );
		AI_RemoveCurrentGoal( self );

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
void monster_medusa( userEntity_t *self )
{
	playerHook_t *hook  = AI_InitMonster( self, TYPE_MEDUSA );

	self->className		= "monster_medusa";
	self->netname		= tongue_monsters[T_MONSTER_MEDUSA];

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
	////////////////	///////////////////////////////////////////

	hook->upward_vel	= 270.0;
	hook->run_speed		= 250;
	hook->walk_speed	= 150;
	hook->attack_speed	= 250;
	hook->max_jump_dist = ai_max_jump_dist( hook->run_speed, hook->upward_vel );

	hook->dflags		|= DFL_RANGEDATTACK;
	hook->attack_dist	= 80;

	self->health		= 200;
	hook->base_health	= 200;
	hook->pain_chance	= 15;
	self->mass			= 2.0;

	hook->fnStartAttackFunc = medusa_start_attack;
	hook->fnAttackFunc  = medusa_attack;

	hook->fnInAttackRange = medusa_check_range;

	self->think			= AI_ParseEpairs;
	self->nextthink		= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give medusa a weapon
	///////////////////////////////////////////////////////////////////////////
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
										  "poison bite mild",
										  poison_bite_mild,
										  ITF_TRACE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );

		self->curWeapon = ai_init_weapon( self,
										  pWeaponAttributes[1].fBaseDamage,
										  pWeaponAttributes[1].fRandomDamage,
										  pWeaponAttributes[1].fSpreadX,
										  pWeaponAttributes[1].fSpreadZ,
										  pWeaponAttributes[1].fSpeed,
										  pWeaponAttributes[1].fDistance,
										  pWeaponAttributes[1].offset,
										  "medusa spit",
										  medusa_spit,
										  ITF_PROJECTILE | ITF_NOLEAD ); // don't lead our shots with the spit
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}

	hook->nAttackType = ATTACK_GROUND_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	AI_DetermineMovingEnvironment( self );

	hook->pain_chance		= 1;

	gstate->SoundIndex("global/we_gravela.wav");

	gstate->LinkEntity(self);

	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

	// NSS[6/7/00]:Mark this monster as a boss
	hook->dflags |= DFL_ISBOSS;


}


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_medusa_register_func()
{
	gstate->RegisterFunc("medusa_start_attack",medusa_start_attack);
	gstate->RegisterFunc("medusa_attack",medusa_attack);
	gstate->RegisterFunc("medusa_check_range",medusa_check_range);
	gstate->RegisterFunc("AI_TaskThink",AI_TaskThink);
}

