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
#include "ai_func.h"
#include "MonsterSound.h"
//#include "actorlist.h"// SCG[1/23/00]: not used

/* ***************************** define types ****************************** */

#define MONSTERNAME			"monster_thunderskeet"


#define HOVER_DISTANCE			175.0f
#define FLY_AWAY_DISTANCE		1000.0f

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
//void thunderskeet_start_idle( userEntity_t *self );// SCG[1/23/00]: not used
/* **************************** Global Variables *************************** */
//extern float sin_tbl[];// SCG[1/23/00]: not used
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_thunderskeet(userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------------
//
// Name:		THUNDERSKEET_StartDie
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void THUNDERSKEET_Think(userEntity_t *self)
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_TaskThink(self);

}

// ----------------------------------------------------------------------------
//
// Name:		THUNDERSKEET_ComputeFlyAwayPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void THUNDERSKEET_ComputeFlyAwayPoint( userEntity_t *self, CVector &flyAwayPoint )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	//NSS[10/26/99]: added new flyaway for thunderskeet
	CVector Axis,Target_Angles;

	//Initialize the flyaway point to be ourself in case we don't find one
	flyAwayPoint = self->s.origin;
	
	Target_Angles = self->s.angles;
	Target_Angles.x += crand() * 15.0f;
	Target_Angles.y = 0.0f;
	Target_Angles.z = 0.0f;

	Target_Angles.AngleToForwardVector(Axis);
	
	Axis = self->s.origin + (Axis * (256.0f + 256.0f*rnd()));

	MAPNODE_PTR Node = 	NODE_GetClosestNode( self, Axis);

	if(!Node)
	{
		AI_Dprintf("BAD THINGS ARE HAPPENING!!!Flyaway with no Node system initialized!\n");
		AI_RemoveCurrentTask(self,FALSE);
		return;
	}

	flyAwayPoint = Node->position;
}

// ----------------------------------------------------------------------------
//
// Name:		THUNDERSKEET_Chase
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void THUNDERSKEET_Chase( userEntity_t *self )
{
	self->think = THUNDERSKEET_Think;


	playerHook_t *hook = AI_GetPlayerHook( self );

	float fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );


	
	if(AI_IsEndAnimation(self) || ((hook && hook->cur_sequence && hook->cur_sequence->animation_name) && !(strstr(hook->cur_sequence->animation_name,"fly"))))
	{
		AI_ForceSequence(self,"flya",FRAME_LOOP);
	}
	
	int bEnemyVisible = AI_IsVisible( self, self->enemy );

	if(!self->enemy)
	{
		AI_RemoveCurrentGoal(self);
		return;
	}

	playerHook_t *pTargetHook = AI_GetPlayerHook( self->enemy );
	if ( bEnemyVisible && pTargetHook && !(pTargetHook->items & IT_WRAITHORB))
	{
		
		float zDistance = VectorZDistance(self->s.origin,self->enemy->s.origin);
		if(zDistance < 96.0f)
		{
			AI_AddNewTaskAtFront(self,TASKTYPE_THUNDERSKEET_HOVER);
			return;
		}
		if ( fDistance < hook->attack_dist )
		{
			AI_RemoveCurrentTask( self, FALSE );
			return;
		}
		else
		{
			CVector destPoint = self->enemy->s.origin;
			destPoint.z += 96.0f;
			AI_FlyTowardPoint2( self, destPoint, 0.05f );
		}
		
		self->s.angles.pitch	= 0.0f;
		self->s.angles.roll		= 0.0f;

		// extend the time
		AI_SetTaskFinishTime( hook, 10.0f );
	}
	else
	{
		AI_AddNewTaskAtFront(self,TASKTYPE_THUNDERSKEET_HOVER);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		THUNDERSKEET_StartHover
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void THUNDERSKEET_StartHover( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	TASK_Set( pCurrentTask, gstate->time + 2.75f );
	
	AI_ZeroVelocity( self );

	AI_Dprintf( "Starting TASKTYPE_THUNDERSKEET_HOVER.\n" );

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 2.0f );
	if(AI_Determine_Room_Height( self, 1024.0f, 2) > 128.0f)
		self->velocity.z = 64.0f;
	frameData_t *pSequence = FRAMES_GetSequence( self, "flya" );
	AI_ForceSequence( self, pSequence,FRAME_LOOP );
	self->nextthink	= gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
//
// Name:		THUNDERSKEET_Hover
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void THUNDERSKEET_Hover( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	ai_frame_sounds( self );
	float fDistance = 0.0f;


	if(self->enemy)
	{
		fDistance = VectorDistance(self->s.origin,self->enemy->s.origin);
		if(AI_Determine_Room_Height( self, 1024.0f, 1) > 128.0f && fDistance < 128.0)
		{
			self->velocity.z = 64.0f;
		}
		AI_FaceTowardPoint( self, self->enemy->s.origin );
		self->s.angles.pitch = 0.0f;
	}

	if ( gstate->time > pAIData->fValue )
	{
		AI_RemoveCurrentTask( self, FALSE );
		AI_ZeroVelocity(self);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		THUNDERSKEET_StartDartTowardEnemy
// Description:
//				move straight toward the enemy
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void THUNDERSKEET_StartDartTowardEnemy( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	AI_StartMove( self );

	float fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );
	if ( AI_IsWithinAttackDistance( self, fDistance ) )
	{
		AI_RemoveCurrentTask( self, FALSE );
		return;
	}		

	AI_FaceTowardPoint( self, self->enemy->s.origin );
	AI_UpdatePitchTowardEnemy( self );
	
	float fSpeed = AI_ComputeMovingSpeed( hook ) * 1.5f;

	CVector destPoint = self->enemy->s.origin;
	destPoint.z += 24.0f;
	CVector vectorTowardPoint = destPoint - self->s.origin;
	vectorTowardPoint.Normalize();

	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	self->velocity = vectorTowardPoint * (fSpeed * fScale);

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1 );
	
	frameData_t *pSequence = FRAMES_GetSequence( self, "flya" );
	AI_ForceSequence( self, pSequence,FRAME_LOOP );

	self->nextthink	= gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
//
// Name:		THUNDERSKEET_DartTowardEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void THUNDERSKEET_DartTowardEnemy( userEntity_t *self )
{
	_ASSERTE( self );
	_ASSERTE( self->enemy );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	float fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );
	if (fDistance < 512.0f )
	{
		AI_SetOkToAttackFlag( hook, TRUE );
		AI_RemoveCurrentTask( self, FALSE );
		return;
	}		

	AI_FaceTowardPoint( self, self->enemy->s.origin );
	
	float fSpeed = AI_ComputeMovingSpeed( hook ) * 1.5f;

	CVector destPoint = self->enemy->s.origin;
	destPoint.z += 24.0f;
	CVector vectorTowardPoint = destPoint - self->s.origin;
	vectorTowardPoint.Normalize();

	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	self->velocity = vectorTowardPoint * (fSpeed * fScale);

	CVector org = self->s.origin;
	float fMoveDistance = hook->last_origin.Distance(org);
	if ( fMoveDistance < (self->velocity.Length() * 0.025f) )
	{
		if ( hook->nWaitCounter > 1 )
		{
			AI_SetOkToAttackFlag( hook, TRUE );
			AI_RemoveCurrentTask( self, FALSE );
			return;
		}
		hook->nWaitCounter++;
	}
	else
		hook->nWaitCounter = 0;

	hook->last_origin = self->s.origin;
	ai_frame_sounds( self );
}

// ----------------------------------------------------------------------------
//
// Name:		THUNDERSKEET_StartFlyAway
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void THUNDERSKEET_StartFlyAway( userEntity_t *self )
{
	// find a point to fly away
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_Dprintf( "Starting TASKTYPE_THUNDERSKEET_FLYAWAY\n" );

	AI_StartMove( self );

	CVector flyAwayPoint;
	THUNDERSKEET_ComputeFlyAwayPoint( self, flyAwayPoint );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );
	pAIData->destPoint = flyAwayPoint;
	
	frameData_t *pSequence = FRAMES_GetSequence( self, "flya" );
	AI_ForceSequence( self, pSequence, FRAME_LOOP);
	
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1 );

	self->nextthink	= gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
//
// Name:		THUNDERSKEET_FlyAway
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void THUNDERSKEET_FlyAway( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	float fDistance = VectorDistance( self->s.origin, pAIData->destPoint );
	//NSS[10/25/99]:added new fly function
	AI_FlyTowardPoint2( self,pAIData->destPoint, 0.05f);
	if ( fDistance <= 96.0f )
	{
		AI_RemoveCurrentTask( self, FALSE );
		AI_SetOkToAttackFlag( hook, TRUE );
	}		

}

// ----------------------------------------------------------------------------
//
// Name:		thunderskeet_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void thunderskeet_set_attack_seq( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	self->curWeapon = gstate->InventoryFindItem (self->inventory, "Thunder Jiz");

	frameData_t *pSequence = FRAMES_GetSequence( self, "ataka" );
	AI_ForceSequence( self, pSequence );
}

userEntity_t * spawnZapFlareRotate(userEntity_t *self, CVector *Origin, CVector Rotate,CVector Size,float life, char *flarename);

// ----------------------------------------------------------------------------
//
// Name:		skeeter_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void THUNDERSKEET_Attack( userEntity_t *self )
{
	CVector Scale,Rotate,Offset,Target_Angles,Forward;
	playerHook_t *hook = AI_GetPlayerHook( self );
	AI_FaceTowardPoint( self, self->enemy->s.origin );
	AI_PlayAttackSounds( self );
	
	self->s.angles.roll = 0.0f;
	self->s.angles.pitch = 0.0f;

	if(AI_DetectForLikeMonsters( self ))
	{
		AI_RemoveCurrentTask( self, TASKTYPE_DODGE, self, FALSE );
	}

	//The flare on the eyes
	if( self->s.frame >= 22.0f && self->s.frame <= 24.0f)
	{
	
			Offset = self->enemy->s.origin - self->s.origin;
			Offset.Normalize();
			VectorToAngles(Offset,Target_Angles);
			Target_Angles.pitch -=35.0f;
			Target_Angles.yaw	+=2.0f;
			Target_Angles.AngleToForwardVector(Forward);
			Offset = (Forward * 40.0f) + self->s.origin;
			Offset.z	+= 40.0f;

			Scale.Set (0.3f,1.5f,0.75f);
			Rotate.Set(0.0f,0.0f,45.0f);

			spawnZapFlareRotate(self, &Offset,Rotate, Scale,0.6f,"models/global/e_flgreen.sp2");

			Scale.Set (1.5f,0.3f,0.75f);
			Rotate.Set(0.0f,0.0f,-45.0f);
			spawnZapFlareRotate(self, &Offset,Rotate, Scale,0.6f,"models/global/e_flgreen.sp2");
			
	}
	//The actual attack
	//if ( AI_IsReadyToAttack1( self ) )
	if( (self->s.frame >= 27.0f && self->s.frame <= 28.0f)|| (self->s.frame >= 31.0f && self->s.frame <= 32.0f) )
	{
			Offset		= self->s.origin;
			Offset.z	-= 10.0f;

			if(self->s.frame >= 31.0f)
			{
				hook->acro_boost = 1.0f;
			}
			else
				hook->acro_boost = 0.0f;

			ai_fire_curWeapon( self );
			
			Scale.Set (1.0f,8.0f,2.0f);
			Rotate.Set(0.0f,0.0f,45.0f);

			spawnZapFlareRotate(self, &Offset,Rotate, Scale,0.7f,"models/global/e_flyellow.sp2");

			Scale.Set (8.0f,1.0f,2.0f);
			Rotate.Set(0.0f,0.0f,-45.0f);
			spawnZapFlareRotate(self, &Offset,Rotate, Scale,0.7f,"models/global/e_flyellow.sp2");
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		AI_RemoveCurrentTask( self,FALSE );
	}
}

// ----------------------------------------------------------------------------
////NSS[10/25/99]:Modified
// Name:		THUNDERSKEET_Begin_Attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void THUNDERSKEET_Begin_Attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	thunderskeet_set_attack_seq( self );
	//Immediately face towards the enemy
	CVector dir = self->enemy->s.origin - self->s.origin;
	dir.Normalize();
	self->s.angles.yaw = VectorToYaw(dir);
	self->s.angles.pitch = 0.0f;
	self->s.angles.roll = 0.0f;
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1 );

	self->velocity.Zero();
}


// ----------------------------------------------------------------------------
//
// Name:		monster_thunderskeet
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_thunderskeet( userEntity_t *self )
{
	playerHook_t *hook	= AI_InitMonster( self, TYPE_TH_SKEET );

	self->className		= MONSTERNAME;
	self->netname		= tongue_monsters[T_MONSTER_THUNDERSKEET];

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

	//	turn rates for each axis in degrees per second
	self->ang_speed.Set( 90, 180, 90 );

	self->movetype			= MOVETYPE_FLY;

    self->fragtype          |= FRAGTYPE_NOBLOOD;   // don't leave blood upon death
    self->fragtype          |= FRAGTYPE_ROBOTIC;   // apply robotic gib parts
	self->fragtype			|= FRAGTYPE_ALWAYSGIB;

	hook->dflags			|= DFL_FACEPITCH;
	hook->dflags			|= DFL_RANGEDATTACK;

	hook->fnStartAttackFunc = THUNDERSKEET_Begin_Attack;
	hook->fnAttackFunc		= THUNDERSKEET_Attack;

	hook->pain_chance		= 20;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;

	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );

	//////////////////////////////////////////////
	//////////////////////////////////////////////
	self->inventory = gstate->InventoryNew(MEM_MALLOC);

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
										  "Thunder Jiz", 
										  ThunderJiz_attack, 
										  ITF_PROJECTILE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}

	hook->nAttackType = ATTACK_AIR_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	hook->strafe_time = gstate->time;
	hook->strafe_dir  = STRAFE_LEFT;

	hook->pain_chance		= 10;

	AI_DetermineMovingEnvironment( self );

    gstate->LinkEntity( self );

	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

	// NSS[6/7/00]:Mark this monster as a boss
	hook->dflags |= DFL_ISBOSS;

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



///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_thunderskeet_register_func()
{
	gstate->RegisterFunc("THUNDERSKEET_Think",THUNDERSKEET_Think);
	gstate->RegisterFunc("THUNDERSKEET_Begin_Attack",THUNDERSKEET_Begin_Attack);
	gstate->RegisterFunc("THUNDERSKEET_Attack",THUNDERSKEET_Attack);

	gstate->RegisterFunc("AI_ParseEpairs",AI_ParseEpairs);	
}
