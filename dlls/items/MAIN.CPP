#include	<stdio.h>
#include	<windows.h>

// user interface file between katana and dlls
#include	"p_user.h"
#include	"hooks.h"
#include	"common.h"

#include	"dk_dll.h"

///////////////////////////////////////////////////////////////////////////////
//	exports
///////////////////////////////////////////////////////////////////////////////

DllExport	void	*dll_Query(void);
DllExport	int		dll_Entry (HINSTANCE hParent, DWORD dwReasonForCall, PVOID pvData);

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

serverState_t	*gstate;
common_export_t	*com;
common_export_t	*memory_com;

cvar_t	*maxclients;
cvar_t	*deathmatch;
cvar_t	*coop;
cvar_t	*skill;
cvar_t	*p_gravity;
cvar_t	*sv_episode;

char	*dll_Description = "ITEMS.DLL.  Items and powerups.\n";

///////////////////////////////////////////////////////////////////////////////
//	epair exports
///////////////////////////////////////////////////////////////////////////////

userEpair_t eclassStr[] =
{
	{"classname",    "item_health_25"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 16 16 0"},
	{"rem",          "Gives 25% health"},

	{"classname",    "item_goldensoul"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 16 16 0"},
	{"rem",          "Maxes health to twice vitality."},

/*
	{"classname",    "item_potion"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "Gives 25% health"},
	
	{"classname",    "item_wooden_chest"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "A random item appears in the chest, either"},
	{"rem",          "antidote, health potion, wisp ammo or nightmare ammo"},

	{"classname",    "item_black_chest"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},map
	{"rem",          "There is a 50% chance of explosion which takes 75% "},
	{"rem",			 "damage and -10 HP or 100 HP magical health."},

	{"classname",    "item_health_field"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "There are three health levels: 10%, 25%, 40%. The "},
	{"rem",			 "values are random, within a specified range."},

	{"classname",    "item_health_kit_10"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "gives 10-15% health."},

	{"classname",    "item_health_kit_20"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "gives 20-25% health."},

	{"classname",    "item_plasteel_armor"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "Value 100.  Absorbs 1/3 damage."},

	{"classname",    "item_chromatic_armor"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "VALUE: 200%"},
	{"rem",			 "EFFECT: Absorbs 1/2 damage"},
	{"rem",			 "SPECIAL: Reflects lasers, but you take 1/4 damage "},
	{"rem",			 "from laser."},
	
	{"classname",    "item_silver_armor"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "VALUE: 200%"},
	{"rem",			 "EFFECT: Absorbs 1/2 damage."},

	{"classname",    "item_gold_armor"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "VALUE: 200%"},
	{"rem",			 "EFFECT: Absorbs 3/4 damage."},
	{"rem",			 "SPECIAL: Reflects lasers, but you take 1/4 damage "},
	{"rem",			 "from laser."},

	{"classname",    "item_chainmail_armor"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "VALUE: 125%"},
	{"rem",			 "EFFECT: Absorbs 1/3 damage"},

	{"classname",    "item_black_adamant_armor"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "VALUE: 250%"},
	{"rem",			 "EFFECT: Absorbs 1/2 damage"},
	{"rem",			 "SPECIAL: Fire protection.  Absorbs 3/4 damage"},
	{"rem",			 "from fire."},

	{"classname",    "item_kevlar_armor"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "VALUE: 100%"},
	{"rem",			 "EFFECT: Absorbs 40% damage"},
	
	{"classname",    "item_ebonite_armor"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "VALUE: 200%"},
	{"rem",			 "EFFECT: Absorbs 60% damage"},
	{"rem",			 "SPECIAL: Tazerhook can't grab.  1/2 of rocket"},
	{"rem",			 "radius damage."},

	{"classname",    "item_wraithorb"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "20% translucency, unlessed stopped, in which case"},
	{"rem",          "the player is completely invisible.  Lasts 60 seconds."},

	{"classname",    "item_megashield"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "30 seconds of invulnerability."},

	{"classname",    "item_goldensoul"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "+100 health, max of twice vitality."},

	{"classname",    "item_antidote"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "Cures poisonous wounds."},

	{"classname",    "item_power_boost"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "60 second boost to attack power."},

	{"classname",    "item_attack_boost"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "60 second boost to attack speed."},

	{"classname",    "item_speed_boost"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "60 second boost to movement speed."},

	{"classname",    "item_acro_boost"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "60 second boost to acrobatic speed."},

	{"classname",    "item_vita_boost"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "60 second boost to vitality."},

	{"classname",    "item_jet_boots"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "60 seconds of flying power."},

	{"classname",    "item_oxylung"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "60 of air under water."},

	{"classname",    "item_ice_boots"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "Stops wearer from sliding on ice indefinitely."},

	{"classname",    "item_envirosuit"},
	{"color",        "0.0 0.5 0.8"},
	{"size",         "-16 -16 -24 0 0 0"},
	{"rem",          "60 seconds of air and protection from harmful liquids."},
*/
	{NULL,            NULL},
};

///////////////////////////////////////////////////////////////////////////////////
//
//  cheat for powering up attributes
//
///////////////////////////////////////////////////////////////////////////////////

void	af_powerup (userEntity_t *self)
{
	int				count;
	char			*arg;
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	
	if (deathmatch->value)
		return;
		
	count = gstate->GetArgc ();

	if (count < 2 || count > 2)
	{
		gstate->bprint ("\ngreyskull [powerup]\n\ngives one level of boost to an attribute.");
		gstate->bprint ("[powerup] can be:\n  power\n  speed\n  attack\n  acro\n  vita\n  all\n");
	}

	arg = gstate->GetArgv (1);

	if (!stricmp (arg, "power"))
		hook->power_boost++;
	else if (!stricmp (arg, "speed"))
		hook->speed_boost++;
	else if (!stricmp (arg, "attack"))
		hook->attack_boost++;
	else if (!stricmp (arg, "acro"))
		hook->acro_boost++;
	else if (!stricmp (arg, "vita"))
		hook->vita_boost++;
	else if (!stricmp (arg, "all"))
	{
		hook->power_boost++;
		hook->speed_boost++;
		hook->attack_boost++;
		hook->acro_boost++;
		hook->vita_boost++;		
	}

	com->CalcBoosts (self);
}

///////////////////////////////////////////////////////////////////////////////////
//
//  cheat for powering down attributes
//
///////////////////////////////////////////////////////////////////////////////////

void	af_powerdown (userEntity_t *self)
{
	int				count;
	char			*arg;
	playerHook_t	*hook = (playerHook_t *) self->userHook;
	
	if (deathmatch->value)
		return;
		
	count = gstate->GetArgc ();

	if (count < 2 || count > 2)
	{
		gstate->bprint ("breakme [powerup]\n\nremoves one level of boost from an attribute.");
		gstate->bprint ("[powerup] can be:\n  power\n  speed\n  attack\n  acro\n  vita\n  all\n");
	}

	arg = gstate->GetArgv (1);

	if (!stricmp (arg, "power") && hook->power_boost > 0)
		hook->power_boost--;
	else if (!stricmp (arg, "speed") && hook->speed_boost > 0)
		hook->speed_boost--;
	else if (!stricmp (arg, "attack") && hook->attack_boost > 0)
		hook->attack_boost--;
	else if (!stricmp (arg, "acro") && hook->acro_boost > 0)
		hook->acro_boost--;
	else if (!stricmp (arg, "vita") && hook->vita_boost > 0)
		hook->vita_boost--;
	else if (!stricmp (arg, "all"))
	{
		if (hook->power_boost > 0)
			hook->power_boost--;
		if (hook->speed_boost > 0)
			hook->speed_boost--;
		if (hook->attack_boost > 0)
			hook->attack_boost--;
		if (hook->acro_boost > 0)		
			hook->acro_boost--;
		if (hook->vita_boost > 0)
			hook->vita_boost--;		
	}

	com->CalcBoosts (self);
}

///////////////////////////////////////////////////////////////////////////////
//	
///////////////////////////////////////////////////////////////////////////////

void	Items_InitCVars (void)
{
	deathmatch = gstate->cvar ("deathmatch", "0", CVAR_SERVERINFO|CVAR_LATCH);
	coop = gstate->cvar ("coop", "0", CVAR_SERVERINFO|CVAR_LATCH);
	skill = gstate->cvar ("skill", "100", CVAR_SERVERINFO|CVAR_LATCH);
	maxclients = gstate->cvar ("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
	p_gravity = gstate->cvar ("p_gravity", "4", CVAR_SERVERINFO | CVAR_LATCH);
	sv_episode = gstate->cvar ("sv_episode", "0", CVAR_SERVERINFO | CVAR_LATCH);
}

///////////////////////////////////////////////////////////////////////////////
//	dll_Query
///////////////////////////////////////////////////////////////////////////////

void *dll_Query(void)
{
	return(eclassStr);
}

///////////////////////////////////////////////////////////////////////////////
//	dll_ServerLoad
///////////////////////////////////////////////////////////////////////////////

void	dll_ServerLoad(serverState_t *state)
{
	gstate = state;
	
	gstate->Con_Printf ("* Loaded ITEMS.DLL, %i.\n", sizeof (playerHook_t));

	//	cheats for powering up
	gstate->AddCommand ("greyskull", af_powerup);
	gstate->AddCommand ("breakme", af_powerdown);
};

///////////////////////////////////////////////////////////////////////////////
// dll_Version
//
// returns TRUE if size == IONSTORM_DLL_INTERFACE_VERSION
///////////////////////////////////////////////////////////////////////////////

int dll_Version	(int size)
{
	if (size == IONSTORM_DLL_INTERFACE_VERSION)
		return	TRUE;
	else
		return	FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void dll_LevelLoad (void)
{
	Items_InitCVars ();
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void	dll_ServerInit (void)
{
	memory_com = com = (common_export_t *) gstate->common_exports;	

	Items_InitCVars ();
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void	dll_ServerKill (void)
{
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void	dll_LevelExit (void)
{
}

///////////////////////////////////////////////////////////////////////////////
//	dll_Entry
//
//
/////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	dll_Entry
//
//
///////////////////////////////////////////////////////////////////////////////

int	dll_Entry (HINSTANCE hParent, DWORD dwReasonForCall, PVOID pvData)
{
	int			size;

	switch (dwReasonForCall)
	{
		case	QDLL_VERSION:
			size = *(int *) pvData;

			return	dll_Version (size);
			break;
		
		case 	QDLL_QUERY:
			* (userEpair_t **) pvData = eclassStr;

			return	TRUE;
			break;

		case	QDLL_DESCRIPTION:
			* (char **) pvData = dll_Description;

			return	TRUE;
			break;

		case	QDLL_SERVER_INIT:
			dll_ServerInit ();

			return	TRUE;
			break;

		case	QDLL_SERVER_KILL:
			dll_ServerKill ();

			return	TRUE;
			break;

		case	QDLL_SERVER_LOAD:
			dll_ServerLoad ((serverState_t *) pvData);

			return	TRUE;
			break;

		case	QDLL_LEVEL_LOAD:
			dll_LevelLoad ();

			return	TRUE;
			break;

		case	QDLL_LEVEL_EXIT:
			dll_LevelExit ();

			return	TRUE;
			break;
	}

	return	FALSE;
}

