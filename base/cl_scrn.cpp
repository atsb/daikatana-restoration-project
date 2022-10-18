// added for Japanese version
#ifdef JPN
#include <windows.h>
#endif
// cl_scrn.c -- master for refresh, status bar, console, chat, notify, etc

#include	"client.h"
#include	"daikatana.h"
//#include	"dk_model.h"
//#include	"dk_menu.h"
#include	"dk_gce_main.h"
#include	"dk_scoreboard.h"
//#include  "cl_icons.h"
#include  "cl_inventory.h"
#include  "cl_sidekick.h"
#include  "l__language.h"


#define SCR_VIEWSIZE_NOSKILLS   (110)
#define SCR_VIEWSIZE_NOSTATUS   (120)
#define SCR_VIEWSIZE_MAXSETTING (SCR_VIEWSIZE_NOSTATUS) // set to highest #define

//#define	SCR_MAX_PROGRESS_PICS	4


#define SCR_CHAR_DELAY_MSEC		(5)
#define SCR_CHAR_DELAY_SEC		(SCR_CHAR_DELAY_MSEC * 0.01)

#define INFO_ALPHA(alpha)			((!scr_draw_progress) && (alpha > 0.0f) ? 0.7 : 0)
#define MODEL_ALPHA(alpha)			((!scr_draw_progress) && (alpha > 0.0f) ? 1.0 : 0)


float		scr_con_current;	// aproaches scr_conlines at scr_conspeed
float		scr_conlines;		// 0.0 to 1.0 lines of console to display


qboolean	scr_initialized;		// ready to draw
qboolean  override_showhuds = false;

int			scr_draw_loading;

void *sbar_font_g = NULL;     // green font
void *sbar_font_r = NULL;     // red font
void *scoreboard_font = NULL; // scoreboard font
void *statbar_font = NULL;    // status bar font
extern void	*con_font;        // from console.cpp


int			      scr_draw_progress;
//static	int		progress_pic_count = 0;


//static	char	*progress_pics [SCR_MAX_PROGRESS_PICS]= 
//				{"prog1", "prog2", "prog3", "prog4"};

vrect_t		scr_vrect;		// position of render window on screen

extern cvar_t *cl_dmhints;
cvar_t		*scr_viewsize;
cvar_t		*scr_conspeed;
cvar_t		*scr_centertime;
//cvar_t		*scr_showturtle;
cvar_t		*scr_showpause;
//cvar_t		*scr_printspeed;

cvar_t		*scr_netgraph;
cvar_t		*scr_timegraph;
cvar_t		*scr_debuggraph;
cvar_t		*scr_graphheight;
cvar_t		*scr_graphscale;
cvar_t		*scr_graphshift;
//cvar_t		*scr_drawall;

cvar_t		*scr_letterbox;  // screen displayed in a letterbox aspect ratio
//cvar_t		*scr_textdelay;
cvar_t		*scr_consize;    // percentage of screen console fills

cvar_t    *statusbar_alpha;
cvar_t    *subtitle_cvar;

typedef struct
{
	int		x1, y1, x2, y2;
} dirty_t;

dirty_t		scr_dirty, scr_old_dirty[2];

// crosshair definitions
crosshair_t crosshairList[MAX_CROSSHAIRS][MAX_CROSSHAIR_TYPES]={
	{
		{"pics/crosshair/ch_notarget.tga",0,0},
		{"pics/crosshair/ch_shoot.tga",0,0},
		{"pics/crosshair/ch_use.tga",0,0},
		{"pics/crosshair/ch_center.tga",0,0}
	},
	{
		{"pics/crosshair/ch_notarget2.tga",0,0},
		{"pics/crosshair/ch_shoot2.tga",0,0},
		{"pics/crosshair/ch_use2.tga",0,0},
		{"pics/crosshair/ch_center2.tga",0,0}
	},
	{
		{"pics/crosshair/ch_notarget3.tga",0,0},
		{"pics/crosshair/ch_shoot3.tga",0,0},
		{"pics/crosshair/ch_use3.tga",0,0},
		{"pics/crosshair/ch_center3.tga",0,0}
	},
	{
		{"pics/crosshair/ch_notarget4.tga",0,0},
		{"pics/crosshair/ch_shoot4.tga",0,0},
		{"pics/crosshair/ch_use4.tga",0,0},
		{"pics/crosshair/ch_center4.tga",0,0}
	},
};
//crosshair_t *crosshair;


// screen alpha fx
alphafx_t  flap_mikiko;
alphafx_t  flap_superfly;
alphafx_t  invmode_hud;			// weapons/sidekick
alphafx_t  skill_hud;			// skill bar
alphafx_t  status_hud;			// status bar

void	*scr_centerFont;

// local prototypes
void SCR_TimeRefresh2_f (void);
void SCR_TimeRefresh_f (void);
void SCR_Loading_f (void);
void SCR_DrawStats (void);
void SCR_RegisterStatus (void);

int progress_count;
// local prototypes ///////////////////////////////////////////////////////////

void global_hud_updates(alphafx_t * hud, float * model_alpha, float max_alpha);


// ////////////////////////////////////////////////////////////////////////////

bool SCR_ScreenDisabled()
{
	if (dedicated->value)
	{
		return true;	// screen doesn't work in dedicated!
	}
	else
		return (cls.disable_screen > 0);
}

void SCR_InitVars(void)
// initialize local client/interface variables
{
	// reset xplevel vars
	xplevelup_mode		= false;
	xplevelup_selection	= 0;
	xplevelup_timer		= 0.0f;
	xplevelup_blink		= false;
	
	cl_death_gib		= false;
	cl_savegem_count	= 0;
	
	cl_gameover_time	= 0.0f;
	cl_gameover_alpha	= 0.0f;

	cl_theend_time		= 0.0f;
	cl_theend_alpha		= 0.0f;
	
	memset(&flap_mikiko,0,sizeof(alphafx_t));
	memset(&flap_superfly,0,sizeof(alphafx_t));
	memset(&invmode_hud,0,sizeof(alphafx_t));
	memset(&skill_hud,0,sizeof(alphafx_t));
	memset(&status_hud,0,sizeof(alphafx_t));
	
	if (scr_viewsize->value < SCR_VIEWSIZE_NOSKILLS) // huds initially up
	{
		skill_hud.alpha   = INVMODE_ALPHA_DEFAULT; // set maximum alpha
		
		invmode_hud.alpha = INVMODE_ALPHA_DEFAULT; // set maximum alpha
		inventory_alpha   = invmode_hud.alpha;     // model alpha
	}
	else
		inventory_alpha = 0.0f;                     // model alpha
	
	if (scr_viewsize->value < SCR_VIEWSIZE_NOSTATUS)  // status hud initially up?
		status_hud.alpha = statusbar_alpha->value;      // set maximum alpha
	
}



/*
===============================================================================

  BAR GRAPHS
  
	===============================================================================
*/

/*
==============
CL_AddNetgraph

  A new packet was just parsed
  ==============
*/
void CL_AddNetgraph (void)
{
	int		i;
	int		in;
	int		ping;
	
	// if using the debuggraph for something else, don't
	// add the net lines
	if (scr_debuggraph->value || scr_timegraph->value)
		return;
	
	for (i=0 ; i<cls.netchan.dropped ; i++)
		SCR_DebugGraph (30, 0x40);
	
	for (i=0 ; i<cl.surpressCount ; i++)
		SCR_DebugGraph (30, 0xdf);
	
	// see what the latency was on this packet
	in = cls.netchan.incoming_acknowledged & (CMD_BACKUP-1);
	ping = cls.realtime - cl.cmd_time[in];
	ping /= 30;
	if (ping > 30)
		ping = 30;
	SCR_DebugGraph (ping, 0xd0);
}

typedef struct
{
	float	value;
	int		color;
} graphsamp_t;

static	int			current = 0;
static	graphsamp_t	values[1024];

/*
==============
SCR_DebugGraph
==============
*/
void SCR_DebugGraph (float value, int color)
{
	values[current&1023].value = value;
	values[current&1023].color = color;
	current++;
}

/*
==============
SCR_DrawDebugGraph
==============
*/
void SCR_DrawDebugGraph (void)
{
	//==================================
	// consolidation change: SCG 3-15-99
	int		a, x, y, w, i, h;
	float	v;
	CVector	rgbColor;
	
	//
	// draw the graph
	//
	w = scr_vrect.width;
	
	x = scr_vrect.x;
	y = scr_vrect.y+scr_vrect.height;
	
	rgbColor.x = rgbColor.y = rgbColor.z = 1;
	re.DrawFill (x, y-scr_graphheight->value, w, scr_graphheight->value, rgbColor, 1.0 );
	
	for (a=0 ; a<w ; a++)
	{
		i = (current-1-a+1024) & 1023;
		v = values[i].value;
		v = v*scr_graphscale->value + scr_graphshift->value;
		
		if (v < 0)
			v += scr_graphheight->value * (1+(int)(-v/scr_graphheight->value));
		h = (int)v % (int)scr_graphheight->value;
		re.DrawFill (x+w-1-a, y - h, 1,	h, rgbColor, 1.0 );
	}
	// consolidation change: SCG 3-15-99
	//==================================
}

/*
===============================================================================

  CENTER PRINTING
  
	===============================================================================
*/

char		scr_centerstring[1024];
int			scr_centertime_start;		//	cl.time when message started
float		scr_centertime_off;
int			scr_centertime_end;			//	cl.time when message will end
int			scr_center_lines;
//int			scr_erase_center;
//int			scr_string_width;			//	width of string in pixels
int			scr_font_height;

char		scr_motd[1024];
int			scr_motd_centertime_start;
float		scr_motd_centertime_off;
int			scr_motd_centertime_end;
int			scr_motd_lines;

///////////////////////////////////////////////////////////////////////////////
//	SCR_InitFont
//
///////////////////////////////////////////////////////////////////////////////

void	SCR_InitFont (void)
{
	if (!scr_centerFont)
	{
		//	FIXME:	hack to make software work until proportional fonts are fixed
		if (!stricmp (Cvar_VariableString ("vid_ref"), "soft"))
		{
			scr_font_height = 8;
			scr_centerFont = NULL;
		}
		else
		{
			scr_centerFont = re.LoadFont ("scr_cent");
			
			re.FontGetFontData (scr_centerFont, &scr_font_height, NULL, NULL, NULL, NULL);
		}
	}
}

/*
==============
SCR_CenterPrint

  Called for important messages that should stay in the center of the screen
  for a few moments
  ==============
*/
void SCR_CenterPrint (char *str, float msg_time, bool bmotd)
{
	if (bmotd)
	{
		char	*s;
		int		len;
		
		//	might have had a vid_restart
		if (scr_centerFont == NULL)
			scr_centerFont = re.LoadFont ("scr_cent");
		
		strncpy (scr_motd, str, sizeof(scr_motd)-1);
		len = strlen (scr_motd);
		
		//	Nelno:	time is now passed to this function 
		//	scr_textdelay is a modifier for how long the text should be displayed
		//scr_centertime_off = msg_time * scr_textdelay->value;//scr_centertime->value;
		scr_motd_centertime_off = msg_time;
		scr_motd_centertime_off += SCR_CHAR_DELAY_SEC * (float)(len + 2);	//	adjust for per-char delays
		scr_motd_centertime_start = cl.time;
		scr_motd_centertime_end = cl.time + (int)(scr_motd_centertime_off * 1000.0);
		
		re.FontGetFontData (scr_centerFont, &scr_font_height, NULL, NULL, NULL, NULL);
		
		// count the number of lines for centering
		scr_motd_lines = 1;
		s = str;
		while (*s)
		{
			if (*s == '\n')
				scr_motd_lines++;
			s++;
		}
		Com_Printf (str);
	}
	else
	{
		extern bool	bDrewNotify;
		if (!bDrewNotify)
		{
			char	*s;
			int		len;
			
			//	might have had a vid_restart
			if (scr_centerFont == NULL)
				scr_centerFont = re.LoadFont ("scr_cent");
			
			strncpy (scr_centerstring, str, sizeof(scr_centerstring)-1);
			len = strlen (scr_centerstring);
			
			//	Nelno:	time is now passed to this function 
			//	scr_textdelay is a modifier for how long the text should be displayed
			//scr_centertime_off = msg_time * scr_textdelay->value;//scr_centertime->value;
			scr_centertime_off = msg_time;
			scr_centertime_off += SCR_CHAR_DELAY_SEC * (float)(len + 2);	//	adjust for per-char delays
			scr_centertime_start = cl.time;
			scr_centertime_end = cl.time + (int)(scr_centertime_off * 1000.0);
			
			//scr_string_width = re.StringWidth (scr_centerstring, scr_centerFont, len);
			
			re.FontGetFontData (scr_centerFont, &scr_font_height, NULL, NULL, NULL, NULL);
			
			// count the number of lines for centering
			scr_center_lines = 1;
			s = str;
			while (*s)
			{
				if (*s == '\n')
					scr_center_lines++;
				s++;
			}
			Com_Printf (str);
			Con_ClearNotify ();
		}
		else
		{
			Com_Printf (str);

		}
	}
}

int scr_centerstring_bottom;
extern int notify_text_bottom;	// console.cpp
void SCR_DrawCenterString (void)
{
	char	*start;
	int		l;
	int		x, y;
	int		remaining;
	int		charWidth, pixelWidth;
	DRAWSTRUCT	drawStruct;
	
	//	might have had a vid_restart
	if (scr_centerFont == NULL)
		scr_centerFont = re.LoadFont ("scr_cent");
	
	// the finale prints the characters one at a time
	remaining = 9999;
	
	//scr_erase_center = 0;
	start = scr_centerstring;
	
/*
	if (scr_center_lines <= 4)
		y = viddef.height*0.35;
	else
		y = 48;
*/	
/*
	if (notify_text_bottom)
		y = notify_text_bottom;
	else*/
		y = 16 * ( ( float )viddef.height / 480.0 ); // SCG[11/29/99]: Per Romero's request

	int charcount;
	char oldbreakchar;
	do	
	{
		charcount = 0;
		// scan the width of the line
		for (l = 0, pixelWidth = 0; l < 80 && pixelWidth < viddef.width; l++, pixelWidth += charWidth)
		{
			if (start[l] == '\n' || !start[l])
				break;

// Encompass MarkMa 040599
#ifdef	JPN	// JPN
// added by yokoyama (charcount)
				if(IsDBCSLeadByte(start[l]))
				{ 
					charWidth = re.StringWidth (&start [l++], scr_centerFont, 2);
					charcount +=2; // added by yokoyama
				} 
				else
				{ 
#endif	// JPN
// Encompass MarkMa 040599
			
			charWidth = re.StringWidth (&start [l], scr_centerFont, 1);
			charcount++;
#ifdef JPN //JPN added by yokoyama
				}
#endif  // JPN added by yokoyama
		}
		
		//		pixelWidth = re.StringWidth (start, scr_centerFont, l);
		x = (viddef.width - pixelWidth) >> 1;
		
		SCR_AddDirtyPoint (x, y);
		
		//		for (j=0 ; j<l ; j++, x += charWidth)
		//		{
		//			re.DrawChar (x, y, start[j]);	
		//			charWidth = re.Draw_AlphaChar (x, y, start [j], scr_centertime_start, scr_centertime_end, cl.time,  j * SCR_CHAR_DELAY_MSEC, vec3_identity, ACF_BEGINFLARE | ACF_ENDFADE | ACF_BEGINFADE, scr_centerFont);
		//			charWidth = re.Draw_AlphaString (x, y, start, scr_centertime_start, scr_centertime_end, cl.time,  j * SCR_CHAR_DELAY_MSEC, vec3_identity, ACF_BEGINFLARE | ACF_ENDFADE | ACF_BEGINFADE, scr_centerFont);
		//			if (!remaining--)
		//				return;
		//		}
		//==================================
		// consolidation change: SCG 3-15-99
		oldbreakchar = start[charcount];
		start[charcount] = NULL;
		drawStruct.nFlags = DSFLAG_BLEND;
		drawStruct.szString = start;
		drawStruct.nXPos = x;
		drawStruct.nYPos = y;
		drawStruct.pImage = scr_centerFont;
		
		float	total_time = scr_centertime_end - scr_centertime_start;
		float	current_time = cl.time - scr_centertime_start;
		float	half_time = total_time /2 ;
		float	quarter_time = total_time / 4;
		float	start_end_effect = ( quarter_time + half_time );
		float	fStringAlpha, fFlareAlpha;

		fFlareAlpha = 0;
		fStringAlpha = 0;
		if( current_time < quarter_time )
		{
			fStringAlpha = current_time / quarter_time;
			
			int stop_fade_in = quarter_time / 4;
			
			if( current_time < stop_fade_in ) 
			{
				fFlareAlpha = current_time / stop_fade_in;
			}
			else
			{
				fFlareAlpha = ( current_time - stop_fade_in ) / ( quarter_time - stop_fade_in );
				fFlareAlpha = 1.0 - fFlareAlpha;
			}
		}
		else if( current_time > start_end_effect )
		{
			fStringAlpha = ( current_time - start_end_effect ) / ( total_time - start_end_effect );
			fStringAlpha = 1.0 - fStringAlpha;
		}
		else
		{
			drawStruct.nFlags &= ~DSFLAG_ALPHA;
		}
		
		// now draw the flares
// SCG[1/31/00]: removed per Romero's request
/*
		if( fFlareAlpha > 0.0 )
		{
			drawStruct.fAlpha = fFlareAlpha;
			re.DrawStringFlare( drawStruct );
		}
*/		
		drawStruct.fAlpha = fStringAlpha;
		remaining -= re.DrawString( drawStruct ) + 1;
		
		
		if( !remaining )
			return;
		// consolidation change: SCG 3-15-99
		//==================================
		
		SCR_AddDirtyPoint (x, y + scr_font_height);
		
		y += scr_font_height;
		
/*		while (*start && *start != '\n')
			start++;*/
		start[charcount] = oldbreakchar;
		start += charcount;
		
		if (!*start)
			break;
		start++;		// skip the \n
	} while (1);

	scr_centerstring_bottom = y;
}

void SCR_DrawMotdString (void)
{
	scr_motd_centertime_off -= cls.frametime;
	if (scr_motd_centertime_off <= 0)
		return;

	char	*start;
	int		l;
	int		x, y;
	int		remaining;
	int		charWidth, pixelWidth;
	DRAWSTRUCT	drawStruct;
	
	//	might have had a vid_restart
	if (scr_centerFont == NULL)
		scr_centerFont = re.LoadFont ("scr_cent");
	
	// the finale prints the characters one at a time
	remaining = 9999;
	
	start = scr_motd;
	
	y = viddef.height / 2; 

	int charcount;
	char oldbreakchar;
	do	
	{
		charcount = 0;
		// scan the width of the line
		for (l = 0, pixelWidth = 0; l < 80 && pixelWidth < viddef.width; l++, pixelWidth += charWidth)
		{
			if (start[l] == '\n' || !start[l])
				break;

// Encompass MarkMa 040599
#ifdef	JPN	// JPN
// added by yokoyama (charcount)
				if(IsDBCSLeadByte(start[l]))
				{ 
					charWidth = re.StringWidth (&start [l++], scr_centerFont, 2);
					charcount +=2; // added by yokoyama
				} 
				else
				{ 
#endif	// JPN
// Encompass MarkMa 040599
			
			charWidth = re.StringWidth (&start [l], scr_centerFont, 1);
			charcount++;
#ifdef JPN //JPN added by yokoyama
				}
#endif  // JPN added by yokoyama
		}
		
		//		pixelWidth = re.StringWidth (start, scr_centerFont, l);
		x = (viddef.width - pixelWidth) >> 1;
		
		SCR_AddDirtyPoint (x, y);
		
		oldbreakchar = start[charcount];
		start[charcount] = NULL;
		drawStruct.nFlags = DSFLAG_BLEND;
		drawStruct.szString = start;
		drawStruct.nXPos = x;
		drawStruct.nYPos = y;
		drawStruct.pImage = scr_centerFont;
		
		float	total_time = scr_motd_centertime_end - scr_motd_centertime_start;
		float	current_time = cl.time - scr_motd_centertime_start;
		float	half_time = total_time /2 ;
		float	quarter_time = total_time / 4;
		float	start_end_effect = ( quarter_time + half_time );
		float	fStringAlpha, fFlareAlpha;

		fFlareAlpha = 0;
		fStringAlpha = 0;
		if( current_time < quarter_time )
		{
			fStringAlpha = current_time / quarter_time;
			
			int stop_fade_in = quarter_time / 4;
			
			if( current_time < stop_fade_in ) 
			{
				fFlareAlpha = current_time / stop_fade_in;
			}
			else
			{
				fFlareAlpha = ( current_time - stop_fade_in ) / ( quarter_time - stop_fade_in );
				fFlareAlpha = 1.0 - fFlareAlpha;
			}
		}
		else if( current_time > start_end_effect )
		{
			fStringAlpha = ( current_time - start_end_effect ) / ( total_time - start_end_effect );
			fStringAlpha = 1.0 - fStringAlpha;
		}
		else
		{
			drawStruct.nFlags &= ~DSFLAG_ALPHA;
		}
		
		drawStruct.fAlpha = fStringAlpha;
		remaining -= re.DrawString( drawStruct ) + 1;
		
		
		if( !remaining )
			return;
		// consolidation change: SCG 3-15-99
		//==================================
		
		SCR_AddDirtyPoint (x, y + scr_font_height);
		
		y += scr_font_height;
		
		start[charcount] = oldbreakchar;
		start += charcount;
		
		if (!*start)
			break;
		start++;		// skip the \n
	} while (1);
}

void SCR_CheckDrawCenterString (void)
{
	SCR_DrawMotdString();

	scr_centertime_off -= cls.frametime;
	scr_centerstring_bottom = 0;
	if (scr_centertime_off <= 0)
		return;
	
	SCR_DrawCenterString ();
}


// **************************************************************************************
// **************************************************************************************
// **************************************************************************************

char		scr_subtitle_str[1024];
float		scr_subtitle_end = 0.0f;
int     scr_subtitle_font_height; 
//int     scr_subtitle_lines;


#define TEXT_BUFFER_SIZE	( 1024 )
void SCR_SubtitlePrint(char *str, float msg_time)
// display subtitle text -- similar to center print
// set up the initial variables to draw string
{
	//char *s;
	void	*subtitle_font;

	if( Cvar_VariableValue( "subtitles" ) == 0 )
	{
		return;
	}

	// in case of vid_restart?
	//if (statbar_font == NULL)
	//  statbar_font = re.LoadFont("statbar_font");
	if (con_font == NULL)
		con_font = re.LoadFont("int_font");
	
	// this may change, depending on cinematic vs. in-game subtitles
	subtitle_font = con_font;

	if( str != NULL )
	{
		strncpy(scr_subtitle_str, str, sizeof(scr_subtitle_str) - 1);
		Com_Printf(str);
		Con_ClearNotify();
	}
	else
	{
		memset( &scr_subtitle_str, 0, sizeof( scr_subtitle_str ) );
	}
	
	scr_subtitle_end = msg_time;
	
	// retrieve height
	re.FontGetFontData (subtitle_font, &scr_subtitle_font_height, NULL, NULL, NULL, NULL);
	
	scr_subtitle_font_height *= 0.90; // reduce blank spaces between lines
	
/*
	scr_subtitle_lines = 1;
	s = str;
	while (*s)
	{
		if (*s == '\n')
			scr_subtitle_lines++;
		s++;
	}
*/	
}


#define SUBTITLE_CINE_Y  ( 68)
#define SUBTITLE_GAME_Y  (160)

void SCR_DrawSubtitleString(void)
// draw the subtitle string centered on the screen
{
	char	tokenized_str[1024];
	char  *token;
	int		x, y;
	DRAWSTRUCT	drawStruct;
	void *subtitle_font;
#ifdef JPN // changed by yokoyama for Japanese version.
	char tokenBuffer[1024];
#endif
	
	//Com_Printf("scr_subtitle_end=%d\n",(int)scr_subtitle_end);
	
	// in case of vid_restart?
	//if (statbar_font == NULL)
	//  statbar_font = re.LoadFont("statbar_font");
	if (con_font == NULL)
		con_font = re.LoadFont("int_font");
	
	// this may change, depending on cinematic vs. in-game subtitles
	subtitle_font = con_font;
	
	// copy original subtitle string, and tokenize this copied string
	strncpy(tokenized_str,scr_subtitle_str,sizeof(scr_subtitle_str) - 1); 
	
	
	// base x coords on cinematic subtitle or in-game subtitle
	float scale_y = (float)viddef.height / 480.0;
	
	// currently, a '#' symbol will designate a cinematic subtitle
	if (tokenized_str[0] != '#')
	{
		// cinematic positioning
		y = viddef.height - (SUBTITLE_CINE_Y * scale_y);
	}
	else
	{
		// ingame positioning
		y = viddef.height - (SUBTITLE_GAME_Y * scale_y);
		tokenized_str[0] = ' ';                            // blank out print code
	}
	
	
	
	drawStruct.nFlags = DSFLAG_BLEND;  // note: no alpha here
	drawStruct.pImage = subtitle_font;
	
	token = strtok(tokenized_str, "\n"); // search for linebreak
    
#ifdef JPN // chaned by yokoyama for Japanese version.
	strcpy(tokenBuffer, token);
	while (token != NULL)
	{
		UINT len = strlen(tokenBuffer);

		if((UINT)tokenBuffer[len -1] == 0x0d)
			tokenBuffer[len - 1] = 0;

		x = (viddef.width - re.StringWidth(tokenBuffer, subtitle_font, -1)) >> 1;
		
		drawStruct.szString = tokenBuffer;
		drawStruct.nXPos = x;
		drawStruct.nYPos = y;
		
		re.DrawString( drawStruct ); // draw the string
		
		y += scr_subtitle_font_height;
		
		token = strtok( NULL, "\n");

		if(token != NULL)
			strcpy(tokenBuffer, token);
	}

#else // JPN
	while (token != NULL)
	{
		x = (viddef.width - re.StringWidth(token, subtitle_font, -1)) >> 1;
		
		drawStruct.szString = token;
		drawStruct.nXPos = x;
		drawStruct.nYPos = y;
		
		re.DrawString( drawStruct ); // draw the string
		
		y += scr_subtitle_font_height;
		
		token = strtok( NULL, "\n");
	}
#endif // JPN	
}



void SCR_CheckDrawSubtitleString(void)
// update timer and check for subtitle drawing
{
	if( Cvar_VariableValue( "subtitles" ) )  // only draw subtitles if toggled on
	{
		scr_subtitle_end -= cls.frametime;
		
		if (scr_subtitle_end <= 0)
			return;
		
		SCR_DrawSubtitleString();
	}
}



// **************************************************************************************
// **************************************************************************************
// **************************************************************************************





//=============================================================================

//#if 1

//---------------------------------------------------------------------------
// R_SetVrect() - from q1 code
//---------------------------------------------------------------------------
void R_SetVrect(vrect_t *pvrectin, vrect_t *pvrect, int lineadj)
{
	int     h;
	float   size;
	
	if( DKM_InMenu() )
		size = 100;	// full screen
	else
		size = (scr_viewsize->value > 100) ? 100 : scr_viewsize->value; // 100% fullscreen 
	
	size /= 100;  // scr_viewsize is a percentage, with 100% = fullsize
	
	h = pvrectin->height - lineadj;
	
	pvrect->width = pvrectin->width * size;
	
	if (pvrect->width < 96)
	{
		size = 96.0f / pvrectin->width;
		pvrect->width = 96;              // min for icons       ??????????
	}
	
	//truncate width to multiple of 8
	pvrect->width &= ~7;
	
	pvrect->height = pvrectin->height * size;
	
	if (pvrect->height > pvrectin->height - lineadj)
		pvrect->height = pvrectin->height - lineadj;
	
	//truncate height to multiple of 2
	pvrect->height &= ~1;
	
	pvrect->x = pvrectin->x + ((pvrectin->width - pvrect->width) >> 1);
	pvrect->y = pvrectin->y + ((h - pvrect->height) >> 1);
}

//---------------------------------------------------------------------------
// SCR_CalcVrect() - from q1 code
//---------------------------------------------------------------------------

#define CIN_ASPECT_RATIO 1.85f

void SCR_CalcVrect() 
{
	vrect_t     vrect;
	int sb_lines;
	
	// bound viewsize
	if (scr_viewsize->value < 40) 
		Cvar_Set("viewsize", "40");
	else
		if (scr_viewsize->value > SCR_VIEWSIZE_MAXSETTING ) 
		{
			char tempstr[32];
			
//			sprintf(tempstr,"%d",SCR_VIEWSIZE_MAXSETTING);
			Com_sprintf(tempstr,sizeof(tempstr), "%d",SCR_VIEWSIZE_MAXSETTING);
			Cvar_Set("viewsize", tempstr);
		}
		
		//determine how many lines will be taken up by the status bar.
		
		/*
		//check if a cinematic is running or another view feature that requires
		// letter box mode with no status bar
		if ((CIN_RunningWithView() == 1) || scr_letterbox->value)
        sb_lines = 0;				//no status bar in letterbox mode
		else if (size >= 130) 
        sb_lines = 0;              // no status bar at all
		else if (size >= 120) 
        sb_lines = 50;             // just bottom bar
		else if (size >= 100) 
        sb_lines = 50+50;          // bottom and mid
		else 
        sb_lines = 50+50+16;       // all status info
		*/
		
		sb_lines = 0; // 2.26 dsn  override old status bar relations
		
		// these calculations mirror those in R_Init() for r_refdef, but take no
		// account of water warping
		vrect.x      = 0;
		vrect.y      = 0;
		vrect.width  = viddef.width;
		vrect.height = viddef.height;
		
		//check if a cinematic is running and adjust the vrect so that our view will be letterboxed.
//		if( ( ( CIN_RunningWithView() == 1 ) || ( scr_letterbox->value ) ) && ( DKM_InMenu() == false ) )
		if( ( ( CIN_RunningWithView() == 1 ) || (cl.frame.playerstate.rdflags & RDF_LETTERBOX) ) && ( DKM_InMenu() == false ) )
		{
			//we want to use the full width, and go with a 2:1 aspect ratio.
			//compute the ideal height.
			int height;
			height = vrect.width / CIN_ASPECT_RATIO;
			
			//check if this height is too great
			if (height > vrect.height) 
			{
				height = vrect.height;                    //use whatever height we have.
				vrect.width = height * CIN_ASPECT_RATIO;  //cut our width down to 2 times our height.
			}
			
			vrect.height = height;  //set the rectangle height.
			
			//compute the retangle offsets.
			vrect.x = (viddef.width - vrect.width) >> 1;
			vrect.y = (viddef.height - vrect.height) >> 1;
		}
		
		R_SetVrect(&vrect, &scr_vrect, sb_lines);
		
		// guard against going from one mode to another that's less than half the
		// vertical resolution
		if (scr_con_current > viddef.height) 
			scr_con_current = viddef.height;
		
}


/*
=================
SCR_SizeUp_f

  Keybinding command
  =================
*/
void SCR_SizeUp_f (void)
{
	Cvar_SetValue ("viewsize",scr_viewsize->value+10);
	
	check_weapon_hud(); // check to see if the weapon hud needs to display
	
	if (scr_viewsize->value >= SCR_VIEWSIZE_NOSTATUS)  // status hud no longer active?
	{
		status_hud.direction = -1;      // fade out direction
		status_hud.fade_time = cl.time; // fade out NOW
	}
	
}


/*
=================
SCR_SizeDown_f

  Keybinding command
  =================
*/
void SCR_SizeDown_f (void)
{
	Cvar_SetValue ("viewsize",scr_viewsize->value-10);
	
	check_weapon_hud(); // check to see if the weapon hud needs to display
	
	if (scr_viewsize->value < SCR_VIEWSIZE_NOSTATUS)  // status hud no longer active?
	{
		status_hud.direction = 1;      // fade in direction
	}
	
}

/*
=================
SCR_Sky_f

  Set a specific sky and rotation speed
  =================
*/
void SCR_Sky_f (void)
{
    switch (GetArgc())
    {
    case 2:
        // set sky by name, no clouds
        re.SetSky (GetArgv(1), NULL);
        break;
		
    case 3:
        // set sky and clouds by name
        re.SetSky (GetArgv(1), GetArgv(2));
        break;
		
    case 4:
        // set everything by number from worldspawn data
        re.SetSky (NULL, NULL, atoi(GetArgv(3)));
        break;
		
    default:
		Com_Printf ("Usage 1: skyname <cloudname>\n");
		Com_Printf ("Usage 2: 0 0 skynum\n");
		break;
	}
}

//============================================================================

/*
==================
SCR_Init
==================
*/
void SCR_Init (void)
{
	scr_viewsize    = Cvar_Get("viewsize",        "100",  CVAR_ARCHIVE);
	scr_conspeed    = Cvar_Get("scr_conspeed",    "3",    CVAR_ARCHIVE);
	//	scr_showturtle  = Cvar_Get("scr_showturtle",  "0",    0);
	scr_showpause   = Cvar_Get("scr_showpause",   "1",    0);
	scr_centertime  = Cvar_Get("scr_centertime",  "2.5",  0);
	//	scr_printspeed  = Cvar_Get("scr_printspeed",  "8",    0);
	scr_netgraph    = Cvar_Get("netgraph",        "0",    0);
	scr_timegraph   = Cvar_Get("timegraph",       "0",    0);
	scr_debuggraph  = Cvar_Get("debuggraph",      "0",    0);
	scr_graphheight = Cvar_Get("graphheight",     "32",   0);
	scr_graphscale  = Cvar_Get("graphscale",      "1",    0);
	scr_graphshift  = Cvar_Get("graphshift",      "0",    0);
	//	scr_drawall     = Cvar_Get("scr_drawall",     "0",    0);
	//	scr_textdelay   = Cvar_Get("scr_textdelay",   "1.0",  0);
	scr_consize     = Cvar_Get("scr_consize",     "0.5",  CVAR_ARCHIVE);
	scr_letterbox   = Cvar_Get("scr_letterbox",   "0",    0);
	statusbar_alpha = Cvar_Get("statusbar_alpha", "0.60", CVAR_ARCHIVE);
	subtitle_cvar   = Cvar_Get("subtitles",       "1",    CVAR_ARCHIVE);
	
	scr_centerFont  = re.LoadFont("scr_cent");
	
	// register our commands
	Cmd_AddCommand ("timerefresh2",SCR_TimeRefresh2_f);
	Cmd_AddCommand ("timerefresh", SCR_TimeRefresh_f);
	Cmd_AddCommand ("loading",     SCR_Loading_f);
	Cmd_AddCommand ("sizeup",      SCR_SizeUp_f);
	Cmd_AddCommand ("sizedown",    SCR_SizeDown_f);
	Cmd_AddCommand ("sky",         SCR_Sky_f);
	
	scr_initialized = true;
}


/*
==============
SCR_DrawNet
==============
*/
void SCR_DrawNet (void)
{
	if (cls.netchan.outgoing_sequence - cls.netchan.incoming_acknowledged 
		< CMD_BACKUP-1)
		return;
	
	//	re.DrawPic (scr_vrect.x+64, scr_vrect.y, "net");
}

/*
==============
SCR_DrawPause
==============
*/
void SCR_DrawPause (void)
{
	if (!scr_showpause->value)		// turn off for screenshots
		return;
	
	if (!cl_paused->value)
		return;
	
	//==================================
	// consolidation change: SCG 3-11-99
	DRAWSTRUCT drawStruct;
	int	w;
	
	drawStruct.pImage = re.RegisterPic( "pics/misc/pause.tga", &w, NULL, RESOURCE_GLOBAL );
	drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND;
	drawStruct.nXPos = ( viddef.width - w ) >> 1;
	drawStruct.nYPos = ( viddef.height >> 2 );
	re.DrawPic( drawStruct );
	// consolidation change: SCG 3-11-99
	//==================================
}


#define PROGRESS_BAR_X			451.0
#define PROGRESS_BAR_Y			381.0
#define PROGRESS_BAR_IMAGE_X	( 440.0 - 57.0 )
#define PROGRESS_BAR_IMAGE_Y	( 361.0 - 4.0 )
#define PROGRESS_BAR_MAXWIDTH	184.0
#define PROGRESS_BAR_HEIGHT		14.0
#define PROGRESS_BAR_WIDTH		( PROGRESS_BAR_MAXWIDTH / MAX_MODELS ) // change if progress_idx is modified
#define PROGRESS_BLOCK_WIDTH	5.0

/*
==============
SCR_DrawConnecting ---------------------------------------------------------------------------
==============
*/
#define SCR_CONNECTING_MAX		(100.0)
int		scr_connecting = 0;
float	scr_connecting_pos = 0;
float	scr_connecting_update = 0.0f;
void SCR_SetConnecting(qboolean bON)
{
	if (bON)
	{
		if (scr_connecting)
			return;

		scr_connecting = 1;
		scr_connecting_pos = 0.0f;
		scr_connecting_update = cl.time;
	}
	else
	{
		scr_connecting = 0;
		scr_connecting_pos = 0.0f;
		scr_connecting_update = 0.0f;
	}
/*
	scr_connecting = mode;
	if (mode == 1)
	{
		scr_connecting_pos = 0.0f;
		scr_connecting_update = cl.time;
	}
	else if (mode == 0)
	{
		scr_connecting_pos = 0.0f;
		scr_connecting_update = 0.0f;
	}*/
}

#define CONNECTING_BAR_X	(539.0)
#define CONNECTING_BAR_Y	(395.0)

void SCR_DrawConnecting(void)
{
	if (!scr_connecting)
		return;

	if (cls.state == ca_connecting)
	{
		// update the frame
		if (scr_connecting_update && (cl.time >= scr_connecting_update))
		{
			static CVector vZero( 0,0,0 );
			switch(scr_connecting)
			{
			case 3:
			case 1: scr_connecting = 2; break;
			case 2: scr_connecting = 1; break;
			default: SCR_SetConnecting(FALSE); return;
			};

			scr_connecting_update = cl.time + 250;
		}
	}
	else if (cls.state == ca_connected)
	{
		scr_connecting = 3;
	}
	else
		return;

	DRAWSTRUCT drawStruct;
	switch(scr_connecting)
	{
	case 1: 
		drawStruct.pImage	= re.RegisterPic( "pics/connect_b.tga", NULL, NULL, RESOURCE_INTERFACE );
		break;

	case 2: 
		drawStruct.pImage	= re.RegisterPic( "pics/connect_r.tga", NULL, NULL, RESOURCE_INTERFACE );
		break;

	case 3:
		drawStruct.pImage	= re.RegisterPic( "pics/connect.tga", NULL, NULL, RESOURCE_INTERFACE );
		break;

	default: 
		return;
	}

	float	     scale_x, scale_y;
	scale_x = ( ( float ) viddef.width  / 640.0 );
	scale_y = ( ( float ) viddef.height / 480.0 );
	drawStruct.nFlags	= DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_SCALE;
	drawStruct.nXPos   = CONNECTING_BAR_X * scale_x;
	drawStruct.nYPos   = CONNECTING_BAR_Y * scale_y;
	drawStruct.fScaleX = scale_x;
	drawStruct.fScaleY = scale_y;

	re.DrawPic( drawStruct ); // do it
}

/*
==============
SCR_DrawLoading ---------------------------------------------------------------------------
==============
*/
static float scr_end_loading = 0.0f;
void SCR_ForceEndLoading()
{
	scr_end_loading = cl.time + 1000;	// give it a bit more time
}

extern qboolean bGotLoadscreenName;
extern qboolean no_draw_plaque;

void SCR_DrawLoading (void)
// draw progress bar when loading
{
	//	draw progress indicator during loading
	if (scr_end_loading > 0)
	{
		if (cl.time > scr_end_loading)
		{
			if (scr_draw_progress)
				SCR_EndLoadingPlaque();
			scr_end_loading = 0.0f;
		}
		return;
	}

	if (!scr_draw_progress)// || ((progress_index <= 0) && (no_draw_plaque)) )
	{
		scr_end_loading = 0.0f;
		return;
	}
	
	if (cls.state == ca_active)
	{
		return;
	}

//	if( bGotLoadscreenName == false || con.mode != CON_LOADING )
//	{
//		return;
//	}

	DRAWSTRUCT drawStruct;
	float	     scale_x, scale_y;
	scale_x = ( ( float ) viddef.width  / 640.0 );
	scale_y = ( ( float ) viddef.height / 480.0 );
	
	if (scr_draw_progress == DRAW_PROGRESS_LOADING) // loading bar for loading screen
	{
		// draw loading box
		drawStruct.pImage  = re.RegisterPic( "pics/loadbar.tga", NULL, NULL, RESOURCE_INTERFACE );
		drawStruct.nFlags  = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_SCALE;
		drawStruct.nXPos   = PROGRESS_BAR_IMAGE_X * scale_x;
		drawStruct.nYPos   = PROGRESS_BAR_IMAGE_Y * scale_y;
		drawStruct.fScaleX = scale_x;
		drawStruct.fScaleY = scale_y;
		
		re.DrawPic( drawStruct );
		
		if (scr_connecting && progress_index)
		{
			SCR_SetConnecting(FALSE);
		}

		// draw loading bar
		drawStruct.pImage	= re.RegisterPic( "pics/loadblock.tga", NULL, NULL, RESOURCE_INTERFACE );
		drawStruct.nFlags	= DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_SCALE;
		drawStruct.fScaleX	= scale_x;
		drawStruct.fScaleY	= scale_y;
		drawStruct.nYPos	= ( PROGRESS_BAR_Y + 1.0 ) * scale_y;

		// how far along are we?
		float frac			= progress_index / (float)MAX_MODELS;
		if (frac < 0) frac = 0;
		else if (frac > 1) frac = 1;

		// find the widths and the end of the line
		float blWidth		= PROGRESS_BLOCK_WIDTH * scale_x;
		drawStruct.nXPos	= PROGRESS_BAR_X * scale_x;
		float barEnd		= ( drawStruct.nXPos + frac * ( PROGRESS_BAR_MAXWIDTH * scale_x ) ) - blWidth;

		// draw it!
		int count;
		for (count = 0; drawStruct.nXPos <= barEnd; drawStruct.nXPos += blWidth,count++)
		{
			re.DrawPic( drawStruct ); // do it
		}

		if (count > progress_count)
		{
			// SCG[2/19/00]: Make sure we dont run out of samples.  S_Update goes through all of the sounds and,
			// SCG[2/19/00]: among other things, does some housekeeping with the samples.
			static CVector vZero( 0,0,0 );
			S_Update(vZero,vZero,vZero,vZero);
			S_StartLocalSound("menus/button_002.wav");
		}

		progress_count = count;
	}
	
	if (!scr_draw_loading)
		return;
	
	scr_draw_loading = false;
}

//=============================================================================

/*
==================
SCR_RunConsole

  Scroll it up or down
  ==================
*/
void SCR_RunConsole (void)
{
	float	mul;
	
	// decide on the height of the console
	if (cls.key_dest == key_console)
	{
		//	Nelno:	added support for quick console
		if (con.mode == CON_QUICK)
		{
			//	Nelno: force size for quick console
			scr_conlines = 0.25;
		}
		else if (con.mode == CON_LOADING)
		{
			scr_conlines = 1.0;
		}
		else
		{
			//	Nelno: user-selectable size
			scr_conlines = scr_consize->value;
		}
	}	
	else
		scr_conlines = 0;				// none visible
	
	
	if (con.mode == CON_QUICK || con.mode == CON_LOADING)
		mul = 5000;
	else
		mul = scr_conspeed->value;
	
	if (scr_conlines < scr_con_current)
	{
		scr_con_current -= mul * cls.frametime;
		if (scr_conlines > scr_con_current)
			scr_con_current = scr_conlines;
		
	}
	else if (scr_conlines > scr_con_current)
	{
		scr_con_current += mul * cls.frametime;
		if (scr_conlines < scr_con_current)
			scr_con_current = scr_conlines;
	}
}

/*
==================
SCR_DrawConsole
==================
*/
void SCR_DrawConsole (void)
{
	Con_CheckResize ();

	if ((cls.state != ca_active) || !cl.refresh_prepped)
	{	
		if (cls.state == ca_uninitialized || (cls.state == ca_disconnected)) // && developer->value != 0))
			con.mode = CON_NORMAL;
		else
			// not connected or cannot render, so force full screen console
			con.mode = CON_LOADING;
		Con_DrawConsole (1.0);
		return;
	}
	else if (con.mode == CON_LOADING && cls.state == ca_active)
	{
		//	finished loading, go to quick console
		con.mode = CON_QUICK;
		Con_ConsoleOff ();
	}
	
	if (scr_con_current)
	{
		Con_DrawConsole (scr_con_current);
	}
	else
	{
		if ( (cls.key_dest == key_game || cls.key_dest == key_message) && (cls.state == ca_active) && !cls.disable_screen)
			Con_DrawNotify ();	// only draw notify in game
	}
}

//=============================================================================

/*
================
SCR_BeginLoadingPlaque
================
*/
void SCR_BeginLoadingPlaque (void)
{
	if (scr_draw_progress)
		return;

	cl.refdef.rdflags |= RDF_NO_ICONS;
	scr_draw_progress = DRAW_PROGRESS_LOADING;  // assume loading screen exists to show bar
	
	if(!dedicated)
		S_StopAllSounds ();
	
	cl.sound_prepped = false;		// don't play ambients
	CDAudio_Stop ();
	
	progress_index = 0; // initialize progress bar
	progress_count = 0;
	// randomize starting console's background screen     12.22 dsn
	//Cvar_SetValue ("console_screen_idx", ((rand() % NUM_RANDOM_BKG_SCREENS) * 2));
	
	if (cls.disable_screen)
		return;
	
	//	if (developer->value)
	//		return;
	
	if (cls.state == ca_disconnected)
		return;	// if at console, don't bring up the plaque
	
	if (cls.key_dest == key_console)
		return;
	
	if (cl.cinematictime > 0 || cls.state == ca_connecting)
		scr_draw_loading = 2;	// clear to balack first
	else
		scr_draw_loading = 1;
	
	// cek[2-17-00]: hacky hacky.  pause the shiznit while we do this so the jitter goes away!
	if (!dedicated->value)
	{
		float paused = cl_paused->value;
		float showpause = scr_showpause->value;
		scr_showpause->value = 0;
		cl_paused->value = 1;
		SCR_UpdateScreen ();
		SCR_UpdateScreen ();
		SCR_UpdateScreen ();
		SCR_UpdateScreen ();
		SCR_UpdateScreen ();

		// set things back the way they were..
		if (!paused)
			cl_paused->value = 0;
		if (showpause)
			scr_showpause->value = 1;
	}

	cls.disable_screen = Sys_Milliseconds ();
	cls.disable_servercount = cl.servercount;
}

/*
================
SCR_EndLoadingPlaque
================
*/
extern qboolean no_draw_plaque;
void SCR_EndLoadingPlaque (void)
{
	//	Nelno:	don't show progress indicator
	no_draw_plaque = false;
	bGotLoadscreenName = FALSE;
	cl.refdef.rdflags &= ~RDF_NO_ICONS;
	scr_draw_progress = 0;
	SCR_SetConnecting(FALSE);
	//progress_index    = 0; // initialize progress bar
	
	cls.disable_screen = 0;
	Con_ClearNotify ();
	scr_end_loading = false;
}

/*
================
SCR_Loading_f
================
*/
void SCR_Loading_f (void)
{
	SCR_BeginLoadingPlaque ();
}

/*
================
SCR_TimeRefresh_f
================
*/
int entitycmpfnc( const entity_t *a, const entity_t *b )
{
/*
** all other models are sorted by model then skin
	*/
	if ( a->model == b->model )
	{
		return ( ( int ) a->skin - ( int ) b->skin );
	}
	else
	{
		return ( ( int ) a->model - ( int ) b->model );
	}
}

void SCR_TimeRefresh_f (void)
{
	int		i;
	int		start, stop;
	float	time;
	
	if ( cls.state != ca_active )
		return;
	
	start = Sys_Milliseconds ();
	
	if (GetArgc() == 2)
	{	// run without page flipping
		re.BeginFrame( 0 );
		for (i=0 ; i<128 ; i++)
		{
			cl.refdef.viewangles.y = i/128.0*360.0;
			re.RenderFrame (&cl.refdef);
		}
		re.EndFrame();
	}
	else
	{
		for (i=0 ; i<128 ; i++)
		{
			cl.refdef.viewangles.y = i/128.0*360.0;
			
			re.BeginFrame( 0 );
			re.RenderFrame (&cl.refdef);
			re.EndFrame();
		}
	}
	
	stop = Sys_Milliseconds ();
	time = (stop-start)/1000.0;
	Com_Printf ("%f seconds (%f fps)\n", time, 128/time);
}

void SCR_TimeRefresh2_f (void)
{
	int		i;
	int		start, stop;
	float	time;
	
	if ( cls.state != ca_active )
		return;
	
	start = Sys_Milliseconds ();
	
	if (GetArgc() == 2)
	{	// run without page flipping
		re.BeginFrame( 0 );
		for (i=0 ; i<128 ; i++)
		{
			//			cl.refdef.viewangles[1] = i/128.0*360.0;
			re.RenderFrame (&cl.refdef);
		}
		re.EndFrame();
	}
	else
	{
		for (i=0 ; i<128 ; i++)
		{
			//			cl.refdef.viewangles[1] = i/128.0*360.0;
			
			re.BeginFrame( 0 );
			re.RenderFrame (&cl.refdef);
			re.EndFrame();
		}
	}
	
	stop = Sys_Milliseconds ();
	time = (stop-start)/1000.0;
	Com_Printf ("%f seconds (%f fps)\n", time, 128/time);
}


/*
=================
SCR_AddDirtyPoint
=================
*/
void SCR_AddDirtyPoint (int x, int y)
{
	if (x < scr_dirty.x1)
		scr_dirty.x1 = x;
	if (x > scr_dirty.x2)
		scr_dirty.x2 = x;
	if (y < scr_dirty.y1)
		scr_dirty.y1 = y;
	if (y > scr_dirty.y2)
		scr_dirty.y2 = y;
}


void SCR_DirtyScreen (void)
{
	SCR_AddDirtyPoint (0, 0);
	SCR_AddDirtyPoint (viddef.width-1, viddef.height-1);
}



void SCR_TileClear (void)
// clear any parts of the tiled background that were drawn on last frame
{
	int		i;
	int		top, bottom, left, right;
	dirty_t	clear;
	char	backtilestr[64];
	int		draw_solid;
	
	draw_solid = 1;
	
// SCG[1/16/00]: 	sprintf(backtilestr,  "pics/backtile/tile64x64.pcx");
	Com_sprintf(backtilestr, sizeof( backtilestr ), "pics/backtile/e%d_tile.pcx", cl_episode_num );
	
	
	if ((CIN_RunningWithView() == 1) || (cl.frame.playerstate.rdflags & RDF_LETTERBOX))
	{
		// full screen cinematic
		SCR_DirtyScreen();
		draw_solid = 1;
	}
	
	
	if (scr_con_current == 1.0)
		return;		// full screen console
	
	if (scr_viewsize->value >= 100 && !( ( CIN_RunningWithView() == 1 ) || (cl.frame.playerstate.rdflags & RDF_LETTERBOX) ) )
		return;		// full screen rendering
	
	//if (scr_drawall->value)
	SCR_DirtyScreen ();	// for power vr or broken page flippers...
	
	// erase rect will be the union of the past three frames
	// so tripple buffering works properly
	clear = scr_dirty;
	for (i=0 ; i<2 ; i++)
	{
		if (scr_old_dirty[i].x1 < clear.x1)
			clear.x1 = scr_old_dirty[i].x1;
		if (scr_old_dirty[i].x2 > clear.x2)
			clear.x2 = scr_old_dirty[i].x2;
		if (scr_old_dirty[i].y1 < clear.y1)
			clear.y1 = scr_old_dirty[i].y1;
		if (scr_old_dirty[i].y2 > clear.y2)
			clear.y2 = scr_old_dirty[i].y2;
	}
	
	scr_old_dirty[1] = scr_old_dirty[0];
	scr_old_dirty[0] = scr_dirty;
	
	scr_dirty.x1 = 9999;
	scr_dirty.x2 = -9999;
	scr_dirty.y1 = 9999;
	scr_dirty.y2 = -9999;
	
	// don't bother with anything convered by the console
	top = scr_con_current * viddef.height;
	if (top >= clear.y1)
		clear.y1 = top;
	
	if (clear.y2 <= clear.y1)
		return;		// nothing disturbe
	
	
	top = scr_vrect.y;
	bottom = top + scr_vrect.height-1;
	left = scr_vrect.x;
	right = left + scr_vrect.width-1;
	
	CVector	rgbColor;
	rgbColor.x = rgbColor.y = rgbColor.z = 0;
	if (clear.y1 < top)
	{	
		// clear above view screen
		i = clear.y2 < top-1 ? clear.y2 : top-1;
		
		if (draw_solid == 0)
			//re.DrawTileClear (clear.x1 , clear.y1, clear.x2 - clear.x1 + 1, i - clear.y1+1, (rand() % 2 ? backtilestr : backtilestr2));
			re.DrawTileClear (clear.x1 , clear.y1, clear.x2 - clear.x1 + 1, i - clear.y1+1, backtilestr);
		else
			re.DrawFill( clear.x1, clear.y1, clear.x2 - clear.x1 + 1, i - clear.y1+1, rgbColor, 1.0 );
		//	    re.DrawFill(clear.x1 , clear.y1, clear.x2 - clear.x1 + 1, i - clear.y1+1, );
		
		clear.y1 = top;
	}
	
	if (clear.y2 > bottom)
	{	
		// clear below view screen
		i = clear.y1 > bottom+1 ? clear.y1 : bottom+1;
		
		if (draw_solid == 0)
			//re.DrawTileClear (clear.x1, i, clear.x2-clear.x1+1, clear.y2-i+1, (rand() % 2 ? backtilestr : backtilestr2));
			re.DrawTileClear (clear.x1, i, clear.x2-clear.x1+1, clear.y2-i+1, backtilestr);
		else
			re.DrawFill( clear.x1, i, clear.x2 - clear.x1 + 1, clear.y2 - i + 1, rgbColor, 1.0 );
		
		clear.y2 = bottom;
	}
	
	if (clear.x1 < left)
	{	
		// clear left of view screen
		i = clear.x2 < left-1 ? clear.x2 : left-1;
		
		if (draw_solid == 0)
			//re.DrawTileClear (clear.x1, clear.y1, i-clear.x1+1, clear.y2 - clear.y1 + 1, (rand() % 2 ? backtilestr : backtilestr2));
			re.DrawTileClear (clear.x1, clear.y1, i-clear.x1+1, clear.y2 - clear.y1 + 1, backtilestr);
		else
			re.DrawFill( clear.x1, clear.y1, i-clear.x1 + 1, clear.y2 - clear.y1 + 1, rgbColor, 1.0 );
		
		clear.x1 = left;
	}
	
	if (clear.x2 > right)
	{	
		// clear left of view screen
		i = clear.x1 > right+1 ? clear.x1 : right+1;
		
		if (draw_solid == 0)
			//re.DrawTileClear (i, clear.y1, clear.x2-i+1, clear.y2 - clear.y1 + 1, (rand() % 2 ? backtilestr : backtilestr2));
			re.DrawTileClear (i, clear.y1, clear.x2-i+1, clear.y2 - clear.y1 + 1, backtilestr);
		else
			re.DrawFill( i, clear.y1, clear.x2 - i + 1, clear.y2 - clear.y1 + 1, rgbColor, 1.0 );
		
		clear.x2 = right;
	}
	
}









///////////////////////////////////////////////////////////////////////////////
//	SCR_RegisterCrosshair
//
///////////////////////////////////////////////////////////////////////////////

void	SCR_RegisterCrosshair (void)
{
	crosshair_t *crosshair;
	int i,cnum;
	
	cnum=cv_crosshair->value;
	
	//	register standard crosshair
	if (cnum)
	{
		if (cnum > MAX_CROSSHAIRS || cnum < 0)
			cnum = MAX_CROSSHAIRS;
		
		crosshair=&crosshairList[cnum-1][0];
		for (i=0; i<MAX_CROSSHAIR_TYPES; i++)
		{
			//==================================
			// consolidation change: SCG 3-11-99
			re.RegisterPic( crosshair->pic, &crosshair->width, &crosshair->height, RESOURCE_GLOBAL );
			// consolidation change: SCG 3-11-99
			//==================================
			if (!crosshair->width)
				crosshair->pic[0] = 0;
			crosshair++;
		}
	}
	
	// register special crosshairs
}






/*
==================
SCR_UpdateScreen

  This is called every frame, and can also be called explicitly to flush
  text to the screen.
  ==================
*/
void Con_DrawConsolePic(int x, int y, int w, int h, qboolean bLoading);
void Con_DrawVersion(int x, int y);
void SCR_UpdateScreen (void)
{
	int numframes;
	int i;
	float separation[2] = { 0, 0 };
	
	// ISP - draw screen only when this app is active
	extern qboolean in_appactive;
	if ( !in_appactive )
		return;

	// if the screen is disabled (loading plaque is up, or vid mode changing)
	// do nothing at all
	if (cls.disable_screen)
	{
		if (cls.disable_screen == 1.0f)
		{
			//	Nelno:	not in LoadingPlaque, must be vid mode change
			if (Sys_Milliseconds() - cls.disable_screen > 100)
				cls.disable_screen = 0;
		}
		else
		{
			// SCG[3/7/00]: Keep this value at 30000 please.  
			if (Sys_Milliseconds() - cls.disable_screen > 30000)
			{
				cls.disable_screen = 0;
				Com_Printf ("Loading plaque timed out.\n");
			}

			// do a limited subset of screen updating here so we can see loading bars and plaques, etc.
			if (scr_initialized && con.initialized && (DKM_InMenu() == false))		
			{
				re.BeginFrame( 0 );
				SCR_CalcVrect ();  // do 3D refresh drawing, and then update the screen
				SCR_TileClear ();  // clear any dirty part of the background
				SCR_DrawConsole ();
				SCR_DrawLoading ();
				SCR_DrawConnecting();
				re.EndFrame();
			}
			
			return;
		}
	}
	
	if (!scr_initialized || !con.initialized)
		return;				// not initialized yet
	
							/*
							** range check cl_camera_separation so we don't inadvertently fry someone's
							** brain
	*/
	if (cl_stereo_separation->value > 1.0)
		Cvar_SetValue("cl_stereo_separation", 1.0);
	else if (cl_stereo_separation->value < 0)
		Cvar_SetValue("cl_stereo_separation", 0.0);
	
	if (cl_stereo->value)
	{
		numframes = 2;
		separation[0] = -cl_stereo_separation->value * 0.5;
		separation[1] = cl_stereo_separation->value * 0.5;
	}		
	else
	{
		separation[0] = 0;
		separation[1] = 0;
		numframes = 1;
	}
	
	for (i = 0; i < numframes; i++ )
	{
		re.BeginFrame( separation[i] );
/*		if ( clear_it )
		{
			Con_DrawConsolePic(0, 0, viddef.width, viddef.height, (con.mode == CON_LOADING));
		}*/
/*		
		if (scr_draw_loading == 2)
		{	
			//  loading plaque over black screen
			int		w, h;
			
			re.CinematicSetPalette(NULL);
			scr_draw_loading = false;
			
			DRAWSTRUCT drawStruct;
			drawStruct.pImage = re.RegisterPic( "loading", &w, &h, RESOURCE_GLOBAL );
			drawStruct.nFlags = DSFLAG_BLEND;
			drawStruct.nXPos = ( viddef.width  - w ) >> 1;
			drawStruct.nYPos = ( viddef.height - h ) >> 1;
			re.DrawPic( drawStruct );
		} 
*/		
		// if a cinematic is supposed to be running, handle menus
		// and console specially
		// SCG[2/17/00]: This is for quake cinematics, which we do not use...
/*
		if (cl.cinematictime > 0)
		{
			if (cls.key_dest == key_menu) 
			{
				if (cl.cinematicpalette_active)
				{
					re.CinematicSetPalette(NULL);
					cl.cinematicpalette_active = false;
				}
				
				M_Draw ();
			}
			else if (cls.key_dest == key_console) 
			{
				if (cl.cinematicpalette_active)
				{
					re.CinematicSetPalette(NULL);
					cl.cinematicpalette_active = false;
				}
				
				SCR_DrawConsole ();
			}
			else
				SCR_DrawCinematic();
		}
		else
*/		{
			// make sure the game palette is active
			if (cl.cinematicpalette_active)
			{
				re.CinematicSetPalette(NULL);
				cl.cinematicpalette_active = false;
			}
			
			//adam: if we are in the menu, do not draw anything but the menu
			if (DKM_InMenu() == false)
			{
				
				SCR_CalcVrect ();  // do 3D refresh drawing, and then update the screen
				SCR_TileClear ();  // clear any dirty part of the background
				
				V_RenderView ( separation[i] );
				
				SCR_DrawStats ();
				
				SCR_DrawNet ();
				SCR_CheckDrawCenterString ();
				SCR_CheckDrawSubtitleString();
				
				sb_DrawScoreboard ();
				if (scr_timegraph->value)
					SCR_DebugGraph (cls.frametime*300, 0);
				
				if (scr_debuggraph->value || scr_timegraph->value || scr_netgraph->value)
					SCR_DrawDebugGraph ();
				
				SCR_DrawPause ();
				dk_print_draw ();   //	display printxy text
				SCR_DrawConsole ();
				
				CIN_GCE_HUD();      // game cinematic editor hud.
//#pragma message("cek[2-17-00]: Remove for release")
//				Con_DrawVersion(viddef.width - 200, 5);

				SCR_DrawLoading ();
				SCR_DrawConnecting();
			}
			else
			{
				SCR_CalcVrect();  // SCG[6/1/99]: Added so menu draws correctly whne in letterbox model...
				M_Draw();         //we are in the main full-screen menu, dont draw anything but the menu.
			}
			
//			SCR_DrawLoading ();
		}
		
		re.EndFrame();
	}
}





// ******************************************************************************


#define STATUSBAR_POWER2              (128)       // assume pix are 128x128
#define STATUSBAR_FONT_POWER2         (16)        // assume font pix are 16x16
#define NUM_STATUSBAR_FACES           (47)
#define NUM_STATUSBAR_ICONS           (5)
#define STATUSBAR_FACES_FRAMES        (8)

#define STATUSBAR_FACES_INVULNERABLE  (6)
#define STATUSBAR_FACES_INVISIBLE     (7)
#define STATUSBAR_FACES_FLAP_MIKIKO   (24)
#define STATUSBAR_FACES_FLAP_SUPERFLY (31)
#define STATUSBAR_FACES_FLAP_HIRO	  (38)
#define STATUSBAR_FACES_FLAP_INVIS	  (5)
#define STATUSBAR_FACES_FLAP_INVUL	  (6)
#define STATUSBAR_HIRO_NAME			  (45)
#define STATUSBAR_BLANK_FLAP		  (46)



typedef struct sb_info_s
{
	short x_add, y_add;      // image is centered on bottom of screen -- x,y is added to that position
} sb_info_t;


char * statusbar_faces[NUM_STATUSBAR_FACES] =
{ 
	"pics/statusbar/bottom_2_0a.tga",	// hiro, full health
	"pics/statusbar/bottom_2_0b.tga",
	"pics/statusbar/bottom_2_0c.tga",
	"pics/statusbar/bottom_2_0d.tga",
	"pics/statusbar/bottom_2_0e.tga",
	"pics/statusbar/bottom_2_0f.tga", // dead
	"pics/statusbar/bottom_2_0g.tga", // invulnerable
	"pics/statusbar/bottom_2_0h.tga", // invisible
	
	"pics/statusbar/bottom_2_1a.tga", // mikiko, full health
	"pics/statusbar/bottom_2_1b.tga",
	"pics/statusbar/bottom_2_1c.tga",
	"pics/statusbar/bottom_2_1d.tga",
	"pics/statusbar/bottom_2_1e.tga",
	"pics/statusbar/bottom_2_1f.tga", // dead
	"pics/statusbar/bottom_2_1g.tga", // invulnerable
	"pics/statusbar/bottom_2_1h.tga", // invisible
	
	"pics/statusbar/bottom_2_2a.tga", // superfly, full health
	"pics/statusbar/bottom_2_2b.tga",
	"pics/statusbar/bottom_2_2c.tga",
	"pics/statusbar/bottom_2_2d.tga",
	"pics/statusbar/bottom_2_2e.tga" ,
	"pics/statusbar/bottom_2_2f.tga", // dead
	"pics/statusbar/bottom_2_2g.tga", // invulnerable
	"pics/statusbar/bottom_2_2h.tga", // invisible
	
	"pics/statusbar/mikikoa.tga", 		//24 mikiko sidekick flap, full health
	"pics/statusbar/mikikob.tga", 
	"pics/statusbar/mikikoc.tga", 
	"pics/statusbar/mikikod.tga", 
	"pics/statusbar/mikikoe.tga", 
	"pics/statusbar/mikikof.tga",		// invisible
	"pics/statusbar/mikikog.tga",		// invulnerable
	
	"pics/statusbar/superflya.tga",   //31 superfly sidekick flap, full health
	"pics/statusbar/superflyb.tga", 
	"pics/statusbar/superflyc.tga", 
	"pics/statusbar/superflyd.tga", 
	"pics/statusbar/superflye.tga", 
	"pics/statusbar/superflyf.tga",		// invisible
	"pics/statusbar/superflyg.tga",		// invulnerable

	"pics/statusbar/hiroa.tga", 		//38 hiro sidekick flap, full health
	"pics/statusbar/hirob.tga", 
	"pics/statusbar/hiroc.tga", 
	"pics/statusbar/hirod.tga", 
	"pics/statusbar/hiroe.tga", 
	"pics/statusbar/hirof.tga",		// invisible
	"pics/statusbar/hirof.tga",		// invulnerable

	"pics/statusbar/hirotext.tga",
	"pics/statusbar/blank.tga"
};


char * statusbar_icons[NUM_STATUSBAR_ICONS] = 
{
	"pics/statusbar/armoricon.bmp",
		"pics/statusbar/healthicon.bmp",
		"pics/statusbar/ammoicon.bmp",
		"pics/statusbar/fragicon.bmp",
		"pics/statusbar/explevicon.bmp",
};

enum e_sidekick_ammo_icons
{
	SIDEKICK_AMMO_BORDER = 0,
	SIDEKICK_AMMO_NADA,
	SIDEKICK_AMMO_LOW,
	SIDEKICK_AMMO_GOOD,
	SIDEKICK_AMMO_ROCKIN
};

static char *sidekick_ammo_icons[] = 
{
	"pics/statusbar/ammo.tga",
	"pics/statusbar/ammo4.tga",
	"pics/statusbar/ammo3.tga",
	"pics/statusbar/ammo2.tga",
	"pics/statusbar/ammo1.tga"
};


int bottom_mainbar_offs[4] = { -158, 98, 226, -30 }; // piece 2 & 4 reversed

// used for number stat font placement
sb_info_t sb_info[] = { 
	//    x    y
	{  -162, -35 }, // armor
	{   -71, -35 }, // health
	{    82, -35 }, // ammo
	{   172, -35 }, // frags/exp
	
}; 

// used for icon placement
sb_info_t sb_info_icon[] = { 
	//    x    y
	{  -144, -64 },  // armor
	{   -54, -67 },  // health
	{    98, -66 },  // ammo
	{   196, -64 }   // frags/exps
};


// used for font placement
sb_info_t sb_info_text[] = { 
	//    x    y
	{  -122, -22 },  // armor
	{   -40, -22 },  // health
	{   126, -22 },  // ammo
	{   212, -22 }   // frags/exps
};



// warning levels for primary stats (floor/ceiling values)
//
int stat_warning[4][2] = {    0,  25,   // armor 
-99,  25,   // health
0,   5,   // ammo
-99,  -1,   // frags
};


void SCR_RegisterStatus (void)
// register main status bar bitmaps
{
	int	i;
	
	for (i = 0;i < NUM_STATUSBAR_FACES;i++)
		re.RegisterPic (statusbar_faces[i],   NULL, NULL, RESOURCE_EPISODE);
	
	for (i = 0;i < NUM_STATUSBAR_ICONS;i++)
		re.RegisterPic (statusbar_icons[i], NULL, NULL, RESOURCE_EPISODE);
	
	re.RegisterPic ("pics/statusbar/bottom_1.tga", NULL, NULL, RESOURCE_EPISODE);
	re.RegisterPic ("pics/statusbar/bottom_3.tga", NULL, NULL, RESOURCE_EPISODE);
	re.RegisterPic ("pics/statusbar/bottom_4.tga", NULL, NULL, RESOURCE_EPISODE);
}



void global_hud_updates(alphafx_t * hud, float * model_alpha, float max_alpha)
// update invmode hud settings
{
	if (hud->off_time && cl.time > hud->off_time)
	{
		hud->off_time = 0;
		hud->direction = -1;
		hud->fade_time = cl.time;
		return;
	}

	if (hud->direction == 0)
	{
		if (hud->alpha > 0.0f)
			hud->alpha = max_alpha;
		else
			hud->alpha = 0.0f;
		return;
	}

	if (hud->direction == 1)                       // fading in? 
	{
//		hud->alpha += FLAP_ALPHA_RATE_DOUBLE;        // increase alpha, double-time
		hud->alpha += 2*FLAP_ALPHA_RATE;        // increase alpha, double-time
		
		if (hud->alpha >= max_alpha)                  // alpha level reached maximum?
		{
			hud->alpha     = max_alpha;                // insure maximum
			hud->direction = 0;                        // stabilize alpha
			//hud->fade_time = 0.0f;                   // do not set this, it holds fadeout time
		}
		
//		*model_alpha = hud->alpha;                   // to affect 3D model alphas
		*model_alpha = MODEL_ALPHA(hud->alpha);
	}
	else
	{
		if (hud->fade_time && cl.time > hud->fade_time)    // fading out?
		{
//			hud->alpha -= FLAP_ALPHA_RATE;               // decrease alpha
			hud->alpha -= 2*FLAP_ALPHA_RATE;               // decrease alpha
			
			if (hud->alpha <= 0)                          // alpha level reached minimum?
			{
				hud->alpha     = 0.0f;                     // insure minimum
				hud->direction = 0;                        // stabilize alpha
				hud->fade_time = 0.0f;                     // reset time
				hud->off_time  = 0;
				
//				if (invmode_hud.alpha <= 0)
//					inventory_init(); // reset all vars
			}
			
//			*model_alpha = hud->alpha;                 // to affect 3D model alphas
			*model_alpha = MODEL_ALPHA(hud->alpha);
		}
	}
		
}




void invmode_hud_setfade(int dir)
// set hud mode info 
{
	invmode_hud.direction = dir;
	invmode_hud.fade_time = cl.time + INVMODE_FADETIME;  // set time before fade starts
}



#define SIDEKICK_HEALTHARMOR_BAR_Y      (43)
#define SIDEKICK_ARMOR_BAR_X_OFFSET     (15)
#define SIDEKICK_HEALTHARMOR_BAR_WIDTH   (3)

#define SIDEKICK_BAR_CRUNCHSCALE       (2.8) // compact bar (adjust y base with SIDEKICK_HEALTHARMOR_BAR_Y)


void sidekick_draw_healtharmor_bar(int left, int health, int armor, float alpha)
// draw sidekick's health and armor bars
{
	DRAWSTRUCT drawStruct; 
	int bar_height;
	int i, val;
	
	
	// pixel draw
	drawStruct.pImage  = NULL;
	drawStruct.nFlags  = DSFLAG_FLAT | DSFLAG_COLOR | DSFLAG_ALPHA;
	drawStruct.nLeft   = left;
	drawStruct.nRight  = SIDEKICK_HEALTHARMOR_BAR_WIDTH;
	drawStruct.fAlpha  = alpha;
	
	// draw health and armor bars
	for (i = 0;i < 2;i++)
	{
		drawStruct.nTop = viddef.height - SIDEKICK_HEALTHARMOR_BAR_Y;
		
		if (i == 0) // health first
		{
			val = health;
			drawStruct.rgbColor.Set(1,0,0); // set rgb red color for heatlh
		}
		else        // armor second
		{
			val = armor;
			drawStruct.rgbColor.Set(1,1,0); // set rgb yellow color for armor
		}
		
		if (val < 0) // check lower boundary
			val = 0;
		if (val > 100)
			val = 100;	// check upper boundary too.
		
		bar_height = (100 / SIDEKICK_BAR_CRUNCHSCALE) - (val / SIDEKICK_BAR_CRUNCHSCALE);
		
		drawStruct.nTop    += bar_height;                         // new start position of bar
		drawStruct.nBottom = (100 / SIDEKICK_BAR_CRUNCHSCALE) - bar_height;   // height of bar
		
		re.DrawPic( drawStruct );
		
		// adjust x for armor bar
		drawStruct.nLeft += SIDEKICK_ARMOR_BAR_X_OFFSET;
	}
	
}	 





#define SKILLS_LABEL_TOP           (274)
#define SKILLS_WINDOW_TOP          (SKILLS_LABEL_TOP - 14)
#define SKILLS_LABEL_SPACING       (36)
#define SKILLS_CATEGORY_LABEL_TOP  (100)

#define SKILLS_NODULE_LEFT         (12)
#define SKILLS_NODULE_WIDTH        (6)
#define SKILLS_NODULE_HEIGHT       (8)
#define SKILLS_NODULE_SPACING      (8)
#define SKILLS_Y_OFFSET            (10)

//extern cvar_t *sv_episode;
void draw_skills_info(void)
// draw skill windows and levels
{
	DRAWSTRUCT drawStruct; 
	int i, i2, y, val;
	char buf[32];
	
	//  if (scr_viewsize->value >= SCR_VIEWSIZE_NOSKILLS &&    // is screen size disabling skill display
	//      !xplevelup_mode &&                                 // and not in 'level up' mode?
	//      !override_showhuds)                                // and not showing all huds?
	//	  return;                                              // then bail out
	
	if (!skill_hud.alpha)
		return;
	
	int episode = atoi( CM_KeyValue (map_epairs, "episode"));//floor(sv_episode->value + 0.5);				// make sure its good!
	bool deathmatch = (cl.refdef.rdflags & RDF_DEATHMATCH) > 0;
	// draw skills on left side
	for (i = 0; i < MAX_SKILLS; i++)
	{
		// draw skill titles
// SCG[1/16/00]: 		sprintf(buf,"%s",tongue_statbar[T_SKILLS_POWER + i]);
//		sprintf(buf,sizeof(buf),"%s",tongue_statbar[T_SKILLS_POWER + i]);
		Com_sprintf(buf,sizeof(buf),"%s",tongue_statbar[T_SKILLS_POWER + i]);
		
		drawStruct.nFlags   = DSFLAG_BLEND | DSFLAG_PALETTE | DSFLAG_ALPHA;
		drawStruct.pImage   = statbar_font;
		drawStruct.szString = buf;
		drawStruct.nXPos    = 0;
		drawStruct.nYPos    = (viddef.height - SKILLS_LABEL_TOP) + (i * SKILLS_LABEL_SPACING);
		drawStruct.fAlpha   = INFO_ALPHA(skill_hud.alpha);//skill_hud.alpha;
		re.DrawString( drawStruct );
		
		// draw skill window
		y = (viddef.height - SKILLS_WINDOW_TOP) + (i * SKILLS_LABEL_SPACING);  // store y for nodule placement
		
		drawStruct.pImage = re.RegisterPic( "pics/statusbar/skill_window.tga", NULL, NULL, RESOURCE_INTERFACE );
		drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA;
		drawStruct.nXPos = 0;
		drawStruct.nYPos = y;
		drawStruct.fAlpha = skill_hud.alpha;
		re.DrawPic( drawStruct );
		
		// draw skill nodules for window
		switch (i)
		{
		case 0 : val = cl.frame.playerstate.stats[STAT_POWER];         break; // power
		case 1 : val = cl.frame.playerstate.stats[STAT_ATTACK];        break; // attack
		case 2 : val = cl.frame.playerstate.stats[STAT_SPEED];         break; // speed
		case 3 : val = cl.frame.playerstate.stats[STAT_JUMP];          break; // acro
		case 4 : val = cl.frame.playerstate.stats[STAT_MAX_HEALTH];    break; // vitality
		default: val = 0; break;
		}
		
		// set up draw structure for flat pixel drawing
		drawStruct.pImage = NULL;
		drawStruct.nFlags = DSFLAG_FLAT | DSFLAG_COLOR | DSFLAG_ALPHA; 
		
		drawStruct.rgbColor.Set(0,0,0); // init rgb
		
		drawStruct.nTop    = y + SKILLS_Y_OFFSET;   // y position
		drawStruct.nLeft   = SKILLS_NODULE_LEFT;    // left margin
		drawStruct.nRight  = SKILLS_NODULE_WIDTH;	  // set width of nodule
		drawStruct.nBottom = SKILLS_NODULE_HEIGHT;  // height of nodule
		drawStruct.fAlpha  = INFO_ALPHA(skill_hud.alpha);//skill_hud.alpha;       
		
		for (i2 = 1; i2 < (MAX_SKILLS + 1); i2++) // draw individual nodules
		{
			if (deathmatch)
			{
				if (val >= i2)
					drawStruct.rgbColor.y = 1;   // g -- full
				else
					drawStruct.rgbColor.y = 0.2; // g -- dull
			}
			else
			{
				if (val >= i2)
					drawStruct.rgbColor.y = 1;   // g -- full
//				else if (episode > 2)
//					drawStruct.rgbColor.y = 0.2;   // g -- full
				else
				{
					if (i2 > episode + 1)
						drawStruct.rgbColor.y = 0.1;   // g -- full
					else
						drawStruct.rgbColor.y = 0.25;   // g -- full
				}
			}
/*
			if (val >= i2)
				drawStruct.rgbColor.y = 1;   // g -- full
			else
				drawStruct.rgbColor.y = 0.2; // g -- dull
*/

			re.DrawPic( drawStruct ); // do it
			
			// adjust x for armor bar
			drawStruct.nLeft += SKILLS_NODULE_SPACING; // shift right for next nodule
		}
	}
	
}




#define XPLEVEL_TOP            (SKILLS_LABEL_TOP + 50)
#define XPLEVEL_FONT_SPACING   (12)

#define XPLEVEL_NODULE_TOP     (10)
#define XPLEVEL_NODULE_LEFT    (194)
#define XPLEVEL_NODULE_WIDTH   (2)
#define XPLEVEL_NODULE_HEIGHT  (2)
#define XPLEVEL_NODULE_SPACING (3)

void draw_xplevel_info(void)
// experience points related draw routines
{
	//if (cl.frame.playerstate.stats[STAT_FRAGS] != NO_FRAGS_USED)  // frags active?  don't set up exps
	//	return;
	
	long exp_level[] = 
#include "levels.h"
		
		// draw experience points nodules on window
		DRAWSTRUCT drawStruct; 
	char buf[32];
	int y, i, nodules;
	long next_level_xps, current_xps, range_xps;
	
	
	//if (override_showhuds || scr_viewsize->value < SCR_VIEWSIZE_NOSTATUS)
	if (override_showhuds || status_hud.alpha)
	{ 
		// set up draw structure for flat pixel drawing
		drawStruct.pImage = NULL;
		drawStruct.nFlags = DSFLAG_FLAT | DSFLAG_COLOR | DSFLAG_ALPHA; 
		
		drawStruct.rgbColor.Set(0,0,0); // init rgb
		
		drawStruct.nTop    = viddef.height - XPLEVEL_NODULE_TOP; // y position
		drawStruct.nLeft   = ((viddef.width - STATUSBAR_POWER2) >> 1);  // x position
		drawStruct.nLeft  += XPLEVEL_NODULE_LEFT;
		drawStruct.nRight  = XPLEVEL_NODULE_WIDTH;	             // set width of nodule
		drawStruct.nBottom = XPLEVEL_NODULE_HEIGHT;              // height of nodule
		drawStruct.fAlpha  = status_hud.alpha;
		
		// calculate percentage from next experience point ceiling
		next_level_xps = exp_level[(cl.frame.playerstate.stats[STAT_LEVEL] + 1)]; 
		
		range_xps   = next_level_xps - exp_level[cl.frame.playerstate.stats[STAT_LEVEL]];
		current_xps = next_level_xps - cl.frame.playerstate.stats[STAT_EXP];
		
		nodules = 10 - ((float)current_xps / (float)range_xps) * 10;
		
		for (i = 0;i < 10;i++)
		{
			if (i < nodules)
				drawStruct.rgbColor.y = 1;   // g - full
			else
				drawStruct.rgbColor.y = 0.2; // g - dull
			
			re.DrawPic( drawStruct ); // do it
			
			drawStruct.nTop -= XPLEVEL_NODULE_SPACING; // bump x right
		}
	}
	
	
	// check for 'level up'
	
	if (xplevelup_mode == false)  // not in xp level up mode?
		return;											// get outta here!
	
	// check blink time
	if (cl.time > xplevelup_timer)
	{
		xplevelup_timer = cl.time + XPLEVELUP_BLINKTIME;
		xplevelup_blink ^= 1;                       // toggle on/off
	}
	
	
	// draw header / instructions
	drawStruct.nFlags = DSFLAG_BLEND | DSFLAG_PALETTE;
	drawStruct.pImage = statbar_font;
	drawStruct.nXPos    = 0;
	
	y = viddef.height - XPLEVEL_TOP;
/*	
	for (i = 0;i < 3;i++)
	{
		switch (i)
		{
		case 0 : 
			if (!xplevelup_blink)  
				Com_sprintf(buf,sizeof(buf), "%s",tongue_statbar[T_XPLEVEL_LEVELUP]);
			else
				Com_sprintf(buf,sizeof(buf), ""); // skip
			break;
			
			
		case 1 : // show which keys are used to select up/down
			Com_sprintf(buf,sizeof(buf),"");	// skip
			break;
			
		case 2 : // show which key is used to apply skill point
			Com_sprintf(buf,sizeof(buf), "%c  %s",Key_MatchCommand("hud_use"),
				tongue_statbar[T_XPLEVEL_ASSIGN]); 
			break;
		}
		
		drawStruct.szString = buf;
		drawStruct.nYPos    = y;
		re.DrawString( drawStruct );
		
		y += XPLEVEL_FONT_SPACING;
	}
*/	
	
	// draw skill selection arrow, blinking
	if (!xplevelup_blink)
	{
		drawStruct.nXPos = 64; // to the right of the skill window
		drawStruct.nYPos = (viddef.height - SKILLS_WINDOW_TOP) + (xplevelup_selection * SKILLS_LABEL_SPACING) + 6;
// SCG[1/16/00]: 		sprintf(buf,ARROW_LEFT_STRING);
		Com_sprintf(buf,sizeof( buf), "%s %s",ARROW_LEFT_STRING,tongue_statbar[T_XPLEVEL_LEVELUP]);
		drawStruct.szString = buf;
		re.DrawString( drawStruct );
	}
	
	
	// draw skill highlight
	drawStruct.pImage = re.RegisterPic( "pics/statusbar/selec_skill.tga", NULL, NULL, RESOURCE_INTERFACE );
	drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND;
	drawStruct.nXPos  = 0;
	drawStruct.nYPos  = (viddef.height - SKILLS_WINDOW_TOP) + (xplevelup_selection * SKILLS_LABEL_SPACING) - 3;
	re.DrawPic( drawStruct );
}




#define INV_WINDOW_TOP          (274)
#define INV_WINDOW_LEFT         (57)
#define INV_LABEL_SPACING       (34)
#define INV_CATEGORY_LABEL_TOP  (100)

#define INV_NODULE_LEFT         (57)
#define INV_NODULE_WIDTH        (6)
#define INV_NODULE_HEIGHT       (8)
#define INV_NODULE_SPACING      (8)
#define INV_Y_OFFSET            (9)

#define INVSELECT_WINDOW_LEFT		(INV_WINDOW_LEFT + 4)
#define INVSELECT_WINDOW_TOP    (INV_WINDOW_TOP  - 50)

void draw_inventory_info(void)
// draw inventory windows and information
{
	if (inventory_mode != INVMODE_ITEM_ICONS)
		return;
	
	DRAWSTRUCT drawStruct; 
	
	// draw inventory window
	drawStruct.pImage = re.RegisterPic( "pics/statusbar/inv_window2.tga", NULL, NULL, RESOURCE_INTERFACE );
	drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA;
	drawStruct.nXPos  = viddef.width  - INV_WINDOW_LEFT;
	drawStruct.nYPos  = viddef.height - INV_WINDOW_TOP;
	drawStruct.fAlpha = invmode_hud.alpha;
	re.DrawPic( drawStruct );

// [AJR] HACKAGE: Actually, this graphic shouldn't even be drawing in the FULL version, let alone the demo
#ifndef DAIKATANA_DEMO
	// draw highlighted selection
	drawStruct.pImage = re.RegisterPic( "pics/statusbar/selec_inv.tga", NULL, NULL, RESOURCE_INTERFACE );
	drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA;
	drawStruct.nXPos  = viddef.width  - INVSELECT_WINDOW_LEFT;
	drawStruct.nYPos  = viddef.height - INVSELECT_WINDOW_TOP;
	drawStruct.fAlpha = invmode_hud.alpha;
	re.DrawPic( drawStruct );
#endif

}


qboolean check_weapon_hud(void)
{
	// 10.16 -- if skill levels are shown (checking screen size), also show weapon hud
	if (override_showhuds || scr_viewsize->value < SCR_VIEWSIZE_NOSKILLS)
	{
		skill_hud.direction = 1;       // set skill_hud to fade in
		invmode_hud.direction = 1;
		
		return (true);
	}
	else
	{
		if (!xplevelup_mode)
		{
			skill_hud.direction = -1;       // fade in direction
			skill_hud.fade_time = cl.time;  // start fade NOW
		}

		invmode_hud.direction = -1;
		invmode_hud.fade_time = cl.time;  // start fade NOW
		
		return (false);
	}
}



#define WEAPON_WINDOW_A_TOP            (464)
#define WEAPON_WINDOW_B_TOP            (280)
#define WEAPON_WINDOW_LEFT              (84)
#define WEAPON_SELECTED_WINDOW_TOP     (464)
#define WEAPON_SELECTED_WINDOW_LEFT     (88)


void draw_weapon_info(void)
// draw weapon windows and information
{
	if (inventory_mode != INVMODE_WEAPON_ICONS)
		return;
	
	DRAWSTRUCT drawStruct;
	int i, val, center_x, y, height_spacing;
	char buf[32];
	
	height_spacing = 60;
	int number_offset [] = {-3, 58, 119, 180, 241, 302};	// tweak with the ammo count positions a bit.

/*	
	// draw top weapon window
	drawStruct.pImage = re.RegisterPic( "pics/statusbar/weapn_win_b.tga", NULL, NULL, RESOURCE_INTERFACE );
	drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA;
	drawStruct.nXPos  = viddef.width  - WEAPON_WINDOW_LEFT;
	drawStruct.nYPos  = viddef.height - WEAPON_WINDOW_A_TOP;
	drawStruct.fAlpha = invmode_hud.alpha;
//	re.DrawPic( drawStruct );
	
	
	// draw bottom weapon window
	drawStruct.pImage = re.RegisterPic( "pics/statusbar/weapn_win_b.tga", NULL, NULL, RESOURCE_INTERFACE );
	drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA;
	drawStruct.nXPos  = viddef.width  - WEAPON_WINDOW_LEFT;
	drawStruct.nYPos  = viddef.height - WEAPON_WINDOW_B_TOP;
	drawStruct.fAlpha = invmode_hud.alpha;
//	re.DrawPic( drawStruct );
*/
	// the selected item
	DRAWSTRUCT drawStructSel;
	drawStructSel.pImage = re.RegisterPic( "pics/statusbar/selec_weapn.tga", NULL, NULL, RESOURCE_INTERFACE );
	drawStructSel.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA;
	drawStructSel.nXPos  = viddef.width  - WEAPON_SELECTED_WINDOW_LEFT;
	drawStructSel.nYPos  = (viddef.height - WEAPON_SELECTED_WINDOW_TOP) + (inventory_selected * (height_spacing+1));
	drawStructSel.fAlpha = invmode_hud.alpha;
		
	// draw ammo counts
	drawStruct.nFlags = DSFLAG_BLEND | DSFLAG_PALETTE | DSFLAG_ALPHA;
	drawStruct.pImage = statbar_font;
	
	y = viddef.height - 411;
	
	// the background
	DRAWSTRUCT drawStructPic;
	drawStructPic.pImage = re.RegisterPic( "pics/statusbar/weapn_win.tga", NULL, NULL, RESOURCE_INTERFACE );
	drawStructPic.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA;
	drawStructPic.nXPos  = viddef.width  - WEAPON_SELECTED_WINDOW_LEFT;
	drawStructPic.fAlpha = invmode_hud.alpha;
	for (i = 0; i < 6;i++)
	{
		// draw the weapon bar window
		drawStructPic.nYPos  = (viddef.height - WEAPON_SELECTED_WINDOW_TOP) + (i * (height_spacing+1));
		re.DrawPic( drawStructPic );

		// highlight the selected one
		if ((inventory_selected != -1) && (inventory_selected == i))
		{
			drawStructSel.nYPos  = drawStructPic.nYPos;
			re.DrawPic( drawStructSel );
		}

		if (inventoryIcons[i].count > -1)  // weapon in use
		{
			// if this is the current weapon selected, use the ammo
			// value from the playerstate record
//			if (i == inventory_selected) 
//				val = cl.frame.playerstate.stats[STAT_AMMO];
//			else
				// use the value that was passed down
			val = inventoryIcons[i].count;
			
			// if the ammo count is not -1, draw it (melee weapons use -1, so don't display)
			if (val != -1)
			{
				Com_sprintf(buf,sizeof( buf ), "%d",val);
				
				// determine pixel width to center the string
				center_x = (viddef.width - 28) - (re.StringWidth(buf, statbar_font, -1) >> 1);
				
				drawStruct.szString = buf;
				drawStruct.nXPos    = center_x;
				drawStruct.nYPos    = y + number_offset[i];
				drawStruct.fAlpha   = INFO_ALPHA(invmode_hud.alpha);
				
				re.DrawString( drawStruct );
			}
		}
		
	}
	
}



#define COMMAND_WINDOW_B_TOP            (256)
#define COMMAND_WINDOW_B_LEFT           (128)
#define COMMAND_WINDOW_A_TOP            (COMMAND_WINDOW_B_TOP + 130)
#define COMMAND_WINDOW_A_LEFT           (128)

#define COMMAND_TEXT_CENTERX            (48)
#define COMMAND_TEXT_TOP                (COMMAND_WINDOW_B_TOP - 8)

#define COMMAND_SELECTED_WINDOW_TOP     (COMMAND_WINDOW_B_TOP - 4)
#define COMMAND_SELECTED_WINDOW_LEFT    (128)
#define COMMAND_SELECTED_SPACING         (60)

//  spacing based on table since windows are not spaced consistently
int sidekick_text_y[5] = { COMMAND_TEXT_TOP, 
COMMAND_TEXT_TOP - 36,
COMMAND_TEXT_TOP - 61,
COMMAND_TEXT_TOP - 96,
COMMAND_TEXT_TOP - 121,
};

void draw_sidekickcommand_info(void)
// draw sidekick command interface
{
	if (inventory_mode != INVMODE_SIDEKICK)
		return;
	
	// show sidekick information (only in single-player)
	if (!cl.frame.playerstate.stats[STAT_SIDEKICKS_EXIST])
		return;
	
	if (!cl.sidekicks_active || (cl.refdef.rdflags & RDF_COOP))
		return;
	
	DRAWSTRUCT drawStruct;
	char buf[32];
	int i;
	
	
	// draw commanding sidekick
	if (!cl.sidekick_commanding)
	{
		if (cl.sidekicks_active & SIDEKICK_MIKIKO)
			cl.sidekick_commanding = 2; // ttd: need global here!
		  else
	  if (cl.sidekicks_active & SIDEKICK_SUPERFLY)
		cl.sidekick_commanding = 3; // ttd: need global here!
	}

	if (cl.sidekick_commanding == 2)  // TTD: create dkshared.h const here
		drawStruct.pImage = re.RegisterPic( "pics/statusbar/command_mikiko.tga", NULL, NULL, RESOURCE_INTERFACE );
	else
		drawStruct.pImage = re.RegisterPic( "pics/statusbar/command_superfly.tga", NULL, NULL, RESOURCE_INTERFACE );

	drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA;
	drawStruct.nXPos  = viddef.width  - COMMAND_WINDOW_A_LEFT;
	drawStruct.nYPos  = viddef.height - COMMAND_WINDOW_A_TOP;
	drawStruct.fAlpha = invmode_hud.alpha;
	re.DrawPic( drawStruct );


	// draw command window
	drawStruct.pImage = re.RegisterPic( "pics/statusbar/command_window.tga", NULL, NULL, RESOURCE_INTERFACE );
	drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA;
	drawStruct.nXPos  = viddef.width  - COMMAND_WINDOW_B_LEFT;
	drawStruct.nYPos  = viddef.height - COMMAND_WINDOW_B_TOP;
	drawStruct.fAlpha = invmode_hud.alpha;
	re.DrawPic( drawStruct );


	// draw selected command highlight
	if (sidekickcommand_selected > -1)
	{
		drawStruct.pImage = re.RegisterPic( "pics/statusbar/command_select.tga", NULL, NULL, RESOURCE_INTERFACE );
		drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA;
		drawStruct.nXPos  = viddef.width  - COMMAND_SELECTED_WINDOW_LEFT;
		drawStruct.nYPos  = viddef.height - (sidekick_text_y[sidekickcommand_selected]) - 3;
		drawStruct.fAlpha = invmode_hud.alpha;
		
		re.DrawPic( drawStruct );
	}	

	// draw command text
	drawStruct.nFlags = DSFLAG_BLEND | DSFLAG_PALETTE | DSFLAG_ALPHA;
	drawStruct.pImage = statbar_font;

	for (i = 0; i < 5;i++)
	{
		Com_sprintf(buf,sizeof(buf),"%s",tongue_sidekick[T_SIDEKICK_CMD_GET + i]); // get command text
		drawStruct.szString = buf;
		
		// determine pixel width to center the string
		drawStruct.nXPos  = (viddef.width - COMMAND_TEXT_CENTERX) - (re.StringWidth(buf, statbar_font, -1) >> 1);
		drawStruct.nYPos  = viddef.height - (sidekick_text_y[i]);
		drawStruct.fAlpha = INFO_ALPHA(invmode_hud.alpha);
		
		re.DrawString( drawStruct );
	}
}



int calc_faceframe_index(int health)
// calculate index offset based on health value
{
	if (cl_death_gib)             // gibbed upon death?
		health = 0;                 // show gib face
	else
		if (health < 1)             // boundary check
			health = 1;								// min
		else
			if (health > 100)         // max
				health = 100;
			
			return ((100 - health) / 20);	// index to correct damage face
}


void check_flap_alphas(alphafx_t *flap)
// check/adjust alpha settings for sidekick flap graphics
{
	if (flap->fade_time)                 // active? 
	{
		flap->alpha += (FLAP_ALPHA_RATE * flap->direction);  // change alpha by constant rate
		if (flap->alpha < 0)                                 // check boundary
		{
			flap->alpha     = 0.0f;
			flap->fade_time = 0.0f;                              // no longer show this graphic
		}
		else
			if (flap->alpha > statusbar_alpha->value)          // ceiling is global alpha setting
				flap->alpha = statusbar_alpha->value; 
	}
}


void draw_gameover(void)
// draw 'game over' graphics after the player has died
{
	if (cl_gameover_time && cl.time > cl_gameover_time) // time to display gameover graphics?
	{
		// frags not active?  = single player
		if ( (cl.frame.playerstate.stats[STAT_FRAGS] == NO_FRAGS_USED) && !(cl.frame.playerstate.stats[STAT_LAYOUTS] & LAYOUT_COOP) )
		{
			cl_gameover_alpha += FLAP_ALPHA_RATE_DOUBLEDOUBLE;        // increase alpha
			
			if (cl_gameover_alpha > INVMODE_ALPHA_DEFAULT)            // cap alpha at default ceiling
				cl_gameover_alpha = INVMODE_ALPHA_DEFAULT;
			
			DRAWSTRUCT drawStruct;
			
			drawStruct.pImage = re.RegisterPic( "pics/gameover.tga", NULL, NULL, RESOURCE_INTERFACE );
			drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA;
			drawStruct.nXPos  = (viddef.width  >> 1) - 128;  // middle of horizontal screen
			drawStruct.nYPos  = (viddef.height >> 1) - 128;  // middle of vertical screen
			drawStruct.fAlpha = cl_gameover_alpha;
			
			// TTD: could scale
			
			re.DrawPic( drawStruct );
		}  
	}
}

void draw_theend(void)
{
	if( cl_theend_time && cl.time > cl_theend_time ) // time to display gameover graphics?
	{
		if( cl.frame.playerstate.stats[STAT_FRAGS] == NO_FRAGS_USED )  // frags not active?  = single player
		{
			cl_theend_alpha += FLAP_ALPHA_RATE_DOUBLE;        // increase alpha
			
			if( cl_theend_alpha > INVMODE_ALPHA_DEFAULT )            // cap alpha at default ceiling
				cl_theend_alpha = INVMODE_ALPHA_DEFAULT;
			
			DRAWSTRUCT drawStruct;
			
			drawStruct.pImage = re.RegisterPic( "pics/final_end.tga", NULL, NULL, RESOURCE_INTERFACE );
			drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA;
			drawStruct.nXPos  = (viddef.width  >> 1) - 128;  // middle of horizontal screen
			drawStruct.nYPos  = (viddef.height >> 1) - 128;  // middle of vertical screen
			drawStruct.fAlpha = cl_theend_alpha;
			
			// TTD: could scale
			
			re.DrawPic( drawStruct );
		} 
	}
}

#define SF_STATUS(val)		( ((val) >> 4) & 15)
#define MK_STATUS(val)		( (val) & 15)
static short weapon_status_mikiko = -1;
static short weapon_status_sf = -1;
void CL_ParseSidekickWeaponStatus()
{
	byte status = MSG_ReadByte(&net_message);
	if (cl.refdef.rdflags & RDF_COOP)
	{
		weapon_status_mikiko = weapon_status_sf = -1;	// inactive!
		return;
	}

	weapon_status_sf = SF_STATUS(status);
	if (weapon_status_sf > 3) weapon_status_sf = -1;

	weapon_status_mikiko = MK_STATUS(status);
	if (weapon_status_mikiko > 3) weapon_status_mikiko = -1;
}

typedef struct team_color_info_s
{
	float r,g,b,a;
} team_color_info_t;
extern team_color_info_t cInfo[];

void SCR_HelpLine1(char *str)
{
	if (con_font == NULL)
		con_font = re.LoadFont("int_font");
	DRAWSTRUCT drawStruct;
	drawStruct.fAlpha = INFO_ALPHA(status_hud.alpha);
	drawStruct.nFlags   = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA | DSFLAG_COLOR;
	drawStruct.rgbColor.Set(1,1,0.25);
	drawStruct.szString = str;
	drawStruct.nXPos    = (viddef.width >> 1) - (((strlen(str) - 1) * STATUSBAR_FONT_POWER2) >> 2);
	drawStruct.nYPos	= viddef.height - 95;
	drawStruct.pImage	= con_font;
	re.DrawString(drawStruct);
}

void SCR_HelpLine2(char *str)
{
	if (con_font == NULL)
		con_font = re.LoadFont("int_font");
	DRAWSTRUCT drawStruct;
	drawStruct.fAlpha = INFO_ALPHA(status_hud.alpha);
	drawStruct.nFlags   = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA | DSFLAG_COLOR;
	drawStruct.rgbColor.Set(1,0,0);
	drawStruct.szString = str;
	drawStruct.nXPos    = (viddef.width >> 1) - (((strlen(str) - 1) * STATUSBAR_FONT_POWER2) >> 2);
	drawStruct.nYPos	= viddef.height - 115;
	drawStruct.pImage	= con_font;
	re.DrawString(drawStruct);
}

//---------------------------------------------------------------------------
// SCR_DrawStats()
//---------------------------------------------------------------------------
void SCR_DrawStats (void)
{
	short val, x, y, x1, y1;
	char numstr[64];
	float f;
	int i, x_center, width;
	bool in_the_red;
	void *font;
	CVector rgb;
	DRAWSTRUCT drawStruct;
	
	
	if (cls.state != ca_active || !cl.refresh_prepped)
		return;
	
	// full screen cinematic or letterbox mode.. get out
	if ((CIN_RunningWithView() == 1) || ( cl.refdef.rdflags & RDF_LETTERBOX ) )
	{
		draw_theend();
		draw_gameover();
		return;
	}
	
	if (scr_draw_progress)
		return;

	// value has changed from user pressing 'show_huds' bind?
	val = cl.frame.playerstate.stats[STAT_LAYOUTS] & LAYOUT_SHOWALLHUDS;
	
	if (val != override_showhuds)  // check previous state
	{
		override_showhuds = val;
		check_weapon_hud();          // and change weapon info if necessary
		
		if (override_showhuds)
		{
			if (scr_viewsize->value >= SCR_VIEWSIZE_NOSTATUS) // status hud is not active
				status_hud.direction = 1;                       // so activate it
		}
		else
		{
			if (scr_viewsize->value >= SCR_VIEWSIZE_NOSTATUS) // status hud is not active
			{
				status_hud.direction = -1;                      // fade out
				status_hud.fade_time = cl.time;                 // NOW
			}
		}
	}
    
	
	// update hud settings
	global_hud_updates(&invmode_hud,&inventory_alpha,statusbar_alpha->value);//INVMODE_ALPHA_DEFAULT);  // right-side huds
	global_hud_updates(&skill_hud,  &f,              statusbar_alpha->value);//INVMODE_ALPHA_DEFAULT);  // skill huds
	global_hud_updates(&status_hud, &f,              statusbar_alpha->value); // status bar hud
	
	
	// load status bar fonts
	if (sbar_font_g == NULL)
		sbar_font_g = re.LoadFont("mainnums");
	if (sbar_font_r == NULL)
		sbar_font_r = re.LoadFont("mainnumsred");
	if (statbar_font == NULL)
		statbar_font = re.LoadFont("statbar_font");
	
	
	// screen size set to show status bar?
	//if (override_showhuds || scr_viewsize->value < SCR_VIEWSIZE_NOSTATUS)
	if (override_showhuds || status_hud.alpha)
	{
		
		// precalculate
		x_center = (viddef.width - STATUSBAR_POWER2) >> 1; // center horizontally
		
		rgb.Set(255,255,255);
		
		// draw bottom status bar
		if (statusbar_alpha->value >= 0.01f)  // only show status bar if alpha > 0.1f
		{
			
			drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA;
			
			//drawStruct.fAlpha = statusbar_alpha->value;
			
			drawStruct.fAlpha = status_hud.alpha;
			drawStruct.nYPos  = viddef.height - STATUSBAR_POWER2;       // bottom
			
			
			drawStruct.nXPos  = x_center + bottom_mainbar_offs[0];
			drawStruct.pImage = re.RegisterPic( "pics/statusbar/bottom_1.tga", NULL, NULL, RESOURCE_INTERFACE );
			re.DrawPic( drawStruct );
			
			drawStruct.nXPos  = x_center + bottom_mainbar_offs[1];
			drawStruct.pImage = re.RegisterPic( "pics/statusbar/bottom_3.tga", NULL, NULL, RESOURCE_INTERFACE );
			re.DrawPic( drawStruct );
			
			drawStruct.nXPos  = x_center + bottom_mainbar_offs[2];
			drawStruct.pImage = re.RegisterPic( "pics/statusbar/bottom_4.tga", NULL, NULL, RESOURCE_INTERFACE );
			re.DrawPic( drawStruct );
			
			// various 'faces' per character, get index
			i = cl.baseclientinfo.character * STATUSBAR_FACES_FRAMES;   // initial face index					 
			
			// if player is invisible
			if (cl.frame.playerstate.stats[STAT_INVISIBLE])
				i += STATUSBAR_FACES_INVISIBLE;                     // index to 'invisible' face
			else
			{
				if (cl.frame.playerstate.stats[STAT_INVULNERABLE])
					i += STATUSBAR_FACES_INVULNERABLE;                  // index to 'invulnerable' face
				else
					i += calc_faceframe_index(cl.frame.playerstate.stats[STAT_HEALTH]); // index to health face
			}

			drawStruct.nXPos  = x_center + bottom_mainbar_offs[3];
			drawStruct.pImage = re.RegisterPic( statusbar_faces[i], NULL, NULL, RESOURCE_INTERFACE );
			re.DrawPic( drawStruct );
			
			if (cl.frame.playerstate.stats[STAT_LAYOUTS] & (LAYOUT_CTF|LAYOUT_DT))
			{
				// draw the cool face hilight.--------------------
				// find the color
				int yourTeam = -1;
				int yourFlap = -1;
				if (cl.frame.playerstate.stats[STAT_MIKIKO_FLAGS] & STAT_CTF_YOUR_TEAM)
				{
					yourTeam = (cl.frame.playerstate.stats[STAT_MIKIKO_FLAGS] >> STAT_CTF_TEAM_CONVERT_SHIFT) & 0x000f;
					yourFlap = STAT_MIKIKO_FLAGS;
				}
				else if (cl.frame.playerstate.stats[STAT_SUPERFLY_FLAGS] & STAT_CTF_YOUR_TEAM)
				{
					yourTeam = (cl.frame.playerstate.stats[STAT_SUPERFLY_FLAGS] >> STAT_CTF_TEAM_CONVERT_SHIFT) & 0x000f;
					yourFlap = STAT_SUPERFLY_FLAGS;
				}

				if (yourTeam != -1)
				{
					yourTeam--;
					drawStruct.pImage = re.RegisterPic( "pics/statusbar/bottom_blu.tga", NULL, NULL, RESOURCE_EPISODE );
					drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA | DSFLAG_COLOR;
					drawStruct.nXPos  = viddef.width / 2 - 35;
					drawStruct.nYPos  = viddef.height - 65;
					drawStruct.rgbColor = CVector(cInfo[yourTeam].r,cInfo[yourTeam].g,cInfo[yourTeam].b);
					drawStruct.fAlpha = status_hud.alpha;
					re.DrawPic(drawStruct);
				}

				// draw the team flaps. --------------------------
				// draw the left flap
				drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA;
				drawStruct.pImage = re.RegisterPic( statusbar_faces[STATUSBAR_BLANK_FLAP], NULL, NULL, RESOURCE_INTERFACE );
				drawStruct.nXPos  = 0;
				drawStruct.nYPos  = viddef.height - 87;
				drawStruct.fAlpha = status_hud.alpha;
				re.DrawPic( drawStruct );

				// draw the right flap
				drawStruct.nXPos  = viddef.width - 129;  // right edge
				drawStruct.nFlags |= DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA | DSFLAG_FLIPX;
				re.DrawPic( drawStruct );
				
				// get some info we need for the display
				bool bYouHaveFlag = (yourFlap != -1) && (cl.frame.playerstate.stats[yourFlap] & STAT_CTF_YOU_HAVE_ENEMY_FLAG);
				bool bYourFlagHome = (yourFlap != -1) && (cl.frame.playerstate.stats[yourFlap] & STAT_CTF_YOUR_FLAG_HOME);
				int otherFlap = (yourFlap == STAT_SUPERFLY_FLAGS) ? STAT_MIKIKO_FLAGS : STAT_SUPERFLY_FLAGS;
				bool bOtherFlagHome = (otherFlap != -1) && (cl.frame.playerstate.stats[otherFlap] & STAT_CTF_YOUR_FLAG_HOME);

				// draw the indicator flags!
				if (cl.frame.playerstate.stats[STAT_LAYOUTS] & LAYOUT_CTF)
				{
					drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA | DSFLAG_COLOR | DSFLAG_SCALE;
					drawStruct.pImage = re.RegisterPic( "pics/statusbar/flag_blu.tga", NULL, NULL, RESOURCE_INTERFACE );
					drawStruct.nYPos  = viddef.height - 73;
					drawStruct.fScaleX = drawStruct.fScaleY = 2.0f;
					drawStruct.fAlpha = INFO_ALPHA(status_hud.alpha);

					int teamColor;
					// is team 1's flag home?
					if (cl.frame.playerstate.stats[STAT_MIKIKO_FLAGS] & STAT_CTF_YOUR_FLAG_HOME)
					{
						teamColor = ((cl.frame.playerstate.stats[STAT_MIKIKO_FLAGS] >> STAT_CTF_TEAM_CONVERT_SHIFT) & 0x000f) - 1;
						drawStruct.nXPos  = 17;
						drawStruct.rgbColor = CVector(cInfo[teamColor].r,cInfo[teamColor].g,cInfo[teamColor].b);
						re.DrawPic( drawStruct );
					}

					// is team 2's flag home?
					if (cl.frame.playerstate.stats[STAT_SUPERFLY_FLAGS] & STAT_CTF_YOUR_FLAG_HOME)
					{
						drawStruct.nFlags |= DSFLAG_FLIPX;
						teamColor = ((cl.frame.playerstate.stats[STAT_SUPERFLY_FLAGS] >> STAT_CTF_TEAM_CONVERT_SHIFT) & 0x000f) - 1;
						drawStruct.nXPos  = viddef.width - 83;
						drawStruct.rgbColor = CVector(cInfo[teamColor].r,cInfo[teamColor].g,cInfo[teamColor].b);
						re.DrawPic( drawStruct );
					}

					// do you have a flag?
					if (bYouHaveFlag)
					{
						drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA | DSFLAG_COLOR;
						drawStruct.nYPos = viddef.height - 60;
						if (yourFlap == STAT_MIKIKO_FLAGS)
						{
							drawStruct.nFlags |= DSFLAG_FLIPX;
							drawStruct.nXPos  = 25;
							teamColor = ((cl.frame.playerstate.stats[STAT_SUPERFLY_FLAGS] >> STAT_CTF_TEAM_CONVERT_SHIFT) & 0x000f) - 1;
						}
						else 
						{
							drawStruct.nXPos  = viddef.width - 60;
							teamColor = ((cl.frame.playerstate.stats[STAT_MIKIKO_FLAGS] >> STAT_CTF_TEAM_CONVERT_SHIFT) & 0x000f) - 1;
						}

						drawStruct.rgbColor = CVector(cInfo[teamColor].r,cInfo[teamColor].g,cInfo[teamColor].b);
						re.DrawPic( drawStruct );
					}

					if (cl_dmhints->value)
					{
						if (bYouHaveFlag)
						{
							SCR_HelpLine1(tongue_ctf[T_CTF_MSG_YOU_HAVE_FLAG]);
						}

						if (!bYourFlagHome)
						{
							SCR_HelpLine2(tongue_ctf[T_CTF_MSG_O_HAS_FLAG]);
						}
					}
				}
				else if (cl.frame.playerstate.stats[STAT_LAYOUTS] & LAYOUT_DT)
				{
					if (cl_dmhints->value)
					{
						if (bYouHaveFlag)
						{
							SCR_HelpLine1(tongue_deathtag[TONGUE_DT_MSG_YOU_HAVE_FLAG]);
						}

						if (!bOtherFlagHome)
						{
							SCR_HelpLine2(tongue_deathtag[TONGUE_DT_MSG_O_HAS_FLAG]);
						}
					}
				}

				// draw the team 1 info bars
				sidekick_draw_healtharmor_bar(78, 
					cl.frame.playerstate.stats[STAT_MIKIKO_HEALTH], 
					cl.frame.playerstate.stats[STAT_MIKIKO_ARMOR],
					status_hud.alpha);

				// draw the team 2 info bars
				sidekick_draw_healtharmor_bar(viddef.width  - 97,
					cl.frame.playerstate.stats[STAT_SUPERFLY_HEALTH], 
					cl.frame.playerstate.stats[STAT_SUPERFLY_ARMOR],
					status_hud.alpha);


				// draw the captures
				drawStruct.nFlags   = DSFLAG_COLOR | DSFLAG_BLEND | DSFLAG_ALPHA;
				drawStruct.rgbColor = rgb;
				drawStruct.nYPos    = viddef.height - 38;
				drawStruct.fAlpha   = INFO_ALPHA(status_hud.alpha);

				// team 1
				long captures = (cl.frame.playerstate.stats[STAT_MIKIKO_FLAGS] >> STAT_CTF_TEAM_CAPTURES_SHIFT);
				Com_sprintf(numstr,sizeof(numstr),"%d",captures);
				drawStruct.szString = numstr;
				drawStruct.pImage   = /*(cl.frame.playerstate.stats[STAT_MIKIKO_FLAGS] & STAT_CTF_YOUR_TEAM) ? sbar_font_r : */sbar_font_g;
				drawStruct.nXPos    = 33;
				drawStruct.nXPos	-= ((strlen(numstr) - 1) * STATUSBAR_FONT_POWER2) >> 1;
				re.DrawString( drawStruct );

				// team 2
				captures = (cl.frame.playerstate.stats[STAT_SUPERFLY_FLAGS] >> STAT_CTF_TEAM_CAPTURES_SHIFT);
				Com_sprintf(numstr,sizeof(numstr),"%d",captures);
				drawStruct.szString = numstr;
				drawStruct.pImage   = /*(cl.frame.playerstate.stats[STAT_SUPERFLY_FLAGS] & STAT_CTF_YOUR_TEAM) ? sbar_font_r : */sbar_font_g;
				drawStruct.nXPos    = viddef.width - 43;
				drawStruct.nXPos	-= ((strlen(numstr) - 1) * STATUSBAR_FONT_POWER2) >> 1;
				re.DrawString( drawStruct );

				// draw status label
				long team = (cl.frame.playerstate.stats[STAT_MIKIKO_FLAGS] >> STAT_CTF_TEAM_CONVERT_SHIFT) & 0x000f;
				sprintf(numstr,"%s",tongue_ctf[T_CTF_RED_TEAM_NAME + (team-1)]);				
				drawStruct.nFlags   = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA;
				drawStruct.pImage   = statbar_font;
				drawStruct.nYPos    = viddef.height - 16;
				drawStruct.fAlpha   = INFO_ALPHA(status_hud.alpha);
				drawStruct.szString = numstr;
				drawStruct.nXPos    = 33;//13;
				drawStruct.nXPos	-= ((strlen(numstr) - 1) * STATUSBAR_FONT_POWER2) >> 2;
				re.DrawString(drawStruct);

				team = (cl.frame.playerstate.stats[STAT_SUPERFLY_FLAGS] >> STAT_CTF_TEAM_CONVERT_SHIFT) & 0x000f;
				sprintf(numstr,"%s",tongue_ctf[T_CTF_RED_TEAM_NAME + (team-1)]);				
				drawStruct.szString = numstr;
				drawStruct.nXPos    = viddef.width - 43;
				drawStruct.nXPos	-= ((strlen(numstr) - 1) * STATUSBAR_FONT_POWER2) >> 2;
				re.DrawString(drawStruct);
			}
			else
			{
				// show sidekick information (only in single-player/coop)
				if (!(cl.refdef.rdflags & RDF_DEATHMATCH))
				{
					unsigned long flags = cl.frame.playerstate.stats[STAT_SIDEKICKS_EXIST];
					if (cl.refdef.rdflags & RDF_COOP)
					{
						if (flags & SIDEKICK_SUPERFLY)
							flap_superfly.alpha = status_hud.alpha;
						else
							flap_superfly.alpha = 0;

						if (flags & SIDEKICK_MIKIKO)
							flap_mikiko.alpha = status_hud.alpha;
						else
							flap_mikiko.alpha = 0;
					}
					else// if (flags)  // any sidekicks EXIST?
					{
						check_flap_alphas(&flap_mikiko);
						check_flap_alphas(&flap_superfly);
					}

					bool drawingHiro = false;
					// is mikiko info displaying?
					if (flap_mikiko.alpha)
					{
						if (cl.baseclientinfo.character == 1)	// mikiko is the player ... use hiro flap
						{
							drawingHiro = true;
							if (cl.frame.playerstate.stats[STAT_MIKIKO_FLAGS] & STAT_FLAG_INVISIBLE)
								i = STATUSBAR_FACES_FLAP_HIRO + STATUSBAR_FACES_FLAP_INVIS;
							else if (cl.frame.playerstate.stats[STAT_MIKIKO_FLAGS] & STAT_FLAG_INVULNERABLE)
								i = STATUSBAR_FACES_FLAP_HIRO + STATUSBAR_FACES_FLAP_INVUL;
							else
								i = STATUSBAR_FACES_FLAP_HIRO + calc_faceframe_index(cl.frame.playerstate.stats[STAT_MIKIKO_HEALTH]);
						}
						else
						{
							if (cl.frame.playerstate.stats[STAT_MIKIKO_FLAGS] & STAT_FLAG_INVISIBLE)
								i = STATUSBAR_FACES_FLAP_MIKIKO + STATUSBAR_FACES_FLAP_INVIS;
							else if (cl.frame.playerstate.stats[STAT_MIKIKO_FLAGS] & STAT_FLAG_INVULNERABLE)
								i = STATUSBAR_FACES_FLAP_MIKIKO + STATUSBAR_FACES_FLAP_INVUL;
							else
								i = STATUSBAR_FACES_FLAP_MIKIKO + calc_faceframe_index(cl.frame.playerstate.stats[STAT_MIKIKO_HEALTH]);
						}
						
						drawStruct.pImage = re.RegisterPic( statusbar_faces[i], NULL, NULL, RESOURCE_INTERFACE );
						drawStruct.nFlags |= DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA;
						drawStruct.nXPos  = 0;
						drawStruct.nYPos  = viddef.height - 87;
						drawStruct.fAlpha = flap_mikiko.alpha;
						re.DrawPic( drawStruct );

						// draw the ammo indicator thingy
						if (!(cl.refdef.rdflags & RDF_COOP) && (weapon_status_mikiko != -1))
						{
							// first the pic
							drawStruct.pImage = re.RegisterPic( sidekick_ammo_icons[SIDEKICK_AMMO_BORDER], NULL, NULL, RESOURCE_INTERFACE );
							drawStruct.nFlags |= DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA;
							drawStruct.nXPos  = 69;
							drawStruct.nYPos  = viddef.height - 72;
							drawStruct.fAlpha = flap_mikiko.alpha;
							re.DrawPic( drawStruct );

							// then the icon!
							drawStruct.nXPos  = 75;
							drawStruct.nYPos  = viddef.height - 63;
							drawStruct.fAlpha = INFO_ALPHA(flap_mikiko.alpha);
							drawStruct.pImage = re.RegisterPic( sidekick_ammo_icons[weapon_status_mikiko + SIDEKICK_AMMO_NADA], NULL, NULL, RESOURCE_INTERFACE );
							re.DrawPic( drawStruct );
						}

						// draw health/armor bars
						sidekick_draw_healtharmor_bar(78, 
							cl.frame.playerstate.stats[STAT_MIKIKO_HEALTH], 
							cl.frame.playerstate.stats[STAT_MIKIKO_ARMOR],
							INFO_ALPHA(flap_mikiko.alpha));//flap_mikiko.alpha);
					}
					
					drawingHiro = false;
					if (flap_superfly.alpha)
					{
						drawStruct.nFlags = 0;
						if (cl.baseclientinfo.character == 2)	// sf is the player ... use hiro flap
						{
							drawingHiro = true;
							drawStruct.nXPos  = viddef.width - 129;  // right edge
							drawStruct.nFlags = DSFLAG_FLIPX;
							if (cl.frame.playerstate.stats[STAT_SUPERFLY_FLAGS] & STAT_FLAG_INVISIBLE)
								i = STATUSBAR_FACES_FLAP_HIRO + STATUSBAR_FACES_FLAP_INVIS;
							else if (cl.frame.playerstate.stats[STAT_SUPERFLY_FLAGS] & STAT_FLAG_INVULNERABLE)
								i = STATUSBAR_FACES_FLAP_HIRO + STATUSBAR_FACES_FLAP_INVUL;
							else
								i = STATUSBAR_FACES_FLAP_HIRO + calc_faceframe_index(cl.frame.playerstate.stats[STAT_SUPERFLY_HEALTH]);
						}
						else
						{
							drawStruct.nXPos  = viddef.width - 124;  // right edge
							if (cl.frame.playerstate.stats[STAT_SUPERFLY_FLAGS] & STAT_FLAG_INVISIBLE)
								i = STATUSBAR_FACES_FLAP_SUPERFLY + STATUSBAR_FACES_FLAP_INVIS;
							else if (cl.frame.playerstate.stats[STAT_SUPERFLY_FLAGS] & STAT_FLAG_INVULNERABLE)
								i = STATUSBAR_FACES_FLAP_SUPERFLY + STATUSBAR_FACES_FLAP_INVUL;
							else
								i = STATUSBAR_FACES_FLAP_SUPERFLY + calc_faceframe_index(cl.frame.playerstate.stats[STAT_SUPERFLY_HEALTH]);
						}
						
						drawStruct.pImage = re.RegisterPic( statusbar_faces[i], NULL, NULL, RESOURCE_INTERFACE );
						drawStruct.nFlags |= DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA;
		//				drawStruct.nXPos  = viddef.width - 123;  // right edge
						drawStruct.nYPos  = viddef.height - 87;  // bottom
						drawStruct.fAlpha = flap_superfly.alpha;
						re.DrawPic( drawStruct );
						
						// draw the ammo indicator thingy
						if (!(cl.refdef.rdflags & RDF_COOP) && (weapon_status_sf != -1))
						{
							// first the pic
							drawStruct.pImage = re.RegisterPic( sidekick_ammo_icons[SIDEKICK_AMMO_BORDER], NULL, NULL, RESOURCE_INTERFACE );
							drawStruct.nFlags |= DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA | DSFLAG_FLIPX;
							drawStruct.nXPos  = viddef.width - 135;
							drawStruct.nYPos  = viddef.height - 73;
							drawStruct.fAlpha = flap_superfly.alpha;
							re.DrawPic( drawStruct );

							// then the icon!
							drawStruct.nXPos  = viddef.width - 110;
							drawStruct.nYPos  = viddef.height - 64;
							drawStruct.fAlpha = INFO_ALPHA(flap_superfly.alpha);
							drawStruct.pImage = re.RegisterPic( sidekick_ammo_icons[weapon_status_sf + SIDEKICK_AMMO_NADA], NULL, NULL, RESOURCE_INTERFACE );
							re.DrawPic( drawStruct );
						}

						// draw health/armor bars
						sidekick_draw_healtharmor_bar(viddef.width  - 97,
							cl.frame.playerstate.stats[STAT_SUPERFLY_HEALTH], 
							cl.frame.playerstate.stats[STAT_SUPERFLY_ARMOR],
							INFO_ALPHA(flap_superfly.alpha));//flap_superfly.alpha);

						// draw the 'hiro' name tag
						if (drawingHiro)
						{

						}
					}
				}
			}
		}
		
		// draw floating icons
		for (i = 0;i < 4;i++)
		{ 
			x = x_center + sb_info_icon[i].x_add;
			y = viddef.height + sb_info_icon[i].y_add;
			
			x1 = x_center + sb_info_text[i].x_add;
			y1 = viddef.height + sb_info_text[i].y_add;
			
			if (i == 0)
				x1 += 5;
			if (i == 3)               // frag/level icon
				if (cl.frame.playerstate.stats[STAT_FRAGS] == NO_FRAGS_USED)  // frags not active?
					i++;                  // advance to show exp level icon instead of frag icon
				
				drawStruct.pImage = re.RegisterPic( statusbar_icons[i], NULL, NULL, RESOURCE_EPISODE );
				drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA;
				drawStruct.nXPos  = x;
				drawStruct.nYPos  = y;
				drawStruct.fAlpha = INFO_ALPHA(status_hud.alpha);//status_hud.alpha;
				re.DrawPic(drawStruct);
				
				
#ifdef TONGUE_ENGLISH // only show text in English version
				
				// draw status label
				if ((i == 3) && (cl.frame.playerstate.stats[STAT_LAYOUTS] & (LAYOUT_CTF)))
				{
					Com_sprintf(numstr,sizeof(numstr),"%s",tongue_scoreboard[T_SCOREBOARD_SCORE]);
					for (int k = 0; k < strlen(numstr); k++)
						numstr[k] = toupper(numstr[k]);
				}
				else
					Com_sprintf(numstr,sizeof(numstr),"%s",tongue_statbar[T_STATBAR_ARMOR + i]);
				
				drawStruct.szString = numstr;
				drawStruct.nFlags   = DSFLAG_PALETTE | DSFLAG_BLEND | DSFLAG_ALPHA;
				drawStruct.pImage   = statbar_font;
				drawStruct.nXPos    = x1;
				drawStruct.nYPos    = y1;
				drawStruct.fAlpha   = INFO_ALPHA(status_hud.alpha);;//status_hud.alpha;
				re.DrawString(drawStruct);
				
#endif TONGUE_ENGLISH
				
		}
		
		
		// draw stats on top of status bar
		for (i = 0;i < 4;i++)
		{
			switch (i)
			{
			case 0 : val = cl.frame.playerstate.stats[STAT_ARMOR];   break; 
			case 1 : val = cl.frame.playerstate.stats[STAT_HEALTH];  break;
			case 2 : val = cl.frame.playerstate.stats[STAT_AMMO];    break;
				
			case 3 : if (cl.frame.playerstate.stats[STAT_FRAGS] == NO_FRAGS_USED)  // frags not active?
						 val = cl.frame.playerstate.stats[STAT_LEVEL];   // show exp level 
				else
                    val = cl.frame.playerstate.stats[STAT_FRAGS];   // show frags
				break;
			}
			
			
			// convert stat to string
			itoa(val,numstr,10);
			
			// check for values 'in the red' (danger, will robinson!)
			in_the_red = (val >= stat_warning[i][0] && val <= stat_warning[i][1]);  // between floor/ceiling values
			
			// don't allow health to display below 0
			if (i == 1 && val < 0)
			{
				val = 0;
//				sprintf(numstr,"0")
				Com_sprintf(numstr,sizeof(numstr),"0");
			}
			
			
			x = (viddef.width >> 1) + sb_info[i].x_add;          // set starting x/y
			x -= (strlen(numstr) * STATUSBAR_FONT_POWER2) >> 1;  // center x at this location
			
			if (i == 0)
				x += 5;

			y = viddef.height + sb_info[i].y_add;
			
			if (!in_the_red)
				font = sbar_font_g;
			else
				font = sbar_font_r;
			
			width = strlen(numstr);
			
			drawStruct.nFlags   = DSFLAG_COLOR | DSFLAG_BLEND | DSFLAG_ALPHA;
			drawStruct.rgbColor = rgb;
			drawStruct.szString = numstr;
			drawStruct.pImage   = font;
			drawStruct.nXPos    = x;
			drawStruct.nYPos    = y;
			drawStruct.fAlpha   = INFO_ALPHA(status_hud.alpha);;//status_hud.alpha;
			re.DrawString( drawStruct );
		}
		
	 }
	 
	 draw_skills_info();           // skill levels
	 draw_weapon_info();           // weapons
	 draw_inventory_info();	       // items/weapons
	 draw_sidekickcommand_info();  // sidekick commands
	 draw_xplevel_info();          // draw xplevel up
	 
//	 draw_gameover();

}

