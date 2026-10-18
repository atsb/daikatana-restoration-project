
// draw.c

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#include "gl_local.h"

#include "dk_misc.h"
#include "dk_array.h"
#include "dk_buffer.h"

#include "dk_ref_common.h"
#include "dk_ref_font.h"

image_t     *draw_chars;
void *default_chars_image;

extern  qboolean    scrap_dirty;
void Scrap_Upload (void);

typedef struct
{
    CVector  origin;
    CVector  normal;
    char    str [64];
    float   scale;
    short   flags;
} textEntity_t;

#define MAX_TEXT_ENTITIES   64

int             text_numEntities = 0;   //  reset each frame
textEntity_t    text_EntityList [MAX_TEXT_ENTITIES];

/*
=============
R_GetPicData
=============
*/
void *R_GetPicData( const char *name, int *pWidth, int *pHeight, resource_t resource )
{
	// the image
	image_t	*image;

	// SCG[5/7/99]: Sigh.  Hack for skins that come from .wal files
	if( strstr( name, "skins" ) )
	{
		image = ( image_t * ) RegisterSkin( name, resource );
	}
	else
	{
		image = R_FindImage( name, it_pic, resource );
	}


	if( image == NULL )
	{
		image = r_notexture;
	}

	// set the width if requested
	if( pWidth != NULL )
	{
		*pWidth = image->width;
	}

	// set the height if requested
	if( pHeight != NULL )
	{
		*pHeight = image->height;
	}

	// get a pointer to the image
	return image;
}


/*
===============
Draw_InitLocal
===============
*/
void Draw_InitLocal (void)
{
    char    dk_path [1024];

    //  Nelno:  load Daikatana console characters
    if (!r_resourcedir || !r_resourcedir->string || r_resourcedir->string [0] == 0x00)
        default_chars_image = draw_chars = (image_t*)R_GetPicData( "pics/dkchars.pcx", NULL, NULL, RESOURCE_GLOBAL );
    else
    {
        Com_sprintf (dk_path,sizeof(dk_path), "%sdkchars.pcx", r_resourcedir->string);

        default_chars_image = draw_chars = (image_t*)R_GetPicData ( dk_path, NULL, NULL, RESOURCE_GLOBAL );

        if (!draw_chars)
            default_chars_image = draw_chars = (image_t*)R_GetPicData ( "pics/dkchars.pcx", NULL, NULL, RESOURCE_GLOBAL );
    }

    // load console characters (don't bilerp characters)
    //  draw_chars = GL_FindImage ("pics/conchars.pcx", it_pic);
    GL_Bind( draw_chars->texnum );
    qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	LoadFont("statbar_font" );
	LoadFont("scr_cent" );
	LoadFont("int_font" );
	LoadFont("mainnums" );
	LoadFont("mainnumsred" );
	LoadFont("con_font" );
	LoadFont("int_font_bright" );
	void *button_font = LoadFont("int_buttons" );
	if (button_font != NULL)
	{
		dk_font *button_font_data = (dk_font *)button_font;
		if (button_font_data->image != NULL)
		{
			image_t *button_image = (image_t *)button_font_data->image;
			GL_Bind(button_image->texnum);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
	}

	R_GetPicData("pics/loadscreens/con_0.bmp", NULL, NULL, RESOURCE_GLOBAL );
	R_GetPicData("pics/loadscreens/con_1.bmp", NULL, NULL, RESOURCE_GLOBAL );
	R_GetPicData("pics/loadscreens/con_2.bmp", NULL, NULL, RESOURCE_GLOBAL );
	R_GetPicData("pics/loadscreens/con_3.bmp", NULL, NULL, RESOURCE_GLOBAL );
	R_GetPicData("pics/loadscreens/con_4.bmp", NULL, NULL, RESOURCE_GLOBAL );
	R_GetPicData("pics/loadscreens/con_5.bmp", NULL, NULL, RESOURCE_GLOBAL );
}

typedef struct  drawFont_s
{
    byte    *charWidth;
    byte    *charX;
    byte    *charY;
    image_t *image;
} drawFont_t;

extern void *char_flare_image;

/*
================
R_DrawStringFlare
================
*/
#define FLARE_OFFSET    5
#define XINDEX  0
#define YINDEX  1

int R_DrawStringFlare( DRAWSTRUCT& drawStruct )
{
	//==================================
	// consolidation change: SCG 3-15-99
	float			color[4];
	dk_font			*font;
	unsigned int	nStateFlags;

	nStateFlags = GLSTATE_CULL_FACE | GLSTATE_CULL_FACE_FRONT;

	// make sure the string data is valid
	if( drawStruct.szString == NULL )
	{
		return drawStruct.nXPos;
	}

	// make sure the image data is valid
	if( drawStruct.pImage != NULL ) 
	{
		// get the font data
		font = (dk_font *) drawStruct.pImage;
	}

	if( !( drawStruct.nFlags & DSFLAG_FLAT ) )
	{
		nStateFlags |= GLSTATE_TEXTURE_2D;
	}

	// get the string
	unsigned char *pString = ( unsigned char * ) drawStruct.szString;

	 // get the image.
	image_t *image = (image_t *) char_flare_image;

	if( image == NULL )
	{
		image = r_notexture;
	}

	// set default color
	color[0] = color[1] = color[2] = color[3] = 1.0;

	//
	// set up drawing conditions based on flags
	//
	if( !image->has_alpha )
	{
		nStateFlags |= GLSTATE_ALPHA_TEST;
	}

	// set blending
	if( drawStruct.nFlags & ( DSFLAG_BLEND | DSFLAG_ALPHA ) )
	{
		nStateFlags |= GLSTATE_BLEND;
		GL_TexEnv( GL_MODULATE );
	}
	// set alpha
	if( drawStruct.nFlags & DSFLAG_ALPHA )
	{
		color[3] = drawStruct.fAlpha;
	}
	// set the color
	if( drawStruct.nFlags & DSFLAG_COLOR )
	{
		color[0] = drawStruct.rgbColor.x;
		color[1] = drawStruct.rgbColor.y;
		color[2] = drawStruct.rgbColor.z;
	}

	// set palette
	if( qglColorTableEXT && ( drawStruct.nFlags & DSFLAG_PALETTE ) && !( image->has_alpha ) )
	{
		nStateFlags |= GLSTATE_SHARED_PALETTE;
	}

	// make sure the scrap data is up to date
	if (scrap_dirty)
	{
		Scrap_Upload ();
	}

	int		x1, x2, y1, y2;
	float	sl, tl, sh, th;
	float	fScaleX, fScaleY;

	if( drawStruct.nFlags & DSFLAG_SCALE )
	{
		fScaleX = drawStruct.fScaleX;
		fScaleY = drawStruct.fScaleY;
	}
	else
	{
		fScaleX = drawStruct.fAlpha * 1.5;
		fScaleY = drawStruct.fAlpha * 1.5;
	}

	GL_SetState( nStateFlags );

	GL_SetTexturePalette( image, FALSE );

	qglColor4fv( color );

	GL_Bind( image->texnum );
	if( !( drawStruct.nFlags & DSFLAG_NO_N64FILTER ) )
		R_TextureFilterBeginBase( image );

	if( !( drawStruct.nFlags & DSFLAG_NOMIP ) )
		GL_TextureModeUI(image);

	if( drawStruct.nFlags & DSFLAG_NOMIP )
	{
		if( ( drawStruct.nFlags & DSFLAG_NO_N64FILTER ) &&
			gl_texture_filter != NULL && gl_texture_filter->value == 2.0f )
		{
			/* N64 is bypassed for this draw (loading screen/HUD/UI), so use
			   the same non-shader bilinear sampling as the Bilinear mode. */
			qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		}
		else
		{
			qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		}
	}

	float x_frac = drawStruct.nXPos;

	int nHalfW = image->width / 2;
	int nHalfH = image->height / 2;

//	float fCosTheta = cos( ref_laserRotation / ( 180 / M_PI ) );
//	float fSinTheta = sin( ref_laserRotation / ( 180 / M_PI ) );

	if( drawStruct.pImage != NULL )
	{
		qglBegin( GL_QUADS );
		for( ;pString[ 0 ] != '\0'; pString++ )
		{
			unsigned char c = pString[ 0 ];

			if( c == ' ' )
			{
				x_frac += ( font->height >> 1 );// * fScaleX;
				continue;
			}

			if( font->char_width[ c ] == 0 )
			{
				continue;
			}

			float fCenterX = x_frac + nHalfW;
			float fCenterY = drawStruct.nYPos + nHalfH;

			x1 = ( int ) fCenterX - nHalfW * fScaleX;
			x2 = ( int ) fCenterX + nHalfW * fScaleX;
			y1 = ( int ) fCenterY - nHalfH * fScaleY;
			y2 = ( int ) fCenterY + nHalfH * fScaleY;

			sl = image->sl;
			tl = image->tl;
			sh = image->sh;
			th = image->sh;

			qglTexCoord2f( sl, tl );
			qglVertex2f( x1, y1 );
			qglTexCoord2f( sh, tl );
			qglVertex2f( x2, y1 );
			qglTexCoord2f( sh, th );
			qglVertex2f( x2, y2 );
			qglTexCoord2f( sl, th );
			qglVertex2f( x1, y2 );

			x_frac += ( 1 + font->char_width[ c ] );// * fScaleX;
		}
		qglEnd();
	}
	else
	{
		qglBegin( GL_QUADS );
		for( ;pString[ 0 ] != '\0'; pString++ )
		{
			unsigned char c = pString[ 0 ];

			if( c == ' ' )
			{
				x_frac += 4 * fScaleX;
				continue;
			}

			float fCenterX = x_frac + nHalfW;
			float fCenterY = drawStruct.nYPos + nHalfH;

			x1 = ( int ) fCenterX - nHalfW * fScaleX;
			x2 = ( int ) fCenterX + nHalfW * fScaleX;
			y1 = ( int ) fCenterY - nHalfH * fScaleY;
			y2 = ( int ) fCenterY + nHalfH * fScaleY;

			sl = image->sl;
			tl = image->tl;
			sh = image->sh;
			th = image->sh;

			qglTexCoord2f( sl, tl );
			qglVertex2f( x1, y1 );
			qglTexCoord2f( sh, tl );
			qglVertex2f( x2, y1 );
			qglTexCoord2f( sh, th );
			qglVertex2f( x2, y2 );
			qglTexCoord2f( sl, th );
			qglVertex2f( x1, y2 );

			x_frac += 9 * fScaleX;
		}
		qglEnd();
	}

	if( !( drawStruct.nFlags & DSFLAG_NO_N64FILTER ) )
		R_TextureFilterEnd();

	GL_TexEnv (GL_REPLACE);

	return ( int ) x_frac;
	// consolidation change: SCG 3-15-99
	//==================================
}

/*
================
R_DrawString
================
*/
int R_DrawString( DRAWSTRUCT& drawStruct )
{
//==================================
// consolidation change: SCG 3-15-99
	float			color[4];
	image_t			*image;
	dk_font			*font;
	unsigned int	nStateFlags;

	nStateFlags = GLSTATE_CULL_FACE | GLSTATE_CULL_FACE_FRONT;

	// make sure the string data is valid
	if( drawStruct.szString == NULL || drawStruct.szString[0] == NULL )
	{
		return drawStruct.nXPos;
	}

	// make sure the image data is valid
    if( drawStruct.pImage == NULL ) 
	{
		image = ( image_t * ) default_chars_image;
	}
    else 
	{
		// get the font data
		font = (dk_font *) drawStruct.pImage;

		// get the image.
		image = (image_t *) font->image;
	}

	if( image == NULL )
	{
		image = r_notexture;
	}

	if( !( drawStruct.nFlags & DSFLAG_FLAT ) )
	{
		nStateFlags |= GLSTATE_TEXTURE_2D;
	}

	// get the string
	unsigned char *pString = ( unsigned char * ) drawStruct.szString;

	// set default color
	color[0] = color[1] = color[2] = color[3] = 1.0;

	//
	// set up drawing conditions based on flags
	//

	// set blending
	if( drawStruct.nFlags & DSFLAG_BLEND )
	{
		nStateFlags |= ( GLSTATE_BLEND | GLSTATE_ALPHA_TEST );
		GL_TexEnv( GL_MODULATE );
	}

	// set alpha
	if( drawStruct.nFlags & DSFLAG_ALPHA )
	{
		color[3] = drawStruct.fAlpha;
	}

	// set the color
	if( drawStruct.nFlags & DSFLAG_COLOR )
	{
		color[0] = drawStruct.rgbColor.x;
		color[1] = drawStruct.rgbColor.y;
		color[2] = drawStruct.rgbColor.z;
	}

	// set palette
	if( qglColorTableEXT && ( drawStruct.nFlags & DSFLAG_PALETTE ) && !( image->has_alpha ) )
	{
		nStateFlags |= GLSTATE_SHARED_PALETTE;
	}

	// make sure the scrap data is up to date
    if (scrap_dirty)
	{
        Scrap_Upload ();
	}

	int		x1, x2, y1, y2;
    float	sl, tl, sh, th;
	float	fScaleX, fScaleY;

	if( drawStruct.nFlags & DSFLAG_SCALE )
	{
		fScaleX = drawStruct.fScaleX;
		fScaleY = drawStruct.fScaleY;
	}
	else
	{
		fScaleX = 1;
		fScaleY = 1;
	}

	GL_SetState( nStateFlags );

	GL_SetTexturePalette( image, FALSE );

	qglColor4fv( color );

    GL_Bind( image->texnum );
	if( !( drawStruct.nFlags & DSFLAG_NO_N64FILTER ) )
		R_TextureFilterBeginBase( image );

	if( !( drawStruct.nFlags & DSFLAG_NOMIP ) )
		GL_TextureModeUI(image);

	if( drawStruct.nFlags & DSFLAG_NOMIP )
	{
		if( ( drawStruct.nFlags & DSFLAG_NO_N64FILTER ) &&
			gl_texture_filter != NULL && gl_texture_filter->value == 2.0f )
		{
			/* N64 is bypassed for this draw (loading screen/HUD/UI), so use
			   the same non-shader bilinear sampling as the Bilinear mode. */
			qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		}
		else
		{
			qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		}
	}

	float x_frac = drawStruct.nXPos;

	if( drawStruct.pImage != NULL )
	{
		qglBegin( GL_QUADS );
		for( ;pString[ 0 ] != '\0'; pString++ )
		{
			unsigned char c = pString[ 0 ];

			if( c == ' ' )
			{
				x_frac += ( font->height >> 1 ) * fScaleX;
				continue;
			}

			if( font->char_width[ c ] == 0 )
			{
				continue;
			}

			x1 = ( int ) x_frac;
			x2 = x1 + font->char_width[ c ] * fScaleX;
			y1 = drawStruct.nYPos;
			y2 = y1 + font->height * fScaleY;

			sl = image->sl + font->char_pos_x[ c ];
			tl = image->tl + font->char_pos_y[ c ];
			sh = sl + font->char_width[ c ];
			th = tl + font->height;

			sl /= image->width;
			tl /= image->height;
			sh /= image->width;
			th /= image->height;

			qglTexCoord2f( sl, tl );
			qglVertex2f( x1, y1 );
			qglTexCoord2f( sh, tl );
			qglVertex2f( x2, y1 );
			qglTexCoord2f( sh, th );
			qglVertex2f( x2, y2 );
			qglTexCoord2f( sl, th );
			qglVertex2f( x1, y2 );

			x_frac += ( 1 + font->char_width[ c ] ) * fScaleX;
		}
		qglEnd();
	}
	else
	{
		for( ;pString[ 0 ] != '\0'; pString++ )
		{
			unsigned char c = pString[ 0 ];

			if( c == ' ' )
			{
				x_frac += 4 * fScaleX;
				continue;
			}

			x1 = ( int ) x_frac;
			x2 = x1 + 8 * fScaleX;
			y1 = drawStruct.nYPos;
			y2 = y1 + 8 * fScaleY;

			sl = image->sl + ( c & 15 ) * 8;
			tl = image->tl + ( c >> 4 ) * 8;
			sh = sl + 8;
			th = tl + 8;

			sl /= image->width;
			tl /= image->height;
			sh /= image->width;
			th /= image->height;

			qglBegin( GL_QUADS );
			qglTexCoord2f( sl, tl );
			qglVertex2f( x1, y1 );
			qglTexCoord2f( sh, tl );
			qglVertex2f( x2, y1 );
			qglTexCoord2f( sh, th );
			qglVertex2f( x2, y2 );
			qglTexCoord2f( sl, th );
			qglVertex2f( x1, y2 );
			qglEnd();

			x_frac += 9 * fScaleX;
		}
	}

	if( !( drawStruct.nFlags & DSFLAG_NO_N64FILTER ) )
		R_TextureFilterEnd();

	GL_TexEnv (GL_REPLACE);

	return ( int ) x_frac;
// consolidation change: SCG 3-15-99
//==================================
}

/*
================
R_DrawChar

Draws one 8*8 graphics character with 0 being transparent.
It can be clipped to the top of the screen to allow the console to be
smoothly scrolled off.
================
*/
void R_DrawChar( int x, int y, int num )
{
	int				row, col;
	float			frow, fcol, size;

	num &= 255;
	
	if ( (num&127) == 32 )
		return;		// space

	if (y <= -8)
		return;			// totally off screen

	row = num>>4;
	col = num&15;

	frow = row*0.0625;
	fcol = col*0.0625;
	size = 0.0625;

	GL_Bind (draw_chars->texnum);
	R_TextureFilterBeginBase( draw_chars );

	qglBegin (GL_QUADS);
	qglTexCoord2f (fcol, frow);
	qglVertex2f (x, y);
	qglTexCoord2f (fcol + size, frow);
	qglVertex2f (x+8, y);
	qglTexCoord2f (fcol + size, frow + size);
	qglVertex2f (x+8, y+8);
	qglTexCoord2f (fcol, frow + size);
	qglVertex2f (x, y+8);
	qglEnd ();
	R_TextureFilterEnd();
}

/*
================
R_DrawPic
================
*/
void R_DrawPic( DRAWSTRUCT& drawStruct )
{
//==================================
// consolidation change: SCG 3-11-99

	float			color[4];
	unsigned int	nStateFlags;

	qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	nStateFlags = ( GLSTATE_CULL_FACE | GLSTATE_CULL_FACE_FRONT );

    // get the image.
    image_t *image = (image_t *) drawStruct.pImage;

	// set default color
	color[0] = color[1] = color[2] = color[3] = 1.0;

	//
	// set up drawing conditions based on flags
	//

	if( !( drawStruct.nFlags & DSFLAG_FLAT ) )
	{
		nStateFlags |= GLSTATE_TEXTURE_2D;
	}

	// set blending
	if( drawStruct.nFlags & ( DSFLAG_BLEND | DSFLAG_ALPHA ) )
	{
		nStateFlags |= GLSTATE_BLEND;
	}

	// set alpha
	if( drawStruct.nFlags & DSFLAG_ALPHA )
	{
		color[3] = drawStruct.fAlpha;
	}

	// set the color
	if( drawStruct.nFlags & DSFLAG_COLOR )
	{
		color[0] = drawStruct.rgbColor.x;
		color[1] = drawStruct.rgbColor.y;
		color[2] = drawStruct.rgbColor.z;
	}

	// set palette
	if( qglColorTableEXT && ( drawStruct.nFlags & DSFLAG_PALETTE ) && !( image->has_alpha ) )
	{
		nStateFlags |= GLSTATE_SHARED_PALETTE;
	}

	// make sure the scrap data is up to date
    if (scrap_dirty)
	{
        Scrap_Upload ();
	}

	int	x1, x2, y1, y2;

	GL_SetState( nStateFlags );

	if( drawStruct.nFlags & ( DSFLAG_BLEND | DSFLAG_ALPHA ) )
	{
		GL_TexEnv( GL_MODULATE );
	}

	if( qglColorTableEXT && ( drawStruct.nFlags & DSFLAG_PALETTE ) && !( image->has_alpha ) )
	{
		GL_SetTexturePalette( image, TRUE );
	}

	qglColor4fv( color );

	// SCG[3/30/99]: Added flat polygon drawing
	if( drawStruct.nFlags & DSFLAG_FLAT )
	{
		x1 = drawStruct.nLeft;
		x2 = drawStruct.nLeft + drawStruct.nRight;
		y1 = drawStruct.nTop;
		y2 = drawStruct.nTop + drawStruct.nBottom;

		qglBegin( GL_QUADS );
		qglVertex2f( x1, y1 );
		qglVertex2f( x2, y1 );
		qglVertex2f( x2, y2 );
		qglVertex2f( x1, y2 );
		qglEnd();
	}
	else
	{
		x1 = drawStruct.nXPos;
		x2 = drawStruct.nXPos + image->width;
		y1 = drawStruct.nYPos;
		y2 = drawStruct.nYPos + image->height;

		// texture coordinates
		float	sl, tl, sh, th;
	
		if( drawStruct.nFlags & DSFLAG_FLIPX )
		{
			sl = image->sh; 
			sh = image->sl; 
		}
		else
		{
			sl = image->sl; 
			sh = image->sh; 
		}
		if( drawStruct.nFlags & DSFLAG_FLIPY )
		{
			tl = image->th; 
			th = image->tl;
		}
		else
		{
			tl = image->tl; 
			th = image->th;
		}


		if( drawStruct.nFlags & DSFLAG_SUBIMAGE )
		{
			sl += drawStruct.nLeft;
			tl += drawStruct.nTop;
			sh = sl + drawStruct.nRight;
			th = tl + drawStruct.nBottom;
		}

		if( drawStruct.nFlags & DSFLAG_SCALE )
		{
			x2 = drawStruct.nXPos + ( ( image->width * drawStruct.fScaleX ) + 0.5 );
			y2 = drawStruct.nYPos + ( ( image->height * drawStruct.fScaleY ) + 0.5 );
		}

		GL_Bind( image->texnum );

		if( !( drawStruct.nFlags & DSFLAG_NO_N64FILTER ) )
			R_TextureFilterBeginBase( image );

		if( !( drawStruct.nFlags & DSFLAG_NOMIP ) )
			GL_TextureModeUI(image);

		if( drawStruct.nFlags & DSFLAG_CLAMP )
		{
			qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}

		if( drawStruct.nFlags & DSFLAG_NOMIP )
		{
			qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		}

		qglBegin( GL_QUADS );
		qglTexCoord2f( sl, tl );
		qglVertex2f( x1, y1 );
		qglTexCoord2f( sh, tl );
		qglVertex2f( x2, y1 );
		qglTexCoord2f( sh, th );
		qglVertex2f( x2, y2 );
		qglTexCoord2f( sl, th );
		qglVertex2f( x1, y2 );
		qglEnd();

		if( !( drawStruct.nFlags & DSFLAG_NO_N64FILTER ) )
			R_TextureFilterEnd();

		if( drawStruct.nFlags & DSFLAG_CLAMP )
		{
			qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		}
	}

	GL_TexEnv( GL_REPLACE );

// consolidation change: SCG 3-11-99
//==================================
}

///////////////////////////////////////////////////////////////////////////////
//  Draw_CharFlare
//  
//  Draws one 8*8 graphics character with an alpha channel and optional effects
///////////////////////////////////////////////////////////////////////////////

extern  image_t *charFlare;
extern  float   ref_laserRotation;

#define FLARE_OFFSET    5
#define XINDEX  0
#define YINDEX  1

///////////////////////////////////////////////////////////////////////////////
//  Draw_CharOriented
//  
//  Draws one 8*8 graphics character with 0 being transparent.
///////////////////////////////////////////////////////////////////////////////

void Draw_CharOriented (const CVector &origin, const CVector &right, const CVector &up, int num)
{
    int             char_row, char_col;
    float           frow, fcol, size;
    CVector          vertex;

	GL_SetState( GLSTATE_PRESET1 & ~( GLSTATE_DEPTH_MASK | GLSTATE_CULL_FACE ) );
//	GL_SetState( GLSTATE_PRESET1 );
    num &= 255;
    
    if ((num & 127) == 32)
        return;     // space

    char_row = num >> 4;
    char_col = num & 15;

    frow = char_row * 0.0625;
    fcol = char_col * 0.0625;
    size = 0.0625;

    GL_Bind (draw_chars->texnum);
	R_TextureFilterBeginBase( draw_chars );

    qglBegin (GL_QUADS);

    qglTexCoord2f (fcol, frow);
    qglVertex3fv (origin);

    qglTexCoord2f (fcol + size, frow);
    vertex = origin + right;
    qglVertex3f (vertex.x, vertex.y, vertex.z);

    qglTexCoord2f (fcol + size, frow + size);
    vertex = origin + right;
    vertex = vertex + up;
    qglVertex3f (vertex.x, vertex.y, vertex.z);

    qglTexCoord2f (fcol, frow + size);
    vertex = origin + up;
    qglVertex3f (vertex.x, vertex.y, vertex.z);

    qglEnd ();
	R_TextureFilterEnd();
}

///////////////////////////////////////////////////////////////////////////////
//  Draw_StringOriented
//
//  draw a string of text in 3d, oriented along normal
///////////////////////////////////////////////////////////////////////////////

void    Draw_StringOriented (CVector &origin, CVector &normal, char *str, float scale, int flags)
{
    int     i, len;
    CVector  right, up, org;
    CVector  right_8, up_8;
    float   ofs;

    if (scale == 0.0)
        scale = 8.0;
    len = strlen (str);

	GL_SetState( GLSTATE_PRESET1 & ~( GLSTATE_DEPTH_MASK | GLSTATE_CULL_FACE ) );
//	GL_SetState( GLSTATE_PRESET1 );

    if ( flags & TEXT_ORIENTED && normal && 
         !(normal.x == 0.0 && normal.y == 0.0 && normal.z == 0.0))
    {
        //  oriented to normal
        if (normal.x == 0.0 && normal.y == 0.0)
        {
            //  horizontal surface
            if (normal.z < 0.0)
            {
                //  facing down
                right.x = -1.0;
                right.y = 0.0;
                right.z = 0.0;
            }
            else
            {
                //  facing up
                right.x = 1.0;
                right.y = 0.0;
                right.z = 0.0;
            }

        }
        else
        {
            right.x = normal.y;
            right.y = -normal.x;
            right.z = normal.z;
        }

        right = right * -1.0;
        CrossProduct (normal, right, up);

        //  move origin out a bit to avoid coplaner polys
        VectorMA (origin, normal, 0.125, org);
    }
    else
    {
        //  always orient to view
        right = vright;
        up = vup;
        
        //  move origin out a bit to avoid coplaner polys
        if (normal)
            VectorMA (origin, normal, 0.125, org);
    }

    right_8 = right * scale;
    up_8 = up * -scale;

    if (flags & TEXT_CENTERED)
    {
        ofs = (len * scale) / 2.0;
        VectorMA (org, right, -ofs, org);
        VectorMA (org, up, scale / 2.0, org);
    }

	GL_SetState( ( GLSTATE_PRESET1 | GLSTATE_ALPHA_TEST ) & ~( GLSTATE_DEPTH_MASK | GLSTATE_CULL_FACE ) );
//	GL_SetState( ( GLSTATE_PRESET1 | GLSTATE_ALPHA_TEST ) );

    for (i = 0; i < len; i++)
    {
        Draw_CharOriented (org, right_8, up_8, (int) str [i]);

        org = org + right_8;
    }
}

///////////////////////////////////////////////////////////////////////////////
//  Text_DrawEntities
//
///////////////////////////////////////////////////////////////////////////////

void    Text_DrawEntities (void)
{
    int             i;
    textEntity_t    *te;

    for (i = 0; i < text_numEntities; i++)
    {
        te = &text_EntityList [i];
        
        Draw_StringOriented (te->origin, te->normal, te->str, te->scale, te->flags);
    }
}

///////////////////////////////////////////////////////////////////////////////
//  Text_AddEntity
//
//  Add a text entity to the list of text entities.
///////////////////////////////////////////////////////////////////////////////

void    Text_AddEntity (const CVector &origin, const CVector &normal, char *str, float scale, int flags)
{
    textEntity_t    *te;

    if (text_numEntities >= MAX_TEXT_ENTITIES)
        return;

    te = &text_EntityList [text_numEntities];
    text_numEntities++;

    te->origin = origin;
    if (normal)
    {
	    te->normal = normal;
    }
	else
    {
	    te->normal = vec3_origin;
	}

    strcpy (te->str, str);
    te->scale = scale;
    te->flags = flags;
}

/*
=============
Draw_TileClear

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void DrawTileClear (int x, int y, int w, int h, const char *pic)
{
    image_t *image = (image_t*)R_GetPicData( pic, NULL, NULL, RESOURCE_EPISODE );

	qglBlendFunc( GL_ONE, GL_ONE );

    if (!image)
    {
        ri.Con_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
        return;
    }

	unsigned int	nStateFlags = GLSTATE_PRESET1;

	nStateFlags &= ~( GLSTATE_DEPTH_MASK | GLSTATE_CULL_FACE );

	if ( ( !( gl_config.renderer == GL_RENDERER_MCD ) || !( gl_config.renderer & GL_RENDERER_RENDITION ) ) && image->has_alpha)
	{
		nStateFlags |= GLSTATE_ALPHA_TEST;
	}

	if( qglColorTableEXT && !( image->has_alpha ) )
	{
		nStateFlags |= GLSTATE_SHARED_PALETTE;
	}

	GL_SetState( nStateFlags );

	if( qglColorTableEXT && !( image->has_alpha ) )
	{
		GL_SetTexturePalette( image, FALSE );
	}

	float sl = ( float ) x / ( float ) image->width;
	float sh = ( float ) ( x + w ) / ( float ) image->width;
	float tl = ( float ) y / ( float ) image->height;
	float th = ( float ) ( y + h ) / ( float ) image->height;

    GL_Bind( image->texnum );
    R_TextureFilterBeginBase(image);

    qglBegin( GL_QUADS );
    qglTexCoord2f( sl, tl );
    qglVertex2f( x, y );
    qglTexCoord2f( sh, tl);
    qglVertex2f( x + w, y );
    qglTexCoord2f( sh, th );
    qglVertex2f( x + w, y + h );
    qglTexCoord2f( sl, th );
    qglVertex2f( x, y + h );
    qglEnd();
    R_TextureFilterEnd();
}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_Fill( int x, int y, int w, int h, CVector rgbColor, float alpha )
{
	unsigned int	nStateFlags;

	nStateFlags = GLSTATE_PRESET1;
	
//	nStateFlags &= ~( GLSTATE_TEXTURE_2D );
	nStateFlags &= ~( GLSTATE_TEXTURE_2D | GLSTATE_DEPTH_MASK | GLSTATE_DEPTH_TEST );

	qglShadeModel( GL_FLAT );
	if( alpha < 1.0 )
	{
		nStateFlags |= GLSTATE_BLEND;
		GL_TexEnv( GL_MODULATE );
	}

	GL_SetState( nStateFlags );
    qglColor4f( rgbColor.x, rgbColor.y, rgbColor.z, alpha );

    qglBegin(GL_QUADS);
    qglVertex2f( x,y );
    qglVertex2f( x + w, y );
    qglVertex2f( x + w, y + h );
    qglVertex2f( x, y + h );
    qglEnd ();

	GL_TexEnv (GL_REPLACE);
}

//=============================================================================

/*
=============
Draw_StretchRaw

//  uploads raw data to texture number 0, then displays that texture on the screen
//  at the given coordinates...  this has got to be bad to use!
=============
*/
extern unsigned r_rawpalette[256];

/*
void Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows, byte *data)
{
    unsigned    image32[256*256];
    unsigned char image8[256*256];
    int         i, j, trows;
    byte        *source;
    int         frac, fracstep;
    float       hscale;
    int         row;
    float       t;

    GL_Bind (0);

    if (rows<=256)
    {
        hscale = 1;
        trows = rows;
    }
    else
    {
        hscale = rows/256.0;
        trows = 256;
    }
    t = rows*hscale / 256;

    if ( !qglColorTableEXT )
    {
        unsigned *dest;

        //  Nelno:  not in paletted mode, so map the picture to r_rawpalette...

        for (i=0 ; i<trows ; i++)
        {
            row = (int)(i*hscale);
            if (row > rows)
                break;
            source = data + cols*row;
            dest = &image32[i*256];
            fracstep = cols*0x10000/256;
            frac = fracstep >> 1;
            for (j=0 ; j<256 ; j++)
            {
                dest[j] = r_rawpalette[source[frac>>16]];
                frac += fracstep;
            }
        }

        qglTexImage2D (GL_TEXTURE_2D, 0, 3, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, image32);
    }
    else
    {
        unsigned char *dest;

        for (i=0 ; i<trows ; i++)
        {
            row = (int)(i*hscale);
            if (row > rows)
                break;
            source = data + cols*row;
            dest = &image8[i*256];
            fracstep = cols*0x10000/256;
            frac = fracstep >> 1;
            for (j=0 ; j<256 ; j++)
            {
                dest[j] = source[frac>>16];
                frac += fracstep;
            }
        }

        qglTexImage2D( GL_TEXTURE_2D, 
                       0, 
                       GL_COLOR_INDEX8_EXT, 
                       256, 256, 
                       0, 
                       GL_COLOR_INDEX, 
                       GL_UNSIGNED_BYTE, 
                       image8 );
    }
    qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (gl_config.renderer == GL_RENDERER_MCD )
//		GL_SetState( GLSTATE_PRESET1 & ~( GLSTATE_DEPTH_MASK | GLSTATE_CULL_FACE ) );
		GL_SetState( GLSTATE_PRESET1 );
	else
//		GL_SetState( ( GLSTATE_PRESET1 | GLSTATE_ALPHA_TEST ) & ~( GLSTATE_DEPTH_MASK | GLSTATE_CULL_FACE ) );
		GL_SetState( ( GLSTATE_PRESET1 | GLSTATE_ALPHA_TEST ) );

    qglBegin (GL_QUADS);
    qglTexCoord2f (0, 0);
    qglVertex2f (x, y);
    qglTexCoord2f (1, 0);
    qglVertex2f (x+w, y);
    qglTexCoord2f (1, t);
    qglVertex2f (x+w, y+h);
    qglTexCoord2f (0, t);
    qglVertex2f (x, y+h);
    qglEnd ();
}
*/

///////////////////////////////////////////////////////////////////////////////
//  DrawConsolePic
//
//  draws two pics, one is 256x256 wide, one is 64x256 wide
///////////////////////////////////////////////////////////////////////////////

/*
void DrawConsolePic (int x, int y, int w, int h, int episode_num, int map_num)
{
	char buf[256];
	float	scale_x;
	float	scale_y;
	int		width_256, width_128;
	int		pic1_x, pic2_x, pic3_x;
	int		height;

	DRAWSTRUCT drawStruct;
	int image_width, image_height;
	qboolean level_load;


	// per episode/map picture   (ie: e1m1.pcx)
	sprintf(buf,"pics/loadscreens/e%dm%d_0.bmp", episode_num, map_num); 

	level_load = (ri.FS_LoadFile(buf, NULL) != -1);  // file exists?

	if (!level_load) // failed open, resort to default
	{
		// draw 64 width image

		sprintf(buf,"pics/loadscreens/%s", RandBkgFilename(console_screen_idx->value + 1)); // random generic screen

		drawStruct.pImage = R_GetPicData( buf, &image_width, &image_height, RESOURCE_INTERFACE );

		scale_x = ( float ) w / 320.0;	// 320 is the total width of pic1 and pic2.
		scale_y = ( float ) h / ( float ) image_height ;

		drawStruct.nFlags = DSFLAG_SCALE | DSFLAG_PALETTE;

		drawStruct.fScaleX = scale_x;
		drawStruct.fScaleY = scale_y;
		drawStruct.nXPos = x;
		drawStruct.nYPos = y;
		R_DrawPic( drawStruct );

		// draw 256 width image

		sprintf(buf,"pics/loadscreens/%s", RandBkgFilename(console_screen_idx->value)); // random generic screen

		drawStruct.pImage = R_GetPicData( buf, NULL, NULL, RESOURCE_INTERFACE );

		drawStruct.nXPos = x + image_width * scale_x;
		R_DrawPic( drawStruct );

		// draw three strips of black down each side to account for the blend wrapping
		CVector rgbColor;
		rgbColor.x = rgbColor.y = rgbColor.z = 0;
		Draw_Fill( 0, y, 3, h, rgbColor, 1.0 );
		Draw_Fill( w - 3, y, 3, h, rgbColor, 1.0 );
	}
	else
	{
		// load screen based on map
		scale_x = (vid.width / 640.0);
		scale_y = (vid.height / 480.0);
		width_256 = (int)(256.0 * scale_x);
		width_128 = (int)(128.0 * scale_x);
		pic1_x = 0;
		pic2_x = width_256;
		pic3_x = pic2_x + width_256;
		height = (int)(224.0 * scale_y);
//		height = (int)(256.0 * scale_y);	// SCG[8/26/99]: Test

		drawStruct.nFlags = DSFLAG_SCALE | DSFLAG_PALETTE;
		drawStruct.fScaleX = scale_x;
		drawStruct.fScaleY = scale_y;

		drawStruct.nYPos = 0;

		int index = 0;

		for( int i = 0; i < 2; i++ )
		{
			drawStruct.nXPos = 0;    // reset
			for( int j = 0; j < 3; j++ )
			{
				sprintf( buf, "pics/loadscreens/e%dm%d_%d.bmp", episode_num, map_num, index++ ); 
//				sprintf( buf, "pics/interface/back%d%d.bmp", i, j );	// SCG[8/26/99]: Test

				drawStruct.pImage = R_GetPicData( buf, NULL, NULL, RESOURCE_INTERFACE);
				re.DrawPic( drawStruct );
				drawStruct.nXPos += width_256; // slide over
			}

			drawStruct.nYPos = height;  // next row
		}
	}
}
*/
