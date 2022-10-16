//#include	<windows.h>
#include "dk_system.h"
#include "server.h"
/*
=============================================================================
Encode a client frame onto the network channel
=============================================================================
*/

///////////////////////////////////////////////////////////////////////////////
//	SV_EmitSyncEntity
//
//	Writes a delta update of the passed entity.
//	The entity is added to the client_entities array for the last sent frame
//	When the client receives this message it should add the entity to its
//	current frame for drawing.
///////////////////////////////////////////////////////////////////////////////

void SV_EmitSyncEntity (edict_t *client_entity, edict_t *ent)
{
	client_frame_t	*frame;
	entity_state_t	*state, *new_state;
	int				i, index = 0;
	client_t		*client;
	byte			msg_buf [MAX_MSGLEN];
	sizebuf_t		msg;

	//	allocate the message buffer
	SZ_Init (&msg, msg_buf, sizeof(msg_buf));
	msg.allowoverflow = true;

	//	find the client_t that matches ent
	for (i = 0, client = svs.clients ; i < maxclients->value; i++, client++)
	{
		if (client->edict == client_entity)
			break;
	}

	if (i >= maxclients->value)
		return;

	frame = &client->frames [sv.framenum & UPDATE_MASK];

	MSG_WriteByte (&msg, svc_syncentity);

	state = &svs.client_entities [frame->first_entity % svs.num_client_entities];

	//	determine if the entity is already in the client_entities array
	while (state->number != ent->s.number && index < frame->num_entities)
	{
		//	add it to the circular client_entities array
		index++;
		state = &svs.client_entities [(frame->first_entity + index) % svs.num_client_entities];
	}

	if (index < frame->num_entities)
	{
		//	entity was already in the client_entities array, so just emit it
		new_state = &ent->s;

		MSG_WriteDeltaEntity (state, new_state, &msg, DELTA_FORCE_FRAME);
	}
	else
	{
		state = &svs.client_entities [(frame->first_entity + index) % svs.num_client_entities];

		//	go to next element in client_entities array
		svs.next_client_entities++;
		//	add one to the number of entities in this frame
		frame->num_entities++;

		new_state = &ent->s;
		MSG_WriteDeltaEntity (state, new_state, &msg, DELTA_FORCE_FRAME);
	}

	//	FIXME:	fix number???

	//	FIXME:	don't mark players missiles as solid???

	//	copy ent's state to client_entities array
	*state = ent->s;

	//	send the datagram to the client
	Netchan_Transmit (&client->netchan, msg.cursize, msg.data);
}

/*
=============
SV_EmitPacketEntities
Writes a delta update of an entity_state_t list to the message.
=============
*/
void SV_EmitPacketEntities (client_frame_t *from, client_frame_t *to, sizebuf_t *msg)
{
	entity_state_t	*oldent, *newent;
	int		oldindex, newindex;
	int		oldnum, newnum;
	int		from_num_entities;
	int		bits;
	edict_t	*ent;

	MSG_WriteByte (msg, svc_packetentities);
	if (!from)
		from_num_entities = 0;
	else
		from_num_entities = from->num_entities;
	newindex = 0;
	oldindex = 0;
	while (newindex < to->num_entities || oldindex < from_num_entities)
	{
		if (newindex >= to->num_entities)
			newnum = 9999;
		else
		{
			newent = &svs.client_entities[(to->first_entity+newindex)%svs.num_client_entities];
			newnum = newent->number;
		}

		if (oldindex >= from_num_entities)
			oldnum = 9999;
		else
		{
			oldent = &svs.client_entities[(from->first_entity+oldindex)%svs.num_client_entities];
			oldnum = oldent->number;
		}

		//	Nelno:	force syncing of frame if just starting a level
		if (newnum == oldnum)
		{	// delta update from old position
			// because the force parm is false, this will not result
			// in any bytes being emited if the entity has not changed at all
			if (sv.time < 2.0)
				MSG_WriteDeltaEntity (oldent, newent, msg, DELTA_FORCE_FRAME);
			else
				MSG_WriteDeltaEntity (oldent, newent, msg, DELTA_DONT_FORCE);

			//	Nelno:	make sure FRAME_FORCEINDEX gets cleared...  what a load of shit
			ent = EDICT_NUM (newent->number);
			ent->s.frameInfo.frameFlags &= ~FRAME_FORCEINDEX;

			oldindex++;
			newindex++;
			continue;
		}
		if (newnum < oldnum)
		{	// this is a new entity, send it from the baseline
			MSG_WriteDeltaEntity (&sv.baselines[newnum], newent, msg, DELTA_FORCE);

			//	Nelno:	make sure FRAME_FORCEINDEX gets cleared...  what a load of shit
			ent = EDICT_NUM (newent->number);
			ent->s.frameInfo.frameFlags &= ~FRAME_FORCEINDEX;

			newindex++;
			continue;
		}
		if (newnum > oldnum)
		{	// the old entity isn't present in the new message
			bits = U_REMOVE;
			
			//	if the entity is no longer in use, the send a U_FREED, too
			//	U_FREED has the same value as U_EVENT
			ent = EDICT_NUM (oldent->number);

			if (!(ent->inuse))
				bits |= U_FREED;
			
			if (oldnum >= 256)
				bits |= U_NUMBER16 | U_MOREBITS1;

			//	write first byte of bits
			MSG_WriteByte (msg,	bits & 255);

			//	write second byte of bits (only if oldent's number > 255)
			if (bits & 0x0000ff00)
				MSG_WriteByte (msg,	(bits>>8)&255 );

			//	write oldent's number
			if (bits & U_NUMBER16)
				MSG_WriteShort (msg, oldnum);
			else
				MSG_WriteByte (msg, oldnum);

			oldindex++;
			continue;
		}
	}
	MSG_WriteShort (msg, 0);	// end of packetentities
}
/*
=============
SV_WritePlayerstateToClient
=============
*/
void SV_WritePlayerstateToClient (client_frame_t *from, client_frame_t *to, sizebuf_t *msg)
{
	int				i;
	int				pflags;
	player_state_t	*ps, *ops;
	player_state_t	dummy;
	int				statbits;
	entity_state_t	*view_entity_state;

	ps = &to->ps;
	if (!from)
	{
		memset (&dummy, 0, sizeof(dummy));
		ops = &dummy;
	}
	else
		ops = &from->ps;

	//
	// determine what needs to be sent
	//
	pflags = 0;

	if (ps->pmove.pm_type != ops->pmove.pm_type)
		pflags |= PS_M_TYPE;

	if (ps->view_entity != 0)
	{
		//	view_entity is not client, so update from view_entity's position
		pflags |= PS_M_ORIGIN | PS_M_VELOCITY;

		view_entity_state = &(EDICT_NUM(ps->view_entity)->s);
	}
	else
	{
		if (ps->view_entity != ops->view_entity)
		{
			pflags |= PS_M_ORIGIN | PS_M_VELOCITY;
		}
		else
		{
			if (ps->pmove.origin[0] != ops->pmove.origin[0]
				|| ps->pmove.origin[1] != ops->pmove.origin[1]
				|| ps->pmove.origin[2] != ops->pmove.origin[2] )
				pflags |= PS_M_ORIGIN;

			if (ps->pmove.velocity[0] != ops->pmove.velocity[0]
				|| ps->pmove.velocity[1] != ops->pmove.velocity[1]
				|| ps->pmove.velocity[2] != ops->pmove.velocity[2] )
				pflags |= PS_M_VELOCITY;
		}
	}

	if (ps->pmove.pm_time != ops->pmove.pm_time)
		pflags |= PS_M_TIME;

	if (ps->pmove.pm_flags != ops->pmove.pm_flags)
		pflags |= PS_M_FLAGS;

	if (ps->pmove.gravity != ops->pmove.gravity)
		pflags |= PS_M_GRAVITY;

	if (ps->pmove.delta_angles[0] != ops->pmove.delta_angles[0]
		|| ps->pmove.delta_angles[1] != ops->pmove.delta_angles[1]
		|| ps->pmove.delta_angles[2] != ops->pmove.delta_angles[2] )
		pflags |= PS_M_DELTA_ANGLES;


	if (ps->viewoffset[0] != ops->viewoffset[0]
		|| ps->viewoffset[1] != ops->viewoffset[1]
		|| ps->viewoffset[2] != ops->viewoffset[2] )
		pflags |= PS_VIEWOFFSET;

	if (ps->viewangles[0] != ops->viewangles[0]
		|| ps->viewangles[1] != ops->viewangles[1]
		|| ps->viewangles[2] != ops->viewangles[2] )
		pflags |= PS_VIEWANGLES;

	if (ps->kick_angles[0] != ops->kick_angles[0]
		|| ps->kick_angles[1] != ops->kick_angles[1]
		|| ps->kick_angles[2] != ops->kick_angles[2] )
		pflags |= PS_KICKANGLES;

	if (ps->blend[0] != ops->blend[0]
		|| ps->blend[1] != ops->blend[1]
		|| ps->blend[2] != ops->blend[2]
		|| ps->blend[3] != ops->blend[3] )
		pflags |= PS_BLEND;

	if (ps->fov != ops->fov)
		pflags |= PS_FOV;

	if (ps->rdflags != ops->rdflags)
		pflags |= PS_RDFLAGS;

/*
	if (ps->gunframe != ops->gunframe)
		pflags |= PS_WEAPONFRAME;

	pflags |= PS_WEAPONINDEX;
*/

	if (ps->view_entity != ops->view_entity)
		pflags |= PS_VIEWENTITY;

	if (ps->input_entity != ops->input_entity)
		pflags |= PS_INPUTENTITY;

	//
	// write it
	//
	MSG_WriteByte (msg, svc_playerinfo);
	MSG_WriteShort (msg, pflags);

	//
	// write the pmove_state_t
	//
	if (pflags & PS_M_TYPE)
		MSG_WriteByte (msg, ps->pmove.pm_type);

	if (ps->view_entity != 0)
	{
		//	send down origin of view_entity
		MSG_WriteShort (msg, (int)(view_entity_state->origin [0] * 8));
		MSG_WriteShort (msg, (int)(view_entity_state->origin [1] * 8));
		MSG_WriteShort (msg, (int)(view_entity_state->origin [2] * 8));

		//	send down a zero velocity 
		MSG_WriteShort (msg, 0);
		MSG_WriteShort (msg, 0);
		MSG_WriteShort (msg, 0);
	}
	else
	{
		if (pflags & PS_M_ORIGIN)
		{
			MSG_WriteShort (msg, ps->pmove.origin[0]);
			MSG_WriteShort (msg, ps->pmove.origin[1]);
			MSG_WriteShort (msg, ps->pmove.origin[2]);
		}

		if (pflags & PS_M_VELOCITY)
		{
			MSG_WriteShort (msg, ps->pmove.velocity[0]);
			MSG_WriteShort (msg, ps->pmove.velocity[1]);
			MSG_WriteShort (msg, ps->pmove.velocity[2]);
		}
	}

	if (pflags & PS_M_TIME)
		MSG_WriteByte (msg, ps->pmove.pm_time);

	if (pflags & PS_M_FLAGS)
		MSG_WriteShort (msg, ps->pmove.pm_flags);

	if (pflags & PS_M_GRAVITY)
		MSG_WriteShort (msg, ps->pmove.gravity);

	if (pflags & PS_M_DELTA_ANGLES)
	{
		MSG_WriteShort (msg, ps->pmove.delta_angles[0]);
		MSG_WriteShort (msg, ps->pmove.delta_angles[1]);
		MSG_WriteShort (msg, ps->pmove.delta_angles[2]);
	}

	//
	// write the rest of the player_state_t
	//
	if (pflags & PS_VIEWOFFSET)
	{
		MSG_WriteChar (msg, ps->viewoffset[0]*4);
		MSG_WriteChar (msg, ps->viewoffset[1]*4);
		MSG_WriteChar (msg, ps->viewoffset[2]*4);
	}
	if (pflags & PS_VIEWANGLES)
	{
		MSG_WriteAngle16 (msg, ps->viewangles[0]);
		MSG_WriteAngle16 (msg, ps->viewangles[1]);
		MSG_WriteAngle16 (msg, ps->viewangles[2]);
	}

	if (pflags & PS_KICKANGLES)
	{
		MSG_WriteChar (msg, ps->kick_angles[0]*4);
		MSG_WriteChar (msg, ps->kick_angles[1]*4);
		MSG_WriteChar (msg, ps->kick_angles[2]*4);
	}

/*
	if (pflags & PS_WEAPONINDEX)
	{
		MSG_WriteByte (msg, ps->gunindex);
	}
	if (pflags & PS_WEAPONFRAME)
	{
		MSG_WriteByte (msg, ps->gunframe);
		MSG_WriteChar (msg, ps->gunoffset[0]*4);
		MSG_WriteChar (msg, ps->gunoffset[1]*4);
		MSG_WriteChar (msg, ps->gunoffset[2]*4);
		MSG_WriteChar (msg, ps->gunangles[0]*4);
		MSG_WriteChar (msg, ps->gunangles[1]*4);
		MSG_WriteChar (msg, ps->gunangles[2]*4);
	}
*/

	if (pflags & PS_BLEND)
	{
		MSG_WriteByte (msg, ps->blend[0]*255);
		MSG_WriteByte (msg, ps->blend[1]*255);
		MSG_WriteByte (msg, ps->blend[2]*255);
		MSG_WriteByte (msg, ps->blend[3]*255);
	}
	if (pflags & PS_FOV)
		MSG_WriteByte (msg, ps->fov);
	
	if (pflags & PS_RDFLAGS)
		MSG_WriteShort (msg, ps->rdflags);

	if (pflags & PS_VIEWENTITY)
		MSG_WriteShort (msg, ps->view_entity);

	if (pflags & PS_INPUTENTITY)
		MSG_WriteShort (msg, ps->input_entity);

	// send stats
	// cek[2-5-00]: here's a neat idea...lets send longs as longs.  shorts are too short.
	statbits = 0;
	for (i=0 ; i<MAX_STATS ; i++)
		if (ps->stats[i] != ops->stats[i])
			statbits |= 1<<i;
	MSG_WriteLong (msg, statbits);
	for (i=0 ; i<MAX_STATS ; i++)
		if (statbits & (1<<i) )
			MSG_WriteLong (msg, ps->stats[i]);
}


/*
==================
SV_WriteFrameToClient
==================
*/
void SV_WriteFrameToClient (client_t *client, sizebuf_t *msg)
{
	client_frame_t		*frame, *oldframe;
	int					lastframe;

//Com_Printf ("%i -> %i\n", client->lastframe, sv.framenum);
	// this is the frame we are creating
	frame = &client->frames[sv.framenum & UPDATE_MASK];

	if (client->lastframe <= 0)
	{	// client is asking for a retransmit
		oldframe = NULL;
		lastframe = -1;
	}
	else if (sv.framenum - client->lastframe >= (UPDATE_BACKUP - 3) )
	{	// client hasn't gotten a good message through in a long time
//		Com_Printf ("%s: Delta request from out-of-date packet.\n", client->name);
		oldframe = NULL;
		lastframe = -1;
	}
	else
	{	// we have a valid message to delta from
		oldframe = &client->frames[client->lastframe & UPDATE_MASK];
		lastframe = client->lastframe;
	}

	MSG_WriteByte (msg, svc_frame);
	MSG_WriteLong (msg, sv.framenum);
	MSG_WriteLong (msg, lastframe);	// what we are delta'ing from
	MSG_WriteByte (msg, client->surpressCount);	// rate dropped packets
	client->surpressCount = 0;

	// send over the areabits
	MSG_WriteByte (msg, frame->areabytes);
	SZ_Write (msg, frame->areabits, frame->areabytes);

	// delta encode the playerstate
	SV_WritePlayerstateToClient (oldframe, frame, msg);

	// delta encode the entities
	SV_EmitPacketEntities (oldframe, frame, msg);
}


/*
=============================================================================

Build a client frame structure

=============================================================================
*/

byte		fatpvs[65536/8];	// 32767 is MAX_MAP_LEAFS
/*
============
SV_FatPVS
The client will interpolate the view position,
so we can't use a single PVS point
===========
*/
void SV_FatPVS (const CVector &org)
{
	int		leafs[64];
	int		i, j, count;
	int		longs;
	byte	*src;
	CVector	mins, maxs;

	mins.x = org.x - 8;
	mins.y = org.y - 8;
	mins.z = org.z - 8;
	maxs.x = org.x + 8;
	maxs.y = org.y + 8;
	maxs.z = org.z + 8;

	count = CM_BoxLeafnums (mins, maxs, leafs, 64, NULL);
	if (count < 1)
		Com_Error (ERR_FATAL, "SV_FatPVS: count < 1");
	longs = (CM_NumClusters()+31)>>5;
	// convert leafs to clusters
	for (i=0 ; i<count ; i++)
		leafs[i] = CM_LeafCluster(leafs[i]);
	memcpy (fatpvs, CM_ClusterPVS(leafs[0]), longs<<2);
	// or in all the other leaf bits
	for (i=1 ; i<count ; i++)
	{
		for (j=0 ; j<i ; j++)
			if (leafs[i] == leafs[j])
				break;
		if (j != i)
			continue;		// already have the cluster we want
		src = CM_ClusterPVS(leafs[i]);
		for (j=0 ; j<longs ; j++)
			((long *)fatpvs)[j] |= ((long *)src)[j];
	}
}
/*
=============
SV_BuildClientFrame

Decides which entities are going to be visible to the client, and
copies off the playerstat and areabits.
=============
*/
void SV_BuildClientFrame (client_t *client)
{
	int		e, i;
   short checkpvs,clientnum=client-svs.clients+1;
	CVector	org;
	edict_t	*ent;
	edict_t	*clent;
	client_frame_t	*frame;
	entity_state_t	*state;
	int		l;
	int		clientarea, clientcluster;
	int		leafnum;
	int		c_fullsend;
	byte	*clientphs;
	byte	*bitvector;

	clent = client->edict;
	if (!clent->client)
		return;		// not in game yet

	// this is the frame we are creating
	frame = &client->frames[sv.framenum & UPDATE_MASK];
	
	// Nelno:	calculate PVS from either client or camera position
	if (clent->client->ps.view_entity != 0)
	{
		//	view_entity is not client, so update from view_entity's position
		ent = EDICT_NUM (clent->client->ps.view_entity);

		org.x = ent->s.origin [0];
		org.y = ent->s.origin [1];
		org.z = ent->s.origin [2];
	}
	else
	{
		//	view entity is the client
		org.x = clent->client->ps.pmove.origin [0] * 0.125 + clent->client->ps.viewoffset [0];
		org.y = clent->client->ps.pmove.origin [1] * 0.125 + clent->client->ps.viewoffset [1];
		org.z = clent->client->ps.pmove.origin [2] * 0.125 + clent->client->ps.viewoffset [2];
	}
//	for (i=0 ; i<3 ; i++)
//		org[i] = clent->client->ps.pmove.origin[i]*0.125 + clent->client->ps.viewoffset[i];

	leafnum = CM_PointLeafnum (org);

	clientarea = CM_LeafArea (leafnum);
	clientcluster = CM_LeafCluster (leafnum);

	// calculate the visible areas
	frame->areabytes = CM_WriteAreaBits (frame->areabits, clientarea);

	// grab the current player_state_t
	frame->ps = clent->client->ps;

	SV_FatPVS (org);
	clientphs = CM_ClusterPHS (clientcluster);
	// build up the list of visible entities
	frame->num_entities = 0;
	frame->first_entity = svs.next_client_entities;
	c_fullsend = 0;
	for (e=1 ; e<ge->num_edicts ; e++)
 	{
		ent = EDICT_NUM(e);

		checkpvs=true;
		if (ent == clent)
			checkpvs=false;

		if( ent->solid == SOLID_BSP )
		{
			ent->s.origin_hack.x = ent->absmin.x + ( ( ent->absmax.x - ent->absmin.x ) * 0.5 );
			ent->s.origin_hack.y = ent->absmin.y + ( ( ent->absmax.y - ent->absmin.y ) * 0.5 );
			ent->s.origin_hack.z = ent->absmin.z + ( ( ent->absmax.z - ent->absmin.z ) * 0.5 );
		}

		// send to everyone but owner
		// (lots of owner->owner->owner checks because of hierarchical models)
		// (btw, this was created for the nightmare weapon ... you don't see your models, but others do)
		if (ent->svflags & SVF_NOTOWNER && ent->owner)
		{
			if (ent->owner->s.number==clientnum)
				continue;

			if (ent->owner->owner)
			{
				if (ent->owner->owner->s.number==clientnum)
					continue;

				if ((ent->owner->owner->owner) && (ent->owner->owner->owner->s.number==clientnum))
					continue;
			}
		}

		// as far as i know, only used by weapons.
		// used to only send the weapon to its client (owner).
		// now, sends entity to all clients and sets/clears RF_NODRAW
		// depending on which client it's sending it to. (owner's draw, non-owner doesn't draw)
		if (ent->svflags & SVF_OWNERONLY && ent->owner)
			checkpvs=true;
/*
      {
			if (ent->owner->s.number != clientnum)
         {
            checkpvs=true;
            ent->s.renderfx |= RF_NODRAW;
         }
         else
         {
            checkpvs=false;
            ent->s.renderfx &= ~RF_NODRAW;
         }
      }
*/
		// these entities are handled with special client-side only lists
//		if (ent->s.renderfx & (RF_SNOW|RF_RAIN|RF_DRIP))
		if (ent->s.renderfx & RF_PARTICLEVOLUME)
		{
			continue;
		}

		// let all track entities pass!!!
		if (ent->s.renderfx & (RF_TRACKENT))//|RF_SPOTLIGHT))  // amw - added so spotlights way up in the air work
		{
			checkpvs=false;
		}
		else
		{
   			// ignore ents without visible models		
	   		if (ent->svflags & SVF_NOCLIENT)
		   		continue;

   			// ignore ents without visible models unless they have an effect
	   		if (!ent->s.modelindex && !ent->s.effects && !ent->s.sound
		   		&& !ent->s.event && !ent->s.renderfx)
				continue;
		}

		// ignore if not touching a PV leaf
		if (checkpvs)
		{
			// check area
			if (!CM_AreasConnected (clientarea, ent->areanum))
			{	// doors can legally straddle two areas, so
				// we may need to check another one
				if (!ent->areanum2 || !CM_AreasConnected (clientarea, ent->areanum2))
					continue;		// blocked by a door
			}
			// beams just check one point for PHS
			if (ent->s.renderfx & (RF_BEAM|RF_SPOTLIGHT))
			{
				l = ent->clusternums[0];
				if ( !(clientphs[l >> 3] & (1 << (l&7) )) )
					continue;
			}
			else
			{
				// FIXME: if an ent has a model and a sound, but isn't
				// in the PVS, only the PHS, clear the model
				if (ent->s.sound)
				{
					// mdm99.06.17 - changed back to using the phs
					bitvector = fatpvs;	//clientphs;
				}
				else
					bitvector = fatpvs;
				if (ent->num_clusters == -1)
				{	// too many leafs for individual check, go by headnode
					if (!CM_HeadnodeVisible (ent->headnode, bitvector))
						continue;
					c_fullsend++;
				}
				else
				{	// check individual leafs
					for (i=0 ; i < ent->num_clusters ; i++)
					{
						l = ent->clusternums[i];
						if (bitvector[l >> 3] & (1 << (l&7) ))
							break;
					}
					if (i == ent->num_clusters)
						continue;		// not visible
				}
				// mdm99.06.17 - sounds are mapped defined attn  screw this
//				if (!ent->s.modelindex)
//				{	// don't send sounds if they will be attenuated away
//					CVector	delta;
//					float	len;
//					delta = org - ent->s.origin;
//					len = delta.Length();
//					if (len > 400)
//						continue;
//				}
			}
		}
		// add it to the circular client_entities array
		state = &svs.client_entities[svs.next_client_entities%svs.num_client_entities];
		if (ent->s.number != e)
		{
			Com_DPrintf ("FIXING ENT->S.NUMBER!!!\n");
			ent->s.number = e;
		}
		*state = ent->s;

		// don't mark players missiles as solid
		if (ent->owner == client->edict)
			state->solid = 0;

		svs.next_client_entities++;
		frame->num_entities++;

      if (ent->s.renderfx & RF_MUZZLEFLASH)
         ent->s.renderfx &= ~RF_MUZZLEFLASH;
	}
}


/*
==================
SV_RecordDemoMessage
Save everything in the world out without deltas.
Used for recording footage for merged or assembled demos
==================
*/
void SV_RecordDemoMessage (void)
{
	int			e;
	edict_t		*ent;
	entity_state_t	nostate;
	sizebuf_t	buf;
	byte		buf_data[32768];
	int			len;
	if (!svs.demofile)
		return;
	memset (&nostate, 0, sizeof(nostate));
	SZ_Init (&buf, buf_data, sizeof(buf_data));
	// write a frame message that doesn't contain a player_state_t
	MSG_WriteByte (&buf, svc_frame);
	MSG_WriteLong (&buf, sv.framenum);
	MSG_WriteByte (&buf, svc_packetentities);
	for (e=1, ent=EDICT_NUM(e) ; e<ge->num_edicts ; e++, ent++)
	{
		// invisible trigger
		if (ent->svflags & SVF_NOCLIENT)
			continue;
		// ignore ents without visible models unless they have an effect
		if (!ent->s.modelindex && !ent->s.effects && !ent->s.sound
			&& !ent->s.event)
			continue;
		MSG_WriteDeltaEntity (&nostate, &ent->s, &buf, false);

		//	Nelno:	make sure FRAME_FORCEINDEX gets cleared...  what a load of shit
		ent->s.frameInfo.frameFlags &= ~FRAME_FORCEINDEX;
	}
	MSG_WriteShort (&buf, 0);		// end of packetentities
	// now add the accumulated multicast information
	SZ_Write (&buf, svs.demo_multicast.data, svs.demo_multicast.cursize);
	SZ_Clear (&svs.demo_multicast);
	// now write the entire message to the file, prefixed by the length
	len = LittleLong (buf.cursize);
	fwrite (&len, 4, 1, svs.demofile);
	fwrite (buf.data, buf.cursize, 1, svs.demofile);
}
