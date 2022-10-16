// models.c -- model loading and caching

#include	"gl_local.h"

#include "dk_point.h"
#include "dk_array.h"

#include	"gl_surfsprite.h"

//extern "C" model_t *loadmodel;

model_t	*loadmodel;
int		modfilelen;

void Mod_LoadSpriteModel (model_t *mod, void *buffer, int flags);
void Mod_LoadBrushModel (model_t *mod, void *buffer);
void Mod_LoadAliasModel (model_t *mod, void *buffer, resource_t flags);

model_t *Mod_LoadModel (model_t *mod, qboolean crash);

byte	mod_novis[MAX_MAP_LEAFS/8];

#define	MAX_MOD_KNOWN	1024
model_t	mod_known[MAX_MOD_KNOWN];
int		mod_numknown;

// the inline * models from the current map are kept seperate
model_t	mod_inline[MAX_MOD_KNOWN];

int		registration_sequence;

byte*	bsp_ptr;		// pointer to location of next bmodel load

// Ash -- don't delete
shell_t gAliasShells[50];
int     gNumAliasShells=0;


/*
==============================================================
Mod_ResolveHardpoint

Ash -- Hooks the surface pSurfName up to the hardpoint pHPName
==============================================================
*/

void Mod_ResolveHardpoint(model_t *pModelData, char *pSurfName, char *pHPName)
{
    dmdl_t *pModelHdr = NULL;
    dsurface_t *pSurface = NULL, *pHPSurface = NULL;
    int i, triIndex, hpSurfIndex;

	// SCG[1/23/00]: initialize our vars!
	triIndex = -1;
	// is this a valid model pointer?
	if( pModelData == NULL )
		return;

	// the surface was not found
	if( pSurfName == NULL )
		return;

	// get a pointer to the model header
	pModelHdr = (dmdl_t *)pModelData->extradata;

	// find the surface
	dsurface_t *pSurfaceList = (dsurface_t *) ((byte *) pModelHdr + pModelHdr->ofs_surfaces);
	for (i = 0; i < pModelHdr->num_surfaces &&((pSurface==NULL)||(pHPSurface==NULL)); i++)
	{
		if (!stricmp (pSurfaceList[i].name, pSurfName))
			pSurface = &pSurfaceList[i];
		
        if (!stricmp (pSurfaceList[i].name, pHPName))
        {
            hpSurfIndex = i;	//AMW: not needed once new model format is used
            pHPSurface = &pSurfaceList[i];
        }
	}

    if (!pSurface || !pHPSurface)
        return;

	dtriangle_t *pTriangleList = (dtriangle_t *)((byte *)pModelHdr + pModelHdr->ofs_tris);

	if( pTriangleList == NULL )
	{
		return;
	}

	for (i=0;i<pModelHdr->num_tris;i++)
	{
		if (pTriangleList[i].index_surface == hpSurfIndex)
		{
			triIndex = i;
			break;
		}
	}

	// validate the index
	if (triIndex < 0 || triIndex > pModelHdr->num_tris)
		triIndex = 0;

    pSurface->num_uvframes = triIndex;
}


/*
================
R_FrameNameForModel
================
*/

void R_FrameNameForModel (char *modelname, int frameindex, char *framename)
{
	Sys_Error ("R_FrameNameForModel: no longer supported\n");
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


//===============================================================================

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
	ri.Con_Printf (PRINT_DEVELOPER,"Loaded models:\n");
	for (i=0, mod=mod_known ; i < mod_numknown ; i++, mod++)
	{
		if (!mod->name[0])
			continue;
		ri.Con_Printf (PRINT_DEVELOPER, "%8i : %s\n",mod->extradatasize, mod->name);
		total += mod->extradatasize;
	}
	ri.Con_Printf (PRINT_DEVELOPER, "Total resident: %i\n", total);
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
// SCG[11/13/99]: Old Quake2 size
// SCG[11/13/99]: #define ALIAS_MODEL_HUNK_SIZE		0x200000		// SCG[11/13/99]: 2MB
// SCG[11/13/99]: #define SPRITE_MODEL_HUNK_SIZE	0x10000			// SCG[11/13/99]: 64k
// SCG[11/13/99]: #define BRUSH_MODEL_HUNK_SIZE		0x1000000		// SCG[11/13/99]: 16MB

#define ALIAS_MODEL_HUNK_SIZE	( 0x400000 )		// SCG[11/13/99]: 4MB
#define SPRITE_MODEL_HUNK_SIZE	( 0x10000 )			// SCG[11/13/99]: 64k
#define BRUSH_MODEL_HUNK_SIZE	( 0x1000000 )		// SCG[11/13/99]: 16MB

// cek: win2k -- array moved out of function.  Weird stack problem.
static char		g_filename[256];
model_t *Mod_ForName (const char *name, qboolean crash, resource_t resource)
{
	unsigned	*buf;
	int			i/*, forced_index = -1*/;
	int			is_world_model = NOT_WORLD_MODEL;
	model_t		*mod;
	
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

	strcpy( g_filename, name );
	for( i = 0; i < strlen( g_filename ); i++ )
	{
// Encompass MarkMa 040599
#ifdef	JPN	// JPN
				if(IsDBCSLeadByte(g_filename[i]))	{
					i++;	continue;
				}
#endif	// JPN
// Encompass MarkMa 040599
		if( g_filename[i] == '\\' )
			g_filename[i] = '/';
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
/*
	if (is_world_model)
	{
		mod = mod_known;

		//	if world model has not changed, just return the current one
		if (!strcmp (mod->name, g_filename))
			return	mod;
	}
	else
*/	{
		for (i = 0, mod = mod_known; i < mod_numknown; i++, mod++)
		{
			if (!mod->name [0])
				continue;
			
			//	Nelno:	reload this model or return it as found
			if (!strcmp (mod->name, g_filename))
				return	mod;
		}

		//	find a free model slot spot
		//
		//	Nelno:	start looking after mod_known, because that has to
		//	be the world model!!
		for (i = 0, mod = mod_known; i < mod_numknown; i++, mod++)
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

//	strcpy (mod->name, name);
	strcpy (mod->name, g_filename);
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
	
	switch (LittleLong(*(unsigned *)buf))
	{
		case IDALIASHEADER:
			loadmodel->extradata = Hunk_Begin (ALIAS_MODEL_HUNK_SIZE);
			Mod_LoadAliasModel (mod, buf, resource);
			break;
			
		case IDSPRITEHEADER:
			loadmodel->extradata = Hunk_Begin (SPRITE_MODEL_HUNK_SIZE);
			Mod_LoadSpriteModel (mod, buf, resource);
			break;
		
		case IDBSPHEADER:
			loadmodel->extradata = Hunk_Begin (BRUSH_MODEL_HUNK_SIZE);  //$$$ FIXMEAMW		s/b 0x1000000
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

/*
===============================================================================

					BRUSHMODEL LOADING

===============================================================================
*/

byte	*mod_base;


/*
=================
Mod_LoadLighting
=================
*/
void Mod_LoadLighting (lump_t *l)
{
	if (!l->filelen)
	{
		loadmodel->lightdata = NULL;
		return;
	}
	
	// mdm99.03.17 - get the ptr and inc
//	loadmodel->lightdata = (byte *)bsp_ptr;
//	bsp_ptr += l->filelen;
	
	loadmodel->lightdata = (byte *)Hunk_Alloc ( l->filelen );	
	memcpy (loadmodel->lightdata, mod_base + l->fileofs, l->filelen);
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
#ifdef	CONSOLIDATE_BSP
	loadmodel->vis = ri.bspModel->dVis;
	
	return;
#endif

	// mdm99.03.17 - get the ptr and inc
	//loadmodel->vis = (dvis_t*)bsp_ptr;
	//bsp_ptr += l->filelen;
	loadmodel->vis = (dvis_t *)Hunk_Alloc ( l->filelen );	
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

	in = (dvertex_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadVertexes: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	
	// mdm99.03.17 - get the ptr and inc
	//out = (mvertex_t*)bsp_ptr;
	//bsp_ptr += (count*sizeof(mvertex_t));
	out = (mvertex_t *)Hunk_Alloc ( count*sizeof(*out));	
	
	loadmodel->vertexes = out;
	loadmodel->numvertexes = count;
	
	for ( i=0 ; i<count ; i++, in++, out++)
	{
		out->position.x = LittleFloat (in->point[0]);
		out->position.y = LittleFloat (in->point[1]);
		out->position.z = LittleFloat (in->point[2]);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	Mod_LoadExtendedSurfInfo
//
//	must load after texinfo lump!
///////////////////////////////////////////////////////////////////////////////

void Mod_LoadExtendedSurfInfo (lump_t *l)
{
	float		*in;
	mtexinfo_t	*out;
	int			i, count;
	int			r, g, b;

	in = (float *)(mod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadExtendedSurfInfo: funny lump size in %s",loadmodel->name);

	count = loadmodel->numtexinfo;
	out = loadmodel->texinfo;
	out = loadmodel->texinfo;

	for (i = 0; i < count; i++, out++)
	{
		r = LittleFloat (*in);
		in++;
		g = LittleFloat (*in);
		in++;
		b = LittleFloat (*in);
		in++;

		out->color = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b & 0xf8) >> 3);
	}
}


/*
=================
RadiusFromBounds
=================
*/
float RadiusFromBounds (CVector &mins, CVector &maxs)
{
	CVector	corner;

	corner.x = fabs(mins.x) > fabs(maxs.x) ? fabs(mins.x) : fabs(maxs.x);
	corner.y = fabs(mins.y) > fabs(maxs.y) ? fabs(mins.y) : fabs(maxs.y);
	corner.z = fabs(mins.z) > fabs(maxs.z) ? fabs(mins.z) : fabs(maxs.z);

	return corner.Length();
}


/*
=================
Mod_LoadSubmodels
=================
*/
void Mod_LoadSubmodels (lump_t *l)
{
	dmodel_t	*in;
	mmodel_t	*out;
	int			i, count;

	in = (dmodel_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadSubmodels: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);

	//out = (mmodel_t*)bsp_ptr;
	//bsp_ptr += (count*sizeof(*out));
	out = ( mmodel_t * ) Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->submodels = out;
	loadmodel->numsubmodels = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		// spread the mins / maxs by a pixel
		out->mins.x = LittleFloat (in->mins.x) - 1;
		out->mins.y = LittleFloat (in->mins.y) - 1;
		out->mins.z = LittleFloat (in->mins.z) - 1;

		out->maxs.x = LittleFloat (in->maxs.x) + 1;
		out->maxs.y = LittleFloat (in->maxs.y) + 1;
		out->maxs.z = LittleFloat (in->maxs.z) + 1;

		out->origin.x = LittleFloat (in->origin.x);
		out->origin.y = LittleFloat (in->origin.y);
		out->origin.z = LittleFloat (in->origin.z);

		out->radius = RadiusFromBounds (out->mins, out->maxs);
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

	in = (dedge_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadEdges: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);

	//out = (medge_t*)bsp_ptr;	
	//bsp_ptr += ((count+1)*sizeof(*out));
	out = ( medge_t * ) Hunk_Alloc ( (count + 1) * sizeof(*out));	

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
//image_t	*R_ProcTextureInit( const char *name, int width, int height );

void Mod_LoadTexinfo (lump_t *l)
{
	texinfo_t *in;
	mtexinfo_t *out, *step;
	int 	i, count;
	char	name[MAX_QPATH];
	int		next;

	in = (texinfo_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadTexinfo: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);

//	pTexInfo = bsp_ptr;
	//out = (mtexinfo_t*)bsp_ptr;
	//bsp_ptr += (count*sizeof(*out));
	out = ( mtexinfo_t * )Hunk_Alloc ( count*sizeof(*out));	

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

		out->value = LittleLong( in->value );
		out->flags = LittleLong (in->flags);
		next = LittleLong (in->nexttexinfo);
		if (next > 0)
			out->next = loadmodel->texinfo + next;
		Com_sprintf (name, sizeof(name), "textures/%s.wal", in->texture);

		//	Q2FIXME:	force reload each level?
		out->image = R_FindImage (name, it_wall, RESOURCE_LEVEL);

		if (!out->image)
		{
			ri.Con_Printf (PRINT_ALL, "Couldn't load %s\n", name);
			out->image = r_notexture;
		}
	}
 
	// count animation frames
	for (i=0 ; i<count ; i++)
	{
		out = &loadmodel->texinfo[i];
		out->numframes = 1;

		for (step = out->next ; step && step != out ; step=step->next)
		{
			out->numframes++;

			// amw - to catch corrupted .wal files..
			if (out->numframes > 1024)
			{
				ri.Sys_Error (ERR_DROP, "MOD_LoadTexinfo: corrupted .wal file %s", out->image->name);
			}
		}

	}
}
								
/*
================
CalcSurfaceExtents

Fills in s->texturemins[] and s->extents[]
================
*/
void CalcSurfaceExtents( msurface_t *pSurf )
{
	float	mins[2], maxs[2], val;
	int		i, e;
	mvertex_t	*v;
	mtexinfo_t	*tex;
	int		bmins[2], bmaxs[2];

	mins[0] = mins[1] = 999999;
	maxs[0] = maxs[1] = -99999;

	tex = pSurf->texinfo;
	
	for( i = 0; i < pSurf->numedges; i++ )
	{
		e = loadmodel->surfedges[pSurf->firstedge+i];
		if (e >= 0)
		{
			v = &loadmodel->vertexes[loadmodel->edges[e].v[0]];
		}
		else
		{
			v = &loadmodel->vertexes[loadmodel->edges[-e].v[1]];
		}
		
		val = v->position.x * tex->s.x + v->position.y * tex->s.y + v->position.z * tex->s.z + tex->s_offset;

		if (val < mins[0] )
		{
			mins[0] = val;
		}
		if ( val > maxs[0] )
		{
			maxs[0] = val;
		}

		val = v->position.x * tex->t.x + v->position.y * tex->t.y + v->position.z * tex->t.z + tex->t_offset;
		if (val < mins[1])
		{
			mins[1] = val;
		}
		if ( val > maxs[1] )
		{
			maxs[1] = val;
		}
	}

	for( i = 0; i < 2 ;i++ )
	{	
		bmins[i] = floor( mins[i] / 16 );
		bmaxs[i] = ceil( maxs[i] / 16 );

		pSurf->texturemins[i] = bmins[i] * 16;
		pSurf->extents[i] = ( bmaxs[i] - bmins[i] ) * 16;
	}
}


void GL_BuildPolygonFromSurface( msurface_t *pSurf );
void GL_CreateSurfaceLightmap( msurface_t *pSurf );
void GL_EndBuildingLightmaps (void);
void GL_BeginBuildingLightmaps (model_t *m);

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
	int			ti;

	in = (dface_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadFaces: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	
	//out = (msurface_t*)bsp_ptr;
	//bsp_ptr += (count*sizeof(*out));
	out = (msurface_t *)Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->surfaces = out;
	loadmodel->numsurfaces = count;

	currentmodel = loadmodel;

	GL_BeginBuildingLightmaps (loadmodel);

	for ( surfnum=0 ; surfnum<count ; surfnum++, in++, out++)
	{
		out->firstedge = LittleLong(in->firstedge);
		out->numedges = LittleShort(in->numedges);		
		out->flags = 0;

		planenum = LittleShort(in->planenum);
		side = LittleShort(in->side);
		if (side)
		{
			out->flags |= SURF_PLANEBACK;
		}

		out->plane = loadmodel->planes + planenum;
		
		out->nNumSurfSprites = 0;

		ti = LittleShort (in->texinfo);
		if (ti < 0 || ti >= loadmodel->numtexinfo)
			ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: bad texinfo number");
		out->texinfo = loadmodel->texinfo + ti;

		CalcSurfaceExtents (out);
				
	// lighting info

		for (i=0 ; i<MAXLIGHTMAPS ; i++)
			out->styles[i] = in->styles[i];
		i = LittleLong(in->lightofs);
		if (i == -1)
			out->samples = NULL;
		else
			out->samples = loadmodel->lightdata + i;
		
		// create lightmaps and polygons
		if ( !( out->texinfo->flags & ( SURF_SKY | SURF_FULLBRIGHT) ) && 
			( out->texinfo->flags != SURF_FOGPLANE ) )
			GL_CreateSurfaceLightmap (out);

		// set the drawing flags
		if (out->texinfo->flags & SURF_WARP)
		{
			out->flags |= SURF_DRAWTURB;

			GL_SubdivideSurface (out);	// cut up polygon for warps
		}

		if (! (out->texinfo->flags & (SURF_WARP)) ) 
			GL_BuildPolygonFromSurface(out);
	}


	GL_EndBuildingLightmaps ();
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

	in = (dnode_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadNodes: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	
	//out = (mnode_t*)bsp_ptr;
	//bsp_ptr += (count*sizeof(*out));
	out = (mnode_t *)Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->nodes = out;
	loadmodel->numnodes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		out->mins.x = LittleShort( in->mins[0] );
		out->maxs.x = LittleShort( in->maxs[0] );

		out->mins.y = LittleShort( in->mins[1] );
		out->maxs.y = LittleShort( in->maxs[1] );

		out->mins.z = LittleShort( in->mins[2] );
		out->maxs.z = LittleShort( in->maxs[2] );

		if( ( out->mins.y > out->maxs.y ) || 
			( out->mins.x > out->maxs.x ) || 
			( out->mins.z > out->maxs.z ) ){
			ri.Sys_Warning ("Flipped bounds on node!");
		}

		p = LittleLong(in->planenum);
		out->plane = loadmodel->planes + p;

		out->firstsurface = LittleShort (in->firstface);
		out->numsurfaces = LittleShort (in->numfaces);
		out->contents = -1;	// differentiate from leafs

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
	int			i, count, p, brushnum;
	
	in = (dleaf_t *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadLeags: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);

	//out = (mleaf_t*)bsp_ptr;
	//bsp_ptr += (count*sizeof(*out));
	out = (mleaf_t *)Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->leafs = out;
	loadmodel->numleafs = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		out->mins.x = LittleShort( in->mins[0] );
		out->maxs.x = LittleShort( in->maxs[0] );

		out->mins.y = LittleShort( in->mins[1] );
		out->maxs.y = LittleShort( in->maxs[1] );

		out->mins.z = LittleShort( in->mins[2] );
		out->maxs.z = LittleShort( in->maxs[2] );

		if( ( out->mins.y > out->maxs.y ) || 
			( out->mins.x > out->maxs.x ) || 
			( out->mins.z > out->maxs.z ) ){
			ri.Sys_Warning ("Flipped bounds on node!");
		}

		p = LittleLong(in->contents);
		out->contents = p;

		out->cluster = LittleShort(in->cluster);
		out->area = LittleShort(in->area);

		out->firstmarksurface = loadmodel->marksurfaces +
			LittleShort(in->firstleafface);
		out->nummarksurfaces = LittleShort(in->numleaffaces);

		// SCG[4/17/99]: Added for volumetric fog
		if (out->contents & (CONTENTS_FOG) )
		{
			brushnum = R_FogVolumeForBrush( in->brushnum );
			for (int j=0 ; j<out->nummarksurfaces ; j++)
			{
				out->firstmarksurface[j]->flags |= SURF_DRAWFOG;
				out->firstmarksurface[j]->brushnum = brushnum;
				

				r_fogvolumes[brushnum].hull.Add( ( void * ) out->firstmarksurface[j] );
			}
		}
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
	unsigned short		*in;
	msurface_t **out;
	
	in = (unsigned short *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadMarksurfaces: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);

	//out = (msurface_t**)bsp_ptr;
	//bsp_ptr += (count*sizeof(*out));
	out = ( msurface_t ** )Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->marksurfaces = out;
	loadmodel->nummarksurfaces = count;

	for ( i=0 ; i<count ; i++)
	{
		j = LittleShort(in[i]);
		if (j < 0 ||  j >= loadmodel->numsurfaces)
			ri.Sys_Error (ERR_DROP, "Mod_ParseMarksurfaces: bad surface number");
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
	
	in = (int *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadSurfedges: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	if (count < 1 || count >= MAX_MAP_SURFEDGES)
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: bad surfedges count in %s: %i",
		loadmodel->name, count);

	//out = (int*)bsp_ptr;
	//bsp_ptr += (count*sizeof(*out));
	out = ( int * )Hunk_Alloc ( count*sizeof(*out));	

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
	int			i;
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

	//out = (mplane_t*)bsp_ptr;
	//bsp_ptr += (count*2*sizeof(*out));
	out = ( mplane_t * ) Hunk_Alloc ( count*2*sizeof(*out));	

	loadmodel->planes = out;
	loadmodel->numplanes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		bits = 0;

		out->normal.x = LittleFloat (in->normal.x);
		if (out->normal.x < 0)
			bits |= 1;
		out->normal.y = LittleFloat (in->normal.y);
		if (out->normal.y < 0)
			bits |= 2;
		out->normal.z = LittleFloat (in->normal.z);
		if (out->normal.z < 0)
			bits |= 4;


		out->dist = LittleFloat (in->dist);
		out->type = LittleLong (in->type);
		out->signbits = bits;
		out->planeIndex = i;
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

	numPlanes = loadmodel->numplanes;

	maxFacesForAPlane = 0;
	minFacesForAPlane = MAX_POLYS_PER_PLANE + 1;
	numUsedPlanes = totalFacesForPlanes = 0;

	numPlanes = loadmodel->numplanes;
	//loadmodel->planePolys = (planeList_t*)bsp_ptr;
	//bsp_ptr += (sizeof(void*)*numPlanes);
	loadmodel->planePolys = (planeList_t *)Hunk_Alloc (sizeof (void *) * numPlanes);

	for (planeCount = 0; planeCount < numPlanes; planeCount++)
	{
		//	first number is number of faces for current plane
		faceCount = *(int *)in;

		//	check for no faces on this plane
		if (faceCount > 0)
		{
			faceListSize = sizeof (int) * (faceCount + 1);
			//loadmodel->planePolys->planes[planeCount] = (faceList_t*)bsp_ptr;
			//bsp_ptr += faceListSize;
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

/*
=================
Mod_LoadBrushModel
=================
*/
void Mod_LoadBrushModel (model_t *mod, void *buffer)
{
	int			i, version;
	dheader_t	*header;
	mmodel_t 	*bm;
//	int			size;

	memset( r_fogvolumes, 0, sizeof( fogvolume_t ) * 64 );
    r_fog_volume_count = 0;

	loadmodel->type = mod_brush;
	if (loadmodel != mod_known)
		ri.Sys_Error (ERR_DROP, "Loaded a brush model after the world");

	header = (dheader_t *)buffer;

	version = LittleLong (header->version);
	if( version != BSPVERSION )
		ri.Sys_Error (ERR_FATAL, "Mod_LoadBrushModel: %s has wrong version number (%i should be %i)", mod->name, version, BSPVERSION);

// swap all the lumps
	mod_base = (byte *)header;

	for (i=0 ; i<sizeof(dheader_t)/4 ; i++)
		((int *)header)[i] = LittleLong ( ((int *)header)[i]);

/*
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
		(header->lumps[LUMP_MODELS].filelen/sizeof(dmodel_t))*sizeof(mmodel_t) +	// Mod_LoadSubModels
		(header->lumps[LUMP_PLANEPOLYS].filelen*2); 

	// allocate the bsp model memory
	bsp_ptr = (byte*)ri.X_Malloc(size,MEM_TAG_BMODEL);

	// setup extra data
	mod->extradata = bsp_ptr;
	mod->extradatasize = size;
*/

// load into heap
	
	Mod_LoadVertexes (&header->lumps[LUMP_VERTEXES]);
	Mod_LoadEdges (&header->lumps[LUMP_EDGES]);
	Mod_LoadSurfedges (&header->lumps[LUMP_SURFEDGES]);
	Mod_LoadLighting (&header->lumps[LUMP_LIGHTING]);
	Mod_LoadPlanes (&header->lumps[LUMP_PLANES]);
	Mod_LoadTexinfo (&header->lumps[LUMP_TEXINFO]);
	Mod_LoadExtendedSurfInfo (&header->lumps [LUMP_EXTSURFINFO]);
	Mod_LoadFaces (&header->lumps[LUMP_FACES]);
	Mod_LoadMarksurfaces (&header->lumps[LUMP_LEAFFACES]);
	Mod_LoadVisibility (&header->lumps[LUMP_VISIBILITY]);
	Mod_LoadLeafs (&header->lumps[LUMP_LEAFS]);
	Mod_LoadNodes (&header->lumps[LUMP_NODES]);
	Mod_LoadSubmodels (&header->lumps[LUMP_MODELS]);
	mod->numframes = 2;		// regular and alternate animation

	if( version < BSPVERSION )
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

	// no more hunk avaliable
	bsp_ptr = NULL;

	// SCG[4/19/99]: Init fog volumes
	R_FogInit();

	// SCG[5/27/99]: Subdivide fog surfaces
	msurface_t	*pSurf;
	for( int s = 0; s < loadmodel->numsurfaces; s++ )
	{
		pSurf = &loadmodel->surfaces[s];
		if( pSurf->flags & SURF_DRAWFOG )
		{
			ri.X_Free( pSurf->polys );
//			Hunk_Free( pSurf->polys );
			pSurf->polys = NULL;
			GL_SubdivideSurface( pSurf );	// cut up polygon for warps
		}
	}
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

void	Mod_RegisterEpisodeSkins (void *mod, int episode, int flags)
{
	int			j, i, already_registered = true;
	int			found_postfix = false;
	char		episode_postfix [MAX_OSPATH];
	char		test_postfix [MAX_OSPATH];
	char		dir [_MAX_DIR];
	char		fname [_MAX_FNAME];
	char		ext [_MAX_EXT];
	char		name [MAX_OSPATH];
	image_t		*image;
	model_t		*model;

	model = (model_t*)mod;

	if (!model)
		return;

	if (episode > 9)
	{
		ri.Sys_Error (ERR_FATAL, "Tried to register skin for episode > 9\n");
		return;
	}

// SCG[1/16/00]: 	sprintf (episode_postfix, "_e%i", episode);
	Com_sprintf (episode_postfix,sizeof(episode_postfix), "_e%i", episode);

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
		return;
	}

	//	not registered yet, so purge everything and reload
	for (i = 0; model->skins [i] != NULL; i++)
	{
		_splitpath (model->skins [i]->name, NULL, dir, fname, ext);

		for (j = 1; j < 10; j++)
		{
// SCG[1/16/00]: 			sprintf (test_postfix, "_e%i", j);
			Com_sprintf (test_postfix,sizeof(test_postfix), "_e%i", j);
			if (strstr (fname, test_postfix) == (fname + strlen (fname) - 3))
			{
				//	strip off the postfix if it is there
				fname [strlen (fname) - 3] = 0x00;
// SCG[1/16/00]: 				sprintf (name, "%s%s%s%s", dir, fname, episode_postfix, ext);
				Com_sprintf (name,sizeof(name), "%s%s%s%s", dir, fname, episode_postfix, ext);
				found_postfix = true;
				break;
			}
		}

		if (!found_postfix)
		{
			//	just put the postfix on the end
// SCG[1/16/00]: 			sprintf (name, "%s%s%s%s", dir, fname, episode_postfix, ext);
			Com_sprintf (name,sizeof(name), "%s%s%s%s", dir, fname, episode_postfix, ext);
		}

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
		
		model->skins[i] = R_FindImage (name, it_skin, RESOURCE_NPC_SKIN);
	}

}

/*
=================
Mod_LoadAliasModel
=================
*/
void Mod_LoadAliasModel (model_t *mod, void *buffer, resource_t resource)
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

	if (version != ALIAS_VERSION && version != ALIAS_VERSION2)
	{
		ri.Con_Printf (PRINT_ALL, "%s has wrong version number (%i should be %i)",
				 mod->name, version, ALIAS_VERSION);
		return;
	}

//	pheader = (dmdl_t *)ri.X_Malloc(LittleLong(pinmodel->ofs_end), MEM_TAG_MODEL);
	pheader = (dmdl_t *)Hunk_Alloc (LittleLong(pinmodel->ofs_end));
	
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
		poutsurface[i].num_uvframes	= -1;  // num_uvframes is now the hardpoint tri index

//		poutsurface[i].num_st		= pinsurface[i].num_st;
//		poutsurface[i].num_xyz		= pinsurface[i].num_xyz;
//		poutsurface[i].num_tris		= pinsurface[i].num_tris;

/***** ANDREW: THIS ULTIMATELY NEEDS TO BE REMOVED *************

		// mark heirchical surfaces as no draw
		index = ri.clhr_IndexForSurfaceName (poutsurface [i].name);
		if (index >= 0)
		{
			if (!(ri.clhr_FlagsForSurfaceIndex (index) & SRFL_VISIBLE))
				poutsurface[i].flags |= SRF_NODRAW;
		}
***************************************************************/
		// mark old shitty hierarchical surfaces as no draw
// Ash -- if we make them no draw, we lose gun muzzles
//		if (strstr (poutsurface[i].name, "hr_"))
//			poutsurface[i].flags |= SRF_NODRAW;

		// amw 4.26.99
		// a specific "nd_chrome" surface is usually created just to pull in an
		// environment map.. it is a NODRAW surface... artists will usually
		// name this surface "nd_chrome"...  any *other* surface with "chrome"
		// in it should be drawn with the environment map
		if (strstr (poutsurface[i].name, "nd_"))
			poutsurface[i].flags |= SRF_NODRAW;
		
        if (strstr (poutsurface[i].name, "chrome"))
			poutsurface[i].flags |= SRF_ENVMAP;
        else if (strstr (poutsurface[i].name, "shinya"))
			poutsurface[i].flags |= SRF_TEXENVMAP_ADD;
        else if (strstr (poutsurface[i].name, "shinym"))
			poutsurface[i].flags |= SRF_TEXENVMAP_MULT;
        else if (strstr (poutsurface[i].name, "shiny"))
			poutsurface[i].flags |= SRF_TEXENVMAP_ADD;
        else if (strstr (poutsurface[i].name, "remove"))
			poutsurface[i].flags |= SRF_NODRAW;
        

		// check for translucent surfaces
		if (strstr(poutsurface[i].name,"33"))
			poutsurface[i].flags |= SRF_TRANS33;
		else if (strstr(poutsurface[i].name,"66"))
			poutsurface[i].flags |= SRF_TRANS66;

		// head surfaces shouldn't change skins...
		// main characters have two skins.. one for the head.. another for the body
		if (strstr(poutsurface[i].name,"head"))
			poutsurface[i].flags |= SRF_STATICSKIN;

		// make the little surface triangle draw so you can see where it's supposed to be
		if (strstr (poutsurface[i].name, "hp_"))
			poutsurface[i].flags |= SRF_NODRAW;

        // detect fullbright surfaces
		if (strstr (poutsurface[i].name, "bright"))
			poutsurface[i].flags |= SRF_FULLBRIGHT;

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
		for (i=0 ; i<pheader->num_frames ; i++)
		{
			pinframe2 = (daliasframe2_t *) ((byte *)pinmodel 
				+ pheader->ofs_frames + i * pheader->framesize);
			poutframe2 = (daliasframe2_t *) ((byte *)pheader 
				+ pheader->ofs_frames + i * pheader->framesize);

			memcpy (poutframe2->name, pinframe2->name, sizeof(poutframe2->name));

			poutframe2->scale.x = LittleFloat (pinframe2->scale.x);
			poutframe2->scale.y = LittleFloat (pinframe2->scale.y);
			poutframe2->scale.z = LittleFloat (pinframe2->scale.z);
			poutframe2->translate.x = LittleFloat (pinframe2->translate.x);
			poutframe2->translate.y = LittleFloat (pinframe2->translate.y);
			poutframe2->translate.z = LittleFloat (pinframe2->translate.z);

			// verts are all 8 bit, so no swapping needed
			memcpy (poutframe2->verts, pinframe2->verts, 
				pheader->num_xyz*sizeof(dtrivertx2_t));

		}
	}
	else{
		for (i=0 ; i<pheader->num_frames ; i++)
		{
			pinframe = (daliasframe_t *) ((byte *)pinmodel 
				+ pheader->ofs_frames + i * pheader->framesize);
			poutframe = (daliasframe_t *) ((byte *)pheader 
				+ pheader->ofs_frames + i * pheader->framesize);

			memcpy (poutframe->name, pinframe->name, sizeof(poutframe->name));

			poutframe->scale.x = LittleFloat (pinframe->scale.x);
			poutframe->scale.y = LittleFloat (pinframe->scale.y);
			poutframe->scale.z = LittleFloat (pinframe->scale.z);
			poutframe->translate.x = LittleFloat (pinframe->translate.x);
			poutframe->translate.y = LittleFloat (pinframe->translate.y);
			poutframe->translate.z = LittleFloat (pinframe->translate.z);

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

	// register all skins
	memcpy ((char *)pheader + pheader->ofs_skins, (char *)pinmodel + pheader->ofs_skins,
		pheader->num_skins*MAX_SKINNAME);

/*
	for (i=0 ; i<pheader->num_skins ; i++)
	{
		sprintf (image_path, "%s", ((char *)pheader + pheader->ofs_skins + i * MAX_SKINNAME));
		mod->skins[i] = GL_FindImage (image_path, it_skin, resource);
	}
*/
	for (i=0 ; i<pheader->num_skins ; i++)
	{
// SCG[1/16/00]: 		sprintf (image_path, "%s", ((char *)pheader + pheader->ofs_skins + i * MAX_SKINNAME));
		Com_sprintf (image_path,sizeof(image_path), "%s", ((char *)pheader + pheader->ofs_skins + i * MAX_SKINNAME));
		mod->skins[i] = (image_t*)RegisterSkin (image_path, resource);
	}

	mod->mins.x = -32;
	mod->mins.y = -32;
	mod->mins.z = -32;
	mod->maxs.x = 32;
	mod->maxs.y = 32;
	mod->maxs.z = 32;

	// setup extradata
	mod->extradata = pheader;
	mod->extradatasize = LittleLong(pinmodel->ofs_end);

    // resolve the head hardpoint
    Mod_ResolveHardpoint(mod, "s_head", "hp_head");

//  Ash -- don't delete
/*
    // create shell space
    shell_t *pShell = gAliasShells+gNumAliasShells;
    gNumAliasShells++;
//    shell_patch_t *pPatches = new shell_patch_t[pheader->num_tris];
//    float *pVData = new float[pheader->num_xyz];
    
    pShell->pModel = mod;
    pShell->pPatches = pPatches;
    pShell->numPatches = pheader->num_tris;
    pShell->pVData = pVData;
    pShell->numVData = pheader->num_xyz;
    pShell->numActive = 0;

    
    // init 
    for (i=0;i<pShell->numVData;i++)
        pShell->pVData[i] = 0.0f;

    for (i=0;i<pShell->numPatches;i++)
    {
        pPatches[i].index = i;
        pPatches[i].nindex[0] = -1;
        pPatches[i].nindex[1] = -1;
        pPatches[i].nindex[2] = -1;
        pPatches[i].data = 0;
    }

    dtriangle_t *pitri, *pjtri;
    int ie, je;
    int iv0, iv1, jv0, jv1;

    // create neighbor data
    pitri = (dtriangle_t *) ((byte *)pheader + pheader->ofs_tris);
    for (i=0;i<pheader->num_tris;i++)
    {
        pPatches[i].vindex[0] = pitri->index_xyz[0];
        pPatches[i].vindex[1] = pitri->index_xyz[1];
        pPatches[i].vindex[2] = pitri->index_xyz[2];

        // look for a neighbor
        pjtri = (dtriangle_t *) ((byte *) pitri + sizeof (dtriangle_t) + ((pitri->num_uvframes - 1) * sizeof (dstframe_t)));
        for (j=i+1;j<pheader->num_tris;j++)
        {
            // look for a shared edge
            for (ie=0;ie<3;ie++)
            {
                iv0 = pitri->index_xyz[ie];
                
                if (ie<2)
                    iv1 = pitri->index_xyz[ie+1];
                else
                    iv1 = pitri->index_xyz[0];

                for (je=0;je<3;je++)
                {
                    jv0 = pjtri->index_xyz[je];

                    if (je<2)
                        jv1 = pjtri->index_xyz[je+1];
                    else
                        jv1 = pjtri->index_xyz[0];
                    
                    // are we neighbors?
                    if ((iv0==jv1)&&(iv1==jv0))
                    {
                        pPatches[i].nindex[ie] = j;
                        pPatches[j].nindex[je] = i;
                    }
                }
            }
            pjtri = (dtriangle_t *) ((byte *) pjtri + sizeof (dtriangle_t) + ((pjtri->num_uvframes - 1) * sizeof (dstframe_t)));
        }
		pitri = (dtriangle_t *) ((byte *) pitri + sizeof (dtriangle_t) + ((pitri->num_uvframes - 1) * sizeof (dstframe_t)));
    }
*/    
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
void Mod_LoadSpriteModel (model_t *mod, void *buffer, int flags)
{
	dsprite_t	*sprin, *sprout;
	int			i;

	sprin = (dsprite_t *)buffer;
//	sprout = (dsprite_t *)ri.X_Malloc(modfilelen, MEM_TAG_SPRITE);
	sprout = ( dsprite_t * )Hunk_Alloc (modfilelen);

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
		mod->skins[i] = R_FindImage (sprout->frames[i].name, it_sprite, (resource_t)flags);
	}

	mod->type = mod_sprite;

	// setup extra data
	mod->extradata = sprout;
	mod->extradatasize = modfilelen;
}

//=============================================================================

/*
@@@@@@@@@@@@@@@@@@@@@
BeginRegistration

Specifies the model that will be used as the world
@@@@@@@@@@@@@@@@@@@@@
*/

//extern "C" int g_loaded_old_wal;
static	int	g_startRegTime;
static	int	g_endRegTime;

void BeginRegistration (const char *model)
{
	char	fullname[MAX_QPATH];
	cvar_t	*flushmap;

	g_startRegTime = timeGetTime ();

//	g_loaded_old_wal = false;

	registration_sequence++;
	r_oldviewcluster = -1;		//	force markleafs
	g_currentTexPalette = -3;	//	no palette set

	Com_sprintf (fullname, sizeof(fullname), "maps/%s.bsp", model);

	// explicitly free the old map if different
	// this guarantees that mod_known[0] is the world map
	flushmap = ri.Cvar_Get ("flushmap", "0", CVAR_ARCHIVE);
	if ( strcmp(mod_known[0].name, fullname) || flushmap->value)
		Mod_Free (&mod_known[0]);
	r_worldmodel = Mod_ForName(fullname, true, RESOURCE_LEVEL);

	r_viewcluster = -1;

	//	clear surface sprites
	R_InitSurfaceSprites();

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
				mod->skins[i] = R_FindImage (sprout->frames[i].name, it_sprite, resource);
		}
		else if (mod->type == mod_alias)
		{
			pheader = (dmdl_t *)mod->extradata;
			for (i=0 ; i<pheader->num_skins ; i++)
				mod->skins[i] = (image_t*)RegisterSkin ((char *)pheader + pheader->ofs_skins + i*MAX_SKINNAME, resource);
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

	GL_FreeUnusedImages ();
}


//=============================================================================


/*
================
Mod_Free
================
*/
void Mod_Free (void *mod)
{
	model_t *model = (model_t*)mod;
	msurface_t 	*out;
	int count, surfnum;
	glpoly_t* pnext;

	if(model == NULL)
		return;

	// setup counters
	out = model->surfaces;
	count = model->numsurfaces;
	
	// free glpoly structs
	for (surfnum=0;surfnum<count;surfnum++,out++)
	{
		for (glpoly_t* poly = out->polys;poly;poly = pnext)
		{
			pnext = poly->next;
			ri.X_Free(poly);
			//Hunk_Free(poly);
		}
	}

	if( model == r_worldmodel )
	{
		r_worldmodel = NULL;
	}
	Hunk_Free (model->extradata);
	memset (mod, 0, sizeof(model_t));
}

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
// PurgeResources() - purge resources based on type
//---------------------------------------------------------------------------
void PurgeResources(resource_t type)
{
   model_t *model;
   image_t *image;
   short loop;

   // free images
   for (loop=0, image=gltextures; loop<numgltextures; loop++, image++)
   {
	   if (image->resource >= type && image->type!=it_procedural)
	   {
//		   image->registration_sequence = registration_sequence;
		   GL_FreeImage(image);
		   memset( &gltextures[loop], 0, sizeof( image_t ) );
	   }
   }
   
   // free models
   for (loop=0, model=mod_known; loop<mod_numknown; loop++, model++)
   {
	   if (model->resource >= type)
	   {
//		   image->registration_sequence = registration_sequence;
		   Mod_Free(model);
		   memset( &mod_known[loop], 0, sizeof( model_t ) );
	   }
   }
}
