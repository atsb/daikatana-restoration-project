
#include "r_local.h"
#include "r_protex.h"

#include "dk_ref_common.h"

image_t		r_images[MAX_RIMAGES];
int			numr_images;

int			g_loaded_old_wal;
byte		*current_palette;

/*
===============
R_BuildLightTable
===============
*/
void R_BuildLightTable( const byte pal[256], uint16 *destpal){
	int		i, j, index, levels;
	float	frac, red, green, blue;
	float	range;
	uint16	r, g, b;
	
	range = 1.5;
	levels = NUMLIGHTLEVELS;

	if( destpal == NULL )
		return;

	for (i = 0, index = 0; i < levels; i++){

		frac = range - range * (float)i / (levels - 1);

		for (j = 0 ;j < 256; j++){
			red   = pal[j * 3 + 0];
			green = pal[j * 3 + 1];
			blue  = pal[j * 3 + 2];

			r = (int)(red * frac + 0.5);
			g = (int)(green * frac + 0.5);
			b = (int)(blue * frac + 0.5);

			r = (int)r >> sw_state.rgb16.remove.rgbRed;
			g = (int)g >> sw_state.rgb16.remove.rgbGreen;
			b = (int)b >> sw_state.rgb16.remove.rgbBlue;

			destpal[index++] =
				( r << sw_state.rgb16.position.rgbRed ) |
				( g << sw_state.rgb16.position.rgbGreen ) |
				( b << sw_state.rgb16.position.rgbBlue );
		}
	}
}

/*
===============
R_Build8to16Table
===============
*/
void R_Build8to16Table( const unsigned char *pal ){
	int		i;
	float  	r, g, b;

	if( pal == NULL )
	{
		byte *out = (byte *)d_8to24table;
		for ( i = 0; i < 256; i++, out+=4 )
		{
			r = (int)out[0] >> sw_state.rgb16.remove.rgbRed;
			g = (int)out[1] >> sw_state.rgb16.remove.rgbGreen;
			b = (int)out[2] >> sw_state.rgb16.remove.rgbBlue;

			d_8to16table[i] =
				( (int)r << sw_state.rgb16.position.rgbRed ) |
				( (int)g << sw_state.rgb16.position.rgbGreen ) |
				( (int)b << sw_state.rgb16.position.rgbBlue );
		}
	}
	else
	{
		for ( i = 0; i < 256; i++ )
		{
			r = (int)pal[i * 3 + 0] >> sw_state.rgb16.remove.rgbRed;
			g = (int)pal[i * 3 + 1] >> sw_state.rgb16.remove.rgbGreen;
			b = (int)pal[i * 3 + 2] >> sw_state.rgb16.remove.rgbBlue;

			d_8to16table[i] =
				( (int)r << sw_state.rgb16.position.rgbRed ) |
				( (int)g << sw_state.rgb16.position.rgbGreen ) |
				( (int)b << sw_state.rgb16.position.rgbBlue );

		}
	}
}

void R_BuildFogTable(){
	int	i, r, g, b;
	float	frac, range;


	range = 1.5;

	for ( i = 0; i < 256; i++ )
	{
		frac = range - range * (float)i / 255;

		r = (int)(128 * frac + 0.5) >> sw_state.rgb16.remove.rgbRed;
		g = (int)(128 * frac + 0.5) >> sw_state.rgb16.remove.rgbGreen;
		b = (int)(128 * frac + 0.5) >> sw_state.rgb16.remove.rgbBlue;

		d_fogtable[i] =
			( (int)r << sw_state.rgb16.position.rgbRed ) |
			( (int)g << sw_state.rgb16.position.rgbGreen ) |
			( (int)b << sw_state.rgb16.position.rgbBlue );
	}
}

/*
===============
R_ImageList_f
===============
*/
void	R_ImageList_f (void)
{
	int		i;
	image_t	*image;
	int		texels;

	ri.Con_Printf (PRINT_ALL, "------------------\n");
	texels = 0;

	for (i=0, image=r_images ; i<numr_images ; i++, image++)
	{
		if (image->registration_sequence <= 0)
			continue;
		texels += image->width*image->height;
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

		ri.Con_Printf (PRINT_ALL,  " %3i %3i : %s\n",
			image->width, image->height, image->name);
	}
	ri.Con_Printf (PRINT_ALL, "Total texel count: %i\n", texels);
}

#if 0
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

	targa_header.id_length = *buf_p++;
	targa_header.colormap_type = *buf_p++;
	targa_header.image_type = *buf_p++;
	
	targa_header.colormap_index = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.colormap_length = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.colormap_size = *buf_p++;
	targa_header.x_origin = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.y_origin = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.width = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.height = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.pixel_size = *buf_p++;
	targa_header.attributes = *buf_p++;

	if (targa_header.image_type!=2 
		&& targa_header.image_type!=10) 
		ri.Sys_Error (ERR_DROP, "LoadTGA: Only type 2 and 10 targa RGB images supported\n");

	if (targa_header.colormap_type !=0 
		|| (targa_header.pixel_size!=32 && targa_header.pixel_size!=24))
		ri.Sys_Error (ERR_DROP, "LoadTGA: Only 32 or 24 bit images supported (no colormaps)\n");

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if (width)
		*width = columns;
	if (height)
		*height = rows;

	targa_rgba = malloc (numPixels*4);
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

#endif

//=======================================================

image_t *R_FindFreeImage (void)
{
	image_t		*image;
	int			i;

	// find a free image_t
	for (i=0, image=r_images ; i<numr_images ; i++,image++)
	{
		if (!image->registration_sequence)
			break;
	}
	if (i == numr_images)
	{
		if (numr_images == MAX_RIMAGES)
			ri.Sys_Error (ERR_DROP, "MAX_RIMAGES");
		numr_images++;
	}
	image = &r_images[i];

	return image;
}

/*
================
GL_LoadPic

================
*/
void *GL_LoadPic (const char *name, byte *pic, int width, int height, imagetype_t type)
{
	image_t		*image;
	int			i, c, b;
	uint16		*dest;

	image = R_FindFreeImage ();
	if (strlen(name) >= sizeof(image->name))
		ri.Sys_Error (ERR_DROP, "Draw_LoadPic: \"%s\" is too long", name);
	strcpy (image->name, name);
	image->registration_sequence = registration_sequence;

	image->width = width;
	image->height = height;
	image->type = type;

	c = width*height;
	R_Build8to16Table( current_palette );

	if( type == it_sky || type == it_sprite ){
		image->pixels[0] = (byte*)malloc( c * BYTES_PER_PIXEL );
		dest = (uint16*)image->pixels[0];

		image->transparent = false;
		// The cacheblock must point to 16 bit data.
		// Since skies/sprites have no surface cache, the cacheblock 
		// points directly to the image.
		for ( i = 0; i < c; i++ ){
			if( pic[i] == 255 ){
				image->transparent = true;
				dest[i] = TRANSPARENT_COLOR;
			}
			else{
				dest[i] = d_8to16table[pic[i]];
			}
		}
	}
	else{
		image->pixels[0] = (byte*)malloc( c );

		image->transparent = false;
		image->palette = (uint16*) malloc( 256 * sizeof(uint16) );
		for ( i = 0; i < c; i++ ){
			b = pic[i];
			if( b == 255 ){
				image->transparent = true;
				image->pixels[0][i] = (byte) TRANSPARENT_COLOR;
			}
			else{
				image->pixels[0][i] = (byte) b;
			}
		}

		memcpy( image->palette, d_8to16table, 256 * sizeof(uint16) );
	}


	return image;
}

/*
================
R_LoadWal
================
*/
image_t *R_LoadWal (const char *name, imagetype_t type)
{
	miptex_t	*mt;
	int			ofs;
	image_t		*image;
	int			size;
	char		fname [_MAX_FNAME];
	int			filelen;

	filelen = ri.FS_LoadFile (name, (void **)&mt);
	if (!mt)
	{
		ri.Con_Printf (PRINT_ALL, "R_LoadWal: can't load %s\n", name);
		return r_notexture_mip;
	}

	//	Nelno:	determine if this is an old WAL type (4 mip levels)
	//	or a newer WAL type that contains a version number
	if (mt->version > 0x20){
//		return	R_LoadWalOld (name, mt, type);
		return NULL;
	}

	if (mt->version != MIPTEX_VERSION)
		ri.Sys_Error (ERR_DROP, "%s has wrong version number (%i should be %i)", name, mt->version, MIPTEX_VERSION);

	_splitpath (name, NULL, NULL, fname, NULL);
	if (strstr (fname, "pt_") == fname)
	{
		//	this is a procedural texture, just get the width, height and name
		//	then init the correct procedural texture for it
	
		image = protex_InitProceduralFromWAL (name, NULL, LittleLong (mt->width), LittleLong (mt->height));
		ri.FS_FreeFile ((void *)mt);
		return image;
	}


	image = R_FindFreeImage ();
	strcpy (image->name, name);
	image->width = LittleLong (mt->width);
	image->height = LittleLong (mt->height);
	image->type = type;
	image->registration_sequence = registration_sequence;

	size = filelen - sizeof(miptex_t)+1;
	ofs = LittleLong (mt->offsets[0]);

	if( type == it_skin ){
		image->palette = (uint16*) malloc( (NUMLIGHTLEVELS * 256) * sizeof(uint16) );
		R_BuildLightTable( (byte *) mt->palette, image->palette);
	}
	else{
		image->palette = (uint16*) malloc( 256 * sizeof(uint16) );
		R_Build8to16Table( NULL );
		memcpy( image->palette, d_8to16table, 256 * sizeof(uint16) );
//		image->palette = (uint16*) malloc( (16 * 256) * sizeof(uint16) );
//		R_BuildFogTable( (byte *) mt->palette, image->palette );
	}

	image->pixels[0] = (byte*)malloc (size);
	image->pixels[1] = image->pixels[0] + (image->width*image->height);
	image->pixels[2] = image->pixels[1] + (image->width>>1)*(image->height>>1);
	image->pixels[3] = image->pixels[2] + (image->width>>2)*(image->height>>2);

	memcpy( (byte*)image->pixels[0], (byte *)mt + ofs, size );
/*
	pic = (byte *)mt + ofs;

	for ( i = 0; i < size; i++ ){
		if( pic[i] == 255 ){
			image->transparent = true;
			image->pixels[0][i] = TRANSPARENT_COLOR;
		}
		else{
			image->pixels[0][i] = pic[i];
		}
	}
*/
	ri.FS_FreeFile ((void *)mt);

	return image;
}

///////////////////////////////////////////////////////////////////////////////
//	R_FreeImage
//
///////////////////////////////////////////////////////////////////////////////

void	R_FreeImage (image_t *image)
{
	if( image->pixels[0] == NULL )
		return;

	free (image->pixels[0]);	// the other mip levels just follow
	memset (image, 0, sizeof(*image));
}

/*
===============
R_FindImage

Finds or loads the given image
===============
*/
image_t *R_FindImage (const char *name, imagetype_t type, int flags)
{
	image_t	*image;
	int		i, len;
	byte	*pic, *palette;
	int		width, height;

	int		procedural = false;
	char	fname [_MAX_EXT];

	if (!name)
		return NULL;	// ri.Sys_Error (ERR_DROP, "R_FindImage: NULL name");
	len = strlen(name);
	if (len<5)
		return NULL;	// ri.Sys_Error (ERR_DROP, "R_FindImage: bad name: %s", name);

	// look for it
	for (i=0, image=r_images ; i<numr_images ; i++,image++)
	{
		if (!strcmp(name, image->name))
		{
			image->registration_sequence = registration_sequence;
			return image;
		}
	}

	_splitpath (name, NULL, NULL, fname, NULL);
	if (strstr (fname, "pt_") == fname)
		procedural = true;

	//
	// load the pic from disk
	//
	pic = NULL;
	palette = NULL;
	if (!strcmp(name+len-4, ".pcx"))
	{
		LoadPCX (name, &pic, &palette, &width, &height);
		if (!pic)
			return NULL;	// ri.Sys_Error (ERR_DROP, "R_FindImage: can't load %s", name);

		current_palette = palette;

		if (procedural)
			image = protex_InitProceduralFromWAL (name, pic, width, height);
		else
			image = (image_t*)GL_LoadPic (name, pic, width, height, type);
	}
	else if (!strcmp(name+len-4, ".bmp"))
	{
		LoadBMP (name, &pic, &palette, &width, &height);

		current_palette = palette;

		if (!pic)
			return NULL; // ri.Sys_Error (ERR_DROP, "GL_FindImage: can't load %s", name);
		if (procedural)
			image = protex_InitProceduralFromWAL (name, pic, width, height);
		else
			image = (image_t*)GL_LoadPic (name, pic, width, height, type);
	}
	else if (!strcmp(name+len-4, ".wal"))
	{
		image = R_LoadWal( name, type );
	}
	else
		return NULL;	// ri.Sys_Error (ERR_DROP, "R_FindImage: bad extension on: %s", name);

	if( pic )
		free( pic );
	if( palette )
		free( palette );

	return image;
}



/*
===============
RegisterSkin
===============
*/
void *RegisterSkin (const char *name, resource_t flags)
{
	char	base[128], newname[128];
	image_t	*image;

	COM_StripExtension ((char *)name, &base[0]);

	sprintf (newname, "%s.wal", base);
	image =  (image_t*)R_FindImage (newname, it_skin, flags);

	if (!image || image == r_notexture_mip)
	{
		sprintf (newname, "%s.bmp", base);
		image = (image_t*)R_FindImage (newname, it_skin, flags);
	}

	return image;

//	return R_FindImage (name, it_skin, flags);
}


/*
================
R_FreeUnusedImages

Any image that was not touched on this registration sequence
will be freed.
================
*/
void R_FreeUnusedImages (void)
{
	int		i;
	image_t	*image;

	return;

	for (i=0, image=r_images ; i<numr_images ; i++, image++)
	{
		if (image->registration_sequence == registration_sequence)
		{
			Com_PageInMemory ((byte *)image->pixels[0], image->width*image->height * 2);
			continue;		// used this sequence
		}
		if (!image->registration_sequence)
			continue;		// free texture
		if (image->type == it_pic)
			continue;		// don't free pics
		// free it
		free (image->pixels[0]);	// the other mip levels just follow
		memset (image, 0, sizeof(*image));
	}
}



/*
===============
R_InitImages
===============
*/
void	R_InitImages (void)
{
	registration_sequence = 1;
}

/*
===============
R_ShutdownImages
===============
*/
void	R_ShutdownImages (void)
{
	int		i;
	image_t	*image;

	return;

	for (i=0, image=r_images ; i<numr_images ; i++, image++)
	{
		if (!image->registration_sequence)
			continue;		// free texture
		// free it
		free (image->pixels[0]);	// the other mip levels just follow
		memset (image, 0, sizeof(*image));
	}
}

