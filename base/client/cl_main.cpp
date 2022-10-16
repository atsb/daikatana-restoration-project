// cl_main.c  -- client main loop
#include	<io.h>

#include	"client.h"
//	Nelno:	header for daikatana specific code
//#include	"daikatana.h"
#include	"dk_gce_main.h"
#include	"dk_scoreboard.h"
//#include	"dk_menu.h"
#include	"cl_inventory.h"
#include  "cl_sidekick.h"
#include  "cl_icons.h"
//#include  "ClientEntityManager.h"
#ifdef _WIN32
#include "windows.h"
#endif
//GAMESPY
#include "../GOA/CEngine/goaceng.h"

extern	bspModel_t	bspModel;
//SCG: 2-26-99
#ifdef _WIN32
extern  HWND        cl_hwnd;            // Main window handle for life of program
#else
extern  void*       cl_hwnd;            // Main window handle for life of program
#endif

static const char* szAudioEngineDLL = "audio.dll" ;

cvar_t	*cl_checkkick;
cvar_t	*cl_kick_time_warning;
float	cl_kick_time;
float	cl_check_kick_time;


//	client globals for episode number
int		cl_episode_num;			//	defined in cl_main.c
int		cl_last_episode_num;	//	defined in cl_main.c
char	cl_mapname [32];

cvar_t	*freelook;

cvar_t	*adr0;
cvar_t	*adr1;
cvar_t	*adr2;
cvar_t	*adr3;
cvar_t	*adr4;
cvar_t	*adr5;
cvar_t	*adr6;
cvar_t	*adr7;
cvar_t	*adr8;

cvar_t	*cl_stereo_separation;
cvar_t	*cl_stereo;

cvar_t	*rcon_client_password;
cvar_t	*rcon_address;

cvar_t	*cl_noskins;
cvar_t	*cl_autoskins;
//cvar_t	*cl_footsteps;
cvar_t	*cl_timeout;
cvar_t	*cl_predict;
cvar_t	*cl_minfps;
cvar_t	*cl_maxfps;
cvar_t	*cl_gun;

cvar_t  *dm_footsteps;
cvar_t  *deathmatch;
cvar_t	*cl_dmhints;

cvar_t	*cl_add_particles;
cvar_t	*cl_add_lights;
cvar_t	*cl_add_entities;
cvar_t	*cl_add_blend;

cvar_t	*cl_add_flares;				//  Shawn:  Added for Daikatana
cvar_t	*cl_add_comparticles;		//  Shawn:  Added for Daikatana

cvar_t	*cl_shownet;
cvar_t	*cl_showmiss;
cvar_t	*cl_showclamp;

cvar_t	*cl_paused;
cvar_t	*cl_timedemo;

cvar_t	*lookspring;
cvar_t	*lookstrafe;
cvar_t	*sensitivity;

cvar_t	*m_pitch;
cvar_t	*m_yaw;
cvar_t	*m_forward;
cvar_t	*m_side;

cvar_t	*cl_lightlevel;

cvar_t	*cl_antiskate;

//
// userinfo
//
cvar_t	*info_password;
cvar_t	*name;
cvar_t	*skin;
cvar_t	*rate;
cvar_t	*fov;
cvar_t	*msg;
cvar_t	*hand;

cvar_t  *team;

cvar_t  *weapon_visible;
cvar_t  *subtitles;

cvar_t  *modelname;
cvar_t  *skinname; // 2.15 dsn
cvar_t  *skincolor;
cvar_t  *character;

cvar_t* cvarAudioEngDLL ;

// CClientEntity physics and stuff
cvar_t  *cl_cspeeds;

cvar_t  *cl_loadscreen;
cvar_t  *cl_lastmapname;

cvar_t	*cl_autoweaponswitch;
cvar_t	*play_intro;

client_static_t	cls;
client_state_t	cl;

// some cvars needed for the config selector to work
cvar_t	*cl_configdir;
cvar_t	*cl_curconfig;

//	list of entities that coincides with server
//	to reference a particular entity, use cl_entities [entity_number]
centity_t		cl_entities[MAX_EDICTS];

//	circular list of entities that are being drawn in each frame
//	to reference the entities in each frame use 
//	cl_parse_entities [(frame->parse_entities+ index)&(MAX_PARSE_ENTITIES-1)]
entity_state_t	cl_parse_entities[MAX_PARSE_ENTITIES];	

load_info_t load_info;

char current_palettedir[MAX_QPATH];


// experience point level up vars
int      xplevelup_mode;
int      xplevelup_selection;
float    xplevelup_timer;
qboolean xplevelup_blink;

byte cl_death_gib     = 0;
byte cl_savegem_count = 0;

float cl_gameover_time  = 0.0f;
float cl_gameover_alpha = 0.0f;
float cl_theend_time  = 0.0f;
float cl_theend_alpha = 0.0f;

//extern qboolean in_appactive;

void DKM_Init();
void DKM_GameShutdown();
void CIN_Reset();

//======================================================================

/*
====================
CL_WriteDemoMessage

Dumps the current net message, prefixed by the length
====================
*/
void CL_WriteDemoMessage (void)
{
	int		len, swlen;

	// the first eight bytes are just packet sequencing stuff
	len = net_message.cursize-8;
	swlen = LittleLong(len);
	fwrite (&swlen, 4, 1, cls.demofile);
	fwrite (net_message.data+8,	len, 1, cls.demofile);
}


/*
====================
CL_Stop_f

stop recording a demo
====================
*/
void CL_Stop_f (void)
{
	int		len;

	if (!cls.demorecording)
	{
		Com_Printf ("Not recording a demo.\n");
		return;
	}

// finish up
	len = -1;
	fwrite (&len, 4, 1, cls.demofile);
	fclose (cls.demofile);
	cls.demofile = NULL;
	cls.demorecording = false;
	Com_Printf ("Stopped demo.\n");
}

/*
====================
CL_Record_f

record <demoname>

Begins recording a demo from the current position
====================
*/
void CL_Record_f (void)
{
	char	name[MAX_OSPATH];
	char	buf_data[MAX_MSGLEN];
	sizebuf_t	buf;
	int		i;
	int		len;
	entity_state_t	*ent;
	entity_state_t	nullstate;

	if (GetArgc() != 2)
	{
		Com_Printf ("record <demoname>\n");
		return;
	}

	if (cls.demorecording)
	{
		Com_Printf ("Already recording.\n");
		return;
	}

	if (cls.state != ca_active)
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
	cls.demofile = fopen (name, "wb");
	if (!cls.demofile)
	{
		Com_Printf ("ERROR: couldn't open.\n");
		return;
	}
	cls.demorecording = true;

	// don't start saving messages until a non-delta compressed message is received
	cls.demowaiting = true;

	//
	// write out messages to hold the startup information
	//
	SZ_Init (&buf, (unsigned char *)&buf_data[0], sizeof(buf_data));

	// send the serverdata
	MSG_WriteByte (&buf, svc_serverdata);
	MSG_WriteLong (&buf, PROTOCOL_VERSION);
	MSG_WriteLong (&buf, 0x10000 + cl.servercount);
	MSG_WriteByte (&buf, 1);	// demos are always attract loops
	MSG_WriteString (&buf, cl.gamedir);
	MSG_WriteShort (&buf, cl.playernum);

	MSG_WriteString (&buf, cl.configstrings[CS_NAME]);

	// configstrings
	for (i=0 ; i<MAX_CONFIGSTRINGS ; i++)
	{
		if (cl.configstrings[i][0])
		{
			if (buf.cursize + strlen (cl.configstrings[i]) + 32 > buf.maxsize)
			{	// write it out
				len = LittleLong (buf.cursize);
				fwrite (&len, 4, 1, cls.demofile);
				fwrite (buf.data, buf.cursize, 1, cls.demofile);
				buf.cursize = 0;
			}

			MSG_WriteByte (&buf, svc_configstring);
			MSG_WriteShort (&buf, i);
			MSG_WriteString (&buf, cl.configstrings[i]);
		}

	}

	// baselines
	memset (&nullstate, 0, sizeof(nullstate));
	for (i=0; i<MAX_EDICTS ; i++)
	{
		ent = &cl_entities[i].baseline;
		if (!ent->modelindex)
			continue;

		if (buf.cursize + 64 > buf.maxsize)
		{	// write it out
			len = LittleLong (buf.cursize);
			fwrite (&len, 4, 1, cls.demofile);
			fwrite (buf.data, buf.cursize, 1, cls.demofile);
			buf.cursize = 0;
 		}

		MSG_WriteByte (&buf, svc_spawnbaseline);		
		MSG_WriteDeltaEntity (&nullstate, &cl_entities[i].baseline, &buf, true);
	}

	MSG_WriteByte (&buf, svc_stufftext);
	MSG_WriteString (&buf, "precache\n");

	//	write out DLL info for clients
/*
	MSG_WriteByte (&buf, svc_setclientmodels);
	MSG_WriteShort (&buf, cl.num_player_head);
	MSG_WriteShort (&buf, cl.num_player_torso);
	MSG_WriteShort (&buf, cl.num_player_legs);
	MSG_WriteShort (&buf, cl.num_player_weapon);
*/
	// write it to the demo file

	len = LittleLong (buf.cursize);
	fwrite (&len, 4, 1, cls.demofile);
	fwrite (buf.data, buf.cursize, 1, cls.demofile);

	// the rest of the demo file will be individual frames
}

//======================================================================

/*
===================
Cmd_ForwardToServer

adds the current command line as a clc_stringcmd to the client message.
things like godmode, noclip, etc, are commands directed to the server,
so when they are typed in at the console, they will need to be forwarded.
===================
*/
void Cmd_ForwardToServer (void)
{
	char	*cmd;

	cmd = GetArgv(0);

	if (cls.state <= ca_connected || *cmd == '-' || *cmd == '+')
	{
		Com_Printf ("Unknown command \"%s\"\n", cmd);
		return;
	}

	MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
	SZ_Print (&cls.netchan.message, cmd);
	if (GetArgc() > 1)
	{
		SZ_Print (&cls.netchan.message, " ");
		SZ_Print (&cls.netchan.message, GetArgs());
	}
}

void CL_Setenv_f( void )
{
	int argc = GetArgc();
	if ( argc > 2 )
	{
		char buffer[1000];
		int i;
		strcpy( buffer, GetArgv(1) );
		strcat( buffer, "=" );
		for ( i = 2; i < argc; i++ )
		{
			strcat( buffer, GetArgv( i ) );
			strcat( buffer, " " );
		}
		putenv( buffer );
	}
	else if ( argc == 2 )
	{
		char *env = getenv( GetArgv(1) );

		if ( env )
		{
			Com_Printf( "%s=%s\n", GetArgv(1), env );
		}
		else
		{
			Com_Printf( "%s undefined\n", GetArgv(1), env );
		}
	}
}

/*
==================
CL_ForwardToServer_f
==================
*/
void CL_ForwardToServer_f (void)
{
	if (cls.state != ca_connected && cls.state != ca_active)
	{
		Com_Printf ("Can't \"%s\", not connected\n", GetArgv(0));
		return;
	}
	
	// don't forward the first argument
	if (GetArgc() > 1)
	{
		MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
		SZ_Print (&cls.netchan.message, GetArgs());
	}
}

/*
==================
CL_Pause_f
==================
*/
//void IN_DeactivateMouse ();
//void IN_ActivateMouse ();
void CL_Pause_f (void)
{
	// never pause in multiplayer
	if (Cvar_VariableValue ("maxclients") > 1 || !Com_ServerState ())
	{
		Cvar_SetValue ("paused", 0);
		return;
	}

	Cvar_SetValue ("paused", !cl_paused->value);

	if (cl_paused->value)
	{
		S_StopMusic(CHAN_MUSIC_MAP);
		S_PauseStreams( 1 );
	}
	else
	{
		S_StartMusic(NULL,CHAN_MUSIC_MAP);
		S_PauseStreams( 0 );
	}
}

/*
==================
CL_Quit_f
==================
*/
void CL_Quit_f (void)
{
	CL_Disconnect ();
	Com_Quit ();
}

/*
================
CL_Drop

Called after an ERR_DROP was thrown
================
*/
void CL_Drop (void)
{
	if (cls.state == ca_uninitialized)
		return;

	//	Nelno:	reordered this a but to get rid of progress indicator
	if (cls.disable_servercount != -1)
		SCR_EndLoadingPlaque ();	// get rid of loading plaque

	if (cls.state == ca_disconnected)
		return;

	CL_Disconnect ();

	// drop loading plaque unless this is the initial game start
//	if (cls.disable_servercount != -1)
//		SCR_EndLoadingPlaque ();	// get rid of loading plaque
}


/*
=======================
CL_SendConnectPacket

We have gotten a challenge from the server, so try and
connect.
======================
*/
void CL_SendConnectPacket (void)
{
	netadr_t	adr;
	int		port;

	if (!NET_StringToAdr (cls.servername, &adr))
	{
		Com_Printf ("Bad server address\n");
		cls.connect_time = 0;
		return;
	}
	if (adr.port == 0)
		adr.port = BigShort (PORT_SERVER);

	port = Cvar_VariableValue ("qport");
	userinfo_modified = false;

	Netchan_OutOfBandPrint (NS_CLIENT, adr, "connect %i %i %i \"%s\"\n",
		PROTOCOL_VERSION, port, cls.challenge, Cvar_Userinfo() );
}

/*
=================
CL_CheckForResend
Resend a connect message if the last one has timed out
=================
*/
void SCR_SetConnecting(qboolean bON);
void CL_CheckForResend (void)
{
	netadr_t	adr;

	// if the local server is running and we aren't
	// then connect
	if (cls.state == ca_disconnected && Com_ServerState() )
	{
		cls.state = ca_connecting;
		strncpy (cls.servername, "localhost", sizeof(cls.servername)-1);
		// we don't need a challenge on the localhost
		CL_SendConnectPacket ();
		return;
//		cls.connect_time = -99999;	// CL_CheckForResend() will fire immediately
	}
	// resend if we haven't gotten a reply yet
	if (cls.state != ca_connecting)
		return;
	if (cls.realtime - cls.connect_time < 3000)
		return;

	if (!NET_StringToAdr (cls.servername, &adr))
	{
		Com_Printf ("Bad server address\n");
		cls.state = ca_disconnected;
		return;
	}
	if (adr.port == 0)
		adr.port = BigShort (PORT_SERVER);

	cls.connect_time = cls.realtime;	// for retransmit requests

	SCR_SetConnecting(TRUE);
	Com_Printf ("Connecting to %s...\n", cls.servername);

	Netchan_OutOfBandPrint (NS_CLIENT, adr, "getchallenge\n");
}
/*
================
CL_Connect_f
================
*/
void CL_Connect_f (void)
{
	char	*server;
	if (GetArgc() != 2)
	{
		Com_Printf ("usage: connect <server>\n");
		return;	
	}
	
	if (Com_ServerState ())
	{	// if running a local server, kill it and reissue
		SV_Shutdown (va("Server quit\n", msg), false, SV_UNLOAD_DLLS);
	}
	else
	{
		CL_Disconnect ();
	}
	server = GetArgv (1);
	NET_Config (true);		// allow remote
	CL_Disconnect ();
	cls.state = ca_connecting;
	SCR_BeginLoadingPlaque();
	strncpy (cls.servername, server, sizeof(cls.servername)-1);
	cls.connect_time = -99999;	// CL_CheckForResend() will fire immediately
}
/*
=====================
CL_Rcon_f
  Send the rest of the command line over as
  an unconnected command.
=====================
*/
void CL_Rcon_f (void)
{
	char	message[1024];
	int		i;
	netadr_t	to;
	if (!rcon_client_password->string)
	{
		Com_Printf ("You must set 'rcon_password' before\n"
					"issuing an rcon command.\n");
		return;
	}
	message[0] = (char)255;
	message[1] = (char)255;
	message[2] = (char)255;
	message[3] = (char)255;
	message[4] = 0;

	NET_Config (true);		// allow remote
	strcat (message, "rcon ");
	strcat (message, rcon_client_password->string);
	strcat (message, " ");
	for (i=1 ; i<GetArgc() ; i++)
	{
		strcat (message, GetArgv(i));
		strcat (message, " ");
	}
	if (cls.state >= ca_connected)
		to = cls.netchan.remote_address;
	else
	{
		if (!strlen(rcon_address->string))
		{
			Com_Printf ("You must either be connected,\n"
						"or set the 'rcon_address' cvar\n"
						"to issue rcon commands\n");
			return;
		}
		NET_StringToAdr (rcon_address->string, &to);
		if (to.port == 0)
			to.port = BigShort (PORT_SERVER);
	}
	
	NET_SendPacket (NS_CLIENT, strlen(message)+1, message, to);
}

//beamSeg_t	*beam_AllocSeg (void);
//beam_t	*beam_AllocBeam (void);
void CL_ManagerSetState(void)
{
	return;
/*
    cl.ClientEntityManager = new CClientEntityManager;
    // Client side CClientEntityManager instance, network messaging and useful client-side
    // functions
    cl.ClientEntityManager->NetClient->Com_Printf = Com_Printf;
    // cl.ClientEntityManager.NetClient.MultiCast = MultiCast;
    cl.ClientEntityManager->NetClient->WriteByte = MSG_WriteByte;
    cl.ClientEntityManager->NetClient->WriteShort = MSG_WriteShort;
    cl.ClientEntityManager->NetClient->WriteLong = MSG_WriteLong;
    cl.ClientEntityManager->NetClient->WriteFloat = MSG_WriteFloat;
    cl.ClientEntityManager->NetClient->WriteString = MSG_WriteString;
    // cl.ClientEntityManager->NetClient->WritePosition = MSG_WritePosistion;
    cl.ClientEntityManager->NetClient->WriteDir = MSG_WriteDir;
    cl.ClientEntityManager->NetClient->WriteAngle = MSG_WriteAngle;
    cl.ClientEntityManager->NetClient->ReadChar = MSG_ReadChar;
    cl.ClientEntityManager->NetClient->ReadByte = MSG_ReadByte;
    cl.ClientEntityManager->NetClient->ReadShort = MSG_ReadShort;
    cl.ClientEntityManager->NetClient->ReadLong = MSG_ReadLong;
    cl.ClientEntityManager->NetClient->ReadFloat = MSG_ReadFloat;
    cl.ClientEntityManager->NetClient->ReadString = MSG_ReadString;
    cl.ClientEntityManager->NetClient->ReadStringLine = MSG_ReadStringLine;
    cl.ClientEntityManager->NetClient->ReadCoord = MSG_ReadCoord;
    cl.ClientEntityManager->NetClient->ReadPos = MSG_ReadPos;
    cl.ClientEntityManager->NetClient->ReadAngle = MSG_ReadAngle;
    cl.ClientEntityManager->NetClient->ReadAngle16 = MSG_ReadAngle16;
    cl.ClientEntityManager->NetClient->ReadDir = MSG_ReadDir;
    // Give the manager access to the client state
    cl.ClientEntityManager->NetClient->V_AddEntity = V_AddEntity;
    cl.ClientEntityManager->NetClient->V_AddLight = V_AddLight;
    cl.ClientEntityManager->NetClient->CL_ModelIndex = CL_ModelIndex;
    cl.ClientEntityManager->NetClient->CL_SpawnParticle = CL_SpawnParticle;
    cl.ClientEntityManager->NetClient->beam_AllocBeam = beam_AllocBeam;
    cl.ClientEntityManager->NetClient->beam_AllocSeg = beam_AllocSeg;
    cl.ClientEntityManager->ClientStatic = &cls;
    cl.ClientEntityManager->re = &re;
    cl.ClientEntityManager->NetClient->V_AddParticle = V_AddParticle;
    cl.ClientEntityManager->NetClient->V_AddParticleRGB = V_AddParticleRGB;
    cl.ClientEntityManager->NetClient->V_AddComParticle = V_AddComParticle;
    if(cl_cspeeds->value)
        cl.ClientEntityManager->show_client_frame_speeds = 1;
    else
        cl.ClientEntityManager->show_client_frame_speeds = 0;
    cl.ClientEntityManager->NetClient->S_RegisterSound = S_RegisterSound;
    cl.ClientEntityManager->NetClient->S_StartSound = S_StartSound;
    cl.ClientEntityManager->NetClient->S_StartSoundQuake2 = S_StartSoundQuake2;
    cl.ClientEntityManager->SetClientState(&cl);
    cl.ClientEntityManager->client_entities = cl_entities;
*/
}

/*
=====================
CL_ClearState
// SCG[11/16/99]: called when the level starts and when disconnecting from a server
=====================
*/
void CL_ClearState (void)
{
	S_StopAllSounds ();
	CL_ClearEffects ();
	CL_ClearTEnts ();
//    if(cl.ClientEntityManager)
//        delete cl.ClientEntityManager;
// wipe the entire cl structure
	unsigned int nFlags = 0;

	if( cl.refdef.rdflags & RDF_NO_ICONS )
	{
		nFlags = cl.refdef.rdflags & RDF_NO_ICONS;
	}

	memset (&cl, 0, sizeof(cl));
	memset (&cl_entities, 0, sizeof(cl_entities));

	cl.refdef.rdflags = nFlags;

	SZ_Clear (&cls.netchan.message);
    CL_ManagerSetState();
    CL_InitDispatchFunctionList();

	// SCG[11/16/99]: Clear the kickview shit
	memset(&g_KickView, 0, sizeof(g_KickView));

}

/*
=====================
CL_Disconnect

Goes from a connected state to full screen console state
Sends a disconnect message to the server
This is also called on Com_Error, so it shouldn't cause any errors
=====================
*/
void CL_Disconnect (void)
{
	byte	final[32];

	if (cls.state == ca_disconnected)
		return;

	if (cl_timedemo && cl_timedemo->value)
	{
		int	time;
		
		time = Sys_Milliseconds () - cl.timedemo_start;
		if (time > 0)
			Com_Printf ("%i frames, %3.1f seconds: %3.1f fps\n", cl.timedemo_frames,
			time/1000.0, cl.timedemo_frames*1000.0 / time);
	}

	CIN_Stop( NULL );
	
	cl.refdef.blend[0] = 0;
	cl.refdef.blend[1] = 0;
	cl.refdef.blend[2] = 0; 
	re.CinematicSetPalette(NULL);

	M_ForceMenuOff ();

	cls.connect_time = 0;

// SCG[2/17/00]: 	SCR_StopCinematic ();

	if (cls.demorecording)
		CL_Stop_f ();

	// send a disconnect message to the server
	final[0] = clc_stringcmd;
	strcpy ((char *)final+1, "disconnect");
	Netchan_Transmit (&cls.netchan, strlen((const char *)final), final);
	Netchan_Transmit (&cls.netchan, strlen((const char *)final), final);
	Netchan_Transmit (&cls.netchan, strlen((const char *)final), final);

	CL_ClearState ();

	cls.state = ca_disconnected;

#ifdef	LOCKED_DEMO
	Cbuf_AddText ("d1\n");
#else
	//	Nelno:	start demo loop if invulnerable.dat is found
	if (_access ("./data/invulnerable.dat", 0) != -1)
		Cbuf_AddText ("d1\n");
#ifdef	BOOT_MENU


/*
2.12 -- this is causing the main menu to pop up after starting a server for the second time.
	else
		Cbuf_AddText ("menu_main\n");
*/


#endif
#endif
}

void CL_Disconnect_f (void)
{
	// SCG[11/29/99]: Make sure cinematic stops when we disconnect
	if( CIN_Running() == 1 )
	{
		CIN_Stop( NULL );
	}

	Com_Error (ERR_DROP, "Disconnected from server");
}


void CL_UnloadAll_f(void)
{
	if (cls.state != ca_disconnected)
	{
		Com_Printf("\nYou must be disconnected to unload the resources.\n");
		return;
	}

	// dump all of the rendering resources
	re.PurgeResources(RESOURCE_EPISODE);

	// kill sound resources
	S_BeginRegistration();
	S_EndRegistration(0);

	// free bsp
	Mem_Free_Tag(MEM_TAG_BSP);

	Com_Printf("Unloaded resources.\n");
}



/*
====================
CL_Packet_f

packet <destination> <contents>

Contents allows \n escape character
====================
*/
void CL_Packet_f (void)
{
	char	send[2048];
	int		i, l;
	char	*in, *out;
	netadr_t	adr;

	if (GetArgc() != 3)
	{
		Com_Printf ("packet <destination> <contents>\n");
		return;
	}

	NET_Config (true);		// allow remote

	if (!NET_StringToAdr (GetArgv(1), &adr))
	{
		Com_Printf ("Bad address\n");
		return;
	}
	if (!adr.port)
		adr.port = BigShort (PORT_SERVER);

	in = GetArgv(2);
	out = send+4;
	send[0] = send[1] = send[2] = send[3] = (char)0xff;

	l = strlen (in);
	for (i=0 ; i<l ; i++)
	{
// Encompass MarkMa 040599
#ifdef	JPN	// JPN
		if(IsDBCSLeadByte(in[i]))	{
			*out++ = in[i++];
			*out++ = in[i];
		}
		else
#endif	// JPN Encompass MarkMa 040599
		{ // JPN
			if (in[i] == '\\' && in[i+1] == 'n')
			{
				*out++ = '\n';
				i++;
			}
			else
				*out++ = in[i];
		} //JPN Encompass MarkMa 040599
	}
	*out = 0;

	NET_SendPacket (NS_CLIENT, out-send, send, adr);
}

/*
=================
CL_Changing_f

Just sent as a hint to the client that they should
drop to full console
=================
*/
extern qboolean no_draw_plaque;
extern int scr_draw_progress;
void CL_Changing_f (void)
{
	if (GetArgc() > 1)
	{
		if ((scr_draw_progress == 0) && !stricmp(GetArgv(1),"noplaque"))
			no_draw_plaque = true;
	}
	cls.state = ca_connected;	// not active anymore, but not disconnected

	SCR_BeginLoadingPlaque ();

	Com_Printf ("\nChanging map...\n");

	CIN_Stop( NULL );
}

/*
=================
CL_Reconnect_f

The server is changing levels
=================
*/
void CL_Reconnect_f (void)
{
	S_StopAllSounds ();
	Com_Printf ("reconnecting...\n");
	cls.state = ca_connected;
	MSG_WriteChar (&cls.netchan.message, clc_stringcmd);
	MSG_WriteString (&cls.netchan.message, "new");		
}

/*
=================
CL_ParseStatusMessage

Handle a reply from a ping
=================
*/
void DKM_AddLocalServer(char *server_name, netadr_t addr);
void DKM_ResetLocalServerList();

void CL_ParseStatusMessage (void)
{
	char	*s;

	s = MSG_ReadString(&net_message);

	Com_DPrintf ("%s\n", s);

    //new menu callback.
    DKM_AddLocalServer(s, net_from);

    //old menu junk
	M_AddToServerList (net_from, s);
}


//GAMESPY
void ListCallBack(GServerList serverlist, int msg, void *instance, void *param1, void *param2)
{
	GServer server;
	if (msg == LIST_PROGRESS)
	{
		server = (GServer)param1;
		Com_DPrintf ("%s:%d [%d] %s %d/%d %s\n",ServerGetAddress(server),ServerGetQueryPort(server), ServerGetPing(server),ServerGetStringValue(server, "hostname","(NONE)"), ServerGetIntValue(server,"numplayers",0), ServerGetIntValue(server,"maxplayers",0), ServerGetStringValue(server,"mapname","(NO MAP)"));
	} /*else

		printf("%d \n",msg); */

}

void CL_PingNetServers_f(void)
{
	char goa_secret_key[256];
    GServerList serverlist;

	// WAW[11/23/99]: Keep secret key secret.
	goa_secret_key[0] = 'f';
	goa_secret_key[1] = 'l';
	goa_secret_key[2] = '8';
	goa_secret_key[3] = 'a';
	goa_secret_key[4] = 'Y';
	goa_secret_key[5] = '7';
	goa_secret_key[6] = '\0';
	serverlist = ServerListNew("daikatana","daikatana",goa_secret_key,10,ListCallBack,GCALLBACK_FUNCTION,NULL);
    //serverlist = ServerListNew("unreal","daikatana","fl8aY7",10,ListCallBack,GCALLBACK_FUNCTION,NULL);
    ServerListUpdate(serverlist,0);
    //ServerListLANUpdate(serverlist,false,PORT_SERVER,PORT_SERVER,10);
    ServerListFree(serverlist);
}

void CL_PingServer(char *adrstring)
{
	NET_Config (true);		// allow remote
	netadr_t	adr;
	if (!adrstring || !adrstring[0])
		return;
	Com_DPrintf ("pinging %s...\n", adrstring);
	if (!NET_StringToAdr (adrstring, &adr))
	{
		Com_DPrintf ("Bad address: %s\n", adrstring);
		return;
	}
	if (!adr.port)
		adr.port = BigShort(PORT_SERVER);
	Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", PROTOCOL_VERSION));
}
/*
=================
CL_PingServers_f
=================
*/
void CL_PingServers_f (void)
{
	int			i;
	netadr_t	adr;
	char		name[32];
	char		*adrstring;
	cvar_t		*noudp;
//	cvar_t		*noipx;
	NET_Config (true);		// allow remote
	// send a broadcast packet
	Com_DPrintf ("pinging broadcast...\n");

    DKM_ResetLocalServerList();

	noudp = Cvar_Get ("noudp", "0", CVAR_NOSET);
	if (!noudp->value)
	{
		adr.type = NA_BROADCAST;
		adr.port = BigShort(PORT_SERVER);
		Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", PROTOCOL_VERSION));
	}
/*
	noipx = Cvar_Get ("noipx", "1", CVAR_NOSET);
	if (!noipx->value)
	{
		adr.type = NA_BROADCAST_IPX;
		adr.port = BigShort(PORT_SERVER);
		Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", PROTOCOL_VERSION));
	}
*/
	// send a packet to each address book entry
	for (i=0 ; i<16 ; i++)
	{
		Com_sprintf (name, sizeof(name), "adr%i", i);
		adrstring = Cvar_VariableString (name);
		if (!adrstring || !adrstring[0])
			continue;
		Com_DPrintf ("pinging %s...\n", adrstring);
		if (!NET_StringToAdr (adrstring, &adr))
		{
			Com_DPrintf ("Bad address: %s\n", adrstring);
			continue;
		}
		if (!adr.port)
			adr.port = BigShort(PORT_SERVER);
		Netchan_OutOfBandPrint (NS_CLIENT, adr, va("info %i", PROTOCOL_VERSION));
	}
}

/*
=================
CL_Skins_f

Load or download any custom player skins and models
=================
*/
void CL_Skins_f (void)
{
	int		i;

	for (i=0 ; i<MAX_CLIENTS ; i++)
	{
		if (!cl.configstrings[CS_PLAYERSKINS+i][0])
			continue;
		Com_DPrintf ("client %i: %s\n", i, cl.configstrings[CS_PLAYERSKINS+i]); 
		SCR_UpdateScreen ();
		Sys_SendKeyEvents ();	// pump message loop
		CL_ParseClientinfo (i);
	}
}

/*
=================
CL_ConnectionlessPacket
Responses to broadcasts, etc
=================
*/
void CL_ConnectionlessPacket (void)
{
	char	*s;
	char	*c;
	
	MSG_BeginReading (&net_message);
	MSG_ReadLong (&net_message);	// skip the -1
	s = MSG_ReadStringLine (&net_message);
	Cmd_TokenizeString (s, false);
	c = GetArgv(0);
	Com_DPrintf ("%s: %s\n", NET_AdrToString (net_from), c);
	// server connection
	if (!strcmp(c, "client_connect"))
	{
		if (cls.state == ca_connected)
		{
			Com_DPrintf ("Dup connect received.  Ignored.\n");
			return;
		}
		Netchan_Setup (NS_CLIENT, &cls.netchan, net_from, cls.quakePort);
		MSG_WriteChar (&cls.netchan.message, clc_stringcmd);
		MSG_WriteString (&cls.netchan.message, "new");	
		cls.state = ca_connected;
		Com_DPrintf("client_connect packet received from server.\n");
		return;
	}
	// server responding to a status broadcast
	if (!strcmp(c, "info"))
	{
		CL_ParseStatusMessage ();
		return;
	}
	// remote command from gui front end
	if (!strcmp(c, "cmd"))
	{
		if (!NET_IsLocalAddress(net_from))
		{
			Com_DPrintf ("Command packet from remote host.  Ignored.\n");
			return;
		}
		Sys_AppActivate ();
		s = MSG_ReadString (&net_message);
		Cbuf_AddText (s);
		Cbuf_AddText ("\n");
		return;
	}
	// print command from somewhere
	if (!strcmp(c, "print"))
	{
		s = MSG_ReadString (&net_message);
        Com_Printf (s);
        if(strstr(s, "Server is full."))
        {
            SCR_EndLoadingPlaque ();
            CL_Disconnect();
            Cbuf_AddText ("menu_main\n");
        }
		else if (strstr(s,"Server is version"))		// make sure the client doesn't just keep going....
		{
            SCR_EndLoadingPlaque ();
            CL_Disconnect();
            Cbuf_AddText ("menu_main\n");
		}
        return;
	}
	// ping from somewhere
	if (!strcmp(c, "ping"))
	{
		Netchan_OutOfBandPrint (NS_CLIENT, net_from, "ack");
		return;
	}

	// challenge from the server we are connecting to
	if (!strcmp(c, "challenge"))
	{
		cls.challenge = atoi(GetArgv(1));
		CL_SendConnectPacket ();
		return;
	}

	// echo request from server
	if (!strcmp(c, "echo"))
	{
		Netchan_OutOfBandPrint (NS_CLIENT, net_from, "%s", GetArgv(1) );
		return;
	}

	Com_DPrintf ("Unknown command.\n");
}

/*
=================
CL_DumpPackets

A vain attempt to help bad TCP stacks that cause problems
when they overflow
=================
*/
void CL_DumpPackets (void)
{
	while (NET_GetPacket (NS_CLIENT, &net_from, &net_message))
	{
		Com_DPrintf ("dumnping a packet\n");
	}
}

/*
=================
CL_ReadPackets
=================
*/
void CL_ReadPackets (void)
{
	while (NET_GetPacket (NS_CLIENT, &net_from, &net_message))
	{
//	Com_Printf ("packet\n");
		//
		// remote command packet
		//
		if (*(int *)net_message.data == -1)
		{
			CL_ConnectionlessPacket ();
			continue;
		}
		if (cls.state == ca_disconnected || cls.state == ca_connecting)
			continue;		// dump it if not connected
		if (net_message.cursize < 8)
		{
			Com_DPrintf ("%s: Runt packet\n",NET_AdrToString(net_from));
			continue;
		}
		//
		// packet from server
		//
		if (!NET_CompareAdr (net_from, cls.netchan.remote_address))
		{
			Com_DPrintf ("%s:sequenced packet without connection\n"
				,NET_AdrToString(net_from));
			continue;
		}
		if (!Netchan_Process(&cls.netchan, &net_message))
			continue;		// wasn't accepted for some reason
		CL_ParseServerMessage ();
	}
	//
	// check timeout
	//
	if (cls.state >= ca_connected
	 && cls.realtime - cls.netchan.last_received > cl_timeout->value*1000)
	{
		if (++cl.timeoutcount > 5)	// timeoutcount saves debugger
		{
			Com_Printf ("\nServer connection timed out.\n");
			CL_Disconnect ();
			return;
		}
	}
	else
		cl.timeoutcount = 0;
}

//=============================================================================

/*
==============
CL_Userinfo_f
==============
*/
void CL_Userinfo_f (void)
{
	Com_Printf ("User info settings:\n");
	Info_Print (Cvar_Userinfo());
}

/*
=================
CL_Snd_Restart_f

Restart the sound subsystem so it can pick up
new parameters and flush all sounds
=================
*/
void CL_Snd_Restart_f (void)
{
	S_Shutdown ();
	S_Init (cl_hwnd);
	CL_RegisterSounds ();
}


/*
=================
CL_Precache_f

The server will send this command right
before allowing the client into the server
=================
*/

void CL_GetSidekickModelIndicies();

void CL_Precache_f (void)
{
	unsigned	map_checksum;		// for detecting cheater maps
	char		palette[MAX_QPATH];
	char		*value;
	char		mapname [MAX_QPATH];
	int			time, files;
	long		bytes;

	// start timing
	bytes = FS_GetBytesRead();
	files = FS_GetFilesOpened();
	time = Sys_Milliseconds();

	//	Nelno:	free all malloc'd memory associated with client
	Mem_Free_Tag(MEM_TAG_CLIENT);	// new memmgr

	sidekickcommand_Init();      // init vars
	inventory_init ();
	CL_Boost_Icons_Init();
	CL_GetSidekickModelIndicies();
	SCR_InitVars();
	SCR_DirtyScreen();
	///////////////////////////////////////////////////////////////////////////
	//	Load the map, but only if this is a stand-alone client, ie. the
	//	server has not already loaded the map 
	/////////////////////////////////////////			//////////////////////////////////

	CM_LoadMap (cl.configstrings[CS_MODELS+1], true, &map_checksum); // this holds the entire map name!!

	if (map_checksum != atoi(cl.configstrings[CS_MAPCHECKSUM]))
		Com_Error (ERR_DROP, "Local map version differs from server: %i != '%s'\n",
		map_checksum, cl.configstrings[CS_MAPCHECKSUM]);

	///////////////////////////////////////////////////////////////////////////
	//	get palette directory from epairs
	///////////////////////////////////////////////////////////////////////////


	// SCG[9/24/99]: moved from server side to client so this will work
	// SCG[9/24/99]: in multiplayer mode...
	value = CM_KeyValue( map_epairs, "loadscreen" );

	if( value )
	{
		Cvar_ForceSet( "cl_loadscreen", value );
	}
	else
	{
	  	Cvar_ForceSet( "cl_loadscreen", "gen" );
	}

	char *pString = cl.configstrings [CS_MODELS + 1];
	pString += 5;

	value = CM_KeyValue (map_epairs, "palette");
	if (value)
		strcpy (palette, value);
	else
	{
		//	no palette entry in worldspawn, so use map name
		Com_sprintf( palette, MAX_QPATH, "%s", pString );
		palette [strlen (palette) - 4] = 0;
	}

	///////////////////////////////////////////////////////////////////////////
	//	get episode number from epairs
	///////////////////////////////////////////////////////////////////////////

	cl_last_episode_num = cl_episode_num;

	value = CM_KeyValue (map_epairs, "episode");
	if (value)
		cl_episode_num = atoi (value);
	else
	{
		pString = cl.configstrings [CS_MODELS + 1];
		Com_sprintf( mapname, MAX_QPATH, "%s", pString );
		cl_episode_num = ( int )mapname[1] - 48;
	}

	if (cl_episode_num < 1 || cl_episode_num > 9)
	{
		Com_Printf ("Map has invalid episode number!!\n");
		cl_episode_num = 1;
	}

	if (strcmp (current_palettedir, palette))
	{
		re.SetPaletteDir (palette);
		strcpy (current_palettedir, palette);
	}

	CL_RegisterSounds ();
	CL_PrepRefresh ();

	// show loading info
	load_info.total_time = Sys_Milliseconds() - time;
	load_info.total_files = FS_GetFilesOpened() - files;
	load_info.total_bytes = FS_GetBytesRead() - bytes;
	CL_Loading_Info_f();
}

void	CL_WriteConfig_f (void);

/*
=================
CL_InitLocal
=================
*/
void CL_InitLocal (void)
{
	cls.state = ca_disconnected;
	cls.realtime = Sys_Milliseconds ();

	CL_InitInput ();

	adr0 = Cvar_Get( "adr0", "", CVAR_ARCHIVE );
	adr1 = Cvar_Get( "adr1", "", CVAR_ARCHIVE );
	adr2 = Cvar_Get( "adr2", "", CVAR_ARCHIVE );
	adr3 = Cvar_Get( "adr3", "", CVAR_ARCHIVE );
	adr4 = Cvar_Get( "adr4", "", CVAR_ARCHIVE );
	adr5 = Cvar_Get( "adr5", "", CVAR_ARCHIVE );
	adr6 = Cvar_Get( "adr6", "", CVAR_ARCHIVE );
	adr7 = Cvar_Get( "adr7", "", CVAR_ARCHIVE );
	adr8 = Cvar_Get( "adr8", "", CVAR_ARCHIVE );

//
// register our variables
//
	cl_stereo_separation = Cvar_Get( "cl_stereo_separation", "0.4", CVAR_ARCHIVE );
	cl_stereo = Cvar_Get( "cl_stereo", "0", 0 );

	cl_add_blend = Cvar_Get ("cl_blend", "1", 0);
	cl_add_lights = Cvar_Get ("cl_lights", "1", 0);
	cl_add_particles = Cvar_Get ("cl_particles", "1", 0);
	cl_add_entities = Cvar_Get ("cl_entities", "1", 0);
	cl_gun = Cvar_Get ("cl_gun", "1", 0);
//	cl_footsteps = Cvar_Get ("cl_footsteps", "1", 0);
	cl_noskins = Cvar_Get ("cl_noskins", "0", 0);
	cl_autoskins = Cvar_Get ("cl_autoskins", "0", 0);
	cl_predict = Cvar_Get ("cl_predict", "1", 0);

	cl_minfps = Cvar_Get ("cl_minfps", "5", 0);	
	cl_maxfps = Cvar_Get ("cl_maxfps", "90", 0);

	cl_upspeed = Cvar_Get ("cl_upspeed", "200", 0);
	cl_forwardspeed = Cvar_Get ("cl_forwardspeed", "200", 0);
	cl_sidespeed = Cvar_Get ("cl_sidespeed", "200", 0);
	cl_yawspeed = Cvar_Get ("cl_yawspeed", "140", 0);
	cl_pitchspeed = Cvar_Get ("cl_pitchspeed", "150", 0);
	cl_anglespeedkey = Cvar_Get ("cl_anglespeedkey", "1.5", 0);

	cl_add_flares = Cvar_Get ("cl_flares", "1", 0);				//  Shawn:  Added for Daikatana
	cl_add_comparticles = Cvar_Get ("cl_particles", "1", 0);	//  Shawn:  Added for Daikatana

	cl_run = Cvar_Get ("cl_run", "0", CVAR_ARCHIVE);
	freelook = Cvar_Get( "freelook", "0", CVAR_ARCHIVE );
	lookspring = Cvar_Get ("lookspring", "0", CVAR_ARCHIVE);
	lookstrafe = Cvar_Get ("lookstrafe", "0", CVAR_ARCHIVE);
	sensitivity = Cvar_Get ("sensitivity", "3", CVAR_ARCHIVE);

  weapon_visible = Cvar_Get ("weapon_visible", "1", CVAR_ARCHIVE); // 2.24  dsn
	subtitles = Cvar_Get ("subtitles", "0", CVAR_ARCHIVE); 

	m_pitch = Cvar_Get ("m_pitch", "0.022", CVAR_ARCHIVE);
	m_yaw = Cvar_Get ("m_yaw", "0.022", CVAR_ARCHIVE);       // dsn 12.14, changed to CVAR_ARCHIVE to save out
	m_forward = Cvar_Get ("m_forward", "1", 0);
	m_side = Cvar_Get ("m_side", "1", 0);

	cl_shownet = Cvar_Get ("cl_shownet", "0", 0);
    cl_cspeeds = Cvar_Get ("c_speeds", "0", 0);
	cl_showmiss = Cvar_Get ("cl_showmiss", "0", 0);
	cl_showclamp = Cvar_Get ("showclamp", "0", 0);
	cl_timeout = Cvar_Get ("cl_timeout", "1000", 0);
	cl_paused = Cvar_Get ("paused", "0", 0);
	cl_timedemo = Cvar_Get ("timedemo", "0", 0);

	rcon_client_password = Cvar_Get ("rcon_password", "", 0);
	rcon_address = Cvar_Get ("rcon_address", "", 0);

	cl_lightlevel = Cvar_Get ("r_lightlevel", "0", 0);
	
	cl_antiskate = Cvar_Get ("cl_antiskate", "0", 0);

	//
	// userinfo
	//
	info_password = Cvar_Get ("password", "", CVAR_USERINFO);
	name = Cvar_Get ("name", "Hiro", CVAR_USERINFO | CVAR_ARCHIVE);
    StripInvalidChars(name->string,16);

	team = Cvar_Get ("team", "", CVAR_USERINFO | CVAR_ARCHIVE);
    StripInvalidChars(team->string,16);


//	skin = Cvar_Get ("skin", "male/grunt", CVAR_USERINFO | CVAR_ARCHIVE);
	rate = Cvar_Get ("rate", "25000", CVAR_USERINFO | CVAR_ARCHIVE);
	msg = Cvar_Get ("msg", "1", CVAR_USERINFO | CVAR_ARCHIVE);
	//hand = Cvar_Get ("hand", "0", CVAR_USERINFO | CVAR_ARCHIVE);
	fov = Cvar_Get ("fov", "90", CVAR_USERINFO | CVAR_ARCHIVE);
    
	modelname = Cvar_Get ("modelname", DEFAULT_CHARACTER_MODEL, CVAR_USERINFO | CVAR_ARCHIVE | CVAR_NOSET );
	skinname  = Cvar_Get ("skinname", "", CVAR_USERINFO | CVAR_ARCHIVE);
	skincolor = Cvar_Get ("skincolor", "0", CVAR_USERINFO | CVAR_ARCHIVE);
	character = Cvar_Get ("character", "0", CVAR_USERINFO | CVAR_ARCHIVE);

	if (character->value < 0 || character->value > 2) // boundary check
    Cvar_Set("character", "0");


	dm_footsteps = Cvar_Get("dm_footsteps", "1", CVAR_LATCH);
	deathmatch   = Cvar_Get("deathmatch", "0", CVAR_SERVERINFO|CVAR_LATCH);
	Cvar_Get("dm_taunts","1",CVAR_USERINFO | CVAR_ARCHIVE );	// cek[5-10-00]: disable dm commentary
	Cvar_Get("unlimited_saves","0", CVAR_ARCHIVE );
	cl_dmhints = Cvar_Get("cl_dmhints","1",CVAR_ARCHIVE);

	//	kick idle asses so Nelno can update
	cl_checkkick = Cvar_Get ("cl_checkkick", "1", CVAR_ARCHIVE);
	cl_kick_time_warning = Cvar_Get ("cl_kick_time_warning", "10", CVAR_ARCHIVE);

	//cl_loadscreen = Cvar_Get ("cl_loadscreen", "loadscreen", 0);
	cl_loadscreen = Cvar_Get ("cl_loadscreen", "con", 0);

	cl_autoweaponswitch = Cvar_Get("cl_autowweaponswitch","0",CVAR_USERINFO|CVAR_ARCHIVE);
	play_intro = Cvar_Get("play_intro","1",CVAR_ARCHIVE);
	//
	// register our commands
	//

    //GAMESPY
    Cmd_AddCommand ("slist2", CL_PingNetServers_f);

	Cmd_AddCommand ("cmd", CL_ForwardToServer_f);
	Cmd_AddCommand ("pause", CL_Pause_f);
	Cmd_AddCommand ("pingservers", CL_PingServers_f);
	Cmd_AddCommand ("slist", CL_PingServers_f);
	Cmd_AddCommand ("skins", CL_Skins_f);

	Cmd_AddCommand ("userinfo", CL_Userinfo_f);
	Cmd_AddCommand ("snd_restart", CL_Snd_Restart_f);

	Cmd_AddCommand ("changing", CL_Changing_f);
	Cmd_AddCommand ("disconnect", CL_Disconnect_f);
	Cmd_AddCommand ("record", CL_Record_f);
	Cmd_AddCommand ("stop", CL_Stop_f);

	Cmd_AddCommand ("quit", CL_Quit_f);
	Cmd_AddCommand ("sample", CL_SampleModel_f);

	Cmd_AddCommand ("connect", CL_Connect_f);
	Cmd_AddCommand ("reconnect", CL_Reconnect_f);

	Cmd_AddCommand ("rcon", CL_Rcon_f);
// 	Cmd_AddCommand ("packet", CL_Packet_f); // this is dangerous to leave in

	Cmd_AddCommand ("setenv", CL_Setenv_f );

	Cmd_AddCommand ("precache", CL_Precache_f);

	//	Nelno: write configuration out
	Cmd_AddCommand ("cl_writeconfig", CL_WriteConfig_f);

	// unload everyting
	Cmd_AddCommand("unloadall", CL_UnloadAll_f);

	Cmd_AddCommand("load_info",CL_Loading_Info_f);

	//
	// forward to server commands
	//

	//	Nelno:	removed these -- dumb way of doing it.
	//	they are now just registered directly from DLLs
	//	Odd: removing these causes Cmd_RemoveCommand to fail...
/*
	Cmd_AddCommand ("wave", NULL);
	Cmd_AddCommand ("inven", NULL);
	Cmd_AddCommand ("kill", NULL);
	Cmd_AddCommand ("use", NULL);
	Cmd_AddCommand ("drop", NULL);
	Cmd_AddCommand ("say", NULL);
	Cmd_AddCommand ("say_team", NULL);
	Cmd_AddCommand ("info", NULL);
	Cmd_AddCommand ("prog", NULL);
	Cmd_AddCommand ("give", NULL);
	Cmd_AddCommand ("god", NULL);
	Cmd_AddCommand ("notarget", NULL);
	Cmd_AddCommand ("noclip", NULL);
	Cmd_AddCommand ("fov", NULL);
	Cmd_AddCommand ("invuse", NULL);
	Cmd_AddCommand ("invprev", NULL);
	Cmd_AddCommand ("invnext", NULL);
	Cmd_AddCommand ("invdrop", NULL);
	Cmd_AddCommand ("weapnext", NULL);
	Cmd_AddCommand ("weapprev", NULL);
	Cmd_AddCommand ("gameversion", NULL);
*/
}

void CL_WriteConfiguration (char *name);
void CL_SaveLastConfiguration(void);

void CL_FindDefaultConfig()
{
	cvar_t *cl_curconfig = Cvar_Get("currentconfig","current",0);

	// run daikatana.cfg and default_keys.cfg
	Cbuf_AddText("exec daikatana.cfg\nexec default_keys.cfg");
	Cbuf_Execute();

	// set our config to 'current' and save it out.
	Cvar_FullSet("currentconfig","current",0);
}

short CL_LoadLastConfiguration(void)
{
	FILE *f = NULL;
	char curCfgPath[MAX_QPATH];

	// first, see if the curcfg.cfg file exists.  If not, find the default configs.
	Com_sprintf (curCfgPath, sizeof(curCfgPath),"%s/curcfg.cfg", FS_ConfigDir());
	f = fopen (curCfgPath, "r");
	if (!f)
	{
		CL_FindDefaultConfig();
		return 0;
	}
	fclose(f);f = NULL;

	// load the string from the curcfg.cfg file
    Cbuf_AddText ("exec curcfg.cfg\n");
	Cbuf_Execute ();

	cvar_t *cl_curconfig = Cvar_Get("currentconfig","current",0);
	// does the file specified as current exist? if not, find the default configs
	Com_sprintf (curCfgPath, sizeof(curCfgPath),"%s/%s.cfg", FS_ConfigDir(),cl_curconfig->string);
	f = fopen (curCfgPath, "r");
	if (!f)
	{
		CL_FindDefaultConfig();
		return 0;
	}
	fclose(f);f = NULL;

	// whew.  the current config file exists!  go ahead and exec it.
	Com_sprintf (curCfgPath, sizeof(curCfgPath),"exec \"%s.cfg\"\n", cl_curconfig->string);
    Cbuf_AddText (curCfgPath);
	Cbuf_Execute ();
	Com_Printf ("Executed %s.cfg.\n", cl_curconfig->string);
	
	return 1;
}

/*
===============
CL_SaveLastConfiguration

Saves the current used configuration to the curcfg.txt file
===============
*/
void CL_SaveLastConfiguration(void)
{
	FILE *f;
	char curCfgPath[MAX_QPATH];
	char	buffer[1024];
	cvar_t *cl_curconfig = Cvar_Get("currentconfig","current",0);

	Com_sprintf (curCfgPath, sizeof(curCfgPath),"%s/curcfg.cfg", FS_ConfigDir());
	f = fopen (curCfgPath, "w");
	if (!f)
	{
		Com_Printf("Couldn't write %s\n",curCfgPath);
		return;
	}
	
	fprintf (f, "// This config file is used by the config selector.  Do not change this file.\n\n");
	Com_sprintf (buffer, sizeof(buffer), "set %s \"%s\"\n", cl_curconfig->name, cl_curconfig->string);

	// don't save the .cfg stuff.
	char *buf = strstr(buffer,".");
	if (buf)
		buf = NULL;

	fprintf (f, "%s", buffer);
	fclose (f);
}

/*
===============
CL_WriteConfiguration

Writes key bindings and archived cvars to config.cfg  (search keywords: write config save config)
===============
*/
void CL_WriteConfiguration (char *name)
{
	FILE	*f;
	char	path[MAX_QPATH];

	if (cls.state == ca_uninitialized)
		return;

	//	Nelno:	write to username.cfg instead of config.cfg
	//	Com_sprintf (path, sizeof(path),"%s/config.cfg", FS_Gamedir());
//	Com_sprintf (path, sizeof(path),"%s/%s.cfg", FS_Gamedir (), name);
	Com_sprintf (path, sizeof(path),"%s/%s.cfg", FS_ConfigDir (), name);
	f = fopen (path, "w");
	if (!f)
	{
		Com_Printf ("Couldn't write %s.cfg.\n", name);
		return;
	}

	if (stricmp(name,"daikatana") || stricmp(name,"default_keys"))
		fprintf (f, "// This config is automatically created.  Use AUTOEXEC.CFG for\n// adding custom settings.\n\n");

	Key_WriteBindings (f);
	fclose (f);

	Cvar_WriteVariables (path);
}

///////////////////////////////////////////////////////////////////////////////
//	CL_WriteConfig_f
//
///////////////////////////////////////////////////////////////////////////////

void	CL_WriteConfig_f (void)
{
	if (GetArgc () == 1)
	{
		CL_WriteConfiguration("current");//Cvar_VariableString("currentconfig"));
//		CL_WriteConfiguration (dk_userName); 

//		Com_Printf ("Wrote configuration file %s/%s.cfg\n", FS_Gamedir (), dk_userName);
		Com_Printf ("Wrote configuration file %s/%s.cfg\n", FS_ConfigDir (), Cvar_VariableString("currentconfig"));
	}
	else if (GetArgc () == 2)
	{
		CL_WriteConfiguration (GetArgv (1)); 

//		Com_Printf ("Wrote configuration file %s/%s.cfg\n", FS_Gamedir (), GetArgv (1));
		Com_Printf ("Wrote configuration file %s/%s.cfg\n", FS_ConfigDir (), GetArgv (1));
	}
	else
		Com_Printf ("USAGE: CL_WriteConfig [name]");
}

/*
==================
CL_FixCvarCheats

==================
*/

typedef struct
{
	char	*name;
	char	*value;
	cvar_t	*var;
} cheatvar_t;

cheatvar_t	cheatvars[] = {
	{"timescale",		"1",	NULL},
	{"timedemo",		"0",	NULL},
	{"r_drawworld",		"1",	NULL},
	{"cl_testlights",	"0",	NULL},
	{"r_fullbright",	"0",	NULL},
	{"r_drawflat",		"0",	NULL},
	{"fixedtime",		"0",	NULL},
	{"gl_polylines",	"0",	NULL},
	{"gl_drawflat",		"0",	NULL},
//	{"gl_modulate",		"2",	NULL},
	{"r_fullbright",	"0",	NULL},
	{NULL,				NULL,	NULL}
};

int		numcheatvars;

void CL_FixCvarCheats (void)
{
	int			i;
	cheatvar_t	*var;

//	if ( !strcmp(cl.configstrings[CS_MAXCLIENTS], "1") 
//		|| !cl.configstrings[CS_MAXCLIENTS][0] )
//		return;		// single player can cheat

	// find all the cvars if we haven't done it yet
	if (!numcheatvars)
	{
		while (cheatvars[numcheatvars].name)
		{
			cheatvars[numcheatvars].var = Cvar_ForceSet( cheatvars[numcheatvars].name,
					cheatvars[numcheatvars].value );
			numcheatvars++;
		}
	}

	// make sure they are all set to the proper values
	for (i=0, var = cheatvars ; i<numcheatvars ; i++, var++)
	{
		if ( strcmp (var->var->string, var->value) )
		{
			Cvar_Set (var->name, var->value);
		}
	}
}

//============================================================================

/*
==================
CL_SendCommand

==================
*/
void CL_SendCommand (void)
{
	// get new key events
	Sys_SendKeyEvents ();

	// allow mice or other external controllers to add commands
	IN_Commands ();

	// process console commands
	Cbuf_Execute ();

	// send intentions now
	CL_SendCmd ();

	// resend a connection request if necessary
	CL_CheckForResend ();
}

///////////////////////////////////////////////////////////////////////////////
//	CL_FixModelClip
//
//	fix-up cl.model_clip list when cmodel_dynamic.c reloads a map
///////////////////////////////////////////////////////////////////////////////

void	CL_FixModelClip (void)
{
	int			i;
	char		name [MAX_QPATH];

	Com_Printf ("Fixing up model_clip...\n");

	//	Nelno:	register models from precached names
	for (i = 1; i < MAX_MODELS && cl.configstrings [CS_MODELS + i][0]; i++)
	{
		strcpy (name, cl.configstrings[CS_MODELS+i]);
		name[37] = 0;	// never go beyond one line

		SCR_UpdateScreen ();
		Sys_SendKeyEvents ();	// pump message loop

		//	should already be registered...
		cl.model_draw[i] = re.RegisterModel (cl.configstrings[CS_MODELS+i], RESOURCE_EPISODE);
		
		if (name[0] == '*')
			cl.model_clip[i] = CM_InlineModel (cl.configstrings[CS_MODELS+i]);
		else
			cl.model_clip[i] = NULL;
	}

	bspModel.fixModelClip = false;
}

char *CL_MapName()
{
	if( dedicated->value )
	{
		return NULL;
	}

	return cl.configstrings[0];
}

/*
==================
CL_Frame

==================
*/
void CL_Frame (int msec)
{
	static int	extratime = 0;
	static int  lasttimecalled;

	extern char *szSVMapName;
	extern char *szCLMapName;

	if (dedicated->value)
		return;

	extratime += msec;

	if (!cl_timedemo->value)
	{
		if (cls.state == ca_connected && extratime < 100)
			return;			// don't flood packets out while connecting
		if (extratime < 1000/cl_maxfps->value)
			return;			// framerate is too high
	}

	// let the mouse activate or deactivate
	IN_Frame ();

	// decide the simulation time
	cls.frametime = extratime/1000.0;
	cl.time += extratime;
	cls.realtime = curtime;

	extratime = 0;
	if (cls.frametime > (1.0 / cl_minfps->value))
		cls.frametime = (1.0 / cl_minfps->value);

//#ifndef	_DEBUG
	if (cl_checkkick->value)
	{
		cl_check_kick_time += cls.frametime;
		if (cl_check_kick_time >= 10.0)
		{
			if (_access ("./kick.dat", 0) != -1)
			{
				float defWarningTime = cl_kick_time_warning->value;
				cl_kick_time += cls.frametime;

				Com_Printf ("Exiting in %.2f second(s)\n",defWarningTime-cl_kick_time);

				if (cl_kick_time > defWarningTime)
				{
#ifdef _WIN32					
					WinExec( "./PleaseWait.exe", 0 ); 
#endif
					Com_Quit ();
					return;
				}
			}
			else
			{
				cl_check_kick_time = 0.0f;
				cl_kick_time = 0.0f;
			}
		}
	}
//#endif

	// if in the debugger last frame, don't timeout
	if (msec > 5000)
		cls.netchan.last_received = Sys_Milliseconds ();

	// fetch results from server
	CL_ReadPackets ();

	// send a new command message to the server
	CL_SendCommand ();

	// predict all unacknowledged movements
	CL_PredictMovement ();

	// allow rendering DLL change
	VID_CheckChanges ();
	
	if (!cl.refresh_prepped && cls.state == ca_active)
		CL_PrepRefresh ();

	if( cl.frame.playerstate.rdflags & RDF_NOCHEATING )
	{
		CL_FixCvarCheats();
	}

	if( ( szSVMapName != NULL ) && ( szCLMapName != NULL ) )
	{
		if( strcmp( szSVMapName, szCLMapName ) )
		{
			cl.frame.playerstate.rdflags |= RDF_HALTALLDRAWING;
		}
		else
		{
			cl.frame.playerstate.rdflags &= ~RDF_HALTALLDRAWING;
		}
	}

	// update the screen
	if (host_speeds->value)
		time_before_ref = Sys_Milliseconds ();
	SCR_UpdateScreen ();
	if (host_speeds->value)
		time_after_ref = Sys_Milliseconds ();

	// -- Update audio

	CVector vZero( 0, 0, 0 );
    if (DKM_InMenu() == true)
	{
		S_Update(vZero,vZero,vZero,vZero);  // if in the menu, don't play any 3d sounds
    }
	else
	{
		S_Update (cl.refdef.vieworg, cl.v_forward, cl.v_right, cl.v_up);
	}

	// cek[12-3-99] no cdaudio
//	CDAudio_Update();

	// advance local effects for next frame
	CL_RunDLights ();
	CL_RunLightStyles ();
// SCG[2/17/00]: 	SCR_RunCinematic ();
	SCR_RunConsole ();

// SCG[8/10/99]: Not used
//	CL_RunFlares ();		//  Shawn: Added for Daikatana
	CL_RunFog ();		//  Shawn: Added for Daikatana

	cls.framecount++;

	if ( log_stats->value )
	{
		if ( cls.state == ca_active )
		{
			if ( !lasttimecalled )
			{
				lasttimecalled = Sys_Milliseconds();
				if ( log_stats_file )
					fprintf( log_stats_file, "0\n" );
			}
			else
			{
				int now = Sys_Milliseconds();

				if ( log_stats_file )
					fprintf( log_stats_file, "%d\n", now - lasttimecalled );
				lasttimecalled = now;
			}
		}
	}

    GCE_ClientFrame();
/*
    if(cl.ClientEntityManager)
    {
        if(cl_cspeeds->value)
            cl.ClientEntityManager->show_client_frame_speeds = 1;
        else
            cl.ClientEntityManager->show_client_frame_speeds = 0;
    }
*/    
}


//============================================================================

///////////////////////////////////////////////////////////////////////////////
//	CL_InitConsole
///////////////////////////////////////////////////////////////////////////////

void	CL_InitConsole (void)
{
	if (!dedicated || !dedicated->value)
		return;

	Con_Init ();
}




int LoadAudioEngineDLL (const char* pszAudioEngineDLL);
void FreeAudioEngineDLL ();
/*
====================
CL_Init
====================
*/
void CL_Init (void)
{
   //#ifdef AE_USE_ENGINE_DLL
    int bOk ;
//#endif
	if (dedicated->value)
		return;		// nothing running on the client

//	clhr_ReadSurfaceNames ();		// get rid of this shit SGC 3-19-99

	cl_episode_num = 0;
	cl_last_episode_num = 0;
	map_epairs = NULL;
	cl_mapname [0] = 0x00;

	// all archived variables will now be loaded
//    cls.key_dest = key_console;

	Con_Init ();	

	// force everyone to use the correct DLL for now
	Cvar_Set("AudioEngDLL",szAudioEngineDLL);

	// there is only one audio engine now
	cvarAudioEngDLL = Cvar_Get("AudioEngDLL",szAudioEngineDLL,CVAR_ARCHIVE);

    // -- MUST be called before VID_Init ()
    bOk = LoadAudioEngineDLL (cvarAudioEngDLL->string) ;
    
    if (!bOk)
    {
    	Sys_Error ("Failed to load Audio Engine DLL:\n  %s\nIt either wasn't found or is invalid.", szAudioEngineDLL) ;
    }
//#endif

	FS_ExecAutoexec ();
	Cbuf_Execute ();

	//cek[12-7-99] execute the user's config, too
	short bOK = CL_LoadLastConfiguration();
	VID_Init ();

    S_Init (cl_hwnd);	// sound must be initialized after window is created since we need the hwnd

	V_Init ();
	
	net_message.data = net_message_buffer;
	net_message.maxsize = sizeof(net_message_buffer);

	M_Init ();	
	
	SCR_Init ();
	cls.disable_screen = true;	// don't draw yet

	// cek[12-3-99] no CDAudio dammit!
//	CDAudio_Init ();
	CL_InitLocal ();
	IN_Init ();

	DKM_Init();

	// do a save here if loadconfiguration returned 0
	if (!bOK)
	{
		cvar_t *cl_curconfig = Cvar_Get("currentconfig","current",0);
		CL_WriteConfiguration(cl_curconfig->string);
		CL_SaveLastConfiguration();	
	}

//	Cbuf_AddText ("exec autoexec.cfg\n");
/*
	FS_ExecAutoexec ();
	Cbuf_Execute ();

	//cek[12-7-99] execute the user's config, too
	CL_LoadLastConfiguration();
*/
    GCE_Init();

	sb_InitScoreboard ();

    // LOGGING, startup client log
//    g_cvarLogServer = Cvar_Get("log_server", "", CVAR_ARCHIVE);
//    UDP_Log(6, "Client Logging Enabled");

}


/*
===============
CL_Shutdown

FIXME: this is a callback from Sys_Quit and Com_Error.  It would be better
to run quit through here before the final handoff to the sys code.
===============
*/
void CL_Shutdown(void)
{
	static qboolean isdown = false;
	
	if (isdown)
	{
		printf ("recursive shutdown\n");
		return;
	}
	isdown = true;

#ifdef	LOCKED_DEMO
	CL_WriteConfiguration ("default"); 
#else
//	CL_WriteConfiguration (dk_userName); 
	CL_WriteConfiguration("current");//Cvar_VariableString("currentconfig"));
#endif

	CDAudio_Shutdown ();

//#ifdef AE_USE_ENGINE_DLL
	if (S_Shutdown)
		S_Shutdown();
    FreeAudioEngineDLL () ;
//#else
//	S_Shutdown();
//#endif

	CIN_Reset();

	IN_Shutdown ();
	VID_Shutdown();

	DKM_GameShutdown();
}

void CL_SampleModel_f()
{
	char*		surface;
	char		model_name[256], buf[256];
	int			frame_start, length, i, model_index;
	CVector		hp;
	entity_t	ent;
	FILE*		f;

	if (GetArgc() < 5)
	{
		Com_Printf("Usage:  sample [model_name] [surface] [frame_start] [length]\n");
		return;
	}

	// get all the args
// SCG[1/16/00]: 	sprintf (model_name, "models/%s.dkm", GetArgv(1));
	Com_sprintf (model_name, sizeof( model_name ), "models/%s.dkm", GetArgv(1));
	surface = GetArgv(2);
	frame_start = atoi(GetArgv(3));
	length = atoi(GetArgv(4));

	// get model index
	model_index = CL_ModelIndex(model_name);
	
	if (!model_index)
	{
		Com_Printf("%s not loaded.\n",model_name);
		return;
	}

    ent.model = cl.model_draw[model_index];
	ent.angles.Set(0,0,0); 
	ent.origin.Set(0,0,0);
	ent.backlerp = 0.0f;

	f = fopen("c:/sample.txt","w+");

	for (i=frame_start;i<frame_start+length;i++)
	{
		re.GetModelHardpoint(surface,i,i,ent,hp);
		// x and y are flipped
		Com_DPrintf("Frame: %d  Offset(%.2f,%.2f,%.2f)\n",i-frame_start,hp.y,hp.x,hp.z);
//		sprintf(buf,"(%2d,CVector(%3.2f,%3.2f,%3.2f))\n",i-frame_start,hp.y,hp.x,hp.z);
		Com_sprintf(buf,sizeof(buf), "(%2d,CVector(%3.2f,%3.2f,%3.2f))\n",i-frame_start,hp.y,hp.x,hp.z);
		// write it out
		fwrite(buf,strlen(buf),1,f);		
	}

	fclose(f);
}


qboolean CL_ValidateSelectorPos(int selection);
void CL_ParseXPLevel(void)
// retrieve svc_xplevel server message
{
	xplevelup_mode      = true;
	xplevelup_selection = 0;
	xplevelup_timer     = cl.time + XPLEVELUP_BLINKTIME;
	xplevelup_blink     = false;
	
	for (int i = 0; i < MAX_SKILLS; i++)
	{
		if (CL_ValidateSelectorPos(i))
		{
			xplevelup_selection = i;
			break;
		}
	}

	skill_hud.direction = 1; // start fade-in of skill bar, if not already showing
	
	S_StartLocalSound("global/skill_start.wav");  // play activate sound
}



void CL_InformClientDeath(void)
// server informs client of client/sidekick death
{
	cl_death_gib = MSG_ReadByte(&net_message); // retrieve total damage

	// don't do this all over again!
	if (cl_gameover_time)
		return;

	// reset last map name so when the map reloads soit won't be treated as
	// a level transition
	Cvar_ForceSet( "cl_lastmapname", "none" ); 

	CL_Boost_Icons_Init(); // reset power boosts if active

	cl_gameover_time  = cl.time + 2000; // time before fadein gameover graphic
	cl_gameover_alpha = 0.0f;
}

void CL_InformClientTheEnd(void)
{
	// don't do this all over again!
	if (cl_theend_time)
		return;

	cl_theend_time  = cl.time;
	cl_theend_alpha = 0.0f;
}


void CL_Loading_Info_f()
{
	Com_DPrintf("Loading Stats:\n");
	Com_DPrintf("Sounds:  %4i files %10lu bytes in %3.2fs\n",load_info.sound_files,load_info.sound_bytes,( float ) load_info.sound_time / 1000.0 );
	Com_DPrintf("BSP:     %4i files %10lu bytes in %3.2fs\n",load_info.bsp_files,load_info.bsp_bytes,( float ) load_info.bsp_time / 1000.0 );
	Com_DPrintf("Models:  %4i files %10lu bytes in %3.2fs\n",load_info.models_files,load_info.models_bytes,( float ) load_info.models_time / 1000.0 );
	Com_DPrintf("Images:  %4i files %10lu bytes in %3.2fs\n",load_info.images_files,load_info.images_bytes,( float ) load_info.images_time / 1000.0 );
	Com_DPrintf("Clients: %4i files %10lu bytes in %3.2fs\n",load_info.clients_files,load_info.clients_bytes,( float ) load_info.clients_time / 1000.0 );
	Com_DPrintf("Total:	 %4i files %10lu bytes in %3.2fs\n",load_info.total_files,load_info.total_bytes,( float ) load_info.total_time / 1000.0 );
}
