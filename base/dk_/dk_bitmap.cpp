// mdm 98.02.08 - not portable
//#include <windows.h>
#include "dk_system.h"

#include "qcommon.h"

//loads the palette from a windows .bmp file.  Returns true if the operation was successful.
bool LoadBitmapPalette(const char *file_name, byte palette[128]) {

	// LAB - 19991126, commented out to use the compression code, doesn't seem to be used


    //open the file.
/*
    BITMAPFILEHEADER bmpHeader;
    BITMAPINFO dibHeader;
    RGBQUAD  *quad_palette;

	char *buf;
	int length = FS_LoadFile(file_name, (void **)&buf);

	quad_palette = (RGBQUAD*)buf + 30;

    //copy the colors over to our returned palette.
    for (int i = 1; i < 256; i++) {
        palette[i * 3 + 0] = quad_palette[i].rgbRed;
        palette[i * 3 + 1] = quad_palette[i].rgbGreen;
        palette[i * 3 + 2] = quad_palette[i].rgbBlue;
    }


	FS_FreeFile (buf);
*/

    return true;


/*

    FILE *file = NULL;
    FS_FOpenFile(file_name, &file);
    if (file == NULL) return false;


  //read the bitmap file headers.
  //int n = fread(&bmpHeader, sizeof(bmpHeader), 1, file);

  //check for an error.
  //if (n < 1) {
        //file format error, truncated file.
        FS_FCloseFile(file);
        return false;
    }

  //read the dib header.
    n = fread(&dibHeader, sizeof(dibHeader), 1, file);

  //check for an error
    if (n < 1) {
        //file format error, truncated file.
        FS_FCloseFile(file);
        return false;
    }

    //check the number of colors
    int colors = (0x1 << dibHeader.bmiHeader.biBitCount);
    if (colors != 256) {
        //file format error, not a 256 color bitmap.
        FS_FCloseFile(file);
        return false;
    }

    //read in the remainder of the palette, we already got the first color index.
    palette[0] = dibHeader.bmiColors[0].rgbRed;
    palette[1] = dibHeader.bmiColors[0].rgbGreen;
    palette[2] = dibHeader.bmiColors[0].rgbBlue;

    RGBQUAD quad_palette[256];

    n = fread(&quad_palette[1], sizeof(RGBQUAD), 255, file);

    //check for an error
    if (n < 255) {
        //file format error, truncated palette table.
        FS_FCloseFile(file);
        return false;
    }

    //copy the colors over to our returned palette.
    for (int i = 1; i < 256; i++) {
        palette[i * 3 + 0] = quad_palette[i].rgbRed;
        palette[i * 3 + 1] = quad_palette[i].rgbGreen;
        palette[i * 3 + 2] = quad_palette[i].rgbBlue;
    }

    FS_FCloseFile(file);
    return true;
*/
}


