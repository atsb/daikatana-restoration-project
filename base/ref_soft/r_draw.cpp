
// draw.c
#include "r_local.h"

#include "dk_misc.h"
#include "dk_array.h"
#include "dk_buffer.h"

#include "dk_ref_common.h"
#include "dk_ref_font.h"


image_t		*draw_chars;				// 8*8 graphic characters
void *char_flare_image;
void *default_chars_image;

//=============================================================================

/*
=============
R_GetPicData
=============
*/
void *R_GetPicData( const char *name, int *pWidth, int *pHeight, resource_t resource )
{
	// the image
	image_t	*image;

	image = R_FindImage( name, it_pic, resource );

	if( image == NULL )
	{
		image = r_notexture_mip;
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
	char	dk_path [1024];
	
	//	Nelno:	load Daikatana console characters
	//	default_chars_image = draw_chars = Draw_FindPic ("conchars");
	if( !r_resourcedir || !r_resourcedir->string || r_resourcedir->string [0] == 0x00 )
	{
		default_chars_image = draw_chars = (  image_t * ) R_GetPicData( "dkchars", NULL, NULL, RESOURCE_EPISODE );
	}
	else
	{
		sprintf (dk_path, "%sdkchars.pcx", r_resourcedir->string);

		default_chars_image = draw_chars = ( image_t * ) R_GetPicData( dk_path, NULL, NULL, RESOURCE_EPISODE );

        if( !draw_chars ) 
		{
			default_chars_image = draw_chars = ( image_t * ) R_GetPicData( "dkchars", NULL, NULL, RESOURCE_EPISODE );
        }
	}
}



void DrawPicPortion(int destx, int desty, int srcleft, int srctop, int source_width, int source_height, void *img) {
    image_t *image;
    int srcright;
    int srcbottom;
    int bmp_scan_top;
    int dest_scan_top;
    int bmp_scan_bottom;
    int dest_scan_bottom;
    int bmp_scan_left;
    int dest_scan_left;
    int bmp_scan_right;
    int dest_scan_right;
    int bitmap_width;
    int bitmap_height;

    if (img == NULL) return;
    image = (image_t *)img;

    if (source_width < 1) return;
    if (source_height < 1) return;

    srcright = srcleft + source_width;
    srcbottom = srctop + source_height;

    //get the bitmap width and height.
    bitmap_width = image->width;
    bitmap_height = image->height;

    if (srcleft < 0) return;
    if (srctop < 0) return;
    if (srcright > bitmap_width) return;
    if (srcbottom > bitmap_height) return;

    //determine if the bitmap is on the screen.
    if (destx >= vid.width) return;               //off the right.
    if (desty >= vid.height) return;              //off the bottom.
    if (destx + source_width <= 0) return;                  //off the left.
    if (desty + source_height <= 0) return;                 //off the top.

    //get the top scanline on the bitmap that we will draw, and where we will draw it.
    if (desty < 0) {
        //the top of the source window is off the top of the screen.
        //we start on the first line that is visible on the screen.
        bmp_scan_top = -desty + srctop;
        dest_scan_top = 0;
    }
    else {
        //the top of the source window is on the screen.
        bmp_scan_top = srctop;
        dest_scan_top = desty;
    }

    //get the ending scanline on the bitmap that we will draw, and where we will draw it.
    if (vid.height - desty < source_height) {
        //the bottom of the source window is off the bottom of the screen.
        //(vid.height - desty) is the number of scanlines that we will draw
        bmp_scan_bottom = srctop + (vid.height - desty) - 1;
        dest_scan_bottom = vid.height - 1;
    }
    else {
        //the bottom of the source window is on the screen.
        bmp_scan_bottom = srcbottom - 1;
        dest_scan_bottom = desty + source_height - 1;
    }

    //get the starting position to draw on each bitmap scanline.
    if (destx < 0) {
        //the left of the source window is off the left of the screen.
        //we start on the first pixel that is on the screen.
        bmp_scan_left = -destx + srcleft;
        dest_scan_left = 0;
    }
    else {
        //the left of the source window is on the screen.
        bmp_scan_left = srcleft;
        dest_scan_left = destx;
    }

    //get the ending position to draw on each bitmap scanline.
    if (vid.width - destx < source_width) {
        //the right of the source window is off the right of the screen.
        //(vid.width - destx) is the number of scanlines that we will draw.
        bmp_scan_right = srcleft + (vid.width - destx) - 1;
        dest_scan_right = vid.width - 1;
    }
    else {
        //the right of the source window is on the screen.
        bmp_scan_right = srcright - 1;
        dest_scan_right = destx + source_width - 1;
    }

    //error check.
    if (bmp_scan_right - bmp_scan_left != dest_scan_right - dest_scan_left) return;
    if (bmp_scan_bottom - bmp_scan_top != dest_scan_bottom - dest_scan_top) return;

    //do the actual copy.  
    {
        int cur_src_scan = bmp_scan_top;
        int cur_dest_scan = dest_scan_top;

        //a pointer to the beginning of the source scanline.
      const byte *src = image->pixels[0] + bitmap_width * cur_src_scan;
//      const uint16 *src = image->pixels[0] + bitmap_width * cur_src_scan;

        for (;cur_dest_scan <= dest_scan_bottom;) {
            //draw the current scan.
            int cur_src_pixel = bmp_scan_left;
            int cur_dest_pixel = dest_scan_left;
//          byte *dest = vid.buffer + cur_dest_scan * vid.rowbytes;
//           byte pixel;
            uint16 *dest = (uint16 *) vid.buffer + cur_dest_scan * vid.pixelwidth;
            uint16 pixel;

            for (;cur_dest_pixel <= dest_scan_right;) {
                //get the pixel.
                pixel = src[cur_src_pixel];
                //check if the pixel is not transparent.
                if (pixel != 255) {
                    //draw the pixel.
                    dest[cur_dest_pixel] = image->palette[pixel];
                }

                //increment our current pixel.
                cur_src_pixel++;
                cur_dest_pixel++;
            }

            //increment to the next scan.
            cur_src_scan++;
            cur_dest_scan++;

            //get the new source pointer.
            src += bitmap_width;
        }
    }
}

void R_DrawChar (int x, int y, int num)
{
	int				row, col;

	num &= 255;

	if (num == 32 || num == 32+128)
		return;

    //compute the left and right pixel of the portion of the character pic we will draw.
    row = (num >> 4) * 8;
    col = (num & 15) * 8;

    //draw the character.
//    DrawPicPortion(x, y, col, row, 8, 8, draw_chars);
}

///////////////////////////////////////////////////////////////////////////////
//	Draw_CharFlare
//	
//	Draws one 8*8 graphics character with an alpha channel and optional effects
///////////////////////////////////////////////////////////////////////////////

extern	image_t	*charFlare;
extern	float	ref_laserRotation;

#define	FLARE_OFFSET	5
#define	XINDEX	0
#define	YINDEX	1

extern polydesc_t r_polydesc;
void Draw_CharFlare (int x, int y, int w, int h, void *image, CVector &rgbBlend, float alpha, float scale, float theta)
{
	float	centerX, centerY;
	float	halfW, halfH;
	float	addX, addY;
	float	sinTheta, cosTheta;
	float	xy [4][2];

	halfW = w / 2.0;
	halfH = h / 2.0;
	centerX = x + halfW;
	centerY = y + halfH;

	addX = (halfW + FLARE_OFFSET) * scale;
	addY = (halfH + FLARE_OFFSET) * scale;

	if (theta != 0.0)
	{
		//	rotate the four points by theta radians
		sinTheta = sin (theta);
		cosTheta = cos (theta);

		xy [0][XINDEX] = (cosTheta * -addX) - (sinTheta * addY) + centerX;
		xy [0][YINDEX] = (sinTheta * -addX) + (cosTheta * addY) + centerY;

		xy [1][XINDEX] = (cosTheta * addX) - (sinTheta * addY) + centerX;
		xy [1][YINDEX] = (sinTheta * addX) + (cosTheta * addY) + centerY;

		xy [2][XINDEX] = (cosTheta * addX) - (sinTheta * -addY) + centerX;
		xy [2][YINDEX] = (sinTheta * addX) + (cosTheta * -addY) + centerY;

		xy [3][XINDEX] = (cosTheta * -addX) - (sinTheta * -addY) + centerX;
		xy [3][YINDEX] = (sinTheta * -addX) + (cosTheta * -addY) + centerY;
	}
	else
	{
		xy [0][XINDEX] = centerX - addX;
		xy [0][YINDEX] = centerY + addY;

		xy [1][XINDEX] = centerX + addX;
		xy [1][YINDEX] = centerY + addY;

		xy [2][XINDEX] = centerX + addX;
		xy [2][YINDEX] = centerY - addY;

		xy [3][XINDEX] = centerX - addX;
		xy [3][YINDEX] = centerY - addY;
	}

//	R_DrawPic( xy[0][XINDEX], xy[0][YINDEX], "pics/misc/charflare.bmp", RESOURCE_GLOBAL );
}



/*
=============
Draw_StretchRaw
=============
*/
void Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows, byte *data)
{
	image_t	pic;

	pic.pixels[0] = data;
	pic.width = cols;
	pic.height = rows;
	
	float	scale_x = ( float ) w / ( float ) cols;
	float	scale_y = ( float ) h / ( float ) rows;

	DRAWSTRUCT	drawStruct;
	drawStruct.nFlags = DSFLAG_SCALE;
	drawStruct.nXPos = x;
	drawStruct.nYPos = y;
	drawStruct.fScaleX = scale_x;
	drawStruct.fScaleY = scale_y;
	drawStruct.pImage = &pic;

	R_DrawPic( drawStruct );
//	Draw_StretchPicImplementation (x, y, w, h, &pic);
}

/*
=============
R_DrawString
=============
*/

int R_DrawStringFlare( DRAWSTRUCT& drawStruct )
{
	return drawStruct.nXPos;
}

/*
=============
R_DrawString
=============
*/
int R_DrawString( DRAWSTRUCT& drawStruct )
{
	float fScaleX, fScaleY;

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

	drawStruct.nFlags |= DSFLAG_SUBIMAGE;

	// SCG[3/23/99]: Get the string data
	unsigned char *pString = ( unsigned char * ) drawStruct.szString;

	if( drawStruct.pImage == NULL )
	{
		return drawStruct.nXPos;
	}

	// SCG[3/23/99]: Get the font data
	dk_font *font = ( dk_font * ) drawStruct.pImage;

	drawStruct.pImage = font->image;

	float x_frac = drawStruct.nXPos;
	
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

		drawStruct.nXPos = ( int ) x_frac;
		drawStruct.nLeft = font->char_pos_x[ c ];
		drawStruct.nTop = font->char_pos_y[ c ];
		drawStruct.nRight = drawStruct.nLeft + font->char_width[ c ];
		drawStruct.nBottom = drawStruct.nTop + font->height;

		DrawPicPortion( 
			drawStruct.nXPos, 
			drawStruct.nYPos, 
			drawStruct.nLeft, 
			drawStruct.nTop, 
			font->char_width[ c ], 
			font->height, 
			font->image );

		x_frac += ( 1 + font->char_width[ c ] ) * fScaleX;
	}

	return x_frac;
}

void R_DrawPic16( DRAWSTRUCT &drawStruct );
void R_DrawPicAlpha16( DRAWSTRUCT &drawStruct );
/*
=============
R_DrawPic
=============
*/
void R_DrawPic ( DRAWSTRUCT &drawStruct )
{
	int		nSrcScanX1, nSrcScanY1, nSrcScanX2, nSrcScanY2;
	int		nSrcScanWidth, nSrcScanHeight;
	int		nDstScanX1, nDstScanY1, nDstScanX2, nDstScanY2;
	int		nDstScanWidth, nDstScanHeight;
	int		nSrcFrac, nSrcFracStep, nSrcV;
	int		nSrcPixel;
	int		nDstPixel;
	float	fScaleX, fScaleY;

    // SCG[3/23/99]: Get a pointer to the image data
	image_t *image = ( image_t * ) drawStruct.pImage;

	// SCG[3/23/99]: Make sure the image pointer is valid
    if( image == NULL )
	{
		return;
	}

	// SCG[3/23/99]: Check for scale
	if( drawStruct.nFlags & DSFLAG_SCALE )
	{
		fScaleX = drawStruct.fScaleX;
		fScaleY = drawStruct.fScaleY;
	}

	// SCG[3/23/99]: Set destination drawing parameters
	nDstScanX1 = drawStruct.nXPos;
	nDstScanY1 = drawStruct.nYPos;

	nDstScanWidth = vid.width;
	nDstScanHeight = vid.height;

	// SCG[3/23/99]: Set source drawing parameters
	if( drawStruct.nFlags & DSFLAG_SUBIMAGE )
	{
		// SCG[3/23/99]: Draw a portion of the image
		nSrcScanX1 = drawStruct.nLeft;
		nSrcScanY1 = drawStruct.nTop;
		nSrcScanX2 = nSrcScanX1 + ( drawStruct.nRight - drawStruct.nLeft ) * fScaleX;
	    nSrcScanY2 = nSrcScanY1 + ( drawStruct.nBottom - drawStruct.nTop ) * fScaleY;

		nDstScanX2 = nDstScanX1 + ( drawStruct.nRight - drawStruct.nLeft ) * fScaleX;
		nDstScanY2 = nDstScanY1 + ( drawStruct.nBottom - drawStruct.nTop ) * fScaleY;

	}
	else
	{
		// SCG[3/23/99]: Draw entire image
		nSrcScanX1 = 0;
		nSrcScanY1 = 0;
		nSrcScanX2 = nSrcScanX1 + image->width * fScaleX;
	    nSrcScanY2 = nSrcScanY1 + image->height * fScaleY;

		nDstScanX2 = nDstScanX1 + image->width * fScaleX;
		nDstScanY2 = nDstScanY1 + image->height * fScaleY;
	}

	// SCG[3/23/99]: Set the scanline width and height
	nSrcScanWidth = image->width;
	nSrcScanHeight = image->height;

	if( nDstScanX2 > nDstScanWidth )
	{
		nDstScanX2 = nDstScanWidth;
	}
	if( nDstScanY2 > nDstScanHeight )
	{
		nDstScanY2 = nDstScanHeight;
	}

	// SCG[3/23/99]: Check for image scaling
	if( ( drawStruct.fScaleX == 1 ) && ( drawStruct.fScaleY == 1 ) )
	{
		byte *pSrc = image->pixels[0] + nSrcScanY1 * image->width;
		unsigned short *pDst = ( unsigned short * ) vid.buffer + ( nDstScanY1 * vid.pixelwidth );
		if( image->transparent == 0 )
		{
			// SCG[3/23/99]: Draw image without transparency
			for( int nRow = nDstScanY1; nRow < nDstScanY2; nRow++ )
			{
				nDstPixel = nDstScanX1;
				nSrcPixel = nSrcScanX1;
				for( int nCol = nSrcScanX1; nCol < nSrcScanX2; nCol++ )
				{
					pDst[ nDstPixel++ ] = image->palette[ pSrc[ nSrcPixel++ ] ];
				}
				pSrc += nSrcScanWidth;
				pDst += vid.pixelwidth;
			}
		}
		else
		{
			// SCG[3/23/99]: Draw image with transparency
			for( int nRow = nDstScanY1; nRow < nDstScanY2; nRow++ )
			{
				nDstPixel = nDstScanX1;
				nSrcPixel = nSrcScanX1;
				for( int nCol = nSrcScanX1; nCol < nSrcScanX2; nCol++ )
				{
					if( pSrc[ nSrcPixel ] != 255 )
					{
						pDst[ nDstPixel++ ] = image->palette[ pSrc[ nSrcPixel++ ] ];
					}
				}
				pSrc += nSrcScanWidth;
				pDst += vid.pixelwidth;
			}
		}
	}
	// SCG[3/23/99]: Scaled image
	else
	{
		unsigned short *pDst = ( unsigned short * ) vid.buffer + ( nDstScanY1 * vid.pixelwidth );
		if( image->transparent == 0 )
		{
			// SCG[3/23/99]: Draw image without transparency
			for( int nRow = nDstScanY1; nRow < nDstScanY2; nRow++ )
			{
				nSrcV = nRow * image->height / nSrcScanY2;

				byte *pSrc = image->pixels[0] + nSrcV * image->width;

				nSrcFrac = 0;
				nSrcFracStep = image->width * 0x10000 / nSrcScanX2;

				nDstPixel = nDstScanX1;
				nSrcPixel = nSrcScanX1;
				for( int nCol = nSrcScanX1; nCol < nSrcScanX2; nCol++ )
				{
					pDst[ nDstPixel++ ] = image->palette[ pSrc[ nSrcFrac >> 16 ] ];
					nSrcFrac += nSrcFracStep;
				}
				pDst += vid.pixelwidth;
			}
		}
		else
		{
			// SCG[3/23/99]: Draw image with transparency
			for( int nRow = nDstScanY1; nRow < nDstScanY2; nRow++ )
			{
				nSrcV = nRow * image->height / nSrcScanY2;

				byte *pSrc = image->pixels[0] + nSrcV * image->width;

				nSrcFrac = 0;
				nSrcFracStep = image->width * 0x10000 / nSrcScanX2;

				nDstPixel = nDstScanX1;
				nSrcPixel = nSrcScanX1;
				for( int nCol = nSrcScanX1; nCol < nSrcScanX2; nCol++ )
				{
					if( pSrc[ nSrcFrac >> 16 ] != 255 )
					{
						pDst[ nDstPixel++ ] = image->palette[ pSrc[ nSrcFrac >> 16 ] ];
					}
					nSrcFrac += nSrcFracStep;
				}
				pDst += vid.pixelwidth;
			}
		}
	}
}

/*
=============
DrawTileClear

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void DrawTileClear (int x, int y, int w, int h, const char *name)
{
	int			i, j;
	byte		*psrc;
//	byte		*pdest;
	uint16		*pdest;
	image_t		*pic;
	int			x2;

	if (x < 0)
	{
		w += x;
		x = 0;
	}
	if (y < 0)
	{
		h += y;
		y = 0;
	}
	if (x + w > vid.width)
		w = vid.width - x;
	if (y + h > vid.height)
		h = vid.height - y;
	if (w <= 0 || h <= 0)
		return;

	pic = ( image_t *) R_GetPicData( name, NULL, NULL, RESOURCE_LEVEL );

	if (!pic)
	{
		ri.Con_Printf (PRINT_ALL, "Can't find pic: %s\n", name);
		return;
	}
	x2 = x + w;
	pdest = (uint16 *) vid.buffer + y*vid.pixelwidth;
	for (i=0 ; i<h ; i++, pdest += vid.pixelwidth)
	{
		psrc = pic->pixels[0] + pic->width * ((i+y)&63);
		for (j=x ; j<x2 ; j++)
			pdest[j] = psrc[j&63];
	}
}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_Fill( int x, int y, int w, int h, CVector rgbColor, float alpha )
{
	uint16			*dest;
	int				u, v, c;
	
	// FIXME: make this a real 16 bit color
	c = ( rgbColor.x + rgbColor.y + rgbColor.z ) / 3;

	if (x+w > vid.width)
		w = vid.width - x;
	if (y+h > vid.height)
		h = vid.height - y;
	if (x < 0)
	{
		w += x;
		x = 0;
	}
	if (y < 0)
	{
		h += y;
		y = 0;
	}
	if (w < 0 || h < 0)
		return;
	dest = (uint16 *) vid.buffer + y*vid.pixelwidth + x;
	for (v=0 ; v<h ; v++, dest += vid.pixelwidth)
		for (u=0 ; u<w ; u++)
			dest[u] = c;
}

//=============================================================================

/*
================
Draw_FadeScreen

================
*/
void Draw_FadeScreen (void)
{
	int			x,y;
//	byte		*pbuf;
	uint16		*pbuf;
//	int			t;
//	byte		pix;
	uint16		pix;

	//	Q2FIXME:	needs an int * version for 16 bit modes
	for (y=0 ; y<vid.height ; y++)
	{
//		pbuf = (byte *)(vid.buffer + vid.pixelwidth*y);
		pbuf = (uint16 *)(vid.buffer + vid.pixelwidth*y);

		for (x=0 ; x<vid.width ; x++)
		{
			pix = pbuf [x];

			pbuf [x] = vid.alphamap [pix];
		}
	}

}



///////////////////////////////////////////////////////////////////////////////
//	DrawConsolePic
//
//	finds the correct console pic based on r_resourceDir
///////////////////////////////////////////////////////////////////////////////

void DrawConsolePic (int x, int y, int w, int h)
{
//==================================
// SCG[3/23/99]: Consolidation change

	DRAWSTRUCT drawStruct;
	int image_width, image_height;
	drawStruct.pImage = R_GetPicData( RandBkgFilename(console_screen_idx->value + 1), &image_width, &image_height, RESOURCE_INTERFACE );

	float	scale_x = ( float ) w / 320.0;	// 320 is the total width of pic1 and pic2.
	float	scale_y = ( float ) h / ( float ) image_height ;

	drawStruct.nFlags = DSFLAG_SCALE | DSFLAG_PALETTE;
	drawStruct.fScaleX = scale_x;
	drawStruct.fScaleY = scale_y;
	drawStruct.nXPos = x;
	drawStruct.nYPos = y;
	R_DrawPic( drawStruct );

	drawStruct.pImage = R_GetPicData( RandBkgFilename(console_screen_idx->value), NULL, NULL, RESOURCE_INTERFACE );
	drawStruct.nXPos = x + image_width * scale_x;
	R_DrawPic( drawStruct );

// SCG[3/23/99]: Consolidation change
//==================================
}
