  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#include "ref.h"

#define REF_VERSION     "SOFT 0.01"

#define BYTES_PER_PIXEL	2

// up / down
#define PITCH   0

// left / right
#define YAW             1

// fall over
#define ROLL    2


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
	it_procedural
} imagetype_t;

#define	IMAGE_GLOBAL	0x0001
#define	IMAGE_EPISODE	0x0002
#define	IMAGE_LEVEL		0x0004

//#ifdef __cplusplus
//extern "C" {
//#endif

//#define NUMLIGHTLEVELS 64			// uses 32 k of memory per skin

#define NUMLIGHTLEVELS 32			// uses 16 k of memory per skin

//#define NUMLIGHTLEVELS 16			// uses 8 k of memory per skin

//#define NUMLIGHTLEVELS 8			// uses 4 k of memory per skin

typedef struct image_s
{
	char			name[MAX_QPATH];        // game path, including extension
	imagetype_t     type;
	int             width, height;
	qboolean        transparent;			// true if any 255 pixels in image
	int             registration_sequence;  // 0 = free
	byte			*pixels[4];				// mip levels
	resource_t		resource;			//	GLOBAL, EPISODE and LEVEL
	int				pt_type;			//	procedural type number if it_procedural
	uint16			*palette;
	int				bytes_per_pixel;
} image_t;

#define	MAX_RIMAGES	1024
extern	image_t		r_images[MAX_RIMAGES];
extern	int			numr_images;

//===================================================================

typedef unsigned char pixel_t;

typedef struct vrect_s
{
	int                             x,y,width,height;
	struct vrect_s  *pnext;
} vrect_t;

typedef struct
{
	pixel_t                 *buffer;                // invisible buffer
	pixel_t                 *colormap;              // 256 * VID_GRADES size
	pixel_t                 *alphamap;              // 256 * 256 translucency map
	pixel_t                 *fogmap;
	int						rowbytes;               // may be > width if displayed in a window
									// can be negative for stupid dibs
	int						width;          
	int						height;
	int						colordepth;
	int						pixelwidth;
} viddef_t;

typedef enum
{
	rserr_ok,

	rserr_invalid_fullscreen,
	rserr_invalid_mode,

	rserr_unknown
} rserr_t;

extern viddef_t vid;

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct
{
	vrect_t         vrect;                          // subwindow in video for refresh
									// FIXME: not need vrect next field here?
	vrect_t         aliasvrect;                     // scaled Alias version
	int                     vrectright, vrectbottom;        // right & bottom screen coords
	int                     aliasvrectright, aliasvrectbottom;      // scaled Alias versions
	float           vrectrightedge;                 // rightmost right edge we care about,
										//  for use in edge list
	float           fvrectx, fvrecty;               // for floating-point compares
	float           fvrectx_adj, fvrecty_adj; // left and top edges, for clamping
	int                     vrect_x_adj_shift20;    // (vrect.x + 0.5 - epsilon) << 20
	int                     vrectright_adj_shift20; // (vrectright + 0.5 - epsilon) << 20
	float           fvrectright_adj, fvrectbottom_adj;
										// right and bottom edges, for clamping
	float           fvrectright;                    // rightmost edge, for Alias clamping
	float           fvrectbottom;                   // bottommost edge, for Alias clamping
	float           horizontalFieldOfView;  // at Z = 1.0, this many X is visible 
										// 2.0 = 90 degrees
	float           xOrigin;                        // should probably allways be 0.5
	float           yOrigin;                        // between be around 0.3 to 0.5

	CVector          vieworg;
	CVector          viewangles;
	
	int                     ambientlight;
} oldrefdef_t;

extern "C" oldrefdef_t      r_refdef;

#include "r_model.h"

#define CACHE_SIZE      32

/*
====================================================

  CONSTANTS

====================================================
*/

// 64 levels of lighting
//#define VID_CBITS       6

// 32 levels of lighting
#define VID_CBITS       5

// 16 levels of lighting
//#define VID_CBITS       4

// 8 levels of lighting
//#define VID_CBITS       3

#define VID_GRADES      (1 << VID_CBITS)


// r_shared.h: general refresh-related stuff shared between the refresh and the
// driver


#define MAXVERTS        64              // max points in a surface polygon
#define MAXWORKINGVERTS (MAXVERTS+4)    // max points in an intermediate
										//  polygon (while processing)
// !!! if this is changed, it must be changed in d_ifacea.h too !!!
#define MAXHEIGHT       1200
#define MAXWIDTH        1600

#define INFINITE_DISTANCE       0x10000         // distance that's always guaranteed to
										//  be farther away than anything in
										//  the scene


// d_iface.h: interface header file for rasterization driver modules

#define WARP_WIDTH              320
#define WARP_HEIGHT             240

#define MAX_LBM_HEIGHT  480


#define PARTICLE_Z_CLIP 8.0

// !!! must be kept the same as in quakeasm.h !!!
#define TRANSPARENT_COLOR       0xFF


// !!! if this is changed, it must be changed in d_ifacea.h too !!!
#define TURB_TEX_SIZE   64              // base turbulent texture size

// !!! if this is changed, it must be changed in d_ifacea.h too !!!
#define CYCLE                   128             // turbulent cycle size

#define SCANBUFFERPAD           0x1000

#define DS_SPAN_LIST_END        -128

#define NUMSTACKEDGES           2000
#define MINEDGES                        NUMSTACKEDGES
#define NUMSTACKSURFACES        1000
#define MINSURFACES                     NUMSTACKSURFACES
#define MAXSPANS                        3000

// flags in finalvert_t.flags
#define ALIAS_LEFT_CLIP                         0x0001
#define ALIAS_TOP_CLIP                          0x0002
#define ALIAS_RIGHT_CLIP                        0x0004
#define ALIAS_BOTTOM_CLIP                       0x0008
#define ALIAS_Z_CLIP                            0x0010
#define ALIAS_XY_CLIP_MASK                      0x000F

#define SURFCACHE_SIZE_AT_320X240    (1024*768)*BYTES_PER_PIXEL

#define BMODEL_FULLY_CLIPPED    0x10 // value returned by R_BmodelCheckBBox ()
									 //  if bbox is trivially rejected

#define XCENTERING      (1.0 / 2.0)
#define YCENTERING      (1.0 / 2.0)

#define CLIP_EPSILON            0.001

#define BACKFACE_EPSILON        0.01

// !!! if this is changed, it must be changed in asm_draw.h too !!!
#define NEAR_CLIP       0.01


#define MAXALIASVERTS           2000    // TODO: tune this
#define ALIAS_Z_CLIP_PLANE      4

// turbulence stuff

#define AMP             8*0x10000
#define AMP2    3
#define SPEED   20


/*
====================================================

TYPES

====================================================
*/

typedef struct
{
	float   u, v;
	float   s, t;
	float   zi;
} emitpoint_t;

/*
** if you change this structure be sure to change the #defines
** listed after it!
*/
#define SMALL_FINALVERT 0

#if SMALL_FINALVERT

typedef struct finalvert_s {
	short           u, v, s, t;
	int             l;
	int             zi;
	int             flags;
	float   xyz[3];         // eye space
} finalvert_t;

#define FINALVERT_V0     0
#define FINALVERT_V1     2
#define FINALVERT_V2     4
#define FINALVERT_V3     6
#define FINALVERT_V4     8
#define FINALVERT_V5    12
#define FINALVERT_FLAGS 16
#define FINALVERT_X     20
#define FINALVERT_Y     24
#define FINALVERT_Z     28
#define FINALVERT_SIZE  32

#else

typedef struct finalvert_s {
	int             u, v, s, t;
	int             l;
	int             zi;
	int             flags;
	float   xyz[3];         // eye space
} finalvert_t;

#define FINALVERT_V0     0
#define FINALVERT_V1     4
#define FINALVERT_V2     8
#define FINALVERT_V3    12
#define FINALVERT_V4    16
#define FINALVERT_V5    20
#define FINALVERT_FLAGS 24
#define FINALVERT_X     28
#define FINALVERT_Y     32
#define FINALVERT_Z     36
#define FINALVERT_SIZE  40

#endif

typedef struct
{
	void					*pskin;
	int						pskindesc;
	int						skinwidth;
	int						skinheight;
	dtriangle_t				*ptriangles;
	finalvert_t				*pfinalverts;
	int						numtriangles;
	int						drawtype;
	int						seamfixupX16;
	qboolean				do_vis_thresh;
	int						vis_thresh;
} affinetridesc_t;

typedef struct
{
	byte			*surfdat;       // destination for generated surface
	int				rowbytes;       // destination logical width in bytes
	msurface_t      *surf;          // description for surface to generate
	fixed8_t        lightadj[MAXLIGHTMAPS];
							// adjust for lightmap levels for dynamic lighting
	image_t			*image;
	int				surfmip;        // mipmapped ratio of surface texels / world pixels
	int				surfwidth;      // in mipmapped texels
	int				surfheight;     // in mipmapped texels
} drawsurf_t;



typedef struct {
	int		ambientlight;
	int		shadelight;
	CVector	plightvec;
} alight_t;

// clipped bmodel edges

typedef struct bedge_s
{
	mvertex_t               *v[2];
	struct bedge_s  *pnext;
} bedge_t;


// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct clipplane_s
{
	CVector          normal;
	float           dist;
	struct          clipplane_s     *next;
	byte            leftedge;
	byte            rightedge;
	byte            reserved[2];
} clipplane_t;


typedef struct surfcache_s
{
	struct surfcache_s	*next;
	struct surfcache_s	**owner;                // NULL is an empty chunk of memory
	int					lightadj[MAXLIGHTMAPS]; // checked for strobe flush
	int					dlight;
	int					size;           // including header
	unsigned			width;
	unsigned			height;         // DEBUG only needed for debug
	float				mipscale;
	image_t				*image;
	byte				data[4];        // width*height elements
} surfcache_t;

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct espan_s
{
	int                             u, v, count;
	struct espan_s  *pnext;
} espan_t;

// used by the polygon drawer (R_POLY.C) and sprite setup code (R_SPRITE.C)
typedef struct
{
	int				nump;
	emitpoint_t		*pverts;
	byte			*pixels;                        // image
	int				pixel_width;            // image width
	int				pixel_height;       // image height
	CVector			vup, vright, vpn;       // in worldspace, for plane eq
	float			dist;
	float			s_offset, t_offset;
	CVector			viewer_position;
	void			(*drawspanlet)( void );
	int				stipple_parity;
	uint16			*palette;
} polydesc_t;

// FIXME: compress, make a union if that will help
// insubmodel is only 1, flags is fewer than 32, spanstate could be a byte
typedef struct surf_s
{
	struct surf_s   *next;                  // active surface stack in r_edge.c
	struct surf_s   *prev;                  // used in r_edge.c for active surf stack
	struct espan_s  *spans;                 // pointer to linked list of spans to draw
	int				key;					// sorting key (BSP order)
	int				last_u;					// set during tracing
	int				spanstate;				// 0 = not in span
											// 1 = in span
											// -1 = in inverted span (end before
											//  start)
	int				flags;					// currentface flags
	msurface_t		*msurf;
	entity_t		*entity;
	float			nearzi;					// nearest 1/z on surface, for mipmapping
	qboolean		insubmodel;
	float			d_ziorigin, d_zistepu, d_zistepv;

	int				pad[2];					// to 64 bytes
} surf_t;

// !!! if this is changed, it must be changed in asm_draw.h too !!!
typedef struct edge_s
{
	fixed16_t               u;
	fixed16_t               u_step;
	struct edge_s   *prev, *next;
	unsigned short  surfs[2];
	struct edge_s   *nextremove;
	float                   nearzi;
	medge_t                 *owner;
} edge_t;


/*
====================================================

VARS

====================================================
*/

extern int              d_spanpixcount;
extern "C" int              r_framecount;           // sequence # of current frame since Quake
									//  started
extern float    r_aliasuvscale;         // scale-up factor for screen u and v
									//  on Alias vertices passed to driver
extern qboolean r_dowarp;

extern "C" affinetridesc_t  r_affinetridesc;

extern CVector   r_pright, r_pup, r_ppn;

void R_SetAsmVars(void);
void D_DrawSurfaces (void);
void R_DrawParticle( void );
void D_ViewChanged (void);
void D_WarpScreen (void);
void R_PolysetUpdateTables (void);

extern void *acolormap; // FIXME: should go away

//=======================================================================//

// callbacks to Quake

extern drawsurf_t       r_drawsurf;

void R_DrawSurface (void);

extern int              c_surf;

extern byte             r_warpbuffer[WARP_WIDTH * WARP_HEIGHT];
//extern uint16             r_warpbuffer[WARP_WIDTH * WARP_HEIGHT];

extern uint16 *current_image_palette;


extern float    scale_for_mip;

extern qboolean         d_roverwrapped;
extern surfcache_t      *sc_rover;
extern surfcache_t      *d_initial_rover;

extern "C" float    d_sdivzstepu, d_tdivzstepu, d_zistepu;
extern "C" float    d_sdivzstepv, d_tdivzstepv, d_zistepv;
extern "C" float    d_sdivzorigin, d_tdivzorigin, d_ziorigin;

extern "C" fixed16_t       sadjust, tadjust;
extern "C" fixed16_t       bbextents, bbextentt;

extern "C" void D_DrawSpans16 (espan_t *pspans);
extern "C" void D_DrawZSpans (espan_t *pspans);
extern "C" void D_DrawSpans16_f (espan_t *pspans);
extern "C" void D_DrawZSpans_f (espan_t *pspans);
void Turbulent8 (espan_t *pspan);

surfcache_t     *D_CacheSurface (msurface_t *surface, int miplevel);

extern int      d_vrectx, d_vrecty, d_vrectright_particle, d_vrectbottom_particle;

extern int      d_pix_min, d_pix_max, d_pix_shift;

extern "C" pixel_t  *d_viewbuffer;
extern "C" short *d_pzbuffer;
extern "C" unsigned int d_zrowbytes, d_zwidth;
extern "C" short    *zspantable[MAXHEIGHT];
extern "C" int      d_scantable[MAXHEIGHT];

extern int              d_minmip;
extern float    d_scalemip[3];

//===================================================================

extern "C" int              cachewidth;
//extern "C" pixel_t			*cacheblock;
extern "C" uint16			*cacheblock;
extern int              r_screenwidth;

extern int              r_drawnpolycount;

//extern int      sintable[1280];
//extern int      intsintable[1280];
extern int      sintable[];
extern int      intsintable[];

extern  CVector  vup, base_vup;
extern  CVector  vpn, base_vpn;
extern  CVector  vright, base_vright;

extern "C" vec3_t r_origin_a;
extern "C" vec3_t modelorg_a;
extern "C" vec3_t r_entorigin_a;

extern "C" vec3_t	vup_a, base_vup_a;
extern "C" vec3_t	vpn_a, base_vpn_a;
extern "C" vec3_t	vright_a, base_vright_a;
extern "C" vec3_t	r_origin_a;

extern  "C" surf_t  *surfaces, *surface_p, *surf_max;

// surfaces are generated in back to front order by the bsp, so if a surf
// pointer is greater than another one, it should be drawn in front
// surfaces[1] is the background, and is used as the active surface stack.
// surfaces[0] is a dummy, because index 0 is used to indicate no surface
//  attached to an edge_t

//===================================================================

extern CVector   sxformaxis[4];  // s axis transformed into viewspace
extern CVector   txformaxis[4];  // t axis transformed into viewspac

extern "C"  float   xcenter, ycenter;
extern "C"  float   xscale, yscale;
extern "C"  float   xscaleinv, yscaleinv;
extern "C"  float   xscaleshrink, yscaleshrink;

extern void TransformVector ( CVector &in, CVector &out);
extern void SetUpForLineScan(fixed8_t startvertu, fixed8_t startvertv,
	fixed8_t endvertu, fixed8_t endvertv);

extern "C" int      ubasestep, errorterm, erroradjustup, erroradjustdown;

//===========================================================================

extern cvar_t   *sw_aliasstats;
extern cvar_t   *sw_clearcolor;
extern cvar_t   *sw_drawflat;
extern cvar_t   *sw_draworder;
extern cvar_t   *sw_maxedges;
extern cvar_t   *sw_maxsurfs;
extern cvar_t   *sw_mipcap;
extern cvar_t   *sw_mipscale;
extern cvar_t   *sw_mode;
extern cvar_t   *sw_reportsurfout;
extern cvar_t   *sw_reportedgeout;
extern cvar_t   *sw_stipplealpha;
extern cvar_t   *sw_surfcacheoverride;
extern cvar_t   *sw_waterwarp;

extern cvar_t   *r_fullbright;
//extern cvar_t	*r_lefthand;
extern cvar_t   *r_drawentities;
extern cvar_t   *r_drawworld;
extern cvar_t   *r_dspeeds;
extern cvar_t   *r_lerpmodels;
extern cvar_t	*r_dynamiclights;

extern cvar_t   *r_speeds;

extern cvar_t   *r_lightlevel;  //FIXME HACK

extern cvar_t   *console_screen_idx; // 12.21  dsn

extern cvar_t	*vid_fullscreen;
extern cvar_t	*vid_gamma;

extern cvar_t	*r_palettedir;

//	Nelno:	added for per-level console backgrounds/characters
extern	cvar_t	*r_resourcedir;

extern	cvar_t	*r_alias_fogz;

extern	cvar_t	*sw_modulate;

extern	cvar_t	*sw_asm;

extern  clipplane_t     view_clipplanes[4];
extern int              *pfrustum_indexes[4];

extern	unsigned char	global_color[GC_NUMCOLORS];

extern	int		ref_curTime;
extern	int		ref_lastTime;
extern	float	ref_frameTime;
extern	float	ref_globalTime;	//	total time passed since R_Init, in seconds

extern int		r_blendmask50;
extern int		r_blendmask40;
extern int		r_blendmask60;

//=============================================================================

void R_RenderWorld (void);
void SW_PurgeResources(resource_t resource);

//=============================================================================

extern  mplane_t        screenedge[4];

extern  CVector			r_origin;
extern  CVector			modelorg;
extern  CVector			r_entorigin;

extern	entity_t		r_worldentity;
extern  model_t			*currentmodel;
extern  entity_t		*currententity;

extern  float   verticalFieldOfView;
extern  float   xOrigin, yOrigin;

extern  int             r_visframecount;

extern msurface_t *r_alpha_surfaces;

//=============================================================================

void R_ClearPolyList (void);
void R_DrawPolyList (void);

//
// current entity info
//
extern  qboolean                insubmodel;

void R_DrawAlphaSurfaces( void );

void R_DrawSprite (void);
void R_DrawBeam( entity_t *e );

void R_RenderFace (msurface_t *fa, int clipflags);
void R_RenderBmodelFace (bedge_t *pedges, msurface_t *psurf);
void R_TransformPlane (mplane_t *p, float *normal, float *dist);
void R_TransformFrustum (void);
void R_DrawSurfaceBlock16 (void);
void R_DrawSurfaceBlock8 (void);

#if     id386

extern "C" void R_DrawSurfaceBlock8_mip0 (void);
extern "C" void R_DrawSurfaceBlock8_mip1 (void);
extern "C" void R_DrawSurfaceBlock8_mip2 (void);
extern "C" void R_DrawSurfaceBlock8_mip3 (void);

#endif

void R_GenSkyTile (void *pdest);
void R_GenSkyTile16 (void *pdest);
extern "C" void R_Surf8Patch (void);
extern "C" void R_Surf16Patch (void);
void R_DrawSubmodelPolygons (model_t *pmodel, int clipflags, mnode_t *topnode);
void R_DrawSolidClippedSubmodelPolygons (model_t *pmodel, mnode_t *topnode);

void R_AddPolygonEdges (emitpoint_t *pverts, int numverts, int miplevel);
surf_t *R_GetSurf (void);
void R_AliasDrawModel (void);
void R_BeginEdgeFrame (void);
void R_ScanEdges (void);
void D_DrawSurfaces (void);
extern "C" void R_InsertNewEdges (edge_t *edgestoadd, edge_t *edgelist);
extern "C" void R_StepActiveU (edge_t *pedge);
extern "C" void R_RemoveEdges (edge_t *pedge);
void R_PushDlights (model_t *model);

void R_ViewChanged (vrect_t *vr);

extern "C" void R_Surf8Start (void);
extern "C" void R_Surf8End (void);
extern "C" void R_Surf16Start (void);
extern "C" void R_Surf16End (void);
extern "C" void R_EdgeCodeStart (void);
extern "C" void R_EdgeCodeEnd (void);

extern void R_RotateBmodel (void);

extern int      c_faceclip;
extern int      r_polycount;
extern int      r_wholepolycount;

extern int                      ubasestep, errorterm, erroradjustup, erroradjustdown;

extern fixed16_t        sadjust, tadjust;
extern fixed16_t        bbextents, bbextentt;

extern mvertex_t        *r_ptverts, *r_ptvertsmax;

extern float                    entity_rotation[3][3];

extern int              r_currentkey;
extern int              r_currentbkey;

void    R_InitTurb (void);

void R_DrawParticles (void);
void R_DrawComParticles (void);
extern "C" void R_SurfacePatch (void);

extern "C" int              r_amodels_drawn;
extern "C" edge_t   *auxedges;
extern "C" int              r_numallocatededges;
extern "C" edge_t   *r_edges, *edge_p, *edge_max;

extern "C" edge_t  *newedges[MAXHEIGHT];
extern "C" edge_t  *removeedges[MAXHEIGHT];

// FIXME: make stack vars when debugging done
extern "C" edge_t  edge_head;
extern "C" edge_t  edge_tail;
extern "C" edge_t  edge_aftertail;

extern	int	r_aliasblendcolor;

extern float    aliasxscale, aliasyscale, aliasxcenter, aliasycenter;

extern int              r_outofsurfaces;
extern int              r_outofedges;

extern mvertex_t        *r_pcurrentvertbase;
extern int                      r_maxvalidedgeoffset;

typedef struct
{
	finalvert_t *a, *b, *c;
} aliastriangleparms_t;

extern aliastriangleparms_t aliastriangleparms;

void R_DrawTriangle( void );
void R_AliasClipTriangle (finalvert_t *index0, finalvert_t *index1, finalvert_t *index2);


extern float    r_time1;
extern float	da_time1, da_time2;
extern float	dp_time1, dp_time2, db_time1, db_time2, rw_time1, rw_time2;
extern float	se_time1, se_time2, de_time1, de_time2, dv_time1, dv_time2;
extern int              r_frustum_indexes[4*6];
extern int              r_maxsurfsseen, r_maxedgesseen, r_cnumsurfs;
extern qboolean r_surfsonstack;

extern	mleaf_t		*r_viewleaf;
extern	int			r_viewcluster, r_oldviewcluster;

extern int              r_clipflags;
extern int              r_dlightframecount;
extern qboolean r_fov_greater_than_90;

extern  image_t         *r_notexture_mip;
extern	image_t			*r_lasertexture,*r_novalasertexture,*r_tracertexture, *r_lightningtexture1,*r_lightningtexture2,*r_lightningtexture3;
extern  model_t         *r_worldmodel;

void R_PrintAliasStats (void);
void R_PrintTimes (void);
void R_PrintDSpeeds (void);
void R_AnimateLight (void);
void R_LightPoint (CVector &p, CVector &color);
void R_SetupFrame (void);
void R_cshift_f (void);
void R_EmitEdge (mvertex_t *pv0, mvertex_t *pv1);
extern "C" void R_ClipEdge (mvertex_t *pv0, mvertex_t *pv1, clipplane_t *clip);
void R_SplitEntityOnNode2 (mnode_t *node);

extern  refdef_t        r_newrefdef;

extern  surfcache_t     *sc_rover, *sc_base;

extern "C" void            *colormap;
extern "C" void            *alphamap;
extern "C" void            *fogmap;

//====================================================================

float R_DLightPoint (CVector &p);

void R_NewMap (void);
void R_Register (void);
void R_UnRegister (void);
void Draw_InitLocal (void);
qboolean R_Init( void *hInstance, void *wndProc );
void R_Shutdown (void);
void R_InitCaches (void);
void D_FlushCaches (void);

void	R_ScreenShot_f( void );
void    R_BeginRegistration (const char *map);
void	*RegisterModel (const char *name, resource_t flags);
void    R_EndRegistration (void);

void    R_RenderFrame (refdef_t *fd);

void	*Draw_FindPic (const char *name, resource_t flags);

void    Draw_GetPicSize (int *w, int *h, const char *name, resource_t resource);
void    DrawPic (int x, int y, const char *name, resource_t resource);
void    DrawPicPalette (int x, int y, const char *name, float alpha, resource_t resource);
void    Draw_StretchPic (int x, int y, int w, int h, const char *name, resource_t resource);
void    Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows, byte *data);
void    R_DrawChar (int x, int y, int c);
int		Draw_AlphaChar (int x, int y, int num, int startMsec, int endMsec, int curMsec, int timeOffset, CVector &rgbBlend, int flags, void *vfont);
void    Draw_TileClear (int x, int y, int w, int h, const char *name);
void    Draw_Fill (int x, int y, int w, int h, int c);
void    Draw_FadeScreen (void);

//	Nelno:	added for per-level console backgrounds and chars
void    Draw_ConsolePic (int x, int y, int w, int h);
//  Shawn:  added this for getting frame name
void	R_FrameNameForModel (char *modelname, int frameindex, char *framename);

void    Draw_GetPalette (void);
void	Draw_GetFogTable (void);

void	 R_BeginFrame( float camera_separation );

void	R_CinematicSetPalette( const unsigned char *palette );

extern "C" uint16	d_8to16table[256]; // base
extern unsigned d_8to24table[256]; // base
extern uint16	d_fogtable[256];

void    Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length);
void    Sys_SetFPCW (void);

void    R_InitImages (void);
void	R_ShutdownImages (void);
image_t *R_FindImage (const char *name, imagetype_t type, int resource);
void    R_FreeUnusedImages (void);

void	*GL_LoadPic (const char *name, byte *pic, int width, int height, imagetype_t type);

void	R_GammaCorrectAndSetPalette( const unsigned char *pal );

extern mtexinfo_t  *sky_texinfo[6];

void R_InitSkyBox (void);

typedef struct
{ // rgbq     
	byte    rgbBlue;     
	byte    rgbGreen; 
    byte    rgbRed;     
	byte    rgbReserved; 
} rgbquad_t; 

typedef struct
{
	rgbquad_t	depth;
	rgbquad_t	remove;
	rgbquad_t	position;
} rgb16_t;

typedef struct swstate_s
{
	qboolean fullscreen;
	int      prev_mode;				// last valid SW mode

	byte		gammatable[256];
	byte		currentpalette[1024];

	rgb16_t		rgb16;
} swstate_t;

void R_IMFlatShadedQuad( CVector &a, CVector &b, CVector &c, CVector &d, int color, float alpha );
void R_TexturedQuad( CVector &a, CVector &b, CVector &c, CVector &d, image_t *image, float alpha );

extern swstate_t sw_state;

/*
====================================================================

IMPORTED FUNCTIONS

====================================================================
*/

extern  refimport_t     ri;

/*
====================================================================

IMPLEMENTATION FUNCTIONS

====================================================================
*/

void		RImp_BeginFrame( float camera_separation );
void		RImp_EndFrame (void);
int			RImp_Init( void *hInstance, void *wndProc );
void		RImp_SetPalette( const unsigned char *palette);
void		RImp_Shutdown( void );
rserr_t		RImp_SetMode( int *pwidth, int *pheight, int mode, qboolean fullscreen );
void		Rimp_AppActivate( qboolean active );

///////////////////////////////////////////////////////////////////////////////
//	hierarchy.c
///////////////////////////////////////////////////////////////////////////////

int	R_SurfIndex (void *mod, char *name);
int	R_VertInfo (void *mod, int surfIndex, int frameIndex, int vertIndex, CVector &origin, 
				CVector &oldorigin, CVector &angles, CVector &vert);
int	R_TriVerts (void *mod, int surfIndex, int frameIndex, int vertIndex, CVector &origin, 
				CVector &oldorigin, CVector &angles, CVector &v1, CVector &v2, CVector &v3);

///////////////////////////////////////////////////////////////////////////////
//	gl_model.c
///////////////////////////////////////////////////////////////////////////////

void	Mod_RegisterEpisodeSkins (void *model, int episode, int flags);

///////////////////////////////////////////////////////////////////////////////
//	fog stuff
///////////////////////////////////////////////////////////////////////////////
extern "C" int		alias_fogz;
extern "C" unsigned int		surface_fogz;
extern "C" float	fogz;

//#ifdef __cplusplus
//}
//#endif
