#include "d3d_local.h"

#include "dk_ref_common.h"
#include "dk_point.h"
#include "dk_array.h"



//-----------------------------------------------------------------------------
// DEFINES
//-----------------------------------------------------------------------------
#define	MAX_MOD_KNOWN	256



//-----------------------------------------------------------------------------
// TYPEDEFS
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------



model_t	*loadmodel;
int		modfilelen;

byte	mod_novis[MAX_MAP_LEAFS/8];

int		mod_numknown;
model_t	mod_known[MAX_MOD_KNOWN];

// the inline * models from the current map are kept seperate
model_t	mod_inline[MAX_MOD_KNOWN];

int		registration_sequence;

extern const CTexture *texture_notexture;

//-----------------------------------------------------------------------------
// FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------
//void GL_BuildPolygonFromSurface(msurface_t *fa);
//void GL_CreateSurfaceLightmap (msurface_t *surf);
//void GL_EndBuildingLightmaps (void);
//void GL_BeginBuildingLightmaps (model_t *m);

void Mod_LoadSpriteModel (model_t *mod, void *buffer, int flags);
void Mod_LoadBrushModel (model_t *mod, void *buffer);
void Mod_LoadAliasModel (model_t *mod, void *buffer, resource_t flags);

model_t *Mod_LoadModel (model_t *mod, qboolean crash);



//-----------------------------------------------------------------------------
// Name: R_FrameNameForModel
// Desc: returns the framename for a given model and frame index
//-----------------------------------------------------------------------------
void R_FrameNameForModel (char *modelname, int frameindex, char *framename)
{
	Sys_Error ("R_FrameNameForModel: no longer supported\n");
}



//-----------------------------------------------------------------------------
// Name: Mod_PointInLeaf
// Desc: 
//-----------------------------------------------------------------------------
mleaf_t *Mod_PointInLeaf (const CVector &p, model_t *model)
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



//-----------------------------------------------------------------------------
// Name: Mod_DecompressVis
// Desc: 
//-----------------------------------------------------------------------------
byte *Mod_DecompressVis( byte *in, model_t *model )
{
	static byte	decompressed[MAX_MAP_LEAFS/8];
	int		c;
	byte	*out;
	int		row;

	row = (model->vis->numclusters+7)>>3;	
	out = decompressed;

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
	
	return decompressed;
}



//-----------------------------------------------------------------------------
// Name: Mod_ClusterPVS
// Desc: 
//-----------------------------------------------------------------------------
byte *Mod_ClusterPVS( int cluster, model_t *model )
{
	if (cluster == -1 || !model->vis)
		return mod_novis;
	return Mod_DecompressVis ( (byte *)model->vis + model->vis->bitofs[cluster][DVIS_PVS],
		model);
}



//-----------------------------------------------------------------------------
// Name: Mod_Modellist_f
// Desc: 
//-----------------------------------------------------------------------------
void Mod_Modellist_f()
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



//-----------------------------------------------------------------------------
// Name: Mod_Init
// Desc: 
//-----------------------------------------------------------------------------
void Mod_Init()
{
	memset (mod_novis, 0xff, sizeof(mod_novis));
}



//-----------------------------------------------------------------------------
// Name: Mod_ForName
// Desc: 
//-----------------------------------------------------------------------------
model_t *Mod_ForName( const char *name, qboolean crash, resource_t resource )
{
	model_t		*mod;
	unsigned	*buf;
	int			i, forced_index = -1;
	int			is_world_model = NOT_WORLD_MODEL;

	if (!name[0])
		ri.Sys_Error (ERR_DROP, "Mod_ForName: NULL name");

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
	//	search the currently loaded models
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

		//	if world model has not changed, just return the current one
		if (!strcmp (mod->name, name))
			return	mod;
	}
	else
	{
		for (i = 0, mod = mod_known + 1; i < mod_numknown; i++, mod++)
		{
			if (!mod->name [0])
				continue;
			
			//	Nelno:	reload this model or return it as found
			if (!strcmp (mod->name, name))
				return	mod;
		}

		//	find a free model slot spot
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
	mod->resource = resource;
	
	//
	// load the file
	//
//	ri.Con_Printf (PRINT_ALL, "Loaded model %s.\n", mod->name);

	modfilelen = ri.FS_LoadFile (mod->name, (void **)&buf);
	if (!buf)
	{
		if (crash)
			ri.Sys_Error (ERR_DROP, "Mod_NumForName: %s not found", mod->name);
		memset (mod->name, 0, sizeof(mod->name));
		return NULL;
	}
	
	loadmodel = mod;

	//
	// fill it in
	//


	// call the apropriate loader
	
//	switch (LittleLong(*(unsigned *)buf))
	switch (*(unsigned *)buf)
	{
		case IDALIASHEADER:
			loadmodel->extradata = Hunk_Begin (0x200000);
			Mod_LoadAliasModel (mod, buf, resource);
			break;
			
		case IDSPRITEHEADER:
			loadmodel->extradata = Hunk_Begin (0x10000);
			Mod_LoadSpriteModel (mod, buf, resource);
			break;
		
		case IDBSPHEADER:
			loadmodel->extradata = Hunk_Begin (0x1000000);
			Mod_LoadBrushModel (mod, buf);
			break;

		default:
			ri.Con_Printf (PRINT_ALL, "Mod_NumForName: unknown field for %s\n", mod->name);
			ri.FS_FreeFile (buf);
			mod_numknown--;
			return NULL;
			break;
	}

	loadmodel->extradatasize = Hunk_End ();

	ri.FS_FreeFile (buf);

	return mod;
}



//-----------------------------------------------------------------------------
// BRUSHMODEL LOADING
//-----------------------------------------------------------------------------
byte	*mod_base;



//-----------------------------------------------------------------------------
// Name: Mod_LoadLighting
// Desc: 
//-----------------------------------------------------------------------------
void Mod_LoadLighting( lump_t *l )
{
	if (!l->filelen)
	{
		loadmodel->lightdata = NULL;
		return;
	}
	loadmodel->lightdata = (byte *)Hunk_Alloc ( l->filelen);	
	memcpy (loadmodel->lightdata, mod_base + l->fileofs, l->filelen);
}



//-----------------------------------------------------------------------------
// Name: Mod_LoadVisibility
// Desc: 
//-----------------------------------------------------------------------------
void Mod_LoadVisibility( lump_t *l )
{
	int		i;

	if (!l->filelen)
	{
		loadmodel->vis = NULL;
		return;
	}
#ifdef	CONSOLIDATE_BSP
	loadmodel->vis = ri.bspModel->dVis;
	
	return;
#endif

	loadmodel->vis = (dvis_t *)Hunk_Alloc ( l->filelen);	
	memcpy (loadmodel->vis, mod_base + l->fileofs, l->filelen);

//	loadmodel->vis->numclusters = LittleLong (loadmodel->vis->numclusters);
	loadmodel->vis->numclusters = loadmodel->vis->numclusters;
	for (i=0 ; i<loadmodel->vis->numclusters ; i++)
	{
		loadmodel->vis->bitofs[i][0] = loadmodel->vis->bitofs[i][0];
		loadmodel->vis->bitofs[i][1] = loadmodel->vis->bitofs[i][1];
	}
}



//-----------------------------------------------------------------------------
// Name: Mod_LoadVertexes
// Desc: 
//-----------------------------------------------------------------------------
void Mod_LoadVertexes( lump_t *l )
{
	dvertex_t	*in;
	mvertex_t	*out;
	int			i, count;

	in = (dvertex_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadVertexes: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (mvertex_t *)Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->vertexes = out;
	loadmodel->numvertexes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
//		out->position[0] = LittleFloat (in->point[0]);
//		out->position[1] = LittleFloat (in->point[1]);
//		out->position[2] = LittleFloat (in->point[2]);
		out->position[0] = in->point[0];
		out->position[1] = in->point[1];
		out->position[2] = in->point[2];
	}
}



//-----------------------------------------------------------------------------
// Name: Mod_LoadExtendedSurfInfo
// Desc: 
//-----------------------------------------------------------------------------
void Mod_LoadExtendedSurfInfo( lump_t *l )
{
	float		*in;
	mtexinfo_t	*out;
	int			i, count;

	in = (float *)(mod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadExtendedSurfInfo: funny lump size in %s",loadmodel->name);

	count = loadmodel->numtexinfo;
	out = loadmodel->texinfo;

	for (i = 0; i < count; i++, out++)
	{
//		out->color [0] = LittleFloat (*in);
		out->color [0] = *in;
		in++;
//		out->color [1] = LittleFloat (*in);
		out->color [1] = *in;
		in++;
//		out->color [2] = LittleFloat (*in);
		out->color [2] = *in;
		in++;
	}
}



//-----------------------------------------------------------------------------
// Name: RadiusFromBounds
// Desc: 
//-----------------------------------------------------------------------------
float RadiusFromBounds( const CVector &mins, const CVector &maxs )
{
	int		i;
	CVector	corner;

	for (i=0 ; i<3 ; i++)
	{
		corner[i] = fabs(mins[i]) > fabs(maxs[i]) ? fabs(mins[i]) : fabs(maxs[i]);
	}

	return corner.Length();

	return 0;
}



//-----------------------------------------------------------------------------
// Name: Mod_LoadSubmodels
// Desc: 
//-----------------------------------------------------------------------------
void Mod_LoadSubmodels( lump_t *l )
{
	dmodel_t	*in;
	mmodel_t	*out;
	int			i, j, count;

	in = (dmodel_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadSubmodels: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (mmodel_t *)Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->submodels = out;
	loadmodel->numsubmodels = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{	// spread the mins / maxs by a pixel
//			out->mins[j] = LittleFloat (in->mins[j]) - 1;
//			out->maxs[j] = LittleFloat (in->maxs[j]) + 1;
//			out->origin[j] = LittleFloat (in->origin[j]);
			out->mins[j] = in->mins[j] - 1;
			out->maxs[j] = in->maxs[j] + 1;
			out->origin[j] = in->origin[j];
		}
		out->radius = RadiusFromBounds (out->mins, out->maxs);
//		out->headnode = LittleLong (in->headnode);
//		out->firstface = LittleLong (in->firstface);
//		out->numfaces = LittleLong (in->numfaces);
		out->headnode = in->headnode;
		out->firstface = in->firstface;
		out->numfaces = in->numfaces;
	}
}



//-----------------------------------------------------------------------------
// Name: Mod_LoadEdges
// Desc: 
//-----------------------------------------------------------------------------
void Mod_LoadEdges( lump_t *l )
{
	dedge_t *in;
	medge_t *out;
	int 	i, count;

	in = (dedge_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadEdges: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (medge_t *)Hunk_Alloc ( (count + 1) * sizeof(*out));	

	loadmodel->edges = out;
	loadmodel->numedges = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
//		out->v[0] = (unsigned short)LittleShort(in->v[0]);
//		out->v[1] = (unsigned short)LittleShort(in->v[1]);
		out->v[0] = (unsigned short)in->v[0];
		out->v[1] = (unsigned short)in->v[1];
	}
}



//-----------------------------------------------------------------------------
// Name: Mod_LoadTexinfo
// Desc: 
//-----------------------------------------------------------------------------
void Mod_LoadTexinfo( lump_t *l )
{
	texinfo_t *in;
	mtexinfo_t *out, *step;
	int 	i, j, count;
	char	name[MAX_QPATH];
	int		next;

	in = (texinfo_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadTexinfo: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (mtexinfo_t *)Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->texinfo = out;
	loadmodel->numtexinfo = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<8 ; j++)
//			out->vecs[0][j] = LittleFloat (in->vecs[0][j]);
			out->vecs[0][j] = in->vecs[0][j];

//		out->flags = LittleLong (in->flags);
		out->flags = in->flags;
//		next = LittleLong (in->nexttexinfo);
		next = in->nexttexinfo;
		if (next > 0)
			out->next = loadmodel->texinfo + next;
		Com_sprintf (name, sizeof(name), "textures/%s.wal", in->texture);

		//	Q2FIXME:	force reload each level?
//		out->image = GL_FindImage (name, it_wall, RESOURCE_LEVEL);
//		out->texture = LoadTexture( name, RESOURCE_LEVEL );
		if (!out->texture)
		{
			ri.Con_Printf (PRINT_ALL, "Couldn't load %s\n", name);
//			out->texture = texture_notexture;
			out->texture = NULL;
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



//-----------------------------------------------------------------------------
// Name: Mod_LoadTexinfoOld
// Desc: 
//-----------------------------------------------------------------------------
void Mod_LoadTexinfoOld( lump_t *l )
{
	texInfoOld_t *in;
	mtexinfo_t *out, *step;
	int 	i, j, count;
	char	name[MAX_QPATH];
	int		next;

	in = (texInfoOld_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadTexinfoOld: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (mtexinfo_t *)Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->texinfo = out;
	loadmodel->numtexinfo = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<8 ; j++)
//			out->vecs[0][j] = LittleFloat (in->vecs[0][j]);
			out->vecs[0][j] = in->vecs[0][j];

//		out->flags = LittleLong (in->flags);
		out->flags = in->flags;
//		next = LittleLong (in->nexttexinfo);
		next = in->nexttexinfo;
		if (next > 0)
			out->next = loadmodel->texinfo + next;
		Com_sprintf (name, sizeof(name), "textures/%s.wal", in->texture);

		//	Q2FIXME:	force reload each level?
//		out->image = GL_FindImage (name, it_wall, RESOURCE_LEVEL);
//		out->texture = LoadTexture( name, RESOURCE_LEVEL );
		if (!out->texture)
		{
			ri.Con_Printf (PRINT_ALL, "Couldn't load %s\n", name);
//			out->texture = texture_notexture;
			out->texture = NULL;
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



//-----------------------------------------------------------------------------
// Name: CalcSurfaceExtents
// Desc: 
//-----------------------------------------------------------------------------
void CalcSurfaceExtents( msurface_t *s )
{
	float	mins[2], maxs[2], val;
	int		i,j, e;
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
	}

	for (i=0 ; i<2 ; i++)
	{	
		bmins[i] = floor(mins[i]/16);
		bmaxs[i] = ceil(maxs[i]/16);

		s->texturemins[i] = bmins[i] * 16;
		s->extents[i] = (bmaxs[i] - bmins[i]) * 16;

//		if ( !(tex->flags & TEX_SPECIAL) && s->extents[i] > 512 /* 256 */ )
//			ri.Sys_Error (ERR_DROP, "Bad surface extents");
	}
}



//-----------------------------------------------------------------------------
// Name: Mod_LoadFaces
// Desc: 
//-----------------------------------------------------------------------------
void Mod_LoadFaces( lump_t *l )
{
	dface_t		*in;
	msurface_t 	*out;
	int			i, count, surfnum;
	int			planenum, side;
	int			ti;

	in = (dface_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadFaces: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (msurface_t *)Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->surfaces = out;
	loadmodel->numsurfaces = count;

	currentmodel = loadmodel;

//	GL_BeginBuildingLightmaps (loadmodel);

	for ( surfnum=0 ; surfnum<count ; surfnum++, in++, out++)
	{
//		out->firstedge = LittleLong(in->firstedge);
		out->firstedge = in->firstedge;
//		out->numedges = LittleShort(in->numedges);		
		out->numedges = in->numedges;		
		out->flags = 0;
		out->surfIndex = surfnum;

//		planenum = LittleShort(in->planenum);
		planenum = in->planenum;
//		side = LittleShort(in->side);
		side = in->side;
		if (side)
		{
			out->flags |= SURF_PLANEBACK;
		}

		out->plane = loadmodel->planes + planenum;
		
		//	FIXME:	temp for debugging!!!  remove planenum from structure!
		if (out->flags & SURF_PLANEBACK)
			out->planeNum = planenum + 1;
		else
			out->planeNum = planenum;

//		ti = LittleShort (in->texinfo);
		ti = in->texinfo;
		if (ti < 0 || ti >= loadmodel->numtexinfo)
			ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: bad texinfo number");
		out->texinfo = loadmodel->texinfo + ti;

		CalcSurfaceExtents (out);
				
	// lighting info

		for (i=0 ; i<MAXLIGHTMAPS ; i++)
			out->styles[i] = in->styles[i];
//		i = LittleLong(in->lightofs);
		i = in->lightofs;
		if (i == -1)
			out->samples = NULL;
		else
			out->samples = loadmodel->lightdata + i;
		
	// set the drawing flags
		
		if (out->texinfo->flags & SURF_WARP)
		{
			out->flags |= SURF_DRAWTURB;
			for (i=0 ; i<2 ; i++)
			{
				out->extents[i] = 16384;
				out->texturemins[i] = -8192;
			}
//			GL_SubdivideSurface (out);	// cut up polygon for warps
		}

		// create lightmaps and polygons
//		if ( !(out->texinfo->flags & (SURF_SKY|SURF_TRANS33|SURF_TRANS66|SURF_WARP|SURF_FULLBRIGHT) ) )
//			GL_CreateSurfaceLightmap (out);

//		if (! (out->texinfo->flags & SURF_WARP) ) 
//			GL_BuildPolygonFromSurface(out);

	}

//	GL_EndBuildingLightmaps ();
}



//-----------------------------------------------------------------------------
// Name: Mod_SetParent
// Desc: 
//-----------------------------------------------------------------------------
void Mod_SetParent( mnode_t *node, mnode_t *parent )
{
	node->parent = parent;
	if (node->contents != -1)
		return;
	Mod_SetParent (node->children[0], node);
	Mod_SetParent (node->children[1], node);
}



//-----------------------------------------------------------------------------
// Name: Mod_LoadNodes
// Desc: 
//-----------------------------------------------------------------------------
void Mod_LoadNodes( lump_t *l )
{
	int			i, j, count, p;
	dnode_t		*in;
	mnode_t 	*out;

	in = (dnode_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadNodes: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (mnode_t *)Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->nodes = out;
	loadmodel->numnodes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
//			out->minmaxs[j] = LittleShort (in->mins[j]);
//			out->minmaxs[3+j] = LittleShort (in->maxs[j]);
			out->minmaxs[j] = in->mins[j];
			out->minmaxs[3+j] = in->maxs[j];

			if (out->minmaxs [j] > out->minmaxs [j + 3])
				ri.Sys_Warning ("Flipped bounds on node!");
		}
	
//		p = LittleLong(in->planenum);
		p = in->planenum;
		out->plane = loadmodel->planes + p;

		out->firstsurface = in->firstface;
		out->numsurfaces = in->numfaces;
		out->contents = -1;	// differentiate from leafs

		for (j=0 ; j<2 ; j++)
		{
//			p = LittleLong (in->children[j]);
			p = in->children[j];
			if (p >= 0)
				out->children[j] = loadmodel->nodes + p;
			else
				out->children[j] = (mnode_t *)(loadmodel->leafs + (-1 - p));
		}
	}
	
	Mod_SetParent (loadmodel->nodes, NULL);	// sets nodes and leafs
}



//-----------------------------------------------------------------------------
// Name: Mod_LoadLeafs
// Desc: 
//-----------------------------------------------------------------------------
void Mod_LoadLeafs( lump_t *l )
{
	dleaf_t 	*in;
	mleaf_t 	*out;
	int			i, j, count, p;
//	glpoly_t	*poly;

	in = (dleaf_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadLeags: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (mleaf_t *)Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->leafs = out;
	loadmodel->numleafs = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
//			out->minmaxs[j] = LittleShort (in->mins[j]);
//			out->minmaxs[3+j] = LittleShort (in->maxs[j]);
			out->minmaxs[j] = in->mins[j];
			out->minmaxs[3+j] = in->maxs[j];

			if (out->minmaxs [j] > out->minmaxs [j + 3])
				ri.Sys_Warning ("Flipped bounds on leaf!");
		}

//		p = LittleLong(in->contents);
		p = in->contents;
		out->contents = p;

//		out->cluster = LittleShort(in->cluster);
//		out->area = LittleShort(in->area);
		out->cluster = in->cluster;
		out->area = in->area;

//		out->firstmarksurface = loadmodel->marksurfaces +
//			LittleShort(in->firstleafface);
		out->firstmarksurface = loadmodel->marksurfaces + in->firstleafface;
//		out->nummarksurfaces = LittleShort(in->numleaffaces);
		out->nummarksurfaces = in->numleaffaces;
		
		// gl underwater warp
#if 0
		if (out->contents & (CONTENTS_WATER|CONTENTS_SLIME|CONTENTS_LAVA|CONTENTS_THINWATER) )
		{
			for (j=0 ; j<out->nummarksurfaces ; j++)
			{
				out->firstmarksurface[j]->flags |= SURF_UNDERWATER;
				for (poly = out->firstmarksurface[j]->polys ; poly ; poly=poly->next)
					poly->flags |= SURF_UNDERWATER;
			}
		}
#endif
	}	
}



//-----------------------------------------------------------------------------
// Name: Mod_LoadMarksurfaces
// Desc: 
//-----------------------------------------------------------------------------
void Mod_LoadMarksurfaces( lump_t *l )
{	
	int		i, j, count;
	short		*in;
	msurface_t **out;
	
	in = (short *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadMarksurfaces: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (msurface_t **)Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->marksurfaces = out;
	loadmodel->nummarksurfaces = count;

	for ( i=0 ; i<count ; i++)
	{
//		j = LittleShort(in[i]);
		j = in[i];
		if (j < 0 ||  j >= loadmodel->numsurfaces)
			ri.Sys_Error (ERR_DROP, "Mod_ParseMarksurfaces: bad surface number");
		out[i] = loadmodel->surfaces + j;
	}
}



//-----------------------------------------------------------------------------
// Name: Mod_LoadSurfedges
// Desc: 
//-----------------------------------------------------------------------------
void Mod_LoadSurfedges( lump_t *l )
{	
	int		i, count;
	int		*in, *out;
	
	in = (int *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadSurfedges: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	if (count < 1 || count >= MAX_MAP_SURFEDGES)
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: bad surfedges count in %s: %i",
		loadmodel->name, count);

	out = (int *)Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->surfedges = out;
	loadmodel->numsurfedges = count;

	for ( i=0 ; i<count ; i++)
//		out[i] = LittleLong (in[i]);
		out[i] = in[i];
}



//-----------------------------------------------------------------------------
// Name: Mod_LoadPlanes
// Desc: 
//-----------------------------------------------------------------------------
void Mod_LoadPlanes( lump_t *l )
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

	in = (dplane_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadPlanes: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = (mplane_t *)Hunk_Alloc ( count*2*sizeof(*out));	
	
	loadmodel->planes = out;
	loadmodel->numplanes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		bits = 0;
		for (j=0 ; j<3 ; j++)
		{
//			out->normal[j] = LittleFloat (in->normal[j]);
			out->normal[j] = in->normal[j];
			if (out->normal[j] < 0)
				bits |= 1<<j;
		}

//		out->dist = LittleFloat (in->dist);
//		out->type = LittleLong (in->type);
		out->dist = in->dist;
		out->type = in->type;
		out->signbits = bits;
	}
}



//-----------------------------------------------------------------------------
// Name: Mod_LoadPlanePolys
// Desc: 
//-----------------------------------------------------------------------------
void Mod_LoadPlanePolys( lump_t *lump )
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
	loadmodel->planePolys = (planeList_t *)Hunk_Alloc (sizeof (void *) * numPlanes);
	memset (loadmodel->planePolys, 0x00, sizeof (void *) * numPlanes);

	for (planeCount = 0; planeCount < numPlanes; planeCount++)
	{
		//	first number is number of faces for current plane
		faceCount = *(int *)in;

		//	check for no faces on this plane
		if (faceCount > 0)
		{
			faceListSize = sizeof (int) * (faceCount + 1);
			loadmodel->planePolys->planes [planeCount] = (faceList_t *)Hunk_Alloc (faceListSize);

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



//-----------------------------------------------------------------------------
// Name: Mod_LoadBrushModel
// Desc: 
//-----------------------------------------------------------------------------
void Mod_LoadBrushModel( model_t *mod, void *buffer )
{
	int			i, version;
	dheader_t	*header;
	mmodel_t 	*bm;
	
	loadmodel->type = mod_brush;
	if (loadmodel != mod_known)
		ri.Sys_Error (ERR_DROP, "Loaded a brush model after the world");

	header = (dheader_t *)buffer;

//	version = LittleLong (header->version);
	version = header->version;
	if (version != BSPVERSION && version != BSPVERSION_TEXINFOOLD &&
		version != BSPVERSION_Q2 && version != BSPVERSION_COLORLUMP)
		ri.Sys_Error (ERR_DROP, "Mod_LoadBrushModel: %s has wrong version number (%i should be %i)", mod->name, version, BSPVERSION);

// swap all the lumps
	mod_base = (byte *)header;

	for (i=0 ; i<sizeof(dheader_t)/4 ; i++)
//		((int *)header)[i] = LittleLong ( ((int *)header)[i]);
		((int *)header)[i] = ((int *)header)[i];

// load into heap
	
	Mod_LoadVertexes (&header->lumps[LUMP_VERTEXES]);
	Mod_LoadEdges (&header->lumps[LUMP_EDGES]);
	Mod_LoadSurfedges (&header->lumps[LUMP_SURFEDGES]);
	Mod_LoadLighting (&header->lumps[LUMP_LIGHTING]);
	Mod_LoadPlanes (&header->lumps[LUMP_PLANES]);
	
	if (version == BSPVERSION_TEXINFOOLD)
		Mod_LoadTexinfoOld (&header->lumps[LUMP_TEXINFO]);
	else
		Mod_LoadTexinfo (&header->lumps[LUMP_TEXINFO]);
	
	Mod_LoadExtendedSurfInfo (&header->lumps [LUMP_EXTSURFINFO]);

	Mod_LoadFaces (&header->lumps[LUMP_FACES]);
	Mod_LoadMarksurfaces (&header->lumps[LUMP_LEAFFACES]);
	Mod_LoadVisibility (&header->lumps[LUMP_VISIBILITY]);
	Mod_LoadLeafs (&header->lumps[LUMP_LEAFS]);
	Mod_LoadNodes (&header->lumps[LUMP_NODES]);
	Mod_LoadSubmodels (&header->lumps[LUMP_MODELS]);
	mod->numframes = 2;		// regular and alternate animation

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
		starmod->radius = bm->radius;
	
		if (i == 0)
			*loadmodel = *starmod;

		starmod->numleafs = bm->visleafs;
	}
}



//-----------------------------------------------------------------------------
// ALIAS MODEL LOADING
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Name: Mod_RegisterEpisodeSkins
// Desc: 
//-----------------------------------------------------------------------------
void Mod_RegisterEpisodeSkins( model_t *model, int episode, int flags )
{
/*
	int			j, i, already_registered = true;
	int			found_postfix = false;
	char		episode_postfix [MAX_OSPATH];
	char		test_postfix [MAX_OSPATH];
	char		dir [_MAX_DIR];
	char		fname [_MAX_FNAME];
	char		ext [_MAX_EXT];
	char		name [MAX_OSPATH];
	image_t		*image;

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

		//	find the gltexture with the same name as model->skins [i]
		//	and free it
		if (model->skins [i]->name && model->skins [i]->name [0] != 0x00)
		{
			for (j = 0, image = gltextures; j < numgltextures; j++, image++)
			{
				if (image->name && image->name [0] != 0x00)
				{
					if (!strcmp (model->skins [i]->name, image->name))
					{
						GL_FreeImage (image);
					}
				}
			}
		}
		
		model->skins [i] = GL_FindImage (name, it_skin, RESOURCE_NPC_SKIN);

//		ri.Con_Printf (PRINT_ALL, "Loaded %s.\n", model->skins [i]->name);
	}
*/
}



//-----------------------------------------------------------------------------
// Name: Mod_LoadAliasModel
// Desc: 
//-----------------------------------------------------------------------------
void Mod_LoadAliasModel( model_t *mod, void *buffer, resource_t resource )
{
	int					i, j, k;
	dmdl_t				*pinmodel, *pheader;
	dstvert_t			*pinst, *poutst;
	dtriangle_t			*pintri, *pouttri;
	daliasframe_t		*pinframe, *poutframe;
	dsurface_t			*pinsurface, *poutsurface;
	int					*pincmd, *poutcmd;
	int					version;
	char				image_path [MAX_OSPATH];
//	char				base[128], newname[128];
//	image_t				*image;

	pinmodel = (dmdl_t *)buffer;

//	version = LittleLong (pinmodel->version);
	version = pinmodel->version;

	if (version != ALIAS_VERSION)
	{
		ri.Con_Printf (PRINT_ALL, "%s has wrong version number (%i should be %i)",
				 mod->name, version, ALIAS_VERSION);
		return;
	}

//	pheader = (dmdl_t *)Hunk_Alloc (LittleLong(pinmodel->ofs_end));
	pheader = (dmdl_t *)Hunk_Alloc (pinmodel->ofs_end);
	
	// byte swap the header fields and sanity check
	for (i=0 ; i<sizeof(dmdl_t)/4 ; i++)
//		((int *)pheader)[i] = LittleLong (((int *)buffer)[i]);
		((int *)pheader)[i] = ((int *)buffer)[i];

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

		// mark heirchical surfaces as no draw
		if (strstr (poutsurface[i].name, "hr_"))
			poutsurface[i].flags |= SRF_NODRAW;

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
//		poutst[i].s = LittleShort (pinst[i].s);
//		poutst[i].t = LittleShort (pinst[i].t);
		poutst[i].s = pinst[i].s;
		poutst[i].t = pinst[i].t;
	}

//
// load triangle lists
//
#if 0
	pintri = (dtriangle_t *) ((byte *)pinmodel + pheader->ofs_tris);
	pouttri = (dtriangle_t *) ((byte *)pheader + pheader->ofs_tris);

	for (i=0 ; i<pheader->num_tris ; i++)
	{
//		pouttri[i].num_uvframes = LittleShort (pintri[i].num_uvframes);
		pouttri[i].num_uvframes = pintri[i].num_uvframes;
		
		for (j=0 ; j<3 ; j++)
//			pouttri[i].index_xyz[j] = LittleShort (pintri[i].index_xyz[j]);
			pouttri[i].index_xyz[j] = pintri[i].index_xyz[j];

		for (j = 0 ; j < pintri[i].num_uvframes; j++)
			for (k = 0 ; k < 3 ; k++)
//				pouttri[i].index_st[j * 3 + k] = LittleShort (pintri[i].index_st[j * 3 + k]);
				pouttri[i].index_st[j * 3 + k] = pintri[i].index_st[j * 3 + k];
	}
#endif
	pintri = (dtriangle_t *) ((byte *)pinmodel + pheader->ofs_tris);
	pouttri = (dtriangle_t *) ((byte *)pheader + pheader->ofs_tris);

	for (i=0 ; i<pheader->num_tris ; i++)
	{
		int	num_uvframes;

//		pouttri->index_surface = LittleShort (pintri->index_surface);
//		pouttri->num_uvframes = LittleShort (pintri->num_uvframes);
		pouttri->index_surface = pintri->index_surface;
		pouttri->num_uvframes = pintri->num_uvframes;
		num_uvframes = pouttri->num_uvframes;

		for (j=0 ; j<3 ; j++)
//			pouttri->index_xyz[j] = LittleShort (pintri->index_xyz[j]);
			pouttri->index_xyz[j] = pintri->index_xyz[j];

		for (j=0 ; j<3 ; j++)
			for (k = 0; k < num_uvframes; k++)
//				pouttri->stframes[k].index_st[j] = LittleShort (pintri->stframes[k].index_st[j]);
				pouttri->stframes[k].index_st[j] = pintri->stframes[k].index_st[j];

		pouttri = (dtriangle_t *) ((byte *) pouttri + sizeof (dtriangle_t) + ((num_uvframes - 1) * sizeof (dstframe_t)));
		pintri = (dtriangle_t *) ((byte *) pintri + sizeof (dtriangle_t) + ((num_uvframes - 1) * sizeof (dstframe_t)));
	}

//
// load the frames
//
	for (i=0 ; i<pheader->num_frames ; i++)
	{
		pinframe = (daliasframe_t *) ((byte *)pinmodel 
			+ pheader->ofs_frames + i * pheader->framesize);
		poutframe = (daliasframe_t *) ((byte *)pheader 
			+ pheader->ofs_frames + i * pheader->framesize);

		memcpy (poutframe->name, pinframe->name, sizeof(poutframe->name));
		for (j=0 ; j<3 ; j++)
		{
//			poutframe->scale[j] = LittleFloat (pinframe->scale[j]);
//			poutframe->translate[j] = LittleFloat (pinframe->translate[j]);
			poutframe->scale[j] = pinframe->scale[j];
			poutframe->translate[j] = pinframe->translate[j];
		}
		// verts are all 8 bit, so no swapping needed
		memcpy (poutframe->verts, pinframe->verts, 
			pheader->num_xyz*sizeof(dtrivertx_t));

	}

	mod->type = mod_alias;

	//
	// load the glcmds
	//
	pincmd = (int *) ((byte *)pinmodel + pheader->ofs_glcmds);
	poutcmd = (int *) ((byte *)pheader + pheader->ofs_glcmds);
	for (i=0 ; i<pheader->num_glcmds ; i++)
//		poutcmd[i] = LittleLong (pincmd[i]);
		poutcmd[i] = pincmd[i];

	// register all skins
	memcpy ((char *)pheader + pheader->ofs_skins, (char *)pinmodel + pheader->ofs_skins,
		pheader->num_skins*MAX_SKINNAME);

	for (i=0 ; i<pheader->num_skins ; i++)
	{
		sprintf (image_path, "%s", ((char *)pheader + pheader->ofs_skins + i * MAX_SKINNAME));
		mod->skins[i] = (CPic *) RegisterSkin (image_path, resource);
	}

	mod->mins[0] = -32;
	mod->mins[1] = -32;
	mod->mins[2] = -32;
	mod->maxs[0] = 32;
	mod->maxs[1] = 32;
	mod->maxs[2] = 32;
}



//-----------------------------------------------------------------------------
// SPRITE MODEL LOADING
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Name: Mod_LoadSpriteModel
// Desc: 
//-----------------------------------------------------------------------------
void Mod_LoadSpriteModel( model_t *mod, void *buffer, int flags )
{
	dsprite_t	*sprin, *sprout;
	int			i;

	sprin = (dsprite_t *)buffer;
	sprout = (dsprite_t *)Hunk_Alloc (modfilelen);


//	sprout->ident = LittleLong (sprin->ident);
//	sprout->version = LittleLong (sprin->version);
//	sprout->numframes = LittleLong (sprin->numframes);
	sprout->ident = sprin->ident;
	sprout->version = sprin->version;
	sprout->numframes = sprin->numframes;

	if (sprout->version != SPRITE_VERSION)
		ri.Sys_Error (ERR_DROP, "%s has wrong version number (%i should be %i)",
				 mod->name, sprout->version, SPRITE_VERSION);

	if (sprout->numframes > MAX_MD2SKINS)
		ri.Sys_Error (ERR_DROP, "%s has too many frames (%i > %i)",
				 mod->name, sprout->numframes, MAX_MD2SKINS);

	// byte swap everything
	for (i=0 ; i<sprout->numframes ; i++)
	{
//		sprout->frames[i].width = LittleLong (sprin->frames[i].width);
//		sprout->frames[i].height = LittleLong (sprin->frames[i].height);
//		sprout->frames[i].origin_x = LittleLong (sprin->frames[i].origin_x);
//		sprout->frames[i].origin_y = LittleLong (sprin->frames[i].origin_y);
		sprout->frames[i].width = sprin->frames[i].width;
		sprout->frames[i].height = sprin->frames[i].height;
		sprout->frames[i].origin_x = sprin->frames[i].origin_x;
		sprout->frames[i].origin_y = sprin->frames[i].origin_y;
		memcpy (sprout->frames[i].name, sprin->frames[i].name, MAX_SKINNAME);

		//	Nelno:	pass resource type flags for this image		
//		mod->skins[i] = GL_FindImage (sprout->frames[i].name,
//			it_sprite, (resource_t)flags);
		mod->skins[i] = LoadPic( sprout->frames[i].name, (resource_t)flags );
	}

	mod->type = mod_sprite;
}



//-----------------------------------------------------------------------------
// MODEL REGISTRATION
//-----------------------------------------------------------------------------



static	int	g_startRegTime;
static	int	g_endRegTime;



//-----------------------------------------------------------------------------
// Name: BeginRegistration
// Desc: 
//-----------------------------------------------------------------------------
void BeginRegistration( const char *model )
{
	char	fullname[MAX_QPATH];
	cvar_t	*flushmap;

	g_startRegTime = timeGetTime ();

	registration_sequence++;
	r_oldviewcluster = -1;		//	force markleafs

	Com_sprintf (fullname, sizeof(fullname), "maps/%s.bsp", model);

	// explicitly free the old map if different
	// this guarantees that mod_known[0] is the world map
	flushmap = ri.Cvar_Get ("flushmap", "0", CVAR_ARCHIVE);

	//	force all models to reload
	if (flushmap->value)
		Mod_FreeAll ();

	if ( strcmp(mod_known[0].name, fullname) || flushmap->value)
		Mod_Free (&mod_known[0]);

	r_worldmodel = Mod_ForName(fullname, true, RESOURCE_LEVEL);

	r_viewcluster = -1;

}


//-----------------------------------------------------------------------------
// Name: RegisterModel
// Desc: 
//-----------------------------------------------------------------------------
void *RegisterModel( const char *name, resource_t resource )
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
				mod->skins[i] = (CPic *) LoadPic (sprout->frames[i].name, resource);
		}
		else if (mod->type == mod_alias)
		{
			pheader = (dmdl_t *)mod->extradata;
			for (i=0 ; i<pheader->num_skins ; i++)
				mod->skins[i] = (CPic *) RegisterSkin ((char *)pheader + pheader->ofs_skins + i*MAX_SKINNAME, resource);
		}
		else if (mod->type == mod_brush)
		{
			return NULL;
//			for (i=0 ; i<mod->numtexinfo ; i++)
//				mod->texinfo[i].texture->registration_sequence = registration_sequence;
		}
	}
	return mod;

	return NULL;
}



//-----------------------------------------------------------------------------
// Name: R_EndRegistration
// Desc: 
//-----------------------------------------------------------------------------
void EndRegistration()
{
/*
	int		i;
	model_t	*mod;

	g_endRegTime = timeGetTime ();
	ri.Con_Printf (PRINT_ALL, "----\nRegistration time: %f seconds.----\n", (float)(g_endRegTime - g_startRegTime) / 1000.0);

	for (i=0, mod=mod_known ; i<mod_numknown ; i++, mod++)
	{
		if (!mod->name[0])
			continue;
		if (mod->registration_sequence != registration_sequence)
		{	// don't need this model
			Mod_Free (mod);
		}
	}

//	GL_FreeUnusedImages ();

	if (g_loaded_old_wal)
	{
		ri.Sys_Warning ("Old format WAL files were found during model loading.\nWAL files for models on this map should be regenerated to decrease load times.");
	}
*/
}



//-----------------------------------------------------------------------------
// Name: Mod_Free
// Desc: 
//-----------------------------------------------------------------------------
void Mod_Free( model_t *mod )
{
	Hunk_Free (mod->extradata);
	memset (mod, 0, sizeof(*mod));
}



//-----------------------------------------------------------------------------
// Name: Mod_FreeAll
// Desc: 
//-----------------------------------------------------------------------------
void Mod_FreeAll()
{
	int		i;

	for (i=0 ; i<mod_numknown ; i++)
	{
		if (mod_known[i].extradatasize)
			Mod_Free (&mod_known[i]);
	}
}



//-----------------------------------------------------------------------------
// Name: PurgeResources
// Desc: 
//-----------------------------------------------------------------------------
void PurgeResources( resource_t type )
{
/*
   model_t *model;
   image_t *image;
   short loop;

   // free images
   for (loop=0, image=gltextures; loop<numgltextures; loop++, image++)
      if (image->resource >= type && image->type!=it_procedural)
//		  image->registration_sequence = registration_sequence;
         GL_FreeImage(image);

   // free models
   for (loop=0, model=mod_known; loop<mod_numknown; loop++, model++)
      if (model->flags >= type)
//		  image->registration_sequence = registration_sequence;
         Mod_Free(model);
*/
}
