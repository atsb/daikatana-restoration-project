// r_main.c

#include <io.h>
#include "r_local.h"
#include "r_protex.h"
#include "r_beams.h"

#include "dk_ref_common.h"


// declarations
viddef_t	vid;

uint16		d_8to16table[256];
unsigned	d_8to24table[256];
uint16		d_fogtable[256];

entity_t	r_worldentity;

char		skyname[MAX_QPATH];
float		skyrotate;
CVector		skyaxis;
image_t		*sky_images[6];

refdef_t	r_newrefdef;
model_t		*currentmodel;

model_t		*r_worldmodel;

byte		r_warpbuffer[WARP_WIDTH * WARP_HEIGHT];
//uint16		r_warpbuffer[WARP_WIDTH * WARP_HEIGHT];

uint16 *current_image_palette;

swstate_t sw_state;

void		*colormap;
void		*alphamap;
void		*fogmap;
CVector		viewlightvec;
//alight_t	r_viewlighting = {128, 192, viewlightvec};
float		r_time1;
int			r_numallocatededges;
float		r_aliasuvscale = 1.0;
int			r_outofsurfaces;
int			r_outofedges;

qboolean	r_dowarp;

mvertex_t	*r_pcurrentvertbase;

int			c_surf;
int			r_maxsurfsseen, r_maxedgesseen, r_cnumsurfs;
qboolean	r_surfsonstack;
int			r_clipflags;

//
// view origin
//
CVector	vup, base_vup;
CVector	vpn, base_vpn;
CVector	vright, base_vright;
CVector	r_origin;

vec3_t modelorg_a;
vec3_t r_entorigin_a;

vec3_t	vup_a, base_vup_a;
vec3_t	vpn_a, base_vpn_a;
vec3_t	vright_a, base_vright_a;
vec3_t	r_origin_a;

//
// screen size info
//
oldrefdef_t	r_refdef;
float		xcenter, ycenter;
float		xscale, yscale;
float		xscaleinv, yscaleinv;
float		xscaleshrink, yscaleshrink;
float		aliasxscale, aliasyscale, aliasxcenter, aliasycenter;

int		r_screenwidth;

float	verticalFieldOfView;
float	xOrigin, yOrigin;

mplane_t	screenedge[4];

//
// refresh flags
//
int		r_framecount = 1;	// so frame counts initialized to 0 don't match
int		r_visframecount;
int		d_spanpixcount;
int		r_polycount;
int		r_drawnpolycount;
int		r_wholepolycount;

int			*pfrustum_indexes[4];
int			r_frustum_indexes[4*6];

mleaf_t		*r_viewleaf;
int			r_viewcluster, r_oldviewcluster;

image_t  	*r_notexture_mip;
//image_t		*r_inventoryTextures [5];//	max of one per episode + default
//image_t		*r_inventoryNumbers;
image_t		*r_gunflaretexture;

float	da_time1, da_time2, dp_time1, dp_time2, db_time1, db_time2, rw_time1, rw_time2;
float	se_time1, se_time2, de_time1, de_time2;

void R_MarkLeaves (void);

//cvar_t	*r_lefthand;
cvar_t	*sw_aliasstats;
cvar_t	*sw_allow_modex;
cvar_t	*sw_clearcolor;
cvar_t	*sw_drawflat;
cvar_t	*sw_draworder;
cvar_t	*sw_maxedges;
cvar_t	*sw_maxsurfs;
cvar_t  *sw_mode;
cvar_t	*sw_reportedgeout;
cvar_t	*sw_reportsurfout;
cvar_t  *sw_stipplealpha;
cvar_t	*sw_surfcacheoverride;
cvar_t	*sw_waterwarp;

cvar_t	*r_drawworld;
cvar_t	*r_drawentities;
cvar_t	*r_dspeeds;
cvar_t	*r_fullbright;
cvar_t  *r_lerpmodels;
cvar_t  *r_novis;
cvar_t	*r_dynamiclights;

cvar_t	*r_speeds;
cvar_t  *r_lightlevel;  //FIXME HACK

cvar_t  *console_screen_idx; // 12.21  dsn

cvar_t	*vid_fullscreen;
cvar_t	*vid_gamma;

cvar_t	*r_aliasframerate;

//	Shawn:	added for palette per level 
cvar_t	*r_palettedir;
//	Nelno:	added for per-level console backgrounds/characters
cvar_t	*r_resourcedir;

cvar_t	*r_alias_fogz;

cvar_t	*sw_modulate;

cvar_t	*sw_asm;

#define	STRINGER(x) "x"

unsigned char	global_color[GC_NUMCOLORS];

//	Nelno:	for doing time-based stuff in the render dll, exported in gl_local.h
int		ref_curTime;
int		ref_lastTime;
float	ref_frameTime;
float	ref_globalTime;	//	total time passed since R_Init, in seconds
float	ref_laserRotation = 0.0F;
int		ref_laserDir = 1;

int		r_blendmask50;
int		r_blendmask40;
int		r_blendmask60;

void	R_DrawBeamList (void);
void	inventory_DrawIcons (void);

void R_SetAsmVars(){
	vup_a[0] = vup.x; vup_a[1] = vup.y; vup_a[2] = vup.z;
	vpn_a[0] = vpn.x; vpn_a[1] = vpn.y; vpn_a[2] = vpn.z; 
	vright_a[0] = vright.x; vright_a[1] = vright.y; vright_a[2] = vright.z; 
	r_origin_a[0] = r_origin.x; r_origin_a[1] = r_origin.y; r_origin_a[2] = r_origin.z;
	modelorg_a[0] = modelorg.x; modelorg_a[1] = modelorg.y; modelorg_a[2] = modelorg.z;
}

void CreateBlendMask(){
	byte	r, g, b;
	
	r = sw_state.rgb16.depth.rgbRed & 0xFE;
	g = sw_state.rgb16.depth.rgbGreen & 0xFE;
	b = sw_state.rgb16.depth.rgbBlue & 0xFE;

	r_blendmask50 = r << (sw_state.rgb16.position.rgbRed) | 
		(g << sw_state.rgb16.position.rgbGreen) | 
		(b << sw_state.rgb16.position.rgbBlue); 

	r = sw_state.rgb16.depth.rgbRed & 0xFC;
	g = sw_state.rgb16.depth.rgbGreen & 0xFC;
	b = sw_state.rgb16.depth.rgbBlue & 0xFC;

	r_blendmask40 = r << (sw_state.rgb16.position.rgbRed) | 
		(g << sw_state.rgb16.position.rgbGreen) | 
		(b << sw_state.rgb16.position.rgbBlue); 

	r = sw_state.rgb16.depth.rgbRed & 0xFD;
	g = sw_state.rgb16.depth.rgbGreen & 0xFD;
	b = sw_state.rgb16.depth.rgbBlue & 0xFD;

	r_blendmask60 = r << (sw_state.rgb16.position.rgbRed) | 
		(g << sw_state.rgb16.position.rgbGreen) | 
		(b << sw_state.rgb16.position.rgbBlue); 
}

/*
=================
R_GetModelSkinIndex

  - return the skin index for a particular alias model


FIXME_AMW: this doesn't account for custom skins....  
	
=================
*/
int R_GetModelSkinIndex (void *modelPtr)
{
	image_t	*skin = NULL;
	
	if (!modelPtr)
		return (0);

	// cast the pointer over to a model structure
	model_t	*currentmodel = (model_t*) modelPtr;

	// get the first skin
	skin = currentmodel->skins[0];

	if (skin)
	{
		// pointer arithmetic gives us the index of the skin
		int skinIndex = skin - r_images;
		return (skinIndex);
	}
	else
	{
		// no skins
		return (0);
	}
}

/*
=================
R_BestColor
=================
*/
unsigned char R_BestColor (byte r, byte g, byte b, unsigned *palette)
{
	int	i;
	int	dr, dg, db;
	int	bestdistortion, distortion;
	int	bestcolor;
	byte	*pal;

//
// let any color go to 0 as a last resort
//
	bestdistortion = 256*256*4;
	bestcolor = 0;

	pal = (byte *)palette;
	for (i = 0 ; i < 256 ; i++)
	{
		dr = r - (int)pal[0];
		dg = g - (int)pal[1];
		db = b - (int)pal[2];
		pal += 3;
		distortion = dr*dr + dg*dg + db*db;
		if (distortion < bestdistortion)
		{
			if (!distortion)
				return i;		// perfect match

			bestdistortion = distortion;
			bestcolor = i;
		}
	}

	return bestcolor;
}

/*
=================
R_GetGlobalColors

  - return the rgb values for the specified global colors.. values returned are 0-1
=================
*/
void R_GetGlobalColors (int globalColor, float *r, float *g, float *b)
{
	unsigned	*pal = d_8to24table;
	int palIndex = 0;
	
	// error checking
	if ( !pal || (globalColor < GC_BLACK) || (globalColor > GC_WHITE))
	{
		// return white on error
		(*r) = 1;
		(*g) = 1;
		(*b) = 1;
	}
	else
	{
		palIndex = global_color[globalColor];

		// amw - is this the right way to decode the palette indices??  I'm guessing..
		(*r) = (float)((pal[palIndex] >> 0) & 0xff) / 255.0;
		(*g) = (float)((pal[palIndex] >> 8) & 0xff) / 255.0;
		(*b) = (float)((pal[palIndex] >> 16) & 0xff) / 255.0;
	}
}

/*
=================
R_SetGlobalColors
=================
*/
void R_SetGlobalColors (byte *palette)
{
	global_color[GC_BLACK]		= R_BestColor (0, 0, 0, (unsigned int *) palette);
	global_color[GC_BLUE]		= R_BestColor (0, 0, 255, (unsigned int *) palette);
	global_color[GC_GREEN]		= R_BestColor (0, 255, 0, (unsigned int *) palette);
	global_color[GC_CYAN]		= R_BestColor (0, 0, 0, (unsigned int *) palette);
	global_color[GC_RED]		= R_BestColor (255, 0, 0, (unsigned int *) palette);
	global_color[GC_PURPLE]		= R_BestColor (128, 0, 128, (unsigned int *) palette);
	global_color[GC_BROWN]		= R_BestColor (128, 64, 0, (unsigned int *) palette);
	global_color[GC_LYGRAY]		= R_BestColor (128, 128, 128, (unsigned int *) palette);
	global_color[GC_DKGRAY]		= R_BestColor (64, 64, 64, (unsigned int *) palette);
	global_color[GC_LTBLUE]		= R_BestColor (64, 64, 255, (unsigned int *) palette);
	global_color[GC_LTGRN]		= R_BestColor (64, 255, 64, (unsigned int *) palette);
	global_color[GC_LTCYAN]		= R_BestColor (64, 255, 255, (unsigned int *) palette);
	global_color[GC_LTRED]		= R_BestColor (255, 64, 64, (unsigned int *) palette);
	global_color[GC_LTPURPLE]	= R_BestColor (255, 64, 255, (unsigned int *) palette);
	global_color[GC_YELLOW]		= R_BestColor (255, 255, 64, (unsigned int *) palette);
	global_color[GC_WHITE]		= R_BestColor (255, 255, 255, (unsigned int *) palette);
}


#if	!id386

// r_vars.c

// all global and static refresh variables are collected in a contiguous block
// to avoid cache conflicts.
// d_vars.c

// all global and static refresh variables are collected in a contiguous block
// to avoid cache conflicts.

//-------------------------------------------------------
// global refresh variables
//-------------------------------------------------------

// FIXME: make into one big structure, like cl or sv
// FIXME: do separately for refresh engine and driver

float	d_sdivzstepu, d_tdivzstepu, d_zistepu;
float	d_sdivzstepv, d_tdivzstepv, d_zistepv;
float	d_sdivzorigin, d_tdivzorigin, d_ziorigin;

fixed16_t	sadjust, tadjust, bbextents, bbextentt;

//pixel_t			*cacheblock;
uint16			*cacheblock;
int				cachewidth;
pixel_t			*d_viewbuffer;

pixel_t			*d_viewbuffer;
short			*d_pzbuffer;
unsigned int	d_zrowbytes;
unsigned int	d_zwidth;


#endif	// !id386

byte	r_notexture_buffer[2048];

/*
==================
R_InitTextures
==================
*/
void	R_InitTextures (void)
{
	int		x,y, m;
//	uint16	*dest;
	byte	*dest;
	
// create a simple checkerboard texture for the default
	r_notexture_mip = (image_t *)&r_notexture_buffer;
	
	r_notexture_mip->width = r_notexture_mip->height = 16;
/*
	r_notexture_mip->pixels[0] = (uint16*) &r_notexture_buffer[sizeof(image_t)];
	r_notexture_mip->pixels[1] = (uint16*) r_notexture_mip->pixels[0] + 16*16;
	r_notexture_mip->pixels[2] = (uint16*) r_notexture_mip->pixels[1] + 8*8;
	r_notexture_mip->pixels[3] = (uint16*) r_notexture_mip->pixels[2] + 4*4;
*/
	r_notexture_mip->pixels[0] = (byte*) &r_notexture_buffer[sizeof(image_t)];
	r_notexture_mip->pixels[1] = (byte*) r_notexture_mip->pixels[0] + 16*16;
	r_notexture_mip->pixels[2] = (byte*) r_notexture_mip->pixels[1] + 8*8;
	r_notexture_mip->pixels[3] = (byte*) r_notexture_mip->pixels[2] + 4*4;
	
	r_notexture_mip->palette = d_8to16table;

	for (m=0 ; m<4 ; m++)
	{
		dest = r_notexture_mip->pixels[m];
		for (y=0 ; y< (16>>m) ; y++)
			for (x=0 ; x< (16>>m) ; x++)
			{
				if (  (y< (8>>m) ) ^ (x< (8>>m) ) )
					*dest++ = 0;
				else
					*dest++ = 0xff;
			}
	}	

}


/*
===============
R_InitMiscTextures
===============
*/
void	R_InitMiscTextures (void)
{
	r_beamTextures[BEAM_TEX_LIGHTNING]	= (image_t*)R_FindImage ("pics/misc/w_zap001.bmp", it_sprite, RESOURCE_LEVEL);
	r_beamTextures[BEAM_TEX_LASER]		= (image_t*)R_FindImage ("pics/misc/laser.bmp", it_sprite, RESOURCE_LEVEL);
	r_beamTextures[BEAM_TEX_TRACER]		= (image_t*)R_FindImage ("pics/misc/tracer.bmp", it_sprite, RESOURCE_LEVEL);
	r_beamTextures[BEAM_TEX_NOVALASER]	= //R_FindImage ("pics/misc/novalaser.bmp", it_sprite, RESOURCE_LEVEL);
	r_beamTextures[BEAM_TEX_NOVALASER]	= (image_t*)R_FindImage ("pics/misc/novalaser.bmp", it_sprite, RESOURCE_LEVEL);
	r_beamTextures[BEAM_TEX_MF_GENERIC]	= (image_t*)R_FindImage ("skins/we_mflash2.bmp", it_sprite, RESOURCE_LEVEL);   
   // FIX ME: need chain texture!
   r_beamTextures[BEAM_TEX_GRAPPLE_CHAIN] = (image_t*)R_FindImage("pics/misc/w_zap001.bmp", it_sprite, RESOURCE_LEVEL);

//  r_inventoryTextures[0] = //R_FindImage ("pics/statusbar/invenbox.bmp", it_sprite, RESOURCE_LEVEL);
//	r_inventoryTextures[1] = //R_FindImage ("pics/statusbar/invenbox.bmp", it_sprite, RESOURCE_LEVEL);
//	r_inventoryTextures[2] = //R_FindImage ("pics/statusbar/invenbox.bmp", it_sprite, RESOURCE_LEVEL);
//	r_inventoryTextures[3] = //R_FindImage ("pics/statusbar/e3/invenbox.bmp", it_sprite, RESOURCE_LEVEL);
//	r_inventoryTextures[4] = (image_t*)R_FindImage ("pics/statusbar/invenbox.bmp", it_sprite, RESOURCE_LEVEL);

//	r_inventoryNumbers	= (image_t*)R_FindImage ("pics/statusbar/invennum.bmp", it_sprite, RESOURCE_LEVEL);

	//r_gunflaretexture	= (image_t*)R_FindImage ("pics/misc/gunflare.bmp", it_sprite, RESOURCE_LEVEL);
}


/*
================
R_InitTurb
================
*/
void R_InitTurb (void)
{
	int		i;
	
	for (i=0 ; i<2048 ; i++)
	{
		sintable[i] = AMP + sin(i*3.14159*2/CYCLE)*AMP;
		intsintable[i] = AMP2 + sin(i*3.14159*2/CYCLE)*AMP2;	// AMP2, not 20
	}
}

void R_ImageList_f( void );

void R_Register (void)
{
	sw_aliasstats = ri.Cvar_Get ("sw_polymodelstats", "0", 0);
	sw_allow_modex = ri.Cvar_Get( "sw_allow_modex", "1", CVAR_ARCHIVE );
	sw_clearcolor = ri.Cvar_Get ("sw_clearcolor", "2", 0);
	sw_drawflat = ri.Cvar_Get ("sw_drawflat", "0", 0);
	sw_draworder = ri.Cvar_Get ("sw_draworder", "0", 0);
	sw_maxedges = ri.Cvar_Get ("sw_maxedges", STRINGER(MAXSTACKSURFACES), 0);
	sw_maxsurfs = ri.Cvar_Get ("sw_maxsurfs", "0", 0);
	sw_mipcap = ri.Cvar_Get ("sw_mipcap", "0", 0);
	sw_mipscale = ri.Cvar_Get ("sw_mipscale", "1", 0);
	sw_reportedgeout = ri.Cvar_Get ("sw_reportedgeout", "0", 0);
	sw_reportsurfout = ri.Cvar_Get ("sw_reportsurfout", "0", 0);
	sw_stipplealpha = ri.Cvar_Get( "sw_stipplealpha", "0", CVAR_ARCHIVE );
	sw_surfcacheoverride = ri.Cvar_Get ("sw_surfcacheoverride", "0", 0);
	sw_waterwarp = ri.Cvar_Get ("sw_waterwarp", "1", 0);
	sw_mode = ri.Cvar_Get( "sw_mode", "0", CVAR_ARCHIVE );

	//r_lefthand = ri.Cvar_Get( "hand", "0", CVAR_USERINFO | CVAR_ARCHIVE );
	r_speeds = ri.Cvar_Get ("r_speeds", "0", 0);
	r_fullbright = ri.Cvar_Get ("r_fullbright", "0", 0);
	r_drawentities = ri.Cvar_Get ("r_drawentities", "1", 0);
	r_drawworld = ri.Cvar_Get ("r_drawworld", "1", 0);
	r_dspeeds = ri.Cvar_Get ("r_dspeeds", "0", 0);
	r_lightlevel = ri.Cvar_Get ("r_lightlevel", "0", 0);

    console_screen_idx = ri.Cvar_Get ("console_screen_idx", "0", CVAR_ARCHIVE); // 12.21  dsn
    if (console_screen_idx->value < 0 || console_screen_idx->value > ((NUM_RANDOM_BKG_SCREENS-1) * 2)) 
      ri.Cvar_SetValue("console_screen_idx", 0);

	r_lerpmodels = ri.Cvar_Get( "r_lerpmodels", "1", 0 );
	r_novis = ri.Cvar_Get( "r_novis", "0", 0 );
	r_dynamiclights = ri.Cvar_Get( "r_dynamiclights", "0", 0 );

	vid_fullscreen = ri.Cvar_Get( "vid_fullscreen", "0", CVAR_ARCHIVE );
	vid_gamma = ri.Cvar_Get( "vid_gamma", "1.0", CVAR_ARCHIVE );

	r_aliasframerate = ri.Cvar_Get( "r_aliasframerate", "1.0", 0 );

	r_palettedir = ri.Cvar_Get( "r_palettedir", "", 0 );

	//	Nelno:	added for per-level console and character set
	r_resourcedir = ri.Cvar_Get( "r_resourcedir", "", 0 );	
	r_alias_fogz = ri.Cvar_Get( "r_alias_fogz", "21", 0 );	

	sw_modulate = ri.Cvar_Get( "sw_modulate", "1", 0 );	
	
	ri.Cmd_AddCommand ("modellist", Mod_Modellist_f);
	ri.Cmd_AddCommand( "screenshot", R_ScreenShot_f );
	ri.Cmd_AddCommand( "imagelist", R_ImageList_f );

	sw_mode->modified = true; // force us to do mode specific stuff later
	vid_gamma->modified = true; // force us to rebuild the gamma table later

}

void R_UnRegister (void)
{
	ri.Cmd_RemoveCommand( "screenshot" );
	ri.Cmd_RemoveCommand ("modellist");
	ri.Cmd_RemoveCommand( "imagelist" );
}

/*
===============
R_Init
===============
*/
void R_BuildFogTable(); 
qboolean R_Init( void *hInstance, void *wndProc )
{
	R_Register ();

	Draw_GetPalette ();
	Draw_GetFogTable ();

	RImp_Init( hInstance, wndProc );
	// create the window
	BeginFrame( 0 );

	CreateBlendMask();
	R_BuildFogTable();

	ri.Con_Printf (PRINT_ALL, "ref_soft version: "REF_VERSION"\n");

	R_InitImages ();
	Mod_Init ();
	Draw_InitLocal ();
	R_InitTextures ();
	R_InitMiscTextures ();
	protex_InitTextures ();			//	Nelno:	init procedural texture system

	R_InitTurb ();

	view_clipplanes[0].leftedge = true;
	view_clipplanes[1].rightedge = true;
	view_clipplanes[1].leftedge = view_clipplanes[2].leftedge =
			view_clipplanes[3].leftedge = false;
	view_clipplanes[0].rightedge = view_clipplanes[2].rightedge =
			view_clipplanes[3].rightedge = false;

	r_refdef.xOrigin = XCENTERING;
	r_refdef.yOrigin = YCENTERING;

// TODO: collect 386-specific code in one place
#if	id386
	Sys_MakeCodeWriteable ((long)R_EdgeCodeStart,
					     (long)R_EdgeCodeEnd - (long)R_EdgeCodeStart);
	Sys_SetFPCW ();		// get bit masks for FPCW	(FIXME: is this id386?)
#endif	// id386

	r_aliasuvscale = 1.0;

	return true;
}

/*
===============
R_Shutdown
===============
*/
void R_Shutdown (void)
{
	// free z buffer
	if (d_pzbuffer)
	{
		free (d_pzbuffer);
		d_pzbuffer = NULL;
	}
	// free surface cache
	if (sc_base)
	{
		D_FlushCaches ();
		free (sc_base);
		sc_base = NULL;
	}

	// free colormap
	if (vid.colormap)
	{
		free (vid.colormap);
		vid.colormap = NULL;
	}
	R_UnRegister ();
	Mod_FreeAll ();
	R_ShutdownImages ();

	RImp_Shutdown();
}

/*
===============
R_NewMap
===============
*/
void R_NewMap (void)
{
	r_viewcluster = -1;

	r_cnumsurfs = sw_maxsurfs->value;

	if (r_cnumsurfs <= MINSURFACES)
		r_cnumsurfs = MINSURFACES;

	if (r_cnumsurfs > NUMSTACKSURFACES)
	{
		surfaces = (surf_t*)malloc (r_cnumsurfs * sizeof(surf_t));
		surface_p = surfaces;
		surf_max = &surfaces[r_cnumsurfs];
		r_surfsonstack = false;
	// surface 0 doesn't really exist; it's just a dummy because index 0
	// is used to indicate no edge attached to surface
		surfaces--;
		R_SurfacePatch ();
	}
	else
	{
		r_surfsonstack = true;
	}

	r_maxedgesseen = 0;
	r_maxsurfsseen = 0;

	r_numallocatededges = sw_maxedges->value;

	if (r_numallocatededges < MINEDGES)
		r_numallocatededges = MINEDGES;

	if (r_numallocatededges <= NUMSTACKEDGES)
	{
		auxedges = NULL;
	}
	else
	{
		auxedges = (edge_t*)malloc (r_numallocatededges * sizeof(edge_t));
	}
}


/*
===============
R_MarkLeaves

Mark the leaves and nodes that are in the PVS for the current
cluster
===============
*/
void R_MarkLeaves (void)
{
	byte	*vis;
	mnode_t	*node;
	int		i;
	mleaf_t	*leaf;
	int		cluster;

	if (r_oldviewcluster == r_viewcluster && !r_novis->value && r_viewcluster != -1)
		return;
	
	r_visframecount++;
	r_oldviewcluster = r_viewcluster;

	if (r_novis->value || r_viewcluster == -1 || !r_worldmodel->vis)
	{
		// mark everything
		for (i=0 ; i<r_worldmodel->numleafs ; i++)
			r_worldmodel->leafs[i].visframe = r_visframecount;
		for (i=0 ; i<r_worldmodel->numnodes ; i++)
			r_worldmodel->nodes[i].visframe = r_visframecount;
		return;
	}

	vis = Mod_ClusterPVS (r_viewcluster, r_worldmodel);
	
	for (i=0,leaf=r_worldmodel->leafs ; i<r_worldmodel->numleafs ; i++, leaf++)
	{
		cluster = leaf->cluster;
		if (cluster == -1)
			continue;
		if (vis[cluster>>3] & (1<<(cluster&7)))
		{
			node = (mnode_t *)leaf;
			do
			{
				if (node->visframe == r_visframecount)
					break;
				node->visframe = r_visframecount;
				node = node->parent;
			} while (node);
		}
	}

#if 0
	for (i=0 ; i<r_worldmodel->vis->numclusters ; i++)
	{
		if (vis[i>>3] & (1<<(i&7)))
		{
			node = (mnode_t *)&r_worldmodel->leafs[i];	// FIXME: cluster
			do
			{
				if (node->visframe == r_visframecount)
					break;
				node->visframe = r_visframecount;
				node = node->parent;
			} while (node);
		}
	}
#endif
}

/*
** R_DrawNullModel
**
** IMPLEMENT THIS!
*/
void R_DrawNullModel( void )
{
}

/*
=============
R_DrawEntitiesOnList

//	Nelno:	we could speed this up by placing translucent entities in their own list
//	Second loop would be much shorter, usually nothing...
=============
*/
void R_DrawSpotlight( entity_t *e );
void R_DrawEntitiesOnList (void)
{
	int			i;
	qboolean	translucent_entities = false;

	if (!r_drawentities->value)
		return;

	// all bmodels have already been drawn by the edge list
	for (i=0 ; i<r_newrefdef.num_entities ; i++)
	{
		currententity = &r_newrefdef.entities[i];

		if ( currententity->flags & RF_TRANSLUCENT )
		{
			translucent_entities = true;
			continue;
		}

		currentmodel = (model_t*)currententity->model;
		if (!currentmodel)
		{
			R_DrawNullModel();
			continue;
		}
		r_entorigin = currententity->origin;
		modelorg = r_origin - r_entorigin;

		switch (currentmodel->type)
		{
		case mod_sprite:
			R_DrawSprite ();
			break;

		case mod_alias:
			R_AliasDrawModel ();
			break;

		default:
			break;
		}
	}

	if ( !translucent_entities )
		return;

	for (i=0 ; i<r_newrefdef.num_entities ; i++)
	{
		currententity = &r_newrefdef.entities[i];

		r_entorigin = currententity->origin;
		modelorg = r_origin - r_entorigin;

		if ( !( currententity->flags & RF_TRANSLUCENT ) )
			continue;

		if ( currententity->flags & RF_BEAM_MOVING)
		{
			beam_DrawBeam ( currententity , r_beamTextures [BEAM_TEX_LASER]);
			continue;
		}

		if ( currententity->flags & RF_SPOTLIGHT)
		{
			R_DrawSpotlight (currententity);
			continue;
		}

		currentmodel = (model_t*)currententity->model;
		if (!currentmodel)
		{
			R_DrawNullModel();
			continue;
		}
		r_entorigin = currententity->origin;
		modelorg = r_origin - r_entorigin;

		switch (currentmodel->type)
		{
		case mod_sprite:
			R_DrawSprite ();
			break;

		case mod_alias:
			R_AliasDrawModel ();
			break;

		default:
			break;
		}
	}
}



/*
** R_DrawSpotlight
*/
void R_DrawSpotlight (entity_t *e)
{
	int		i, j;
	float	a, sina, cosa;
	CVector	length;
	float	radius1, radius2;
	CVector	start_points[17], end_points[17];
	CVector	forward, right, up;

	AngleToVectors (e->angles, forward, right, up);

	// spotlights now store the destination point in mins
	length = e->mins - e->origin;

	radius1 = e->frame;
	radius2 = radius1 + (length.Length() * 0.2);


	// create start and end points
	for (i = 16; i>=0 ; i--)
	{
		a = i/16.0 * M_PI*2;
		
		// calculate these once for speed
		sina = sin(a);
		cosa = cos(a);
		
		for (j=0 ; j<3 ; j++)
		{
			start_points[i][j] = e->origin[j] + forward[j]*radius1 + right[j] * cosa * radius1 + up[j] * sina * radius1;
			end_points[i][j] = e->mins[j] + forward[j]*radius2 + right[j] * cosa * radius2 + up[j] * sina * radius2;
		}
	}

	for ( i = 0; i < 17; i++ )
	{
		R_IMFlatShadedQuad( start_points[i],
		                    end_points[i],
							end_points[(i+1)%17],
							start_points[(i+1)%17],
							global_color[e->skinnum],
							e->alpha );
	}
}

extern polydesc_t	r_polydesc;

/*
=============
R_BmodelCheckBBox
=============
*/
int R_BmodelCheckBBox (float *minmaxs)
{
	int			i, *pindex, clipflags;
	CVector		acceptpt, rejectpt;
	float		d;

	clipflags = 0;

	for (i=0 ; i<4 ; i++)
	{
	// generate accept and reject points
	// FIXME: do with fast look-ups or integer tests based on the sign bit
	// of the floating point values

		pindex = pfrustum_indexes[i];

		rejectpt[0] = minmaxs[pindex[0]];
		rejectpt[1] = minmaxs[pindex[1]];
		rejectpt[2] = minmaxs[pindex[2]];
		
		d = DotProduct (rejectpt, view_clipplanes[i].normal);
		d -= view_clipplanes[i].dist;

		if (d <= 0)
			return BMODEL_FULLY_CLIPPED;

		acceptpt[0] = minmaxs[pindex[3+0]];
		acceptpt[1] = minmaxs[pindex[3+1]];
		acceptpt[2] = minmaxs[pindex[3+2]];

		d = DotProduct (acceptpt, view_clipplanes[i].normal);
		d -= view_clipplanes[i].dist;

		if (d <= 0)
			clipflags |= (1<<i);
	}

	return clipflags;
}


/*
===================
R_FindTopnode

Find the first node that splits the given box
===================
*/
mnode_t *R_FindTopnode (CVector &mins, CVector &maxs)
{
	mplane_t	*splitplane;
	int			sides;
	mnode_t *node;

	node = r_worldmodel->nodes;

	while (1)
	{
		if (node->visframe != r_visframecount)
			return NULL;		// not visible at all
		
		if (node->contents != CONTENTS_NODE)
		{
			if (node->contents != CONTENTS_SOLID)
				return	node; // we've reached a non-solid leaf, so it's
							//  visible and not BSP clipped
			return NULL;	// in solid, so not visible
		}
		
		splitplane = node->plane;
		sides = BOX_ON_PLANE_SIDE(mins, maxs, (cplane_t *)splitplane);
		
		if (sides == 3)
			return node;	// this is the splitter
		
	// not split yet; recurse down the contacted side
		if (sides & 1)
			node = node->children[0];
		else
			node = node->children[1];
	}
}


/*
=============
RotatedBBox

Returns an axially aligned box that contains the input box at the given rotation
=============
*/
void RotatedBBox (CVector &mins, CVector &maxs, CVector &angles, CVector &tmins, CVector &tmaxs)
{
	CVector	tmp, v;
	int		i, j;
	CVector	forward, right, up;

	if (!angles[0] && !angles[1] && !angles[2])
	{
		tmins = mins;
		tmaxs = maxs;
		return;
	}

	for (i=0 ; i<3 ; i++)
	{
		tmins[i] = 99999;
		tmaxs[i] = -99999;
	}

	AngleToVectors (angles, forward, right, up);

	for ( i = 0; i < 8; i++ )
	{
		if ( i & 1 )
			tmp[0] = mins[0];
		else
			tmp[0] = maxs[0];

		if ( i & 2 )
			tmp[1] = mins[1];
		else
			tmp[1] = maxs[1];

		if ( i & 4 )
			tmp[2] = mins[2];
		else
			tmp[2] = maxs[2];


		//VectorScale (forward, tmp[0], v);
		v = forward * tmp[0];
		VectorMA (v, right, -tmp[1], v);
		VectorMA (v, up, tmp[2], v);

		for (j=0 ; j<3 ; j++)
		{
			if (v[j] < tmins[j])
				tmins[j] = v[j];
			if (v[j] > tmaxs[j])
				tmaxs[j] = v[j];
		}
	}
}

/*
=============
R_DrawBEntitiesOnList
=============
*/
void R_DrawBEntitiesOnList (void)
{
	int			i, j, clipflags;
	CVector		oldorigin;
	CVector		mins, maxs;
	float		minmaxs[6];
	mnode_t		*topnode;

	if (!r_drawentities->value)
		return;

	oldorigin = modelorg;
	insubmodel = true;
	r_dlightframecount = r_framecount;

	for (i=0 ; i<r_newrefdef.num_entities ; i++)
	{
		currententity = &r_newrefdef.entities[i];
		currentmodel = (model_t*)currententity->model;
		if (!currentmodel)
			continue;
		if (currentmodel->nummodelsurfaces == 0)
			continue;	// clip brush only
		if ( currententity->flags & RF_BEAM )
			continue;
		if (currentmodel->type != mod_brush)
			continue;
		if ( currententity->flags & RF_SPOTLIGHT)
			continue;

		if ( currententity->flags)
		{
			ri.Con_Printf (PRINT_ALL, "entityflags = %x\n", currententity->flags);
			continue;
		}

	// see if the bounding box lets us trivially reject, also sets
	// trivial accept status
		RotatedBBox (currentmodel->mins, currentmodel->maxs,
			currententity->angles, mins, maxs);
		for(j = 0; j < 3; j++ ){
			minmaxs[j] = mins[j] + currententity->origin[j];
			minmaxs[j+3] = maxs[j] + currententity->origin[j];
		}

		clipflags = R_BmodelCheckBBox (minmaxs);
		if (clipflags == BMODEL_FULLY_CLIPPED)
		{
/*	//&&& amw: this should no longer be called here
			// if a particle volume, remove it
			ri.CL_RemoveParticleVolume ((int) currententity);
*/
			continue;	// off the edge of the screen
		}

		for(j = 0; j < 3; j++ ){
			mins[j] = minmaxs[j];
			maxs[j] = minmaxs[j+3];
		}
		topnode = R_FindTopnode (mins, maxs);
		if (!topnode)
			continue;	// no part in a visible leaf

/*	//&&& amw: this should no longer be called here
		if (currententity->flags & (RF_RAIN | RF_SNOW | RF_DRIP))
		{	
			ri.CL_GenerateVolumeParticles ((int) currententity, currententity->flags, currententity->mins, currententity->maxs);
			return;
		}
*/
		r_entorigin = currententity->origin;
		modelorg = r_origin - r_entorigin;

		r_pcurrentvertbase = currentmodel->vertexes;

	// FIXME: stop transforming twice
		R_RotateBmodel ();

	// calculate dynamic lighting for bmodel
		R_PushDlights (currentmodel);

		if (topnode->contents == CONTENTS_NODE)
		{
		// not a leaf; has to be clipped to the world BSP
			r_clipflags = clipflags;
			R_DrawSolidClippedSubmodelPolygons (currentmodel, topnode);
		}
		else
		{
		// falls entirely in one leaf, so we just put all the
		// edges in the edge list and let 1/z sorting handle
		// drawing order
			R_DrawSubmodelPolygons (currentmodel, clipflags, topnode);
		}

	// put back world rotation and frustum clipping		
	// FIXME: R_RotateBmodel should just work off base_vxx
		vpn = base_vpn;
		vup = base_vup;
		vright = base_vright;
		modelorg = oldorigin;
		R_TransformFrustum ();
	}

	insubmodel = false;
}

/*
================
R_SetupParticleVolumes
================
*/

void R_SetupParticleVolumes( void )
{
	int	i, j, clipflags = 0;
	particleVolume_t	*pVolume = NULL;
	float				minmaxs[6];

	// do we want to render particles?
//	if (r_drawparticleflag->value)	// not defined in ref_soft????
	{
		// loop through the particle volumes and generate complex
		// particles for the ones that pass the culling tests
		for (i = 0 ; i < r_newrefdef.num_particleVolumes ; i++)
		{
			// get a pointer to the particle volume record
			pVolume = &r_newrefdef.particleVolumes[i];

			// if this was marked as visible...
			if (pVolume->bVisible)
			{
				for(j = 0; j < 3; j++){
				minmaxs[j] = pVolume->mins[j];
				minmaxs[j+3] = pVolume->maxs[j];
				}

				// cull it to the view frustrum
				clipflags = R_BmodelCheckBBox (minmaxs);
				if (clipflags != BMODEL_FULLY_CLIPPED)
				{
					// generate volume particles (pVolume acts as the unique 'key')
					ri.CL_GenerateVolumeParticles ((int) pVolume, pVolume->type, pVolume->mins, pVolume->maxs, pVolume->dist, pVolume->effects);
//					g_PVDrawCount++;  // not defined in ref_soft????
				}
				else
				{
					//	remove any particle volumes from the active list that are not visible anymore
					//  NOTE: this should rarely get called since most instances are caught before
					//        the renderer is called in CL_AddParticleVolumes()
					ri.CL_RemoveParticleVolume ((int) pVolume);
//					g_PVCullCount++;  // not defined in ref_soft????
				}
			}
		}
	}
}


/*
================
R_EdgeDrawing
================
*/
void R_EdgeDrawing (void)
{
	edge_t	ledges[NUMSTACKEDGES +
				((CACHE_SIZE - 1) / sizeof(edge_t)) + 1];
	surf_t	lsurfs[NUMSTACKSURFACES +
				((CACHE_SIZE - 1) / sizeof(surf_t)) + 1];

	if ( r_newrefdef.rdflags & RDF_NOWORLDMODEL )
		return;

	if (auxedges)
	{
		r_edges = auxedges;
	}
	else
	{
		r_edges =  (edge_t *)
				(((long)&ledges[0] + CACHE_SIZE - 1) & ~(CACHE_SIZE - 1));
	}

	if (r_surfsonstack)
	{
		surfaces =  (surf_t *)
				(((long)&lsurfs[0] + CACHE_SIZE - 1) & ~(CACHE_SIZE - 1));
		surf_max = &surfaces[r_cnumsurfs];
	// surface 0 doesn't really exist; it's just a dummy because index 0
	// is used to indicate no edge attached to surface
		surfaces--;
		R_SurfacePatch ();
	}

	R_BeginEdgeFrame ();

	if (r_dspeeds->value)
	{
		rw_time1 = Sys_Milliseconds ();
	}

	R_RenderWorld ();

	if (r_dspeeds->value)
	{
		rw_time2 = Sys_Milliseconds ();
		db_time1 = rw_time2;
	}

	R_DrawBEntitiesOnList ();

	if (r_dspeeds->value)
	{
		db_time2 = Sys_Milliseconds ();
		se_time1 = db_time2;
	}

	R_ScanEdges ();
}

//=======================================================================


/*
=============
R_CalcPalette

=============
*/
void R_CalcPalette (void)
{
	static qboolean modified;
	byte	palette[256][4], *in, *out;
	int		i, j;
	float	alpha, one_minus_alpha;
	CVector	premult;
	int		v;

	alpha = r_newrefdef.blend[3];
	if (alpha <= 0)
	{
		if (modified)
		{	// set back to default
			modified = false;
			R_GammaCorrectAndSetPalette( ( const unsigned char * ) d_8to24table );
			return;
		}
		return;
	}

	modified = true;
	if (alpha > 1)
		alpha = 1;

	premult[0] = r_newrefdef.blend[0]*alpha*255;
	premult[1] = r_newrefdef.blend[1]*alpha*255;
	premult[2] = r_newrefdef.blend[2]*alpha*255;

	one_minus_alpha = (1.0 - alpha);

	in = (byte *)d_8to24table;
	out = palette[0];
	for (i=0 ; i<256 ; i++, in+=4, out+=4)
	{
		for (j=0 ; j<3 ; j++)
		{
			v = premult[j] + one_minus_alpha * in[j];
			if (v > 255)
				v = 255;
			out[j] = v;
		}
		out[3] = 255;
	}

	R_GammaCorrectAndSetPalette( ( const unsigned char * ) palette[0] );
//	SWimp_SetPalette( palette[0] );
}

//=======================================================================

void R_SetLightLevel (void)
{
	CVector		light;

	if ((r_newrefdef.rdflags & RDF_NOWORLDMODEL) || (!r_drawentities->value) || (!currententity))
	{
		r_lightlevel->value = 150.0;
		return;
	}

	// save off light value for server to look at (BIG HACK!)
	R_LightPoint (r_newrefdef.vieworg, light);
	r_lightlevel->value = 150.0 * light[0];
}

void	R_InitTiming (void)
{
	ref_curTime = 0;
	ref_frameTime = 0;
	ref_lastTime = Sys_Milliseconds ();
	ref_globalTime = 0;
}

void	R_DoRotations (void)
{
	ref_laserRotation = (ref_laserRotation + (60 * ref_frameTime * (float) ref_laserDir));
	if (ref_laserDir == 1 && ref_laserRotation > 360)
		ref_laserRotation -= 360.0;
}

/*
@@@@@@@@@@@@@@@@
RenderFrame

@@@@@@@@@@@@@@@@
*/
void RenderFrame (refdef_t *fd)
{
	r_newrefdef = *fd;

	if (!r_worldmodel && !( r_newrefdef.rdflags & RDF_NOWORLDMODEL ) )
		ri.Sys_Error (ERR_FATAL,"R_RenderView: NULL worldmodel");

	ref_curTime = Sys_Milliseconds ();
	ref_frameTime = ((float) (ref_curTime - ref_lastTime)) / 1000.0;
	ref_lastTime = ref_curTime;
	ref_globalTime += ref_frameTime;

	R_DoRotations ();

	protex_InitForFrame ();

	r_refdef.vieworg = fd->vieworg;
	r_refdef.viewangles = fd->viewangles;

	if (r_speeds->value || r_dspeeds->value)
		r_time1 = Sys_Milliseconds ();

	R_SetupFrame ();

	R_MarkLeaves ();	// done here so we know if we're in water

	R_SetupParticleVolumes ();	// generates complex particles from particle volumes

	R_PushDlights ( r_worldmodel );

	R_EdgeDrawing ();

	if (r_dspeeds->value)
	{
		se_time2 = Sys_Milliseconds ();
		de_time1 = se_time2;
	}

	R_DrawEntitiesOnList ();

	if (r_dspeeds->value)
	{
		de_time2 = Sys_Milliseconds ();
		dp_time1 = Sys_Milliseconds ();
	}

	R_DrawParticles ();

	R_DrawComParticles ();

	if (r_dspeeds->value)
		dp_time2 = Sys_Milliseconds ();

	R_DrawAlphaSurfaces();

	R_DrawBeamList ();

	R_SetLightLevel ();

//	Text_DrawEntities ();

	inventory_DrawIcons ();

//	if (r_dowarp)
//		D_WarpScreen ();

	if (r_dspeeds->value)
		da_time1 = Sys_Milliseconds ();

	if (r_dspeeds->value)
		da_time2 = Sys_Milliseconds ();

//	R_CalcPalette ();

	if (sw_aliasstats->value)
		R_PrintAliasStats ();
		
	if (r_speeds->value)
		R_PrintTimes ();

	if (r_dspeeds->value)
		R_PrintDSpeeds ();

	if (sw_reportsurfout->value && r_outofsurfaces)
		ri.Con_Printf (PRINT_ALL,"Short %d surfaces\n", r_outofsurfaces);

	if (sw_reportedgeout->value && r_outofedges)
		ri.Con_Printf (PRINT_ALL,"Short roughly %d edges\n", r_outofedges * 2 / 3);
}

/*
** R_InitGraphics
*/
void R_InitGraphics( int width, int height )
{
	vid.width  = width;
	vid.height = height;

	// free z buffer
	if ( d_pzbuffer )
	{
		free( d_pzbuffer );
		d_pzbuffer = NULL;
	}

	// free surface cache
	if ( sc_base )
	{
		D_FlushCaches ();
		free( sc_base );
		sc_base = NULL;
	}

	d_pzbuffer = (short*)malloc(vid.width*vid.height*2);

	R_InitCaches ();

	R_GammaCorrectAndSetPalette( ( const unsigned char *) d_8to24table );
}

/*
** BeginFrame
*/
void BeginFrame( float camera_separation )
{
	extern void Draw_BuildGammaTable( void );

	/*
	** rebuild the gamma correction palette if necessary
	*/
	if ( vid_gamma->modified )
	{
		Draw_BuildGammaTable();
		R_GammaCorrectAndSetPalette( ( const unsigned char * ) d_8to24table );

		vid_gamma->modified = false;
	}

	while ( sw_mode->modified || vid_fullscreen->modified )
	{
		rserr_t err;

		vid.colordepth = 16;
		/*
		** if this returns rserr_invalid_fullscreen then it set the mode but not as a
		** fullscreen mode, e.g. 320x200 on a system that doesn't support that res
		*/
		if ( ( err = RImp_SetMode( &vid.width, &vid.height, sw_mode->value, vid_fullscreen->value ) ) == rserr_ok )
		{
			R_InitGraphics( vid.width, vid.height );

			sw_state.prev_mode = sw_mode->value;
			vid_fullscreen->modified = false;
			sw_mode->modified = false;
		}
		else
		{
			if ( err == rserr_invalid_mode )
			{
				ri.Cvar_SetValue( "sw_mode", sw_state.prev_mode );
				ri.Con_Printf( PRINT_ALL, "ref_soft::R_BeginFrame() - could not set mode\n" );
			}
			else if ( err == rserr_invalid_fullscreen )
			{
				R_InitGraphics( vid.width, vid.height );

				ri.Cvar_SetValue( "vid_fullscreen", 0);
				ri.Con_Printf( PRINT_ALL, "ref_soft::R_BeginFrame() - fullscreen unavailable in this mode\n" );
				sw_state.prev_mode = sw_mode->value;
				vid_fullscreen->modified = false;
				sw_mode->modified = false;
			}
			else
			{
				ri.Sys_Error( ERR_FATAL, "ref_soft::R_BeginFrame() - catastrophic mode change failure\n" );
			}
		}
	}
	RImp_BeginFrame( 0 );
}

/*
** R_GammaCorrectAndSetPalette
*/
void R_GammaCorrectAndSetPalette( const unsigned char *palette )
{
	int i;

	for ( i = 0; i < 256; i++ )
	{
		sw_state.currentpalette[i*4+0] = sw_state.gammatable[palette[i*4+0]];
		sw_state.currentpalette[i*4+1] = sw_state.gammatable[palette[i*4+1]];
		sw_state.currentpalette[i*4+2] = sw_state.gammatable[palette[i*4+2]];
	}

	RImp_SetPalette( sw_state.currentpalette );
}

/*
** R_CinematicSetPalette
*/
void R_CinematicSetPalette( const unsigned char *palette )
{
	byte palette32[1024];
	int		i, j, w;
	int		*d;

	return;

	// clear screen to black to avoid any palette flash
	w = abs(vid.rowbytes)>>2;	// stupid negative pitch win32 stuff...
	for (i=0 ; i<vid.height ; i++, d+=w)
	{
		d = (int *)(vid.buffer + i*vid.rowbytes);
		for (j=0 ; j<w ; j++)
			d[j] = 0;
	}
	// flush it to the screen
	RImp_EndFrame ();

	if ( palette )
	{
		for ( i = 0; i < 256; i++ )
		{
			palette32[i*4+0] = palette[i*3+0];
			palette32[i*4+1] = palette[i*3+1];
			palette32[i*4+2] = palette[i*3+2];
			palette32[i*4+3] = 0xFF;
		}

		R_GammaCorrectAndSetPalette( palette32 );
	}
	else
	{
		R_GammaCorrectAndSetPalette( ( const unsigned char * ) d_8to24table );
	}
}

/*
================
Draw_BuildGammaTable
================
*/
void Draw_BuildGammaTable (void)
{
	int		i, inf;
	float	g;

	g = vid_gamma->value;

	if (g == 1.0)
	{
		for (i=0 ; i<256 ; i++)
			sw_state.gammatable[i] = i;
		return;
	}
	
	for (i=0 ; i<256 ; i++)
	{
		inf = 255 * pow ( (i+0.5)/255.5 , g ) + 0.5;
		if (inf < 0)
			inf = 0;
		if (inf > 255)
			inf = 255;
		sw_state.gammatable[i] = inf;
	}
}

/*
** R_SetInterfacePalette
*/
void R_SetInterfacePalette( const unsigned char *palette )
{
	byte		*pal;
	int			width, height;
	int			default_bmp = 0;
	static int	old_mode;

	old_mode = (int)sw_mode->value;
	if (palette != NULL)
	{
		if (sw_mode->value < 3)
		{
			RImp_SetMode( &vid.width, &vid.height, 3, vid_fullscreen->value );
			R_InitGraphics( vid.width, vid.height );
		}

        pal = NULL;
		LoadBMP ("pics/interface/colormap.bmp", &vid.colormap, &pal, &height, &width);

		vid.alphamap = vid.colormap + 64*256;

		R_SetGlobalColors (pal);

		if (!vid.colormap)
		{
			ri.Con_Printf (PRINT_ALL, "Couldn't load global colormap.  Loading default.\n");
			Draw_GetPalette ();
		}

		free (pal);
	}
	else
	{
		if (sw_mode->value < 3)
		{
			RImp_SetMode( &vid.width, &vid.height, old_mode, vid_fullscreen->value );
			R_InitGraphics( vid.width, vid.height );
		}
		Draw_GetPalette ();
	}

	if (!vid.alphamap || !vid.colormap)
		ri.Sys_Error (ERR_FATAL, "Error restoring colormap\n");
		
	R_CinematicSetPalette( palette );	
}

//===================================================================

/*
============
SetSky
============
*/
// 3dstudio environment map names
char	*suf[6] = {"rt", "bk", "lf", "ft", "up", "dn"};
int	r_skysideimage[6] = {5, 2, 4, 1, 0, 3};
extern	mtexinfo_t		r_skytexinfo[6];
void SetSky (const char *name, float rotate, CVector &axis)
{
	int		i;
	char	pathname[MAX_QPATH];

	strncpy (skyname, name, sizeof(skyname)-1);
	skyrotate = rotate;
	skyaxis = axis;

	for (i=0 ; i<6 ; i++)
	{
/*
		Com_sprintf (pathname, sizeof(pathname), "env/%s%s_s.bmp", skyname, suf[r_skysideimage[i]]);
		r_skytexinfo[i].image = (image_t*)R_FindImage (pathname, it_sky, RESOURCE_LEVEL);
		if (!r_skytexinfo[i].image)
		{
			Com_sprintf (pathname, sizeof(pathname), "env/%s%s_8.pcx", skyname, suf[r_skysideimage[i]]);
			r_skytexinfo[i].image = (image_t*)R_FindImage (pathname, it_sky, RESOURCE_LEVEL);
		}
		if (!r_skytexinfo[i].image)
			r_skytexinfo[i].image = (image_t*)R_FindImage ("env/default_8.pcx", it_sky, RESOURCE_LEVEL);
*/
		Com_sprintf (pathname, sizeof(pathname), "env/%s%s.pcx", skyname, suf[r_skysideimage[i]]);
		r_skytexinfo[i].image = (image_t*) R_FindImage (pathname, it_sky, RESOURCE_LEVEL);
		if (!r_skytexinfo[i].image){
			r_skytexinfo[i].image = (image_t*) R_FindImage ("env/default.pcx", it_sky, RESOURCE_LEVEL);
		}
	}
}

/*
===============
R_SetPaletteDir
===============
*/
void R_SetPaletteDir (const char *name)
{
	ri.Cvar_Set( "r_palettedir", name);
	Draw_GetPalette ();
	Draw_GetFogTable ();
 	R_GammaCorrectAndSetPalette( ( const unsigned char * ) d_8to24table );
}

///////////////////////////////////////////////////////////////////////////////
//	R_SetResourceDir
//
//	set the resource directory (location of console background and console chars)
//	for Daikatana 
///////////////////////////////////////////////////////////////////////////////

void R_SetResourceDir (char *name)
{
	ri.Cvar_Set( "r_resourcedir", name);

	//	Nelno:	reload console character set
	Draw_InitLocal ();
}

/*
===============
Draw_GetFogTable
===============
*/
void Draw_GetFogTable (void)
{
	int		filelen;
	int		default_fogmap = 0;
	char	filename[1024];


	//	load fog.dat from data/pics if r_palettedir->string == NULL
	if (!r_palettedir->string || r_palettedir->string [0] == 0x00)
	{
		sprintf (filename, "pics/fog.dat");
		default_fogmap = 1;
	}
	else
		sprintf (filename, "textures/%s/fog.dat", r_palettedir->string);

	filelen = ri.FS_LoadFile (filename, (void **)&vid.fogmap);
	
	if (!vid.fogmap && !default_fogmap)
	{
		//	tried the per-level bmp to no avail, so try the default colormap.bmp
		sprintf (filename, "pics/fog.dat");
		filelen = ri.FS_LoadFile (filename, (void **)&vid.fogmap);
	}

//	if (!vid.fogmap)
//		ri.Sys_Error (ERR_FATAL, "Couldn't load %s\n", filename);
}



/*
===============
Draw_GetPalette
===============
*/
void Draw_GetPalette (void)
{
	byte	*pal, *out;
	int		i;
	int		width, height;
	char	filename[1024];
	int		default_bmp = 0;

	//	Nelno:	load colormap from "data/pics/colormap.bmp" if r_palettedir->string == NULL || == ""
	if (!r_palettedir->string || r_palettedir->string [0] == 0x00)
	{
		sprintf (filename, "pics/colormap.bmp");
		default_bmp = 1;
	}
	else
		// get the palette and colormap
		sprintf (filename, "textures/%s/colormap.bmp", r_palettedir->string);

    pal = NULL;
	LoadBMP (filename, &vid.colormap, &pal, &height, &width);
	
	if (!vid.colormap && !default_bmp)
	{
		//	tried the per-level bmp to no avail, so try the default colormap.bmp
		sprintf (filename, "pics/colormap.bmp");
        pal = NULL;
		LoadBMP (filename, &vid.colormap, &pal, &height, &width);
	}

	if (!vid.colormap)
		ri.Sys_Error (ERR_FATAL, "Couldn't load %s\n", filename);

	vid.alphamap = vid.colormap + 64*256;

	out = (byte *)d_8to24table;
	for (i=0 ; i<256 ; i++, out+=4)
	{
		out[0] = pal[i*3+0];
		out[1] = pal[i*3+1];
		out[2] = pal[i*3+2];
	}	

	R_SetGlobalColors (pal);

	free (pal);
}

void *RegisterSkin (const char *name, resource_t flags);

/*
@@@@@@@@@@@@@@@@@@@@@
GetRefAPI

@@@@@@@@@@@@@@@@@@@@@
*/
/*
refexport_t	re;

void stub (CVector &origin, CVector &entityOrigin, 
		CVector &entityAngles, void *hitModel, int planeIndex, void *model, 
		int frame, float roll, float scale, byte flags)
{
}

refexport_t GetRefAPI (refimport_t rimp)
{
	ri = rimp;

	re.api_version = API_VERSION;

	re.PurgeResources		= SW_PurgeResources;
	re.BeginRegistration	= R_BeginRegistration;
	re.Mod_Free				= Mod_Free;
    re.RegisterModel		= R_RegisterModel;
    re.RegisterSkin			= R_RegisterSkin;
	re.RegisterPic			= Draw_FindPic;
	re.SetSky				= R_SetSky;
	re.SetPaletteDir		= R_SetPaletteDir;
	
	re.EndRegistration		= R_EndRegistration;

	re.RenderFrame			= R_RenderFrame;

	re.DrawPic				= DrawPic;
	re.DrawChar				= DrawChar;
	re.DrawTileClear		= Draw_TileClear;
	re.DrawFill				= Draw_Fill;

	re.DrawStretchRaw		= Draw_StretchRaw;

	re.Init					= R_Init;
	re.Shutdown				= R_Shutdown;

	re.CinematicSetPalette	= R_CinematicSetPalette;
	re.SetInterfacePalette	= R_SetInterfacePalette;
	re.BeginFrame			= R_BeginFrame;
	re.EndFrame				= SWimp_EndFrame;

	re.AppActivate			= SWimp_AppActivate;

	//	Nelno:	added for per-level console backgrounds and chars
	re.SetResourceDir		= R_SetResourceDir;
	re.DrawConsolePic		= Draw_ConsolePic;

	//  Shawn:  added this for getting frame name
	re.GetFrameName			= R_FrameNameForModel;
	re.VertInfo				= R_VertInfo;
	re.SurfIndex			= R_SurfIndex;
	re.TriVerts				= R_TriVerts;
//	re.TriVerts_Lerp		= R_TriVerts_Lerp;

	re.RegisterEpisodeSkins = Mod_RegisterEpisodeSkins;

	// amw
	re.GetGlobalColors		= R_GetGlobalColors;
	re.BestColor			= R_BestColor;
	re.GetModelSkinIndex	= R_GetModelSkinIndex;
	
	re.AddSurfaceSprite		= stub;
	Swap_Init ();

	return re;
}

#ifndef REF_HARD_LINKED
// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	ri.Sys_Error (ERR_FATAL, "%s", text);
}

void Com_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, fmt);
	vsprintf (text, fmt, argptr);
	va_end (argptr);

	ri.Con_Printf (PRINT_ALL, "%s", text);
}

#endif
*/