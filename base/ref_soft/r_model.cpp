// models.c -- model loading and caching

// models are the only shared resource between a client and server running
// on the same machine.

#include "windows.h"
#include "r_local.h"

model_t	*loadmodel;
char	loadname[32];	// for hunk tags

void	Mod_LoadSpriteModel (model_t *mod, void *buffer, int resource);
void	Mod_LoadBrushModel (model_t *mod, void *buffer);
void	Mod_LoadAliasModel (model_t *mod, void *buffer, int resource);
model_t *Mod_LoadModel (model_t *mod, qboolean crash);
void	R_FreeImage (image_t *image);
void	*RegisterSkin (const char *name, resource_t flags);

byte	mod_novis[MAX_MAP_LEAFS/8];

#define	MAX_MOD_KNOWN	256
model_t	mod_known[MAX_MOD_KNOWN];
int		mod_numknown;

// the inline * models from the current map are kept seperate
model_t	mod_inline[MAX_MOD_KNOWN];

int		registration_sequence;
int		modfilelen;
byte*	bsp_ptr;		// pointer to location of next bmodel load

//===============================================================================


/*
================
R_FrameNameForModel
================
*/

void R_FrameNameForModel (char *modelname, int frameindex, char *framename)
{
	Sys_Error ("R_FrameNameForModel no longer supported!\n");
}
/*
	model_t			*mod;
	dmdl_t			*paliashdr;
	daliasframe_t	*paliasframe;

	mod = Mod_ForName (modelname, false, RESOURCE_RELOAD);

	if (!mod)
	{
		strcpy (framename, "");
		return;
	}

	paliashdr = (dmdl_t *) mod->extradata;

	if (frameindex > paliashdr->num_frames)
	{
		strcpy (framename, "");
		return;
	}

	paliasframe = (daliasframe_t *) ((byte *)paliashdr 
		+ paliashdr->ofs_frames + frameindex * paliashdr->framesize);

	strcpy (framename, paliasframe->name);
}
*/

/*
================
Mod_Modellist_f
================
*/
void Mod_Modellist_f (void)
{
	int		i;
	model_t	*mod;
	int		total;

	total = 0;
	ri.Con_Printf (PRINT_ALL,"Loaded models:\n");
	for (i=0, mod=mod_known ; i < mod_numknown ; i++, mod++)
	{
		if (!mod->name[0])
			continue;
		ri.Con_Printf (PRINT_ALL, "%8i : %s\n",mod->extradatasize, mod->name);
		total += mod->extradatasize;
	}
	ri.Con_Printf (PRINT_ALL, "Total resident: %i\n", total);
}

/*
===============
Mod_Init
===============
*/
void Mod_Init (void)
{
	memset (mod_novis, 0xff, sizeof(mod_novis));
}

/*
==================
Mod_ForName

Loads in a model for the given name
==================
*/
model_t *Mod_ForName (const char *name, qboolean crash, int resource)
{
	model_t	*mod;
	unsigned *buf;
	int		i, forced_index = -1;
	int		is_world_model = NOT_WORLD_MODEL;
	
	if (!name[0])
		ri.Sys_Error (ERR_DROP,"Mod_ForName: NULL name");

	//	Nelno:	determine if this is the world model
	if (strstr (name, ".bsp"))
		is_world_model = IS_WORLD_MODEL;

	//
	// inline models are grabbed only from worldmodel
	//
	if (name[0] == '*')
	{
		i = atoi(name+1);
		if (i < 1 || !r_worldmodel || i >= r_worldmodel->numsubmodels)
			ri.Sys_Error (ERR_DROP, "bad inline model number");
		return &mod_inline[i];
	}

	//
	// search the currently loaded models
	//
	//	Nelno:	world model can ONLY load at mod_known, nothing else can 
	//	load there or it causes major problems.  This was not a problem 
	//	with Quake 2 because the world model was always guaranteed to be
	//	the first model loaded, but with Daikatana that is not the case
	//	because R_FrameNameForModel can load a model before the .bsp is
	//	loaded.
	if (is_world_model)
	{
		mod = mod_known;

		if (!stricmp (mod->name, name))
			return	mod;

		forced_index = 0;
	}
	else
	{
		for (i=0 , mod=mod_known ; i<mod_numknown ; i++, mod++)
		{
			if (!mod->name [0])
				continue;
			
			//	Nelno:	reload this model or return it as found
			if (!stricmp (mod->name, name))
				return	mod;
		}

		// find a free model slot spot
		//
		//	Nelno:	start looking after mod_known, because that has to
		//	be the world model!!
		for (i = 0, mod = mod_known + 1; i < mod_numknown; i++, mod++)
		{
			if (!mod->name[0])
				break;	// free spot
		}

		if (i >= mod_numknown)
		{
			if (mod_numknown == MAX_MOD_KNOWN)
				ri.Sys_Error (ERR_DROP, "mod_numknown == MAX_MOD_KNOWN");
			mod_numknown++;
		}
	}

	strcpy (mod->name, name);
	mod->resource = (resource_t)resource;
	
	//
	// load the file
	//
	modfilelen = ri.FS_LoadFile (mod->name, (void **)&buf);
	if (!buf)
	{
		if (crash)
			ri.Sys_Error (ERR_DROP,"Mod_NumForName: %s not found", mod->name);
		memset (mod->name, 0, sizeof(mod->name));
		return NULL;
	}
	
	loadmodel = mod;

	//
	// fill it in
	//

	// call the apropriate loader
	
	switch (LittleLong(*(unsigned *)buf))
	{
	case IDALIASHEADER:
//		loadmodel->extradata = Hunk_Begin (0x200000);
		Mod_LoadAliasModel (mod, buf, resource);
		break;
		
	case IDSPRITEHEADER:
//		loadmodel->extradata = Hunk_Begin (0x10000);
		Mod_LoadSpriteModel (mod, buf, resource);
		break;
	
	case IDBSPHEADER:
//		loadmodel->extradata = Hunk_Begin (0x1000000);
		Mod_LoadBrushModel (mod, buf);
		break;

	default:
		ri.Con_Printf (PRINT_ALL, "Mod_NumForName: unknown field for %s\n", mod->name);
		ri.FS_FreeFile (buf);
		mod_numknown--;
		return NULL;
		break;
	}

//	loadmodel->extradatasize = Hunk_End ();

	if (mod == mod_known)
		ri.Con_Printf (PRINT_ALL, "world size = %i\n", loadmodel->extradatasize);

	ri.FS_FreeFile (buf);

	return mod;
}


/*
===============
Mod_PointInLeaf
===============
*/
mleaf_t *Mod_PointInLeaf (CVector &p, model_t *model)
{
	mnode_t		*node;
	float		d;
	mplane_t	*plane;
	
	if (!model || !model->nodes)
		ri.Sys_Error (ERR_DROP, "Mod_PointInLeaf: bad model");

	node = model->nodes;
	while (1)
	{
		if (node->contents != -1)
			return (mleaf_t *)node;
		plane = node->plane;
		d = DotProduct (p,plane->normal) - plane->dist;
		if (d > 0)
			node = node->children[0];
		else
			node = node->children[1];
	}
	
	return NULL;	// never reached
}


/*
===================
Mod_DecompressVis
===================
*/
byte *Mod_DecompressVis (byte *in, model_t *model)
{
	static byte	decompressed[MAX_MAP_LEAFS/8];
	int		c;
	byte	*out;
	int		row;

	row = (model->vis->numclusters+7)>>3;	
	out = decompressed;

#if 0
	memcpy (out, in, row);
#else
	if (!in)
	{	// no vis info, so make all visible
		while (row)
		{
			*out++ = 0xff;
			row--;
		}
		return decompressed;		
	}

	do
	{
		if (*in)
		{
			*out++ = *in++;
			continue;
		}
	
		c = in[1];
		in += 2;
		while (c)
		{
			*out++ = 0;
			c--;
		}
	} while (out - decompressed < row);
#endif
	
	return decompressed;
}

/*
==============
Mod_ClusterPVS
==============
*/
byte *Mod_ClusterPVS (int cluster, model_t *model)
{
	if (cluster == -1 || !model->vis)
		return mod_novis;
	return Mod_DecompressVis ( (byte *)model->vis + model->vis->bitofs[cluster][DVIS_PVS],
		model);
}

/*
===============================================================================

					BRUSHMODEL LOADING

===============================================================================
*/

byte	*mod_base;


/*
=================
Mod_LoadLighting

Converts the 24 bit lighting down to 8 bit
by taking the brightest component
=================
*/
void Mod_LoadLighting (lump_t *l)
{
//	int		i, size;
//	byte	*in;

	if (!l->filelen)
	{
		loadmodel->lightdata = NULL;
		return;
	}
	// mdm99.03.17 - get the ptr and inc
	loadmodel->lightdata = (byte *)bsp_ptr;
	bsp_ptr += l->filelen;
	
	//loadmodel->lightdata = (byte *)Hunk_Alloc ( l->filelen);	
	memcpy (loadmodel->lightdata, mod_base + l->fileofs, l->filelen);
/*
	size = l->filelen/3;
	loadmodel->lightdata = (Hunk_Alloc (size);
	in = (void *)(mod_base + l->fileofs);
	for (i=0 ; i<size ; i++, in+=3)
	{
		if (in[0] > in[1] && in[0] > in[2])
			loadmodel->lightdata[i] = in[0];
		else if (in[1] > in[0] && in[1] > in[2])
			loadmodel->lightdata[i] = in[1];
		else
			loadmodel->lightdata[i] = in[2];
	}
*/
}


int		r_leaftovis[MAX_MAP_LEAFS];
int		r_vistoleaf[MAX_MAP_LEAFS];
int		r_numvisleafs;

void	R_NumberLeafs (mnode_t *node)
{
	mleaf_t	*leaf;
	int		leafnum;

	if (node->contents != -1)
	{
		leaf = (mleaf_t *)node;
		leafnum = leaf - loadmodel->leafs;
		if (leaf->contents & CONTENTS_SOLID)
			return;
		r_leaftovis[leafnum] = r_numvisleafs;
		r_vistoleaf[r_numvisleafs] = leafnum;
		r_numvisleafs++;
		return;
	}

	R_NumberLeafs (node->children[0]);
	R_NumberLeafs (node->children[1]);
}


/*
=================
Mod_LoadVisibility
=================
*/
void Mod_LoadVisibility (lump_t *l)
{
	int		i;

	if (!l->filelen)
	{
		loadmodel->vis = NULL;
		return;
	}
	// mdm99.03.17 - get the ptr and inc
	loadmodel->vis = (dvis_t*)bsp_ptr;
	bsp_ptr += l->filelen;
	//loadmodel->vis = (dvis_t *)Hunk_Alloc ( l->filelen);	
	memcpy (loadmodel->vis, mod_base + l->fileofs, l->filelen);

	loadmodel->vis->numclusters = LittleLong (loadmodel->vis->numclusters);
	for (i=0 ; i<loadmodel->vis->numclusters ; i++)
	{
		loadmodel->vis->bitofs[i][0] = LittleLong (loadmodel->vis->bitofs[i][0]);
		loadmodel->vis->bitofs[i][1] = LittleLong (loadmodel->vis->bitofs[i][1]);
	}
}


/*
=================
Mod_LoadVertexes
=================
*/
void Mod_LoadVertexes (lump_t *l)
{
	dvertex_t	*in;
	mvertex_t	*out;
	int			i, count;

	in = (dvertex_t*)(void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP,"MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	// mdm99.03.17 - get the ptr and inc
	out = (mvertex_t*)bsp_ptr;
	bsp_ptr += (count*sizeof(mvertex_t));
	//out = (mvertex_t *)Hunk_Alloc ( count*sizeof(mvertex_t));	

	loadmodel->vertexes = out;
	loadmodel->numvertexes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		out->position.x = LittleFloat (in->point[0]);
		out->position.y = LittleFloat (in->point[1]);
		out->position.z = LittleFloat (in->point[2]);
	}
}

/*
=================
Mod_LoadSubmodels
=================
*/
void Mod_LoadSubmodels (lump_t *l)
{
	dmodel_t	*in;
	dmodel_t	*out;
	int			i, j, count;

	in = (dmodel_t*)(void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP,"MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (dmodel_t*)bsp_ptr;
	bsp_ptr += (count*sizeof(*out));
//	out = (dmodel_t*)Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->submodels = out;
	loadmodel->numsubmodels = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{	// spread the mins / maxs by a pixel
			out->mins[j] = LittleFloat (in->mins[j]) - 1;
			out->maxs[j] = LittleFloat (in->maxs[j]) + 1;
			out->origin[j] = LittleFloat (in->origin[j]);
		}
		out->headnode = LittleLong (in->headnode);
		out->firstface = LittleLong (in->firstface);
		out->numfaces = LittleLong (in->numfaces);
	}
}

/*
=================
Mod_LoadEdges
=================
*/
void Mod_LoadEdges (lump_t *l)
{
	dedge_t *in;
	medge_t *out;
	int 	i, count;

	in = (dedge_t*)(void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP,"MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (medge_t*)bsp_ptr;	
	bsp_ptr += ((count+1)*sizeof(*out));
//	out = (medge_t*)Hunk_Alloc ( (count + 13) * sizeof(*out));	// extra for skybox

	loadmodel->edges = out;
	loadmodel->numedges = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		out->v[0] = (unsigned short)LittleShort(in->v[0]);
		out->v[1] = (unsigned short)LittleShort(in->v[1]);
	}
}

/*
=================
Mod_LoadTexinfo
=================
*/
void Mod_LoadTexinfo (lump_t *l)
{
	texinfo_t *in;
	mtexinfo_t *out, *step;
	int 	i, count;
	int		len1, len2;
	char	name[MAX_QPATH];
	int		next;

	in = (texinfo_t*)(void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP,"MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (mtexinfo_t*)bsp_ptr;
	bsp_ptr += (count*sizeof(*out));
//	out = (mtexinfo_t*)Hunk_Alloc ( (count+6)*sizeof(*out));	// extra for skybox

	loadmodel->texinfo = out;
	loadmodel->numtexinfo = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
//		for (j=0 ; j<8 ; j++)
//			out->vecs[0][j] = LittleFloat (in->vecs[0][j]);

		out->s.x = LittleFloat( in->vecs[0][0] );
		out->s.y = LittleFloat( in->vecs[0][1] );
		out->s.z = LittleFloat( in->vecs[0][2] );
		out->s_offset = LittleFloat( in->vecs[0][3] );

		out->t.x = LittleFloat( in->vecs[1][0] );
		out->t.y = LittleFloat( in->vecs[1][1] );
		out->t.z = LittleFloat( in->vecs[1][2] );
		out->t_offset = LittleFloat( in->vecs[1][3] );

		len1 = out->s.Length();
		len2 = out->t.Length();
		len1 = (len1 + len2)/2;
		if (len1 < 0.32)
			out->mipadjust = 4;
		else if (len1 < 0.49)
			out->mipadjust = 3;
		else if (len1 < 0.99)
			out->mipadjust = 2;
		else
			out->mipadjust = 1;
#if 0
		if (len1 + len2 < 0.001)
			out->mipadjust = 1;		// don't crash
		else
			out->mipadjust = 1 / floor( (len1+len2)/2 + 0.1 );
#endif

		out->flags = LittleLong (in->flags);

		next = LittleLong (in->nexttexinfo);
		if (next > 0)
			out->next = loadmodel->texinfo + next;

		Com_sprintf (name, sizeof(name), "textures/%s.wal", in->texture);
		out->image = (image_t*)R_FindImage (name, it_wall, RESOURCE_LEVEL);
		if (!out->image)
		{
			out->image = r_notexture_mip; // texture not found
			out->flags = 0;
		}
	}

	// count animation frames
	for (i=0 ; i<count ; i++)
	{
		out = &loadmodel->texinfo[i];
		out->numframes = 1;
		for (step = out->next ; step && step != out ; step=step->next)
			out->numframes++;
	}
}

/*
================
CalcSurfaceExtents

Fills in s->texturemins[] and s->extents[]
================
*/
void CalcSurfaceExtents (msurface_t *s)
{
	float	mins[2], maxs[2], val;
	int		i, e;
	mvertex_t	*v;
	mtexinfo_t	*tex;
	int		bmins[2], bmaxs[2];

	mins[0] = mins[1] = 999999;
	maxs[0] = maxs[1] = -99999;

	tex = s->texinfo;
	
	for (i=0 ; i<s->numedges ; i++)
	{
		e = loadmodel->surfedges[s->firstedge+i];
		if (e >= 0)
			v = &loadmodel->vertexes[loadmodel->edges[e].v[0]];
		else
			v = &loadmodel->vertexes[loadmodel->edges[-e].v[1]];
		
/*
		for (j=0 ; j<2 ; j++)
		{
			val = v->position[0] * tex->vecs[j][0] + 
				v->position[1] * tex->vecs[j][1] +
				v->position[2] * tex->vecs[j][2] +
				tex->vecs[j][3];
			if (val < mins[j])
				mins[j] = val;
			if (val > maxs[j])
				maxs[j] = val;
		}
*/
		val = v->position.x * tex->s.x + 
			v->position.y * tex->s.y + 
			v->position.z * tex->s.z + 
			tex->s_offset;
		if (val < mins[0] )
		{
			mins[0] = val;
		}
		if ( val > maxs[0] )
		{
			maxs[0] = val;
		}

		val = v->position.x * tex->t.x + 
			v->position.y * tex->t.y + 
			v->position.z * tex->t.z + 
			tex->t_offset;
		if (val < mins[1])
		{
			mins[1] = val;
		}
		if ( val > maxs[1] )
		{
			maxs[1] = val;
		}
	}

	for (i=0 ; i<2 ; i++)
	{	
		bmins[i] = floor(mins[i]/16);
		bmaxs[i] = ceil(maxs[i]/16);

		s->texturemins[i] = bmins[i] * 16;
		s->extents[i] = (bmaxs[i] - bmins[i]) * 16;
		if (s->extents[i] < 16)
			s->extents[i] = 16;	// take at least one cache block
		if ( !(tex->flags & (SURF_WARP|SURF_SKY)) && s->extents[i] > 256)
			ri.Sys_Error (ERR_DROP,"Bad surface extents");
	}
}


/*
=================
Mod_LoadFaces
=================
*/
void Mod_LoadFaces (lump_t *l)
{
	dface_t		*in;
	msurface_t 	*out;
	int			i, count, surfnum;
	int			planenum, side;

	in = (dface_t*)(void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP,"MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (msurface_t*)bsp_ptr;
	bsp_ptr += (count*sizeof(*out));
	//out = (msurface_t *)Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->surfaces = out;
	loadmodel->numsurfaces = count;

	for ( surfnum=0 ; surfnum<count ; surfnum++, in++, out++)
	{
		out->firstedge = LittleLong(in->firstedge);
		out->numedges = LittleShort(in->numedges);		
		if (out->numedges < 3)
			ri.Sys_Error (ERR_DROP,"Surface with %s edges", out->numedges);
		out->flags = 0;

		planenum = LittleShort(in->planenum);
		side = LittleShort(in->side);
		if (side)
			out->flags |= SURF_PLANEBACK;

		out->plane = loadmodel->planes + planenum;

		out->texinfo = loadmodel->texinfo + LittleShort (in->texinfo);

		CalcSurfaceExtents (out);
				
	// lighting info is converted from 24 bit on disk to 8 bit

		for (i=0 ; i<MAXLIGHTMAPS ; i++)
			out->styles[i] = in->styles[i];
		i = LittleLong(in->lightofs);
		if (i == -1)
			out->samples = NULL;
		else
			out->samples = loadmodel->lightdata + i;
//			out->samples = loadmodel->lightdata + i/3;

	// set the drawing flags flag
		
		if (!out->texinfo->image)
			continue;
		if (out->texinfo->flags & SURF_SKY)
		{
			out->flags |= SURF_DRAWSKY;
			continue;
		}
		
		if (out->texinfo->flags & SURF_WARP)
		{
			out->flags |= SURF_DRAWTURB;
			for (i=0 ; i<2 ; i++)
			{
				out->extents[i] = 16384;
				out->texturemins[i] = -8192;
			}
			continue;
		}
	}
}


/*
=================
Mod_SetParent
=================
*/
void Mod_SetParent (mnode_t *node, mnode_t *parent)
{
	node->parent = parent;
	if (node->contents != -1)
		return;
	Mod_SetParent (node->children[0], node);
	Mod_SetParent (node->children[1], node);
}

/*
=================
Mod_LoadNodes
=================
*/
void Mod_LoadNodes (lump_t *l)
{
	int			i, j, count, p;
	dnode_t		*in;
	mnode_t 	*out;

	in = (dnode_t*)(void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP,"MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (mnode_t*)bsp_ptr;
	bsp_ptr += (count*sizeof(*out));
//	out = (mnode_t *)Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->nodes = out;
	loadmodel->numnodes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->minmaxs[j] = LittleShort (in->mins[j]);
			out->minmaxs[3+j] = LittleShort (in->maxs[j]);
		}
	
		p = LittleLong(in->planenum);
		out->plane = loadmodel->planes + p;

		out->firstsurface = LittleShort (in->firstface);
		out->numsurfaces = LittleShort (in->numfaces);
		out->contents = CONTENTS_NODE;	// differentiate from leafs
		
		for (j=0 ; j<2 ; j++)
		{
			p = LittleLong (in->children[j]);
			if (p >= 0)
				out->children[j] = loadmodel->nodes + p;
			else
				out->children[j] = (mnode_t *)(loadmodel->leafs + (-1 - p));
		}
	}
	
	Mod_SetParent (loadmodel->nodes, NULL);	// sets nodes and leafs
}

/*
=================
Mod_LoadLeafs
=================
*/
void Mod_LoadLeafs (lump_t *l)
{
	dleaf_t 	*in;
	mleaf_t 	*out;
	int			i, j, count;

	in = (dleaf_t*)(void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP,"MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (mleaf_t*)bsp_ptr;
	bsp_ptr += (count*sizeof(*out));
//	out = (mleaf_t *)Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->leafs = out;
	loadmodel->numleafs = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->minmaxs[j] = LittleShort (in->mins[j]);
			out->minmaxs[3+j] = LittleShort (in->maxs[j]);
		}

		out->contents = LittleLong(in->contents);
		out->cluster = LittleShort(in->cluster);
		out->area = LittleShort(in->area);

		out->firstmarksurface = loadmodel->marksurfaces +
			LittleShort(in->firstleafface);
		out->nummarksurfaces = LittleShort(in->numleaffaces);
	}	
}


/*
=================
Mod_LoadMarksurfaces
=================
*/
void Mod_LoadMarksurfaces (lump_t *l)
{	
	int		i, j, count;
	short		*in;
	msurface_t **out;
	
	in = (short*)(void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP,"MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (msurface_t**)bsp_ptr;
	bsp_ptr += (count*sizeof(*out));
//	out = (msurface_t **)Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->marksurfaces = out;
	loadmodel->nummarksurfaces = count;

	for ( i=0 ; i<count ; i++)
	{
		j = LittleShort(in[i]);
		if (j >= loadmodel->numsurfaces)
			ri.Sys_Error (ERR_DROP,"Mod_ParseMarksurfaces: bad surface number");
		out[i] = loadmodel->surfaces + j;
	}
}

/*
=================
Mod_LoadSurfedges
=================
*/
void Mod_LoadSurfedges (lump_t *l)
{	
	int		i, count;
	int		*in, *out;
	
	in = (int*)(void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP,"MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (int*)bsp_ptr;
	bsp_ptr += (count*sizeof(*out));
//	out = (int *)Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->surfedges = out;
	loadmodel->numsurfedges = count;

	for ( i=0 ; i<count ; i++)
		out[i] = LittleLong (in[i]);
}

/*
=================
Mod_LoadPlanes
=================
*/
void Mod_LoadPlanes (lump_t *l)
{
	int			i, j;
	mplane_t	*out;
	dplane_t 	*in;
	int			count;
	int			bits;
	
#ifdef	CONSOLIDATE_BSP
	loadmodel->planes = (mplane_t *) ri.bspModel->planes;
	loadmodel->numplanes = ri.bspModel->numPlanes;

	return;
#endif

	in = (dplane_t*)(void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP,"MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (mplane_t*)bsp_ptr;
	bsp_ptr += (count*2*sizeof(*out));
//	out = (mplane_t *)Hunk_Alloc ( count*2*sizeof(*out));	
	
	loadmodel->planes = out;
	loadmodel->numplanes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		bits = 0;
		for (j=0 ; j<3 ; j++)
		{
			out->normal[j] = LittleFloat (in->normal[j]);
			if (out->normal[j] < 0)
				bits |= 1<<j;
		}

		out->dist = LittleFloat (in->dist);
		out->type = LittleLong (in->type);
		out->signbits = bits;
	}
}


///////////////////////////////////////////////////////////////////////////////
//	Mod_LoadPlanePolys
//
//	Loads the list of faces for each plane.  They are stored in the lump
//	as follows:
//
//	First int in the lump is # of faces for plane 0.
//	Following that are the indexes of each face for plane 0.
//	This sequence repeats for planes 0 trough (loadmodel->numplanes - 1).
//	Only the exact amount of memory required to store each list is allocated.
//	This runs about 80KB for a large (9000 plane) level.
///////////////////////////////////////////////////////////////////////////////

void	Mod_LoadPlanePolys (lump_t *lump)
{
	int			numPlanes;
	int			planeCount, faceCount;
	int			faceListSize;
	int			*in;
	int			maxFacesForAPlane, minFacesForAPlane;
	int			numUsedPlanes, totalFacesForPlanes;

	in = (int *)(mod_base + lump->fileofs);
	if (lump->filelen % sizeof (*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadPlanePolys: funny lump size in %s",loadmodel->name);
/*	
	numPlanes = lump->filelen / sizeof(*in);
	if (numPlanes != loadmodel->numplanes)
		ri.Sys_Error (ERR_DROP, "MOD_LoadPlanePolys: diffent number of planes than LUMP_PLANES.\n");
*/
	numPlanes = loadmodel->numplanes;

	maxFacesForAPlane = 0;
	minFacesForAPlane = MAX_POLYS_PER_PLANE + 1;
	numUsedPlanes = totalFacesForPlanes = 0;

	numPlanes = loadmodel->numplanes;

	loadmodel->planePolys = (planeList_t*)bsp_ptr;
	bsp_ptr += (sizeof(void*)*numPlanes);
//	loadmodel->planePolys = (planeList_t *)Hunk_Alloc (sizeof (void *) * numPlanes);
//	memset (loadmodel->planePolys, 0x00, sizeof (void *) * numPlanes);

	for (planeCount = 0; planeCount < numPlanes; planeCount++)
	{
		//	first number is number of faces for current plane
		faceCount = *(int *)in;

		//	check for no faces on this plane
		if (faceCount > 0)
		{
			faceListSize = sizeof (int) * (faceCount + 1);
			loadmodel->planePolys->planes[planeCount] = (faceList_t*)bsp_ptr;
			bsp_ptr += faceListSize;
			//loadmodel->planePolys->planes [planeCount] = (faceList_t *)Hunk_Alloc (faceListSize);

			//	copy both numFaces, and faceNum array
			memcpy (loadmodel->planePolys->planes [planeCount], in, faceListSize);

			in += faceCount + 1;

			totalFacesForPlanes += faceCount;
			numUsedPlanes++;
		}
		else
		{
			//	skip past numFaces in buffer
			in++;

			//	plane has 0 faces, so faceList element is NULL
			loadmodel->planePolys->planes [planeCount] = NULL;
		}

		if (faceCount > maxFacesForAPlane)
			maxFacesForAPlane = faceCount;
		if (faceCount < minFacesForAPlane)
			minFacesForAPlane = faceCount;
	}

	ri.Con_Printf (PRINT_ALL, "Min faces for a plane: %i\n", minFacesForAPlane);
	ri.Con_Printf (PRINT_ALL, "Max faces for a plane: %i\n", maxFacesForAPlane);
	ri.Con_Printf (PRINT_ALL, "Average faces per plane: %i\n", totalFacesForPlanes / numUsedPlanes);
	ri.Con_Printf (PRINT_ALL, "Percentage of planes used: %.2f%%\n", ((float)numUsedPlanes / (float)numPlanes) * 100.0);
}

/*
=================
Mod_LoadBrushModel
=================
*/
void Mod_LoadBrushModel (model_t *mod, void *buffer)
{
	int			i, version;
	dheader_t	*header;
	dmodel_t 	*bm;
	int			size;

	loadmodel->type = mod_brush;
	if (loadmodel != mod_known)
		ri.Sys_Error (ERR_DROP, "Loaded a brush model after the world");
	
	header = (dheader_t *)buffer;

	version = LittleLong (header->version);
	if (version != BSPVERSION && version != BSPVERSION_TEXINFOOLD &&
		version != BSPVERSION_Q2 && version != BSPVERSION_COLORLUMP)
		ri.Sys_Error (ERR_DROP,"Mod_LoadBrushModel: %s has wrong version number (%i should be %i)", mod->name, version, BSPVERSION);

// swap all the lumps
	mod_base = (byte *)header;

	for (i=0 ; i<sizeof(dheader_t)/4 ; i++)
		((int *)header)[i] = LittleLong ( ((int *)header)[i]);

	// calculate size of bsp for allocation
	size = header->lumps[LUMP_VERTEXES].filelen +									// Mod_LoadVertexes
		(header->lumps[LUMP_EDGES].filelen*2)+sizeof(medge_t) +						// Mod_LoadEdges
		header->lumps[LUMP_SURFEDGES].filelen +										// Mod_LoadSurfEdges
		header->lumps[LUMP_LIGHTING].filelen +										// Mod_LoadLighting
		// we don't need this memory if bsp is consolidated
#ifndef CONSOLIDATE_BSP		
		(header->lumps[LUMP_PLANES].filelen*2) +									// Mod_LoadPlanes
#endif
		header->lumps[LUMP_TEXINFO].filelen +										// Mod_LoadTexInfo
		(header->lumps[LUMP_FACES].filelen/sizeof(dface_t))*sizeof(msurface_t) +	// Mod_LoadFaces
		(header->lumps[LUMP_LEAFFACES].filelen*2) +									// Mod_LoadMarksurfaces
#ifndef CONSOLIDATE_BSP		
		header->lumps[LUMP_VISIBILITY].filelen +									// Mod_LoadVisibility
#endif		
		(header->lumps[LUMP_LEAFS].filelen/sizeof(dleaf_t))*sizeof(mleaf_t) +		// Mod_LoadLeafs
		(header->lumps[LUMP_NODES].filelen/sizeof(dnode_t))*sizeof(mnode_t) +		// Mod_LoadNodes
		(header->lumps[LUMP_MODELS].filelen/sizeof(dmodel_t))*sizeof(model_t) +		// Mod_LoadSubModels
		header->lumps[LUMP_PLANEPOLYS].filelen; 

	// allocate the bsp model memory
	bsp_ptr = (byte*)ri.X_Malloc(size,MEM_TAG_BMODEL);

	// setup extra data
	mod->extradata = bsp_ptr;
	mod->extradatasize = size;

// load into heap
	
	Mod_LoadVertexes (&header->lumps[LUMP_VERTEXES]);
	Mod_LoadEdges (&header->lumps[LUMP_EDGES]);
	Mod_LoadSurfedges (&header->lumps[LUMP_SURFEDGES]);
	Mod_LoadLighting (&header->lumps[LUMP_LIGHTING]);
	Mod_LoadPlanes (&header->lumps[LUMP_PLANES]);
	Mod_LoadTexinfo (&header->lumps[LUMP_TEXINFO]);
	Mod_LoadFaces (&header->lumps[LUMP_FACES]);
	Mod_LoadMarksurfaces (&header->lumps[LUMP_LEAFFACES]);
	Mod_LoadVisibility (&header->lumps[LUMP_VISIBILITY]);
	Mod_LoadLeafs (&header->lumps[LUMP_LEAFS]);
	Mod_LoadNodes (&header->lumps[LUMP_NODES]);
	Mod_LoadSubmodels (&header->lumps[LUMP_MODELS]);
	r_numvisleafs = 0;
	R_NumberLeafs (loadmodel->nodes);

	if (version < BSPVERSION)
	{
		loadmodel->planePolys = NULL;
	}
	else
	{
		Mod_LoadPlanePolys (&header->lumps [LUMP_PLANEPOLYS]);
	}
	
//
// set up the submodels
//
	for (i=0 ; i<mod->numsubmodels ; i++)
	{
		model_t	*starmod;

		bm = &mod->submodels[i];
		starmod = &mod_inline[i];

		*starmod = *loadmodel;
		
		starmod->firstmodelsurface = bm->firstface;
		starmod->nummodelsurfaces = bm->numfaces;
		starmod->firstnode = bm->headnode;
		if (starmod->firstnode >= loadmodel->numnodes)
			ri.Sys_Error (ERR_DROP, "Inline model %i has bad firstnode", i);

		starmod->maxs = bm->maxs;
		starmod->mins = bm->mins;
	
		if (i == 0)
			*loadmodel = *starmod;
	}

	R_InitSkyBox ();
}

/*
==============================================================================

ALIAS MODELS

==============================================================================
*/

///////////////////////////////////////////////////////////////////////////////
//	Mod_RegisterEpisodeSkins
//
//	loads correct skins for a model based on episode
///////////////////////////////////////////////////////////////////////////////

void	Mod_RegisterEpisodeSkins (void *mod, int episode, int resource)
{
	int			j, i, already_registered = true;
	int			found_postfix = false;
	char		episode_postfix [MAX_OSPATH];
	char		test_postfix [MAX_OSPATH];
	char		dir [_MAX_DIR];
	char		fname [_MAX_FNAME];
	char		ext [_MAX_EXT];
	char		name [MAX_OSPATH];
	model_t		*model;
	
	model = (model_t*)mod;

	if (!model)
		return;

	if (episode > 9)
	{
		ri.Sys_Error (ERR_FATAL, "Tried to register skin for episode > 9\n");
		return;
	}

	sprintf (episode_postfix, "_e%i", episode);

	//	first, check to see if the right skins are already loaded so we
	//	don't end up reloading them
	for (i = 0; model->skins [i] != NULL; i++)
	{
		_splitpath (model->skins [i]->name, NULL, NULL, fname, ext);
		if (strstr (fname, episode_postfix) != (fname + strlen (fname) - 3))
		{
			already_registered = false;
			break;
		}	
	}

	if (already_registered)
	{
//		ri.Con_Printf (PRINT_ALL, "Skins already registered.\n");
		return;
	}

	//	not registered yet, so purge everything and reload
	for (i = 0; model->skins [i] != NULL; i++)
	{
		_splitpath (model->skins [i]->name, NULL, dir, fname, ext);

		for (j = 1; j < 10; j++)
		{
			sprintf (test_postfix, "_e%i", j);
			if (strstr (fname, test_postfix) == (fname + strlen (fname) - 3))
			{
				//	strip off the postfix if it is there
				fname [strlen (fname) - 3] = 0x00;
				sprintf (name, "%s%s%s%s", dir, fname, episode_postfix, ext);
				found_postfix = true;
				break;
			}
		}

		if (!found_postfix)
		{
			//	just put the postfix on the end
			sprintf (name, "%s%s%s%s", dir, fname, episode_postfix, ext);
		}

//		ri.Con_Printf (PRINT_ALL, "Purged %s.\n", model->skins [i]->name);

		R_FreeImage (model->skins [i]);
		model->skins [i] = (image_t*)R_FindImage (name, it_skin, RESOURCE_NPC_SKIN);

//		ri.Con_Printf (PRINT_ALL, "Loaded %s.\n", model->skins [i]->name);
	}

}

/*
=================
Mod_LoadAliasModel
=================
*/
void Mod_LoadAliasModel (model_t *mod, void *buffer, int resource)
{
	int					i, j, k;
	dmdl_t				*pinmodel, *pheader;
	dstvert_t			*pinst, *poutst;
	dtriangle_t			*pintri, *pouttri;
	daliasframe_t		*pinframe, *poutframe;
	daliasframe2_t		*pinframe2, *poutframe2;
	dsurface_t			*pinsurface, *poutsurface;
	int					*pincmd, *poutcmd;
	int					version;
	char				image_path [MAX_OSPATH];

	pinmodel = (dmdl_t *)buffer;

	version = LittleLong (pinmodel->version);

	if ( version != ALIAS_VERSION && version != ALIAS_VERSION2 )
	{
		ri.Con_Printf (PRINT_ALL, "%s has wrong version number (%i should be %i)",
				 mod->name, version, ALIAS_VERSION);
		return;
	}

	pheader = (dmdl_t *)ri.X_Malloc(LittleLong(pinmodel->ofs_end), MEM_TAG_MODEL);
//	pheader = (dmdl_t *)Hunk_Alloc (LittleLong(pinmodel->ofs_end));
	
	// byte swap the header fields and sanity check
	for (i=0 ; i<sizeof(dmdl_t)/4 ; i++)
		((int *)pheader)[i] = LittleLong (((int *)buffer)[i]);

	if (pheader->num_xyz <= 0)
		ri.Sys_Error (ERR_DROP, "model %s has no vertices", mod->name);

	if (pheader->num_xyz > MAX_VERTS)
		ri.Sys_Error (ERR_DROP, "model %s has too many vertices", mod->name);

	if (pheader->num_st <= 0)
		ri.Sys_Error (ERR_DROP, "model %s has no st vertices", mod->name);

	if (pheader->num_tris <= 0)
		ri.Sys_Error (ERR_DROP, "model %s has no triangles", mod->name);

	if (pheader->num_frames <= 0)
		ri.Sys_Error (ERR_DROP, "model %s has no frames", mod->name);

//
// load the surface headers
//
	pinsurface = (dsurface_t *) ((byte *) pinmodel + pheader->ofs_surfaces);
	poutsurface = (dsurface_t *) ((byte *) pheader + pheader->ofs_surfaces);

	for (i = 0; i < pheader->num_surfaces; i++)
	{
		memcpy (poutsurface[i].name, pinsurface[i].name, 32);
		poutsurface[i].flags		= pinsurface[i].flags;
		poutsurface[i].skinindex	= pinsurface[i].skinindex;
		poutsurface[i].skinwidth	= pinsurface[i].skinwidth;
		poutsurface[i].skinheight	= pinsurface[i].skinheight;
		poutsurface[i].num_uvframes	= pinsurface[i].num_uvframes;
//		poutsurface[i].num_st		= pinsurface[i].num_st;
//		poutsurface[i].num_xyz		= pinsurface[i].num_xyz;
//		poutsurface[i].num_tris		= pinsurface[i].num_tris;

/******* ANDREW: THIS ULTIMATELY NEEDS TO BE REMOVED **********/
		// mark heirchical surfaces as no draw
		if (strstr (pinsurface[i].name, "hr_"))
			poutsurface[i].flags |= SRF_NODRAW;

		// check for translucent surfaces
		if (strstr(poutsurface[i].name,"33"))
			poutsurface[i].flags |= SRF_TRANS33;
		else if (strstr(poutsurface[i].name,"66"))
			poutsurface[i].flags |= SRF_TRANS66;
	
		if (poutsurface->skinheight > MAX_LBM_HEIGHT)
			ri.Sys_Error (ERR_DROP, "model %s has a skin taller than %d", mod->name,
					   MAX_LBM_HEIGHT);
	}

//
// load base s and t vertices (not used in gl version)
//
	pinst = (dstvert_t *) ((byte *)pinmodel + pheader->ofs_st);
	poutst = (dstvert_t *) ((byte *)pheader + pheader->ofs_st);

	for (i=0 ; i<pheader->num_st ; i++)
	{
		poutst[i].s = LittleShort (pinst[i].s);
		poutst[i].t = LittleShort (pinst[i].t);
	}

//
// load triangle lists
//
/*
	pintri = (dtriangle_t *) ((byte *)pinmodel + pheader->ofs_tris);
	pouttri = (dtriangle_t *) ((byte *)pheader + pheader->ofs_tris);

	for (i=0 ; i<pheader->num_tris ; i++)
	{
		pouttri[i].index_surface = LittleShort (pintri[i].index_surface);
		pouttri[i].num_uvframes = LittleShort (pintri[i].num_uvframes);

		for (j=0 ; j<3 ; j++)
		{
			pouttri[i].index_xyz[j] = LittleShort (pintri[i].index_xyz[j]);
			pouttri[i].index_st[j] = LittleShort (pintri[i].index_st[j]);
		}
	}
*/
	pintri = (dtriangle_t *) ((byte *)pinmodel + pheader->ofs_tris);
	pouttri = (dtriangle_t *) ((byte *)pheader + pheader->ofs_tris);

	for (i=0 ; i<pheader->num_tris ; i++)
	{
		int	num_uvframes;

		pouttri->index_surface = LittleShort (pintri->index_surface);
		pouttri->num_uvframes = LittleShort (pintri->num_uvframes);

		num_uvframes = pouttri->num_uvframes;

		for (j=0 ; j<3 ; j++)
			pouttri->index_xyz[j] = LittleShort (pintri->index_xyz[j]);

		for (j=0 ; j<3 ; j++)
			for (k = 0; k < num_uvframes; k++)
				pouttri->stframes[k].index_st[j] = LittleShort (pintri->stframes[k].index_st[j]);

		pouttri = (dtriangle_t *) ((byte *) pouttri + sizeof (dtriangle_t) + ((num_uvframes - 1) * sizeof (dstframe_t)));
		pintri = (dtriangle_t *) ((byte *) pintri + sizeof (dtriangle_t) + ((num_uvframes - 1) * sizeof (dstframe_t)));
	}

//
// load the frames
//
	if( version == ALIAS_VERSION2 ){
		for ( i = 0; i < pheader->num_frames; i++ ){
			pinframe2 = (daliasframe2_t *) ((byte *)pinmodel 
				+ pheader->ofs_frames + i * pheader->framesize);
			poutframe2 = (daliasframe2_t *) ((byte *)pheader 
				+ pheader->ofs_frames + i * pheader->framesize);

			memcpy (poutframe2->name, pinframe2->name, sizeof(poutframe2->name));
			for ( j = 0; j < 3; j++ ){
				poutframe2->scale[j] = LittleFloat (pinframe2->scale[j]);
				poutframe2->translate[j] = LittleFloat (pinframe2->translate[j]);
			}
			// verts are all 8 bit, so no swapping needed
			memcpy (poutframe2->verts, pinframe2->verts, 
				pheader->num_xyz*sizeof(dtrivertx2_t));

		}
	}
	else{
		for ( i = 0; i < pheader->num_frames; i++ ){
			pinframe = (daliasframe_t *) ((byte *)pinmodel 
				+ pheader->ofs_frames + i * pheader->framesize);
			poutframe = (daliasframe_t *) ((byte *)pheader 
				+ pheader->ofs_frames + i * pheader->framesize);

			memcpy (poutframe->name, pinframe->name, sizeof(poutframe->name));
			for ( j = 0; j < 3; j++ ){
				poutframe->scale[j] = LittleFloat (pinframe->scale[j]);
				poutframe->translate[j] = LittleFloat (pinframe->translate[j]);
			}
			// verts are all 8 bit, so no swapping needed
			memcpy (poutframe->verts, pinframe->verts, 
				pheader->num_xyz*sizeof(dtrivertx_t));

		}
	}

	mod->type = mod_alias;

	//
	// load the glcmds
	//
	pincmd = (int *) ((byte *)pinmodel + pheader->ofs_glcmds);
	poutcmd = (int *) ((byte *)pheader + pheader->ofs_glcmds);
	for (i=0 ; i<pheader->num_glcmds ; i++)
		poutcmd[i] = LittleLong (pincmd[i]);

	//	Nelno:	do not load skins if this is RESOURCE_FRAMES
	// register all skins
	memcpy ((char *)pheader + pheader->ofs_skins, (char *)pinmodel + pheader->ofs_skins,
		pheader->num_skins*MAX_SKINNAME);
	for (i=0 ; i<pheader->num_skins ; i++)
	{
		sprintf (image_path, "%s", ((char *)pheader + pheader->ofs_skins + i * MAX_SKINNAME));
//		mod->skins[i] = R_FindImage (image_path, it_skin, resource);
		mod->skins[i] = ( image_t * ) RegisterSkin (image_path, (resource_t) resource);
	}

	// setup extradata
	mod->extradata = pheader;
	mod->extradatasize = LittleLong(pinmodel->ofs_end);
}

/*
==============================================================================

SPRITE MODELS

==============================================================================
*/

/*
=================
Mod_LoadSpriteModel
=================
*/
void Mod_LoadSpriteModel (model_t *mod, void *buffer, int resource)
{
	dsprite_t	*sprin, *sprout;
	int			i;
 
	sprin = (dsprite_t *)buffer;
	sprout = (dsprite_t *)ri.X_Malloc(modfilelen, MEM_TAG_SPRITE);
//	sprout = (dsprite_t *)Hunk_Alloc (modfilelen);

	sprout->ident = LittleLong (sprin->ident);
	sprout->version = LittleLong (sprin->version);
	sprout->numframes = LittleLong (sprin->numframes);

	if (sprout->version != SPRITE_VERSION)
		ri.Sys_Error (ERR_DROP, "%s has wrong version number (%i should be %i)",
				 mod->name, sprout->version, SPRITE_VERSION);

	if (sprout->numframes > MAX_MD2SKINS)
		ri.Sys_Error (ERR_DROP, "%s has too many frames (%i > %i)",
				 mod->name, sprout->numframes, MAX_MD2SKINS);

	// byte swap everything
	for (i=0 ; i<sprout->numframes ; i++)
	{
		sprout->frames[i].width = LittleLong (sprin->frames[i].width);
		sprout->frames[i].height = LittleLong (sprin->frames[i].height);
		sprout->frames[i].origin_x = LittleLong (sprin->frames[i].origin_x);
		sprout->frames[i].origin_y = LittleLong (sprin->frames[i].origin_y);
		memcpy (sprout->frames[i].name, sprin->frames[i].name, MAX_SKINNAME);

		if (!strcmp(sprout->frames[i].name + (strlen(sprout->frames[i].name)-4), ".tga"))
		{
			COM_StripExtension (sprout->frames[i].name, sprout->frames[i].name);
			strcat (sprout->frames[i].name, ".bmp");
		}
		mod->skins[i] = (image_t*)R_FindImage (sprout->frames[i].name, it_sprite, resource);
	}

	mod->type = mod_sprite;

	// setup extra data
	mod->extradata = sprout;
	mod->extradatasize = modfilelen;
}

//=============================================================================

/*
================
Mod_Free
================
*/
void Mod_Free (void *mod)
{
	model_t	*model = (model_t*) mod;

	ri.X_Free(model->extradata);
//	Hunk_Free (model->extradata);
	memset (model, 0, sizeof(*model));
}

/*
@@@@@@@@@@@@@@@@@@@@@
BeginRegistration

Specifies the model that will be used as the world
@@@@@@@@@@@@@@@@@@@@@
*/

extern	int	g_loaded_old_wal;

void BeginRegistration (const char *model)
{
	char	fullname[MAX_QPATH];
	cvar_t	*flushmap;

	g_loaded_old_wal = false;

	registration_sequence++;
	r_oldviewcluster = -1;		// force markleafs
	Com_sprintf (fullname, sizeof(fullname), "maps/%s.bsp", model);

	D_FlushCaches ();
	// explicitly free the old map if different
	// this guarantees that mod_known[0] is the world map
	flushmap = ri.Cvar_Get ("flushmap", "0", CVAR_ARCHIVE);
	if ( strcmp(mod_known[0].name, fullname) || flushmap->value)
		Mod_Free (&mod_known[0]);
	r_worldmodel = ( model_t * ) RegisterModel (fullname, RESOURCE_LEVEL);
	R_NewMap ();

	if (g_loaded_old_wal)
	{
		ri.Sys_Warning ("Old format WAL files were found during loading.\nWAL files for this map should be regenerated to decrease load times in GL.");
//		ri.Con_Printf (PRINT_ALL, "Loaded an old format WAL file.\n");
	}
}


/*
@@@@@@@@@@@@@@@@@@@@@
RegisterModel

@@@@@@@@@@@@@@@@@@@@@
*/
void *RegisterModel (const char *name, resource_t resource)
{
	model_t	*mod;
	int		i;
	dsprite_t	*sprout;
	dmdl_t		*pheader;

	mod = Mod_ForName (name, false, resource);

	if (mod)
	{
		mod->registration_sequence = registration_sequence;

		// register any images used by the models
		if (mod->type == mod_sprite)
		{
			sprout = (dsprite_t *)mod->extradata;
			for (i=0 ; i<sprout->numframes ; i++)
				mod->skins[i] = (image_t*)R_FindImage (sprout->frames[i].name, it_sprite, resource);
		}
		else if (mod->type == mod_alias)
		{
			pheader = (dmdl_t *)mod->extradata;
			for (i=0 ; i<pheader->num_skins ; i++)
//				mod->skins[i] = R_FindImage ((char *)pheader + pheader->ofs_skins + i*MAX_SKINNAME, it_skin, resource);
				mod->skins[i] = ( image_t * ) RegisterSkin ((char *)pheader + pheader->ofs_skins + i*MAX_SKINNAME, resource);
		}
		else if (mod->type == mod_brush)
		{
			for (i=0 ; i<mod->numtexinfo ; i++)
				mod->texinfo[i].image->registration_sequence = registration_sequence;
		}
	}
	return mod;
}

/*
@@@@@@@@@@@@@@@@@@@@@
R_EndRegistration

@@@@@@@@@@@@@@@@@@@@@
*/
void R_EndRegistration (void)
{
	int		i;
	model_t	*mod;

	for (i=0, mod=mod_known ; i<mod_numknown ; i++, mod++)
	{
		if (!mod->name[0])
			continue;
		if (mod->registration_sequence != registration_sequence)
		{	// don't need this model
			Mod_Free (mod);
		}
		else
		{	// make sure it is paged in
			Com_PageInMemory ((unsigned char*)mod->extradata, mod->extradatasize);
		}
	}

	R_FreeUnusedImages ();

	if (g_loaded_old_wal)
	{
		ri.Sys_Warning ("Old format WAL files were found during model loading.\nWAL files for models on this map should be regenerated to decrease load times.");
	}
}


//=============================================================================

/*
================
Mod_FreeAll
================
*/
void Mod_FreeAll (void)
{
	int		i;

	for (i=0 ; i<mod_numknown ; i++)
	{
		if (mod_known[i].extradatasize)
			Mod_Free (&mod_known[i]);
	}
}

//---------------------------------------------------------------------------
// SW_PurgeResources()
//---------------------------------------------------------------------------
void PurgeResources(resource_t type)
{
	model_t *model;
	image_t *image;
	short loop;

	// free images
	for (loop=0, image = r_images; loop < numr_images; loop++, image++)
		if (image->resource >= type && image->type!=it_procedural)
			R_FreeImage(image);


	// free models
	for (loop=0, model=mod_known; loop<mod_numknown; loop++, model++)
	{
		if (model == r_worldmodel)
			continue;
		if (model->flags >= type)
			Mod_Free(model);
	}
}

