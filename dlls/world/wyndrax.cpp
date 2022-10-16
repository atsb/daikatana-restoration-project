// ==========================================================================
// NSS[12/13/99]:
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#include "world.h"
#include "ai_common.h"
#include "ai_utils.h"
//#include "ai_move.h"// SCG[1/23/00]: not used
#include "ai_weapons.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
#include "MonsterSound.h"
#include  "WyndraxandWispShare.h"


/* ***************************** define types ****************************** */

/* ***************************** Local Variables *************************** */
int HasWyndraxCharged;
/* ***************************** Local Functions *************************** */

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
userEntity_t *spawnZap(userEntity_t *owner,CVector src, userEntity_t *dst, CVector dest_origin, float killtime);
userEntity_t * findEntityFromName(char *name);
userEntity_t * spawnZapFlare(userEntity_t *self, CVector *Origin, float life, float size, char *flarename);
//static int Active_Distance;// SCG[1/23/00]: not used
/* ******************************* exports ********************************* */
DllExport void monster_wyndrax(userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////



// ----------------------------------------------------------------------------
//
// Name:WYNDRAX_Start_Find_WispMaster
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void WYNDRAX_Start_Find_WispMaster(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	//Absolutely under no circumstance should we attack!!! We need ammo!
	
	// NSS[7/14/00]:This is a cheeese hack and if you really
	// want to know why it is here then come talk to me.
	if(!HasWyndraxCharged)
	{
		self->takedamage	= DAMAGE_NO;
	}

	hook->ai_flags |= AI_IGNORE_PLAYER;
	AI_SetOkToAttackFlag( hook, FALSE );
}
// ----------------------------------------------------------------------------
//
// Name:WYNDRAX_Find_WispMaster
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void WYNDRAX_Find_WispMaster(userEntity_t *self)
{
	float Distance,Check_Dis;
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	wispMasterHook_t *wmhook;
	userEntity_t *ent = gstate->FirstEntity();
	//find a wispMaster
	//we might have to check and see how many active wisps are around.
	Distance = 10000;
	//Process through all the entities on the map
	while(ent)
	{
		//Make sure the entity is at least the right 'type' before using up a stricmp
		if(ent->movetype == MOVETYPE_HOVER)
		{
			//Mae sure the entity is the monster_wisp
			if(_stricmp(ent->className, "monster_wisp")==0)
			{
				//Check its distance and make sure it has some active wisps
				Check_Dis = VectorDistance( self->s.origin, ent->s.origin);	
				wmhook = (wispMasterHook_t *) ent->userHook;
				if( Check_Dis < Distance && wmhook->ActiveWisps > 0)
				{
					//setup our new goal entity
					self->enemy = ent;
					Distance = Check_Dis;
				}
			}
		}
		ent = gstate->NextEntity(ent);
	}
	CVector Destination;
	Destination.x = self->enemy->s.origin.x + 10;
	Destination.y = self->enemy->s.origin.y - 10;
	
	//Get the distance from the master wisp to the floor
	float floor = AI_Determine_Room_Height( self->enemy, 1000, 1); 
	//set the new destination
	Destination.z = self->enemy->s.origin.z - floor;

	if(VectorDistance( self->s.origin, Destination) < 200.0f)
	{
		AI_RemoveCurrentTask(self,FALSE);
	}
	else
	{
		AI_RemoveCurrentTask(self,TASKTYPE_MOVETOLOCATION, Destination );
		if(rnd()<0.10f)
		{
			if(rnd() > 0.50f)
				gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e3/m_wyndraxsightb.wav"), 0.70f, 400.0f, 800.0f );
			else
				gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e3/m_wyndraxsightc.wav"), 0.60f, 400.0f, 800.0f );
		}

	}
}



// ----------------------------------------------------------------------------
//
// Name:WYNDRAX_Start_Collect_Wisps
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void WYNDRAX_Start_Collect_Wisps(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	AI_Dprintf("We have made it to first wispmaster!\n");
	//Get the WispMaster's hook
	wispMasterHook_t *wmhook = (wispMasterHook_t *) self->enemy->userHook;
	//Set the NewGoal on the WispMaster
	wmhook->NewGoal = self->s.origin;
	//Set the enemy on the WispMaster
	self->enemy->enemy = self;

	//This task isn't done until it is done
	AI_SetTaskFinishTime( hook, -1 );

	//Set velocity to 0
	self->velocity.Zero();

	//Start animations
	AI_StartSequence(self, "wispb", FRAME_ONCE);
	hook->wraithorb_time = 0.0f;
	
	self->nextthink	= gstate->time + 0.1f;

	// NSS[7/14/00]:This is a cheeese hack and if you really
	// want to know why it is here then come talk to me.
	if(!HasWyndraxCharged && self->takedamage == DAMAGE_NO)
	{
		self->takedamage	= DAMAGE_YES;
		HasWyndraxCharged   = 1;
	}


}

// ----------------------------------------------------------------------------
//
// Name:WYNDRAX_Collect_Wisps
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void WYNDRAX_Collect_Wisps(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	wispMasterHook_t *wmhook = (wispMasterHook_t *) self->enemy->userHook;
	userEntity_t *wisp;	// SCG[1/23/00]: not used
	
	AI_FaceTowardPoint( self, self->enemy->s.origin );
	self->velocity.Zero();
	if(wmhook)
	{
		if(hook->acro_boost >= MAX_WISPS)
		{
			AI_Dprintf("We are charged! Let's kick some ass!\n");
			wispMasterHook_t *wmhook = (wispMasterHook_t *) self->enemy->userHook;
			self->enemy->enemy = NULL;
			wmhook->NewGoal = NULL;
			self->enemy = NULL;
			hook->ai_flags &= ~AI_IGNORE_PLAYER;
			AI_SetOkToAttackFlag( hook, TRUE );
			AI_RemoveCurrentGoal(self);
		}
		else if(wmhook->ActiveWisps <= 0)
		{
			//we have sucked this wisp cluster dry...move on to another wisp cluster!
			AI_Dprintf("Next!  Damnit Scotty we need more power!!\n");
			self->enemy->enemy = NULL;
			wmhook->NewGoal = NULL;
			self->enemy = NULL;
			//We know we had just consumed the last wisp
			hook->acro_boost++;
			AI_RemoveCurrentGoal(self);
			if(hook->acro_boost < MAX_WISPS)
			{
				AI_AddNewGoal(self,GOALTYPE_WYNDRAX_RECHARGE);
			}
			else
			{
				hook->ai_flags &= ~AI_IGNORE_PLAYER;
				AI_SetOkToAttackFlag( hook, TRUE );
			}
		}

		if( wmhook->CurrentWisp >= 11 )
		{
			AI_Dprintf("Error: wmhook->CurrentWisp >= 11\n");
			AI_Dprintf("Going into Wander mode.\n");
			AI_SetOkToAttackFlag( hook, FALSE );
			AI_RemoveCurrentGoal(self);
			AI_AddNewGoal(self,GOALTYPE_WANDER);
			return;
		}

		wisp = wmhook->FireFlies[wmhook->CurrentWisp];

		if(AI_IsEndAnimation( self ))
		{
			//Collection animations
			frameData_t *pSequence = FRAMES_GetSequence( self, "wispc" );
			AI_ForceSequence(self, pSequence,FRAME_ONCE );
		}
	}
	else
	{
		AI_Dprintf("Error:No wisps on current level.\n");
		AI_Dprintf("Going into wander mode.\n");
		AI_SetOkToAttackFlag( hook, FALSE );
		AI_RemoveCurrentGoal(self);
		AI_AddNewGoal(self,GOALTYPE_WANDER);
	}
}



// ----------------------------------------------------------------------------
// <nss>
// Name:WYNDRAX_Start_Charge_Up
// Description:Task for Wyndrax to go and Charge his little Arse up!
// Input:userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void WYNDRAX_Start_Charge_Up( userEntity_t *self )
{
	frameData_t *pSequence;
	userEntity_t *ent;
	playerHook_t *hook = AI_GetPlayerHook( self );
	//Get the Charge 
	if(self->teamchain == NULL)
		self->teamchain = findEntityFromName("WyndraxCharge");
	
	ent = self->teamchain;

	if(ent) // If we found things(i.e. lightning effect) then let's do that thing
	{
		if(VectorDistance(ent->s.origin, self->s.origin) >=500.00)
		{
				CVector Destination;
				Destination = ent->s.origin;
				Destination.z = ent->s.origin.z - (AI_Determine_Room_Height(ent, 2000.0f, 1)-24.0f);
				hook->ai_flags |= AI_IGNORE_PLAYER;
				AI_SetStateRunning(hook);
				AI_RemoveCurrentTask(self,TASKTYPE_MOVETOLOCATION, Destination );
				AI_AddNewTask(self,TASKTYPE_WYNDRAX_CHARGE_UP);
				if(rnd()<0.30f)
				{
					if(rnd() > 0.50f)
						gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e3/m_wyndraxsightb.wav"), 0.70f, 400.0f, 800.0f );
					else
						gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e3/m_wyndraxsightc.wav"), 0.60f, 400.0f, 800.0f );
				}
				return;
		}
	}
	else
	{
		AI_Dprintf("No WyndraxCharge Unit!\n");
	}
	

	
	pSequence = FRAMES_GetSequence( self, "chargea" );
	AI_ForceSequence( self, pSequence );


}

// ----------------------------------------------------------------------------
// <nss>
// Name:WYNDRAX_Charge_Up
// Description:Task that actually charges his arse up!
// Input:userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void WYNDRAX_Charge_Up(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	userEntity_t *ent;
	CVector Offset;
	ent = self->teamchain;
	if(AI_IsEndAnimation(self) )
	{
		if(hook->attack_boost <= 0)
			hook->attack_boost ++;
		
		AI_SetOkToAttackFlag( hook, TRUE );
		if(hook->ai_flags & AI_IGNORE_PLAYER)
			hook->ai_flags &= ~AI_IGNORE_PLAYER;
		AI_RemoveCurrentGoal( self );
		return;
	}
	else if (self->s.frame > 83 && self->s.frame < 87)  //Shoot Lightning from Emitter to Wyndrax
	{
		if(ent) // If we found things(i.e. lightning effect) then let's do that thing
		{
			CVector Ent_Orig = ent->s.origin;
			Offset = self->s.origin;
			Offset.z += 55.0f;
			spawnZap(self,Ent_Orig, self, Offset, 0.75f);
		
			spawnZap(self,Ent_Orig, self, Offset, 0.75f);
			spawnZapFlare(self, &Ent_Orig, 1.35f, 5.0f, "models/global/e_flblue.sp2");

		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:wyndrax_attack
// Description:Wyndrax's main attack function
// Input:userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void wyndrax_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	CVector Offset,Target_Angles,Forward;

	AI_FaceTowardPoint( self, self->enemy->s.origin );

	if(self->health > (0.50f*hook->base_health) )
	{

		AI_FaceTowardPoint( self, self->enemy->s.origin );
		//Do the lightning attack if we are at 50% or greater health
		if(hook->attack_boost > 0.0f && self->s.frame > 120 && AI_IsFacingEnemy( self, self->enemy ))//Shoot lightning code here.
		{
			Offset = self->enemy->s.origin - self->s.origin;
			
			Offset.Normalize();
			VectorToAngles(Offset,Target_Angles);

			Target_Angles.yaw -= 55;
			Target_Angles.pitch -= 65;

			Target_Angles.AngleToForwardVector(Forward);
			
			Offset = (Forward * 25)+self->s.origin;
			
			gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e3/m_wwisplightning.wav"), 0.75f, 600.0f, 800.0f );
			ai_fire_curWeapon( self );
			
			hook->attack_boost --;
			AI_SetOkToAttackFlag( hook, TRUE );
			AI_RemoveCurrentTask( self );
			return;
		}
	}
	else
	{
		//make sure we haven't spawned too many wisps
		if(hook->base_power < MAX_WISPS && hook->acro_boost > 0)
		{
			AI_PlayAttackSounds( self );
			
			if ( AI_IsReadyToAttack1( self ) && AI_IsFacingEnemy( self, self->enemy ))
			{
				ai_fire_curWeapon( self );
				hook->acro_boost--;
			}
			
			if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
			{
				//float dist = VectorDistance( self->s.origin, self->enemy->s.origin );
				//if ( !AI_IsWithinAttackDistance( self, dist ) || !com->Visible (self, self->enemy) )
				//{
					AI_RemoveCurrentTask( self );

					return;
				//}
			}
		}
		else if( hook->acro_boost <=0)
		{
			AI_AddNewGoal(self,GOALTYPE_WYNDRAX_RECHARGE);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:wyndrax_begin_attack
// Description:Start sequence for Wyndrax's two types of Attacks
// Input:userEntity_t *self
// Output:NA
// Note:This is also where Wyndrax does a lot of his thinking... 
//
// ----------------------------------------------------------------------------
void wyndrax_begin_attack( userEntity_t *self )
{
	frameData_t *pSequence;
	playerHook_t *hook = AI_GetPlayerHook( self );

	self->velocity.Zero();

	hook->attack_finished = gstate->time + 1.0;

	if(self->health > (0.50f*hook->base_health) )
	{
		AI_SetStateRunning(hook);
		//Do the lightning attack if we are at 50% or greater health
		if(hook->attack_boost > 0.0f )
		{
			pSequence = FRAMES_GetSequence( self, "charged" );
		}
		else
		{
			AI_AddNewGoal(self,GOALTYPE_WYNDRAX_POWERUP);
			return;
		}
		AI_ForceSequence( self, pSequence );
		AI_FaceTowardPoint( self, self->enemy->s.origin );
		self->curWeapon = gstate->InventoryFindItem (self->inventory, "WyndraxZap");
		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e3/m_wyndraxataka.wav"), 0.60f, 400.0f, 800.0f );
	}
	else
	{
		//We need to haul some major ass! 
		AI_SetStateRunning(hook);
		self->curWeapon = gstate->InventoryFindItem (self->inventory, "WyndraxWisp");
		if(hook->base_power < MAX_WISPS && hook->acro_boost > 0)
		{
			pSequence = FRAMES_GetSequence( self, "wispa" );
			AI_ForceSequence( self, pSequence );
			AI_FaceTowardPoint( self, self->enemy->s.origin );
		}
		else
		{
			if(hook->acro_boost <=0)
			{
				
				AI_AddNewGoal(self,GOALTYPE_WYNDRAX_RECHARGE);
			}
			else
			{
				AI_RemoveCurrentTask(self, TASKTYPE_MOVEAWAY, self->enemy);
			}
			
		}
	}
	AI_SetOkToAttackFlag( hook, FALSE );
}

// ----------------------------------------------------------------------------
// NSS[7/14/00]:CHEESE
// Name:		wyndrax_start_pain
// Description:FUCK FUCK FUCK 
// Input:userEntity_t *self, userEntity_t *attacker, float kick, int damage 
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void wyndrax_start_pain(userEntity_t *self, userEntity_t *attacker, float kick, int damage )
{
	playerHook_t *hook = (playerHook_t *) self->userHook;
	if(!HasWyndraxCharged && self->health < (0.45f*hook->base_health))
	{
		self->health = 0.45f*hook->base_health;
		return;
	}
	else
	{
		ai_generic_pain_handler( self, attacker, kick,damage,35);
	}
}


// ----------------------------------------------------------------------------
//
// Name:		monster_wyndrax
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_wyndrax( userEntity_t *self )
{
	playerHook_t *hook		= AI_InitMonster( self, TYPE_WYNDRAX );

	self->className			= "monster_wyndrax";
	self->netname			= tongue_monsters[T_MONSTER_WYNDRAX];

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

	hook->fnStartAttackFunc = wyndrax_begin_attack;
	hook->fnAttackFunc		= wyndrax_attack;

	hook->dflags			|= DFL_RANGEDATTACK;

	hook->run_speed			= 250;
	hook->walk_speed		= 120;
	hook->attack_speed		= 160;
	hook->pain_chance		= 20;

	self->think				= AI_ParseEpairs;
	self->nextthink			= gstate->time + 0.2;

	///////////////////////////////////////////////////////////////////////////
	self->inventory = gstate->InventoryNew(MEM_MALLOC);

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
										  "WyndraxWisp", 
										  WyndraxWisp_Attack, 
										  ITF_PROJECTILE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
		self->curWeapon = ai_init_weapon( self, 
										  pWeaponAttributes[0].fBaseDamage, 
										  pWeaponAttributes[0].fRandomDamage,
										  pWeaponAttributes[0].fSpreadX,
										  pWeaponAttributes[0].fSpreadZ,
										  pWeaponAttributes[0].fSpeed,
										  pWeaponAttributes[0].fDistance,
										  pWeaponAttributes[0].offset, 
										  "WyndraxZap", 
										  WyndraxZap_Attack,
										  ITF_PROJECTILE );
		gstate->InventoryAddItem( self, self->inventory, self->curWeapon );
	}

	hook->nAttackMode	= ATTACKMODE_NORMAL;
	hook->nAttackType	= ATTACK_GROUND_RANGED;
	hook->acro_boost	= 3; //Set Wyndrax with 0 wisps
	hook->attack_boost	= 1;//Set the lightning value to 0
	self->teamchain		= NULL;

	AI_DetermineMovingEnvironment( self );
	// NSS[2/12/00]:Flag which allows AI to use these things..
	hook->dflags  |= DFL_CANUSEDOOR;

	self->pain              = wyndrax_start_pain;	
	hook->pain_chance		= 5;

	gstate->LinkEntity(self);
	// NSS[3/10/00]:Setup the next think time... offset.
	AI_SetInitialThinkTime(self);

	// NSS[6/7/00]:Mark this monster as a boss
	hook->dflags |= DFL_ISBOSS;

	// NSS[7/14/00]:Cheese Hack
	HasWyndraxCharged = 0;

}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_wyndrax_register_func()
{
	gstate->RegisterFunc("wyndrax_begin_attack",wyndrax_begin_attack);
	gstate->RegisterFunc("wyndrax_attack",wyndrax_attack);
	gstate->RegisterFunc("wyndrax_start_pain",wyndrax_start_pain);
}
