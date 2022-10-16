//////////////////////////////////////////////////////////////////////
//	artifacts.cpp
//
//	routines for handling pick up items and power ups
//
//////////////////////////////////////////////////////////////////////

#include	"world.h"
#include "ai_utils.h"
#include "coop.h"

///////////////////////////////////////////////////////////////////////////////
//	exports
///////////////////////////////////////////////////////////////////////////////

DllExport void item_health_25 (userEntity_t *self);
DllExport void item_health_50 (userEntity_t *self);
DllExport void item_goldensoul (userEntity_t *self);
DllExport void item_wraithorb (userEntity_t *self);

DllExport void item_power_boost (userEntity_t *self);
DllExport void item_attack_boost (userEntity_t *self);
DllExport void item_speed_boost (userEntity_t *self);
DllExport void item_acro_boost (userEntity_t *self);
DllExport void item_vita_boost (userEntity_t *self);

DllExport void item_plasteel_armor (userEntity_t *self);
DllExport void item_chromatic_armor (userEntity_t *self);
DllExport void item_silver_armor (userEntity_t *self);
DllExport void item_gold_armor (userEntity_t *self);
DllExport void item_chainmail_armor (userEntity_t *self);
DllExport void item_black_adamant_armor (userEntity_t *self);
DllExport void item_kevlar_armor (userEntity_t *self);
DllExport void item_ebonite_armor (userEntity_t *self);

DllExport void item_sulphur (userEntity_t *self);
DllExport void item_charcoal (userEntity_t *self);
DllExport void item_saltpeter (userEntity_t *self);
DllExport void item_bottle (userEntity_t *self);

DllExport void item_crypt_key (userEntity_t *self);
DllExport void item_wyndrax_key (userEntity_t *self);

// SCG[12/5/99]: DllExport void item_evilbook (userEntity_t *self);
DllExport void item_spellbook (userEntity_t *self);
//NSS[11/16/99]:A "No no" do not export functions twice
//DllExport void item_keycard_cell (userEntity_t *self);

DllExport void view_rotate(userEntity_t *self);

DllExport void item_savegem(userEntity_t *self);

DllExport void item_control_card_red( userEntity_t *self );
DllExport void item_control_card_blue( userEntity_t *self );
DllExport void item_control_card_yellow( userEntity_t *self );
DllExport void item_control_card_green( userEntity_t *self );
DllExport void item_keycard_cell (userEntity_t *self);
DllExport void item_hex_keystone( userEntity_t *self );
DllExport void item_quad_keystone( userEntity_t *self );
DllExport void item_trigon_keystone( userEntity_t *self );

DllExport void item_megashield( userEntity_t *self );
DllExport void item_invincibility( userEntity_t *self );
DllExport void item_antidote( userEntity_t *self );
DllExport void item_drachma( userEntity_t *self );
DllExport void item_horn( userEntity_t *self );
DllExport void item_rune_a( userEntity_t *self );
DllExport void item_rune_e( userEntity_t *self );
DllExport void item_rune_g( userEntity_t *self );
DllExport void item_rune_i( userEntity_t *self );
DllExport void item_rune_s( userEntity_t *self );
DllExport void item_purifier_shard1( userEntity_t *self );
//DllExport void item_purifier_shard2( userEntity_t *self );
DllExport void item_purifier_shard2_1( userEntity_t *self );
DllExport void item_purifier_shard2_2( userEntity_t *self );
DllExport void item_purifier_shard2_3( userEntity_t *self );
DllExport void item_purifier_shard2_4( userEntity_t *self );
DllExport void item_purifier_shard2_5( userEntity_t *self );
DllExport void item_purifier_shard3( userEntity_t *self );
DllExport void item_black_chest( userEntity_t *self );
DllExport void item_wood_chest( userEntity_t *self );
// SCG[12/5/99]: DllExport void item_ice_boots( userEntity_t *self );
DllExport void item_ring_of_fire( userEntity_t *self );
DllExport void item_ring_of_undead( userEntity_t *self );
DllExport void item_ring_of_lightning( userEntity_t *self );
DllExport void item_envirosuit( userEntity_t *self );

void item_bomb_part_check_for_all( userEntity_t *self );
void item_rune_check_for_all( userEntity_t *self );
void item_keystone_check_for_all( userEntity_t *self );
void item_purifier_check_for_all( userEntity_t *self );

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////
#define	IS_INSTAGIB				(deathmatch->value && dm_instagib->value)


#define ARMOR_RESPAWN_TIME	30.0

#define BOOST_ACTIVE_TIME   30.0 // 30.0
#define BOOST_RESPAWN_TIME  60.0 // 60.0

#define	ITEM_BOOSTBASEHEALTH	0x00000001

#define	KEY_USE_ONCE		0x0001		// use a key once and then it deletes itself from the inventory
#define	KEY_UNLOCK			0x0002		// use a key and it removes the targetname and the keyname on a door
										// so it can now be opened freely
#define	KEY_USETARGETS		0x0004		// use the targets when the key item is picked up



// armor defines  (TTD: move to armor.csv)
// cek[2-19-00] changed some absorption values per romero's request
#define ARMOR_MEGASHIELD_MAXHEALTH	   (400)
#define ARMOR_MEGASHIELD_ABSORPTION    (75) //%

#define	ARMOR_PLASTEEL_MAXHEALTH       (200)
#define ARMOR_PLASTEEL_ABSORPTION      ( 75) // %

#define	ARMOR_CHROMATIC_MAXHEALTH      (100)
#define	ARMOR_CHROMATIC_ABSORPTION     ( 50) // %

#define	ARMOR_SILVER_MAXHEALTH         (150)
#define	ARMOR_SILVER_ABSORPTION        ( 65) // %

#define	ARMOR_GOLD_MAXHEALTH           (200)
#define	ARMOR_GOLD_ABSORPTION          ( 75) // %

#define	ARMOR_CHAINMAIL_MAXHEALTH      (125)
#define	ARMOR_CHAINMAIL_ABSORPTION     ( 50) // %

#define	ARMOR_BLACK_ADAMANT_MAXHEALTH  (250)
#define	ARMOR_BLACK_ADAMANT_ABSORPTION ( 80) // %

#define	ARMOR_KEVLAR_MAXHEALTH         (100)
#define	ARMOR_KEVLAR_ABSORPTION        ( 40) // %

#define	ARMOR_EBONITE_MAXHEALTH        (200)
#define	ARMOR_EBONITE_ABSORPTION       ( 75) // %




//int FindClient(userEntity_t *self, int Distance);// SCG[1/23/00]: not used

///////////////////////////////////////////////////////////////////////////////
//	typedef
///////////////////////////////////////////////////////////////////////////////
#define ITEM_RESP_COOP_STAY				0X0001		// item is persistant in coop play
#define ITEM_RESP_COOP_REMOVEALL		0x0002		// item is removed from all players in coop
#define ITEM_RESP_USE_ONCE				0X0004		// OK So I'm hacking a bit...

#define	ITEMINFO(x) (int)&(((itemInfo_t *)0)->x)
field_t item_info_fields[] = 
{
	{"respawnFlags",		ITEMINFO(respawnFlags),			F_SHORT},
	{"health",				ITEMINFO(health),				F_INT},
	{"respawnTime",			ITEMINFO(respawnTime),			F_INT},
	{"frame_start",			ITEMINFO(frame_start),			F_INT},
	{"frame_end",			ITEMINFO(frame_end),			F_INT},

	{"frame_time",			ITEMINFO(frame_time),			F_FLOAT},
	{"render_scale",		ITEMINFO(render_scale),			F_FLOAT},

	{"flags",				ITEMINFO(flags),				F_INT},

	{"className",			ITEMINFO(className),			F_LSTRING},
	{"netName",				ITEMINFO(netName),				F_LSTRING},
	{"episodeInde",			ITEMINFO(episodeIndependentModelName),			F_LSTRING},

	{"soundPickup",			ITEMINFO(soundPickup),			F_LSTRING},
	{"soundRespawn",		ITEMINFO(soundRespawn),			F_LSTRING},
	{"soundAmbient",		ITEMINFO(soundAmbient),			F_LSTRING},
	{"soundSpecial",		ITEMINFO(soundSpecial),			F_LSTRING},

	{"soundAmbientVolume",	ITEMINFO(soundAmbientVolume),	F_FLOAT},
	{"soundDist_min",		ITEMINFO(soundDist_min),		F_FLOAT},
	{"soundDist_max",		ITEMINFO(soundDist_max),		F_FLOAT},

	{"touchFunc",			ITEMINFO(touchFunc),			F_FUNC},

	
	{"mins",				ITEMINFO(mins),					F_VECTOR},
	{"maxs",				ITEMINFO(maxs),					F_VECTOR},
	{NULL, 0, F_INT}
};

#define	ITEMHOOK(x) (int)&(((itemHook_t *)0)->x)
field_t item_hook_fields[] = 
{
	{"respawnTime",			ITEMHOOK(respawnTime),			F_FLOAT},
	{"health",				ITEMHOOK(health),				F_FLOAT},
	{"link",				ITEMHOOK(link),					F_EDICT},
	{"info",				ITEMHOOK(info),					F_RESTORE_AS_NULL},
	{"soundPickupIndex",	ITEMHOOK(soundPickupIndex),		F_INT},
	{"soundRespawnIndex",	ITEMHOOK(soundRespawnIndex),	F_INT},
	{"soundSpecialIndex",	ITEMHOOK(soundSpecialIndex),	F_INT},
	{"flags",				ITEMHOOK(flags),				F_INT},
	{NULL, 0, F_INT}
};

/*
typedef struct	keyItem_s
{
	//	!!!! FIELDS BELOW MUST BE IDENTICAL TO userInventory_t!!!!
	userInventory_t	invRec;
	//	!!!! FIELDS ABOVE MUST BE IDENTICAL TO userInventory_t!!!!
	char	target[64];	// entity this key/book is used upon
	char	netname[64];	// entity net name	
	int		soundIndex;		// sound to play when used
} keyItem_t;
*/


///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

// NSS[2/9/00]:So we can find sidekicks...
userEntity_t *AIINFO_GetSuperfly();
userEntity_t *AIINFO_GetMikikofly();
userEntity_t *AIINFO_GetMikiko();


// NSS[1/11/00]:Checks items in the item lists and removes them
void SIDEKICK_RemoveItem(userEntity_t *Item);

void Item_SaveInfo( FILE *f, itemHook_t *hook, field_t *hook_fields, int nSize )
{
	field_t			*field;
	void			*info = gstate->X_Malloc( nSize, MEM_TAG_MISC );

	// SCG[11/21/99]: Allocate temp memory for hook
	memcpy( info, hook->info, nSize );

	// change the pointers to lengths or indexes
	for( field = hook_fields; field->name; field++ )
	{
		com->AI_WriteSaveField1( f, field, ( byte * ) info );
	}

	// write the block
	com->AI_SaveHook( f, info, nSize );

	// now write any allocated data following the edict
	for( field = hook_fields; field->name; field++ )
	{
		com->AI_WriteSaveField2( f, field, ( byte * ) hook->info );
	}

	// SCG[11/21/99]: Free temp memory
	gstate->X_Free( info );
}

void Item_LoadInfo( FILE *f, itemHook_t *hook, field_t *hook_fields, int nSize )
{
	field_t		*field;

	hook->info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);

	com->AI_LoadHook( f, hook->info, nSize );

	for( field = hook_fields; field->name; field++ )
	{
		com->AI_ReadField (f, field, (byte *)hook->info );
	}
}
void item_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, item_hook_fields, sizeof( itemHook_t ) );
	Item_SaveInfo(f,(itemHook_t *)ent->userHook, item_info_fields, sizeof(itemInfo_t));
}

void item_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, item_hook_fields, sizeof( itemHook_t ) );
	Item_LoadInfo(f,(itemHook_t *)ent->userHook, item_info_fields, sizeof(itemInfo_t));
}

///////////////////////////////////////////////////////////////////////////////////
//	isVowel
//
///////////////////////////////////////////////////////////////////////////////////

int	isVowel (char ch)
{
	if (ch == 'a' || ch == 'A')
		return	TRUE;
	else
	if (ch == 'e' || ch == 'E')
		return	TRUE;
	else
	if (ch == 'i' || ch == 'I')
		return	TRUE;
	else
	if (ch == 'o' || ch == 'O')
		return	TRUE;
	else
	if (ch == 'u' || ch == 'U')
		return	TRUE;

	return	FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//	item_NameForEpisode
//
//	returns the model name filled in with episode numbers
///////////////////////////////////////////////////////////////////////////////

static	char	itemName [MAX_QPATH];

char	*item_NameForEpisode (char *name)
{
	int	episode = (int) sv_episode->value;
//	sprintf (itemName, "models\\e%i\\a%i_%s", episode, episode, name);
	Com_sprintf (itemName, sizeof(itemName),"models\\e%i\\a%i_%s", episode, episode, name);
	return	itemName;
}

void item_setSound(userEntity_t *self, itemInfo_t *info)
{
	if (!self || !info)
		return;

	if (info->soundAmbient != NULL)
	{
		self->s.sound		= self->soundAmbientIndex = gstate->SoundIndex (info->soundAmbient);
		self->s.volume		= (info->soundAmbientVolume) ? info->soundAmbientVolume : 1.0;
		self->s.dist_max	= (info->soundDist_max) ? info->soundDist_max : ATTN_NORM_MAX;
		self->s.dist_min	= (info->soundDist_min) ? info->soundDist_min : ATTN_NORM_MIN;
	}
	else
		self->soundAmbientIndex = 0;

}

// ----------------------------------------------------------------------------
// NSS[3/4/00]:
// Name:		Item_Respawn
// Description:The actual function which respawns the item.
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void	Item_Respawn (userEntity_t *self)
{
	itemHook_t		*ihook = (itemHook_t *) self->userHook;
	if (deathmatch->value || !(deathmatch->value))//FIXME: no respawn unless deathmatch
	{
		self->s.sound		= self->soundAmbientIndex;
		self->s.modelindex	= gstate->ModelIndex(self->modelName);
	
		self->solid			= SOLID_TRIGGER;
		self->nextthink		= -1;

		self->s.effects		|= EF_RESPAWN;	//	tell client to start respawn effect
		self->s.renderfx	|= RF_GLOW;

		gstate->LinkEntity(self);
		if(ihook)
		{
			// NSS[3/4/00]:Set the respawn sound.
			if(ihook->soundRespawnIndex)
				gstate->StartEntitySound( self, CHAN_AUTO,ihook->soundRespawnIndex,1.0f,256.0f, 512.0f);	
		}
	}
}

// ----------------------------------------------------------------------------
// NSS[3/4/00]:
// Name:		Item_SetRespawn
// Description:Generic function to set the respawning of items.
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void Item_SetRespawn(userEntity_t *self)
{
	self->s.modelindex = 0;	//	set model to 0, modelName should still be valid, though
	self->solid = SOLID_NOT;
	
	itemHook_t		*ihook = (itemHook_t *) self->userHook;
	
	//	shut off looping ambient sounds
	if (self->s.sound > 0)
	{
		self->soundAmbientIndex = self->s.sound;
		self->s.sound = 0;
	}
	else
		self->soundAmbientIndex = 0;


	if (ihook && deathmatch->value && ihook->respawnTime > 0.0f)
	{
		// NSS[3/4/00]:Set the respawn value for the item.
		self->nextthink = gstate->time + ihook->respawnTime;
		self->think = Item_Respawn;
	}
	else
		self->remove (self);
}


///////////////////////////////////////////////////////////////////////////////
//	item_Spawn
//
//	spawns an artifact using the passed model name
///////////////////////////////////////////////////////////////////////////////

void item_Spawn (userEntity_t *self, itemInfo_t *info, movetype_t moveType = MOVETYPE_TOSS, qboolean bRotate = FALSE, char *modelName = NULL)
{
	itemHook_t	*ihook;
	float		speed;

	self->netname = info->netName;
	self->netNameID = info->netNameID;

	self->userHook = (itemHook_t*)gstate->X_Malloc(sizeof(itemHook_t),MEM_TAG_HOOK);
	ihook = (itemHook_t *) self->userHook;
	
	self->solid = SOLID_TRIGGER;
	self->movetype = moveType;
	self->s.renderfx = RF_GLOW;
	self->clipmask = MASK_PLAYERSOLID;
	self->flags = FL_ITEM;
	self->svflags |= ( SVF_ITEM | SVF_NOPUSHCOLLIDE );	// amw: causes items in a cabinet to not collide with cabinet doors etc..
	
	if (bRotate)
	{
		self->s.effects |= EF_ROTATE;

		for (int i = 0; ( self->epair != NULL ) && ( self->epair[i].key != NULL ); i++) 
		{
			// PITCH YAW ROLL
			if (!stricmp(self->epair[i].key, "x_speed")) 
			{
				//	if EF_ROTATE is set, it dictates how fast the entity will rotate on the x axis
				speed = atof(self->epair[i].value);
				speed = speed / 20;
				if (speed < 0)
				{
					if (speed < -179)
						speed = -179;
					self->s.angle_delta.z = 180 - abs(speed);  // encoded..negatives are 0-179 positives are 180-360
				}
				else
				{
					if (speed > 179)
						speed = 179;
					self->s.angle_delta.z = 180 + speed;
				}

			}
			else if (!stricmp(self->epair[i].key, "y_speed")) 
			{
				//	if EF_ROTATE is set, it dictates how fast the entity will rotate on the y axis
				speed = atof(self->epair[i].value);
				speed = speed / 20;
				if (speed < 0)
				{
					if (speed < -179)
						speed = -179;
					self->s.angle_delta.x = 180 - abs(speed);	// encoded..negatives are 0-179 positives are 180-360
				}
				else
				{
					if (speed > 179)
						speed = 179;
					self->s.angle_delta.x = 180 + speed;
				}
			}
			else if (!stricmp(self->epair[i].key, "z_speed")) 
			{
				//	if EF_ROTATE is set, it dictates how fast the entity will rotate on the z axis
				speed = atof(self->epair[i].value);
				speed = speed / 20;
				if (speed < 0)
				{
					if (speed < -179)
						speed = -179;
					self->s.angle_delta.y = 180 - abs(speed);	// encoded..negatives are 0-179 positives are 180-360
				}
				else
				{
					if (speed > 179)
						speed = 179;
					self->s.angle_delta.y = 180 + speed;
				}
			}
		}
	}
	else
	{
		self->s.effects = 0;
	}
	// set the touch function
	self->touch = info->touchFunc;
	
	// SCG[11/24/99]: Save game stuff
	self->save = item_hook_save;
	self->load = item_hook_load;

	if (modelName)
	{
		self->modelName = modelName;
	}
	else
	{
		char *pStaticName = item_NameForEpisode (info->episodeIndependentModelName);
//		self->modelName = new char[strlen( pStaticName ) + 1];	
		self->modelName = (char*)gstate->X_Malloc(sizeof(char)*(strlen(pStaticName)+1),MEM_TAG_LEVEL);
        strcpy(self->modelName, pStaticName); 
	}
	
	self->s.modelindex = gstate->ModelIndex (self->modelName);
	self->s.mins = info->mins;
	self->s.maxs = info->maxs;



	ihook->health = info->health;

    if (deathmatch->value && dm_item_respawn->value)  // 2.28 dsn
	  ihook->respawnTime = info->respawnTime;
    else
      ihook->respawnTime = -1.0f;

	ihook->info = info;
	ihook->flags = info->flags;
	
	ihook->soundPickupIndex = gstate->SoundIndex (info->soundPickup);
	ihook->soundRespawnIndex = gstate->SoundIndex (info->soundRespawn);
	ihook->soundSpecialIndex = gstate->SoundIndex (info->soundSpecial);

	item_setSound(self,info);

	// animated item
	if (info->frame_start || info->frame_end)
	{
	    self->s.frame = info->frame_start;
		self->s.frameInfo.startFrame = info->frame_start;
		self->s.frameInfo.endFrame = info->frame_end;
		self->s.frameInfo.frameFlags = FRAME_LOOP|FRAME_FORCEINDEX; 
		self->s.frameInfo.frameInc = 1;
		self->s.frameInfo.frameTime = info->frame_time;
		self->s.frameInfo.frameState = FRSTATE_STARTANIM;
		self->s.frameInfo.next_frameTime = 0;
	}

	// resizing
	if (info->render_scale)
	{
		self->s.render_scale.Set(info->render_scale,info->render_scale,info->render_scale);
	}
	else
	{
		self->s.render_scale.Set( 1.0, 1.0, 1.0 );
	}
	//NSS[12/1/99]:Last thing we always do is link the entity.
	//	origin is already set in physics when spawning this entity
	gstate->LinkEntity (self);
}

void artifact_special_print(int EnumType, userEntity_t *receiver, char *format, int form_count, char ** tongue, int NetNameID)

{
		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierStr(format,form_count);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue,EnumType));
		gstate->cs.SendStringID(NetNameID);
		gstate->cs.Unicast(receiver,CS_print_center,2.0);
		gstate->cs.EndSendString();

}


///////////////////////////////////////////////////////////////////////////////
// item_health_touch
///////////////////////////////////////////////////////////////////////////////

void	item_health_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!com->ValidTouch (self, other))
		return;

	playerHook_t	*ohook = AI_GetPlayerHook( other );
	itemHook_t		*ihook = (itemHook_t *) self->userHook;

	// NSS[1/11/00]:Check for client picking up items
	if(other->flags & (FL_CLIENT|FL_BOT))
		SIDEKICK_RemoveItem(self);

	if (ihook->flags & ITEM_BOOSTBASEHEALTH)
	{
        if(other->health <= ohook->base_health + ihook->health)
        {
		    com->Health (other, ihook->health, ohook->base_health + ihook->health);
            ohook->fLastHealthTick = gstate->time;
        }
	}
	else 
	{
		//	only boost health up to base health
		if( ( other->health + 0.5 ) >= ohook->base_health)
			return;

		com->Health (other, ihook->health, ohook->base_health);
	}

	//	play pickup sound
	gstate->StartEntitySound(other, CHAN_VOICE, ihook->soundPickupIndex, 0.85f, ohook->fMinAttenuation, ohook->fMaxAttenuation);

	if (other->flags & FL_CLIENT)
	{
		other->client->bonus_alpha = 0.25;
		int Type = T_PICKUP_HEALTH;
		
		if (ihook->info->flags & ITEM_BOOSTBASEHEALTH)
		{
			Type = T_PICKUP_BOOST;
		}
		// NSS[6/23/00]:New print stuff
		artifact_special_print(Type, other, "%s %s.\n", 2, tongue, self->netNameID);
    }
	
	// call respawn last because it can remove the entity
	//com->SetRespawn (self, ihook->respawnTime);
	// NSS[3/4/00]:
	Item_SetRespawn(self);

	if ( other->PickedUpItem )
	{
		other->PickedUpItem( other, self->className );
	}

}

///////////////////////////////////////////////////////////////////////////////
// item_health_25
///////////////////////////////////////////////////////////////////////////////
void	item_health_25 (userEntity_t *self)
{
	itemInfo_t	*info;

	if (IS_INSTAGIB || (deathmatch->value && !dm_allow_health->value))
	  return;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;

	// cek[1-3-99] added health vial for episode 3
	//	info->netName = tongue[T_HEALTH_KIT];
	switch(gstate->episode)
	{
	default:
	case 1:
		info->netName = tongue[T_HEALTH_KIT];
		info->netNameID = RESOURCE_ID_FOR_STRING(tongue,T_HEALTH_KIT);
		break;
	case 2:
		info->netName = tongue[T_HEALTH_E2_VASE];
		info->netNameID = RESOURCE_ID_FOR_STRING(tongue,T_HEALTH_E2_VASE);
		break;
	case 3:
		info->netName = tongue[T_HEALTH_E3_FLASK];
		info->netNameID = RESOURCE_ID_FOR_STRING(tongue,T_HEALTH_E3_FLASK);
		break;
	case 4:
		info->netName = tongue[T_HEALTH_E4_BOX];
		info->netNameID = RESOURCE_ID_FOR_STRING(tongue,T_HEALTH_E4_BOX);
		break;
	};
//	info->netName = (gstate->episode != 3) ? tongue[T_HEALTH_KIT] : tongue[T_HEALTH_VIAL];
	info->episodeIndependentModelName = "hlth.dkm";
	info->touchFunc = item_health_touch;

	info->mins.Set(-10, -10, -24);
	info->maxs.Set(10, 10, 5);

	info->health = 25;
	info->respawnTime = 30.0;

	info->soundPickup = "global/a_hpick.wav";
	info->soundRespawn = "global/a_itspwn.wav";
	info->soundAmbient = NULL;

	item_Spawn (self, info);
}


///////////////////////////////////////////////////////////////////////////////
// item_health_50
///////////////////////////////////////////////////////////////////////////////
void item_health_50(userEntity_t *self)
{
	itemInfo_t	*info;

	if (IS_INSTAGIB || (deathmatch->value && !dm_allow_health->value ))
	  return;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
//	info->netName = tongue[T_HEALTH_KIT];
	info->episodeIndependentModelName = "hlth2.dkm";
	info->touchFunc = item_health_touch;

	switch(gstate->episode)
	{
	default:
	case 1:
		info->netName = tongue[T_HEALTH_E1_PACK];
		info->netNameID = RESOURCE_ID_FOR_STRING(tongue,T_HEALTH_E1_PACK);
		break;
	case 3:
		info->netName = tongue[T_HEALTH_E3_CRATE];
		info->netNameID = RESOURCE_ID_FOR_STRING(tongue,T_HEALTH_E3_CRATE);
		break;
	case 4:
		info->netName = tongue[T_HEALTH_E4_BOX];
		info->netNameID = RESOURCE_ID_FOR_STRING(tongue,T_HEALTH_E4_BOX);
		break;
	};

	info->mins.Set(-13, -13, -24);
	info->maxs.Set(13, 13, -5);

	info->health = 50;
	info->respawnTime = 30.0;

	info->soundPickup = "global/a_h50pick.wav";
	info->soundRespawn = "global/a_itspwn.wav";
	info->soundAmbient = NULL;

	item_Spawn (self, info);
}


///////////////////////////////////////////////////////////////////////////////
// item_goldensoul
///////////////////////////////////////////////////////////////////////////////

void	item_goldensoul (userEntity_t *self)
{
	itemInfo_t	*info;

	if (IS_INSTAGIB || (deathmatch->value && !dm_allow_health->value))
	  return;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	info->netName = tongue[T_GOLDENSOUL];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue,T_GOLDENSOUL);


	info->touchFunc = item_health_touch;

	info->mins.Set(-8, -8, -24);
	info->maxs.Set(8, 8, 0);

	//	goldensoul will give 1000 health, but that will be capped at base_health * 2.0
	info->health = 100;
	info->respawnTime = 60.0;

	info->soundPickup = "artifacts/goldensoulpickup.wav";
	info->soundRespawn = "global/a_itspwn.wav";
	info->soundAmbient = "artifacts/goldensoulwait.wav";

	info->flags |= ITEM_BOOSTBASEHEALTH;

	info->frame_start = 0;
	info->frame_end = 9;
	info->frame_time = 0.1;

	info->render_scale = 1.0;

	item_Spawn (self, info, MOVETYPE_TOSS, TRUE, "models/global/a_gsoul.dkm");
}

///////////////////////////////////////////////////////////////////////////////
// item_wraithorb_touch
///////////////////////////////////////////////////////////////////////////////

void	item_wraithorb_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!com->ValidTouch (self, other))
		return;
	
	// NSS[2/8/00]:Sidekicks cannot touch these things
	if(other->flags & FL_BOT)
		return;

	playerHook_t	*ohook = AI_GetPlayerHook( other );
	itemHook_t		*ihook = (itemHook_t *) self->userHook;

	//	play pickup sound
	gstate->StartEntitySound(other, CHAN_VOICE, ihook->soundPickupIndex, 0.85f, ohook->fMinAttenuation, ohook->fMaxAttenuation);

	if (other->flags & FL_CLIENT)
	{
		other->client->bonus_alpha = 0.25;
 		
		artifact_special_print(T_PICKUP_BOOST, other, "%s %s.\n", 2, tongue, self->netNameID);
		//gstate->centerprint(other, 2.0, "%s %s.\n", tongue[T_PICKUP_BOOST], self->netname);
    }

	if(!deathmatch->value)
	{
		userEntity_t *Super = AIINFO_GetSuperfly();
		if(!Super)
		{
			Super = AIINFO_GetMikikofly();
		}
		
		if(Super)
		{
			playerHook_t *shook = AI_GetPlayerHook(Super);
			if(shook)
			{
				shook->items = shook->items | IT_WRAITHORB;
				shook->exp_flags = shook->exp_flags | EXP_WRAITHORB;
				shook->wraithorb_time = 60; //BOOST_ACTIVE_TIME;
				Super->s.alpha = 1.0f;
				Super->s.renderfx |= RF_TRANSLUCENT;
			}
		}
		userEntity_t *Mikiko = AIINFO_GetMikiko();
		if(Mikiko)
		{
			playerHook_t *shook = AI_GetPlayerHook(Mikiko);
			if(shook)
			{
				shook->items = shook->items | IT_WRAITHORB;
				shook->exp_flags = shook->exp_flags | EXP_WRAITHORB;
				shook->wraithorb_time = 60; //BOOST_ACTIVE_TIME;
				Mikiko->s.alpha = 1.0f;
				Mikiko->s.renderfx |= RF_TRANSLUCENT;
			}
		}
	}

	ohook->items = ohook->items | IT_WRAITHORB;
	ohook->exp_flags = ohook->exp_flags | EXP_WRAITHORB;

//	ohook->wraithorb_time = gstate->time + 60; //BOOST_ACTIVE_TIME;
	ohook->wraithorb_time = 60; //BOOST_ACTIVE_TIME;
	other->s.alpha = 1.0f;
	other->s.renderfx |= RF_TRANSLUCENT;

	// call respawn last because it can remove the entity
	//com->SetRespawn (self, ihook->respawnTime);
	// NSS[3/4/00]:
	Item_SetRespawn(self);


	if ( other->PickedUpItem )
	{
		other->PickedUpItem( other, self->className );
	}

}


///////////////////////////////////////////////////////////////////////////////
// item_wraithorb
///////////////////////////////////////////////////////////////////////////////

void	item_wraithorb (userEntity_t *self)
{
	itemInfo_t	*info;

	if (deathmatch->value && !dm_allow_powerups->value)
	  return;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
//	info = (itemInfo_t*) new itemInfo_t;
	info->className = self->className;
	info->netName = tongue[T_WRAITHORB];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue,T_WRAITHORB);

	info->touchFunc = item_wraithorb_touch;

	info->mins.Set(-8, -8, -24);
	info->maxs.Set(8, 8, 0);

	//	goldensoul will give 1000 health, but that will be capped at base_health * 2.0
	info->health = 100;
	info->respawnTime = 300.0;

	info->soundPickup = "artifacts/wraithorbpickup.wav";
	info->soundRespawn = "global/a_itspwn.wav";
	info->soundAmbient = "artifacts/wraithorbwait.wav";

	info->frame_start = 0;
	info->frame_end = 29;
	info->frame_time = 0.1;

	info->render_scale = 1.0;

	item_Spawn (self, info, MOVETYPE_TOSS, TRUE, "models/global/a_wraithorb.dkm");

	self->s.renderfx |= RF_TRANSLUCENT;
	self->s.effects |= EF_ROTATE;
	self->s.alpha = 0.66f;

}


///////////////////////////////////////////////////////////////////////////////
//
// ARMOR ITEMS
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// item_armor_touch
///////////////////////////////////////////////////////////////////////////////

#define SPAWNFLAG_SUPERFLY	1
#define SPAWNFLAG_MIKIKO	2
void item_armor_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!com->ValidTouch (self, other))
		return;

	// check to see if this item is meant for a particular sidekick (single player only!)
	/*if (!deathmatch->value && !coop->value)
	{
		if (self->spawnflags & (SPAWNFLAG_SUPERFLY|SPAWNFLAG_MIKIKO))
		{
			short passed = 0;
			if (self->spawnflags & SPAWNFLAG_SUPERFLY)				// superfly can pick this up
			{
				passed = (!strcmp( other->className, "Superfly" )) ? 1 : 0;
			}

			if (!passed && (self->spawnflags & SPAWNFLAG_MIKIKO))
			{
				passed = (!strcmp( other->className, "Mikiko" )) ? 1 : 0;
			}

			if (!passed)
				return;
		}
	}*/

	playerHook_t	*ohook = AI_GetPlayerHook( other );
	itemHook_t		*ihook = (itemHook_t *) self->userHook;
	
	// NSS[1/11/00]:Check for client picking up items
	if(other->flags & (FL_CLIENT|FL_BOT))
		SIDEKICK_RemoveItem(self);
	
	// check to see if current armor is worse than this armor
	if ((self->health * ((float)self->armor_abs/100.0)) <= (other->armor_val * other->armor_abs))
		return;

	other->armor_val = self->health;
	other->armor_abs = (float)self->armor_abs/100.0;

	// subtract the old armor (if we have any)
	ohook->items = ohook->items - (ohook->items & (IT_PLASTEEL | IT_CHROMATIC | IT_SILVER | 
				  IT_GOLD | IT_CHAINMAIL | IT_BLACK_ADAMANT | IT_KEVLAR | IT_EBONITE | IT_MEGASHIELD));
	
	// add the new armor
	ohook->items = ohook->items | ihook->flags;

/*	
	// send new armor value down to client
	gstate->WriteByte (other, MSG_ONE, SVC_ARMORVAL);
	gstate->WriteByte (other, MSG_ONE, (int)ohook->armor_val);
*/

	gstate->StartEntitySound(other, CHAN_VOICE, ihook->soundPickupIndex, 0.85f, ohook->fMinAttenuation, ohook->fMaxAttenuation);
	
// cek[12-6-99] Put back in per Romero
	if (other->flags & FL_CLIENT)
	{
		// screen flash
		other->client->bonus_alpha = 0.25;

		//gstate->centerprint (other, 2.0, "%s %s.\n", tongue[T_PICKUP_ARMOR_GOOD], self->netname);
		artifact_special_print(T_PICKUP_ARMOR_GOOD, other, "%s %s.\n", 2, tongue, self->netNameID);
//        gstate->sprint (other, "%s %s.\n", tongue[T_PICKUP_ARMOR_GOOD], self->netname);
	}


	// call respawn last because it can remove the entity
	//com->SetRespawn (self, ihook->respawnTime);
	// NSS[3/4/00]:
	Item_SetRespawn(self);


	if ( other->PickedUpItem )
	{
		other->PickedUpItem( other, self->className );
	}

}

///////////////////////////////////////////////////////////////////////////////
// item_plasteel_armor
///////////////////////////////////////////////////////////////////////////////

void	item_plasteel_armor (userEntity_t *self)
{
	itemInfo_t	*info;

	if(IS_INSTAGIB || (deathmatch->value && !dm_allow_armor->value ))
		return;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	info->netName = tongue_armors[T_ARMOR_PLASTEEL];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue_armors,T_ARMOR_PLASTEEL);


	info->episodeIndependentModelName = "ar1.dkm";
	info->touchFunc = item_armor_touch;

	info->mins.Set(-16, -16, -16);
	info->maxs.Set(16, 16, 16);

	self->health     = ARMOR_PLASTEEL_MAXHEALTH;
	self->armor_abs  = ARMOR_PLASTEEL_ABSORPTION;
	self->takedamage = DAMAGE_NO;

	info->flags |= IT_PLASTEEL;
	info->respawnTime = ARMOR_RESPAWN_TIME;

	info->soundPickup = "global/armorpickup1.wav";
	info->soundRespawn = "global/a_itspwn.wav";
	info->soundAmbient = NULL;

	item_Spawn (self, info, MOVETYPE_TOSS, TRUE);
}

///////////////////////////////////////////////////////////////////////////////
// item_chromatic_armor
///////////////////////////////////////////////////////////////////////////////

void	item_chromatic_armor (userEntity_t *self)
{
	itemInfo_t	*info;

	if( IS_INSTAGIB || (deathmatch->value && !dm_allow_armor->value ))
		return;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	info->netName = tongue_armors[T_ARMOR_CHROMATIC];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue_armors,T_ARMOR_CHROMATIC);

	info->episodeIndependentModelName = "ar2.dkm";
	info->touchFunc = item_armor_touch;

	info->mins.Set(-16, -16, -16);
	info->maxs.Set(16, 16, 16);

	self->health     = ARMOR_CHROMATIC_MAXHEALTH;
	self->armor_abs  = ARMOR_CHROMATIC_ABSORPTION;
	self->takedamage = DAMAGE_NO;

	info->flags |= IT_CHROMATIC;
	info->respawnTime = ARMOR_RESPAWN_TIME;

	info->soundPickup = "global/armorpickup2.wav";
	info->soundRespawn = "global/a_itspwn.wav";
	info->soundAmbient = NULL;

	item_Spawn (self, info, MOVETYPE_TOSS, TRUE);
}

///////////////////////////////////////////////////////////////////////////////
// item_silver_armor
///////////////////////////////////////////////////////////////////////////////

void	item_silver_armor (userEntity_t *self)
{
	itemInfo_t	*info;

	if( IS_INSTAGIB || (deathmatch->value && !dm_allow_armor->value ))
		return;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	info->netName	= tongue_armors[T_ARMOR_SILVER];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue_armors,T_ARMOR_SILVER);

	info->episodeIndependentModelName = "ar1.dkm";
	info->touchFunc = item_armor_touch;

	info->mins.Set(-16, -16, -16);
	info->maxs.Set(16, 16, 16);

	self->health     = ARMOR_SILVER_MAXHEALTH;
	self->armor_abs  = ARMOR_SILVER_ABSORPTION;
	self->takedamage = DAMAGE_NO;

	info->flags |= IT_SILVER;
	info->respawnTime = ARMOR_RESPAWN_TIME;

	info->soundPickup = "global/armorpickup1.wav";
	info->soundRespawn = "global/a_itspwn.wav";
	info->soundAmbient = NULL;

	item_Spawn (self, info, MOVETYPE_TOSS, TRUE);
}

///////////////////////////////////////////////////////////////////////////////
// item_gold_armor
///////////////////////////////////////////////////////////////////////////////

void	item_gold_armor (userEntity_t *self)
{
	itemInfo_t	*info;

	if( IS_INSTAGIB || (deathmatch->value && !dm_allow_armor->value ))
		return;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	info->netName = tongue_armors[T_ARMOR_GOLD];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue_armors,T_ARMOR_GOLD);
	
	info->episodeIndependentModelName = "ar2.dkm";
	info->touchFunc = item_armor_touch;

	info->mins.Set(-16, -16, -16);
	info->maxs.Set(16, 16, 16);

	self->health     = ARMOR_GOLD_MAXHEALTH;
	self->armor_abs  = ARMOR_GOLD_ABSORPTION;
	self->takedamage = DAMAGE_NO;

	info->flags |= IT_GOLD;
	info->respawnTime = ARMOR_RESPAWN_TIME;

	info->soundPickup = "global/armorpickup2.wav";
	info->soundRespawn = "global/a_itspwn.wav";
	info->soundAmbient = NULL;

	item_Spawn (self, info, MOVETYPE_TOSS, TRUE);
}

///////////////////////////////////////////////////////////////////////////////
// item_chainmail_armor
///////////////////////////////////////////////////////////////////////////////

void	item_chainmail_armor (userEntity_t *self)
{
	itemInfo_t	*info;

	if( IS_INSTAGIB || (deathmatch->value && !dm_allow_armor->value ))
		return;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	info->netName = tongue_armors[T_ARMOR_CHAINMAIL];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue_armors,T_ARMOR_CHAINMAIL);

	info->episodeIndependentModelName = "ar1.dkm";
	info->touchFunc = item_armor_touch;

	info->mins.Set(-16, -16, -16);
	info->maxs.Set(16, 16, 16);

	self->health     = ARMOR_CHAINMAIL_MAXHEALTH;
	self->armor_abs  = ARMOR_CHAINMAIL_ABSORPTION;
	self->takedamage = DAMAGE_NO;

	info->flags |= IT_CHAINMAIL;
	info->respawnTime = ARMOR_RESPAWN_TIME;

	info->soundPickup = "global/armorpickup1.wav";
	info->soundRespawn = "global/a_itspwn.wav";
	info->soundAmbient = NULL;

	item_Spawn (self, info, MOVETYPE_TOSS, TRUE);
}

///////////////////////////////////////////////////////////////////////////////
// item_black_adamant_armor
///////////////////////////////////////////////////////////////////////////////

void	item_black_adamant_armor (userEntity_t *self)
{
	itemInfo_t	*info;

	if( IS_INSTAGIB || (deathmatch->value && !dm_allow_armor->value ))
		return;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	info->netName = tongue_armors[T_ARMOR_BLACKADAMANT];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue_armors,T_ARMOR_BLACKADAMANT);

	info->episodeIndependentModelName = "ar2.dkm";
	info->touchFunc = item_armor_touch;

	info->mins.Set(-16, -16, -16);
	info->maxs.Set(16, 16, 16);

	self->health     = ARMOR_BLACK_ADAMANT_MAXHEALTH;
	self->armor_abs  = ARMOR_BLACK_ADAMANT_ABSORPTION;
	self->takedamage = DAMAGE_NO;

	info->flags |= IT_BLACK_ADAMANT;
	info->respawnTime = ARMOR_RESPAWN_TIME;

	info->soundPickup = "global/armorpickup2.wav";
	info->soundRespawn = "global/a_itspwn.wav";
	info->soundAmbient = NULL;

	item_Spawn (self, info, MOVETYPE_TOSS, TRUE);
}

///////////////////////////////////////////////////////////////////////////////
// item_kevlar_armor
///////////////////////////////////////////////////////////////////////////////

void	item_kevlar_armor (userEntity_t *self)
{
	itemInfo_t	*info;

	if( IS_INSTAGIB || (deathmatch->value && !dm_allow_armor->value ))
		return;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	info->netName = tongue_armors[T_ARMOR_KEVLAR];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue_armors,T_ARMOR_KEVLAR);
	info->episodeIndependentModelName = "ar1.dkm";
	info->touchFunc = item_armor_touch;

	info->mins.Set(-16, -16, -16);
	info->maxs.Set(16, 16, 16);
	// NSS[12/16/99]:
	gstate->SetSize(self, -16.0f, -16.0f, -8.0f, 16.0f,16.0f, 24.0f);

	self->health     = ARMOR_KEVLAR_MAXHEALTH;
	self->armor_abs  = ARMOR_KEVLAR_ABSORPTION;
	self->takedamage = DAMAGE_NO;

	info->flags |= IT_KEVLAR;
	info->respawnTime = ARMOR_RESPAWN_TIME;

	info->soundPickup = "global/armorpickup1.wav";
	info->soundRespawn = "global/a_itspwn.wav";
	info->soundAmbient = NULL;

	item_Spawn (self, info, MOVETYPE_TOSS, TRUE);
}

///////////////////////////////////////////////////////////////////////////////
// item_ebonite_armor
///////////////////////////////////////////////////////////////////////////////

void	item_ebonite_armor (userEntity_t *self)
{
	itemInfo_t	*info;

	if( IS_INSTAGIB || (deathmatch->value && !dm_allow_armor->value ))
		return;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	info->netName = tongue_armors[T_ARMOR_EBONITE];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue_armors,T_ARMOR_EBONITE);

	info->episodeIndependentModelName = "ar2.dkm";
	info->touchFunc = item_armor_touch;

	info->mins.Set(-16, -16, -16);
	info->maxs.Set(16, 16, 16);
	// NSS[12/16/99]:
	gstate->SetSize(self, -16, -16, -8, 16,16, 24);

	self->health     = ARMOR_EBONITE_MAXHEALTH;
	self->armor_abs  = ARMOR_EBONITE_ABSORPTION;
	self->takedamage = DAMAGE_NO;

	info->flags |= IT_EBONITE;
	info->respawnTime = ARMOR_RESPAWN_TIME;

	info->soundPickup = "global/armorpickup2.wav";
	info->soundRespawn = "global/a_itspwn.wav";
	info->soundAmbient = NULL;

	item_Spawn (self, info, MOVETYPE_TOSS, TRUE);
}

///////////////////////////////////////////////////////////////////////////////
//
// BOOST ITEM POWERUPS
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// item_boost_execute
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// item_powerboost_execute
///////////////////////////////////////////////////////////////////////////////
void item_powerboost_execute( struct userInventory_s *ptr, struct edict_s *user )
{
	playerHook_t *ohook = AI_GetPlayerHook( user );


	ohook->power_boost = 5;
	ohook->items = ohook->items | IT_POWERBOOST;
	ohook->exp_flags = ohook->exp_flags | EXP_POWERBOOST;

// SCG[1/4/00]: 	ohook->power_boost_time = gstate->time + BOOST_ACTIVE_TIME;
	ohook->power_boost_time = BOOST_ACTIVE_TIME;

	com->CalcBoosts (user);

	gstate->InventoryDeleteItem( user, user->inventory, ptr );

	// ISP: 3-10-99 only for clients
	if (user->flags & FL_CLIENT)
	{
	    com->Boost_Icons(user, BOOSTICON_ADD, ICON_POWERBOOST, BOOST_ACTIVE_TIME); // user, icon type, seconds
	}

	//gstate->sprint (user, "%s.\n",tongue[T_USEBOOST_POWER]);
}



///////////////////////////////////////////////////////////////////////////////
// item_attackboost_execute
///////////////////////////////////////////////////////////////////////////////
void item_attackboost_execute( struct userInventory_s *ptr, struct edict_s *user )
{
	playerHook_t *ohook = AI_GetPlayerHook( user );

	ohook->attack_boost = 5;
	ohook->items = ohook->items | IT_ATTACKBOOST;
	ohook->exp_flags = ohook->exp_flags | EXP_ATTACKBOOST;

// SCG[1/4/00]: 	ohook->attack_boost_time = gstate->time + BOOST_ACTIVE_TIME;
	ohook->attack_boost_time = BOOST_ACTIVE_TIME;

	com->CalcBoosts (user);

	gstate->InventoryDeleteItem( user, user->inventory, ptr );

	// ISP: 3-10-99 only for clients
	if( user->flags & FL_CLIENT )
	{
	    com->Boost_Icons(user, BOOSTICON_ADD, ICON_ATTACKBOOST, BOOST_ACTIVE_TIME); // user, icon type, seconds
	}

	//gstate->sprint (user, "%s.\n",tongue[T_USEBOOST_ATTACK]);
}



///////////////////////////////////////////////////////////////////////////////
// item_speedboost_execute
///////////////////////////////////////////////////////////////////////////////
void item_speedboost_execute( struct userInventory_s *ptr, struct edict_s *user )
{
	playerHook_t *ohook = AI_GetPlayerHook( user );

	ohook->speed_boost = 5;
	ohook->items = ohook->items | IT_SPEEDBOOST;
	ohook->exp_flags = ohook->exp_flags | EXP_SPEEDBOOST;

// SCG[1/4/00]: 	ohook->speed_boost_time = gstate->time + BOOST_ACTIVE_TIME;
	ohook->speed_boost_time = BOOST_ACTIVE_TIME;

	com->CalcBoosts (user);

	gstate->InventoryDeleteItem( user, user->inventory, ptr );

	// ISP: 3-10-99 only for clients
	if (user->flags & FL_CLIENT)
	{
	    com->Boost_Icons(user, BOOSTICON_ADD, ICON_SPEEDBOOST, BOOST_ACTIVE_TIME);  // user, icon type, seconds
	}

	//gstate->sprint (user, "%s.\n",tongue[T_USEBOOST_SPEED]);
}



///////////////////////////////////////////////////////////////////////////////
// item_acroboost_execute
///////////////////////////////////////////////////////////////////////////////
void item_acroboost_execute( struct userInventory_s *ptr, struct edict_s *user )
{
	playerHook_t	*ohook = AI_GetPlayerHook( user );

	ohook->acro_boost = 5;
	ohook->items = ohook->items | IT_ACROBOOST;
	ohook->exp_flags = ohook->exp_flags | EXP_ACROBOOST;

// SCG[1/4/00]: 	ohook->acro_boost_time = gstate->time + BOOST_ACTIVE_TIME;
	ohook->acro_boost_time = BOOST_ACTIVE_TIME;

	com->CalcBoosts (user);

	gstate->InventoryDeleteItem( user, user->inventory, ptr );

	// ISP: 3-10-99 only for clients
	if (user->flags & FL_CLIENT)
	{
	    com->Boost_Icons(user, BOOSTICON_ADD, ICON_ACROBOOST, BOOST_ACTIVE_TIME);  // user, icon type, seconds
	}

	//gstate->sprint (user, "%s.\n",tongue[T_USEBOOST_ACRO]);
}



///////////////////////////////////////////////////////////////////////////////
// item_vitaboost_execute
///////////////////////////////////////////////////////////////////////////////
void item_vitaboost_execute( struct userInventory_s *ptr, struct edict_s *user )
{
	playerHook_t	*ohook = AI_GetPlayerHook( user );

	ohook->vita_boost = 5;
	ohook->items = ohook->items | IT_VITABOOST;
	ohook->exp_flags = ohook->exp_flags | EXP_VITABOOST;

	ohook->vita_boost_time = BOOST_ACTIVE_TIME;

	com->CalcBoosts (user);

	// cek: boost their health too
	com->Health (user, 75, ohook->base_health);

	gstate->InventoryDeleteItem( user, user->inventory, ptr );

	// ISP: 3-10-99 only for clients
	if (user->flags & FL_CLIENT)
	{
	    com->Boost_Icons(user, BOOSTICON_ADD, ICON_VITABOOST, BOOST_ACTIVE_TIME);  // user, icon type, seconds
	}

	//gstate->sprint (user, "%s.\n",tongue[T_USEBOOST_VITA]);
}

void item_wood_box_open( userEntity_t *self )
{
	userEntity_t	*pEntity;
	int				nIndex = ( int ) ( rnd() * 100 );
	CVector			vTemp;
	
	nIndex %= 5;

	// cek[12-14-99] move the thinggy a bit in front of the chest so the player can reach it easily
	AngleToVectors(self->s.angles,forward);
	vTemp = self->s.origin;
	vTemp.z += 40;
	vTemp += 8 * forward;

	switch( nIndex )
	{
	case 0:
		pEntity = gstate->SpawnEntity();
		pEntity->s.origin = vTemp;
		item_power_boost( pEntity );
// SCG[12/10/99]: 		com->SpawnDynamicEntity( self, "item_power_boost", FALSE );
		break;
	case 1:
		pEntity = gstate->SpawnEntity();
		pEntity->s.origin = vTemp;
		item_attack_boost( pEntity );
// SCG[12/10/99]: 		com->SpawnDynamicEntity( self, "item_attack_boost", FALSE );
		break;
	case 2:
		pEntity = gstate->SpawnEntity();
		pEntity->s.origin = vTemp;
		item_acro_boost( pEntity );
// SCG[12/10/99]: 		com->SpawnDynamicEntity( self, "item_acro_boost", FALSE );
		break;
	case 3:
		pEntity = gstate->SpawnEntity();
		pEntity->s.origin = vTemp;
		item_speed_boost( pEntity );
// SCG[12/10/99]: 		com->SpawnDynamicEntity( self, "item_speed_boost", FALSE );
		break;
	case 4:
		pEntity = gstate->SpawnEntity();
		pEntity->s.origin = vTemp;
		item_vita_boost( pEntity );
// SCG[12/10/99]: 		com->SpawnDynamicEntity( self, "item_vita_boost", FALSE );
		break;
	}

}

#define EXPLOSION_CHANCE	50
void spawnPolyExplosion(CVector &org, float scale, float lsize, CVector &color, short flags);
void spawn_small_sprite_explosion (userEntity_t *self, CVector &org, int play_sound);
extern	void	spawn_sprite_explosion (userEntity_t *self, CVector &org, int play_sound);

///////////////////////////////////////////////////////////////////////////////////
//	item_black_box_think
//NSS[11/17/99]: Think function occurs every 1 second and checks to see if a client
//is within 'x' distance.
///////////////////////////////////////////////////////////////////////////////////
void item_black_box_think(userEntity_t * self)
{
	CVector vTemp;
	userEntity_t	*pEntity;
	int				nIndex = ( int ) self->hacks;
	
	if(self->hacks_int >= EXPLOSION_CHANCE)
	{
		nIndex %= 4;

		// cek[12-14-99] move the thinggy a bit in front of the chest so the player can reach it easily
		AngleToVectors(self->s.angles,forward);
		vTemp = self->s.origin;
		vTemp.z += 40;
		vTemp += 8 * forward;

		switch( nIndex )
		{
		case 0:
			pEntity = com->SpawnDynamicEntity( self, "item_goldensoul", FALSE );
			break;
		case 1:
			pEntity = com->SpawnDynamicEntity( self, "item_wraithorb", FALSE );
			break;
		case 2:
			pEntity = com->SpawnDynamicEntity( self, "item_megashield", FALSE );
			break;
		case 3:
			pEntity = com->SpawnDynamicEntity( self, "item_invincibility", FALSE );
			break;
		}
		pEntity->s.origin = vTemp;
	}	

	com->untrackEntity(self,NULL,MULTICAST_ALL);
//	gstate->RemoveEntity(self);
}

userEntity_t *throw_debris (char *modelname, float speed, CVector &origin, 
			int renderfx, CVector &scale, CVector &velocity, float delay);
void item_black_box_open( userEntity_t *self )
{
	CVector vTemp;
	vTemp.Set( 1, 1, 1 );

	// SCG[11/18/99]: Well, we had to store it somewhere!
	userEntity_t *pPlayer = self->owner;
	self->owner = NULL;
	//Setup whether or not it will explode first
//	self->hacks_int = ( int )( rnd() * 100 );

	if( self->hacks_int  < EXPLOSION_CHANCE )
	{
		CVector dir = pPlayer->s.origin - self->s.origin;
		dir.Normalize();
		dir.Multiply(16);
		dir.z += 30;

		vTemp.Set( 0, 0, 0 );
		com->Damage( pPlayer, self, self, self->s.origin, vTemp, 25.0, 0 );
		spawnPolyExplosion( self->s.origin, 0.7, 100, vTemp, TE_POLYEXPLOSION_SND );
		spawn_sprite_explosion (self, self->s.origin + dir, false);
		spawn_small_sprite_explosion(self, self->s.origin + dir, false);

		CVector size;
		for(int i=0;i < 15 ; i++)
		{
			size.Set(2 + 1.5*frand(),1 * 0.75*frand(),1 * 0.75*frand());
			throw_debris(gib_models[GIB_WOOD1+(rand() & 1)],10,self->s.origin + dir, RF_TRANSLUCENT, size, CVector (0,0,1),0);
		}
		com->untrackEntity(self,NULL,MULTICAST_ALL);
		gstate->RemoveEntity(self);
		return;
	}
	else
	{
		//Store the pregenerated value for the item spawning
		self->hacks  = ( rnd() * 100 );

		//Create the Special FX for the spawning
		trackInfo_t tinfo;
		// clear this variable
		memset(&tinfo, 0, sizeof(tinfo));

		tinfo.ent			=self;
		tinfo.srcent		=self;
		tinfo.fru.Zero();
		tinfo.Long1			= ART_BLACKBOX;
		tinfo.flags			= TEF_FXFLAGS|TEF_SRCINDEX|TEF_LONG1;
		tinfo.fxflags		= TEF_ARTIFACT_FX|TEF_FX_ONLY;
		
		com->trackEntity(&tinfo,MULTICAST_ALL);
	}
	self->think		= item_black_box_think;
	self->nextthink	= gstate->time + 1.5f;
}

void item_black_chest_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator )
{
	self->s.effects = 0;
	self->s.frame = 0;
	self->s.frameInfo.startFrame = 1;
	self->s.frameInfo.endFrame = 19;
	self->s.frameInfo.frameInc = 1;
	self->s.frameInfo.frameFlags = ( FRAME_FORCEINDEX | FRAME_ONCE );
	self->s.frameInfo.modelAnimSpeed = 1;

	// precompute this so we can customize the time accordingly
	self->hacks_int = ( int )( rnd() * 100 );
	if( self->hacks_int  < EXPLOSION_CHANCE )
		self->nextthink = gstate->time + 1.0;
	else
		self->nextthink = gstate->time + 1.8;

	self->think = item_black_box_open;

	// cek[12-14-99] play the creaky door open sound
	gstate->StartEntitySound (self, CHAN_VOICE, gstate->SoundIndex("doors/e3/woodendoor4open.wav"), 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	self->use = NULL;
	self->owner = other;
}

void item_wood_chest_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator )
{
	self->s.effects = 0;
	self->s.frame = 0;
	self->s.frameInfo.startFrame = 0;
	self->s.frameInfo.endFrame = 19;
	self->s.frameInfo.frameInc = 1;
	self->s.frameInfo.frameFlags = ( FRAME_FORCEINDEX | FRAME_ONCE );
	self->s.frameInfo.modelAnimSpeed = 1;

	self->think = item_wood_box_open;
	self->nextthink = gstate->time + 1.8;

	// cek[12-14-99] play the creaky door open sound
	gstate->StartEntitySound (self, CHAN_VOICE, gstate->SoundIndex("doors/e3/woodendoor4open.wav"), 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	self->use = NULL;
//cek[12-14-99] so the player clips against it.	self->owner = other;
}

// ----------------------------------------------------------------------------
// NSS[3/2/00]:
// Name:		item_sidekick_megashield
// Description:Sets the megashield value for the sidekicks.
// Input:userEntity_t *self (sidekick), userEntity_t *item (the item)
// Output:TRUE/FALSE
// Note:called from item_megashield_execute
// ----------------------------------------------------------------------------
int item_sidekick_megashield(userEntity_t *self, userEntity_t *item)
{
	playerHook_t *sideHook;
	if(self)
	{	
		sideHook = AI_GetPlayerHook(self);
		if(sideHook && AI_IsSidekick(sideHook))
		{
			self->armor_val = ARMOR_MEGASHIELD_MAXHEALTH;
			self->armor_abs = ARMOR_MEGASHIELD_ABSORPTION/100.0f;	
			sideHook->items &= ~(IT_PLASTEEL|IT_CHROMATIC|IT_SILVER|IT_GOLD|IT_CHAINMAIL|IT_BLACK_ADAMANT|IT_KEVLAR|IT_EBONITE|IT_MEGASHIELD);	// clear the old one...
			sideHook->items |= IT_MEGASHIELD;
			if(self->PickedUpItem)
				self->PickedUpItem( self, item->className );
			return TRUE;
		}
	}
	return FALSE;
}

void item_megashield_execute( userEntity_t *self, userEntity_t *user )
{
	artifact_special_print(T_PICKUP_ARMOR_GOOD, user, "%s %s.\n", 2, tongue, self->netNameID);
//	gstate->centerprint (user, 2.0, "%s %s.\n", tongue[T_PICKUP_ARMOR_GOOD], self->netname);
	
	// NSS[3/2/00]:As long as we are not in deathmatch mode... let's do this...
	if(!deathmatch->value && !coop->value)
	{
		// NSS[3/2/00]:Sidekicks get this too!
		if(!item_sidekick_megashield( AIINFO_GetSuperfly(), self ) )
		{
			item_sidekick_megashield( AIINFO_GetMikikofly(), self );
		}
		item_sidekick_megashield( AIINFO_GetMikiko(), self );
	}

	// cek[3-6-00]: we need to make sure the armor flags get cleared and IT_MEGASHIELD gets set...so things work right in coop mode...

	playerHook_t *phook = AI_GetPlayerHook(user);
	if (phook)
	{
		phook->items &= ~(IT_PLASTEEL|IT_CHROMATIC|IT_SILVER|IT_GOLD|IT_CHAINMAIL|IT_BLACK_ADAMANT|IT_KEVLAR|IT_EBONITE|IT_MEGASHIELD);
		
		// add the new armor
		phook->items |= IT_MEGASHIELD;
	}

	user->armor_val = ARMOR_MEGASHIELD_MAXHEALTH;
	user->armor_abs = ARMOR_MEGASHIELD_ABSORPTION/100.0f;	
}

// ----------------------------------------------------------------------------
// NSS[3/2/00]:
// Name:		item_sidekick_manaskull
// Description:Sets the manaskull value for the sidekicks.
// Input:userEntity_t *self (sidekick), userEntity_t *item (the item)
// Output:TRUE/FALSE
// Note:called from item_manaskull_execute
// ----------------------------------------------------------------------------
int item_sidekick_manaskull(userEntity_t *self, userEntity_t *item)
{
	playerHook_t *sideHook;
	if(self)
	{	
		sideHook = AI_GetPlayerHook(self);
		if(sideHook && AI_IsSidekick(sideHook))
		{
			sideHook->items					|= IT_MANASKULL;
			sideHook->exp_flags				|= EXP_MANASKULL;
			sideHook->invulnerability_time	= 30.0;
			if(self->PickedUpItem)
				self->PickedUpItem( self, item->className );
			return TRUE;
		}
	}
	return FALSE;
}

void item_manaskull_execute( userEntity_t *self, userEntity_t *user )
{
	playerHook_t *ohook = AI_GetPlayerHook( user );
	if (!ohook)
		return;

	//gstate->centerprint (user, 2.0, "%s %s.\n", tongue[T_PICKUP_ARMOR_GOOD], self->netname);
	artifact_special_print(T_PICKUP_ARMOR_GOOD, user, "%s %s.\n", 2, tongue, self->netNameID);
	
	// NSS[3/2/00]:As long as we are not in deathmatch mode... let's do this...
	if(!deathmatch->value && !coop->value)
	{
		// NSS[3/2/00]:Sidekicks get this too!
		if(!item_sidekick_manaskull( AIINFO_GetSuperfly(), self ) )
		{
			item_sidekick_manaskull( AIINFO_GetMikikofly(), self );
		}
		item_sidekick_manaskull( AIINFO_GetMikiko(), self );
	}
	ohook->items |= IT_MANASKULL;
	ohook->exp_flags |= EXP_MANASKULL;
	ohook->invulnerability_time = 30.0;
}

void boost_respawn( userEntity_t *self )
{
	itemHook_t		*ihook = (itemHook_t *) self->userHook;
	if (ihook && (deathmatch->value && dm_item_respawn->value) && ihook->respawnTime > 0) // || !(deathmatch->value))//FIXME: no respawn unless deathmatch
	{
		self->s.modelindex = gstate->ModelIndex (self->modelName);
		self->solid = SOLID_TRIGGER;
		self->nextthink = -1;

		self->s.effects |= EF_RESPAWN;	//	tell client to start respawn effect
		self->s.renderfx |= RF_GLOW;

		gstate->LinkEntity (self);
		gstate->StartEntitySound (self, CHAN_VOICE, ihook->soundRespawnIndex, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);

		// turn on the ambient
		item_setSound(self,ihook->info);
	}
	self->think = NULL;
	self->nextthink = -1;
}

///////////////////////////////////////////////////////////////////////////////
// item_boost_touch
///////////////////////////////////////////////////////////////////////////////

void item_boost_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	if (!com->ValidTouch (self, other))
		return;

	if (!self->className)
		return;

	// NSS[2/8/00]:Sidekicks cannot touch these things
	if(other->flags & FL_BOT)
		return;
	
	playerHook_t	*ohook = AI_GetPlayerHook( other );
	itemHook_t		*ihook = (itemHook_t *) self->userHook;

	// check the ihook->flags value and see what kind of power up it is
	if( ihook->flags & IT_POWERBOOST )
	{
		item_powerboost_execute( NULL, other );
	}
	else if( ihook->flags & IT_ATTACKBOOST )
	{
		item_attackboost_execute( NULL, other );
	}
	else if( ihook->flags & IT_SPEEDBOOST )
	{
		item_speedboost_execute( NULL, other );
	}
	else if( ihook->flags & IT_ACROBOOST )
	{
		item_acroboost_execute( NULL, other );
	}
	else if( ihook->flags & IT_VITABOOST )
	{
		item_vitaboost_execute( NULL, other );
	}
	else if( ihook->flags & IT_MEGASHIELD )
	{
		item_megashield_execute( self, other );
		self->s.sound = 0;
	}
	else if( ihook->flags & IT_MANASKULL )
	{
		item_manaskull_execute( self, other );		
		self->s.sound = 0;
	}

	//	play pickup sound
	gstate->StartEntitySound(other, CHAN_VOICE, ihook->soundPickupIndex, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);

	
	if( ihook->flags & ( IT_POWERBOOST | IT_ATTACKBOOST | IT_SPEEDBOOST | IT_ACROBOOST | IT_VITABOOST ) ) 
	{
		if (other->flags & FL_CLIENT)
		{
			other->client->bonus_alpha = 0.25;
		}

		// remove 'nucleus' inner model, if exists
		if (ihook->link)
		{
		  userEntity_t *nucleus = ihook->link;
		  com->SetRespawn (nucleus, ihook->respawnTime);
			// NSS[3/4/00]:
		  //Item_SetRespawn(self);
		}
	}

	self->s.modelindex = 0;	//	set model to 0, modelName should still be valid, though
	self->solid = SOLID_NOT;
	self->nextthink = gstate->time + ihook->respawnTime;
	self->think = boost_respawn;

	if ( other->PickedUpItem )
	{
		other->PickedUpItem( other, self->className );
	}
}
// ----------------------------------------------------------------------------
// NSS[3/15/00]:
// Name:        boost_nucleus_think
// Description: think function for the nucleous to assure that it follows the owner
// Input:userEntity_t *self(client)
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void boost_nucleus_think(userEntity_t *self)
{
    if(self)
	{
		if(self->owner)
		{
			self->s.origin = self->owner->s.origin;
			self->s.origin.z += 1.0f;           // height may be specific for each boost
		}
		self->nextthink = gstate->time + 0.20f;
	}
}


void create_boost_nucleus(userEntity_t *self, itemInfo_t *info, char * model_name, int fx_flag)
// creates inner model for powerup boosts
{
    itemHook_t   *ihook;
    userEntity_t *nucleus;

  	// set up info for inner 'nucleus' model
    info->touchFunc = NULL; 
	info->soundPickup = NULL;
	info->soundRespawn = NULL;
	info->soundAmbient = NULL;
    
    nucleus = gstate->SpawnEntity(); // create second 'nucleus' model

	nucleus->think = boost_nucleus_think;
	nucleus->owner = self;
	nucleus->netname = info->netName;
	nucleus->userHook = (itemHook_t*)gstate->X_Malloc(sizeof(itemHook_t),MEM_TAG_HOOK);
	ihook = (itemHook_t *) nucleus->userHook;
	
	nucleus->solid = SOLID_TRIGGER;
	nucleus->movetype = MOVETYPE_TOSS;
	nucleus->s.renderfx = RF_GLOW;
    
	nucleus->clipmask = MASK_PLAYERSOLID;
	nucleus->flags = FL_ITEM;
	nucleus->svflags |= ( SVF_ITEM | SVF_NOPUSHCOLLIDE );	// amw: causes items in a cabinet to not collide with cabinet doors etc..
    nucleus->avelocity.Set(0, 180, 0);

    nucleus->s.origin = self->s.origin;
    nucleus->s.origin.z += 1.0f;           // height may be specific for each boost
    nucleus->s.angle_delta.y = 359;        // rotation per second

    nucleus->s.effects  = EF_ROTATE;                                // rotate
    nucleus->s.effects2 = EF2_ROTATE_CLOCK | EF2_BOOSTFX | fx_flag; // force rotation clockwise, with boost fx

    nucleus->touch = NULL; // no touch function (outside model handles link to inner model)
	
	nucleus->modelName = model_name;

	nucleus->s.modelindex = gstate->ModelIndex (nucleus->modelName);
	nucleus->s.mins = info->mins;
	nucleus->s.maxs = info->maxs;

	ihook->health = info->health;

    if (deathmatch->value && dm_item_respawn->value)  // 2.28 dsn
  	  ihook->respawnTime = info->respawnTime;
    else
      ihook->respawnTime = -1.0f;

	ihook->info = info;
	ihook->flags = info->flags;
	
	ihook->soundPickupIndex = 0;
	ihook->soundRespawnIndex = 0;
	ihook->soundSpecialIndex = 0;

    nucleus->soundAmbientIndex = 0;

    // link items together for clean-up references
    ihook = (itemHook_t *) self->userHook;
    ihook->link = nucleus;

    gstate->LinkEntity (nucleus);
	// NSS[3/15/00]:
	self->nextthink = gstate->time + 0.20f;
}



///////////////////////////////////////////////////////////////////////////////
// item_power_boost
///////////////////////////////////////////////////////////////////////////////

void	item_power_boost (userEntity_t *self)
{
	if (IS_INSTAGIB || (deathmatch->value && !dm_allow_powerups->value))
		return;

	itemInfo_t	 *info;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	info->netName = tongue[T_USEBOOST_POWER];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue,T_USEBOOST_POWER);

	info->touchFunc = item_boost_touch;

	info->mins.Set(-8, -8, -16);
	info->maxs.Set(8, 8, 16);

	info->flags |= IT_POWERBOOST;
	info->respawnTime = BOOST_RESPAWN_TIME;

	info->soundPickup = "global/a_pboost.wav"; 
	info->soundRespawn = "global/a_itspwn.wav";
	info->soundAmbient = NULL;

	self->avelocity.Set(0, 180, 0);

	item_Spawn (self, info, MOVETYPE_TOSS, TRUE, "models/global/a_pwrb.dkm");

    self->s.angle_delta.y = 270;          // rotation per second   359 is fastest
    self->s.alpha = 0.7f;                 // translucency
    self->s.renderfx = RF_TRANSLUCENT;   
    self->s.renderfx &= ~RF_GLOW;         // do not glow outer model

    create_boost_nucleus(self, info, "models/global/a_pwrc.dkm", EF2_BOOSTFX_POWER); 
}

///////////////////////////////////////////////////////////////////////////////
// item_attack_boost
///////////////////////////////////////////////////////////////////////////////

void	item_attack_boost (userEntity_t *self)
{
	itemInfo_t	*info;

	if (deathmatch->value && !dm_allow_powerups->value)
		return;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	info->netName = tongue[T_USEBOOST_ATTACK];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue,T_USEBOOST_ATTACK);
	
	info->touchFunc = item_boost_touch;

	info->mins.Set(-8, -8, -16);
	info->maxs.Set(8, 8, 16);

	info->flags |= IT_ATTACKBOOST;
	info->respawnTime = BOOST_RESPAWN_TIME;

	info->soundPickup = "global/a_atkboost.wav";
	info->soundRespawn = "global/a_itspwn.wav";
	info->soundAmbient = NULL;

	self->avelocity.Set(0, 180, 0);

	item_Spawn (self, info, MOVETYPE_TOSS, TRUE, "models/global/a_atkb.dkm");

    self->s.angle_delta.y = 270;          // rotation per second   359 is fastest
    self->s.alpha = 0.7f;                 // translucency
    self->s.renderfx = RF_TRANSLUCENT;   
    self->s.renderfx &= ~RF_GLOW;         // do not glow outer model

    create_boost_nucleus(self, info, "models/global/a_atkc.dkm", EF2_BOOSTFX_ATTACK); 
}


///////////////////////////////////////////////////////////////////////////////
// item_speed_boost
///////////////////////////////////////////////////////////////////////////////

void	item_speed_boost (userEntity_t *self)
{
	itemInfo_t	*info;

	if (deathmatch->value && !dm_allow_powerups->value )
	  return;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	info->netName = tongue[T_USEBOOST_SPEED];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue,T_USEBOOST_SPEED);

	info->touchFunc = item_boost_touch;

	info->mins.Set(-8, -8, -16);
	info->maxs.Set(8, 8, 16);

	info->flags |= IT_SPEEDBOOST;
	info->respawnTime = BOOST_RESPAWN_TIME;

	info->soundPickup = "global/a_sboost.wav";
	info->soundRespawn = "global/a_itspwn.wav";
	info->soundAmbient = NULL;

	self->avelocity.Set(0, 180, 0);

	item_Spawn (self, info, MOVETYPE_TOSS, TRUE, "models/global/a_spdb.dkm");

    self->s.angle_delta.y = 270;          // rotation per second   359 is fastest
    self->s.alpha = 0.7f;                 // translucency
    self->s.renderfx = RF_TRANSLUCENT;   
    self->s.renderfx &= ~RF_GLOW;         // do not glow outer model

    create_boost_nucleus(self, info, "models/global/a_spdc.dkm", EF2_BOOSTFX_SPEED); 
}


///////////////////////////////////////////////////////////////////////////////
// item_acro_boost
///////////////////////////////////////////////////////////////////////////////

void	item_acro_boost (userEntity_t *self)
{
	itemInfo_t   *info;
    
	if (deathmatch->value && !dm_allow_powerups->value)
		return;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	info->netName = tongue[T_USEBOOST_ACRO];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue,T_USEBOOST_ACRO);
	
	info->touchFunc = item_boost_touch;

	info->mins.Set(-8, -8, -16);
	info->maxs.Set(8, 8, 16);

	info->flags |= IT_ACROBOOST;
	info->respawnTime = BOOST_RESPAWN_TIME;

	info->soundPickup = "global/a_aboost.wav";
	info->soundRespawn = "global/a_itspwn.wav";
	info->soundAmbient = NULL;

	self->avelocity.Set(0, 180, 0);
     
    item_Spawn (self, info, MOVETYPE_TOSS, TRUE, "models/global/a_acrb.dkm"); // outside model

    self->s.angle_delta.y = 270;          // rotation per second   359 is fastest
    self->s.alpha = 0.7f;                 // translucency
    self->s.renderfx = RF_TRANSLUCENT;   
    self->s.renderfx &= ~RF_GLOW;         // do not glow outer model
 
    create_boost_nucleus(self, info, "models/global/a_acrc.dkm", EF2_BOOSTFX_ACRO); 
}



///////////////////////////////////////////////////////////////////////////////
// item_vita_boost
///////////////////////////////////////////////////////////////////////////////

void	item_vita_boost (userEntity_t *self)
{
	if (IS_INSTAGIB || (deathmatch->value && !dm_allow_health->value) || (deathmatch->value && !dm_allow_powerups->value))
		return;

	itemInfo_t	*info;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	info->netName = tongue[T_USEBOOST_VITA];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue,T_USEBOOST_VITA);
	info->touchFunc = item_boost_touch;

	info->mins.Set(-8, -8, -16);
	info->maxs.Set(8, 8, 16);

	info->flags |= IT_VITABOOST;
	info->respawnTime = BOOST_RESPAWN_TIME;

	info->soundPickup = "global/a_vboost.wav";
	info->soundRespawn = "global/a_itspwn.wav";
	info->soundAmbient = NULL;

	self->avelocity.Set(0, 180, 0);

	item_Spawn (self, info, MOVETYPE_TOSS, TRUE, "models/global/a_vtlb.dkm");

    self->s.angle_delta.y = 270;          // rotation per second   359 is fastest
    self->s.alpha = 0.7f;                 // translucency
    self->s.renderfx = RF_TRANSLUCENT;   
    self->s.renderfx &= ~RF_GLOW;         // do not glow outer model

    create_boost_nucleus(self, info, "models/global/a_vtlc.dkm", EF2_BOOSTFX_VITA); 
}



///////////////////////////////////////////////////////////////////////////////////
//	item_bomb
//
//	items for making e4m1 bomb
///////////////////////////////////////////////////////////////////////////////////

#define	BOMB_SULPHUR	0x01
#define	BOMB_CHARCOAL	0x02
#define	BOMB_SALTPETER	0x04

static	char	bombMsg [128];


///////////////////////////////////////////////////////////////////////////////////
//	item_bomb_kapow
//
//
///////////////////////////////////////////////////////////////////////////////////

void	item_bomb_kapow (userEntity_t *self)
{
	CVector	org, saveOrg;

	self->s.modelindex = 0;
	org = self->s.origin;
	if (self->delay < 6)
	{
		org.x += rnd() * 64 - 32;
		org.y += rnd() * 64 - 32;
		org.z += rnd() * 64;
	}
	spawn_sprite_explosion (self, org, true);

	saveOrg = self->s.origin;
	self->s.origin = org;
	com->RadiusDamage (self, self->owner, NULL, 200, 0, 128.0 );
	self->s.origin = saveOrg;

	self->delay -= 1.0;

	if (self->delay > 0.0)
	{
		self->nextthink = gstate->time + 0.1;
	}
	else
	{
		self->remove (self);
	}
}

///////////////////////////////////////////////////////////////////////////////////
//	item_bomb_countdown
//
///////////////////////////////////////////////////////////////////////////////////

void	item_bomb_countdown (userEntity_t *self)
{
	char	msg [8];

//	sprintf (msg, "%i...", self->hacks_int);
	Com_sprintf (msg, sizeof(msg),"%i...", self->hacks_int);
	gstate->centerprint (self->owner, 1.0, msg);

	self->nextthink =gstate->time + 1.0;

	self->hacks_int--;
	if (self->hacks_int == 0)
		self->think = item_bomb_kapow;
}

///////////////////////////////////////////////////////////////////////////////////
//	item_bomb_explode
//
//	This is the use for the item_bomb, which causes it to drop from the player's
//	inventory and wait a small amount of time before blowing up.
///////////////////////////////////////////////////////////////////////////////////

void	item_bomb_explode (struct userInventory_s *item, struct edict_s *user)
{
	userEntity_t	*bomb;
	CVector			ang;

	bomb = gstate->SpawnEntity ();
	bomb->movetype = MOVETYPE_TOSS;
	bomb->solid = SOLID_BBOX;
	bomb->owner = user;
	bomb->s.mins.Set(-16, -16, 0);
	bomb->s.maxs.Set(16, 16, 40);

	ang = user->s.angles;
	ang.AngleToVectors(forward, right, up);

	ang = forward * 100;
	ang.z = 100;
	bomb->velocity = ang;

	bomb->s.modelindex = item->modelIndex;
	bomb->s.frame = 0;
	bomb->delay = 6;

	bomb->s.origin = user->s.origin;
	gstate->LinkEntity (bomb);

	gstate->InventoryDeleteItem (user, user->inventory, item);

	gstate->centerprint (user, 1.0, "%s!\n",tongue[T_BOMB_EXPLODE]);

	bomb->think = item_bomb_countdown;
	bomb->hacks_int = 5;
	bomb->nextthink = gstate->time + 1.0;
}

///////////////////////////////////////////////////////////////////////////////////
//	item_bomb_use
//
//	All bomb parts have this for their use.  Using any bomb part when all
//	bomb parts are collected will remove them all and create the bomb item.
///////////////////////////////////////////////////////////////////////////////////

short item_bomb_message(userEntity_t *self)
{
	userInventory_t	*sulphur, *bottle, *charcoal, *saltpeter, *bomb;

	bomb	  = gstate->InventoryFindItem (self->inventory, "item_bomb");
	if (bomb)
		return TRUE;

	sulphur   = gstate->InventoryFindItem (self->inventory, "item_sulphur");
	bottle    = gstate->InventoryFindItem (self->inventory, "item_bottle");
	charcoal  = gstate->InventoryFindItem (self->inventory, "item_charcoal");
	saltpeter = gstate->InventoryFindItem (self->inventory, "item_saltpeter");

	Com_sprintf(bombMsg,sizeof(bombMsg),"%s\n",tongue[T_BOMB_ITEM_REQUIRED1]);

	if (!bottle)
	{
		strcat (bombMsg, tongue[T_BOMB_BOTTLE]);
	}

	if (!sulphur)
	{
		if (!bottle)
			strcat (bombMsg, ", ");
		strcat (bombMsg, tongue[T_BOMB_INGREDIENT_SHORT_1]);
	}
	if (!charcoal)
	{
		if (!bottle || !sulphur)
			strcat (bombMsg, ", ");
		strcat (bombMsg, tongue[T_BOMB_INGREDIENT_SHORT_2]);
	}
	if (!saltpeter)
	{
		if (!bottle || !sulphur || !charcoal)
			strcat (bombMsg, ", ");
		strcat (bombMsg, tongue[T_BOMB_INGREDIENT_SHORT_3]);
	}

	strcat (bombMsg, ".\n");
	gstate->centerprint (self, 2.0, bombMsg);
	return FALSE;
}

void	item_bombpart_use (struct userInventory_s *item, struct edict_s *user)
{

	userInventory_t	*sulphur, *bottle, *charcoal, *saltpeter, *bomb;
	int				flags = 0, count = 3, listed = 0;
	int				i, bits, modelIndex;

	char			*itemNames [3] = { tongue[T_BOMB_INGREDIENT_SHORT_1],  // sulphur
	                             tongue[T_BOMB_INGREDIENT_SHORT_2],  // charcoal
                               tongue[T_BOMB_INGREDIENT_SHORT_3]   // saltpeter
                             };

	if (gstate->InventoryFindItem(user->inventory, "item_bomb"))
		return;

	sulphur   = gstate->InventoryFindItem (user->inventory, "item_sulphur");
	bottle    = gstate->InventoryFindItem (user->inventory, "item_bottle");
	charcoal  = gstate->InventoryFindItem (user->inventory, "item_charcoal");
	saltpeter = gstate->InventoryFindItem (user->inventory, "item_saltpeter");

	if (!bottle)
	{
		gstate->centerprint (user, 2.0, "%s\n",tongue[T_BOMB_NEED_BOTTLE]);
		return;
	}

	if (sulphur)
	{
		flags |= BOMB_SULPHUR;
		count--;
	}
	if (charcoal)
	{
		flags |= BOMB_CHARCOAL;
		count--;
	}
	if (saltpeter)
	{
		flags |= BOMB_SALTPETER;
		count--;
	}

	if (count != 0)
	{
//		sprintf(bombMsg,"%s ",tongue[T_BOMB_ITEM_REQUIRED1]);
		Com_sprintf(bombMsg,sizeof(bombMsg),"%s ",tongue[T_BOMB_ITEM_REQUIRED1]);
		
		for (bits = 1, i = 0; i < 3; i++, bits <<= 1)
		{
			if (!(flags & bits))
			{
				listed++;
			
				strcat (bombMsg, itemNames [i]);

				if (listed != count)  // not last item to display?
				{
					//	append a comma
					strcat (bombMsg, ", ");
				}
			}
		}

		strcat (bombMsg, ".\n");

		gstate->centerprint (user, 2.0, bombMsg);

		return;
	}


	modelIndex = gstate->ModelIndex ("models/e4/a_bomb.dkm");
	bomb = gstate->InventoryCreateItem (user->inventory, "item_bomb", 
		NULL, NULL, modelIndex, ITF_SPECIAL | ITF_INVSACK | ITF_USEONCE | ITF_COOP_REMOVEALL, sizeof (userInventory_t));

	//	remove the bomb parts
	gstate->InventoryDeleteItem (user, user->inventory, bottle);
	gstate->InventoryDeleteItem (user, user->inventory, sulphur);
	gstate->InventoryDeleteItem (user, user->inventory, charcoal);
	gstate->InventoryDeleteItem (user, user->inventory, saltpeter);

	//	add the bomb itself
	gstate->InventoryAddItem (user, user->inventory, bomb);

	gstate->centerprint (user, 2.0, "%s\n",tongue[T_BOMB_CREATED]);
}

///////////////////////////////////////////////////////////////////////////////////
//	item_bombpart_think
//
//	restores the touch function for a bomb object
///////////////////////////////////////////////////////////////////////////////////

void	item_bombpart_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf);

void	item_bombpart_think (userEntity_t *self)
{
	self->touch = item_bombpart_touch;
	self->nextthink = -1;
}

///////////////////////////////////////////////////////////////////////////////////
//	item_bomb_touch
//
///////////////////////////////////////////////////////////////////////////////////

void item_bombpart_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	userInventory_t	*invItem;

	//NSS[11/30/99]:get hook
	itemHook_t		*ihook = (itemHook_t *) self->userHook;

	if (!(other->flags & FL_CLIENT))
		return;

	if (!self->className)
		return;

	// don't need these anymore...you have da bom
	if (gstate->InventoryFindItem (other->inventory, "item_bomb"))
		return;

	invItem = gstate->InventoryFindItem (other->inventory, self->className);
	if (invItem)
		return;
	
	//NSS[11/30/99]: play pickup sound 
	gstate->StartEntitySound(other, CHAN_VOICE, ihook->soundPickupIndex, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);

	//	FIXME:	change to ITF_SPECIAL after testing
	invItem = gstate->InventoryCreateItem (other->inventory, self->className, 
		item_bombpart_use, NULL, self->s.modelindex, ITF_SPECIAL | ITF_INVSACK, sizeof (userInventory_t));

	if (!invItem)
		return;

	if (ihook->info->respawnFlags & ITEM_RESP_COOP_REMOVEALL)
		invItem->flags |= ITF_COOP_REMOVEALL;

	gstate->InventoryAddItem (other, other->inventory, invItem);

	if (other->flags & FL_CLIENT)
	{
		other->client->bonus_alpha = 0.25;
#ifndef JPN // Original
		gstate->centerprint (other, 2.0, "%s %s\n", tongue[T_BOMB_FOUND],self->netname);
#else // 
 #ifdef MAN // for MAN
		gstate->centerprint (other, 2.0, "%s %s\n", tongue[T_BOMB_FOUND],self->netname);
 #else
  #ifdef TIW // for TIW
		gstate->centerprint (other, 2.0, "%s %s\n", tongue[T_BOMB_FOUND],self->netname);
  #else
		gstate->centerprint (other, 2.0, "%s %s\n",self->netname, tongue[T_BOMB_FOUND]);
  #endif // TIW
 #endif // MAN
#endif // JPN
	}

	item_bomb_part_check_for_all( other );

	// remove the entity
	if (!(coop->value && (ihook->info->respawnFlags & ITEM_RESP_COOP_STAY)))
	{
		self->touch = NULL;
		self->think = NULL;
		self->nextthink = -1;

		com->SetRespawn (self, -1);
	}
}

///////////////////////////////////////////////////////////////////////////////////
//	item_sulphur
//
///////////////////////////////////////////////////////////////////////////////////

void	item_sulphur (userEntity_t *self)
{
	itemInfo_t	*info;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	info->netName = tongue[T_BOMB_INGREDIENT_1];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue,T_BOMB_INGREDIENT_1);
	info->touchFunc = item_bombpart_touch;

	info->mins.Set(-16, -16, -24);
	info->maxs.Set(16, 16, 5);

	info->flags |= FL_ITEM;
	info->respawnTime = -1.0;	//	never respawns

	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;
	info->soundPickup  = "e4/sulpickup.wav";//"global/a_specialitem.wav"; //NSS[11/30/99]:
	info->soundRespawn = NULL;
	info->soundAmbient = NULL;

	self->avelocity.Set(0, 180, 0);

	item_Spawn (self, info, MOVETYPE_TOSS, FALSE, "models/e4/a_sulphur.dkm");
}

///////////////////////////////////////////////////////////////////////////////////
//	item_charcoal
//
///////////////////////////////////////////////////////////////////////////////////

void	item_charcoal (userEntity_t *self)
{
	itemInfo_t	*info;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	info->netName = tongue[T_BOMB_INGREDIENT_2];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue,T_BOMB_INGREDIENT_2);
	info->touchFunc = item_bombpart_touch;

	info->mins.Set(-16, -16, -24);
	info->maxs.Set(16, 16, -5);

	info->flags |= FL_ITEM;
	info->respawnTime = -1.0;	//	never respawns

	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;
	info->soundPickup  = "e4/sulpickup.wav";//"global/a_specialitem.wav"; //NSS[11/30/99]:
	info->soundRespawn = NULL;
	info->soundAmbient = NULL;

	self->avelocity.Set(0, 180, 0);

	item_Spawn (self, info, MOVETYPE_TOSS, FALSE, "models/e4/a_charcoal.dkm");
}

///////////////////////////////////////////////////////////////////////////////////
//	item_saltpeter
//
///////////////////////////////////////////////////////////////////////////////////

void	item_saltpeter (userEntity_t *self)
{
	itemInfo_t	*info;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	info->netName = tongue[T_BOMB_INGREDIENT_3];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue,T_BOMB_INGREDIENT_3);
	info->touchFunc = item_bombpart_touch;

	info->mins.Set(-16, -16, -24);
	info->maxs.Set(16, 16, 5);

	info->flags |= FL_ITEM;
	info->respawnTime = -1.0;	//	never respawns

	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;
	info->soundPickup  = "e4/saltpickup.wav";//"global/a_specialitem.wav"; //NSS[11/30/99]:
	info->soundRespawn = NULL;
	info->soundAmbient = NULL;

	self->avelocity.Set(0, 180, 0);

	item_Spawn (self, info, MOVETYPE_TOSS, FALSE, "models/e4/a_saltp.dkm");
}

///////////////////////////////////////////////////////////////////////////////////
//	item_bottle
//
///////////////////////////////////////////////////////////////////////////////////

void	item_bottle (userEntity_t *self)
{
	itemInfo_t	*info;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	info->netName = tongue[T_BOMB_BOTTLE];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue,T_BOMB_BOTTLE);
	info->touchFunc = item_bombpart_touch;

	info->mins.Set(-16, -16, -24);
	info->maxs.Set(16, 16, 7);

	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;
	info->flags |= FL_ITEM;
	info->respawnTime = -1.0;	//	never respawns

	info->soundPickup  = "e4/empbotpickup.wav";//"global/a_specialitem.wav"; //NSS[11/30/99]:
	info->soundRespawn = NULL;
	info->soundAmbient = NULL;

	self->avelocity.Set(0, 180, 0);

	item_Spawn (self, info, MOVETYPE_TOSS, TRUE, "models/e4/a_bottle.dkm");
}

///////////////////////////////////////////////////////////////////////////////
//
// KEY SYSTEM
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
//	item_book_use
//
//	if a book does not have a target, this function is basically ignored
///////////////////////////////////////////////////////////////////////////////////

void	item_book_use (struct userInventory_s *item, struct edict_s *user)
{
	keyItem_t		*keyItem = (keyItem_t*)item;
	userInventory_t	*invItem = (userInventory_t*) keyItem;
	float			dist = 0;

	if (keyItem->target)
	{
		userEntity_t *target = com->FindTarget (keyItem->target);
		if (target)
		{
			// is it in direct site and not behind a wall etc.. ?
			dist = com->Visible (user, target);
			if ((dist > 0) && (dist < 68.0))
			{
				CVector	angles, entPos, entVector, forward;
				float	dotProduct = 0.0;

				if (target->movetype == MOVETYPE_PUSH || target->solid == SOLID_BSP || target->solid == SOLID_TRIGGER)
				{
					entPos = (target->absmin + target->absmax) * 0.5;
				}
				else 
				{
					entPos = target->s.origin;
				}

				// check to see if we're facing the target				
				entVector = entPos - user->s.origin;
				angles = user->s.angles;
				angles.AngleToForwardVector(forward);

				dotProduct = DotProduct( forward, entVector );

				if (dotProduct > 0)
				{
					CVector	maxs(32, 32, 32);
					CVector mins(-32, -32, 0);

					// create spiral particle effect
					gstate->WriteByte(SVC_TEMP_ENTITY);
					gstate->WriteByte(TE_SPIRAL_PARTICLES);
					gstate->WritePosition(entPos);
					gstate->WritePosition(mins);
					gstate->WritePosition(maxs);
					gstate->WriteByte(5);	// display for five seconds
					gstate->MultiCast(entPos,MULTICAST_PVS);

					//	play pickup sound
					if (keyItem->soundIndex)
					{
						gstate->StartEntitySound(user, CHAN_VOICE, keyItem->soundIndex, 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
					}
// changed by yokoyama for Japanese version
#ifndef JPN // Original
					gstate->centerprint (user, 2.0, "%s %s!\n", tongue[T_BOOK_READ],keyItem->netname);
					//artifact_special_print(T_BOOK_READ, user, "%s %s.\n", 2, tongue, ->netNameID);
#else // for Japanese
 #ifdef MAN // for MAN
					gstate->centerprint (user, 2.0, "%s %s!\n", tongue[T_BOOK_READ],keyItem->netname);
					//artifact_special_print(T_BOOK_READ, user, "%s %s.\n", 2, tongue, ->netNameID);
 #else
  #ifdef TIW // for TIW
					gstate->centerprint (user, 2.0, "%s %s!\n", tongue[T_BOOK_READ],keyItem->netname);
					//artifact_special_print(T_BOOK_READ, user, "%s %s.\n", 2, tongue, ->netNameID);
  #else
					gstate->centerprint (user, 2.0, "%s ‚ð%s!\n",keyItem->netname, tongue[T_BOOK_READ]);
					//artifact_special_print(T_BOOK_READ, user, "%s %s.\n", 2, tongue, ->netNameID);
 
  #endif // TIW
 #endif // MAN
#endif // JPN

					// now use the target... most likely it's some type of trigger
					gstate->targeter = user;
					target->use (target, user, user);

					// see if this key can only be used once.. if so, delete it
					if (invItem->flags & ITF_USEONCE)
					{
						gstate->InventoryDeleteItem (user, user->inventory, invItem);
					}
					
					return;
				}
			}
		}
		gstate->centerprint (user, 2.0, "%s!\n",tongue[T_BOOK_NO_READ]);
		//artifact_special_print(T_BOOK_NO_READ, user, "%s %s.\n", 2, tongue, self->netNameID);
 
			
	}
	else
	{
// changed by yokoyama for Japanese version
#ifndef JPN // Original
		gstate->centerprint (user, 2.0, "%s %s!\n", tongue[T_BOOK_READ],keyItem->netname);
#else
 #ifdef MAN // for MAN
		gstate->centerprint (user, 2.0, "%s %s!\n", tongue[T_BOOK_READ],keyItem->netname);
 #else
  #ifdef TIW // for TIW
		gstate->centerprint (user, 2.0, "%s %s!\n", tongue[T_BOOK_READ],keyItem->netname);
  #else
        gstate->centerprint (user, 2.0, "%s ‚ð%s!\n",keyItem->netname, tongue[T_BOOK_READ]);
  #endif // TIW
 #endif // MAN
#endif // JPN
	}

	return;

}


///////////////////////////////////////////////////////////////////////////////
//NSS[11/22/99]:
//item_add_hook_flags
//Adds flags and related information to the player hook
//Notes:
//NSS[11/24/99]:Added rings
///////////////////////////////////////////////////////////////////////////////
void item_add_hook_flags(userEntity_t *item, userEntity_t *person)
{
	playerHook_t	*phook = AI_GetPlayerHook( person );
	

	if(phook)
	{
		if(_stricmp(item->className,"item_envirosuit")==0)
		{
			phook->items |= IT_ENVIROSUIT;
			phook->envirosuit_time = 400;  //Roughly 1 minute of time.
		}
		else if(_stricmp(item->className,"item_ring_of_fire")==0)		//75% protection from stavros's attacks
		{
			phook->items |= IT_RINGOFIRE;
		}
		else if(_stricmp(item->className,"item_ring_of_lightning")==0)	//75% protection from wyndrax's attacks
		{
			phook->items |= IT_RINGOFLIGHTNING;
		}
		else if(_stricmp(item->className,"item_ring_of_undead")==0)		//75% protection from Nharre's attacks
		{
			phook->items |= IT_RINGOFUNDEAD;
		}
		else if (_stricmp(item->className,"item_antidote")==0)			// cek[2-1-00]: add antidote
		{
			phook->items |= IT_ANTIDOTE;
			if (person->client)
				person->client->pers.fAntidotePoints += 100.0f;

			// count these...only need one of em
			userInventory_t *antidote = NULL;
			while (gstate->InventoryItemCount(person->inventory,"item_antidote") > 1)
			{
				antidote = gstate->InventoryFindItem(person->inventory,"item_antidote");
				gstate->InventoryDeleteItem(person,person->inventory,antidote);
			}
		}
	}
	
}

void item_bomb_part_check_for_all( userEntity_t *self )
{
	if( ( self == NULL ) || ( self->inventory == NULL ) )
	{
		return;
	}

	if (gstate->InventoryFindItem(self->inventory, "item_bomb"))
		return;

	int nHas1 = gstate->InventoryItemCount( self->inventory, "item_sulphur" );
	int nHas2 = gstate->InventoryItemCount( self->inventory, "item_charcoal" );
	int nHas3 = gstate->InventoryItemCount( self->inventory, "item_saltpeter" );
	int nHas4 = gstate->InventoryItemCount( self->inventory, "item_bottle" );

	int nModelIndex = gstate->ModelIndex( "models/e4/a_bomb.dkm" );
	if( ( nHas1 != 0 ) && 
		( nHas2 != 0 ) && 
		( nHas3 != 0 ) && 
		( nHas4 != 0 ) )
	{
		userInventory_t *pBomb = gstate->InventoryCreateItem( self->inventory, 
			"item_bomb", 
			NULL, 
			NULL, 
			nModelIndex, 
			( ITF_SPECIAL | ITF_INVSACK | ITF_USEONCE | ITF_COOP_REMOVEALL), 
			sizeof( userInventory_t ) );

		userInventory_t *pSulphur   = gstate->InventoryFindItem( self->inventory, "item_sulphur" );
		userInventory_t *pBottle    = gstate->InventoryFindItem( self->inventory, "item_bottle" );
		userInventory_t *pCharcoal  = gstate->InventoryFindItem( self->inventory, "item_charcoal" );
		userInventory_t *pSaltpeter = gstate->InventoryFindItem( self->inventory, "item_saltpeter" );

		//	remove the bomb parts
		gstate->InventoryDeleteItem( self, self->inventory, pBottle );
		gstate->InventoryDeleteItem( self, self->inventory, pSulphur );
		gstate->InventoryDeleteItem( self, self->inventory, pCharcoal );
		gstate->InventoryDeleteItem( self, self->inventory, pSaltpeter );

		//	add the bomb itself
		gstate->InventoryAddItem( self, self->inventory, pBomb );

		gstate->centerprint( self, 2.0, "%s\n",tongue[T_BOMB_CREATED] );
	}
}

void item_rune_check_for_all( userEntity_t *self )
{
	if( ( self == NULL ) || ( self->inventory == NULL ) )
	{
		return;
	}

	int nHas1 = gstate->InventoryItemCount( self->inventory, "item_rune_a" );
	int nHas2 = gstate->InventoryItemCount( self->inventory, "item_rune_e" );
	int nHas3 = gstate->InventoryItemCount( self->inventory, "item_rune_g" );
	int nHas4 = gstate->InventoryItemCount( self->inventory, "item_rune_i" );
	int nHas5 = gstate->InventoryItemCount( self->inventory, "item_rune_s" );

	if( ( nHas1 != 0 ) && 
		( nHas2 != 0 ) && 
		( nHas3 != 0 ) && 
		( nHas4 != 0 ) && 
		( nHas5 != 0 ) )
	{
		gstate->centerprint( self, 2.0, "%s\n", tongue[T_AEGIS_FOUND] );
	}
}

void item_keystone_check_for_all( userEntity_t *self )
{
	if( ( self == NULL ) || ( self->inventory == NULL ) )
	{
		return;
	}

	int nHas1 = gstate->InventoryItemCount( self->inventory, "item_hex_keystone" );
	int nHas2 = gstate->InventoryItemCount( self->inventory, "item_quad_keystone" );
	int nHas3 = gstate->InventoryItemCount( self->inventory, "item_trigon_keystone" );

	if( ( nHas1 != 0 ) && 
		( nHas2 != 0 ) && 
		( nHas3 != 0 ) )
	{
		gstate->centerprint( self, 2.0, "%s\n", tongue[T_KEYSTONES_FOUND] );
	}
}

void item_purifier_check_for_all( userEntity_t *self )
{
	if( ( self == NULL ) || ( self->inventory == NULL ) )
	{
		return;
	}

	int nHas1 = gstate->InventoryItemCount( self->inventory, "item_purifier_shard1" );

	int nHas2 = 0;//gstate->InventoryItemCount( self->inventory, "item_purifier_shard2" );
	char name[32];
	for (int i = 1; i <= 5; i++)
	{
		Com_sprintf(name,sizeof(name),"item_purifier_shard2_%d",i);
		nHas2 += gstate->InventoryItemCount( self->inventory, name );
	}

	int nHas3 = gstate->InventoryItemCount( self->inventory, "item_purifier_shard3" );

	if( ( nHas1 == 1 ) && 
		( nHas2 == 5 ) && 
		( nHas3 == 1 ) )
	{
		gstate->centerprint( self, 2.0, "%s\n", tongue[T_PURIFIER_FOUND] );
	}
}

void item_add_to_inventory( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	userInventory_t	*invItem = NULL;
	keyItem_t		*keyItem = NULL;

	if( !com->ValidTouch( self, other ) )
		return;

	if( !self->className )
		return;

	if( !( other->flags & ( FL_CLIENT)) )
	{
		return;
	}

	// don't add these over and over again!
	if (gstate->InventoryFindItem(other->inventory,self->className))
		return;

	playerHook_t	*ohook = AI_GetPlayerHook( other );
	itemHook_t		*ihook = (itemHook_t *) self->userHook;

	invItem = gstate->InventoryCreateItem( 
		other->inventory, 
		self->className,
		NULL,
		NULL,
		self->s.modelindex,
		ITF_SPECIAL | ITF_INVSACK,
		sizeof( userInventory_t ) );

	if ( (ihook->info->respawnFlags & ITEM_RESP_USE_ONCE) || (self->spawnflags & KEY_USE_ONCE) )
		invItem->flags |= ITF_USEONCE;

	if (ihook->info->respawnFlags & ITEM_RESP_COOP_REMOVEALL)
		invItem->flags |= ITF_COOP_REMOVEALL;

	gstate->InventoryAddItem (other, other->inventory, invItem);

	//	play pickup sound
	gstate->StartEntitySound(other, CHAN_VOICE, ihook->soundPickupIndex, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);

	// use the key's target if it is specified
	if (self->spawnflags & KEY_USETARGETS)
	{
		com->UseTargets (self, other, other);
	}

	if (other->flags & FL_CLIENT)
	{
		other->client->bonus_alpha = 0.25;
// changed by yokoyama for Japanese version
#ifndef JPN // Original
		artifact_special_print(T_KEY_PICKUP, other, "%s %s.\n", 2, tongue, self->netNameID);
		//gstate->centerprint( other, 2.0, "%s %s\n", tongue[T_KEY_PICKUP], self->netname );
#else // JPN
 #ifdef MAN // for MAN
		artifact_special_print(T_KEY_PICKUP, other, "%s %s.\n", 2, tongue, self->netNameID);
		//gstate->centerprint( other, 2.0, "%s %s\n", tongue[T_KEY_PICKUP], self->netname );
 #else
  #ifdef TIW // for TIW
		artifact_special_print(T_KEY_PICKUP, other, "%s %s.\n", 2, tongue, self->netNameID);
		//gstate->centerprint( other, 2.0, "%s %s\n", tongue[T_KEY_PICKUP], self->netname );
  #else
		artifact_special_print(T_KEY_PICKUP, other, "%s %s.\n", 2, tongue, self->netNameID);
		//gstate->centerprint( other, 2.0, "%s %s\n", self->netname, tongue[T_KEY_PICKUP] );
  #endif // TIW
 #endif // MAN
#endif // JPN
	}

	if( strstr( self->className, "rune" ) )
	{
		item_rune_check_for_all( other );
	}
	else if( strstr( self->className, "purifier" ) )
	{
		item_purifier_check_for_all( other );
	}
	else if( strstr( self->className, "keystone" ) )
	{
		item_keystone_check_for_all( other );
	}
	
	item_add_hook_flags(self,other);
	// never respawns
	// cek[2-2-00] actually, some do.... but those that don't have ihook->respawnTime as -1 which will remove the entity

	if ( !(coop->value && (ihook->info->respawnFlags & ITEM_RESP_COOP_STAY)) )
	{
		//com->SetRespawn (self, ihook->respawnTime);
		// NSS[3/4/00]:
		Item_SetRespawn(self);
	}
}
///////////////////////////////////////////////////////////////////////////////
// item_key_touch
//
///////////////////////////////////////////////////////////////////////////////

void	item_key_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	userInventory_t	*invItem = NULL;
	keyItem_t		*keyItem = NULL;

	if (!com->ValidTouch (self, other))
		return;

	if (!self->className)
		return;

	// NSS[2/8/00]:Sidekicks cannot touch these things
	if(other->flags & FL_BOT)
		return;

	playerHook_t	*ohook = AI_GetPlayerHook( other );
	itemHook_t		*ihook = (itemHook_t *) self->userHook;

	// don't add these over and over again!
	if (gstate->InventoryFindItem(other->inventory,self->className))
		return;

	// if this is a spell book.. give it an inventory use function
	if (strstr(self->className, "book")) 
	{
		keyItem = (keyItem_t*) gstate->InventoryCreateItem (other->inventory, self->className, 
			item_book_use, NULL, self->s.modelindex, ITF_INVSACK, sizeof (keyItem_t));
	}
	else
	{
		invItem = gstate->InventoryCreateItem (other->inventory, self->className, 
			NULL, NULL, self->s.modelindex, ITF_SPECIAL | ITF_INVSACK, sizeof (userInventory_t));
	}

	if (!keyItem && !invItem)
		return;

	// if this is a book
	if (keyItem)
	{
		invItem = (userInventory_t*)&keyItem->invRec;
		if (self->target && strlen(self->target) < sizeof(keyItem->target))
			strcpy(keyItem->target, self->target);
		if (self->netname && strlen(self->netname) < sizeof(keyItem->netname))
			strcpy(keyItem->netname, self->netname);
		else
			strcpy(keyItem->netname, self->className);
		keyItem->soundIndex = ihook->soundSpecialIndex;
		keyItem->invRec.flags |= ITF_KEY_ITEM;
	}

	// if this key is only supposed to be used once, set the correct flag 
	if (self->spawnflags & KEY_USE_ONCE)
		invItem->flags |= ITF_USEONCE;

	// if this key is only supposed to be used once, set the correct flag 
	if (self->spawnflags & KEY_UNLOCK)
		invItem->flags |= ITF_UNLOCK;

	// add keys to 'sack' inventory
	invItem->flags |= ITF_INVSACK;

	if (ihook->info->respawnFlags & ITEM_RESP_COOP_REMOVEALL)
		invItem->flags |= ITF_COOP_REMOVEALL;

	gstate->InventoryAddItem (other, other->inventory, invItem);

	//	play pickup sound
	gstate->StartEntitySound(other, CHAN_VOICE, ihook->soundPickupIndex, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);

	// use the key's target if it is specified
	if (self->spawnflags & KEY_USETARGETS)
	{
		com->UseTargets (self, other, other);
	}

	if (other->flags & FL_CLIENT)
	{
		other->client->bonus_alpha = 0.25;
// changed by yokoyama for Japanese version
#ifndef JPN // Original
		//gstate->centerprint (other, 2.0, "%s %s\n", tongue[T_KEY_PICKUP],self->netname);
		artifact_special_print(T_KEY_PICKUP, other, "%s %s.\n", 2, tongue, self->netNameID);
#else
 #ifdef MAN // for MAN

 #else
  #ifdef TIW // for TIW

  #else
		artifact_special_print(T_KEY_PICKUP, other, "%s %s.\n", 2, tongue, self->netNameID);
		//gstate->centerprint( other, 2.0, "%s %s\n", self->netname, tongue[T_KEY_PICKUP] );
  #endif // TIW
 #endif // MAN
#endif // JPN
	}	

	// never respawns
	if (!(coop->value && (ihook->info->respawnFlags & ITEM_RESP_COOP_STAY)))
		self->remove(self);
}


//itemInfo_t *item_key_init( userEntity_t *self, int nNetName, CVector &vMins, CVector &vMaxs, touch_t touch )
itemInfo_t *item_init( userEntity_t *self, int nNetName, CVector &vMins, CVector &vMaxs, touch_t touch )
{
	itemInfo_t	*info;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	
 	info->netName = tongue[nNetName];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue,nNetName);
//	info->touchFunc = item_key_touch;
	info->touchFunc = touch;

	info->mins = vMins;
	info->maxs = vMaxs;

	info->respawnFlags = 0;
	info->flags |= FL_ITEM;
	info->respawnTime = -1.0;	//	never respawns

	info->soundPickup = NULL;
	info->soundRespawn = NULL;
	info->soundAmbient = NULL;

	return info;
}

///////////////////////////////////////////////////////////////////////////////////
//	item_crypt_key
//
///////////////////////////////////////////////////////////////////////////////////

void	item_crypt_key (userEntity_t *self)
{
	CVector	vMins, vMaxs;
	vMins.Set( -16, -16, 0 );
	vMaxs.Set( 16, 16, 40 );

	itemInfo_t *info = item_init( self, T_KEY_CRYPT, vMins, vMaxs, item_key_touch );

	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;
	info->soundPickup  = "e3/crykeypickup.wav";//"global/a_specialitem.wav"; //NSS[11/30/99]:

	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e3/a3_crkey.dkm" );
}


///////////////////////////////////////////////////////////////////////////////////
//	item_wyndrax_key
//
///////////////////////////////////////////////////////////////////////////////////

void	item_wyndrax_key (userEntity_t *self)
{
	CVector	vMins, vMaxs;
	vMins.Set( -16, -16, 0 );
	vMaxs.Set( 16, 16, 40 );

	itemInfo_t *info = item_init( self, T_KEY_WYNDRAX, vMins, vMaxs, item_key_touch );

	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;
	info->soundPickup  = "e3/wynkeypickup.wav";//"global/a_specialitem.wav"; //NSS[11/30/99]:

	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e3/a3_ltkey.dkm" );
}

///////////////////////////////////////////////////////////////////////////////////
//	item_spellbook
//
///////////////////////////////////////////////////////////////////////////////////

void item_spellbook( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -16, -16, 0 );
	vMaxs.Set( 16, 16, 40 );

	itemInfo_t *info = item_init( self, T_BOOK_WYNDRAX, vMins, vMaxs, item_key_touch );

	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;
	info->soundPickup  = "e3/wynspellpickup.wav";//"global/a_specialitem.wav"; //NSS[11/30/99]:

	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e3/a3_bookw.dkm" );
}

/*
// SCG[11/17/99]: Removed per John's request
///////////////////////////////////////////////////////////////////////////////////
//	item_evilbook
//
///////////////////////////////////////////////////////////////////////////////////

void item_evilbook( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -16, -16, 0 );
	vMaxs.Set( 16, 16, 16 );

	itemInfo_t *info = item_init( self, T_BOOK_EVIL, vMins, vMaxs, item_key_touch );

	info->soundSpecial = "e3/we_chant1.wav";

	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e3/a3_booke.dkm" );
}
*/

///////////////////////////////////////////////////////////////////////////////////
//	item_control_card_red
///////////////////////////////////////////////////////////////////////////////////
void item_control_card_red( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_KEY_RED_CARD, vMins, vMaxs, item_key_touch );

	self->s.color.Set( 1.0, 0.0, 0.0 );
	info->soundPickup  = "global/a_specialitem.wav"; //NSS[11/30/99]:

	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e4/a4_clcbl.dkm" );
}

///////////////////////////////////////////////////////////////////////////////////
//	item_control_card_blue
///////////////////////////////////////////////////////////////////////////////////
void item_control_card_blue( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_KEY_BLUE_CARD, vMins, vMaxs, item_key_touch );

	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;
	self->s.color.Set( 0.0, 0.0, 1.0 );
	info->soundPickup  = "e4/bkeypickup.wav";//"global/a_specialitem.wav"; //NSS[11/30/99]:

	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e4/a4_clcbl.dkm" );
}

///////////////////////////////////////////////////////////////////////////////////
//	item_control_card_yellow
///////////////////////////////////////////////////////////////////////////////////
void item_control_card_yellow( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_KEY_YELLOW_CARD, vMins, vMaxs, item_key_touch );

	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;
	self->s.color.Set( 1.0, 1.0, 0.0 );
	info->soundPickup  = "e4/ykeypickup.wav";//"global/a_specialitem.wav"; //NSS[11/30/99]:

	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e4/a4_clcyl.dkm" );
}

///////////////////////////////////////////////////////////////////////////////////
//	item_control_card_green
///////////////////////////////////////////////////////////////////////////////////
void item_control_card_green( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_KEY_GREEN_CARD, vMins, vMaxs, item_key_touch );

	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;
	self->s.color.Set( 0.0, 1.0, 0.0 );
	info->soundPickup  = "e4/gkeypickup.wav";//"global/a_specialitem.wav"; //NSS[11/30/99]:

	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e4/a4_clcgr.dkm" );
}

///////////////////////////////////////////////////////////////////////////////////
//	item_keycard_cell
///////////////////////////////////////////////////////////////////////////////////
void item_keycard_cell (userEntity_t *self)
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_KEY_PRISONCELL, vMins, vMaxs, item_key_touch );
	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;
	info->soundPickup  = "e1/ccardpickup.wav";//"global/a_specialitem.wav"; //NSS[11/30/99]:
	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e1/a1_clcrd.dkm" );
}

///////////////////////////////////////////////////////////////////////////////////
//	item_hex_keystone
///////////////////////////////////////////////////////////////////////////////////
void item_hex_keystone( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_KEY_HEX_KEYSTONE, vMins, vMaxs, item_key_touch );

	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;
	info->soundPickup  = "e3/hexkeypickup.wav";//"global/a_specialitem.wav";//NSS[11/30/99]:

	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e3/a_hex.dkm" );
}

///////////////////////////////////////////////////////////////////////////////////
//	item_quad_keystone
///////////////////////////////////////////////////////////////////////////////////
void item_quad_keystone( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_KEY_QUAD_KEYSTONE, vMins, vMaxs, item_key_touch );
	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;
	info->soundPickup  = "e3/qkeypickup.wav";//"global/a_specialitem.wav";//NSS[11/30/99]:
	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e3/a_quad.dkm" );
}

///////////////////////////////////////////////////////////////////////////////////
//	item_trigon_keystone
///////////////////////////////////////////////////////////////////////////////////
void item_trigon_keystone( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_KEY_TRIGON_KEYSTONE, vMins, vMaxs, item_key_touch );
	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;
	info->soundPickup  = "e3/trikeypickup.wav";//"global/a_specialitem.wav";//NSS[11/30/99]:
	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e3/a_tri.dkm" );
}

///////////////////////////////////////////////////////////////////////////////
// item_megashield
///////////////////////////////////////////////////////////////////////////////

void item_megashield( userEntity_t *self )
{
	if (IS_INSTAGIB || (deathmatch->value && !dm_allow_health->value))
		return;

	itemInfo_t  *info;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	info->netName = tongue[T_MEGASHIELD];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue,T_MEGASHIELD);
	info->touchFunc = item_boost_touch;

	info->mins.Set(-10, -10, 0);
	info->maxs.Set(10, 10, 24);

	info->health = 100;
	info->respawnTime = 300.0;

	info->soundAmbient = "artifacts/megaamba.wav";
	info->soundAmbientVolume = 0.35;
	info->soundPickup  = "artifacts/megauser.wav";
	info->soundRespawn = "global/a_itspwn.wav";

	info->flags |= IT_MEGASHIELD;

	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/global/a_mshield.dkm" );
}

///////////////////////////////////////////////////////////////////////////////
// item_invincibility
///////////////////////////////////////////////////////////////////////////////

void item_invincibility( userEntity_t *self )
{
	if (deathmatch->value && !dm_allow_powerups->value)
		return;

	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_SKULL_OF_INVINCIBILITY, vMins, vMaxs, item_boost_touch );

	info->health = 100;
	info->respawnTime = 300.0;

	info->soundAmbient = "artifacts/manaskullamba.wav";
	info->soundAmbientVolume = 1.0;
	info->soundPickup  = "artifacts/manaskullpickup.wav";
	info->soundRespawn = "global/a_itspwn.wav";

	info->flags |= IT_MANASKULL;

	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/global/a_invincibility.dkm" );
}

void item_antidote( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_POISON_ANTIDOTE, vMins, vMaxs, item_add_to_inventory );
	info->respawnTime = 30.0;
	info->soundPickup  = "global/a_specialitem.wav"; //cek[1-3-00]

	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/global/a_antidote.dkm" );
}

void item_drachma( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_DRACHMA, vMins, vMaxs, item_add_to_inventory );
	info->soundPickup  = "e2/drachmapickup1.wav";//"global/a_specialitem.wav"; //NSS[11/30/99]:
	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL|ITEM_RESP_USE_ONCE;
	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e2/a2_drachma.dkm" );
}

void item_horn( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_HORN, vMins, vMaxs, item_add_to_inventory );
	
	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL|ITEM_RESP_USE_ONCE;
	info->soundPickup  = "e2/greekhorn.wav";//"global/a_specialitem.wav"; //NSS[11/30/99]:
	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e2/a2_horn.dkm" );
}

void item_rune_a( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_KEY_AEGIS_A, vMins, vMaxs, item_add_to_inventory );
	info->soundPickup  = "e2/runepickup.wav";//"global/a_specialitem.wav"; //NSS[11/30/99]:
	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;

	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e2/c_runea.dkm" );
}

void item_rune_e( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_KEY_AEGIS_E, vMins, vMaxs, item_add_to_inventory );
	info->soundPickup  = "e2/runepickup.wav";//"global/a_specialitem.wav"; //NSS[11/30/99]:
	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;

	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e2/c_runee.dkm" );
}

void item_rune_g( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_KEY_AEGIS_G, vMins, vMaxs, item_add_to_inventory );
	info->soundPickup  = "e2/runepickup.wav";//"global/a_specialitem.wav"; //NSS[11/30/99]:
	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;

	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e2/c_runeg.dkm" );
}

void item_rune_i( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_KEY_AEGIS_I, vMins, vMaxs, item_add_to_inventory );
	info->soundPickup  = "e2/runepickup.wav";//"global/a_specialitem.wav"; //NSS[11/30/99]:
	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;

	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e2/c_runei.dkm" );
}

void item_rune_s( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_KEY_AEGIS_S, vMins, vMaxs, item_add_to_inventory );
	info->soundPickup  = "e2/runepickup.wav";//"global/a_specialitem.wav"; //NSS[11/30/99]:
	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;

	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e2/c_runes.dkm" );
}

void item_purifier_shard1( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_PURIFIER_SHARD, vMins, vMaxs, item_add_to_inventory );
	info->soundPickup  = "e3/shpuripickup.wav";//"global/a_specialitem.wav"; //NSS[11/30/99]:
	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;
	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e3/Purifier_1.dkm" );
}

void item_purifier_shard2( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_PURIFIER_SHARD, vMins, vMaxs, item_add_to_inventory );
	info->soundPickup  = "e3/shpuripickup.wav";//""global/a_specialitem.wav"; //NSS[11/30/99]:
	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;
	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e3/Purifier_2.dkm" );
}

void item_purifier_shard2_1( userEntity_t *self )
{
	item_purifier_shard2(self);
}

void item_purifier_shard2_2( userEntity_t *self )
{
	item_purifier_shard2(self);
}

void item_purifier_shard2_3( userEntity_t *self )
{
	item_purifier_shard2(self);
}

void item_purifier_shard2_4( userEntity_t *self )
{
	item_purifier_shard2(self);
}

void item_purifier_shard2_5( userEntity_t *self )
{
	item_purifier_shard2(self);
}

void item_purifier_shard3( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_PURIFIER_SHARD, vMins, vMaxs, item_add_to_inventory );
	info->soundPickup  = "e3/shpuripickup.wav";//""global/a_specialitem.wav"; //NSS[11/30/99]:
	info->respawnFlags |= ITEM_RESP_COOP_STAY|ITEM_RESP_COOP_REMOVEALL;
	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e3/Purifier_3.dkm" );
}

void item_black_chest( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 32 );

	itemInfo_t *info = item_init( self, T_BLACK_CHEST, vMins, vMaxs, NULL );

	info->flags |= IT_BLACK_CHEST;

	item_Spawn( self, info, MOVETYPE_TOSS, FALSE, "models/e3/a_blackchest.dkm" );

	self->use = item_black_chest_use;
	self->solid = SOLID_BBOX;

	gstate->LinkEntity (self);
}

void item_wood_chest( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 32 );

	itemInfo_t *info = item_init( self, T_BLACK_CHEST, vMins, vMaxs, NULL );

	info->flags |= IT_WOOD_CHEST;

	item_Spawn( self, info, MOVETYPE_TOSS, FALSE, "models/e3/a_chest.dkm" );

	self->use = item_wood_chest_use;
	self->solid = SOLID_BBOX;

	gstate->LinkEntity (self);
}

/*
// SCG[11/17/99]: Removed per John's request
void item_ice_boots( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_ICE_BOOTS, vMins, vMaxs, item_add_to_inventory );

	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e3/a_iceboot.dkm" );
}
*/
//NSS[11/28/99]:Added specificly for this scenario.
int FindClientInPVS(userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////////
//	ring_of_fire_think
//NSS[11/17/99]: Think function occurs every 1 second and checks to see if a client
//is within 'x' distance.
///////////////////////////////////////////////////////////////////////////////////
void ring_of_fire_think(userEntity_t * self)
{
	int Enemy = FindClientInPVS(self);
	
	if(Enemy && !self->deadflag)
	{
		trackInfo_t tinfo;
		// clear this variable
		memset(&tinfo, 0, sizeof(tinfo));

		tinfo.ent			=self;
		tinfo.srcent		=self;
		tinfo.fru.Zero();
		tinfo.length		= 0;	
		tinfo.Long1			= ART_FIRE;
		tinfo.flags			= TEF_FXFLAGS|TEF_SRCINDEX|TEF_LENGTH|TEF_LONG1;
		tinfo.fxflags		= TEF_ARTIFACT_FX|TEF_FX_ONLY;
		
		com->trackEntity(&tinfo,MULTICAST_ALL);
		self->deadflag	= 1;
	}
	else if (self->deadflag && !Enemy)
	{
		RELIABLE_UNTRACK(self);
		self->deadflag = 0;
	}
	self->nextthink = gstate->time + 0.2f;
}
///////////////////////////////////////////////////////////////////////////////////
//	item_ring_of_fire
//NSS[11/17/99]:
///////////////////////////////////////////////////////////////////////////////////
void item_ring_of_fire( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_RING_OF_FIRE, vMins, vMaxs, item_add_to_inventory );

	
	info->soundAmbient = "global/e_torcha.wav";
	info->soundAmbientVolume = 0.15;
	info->soundPickup  = "global/a_specialitem.wav"; //NSS[11/30/99]:
	info->flags	|= IT_RINGOFIRE;
	//char	*soundRespawn;
	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e3/a_ring_fire.dkm" );

	self->think		= ring_of_fire_think;
	self->nextthink = gstate->time + 4.0f;
}


///////////////////////////////////////////////////////////////////////////////////
//	ring_of_undead_think
//NSS[11/17/99]: Think function occurs every 1 second and checks to see if a client
//is within 'x' distance.
///////////////////////////////////////////////////////////////////////////////////
void ring_of_undead_think(userEntity_t * self)
{
	int Enemy = FindClientInPVS(self);
	
	if(Enemy && !self->deadflag)
	{
		trackInfo_t tinfo;
		// clear this variable
		memset(&tinfo, 0, sizeof(tinfo));

		tinfo.ent			=self;
		tinfo.srcent		=self;
		tinfo.fru.Zero();
		tinfo.Long1			= ART_UNDEAD;
		tinfo.flags			= TEF_FXFLAGS|TEF_SRCINDEX|TEF_LONG1;
		tinfo.fxflags		= TEF_ARTIFACT_FX|TEF_FX_ONLY;
		
		com->trackEntity(&tinfo,MULTICAST_ALL);
		self->deadflag	= 1;
	}
	else if (self->deadflag && !Enemy)
	{
		RELIABLE_UNTRACK(self);
		self->deadflag = 0;
	}
	self->nextthink = gstate->time + 0.2f;
}

///////////////////////////////////////////////////////////////////////////////////
//	item_ring_of_undead 
//NSS[11/17/99]:
///////////////////////////////////////////////////////////////////////////////////
void item_ring_of_undead( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_RING_OF_UNDEAD, vMins, vMaxs, item_add_to_inventory );

	info->soundAmbient	= "global/e_warploopa.wav";
	info->soundAmbientVolume = 0.15;
	info->soundPickup  = "global/a_specialitem.wav"; //NSS[11/30/99]:
	info->soundRespawn	= "global/e_warpenda.wav";

	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e3/a_ringund.dkm" );
	self->think		= ring_of_undead_think;
	self->nextthink = gstate->time + 3.75f;
}

///////////////////////////////////////////////////////////////////////////////////
//	item_ring_of_lightning
//NSS[11/17/99]:
///////////////////////////////////////////////////////////////////////////////////
void item_ring_of_lightning( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_RING_OF_LIGHTNING, vMins, vMaxs, item_add_to_inventory );
	
	info->soundAmbient = "global/e_electronclaw.wav";
	info->soundAmbientVolume = 0.15;
	info->soundPickup  = "global/a_specialitem.wav"; //NSS[11/30/99]:
	
	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e3/a_ringlig.dkm" );
}

void item_envirosuit( userEntity_t *self )
{
	CVector	vMins, vMaxs;
	vMins.Set( -10, -10, 0 );
	vMaxs.Set( 10, 10, 16 );

	itemInfo_t *info = item_init( self, T_ENVIROSUIT, vMins, vMaxs, item_add_to_inventory );
	info->soundPickup = "artifacts/envirosuitpickup.wav";
	info->respawnFlags |= ITEM_RESP_COOP_STAY;
	
	// NSS[3/2/00]:Cache this sound
	gstate->SoundIndex("artifacts/envirosuitfade.wav");

	item_Spawn( self, info, MOVETYPE_TOSS, TRUE, "models/e4/a_envsuit.dkm" );
}


void view_rotate(userEntity_t *self)
// view rotating item -- DEBUGGING ONLY
{
	itemInfo_t	*info;

	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);
	info->className = self->className;
	info->netName = "view_rotate";
	info->touchFunc = NULL;

	//info->mins.Set(-8, -8, -8);
	//info->maxs.Set( 8,  8,  8);
	info->mins.Set(-8, -8, -24);
	info->maxs.Set(8, 8, 0);

	info->mins.Set(0, 0, 0);
	info->maxs.Set(0, 0, 0);

	info->health = 100;

	item_Spawn (self, info, MOVETYPE_TOSS, TRUE, "models/global/e_gibhead.dkm");
    //item_Spawn (self, info, MOVETYPE_TOSS, TRUE, "models/global/a_gsoul.dkm");

    self->s.renderfx = RF_FULLBRIGHT;
}

int GetArmorPercentage(int armor_val, int item_flags)
// return percentage of armor based on armor type and current value
{
	int max_value = 1;

	if (!item_flags) // no items?
		return (0);

	if (item_flags & IT_PLASTEEL)      
		max_value = ARMOR_PLASTEEL_MAXHEALTH;
	else
	if (item_flags & IT_CHROMATIC)     
		max_value = ARMOR_CHROMATIC_MAXHEALTH;
	else
	if (item_flags & IT_SILVER)        
		max_value = ARMOR_SILVER_MAXHEALTH;
	else
	if (item_flags & IT_GOLD)          
		max_value = ARMOR_GOLD_MAXHEALTH;
	else
	if (item_flags & IT_CHAINMAIL)     
		max_value = ARMOR_CHAINMAIL_MAXHEALTH;
	else
	if (item_flags & IT_BLACK_ADAMANT) 
		max_value = ARMOR_BLACK_ADAMANT_MAXHEALTH;
	else
	if (item_flags & IT_KEVLAR)        
		max_value = ARMOR_KEVLAR_MAXHEALTH;
	else
	if (item_flags & IT_EBONITE)       
		max_value = ARMOR_EBONITE_MAXHEALTH;
	else
	if (item_flags & IT_MEGASHIELD)
		max_value = ARMOR_MEGASHIELD_MAXHEALTH; //NSS[11/27/99]:Added for megashield

	return((100 * armor_val) / max_value); // return percentage cek[1-30-00]: this was getting rounded down to 0
}



// * SAVE GEMS *********************************************************************


void item_savegem_use(userEntity_t *self)
// command to decrement save gem from player's inventory -- used as an acknowledgement
// from the client that a save-game was successfully performed, and to decrement the
// savegem from user's inventory
{
	userInventory_t	*item_savegem;

//	gstate->Con_Printf("item_savegem_use()\n");

	// find a savegem to remove from inventory
	item_savegem = gstate->InventoryFindItem (self->inventory, "item_savegem");

	if (item_savegem)  // found one?
	{
		// play a cool sound!
		int episode = gstate->episode;
		if ((episode > 4) || (episode < 1))
			episode = 1;

		char buffer[30];
//		sprintf(buffer,"e%d/e%d_secret.wav",episode,episode);
		Com_sprintf(buffer,sizeof(buffer),"e%d/e%d_secret.wav",episode,episode);
		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex(buffer),1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );

		// remove the save gem
		gstate->InventoryDeleteItem (self, self->inventory, item_savegem); // nuke it
		// inform client of the new savegem count

		gstate->WriteByte(SVC_SAVEGEM);  // server -> client
		gstate->WriteByte(false);        // using savegem
		gstate->WriteByte(gstate->InventoryItemCount(self->inventory,"item_savegem")); // # of savegems
		gstate->UniCast(self,true);      // broadcast
	}
	else
	{
		gstate->Con_Printf("No savegems found.\n");
	}
	gstate->InventoryUpdateClient(self,false);
}

void item_savegem_invuse( struct userInventory_s *ptr, struct edict_s *user )
// using the savegem from player inventory
{
//	gstate->Con_Printf("item_savegem_invuse()\n");

	gstate->WriteByte(SVC_SAVEGEM);  // server -> client
	gstate->WriteByte(true);         // using savegem
	gstate->WriteByte(gstate->InventoryItemCount(user->inventory,"item_savegem")); // # of savegems
	gstate->UniCast(user,true);      // broadcast
}


void item_savegem_touch(userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
// when touching a save gem entity, add to users inventory
{
	if (!com->ValidTouch (self, other) || !self->className)
	return;

	// NSS[2/8/00]:Sidekicks cannot touch these things
	if(other->flags & FL_BOT)
		return;
	
	if (coop->value)
	{
		CoopFindPlayers(&other, NULL, NULL, false);
		if (!other)
			return;
	}
	// can we carry another?
	if (gstate->InventoryItemCount(other->inventory,"item_savegem") >= 3)
	{
		return;
	}

	playerHook_t	*ohook = AI_GetPlayerHook( other );
	itemHook_t		*ihook = (itemHook_t *) self->userHook;

	if (other->flags & FL_CLIENT) // must be a player who touches the gem
	{
		userInventory_t	*invItem = NULL;

		// initialize new inventory item
		invItem = gstate->InventoryCreateItem(
											other->inventory, 
											self->className, 
												  item_savegem_invuse, 
											NULL, 
											self->s.modelindex, 
											ITF_INVSACK, 
											sizeof (userInventory_t));

		_ASSERTE(invItem);   // guarantee item created


		// officially add it to player's inventory
		gstate->InventoryAddItem (other, other->inventory, invItem); 

		//	play pickup sound notification
		gstate->StartEntitySound(other, CHAN_VOICE, ihook->soundPickupIndex, 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierID(RESOURCE_ID_FOR_STRING(tongue,T_PICKUP_SAVEGEM),0);
		gstate->cs.Unicast(other,CS_print_center,2.0);
		gstate->cs.EndSendString();
		//gstate->centerprint (other, 2.0, "%s!\n", tongue[T_PICKUP_SAVEGEM]);

		other->client->bonus_alpha = 0.25;  // flash alpha screen

		// inform client that user has picked up a savegem (and the client will store a local variable, noting so)
		gstate->WriteByte(SVC_SAVEGEM);  // server -> client
		gstate->WriteByte(false);        // using savegem
		gstate->WriteByte(gstate->InventoryItemCount(other->inventory,"item_savegem")); // # of savegems
		gstate->UniCast(other,true);      // broadcast

		//NSS[11/20/99]:Umm... fuck.. jesus.. help.. remove entities once you are DONE with them.
		self->remove(self);  // savegems never respawn -- done with 'self', so nuke 'im

	}

}


///////////////////////////////////////////////////////////////////////////////
// savegems
//   -- these little fellas can be collected in your inventory, and then used
//      to save a game.
///////////////////////////////////////////////////////////////////////////////
void item_savegem(userEntity_t *self)
{
	if (unlimited_saves->value || deathmatch->value || (gstate->GetCvar("dedicated") > 0)) // ttd: allow for save gems in co-op mode
	{
		gstate->RemoveEntity(self);
		return;
	}

	itemInfo_t *info;
	info = (itemInfo_t*)gstate->X_Malloc(sizeof(itemInfo_t), MEM_TAG_HOOK);

	info->className = self->className;
	info->netName = tongue[T_SAVEGEM];
	info->netNameID = RESOURCE_ID_FOR_STRING(tongue,T_SAVEGEM);
	info->touchFunc = item_savegem_touch;

	info->mins.Set(-8, -8, -24);  // ttd: adjust box
	info->maxs.Set(8, 8, 0);

	info->health      = 100;
	info->respawnTime = 0.0; // one-time, no respawn

	info->soundPickup  = "artifacts/savegem_pickup.wav";
	info->soundAmbient = "artifacts/savegem_wait.wav";

	info->render_scale = 1.0; // standard size

	item_Spawn (self, info, MOVETYPE_TOSS, TRUE, "models/global/a_savegem.dkm");
}

// *********************************************************************************

invenUse_t artifact_inv_use(char *name)
{
	if (!stricmp(name,"item_savegem"))
		return item_savegem_invuse;
	else if (strstr(name, "book"))
		return item_book_use;
	else if (!stricmp(name,"item_sulphur") || !stricmp(name,"item_bottle") || !stricmp(name,"item_charcoal") || !stricmp(name,"item_saltpeter"))
		return item_bombpart_use;
	else
		return NULL;
}

invenCommand_t artifact_inv_command(char *name)
{
	return NULL;
}

void artifact_register_func(void)
{
	gstate->RegisterFunc("item_bombpart_touch",item_bombpart_touch);
	gstate->RegisterFunc("item_health_touch",item_health_touch);
	gstate->RegisterFunc("item_armor_touch",item_armor_touch);
	gstate->RegisterFunc("item_boost_touch",item_boost_touch);
	gstate->RegisterFunc("item_key_touch",item_key_touch);
	gstate->RegisterFunc("item_wraithorb_touch",item_wraithorb_touch);
	gstate->RegisterFunc("boost_respawn",boost_respawn);
	gstate->RegisterFunc("item_bomb_kapow",item_bomb_kapow);
	gstate->RegisterFunc("item_bomb_countdown",item_bomb_countdown);
	gstate->RegisterFunc("item_savegem_touch", item_savegem_touch);
	gstate->RegisterFunc("item_add_to_inventory", item_add_to_inventory);

	gstate->RegisterFunc("item_hook_save", item_hook_save);
	gstate->RegisterFunc("item_hook_load", item_hook_load);

	gstate->RegisterFunc( "item_black_chest_use", item_black_chest_use );
	gstate->RegisterFunc( "item_wood_chest_use", item_wood_chest_use );

	gstate->RegisterFunc( "item_savegem_invuse", item_savegem_invuse);
	gstate->RegisterFunc( "item_book_use", item_book_use);
	gstate->RegisterFunc( "item_bombpart_use", item_bombpart_use);
	gstate->RegisterFunc( "Item_Respawn", Item_Respawn);
	gstate->RegisterFunc( "boost_nucleus_think", boost_nucleus_think);
}

void artifact_precache_sounds(void)
{
	switch(gstate->episode)
	{
	case 1:
		gstate->SoundIndex("e1/ccardpickup.wav");
		gstate->SoundIndex("e1/e1_secret.wav");
		break;
	case 2:
		gstate->SoundIndex("e2/runepickup.wav");
		gstate->SoundIndex("e2/greekhorn.wav");
		gstate->SoundIndex("e2/drachmapickup1.wav");
		gstate->SoundIndex("e2/e2_secret.wav");
		break;
	case 3:
		gstate->SoundIndex("doors/e3/woodendoor4open.wav");
		gstate->SoundIndex("e3/wynspellpickup.wav");
		gstate->SoundIndex("e3/wynkeypickup.wav");
		gstate->SoundIndex("e3/crykeypickup.wav");
		gstate->SoundIndex("e3/trikeypickup.wav");
		gstate->SoundIndex("e3/qkeypickup.wav");
		gstate->SoundIndex("e3/hexkeypickup.wav");
		gstate->SoundIndex("e3/e3_secret.wav");
		gstate->SoundIndex("e3/shpuripickup.wav");
		break;
	case 4:
		gstate->SoundIndex("e4/empbotpickup.wav");
		gstate->SoundIndex("e4/saltpickup.wav");
		gstate->SoundIndex("e4/sulpickup.wav");
		gstate->SoundIndex("e4/gkeypickup.wav");
		gstate->SoundIndex("e4/ykeypickup.wav");
		gstate->SoundIndex("e4/bkeypickup.wav");
		gstate->SoundIndex("e4/e4_secret.wav");
		gstate->SoundIndex("artifacts/envirosuitfade.wav");
		gstate->SoundIndex("artifacts/envirosuituse.wav");
		break;
	default:
		break;
	};
	// NSS[3/2/00]:
	gstate->SoundIndex("artifacts/manaskullfade.wav");
	gstate->SoundIndex("artifacts/wraithorbburnout.wav");
	gstate->SoundIndex("artifacts/savegem_wait.wav");
	gstate->SoundIndex("artifacts/savegem_pickup.wav");
	gstate->SoundIndex("artifacts/envirosuitpickup.wav");
	gstate->SoundIndex("global/a_specialitem.wav");
	gstate->SoundIndex("global/e_electronclaw.wav");
	gstate->SoundIndex("global/e_warploopa.wav");
	gstate->SoundIndex("global/e_warpenda.wav");
	gstate->SoundIndex("global/e_torcha.wav");
	gstate->SoundIndex("artifacts/manaskullamba.wav");
	gstate->SoundIndex("artifacts/megauser.wav");
	gstate->SoundIndex("global/a_itspwn.wav");
	gstate->SoundIndex("global/a_vboost.wav");
	gstate->SoundIndex("global/a_aboost.wav");
	gstate->SoundIndex("global/a_sboost.wav");
	gstate->SoundIndex("global/a_atkboost.wav");
	gstate->SoundIndex("global/a_pboost.wav");
	gstate->SoundIndex("global/armorpickup1.wav");
	gstate->SoundIndex("global/armorpickup2.wav");
	gstate->SoundIndex("artifacts/wraithorbwait.wav");
	gstate->SoundIndex("artifacts/wraithorbpickup.wav");
	gstate->SoundIndex("artifacts/goldensoulwait.wav");
	gstate->SoundIndex("artifacts/goldensoulpickup.wav");
	gstate->SoundIndex("global/a_hpick.wav");
	gstate->SoundIndex("global/a_h50pick.wav");
}