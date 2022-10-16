// r_surf.c: surface-related refresh code

#include "r_local.h"

drawsurf_t	r_drawsurf;

extern "C" int		lightleft, sourcesstep, blocksize, sourcetstep;
extern "C" int		lightdelta, lightdeltastep;
extern "C" int		lightright, lightleftstep, lightrightstep, blockdivshift;
extern "C" void		*prowdestbase;
//extern "C" uint16	*pbasesource;
extern "C" byte		*pbasesource;
extern "C" int		surfrowbytes;	// used by ASM files
//extern "C" byte		*r_lightptr;
extern "C" uint16		*r_lightptr;
extern "C" int		r_stepback;
extern "C" int		r_lightwidth;
extern "C" int		r_numhblocks, r_numvblocks;
//extern "C" uint16	*r_source, *r_sourcemax;
extern "C" byte	*r_source, *r_sourcemax;

int				lightleft, sourcesstep, blocksize, sourcetstep;
int				lightdelta, lightdeltastep;
int				lightright, lightleftstep, lightrightstep, blockdivshift;
void			*prowdestbase;
//uint16			*pbasesource;
byte			*pbasesource;
int				surfrowbytes;	// used by ASM files
//byte			*r_lightptr;
uint16			*r_lightptr;
int				r_stepback;
int				r_lightwidth;
int				r_numhblocks, r_numvblocks;
//uint16			*r_source, *r_sourcemax;
byte			*r_source, *r_sourcemax;

int		sb_v;
int		rowbytes;
int		tstep;
int		stepback;

int		rshift_r, rshift_g, rshift_b;
int		lshift_r, lshift_g, lshift_b;

extern "C" void R_DrawSurfaceBlock8_mip0();
extern "C" void R_DrawSurfaceBlock8_mip1();
extern "C" void R_DrawSurfaceBlock8_mip2();
extern "C" void R_DrawSurfaceBlock8_mip3();

static void	(*surfmiptable[4])(void) = {
	R_DrawSurfaceBlock8_mip0,
	R_DrawSurfaceBlock8_mip1,
	R_DrawSurfaceBlock8_mip2,
	R_DrawSurfaceBlock8_mip3
};

extern "C" void R_DrawSurfaceBlock16_mip0( uint16 *palette );
extern "C" void R_DrawSurfaceBlock16_mip1( uint16 *palette );
extern "C" void R_DrawSurfaceBlock16_mip2( uint16 *palette );
extern "C" void R_DrawSurfaceBlock16_mip3( uint16 *palette );

static void	(*surfmiptable16[4])( uint16 *palette ) = {
	R_DrawSurfaceBlock16_mip0,
	R_DrawSurfaceBlock16_mip1,
	R_DrawSurfaceBlock16_mip2,
	R_DrawSurfaceBlock16_mip3
};

void R_BuildLightMap (void);
void R_BuildColorLightMap (void);
//extern	byte		blocklights[2048];	// allow some very large lightmaps
extern	uint16		blocklights[4096];	// allow some very large lightmaps

float           surfscale;
qboolean        r_cache_thrash;         // set if surface cache is thrashing

int         sc_size;
surfcache_t	*sc_rover, *sc_base;

/*
===============
R_TextureAnimation

Returns the proper texture for a given time and base texture
===============
*/
image_t *R_TextureAnimation (mtexinfo_t *tex)
{
	int		c;

	if (!tex->next)
		return tex->image;

	c = currententity->frame % tex->numframes;
	while (c)
	{
		tex = tex->next;
		c--;
	}

	return tex->image;
}


/*
===============
R_DrawSurface
===============
*/
void R_DrawSurface (void)
{
	byte			*basetptr;
	int				smax, tmax, twidth;
	int				u;
	int				soffset, basetoffset, texwidth;
	int				horzblockstep;
	byte			*pcolumndest;
	void			(*pblockdrawer)( uint16 *palette );
	image_t			*mt;
	uint16			*surface_palette;

	rshift_r = sw_state.rgb16.remove.rgbRed;
	rshift_g = sw_state.rgb16.remove.rgbGreen;
	rshift_b = sw_state.rgb16.remove.rgbBlue;
	lshift_r = sw_state.rgb16.position.rgbRed;
	lshift_g = sw_state.rgb16.position.rgbGreen;
	lshift_b = sw_state.rgb16.position.rgbBlue;

	surfrowbytes = r_drawsurf.rowbytes;						// number of bytes in a row (not necessarily the same as number of pixels)

	mt = r_drawsurf.image;									// texture info
	
	r_source = mt->pixels[r_drawsurf.surfmip];				// pointer to bitmap
	
	surface_palette = mt->palette;							// pointer to palette

	texwidth = mt->width >> r_drawsurf.surfmip;				// texture width

	blocksize = 16 >> r_drawsurf.surfmip;					// blocksize for lightmap(?)
	blockdivshift = 4 - r_drawsurf.surfmip;					// 

	r_lightwidth = (r_drawsurf.surf->extents[0]>>4)+1;		// the lightmaps are 1/16th the size of the surface
	r_lightwidth *= 4; // 4 bytes in a lightmap

	r_numhblocks = r_drawsurf.surfwidth >> blockdivshift;	// number of horizontal 16x16 blocks
	r_numvblocks = r_drawsurf.surfheight >> blockdivshift;	// number of vertical 16x16 blocks

	//==============================

	pblockdrawer = surfmiptable16[r_drawsurf.surfmip];

// TODO: only needs to be set when there is a display settings change
	horzblockstep = blocksize * BYTES_PER_PIXEL;

	smax = mt->width >> r_drawsurf.surfmip;				// maximum s value
	twidth = texwidth;									// texture width
	tmax = mt->height >> r_drawsurf.surfmip;			// maximum t value
	sourcetstep = texwidth;
	r_stepback = tmax * twidth;

	r_sourcemax = r_source + (tmax * smax);

	soffset = r_drawsurf.surf->texturemins[0];
	basetoffset = r_drawsurf.surf->texturemins[1];

// << 16 components are to guarantee positive values for %
	soffset = ((soffset >> r_drawsurf.surfmip) + (smax << 16)) % smax;
	basetptr = &r_source[((((basetoffset >> r_drawsurf.surfmip) 
		+ (tmax << 16)) % tmax) * twidth)];

	pcolumndest = r_drawsurf.surfdat;

	for (u=0 ; u<r_numhblocks; u++)
	{
		r_lightptr = blocklights + (u * 4);

		prowdestbase = pcolumndest;

		pbasesource = basetptr + soffset;

//		(*pblockdrawer)();
		(*pblockdrawer)( mt->palette );

		soffset = soffset + blocksize - 1;
		if (soffset >= smax)
			soffset = 0;

		pcolumndest += horzblockstep;
	}
}


//=============================================================================

#if	!id386

/*
================
R_DrawSurfaceBlock8_mip0
================
*/
void R_DrawSurfaceBlock8_mip0 (void)
{
	int				v, i, b, lightstep, lighttemp, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 4;
		lightrightstep = (r_lightptr[1] - lightright) >> 4;

		for (i=0 ; i<16 ; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 4;

			light = lightright;

			for (b=15; b>=0; b--)
			{
				pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)
						[(light & 0xFF00) + pix];
				light += lightstep;
			}
	
			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}


/*
================
R_DrawSurfaceBlock8_mip1
================
*/
void R_DrawSurfaceBlock8_mip1 (void)
{
	int				v, i, b, lightstep, lighttemp, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 3;
		lightrightstep = (r_lightptr[1] - lightright) >> 3;

		for (i=0 ; i<8 ; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 3;

			light = lightright;

			for (b=7; b>=0; b--)
			{
				pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)
						[(light & 0xFF00) + pix];
				light += lightstep;
			}
	
			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}


/*
================
R_DrawSurfaceBlock8_mip2
================
*/
void R_DrawSurfaceBlock8_mip2 (void)
{
	int				v, i, b, lightstep, lighttemp, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 2;
		lightrightstep = (r_lightptr[1] - lightright) >> 2;

		for (i=0 ; i<4 ; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 2;

			light = lightright;

			for (b=3; b>=0; b--)
			{
				pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)
						[(light & 0xFF00) + pix];
				light += lightstep;
			}
	
			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}


/*
================
R_DrawSurfaceBlock8_mip3
================
*/
void R_DrawSurfaceBlock8_mip3 (void)
{
	int				v, i, b, lightstep, lighttemp, light;
	unsigned char	pix, *psource, *prowdest;

	psource = pbasesource;
	prowdest = prowdestbase;

	for (v=0 ; v<r_numvblocks ; v++)
	{
	// FIXME: make these locals?
	// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 1;
		lightrightstep = (r_lightptr[1] - lightright) >> 1;

		for (i=0 ; i<2 ; i++)
		{
			lighttemp = lightleft - lightright;
			lightstep = lighttemp >> 1;

			light = lightright;

			for (b=1; b>=0; b--)
			{
				pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)
						[(light & 0xFF00) + pix];
				light += lightstep;
			}
	
			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

#endif


//=============================================================================

#if 1
/*
================
R_DrawSurfaceBlock16_mip0
================
*/
void R_DrawSurfaceBlock16_mip0( uint16 *palette )
{
	int				v, i, b;//, lightstep, lighttemp, light;
	byte			*psource;
	uint16			*prowdest, pix;
	byte			dr, dg, db;
	byte			sr, sg, sb;
	int				leftcolor[3], rightcolor[3], lightcolor[3];
	int				leftdelta[3], rightdelta[3], lightdelta[3];
		
	psource = pbasesource;
	prowdest = (uint16*) prowdestbase;

	for ( v = 0; v < r_numvblocks; v++ ){
		leftcolor[0] = r_lightptr[0];
		leftcolor[1] = r_lightptr[1];
		leftcolor[2] = r_lightptr[2];
		
		rightcolor[0] = r_lightptr[4];
		rightcolor[1] = r_lightptr[5];
		rightcolor[2] = r_lightptr[6];

		r_lightptr += r_lightwidth;

		leftdelta[0] = (r_lightptr[0]	- leftcolor[0]) >> 4;
		leftdelta[1] = (r_lightptr[1]	- leftcolor[1]) >> 4;
		leftdelta[2] = (r_lightptr[2]	- leftcolor[2]) >> 4;

		rightdelta[0] = (r_lightptr[4]	- rightcolor[0]) >> 4;
		rightdelta[1] = (r_lightptr[5]	- rightcolor[1]) >> 4;
		rightdelta[2] = (r_lightptr[6]	- rightcolor[2]) >> 4;

		for ( i = 0; i < 16; i++ ){
			lightdelta[0] = (leftcolor[0] - rightcolor[0]) >> 4;
			lightdelta[1] = (leftcolor[1] - rightcolor[1]) >> 4;
			lightdelta[2] = (leftcolor[2] - rightcolor[2]) >> 4;

			lightcolor[0] = rightcolor[0];
			lightcolor[1] = rightcolor[1];
			lightcolor[2] = rightcolor[2];

			for ( b = 15; b >= 0; b-- ){
				if( psource[b] != TRANSPARENT_COLOR ){
					sr = (palette[psource[b]] >> lshift_r) << rshift_r;
					sg = (palette[psource[b]] >> lshift_g) << rshift_g;
					sb = (palette[psource[b]] >> lshift_b) << rshift_b;
					dr = ((sr + lightcolor[0]) >> 1) >> rshift_r;
					dg = ((sg + lightcolor[1]) >> 1) >> rshift_g;
					db = ((sb + lightcolor[2]) >> 1) >> rshift_b;

					prowdest[b] = (dr << lshift_r) | (dg << lshift_g) | (db << lshift_b);
				}
				else{
					prowdest[b] = TRANSPARENT_COLOR;
				}

				lightcolor[0] += lightdelta[0];
				lightcolor[1] += lightdelta[1];
				lightcolor[2] += lightdelta[2];
			}
	
			psource += sourcetstep;

			rightcolor[0] += rightdelta[0];
			rightcolor[1] += rightdelta[1];
			rightcolor[2] += rightdelta[2];
			leftcolor[0] += leftdelta[0];
			leftcolor[1] += leftdelta[1];
			leftcolor[2] += leftdelta[2];

			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

/*
================
R_DrawSurfaceBlock16_mip1
================
*/
void R_DrawSurfaceBlock16_mip1( uint16 *palette )
{
	int				v, i, b;//, lightstep, lighttemp, light;
	byte			*psource;
	uint16			*prowdest, pix;
	byte			dr, dg, db;
	byte			sr, sg, sb;
	int				leftcolor[3], rightcolor[3], lightcolor[3];
	int				leftdelta[3], rightdelta[3], lightdelta[3];
		
	psource = pbasesource;
	prowdest = (uint16*) prowdestbase;

	for ( v = 0; v < r_numvblocks; v++){
		leftcolor[0] = r_lightptr[0];
		leftcolor[1] = r_lightptr[1];
		leftcolor[2] = r_lightptr[2];
		
		rightcolor[0] = r_lightptr[4];
		rightcolor[1] = r_lightptr[5];
		rightcolor[2] = r_lightptr[6];

		r_lightptr += r_lightwidth;

		leftdelta[0] = (r_lightptr[0]	- leftcolor[0]) >> 3;
		leftdelta[1] = (r_lightptr[1]	- leftcolor[1]) >> 3;
		leftdelta[2] = (r_lightptr[2]	- leftcolor[2]) >> 3;

		rightdelta[0] = (r_lightptr[4]	- rightcolor[0]) >> 3;
		rightdelta[1] = (r_lightptr[5]	- rightcolor[1]) >> 3;
		rightdelta[2] = (r_lightptr[6]	- rightcolor[2]) >> 3;

		for ( i = 0; i < 8; i++ ){
			lightdelta[0] = (leftcolor[0] - rightcolor[0]) >> 3;
			lightdelta[1] = (leftcolor[1] - rightcolor[1]) >> 3;
			lightdelta[2] = (leftcolor[2] - rightcolor[2]) >> 3;

			lightcolor[0] = rightcolor[0];
			lightcolor[1] = rightcolor[1];
			lightcolor[2] = rightcolor[2];

			for ( b = 7; b >= 0; b-- ){
				if( psource[b] != TRANSPARENT_COLOR ){
					sr = (palette[psource[b]] >> lshift_r) << rshift_r;
					sg = (palette[psource[b]] >> lshift_g) << rshift_g;
					sb = (palette[psource[b]] >> lshift_b) << rshift_b;
					dr = ((sr + lightcolor[0]) >> 1) >> rshift_r;
					dg = ((sg + lightcolor[1]) >> 1) >> rshift_g;
					db = ((sb + lightcolor[2]) >> 1) >> rshift_b;

					prowdest[b] = (dr << lshift_r) | (dg << lshift_g) | (db << lshift_b);
				}
				else{
					prowdest[b] = TRANSPARENT_COLOR;
				}

				lightcolor[0] += lightdelta[0];
				lightcolor[1] += lightdelta[1];
				lightcolor[2] += lightdelta[2];
			}
	
			psource += sourcetstep;

			rightcolor[0] += rightdelta[0];
			rightcolor[1] += rightdelta[1];
			rightcolor[2] += rightdelta[2];
			leftcolor[0] += leftdelta[0];
			leftcolor[1] += leftdelta[1];
			leftcolor[2] += leftdelta[2];

			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

/*
================
R_DrawSurfaceBlock16_mip2
================
*/
void R_DrawSurfaceBlock16_mip2( uint16 *palette )
{
	int				v, i, b;//, lightstep, lighttemp, light;
	byte			*psource;
	uint16			*prowdest, pix;
	byte			dr, dg, db;
	byte			sr, sg, sb;
	int				leftcolor[3], rightcolor[3], lightcolor[3];
	int				leftdelta[3], rightdelta[3], lightdelta[3];
		
	psource = pbasesource;
	prowdest = (uint16*) prowdestbase;

	for ( v = 0; v < r_numvblocks; v++ ){
		leftcolor[0] = r_lightptr[0];
		leftcolor[1] = r_lightptr[1];
		leftcolor[2] = r_lightptr[2];
		
		rightcolor[0] = r_lightptr[4];
		rightcolor[1] = r_lightptr[5];
		rightcolor[2] = r_lightptr[6];

		r_lightptr += r_lightwidth;

		leftdelta[0] = (r_lightptr[0]	- leftcolor[0]) >> 2;
		leftdelta[1] = (r_lightptr[1]	- leftcolor[1]) >> 2;
		leftdelta[2] = (r_lightptr[2]	- leftcolor[2]) >> 2;

		rightdelta[0] = (r_lightptr[4]	- rightcolor[0]) >> 2;
		rightdelta[1] = (r_lightptr[5]	- rightcolor[1]) >> 2;
		rightdelta[2] = (r_lightptr[6]	- rightcolor[2]) >> 2;

		for ( i = 0; i < 4; i++ ){
			lightdelta[0] = (leftcolor[0] - rightcolor[0]) >> 2;
			lightdelta[1] = (leftcolor[1] - rightcolor[1]) >> 2;
			lightdelta[2] = (leftcolor[2] - rightcolor[2]) >> 2;

			lightcolor[0] = rightcolor[0];
			lightcolor[1] = rightcolor[1];
			lightcolor[2] = rightcolor[2];

			for ( b = 3; b >= 0; b-- ){
				if( psource[b] != TRANSPARENT_COLOR ){
					sr = (palette[psource[b]] >> lshift_r) << rshift_r;
					sg = (palette[psource[b]] >> lshift_g) << rshift_g;
					sb = (palette[psource[b]] >> lshift_b) << rshift_b;
					dr = ((sr + lightcolor[0]) >> 1) >> rshift_r;
					dg = ((sg + lightcolor[1]) >> 1) >> rshift_g;
					db = ((sb + lightcolor[2]) >> 1) >> rshift_b;

					prowdest[b] = (dr << lshift_r) | (dg << lshift_g) | (db << lshift_b);
				}
				else{
					prowdest[b] = TRANSPARENT_COLOR;
				}

				lightcolor[0] += lightdelta[0];
				lightcolor[1] += lightdelta[1];
				lightcolor[2] += lightdelta[2];
			}
	
			psource += sourcetstep;

			rightcolor[0] += rightdelta[0];
			rightcolor[1] += rightdelta[1];
			rightcolor[2] += rightdelta[2];
			leftcolor[0] += leftdelta[0];
			leftcolor[1] += leftdelta[1];
			leftcolor[2] += leftdelta[2];

			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

/*
================
R_DrawSurfaceBlock16_mip3
================
*/
void R_DrawSurfaceBlock16_mip3( uint16 *palette )
{
	int				v, i, b;//, lightstep, lighttemp, light;
	byte			*psource;
	uint16			*prowdest, pix;
	byte			dr, dg, db;
	byte			sr, sg, sb;
	int				leftcolor[3], rightcolor[3], lightcolor[3];
	int				leftdelta[3], rightdelta[3], lightdelta[3];
		
	psource = pbasesource;
	prowdest = (uint16*) prowdestbase;

	for ( v = 0; v < r_numvblocks; v++ ){
		leftcolor[0] = r_lightptr[0];
		leftcolor[1] = r_lightptr[1];
		leftcolor[2] = r_lightptr[2];
		
		rightcolor[0] = r_lightptr[4];
		rightcolor[1] = r_lightptr[5];
		rightcolor[2] = r_lightptr[6];

		r_lightptr += r_lightwidth;

		leftdelta[0] = (r_lightptr[0]	- leftcolor[0]) >> 1;
		leftdelta[1] = (r_lightptr[1]	- leftcolor[1]) >> 1;
		leftdelta[2] = (r_lightptr[2]	- leftcolor[2]) >> 1;

		rightdelta[0] = (r_lightptr[4]	- rightcolor[0]) >> 1;
		rightdelta[1] = (r_lightptr[5]	- rightcolor[1]) >> 1;
		rightdelta[2] = (r_lightptr[6]	- rightcolor[2]) >> 1;
		
		for (i = 0; i < 2; i++ ){
			lightdelta[0] = (leftcolor[0] - rightcolor[0]) >> 1;
			lightdelta[1] = (leftcolor[1] - rightcolor[1]) >> 1;
			lightdelta[2] = (leftcolor[2] - rightcolor[2]) >> 1;
			
			lightcolor[0] = rightcolor[0];
			lightcolor[1] = rightcolor[1];
			lightcolor[2] = rightcolor[2];

			for ( b = 1; b >= 0; b-- ){
				if( psource[b] != TRANSPARENT_COLOR ){
					sr = (palette[psource[b]] >> lshift_r) << rshift_r;
					sg = (palette[psource[b]] >> lshift_g) << rshift_g;
					sb = (palette[psource[b]] >> lshift_b) << rshift_b;
					dr = ((sr + lightcolor[0]) >> 1) >> rshift_r;
					dg = ((sg + lightcolor[1]) >> 1) >> rshift_g;
					db = ((sb + lightcolor[2]) >> 1) >> rshift_b;

					prowdest[b] = (dr << lshift_r) | (dg << lshift_g) | (db << lshift_b);
				}
				else{
					prowdest[b] = TRANSPARENT_COLOR;
				}

				lightcolor[0] += lightdelta[0];
				lightcolor[1] += lightdelta[1];
				lightcolor[2] += lightdelta[2];
			}
	
			psource += sourcetstep;

			rightcolor[0] += rightdelta[0];
			rightcolor[1] += rightdelta[1];
			rightcolor[2] += rightdelta[2];
			leftcolor[0] += leftdelta[0];
			leftcolor[1] += leftdelta[1];
			leftcolor[2] += leftdelta[2];

			prowdest += surfrowbytes;
		}

		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}
#endif
//============================================================================


/*
================
R_InitCaches

================
*/
void R_InitCaches (void)
{
	int		size;
	int		pix;

	// calculate size to allocate
	if (sw_surfcacheoverride->value)
	{
		size = sw_surfcacheoverride->value;			// use overridden surface cache size
	}
	else
	{
		size = SURFCACHE_SIZE_AT_320X240;

		pix = vid.width*vid.height;
		if (pix > 64000)
			size += (pix-64000)*3;
	}		

	// round up to page size
	size = (size + 8191) & ~8191;

	ri.Con_Printf (PRINT_ALL,"%ik surface cache\n", size/1024);

	sc_size = size * BYTES_PER_PIXEL;
//	sc_size = size;
	sc_base = (surfcache_t *) malloc(size * BYTES_PER_PIXEL);
//	sc_base = (surfcache_t *) malloc(size);
	sc_rover = sc_base;
	
	sc_base->next = NULL;
	sc_base->owner = NULL;
	sc_base->size = sc_size;
}


/*
==================
D_FlushCaches
==================
*/
void D_FlushCaches (void)
{
	surfcache_t     *cache;

	if (!sc_base)
		return;

	for (cache = sc_base ; cache ; cache = cache->next)
	{
		if (cache->owner)
			*cache->owner = NULL;
	}
	
	sc_rover = sc_base;
	sc_base->next = NULL;
	sc_base->owner = NULL;
	sc_base->size = sc_size;
}

/*
=================
D_SCAlloc
=================
*/
surfcache_t     *D_SCAlloc (int width, int size)
{
//	surfcache_t             *new;
	surfcache_t             *sc_new;
	qboolean                wrapped_this_time;

	if ((width < 0) || (width > 256))
		ri.Sys_Error (ERR_FATAL,"D_SCAlloc: bad cache width %d\n", width);

	if ((size <= 0) || (size > 0x10000))
		ri.Sys_Error (ERR_FATAL,"D_SCAlloc: bad cache size %d\n", size);
	
	size = (int)&((surfcache_t *)0)->data[size];
	size *= BYTES_PER_PIXEL;
	size = (size + 3) & ~3;
	if (size > sc_size)
		ri.Sys_Error (ERR_FATAL,"D_SCAlloc: %i > cache size of %i",size, sc_size);

// if there is not size bytes after the rover, reset to the start
	wrapped_this_time = false;

	if ( !sc_rover || (byte *)sc_rover - (byte *)sc_base > sc_size - size)
	{
		if (sc_rover)
		{
			wrapped_this_time = true;
		}
		sc_rover = sc_base;
	}
		
// colect and free surfcache_t blocks until the rover block is large enough
	sc_new = sc_rover;
	if (sc_rover->owner)
		*sc_rover->owner = NULL;
	
	while (sc_new->size < size)
	{
	// free another
		sc_rover = sc_rover->next;
		if (!sc_rover)
			ri.Sys_Error (ERR_FATAL,"D_SCAlloc: hit the end of memory");
		if (sc_rover->owner)
			*sc_rover->owner = NULL;
			
		sc_new->size += sc_rover->size;
		sc_new->next = sc_rover->next;
	}

// create a fragment out of any leftovers
	if (sc_new->size - size > 256)
	{
		sc_rover = (surfcache_t *)( (byte *)sc_new + size);
		sc_rover->size = sc_new->size - size;
		sc_rover->next = sc_new->next;
		sc_rover->width = 0;
		sc_rover->owner = NULL;
		sc_new->next = sc_rover;
		sc_new->size = size;
	}
	else
		sc_rover = sc_new->next;
	
	sc_new->width = width;
// DEBUG
	if (width > 0)
		sc_new->height = (size - sizeof(*sc_new) + sizeof(sc_new->data)) / width;

	sc_new->owner = NULL;              // should be set properly after return

	if (d_roverwrapped)
	{
		if (wrapped_this_time || (sc_rover >= d_initial_rover))
			r_cache_thrash = true;
	}
	else if (wrapped_this_time)
	{       
		d_roverwrapped = true;
	}

	return sc_new;
}


/*
=================
D_SCDump
=================
*/
void D_SCDump (void)
{
	surfcache_t             *test;

	for (test = sc_base ; test ; test = test->next)
	{
		if (test == sc_rover)
			ri.Con_Printf (PRINT_ALL,"ROVER:\n");
		ri.Con_Printf (PRINT_ALL,"%p : %i bytes     %i width\n",test, test->size, test->width);
	}
}

//=============================================================================

// if the num is not a power of 2, assume it will not repeat

int     MaskForNum (int num)
{
	if (num==128)
		return 127;
	if (num==64)
		return 63;
	if (num==32)
		return 31;
	if (num==16)
		return 15;
	return 255;
}

int D_log2 (int num)
{
	int     c;
	
	c = 0;
	
	while (num>>=1)
		c++;
	return c;
}

//=============================================================================

/*
================
D_CacheSurface
================
*/
surfcache_t *D_CacheSurface (msurface_t *surface, int miplevel)
{
	surfcache_t     *cache;
	
//
// if the surface is animating or flashing, flush the cache
//
	r_drawsurf.image = R_TextureAnimation (surface->texinfo);
	r_drawsurf.lightadj[0] = r_newrefdef.lightstyles[surface->styles[0]].rgb[0]*128;
	r_drawsurf.lightadj[1] = r_newrefdef.lightstyles[surface->styles[1]].rgb[1]*128;
	r_drawsurf.lightadj[2] = r_newrefdef.lightstyles[surface->styles[2]].rgb[2]*128;
	r_drawsurf.lightadj[3] = r_newrefdef.lightstyles[surface->styles[3]].white*128;
	
//
// see if the cache holds apropriate data
//
	cache = surface->cachespots[miplevel];

	// if the cache is valid and it's not dynamically lit and dlightframe is not valid
	if (cache && !cache->dlight && surface->dlightframe != r_framecount
			&& cache->image == r_drawsurf.image					// and the image is valid
			&& cache->lightadj[0] == r_drawsurf.lightadj[0]		// and the light is equal
			&& cache->lightadj[1] == r_drawsurf.lightadj[1]
			&& cache->lightadj[2] == r_drawsurf.lightadj[2]
			&& cache->lightadj[3] == r_drawsurf.lightadj[3] )
		return cache;

//
// determine shape of surface
//
	surfscale = 1.0 / (1<<miplevel);
	r_drawsurf.surfmip = miplevel;
	r_drawsurf.surfwidth = surface->extents[0] >> miplevel;
	r_drawsurf.rowbytes = r_drawsurf.surfwidth;
	r_drawsurf.surfheight = surface->extents[1] >> miplevel;
	
//
// allocate memory if needed
//
	if (!cache)     // if a texture just animated, don't reallocate it
	{
		cache = D_SCAlloc (r_drawsurf.surfwidth,
						   r_drawsurf.surfwidth * r_drawsurf.surfheight);
		surface->cachespots[miplevel] = cache;
		cache->owner = &surface->cachespots[miplevel];
		cache->mipscale = surfscale;
	}
	
	if (surface->dlightframe == r_framecount)
		cache->dlight = 1;
	else
		cache->dlight = 0;

	r_drawsurf.surfdat = (pixel_t *)cache->data;
//	r_drawsurf.surfdat = (uint16*)cache->data;
	
	cache->image = r_drawsurf.image;
	cache->lightadj[0] = r_drawsurf.lightadj[0];
	cache->lightadj[1] = r_drawsurf.lightadj[1];
	cache->lightadj[2] = r_drawsurf.lightadj[2];
	cache->lightadj[3] = r_drawsurf.lightadj[3];

//
// draw and light the surface texture
//
	r_drawsurf.surf = surface;

	c_surf++;

	// calculate the lightings
//	R_BuildLightMap ();
	R_BuildColorLightMap ();
	
	// rasterize the surface into the cache
	R_DrawSurface ();

	return cache;
}


