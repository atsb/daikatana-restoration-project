
#include "server.h"
#include "daikatana.h"

#include "l__language.h"
/*
===============================================================================
OPERATOR CONSOLE ONLY COMMANDS
These commands can only be entered from stdin or by a remote operator datagram
===============================================================================
*/
/*
====================
SV_SetMaster_f
Specify a list of master servers
====================
*/
void SV_SetMaster_f (void)
{
	int		i, slot;

	// only dedicated servers send heartbeats
	if (!dedicated->value)
	{
		Com_Printf ("Only dedicated servers use masters.\n");
		return;
	}
	// make sure the server is listed public
	Cvar_Set ("public", "1");

	for (i=1 ; i<MAX_MASTERS ; i++)
		memset (&master_adr[i], 0, sizeof(master_adr[i]));

	slot = 1;		// slot 0 will always contain the id master
	for (i=1 ; i<GetArgc() ; i++)
	{
		if (slot == MAX_MASTERS)
			break;
		if (!NET_StringToAdr (GetArgv(i), &master_adr[i]))
		{
			Com_Printf ("Bad address: %s\n", GetArgv(i));
			continue;
		}
		if (master_adr[slot].port == 0)
			master_adr[slot].port = BigShort (PORT_MASTER);
		Com_Printf ("Master server at %s\n", NET_AdrToString (master_adr[slot]));
		Com_Printf ("Sending a ping.\n");
		Netchan_OutOfBandPrint (NS_SERVER, master_adr[slot], "ping");
		slot++;
	}
	svs.last_heartbeat = -9999999;
}
/*
==================
SV_SetPlayer
Sets sv_client and sv_player to the player with idnum Cmd_Argv(1)
==================
*/
qboolean SV_SetPlayer (void)
{
	client_t	*cl;
	int			i;
	int			idnum;
	char		*s;

	if (GetArgc () < 2)
		return false;

	s = GetArgv(1);
	// numeric values are just slot numbers
	if (s[0] >= '0' && s[0] <= '9')
	{
		idnum = atoi(GetArgv(1));
		if (idnum < 0 || idnum >= maxclients->value)
		{
			Com_Printf ("Bad client slot: %i\n", idnum);
			return false;
		}
		sv_client = &svs.clients[idnum];
		sv_player = sv_client->edict;
		if (!sv_client->state)
		{
			Com_Printf ("Client %i is not active\n", idnum);
			return false;
		}
		return true;
	}
	// check for a name match
	for (i=0,cl=svs.clients ; i<maxclients->value; i++,cl++)
	{
		if (!cl->state)
			continue;
		if (!strcmp(cl->name, s))
		{
			sv_client = cl;
			sv_player = sv_client->edict;
			return true;
		}
	}
	Com_Printf ("Userid %s is not on the server\n", s);
	return false;
}
/*
===============================================================================
SAVEGAME FILES
===============================================================================
*/
/*
=====================
SV_WipeSavegame
Delete save/<XXX>/
=====================
*/
void SV_WipeSavegame (char *savename)
{
	char	name[MAX_OSPATH];
	char	*s;
/*
	if( sv_savegamedir->string != NULL && sv_savegamedir->string[0] != NULL )
	{
		Com_sprintf (name, sizeof(name), "%s/save/%s/server.ssv", sv_savegamedir->string, savename);
		remove (name);
		Com_sprintf (name, sizeof(name), "%s/save/%s/game.ssv", sv_savegamedir->string, savename);
		remove (name);

		// cek[12-12-99] remove the screen shot and header, too
		Com_sprintf (name, sizeof(name), "%s/save/%s/info.hdr", sv_savegamedir->string, savename);
		remove (name);
		Com_sprintf (name, sizeof(name), "%s/save/%s/shot.tga", sv_savegamedir->string, savename);
		remove (name);

		Com_sprintf (name, sizeof(name), "%s/save/%s/*.sav", sv_savegamedir->string, savename);
	}
	else
	{
		Com_sprintf (name, sizeof(name), "%s/save/%s/server.ssv", FS_Gamedir (), savename);
		remove (name);
		Com_sprintf (name, sizeof(name), "%s/save/%s/game.ssv", FS_Gamedir (), savename);
		remove (name);

		// cek[12-12-99] remove the screen shot and header, too
		Com_sprintf (name, sizeof(name), "%s/save/%s/info.hdr", FS_Gamedir (), savename);
		remove (name);
		Com_sprintf (name, sizeof(name), "%s/save/%s/shot.tga", FS_Gamedir (), savename);
		remove (name);

		Com_sprintf (name, sizeof(name), "%s/save/%s/*.sav", FS_Gamedir (), savename);
	}
*/
	Com_sprintf (name, sizeof(name), "%s/save/%s/server.ssv", FS_SaveGameDir (), savename);
	remove (name);
	Com_sprintf (name, sizeof(name), "%s/save/%s/game.ssv", FS_SaveGameDir (), savename);
	remove (name);

	// cek[12-12-99] remove the screen shot and header, too
	Com_sprintf (name, sizeof(name), "%s/save/%s/info.hdr", FS_SaveGameDir (), savename);
	remove (name);
	Com_sprintf (name, sizeof(name), "%s/save/%s/shot.tga", FS_SaveGameDir (), savename);
	remove (name);

	Com_sprintf (name, sizeof(name), "%s/save/%s/*.sav", FS_SaveGameDir (), savename);

	s = Sys_FindFirst( name, 0, 0 );
	while (s)
	{
		remove (s);
		s = Sys_FindNext( 0, 0 );
	}
	Sys_FindClose ();
/*
	if( sv_savegamedir->string != NULL && sv_savegamedir->string[0] != NULL )
	{
		Com_sprintf (name, sizeof(name), "%s/save/%s/*.sv2", sv_savegamedir->string, savename);
	}
	else
	{
		Com_sprintf (name, sizeof(name), "%s/save/%s/*.sv2", FS_Gamedir (), savename);
	}
*/
	Com_sprintf (name, sizeof(name), "%s/save/%s/*.sv2", FS_SaveGameDir (), savename);
	s = Sys_FindFirst(name, 0, 0 );
	while (s)
	{
		remove (s);
		s = Sys_FindNext( 0, 0 );
	}
	Sys_FindClose ();
}
/*
================
CopyFile
================
*/
void CopyFile (char *src, char *dst)
{
	FILE	*f1, *f2;
	int		l;
	byte	buffer[65536];
	Com_DPrintf ("CopyFile (%s, %s)\n", src, dst);
	f1 = fopen (src, "rb");
	if (!f1)
		return;
	f2 = fopen (dst, "wb");
	if (!f2)
	{
		fclose (f1);
		return;
	}
	while (1)
	{
		l = fread (buffer, 1, sizeof(buffer), f1);
		if (!l)
			break;
		fwrite (buffer, 1, l, f2);
	}
	fclose (f1);
	fclose (f2);
}
/*
================
SV_CopySaveGame
================
*/
void SV_CopySaveGame (char *src, char *dst, qboolean bLoading)
{
	char	name[MAX_OSPATH], name2[MAX_OSPATH];
	int		l, len;
	char	*found;

	Com_DPrintf("SV_CopySaveGame(%s, %s)\n", src, dst);

	SV_WipeSavegame (dst);

	// copy the savegame over
	Com_sprintf (name, sizeof(name), "%s/save/%s/server.ssv", FS_SaveGameDir(), src);
	Com_sprintf (name2, sizeof(name2), "%s/save/%s/server.ssv", FS_SaveGameDir(), dst);
	FS_CreatePath (name2);
	CopyFile (name, name2);

	Com_sprintf (name, sizeof(name), "%s/save/%s/game.ssv", FS_SaveGameDir(), src);
	Com_sprintf (name2, sizeof(name2), "%s/save/%s/game.ssv", FS_SaveGameDir(), dst);
	CopyFile (name, name2);

	// copy the header
	Com_sprintf (name, sizeof(name), "%s/save/%s/info.hdr", FS_SaveGameDir(), src);
	Com_sprintf (name2, sizeof(name2), "%s/save/%s/info.hdr", FS_SaveGameDir(), dst);
	CopyFile(name,name2);

	// copy the shot
	Com_sprintf (name, sizeof(name), "%s/save/%s/shot.tga", FS_SaveGameDir(), src);
	Com_sprintf (name2, sizeof(name2), "%s/save/%s/shot.tga", FS_SaveGameDir(), dst);
	CopyFile(name,name2);

	Com_sprintf (name, sizeof(name), "%s/save/%s/", FS_SaveGameDir(), src);
	len = strlen(name);
	Com_sprintf (name, sizeof(name), "%s/save/%s/*.sav", FS_SaveGameDir(), src);

	found = Sys_FindFirst(name, 0, 0 );
	while (found)
	{
/*
		if ( !bLoading && (sv_episode->value >= 1) && (sv_episode->value <= 4) )	// the episode # is not known when loading.  rely on the game being copied right.
		{
			char *str = found+len;
			if ( (str[5] == '.') && (str[0] == 'e') && (str[2] == 'm') && (str[1] >= '1') && (str[1] <= '9') )	// validate the format
			{
				int ep = (int)str[1] - '1' + 1;

				if (ep != (int)(sv_episode->value) )
				{
					found = Sys_FindNext( 0, 0 );
					continue;
				}
			}
		}
*/
		strcpy (name+len, found+len);

		Com_sprintf (name2, sizeof(name2), "%s/save/%s/%s", FS_SaveGameDir(), dst, found+len);

		CopyFile (name, name2);

		// change sav to sv2
		l = strlen(name);
		strcpy (name+l-3, "sv2");
		l = strlen(name2);
		strcpy (name2+l-3, "sv2");
		CopyFile (name, name2);

		found = Sys_FindNext( 0, 0 );
	}
	Sys_FindClose ();
}

void SV_WriteHeaderFile(char *comment, qboolean autoSave)
{
	char	name[MAX_OSPATH];

	Com_sprintf (name, sizeof(name), "%s/save/current/info.hdr", FS_SaveGameDir());
	ge->WriteHeader( name, comment, autoSave );

	// cek[12-10-99] copy the screenshot to current, also
	if (!autoSave)
	{
		char name2[MAX_OSPATH];
		Com_sprintf (name, sizeof(name), "%s/shot.tga", FS_SaveGameDir());
		Com_sprintf (name2, sizeof(name2), "%s/save/current/shot.tga", FS_SaveGameDir());

		CopyFile (name, name2);
	}
}

/*
==============
SV_WriteLevelFile
==============
*/
void SV_WriteLevelFile (void)
{
	char	name[MAX_OSPATH];
	FILE	*f;

	Com_sprintf (name, sizeof(name), "%s/save/current/%s.sv2", FS_SaveGameDir(), sv.name);

	f = fopen(name, "wb");
	if (!f)
	{
		Com_Printf ("Failed to open %s\n", name);
		return;
	}
	fwrite (sv.configstrings, sizeof(sv.configstrings), 1, f);
	CM_WritePortalState (f);
	fclose (f);

	Com_sprintf (name, sizeof(name), "%s/save/current/%s.sav", FS_SaveGameDir(), sv.name);

	ge->WriteLevel (name);
}
/*
==============
SV_ReadLevelFile
==============
*/
void SV_ReadLevelFile (void)
{
	char	name[MAX_OSPATH];
	FILE	*f;
/*
	if( sv_savegamedir->string != NULL && sv_savegamedir->string[0] != NULL )
	{
		Com_sprintf (name, sizeof(name), "%s/save/current/%s.sv2", sv_savegamedir->string, sv.name);
	}
	else
	{
		Com_sprintf (name, sizeof(name), "%s/save/current/%s.sv2", FS_Gamedir(), sv.name);
	}
*/
	Com_sprintf (name, sizeof(name), "%s/save/current/%s.sv2", FS_SaveGameDir(), sv.name);

	f = fopen(name, "rb");
	if (!f)
	{
		Com_Printf ("Failed to open %s\n", name);
		return;
	}
	FS_Read (sv.configstrings, sizeof(sv.configstrings), f);
	CM_ReadPortalState (f);
	fclose (f);
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

	ge->ReadLevel (name);
}
/*
==============
SV_WriteServerFile
==============
*/
void SV_WriteServerFile (qboolean autosave)
{
	FILE	*f;
	cvar_t	*var;
	char	name[MAX_OSPATH], string[128];
	char	comment[32];
	time_t	aclock;
	struct tm	*newtime;

	Com_DPrintf("SV_WriteServerFile(%s)\n", autosave ? "true" : "false");
/*
	if( sv_savegamedir->string != NULL && sv_savegamedir->string[0] != NULL )
	{
		Com_sprintf (name, sizeof(name), "%s/save/current/server.ssv", sv_savegamedir->string);
	}
	else
	{
		Com_sprintf (name, sizeof(name), "%s/save/current/server.ssv", FS_Gamedir());
	}
*/
	Com_sprintf (name, sizeof(name), "%s/save/current/server.ssv", FS_SaveGameDir());

	f = fopen (name, "wb");
	if (!f)
	{
		Com_Printf ("Couldn't write %s\n", name);
		return;
	}
	// write the comment field
	memset (comment, 0, sizeof(comment));

	if (!autosave)
	{
		time (&aclock);
		newtime = localtime (&aclock);
		Com_sprintf (comment,sizeof(comment), "%2i:%i%i %2i/%2i  ", newtime->tm_hour
			, newtime->tm_min/10, newtime->tm_min%10,
			newtime->tm_mon+1, newtime->tm_mday);
		strncat (comment, sv.configstrings[CS_NAME], sizeof(comment)-1-strlen(comment) );
	}
	else
	{	// autosaved
		Com_sprintf (comment, sizeof(comment), "ENTERING %s", sv.configstrings[CS_NAME]);
	}

	fwrite (comment, 1, sizeof(comment), f);

	// write the mapcmd
	fwrite (svs.mapcmd, 1, sizeof(svs.mapcmd), f);

	// write all CVAR_LATCH cvars
	// these will be things like coop, skill, deathmatch, etc
	for (var = cvar_vars ; var ; var=var->next)
	{
		if (!(var->flags & CVAR_LATCH))
			continue;
		if (strlen(var->name) >= sizeof(name)-1
			|| strlen(var->string) >= sizeof(string)-1)
		{
			Com_Printf ("Cvar too long: %s = %s\n", var->name, var->string);
			continue;
		}
		memset (name, 0, sizeof(name));
		memset (string, 0, sizeof(string));
		strcpy (name, var->name);
		strcpy (string, var->string);
		fwrite (name, 1, sizeof(name), f);
		fwrite (string, 1, sizeof(string), f);
	}

	fclose (f);
/*
	// write game state
	if( sv_savegamedir->string != NULL && sv_savegamedir->string[0] != NULL )
	{
		Com_sprintf (name, sizeof(name), "%s/save/current/game.ssv", sv_savegamedir->string);
	}
	else
	{
		Com_sprintf (name, sizeof(name), "%s/save/current/game.ssv", FS_Gamedir());
	}
*/
	Com_sprintf (name, sizeof(name), "%s/save/current/game.ssv", FS_SaveGameDir());

	ge->WriteGame (name, (char) autosave);
}
/*
==============
SV_ReadServerFile

HUH?:	Load a saved game???
==============
*/
void SV_ReadServerFile (void)
{
	FILE	*f;
	char	name[MAX_OSPATH], string[128];
	char	comment[32];
	char	mapcmd[MAX_TOKEN_CHARS];

	Com_DPrintf("SV_ReadServerFile()\n");
/*
	if( sv_savegamedir->string != NULL && sv_savegamedir->string[0] != NULL )
	{
		Com_sprintf (name, sizeof(name), "%s/save/current/server.ssv", sv_savegamedir->string);
	}
	else
	{
		Com_sprintf (name, sizeof(name), "%s/save/current/server.ssv", FS_Gamedir());
	}
*/
	Com_sprintf (name, sizeof(name), "%s/save/current/server.ssv", FS_SaveGameDir());

	f = fopen (name, "rb");
	if (!f)
	{
		Com_Printf ("Couldn't read %s\n", name);
		return;
	}
	// read the comment field
	FS_Read (comment, sizeof(comment), f);

	// read the mapcmd
	FS_Read (mapcmd, sizeof(mapcmd), f);

	// read all CVAR_LATCH cvars
	// these will be things like coop, skill, deathmatch, etc
	while (1)
	{
		if (!fread (name, 1, sizeof(name), f))
			break;
		FS_Read (string, sizeof(string), f);
		Com_DPrintf ("Set %s = %s\n", name, string);
		Cvar_ForceSet (name, string);
	}

	fclose (f);

	// start a new game fresh with new cvars
	SV_InitGame ();

	strcpy (svs.mapcmd, mapcmd);
	// read game state
/*
	if( sv_savegamedir->string != NULL && sv_savegamedir->string[0] != NULL )
	{
		Com_sprintf (name, sizeof(name), "%s/save/current/game.ssv", sv_savegamedir->string);
	}
	else
	{
		Com_sprintf (name, sizeof(name), "%s/save/current/game.ssv", FS_Gamedir());
	}
*/
	Com_sprintf (name, sizeof(name), "%s/save/current/game.ssv", FS_SaveGameDir());

	ge->ReadGame (name);
}
//=========================================================
/*
==================
SV_DemoMap_f
Puts the server in demo mode on a specific map/cinematic
==================
*/
void SV_DemoMap_f (void)
{
//	Nelno:	don't demo maps during development
//	if (developer->value == 0)
		SV_Map (true, GetArgv(1), false );
}

// just took some stuff from the console command gamemap_f and separated it out so 
// it could be used elsewhere.
void SV_GameMap(char *map)
{
	int			i;
	client_t	*cl;
	qboolean	*savedInuse;

	FS_CreatePath (va("%s/save/current/", FS_SaveGameDir()));

	// check for clearing the current savegame
	if (map[0] == '*')
	{
		// wipe all the *.sav files
		SV_WipeSavegame ("current");
	}
	else
	{	// save the map just exited
		if (sv.state == ss_game)
		{
			// clear all the client inuse flags before saving so that
			// when the level is re-entered, the clients will spawn
			// at spawn points instead of occupying body shells
			savedInuse = (qboolean*)X_Malloc(maxclients->value * sizeof(qboolean), MEM_TAG_MISC);
			for (i=0,cl=svs.clients ; i<maxclients->value; i++,cl++)
			{
				savedInuse[i] = cl->edict->inuse;
				cl->edict->inuse = false;
			}

			SV_WriteLevelFile ();

			// we must restore these for clients to transfer over correctly
			for (i=0,cl=svs.clients ; i<maxclients->value; i++,cl++)
				cl->edict->inuse = savedInuse[i];
			X_Free (savedInuse);
		}
	}
	// start up the next map
	SV_Map (false, map, false );
	
	// archive server state
	strncpy (svs.mapcmd, map, sizeof(svs.mapcmd)-1);

	// copy off the level to the autosave slot
	if (!dedicated->value && !Cvar_VariableValue("deathmatch"))
	{
		SV_WriteServerFile (true);
		SV_WriteHeaderFile(NULL, true);
		SV_CopySaveGame ("current", "save0", false);
		SV_CopySaveGame ("current", "save1", false);
	}
}
/*
==================
SV_GameMap_f
Saves the state of the map just being exited and goes to a new map.
If the initial character of the map string is '*', the next map is
in a new unit, so the current savegame directory is cleared of
map files.
Example:
*inter.cin+jail
Clears the archived maps, plays the inter.cin cinematic, then
goes to map jail.bsp.
==================
*/
qboolean no_draw_plaque;
void SV_GameMap_f (void)
{
	no_draw_plaque = false;
	switch(GetArgc())
	{
	case 3:
		if (!stricmp(GetArgv(0),"changelevel"))
		{
			if (!stricmp(GetArgv(2),"noplaque"))
				no_draw_plaque = true;
		}
		break;
	case 2:	
		break;
	default:
		Com_Printf ("USAGE: gamemap <map>\n");
		return;
	};

	SV_GameMap(GetArgv(1));
}

/*
==================
SV_Map_f
Goes directly to a given map without any savegame archiving.
For development work
==================
*/

void SV_KillServer_f (void);
void SV_Map_f (void)
{
	char	*map;
	char	expanded[MAX_QPATH];

//	SV_BroadcastCommand ("stoprendering\n");
//	if (sv.state == ss_game)
//		SV_KillServer_f();

	// if not a pcx, demo, or cinematic, check to make sure the level exists
	map = GetArgv(1);
	if (!strstr (map, "."))
	{
		Com_sprintf (expanded, sizeof(expanded), "maps/%s.bsp", map);
		if (FS_LoadFile (expanded, NULL) == -1)
		{
			//Com_Printf ("Can't find %s\n", expanded);
			Com_Error( ERR_DROP,  "Can't find %s\n", expanded );
			return;
		}
	}

	sv.state = ss_dead;		// don't save current level when changing
	SV_WipeSavegame("current");
	SV_GameMap_f ();
}

///////////////////////////////////////////////////////////////////////////////
//	SV_LevelExists
//	
///////////////////////////////////////////////////////////////////////////////

int	SV_LevelExists (char *map)
{
	char	expanded[MAX_QPATH];

	//	make sure the level exists
	if (!strstr (map, "."))
	{
		Com_sprintf (expanded, sizeof(expanded), "maps/%s.bsp", map);

		if (FS_LoadFile (expanded, NULL) == -1)
		{
//			Com_Printf ("Can't find map %s\n", expanded);
			Com_Error( ERR_DROP, "Can't find map %s\n", expanded );
			return	false;
		}
	}

	return	true;
}

///////////////////////////////////////////////////////////////////////////////
//	SV_Changelevel_f
//
//	starts a new game on specified map, unloads and then reloads DLLs
///////////////////////////////////////////////////////////////////////////////

void SV_Changelevel_f (void)
{
	if (!SV_LevelExists (GetArgv (1)))
		return;

	if (sv.state == ss_dead)
	{
		Com_Printf ("Game must be running to \"changelevel\".\n");
		return;
	}

	SV_GameMap_f();
// SCG[12/2/99]: 	SV_ChangeLevel (false, GetArgv (1), false);
}

///////////////////////////////////////////////////////////////////////////////
//	SV_Unload_f
//
//	forces game to shut down and DLLs to unload
///////////////////////////////////////////////////////////////////////////////

void SV_Unload_f (void)
{
	SV_Unload ();
}

///////////////////////////////////////////////////////////////////////////////
//	SV_KillServer_f
//
//	ends the game in progress and unloads all DLLs, returning the user to
//	the console
///////////////////////////////////////////////////////////////////////////////

/*
=====================================================================
  SAVEGAMES
=====================================================================
*/
/*
==============
SV_Loadgame_f
==============
*/
void SV_Loadgame_f (void)
{
	char	name[MAX_OSPATH];
	FILE	*f;
	char	*dir;
	if (GetArgc() != 2)
	{
		char buf[32];
// SCG[1/16/00]: 		sprintf(buf,"menu_main_force %d\n", (int)DKMB_LOADGAME);
		Com_sprintf(buf,sizeof(buf),"menu_main_force %d\n", (int)DKMB_LOADGAME);
		Cbuf_AddText (buf); 
//		Com_Printf ("USAGE: loadgame <directory>\n");
		return;
	}
	Com_DPrintf ("Loading game...\n");

	dir = GetArgv(1);
	if (strstr (dir, "..") || strstr (dir, "/") || strstr (dir, "\\") )
	{
		Com_Printf ("Bad savedir.\n");
	}
/*
	// make sure the server.ssv file exists
	if( sv_savegamedir->string != NULL && sv_savegamedir->string[0] != NULL )
	{
		Com_sprintf (name, sizeof(name), "%s/save/%s/server.ssv", sv_savegamedir->string, GetArgv(1));
	}
	else
	{
		Com_sprintf (name, sizeof(name), "%s/save/%s/server.ssv", FS_Gamedir(), GetArgv(1));
	}
*/
	Com_sprintf (name, sizeof(name), "%s/save/%s/server.ssv", FS_SaveGameDir(), GetArgv(1));

	f = fopen (name, "rb");
	if (!f)
	{
		Com_Printf ("No such savegame: %s\n", name);
		return;
	}
	fclose (f);

	SV_CopySaveGame (GetArgv(1), "current", true);

	SV_ReadServerFile (); 

	// go to the map
	sv.state = ss_dead;		// don't save current level when changing
	if( svs.mapcmd[0] == NULL )
	{
		Com_Error( ERR_DROP, "mapname is NULL" );
	}

	SV_Map (false, svs.mapcmd, true);
}
/*
==============
SV_Savegame_f
==============
*/
void PF_centerprint (edict_t *ent, float msg_time, char *fmt, ...);
void SV_SendToLoaclHost(const char *s);
byte  sv_savegem_count = 0;
bool SCR_ScreenDisabled();
client_t *GetLocalClient();

bool SV_CanSave (client_t *cl, bool msg)
{
	// NULL passed into cl means to get the local client...
	if (!cl)
	{
		cl = GetLocalClient();
	}

	if ( !cl || !ge )
	{
		if (msg)
			Com_Printf (TONGUE_SAVE_ERR_LOCAL);
		return false;
	}	
		
	if ( (sv.state != ss_game) )
	{
		if (msg)
			Com_Printf (TONGUE_SAVE_ERR_INGAME);
		return false;
	}
	if (Cvar_VariableValue("deathmatch"))
	{
		if (msg)
			Com_Printf (TONGUE_SAVE_ERR_DM);
		return false;
	}
	if(cl->edict->client->ps.rdflags & RDF_LETTERBOX)
	{
		if (msg)
			Com_Printf (TONGUE_SAVE_ERR_CINE);
		return false;
	}

	return (cl->edict, msg);
}

short SV_DoSaveGame(char *dir, char *comment)
{
/*
	if ( (sv.state != ss_game))
	{
		Com_Printf ("You must be in a game to save.\n");
		return 0;
	}
	if (Cvar_VariableValue("deathmatch"))
	{
		Com_Printf ("Can't savegame in a deathmatch\n");
		return 0;
	}
	if(svs.clients[0].edict->client->ps.rdflags & RDF_LETTERBOX)
	{
		Com_Printf ("Can't savegame in a cinematic.\n");
		return 0;
	}
	if (maxclients->value == 1 && svs.clients[0].edict->client->ps.stats[STAT_HEALTH] <= 0)
	{
		Com_Printf ("\nCan't savegame while dead!\n");
		return 0;
	}
*/
	if (!SV_CanSave(NULL, true))
		return false;

	if (Cvar_VariableValue("unlimited_saves") == 0.0)
	{
		sv_savegem_count--;	// so the count is updated fast.

		// now, actually use it.  It'll send down a message with the correct count.
		SV_SendToLoaclHost("item_savegem_use");
	}

	Com_Printf ("Saving game...\n");

	// SCG[12/6/99]: Contains player and sidekick stats for savegame menu
	SV_WriteHeaderFile(comment, false);

	// archive current level
	// when the level is reloaded, they will be shells awaiting
	// a connecting client
	SV_WriteLevelFile ();

	// save server state
	SV_WriteServerFile (false);

	// copy it off
	SV_CopySaveGame ("current", dir, false);
	SV_CopySaveGame ("current", "save1", false);

	Com_Printf ("Done.\n");

	return 1;
}

void SV_Savegame_f (void)
{
/*
	if (cl_savegem_count < 1)//svs.clients[0].edict->client->ps.stats[STAT_SAVE_GEMS] < 1)
	{
		if (sv.state != ss_game)
			Com_Printf ("%s\n",tongue[T_NO_SAVEGEMS]);
		else
			PF_centerprint(svs.clients[0].edict,2.0,tongue[T_NO_SAVEGEMS]);
		return;
	}
	if ( (sv.state != ss_game) )
	{
		Com_Printf ("You must be in a game to save.\n");
		return;
	}
	if (Cvar_VariableValue("deathmatch"))
	{
		Com_Printf ("Can't savegame in a deathmatch\n");
		return;
	}
	if(svs.clients[0].edict->client->ps.rdflags & RDF_LETTERBOX)
	{
		Com_Printf ("Can't savegame in a cinematic.\n");
		return;
	}
	if (maxclients->value == 1 && svs.clients[0].edict->client->ps.stats[STAT_HEALTH] <= 0)
	{
		Com_Printf ("\nCan't savegame while dead!\n");
		return;
	}
*/

	if (!SV_CanSave(NULL, true))
		return;

	if (GetArgc() != 2)
	{
		char buf[32];
		Com_sprintf(buf,sizeof(buf),"menu_main_force %d\n", (int)DKMB_SAVEGAME);
		Cbuf_AddText (buf); 
		return;
	}
	if (!strcmp (GetArgv(1), "current"))
	{
		Com_Printf ("Can't save to 'current'\n");
		return;
	}

	char dir[MAX_OSPATH];
	strcpy(dir,GetArgv(1));
	if (strstr (dir, "..") || strstr (dir, "/") || strstr (dir, "\\") )
	{
		Com_Printf ("Bad savedir.\n");
		return;
	}

	SV_DoSaveGame(dir,NULL);
}

void SV_DoAutoSave()
{
	Com_DPrintf("SV_AutoSave()\n");

	SV_WriteHeaderFile(NULL, true);
	SV_WriteLevelFile ();
	SV_WriteServerFile (false);

	// copy it off to the autosave
	SV_CopySaveGame ("current", "save0", false);
	SV_CopySaveGame ("current", "save1", false);
}


//===============================================================
/*
==================
SV_Kick_f
Kick a user off of the server
==================
*/
void SV_Kick_f (void)
{
	if (!svs.initialized)
	{
		Com_Printf ("No server running.\n");
		return;
	}

	if (GetArgc() != 2)
	{
		Com_Printf ("Usage: kick <userid>\n");
		return;
	}

	if (!SV_SetPlayer ())
		return;
	SV_BroadcastPrintf (PRINT_HIGH, "%s was kicked\n", sv_client->name);
	// print directly, because the dropped client won't get the
	// SV_BroadcastPrintf message
	SV_ClientPrintf (sv_client, PRINT_HIGH, "You were kicked from the game\n");
	SV_DropClient (sv_client);
	sv_client->lastmessage = svs.realtime;	// min case there is a funny zombie
}
/*
================
SV_Status_f
================
*/
void SV_Status_f (void)
{
	int			i, j, l;
	client_t	*cl;
	char		*s;
	int			ping;

	if (!svs.clients)
	{
		Com_Printf ("No server running.\n");
		return;
	}
	Com_Printf ("map              : %s\n", sv.name);
	Com_Printf ("num score ping name            lastmsg address               qport \n");
	Com_Printf ("--- ----- ---- --------------- ------- --------------------- ------\n");
	for (i=0,cl=svs.clients ; i<maxclients->value; i++,cl++)
	{
		if (!cl->state)
			continue;
		Com_Printf ("%3i ", i);
		Com_Printf (" %4i ", cl->edict->record.frags);		

		if (cl->state == cs_connected)
			Com_Printf ("CNCT ");
		else if (cl->state == cs_zombie)
			Com_Printf ("ZMBI ");
		else
		{
			ping = cl->ping < 9999 ? cl->ping : 9999;
			Com_Printf ("%4i ", ping);
		}

		Com_Printf ("%s", cl->name);
		l = 16 - strlen(cl->name);
		for (j=0 ; j<l ; j++)
			Com_Printf (" ");

		Com_Printf ("%7i ", svs.realtime - cl->lastmessage );

		s = NET_AdrToString ( cl->netchan.remote_address);
		Com_Printf ("%s", s);
		l = 22 - strlen(s);
		for (j=0 ; j<l ; j++)
			Com_Printf (" ");
		
		Com_Printf ("%5i", cl->netchan.qport);
		Com_Printf ("\n");
	}
	Com_Printf ("\n");
}
/*
==================
SV_ConSay_f
==================
*/
void SV_ConSay_f(void)
{
	client_t *client;
	int		j;
	char	*p;
	char	text[1024];
	if (GetArgc () < 2)
		return;
	strcpy (text, "console: ");
	p = GetArgs();
	if (*p == '"')
	{
		p++;
		p[strlen(p)-1] = 0;
	}
	strcat(text, p);
	for (j = 0, client = svs.clients; j < maxclients->value; j++, client++)
	{
		if (!client || client->state != cs_spawned)
			continue;
		SV_ClientPrintf(client, PRINT_CHAT, "%s\n", text);
	}
}
/*
==================
SV_Heartbeat_f
==================
*/
void SV_Heartbeat_f (void)
{
	svs.last_heartbeat = -9999999;
}
/*
===========
SV_Serverinfo_f
  Examine or change the serverinfo string
===========
*/
void SV_Serverinfo_f (void)
{
	Com_Printf ("Server info settings:\n");
	Info_Print (Cvar_Serverinfo());
}
/*
===========
SV_DumpUser_f
Examine all a users info strings
===========
*/
void SV_DumpUser_f (void)
{
	if (GetArgc() != 2)
	{
		Com_Printf ("Usage: info <userid>\n");
		return;
	}
	if (!SV_SetPlayer ())
		return;
	Com_Printf ("userinfo\n");
	Com_Printf ("--------\n");
	Info_Print (sv_client->userinfo);
}

/*
==============
SV_ServerRecord_f
Begins server demo recording.  Every entity and every message will be
recorded, but no playerinfo will be stored.  Primarily for demo merging.
==============
*/
void SV_ServerRecord_f (void)
{
	char	name[MAX_OSPATH];
	char	buf_data[MAX_MSGLEN];
	sizebuf_t	buf;
	int		len;
	int		i;
	if (GetArgc() != 2)
	{
		Com_Printf ("serverrecord <demoname>\n");
		return;
	}
	if (svs.demofile)
	{
		Com_Printf ("Already recording.\n");
		return;
	}
	if (sv.state != ss_game)
	{
		Com_Printf ("You must be in a level to record.\n");
		return;
	}
	//
	// open the demo file
	//
	Com_sprintf (name, sizeof(name), "%s/demos/%s.dem", FS_Gamedir(), GetArgv(1));
	Com_Printf ("recording to %s.\n", name);
	FS_CreatePath (name);
	svs.demofile = fopen (name, "wb");
	if (!svs.demofile)
	{
		Com_Printf ("ERROR: couldn't open.\n");
		return;
	}
	// setup a buffer to catch all multicasts
	SZ_Init (&svs.demo_multicast, svs.demo_multicast_buf, sizeof(svs.demo_multicast_buf));
	//
	// write a single giant fake message with all the startup info
	//
	SZ_Init (&buf, (byte *)buf_data, sizeof(buf_data));
/*
	//
	// serverdata needs to go over for all types of servers
	// to make sure the protocol is right, and to set the gamedir
	//
	// send the serverdata
	MSG_WriteByte (&buf, svc_serverdata);
	MSG_WriteLong (&buf, PROTOCOL_VERSION);
	MSG_WriteLong (&buf, svs.spawncount);
	// 2 means server demo
	MSG_WriteByte (&buf, 2);	// demos are always attract loops
	MSG_WriteString (&buf, Cvar_VariableString ("gamedir"));
	MSG_WriteShort (&buf, -1);
	// send full levelname
	MSG_WriteString (&buf, sv.configstrings[CS_NAME]);
*/
	for (i=0 ; i<MAX_CONFIGSTRINGS ; i++)
		if (sv.configstrings[i][0])
		{
			MSG_WriteByte (&buf, svc_configstring);
			MSG_WriteShort (&buf, i);
			MSG_WriteString (&buf, sv.configstrings[i]);
		}
	// write it to the demo file
	Com_DPrintf ("signon message length: %i\n", buf.cursize);
	len = LittleLong (buf.cursize);
	fwrite (&len, 4, 1, svs.demofile);
	fwrite (buf.data, buf.cursize, 1, svs.demofile);
	// the rest of the demo file will be individual frames
}
/*
==============
SV_ServerStop_f
Ends server demo recording
==============
*/
void SV_ServerStop_f (void)
{
	if (!svs.demofile)
	{
		Com_Printf ("Not doing a serverrecord.\n");
		return;
	}
	fclose (svs.demofile);
	svs.demofile = NULL;
	Com_Printf ("Recording completed.\n");
}
/*
===============
SV_KillServer_f
Kick everyone off, possibly in preparation for a new game
===============
*/
void SV_KillServer_f (void)
{
	if (!svs.initialized)
		return;
	SV_Shutdown ("Server was killed.\n", false, SV_UNLOAD_DLLS);
	NET_Config ( false );	// close network sockets

	// WAW[12/6/99]: Zero all network varables on Server exit.
//	Cvar_ForceSet( "ctf", "0" );
//	Cvar_ForceSet( "deathmatch", "0" );
//	Cvar_ForceSet( "coop", "0" );
	// =======================================================
}

/*
===============
SV_ServerCommand_f

Let the game dll handle a command
===============
*/
void SV_ServerCommand_f (void)
{
	if (!ge)
	{
		Com_Printf ("No game loaded.\n");
		return;
	}

	ge->ServerCommand();
}

//===========================================================
/*
==================
SV_InitOperatorCommands
==================
*/
void SV_InitOperatorCommands (void)
{
	Cmd_AddCommand ("heartbeat", SV_Heartbeat_f);
	Cmd_AddCommand ("kick", SV_Kick_f);
	Cmd_AddCommand ("status", SV_Status_f);
	Cmd_AddCommand ("serverinfo", SV_Serverinfo_f);
	Cmd_AddCommand ("dumpuser", SV_DumpUser_f);
	Cmd_AddCommand ("map", SV_Map_f);
	Cmd_AddCommand ("demomap", SV_DemoMap_f);
	Cmd_AddCommand ("demoplay", SV_DemoMap_f);
	Cmd_AddCommand ("gamemap", SV_GameMap_f);
	Cmd_AddCommand ("setmaster", SV_SetMaster_f);
	if ( dedicated->value )
		Cmd_AddCommand ("say", SV_ConSay_f);
	Cmd_AddCommand ("serverrecord", SV_ServerRecord_f);
	Cmd_AddCommand ("serverstop", SV_ServerStop_f);
	Cmd_AddCommand ("save", SV_Savegame_f);
	Cmd_AddCommand ("load", SV_Loadgame_f);
	Cmd_AddCommand ("killserver", SV_KillServer_f);

	Cmd_AddCommand ("sv", SV_ServerCommand_f);

	//	Nelno's addition for loading maps without unloading DLLs
	Cmd_AddCommand ("changelevel", SV_GameMap_f);
	Cmd_AddCommand ("unload", SV_Unload_f);
}
