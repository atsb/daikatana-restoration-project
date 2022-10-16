// user interface file between katana and dlls
#include	"p_user.h"
#include	"hooks.h"
#include	"common.h"

///////////////////////////////////////////////////////////////////////////////
//	exports
///////////////////////////////////////////////////////////////////////////////

DllExport	void	item_health_25 (userEntity_t *self);
DllExport	void	item_goldensoul (userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define	ITF_BOOSTBASEHEALTH	0x00000001

///////////////////////////////////////////////////////////////////////////////
//	typedef
///////////////////////////////////////////////////////////////////////////////

typedef	struct	itemInfo_s
{
	char	*className;
	char	*netName;
	char	*episodeIndependentModelName;

	char	*soundPickup;
	char	*soundRespawn;
	char	*soundAmbient;

	vec3_t	mins;
	vec3_t	maxs;

	touch_t	touchFunc;

	int		health;
	int		respawnTime;

	unsigned	long	flags;
} itemInfo_t;

typedef	struct	itemHook_s
{
	float		respawnTime;
	float		health;

	itemInfo_t	*info;	

	int			soundPickupIndex;
	int			soundRespawnIndex;

	//	store some flags here that define who can pick this item up???
	unsigned long	flags;
} itemHook_t;

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

extern	serverState_t	*gstate;
extern	common_export_t	*com;

extern	cvar_t	*maxclients;
extern	cvar_t	*deathmatch;
extern	cvar_t	*coop;
extern	cvar_t	*skill;
extern	cvar_t	*p_gravity;
extern	cvar_t	*sv_episode;

///////////////////////////////////////////////////////////////////////////////
//	item_NameForEpisode
//
//	returns the model name filled in with episode numbers
///////////////////////////////////////////////////////////////////////////////

static	char	itemName [MAX_QPATH];

char	*item_NameForEpisode (char *name)
{
	int	episode = (int) sv_episode->value;
	sprintf (itemName, "models\\e%i\\a%i_%s", episode, episode, name);
	return	itemName;
}

///////////////////////////////////////////////////////////////////////////////
//	item_Spawn
//
//	spawns an artifact using the passed model name
///////////////////////////////////////////////////////////////////////////////

void	item_Spawn (userEntity_t *self, itemInfo_t *info)
{
	itemHook_t	*ihook;

	self->netname = info->netName;
	self->userHook = new (HOOK_MALLOC) itemHook_t;
	ihook = (itemHook_t *) self->userHook;
	
	self->solid = SOLID_TRIGGER;
	self->movetype = MOVETYPE_TOSS;
	self->s.effects = 0;
	self->s.renderfx = RF_GLOW;

	self->touch = info->touchFunc;
	
	self->modelName = item_NameForEpisode (info->episodeIndependentModelName);
	self->s.modelindex = gstate->ModelIndex (self->modelName);

	VectorCopy (info->mins, self->s.mins);
	VectorCopy (info->maxs, self->s.maxs);

	//	origin is already set in physics when spawning this entity
	gstate->LinkEntity (self);

	ihook->health = info->health;
	ihook->respawnTime = info->respawnTime;
	ihook->info = info;
	ihook->flags = info->flags;
	
	ihook->soundPickupIndex = gstate->SoundIndex (info->soundPickup);
	ihook->soundRespawnIndex = gstate->SoundIndex (info->soundRespawn);
	if (info->soundAmbient != NULL)
	{
		self->s.sound = self->soundAmbientIndex = gstate->SoundIndex (info->soundAmbient);
	}
	else
		self->soundAmbientIndex = 0;
}

///////////////////////////////////////////////////////////////////////////////
// item_health_touch
///////////////////////////////////////////////////////////////////////////////

void	item_health_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	playerHook_t	*ohook = (playerHook_t *) other->userHook;
	itemHook_t		*ihook = (itemHook_t *) self->userHook;

	if (!com->ValidTouch (self, other))
		return;

	if (ihook->flags & ITF_BOOSTBASEHEALTH)
	{
		//	boost health up to twice base health
		if (other->health >= ohook->base_health * 2.0)
			return;

		com->Health (other, ihook->health, ohook->base_health * 2.0);
	}
	else 
	{
		//	only boost health up to base health
		if (other->health >= ohook->base_health)
			return;

		com->Health (other, ihook->health, ohook->base_health);
	}

	//	play pickup sound
	gstate->Sound (other, CHAN_VOICE, ihook->soundPickupIndex, 1.0, ATTN_NORM, 0.0);

	if (other->flags & FL_CLIENT)
	{
		other->client->bonus_alpha = 0.25;
		gstate->sprint (other, "You picked up the %s.\n", self->netname);
	}

	com->SetRespawn (self, ihook->respawnTime);
}

///////////////////////////////////////////////////////////////////////////////
// item_health_25
///////////////////////////////////////////////////////////////////////////////

void	item_health_25 (userEntity_t *self)
{
	itemInfo_t	*info;

	info = (itemInfo_t *) new (HOOK_MALLOC) itemInfo_t;
	info->className = self->className;
	info->netName = "health kit";
	info->episodeIndependentModelName = "hlth.dkm";
	info->touchFunc = item_health_touch;

	VectorSet (info->mins, -16, -16, -24);
	VectorSet (info->maxs, 16, 16, 0);

	info->health = 25;
	info->respawnTime = 30.0;

	info->soundPickup = "global/a_hpick.wav";
	info->soundRespawn = "global/a_ispawn.wav";
	info->soundAmbient = NULL;

	item_Spawn (self, info);
}

///////////////////////////////////////////////////////////////////////////////
// item_goldensoul
///////////////////////////////////////////////////////////////////////////////

void	item_goldensoul (userEntity_t *self)
{
	itemInfo_t	*info;

	info = (itemInfo_t *) new (HOOK_MALLOC) itemInfo_t;
	info->className = self->className;
	info->netName = "GoldenSoul";
	info->episodeIndependentModelName = "hlth.dkm";
	info->touchFunc = item_health_touch;

	VectorSet (info->mins, -16, -16, -24);
	VectorSet (info->maxs, 16, 16, 0);

	//	goldensoul will give 1000 health, but that will be capped at base_health * 2.0
	info->health = 1000;
	info->respawnTime = 60.0;

	info->soundPickup = "global/a_gspick.wav";
	info->soundRespawn = "global/a_ispawn.wav";
	info->soundAmbient = "global/a_gswait.wav";

	info->flags |= ITF_BOOSTBASEHEALTH;

	item_Spawn (self, info);
}