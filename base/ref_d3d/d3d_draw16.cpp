#include "d3d_local.h"



//-----------------------------------------------------------------------------
// Name: DrawStretchPic16
// Desc: 16 bit version of DrawStretchPic
//-----------------------------------------------------------------------------
void DrawStretchPic16( const CPic *bitmap, int destx, int desty, int dest_width, int dest_height )
{
	// make sure the surface pointer is valid
	if( global_lock == NULL )
		return;

	// make sure the imade data is valid
	if( bitmap->Data() == NULL )
		return;

	// check for negative coordinates
	if( desty < 0 || destx < 0 )
		return;

	// get a pointer to the image palette
	const CPalette *palette	= bitmap->Palette();

	// get number of bytes per pixel
	byte	bytes_per_pixel	= d3d_core->BitsPerPixel() / 8;

	// get the image width
	uint16	bitmap_width	= bitmap->Width();

	// get the image height
	uint16	bitmap_height	= bitmap->Height();

	// get the draw width
	uint16	draw_width		= dest_width;

	// get the draw height
	uint16	draw_height		= dest_height;

	// if the draw width is greater than the display width, chop the image	
	if( destx + draw_width > d3d_core->Width() )
		draw_width = d3d_core->Width() - destx;

	// if the draw height is greater than the display height, chop the image	
	if( desty + draw_height > d3d_core->Height() )
		draw_height = d3d_core->Height() - desty;

	// get the display pitch (distance in bytes from row(x) to row(x+1))
	DWORD	display_pitch	= d3d_core->Pitch();

	// get a pointer to start of destination pixels
	uint16		*dst_pixels	= (uint16 *) global_lock + (desty * display_pitch) + destx;

	// get a pointer to start of source pixels
	const byte	*src_pixels	= bitmap->Data();


	// carmack stuff
	int	sv, skip;
	if (desty < 0)
	{
		skip = -desty;
		draw_height += desty;
		desty = 0;
	}
	else
		skip = 0;

	// do some drawing
	for( int row = 0; row < draw_height; row++, dst_pixels += display_pitch )
	{
		sv = (skip + row) * bitmap_height / draw_height;
		src_pixels = bitmap->Data() + sv * bitmap_width;
		
		if( bitmap_width == draw_width )
		{
			for( int col = 0; col < draw_width; col++ )
			{
				dst_pixels[col] = palette->colors[src_pixels[col]].color16;
			}
		}
		else
		{
			int	f = 0;
			int fstep = bitmap_width * 0x10000 / draw_width;
			for( int u = 0; u < draw_width; u += 4 )
			{
				dst_pixels[u]	= palette->colors[src_pixels[f>>16]].color16;
				f += fstep;
				dst_pixels[u+1]	= palette->colors[src_pixels[f>>16]].color16;
				f += fstep;
				dst_pixels[u+2]	= palette->colors[src_pixels[f>>16]].color16;
				f += fstep;
				dst_pixels[u+3]	= palette->colors[src_pixels[f>>16]].color16;
				f += fstep;
			}
		}
	}
}



//-----------------------------------------------------------------------------
// Name: DrawPic16
// Desc: 16 bit version of d3d_DrawPic
//-----------------------------------------------------------------------------
void DrawPic16(const CPic *bitmap, int destx, int desty, RECT *sourceRect )
{
	// make sure the surface pointer is valid
	if( global_lock == NULL )
		return;

	// make sure the imade data is valid
	if( bitmap->Data() == NULL )
		return;

	// check for negative coordinates
	if( desty < 0 || destx < 0 )
		return;

	// get a pointer to the image palette
	const CPalette *palette	= bitmap->Palette();

	// get the image width
	uint16	bitmap_width	= bitmap->Width();

	// get the image height
	uint16	bitmap_height	= bitmap->Height();

	// get the draw width
	uint16	draw_width		= sourceRect->right - sourceRect->left;

	// get the draw height
	uint16	draw_height		= sourceRect->bottom - sourceRect->top;

	// if the image width is greater than the display width, chop the image	
	if( sourceRect->right > d3d_core->Width() )
		draw_width = d3d_core->Width() - sourceRect->left;

	// if the image height is greater than the display height, chop the image	
	if( sourceRect->bottom > d3d_core->Height() )
		draw_height = d3d_core->Height() - sourceRect->top;

	// get the display pitch (distance in bytes from row(x) to row(x+1))
	DWORD	display_pitch	= d3d_core->Pitch();

	// get a pointer to start of destination pixels
	uint16	*dst_pixels		= (uint16 *) global_lock + (desty * display_pitch) + destx;

	// get a pointer to start of source pixels
	const byte	*src_pixels	= bitmap->Data() + (sourceRect->top * bitmap_width) + sourceRect->left;

	// do some drawing
	for( int row = 0; row < draw_height; row++, dst_pixels += display_pitch, src_pixels += bitmap_width)
	{
		for( int col = 0; col < draw_width; col++ )
		{
			if( src_pixels[col] == 255 )
				continue;
			dst_pixels[col] = palette->colors[src_pixels[col]].color16;
		}
	}
}
