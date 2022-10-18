// sv_game.c -- interface to the game dll
#include	"server.h"
#include	"daikatana.h"

//	inventory system
#include	"dk_inv.h"

//ingame cinematics
#include "dk_gce_main.h"

#include "sound.h"		// jas 9/11/98

#include "ref.h"

game_export_t	*ge;
extern	refexport_t	re;		// interface to refresh .dll

qboolean PF_TestUnicast(edict_t *ent, qboolean reliable)
{
	int			p;
	client_t	*client;
	sizebuf_t	*buf;
	int			length;
	if (!ent)
		return FALSE;
	p = NUM_FOR_EDICT(ent);
	if (p < 1 || p > maxclients->value)
		return FALSE;
	client = svs.clients + (p-1);

	if (reliable)
		buf = &client->netchan.message;
	else
		buf = &client->datagram;

	if (!buf)
		return FALSE;

	length = sv.multicast.cursize;
	if ( (buf->maxsize == 0) || ((buf->cursize + length > buf->maxsize) && ((!buf->allowoverflow) || (length > buf->maxsize))) )
		return FALSE;

	return TRUE;
}

/*
===============
PF_Unicast
Sends the contents of the mutlicast buffer to a single client
===============
*/
void PF_Unicast (edict_t *ent, qboolean reliable)
{
	int		p;
	client_t	*client;
	if (!ent)
		return;
	p = NUM_FOR_EDICT(ent);
	if (p < 1 || p > maxclients->value)
		return;
	client = svs.clients + (p-1);
	if (reliable)
		SZ_Write (&client->netchan.message, sv.multicast.data, sv.multicast.cursize);
	else
		SZ_Write (&client->datagram, sv.multicast.data, sv.multicast.cursize);
	SZ_Clear (&sv.multicast);
}

///////////////////////////////////////////////////////////////////////////////
//	PF_dprintf
//
//	Debug print to server console
//	only prints if developer flag is set
///////////////////////////////////////////////////////////////////////////////

void Con_Dprintf(char *fmt, ...)
{
	char		msg[1024];
	va_list		argptr;

	if (!developer || !developer->value)	
		return;

	va_start (argptr,fmt);
	vsprintf (msg, fmt, argptr);
	va_end (argptr);
	
	Com_Printf ("%s", msg);
}

///////////////////////////////////////////////////////////////////////////////
//	Con_Printf
//	Debug print to server console
///////////////////////////////////////////////////////////////////////////////

void Con_Printf(char *fmt, ...) {
    char        msg[1024];
    va_list     argptr;
    
    va_start (argptr,fmt);
    vsprintf (msg, fmt, argptr);
    va_end (argptr);
    Com_Printf("%s", msg);
}

/*
===============
PF_cprintf
Print to a single client
===============
*/
void PF_cprintf (edict_t *ent, int level, char *fmt, ...)
{
	char		msg[1024];
	va_list		argptr;
	int			n;

	if (ent)
	{
		n = NUM_FOR_EDICT(ent);
		if (n < 1 || n > maxclients->value)
			Com_Error (ERR_DROP, "cprintf to a non-client");
	}
	va_start (argptr,fmt);
	vsprintf (msg, fmt, argptr);
	va_end (argptr);

	if (ent)
	{
		client_t *cl = ( client_t *)( svs.clients+(n-1) );
		if( cl->state >= cs_connected )
		{
			SV_ClientPrintf (svs.clients+(n-1), level, "%s", msg);
		}
	}
	else
		Com_Printf ("%s", msg);
}
/*
===============
PF_centerprintf
centerprint to a single client
===============
*/
void PF_centerprintf (edict_t *ent, char *fmt, ...)
{
	char		msg[1024];
	va_list		argptr;
	int			n;
	
	n = NUM_FOR_EDICT(ent);
	if (n < 1 || n > maxclients->value)
		return;	// Com_Error (ERR_DROP, "centerprintf to a non-client");
	va_start (argptr,fmt);
	vsprintf (msg, fmt, argptr);
	va_end (argptr);
	MSG_WriteByte (&sv.multicast,svc_centerprint);
	MSG_WriteString (&sv.multicast,msg);
	MSG_WriteShort (&sv.multicast, 20);
	PF_Unicast (ent, true);
}

///////////////////////////////////////////////////////////////////////////////
//	PF_centerprint
//
//	Nelno:	I made this!
//	centerprint to a single client with variable delay time
///////////////////////////////////////////////////////////////////////////////

void PF_centerprint (edict_t *ent, float msg_time, char *fmt, ...)
{
	char		msg[1024];
	va_list		argptr;
	int			n;
	
	n = NUM_FOR_EDICT(ent);
	if (n < 1 || n > maxclients->value)
		return;	// Com_Error (ERR_DROP, "centerprintf to a non-client");
	va_start (argptr,fmt);
	vsprintf (msg, fmt, argptr);
	va_end (argptr);
	MSG_WriteByte (&sv.multicast,svc_centerprint);
	MSG_WriteString (&sv.multicast,msg);
	MSG_WriteShort (&sv.multicast,(int) (msg_time * 8.0));
	PF_Unicast (ent, true);
}

/*
===============
PF_error
Abort the server with a game error
===============
*/
void PF_error (char *fmt, ...)
{
	char		msg[1024];
	va_list		argptr;
	
	va_start (argptr,fmt);
	vsprintf (msg, fmt, argptr);
	va_end (argptr);
	Com_Error (ERR_DROP, "Game Error: %s", msg);
}
/*
=================
PF_setmodel
Also sets mins and maxs for inline bmodels
=================
*/
void PF_setmodel (edict_t *ent, char *name)
{
	int		i;
	cmodel_t	*mod;

	//	Nelno: don't drop out on NULL model names	
	
	if (!name)
	{
		//Com_Error (ERR_DROP, "PF_setmodel: NULL");
		Com_Printf ("PF_setmodel: NULL\n");
		ent->s.modelindex = 0;
		return;
	}

	i = SV_ModelIndex (name);
		
//	if (dedicated->value)
//		dk_LoadServerModel (i, name);

//	ent->modelName = name;
	ent->s.modelindex = i;

	//	if it is an inline model, get the size information for it
	//	Nelno:	ie. a b-model.  Info stored in BSP...
	if (name[0] == '*')
	{
		mod = CM_InlineModel (name);
		ent->s.mins = mod->mins;
		ent->s.maxs = mod->maxs;
		SV_LinkEdict (ent);
	}
}
/*
===============
PF_Configstring
===============
*/
void PF_Configstring (int index, char *val)
{
	if (index < 0 || index >= MAX_CONFIGSTRINGS)
		Com_Error (ERR_DROP, "configstring: bad index %i\n", index);
	if (!val)
		val = "";
	// change the string in sv
	strcpy (sv.configstrings[index], val);
	
	if (sv.state != ss_loading)
	{	// send the update to everyone
		SZ_Clear (&sv.multicast);
		MSG_WriteChar (&sv.multicast, svc_configstring);
		MSG_WriteShort (&sv.multicast, index);
		MSG_WriteString (&sv.multicast, val);
		//SV_Multicast (CVector(0,0,0), MULTICAST_ALL_R);
	}
}
void PF_WriteChar (int c) {MSG_WriteChar (&sv.multicast, c);}
void PF_WriteByte (int c) {MSG_WriteByte (&sv.multicast, c);}
void PF_WriteShort (int c) {MSG_WriteShort (&sv.multicast, c);}
void PF_WriteLong (int c) {MSG_WriteLong (&sv.multicast, c);}
void PF_WriteFloat (float f) {MSG_WriteFloat (&sv.multicast, f);}
void PF_WriteString (char *s) {MSG_WriteString (&sv.multicast, s);}
void PF_WritePos (CVector &pos) {MSG_WritePos (&sv.multicast, pos);}
void PF_WriteDir (CVector &dir) {MSG_WriteDir (&sv.multicast, dir);}
void PF_WriteAngle (float f) {MSG_WriteAngle16 (&sv.multicast, f);}
char	*SV_GetConfigString (int index);
/*
=================
PF_inPVS
Also checks portalareas so that doors block sight
=================
*/
qboolean PF_inPVS (CVector &p1, CVector &p2)
{
	int		leafnum;
	int		cluster;
	int		area1, area2;
	byte	*mask;

	leafnum = CM_PointLeafnum (p1);
	cluster = CM_LeafCluster (leafnum);
	area1 = CM_LeafArea (leafnum);
	mask = CM_ClusterPVS (cluster);
	leafnum = CM_PointLeafnum (p2);
	cluster = CM_LeafCluster (leafnum);
	area2 = CM_LeafArea (leafnum);
	if ( mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
		return false;
	if (!CM_AreasConnected (area1, area2))
		return false;		// a door blocks sight
	return true;
}
/*
=================
PF_inPHS
Also checks portalareas so that doors block sound
=================
*/
qboolean PF_inPHS (CVector &p1, CVector &p2)
{
	int		leafnum;
	int		cluster;
	int		area1, area2;
	byte	*mask;

	leafnum = CM_PointLeafnum (p1);
	cluster = CM_LeafCluster (leafnum);
	area1 = CM_LeafArea (leafnum);
	mask = CM_ClusterPHS (cluster);
	leafnum = CM_PointLeafnum (p2);
	cluster = CM_LeafCluster (leafnum);
	area2 = CM_LeafArea (leafnum);
	if ( mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
		return false;		// more than one bounce away
	if (!CM_AreasConnected (area1, area2))
		return false;		// a door blocks hearing
	return true;
}

qboolean    NET_IsLocalAddress (netadr_t adr);
short PF_IsPlayerLocal(int playerNum)
{
	// uhh, not ready for this yet
	if (!svs.initialized || !svs.clients[playerNum].state)
		return -1;

	// can't be local in dedicated mode!
	if (dedicated->value)
		return FALSE;

	return NET_IsLocalAddress(svs.clients[playerNum].netchan.remote_address);
}

client_t *GetLocalClient()
{
	for (int i = 0; i < maxclients->value; i++)
	{
		if (PF_IsPlayerLocal(i) == TRUE)
		{
			return &svs.clients[i];
		}
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////
////	PF_StartSound
////
////	Starts a sound that is associated with an entity
/////////////////////////////////////////////////////////////////////////////////
//
//void PF_StartSound (edict_t *entity, int channel, int sound_num, float volume,float dist_min, float dist_max, float timeofs)
//{
//	if (!entity)
//		return;
//	SV_StartSound (CVector(0,0,0), entity, channel, sound_num, volume, dist_min, dist_max, timeofs);
//}


//==============================================

///////////////////////////////////////////////////////////////////////////////
//	SV_ShutdownDLLs
//
//	called when a map starts
///////////////////////////////////////////////////////////////////////////////

void SV_ShutdownDLLs (void)
{
	//	make sure DLLs are loaded
	if (!ge)
		return;

	///////////////////////////////////////////////////////////////////////////
	//	call each DLLs exit level function, unloads each DLL
	///////////////////////////////////////////////////////////////////////////

	ge->LevelExit ();
}

///////////////////////////////////////////////////////////////////////////////
//	SV_UnloadDLLs
//
//	Called when either the entire server is being killed, or
//	it is changing to a different game directory.
///////////////////////////////////////////////////////////////////////////////

void SV_UnloadDLLs (void)
{
	//	make sure DLLs are loaded
	if (!ge)
		return;

	SV_ShutdownDLLs ();	

	///////////////////////////////////////////////////////////////////////////
	//	call each DLLs exit function, unloads each DLL
	///////////////////////////////////////////////////////////////////////////

	ge->UnloadDLLs ();

	///////////////////////////////////////////////////////////////////////////
	//	unloads physics.dll
	///////////////////////////////////////////////////////////////////////////
	Sys_UnloadPhysicsDLL ();

	ge = NULL;
}

///////////////////////////////////////////////////////////////////////////////
//	SV_InitNewGame
//
//	shuts down the DLLs (if loaded) and then reloads DLLs for a new game.
///////////////////////////////////////////////////////////////////////////////
void SV_AutoSave();
void CS_Init(game_import_t &import);
char*	dk_GetModelSkinName(int modelIndex);

void	SV_InitNewGame (void)
{
	game_import_t	import;

	// unload anything we have now
	if (ge) SV_UnloadDLLs ();
	
	// load a new game dll
	import.multicast = SV_Multicast;
	import.unicast = PF_Unicast;
	import.bprintf = SV_BroadcastPrintf;
	import.Con_Dprintf = Con_Dprintf;
	import.Con_Printf = Con_Printf;
	import.cprintf = PF_cprintf;
	import.centerprintf = PF_centerprintf;
	import.centerprint = PF_centerprint;
	import.Error = PF_error;
	import.linkentity = SV_LinkEdict;
	import.unlinkentity = SV_UnlinkEdict;
	import.BoxEdicts = SV_AreaEdicts;
	import.TraceLine = SV_TraceLine;
	import.TraceLineTrigger = SV_TraceLineTrigger;
	import.TraceBox = SV_TraceBox;
	import.GetTraceCount = SV_GetTraceCount;
	import.pointcontents = SV_PointContents;
	import.setmodel = PF_setmodel;
	import.inPVS = PF_inPVS;
	import.inPHS = PF_inPHS;
	import.Pmove = Pmove;
	import.modelindex = SV_ModelIndex;
	import.soundindex = SV_SoundIndex;
	import.imageindex = SV_ImageIndex;
	import.configstring = PF_Configstring;
	import.GetConfigString = SV_GetConfigString;

	import.StartSound = SV_StartSound;
    import.set_reverb_preset = SV_ReverbPreset ; // jas
	import.streamed_sound = SV_StartStream;
	import.StartMusic = SV_StartMusic;
	import.StartMP3 = SV_StartMP3;
	import.StopMP3 = SV_StopMP3;

	import.WriteChar = PF_WriteChar;
	import.WriteByte = PF_WriteByte;
	import.WriteShort = PF_WriteShort;
	import.WriteLong = PF_WriteLong;
	import.WriteFloat = PF_WriteFloat;
	import.WriteString = PF_WriteString;
	import.WritePosition = PF_WritePos;
	import.WriteDir = PF_WriteDir;
	import.WriteAngle = PF_WriteAngle;
	import.X_Malloc = X_Malloc;
	import.X_Free = X_Free;
	import.Mem_Free_Tag = Mem_Free_Tag;
	import.Mem_Heap_Walk = Mem_Heap_Walk;
	import.cvar = Cvar_Get;
	import.cvar_set = Cvar_Set;
	import.cvar_forceset = Cvar_ForceSet;
	import.GetArgc = GetArgc;
	import.GetArgv = GetArgv;
	import.GetArgs = GetArgs;
	import.CBuf_AddText = Cbuf_AddText;
	import.DebugGraph = SCR_DebugGraph;
	import.SetAreaPortalState = CM_SetAreaPortalState;
	import.AreasConnected = CM_AreasConnected;

	//	Nelno:	Additional functions that for export to the physics DLL
	
	import.RemoveDLLCommands = Cmd_RemoveDLLCommands;
	import.AddCommand = Cmd_AddCommandDLL;
	import.dk_dlldir = dk_dllDir;
	import.dk_gamedir = FS_Gamedir ();
	import.dk_gamename = gamename;

	//	so that DLL_Init can find -dll overrides
	import.EXEGetArgc = EXEGetArgc;
	import.EXEGetArgv = EXEGetArgv;

	import.printxy = dk_printxy;
	import.clearxy = dk_print_clear;

	import.dk_GetFrameName = dk_GetFrameName;
	import.dk_GetAnimSequences = dk_GetAnimSequences;
	import.dk_GetModelSkinIndex = dk_GetModelSkinIndex;
	import.dk_GetModelSkinName = dk_GetModelSkinName;
	import.dk_ForceModelLoad = dk_ForceModelLoad;
	import.dk_PreCacheModel = dk_PreCacheModel;

	import.sv_prepped_refresh = &sv.prepped_refresh;
	import.Sys_Milliseconds = Sys_Milliseconds;

	//	File IO routines
	import.FS_FileLength = FS_filelength;
	import.FS_Close = FS_FCloseFile;
	import.FS_Open = FS_FOpenFile;
	import.FS_Read = FS_Read;
	import.FS_LoadFile = FS_LoadFile;
	import.FS_FreeFile = FS_FreeFile;
	import.FS_Seek = FS_Seek;
	import.FS_Tell = FS_Tell;
	import.FS_Getc = FS_Getc;
	import.FS_Ungetc = FS_Ungetc;
	import.FS_Gets = FS_Gets;
	import.FS_GetBytesRead = FS_GetBytesRead;

    	import.CIN_CamLocation = CIN_CamLocation;
    	import.CIN_GCE_Interface = CIN_GCE_Interface;
    	import.CIN_WorldPlayback = CIN_WorldPlayback;
	
	import.SV_EmitSyncEntity = SV_EmitSyncEntity;

	// SCG[11/4/99]: Added back quake type memory allocation...
	import.TagMalloc = Z_TagMalloc;
	import.TagFree = Z_Free;
	import.FreeTags = Z_FreeTags;

	import.IsPlayerLocal = PF_IsPlayerLocal;

	import.TestMulticast = SV_TestMulticast;
	import.TestUnicast = PF_TestUnicast;

	//import.AutoSave = SV_AutoSave;
	//CS_Init(import);

	//	Nelno:	Load up physics.dll, which will load other DLLs
	ge = (game_export_t *)Sys_GetGameAPI (&import);

	if (!ge)
		Com_Error (ERR_DROP, "failed to load %s\n", gamename);

	//	Q2FIXME:	check physics.dll version...
/*
	if (ge->apiversion != GAME_API_VERSION)
		Com_Error (ERR_DROP, "game is version %i, not %i", ge->apiversion,
		GAME_API_VERSION);
*/

	ge->InitDLLs ();
}


///////////////////////////////////////////////////////////////////////////////
//	SV_InitNewMap
//
//	shuts down the DLLs (if loaded) and then reloads DLLs for a new game.
///////////////////////////////////////////////////////////////////////////////

void	SV_InitNewMap (void)
{
	///////////////////////////////////////////////////////////////////////////
	//	do level shutdown stuff for DLLs
	///////////////////////////////////////////////////////////////////////////

	//	already gets called in SV_Shutdown!!
//	ge->LevelExit ();

	///////////////////////////////////////////////////////////////////////////
	//	do level init stuff for DLLs (malloc edicts, etc)
	///////////////////////////////////////////////////////////////////////////

	ge->InitChangelevel ();
}
