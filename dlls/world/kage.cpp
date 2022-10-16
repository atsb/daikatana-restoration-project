// ==========================================================================
//
//  File:Kage
//  Contents:The End of "Times"... man I am one tired coder... phew!
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
#include "actorlist.h"

/* ***************************** define types ****************************** */
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
userEntity_t * KagespawnZapFlareRotate(userEntity_t *self, CVector *Origin, CVector Rotate,CVector Size,float life, char *flarename);
int SIDEKICK_FindClearSpotFrom( userEntity_t *self,CVector &fromPoint, CVector &clearPoint);
int AI_HasGoalInQueue( userEntity_t *self, GOALTYPE SearchGoal );
void CacheMonsterSounds(userEntity_t *self, char *className );
void SIDEKICK_EndGameHandler(void);
/* ******************************* exports ********************************* */

DllExport void monster_kage (userEntity_t *self);
// ----------------------------------------------------------------------------
// NSS[2/19/00]:
// Name:		KAGE_StartSmokeScreen
// Description:We are outta there!
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void KAGE_StartSmokeScreen(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	if(hook)
	{
		CVector Location;
		// NSS[2/19/00]:Get rid of eyes and sword FX
		RELIABLE_UNTRACK(self);
		AI_SetOkToAttackFlag( hook, FALSE );
		AI_ForceSequence(self,"atakc",FRAME_ONCE);
		AI_SetTaskFinishTime( hook, -1 );
		Location = self->s.origin;
		Location.z -= 16.0f;

		gstate->WriteByte(SVC_TEMP_ENTITY);
		gstate->WriteByte(TE_SMOKE);
		gstate->WritePosition(Location);
		gstate->WriteFloat(10);
		gstate->WriteFloat(10);
		gstate->WriteFloat(5);
		gstate->WriteFloat(50);
		gstate->MultiCast(Location,MULTICAST_PVS);

		Location.z += 16.0f;
		gstate->WriteByte(SVC_TEMP_ENTITY);
		gstate->WriteByte(TE_SMOKE);
		gstate->WritePosition(Location);
		gstate->WriteFloat(10);
		gstate->WriteFloat(10);
		gstate->WriteFloat(5);
		gstate->WriteFloat(50);
		gstate->MultiCast(Location,MULTICAST_PVS);

		Location.z += 16.0f;
		gstate->WriteByte(SVC_TEMP_ENTITY);
		gstate->WriteByte(TE_SMOKE);
		gstate->WritePosition(Location);
		gstate->WriteFloat(10);
		gstate->WriteFloat(10);
		gstate->WriteFloat(5);
		gstate->WriteFloat(50);
		gstate->MultiCast(Location,MULTICAST_PVS);

		self->s.renderfx	|= SPR_ALPHACHANNEL;
		self->svflags		|= SVF_DEADMONSTER;
		hook->ai_flags		|= AI_IGNORE_PLAYER;
		self->takedamage	= DAMAGE_NO;
		self->solid			= SOLID_NOT;
		AI_SetOkToAttackFlag( hook, FALSE );
		gstate->LinkEntity(self);
		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e4/m_kage_hide.wav"),1.0f, hook->fMinAttenuation, hook->fMaxAttenuation );

	}
	else
	{
		AI_RemoveCurrentGoal(self);
	}


}

// ----------------------------------------------------------------------------
// NSS[2/19/00]:
// Name:		KAGE_SmokeScreen
// Description:Send out the Protectors to protect us!
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void KAGE_SmokeScreen(userEntity_t *self)
{
	self->s.alpha -= 0.25f;
	if(self->s.alpha < 0.05)
	{
		self->s.alpha = 0.0f;
		gstate->WriteByte(SVC_TEMP_ENTITY);
		gstate->WriteByte(TE_SMOKE);
		gstate->WritePosition(self->s.origin);
		gstate->WriteFloat(10);
		gstate->WriteFloat(10);
		gstate->WriteFloat(5);
		gstate->WriteFloat(50);
		gstate->MultiCast(self->s.origin,MULTICAST_PVS);
		AI_RemoveCurrentTask(self,FALSE);
	}
}


// ----------------------------------------------------------------------------
// NSS[2/19/00]:
// Name:		KAGE_StartWait
// Description:Send out the Protectors to protect us!
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void KAGE_StartWait(userEntity_t *self)
{
	AIDATA_PTR pAIData;
	pAIData = AI_GetAIDATA(self);
	pAIData->fValue = gstate->time + (5.0f + (rnd()*4.0f));
}


// ----------------------------------------------------------------------------
// NSS[1/15/00]:
// Name:		KAGE_Wait
// Description:Send out the Protectors to protect us!
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void KAGE_Wait(userEntity_t *self)
{
	AIDATA_PTR pAIData;
	pAIData = AI_GetAIDATA(self);
	if(pAIData->fValue < gstate->time)
	{
		AI_RemoveCurrentTask(self,FALSE);
	}
}


// ----------------------------------------------------------------------------
// NSS[1/15/00]:
// Name:		KAGE_StartReturn
// Description:Return from the Ninja Hide!
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void KAGE_StartReturn(userEntity_t *self)
{
	CVector clearPoint;
	if(self->enemy)
	{
		if(SIDEKICK_FindClearSpotFrom( self,self->enemy->s.origin, clearPoint))
		{
			
			MAPNODE_PTR pCurrentNode = NODE_GetClosestNode(self, clearPoint);
			if(pCurrentNode)
			{
				float zDistance = VectorZDistance(self->s.origin,self->enemy->s.origin);
				if(zDistance < 64.0f)
				{
					self->s.origin = pCurrentNode->position;
					self->s.origin.z += 4.0f;
					self->solid = SOLID_BBOX;
					gstate->LinkEntity(self);
				}
			}
		}
	}
}


// ----------------------------------------------------------------------------
// NSS[1/15/00]:
// Name:		KAGE_Return
// Description:Return from the Ninja Hide!
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void KAGE_Return(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);

	if(self->solid != SOLID_BBOX)
	{
		AI_RemoveCurrentTask(self,TASKTYPE_KAGE_RETURN);
		AI_AddNewTaskAtFront(self,TASKTYPE_KAGE_WAIT);
		return;
	}
	self->s.alpha += 0.25;
	if(self->s.alpha > 1.0f)
	{
		self->s.renderfx	&= ~SPR_ALPHACHANNEL;
		self->svflags		&= ~SVF_DEADMONSTER;
		hook->ai_flags		&= ~AI_IGNORE_PLAYER;
		self->svflags		&= ~SVF_DEADMONSTER;
		self->takedamage	= DAMAGE_YES;
		AI_SetOkToAttackFlag( hook, TRUE );
		if(hook)
			hook->oxylung_time--;
		AI_RemoveCurrentGoal(self);
	}
}

// ----------------------------------------------------------------------------
// NSS[1/15/00]:
// Name:		KAGE_StartCreateProtectors
// Description:Send out the Protectors to protect us!
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void KAGE_StartCreateProtectors(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	AIDATA_PTR pAIData;
	self->curWeapon = gstate->InventoryFindItem (self->inventory, "kage bouncing blast");
	// Keep track of how many we have let loose
	pAIData = AI_GetAIDATA(self);

	if(pAIData)
	{
		pAIData->nValue = 0;
		pAIData->fValue = gstate->time;
	}
	AI_ForceSequence(self,"atake",FRAME_LOOP);
	hook->ai_flags |= AI_IGNORE_PLAYER;
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1 );
	gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e4/m_kage_spawn.wav"),1.0f, hook->fMinAttenuation, hook->fMaxAttenuation );

}


// ----------------------------------------------------------------------------
// NSS[1/15/00]:
// Name:		KAGE_CreateProtectors
// Description:
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void KAGE_CreateProtectors(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	AIDATA_PTR pAIData;
	pAIData = AI_GetAIDATA(self);
	
	if(pAIData && pAIData->nValue < 12)
	{
		if(pAIData->fValue < gstate->time )
		{
			gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e4/m_kage_spawnloop.wav"),1.0f, hook->fMinAttenuation, hook->fMaxAttenuation );
			ai_fire_curWeapon( self );
			pAIData->nValue++;
			self->hacks_int = pAIData->nValue;
			pAIData->fValue = gstate->time + 0.65f;
		}
	}
	else
	{
		AI_RemoveCurrentTask(self,FALSE);
	}
}

	
	
// ----------------------------------------------------------------------------
// NSS[1/15/00]:
// Name:		KAGE_Start_ChargeHealth
// Description:Setup the sparks of life meditation
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void KAGE_Start_ChargeHealth(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);

	AI_SetOkToAttackFlag( hook, FALSE );
	
	gstate->StartEntitySound( self, CHAN_LOOP, gstate->SoundIndex("e4/m_kage_ghost_am.wav"),1.0f, hook->fMinAttenuation, hook->fMaxAttenuation );

	AI_ForceSequence(self,"atake",FRAME_LOOP);
	
	AI_SetTaskFinishTime( hook, -1 );
	
	self->hacks = gstate->time + 1.0f;
}


// ----------------------------------------------------------------------------
// NSS[1/15/00]:
// Name:		KAGE_ChargeHealth
// Description: Charge up and do special FX until at least 'x'% of health is restored
// maximum valued depends upon skill level selected
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void KAGE_ChargeHealth(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	CVector Limit,Charge,Rate, Spacer;

	// NSS[1/15/00]:Values for different skill levels
	Limit.Set(0.25,0.50,0.75);
	Charge.Set(1.0f,5.0f,10.f);
	Rate.Set(2.0f,1.5,1.0f);
	Spacer.Set(15,7.5,4.5);

	// NSS[1/15/00]:Check to see if we have reached our max level of health
	// NSS[1/15/00]:Base all calculations off of the skill level
	if(self->health < (hook->base_health * AI_GetSkill(Limit)) )
	{
		if(self->hacks < gstate->time)
		{
			self->health	+= AI_GetSkill(Charge);
			self->hacks		= gstate->time + AI_GetSkill(Rate);
		}
	}
	else
	{
		// NSS[1/15/00]:For pacing the time between health charges
		hook->base_health = self->health;
		hook->invulnerability_time = gstate->time + AI_GetSkill(Spacer);
		hook->skill --;
		AI_SetOkToAttackFlag( hook, TRUE );
		hook->ai_flags &= ~AI_IGNORE_PLAYER;
		gstate->StartEntitySound( self, CHAN_LOOP, gstate->SoundIndex("e4/m_kage_ghost_am.wav"),0.0f,0.0f, 0.0f);	
		AI_RemoveCurrentGoal(self);
	}
}


// ----------------------------------------------------------------------------
// NSS[1/15/00]:
// Name:		kage_set_attack_seq
// Description: Setup one of 3 normal attack sequences using the daikatana as a melee weapon
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void kage_set_attack_seq( userEntity_t *self )
{
    if ( AI_IsFacingEnemy( self, self->enemy ) )
    {
	    playerHook_t *hook = AI_GetPlayerHook( self );

		self->curWeapon = gstate->InventoryFindItem (self->inventory, "kage slice");

		frameData_t *pSequence;
		switch( rand() % 3 )
		{
			case 0:
				{
					pSequence = FRAMES_GetSequence( self, "ataka" );
					hook->acro_boost = 0;
					break;
				}
			case 1:
				{
					pSequence = FRAMES_GetSequence( self, "atakb" );
					hook->acro_boost = 1;
					break;
				}
			case 2:
				{
					pSequence = FRAMES_GetSequence( self, "atakc" );
					hook->acro_boost = 2;
					break;
				}
			default:
				{
					pSequence = FRAMES_GetSequence( self, "ataka" );
					hook->acro_boost = 0;
					break;
				}
		}
		if(pSequence)
		{
			AI_ForceSequence(self, pSequence);
		}
		else
		{
			AI_Dprintf("pSequence has not been set!!!\n");
		}
    }
}

// ----------------------------------------------------------------------------
// NSS[1/15/00]:
// Name:		kage_attack
// Description: Mele Weapon Attack handler
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void kage_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	AI_ZeroVelocity( self );

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	AI_PlayAttackSounds( self );

	if ( AI_IsReadyToAttack1( self ) && AI_IsFacingEnemy( self, self->enemy ) )
	{
		ai_fire_curWeapon( self );
	}
	
	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		float dist = VectorDistance (self->s.origin, self->enemy->s.origin);
		if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible (self, self->enemy))
		{
			AI_RemoveCurrentTask( self );

			return;
		}
		else
		{
			kage_set_attack_seq (self);
		}
	}
	
	if (self->enemy && AI_IsEnemyTargetingMe(self,self->enemy)  && hook && hook->oxylung_time && hook->wraithorb_time < gstate->time)
	{
		if(rnd() < 0.30)
		{
			AI_AddNewGoal(self,GOALTYPE_KAGE_YINYANG);
		}
		hook->wraithorb_time = gstate->time + 5.0f;
	}
}

// ----------------------------------------------------------------------------
// NSS[1/15/00]:
// Name:		kage_begin_attack
// Description:Start the Melee attack sequence.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void kage_begin_attack( userEntity_t *self )
{
	if(AI_HasGoalInQueue( self,GOALTYPE_KAGE_SPARKUP ))
	{
		AI_RemoveCurrentGoal(self);
		return;
	}
	self->curWeapon = gstate->InventoryFindItem (self->inventory, "kage slice");
	kage_set_attack_seq (self);
}

int Kage_First_Encounter;

// ----------------------------------------------------------------------------
// NSS[1/9/00]:
// Name:		kage_think
// Description:Think function for KAGE
// Input:userEntity_t self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void KAGE_Think(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	// NSS[5/9/00]:Trap to restore Kage if he was saved while hiding.
	if(self->s.renderfx & SPR_ALPHACHANNEL && self->svflags & SVF_DEADMONSTER && hook->ai_flags & AI_IGNORE_PLAYER && self->s.alpha == 0.0f)
	{
		// NSS[5/9/00]:If all of these settings are true and we do not have this goal then we need to reset ourself
		if(!AI_HasGoalInQueue(self,GOALTYPE_KAGE_YINYANG))
		{
			self->s.renderfx	&= ~SPR_ALPHACHANNEL;
			self->svflags		&= ~SVF_DEADMONSTER;
			hook->ai_flags		&= ~AI_IGNORE_PLAYER;
			self->takedamage	= DAMAGE_YES;
			self->solid			= SOLID_BBOX;
			AI_SetOkToAttackFlag( hook, TRUE );
		}
	}

	if(self->enemy )
	{
		//Handle those nasty sidekicks
		if((self->enemy->flags & FL_CLIENT) && !Kage_First_Encounter)
		{
			Kage_First_Encounter = 1;
			SIDEKICK_EndGameHandler();
		}
		
		if(AI_IsVisible(self,self->enemy) && hook->fatigue==0)
		{
			//Attach the Sprite Light&Glow and fire stuff 
			trackInfo_t tinfo;

			// clear this variable
			memset(&tinfo, 0, sizeof(tinfo));

			tinfo.ent			= self;
			tinfo.srcent		= self;
			tinfo.modelindex	= gstate->ModelIndex("models/global/e_flare4+.sp2");		// sword flare
			tinfo.modelindex2	= gstate->ModelIndex("models/global/e_flare4xo.sp2");		// eye flare
			tinfo.Long1			= ART_DAIKATANA;
			tinfo.scale			= 0.2;
			tinfo.scale2		= 0.2;

			tinfo.lightColor.Set(0.65,0.65,0.65);											// fx color!
			tinfo.altpos2.Set(-1,-1,-1);													// neg. light color

			tinfo.flags |= TEF_SCALE|TEF_SCALE2|TEF_MODELINDEX|TEF_MODELINDEX2|TEF_FXFLAGS|TEF_LONG1|TEF_SRCINDEX|TEF_LIGHTCOLOR|TEF_ALTPOS2;
			tinfo.fxflags |= TEF_ARTIFACT_FX|TEF_FX_ONLY;
						
			com->trackEntity(&tinfo,MULTICAST_PVS);
			hook->fatigue = 1;

			// set up the ambient			
			self->s.sound = gstate->SoundIndex("global/we_dk_03a.wav"); 
			self->s.volume = 0.65;
		}
		else if (!(AI_IsVisible(self,self->enemy)) && hook->acro_boost == 1)
		{
			RELIABLE_UNTRACK(self);
			hook->fatigue = 0;
		}
	}
	AI_TaskThink( self);
	if(hook)
	{
		CVector Limit;
		// NSS[1/15/00]:Values for different skill levels
		Limit.Set(0.25,0.50,0.75);

		// NSS[1/15/00]:if we are low on health and we have the time let's charge up.
		if(!AI_HasGoalInQueue( self,GOALTYPE_KAGE_SPARKUP ) && (self->health < (hook->base_health*AI_GetSkill(Limit))) && hook->invulnerability_time < gstate->time && hook->skill > 0)
		{
				AI_AddNewGoal(self,GOALTYPE_KAGE_SPARKUP);
				return;
		}
	}
}

// ----------------------------------------------------------------------------
// NSS[1/9/00]:
// Name:		KAGE_ParseEpairs
// Description:Initialize Epairs for Kage
// Input:userEntity_t self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void KAGE_ParseEpairs(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_ParseEpairs(self);
	self->think = KAGE_Think;
	self->nextthink = gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
// NSS[2/18/00]:
// Name:		KAGE_Pain
// Description:Kage's Pain
// Input: userEntity_t *self, userEntity_t *attacker, float kick, int damage 
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void KAGE_Pain( userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	//special hack.. for when he is charging...
	if(AI_HasGoalInQueue( self,GOALTYPE_KAGE_SPARKUP ))
	{
		if(self->health < hook->base_health * 0.20f)
		{
			self->health = hook->base_health * 0.25f + damage;
		}
		else
		{
			self->health += damage*1.05;
		}
			
		if(hook->ambient_time < gstate->time)
		{
			CVector Scale,Rotate,Location,Dir;
			
			if(self->enemy)
			{
				Dir = self->enemy->s.origin - self->s.origin;
				Dir.Normalize();
			}
			else
			{
				Dir.Set(0,0,0);
			}
			Location = self->s.origin + (Dir * 32.0f);
			Scale.Set (5.0f,10.0f,7.5f);
			Rotate.Set(60.0f,5.0f,10.0f);
			Location.z += 18.0f;
			KagespawnZapFlareRotate(self, &Location,Rotate, Scale,0.70f,"models/global/e_flblue.sp2");	
			Location = self->s.origin + (Dir * 32.0f);
			Scale.Set (7.5f,5.0f,10.0f);
			Rotate.Set(5.0f,60.0f,10.0f);
			Location.z += 18.0f;
			KagespawnZapFlareRotate(self, &Location,Rotate, Scale,0.50f,"models/global/e_flblue.sp2");
			
			gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("e4/ykeypickup.wav"),1.0f, 900.0f, 2048.0f);
			hook->ambient_time = gstate->time + 1.0f;
		}
	}
	else
	{
		CVector Limit;
		// NSS[1/15/00]:Values for different skill levels
		Limit.Set(0.25,0.50,0.75);
		if(!AI_HasGoalInQueue( self,GOALTYPE_KAGE_SPARKUP ) && (self->health < (hook->base_health*AI_GetSkill(Limit))) && hook->invulnerability_time < gstate->time && hook->skill > 0)
		{
				AI_AddNewGoal(self,GOALTYPE_KAGE_SPARKUP);
				return;
		}		
		AI_StartPain(self,attacker,kick,damage);
	}
}
void AI_HolyShitIdied(userEntity_t *self, userEntity_t *attacker, userEntity_t *inflictor);
// ----------------------------------------------------------------------------
// NSS[3/23/00]:
// Name:		KAGE_RemoveGhosts
// Description:Kage Remove all Ghosts that are alive.
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void KAGE_RemoveGhosts(userEntity_t *self)
{
	userEntity_t *ent	= gstate->FirstEntity();
	playerHook_t *mhook;

	//Process through all the entities on the map
	while(ent)
	{
		//Make sure the entity is the right classname
		if((ent->flags & FL_MONSTER) && AI_IsAlive(ent))
		{
			//Check its distance 
			mhook = AI_GetPlayerHook(self);
			if(mhook && mhook->type == TYPE_GHOST)
			{
				AI_HolyShitIdied(ent, self, self);
				gstate->RemoveEntity(ent);
			}
		}
		ent = gstate->NextEntity(ent);
	}
}



// ----------------------------------------------------------------------------
// NSS[2/19/00]:
// Name:		KAGE_Die
// Description:Kage's Death
// Input:userEntity_t *self,userEntity_t *inflictor,userEntity_t *attacker,int damage,CVector &destPoint
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void KAGE_Die(userEntity_t *self,userEntity_t *inflictor,userEntity_t *attacker,int damage,CVector &destPoint)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	
	//special hack.. for when he is charging...
	if(AI_HasGoalInQueue( self,GOALTYPE_KAGE_SPARKUP ))
	{
		if(self->health < hook->base_health * 0.20f)
		{
			self->health = hook->base_health * 0.25f + damage;
		}
		else
		{
			self->health += damage*1.05;
		}
			
		if(hook->ambient_time < gstate->time)
		{
			CVector Scale,Rotate,Location,Dir;
			
			if(self->enemy)
			{
				Dir = self->enemy->s.origin - self->s.origin;
				Dir.Normalize();
			}
			else
			{
				Dir.Set(0,0,0);
			}
			Location = self->s.origin + (Dir * 32.0f);
			Scale.Set (5.0f,10.0f,7.5f);
			Rotate.Set(60.0f,5.0f,10.0f);
			Location.z += 18.0f;
			KagespawnZapFlareRotate(self, &Location,Rotate, Scale,0.70f,"models/global/e_flblue.sp2");	
			Location = self->s.origin + (Dir * 32.0f);
			Scale.Set (7.5f,5.0f,10.0f);
			Rotate.Set(5.0f,60.0f,10.0f);
			Location.z += 18.0f;
			KagespawnZapFlareRotate(self, &Location,Rotate, Scale,0.50f,"models/global/e_flblue.sp2");
			
			gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("e4/ykeypickup.wav"),1.0f, 900.0f, 2048.0f);
			hook->ambient_time = gstate->time + 1.0f;
		}
	}
	else// NSS[3/15/00]:If we are truly going to die then we need to remove the Daikatana from the user's inventory.
	{
		KAGE_RemoveGhosts(self);
		userEntity_t *head = alist_FirstEntity( client_list );
		if(head)
		{
			// see if the user has the daikatana armed.  Disarm it if so.
			weapon_t *weap = (weapon_t *)head->curWeapon;
			if (weap && !stricmp(weap->name,"weapon_daikatana"))
			{
				gstate->WeaponSwitch(head,1);

				// check head->winfo and see if it is a new weapon...(this verifies the switch worked)
				if (!stricmp( ((weaponInfo_t *)head->winfo)->weaponName,"weapon_daikatana"))
				{
					// the switch failed...give them a glock and select it...
					weaponInfo_t *winfo = com->FindRegisteredWeapon( "weapon_glock" );
					if (winfo)
					{
						winfo->give_func (head, winfo->initialAmmo);
						winfo->select_func(head);
					}

				}
			}

			// now pull the dk out of the player's inventory.
			userInventory_t *inv = gstate->InventoryFindItem(head->inventory, "weapon_daikatana");
			if(inv)
			{
				gstate->InventoryDeleteItem(head,head->inventory,inv);
			}
		}
		AI_StartDie(self,inflictor,attacker,damage,destPoint);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		monster_kage
// Description:
//				spawns a kageeton during level load
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_kage( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_KAGE );

	self->className			= "monster_kage";
	self->netname			= tongue_monsters[T_MONSTER_KAGE];

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

	hook->upward_vel		= 270.0;
	hook->run_speed			= 250;
	hook->walk_speed		= 100;
	hook->attack_speed		= 250;
	hook->max_jump_dist		= ai_max_jump_dist( hook->run_speed, hook->upward_vel );

	hook->acro_boost		= 0;

	

	hook->fnStartAttackFunc = kage_begin_attack;
	hook->fnAttackFunc		= kage_attack;

	self->die				= KAGE_Die;
	self->think				= KAGE_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;	// node links are set up 0.2 secs after map load
	self->pain				= KAGE_Pain;
	hook->pain_chance		= 2;

	///////////////////////////////////////////////////////////////////////////
	//	give kage a weapon
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
										  "kage slice", 
										  kage_slice, 
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
										  "kage bouncing blast", 
										  Kage_Bouncing_Blast, 
										  ITF_PROJECTILE | ITF_EXPLOSIVE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}
	
	self->curWeapon = gstate->InventoryFindItem (self->inventory, "kage slice");
	hook->nAttackType = ATTACK_GROUND_MELEE;
	hook->nAttackMode = ATTACKMODE_NORMAL;

	AI_DetermineMovingEnvironment( self );
	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;

	CVector Rate;
	
	// NSS[2/19/00]:
	Rate.Set(2,5,10);
	//How many times he can 'heal' himself.
	hook->skill = AI_GetSkill(Rate);
	
	// NSS[2/19/00]:How many smoekscreens?
	Rate.Set(2,4,8);
	hook->oxylung_time = AI_GetSkill(Rate);

	hook->envirosuit_time = 0.0f;

	gstate->LinkEntity(self);

	// NSS[2/22/00]:Finally Cache the sounds for the monsters we are going to spawn.... 
	CacheMonsterSounds(self,"monster_ghost");
	// Cache the sounds we will be using.
	gstate->SoundIndex("e4/ykeypickup.wav");
	gstate->SoundIndex("e4/m_kage_ghost_am.wav");
	gstate->SoundIndex("e4/m_kage_hide.wav");
	gstate->SoundIndex("e4/m_kage_spawn.wav");

	// NSS[5/9/00]:Hack in this first encounter thang... this is a quick hack to determine if we have seen the player yet..
	Kage_First_Encounter = 0;

	// NSS[6/7/00]:Mark this monster as a boss
	hook->dflags |= DFL_ISBOSS;


}


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_kage_register_func()
{
	gstate->RegisterFunc("kage_begin_attack",kage_begin_attack);
	gstate->RegisterFunc("KAGE_Pain",KAGE_Pain);
	gstate->RegisterFunc("kage_attack",kage_attack);
	gstate->RegisterFunc("KAGE_ParseEpairs",kage_attack);
	gstate->RegisterFunc("KAGE_Think",KAGE_Think);
	gstate->RegisterFunc("KAGE_Die",KAGE_Die);
	
}

