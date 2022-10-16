///////////////////////////////////////////////////////////////////////////////
//NSS
//  death sphere
//
//	guards the mishima complex and fires four laser blasts at a time.
//  slow charge rate, but hard core damage.
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER
#include <crtdbg.h>
#endif
#include "world.h"
#include "nodelist.h"
//	#include "ai_move.h"// SCG[1/23/00]: not used
#include "ai_weapons.h"
#include "ai_frames.h"
#include "ai.h"
#include "ai_func.h"
#include "MonsterSound.h"
#include "ai_utils.h"

extern float sin_tbl[];
////////////////////////////////// defines ///////////////////////////////////////

#define MONSTER_NAME	"monster_deathsphere"

#define BACKAWAY_DISTANCE	72.0f
#define HEIGHT_FROM_PLAYER	72.0f

#define MIN_DISTANCE_AWAY	72.0f
#define MAX_DISTANCE_AWAY	192.0f




/* ******************************* Exterior functions  ********************************* */
int AI_ComputeBestAwayYawPoint(userEntity_t *self, CVector &flyAwayPoint, int Distance, int Resolution);

/* ******************************* exports ********************************* */

DllExport void monster_deathsphere( userEntity_t *self );

CVector	muzzle[] =
{
	CVector(  1.21,10.90,27.18),
	CVector( 23.99,11.05, 9.38),
	CVector(  0.77,11.05,-7.40),
	CVector(-24.23,11.05, 9.21),
};

void deathsphere_fire_weapon(userEntity_t* self);

// ----------------------------------------------------------------------------
//
// Name:		deathsphere_moveup
// Description:Function which prevents the deathsphere from slamming against the ceiling.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------

void deathsphere_moveup(userEntity_t *self)
{
		CVector Axis,Target_Angles,flyAwayPoint;

		// NSS[12/7/99]:
		playerHook_t *hook = AI_GetPlayerHook( self );
		if(hook)
		{
			GOALSTACK_PTR pGoalStack	= AI_GetCurrentGoalStack( hook );
			if(pGoalStack)
			{
				
				TASK_PTR pCurrentTask		= GOALSTACK_GetCurrentTask( pGoalStack );
				if(pCurrentTask)
				{
					// NSS[12/7/99]:Don't add multiple tasks onto the stack.
					if(pCurrentTask->nTaskType == TASKTYPE_DEATHSPHERE_MOVETOLOCATION)
						return;
				}
			}
		}

		
		
		//Initialize the flyaway point to be ourself in case we don't find one
		flyAwayPoint = self->s.origin;
	

		Target_Angles	= self->s.angles;
		Target_Angles.x = 0.0f;
		Target_Angles.y = 0.0f;
		Target_Angles.z	= 1.0f;

		Target_Angles.AngleToForwardVector(Axis);
	
		Axis = self->s.origin + (Axis * (96.0f + 128.0f*rnd()));

		MAPNODE_PTR Node = 	NODE_GetClosestNode( self, Axis);
		if(Node)
		{
			gstate->StartEntitySound( self, CHAN_AUTO,gstate->SoundIndex("e1/m_dspheresteama.wav"),0.85f,256.0f, 512.0f);	
			AI_AddNewTaskAtFront( self, TASKTYPE_DEATHSPHERE_MOVETOLOCATION, Node->position );
		}
}


// ----------------------------------------------------------------------------
//
// Name:		deathsphere_movedown
// Description:Function which prevents the deathsphere from slamming against the ceiling.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------

void deathsphere_movedown(userEntity_t *self)
{
		CVector Axis,Target_Angles,flyAwayPoint;

		// NSS[12/7/99]:
		playerHook_t *hook = AI_GetPlayerHook( self );
		if(hook)
		{
			GOALSTACK_PTR pGoalStack	= AI_GetCurrentGoalStack( hook );
			if(pGoalStack)
			{
				
				TASK_PTR pCurrentTask		= GOALSTACK_GetCurrentTask( pGoalStack );
				if(pCurrentTask)
				{
					// NSS[12/7/99]:Don't add multiple tasks onto the stack.
					if(pCurrentTask->nTaskType == TASKTYPE_DEATHSPHERE_MOVETOLOCATION)
						return;
				}
			}
		}
		
		
		//Initialize the flyaway point to be ourself in case we don't find one
		flyAwayPoint = self->s.origin;
	

		//Target_Angles	= self->s.angles;
		///Target_Angles.x = 0.0f;
		//Target_Angles.y = 0.0f;
		//Target_Angles.z	= -1.0f;

		//Target_Angles.AngleToForwardVector(Axis);
		Axis.Set(0.0f,0.0f,-1.0f);
	
		Axis = self->s.origin + (Axis * (96.0f + 128.0f*rnd()));

		//MAPNODE_PTR Node = 	NODE_GetClosestNode( self, Axis);
		//if(Node)
		//{
		gstate->StartEntitySound( self, CHAN_AUTO,gstate->SoundIndex("e1/m_dspheresteama.wav"),0.85f,256.0f, 512.0f);	
		AI_AddNewTaskAtFront( self, TASKTYPE_DEATHSPHERE_MOVETOLOCATION, Axis );
		//}
}


// ----------------------------------------------------------------------------
//
// Name:		deathsphere_startmove
// Description:Initialize the movefunction
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void DEATHSPHERE_StartMove(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack	= AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask		= GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData			= TASK_GetData( pCurrentTask );

	frameData_t *pSequence = FRAMES_GetSequence( self, "flya" );
	AI_ForceSequence( self, pSequence, FRAME_LOOP);

	pAIData->fValue = gstate->time + 0.25f;
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook,1.2f );
}


// ----------------------------------------------------------------------------
//
// Name:		deathsphere_move
// Description:This is an actual move function for the deathsphere
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void DEATHSPHERE_Move(userEntity_t *self)
{
	CVector Dir;
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack	= AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask		= GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData			= TASK_GetData( pCurrentTask );
	
	float fDistance = VectorDistance(self->s.origin, pAIData->destPoint);

	if(gstate->time > pAIData->fValue || fDistance < 34.0f)
	{
		AI_SetOkToAttackFlag( hook, TRUE);
		AI_RemoveCurrentTask(self,FALSE);	
	}
	else
	{
		Dir = pAIData->destPoint - self->s.origin;
		Dir.Normalize();
		Dir = Dir * hook->run_speed;
		self->velocity = Dir;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		deathsphere_avoid
// Description:Function which has a 15% chance of trying to dodge the player's shot
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void deathsphere_avoid(userEntity_t *self)
{
	CVector flyAwayPoint;
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack	= AI_GetCurrentGoalStack( hook );
//	TASK_PTR pCurrentTask		= GOALSTACK_GetCurrentTask( pGoalStack );	// SCG[1/23/00]: not used
	if(pGoalStack->pTopOfStack->pTasks->pStartList->pNext)
		if(pGoalStack->pTopOfStack->pTasks->pStartList->pNext->nTaskType == TASKTYPE_DEATHSPHERE_MOVETOLOCATION)
			return;
	flyAwayPoint = self->s.origin;
	AI_ComputeBestAwayYawPoint(self, flyAwayPoint, 500.0f, 20);
	gstate->StartEntitySound( self, CHAN_AUTO,gstate->SoundIndex("e1/m_dspheresteama.wav"),0.85f,256.0f, 512.0f);
	AI_RemoveCurrentTask( self, TASKTYPE_DEATHSPHERE_MOVETOLOCATION, flyAwayPoint );
}


///////////////////////////////////////////////////////////////////////////////
//
//  DEATHSPHERE_StartChargeWeapon
//
///////////////////////////////////////////////////////////////////////////////
void DEATHSPHERE_Think( userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	AI_TaskThink( self);
	
	if(!AI_IsAlive(self))
	{
		self->gravity=1.75f;
		self->movetype = MOVETYPE_BOUNCE;
	}
	else
	{
		if(AI_Determine_Room_Height( self, 300.0f, 2) < 32.0f) //Check distance to floor
		{
			deathsphere_movedown( self );
		}
		else if(AI_Determine_Room_Height( self, 300.0f, 1) < 32.0f)//Check Distance to Ceiling
		{
			deathsphere_moveup( self );
		}
		else
		{
			if(hook->stateFlags & SFL_WALKING && hook->acro_boost == 1.0)
			{
				gstate->StartEntitySound( self, CHAN_LOOP, gstate->SoundIndex("e1/m_dspherehovera.wav"),0.0f, 0, 0 );

				gstate->StartEntitySound( self, CHAN_LOOP, gstate->SoundIndex("e1/m_dspherehoverf.wav"),0.45f, ATTN_NORM_MIN, ATTN_NORM_MAX );
				hook->acro_boost = 0;
			}
			else if ((hook->stateFlags & SFL_RUNNING || hook->stateFlags & SFL_ATTACKING) && hook->acro_boost == 0)
			{
				gstate->StartEntitySound( self, CHAN_LOOP, gstate->SoundIndex("e1/m_dspherehoverf.wav"),0.0f, 0, 0 );
				gstate->StartEntitySound( self, CHAN_LOOP, gstate->SoundIndex("e1/m_dspherehovera.wav"),0.45f, ATTN_NORM_MIN, ATTN_NORM_MAX );
				hook->acro_boost = 1;
			}
	
			if(hook->acro_boost_time >= 12.0f)
			{	
				hook->acro_boost_time = 0.0f;
			}
			self->velocity.z += 15.0*(sin_tbl[(int)hook->acro_boost_time]);
			hook->acro_boost_time +=1.0f;
		
		}
	}

}


///////////////////////////////////////////////////////////////////////////////
//
//  DEATHSPHERE_StartChargeWeapon
//
///////////////////////////////////////////////////////////////////////////////
void DEATHSPHERE_StartChargeWeapon( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_FREEZE )
	{
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_DEATHSPHERE_CHARGEWEAPON.\n" );

	// play the ready sequence
	AI_ForceSequence(self,"ready",FRAME_ONCE);

	// now charge for 2 seconds
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	pAIData->fValue = gstate->time + 0.75f;

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook,-1);
	
	gstate->StartEntitySound( self, CHAN_AUTO,gstate->SoundIndex("e1/m_dspherechargea.wav"),0.4f,400.0f, 512.0f);	
	
	AI_ZeroVelocity( self );
}


///////////////////////////////////////////////////////////////////////////////
//
//  DEATHSPHERE_ChargeWeapon
//
///////////////////////////////////////////////////////////////////////////////
void DEATHSPHERE_ChargeWeapon( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	if ( gstate->time > pAIData->fValue )
	{
		AI_SetOkToAttackFlag( hook, TRUE );
		AI_RemoveCurrentTask( self );
	}

	
	// look at enemy
	if(self->enemy)
		AI_OrientTowardPoint( self, self->enemy->s.origin );
	else
		AI_RemoveCurrentGoal(self);

	// try to keep a certain distance to the enemy
}

///////////////////////////////////////////////////////////////////////////////
//
//  deathsphere_set_attack_seq
//
///////////////////////////////////////////////////////////////////////////////
void deathsphere_set_attack_seq( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	hook->nAttackMode = ATTACKMODE_NORMAL;

	if ( AI_IsFacingEnemy(self, self->enemy, 2.0f, 10.0f) )
	{
		hook->nAttackMode = ATTACKMODE_RANGED;

		AI_ForceSequence(self,"ataka",FRAME_LOOP);
	}
	else
	{
		if(self->enemy)
			AI_FaceTowardPoint( self, self->enemy->s.origin );	
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//  deathsphere_begin_attack
//
//	it will issue a warning and charge for 2 seconds before firing
//
///////////////////////////////////////////////////////////////////////////////
void deathsphere_begin_attack( userEntity_t *self )
{
	AI_ZeroVelocity( self );

	deathsphere_set_attack_seq( self );
}

///////////////////////////////////////////////////////////////////////////////
//
//  deathsphere_checkforobstruction //NSS[11/6/99]:
//
///////////////////////////////////////////////////////////////////////////////
bool deathsphere_checkforobstruction( userEntity_t *self)
{
	CVector endPoint,Dir;
	
	Dir = self->s.angles;
	Dir.AngleToForwardVector(endPoint);
	
	//Just check a little ahead of the little sphere dude.
	endPoint = (endPoint * 64.0f)+self->s.origin;
	
	tr = gstate->TraceBox_q2( self->s.origin, self->s.mins,self->s.maxs, endPoint, self, MASK_SOLID);
	if(tr.fraction >= 1.0f)
		return FALSE;
	else
		return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//NSS[11/6/99]:
//  deathsphere_attack
//
///////////////////////////////////////////////////////////////////////////////
void deathsphere_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	// orient toward enemy (adjust pitch as well as yaw)
	AI_OrientTowardPoint( self, self->enemy->s.origin );


	if(AI_IsEnemyTargetingMe( self, self->enemy ) && rnd() >= 0.50f)
		deathsphere_avoid(self);
	
	if(!AI_DetectForLikeMonsters( self ))
	{
		
		if(!deathsphere_checkforobstruction(self))
		{
			if ( hook->nAttackMode == ATTACKMODE_RANGED )
			{

				if ( AI_IsReadyToAttack1( self ) )
				{
					AI_PlayAttackSounds( self );
					deathsphere_fire_weapon(self);
					hook->attack_boost = self->s.frame + 4;
				}
				else if ( self->s.frame >= hook->attack_boost && self->s.frame <= hook->attack_boost +1)
				{
					AI_PlayAttackSounds( self );
					deathsphere_fire_weapon(self);
				}

				if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
				{
//					playerHook_t *hook = AI_GetPlayerHook( self );	// SCG[1/23/00]: not used

					float dist = VectorDistance(self->s.origin, self->enemy->s.origin);
					if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible(self, self->enemy) )
					{
						AI_RemoveCurrentTask( self );
						return;
					}
					else
					{
						AI_RemoveCurrentTask( self, FALSE);
					}
				}
			}
			else
			{
				deathsphere_avoid(self);
			}
		}
		else
		{
			AI_RemoveCurrentTask(self,TASKTYPE_SIDESTEP);
		}
	}
	else
	{
		AI_RemoveCurrentTask(self,TASKTYPE_SIDESTEP);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//  deathsphere_fire_weapon
//
//	fires all four of its laser bolts at once
//
///////////////////////////////////////////////////////////////////////////////
void deathsphere_fire_weapon(userEntity_t* self)
{
	ai_weapon_t *weapon = (ai_weapon_t *) self->curWeapon;

	weapon->spread_x = 0.0f;
	weapon->spread_z = 0.0f;

	for ( int i = 0; i < 4; i++ )
	{
		// fire weapon i
		weapon->ofs = muzzle[i];

		ai_fire_curWeapon( self );
	}
}


///////////////////////////////////////////////////////////////////////////////
//
//  DEATHSPHERE_ParseEpairs
//
///////////////////////////////////////////////////////////////////////////////
void DEATHSPHERE_ParseEpairs(userEntity_t *self)
{
	AI_ParseEpairs(self);
	self->think = DEATHSPHERE_Think;
	self->nextthink = gstate->time + 0.1f;
}


///////////////////////////////////////////////////////////////////////////////
//
//  monster_deathsphere
//
///////////////////////////////////////////////////////////////////////////////
void monster_deathsphere( userEntity_t *self )
{
	playerHook_t *hook	= AI_InitMonster( self, TYPE_DEATHSPHERE );

	self->className		= MONSTER_NAME;
	self->netname		= tongue_monsters[T_MONSTER_DEATHSPHERE];

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

	gstate->SetSize (self, -16.0, -16.0, -24.0, 16.0, 16.0, 32.0);

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////
	hook->fnStartAttackFunc = deathsphere_begin_attack;
	hook->fnAttackFunc		= deathsphere_attack;

	//	turn rates for each axis in degrees per second
	self->ang_speed.Set( 90, 135, 90 );

	self->movetype			= MOVETYPE_HOVER;

	self->fragtype          |= FRAGTYPE_NOBLOOD;   // don't leave blood upon death
	self->fragtype          |= FRAGTYPE_ROBOTIC;   // apply robotic gib parts
	self->fragtype          |= FRAGTYPE_ALWAYSGIB; // boom, bitchie

	hook->dflags			|= DFL_MECHANICAL;
	hook->dflags			|= DFL_FACEPITCH;

	hook->run_speed			= 250;
	hook->walk_speed		= 50;
	hook->attack_speed		= 250;

	hook->active_distance	= 3000;
	hook->attack_dist		= 64;

	self->health			= 200;
	hook->base_health		= 200;
	hook->pain_chance		= 10;
	
	hook->bOkToAttack		= FALSE;

	hook->attack_dist		= 1024;

	self->think				= DEATHSPHERE_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;

	WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );

	// give a weapon
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
										  "death laser", 
										  deathlaser_fire, 
										  ITF_PROJECTILE|ITF_NOLEAD);
		gstate->InventoryAddItem(self, self->inventory, self->curWeapon);
	}

	hook->nAttackType = ATTACK_AIR_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	AI_DetermineMovingEnvironment( self );

	gstate->LinkEntity( self );
	hook->acro_boost = 1;

	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

}

void world_ai_deathsphere_register_func()
{
	gstate->RegisterFunc( "deathsphere_begin_attack", deathsphere_begin_attack );
	gstate->RegisterFunc( "deathsphere_attack", deathsphere_attack );
	gstate->RegisterFunc( "DEATHSPHERE_ParseEpairs", DEATHSPHERE_ParseEpairs );
	gstate->RegisterFunc( "DEATHSPHERE_Think", DEATHSPHERE_Think );
}