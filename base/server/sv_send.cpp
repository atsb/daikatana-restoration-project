// sv_main.c -- server main program
#include "server.h"
/*
=============================================================================
Com_Printf redirection
=============================================================================
*/
char sv_outputbuf[SV_OUTPUTBUF_LENGTH];
void SV_FlushRedirect (int sv_redirected, char *outputbuf)
{
	if (sv_redirected == RD_PACKET)
	{
		Netchan_OutOfBandPrint (NS_SERVER, net_from, "print\n%s", outputbuf);
	}
	else if (sv_redirected == RD_CLIENT)
	{
		MSG_WriteByte (&sv_client->netchan.message, svc_print);
		MSG_WriteByte (&sv_client->netchan.message, PRINT_HIGH);
		MSG_WriteString (&sv_client->netchan.message, outputbuf);
	}
}
/*
=============================================================================
EVENT MESSAGES
=============================================================================
*/
/*
=================
SV_ClientPrintf
Sends text across to be displayed if the level passes
=================
*/
void SV_ClientPrintf (client_t *cl, int level, char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];
	
	if (level < cl->messagelevel)
		return;
	
	va_start (argptr,fmt);
	vsprintf (string, fmt,argptr);
	va_end (argptr);
	
	MSG_WriteByte (&cl->netchan.message, svc_print);
	MSG_WriteByte (&cl->netchan.message, level);
	MSG_WriteString (&cl->netchan.message, string);
}
/*
=================
SV_BroadcastPrintf
Sends text to all active clients
=================
*/
void SV_BroadcastPrintf (int level, char *fmt, ...)
{
	va_list		argptr;
	char		string[2048];
	client_t	*cl;
	int			i;
	va_start (argptr,fmt);
	vsprintf (string, fmt,argptr);
	va_end (argptr);
	
	// echo to console
	if (dedicated->value)
	{
		char	copy[1024];
		int		i;
		
		// mask off high bits
		for (i=0 ; i<1023 && string[i] ; i++)
			copy[i] = string[i]&127;
		copy[i] = 0;
		Com_Printf ("%s", copy);
	}
	for (i=0, cl = svs.clients ; i<maxclients->value; i++, cl++)
	{
		if (level < cl->messagelevel)
			continue;
		if (cl->state != cs_spawned)
			continue;
		MSG_WriteByte (&cl->netchan.message, svc_print);
		MSG_WriteByte (&cl->netchan.message, level);
		MSG_WriteString (&cl->netchan.message, string);
	}
}
/*
=================
SV_BroadcastCommand
Sends text to all active clients
=================
*/
void SV_BroadcastCommand (char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];
	CVector		vZero(0,0,0);
	
	if (!sv.state)
		return;
	va_start (argptr,fmt);
	vsprintf (string, fmt,argptr);
	va_end (argptr);
	MSG_WriteByte (&sv.multicast, svc_stufftext);
	MSG_WriteString (&sv.multicast, string);
	SV_Multicast (vZero, MULTICAST_ALL_R);
}

qboolean SV_TestMulticast(multicast_t to)
{
	qboolean reliable = false;
	switch(to)
	{
	case MULTICAST_ALL_R:
	case MULTICAST_PHS_R:
	case MULTICAST_PVS_R:
		reliable = true;
		break;

	case MULTICAST_ALL:
	case MULTICAST_PHS:
	case MULTICAST_PVS:
		reliable = false;
		break;

	default:
		return FALSE;
	};

	sizebuf_t	*buf;
	client_t	*client;
	int			j;
	int			length;
	for (j = 0, client = svs.clients; j < maxclients->value; j++, client++)
	{
		if (client->state == cs_free || client->state == cs_zombie)
			continue;
		if (client->state != cs_spawned && !reliable)
			continue;

		if (reliable)
			buf = &client->netchan.message;
		else
			buf = &client->datagram;

		if (!buf)
			return FALSE;

		length = sv.multicast.cursize;
		if ( (buf->maxsize == 0) ||  ((buf->cursize + length > buf->maxsize) && ((!buf->allowoverflow) || (length > buf->maxsize))) )
			return FALSE;
	}

	return TRUE;
}

/*
=================
SV_Multicast
Sends the contents of sv.multicast to a subset of the clients,
then clears sv.multicast.
MULTICAST_ALL	same as broadcast (origin can be NULL)
MULTICAST_PVS	send to clients potentially visible from org
MULTICAST_PHS	send to clients potentially hearable from org
=================
*/
void SV_Multicast (CVector &origin, multicast_t to)
{
	client_t	*client;
	byte		*mask;
	int			leafnum, cluster;
	int			j;
	qboolean	reliable;
	int			area1, area2;

	reliable = false;

	if (to != MULTICAST_ALL_R && to != MULTICAST_ALL)
	{
		leafnum = CM_PointLeafnum (origin);
		area1 = CM_LeafArea (leafnum);
	}
	else
	{
		leafnum = 0;	// just to avoid compiler warnings
		area1 = 0;
	}
	// if doing a serverrecord, store everything
	if (svs.demofile)
		SZ_Write (&svs.demo_multicast, sv.multicast.data, sv.multicast.cursize);
	
	switch (to)
	{
	case MULTICAST_ALL_R:
		reliable = true;	// intentional fallthrough
	case MULTICAST_ALL:
		leafnum = 0;
		mask = NULL;
		break;

	case MULTICAST_PHS_R:
		reliable = true;	// intentional fallthrough
	case MULTICAST_PHS:
		leafnum = CM_PointLeafnum (origin);
		cluster = CM_LeafCluster (leafnum);
		mask = CM_ClusterPHS (cluster);
		break;

	case MULTICAST_PVS_R:
		reliable = true;	// intentional fallthrough
	case MULTICAST_PVS:
		leafnum = CM_PointLeafnum (origin);
		cluster = CM_LeafCluster (leafnum);
		mask = CM_ClusterPVS (cluster);
		break;
	default:
		mask = NULL;
		Com_Error (ERR_FATAL, "SV_Multicast: bad to:%i", to);
	}
	// send the data to all relevent clients
	for (j = 0, client = svs.clients; j < maxclients->value; j++, client++)
	{
		if (client->state == cs_free || client->state == cs_zombie)
			continue;
		if (client->state != cs_spawned && !reliable)
			continue;
		if (mask)
		{
			leafnum = CM_PointLeafnum (client->edict->s.origin);
			cluster = CM_LeafCluster (leafnum);
			area2 = CM_LeafArea (leafnum);
			if (!CM_AreasConnected (area1, area2))
				continue;
			if ( mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
				continue;
		}
		if (reliable)
			SZ_Write (&client->netchan.message, sv.multicast.data, sv.multicast.cursize);
		else
			SZ_Write (&client->datagram, sv.multicast.data, sv.multicast.cursize);
	}
	SZ_Clear (&sv.multicast);
}

void PF_Unicast (edict_t *ent, qboolean reliable);
void  SV_ReverbPreset (int nPresetID)
{
	MSG_WriteByte (&sv.multicast, svc_reverb_preset);
	MSG_WriteShort (&sv.multicast, nPresetID);
	SV_Multicast (CVector(0,0,0), MULTICAST_ALL);
}

void  SV_StartStream (const char* name, float vol)
{
	if (!name)
		return;

	MSG_WriteByte (&sv.multicast, svc_start_stream);
	MSG_WriteString (&sv.multicast, name);
	MSG_WriteFloat (&sv.multicast, vol);
	SV_Multicast (CVector(0,0,0), MULTICAST_ALL);
}

void SV_StartMusic(const char* name, int chan, edict_t *ent)
{
	if (!name)
		return;

	MSG_WriteByte (&sv.multicast, svc_start_music);
	MSG_WriteString (&sv.multicast, name);
	MSG_WriteShort (&sv.multicast, chan);
	if (ent)
		PF_Unicast (ent, true);
	else
		SV_Multicast (CVector(0,0,0), MULTICAST_ALL);
}

void SV_StartMP3( const char* name, int channel, float fvol, int play_count, qboolean bRestart, edict_t *ent )
{
	if (!name)
		return;

	MSG_WriteByte (&sv.multicast, svc_start_mp3);
	MSG_WriteString (&sv.multicast, name);
	MSG_WriteShort (&sv.multicast, channel);
	MSG_WriteFloat (&sv.multicast, fvol);
	MSG_WriteShort (&sv.multicast, play_count);
	MSG_WriteByte (&sv.multicast, bRestart);
	if (ent)
		PF_Unicast (ent, true);
	else
		SV_Multicast (CVector(0,0,0), MULTICAST_ALL);
}


void SV_StopMP3(int channel, edict_t *ent)
{
	MSG_WriteByte (&sv.multicast, svc_stop_mp3);
	MSG_WriteShort (&sv.multicast, channel);
	if (ent)
		PF_Unicast (ent, true);
	else
		SV_Multicast (CVector(0,0,0), MULTICAST_ALL);
}
/*  
==================
SV_StartSound
Each entity can have eight independant sound sources, like voice,
weapon, feet, etc.
If cahnnel & 8, the sound will be sent to everyone, not just
things in the PHS.
FIXME: if entity isn't in PHS, they must be forced to be sent or
have the origin explicitly sent.
Channel 0 is an auto-allocate channel, the others override anything
already running on that entity/channel pair.
An attenuation of 0 will play full volume everywhere in the level.
Larger attenuations will drop off.  (max 4 attenuation)
Timeofs can range from 0.0 to 0.1 to cause sounds to be started
later in the frame than they normally would.
If origin is NULL, the origin is determined from the entity origin
or the midpoint of the entity box for bmodels.
==================
*/  
void SV_StartSound (CVector &origin, edict_t *entity, int channel,int soundindex, float volume,float dist_min, float dist_max, float timeofs)
{       
	int			sendchan;
    int			flags = 0;
	int			ent;
	CVector		soundOrigin;
	qboolean	use_phs;

	//	Nelno:	Only error if volume is negative
	if (volume < 0)
			Com_Error (ERR_FATAL, "SV_StartSound: volume = %f", volume);

	if (volume > 1.0)
	{
		Com_DPrintf("WARNING: Bad volume sent to SV_StartSound\n");
	}

//	if (channel < 0 || channel > 15)
//		Com_Error (ERR_FATAL, "SV_StartSound: channel = %i", channel);
	if (timeofs < 0 || timeofs > 0.255)
		Com_Error (ERR_FATAL, "SV_StartSound: timeofs = %f", timeofs);
	ent = NUM_FOR_EDICT(entity);
	if (channel & 8)	// no PHS flag
	{
		use_phs = false;
		channel &= 7;
	}
	else
	{
		use_phs = true;
	}
	sendchan = (ent<<3) | (channel&7);

	if (volume != DEFAULT_SOUND_PACKET_VOLUME)
		flags |= SND_VOLUME;

	if (dist_min != ATTN_NORM_MIN || dist_max != ATTN_NORM_MAX)
		flags |= SND_DISTANCE;

	// the client doesn't know that bmodels have weird origins
	// the origin can also be explicitly set... when fOriginLength > 0,
	// then it has been explicitly set and should be used for the location
	// of the sound

	float fOriginLength = origin.Length();
	if ( (entity->svflags & SVF_NOCLIENT) || (entity->solid == SOLID_BSP) || fOriginLength > 0.0f )
	{
		flags |= SND_POS;
	}

	// always send the entity number for channel overrides
	flags |= SND_ENT;
	if (timeofs)
		flags |= SND_OFFSET;

	// use the entity origin unless it is a bmodel or explicitly specified
	if ( fOriginLength == 0.0f )
	{
		if (entity->solid == SOLID_BSP)
		{
			soundOrigin.x = entity->s.origin.x+0.5*(entity->s.mins.x+entity->s.maxs.x);
			soundOrigin.y = entity->s.origin.y+0.5*(entity->s.mins.y+entity->s.maxs.y);
			soundOrigin.z = entity->s.origin.z+0.5*(entity->s.mins.z+entity->s.maxs.z);
		}
		else
		{
			soundOrigin = entity->s.origin;
		}
	}
	else
	{
		// the sound origin was explicitly set
		soundOrigin = origin;
	}

	MSG_WriteByte (&sv.multicast, svc_sound);
	MSG_WriteByte (&sv.multicast, flags);
	// mdm99.07.01 - ugh, we have more than 256 sounds per level :(
//	MSG_WriteByte (&sv.multicast, soundindex);
	MSG_WriteShort(&sv.multicast, soundindex);
	if (flags & SND_VOLUME)
		MSG_WriteByte (&sv.multicast, volume*255);
	if (flags & SND_DISTANCE)
	{
		MSG_WriteByte(&sv.multicast, ((int)dist_min)>>3);
		MSG_WriteByte(&sv.multicast, ((int)dist_max)>>5);
	}
	if (flags & SND_OFFSET)
		MSG_WriteByte (&sv.multicast, timeofs*1000);
	if (flags & SND_ENT)
		MSG_WriteShort (&sv.multicast, sendchan);
	if (flags & SND_POS)
		MSG_WritePos (&sv.multicast, soundOrigin);

	// if the sound doesn't attenuate,send it to everyone
	// (global radio chatter, voiceovers, etc)
	if (!dist_min || !dist_max)
		use_phs = false;
	if (channel & CHAN_RELIABLE)
	{
		if (use_phs)
			SV_Multicast (soundOrigin, MULTICAST_PHS_R);
		else
			SV_Multicast (soundOrigin, MULTICAST_ALL_R);
	}
	else
	{
		if (use_phs)
			SV_Multicast (soundOrigin, MULTICAST_PHS);
		else
			SV_Multicast (soundOrigin, MULTICAST_ALL);
	}
}           

///*
//void SV_StartSound (edict_t *entity, int channel,
//					int soundindex, float volume,
//					float attenuation, float timeofs)
//{       
//	int			sendchan;
//    int			flags;
//	int			ent;
//	CVector		origin_v;
//	qboolean	use_phs;
//
//	//	Nelno:	Only error if volume is negative
//	if (volume < 0)
//			Com_Error (ERR_FATAL, "SV_StartSound: volume = %f", volume);
//	//	Nelno:	for Quake 1 DLL support, scale volumes > 1.0 to 0 - 1.0
//	if (volume > 1.0)
//	{
//		volume = 255.0 / volume;
//	}
//
//	if (attenuation < 0 || attenuation > 4)
//		Com_Error (ERR_FATAL, "SV_StartSound: attenuation = %f", attenuation);
//	if (timeofs < 0 || timeofs > 0.255)
//		Com_Error (ERR_FATAL, "SV_StartSound: timeofs = %f", timeofs);
//	ent = NUM_FOR_EDICT(entity);
//	if (channel & 8)	// no PHS flag
//	{
//		use_phs = false;
//		channel &= 7;
//	}
//	else
//	{
//		use_phs = true;
//	}
//	sendchan = (ent<<3) | (channel&7);
//	flags = 0;
//	if (volume != DEFAULT_SOUND_PACKET_VOLUME)
//		flags |= SND_VOLUME;
//	if (attenuation != DEFAULT_SOUND_PACKET_ATTENUATION)
//		flags |= SND_ATTENUATION;
//
//	// the client doesn't know that bmodels have weird origins
//	// the origin can also be explicitly set
//	if ( (entity->svflags & SVF_NOCLIENT) || (entity->solid == SOLID_BSP) )
//	{
//		flags |= SND_POS;
//	}
//
//	// always send the entity number for channel overrides
//	flags |= SND_ENT;
//	if (timeofs)
//		flags |= SND_OFFSET;
//
//	// use the entity origin unless it is a bmodel or explicitly specified
//	CVector origin = origin_v;
//	if (entity->solid == SOLID_BSP)
//	{
//		origin_v.x = entity->s.origin.x+0.5*(entity->s.mins.x+entity->s.maxs.x);
//		origin_v.y = entity->s.origin.y+0.5*(entity->s.mins.y+entity->s.maxs.y);
//		origin_v.z = entity->s.origin.z+0.5*(entity->s.mins.z+entity->s.maxs.z);
//	}
//	else
//	{
//		origin_v = entity->s.origin;
//	}
//
//	MSG_WriteByte (&sv.multicast, svc_sound);
//	MSG_WriteByte (&sv.multicast, flags);
//	MSG_WriteByte (&sv.multicast, soundindex);
//	if (flags & SND_VOLUME)
//		MSG_WriteByte (&sv.multicast, volume*255);
//	if (flags & SND_ATTENUATION)
//		MSG_WriteByte (&sv.multicast, attenuation*64);
//	if (flags & SND_OFFSET)
//		MSG_WriteByte (&sv.multicast, timeofs*1000);
//	if (flags & SND_ENT)
//		MSG_WriteShort (&sv.multicast, sendchan);
//
//	// if the sound doesn't attenuate,send it to everyone
//	// (global radio chatter, voiceovers, etc)
//	if (attenuation == ATTN_NONE)
//		use_phs = false;
//	if (channel & CHAN_RELIABLE)
//	{
//		if (use_phs)
//			SV_Multicast (origin, MULTICAST_PHS_R);
//		else
//			SV_Multicast (origin, MULTICAST_ALL_R);
//	}
//	else
//	{
//		if (use_phs)
//			SV_Multicast (origin, MULTICAST_PHS);
//		else
//			SV_Multicast (origin, MULTICAST_ALL);
//	}
//}           
//*/

/*
===============================================================================
FRAME UPDATES
===============================================================================
*/
/*
=======================
SV_SendClientDatagram
=======================
*/
qboolean SV_SendClientDatagram (client_t *client)
{
	byte		msg_buf[MAX_MSGLEN];
	sizebuf_t	msg;

	SV_BuildClientFrame (client);

	SZ_Init (&msg, msg_buf, sizeof(msg_buf));
	msg.allowoverflow = true;
	// send over all the relevant entity_state_t
	// and the player_state_t
	SV_WriteFrameToClient (client, &msg);
	// copy the accumulated multicast datagram
	// for this client out to the message
	// it is necessary for this to be after the WriteEntities
	// so that entity references will be current
	if (client->datagram.overflowed)
		Com_DPrintf ("WARNING: datagram overflowed for %s\n", client->name);
	else
		SZ_Write (&msg, client->datagram.data, client->datagram.cursize);
	SZ_Clear (&client->datagram);
	if (msg.overflowed)
	{	// must have room left for the packet header
		Com_DPrintf ("WARNING: msg overflowed for %s\n", client->name);
		SZ_Clear (&msg);
	}
	// send the datagram
	Netchan_Transmit (&client->netchan, msg.cursize, msg.data);

	// record the size for rate estimation
	client->message_size[sv.framenum % RATE_MESSAGES] = msg.cursize;
	return true;
}
/*
==================
SV_DemoCompleted
==================
*/
void SV_DemoCompleted (void)
{
	if (sv.demofile)
	{
		fclose (sv.demofile);
		sv.demofile = NULL;
	}
	SV_Nextserver ();
}


/*
=======================
SV_RateDrop

Returns true if the client is over its current
bandwidth estimation and should not be sent another packet
=======================
*/
qboolean SV_RateDrop (client_t *c)
{
	int		total;
	int		i;

	// never drop over the loopback
	if (c->netchan.remote_address.type == NA_LOOPBACK)
		return false;

	total = 0;

	for (i = 0 ; i < RATE_MESSAGES ; i++)
	{
		total += c->message_size[i];
	}

	if (total > c->rate)
	{
		c->surpressCount++;
		c->message_size[sv.framenum % RATE_MESSAGES] = 0;
		return true;
	}

	return false;
}

/*
=======================
SV_SendClientMessages
=======================
*/
void SV_SendClientMessages (void)
{
	int			i;
	client_t	*c;
	int			msglen;
	byte		msgbuf[MAX_MSGLEN];
	int			r;
	msglen = 0;
	// read the next demo message if needed
	if (sv.state == ss_demo && sv.demofile)
	{
		if (sv_paused->value)
			msglen = 0;
		else
		{
			// get the next message
			r = fread (&msglen, 4, 1, sv.demofile);
			if (r != 1)
			{
				SV_DemoCompleted ();
				return;
			}
			msglen = LittleLong (msglen);
			if (msglen == -1)
			{
				SV_DemoCompleted ();
				return;
			}
			if (msglen > MAX_MSGLEN)
				Com_Error (ERR_DROP, "SV_SendClientMessages: msglen > MAX_MSGLEN");
			r = fread (msgbuf, msglen, 1, sv.demofile);
			if (r != 1)
			{
				SV_DemoCompleted ();
				return;
			}
		}
	}
	// send a message to each connected client
	for (i=0, c = svs.clients ; i<maxclients->value; i++, c++)
	{
		if (!c->state)
			continue;
		// if the reliable message overflowed,
		// drop the client
		if (c->netchan.message.overflowed)
		{
			SZ_Clear (&c->netchan.message);
			SZ_Clear (&c->datagram);
			SV_BroadcastPrintf (PRINT_HIGH, "%s overflowed\n", c->name);
			SV_ClientPrintf (c, PRINT_HIGH, "DROPPED: Your Server buffer overflowed\n");
			SV_DropClient (c);
			c->lastmessage = svs.realtime;	// min case there is a funny zombie
		}

		if (sv.state == ss_cinematic 
			|| sv.state == ss_demo 
			|| sv.state == ss_pic
			)
			Netchan_Transmit (&c->netchan, msglen, msgbuf);
		else if (c->state == cs_spawned)
		{
			// don't overrun bandwidth
			if (SV_RateDrop (c))
				continue;

			SV_SendClientDatagram (c);
		}
		else
		{
	// just update reliable	if needed
			if (c->netchan.message.cursize	|| curtime - c->netchan.last_sent > 1000 )
				Netchan_Transmit (&c->netchan, 0, NULL);
		}
	}
}
