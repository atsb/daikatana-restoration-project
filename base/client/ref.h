#ifndef _ref_h
#define _ref_h

#include "qcommon.h"
#include "cmodel.h"

typedef enum {
	RESOURCE_GLOBAL,
	RESOURCE_EPISODE,
	RESOURCE_LEVEL,
	RESOURCE_NPC_SKIN,
	RESOURCE_INTERFACE
} resource_t;

// complex particle flags 

#define PF_DEPTHHACK    0x0001
#define PF_ADDSPLASH	0x0002
#define	PF_SCALEVEL		0x0004
#define	PF_RAIN_NORTH	0x0008
#define	PF_RAIN_SOUTH	0x0010
#define	PF_RAIN_EAST	0x0020
#define	PF_RAIN_WEST	0x0040

//	surface sprite flags
#define	SSF_LIGHTBEAM		0x01
#define	SSF_LOCALORIGIN		0x02	//	origin coords are local to a model
									//	and not in world coordinates
#define	SSF_FULLBRIGHT		0x04
#define	SSF_QUICKFADE		0x08

#define	MAX_DLIGHTS			32
#define	MAX_ENTITIES		1024
#define	MAX_PARTICLES		4096
#define	MAX_DHPARTICLES		1024
//#define	MAX_PARTICLES	2048
#define	MAX_LIGHTSTYLES		256
#define MAX_FLARES			32				//  Shawn:  Added for Daikatana
#define MAX_PVOLUMES		48				//  Shawn:  Added for Daikatana
#define MAX_SURFACES		16				//  Shawn:  Added for Daikatana

#define POWERSUIT_SCALE		4.0F

#define SHELL_RED_COLOR		0xF2
#define SHELL_GREEN_COLOR	0xD0
#define SHELL_BLUE_COLOR	0xF3

#define SHELL_RG_COLOR		0xDC
#define SHELL_RB_COLOR		0x86
#define SHELL_BG_COLOR		0x78

#define SHELL_WHITE_COLOR	0xD7

#define ONEDIV255			0.003921

#define DSFLAG_RESOURCE		0x00000001		// use resource member
#define DSFLAG_SCALE		0x00000002		// use fScaleX, and fScaleY members
#define DSFLAG_COLOR		0x00000004		// use rgbColor field
#define DSFLAG_ALPHA		0x00000008		// use blending
#define DSFLAG_BLEND		0x00000010		// use fAlpha member
#define DSFLAG_PALETTE		0x00000020		// set the palette to the image palette
#define DSFLAG_SUBIMAGE		0x00000040		// only use part of the image
#define DSFLAG_FLAT			0x00000080		// flat shaded poly
#define DSFLAG_FLIPX		0x00000100		// SCG[1/13/00]: mirror image in x
#define DSFLAG_FLIPY		0x00000200		// SCG[1/13/00]: mirror image in y
#define DSFLAG_NOMIP		0x00000400		// SCG[3/14/00]: No mipmapping

typedef struct __DRAWSTRUCT
{
	int		nFlags;			// flags for drawing options
	int		dwResource;		// resource flags (do we need this?)

	float	fScaleX;		// x scale for drawing stretch pics
	float	fScaleY;		// y scale for drawing stretch pics

	float	fAlpha;			// image alpha level

// draw coordinates
	int		nXPos;
	int		nYPos;	

// image coordinates only used if DSFLAG_SUBIMAGE is set
// SCG[3/30/99]: Also used for x, y, w, h flor flat shaded poly
	int		nLeft; 
	int		nTop; 
	int		nRight; 
	int		nBottom; 

	void	*pImage;		// the image data

	CVector	rgbColor;		// color data for adding color

	const char		*szString;

} DRAWSTRUCT, *LPDRAWSTRUCT;

typedef struct particle_def_s
{
	float	s1, t1;
	float	s2, t2;
	float	fScale;
} particle_def_t;
typedef struct entity_s
{
	void			*model;			// opaque type outside refresh
	CVector			angles;

	/*
	** most recent data
	*/
	CVector			origin;			// also used as RF_BEAM's "from"
	int				frame;			// also used as RF_BEAM's diameter

	/*
	** previous data for lerping
	*/
	CVector			oldorigin;		// also used as RF_BEAM's "to"
	int				oldframe;

	/*
	** misc
	*/
	float			backlerp;		// 0.0 = current, 1.0 = old
	int				skinnum;		// also used as RF_BEAM's palette index

///	int				lightstyle;		// for flashing entities	// amw: unused....
	float			alpha;			// ignore if RF_TRANSLUCENT isn't set

	void			*skin;			// NULL for inline skin
	int				flags;

	CVector			render_scale;
	CVector			mins, maxs;		//  added for particle volumes
//	float			color;			// amw: I couldn't find where this was being used so I commented it out
	
	/*
	** cluster animation data
	*/
	clusterDef_t	*pDef;			// ptr to cluster definition data (in drawing order)
	int				count;			// number of clusters

	CVector			color;			// SCG[8/11/99]: needed for effects
} entity_t;

#define ENTITY_FLAGS  68

typedef struct dlight_s
{
	CVector origin;
	CVector color;
	float	intensity;
} dlight_t;

//  Shawn:  Added for Daikatana
typedef struct flare_s
{
	CVector	origin;
	CVector	angles;
	CVector	color;
	float	intensity;
	float	falloff;
} flare_t;

typedef struct foginfo_s
{
	float	r, g, b;
	float	start, end, skyend;
	int		active;
	int		inc;
} foginfo_t;

typedef struct ref_particle_s
{
	CVector			origin;
	float			alpha;
	particle_type_t	type;			//	Nelno:	added for particle effects
// SCG[6/17/99]: not used
//	int				rotation_vec;	//	Nelno:	nubmer of rotation vector for this particle
	float			pscale;			//	Nelno:	size scale for this particle
	short			flags;
	int				color;
	CVector			color_rgb;
} particle_t;

// --- amw: this structure contains all client side info necessary for fast particle volumes

typedef struct particleVolume_s
{
	int		bVisible;		// currently visible
	CVector	mins,maxs;		// bounding box of particle volume
	CVector	corners[4];		// the bounding corners with z == 0
	int		type;			// type of particle volume
	int		height;			// height of particle volume (to ground)
	int		effects;		// effects flags     						   
	int		effects2;		// effects flags 2
	float	dist;			// current distance from viewer
//	float	alpha;			// alpha value		{	NOT USED AT THIS TIME  }
} particleVolume_t;

typedef struct lightstyle_s
{
	CVector		rgb;			// 0.0 - 2.0
	float		white;			// highest of rgb
} lightstyle_t;

///////////////////////////////////////////////////////////////////////////////
//	beamList types
///////////////////////////////////////////////////////////////////////////////

#define	BEAM_MAX_SEGMENTS	4096	//	maximum segments of beams
#define	BEAM_MAX_BEAMS		512		//	total number of beams that can display

//	flags for beams
#define	BEAMFLAG_GLOBALCOLOR	0x0001	//	use beam->rgbColor [0] as global color index			
										//	this along with texIndex >= 0 can be used to 
										//	add a global color on top of texture from texIndex
#define	BEAMFLAG_ALPHAFADE		0x0002	//	beam alpha will fade to 0 towards end point
										//	value of beam->rgbColor is blended with beam, use
										//	{1.0, 1.0, 1.0} for straight texture color
#define	BEAMFLAG_RGBCOLOR		0x0004	//	apply rgbColor to texture (not necessary if 
										//	BEAM_TEX_NONE is used because rgbColor will be
										//	used by default

enum
{
	BEAM_TEX_NONE = -1,
	BEAM_TEX_LIGHTNING = 0,
	BEAM_TEX_LASER,
    BEAM_TEX_LASERSPARK,
	BEAM_TEX_NOVALASER,
	BEAM_TEX_TRACER,
   BEAM_TEX_MF_GENERIC,
   BEAM_TEX_GRAPPLE_CHAIN,
   BEAM_TEX_DISCUSTRAIL,
	BEAM_TEX_NUMTEXTURES
};
									//	per frame
typedef	struct	beamSeg_s
{
	CVector	segStart;	//	starting point of this segment
						//	should always be the previous segStart + segNormal
	CVector	segNormal;	//	normal of this segment
	CVector	segEnd;		//	ending point of this segment

    float alpha;        //adam: alpha value for this segment.

	struct	beamSeg_s	*nextSeg;	//	index of next segment in segList
} beamSeg_t;

typedef	struct	beam_s
{
	beamSeg_t	*firstSeg;
	float		alpha;
	float		startRadius;		//	beam width in texels at start
	float		endRadius;			//	beam width in texels at end
	int			texIndex;			//	index in r_beamTextures in ref
	int			texIndex2;			//	overlaid texture index -- not used yet
	CVector		rgbColor;			//	red, green, blue value, used if texIndex < 0

	CVector		beamStart;
	CVector		beamEnd;
	float		beamLength;
	
	int			numSegs;

	unsigned	short	flags;		//	flags for this beam
} beam_t;

typedef	struct	beamList_s
{
	beam_t			beams [BEAM_MAX_BEAMS];	//	pointers to first seg in each beam
	int				numBeams;
	int				numSegs;

//	these are only used if we're doing per-frame initialization of all beams...
//	otherwise, beam_AllocBeam and beam_AllocSeg need to be changed so that
//	they find free beams and segs instead of just choosing the next beam in
//	the list
	int				freeBeam;
	int				freeSeg;
} beamList_t;

//	end of beamList types

///////////////////////////////////////////////////////////////////////////////
//	inventory types
///////////////////////////////////////////////////////////////////////////////

typedef struct invenIcon_s
{
	void		*model;
	char		modelname[MAX_QPATH];
	int			modelindex;
	int			count;		//	for displaying ammo counts, etc.
	float		alpha;
	CVector		render_scale;
} invenIcon_t;




#define MAX_ONSCREEN_ICONS    (5)

#define INVMODE_OFF           (0)
#define INVMODE_WEAPON_ICONS  (1)
#define INVMODE_ITEM_ICONS    (2)
#define INVMODE_SIDEKICK      (3)



// 3.1  dsn
typedef struct onscreen_icons_s
{
  void    *models[2];
	qboolean blink;
  int      blink_time;
  float    alpha;
} onscreen_icons_t;

typedef struct onscreen_icons_info_s      
{
  onscreen_icons_t icons[MAX_ONSCREEN_ICONS];
  int              num_icons;
  
} onscreen_icons_info_t;


// SCG[3/29/99]: Added for entity sorting
typedef struct sortentity_s
{
	entity_t	ent;
	float		distance;
} sortentity_t;

typedef struct refdef_s
{
	int					x, y, width, height;// in virtual screen coordinates
	float				fov_x, fov_y;
	CVector				vieworg;
	CVector				viewangles;
	float				blend[4];			// rgba 0-1 full screen blend
	float				cine_blend[4];			// rgba 0-1 full screen blend
	float				time;				// time is uesed to auto animate
	int					rdflags;			// RDF_UNDERWATER, etc
						
	byte				*areabits;			// if not NULL, only areas with set bits will be drawn
						
	lightstyle_t		*lightstyles;	// [MAX_LIGHTSTYLES]
						
	int					num_entities;
	entity_t			*entities;
						
	int					num_alphaentities;
	sortentity_t		*alphaentities;
						
	int					num_dlights;
	dlight_t			*dlights;
						
	int					num_flares;
	flare_t				*flares;

	int					num_particleVolumes;
	particleVolume_t	*particleVolumes;	

	int					num_particles;
	particle_t			*particles;

	int					num_comparticles;
	particle_t			*comparticles;

	int					num_dhcomparticles;
	particle_t			*dhcomparticles;

	foginfo_t			foginfo;

	beamList_t			*beamList;

	invenIcon_t *inventoryIcons;
	int					 numInventoryIcons;
  int          inventory_mode;
  int          inventory_selected;
  float        inventory_alpha;

  onscreen_icons_info_t onscreen_icons_info;

	int					episodeNum;		//	used to index into so texture arrays

} refdef_t;


//	alpha character drawing routine flags
#define	ACF_BEGINFLARE	0x00000001	//	text flares into existence
#define	ACF_ENDFLARE	0x00000002	//	text flares out of existence -- NOT SUPPORTED YET
#define	ACF_BEGINFADE	0x00000004	//	text fades in
#define	ACF_ENDFADE		0x00000008	//	text fades out
#define	ACF_TOPALPHA	0x00000010	//	text fades out towards top of letters -- NOT SUPPORTED YET
#define	ACF_BOTTOMALPHA	0x00000020	//	text fades out towards bottom of letters -- NOT SUPPORTED YET

//	hierarchical surface stuff
#define	MAX_SURFACE_NAMES	16

#define	SRFL_VISIBLE	0x00000001
//	end hierarchical surface stuff

#define	API_VERSION		3

//
// these are the functions exported by the refresh module
//
typedef struct refexport_s
{
	// if api_version is different, the dll cannot be used
	int		api_version;

	// called when the library is loaded
	qboolean	(*Init) ( void *hinstance, void *wndproc );

	// called before the library is unloaded
	void	(*Shutdown)();

	// All data that will be used in a level should be
	// registered before rendering any frames to prevent disk hits,
	// but they can still be registered at a later time
	// if necessary.
	//
	// EndRegistration will free any remaining data that wasn't registered.
	// Any model_s or skin_s pointers from before the BeginRegistration
	// are no longer valid after EndRegistration.
	//
	// Skins and images need to be differentiated, because skins
	// are flood filled to eliminate mip map edge errors, and pics have
	// an implicit "pics/" prepended to the name. (a pic name that starts with a
	// slash will not use the "pics/" prefix or the ".pcx" postfix)
	void	(*PurgeResources)(resource_t resource);
	void	(*BeginRegistration) (const char *map);
	void	(*Mod_Free) (void *mod);
    //adam: these functions used to return structures, however ref_soft and ref_gl returned different
    //structures, so the return value was basically useless.  They return void * now.
	void	*(*RegisterModel) ( const char *name, resource_t resource );
	void	*(*RegisterSkin)( const char *name, resource_t resource );
	void	*(*RegisterPic)( const char *name, int *pWidth, int *pHeight, resource_t resource );
	void	(*SetSky) (const char *name, const char *cloudname, int skynum);
	void	(*SetPaletteDir) (const char *name);
	
	void	(*EndRegistration)();

	void	(*RenderFrame) (refdef_t *fd);

//==================================
// consolidation change: SCG 3-11-99
	void	(*DrawPic)( DRAWSTRUCT& drawStruct );
// consolidation change: SCG 3-11-99
//==================================
	void	(*DrawChar) (int x, int y, int c);
	void	(*DrawTileClear) (int x, int y, int w, int h, const char *name);
	void	(*DrawFill)( int x, int y, int w, int h, CVector rgbColor, float alpha );

	// Draw images for cinematic rendering (which can have a different palette). Note that calls
//	void	(*DrawStretchRaw) (int x, int y, int w, int h, int cols, int rows, byte *data);

	/*
	** video mode and refresh state management entry points
	*/
	void	(*CinematicSetPalette)( const unsigned char *palette);	// NULL = game palette
	void	(*SetInterfacePalette)( const unsigned char *palette);	// NULL = game palette
	void	(*BeginFrame)( float camera_separation );
	void	(*EndFrame)();

	void	(*AppActivate)( qboolean activate );

	//	Nelno:	added these for per-level console background and chars
	void	(*SetResourceDir) (char *name);
//	void	(*DrawConsolePic) (int x, int y, int w, int h, int episode_num, int map_num);

	//  Shawn:  added this for getting frame name
	void	(*GetFrameName) (char *modelname, int frameindex, char *framename);

	//  andrew: added this to get the skins currently used on a model
	int		(*GetModelSkinIndex) (void *modelPtr);
	char*	(*GetModelSkinName)	(void *modelPtr);

	// andrew: get a hardpoint from a model
	void	(*GetModelHardpoint)(char *pSurfName, int curFrame, int lastFrame, entity_t &ent, CVector &hardPt);

	//	registers new skins for model
	void	(*RegisterEpisodeSkins) (void *model, int episode, int flags);
	
	// palette operations
	unsigned char (*BestColor) (byte r, byte g, byte b, unsigned *palette);

	void	(*AddSurfaceSprite)( CVector& vOrigin, CVector& vEntOrigin, CVector& vEntAngles, 
						 void *pHitModel, int nIndex, void *pSpriteModel, int nFrame, 
						 float fRoll, float fScale, byte nFlags );

    //new string drawing routines.
    void	*(*LoadFont)(const char *name);
//==================================
// consolidation change: SCG 3-11-99
	int		(*DrawString)( DRAWSTRUCT& drawStruct );
	int		(*DrawStringFlare)( DRAWSTRUCT& drawStruct );
// consolidation change: SCG 3-11-99
//==================================
    int		(*StringWidth)(const char *string, void *font, int max_chars_counted);
    int		(*FontHeight)(void *font);
    int		(*FontNumCharsInWidth)(const char *string, void *font, int allowed_pixel_width, int start_pos);
	void	(*FontGetFontData) (void *vfont, int *height, byte **charWidth, byte **charX, byte **charY, void **image);

	void	*(*GetHwnd)();

	void	(*GetSavegameScreenShot)();
	void	(*ClearSavegameScreenShot)();
	void	(*SaveSavegameScreenShot)( char *path, char *savename );
} refexport_t;

//
// these are the functions imported by the refresh module
//
typedef struct refimport_s
{
	void	(*Sys_Error) (int err_level, char *str, ...);
	void	(*Sys_Warning) (char *msg, ...);

	void	(*Cmd_AddCommand) (char *name, void(*cmd)());
	void	(*Cmd_RemoveCommand) (char *name);
	int		(*GetArgc)();
	char	*(*GetArgv) (int i);
	void	(*Cmd_ExecuteText) (int exec_when, char *text);

	void	(*Con_Printf) (int print_level, char *str, ...);

	// files will be memory mapped read only
	// the returned buffer may be part of a larger pak file,
	// or a discrete file from anywhere in the quake search path
	// a -1 return means the file does not exist
	// NULL can be passed for buf to just determine existance
	int		(*FS_LoadFile) (const char *name, void **buf);
	void	(*FS_FreeFile) (void *buf);

	//	the other PAK File IO routines
	int		(*FS_FileLength)	(FILE *F);
	void	(*FS_Close)			(FILE *F);
	int		(*FS_Open)			(const char *filename, FILE **file);
	void	(*FS_Read)			(void *buffer, int len, FILE *f);
	int		(*FS_Seek)			(FILE *f, long offset, int origin);
	long	(*FS_Tell)			(FILE *f);
	int		(*FS_Getc)			(FILE *f);
	char*	(*FS_Gets)			(char *string, int n, FILE *f);
	long	(*FS_GetBytesRead)	();

    // gamedir will be the current directory that generated
	// files should be stored to, ie: "f:\quake\id1"
	char	*(*FS_Gamedir)();

	cvar_t	*(*Cvar_Get) (const char *name, const char *value, int flags);
	cvar_t	*(*Cvar_Set)(const char *name, const char *value );
	void	 (*Cvar_SetValue)(const char *name, float value );

	qboolean	(*Vid_GetModeInfo)( int *width, int *height, int mode );
	void		(*Vid_MenuInit)();
	void		(*Vid_NewWindow)( int width, int height );

	void		(*CL_GenerateVolumeParticles) (int key, int type, CVector &mins, CVector &maxs, float dist, int flags);
	void		(*CL_RemoveParticleVolume) (int key);

	void		(*dk_printxy) (int x, int y, char *fmt, ...);

	beam_t		*(*beam_AllocBeam) (void);
	beamSeg_t	*(*beam_AllocSeg) (void);

	bspModel_t	*bspModel;

	int		(*clhr_IndexForSurfaceName) (char *surfName);
	char	*(*clhr_SurfaceNameForIndex) (int index);
	int		(*clhr_FlagsForSurfaceIndex) (int index);
	int		(*clhr_FlagsForSurfaceName) (char *surfName);

#ifdef DEBUG_MEMORY
	void*	(*X_Malloc) (size_t size, MEM_TAG tag, char* file, int line);
#else
	void*	(*X_Malloc) (size_t size, MEM_TAG tag);
#endif
	void	(*X_Free) (void* mem);
	int		(*Mem_Heap_Walk) (int stats);
} refimport_t;


// this is the only function actually exported at the linker level
typedef	refexport_t	(*GetRefAPI_t) (refimport_t);


#endif  //#ifndef _ref_h
