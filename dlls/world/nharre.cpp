// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#include "world.h"
#include "ai_common.h"
#include "ai_utils.h"
//#include "ai_move.h"
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"
#include "ai.h"


/* ***************************** define types ****************************** */
float LastSummon;
#define NHARRE_SUMMON_CHANCE 0.75f
#define NHARRE_SUMMON_DEMON_CHANCE 0.10f
#define NHARRE_MAX_TELEPORT_LOCATIONS 10

/* ***************************** Local Variables *************************** */
CVector N_Teleport[NHARRE_MAX_TELEPORT_LOCATIONS];

/* ***************************** Local Functions *************************** */

void nharre_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage );
void nharre_set_attack_seq( userEntity_t *self, int always_teleport = FALSE );
void CacheMonsterSounds(userEntity_t *self, char *className );

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

DllExport void monster_nharre(userEntity_t *self);

// ----------------------------------------------------------------------------
// NSS[12/13/99]:
// Name:NHARRE_Initialize_Teleports
// Description:Find all of the locations to teleport to.
// Input:userEntity_t *self
// Output:NA
// Note:Teleport Destinations should have a targetname of "nharre"
// ----------------------------------------------------------------------------
void NHARRE_Initialize_Teleports(userEntity_t *self)
{
	userEntity_t *ent = gstate->FirstEntity();
	CVector *TempV;
	int i= 0;
	
	//Process through all the entities on the map
	while(ent)
	{
		//Make sure the entity is the right classname
		if(ent->targetname)
		{
			if(!(_stricmp(ent->targetname, "nharre")))
			{
				// NSS[12/13/99]:Store the locations
				TempV = &N_Teleport[i];
				TempV->x = ent->s.origin.x;
				TempV->y = ent->s.origin.y;
				TempV->z = ent->s.origin.z;

				i++;
				if(i > NHARRE_MAX_TELEPORT_LOCATIONS)
				{
					break;
				}
				
			}
		}
		ent = gstate->NextEntity(ent);
	}
	self->hacks_int = i-1;

}


// ----------------------------------------------------------------------------
// NSS[12/13/99]:
// Name:NHARRE_StartTeleport
// Description:The start process of teleporting
// Input:userEntity_t *self
// Output:NA
// Note:Teleport Destinations should have a targetname of "nharre"
// ----------------------------------------------------------------------------
void NHARRE_StartTeleport(userEntity_t *self)
{
	int Marker;
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);	
	
	// NSS[12/13/99]:If we haven't initialized the teleporter info let's do so.
	if(!self->hacks_int)
	{
		NHARRE_Initialize_Teleports(self);
	}
	ai_frame_sounds( self );
	
	AI_SetOkToAttackFlag( hook, FALSE );
 
	Marker = (int)(self->hacks_int * rnd());
	
	// NSS[12/13/99]:Set the destination
	pAIData->destPoint.x = N_Teleport[Marker].x;
	pAIData->destPoint.y = N_Teleport[Marker].y;
	pAIData->destPoint.z = N_Teleport[Marker].z;
	
	// NSS[12/13/99]:we are visible right now.
	pAIData->nValue = 0;
	
	// NSS[12/13/99]:During this period he cannot be hit.
	self->takedamage = DAMAGE_NO;

	frameData_t *pSequence = FRAMES_GetSequence( self, "atakb" );
	AI_ForceSequence( self, pSequence );

	AI_SetTaskFinishTime( hook, -1.0f );

}

// ----------------------------------------------------------------------------
// NSS[12/13/99]:
// Name:NHARRE_Teleport
// Description:The actual process of teleporting
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void NHARRE_Teleport(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);	
	
	if(!pAIData->nValue)
	{
		// NSS[12/13/99]:If we are not in TRANSLUCENT mode.
		if(!(self->s.renderfx & RF_TRANSLUCENT))
		{
			self->s.renderfx |= RF_TRANSLUCENT;
		}
		if(self->s.alpha <= 0.10f)
		{
			// NSS[12/13/99]:We are basically invisible... let's warp.
			tr = gstate->TraceBox_q2( pAIData->destPoint, self->s.mins, self->s.maxs, pAIData->destPoint, self, MASK_PLAYERSOLID );
			if(!tr.allsolid && !tr.startsolid)
			{
				gstate->SetOrigin (self, pAIData->destPoint.x, pAIData->destPoint.y,pAIData->destPoint.z);
				pAIData->nValue = 1;
			}

		}
		else
		{
			self->s.alpha -= 0.10f;
		}
	}
	else
	{
		if(self->s.alpha >=1.0f)
		{
			self->s.renderfx &= ~ RF_TRANSLUCENT;
			self->takedamage = DAMAGE_YES;
			AI_RemoveCurrentTask(self,FALSE);
		}
		else
		{
			self->s.alpha += 0.10f;
		}
	}
	
	AI_SetNextThinkTime( self, 0.1f );
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
int AI_ComputeBestAwayYawPoint(userEntity_t *self, CVector &flyAwayPoint, int Distance, int Resolution);


int	SIDEKICK_FindFurthestNodeFrom (NodeTraverse *NodeT);



// ----------------------------------------------------------------------------
// NSS[12/17/99]:
// Name:nharre_set_attack_seq
// Description:sets up NHarre's attack mode...
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void nharre_set_attack_seq( userEntity_t *self, int always_teleport )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	int targeted = self->enemy && AI_IsEnemyTargetingMe( self, self->enemy );

	// choose whether to teleport, idle, or summon

	// only teleport if demanded or if self was targeted while idle last time around
	if ( always_teleport || (targeted && (rnd() < 0.25f) && self->hacks < gstate->time ) )
	{
		self->hacks = gstate->time + 2.0f;
		AI_AddNewTaskAtFront(self,TASKTYPE_NHARRE_TELEPORT,FALSE);
	}
	else if ( rnd() < NHARRE_SUMMON_CHANCE && (LastSummon < gstate->time))
	{
		// summon, but not twice in a row

		frameData_t *pSequence;

		hook->nAttackMode = ATTACKMODE_RANGED;		

		if ( rnd() < NHARRE_SUMMON_DEMON_CHANCE )
		{
			self->curWeapon = gstate->InventoryFindItem( self->inventory, "nharre summon demon" );
			pSequence = FRAMES_GetSequence( self, "atakc" );
			LastSummon = gstate->time + 10;
		}
		else
		{
			self->curWeapon = gstate->InventoryFindItem( self->inventory, "nharre summon" );
			pSequence = FRAMES_GetSequence( self, "atakb" );
			LastSummon = gstate->time + 1.5;
		}
		AI_ForceSequence( self, pSequence );
	}
	else if (!SIDEKICK_HasTaskInQue(self,TASKTYPE_MOVETOLOCATION) && self->enemy)// NSS[12/13/99]:Otherwise let's move around some... 
	{
		CVector				AwayPoint;
		NodeTraverse		NodeT;
		float fDistanceFrom	= VectorDistance(self->s.origin, self->enemy->s.origin);

		AI_SetStateRunning(hook);

		// NSS[1/24/00]:Setup Start Point
		NodeT.origin				= self->enemy->s.origin;
		NodeT.NodeDepth				= 0;
		NodeT.CurrentDistance		= VectorXYDistance(self->s.origin,self->enemy->s.origin);
		NodeT.CurrentNodeIndex		= hook->pNodeList->nCurrentNodeIndex;
		NodeT.GreatestDistance		= 0.0f;
		NodeT.pNodeList				= hook->pNodeList;
		NodeT.MaximumNodeDepth		= 5;
		NodeT.pBestNode				= NULL;
		NodeT.MaximumDistance		= 400.0f + fDistanceFrom;
		
		if(!SIDEKICK_FindFurthestNodeFrom (&NodeT))
		{
			// NSS[1/22/00]:If not then get the HideNode	
			MAPNODE_PTR pRetreatNode = NODE_GetClosestCompleteHideNode( hook->pNodeList->pNodeHeader, self, self->enemy );
			if(!pRetreatNode)
			{
				AI_ComputeBestAwayYawPoint(self, AwayPoint, 512.0f, 0.15f);
			}
			else
			{
				AwayPoint = pRetreatNode->position;
			}
		}
		else
		{
			AwayPoint = NodeT.pBestNode->position;
		}

		AI_AddNewTaskAtFront(self,TASKTYPE_MOVETOLOCATION,AwayPoint);
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
void nharre_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( hook->nAttackMode == ATTACKMODE_RANGED )
	{
		if ( self->s.alpha == 0.0f)
		{
			self->s.alpha = 0.5f; // one frame of fade-in
		}
		else
		{
			self->s.renderfx &= ~RF_TRANSLUCENT;
		}

		AI_FaceTowardPoint( self, self->enemy->s.origin );

		AI_PlayAttackSounds( self );

		if ( AI_IsReadyToAttack1( self ) && AI_IsFacingEnemy( self, self->enemy ) )
		{
			// "fire" our summoning "weapon"
			ai_fire_curWeapon( self );
		}
		else if (self->enemy)
		{
			AI_FaceTowardPoint(self,self->enemy->s.origin);
		}

		if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
		{
			nharre_set_attack_seq( self );
		}
	}
}


// ----------------------------------------------------------------------------
//
// Name:        nharre_start_pain
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void nharre_start_pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	TASKTYPE tasktype = GOALSTACK_GetCurrentTaskType( AI_GetCurrentGoalStack( hook ) );

	// check whether teleporting
	if ( tasktype == TASKTYPE_ATTACK && hook->nAttackMode == ATTACKMODE_TAKECOVER )
	{
		// give ourselves a damage rebate, self can't be damaged while teleporting
		self->health += damage;
	}
	else
	{
		ai_generic_pain_handler( self, attacker, kick,damage,35);
		// make sure we're not IN pain...
		if ( tasktype == TASKTYPE_ATTACK && self->deadflag == DEAD_NO && hook->pain_finished < gstate->time && self->hacks < gstate->time)
		{
			// set self up to teleport
			nharre_set_attack_seq( self, TRUE );
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
void nharre_begin_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	self->velocity.Zero();

	self->s.renderfx &= ~RF_TRANSLUCENT;

	hook->attack_finished = gstate->time + 1.0;

	nharre_set_attack_seq( self );
}


// ----------------------------------------------------------------------------
//
// Name:		monster_nharre
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_nharre( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_NHARRE );

	self->className			= "monster_nharre";
	self->netname			= tongue_monsters[T_MONSTER_NHARRE];

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

	//	turn rates for each axis in degrees per second
	self->ang_speed.Set( 90, 180, 90 );

	hook->fnStartAttackFunc = nharre_begin_attack;
	hook->fnAttackFunc		= nharre_attack;

	hook->run_speed			= 250;
	hook->walk_speed		= 120;
	hook->attack_speed		= 160;
	hook->pain_chance		= 33; // occasional pain, a vulnerability

	self->pain              = nharre_start_pain;
	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;

	// Nharre doesn't really move, he just teleports
	self->movetype          = MOVETYPE_WALK;

	//////////////////////////////////////////////
	//////////////////////////////////////////////
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
										  "nharre summon",
										  nharre_summon,
										  ITF_TRACE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
		self->curWeapon = ai_init_weapon( self, 
										  pWeaponAttributes[0].fBaseDamage,
										  pWeaponAttributes[0].fRandomDamage,
										  pWeaponAttributes[0].fSpreadX,
										  pWeaponAttributes[0].fSpreadZ,
										  pWeaponAttributes[0].fSpeed,
										  pWeaponAttributes[0].fDistance,
										  pWeaponAttributes[0].offset,
										  "nharre summon demon",
										  nharre_summon_demon,
										  ITF_TRACE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}

	hook->nAttackType = ATTACK_GROUND_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	AI_DetermineMovingEnvironment( self );
	
	
	hook->pain_chance		= 1;

	// NSS[12/17/99]:
	self->hacks_int = 0;
	LastSummon = gstate->time;
	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;

	// NSS[2/22/00]:Finally Cache the sounds for the monsters we are going to spawn.... 
	CacheMonsterSounds(self,"monster_buboid");
	// NSS[2/22/00]:Finally Cache the sounds for the monsters we are going to spawn.... 
	CacheMonsterSounds(self,"monster_doombat");
	// NSS[2/22/00]:Finally Cache the sounds for the monsters we are going to spawn.... 
	CacheMonsterSounds(self,"monster_plague_rat");


	gstate->LinkEntity(self);

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
void world_ai_nharre_register_func()
{
	gstate->RegisterFunc("nharre_start_pain",nharre_start_pain);
	gstate->RegisterFunc("nharre_begin_attack",nharre_begin_attack);
	gstate->RegisterFunc("nharre_attack",nharre_attack);
}
