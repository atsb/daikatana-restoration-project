#include "world.h"

//#include "ai_common.h"// SCG[1/23/00]: not used
#include "ai_frames.h"
#include "ai_utils.h"

#include "nodelist.h"
#include "triggers.h"
#include "client.h"
#include "bodylist.h"
#include "chasecam.h"
//#include "actorlist.h"// SCG[1/23/00]: not used
#include "client_init.h"

DllExport void	dll_BeginIntermission (char *nextMap);

///////////////////////////////////////////////////////////////////////////////
//	Client_InitUserEntity
//
//	self->s.origin should be set before calling this function
///////////////////////////////////////////////////////////////////////////////

void	Client_InitUserEntity (userEntity_t *self)
{
	playerHook_t *hook		= AI_GetPlayerHook( self );
	//	set the size
	gstate->SetSize( self, -16, -16, -24, 16, 16, 32 );
	self->s.render_scale.Set( 1.0, 1.0, 1.0 );

	// SCG[11/22/99]: set speed
	self->ang_speed.Set(90, 90, 90);
	self->velocity.Zero();

	self->className			= "player";

	int index				= self - gstate->g_edicts - 1;
	self->client			= &gstate->game->clients[index];

	//	server and physics
	self->movetype			= MOVETYPE_WALK;
	self->solid				= SOLID_SLIDEBOX;
	self->clipmask			= MASK_PLAYERSOLID;
	self->flags				|= ( FL_CLIENT | FL_PUSHABLE );
	self->elasticity		= 1.0;
	self->mass				= 300.0;
	self->svflags			|= SVF_MONSTER;
	self->svflags			&= ~SVF_DEADMONSTER;
	self->s.effects			= EF_GIB;				// this makes server send client info to the clients //???
// SCG[12/7/99]: 	self->s.renderfx		= RF_CLUSTERBASED;		// makes this model draw with cluster based animation
	self->s.iflags			= IF_SV_SENDBBOX;		// send the real BBOX size down to client for prediction	
							
	self->s.effects			= 0;
	self->s.origin.z		+= 0.125;		// make sure off ground
							
	//	set up skins		
	self->s.skinnum			= 0;
	self->s.frame			= 0;
							
	self->viewheight		= 22.0;

	// cek[5-16-00]: effectively resets max_speed after dying
//	self->max_speed			= ATTR_RUNSPEED;
	self->max_speed			= ATTR_RUNSPEED + (ATTR_RUNSPEED / ATTR_MAX_LEVELS) * (float) hook->speed_boost;

	if (deathmatch->value || coop->value)
		self->health			= (hook && (hook->base_health > 0)) ? hook->base_health : ATTR_HEALTH;
	else
		self->health			= ATTR_HEALTH;

	self->takedamage		= DAMAGE_AIM;
	self->deadflag			= DEAD_NO;
							
	//	thinks				
	self->prethink			= Client_PreThink;
	self->postthink			= Client_PostThink;
	self->pain				= Client_Pain;
	self->die				= Client_Die;

	if (hook)
	{
		hook->bInTransition		= FALSE;
		hook->nTransitionIndex	= 0;
		hook->fTransitionTime	= 0.0f;
		hook->pTransitions		= NULL;
	}

    // set this value here so that sidekicks can be on the same team as the client
    // and they won't hurt each other during fighting
    if ( !deathmatch->value )
    {
        self->team  = 1;
    }
	gstate->LinkEntity(self);
}

///////////////////////////////////////////////////////////////////////////////
//	Client_FreeInventory
//
//	Frees inventory when a player restarts (single player) or respawns (coop
//	and deathmatch)
//
//	Called
///////////////////////////////////////////////////////////////////////////////

void Client_FreeInventory (userEntity_t *self)
{
	if (!self->inventory)
		return;

	if (deathmatch->value)
	{
		//	in deathmatch and coop, free up our inventory
		gstate->InventoryFree (self->inventory);
		self->inventory = NULL;
		//	make sure there are no pointers to weapons left
		Client_InitWeapons (self);
	}

}

///////////////////////////////////////////////////////////////////////////////
//	Client_InitInventory
//
//	Frees inventory when a player restarts (single player) or respawns (coop
//	and deathmatch)
///////////////////////////////////////////////////////////////////////////////

void Client_InitInventory (userEntity_t *self)
{
	if( self->inventory )
	{
		//	client already has an inventory, so free it and set ptr to NULL
		Client_FreeInventory (self);
	}

	self->inventory = gstate->InventoryNew (MEM_PERMANENT);
}

///////////////////////////////////////////////////////////////////////////////
//	Client_InitTimers
//
//	Inits all time-related variables in userHook
///////////////////////////////////////////////////////////////////////////////

void	Client_InitTimers (userEntity_t *self)
{
	playerHook_t	*hook = AI_GetPlayerHook( self );
	
	//	poison stuff
	hook->poison_time = 0;
	hook->poison_next_damage_time = 0;
	hook->poison_damage = 0;
	hook->poison_interval = 0;

	//	power ups
	hook->invulnerability_time = 0;
	hook->envirosuit_time = 0;
	hook->wraithorb_time = 0;
	hook->oxylung_time = 0;

	//	temporary attribute boosts
	hook->power_boost_time = 0;
	hook->attack_boost_time = 0;
	hook->speed_boost_time = 0;
	hook->acro_boost_time = 0;
	hook->vita_boost_time = 0;

	//	other timer fields
	hook->pain_finished = 0;
	hook->drown_time = gstate->time + 12.0;
	hook->attack_finished = 0;
	hook->jump_time = 0;
	hook->sound_time = 0;
	hook->path_time = 0;
	hook->think_time = 0.1;
	hook->shake_time = 0;
	hook->ambient_time = 0;
	hook->freezeLevel = 0;
	hook->freezeTime = 0;

	//	clear flags
	hook->exp_flags = 0;

	//	Nelno:	never finished implementing this...
	hook->fatigue = 0;

	// clear out some stuff related to these timers

}

///////////////////////////////////////////////////////////////////////////////
//	Client_InitAttributes
//
//	Inits all variables related to client attributes
///////////////////////////////////////////////////////////////////////////////

void	Client_InitAttributes (userEntity_t *self)
{
	playerHook_t	*hook = AI_GetPlayerHook( self );

    if(hook)
    {
	    hook->base_health = ATTR_HEALTH;
	    hook->pain_chance = 0;
	    
		// set defaults for deathmatch mode
		if (deathmatch->value)
		{
			// base attribs
			switch(hook->iPlayerClass)
			{
				case PLAYERCLASS_HIRO:
					hook->base_power    = 0;
					hook->base_attack   = 1;
					hook->base_acro     = 1;
					hook->base_vita     = 0;
					hook->base_speed    = 0;
					break;
				case PLAYERCLASS_MIKIKO:
					hook->base_power    = 0;
					hook->base_attack   = 1;
					hook->base_acro     = 0;
					hook->base_vita     = 0;
					hook->base_speed    = 1;
					break;
				case PLAYERCLASS_SUPERFLY:
					hook->base_power    = 1;
					hook->base_attack   = 0;
					hook->base_acro     = 0;
					hook->base_vita     = 1;
					hook->base_speed    = 0;
					break;
				default:
					hook->base_power    = 0;
					hook->base_attack   = 0;
					hook->base_acro     = 0;
					hook->base_vita     = 0;
					hook->base_speed    = 0;
					break;
			}
		}
		else
		{
			// single player they build stats on their own
			hook->base_power = hook->base_attack = hook->base_acro = hook->base_vita = hook->base_speed = 0;
		}

        //	special attributes
	    hook->power_boost = hook->base_power;
	    hook->attack_boost = hook->base_attack;
	    hook->speed_boost = hook->base_speed;
	    hook->acro_boost = hook->base_acro;
	    hook->vita_boost = hook->base_vita;
        com->CalcBoosts(self);
    }
}

///////////////////////////////////////////////////////////////////////////////
//	Client_FreeWeapon
//
//	untracks the client's weapon and removes the entity associated with it
///////////////////////////////////////////////////////////////////////////////

void	Client_FreeWeapon (userEntity_t *self)
{
	userEntity_t	*weapon = (userEntity_t *) self->client->ps.weapon;

/*    self->oldWinfo = self->winfo;
    // free the weapon entity
	if (weapon != NULL && weapon->inuse)
	{
		RELIABLE_UNTRACK(weapon);
		weapon->remove (weapon);
	}
    self->s.modelindex2 = 0;

	self->curWeapon = NULL;
	self->client->ps.weapon = NULL;*/
	// cek[11-30-99] let's just hide the friggin thing...alot easier.
	//NSS[11/30/99]: What say we check to see if this is valid or not?
	if(weapon)
	{
		weapon->s.renderfx ^= RF_NODRAW;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//  Client_InitDefaultWeapon
//
//  Give client default weapon
//
///////////////////////////////////////////////////////////////////////////////
static char *weaponChoices[5][7] =
{
	{
		NULL,NULL,NULL,NULL,NULL,NULL,NULL
	},
	{
		"weapon_disruptor","weapon_ionblaster","weapon_c4","weapon_shotcycler",
		"weapon_sidewinder","weapon_shockwave","weapon_daikatana"
	},
	{
		"weapon_discus","weapon_venomous","weapon_sunflare","weapon_hammer",
		"weapon_trident","weapon_zeus","weapon_daikatana"
	},
	{
		"weapon_silverclaw","weapon_bolter","weapon_stavros","weapon_ballista",
		"weapon_wyndrax","weapon_nightmare","weapon_daikatana"
	},
	{
		"weapon_glock","weapon_slugger","weapon_kineticore","weapon_ripgun",
		"weapon_novabeam","weapon_metamaser","weapon_daikatana"
	}
};

void Client_InitDefaultWeapon(userEntity_t* self)
{
	weaponInfo_t	*winfo = NULL;

	// WAW[11/19/99]: Initial weapons
	if (deathmatch->value || coop->value)
	{
		self->client->pers.fGasHandsTime = 0;

		// in coop mode, first refill all of the weapons that the player is carrying...
		if (coop->value && self->inventory)
		{
			int curAmmo,initAmmo;
			userInventory_t *item;
			for (item = gstate->InventoryFirstItem(self->inventory); item != NULL; item = gstate->InventoryNextItem(self->inventory))
			{
				winfo = com->FindRegisteredWeapon( item->name );
				if (winfo)
				{
					// don't give refills for some weapons...
					if (!stricmp(item->name,"weapon_daikatana") || !stricmp(item->name,"weapon_disruptor") || 
						!stricmp(item->name,"weapon_gashands") || !stricmp(item->name,"weapon_silverclaw"))
						continue;

					// ok, the weapon should be refilled...
					weapon_t *weapon = (weapon_t *)item;
					curAmmo = weapon->ammo->count;
					initAmmo = winfo->initialAmmo;
					if (curAmmo < initAmmo)
					{
						winfo->give_func(self,initAmmo - curAmmo);
					}
				}
			}
		}
		else if (dm_instagib->value)
		{
			char *weap = NULL;
			int r = rand() % 2;

			switch(gstate->episode)
			{
			case 1:
				switch(r)
				{
				default:
				case 0: weap = "weapon_sidewinder"; break;
				case 1: weap = "weapon_ionblaster"; break;
				}
				break;
				
			case 2:
				switch(r)
				{
				default:
				case 0: weap = "weapon_trident"; break;
				case 1: weap = "weapon_venomous"; break;
				}
				break;
				
			case 3:
				switch(r)
				{
				default:
				case 0: weap = "weapon_bolter"; break;
				case 1: weap = "weapon_ballista"; break;
				}
				break;
				
			case 4:
				switch(r)
				{
				default:
				case 0: weap = "weapon_slugger"; break;
				case 1: weap = "weapon_kineticore"; break;
				}
				break;			
			};

			if (weap)
			{
				// find
				winfo = com->FindRegisteredWeapon( weap );
				if (winfo)
				{
					// give
					winfo->give_func (self, winfo->initialAmmo);

					// select the weapon
					if (!self->curWeapon)
						winfo->select_func(self);
				}
			}

			return;
		}

		// see if the spawnflags indicates that weapons should be given to the player...
		if (self->spawnflags & PLAYER_SF_OVERRIDE_WEAPONS)
		{
			int episode = (int)sv_episode->value;
			if ((episode < 1) || (episode > 4))
				episode = 0;

			int i = 0;
			for (long flag = PLAYER_SF_WEAPON_1; i < 7; flag <<= 1,i++)
			{
				if (self->spawnflags & flag)
				{
					char *wname = weaponChoices[episode][i];

					if (wname)
					{
						winfo = com->FindRegisteredWeapon( wname );
						if (winfo)
						{
							if (self->inventory)
							{
								userInventory_t *item = gstate->InventoryFindItem(self->inventory,wname);
								if (item)
								{
									if (!self->curWeapon)
										winfo->select_func(self);
									continue;
								}
							}

							winfo->give_func( self, winfo->initialAmmo );
							if (!self->curWeapon)
								winfo->select_func(self);
						}
					}
				}
			}
		}
		else
		{
			// nope...just give default weapons...
			char *wname = NULL;
			switch( (int)sv_episode->value )
			{
			default:
			case 1:
				wname = "weapon_disruptor";
				break;
			case 2:
				wname = "weapon_discus";
				break;
			case 3:
				wname = "weapon_silverclaw";
				break;
			case 4:
				wname = "weapon_glock";
				break;
			}

			// give em a daikatana (in all but ep 1 in coop)
			if ( (!coop->value) || ((coop->value) && ((int)sv_episode->value != 1)) )
			{
				// cek[1-7-99]: give the players a daikatana in deathmatch
				winfo = com->FindRegisteredWeapon ("weapon_daikatana");
				if (winfo)
				{
					winfo->give_func (self, winfo->initialAmmo);
				}
			}

			if (wname)
			{
				winfo = com->FindRegisteredWeapon( wname );
				if (winfo)
				{
					// in coop, only give em a weapon if they don't have it
					if (self->inventory)
					{
						userInventory_t *item = gstate->InventoryFindItem(self->inventory,wname);
						if (item)
						{
							if (!self->curWeapon)
							{
								winfo->select_func(self);
								self->client->needsUpdate |= HUD_UPDATE_WEAP;
							}
							return;
						}
					}

					winfo->give_func( self, winfo->initialAmmo );
					if (!self->curWeapon)
						winfo->select_func(self);
				}
			}
		}
		self->client->needsUpdate |= HUD_UPDATE_WEAP;
		return;
	}

	// give client a weapon if deathmatch
	// Logic[5/8/99]: if (deathmatch->value && self->inventory)
    if (self->inventory)
	{
		if (sv_episode->value == 1)
		{
			winfo = com->FindRegisteredWeapon ("weapon_disruptor");
			if (winfo)
			{
				winfo->give_func (self, winfo->initialAmmo);
			}
		}	
		else
		{
			winfo = com->FindRegisteredWeapon ("weapon_daikatana");
			if (winfo)
			{
				winfo->give_func (self, winfo->initialAmmo);
			}
		}

		// select the weapon
		if (winfo && !self->curWeapon)
			winfo->select_func(self);
	}

}

///////////////////////////////////////////////////////////////////////////////
//	Client_InitWeapons
//
//	Inits all variables related to client weapons
///////////////////////////////////////////////////////////////////////////////

void	Client_InitWeapons (userEntity_t *self)
{
	playerHook_t	*hook = AI_GetPlayerHook( self );
	userEntity_t	*weapon;
//	weaponInfo_t	*winfo = NULL;// SCG[1/23/00]: not used

	//	don't show weapon
	weapon = (userEntity_t *) self->client->ps.weapon;
	if (weapon)
		weapon->s.renderfx |= RF_NODRAW;

	self->curWeapon = NULL;
	self->client->ps.weapon = NULL;

	hook->fxFrameNum = 0;
	hook->fxFrameFunc = NULL;

	hook->last_button0 = 0;
	hook->viewmodel_start_frame = 0;
	hook->viewmodel_end_frame = 0;
	hook->viewmodel_frame_time = 0;

	hook->weapon_fired = NULL;
	hook->weapon_next = NULL;

	// clear out armor also
	self->armor_val = 0;
	self->armor_abs = 0.0f;
}

///////////////////////////////////////////////////////////////////////////////
//	Client_InitHookMisc
//
//	Inits miscellaneous userHook variables 
///////////////////////////////////////////////////////////////////////////////

void	Client_InitHookMisc (userEntity_t *self)
{
	playerHook_t	*hook = AI_GetPlayerHook( self );

	hook->dflags	&= ~(DFL_EVIL | DFL_NEUTRAL | DFL_BERSERK);
	hook->dflags	= DFL_GOOD;
	hook->type		= TYPE_CLIENT;	//	default character type

	hook->fov_desired	= 90.0;
	hook->fov_rate		= 0.0;
}

///////////////////////////////////////////////////////////////////////////////
//	Client_InitAnims
//
//	Inits deathmatch statistics
///////////////////////////////////////////////////////////////////////////////

void	Client_InitAnims (userEntity_t *self)
{
//	playerHook_t	*hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	userEntity_t	*went;

	self->s.frameInfo.frameInc			= 1;
	self->s.frameInfo.frameTime			= FRAMETIME_FPS20;
	self->s.frameInfo.next_frameTime	= gstate->time + self->s.frameInfo.frameTime;

	went = (userEntity_t *) self->client->ps.weapon;
	if (went)
	{
		went->s.frameInfo.frameInc			= 1;
		went->s.frameInfo.frameTime			= FRAMETIME_FPS20;
		went->s.frameInfo.next_frameTime	= gstate->time + went->s.frameInfo.frameTime;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	Client_InitStats
//
//	Inits deathmatch statistics
///////////////////////////////////////////////////////////////////////////////

void	Client_InitStats (userEntity_t *self)
{
	playerHook_t	*hook = AI_GetPlayerHook( self );

	hook->killed = 0;
	hook->died = 0;
	hook->fired = 0;
	hook->hit = 0;

	//	Nelno:	????
	//	hook->dflags = hook->dflags | DFL_SHOWSTATS;
}

///////////////////////////////////////////////////////////////////////////////
//	Client_InitCamera
//
//	Inits camera-related fields
///////////////////////////////////////////////////////////////////////////////

void	Client_InitCamera (userEntity_t *self)
{
	playerHook_t	*hook = AI_GetPlayerHook( self );

	if (hook->camera)
		camera_stop (self, false);

	hook->camera = NULL;

	//	camera / possession
	gstate->SetClientViewEntity (self, self);
	gstate->SetClientInputEntity (self, self);
}

///////////////////////////////////////////////////////////////////////////////
//	Client_InitNodeSystem
//
//	Inits all variables related to client node system
//	FIXME:	make sure client's nodelists get freed when logging off, changing
//			levels, etc.
///////////////////////////////////////////////////////////////////////////////

void	Client_InitNodeSystem (userEntity_t *self)
{
	playerHook_t	*hook = AI_GetPlayerHook( self );

//!	hook->path				= NULL;
//!	hook->max_path_length	= 50;

	//	node list
	if ( !pGroundNodes )
	{
		pGroundNodes = NODE_Init( pGroundNodes );
	}

	if ( !pAirNodes )
	{
		pAirNodes = NODE_Init( pAirNodes );
	}

	if ( !pTrackNodes )
	{
		pTrackNodes = NODE_Init( pTrackNodes );
	}

	AI_InitNodeList( self );

	hook->pPathList = PATHLIST_Create();
}

///////////////////////////////////////////////////////////////////////////////
//	Client_FreeNodeSystem
//
//	frees data associated with node system
///////////////////////////////////////////////////////////////////////////////

void	Client_FreeNodeSystem (userEntity_t *self)
{
	playerHook_t	*hook = AI_GetPlayerHook( self );

	hook->pNodeList = AI_DeleteNodeList(hook->pNodeList);
	
	//	by default, players should not have either of these...
	PATHLIST_KillPath (hook->pPathList);
	hook->pPathList = PATHLIST_Destroy (hook->pPathList);
}

///////////////////////////////////////////////////////////////////////////////
//	Client_InitAIDefaults
//
//	Inits all AI related userHook fields
///////////////////////////////////////////////////////////////////////////////

void	Client_InitAIDefaults (userEntity_t *self)
{
	playerHook_t	*hook = AI_GetPlayerHook( self );

	hook->upward_vel	= 250.0;
	hook->forward_vel	= 350.0;
	hook->max_jump_dist = ai_max_jump_dist (hook->upward_vel, hook->forward_vel);

	ai_setfov (self, self->client->ps.fov);

	hook->attack_dist		= 2000.0;
	hook->run_speed			= ATTR_RUNSPEED;
	hook->walk_speed		= ATTR_RUNSPEED;
	hook->attack_speed		= ATTR_RUNSPEED;
	hook->jump_chance		= 0;
	hook->back_away_dist	= 0;
	hook->active_distance	= 8192;

	self->goalentity	= self;
	self->enemy			= NULL;
}

void Client_RemovePowerups(userEntity_t *self)
{
	// cek[1-27-00]: unwraith the player
	playerHook_t	*hook = AI_GetPlayerHook( self );
	if (hook)
	{
		hook->items &= ~(IT_WRAITHORB|IT_MEGASHIELD|IT_MANASKULL);
	}

    self->s.renderfx &= ~RF_TRANSLUCENT;
    self->s.renderfx &= ~RF_PREDATOR;
	self->s.renderfx &= ~RF_NODRAW;
}

///////////////////////////////////////////////////////////////////////////////
//	Client_InitGClient
//
///////////////////////////////////////////////////////////////////////////////

void	Client_InitGClient (userEntity_t *self, CVector &origin, CVector &angles)
{
	gclient_t	*client = self->client;

	// clear playerstate values
	memset (&self->client->ps, 0, sizeof(client->ps));

	//	so client doesn't still see blends after respawn
	client->damage_alpha = 0;
	client->bonus_alpha = 0;
	client->flash_alpha = 0;

	client->needsUpdate = 0;

	self->client->last_message_frame = 0;
	self->client->spam_count = 0;

//cek[1-10-00] totally breaks deathmatch
	if (maxclients->value > 1)	// WAW[11/17/99]: Don't do this durring network play.
	{
		self->client->ps.rdflags |= RDF_NOLERP;
		self->client->ps.pmove.pm_flags &= ~( PMF_CAMERAMODE | PMF_CAMERA_NOLERP );
	}

	// add a hacky flag to tell the client they're in coop mode...
	if (coop->value)
		self->client->ps.rdflags |= RDF_COOP;
	else if (deathmatch->value)
		self->client->ps.rdflags |= RDF_DEATHMATCH;

	client->ps.pmove.origin[0] = origin.x * 8;
	client->ps.pmove.origin[1] = origin.y * 8;
	client->ps.pmove.origin[2] = origin.z * 8;
	
//	client->ps.fov = 90;
/*
	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		client->ps.fov = 90;
	}
	else
*/	{
		client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
		if (client->ps.fov < 1)
			client->ps.fov = 90;
		else if (client->ps.fov > 160)
			client->ps.fov = 160;
	}

	//	set view angles
	gstate->SetClientAngles (self, angles);
	self->flags |= FL_FIXANGLES;	//	fix on next frame

	Client_RemovePowerups(self);
//	self->client->v_angle.Zero();


	self->takedamage = DAMAGE_YES;

	client->endIntermission = FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//	Client_Resurrect
//
///////////////////////////////////////////////////////////////////////////////

int	Client_Resurrect (userEntity_t *self)
{
	CVector			temp;
//	playerHook_t	*hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	// add a dead body to the world
	bodylist_add (self);	// amw: IS THIS EVEN TEH RIGHT PLACE TO CALL THIS???

	self->s.origin.Zero();
	self->s.angles.Zero();

//	Client_InitGClient (self, spot->s.origin, spot->s.angles);

	Client_InitUserEntity (self);
	gstate->LinkEntity (self);

	Client_InitTimers (self);
	//	don't strip inventory
//	Client_InitInventory (self);

	Client_InitWeapons (self);
	Client_InitHookMisc (self);
	Client_InitCamera (self);

	spawn_tele_gib( self->s.origin, self );

/* amw: remove shit code
	//	destroy all our hierarchical parts
	hr_remove_parts (self, true);
*/
	return	TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//	Client_MoveToIntermission
//
///////////////////////////////////////////////////////////////////////////////

void Client_MoveToIntermission (userEntity_t *ent)
{
	if (deathmatch->value || coop->value)
	{
		// um... SCG: 2-26-99
//		ent->client->showflags & (SHOW_SCORES_NOW | SHOW_SCORES);
		ent->client->showflags |= (SHOW_SCORES_NOW | SHOW_SCORES);
	}

	ent->s.renderfx |= RF_NODRAW;
	// SCG[9/24/99]: Don't change viewpoint.
	ent->s.origin = gstate->level->intermission_origin;

	ent->client->ps.pmove.origin[0]	= gstate->level->intermission_origin[0]*8;
	ent->client->ps.pmove.origin[1]	= gstate->level->intermission_origin[1]*8;
	ent->client->ps.pmove.origin[2]	= gstate->level->intermission_origin[2]*8;
	ent->client->fix_angles			= gstate->level->intermission_angle;
	ent->client->v_angle			= gstate->level->intermission_angle;
	ent->client->oldviewangles		= gstate->level->intermission_angle;
	ent->client->ps.viewangles		= gstate->level->intermission_angle;
	ent->s.angles					= gstate->level->intermission_angle;
	ent->flags |= FL_FIXANGLES;

	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.viewoffset.Zero();
	ent->client->ps.blend[3] = 0;
/*
	//	FIXME:	necessary for DK?
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	// FIXME:	init powerups
	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;
*/	

	//	FIXME:	force deathmatch scoreboard
}

///////////////////////////////////////////////////////////////////////////////
//	Client_BeginIntermission
//
//	Set up stuff for intermission
///////////////////////////////////////////////////////////////////////////////

#define	MAX_INTERMISSION_POINTS	16
int	Client_Respawn (userEntity_t *self);
void Client_BeginIntermission( char *nextMap, qboolean bShowStats )
{
	int				i, numSpots;
	userEntity_t	*client, spot [MAX_INTERMISSION_POINTS], *ent;

	if (gstate->level->intermissiontime)
		//	already in an intermission
		return;

	//	respawn dead clients
	for (i = 0; i < maxclients->value; i++)
	{
		client = &gstate->g_edicts [i + 1];
        if(!client->inuse)
            continue;
        if(client->health < 1)
            Client_Respawn(client);

		if (deathmatch->value)
			Client_RemovePowerups(client);
		client->flags |= FL_FREEZE|FL_NOTARGET;
		client->takedamage = DAMAGE_NO;
	}

	gstate->level->intermissiontime = gstate->level->time;
	strcpy (gstate->level->nextMap, nextMap);

	if (coop->value)
	{
		for (i = 0; i < maxclients->value; i++)
		{
			client = &gstate->g_edicts [i + 1];

			if (!client->inuse)
				continue;

			if( bShowStats == TRUE )
			{
			    client->client->showflags |= (SHOW_SCORES | SHOW_SCORES_NOW);	//	send update immediately
			}

			client->s.renderfx |= RF_NODRAW;
//			client->client->ps.pmove.origin[0] = gstate->level->intermission_origin[0]*8;
//			client->client->ps.pmove.origin[1] = gstate->level->intermission_origin[1]*8;
//			client->client->ps.pmove.origin[2] = gstate->level->intermission_origin[2]*8;
			client->client->ps.pmove.pm_type = PM_FREEZE;
//			client->client->ps.viewoffset.Zero();
			client->client->ps.blend[3] = 0;
		}

		return;
	}

	//	find up to 16 intermission spots
	for (numSpots = 0, ent = gstate->FirstEntity (); ent; ent = gstate->NextEntity (ent))
	{
		if (ent->className && !stricmp (ent->className, "info_player_intermission"))
		{
			spot[numSpots] = *ent;
			numSpots++;
		}
	}
	if (numSpots == 0)
	{
		ent = com->FindEntity ("info_player_start");
	}
	else
		ent = &spot[rand () % numSpots];

//	ent = com->FindEntity ("info_player_start");
	
	gstate->level->intermission_origin = ent->s.origin;

	userEntity_t *target = NULL;
	if( ent->target )
	{
		target = com->FindTarget( ent->target );
	}
	if( target != NULL )
	{
		CVector vDirection = target->s.origin - ent->s.origin;
		CVector vAngles;
		VectorToAngles( vDirection, vAngles );
		gstate->level->intermission_angle = vAngles;
	}
	else
	{
		gstate->level->intermission_angle = ent->s.angles;
	}

	for (i = 0; i < maxclients->value; i++)
	{
		client = &gstate->g_edicts [i + 1];

		if (!client->inuse)
			continue;

// SCG[1/29/00]: we want the client to see the scoreboard in all game modes
//        if(deathmatch->value)
			if( bShowStats == TRUE )
			{
			    client->client->showflags |= (SHOW_SCORES | SHOW_SCORES_NOW);	//	send update immediately
			}

		//	resurrect the client, but don't mess with their inventory
		//	or anything else
		if (client->deadflag != DEAD_NO)
			Client_Resurrect (client);
		else
		{
			userEntity_t	*weapon;

			//	don't show weapon
			weapon = (userEntity_t *) client->client->ps.weapon;
			if (weapon)
				weapon->s.renderfx |= RF_NODRAW;
		}

		client->client->ps.rdflags |= RDF_LETTERBOX;

		//	move client to intermission point
		Client_MoveToIntermission (client);

		gstate->LinkEntity (client);
	}
}

void	dll_BeginIntermission (char *nextMap)
{
	Client_BeginIntermission(nextMap, TRUE );
}
