#include "dk_shared.h"

#include "dk_object_reference.h"
#include "dk_pointer.h"
#include "dk_buffer.h"
#include "dk_array.h"

#include "dk_ref_pic.h"
#include "dk_ref_wal.h"

#include "d3d_texture.h"



//-----------------------------------------------------------------------------
// DEFINES
//-----------------------------------------------------------------------------
#define	IS_WORLD_MODEL			true
#define	NOT_WORLD_MODEL			false

#define	SIDE_FRONT				0
#define	SIDE_BACK				1
#define	SIDE_ON					2

#define	SURF_PLANEBACK			2
#define	SURF_DRAWSKY			4
#define SURF_DRAWTURB			0x10
#define SURF_DRAWBACKGROUND		0x40
#define SURF_UNDERWATER			0x80

#define	MAX_POLYS_PER_PLANE		1024

#define	MAX_LBM_HEIGHT			480

#define BACKFACE_EPSILON		0.01

//-----------------------------------------------------------------------------
// TYPEDEFS
//-----------------------------------------------------------------------------

typedef	struct
{
	int	numFaces;
	int	faceNum [MAX_POLYS_PER_PLANE];
} faceList_t;

typedef	struct
{
	faceList_t	*planes [MAX_MAP_PLANES];
} planeList_t;
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

typedef struct
{
	unsigned short	v[2];				//	Nelno:	index into vertex list
	unsigned int	cachededgeoffset;
} medge_t;

typedef struct mtexinfo_s
{
	float				vecs[2][4];
	int					flags;
	int					numframes;
	struct mtexinfo_s	*next;		// animation chain
	const CTexture		*texture;
	CVector				color;
} mtexinfo_t;

typedef struct msurface_s
{
	int					visframe;		// should be drawn when node is crossed

	mplane_t			*plane;
	int					flags;

	int					firstedge;	// look up in model->surfedges[], negative numbers
	int					numedges;	// are backwards edges
	
	short				texturemins[2];
	short				extents[2];

	int					light_s, light_t;	// gl lightmap coordinates
	int					dlight_s, dlight_t; // gl lightmap coordinates for dynamic lightmaps

//	glpoly_t	*polys;				// multiple if warped
	struct	msurface_s	*texturechain;
	struct  msurface_s	*lightmapchain;

	mtexinfo_t			*texinfo;
	
// lighting info
	int					dlightframe;
	int					dlightbits;

	int					lightmaptexturenum;
	byte				styles[MAXLIGHTMAPS];
	float				cached_light[MAXLIGHTMAPS];	// values currently used in lightmap
	byte				*samples;		// [numstyles*surfsize]

	entity_t			*entity;
	short				surfSpriteIndex;	//	index in wallSpriteList of first sprite attached to this surface
									//	-1 means no sprites!
	short				planeNum;
	short				surfIndex;			//	index of this surface
} msurface_t;

typedef struct mnode_s
{
// common with leaf
	int				contents;		// -1, to differentiate from leafs
	int				visframe;		// node needs to be traversed if current
	
	float			minmaxs[6];		// for bounding box culling

	struct mnode_s	*parent;

// node specific
	mplane_t		*plane;
	struct mnode_s	*children[2];	

	unsigned short	firstsurface;
	unsigned short	numsurfaces;
} mnode_t;

typedef struct mleaf_s
{
// common with node
	int				contents;		// wil be a negative contents number
	int				visframe;		// node needs to be traversed if current

	float			minmaxs[6];		// for bounding box culling

	struct mnode_s	*parent;

// leaf specific
	int				cluster;
	int				area;

	msurface_t		**firstmarksurface;
	int				nummarksurfaces;
} mleaf_t;

typedef enum 
{
	mod_bad, 
	mod_brush, 
	mod_sprite, 
	mod_alias 
} modtype_t;

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

	int			numsurfedges;
	int			*surfedges;

	int			nummarksurfaces;
	msurface_t	**marksurfaces;

	dvis_t		*vis;

	byte		*lightdata;

	// for alias models and sprites
//	image_t		*skins[MAX_MD2SKINS];
	const CPic	*skins[MAX_MD2SKINS];

	int			extradatasize;
	void		*extradata;

	planeList_t	*planePolys;	//	list of polys for each plane
} model_t;



//-----------------------------------------------------------------------------
// FUNCTION PROTOTYPES
//-----------------------------------------------------------------------------
void	Mod_Free (model_t *mod);
