// cl_parse.c  -- parse a message received from the server
#include    <windows.h>
//#include    <mmsystem.h>
#include	  "client.h"
#include	  "dk_scoreboard.h"
#include	  "cl_inventory.h"
#include    "cl_sidekick.h"
#include    "cl_selector.h"
#include    "cl_icons.h"
//#include    "ClientEntityManager.h"
#include    "p_user.h"


#ifdef _DEBUG
char *svc_strings[256] =
{
	"svc_bad",
		
	"svc_muzzleflash",
	"svc_muzzlflash2",
	"svc_temp_entity",
	"svc_layout",
	"svc_inventory",
	"svc_onscreen_icon",
	"svc_clientdeath",
	"svc_sidekick_update",
	//"svc_sidekick_command",
	"svc_selector",
	"svc_weapon_icon",
	"svc_xplevel",
	"svc_savegem",
	"svc_clientrespawn",
	
	"svc_nop",
	"svc_disconnect",
	"svc_reconnect",
	"svc_sound",
	"svc_print",
	"svc_stufftext",
	"svc_serverdata",
	"svc_configstring",
	"svc_spawnbaseline",	
	"svc_centerprint",
	"svc_download",
	"svc_playerinfo",
	"svc_packetentities",
	"svc_deltapacketentities",
	"svc_frame",
	"svc_effect_notify",			//  Shawn:  Added for Daikatana
	"svc_setclientmodels",			//	Nelno
	"svc_syncentity",				//	Nelno:	weapon shot syncing
	"svc_scoreboard",				//	Nelno:	deathmatch scoreboard message
	"svc_cliententity",             //  Logic:  client gets brains, see libclient for info
	"svc_crosshair",				// SCG[10/4/99]: For enabling/disabling the crosshair
	"svc_end_of_game",				// SCG[1/18/00]: 
	"svc_ammo_count_update",
	"svc_sidekick_weapon_status",
	"svc_start_music",
	"svc_start_mp3",
	"svc_stop_mp3",
	"svc_reverb_preset",
	"svc_start_stream",
	"svc_client_string"
};
#endif

extern cvar_t	*fs_gamedirvar;

//=============================================================================

#ifdef _DEBUG
void SHOWNET(char *s)
{
	if (cl_shownet->value>=2)
	{
		Com_DPrintf ("%3i:%s\n", net_message.readcount-1, s);
	}
}
#endif /* DEBUG */

/*
===============
CL_CheckOrDownloadFile

  Returns true if the file exists, otherwise it attempts
  to start a download from the server.
  ===============
*/
qboolean	CL_CheckOrDownloadFile (char *filename)
{
	if (strstr (filename, ".."))
	{
		Com_Printf ("Refusing to download a path with ..\n");
		return true;
	}
	
	if (FS_LoadFile (filename, NULL) != -1)
	{	// it exists, no need to download
		return true;
	}
	
	strcpy (cls.downloadname, filename);
	Com_Printf ("Downloading %s\n", cls.downloadname);
	
	// download to a temp name, and only rename
	// to the real name when done, so if interrupted
	// a runt file wont be left
	COM_StripExtension (cls.downloadname, cls.downloadtempname);
	strcat (cls.downloadtempname, ".tmp");
	
	MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
	MSG_WriteString (&cls.netchan.message,
		va("download %s", cls.downloadname));
	
	cls.downloadnumber++;
	
	return false;
}


/*
======================
CL_RegisterSounds
======================
*/
void DKM_PrecacheSounds(void);
void CL_RegisterSounds (void)
{
	int		i;
	int		time, files;
	long	bytes;
	
	// time the function
	bytes = FS_GetBytesRead();
	files = FS_GetFilesOpened();	
	time = Sys_Milliseconds();
	
	S_BeginRegistration ();

	DKM_PrecacheSounds();
	S_RegisterSound( "menus/button_002.wav" );
	S_RegisterSound( "global/skill_start.wav" );

	CL_RegisterTEntSounds ();
	
	for (i = 1;i < MAX_SOUNDS;i++)
	{
		if (!cl.configstrings[CS_SOUNDS+i][0])
			break;
		
		cl.sound_precache[i] = S_RegisterSound (cl.configstrings[CS_SOUNDS+i]);
		Sys_SendKeyEvents ();	// pump message loop
	}
	
//	if (cl.ClientEntityManager)
//		cl.ClientEntityManager->SoundRegistration();
	
	S_EndRegistration (0);
	
	// end timing
	load_info.sound_time = Sys_Milliseconds() - time;	
	load_info.sound_bytes = FS_GetBytesRead() - bytes;
	load_info.sound_files = FS_GetFilesOpened() - files;
}


/*
======================
CL_RequestNextDownload
======================
*/
void CL_RequestNextDownload (void)
{
}

/*
=====================
CL_ParseDownload

  A download message has been received from the server
  =====================
*/
void CL_ParseDownload (void)
{
	int		size, percent;
	char	name[MAX_OSPATH];
	int		r;
	
	// read the data
	size = MSG_ReadShort (&net_message);
	percent = MSG_ReadByte (&net_message);
	if (size == -1)
	{
		Com_Printf ("File not found.\n");
		if (cls.download)
		{
			Com_Printf ("cls.download shouldn't have been set\n");
			fclose (cls.download);
			cls.download = NULL;
		}
		CL_RequestNextDownload ();
		return;
	}
	
	// open the file if not opened yet
	if (!cls.download)
	{
		Com_sprintf (name, sizeof(name), "%s/%s", FS_Gamedir(), cls.downloadtempname);
		
		FS_CreatePath (name);
		
		cls.download = fopen (name, "wb");
		if (!cls.download)
		{
			net_message.readcount += size;
			Com_Printf ("Failed to open %s\n", cls.downloadtempname);
			CL_RequestNextDownload ();
			return;
		}
	}
	
	fwrite (net_message.data + net_message.readcount, 1, size, cls.download);
	net_message.readcount += size;
	
	if (percent != 100)
	{
		// request next block
		Com_Printf (".");
		if (10*(percent/10) != cls.downloadpercent)
		{
			cls.downloadpercent = 10*(percent/10);
			Com_Printf ("%i%%", cls.downloadpercent);
		}
		MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
		SZ_Print (&cls.netchan.message, "nextdl");
	}
	else
	{
		char	oldn[MAX_OSPATH];
		char	newn[MAX_OSPATH];
		
		Com_Printf ("100%%\n");
		
		fclose (cls.download);
		
		// rename the temp file to it's final name
		Com_sprintf (oldn, sizeof(oldn), "%s/%s", FS_Gamedir(), cls.downloadtempname);
		Com_sprintf (newn, sizeof(newn), "%s/%s", FS_Gamedir(), cls.downloadname);
		r = rename (oldn, newn);
		if (r)
			Com_Printf ("failed to rename.\n");
		
		cls.download = NULL;
		cls.downloadpercent = 0;
		
		// get another file if needed
		
		CL_RequestNextDownload ();
	}
}


/*
=====================================================================

  SERVER CONNECTING MESSAGES
  
	=====================================================================
*/

/*
==================
CL_ParseServerData
==================
*/
void CL_ParseServerData (void)
{
	char	*str;
	int		i;
	
	Com_DPrintf ("Serverdata packet received.\n");
	//
	// wipe the client_state_t struct
	//
	CL_ClearState ();
	cls.state = ca_connected;
	
	// parse protocol version number
	i = MSG_ReadLong (&net_message);
	cls.serverProtocol = i;
	
	// BIG HACK to let demos from release work with the 3.0x patch!!!
	if (Com_ServerState() && PROTOCOL_VERSION == 31)
	{
	}
	else if (i != PROTOCOL_VERSION)
		Com_Error (ERR_DROP,"Server returned version %i, not %i", i, PROTOCOL_VERSION);
	
	cl.servercount = MSG_ReadLong (&net_message);
	cl.attractloop = MSG_ReadByte (&net_message);
	
	// game directory
	str = MSG_ReadString (&net_message);
	strncpy (cl.gamedir, str, sizeof(cl.gamedir)-1);
	
	// set gamedir
	if ((*str && (!fs_gamedirvar->string || !*fs_gamedirvar->string || strcmp(fs_gamedirvar->string, str))) || (!*str && (fs_gamedirvar->string || *fs_gamedirvar->string)))
		Cvar_Set("game", str);
	
	// parse player entity number
	cl.playernum = MSG_ReadShort (&net_message);
	
	// get the full level name
	str = MSG_ReadString (&net_message);
	
	if (cl.playernum == -1)
	{	// playing a cinematic or showing a pic, not a level
// SCG[2/17/00]: 		SCR_PlayCinematic (str);
	}
	else
	{
		//		// seperate the printfs so the server message can have a color
		//		Com_Printf("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");
		//		Com_Printf ("%c%s\n", 2, str);
		
		// need to prep refresh at next oportunity
		cl.refresh_prepped = false;
	}
}

/*
==================
CL_ParseBaseline
==================
*/
void CL_ParseBaseline (void)
{
	entity_state_t	*es;
	int				bits, bits2;
	int				newnum;
	entity_state_t	nullstate;
	
	memset (&nullstate, 0, sizeof(nullstate));
	
	newnum = CL_ParseEntityBits ((unsigned int *)&bits, (unsigned int *)&bits2);
	es = &cl_entities[newnum].baseline;
	
	CL_ParseDelta (&nullstate, es, newnum, bits, bits2);
}


/*
================
CL_LoadClientinfo

  ================
*/
void CL_LoadClientinfo (clientinfo_t *ci, char *s)
{
	char *temp;
	char buf[512];
    int   i;
	
	//	get player's name out of client info string
	//	player's name is everything up to the first backslash
	strcpy (ci->name, s);

// Encompass MarkMa 040599
#ifdef	JPN	// JPN
	char *s1 = s;
	temp = NULL;
	while (*s1)	{
		if( *s1 == '\\')	{
			temp = s1;
			break;
		}
		if(IsDBCSLeadByte(*s1))		s1++;
		s1++;
	}
#else	// JPN
	temp = strstr (s, "\\");
#endif	// JPN
// Encompass MarkMa 040599

	if (temp)
	{
		ci->name [temp - s] = 0x00;
		s = temp + 1;
	}
	
	// no skins on, or no skin specified?
	if (cl_noskins->value || *s == 0x00)
	{
		ci->skin = NULL;
	}
	else
	{
// Encompass MarkMa 040599
#ifdef	JPN	// JPN
        // parse model name
        temp = s;
        i = 0;

		while (*temp)	{
			if( *temp == '\\')		break;
			if(IsDBCSLeadByte(*temp))	{
			    i++;
				temp++;
			}
	        i++;
			temp++;
		}

        *(s + i) = 0x00;
        ci->model = re.RegisterModel (s, RESOURCE_EPISODE);
#else	// JPN
		// parse model name
		strncpy(buf, s,512);
		buf[511] = 0;
		temp = buf;
		i = 0;
		
		while (temp && *temp != '\\')  // advance pointer to next divider
		{
			i++;
			temp++;
		}
		
		buf[i] = 0x00;
		ci->model = re.RegisterModel (buf, RESOURCE_EPISODE);
		
		s = temp + 1;  // advance pointer past slash
		
		// // parse skin name
		strcpy(buf, s);
		temp = buf;
		i = 0;
		
		while (temp && *temp != '\\')  // advance pointer to next divider
		{
			i++;
			temp++;
		}
		
		buf[i] = 0x00;
		ci->skin = re.RegisterSkin(buf, RESOURCE_EPISODE);
#endif	// JPN
// Encompass MarkMa 040599
		
		s = temp + 1;  // advance pointer past slash
		
		// last token in string, just grab
		ci->character = atoi(s);
		
		if (ci->character < 0 || ci->character > 2)
			ci->character = 0; // Hrio default
	}
}   


/*
================
CL_ParseClientinfo

  Load the skin, icon, and model for a client
  ================
*/
void CL_ParseClientinfo (int player)
{
	char         *s;
	clientinfo_t *ci;
	
	s = cl.configstrings[player+CS_PLAYERSKINS];
	
	_ASSERTE(strlen(s)); // assert if 's' is blank
	
	ci = &cl.clientinfo[player];
	
	CL_LoadClientinfo (ci, s);
	
	if (player == cl.playernum) // base client info?
	{
		memcpy(&cl.baseclientinfo,&cl.clientinfo[player],sizeof(clientinfo_t));  // store base client info
	}
}


/*
================
CL_ParseConfigString
================
*/
void CL_ParseConfigString (void)
{
	int		i;
	char	*s;
	
	i = MSG_ReadShort (&net_message);
	if (i < 0 || i >= MAX_CONFIGSTRINGS)
		Com_Error (ERR_DROP, "configstring > MAX_CONFIGSTRINGS");
	s = MSG_ReadString(&net_message);
	Com_sprintf( cl.configstrings[i], MAX_CONFIGSTR_LEN, "%s", s );
//	strcpy (cl.configstrings[i], s);
	
	// do something apropriate 
	
	if (i >= CS_LIGHTS && i < CS_LIGHTS+MAX_LIGHTSTYLES)
		CL_SetLightstyle (i - CS_LIGHTS);
	else if (i == CS_CDTRACK)
	{
		if (cl.refresh_prepped)
			CDAudio_Play (atoi(cl.configstrings[CS_CDTRACK]), true);
	}
	else if (i >= CS_MODELS && i < CS_MODELS+MAX_MODELS)
	{
		if (cl.refresh_prepped)
		{
			cl.model_draw[i-CS_MODELS] = re.RegisterModel (cl.configstrings[i], RESOURCE_EPISODE);
			if (cl.configstrings[i][0] == '*')
				cl.model_clip[i-CS_MODELS] = CM_InlineModel (cl.configstrings[i]);
			else
				cl.model_clip[i-CS_MODELS] = NULL;
		}
	}
	else if (i >= CS_SOUNDS && i < CS_SOUNDS+MAX_MODELS)
	{
		if (cl.refresh_prepped)
			cl.sound_precache[i-CS_SOUNDS] = S_RegisterSound (cl.configstrings[i]);
	}
	else if (i >= CS_IMAGES && i < CS_IMAGES+MAX_MODELS)
	{
		if (cl.refresh_prepped)
			//==================================
			// consolidation change: SCG 3-11-99
			cl.image_precache[i-CS_IMAGES] = re.RegisterPic (cl.configstrings[i], NULL, NULL, RESOURCE_EPISODE);
		// consolidation change: SCG 3-11-99
		//==================================
	}
	else if (i >= CS_PLAYERSKINS && i < CS_PLAYERSKINS+MAX_CLIENTS)
	{
		if (cl.refresh_prepped)
			CL_ParseClientinfo (i-CS_PLAYERSKINS);
	}
	
}


void CL_ParseCrosshairMessage( void )
{
	byte	show_crosshair;
	
	show_crosshair = MSG_ReadByte( &net_message );
	
	if (show_crosshair)
		Cvar_Set("cv_crosshair_enabled", "1" );
	else
		Cvar_Set("cv_crosshair_enabled", "0" );
}


///////////////////////////////////////////////////////////////////////////////
//	CL_ParseClientModels
///////////////////////////////////////////////////////////////////////////////

/*
void	CL_ParseClientModels (void)
{
cl.num_player_head = MSG_ReadShort (&net_message);
cl.num_player_torso = MSG_ReadShort (&net_message);
cl.num_player_legs = MSG_ReadShort (&net_message);
cl.num_player_weapon = MSG_ReadShort (&net_message);
}
*/

///////////////////////////////////////////////////////////////////////////////
//	CL_ParseSyncEntity
//
//	searches throught the list for the client entity with a matching number
//	and updates its data from the message
//
//	if the entity does not exist, then it is added to the entities for the
//	current frame
////////////////////////////////////////////////////////////////////////////////

void	CL_ParseSyncEntity (void)
{
	entity_state_t	*sync_state;	//	the state we're going to
	entity_state_t	*parse_state;	//	the state we're coming from
	centity_t		*client_ent;	//	holds baseline, prev and current states
	unsigned int	ent_num, ent_bits, ent_bits2;
	int				parse_num, index = 0;
	frame_t			*old_frame;
	
	//	FIXME:	is this the correct frame?
	old_frame = &cl.frame;
	
	//	read the entity bits and get the number
	ent_num = CL_ParseEntityBits (&ent_bits, &ent_bits2);
	
	parse_state = &cl_parse_entities [(old_frame->parse_entities + index) & (MAX_PARSE_ENTITIES - 1)];
	parse_num = parse_state->number;
	
	//	find the entity with the matching number
	while (parse_num != ent_num && index < old_frame->num_entities)
	{
		index++;
		
		parse_state = &cl_parse_entities [(old_frame->parse_entities + index) & (MAX_PARSE_ENTITIES - 1)];
		parse_num = parse_state->number;
	}
	
	client_ent = &cl_entities [ent_num];
	
	if (parse_num == ent_num)
	{
		//	entity is already here, so update it
		
		//	get the state of this entity, for this frame, in cl_parse_entities
		sync_state = &cl_parse_entities [(old_frame->parse_entities + ent_num) & (MAX_PARSE_ENTITIES - 1)];
		
		CL_ParseDelta (parse_state, sync_state, ent_num, ent_bits, ent_bits2);
	}
	else
	{
		//	entity wasn't in the last frame, so add it now
		//	deltaing it from the baseline
		
		//	allocate a new entity from the circular cl_parse_entities array
		sync_state = &cl_parse_entities [cl.parse_entities & (MAX_PARSE_ENTITIES-1)];
		cl.parse_entities++;
		
		//	add one to the number of entities in the frame
		old_frame->num_entities++;
		
		CL_ParseDelta (&cl_entities [ent_num].baseline, sync_state, ent_num, ent_bits, ent_bits2);
		
		// wasn't in last update, so initialize some things
		client_ent->trailcount = 1024;		// for diminishing rocket / grenade trails
		
		// duplicate the current state so lerping doesn't hurt anything
		client_ent->prev = *sync_state;
		client_ent->prev.origin = sync_state->old_origin;
		client_ent->lerp_origin = sync_state->old_origin;
		
		client_ent->serverframe = cl.frame.serverframe;
		client_ent->current = *sync_state;
	}
}

/*
=====================================================================

  ACTION MESSAGES
  
	=====================================================================
*/

/*
void  (*S_StartMusic) (const char* name, int chan);
void  (*S_StopMusic) (int chan);
void  (*S_StartMP3)(const char* name, int channel, float fvol, int play_count, qboolean bRestart );
void  (*S_ StopMP3)(int channel);

void SV_StartMusic(const char* name, int chan)
{
	if (!name)
		return;

	MSG_WriteByte (&sv.multicast, svc_start_music);
	MSG_WriteString (&sv.multicast, name);
	MSG_WriteShort (&sv.multicast, chan);
}

void SV_StartMP3( const char* name, int channel, float fvol, int play_count, qboolean bRestart )
{
	if (!name)
		return;

	MSG_WriteByte (&sv.multicast, svc_start_mp3);
	MSG_WriteString (&sv.multicast, name);
	MSG_WriteShort (&sv.multicast, channel);
	MSG_WriteFloat (&sv.multicast, fvol);
	MSG_WriteShort (&sv.multicast, play_count);
	MSG_WriteByte (&sv.multicast, bRestart);
}


void SV_StopMP3(int channel)
{
	MSG_WriteByte (&sv.multicast, svc_stop_mp3);
	MSG_WriteShort (&sv.multicast, channel);
}
void  SV_ReverbPreset (int nPresetID)
{
	MSG_WriteByte (&sv.multicast, svc_reverb_preset);
	MSG_WriteShort (&sv.multicast, nPresetID);
}

void  SV_StartStream (const char* name, float vol)
{
	if (!name)
		return;

	MSG_WriteByte (&sv.multicast, svc_start_stream);
	MSG_WriteString (&sv.multicast, name);
	MSG_WriteFloat (&sv.multicast, vol);
}
*/

void CL_ParseReverbPreset(void)
{
#ifdef _DEBUG
	SHOWNET("CL_ParseReverbPreset");
#endif /* DEBUG */

	int presetID = MSG_ReadShort(&net_message);
	S_ReverbPreset(presetID);
}

void CL_ParseStartStream(void)
{
#ifdef _DEBUG
	SHOWNET("CL_ParseStartStream");
#endif /* DEBUG */

	char *name = MSG_ReadString(&net_message);
	float vol = MSG_ReadFloat(&net_message);

	S_StartStream(name,vol);
}

void CL_ParseStartMusic(void)
{
#ifdef _DEBUG
	SHOWNET("CL_ParseStartMusic");
#endif /* DEBUG */

	char *name = MSG_ReadString(&net_message);
	int chan = MSG_ReadShort(&net_message);

	S_StartMusic(name,chan);
}

void CL_ParseStartMP3(void)
{
#ifdef _DEBUG
	SHOWNET("CL_ParseStartMP3");
#endif /* DEBUG */

	char *name = MSG_ReadString(&net_message);
	int chan = MSG_ReadShort(&net_message);
	float vol = MSG_ReadFloat(&net_message);
	int play_count = MSG_ReadShort(&net_message);
	qboolean bRestart = MSG_ReadByte(&net_message) ? TRUE : FALSE;

	S_StartMP3(name, chan, vol, play_count, bRestart);
}

void CL_ParseStopMP3(void)
{
#ifdef _DEBUG
	SHOWNET("CL_ParseStopMP3");
#endif /* DEBUG */

	int chan = MSG_ReadShort(&net_message);
	S_StopMP3(chan);
}

/*
==================
CL_ParseStartSoundPacket
==================
*/
void CL_ParseStartSoundPacket(void)
{
    CVector pos_v;
	CVector	pos;
    int 	channel, ent;
    int 	sound_num;
    float 	volume;
    float 	attenuation;  
	int		flags;
	float	ofs;
	float	dist_min, dist_max;
	
#ifdef _DEBUG
	SHOWNET("CL_ParseStartSoundPacket");
#endif /* DEBUG */
	
	
	flags = MSG_ReadByte (&net_message);
	// mdm99.07.01 - too many sounds
	//	sound_num = MSG_ReadByte (&net_message);
	sound_num = MSG_ReadShort(&net_message);
	
    if (flags & SND_VOLUME)
		volume = MSG_ReadByte (&net_message) / 255.0;
	else
		volume = DEFAULT_SOUND_PACKET_VOLUME;
	
    if (flags & SND_ATTENUATION)
        attenuation = MSG_ReadByte (&net_message) >> 6;
	else
		attenuation = 0; 
	
	if (flags & SND_DISTANCE)
	{
		dist_min = ((int)MSG_ReadByte(&net_message)) << 3;
		dist_max = ((int)MSG_ReadByte(&net_message)) << 5;
	}
	else
	{
		dist_min = ATTN_NORM_MIN;
		dist_max = ATTN_NORM_MAX;
	}
	
    if (flags & SND_OFFSET)
		ofs = MSG_ReadByte (&net_message) / 1000.0;
	else
		ofs = 0;
	
	if (flags & SND_ENT)
	{	// entity reletive
		channel = MSG_ReadShort(&net_message); 
		ent = channel>>3;
		if (ent > MAX_EDICTS)
			Com_Error (ERR_DROP,"CL_ParseStartSoundPacket: ent = %i", ent);
		
		channel &= 7;
	}
	else
	{
		ent = 0;
		channel = 0;
	}
	
	if (flags & SND_POS)
	{	// positioned in space
		MSG_ReadPos (&net_message, pos_v);
		
		pos = pos_v;
	}
	else	// use entity number
	{
		pos.Set(0,0,0);
	}
	
	if (!cl.sound_precache[sound_num])
		return;
	
	if (!attenuation)
	{
		S_StartSound(pos,ent,channel,cl.sound_precache[sound_num],volume,dist_min,dist_max);
	}
	else
	{
		S_StartSoundQuake2 (pos, ent, channel, cl.sound_precache[sound_num], volume, attenuation, ofs);
	}
}       

void CL_ParseSaveGem(void)
// interpret svc_savegem server message
{
	char buf[32];
	byte gem_activate;
	
//	Com_Printf("CL_ParseSaveGem()\n");
	
	gem_activate      = MSG_ReadByte(&net_message);  // 'using' the savegem?  true/false
	cl_savegem_count  = MSG_ReadByte(&net_message);  // store # of savegems collected
	
	if (gem_activate) // using the savegem?
	{
//		sprintf(buf,"menu_main_force %d\n", (int)DKMB_SAVEGAME);  // summon the main menu with save menu as first
		Com_sprintf(buf,sizeof(buf), "menu_main_force %d\n", (int)DKMB_SAVEGAME);  // summon the main menu with save menu as first
		Cbuf_AddText (buf); 
	}
}

void MoveToEnd(char **dst)
{
	char *findEnd = strchr(*dst,'\0');
	*dst = findEnd;
}

char *MoveToSpecifier(char **fmt, bool &replacedSpecifier)
{
	char findChar = '%';
	if (replacedSpecifier)
		findChar = '\0';

	char *findSpecifier = strchr(*fmt,findChar);
	if (findSpecifier)
	{
		*fmt = findSpecifier;

		// put the % back if it was replaced previously
		if (replacedSpecifier)
			*findSpecifier = '%';

		// look ahead for the next findSpecifier and replace it if necessary
		findSpecifier = strchr((*fmt+1),'%');
		if (findSpecifier)
		{
			*findSpecifier = '\0';
			replacedSpecifier = true;
		}
		else
			replacedSpecifier = false;
	}
	else
		*fmt = NULL;

	return *fmt;
}


char *CS_StringForResourceID(long str_list_index);

void CL_ParseClientString()
{	
	static char result[2048];
	static char fmt[2048];
	byte datatype = 0;
	byte printtype = 0;
	float msg_time = 0.0f;

	// set up some the vars
	bool replacedSpecifier = false;
	memset(fmt,0,sizeof(fmt));

	// read the format specifier (type and data)
	datatype = MSG_ReadByte (&net_message);
	switch(datatype)
	{
	case CS_data_str_resource:
		strcpy(fmt,CS_StringForResourceID(MSG_ReadLong(&net_message)));
		break;

	case CS_data_str_literal:
		strcpy(fmt,MSG_ReadString(&net_message));
		break;

	default:
		Com_Error (ERR_DROP,"CL_ParseClientString: Bad format specifier type");
		break;
	};

	// parse out the instructions
	byte parmCount = 0, do_continue = 0;
	int i = 0;
	do
	{
		// reset the result and the pointers
		memset(result,0,sizeof(result));
		char *pResult = result;
		char *pFmt = fmt;

		// read the parameter count
		parmCount = MSG_ReadByte (&net_message);

		if (!parmCount)
		{
			strcpy(pResult,pFmt);
		}
		else
		{
			// the first specifier.  Move to the %blah and copy the stuff to the result buffer
			MoveToSpecifier(&pFmt, replacedSpecifier);

			// copy and move any stuff before the first %
			if (pFmt != fmt)
			{
				strncpy(pResult, fmt, pFmt - fmt);
				MoveToEnd(&pResult);
			}

			// loop through the rest of the parameters
			for(i = 0; i < parmCount; i++)
			{
				// read the data type
				datatype = MSG_ReadByte (&net_message);

				// read the data and perform the sprintf into the result buffer
				switch(datatype)
				{
				case CS_data_str_resource:
					sprintf(pResult,pFmt,CS_StringForResourceID(MSG_ReadLong(&net_message)));
					break;

				case CS_data_str_literal:
					sprintf(pResult,pFmt,MSG_ReadString(&net_message));
					break;

				case CS_data_integer:
					sprintf(pResult,pFmt,MSG_ReadLong(&net_message));
					break;

				case CS_data_float:
					sprintf(pResult,pFmt,MSG_ReadFloat(&net_message));
					break;

				default:
					Com_Error (ERR_DROP,"CL_ParseClientString: Bad data type");
					break;
				};

				// increment the pointers
				MoveToSpecifier(&pFmt, replacedSpecifier);
				MoveToEnd(&pResult);
			}
		}

		do_continue = MSG_ReadByte(&net_message);
		if (do_continue)
		{
			parmCount = 0;	// make sure stuff doesn't break if we forget to send the count!
			// copy the results into the format if we're continuing
			strcpy(fmt,result);
		}
	}
	while(do_continue);

	// read the print type
	printtype = MSG_ReadByte (&net_message);
	if (printtype == CS_print_center)
	{
		msg_time = MSG_ReadFloat(&net_message);
	}

	// all done... print the string
	switch(printtype)
	{
	case CS_print_normal:
		Com_Printf ("%s", result);
		break;

	case CS_print_center:
		SCR_CenterPrint (result, msg_time);
		break;

	default:
		Com_Error (ERR_DROP,"CL_ParseClientString: Bad print type");
		break;
	};
}

void CL_ClientRespawn(void)
// interpret svc_clientrespawn server message
// re-initialize client side variables regarding respawn
{
	cl_death_gib = false;
	
	cl_gameover_time  = 0.0f;
	cl_gameover_alpha = 0.0f;
}

// NSS[2/21/00]:
void    CL_ClearTrackEntities (void);
void CL_ParseSidekickWeaponStatus();
//=====================
//CL_ParseServerMessage
//=====================
void CL_ParseServerMessage (void)
{
	int			cmd;
	char		*s;
	int			i;
	float		msg_time;
	
	//
	// if recording demos, copy the message out
	//
	if (cl_shownet->value == 1)
		Com_Printf ("%i ",net_message.cursize);
	else if (cl_shownet->value >= 2)
		Com_Printf ("------------------\n");
	
/*
    if(cl.ClientEntityManager)
    {
        cl.ClientEntityManager->received_server_frame = timeGetTime();
        cl.ClientEntityManager->bytes_second+=net_message.cursize;
    }
*/	//
	// parse the message
	//
	while (1)
	{
		if (net_message.readcount > net_message.cursize)
		{
			Com_Error (ERR_DROP,"CL_ParseServerMessage: Bad server message");
			break;
		}
		
		cmd = MSG_ReadByte (&net_message);
		
		if (cmd == -1)
		{
#ifdef _DEBUG
			SHOWNET("END OF MESSAGE");
#endif /* DEBUG */
			break;
		}
		
#ifdef _DEBUG
		if (cl_shownet->value>=2)
		{
			if (!svc_strings[cmd])
				Com_Printf ("%3i:BAD CMD %i\n", net_message.readcount-1,cmd);
			else
				SHOWNET(svc_strings[cmd]);
		}
#endif /* DEBUG */
		
		// other commands
		switch (cmd)
		{
		default:
			Com_Error (ERR_DROP,"CL_ParseServerMessage: Illegible server message\n");
			break;
			
		case svc_nop:
			//			Com_Printf ("svc_nop\n");
			break;
			
		case svc_disconnect:
			{
				S_StopAllSounds();
				CL_ClearTrackEntities();
				Com_Error (ERR_DISCONNECT,"Server disconnected\n");
				break;
			}
			
		case svc_reconnect:
			{
				CL_ClearTrackEntities();
				Com_Printf ("Server disconnected, reconnecting\n");
				cls.state = ca_connecting;
				cls.connect_time = -99999;	// CL_CheckForResend() will fire immediately			Com_Printf ("Server disconnected, reconnecting\n");
				break;
			}
			
		case svc_print:
			i = MSG_ReadByte (&net_message);
			if (i == PRINT_CHAT)
			{
				S_StartLocalSound ("global/a_ames.wav");
				// mdm 98.01.29 - this was causing say cmds not to display
				//con.ormask = 128;
			}
			Com_Printf ("%s", MSG_ReadString (&net_message));
			con.ormask = 0;
			break;
			
		case svc_centerprint:
			//	Nelno:	now reads a byte which specifies time message should stay on screen
			s = MSG_ReadString (&net_message);
			msg_time = (float) MSG_ReadShort (&net_message) * 0.125;

			if ( (strlen(s) >= 5) && !strncmp(s,"motd:",5) )
			{
				SCR_CenterPrint (s+5, msg_time,true);
			}
			else
			{
				SCR_CenterPrint (s, msg_time);
			}
			break;
			
		case svc_stufftext:
			s = MSG_ReadString (&net_message);
			//Com_DPrintf ("stufftext: %s\n", s);
			Cbuf_AddText (s);
			break;
			
		case svc_serverdata:
			Cbuf_Execute ();		// make sure any stuffed commands are done
			CL_ParseServerData ();
			break;
			
		case svc_configstring:
			CL_ParseConfigString ();
			break;
			
		case svc_sound:
			CL_ParseStartSoundPacket();
			break;
			
		case svc_spawnbaseline:
			CL_ParseBaseline ();
			break;
			
		case svc_temp_entity:
			CL_ParseTEnt ();
			break;
			
			// Logic[6/24/99]: unused
			/*
			case svc_muzzleflash:
			CL_ParseMuzzleFlash ();
			break;
			case svc_muzzleflash2:
			CL_ParseMuzzleFlash2 ();
			break;
			*/
		case svc_effect_notify:
			CL_ParseEffect ();
			break;
			
		case svc_download:
			CL_ParseDownload ();
			break;
			
		case svc_frame:
			CL_ParseFrame ();
			break;
			
		case svc_inventory:
			inventory_ParseInventory ();
			break;
			
		case svc_onscreen_icon:
			CL_Boost_Icons();
			break;
			
		case svc_clientdeath:
			CL_InformClientDeath(); 
			break;
			
		case svc_sidekick_update:
			CL_Sidekick_Update();     // 5.25
			break;
			
			//case svc_sidekick_command :
			//	CL_ParseSidekickCommand();
			//	break;
			
		case svc_selector :
			CL_ParseSelectorCommand();
			break;
			
		case svc_weapon_icon:
			CL_ParseWeaponIcons();
			break;
			
		case svc_xplevel:
			CL_ParseXPLevel();
			break;
			
		case svc_savegem:
			CL_ParseSaveGem();
			break;
			
		case svc_clientrespawn:
			CL_ClientRespawn();
			break;
			
		case svc_layout:
			s = MSG_ReadString (&net_message);
			strncpy (cl.layout, s, sizeof(cl.layout)-1);
			break;
			
		case svc_playerinfo:
		case svc_packetentities:
		case svc_deltapacketentities:
			Com_Error (ERR_DROP, "Out of place frame data");
			break;
			
			//	Nelno:
			/*
			case svc_setclientmodels:
			CL_ParseClientModels ();
			break;
			*/
		case svc_syncentity:
			CL_ParseSyncEntity ();
			break;
			
		case svc_scoreboard:
			sb_ParseScoreboardMessage ();
			break;
		case svc_cliententity:
//			cl.ClientEntityManager->ReceiveServerMessage(&net_message);
			break;
		case svc_crosshair:
			CL_ParseCrosshairMessage();
			break;

		case svc_end_of_game:
			CL_InformClientTheEnd(); 
			break;

		case svc_ammo_count_update:
			CL_ParseAmmoUpdate();
			break;

		case svc_sidekick_weapon_status:
			CL_ParseSidekickWeaponStatus();
			break;

		case svc_start_music:
			CL_ParseStartMusic();
			break;

		case svc_start_mp3:
			CL_ParseStartMP3();
			break;

		case svc_stop_mp3:
			CL_ParseStopMP3();
			break;

		case svc_reverb_preset:
			CL_ParseReverbPreset();
			break;
		
		case svc_start_stream:
			CL_ParseStartStream();
			break;

		case svc_client_string:
			CL_ParseClientString();
			break;

		}
	}
	
	CL_AddNetgraph ();
	
	//
	// we don't know if it is ok to save a demo message until
	// after we have parsed the frame
	//
	if (cls.demorecording && !cls.demowaiting)
		CL_WriteDemoMessage ();
	
}


