#include "dk_shared.h"


#ifndef _GL_MODEL_H
#define _GL_MODEL_H

//#ifdef __cplusplus
//extern "C" {
//#endif

/*

d*_t structures are on-disk representations
m*_t structures are in-memory

*/

/*
==============================================================================

BRUSH MODELS

==============================================================================
*/


//
// in memory representation
//
// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct
{
	CVector		position;
} mvertex_t;

typedef struct
{
	CVector		mins, maxs;
	CVector		origin;		// for sounds or lights
	float		radius;
	int			headnode;
	int			visleafs;		// not including the solid leaf 0
	int			firstface, numfaces;
} mmodel_t;


#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2


#define	SURF_PLANEBACK		2
#define	SURF_DRAWSKY		4
#define SURF_DRAWTURB		0x10
#define SURF_DRAWBACKGROUND	0x40
#define SURF_UNDERWATER		0x80
#define SURF_DRAWFOG		0x100

//	this is the GL version of cplane_t.  Since GL does not use the planeIndex field
//	it was removed to decrease memory requirements...  someone please make the client/
//	server/ref_gl use the same f-ing bsp...
typedef struct mplane_s
{
	CVector	normal;
	float	dist;
	byte	type;			// for fast side tests
	byte	signbits;		// signx + (signy<<1) + (signz<<1)
	byte	pad[2];
	unsigned	short	planeIndex;	//	index of this plane into map_planes (cmodel.c) and 
									//	r_worldmodel->planes (ref_gl) so that plane indices 
									//	can be passed to client for wall damage
} mplane_t;

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct
{
	unsigned short	v[2];				//	Nelno:	index into vertex list
	unsigned int	cachededgeoffset;
} medge_t;

/*
typedef struct mtexinfo_s
{
	float		vecs[2][4];
	int			flags;
	int			numframes;
	struct mtexinfo_s	*next;		// animation chain
	image_t		*image;
	CVector		color;
} mtexinfo_t;
*/
typedef struct mtexinfo_s
{
//	float		vecs[2][4];
	CVector		s, t;
	float		s_offset, t_offset;
	int			flags;
	int			numframes;
	struct mtexinfo_s	*next;		// animation chain
	image_t		*image;
	unsigned short	color;
	int			value;
} mtexinfo_t;

#define	VERTEXSIZE	7

typedef struct glpoly_s
{
	struct	glpoly_s	*next;
	struct	glpoly_s	*chain;
	int		numverts;
	CVector *verts;
	float	*s1, *s2, *t1, *t2;
} glpoly_t;

typedef struct msurface_s
{
	int			visframe;		// should be drawn when node is crossed
	mplane_t	*plane;
	int			flags;

	int			firstedge;	// look up in model->surfedges[], negative numbers
	int			numedges;	// are backwards edges
	
	short		texturemins[2];
	short		extents[2];

	int			light_s, light_t;	// gl lightmap coordinates
	int			dlight_s, dlight_t; // gl lightmap coordinates for dynamic lightmaps

	glpoly_t	*polys;				// multiple if warped
	struct	msurface_s	*texturechain;
	struct  msurface_s	*lightmapchain;

	mtexinfo_t	*texinfo;
	
// lighting info
	int			dlightframe;
	int			dlightbits;

	int			lightmaptexturenum;
	byte		styles[MAXLIGHTMAPS];
	float		cached_light[MAXLIGHTMAPS];	// values currently used in lightmap
	byte		*samples;		// [numstyles*surfsize]

	entity_t	*entity;			// SCG[5/10/99]: the entity that this surface belongs to ( for transofrmation )

	short		brushnum;			// SCG[5/10/99]: the [map] brush this surface belongs to

	short		nNumSurfSprites;	// SCG[5/10/99]: added to keep the number of surface sprites per surface to a minimum

} msurface_t; // SCG[5/10/99]: 108 bytes ( padds out to 128 )

typedef struct mnode_s
{
// common with leaf
	int			contents;		// -1, to differentiate from leafs
	int			visframe;		// node needs to be traversed if current

//	float		minmaxs[6];		// for bounding box culling
	CVector		mins, maxs;

	struct mnode_s	*parent;

// node specific
	mplane_t	*plane;
	struct mnode_s	*children[2];	

	unsigned short		firstsurface;
	unsigned short		numsurfaces;
} mnode_t;



typedef struct mleaf_s
{
// common with node
	int			contents;		// wil be a negative contents number
	int			visframe;		// node needs to be traversed if current

	CVector		mins;
	CVector		maxs;		// for bounding box culling

	struct mnode_s	*parent;

// leaf specific
	int			cluster;
	int			area;

	msurface_t	**firstmarksurface;
	int			nummarksurfaces;
} mleaf_t;


//===================================================================

//
// Whole model
//

#define	MAX_POLYS_PER_PLANE	1024

typedef	struct
{
	int	numFaces;
	int	faceNum [MAX_POLYS_PER_PLANE];
} faceList_t;

typedef	struct
{
	faceList_t	*planes [MAX_MAP_PLANES];
} planeList_t;


typedef enum {mod_bad, mod_brush, mod_sprite, mod_alias } modtype_t;

typedef struct model_s
{
	char		name[MAX_QPATH];

	int			registration_sequence;

	modtype_t	type;
	int			numframes;
	
	int			flags;
	resource_t	resource;
//
// volume occupied by the model graphics
//		
	CVector		mins, maxs;
	float		radius;

//
// solid volume for clipping 
//
	qboolean	clipbox;
	CVector		clipmins, clipmaxs;

//
// brush model
//
	int			firstmodelsurface, nummodelsurfaces;
	int			lightmap;		// only for submodels

	int			numsubmodels;
	mmodel_t	*submodels;

	int			numplanes;
	mplane_t	*planes;

	int			numleafs;		// number of visible leafs, not counting 0
	mleaf_t		*leafs;

	int			numvertexes;
	mvertex_t	*vertexes;

	int			numedges;
	medge_t		*edges;

	int			numnodes;
	int			firstnode;
	mnode_t		*nodes;

	int			numtexinfo;
	mtexinfo_t	*texinfo;

	int			numsurfaces;
	msurface_t	*surfaces;

	int			numfogsurfaces;
	msurface_t	*fogsurfaces;

	int			numsurfedges;
	int			*surfedges;

	int			nummarksurfaces;
	msurface_t	**marksurfaces;

	dvis_t		*vis;

	byte		*lightdata;

	// for alias models and skins
	image_t		*skins[MAX_MD2SKINS];

	int			extradatasize;
	void		*extradata;

	planeList_t	*planePolys;	//	list of polys for each plane
} model_t;

//============================================================================

#define	IS_WORLD_MODEL	true
#define	NOT_WORLD_MODEL	false

void	Mod_Init (void);
void	Mod_ClearAll (void);
model_t *Mod_ForName (const char *name, qboolean crash, int flags);
mleaf_t *Mod_PointInLeaf (CVector &p, model_t *model);
byte	*Mod_ClusterPVS (int cluster, model_t *model);

void	Mod_Modellist_f (void);

void	*Hunk_Begin (int maxsize);
void	*Hunk_Alloc (int size);
int		Hunk_End (void);
void	Hunk_Free (void *base);

void	Mod_FreeAll (void);
void	Mod_Free (void *mod);


//#ifdef __cplusplus
//}
//#endif

#endif _GL_MODEL_H