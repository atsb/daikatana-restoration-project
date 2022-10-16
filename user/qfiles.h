//#pragma once

#ifndef _qfile_h
#define _qfile_h


//
// qfiles.h: quake file formats
// This file must be identical in the quake and utils directories
//

/*
========================================================================

The .pak files are just a linear collapse of a directory tree

========================================================================
*/

#define IDPAKHEADER		(('K'<<24)+('C'<<16)+('A'<<8)+'P')

typedef struct
{
	char	name[56];
	int		filepos, filelen;
	int     compresslen, compresstype;

} dpackfile_t;

typedef struct
{
	int		ident;		// == IDPAKHEADER
	int		dirofs;
	int		dirlen;
} dpackheader_t;

//#define	MAX_FILES_IN_PACK	4096
#define	MAX_FILES_IN_PACK	20480


/*
========================================================================

PCX files are used for as many images as possible

========================================================================
*/

typedef struct
{
    char	manufacturer;
    char	version;
    char	encoding;
    char	bits_per_pixel;
    unsigned short	xmin,ymin,xmax,ymax;
    unsigned short	hres,vres;
    unsigned char	palette[48];
    char	reserved;
    char	color_planes;
    unsigned short	bytes_per_line;
    unsigned short	palette_type;
    char	filler[58];
    unsigned char	data;			// unbounded
} pcx_t;


/*
========================================================================

.MD2 triangle model file format

========================================================================
*/

#define IDALIASHEADER	(('D'<<24)+('M'<<16)+('K'<<8)+'D')
#define ALIAS_VERSION	1
#define ALIAS_VERSION2	ALIAS_VERSION+1

#define	MAX_TRIANGLES	4096
#define MAX_VERTS		3072
#define MAX_FRAMES		2048
#define MAX_MD2SKINS	32
#define	MAX_SKINNAME	64
#define MAX_SEQUENCES	256


typedef struct
{
	short	s;
	short	t;
} dstvert_t;

typedef struct
{
	short	index_st[3];
} dstframe_t;

/*
typedef struct 
{
	short	index_surface;
	short	index_xyz[3];
	short	index_st[3];
} dtriangle_t;
*/

typedef struct 
{
	short		index_surface;
	short		num_uvframes;
	short		index_xyz[3];
	dstframe_t	stframes[1];
} dtriangle_t;

typedef struct
{
	byte	v[3];			// scaled byte to fit in frame mins/maxs
	byte	lightnormalindex;
} dtrivertx_t;

#pragma pack( push, 1 )
typedef struct
{
	unsigned int		v;				// scaled byte to fit in frame mins/maxs
	byte	lightnormalindex;
} dtrivertx2_t;
#pragma pack( pop, 1 )

#define DTRIVERTX_V0   0
#define DTRIVERTX_V1   1
#define DTRIVERTX_V2   2
#define DTRIVERTX_LNI  3
#define DTRIVERTX_SIZE 4

#define DTRIVERTX2_V0   0
#define DTRIVERTX2_V1   0
#define DTRIVERTX2_V2   0
#define DTRIVERTX2_LNI  4
#define DTRIVERTX2_SIZE 5

typedef struct
{
	CVector		scale;	// multiply byte verts by this
	CVector		translate;	// then add this
	char		name[16];	// frame name from grabbing
	dtrivertx_t	verts[1];	// variable sized
} daliasframe_t;

typedef struct
{
	CVector		scale;	// multiply byte verts by this
	CVector		translate;	// then add this
	char		name[16];	// frame name from grabbing
	dtrivertx2_t	verts[1];	// variable sized
} daliasframe2_t;

typedef struct animSeq_s
{
	char			seqName[16];	// animation sequence name
	int				startFrame;		// frame the sequence starts on
	int				endFrame;		// frame the sequence ends on
} animSeq_t;


// the glcmd format:
// a positive integer starts a tristrip command, followed by that many
// vertex structures.
// a negative integer starts a trifan command, followed by -x vertexes
// a zero indicates the end of the command list.
// a vertex consists of a floating point s, a floating point t,
// and an integer vertex index.

typedef struct dmdl_s 
{
    
    int ident;			//file identifier.
    int version;		//file version number.
    CVector org;		//origin of model.
    int framesize;		//the size, in bytes of each frame of animation.
    int num_skins;		//the number of skins in the model.  each skin is a string MAX_SKINNAME characters long.
    int num_xyz;		//the number of points in each frame.
    //the number of s-t texture coordinates.  points on some triangles share
    //texture coordinates, some points have multiple texture coordinates, each for different triangles
    //that share that point.  Also, s-t's for all the animated texture frames are stored here, just sorta
    //glommed together.
    int num_st;
    int num_tris;		//the number of triangles.
    int num_glcmds;		//the length of the gl execute buffer.
    int num_frames;		//total number of frames in the model.
    int num_surfaces;	//the number of surface definitions.

    //the offsets of the various data arrays.
    int ofs_skins;      // each skin is char[MAX_SKINNAME]
    int ofs_st;         // byte offset from start for stverts
    int ofs_tris;       // offset for dtriangles
    int ofs_frames;     // offset for first frame
    int ofs_glcmds; 
    int ofs_surfaces;   // offset for surface headers
    int ofs_end;        // end of file

	//
	// add new stuff below here for 'backwards' compatibility
	//

	int num_sequences;	// number of animation sequences
	int ofs_sequences;	// offset for the sequence data

} dmdl_t;

/*
**  Version 1 layout of model on disk (all vars written little-endian):  
**  
**    int: file id  ('DKMD')
**    int: version number (1)
**    vec3_t: model origin
**    int: size of each frame's data
**    int: number of skin names (=num_skins)
**    int: number of points in each frame. (=num_xyz)
**    int: number of s-t texture coordinates (=num_st)
**    int: number of triangles (=num_triangles)
**    int: number of ints in GL execute buffer. (=gl_buffer_len)
**    int: number of frames (=num_frames)
**    int: number of surface definitions (=num_surfaces)
**    int: offset in file where skin names start.
**    int: offset if file where s-t coordinates start.
**    int: offset in file where triangles begin.
**    int: offset in file where frames begin
**    int: offset in file where GL execute buffer begins.
**    int: offset in file where surface descriptions begin.
**    int: offset in file where something else might go, if someone wanted to add it to the format.  wtf
**      
**    num_skins of:
**      char[64]
**
**    num_st of:
**      float: s    not 0-1 normalized.
**      float: t    not 0-1 normalized.
**
**    num_triangles of:
**      short: surface index used on this poly.
**      short: number of uv frames of texture information for this poly (=num_uv)
**      short[3]: indices of the 3 points used for this poly.
**      num_uv of:
**        short[3]: indices of the 3 s-t's used for the 3 points of the poly.
**
**    num_frames of:
**      float[3]: x y and z scale       //multiply quantized vertex values by scale and add offset to get 
**      float[3]: x y and z offset      //float values for the vertex coordinates in model space.
**      char[16]: frame name
**      num_xyz of:
**        byte[3]: quantized x y and z values.
**        byte: index of light normal for point.
**
**    gl_buffer_len of:
**      int: fuckin void *.  Carmack implemented an execute buffer, cute.
**        interpret it as follows:
**          int: number of points in this op (=num_points), and flag for strip or fan.  if
**              positive, it's a strip, negative means a fan. 0 means end of buffer.
**          int: skin index into model skin array.
**          int: surface index into model surface array.
**          num_points of:
**            int: index of vertex into the frame vertex array.
**            num uv frames of:  //get the number of uv frames from the surface definition.
**              float: u    
**              float: v
**
**    num_surfaces of:
**      char[32]: name of surface.
**      int: flags for translucency, etc... SRF_* constants.
**      int: index of skin into the model's skin array.
**      int: width of skin texture
**      int: height of skin texture
**      int: number of uv frames stored for triangles using this surface.
**      
**      
**    num_sequences:  
**      see animSeq_t
**      
**      
**      
**      
**
*/


#define	SRF_NODRAW		    0x00000001
#define	SRF_TRANS33		    0x00000002
#define	SRF_TRANS66		    0x00000004
#define	SRF_ALPHA		    0x00000008
#define	SRF_GLOW		    0x00000010
#define	SRF_COLORBLEND	    0x00000020
#define	SRF_HARDPOINT	    0x00000040
#define	SRF_STATICSKIN	    0x00000080
#define SRF_FULLBRIGHT      0x00000100
#define SRF_ENVMAP          0x00000200  // env map texture only
#define SRF_TEXENVMAP_ADD   0x00000400  // base texture plus env map texture
#define SRF_TEXENVMAP_MULT  0x00000800  // base texture times env map texture

typedef struct
{
	char	name[32];		// surface name

	int		flags;			// surface flags (translucency, etc.)

	int		skinindex;		// base skin index
	int		skinwidth;		// width for this groups texture
	int		skinheight;		// height for this groups texture

	int		num_uvframes;	// Ash -- used for storing the hardpoint tri index for the surface
//	int		num_st;			// greater than num_xyz for seams				// do we need this? - no
//	int		num_xyz;		// number of vertices this surface contains		// do we need this? - no
//	int		num_tris;		// number of polys this surface contains		// do we need this? - no
} dsurface_t;

/*
========================================================================

.SP2 sprite file format

========================================================================
*/

#define IDSPRITEHEADER	(('2'<<24)+('S'<<16)+('D'<<8)+'I')
		// little-endian "IDS2"
#define SPRITE_VERSION	2

typedef struct
{
	int		width, height;
	int		origin_x, origin_y;		// raster coordinates inside pic
	char	name[MAX_SKINNAME];		// name of pcx file
} dsprframe_t;

typedef struct {
	int			ident;
	int			version;
	int			numframes;
	dsprframe_t	frames[1];			// variable sized
} dsprite_t;

/*
==============================================================================

  .WAL texture file format

==============================================================================
*/

//	.WAL flags

#define	WF_PROCEDURAL	0x00000001	//	procedural texture, don't acually load 
									//	the bitmap, but get the width & height
									//	and malloc up memory for it

//	New miptex's have a version char at the beginning
//	For now, when the WAL is loaded, the first char is checked.  If it is
//	greater than 32, then it is assumed this is an old WAL type, otherwise,
//	it is assumed that the first char is the version number and it is a 
//	new WAL type.

#define MIPTEX_VERSION	3
#define	MIPLEVELS_OLD	4
typedef struct miptexOld_s
{
	char		name[32];
	unsigned	width, height;
	unsigned	offsets[MIPLEVELS_OLD];		// four mip maps stored
	char		animname[32];			// next frame in animation chain
	int			flags;
	int			contents;
	int			value;
} miptexOld_t;

#define	MIPLEVELS	9
typedef struct miptex_s
{
	char		version;
	char		name[32];
	unsigned	width, height;
	unsigned	offsets[MIPLEVELS];		// 9 mip maps stored
	char		animname[32];			// next frame in animation chain
	int			flags;
	int			contents;
	byte		palette[768];
	int			value;
} miptex_t;


/*
==============================================================================

  .BSP file format

==============================================================================
*/

#define IDBSPHEADER	(('P'<<24)+('S'<<16)+('B'<<8)+'I')
		// little-endian "IBSP"

#define	BSPVERSION				41	//	planepolys lump (only adjusted so -planypolys BSP 
									//	switch can detect versions and update the header
									//	appropriately
#define BSPVERSION_COLORLUMP	40	//	color lump
#define BSPVERSION_TEXINFOOLD	39	//	color info in texinfo struct
#define	BSPVERSION_Q2			38	//	original Quake 2 format BSP


// upper design bounds
// leaffaces, leafbrushes, planes, and verts are still bounded by
// 16 bit short limits
#define	MAX_MAP_MODELS		1024
#define	MAX_MAP_BRUSHES		8192
#define	MAX_MAP_ENTITIES	2048
#define	MAX_MAP_ENTSTRING	0x40000
#define	MAX_MAP_TEXINFO		8192

#define	MAX_MAP_AREAS		256
#define	MAX_MAP_AREAPORTALS	1024
#define	MAX_MAP_PLANES		65536
#define	MAX_MAP_NODES		65536
#define	MAX_MAP_BRUSHSIDES	65536
#define	MAX_MAP_LEAFS		65536
#define	MAX_MAP_VERTS		65536
#define	MAX_MAP_FACES		65536
#define	MAX_MAP_LEAFFACES	65536
#define	MAX_MAP_LEAFBRUSHES 65536
#define	MAX_MAP_PORTALS		65536
#define	MAX_MAP_EDGES		128000
#define	MAX_MAP_SURFEDGES	256000
//	For development only!!  Change before release!
#define	MAX_MAP_LIGHTING	0x600000
//#define	MAX_MAP_LIGHTING	0x400000

//	For development only!!  Change before release!
#define	MAX_MAP_VISIBILITY	0x600000
//#define	MAX_MAP_VISIBILITY	0x400000
//#define	MAX_MAP_VISIBILITY	0x200000

#define MAX_MAP_SURFCOLORINFO 8192

// key / value pair sizes

#define	MAX_KEY		32
#define	MAX_VALUE	1024

//=============================================================================

typedef struct
{
	int		fileofs, filelen;
} lump_t;

#define	LUMP_ENTITIES		0
#define	LUMP_PLANES			1
#define	LUMP_VERTEXES		2
#define	LUMP_VISIBILITY		3
#define	LUMP_NODES			4
#define	LUMP_TEXINFO		5
#define	LUMP_FACES			6
#define	LUMP_LIGHTING		7
#define	LUMP_LEAFS			8
#define	LUMP_LEAFFACES		9
#define	LUMP_LEAFBRUSHES	10
#define	LUMP_EDGES			11
#define	LUMP_SURFEDGES		12
#define	LUMP_MODELS			13
#define	LUMP_BRUSHES		14
#define	LUMP_BRUSHSIDES		15
#define	LUMP_POP			16
#define	LUMP_AREAS			17
#define	LUMP_AREAPORTALS	18
#define	LUMP_EXTSURFINFO	19
#define	LUMP_PLANEPOLYS		20
#define	HEADER_LUMPS		21

typedef struct
{
	int			ident;
	int			version;	
	lump_t		lumps[HEADER_LUMPS];
} dheader_t;

typedef struct
{
	CVector		mins;
	CVector		maxs;
	CVector		origin;		// for sounds or lights
	int			headnode;
	int			firstface, numfaces;	// submodels just draw faces
										// without walking the bsp tree
} dmodel_t;


typedef struct
{
	float	point[3];
} dvertex_t;


// 0-2 are axial planes
#define	PLANE_X			0
#define	PLANE_Y			1
#define	PLANE_Z			2

// 3-5 are non-axial planes snapped to the nearest
#define	PLANE_ANYX		3
#define	PLANE_ANYY		4
#define	PLANE_ANYZ		5

// planes (x&~1) and (x&~1)+1 are allways opposites

typedef struct
{
	CVector	normal;
	float	dist;
	int		type;		// PLANE_X - PLANE_ANYZ ?remove? trivial to regenerate
} dplane_t;


// contents flags are seperate bits
// a given brush can contribute multiple content bits
// multiple brushes can be in a single leaf

// these definitions also need to be in q_shared.h!

// lower bits are stronger, and will eat weaker brushes completely
#define	CONTENTS_SOLID			0x00000001		// an eye is never valid in a solid
#define	CONTENTS_WINDOW			0x00000002		// translucent, but not watery
#define	CONTENTS_AUX			0x00000004
#define	CONTENTS_LAVA			0x00000008
#define	CONTENTS_SLIME			0x00000010
#define	CONTENTS_WATER			0x00000020
#define	CONTENTS_MIST			0x00000040
#define	CONTENTS_CLEAR			0x00000080
#define	CONTENTS_NOTSOLID		0x00000100
#define	CONTENTS_NOSHOOT		0x00000200
#define	CONTENTS_FOG			0x00000400
#define CONTENTS_NITRO			0x00000800	// SCG[4/21/99]: Added for freezing nitro
#define	LAST_VISIBLE_CONTENTS	0x00000800

// remaining contents are non-visible, and don't eat brushes

#define	CONTENTS_AREAPORTAL		0x00008000

#define	CONTENTS_PLAYERCLIP		0x00010000
#define	CONTENTS_MONSTERCLIP	0x00020000

// currents can be added to any other contents, and may be mixed
#define	CONTENTS_CURRENT_0		0x00040000
#define	CONTENTS_CURRENT_90		0x00080000
#define	CONTENTS_CURRENT_180	0x00100000
#define	CONTENTS_CURRENT_270	0x00200000
#define	CONTENTS_CURRENT_UP		0x00400000
#define	CONTENTS_CURRENT_DOWN	0x00800000

#define	CONTENTS_ORIGIN			0x01000000	// removed before bsping an entity

#define	CONTENTS_MONSTER		0x02000000	// should never be on a brush, only in game
#define	CONTENTS_DEADMONSTER	0x04000000
#define	CONTENTS_DETAIL			0x08000000	// brushes to be added after vis leafs
#define	CONTENTS_TRANSLUCENT	0x10000000	// auto set if any surface has trans
#define	CONTENTS_LADDER			0x20000000

//	Nelno:	FL_BOT considers this solid
#define	CONTENTS_NPCCLIP		0x40000000

typedef struct
{
	int			planenum;
	int			children[2];	// negative numbers are -(leafs+1), not nodes
	short		mins[3];		// for frustom culling
	short		maxs[3];
	unsigned short	firstface;
	unsigned short	numfaces;	// counting both sides
} dnode_t;

typedef struct texinfo_s
{
	float		vecs[2][4];		// [s/t][xyz offset]
	int			flags;			// miptex flags + overrides
	int			value;			// light emission, etc
	char		texture[32];	// texture name (textures/*.wal)
	int			nexttexinfo;	// for animations, -1 = end of chain
} texinfo_t;

// note that edge 0 is never used, because negative edge nums are used for
// counterclockwise use of the edge in a face
typedef struct
{
	unsigned short	v[2];		// vertex numbers
} dedge_t;

#define	MAXLIGHTMAPS	4
typedef struct
{
	unsigned short	planenum;
	short		side;

	int			firstedge;		// we must support > 64k edges
	short		numedges;	
	short		texinfo;

// lighting info
	byte		styles[MAXLIGHTMAPS];
	int			lightofs;		// start of [numstyles*surfsize] samples
} dface_t;

typedef struct
{
	int				contents;			// OR of all brushes (not needed?)

	short			cluster;
	short			area;

	short			mins[3];			// for frustum culling
	short			maxs[3];

	unsigned short	firstleafface;
	unsigned short	numleaffaces;

	unsigned short	firstleafbrush;
	unsigned short	numleafbrushes;

	int				brushnum;
} dleaf_t;

typedef struct
{
	unsigned short	planenum;		// facing out of the leaf
	short	texinfo;
} dbrushside_t;

typedef struct
{
	int			firstside;
	int			numsides;
	int			contents;
} dbrush_t;

#define	ANGLE_UP	-1
#define	ANGLE_DOWN	-2


// the visibility lump consists of a header with a count, then
// byte offsets for the PVS and PHS of each cluster, then the raw
// compressed bit vectors
#define	DVIS_PVS	0
#define	DVIS_PHS	1
typedef struct
{
	int			numclusters;
	int			bitofs[8][2];	// bitofs[numclusters][2]
} dvis_t;

// each area has a list of portals that lead into other areas
// when portals are closed, other areas may not be visible or
// hearable even if the vis info says that it should be
typedef struct
{
	int		portalnum;
	int		otherarea;
} dareaportal_t;

typedef struct
{
	int		numareaportals;
	int		firstareaportal;
} darea_t;

typedef struct
{
	CVector	color;
} extsurfinfo_t;

typedef	struct	frameData_s
{
    char    animation_name[16]; // the name of the animation (ataka, amba, etc)
	int		first;              // first frame in this sequence
	int		last;               // last frame in this sequence
	char	*sound1;            // sound to play during sequence
	int		soundframe1;        // frame to play sound on
	char	*sound2;            // second sound to play during sequence
	int		soundframe2;        // frame to play second sound on
	unsigned long int flags;    // flags this sequence starts with (FRAME_LOOP, FRAME_ONCE, etc.)
	int		nAttackFrame1;      // frame at the actual attack needs to happen
	int		nAttackFrame2;      // frame at the actual attack needs to happen
	float   fModelAnimSpeed;    //  Ash -- speed unscaled model should move for animation to not skate
	float   fAnimChance;        // chance of playing this animation vs. selecting alternate
	float   fSound2Chance;      // chance of playing sound2 instead of sound1
} frameData_t;

#endif
