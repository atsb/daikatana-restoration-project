///////////////////////////////////////////////////////////////////////////////
//	gl_protex.c, 6/7/98, by Nelno
//
//	Handles generation, uploading of procedural textures
//
//	TODO:	implement this in ref_soft, or replace procedural stuff with
//			animations
///////////////////////////////////////////////////////////////////////////////

#include	"r_local.h"
#include	"r_protex.h"

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

//	extern
proTex_t	protex_textures [PROTEX_NUM_TYPES];

cvar_t		*pt_type;

///////////////////////////////////////////////////////////////////////////////
//	prototypes
///////////////////////////////////////////////////////////////////////////////

//qboolean GL_Upload32 (unsigned *data, int width, int height,  qboolean mipmap);

///////////////////////////////////////////////////////////////////////////////
//	protex_DoFire
//
//	TODO:	can this be sped up with unsigned * instead of byte *?
///////////////////////////////////////////////////////////////////////////////

byte protextable_red = 64;
byte protextable_grey = 0;
byte protextable_blue = 80;

void	protex_DoFire (proTex_t *texture)
{
	int			x, y, w, h, endx;
	int			center;
	byte		*cur_row, *last_row, *bottom_row;
	float		frac;
	int			adjust;
	static float a = 0;

	last_row = texture->data;

	w = texture->width;
	h = texture->height - 1;

	/////////////////////////////////////////////////////////
	//	put a random row of reddish/yellow along the bottom
	/////////////////////////////////////////////////////////

	cur_row = texture->data + w * (h - 1);
	center = w >> 1;

	if (++a > 360)
		a = 0;
	
	for (x = 0; x < center; x++)
	{
		cur_row[x] = protextable_red + x;
		cur_row[w-x-1] = protextable_red + x;
	}

	// red is not a complete gradient, so fill the 4 middle pixels 
	// with the last color in the gradient
	for (x = center-2; x < center+2; x++)
		cur_row[x] = cur_row[center-3];

	/////////////////////////////////////////////////////////
	//	smooth and move rows up
	/////////////////////////////////////////////////////////

	endx = w - 1;
	center = endx >> 1;
	frac = 0;
	for (y = 1; y < h; y++)
	{
		//	copy each row to the row above it
		cur_row = texture->data + w * y;
	
		//	zero left-most pixel
		last_row[0] = 255;

		adjust = Q_ftol (frac);
		frac+=0.25;

		for (x = 1; x < endx; x++)
			last_row[x] = cur_row[x];

		//	zero-out right-most pixel
		last_row [x] = 255;

		last_row = cur_row;
	}

	bottom_row = texture->data + texture->width * (texture->height - 1);
	for (x = 0; x < texture->width; x++)
		bottom_row [x] = 255;
}
///////////////////////////////////////////////////////////////////////////////
//	protex_DoSwampGas
//
///////////////////////////////////////////////////////////////////////////////

#define	gas_NORMAL		0
#define	gas_WEIGHTED		1	//	center pixel is prominent
#define	gas_WEIGHTED_Y	2	//	center and upper pixels are prominent
#define	gas_WEIGHTED_X	3	//	center and left/right pixels are prominent
	
void	protex_DoSwampGas (proTex_t *texture)
{
}

///////////////////////////////////////////////////////////////////////////////
//	protex_DoSteam
//
///////////////////////////////////////////////////////////////////////////////

#define	STEAM_NORMAL		0
#define	STEAM_WEIGHTED		1	//	center pixel is prominent
#define	STEAM_WEIGHTED_Y	2	//	center and upper pixels are prominent
#define	STEAM_WEIGHTED_X	3	//	center and left/right pixels are prominent
	
void	protex_DoSteam (proTex_t *texture)
{
}

///////////////////////////////////////////////////////////////////////////////
//	protex_DoFireSkin
//
///////////////////////////////////////////////////////////////////////////////

#define	FIRESKIN_NORMAL		0
#define	FIRESKIN_WEIGHTED		1	//	center pixel is prominent
#define	FIRESKIN_WEIGHTED_Y	2	//	center and upper pixels are prominent
#define	FIRESKIN_WEIGHTED_X	3	//	center and left/right pixels are prominent
	
void	protex_DoFireSkin (proTex_t *texture)
{
}


///////////////////////////////////////////////////////////////////////////////
//	protex_DoStatic
//
///////////////////////////////////////////////////////////////////////////////

void	protex_DoStatic (proTex_t *texture)
{
}

///////////////////////////////////////////////////////////////////////////////
//	protex_Alloc
//
///////////////////////////////////////////////////////////////////////////////

void	protex_Alloc (proTex_t *texture, int w, int h, int bit_depth, float frame_time, void (*func)(proTex_t *texture))
{
	texture->width = w;
	texture->height = h;
	texture->bit_depth = 1;
	
	texture->data = (unsigned char*)malloc (w * h * bit_depth);
	memset (texture->data, 0x00, (w * h * bit_depth));

	texture->flags = 0;
	texture->func = func;
	texture->frame_time = frame_time;
	texture->next_time = ref_globalTime + frame_time;
}

///////////////////////////////////////////////////////////////////////////////
//	protex_LoadTexture
//
//	allocates a texture and sets it up as an image_t
///////////////////////////////////////////////////////////////////////////////

void	protex_LoadTexture (char *name, proTex_t *texture, int w, int h, int bit_depth, float frame_time, void (*func)(proTex_t *texture), int type)
{
	protex_Alloc (texture, w, h, bit_depth, frame_time, func);
	texture->image = (image_t*)GL_LoadPic (name, texture->data, w, h, it_procedural);
	texture->image->pt_type = type;
}

///////////////////////////////////////////////////////////////////////////////
//	protex_Free
//
///////////////////////////////////////////////////////////////////////////////

void	protex_Free (proTex_t *texture)
{
	if (!texture->data)
		return;

	free (texture->data);
	texture->data = NULL;
}

///////////////////////////////////////////////////////////////////////////////
//	protex_InitTextures
//
///////////////////////////////////////////////////////////////////////////////

void	protex_InitTextures (void)
{
	memset (protex_textures, 0x00, sizeof (proTex_t) * PROTEX_NUM_TYPES);
	protex_LoadTexture ("fireskin", &protex_textures [PROTEX_FIRESKIN], 64, 64, 1, 0.05, protex_DoFireSkin, PROTEX_FIRESKIN);
}

///////////////////////////////////////////////////////////////////////////////
//	protex_FreeTextures
//
///////////////////////////////////////////////////////////////////////////////

void	protex_FreeTextures (void)
{
	int		i;

	for (i = 0; i < PROTEX_NUM_TYPES; i++)
		protex_Free (&protex_textures[i]);
}

///////////////////////////////////////////////////////////////////////////////
//	protex_InitForFrame
//
//	clear run_this_frame flag for all textures
///////////////////////////////////////////////////////////////////////////////

void	protex_InitForFrame (void)
{
	int			i;

	for (i = 0; i < PROTEX_NUM_TYPES; i++)
	{
		//	clear RUN_THIS_FRAME flag
		protex_textures [i].flags &= ~PTF_RAN_THIS_FRAME;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	protex_InitProceduralFromWAL
//
///////////////////////////////////////////////////////////////////////////////

image_t	*protex_InitProceduralFromWAL (const char *name, byte *pic, int width, int height)
{
	char		fname [_MAX_FNAME];
	proTex_t	*texture;
	protex_type	type;
	void		(*func)(proTex_t *texture);

	_splitpath (name, NULL, NULL, fname, NULL);
	
	//	find the correct texture based on the name
	if (!stricmp (fname, "pt_fire1"))
	{
		func = protex_DoFire;
		type = PROTEX_FIRE;
	}
	else if (!stricmp (fname, "pt_steam1"))
	{
		func = protex_DoSteam;
		type = PROTEX_STEAM;
	}
	else if (!stricmp (fname, "pt_static1"))
	{
		func = protex_DoStatic;
		type = PROTEX_STATIC;
	}
	else if (!stricmp (fname, "pt_swampgas1"))
	{
		func = protex_DoSwampGas;
		type = PROTEX_SWAMPGAS;
	}

	texture = &protex_textures [type];

//	now done in protex_InitTextures
	protex_Alloc (texture, width, height, 1, 0.05, func);

	if (pic)
		texture->image = (image_t*)GL_LoadPic (name, pic, width, height, it_procedural);
	else
		texture->image = (image_t*)GL_LoadPic (name, texture->data, width, height, it_procedural);

	//	set type for later reference in protex_GL_Bind
	texture->image->pt_type = type;

	return	texture->image;
}

///////////////////////////////////////////////////////////////////////////////
//	protex_RunProceduralTexture
//
///////////////////////////////////////////////////////////////////////////////

void	protex_RunProceduralTexture (proTex_t *texture)
{
	//	limit procedural textures to 20 fps
	if (ref_globalTime >= texture->next_time)
	{
		if (!(texture->flags & PTF_RAN_THIS_FRAME))
		{
			texture->func (texture);

			texture->flags |= PTF_RAN_THIS_FRAME;
			texture->next_time = ref_globalTime + texture->frame_time;
		}
	}
}
