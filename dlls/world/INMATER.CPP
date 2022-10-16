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
#include "ai.h"
#include "ai_utils.h"
//#include "ai_move.h"// SCG[1/23/00]: not used
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "actorlist.h"
//#include "chasecam.h"// SCG[1/23/00]: not used
#include "ai_func.h"
#include "MonsterSound.h"
#include "transition.h"

/* ***************************** define types ****************************** */

#define MONSTERNAME			"monster_inmater"
#define TRANSITION_NAME		"models/e1/animTEXTfiles/t_inmater.csv"

#define MELEE_ATTACK_DISTANCE	128.0f
#define PRISONER_DISTANCE		256.0f 

static int nTargetedSound;
static int nTerminatedSound;
static int nPrisonerSound[3];

class CThirdAttack
{
public:
	short nFrameIndex;
	float fYawAdjuster;
	CVector offset;

	CThirdAttack( short nNum, float fNewValue, CVector &newOffset )
	{
		nFrameIndex = nNum;
		fYawAdjuster = fNewValue;
		offset = newOffset;
	}
};

CThirdAttack aThirdAttackInfo[] = {

	CThirdAttack( 10, -5.0f, CVector(5, 16, 16) ),
	CThirdAttack( 14, 0.0f, CVector(-3, 16, 16) ),
	CThirdAttack( 18, 5.0f, CVector(-11, 16, 16) ),
	CThirdAttack( 22, 10.0f, CVector(-19, 16, 16) ),
	CThirdAttack( 26, 15.0f, CVector(-27, 16, 16) ),
	CThirdAttack( 36, 10.0f, CVector(-18, 16, 16) ),

};

int nNumThirdAttacks = sizeof(aThirdAttackInfo) / sizeof(CThirdAttack);

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_inmater (userEntity_t *self);


///////////////////////////////////////////////////////////////////////////////////
// inmater specific functions
///////////////////////////////////////////////////////////////////////////////////

static int inmater_IsClearShot( userEntity_t *self, userEntity_t *pEnemy )
{
	ai_weapon_t	*pWeapon = (ai_weapon_t *) self->curWeapon;
    if ( !pWeapon )
    {
        return FALSE;
    }
    
	self->s.angles.AngleToVectors(forward, right, up);

	for ( int i = 0; i < nNumThirdAttacks; i++ )
	{
    	CVector offset = aThirdAttackInfo[i].offset;

	    CVector weaponOrigin = self->s.origin + (offset.x * right) + 
				               (offset.y * forward) + (offset.z * up);

        tr = gstate->TraceLine_q2( weaponOrigin, pEnemy->s.origin, self, MASK_SHOT );
        if ( tr.ent && AI_IsAlive( tr.ent ) && tr.ent != pEnemy )
        {
            playerHook_t *hook = AI_GetPlayerHook( self );

            userEntity_t *pHitEntity = tr.ent;
            playerHook_t *pHitEntityHook = (playerHook_t *)pHitEntity->userHook;
            if ( pHitEntityHook )
            {
                if ( ((hook->dflags & DFL_EVIL) && (pHitEntityHook->dflags & DFL_EVIL)) ||
                     ((hook->dflags & DFL_GOOD) && (pHitEntityHook->dflags & DFL_GOOD)) )
                {
                    return FALSE;
                }
            }
        }
    }

    return TRUE;

}

// ----------------------------------------------------------------------------
//
// Name:		inmater_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//				ataka	=> ranged attack
//				atakb	=> melee attack
//
// ----------------------------------------------------------------------------
static void inmater_set_attack_seq(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if(self->enemy == NULL)
	{
		AI_RemoveCurrentTask(self,FALSE);
		return;
	}
	if ( AI_IsFacingEnemy(self, self->enemy, 1.0f, 2.0f))
	{
		frameData_t *pSequence = NULL;
		float dist = VectorDistance( self->s.origin, self->enemy->s.origin );
		if ( dist > MELEE_ATTACK_DISTANCE )
		{
			if( rand() % 4 )
            {
				pSequence = FRAMES_GetSequence( self, "ataka" );
			}
            else
			{
            	pSequence = FRAMES_GetSequence( self, "atakc" );
			}
            
            AI_ForceSequence(self, pSequence);

			self->curWeapon = gstate->InventoryFindItem( self->inventory, "laser" );

			hook->nAttackMode = ATTACKMODE_RANGED;
		}
		else
		{
			pSequence = FRAMES_GetSequence( self, "atakb" );
			AI_ForceSequence(self, pSequence);

			self->curWeapon = gstate->InventoryFindItem(self->inventory, "punch");

			hook->nAttackMode = ATTACKMODE_MELEE;
		}

		self->s.frameInfo.frameFlags |= FRAME_ONCE;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		INMATER_FireThirdWeapon
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void INMATER_FireThirdWeapon(userEntity_t *self, int nAttackIndex)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	ai_weapon_t	*weapon = (ai_weapon_t *) self->curWeapon;

	if (!self->curWeapon || self->enemy == NULL)
	{
		return;
	}

	CVector offset = aThirdAttackInfo[nAttackIndex].offset;
	weapon->ofs	= offset;

	//	save current view angles
// SCG[10/26/99]: monsters do not get a client struct
//	CVector old_angles = self->client->v_angle;

	// do the aiming stuff
	CVector dir = self->enemy->s.origin - self->s.origin;
	dir.Normalize();
	CVector ang;
	VectorToAngles( dir, ang );

	// make the projectile shoot straight out of the arm, or at least make it seem so
	ang.yaw = self->s.angles.yaw + aThirdAttackInfo[nAttackIndex].fYawAdjuster;
	ang.roll = 0.0f;

// SCG[10/26/99]: monsters do not get a client struct
//	self->client->v_angle = ang;

	//	fire the weapon
	self->curWeapon->use(self->curWeapon, self);

// SCG[10/26/99]: monsters do not get a client struct
//	self->client->v_angle = old_angles;
}

// ----------------------------------------------------------------------------
//
// Name:		inmater_find_prisoner
// Description:
//				finds the closest visible prisoner for the inmater to gib
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *inmater_find_prisoner( userEntity_t *self )
{
	float last_d = PRISONER_DISTANCE;
	userEntity_t *enemy = NULL;
	userEntity_t *head = alist_FirstEntity (monster_list);

	while ( head )
	{
		if ( head->className && head->deadflag == DEAD_NO && 
			 !stricmp (head->className, "monster_prisoner") )
		{
			float dist = VectorDistance (self->s.origin, head->s.origin);
			if ( dist < last_d )
			{
				enemy = head;
				last_d = dist;
			}
		}

		head = alist_NextEntity (monster_list);
	}

	return enemy;
}

// ----------------------------------------------------------------------------
//
// Name:		INMATER_StartGotoPrisoner
// Description:	
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void INMATER_StartGotoPrisoner( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FREEZE )
	{
		return;
	}

	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) != TASKTYPE_INMATER_GOTOPRISONER )
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		AIDATA_PTR pAIData = GOAL_GetData(pCurrentGoal);
		AI_AddNewTaskAtFront( self, TASKTYPE_INMATER_GOTOPRISONER, pAIData->pEntity );
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_INMATER_GOTOPRISONER.\n" );

	if ( AI_StartMove( self ) == FALSE )
	{
		return;
	}

	//////////////////////////////////////////////////////
	//	play "prisoner targetted..." sound
	//////////////////////////////////////////////////////
	gstate->StartEntitySound(self, CHAN_AUTO, nTargetedSound,0.85f, hook->fMinAttenuation, hook->fMaxAttenuation);

	self->nextthink	= gstate->time + 0.1f;

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, 10.0f );
}

// ----------------------------------------------------------------------------
//
// Name:		INMATER_GotoPrisoner
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void INMATER_GotoPrisoner( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	userEntity_t *pPrisoner = pAIData->pEntity;
	
	CVector destPoint = pPrisoner->s.origin;
	
	float fDistance = VectorXYDistance( self->s.origin, destPoint );
	float fZDistance = VectorZDistance( self->s.origin, destPoint );
	if ( fDistance < 64.0f && fZDistance < 32.0f )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	tr = gstate->TraceLine_q2( self->s.origin, destPoint, self, MASK_SOLID );
	if ( (tr.fraction >= 1.0f && fZDistance < MAX_JUMP_HEIGHT) || 
		 (tr.fraction >= 0.8f && ((1.2f-tr.fraction) * (fDistance+fZDistance)) < 32.0f) )
	{
		AI_MoveTowardPoint( self, destPoint, FALSE );
	}
	else
	{
		if ( !AI_HandleUse( self ) )
		{
			if ( hook->pPathList->pPath )
			{
				if ( AI_Move( self ) == FALSE && 
					 AI_FindPathToPoint( self, destPoint ) && !hook->pPathList->pPath )
				{
					// restart the current goal
					GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
					GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
					GOAL_ClearTasks( pCurrentGoal );
					AI_AddInitialTasksToGoal( self, pCurrentGoal, TRUE );
				}
			}
			else
			{
				if ( AI_FindPathToPoint( self, destPoint ) == TRUE && 
					 !hook->pPathList->pPath )
				{
					// restart the current goal
					GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
					GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
					GOAL_ClearTasks( pCurrentGoal );
					AI_AddInitialTasksToGoal( self, pCurrentGoal, TRUE );
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		INMATER_StartWaitAtPrisoner
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void INMATER_StartWaitAtPrisoner( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FREEZE )
	{
		return;
	}

	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) != TASKTYPE_INMATER_WAITATPRISONER )
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		AIDATA_PTR pAIData = GOAL_GetData(pCurrentGoal);
		AI_AddNewTaskAtFront( self, TASKTYPE_INMATER_WAITATPRISONER, pAIData->fValue );
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_INMATER_WAITATPRISONER.\n" );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	pAIData->fValue = gstate->time + pAIData->fValue;

	frameData_t *pSequence = FRAMES_GetSequence( self, "amba" );
	if ( AI_StartSequence( self, pSequence ) == FALSE )
	{
		return;
	}

	self->nextthink	= gstate->time + 0.1f;

	/////////////////////////////////////////
	//	make the prisoner cower 
	/////////////////////////////////////////
	userEntity_t *prisoner = self->enemy;
	_ASSERTE( prisoner );
    if (prisoner == NULL) 
	{
		return;
	}
	prisoner->enemy = self;

	//	make the prisoner follow the inmater
	AI_AddNewTaskAtFront( prisoner, TASKTYPE_PRISONER_COWER );

	AI_ZeroVelocity( self );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, -1.0f );
}

// ----------------------------------------------------------------------------
//
// Name:		INMATER_WaitAtPrisoner
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void INMATER_WaitAtPrisoner( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	if ( gstate->time > pAIData->fValue )
	{
		AI_RemoveCurrentTask( self );
	}

	/////////////////////////////////////////
	//	make the prisoner scream
	/////////////////////////////////////////
	int r = (int) (rnd () * 3);
	gstate->StartEntitySound(self, CHAN_AUTO, nPrisonerSound[r], 0.85f, hook->fMinAttenuation, hook->fMaxAttenuation);
}

// ----------------------------------------------------------------------------
//
// Name:		INMATER_StartKillPrisoner
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
// SCG[2/13/00]: 
/*
void INMATER_StartKillPrisoner( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FREEZE )
	{
		return;
	}

	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) != TASKTYPE_INMATER_KILLPRISONER )
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		AIDATA_PTR pAIData = GOAL_GetData(pCurrentGoal);
		AI_AddNewTaskAtFront( self, TASKTYPE_INMATER_KILLPRISONER, pAIData->pEntity );
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_INMATER_KILLPRISONER.\n" );

	frameData_t *pSequence = FRAMES_GetSequence( self, "atakb" );
	if ( AI_StartSequence(self, pSequence) == FALSE )
	{
		return;
	}

	self->nextthink	= gstate->time + 0.1f;

	// start the prisoner's dying sequence
	userEntity_t *pPrisoner = self->enemy;
	_ASSERTE( pPrisoner );
	pPrisoner->enemy = self;

	AI_ZeroVelocity( self );

	AI_AddNewTaskAtFront( pPrisoner, TASKTYPE_PRISONER_INMATERHASITINME );

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 10.0f );
}
*/
// ----------------------------------------------------------------------------
//
// Name:		INMATER_KillPrisoner
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void INMATER_KillPrisoner( userEntity_t *self )
{
	playerHook_t	*hook = AI_GetPlayerHook( self );
	userEntity_t	*pris = self->enemy;
	_ASSERTE( pris );
//	playerHook_t	*phook = AI_GetPlayerHook( pris );// SCG[1/23/00]: not used
	CVector			dir;

	AI_PlayAttackSounds( self );

	//////////////////////////////////////////////
	//	go back to wandering
	//////////////////////////////////////////////
	if ( AI_IsEndAnimation( self ) )
	{
		AI_RemoveCurrentGoal( self );
		AI_KilledEnemy( self );
		
		// play the inmate terminated sound
		gstate->StartEntitySound( self, CHAN_AUTO, nTerminatedSound, 0.85f, hook->fMinAttenuation, hook->fMaxAttenuation);

		// go back to path follow
		userEntity_t *pPathCorner = com->FindClosestEntity( self, "monster_path_corner" );
		if ( pPathCorner )
		{
			AI_AddNewGoal( self, GOALTYPE_MOVETOENTITY, pPathCorner );
		}

		return;
	}

	self->nextthink = gstate->time + THINK_TIME;
}

// ----------------------------------------------------------------------------
//
// Name:		INMATER_AddKillPrisonerGoal
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int INMATER_AddKillPrisonerGoal( userEntity_t *self )
{
	userEntity_t *pPrisoner = inmater_find_prisoner(self);
	if ( pPrisoner )
	{ 
		// set the prisoner to the inmater's enemy
		self->enemy = pPrisoner;
		AI_AddNewGoal( self, GOALTYPE_INMATER_KILLPRISONER, pPrisoner );

		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		inmater_do_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void inmater_do_attack(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if(self->enemy == NULL)
	{
		AI_RemoveCurrentTask(self,FALSE);
	}
	AI_FaceTowardPoint( self, self->enemy->s.origin );

    if( hook->cur_sequence && strstr( hook->cur_sequence->animation_name, "atak" ) )
    {
        AI_PlayAttackSounds( self );

	    int bAttackReady = AI_IsReadyToAttack1( self );

        if( strstr( hook->cur_sequence->animation_name, "atakc" ) )
	    {
		    // ranged attack
            // do the strafing fires
		    for( int i = 0; i < nNumThirdAttacks; i++ )
		    {
			    if( aThirdAttackInfo[i].nFrameIndex == (self->s.frame - hook->cur_sequence->first) )
			    {
					if ( inmater_IsClearShot( self, self->enemy ) )
					{
					    INMATER_FireThirdWeapon( self, i );
						break;
					}
					else
					{
						// Ach! We can't hit our enemy!
						AI_AddNewTaskAtFront( self, TASKTYPE_SIDESTEP ); // sidestep
					}
			    }
		    }
	    }
	    else
	    {
            // melee attack

		    if( bAttackReady )
		    {
                int bAttacked = FALSE;
                userEntity_t *pEnemy = self->enemy;
		        if ( AI_IsFacingEnemy(self, self->enemy, 5.0f) )
                {
                    if ( AI_IsAlive( pEnemy ) && (pEnemy->flags & FL_CLIENT) )
                    {
                        gclient_t *client = pEnemy->client;
                        if ( client->ps.pmove.pm_flags & PMF_DUCKED )
                        {
                            bAttacked = TRUE;
                            ai_weapon_t *weapon = (ai_weapon_t *) self->curWeapon;
                            punch_DoDamage( self, pEnemy, weapon );
                        }
                    }

					if ( bAttacked == FALSE )
					{
    					ai_fire_curWeapon( self );
					}
                }
		    }
	    }

	    if( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	    {
			float dist = VectorDistance (self->s.origin, self->enemy->s.origin);

		    if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
		    {
			    if ( AI_ShouldFollow( self ) == TRUE )
			    {
				    AI_RemoveCurrentTask( self );
			    }
			    else
			    {
				    // give up attacking the enemy
				    AI_RemoveCurrentGoal( self );
			    }
		    }
		    else
		    {
				inmater_set_attack_seq( self );
			}
	    }
    }
    else
    {
        inmater_set_attack_seq( self );
    }
}

// ----------------------------------------------------------------------------
//
// Name:		inmater_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void inmater_attack( userEntity_t *self )
{
	_ASSERTE( self );
    if ( self->enemy == NULL  || !AI_IsAlive( self->enemy ) )
    {
        AI_RemoveCurrentTask( self );
        return;
    }

    playerHook_t *hook = AI_GetPlayerHook( self );

	float fDist = VectorDistance (self->s.origin, self->enemy->s.origin);
	if ( fDist > MELEE_ATTACK_DISTANCE )
	{
		if ( hook->nAttackMode != ATTACKMODE_RANGED )
		{
			inmater_set_attack_seq( self );
		}		
	}
	else
	{
		if ( hook->nAttackMode != ATTACKMODE_MELEE )
		{
			inmater_set_attack_seq( self );
		}		
	}

	inmater_do_attack(self);
}

// ----------------------------------------------------------------------------
//
// Name:		inmater_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void inmater_begin_attack (userEntity_t *self)
{
	AI_StopCurrentSequence( self );
	AI_ZeroVelocity( self );

	inmater_set_attack_seq (self);

}

// ----------------------------------------------------------------------------
//
// Name:		monster_inmater
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_inmater( userEntity_t *self )
{
	self->className = MONSTERNAME;
	self->netname = tongue_monsters[T_MONSTER_INMATER];

	playerHook_t *hook = AI_InitMonster(self, TYPE_INMATER);

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

	nTargetedSound		= gstate->SoundIndex("e1/m_inmatertarget.wav");
	nTerminatedSound	= gstate->SoundIndex("e1/m_inmaterterminator.wav");
	nPrisonerSound[0]	= gstate->SoundIndex("e1/m_prizano.wav");
	nPrisonerSound[1]	= gstate->SoundIndex("e1/m_prizano.wav");
	nPrisonerSound[2]	= gstate->SoundIndex("e1/m_prizano.wav");


//	int nRetValue = TRANSITION_ReadFile( TRANSITION_NAME, self );
	TRANSITION_ReadFile( TRANSITION_NAME, self );// SCG[1/23/00]: nRetValue not used, no use wasting memory on a variable

	gstate->SetSize(self, -16.0, -16.0, -24.0, 16.0, 16.0, 32.0);


	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////

	hook->fnStartAttackFunc = inmater_begin_attack;
	hook->fnAttackFunc		= inmater_attack;

	hook->dflags			|= DFL_RANGEDATTACK;
	hook->max_jump_dist		= 0;
	hook->attack_dist		= 512.0;
	hook->base_health		= 200.0;
	hook->run_speed			= 80;
	hook->walk_speed		= 40;

	hook->upward_vel		= 100.0;
	hook->max_jump_dist		= ai_max_jump_dist( hook->run_speed, hook->upward_vel );

	self->ang_speed.y		= 180;
	
	self->health			= 200.0;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;

    self->fragtype          |= FRAGTYPE_NOBLOOD;   // don't leave blood upon death
    self->fragtype          |= FRAGTYPE_ROBOTIC;   // apply robotic gib parts

	self->s.render_scale.Set(1.3, 1.3, 1.3);

	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );

	//////////////////////////////////////////////
	//	give inmater its uh, what... uh...
	//////////////////////////////////////////////
	self->inventory = gstate->InventoryNew (MEM_MALLOC);

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
										  "punch", 
										  melee_punch, 
										  ITF_TRACE);
		gstate->InventoryAddItem (self, self->inventory, self->curWeapon);

		self->curWeapon = ai_init_weapon( self, 
										  pWeaponAttributes[1].fBaseDamage, 
										  pWeaponAttributes[1].fRandomDamage,
										  pWeaponAttributes[1].fSpreadX,
										  pWeaponAttributes[1].fSpreadZ,
										  pWeaponAttributes[1].fSpeed,
										  pWeaponAttributes[1].fDistance,
										  pWeaponAttributes[1].offset, 
										  "laser", 
										  laser_fire, 
										  ITF_PROJECTILE | ITF_NOLEAD );
		gstate->InventoryAddItem(self, self->inventory, self->curWeapon);
	}

	hook->pain_chance		= 5;


	hook->nAttackType = ATTACK_GROUND_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	hook->ai_flags	  &= ~AI_CANDODGE;
	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;

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
void world_ai_inmater_register_func()
{
	gstate->RegisterFunc( "inmater_begin_attack", inmater_begin_attack );
	gstate->RegisterFunc( "inmater_attack", inmater_attack );
	gstate->RegisterFunc( "AI_ParseEpairs", AI_ParseEpairs );
}
