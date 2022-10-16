// disable data conversion warnings

#pragma warning(disable : 4244)     // MIPS
#pragma warning(disable : 4136)     // X86
#pragma warning(disable : 4051)     // ALPHA

#ifdef _WIN32
#  include <windows.h>
#endif

#include <stdio.h>

#include <gl\gl.h>
#include <gl\glu.h>
#include <math.h>
#include "collect.h"

#include "ref.h"

#include "qgl.h"

#define	CONSOLIDATE_BSP	//	use BSP data from daikatana.exe!!

#define	REF_VERSION	"GL 0.01"

// up / down
#define	PITCH	0

// left / right
#define	YAW		1

// fall over
#define	ROLL	2

// maximum number of lightmaps in a bsp
#define	MAX_LIGHTMAPS	128

typedef struct
{
	unsigned		width, height;			// coordinates from main game
} viddef_t;

//#ifdef __cplusplus
//extern "C" {
//#endif

extern	viddef_t	vid;

extern	int			g_PVCullCount;
extern	int			g_PVDrawCount;
extern	int			g_currentTexPalette;

/*

  skins will be outline flood filled and mip mapped
  pics and sprites with alpha will be outline flood filled
  pic won't be mip mapped

  model skin
  sprite frame
  wall texture
  pic

*/

typedef enum 
{
	it_skin,
	it_sprite,
	it_wall,
	it_pic,
	it_sky,
	it_procedural	//	doesn't acually load any data in GL_LoadPic
} imagetype_t;

typedef struct image_s
{
	char	name[MAX_QPATH];			// game path, including extension
	imagetype_t	type;
	int		width, height;				// source image
	int		upload_width, upload_height;	// after power of two and picmip
	int		registration_sequence;		// 0 = free
	struct msurface_s	*texturechain;	// for sort-by-texture world drawing
	int		texnum;						// gl texture binding
	float	sl, tl, sh, th;				// 0,0 - 1,1 unless part of the scrap
	qboolean	scrap;
	qboolean	has_alpha;

	qboolean paletted;

	resource_t	resource;	        	//	holds resource types
	int			pt_type;	            //	procedural type number if it_procedural
    unsigned char gl_palette[768];      // Ash -- palette in form gl needs
} image_t;

#define	TEXNUM_LIGHTMAPS	1024
#define	TEXNUM_SCRAPS		1152
#define	TEXNUM_IMAGES		1153

#define		MAX_GLTEXTURES	1024

//===================================================================

typedef enum
{
	rserr_ok,

	rserr_invalid_fullscreen,
	rserr_invalid_mode,

	rserr_unknown
} rserr_t;

#include "gl_model.h"

void GL_BeginRendering (int *x, int *y, int *width, int *height);
void GL_EndRendering (void);

void GL_SetDefaultState( void );
void GL_UpdateSwapInterval( void );

extern	float	gldepthmin, gldepthmax;

typedef struct
{
	float	x, y, z;
	float	s, t;
	float	r, g, b;
} glvert_t;


#define	MAX_LBM_HEIGHT		480

#define BACKFACE_EPSILON	0.01


//====================================================

//	Nelno:	for doing time-based stuff in the render dll
extern	int		ref_curTime;
extern	int		ref_lastTime;
extern	float	ref_frameTime;
extern	float	ref_globalTime;	//	total time passed since R_Init, in seconds

extern	image_t		gltextures[MAX_GLTEXTURES];
extern	int			numgltextures;


extern	image_t		*r_notexture;

//extern	image_t		*r_particletexture [NUM_PARTICLETYPES];
extern	image_t		*r_particletexture;
extern	image_t		*r_lasertexture,*r_novalasertexture,*r_tracertexture,*r_gunflaretexture;
extern   image_t		*r_lightningtexture1,*r_lightningtexture2,*r_lightningtexture3;

extern	entity_t	*currententity;
extern	model_t		*currentmodel;
extern	int			r_visframecount;
extern	int			r_framecount;
extern	mplane_t	frustum[4];
extern	int			c_brush_polys, c_alias_polys, c_lightmap_polys, c_damage_polys;
extern	int			c_wal_memory, c_skin_memory, c_pic_memory, c_sky_memory, c_sprite_memory;

extern	int			gl_filter_min, gl_filter_max;

//
// view origin
//
extern	CVector	vup;
extern	CVector	vpn;
extern	CVector	vright;
extern	CVector	r_origin;

//
// screen size info
//
extern	refdef_t	r_newrefdef;
extern	int		r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;

extern	cvar_t	*r_countpvolumes;	//	Nelson
extern	cvar_t	*r_countsnow;		//	Nelson
extern	cvar_t	*r_countents;		//	Nelson
extern	cvar_t	*r_drawbmodels;		//	Nelson
extern	cvar_t	*r_drawalias;		//	Nelson
extern	cvar_t	*r_drawsprites;		//	Nelson
//extern	cvar_t	*r_drawweapon;		//	Nelson
extern	int		ent_draw_count;

extern	cvar_t	*r_norefresh;
//extern	cvar_t	*r_lefthand;

extern  cvar_t  *weapon_visible;

extern	cvar_t	*r_drawentities;
extern	cvar_t	*r_drawworld;
extern	cvar_t	*r_speeds;
extern	cvar_t	*r_fullbright;
extern	cvar_t	*r_novis;
extern	cvar_t	*r_nocull;
extern	cvar_t	*r_lerpmodels;

extern	cvar_t	*r_lightlevel;	// FIXME: This is a HACK to get the client's light level

//extern  cvar_t  *console_screen_idx; // 12.21  dsn

extern cvar_t	*gl_ext_swapinterval;
extern cvar_t	*gl_ext_gamma;
extern cvar_t	*gl_ext_multitexture;
extern cvar_t	*gl_ext_pointparameters;

extern cvar_t	*gl_vertex_arrays;	// SCG[5/20/99]: Quake2 V3.19 addition

extern cvar_t	*gl_particle_min_size;
extern cvar_t	*gl_particle_max_size;
extern cvar_t	*gl_particle_size;
extern cvar_t	*gl_particle_att_a;
extern cvar_t	*gl_particle_att_b;
extern cvar_t	*gl_particle_att_c;

extern	cvar_t	*gl_nosubimage;
extern	cvar_t	*gl_bitdepth;
extern	cvar_t	*gl_mode;
extern	cvar_t	*gl_log;
extern	cvar_t	*gl_lightmap;
extern	cvar_t	*gl_shadows;
extern	cvar_t	*gl_dynamic;
extern  cvar_t  *gl_monolightmap;
extern	cvar_t	*gl_nobind;
extern	cvar_t	*gl_round_down;
extern	cvar_t	*gl_picmip;
extern	cvar_t	*gl_skymip;
extern	cvar_t	*gl_showtris;
extern	cvar_t	*gl_finish;
extern	cvar_t	*gl_ztrick;
extern	cvar_t	*gl_clear;
extern	cvar_t	*gl_cull;
extern	cvar_t	*gl_poly;
extern	cvar_t	*gl_texsort;
extern	cvar_t	*gl_polyblend;
extern	cvar_t	*gl_flashblend;
extern	cvar_t	*gl_lightmaptype;
extern	cvar_t	*gl_modulate;
extern	cvar_t	*gl_playermip;
extern	cvar_t	*gl_drawbuffer;
extern	cvar_t	*gl_3dlabs_broken;
extern  cvar_t  *gl_driver;
extern	cvar_t	*gl_swapinterval;
extern	cvar_t	*gl_texturemode;
extern  cvar_t  *gl_saturatelighting;
extern  cvar_t  *gl_lockpvs;
extern	cvar_t	*gl_drawflat;
extern  cvar_t  *gl_polylines;
extern  cvar_t  *gl_vfog;
extern  cvar_t  *gl_vfogents;
extern  cvar_t  *gl_vertnormals;
extern  cvar_t  *gl_envmap;
extern  cvar_t  *gl_texenvmap;
extern  cvar_t  *gl_texenvcolor;
extern  cvar_t  *gl_predator;
extern	cvar_t  *gl_midtextures;
extern	cvar_t  *gl_surfacesprites;
extern	cvar_t  *gl_alphasurfaces;
extern	cvar_t  *gl_fogsurfaces;
extern	cvar_t  *gl_procshell;
extern	cvar_t  *gl_procp;
extern	cvar_t  *gl_procm;
extern	cvar_t  *gl_procd;
extern	cvar_t  *gl_procn;
extern	cvar_t  *gl_bbox;
extern	cvar_t	*gl_texturealphamode;	// SCG[5/20/99]: Quake2 V3.19 addition
extern	cvar_t	*gl_texturesolidmode;	// SCG[5/20/99]: Quake2 V3.19 addition
extern  cvar_t  *gl_lightningfreq;

extern  cvar_t  *gl_skyname;
extern  cvar_t  *gl_cloudname;
extern  cvar_t  *gl_cloudxdir;
extern  cvar_t  *gl_cloudydir;
extern  cvar_t  *gl_cloud1tile;
extern  cvar_t  *gl_cloud1speed;
extern  cvar_t  *gl_cloud2tile;
extern  cvar_t  *gl_cloud2speed;
extern  cvar_t  *gl_cloud2alpha;

extern  cvar_t  *gl_skyname_2;
extern  cvar_t  *gl_cloudname_2;
extern  cvar_t  *gl_lightningfreq_2;
extern  cvar_t  *gl_cloudxdir_2;
extern  cvar_t  *gl_cloudydir_2;
extern  cvar_t  *gl_cloud1tile_2;
extern  cvar_t  *gl_cloud1speed_2;
extern  cvar_t  *gl_cloud2tile_2;
extern  cvar_t  *gl_cloud2speed_2;
extern  cvar_t  *gl_cloud2alpha_2;

extern  cvar_t  *gl_skyname_3;
extern  cvar_t  *gl_cloudname_3;
extern  cvar_t  *gl_lightningfreq_3;
extern  cvar_t  *gl_cloudxdir_3;
extern  cvar_t  *gl_cloudydir_3;
extern  cvar_t  *gl_cloud1tile_3;
extern  cvar_t  *gl_cloud1speed_3;
extern  cvar_t  *gl_cloud2tile_3;
extern  cvar_t  *gl_cloud2speed_3;
extern  cvar_t  *gl_cloud2alpha_3;

extern  cvar_t  *gl_skyname_4;
extern  cvar_t  *gl_cloudname_4;
extern  cvar_t  *gl_lightningfreq_4;
extern  cvar_t  *gl_cloudxdir_4;
extern  cvar_t  *gl_cloudydir_4;
extern  cvar_t  *gl_cloud1tile_4;
extern  cvar_t  *gl_cloud1speed_4;
extern  cvar_t  *gl_cloud2tile_4;
extern  cvar_t  *gl_cloud2speed_4;
extern  cvar_t  *gl_cloud2alpha_4;

extern  cvar_t  *gl_skyname_5;
extern  cvar_t  *gl_cloudname_5;
extern  cvar_t  *gl_lightningfreq_5;
extern  cvar_t  *gl_cloudxdir_5;
extern  cvar_t  *gl_cloudydir_5;
extern  cvar_t  *gl_cloud1tile_5;
extern  cvar_t  *gl_cloud1speed_5;
extern  cvar_t  *gl_cloud2tile_5;
extern  cvar_t  *gl_cloud2speed_5;
extern  cvar_t  *gl_cloud2alpha_5;

extern  cvar_t  *gl_brute_force_state;
extern  cvar_t  *gl_24bittextures;
extern  cvar_t  *gl_32bitatextures;

extern	cvar_t	*vid_fullscreen;
extern	cvar_t	*vid_gamma;

extern	cvar_t	*r_palettedir;			//	Shawn:  added for palette per level
extern	cvar_t	*r_drawparticleflag;	//	Shawn:  added for testing particle speed
extern	cvar_t	*r_drawcomparticles;
extern	cvar_t	*r_useimagelog;			//  Shawn:  added for image log 	
extern	cvar_t	*r_resourcedir;			//	Nelno:	added for per-level console backgrounds/characters
extern	cvar_t	*gl_mem;				//  Shawn:  added for texture memory usage

extern	cvar_t	*intensity;

extern	cvar_t	*r_fogcolor;
extern	cvar_t	*r_testfog;
extern	cvar_t	*r_fognear;
extern	cvar_t	*r_fogfar;
extern	cvar_t	*r_skyfogfar;

extern	cvar_t	*gl_clip_surface_sprites;

extern	cvar_t	*gl_subdivide_size;

extern	cvar_t	*gl_no_error_check;

extern	int		gl_lightmap_format;
extern	int		gl_solid_format;
extern	int		gl_alpha_format;
extern	int		gl_tex_solid_format;
extern	int		gl_tex_alpha_format;

extern	int		c_visible_lightmaps;
extern	int		c_visible_lightmap_bytes;
extern  byte	c_lightmap_list[MAX_LIGHTMAPS];
extern	int		c_visible_textures;
extern  int		c_visible_texture_bytes;
extern	byte	c_texture_list[MAX_GLTEXTURES];

extern	float	r_world_matrix[16];

void R_TranslatePlayerSkin (int playernum);
void GL_Bind (int texnum);
void GL_MBind( GLenum target, int texnum );
void GL_TexEnv( GLenum value );
void GL_MTexEnv( GLenum target, GLenum value );
void GL_EnableMultitexture( qboolean enable );
void GL_SelectTexture( GLenum );
void	GL_FreeImage (image_t *image);
//void R_PurgeResources(resource_t resource);	// moved to dk_ref_local - SCG: 3-10-99

bool R_LightPoint (CVector &p, CVector &color, CVector *pDLoc=NULL, msurface_t *pPointSurf = NULL );
void R_PushDlights (void);

//====================================================================

extern	model_t	*r_worldmodel;

extern	unsigned	d_8to24table[256];
extern  unsigned char d_gl_8to24table[768];
extern	unsigned	currentpalette[256];

extern	int		registration_sequence;


void V_AddBlend (float r, float g, float b, float a, float *v_blend);

int 	R_Init( void *hinstance, void *hWnd );
void	R_Shutdown( void );

void R_RenderView (refdef_t *fd);
void GL_ScreenShot_f (void);

void R_DrawAliasModel (entity_t *e);

void R_DrawBrushModel (entity_t *e);
void R_DrawSpriteModel (entity_t *e);
void R_DrawBeam( entity_t *e );
void R_DrawSpotlight( entity_t *e );
void R_DrawWorld (void);
void R_RenderDlights (void);
void R_DrawAlphaSurfaces (void);
void R_DrawMidTextures (void);
void R_DrawFullbrightSurfaces(void);
void R_RenderBrushPoly( msurface_t *pSurf );
void R_InitParticles();// SCG[6/18/99]: 
void R_InitNoTexture();// SCG[6/18/99]: 

void R_InitMiscTextures (void);	//	Nelno
void Draw_InitLocal (void);

void GL_SubdivideSurface( msurface_t *pSurf );
qboolean R_CullBox (CVector &mins, CVector &maxs);
void R_RotateForEntity (entity_t *e);
void R_MarkLeaves (void);

glpoly_t *AllocGLPoly( int numverts );

glpoly_t *WaterWarpPolyVerts (glpoly_t *p);
void R_AddSkySurface( msurface_t *pSurf );
void R_ClearSkyBox (void);
void R_DrawSkyBox (void);
void R_MarkLights (dlight_t *light, int bit, mnode_t *node);

void R_SetGlobalColors (void);

#define	TEXT_ORIENTED	0x0001
#define	TEXT_CENTERED	0x0002

void COM_StripExtension (char *in, char *out);

void	*R_GetPicData( const char *name, int *pWidth, int *pHeight, resource_t resource );
void	DrawChar (int x, int y, int c);
void	DrawTileClear (int x, int y, int w, int h, const char *name);
void	Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows, byte *data);
void	Draw_StringOriented (CVector &origin, CVector &normal, char *str, float scale, int flags);

extern	int		text_numEntities;	//	reset each frame
void	Text_DrawEntities (void);
void	Text_AddEntity (CVector &origin, CVector &normal, char *str, float scale, int flags);

void	BeginFrame( float camera_separation );
void	R_SwapBuffers( int );
void	R_SetPalette ( const unsigned char *palette);
void	R_SetPaletteDir (const char *name);

//	Nelno:	added for per-level console background and characters
void	R_SetResourceDir (char *name);
//  Shawn:  added this for getting frame name
void	R_FrameNameForModel (char *modelname, int frameindex, char *framename);
void	DrawConsolePic (int x, int y, int w, int h, int episode_num, int map_num);

int		Draw_GetPalette (void);

void	GL_ResampleTexture (unsigned *in, int inwidth, int inheight, unsigned *out,  int outwidth, int outheight);

void	*RegisterSkin (const char *name, resource_t flags);

//for 8 bit textures, pass in a non-null palette to use a special palette if the pic is going to be
//expanded to 32 bits.  otherwise, palette is ignored.
image_t *GL_LoadPic (const char *name, byte *pic, int width, int height, imagetype_t type, int bits, byte *palette);

image_t	*R_FindImage (const char *name, imagetype_t type, resource_t resource);
void	GL_TextureMode( const char *string );
void	GL_ImageList_f (void);

void	GL_UploadPalette( unsigned palette[256], int palNum );
void	GL_SetTexturePalette( image_t *image, qboolean bForcePalette );

void	GL_InitImages (void);
void	GL_ShutdownImages (void);

void	GL_FreeUnusedImages (void);

void GL_TextureAlphaMode( char *string );// SCG[5/20/99]: Quake2 V3.19 addition
void GL_TextureSolidMode( char *string );// SCG[5/20/99]: Quake2 V3.19 addition

//  Shawn:  added for image logging
void R_OpenImageLog (void);
void R_CloseImageLog (void);
void R_DumpImage (image_t *image);

// SCG[6/18/99]: 
void GL_DrawParticles( int n, const particle_t particles[] );
void R_DrawSurfaceSprites();
void R_DrawParticles ();
void R_DrawComplexParticles ();
void GL_GenerateRainVectors (const CVector &p_origin, int type);

/*
** GL extension emulation functions
*/

/*
** GL config stuff
*/
#define GL_RENDERER_VOODOO		0x00000001
#define GL_RENDERER_VOODOO2   	0x00000002
#define GL_RENDERER_VOODOO_RUSH	0x00000004
#define GL_RENDERER_BANSHEE		0x00000008
#define		GL_RENDERER_3DFX		0x0000000F

#define GL_RENDERER_PCX1		0x00000010
#define GL_RENDERER_PCX2		0x00000020
#define GL_RENDERER_PMX			0x00000040
#define		GL_RENDERER_POWERVR		0x00000070

#define GL_RENDERER_PERMEDIA2	0x00000100
#define GL_RENDERER_GLINT_MX	0x00000200
#define GL_RENDERER_GLINT_TX	0x00000400
#define GL_RENDERER_3DLABS_MISC	0x00000800
#define		GL_RENDERER_3DLABS	0x00000F00

#define GL_RENDERER_REALIZM		0x00001000
#define GL_RENDERER_REALIZM2	0x00002000
#define		GL_RENDERER_INTERGRAPH	0x00003000

#define GL_RENDERER_3DPRO		0x00004000
#define GL_RENDERER_REAL3D		0x00008000
#define GL_RENDERER_RIVA128		0x00010000
#define GL_RENDERER_DYPIC		0x00020000

#define GL_RENDERER_V1000		0x00040000
#define GL_RENDERER_V2100		0x00080000
#define GL_RENDERER_V2200		0x00100000
#define		GL_RENDERER_RENDITION	0x001C0000

#define GL_RENDERER_O2          0x00100000
#define GL_RENDERER_IMPACT      0x00200000
#define GL_RENDERER_RE			0x00400000
#define GL_RENDERER_IR			0x00800000
#define		GL_RENDERER_SGI			0x00F00000

#define GL_RENDERER_MCD			0x01000000
#define GL_RENDERER_OTHER		0x80000000

typedef struct
{
	int         renderer;
	const char *renderer_string;
	const char *vendor_string;
	const char *version_string;
	const char *extensions_string;

	qboolean	allow_cds;
} glconfig_t;

typedef struct
{
	float inverse_intensity;
	qboolean fullscreen;

	int     prev_mode;

	unsigned char *d_16to8table;

	int lightmap_textures;

	int		nTexnum[2];
	GLenum	nTexEnv[2];
	int		nTMU;

	float camera_separation;
	qboolean stereo_enabled;

	unsigned char originalRedGammaTable[256];
	unsigned char originalGreenGammaTable[256];
	unsigned char originalBlueGammaTable[256];

	// SCG[5/18/99]: Added to extend carmacks state management
	unsigned int	nStateFlags;

	float			fAlphaFunc;
	GLenum			eAlphaFunc;

	GLenum			eDepthFunc;

	int				nColorBits;
	int				nDepthBits;

	unsigned int	procedural_texture_flags;
} glstate_t;

extern glconfig_t  gl_config;
extern glstate_t   gl_state;

/*
====================================================================

IMPORTED FUNCTIONS

====================================================================
*/

extern	refimport_t	ri;


/*
====================================================================

IMPLEMENTATION SPECIFIC FUNCTIONS

====================================================================
*/

void		RImp_BeginFrame( float camera_separation );
void		RImp_EndFrame( void );
void		RImp_CheckError( void );
int 		RImp_Init( void *hinstance, void *hWnd );
void		RImp_Shutdown( void );
int     	RImp_SetMode( int *pwidth, int *pheight, int mode, qboolean fullscreen );
void		RImp_AppActivate( qboolean active );
int			RImp_GetBitDepth();
int			RImp_GetZDepth();
void		GLimp_EnableLogging( qboolean enable );
void		GLimp_LogNewFrame( void );

///////////////////////////////////////////////////////////////////////////////
//	hierarchy.c
///////////////////////////////////////////////////////////////////////////////

int	R_SurfIndex (void *mod, char *name);
int	R_VertInfo (void *mod, int surfIndex, int frameIndex, int vertIndex, CVector &origin, 
				CVector &oldorigin, CVector &angles, CVector &vert);
int	R_TriVerts (void *mod, int surfIndex, int frameIndex, int vertIndex, CVector &origin, 
				CVector &oldorigin, CVector &angles, CVector &v1, CVector &v2, CVector &v3);
int	R_TriVerts_Lerp (void *mod, int surfIndex, int frameIndex, int vertIndex, CVector &origin, 
				CVector &oldorigin, CVector &angles, CVector &v1, CVector &v2, CVector &v3, int oldFrame, float backlerp);


///////////////////////////////////////////////////////////////////////////////
//	gl_model.c
///////////////////////////////////////////////////////////////////////////////

void	Mod_RegisterEpisodeSkins (void *mod, int episode, int flags);

//#ifdef __cplusplus
//}
//#endif

///////////////////////////////////////////////////////////////////////////////
// SCG[4/15/99]: Fog stuff
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
	CPtrArray	hull;
	CVector		vMins;
	CVector		vMaxs;
	CVector		vColor;
	int			brushnum;
	float		distance;
	msurface_t	*surfaces;
	int			surface_count;
} fogvolume_t;

// Ash - for fogging entities
typedef struct
{
    float z;        // world space
    float alpha;    // fog alpha
} fogsample_s;

// Ash - for fogging entities
#define R_MAX_FOG_SAMPLES  10

// Ash - for fogging entities
typedef struct
{
    int fogVolume;
    bool fPartial;  // is the z range only partially in the fog?
    bool fViewerInFog; // is the viewer in the fog?
    float fog_r;
    float fog_g;
    float fog_b;
    float deltaZ;
    float invDeltaZ;
    fogsample_s fogSamples[R_MAX_FOG_SAMPLES];
    int   nFogSamples;
} fogsample_t;
                  
                  
extern fogsample_t gFogSample;

extern fogvolume_t	r_fogvolumes[];
extern int			r_fog_volume_count;
extern int			r_current_fog_volume;

void	R_FogInit();
void	R_DrawFogSurfaces();
void	R_DrawFogVolumes();
void	R_DrawFogEntities();
int		R_FogAddVolume( int brushnum );
int		R_FogVolumeForBrush( int brushnum );

bool    GL_FogBBox(CVector* bbox, int bboxSize, fogsample_t& fogSample, int nFogSamples);

void    R_DrawBrushModelMidTextureFogShell (entity_t *e);
//
// Ash -- sample a fogsample_s at height z.
//
inline float GL_GetFogSampleAlphaAtZ(float z, fogsample_t& fogSample)
{
    int i;
    float t, alpha;

    // early out if possible
    if (z<=fogSample.fogSamples[0].z)
        alpha = fogSample.fogSamples[0].alpha;
    else if (z>=fogSample.fogSamples[fogSample.nFogSamples-1].z)
    {
        if (true==fogSample.fPartial)
            alpha = 0.0f;   // we're out of the fog
        else
            alpha = fogSample.fogSamples[fogSample.nFogSamples-1].alpha;
    }
    else
    {
        // find the first sample above our z
        i=0;
        while ((i<fogSample.nFogSamples)&&(fogSample.fogSamples[i].z<=z))
        {
            i++;
        }

		if( i >= 1 )
		{
			t = (z-fogSample.fogSamples[i-1].z)*fogSample.invDeltaZ;
			alpha = fogSample.fogSamples[i-1].alpha + t*(fogSample.fogSamples[i].alpha-fogSample.fogSamples[i-1].alpha);
		}
		else
		{
			t = 0;
			alpha = 0;
		}
    }

    return alpha;
}

// Ash -- byte indexed unit normal array for approximating effects
extern CVector r_vertnormals[];

int R_GetClosestVertNormal(CVector& v);

// Ash -- my secret data structures
typedef struct
{
    int index;      // my index
    int vindex[3];  // my vert indices
    int nindex[3];  // my neighbor indices
    int data;       // data
} shell_patch_t;

typedef struct
{
    model_t *pModel;            // model this info describes
    shell_patch_t *pPatches;    // patch info
    float *pVData;              // vert data
    int numVData;               // num verts
    int numPatches;             // number of patches
    int numActive;
} shell_t;

extern shell_t gAliasShells[];
extern int gNumAliasShells;

#define	GLSTATE_TEXENV_MODULATE		0x00000001
#define	GLSTATE_TEXENV_REPLACE		0x00000002
#define	GLSTATE_DEPTH_TEST			0x00000004
#define GLSTATE_DEPTH_FUNC			0x00000008

#define GLSTATE_ALPHA_TEST			0x00000010
#define GLSTATE_ALPHA_FUNC			0x00000020
#define GLSTATE_BLEND				0x00000040
#define GLSTATE_CULL_FACE			0x00000080

#define	GLSTATE_FOG					0x00000100
#define	GLSTATE_SCISSOR_TEST		0x00000200
#define GLSTATE_TEXTURE_2D			0x00000400
#define GLSTATE_TEXTURE_GEN_Q		0x00000800

#define GLSTATE_TEXTURE_GEN_R		0x00001000
#define GLSTATE_TEXTURE_GEN_S		0x00002000
#define GLSTATE_TEXTURE_GEN_T		0x00004000
#define GLSTATE_BIND				0x00008000

#define GLSTATE_DEPTH_MASK			0x00010000
#define GLSTATE_MULTITEXTURE		0x00020000
#define GLSTATE_SHARED_PALETTE		0x00040000
#define GLSTATE_BLEND_FUNC			0x00080000

#define	GLSTATE_CULL_FACE_FRONT		0x00100000

#define GLSTATE_PRESET1				( GLSTATE_DEPTH_TEST | GLSTATE_CULL_FACE | GLSTATE_TEXTURE_2D | GLSTATE_DEPTH_MASK | GLSTATE_CULL_FACE_FRONT | GLSTATE_FOG )
#define GLSTATE_PRESET2				( GLSTATE_DEPTH_TEST | GLSTATE_CULL_FACE | GLSTATE_DEPTH_MASK | GLSTATE_CULL_FACE_FRONT | GLSTATE_FOG )

/*
====================================================================

// SCG[5/17/99]: State management

====================================================================
*/
void GL_InitState();
void GL_SetState( unsigned int nStateFlags );
unsigned int GL_GetState();
void GL_SetFunc( unsigned int nStateFlags, GLenum nUser, float fUser );
void GL_BindTexture( int nTMU, int nTexNum, GLenum nMode );
