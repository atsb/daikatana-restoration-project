#ifndef	CMODEL_H_
#define	CMODEL_H_

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define	CMOD_EXTRA_DATA_SIZE	12	//	extra elements added to the end of some
									//	structures where dynamic data is stored
									//	during clipping
	
///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
	cplane_t	*plane;
	int			children[2];		// negative numbers are leafs
} cnode_t;

typedef struct
{
	cplane_t	*plane;
	csurface_t	*surface;
} cbrushside_t;

typedef struct
{
	int			contents;
	int			cluster;
	int			area;
	unsigned short	firstleafbrush;
	unsigned short	numleafbrushes;
} cleaf_t;

typedef struct
{
	int			contents;
	int			numsides;
	int			firstbrushside;
	int			checkcount;		// to avoid repeated testings
} cbrush_t;

typedef struct
{
	int		numareaportals;
	int		firstareaportal;
	int		floodnum;			// if two areas have equal floodnums, they are connected
	int		floodvalid;
} carea_t;

typedef	struct	bspModel_s
{
	char				mapName [MAX_QPATH];
	void				*hunkBase;

	int					numBrushSides;
	cbrushside_t		*brushSides;
	
	int					numSurfaces;
	csurface_t			*surfaces;

	int					numPlanes;
	cplane_t			*planes;

	int					numNodes;
	cnode_t				*nodes;

	int					numLeafs;
	cleaf_t				*leafs;

	int					numLeafBrushes;
	unsigned short		*leafBrushes;

	int					numSubModels;
	cmodel_t			*subModels;

	int					numBrushes;
	cbrush_t			*brushes;

	int					numVisibility;
	byte				*visibility;
	dvis_t				*dVis;

	int					numEntityChars;
	char				*entityChars;

	int					numAreas;
	carea_t				*areas;

	int					numAreaPortals;
	dareaportal_t		*areaPortals;
	qboolean			*portalOpen;

	int					checkCount;
	int					numClusters;
	int					floodValid;
	int					emptyLeaf;
	int					solidLeaf;

	int					fixModelClip;	//	when true, cl.model_clip needs to be remade
} bspModel_t;

#endif