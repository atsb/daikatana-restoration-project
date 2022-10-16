



///////////////////////////////////////////////////////////////////////////////
//	p_main.c
//
//	initialization and shutdown routines for physics DLL
///////////////////////////////////////////////////////////////////////////////

//#include	"windows.h"
#include "dk_system.h"
#include	"memmgrcommon.h"

#include	"p_global.h"
#include	"p_user.h"

//	Client physics
#include	"p_client.h"

//	console commands for physics DLL
#include	"p_concmds.h"

#include  "p_inventory.h"
#include  "p_selector.h"
//#include  "p_sidekick.h"

#include    "l__language.h"
//#include    "ClientEntityManager.h"


#include	"csv.h"
#include	"dk_io.h"



///////////////////////////////////////////////////////////////////////////////
//	exports
///////////////////////////////////////////////////////////////////////////////

Quake_DllExport	void	*dll_Query	(void);

void *dll_Query(void)
{
	return (NULL);
}

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

game_locals_t	game;
level_locals_t	level;
game_import_t	gi;
game_export_t	globals;
memory_import_s memmgr;
// spawn_temp_t	st;
//ISoundDefs      g_SoundDefs ;   // jas 9/23/98

gclient_t		**ass;

serverState_t serverState;

int	sm_meat_index;
int	snd_fry;

edict_t		*g_edicts;

cvar_t	*deathmatch;
cvar_t	*coop;
cvar_t	*dmflags;
cvar_t	*skill;
cvar_t	*fraglimit;
cvar_t	*timelimit;
cvar_t	*samelevel;
cvar_t	*maxclients;
cvar_t	*maxentities;
cvar_t	*g_select_empty;
cvar_t	*g_unlimited_ammo;
cvar_t	*nomonsters;

cvar_t	*dm_respawn;

cvar_t	*p_maxvelocity;
cvar_t	*p_gravity;

cvar_t	*p_rollspeed;
cvar_t	*p_rollangle;
cvar_t	*gun_x;
cvar_t	*gun_y;
cvar_t	*gun_z;

cvar_t	*run_pitch;
cvar_t	*run_roll;
cvar_t	*bob_up;
cvar_t	*bob_pitch;
cvar_t	*bob_roll;

cvar_t	*p_cheats;
cvar_t	*dedicated;

//	Nelno:	Daikatana's base directory
cvar_t	*p_basedir;

//	Nelno:	physics frame rate multiplier -- 1.0 = normal framerate, < 1.0 = slower framerate
cvar_t	*p_frametime;
cvar_t	*p_runphysics;		// 1 = default, 0 = don't do any physics except for clients
cvar_t	*p_speeds;			// 0 = default, 1 = report how much time in ms physics takes
cvar_t	*p_sync;			// 1 = default, 0 = don't sync SVF_SYNCTOCLIENT entities
cvar_t	*p_showboxes;		// 0 = default  1 = show bounding boxes of entity in front of client
cvar_t	*p_sendparticles;	// 1 = particle commands are sent during multiplayer (default)

float	p_frametime_scaled;
int		p_realtime;

cvar_t	*p_spamkick;		//	auto-kick spammers if not 0
cvar_t	*p_spamcount;		//	how many spams before user gets kicked
cvar_t	*p_spamticks;		//	how long between messages from same use constitutes a spam
cvar_t	*p_spamresetticks;	//	how long before resetting spam counter

cvar_t	*sv_violence;
cvar_t  *allow_friendly_fire;

// multiplayer options
//cvar_t	*dm_frag_limit;
//cvar_t	*dm_time_limit;
cvar_t	*dm_skill_level;
cvar_t	*dm_max_players;

// deathmatch flags
cvar_t	*dm_weapons_stay;
cvar_t	*dm_spawn_farthest;
cvar_t	*dm_allow_exiting;
cvar_t	*dm_same_map;
cvar_t	*dm_force_respawn;
cvar_t	*dm_falling_damage;
//cvar_t	*dm_instant_powerups;
cvar_t	*dm_allow_powerups;
cvar_t	*dm_allow_health;
cvar_t	*dm_allow_armor;
cvar_t	*dm_infinite_ammo;
cvar_t	*dm_fixed_fov;
cvar_t	*dm_teamplay;
cvar_t	*dm_friendly_fire;
cvar_t  *dm_footsteps;
cvar_t  *dm_allow_hook;
cvar_t	*dm_use_skill_system;
cvar_t	*dm_instagib;
cvar_t	*unlimited_saves;

cvar_t  *cvarBuildPathTable;

cvar_t	*ctf_limit;	// WAW[12/9/99]: 
cvar_t	*ctf;
cvar_t	*deathtag;

cvar_t	*maps_dm;
cvar_t	*maps_ctf;
cvar_t	*maps_deathtag;

void SpawnEntities (char *mapname, char *entities, char *spawnpoint, qboolean bLoadgame);

///////////////////////////////////////////////////////////////////////////////
//	SetServerTime
//
//	Allows one to have access to the server time.
//
///////////////////////////////////////////////////////////////////////////////
void SetServerTime(float time)
{
	serverState.time = time;
}
	

///////////////////////////////////////////////////////////////////////////////
//	P_InitPerLevelCvars
//
//	initialize the CVars that need to be reset each level
//
//	Q2FIXME:	make sure these all need to be loaded each level.  Currently
//				it's just all the cvars that were in P_InitDLLs
///////////////////////////////////////////////////////////////////////////////

void	P_InitPerLevelCvars (void)
{
	// gun_x = gi.cvar ("gun_x", "0", 0);
	// gun_y = gi.cvar ("gun_y", "0", 0);
	// gun_z = gi.cvar ("gun_z", "0", 0);

	p_rollspeed = gi.cvar ("p_rollspeed", "200", 0);
	p_rollangle = gi.cvar ("p_rollangle", "2", 0);
	p_maxvelocity = gi.cvar ("p_maxvelocity", "4000", 0);
	p_gravity = gi.cvar ("p_gravity", "800", 0);

	// latched vars
	p_cheats = gi.cvar ("cheats", "0", CVAR_SERVERINFO|CVAR_LATCH);
	deathmatch = gi.cvar ("deathmatch", "0", CVAR_SERVERINFO|CVAR_LATCH);
	coop = gi.cvar ("coop", "0", CVAR_SERVERINFO|CVAR_LATCH);
	skill = gi.cvar ("skill", "1", CVAR_SERVERINFO|CVAR_LATCH);
	maxclients = gi.cvar ("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
	maxentities = gi.cvar ("maxentities", "1024", CVAR_LATCH);
	dedicated = gi.cvar ("dedicated","0",0);

	// change anytime vars
	dmflags = gi.cvar ("dmflags", "0", CVAR_SERVERINFO);
	fraglimit = gi.cvar ("fraglimit", "0", CVAR_SERVERINFO|CVAR_LATCH|CVAR_ARCHIVE);
	timelimit = gi.cvar ("timelimit", "0", CVAR_SERVERINFO|CVAR_LATCH|CVAR_ARCHIVE);
	samelevel = gi.cvar ("samelevel", "0", CVAR_SERVERINFO);
	g_select_empty = gi.cvar ("g_select_empty", "0", CVAR_ARCHIVE);
	g_unlimited_ammo = gi.cvar ("g_unlimited_ammo", "0", CVAR_SERVERINFO);
	dm_respawn = gi.cvar ("dm_respawn", "2", CVAR_SERVERINFO);
	nomonsters = gi.cvar ("nomonsters", "0", CVAR_SERVERINFO);

	run_pitch = gi.cvar ("run_pitch", "0.002", 0);
	run_roll = gi.cvar ("run_roll", "0.005", 0);
	bob_up  = gi.cvar ("bob_up", "0.005", 0);
	bob_pitch = gi.cvar ("bob_pitch", "0.002", 0);
	bob_roll = gi.cvar ("bob_roll", "0.002", 0);

	//	Nelno: force set???
	p_frametime = gi.cvar_forceset ("p_frametime", "1.0");

   // temp shit for status bar (for screenshots)
	gi.cvar("st_armor","0",0);
	gi.cvar("st_level","0",0);

	p_runphysics = gi.cvar("p_runphysics","1",0);
	p_speeds = gi.cvar("p_speeds","0",0);
	p_sync = gi.cvar("p_sync","1",0);
	p_showboxes = gi.cvar("p_showboxes","0",0);
	p_sendparticles = gi.cvar ("p_sendparticles","0",0);

	p_spamkick = gi.cvar ("p_spamkick", "1", CVAR_ARCHIVE);
	p_spamticks = gi.cvar ("p_spamticks", "20", CVAR_ARCHIVE);
	p_spamresetticks = gi.cvar ("p_spamresetticks", "1200", CVAR_ARCHIVE);
	p_spamcount = gi.cvar ("p_spamcount", "20", CVAR_ARCHIVE);

	sv_violence = gi.cvar ("sv_violence", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE);

//	dm_frag_limit		= gi.cvar ("dm_frag_limit", "0", CVAR_SERVERINFO|CVAR_LATCH);
//	dm_time_limit		= gi.cvar ("dm_time_limit", "0", CVAR_SERVERINFO|CVAR_LATCH);
	dm_skill_level		= gi.cvar ("dm_skill_level", "0", CVAR_SERVERINFO|CVAR_LATCH);
	dm_max_players		= gi.cvar ("dm_max_players", "0", CVAR_SERVERINFO|CVAR_LATCH);
	dm_weapons_stay		= gi.cvar ("dm_weapons_stay", "0", CVAR_SERVERINFO|CVAR_LATCH);
	dm_spawn_farthest	= gi.cvar ("dm_spawn_farthest", "0", CVAR_SERVERINFO|CVAR_LATCH);
	dm_allow_exiting	= gi.cvar ("dm_allow_exiting", "1", CVAR_SERVERINFO|CVAR_LATCH);
	dm_same_map			= gi.cvar ("dm_same_map", "0", CVAR_SERVERINFO|CVAR_LATCH);
	dm_force_respawn	= gi.cvar ("dm_force_respawn", "0", CVAR_SERVERINFO|CVAR_LATCH);
	dm_falling_damage	= gi.cvar ("dm_falling_damage", "1", CVAR_SERVERINFO|CVAR_LATCH);
//	dm_instant_powerups	= gi.cvar ("dm_instant_powerups", "1", CVAR_SERVERINFO|CVAR_LATCH);
	dm_allow_powerups	= gi.cvar ("dm_allow_powerups", "1", CVAR_SERVERINFO|CVAR_LATCH);
	dm_allow_health		= gi.cvar ("dm_allow_health", "1", CVAR_SERVERINFO|CVAR_LATCH);
	dm_allow_armor		= gi.cvar ("dm_allow_armor", "1", CVAR_SERVERINFO|CVAR_LATCH);
	dm_infinite_ammo	= gi.cvar ("dm_infinite_ammo", "0", CVAR_SERVERINFO|CVAR_LATCH);
	//dm_fixed_fov		= gi.cvar ("dm_fixed_fov", "0", CVAR_SERVERINFO|CVAR_LATCH);
	dm_teamplay			= gi.cvar ("dm_teamplay", "0", CVAR_SERVERINFO|CVAR_LATCH);
	dm_friendly_fire	= gi.cvar ("dm_friendly_fire", "0", CVAR_SERVERINFO|CVAR_LATCH);
    dm_footsteps        = gi.cvar ("dm_footsteps", "1", CVAR_SERVERINFO|CVAR_LATCH);
    dm_allow_hook       = gi.cvar ("dm_allow_hook", "1", CVAR_SERVERINFO|CVAR_LATCH);
    dm_use_skill_system = gi.cvar ("dm_use_skill_system", "1", CVAR_SERVERINFO|CVAR_LATCH);
	dm_instagib			= gi.cvar ("dm_instagib", "0", CVAR_SERVERINFO|CVAR_LATCH);
	unlimited_saves		= gi.cvar ("unlimited_saves", "0", CVAR_ARCHIVE);


    allow_friendly_fire = gi.cvar( "allow_friendly_fire", "1", CVAR_ARCHIVE );

	deathtag			= gi.cvar("deathtag","0", CVAR_SERVERINFO|CVAR_LATCH);
	ctf					= gi.cvar("ctf", "0", CVAR_SERVERINFO|CVAR_LATCH);
	ctf_limit			= gi.cvar("ctf_limit","0", CVAR_SERVERINFO|CVAR_LATCH|CVAR_ARCHIVE);

    cvarBuildPathTable = gi.cvar( "BuildPathTable", "", 0 ); 
	
	maps_dm				= gi.cvar("maps_dm","",CVAR_SERVERINFO|CVAR_ARCHIVE);
	maps_deathtag		= gi.cvar("maps_deathtag","",CVAR_SERVERINFO|CVAR_ARCHIVE);
	maps_ctf			= gi.cvar("maps_ctf","",CVAR_SERVERINFO|CVAR_ARCHIVE);
}

///////////////////////////////////////////////////////////////////////////////
//	P_InitEdicts
//
//	get the memory for all the edicts
///////////////////////////////////////////////////////////////////////////////

void	P_InitEdicts (void)
{

	if (g_edicts)
		gi.Error ("Edict memory was not freed.\n");

	///////////////////////////////////////////////////////////////////////////
	//	initialize all entities for this game
	///////////////////////////////////////////////////////////////////////////

	game.maxentities = maxentities->value;
	if (maxclients->value * 8 > game.maxentities)
		game.maxentities = maxclients->value * 8;

	g_edicts =  (edict_t *) gi.X_Malloc( game.maxentities * sizeof(g_edicts[0] ), MEM_TAG_GAME);
	globals.edicts = g_edicts;
	globals.max_edicts = game.maxentities;

	///////////////////////////////////////////////////////////////////////////////
	// initialize all clients for this game
	///////////////////////////////////////////////////////////////////////////////

	game.maxclients = maxclients->value;
	game.clients = (gclient_t *) gi.X_Malloc ( game.maxclients * sizeof(game.clients[0] ), MEM_TAG_GAME);
	globals.num_edicts = game.maxclients+1;
	memset( game.clients, 0, game.maxclients * sizeof( game.clients[0] ) );

	game.transients = ( transient_t * ) gi.X_Malloc ( 8 * sizeof( transient_t ), MEM_TAG_GAME );
	memset( game.transients, 0, 8 * sizeof( transient_t ) );

	//	Nelno:	set up g_edicts for DLLs using gstate
	serverState.g_edicts = g_edicts;
}

///////////////////////////////////////////////////////////////////////////////
//	P_InitDLLs
//
//	This will be called when the dll is first loaded, which
//	only happens when a new game is begun
//
//	Nelno:	mallocs memory for edicts
///////////////////////////////////////////////////////////////////////////////

void P_InitDLLs (void)
{
	gi.Con_Dprintf("---- P_InitGame ----\n");

	// items
	// InitItems ();

	//	Q2FIXME:	what's this do?
/*
	Com_sprintf (game.helpmessage1, sizeof(game.helpmessage1), "No help message1");

	Com_sprintf (game.helpmessage2, sizeof(game.helpmessage2), "No help message2");
*/
	// initialize the function list for save/load
	P_InitFuncList();

	// register functions
	P_RegisterFunc("P_FreeEdict",P_FreeEdict);
	P_RegisterFunc("P_PercentSubmerged",P_PercentSubmerged);
	P_RegisterFunc("P_FlyMove",P_FlyMove);

	P_InitPerLevelCvars ();

	P_InitEdicts ();

	com_ServerLoad ();

	//	all DLLs have been loaded, and all of their dll_ServerLoad functions have executed
	//	so run their init functions (init functions can rely on other DLLs already having
	//	been set up).
	DLL_InitFunctions ();

//	g_cvarLogOutput = gi.cvar( "LogOutput", "1", CVAR_ARCHIVE );

//	cvar_t* cvarLogFilePath;
//	cvarLogFilePath = gi.cvar( "LogFilePath", "", CVAR_ARCHIVE );
//	DKLOG_Initialize( cvarLogFilePath->string );
	
//	DKLOG_Write( LOGTYPE_PHYSICS, 0.0, "P_InitDLLs()" );

}

///////////////////////////////////////////////////////////////////////////////
//	P_InitChangelevel
//
//	called when a "changelevel" command is issued.
//
//	mallocs memory for edicts and clears per-level Cvars
///////////////////////////////////////////////////////////////////////////////

void P_InitChangelevel (void)
{
	gi.Con_Dprintf("---- P_InitChangelevel ----\n");

	// items
	// InitItems ();

	//	reset level locals
	memset (&level, 0x00, sizeof (level_locals_t));
	
	//	Q2FIXME:	what's this do?
/*
	Com_sprintf (game.helpmessage1, sizeof(game.helpmessage1), "No help message1");

	Com_sprintf (game.helpmessage2, sizeof(game.helpmessage2), "No help message2");
*/

	P_InitPerLevelCvars ();

	P_InitEdicts ();

	//	all DLLs have been loaded, and all of their dll_ServerLoad functions have executed
	//	so run their init functions (init functions can rely on other DLLs already having
	//	been set up).
	DLL_InitFunctions ();
}

///////////////////////////////////////////////////////////////////////////////
//	P_LevelExit
//
//	called by server when switching maps.  Calls dll entry functions with
//	QDLL_LEVEL_EXIT
///////////////////////////////////////////////////////////////////////////////

void	P_LevelExit (void)
{
	gi.Con_Dprintf("---- P_LevelExit ----\n");

	//	call exit level functions for each DLL loaded by physics.dll
	DLL_ExitLevelFunctions ();

	com_LevelExit ();
}

///////////////////////////////////////////////////////////////////////////////
//	P_LevelLoad
//
//	called by server when switching maps, after entities have been spawned and
//	just prior to running the first two physics frames.  Calls dll entry 
//	functions with QDLL_LEVEL_LOAD
///////////////////////////////////////////////////////////////////////////////

void	P_GetSoundIndices (void);

void	P_LevelLoad (void)
{
	gi.Con_Dprintf("---- P_LevelLoad ----\n");

	com_LevelLoad ();

	//	call load level functions for each DLL loaded by physics.dll
	DLL_LoadLevelFunctions ();

	P_GetSoundIndices ();
}

///////////////////////////////////////////////////////////////////////////////
//	P_UnloadDLLs
//
//	called to unload all DLLs from memory
///////////////////////////////////////////////////////////////////////////////

void	P_UnloadDLLs (void)
{
	gi.Con_Dprintf("---- P_UnloadDLLs ----\n");

	//	Unload all of the dlls that physics.dll loaded
	DLL_UnloadDLLs ();
}

// SCG[10/23/99]: hacked in shit 
int P_ClientConnect( userEntity_t *self, char *userinfo, int loadgame )
{
	self->client = game.clients + ( self - g_edicts - 1 );
	self->input_entity = self;
	return dll_ClientConnect( self, userinfo, loadgame );
}

// need this to find out the monster type below....
class p_playerHook_t 
{ 
public:
   	//unix - fixed inline constructor
	//__inline playerHook_t();
	p_playerHook_t() 
	{
//		nFollowing	= FOLLOWING_NONE;
		//memset(this,0,sizeof(playerHook_t));
	}

	userEntity_t		*owner;

	//////////////////////////////////////////////
	//	stats
	//////////////////////////////////////////////

	float		base_health;
	char		pain_chance;
	int			intQuota;			// how smart this monster/bot is 

	int			active_distance;	// distance from a client before
									// becoming inactive;
	short		back_away_dist;		// distance from goal when entity will back away
	char		type;				// what monster is this?  For avoiding stricmps
};

bool P_CanSave (userEntity_t *ent, bool msg)
{
	if (!ent || !(ent->flags & FL_CLIENT) )
	{
		if (msg)
			Com_Printf("Non-client sent to CanSave()\n");
		return false;
	}

	if (ent->health <= 0)
	{
		if (msg)
			Com_Printf (tongue_save_error[TONGUE_SAVE_ERR_DEAD]);
		return false;
	}

	if (serverState.level->intermissiontime > 0)
	{
		if (msg)
			Com_Printf (tongue_save_error[TONGUE_SAVE_ERR_INTERMISSION]);
		return false;
	}

	if ((unlimited_saves->value == 0.0) && serverState.InventoryItemCount(ent->inventory,"item_savegem") < 1)
	{
		if (msg)
			serverState.centerprint(ent, 2.0, tongue[T_NO_SAVEGEMS]);
		return false;
	}

	// if we have no ground entity, we're in air or in water.  only save if in water.
	if ( !ent->groundEntity )
	{
		if (ent->watertype & CONTENTS_WATER)	// swimming
		{
			return true;
		}
		else
		{
			if (msg)
				serverState.centerprint(ent, 2.0, tongue_save_error[TONGUE_SAVE_ERR_HERE]);
			return false;
		}
	}
	else if (ent->groundEntity->velocity.Length() > 0)
	{
		if (msg)
			serverState.centerprint(ent, 2.0, tongue_save_error[TONGUE_SAVE_ERR_HERE]);
		return false;
	}
/*
	// special case hack.  no saving while kage is running around.
	if (serverState.episode == 4)
	{
		p_playerHook_t *phook;
		int i = 0;
		userEntity_t *entity = g_edicts;
		for (; i < globals.num_edicts; i++, entity++)
		{
			if (!entity->inuse)
				continue;

			if ( !(entity->flags & FL_MONSTER) || !entity->userHook || (entity->health <= 0) ) 
				continue;
		
			phook = (p_playerHook_t *)entity->userHook;
			if (phook->type == 90)
			{
				if (msg)
					serverState.centerprint(ent, 2.0, "Cannot save here!\n");
				return false;
			}
		}

		return true;
	}
*/
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//	GetGameAPI
//
//	Returns a pointer to the structure with all entry points
//	and global variables.  Called only when physics.dll is loaded!!!!!!!!!!!!!!
///////////////////////////////////////////////////////////////////////////////

//	Q2FIXME:	rename to something more appropriate, since this inits everything
DllExport game_export_t *GetGameAPI (game_import_t *import)
{
	char	dll_init_dir	[1024];

	gi = *import;

	memmgr.X_Malloc = gi.X_Malloc;
	memmgr.X_Free = gi.X_Free;

	gi.Con_Dprintf("PHYSICS.DLL: GetGameAPI\n");

	///////////////////////////////////////////////////////////////////////////////
	//	initialize server state
	///////////////////////////////////////////////////////////////////////////////

	InitServerState (&gi);

	///////////////////////////////////////////////////////////////////////////////
	//	load other dlls and call their dll_ServerLoad functions
	///////////////////////////////////////////////////////////////////////////////
	
	//sprintf (dll_init_dir, "%s\\%s", gi.dk_gamedir, gi.dk_dlldir);
// SCG[1/16/00]: 	sprintf (dll_init_dir, "./%s", gi.dk_dlldir);
	Com_sprintf (dll_init_dir,sizeof(dll_init_dir), "./%s", gi.dk_dlldir);
	DLL_LoadDLLs (dll_init_dir);

	///////////////////////////////////////////////////////////////////////////////
	//	set up pointers for calling from server
	///////////////////////////////////////////////////////////////////////////////

	globals.apiversion = GAME_API_VERSION;
	globals.SpawnEntities = SpawnEntities;

	globals.WriteGame = P_WriteGame;
	globals.ReadGame = P_ReadGame;
	globals.WriteLevel = P_WriteLevel;
	globals.ReadLevel = P_ReadLevel;
	globals.WriteHeader = P_WriteHeader;

	globals.ClientThink = Client_Think;
	globals.ClientConnect = P_ClientConnect;
	globals.ClientUserinfoChanged = dll_ClientUserinfoChanged;
	globals.ClientDisconnect = dll_ClientDisconnect;
	globals.ClientBegin = dll_ClientBegin;

	//	Nelno:	added these for DLL init, de-init functions
	globals.LevelExit = P_LevelExit;
	globals.LevelLoad = P_LevelLoad;

	//NSS[11/11/99]:
	globals.SetServerTime = SetServerTime;

	globals.InitDLLs = P_InitDLLs;
	globals.UnloadDLLs = P_UnloadDLLs;

	globals.InitChangelevel = P_InitChangelevel;
	globals.RegisterFunc = P_RegisterFunc;

	//	Nelno:	no longer used
	globals.ClientCommand = NULL;

	globals.RunFrame = P_RunFrame;

	//	Q2FIXME: add this
	globals.ServerCommand = NULL;

	globals.edict_size = sizeof(edict_t);

	globals.LoadNodes = dll_LoadNodes;
	globals.RegisterWorldFuncs = dll_RegisterWorldFuncs;

	globals.EntityLoadCleanup = dll_EntityLoadCleanup;

	// cek[5-8-00]: determines whether a save can be completed.
	globals.CanSave = P_CanSave;
	//	Nelno:	add console commands for physics DLL here
	//	this is because we're throwing out the ClientCommand crap
	concmd_AddCommands ();

	inventory_AddCommands();		  // add commands for inventory
	//sidekickcommand_AddCommands();   // add commands for sidekick command
  cmdselector_AddCommands();       // add commands for command selector


	return &globals;
}

#ifndef GAME_HARD_LINKED
// this is only here so the functions in dk_shared.c and dk_shwin.c can link
void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	gi.Error ("%s", text);
}

void Com_Printf (char *msg, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	gi.Con_Dprintf("%s", text);
}

#endif

//======================================================================

static char mapList[64][16];
static int mapCount = 0;

void ParseMapList(const char *type)
{
	cvar_t *cv = NULL;	
	if (!stricmp(type,"deathmatch"))
		cv = maps_dm;
	else if (!stricmp(type,"ctf"))
		cv = maps_ctf;
	else if (!stricmp(type,"deathtag"))
		cv = maps_deathtag;

	if (!cv)
		return;


	char ml[1024];

	Com_sprintf(ml,sizeof(ml),"%s",cv->string);
	memset(mapList,0,sizeof(mapList));

	char *token, *ptr = ml;

	mapCount = 0;
	while (mapCount < 16)
	{
		// skip whitespace
		while (*ptr && *ptr <= ' ' && *ptr != '\n')
		{
			ptr++;
		}

		// at the end?
		if ( !*ptr || (*ptr == '\n') )
		{	
			return;
		}

		token = COM_Parse ((char **)&ptr);
		if (!ptr)
			return;

		strcpy (mapList[mapCount], token);
		mapCount++;
	}
}
/*
=================
ChooseNextMap

Choses the next level from the csv - places the name into mapname. 
=================
*/
bool ChooseNextMap( const char *type, char mapname[64] )
{
	bool bfound = false;
	
	ParseMapList(type);
	if (mapCount)
	{
		int nextMap;
		for (int i = 0; i < mapCount; i++)
		{
			if (!stricmp(level.mapname,mapList[i]))		// found the current map!
			{
				bfound = true;
				nextMap = i+1;
				break;
			}
		}

		if ( bfound )
		{
			if (nextMap >= mapCount)
				nextMap = 0;
		}
		else 
			nextMap = 0;	// if current map not found, load the first map in the cycle.

		strcpy( mapname, mapList[nextMap] );
		return true;
	}
	else
	{
		// if no maps in the list, use the CSV to find the next map
		CCSVFile *pCsvFile = NULL;
		char szLine[2048];
		char szMapName[64];
		char szType[64];
		char szFirstMapName[64];
		*szFirstMapName = '\0';

		int nRetCode = CSV_OpenFile( "MultiplayerMaps.csv", &pCsvFile );
		if ( nRetCode == DKIO_ERROR_NONE )
		{
			while (CSV_GetNextLine(pCsvFile,szLine) != EOF)
			{
				// get level name
				CSV_GetFirstElement(pCsvFile, szLine, szMapName);
				CSV_GetNextElement(pCsvFile, szLine, szType );
				if (!_stricmp(szType, type ))
				{
					if (!*szFirstMapName)
						strcpy( szFirstMapName, szMapName );
					if (bfound)
					{
						strcpy( mapname, szMapName );
						CSV_CloseFile( pCsvFile );
						return true;
					}
					else if (!_stricmp( szMapName, level.mapname ))
						bfound = true;
				}
			}
			CSV_CloseFile( pCsvFile );
			if (bfound)		// Found but no next map... use the first one of that type found.
				strcpy( mapname, szFirstMapName );
		}
	}

	return bfound;
}



//======================================================================

/*
=================
EndDMLevel

The timelimit or fraglimit has been exceeded
=================
*/
void	P_ScoreboardMessage (userEntity_t *dest_ent);
void EndDMLevel (void)
{
	userEntity_t	*ent;
    userEntity_t    *dest_ent;
    int             i;

#ifdef TONGUE_ENGLISH
	for (dest_ent = &g_edicts[0],i = 0; i < maxclients->value; i++, dest_ent++)
	{
        if(! dest_ent->inuse)
            continue;

		serverState.StartEntitySound(dest_ent, CHAN_AUTO, serverState.SoundIndex("global/matchcomplete.wav"),1.0, 64, 64);
	}
#endif	

	// stay on same level flag
	if (dm_same_map->value)
	{
 		dll_BeginIntermission(level.mapname);
	}
//	else if (level.nextmap[0])
//	{	// go to a specific map
// 		dll_BeginIntermission(level.nextmap);
//	}
	else
	{	// search for a changelevel
		char mapName[64];
		
		char *gameType = NULL;
		if (deathmatch->value)
		{
			if (ctf->value)
				gameType = "ctf";
			else if (deathtag->value)
				gameType = "deathtag";
			else
				gameType = "deathmatch";
		}

		if (gameType && ChooseNextMap( gameType, mapName ))
		{
			dll_BeginIntermission( mapName );
		}
		else
		{
			ent = com.FindTarget( "trigger_changelevel" );
			if (!ent)
			{
				dll_BeginIntermission(level.mapname);
				// gi.Error( "no trigger_changelevel!\n");
			} else {
				dll_BeginIntermission (ent->message);
			}
		}
	}    // send scoreboard message
	dest_ent = &g_edicts[0];

	for (i = 0; i < maxclients->value; i++, dest_ent++)
	{
        if(! dest_ent->inuse)
            continue;
        P_ScoreboardMessage (dest_ent);
    }
}

///////////////////////////////////////////////////////////////////////////////
//	P_ExitLevel
//
///////////////////////////////////////////////////////////////////////////////
int com_transitionType(char *mapFrom, char *mapTo, qboolean loadgame);
void P_ExitLevel (void)
{
	int				i;
	userEntity_t	*ent;
	char			command [256];

	//	Nelno:	FIXME:	changed "gamemap" to "map" what exactly did gamemap do?
	int type = com_transitionType(level.mapname,level.nextMap,false);
	char nextMap[64];
	if ( !(deathmatch->value) && ((type == 2) || (type == 0)) )
	{
		Com_sprintf (nextMap, sizeof(nextMap),"*%s", level.nextMap);
	}
	else
	{
		Com_sprintf (nextMap, sizeof(nextMap),"%s", level.nextMap);
	}

	if (coop->value)
	{
		int length = strlen( level.nextMap );
		char test = level.nextMap[length - 1];
		test = tolower(test);

		if ( (test == 'a') && (type != 1) )
			Com_sprintf (command, sizeof(command), "changelevel %s\n", nextMap);
		else
			Com_sprintf (command, sizeof(command), "changelevel %s noplaque\n", nextMap);
	}
	else
	{
		Com_sprintf (command, sizeof(command), "changelevel %s\n", nextMap);
	}

	gi.CBuf_AddText(command);

//	level.changemap = NULL;
	level.exitintermission = 0;
	level.intermissiontime = 0;
	
	// clear some things before going to next level
	// cek[2-13-00]: lets do this before ending the frames...so the scoreboard will be cleared out..
	for (i = 0; i < maxclients->value; i++)
	{
		ent = g_edicts + 1 + i;

		if (!ent->inuse)
			continue;

		ent->flags &= ~(FL_NOTARGET|FL_FREEZE);
	    ent->client->showflags &= ~(SHOW_SCORES | SHOW_SCORES_NOW);	//	send update immediately
		ent->client->ps.rdflags &= ~RDF_LETTERBOX;
		ent->client->endIntermission = TRUE;
	}

	Client_EndServerFrames ();
}

///////////////////////////////////////////////////////////////////////////////
//	P_CheckDMRules
//
///////////////////////////////////////////////////////////////////////////////

void P_CheckDMRules (void)
{
	int			i;
//	gclient_t	*cl;
	edict_t		*ent;

	if (level.intermissiontime)
		return;

	if (!deathmatch->value)
		return;

	// check the ctf rules.  ctf_limit and ctf_timelimit are handled within
	if (ctf->value || deathtag->value)
	{
		if (dll_FLAG_CheckRules())
			EndDMLevel();

		return;
	}

	if (timelimit->value)
	{
		if (level.time >= timelimit->value*60)
		{
			gi.bprintf (PRINT_HIGH, "%s",tongue_ctf[T_CTF_TIME_EXPIRED]);// umm..empty tongue_world[T_PLAYER_TIME_LIMIT_HIT]);
			EndDMLevel ();
			return;
		}
	}

	if (fraglimit->value)
	{
		// cek[3-20-00]: make a very timely change to the teamplay dm scoring system
		if (dm_teamplay->value)
		{
			short teamscores[8];
			memset(&teamscores,0,sizeof(teamscores));
			int team;
			for (i=0 ; i<maxclients->value ; i++)
			{
				ent = &g_edicts [i + 1];
				if (!ent->inuse)
					continue;

				team = ent->team - 1;
				if ((team >= 0) && (team < 8))
				{
					teamscores[team] += ent->record.frags;
					if (teamscores[team] >= fraglimit->value)
					{
						gi.bprintf (PRINT_HIGH, "%s",tongue_deathtag[TONGUE_DT_SCORE_LIMIT]);//tongue_world[T_PLAYER_FRAG_LIMIT_HIT]);
						EndDMLevel ();
						return;
					}
				}
			}
		}
		else
		{
			for (i=0 ; i<maxclients->value ; i++)
			{
				ent = &g_edicts [i + 1];
				if (!ent->inuse)
					continue;

				if (ent->record.frags >= fraglimit->value)
				{
					gi.bprintf (PRINT_HIGH, "%s",tongue_deathtag[TONGUE_DT_SCORE_LIMIT]);//tongue_world[T_PLAYER_FRAG_LIMIT_HIT]);
					EndDMLevel ();
					return;
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////
/// Physics
//////////////////////////////////////////////////////////////////////////////////
// Logic[5/10/99]: E3 Nitro Hack
void P_CheckNitro(userEntity_t *self)
{
    int iPC;
    if( (iPC = gi.pointcontents(self->s.origin)) & CONTENTS_NITRO)
    {
        self->s.renderfx |= RF_IN_NITRO;
        self->s.alpha = 0.1111;
    } else if(self->s.renderfx & RF_IN_NITRO) {
        self->s.renderfx &= ~RF_IN_NITRO;
        if(self->s.alpha == 0.1111)
            self->s.alpha = 0;
    }
}
//////////////////////////////////////////////////////////////////////////////////
//	P_RunFrame
//
//	Advances the world by 0.1 seconds
//////////////////////////////////////////////////////////////////////////////////

void P_RunFrame (void)
{
	int		i, startTime, elapsedTime;
	static	int maxProfileTime = 0, maxTraceCount = 0;
	edict_t	*ent;

	//	actual time last physics frame started, used by P_RunSyncFrame
	p_realtime = gi.Sys_Milliseconds ();

	//	calculate p_frametime based on p_frametime multiplier
	p_frametime_scaled = FRAMETIME * p_frametime->value;

	level.framenum++;
	//	Nelno:	FIXME:	is it right not to scale frametime???
	level.time = level.framenum * FRAMETIME;
	serverState.frametime = level.time - serverState.time; 
	serverState.time = level.time;
    serverState.realtime = p_realtime;

	// choose a client for monsters to target this frame
	// AI_SetSightClient ();

	if (level.exitintermission)
	{
		//	exit intermission
		P_ExitLevel ();
		return;
	}

	//&&& AMW - display bounding box of entity found in client traceline
	if (p_showboxes->value && (globals.num_edicts > 0))
	{
		// get the client entity
		ent = &g_edicts[1];	
		
		if (ent && ent->client && ent->className && !strcmp(ent->className,"player") &&
			dll_UpdateBoundingBoxes)
		{
			// function in world.cpp to draw a bounding box around 
			// the entity you're pointed at
			dll_UpdateBoundingBoxes( ent );
		}

	}

	// debug/profiling code
	if (p_speeds->value)
	{
		startTime = gi.Sys_Milliseconds ();
	}

	//
	// treat each object in turn
	// even the world gets a chance to think
	//

	ent = &g_edicts[0];


	//	do stuff for clients
	for (i = 0; i <= maxclients->value; i++, ent++)
	{
		if (!ent->inuse)
		{
			continue;
		}
		level.current_entity = ent;

		///////////////////////////////////////////////////////////////////////////
		//	Run client-only stuff, skip over clients
		///////////////////////////////////////////////////////////////////////////

		if (i > 0)
		{
			//	make sure we're connected!!  inuse only gets set right before ClientBegin is called
			Client_BeginServerFrame (ent);
		}
		// else 
		if ( !(ent->s.renderfx & (RF_BEAM | RF_SPOTLIGHT)) )
		{
			ent->s.old_origin = ent->s.origin;
		}

		//	Nelno:	clear EF_RESPAWN flag for each entity every frame, or it could
		//	get sent the next time the client enters this entities PVS!
//			ent->s.effects &= ~EF_RESPAWN;

		///////////////////////////////////////////////////////////////////////////
		//	PreThinks!!
		///////////////////////////////////////////////////////////////////////////
		if (ent->prethink && !level.intermissiontime)
		{
			ent->prethink (ent);
		}

		//	just run the think function
		P_RunThink (ent, true);

		///////////////////////////////////////////////////////////////////////////
		//	PostThinks!!
		///////////////////////////////////////////////////////////////////////////

		if (ent->postthink && !level.intermissiontime)
		{
			ent->postthink (ent);
		}

		if (ent->svflags & SVF_NEWSPAWN)
		{
			ent->svflags &= ~SVF_NEWSPAWN;
		}
	}

	if (p_runphysics->value)
	{
		for (i = i ; i < globals.num_edicts ; i++, ent++)
		{
			if (!ent->inuse)
			{
				continue;
			}

			level.current_entity = ent;

			///////////////////////////////////////////////////////////////////////////
			//	Run client-only stuff, skip over clients
			///////////////////////////////////////////////////////////////////////////

			if ( !(ent->s.renderfx & (RF_BEAM | RF_SPOTLIGHT)) )
			{
				ent->s.old_origin = ent->s.origin;
			}

			//	Nelno:	clear EF_RESPAWN flag for each entity every frame, or it could
			//	get sent the next time the client enters this entities PVS!
			ent->s.effects &= ~EF_RESPAWN;

			///////////////////////////////////////////////////////////////////////////
			//	PreThinks!!
			///////////////////////////////////////////////////////////////////////////

			if (ent->prethink)
			{
				ent->prethink (ent);
			}

			if (!(ent->flags & FL_HCHILD))
			{
				//	run physics for all but client
				P_RunEntity (ent, true);
                // Logic[5/10/99]: E3 Nitro Hack
                if(ent->flags & FL_BOT)
                    P_CheckNitro(ent);

                
			}
			else 
			if (ent->think)
			{
				//	just run the think function
				P_RunThink (ent, true);
			}

			///////////////////////////////////////////////////////////////////////////
			//	PostThinks!!
			///////////////////////////////////////////////////////////////////////////
			if (ent->postthink)
				ent->postthink (ent);

			if (ent->svflags & SVF_NEWSPAWN)
			{
				ent->svflags &= ~SVF_NEWSPAWN;
			}
		}
	}

	// debug/profiling code
#ifdef _DEBUG
	if (p_speeds->value)
	{
		elapsedTime = gi.Sys_Milliseconds () - startTime;
		if (elapsedTime > maxProfileTime)
			maxProfileTime = elapsedTime;
		
		int traceCount = gi.GetTraceCount();
		if (traceCount > maxTraceCount)
			maxTraceCount = traceCount;

		gi.Con_Dprintf("PHYSICS: %ims  MAX: %ims  TRACES: %i  MAX: %i\n", 
			elapsedTime, 
			maxProfileTime,
			traceCount,
			maxTraceCount);
	}
	else
#endif
	{
		maxTraceCount = maxProfileTime = 0;
	}


	// see if it is time to end a deathmatch
	P_CheckDMRules ();

//    serverState.ClientEntityManager->RunFrame();

	//	Nelno: finish up with special routines for clients
	//	adjust client view if client was affected by other objects
	//	do water damage, etc.
	Client_EndServerFrames ();
}
