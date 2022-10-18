
// added for Japanese version
#ifdef JPN
#include <windows.h>
#endif // JPN

#include "server.h"

netadr_t	master_adr[MAX_MASTERS];	// address of group servers
client_t	*sv_client;			// current client
cvar_t	*sv_paused;
cvar_t	*sv_timedemo;

cvar_t	*sv_enforcetime;
cvar_t	*timeout;				// seconds without any message
cvar_t	*zombietime;			// seconds to sink messages after disconnect
cvar_t	*rcon_password;			// password for remote server commands
cvar_t	*allow_download;
cvar_t	*sv_noreload;			// don't reload level state when reentering
cvar_t	*maxclients;		// FIXME: rename sv_maxclients
cvar_t	*sv_showclamp;
cvar_t	*hostname;
cvar_t	*public_server;			// should heartbeats be sent
cvar_t	*sv_episode;
cvar_t  *sv_mapnum;
cvar_t	*sv_violence;
cvar_t	*sv_demomode;
cvar_t	*sv_frictionfactor;

cvar_t	*sv_reconnect_limit;	// minimum seconds between connect messages

cvar_t	*sv_savegamedir;

//#ifdef _DEBUG
cvar_t	*sv_cinematics;
//#endif

// WAW[12/7/99]: Added.
cvar_t	*coop;
cvar_t	*ctf;
cvar_t  *deathtag;

void Master_Shutdown (void);
//============================================================================
/*
=====================
SV_DropClient
Called when the player is totally leaving the server, either willingly
or unwillingly.  This is NOT called if the entire server is quiting
or crashing.
=====================
*/
void SV_DropClient (client_t *drop)
{
	// add the disconnect
	MSG_WriteByte (&drop->netchan.message, svc_disconnect);
	if (drop->state == cs_spawned)
	{
		// call the prog function for removing a client
		// this will remove the body, among other things
		ge->ClientDisconnect (drop->edict);
	}
	if (drop->download)
	{
		FS_FreeFile (drop->download);
		drop->download = NULL;
	}
	drop->state = cs_zombie;		// become free in a few seconds
	drop->name[0] = 0;
}
/*
==============================================================================
CONNECTIONLESS COMMANDS
==============================================================================
*/

/*
===============
SV_StatusString

Builds the string that is sent as heartbeats and status replies
===============
*/
char	*SV_StatusString (void)
{
	char	player[1024];
	static char	status[MAX_MSGLEN];
	int		i;
	client_t	*cl;
	int		statusLength;
	int		playerLength;

	strcpy (status, Cvar_Serverinfo());
	strcat (status, "\n");
	statusLength = strlen(status);

	for (i=0 ; i<maxclients->value ; i++)
	{
		cl = &svs.clients[i];
		if (cl->state == cs_connected || cl->state == cs_spawned )
		{
			Com_sprintf (player, sizeof(player), "%i %i \"%s\"\n", 
				cl->edict->client->ps.stats[STAT_FRAGS], cl->ping, cl->name);
			playerLength = strlen(player);
			if (statusLength + playerLength >= sizeof(status) )
				break;		// can't hold any more
			strcpy (status + statusLength, player);
			statusLength += playerLength;
		}
	}

	return status;
}

//GAMESPY
/*************
basic_callback
sends a (sample) response to the basic query 
includes the following keys:
\gamename\
\gamever\
\location\
*************/
void basic_callback(char *outbuf, int maxlen, void *userdata)
{
	Com_sprintf(outbuf,maxlen, "\\gamename\\%s\\gamever\\%d\\location\\%d",
			"daikatana",PROTOCOL_VERSION,(int)Cvar_VariableValue("location"));
}

/************
info_callback
Sends a (sample) response to the info query 
including the following keys:
\hostname\
\hostport\
\mapname\
\gametype\
\numplayers\
\maxplayers\
\gamemode\
************/
void info_callback(char *outbuf, int maxlen, void *userdata)
{
	char keyvalue[200];
	int i,numplayers;
	client_t	*cl;

	for (i=0, numplayers=0 ; i<maxclients->value ; i++)
	{
		cl = &svs.clients[i];
		if (cl->state == cs_connected || cl->state == cs_spawned )
		    numplayers++;
    }

	Com_sprintf (keyvalue, sizeof(keyvalue),"\\hostname\\%s",Cvar_VariableString("hostname"));
	strncat(outbuf, keyvalue, maxlen);
	Com_sprintf (keyvalue, sizeof(keyvalue),"\\hostport\\%d",(int)Cvar_VariableValue("port"));
	strncat(outbuf, keyvalue, maxlen);
	Com_sprintf (keyvalue, sizeof(keyvalue),"\\mapname\\%s",Cvar_VariableString("mapname"));
	strncat(outbuf, keyvalue, maxlen);  
	Com_sprintf (keyvalue, sizeof(keyvalue),"\\gametype\\%s",Cvar_VariableString("gamemode"));
	strncat(outbuf, keyvalue, maxlen);
	Com_sprintf (keyvalue, sizeof(keyvalue),"\\numplayers\\%d",numplayers);
	strncat(outbuf, keyvalue, maxlen);
	Com_sprintf (keyvalue, sizeof(keyvalue),"\\maxplayers\\%d",(int)maxclients->value);
	strncat(outbuf, keyvalue, maxlen);
	Com_sprintf (keyvalue, sizeof(keyvalue),"\\gamemode\\%s","openplaying");
	strncat(outbuf, keyvalue, maxlen);

}

/***************
rules_callback
Sends a response to the rules query. You may
need to add custom fields for your game in here. Some are provided
as an example 
The following rules are included:
\timelimit\
\fraglimit\
\teamplay\
\rankedserver\
****************/
void rules_callback(char *outbuf, int maxlen, void *userdata)
{
	strncpy (outbuf, Cvar_Serverinfo(),maxlen);
}

/***************
players_callback
sends the players and their information. 
Note that \ characters are not stripped out of player names. If
your game allows players or team names with the \ character, you will need
to strip or change it here. 
The following keys are included for each player:
\player_N\
\frags_N\
\deaths_N\
\skill_N\
\ping_N\
\team_N\
***************/
void players_callback(char *outbuf, int maxlen, void *userdata)
{
	char keyvalue[160];
	int i,j;
	client_t	*cl;

	for (i=0, j=0 ; i<maxclients->value ; i++)
	{
		cl = &svs.clients[i];
		if (cl->state == cs_connected || cl->state == cs_spawned )
		{
			Com_sprintf (keyvalue, sizeof(keyvalue), "\\player_%d\\%s\\frags_%d\\%d\\ping_%d\\%d", 
                j, cl->name, j, cl->edict->client->ps.stats[STAT_FRAGS],j, cl->ping);
            j++;
    		strncat(outbuf, keyvalue,maxlen);
		}
	}

}



/*
================
SVC_Status

Responds with all the info that qplug or qspy can see
================
*/
void SVC_Status (void)
{
	Com_BeginRedirect (RD_PACKET, sv_outputbuf, SV_OUTPUTBUF_LENGTH, SV_FlushRedirect);
	Com_Printf (SV_StatusString());
	Com_EndRedirect ();
}

/*
================
SVC_Ack

================
*/
void SVC_Ack (void)
{
	Com_Printf ("Ping acknowledge from %s\n", NET_AdrToString(net_from));
}

/*
================
SVC_Info
Responds with short info for broadcast scans
The second parameter should be the current protocol version number.
================
*/
void SVC_Info (void)
{
	char	string[64];
	int		i, count;
	int		version;

	if (maxclients->value == 1)
		return;		// ignore in single player

	version = atoi (GetArgv(1));

	if (version != PROTOCOL_VERSION)
		Com_sprintf (string, sizeof(string), "%s: wrong version\n", hostname->string, sizeof(string));
	else
	{
		count = 0;
		for (i=0 ; i<maxclients->value ; i++)
			if (svs.clients[i].state >= cs_connected)
				count++;

		//Com_sprintf (string, sizeof(string), "%16s %8s %2i/%2i\n", hostname->string, sv.name, count, (int)maxclients->value);
		// WAW[12/7/99]: Game Type.
        Com_sprintf (string, sizeof(string), "%s/%s/%d/%d/%s/%s\n", hostname->string, sv.name, count, (int)maxclients->value,
			Cvar_VariableString("gamemode"),NET_AdrToString(net_from));
        // 2.5 dsn  added '/' separators for easy parsing

	}
	Netchan_OutOfBandPrint (NS_SERVER, net_from, "info\n%s", string);
}
/*
================
SVC_Ping
Just responds with an acknowledgement
================
*/
void SVC_Ping (void)
{
	Netchan_OutOfBandPrint (NS_SERVER, net_from, "ack");
}

/*
=================
SVC_GetChallenge

Returns a challenge number that can be used
in a subsequent client_connect command.
We do this to prevent denial of service attacks that
flood the server with invalid connection IPs.  With a
challenge, they must give a valid IP address.
=================
*/
void SVC_GetChallenge (void)
{
	int		i;
	int		oldest;
	int		oldestTime;

	oldest = 0;
	oldestTime = 0x7fffffff;

	// see if we already have a challenge for this ip
	for (i = 0 ; i < MAX_CHALLENGES ; i++)
	{
		if (NET_CompareBaseAdr (net_from, svs.challenges[i].adr))
			break;
		if (svs.challenges[i].time < oldestTime)
		{
			oldestTime = svs.challenges[i].time;
			oldest = i;
		}
	}

	if (i == MAX_CHALLENGES)
	{
		// overwrite the oldest
		svs.challenges[oldest].challenge = (rand() << 16) ^ rand();
		svs.challenges[oldest].adr = net_from;
		svs.challenges[oldest].time = curtime;
		i = oldest;
	}

	// send it back
	Netchan_OutOfBandPrint (NS_SERVER, net_from, "challenge %i", svs.challenges[i].challenge);
}

/*
==================
SVC_DirectConnect
A connection request that did not come from the master
==================
*/
void SVC_DirectConnect (void)
{
	char		userinfo[MAX_INFO_STRING];
	netadr_t	adr;
	int			i;
	client_t	*cl, *newcl;
	client_t	temp;
	edict_t		*ent;
	int			edictnum;
	int			version;
	int			qport;
	int			challenge;
	adr = net_from;

	Com_DPrintf ("SVC_DirectConnect ()\n");

	version = atoi(GetArgv(1));
	if (version != PROTOCOL_VERSION)
	{
		Netchan_OutOfBandPrint (NS_SERVER, adr, "print\nServer is version %d.\n", PROTOCOL_VERSION);
		Com_DPrintf ("    rejected connect from version %i\n", version);
		return;
	}

	qport = atoi(GetArgv(2));

	challenge = atoi(GetArgv(3));

	strncpy (userinfo, GetArgv(4), sizeof(userinfo)-1);

//	// FIXME: take this bullshit out
//	long l = strlen(userinfo);

	// force the IP key/value pair so the game can filter based on ip
	Info_SetValueForKey (userinfo, "ip", NET_AdrToString(net_from));

	// attractloop servers are ONLY for local clients
	if (sv.attractloop)
	{
		if (!NET_IsLocalAddress (adr))
		{
			Com_Printf ("Remote connect in attract loop.  Ignored.\n");
			Netchan_OutOfBandPrint (NS_SERVER, adr, "print\nConnection refused.\n");
			return;
		}
	}

	// see if the challenge is valid
	if (!NET_IsLocalAddress (adr))
	{
		for (i=0 ; i<MAX_CHALLENGES ; i++)
		{
			if (NET_CompareBaseAdr (net_from, svs.challenges[i].adr))
			{
				if (challenge == svs.challenges[i].challenge)
					break;		// good
				Netchan_OutOfBandPrint (NS_SERVER, adr, "print\nBad challenge.\n");
				return;
			}
		}
		if (i == MAX_CHALLENGES)
		{
			Netchan_OutOfBandPrint (NS_SERVER, adr, "print\nNo challenge for address.\n");
			return;
		}
	}
	newcl = &temp;
	memset (newcl, 0, sizeof(client_t));
	// if there is already a slot for this ip, reuse it
	for (i=0,cl=svs.clients ; i<maxclients->value ; i++,cl++)
	{
		if (cl->state == cs_free)
			continue;
		if (NET_CompareBaseAdr (adr, cl->netchan.remote_address)
			&& ( cl->netchan.qport == qport 
			|| adr.port == cl->netchan.remote_address.port ) )
		{
			if ((svs.realtime - cl->lastconnect) < ((int)sv_reconnect_limit->value * 1000))
			{
				Com_DPrintf ("%s:reconnect rejected : too soon\n", NET_AdrToString (adr));
				return;
			}
			Com_Printf ("%s:reconnect\n", NET_AdrToString (adr));
			newcl = cl;
			goto gotnewcl;
		}
	}
	// find a client slot
	newcl = NULL;
	for (i=0,cl=svs.clients ; i<maxclients->value ; i++,cl++)
	{
		if (cl->state == cs_free)
		{
			newcl = cl;
			break;
		}
	}
	if (!newcl)
	{
		Netchan_OutOfBandPrint (NS_SERVER, adr, "print\nServer is full.\n");
		Com_DPrintf ("Rejected a connection.\n");
		return;
	}
gotnewcl:	
	// build a new connection
	// accept the new client
	// this is the only place a client_t is ever initialized
	*newcl = temp;
	sv_client = newcl;
	edictnum = (newcl-svs.clients)+1;
	ent = EDICT_NUM(edictnum);
	newcl->edict = ent;
	// get the game a chance to reject this connection or modify the userinfo
	if (!(ge->ClientConnect (ent, userinfo, false)))
	{
		Netchan_OutOfBandPrint (NS_SERVER, adr, "print\nConnection refused.\n" );
		Com_DPrintf ("Game rejected a connection.\n");
		return;
	}

	// parse some info from the info strings
	strncpy (newcl->userinfo, userinfo, sizeof(newcl->userinfo)-1);
	SV_UserinfoChanged (newcl);

	// send the connect packet to the client
	Netchan_OutOfBandPrint (NS_SERVER, adr, "client_connect");
	Netchan_Setup (NS_SERVER, &newcl->netchan , adr, qport);
	newcl->state = cs_connected;
	
	SZ_Init (&newcl->datagram, newcl->datagram_buf, sizeof(newcl->datagram_buf) );
	newcl->datagram.allowoverflow = true;
	newcl->lastmessage = svs.realtime;	// don't timeout
	newcl->lastconnect = svs.realtime;
}



int Rcon_Validate (void)
{
	if (!strlen (rcon_password->string))
		return 0;
	if (strcmp (GetArgv(1), rcon_password->string) )
		return 0;
	return 1;
}


/*
===============
SVC_RemoteCommand
A client issued an rcon command.
Shift down the remaining args
Redirect all printfs
===============
*/
void SVC_RemoteCommand (void)
{
	int		i;
	char	remaining[1024];
	i = Rcon_Validate ();
	if (i == 0)
		Com_Printf ("Bad rcon from %s:\n%s\n", NET_AdrToString (net_from), net_message.data+4);
	if (i == 1)
		Com_Printf ("Rcon from %s:\n%s\n", NET_AdrToString (net_from), net_message.data+4);
	Com_BeginRedirect (RD_PACKET, sv_outputbuf, SV_OUTPUTBUF_LENGTH, SV_FlushRedirect);
	if (!Rcon_Validate ())
	{
		Com_Printf ("Bad rcon_password.\n");
	}
	else
	{
		remaining[0] = 0;
		for (i=2 ; i<GetArgc() ; i++)
		{
			strcat (remaining, GetArgv(i) );
			strcat (remaining, " ");
		}
		Cmd_ExecuteString (remaining);
	}
	Com_EndRedirect ();
}
/*
=================
SV_ConnectionlessPacket
A connectionless packet has four leading 0xff
characters to distinguish it from a game channel.
Clients that are in the game can still send
connectionless packets.
=================
*/
void SV_ConnectionlessPacket (void)
{
	const char	*s;
	char	*c;
	MSG_BeginReading (&net_message);
	MSG_ReadLong (&net_message);		// skip the -1 marker
	s = MSG_ReadStringLine (&net_message);
	Cmd_TokenizeString (s, false);
	c = GetArgv(0);
	Com_DPrintf ("Packet %s : %s\n", NET_AdrToString(net_from), c);
	if (!strcmp(c, "ping"))
		SVC_Ping ();
	else if (!strcmp(c, "ack"))
		SVC_Ack ();
	else if (!strcmp(c,"status"))
		SVC_Status ();
	else if (!strcmp(c,"info"))
		SVC_Info ();
	else if (!strcmp(c,"getchallenge"))
		SVC_GetChallenge ();
	else if (!strcmp(c,"connect"))
		SVC_DirectConnect ();
	else if (!strcmp(c, "rcon"))
		SVC_RemoteCommand ();
	else
		Com_Printf ("bad connectionless packet from %s:\n%s\n"
		, NET_AdrToString (net_from), s);
}
//============================================================================

/*
===================
SV_CalcPings

Updates the cl->ping variables
===================
*/
void SV_CalcPings (void)
{
	int			i, j;
	client_t	*cl;
	int			total, count;

	for (i=0 ; i<maxclients->value ; i++)
	{
		cl = &svs.clients[i];
		if (cl->state != cs_spawned )
			continue;

		if (cl->lastframe > 0)
			cl->frame_latency[sv.framenum&(LATENCY_COUNTS-1)] = sv.framenum - cl->lastframe + 1;
		else
			cl->frame_latency[sv.framenum&(LATENCY_COUNTS-1)] = 0;

		total = 0;
		count = 0;
		for (j=0 ; j<LATENCY_COUNTS ; j++)
		{
			if (cl->frame_latency[j] > 0)
			{
				count++;
				total += cl->frame_latency[j];
			}
		}
		if (!count)
			cl->ping = 0;
		else
			cl->ping = total* SV_FRAME_MILLISECONDS / count - SV_FRAME_MILLISECONDS;

		// let the game dll know about the ping
		if ( cl->edict->client )
		{
			cl->edict->client->ping = cl->ping;
		}
	}
}


/*
===================
SV_GiveMsec

Every few frames, gives all clients an allotment of milliseconds
for their command moves.  If they exceed it, assume cheating.
===================
*/
void SV_GiveMsec (void)
{
	int			i;
	client_t	*cl;

	if (sv.framenum & 15)
		return;

	for (i=0 ; i<maxclients->value ; i++)
	{
		cl = &svs.clients[i];
		if (cl->state == cs_free )
			continue;
		
		cl->commandMsec = 1800;		// 1600 + some slop
	}
}


/*
=================
SV_ReadPackets
=================
*/
void SV_ReadPackets (void)
{
	int			i;
	client_t	*cl;
	int			qport;
	while (NET_GetPacket (NS_SERVER, &net_from, &net_message))
	{
		// check for connectionless packet (0xffffffff) first
		if (*(int *)net_message.data == -1)
		{
			SV_ConnectionlessPacket ();
			continue;
		}

		// read the qport out of the message so we can fix up
		// stupid address translating routers
		MSG_BeginReading (&net_message);
		MSG_ReadLong (&net_message);		// sequence number
		MSG_ReadLong (&net_message);		// sequence number
		qport = MSG_ReadShort (&net_message) & 0xffff;

		// check for packets from connected clients
		for (i=0, cl=svs.clients ; i<maxclients->value ; i++,cl++)
		{
			if (cl->state == cs_free)
				continue;
			if (!NET_CompareBaseAdr (net_from, cl->netchan.remote_address))
				continue;
			if (cl->netchan.qport != qport)
				continue;
			if (cl->netchan.remote_address.port != net_from.port)
			{
				Com_Printf ("SV_ReadPackets: fixing up a translated port\n");
				cl->netchan.remote_address.port = net_from.port;
			}
			if (Netchan_Process(&cl->netchan, &net_message))
			{	// this is a valid, sequenced packet, so process it
				if (cl->state != cs_zombie)
				{
					cl->lastmessage = svs.realtime;	// don't timeout
					SV_ExecuteClientMessage (cl);
				}
			}
			break;
		}
		
		if (i != maxclients->value)
			continue;
	}
}
/*
==================
SV_CheckTimeouts
If a packet has not been received from a client for timeout->value
seconds, drop the conneciton.  Server frames are used instead of
realtime to avoid dropping the local client while debugging.
When a client is normally dropped, the client_t goes into a zombie state
for a few seconds to make sure any final reliable message gets resent
if necessary
==================
*/
void SV_CheckTimeouts (void)
{
	int		i;
	client_t	*cl;
	int			droppoint;
	int			zombiepoint;
	droppoint = svs.realtime - 1000*timeout->value;
	zombiepoint = svs.realtime - 1000*zombietime->value;
	for (i=0,cl=svs.clients ; i<maxclients->value ; i++,cl++)
	{
		// message times may be wrong across a changelevel
		if (cl->lastmessage > svs.realtime)
			cl->lastmessage = svs.realtime;
		if (cl->state == cs_zombie
		&& cl->lastmessage < zombiepoint)
		{
			cl->state = cs_free;	// can now be reused
			continue;
		}
		if ( (cl->state == cs_connected || cl->state == cs_spawned) 
			&& cl->lastmessage < droppoint)
		{
			SV_BroadcastPrintf (PRINT_HIGH, "%s timed out\n", cl->name);
			SV_ClientPrintf (cl, PRINT_HIGH, "DROPPED: Your client timed out\n");
			SV_DropClient (cl); 
			cl->lastmessage = svs.realtime;	// min case there is a funny zombie
			cl->state = cs_free;	// don't bother with zombie state
		}
	}
}
/*
================
SV_PrepWorldFrame
This has to be done before the world logic, because
player processing happens outside RunWorldFrame
================
*/
void SV_PrepWorldFrame (void)
{
	edict_t	*ent;
	int		i;
	for (i=0 ; i<ge->num_edicts ; i++, ent++)
	{
		ent = EDICT_NUM(i);
		// events only last for a single message
		ent->s.event = 0;
	}
}
/*
=================
SV_RunGameFrame
=================
*/

void SV_RunGameFrame (void)
{
	if (host_speeds->value)
		time_before_game = Sys_Milliseconds ();

	// we always need to bump framenum, even if we
	// don't run the world, otherwise the delta
	// compression can get confused when a client
	// has the "current" frame
	sv.framenum++;
	
	//	Nelno:	100 = 10 fps, 50 = 20 fps, etc...
	sv.time = sv.framenum * SV_FRAME_MILLISECONDS;

	// don't run if paused
	if ( !sv_paused->value || maxclients->value > 1 )
	{
		ge->RunFrame ();
		// never get more than one tic behind
		if (sv.time < svs.realtime)
		{
			if (sv_showclamp->value)
			{
				Com_Printf ("sv highclamp\n");
			}
			svs.realtime = sv.time;
		}
	}

	if (host_speeds->value)
	{
		time_after_game = Sys_Milliseconds ();
	}
}
/*
==================
SV_Frame
==================
*/

#define	MAX_FRAME_SKIP	0
static	int	sv_frameskip = 0;

/*
typedef	struct	damnit_s
{
	int		realtime;
	int		ran_dlls;
	int		last;
	int		cur;
	int		ms;
	float	between;
} damnit_t;
*/

//static	int	sv_timecount = 0;
//static	damnit_t	damn [100];

//static	int	last_dll_time = 0;
//static	int	cur_dll_time = 0;
//static	float	time_between_dlls = 0;

///////////////////////////////////////////////////////////////////////////////
//	SV_SaveTime
//
//	Nelno:	damn, damn, damn, damn, damn, damn
///////////////////////////////////////////////////////////////////////////////

// SCG[1/21/00]: 'damn' Never used. Useless
/*
void	SV_SaveTime (int time, int ran_dlls, int last, int cur, float between)
{
	sv_timecount++;
	if (sv_timecount > 99)
		sv_timecount = 0;

	damn [sv_timecount].realtime = time;
	damn [sv_timecount].ran_dlls = ran_dlls;
	damn [sv_timecount].last = last;
	damn [sv_timecount].cur = cur;
	damn [sv_timecount].ms = cur - last;
	damn [sv_timecount].between = between;
}
*/

char *SV_MapName()
{
	if( !svs.initialized )
	{
		return NULL;
	}

	return sv.configstrings[0];
}

// cek[6-16-00]: fix autosave bug
qboolean sv_doautosave = FALSE;
void SV_DoAutoSave();
void SV_AutoSave()
{
	sv_doautosave = TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//	SV_Frame
//
///////////////////////////////////////////////////////////////////////////////


extern	int	traceCount;

void SV_Frame (int msec)
{ 
	// cek[6-16-00]: fix autosave bug
	if (sv_doautosave)
	{
		sv_doautosave = FALSE;
		SV_DoAutoSave();
	}
	traceCount = time_before_game = time_after_game = 0;

	// if server is not active, do nothing
	if (!svs.initialized)
		return;

    svs.realtime += msec;

//	Com_Printf ("realtime = %i, time = %i\n", svs.realtime, sv.time);

	// keep the random time dependent
	rand ();

	// check timeouts
	SV_CheckTimeouts ();
	// get packets from clients
	SV_ReadPackets ();

	// move autonomous things around if enough time has passed
	if (!sv_timedemo->value && svs.realtime < sv.time)
	{
//		SV_SaveTime (svs.realtime, false, last_dll_time, last_dll_time, 0);

		// never let the time get too far off
		if (sv.time - svs.realtime > SV_FRAME_MILLISECONDS)
		{
			if (sv_showclamp->value)
				Com_Printf ("sv lowclamp\n");
			svs.realtime = sv.time - SV_FRAME_MILLISECONDS;
		}

		return;
	}

//	cur_dll_time = Sys_Milliseconds ();
//	time_between_dlls = (float) (cur_dll_time - last_dll_time) / 1000.0;
	
//	SV_SaveTime (svs.realtime, true, last_dll_time, cur_dll_time, time_between_dlls);
//	last_dll_time = cur_dll_time;

	// update ping based on the last known frame from all clients
	SV_CalcPings ();

	// give the clients some timeslices
	SV_GiveMsec ();

	// let everything in the world think and move
	SV_RunGameFrame ();

	//	Nelno: only do this stuff ten times per second MAX!

	if (sv_frameskip == 0)
	{
		// send messages back to the clients that had packets read this frame
		SV_SendClientMessages ();
		// save the entire world state if recording a serverdemo
		SV_RecordDemoMessage ();
	}

	sv_frameskip++;
	if (sv_frameskip > MAX_FRAME_SKIP)
		sv_frameskip = 0;

	// clear teleport flags, etc for next frame
	SV_PrepWorldFrame ();

//	Com_Printf ("traceCount = %i\n", traceCount);
}
//============================================================================
/*
================
Master_Heartbeat
Send a message to the master every few minutes to
let it know we are alive, and log information
================
*/
#define	HEARTBEAT_SECONDS	300
void Master_Heartbeat (void)
{
	char		*string;
	int			i;
	
	if (!dedicated->value)
		return;		// only dedicated servers send heartbeats

	if (!public_server->value)
		return;		// a private dedicated game

	// check for time wraparound
	if (svs.last_heartbeat > svs.realtime)
		svs.last_heartbeat = svs.realtime;
	if (svs.realtime - svs.last_heartbeat < HEARTBEAT_SECONDS*1000)
		return;		// not time to send yet
	svs.last_heartbeat = svs.realtime;

	// send the same string that we would give for a status OOB command
	string = SV_StatusString();
	// send to group master
	for (i=0 ; i<MAX_MASTERS ; i++)
		if (master_adr[i].port)
		{
			Com_Printf ("Sending heartbeat to %s\n", NET_AdrToString (master_adr[i]));
			Netchan_OutOfBandPrint (NS_SERVER, master_adr[i], "heartbeat\n%s", string);
		}
}
/*
=================
Master_Shutdown
Informs all masters that this server is going down
=================
*/
void Master_Shutdown (void)
{
	int			i;
	if (!dedicated || !dedicated->value)
		return;		// only dedicated servers send heartbeats

	if (!public_server || !public_server->value)
		return;		// a private dedicated game
	// send to group master
	for (i=0 ; i<MAX_MASTERS ; i++)
		if (master_adr[i].port)
		{
			if (i > 0)
				Com_Printf ("Sending heartbeat to %s\n", NET_AdrToString (master_adr[i]));
			Netchan_OutOfBandPrint (NS_SERVER, master_adr[i], "shutdown");
		}
}
//============================================================================
/*
=================
SV_UserinfoChanged
Pull specific info from a newly changed userinfo string
into a more C freindly form.
=================
*/
void SV_UserinfoChanged (client_t *cl)
{
	char	*val;
    char buf[MAX_QPATH];
	int		i;

	// call prog code to allow overrides
	ge->ClientUserinfoChanged (cl->edict, cl->userinfo);
	
	// name for C code
	strncpy (cl->name, Info_ValueForKey (cl->userinfo, "name"), sizeof(cl->name)-1);

    StripInvalidChars(cl->name,16);  // insure valid filename

// Encompass MarkMa 041399
//#ifdef	EXTASCII	// EXTASCII
//#else 
#ifdef	JPN	// JPN
	// mask off high bit
	for (i=0 ; i<sizeof(cl->name) ; i++)
		if(IsDBCSLeadByte(cl->name[i]))
			i++;
		else
			cl->name[i] &= 127;
#else	// JPN
	// mask off high bit
	for (i=0 ; i<sizeof(cl->name) ; i++)
		cl->name[i] &= 127;
#endif	// JPN
//#endif	// EXTASCII
// Encompass MarkMa 041399


	// single-player?  override with default settings
	/* WAW[11/10/99]: Redundand and problematic.  Removed, does this in ClinetUserinfoChanged.
	if (!Cvar_VariableValue ("deathmatch"))
	{
		// Hiro!
		Info_SetValueForKey(cl->userinfo, "character", "0"); 
		Info_SetValueForKey(cl->userinfo, "modelname", DEFAULT_CHARACTER_MODEL); 
		Info_SetValueForKey(cl->userinfo, "skinname",  DEFAULT_HIRO_SKIN);
		Info_SetValueForKey(cl->userinfo, "skincolor", "0");
	}
	*/

	// character model
    val = Info_ValueForKey (cl->userinfo, "character");
	if (strlen(val))
  	  cl->character = atoi(val);
    else
      cl->character = 0; // Hiro default

    //strncpy (cl->modelname, Info_ValueForKey (cl->userinfo, "modelname"), sizeof(cl->modelname)-1);

    // pull model name from config string
    strcpy(buf, Info_ValueForKey(cl->userinfo, "modelname"));
    
    // error checking here for invalid model name
    if (FS_LoadFile(buf, NULL) != -1)
      strcpy (cl->modelname, buf);
    else
    {
      Com_Printf("Model file not found: %s",buf); // display error
      strcpy(cl->modelname, DEFAULT_CHARACTER_MODEL); // set default model
      strcpy(cl->skinname, DEFAULT_HIRO_SKIN);   // set default skin
    }
    

    // skinname
    //strncpy (cl->skinname, Info_ValueForKey (cl->userinfo, "skinname"), sizeof(cl->skinname)-1);

    // pull skin name from config string
	strcpy(buf, Info_ValueForKey(cl->userinfo, "skinname"));
    
    // error checking here for invalid skin name
    if (FS_LoadFile(buf, NULL) != -1)
      strcpy (cl->skinname, buf);
    else
    {
      Com_Printf("Skin file not found: %s",buf); // display error

      switch (cl->character) // default skin dependant on model
      {
         case 0  : strcpy(cl->skinname, DEFAULT_HIRO_SKIN);     break; // set default model
         case 1  : strcpy(cl->skinname, DEFAULT_MIKIKO_SKIN);   break; // set default model
         case 2  : strcpy(cl->skinname, DEFAULT_SUPERFLY_SKIN); break; // set default model
         default : strcpy(cl->skinname, DEFAULT_HIRO_SKIN);     break; // set default model
      }
    }


    // skincolor
    val = Info_ValueForKey(cl->userinfo, "skincolor");
	if (strlen(val))
  	  cl->skincolor = atoi(val);
    else
      cl->skincolor = 0; // Hiro default



	// rate command
	val = Info_ValueForKey (cl->userinfo, "rate");
	if (strlen(val))
	{
		i = atoi(val);
		cl->rate = i;

		//	Q2FIXME:	does this have to to wit SV_FRAME_MILLISECONDS???
		if (cl->rate < 100)
			cl->rate = 100;
		//if (cl->rate > 15000) // what the ...
		//	cl->rate = 15000;

		if (cl->rate > 25000) // not sure if this function is even used
			cl->rate = 25000;

	}
	else
		cl->rate = 5000;
	// msg command
	val = Info_ValueForKey (cl->userinfo, "msg");
	if (strlen(val))
	{
		cl->messagelevel = atoi(val);
	}
}

//============================================================================
/*
===============
SV_Init
Only called at quake2.exe startup, not for each game
===============
*/
void SV_Init (void)
{
	SV_InitOperatorCommands	();
	rcon_password = Cvar_Get ("rcon_password", "", 0);
	Cvar_Get ("skill", "1", 0);
	Cvar_Get ("deathmatch", "0", CVAR_LATCH);
	coop	= Cvar_Get ("coop", "0", CVAR_LATCH);
	ctf		= Cvar_Get ("ctf", "0", CVAR_LATCH);	// WAW[11/12/99]: Added for CTF mode.
	deathtag = Cvar_Get("deathtag","0",CVAR_LATCH);
	Cvar_Get ("dmflags", va("%i", DF_INSTANT_ITEMS), CVAR_SERVERINFO);
	Cvar_Get ("fraglimit", "0", CVAR_SERVERINFO|CVAR_LATCH|CVAR_ARCHIVE);
	Cvar_Get ("timelimit", "0", CVAR_SERVERINFO|CVAR_LATCH|CVAR_ARCHIVE);
	Cvar_Get ("cheats", "0", CVAR_SERVERINFO|CVAR_LATCH);
	Cvar_Get ("protocol", va("%i", PROTOCOL_VERSION), CVAR_SERVERINFO|CVAR_NOSET);;
	maxclients = Cvar_Get ("maxclients", "1", CVAR_SERVERINFO | CVAR_LATCH);
	hostname = Cvar_Get ("hostname", "noname", CVAR_SERVERINFO | CVAR_ARCHIVE);
    StripInvalidChars(hostname->string,31);

#ifdef _DEBUG
	timeout = Cvar_Get ("timeout", "600", 0);	//	was 125, set higher for debugging
#else
	timeout = Cvar_Get ("timeout", "125", 0);	//	was 125, set higher for debugging
#endif

	zombietime = Cvar_Get ("zombietime", "2", 0);
	sv_showclamp = Cvar_Get ("showclamp", "0", 0);
	sv_paused = Cvar_Get ("paused", "0", 0);
	sv_timedemo = Cvar_Get ("timedemo", "0", 0);
	sv_enforcetime = Cvar_Get ("sv_enforcetime", "0", 0);
	allow_download = Cvar_Get ("allow_download", "1", 0);
	sv_noreload = Cvar_Get ("sv_noreload", "0", 0);
	sv_episode = Cvar_Get ("sv_episode", "1", 0);
	sv_mapnum  = Cvar_Get ("sv_mapnum", "1", 0);
	sv_demomode = Cvar_Get ("sv_demomode", "0", 0);
	
	sv_frictionfactor = Cvar_Get( "sv_frictionfactor", "1.0", CVAR_ARCHIVE );

#ifdef _DEBUG
	sv_cinematics = Cvar_Get( "sv_cinematics", "1.0", CVAR_ARCHIVE );
#endif

	// WAW[12/7/99]: Make sure DM Varables are around for the Dedicated server.
	Cvar_Get("rate", "25000", CVAR_USERINFO | CVAR_ARCHIVE);
	Cvar_Get("dm_skill_level", "0", CVAR_SERVERINFO|CVAR_LATCH);
	Cvar_Get("dm_max_players", "0", CVAR_SERVERINFO|CVAR_LATCH);
	Cvar_Get("dm_weapons_stay", "0", CVAR_SERVERINFO|CVAR_LATCH);
	Cvar_Get("dm_spawn_farthest", "0", CVAR_SERVERINFO|CVAR_LATCH);
	Cvar_Get("dm_allow_exiting", "0", CVAR_SERVERINFO|CVAR_LATCH);
	Cvar_Get("dm_same_map", "0", CVAR_SERVERINFO|CVAR_LATCH);
	Cvar_Get("dm_force_respawn", "0", CVAR_SERVERINFO|CVAR_LATCH);
	Cvar_Get("dm_falling_damage", "1", CVAR_SERVERINFO|CVAR_LATCH);
	Cvar_Get("dm_allow_powerups", "1", CVAR_SERVERINFO|CVAR_LATCH);
	Cvar_Get("dm_allow_health", "1", CVAR_SERVERINFO|CVAR_LATCH);
	Cvar_Get("dm_allow_armor", "1", CVAR_SERVERINFO|CVAR_LATCH);
	Cvar_Get("dm_infinite_ammo", "0", CVAR_SERVERINFO|CVAR_LATCH);
	Cvar_Get("dm_fixed_fov", "0", CVAR_SERVERINFO|CVAR_LATCH);
	Cvar_Get("dm_teamplay", "0", CVAR_SERVERINFO|CVAR_LATCH);
	Cvar_Get("dm_friendly_fire", "0", CVAR_SERVERINFO|CVAR_LATCH);
    Cvar_Get("dm_footsteps", "1", CVAR_SERVERINFO|CVAR_LATCH);
    Cvar_Get("dm_allow_hook", "1", CVAR_SERVERINFO|CVAR_LATCH);
    Cvar_Get("dm_item_respawn", "1", CVAR_SERVERINFO|CVAR_LATCH);
    Cvar_Get("dm_use_skill_system", "1", CVAR_SERVERINFO|CVAR_LATCH);
    Cvar_Get("dm_instagib", "0", CVAR_SERVERINFO|CVAR_LATCH);

	Cvar_Get("maps_dm","",CVAR_SERVERINFO|CVAR_LATCH|CVAR_ARCHIVE);
	Cvar_Get("maps_deathtag","",CVAR_SERVERINFO|CVAR_LATCH|CVAR_ARCHIVE);
	Cvar_Get("maps_ctf","",CVAR_SERVERINFO|CVAR_LATCH|CVAR_ARCHIVE);
	Cvar_Get("motd","",CVAR_ARCHIVE);
	
	//	sv_violence
	//	2 = show blood and gibs
	//	1 = show blood, no gibs
	//	0 = no blood, no gibs
	sv_violence = Cvar_Get ("sv_violence", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE);

	public_server = Cvar_Get ("public", "0", 0);

	sv_reconnect_limit = Cvar_Get ("sv_reconnect_limit", "3", CVAR_ARCHIVE);
	SZ_Init (&net_message, net_message_buffer, sizeof(net_message_buffer));

	sv_savegamedir = Cvar_Get ("sv_savegamedir", "", CVAR_ARCHIVE);
	
}

/*
==================
SV_FinalMessage
Used by SV_Shutdown to send a final message to all
connected clients before the server goes down.  The messages are sent immediately,
not just stuck on the outgoing message list, because the server is going
to totally exit after returning from this function.
==================
*/
void SV_FinalMessage (const char *message, qboolean reconnect)
{
	int			i;
	client_t	*cl;
	
	SZ_Clear (&net_message);
	MSG_WriteByte (&net_message, svc_print);
	MSG_WriteByte (&net_message, PRINT_HIGH);
	MSG_WriteString (&net_message, message);
	if (reconnect)
		MSG_WriteByte (&net_message, svc_reconnect);
	else
		MSG_WriteByte (&net_message, svc_disconnect);
	// send it twice
	// stagger the packets to crutch operating system limited buffers
	for (i=0, cl = svs.clients ; i<maxclients->value ; i++, cl++)
		if (cl->state >= cs_connected)
			Netchan_Transmit (&cl->netchan, net_message.cursize
			, net_message.data);
	for (i=0, cl = svs.clients ; i<maxclients->value ; i++, cl++)
		if (cl->state >= cs_connected)
			Netchan_Transmit (&cl->netchan, net_message.cursize
			, net_message.data);
}

/*
================
SV_Shutdown
Called when each game quits,
before Sys_Quit or Sys_Error
================
*/
void SV_Shutdown (const char *finalmsg, qboolean reconnect, qboolean unload_dlls)
{
	if (svs.clients)
		SV_FinalMessage (finalmsg, reconnect);

	//Master_Shutdown ();

	if (unload_dlls)
		//	shut down AND unload the DLLs
		SV_UnloadDLLs ();
	else
		//	just shut down the DLLs (run level exit functions, free memory)
		SV_ShutdownDLLs ();

	// free current level
	if (sv.demofile)
		fclose (sv.demofile);
	memset (&sv, 0, sizeof(sv));
	Com_SetServerState (sv.state);
	// free server static data
	if (svs.clients)
		Z_Free( svs.clients );
//		X_Free (svs.clients);
	if (svs.client_entities)
		Z_Free( svs.client_entities );
//		X_Free (svs.client_entities);
	if (svs.demofile)
		fclose (svs.demofile);
	memset (&svs, 0, sizeof(svs));
//   UDP_Log(6, "SV_Shutdown");
//   UDP_ShutDown();
}
