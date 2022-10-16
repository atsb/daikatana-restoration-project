#include "r_local.h"


// ----------------------------------------------------------------------------
//
// Name:		
// Description:
// Input:		
// Output:		
// Note:		
//
// ---------------------------------------------------------------------------
void R_DrawPic8( DRAWSTRUCT &ds )
{
	image_t *pImage = ( image_t * ) ds.pImage;

	if( pImage == NULL )
	{
		pImage = r_notexture_mip;
	}

	int nPolyWidth = pImage->width;				// polygon width
	int nPolyHeight = pImage->height;			// polygon height
	int nPolyX1 = ds.nXPos;						// upper left of polygon
	int nPolyY1 = ds.nYPos;
	int nPolyX2 = ds.nXPos + nPolyWidth;		// lower right of polygon
	int nPolyY2 = ds.nYPos + nPolyHeight;		

	// validate poly boundaries
	if( nPolyX1 > vid.width - 1 )
	{
		return;
	}
	if( nPolyY1 > vid.height - 1 )
	{
		return;
	}
	if( nPolyX2 > vid.width - 1 )
	{
		nPolyX2 = vid.width - 1;
	}
	if( nPolyY2 > vid.height - 1 )
	{
		nPolyY2 = vid.height - 1;
	}
	
	int	nPixelOffset = ( ( nPolyY1 * vid.pixelwidth ) + nPolyX1 );

	byte *pScreen = vid.buffer + nPixelOffset;
	byte *pSource = pImage->pixels[0];

	for( int nRowCount = 0; nRowCount < nPolyHeight; nRowCount++ )
	{
		for( int nColCount = 0; nColCount < nPolyWidth; nColCount++ )
		{
			*pScreen++ = *pSource++;
		}
	}
}
