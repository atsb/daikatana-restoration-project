// console.c
// added for Japanese version
#ifdef JPN
#include <windows.h>
#endif // JPN

#include "client.h"

//	Nelno:	header for daikatana specific code
#include	"daikatana.h"
#include  "l__language.h"

console_t	con;

cvar_t		*con_notifytime;
cvar_t		*con_usefixedfont;
cvar_t    *console_screen_idx;
cvar_t    *loadscreen;
cvar_t    *lastmapname;

void	*con_font = NULL;
void	*notify_font = NULL;
static	int		con_fontHeight = 8;
static	int		notify_fontHeight = 8;

extern void		*scoreboard_font;
static int		scoreboard_font_height = 24;

#define		MAXCMDLINE	256
extern	char	key_lines[32][MAXCMDLINE];
extern	int		edit_line;
extern	int		key_linepos;

//extern	cvar_t *sv_episode;
//extern  cvar_t *sv_mapnum;

extern scr_draw_progress;


extern qboolean bGotLoadscreenName;

#define CONSOLE_CURSOR  (95)  // ascii char that represents text-input cursor

#ifdef JPN

void ImeSetMode3(BOOL mode);
extern HWND        cl_hwnd;


//#ifdef CONSOLE

HANDLE hConImeStr=NULL;
char ImeBuffer[255];

void ConSaveIme(char* lpStr)
{
	char *lpConImeStr;
	int x;

	if (hConImeStr)
		LocalFree(hConImeStr);
	hConImeStr = NULL;

	if (!lpStr)
		return;
	x = lstrlen(lpStr);
	if ( !(hConImeStr = LocalAlloc( LPTR, x+1)) )
		goto exit2;
	if ( !( lpConImeStr = (char*) LocalLock( hConImeStr ) ) )
		goto exit1;
	lstrcpy(lpConImeStr, lpStr);
	LocalUnlock( hConImeStr );
	return;
	

exit1:
	LocalFree( hConImeStr );

exit2:
	return;
}
void ConSetIme(char* lpStr)
{
	if(lpStr == NULL)
	{
		strcpy(ImeBuffer,"");
		return;
	}

	strncpy(ImeBuffer, lpStr, 255);
}

void ConAddChat()
{
	strcat(chat_buffer, ImeBuffer);
	chat_bufferlen += strlen(ImeBuffer);

}
//#endif
#endif // JPN

///////////////////////////////////////////////////////////////////////////////
//	Con_InitFont
//
///////////////////////////////////////////////////////////////////////////////

void	Con_InitFont (void)
{
	if (!con_font)
	{
		//	FIXME:	hack to make software work until proportional fonts are fixed
/*		if (!stricmp (Cvar_VariableString ("vid_ref"), "soft"))
		{
			con_fontHeight = 8;
			con_font = NULL;f
		}
		else
*/
		{
//			con_font = re.LoadFont ("con_font");
			con_font = re.LoadFont ("int_font");

			re.FontGetFontData (con_font, &con_fontHeight, NULL, NULL, NULL, NULL);
		}
	}
	
	con_usefixedfont = Cvar_Get ("con_usefixedfont", "1", 0);

	//&&& amw - because proportional fonts on the console look like ass
	if (con_usefixedfont->value)
	{
		notify_font = con_font;
		notify_fontHeight = con_fontHeight;
	}
	else
	{
		if (!notify_font)
		{
			//	FIXME:	hack to make software work until proportional fonts are fixed
			if (!stricmp (Cvar_VariableString ("vid_ref"), "soft"))
			{
				notify_fontHeight = 8;
				notify_font = NULL;
			}
			else
			{
				notify_font = re.LoadFont ("scr_cent");

				re.FontGetFontData (notify_font, &notify_fontHeight, NULL, NULL, NULL, NULL);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////
		
int	DrawString (int x, int y, char *s)
{
//	int		w = 0;

	if (!con_font)
		Con_InitFont ();

/*
	while (*s)
	{
		//	re.DrawChar (x, y, *s);
		//	x+=8;
		w += re.Draw_AlphaChar (x + w, y, *s, 0, 0, 0, 0, vec3_identity, 0, con_font);
		s++;
	}
*/
//	w = re.Draw_AlphaString( x, y, s, 0, 0, 0, 0, vec3_identity, 0, con_font );
//==================================
// consolidation change: SCG 3-15-99
	DRAWSTRUCT drawStruct;
	drawStruct.nFlags = DSFLAG_BLEND;
	drawStruct.szString = s;
	drawStruct.nXPos = x;
	drawStruct.nYPos = y;
	drawStruct.pImage = con_font;
	return re.DrawString( drawStruct );
// consolidation change: SCG 3-15-99
//==================================

//	return	w;
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

int	DrawAltString (int x, int y, char *s)
{
	int		w = 0;

	if (!con_font)
		Con_InitFont ();

/*
	while (*s)
	{
		//	re.DrawChar (x, y, *s ^ 0x80);
		//	x+=8;
		w += re.Draw_AlphaChar (x + w, y, *s ^ 0x80, 0, 0, 0, 0, vec3_identity, 0, con_font);
		s++;
	}
*/
//	w = re.Draw_AlphaString( x, y, s, 0, 0, 0, 0, vec3_identity, 0, con_font );
//==================================
// consolidation change: SCG 3-15-99
	DRAWSTRUCT drawStruct;
	drawStruct.nFlags = DSFLAG_BLEND;
	drawStruct.szString = s;
	drawStruct.nXPos = x;
	drawStruct.nYPos = y;
	drawStruct.pImage = con_font;
	return re.DrawString( drawStruct );
// consolidation change: SCG 3-15-99
//==================================

//	return	w;
}


void Key_ClearTyping (void)
{
	key_lines[edit_line][1] = 0;	// clear any typing
	key_linepos = 1;
}

///////////////////////////////////////////////////////////////////////////////
//	Con_ConsoleOff
//
///////////////////////////////////////////////////////////////////////////////

void	Con_ConsoleOff (void)
{
	Key_ClearTyping ();
	Con_ClearNotify ();

	M_ForceMenuOff ();
}

/*
================
Con_ToggleConsole_f
================
*/
void Con_ToggleConsole_f (void)
{
	SCR_EndLoadingPlaque ();	// get rid of loading plaque

	//	Nelno:	don't kill server in attract loop
/*
	if (cl.attractloop)
	{
		//	Cbuf_AddText ("killserver\n");
		return;
	}
*/
	if (cls.state == ca_disconnected)
	{
		// Nelno:	server was killed or connection lost, automatically go into demo mode
//		Cbuf_AddText ("d1\n");		

		return;
	}

	Key_ClearTyping ();
	Con_ClearNotify ();

	if (cls.key_dest == key_console)
	{
		M_ForceMenuOff ();

		//	done in M_ForceMenuOff
		Cvar_Set ("paused", "0");
	}
	else
	{
		M_ForceMenuOff ();
		cls.key_dest = key_console;	
		
		con.mode = CON_NORMAL;

		if (Cvar_VariableValue ("maxclients") == 1 
			&& Com_ServerState ())
			Cvar_Set ("paused", "1");
	}
}

///////////////////////////////////////////////////////////////////////////////
//	Con_ToggleQuickConsole_f
//
///////////////////////////////////////////////////////////////////////////////

void Con_ToggleQuickConsole_f (void)
{
	SCR_EndLoadingPlaque ();	// get rid of loading plaque

	//	Nelno:	don't kill server in attract loop
/*
	if (cl.attractloop)
	{
		Cbuf_AddText ("killserver\n");
		return;
	}
*/
	if (cls.state == ca_disconnected)
	{	
		// Nelno:	server was killed or connection lost, automatically go into demo mode
//		Cbuf_AddText ("d1\n");

		return;
	}

	Key_ClearTyping ();
	Con_ClearNotify ();

	if (cls.key_dest == key_console)
	{
		//	turn console off
		M_ForceMenuOff ();

		if( con.mode == CON_NORMAL )
		{
			Cvar_Set ("paused", "0");
		}

	}
	else
	{
		//	turn console on
		M_ForceMenuOff ();
		cls.key_dest = key_console;	

		if (cls.state != ca_disconnected)
			con.mode = CON_QUICK;
		else
			con.mode = CON_NORMAL;

		//	Nelno:	never pause for quick console
//		if (Cvar_VariableValue ("maxclients") == 1 && Com_ServerState ())
//			Cvar_Set ("paused", "1");
	}
}

/*
================
Con_ToggleChat_f
================
*/
void Con_ToggleChat_f (void)
{
	Key_ClearTyping ();

	if (cls.key_dest == key_console)
	{
		if (cls.state == ca_active)
		{
			M_ForceMenuOff ();
			cls.key_dest = key_game;
		}
	}
	else
		cls.key_dest = key_console;
	
	Con_ClearNotify ();
}

/*
================
Con_Clear_f
================
*/
void Con_Clear_f (void)
{
	memset (con.text, ' ', CON_TEXTSIZE);
}

						
/*
================
Con_Dump_f

Save the console contents out to a file
================
*/
void Con_Dump_f (void)
{
	int		l, x;
	char	*line;
	FILE	*f;
	char	buffer[1024];
	char	name[MAX_OSPATH];

	if (GetArgc() != 2)
	{
		Com_Printf ("usage: condump <filename>\n");
		return;
	}

	Com_sprintf (name, sizeof(name), "%s/%s.txt", FS_Gamedir(), GetArgv(1));

	Com_Printf ("Dumped console text to %s.\n", name);
	FS_CreatePath (name);
	f = fopen (name, "w");
	if (!f)
	{
		Com_Printf ("ERROR: couldn't open.\n");
		return;
	}

	// skip empty lines
	for (l = con.current - con.totallines + 1 ; l <= con.current ; l++)
	{
		line = con.text + (l%con.totallines)*con.linewidth;
		for (x=0 ; x<con.linewidth ; x++)
			if (line[x] != ' ')
				break;
		if (x != con.linewidth)
			break;
	}

	// write the remaining lines
	buffer[con.linewidth] = 0;
	for ( ; l <= con.current ; l++)
	{
		line = con.text + (l%con.totallines)*con.linewidth;
		strncpy (buffer, line, con.linewidth);
		for (x=con.linewidth-1 ; x>=0 ; x--)
		{
			if (buffer[x] == ' ')
				buffer[x] = 0;
			else
				break;
		}

		fprintf (f, "%s\n", buffer);
	}

	fclose (f);
}


/*
=================
Con_ShowScreen   12.21  dsn

Show a specific background screen on the console
=================
*/
void Con_ShowScreen_f (void)
{
  int scr_idx;

  if (GetArgc() < 2)
  {
    Com_Printf ("Usage: showscreen <index>\n");
	return;
  }
  
  scr_idx = atoi(GetArgv(1));
  if (scr_idx < 0 || scr_idx > (NUM_RANDOM_BKG_SCREENS-1))
  {
    Com_Printf ("Valid range: (0-%d)\n",(NUM_RANDOM_BKG_SCREENS-1));
	return;
  }

  scr_idx *= 2; // multiply times 2 because of two filename indices per 'screen'

  Cvar_SetValue("console_screen_idx", scr_idx);

  return;
}

						
/*
================
Con_ClearNotify
================
*/
void Con_ClearNotify (void)
{
	int		i;
	
	for (i=0 ; i<NUM_CON_TIMES ; i++)
		con.times[i] = 0;
}

						
/*
================
Con_MessageMode_f
================
*/
void Con_MessageMode_f (void)
{
	chat_team = false;
	cls.key_dest = key_message;
}

/*
================
Con_MessageMode2_f
================
*/
void Con_MessageMode2_f (void)
{
	chat_team = true;
	cls.key_dest = key_message;
}

/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/
void Con_CheckResize (void)
{
	int		i, j, width, oldwidth, oldtotallines, numlines, numchars;
	char	tbuf[CON_TEXTSIZE];

	width = (viddef.width / 8) - 2;

	if (width == con.linewidth)
		return;

	if (width < 1)			// video hasn't been initialized yet
	{
		width = 38;
		con.linewidth = width;
		con.totallines = CON_TEXTSIZE / con.linewidth;
		memset (con.text, ' ', CON_TEXTSIZE);
	}
	else
	{
		oldwidth = con.linewidth;
		con.linewidth = width;
		oldtotallines = con.totallines;
		con.totallines = CON_TEXTSIZE / con.linewidth;
		numlines = oldtotallines;

		if (con.totallines < numlines)
			numlines = con.totallines;

		numchars = oldwidth;
	
		if (con.linewidth < numchars)
			numchars = con.linewidth;

		memcpy (tbuf, con.text, CON_TEXTSIZE);
		memset (con.text, ' ', CON_TEXTSIZE);

		for (i=0 ; i<numlines ; i++)
		{
			for (j=0 ; j<numchars ; j++)
			{
				con.text[(con.totallines - 1 - i) * con.linewidth + j] =
						tbuf[((con.current - i + oldtotallines) %
							  oldtotallines) * oldwidth + j];
			}
		}

		Con_ClearNotify ();
	}

	con.current = con.totallines - 1;
	con.display = con.current;
}

cvar_t *ai_debug_info;

/*
================
Con_Init
================
*/
void Con_Init (void)
{
	con.linewidth = -1;

    ai_debug_info = Cvar_Get("ai_debug_info", "0", CVAR_ARCHIVE);

	Con_CheckResize ();
	
	Com_Printf ("Console initialized.\n");

//
// register our commands
//
	con_notifytime = Cvar_Get ("con_notifytime", "6", CVAR_ARCHIVE);
    
	Cmd_AddCommand ("toggleconsole", Con_ToggleConsole_f);
	Cmd_AddCommand ("togglechat", Con_ToggleChat_f);
	Cmd_AddCommand ("messagemode", Con_MessageMode_f);
	Cmd_AddCommand ("messagemode2", Con_MessageMode2_f);
	Cmd_AddCommand ("clear", Con_Clear_f);
	Cmd_AddCommand ("condump", Con_Dump_f);
    Cmd_AddCommand ("showscreen", Con_ShowScreen_f);

	dk_print_init ();

//	con_font = re.LoadFont("scr_cent");

	con.initialized = true;
	con.mode = CON_NORMAL;

    RANDOMIZE();
    
    // randomize starting console's background screen 
	console_screen_idx = Cvar_Get ("console_screen_idx", "1", 0);
    Cvar_SetValue ("console_screen_idx", ((rand() % NUM_RANDOM_BKG_SCREENS) * 2));
    //Cvar_SetValue ("console_screen_idx", 0);
}


/*
===============
Con_Linefeed
===============
*/
void Con_Linefeed (void)
{
	con.x = 0;
	if (con.display == con.current)
		con.display++;
	con.current++;
	memset (&con.text[(con.current%con.totallines)*con.linewidth]
	, ' ', con.linewidth);
}

/*
================
Con_Print

Handles cursor positioning, line wrapping, etc
All console printing must go through this in order to be logged to disk
If no console is visible, the text will appear at the top of the game window
================
*/
void Con_Print (char *txt)
{
	int		y;
	int		c, l;
	static int	cr;
	int		mask;

	if (!con.initialized)
		return;

	if (txt[0] == 1 || txt[0] == 2)
	{
		mask = 128;		// go to colored text
		txt++;
	}
	else
		mask = 0;


	while ( (c = *txt) )
	{
	// count word length
// Encompass MarkMa 032999
#ifdef	JPN		// JPN
		for (l=0 ; l< con.linewidth ; l++)	{
			if ( (unsigned char)txt[l] <= ' ')		// add "(unsigned char)" for bug 1497
				break;
			
			if(IsDBCSLeadByte(txt[l]))	{
				l++;
				break;			
			}
		}

	// word wrap
		if(IsDBCSLeadByte(txt[l-1]))	{
			if (l != con.linewidth && (con.x + l+1 >= con.linewidth) )
				con.x = 0;
		}
		else	{
			if (l != con.linewidth && (con.x + l+1 > con.linewidth) )
				con.x = 0;
		}
#else	// JPN
		for (l=0 ; l< con.linewidth ; l++)
			if ( txt[l] <= ' ')
				break;

	// word wrap
		if (l != con.linewidth && (con.x + l > con.linewidth) )
			con.x = 0;
#endif	// JPN
// Encompass MarkMa 032999

		txt++;

		if (cr)
		{
			con.current--;
			cr = false;
		}

		
		if (!con.x)
		{
			Con_Linefeed ();
		// mark time for transparent overlay
			if (con.current >= 0)
				con.times[con.current % NUM_CON_TIMES] = cls.realtime;
		}

		switch (c)
		{
		case '\n':
			con.x = 0;
			break;

		case '\r':
			con.x = 0;
			cr = 1;
			break;

		default:	// display character and advance
			y = con.current % con.totallines;
// Encompass MarkMa 032999
#ifdef	JPN		// JPN
			if(IsDBCSLeadByte(c))	{
				con.text[y*con.linewidth+con.x] = c;
				con.x++;
				c = *txt;	txt++;
			}
			con.text[y*con.linewidth+con.x] = c;
			con.x++;

			if (con.x+1 >= con.linewidth)
				con.x = 0;
#else	// JPN
			con.text[y*con.linewidth+con.x] = c | mask | con.ormask;
			con.x++;
			if (con.x >= con.linewidth)
				con.x = 0;
#endif	// JPN
// Encompass MarkMa 032999

			break;
		}
		
	}
}


/*
==============
Con_CenteredPrint
==============
*/
void Con_CenteredPrint (char *text)
{
	int		l;
	char	buffer[1024];

	l = strlen(text);
	l = (con.linewidth-l)/2;
	if (l < 0)
		l = 0;
	memset (buffer, ' ', l);
	strcpy (buffer+l, text);
	strcat (buffer, "\n");
	Con_Print (buffer);
}

#ifdef JPN
//#ifdef CONSOLE
extern void *menu_font_bright;

void ConShowComposition() // DK_CONSOLE
{
	char *lpConImeStr;
	DRAWSTRUCT drawStruct;
	char *text;
	int i;

	if (!hConImeStr)
		return;
	if ( !( lpConImeStr = (char*) LocalLock( hConImeStr ) ) )
		return;
	
	text = key_lines[edit_line];
	
	// add the cursor frame
	text[key_linepos] = 10 + ((int)(cls.realtime >> 8)&1);
	
	// fill out remainder with spaces
	//for (i=key_linepos+1 ; i< con.linewidth ; i++)
	//	text[i] = ' ';
		
	//	prestep if horizontally scrolling
	for (i=0; key_linepos - i >	con.linewidth - 10; )
	{
		if (IsDBCSLeadByte(text[i]))
			i++;
		i++;
	}
	text += i;
/*	//key_linepos;	  viddef.width

        //we are editing the text.  Copy the text, removing the portion that is off the left of the field.
        buffer256 print; //we dont use operator= here because it will call the constructor and interpret text as the format string.
        print = text.Right(num_chars_off_left);

        //Determine how much of the text is visible.
        int32 num_visible_chars = re.FontNumCharsInWidth(print, font_normal, w - 2 * INPUT_FIELD_SPACE_LEFT, 0);

        //truncate the string we will print.
        print.NewEnding(num_visible_chars, "");

        //print the string.
        //M_DrawString(norm_x + INPUT_FIELD_SPACE_LEFT, norm_y + 5, print, font_normal, false, false);

        //
        //draw the insertion bar.
        //
        
        //truncate the string at the insertion bar position.
        //int.NewEnding(insertion_bar_position - num_chars_off_left, "");*/

        //get the width of that string.
	int32 bar_offset = re.StringWidth(text, con_font, key_linepos - i);


	drawStruct.nFlags = 0;
	drawStruct.szString = lpConImeStr;
	drawStruct.nXPos = bar_offset; //0;
	drawStruct.nYPos = viddef.height - con_fontHeight;
	drawStruct.pImage = menu_font_bright;//con_font;
	re.DrawString( drawStruct );

	// get the width of the composition string
	int len_imetext = re.StringWidth(lpConImeStr,menu_font_bright, -1);
	// show the caret after the composition
	bar_offset += len_imetext;
    
	//
    //draw the insertion bar.
    //
	drawStruct.nFlags = 0;
	drawStruct.szString = key_lines[edit_line] +  key_linepos;
	drawStruct.nXPos = bar_offset; //0;
	drawStruct.nYPos = viddef.height - con_fontHeight;
	drawStruct.pImage = con_font;
	re.DrawString( drawStruct );

	LocalUnlock( hConImeStr);
}
//#endif
#endif // JPN

/*
==============================================================================

DRAWING

==============================================================================
*/


/*
================
Con_DrawInput

The input line scrolls horizontally if typing goes beyond the right edge
================
*/
void Con_DrawInput (void)
{
//	int		y, x;
	int		i, lineHeight = con_fontHeight + 2;
	char	*text;

	if (cls.key_dest == key_menu)
		return;
	if (cls.key_dest != key_console && cls.state == ca_active)
		return;		// don't draw anything (allways draw if not active)

	if (!con_font)
		Con_InitFont ();

	text = key_lines[edit_line];

#ifdef JPN
	if (cls.key_dest == key_game)
		cls.key_dest = key_console;		// Enable IME during console
/* // changed by yokoyama
	if (!hConImeStr)
		// add the cursor frame
		text[key_linepos] = 10 + ((int)(cls.realtime >> 8)&1);
	else
		text[key_linepos] = ' ';
*/
// added by yokoyama
    // blinking cursor text entry
	if (!hConImeStr)
	{
		if ( (int)(cls.realtime >> 8) & 1)
			text[key_linepos] = CONSOLE_CURSOR;				 // ascii/font chararacter
		else
			text[key_linepos] = 0;
	}
	else
		text[key_linepos] = ' ';


	// fill out remainder with spaces
	for (i=key_linepos+1 ; i< con.linewidth ; i++)
		text[i] = ' ';

	for (i=0; key_linepos - i >	con.linewidth - 10; )
	{
		if (IsDBCSLeadByte(text[i]))
			i++;
		i++;
	}
	text += i;
#else // JPN	
	
	// add the cursor frame
	//text[key_linepos] = 10 + ((int)(cls.realtime >> 8)&1);

	// blinking cursor text entry
	if ( (int)(cls.realtime >> 8) & 1)
	  text[key_linepos] = CONSOLE_CURSOR;				 // ascii/font chararacter
	else
		text[key_linepos] = 0;
	
	// fill out remainder with spaces
	for (i=key_linepos+1 ; i< con.linewidth ; i++)
		text[i] = ' ';
		
	//	prestep if horizontally scrolling
	if (key_linepos >= con.linewidth)
		text += 1 + key_linepos - con.linewidth;

#endif //JPN
	// draw it
//	y = con.vislines - lineHeight;

	//	Nelno:	take input from bottom line of screen for quick console
	CVector rgbColor;
	rgbColor.x = rgbColor.y = rgbColor.z = 0;
	DRAWSTRUCT drawStruct;
	if (con.mode == CON_QUICK)
	{
	    re.DrawFill( 0, viddef.height - con_fontHeight - 2, viddef.width, con_fontHeight + 2, rgbColor, 1.0 );// consolidation change: SCG 3-11-99
//		for (i = 0, x = 0; i < con.linewidth; i++)
//		{
//			//	re.DrawChar ( (i+1)<<3, viddef.height - 10, text[i]);
//			x += re.Draw_AlphaChar (x, viddef.height - con_fontHeight, text [i], 0, 0, 0, 0, vec3_identity, 0, con_font);
//		}
//==================================
// consolidation change: SCG 3-15-99
		text[con.linewidth - 1] = '\0';
		drawStruct.nFlags = DSFLAG_BLEND;
		drawStruct.szString = text;
		drawStruct.nXPos = 0;
		drawStruct.nYPos = viddef.height - con_fontHeight;
		drawStruct.pImage = con_font;
		re.DrawString( drawStruct );
// consolidation change: SCG 3-15-99
//==================================
	}
	else
	{
//		for (i = 0, x = 0; i < con.linewidth; i++)
			//	re.DrawChar ( (i+1)<<3, con.vislines - 16, text[i]);
//			x += re.Draw_AlphaChar (x, con.vislines - lineHeight, text [i], 0, 0, 0, 0, vec3_identity, 0, con_font);
//==================================
// consolidation change: SCG 3-15-99
		text[con.linewidth - 1] = '\0';
		drawStruct.nFlags = DSFLAG_BLEND;
		drawStruct.szString = text;
		drawStruct.nXPos = 0;
		drawStruct.nYPos = con.vislines - lineHeight;
		drawStruct.pImage = con_font;
		re.DrawString( drawStruct );
// consolidation change: SCG 3-15-99
//==================================
	}

	// remove cursor
	key_lines[edit_line][key_linepos] = 0;
#ifdef JPN //JPN
//#ifdef CONSOLE
	// show IME composition windows
	ConShowComposition();
//#endif
#endif // JPN
}


/*
================
Con_DrawNotify

Draws the last few lines of output transparently over the game top
================
*/

int					notify_text_bottom;
extern float		scr_centertime_off;

#define C_TEXTENTRY_X      (52)
#define C_TEXTENTRY_Y      (5)

#define C_TEXTENTRY_WIDTH  (566)
#define C_TEXTENTRY_HEIGHT (32)

bool	bDrewNotify;
void Con_DrawNotify (void)
{
	bDrewNotify = false;

	notify_text_bottom = 0;
	float fAlpha = 1.0;
	if (scr_centertime_off > 0)
		fAlpha = 0.3;

//	int		x, v;
	int		v;
	char	*text;
//	int		i, j;
	int		i;
	int		time;
	char	*s;
//	int		skip;
//  int   scr_string_width;
	DRAWSTRUCT drawStruct;

	if (!con_font)
		Con_InitFont ();

	// need to position current text below chat graphic if its visible
	if (cls.key_dest != key_message)
	  v = 0;
	else
    v = C_TEXTENTRY_HEIGHT;  // position Y underneath chat graphic
	
	for (i = con.current - NUM_CON_TIMES + 1; i <= con.current; i++)
	{
		if (i < 0)
			continue;

		time = con.times [i % NUM_CON_TIMES];

		if (time == 0)
			continue;

		time = cls.realtime - time;

		if (time > con_notifytime->value*1000)
			continue;

		text = con.text + (i % con.totallines)*con.linewidth;
		qboolean talk = FALSE;
		if (text[0] == '\t')
		{	
			talk = TRUE;
			text[con.linewidth - 1] = '\0';
			text++;
		}
		else
		{
			text[con.linewidth - 1] = '\0';
		}

		drawStruct.nFlags = DSFLAG_BLEND|DSFLAG_ALPHA;
		drawStruct.fAlpha = fAlpha;
		drawStruct.szString = text;
		drawStruct.nXPos = 0;
		drawStruct.nYPos = v;
		if (talk)
		{
			v += scoreboard_font_height;
			drawStruct.pImage = scoreboard_font;
		}
		else
		{
			v += notify_fontHeight;
			drawStruct.pImage = notify_font;
		}

		re.DrawString( drawStruct );
		bDrewNotify = true;
//		v += notify_fontHeight;
	}

	notify_text_bottom = v;


	if (cls.key_dest == key_message)
	{

	// insert 'chat' graphic here

		if (!chat_team)
			drawStruct.pImage = re.RegisterPic( "pics/chat/chat1.tga", NULL, NULL, RESOURCE_INTERFACE );
		else
			drawStruct.pImage = re.RegisterPic( "pics/chat/chat1_team.tga", NULL, NULL, RESOURCE_INTERFACE );

    drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND;
		drawStruct.nXPos = 0;
		drawStruct.nYPos = 0;
		re.DrawPic(drawStruct);

    drawStruct.pImage = re.RegisterPic( "pics/chat/chat2.tga", NULL, NULL, RESOURCE_INTERFACE );
		drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND;
		drawStruct.nXPos = 256;
		drawStruct.nYPos = 0;
		re.DrawPic(drawStruct);

    drawStruct.pImage = re.RegisterPic( "pics/chat/chat3.tga", NULL, NULL, RESOURCE_INTERFACE );
		drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND;
		drawStruct.nXPos = 512;
		drawStruct.nYPos = 0;
		re.DrawPic(drawStruct);

		/*
		if (chat_team)
		{

			drawStruct.nFlags = DSFLAG_BLEND;
			drawStruct.szString = "say_team:";
			drawStruct.nXPos = 8;
			drawStruct.nYPos = v;
			drawStruct.pImage = notify_font;
			skip = 8 + re.DrawString( drawStruct );

		}
		else
		{

			drawStruct.nFlags = DSFLAG_BLEND;
			drawStruct.szString = "say:";
			drawStruct.nXPos = 8;
			drawStruct.nYPos = v;
			drawStruct.pImage = notify_font;
			skip = 8 + re.DrawString( drawStruct );

		}
    */

//		skip = 0;
		s = chat_buffer;
#ifdef JPN
		// check length of actual graphics string instead of string length
//		while ( (scr_string_width = re.StringWidth(s, notify_font, -1)) > C_TEXTENTRY_WIDTH )
		while ( (re.StringWidth(s, notify_font, -1)) > (C_TEXTENTRY_WIDTH - re.StringWidth(ImeBuffer, notify_font, -1) - 20) )
      s++;
#else // JPN
		// check length of actual graphics string instead of string length
//		while ( (scr_string_width = re.StringWidth(s, notify_font, -1)) > C_TEXTENTRY_WIDTH )
		while ( (re.StringWidth(s, notify_font, -1)) > C_TEXTENTRY_WIDTH )
      s++;
#endif // JPN

  	/*
	  scr_string_width = re.StringWidth(s, notify_font, -1);
		
		if (scr_string_width > C_TEXTENTRY_WIDTH)
		{

		  //s += chat_bufferlen - ((viddef.width >> 3)-(skip + 1));

		}
		*/

		//if (chat_bufferlen > (viddef.width >> 3)-(skip+1))
		//	s += chat_bufferlen - ((viddef.width >> 3)-(skip + 1));

//		x = j = 0;

		drawStruct.nFlags   = DSFLAG_BLEND;
		drawStruct.szString = s;
		drawStruct.nXPos    = C_TEXTENTRY_X;
		drawStruct.nYPos    = C_TEXTENTRY_Y;     // always position Y within chat graphic
		drawStruct.pImage   = notify_font;

//		x += re.DrawString( drawStruct );
		re.DrawString( drawStruct );


#ifdef JPN // if (!hConImeStr)
	  // blinking cursor text entry
		char	tempstr[32];
		char    sIme[300];

		if(strlen(ImeBuffer) > 0)
		{
			strcpy(sIme, "„");
			strcat(sIme, ImeBuffer);
			//strcat(sIme, "ƒ");



			// for IME
			drawStruct.nFlags   = DSFLAG_BLEND;
			drawStruct.szString = sIme;
			drawStruct.nXPos    = C_TEXTENTRY_X + re.StringWidth(s, notify_font, -1) + 10;
			drawStruct.nYPos    = C_TEXTENTRY_Y;     // always position Y within chat graphic
			drawStruct.pImage   = notify_font;//notify_font;

			re.DrawString( drawStruct );

		}

	if (!hConImeStr)
	{
  	  if ( (int)(cls.realtime >> 8) & 1)
		  Com_sprintf( tempstr, sizeof(tempstr) ,"%c\n", CONSOLE_CURSOR);
	  else
	      Com_sprintf( tempstr, sizeof(tempstr), "\n");

  }
	else
		Com_sprintf( tempstr, sizeof(tempstr), "\n");


#else // JPN
	  // blinking cursor text entry
		char	tempstr[32];
	  if ( (int)(cls.realtime >> 8) & 1)
// SCG[1/16/00]: 		  sprintf( tempstr, "%c\n", CONSOLE_CURSOR);
		  Com_sprintf( tempstr, sizeof(tempstr) ,"%c\n", CONSOLE_CURSOR);
		else
//      sprintf( tempstr, "\n");
      Com_sprintf( tempstr, sizeof(tempstr), "\n");

#endif // JPN

		drawStruct.nFlags = DSFLAG_BLEND;
		drawStruct.szString = tempstr;
		drawStruct.nXPos = C_TEXTENTRY_X + re.StringWidth(s, notify_font, -1);
		drawStruct.nYPos = C_TEXTENTRY_Y;     // always position Y within chat graphic
		drawStruct.pImage = notify_font;

		re.DrawString( drawStruct );
		

		v += notify_fontHeight;
	}
	
	if (v)
	{
		SCR_AddDirtyPoint (0,0);
		SCR_AddDirtyPoint (viddef.width-1, v);
	}
}

#define CON_PIC_NOT_DRAWN				0
#define CON_PIC_LOAD_PLAQUE				1
#define CON_PIC_GENERIC					2
short Con_DrawConsolePic(int x, int y, int w, int h, qboolean bLoading)
{
	char buf[256];
	float	scale_x;
	float	scale_y;
	int		width_256/*, width_128*/;
	int		pic1_x, pic2_x/*, pic3_x*/;
	int		height;
	DRAWSTRUCT drawStruct;
	//int image_width, image_height;
	qboolean level_load = false;
	short result = CON_PIC_NOT_DRAWN;

	if (bLoading) // loading a level?  check for loadscreen
	{
		if( bGotLoadscreenName == FALSE )
		{
			Cvar_Set("cl_loadscreen","con");
		}
		else
		{


#ifndef TONGUE_ENGLISH

		  // special case: foreign builds, use regular console pic as loading screen
			Cvar_Set("cl_loadscreen","con");

#endif

			loadscreen = Cvar_Get ("cl_loadscreen", "con", 0); // dynamically set via worldspawn info
			Com_sprintf(buf,sizeof(buf), "pics/loadscreens/%s_0.bmp", loadscreen->string); 
			level_load = (FS_LoadFile(buf, NULL) != -1);  // file exists?

			// SCG[9/24/99]: We don't want to draw stuff if we are going between sub levels...
			lastmapname = Cvar_Get ("cl_lastmapname", "lastmapname", 0); // dynamically set via worldspawn info
			int length = strlen( cl_mapname );
			char test = cl_mapname[length - 1];

			result = CON_PIC_LOAD_PLAQUE;
			if( ( test != 'a' ) ||
				( ( test == 'a' ) && ( strncmp( cl_mapname, lastmapname->string, length - 1 ) == 0 ) ) )
			{
				Cvar_Set("cl_loadscreen","con");
				result = CON_PIC_GENERIC;
			}
			else
			{
				// if file not found for specific loadscreen, so resort to the default loadscreen
				if (!level_load) 
				{
					Cvar_Set("cl_loadscreen","gen"); // set generic loadscreen root name
					result = CON_PIC_GENERIC;
				}
			}
		}
	}
	else
	{
	  // not loading a map; show normal console pic
		Cvar_Set("cl_loadscreen","con");
		result = CON_PIC_GENERIC;
	}

	loadscreen = Cvar_Get ("cl_loadscreen", "con", 0);

	scale_x = (viddef.width / 640.0);
	scale_y = (viddef.height / 480.0);
	width_256 = (int)(256.0 * scale_x);
//		width_128 = (int)(128.0 * scale_x);
	pic1_x = 0;
	pic2_x = width_256;
//		pic3_x = pic2_x + width_256;
	height = (int)(224.0 * scale_y);
//		height = (int)(256.0 * scale_y);	// SCG[8/26/99]: Test

	drawStruct.nFlags = ( DSFLAG_SCALE | DSFLAG_PALETTE | DSFLAG_NOMIP ) ;
	drawStruct.fScaleX = scale_x;
	drawStruct.fScaleY = scale_y;

	drawStruct.nYPos = 0;

	int index = 0;

	for( int i = 0; i < 2; i++ )
	{
		drawStruct.nXPos = 0;    // reset

		for( int j = 0; j < 3; j++ )
		{
			Com_sprintf( buf, sizeof(buf), "pics/loadscreens/%s_%d.bmp", loadscreen->string, index++ ); 
			drawStruct.pImage = re.RegisterPic( buf, NULL, NULL, RESOURCE_INTERFACE);
			re.DrawPic( drawStruct );
			drawStruct.nXPos += width_256; // slide over
		}

		drawStruct.nYPos = height;  // next row
	}
	return result;
}


void Con_DrawVersion(int x, int y)
{
	if (!con_font)
		Con_InitFont ();

	char version[64];
	Com_sprintf (version, sizeof(version), "ver: %s", VERSION);
	strupr( version );

	DRAWSTRUCT drawStruct;
	memset(&drawStruct,0,sizeof(drawStruct));
	drawStruct.nFlags = DSFLAG_BLEND;
	drawStruct.szString = version;
	drawStruct.nXPos = x;
	drawStruct.nYPos = y;
	drawStruct.pImage = con_font;
	re.DrawString( drawStruct );
}
/*
================
Con_DrawConsole

Draws the console with the solid background
================
*/
extern qboolean no_draw_plaque;
int CIN_RunningWithView();
void Con_DrawConsole (float frac)
{
	DRAWSTRUCT drawStruct;
	int				i, x, y, w, h;
	int				rows;
	char			*text;
	int				row;
	int				lines;
//	char			version[64];
//	char			tempstr[128];

  lines = viddef.height * frac;
	if (lines <= 0)
		return;

	if (no_draw_plaque)
		return;

	if( con.mode != CON_LOADING )
	{
		bGotLoadscreenName = FALSE;
	}

	if (!con_font)
		Con_InitFont ();

	if (lines > viddef.height)
		lines = viddef.height;

  // draw the background
	short con_pic_result = CON_PIC_NOT_DRAWN;
	if( !CIN_RunningWithView() )
	{
		if (con.mode == CON_QUICK )
		{
			lines = 110 * (viddef.height / 480.0); 

			drawStruct.pImage = re.RegisterPic( "pics/dkqcon2.pcx", &w, &h, RESOURCE_INTERFACE );
			drawStruct.nFlags = DSFLAG_SCALE | DSFLAG_PALETTE;
			drawStruct.fScaleX = ( float ) viddef.width / 320.0;
			drawStruct.fScaleY = ( float ) viddef.height / ( float ) 256.0;
			drawStruct.nXPos = 0;
			drawStruct.nYPos = 0;
			re.DrawPic( drawStruct );

			drawStruct.pImage = re.RegisterPic( "pics/dkqcon.pcx", NULL, NULL, RESOURCE_INTERFACE );
			drawStruct.nXPos = w * drawStruct.fScaleX;
			drawStruct.nYPos = 0;
			re.DrawPic( drawStruct );
		}
		else
		{
/*			if (con.mode == CON_LOADING)
			{
				Con_DrawConsolePic(0, -viddef.height+lines, viddef.width, viddef.height, true);
			}
			else
			{
				Con_DrawConsolePic(0, -viddef.height+lines, viddef.width, viddef.height, false);
			}*/
			con_pic_result = Con_DrawConsolePic(0, -viddef.height+lines, viddef.width, viddef.height, (con.mode == CON_LOADING));

		}
	}

	SCR_AddDirtyPoint (0,0);
	SCR_AddDirtyPoint (viddef.width-1,lines-1);

//	if (con.mode == CON_LOADING)
//		return;

	if( Cvar_VariableValue( "console" ) == 0 )
	{
		return;
	}

// draw the text
	if (con_pic_result == CON_PIC_LOAD_PLAQUE)
		return;
//	if (scr_draw_progress)
//		return;

	con.vislines = lines;

	if( CIN_RunningWithView() )
	{
		rows = 0;

		y = 0;
	}
	else if (con.mode == CON_QUICK)
	{
		rows = (lines / con_fontHeight) - 1;		// rows of text to draw

		y = lines - con_fontHeight;
	}
	else
	{
		rows = (lines - con_fontHeight) / con_fontHeight;		// rows of text to draw

		y = lines - con_fontHeight * 2;
		Con_DrawVersion(viddef.width - 200,y);
/*
		Com_sprintf (version, sizeof(version), "ver: %s", VERSION);
		strupr( version );

		drawStruct.nFlags = DSFLAG_BLEND;
		drawStruct.szString = version;
		drawStruct.nXPos = viddef.width - 200;
		drawStruct.nYPos = lines - 15;
		drawStruct.pImage = con_font;
		re.DrawString( drawStruct );*/
	}

	// draw from the bottom up
	if (con.display != con.current)
	{
	// draw arrows to show the buffer is backscrolled
		char tempstr[1024];// yes, it's huge.. but better than too small.
		for (x = 0; x < con.linewidth; x ++)
		{
			tempstr[x] = '^';
//			re.DrawChar ( (x+1)<<3, y, '^');
//			re.Draw_AlphaChar ((x + 1) << 3, y, '^', 0, 0, 0, 0, vec3_identity, 0, con_font);
		}
		tempstr[con.linewidth - 1] = '\0';
		drawStruct.nFlags = DSFLAG_BLEND;
		drawStruct.szString = tempstr;
		drawStruct.nXPos = 0;
		drawStruct.nYPos = y;
		drawStruct.pImage = con_font;
		re.DrawString( drawStruct );

		y -= con_fontHeight;
		rows--;
	}
	
	row = con.display;
	for (i=0 ; i < rows ; i++, y -= con_fontHeight, row--)
	{
		if (row < 0)
			break;
		if (con.current - row >= con.totallines)
			break;		// past scrollback wrap point
			
		text = con.text + (row % con.totallines)*con.linewidth;

//		for (x = 0, j = 0; x < con.linewidth ; x++)
//		{
			//	re.DrawChar ( (x+1)<<3, y, text[x]);
//			j += re.Draw_AlphaChar (j, y, text[x], 0, 0, 0, 0, vec3_identity, 0, con_font);
//		}
//==================================
// consolidation change: SCG 3-15-99
		text[con.linewidth - 1] = '\0';
		drawStruct.nFlags = DSFLAG_BLEND;
		drawStruct.szString = text;
		drawStruct.nXPos = 0;
		drawStruct.nYPos = y;
		drawStruct.pImage = con_font;
		re.DrawString( drawStruct );
// consolidation change: SCG 3-15-99
//==================================
	}

	// draw the input prompt, user text, and cursor if desired
	Con_DrawInput ();
}
#ifdef JPN
void ImeSetMode3(BOOL mode)
{
	HIMC hIMC;

    if ( !( hIMC = ImmGetContext( cl_hwnd ) ) )
        return;

	ImmSetOpenStatus(hIMC, mode);

	ImmReleaseContext(cl_hwnd, hIMC);
}
#endif // JPN