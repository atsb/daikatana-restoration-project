// world.c -- world query functions
#include "server.h"
/*
===============================================================================
ENTITY AREA CHECKING
FIXME: this use of "area" is different from the bsp file use
===============================================================================
*/
#define	STRUCT_FROM_LINK(l,t,m) ((t *)((byte *)l - (byte *)&(((t *)0)->m)))
#define	EDICT_FROM_AREA(l) STRUCT_FROM_LINK(l,edict_t,area)
typedef struct areanode_s
{
	int		axis;		// -1 = leaf node
	float	dist;
	struct areanode_s	*children[2];
	link_t	trigger_edicts;
	link_t	solid_edicts;
} areanode_t;
#define	AREA_DEPTH	4
#define	AREA_NODES	32
areanode_t	sv_areanodes[AREA_NODES];
int			sv_numareanodes;
CVector		area_mins, area_maxs;
edict_t	**area_list;
int		area_count, area_maxcount;
int		area_type;
int SV_HullForEntity (edict_t *ent);
// ClearLink is used for new headnodes
void ClearLink (link_t *l)
{
	l->prev = l->next = l;
}
void RemoveLink (link_t *l)
{
	l->next->prev = l->prev;
	l->prev->next = l->next;
}
void InsertLinkBefore (link_t *l, link_t *before)
{
	l->next = before;
	l->prev = before->prev;
	l->prev->next = l;
	l->next->prev = l;
}
/*
===============
SV_CreateAreaNode
Builds a uniformly subdivided tree for the given world size
===============
*/
areanode_t *SV_CreateAreaNode (int depth, const CVector &mins, const CVector &maxs)
{
	areanode_t	*anode;
	CVector		size;
	CVector		mins1, maxs1, mins2, maxs2;
	anode = &sv_areanodes[sv_numareanodes];
	sv_numareanodes++;
	ClearLink (&anode->trigger_edicts);
	ClearLink (&anode->solid_edicts);
	
	if (depth == AREA_DEPTH)
	{
		anode->axis = -1;
		anode->children[0] = anode->children[1] = NULL;
		return anode;
	}
	
	size = maxs - mins;
	if (size[0] > size[1])
		anode->axis = 0;
	else
		anode->axis = 1;
	
	anode->dist = 0.5 * (maxs[anode->axis] + mins[anode->axis]);
	mins1 = mins;
	mins2 = mins;	
	maxs1 = maxs;	
	maxs2 = maxs;	
	
	maxs1.SetValue( anode->dist, anode->axis );
	mins2.SetValue( anode->dist, anode->axis );

	anode->children[0] = SV_CreateAreaNode (depth+1, mins2, maxs2);
	anode->children[1] = SV_CreateAreaNode (depth+1, mins1, maxs1);
	return anode;
}
/*
===============
SV_ClearWorld
===============
*/
void SV_ClearWorld (void)
{
	memset (sv_areanodes, 0, sizeof(sv_areanodes));
	sv_numareanodes = 0;
	SV_CreateAreaNode (0, sv.models[1]->mins, sv.models[1]->maxs);
}
/*
===============
SV_UnlinkEdict
===============
*/
void SV_UnlinkEdict (edict_t *ent)
{
	if (!ent->area.prev)
		return;		// not linked in anywhere
	RemoveLink (&ent->area);
	ent->area.prev = ent->area.next = NULL;
}
/*
===============
SV_LinkEdict
===============
*/
#define MAX_TOTAL_ENT_LEAFS		128
void SV_LinkEdict (edict_t *ent)
{
	areanode_t	*node;
	int			leafs[MAX_TOTAL_ENT_LEAFS];
	int			clusters[MAX_TOTAL_ENT_LEAFS];
	int			num_leafs;
	int			i, j, k;
	int			area;
	int			topnode;
	if (ent->area.prev)
		SV_UnlinkEdict (ent);	// unlink from old position
		
	if (ent == ge->edicts)
		return;		// don't add the world
	if (!ent->inuse)
		return;
	// set the size
	ent->size = ent->s.maxs - ent->s.mins;
	
	// encode the size into the entity_state for client prediction
	if (ent->solid == SOLID_BBOX && !(ent->svflags & SVF_DEADMONSTER) )
	{	
		if (ent->s.iflags & IF_SV_SENDBBOX)
		{
			ent->s.solid = SOLID_BBOX;
		}
		else
		{
			// assume that x/y are equal and symetric
			i = ent->s.maxs.x/8;
			if (i<1)
				i = 1;
			if (i>31)
				i = 31;

			// z is not symetric
			j = (-ent->s.mins.z)/8;
			if (j<1)
				j = 1;
			if (j>31)
				j = 31;

			// and z maxs can be negative...
			k = (ent->s.maxs.z+32)/8;
			if (k<1)
				k = 1;
			if (k>63)
				k = 63;

			ent->s.solid = (k<<10) | (j<<5) | i;
		}
	}
	else if (ent->solid == SOLID_BSP)
	{
		ent->s.solid = 31;		// a solid_bbox will never create this value
	}
	else
		ent->s.solid = 0;
	// set the abs box
	if (ent->solid == SOLID_BSP && 
	(ent->s.angles.x || ent->s.angles.y || ent->s.angles.z) )
	{
		// expand for rotation
		float		max, v;
		int			i;
		max = 0;
		for (i=0 ; i<3 ; i++)
		{
			v =fabs( ent->s.mins[i]);
			if (v > max)
				max = v;
			v =fabs( ent->s.maxs[i]);
			if (v > max)
				max = v;
		}

		ent->absmin.x = ent->s.origin.x - max;
		ent->absmin.y = ent->s.origin.y - max;
		ent->absmin.z = ent->s.origin.z - max;

		ent->absmax.x = ent->s.origin.x + max;
		ent->absmax.y = ent->s.origin.y + max;
		ent->absmax.z = ent->s.origin.z + max;
	}
/*
	if( ent->solid == SOLID_BSP && ( ent->s.angles.x || ent->s.angles.y || ent->s.angles.z ) )
	{
		if( ent->s.angles.x )
		{
			ent->absmin.x = ent->s.origin.x - ( ent->s.mins.x * 1.14 );
			ent->absmax.x = ent->s.origin.x + ( ent->s.maxs.x * 1.14 );
		}
		if( ent->s.angles.y )
		{
			ent->absmin.y = ent->s.origin.y - ( ent->s.mins.y * 1.14 );
			ent->absmax.y = ent->s.origin.y + ( ent->s.maxs.y * 1.14 );
		}
		if( ent->s.angles.z )
		{
			ent->absmin.z = ent->s.origin.z - ( ent->s.mins.z * 1.14 );
			ent->absmax.z = ent->s.origin.z + ( ent->s.maxs.z * 1.14 );
		}
	}
*/
	else
	{	// normal
		ent->absmin = ent->s.origin + ent->s.mins;	
		ent->absmax = ent->s.origin + ent->s.maxs;
	}
	// because movement is clipped an epsilon away from an actual edge,
	// we must fully check even when bounding boxes don't quite touch
	ent->absmin.x -= 1;
	ent->absmin.y -= 1;
	ent->absmin.z -= 1;
	ent->absmax.x += 1;
	ent->absmax.y += 1;
	ent->absmax.z += 1;
// link to PVS leafs
	ent->num_clusters = 0;
	ent->areanum = 0;
	ent->areanum2 = 0;
	//get all leafs, including solids
	num_leafs = CM_BoxLeafnums (ent->absmin, ent->absmax,
		leafs, MAX_TOTAL_ENT_LEAFS, &topnode);
	// set areas
	for (i=0 ; i<num_leafs ; i++)
	{
		clusters[i] = CM_LeafCluster (leafs[i]);
		area = CM_LeafArea (leafs[i]);
		if (area)
		{	// doors may legally straggle two areas,
			// but nothing should evern need more than that
			if (ent->areanum && ent->areanum != area)
			{
				if (ent->areanum2 && ent->areanum2 != area && sv.state == ss_loading)
					Com_DPrintf ("Object touching 3 areas at %f %f %f\n",
					ent->absmin[0], ent->absmin[1], ent->absmin[2]);
				ent->areanum2 = area;
			}
			else
				ent->areanum = area;
		}
	}
	if (num_leafs >= MAX_TOTAL_ENT_LEAFS)
	{	// assume we missed some leafs, and mark by headnode
		ent->num_clusters = -1;
		ent->headnode = topnode;
	}
	else
	{
		ent->num_clusters = 0;
		for (i=0 ; i<num_leafs ; i++)
		{
			if (clusters[i] == -1)
				continue;		// not a visible leaf
			for (j=0 ; j<i ; j++)
				if (clusters[j] == clusters[i])
					break;
			if (j == i)
			{
				if (ent->num_clusters == MAX_ENT_CLUSTERS)
				{	// assume we missed some leafs, and mark by headnode
					ent->num_clusters = -1;
					ent->headnode = topnode;
					break;
				}
				ent->clusternums[ent->num_clusters++] = clusters[i];
			}
		}
	}
	// if first time, make sure old_origin is valid
	if (!ent->linkcount)
	{
		ent->s.old_origin = ent->s.origin;
	}
	ent->linkcount++;

	if (ent->solid == SOLID_NOT)
		return;
// find the first node that the ent's box crosses
	node = sv_areanodes;
	while (1)
	{
		if (node->axis == -1)
			break;
		if (ent->absmin[node->axis] > node->dist)
			node = node->children[0];
		else if (ent->absmax[node->axis] < node->dist)
			node = node->children[1];
		else
			break;		// crosses the node
	}
	
	// link it in	
	if (ent->solid == SOLID_TRIGGER)
		InsertLinkBefore (&ent->area, &node->trigger_edicts);
	else
		InsertLinkBefore (&ent->area, &node->solid_edicts);
}
/*
====================
SV_AreaEdicts_r
====================
*/
#pragma inline_recursion(on)
void inline SV_AreaEdicts_r (areanode_t *node)
{
	link_t		*l, *next, *start;
	edict_t		*check;
	int			count;
	count = 0;
	// touch linked edicts
	if (area_type == AREA_SOLID)
		start = &node->solid_edicts;
	else
		start = &node->trigger_edicts;
	for (l=start->next  ; l != start ; l = next)
	{
		next = l->next;
		check = EDICT_FROM_AREA(l);
		if (check->solid == SOLID_NOT)
			continue;		// deactivated
		if (check->absmin.x > area_maxs.x
		|| check->absmin.y > area_maxs.y
		|| check->absmin.z > area_maxs.z
		|| check->absmax.x < area_mins.x
		|| check->absmax.y < area_mins.y
		|| check->absmax.z < area_mins.z)
			continue;		// not touching
		if (area_count == area_maxcount)
		{
			Com_Printf ("SV_AreaEdicts: MAXCOUNT\n");
			return;
		}
		area_list[area_count] = check;
		area_count++;
	}
	
	if (node->axis == -1)
		return;		// terminal node
	// recurse down both sides
	if ( area_maxs[node->axis] > node->dist )
		SV_AreaEdicts_r ( node->children[0] );
	if ( area_mins[node->axis] < node->dist )
		SV_AreaEdicts_r ( node->children[1] );
}
#pragma inline_recursion(off)

///////////////////////////////////////////////////////////////////////////////
//	SV_AreaEdicts
//
//	Nelno:	returns all of the edicts which have any part of their bounding box
//			inside the area specified by mins/maxs
///////////////////////////////////////////////////////////////////////////////

int SV_AreaEdicts ( CVector &mins, CVector &maxs, edict_t **list,
	int maxcount, int areatype)
{
 	area_mins = mins;
	area_maxs = maxs;
	area_list = list;
	area_count = 0;
	area_maxcount = maxcount;
	area_type = areatype;
	SV_AreaEdicts_r (sv_areanodes);
	return area_count;
}
//===========================================================================
/*
=============
SV_PointContents
=============
*/
int SV_PointContents (CVector &p)
{
	edict_t		*touch[MAX_EDICTS], *hit;
	int			i, num;
	int			contents, c2;
	int			headnode;
	CVector		angles;
	// get base contents from world
	contents = CM_PointContents (p, sv.models[1]->headnode);
	// or in contents from all the other entities
	num = SV_AreaEdicts (p, p, touch, MAX_EDICTS, AREA_SOLID);
	for (i=0 ; i<num ; i++)
	{
		hit = touch[i];

        if ( !hit || hit == (edict_t*)0xcccccccc )
            continue;

		// might intersect, so do an exact clip
		headnode = SV_HullForEntity (hit);
		angles = hit->s.angles;
		if (hit->solid != SOLID_BSP)
			angles = vec3_origin;	// boxes don't rotate
		c2 = CM_TransformedPointContents (p, headnode, hit->s.origin, hit->s.angles);
		contents |= c2;
	}
	return contents;
}
typedef struct
{
	CVector		boxmins, boxmaxs;// enclose the test object along entire move
	CVector		mins, maxs;	// size of the moving object
	CVector		mins2, maxs2;	// size when clipping against mosnters
	CVector		start, end;
	trace_t		trace;
	edict_t		*passedict;
	int			contentmask;
} moveclip_t;
/*
================
SV_HullForEntity
Returns a headnode that can be used for testing or clipping an
object of mins/maxs size.
Offset is filled in to contain the adjustment that must be added to the
testing object's origin to get a point to use with the returned hull.
================
*/
int SV_HullForEntity (edict_t *ent)
{
	cmodel_t	*model;
// decide which clipping hull to use, based on the size
	if (ent->solid == SOLID_BSP)
	{	// explicit hulls in the BSP model
		model = sv.models[ ent->s.modelindex ];
		if (!model)
			Com_Error (ERR_FATAL, "MOVETYPE_PUSH with a non bsp model");
		return model->headnode;
	}
	// create a temp hull from bounding box sizes
	return CM_HeadnodeForBox (ent->s.mins, ent->s.maxs);
}
//===========================================================================
/*
====================
SV_ClipMoveToEntities
====================
*/
void SV_ClipMoveToEntities ( moveclip_t *clip )
{
	int			i, num;
	edict_t		*touchlist[MAX_EDICTS], *touch;
	trace_t		trace;
	int			headnode;
	CVector		angles;
	memset( touchlist, 0, sizeof( touchlist ) );
	num = SV_AreaEdicts (clip->boxmins, clip->boxmaxs, touchlist
		, MAX_EDICTS, AREA_SOLID);
	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for (i=0 ; i<num ; i++)
	{
		touch = touchlist[i];
		
		// ISP 07-02-99 make sure touch is a valid pointer
        if (!touch || touch == (edict_t*)0xcccccccc || touch->solid == SOLID_NOT)
			continue;
		
		if (touch == clip->passedict)
			continue;
		
		if (clip->trace.allsolid)
			return;

		if (clip->passedict)
		{
	 		if (touch->owner == clip->passedict)
				continue;	// don't clip against own missiles

			// SCG[8/27/99]: make sure we only test for missiles of the same class
//			if( strcmp( touch->className, clip->passedict->className ) )
			{
				if (clip->passedict->owner == touch)
					continue;	// don't clip against owner
			}
		}

		if ( !(clip->contentmask & CONTENTS_DEADMONSTER)
		&& (touch->svflags & SVF_DEADMONSTER) )
				continue;

		if (touch->svflags & SVF_SHOT && clip->contentmask == MASK_SHOT)
			continue;

		// might intersect, so do an exact clip
		headnode = SV_HullForEntity (touch);
		angles = touch->s.angles;

		if (touch->solid != SOLID_BSP)
			angles = vec3_origin;	// boxes don't rotate

		if (touch->svflags & SVF_MONSTER)
			trace = CM_TransformedBoxTrace (clip->start, clip->end,
				clip->mins2, clip->maxs2, headnode, clip->contentmask,
				touch->s.origin, angles);
		else
			trace = CM_TransformedBoxTrace (clip->start, clip->end,
				clip->mins, clip->maxs, headnode,  clip->contentmask,
				touch->s.origin, angles);

		if (trace.allsolid || trace.startsolid || trace.fraction < clip->trace.fraction)
		{
			trace.ent = touch;
		 	if (clip->trace.startsolid)
			{
				clip->trace = trace;
				clip->trace.startsolid = true;
			}
			else
				clip->trace = trace;
		}
		else if (trace.startsolid)
			clip->trace.startsolid = true;
	}
}

/*
====================
SV_ClipMoveToEntities
====================
*/
void SV_ClipMoveToTriggers ( moveclip_t *clip, unsigned int nAcceptFlag )
{
	int			i, num;
	edict_t		*touchlist[MAX_EDICTS], *touch;
	trace_t		trace;
	int			headnode;
	CVector		angles;

	memset( touchlist, 0, sizeof( touchlist ) );
	
	num = SV_AreaEdicts( clip->boxmins, clip->boxmaxs, touchlist, MAX_EDICTS, AREA_TRIGGERS );

	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for( i = 0; i < num; i++ )
	{
		touch = touchlist[i];

		// SCG[3/9/00]: check for the accept flag
		if( ( nAcceptFlag != 0 ) && !( touch->svflags & nAcceptFlag ) )
		{
			continue;
		}

        if( touch->solid != SOLID_TRIGGER )
		{
			continue;
		}
		
		if( touch == clip->passedict )
		{
			continue;
		}
		
		if( clip->trace.allsolid )
		{
			return;
		}

		if( clip->passedict )
		{
	 		if( touch->owner == clip->passedict )
			{
				continue;
			}

			if (clip->passedict->owner == touch)
			{
				continue;
			}
		}

		// might intersect, so do an exact clip
		headnode = SV_HullForEntity( touch );
		angles = touch->s.angles;

		if (touch->solid != SOLID_BSP )
			angles = vec3_origin;	// boxes don't rotate

		trace = CM_TransformedBoxTrace (clip->start, clip->end,
				clip->mins, clip->maxs, headnode,  clip->contentmask,
				touch->s.origin, angles);

		if (trace.allsolid || trace.startsolid || trace.fraction < clip->trace.fraction)
		{
			trace.ent = touch;
		 	if (clip->trace.startsolid)
			{
				clip->trace = trace;
				clip->trace.startsolid = true;
			}
			else
				clip->trace = trace;
		}
		else if (trace.startsolid)
			clip->trace.startsolid = true;
	}
}
/*
==================
SV_TraceBounds
==================
*/
void SV_TraceBounds (const CVector &start, const CVector &mins, const CVector &maxs, 
				const CVector &end, CVector &boxmins, CVector &boxmaxs)
{
#if 0
// debug to test against everything
boxmins[0] = boxmins[1] = boxmins[2] = -9999;
boxmaxs[0] = boxmaxs[1] = boxmaxs[2] = 9999;
#else
	if (end.x > start.x)
	{
		boxmins.x = start.x + mins.x - 1;
		boxmaxs.x = end.x + maxs.x + 1;
	}
	else
	{
		boxmins.x = end.x + mins.x - 1;
		boxmaxs.x = start.x + maxs.x + 1;
	}

	if (end.y > start.y)
	{
		boxmins.y = start.y + mins.y - 1;
		boxmaxs.y = end.y + maxs.y + 1;
	}
	else
	{
		boxmins.y = end.y + mins.y - 1;
		boxmaxs.y = start.y + maxs.y + 1;
	}

	if (end.z > start.z)
	{
		boxmins.z = start.z + mins.z - 1;
		boxmaxs.z = end.z + maxs.z + 1;
	}
	else
	{
		boxmins.z = end.z + mins.z - 1;
		boxmaxs.z = start.z + maxs.z + 1;
	}
#endif
}
/*
==================
SV_Trace
Moves the given mins/maxs volume through the world from start to end.
Passedict and edicts owned by passedict are explicitly not checked.
==================
*/

int traceCount;  // TTD: does this need to be initialized to 0??  3.4 dsn

trace_t SV_TraceLineTrigger( CVector &start, CVector &end, edict_t *passedict, int contentmask, unsigned int nAcceptFlag )
{
	moveclip_t	clip;

	traceCount++;

	CVector mins = vec3_origin;
	CVector maxs = vec3_origin;
	memset ( &clip, 0, sizeof ( moveclip_t ) );
	// clip to world
	clip.trace = CM_BoxTrace (start, end, mins, maxs, 0, contentmask );
	
/*
	if ((clip.trace.plane.normal.x == 0.0) && 
		(clip.trace.plane.normal.y == 0.0) && 
		(clip.trace.plane.normal.z == 0.0) && 
		!clip.trace.startsolid && 
		!clip.trace.allsolid)
	{
		clip.trace.ent = NULL;
	}
	else
*/	{
		clip.trace.ent = ge->edicts;
	}

	if (clip.trace.fraction == 0)
		return clip.trace;		// blocked by the world

	clip.contentmask = contentmask;
	clip.start = start;
	clip.end = end;
	clip.mins = mins;
	clip.maxs = maxs;
	clip.passedict = passedict;
	clip.mins2 = mins;
	clip.maxs2 = maxs;
	
	// create the bounding box of the entire move
	SV_TraceBounds ( start, clip.mins2, clip.maxs2, end, clip.boxmins, clip.boxmaxs );
	// clip to other solid entities
	SV_ClipMoveToTriggers ( &clip, nAcceptFlag );
	return clip.trace;
}

trace_t SV_TraceLine(CVector &start, CVector &end, edict_t *passedict, int contentmask)
{
	moveclip_t	clip;

	traceCount++;

	CVector mins = vec3_origin;
	CVector maxs = vec3_origin;
	memset ( &clip, 0, sizeof ( moveclip_t ) );
	// clip to world
	clip.trace = CM_BoxTrace (start, end, mins, maxs, 0, contentmask);
	
/*
	if ((clip.trace.plane.normal.x == 0.0) && 
		(clip.trace.plane.normal.y == 0.0) && 
		(clip.trace.plane.normal.z == 0.0) && 
		!clip.trace.startsolid && 
		!clip.trace.allsolid)
	{
		clip.trace.ent = NULL;
	}
	else
*/	{
		clip.trace.ent = ge->edicts;
	}

	if (clip.trace.fraction == 0)
		return clip.trace;		// blocked by the world

	clip.contentmask = contentmask;
	clip.start = start;
	clip.end = end;
	clip.mins = mins;
	clip.maxs = maxs;
	clip.passedict = passedict;
	clip.mins2 = mins;
	clip.maxs2 = maxs;
	
	// create the bounding box of the entire move
	SV_TraceBounds ( start, clip.mins2, clip.maxs2, end, clip.boxmins, clip.boxmaxs );
	// clip to other solid entities
	SV_ClipMoveToEntities ( &clip );
	return clip.trace;
}

trace_t SV_TraceBox(CVector &start, CVector &mins, CVector &maxs, CVector &end, edict_t *passedict, int contentmask)
{
	moveclip_t	clip;

	traceCount++;

	if (!mins)
		mins = vec3_origin;
	if (!maxs)
		maxs = vec3_origin;

	memset ( &clip, 0, sizeof ( moveclip_t ) );	// Ash: this is an unnecessary memset.
	
	// clip to world
	clip.trace = CM_BoxTrace (start, end, mins, maxs, 0, contentmask);

/*
	if ((clip.trace.plane.normal.x == 0.0) && 
		(clip.trace.plane.normal.y == 0.0) && 
		(clip.trace.plane.normal.z == 0.0) && 
		!clip.trace.startsolid && 
		!clip.trace.allsolid)
	{
		clip.trace.ent = NULL;
	}
	else
*/	{
		clip.trace.ent = ge->edicts;
	}

	if (clip.trace.fraction == 0)
		return clip.trace;		// blocked by the world

	clip.contentmask = contentmask;
	clip.start = start;
	clip.end = end;
	clip.mins = mins;
	clip.maxs = maxs;
	clip.passedict = passedict;
	clip.mins2 = mins;
	clip.maxs2 = maxs;
	
	// create the bounding box of the entire move
	SV_TraceBounds ( start, clip.mins2, clip.maxs2, end, clip.boxmins, clip.boxmaxs );
	// clip to other solid entities
	SV_ClipMoveToEntities ( &clip );

	return clip.trace;
}

// returns the number of traces done during the last frame
int SV_GetTraceCount(void)
{
	return (traceCount);
}

