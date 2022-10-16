// sv_user.c -- server code for moving users
#include "server.h"
edict_t	*sv_player;
/*
============================================================
USER STRINGCMD EXECUTION
sv_client and sv_player will be valid.
============================================================
*/
/*
==================
SV_BeginDemoServer
==================
*/
void SV_BeginDemoserver (void)
{
	char		name[MAX_OSPATH];
	Com_sprintf (name, sizeof(name), "demos/%s", sv.name);
	FS_FOpenFile (name, &sv.demofile);
	if (!sv.demofile)
		Com_Error (ERR_DROP, "Couldn't open %s\n", name);
}
/*
================
SV_New_f
Sends the first message from the server to a connected client.
This will be sent on the initial connection and upon each server load.
================
*/
void SV_New_f (void)
{
	char		*gamedir;
	int			playernum;
	edict_t		*ent;

//	Com_DPrintf ("New() from %s\n", sv_client->name);
	if (sv_client->state != cs_connected)
	{
		Com_Printf ("New not valid -- already spawned\n");
		return;
	}
	// demo servers just dump the file message
	if (sv.state == ss_demo)
	{
		SV_BeginDemoserver ();
		return;
	}
	//
	// serverdata needs to go over for all types of servers
	// to make sure the protocol is right, and to set the gamedir
	//
	gamedir = Cvar_VariableString ("gamedir");
	// send the serverdata
	MSG_WriteByte (&sv_client->netchan.message, svc_serverdata);
	MSG_WriteLong (&sv_client->netchan.message, PROTOCOL_VERSION);
	MSG_WriteLong (&sv_client->netchan.message, svs.spawncount);
	MSG_WriteByte (&sv_client->netchan.message, sv.attractloop);
	MSG_WriteString (&sv_client->netchan.message, gamedir);
	if (sv.state == ss_cinematic || sv.state == ss_pic)
		playernum = -1;
	else
		playernum = sv_client - svs.clients;
	MSG_WriteShort (&sv_client->netchan.message, playernum);
	// send full levelname
	MSG_WriteString (&sv_client->netchan.message, sv.configstrings[CS_NAME]);
	//
	// game server
	// 
	if (sv.state == ss_game)
	{
		// set up the entity for the client
		ent = EDICT_NUM(playernum+1);
		ent->s.number = playernum+1;
		sv_client->edict = ent;
		memset (&sv_client->lastcmd, 0, sizeof(sv_client->lastcmd));
		// begin fetching configstrings
		MSG_WriteByte (&sv_client->netchan.message, svc_stufftext);
		MSG_WriteString (&sv_client->netchan.message, va("cmd configstrings %i 0\n",svs.spawncount) );
	}
}
/*
==================
SV_Configstrings_f
==================
*/
void SV_Configstrings_f (void)
{
	int			start;

//	Com_DPrintf ("Configstrings() from %s\n", sv_client->name);

	if (sv_client->state != cs_connected)
	{
		Com_Printf ("configstrings not valid -- already spawned\n");
		return;
	}
	// handle the case of a level changing while a client was connecting
	if ( atoi(GetArgv(1)) != svs.spawncount )
	{
		Com_Printf ("SV_Configstrings_f from different level\n");
		SV_New_f ();
		return;
	}
	
	start = atoi(GetArgv(2));
	// write a packet full of data
	while ( sv_client->netchan.message.cursize < MAX_MSGLEN/2 
		&& start < MAX_CONFIGSTRINGS)
	{
		if (sv.configstrings[start][0])
		{
			MSG_WriteByte (&sv_client->netchan.message, svc_configstring);
			MSG_WriteShort (&sv_client->netchan.message, start);
			MSG_WriteString (&sv_client->netchan.message, sv.configstrings[start]);
		}
		start++;
	}
	// send next command
	if (start == MAX_CONFIGSTRINGS)
	{
		MSG_WriteByte (&sv_client->netchan.message, svc_stufftext);
		MSG_WriteString (&sv_client->netchan.message, va("cmd baselines %i 0\n",svs.spawncount) );
	}
	else
	{
		MSG_WriteByte (&sv_client->netchan.message, svc_stufftext);
		MSG_WriteString (&sv_client->netchan.message, va("cmd configstrings %i %i\n",svs.spawncount, start) );
	}
}
/*
==================
SV_Baselines_f
==================
*/
void SV_Baselines_f (void)
{
	int		start;
	entity_state_t	nullstate;
	entity_state_t	*base;

//	Com_DPrintf ("Baselines() from %s\n", sv_client->name);

	if (sv_client->state != cs_connected)
	{
		Com_Printf ("baselines not valid -- already spawned\n");
		return;
	}
	
	// handle the case of a level changing while a client was connecting
	if ( atoi(GetArgv(1)) != svs.spawncount )
	{
		Com_Printf ("SV_Baselines_f from different level\n");
		SV_New_f ();
		return;
	}
	
	start = atoi(GetArgv(2));
	memset (&nullstate, 0, sizeof(nullstate));
	// write a packet full of data
	while ( sv_client->netchan.message.cursize <  MAX_MSGLEN/2
		&& start < MAX_EDICTS)
	{
		base = &sv.baselines[start];
		if (base->modelindex || base->sound || base->effects)
		{
			MSG_WriteByte (&sv_client->netchan.message, svc_spawnbaseline);
			MSG_WriteDeltaEntity (&nullstate, base, &sv_client->netchan.message, true);
		}
		start++;
	}
	// send next command
	if (start == MAX_EDICTS)
	{
		MSG_WriteByte (&sv_client->netchan.message, svc_stufftext);
		MSG_WriteString (&sv_client->netchan.message, va("precache ; cmd begin %i\n",svs.spawncount) );
	}
	else
	{
		MSG_WriteByte (&sv_client->netchan.message, svc_stufftext);
		MSG_WriteString (&sv_client->netchan.message, va("cmd baselines %i %i\n",svs.spawncount, start) );
	}
}

///////////////////////////////////////////////////////////////////////////////
//	SV_Begin_f
//
//	Nelno:	executed when a "begin" command is sent from a connecting client ???
///////////////////////////////////////////////////////////////////////////////

void SV_Begin_f (void)
{
	Com_DPrintf ("Begin() from %s\n", sv_client->name);

	// handle the case of a level changing while a client was connecting
	if ( atoi(GetArgv(1)) != svs.spawncount )
	{
		Com_Printf ("SV_Begin_f from different level\n");
		SV_New_f ();
		return;
	}
	
	sv_client->state = cs_spawned;

	// call the game begin function
	ge->ClientBegin (sv_player, sv.loadgame);

	Cbuf_InsertFromDefer ();
}
//=============================================================================
/*
==================
SV_NextDownload_f
==================
*/
void SV_NextDownload_f (void)
{
	int		r;
	int		percent;
	int		size;
	if (!sv_client->download)
		return;
	r = sv_client->downloadsize - sv_client->downloadcount;
	if (r > 1024)
		r = 1024;
	MSG_WriteByte (&sv_client->netchan.message, svc_download);
	MSG_WriteShort (&sv_client->netchan.message, r);
	sv_client->downloadcount += r;
	size = sv_client->downloadsize;
	if (!size)
		size = 1;
	percent = sv_client->downloadcount*100/size;
	MSG_WriteByte (&sv_client->netchan.message, percent);
	SZ_Write (&sv_client->netchan.message,
		sv_client->download + sv_client->downloadcount - r, r);
	if (sv_client->downloadcount != sv_client->downloadsize)
		return;
	FS_FreeFile (sv_client->download);
	sv_client->download = NULL;
}
/*
==================
SV_BeginDownload_f
==================
*/
void SV_BeginDownload_f(void)
{
	char	*name;
	extern	cvar_t *allow_download;
	name = GetArgv(1);
	if (strstr (name, "..") || !allow_download->value
		|| strstr (name, "maps") )	// don't allow full map downloads
	{	// don't allow anything with .. path
		MSG_WriteByte (&sv_client->netchan.message, svc_download);
		MSG_WriteShort (&sv_client->netchan.message, -1);
		MSG_WriteByte (&sv_client->netchan.message, 0);
		return;
	}
	if (sv_client->download)
		FS_FreeFile (sv_client->download);
	sv_client->downloadsize = FS_LoadFile (name, (void **)&sv_client->download);
	sv_client->downloadcount = 0;
	if (!sv_client->download)
	{
		Com_DPrintf ("Couldn't download %s to %s\n", name, sv_client->name);
		MSG_WriteByte (&sv_client->netchan.message, svc_download);
		MSG_WriteShort (&sv_client->netchan.message, -1);
		MSG_WriteByte (&sv_client->netchan.message, 0);
		return;
	}
	SV_NextDownload_f ();
	Com_DPrintf ("Downloading %s to %s\n", name, sv_client->name);
}

//============================================================================
/*
=================
SV_Disconnect_f
The client is going to disconnect, so remove the connection immediately
=================
*/
void SV_Disconnect_f (void)
{
//	SV_EndRedirect ();
	SV_DropClient (sv_client);	
}

/*
==================
SV_ShowServerinfo_f
Dumps the serverinfo info string
==================
*/
void SV_ShowServerinfo_f (void)
{
	Info_Print (Cvar_Serverinfo());
}
void SV_Nextserver (void)
{
	char	*v;
	if (sv.state == ss_game || sv.state == ss_pic)
		return;		// can't nextserver while playing a normal game
	svs.spawncount++;	// make sure another doesn't sneak in
	v = Cvar_VariableString ("nextserver");
	if (!v[0])
		Cbuf_AddText ("killserver\n");
	else
	{
		Cbuf_AddText (v);
		Cbuf_AddText ("\n");
	}
	Cvar_Set ("nextserver","");
}

/*
==================
SV_Nextserver_f
A cinematic has completed or been aborted by a client, so move
to the next server,
==================
*/
void SV_Nextserver_f (void)
{
	if ( atoi(GetArgv(1)) != svs.spawncount )
		return;		// leftover from last server
	SV_Nextserver ();
}
typedef struct
{
	char	*name;
	void	(*func) (void);
} ucmd_t;
ucmd_t ucmds[] =
{
	// auto issued
	{"new", SV_New_f},
	{"configstrings", SV_Configstrings_f},
	{"baselines", SV_Baselines_f},
	{"begin", SV_Begin_f},
	{"nextserver", SV_Nextserver_f},
	{"disconnect", SV_Disconnect_f},

	// issued by hand at client consoles	
	{"info", SV_ShowServerinfo_f},
	{"download", SV_BeginDownload_f},
	{"nextdl", SV_NextDownload_f},
	{NULL, NULL}
};
/*
==================
SV_ExecuteUserCommand
==================
*/
void SV_ExecuteUserCommand (const char *s)
{
	ucmd_t	*u;
	
	Cmd_TokenizeString (s, true);
	sv_player = sv_client->edict;

//	SV_BeginRedirect (RD_CLIENT);

	for (u=ucmds ; u->name ; u++)
		if (!strcmp (GetArgv(0), u->name) )
		{
			u->func ();
			break;
		}
	if (!u->name && sv.state == ss_game)
		//	Nelno: added call to Cmd_ExecuteDLLCommand
		Cmd_ExecuteDLLCommand (GetArgv (0), sv_player);
//		ge->ClientCommand (sv_player);

//	SV_EndRedirect ();
}

short PF_IsPlayerLocal(int playerNum);

/*
==================
SV_SendToLoaclHost -- used to make sure savegems get used from the proper client.
==================
*/
void SV_SendToLoaclHost(const char *s)
{
	client_t *LocalClient = NULL;
	for (int i = 0; i < maxclients->value; i++)
	{
		if (PF_IsPlayerLocal(i) == TRUE)
		{
			LocalClient = &svs.clients[i];
			break;
		}
	}
	
	if (!LocalClient)
		return;

	ucmd_t	*u;
	
	Cmd_TokenizeString (s, true);

	for (u=ucmds ; u->name ; u++)
		if (!strcmp (GetArgv(0), u->name) )
		{
			u->func ();
			break;
		}
	if (!u->name && sv.state == ss_game)
		//	Nelno: added call to Cmd_ExecuteDLLCommand
		Cmd_ExecuteDLLCommand (GetArgv (0), LocalClient->edict);
}

/*
===========================================================================
USER CMD EXECUTION
===========================================================================
*/

void ClientThink (client_t *cl, usercmd_t *cmd)
{
	cl->commandMsec -= cmd->msec;
	if (cl->commandMsec < 0 && sv_enforcetime->value )
	{
		Com_DPrintf ("commandMsec underflow from %s\n", cl->name);
		return;
	}
	ge->ClientThink (cl->edict, cmd);
}

#define	MAX_STRINGCMDS	8
/*
===================
SV_ExecuteClientMessage
The current net_message is parsed for the given client
===================
*/
void SV_ExecuteClientMessage (client_t *cl)
{
	int		c;
	char	*s;
	usercmd_t	nullcmd;
	usercmd_t	oldest, oldcmd, newcmd;
	int		net_drop;
	int		stringCmdCount;
	int		checksum, calculatedChecksum;
	int		checksumIndex;
	qboolean	move_issued;
	sv_client = cl;
	sv_player = sv_client->edict;

	// only allow one move command
	move_issued = false;

	stringCmdCount = 0;
	while (1)
	{
		if (net_message.readcount > net_message.cursize)
		{
			Com_Printf ("SV_ReadClientMessage: badread\n");
			SV_ClientPrintf (cl, PRINT_HIGH, "DROPPED: Bad Read from your client\n");
			SV_DropClient (cl);
			cl->lastmessage = svs.realtime;	// min case there is a funny zombie
			return;
		}	
		c = MSG_ReadByte (&net_message);
		if (c == -1)
			break;
				
		switch (c)
		{
		default:
			Com_Printf ("SV_ReadClientMessage: unknown command char\n");
			SV_ClientPrintf (cl, PRINT_HIGH, "DROPPED: Unknown command from your client\n");
			SV_DropClient (cl);
			cl->lastmessage = svs.realtime;	// min case there is a funny zombie
			return;
						
		case clc_nop:
			break;
		case clc_userinfo:
			strncpy (cl->userinfo, MSG_ReadString (&net_message), sizeof(cl->userinfo)-1);
			SV_UserinfoChanged (cl);
			break;
		case clc_move:
			if (move_issued)
				return;		// someone is trying to cheat...
			move_issued = true;

			checksumIndex = net_message.readcount;
			checksum = MSG_ReadByte (&net_message);

			cl->lastframe = MSG_ReadLong (&net_message);
			memset (&nullcmd, 0, sizeof(nullcmd));
			MSG_ReadDeltaUsercmd (&net_message, &nullcmd, &oldest);		// clear out oldset and read oldest command
			MSG_ReadDeltaUsercmd (&net_message, &oldest, &oldcmd);		// oldest to oldcmd and read oldcmd
			MSG_ReadDeltaUsercmd (&net_message, &oldcmd, &newcmd);		// oldcmd to newcmd and read newcmd
			if ( cl->state != cs_spawned )
			{
				cl->lastframe = -1;
				break;
			}

			// if the checksum fails, ignore the rest of the packet
			calculatedChecksum = COM_BlockSequenceCheckByte (
				net_message.data + checksumIndex + 1,
				net_message.readcount - checksumIndex - 1,
				cl->netchan.incoming_sequence);

			if (calculatedChecksum != checksum)
			{
				Com_DPrintf ("Failed command checksum for %s\n", cl->name);	
				return;
			}

			if (!sv_paused->value)
			{
				net_drop = cl->netchan.dropped;
				if (net_drop < 20)
				{
//if (net_drop > 2)
//	Com_Printf ("drop %i\n", net_drop);
					while (net_drop > 2)
					{
						ClientThink (cl, &cl->lastcmd);
						net_drop--;
					}
					if (net_drop > 1)
						ClientThink (cl, &oldest);
					if (net_drop > 0)
						ClientThink (cl, &oldcmd);
				}
				ClientThink (cl, &newcmd);
			}

			cl->lastcmd = newcmd;
			break;
		case clc_stringcmd:	
			s = MSG_ReadString (&net_message);

			// malicious users may try using too many string commands
			if (++stringCmdCount < MAX_STRINGCMDS)
				SV_ExecuteUserCommand (s);
			if (cl->state == cs_zombie)
				return;	// disconnect command
			break;		
			
		}
	}
}
