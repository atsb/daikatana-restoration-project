#include <stdint.h>
// r_misc.c

#include "r_local.h"

#define NUM_MIPS	4

cvar_t	*sw_mipcap;
cvar_t	*sw_mipscale;

surfcache_t		*d_initial_rover;
qboolean		d_roverwrapped;
int				d_minmip;
float			d_scalemip[NUM_MIPS-1];

static float	basemip[NUM_MIPS-1] = {1.0, 0.5*0.8, 0.25*0.8};

extern int			d_aflatcolor;

int	d_vrectx, d_vrecty, d_vrectright_particle, d_vrectbottom_particle;

int	d_pix_min, d_pix_max, d_pix_shift;

int		d_scantable[MAXHEIGHT];
short	*zspantable[MAXHEIGHT]; 

/*
================
D_Patch
================
*/
extern "C" void D_Aff8Patch( void );
extern "C" void D_PolysetAff8Start( void );

extern "C" void D_Aff8Patch_f( void );
extern "C" void D_PolysetAff8Start_f( void );

extern "C" void D_Aff8Patch_33( void );
extern "C" void D_PolysetAff8Start_33( void );

extern "C" void D_Aff8Patch_66( void );
extern "C" void D_PolysetAff8Start_66( void );

extern "C" void D_Aff8Patch_33f( void );
extern "C" void D_PolysetAff8Start_33f( void );

extern "C" void D_Aff8Patch_66f( void );
extern "C" void D_PolysetAff8Start_66f( void );

extern "C" void D_Spans16Patch_f( void );
extern "C" void D_DrawSpans16_f (espan_t *pspans);
void D_Patch (void)
{
#if id386
	static qboolean protectset8 = false;


	if (!protectset8)
	{
		Sys_MakeCodeWriteable ((int)D_PolysetAff8Start,
						     (int)D_Aff8Patch - (int)D_PolysetAff8Start);

		Sys_MakeCodeWriteable ((int)D_PolysetAff8Start_f,
						     (int)D_Aff8Patch_f - (int)D_PolysetAff8Start_f);

		Sys_MakeCodeWriteable ((int)D_PolysetAff8Start_33,
						     (int)D_Aff8Patch_33 - (int)D_PolysetAff8Start_33);

		Sys_MakeCodeWriteable ((int)D_PolysetAff8Start_66,
						     (int)D_Aff8Patch_66 - (int)D_PolysetAff8Start_66);

		Sys_MakeCodeWriteable ((int)D_PolysetAff8Start_33f,
						     (int)D_Aff8Patch_33f - (int)D_PolysetAff8Start_33f);

		Sys_MakeCodeWriteable ((int)D_PolysetAff8Start_66f,
						     (int)D_Aff8Patch_66f - (int)D_PolysetAff8Start_66f);

		Sys_MakeCodeWriteable ((int)D_DrawSpans16_f,
						     (int)D_Spans16Patch_f - (int)D_DrawSpans16_f);

		Sys_MakeCodeWriteable ((uintptr_t)R_Surf8Start,
						 (size_t)((uintptr_t)R_Surf8End - (uintptr_t)R_Surf8Start));

//		Sys_MakeCodeWriteable ((long)R_Surf16Start,
//						 (long)R_Surf16End - (long)R_Surf16Start);
		protectset8 = true;
	}

	colormap = vid.colormap;
	colormap = d_8to16table;
	alphamap = vid.alphamap;
	fogmap = vid.fogmap;
	
	R_Surf8Patch ();
//	R_Surf16Patch ();
	D_Aff8Patch();
	D_Aff8Patch_f ();
	D_Aff8Patch_33 ();
	D_Aff8Patch_66 ();
	D_Aff8Patch_33f ();
	D_Aff8Patch_66f ();
	D_Spans16Patch_f ();
#endif
}
/*
================
D_ViewChanged
================
*/
extern "C" unsigned char *alias_colormap, *alias_alphamap, *alias_fogmap;
extern "C" uint16 *skin_colormap;
extern "C" int	alias_fogdiv;

unsigned char *alias_colormap, *alias_alphamap, *alias_fogmap;
uint16 *skin_colormap;
int	alias_fogdiv;

void D_ViewChanged (void)
{
	int		i;

	scale_for_mip = xscale;
	if (yscale > xscale)
		scale_for_mip = yscale;

	d_zrowbytes = vid.width * 2;
	d_zwidth = vid.width;

	d_pix_min = r_refdef.vrect.width / 320;
	d_pix_min /= 4;
	if (d_pix_min < 1)
		d_pix_min = 1;

	d_pix_max = (int)((float)r_refdef.vrect.width / (320.0 / 4.0) + 0.5);
	d_pix_shift = 8 - (int)((float)r_refdef.vrect.width / 320.0 + 0.5);
	d_pix_max /= 4;
	if (d_pix_max < 1)
		d_pix_max = 1;

	d_vrectx = r_refdef.vrect.x;
	d_vrecty = r_refdef.vrect.y;
	d_vrectright_particle = r_refdef.vrectright - d_pix_max;
	d_vrectbottom_particle =
			r_refdef.vrectbottom - d_pix_max;

	for (i=0 ; i<vid.height; i++)
	{
		d_scantable[i] = i*r_screenwidth;
		zspantable[i] = d_pzbuffer + i*d_zwidth;
	}

	/*
	** clear Z-buffer and color-buffers if we're doing the gallery
	*/
	if ( r_newrefdef.rdflags & RDF_NOWORLDMODEL )
	{
		if ( r_newrefdef.rdflags & RDF_CLEARDEPTHBUFF	)
			memset( d_pzbuffer, 0xff, vid.width * vid.height * sizeof( d_pzbuffer[0] ) );

		// FIXME: this needs an alpha and a color vector
//		if ( r_newrefdef.rdflags & RDF_CLEARCOLORBUFF	)
//			Draw_Fill( r_newrefdef.x, r_newrefdef.y, r_newrefdef.width, r_newrefdef.height,( int ) sw_clearcolor->value & 0xff );
	}

	alias_colormap		= vid.colormap;
	alias_alphamap		= vid.alphamap;
	alias_fogmap		= vid.fogmap;
	alias_fogdiv		= (int) r_alias_fogz->value;

	D_Patch ();
}



/*
=============
R_PrintTimes
=============
*/
void R_PrintTimes (void)
{
	int		r_time2;
	int		ms;

	r_time2 = Sys_Milliseconds ();

	ms = r_time2 - r_time1;
	
	ri.Con_Printf (PRINT_ALL,"%5i ms %3i/%3i/%3i poly %3i surf\n",
				ms, c_faceclip, r_polycount, r_drawnpolycount, c_surf);
	c_surf = 0;
}


/*
=============
R_PrintDSpeeds
=============
*/
void R_PrintDSpeeds (void)
{
	int	ms, dp_time, r_time2, rw_time, db_time, se_time, de_time, da_time;

	r_time2 = Sys_Milliseconds ();

	da_time = (da_time2 - da_time1);
	dp_time = (dp_time2 - dp_time1);
	rw_time = (rw_time2 - rw_time1);
	db_time = (db_time2 - db_time1);
	se_time = (se_time2 - se_time1);
	de_time = (de_time2 - de_time1);
	ms = (r_time2 - r_time1);

	ri.Con_Printf (PRINT_ALL,"%3i %2ip %2iw %2ib %2is %2ie %2ia\n",
				ms, dp_time, rw_time, db_time, se_time, de_time, da_time);
}


/*
=============
R_PrintAliasStats
=============
*/
void R_PrintAliasStats (void)
{
	ri.Con_Printf (PRINT_ALL,"%3i polygon model drawn\n", r_amodels_drawn);
}



/*
===================
R_TransformFrustum
===================
*/
void R_TransformFrustum (void)
{
	int		i;
	CVector	v, v2;
	
	for (i=0 ; i<4 ; i++)
	{
		v[0] = screenedge[i].normal[2];
		v[1] = -screenedge[i].normal[0];
		v[2] = screenedge[i].normal[1];

		v2[0] = v[1]*vright[0] + v[2]*vup[0] + v[0]*vpn[0];
		v2[1] = v[1]*vright[1] + v[2]*vup[1] + v[0]*vpn[1];
		v2[2] = v[1]*vright[2] + v[2]*vup[2] + v[0]*vpn[2];

		view_clipplanes[i].normal = v2;

		view_clipplanes[i].dist = DotProduct (modelorg, v2);
	}
}



/*
================
TransformVector
================
*/
void TransformVector (CVector &in, CVector &out)
{
	out[0] = DotProduct(in, vright);
	out[1] = DotProduct(in, vup);
	out[2] = DotProduct(in, vpn);
}



/*
================
R_TransformPlane
================
*/
/*

void R_TransformPlane (mplane_t *p, float *normal, float *dist)
{
	float	d;
	
	d = DotProduct (r_origin, p->normal);
	*dist = p->dist - d;
// TODO: when we have rotating entities, this will need to use the view matrix
	TransformVector (p->normal, normal);
}
*/

/*
===============
R_SetUpFrustumIndexes
===============
*/
void R_SetUpFrustumIndexes (void)
{
	int		i, j, *pindex;

	pindex = r_frustum_indexes;

	for (i=0 ; i<4 ; i++)
	{
		for (j=0 ; j<3 ; j++)
		{
			if (view_clipplanes[i].normal[j] < 0)
			{
				pindex[j] = j;
				pindex[j+3] = j+3;
			}
			else
			{
				pindex[j] = j+3;
				pindex[j+3] = j;
			}
		}

	// FIXME: do just once at start
		pfrustum_indexes[i] = pindex;
		pindex += 6;
	}
}

/*
===============
R_ViewChanged

Called every time the vid structure or r_refdef changes.
Guaranteed to be called before the first refresh
===============
*/
void R_ViewChanged (vrect_t *vr)
{
	int		i;

	r_refdef.vrect = *vr;

	r_refdef.horizontalFieldOfView = 2*tan((float)r_newrefdef.fov_x/360*M_PI);;
	verticalFieldOfView = 2*tan((float)r_newrefdef.fov_y/360*M_PI);

	r_refdef.fvrectx = (float)r_refdef.vrect.x;
	r_refdef.fvrectx_adj = (float)r_refdef.vrect.x - 0.5;
	r_refdef.vrect_x_adj_shift20 = (r_refdef.vrect.x<<20) + (1<<19) - 1;
	r_refdef.fvrecty = (float)r_refdef.vrect.y;
	r_refdef.fvrecty_adj = (float)r_refdef.vrect.y - 0.5;
	r_refdef.vrectright = r_refdef.vrect.x + r_refdef.vrect.width;
	r_refdef.vrectright_adj_shift20 = (r_refdef.vrectright<<20) + (1<<19) - 1;
	r_refdef.fvrectright = (float)r_refdef.vrectright;
	r_refdef.fvrectright_adj = (float)r_refdef.vrectright - 0.5;
	r_refdef.vrectrightedge = (float)r_refdef.vrectright - 0.99;
	r_refdef.vrectbottom = r_refdef.vrect.y + r_refdef.vrect.height;
	r_refdef.fvrectbottom = (float)r_refdef.vrectbottom;
	r_refdef.fvrectbottom_adj = (float)r_refdef.vrectbottom - 0.5;

	r_refdef.aliasvrect.x = (int)(r_refdef.vrect.x * r_aliasuvscale);
	r_refdef.aliasvrect.y = (int)(r_refdef.vrect.y * r_aliasuvscale);
	r_refdef.aliasvrect.width = (int)(r_refdef.vrect.width * r_aliasuvscale);
	r_refdef.aliasvrect.height = (int)(r_refdef.vrect.height * r_aliasuvscale);
	r_refdef.aliasvrectright = r_refdef.aliasvrect.x +
			r_refdef.aliasvrect.width;
	r_refdef.aliasvrectbottom = r_refdef.aliasvrect.y +
			r_refdef.aliasvrect.height;

	xOrigin = r_refdef.xOrigin;
	yOrigin = r_refdef.yOrigin;
	
// values for perspective projection
// if math were exact, the values would range from 0.5 to to range+0.5
// hopefully they wll be in the 0.000001 to range+.999999 and truncate
// the polygon rasterization will never render in the first row or column
// but will definately render in the [range] row and column, so adjust the
// buffer origin to get an exact edge to edge fill
	xcenter = ((float)r_refdef.vrect.width * XCENTERING) +
			r_refdef.vrect.x - 0.5;
	aliasxcenter = xcenter * r_aliasuvscale;
	ycenter = ((float)r_refdef.vrect.height * YCENTERING) +
			r_refdef.vrect.y - 0.5;
	aliasycenter = ycenter * r_aliasuvscale;

	xscale = r_refdef.vrect.width / r_refdef.horizontalFieldOfView;
	aliasxscale = xscale * r_aliasuvscale;
	xscaleinv = 1.0 / xscale;

	yscale = xscale;
	aliasyscale = yscale * r_aliasuvscale;
	yscaleinv = 1.0 / yscale;
	xscaleshrink = (r_refdef.vrect.width-6)/r_refdef.horizontalFieldOfView;
	yscaleshrink = xscaleshrink;

// left side clip
	screenedge[0].normal.x = -1.0 / (xOrigin*r_refdef.horizontalFieldOfView);
	screenedge[0].normal.y = 0;
	screenedge[0].normal.z = 1;
	screenedge[0].type = PLANE_ANYZ;
	
// right side clip
	screenedge[1].normal.x =
			1.0 / ((1.0-xOrigin)*r_refdef.horizontalFieldOfView);
	screenedge[1].normal.y = 0;
	screenedge[1].normal.z = 1;
	screenedge[1].type = PLANE_ANYZ;
	
// top side clip
	screenedge[2].normal.x = 0;
	screenedge[2].normal.y = -1.0 / (yOrigin*verticalFieldOfView);
	screenedge[2].normal.z = 1;
	screenedge[2].type = PLANE_ANYZ;
	
// bottom side clip
	screenedge[3].normal.x = 0;
	screenedge[3].normal.y = 1.0 / ((1.0-yOrigin)*verticalFieldOfView);
	screenedge[3].normal.z = 1;	
	screenedge[3].type = PLANE_ANYZ;
	
	for (i=0 ; i<4 ; i++)
	{
		screenedge[i].normal.Normalize();
	}

	D_ViewChanged ();
}


/*
===============
R_SetupFrame
===============
*/
void R_SetupFrame (void)
{
	int			i;
	vrect_t		vrect;

	if (r_fullbright->modified)
	{
		r_fullbright->modified = false;
		D_FlushCaches ();	// so all lighting changes
	}
	
	r_framecount++;


// build the transformation matrix for the given view angles
	modelorg = r_refdef.vieworg;
	r_origin = r_refdef.vieworg;

	AngleToVectors (r_refdef.viewangles, vpn, vright, vup);

// current viewleaf
	if ( !( r_newrefdef.rdflags & RDF_NOWORLDMODEL ) )
	{
		r_viewleaf = Mod_PointInLeaf (r_origin, r_worldmodel);
		r_viewcluster = r_viewleaf->cluster;
	}

/*
	if (sw_waterwarp->value && (r_newrefdef.rdflags & RDF_UNDERWATER) )
		r_dowarp = true;
	else
*/		r_dowarp = false;

	if (r_dowarp)
	{	// warp into off screen buffer
		vrect.x = 0;
		vrect.y = 0;
		vrect.width = r_newrefdef.width < WARP_WIDTH ? r_newrefdef.width : WARP_WIDTH;
		vrect.height = r_newrefdef.height < WARP_HEIGHT ? r_newrefdef.height : WARP_HEIGHT;

		d_viewbuffer = (unsigned char *) r_warpbuffer;
		r_screenwidth = WARP_WIDTH;
	}
	else
	{
		vrect.x = r_newrefdef.x;
		vrect.y = r_newrefdef.y;
		vrect.width = r_newrefdef.width;
		vrect.height = r_newrefdef.height;

		d_viewbuffer = (unsigned char *)vid.buffer;
		r_screenwidth = vid.rowbytes;
	}
	
	R_ViewChanged (&vrect);

// start off with just the four screen edge clip planes
	R_TransformFrustum ();
	R_SetUpFrustumIndexes ();

// save base values
	base_vpn = vpn;
	base_vright = vright;
	base_vup = vup;

// clear frame counts
	c_faceclip = 0;
	d_spanpixcount = 0;
	r_polycount = 0;
	r_drawnpolycount = 0;
	r_wholepolycount = 0;
	r_amodels_drawn = 0;
	r_outofsurfaces = 0;
	r_outofedges = 0;

// d_setup
	d_roverwrapped = false;
	d_initial_rover = sc_rover;

	d_minmip = sw_mipcap->value;
	if (d_minmip > 3)
		d_minmip = 3;
	else if (d_minmip < 0)
		d_minmip = 0;

	for (i=0 ; i<(NUM_MIPS-1) ; i++)
		d_scalemip[i] = basemip[i] * sw_mipscale->value;

	d_aflatcolor = 0;
}


#if	!id386

/*
================
R_SurfacePatch
================
*/
void R_SurfacePatch (void)
{
	// we only patch code on Intel
}

#endif	// !id386


/* 
============================================================================== 
 
						SCREEN SHOTS 
 
============================================================================== 
*/ 


/* 
============== 
WritePCXfile 
============== 
*/ 
void WritePCXfile (char *filename, byte *data, int width, int height,
	int rowbytes, byte *palette) 
{
	int			i, j, length;
	pcx_t		*pcx;
	byte		*pack;
	FILE		*f;

	pcx = (pcx_t *)malloc (width*height*2+1000);
	if (!pcx)
		return;

	pcx->manufacturer = 0x0a;	// PCX id
	pcx->version = 5;			// 256 color
 	pcx->encoding = 1;		// uncompressed
	pcx->bits_per_pixel = 8;		// 256 color
	pcx->xmin = 0;
	pcx->ymin = 0;
	pcx->xmax = LittleShort((short)(width-1));
	pcx->ymax = LittleShort((short)(height-1));
	pcx->hres = LittleShort((short)width);
	pcx->vres = LittleShort((short)height);
	memset (pcx->palette,0,sizeof(pcx->palette));
	pcx->color_planes = 1;		// chunky image
	pcx->bytes_per_line = LittleShort((short)width);
	pcx->palette_type = LittleShort(2);		// not a grey scale
	memset (pcx->filler,0,sizeof(pcx->filler));

// pack the image
	pack = &pcx->data;
	
	for (i=0 ; i<height ; i++)
	{
		for (j=0 ; j<width ; j++)
		{
			if ( (*data & 0xc0) != 0xc0)
				*pack++ = *data++;
			else
			{
				*pack++ = 0xc1;
				*pack++ = *data++;
			}
		}

		data += rowbytes - width;
	}
			
// write the palette
	*pack++ = 0x0c;	// palette ID byte
	for (i=0 ; i<768 ; i++)
		*pack++ = *palette++;
		
// write output file 
	length = pack - (byte *)pcx;
	f = fopen (filename, "wb");
	if (!f)
		ri.Con_Printf (PRINT_ALL, "Failed to open to %s\n", filename);
	else
	{
		fwrite ((void *)pcx, 1, length, f);
		fclose (f);
	}

	free (pcx);
} 
 


/* 
================== 
R_ScreenShot_f
================== 
*/  
void R_ScreenShot_f (void) 
{ 
/*
	int			i; 
	char		pcxname[80]; 
	char		checkname[MAX_OSPATH];
	FILE		*f;
	byte		palette[768];

	// create the scrnshots directory if it doesn't exist
	Com_sprintf (checkname, sizeof(checkname), "%s/scrnshot", ri.FS_Gamedir());
	Sys_Mkdir (checkname);

// 
// find a file name to save it to 
// 
	strcpy(pcxname,"quake00.pcx");
		
	for (i=0 ; i<=99 ; i++) 
	{ 
		pcxname[5] = i/10 + '0'; 
		pcxname[6] = i%10 + '0'; 
		Com_sprintf (checkname, sizeof(checkname), "%s/scrnshot/%s", ri.FS_Gamedir(), pcxname);
		f = fopen (checkname, "r");
		if (!f)
			break;	// file doesn't exist
		fclose (f);
	} 
	if (i==100) 
	{
		ri.Con_Printf (PRINT_ALL, "R_ScreenShot_f: Couldn't create a PCX"); 
		return;
	}

	// turn the current 32 bit palette into a 24 bit palette
	for (i=0 ; i<256 ; i++)
	{
		palette[i*3+0] = sw_state.currentpalette[i*4+0];
		palette[i*3+1] = sw_state.currentpalette[i*4+1];
		palette[i*3+2] = sw_state.currentpalette[i*4+2];
	}

// 
// save the pcx file 
// 

	WritePCXfile (checkname, vid.buffer, vid.width, vid.height, vid.rowbytes,
				  palette);

	ri.Con_Printf (PRINT_ALL, "Wrote %s\n", checkname);
*/
	byte		*buffer;
	byte		*pdest;
	uint16		*psource;
	char		picname[80]; 
	char		checkname[MAX_OSPATH];
	int			i, j, c, temp;
	FILE		*f;
	int		rshift_r, rshift_g, rshift_b;
	int		lshift_r, lshift_g, lshift_b;

	rshift_r = sw_state.rgb16.remove.rgbRed;
	rshift_g = sw_state.rgb16.remove.rgbGreen;
	rshift_b = sw_state.rgb16.remove.rgbBlue;
	lshift_r = sw_state.rgb16.position.rgbRed;
	lshift_g = sw_state.rgb16.position.rgbGreen;
	lshift_b = sw_state.rgb16.position.rgbBlue;

	// create the scrnshots directory if it doesn't exist
	Com_sprintf (checkname, sizeof(checkname), "%s/scrnshot", ri.FS_Gamedir());
	Sys_Mkdir (checkname);

// 
// find a file name to save it to 
// 
	strcpy(picname,"daikatana00.tga");

	for (i=0 ; i<=99 ; i++) 
	{ 
		picname[9] = i/10 + '0'; 
		picname[10] = i%10 + '0'; 
		Com_sprintf (checkname, sizeof(checkname), "%s/scrnshot/%s", ri.FS_Gamedir(), picname);
		f = fopen (checkname, "rb");
		if (!f)
			break;	// file doesn't exist
		fclose (f);
	} 
	if (i==100) 
	{
		ri.Con_Printf (PRINT_ALL, "SCR_ScreenShot_f: Couldn't create a file\n"); 
		return;
 	}


	buffer = (unsigned char*)malloc(vid.width*vid.height*3 + 18);
	memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = vid.width&255;
	buffer[13] = vid.width>>8;
	buffer[14] = vid.height&255;
	buffer[15] = vid.height>>8;
	buffer[16] = 24;	// pixel size

	pdest = buffer+18;
	psource = (uint16*)vid.buffer + vid.width  * (vid.height - 1);

	for(i = 0; i < vid.height; i++ )
	{
		for(j = 0; j < vid.width; j++ )
		{
			*pdest++ = (psource[j] >> lshift_r) << rshift_r;
			*pdest++ = (psource[j] >> lshift_g) << rshift_g;
			*pdest++ = (psource[j] >> lshift_b) << rshift_b;
		}
		psource -= vid.width;
	}

	// swap rgb to bgr
	c = 18+vid.width*vid.height*3;
	for (i=18 ; i<c ; i+=3)
	{
		temp = buffer[i];
		buffer[i] = buffer[i+2];
		buffer[i+2] = temp;
	}

	f = fopen (checkname, "wb");
	fwrite (buffer, 1, c, f);
	fclose (f);

	free (buffer);
	ri.Con_Printf (PRINT_ALL, "Wrote %s\n", picname);
}

// ============================================================================
// Savegame screenshot support
//
// The common renderer API expects a 256x256, uncompressed 24-bit TGA image
// to be captured and retained until SaveSavegameScreenShot() is called.
// The software renderer's framebuffer is 16-bit RGB, so convert from the
// active DirectDraw pixel masks using the same shift information used by the
// normal software-renderer screenshot path above.
// ============================================================================

static byte *pSavegamePic = NULL;

#define SAVEGAME_SCREENSHOT_WIDTH   256
#define SAVEGAME_SCREENSHOT_HEIGHT  256
#define SAVEGAME_TGA_HEADER_SIZE    18
#define SAVEGAME_IMAGE_START        SAVEGAME_TGA_HEADER_SIZE
#define SAVEGAME_IMAGE_SIZE         ((SAVEGAME_SCREENSHOT_WIDTH * SAVEGAME_SCREENSHOT_HEIGHT * 3) + SAVEGAME_TGA_HEADER_SIZE)

static void SoftSavegameWritePixel(byte *dest, uint16 pixel)
{
    int red;
    int green;
    int blue;

    red = ((pixel >> sw_state.rgb16.position.rgbRed) & sw_state.rgb16.depth.rgbRed) << sw_state.rgb16.remove.rgbRed;
    green = ((pixel >> sw_state.rgb16.position.rgbGreen) & sw_state.rgb16.depth.rgbGreen) << sw_state.rgb16.remove.rgbGreen;
    blue = ((pixel >> sw_state.rgb16.position.rgbBlue) & sw_state.rgb16.depth.rgbBlue) << sw_state.rgb16.remove.rgbBlue;

    // TGA stores BGR with the default bottom-left image origin.
    dest[0] = (byte)(blue & 0xff);
    dest[1] = (byte)(green & 0xff);
    dest[2] = (byte)(red & 0xff);
}

void GetSavegameScreenShot()
{
    int x;
    int y;
    int src_x;
    int src_y;
    int src_pitch;
    byte *dest;
    const uint16 *src_row;

    // Match the renderer contract: one retained screenshot at a time.
    if (pSavegamePic != NULL)
    {
        ri.X_Free(pSavegamePic);
        pSavegamePic = NULL;
    }

    pSavegamePic = (byte *)ri.X_Malloc(SAVEGAME_IMAGE_SIZE, MEM_TAG_MISC);
    if (pSavegamePic == NULL)
        return;

    memset(pSavegamePic, 0, SAVEGAME_TGA_HEADER_SIZE);
    pSavegamePic[2] = 2;  // uncompressed true-color image
    pSavegamePic[12] = SAVEGAME_SCREENSHOT_WIDTH & 255;
    pSavegamePic[13] = SAVEGAME_SCREENSHOT_WIDTH >> 8;
    pSavegamePic[14] = SAVEGAME_SCREENSHOT_HEIGHT & 255;
    pSavegamePic[15] = SAVEGAME_SCREENSHOT_HEIGHT >> 8;
    pSavegamePic[16] = 24;

    if (vid.buffer == NULL || vid.width <= 0 || vid.height <= 0 ||
        vid.width < SAVEGAME_SCREENSHOT_WIDTH ||
        vid.height < SAVEGAME_SCREENSHOT_HEIGHT)
    {
        return;
    }

    src_pitch = vid.pixelwidth;
    if (src_pitch < vid.width)
        src_pitch = vid.width;

    dest = pSavegamePic + SAVEGAME_IMAGE_START;

    // Write the TGA from bottom row to top row, matching the software
    // renderer's normal screenshot orientation.
    for (y = 0; y < SAVEGAME_SCREENSHOT_HEIGHT; ++y)
    {
        src_y = vid.height - 1 - ((y * vid.height) / SAVEGAME_SCREENSHOT_HEIGHT);
        src_row = (const uint16 *)(vid.buffer + src_y * vid.rowbytes);

        for (x = 0; x < SAVEGAME_SCREENSHOT_WIDTH; ++x)
        {
            src_x = (x * vid.width) / SAVEGAME_SCREENSHOT_WIDTH;
            SoftSavegameWritePixel(dest, src_row[src_x]);
            dest += 3;
        }
    }
}

void ClearSavegameScreenShot()
{
    if (pSavegamePic == NULL)
        return;

    ri.X_Free(pSavegamePic);
    pSavegamePic = NULL;
}

void SaveSavegameScreenShot(char *path, char *savename)
{
    FILE *f;
    char name[32];
    char filename[128];
    char c;

    if (pSavegamePic == NULL || savename == NULL)
        return;

    _splitpath(savename, NULL, NULL, name, NULL);

    if (path == NULL || path[0] == '\0')
    {
        Com_sprintf(filename, sizeof(filename), "%s.tga", name);
    }
    else
    {
        c = *(path + strlen(path) - 1);
        if (c != '/' && c != '\\')
            Com_sprintf(filename, sizeof(filename), "%s/%s.tga", path, name);
        else
            Com_sprintf(filename, sizeof(filename), "%s%s.tga", path, name);
    }

    f = fopen(filename, "wb");
    if (f == NULL)
    {
        ri.Con_Printf(PRINT_ALL, "Failed to open %s\n", filename);
        return;
    }

    fwrite(pSavegamePic, 1, SAVEGAME_IMAGE_SIZE, f);
    fclose(f);
}

