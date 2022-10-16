#include "world.h"
#include "ai.h"
#include "ai_utils.h"
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"
#include "ai_common.h"

DllExport	void	monster_ghost(userEntity_t *self);
float AI_Determine_Room_Height( userEntity_t *self, int Max_Mid, int type);
int AI_HasTaskInQue( userEntity_t *self, TASKTYPE SearchTask );
int AI_HasGoalInQueue( userEntity_t *self, GOALTYPE SearchGoal );
void AI_Chase( userEntity_t *self );
extern float cos_tbl[];
extern float sin_tbl[];
int AI_CanPlaySightSound(userEntity_t *self);

// ----------------------------------------------------------------------------
// NSS[2/15/00]:
// Name:		Ghost_Start_WakeUp
// Description:Wake up mr. ghost!
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void Ghost_Start_WakeUp(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	if(self->s.alpha == 0.0f && hook)
	{
		AI_SetTaskFinishTime( hook, -1 );
		AI_SetOkToAttackFlag( hook, FALSE );
		self->s.renderfx	&= ~RF_NODRAW;
		self->s.renderfx	|= SPR_ALPHACHANNEL;
		self->s.alpha		= 0.0f;
		AI_ForceSequence(self,"amba",FRAME_ONCE);
	}
	else
	{
		AI_RemoveCurrentTask(self,FALSE);
	}
}


// ----------------------------------------------------------------------------
// NSS[2/15/00]:
// Name:		Ghost_WakeUp
// Description:Once you are done...go do other things.
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void Ghost_WakeUp(userEntity_t *self)
{
	if(AI_IsEndAnimation(self))
	{
		self->s.alpha = 0.40f;
		AI_RemoveCurrentTask(self,FALSE);
	}
	else
	{
		self->s.alpha += 0.04f;
	}
}


// ----------------------------------------------------------------------------
// NSS[2/15/00]:
// Name:		Ghost_StartSpiral_Upward
// Description:Start of when the Ghost comes out of Kage
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void Ghost_StartSpiral_Upward(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	AIDATA_PTR pAIData;

	pAIData = AI_GetAIDATA(self);

	if(pAIData)
	{
		AI_ForceSequence(self,"flya",FRAME_LOOP);
		pAIData->destPoint.x	= 0.0f;						//counter for the roll..
		pAIData->destPoint.y	= 0.0f;						//counter for the yaw...
		pAIData->destPoint.z    = 0.0f;						//counter for pitch
		pAIData->cDYaw			= 5.5 + (crand()*10.5);		
		pAIData->gDYaw			= 5.5 + (crand()*10.5);	
		pAIData->nValue			= 0;
		pAIData->fValue			= gstate->time + 5.0f;
		self->nextthink	= gstate->time + 0.1f;		//
		AI_SetTaskFinishTime( hook, -1 );
		AI_SetOkToAttackFlag( hook, FALSE );
	}
	self->hacks_int = 0;
}

// ----------------------------------------------------------------------------
// NSS[2/15/00]:
// Name:		Ghost_Spiral_Upward
// Description:When the Ghost comes out of Kage
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void Ghost_Spiral_Upward(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	AIDATA_PTR pAIData;
	CVector Dir,Angles;
	CVector destPoint;
	float fDistance,zDistance,xRange;
	
	pAIData = AI_GetAIDATA(self);

	if(self->enemy)
	{
		if(self->owner)
		{
			fDistance = VectorXYDistance(self->owner->s.origin,self->enemy->s.origin);
			zDistance = VectorZDistance(self->owner->s.origin,self->enemy->s.origin);
			xRange	  = 256.0f;
		}
		else
		{
			fDistance = VectorXYDistance(self->s.origin,self->enemy->s.origin);
			zDistance = VectorZDistance(self->s.origin,self->enemy->s.origin);		
			xRange	  = 128.0f;
		}
	}
	

	if(pAIData)
	{
		AI_SetOkToAttackFlag( hook, FALSE );
		
		if(fDistance > xRange || (zDistance < 226.0f && !self->owner))
		{
			hook->run_speed			= 100.0f;
			Angles.yaw				= pAIData->destPoint.x;
			Angles.roll				= pAIData->destPoint.y;
			Angles.pitch			= -45.0f;
			self->s.angles.yaw		= Angles.yaw;
			self->s.angles.pitch	= Angles.pitch;
			self->s.angles.roll		= 0.0f;

			Angles.AngleToVectors(forward,right,up);
			
			Dir.x = forward.x;
			Dir.y = forward.y;
			fDistance = AI_Determine_Room_Height(self,1024.0f,2);
			if(fDistance > 200.0f)
			{
				Dir.z = 0.25f;
			}
			else
			{
				Dir.z = 0.0f;
			}
			
			destPoint = self->s.origin + Dir * 35.6;
			
			AI_FlyTowardPoint2( self, destPoint, 0.75 );
			
			pAIData->destPoint.x += pAIData->cDYaw;
			pAIData->destPoint.y += pAIData->gDYaw;

		}
		else
		{
			AI_SetOkToAttackFlag( hook, TRUE );
			hook->run_speed			= 325.0f;
			AI_RemoveCurrentTask(self,FALSE);
		}

	}
}

// ----------------------------------------------------------------------------
// NSS[2/18/00]:
// Name:		Ghost_Chase
// Description:Start to die... 
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void Ghost_Chase(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	AIDATA_PTR pAIData;
	
	pAIData = AI_GetAIDATA(self);
	
	if(AI_IsEndAnimation(self))
	{
		
		if(AI_CanPlaySightSound(self))
		{
			if(rnd() > 0.50f)
			{
				gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e4/m_kage_ghost_attack.wav"),0.95f,512.0f, 1024.0f);		
			}
			else
			{
				gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e4/m_kage_ghost_am.wav"),0.95f,512.0f, 1024.0f);		
			}
		}
		AI_ForceSequence(self, "flya", FRAME_LOOP);
	}

	//Can we see the enemy?
	int bEnemyVisible = AI_IsVisible( self, self->enemy );

	if ( self->enemy && bEnemyVisible )
	{

		float fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );

		//Close enough to bust their heads yet?
		if (fDistance < 128.0f)
		{
			AI_RemoveCurrentTask( self, FALSE );
			return;
		}
		else
		{
			pAIData->destPoint = self->enemy->s.origin; //Set the origin

			pAIData->destPoint.z += 8.0f;

			//Set the resolution for % change in degrees for smooth turn
			//this value works well for the Doombat
			pAIData->fValue		= 0.03f;
			AI_FlyTowardPoint2( self, pAIData->destPoint, pAIData->fValue);
			
			//Check to see if we haven't moved...we could be stuck on something.
			if(AI_FLY_Debug_Origin(self))
			{
				pAIData->nValue++;
			}

		}
		// extend the time
		AI_SetTaskFinishTime( hook,-1);
	}
	else
	{
		AI_Chase(self);
	}
}

// ----------------------------------------------------------------------------
// NSS[2/15/00]:
// Name:		Ghost_Begin_Attack
// Description:The pre-attack setup for the ghost that Garroth spawns
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void Ghost_Begin_Attack(userEntity_t *self)
{
	AI_ForceSequence(self,"ataka",FRAME_LOOP);
}

// ----------------------------------------------------------------------------
// NSS[2/15/00]:
// Name:		Ghost_Attack
// Description:The attack sequence of the ghost that Garroth spawns
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void Ghost_Attack(userEntity_t *self)
{
	if(self->enemy)
	{
		float fDistance = VectorDistance(self->s.origin,self->enemy->s.origin);
		if(AI_IsReadyToAttack1( self ))
		{
			CVector Dir;
			Dir = self->enemy->s.origin - self->s.origin;
			Dir.Normalize();
			AI_PlayAttackSounds( self );
			com->Damage( self->enemy, self, self, self->enemy->s.origin, Dir, 1+3*rnd(), DAMAGE_NONE );
		}
		if(AI_IsEndAnimation(self))
		{
			AI_RemoveCurrentTask(self,TASKTYPE_GHOST_DEATH);
		}
	}
	// NSS[3/13/00]:If we are not within range then let's not continue our attack.
	if(self->enemy && !AI_IsWithinAttackDistance(self,self->enemy))
	{
		AI_RemoveCurrentTask(self,FALSE);
	}
}


// ----------------------------------------------------------------------------
// NSS[2/18/00]:
// Name:		Ghost_Begin_Death
// Description:Start to die... 
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void Ghost_Begin_Death(userEntity_t *self)
{
	AI_ForceSequence(self,"flya",FRAME_LOOP);
}

// ----------------------------------------------------------------------------
// NSS[2/18/00]:
// Name:		Ghost_Die
// Description:The process a ghost goes through while they die... 
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void Ghost_Die(userEntity_t *self)
{
	self->s.alpha -= 0.02f;
	if(self->s.alpha < 0.10f)
	{
		gstate->RemoveEntity(self);
	}
}


// ----------------------------------------------------------------------------
// NSS[2/18/00]:
// Name:		Ghost_Pain
// Description:The pain of a ghost is next to nothing... 
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void Ghost_Pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	if(attacker->flags & FL_CLIENT)
	{
		self->health -= damage;
	}
	
	if(self->health < hook->base_health)
	{
		//mayhapes we will do something here.
	}
	else
	{
		//add ghost pain sound here....
	}
}

// ----------------------------------------------------------------------------
// NSS[2/15/00]:
// Name:		Ghost_Think
// Description: The think function for the ghost man.
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void Ghost_Think(userEntity_t *self)
{
	AI_TaskThink(self);
	
	if((self->owner && (!AI_IsAlive(self->owner) || !AI_HasGoalInQueue(self->owner,GOALTYPE_KAGE_SPARKUP)) && !AI_HasTaskInQue(self,TASKTYPE_GHOST_DEATH)) || self->health < 0.0f)
	{
		AI_AddNewTaskAtFront(self,TASKTYPE_GHOST_DEATH);
	}
}

// ----------------------------------------------------------------------------
// NSS[2/15/00]:
// Name:		Ghost_ParseEpairs
// Description:Parsing Epair values for a ghost is next to nothing.
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void Ghost_ParseEpairs(userEntity_t *self)
{
	AI_ParseEpairs(self);
	self->think		= Ghost_Think;
	self->nextthink	= gstate->time + 0.10f;
}

// ----------------------------------------------------------------------------
// NSS[2/15/00]:
// Name:		Monster_Ghost
// Description:The ghost that Garroth spawns out
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void	monster_ghost(userEntity_t *self)
{
	playerHook_t *hook = AI_InitMonster(self, TYPE_GHOST);

	self->className = "monster_ghost";
	self->netname = "monster_ghost";

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

	// Silly human, Ghosts don't gib
	self->fragtype         |= FRAGTYPE_NEVERGIB|FRAGTYPE_NOBLOOD;

	hook->fnStartAttackFunc = Ghost_Begin_Attack;
	hook->fnAttackFunc		= Ghost_Attack;

	self->pain				= Ghost_Pain;
	self->s.alpha			= 0.50;
	self->think				= Ghost_ParseEpairs;
	self->nextthink         = gstate->time + 0.2; // node links are set up 0.2 secs after map load

	///////////////////////////////////////////////////////////////////////////
	//	give garroth a weapon
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
										  "punch",
										  melee_punch,
										  ITF_TRACE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}
	hook->nAttackType	= ATTACK_AIR_SWOOP_MELEE;
	hook->nAttackMode	= ATTACKMODE_MELEE;

	self->movetype		= MOVETYPE_FLY;
	AI_DetermineMovingEnvironment( self );
	self->s.renderfx		|= (RF_NODRAW|SPR_ALPHACHANNEL);
	self->s.alpha			= 0.0f;
	self->svflags			= SVF_DEADMONSTER;
	//self->solid			= SOLID_NOT;
	gstate->LinkEntity(self);
	AI_SetOkToAttackFlag( hook, FALSE );
	gstate->SoundIndex("e4/m_kage_ghost_attack.wav");
	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

}


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_ghost_register_func()
{
	gstate->RegisterFunc("Ghost_ParseEpairs",Ghost_ParseEpairs);
	gstate->RegisterFunc("Ghost_Pain",Ghost_Pain);
	gstate->RegisterFunc("Ghost_Attack",Ghost_Attack);
	gstate->RegisterFunc("Ghost_Begin_Attack",Ghost_Begin_Attack);
	gstate->RegisterFunc("Ghost_Think",Ghost_Think);
}