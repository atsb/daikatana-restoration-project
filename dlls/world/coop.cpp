// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#include "dk_system.h"

#ifndef WIN32
#include <dlfcn.h>
#endif

#include "world.h"
#include "ai_info.h"
#include "ai_utils.h"
#include "ai_func.h"
#include "Sidekick.h"
#include "spawn.h"
#include "ai_save.h"
#include "ctf.h"
#include "coop.h"
#include "dk_array.h"

static userEntity_t *spawnPoints[3] = {NULL,NULL,NULL};

void CoopLevelStart()
{

	memset(&spawnPoints[0],0,3*sizeof(spawnPoints[0]));
}

//=============================================================================
// void CoopIsMainPlayer( userEntity_t *self )
// 
// Returns TRUE if this is the main player entity IN COOP MODE.
//=============================================================================
bool CoopIsMainPlayer( userEntity_t *self )
{
	if (!coop->value)
		return false;
	return (&gstate->g_edicts[1] == self);
}

//=============================================================================
// userEntity_t *CoopFindMainPlayer()
// 
// Returns "main" player for coop mode only.
//=============================================================================
userEntity_t *CoopFindMainPlayer()
{
	if (!coop->value)
		return NULL;
	userEntity_t *head = &gstate->g_edicts[1];
	playerHook_t *hook = AI_GetPlayerHook( head );
	if (hook)
		return head;
	return NULL;
}

//=============================================================================
// userentity_t *CoopFindPlayersBot( userentity_t *self )
// 
// Find the bot representing that player.
//=============================================================================
userEntity_t *CoopFindPlayersBot( userEntity_t *self )
{
	if (!coop->value)
		return NULL;
	playerHook_t	*hook = AI_GetPlayerHook( self );

	// Is a bot around for the current type.
	if (hook->iPlayerClass == PLAYERCLASS_MIKIKO)
		return AIINFO_GetMikiko();
	else if (hook->iPlayerClass == PLAYERCLASS_SUPERFLY)
		return AIINFO_GetSuperfly();
	return NULL;
}


//=============================================================================
// void CoopSelectCharacter( userEntity_t *self, char *userinfo )
// 
// Changes the user info to be that of one of the characters for the coop mode.
//	RULES:	1] No users in the game... HIRO.
//			2] Only 1 user in the game and one BOT, the bot.
//			3] Two user in the game... The non-used character.
//			4] No or two bots... Player selection if available.
//			5] otherwise SUPERFLY.
//=============================================================================
int COOP_PlayerCount()
{
	int count = 0;
	userEntity_t *p;
	for (int c = 0; c < maxclients->value; c++)
	{
		p = &gstate->g_edicts [c + 1];
		if (p && p->inuse && AI_GetPlayerHook(p))
			count++;
	}

	return count;
}

void COOP_DropClient(userEntity_t *self)
{
	gstate->WriteByte(SVC_DISCONNECT);
	gstate->UniCast(self,true);
}

void Client_InitClientModel ( userEntity_t *self );

void CoopSelectCharacter( userEntity_t *self, char *userinfo )
{
	if (!coop->value)
		return;

	short local = gstate->gi->IsPlayerLocal(self - gstate->g_edicts - 1);
	if (local == -1)
		return;

	int character = -1,pc = 0;
	char *modelname = "";
	char skin[MAX_QPATH];
	int skinNum = -1;

	// find the skin this player is using.
	char *s = Info_ValueForKey      ((char *)self->client->pers.userinfo, "skinname");    // get skin
	char *pos = strstr(s,"_bod_");
	if (pos)
	{
		pos += 5;	// increment past "_bod_";
		skinNum = atoi(pos);
	}

	if (gstate->GetCvar("dedicated"))		// doesn't really matter which character you get!
	{
		// What user are we?
		int c;
		for (c = 0; c < maxclients->value; c++)
		{
			if (self == &gstate->g_edicts [c + 1])
				break;
		}
		if (c >= 3)	// Bad!
		{
			COOP_DropClient(self);
			return;
		}

		switch( c )
		{
		default:
			// What should be done when non are available or invalid kk?
			// this!
			COOP_DropClient(self);
			return;
		case 0:
			pc = PLAYERCLASS_HIRO;
			character = 0;
			modelname = DEFAULT_CHARACTER_MODEL;
			if (skinNum != -1)
				Com_sprintf(skin,sizeof(skin),"skins/hiro_bod_%d.wal",skinNum);
			else
				Com_sprintf(skin,sizeof(skin),"%s",DEFAULT_HIRO_SKIN );
			break;
		case 1:
			pc = PLAYERCLASS_MIKIKO;
			character = 1;
			modelname = "models/global/m_mikiko.dkm";
			if (skinNum != -1)
				Com_sprintf(skin,sizeof(skin),"skins/miko_bod_%d.wal",skinNum);
			else
				Com_sprintf(skin,sizeof(skin),"%s",DEFAULT_MIKIKO_SKIN );
			break;
		case 2:
			pc = PLAYERCLASS_SUPERFLY;
			character = 2;
			modelname = "models/global/m_superfly.dkm";
			if (skinNum != -1)
				Com_sprintf(skin,sizeof(skin),"skins/sfly_bod_%d.wal",skinNum);
			else
				Com_sprintf(skin,sizeof(skin),"%s",DEFAULT_SUPERFLY_SKIN );
			break;
		}
	}
	else
	{
		if (local)
		{
			pc = PLAYERCLASS_HIRO;
			character = 0;
			modelname = DEFAULT_CHARACTER_MODEL;
			if (skinNum != -1)
				Com_sprintf(skin,sizeof(skin),"skins/hiro_bod_%d.wal",skinNum);
			else
				Com_sprintf(skin,sizeof(skin),"%s",DEFAULT_HIRO_SKIN );
		}
		else
		{
			userEntity_t *hiro,*mk,*sf;
			int _pc = -1;
			playerHook_t *phook = AI_GetPlayerHook(self);
			if (phook)
			{
				_pc = phook->iPlayerClass;
				phook->iPlayerClass = 0;
			}
			CoopFindPlayers(&hiro, &mk, &sf, false);
			if (_pc != -1)
			{
				phook->iPlayerClass = _pc;
			}

			int num = COOP_PlayerCount();
			if (hiro)
				num--;

			if (mk && sf)	// they both exist already?  argh.
			{
				COOP_DropClient(self);
				return;
			}
			else if (!mk)
			{
				pc = PLAYERCLASS_MIKIKO;
				character = 1;
				modelname = "models/global/m_mikiko.dkm";
				if (skinNum != -1)
					Com_sprintf(skin,sizeof(skin),"skins/miko_bod_%d.wal",skinNum);
				else
					Com_sprintf(skin,sizeof(skin),"%s",DEFAULT_MIKIKO_SKIN );
			}
			else
			{
				pc = PLAYERCLASS_SUPERFLY;
				character = 2;
				modelname = "models/global/m_superfly.dkm";
				if (skinNum != -1)
					Com_sprintf(skin,sizeof(skin),"skins/sfly_bod_%d.wal",skinNum);
				else
					Com_sprintf(skin,sizeof(skin),"%s",DEFAULT_SUPERFLY_SKIN );
			}
		}
	}
	playerHook_t *phook = AI_GetPlayerHook(self);
	if (phook)
		phook->iPlayerClass = pc;

	userEntity_t *hiro,*mikiko,*superfly;
	CoopFindPlayers( &hiro, &mikiko, &superfly, false);
	Info_SetValueForKey(userinfo, "character", va("%d",character)); 
	Info_SetValueForKey(userinfo, "modelname", modelname ); 
	Info_SetValueForKey(userinfo, "skinname",  skin);

	strncpy(self->client->pers.body_info.modelname,modelname,sizeof(self->client->pers.body_info.modelname)-1);
	strncpy(self->client->pers.body_info.skinname, skin,sizeof(self->client->pers.body_info.skinname)-1);
	self->client->pers.body_info.character = character;

    int playernum = self - gstate->g_edicts - 1; // get player number
	gstate->ConfigString (CS_PLAYERSKINS + playernum, va("%s\\%s\\%s\\%d", self->netname, modelname, skin, character) );

	// update the animations to the new character model.
	Client_InitClientModel ( self );
}


//=============================================================================
// void CoopTransferCharacter( userEntity_t *t, userEntity_t *f )
// 
// Transfers stuff from one player/bot to another player/bot - Duplicates the stuff.
//=============================================================================
void CoopTransferCharacter( userEntity_t *t, userEntity_t *f )
{
	t->health		= f->health;
	t->armor_val	= f->armor_val;
	t->armor_abs	= f->armor_abs;
	t->team			= f->team;
	t->s.angles		= f->s.angles;
	t->s.origin		= f->s.origin;

	playerHook_t *fhook = AI_GetPlayerHook( f );
	playerHook_t *thook = AI_GetPlayerHook( f );

	thook->base_power				= fhook->base_power;
	thook->base_attack				= fhook->base_attack;
	thook->base_speed				= fhook->base_speed;
	thook->base_acro				= fhook->base_acro;
	thook->base_vita				= fhook->base_vita;
									
	thook->power_boost				= fhook->power_boost;
	thook->attack_boost				= fhook->attack_boost;
	thook->speed_boost				= fhook->speed_boost;
	thook->acro_boost				= fhook->acro_boost;
	thook->vita_boost				= fhook->vita_boost;

	thook->power_boost_time			= fhook->power_boost_time;
	thook->attack_boost_time		= fhook->attack_boost_time;
	thook->speed_boost_time			= fhook->speed_boost_time;
	thook->acro_boost_time			= fhook->acro_boost_time;
	thook->vita_boost_time			= fhook->vita_boost_time;

	userInventory_t *inv = gstate->InventoryFirstItem( f->inventory );
	weaponInfo_t	*pweaponinfo;
	weapon_t		*pweapon;
	ammo_t			*pammo;
	while( inv )
	{
		if( inv->flags & ITF_WEAPON )
		{
			pweapon = (weapon_t *)inv;
			pammo = (ammo_t *)pweapon->ammo;
			if ((pweaponinfo = com->FindRegisteredWeapon( pweapon->name )))
			{
				pweaponinfo->give_func( t, pammo->count );
			}
		}
		inv = gstate->InventoryNextItem( f->inventory );
	}
}


//=============================================================================
// bool CoopIsOtherClose( userEntity_t *self, userEntity_t *other, float dist )
// 
//
//=============================================================================
bool CoopIsOtherClose( userEntity_t *self, userEntity_t *other, float maxdist )
{
    playerHook_t *hook = AI_GetPlayerHook( self );
	if ((self->health <= 0) || (other->health <= 0))
		return false;	// Can't work if dead.

	float dist = VectorDistance( self->s.origin, other->s.origin );

	if (dist > maxdist)
		return false;

	if ((dist > maxdist/2.0f) && !AI_IsLineOfSight( self, other ))
		return false;

	return true;
}

//=============================================================================
// bool CoopIsAllOthersClose( userEntity_t *self, float maxdist, char names[2] )
// 
// 
//=============================================================================
bool CoopIsAllOthersClose( userEntity_t *self, float maxdist, char **name1, char **name2 )
{
	char *name[2];
	int cnt = 0;
	userEntity_t *plr1 = NULL,*plr2 = NULL;
	char *plrname1 = NULL, *plrname2 = NULL;

	if (!coop->value)
		return true;

	userEntity_t *ent_hiro,*ent_mikiko,*ent_sfly;
	CoopFindPlayers(&ent_hiro, &ent_mikiko, &ent_sfly);

	if (self == ent_hiro)
	{
		plr1 = ent_mikiko;
		plrname1 = tongue_world[T_MIKIKO_IBIHARA];

		plr2 = ent_sfly;
		plrname2 = tongue_world[T_SUPERFLY_JOHNSON];
	}
	else if (self == ent_sfly)
	{
		plr1 = ent_mikiko;
		plrname1 = tongue_world[T_MIKIKO_IBIHARA];

		plr2 = ent_hiro;
		plrname2 = tongue_world[T_HIRO_MIYAMOTO];
	}
	else if (self == ent_mikiko)
	{
		plr1 = ent_hiro;
		plrname1 = tongue_world[T_HIRO_MIYAMOTO];

		plr2 = ent_sfly;
		plrname2 = tongue_world[T_SUPERFLY_JOHNSON];
	}

	if (!plr1 && !plr2)
		return true;

	if (plr1)
	{
		if (!CoopIsOtherClose( self, plr1, maxdist ))
			name[cnt++] = plrname1;
	}

	if (plr2)
	{
		if (!CoopIsOtherClose( self, plr2, maxdist ))
			name[cnt++] = plrname2;
	}

	if (name1)
		*name1 = (cnt > 0 ? name[0]:NULL);
	if (name2)
		*name2 = (cnt > 1 ? name[1]:NULL);
	return (cnt == 0);
}

void COOP_Message(char *message, float time)
{
	userEntity_t *hiro,*mikiko,*sfly;
	CoopFindPlayers(&hiro,&mikiko,&sfly);
	if (hiro && (hiro->flags & FL_CLIENT))
		gstate->centerprint( hiro, time, message );
	if (mikiko && (mikiko->flags & FL_CLIENT))
		gstate->centerprint( mikiko, time, message );
	if (sfly && (sfly->flags & FL_CLIENT))
		gstate->centerprint( sfly, time, message );
}

int CoopFindOtherPlayers( userEntity_t *self, userEntity_t **ppe1 = NULL, userEntity_t **ppe2 = NULL )
{
	int cnt = 0;
	userEntity_t *p = &gstate->g_edicts[1];
	for(int i=1;i<3;i++,p++)
	{
		if ((p != self) && p->inuse)
		{
			if (!cnt)
			{
				if (ppe1)
					*ppe1 = p;
				cnt = 1;
			}
			if (cnt)
			{
				if (ppe2)
					*ppe2 = p;
				return cnt;
			}
		}
	}
	return cnt;
}

#define ASSIGN_VAL(ent,val)		if (ent) *ent = val
void CoopFindPlayers(userEntity_t **hiro, userEntity_t **mikiko, userEntity_t **sfly, bool bAllowAI)
{
/*
	*hiro = NULL;
	*mikiko = NULL;
	*sfly = NULL;
*/
	ASSIGN_VAL(hiro,NULL);
	ASSIGN_VAL(mikiko,NULL);
	ASSIGN_VAL(sfly,NULL);

	playerHook_t	*hook;

	userEntity_t *p = &gstate->g_edicts[1];
	for(int i=0;i<3;i++,p++)
	{
		if (p && p->inuse && (hook = AI_GetPlayerHook( p )))
		{
			switch(hook->iPlayerClass)
			{
			case PLAYERCLASS_HIRO:
				ASSIGN_VAL(hiro,p);
				break;
			case PLAYERCLASS_MIKIKO:
				ASSIGN_VAL(mikiko,p);
				break;
			case PLAYERCLASS_SUPERFLY:
				ASSIGN_VAL(sfly,p);
				break;
			};
		}
	}

	if (bAllowAI)
	{
		if (mikiko && !(*mikiko))
			ASSIGN_VAL(mikiko,AIINFO_GetMikiko());
		if (sfly && !(*sfly))
			ASSIGN_VAL(sfly,AIINFO_GetSuperfly());
	}
}

typedef	(*func_t)(edict_t *);
void InventoryItemDrop(userEntity_t *self, userInventory_t *inv)
{
	// early out for a nodrop weapon.
	if ((inv->flags & (ITF_WEAPON|WFL_NODROP)) == (ITF_WEAPON|WFL_NODROP))
	{
		gstate->InventoryDeleteItem(self,self->inventory,inv);
		return;
	}

	userEntity_t *item;

	if (inv->flags & ITF_WEAPON)
	{
		weaponInfo_t *winfo = com->FindRegisteredWeapon (inv->name);
		if (winfo)
		{
			// is there any ammo?
			int ammo_count = 0;
			ammo_t *ammo = (ammo_t *) gstate->InventoryFindItem (self->inventory, winfo->ammoName);
			if ( ammo )
			{
				ammo->command ((userInventory_t *) ammo, "count", &ammo_count);
			}

			if (ammo && ammo_count)
			{
				item = gstate->SpawnEntity ();
				item->className = winfo->weaponName;
				item->netname = winfo->weaponNetName;
				item->spawnflags = (DROPPED_ITEM|DROPPED_PLAYER_ITEM);

				//	set origin
				item->s.origin = self->s.origin;
				//	give some velocity
				item->velocity.x = rnd() * 400.0 - 200.0;
				item->velocity.y = rnd() * 400.0 - 200.0;
				item->velocity.z = rnd() * 250.0 + 250.0;

				winfo->spawn_func (winfo, item, -1.0, NULL);

				//	find out how much ammo player has for this weapon,
				//	and store it in the weapon pickup item
				item->hacks_int = ammo_count;
				item->clipmask = MASK_PLAYERSOLID;
				item->solid = SOLID_BBOX;
				item->movetype = MOVETYPE_BOUNCE;
				gstate->LinkEntity(item);
			}
		}
	}
	else
	{
		func_t func = (func_t)com->FindSpawnFunction( inv->name );
		if (func != NULL)
		{
			item = gstate->SpawnEntity ();
			item->className = inv->name;

			//	set origin
			item->s.origin = self->s.origin;
			//	give some velocity
			item->velocity.x = 100 * crand();
			item->velocity.y = 100 * crand();
			item->velocity.z = rnd() * 250.0 + 250.0;

			func(item);

			item->spawnflags = (DROPPED_ITEM|DROPPED_PLAYER_ITEM);
			//	find out how much ammo player has for this weapon,
			//	and store it in the weapon pickup item
			item->clipmask = MASK_PLAYERSOLID;
			item->movetype = MOVETYPE_BOUNCE;
			item->solid = SOLID_BBOX;
			gstate->LinkEntity(item);
		}
	}

	gstate->InventoryDeleteItem(self,self->inventory,inv);
}

void InventoryMove(userEntity_t *to, userEntity_t *from)
{
	if (!to->inventory || !from->inventory)
		return;

	userInventory_t *inv;
	while(inv = gstate->InventoryFirstItem(from->inventory))
	{
		if (inv->flags & ITF_COOP_REMOVEALL)
		{
			inv->flags &= ~ITF_COOP_REMOVEALL;
		}
		else
		{
			gstate->InventoryAddItem(to,to->inventory,inv);

		}
		gstate->InventoryDeleteItem(from,from->inventory,inv);
	}

	// send the change to applicable entityies
	if (to->flags & FL_CLIENT)
	{
		gstate->WeaponDisplayUpdateClient(to, true);
		gstate->InventoryUpdateClient(to,false);
	}
	if (from->flags & FL_CLIENT)
	{
		gstate->WeaponDisplayUpdateClient(from, true);
		gstate->InventoryUpdateClient(from,false);
	}
}

void pack_remove(userEntity_t *self)
{
	if (self->inventory)
		gstate->InventoryFree(self->inventory);
	self->inventory = NULL;
	gstate->RemoveEntity(self);
}

void pack_touch(userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!(other->flags & FL_CLIENT))
		return;

	// no more touching dammit!
	self->touch = NULL;
	
	// give the player touching the stuff in the pack
	InventoryMove(other,self);

	// play a cool sound!
	int episode = gstate->episode;
	if ((episode > 4) || (episode < 1))
		episode = 1;

	char buffer[30];
	Com_sprintf(buffer,sizeof(buffer),"e%d/e%d_secret.wav",episode,episode);
	gstate->StartEntitySound( other, CHAN_AUTO, gstate->SoundIndex(buffer),1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );

	self->remove(self);
}


//=============================================================================
// void COOP_AddInventoryItem(userEntity_t *self, userInventory_t *data)
// 
// self is the player who will be getting the item added to their inventory anyway
//=============================================================================
void COOP_AddInventoryItem(userEntity_t *self, userInventory_t *data)
{
	if (!coop->value)
		return;

	userEntity_t *hiro,*mikiko,*sfly;
	CoopFindPlayers(&hiro, &mikiko, &sfly, false);

	if (hiro && (hiro != self) && hiro->inventory)
	{
		gstate->InventoryAddItem(hiro,hiro->inventory,data);
	}

	if (mikiko && (mikiko != self) && mikiko->inventory)
	{
		gstate->InventoryAddItem(mikiko,mikiko->inventory,data);
	}

	if (sfly && (sfly != self) && sfly->inventory)
	{
		gstate->InventoryAddItem(sfly,sfly->inventory,data);
	}
}
userEntity_t *COOP_SpawnPack(userEntity_t *self)
{
	// set up the entity
	userEntity_t *pack	= gstate->SpawnEntity ();
	pack->className			= "item_pack";
	pack->modelName			= "models/global/dt_bpack.dkm";
	pack->s.modelindex		= gstate->ModelIndex (pack->modelName);
	pack->clipmask			= MASK_PLAYERSOLID;
	pack->movetype			= MOVETYPE_BOUNCE;
	pack->solid				= SOLID_TRIGGER;
	pack->s.origin			= self->s.origin;
	pack->s.renderfx		= RF_MINLIGHT;
	pack->s.effects			= EF_ROTATE;
	pack->flags				|= FL_NOSAVE;

	// set up the funcs
	pack->remove			= pack_remove;
	pack->touch				= pack_touch;

	// set up and copy the inventory
	pack->inventory = gstate->InventoryNew (MEM_PERMANENT);
	InventoryMove(pack,self);

	// set the size and link!
	gstate->SetSize(pack, -16,-16,-16,16,16,16);
	gstate->LinkEntity(pack);

	return pack;
}

//=============================================================================
// void COOP_ClientDisconnect(userEntity_t *self)
// 
// Drops and spawns all items in the player's inventory
//=============================================================================
void Client_RemoveWeapon(userEntity_t *self);
void InitClientPersistant (gclient_t *client);

void COOP_ClientDisconnect(userEntity_t *self)
{
	if (!coop->value)
		return;

	Client_RemoveWeapon(self);
	if (!self->inventory)
		return;

	COOP_SpawnPack(self);
	InitClientPersistant(self->client);

	self->curWeapon = NULL;
	self->inuse = false;
}

void COOP_LevelExit()
{
}

#define SHOWSPAWNS 1
#define MAX_COOP_SPAWN		18
userEntity_t *COOP_GetSpawnPoint(userEntity_t *self)
{
	if (!coop->value)
		return NULL;

	char *spawnName = gstate->game->spawnpoint;


	userEntity_t *ent;
	int i = 0;
	if (!spawnPoints[0] && !spawnPoints[1] && !spawnPoints[2])
	{
		// find all of the info_player_coops in the map and sort by their targetnames
		userEntity_t *entarr[MAX_COOP_SPAWN];
		int cnt = GetAllEntities( "info_player_coop", entarr, MAX_COOP_SPAWN );
		if (!cnt)
			return NULL;

#ifdef SHOWSPAWNS
		for (i = 0; i < cnt; i++)
		{
			if (!(ent = *(entarr + i)))
				continue;

			gstate->Con_Dprintf ("COOP: info_player_coop(%d) = %s\n", i,ent->targetname);
		}
#endif
		int tries = 2;
		int c;
		bool success = false;
		while (tries && !success)
		{
			success = false;
			c = 0;
			// try to find the appropriate entries in the array
			for (i = 0; ((i < cnt) && (c < 3)); i++)
			{
				if (!(ent = *(entarr + i)))
					continue;

				if ( (!strlen(spawnName) && (!ent->targetname || !strlen(ent->targetname))) || 
					 (ent->targetname && !stricmp(spawnName,ent->targetname)))
				{
					spawnPoints[c++] = ent;
					success = true;
					gstate->Con_Dprintf ("COOP(%d): Found spawn point %d: %s\n", tries - 1,c, spawnName);
				}
			}
			
			// if we didn't find an appropriate spawn point, find the 'earliest' one.  NULL, from_a, from_b, ...
			if (!success && (tries == 2))
			{
				spawnName = NULL; 
				// find the 'lowest' value in the list.
				for (i = 0; i < cnt; i++)
				{
					if (!(ent = *(entarr + i)))
						continue;

					if (!spawnName)
						spawnName = ent->targetname;
					else
					{
						if (!ent->targetname || (ent->targetname && !strlen(ent->targetname)))	// null or empty
						{
							spawnName = "";
							break;
						}
						else if (stricmp(ent->targetname,spawnName) < 0)
							spawnName = ent->targetname;
					}
				}
			}
			tries--;
		}

		if (!success)
			return NULL;
	}

	CVector mins(-16, -16, -24),maxs(16, 16, 32);		// the client ent info isn't initialized yet..
	trace_t tr;
	CVector bottomPoint, endPoint;
	for (i = 0; ((i < 3) && spawnPoints[i]); i++)
	{
		endPoint = spawnPoints[i]->s.origin;
		endPoint.z += 16;

        tr = gstate->TraceBox_q2( endPoint, mins, maxs, endPoint, self, MASK_SHOT );
	    if ( tr.fraction >= 1.0f && !tr.allsolid && !tr.startsolid)// && !tr.ent )
	    {
		    return spawnPoints[i];
	    }
	}

	// uhh, hmmm.  just return the first non-null spawn point in the list...
	for (i = 0; i < 3; i++)
	{
		if (spawnPoints[i]) return spawnPoints[i];
	}

	// oh dear....
	return NULL;
}

void give_me_an_antidote(userEntity_t *self)
{
	playerHook_t *phook = AI_GetPlayerHook(self);
	if (!phook || !self->inventory)
		return;

	userInventory_t *invItem = gstate->InventoryCreateItem( 
		self->inventory, 
		"item_antidote",
		NULL,
		NULL,
		gstate->ModelIndex("models/global/a_antidote.dkm"),
		ITF_SPECIAL | ITF_INVSACK,
		sizeof( userInventory_t ) );

	gstate->InventoryAddItem (self, self->inventory, invItem);
				
	phook->items |= IT_ANTIDOTE;
	if (self->client)
		self->client->pers.fAntidotePoints = 100.0f;

	// count these...only need one of em
	userInventory_t *antidote = NULL;
	while (gstate->InventoryItemCount(self->inventory,"item_antidote") > 1)
	{
		antidote = gstate->InventoryFindItem(self->inventory,"item_antidote");
		gstate->InventoryDeleteItem(self,self->inventory,antidote);
	}
}

void COOP_SpawnHacks(userEntity_t *self)
{
	if (!coop->value)
		return;

	switch(gstate->episode)
	{
	case 1:
		{

		}
		break;
	case 2:
		{
			// check for slime and give antidote if necessary
			if (gstate->PointContents(self->s.origin) & CONTENTS_SLIME)
			{
				give_me_an_antidote(self);
			}
		}
		break;
	case 3:
		{

		}
		break;
	case 4:
		{

		}
		break;
	default:
		return;
	};
}

void coop_register_func()
{
	gstate->RegisterFunc("pack_remove",pack_remove);
	gstate->RegisterFunc("pack_touch",pack_touch);
}

