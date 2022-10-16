// ==========================================================================
//  NSS[2/17/00]:
//  File:dragon.cpp
//  Contents:A REALLY quick hack to get the dragon in the game.
//  Author:Noel Stephens
//
// ==========================================================================
#include "world.h"
#include "ai.h"
#include "ai_utils.h"
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"
#include "ai_common.h"
/* ***************************** define types ****************************** */

#define DRAGON_FIRE_CHANCE 0.4
#define DRAGON_FLY_AWAY_DISTANCE  512.0f
#define DRAGON_HOVER_DISTANCE 512.0f;
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
void dragon_attack( userEntity_t *self );

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */
DllExport void monster_dragon( userEntity_t *self );
int AI_ComputeFlyAwayPoint2( userEntity_t *self, CVector &flyAwayPoint, int Distance, int Resolution, CVector &Axis);
int AI_HasTaskInQue( userEntity_t *self, TASKTYPE SearchTask );




// ----------------------------------------------------------------------------
// NSS[2/17/00]:
// Name:		DRAGON_Attack_Think
// Description:This will determine what the dragon needs to be doing.
// Input:self
// Output:<nothing>
// Note:
// ----------------------------------------------------------------------------
void DRAGON_Attack_Think( userEntity_t *self )
{
	//Attack states are no yet implemented.  So far all choices are based off of health and
	//random values.
	playerHook_t *hook = AI_GetPlayerHook( self );

	//Go ahead and remove the DRAGON_Attack_Think task from the set of tasks
	//We do this because we know that by the time this code is finished we will
	//have a new set of tasks assigned.
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );
	GOAL_PTR pGoal = pGoalStack->pTopOfStack;
	
	if ( !self->enemy )
	{
		self->enemy = pAIData->pEntity;
	}
	//Dumb temp flag to tell me that I have seen the player so if I go back to idle state I can reset my goaltype to wander
	hook->acro_boost = 1.0f;

	if ( AI_IsLineOfSight( self, self->enemy ) )
	{
		self->curWeapon = gstate->InventoryFindItem (self->inventory, "fireball");
		AI_Dprintf( "Started Long Range Attack Mode.\n" );
		AI_RemoveCurrentTask(self, TASKTYPE_DRAGON_HOVER);
		AI_AddNewTask(self, TASKTYPE_DRAGON_ATTACK);
	}
	else // Go into Path Finding Mode
	{
		// NSS[2/17/00]:Can't see the player so let's keep doing what we were doing in the first place(probably pathfollowing).
		AI_RemoveCurrentGoal(self);
	}
}

// ----------------------------------------------------------------------------
// NSS[2/17/00]:
// Name:		DRAGON_StartHover
// Description: Hover sequence intialization.  Just before Long Ranged Attack
// Input: userEntity_t *self 
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void DRAGON_StartHover( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	//Alert the player the we are gonna spit fire giz at them!
	gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e3/m_dragonsighta.wav"), 
                              1.0f, 2048.0f, 2048.0f);
	
	//Calculate a 1/4 of the way point to the player
	pAIData->destPoint.x = (self->enemy->s.origin.x - self->s.origin.x)/4;
	pAIData->destPoint.y = (self->enemy->s.origin.y - self->s.origin.y)/4;
	AI_ForceSequence(self,"hover",FRAME_LOOP);
	AI_ZeroVelocity(self);
	self->s.angles.roll = 0.0f;
	AI_SetTaskFinishTime( hook, -1 );
	self->nextthink	= gstate->time + 0.1f;
	pAIData->fValue = gstate->time + (1.0f + rnd()*2.0);
	AI_SetOkToAttackFlag( hook, FALSE );
}

// ----------------------------------------------------------------------------
// NSS[2/17/00]:
// Name:		DRAGON_Hover
// Description: Hover sequence intialization.  Just before Long Ranged Attack
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void DRAGON_Hover( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	AI_FaceTowardPoint(self,self->enemy->s.origin);
	if(AI_IsEndAnimation(self))
	{
		AI_ForceSequence(self,"hover",FRAME_LOOP);
	}
	if ( pAIData->fValue <= gstate->time)
	{
		AI_RemoveCurrentTask( self );
		return;
	}
}	

// ----------------------------------------------------------------------------
//<nss>
// Name:		DRAGON_StartFlyAway
// Description: Begin to fly away to a new random point
// Input:userEntity_t *self
// Output:<none>
// Notes:
// Axis--> This vector is to tell the AI_ComputeFlyAwayPoint what axis it should
// search.  DO NOT SET THE VALUE ABOVE 1 OR IT WILL MULTIPLY THE RESULTANT VECOTOR
// FOR THAT AXIS BY ANY VALUE GREATER THAN 1.  To eliminate an axis, just set that
// axis to the value of 0.
// flyAwaPoint-->the Vector which will have the new location stored in
// ----------------------------------------------------------------------------
void DRAGON_StartFlyAway( userEntity_t *self )
{
	CVector flyAwayPoint;
	CVector Axis;
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_DOOMBAT_FLYAWAY )
		{
			return;
		}
	}
	//Special case move around objects
	if(pAIData->destPoint.x || pAIData->destPoint.y || pAIData->destPoint.z)
	{
		Axis = pAIData->destPoint;
		AI_ComputeFlyAwayPoint2( self, flyAwayPoint, DRAGON_FLY_AWAY_DISTANCE/2, 12, Axis );
		pAIData->destPoint = flyAwayPoint;
	}
	else
	{
		flyAwayPoint = self->s.origin;
		Axis.x = 1;
		Axis.y = 0;
		Axis.z = 1;
		if(AI_ComputeFlyAwayPoint( self, flyAwayPoint, DRAGON_FLY_AWAY_DISTANCE, 12, &Axis )==0)
		{
			
			Axis.x = 0;
			Axis.y = 1;
			Axis.z = 1;
			if(AI_ComputeFlyAwayPoint( self, flyAwayPoint, DRAGON_FLY_AWAY_DISTANCE, 12, &Axis )==0)
			{
				AI_Dprintf( "No point found setting my new origin to above enemy's head.\n");
				flyAwayPoint = self->enemy->s.origin;
				flyAwayPoint.z += DRAGON_HOVER_DISTANCE;
			}
		}

		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
		AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );
		
		MAPNODE_PTR Node = 	NODE_GetClosestNode( self, flyAwayPoint);

		if(!Node)
		{
			AI_Dprintf("BAD THINGS ARE HAPPENING!!!Flyaway with no Node system initialized!\n");
			AI_RemoveCurrentTask(self,FALSE);
			return;
		}

		pAIData->destPoint = Node->position;
	}

	//Setup things for fly_to_point function
	pAIData->cDYaw = pAIData->gDYaw = pAIData->nValue = 0;
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1 );
	self->nextthink	= gstate->time + 0.1f;
}


// ----------------------------------------------------------------------------
// NSS[2/17/00]:
// Name:		DRAGON_FlyAway
// Description: Flyaway... 
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void DRAGON_FlyAway( userEntity_t *self )
{
	userEntity_t *attacker = self->enemy;
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	float fDistance = VectorDistance( self->s.origin, pAIData->destPoint );
	// NSS[12/13/99]:More than likely this is an invalid point
	if(pAIData->destPoint.Length() == 0.0f || AI_IsCloseDistance2(self,fDistance))
	{
		AI_RemoveCurrentTask(self,FALSE);
	}
	//Check to see if we are stuck.
	if(AI_FLY_Debug_Origin(self))
	{
		pAIData->nValue++;
	}

	pAIData->fValue		= 0.15f;
	
	AI_ComputeMovingSpeed( hook );
	
	AI_FlyTowardPoint2( self, pAIData->destPoint, pAIData->fValue);

	//Set our last origin to help with collision detection
	hook->last_origin = self->s.origin;
}

// ----------------------------------------------------------------------------
// NSS[2/17/00]:
// Name:		DRAGON_Attack
// Description:Attack... 
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void DRAGON_Attack(userEntity_t *self)
{
	//Create the Special FX for the breath
	trackInfo_t tinfo;
	playerHook_t *hook = AI_GetPlayerHook( self );
	AIDATA_PTR pAIData;
	AI_FaceTowardPoint( self, self->enemy->s.origin );
	CVector Dir;
	AI_PlayAttackSounds( self );
	
	if(!AI_IsAlive(self))
	{
		AI_AddNewTaskAtFront(self,TASKTYPE_DIE);
		return;
	}
	// NSS[2/23/00]:Dragon breath ... bleah!
	pAIData = AI_GetAIDATA(self);
	if(pAIData)
	{
		if(pAIData->fValue < gstate->time)
		{
			if(self->enemy && !pAIData->nValue)
			{
				// NSS[2/22/00]:Get the direction
				Dir = self->enemy->s.origin - self->s.origin;
				Dir.Normalize();

				// clear this variable
				memset(&tinfo, 0, sizeof(tinfo));

				tinfo.ent			=	self;
				tinfo.srcent		=	self;
				tinfo.fru.Zero();
				tinfo.Long1			= ART_DRAGON_BREATH;
				tinfo.flags			= TEF_FXFLAGS|TEF_SRCINDEX|TEF_LONG1|TEF_ALTPOS|TEF_ALTPOS2|TEF_DSTPOS;
				tinfo.fxflags		= TEF_ARTIFACT_FX|TEF_FX_ONLY;
				tinfo.dstpos.Set( Dir );				//Used for the direction
				tinfo.altpos.Set(0.0f,0.0f, 0.0f);	//First Color (smaller Particles)
				tinfo.altpos2.Set(0.70f,0.35f,0.05f);	//Second Color (larger Particles)

				com->trackEntity(&tinfo,MULTICAST_ALL);
				pAIData->fValue = gstate->time + 0.85f;
				pAIData->nValue = 1;
			}			
			else if (pAIData->nValue)
			{
				RELIABLE_UNTRACK(self);
				pAIData->fValue = gstate->time + 10000.0f;
			}
		}
	}
	
	if ( AI_IsReadyToAttack1( self ))
	{
		ai_fire_curWeapon( self );
	}

	// NSS[2/17/00]:When done with our animation let's remove our current task.
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		AI_RemoveCurrentTask(self);
	}
}

// ----------------------------------------------------------------------------
// NSS[2/17/00]:
// Name:		DRAGON_BeginAttack
// Description:start attcking.
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void DRAGON_BeginAttack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	AIDATA_PTR pAIData;

	AI_ZeroVelocity( self );
	if(self->enemy)
		AI_FaceTowardPoint(self,self->enemy->s.origin);
	self->s.angles.roll = 0.0f;
	frameData_t *pSequence = FRAMES_GetSequence( self, "ataka" );
	AI_ForceSequence( self, pSequence, FRAME_ONCE );
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1 );
	pAIData = AI_GetAIDATA(self);
	if(pAIData)
	{
		pAIData->fValue = gstate->time + 1.10f;
	}
}

// ----------------------------------------------------------------------------
// NSS[2/17/00]:
// Name:		DRAGON_Think
// Description:Trap before AI_TaskThink to do any 'on the fly' debugging or
// trap for specific conditions.
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void DRAGON_Think(userEntity_t *self)
{
	if(AI_HasTaskInQue(self,TASKTYPE_PATHFOLLOW) && self->hacks < gstate->time)
	{
		if(rnd() < 0.25)
		{
			if(rnd() > 0.50)
			{
				gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e3/m_dragonsighta.wav"), 1.0f, 900.0f, 1024.0f);
			}
			else
			{
				gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e3/m_dragonsightb.wav"), 1.0f, 900.0f, 1024.0f);
			}
		}
		self->hacks = gstate->time + 5.0f;
	}
	AI_TaskThink(self);
}

// ----------------------------------------------------------------------------
// NSS[2/17/00]:
// Name:		DRAGON_ParseEpairs
// Description:Trap to get the Dragon Think going through a specific point...the norm.
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void DRAGON_ParseEpairs(userEntity_t *self)
{
	AI_ParseEpairs(self);
	self->think = DRAGON_Think;
	self->nextthink = gstate->time + 0.1f;
}


// ----------------------------------------------------------------------------
// NSS[2/17/00]:
// Name:		monster_dragon
// Description:The dragon... that has been sooo close to being axed... 
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void monster_dragon( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_DRAGON );

	self->className			= "monster_dragon";
	self->netname			= tongue_monsters[T_MONSTER_DRAGON];

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

	//	turn rates for each axis in degrees per second
	self->ang_speed.Set( 20,15 ,45 );

	self->movetype			= MOVETYPE_FLY;
	
	hook->dflags			|= DFL_FACEPITCH;

	hook->fnStartAttackFunc = DRAGON_BeginAttack;
	hook->fnAttackFunc		= DRAGON_Attack;

	self->think				= DRAGON_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give dragon a weapon
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
										  "fireball",
										  fireball_attack,
										  ITF_PROJECTILE | ITF_EXPLOSIVE);
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}

	hook->nAttackType = ATTACK_AIR_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;
	self->fragtype     |= FRAGTYPE_ALWAYSGIB;

	AI_DetermineMovingEnvironment( self );

	gstate->LinkEntity(self);

	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

	// NSS[3/20/00]:Fireball sound cache
	gstate->SoundIndex("global/e_explodef.wav");
	gstate->SoundIndex("global/e_explodeq.wav");
	gstate->SoundIndex("global/e_exploded.wav");

	// NSS[6/7/00]:Mark this monster as a boss
	hook->dflags |= DFL_ISBOSS;


}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_dragon_register_func()
{
	gstate->RegisterFunc("DRAGON_Think",DRAGON_Think);
	gstate->RegisterFunc("DRAGON_ParseEpairs",DRAGON_ParseEpairs);
	gstate->RegisterFunc("DRAGON_BeginAttack",DRAGON_BeginAttack);
	gstate->RegisterFunc("DRAGON_Attack",DRAGON_Attack);
}


