
#include	"gl_local.h"
#include	<io.h>
#include	<windows.h>
//#include	"gl_protex.h"

#include "dk_ref_common.h"

image_t		gltextures[MAX_GLTEXTURES];
int			numgltextures;
int			base_textureid;		// gltextures[i] = base_textureid+i

#define		MIPTEX_4	0xff		//	WAL with 4 pre-generated mip levels
#define		MIPTEX_8	0x00		//	WAL with 8 pre-generated mip levels
#define		MIPTEX_NULL	-1			//	no current miptex

miptex_t	*g_current_miptex;
miptexOld_t	*g_current_miptex_old;
int			g_current_miptex_type;	//	either MIPTEX_4 or MIPTEX_8
int			g_loaded_old_wal;

static byte			 intensitytable[256];
static unsigned char gammatable[256];

cvar_t		*intensity;

unsigned	d_8to24table[256];
unsigned char d_gl_8to24table[768]; // Ash - 8 to 24 table in form gl needs
unsigned	currentpalette[256];
int			g_currentTexPalette;	//	index of texture in gltextures for current palette
									//	so the current palette can be found as
									//	gltextures [currentPaletteNum]->palette
									//	-1 means use default d_8to24table
									//	-2 means a call to GL_UploadPalette was
									//	made, so the current palette will only
									//	be set by a subsequent call to GL_UploadPalette
									//	-3 means no palette has been set, so
									//	reset the palette on the first call to
									//	GL_SetTexPalette

//adam: palette is optional, and is used if the texture needs to be expanded to 32 bits to uploaded to the card.
qboolean GL_Upload8(byte *data, int width, int height,  qboolean mipmap, qboolean is_sky, byte *palette);

qboolean GL_Upload32 (unsigned *data, int width, int height,  qboolean mipmap);

unsigned char R_BestColor (byte r, byte g, byte b, unsigned *palette);

byte AveragePixels (byte p0, byte p1, byte p2, byte p3);

int		gl_solid_format = 3;
int		gl_alpha_format = 4;

int		gl_tex_solid_format = 3;
int		gl_tex_alpha_format = 4;

int		gl_filter_min = GL_LINEAR_MIPMAP_NEAREST;
int		gl_filter_max = GL_LINEAR;

void GL_MakePalette24 (byte *palette8, unsigned palette24[256])
{
	int			i, r, g, b;
	unsigned	v;

	if (palette8)
	{
		for (i=0 ; i<256 ; i++)
		{
			r = palette8[i*3+0];
			g = palette8[i*3+1];
			b = palette8[i*3+2];
			
			v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
			palette24[i] = v;
		}

		palette24[255] &= 0xffffff;	// 255 is transparent
	}
	else
	{
		for (i=0 ; i<256 ; i++)
			palette24[i] = d_8to24table[i];
	}
}

void GL_UploadPalette ( unsigned palette[256], int palNum )
{
	int i, j;
	unsigned char temptable[768];
    
    int internalFormat;

    if ((16==gl_state.nColorBits) || !(gl_24bittextures->value))
        internalFormat = GL_RGB;
    else
        internalFormat = GL_RGB8;

	if ( qglColorTableEXT )
	{
		for ( i = j = 0; i < 256; i++, j+=3 )
		{
			temptable[j+0] = ( palette[i] >> 0 ) & 0xff;
			temptable[j+1] = ( palette[i] >> 8 ) & 0xff;
			temptable[j+2] = ( palette[i] >> 16 ) & 0xff;
		}

		qglColorTableEXT( GL_SHARED_TEXTURE_PALETTE_EXT,
						   internalFormat,
						   256,
						   GL_RGB,
						   GL_UNSIGNED_BYTE,
						   temptable );

		g_currentTexPalette = palNum;	//	cinematic palette
	}
}

void GL_SetTexturePalette( image_t *image, qboolean bForcePalette )
{
	int				texNum;
	unsigned char	*gl_palette;
    int internalFormat;

	if( image && image->has_alpha )
	{
		return;
	}

	//	in cinematic palette, only R_SetPalette will change palette
	if( g_currentTexPalette == -2 && !bForcePalette )
		return;

	if ( qglColorTableEXT )
	{
		if (!image)
			texNum = -1;
		else
			texNum = image->texnum;

		
        if (texNum == g_currentTexPalette && !bForcePalette )
			return;

		
        if (texNum == -1)
            gl_palette = d_gl_8to24table;
		else
            gl_palette = image->gl_palette;
        
        
        if ((16==gl_state.nColorBits) || !(gl_24bittextures->value))
            internalFormat = GL_RGB;
        else
            internalFormat = GL_RGB8;

        qglColorTableEXT( GL_SHARED_TEXTURE_PALETTE_EXT,
                          internalFormat,
                          256,
                          GL_RGB,
                          GL_UNSIGNED_BYTE,
                          gl_palette );

		g_currentTexPalette = texNum;
	}
}

FILE	*imagelog_file;

/*
===============
R_OpenImageLog
===============
*/
void R_OpenImageLog (void)
{
	char	filename[1024];
	char			username [256];
	unsigned long	size = 256;

	if (!r_useimagelog->value)
		return;

	if (imagelog_file)
		return;

	GetUserName (username, &size);

// SCG[1/16/00]: 	sprintf (filename, "%s_image.log", username);
	Com_sprintf (filename,sizeof(filename), "%s_image.log", username);
	imagelog_file = fopen (filename, "w+");

	if(!imagelog_file)
		return;
}

/*
===============
R_CloseImageLog
===============
*/
void R_CloseImageLog (void)
{
	if (!r_useimagelog->value)
		return;

	if (!imagelog_file)
		return;

	fclose (imagelog_file);

	imagelog_file = NULL;
}

/*
===============
R_DumpImage
===============
*/
void R_DumpImage (image_t *image)
{
	int	width, height, bytes, bitdepth;

	if (!r_useimagelog->value)
		return;

	if (!imagelog_file)
		return;

	if (strstr (image->name, ".tga"))
		bitdepth = 32;
	else
		bitdepth = 8;

	width = image->width;
	height = image->height;
	bytes = ((width * height) * (bitdepth / 8));

	fprintf (imagelog_file, "image:%s\n\twidth:\t%4d\theight:\t%4d\tbitdepth:\t%4d\tbytes:\t%4d\n", image->name, width, height, bitdepth, bytes);
}

typedef struct
{
	char *name;
	int	minimize, maximize;
} mode_t;

mode_t modes[] = {
	{"GL_NEAREST", GL_NEAREST, GL_NEAREST},
	{"GL_LINEAR", GL_LINEAR, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR}
};

#define NUM_GL_MODES (sizeof(modes) / sizeof (glmode_t))

typedef struct
{
	char *name;
	int mode;
} gltmode_t;

gltmode_t gl_alpha_modes[] = {
	{"default", 4},
	{"GL_RGBA", GL_RGBA},
	{"GL_RGBA8", GL_RGBA8},
	{"GL_RGB5_A1", GL_RGB5_A1},
	{"GL_RGBA4", GL_RGBA4},
	{"GL_RGBA2", GL_RGBA2},
};

#define NUM_GL_ALPHA_MODES (sizeof(gl_alpha_modes) / sizeof (gltmode_t))

gltmode_t gl_solid_modes[] = {
	{"default", 3},
	{"GL_RGB", GL_RGB},
	{"GL_RGB8", GL_RGB8},
	{"GL_RGB5", GL_RGB5},
	{"GL_RGB4", GL_RGB4},
	{"GL_R3_G3_B2", GL_R3_G3_B2},
#ifdef GL_RGB2_EXT
	{"GL_RGB2", GL_RGB2_EXT},
#endif
};

#define NUM_GL_SOLID_MODES (sizeof(gl_solid_modes) / sizeof (gltmode_t))

/*
===============
GL_TextureMode
===============
*/
void GL_TextureMode( const char *string )
{
	int		i;
	image_t	*glt;

	for (i=0 ; i< 6 ; i++)
	{
		if ( !stricmp( modes[i].name, string ) )
			break;
	}

	if (i == 6)
	{
		ri.Con_Printf (PRINT_ALL, "bad filter name\n");
		return;
	}

	gl_filter_min = modes[i].minimize;
	gl_filter_max = modes[i].maximize;

	// change all the existing mipmap texture objects
	for (i=0, glt=gltextures ; i<numgltextures ; i++, glt++)
	{
		if (glt->type != it_pic && glt->type != it_sky )
		{
			GL_Bind (glt->texnum);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
		}
	}
}

/*
===============
GL_TextureAlphaMode
===============
*/
void GL_TextureAlphaMode( char *string )
{
	int		i;

	for (i=0 ; i< NUM_GL_ALPHA_MODES ; i++)
	{
		if ( !Q_stricmp( gl_alpha_modes[i].name, string ) )
			break;
	}

	if (i == NUM_GL_ALPHA_MODES)
	{
		ri.Con_Printf (PRINT_ALL, "bad alpha texture mode name\n");
		return;
	}

	gl_tex_alpha_format = gl_alpha_modes[i].mode;
}

/*
===============
GL_BestFormat
===============
*/
int GL_BestFormat()
{
    if ((16==gl_state.nColorBits) || !(gl_24bittextures->value))
        return gl_tex_solid_format;
    else
        return GL_RGB8;
}

/*
===============
GL_BestAlphaFormat
===============
*/
int GL_BestAlphaFormat(byte *data, int size)
{
	if( gl_config.renderer == GL_RENDERER_VOODOO )
	{
        return gl_tex_alpha_format;
	}

    // We already know we have an alpha texture, but how many alpha bits do we really need?
    if ((16==gl_state.nColorBits) || !(gl_32bitatextures->value))
    {
        // for 16 bit textures the choices are 5551 or 4444
        byte *ptr = data+3;
        int i;
        unsigned char alpha;
    
        for(i=0;i<size;i++, ptr+=4)
        {
            alpha = *ptr;

            if (alpha!=0 && alpha!=255)
                return GL_RGBA4;
        }

        return GL_RGB5_A1;
    }
    else
    {
        // for 32 bit textures, we use 8888 of course
        return GL_RGBA8;
    }
}

/*
===============
GL_TextureSolidMode
===============
*/
void GL_TextureSolidMode( char *string )
{
	int		i;

	for (i=0 ; i< NUM_GL_SOLID_MODES ; i++)
	{
		if ( !Q_stricmp( gl_solid_modes[i].name, string ) )
			break;
	}

	if (i == NUM_GL_SOLID_MODES)
	{
		ri.Con_Printf (PRINT_ALL, "bad solid texture mode name\n");
		return;
	}

	gl_tex_solid_format = gl_solid_modes[i].mode;
}

/*
===============
GL_ImageList_f
===============
*/
void	GL_ImageList_f (void)
{
	int		i;
	image_t	*image;
	int		texels;
	const char *palstrings[2] =
	{
		"RGB",
		"PAL"
	};

	ri.Con_Printf (PRINT_ALL, "------------------\n");
	texels = 0;

	for (i=0, image=gltextures ; i<numgltextures ; i++, image++)
	{
		if (image->texnum <= 0)
			continue;
		texels += image->upload_width*image->upload_height;
		switch (image->type)
		{
		case it_skin:
			ri.Con_Printf (PRINT_ALL, "M");
			break;
		case it_sprite:
			ri.Con_Printf (PRINT_ALL, "S");
			break;
		case it_wall:
			ri.Con_Printf (PRINT_ALL, "W");
			break;
		case it_pic:
			ri.Con_Printf (PRINT_ALL, "P");
			break;
		default:
			ri.Con_Printf (PRINT_ALL, " ");
			break;
		}

		ri.Con_Printf (PRINT_ALL,  " %3i %3i %s: %s\n",
			image->upload_width, image->upload_height, palstrings[image->paletted], image->name);
	}
	ri.Con_Printf (PRINT_ALL, "Total texel count (not counting mipmaps): %i\n", texels);
}


/*
=============================================================================

  scrap allocation

  Allocate all the little status bar obejcts into a single texture
  to crutch up inefficient hardware / drivers

=============================================================================
*/

#define	MAX_SCRAPS		1
#define	SCRAP_BLOCK_WIDTH	256		// SCG[6/7/99]: Changed name so as not to confse with BLOCK_WIDTH in gl_light
#define	SCRAP_BLOCK_HEIGHT	256		// SCG[6/7/99]: Changed name so as not to confse with BLOCK_HEIGHT in gl_light

int			scrap_allocated[MAX_SCRAPS][SCRAP_BLOCK_WIDTH];
byte		scrap_texels[MAX_SCRAPS][SCRAP_BLOCK_WIDTH * SCRAP_BLOCK_HEIGHT];
qboolean	scrap_dirty;

// returns a texture number and the position inside it
int Scrap_AllocBlock (int w, int h, int *x, int *y)
{
	int		i, j;
	int		best, best2;
	int		texnum;

	for (texnum=0 ; texnum<MAX_SCRAPS ; texnum++)
	{
		best = SCRAP_BLOCK_HEIGHT;

		for (i=0 ; i<SCRAP_BLOCK_WIDTH-w ; i++)
		{
			best2 = 0;

			for (j=0 ; j<w ; j++)
			{
				if (scrap_allocated[texnum][i+j] >= best)
					break;
				if (scrap_allocated[texnum][i+j] > best2)
					best2 = scrap_allocated[texnum][i+j];
			}
			if (j == w)
			{	// this is a valid spot
				*x = i;
				*y = best = best2;
			}
		}

		if (best + h > SCRAP_BLOCK_HEIGHT)
			continue;

		for (i=0 ; i<w ; i++)
			scrap_allocated[texnum][*x + i] = best + h;

		return texnum;
	}

	return -1;
//	Sys_Error ("Scrap_AllocBlock: full");
}

int	scrap_uploads;

void Scrap_Upload (void)
{
	int	i;

	// scraps always use the level palette
	for( i = 0; i < 256; i++ )
		currentpalette[i] = d_8to24table[i];

	scrap_uploads++;
	GL_Bind(TEXNUM_SCRAPS);
	GL_Upload8 (scrap_texels[0], SCRAP_BLOCK_WIDTH, SCRAP_BLOCK_HEIGHT, false, false, NULL);
	scrap_dirty = false;
}


/*
=========================================================

TARGA LOADING

=========================================================
*/

typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;


/*
=============
LoadTGA
=============
*/
void LoadTGA (const char *name, byte **pic, int *width, int *height)
{
	int		columns, rows, numPixels;
	byte	*pixbuf;
	int		row, column;
	byte	*buf_p;
	byte	*buffer;
	int		length;
	TargaHeader		targa_header;
	byte			*targa_rgba;

	*pic = NULL;

	//
	// load the file
	//
	length = ri.FS_LoadFile (name, (void **)&buffer);
	if (!buffer)
	{
		ri.Con_Printf (PRINT_DEVELOPER, "Bad tga file %s\n", name);
		return;
	}

	buf_p = buffer;

	targa_header.id_length		= *buf_p++;
	targa_header.colormap_type	= *buf_p++;
	targa_header.image_type		= *buf_p++;
	
	targa_header.colormap_index	 = LittleShort ( *((short *)buf_p) ); buf_p += 2;
	targa_header.colormap_length = LittleShort ( *((short *)buf_p) ); buf_p += 2;
	targa_header.colormap_size	 = *buf_p++;
	targa_header.x_origin		 = LittleShort ( *((short *)buf_p) ); buf_p += 2;
	targa_header.y_origin		 = LittleShort ( *((short *)buf_p) ); buf_p += 2;
	targa_header.width			 = LittleShort ( *((short *)buf_p) ); buf_p += 2;
	targa_header.height			 = LittleShort ( *((short *)buf_p) ); buf_p += 2;
	targa_header.pixel_size		 = *buf_p++;
	targa_header.attributes		 = *buf_p++;

	if (targa_header.image_type!=2 
		&& targa_header.image_type!=10) 
		ri.Sys_Error (ERR_DROP, "LoadTGA: > %s < Only type 2 and 10 targa RGB images supported\n", name);

	if (targa_header.colormap_type !=0 
		|| (targa_header.pixel_size!=32 && targa_header.pixel_size!=24))
		ri.Sys_Error (ERR_DROP, "LoadTGA: > %s < Only 32 or 24 bit images supported (no colormaps)\n", name);

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if (width)
		*width = columns;
	if (height)
		*height = rows;

	targa_rgba = (unsigned char *)ri.X_Malloc(numPixels*4, MEM_TAG_IMAGE );
	*pic = targa_rgba;

	if (targa_header.id_length != 0)
		buf_p += targa_header.id_length;  // skip TARGA image comment
	
	if (targa_header.image_type==2) {  // Uncompressed, RGB images
		for(row=rows-1; row>=0; row--) {
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; column++) {
				unsigned char red,green,blue,alphabyte;
				switch (targa_header.pixel_size) {
					case 24:
							
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = 255;
							break;
					case 32:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							alphabyte = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = alphabyte;
							break;
				}
			}
		}
	}
	else if (targa_header.image_type==10) {   // Runlength encoded RGB images
		unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;
		for(row=rows-1; row>=0; row--) {
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; ) {
				packetHeader= *buf_p++;
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) {        // run-length packet
					switch (targa_header.pixel_size) {
						case 24:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = 255;
								break;
						case 32:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = *buf_p++;
								break;
					}
	
					for(j=0;j<packetSize;j++) {
						*pixbuf++=red;
						*pixbuf++=green;
						*pixbuf++=blue;
						*pixbuf++=alphabyte;
						column++;
						if (column==columns) { // run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*4;
						}
					}
				}
				else {                            // non run-length packet
					for(j=0;j<packetSize;j++) {
						switch (targa_header.pixel_size) {
							case 24:
									blue = *buf_p++;
									green = *buf_p++;
									red = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = 255;
									break;
							case 32:
									blue = *buf_p++;
									green = *buf_p++;
									red = *buf_p++;
									alphabyte = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = alphabyte;
									break;
						}
						column++;
						if (column==columns) { // pixel packet run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*4;
						}						
					}
				}
			}
			breakOut:;
		}
	}

	ri.FS_FreeFile (buffer);
}


/*
====================================================================

IMAGE FLOOD FILLING

====================================================================
*/


/*
=================
Mod_FloodFillSkin

Fill background pixels so mipmapping doesn't have haloes
=================
*/

typedef struct
{
	short		x, y;
} floodfill_t;

// must be a power of 2
#define FLOODFILL_FIFO_SIZE 0x1000
#define FLOODFILL_FIFO_MASK (FLOODFILL_FIFO_SIZE - 1)

#define FLOODFILL_STEP( off, dx, dy ) \
{ \
	if (pos[off] == fillcolor) \
	{ \
		pos[off] = 255; \
		fifo[inpt].x = x + (dx), fifo[inpt].y = y + (dy); \
		inpt = (inpt + 1) & FLOODFILL_FIFO_MASK; \
	} \
	else if (pos[off] != 255) fdc = pos[off]; \
}

void R_FloodFillSkin( byte *skin, int skinwidth, int skinheight )
{
	byte				fillcolor = *skin; // assume this is the pixel to fill
	floodfill_t			fifo[FLOODFILL_FIFO_SIZE];
	int					inpt = 0, outpt = 0;
	int					filledcolor = -1;
	int					i;

	if (filledcolor == -1)
	{
		filledcolor = 0;
		// attempt to find opaque black
		for (i = 0; i < 256; ++i)
			if (currentpalette[i] == (255 << 0)) // alpha 1.0
			{
				filledcolor = i;
				break;
			}
	}

	// can't fill to filled color or to transparent color (used as visited marker)
	if ((fillcolor == filledcolor) || (fillcolor == 255))
	{
		//printf( "not filling skin from %d to %d\n", fillcolor, filledcolor );
		return;
	}

	fifo[inpt].x = 0, fifo[inpt].y = 0;
	inpt = (inpt + 1) & FLOODFILL_FIFO_MASK;

	while (outpt != inpt)
	{
		int			x = fifo[outpt].x, y = fifo[outpt].y;
		int			fdc = filledcolor;
		byte		*pos = &skin[x + skinwidth * y];

		outpt = (outpt + 1) & FLOODFILL_FIFO_MASK;

		if (x > 0)				FLOODFILL_STEP( -1, -1, 0 );
		if (x < skinwidth - 1)	FLOODFILL_STEP( 1, 1, 0 );
		if (y > 0)				FLOODFILL_STEP( -skinwidth, 0, -1 );
		if (y < skinheight - 1)	FLOODFILL_STEP( skinwidth, 0, 1 );
		skin[x + skinwidth * y] = fdc;
	}
}

//=======================================================


/*
================
GL_ResampleTexture32
================
*/
void GL_ResampleTexture32 (unsigned *in, int inwidth, int inheight, unsigned *out,  int outwidth, int outheight)
{
	int		i, j;
	unsigned	*inrow, *inrow2;
	unsigned	frac, fracstep;
	unsigned	p1[1024], p2[1024];
	byte		*pix1, *pix2, *pix3, *pix4;

	fracstep = inwidth*0x10000/outwidth;

	frac = fracstep>>2;
	for (i=0 ; i<outwidth ; i++)
	{
		p1[i] = 4*(frac>>16);
		frac += fracstep;
	}
	frac = 3*(fracstep>>2);
	for (i=0 ; i<outwidth ; i++)
	{
		p2[i] = 4*(frac>>16);
		frac += fracstep;
	}

	for (i=0 ; i<outheight ; i++, out += outwidth)
	{
		inrow = in + inwidth*(int)((i+0.25)*inheight/outheight);
		inrow2 = in + inwidth*(int)((i+0.75)*inheight/outheight);
//		inrow = in + inwidth*Q_ftol(((i+0.25)*inheight/outheight));
//		inrow2 = in + inwidth*Q_ftol(((i+0.75)*inheight/outheight));
		frac = fracstep >> 1;
		for (j=0 ; j<outwidth ; j++)
		{
			pix1 = (byte *)inrow + p1[j];
			pix2 = (byte *)inrow + p2[j];
			pix3 = (byte *)inrow2 + p1[j];
			pix4 = (byte *)inrow2 + p2[j];
			((byte *)(out+j))[0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0])>>2;
			((byte *)(out+j))[1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1])>>2;
			((byte *)(out+j))[2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2])>>2;
			((byte *)(out+j))[3] = (pix1[3] + pix2[3] + pix3[3] + pix4[3])>>2;
		}
	}
}

/*
================
GL_ResampleTexture8
================
*/
void	GL_ResampleTexture8 (byte *in, int inwidth, int inheight, byte *out,  int outwidth, int outheight)
{
	int		i, j;
	byte	*inrow, *inrow2;
	unsigned	frac, fracstep;
	unsigned	p1[1024], p2[1024];
	byte	*pix1, *pix2, *pix3, *pix4;

	fracstep = inwidth*0x10000/outwidth;

	frac = fracstep>>2;
	for (i=0 ; i<outwidth ; i++)
	{
		p1[i] = (frac>>16);
		frac += fracstep;
	}
	frac = 3*(fracstep>>2);
	for (i=0 ; i<outwidth ; i++)
	{
		p2[i] = (frac>>16);
		frac += fracstep;
	}

	for (i=0 ; i<outheight ; i++, out += outwidth)
	{
		inrow = in + inwidth*(int)((i+0.25)*inheight/outheight);
		inrow2 = in + inwidth*(int)((i+0.75)*inheight/outheight);
//		inrow = in + inwidth*Q_ftol(((i+0.25)*inheight/outheight));
//		inrow2 = in + inwidth*Q_ftol(((i+0.75)*inheight/outheight));
		frac = fracstep >> 1;
		for (j=0 ; j<outwidth ; j++)
		{
			pix1 = inrow + p1[j];
			pix2 = inrow + p2[j];
			pix3 = inrow2 + p1[j];
			pix4 = inrow2 + p2[j];
			out[j] = AveragePixels(pix1[0], pix2[0], pix3[0], pix4[0]);
		}
	}
}

/*
================
GL_LightScaleTexture

Scale up the pixel values in a texture to increase the
lighting range
================
*/
void GL_LightScaleTexture (unsigned *in, int inwidth, int inheight, qboolean only_gamma )
{
	if ( only_gamma )
	{
		int		i, c;
		byte	*p;

		p = (byte *)in;

		c = inwidth*inheight;
		for (i=0 ; i<c ; i++, p+=4)
		{
			p[0] = gammatable[p[0]];
			p[1] = gammatable[p[1]];
			p[2] = gammatable[p[2]];
		}
	}
	else
	{
		int		i, c;
		byte	*p;

		p = (byte *)in;

		c = inwidth*inheight;
		for (i=0 ; i<c ; i++, p+=4)
		{
			p[0] = gammatable[intensitytable[p[0]]];
			p[1] = gammatable[intensitytable[p[1]]];
			p[2] = gammatable[intensitytable[p[2]]];
		}
	}
}

/*
================
GL_MipMap32

Operates in place, quartering the size of the texture
================
*/
void GL_MipMap32 (byte *in, int width, int height)
{
	int		i, j;
	byte	*out;

	width <<=2;
	height >>= 1;
	out = in;
	for (i=0 ; i<height ; i++, in+=width)
	{
		for (j=0 ; j<width ; j+=8, out+=4, in+=8)
		{
			out[0] = (in[0] + in[4] + in[width+0] + in[width+4])>>2;
			out[1] = (in[1] + in[5] + in[width+1] + in[width+5])>>2;
			out[2] = (in[2] + in[6] + in[width+2] + in[width+6])>>2;
			out[3] = (in[3] + in[7] + in[width+3] + in[width+7])>>2;
		}
	}
}


/*
=============
AveragePixels
=============
*/
byte AveragePixels (byte p0, byte p1, byte p2, byte p3)
{
	int		i;
	int		bestcolor;
	int		color[4];
	
	// add pixel 1 
	for (i = 0; i < 3; i++)
		color[i] = ((byte *)&currentpalette[p0])[i];
	// add pixel 2 
	for (i = 0; i < 3; i++)
		color[i] += ((byte *)&currentpalette[p1])[i];
	// add pixel 3
	for (i = 0; i < 3; i++)
		color[i] += ((byte *)&currentpalette[p2])[i];
	// add pixel 4
	for (i = 0; i < 3; i++)
		color[i] += ((byte *)&currentpalette[p3])[i];

	color[0] /= 4;
	color[1] /= 4;
	color[2] /= 4;

//
// find the best color
//
	bestcolor = R_BestColor ((byte) color[0], (byte) color[1], (byte) color[2], currentpalette);

	return bestcolor;

}

/*
================
GL_MipMap8

Operates in place, quartering the size of the texture
================
*/
void GL_MipMap8 (byte *in, int width, int height)
{
    int x, y, destx, desty, outwidth;
//	int	count;
	byte	pix0, pix1, pix2, pix3;

    outwidth = width >> 1;

    desty = 0;
//	count = 0;

    //go down each line.
    for (y = 0; y < height; y += 2) {
        //go across the scanline.
        destx = 0;
        for (x = 0; x < width; x += 2) {

			// need to average pix + 1, pix + width, pix + width + 1
			pix0 = in[y * width + x];
			pix1 = in[y * width + x + 1];
			pix2 = in[y * width + x + width];
			pix3 = in[y * width + x + width + 1];

            //copy over the pixel.
			in[desty * outwidth + destx] = AveragePixels (pix0, pix1, pix2, pix3);

            destx++;    
        }

        desty++;
    }
}

int		upload_width, upload_height;
qboolean uploaded_paletted;

/*
===============
GL_Upload32

Returns has_alpha
===============
*/
qboolean GL_Upload32 (unsigned *data, int width, int height,  qboolean mipmap)
{
	int			samples, comp;
	unsigned	scaled[256*256];
//	unsigned char paletted_texture[256*256];
	int			scaled_width, scaled_height;
	int			i, c;
	byte		*scan;

	uploaded_paletted = false;

	for (scaled_width = 1 ; scaled_width < width ; scaled_width<<=1)
		;
	if (gl_round_down->value && scaled_width > width && mipmap)
		scaled_width >>= 1;
	for (scaled_height = 1 ; scaled_height < height ; scaled_height<<=1)
		;
	if (gl_round_down->value && scaled_height > height && mipmap)
		scaled_height >>= 1;

	// let people sample down the world textures for speed
	if (mipmap)
	{
		scaled_width >>= (int)gl_picmip->value;
		scaled_height >>= (int)gl_picmip->value;
	}

	// don't ever bother with >256 textures
	if (scaled_width > 256)
		scaled_width = 256;
	if (scaled_height > 256)
		scaled_height = 256;

	if (scaled_width < 1)
		scaled_width = 1;
	if (scaled_height < 1)
		scaled_height = 1;

	upload_width = scaled_width;
	upload_height = scaled_height;

	if (scaled_width * scaled_height > sizeof(scaled)/4)
		ri.Sys_Error (ERR_DROP, "GL_Upload32: too big");

	// scan the texture for any non-255 alpha
	c = width*height;
	scan = ((byte *)data) + 3;
	samples = gl_solid_format;
	for (i=0 ; i<c ; i++, scan += 4)
	{
		if ( *scan != 255 )
		{
			samples = gl_alpha_format;
			break;
		}
	}

	// SCG[5/20/99]: Quake2 V3.19 addition
	if (samples == gl_solid_format)
	    comp = GL_BestFormat();
	else if (samples == gl_alpha_format)
	    comp = GL_BestAlphaFormat((byte *)data, width*height);
	else {
	    ri.Con_Printf (PRINT_ALL,
			   "Unknown number of texture components %i\n",
			   samples);
	    comp = samples;
	}

	if (scaled_width == width && scaled_height == height)
	{
		if (!mipmap)
		{
			qglTexImage2D (GL_TEXTURE_2D, 0, comp, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			goto done;
		}
		memcpy (scaled, data, width*height*4);
	}
	else
		GL_ResampleTexture32 (data, width, height, scaled, scaled_width, scaled_height);

	GL_LightScaleTexture( scaled, scaled_width, scaled_height, !mipmap );

	qglTexImage2D( GL_TEXTURE_2D, 0, comp, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled );

	if (mipmap)
	{
		int		miplevel;

		miplevel = 0;
		while (scaled_width > 1 || scaled_height > 1)
		{
			// SCG[3/28/00]: make sure we are not adding extra 1x1 mips.
			if( ( scaled_width == 1 ) && ( scaled_height == 1 ) )
			{
				break;
			}

			GL_MipMap32 ((byte *)scaled, scaled_width, scaled_height);
			scaled_width >>= 1;
			scaled_height >>= 1;
			if (scaled_width < 1)
				scaled_width = 1;
			if (scaled_height < 1)
				scaled_height = 1;
			miplevel++;

			qglTexImage2D (GL_TEXTURE_2D, miplevel, comp, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
		}
	}
done: ;

	if (mipmap)
	{
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
	}
	else
	{
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
	}

	return (samples == gl_alpha_format);
}

/*
===============
GL_Upload8_2

Returns has_alpha
===============
*/
qboolean GL_Upload8_2 (byte *data, int width, int height,  qboolean mipmap)
{
	byte	scaled[256*256];
	int		scaled_width, scaled_height;
	int		ofs;
	int		resampled;

	resampled = 0;

	for (scaled_width = 1 ; scaled_width < width ; scaled_width<<=1)
		;
	if (gl_round_down->value && scaled_width > width && mipmap)
		scaled_width >>= 1;
	for (scaled_height = 1 ; scaled_height < height ; scaled_height<<=1)
		;
	if (gl_round_down->value && scaled_height > height && mipmap)
		scaled_height >>= 1;

	// let people sample down the world textures for speed
	if (mipmap)
	{
		scaled_width >>= (int)gl_picmip->value;
		scaled_height >>= (int)gl_picmip->value;
	}

	// don't ever bother with >256 textures
	if (scaled_width > 256)
		scaled_width = 256;
	if (scaled_height > 256)
		scaled_height = 256;

	if (scaled_width < 1)
		scaled_width = 1;
	if (scaled_height < 1)
		scaled_height = 1;

	upload_width = scaled_width;
	upload_height = scaled_height;

	if (scaled_width * scaled_height > sizeof(scaled))
		ri.Sys_Error (ERR_DROP, "GL_Upload8_2: too big");

	if (scaled_width == width && scaled_height == height)
	{
		if (!mipmap)
		{
			qglTexImage2D( GL_TEXTURE_2D,
						  0,
						  GL_COLOR_INDEX8_EXT,
						  width,
						  height,
						  0,
						  GL_COLOR_INDEX,
						  GL_UNSIGNED_BYTE,
						  data );

			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

			return 0;
		}
		memcpy (scaled, data, width*height);
	}
	else
	{
		GL_ResampleTexture8 (data, width, height, scaled, scaled_width, scaled_height);
		resampled = true;
	}
	qglTexImage2D( GL_TEXTURE_2D,
				  0,
				  GL_COLOR_INDEX8_EXT,
				  scaled_width,
				  scaled_height,
				  0,
				  GL_COLOR_INDEX,
				  GL_UNSIGNED_BYTE,
				  scaled );


	if (mipmap)
	{
		int		miplevel;

		//	generate all mip-levels on the fly
		if ((!g_current_miptex && g_current_miptex_type == MIPTEX_8) ||
			(!g_current_miptex_old && g_current_miptex_type == MIPTEX_4) || resampled || g_current_miptex_type == MIPTEX_NULL)
		{
			miplevel = 0;
			while (scaled_width > 1 || scaled_height > 1)
			{
				// SCG[3/28/00]: make sure we are not adding extra 1x1 mips.
				if( ( scaled_width == 1 ) && ( scaled_height == 1 ) )
				{
					break;
				}

				GL_MipMap8 (scaled, scaled_width, scaled_height);
				scaled_width >>= 1;
				scaled_height >>= 1;
				if (scaled_width < 1)
					scaled_width = 1;
				if (scaled_height < 1)
					scaled_height = 1;
				miplevel++;
		
				qglTexImage2D( GL_TEXTURE_2D,
							  miplevel,
							  GL_COLOR_INDEX8_EXT,
							  scaled_width,
							  scaled_height,
							  0,
							  GL_COLOR_INDEX,
							  GL_UNSIGNED_BYTE,
							  scaled);
			}
		}
		else if (g_current_miptex_type == MIPTEX_4)
		{
			//	Upload the pre-generated mipmaps if this is a 4 mip-map WAL
			for (miplevel = 1; miplevel < 4; miplevel++)
			{
				// SCG[3/28/00]: make sure we are not adding extra 1x1 mips.
				if( ( scaled_width == 1 ) && ( scaled_height == 1 ) )
				{
					break;
				}

				scaled_width >>= 1;
				scaled_height >>= 1;
				if (scaled_width < 1)
					scaled_width = 1;
				if (scaled_height < 1)
					scaled_height = 1;
				ofs = LittleLong (g_current_miptex_old->offsets[miplevel]);
				qglTexImage2D( GL_TEXTURE_2D,
							  miplevel,
							  GL_COLOR_INDEX8_EXT,
							  scaled_width,
							  scaled_height,
							  0,
							  GL_COLOR_INDEX,
							  GL_UNSIGNED_BYTE,
							  (byte *)g_current_miptex_old + ofs);
			}

			memcpy (scaled, (byte *)g_current_miptex_old + ofs, scaled_width*scaled_height);

			//	generate the rest of the mip-maps for a 4 mip-map WAL
			while (scaled_width > 1 || scaled_height > 1)
			{
				// SCG[3/28/00]: make sure we are not adding extra 1x1 mips.
				if( ( scaled_width == 1 ) && ( scaled_height == 1 ) )
				{
					break;
				}

				GL_MipMap8 (scaled, scaled_width, scaled_height);
				scaled_width >>= 1;
				scaled_height >>= 1;
				if (scaled_width < 1)
					scaled_width = 1;
				if (scaled_height < 1)
					scaled_height = 1;
		
				qglTexImage2D( GL_TEXTURE_2D,
							  miplevel,
							  GL_COLOR_INDEX8_EXT,
							  scaled_width,
							  scaled_height,
							  0,
							  GL_COLOR_INDEX,
							  GL_UNSIGNED_BYTE,
							  scaled);
				miplevel++;
			}
		}
		else if (g_current_miptex_type == MIPTEX_8)
		{
			//	Upload the pre-generated mipmaps if this is a 4 mip-map WAL
			for (miplevel = 1; miplevel < MIPLEVELS; miplevel++)
			{
				// SCG[3/28/00]: make sure we are not adding extra 1x1 mips.
				if( ( scaled_width == 1 ) && ( scaled_height == 1 ) )
				{
					break;
				}

				scaled_width >>= 1;
				scaled_height >>= 1;
				if (scaled_width < 1)
					scaled_width = 1;
				if (scaled_height < 1)
					scaled_height = 1;
				ofs = LittleLong (g_current_miptex->offsets[miplevel]);
				qglTexImage2D( GL_TEXTURE_2D,
							  miplevel,
							  GL_COLOR_INDEX8_EXT,
							  scaled_width,
							  scaled_height,
							  0,
							  GL_COLOR_INDEX,
							  GL_UNSIGNED_BYTE,
							  (byte *)g_current_miptex + ofs);
			}
		}
		else
			ri.Sys_Error (ERR_DROP, "%s or %s: Unknown miptex type!", g_current_miptex->name, g_current_miptex_old->name);
	}
	if (mipmap)
	{
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
	}
	else
	{
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
	}

	return 0;
}

/*
===============
GL_Upload8

Returns has_alpha
===============
*/
qboolean GL_Upload8 (byte *data, int width, int height,  qboolean mipmap, qboolean is_sky, byte *palette)
{
	unsigned	trans[512*256];
	int			i, s;
	int			p;
	int			hasalpha;

	hasalpha = 0;

	s = width*height;

	if (s > sizeof(trans)/4)
		ri.Sys_Error (ERR_DROP, "GL_Upload8: too large");

//	if the image is not a sky, the image is converted to 24 bit data then uploaded as 8 bit data in upload32
	if ( qglColorTableEXT && is_sky )
	{
		upload_width = width;
		upload_height = height;
		qglTexImage2D( GL_TEXTURE_2D,
					  0,
					  GL_COLOR_INDEX8_EXT,
					  width,
					  height,
					  0,
					  GL_COLOR_INDEX,
					  GL_UNSIGNED_BYTE,
					  data );

		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

		return	0;
	}
	else 
	{
		// s is the byte count for the image
		// data is the image
		for (i=0 ; i<s ; i++) 
		{
            p = data[i];
            //check if we had a palette passed in.
            if (palette != NULL) 
			{
                int offset;
                offset = p * 3;
                trans[i] = (0xff << 24) | (palette[offset]) | (palette[offset + 1] << 8) | (palette[offset + 2] << 16);
            }
            else 
			{
				// trans is a 24 bit version of the image
				trans[i] = currentpalette[p];
				// if the color is the transparent color...
				if (p == 255) 
				{	
					hasalpha = 1;
                    // transparent, so scan around for another color
				    // to avoid alpha fringes
				    // FIXME: do a full flood fill so mips work...
				    if (i > width && data[i-width] != 255)
					    p = data[i-width];
				    else if (i < s-width && data[i+width] != 255)
					    p = data[i+width];
				    else if (i > 0 && data[i-1] != 255)
					    p = data[i-1];
				    else if (i < s-1 && data[i+1] != 255)
					    p = data[i+1];
				    else
					    p = 0;
				    // copy rgb components
				    ((byte *)&trans[i])[0] = ((byte *)&currentpalette[p])[0];	//r
				    ((byte *)&trans[i])[1] = ((byte *)&currentpalette[p])[1];	//g
				    ((byte *)&trans[i])[2] = ((byte *)&currentpalette[p])[2];	//b
			    }
            }
		}

		if (hasalpha || qglColorTableEXT == NULL)
			GL_Upload32 (trans, width, height, mipmap);
		else
			GL_Upload8_2 (data, width, height, mipmap);

		return hasalpha;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	GL_CheckPowerOf2
//
///////////////////////////////////////////////////////////////////////////////

int	GL_CheckPowerOf2 (int check)
{
	int	power = 0, value = 2;
	int	i;

	while (value < 512)
	{
		if (check == value)
			return	true;

		value = 2;
		power++;
		for (i = 0; i < power; i++)
		{
			value = 2 * value;
		}
	}

	return	false;
}

/*
================
GL_LoadPic

This is also used as an entry point for the generated r_notexture
================
*/
image_t *GL_LoadPic (const char *name, byte *pic, int width, int height, imagetype_t type, int bits, byte *palette) 
{
	image_t		*image;
	int			i, j;


	if (width == 0 || height == 0)
		ri.Sys_Error (ERR_FATAL, "GL_LoadPic: texture %s has a 0 width or height.\n", name);
	if (((float)height / (float)width < 0.125) || ((float)width / (float)height < 0.125))
		ri.Sys_Error (ERR_FATAL, "GL_LoadPic: texture %s has > 8:1 size ratio.\n", name);

	// find a free image_t
	for (i=0, image=gltextures ; i<numgltextures ; i++,image++)
	{
		if (!image->texnum)
			break;
	}
	if (i == numgltextures)
	{
		if (numgltextures == MAX_GLTEXTURES)
			ri.Sys_Error (ERR_FATAL, "MAX_GLTEXTURES");
		numgltextures++;
	}
	image = &gltextures[i];

	if (!GL_CheckPowerOf2 (width) || !GL_CheckPowerOf2 (height))
	{
		memcpy (image, r_notexture, sizeof (image_t));
		strcpy (image->name, name);
			ri.Con_Printf (PRINT_ALL, "GL_LoadPic: %s texture dimensions are not a power of 2!\n", name);
		return	image;
			ri.Sys_Error (ERR_DROP, "GL_LoadPic: %s texture dimensions are not a power of 2!\n", name);
	}

	if (strlen(name) >= sizeof(image->name))
		ri.Sys_Error (ERR_FATAL, "GL_LoadPic: \"%s\" is too long", name);
	strcpy (image->name, name);
	image->registration_sequence = registration_sequence;

	image->width = width;
	image->height = height;
	image->type = type;

    // Ash - now build the gl_palette
	for ( i = 0, j = 0; i < 256; i++, j+=3 )
	{
		image->gl_palette[j+0] = ( currentpalette [i] >> 0 ) & 0xff;
		image->gl_palette[j+1] = ( currentpalette [i] >> 8 ) & 0xff;
		image->gl_palette[j+2] = ( currentpalette [i] >> 16 ) & 0xff;
	}

	if (type == it_skin && bits == 8)
		R_FloodFillSkin(pic, width, height);

	// load little pics into the scrap
	if (image->type == it_pic && bits == 8
		&& image->width < 64 && image->height < 64 && palette == NULL)
	{
		int		x, y;
		int		i, j, k;
		int		texnum;

		texnum = Scrap_AllocBlock (image->width, image->height, &x, &y);
		if (texnum == -1)
			goto nonscrap;
		scrap_dirty = true;

		// copy the texels into the scrap block
		k = 0;
		for (i=0 ; i<image->height ; i++)
			for (j=0 ; j<image->width ; j++, k++)
				scrap_texels[texnum][(y+i)*SCRAP_BLOCK_WIDTH + x + j] = pic[k];

		image->texnum = TEXNUM_SCRAPS + texnum;
		image->scrap = true;
		image->has_alpha = true;
		image->sl = (x+0.01)/(float)SCRAP_BLOCK_WIDTH;
		image->sh = (x+image->width-0.01)/(float)SCRAP_BLOCK_WIDTH;
		image->tl = (y+0.01)/(float)SCRAP_BLOCK_WIDTH;
		image->th = (y+image->height-0.01)/(float)SCRAP_BLOCK_WIDTH;
	}
	else
	{
nonscrap:
		image->scrap = false;
		image->texnum = TEXNUM_IMAGES + (image - gltextures);
		GL_Bind(image->texnum);

		qboolean mipmap = (image->type != it_pic && image->type != it_sky && image->type != it_procedural);

		// SCG[11/30/99]: Hack.  We don't want interface skins to mipmap
		if( strstr( image->name, "ib_" ) )
		{
			mipmap = 0;
		}

		if (bits == 8)
			image->has_alpha = GL_Upload8 (pic, width, height, mipmap, image->type == it_sky, palette);
		else
			image->has_alpha = GL_Upload32 ((unsigned *)pic, width, height, mipmap );

		image->upload_width = upload_width;		// after power of 2 and scales
		image->upload_height = upload_height;
		image->paletted = uploaded_paletted;
		image->sl = 0;
		image->sh = 1;
		image->tl = 0;
		image->th = 1;
	}

	if( bits == 8 && image->has_alpha )
	{
/*
		ri.Con_Printf ( PRINT_ALL, "GL_FindImage: image contained color 255 and was made 24-bit: %s\n", name );
		if( imagelog_file != NULL )
		{
			fprintf(imagelog_file, "GL_FindImage: image contained color 255 and was made 24-bit: %s\n", name );
		}
*/
	}


	return image;
}

/*
================
GL_LoadWal
================
*/
image_t *GL_LoadWal (const char *name, imagetype_t type)
{
	miptex_t	*mt;
	int			width, height, ofs;
	image_t		*image;
//	char		fname [_MAX_FNAME];

	ri.FS_LoadFile (name, (void **)&mt);
	if (!mt)
	{
		ri.Con_Printf (PRINT_ALL, "GL_FindImage: can't load %s\n", name);
		return r_notexture;
	}

	//	Nelno:	determine if this is an old WAL type (4 mip levels)
	//	or a newer WAL type that contains a version number

	if (mt->version > 0x20)
	{
//		return GL_LoadWalOld (name, mt, type);
		ri.Con_Printf (PRINT_ALL, "GL_FindImage: old wal file not supported %s\n", name);
		return r_notexture;
	}

	if (mt->version != MIPTEX_VERSION)
		ri.Sys_Error (ERR_DROP, "%s has wrong version number (%i should be %i)", name, mt->version, MIPTEX_VERSION);

	width = LittleLong (mt->width);
	height = LittleLong (mt->height);

	ofs = LittleLong (mt->offsets[0]);

	g_current_miptex = mt;
	g_current_miptex_type = MIPTEX_8;	//	all levels pre-generated

	if (type == it_skin ){
		GL_MakePalette24 (mt->palette, currentpalette);
	}
	else{
		GL_MakePalette24 (NULL, currentpalette);
	}
	
	image = GL_LoadPic (name, (byte *)mt + ofs, width, height, type, 8, NULL);

	ri.FS_FreeFile ((void *)mt);

	return image;
}

/*
===============
R_FindImage

Finds or loads the given image
===============
*/
image_t	*R_FindImage (const char *name, imagetype_t type, resource_t resource)
{
	image_t	*image;
	int		i, len;
	byte	*pic, *palette;
	int		width, height;
	int		bitdepth;

	if (!name)
	{
		return NULL;
	}

	len = strlen(name);
	if (len<5)
	{
		return NULL;
	}

	// look for it
	for( i = 0, image = gltextures; i < numgltextures; i++, image++ )
	{
		//	Nelno:	reject if first char is not the same
		if( name[0] != image->name[0] )
		{
			continue;
		}

		//	Nelno:	if we find the image is already loaded, but it's flags tell us
		//	to reload it, then get rid of it
		if( !strcmp( name, image->name) )
		{
			image->registration_sequence = registration_sequence;
			return image;
		}
	}

	//
	// load the pic from disk
	//
	pic = palette = NULL;
	image = NULL;

	if( !strcmp( name + len - 4, ".pcx" ) )
	{
		LoadPCX( name, &pic, &palette, &width, &height );
		bitdepth = 8;
	}
	else if (!strcmp(name+len-4, ".bmp"))
	{
		LoadBMP (name, &pic, &palette, &width, &height);
		bitdepth = 8;
	}
	else if (!strcmp(name+len-4, ".wal"))
	{
		image = GL_LoadWal (name, type);
	}
	else if( !strcmp( name + len - 4, ".tga" ) )
	{
		LoadTGA( name, &pic, &width, &height );
		bitdepth = 32;
	}
	else
	{
		return NULL;
	}

	// SCG[7/2/99]: If this isn't a .wal file...
	if( image == NULL )
	{
		if( pic == NULL )
		{
			return NULL;
		}

		if( bitdepth == 8 )
		{
			GL_MakePalette24 (palette, currentpalette);
		}

		image = GL_LoadPic (name, pic, width, height, type, bitdepth, NULL);
	}

	if( pic )
	{
//		free( pic );
		ri.X_Free( pic );
	}

	if( palette )
	{
//		free( palette );
		ri.X_Free( palette );
	}

	//	store resource type
	if( image != r_notexture )
	{
		image->resource = resource;
	}

	int mem = ( ( image->width * image->height ) * bitdepth / 8 );

	switch (image->type)
	{
	case it_skin:
		c_skin_memory += mem;
		break;
	case it_sprite:
		c_sprite_memory += mem;
		break;
	case it_wall:
		c_wal_memory += mem;
		break;
	case it_pic:
		c_pic_memory += mem;
		break;
	case it_sky:
		c_sky_memory += mem;
		break;
	}

	R_DumpImage (image);

	return image;
}



/*
===============
RegisterSkin
===============
*/
void *RegisterSkin (const char *name, resource_t flags)
{
	char	base [128], newname[128];
	char	ext [_MAX_EXT];
	image_t	*image = NULL;

	if( name == NULL )
	{
		return r_notexture;
	}

	_splitpath (name, NULL, NULL, NULL, ext);
	if (!stricmp (ext, ".tga"))
	{
		//	load tga's without using .wals
		image = R_FindImage (name, it_skin, (resource_t)flags);
	}
	else
	{
		COM_StripExtension ((char*)name, &base[0]);

		if (!image || image == r_notexture)
		{
// SCG[1/16/00]: 			sprintf (newname, "%s.wal", base);
			Com_sprintf (newname,sizeof(newname), "%s.wal", base);
			image = R_FindImage (newname, it_skin, (resource_t)flags);
		}
	}

	if (!image || image == r_notexture)
	{
// SCG[1/16/00]: 		sprintf (newname, "%s.bmp", base);
		Com_sprintf (newname,sizeof(newname), "%s.bmp", base);
		image  = R_FindImage (name, it_skin, (resource_t)flags);
	}

	return image;
}

///////////////////////////////////////////////////////////////////////////////
//	GL_FreeImage
//
//	frees a gl image
///////////////////////////////////////////////////////////////////////////////

void	GL_FreeImage (image_t *image)
{
	int	bitdepth;
	int mem;

	// free it
	qglDeleteTextures (1, (unsigned int *)&image->texnum);

	//	Nelno:	free memory if procedural
//	if (image->type == it_procedural)
//		R_ProcFree( &protex_textures[image->pt_type] );

	// update texture memory usage
	if (image->has_alpha)
		bitdepth = 32;
	else
		bitdepth = 8;

	mem = ((image->width * image->height) * bitdepth / 8);

	switch (image->type)
	{
	case it_skin:
		c_skin_memory -= mem;
		break;
	case it_sprite:
		c_sprite_memory -= mem;
		break;
	case it_wall:
		c_wal_memory -= mem;
		break;
	case it_pic:
		c_pic_memory -= mem;
		break;
	case it_sky:
		c_sky_memory -= mem;
		break;
//	case it_procedural:
//		c_procedural_memory += mem;
//		break;
	}

//	c_texture_memory -= ((image->width * image->height) * bitdepth / 8);

	memset (image, 0, sizeof(image_t));
}

/*
================

GL_FreeUnusedImages

Any image that was not touched on this registration sequence
will be freed.
================
*/
void GL_FreeUnusedImages (void)
{
	int		i;
	image_t	*image;

	// never free r_notexture or particle texture
	r_notexture->registration_sequence = registration_sequence;
	r_particletexture->registration_sequence = registration_sequence;

	for (i=0, image=gltextures ; i<numgltextures ; i++, image++)
	{
		//	never free globals or procedurals
//		if (image->resource == RESOURCE_GLOBAL || image->type == it_procedural)
		if (image->resource == RESOURCE_GLOBAL)
			continue;
		if (image->registration_sequence == registration_sequence)
			continue;		// used this sequence
		if (!image->registration_sequence)
			continue;		// free image_t slot
		if (image->type == it_pic)
			continue;		// don't free pics

		GL_FreeImage (image);
	}
}


/*
===============
R_SetPaletteDir
===============
*/
void R_SetPaletteDir (const char *name)
{
	ri.Cvar_Set( "r_palettedir", name);			//added for daikatana
	GL_InitImages ();
}

///////////////////////////////////////////////////////////////////////////////
//	R_SetResourceDir
//
//	Nelno:	3/23/98
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
Draw_GetPalette
===============
*/
char Palette_filename[1024];


int Draw_GetPalette (void)
{
	int		i, j;
	int		r, g, b;
	unsigned v;
	byte	*pic, *pal;
	int		width, height;
	int		default_bmp = 0;
	
	

	if (!r_palettedir->string || r_palettedir->string [0] == 0x00)
	{
		Com_sprintf (Palette_filename,sizeof(Palette_filename), "pics/colormap.bmp");
		default_bmp = 1;
	}
	else
	{
		Com_sprintf (Palette_filename,sizeof(Palette_filename), "textures/%s/colormap.bmp", r_palettedir->string);
	}

	LoadBMP (Palette_filename, &pic, &pal, &height, &width);
	
	if (!pal && !default_bmp)
	{
		Com_sprintf (Palette_filename,sizeof(Palette_filename), "pics/colormap.bmp");
		LoadBMP (Palette_filename, &pic, &pal, &height, &width);
	}

	if (!pal)
	{
		ri.Sys_Error (ERR_FATAL, "Couldn't load colormap.bmp from %s",Palette_filename);
	}

	for (i=0 ; i<256 ; i++)
	{
		r = pal[i*3+0];
		g = pal[i*3+1];
		b = pal[i*3+2];
		
		v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
		d_8to24table[i] = v;
	}

	d_8to24table[255] &= 0xffffff;	// 255 is transparent

    // Ash - now build the gl_palette
    for ( i = 0, j = 0; i < 256; i++, j+=3 )
    {
        d_gl_8to24table[j+0] = ( d_8to24table [i] >> 0 ) & 0xff;
        d_gl_8to24table[j+1] = ( d_8to24table [i] >> 8 ) & 0xff;
        d_gl_8to24table[j+2] = ( d_8to24table [i] >> 16 ) & 0xff;
    }

	ri.X_Free(pic);
	ri.X_Free(pal);

	return 0;
}


/*
===============
GL_InitImages
===============
*/
void	GL_InitImages (void)
{
	int		i, j;
	float	g = vid_gamma->value;
//	char	filename[1024];

	registration_sequence = 1;

	// init intensity conversions
	intensity = ri.Cvar_Get ("intensity", "1", 0);

	if ( intensity->value <= 1 )
		ri.Cvar_Set( "intensity", "1" );

	gl_state.inverse_intensity = 1 / intensity->value;

	Draw_GetPalette ();

/*
	if ( qglColorTableEXT )
	{
		sprintf (filename, "textures/%s/16to8.dat", r_palettedir->string);
		if (ri.FS_LoadFile( filename, &gl_state.d_16to8table ) == -1)
			ri.FS_LoadFile( "pics/16to8.dat", &gl_state.d_16to8table );

		if ( !gl_state.d_16to8table )
			ri.Sys_Error( ERR_FATAL, "Couldn't load pics/16to8.dat");
	}
*/

	if ( gl_config.renderer & ( GL_RENDERER_VOODOO | GL_RENDERER_VOODOO2 ) )
	{
		g = 1.0F;
	}

	for ( i = 0; i < 256; i++ )
	{
		if ( g == 1 )
		{
			gammatable[i] = i;
		}
		else
		{
			float inf;

			inf = 255 * pow ( (i+0.5)/255.5 , g ) + 0.5;
			if (inf < 0)
				inf = 0;
			if (inf > 255)
				inf = 255;
			gammatable[i] = inf;
		}
	}

	for (i=0 ; i<256 ; i++)
	{
		j = i*intensity->value;
		if (j > 255)
			j = 255;
		intensitytable[i] = j;
	}
}

/*
===============
GL_ShutdownImages
===============
*/
void	GL_ShutdownImages (void)
{
	int		i;
	image_t	*image;

	for (i=0, image=gltextures ; i<numgltextures ; i++, image++)
	{
		if (!image->registration_sequence)
			continue;		// free image_t slot

		//	Nelno:	why in the hell wasn't he just calling this before???
		GL_FreeImage (image);
/*
		// free it
		qglDeleteTextures (1, &image->texnum);
		memset (image, 0, sizeof(*image));
*/
	}
}

