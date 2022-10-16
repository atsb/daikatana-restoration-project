// r_bsp.c

#include "r_local.h"

//
// current entity info
//
qboolean		insubmodel;
entity_t		*currententity;
CVector			modelorg;		// modelorg is the viewpoint reletive to
								// the currently rendering entity
CVector			r_entorigin;	// the currently rendering entity in world
								// coordinates

float			entity_rotation[3][3];

int				r_currentbkey;

typedef enum {touchessolid, drawnode, nodrawnode} solidstate_t;

#define MAX_BMODEL_VERTS	500			// 6K
#define MAX_BMODEL_EDGES	1000		// 12K

static mvertex_t	*pbverts;
static bedge_t		*pbedges;
static int			numbverts, numbedges;

static mvertex_t	*pfrontenter, *pfrontexit;

static qboolean		makeclippededge;


//===========================================================================

/*
================
R_EntityRotate
================
*/
void R_EntityRotate (CVector &vec)
{
	CVector	tvec;

	tvec = vec;
	vec.x = DotProduct (entity_rotation[0], tvec);
	vec.y = DotProduct (entity_rotation[1], tvec);
	vec.z = DotProduct (entity_rotation[2], tvec);
}


/*
================
R_RotateBmodel
================
*/
void R_RotateBmodel (void)
{
	float	angle, s, c, temp1[3][3], temp2[3][3], temp3[3][3];

// TODO: should use a look-up table
// TODO: should really be stored with the entity instead of being reconstructed
// TODO: could cache lazily, stored in the entity
// TODO: share work with R_SetUpAliasTransform

// yaw
	angle = currententity->angles[YAW];		
	angle = angle * M_PI*2 / 360;
	s = sin(angle);
	c = cos(angle);

	temp1[0][0] = c;
	temp1[0][1] = s;
	temp1[0][2] = 0;
	temp1[1][0] = -s;
	temp1[1][1] = c;
	temp1[1][2] = 0;
	temp1[2][0] = 0;
	temp1[2][1] = 0;
	temp1[2][2] = 1;


// pitch
	angle = currententity->angles[PITCH];		
	angle = angle * M_PI*2 / 360;
	s = sin(angle);
	c = cos(angle);

	temp2[0][0] = c;
	temp2[0][1] = 0;
	temp2[0][2] = -s;
	temp2[1][0] = 0;
	temp2[1][1] = 1;
	temp2[1][2] = 0;
	temp2[2][0] = s;
	temp2[2][1] = 0;
	temp2[2][2] = c;

	R_ConcatRotations (temp2, temp1, temp3);

// roll
	angle = currententity->angles[ROLL];		
	angle = angle * M_PI*2 / 360;
	s = sin(angle);
	c = cos(angle);

	temp1[0][0] = 1;
	temp1[0][1] = 0;
	temp1[0][2] = 0;
	temp1[1][0] = 0;
	temp1[1][1] = c;
	temp1[1][2] = s;
	temp1[2][0] = 0;
	temp1[2][1] = -s;
	temp1[2][2] = c;

	R_ConcatRotations (temp1, temp3, entity_rotation);

//
// rotate modelorg and the transformation matrix
//
	R_EntityRotate (modelorg);
	R_EntityRotate (vpn);
	R_EntityRotate (vright);
	R_EntityRotate (vup);

	R_TransformFrustum ();
}


/*
================
R_RecursiveClipBPoly

Clip a bmodel poly down the world bsp tree
================
*/
void R_RecursiveClipBPoly (bedge_t *pedges, mnode_t *pnode, msurface_t *psurf)
{
	bedge_t		*psideedges[2], *pnextedge, *ptedge;
	int			i, side, lastside;
	float		dist, frac, lastdist;
	mplane_t	*splitplane, tplane;
	mvertex_t	*pvert, *plastvert, *ptvert;
	mnode_t		*pn;
	int			area;

	psideedges[0] = psideedges[1] = NULL;

	makeclippededge = false;

// transform the BSP plane into model space
// FIXME: cache these?
	splitplane = pnode->plane;
	tplane.dist = splitplane->dist -
			DotProduct(r_entorigin, splitplane->normal);
	tplane.normal.x = DotProduct (entity_rotation[0], splitplane->normal);
	tplane.normal.y = DotProduct (entity_rotation[1], splitplane->normal);
	tplane.normal.z = DotProduct (entity_rotation[2], splitplane->normal);

// clip edges to BSP plane
	for ( ; pedges ; pedges = pnextedge)
	{
		pnextedge = pedges->pnext;

	// set the status for the last point as the previous point
	// FIXME: cache this stuff somehow?
		plastvert = pedges->v[0];
		lastdist = DotProduct (plastvert->position, tplane.normal) -
				   tplane.dist;

		if (lastdist > 0)
			lastside = 0;
		else
			lastside = 1;

		pvert = pedges->v[1];

		dist = DotProduct (pvert->position, tplane.normal) - tplane.dist;

		if (dist > 0)
			side = 0;
		else
			side = 1;

		if (side != lastside)
		{
		// clipped
			if (numbverts >= MAX_BMODEL_VERTS)
				return;

		// generate the clipped vertex
			frac = lastdist / (lastdist - dist);
			ptvert = &pbverts[numbverts++];
			ptvert->position.x = plastvert->position.x +
					frac * (pvert->position.x -
					plastvert->position.x);
			ptvert->position.y = plastvert->position.y +
					frac * (pvert->position.y -
					plastvert->position.y);
			ptvert->position.z = plastvert->position.z +
					frac * (pvert->position.z -
					plastvert->position.z);

		// split into two edges, one on each side, and remember entering
		// and exiting points
		// FIXME: share the clip edge by having a winding direction flag?
			if (numbedges >= (MAX_BMODEL_EDGES - 1))
			{
				ri.Con_Printf (PRINT_ALL,"Out of edges for bmodel\n");
				return;
			}

			ptedge = &pbedges[numbedges];
			ptedge->pnext = psideedges[lastside];
			psideedges[lastside] = ptedge;
			ptedge->v[0] = plastvert;
			ptedge->v[1] = ptvert;

			ptedge = &pbedges[numbedges + 1];
			ptedge->pnext = psideedges[side];
			psideedges[side] = ptedge;
			ptedge->v[0] = ptvert;
			ptedge->v[1] = pvert;

			numbedges += 2;

			if (side == 0)
			{
			// entering for front, exiting for back
				pfrontenter = ptvert;
				makeclippededge = true;
			}
			else
			{
				pfrontexit = ptvert;
				makeclippededge = true;
			}
		}
		else
		{
		// add the edge to the appropriate side
			pedges->pnext = psideedges[side];
			psideedges[side] = pedges;
		}
	}

// if anything was clipped, reconstitute and add the edges along the clip
// plane to both sides (but in opposite directions)
	if (makeclippededge)
	{
		if (numbedges >= (MAX_BMODEL_EDGES - 2))
		{
			ri.Con_Printf (PRINT_ALL,"Out of edges for bmodel\n");
			return;
		}

		ptedge = &pbedges[numbedges];
		ptedge->pnext = psideedges[0];
		psideedges[0] = ptedge;
		ptedge->v[0] = pfrontexit;
		ptedge->v[1] = pfrontenter;

		ptedge = &pbedges[numbedges + 1];
		ptedge->pnext = psideedges[1];
		psideedges[1] = ptedge;
		ptedge->v[0] = pfrontenter;
		ptedge->v[1] = pfrontexit;

		numbedges += 2;
	}

// draw or recurse further
	for (i=0 ; i<2 ; i++)
	{
		if (psideedges[i])
		{
		// draw if we've reached a non-solid leaf, done if all that's left is a
		// solid leaf, and continue down the tree if it's not a leaf
			pn = pnode->children[i];

		// we're done with this branch if the node or leaf isn't in the PVS
			if (pn->visframe == r_visframecount)
			{
				if (pn->contents != CONTENTS_NODE)
				{
					if (pn->contents != CONTENTS_SOLID)
					{
						if (r_newrefdef.areabits)
						{
							area = ((mleaf_t *)pn)->area;
							if (! (r_newrefdef.areabits[area>>3] & (1<<(area&7)) ) )
								continue;		// not visible
						}

						r_currentbkey = ((mleaf_t *)pn)->key;
						R_RenderBmodelFace (psideedges[i], psurf);
					}
				}
				else
				{
					R_RecursiveClipBPoly (psideedges[i], pnode->children[i],
									  psurf);
				}
			}
		}
	}
}


/*
================
R_DrawSolidClippedSubmodelPolygons

Bmodel crosses multiple leafs
================
*/
void R_DrawSolidClippedSubmodelPolygons (model_t *pmodel, mnode_t *topnode)
{
	int			i, j, lindex;
	float		dot;
	msurface_t	*psurf;
	int			numsurfaces;
	mplane_t	*pplane;
	mvertex_t	bverts[MAX_BMODEL_VERTS];
	bedge_t		bedges[MAX_BMODEL_EDGES], *pbedge;
	medge_t		*pedge, *pedges;

// FIXME: use bounding-box-based frustum clipping info?

	psurf = &pmodel->surfaces[pmodel->firstmodelsurface];
	numsurfaces = pmodel->nummodelsurfaces;
	pedges = pmodel->edges;

	for (i=0 ; i<numsurfaces ; i++, psurf++)
	{
		if (psurf->texinfo->flags & SURF_NODRAW)
			continue;

	// find which side of the node we are on
		pplane = psurf->plane;

		dot = DotProduct (modelorg, pplane->normal) - pplane->dist;

		// draw the polygon
		if (( !(psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
			((psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)))
			continue;

	// FIXME: use bounding-box-based frustum clipping info?

	// copy the edges to bedges, flipping if necessary so always
	// clockwise winding
	// FIXME: if edges and vertices get caches, these assignments must move
	// outside the loop, and overflow checking must be done here
		pbverts = bverts;
		pbedges = bedges;
		numbverts = numbedges = 0;
		pbedge = &bedges[numbedges];
		numbedges += psurf->numedges;

		for (j=0 ; j<psurf->numedges ; j++)
		{
		   lindex = pmodel->surfedges[psurf->firstedge+j];

			if (lindex > 0)
			{
				pedge = &pedges[lindex];
				pbedge[j].v[0] = &r_pcurrentvertbase[pedge->v[0]];
				pbedge[j].v[1] = &r_pcurrentvertbase[pedge->v[1]];
			}
			else
			{
				lindex = -lindex;
				pedge = &pedges[lindex];
				pbedge[j].v[0] = &r_pcurrentvertbase[pedge->v[1]];
				pbedge[j].v[1] = &r_pcurrentvertbase[pedge->v[0]];
			}

			pbedge[j].pnext = &pbedge[j+1];
		}

		pbedge[j-1].pnext = NULL;	// mark end of edges

		if ( !( psurf->texinfo->flags & ( SURF_TRANS66 | SURF_TRANS33 | SURF_MIDTEXTURE) ) )
			R_RecursiveClipBPoly (pbedge, topnode, psurf);
		else
			R_RenderBmodelFace( pbedge, psurf );
	}
}


/*
================
R_DrawSubmodelPolygons

All in one leaf
================
*/
void R_DrawSubmodelPolygons (model_t *pmodel, int clipflags, mnode_t *topnode)
{
	int			i;
	float		dot;
	msurface_t	*psurf;
	int			numsurfaces;
	mplane_t	*pplane;

// FIXME: use bounding-box-based frustum clipping info?

	psurf = &pmodel->surfaces[pmodel->firstmodelsurface];
	numsurfaces = pmodel->nummodelsurfaces;

	for (i=0 ; i<numsurfaces ; i++, psurf++)
	{
		if (psurf->texinfo->flags & SURF_NODRAW)
			continue;

	// find which side of the node we are on
		pplane = psurf->plane;

		dot = DotProduct (modelorg, pplane->normal) - pplane->dist;

	// draw the polygon
		if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
			(!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)))
		{
			r_currentkey = ((mleaf_t *)topnode)->key;

		// FIXME: use bounding-box-based frustum clipping info?
			R_RenderFace (psurf, clipflags);
		}
	}
}


int c_drawnode;

/*
================
R_RecursiveWorldNode
================
*/
void R_RecursiveWorldNode (mnode_t *node, int clipflags)
{
	int			i, c, side, *pindex;
	CVector		acceptpt, rejectpt;
	mplane_t	*plane;
	msurface_t	*surf, **mark;
	float		d, dot;
	mleaf_t		*pleaf;

	if (node->contents == CONTENTS_SOLID)
		return;		// solid

	if (node->visframe != r_visframecount)
		return;

// cull the clipping planes if not trivial accept
// FIXME: the compiler is doing a lousy job of optimizing here; it could be
//  twice as fast in ASM
	if (clipflags)
	{
		for (i=0 ; i<4 ; i++)
		{
			if (! (clipflags & (1<<i)) )
				continue;	// don't need to clip against it

		// generate accept and reject points
		// FIXME: do with fast look-ups or integer tests based on the sign bit
		// of the floating point values

			pindex = pfrustum_indexes[i];

			rejectpt.x = (float)node->minmaxs[pindex[0]];
			rejectpt.y = (float)node->minmaxs[pindex[1]];
			rejectpt.z = (float)node->minmaxs[pindex[2]];
			
			d = DotProduct (rejectpt, view_clipplanes[i].normal);
			d -= view_clipplanes[i].dist;
			if (d <= 0)
				return;
			acceptpt.x = (float)node->minmaxs[pindex[3+0]];
			acceptpt.y = (float)node->minmaxs[pindex[3+1]];
			acceptpt.z = (float)node->minmaxs[pindex[3+2]];

			d = DotProduct (acceptpt, view_clipplanes[i].normal);
			d -= view_clipplanes[i].dist;

			if (d >= 0)
				clipflags &= ~(1<<i);	// node is entirely on screen
		}
	}

c_drawnode++;

// if a leaf node, draw stuff
	if (node->contents != -1)
	{
		pleaf = (mleaf_t *)node;

		// check for door connected areas
		if (r_newrefdef.areabits)
		{
			if (! (r_newrefdef.areabits[pleaf->area>>3] & (1<<(pleaf->area&7)) ) )
				return;		// not visible
		}

		mark = pleaf->firstmarksurface;
		c = pleaf->nummarksurfaces;

		if (c)
		{
			do
			{
				(*mark)->visframe = r_framecount;
				mark++;
			} while (--c);
		}

		pleaf->key = r_currentkey;
		r_currentkey++;		// all bmodels in a leaf share the same key
	}
	else
	{
	// node is just a decision point, so go down the apropriate sides

	// find which side of the node we are on
		plane = node->plane;

		switch (plane->type)
		{
		case PLANE_X:
			dot = modelorg.x - plane->dist;
			break;
		case PLANE_Y:
			dot = modelorg.y - plane->dist;
			break;
		case PLANE_Z:
			dot = modelorg.z - plane->dist;
			break;
		default:
			dot = DotProduct (modelorg, plane->normal) - plane->dist;
			break;
		}
	
		if (dot >= 0)
			side = 0;
		else
			side = 1;

	// recurse down the children, front side first
		R_RecursiveWorldNode (node->children[side], clipflags);

	// draw stuff
		c = node->numsurfaces;

		if (c)
		{
			surf = r_worldmodel->surfaces + node->firstsurface;

			if (dot < -BACKFACE_EPSILON)
			{
				do
				{
					if (!(surf->texinfo->flags & SURF_NODRAW))
					{
						if ((surf->flags & SURF_PLANEBACK) &&
							(surf->visframe == r_framecount))
						{
							R_RenderFace (surf, clipflags);
						}
					}

					surf++;
				} while (--c);
			}
			else if (dot > BACKFACE_EPSILON)
			{
				do
				{
					
					if (!(surf->texinfo->flags & SURF_NODRAW))
					{
						if (!(surf->flags & SURF_PLANEBACK) &&
							(surf->visframe == r_framecount))
						{
							R_RenderFace (surf, clipflags);
						}
					}

					surf++;
				} while (--c);
			}

		// all surfaces on the same node share the same sequence number
			r_currentkey++;
		}

	// recurse down the back side
		R_RecursiveWorldNode (node->children[!side], clipflags);
	}
}



/*
================
R_RenderWorld
================
*/
void R_RenderWorld (void)
{

	if (!r_drawworld->value)
		return;
	if ( r_newrefdef.rdflags & RDF_NOWORLDMODEL )
		return;

	c_drawnode=0;

	// auto cycle the world frame for texture animation
	r_worldentity.frame = (int)(r_newrefdef.time*2);
	currententity = &r_worldentity;

	modelorg = r_origin;
	currentmodel = r_worldmodel;
	r_pcurrentvertbase = currentmodel->vertexes;

	R_RecursiveWorldNode (currentmodel->nodes, 15);
}


