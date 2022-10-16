#include "ref.h"
#include "dk_ref_common.h"


// randomize background filename   12.18  dsn
/*
const char * RandBkgFilename (int fname_idx)  
{
  return (bkg_rootNames[fname_idx]);
}
*/



//loads a pcx file.  pic is allocated and returned with the appropriate info, and
//the width and height parameters are filled in.  If palette is NULL, the palette will not be loaded,
//otherwise a 768 byte palette will be allocated and the data loaded from the pcx file
void LoadPCX(const char *filename, byte **pic, byte **palette, int *width, int *height) {
    const byte    *raw;
    const pcx_t   *pcx;
    int     x, y;
    int     len;
    int     dataByte, runLength;
    byte    *out, *pix;

    *pic = NULL;
    *palette = NULL;

    //
    // load the file
    //
    len = ri.FS_LoadFile (filename, (void **)&raw);
    if (!raw) {
        //  Nelno:  don't tell me this, since we're using this as a test for per-level data
//        ri.Con_Printf (PRINT_DEVELOPER, "Bad pcx file %s\n", filename);
        return;
    }

    //
    // parse the PCX file
    //
    pcx = (pcx_t *)raw;
    raw = &pcx->data;

    if (pcx->manufacturer != 0x0a
        || pcx->version != 5
        || pcx->encoding != 1
        || pcx->bits_per_pixel != 8
        || pcx->xmax >= 640
        || pcx->ymax >= 480)
    {
        ri.Con_Printf (PRINT_ALL, "Bad pcx file %s\n", filename);
        return;
    }

	out = ( byte * ) ri.X_Malloc( ( pcx->ymax + 1 ) * ( pcx->xmax+1 ), MEM_TAG_IMAGE );

    *pic = out;

    pix = out;

    if (palette) {
		*palette = ( byte * ) ri.X_Malloc( 768, MEM_TAG_IMAGE );

        memcpy (*palette, (byte *)pcx + len - 768, 768);
    }

    if (width) {
        *width = pcx->xmax+1;
    }

    if (height) {
        *height = pcx->ymax+1;
    }

    for (y=0 ; y<=pcx->ymax ; y++, pix += pcx->xmax+1) {
        for (x=0 ; x<=pcx->xmax ; ) {
            dataByte = *raw++;

            if ((dataByte & 0xC0) == 0xC0) {
                runLength = dataByte & 0x3F;
                dataByte = *raw++;
            }
            else {
                runLength = 1;
            }

            while (runLength-- > 0) {
                pix[x++] = dataByte;
            }
        }

    }

    if (raw - (byte *)pcx > len) {
        ri.Con_Printf (PRINT_DEVELOPER, "PCX file %s was malformed", filename);
        ri.X_Free( *pic );
        *pic = NULL;
    }

    ri.FS_FreeFile((void *)pcx);
}

/*
=================================================================

BMP LOADING

=================================================================
*/

#pragma pack(push, 1)

#define BMP_SIGNATURE_WORD  0x4d42

typedef struct 
{
    unsigned short    bfType;       // signature - 'BM'
    unsigned long     bfSize;       // file size in bytes
    unsigned short    bfReserved1;  // 0
    unsigned short    bfReserved2;  // 0
    unsigned long     bfOffBits;    // offset to bitmap
} bmpheader_t;

typedef struct 
{
    unsigned long     biSize;       // size of this struct
    long              biWidth;      // bmap width in pixels
    long              biHeight;     // bmap height in pixels
    unsigned short    biPlanes;     // num planes - always 1
    unsigned short    biBitCount;   // bits perpixel
    unsigned long     biCompression; // compression flag
    unsigned long     biSizeImage;   // image size in bytes
    long              biXPelsPerMeter; // horz resolution
    long              biYPelsPerMeter; // vert resolution
    unsigned long     biClrUsed;       // 0 -> color table size
    unsigned long     biClrImportant;  // important color count
} bmpinfo_t;

typedef struct 
{
    unsigned char blue;
    unsigned char green;
    unsigned char red;
    unsigned char reserved;
} bgr_t;

typedef struct 
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} rgb_t;

typedef struct 
{
	int            bpp;        // bits per pixel
    int            width;
    int            height;
    unsigned char *data;
    rgb_t         *palette;
} bitmap_t;

#pragma pack(pop)

static void BMPLineNone(const byte **ptr, char *sline, int pixbytes, int width)
{
    int nbytes, i, k, j;

	switch (pixbytes)
	{
	    case 1 :
            nbytes = (width + 3) / 4;
            nbytes *= 4;

			memcpy (sline, *ptr, width);

			//	advance ptr!!!
			*ptr += width;

            nbytes -= width;
#pragma message( "// SCG[1/23/00]: What the hell?" )
            while (nbytes-- > 0);
				*ptr++;
			return;

		case 3 :
			nbytes = ((width * 3) + 3) / 4;
			nbytes *= 4;

			memcpy (sline, *ptr, width * 3);

			//	advance ptr!!!
			*ptr += width * 3;

			nbytes -= width * 3;

#pragma message( "// SCG[1/23/00]: What the hell?" )
            while (nbytes-- > 0);
				*ptr++;

			// reorder bgr to rgb
			for (i = 0, j = 0; i < width; i++, j += 3)
			{
				k = sline[j];
				sline[j] = sline[j+2];
				sline[j+2] = k;
			}

			return;
	}

	ri.Sys_Error (ERR_FATAL, "BMPLineNone failed.");
}

/*
==============
LoadBMP
==============
*/
void LoadBMP (const char *filename, byte **pic, byte **palette, int *width, int *height)
{
	const byte			*ptr;
    const byte *raw;
    bmpheader_t		*bhd;
    bmpinfo_t		*info;
    bgr_t			bgrpal[256];
	rgb_t			rgbpal[256];
    int				pxlsize = 1;
    int				rowbytes, i, pixbytes;
    char			*scanline;

	*pic = NULL;
    *palette = NULL;

	ri.FS_LoadFile (filename, (void **)&raw);

	if (!raw)
	{
		ri.Con_Printf (PRINT_DEVELOPER, "Bad bmp file %s\n", filename);
		return;
	}

	ptr = raw;

	// point to header
	bhd = (bmpheader_t *) ptr;
	ptr += sizeof (bmpheader_t);

	// make sure we have a valid bitmap file
	if (bhd->bfType != BMP_SIGNATURE_WORD)
		ri.Sys_Error (ERR_FATAL, "Invalid BMP file: %s", filename);

	info = (bmpinfo_t *) ptr;
	ptr += sizeof (bmpinfo_t);

	// make sure this is an info type of bitmap
	if (info->biSize != sizeof(bmpinfo_t))
		ri.Sys_Error (ERR_FATAL, "We only support the info bitmap type.");

	*width	= info->biWidth;
	*height	= info->biHeight;

	//currently we only read in 8 bit bmp files
	pixbytes = 1;
	if (info->biBitCount != 8)  
		ri.Sys_Error (ERR_FATAL, "%s: BPP %d not supported.", filename, info->biBitCount);

    //allocate space for the palette if there is not space already given to us.
	*palette = ( byte * ) ri.X_Malloc( sizeof( rgb_t ) * 256, MEM_TAG_IMAGE );

	memcpy (bgrpal, ptr, 256 * 4);
	ptr += 256 * 4;
	
	for (i = 0; i < 256; i++)
	{
		rgbpal[i].r = bgrpal[i].red;
		rgbpal[i].g = bgrpal[i].green;
		rgbpal[i].b = bgrpal[i].blue;
	}

	// load palette
	memcpy (*palette, rgbpal, 768);

//	// position to start of bitmap
//	fseek(f, bhd.bfOffBits, SEEK_SET);

	ptr = raw + bhd->bfOffBits;

	// create scanline to read data into
	rowbytes = ((info->biWidth * pixbytes) + 3) / 4;
	rowbytes *= 4;

	scanline = ( char * )ri.X_Malloc( rowbytes, MEM_TAG_MISC );

	// alloc space for new bitmap
	*pic = ( byte * ) ri.X_Malloc( info->biWidth * pixbytes * info->biHeight, MEM_TAG_IMAGE );

	if (info->biCompression)
	    ri.Sys_Error (ERR_FATAL, "Compressed bitmaps not supported.");
	
	// read in image
	for (i = 0; i < info->biHeight; i++)
	{
		BMPLineNone (&ptr, scanline, pixbytes, info->biWidth);

		// store line
		memcpy(*pic + (info->biWidth * pixbytes * (info->biHeight - i - 1)), scanline, info->biWidth * pixbytes);
	}

	ri.X_Free( scanline );

    ri.FS_FreeFile((void *)raw);

}

