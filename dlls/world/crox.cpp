// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#include "world.h"
#include "ai.h"
#include "ai_utils.h"
//#include "ai_move.h"	// SCG[1/23/00]: no used
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "monstersound.h"


/* ***************************** define types ****************************** */

#define MONSTERNAME			  "monster_crox"
#define CROX_ATTACKSTR_1      "punch"

/* ***************************** Local Functions *************************** */

void crox_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                     int damage, CVector &destPoint );

void crox_attack( userEntity_t *self );
void crox_set_attack_seq( userEntity_t *self );

/* ******************************* exports ********************************* */

DllExport void monster_crox( userEntity_t *self );


void crox_set_attack_seq( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	frameData_t *pSequence = NULL;

	if ( AI_IsFacingEnemy( self, self->enemy ) )
	{
		if ( self->waterlevel < 3 )
		{
			if (rnd() < 0.666f)
			{
				pSequence = FRAMES_GetSequence( self, "atakc" );
			}
			else
			{
				pSequence = FRAMES_GetSequence( self, "atakd" );
			}
		}
		else
		{
			if (rnd() < 0.666f)
			{
				pSequence = FRAMES_GetSequence( self, "atakb" );
			}
			else
			{
				pSequence = FRAMES_GetSequence( self, "ataka" );
			}
		}

		hook->nAttackMode = ATTACKMODE_MELEE;
	}
	else
	{
		// play a moving sequence instead
		if ( self->waterlevel < 3 )
		{
			pSequence = FRAMES_GetSequence( self, "walka" );
		}
		else
		{
			pSequence = FRAMES_GetSequence( self, "swima" );
		}

		hook->nAttackMode = ATTACKMODE_NORMAL;
	}

	AI_ForceSequence(self, pSequence);

	self->curWeapon = gstate->InventoryFindItem(self->inventory, CROX_ATTACKSTR_1);
}


// attack the enemy
void crox_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	float dist = VectorDistance (self->s.origin, self->enemy->s.origin);

	if(!AI_IsWithinAttackDistance( self, dist ))
	{
		AI_RemoveCurrentTask( self , FALSE); // remove attack task
		//AI_SetStateRunning(hook);
		//AI_MoveTowardPoint( self, self->enemy->s.origin, FALSE );
		return;
	}
	
	if ( hook->nAttackMode == ATTACKMODE_MELEE && AI_IsReadyToAttack1( self ) &&
	     AI_IsFacingEnemy( self, self->enemy ) )
	{
        AI_PlayAttackSounds( self );
		ai_fire_curWeapon( self );
	}

	// make a choice if not attacking or attack animation is ending
	if ( !AI_IsEnemyDead( self ) &&
	     (hook->nAttackMode == ATTACKMODE_NORMAL || AI_IsEndAnimation( self )) )
	{
		AI_RemoveCurrentTask( self , FALSE); // remove attack task
	}
}



// initiate attack sequence
void crox_begin_attack( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );	// SCG[1/23/00]: not used

	AI_ZeroVelocity( self );

	crox_set_attack_seq( self );  // start attack sequence
}


void crox_start_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
                     int damage, CVector &destPoint )
{
	int notdeadyet = (self->deadflag == DEAD_NO);

	// call AI_StartDie, which does generic dying stuff
	AI_StartDie( self, inflictor, attacker, damage, destPoint );

    if ( notdeadyet && self->deadflag == DEAD_DYING )
	{ // pick a different sequence for different deaths
		if ( rnd() < 0.5f )
		{
			AI_ForceSequence( self, "diea" ); // fall over slowly
		}
		else
		{
			AI_ForceSequence( self, "dieb" ); // collapse
		}
	}
}
///////////////////////////////////////////////////////////////////////////////
//
//  crox_parseEpairs
//NSS[11/12/99]:Trap for debugging and possible changes.
///////////////////////////////////////////////////////////////////////////////
void crox_think(userEntity_t *self)
{
	int catchanimation = 0;
	playerHook_t *hook = AI_GetPlayerHook( self );
	if(hook)
	{
		//Get the goal pointers and task pointers
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		GOAL_PTR pGoal = pGoalStack->pTopOfStack;

		if( pGoal == NULL )
		{
			AI_AddNewGoal(self,GOALTYPE_WANDER);
			self->nextthink = gstate->time + 0.1f;
			return;
		}
		//NSS[11/16/99]:Check for player distance in the Z axis.. if > than 350.0f go into wander
		if(pGoal->nGoalType == GOALTYPE_KILLENEMY && self->enemy)
		{
			if(abs(self->enemy->s.origin.z - self->s.origin.z) > 100.0f)
			{
				AI_AddNewGoal(self,GOALTYPE_WANDER);
				hook->ai_flags |= AI_IGNORE_PLAYER;
				self->nextthink = gstate->time + 0.1f;
				return;
			}
		}
		else if(pGoal->nGoalType == GOALTYPE_WANDER && self->enemy)
		{
			if(abs(self->enemy->s.origin.z - self->s.origin.z) <= 100.0f)
			{
				AI_RemoveCurrentGoal(self);
				hook->ai_flags &= ~AI_IGNORE_PLAYER;
			}
		}
		
		if(pGoalStack->pTopOfStack)
		{
			if(pGoalStack->pTopOfStack->pTasks)
			{
				if(pGoalStack->pTopOfStack->pTasks->pStartList)
				{
					if(pGoalStack->pTopOfStack->pTasks->pStartList->nTaskType != TASKTYPE_ATTACK)
					{
						AI_Dprintf("Waterlevel:%d\n",self->waterlevel);
						if(self->waterlevel >= 2 && AI_IsAlive(self) && AI_IsEndAnimation( self ))
						{
							self->movetype = MOVETYPE_SWIM;
							catchanimation = 1;
						}
						else if(self->waterlevel < 2 && AI_IsAlive(self) && AI_IsEndAnimation( self ))
						{
							self->movetype = MOVETYPE_WALK;
							catchanimation = 1;
						}		
					}
				}
			}
		}
	}
	
	AI_TaskThink(self);

	if(self->movetype == MOVETYPE_SWIM && catchanimation && AI_IsAlive(self))
	{
		AI_ForceSequence( self, "swima",FRAME_LOOP);
		if((int)gstate->time % 2)
		{
			char buffer[42];
			int number = (int)(1+rnd()*3);
//			sprintf(buffer,"hiro/swim%d.wav",number);
			Com_sprintf(buffer,sizeof(buffer),"hiro/swim%d.wav",number);
			gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex(buffer), 0.85f, hook->fMinAttenuation, hook->fMaxAttenuation);
		}
	}
	else if(self->movetype == MOVETYPE_WALK && catchanimation && AI_IsAlive(self))
	{
		self->movetype = MOVETYPE_WALK;
		if(self->enemy)
		{
			AI_SetStateRunning(hook);
			AI_ForceSequence( self, "runa",FRAME_LOOP);
		}
		else
		{
			AI_SetStateWalking(hook);
			AI_ForceSequence( self, "walka",FRAME_LOOP);
		}
	}

}

///////////////////////////////////////////////////////////////////////////////
//
//  crox_parseEpairs
//
///////////////////////////////////////////////////////////////////////////////
void crox_parseEpairs(userEntity_t *self)
{
	AI_ParseEpairs(self);
	self->think = crox_think;
	self->nextthink = gstate->time + 0.1f;
}

///////////////////////////////////////////////////////////////////////////////
//
//  monster_crox
//
///////////////////////////////////////////////////////////////////////////////

void monster_crox( userEntity_t *self )
{
	playerHook_t *hook = AI_InitMonster( self, TYPE_ROBOCROC );

	self->className		= MONSTERNAME;
	self->netname		= tongue_monsters[T_MONSTER_CROX];

	char *szModelName = AIATTRIBUTE_GetModelName( self->className );
    _ASSERTE( szModelName );
    self->s.modelindex = gstate->ModelIndex( szModelName );

	if ( !ai_get_sequences(self) )
	{
		char *szCSVFileName = AIATTRIBUTE_GetCSVFileName( self->className );
        _ASSERTE( szCSVFileName );
        FRAMEDATA_ReadFile( szCSVFileName, self );
	}
    ai_register_sounds(self); // register all animation sounds

	

	// set default attributes

	hook->dflags			|= DFL_ORIENTTOFLOOR;
	hook->dflags			|= DFL_AMPHIBIOUS;

	hook->fnStartAttackFunc = crox_begin_attack;
	hook->fnAttackFunc		= crox_attack;

	hook->nAttackType       = ATTACK_GROUND_MELEE;
	hook->nAttackMode       = ATTACKMODE_NORMAL;

	self->think             = crox_parseEpairs;
	self->nextthink         = gstate->time + 0.2;

	hook->attack_dist       = 80;

	hook->pain_chance       = 20;

    self->fragtype         |= FRAGTYPE_ROBOTIC;
    self->fragtype         |= FRAGTYPE_NOBLOOD;

	self->die = crox_start_die;

    // read in attributes from .csv file
    WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );


	///////////////////////////////////////////////////////////////////////////
	//	give crox a 'weapon', per se
	///////////////////////////////////////////////////////////////////////////

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
										  CROX_ATTACKSTR_1,
										  melee_punch,
										  ITF_TRACE);

		gstate->InventoryAddItem(self,self->inventory,self->curWeapon);
	}

	
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
void world_ai_crox_register_func()
{
	gstate->RegisterFunc( "crox_start_die", crox_start_die );
	gstate->RegisterFunc( "crox_begin_attack", crox_begin_attack );
	gstate->RegisterFunc( "crox_attack", crox_attack );
	gstate->RegisterFunc( "crox_parseEpairs", crox_parseEpairs );
	gstate->RegisterFunc( "crox_think", crox_think );
}
