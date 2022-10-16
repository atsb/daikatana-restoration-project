// HUD.cpp: implementation of the CHUD class.
//
//////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdarg.h>
#include "ClientEntityManager.h"
#include "ref.h"
#include "HUD.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHUD::CHUD()
{
    default_font = NULL;
}

CHUD::~CHUD()
{

}

///////////////////////////////////////////////////////////////////////
// PrintXY()
//
//  Description:
//      Prints a string starting at x,y using a specified font. If the
//      font is not supplied, then the default font is used.
//
//  Parameters:
//      x           x position of the first character of the string
//      y           y ""
//      pvfont      a void pointer to the dk_font to use
//      flags       not yet used, will be used for justification
//                  paramaters (left top, left bottom, left center, etc...)
//      fmt         printf() style formatting
//
//  Return:
//      void
//
//  BUGS:
//
//
void CHUD::PrintXY(int x, int y, void *pvfont, int flags, char * fmt, ...)
{
    va_list     argptr;
    char        output[256]={"\0"};
    int         s, i;
    void        *font;

    if(fmt)
    {
        va_start (argptr, fmt);
        vsprintf (output, fmt, argptr);
        va_end (argptr);
    }
    
    // find an unused HUD string
    for(s=0;s<HUD_STRINGS;s++)
    {
        if(Text[s][0].x == x && Text[s][0].y == y)
        {
            for(i=0;i<HUD_STRINGWIDTH;i++)
            {
                Text[s][i].font = NULL;
                Text[s][i].character = 0;
            }
            break;
        }
        if(Text[s][0].font == NULL)
            break;
    }
   
    // if fmt is NULL and xy is the same as an existing
    // layout, then the string was cleared out above 
    if(fmt)
    {
        if(!pvfont)
        {
            if(!default_font)
                default_font = manager->re->LoadFont("int_font");
            font = default_font;
        } else {
            font = pvfont;
        }
        
        for(i=0 ; i<strlen(output); i++)
        {
            if(i>=HUD_STRINGWIDTH-1)
                break;  // clip, don't overrun string buffer
            Text[s][i].font = font;
            Text[s][i].x = x+(i*8);    // do calc here
            Text[s][i].y = y;
            Text[s][i].character = output[i];
        }
        if(i>=HUD_STRINGWIDTH-1)
        {
            Text[s][i].character = 0;
            Text[s][i].font = NULL;
            Text[s][i].x=0;
            Text[s][i].y = 0;
        }
    }
        
}

void CHUD::CL_RunFrame()
{
    DrawText();
}


///////////////////////////////////////////////////////////////////////
//  DrawText()
//
//  Description:
//      Internal routine to draw all of the text in this HUD
//
void CHUD::DrawText()
{
    int i,s,w=0;

    for(s=0;s<HUD_STRINGS;s++)
    {
        if(!Text[s][0].character)
            continue;
        for(i=0; i<HUD_STRINGWIDTH; i++)
        {

            if(Text[s][i].character && Text[s][i].font)
                manager->re->DrawChar(Text[s][i].x, Text[s][i].y, Text[s][i].character);//, 0, 0, 0, 0, Text[s][i].color_rgb, 0, Text[s][i].font);
        }
    }
}

CClientEntity * CHUD::Alloc()
{
    return new CHUD;

}
