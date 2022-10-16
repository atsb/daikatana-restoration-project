///////////////////////////////////////////////////////////////////////////////
//	gl_protex.c, 6/7/98, by Nelno
//
//	Handles generation, uploading of procedural textures
//
//	TODO:	implement this in ref_soft, or replace procedural stuff with
//			animations
///////////////////////////////////////////////////////////////////////////////

#include	"gl_local.h"
#include	"gl_protex.h"

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

//	extern
proTex_t	protex_textures[PROTEX_NUM_TYPES];

cvar_t		*pt_type;

static	int		sinTable [256];		//	plasma
static	byte	redTable [256];		//	plasma
static	byte	greenTable [256];		//	plasma
static	byte	blueTable [256];		//	plasma
static	byte	p1, p2, p3;			//	plasma
static float last_procedural_frame;

#define lerp(t, a, b) ( a.x + t * (b.x - a.x) );( a.y + t * (b.y - a.y) ); ( a.z + t * (b.z - a.z) ); 

///////////////////////////////////////////////////////////////////////////////
//	prototypes
///////////////////////////////////////////////////////////////////////////////

qboolean GL_Upload32 (unsigned *data, int width, int height,  qboolean mipmap);

///////////////////////////////////////////////////////////////////////////////
//	stuff, crazy, crazy stuff
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	protex_DoPlasma
//
///////////////////////////////////////////////////////////////////////////////


void RunProc_Plasma (proTex_t *texture)
{
	byte	*buff = texture->data;
	byte	q1, q2;
	byte	st1, st2;
	byte	t1, t2;
	int		x, y;

	q1 = p1;
	q2 = p2;

	for (y = 0; y < texture->height; y++)
	{
		st1 = sinTable [sinTable [q1]];
		st2 = sinTable [sinTable [q2]];

		for (x = 0; x < texture->width; x++)
		{
			t1 = sinTable [st1];
			t2 = sinTable [st2];

			*buff = (t1 + t2) >> 1;
			*(buff + 1)= (t2 + t2) >> 1;
			*(buff + 2) = t1 + t2;
			*(buff + 3) = t1 + t2;

			buff += 4;

			st1 += 16;
			st2 -= 4;
		}

		q1 -= 8;
		q2 += 4;
	}

	p1 += 2;
	p2 -= 5;	//	y scroll rate

	qglTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, texture->width, texture->height, GL_RGBA, GL_UNSIGNED_BYTE, texture->data);
}

///////////////////////////////////////////////////////////////////////////////
//	protex_DoFire
//
//	TODO:	can this be sped up with unsigned * instead of byte *?
///////////////////////////////////////////////////////////////////////////////

void RunProc_Fire (proTex_t *texture)
{
	int			x, y, w, h, endx, test;
	int			r, g, b, a;
	int			bytes_per_pixel = 4;
	int			center;
	byte		*cur_row, *last_row;
	unsigned	*bottom_row;
	float		dist, scale;

	last_row = texture->data;

	bytes_per_pixel = texture->bytes_per_pixel;	
	w = texture->width * bytes_per_pixel;
	h = texture->height - 1;

	/////////////////////////////////////////////////////////
	//	put a random row of reddish/yellow along the bottom
	/////////////////////////////////////////////////////////

	cur_row = texture->data + w * (h - 1);
	center = w >> 1;
	for (x = 0; x < w; x += bytes_per_pixel)
	{
		//	the closer x is to (w / bytes_per_pixel) / 2
		//	the closer the color should be to yellow
		//	and the closer alpha should be to 1.0
		dist = fabs (center - x);
		
		//	start fading to black
		scale = 1.0 / (dist * 0.15);
		if (scale > 1.0)
			scale = 1.0;

//		r = (rand () % 127) + 128;
		r = (rand () % 96) + 160;

		g = (rand () % r)* scale;

		b = 0;

		a = 255 * scale * 1.3;
		if (a > 255) a = 255;

		cur_row [x] = (byte) (r * scale);
		cur_row [x + 1] = (byte) (g * scale);
		cur_row [x + 2] = (byte) (b * scale);
		cur_row [x + 3] = (byte) (a * scale);
	}

	/////////////////////////////////////////////////////////
	//	smooth and move rows up
	/////////////////////////////////////////////////////////

	endx = w - bytes_per_pixel;

	for (y = 1; y < h; y++)
	{
		//	copy each row to the row above it
		cur_row = texture->data + w * y;
	
		//	zero left-most pixel
		last_row [0] = 0;
		last_row [1] = 0;
		last_row [2] = 0;
		last_row [3] = 0;

		for (x = bytes_per_pixel; x < endx; x += bytes_per_pixel)
		{
			//	copy each pixel on the row to the row above it
			//	averaging it with the pixels around it

			scale = ((float)y / (float)h) * 1.2;
			//	average all the pixels in the middle
			r = ((cur_row [x] * 40 + cur_row [x - 4] + cur_row [x + 4] + cur_row [x + w] * 22) >> 6);
			if (r > 255) r = 255;
			
			g = ((cur_row [x + 1] * 40 + cur_row [x - 3] + cur_row [x + 5] + cur_row [x + w + 1] * 22) >> 6) * scale;
			if (g > 255) g = 255;
			
			b = ((cur_row [x + 2] * 40 + cur_row [x - 2] + cur_row [x + 6] + cur_row [x + w + 2] * 22) >> 6) * scale;
			if (b > 255) b = 255;
			
			a = cur_row [x + 3] * scale * 1.2;
			if (a > 255) a = 255;

			//	don't show really dark colors
			test = (r + g) >> 1;
			if (test < 16)
				a = 0;
			else if (test < 64)
				a = a * 0.8;

			last_row [x] = (byte) r;
			last_row [x + 1] = (byte) g;
			last_row [x + 2] = (byte) b;
			last_row [x + 3] = (byte) a;
		}

		//	zero-out right-most pixel
		last_row [x] = 0;
		last_row [x + 1] = 0;
		last_row [x + 2] = 0;
		last_row [x + 3] = 0;

		last_row = cur_row;
	}

	//	make sure bottom row has 0 alpha, cause it seems to wrap around to
	//	the top of the sprite.  Blending??

	bottom_row = (unsigned *) texture->data + texture->width * (texture->height - 1);
	for (x = 0; x < texture->width; x++)
		bottom_row [x] = 0;

	//	upload texture here, SHOULD BE BOUND ALREADY!!!
	//GL_Upload32 ((unsigned *) texture->data, texture->width, texture->height, false);
	qglTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, texture->width, texture->height, GL_RGBA, GL_UNSIGNED_BYTE, texture->data);
}

///////////////////////////////////////////////////////////////////////////////
//	protex_DoSwampGas
//
///////////////////////////////////////////////////////////////////////////////

#define	gas_NORMAL		0
#define	gas_WEIGHTED		1	//	center pixel is prominent
#define	gas_WEIGHTED_Y	2	//	center and upper pixels are prominent
#define	gas_WEIGHTED_X	3	//	center and left/right pixels are prominent
	
void RunProc_SwampGas (proTex_t *texture)
{
	int			x, y, w, h, endx;
	int			r, g, b, a;
	int			bytes_per_pixel = 4;
	byte		*cur_row, *last_row;
	unsigned	*bottom_row;
	float		fade_rate;

	//	for testing
	texture->sub_type = pt_type->value;

	last_row = texture->data;

	bytes_per_pixel = texture->bytes_per_pixel;	
	w = texture->width * bytes_per_pixel;
	h = texture->height;

	fade_rate = 0.94;

	/////////////////////////////////////////////////////////
	//	put a random row of pixels along the bottom
	/////////////////////////////////////////////////////////

	cur_row = texture->data + w * (h - 1);

	for (x = 0; x < w; x += bytes_per_pixel)
	{
//		r = g = b = rand () % 192 + 64;
//		a = (r + g + b) / 3;

		g = (byte) (rand () % 128 + 127);
		r = (byte) (rand () % g);
		b = 0;
		a = (g + r) >> 1;

		cur_row [x] = r;
		cur_row [x + 1] = g;
		cur_row [x + 2] = b;
		cur_row [x + 3] = a;
	}

	/////////////////////////////////////////////////////////
	//	smooth and move rows up
	/////////////////////////////////////////////////////////

	endx = w - bytes_per_pixel;

	for (y = 1; y < h; y++)
	{
		//	copy each row to the row above it
		cur_row = texture->data + w * y;

		//	FIXME:	doing this without correct weighting may make imperfection in the tiling
		//	average first pixel on row so that it blends with last pixel
		r = (cur_row [0] + cur_row [w - 4] + cur_row [4] + cur_row [w]) >> 2;
		r *= fade_rate;
		g = (cur_row [1] + cur_row [w - 3] + cur_row [5] + cur_row [w + 1]) >> 2;
		g *= fade_rate;
		b = (cur_row [2] + cur_row [w - 2] + cur_row [6] + cur_row [w + 2]) >> 2;
		b *= fade_rate;
		a = (cur_row [3] + cur_row [w - 1] + cur_row [7] + cur_row [w + 3]) >> 2;
		a *= fade_rate;
		
		last_row [0] = (byte) r;
		last_row [1] = (byte) g;
		last_row [2] = (byte) b;
		last_row [3] = (byte) a;

		switch	(texture->sub_type)
		{
			case	gas_NORMAL:
				for (x = bytes_per_pixel; x < endx; x += bytes_per_pixel)
				{
					//	copy each pixel on the row to the row above it
					//	fading the color towards black
					r = (cur_row [x] + cur_row [x - 4] + cur_row [x + 4] + cur_row [x + w]) >> 2;
					r *= fade_rate;
					g = (cur_row [x + 1] + cur_row [x - 3] + cur_row [x + 5] + cur_row [x + w + 1]) >> 2;
					g *= fade_rate;
					b = (cur_row [x + 2] + cur_row [x - 2] + cur_row [x + 6] + cur_row [x + w + 2]) >> 2;
					b *= fade_rate;
					a = (cur_row [x + 3] + cur_row [x - 1] + cur_row [x + 7] + cur_row [x + w + 3]) >> 2;
					a *= fade_rate;

					last_row [x] = (byte) r;
					last_row [x + 1] = (byte) g;
					last_row [x + 2] = (byte) b;
					last_row [x + 3] = (byte) a;
				}
				break;

			case	gas_WEIGHTED:
				for (x = bytes_per_pixel; x < endx; x += bytes_per_pixel)
				{
					r = (cur_row [x] * 13 + cur_row [x - 4] + cur_row [x + 4] + cur_row [x + w]) >> 4;
					r *= fade_rate;
					g = (cur_row [x + 1] * 13 + cur_row [x - 3] + cur_row [x + 5] + cur_row [x + w + 1]) >> 4;
					g *= fade_rate;
					b = (cur_row [x + 2] * 13 + cur_row [x - 2] + cur_row [x + 6] + cur_row [x + w + 2]) >> 4;
					b *= fade_rate;
					a = (cur_row [x + 3] * 13 + cur_row [x - 1] + cur_row [x + 7] + cur_row [x + w + 3]) >> 4;
					a *= fade_rate;

					last_row [x] = (byte) r;
					last_row [x + 1] = (byte) g;
					last_row [x + 2] = (byte) b;
					last_row [x + 3] = (byte) a;
				}
				break;

			case	gas_WEIGHTED_Y:
				for (x = bytes_per_pixel; x < endx; x += bytes_per_pixel)
				{
					r = (cur_row [x] * 10 + cur_row [x - 4] + cur_row [x + 4] + cur_row [x + w] * 4) >> 4;
					r *= fade_rate;
					g = (cur_row [x + 1] * 10 + cur_row [x - 3] + cur_row [x + 5] + cur_row [x + w + 1] * 4) >> 4;
					g *= fade_rate;
					b = (cur_row [x + 2] * 10 + cur_row [x - 2] + cur_row [x + 6] + cur_row [x + w + 2] * 4) >> 4;
					b *= fade_rate;
					a = (cur_row [x + 3] * 10 + cur_row [x - 1] + cur_row [x + 7] + cur_row [x + w + 3] * 4) >> 4;
					a *= fade_rate;

					last_row [x] = (byte) r;
					last_row [x + 1] = (byte) g;
					last_row [x + 2] = (byte) b;
					last_row [x + 3] = (byte) a;
				}
				break;

			case	gas_WEIGHTED_X:
				for (x = bytes_per_pixel; x < endx; x += bytes_per_pixel)
				{
					r = (cur_row [x] * 2 + cur_row [x - 4] + cur_row [x + 4]) >> 2;
					r *= fade_rate;
					g = (cur_row [x + 1] * 2 + cur_row [x - 3] + cur_row [x + 5]) >> 2;
					g *= fade_rate;
					b = (cur_row [x + 2] * 2 + cur_row [x - 2] + cur_row [x + 6]) >> 2;
					b *= fade_rate;
					a = (cur_row [x + 3] * 2 + cur_row [x - 1] + cur_row [x + 7]) >> 2;
					a *= fade_rate;

					last_row [x] = (byte) r;
					last_row [x + 1] = (byte) g;
					last_row [x + 2] = (byte) b;
					last_row [x + 3] = (byte) a;
				}
				break;
		}

		//	FIXME:	doing this without correct weighting may make imperfection in the tiling
		//	average last pixel so that it blends with first pixel on this row
		r = (cur_row [x] + cur_row [x - 4] + cur_row [0] + cur_row [x + w]) >> 2;
		r *= fade_rate;
		g = (cur_row [x + 1] + cur_row [x - 3] + cur_row [1] + cur_row [x + w + 1]) >> 2;
		g *= fade_rate;
		b = (cur_row [x + 2] + cur_row [x - 2] + cur_row [2] + cur_row [x + w + 2]) >> 2;
		b *= fade_rate;
		a = (cur_row [x + 3] + cur_row [x - 1] + cur_row [3] + cur_row [x + w + 3]) >> 2;
		a *= fade_rate;

		last_row [x] = (byte) r;
		last_row [x + 1] = (byte) g;
		last_row [x + 2] = (byte) b;
		last_row [x + 3] = (byte) a;

		last_row = cur_row;
	}

	//	make sure bottom row has 0 alpha, cause it seems to wrap around to
	//	the top of the sprite

	bottom_row = (unsigned *) texture->data + texture->width * (texture->height - 1);
	for (x = 0; x < texture->width; x++)
		bottom_row [x] = 0;

	//	upload texture here, SHOULD BE BOUND ALREADY!!!
	//GL_Upload32 ((unsigned *) texture->data, texture->width, texture->height, false);
	qglTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, texture->width, texture->height, GL_RGBA, GL_UNSIGNED_BYTE, texture->data);
}

///////////////////////////////////////////////////////////////////////////////
//	protex_DoSteam
//
///////////////////////////////////////////////////////////////////////////////

#define	STEAM_NORMAL		0
#define	STEAM_WEIGHTED		1	//	center pixel is prominent
#define	STEAM_WEIGHTED_Y	2	//	center and upper pixels are prominent
#define	STEAM_WEIGHTED_X	3	//	center and left/right pixels are prominent
	
void RunProc_Steam (proTex_t *texture)
{
	int			x, y, w, h, endx;
	int			r, g, b, a;
	int			bytes_per_pixel = 4;
	byte		*cur_row, *last_row;
	unsigned	*bottom_row;
	float		fade_rate;

	//	for testing
	texture->sub_type = pt_type->value;

	last_row = texture->data;

	bytes_per_pixel = texture->bytes_per_pixel;	
	w = texture->width * bytes_per_pixel;
	h = texture->height;

	fade_rate = 0.94;

	/////////////////////////////////////////////////////////
	//	put a random row of reddish/yellow along the bottom
	/////////////////////////////////////////////////////////

	cur_row = texture->data + w * (h - 1);

	for (x = 0; x < w; x += bytes_per_pixel)
	{
		r = g = b = rand () % 192 + 64;
		a = (r + g + b) / 3;

		cur_row [x] = r;
		cur_row [x + 1] = g;
		cur_row [x + 2] = b;
		cur_row [x + 3] = a;
	}

	/////////////////////////////////////////////////////////
	//	smooth and move rows up
	/////////////////////////////////////////////////////////

	endx = w - bytes_per_pixel;

	for (y = 1; y < h; y++)
	{
		//	copy each row to the row above it
		cur_row = texture->data + w * y;

		//	FIXME:	doing this without correct weighting may make imperfections in the tiling
		//	average first pixel on row so that it blends with last pixel
		r = (cur_row [0] + cur_row [w - 4] + cur_row [4] + cur_row [w]) >> 2;
		r *= fade_rate;
		g = (cur_row [1] + cur_row [w - 3] + cur_row [5] + cur_row [w + 1]) >> 2;
		g *= fade_rate;
		b = (cur_row [2] + cur_row [w - 2] + cur_row [6] + cur_row [w + 2]) >> 2;
		b *= fade_rate;
		a = (cur_row [3] + cur_row [w - 1] + cur_row [7] + cur_row [w + 3]) >> 2;
		a *= fade_rate;
		
		last_row [0] = (byte) r;
		last_row [1] = (byte) g;
		last_row [2] = (byte) b;
		last_row [3] = (byte) a;

		switch	(texture->sub_type)
		{
			case	STEAM_NORMAL:
				for (x = bytes_per_pixel; x < endx; x += bytes_per_pixel)
				{
					//	copy each pixel on the row to the row above it
					//	fading the color towards black
					r = (cur_row [x] + cur_row [x - 4] + cur_row [x + 4] + cur_row [x + w]) >> 2;
					r *= fade_rate;
					g = (cur_row [x + 1] + cur_row [x - 3] + cur_row [x + 5] + cur_row [x + w + 1]) >> 2;
					g *= fade_rate;
					b = (cur_row [x + 2] + cur_row [x - 2] + cur_row [x + 6] + cur_row [x + w + 2]) >> 2;
					b *= fade_rate;
					a = (cur_row [x + 3] + cur_row [x - 1] + cur_row [x + 7] + cur_row [x + w + 3]) >> 2;
					a *= fade_rate;

					last_row [x] = (byte) r;
					last_row [x + 1] = (byte) g;
					last_row [x + 2] = (byte) b;
					last_row [x + 3] = (byte) a;
				}
				break;

			case	STEAM_WEIGHTED:
				for (x = bytes_per_pixel; x < endx; x += bytes_per_pixel)
				{
					r = (cur_row [x] * 13 + cur_row [x - 4] + cur_row [x + 4] + cur_row [x + w]) >> 4;
					r *= fade_rate;
					g = (cur_row [x + 1] * 13 + cur_row [x - 3] + cur_row [x + 5] + cur_row [x + w + 1]) >> 4;
					g *= fade_rate;
					b = (cur_row [x + 2] * 13 + cur_row [x - 2] + cur_row [x + 6] + cur_row [x + w + 2]) >> 4;
					b *= fade_rate;
					a = (cur_row [x + 3] * 13 + cur_row [x - 1] + cur_row [x + 7] + cur_row [x + w + 3]) >> 4;
					a *= fade_rate;

					last_row [x] = (byte) r;
					last_row [x + 1] = (byte) g;
					last_row [x + 2] = (byte) b;
					last_row [x + 3] = (byte) a;
				}
				break;

			case	STEAM_WEIGHTED_Y:
				for (x = bytes_per_pixel; x < endx; x += bytes_per_pixel)
				{
					r = (cur_row [x] * 10 + cur_row [x - 4] + cur_row [x + 4] + cur_row [x + w] * 4) >> 4;
					r *= fade_rate;
					g = (cur_row [x + 1] * 10 + cur_row [x - 3] + cur_row [x + 5] + cur_row [x + w + 1] * 4) >> 4;
					g *= fade_rate;
					b = (cur_row [x + 2] * 10 + cur_row [x - 2] + cur_row [x + 6] + cur_row [x + w + 2] * 4) >> 4;
					b *= fade_rate;
					a = (cur_row [x + 3] * 10 + cur_row [x - 1] + cur_row [x + 7] + cur_row [x + w + 3] * 4) >> 4;
					a *= fade_rate;

					last_row [x] = (byte) r;
					last_row [x + 1] = (byte) g;
					last_row [x + 2] = (byte) b;
					last_row [x + 3] = (byte) a;
				}
				break;

			case	STEAM_WEIGHTED_X:
				for (x = bytes_per_pixel; x < endx; x += bytes_per_pixel)
				{
					r = (cur_row [x] * 2 + cur_row [x - 4] + cur_row [x + 4]) >> 2;
					r *= fade_rate;
					g = (cur_row [x + 1] * 2 + cur_row [x - 3] + cur_row [x + 5]) >> 2;
					g *= fade_rate;
					b = (cur_row [x + 2] * 2 + cur_row [x - 2] + cur_row [x + 6]) >> 2;
					b *= fade_rate;
					a = (cur_row [x + 3] * 2 + cur_row [x - 1] + cur_row [x + 7]) >> 2;
					a *= fade_rate;

					last_row [x] = (byte) r;
					last_row [x + 1] = (byte) g;
					last_row [x + 2] = (byte) b;
					last_row [x + 3] = (byte) a;
				}
				break;
		}

		//	FIXME:	doing this without correct weighting may make imperfections in the tiling
		//	average last pixel so that it blends with first pixel on this row
		r = (cur_row [x] + cur_row [x - 4] + cur_row [0] + cur_row [x + w]) >> 2;
		r *= fade_rate;
		g = (cur_row [x + 1] + cur_row [x - 3] + cur_row [1] + cur_row [x + w + 1]) >> 2;
		g *= fade_rate;
		b = (cur_row [x + 2] + cur_row [x - 2] + cur_row [2] + cur_row [x + w + 2]) >> 2;
		b *= fade_rate;
		a = (cur_row [x + 3] + cur_row [x - 1] + cur_row [3] + cur_row [x + w + 3]) >> 2;
		a *= fade_rate;

		last_row [x] = (byte) r;
		last_row [x + 1] = (byte) g;
		last_row [x + 2] = (byte) b;
		last_row [x + 3] = (byte) a;

		last_row = cur_row;
	}

	//	make sure bottom row has 0 alpha, cause it seems to wrap around to
	//	the top of the sprite

	bottom_row = (unsigned *) texture->data + texture->width * (texture->height - 1);
	for (x = 0; x < texture->width; x++)
		bottom_row [x] = 0;

	//	upload texture here, SHOULD BE BOUND ALREADY!!!
//	GL_Upload32 ((unsigned *) texture->data, texture->width, texture->height, false);
	qglTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, texture->width, texture->height, GL_RGBA, GL_UNSIGNED_BYTE, texture->data);
}

///////////////////////////////////////////////////////////////////////////////
//	protex_DoFireSkin
//
///////////////////////////////////////////////////////////////////////////////

#define	FIRESKIN_NORMAL		0
#define	FIRESKIN_WEIGHTED		1	//	center pixel is prominent
#define	FIRESKIN_WEIGHTED_Y	2	//	center and upper pixels are prominent
#define	FIRESKIN_WEIGHTED_X	3	//	center and left/right pixels are prominent
	
void RunProc_FireSkin (proTex_t *texture)
{
	int			x, y, w, h, endx;
	int			r, g, b, a;
	int			bytes_per_pixel = 4;
	byte		*cur_row, *last_row;
	unsigned	*bottom_row;
	float		fade_rate;

	//	for testing
	texture->sub_type = pt_type->value;

	last_row = texture->data;

	bytes_per_pixel = texture->bytes_per_pixel;	
	w = texture->width * bytes_per_pixel;
	h = texture->height;

	fade_rate = 1.0;

	/////////////////////////////////////////////////////////
	//	put a random row of reddish/yellow along the bottom
	/////////////////////////////////////////////////////////

	cur_row = texture->data + w * (h - 1);

	for (x = 0; x < w; x += bytes_per_pixel)
	{
//		r = g = b = rand () % 192 + 64;

		r = (rand () % 255) + 255;
		g = (rand () % r);
		b = 0;

		a = 255;

		cur_row [x] = (byte) r;
		cur_row [x + 1] = (byte) g;
		cur_row [x + 2] = (byte) b;
		cur_row [x + 3] = (byte) a;
	}

	/////////////////////////////////////////////////////////
	//	smooth and move rows up
	/////////////////////////////////////////////////////////

	endx = w - bytes_per_pixel;

	for (y = 1; y < h; y++)
	{
		//	copy each row to the row above it
		cur_row = texture->data + w * y;

		//	FIXME:	doing this without correct weighting may make imperfections in the tiling
		//	average first pixel on row so that it blends with last pixel
		r = (cur_row [0] + cur_row [w - 4] + cur_row [4] + cur_row [w]) >> 2;
		r *= fade_rate;
		g = (cur_row [1] + cur_row [w - 3] + cur_row [5] + cur_row [w + 1]) >> 2;
		g *= fade_rate;
		b = (cur_row [2] + cur_row [w - 2] + cur_row [6] + cur_row [w + 2]) >> 2;
		b *= fade_rate;
		a = (cur_row [3] + cur_row [w - 1] + cur_row [7] + cur_row [w + 3]) >> 2;
		a *= fade_rate;
		
		last_row [0] = (byte) r;
		last_row [1] = (byte) g;
		last_row [2] = (byte) b;
		last_row [3] = (byte) a;

		switch	(texture->sub_type)
		{
			case	FIRESKIN_NORMAL:
				for (x = bytes_per_pixel; x < endx; x += bytes_per_pixel)
				{
					//	copy each pixel on the row to the row above it
					//	fading the color towards black
					r = (cur_row [x] + cur_row [x - 4] + cur_row [x + 4] + cur_row [x + w]) >> 2;
					r *= fade_rate;
					g = (cur_row [x + 1] + cur_row [x - 3] + cur_row [x + 5] + cur_row [x + w + 1]) >> 2;
					g *= fade_rate;
					b = (cur_row [x + 2] + cur_row [x - 2] + cur_row [x + 6] + cur_row [x + w + 2]) >> 2;
					b *= fade_rate;
					a = (cur_row [x + 3] + cur_row [x - 1] + cur_row [x + 7] + cur_row [x + w + 3]) >> 2;
					a *= fade_rate;

					last_row [x] = (byte) r;
					last_row [x + 1] = (byte) g;
					last_row [x + 2] = (byte) b;
					last_row [x + 3] = (byte) a;
				}
				break;

			case	FIRESKIN_WEIGHTED:
				for (x = bytes_per_pixel; x < endx; x += bytes_per_pixel)
				{
					r = (cur_row [x] * 13 + cur_row [x - 4] + cur_row [x + 4] + cur_row [x + w]) >> 4;
					r *= fade_rate;
					g = (cur_row [x + 1] * 13 + cur_row [x - 3] + cur_row [x + 5] + cur_row [x + w + 1]) >> 4;
					g *= fade_rate;
					b = (cur_row [x + 2] * 13 + cur_row [x - 2] + cur_row [x + 6] + cur_row [x + w + 2]) >> 4;
					b *= fade_rate;
					a = (cur_row [x + 3] * 13 + cur_row [x - 1] + cur_row [x + 7] + cur_row [x + w + 3]) >> 4;
					a *= fade_rate;

					last_row [x] = (byte) r;
					last_row [x + 1] = (byte) g;
					last_row [x + 2] = (byte) b;
					last_row [x + 3] = (byte) a;
				}
				break;

			case	FIRESKIN_WEIGHTED_Y:
				for (x = bytes_per_pixel; x < endx; x += bytes_per_pixel)
				{
					r = (cur_row [x] * 10 + cur_row [x - 4] + cur_row [x + 4] + cur_row [x + w] * 4) >> 4;
					r *= fade_rate;
					g = (cur_row [x + 1] * 10 + cur_row [x - 3] + cur_row [x + 5] + cur_row [x + w + 1] * 4) >> 4;
					g *= fade_rate;
					b = (cur_row [x + 2] * 10 + cur_row [x - 2] + cur_row [x + 6] + cur_row [x + w + 2] * 4) >> 4;
					b *= fade_rate;
					a = (cur_row [x + 3] * 10 + cur_row [x - 1] + cur_row [x + 7] + cur_row [x + w + 3] * 4) >> 4;
					a *= fade_rate;

					last_row [x] = (byte) r;
					last_row [x + 1] = (byte) g;
					last_row [x + 2] = (byte) b;
					last_row [x + 3] = (byte) a;
				}
				break;

			case	FIRESKIN_WEIGHTED_X:
				for (x = bytes_per_pixel; x < endx; x += bytes_per_pixel)
				{
					r = (cur_row [x] + cur_row [x - 4] * 7 + cur_row [x + 4] * 8) >> 4;
					r *= fade_rate;
					g = (cur_row [x + 1] + cur_row [x - 3] * 7 + cur_row [x + 5] * 8) >> 4;
					g *= fade_rate;
					b = (cur_row [x + 2] + cur_row [x - 2] * 7 + cur_row [x + 6] * 8) >> 4;
					b *= fade_rate;
					a = (cur_row [x + 3] + cur_row [x - 1] * 7 + cur_row [x + 7] * 8) >> 4;
					a *= fade_rate;

					last_row [x] = (byte) r;
					last_row [x + 1] = (byte) g;
					last_row [x + 2] = (byte) b;
					last_row [x + 3] = (byte) a;
				}
				break;
		}

		//	FIXME:	doing this without correct weighting may make imperfections in the tiling
		//	average last pixel so that it blends with first pixel on this row
		r = (cur_row [x] + cur_row [x - 4] + cur_row [0] + cur_row [x + w]) >> 2;
		r *= fade_rate;
		g = (cur_row [x + 1] + cur_row [x - 3] + cur_row [1] + cur_row [x + w + 1]) >> 2;
		g *= fade_rate;
		b = (cur_row [x + 2] + cur_row [x - 2] + cur_row [2] + cur_row [x + w + 2]) >> 2;
		b *= fade_rate;
		a = (cur_row [x + 3] + cur_row [x - 1] + cur_row [3] + cur_row [x + w + 3]) >> 2;
		a *= fade_rate;

		last_row [x] = (byte) r;
		last_row [x + 1] = (byte) g;
		last_row [x + 2] = (byte) b;
		last_row [x + 3] = (byte) a;

		last_row = cur_row;
	}

	//	make sure bottom row has 0 alpha, cause it seems to wrap around to
	//	the top of the sprite

	bottom_row = (unsigned *) texture->data + texture->width * (texture->height - 1);
	for (x = 0; x < texture->width; x++)
		bottom_row [x] = 0;

	qglTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, texture->width, texture->height, GL_RGBA, GL_UNSIGNED_BYTE, texture->data);
}


///////////////////////////////////////////////////////////////////////////////
//	protex_DoStatic
//
///////////////////////////////////////////////////////////////////////////////

void RunProc_Static( proTex_t *texture )
{
	int		i, size, r, w, x;
	byte	*data;

	w = texture->width * texture->bytes_per_pixel;
	size = texture->height * w;
	data = texture->data;

	for (i = 0; i < size; i++)
		data [i] = rand () % 255;

	//	draw a random number of lines across texture with
	//	random colors

	r = rand () % (texture->height >> 2);
	if (r)
	{
		for (i = 0; i < r; i++)
		{
			data = texture->data + (w * (rand () % texture->height));

			r = rand () % 255;
			for (x = 0; x < w; x += texture->bytes_per_pixel)
			{
				data [x] = data [x + 1] = data [x + 2] = r;
				data [x + 3] = 255;
			}
		}
	}

	//	upload texture here, SHOULD BE BOUND ALREADY!!!
	qglTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, texture->width, texture->height, GL_RGBA, GL_UNSIGNED_BYTE, texture->data);
}

void R_ProcAlloc( proTex_t *texture, int w, int h, int bytes_per_pixel )
{
	texture->width = w;
	texture->height = h;
	texture->bytes_per_pixel = bytes_per_pixel;
	
	texture->data = (byte*)ri.X_Malloc(w * h * bytes_per_pixel,MEM_TAG_MISC);
	memset (texture->data, 0x00, (w * h * bytes_per_pixel));

	texture->flags = 0;
}

void R_ProcFree( proTex_t *texture )
{
	if (!texture->data)
		return;

	ri.X_Free(texture->data);
	texture->data = NULL;
}

void Proc_InitSinTable (void)
{
	int		i;
	float	v, vAdd;
	byte	r, g, b;

	v = 0.0;
	vAdd = (2.0 * M_PI / 256.0);

	for (i = 0; i < 256; i++)
	{
		sinTable [i] = (int)(sin (v) * 63) + 64;
		v += vAdd;
	}

	r = 0;
	g = 0;
	b = 0;

	for (i = 0; i < 64; i++)
	{
		redTable [i] = r;
		greenTable [i] = g;
		blueTable [i] = b;
		r+=4;
	}

	for (i = 64; i < 128; i++)
	{
		r-=4;
		redTable [i] = r;
		greenTable [i] = g;
		blueTable [i] = b;
	}

	for (i = 128; i < 192; i++)
	{
		redTable [i] = r;
		greenTable [i] = g;
		blueTable [i] = b;
		r+=4;
	}

	for (i = 192; i < 256; i++)
	{
		r-=4;
		redTable [i] = r;
		greenTable [i] = g;
		blueTable [i] = b;
	}

	p1 = 87;
	p2 = 230;
	p3 = 63;
}

void R_ProcInit()
{
	Proc_InitSinTable ();
	last_procedural_frame = r_newrefdef.time;
}

void R_ProcFreeTextures (void)
{
	int		i;

	for (i = 0; i < PROTEX_NUM_TYPES; i++)
	{
		R_ProcFree( &protex_textures [i] );
	}
}

image_t	*R_ProcTextureInit( const char *name, int width, int height )
{
	char		fname [_MAX_FNAME];
	proTex_t	*texture;
	protex_type	type;

	_splitpath (name, NULL, NULL, fname, NULL);
	
	//	find the correct texture based on the name
	if (!stricmp (fname, "pt_fire"))
	{
		type = PROTEX_FIRE;
		gl_state.procedural_texture_flags |= PROTEX_FLAG_FIRE;
	}
	else if (!stricmp (fname, "pt_steam"))
	{
		type = PROTEX_STEAM;
		gl_state.procedural_texture_flags |= PROTEX_FLAG_STEAM;
	}
	else if (!stricmp (fname, "pt_static"))
	{
		type = PROTEX_STATIC;
		gl_state.procedural_texture_flags |= PROTEX_FLAG_STATIC;
	}
	else if (!stricmp (fname, "pt_swampgas"))
	{
		type = PROTEX_SWAMPGAS;
		gl_state.procedural_texture_flags |= PROTEX_FLAG_SWAMPGAS;
	}
	else if (!stricmp (fname, "pt_lava"))
	{
		type = PROTEX_LAVA;
		gl_state.procedural_texture_flags |= PROTEX_FLAG_LAVA;
	}
	else if (!stricmp (fname, "pt_mist"))
	{
		type = PROTEX_MIST;
		gl_state.procedural_texture_flags |= PROTEX_FLAG_MIST;
	}
	else if (!stricmp (fname, "pt_water"))
	{
		type = PROTEX_WATER;
		gl_state.procedural_texture_flags |= PROTEX_FLAG_WATER;
	}
	else if (!stricmp (fname, "pt_fireskin"))
	{
		type = PROTEX_FIRESKIN;
		gl_state.procedural_texture_flags |= PROTEX_FLAG_FIRESKIN;
	}
	else
	{
		type = PROTEX_FIRE;
		gl_state.procedural_texture_flags |= PROTEX_FLAG_FIRE;
	}

	texture = &protex_textures[type];

	R_ProcAlloc( texture, width, height, 4 );

	//	set up image_t in global gltextures
	texture->image = GL_LoadPic( name, texture->data, width, height, it_procedural, 32, NULL );
	texture->image->resource = RESOURCE_LEVEL;

	//	set type for later reference in protex_GL_Bind
	texture->image->pt_type = type;

	return	texture->image;
}

///////////////////////////////////////////////////////////////////////////////
//	protex_RunProceduralTexture
//
///////////////////////////////////////////////////////////////////////////////

void R_ProceduralFrame()
{
	if( r_newrefdef.rdflags & RDF_NOWORLDMODEL )
	{
		return;
	}

	if( r_newrefdef.time >= ( last_procedural_frame + ( 1 / gl_procedural_rate->value ) ) )
	{
		if(	gl_state.procedural_texture_flags & PROTEX_FLAG_FIRE )
		{
			GL_Bind( protex_textures[PROTEX_FIRE].image->texnum );
			RunProc_Fire( &protex_textures[PROTEX_FIRE] );
		}
		if(	gl_state.procedural_texture_flags & PROTEX_FLAG_SWAMPGAS )
		{
			GL_Bind( protex_textures[PROTEX_SWAMPGAS].image->texnum );
			RunProc_SwampGas( &protex_textures[PROTEX_SWAMPGAS] );
		}
		if(	gl_state.procedural_texture_flags & PROTEX_FLAG_STEAM )
		{
			GL_Bind( protex_textures[PROTEX_STEAM].image->texnum );
			RunProc_Steam( &protex_textures[PROTEX_STEAM] );
		}
		if(	gl_state.procedural_texture_flags & PROTEX_FLAG_STATIC )
		{
			GL_Bind( protex_textures[PROTEX_STATIC].image->texnum );
			RunProc_Static( &protex_textures[PROTEX_STATIC] );
		}
		if(	gl_state.procedural_texture_flags & PROTEX_FLAG_FIRESKIN )
		{
			GL_Bind( protex_textures[PROTEX_FIRESKIN].image->texnum );
			RunProc_FireSkin( &protex_textures[PROTEX_FIRESKIN] );
		}
		if(	gl_state.procedural_texture_flags & PROTEX_FLAG_LAVA )
		{
			GL_Bind( protex_textures[PROTEX_LAVA].image->texnum );
			RunProc_Plasma( &protex_textures[PROTEX_LAVA] );
		}
		if(	gl_state.procedural_texture_flags & PROTEX_FLAG_WATER )
		{
			GL_Bind( protex_textures[PROTEX_WATER].image->texnum );
			RunProc_Plasma( &protex_textures[PROTEX_WATER] );
		}
		if(	gl_state.procedural_texture_flags & PROTEX_FLAG_MIST )
		{
			GL_Bind( protex_textures[PROTEX_MIST].image->texnum );
			RunProc_Plasma( &protex_textures[PROTEX_MIST] );
		}

		last_procedural_frame = r_newrefdef.time;
	}
}
