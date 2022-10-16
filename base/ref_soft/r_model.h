
#ifndef __MODEL__
#define __MODEL__

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

#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2


// plane_t structure
// !!! if this is changed, it must be changed in asm_i386.h too !!!
typedef struct mplane_s
{
	CVector	normal;
	float	dist;
	byte	type;			// for texture axis selection and fast side tests
	byte	signbits;		// signx + signy<<1 + signz<<1
	byte	pad[2];
} mplane_t;


// FIXME: differentiate from texinfo SURF_ flags
#define	SURF_PLANEBACK		2
#define	SURF_DRAWSKY		4			// sky brush face
#define SURF_DRAWTURB		0x10
#define SURF_DRAWBACKGROUND	0x40
#define SURF_DRAWSKYBOX		0x80		// sky box

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct
{
	unsigned short	v[2];
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
	float		mipadjust;
	int			flags;
	int			numframes;
	struct mtexinfo_s	*next;		// animation chain
	image_t		*image;
	CVector		color;
} mtexinfo_t;

#define	VERTEXSIZE	7

/*
typedef struct glpoly_s
{
	struct	glpoly_s	*next;
	struct	glpoly_s	*chain;
	int		numverts;
	int		flags;			// for SURF_UNDERWATER (not needed anymore?)
	float	verts[4][VERTEXSIZE];	// variable sized (xyz s1t1 s2t2)
} glpoly_t;
*/
typedef struct glpoly_s
{
	struct	glpoly_s	*next;
	struct	glpoly_s	*chain;
	int		numverts;
	int		flags;			// for SURF_UNDERWATER (not needed anymore?)
//	float	verts[4][VERTEXSIZE];	// variable sized (xyz s1t1 s2t2)
	CVector *verts;
	float	*s1, *s2, *t1, *t2;
} glpoly_t;

typedef struct msurface_s
{
	int			visframe;		// should be drawn when node is crossed

	int			dlightframe;
	int			dlightbits;

	mplane_t	*plane;
	int			flags;

	int			firstedge;	// look up in model->surfedges[], negative numbers
	int			numedges;	// are backwards edges
	
// surface generation data
	struct surfcache_s	*cachespots[MIPLEVELS];

	short		texturemins[2];
	short		extents[2];

	mtexinfo_t	*texinfo;
	
// lighting info
	byte		styles[MAXLIGHTMAPS];
	byte		*samples;		// [numstyles*surfsize]

	struct msurface_s *nextalphasurface;

	glpoly_t	*polys;
	entity_t	*entity;
	short		surfSpriteIndex;	//	index in wallSpriteList of first sprite attached to this surface
									//	-1 means no sprites!
	short		planeNum;
	short		surfIndex;			//	index of this surface
} msurface_t;


#define	CONTENTS_NODE	-1
typedef struct mnode_s
{
// common with leaf
	int			contents;		// CONTENTS_NODE, to differentiate from leafs
	int			visframe;		// node needs to be traversed if current
	
	short		minmaxs[6];		// for bounding box culling

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
	int			contents;		// wil be something other than CONTENTS_NODE
	int			visframe;		// node needs to be traversed if current

	short		minmaxs[6];		// for bounding box culling

	struct mnode_s	*parent;

// leaf specific
	int			cluster;
	int			area;

	msurface_t	**firstmarksurface;
	int			nummarksurfaces;
	int			key;			// BSP sequence number for leaf's contents
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
// solid volume for clipping (sent from server)
//
	qboolean	clipbox;
	CVector		clipmins, clipmaxs;

//
// brush model
//
	int			firstmodelsurface, nummodelsurfaces;

	int			numsubmodels;
	dmodel_t	*submodels;

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

	int			numsurfedges;
	int			*surfedges;

	int			nummarksurfaces;
	msurface_t	**marksurfaces;

	dvis_t		*vis;

	byte		*lightdata;

	// for alias models and sprites
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
void	*Mod_Extradata (model_t *mod);	// handles caching
void	Mod_TouchModel (char *name);

mleaf_t *Mod_PointInLeaf (CVector& p, model_t *model);
byte	*Mod_ClusterPVS (int cluster, model_t *model);

void Mod_Modellist_f (void);
void Mod_FreeAll (void);
void Mod_Free (void *mod);

extern	int		registration_sequence;

#endif	// __MODEL__
