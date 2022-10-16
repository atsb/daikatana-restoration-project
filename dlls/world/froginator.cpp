// ==========================================================================
//
//  File:Froginator "The Slime Days"
//  Contents:The frog's brains!
//  Author:N.S.S.
//
// ==========================================================================
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
#include "ai_common.h"
#include "ai_think.h"

/* ***************************** define types ****************************** */

const float FROG_JUMP_RANGE			= 500.0f;
const float FROG_ATTACK_DISTANCE    = 400.0f;
const float FROG_RANGED_DISTANCE	= 80.0f;

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
//void AI_OrientToFloor( userEntity_t *self );// SCG[1/23/00]: not used
void AI_StartChase( userEntity_t *self );
void AI_Chase( userEntity_t *self);


/* ******************************* exports ********************************* */

DllExport void monster_froginator (userEntity_t *self);


///////////////////////////////////////////////////////////////////////////////////
// frog specific functions
///////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
//
// Name:		FROG_Think
// Description:All internal pre-thinking stuff goes here
// Input:self
// Output:NA
// Note: 
// I put a hook in the for think (instead of standard AI_TASKTHINK) for several
// reasons, the first is that it is easier to debug when you have multiple entities,
// the second is that there are some things you might need to do before the AI_TASKTHINK
// ----------------------------------------------------------------------------
void FROG_Think(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if(self->movetype == MOVETYPE_SWIM)
	{
		// NSS[2/17/00]:Adding a check for the 'current sequence'.
		// I have to add this check because nothing gets restored from a saved game but location and health... everything else has to 'know' how to set itself back up
		// talk about dumb.
		if(hook && AI_IsAlive(self))
		{
			if(hook->cur_sequence)
			{
				if(hook->cur_sequence->animation_name)
				{
					if(!strstr(hook->cur_sequence->animation_name,"swim"))
					{
						AI_ForceSequence(self,"swima");
					}
				}
			}
		}
		self->curWeapon = gstate->InventoryFindItem (self->inventory, "punch");
	}
	
	AI_TaskThink(self);
	if(self->movetype == MOVETYPE_SWIM)
	{
		self->velocity.x = self->velocity.x * 0.55f;
		self->velocity.y = self->velocity.y * 0.55f;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		FROG_Start_Attack_Think
// Description:Begining think function for ATTACK_THINK
// Input:self
// Output:<nothing>
// Note:
// 
// ----------------------------------------------------------------------------
void FROG_Start_Attack_Think( userEntity_t *self )
{
	return;
}



// ----------------------------------------------------------------------------
//
// Name:		FROG_Attack_Think
// Description:Determines what tasks the frog will do during its attack
// Input:self
// Output:<nothing>
// Note:
// 
// ----------------------------------------------------------------------------
void FROG_Attack_Think( userEntity_t *self )
{
	
	//Attack states are no yet implemented.  So far all choices are based off of health and
	//random values.
	playerHook_t *hook = AI_GetPlayerHook( self );

	CVector Skill;
	Skill.Set(0.0f,1.0f,1.0f);

	//Get the goal pointers and task pointers
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData( pCurrentTask );

	if ( !self->enemy )
	{
		self->enemy = pAIData->pEntity;
		if(!self->enemy)
		{
			AI_RemoveCurrentTask(self,FALSE);
		}
	}

	AI_SetOkToAttackFlag( hook, TRUE );
	float fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );

	CVector Destination = self->s.origin;
	Destination.z -= 20;

	tr = gstate->TraceBox_q2( self->s.origin, self->s.mins, self->s.maxs, Destination, self, MASK_MONSTERSOLID );
	
	if( (fDistance < FROG_ATTACK_DISTANCE) && (rnd() < 0.40f)  && AI_GetSkill(Skill))
	{
			AI_RemoveCurrentTask(self,TASKTYPE_FROG_ATTACK,self);
	}
	else
	{
		AI_RemoveCurrentTask(self,TASKTYPE_FROG_CHASE,self);
	}

	//Important to set your think time.... 
	self->nextthink = gstate->time + 0.1f;

}


// ----------------------------------------------------------------------------
//
// Name:		FROG_Start_Chase
// Description:Hook before the standard AI_Start_Chase to setup specific choices
// Input:userEntity_t *self
// Output:NA
// Note:NATM
//
// ----------------------------------------------------------------------------
void FROG_Start_Chase( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
 	self->nextthink	= gstate->time + 0.1f;
	AI_Dprintf("Frog Start Chase!\n");
	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	if ( !self->enemy )
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		AIDATA_PTR pAIData = GOAL_GetData(pCurrentGoal);
		self->enemy = pAIData->pEntity;
		if(!self->enemy)
		{
			AI_RemoveCurrentTask(self,FALSE);
			return;
		}
	}

    userEntity_t *pEnemy = self->enemy;
    float fXYDistance = VectorXYDistance( self->s.origin, pEnemy->s.origin );
    float fZDistance = VectorZDistance( self->s.origin, pEnemy->s.origin );
    if ( !AI_FindPathToEntity( self, pEnemy, FALSE ) &&
         !AI_IsOkToMoveStraight( self, pEnemy->s.origin, fXYDistance, fZDistance ) )
	{
		// do not try another path for 2 seconds
		AI_SetNextPathTime( hook, 2.0f );

		//	no path was found, so do not do a chasing attack
		AI_RemoveCurrentGoal( self );

		return;
	}
    
	
	AI_SetStateRunning( hook );

	if(self->movetype == MOVETYPE_SWIM && AI_IsEndAnimation(self))
	{
		AI_ForceSequence(self,"swima",FRAME_LOOP);
	}


	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_CHASE )
		{
			return;
		}
	}

	FROG_Chase( self );

 	self->nextthink	= gstate->time + 0.1f;

	AI_SetOkToAttackFlag( hook, FALSE );

	AI_SetTaskFinishTime( hook, 10.0f );
	AI_SetMovingCounter( hook, 0 );
}
void AI_OrientToFloor( userEntity_t *self );
// ----------------------------------------------------------------------------
//
// Name:		FROG_Chase
// Description:Hook before the standard AI_Chase to setup specific choices
// Input: userEntity_t
// Output:NA
// Note:NATM
//
// ----------------------------------------------------------------------------
void FROG_Chase( userEntity_t *self )
{
	CVector Skill;
	Skill.Set(0.0f,1.0f,2.0f);

	AI_OrientToFloor(self);
	if(!self->enemy)
	{
		AI_RemoveCurrentTask(self,FALSE);
	}
	float fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );

	if( ((fDistance > FROG_ATTACK_DISTANCE/2 ) && (fDistance < FROG_JUMP_RANGE) && (rnd() > 0.50f)) && self->movetype != MOVETYPE_SWIM && AI_GetSkill(Skill) > 1)// || ((self->movetype == MOVETYPE_SWIM) && rnd() > 0.35f))//If not should we jump at the player?
	{
		AI_AddNewTaskAtFront(self,TASKTYPE_FROG_JUMP);
	 	self->nextthink	= gstate->time + 0.1f;
		return;
	}
	self->curWeapon = gstate->InventoryFindItem (self->inventory, "punch");
	if(AI_IsEndAnimation( self ))
	{
		frameData_t *pSequence = FRAMES_GetSequence( self, "runa" );
		if(self->movetype == MOVETYPE_SWIM)
		{
			pSequence = FRAMES_GetSequence( self, "swima" );
		}
		AI_ForceSequence( self, pSequence, FRAME_LOOP );
	}
	// NSS[2/29/00]:If within distance let's go for it.
	fDistance = VectorDistance(self->s.origin,self->enemy->s.origin);
	if(AI_IsWithinAttackDistance(self,fDistance,self->enemy))
	{
		AI_RemoveCurrentTask(self,TASKTYPE_FROG_ATTACK);
		return;
	}
	//otherwise let's keep chasing them down!
	AI_Chase(self);
}

// ----------------------------------------------------------------------------
//
// Name:		FROG_Start_Jump
// Description:Set up the initial velocity vector for a nice jump arch
// Input: userEntity_t *self
// Output:NA
// Note: 
//
// ----------------------------------------------------------------------------
void FROG_Start_Jump( userEntity_t *self )
{
	CVector			vel;
	float			fDistance;
	frameData_t *pSequence = FRAMES_GetSequence( self, "atakc" );
	AI_ForceSequence( self, pSequence );
	
	playerHook_t	*hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	//NSS[11/23/99]:Hack for froginator to wander around in shallow water *phttt*
	//Get vector to point
	if(self->enemy)
	{
		vel = self->enemy->s.origin - self->s.origin;
		vel.Normalize();
	}
	else
		vel.Set(crand(),crand(),crand());
	
	//NSS[11/23/99]:Hack for froginator to wander around in shallow water *phttt*
	//Get distance and calculate the Amount of force we need.
	if(self->enemy)
	{
		fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );
		tr		= gstate->TraceLine_q2(self->s.origin, self->enemy->s.origin, self, MASK_SOLID);
		if(fDistance < 0.0f)
			fDistance *= -1;
		if(tr.fraction >=1.0)
		{
			pAIData->destPoint = self->enemy->s.origin;
		}
		else
		{
			CVector Dir;
			Dir.Set(crand(),crand(),0);
			pAIData->destPoint = self->s.origin + 100 * Dir;
		}
	}
	else
	{
		fDistance = 100.0f;
		pAIData->destPoint = self->s.origin + (fDistance * vel);
	}
	
	vel = vel * (hook->run_speed * ((fDistance/FROG_JUMP_RANGE)*2.35));
	self->velocity = vel;
	self->velocity.z = hook->upward_vel;

	self->curWeapon = gstate->InventoryFindItem (self->inventory, "punch");
	
	self->movetype = MOVETYPE_BOUNCE;
	//This needs to be fixed....
	gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e1/m_frogjumpa.wav"), 0.85f, hook->fMinAttenuation, hook->fMaxAttenuation);
	gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e1/m_frogamba.wav"), 0.65f, hook->fMinAttenuation, hook->fMaxAttenuation);
	//Set the Time marker to end the jump sequence.
	pAIData->fValue = gstate->time; 

	AI_SetTaskFinishTime( hook, -1);
	self->nextthink	= gstate->time + 0.1f;
	AI_SetOkToAttackFlag( hook, FALSE );
	self->s.origin.z += 10.0f;
}

// ----------------------------------------------------------------------------
//
// Name:		FROG_Pain
// Description: Let's do something when the player hits us...
// Input:userEntity_t *self, userEntity_t *attacker, float kick, int damage
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void FROG_Pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	
	playerHook_t	*hook = AI_GetPlayerHook( self );
	AI_StartPain(self , attacker, kick, damage);
	AI_SetOkToAttackFlag( hook, TRUE );
}


// ----------------------------------------------------------------------------
//
// Name:		Frog Jump
// Description: Do a jump to get closer, if close enough initiate frog attack!
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void FROG_Jump( userEntity_t *self )
{
	CVector vel;
	playerHook_t	*hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	CVector Face,Angles;
	
	Face = self->velocity;
	Face.Normalize();
	VectorToAngles(Face,Angles);
	self->s.angles = Angles;
	
	if(self->groundEntity)
	{
		//Remove the Jump function and add the Jumpforward function
		AI_Dprintf("Adding JumpForward Task!\n");
		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e1/m_frogambb.wav"), 0.85f, ATTN_NORM_MIN,ATTN_NORM_MAX);
		AI_RemoveCurrentTask(self,FALSE);
		AI_SetOkToAttackFlag( hook, TRUE );
		self->movetype = MOVETYPE_HOP;
	}
/*	else
	{
		if(self->velocity.Length() > 0.0f)
		{
			self->movetype = MOVETYPE_WALK;
			float fDistance = FROG_JUMP_RANGE - VectorDistance( self->s.origin, pAIData->destPoint );
		}
		else
		{
			AI_Dprintf("Adding JumpForward Task!\n");
			gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e1/m_frogambb.wav"), 0.85f, ATTN_NORM_MIN,ATTN_NORM_MAX);
			AI_RemoveCurrentTask(self,FALSE);
			AI_SetOkToAttackFlag( hook, TRUE );			
		}
	}*/
}

// ----------------------------------------------------------------------------
//
// Name:		FROG_Begin_Attack
// Description: Determine what animations we should display
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void FROG_Begin_Attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	CVector Skill;
	pAIData->cDYaw = 0;
	Skill.Set(0.0f,1.0f,1.0f);
	if(!self->enemy)
	{
		return;
	}
	
	AI_ZeroVelocity(self);
	
	float fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );	

	if ( fDistance > FROG_RANGED_DISTANCE && self->waterlevel < 1  && AI_GetSkill(Skill))
	{
		if(AI_GetSkill(Skill))
		{
			AI_ForceSequence( self, "ataka" );
			self->curWeapon = gstate->InventoryFindItem (self->inventory, "FrogSpit");
		}
		else
		{
			AI_RemoveCurrentTask(self,FALSE);
		}
	}
	else if ( fDistance <= FROG_RANGED_DISTANCE)
	{
		AI_ForceSequence( self, "atakb" );
		self->curWeapon = gstate->InventoryFindItem (self->inventory, "punch");
	}
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1 );
	if(self->movetype != MOVETYPE_SWIM && self->velocity.z < 20.0f)
		self->velocity.Zero();
}


// ----------------------------------------------------------------------------
//
// Name:		FROG_Attack
// Description:The actual attacking function
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void FROG_Attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	if ( !self->enemy )
	{
		AI_RemoveCurrentTask(self,TASKTYPE_FROG_ATTACK_THINK,self, FALSE);
		AI_SetOkToAttackFlag( hook, TRUE );
		return;
	}
	
	AI_FaceTowardPoint( self, self->enemy->s.origin );
	
	if(self->movetype != MOVETYPE_SWIM)
	{
		CVector Destination = self->s.origin;
		Destination.z -= 20;
		
		AI_FaceTowardPoint( self, self->enemy->s.origin );

		tr = gstate->TraceBox_q2( self->s.origin, self->s.mins, self->s.maxs, Destination, self, MASK_MONSTERSOLID );
		//if we are still in the air drop down.
		if(tr.fraction >= 1.0f)
		{
			AI_RemoveCurrentTask(self,FALSE);
	 		self->nextthink	= gstate->time + 0.1f;
			AI_SetOkToAttackFlag( hook, TRUE );
			return;
		}
	}
	
	AI_PlayAttackSounds( self );
	if ( AI_IsReadyToAttack1( self ) && pAIData->cDYaw < 3)
	{
		AI_Dprintf("Fire!!\n");
		ai_fire_curWeapon( self );
		pAIData->cDYaw++;
	}
    if ( AI_IsReadyToAttack2( self ) && pAIData->cDYaw < 3 )
    {
		AI_Dprintf( "Frog firing second weapon!!\n" );
		ai_fire_curWeapon( self );
		pAIData->cDYaw++;
    }

	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		//	self->curWeapon = gstate->InventoryFindItem (self->inventory, "punch");
		//Remove the attack function and add the Dodge function
		AI_SetOkToAttackFlag( hook, TRUE );
		AI_RemoveCurrentTask(self,FALSE);
	}
}

// ----------------------------------------------------------------------------
// NSS[2/16/00]:
// Name:		FROG_Touch
// Description:So we can play a 'clank' noise when we hit things other than other monsters...
// Input:userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf 
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------

void FROG_Touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	if(other->flags & (FL_BOT|FL_CLIENT|FL_MONSTER))
	{
		return;
	}
	char Sound[32];
	int Letter_Offset;
	Letter_Offset = (int)((rnd()*3.0f)+97.0f);
	Com_sprintf(Sound,sizeof(Sound),"global/m_shieldhit%c.wav",Letter_Offset);
	if(self->hacks < gstate->time && plane && (plane->normal.x||plane->normal.y) && !self->groundEntity && self->velocity.Length() > 80.0f)
	{
		CVector Dir;
		Dir = self->velocity *0.50;
		Dir.Negate();
		self->velocity = Dir;
		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex(Sound), 1.0f, ATTN_NORM_MIN,ATTN_NORM_MAX);
		self->hacks = gstate->time + 0.5f;
	}
}


// ----------------------------------------------------------------------------
//
// Name:		FROG_ParseEpairs
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void FROG_ParseEpairs( userEntity_t *self )
{
	_ASSERTE( self );
	AI_InitNodeList( self );
	Terrain_Node_Change(self, self->movetype );
	AI_ParseEpairs(self);
	self->think = FROG_Think;
	self->nextthink = gstate->time +0.1f;
}

// ----------------------------------------------------------------------------
// Coder: NSS
// Name:		monster_froginator
// Description:spawns a frog during level load
// Input:userEntity_T *self
// Output:NA
// Note:NA
// ----------------------------------------------------------------------------
void monster_froginator( userEntity_t *self )
{
	playerHook_t *hook	= AI_InitMonster (self, TYPE_FROGINATOR);

	self->className		= "monster_froginator";
	self->netname		= tongue_monsters[T_MONSTER_FROGINATOR];

	char *szModelName = AIATTRIBUTE_GetModelName( self->className );
    _ASSERTE( szModelName );
    self->s.modelindex = gstate->ModelIndex( szModelName );

	if ( !ai_get_sequences(self) )
	{
		char *szCSVFileName = AIATTRIBUTE_GetCSVFileName( self->className );
        _ASSERTE( szCSVFileName );
        FRAMEDATA_ReadFile( szCSVFileName, self );
	}
    ai_register_sounds( self );

	hook->fnStartAttackFunc = FROG_Begin_Attack;
	hook->fnAttackFunc		= FROG_Attack;

	hook->pain_chance		= 20;
	hook->dflags			|= DFL_AMPHIBIOUS;
	
	self->movetype			= MOVETYPE_WALK;
	
    self->fragtype          |= FRAGTYPE_NOBLOOD;   // don't leave blood upon death
    self->fragtype          |= FRAGTYPE_ROBOTIC;   // apply robotic gib parts
	self->think				= FROG_ParseEpairs;
	self->touch				= FROG_Touch;
	self->nextthink			= gstate->time + 0.2;
	self->pain				= FROG_Pain;
	self->view_ofs.Set(0.0f, 0.0f, 0.0f);

	// read in attributes from .csv file
    WEAPON_ATTRIBUTE_PTR pWeaponAttributes = AIATTRIBUTE_SetInfo( self );

	//////////////////////////////////////////////
	//	give frog its melee and spit weapon		//
	//////////////////////////////////////////////
	self->inventory = gstate->InventoryNew(MEM_MALLOC);

	if ( pWeaponAttributes )
	{
		//Jaw Clamping Weapon(punch)
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
										  ITF_TRACE );
		gstate->InventoryAddItem(self, self->inventory, self->curWeapon);
		//Spit Weapon
		self->curWeapon = ai_init_weapon( self, 
										  pWeaponAttributes[1].fBaseDamage, 
										  pWeaponAttributes[1].fRandomDamage,
										  pWeaponAttributes[1].fSpreadX,
										  pWeaponAttributes[1].fSpreadZ,
										  pWeaponAttributes[1].fSpeed,
										  pWeaponAttributes[1].fDistance,
										  pWeaponAttributes[1].offset, 
										  "FrogSpit",
										  FrogSpit_Attack,
										  ITF_PROJECTILE);
		gstate->InventoryAddItem(self, self->inventory, self->curWeapon);
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
void world_ai_froginator_register_func()
{
	gstate->RegisterFunc( "FROG_Think",FROG_Think);
	gstate->RegisterFunc( "FROG_Pain",FROG_Pain);

	gstate->RegisterFunc( "FROG_ParseEpairs", FROG_ParseEpairs);
	gstate->RegisterFunc( "FROG_Begin_Attack", FROG_Begin_Attack);
	gstate->RegisterFunc( "FROG_Attack", FROG_Attack);
	gstate->RegisterFunc( "FROG_Jump", FROG_Jump);
	gstate->RegisterFunc( "FROG_Start_Jump", FROG_Start_Jump);
	gstate->RegisterFunc( "FROG_Chase", FROG_Chase);
	gstate->RegisterFunc( "FROG_Start_Chase", FROG_Start_Chase);
	gstate->RegisterFunc( "FROG_Attack_Think", FROG_Attack_Think);
	gstate->RegisterFunc( "FROG_Start_Attack_Think", FROG_Start_Attack_Think);
	gstate->RegisterFunc( "FROG_Touch", FROG_Touch);
}
