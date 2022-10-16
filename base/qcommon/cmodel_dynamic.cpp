///////////////////////////////////////////////////////////////////////////////
//	Dynamic C-Model loading.
//
//	Our happy phrase for Sept. 18, 1998 is "Carmack code sucks lumps!" or
//	"Carmack's lump code sucks!"
///////////////////////////////////////////////////////////////////////////////

#include	"qcommon.h"
#include	"qfiles.h"
#include	"cmodel.h"
#include	"client.h"

///////////////////////////////////////////////////////////////////////////////
//	prototypes
///////////////////////////////////////////////////////////////////////////////

extern	void	Sys_Warning (char *msg, ...);
void			CM_InitBoxHull (void);
void			FloodAreaConnections (void);

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

cvar_t		*map_noareas;

csurface_t	nullSurface;
csurface_t	clipSurface;
int			c_pointcontents;
int			c_traces, c_brush_traces;

bspModel_t		bspModel;
static unsigned	last_checksum;

byte		*cmod_base;

cplane_t	*box_planes;
int			box_headnode;
cbrush_t	*box_brush;
cleaf_t		*box_leaf;

byte	pvsrow[MAX_MAP_LEAFS/8];
byte	phsrow[MAX_MAP_LEAFS/8];

userEpair_t	*map_epairs;

//	debugging
byte	*visTest;

// pointer to bsp hunk
byte*	hunk_ptr;

/*
===============================================================================
					MAP LOADING
===============================================================================
*/

///////////////////////////////////////////////////////////////////////////////
//	CM_EpairsForEdict
//
//	returns a pointer to all epairs for an entity
///////////////////////////////////////////////////////////////////////////////

#pragma warning(disable: 4090)
#define MAX_EPAIRS	32
int	CM_EpairsForEdict (const char **data, userEpair_t **epair)
{
	userEpair_t		tpair [MAX_EPAIRS];
	userEpair_t		*return_epair;
	int				tpair_count;
	char			keyname [MAX_TOKEN_CHARS];
	char			*com_token;
	int				n, i;

	tpair_count = 0;

	// go through all the dictionary pairs
	while (1)
	{	
		///////////////////////////////////////////////////////////////////////
		//	get the key
		///////////////////////////////////////////////////////////////////////
		com_token = COM_Parse ((char **)data);

		// see if we are done
		if (com_token[0] == '}') break;

		if (*data == NULL)
			Sys_Error ("CM_EpairsForEdict: EOF without closing brace");
		
		if( tpair_count >= MAX_EPAIRS )
		{
			Sys_Error("CM_EpairsForEdict: more than 32 epairs found.");
		}

		strcpy (keyname, com_token);

		//	remove any trailing spaces from keyname
		n = strlen (keyname);
		while (n && keyname[n-1] == ' ')
		{
			keyname[n-1] = 0;
			n--;
		}

		///////////////////////////////////////////////////////////////////////
		//	get the key
		///////////////////////////////////////////////////////////////////////
		com_token = COM_Parse ((char **)data);

		if (*data == NULL)
			Sys_Error ("CM_EpairsForEdict: EOF without closing brace");
		
		if (tpair_count >= 32)
			Sys_Error("CM_EpairsForEdict: more than 32 epairs found.");

		//	don't add keynames with a leading underscore
		if (keyname[0] == '_')
			continue;

//		//	strdup -- use CM_FreeEpairs to free up this memory!!
		tpair[tpair_count].key	  = strdup (keyname);
		if( tpair [tpair_count].key == NULL )
			continue;
		tpair[tpair_count].value = strdup (com_token);
		if( tpair [tpair_count].value == NULL )
			continue;
//		tpair [tpair_count].key	  = CopyString(keyname);
//		tpair [tpair_count].value = CopyString(com_token);
		
		tpair_count++;
	}

	//	set up epair to return
//	*epair = (userEpair_t *)X_Malloc((tpair_count + 1) * sizeof (userEpair_t), MEM_TAG_EPAIR);
	*epair = (userEpair_t *)malloc((tpair_count + 1) * sizeof (userEpair_t));
//	*epair = (userEpair_t *)Z_TagMalloc((tpair_count + 1) * sizeof (userEpair_t), TAG_LEVEL);
	return_epair = *epair;

	//	copy over pointers
	for (i = 0; i < tpair_count; i++)
	{
		return_epair[i].key   = tpair[i].key;
		return_epair[i].value = tpair[i].value;
	}

	// make sure end ends in NULL
	return_epair[tpair_count].key   = NULL;
	return_epair[tpair_count].value = NULL;

	return	tpair_count;
}
#pragma warning(default: 4090)

///////////////////////////////////////////////////////////////////////////////
//	CM_FreeEpairs
//
//	frees memory for userEpair_t list
///////////////////////////////////////////////////////////////////////////////

void	CM_FreeEpairs (userEpair_t *epair)
{
	int		i;

	for (i = 0; epair [i].key != NULL; i++)
	{
		// free mem alloced by strdup
		free(epair [i].key);
//		Z_Free(epair [i].key);
		free(epair [i].value);
//		Z_Free(epair [i].value);
	}

	free(epair);
//	Z_Free (epair);
}

///////////////////////////////////////////////////////////////////////////////
//	CM_KeyValue
//
///////////////////////////////////////////////////////////////////////////////

char	*CM_KeyValue (userEpair_t *epair, char *key)
{
	if (!epair)
		return NULL;

	int		i;

	for (i = 0; epair [i].key != NULL; i++)
	{
		if (!stricmp (epair [i].key, key))
			return	epair [i].value;
	}

	return	NULL;
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

#pragma warning(disable: 4090)
userEpair_t	*CM_EpairsForClass (byte *buffer, char *className)
{
	char			*com_token;
	userEpair_t		*epairs;
	int				num_epairs;

	while (1)
	{
		//	start with the first entity
		com_token = COM_Parse ((char **)&buffer);

		//	first character in an entity record should be an open bracket
		if (com_token[0] != '{')
			Com_Error (ERR_DROP, "CM_EpairsForClass: expected '{'\n");

		num_epairs = CM_EpairsForEdict ((const char **)&buffer, &epairs);

		if (!stricmp (CM_KeyValue (epairs, "classname"), className))
			break;
		else
			CM_FreeEpairs (epairs);

		if (!buffer)
			//	no more buffer
			break;
	}

	//	return the number of epairs found
	return	epairs;
}
#pragma warning(default: 4090)

///////////////////////////////////////////////////////////////////////////////
//	end epair functions
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	CMod_LoadPlanes
//
///////////////////////////////////////////////////////////////////////////////

int	CMod_LoadPlanes (lump_t *l)
{
	int			i;
	cplane_t	*out;
	dplane_t 	*in;
	int			count;
	int			bits;
	int			size;
	
	in = (dplane_t *)(cmod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Com_Error (ERR_DROP, "CMod_LoadPlanes: funny lump size");

	count = l->filelen / sizeof(*in);

	if (count < 1)
		Com_Error (ERR_DROP, "Map with no planes");

	// need to save space for box planes
	if (count > MAX_MAP_PLANES)
		Com_Error (ERR_DROP, "Map has too many planes");

	size = sizeof (cplane_t) * (count + CMOD_EXTRA_DATA_SIZE);
	//bspModel.planes = (cplane_t*)X_Malloc(size,MEM_TAG_BSP);

//	bspModel.planes = (cplane_t*)hunk_ptr;
//	hunk_ptr += size;

	bspModel.planes = (cplane_t *)Hunk_Alloc (size);
	
	out = &bspModel.planes [0];
	bspModel.numPlanes = count;

	for ( i=0 ; i < count ; i++, in++)
	{
		bits = 0;

		out[i].normal.x = LittleFloat(in->normal.x);
		if (out[i].normal.x < 0)
			bits |= 1<<0;
		out[i].normal.y = LittleFloat(in->normal.y);
		if (out[i].normal.y < 0)
			bits |= 1<<1;
		out[i].normal.z = LittleFloat(in->normal.z);
		if (out[i].normal.z < 0)
			bits |= 1<<2;

		out[i].dist = LittleFloat (in->dist);
		out[i].type = LittleLong (in->type);
		out[i].signbits = bits;
		out[i].planeIndex = i;
	}

	return	size;
}

///////////////////////////////////////////////////////////////////////////////
//	CMod_LoadVisibility
///////////////////////////////////////////////////////////////////////////////

int	CMod_LoadVisibility (lump_t *l)
{
	int		i;

	bspModel.numVisibility = l->filelen;
	if (!l->filelen)
	{
		// this map contains no vis data...
        Sys_Warning ("WARNING: This map contains no VIS data!");
		bspModel.visibility = NULL;
		bspModel.dVis = NULL;
		return	0;
	}

	if (l->filelen > MAX_MAP_VISIBILITY)
		Com_Error (ERR_DROP, "Map has too large visibility lump");

//	bspModel.visibility = hunk_ptr;
//	hunk_ptr += bspModel.numVisibility;
	bspModel.visibility = (byte *)Hunk_Alloc (bspModel.numVisibility);
	bspModel.dVis = (dvis_t *) bspModel.visibility;
	
	visTest = bspModel.visibility;
	visTest += 1;

	memcpy (bspModel.visibility, cmod_base + l->fileofs, l->filelen);

	bspModel.dVis->numclusters = LittleLong (bspModel.dVis->numclusters);
	for (i=0 ; i < bspModel.dVis->numclusters ; i++)
	{
		bspModel.dVis->bitofs[i][0] = LittleLong (bspModel.dVis->bitofs[i][0]);
		bspModel.dVis->bitofs[i][1] = LittleLong (bspModel.dVis->bitofs[i][1]);
	}

	return	l->filelen;
}

///////////////////////////////////////////////////////////////////////////////
//	CMod_LoadLeafs
//
///////////////////////////////////////////////////////////////////////////////

int	CMod_LoadLeafs (lump_t *l)
{
	int			i;
	cleaf_t		*out;
	dleaf_t 	*in;
	int			count, size;
	
	in = (dleaf_t *)(cmod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		Com_Error (ERR_DROP, "CMod_LoadLeafs: funny lump size");

	count = l->filelen / sizeof(*in);
	if (count < 1)
		Com_Error (ERR_DROP, "Map with no leafs");

	// need to save space for box planes
	if (count > MAX_MAP_PLANES)
		Com_Error (ERR_DROP, "Map has too many planes");

	size = (count + CMOD_EXTRA_DATA_SIZE) * sizeof (cleaf_t);
//	bspModel.leafs = (cleaf_t*)hunk_ptr;
//	hunk_ptr += size;
	bspModel.leafs = (cleaf_t *)Hunk_Alloc (size);

	out = bspModel.leafs;
	bspModel.numLeafs = count;
	bspModel.numClusters = 0;

	for (i = 0; i < count; i++, in++)
	{
		out [i].contents = LittleLong (in->contents);
		out [i].cluster = LittleShort (in->cluster);
		out [i].area = LittleShort (in->area);
		out [i].firstleafbrush = LittleShort (in->firstleafbrush);
		out [i].numleafbrushes = LittleShort (in->numleafbrushes);
		if (out [i].cluster >= bspModel.numClusters)
			bspModel.numClusters = out [i].cluster + 1;
	}

	if (bspModel.leafs [0].contents != CONTENTS_SOLID)
		Com_Error (ERR_DROP, "Map leaf 0 is not CONTENTS_SOLID");

	bspModel.solidLeaf = 0;
	bspModel.emptyLeaf = -1;

	for (i = 1 ; i < bspModel.numLeafs ; i++)
	{
		if (!bspModel.leafs [i].contents)
		{
			bspModel.emptyLeaf = i;
			break;
		}
	}

	if (bspModel.emptyLeaf == -1)
		Com_Error (ERR_DROP, "Map does not have an empty leaf");

	return	size;
}

///////////////////////////////////////////////////////////////////////////////
//	CMod_LoadNodes
//
///////////////////////////////////////////////////////////////////////////////

int	CMod_LoadNodes (lump_t *l)
{
	dnode_t		*in;
	int			child;
	cnode_t		*out;
	int			i, j, count, size;
	
	in = (dnode_t *)(cmod_base + l->fileofs);

	if (l->filelen % sizeof (*in))
		Com_Error (ERR_DROP, "CMod_LoadNodes: funny lump size");

	count = l->filelen / sizeof(*in);

	size = (count + CMOD_EXTRA_DATA_SIZE) * sizeof (cnode_t);
//	bspModel.nodes = (cnode_t*)hunk_ptr;
//	hunk_ptr += size;
	bspModel.nodes = (cnode_t *)Hunk_Alloc (size);

	if (count < 1)
		Com_Error (ERR_DROP, "Map has no nodes");
	if (count > MAX_MAP_NODES)
		Com_Error (ERR_DROP, "Map has too many nodes");

	out = bspModel.nodes;
	bspModel.numNodes = count;

	for (i=0 ; i<count ; i++, in++)
	{
		out [i].plane = bspModel.planes + LittleLong(in->planenum);

		for (j=0 ; j<2 ; j++)
		{
			child = LittleLong (in->children[j]);
			out [i].children[j] = child;
		}
	}

	return	size;
}

///////////////////////////////////////////////////////////////////////////////
//	CMod_LoadBrushSides
//
///////////////////////////////////////////////////////////////////////////////

int	CMod_LoadBrushSides (lump_t *l)
{
	int			i, j;
	cbrushside_t	*out;
	dbrushside_t 	*in;
	int			count;
	int			num, size;

	in = (dbrushside_t *)(cmod_base + l->fileofs);
	
	if (l->filelen % sizeof (*in))
		Com_Error (ERR_DROP, "CMod_LoadBrushSides: funny lump size");
	
	count = l->filelen / sizeof(*in);
	
	// need to save space for box planes
	if (count > MAX_MAP_BRUSHSIDES)
		Com_Error (ERR_DROP, "Map has too many planes");
	
	size = (count + CMOD_EXTRA_DATA_SIZE) * sizeof (cbrushside_t);
//	bspModel.brushSides = (cbrushside_t*)hunk_ptr;
//	hunk_ptr += size;
	bspModel.brushSides = (cbrushside_t *)Hunk_Alloc (size);

	out = bspModel.brushSides;	
	bspModel.numBrushSides = count;
	
	for (i = 0; i < count; i++, in++)
	{
		num = LittleShort (in->planenum);
	
		out [i].plane = bspModel.planes + num;
		j = LittleShort (in->texinfo);

		if (j >= bspModel.numSurfaces)
			Com_Error (ERR_DROP, "Bad brushside texinfo");

		if (j >= 0)
			out [i].surface = &bspModel.surfaces [j];
		else
			//	fix for Quake 2 fuckup: clip brushes have -1 for texinfo
			//	which was pointing to data outside of surface array
			//	dynamic allocation caught this bug as a memory
			out [i].surface = &clipSurface;
	}

	return	size;
}

int CMod_LoadExtendedSurfInfo (lump_t *l)
{
	float		*in;
	csurface_t	*out;
	int			i, count, size;
	int			r, g, b;

	in = (float *)(cmod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		Com_Error (ERR_DROP, "MOD_LoadExtendedSurfInfo: funny lump size");

	count = l->filelen / sizeof(*in);
	size = count * sizeof (float);

	out = bspModel.surfaces;

//	for (i = 0; i < count; i++, out++)
	for (i = 0; i < bspModel.numSurfaces; i++, out++)
	{
		r = LittleFloat (*in);
		in++;
		g = LittleFloat (*in);
		in++;
		b = LittleFloat (*in);
		in++;

		out->color = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b & 0xf8) >> 3);
	}

	return size;
}
///////////////////////////////////////////////////////////////////////////////
//	CMod_LoadSurfaces
//
///////////////////////////////////////////////////////////////////////////////

int	CMod_LoadSurfaces (lump_t *l)
{
	texinfo_t	*in;
	csurface_t	*out;
	int			i, count, size;

	in = (texinfo_t *)(cmod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		Com_Error (ERR_DROP, "CMod_LoadSurfaces: funny lump size");

	count = l->filelen / sizeof(*in);

	if (count < 1)
		Com_Error (ERR_DROP, "Map with no surfaces");
	if (count > MAX_MAP_TEXINFO)
		Com_Error (ERR_DROP, "Map has too many surfaces");

	size = count * sizeof (csurface_t);
//	bspModel.surfaces = (csurface_t*)hunk_ptr;
//	hunk_ptr += size;
	bspModel.surfaces = (csurface_t *)Hunk_Alloc (size);

	bspModel.numSurfaces = count;
	out = bspModel.surfaces;

	for ( i=0 ; i<count ; i++, in++)
	{
		strncpy (out [i].name, in->texture, sizeof(out [i].name) - 1);
		out[i].flags = LittleLong (in->flags);
		out[i].value = LittleLong (in->value);
		out[i].index = i;
	}

	return	size;
}

///////////////////////////////////////////////////////////////////////////////
//	CMod_LoadLeafBrushes
//
///////////////////////////////////////////////////////////////////////////////

int	CMod_LoadLeafBrushes (lump_t *l)
{
	int			i;
	unsigned short	*out;
	unsigned short 	*in;
	int			count, size;
	
	in = (unsigned short *)(cmod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		Com_Error (ERR_DROP, "CMod_LoadLeafBrushes: funny lump size");

	count = l->filelen / sizeof(*in);

	if (count < 1)
		Com_Error (ERR_DROP, "Map with no planes");

	// need to save space for box planes
	if (count > MAX_MAP_LEAFBRUSHES)
		Com_Error (ERR_DROP, "Map has too many leafbrushes");

	size = (count + CMOD_EXTRA_DATA_SIZE) * sizeof (unsigned short);
//	bspModel.leafBrushes = (unsigned short*)hunk_ptr;
//	hunk_ptr += size;
	bspModel.leafBrushes = (unsigned short *)Hunk_Alloc (size);

	out = bspModel.leafBrushes;
	bspModel.numLeafBrushes = count;
	
	for ( i=0 ; i<count ; i++, in++, out++)
		*out = LittleShort (*in);

	return	size;
}

///////////////////////////////////////////////////////////////////////////////
//	CMod_LoadSubmodels
//
///////////////////////////////////////////////////////////////////////////////

int	CMod_LoadSubmodels (lump_t *l)
{
	dmodel_t	*in;
	cmodel_t	*out;
	int			i, count, size;

	in = (dmodel_t *)(cmod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		Com_Error (ERR_DROP, "CMod_LoadSubmodels: funny lump size");

	count = l->filelen / sizeof(*in);

	if (count < 1)
		Com_Error (ERR_DROP, "Map with no models");
	if (count > MAX_MAP_MODELS)
		Com_Error (ERR_DROP, "Map has too many models");

	size = count * sizeof (cmodel_t);
	bspModel.subModels = (cmodel_t *)Hunk_Alloc (size);
//	bspModel.subModels = (cmodel_t*)hunk_ptr;
//	hunk_ptr += size;
	bspModel.numSubModels = count;

	out = bspModel.subModels;

	for (i = 0; i < count; i++, in++, out++)
	{
		out = &bspModel.subModels [i];

		out->mins.x = LittleFloat(in->mins.x) - 1;
		out->mins.y = LittleFloat(in->mins.y) - 1;
		out->mins.z = LittleFloat(in->mins.z) - 1;

		out->maxs.x = LittleFloat(in->maxs.x) + 1;
		out->maxs.y = LittleFloat(in->maxs.y) + 1;
		out->maxs.z = LittleFloat(in->maxs.z) + 1;

		out->origin.x = LittleFloat(in->origin.x);
		out->origin.y = LittleFloat(in->origin.y);
		out->origin.z = LittleFloat(in->origin.z);

		out->headnode = LittleLong (in->headnode);
	}

	return	size;
}

///////////////////////////////////////////////////////////////////////////////
//	CMod_LoadBrushes
//
///////////////////////////////////////////////////////////////////////////////

int	CMod_LoadBrushes (lump_t *l)
{
	dbrush_t	*in;
	cbrush_t	*out;
	int			i, count, size;
	
	in = (dbrush_t *)(cmod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		Com_Error (ERR_DROP, "CMod_LoadBrushes: funny lump size");

	count = l->filelen / sizeof(*in);

	if (count > MAX_MAP_BRUSHES)
		Com_Error (ERR_DROP, "Map has too many brushes");

	size = sizeof (cbrush_t) * (count + CMOD_EXTRA_DATA_SIZE);
//	bspModel.brushes = (cbrush_t*)hunk_ptr;
//	hunk_ptr += size;
	bspModel.brushes = (cbrush_t *)Hunk_Alloc (size);

	out = bspModel.brushes;
	bspModel.numBrushes = count;

	for (i=0 ; i<count ; i++, out++, in++)
	{
		out->firstbrushside = LittleLong(in->firstside);
		out->numsides = LittleLong(in->numsides);
		out->contents = LittleLong(in->contents);
	}

	return	size;
}

///////////////////////////////////////////////////////////////////////////////
//	CMod_LoadEntityString
//
///////////////////////////////////////////////////////////////////////////////

int	CMod_LoadEntityString (lump_t *l)
{
	bspModel.numEntityChars = l->filelen;

	if (l->filelen > MAX_MAP_ENTSTRING)
		Com_Error (ERR_DROP, "Map has too large entity lump");

//	bspModel.entityChars = (char*)hunk_ptr;
//	hunk_ptr += l->filelen + 1;
	bspModel.entityChars = (char *)Hunk_Alloc (l->filelen + 1);

	memcpy (bspModel.entityChars, cmod_base + l->fileofs, l->filelen);

	return	l->filelen + 1;
}

///////////////////////////////////////////////////////////////////////////////
//	CMod_LoadAreas
//
///////////////////////////////////////////////////////////////////////////////

int	CMod_LoadAreas (lump_t *l)
{
	int			i;
	carea_t		*out;
	darea_t 	*in;
	int			count, size;

	in = (darea_t *)(cmod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		Com_Error (ERR_DROP, "CMod_LoadAreas: funny lump size");

	count = l->filelen / sizeof(*in);

	if (count > MAX_MAP_AREAS)
		Com_Error (ERR_DROP, "Map has too many areas");

	size = count * sizeof (carea_t);
//	bspModel.areas = (carea_t*)hunk_ptr;
//	hunk_ptr += size;
	bspModel.areas = (carea_t *)Hunk_Alloc (size);

	out = bspModel.areas;
	bspModel.numAreas = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		out->numareaportals = LittleLong (in->numareaportals);
		out->firstareaportal = LittleLong (in->firstareaportal);
		out->floodvalid = 0;
		out->floodnum = 0;
	}

	return	size;
}

///////////////////////////////////////////////////////////////////////////////
//	CMod_LoadAreaPortals
//
///////////////////////////////////////////////////////////////////////////////

int	CMod_LoadAreaPortals (lump_t *l)
{
	int				i;
	dareaportal_t	*out;
	dareaportal_t 	*in;
	int				count, size;

	in = (dareaportal_t *)(cmod_base + l->fileofs);

	if (l->filelen % sizeof(*in))
		Com_Error (ERR_DROP, "CMod_LoadAreaPortals: funny lump size");

	count = l->filelen / sizeof(*in);

	if (count > MAX_MAP_AREAS)
		Com_Error (ERR_DROP, "Map has too many areas");

	size = count * sizeof (dareaportal_t);

//	bspModel.areaPortals = (dareaportal_t*)hunk_ptr;
//	hunk_ptr += size;
//	bspModel.portalOpen = (int*)hunk_ptr;
//	hunk_ptr += (count*sizeof(qboolean));

	bspModel.areaPortals = (dareaportal_t *)Hunk_Alloc (size);
	bspModel.portalOpen = (int *)Hunk_Alloc (count * sizeof (qboolean));

	out = bspModel.areaPortals;
	bspModel.numAreaPortals = count;

	for (i = 0; i < count; i++, in++, out++)
	{
		out->portalnum = LittleLong (in->portalnum);
		out->otherarea = LittleLong (in->otherarea);
	}

	return	size;
}

///////////////////////////////////////////////////////////////////////////////
//	CM_LoadDemoMap
//
//	Load up some of the structures needed to run a demo map
///////////////////////////////////////////////////////////////////////////////

cmodel_t	*CM_LoadDemoMap (char *name, unsigned *checksum)
{
	char		fName [_MAX_FNAME];
	char		mapName [_MAX_PATH];

	_splitpath (name, NULL, NULL, fName, NULL);
// SCG[1/16/00]: 	sprintf (mapName, "maps\\%s.bsp", fName);
	Com_sprintf (mapName,sizeof(mapName), "maps\\%s.bsp", fName);

	Hunk_Free (bspModel.hunkBase);

	memset (&bspModel, 0x00, sizeof (bspModel_t));

	//bspModel.hunkBase = Hunk_Begin (0x03e8000);	//	4 megs max bsp size
	bspModel.hunkBase = Hunk_Begin (0x500000);	//	5 megs max bsp size

/*
	size = CMod_LoadSubmodels (&header.lumps[LUMP_MODELS]);
	totalMapMemory += size;
	Com_Printf ("  %i submodels, bytes = %i.\n", bspModel.numSubModels, size);

	size = CMod_LoadAreaPortals (&header.lumps[LUMP_AREAPORTALS]);
	totalMapMemory += size;
	Com_Printf ("  %i area portals, bytes = %i.\n", bspModel.numAreaPortals, size);

	size = CMod_LoadEntityString (&header.lumps[LUMP_ENTITIES]);
	totalMapMemory += size;
	Com_Printf ("  %i entity bytes, bytes = %i.\n", bspModel.numEntityChars, size);
*/
//	bspModel.subModels = (cmodel_t *)X_Malloc(sizeof (cmodel_t), MEM_TAG_BSP);
	// SCG[11/12/99]: Remove mike montague memory manager
	bspModel.subModels = ( cmodel_t * )Hunk_Alloc( sizeof ( cmodel_t ) );
	bspModel.numSubModels = 1;

	// SCG[11/12/99]: Remove mike montague memory manager
//	bspModel.portalOpen = (int *)X_Malloc(sizeof (qboolean) * MAX_MAP_AREAPORTALS, MEM_TAG_BSP);
	bspModel.portalOpen = (int *)Hunk_Alloc( sizeof( qboolean ) * MAX_MAP_AREAPORTALS );

	Hunk_End ();

//	Com_Printf ("Total memory size for map = %i bytes.\n", totalMapMemory);

	memset (bspModel.portalOpen, 0, sizeof(bspModel.portalOpen));
	FloodAreaConnections ();

// SCG[1/16/00]: 	sprintf (bspModel.mapName, "maps\\%s.dm2", fName);
//	Com_sprintf (bspModel.mapName,sizeof(bspModel.mapName), "maps\\%s.dem", fName);

	return	bspModel.subModels;
}

///////////////////////////////////////////////////////////////////////////////
//	CM_LoadMap
//
//	Loads in the map and all submodels
//
//	if clientload == true, then the map is only loaded if it has not been
//	loaded once
//
//	the server calls this function first, then the client will call it
//	from CL_Precache_f.
///////////////////////////////////////////////////////////////////////////////

cmodel_t *CM_LoadMap (char *name, qboolean clientload, unsigned *checksum)
{
	unsigned		*buf;
	int				i, size, totalMapMemory = 0;
	dheader_t		header;
	int				length;
	char			fExt [_MAX_EXT];

	_splitpath (name, NULL, NULL, NULL, fExt);

	map_noareas = Cvar_Get ("map_noareas", "0", 0);

	if( !stricmp( fExt, ".dem" ) )
	{
		CM_LoadDemoMap (name, checksum);

		return bspModel.subModels;		// still have the right version
	}

	if( !strcmp( bspModel.mapName, name ) && (clientload || !Cvar_VariableValue ("flushmap")) )
	{
		*checksum = last_checksum;
		
		if (!clientload)
		{
			memset (bspModel.portalOpen, 0, sizeof (qboolean) * bspModel.numAreaPortals);
			FloodAreaConnections ();				
		}

		return	bspModel.subModels;		// still have the right version
	}

	// free old stuff
	if( bspModel.hunkBase )
	{
		Hunk_Free (bspModel.hunkBase);
		bspModel.hunkBase = NULL;
	}

	if( map_epairs != NULL )
	{
		CM_FreeEpairs (map_epairs);
		map_epairs = NULL;
	}

	memset (&bspModel, 0x00, sizeof (bspModel_t));

	// SCG[10/5/99]: We just did a memset, why the fuck do we want to do this?!?!?
/*
	bspModel.numPlanes = 0;
	bspModel.numVisibility = 0;
	bspModel.numLeafs = 0;
	bspModel.numClusters = 0;
	bspModel.numNodes = 0;
	bspModel.numSubModels = 0;
	bspModel.numEntityChars = 0;
	bspModel.entityChars = NULL;
	bspModel.mapName [0] = 0;
*/
	bspModel.fixModelClip = true;
	
	if (!name || !name[0])
	{
		bspModel.numLeafs = 1;
		bspModel.numClusters = 1;
		bspModel.numAreas = 1;
		*checksum = 0;
		return bspModel.subModels;			// cinematic servers won't have anything at all
	}

	//
	// load the file
	//
	length = FS_LoadFile (name, (void **)&buf);
	if (!buf)
		Com_Error (ERR_DROP, "Couldn't load %s", name);

	last_checksum = LittleLong (Com_BlockChecksum (buf, length));
	*checksum = last_checksum;
	header = *(dheader_t *)buf;
	for (i=0 ; i<sizeof(dheader_t)/4 ; i++)
		((int *)&header)[i] = LittleLong ( ((int *)&header)[i]);

	if (header.version != BSPVERSION )
		Com_Error (ERR_DROP, "CMod_LoadBrushModel: %s has wrong version number (%i should be %i)"
		, name, header.version, BSPVERSION);

	// calculate the size of the hunk
/*
	size = (header.lumps[LUMP_TEXINFO].filelen/sizeof(texinfo_t))*sizeof(csurface_t) + 
		((header.lumps[LUMP_LEAFS].filelen/sizeof(dleaf_t))+CMOD_EXTRA_DATA_SIZE)*sizeof(cleaf_t) +
		((header.lumps[LUMP_LEAFBRUSHES].filelen/sizeof(unsigned short))+CMOD_EXTRA_DATA_SIZE)*sizeof(unsigned short) +
		((header.lumps[LUMP_PLANES].filelen/sizeof(dplane_t))+CMOD_EXTRA_DATA_SIZE)*sizeof(cplane_t) + 
		((header.lumps[LUMP_BRUSHES].filelen/sizeof(dbrush_t))+CMOD_EXTRA_DATA_SIZE)*sizeof(cbrush_t) +
		((header.lumps[LUMP_BRUSHSIDES].filelen/sizeof(dbrushside_t))+CMOD_EXTRA_DATA_SIZE)*sizeof(cbrushside_t) +
		(header.lumps[LUMP_MODELS].filelen/sizeof(dmodel_t))*sizeof(cmodel_t) +
		((header.lumps[LUMP_NODES].filelen/sizeof(dnode_t))+CMOD_EXTRA_DATA_SIZE)*sizeof(cnode_t) + 
		(header.lumps[LUMP_AREAS].filelen/sizeof(darea_t))*sizeof(carea_t) +
		(header.lumps[LUMP_AREAPORTALS].filelen/sizeof(dareaportal_t))*sizeof(qboolean) + header.lumps[LUMP_AREAPORTALS].filelen +
		(header.lumps[LUMP_VISIBILITY].filelen) + 
		(header.lumps[LUMP_ENTITIES].filelen+1);

	// allocate memory for bsp
	bspModel.hunkBase = hunk_ptr = (byte*)X_Malloc(size,MEM_TAG_BSP);
*/
	bspModel.hunkBase = Hunk_Begin (0x500000);	//	4 megs max bsp size

	if( bspModel.hunkBase == NULL )
	{
		Com_Error (ERR_DROP, "Couldn't allocate %d bytes for map", 0x500000);
	}

	cmod_base = (byte *)buf;

	// load into heap
	size = CMod_LoadSurfaces (&header.lumps[LUMP_TEXINFO]);
	totalMapMemory += size;
	Com_Printf ("  %i surfaces, bytes = %i.\n", bspModel.numSurfaces, size);

	size = CMod_LoadExtendedSurfInfo( &header.lumps[LUMP_EXTSURFINFO] );
	totalMapMemory += size;

	size = CMod_LoadLeafs (&header.lumps[LUMP_LEAFS]);
	totalMapMemory += size;
	Com_Printf ("  %i leafs, bytes = %i.\n", bspModel.numLeafs, size);

	size = CMod_LoadLeafBrushes (&header.lumps[LUMP_LEAFBRUSHES]);
	totalMapMemory += size;
	Com_Printf ("  %i leaf brushes, bytes = %i.\n", bspModel.numLeafBrushes, size);

	size = CMod_LoadPlanes (&header.lumps[LUMP_PLANES]);
	totalMapMemory += size;
	Com_Printf ("  %i planes, bytes = %i.\n", bspModel.numPlanes, size);

	size = CMod_LoadBrushes (&header.lumps[LUMP_BRUSHES]);
	totalMapMemory += size;
	Com_Printf ("  %i brushes, bytes = %i.\n", bspModel.numBrushes, size);

	size = CMod_LoadBrushSides (&header.lumps[LUMP_BRUSHSIDES]);
	totalMapMemory += size;
	Com_Printf ("  %i brush sides, bytes = %i.\n", bspModel.numBrushSides, size);
	
	size = CMod_LoadSubmodels (&header.lumps[LUMP_MODELS]);
	totalMapMemory += size;
	Com_Printf ("  %i submodels, bytes = %i.\n", bspModel.numSubModels, size);

	size = CMod_LoadNodes (&header.lumps[LUMP_NODES]);
	totalMapMemory += size;
	Com_Printf ("  %i nodes, bytes = %i.\n", bspModel.numNodes, size);

	size = CMod_LoadAreas (&header.lumps[LUMP_AREAS]);
	totalMapMemory += size;
	Com_Printf ("  %i areas, bytes = %i.\n", bspModel.numAreas, size);

	size = CMod_LoadAreaPortals (&header.lumps[LUMP_AREAPORTALS]);
	totalMapMemory += size;
	Com_Printf ("  %i area portals, bytes = %i.\n", bspModel.numAreaPortals, size);

	size = CMod_LoadVisibility (&header.lumps[LUMP_VISIBILITY]);
	totalMapMemory += size;
	Com_Printf ("  %i visibility, bytes = %i.\n", bspModel.numVisibility, size);

	size = CMod_LoadEntityString (&header.lumps[LUMP_ENTITIES]);
	totalMapMemory += size;
	Com_Printf ("  %i entity bytes, bytes = %i.\n", bspModel.numEntityChars, size);

//	hunk_ptr = NULL;
	Hunk_End ();

	if (map_epairs != NULL)
		CM_FreeEpairs (map_epairs);
	map_epairs = CM_EpairsForClass ((byte *)bspModel.entityChars, "worldspawn");

	Com_Printf ("Total memory size for map = %i bytes.\n", totalMapMemory);

	FS_FreeFile (buf);

	CM_InitBoxHull ();

	memset (bspModel.portalOpen, 0, sizeof(bspModel.portalOpen));
	FloodAreaConnections ();

	strcpy (bspModel.mapName, name);

	return bspModel.subModels;
}

///////////////////////////////////////////////////////////////////////////////
//	CM_GetPaletteDirectory
//
///////////////////////////////////////////////////////////////////////////////

#pragma warning(disable: 4090)
void CM_GetPaletteDirectory (char *name, char *palette)
{
	FILE			*f;
	unsigned		*buf;
	byte			*buf2;
	int				i;
	dheader_t		header;
	int				length;
	char			*com_token;

	buf = (unsigned int *)X_Malloc(sizeof (dheader_t), MEM_TAG_FILE);
	if( buf == NULL )
	{
		return;
	}

	// open the file
	length = FS_FOpenFile (name, &f);
	
	if( length == 0 )
	{
		return;
	}

	// read in the header information
	FS_Read (buf, sizeof(dheader_t), f);

	// swap header
	header = *(dheader_t *)buf;
	for (i=0 ; i<sizeof(dheader_t)/4 ; i++)
		((int *)&header)[i] = LittleLong ( ((int *)&header)[i]);

	// make sure it's the current version
	if (header.version != BSPVERSION )
		Com_Error (ERR_DROP, "CM_GetPaletteDirectory: %s has wrong version number (%i should be %i)"
		, name, header.version, BSPVERSION);

	// go to the entity lump
	FS_Seek (f, header.lumps[LUMP_ENTITIES].fileofs, SEEK_SET);

	buf2 = (byte *)X_Malloc(header.lumps[LUMP_ENTITIES].filelen, MEM_TAG_FILE);
	
	// read the entity info
	FS_Read (buf2, header.lumps[LUMP_ENTITIES].filelen, f);

	//numentitychars = header.lumps[LUMP_ENTITIES].filelen;
	if (header.lumps[LUMP_ENTITIES].filelen > MAX_MAP_ENTSTRING)
		Com_Error (ERR_DROP, "Map has too large entity lump");

	com_token = COM_Parse ((char **)&buf2);
	if (com_token[0] != '{')
		Com_Error (ERR_DROP, "CM_GetPaletteDirectory: expected '{'\n");

	strcpy (palette, name + 5);
	palette[strlen(palette)-4] = 0;

	while (1)
	{
		com_token = COM_Parse ((char **)&buf2);
		if (!buf2)
			break;
		if (com_token[0] == '}')
			break;

		if (!strcmp (com_token, "palette"))
		{
			com_token = COM_Parse ((char **)&buf2);
			strcpy (palette, com_token);
		}
	}

	FS_FCloseFile (f);

	X_Free (buf);
//	Z_Free (buf);
}
#pragma warning(default: 4090)

/*
==================
CM_InlineModel
==================
*/
cmodel_t	*CM_InlineModel (char *name)
{
	int		num;
	
	if (!name || name[0] != '*')
		Com_Error (ERR_DROP, "CM_InlineModel: bad name");
	
	num = atoi (name+1);
	
	if (num < 1 || num >= bspModel.numSubModels)
		Com_Error (ERR_DROP, "CM_InlineModel: bad number, %i\n", num);
	
	return	&bspModel.subModels [num];
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int		CM_NumClusters (void)
{
	return bspModel.numClusters;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int		CM_NumInlineModels (void)
{
	return bspModel.numSubModels;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

char	*CM_EntityString (void)
{
	return	bspModel.entityChars;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int		CM_LeafContents (int leafnum)
{
	if (leafnum < 0 || leafnum >= bspModel.numLeafs)
		Com_Error (ERR_DROP, "CM_LeafContents: bad number");
	
	return	bspModel.leafs [leafnum].contents;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int		CM_LeafCluster (int leafnum)
{
	if (leafnum < 0 || leafnum >= bspModel.numLeafs)
		Com_Error (ERR_DROP, "CM_LeafCluster: bad number");
	
	return bspModel.leafs [leafnum].cluster;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int		CM_LeafArea (int leafnum)
{
	if (leafnum < 0 || leafnum >= bspModel.numLeafs)
		Com_Error (ERR_DROP, "CM_LeafArea: bad number");
	
	return bspModel.leafs [leafnum].area;
}

//=======================================================================

/*
===================
CM_InitBoxHull
Set up the planes and nodes so that the six floats of a bounding box
can just be stored out and get a proper clipping hull structure.
===================
*/
void CM_InitBoxHull (void)
{
	int			i;
	int			side;
	cnode_t		*c;
	cplane_t	*p;
	cbrushside_t	*s;
	box_headnode = bspModel.numNodes;
//	box_planes = &map_planes[numplanes];
	box_planes = bspModel.planes + bspModel.numPlanes;

	if (bspModel.numNodes + 6 > MAX_MAP_NODES
		|| bspModel.numBrushes + 1 > MAX_MAP_BRUSHES
		|| bspModel.numLeafBrushes + 1 > MAX_MAP_LEAFBRUSHES
		|| bspModel.numBrushSides + 6 > MAX_MAP_BRUSHSIDES
		|| bspModel.numPlanes + 12 > MAX_MAP_PLANES)
		Com_Error (ERR_DROP, "Not enough room for box tree");

	box_brush = &bspModel.brushes [bspModel.numBrushes];
	box_brush->numsides = 6;
	box_brush->firstbrushside = bspModel.numBrushSides;
	box_brush->contents = CONTENTS_MONSTER;
	box_leaf = &bspModel.leafs [bspModel.numLeafs];
	box_leaf->contents = CONTENTS_MONSTER;
	box_leaf->firstleafbrush = bspModel.numLeafBrushes;
	box_leaf->numleafbrushes = 1;
	bspModel.leafBrushes [bspModel.numLeafBrushes] = bspModel.numBrushes;

	//	surface returned for clip brushes (PLAYERCLIP, MONSTERCLIP, etc.)
	strcpy (clipSurface.name, "clipbrush");
	clipSurface.flags = SURF_NODRAW;

	for (i=0 ; i<6 ; i++)
	{
		side = i&1;
		// brush sides
		s = &bspModel.brushSides [bspModel.numBrushSides + i];
//		s->plane = 	map_planes + (numplanes+i*2+side);
		s->plane = 	bspModel.planes + (bspModel.numPlanes + i * 2 + side);

		s->surface = &nullSurface;
		// nodes
		c = &bspModel.nodes [box_headnode+i];
//		c->plane = map_planes + (numplanes+i*2);
		c->plane = bspModel.planes + (bspModel.numPlanes + i * 2);

		c->children[side] = -1 - bspModel.emptyLeaf;
		if (i != 5)
			c->children[side^1] = box_headnode+i + 1;
		else
			c->children[side^1] = -1 - bspModel.numLeafs;

		// planes
		p = &box_planes[i*2];
		p->type = i>>1;
		p->signbits = 0;
		p->normal.Zero();
		p->normal.SetValue( 1, i>>1 );
		p = &box_planes[i*2+1];
		p->type = 3 + (i>>1);
		p->signbits = 0;
		p->normal.Zero();
		p->normal.SetValue( -1, i>>1 );
	}	
}
/*
===================
CM_HeadnodeForBox
To keep everything totally uniform, bounding boxes are turned into small
BSP trees instead of being compared directly.
===================
*/
int	CM_HeadnodeForBox ( CVector &mins, CVector &maxs)
{
	box_planes[0].dist = maxs.x;
	box_planes[1].dist = -maxs.x;
	box_planes[2].dist = mins.x;
	box_planes[3].dist = -mins.x;
	box_planes[4].dist = maxs.y;
	box_planes[5].dist = -maxs.y;
	box_planes[6].dist = mins.y;
	box_planes[7].dist = -mins.y;
	box_planes[8].dist = maxs.z;
	box_planes[9].dist = -maxs.z;
	box_planes[10].dist = mins.z;
	box_planes[11].dist = -mins.z;

	return box_headnode;
}
/*
==================
CM_PointLeafnum_r
==================
*/
int CM_PointLeafnum_r ( CVector& p, int num)
{
/*
	float		d;
	cnode_t		*node;
	cplane_t	*plane;
	while (num >= 0)
	{
		node = bspModel.nodes + num;
		plane = node->plane;
		
		if (plane->type < 3)
		{
			d = p[plane->type] - plane->dist;
		}
		else
		{
//			d = DotProduct (plane->normal, p) - plane->dist;
			d = (plane->normal.x * p[0] + plane->normal.y * p[1] + plane->normal.z * p[2]) - plane->dist;
		}

		if (d < 0)
			num = node->children[1];
		else
			num = node->children[0];
	}
	c_pointcontents++;		// optimize counter
	return -1 - num;
*/
	float		d;
	cnode_t		*node;
	cplane_t	*plane;
	int			type;

	while (num >= 0)
	{
		node = bspModel.nodes + num;
		plane = node->plane;
		type = plane->type;
		
		if (type < 3)
			d = p[type] - plane->dist;
		else
			d = DotProduct (plane->normal, p) - plane->dist;
		if (d < 0)
			num = node->children[1];
		else
			num = node->children[0];
	}

	c_pointcontents++;		// optimize counter

	return -1 - num;
}
int CM_PointLeafnum ( CVector& p )
{
	if (!bspModel.numPlanes)
		return 0;		// sound may call this without map loaded
	return CM_PointLeafnum_r (p, 0);
}
/*
=============
CM_BoxLeafnums
Fills in a list of all the leafs touched
=============
*/
int		leaf_count, leaf_maxcount;
int		*leaf_list;
CVector	leaf_mins, leaf_maxs;
int		leaf_topnode;
void CM_BoxLeafnums_r (int nodenum)
{
	cplane_t	*plane;
	cnode_t		*node;
	int		s;
	while (1)
	{
		if (nodenum < 0)
		{
			if (leaf_count >= leaf_maxcount)
			{
//				Com_Printf ("CM_BoxLeafnums_r: overflow\n");
				return;
			}
			leaf_list[leaf_count++] = -1 - nodenum;
			return;
		}
	
		node = &bspModel.nodes [nodenum];
		plane = node->plane;
//		s = BoxOnPlaneSide (leaf_mins, leaf_maxs, plane);
		s = BOX_ON_PLANE_SIDE(leaf_mins, leaf_maxs, plane);
		if (s == 1)
			nodenum = node->children[0];
		else if (s == 2)
			nodenum = node->children[1];
		else
		{	// go down both
			if (leaf_topnode == -1)
				leaf_topnode = nodenum;
			CM_BoxLeafnums_r (node->children[0]);
			nodenum = node->children[1];
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int	CM_BoxLeafnums_headnode ( CVector &mins, CVector &maxs, int *list, int listsize, int headnode, int *topnode)
{
	leaf_list = list;
	leaf_count = 0;
	leaf_maxcount = listsize;
	leaf_mins = mins;
	leaf_maxs = maxs;
	leaf_topnode = -1;
	CM_BoxLeafnums_r (headnode);
	if (topnode)
		*topnode = leaf_topnode;
	return leaf_count;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int	CM_BoxLeafnums ( CVector &mins, CVector &maxs, int *list, int listsize, int *topnode)
{
	return CM_BoxLeafnums_headnode (mins, maxs, list,
		listsize, bspModel.subModels [0].headnode, topnode);
}

/*
==================
CM_PointContents
==================
*/
int CM_PointContents ( CVector &p, int headnode)
{
	int		l;
	if (!bspModel.numNodes)	// map not loaded
		return 0;

	l = CM_PointLeafnum_r (p, headnode);

	return bspModel.leafs [l].contents;
}

int CM_ContentsForLeaf( int leaf )
{
	return bspModel.leafs[leaf].contents;
}

unsigned short CM_ContentsColor( int leaf )
{
	unsigned short color;
	int firstleafbrush = bspModel.leafs[leaf].firstleafbrush;
	int	brush = bspModel.leafBrushes[firstleafbrush];
	int	firstbrushside = bspModel.brushes[brush].firstbrushside;
//	for( int i = 0; i < bspModel.brushes[brush].numsides; i++ )
	{
		color = bspModel.brushSides[firstbrushside].surface->color;
	}

	return color;
}

/*
==================
CM_TransformedPointContents
Handles offseting and rotation of the end points for moving and
rotating entities
==================
*/
int	CM_TransformedPointContents ( CVector &p, int headnode, CVector &origin, CVector &angles)
{
	CVector		p_l;
	CVector		temp;
	CVector		forward, right, up;
	int			l;
	// subtract origin offset
	p_l = p - origin;
	// rotate start and end into the models frame of reference
	if ((headnode != box_headnode) && (angles.x || angles.y || angles.z) )
	{
		AngleToVectors (angles, forward, right, up);
		temp = p_l;
		p_l.x = DotProduct (temp, forward);
		p_l.y = -DotProduct (temp, right);
		p_l.z = DotProduct (temp, up);
	}

	l = CM_PointLeafnum_r (p_l, headnode);
	 
	return bspModel.leafs [l].contents;
}
/*
===============================================================================
BOX TRACING
===============================================================================
*/
// 1/32 epsilon to keep floating point happy

CVector	trace_start, trace_end;
CVector	trace_mins, trace_maxs;
float	trace_extents[3];
trace_t	trace_trace;
int		trace_contents;
qboolean	trace_ispoint;		// optimized case
/*
================
CM_ClipBoxToBrush
================
*/
void CM_ClipBoxToBrush ( CVector &mins, CVector &maxs, CVector &p1, CVector &p2,
					  trace_t *trace, cbrush_t *brush)
{
	int			i;
	cplane_t	*plane, *clipplane;
	float		dist;
	float		enterfrac, leavefrac;
	CVector		ofs;
	float		d1, d2;
	qboolean	getout, startout;
	float		f;
	cbrushside_t	*side, *leadside;

	enterfrac = -1;
	leavefrac = 1;
	clipplane = NULL;

	if (!brush->numsides)
		return;

	c_brush_traces++;

	getout = false;
	startout = false;
	leadside = NULL;

	for (i=0 ; i<brush->numsides ; i++)
	{
		side = &bspModel.brushSides [brush->firstbrushside+i];
		plane = side->plane;

		// FIXME: special case for axial

		if (!trace_ispoint)
		{	// general box case

			// push the plane out apropriately for mins/maxs

			// FIXME: use signbits into 8 way lookup for each mins/maxs
			if (plane->normal.x < 0)
				ofs.x = maxs.x;
			else
				ofs.x = mins.x;
			if (plane->normal.y < 0)
				ofs.y = maxs.y;
			else
				ofs.y = mins.y;
			if (plane->normal.z < 0)
				ofs.z = maxs.z;
			else
				ofs.z = mins.z;

			dist = DotProduct (ofs, plane->normal);
			dist = plane->dist - dist;
		}
		else
		{	// special point case
			dist = plane->dist;
		}

		d1 = DotProduct (p1, plane->normal) - dist;
		d2 = DotProduct (p2, plane->normal) - dist;

		if (d2 > 0)
			getout = true;	// endpoint is not in solid
		if (d1 > 0)
			startout = true;

		// if completely in front of face, no intersection
		if (d1 > 0 && d2 >= d1)
			return;

		if (d1 <= 0 && d2 <= 0)
			continue;

		// crosses face
		if (d1 > d2)
		{	// enter
			f = (d1-DIST_EPSILON) / (d1-d2);
			if (f > enterfrac)
			{
				enterfrac = f;
				clipplane = plane;
				leadside = side;
			}
		}
		else
		{	// leave
			f = (d1+DIST_EPSILON) / (d1-d2);
			if (f < leavefrac)
				leavefrac = f;
		}
	}

	if (!startout)
	{	// original point was inside brush
		trace->startsolid = true;
		if (!getout)
			trace->allsolid = true;
		return;
	}
	if (enterfrac < leavefrac)
	{
		if (enterfrac > -1 && enterfrac < trace->fraction)
		{
			if (enterfrac < 0)
				enterfrac = 0;
			trace->fraction = enterfrac;
			if( clipplane != NULL )
				trace->plane = *clipplane;

			if( leadside != NULL )
				trace->surface = leadside->surface;
			else
				trace->surface = NULL;
			trace->contents = brush->contents;
		}
	}
}

/*
================
CM_TestBoxInBrush
================
*/
void CM_TestBoxInBrush ( CVector &mins, CVector &maxs, CVector &p1,
					  trace_t *trace, cbrush_t *brush)
{
	int			i;
	cplane_t	*plane;
	float		dist;
	CVector		ofs;
	float		d1;
	cbrushside_t	*side;

	if (!brush->numsides)
		return;

	for (i=0 ; i<brush->numsides ; i++)
	{
		side = &bspModel.brushSides [brush->firstbrushside+i];
		plane = side->plane;

		// FIXME: special case for axial

		// general box case

		// push the plane out apropriately for mins/maxs

		// FIXME: use signbits into 8 way lookup for each mins/maxs
		if (plane->normal.x < 0)
			ofs.x = maxs.x;
		else
			ofs.x = mins.x;
		if (plane->normal.y < 0)
			ofs.y = maxs.y;
		else
			ofs.y = mins.y;
		if (plane->normal.z < 0)
			ofs.z = maxs.z;
		else
			ofs.z = mins.z;

		dist = DotProduct (ofs, plane->normal);
		dist = plane->dist - dist;

		d1 = DotProduct (p1, plane->normal) - dist;

		// if completely in front of face, no intersection
		if (d1 > 0)
			return;

	}

	// inside this brush
	trace->startsolid = trace->allsolid = true;
	trace->fraction = 0;
	trace->contents = brush->contents;
}

/*
================
CM_TraceToLeaf
================
*/
void CM_TraceToLeaf (int leafnum)
{
	int			k;
	int			brushnum;
	cleaf_t		*leaf;
	cbrush_t	*b;

	leaf = &bspModel.leafs [leafnum];
	if ( !(leaf->contents & trace_contents))
		return;
	// trace line against all brushes in the leaf
	for (k=0 ; k<leaf->numleafbrushes ; k++)
	{
		brushnum = bspModel.leafBrushes[leaf->firstleafbrush+k];
		b = &bspModel.brushes [brushnum];
		if (b->checkcount == bspModel.checkCount)
			continue;	// already checked this brush in another leaf
		b->checkcount = bspModel.checkCount;

		if ( !(b->contents & trace_contents))
			continue;
		CM_ClipBoxToBrush (trace_mins, trace_maxs, trace_start, trace_end, &trace_trace, b);
		if (!trace_trace.fraction)
			return;
	}

}


/*
================
CM_TestInLeaf
================
*/
void CM_TestInLeaf (int leafnum)
{
	int			k;
	int			brushnum;
	cleaf_t		*leaf;
	cbrush_t	*b;

	leaf = &bspModel.leafs [leafnum];
	if ( !(leaf->contents & trace_contents))
		return;
	// trace line against all brushes in the leaf
	for (k=0 ; k<leaf->numleafbrushes ; k++)
	{
		brushnum = bspModel.leafBrushes [leaf->firstleafbrush+k];
		b = &bspModel.brushes [brushnum];
		if (b->checkcount == bspModel.checkCount)
			continue;	// already checked this brush in another leaf
		b->checkcount = bspModel.checkCount;

		if ( !(b->contents & trace_contents))
			continue;
		CM_TestBoxInBrush (trace_mins, trace_maxs, trace_start, &trace_trace, b);
		if (!trace_trace.fraction)
			return;
	}

}


/*
==================
CM_RecursiveHullCheck
==================
*/
void CM_RecursiveHullCheck (int num, float p1f, float p2f, CVector& p1, CVector& p2)
//void CM_RecursiveHullCheck (int num, float p1f, float p2f, CVector &p1, CVector &p2)
{
	cnode_t		*node;
	cplane_t	*plane;
	float		t1, t2, offset;
	float		frac, frac2;
	float		idist;
	CVector		mid;
	int			side;
	float		midf;
	int			type;

	if (trace_trace.fraction <= p1f)
		return;		// already hit something nearer

	// if < 0, we are in a leaf node
	if (num < 0)
	{
		CM_TraceToLeaf (-1-num);
		return;
	}
	//
	// find the point distances to the seperating plane
	// and the offset for the size of the box
	//
	node = bspModel.nodes + num;
	plane = node->plane;

	type = plane->type;
	if (type < 3)
	{
		t1 = p1[type] - plane->dist;
		t2 = p2[type] - plane->dist;
		offset = trace_extents[type];
	}
	else
	{
		t1 = DotProduct (plane->normal, p1) - plane->dist;
		t2 = DotProduct (plane->normal, p2) - plane->dist;
		if (trace_ispoint)
			offset = 0;
		else
			offset = fabs(trace_extents[0] * plane->normal.x) +
					 fabs(trace_extents[1] * plane->normal.y) +
					 fabs(trace_extents[2] * plane->normal.z);
	}
#if 0
CM_RecursiveHullCheck (node->children[0], p1f, p2f, p1, p2);
CM_RecursiveHullCheck (node->children[1], p1f, p2f, p1, p2);
return;
#endif
	// see which sides we need to consider
	if (t1 >= offset && t2 >= offset)
	{
		CM_RecursiveHullCheck (node->children[0], p1f, p2f, p1, p2);
		return;
	}
	if (t1 < -offset && t2 < -offset)
	{
		CM_RecursiveHullCheck (node->children[1], p1f, p2f, p1, p2);
		return;
	}
	// put the crosspoint DIST_EPSILON pixels on the near side
	if (t1 < t2)
	{
		idist = 1.0/(t1-t2);
		side = 1;
		frac2 = (t1 + offset + DIST_EPSILON)*idist;
		frac = (t1 - offset + DIST_EPSILON)*idist;
	}
	else if (t1 > t2)
	{
		idist = 1.0/(t1-t2);
		side = 0;
		frac2 = (t1 - offset - DIST_EPSILON)*idist;
		frac = (t1 + offset + DIST_EPSILON)*idist;
	}
	else
	{
		side = 0;
		frac = 1;
		frac2 = 0;
	}
	// move up to the node
	if (frac < 0)
		frac = 0;
	if (frac > 1)
		frac = 1;
		
	midf = p1f + (p2f - p1f)*frac;
	mid.x = p1.x + frac*(p2.x - p1.x);
	mid.y = p1.y + frac*(p2.y - p1.y);
	mid.z = p1.z + frac*(p2.z - p1.z);

	CM_RecursiveHullCheck (node->children[side], p1f, midf, p1, mid);
	// go past the node
	if (frac2 < 0)
		frac2 = 0;
	if (frac2 > 1)
		frac2 = 1;
		
	midf = p1f + (p2f - p1f)*frac2;
	mid.x = p1.x + frac2*(p2.x - p1.x);
	mid.y = p1.y + frac2*(p2.y - p1.y);
	mid.z = p1.z + frac2*(p2.z - p1.z);

	CM_RecursiveHullCheck (node->children[side^1], midf, p2f, mid, p2);
}
//======================================================================
/*
==================
CM_BoxTrace
==================
*/
trace_t		CM_BoxTrace ( CVector &start, CVector &end,
						  CVector &mins, CVector &maxs,
						  int headnode, int brushmask )
{
	bspModel.checkCount++;		// for multi-check avoidance
	c_traces++;			// for statistics, may be zeroed
	// fill in a default trace
	memset (&trace_trace, 0, sizeof(trace_trace));
	trace_trace.fraction = 1;
	trace_trace.surface = &nullSurface;

	if (!bspModel.numNodes)	// map not loaded
		return trace_trace;
	trace_contents = brushmask;
	trace_start = start;
	trace_end = end;
	trace_mins = mins;
	trace_maxs = maxs;

	//
	// check for position test special case
	//
	if (start.x == end.x && start.y == end.y && start.z == end.z)
	{
		int		leafs[1024];
		int		i, numleafs;
		CVector	c1, c2;
		int		topnode;

		c1 = start + mins;
		c2 = start + maxs;
		c1.x -= 1;
		c1.y -= 1;
		c1.z -= 1;

		c2.x += 1;
		c2.y += 1;
		c2.z += 1;

		numleafs = CM_BoxLeafnums_headnode (c1, c2, leafs, 1024, headnode, &topnode);
		for (i=0 ; i<numleafs ; i++)
		{
			CM_TestInLeaf (leafs[i]);
			if (trace_trace.allsolid)
				break;
		}
		trace_trace.endpos = start;
		return trace_trace;
	}

	//
	// check for point special case
	//
	if (mins.x == 0 && mins.y == 0 && mins.z == 0 && maxs.x == 0 && maxs.y == 0 && maxs.z == 0)
	{
		trace_ispoint = true;
		trace_extents[0] = trace_extents[1] = trace_extents[2] = 0;
	}
	else
	{
		trace_ispoint = false;
		trace_extents[0] = -mins.x > maxs.x ? -mins.x : maxs.x;
		trace_extents[1] = -mins.y > maxs.y ? -mins.y : maxs.y;
		trace_extents[2] = -mins.z > maxs.z ? -mins.z : maxs.z;
	}

	//
	// general sweeping through world
	//
	CM_RecursiveHullCheck (headnode, 0, 1, start, end);

	if (trace_trace.fraction == 1)
	{
		trace_trace.endpos = end;
	}
	else
	{
		trace_trace.endpos.x = start.x + trace_trace.fraction * (end.x - start.x);
		trace_trace.endpos.y = start.y + trace_trace.fraction * (end.y - start.y);
		trace_trace.endpos.z = start.z + trace_trace.fraction * (end.z - start.z);
	}
	return trace_trace;
}
/*
==================
CM_TransformedBoxTrace
Handles offseting and rotation of the end points for moving and
rotating entities
==================
*/
#ifdef _WIN32
#pragma optimize( "", off )
#endif


trace_t CM_TransformedBoxTrace ( CVector &start, CVector &end,
								  CVector &mins, CVector &maxs,
								  int headnode, int brushmask,
								  CVector &origin, CVector &angles)
{
	trace_t		trace;
	CVector		start_l, end_l;
	CVector		a;
	CVector		forward, right, up;
	CVector		temp;
	qboolean	rotated;
	// subtract origin offset
	start_l = start - origin;
	end_l = end - origin;
	// rotate start and end into the models frame of reference
	if (headnode != box_headnode && (angles.x || angles.y || angles.z) )
		rotated = true;
	else
		rotated = false;
	if (rotated)
	{
		AngleToVectors (angles, forward, right, up);
		temp = start_l;
		start_l.x = DotProduct (temp, forward);
		start_l.y = -DotProduct (temp, right);
		start_l.z = DotProduct (temp, up);
		temp = end_l;
		end_l.x = DotProduct (temp, forward);
		end_l.y = -DotProduct (temp, right);
		end_l.z = DotProduct (temp, up);
	}
	// sweep the box through the model
	trace = CM_BoxTrace (start_l, end_l, mins, maxs, headnode, brushmask);
	if (rotated && trace.fraction != 1.0)
	{
		// FIXME: figure out how to do this with existing angles
		a.Negate( angles );
		AngleToVectors (a, forward, right, up);
		temp = trace.plane.normal;
		trace.plane.normal.x = DotProduct (temp, forward);
		trace.plane.normal.y = -DotProduct (temp, right);
		trace.plane.normal.z = DotProduct (temp, up);
	}
	trace.endpos.x = start.x + trace.fraction * (end.x - start.x);
	trace.endpos.y = start.y + trace.fraction * (end.y - start.y);
	trace.endpos.z = start.z + trace.fraction * (end.z - start.z);
	return trace;
}

#ifdef _WIN32
#pragma optimize( "", on )
#endif


/*
===============================================================================
PVS / PHS
===============================================================================
*/
/*
===================
CM_DecompressVis
===================
*/
void CM_DecompressVis (byte *in, byte *out)
{
	int		c;
	byte	*out_p;
	int		row;
	
	row = (bspModel.numClusters + 7) >> 3;	
	out_p = out;

	if (!in || bspModel.numVisibility == 0)
	{	// no vis info, so make all visible
		while (row)
		{
			*out_p++ = 0xff;
			row--;
		}
		return;		
	}
	do
	{
		if (*in)
		{
			*out_p++ = *in++;
			continue;
		}
	
		c = in[1];
		in += 2;
		if ((out_p - out) + c > row)
		{
			c = row - (out_p - out);
			Com_DPrintf ("warning: Vis decompression overrun\n");
		}
		while (c)
		{
			*out_p++ = 0;
			c--;
		}
	} while (out_p - out < row);
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

byte	*CM_ClusterPVS (int cluster)
{
	if (cluster == -1 || !bspModel.visibility)
		memset (pvsrow, 0, (bspModel.numClusters + 7) >> 3);
	else
		//	CM_DecompressVis (map_visibility + map_vis->bitofs[cluster][DVIS_PVS], pvsrow);
		CM_DecompressVis (bspModel.visibility + (bspModel.dVis->bitofs[cluster][DVIS_PVS]), pvsrow);

	return pvsrow;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

byte	*CM_ClusterPHS (int cluster)
{
	if (cluster == -1 || !bspModel.visibility)
		memset (phsrow, 0, (bspModel.numClusters + 7) >> 3);
	else
		//	CM_DecompressVis (map_visibility + map_vis->bitofs[cluster][DVIS_PHS], phsrow);
		CM_DecompressVis (bspModel.visibility + (bspModel.dVis->bitofs[cluster][DVIS_PHS]), phsrow);
	return phsrow;
}
/*
===============================================================================
AREAPORTALS
===============================================================================
*/
void FloodArea_r (carea_t *area, int floodnum)
{
	int		i;
	dareaportal_t	*p;
	if (area->floodvalid == bspModel.floodValid)
	{
		if (area->floodnum == floodnum)
			return;
		Com_Error (ERR_DROP, "FloodArea_r: reflooded");
	}
	area->floodnum = floodnum;
	area->floodvalid = bspModel.floodValid;
	
	p = &bspModel.areaPortals [area->firstareaportal];
	
	for (i=0 ; i<area->numareaportals ; i++, p++)
	{
		if (bspModel.portalOpen [p->portalnum])
			FloodArea_r (&bspModel.areas[p->otherarea], floodnum);
	}
}
/*
====================
FloodAreaConnections
====================
*/
void	FloodAreaConnections (void)
{
	int		i;
	carea_t	*area;
	int		floodnum;
	// all current floods are now invalid
	bspModel.floodValid++;
	floodnum = 0;
	// area 0 is not used
	for (i = 1; i < bspModel.numAreas; i++)
	{
		area = &bspModel.areas [i];

		if (area->floodvalid == bspModel.floodValid)
			continue;		// already flooded into

		floodnum++;
		FloodArea_r (area, floodnum);
	}
}


void	CM_SetAreaPortalState (int portalnum, qboolean open)
{
	if (portalnum > bspModel.numAreaPortals)
		Com_Error (ERR_DROP, "areaportal > numareaportals");
	bspModel.portalOpen [portalnum] = open;
	FloodAreaConnections ();
}


qboolean	CM_AreasConnected (int area1, int area2)
{
	if (map_noareas->value)
		return true;
	if (area1 > bspModel.numAreas || area2 > bspModel.numAreas)
		Com_Error (ERR_DROP, "area > numareas");
	if (bspModel.areas [area1].floodnum == bspModel.areas [area2].floodnum)
		return true;
	return false;
}

qboolean CM_inPVS( CVector &p1, CVector &p2 )
{
	int		leafnum;
	int		cluster;
	int		area1, area2;
	byte	*mask;

	leafnum = CM_PointLeafnum (p1);
	cluster = CM_LeafCluster (leafnum);
	area1 = CM_LeafArea (leafnum);
	mask = CM_ClusterPVS (cluster);
	leafnum = CM_PointLeafnum (p2);
	cluster = CM_LeafCluster (leafnum);
	area2 = CM_LeafArea (leafnum);
	if ( mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
		return false;
	if (!CM_AreasConnected (area1, area2))
		return false;		// a door blocks sight
	return true;
}


/*
=================
CM_WriteAreaBits
Writes a length byte followed by a bit vector of all the areas
that area in the same flood as the area parameter
This is used by the client refreshes to cull visibility
=================
*/
int CM_WriteAreaBits (byte *buffer, int area)
{
	int		i;
	int		floodnum;
	int		bytes;
	bytes = (bspModel.numAreas + 7) >> 3;

	if (map_noareas->value)
	{	// for debugging, send everything
		memset (buffer, 255, bytes);
	}
	else
	{
		memset (buffer, 0, bytes);
		floodnum = bspModel.areas [area].floodnum;

		for (i = 0; i<bspModel.numAreas; i++)
		{
			if (bspModel.areas[i].floodnum == floodnum || !area)
				buffer[i>>3] |= 1<<(i&7);
		}
	}

	return bytes;
}
/*
===================
CM_WritePortalState
Writes the portal state to a savegame file
===================
*/
void	CM_WritePortalState (FILE *f)
{
	fwrite (bspModel.portalOpen, sizeof(bspModel.portalOpen), 1, f);
}
/*
===================
CM_ReadPortalState
Reads the portal state from a savegame file
and recalculates the area connections
===================
*/
void	CM_ReadPortalState (FILE *f)
{
	FS_Read (bspModel.portalOpen, sizeof (bspModel.portalOpen), f);
	FloodAreaConnections ();
}
/*
=============
CM_HeadnodeVisible
Returns true if any leaf under headnode has a cluster that
is potentially visible
=============
*/
qboolean CM_HeadnodeVisible (int nodenum, byte *visbits)
{
	int		leafnum;
	int		cluster;
	cnode_t	*node;

	if (nodenum < 0)
	{
		leafnum = -1-nodenum;
		cluster = bspModel.leafs [leafnum].cluster;
		if (cluster == -1)
			return false;
		if (visbits[cluster>>3] & (1<<(cluster&7)))

			return true;
		return false;
	}
	node = &bspModel.nodes [nodenum];
	if (CM_HeadnodeVisible(node->children[0], visbits))
		return true;
	return CM_HeadnodeVisible(node->children[1], visbits);
}
