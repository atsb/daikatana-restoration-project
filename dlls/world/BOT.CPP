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
#include "ai_common.h"
#include "ai_utils.h"
#include "ai_move.h"
//#include "ai_weapons.h"// SCG[1/23/00]: not used
#include "ai_frames.h"
#include "nodelist.h"
#include "bot.h"
//#include "client.h"// SCG[1/23/00]: not used
#include "actorlist.h"
#include "ai_func.h"
#include "MonsterSound.h"
//#include "chasecam.h"// SCG[1/23/00]: not used
#include "bodylist.h"
#include "triggers.h"
#include "collect.h"
#include "ai.h"
#include "items.h"
#include "Sidekick.h"
#include "SequenceMap.h"
#include "ai_think.h"

/* ***************************** define types ****************************** */

typedef	struct	botKillerInfo_s
{
	userEntity_t	*attacker;
	userEntity_t	*victim;
} botKillerInfo_t;


#define MONSTERNAME			"monster_bot"
#define BOTNAME_FILE		"botnames.dat"

/* ***************************** Local Variables *************************** */

static char m_name[MAX_QPATH];
static char szBotName[64];

static DEATHTYPE bot_deathtype = NONE;
static char szAnimation[16];
static botKillerInfo_t	killerInfo;

static int nBotCount = 0;

/* ***************************** Local Functions *************************** */

static void BOT_FreeInventory( userEntity_t *self );

int BOT_Move( userEntity_t *self );
void BOT_Think( userEntity_t *self );
void BOT_PreThink( userEntity_t *self );
void BOT_PostThink( userEntity_t *self );
void BOT_StartDie( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point );
void BOT_Pain( userEntity_t *self, userEntity_t *other, float kick, int damage );
int	BOT_Respawn( userEntity_t *self );
userEntity_t *BOT_FindEnemy( userEntity_t *self );
void BOT_PickedUpItem( userEntity_t *self, const char *szWeaponName );

static void bot_begin_attack( userEntity_t *self );
static void bot_attack( userEntity_t *self );

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* ******************************* exports ********************************* */

DllExport void monster_bot( userEntity_t *self );

///////////////////////////////////////////////////////////////////////////////////
// bot specific functions
///////////////////////////////////////////////////////////////////////////////////

__inline int IsDeathMatch()
{
	return deathmatch->value;
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_IsFightOrFlee
// Description:
// Input:
// Output:
//				TRUE	=> fight
//				FALSE	=> flee
// Note:
//
// ----------------------------------------------------------------------------
__inline int BOT_IsFightOrFlee( userEntity_t *self, userEntity_t *pEnemy )
{
	if ( self->health < 30 && pEnemy->health >= (self->health * 2.0f) )
	{
		return FALSE;
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_PickupWeapon
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int BOT_PickupWeapon( userEntity_t *self, char *szWeaponName )
{
	userEntity_t *pItem = ITEM_GetClosestWeapon( self, szWeaponName );
	if ( pItem )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_BOT_PICKUPITEM, pItem );
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_PickedUpItem
// Description:
//				whenever a bot picks up a new item, this function is called
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_PickedUpItem( userEntity_t *self, const char *szItemName )
{
    if ( _strnicmp( szItemName, "weapon", 6 ) == 0 || _strnicmp( szItemName, "ammo", 4 ) == 0 )
	{
		SIDEKICK_ChooseBestWeapon( self );
	}
	else
	if ( _stricmp( szItemName, "item_plasteel_armor" ) == 0 ||
		 _stricmp( szItemName, "item_chromatic_armor" ) == 0 )
	{
	}									   
	else
	if ( _stricmp( szItemName, "item_power_boost" ) == 0 ||
		 _stricmp( szItemName, "item_acro_boost" ) == 0 ||
		 _stricmp( szItemName, "item_attack_boost" ) == 0 ||
		 _stricmp( szItemName, "item_speed_boost" ) == 0 )
	{
	}
	else
	if ( _stricmp( szItemName, "item_vita_boost" ) == 0 )
	{
	}
	else
	if ( _strnicmp( szItemName, "item_health", 11 ) == 0 )
	{
	}
	else
	if ( _stricmp( szItemName, "item_goldensoul" ) == 0 )
	{
	}

#if 0
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( hook->pGoals );
	TASK_PTR pCurrentTask = GOAL_GetCurrentTask( pCurrentGoal );
	_ASSERTE( pCurrentTask );
	int nCurrentTaskType = TASK_GetType( pCurrentTask );
	if ( nCurrentTaskType == TASKTYPE_BOT_PICKUPITEM )
	{
		// search for the next visible item and pick that up as well
		userEntity_t *pItem = ITEM_GetNextClosestVisibleItem( self, self->s.origin );
		if ( pItem )
		{
			TASK_Set( pCurrentTask, pItem );
		}
		else
		{
			// no more visible items to pick up
			GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
			GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
			if ( GOAL_GetType( pCurrentGoal ) == GOALTYPE_BOT_PICKUPITEM )
			{
				AIDATA_PTR pAIData = GOAL_GetData( pCurrentGoal );
				userEntity_t *pItem = pAIData->pEntity;
				float fDistance = VectorXYDistance( self->s.origin, pItem->s.origin );
				if ( fDistance < 128.0f )
				{
					GOAL_Satisfied( pCurrentGoal );
				}
			}

			AI_RemoveCurrentTask( self );
		}
	}
#endif 0
}

// ----------------------------------------------------------------------------
//
// Name:		bot_choose_netname
// Description:
//	chooses a random name for a bot
//
//	grabs names from the file data\botnames.txt
//	if it exists, otherwise chooses from a list
//	of hardcoded names
//
//	maximum of 256 names will be read
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void bot_choose_netname( userEntity_t *self )
{
	FILE *f = NULL;
	int	 num_names = 0;
	char fname[256] = { "\0" };
	char bot_names[256][40];

	strcpy(fname, gstate->basedir);
	strcat(fname, BOTNAME_FILE);
//	int nFileLen = gstate->FS_Open( BOTNAME_FILE, &f );
	gstate->FS_Open( BOTNAME_FILE, &f );// SCG[1/23/00]: nFileLen not used
	if ( f )
	{
		int i = 0;
		char line[256] = { "\0" };
		while ( gstate->FS_Gets( line, 256, f ) && i < 256 )
		{
			sscanf( line, "%s", bot_names[i++] );
		}
		num_names = i;

		gstate->FS_Close(f);
	}
	else
	{
		strcpy(bot_names[0], "Zeus");
		strcpy(bot_names[1], "Ripley");
		strcpy(bot_names[2], "The Avatar");
		strcpy(bot_names[3], "The Dark One");
		strcpy(bot_names[4], "Moridin");
		strcpy(bot_names[5], "Shaidar Haran");
		strcpy(bot_names[6], "Amoeba God");
		strcpy(bot_names[7], "Lord Brutish");
		strcpy(bot_names[8], "Centrifuge");
		strcpy(bot_names[9], "Captain Kangaroo");
		strcpy(bot_names[10], "Wannabe");
		strcpy(bot_names[11], "Mr. Bill");
		strcpy(bot_names[12], "Killroy");
		strcpy(bot_names[13], "The Reaper");
		strcpy(bot_names[14], "Cornholio");
		strcpy(bot_names[15], "The Q Continuum");
		strcpy(bot_names[16], "Vehement");
		strcpy(bot_names[16], "Dopey");

		num_names = 18;
	}

	strcpy( szBotName, bot_names[(int)floor((float)num_names * rnd())] );

	self->netname = szBotName;
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_InitUserEntity
// Description:
// Input:
// Output:
// Note:
//	self->s.origin should be set before calling this function
//
// ----------------------------------------------------------------------------
void BOT_InitUserEntity( userEntity_t *self )
{
	self->className		= MONSTERNAME;
	if ( strlen(self->netname) == 0 )
	{
		bot_choose_netname(self);
	}

	//	set the size
	gstate->SetSize( self, -16, -16, -24, 16, 16, 32 );

	/////////////////////////////////////////////////////////
	//	make sure we're drawing the models
	/////////////////////////////////////////////////////////
	self->s.renderfx &= ~RF_NODRAW;

    // pull model name from userinfo 
	memset( m_name, 0, sizeof(m_name) );  // reset

	strcpy( m_name, HIRO_MODELNAME );		// default model name 

	self->s.modelindex = gstate->ModelIndex(m_name);
	self->modelName = m_name;
    
	// initialize the animation frame data
	if ( !ai_get_sequences(self) )
	{
		// initialize the animation sound data
		FRAMEDATA_ReadFile( "sounds/player/mikiko/m_mikiko.csv", self );
	}

	//	server and physics
	self->movetype		= MOVETYPE_WALK;
	self->solid			= SOLID_SLIDEBOX;
	self->clipmask		= MASK_PLAYERSOLID;
	self->flags			|= FL_BOT | FL_PUSHABLE;
	self->elasticity	= 1.0;
	self->mass			= 1.0;
	self->max_speed		= SIDEKICK_RUNSPEED;
	self->svflags		|= SVF_MONSTER;
	self->svflags		&= ~SVF_DEADMONSTER;
	self->s.effects		= EF_GIB;				// this makes server send client info to the clients //???
// SCG[12/7/99]: 	self->s.renderfx	= RF_CLUSTERBASED;		// makes this model draw with cluster based animation
	self->s.iflags		= IF_SV_SENDBBOX;		// send the real BBOX size down to client for prediction	

	self->ang_speed.Set(90, 90, 90);
	self->velocity.Zero();

	self->s.effects		= 0;
	self->s.origin.z	+= 0.125;		// make sure off ground

	//	set up skins
	self->s.skinnum		= 0;
	self->s.frame		= 0;

	self->viewheight	= 22.0;
	self->health		= ATTR_HEALTH;
	self->takedamage	= DAMAGE_AIM;
	self->deadflag		= DEAD_NO;

	//	thinks
	self->think			= BOT_Think;
	self->nextthink		= gstate->time + 0.1;
	self->prethink		= BOT_PreThink;
	self->postthink		= BOT_PostThink;
	self->pain			= BOT_Pain;
	self->die			= BOT_StartDie;

	self->goalentity	= self;
	self->enemy			= NULL;

	self->PickedUpItem	= BOT_PickedUpItem;

	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	hook->upward_vel	= 270.0;
	hook->forward_vel	= 350.0;
	hook->max_jump_dist = ai_max_jump_dist(hook->upward_vel, hook->forward_vel);

	ai_setfov( self, self->client->ps.fov );

	hook->attack_dist		= 2000.0;
	hook->run_speed			= SIDEKICK_RUNSPEED;
	hook->walk_speed		= SIDEKICK_RUNSPEED;
	hook->attack_speed		= SIDEKICK_RUNSPEED;
	hook->jump_chance		= 0;
	hook->back_away_dist	= 0;
	hook->active_distance	= 8192;

	hook->think_time	    = THINK_TIME;

	hook->fnStartAttackFunc = bot_begin_attack;
	hook->fnAttackFunc		= bot_attack;

	hook->bOkToAttack		= TRUE;
	hook->nTargetFrequency	= 1;

	hook->bInTransition		= FALSE;
	hook->nTransitionIndex	= 0;
	hook->fTransitionTime	= 0.0f;
	hook->pTransitions		= NULL;

	//	poison stuff
	hook->poison_time				= 0;
	hook->poison_next_damage_time	= 0;
	hook->poison_damage				= 0;
	hook->poison_interval			= 0;

	//	power ups
	hook->invulnerability_time	= 0;
	hook->envirosuit_time		= 0;
	hook->wraithorb_time		= 0;
	hook->oxylung_time			= 0;

	//	temporary attribute boosts
	hook->power_boost_time	= 0;
	hook->attack_boost_time = 0;
	hook->speed_boost_time	= 0;
	hook->acro_boost_time	= 0;
	hook->vita_boost_time	= 0;

	//	other timer fields
	hook->pain_finished		= 0;
	hook->drown_time		= gstate->time + 12.0;
	hook->attack_finished	= 0;
	hook->jump_time			= 0;
	hook->sound_time		= 0;
	hook->path_time			= 0;
	hook->think_time		= 0.1;
	hook->shake_time		= 0;
	hook->ambient_time		= 0;

	//	clear flags
	hook->exp_flags			= 0;

	//	Nelno:	never finished implementing this...
	hook->fatigue			= 0;

	hook->fnFindTarget	    = BOT_FindEnemy;
	hook->dflags		    = DFL_RANGEDATTACK + DFL_CANSTRAFE + DFL_EVIL;
	hook->type			    = TYPE_BOT;

	hook->fov_desired	    = 90.0;
	hook->fov_rate		    = 0.0;

	hook->strafe_time       = gstate->time;
	hook->strafe_dir        = STRAFE_LEFT;


	// amw: right now zero means don't look at anyone
	self->s.numClusters     = 0;

	// init animation info
	self->s.frameInfo.frameInc			= 1;
	self->s.frameInfo.frameTime			= FRAMETIME_FPS20;
	self->s.frameInfo.next_frameTime	= gstate->time + self->s.frameInfo.frameTime;

	if ( self->inventory )
	{
		//	client already has an inventory, so free it and set ptr to NULL
		BOT_FreeInventory(self);
	}

    self->inventory = gstate->InventoryNew(MEM_PERMANENT);

    AIATTRIBUTE_SetInfo( self );
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_InitWeapons
// Description:
// Input:
// Output:
// Note:
//	Inits all variables related to client weapons
//
// ----------------------------------------------------------------------------
void BOT_InitWeapons( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	self->curWeapon				= NULL;

	hook->fxFrameNum			= 0;
	hook->fxFrameFunc			= NULL;

	hook->last_button0			= 0;
	hook->viewmodel_start_frame = 0;
	hook->viewmodel_end_frame	= 0;
	hook->viewmodel_frame_time	= 0;

	hook->weapon_fired			= NULL;
	hook->weapon_next			= NULL;

	// clear out armor also
	self->armor_val				= 0;
	self->armor_abs				= 0.0f;
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_FreeInventory
// Description:
// Input:
// Output:
// Note:
//	Frees inventory when a player restarts (single player) or respawns (coop
//	and deathmatch)
//
// ----------------------------------------------------------------------------
static void BOT_FreeInventory( userEntity_t *self )
{
	if ( !self->inventory )
	{
		return;
	}

	gstate->InventoryFree(self->inventory);
	self->inventory = NULL;

	//	make sure there are no pointers to weapons left
	BOT_InitWeapons(self);
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_InitAttributes
// Description:
// Input:
// Output:
// Note:
//	Inits all variables related to client attributes
//
// ----------------------------------------------------------------------------
void BOT_InitAttributes( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	hook->base_health	= ATTR_HEALTH;
	hook->pain_chance	= 0;
	
	//	special attributes
	hook->power_boost	= 0;
	hook->attack_boost	= 0;
	hook->speed_boost	= 0;
	hook->acro_boost	= 0;
	hook->vita_boost	= 0;

	// base attribs
	hook->base_power	= 0;
	hook->base_attack	= 0;
	hook->base_speed	= 0;
	hook->base_acro		= 0;
	hook->base_vita		= 0;
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_FreeWeapon
// Description:
// Input:
// Output:
// Note:
//	untracks the client's weapon and removes the entity associated with it
//
// ----------------------------------------------------------------------------
void BOT_FreeWeapon( userEntity_t *self )
{
	self->curWeapon = NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_InitDefaultWeapon
// Description:
// Input:
// Output:
// Note:
//  Give client default weapon
//
// ----------------------------------------------------------------------------
void BOT_InitDefaultWeapon( userEntity_t* self )
{
	// give client a weapon if deathmatch
	_ASSERTE( self->inventory );
	
	weaponInfo_t *winfo = NULL;
	
	int nCurrentEpisode = GetCurrentEpisode();
	switch ( nCurrentEpisode )
	{
		case 1:
		{
			winfo = com->FindRegisteredWeapon( "weapon_disruptor" );
			if ( winfo )
			{
				winfo->give_func( self, winfo->initialAmmo );
			}
			break;
		}	
		case 2:
		{
			break;
		}
		case 3:
		{
			break;
		}
		case 4:
		{
			break;
		}

		default:
			break;
	}
	
	// select the weapon
	if (winfo)
	{
		winfo->select_func(self);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_InitStats
// Description:
// Input:
// Output:
// Note:
//	Inits deathmatch statistics
//
// ----------------------------------------------------------------------------
void BOT_InitStats( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	hook->killed = 0;
	hook->died = 0;
	hook->fired = 0;
	hook->hit = 0;

}

// ----------------------------------------------------------------------------
//
// Name:		BOT_FreeNodeSystem
// Description:
// Input:
// Output:
// Note:
//	frees data associated with node system
//
// ----------------------------------------------------------------------------
void BOT_FreeNodeSystem( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	hook->pNodeList = AI_DeleteNodeList( hook->pNodeList );
	
	//	by default, players should not have either of these...
	PATHLIST_KillPath(hook->pPathList);
	hook->pPathList = PATHLIST_Destroy(hook->pPathList);
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_DropInventory
// Description:
// Input:
// Output:
// Note:
//	makes client drop their weapon and ammo in deathmatch
//
// ----------------------------------------------------------------------------
void BOT_DropInventory( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	_ASSERTE( hook );

	if ( !self->inventory )
	{
		return;
	}

	weaponInfo_t *winfo = (weaponInfo_t *) self->winfo;
	//	drop the client's current weapon with current amount of ammo
	if ( self->winfo )
	{
		// no model should be drawn on/with the client model
		self->s.modelindex2 = 0;
        
		weapon_t *pWeapon = (weapon_t *) self->curWeapon;
        if ( pWeapon->ammo->count && !(winfo->flags & WFL_NODROP) )
        {
		    userEntity_t *pNewWeapon = gstate->SpawnEntity();
		    pNewWeapon->className	= winfo->weaponName;
		    pNewWeapon->netname		= winfo->weaponNetName;
		    pNewWeapon->spawnflags	= (DROPPED_ITEM|DROPPED_PLAYER_ITEM);

		    //	set origin
		    pNewWeapon->s.origin = self->s.origin;
		    //	give some velocity
		    pNewWeapon->velocity.x = rnd() * 400.0 - 200.0;
		    pNewWeapon->velocity.y = rnd() * 400.0 - 200.0;
		    pNewWeapon->velocity.z = rnd() * 250.0 + 250.0;

		    winfo->spawn_func( winfo, pNewWeapon, -1.0, NULL );

		    //	find out how much ammo player has for this weapon,
		    //	and store it in the weapon pickup item
		    pNewWeapon->hacks_int = pWeapon->ammo->count;
	    }
    }

	//	clear everything having to do with weapons
	BOT_FreeWeapon(self);
	BOT_InitWeapons(self);
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_CheckPowerUps
// Description:
//	called each frame before the physics are run
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_CheckPowerUps( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	int			 recalc = FALSE;

	// exp_flags - set for each item when picked up and cleared when the item's expire message is sent
	// items - set when an item is picked up and cleared when the item becomes inactive
	// xx_time - set when an item is picked up to number of seconds till item expires

	////////////////////////////////////////
	// poison
	////////////////////////////////////////
	if ( hook->exp_flags & EXP_POISON && hook->poison_time <= gstate->time + 3.0 )
	{
		//	poison expiration sound and message
		gstate->StartEntitySound(self, CHAN_VOICE, gstate->SoundIndex("artifacts/poisonfade.wav"), 
                                 1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
		bot_deathtype = POISON;
		//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_POISON_EXPIRE]);
		hook->exp_flags -= (hook->exp_flags & EXP_POISON);
	}
	if ( hook->items & IT_POISON )
	{
		//	do damage
		if ( hook->poison_next_damage_time <= gstate->time )
		{
			gstate->Con_Printf("poisoning\n");
			com->Damage(self, self, self, zero_vector, zero_vector, hook->poison_damage, DAMAGE_POISON | DAMAGE_NO_BLOOD);
			hook->poison_next_damage_time = gstate->time + hook->poison_interval;
		}
		//	poison expire
		if (hook->poison_time < gstate->time)
		{
			hook->items &= ~IT_POISON;
		}
	}

	////////////////////////////////////////
	// envirosuit
	////////////////////////////////////////
	if ( hook->envirosuit_time <= gstate->time + 3.0 && (hook->exp_flags & EXP_ENVIROSUIT) )
	{
		//	envirosuit expiration sound and message
		gstate->StartEntitySound(self, CHAN_VOICE, gstate->SoundIndex("artifacts/envirosuit/envirosuitfade.wav"), 
                                 1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
		//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_ENVIROSUIT_EXPIRE]);
		hook->exp_flags = hook->exp_flags - (hook->exp_flags & EXP_ENVIROSUIT);
	}
	if ( hook->items & IT_ENVIROSUIT )
	{
		//	envirosuit expires
		if (hook->envirosuit_time < gstate->time)
		{
			hook->items -= (hook->items & IT_ENVIROSUIT);
		}
	}

	////////////////////////////////////////
	// megashield
	////////////////////////////////////////
	if ( hook->invulnerability_time <= gstate->time + 3.0 && (hook->exp_flags & EXP_MEGASHIELD) )
	{
		//	megashield expiration sound and message
		//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_MEGASHIELD_EXPIRE]);
		gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("artifacts/megashield/megafade.wav"), 
                                 1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
		hook->exp_flags = hook->exp_flags - (hook->exp_flags & EXP_MEGASHIELD);
	}
	if ( hook->items & IT_MEGASHIELD )
	{
		//	megashield expires
		if (hook->invulnerability_time < gstate->time)
		{
			hook->items -= (hook->items & IT_MEGASHIELD);
		}
	}

	/////////////////////////////////////////////
	// stat boosters
	/////////////////////////////////////////////
	if ( hook->items & IT_POWERBOOST )
	{
		if (hook->power_boost_time <= gstate->time + 3.0 && hook->exp_flags & EXP_POWERBOOST)
		{
			//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_BOOST_POWER_EXPIRE]);
			hook->exp_flags -= (hook->exp_flags & (EXP_POWERBOOST));
		}

		if ( hook->power_boost_time <= gstate->time )
		{
			hook->items -= (hook->items & IT_POWERBOOST);
			hook->power_boost = 0; //hook->base_power;
			recalc = TRUE;
		}
	}
	if ( hook->items & IT_ATTACKBOOST )
	{
		if ( hook->attack_boost_time <= gstate->time + 3.0 && hook->exp_flags & EXP_ATTACKBOOST )
		{
			//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_BOOST_ATTACK_EXPIRE]);
			hook->exp_flags -= (hook->exp_flags & (EXP_ATTACKBOOST));
		}

		if ( hook->attack_boost_time <= gstate->time )
		{
			hook->items -= (hook->items & IT_ATTACKBOOST);
			hook->attack_boost = 0; //hook->base_attack;
			recalc = TRUE;
		}
	}
	if ( hook->items & IT_SPEEDBOOST )
	{
		if (hook->speed_boost_time <= gstate->time + 3.0 && hook->exp_flags & EXP_SPEEDBOOST)
		{
			//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_BOOST_SPEED_EXPIRE]);
			hook->exp_flags -= (hook->exp_flags & (EXP_SPEEDBOOST));
		}

		if ( hook->speed_boost_time <= gstate->time )
		{
			hook->items -= (hook->items & IT_SPEEDBOOST);
			hook->speed_boost = 0; //hook->base_speed;
			recalc = TRUE;
		}
	}
	if ( hook->items & IT_ACROBOOST )
	{
		if ( hook->acro_boost_time <= gstate->time + 3.0 && hook->exp_flags & EXP_ACROBOOST )
		{
			//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_BOOST_ACRO_EXPIRE]);
			hook->exp_flags -= (hook->exp_flags & (EXP_ACROBOOST));
		}

		if ( hook->acro_boost_time <= gstate->time )
		{
			hook->items -= (hook->items & IT_ACROBOOST);
			hook->acro_boost = 0; //hook->base_acro;
			recalc = TRUE;
		}
	}
	if ( hook->items & IT_VITABOOST )
	{
		if (hook->vita_boost_time <= gstate->time + 3.0 && hook->exp_flags & EXP_VITABOOST)
		{
			//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_BOOST_VITA_EXPIRE]);
			hook->exp_flags -= (hook->exp_flags & (EXP_VITABOOST));
		}

		if ( hook->vita_boost_time <= gstate->time )
		{
			hook->items -= (hook->items & IT_VITABOOST);
			hook->vita_boost = 0; //hook->base_vita;
			recalc = TRUE;
		}
	}

	if (recalc) com->CalcBoosts (self);
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_FindPathNextWanderNode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
MAPNODE_PTR BOT_FindPathNextWanderNode( userEntity_t *self, NODEHEADER_PTR pNodeHeader, MAPNODE_PTR pCurrentNode )
{
	_ASSERTE( self );
	_ASSERTE( pNodeHeader );

	if ( !pCurrentNode || pCurrentNode->nNumLinks == 0 )
	{
		return NULL;
	}

	playerHook_t *hook = AI_GetPlayerHook( self );
//	NODELIST_PTR pNodeList = hook->pNodeList;
//	_ASSERTE( pNodeList );

	// find a random node that is within a certain distance from the starting position
	int nNumLinks = 0;
	int aNodeIndices[MAX_NODE_LINKS];

	for ( int i = 0; i < pCurrentNode->nNumLinks; i++ )
	{
		MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, pCurrentNode->aLinks[i].nIndex );

		float fXYDistance = VectorXYDistance( self->s.origin, pNode->position );
		float fZDistance = VectorZDistance( self->s.origin, pNode->position );

		// choose a node that is in the field of view (~180 degrees)
		CVector vector, angles;
		vector = pNode->position - self->s.origin;
		VectorToAngles( vector, angles );
		float fAngleDiff = fabs(self->s.angles.yaw - angles.yaw);

		if ( !(AI_IsCloseDistance2( self, fXYDistance ) && fZDistance < 32.0f) &&
			 (fAngleDiff <= 90.0f || fAngleDiff >= 270.0f) )
		{
			aNodeIndices[nNumLinks] = pCurrentNode->aLinks[i].nIndex;
			nNumLinks++;
		}
	}

	MAPNODE_PTR pDestNode = NULL;
	if ( nNumLinks > 0 )
	{
   		int nTargetNodeIndex = aNodeIndices[(rand() % nNumLinks)];

		pDestNode = NODE_GetNode( pNodeHeader, nTargetNodeIndex );
		_ASSERTE( pDestNode );

		// get rid of old path
		PATHLIST_KillPath(hook->pPathList);
		PATHLIST_AddNodeToPath(hook->pPathList, pDestNode);
	}
	else
	{
		if ( pCurrentNode->nNumLinks > 0 )
		{
   			int nLinkIndex = (rand() % pCurrentNode->nNumLinks);

			pDestNode = NODE_GetNode( pNodeHeader, pCurrentNode->aLinks[nLinkIndex].nIndex );
			_ASSERTE( pDestNode );

			// get rid of old path

			PATHLIST_KillPath(hook->pPathList);
			PATHLIST_AddNodeToPath(hook->pPathList, pDestNode);
		}
	}

	return pDestNode;
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_MoveTowardPoint
// Description:
//				move straight toward a point
// Input:
// Output:
//				TRUE	=> if reached the point
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int BOT_MoveTowardPoint( userEntity_t *self, CVector &destPoint, int bMovingTowardNode )
{
	_ASSERTE( self );
	AI_ZeroVelocity( self );
	
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );
	
	AI_FaceTowardPoint( self, destPoint );

	float fSpeed = hook->run_speed;
	
	if ( !self->groundEntity && self->waterlevel < 3 )
	{
		float fDistance = VectorDistance( self->s.origin, destPoint );
		if ( AI_IsCloseDistance2( self, fDistance ) )
		{
			return TRUE;
		}

		AI_HandleOffGround( self, destPoint, fSpeed );

		return FALSE;
	}

	// we can just check the x,y distance since this function assumes that the 
	// dest point is visible from self
	float fDistance = VectorXYDistance( self->s.origin, destPoint );
	float fZDistance = VectorZDistance( self->s.origin, destPoint );
	if ( AI_IsCloseDistance2( self, fDistance ) )
	{
		if ( fZDistance < 32.0f )
		{
			return TRUE;
		}
		else
		{
			if ( destPoint.z < self->s.origin.z )
			{
				// going down a ladder or something, so just jump down
				AI_AddNewTaskAtFront( self, TASKTYPE_MOVEDOWN, destPoint );
				return FALSE;
			}
		}
	}

	if ( AI_HandleDirectPath( self, destPoint, bMovingTowardNode ) )
	{
		return FALSE;
	}

	forward = destPoint - self->s.origin;

	// should never be in water, but just in case
	int bOnFlatSurface = AI_IsOnFlatSurface( self );
	if ( self->waterlevel < 3 && bOnFlatSurface )
	{
		forward.z = 0.0f;
	}
	forward.Normalize();

	if ( AI_HandleCollisionWithEntities( self, destPoint, fSpeed ) )
	{
		return FALSE;
	}

	if ( AI_HandleGroundObstacle( self, fSpeed, bMovingTowardNode ) )
	{
		return FALSE;
	}

	//-------------------------------------------------------------------------
	// NOTE: Bots should not do ledge detection, all jumping should be done with node system
	//-------------------------------------------------------------------------

	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	self->velocity = forward * (fSpeed * fScale);

	if ( AI_HandleGettingStuck( self, destPoint, fSpeed ) )
	{
		return FALSE;
	}

	hook->last_origin = self->s.origin;

	// update the current node for this entity
	NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	node_find_cur_node( self, pNodeList );

	ai_frame_sounds( self );
	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_PreThink
// Description:
// Input:
// Output:
// Note:
//	called each frame before the physics are run
//
// ----------------------------------------------------------------------------
void BOT_PreThink( userEntity_t *self )
{
//	weapon_t		*weap = (weapon_t *) self->curWeapon;// SCG[1/23/00]: not used
	playerHook_t	*hook = AI_GetPlayerHook( self );

	client_deathtype = NONE;

    AI_ClearExecutedTasks( hook );

	/////////////////////////////////////////////////////////////////
	//	check to see if client on a warp path
	//	frame updates don't happen in a warp
	/////////////////////////////////////////////////////////////////

	if (self->flags & FL_INWARP)
	{
		return;
	}

	/////////////////////////////////////////////////////////////////
	//	animate the client's player models and weapon
	///////////////////////////////////////////////////////////////

	//	animate the input entity's models
	com->FrameUpdate (self);

	//	fire player weapons that are based on frames
	if ( hook->fxFrameFunc )
	{
		hook->fxFrameFunc(self);
		hook->fxFrameFunc = NULL;
	}

	//	drowning can kill player, so don't go on
	if (self->deadflag == DEAD_DEAD)
	{
		return;
	}

	hook->last_origin = self->s.origin;

	// determine if the player/bot has left the ground between nodes
	// used for determining when to link nodes back
	if (self->groundEntity == NULL)
	{
		hook->bOffGround = TRUE;
	}

}

// ----------------------------------------------------------------------------
//
// Name:		BOT_PostThink
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_PostThink( userEntity_t *self )
{
	if ( !self )
	{
		// this routine is fucked by garbage data, whine to the developer, bail out and try to move on
		gstate->Con_Dprintf ("BOT_PostThink():%s,%i: bad parameter 1 == NULL\n", __FILE__, __LINE__);
		return; 
	}

	// Don't branch to camera think code if hook is screwed
	playerHook_t *hook = AI_GetPlayerHook( self );
	if ( !hook )
	{
		gstate->Con_Dprintf("BOT_PostThink():%s,%i: self->userHook == NULL\n", __FILE__, __LINE__);
	} 

	if ( self->flags & FL_INWARP )
	{
		return;
	}

	if ( self->deadflag != DEAD_NO )
	{
		return;
	}

	BOT_CheckPowerUps( self );

/// moved to friction hook
//	hook->last_water_type = self->watertype;
//	hook->last_water_level = self->waterlevel;
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_RecalcLevel
// Description:
//  calculates and set the level of the player passed in
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_RecalcLevel( userEntity_t* self )
{
#if 0
	int level;
	int	attr;
	playerHook_t*	hook = (playerHook_t*)self->userHook;

	// set level
	for ( level = 1; level < MAX_LEVEL; level++ )
	{
		if ( self->record.exp < exp_level[level] )
		{
			self->record.level = level - 1;
			break;
		}
	}

	// clear stats
	hook->base_power = 0;
	hook->base_attack = 0;
	hook->base_acro = 0;
	hook->base_vita = 0;
	hook->base_speed = 0;

	// distribute points
	attr = 0;
	for ( level = 0; level < self->record.level; level++ )
	{
		switch (attr++ % 5)
		{
			case 0:
				hook->base_power++;
				break;
			case 1:
				hook->base_attack++;
				break;
			case 2:
				hook->base_acro++;
				break;
			case 3:
				hook->base_vita++;
				break;
			case 4:
				hook->base_speed++;
				break;
		}
	}

	com->CalcBoosts(self);
#endif
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_DeathMessage
// Description:
//	called from com->Damage when a player is killed
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_DeathMessage( userEntity_t *self )
{
    if ( !gstate->attacker )
    {
	    return;
	}

	// set up killer info for passing to weapon command
	killerInfo.attacker = gstate->attacker;
	killerInfo.victim = self;

	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	userEntity_t *pAttacker = gstate->attacker;
	playerHook_t *ahook = AI_GetPlayerHook( pAttacker );
	if ( !ahook )
	{
		// must have been a telegibed or something
		return;
	}

	if ( pAttacker )
	{
		if ( bot_deathtype )
		{	
            char deathmsg[80];

//            sprintf(deathmsg, "%s %s.\n",self->netname, tongue_deathmsg_self[client_deathtype - 1]);
            Com_sprintf(deathmsg, sizeof(deathmsg),"%s %s.\n",self->netname, tongue_deathmsg_self[client_deathtype - 1]);

			// got yourself killed, lose a frag
			self->record.frags--;
			hook->died++;

			// lose some of your experience too dumbass
			//self->record.exp -= 0.10 * self->record.exp;
			//BOT_RecalcLevel(self);
		}
		else 
		if ( pAttacker == self && 
			 (pAttacker->flags & FL_CLIENT || pAttacker->flags & FL_BOT) )
		{
			// lose a frag, stooopid
			self->record.frags--;
			hook->died++;
			// lose some of your experience too dumbass
			//self->record.exp -= 0.10 * self->record.exp;
			//BOT_RecalcLevel(self);

			if ( ahook->weapon_fired )
			{
				if ( ahook->weapon_fired->command )
				{
					ahook->weapon_fired->command( ahook->weapon_fired, "suicide_msg", &killerInfo );
				}
			}
		}
		else 
		if ( pAttacker != self && 
			 (pAttacker->flags & FL_CLIENT || pAttacker->flags & FL_BOT) )
		{
			//pAttacker->record.exp += (0.10 * self->record.exp) + 10;
			pAttacker->record.frags++;
			
			hook->died++;
			ahook->killed++;

			// lose exp for dying
			//self->record.exp -= 0.10 * self->record.exp;
			//BOT_RecalcLevel(self);
			//BOT_RecalcLevel(pAttacker);

			if ( ahook->weapon_fired )
			{
				if ( ahook->weapon_fired->command )
				{
					ahook->weapon_fired->command( ahook->weapon_fired, "death_msg", &killerInfo );
				}
			}
		}
		else 
		if ( pAttacker->owner == self && pAttacker->owner && 
			 (pAttacker->owner->flags & FL_CLIENT || pAttacker->owner->flags & FL_BOT) )
		{
			// lose a frag, stooopid
			self->record.frags--;
			hook->died++;

			// lose some of your experience too dumbass
			//self->record.exp -= 0.10 * self->record.exp;
			//BOT_RecalcLevel(self);

			char msg[80];
			Com_sprintf(msg,sizeof(msg),"%s sucked down C4.\n", self->netname);
//			sprintf(msg, "%s sucked down C4.\n", self->netname); 
			gstate->bprint(msg);
		}
		else 
		if ( pAttacker->owner != self && pAttacker->owner && 
			 (pAttacker->owner->flags & FL_CLIENT || pAttacker->owner->flags & FL_BOT) )
		{
			// reset ahook
			ahook = AI_GetPlayerHook( pAttacker->owner );
			
			//pAttacker->owner->record.exp += (0.10 * self->record.exp) + 10;
			pAttacker->owner->record.frags++;
			
			hook->died++;
			ahook->killed++;

			// lose exp for dying
			//self->record.exp -= 0.10 * self->record.exp;
			//BOT_RecalcLevel(self);
			//BOT_RecalcLevel(pAttacker->owner);
			
			char msg[80];
//			sprintf(msg, "%s was blown away by %s.\n", self->netname, pAttacker->owner->netname);
			Com_sprintf(msg, sizeof(msg),"%s was blown away by %s.\n", self->netname, pAttacker->owner->netname);
			gstate->bprint(msg);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_Die
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_Die( userEntity_t *self )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
//	_ASSERTE( hook  );// SCG[1/23/00]: not used

	ai_frame_sounds( self );

	if ( gstate->time > self->delay )
	{
		int bReadyToRespawn = FALSE;
		if ( self->deadflag == DEAD_DYING )
		{
			if ( self->s.frameInfo.frameState & FRSTATE_LAST ||
				 self->s.frameInfo.frameState & FRSTATE_STOPPED )
			{
				bReadyToRespawn = TRUE;
			}
		}
		else
		if ( self->deadflag == DEAD_DEAD )
		{
			bReadyToRespawn = TRUE;
		}

		if ( bReadyToRespawn ) 
		{
			self->deadflag = DEAD_DEAD;
			self->die		= NULL;
			self->prethink	= NULL;
			self->postthink = NULL;
			self->think		= NULL;

			gstate->LinkEntity( self );

			AI_Dprintf( "Finished TASKTYPE_BOT_DIE.\n" );

			BOT_Respawn( self );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_StartDie
// Description:
//	called from com->Damage when a player is killed
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_StartDie( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	if ( self->deadflag == DEAD_NO )
	{
		BOT_DeathMessage (self);
		BOT_DropInventory (self);
	}

	//	do not clip any longer!
	self->solid			= SOLID_NOT;
	self->groundEntity	= NULL;
	self->movetype		= MOVETYPE_BOUNCE;
	self->svflags		|= SVF_DEADMONSTER;	//&&& AMW 7/17/98 - as per quake2
    self->armor_val		= 0;
    self->armor_abs		= 0;

	// lock the angles of the entity so he can't rotate or spin
	self->s.angles.x	= 0.0f;
	self->s.angles.z	= 0.0f;

	if ( self->deadflag == DEAD_NO )
	{
		//	player was just killed
		if ( self->waterlevel > 2 )
        {
		    SIDEKICK_PlaySound(self, "waterdeath.wav");
			// gstate->sound (self, CHAN_VOICE, "player/playerwaterdeath.wav", 255, ATTN_NORM);
		}
		else 
		if ( self->health < -40 )
        {
		    SIDEKICK_PlaySound(self, "udeath.wav");
			// gstate->sound (self, CHAN_VOICE, "player/udeath.wav", 255, ATTN_NORM);
		}
		else
		{
			float r = int(rnd () * 4) + 1;
			if (r == 1)
            {
			    SIDEKICK_PlaySound(self, "death1.wav");
				// gstate->sound (self, CHAN_VOICE, "player/playerdeath1.wav", 255, ATTN_NORM);
			}
			else 
			if (r == 2)
            {
			    SIDEKICK_PlaySound(self, "death2.wav");
				// gstate->sound (self, CHAN_VOICE, "player/playerdeath2.wav", 255, ATTN_NORM);
			}
			else 
			if (r == 3)
            {
			    SIDEKICK_PlaySound(self, "death3.wav");
				// gstate->sound (self, CHAN_VOICE, "player/playerdeath3.wav", 255, ATTN_NORM);
			}
			else 
			if (r == 4)
            {
			    SIDEKICK_PlaySound(self, "death4.wav");
				// gstate->sound (self, CHAN_VOICE, "player/playerdeath4.wav", 255, ATTN_NORM);
			}
		}

		if ( damage >= PLAYER_GIB_POINT && sv_violence->value == 0 )
		{
			/////////////////////////////////////////////////////////
			//	make sure we're not drawing the player model
			/////////////////////////////////////////////////////////
			self->s.renderfx |= RF_NODRAW;
            // TTD: make a global gib function .. .ai_throw_gib is everywhere.  dsn

			AI_StartGibFest(self , inflictor);
			self->deadflag = DEAD_DEAD;
		}
		else
		{
			AI_ForceSequence(self, "diea");
		
			self->deadflag = DEAD_DYING;
		}

		// die task need to be inserted since the die task gets triggered from outside 
		// the AI framework
		AI_AddNewTaskAtFront( self, TASKTYPE_BOT_DIE );

		if ( IsDeathMatch() )
		{
			//	don't allow restart for a bit
			self->delay = gstate->time + 1.0f;
		}
		else
		{
			//	don't allow restart for a few seconds
			self->delay = gstate->time + 3.0f;
		}

		self->client->latched_buttons = 0;
	}
	else 
	if ( self->deadflag == DEAD_DYING )
	{
		if (damage >= BODY_GIB_POINT)
		{
			AI_StartGibFest(self, inflictor);
			self->deadflag = DEAD_DEAD;
		}
		else
		{
			frameData_t *pSequence = FRAMES_GetSequence( self, "diea" );
			AI_StartSequence(self, pSequence);

			self->deadflag = DEAD_DEAD;
		}
	}

	self->nextthink	= gstate->time + 0.1f;

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 5.0f );
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_Pain
// Description:
//	initializes the client data structures
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_Pain( userEntity_t *self, userEntity_t *other, float kick, int damage)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	if ( hook->pain_finished > gstate->time )
	{
		return;
	}

	int r = (int)(floor(rnd() * 4) + 1);
	switch ( r )
    {
        case 1:
	    {
            SIDEKICK_PlaySound(self, "pain1.wav");
		    // gstate->sound (self, CHAN_VOICE, "player/playerpain1.wav", 255.0, ATTN_NORM);
		    hook->pain_finished = gstate->time + 1.0;
            break;
	    }
        case 2:
	    {
            SIDEKICK_PlaySound(self, "pain2.wav");
		    // gstate->sound (self, CHAN_VOICE, "player/playerpain2.wav", 255.0, ATTN_NORM);
		    hook->pain_finished = gstate->time + 1.0;
            break;
	    }
        case 3:
	    {
            SIDEKICK_PlaySound(self, "pain3.wav");
		    // gstate->sound (self, CHAN_VOICE, "player/playerpain3.wav", 255.0, ATTN_NORM);
		    hook->pain_finished = gstate->time + 1.0;
            break;
	    }
        default:
	    {
            SIDEKICK_PlaySound(self, "pain4.wav");
		    // gstate->sound (self, CHAN_VOICE, "player/playerpain4.wav", 255.0, ATTN_NORM);
		    hook->pain_finished = gstate->time + 1.0;
            break;
	    }
    }
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_StartRoam
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_StartRoam( userEntity_t *self )
{
 	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	// before starting to roam, see if this bot needs anything
	userEntity_t *pItem = AI_DetermineNeededItem( self );
	if ( pItem )
	{
		AI_AddNewGoal( self, GOALTYPE_BOT_PICKUPITEM, pItem );
		return;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_BOT_ROAM );

	AI_SetStateRunning(hook);
	if ( AI_StartMove(self) == FALSE )
	{
		return;
	}

	NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	MAPNODE_PTR pCurrentNode = NODE_GetClosestNode(self);
	if ( !pCurrentNode && pNodeList->nCurrentNodeIndex != -1 )
	{
		pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
	}

	MAPNODE_PTR pNode = BOT_FindPathNextWanderNode( self, pNodeHeader, pCurrentNode );
	if ( !pNode )
	{
		AI_Dprintf( "Did not find the next wander node!!!!.\n" );
		AI_AddNewGoal( self, GOALTYPE_IDLE );
		return;
	}

	AI_Dprintf( "Starting TASKTYPE_BOT_ROAM.\n" );

	self->nextthink	= gstate->time + 0.1f;

	hook->nMoveCounter = 0;	// used to keep track of how long the bot is in a roam task

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, -1.0f );
}	

// ----------------------------------------------------------------------------
//
// Name:		BOT_Roam
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_Roam( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

#define _PICKUP_ITEMS_
#ifdef _PICKUP_ITEMS_
	hook->nMoveCounter++;
	if ( hook->nMoveCounter % 10 == 0 )
	{
		if ( hook->nMoveCounter % 50 == 0 )
		{
			// roamed around for long enough, search for good items or an enemy to attack
			userEntity_t *pItem = AI_DetermineWantItem( self );
			if ( pItem )
			{
				AI_AddNewGoal( self, GOALTYPE_BOT_PICKUPITEM, pItem );
				return;
			}			
		}
		else
		{
			// search for nearby visible items
			userEntity_t *pItem = ITEM_GetNextClosestVisibleItem( self, self->s.origin );
			if ( pItem )
			{
				AI_AddNewGoal( self, GOALTYPE_BOT_PICKUPITEM, pItem );
				return;
			}
		}
	}
#endif _PICKUP_ITEMS_

	if ( hook->pPathList->pPath )
	{
		if ( !AI_HandleUse( self )  )
		{
			NODELIST_PTR pNodeList = hook->pNodeList;
			_ASSERTE( pNodeList );
			NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
			_ASSERTE( pNodeHeader );

			MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, hook->pPathList->pPath->nNodeIndex );
			if ( BOT_MoveTowardPoint( self, pNode->position, TRUE ) == TRUE )
			{
				// we're already at this node, so delete the first one
				PATHLIST_DeleteFirstInPath( hook->pPathList );

				MAPNODE_PTR pNextNode = BOT_FindPathNextWanderNode( self, pNodeHeader, pNode );
				if ( !pNextNode )
				{
					AI_Dprintf( "Did not find the next wander node!!!!.\n" );
					AI_AddNewGoal( self, GOALTYPE_IDLE );

					return;
				}
				if ( AI_HandleUse( self, pNodeHeader, pNode, pNextNode ) )
				{
					return;
				}

				BOT_Roam( self );
			}			
		}
	}
	else
	{
		NODELIST_PTR pNodeList = hook->pNodeList;
		_ASSERTE( pNodeList );
		NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
		_ASSERTE( pNodeHeader );

		MAPNODE_PTR pCurrentNode = NODE_GetClosestNode(self);
		if ( !pCurrentNode && pNodeList->nCurrentNodeIndex != -1 )
		{
			pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
		}

		MAPNODE_PTR pNextNode = BOT_FindPathNextWanderNode( self, pNodeHeader, pCurrentNode );
		if ( !pNextNode )
		{
			AI_Dprintf( "Did not find the next wander node!!!!.\n" );
			AI_AddNewGoal( self, GOALTYPE_IDLE );

			return;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_StartPickupItem
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_StartPickupItem( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_BOT_PICKUPITEM );

	AI_SetStateRunning(hook);
	if ( AI_StartMove(self) == FALSE )
	{
		return;
	}

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	_ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	userEntity_t *pItem = pAIData->pEntity;
	_ASSERTE( pItem );

	// compute the time that may take to get there
	float fDistance = 0;
	if ( AI_FindPathToPoint( self, pItem->s.origin ) )
	{
		CVector currentPos = self->s.origin;

		NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( self );
		PATHNODE_PTR pPath = hook->pPathList->pPath;
		while ( pPath )
		{
			MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, pPath->nNodeIndex );
			fDistance += VectorXYDistance( currentPos, pNode->position );
			
			currentPos.Set( pNode->position );

			pPath = pPath->next_node;
		}
	}

	AI_Dprintf( "Starting TASKTYPE_BOT_PICKUPITEM.\n" );

	self->nextthink	= gstate->time + 0.1f;

	AI_SetOkToAttackFlag( hook, TRUE );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 5.0f;
	AI_SetTaskFinishTime( hook, fTime );
}	

// ----------------------------------------------------------------------------
//
// Name:		BOT_PickupItem
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_PickupItem( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	userEntity_t *pItem = pAIData->pEntity;
	_ASSERTE( pItem );
	
	float fDistance = VectorXYDistance( self->s.origin, pItem->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pItem->s.origin );
	if ( (AI_IsCloseDistance2( self, fDistance ) && fZDistance < 32.0f) ||
		 (AI_IsLineOfSight( self, pItem ) && !ITEM_IsVisible( pItem ))
	   )
	{
		// move a little further to make sure the item is picked up
		forward = pItem->s.origin - self->s.origin;
		forward.z = 0.0f;
		forward.Normalize();
		
		float fSpeed = hook->run_speed;
		float fScale = FRAMES_ComputeFrameScale( hook );
		self->velocity = forward * (fSpeed * fScale);

		// search for the next visible item and pick that up as well
		userEntity_t *pNewItem = ITEM_GetNextClosestVisibleItem( self, pItem->s.origin );
		if ( pNewItem )
		{
			TASK_Set( pCurrentTask, pNewItem );

			// compute the time that may take to get there
			float fDistance = 0;
			if ( AI_FindPathToPoint( self, pNewItem->s.origin ) )
			{
				CVector currentPos = self->s.origin;

				NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( self );
				PATHNODE_PTR pPath = hook->pPathList->pPath;
				while ( pPath )
				{
					MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, pPath->nNodeIndex );
					fDistance += VectorXYDistance( currentPos, pNode->position );
					
					currentPos = pNode->position;

					pPath = pPath->next_node;
				}
			}
			float fSpeed = AI_ComputeMovingSpeed( hook );
			float fTime = (fDistance / fSpeed) + 5.0f;
			AI_SetTaskFinishTime( hook, fTime );
		}
		else
		{
			// no more visible items to pick up
			GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
			if ( GOAL_GetType( pCurrentGoal ) == GOALTYPE_BOT_PICKUPITEM )
			{
//				AIDATA_PTR pAIData = GOAL_GetData( pCurrentGoal );
				GOAL_GetData( pCurrentGoal );// SCG[1/23/00]: pAIData not used
				float fDistance = VectorXYDistance( self->s.origin, pItem->s.origin );
				if ( fDistance < 128.0f )
				{
					GOAL_Satisfied( pCurrentGoal );
				}
			}

			AI_RemoveCurrentTask( self );
		}

		return;
	}

	int bMoved = FALSE;
	if ( hook->pPathList->nPathLength == 0 )
	{
		tr = gstate->TraceLine_q2( self->s.origin, pItem->s.origin, self, MASK_SOLID );
		if ( (tr.fraction >= 1.0f && fZDistance < MAX_JUMP_HEIGHT) || 
			 (tr.fraction >= 0.8f && ((1.2f-tr.fraction) * (fDistance+fZDistance)) < 32.0f) )
		{
			BOT_MoveTowardPoint( self, pItem->s.origin, FALSE );
			bMoved = TRUE;
		}
	}

	if ( !bMoved )
	{
		if ( hook->pPathList->pPath )
		{
			if ( !AI_HandleUse( self ) )
			{
				if ( BOT_Move( self ) == FALSE )
				{
					if ( AI_CanPath( hook ) && !AI_FindPathToPoint( self, pItem->s.origin ) )
					{
						AI_RestartCurrentGoal( self );
					}
				}					
			}
		}
		else
		{
			if ( AI_CanPath( hook ) && !AI_FindPathToPoint( self, pItem->s.origin ) )
			{
				AI_RestartCurrentGoal( self );
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_StartStrafeLeft
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_StartStrafeLeft( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) != TASKTYPE_BOT_STRAFELEFT )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_BOT_STRAFELEFT );
		return;
	}

	CVector sideStepPoint;
	int bSuccess = AI_ComputeChaseSideStepPoint( self, self->enemy, LEFT, sideStepPoint );
	if ( bSuccess )
	{
		AI_SetStateRunning( hook );
		if ( AI_StartMove( self ) == FALSE )
		{
			return;
		}

		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
		TASK_Set( pCurrentTask, sideStepPoint );

		AI_Dprintf( "Starting TASKTYPE_BOT_STRAFELEFT.\n" );

		AI_SetOkToAttackFlag( hook, FALSE );
		AI_SetTaskFinishTime( hook, 1.0f );

		self->nextthink	= gstate->time + 0.1f;
	}
	else
	{
		AI_RemoveCurrentTask( self );
		AI_AddNewTaskAtFront( self, TASKTYPE_BOT_CHARGETOWARDENEMY );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_StrafeLeft
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_StrafeLeft( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	int bEnemyVisible = FALSE;
	bEnemyVisible = AI_IsCompletelyVisible( self, self->enemy );
	if ( bEnemyVisible == COMPLETELY_VISIBLE )
	{
		bEnemyVisible = TRUE;
	}
	else
	{
		bEnemyVisible = FALSE;
	}

	float fDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	if ( bEnemyVisible || AI_IsCloseDistance2( self, fDistance ) )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	BOT_MoveTowardPoint( self, pAIData->destPoint, FALSE );
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_StartStrafeRight
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_StartStrafeRight( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) != TASKTYPE_BOT_STRAFERIGHT )
	{
		AI_AddNewTaskAtFront( self, TASKTYPE_BOT_STRAFERIGHT );
		return;
	}

	CVector sideStepPoint;
	int bSuccess = AI_ComputeChaseSideStepPoint( self, self->enemy, RIGHT, sideStepPoint );
	if ( bSuccess )
	{
		AI_SetStateRunning( hook );
		if ( AI_StartMove( self ) == FALSE )
		{
			return;
		}

		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
		TASK_Set( pCurrentTask, sideStepPoint );

		AI_Dprintf( "Starting TASKTYPE_BOT_STRAFERIGHT.\n" );

		AI_SetOkToAttackFlag( hook, FALSE );
		AI_SetTaskFinishTime( hook, 1.0f );

		self->nextthink	= gstate->time + 0.1f;
	}
	else
	{
		AI_RemoveCurrentTask( self );
		AI_AddNewTaskAtFront( self, TASKTYPE_BOT_CHARGETOWARDENEMY );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_StrafeRight
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_StrafeRight( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	int bEnemyVisible = FALSE;
	bEnemyVisible = AI_IsCompletelyVisible( self, self->enemy );
	if ( bEnemyVisible == COMPLETELY_VISIBLE )
	{
		bEnemyVisible = TRUE;
	}
	else
	{
		bEnemyVisible = FALSE;
	}

	float fDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	if ( bEnemyVisible || AI_IsCloseDistance2( self , fDistance ) )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	BOT_MoveTowardPoint( self, pAIData->destPoint, FALSE );
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_StartChargeTowardEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_StartChargeTowardEnemy( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *pEnemy = self->enemy;
	_ASSERTE( pEnemy );

	if ( AI_StartMove( self ) == FALSE )
	{
		return;
	}

	float fDistance = VectorXYDistance( self->s.origin, pEnemy->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pEnemy->s.origin );
	tr = gstate->TraceLine_q2( self->s.origin, pEnemy->s.origin, self, MASK_SOLID );
	if ( (tr.fraction >= 1.0f && fZDistance < MAX_JUMP_HEIGHT) || 
		 (tr.fraction >= 0.8f && ((1.2f-tr.fraction) * (fDistance+fZDistance)) < 32.0f) )
	{
		BOT_MoveTowardPoint( self, pEnemy->s.origin, FALSE );
	}
	else
	{
		if ( !AI_FindPathToPoint( self, pEnemy->s.origin ) )
		{
			AI_RestartCurrentGoal( self );
		}
	}

	AI_Dprintf( "Starting TASKTYPE_BOT_CHARGETOWARDENEMY.\n" );

	self->nextthink	= gstate->time + 0.1f;

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, 1.0f );		// charge for a second only
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_ChargeTowardEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_ChargeTowardEnemy( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *pEnemy = self->enemy;
	_ASSERTE( pEnemy );

	float fDistance = VectorXYDistance( self->s.origin, pEnemy->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pEnemy->s.origin );
	if ( fDistance < 96.0f && fZDistance < 48.0f )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	tr = gstate->TraceLine_q2( self->s.origin, pEnemy->s.origin, self, MASK_SOLID );
	if ( (tr.fraction >= 1.0f && fZDistance < MAX_JUMP_HEIGHT) || 
		 (tr.fraction >= 0.8f && ((1.2f-tr.fraction) * (fDistance+fZDistance)) < 32.0f) )
	{
		BOT_MoveTowardPoint( self, pEnemy->s.origin, FALSE );
	}
	else
	{
		if ( hook->pPathList->pPath )
		{
			if ( !AI_HandleUse( self ) )
			{
				if ( BOT_Move( self ) == FALSE )
				{
					if ( AI_CanPath( hook ) && !AI_FindPathToPoint( self, pEnemy->s.origin ) )
					{
						AI_RestartCurrentGoal( self );
					}
				}					
			}
		}
		else
		{
			if ( AI_CanPath( hook ) && !AI_FindPathToPoint( self, pEnemy->s.origin ) )
			{
				AI_RestartCurrentGoal( self );
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_StartChaseAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_StartChaseAttack( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_BOT_CHASEATTACK );

	AI_SetStateRunning(hook);
	if ( AI_StartMove(self) == FALSE )
	{
		return;
	}

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	_ASSERTE( pCurrentTask );

	_ASSERTE( self->enemy );
	// compute the time that may take to get there
	float fDistance = 0;
	if ( AI_FindPathToEntity( self, self->enemy ) )
	{
		CVector currentPos = self->s.origin;

		NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( self );
		PATHNODE_PTR pPath = hook->pPathList->pPath;
		while ( pPath )
		{
			MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, pPath->nNodeIndex );
			fDistance += VectorXYDistance( currentPos, pNode->position );
			
			currentPos.Set( pNode->position );

			pPath = pPath->next_node;
		}
	}

	AI_Dprintf( "Starting TASKTYPE_BOT_CHASEATTACK.\n" );

	self->nextthink	= gstate->time + 0.1f;

	AI_SetOkToAttackFlag( hook, TRUE );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 5.0f;
	AI_SetTaskFinishTime( hook, fTime );
}	

// ----------------------------------------------------------------------------
//
// Name:		BOT_ChaseAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_ChaseAttack( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	_ASSERTE( self->enemy );

	int bEnemyVisible = AI_IsCompletelyVisible( self, self->enemy );
	if ( bEnemyVisible == COMPLETELY_VISIBLE )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	_ASSERTE( hook->pPathList->pPath );

	if ( !AI_HandleUse( self ) )
	{
		// test to see if the last path point is close to the enemy's position
		if ( !AI_IsPathToEntityClose( self, self->enemy ) )
		{
			if ( AI_CanPath( hook ) && !AI_FindPathToEntity( self, self->enemy ) )
			{
				// do not try another path for 5 seconds
				AI_SetNextPathTime( hook, 5.0f );

				//	no path was found, so give up
				AI_RemoveCurrentGoal( self );

				return;
			}
		}

		if ( BOT_Move( self ) == FALSE )
		{
			PATHLIST_KillPath( hook->pPathList );
		}
		
		hook->nMoveCounter++;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_StartEngageEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_StartEngageEnemy( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	hook->nSpecificAttackMode = 0;

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if ( GOALSTACK_GetCurrentTaskType( pGoalStack ) != TASKTYPE_BOT_ENGAGEENEMY )
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		AIDATA_PTR pAIData = GOAL_GetData(pCurrentGoal);
		AI_AddNewTaskAtFront( self, TASKTYPE_BOT_ENGAGEENEMY, pAIData->pEntity );
		return;
	}

	if ( !self->enemy )
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		AIDATA_PTR pAIData = GOAL_GetData(pCurrentGoal);
		self->enemy = pAIData->pEntity;
	}

	AI_Dprintf( "Starting TASKTYPE_BOT_ENGAGEENEMY.\n" );

	AI_SetStateAttacking( hook );
	if ( hook->fnStartAttackFunc )
	{
		hook->fnStartAttackFunc( self );
	}

	self->nextthink	= gstate->time + 0.1f;

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 30.0f );
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_EngageEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_EngageEnemy( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	int bEnemyVisible = AI_IsCompletelyVisible( self, self->enemy );
	if ( bEnemyVisible == LEFTSIDE_VISIBLE )
	{
		// side step right
		AI_AddNewTaskAtFront( self, TASKTYPE_BOT_STRAFELEFT );
		return;
	}
	else
	if ( bEnemyVisible == RIGHTSIDE_VISIBLE )
	{
		// side step left
		AI_AddNewTaskAtFront( self, TASKTYPE_BOT_STRAFERIGHT );
		return;
	}

	if ( bEnemyVisible )
	{
		if ( hook->fnAttackFunc )
		{
			hook->fnAttackFunc( self );
		}

		// extend the time
		AI_SetTaskFinishTime( hook, 10.0f );
	}
	else
	{
		
		// enemy not visible
		AI_RemoveCurrentTask( self );
		return;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		bot_set_attack_seq
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void bot_set_attack_seq( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	AI_SelectRunningAnimation( self, szAnimation );
	frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
	AI_ForceSequence(self, pSequence, FRAME_LOOP);
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_ModifyStrafeDirectionAfterCollision
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void BOT_ModifyStrafeDirectionAfterCollision( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	switch ( hook->strafe_dir )
	{
		case STRAFE_LEFT:
		{
			hook->strafe_dir = STRAFE_RIGHT;
			break;
		}
		case STRAFE_RIGHT:
		{
			hook->strafe_dir = STRAFE_LEFT;
			break;
		}
		case STRAFE_UPLEFT:
		{
			hook->strafe_dir = STRAFE_DOWNRIGHT;
			break;
		}
		case STRAFE_UPRIGHT:
		{
			hook->strafe_dir = STRAFE_DOWNLEFT;
			break;
		}
		case STRAFE_DOWNLEFT:
		{
			hook->strafe_dir = STRAFE_UPRIGHT;
			break;
		}
		case STRAFE_DOWNRIGHT:
		{
			hook->strafe_dir = STRAFE_UPLEFT;
			break;
		}
		default:
		{
			break;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_HandleCollision
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void BOT_HandleCollision( userEntity_t *self, float fSpeed, CVector &forwardDirection )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	float fDist = fSpeed / 10.0f;
	if ( self->velocity.Length() > 0 )
	{
		fDist = fSpeed / 5.0f;
	}
	if ( fDist < 32.0f )
	{
		fDist = 32.0f;
	}

	int nWallType = AI_CheckAirTerrain( self, forwardDirection, fDist );
	switch ( nWallType )
	{
		case TER_OBSTRUCTED:
		{
			AI_Dprintf( "Strafing along the wall.\n" );

			CVector dir;
			AI_ComputeAlongWallVector( self, forwardDirection, terrain.wall_normal, dir );
			forward = dir;
			
			BOT_ModifyStrafeDirectionAfterCollision( self );
			break;
		}
		case TER_WALL:
		{
			AI_Dprintf( "Strafing along the wall.\n" );

			CVector dir;
			AI_ComputeAlongWallVector( self, forwardDirection, terrain.wall_normal, dir );
			forwardDirection = dir;

			BOT_ModifyStrafeDirectionAfterCollision( self );
			break;
		}
		
		case TER_HIGH_OBS:
		{
			gstate->Con_Dprintf( "Detected high obstruction.\n" );
			
			// if it is a ceiling then, glide along it
			if ( terrain.wall_normal.z < -0.7f )
			{
				forwardDirection.z = 0.0f;
				forwardDirection.Normalize();
			}
			else
			{
				// duck under
				forwardDirection.x = 0.0f;
				forwardDirection.y = 0.0f;
				forwardDirection.z = -1.0f;
			}
			break;
		}
		case TER_WALL_RIGHT:
		{
			gstate->Con_Dprintf( "Right side Blocked.\n" );

			// right side is blocked
			// rotate the wall normal to the left so that we can move around this 
			// obstacle
			CVector dir( terrain.wall_normal.y, -terrain.wall_normal.x, forwardDirection.z );
			dir.Normalize();
			forwardDirection = dir;
			
			BOT_ModifyStrafeDirectionAfterCollision( self );
			break;
		}
		case TER_WALL_LEFT:
		{
			gstate->Con_Dprintf( "Left side Blocked.\n" );
			
			// left side is blocked
			// rotate the wall normal to the right so that we can move around this 
			// obstacle
			CVector dir( -terrain.wall_normal.y, terrain.wall_normal.x, forwardDirection.z );
			dir.Normalize();
			forwardDirection = dir;
			
			BOT_ModifyStrafeDirectionAfterCollision( self );
			break;
		}		
		case TER_LOW_OBS:
		{
			// go up
			gstate->Con_Dprintf( "Detected low obstruction.\n" );

			if ( terrain.wall_normal.z > 0.7f )
			{
				forwardDirection.z = 0.0f;
				forwardDirection.Normalize();
			}
			else
			{
				forwardDirection.x = 0.0f;
				forwardDirection.y = 0.0f;
				forwardDirection.z = 1.0f;
			}
			break;
		}

		default:
		{
			break;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_UpdateAttackMovement
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_UpdateAttackMovement( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );
	userEntity_t *enemy = self->enemy;
	_ASSERTE( enemy );

	if ( hook->strafe_time <= gstate->time )
	{
		hook->strafe_time = gstate->time + (rnd() * 3.0f) + 1.5f;
		hook->strafe_dir = rand() % (STRAFE_DOWNRIGHT+1);
	}
	
	CVector vectorTowardPoint;
	CVector angleTowardPoint;
	vectorTowardPoint = self->s.origin - enemy->s.origin;
	vectorTowardPoint.Normalize();
	VectorToAngles( vectorTowardPoint, angleTowardPoint );
	
	switch ( hook->strafe_dir )
	{
		case STRAFE_LEFT:
		{
			angleTowardPoint.y += 90.0f;
			break;
		}
		case STRAFE_RIGHT:
		{
			angleTowardPoint.y -= 90.0f;
			break;
		}
		case STRAFE_UPLEFT:
		{
			angleTowardPoint.y += 45.0f;
			break;
		}
		case STRAFE_UPRIGHT:
		{
			angleTowardPoint.y -= 45.0f;
			break;
		}
		case STRAFE_DOWNLEFT:
		{
			angleTowardPoint.y += 135.0f;
			break;
		}
		case STRAFE_DOWNRIGHT:
		{
			angleTowardPoint.y -= 135.0f;
			break;
		}
		default:
		{
			break;
		}
	}
	
	angleTowardPoint.y = AngleMod( angleTowardPoint.y );
	
	CVector facingDir, right, up;
	angleTowardPoint.AngleToVectors( facingDir, right, up );
	facingDir.Normalize();

	float fHalfAttackDistance = hook->attack_dist * 0.5f;
	CVector destPoint;
	VectorMA( enemy->s.origin, facingDir, fHalfAttackDistance, destPoint );

	CVector movingVector;
	vectorTowardPoint = destPoint - self->s.origin;
	vectorTowardPoint.Normalize();

	float fSpeed = hook->attack_speed;
	BOT_HandleCollision( self, fSpeed, vectorTowardPoint );
	
	// scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	self->velocity = vectorTowardPoint * (fSpeed * fScale);
}

// ----------------------------------------------------------------------------
//
// Name:		bot_attack
// Description:
//	bot has started firing weapon at enemy
//	bot will remain in this routine as long as his enemy is visible
//
//	bot will fire weapon at enemy whenever hook->attack_finished <= gstate->time
//	and there is a clear shot
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void bot_attack( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	_ASSERTE( self->enemy );

	AI_FaceTowardPoint( self, self->enemy->s.origin );
	AI_UpdatePitchTowardEnemy( self );
	BOT_UpdateAttackMovement( self );

	float fDist = VectorDistance(self->enemy->s.origin, self->s.origin);

	if ( gstate->time >= hook->attack_finished )
	{
		//	fire the current weapon
		if ( self->curWeapon )
		{
			//	choose an optimal point of impact, taking bot skill into account
			//	bot_aim (self);

			//	already facing our enemy in yaw direction
			//	temporarily set pitch to face enemy
			CVector vectorTowardEnemy = self->enemy->s.origin - self->s.origin;
			vectorTowardEnemy.Normalize();
			
			CVector angleTowardEnemy;
			VectorToAngles(vectorTowardEnemy, angleTowardEnemy);
			
			self->client->v_angle = self->s.angles;
			self->client->v_angle.pitch = angleTowardEnemy.pitch;
			self->client->v_angle.roll = angleTowardEnemy.roll;

			hook->weapon_fired = self->curWeapon;
			self->curWeapon->use( self->curWeapon, self );

	  		gstate->Con_Printf( "firing weapon\n" );
		}

		hook->attack_finished = gstate->time + 1.0;
	}

	if ( AI_IsEndAnimation( self ) && !AI_IsEnemyDead( self ) )
	{
		if ( fDist > hook->attack_dist || !com->Visible(self, self->enemy) )
		{
			AI_RemoveCurrentTask( self );

			return;
		}
		else
		{
			bot_set_attack_seq(self);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		bot_begin_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void bot_begin_attack( userEntity_t *self )
{
	_ASSERTE( self );
	_ASSERTE( self->enemy );

	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	AI_ZeroVelocity( self );

	AI_SelectRunningAnimation( self, szAnimation );
	frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
	AI_ForceSequence(self, pSequence, FRAME_LOOP);
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_FindEnemy
//
// Description:	finds the first client or bot within hook->active_dist units
//				of self.  Only entities with an opposing alignment are 
//				considered.
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *BOT_FindEnemy( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	userEntity_t *head = alist_FirstEntity( client_list );
	while ( head != NULL )
	{
		// is this a client, bot or monster?
		if ( self != head && 
			 head->health > 0 && !(head->flags & FL_NOTARGET) && (head->deadflag == DEAD_NO) )
		{
			float fDistance = VectorDistance( self->s.origin, head->s.origin );
			if ( fDistance < hook->active_distance && AI_IsVisible(self, head) )
			{
				return head;
			}
		}

		head = alist_NextEntity( client_list );
	}

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		monster_bot
// Description:
//	sets up an entity to be a deathmatch bot
//	does not spawn the entity, that is done
//	by the server if it is a map entity or 
//	by bot_spawn
//
//	FIXME: won't spawn directly from a map
//	because node list isn't set up correctly
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void monster_bot( userEntity_t *self )
{
	playerHook_t *hook = AI_InitMonster( self, TYPE_BOT );
	_ASSERTE( hook );

	BOT_InitUserEntity(self);

	BOT_InitWeapons(self);
	BOT_InitDefaultWeapon(self);

	spawn_tele_gib( self->s.origin, self );

	AI_SelectAmbientAnimation( self, szAnimation );
	frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
	AI_StartSequence(self, pSequence, FRAME_LOOP);

	gstate->LinkEntity(self);

	AI_DetermineMovingEnvironment( self );
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_GetSpawnPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *BOT_GetSpawnPoint()
{
	userEntity_t *spot = NULL;

	if ( IsDeathMatch() )
	{
		spot = com->DeathmatchSpawnPoint("info_player_deathmatch");
	}
	else
	{
		spot = com->SelectSpawnPoint("info_player_start", gstate->game->spawnpoint);
	}

	return spot;
}

// ----------------------------------------------------------------------------
//
// Name:		bot_spawn
// Description:
//	spawns a deathmatch opponent
//	then calls monster_bot to initialize it
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void bot_spawn( userEntity_t *self )
{
	userEntity_t *bot = gstate->SpawnEntity();

	// look for place to put player
	userEntity_t *spot = BOT_GetSpawnPoint();
	if ( !spot )
	{
		if ( IsDeathMatch() )
		{
			com->Warning ("No info_player_deathmatch spawn points.");
		}
		else
		{
			com->Warning ("No info_player_start spawn points.");
		}
	}
	else
	{
		bot->s.origin = spot->s.origin;
		bot->s.angles = spot->s.angles;
	}

	monster_bot( bot );

	nBotCount++;
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_Respawn
// Description:
//	respawns the player after death
//	FIXME: sometimes player can't respawn... not fully dead?
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int	BOT_Respawn( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	// get rid of all previous tasksm, a fresh start
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOALSTACK_ClearAllGoals( pGoalStack );
	
	// add a dead body to the world only if the client wasn't gibbed
	if ( !(self->s.renderfx & RF_NODRAW) )
	{
		bodylist_add( self );
	}

	userEntity_t *spot = BOT_GetSpawnPoint();
	if ( !spot )
	{
		gstate->Con_Printf( "No open spawn position for %s!\n", self->netname );
		return FALSE;
	}

    if ( IsDeathMatch() )
    {
        hook->items			= 0;
		hook->power_boost	= 0;
		hook->attack_boost	= 0;
		hook->speed_boost	= 0;
		hook->acro_boost	= 0;
		hook->vita_boost	= 0;
    }

	self->s.origin	= spot->s.origin;
	self->s.angles	= spot->s.angles;
	self->s.event	= EV_PLAYER_TELEPORT;

	BOT_InitUserEntity(self);

	BOT_InitWeapons(self);
	BOT_InitDefaultWeapon(self);

	spawn_tele_gib( self->s.origin, self );

	AI_SelectAmbientAnimation( self, szAnimation );
	frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
	AI_StartSequence(self, pSequence, FRAME_LOOP);

	gstate->LinkEntity(self);

	return	TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_Move
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int BOT_Move( userEntity_t *self )
{

#ifdef CLIENT_ADDITIVE_VELOCITY
/*
	self->velocity.x = push.forward.x * pm.cmd.forwardmove + push.right.x * pm.cmd.sidemove;
	self->velocity.y = push.forward.y * pm.cmd.forwardmove + push.right.y * pm.cmd.sidemove;
*/
#else
/*
	float fForward = pm.cmd.forwardmove;
	float fSide = pm.cmd.sidemove;
	if ( fabs(fForward) > 0.0f && fabs(fSide) > 0.0f )
	{
		fForward = fForward * 0.708f;
		fSide = fSide * 0.708f;
	}
	self->velocity.x = push.forward.x * fForward + push.right.x * fSide;
	self->velocity.y = push.forward.y * fForward + push.right.y * fSide;
*/
#endif CLIENT_ADDITIVE_VELOCITY

	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	if ( hook->pPathList->pPath && hook->pPathList->pPath->nNodeIndex >= 0 )
	{
		_ASSERTE( pGroundNodes );

		MAPNODE_PTR pNode = NODE_GetNode( pGroundNodes, hook->pPathList->pPath->nNodeIndex );
		if ( BOT_MoveTowardPoint( self, pNode->position, TRUE ) == TRUE )
		{
			// we're already at this node, so delete the first one
			PATHLIST_DeleteFirstInPath( hook->pPathList );

			if ( hook->pPathList->pPath )
			{
				NODELIST_PTR pNodeList = hook->pNodeList;
				_ASSERTE( pNodeList );
				NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
				_ASSERTE( pNodeHeader );

				MAPNODE_PTR pNextNode = NODE_GetNode( pNodeHeader, hook->pPathList->pPath->nNodeIndex );
				if ( AI_HandleUse( self, pNodeHeader, pNode, pNextNode ) )
				{
					return TRUE;
				}
			}

			BOT_Move( self );
		}

		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		BOT_Think
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void BOT_Think( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( self->deadflag == DEAD_DEAD )
	{
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( hook->pGoals );
		if ( !pCurrentGoal )
		{
			return;
		}
		TASK_PTR pCurrentTask = GOAL_GetCurrentTask( pCurrentGoal );
		_ASSERTE( pCurrentTask );

		int nCurrentTaskType = TASK_GetType( pCurrentTask );
		if ( nCurrentTaskType == TASKTYPE_BOT_DIE )
		{
			BOT_Die( self );
			self->nextthink = gstate->time + 0.1f;
		}
		else
		{
			_ASSERTE( FALSE );
			self->nextthink = gstate->time + 2.0f;
		}
		return;
	}

	GOAL_PTR pCurrentGoal = NULL;

//#define _BOT_ATTACK
#ifdef _BOT_ATTACK
	if ( self->deadflag == DEAD_NO && AI_IsOkToAttack( hook ) )
	{
		// do self preservation actions
		if ( AI_HandleSelfPreservation( self ) )
		{
			return;
		}

		//	check for a target
		userEntity_t *pEnemy = BOT_FindEnemy( self );
		if ( pEnemy && pEnemy->inuse )
		{
			if ( pEnemy != self->enemy )
			{
				AI_SetEnemy( self, pEnemy );
				AI_AddNewGoal( self, GOALTYPE_BOT_ENGAGEENEMY, pEnemy );
				self->nextthink = gstate->time + 0.1f;
				return;
			}
			else
			{
				GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
				pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
				GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
				if ( nGoalType == GOALTYPE_BOT_ENGAGEENEMY )
				{
					AI_RestartCurrentGoal( self );
				}
				else
				{
					AI_AddNewGoal( self, GOALTYPE_BOT_ENGAGEENEMY, pEnemy );

					self->nextthink = gstate->time + 0.1f;
					return;
				}
			}
		}
	}
#endif 

	pCurrentGoal = GOALSTACK_GetCurrentGoal( hook->pGoals );
	if ( !pCurrentGoal )
	{
		AI_AddNewGoal( self, GOALTYPE_BOT_ROAM );
		
		pCurrentGoal = GOALSTACK_GetCurrentGoal( hook->pGoals );
		_ASSERTE( pCurrentGoal );
	}
	TASK_PTR pCurrentTask = GOAL_GetCurrentTask( pCurrentGoal );
	_ASSERTE( pCurrentTask );

	CVector lastPosition = hook->last_origin;

	int nCurrentTaskType = TASK_GetType( pCurrentTask );
	switch ( nCurrentTaskType )
	{
		case TASKTYPE_IDLE:
		{
			AI_Idle( self );
			self->nextthink	= gstate->time + 0.2f;
			break;
		}
		case TASKTYPE_MOVE:
		{
			break;
		}
		case TASKTYPE_JUMPUP:
		{
			if ( hook->fnJumpUp )
			{
				hook->fnJumpUp( self );
				self->nextthink	= gstate->time + 0.1f;
			}
			break;
		}
		case TASKTYPE_JUMPFORWARD:
		{
			if ( hook->fnJumpForward )
			{
				hook->fnJumpForward( self );
				self->nextthink	= gstate->time + 0.1f;
			}
			break;
		}
		case TASKTYPE_SWITCH:
		{
			break;
		}
		case TASKTYPE_HIDE:
		{
			AI_Hide( self );
			self->nextthink	= gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_AMBUSH:
		{
			break;
		}
		case TASKTYPE_WAITFORTRAINTOCOME:
		{
			AI_WaitForTrainToCome( self );
			self->nextthink	= gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_WAITFORTRAINTOSTOP:
		{
			AI_WaitForTrainToStop( self );
			self->nextthink	= gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_WAITFORPLATFORMTOCOME:
		{
			break;
		}
		case TASKTYPE_WAITFORPLATFORMTOSTOP:
		{
			break;
		}
		case TASKTYPE_WAITFORDOORTOOPEN:
		{
			break;
		}
		case TASKTYPE_WAITFORDOORTOCLOSE:
		{
			break;
		}
		case TASKTYPE_MOVEINTOPLATFORM:
		{
			break;
		}
		case TASKTYPE_MOVEOUTOFPLATFORM:
		{
			break;
		}
		case TASKTYPE_MOVETOLOCATION:
		{
			AI_MoveToLocation( self );
			self->nextthink	= gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_MOVETOEXACTLOCATION:
		{
			AI_MoveToExactLocation( self );
			self->nextthink	= gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_GOINGAROUNDOBSTACLE:
		{
			AI_GoingAroundObstacle( self );
			self->nextthink	= gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_SWIMTOLOCATION:
		{
			AI_SwimToLocation( self );
			self->nextthink	= gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_FACEANGLE:
		{
			AI_FaceAngle( self );
			self->nextthink	= gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_WAIT:
		{
			AI_Wait( self );
			self->nextthink	= gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_ACTIVATESWITCH:
		{
			break;
		}
		case TASKTYPE_MOVETOENTITY:
		{
			AI_MoveToEntity( self );
			self->nextthink	= gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_STAND:
		{
			break;
		}				
		case TASKTYPE_TAKECOVER:
		{
			AI_TakeCover( self );
			self->nextthink	= gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_DODGE:
			{
			AI_Dodge( self );
			self->nextthink	= gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_SIDESTEP:
		{
			AI_SideStep( self );
			self->nextthink	= gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_GOINGAROUNDENTITY:
		{
			AI_GoingAroundEntity( self );
			self->nextthink	= gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_SEQUENCETRANSITION:
		{
			AI_SequenceTransition( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_FINISHCURRENTSEQUENCE:
		{
			AI_FinishCurrentSequence( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_CHASESIDESTEPLEFT:
		{
			AI_ChaseSideStepLeft( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_CHASESIDESTEPRIGHT:
		{
			AI_ChaseSideStepRight( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_CHARGETOWARDENEMY:
		{
			AI_ChargeTowardEnemy( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_JUMPTOWARDPOINT:
		{
			AI_JumpTowardPoint( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_SHOTCYCLERJUMP:
		{
			AI_ShotCyclerJump( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_MOVEUNTILVISIBLE:
		{
			AI_MoveUntilVisible( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_USEPLATFORM:
		{
			AI_UsePlatform( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_MOVEDOWN:
		{
			AI_MoveDown( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_UPLADDER:
		{
			AI_UpLadder( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_DOWNLADDER:
		{
			AI_DownLadder( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_WAITONLADDERFORNOCOLLISION:
		{
			AI_WaitOnLadderForNoCollision( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}


		case TASKTYPE_BOT_DIE:
		{
			BOT_Die( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_BOT_ROAM:
		{
			BOT_Roam( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_BOT_PICKUPITEM:
		{
			BOT_PickupItem( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_BOT_ENGAGEENEMY:
		{
			BOT_EngageEnemy( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_BOT_CHASEATTACK:
		{
			BOT_ChaseAttack( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_BOT_STRAFELEFT:
		{
			BOT_StrafeLeft( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_BOT_STRAFERIGHT:
		{
			BOT_StrafeRight( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}
		case TASKTYPE_BOT_CHARGETOWARDENEMY:
		{
			BOT_ChargeTowardEnemy( self );
			self->nextthink = gstate->time + 0.1f;
			break;
		}

		default:
		{
			_ASSERTE( FALSE );
			break;
		}
	}

	// the following code makes sure that a monster does not get stuck for a long time
	if ( AI_IsAlive( self ) && self->movetype != MOVETYPE_NONE )
	{
		float fDistanceTraveled = VectorDistance( self->s.origin, lastPosition );
        float fVelocity = self->velocity.Length();
		if ( fVelocity > 0.0f && fDistanceTraveled > 0.0f )
		{
			hook->nMovingCounter++;
			hook->fTenUpdatesDistance += fDistanceTraveled;
			if ( (hook->nMovingCounter % 10) == 0 )
			{
				float fEnoughMoveDistance = fVelocity * 0.125f;

				if ( hook->fTenUpdatesDistance < fEnoughMoveDistance )
				{
					AI_RestartCurrentGoal( self );
				}
				else
				{
					hook->fTenUpdatesDistance = 0.0f;
				}
			}
		}
		else
		{
			hook->nMovingCounter = 0;
            hook->fTenUpdatesDistance = 0.0f;
		}

		if ( AI_IsOverTaskFinishTime( hook ) )
		{
			if ( nCurrentTaskType == TASKTYPE_BOT_PICKUPITEM )
			{
				AI_RemoveCurrentGoal( self );
			}
			else
			{
				AI_RemoveCurrentTask( self );
			}
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


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_bot_register_func()
{
	gstate->RegisterFunc("BOT_StartDie",BOT_StartDie);
	gstate->RegisterFunc("BOT_Think",BOT_Think);
	gstate->RegisterFunc("BOT_Pain",BOT_Pain);
	gstate->RegisterFunc("BOT_PreThink",BOT_PreThink);
	gstate->RegisterFunc("BOT_PostThink",BOT_PostThink);
}
