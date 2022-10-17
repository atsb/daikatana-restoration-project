#include "../GOA/SDK/goautil.h"

#include "server.h"
server_static_t	svs;				// persistant server info
server_t		sv;					// local server


void SetServerTime(float time);

/*
================
SV_FindIndex
================
*/
int SV_FindIndex (const char *name, int start, int max, qboolean create)
{
	int		i;
	
	if (!name || !name[0])
		return 0;
	
	for (i = 1 ; i < max && sv.configstrings [start+i][0] ; i++)
		if (!strcmp(sv.configstrings [start+i], name))
			return i;
	
	if (!create)
		return 0;
		
	if (i == max)
		Com_Error (ERR_DROP, "*Index: overflow");
	
	strncpy (sv.configstrings[start+i], name, sizeof(sv.configstrings[i]));
	
	if (sv.state != ss_loading)
	{	// send the update to everyone
		SZ_Clear (&sv.multicast);
		MSG_WriteChar (&sv.multicast, svc_configstring);
		MSG_WriteShort (&sv.multicast, start+i);
		MSG_WriteString (&sv.multicast, name);
		//SV_Multicast (CVector(0,0,0), MULTICAST_ALL_R);
	}
	
	return i;
}
int SV_ModelIndex (const char *name)
{
	return SV_FindIndex (name, CS_MODELS, MAX_MODELS, true);
}
int SV_SoundIndex (const char *name)
{
    if(name == ((char *)-1))
        return -1;
	return SV_FindIndex (name, CS_SOUNDS, MAX_SOUNDS, true);
}
int SV_ImageIndex (const char *name)
{
	return SV_FindIndex (name, CS_IMAGES, MAX_IMAGES, true);
}

///////////////////////////////////////////////////////////////////////////////
//	SV_GetConfigString
///////////////////////////////////////////////////////////////////////////////

char	*SV_GetConfigString (int index)
{
	return	sv.configstrings [index];
}

/*
================
SV_CreateBaseline
Entity baselines are used to compress the update messages
to the clients -- only the fields that differ from the
baseline will be transmitted
================
*/
void SV_CreateBaseline (void)
{
	edict_t			*svent;
	int				entnum;	
	for (entnum = 1; entnum < ge->num_edicts ; entnum++)
	{
		svent = EDICT_NUM(entnum);
		if (!svent->inuse)
			continue;
		if (!svent->s.modelindex && !svent->s.sound && !svent->s.effects)
			continue;
		svent->s.number = entnum;
		//
		// take current state as baseline
		//
		svent->s.old_origin = svent->s.origin;
		sv.baselines[entnum] = svent->s;
	}
}
/*
=================
SV_CheckForSavegame
=================
*/
void SV_CheckForSavegame (void)
{
	char		name[MAX_OSPATH];
	FILE		*f;
	int			i;

	if (sv_noreload->value)
		return;

	if (Cvar_VariableValue ("deathmatch"))
		return;
/*
	if( sv_savegamedir->string != NULL && sv_savegamedir->string[0] != NULL )
	{
		Com_sprintf (name, sizeof(name), "%s/save/current/%s.sav", sv_savegamedir->string, sv.name);
	}
	else
	{
		Com_sprintf (name, sizeof(name), "%s/save/current/%s.sav", FS_Gamedir(), sv.name);
	}
*/
	Com_sprintf (name, sizeof(name), "%s/save/current/%s.sav", FS_SaveGameDir(), sv.name);

	f = fopen (name, "rb");
	if (!f)
		return;		// no savegame
	fclose (f);

	SV_ClearWorld ();
	// get configstrings and areaportals
	SV_ReadLevelFile ();
	if (!sv.loadgame)
	{	// coming back to a level after being in a different
		// level, so run it for ten seconds
		server_state_t		previousState;		// PGM

		previousState = sv.state;				// PGM
		sv.state = ss_loading;					// PGM
		for (i=0 ; i<100 ; i++)
			ge->RunFrame ();

		sv.state = previousState;				// PGM
	}
}
/*
================
SV_SpawnServer
Change the server to a new map, taking all connected
clients along with it.
================
*/
void SV_SpawnServer (char *server, char *spawnpoint, server_state_t serverstate, qboolean attractloop, qboolean loadgame)
{
	int			i, episode;
	unsigned	checksum;
	char		*value;
	char		mapname [MAX_QPATH];

	if (attractloop)
		Cvar_Set ("paused", "0");

	Com_Printf ("------- Server Initialization -------\n");

	Com_DPrintf ("SpawnServer: %s\n",server);
	if (sv.demofile)
		fclose (sv.demofile);

	svs.spawncount++;		// any partially connected client will be
							// restarted
	sv.state = ss_dead;
	Com_SetServerState (sv.state);

	// wipe the entire per-level structure
	memset (&sv, 0, sizeof(sv));
	svs.realtime = 0;
	sv.loadgame = loadgame;
	sv.attractloop = attractloop;

	// save name for levels that don't set message
	strcpy (sv.configstrings[CS_NAME], server);

	SZ_Init (&sv.multicast, sv.multicast_buf, sizeof(sv.multicast_buf));
	
	strcpy (sv.name, server);

	// leave slots at start for clients only
	for (i=0 ; i<maxclients->value ; i++)
	{
		// needs to reconnect
		if (svs.clients[i].state > cs_connected)
			svs.clients[i].state = cs_connected;
		svs.clients[i].lastframe = -1;
	}

	sv.time = 1000;
	
	strcpy (sv.name, server);
	strcpy (sv.configstrings[CS_NAME], server);

	if (serverstate != ss_game)
	{
//		sv.models[1] = CM_LoadMap ("", false, &checksum);	// no real map
		// SCG[6/12/00]: we need to pass in the filename here because 
		// SCG[6/12/00]: CM_LoadMap needs it for demo maps....
		Com_sprintf (sv.configstrings[CS_MODELS+1],sizeof(sv.configstrings[CS_MODELS+1]),
			"demos/%s", server);
		sv.models[1] = CM_LoadMap (sv.configstrings[CS_MODELS+1], false, &checksum);	// no real map
	}
	else
	{
		Com_sprintf (sv.configstrings[CS_MODELS+1],sizeof(sv.configstrings[CS_MODELS+1]),
			"maps/%s.bsp", server);
		sv.models[1] = CM_LoadMap (sv.configstrings[CS_MODELS+1], false, &checksum);
	}
	Com_sprintf (sv.configstrings[CS_MAPCHECKSUM],sizeof(sv.configstrings[CS_MAPCHECKSUM]),
		"%i", checksum);

	///////////////////////////////////////////////////////////////////////////
	//	get episode number from epairs, set it for physics
	///////////////////////////////////////////////////////////////////////////

	value = 0;
	if( serverstate != ss_demo )
		value = CM_KeyValue (map_epairs, "episode");
	if( value )
		episode = atoi( value );

	if (episode < 1 || episode > 9)
	{
		Com_Printf ("Map has invalid episode number!!\n");
		episode = 0;
	}

	mapname [0] = (char)(episode + 48);
	mapname [1] = 0x00;
	Cvar_ForceSet ("sv_episode", mapname);

	// clear physics interaction links
	//
	SV_ClearWorld ();
	
	for (i=1 ; i< CM_NumInlineModels() ; i++)
	{
		Com_sprintf (sv.configstrings[CS_MODELS+1+i], sizeof(sv.configstrings[CS_MODELS+1+i]),
			"*%i", i);
		sv.models[i+1] = CM_InlineModel (sv.configstrings[CS_MODELS+1+i]);
	}
	//
	// spawn the rest of the entities on the map
	//	

	// precache and static commands can be issued during
	// map initialization
	sv.state = ss_loading;
	Com_SetServerState (sv.state);

	// SCG[11/3/99]: Register the world functions
	ge->RegisterWorldFuncs();

	// SCG[10/22/99]: what goes on here?
	// SCG[10/22/99]: does this need to be done here?
	ge->LevelLoad ();

	///////////////////////////////////////////////////////////////////////////
	// load and spawn all other entities
	///////////////////////////////////////////////////////////////////////////

	// SCG[11/3/99]: Load the nodes
	ge->LoadNodes( sv.name );

//NSS[11/11/99]:RE-Setting the Server time (giving a little offset)
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	ge->SetServerTime(0.75f);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

	//NSS[11/11/99]:Now spawn the entities
	ge->SpawnEntities ( sv.name, CM_EntityString(), spawnpoint, loadgame );

	//NSS[11/11/99]:Synchronize the Servertime with the level time BEFORE we spawn entities.
	// run two frames to allow everything to settle
	ge->RunFrame ();
	ge->RunFrame ();

	// all precaches are complete
	sv.state = serverstate;
	Com_SetServerState (sv.state);
	
	// create a baseline for more efficient communications
	SV_CreateBaseline ();

	// check for a savegame
	SV_CheckForSavegame ();

	// set serverinfo variable
	// does this work???
	Cvar_FullSet ("mapname", sv.name, CVAR_SERVERINFO | CVAR_NOSET);

	Com_Printf ("-------------------------------------\n");
}

///////////////////////////////////////////////////////////////////////////////
//	SV_PrepLevelChange
//
//	shut down the server and init the DLLs
///////////////////////////////////////////////////////////////////////////////
void SV_PrepLevelChange( qboolean unload_dlls )
{
//	char	idmaster[32];

	if (svs.initialized)
	{
		// cause any connected clients to reconnect
		SV_Shutdown( "Server restarted\n", true, unload_dlls );
	}
	else
	{
		// make sure the client is down
		CL_Drop ();
		SCR_BeginLoadingPlaque ();
	}
   
   // LOGGING, server and client contexts differ, if a log server is
   // specified before level startup, open a socket for server 
   // debugging as well
//   g_cvarLogServer = Cvar_Get("log_server", "", CVAR_ARCHIVE);
//   UDP_Log(6, "SV_InitGame() -Server Logging enabled");

   // get any latched variable changes (maxclients, etc)
	Cvar_GetLatchedVars ();

	svs.initialized = true;

	if (Cvar_VariableValue ("coop") && Cvar_VariableValue ("deathmatch"))
	{
		Com_Printf("Deathmatch and Coop both set, disabling Coop\n");
		Cvar_FullSet ("coop", "0",  CVAR_SERVERINFO | CVAR_LATCH);
	}

	// dedicated servers are can't be single player and are usually DM
	// so unless they explicity set coop, force it to deathmatch
	if (dedicated->value)
	{
		if (!Cvar_VariableValue ("coop"))
			Cvar_FullSet ("deathmatch", "1",  CVAR_SERVERINFO | CVAR_LATCH);
	}

	// init clients
	if (Cvar_VariableValue ("deathmatch"))
	{
		if (maxclients->value <= 1)
			Cvar_FullSet ("maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH);
		else if (maxclients->value > MAX_CLIENTS)
			Cvar_FullSet ("maxclients", va("%i", MAX_CLIENTS), CVAR_SERVERINFO | CVAR_LATCH);
	}
	else if (Cvar_VariableValue ("coop"))
	{
		if (maxclients->value <= 1 || maxclients->value > 4)
			Cvar_FullSet ("maxclients", "3", CVAR_SERVERINFO | CVAR_LATCH);
#ifdef ENABLE_COPY_PROTECTION
		if (!sv.attractloop && !dedicated->value)
			Sys_CopyProtect ();
#endif // end ENABLE_COPY_PROTECTION
	}
	else	// non-deathmatch, non-coop is one player
	{
		Cvar_FullSet ("maxclients", "1", CVAR_SERVERINFO | CVAR_LATCH);

	// currently do not check for the CD-ROM for internal use only
	// so debugging is possible
#ifdef ENABLE_COPY_PROTECTION
		if (!sv.attractloop)
			Sys_CopyProtect ();
#endif // end ENABLE_COPY_PROTECTION
	}
	
	svs.spawncount = rand();
//	svs.clients = (client_t *)X_Malloc (sizeof(client_t)*maxclients->value, MEM_TAG_ENTITY);
//	svs.num_client_entities = maxclients->value*UPDATE_BACKUP*64;
//	svs.client_entities = (entity_state_t *)X_Malloc (sizeof(entity_state_t)*svs.num_client_entities, MEM_TAG_ENTITY);
	svs.clients = (client_t *)Z_Malloc( sizeof( client_t ) * maxclients->value );
	memset( svs.clients, 0, sizeof( client_t ) * maxclients->value );

	svs.num_client_entities = maxclients->value * UPDATE_BACKUP * 64;
	svs.client_entities = (entity_state_t *)Z_Malloc( sizeof( entity_state_t ) * svs.num_client_entities );
	memset( svs.client_entities, 0, sizeof( entity_state_t ) * svs.num_client_entities );

	// init network stuff
	NET_Config ( (maxclients->value > 1) );
	
	// heartbeats will always be sent to the id master
//	svs.last_heartbeat = -99999;		// send immediately
//	Com_sprintf(idmaster, sizeof(idmaster), "192.246.40.37:%i", PORT_MASTER);
//	NET_StringToAdr (idmaster, &master_adr[0]);
}

///////////////////////////////////////////////////////////////////////////////
//	SV_InitGame
//
//	shut down the server and init the DLLs
///////////////////////////////////////////////////////////////////////////////
void SV_InitGame (void)
{
	int		i;
	edict_t	*ent;

	SV_PrepLevelChange( SV_UNLOAD_DLLS );

	SV_InitNewGame ();

	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = EDICT_NUM(i+1);
		ent->s.number = i+1;
		svs.clients[i].edict = ent;
		memset (&svs.clients[i].lastcmd, 0, sizeof(svs.clients[i].lastcmd));
	}

    //GAMESPY
	if (maxclients->value > 1 && dedicated->value != 0) //if we are multi-player
	{
        cvar_t	*ip, *port;
		ip = Cvar_Get ("ip", "localhost", CVAR_NOSET);
		port = Cvar_Get ("port", va("%i", PORT_SERVER), CVAR_NOSET);

		if (goa_init((int)port->value - 10, "daikatana",ip->string, NULL) == 0)
		{

			//set the callback functions
			goa_basic_callback = basic_callback;
			goa_info_callback = info_callback;
			goa_rules_callback = rules_callback;
			goa_players_callback = players_callback;
			//set the secret key, in a semi-obfuscated manner
			goa_secret_key[0] = 'f';
			goa_secret_key[1] = 'l';
			goa_secret_key[2] = '8';
			goa_secret_key[3] = 'a';
			goa_secret_key[4] = 'Y';
			goa_secret_key[5] = '7';
			goa_secret_key[6] = '\0';
		}

	}
}

///////////////////////////////////////////////////////////////////////////////
//	SV_InitLevel
//
//	shut down the server and load a new map.  Don't re-init the DLLs
///////////////////////////////////////////////////////////////////////////////

void SV_InitLevel (void)
{
	int		i;
	edict_t	*ent;

	SV_PrepLevelChange( SV_KEEP_DLLS );

	SV_InitNewMap ();

	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = EDICT_NUM(i+1);
		ent->s.number = i+1;
		svs.clients[i].edict = ent;
		memset (&svs.clients[i].lastcmd, 0, sizeof(svs.clients[i].lastcmd));
	}
}

///////////////////////////////////////////////////////////////////////////////
//	SV_Map
//
//  the full syntax is:
//  map [*]<map>$<startspot>+<nextserver>
//
//	command from the console or progs.
//	Map can also be a.cin, .pcx, or .dm2 file
//	Nextserver is used to allow a cinematic to play, then proceed to
//	another level:
//	map tram.cin+jail_e3
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	SV_Map
//
//	Nelno's map loading function.
//	
//	the "map" command stuffed into the console ALWAYS has these effects:
//		runs all DLL_ExitLevel, DLL_Exit functions
//	`	unloads the physics DLL
//		reloads the physics DLL
//		runs all DLL_ServerLoad functions
//		runs all DLL_LoadLevel functions
//
//	this has the effect of destroying any memory that a DLL has allocated.
//	the "map" command should be issued upon starting a new game only!!
//
//	to change levels during a game, issue the "changelevel" command, which
//	does not unload DLLs, but DOES call DLL_ExitLevel and DLL_LoadLevel
//	functions
//		
//
//  the full syntax is:
//  map [*]<map>$<startspot>+<nextserver>
//
//	command from the console or progs.
//	Map can also be a.cin, .pcx, or .dm2 file
//	Nextserver is used to allow a cinematic to play, then proceed to
//	another level:
//	map tram.cin+jail_e3
//
///////////////////////////////////////////////////////////////////////////////

extern qboolean no_draw_plaque;
void SV_Map (qboolean attractloop, char *levelstring, qboolean loadgame)
{
	char level[MAX_QPATH];
	char pluck_mapnum[32];
	char	*ch;
	int		l;
	char	spawnpoint[MAX_QPATH];

	// unpause the game if it is paused ... or else bad things happen
	if (!dedicated->value && Cvar_VariableValue("paused"))
		Cvar_Set("paused","0");

	sv.loadgame = loadgame;
	sv.attractloop = attractloop;

	if (sv.state == ss_dead && !sv.loadgame)
	{
		SV_InitGame ();	// the game is just starting
	}
	else
	{
		SV_ShutdownDLLs();
	}
	//	copy level name into level 
	strcpy (level, levelstring);

	// SCG[10/7/99]: Someone removed this, I put it back in...
	ch = strstr( level, "+");
	if (ch)
	{
		*ch = 0;
			Cvar_Set ("nextserver", va("gamemap \"%s\"", ch+1));
	}
	else
		Cvar_Set ("nextserver", "");

	ch = strstr(level, "$");
	if (ch)
	{
		*ch = 0;
		strcpy (spawnpoint, ch+1);
	}
	else
		spawnpoint[0] = 0;

	if (level[0] == '*')
		strcpy (level, level+1);

	if (strlen(level) >= 5)  // standard map name?  (ie: e1m2)
	{
		strncpy(pluck_mapnum,&levelstring[4],1);
		Cvar_Set("sv_mapnum", pluck_mapnum);
	}
	else
		Cvar_Set("sv_mapnum", "0");   // not an episode map

	l = strlen(level);
	if (l > 4 && !stricmp (level + l - 4, ".dem") )
	{
		//	play a demo
		SCR_BeginLoadingPlaque ();			// for local system
		if (no_draw_plaque)
			SV_BroadcastCommand ("changing noplaque\n");
		else
			SV_BroadcastCommand ("changing\n");
		SV_SpawnServer (level, spawnpoint, ss_demo, attractloop, loadgame);
	}
	else
	{
		//	Load a map
		SCR_BeginLoadingPlaque ();			// for local system
		if (no_draw_plaque)
			SV_BroadcastCommand ("changing noplaque\n");
		else
			SV_BroadcastCommand ("changing\n");
		SV_SendClientMessages ();
		SV_SpawnServer (level, spawnpoint, ss_game, attractloop, loadgame);
		Cbuf_CopyToDefer ();
	}
	
	SV_BroadcastCommand ("reconnect\n");
}

///////////////////////////////////////////////////////////////////////////////
//	SV_ChangeLevel
//
//	Nelno's non-destrucitve map loading function.  For switching levels during
//	a game
//	
//	the "changelevel" command stuffed into the console ALWAYS has these effects:
//		runs all DLL_ExitLevel
//		runs all DLL_LoadLevel functions
//
//	does not unload DLLs, but DOES call DLL_ExitLevel and DLL_LoadLevel
//	functions
//		
//
//  the full syntax is:
//  map [*]<map>$<startspot>+<nextserver>
//
//	command from the console or progs.
//	Map can also be a.cin, .pcx, or .dm2 file
//	Nextserver is used to allow a cinematic to play, then proceed to
//	another level:
//	map tram.cin+jail_e3
//
///////////////////////////////////////////////////////////////////////////////

void SV_ChangeLevel (qboolean attractloop, char *levelstring, qboolean loadgame)
{

	// JAR 19990305 - Extreme fasthack for MPlayer demo... ent things are trashed with just a changelevel!
	SV_Map(attractloop, levelstring, loadgame);
/*
	char	level[MAX_QPATH];
//	char	*ch;
//	int		l;
//	char	spawnpoint[MAX_QPATH];

	//	Q2FIXME:	Huh?!?!?
	sv.loadgame = loadgame;
	sv.attractloop = attractloop;

	SV_InitLevel ();	// start a new level

	//	copy level name into level for some odd reason
	strcpy (level, levelstring);

	SCR_BeginLoadingPlaque ();			// for local system
	SV_BroadcastCommand ("changing\n");
	SV_SendClientMessages ();
	SV_SpawnServer (level, NULL, ss_game, attractloop, loadgame);
	
	//	Quake 314
	Cbuf_CopyToDefer ();

	SV_BroadcastCommand ("reconnect\n");
	*/
}

///////////////////////////////////////////////////////////////////////////////
//	SV_Unload
//
//	ends the current game and forces the DLLs to be unloaded -- for debugging
///////////////////////////////////////////////////////////////////////////////

void	SV_Unload (void)
{
	if (svs.initialized)
	{
		// cause any connected clients to reconnect
		SV_Shutdown ("DLLs unloaded.\n", false, SV_UNLOAD_DLLS);
	}
	else
		Com_Printf ("No DLLs loaded.\n");

}
