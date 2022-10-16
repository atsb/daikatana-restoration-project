#include <stdio.h>
#include "dk_misc.h"
#include "dk_array.h"
#include "dk_buffer.h"

#include "ref.h"
#include "dk_ref_common.h"
#include "dk_ref_font.h"

// Encompass MarkMa 032499
//#ifdef	JPN
#include <windows.h>
extern void GetStringSize( HDC *hMyDC, HFONT *hFont, HFONT *hOldFont,
				   LPCTSTR lpStr, UINT *nCharWidth, UINT *nCharHeight, UINT len );
//#endif	// JPN
// Encompass MarkMa 032499

//fill in the refexport_t function pointers that are implemented in this file.
FILE_INIT(dk_ref_font_cpp) {
    re.LoadFont = LoadFont;
    re.StringWidth = StringWidth;
    re.FontHeight = FontHeight;
    re.FontNumCharsInWidth = FontNumCharsInWidth;
	re.FontGetFontData = FontGetFontData;
//	re.Draw_AlphaChar = Draw_AlphaChar;
}

dk_font::dk_font() {
    for (int32 i = 0; i < 256; i++) {
        char_width[i] = 0;
        char_pos_x[i] = 0;
        char_pos_y[i] = 0;
    }

    image = NULL;
}

int dk_font::Compare(const dk_font **item1, const dk_font **item2) {
    return stricmp((*item1)->name, (*item2)->name);
}

int dk_font::Find(const dk_font *item, const void *id) {
    //the id is a character string.
    const char *name = (const char *)id;

    //check if the models's name is equal to the passed in name.
    return stricmp(name, item->name);
}


bool ReadDKF(const char *path, dk_font *font) {
    if (path == NULL) return false;
    if (font == NULL) return false;

    //open the file
    FILE *file;
    ri.FS_Open(path, &file);
    if (file == NULL) return false;

    //read the header.
    char header[5] = "    ";
    ri.FS_Read(header, 4, file);
    
    if (strncmp(header, "dkf ", 4) != 0) {
        //bad header.
        ri.FS_Close(file);
        return false;
    }

    //read the version number.
    int32 version;
    ri.FS_Read(&version, 4, file);

    if (version != 0) {
        //bad version.
        ri.FS_Close(file);
        return false;
    }

    //read the height.
    ri.FS_Read(&font->height, sizeof(font->height), file);

    //read the widths
    ri.FS_Read(font->char_width, 256, file);

    //read the character positions.
    ri.FS_Read(font->char_pos_x, 256, file);
    ri.FS_Read(font->char_pos_y, 256, file);

    //close the file
    ri.FS_Close(file);

    return true;
}

dk_font *ReadDiskFont(const char *bitmap_path, const char *dkf_path) {
    //allocate a font structure
    dk_font *font = new dk_font;

	if( font == NULL )
	{
		return NULL;
	}

    //try to load the pic.
    font->image = re.RegisterPic(bitmap_path, NULL, NULL, RESOURCE_GLOBAL);
    if (font->image == NULL) {
        delete font;
        return NULL;
    }

    //load the dkf file.
    if (ReadDKF(dkf_path, font) == false) {
        //couldn't load the font.
        delete font;
        return NULL;
    }

    return font;
}


//our array of loaded fonts.
array<dk_font> loaded_fonts;

void *LoadFont(const char *name) {
    if (name == NULL || name[0] == '\0') return NULL;

    //check if the font is in the array.
    const dk_font *font = loaded_fonts.Find(name, dk_font::Find);
    if (font != NULL) return (void *)font;

    //try to load a .tga first for alpha fonts
    //buffer256 tga_path("fonts/%s.font.tga", name);   // use .bmps   1.10dsn

    //make the path name for the font file.
    buffer256 dkf_path("fonts/%s.dkf", name);

    //try to load the font off disk.
    //dk_font *newfont = ReadDiskFont(tga_path, dkf_path);    // use .bmps   1.10dsn

    //if (newfont == NULL) {  // use .bmps   1.10dsn

        //look for a bitmap instead of a tga
//        buffer256 bitmap_path("pics/%s.font.bmp", name);
        buffer256 bitmap_path("fonts/%s.font.bmp", name);

        //try to load the font off disk.
        dk_font *newfont = ReadDiskFont(bitmap_path, dkf_path);
        if (newfont == NULL) return NULL;
    //}

    //set the font's name.
    newfont->name = name;

    //put the font into the array.
    loaded_fonts.Add(newfont, dk_font::Compare);

    return newfont;
}

int FontHeight(void *vfont) {
    if (vfont == NULL) return 8;

    dk_font *font = (dk_font *)vfont;

    return font->height;
}

int StringWidth(const char *string, void *vfont, int max_chars_counted) {
// Encompass MarkMa 032499
#ifdef	JPN
	HFONT			hOldFont;
	HDC				hMyDC;
	HFONT			hFont;
	UINT			nWidth, nHeight, nLen;
#endif	// JPN
// Encompass MarkMa 032499
    
	if (string == NULL) return 1;
    if (vfont == NULL) return 1;

    //cast our point to a font.
    dk_font *font = (dk_font *)vfont;

    if (font->image == NULL) return 1;

    //add the widths of each character.
    int total_width = 0;

    //if max_chars_counted is -1, we count all characters in the string.
    if (max_chars_counted == -1) max_chars_counted = 9999;

// Encompass MarkMa 032499
#ifdef	JPN	// JPN
	unsigned char cc = string[0];
	nLen = strlen(string);
	if( max_chars_counted > 0 && lstrcmp((LPCTSTR) font->name, (LPCTSTR) "int_buttons")
			&& ( !(nLen==1 && ( (126<cc && cc<130) || (140<cc && cc<145) 
			   || (156<cc && cc<159) || cc==176 || cc==183)) )
			   )  {	// Draw string on screen
		if(max_chars_counted != 9999)	 nLen = max_chars_counted;
		GetStringSize( &hMyDC, &hFont, &hOldFont, (LPCTSTR) string, &nWidth, &nHeight, nLen);
		total_width = nWidth;
		SelectObject(hMyDC, hOldFont);
		if (hFont)	DeleteObject(hFont);
		DeleteDC(hMyDC);
	}
	else
#endif	// JPN
    { // Encompass MarkMa 032499 JPN
    //loop until we reach the end of the string, or have counted all the characters we are supposed to
    for (; string[0] != '\0' && max_chars_counted > 0; string++, max_chars_counted--) {
        //get the character.
        unsigned char c = string[0];

        //check for space.
        if (c == ' ') {
            total_width += font->height >> 1;
            continue;
        }

        //check if we have this character in the font.
        if (font->char_width[c] == 0) {
            //dont add anything to the width.
            continue;
        }

        //add the width of this character, plus a 1 pixel gap after it.
        total_width += font->char_width[c] + 1;
    }
	} // Encompass MarkMa 032499 JPN

    return total_width;
}

int FontNumCharsInWidth(const char *string, void *vfont, int allowed_pixel_width, int start_pos) {
    if (vfont == NULL) return 0;
    if (string == NULL) return 0;

    //get a pointer to our real font.
    dk_font *font = (dk_font *)vfont;

    //first get to the starting character in the string.
    for (int32 pos = 0; pos < start_pos; pos++) {
        //make sure the string hasn't ended.
        if (string[pos] == '\0') return 0;
    }

    //the number of pixels we have taken into account so far.
    int32 current_width = 0;
    
    //the number of characters we have counted that use up that width.
    int32 num_chars = 0;

    //continue in the string and add up the widths of the characters.
    for (; string[pos] != '\0'; pos++) {
        //get the character.
        unsigned char c = string[pos];

#ifdef JPN // Frank 3/29/99
		if (IsDBCSLeadByte(c)) {
            current_width += StringWidth(string+pos, vfont, 2);
			pos++;
			//check if we have exceeded our allowed width.
	        if (current_width > allowed_pixel_width) {
		        //stop.
			    return num_chars;
	        }
			num_chars += 2;
		}
		else {
            current_width += StringWidth(string+pos, vfont, 1);
			//check if we have exceeded our allowed width.
	        if (current_width > allowed_pixel_width) {
		        //stop.
			    return num_chars;
	        }

			//increment our character count.
			num_chars++;
        }
#else // JPN
        //check for space.
        if (c == ' ') {
            current_width += font->height >> 1;
        }
        //check if we have this character in the font.
        else if (font->char_width[c] == 0) {
            //dont add anything to the width.
            continue;
        }
        else {
            //add the width of this character, plus a 1 pixel gap after it.
            current_width += font->char_width[c] + 1;
        }

        //check if we have exceeded our allowed width.
        if (current_width > allowed_pixel_width) {
            //stop.
            return num_chars;
        }

        //increment our character count.
        num_chars++;
#endif // JPN
    }

    //return the number of characters we counted.
    return num_chars;
}

///////////////////////////////////////////////////////////////////////////////
//  FontGetFontData
//
//  Returns some pointers to a dk_font fields so that other routines can
//  use Draw_AlphaChar with proportional fonts to draw a string
///////////////////////////////////////////////////////////////////////////////

void FontGetFontData(void *vfont, int *height, byte **charWidth, byte **charX, byte **charY, void **image) {
    dk_font *font = (dk_font *) vfont;

    if (font == NULL) {
        if (height) {
            *(int *) height = 8;
        }
        if (charWidth) {
            *(byte **) charWidth = NULL;
        }
        if (charX) {
            *(byte **) charX = NULL;
        }
        if (charY) {
            *(byte **) charY = NULL;
        }
        if (image) {
            *(void **) image = NULL;
        }
    }       
    else{
        if (height) {
            *(int *) height = font->height;
        }
        if (charWidth) {
            *(byte **) charWidth = font->char_width;
        }
        if (charX) {
            *(byte **) charX = font->char_pos_x;
        }
        if (charY) {
            *(byte **) charY = font->char_pos_y;
        }
        if (image) {
            *(void **) image = font->image;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//  Draw_AlphaChar
//  
//  Draws one 8*8 graphics character with an alpha channel and optional effects
///////////////////////////////////////////////////////////////////////////////
/*
int Draw_AlphaChar(int x, int y, int num, int startMsec, int endMsec, int curMsec, int timeOffset, CVector &rgbBlend, int flags, void *vfont) {
    //get our font object.
    dk_font *font = (dk_font *)vfont;

    //mod num with 256.
    num &= 255;

    //the image we will draw the character from.
    void *font_image;

    //the width and height of the character.
    int char_width, char_height;

    //the position of the character in the font image.
    int char_left, char_top;
    
    //check if we were given a font.
    if (vfont != NULL) {
        //check for space.
        if (num == ' ') {
            return font->height >> 1;      // space
        }

        //check if this character is in the font.
        if (font->char_width[num] < 1) return 0;

        //get the image.
        font_image = font->image;

        //get the width and height of the character.
        char_width = font->char_width[num];
        char_height = font->height;

        //get the position of the character in the image.
        char_left = font->char_pos_x[num];
        char_top = font->char_pos_y[num];
    }
    else {
        //check for space.
        if ((num & 127) == ' ') {
            return 8;
        }

        //get the image.
        font_image = default_chars_image;

        //get the width and height of the character.
        char_width = 8;
        char_height = 8;

        //get the position of the character in the image.
        //there are 16 characters per row.  take num mod 16 to get the column of the character.
        char_left = (num & 15) * 8;
        //take num / 16 to get the row of the character.
        char_top = (num >> 4) * 8;
    }

    //
    //do processing to compute the alpha factor, flare, etc.
    //

    startMsec += timeOffset;
    endMsec += timeOffset;

    int totalTime = endMsec - startMsec;
    int curTime = curMsec - startMsec;
    int halfTime = totalTime / 2;
    int quarterTime = totalTime / 4;

    int endBeginEffect = quarterTime;               //  1/4 of total time
    int startEndEffect = quarterTime + halfTime;    //  3/4 of total time

    //the alpha we will draw the character.
    float alpha = 1.0f;

    //the alpha we will draw the flare.
    float flareAlpha = 0.0f;

    if (curTime < endBeginEffect) {
        if (flags & ACF_BEGINFADE) {
            //  letters are fading in, so compute fractional alpha
            alpha = (float)curTime / (float)endBeginEffect;
        }

        if (flags & ACF_BEGINFLARE) {
            int stopFadeIn = endBeginEffect / 4;

            if (curTime < stopFadeIn) {
                flareAlpha = (float)curTime / (float)stopFadeIn;
            }
            else {
                flareAlpha = (float)(curTime - stopFadeIn) / (float)(endBeginEffect - stopFadeIn);
                flareAlpha = 1.0 - flareAlpha;
            }
        }
    }
    else if (curTime > startEndEffect) {
        if (flags & ACF_ENDFADE) {
            //  letters are fading out, so compute fractional alpha
            alpha = (float)(curTime - startEndEffect) / (float)(totalTime - startEndEffect);
            alpha = 1.0 - alpha;
        }

        if (flags & ACF_ENDFLARE) {
            int totalFlareTime = (totalTime - startEndEffect);
            int stopFadeIn = totalFlareTime / 4;

            if (curTime < stopFadeIn + startEndEffect) {
                flareAlpha = (float)curTime / (float)stopFadeIn;
            }
            else {
                flareAlpha = (float)(curTime - stopFadeIn) / (float)(totalTime - stopFadeIn);
                flareAlpha = 1.0 - flareAlpha;
            }
        }
    }

    //check if we can see the flare
    if (flareAlpha > 0.0) {
        //the angle of rotation for the flare.
        float angle = 0.0f;

        if (1) {//    flags & ACF_ROTATEFLARE) {
            angle = ref_laserRotation;
            angle = angle / (180 / M_PI);   //  convert to radians
        }

        //draw the flare.
        Draw_CharFlare (x, y, char_width, char_width, char_flare_image, rgbBlend, flareAlpha, flareAlpha + 0.5, angle);
    }

    //draw the character.
//    DrawPic( x, y, char_left, char_top, char_width, char_height, font_image, rgbBlend, alpha, 0 );

    //return the width plus 1.
    return char_width + 1;
}
*/