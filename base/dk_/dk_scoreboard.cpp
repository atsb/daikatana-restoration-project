///////////////////////////////////////////////////////////////////////////////
//	dk_scoreboard.c
//
//	routines for drawing deathmatch scoreboard
//	should be moved to client-side DLLs if there is time
//
//	FIXME:	implement with real art instead of printxy
///////////////////////////////////////////////////////////////////////////////

#include	"client.h"
#include	"daikatana.h"

#include    "l__language.h"
#include	"dk_buffer.h"

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define	SB_MAX_CLIENTS			16

#define SB_WIDTH				50//64

#define SB_CTF_MAX_SCORES		2		// WAW: Max Scores.
///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

typedef	struct
{
	short	int		client_index;		//	index into cl.clientinfo
	short	int		frags;				//	number of frags this client has
	//int			exp;				//  experience points
	short	int		level;
	short	int		ping;				//	ping time of this client
	short	int		time_on;			//	time this client has been on server
	short	int		shots_fired;		//	number of shots this client has fired
	short	int		shots_hit;			//	number of shots that hit a player or monster target
	short	int		num_deaths;			//	number of times this player has been killed
	short	int		team;
} playerInfo_t;

typedef struct CTFInfo_s
{
	int				num;
	int				captures;
	int				score;
} CTFInfo_t;

#define MAX_TEAMS	8
typedef	struct
{
	byte			teamplay;			// teamplay deathmatch
	bool			bActive;
	int				total_scores;		//	current number of scores being displayed
	int				top_score;			//	first score in the list (so clients can
								//	scroll through a long list
	int				sx, sy;				//	starting x and starting y
	int				x, y;				//	current x and current y
	int				max_visible_scores;	//	maximum number of lines that can be displayed in 
								//	current video mode
	int				char_width;			//	width of a character
	int				char_height;		//	height of a character + 2 (really height of a line)
	int				up_last_frame;		//	score was displayed last frame
	bool			details;			//  should we display detailed scoreboard?

	playerInfo_t	scores [SB_MAX_CLIENTS];	//	array of client scores
	int				teamscores[MAX_TEAMS];

	bool			ctf;				// WAW[11/16/99]: Turned on when CTF mode is active.
	int				ctf_scores[ SB_CTF_MAX_SCORES ];
	CTFInfo_t		ctf_info[SB_CTF_MAX_SCORES];

	int				episode;
} scoreboard_multi_t;

typedef struct
{
	bool	bActive;

	int		nNumMonsters;
	int		nNumMonstersKilled;
	int		nNumSecrets;
	int		nNumSecretsFound;
	int		nDaikatanaExperience;
	float	fTime;
	float	fTimeTotal;

	int		nStartX, nStartY;
	int		nCurrentX, nCurrentY;
	int		nCharWidth, nCharHeight;
	int		nUpLastFrame;

	int		episode;
} scoreboard_single_t;

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////
scoreboard_multi_t	g_MultiScoreboard;
scoreboard_single_t	g_SingleScoreboard;
extern void *scoreboard_font;

#define STRING_WIDTH(str)				(re.StringWidth(str, scoreboard_font, -1))
void SB_DrawString(int32 x, int32 y, const char *str, int32 max_pixel_width, int32 char_width) 
{
	static DRAWSTRUCT strDrawStruct;
	strDrawStruct.nFlags = DSFLAG_BLEND;
	strDrawStruct.pImage = scoreboard_font;

	// setup drawing info
	strDrawStruct.nXPos = x;
	strDrawStruct.nYPos = y;

    //get the width of the given string.
    int32 width = STRING_WIDTH(str);
    
    //check if the width is greater than the space we have.
    if (width > max_pixel_width) 
	{
        //get the width of the "..." we will put at the end of the string.
        int32 dots_width = STRING_WIDTH("...");
		
        //get the number of characters of the given string that will fit before the dots.
		int32 max_chars = ((max_pixel_width - dots_width) / char_width) - 1;
        int32 num_chars = strlen(str);
		if (num_chars > max_chars)
			num_chars = max_chars;
		
        //copy that many characters from the string to a new buffer.
        char final_str[256];
        strncpy(final_str, str, num_chars);
		
        //put the ... at the end.
        strcpy(&final_str[num_chars], "...");
		
        //draw the string.
		strDrawStruct.szString = final_str;
    }
    else 
	{
        //draw the whole string.
		strDrawStruct.szString = str;
    }
	re.DrawString( strDrawStruct );
}

void SB_DrawTime(int32 x, int32 y, int32 x1)
{
	int time = cl.frame.playerstate.stats[STAT_TIME];

	int nMinutes	= ( time / 60 ) % 60;
	int nHours		= ( time / 3600 );
	int nSeconds	= ( time % 60 );

	SB_DrawString(x,y,tongue_menu[T_MENU_TIME],300,10);
	char string[128];
	Com_sprintf(string,sizeof(string), "%02d:%02d:%02d",nHours,nMinutes,nSeconds);
	SB_DrawString( x1, y, string, 300, 10);
/*
	// SCG[11/9/99]: Draw the Time
	char string[128];
	Com_sprintf(string,sizeof(string), "%s %02d:%02d:%02d",tongue_menu[T_MENU_TIME],nHours,nMinutes,nSeconds);
*/
}
///////////////////////////////////////////////////////////////////////////////
//	sb_InitScoreboard
//
///////////////////////////////////////////////////////////////////////////////

void	sb_InitScoreboard (void)
{
	// SCG[12/20/99]: init multi-player stuff
	memset( &g_MultiScoreboard, 0x00, sizeof( scoreboard_multi_t ) );

	g_MultiScoreboard.char_width = 10;
	g_MultiScoreboard.char_height = 24;//8 + 2;

	g_MultiScoreboard.sx = g_MultiScoreboard.char_width; //(viddef.width - g_MultiScoreboard.char_width * 52) / 2;
	g_MultiScoreboard.sy = (viddef.height - g_MultiScoreboard.char_height * 18) / 2;
	g_MultiScoreboard.x = g_MultiScoreboard.sx;
	g_MultiScoreboard.y = g_MultiScoreboard.sy;

	// preload pictures for scoreboard
	/*

  // let's wait until graphics delivered and used in game   9.29  dsn

	re.RegisterPic("pics/misc/scoreboard/baseb1.pcx", NULL, NULL, RESOURCE_GLOBAL);
	re.RegisterPic("pics/misc/scoreboard/baseb2.pcx", NULL, NULL, RESOURCE_GLOBAL);
    re.RegisterPic("pics/misc/scoreboard/blood1.tga", NULL, NULL, RESOURCE_GLOBAL);
    re.RegisterPic("pics/misc/scoreboard/blood2.tga", NULL, NULL, RESOURCE_GLOBAL);
    re.RegisterPic("pics/misc/scoreboard/blood3.tga", NULL, NULL, RESOURCE_GLOBAL);
    re.RegisterPic("pics/misc/scoreboard/blood4.tga", NULL, NULL, RESOURCE_GLOBAL);
    re.RegisterPic("pics/misc/scoreboard/-2.tga",	  NULL, NULL, RESOURCE_GLOBAL);
    re.RegisterPic("pics/misc/scoreboard/hiro-1.tga", NULL, NULL, RESOURCE_GLOBAL);
    re.RegisterPic("pics/misc/scoreboard/mik0.tga",   NULL, NULL, RESOURCE_GLOBAL);
    re.RegisterPic("pics/misc/scoreboard/hiro+1.tga", NULL, NULL, RESOURCE_GLOBAL);
    re.RegisterPic("pics/misc/scoreboard/+2.tga",	  NULL, NULL, RESOURCE_GLOBAL);
	*/

	// SCG[12/20/99]: init single-player stuff
	memset( &g_SingleScoreboard, 0x00, sizeof( scoreboard_single_t ) );
	g_SingleScoreboard.nCharWidth	= 10;
	g_SingleScoreboard.nCharHeight	= 24;
	g_SingleScoreboard.nStartX		= g_SingleScoreboard.nCharWidth;
	g_SingleScoreboard.nStartY		= ( viddef.height - g_SingleScoreboard.nCharHeight * 18 ) / 2;
	g_SingleScoreboard.nCurrentX	= g_SingleScoreboard.nStartX;
	g_SingleScoreboard.nCurrentY	= g_SingleScoreboard.nStartY;
}

///////////////////////////////////////////////////////////////////////////////
//	sb_ClearScreen
//
///////////////////////////////////////////////////////////////////////////////

void	sb_ClearScreen (void)
{
	dk_print_clear ();

	g_MultiScoreboard.sx = (viddef.width - g_MultiScoreboard.char_width * SB_WIDTH) / 2;
	g_MultiScoreboard.sy = (viddef.height - g_MultiScoreboard.char_height * 18) / 2;
	
	g_MultiScoreboard.x = g_MultiScoreboard.sx;
	g_MultiScoreboard.y = g_MultiScoreboard.sy;
}

typedef struct team_color_info_s
{
	float r,g,b,a;
} team_color_info_t;

team_color_info_t cInfo[] =
{
	{1.0,	0.0,	0.0,	0.075},				// red
	{0.0,	0.0,	1.0,	0.075},				// blue
	{0.7,	0.7,	0.7,	0.075},				// chrome
	{0.5,	0.5,	0.5,	0.075},				// metal
	{0.0,	1.0,	0.0,	0.075},				// green
	{0.8,	0.4,	0.2,	0.075},				// orange
	{1.0,	0.0,	1.0,	0.075},				// purple
	{1.0,	1.0,	0.25,	0.075}				// yellow
};

static short episode_colors[] = 
{
	4,7,0,1
};

#define CTF_X_POS(team,tab)				(g_MultiScoreboard.sx + (tabwidth * ((team * 3) + tab)) - (tabwidth/2))
#define CTF_TAB_PLAYER					0
#define CTF_TAB_KILLS					1.70
#define CTF_TAB_PING					2.4
#define BANNER_PAD						4
//----------------------------------------------------------------------
// sb_DrawScoreboard()
//
//  Description:
//      Queues render commands for displaying the deathmatch scoreboard.
//      Sorts players by frag count. Also displays additional graphical
//      status of just how badass a player really is. A high kill/minute
//      ratio is indicated by the "blood" icon. A high kill/death ratio
//      is indicated by the "head" icon. A bloody DK with a skull means
//      this player is getting a lot of kills/minute and isn't easily 
//      killed.
//
//  Parameters:
//      nada
//
//  Return:
//      nada
//
extern int			scr_centerstring_bottom;
extern float		scr_centertime_off;
void sb_DrawMulti()
{
	int			    i, j, cur_index;
	char		    out [128];
	char		    name [128];
    char            badass[64]={"\0"};  // which playerpic?
    char            blood[64]={"\0"};   // which blood pic?
    int             max_kill_ratio, max_death_ratio, min_kill_ratio, min_death_ratio;
    float           kill_ratio, death_ratio;
    float           player_kill_ratio, player_death_ratio;
    int             blood_level, head_level;    
	char		    *cl_name;
	playerInfo_t	*score;
    int             dw, dh;  // scoreboard background height/width
    int             bh, bw;  // blood indicator height/width
    int             hh, hw;  // hiro/player icon height/width
	int				tabwidth, tabstops;
	
	if (!(cl.frame.playerstate.stats[STAT_LAYOUTS] & LAYOUT_SCOREBOARD))
	{
		if (g_MultiScoreboard.up_last_frame)
		{
			dk_print_clear ();
			g_MultiScoreboard.up_last_frame = 0;
		}

		return;
	}

//==================================
// consolidation change: SCG 3-16-99

	DRAWSTRUCT drawStruct;
	drawStruct.nFlags = DSFLAG_BLEND;
	drawStruct.pImage = scoreboard_font;

	// setup drawing info
	g_MultiScoreboard.y = 100;//(g_MultiScoreboard.ctf || g_MultiScoreboard.teamplay) ? 64 : 96;
	if (scr_centertime_off > 0)
	{
		if (g_MultiScoreboard.y < scr_centerstring_bottom)
			g_MultiScoreboard.y = scr_centerstring_bottom;
	}

	drawStruct.nYPos = g_MultiScoreboard.y+(g_MultiScoreboard.char_height/2);
	tabwidth = (g_MultiScoreboard.char_width * SB_WIDTH);
	tabwidth = tabwidth / (g_MultiScoreboard.details ? 8 : 5);
	tabstops = 0;

	if (g_MultiScoreboard.details && !g_MultiScoreboard.ctf)
	{
		g_MultiScoreboard.sx -= tabwidth * 0.5;
	}
	drawStruct.nXPos = g_MultiScoreboard.sx;

#define TIME_LEFT		(viddef.width - 270)
#define TIME_RIGHT		(viddef.width - 130)
	SB_DrawTime(TIME_LEFT,0,TIME_RIGHT);

	// put the sword level here
	int lev = cl.frame.playerstate.stats[STAT_SWORD_LEVEL];
	char dklev[64];
	if( lev != -1 )
	{
		drawStruct.nYPos = g_MultiScoreboard.char_height + 4;

		drawStruct.nXPos = TIME_LEFT;
		drawStruct.szString = "Daikatana";
		re.DrawString( drawStruct );

		drawStruct.nXPos = TIME_RIGHT;
		Com_sprintf(dklev,sizeof(dklev),"%3.2f / 5",(float)lev / 100.0);
		drawStruct.szString = dklev;
		re.DrawString( drawStruct );
	}

	// put the player level here
	lev = cl.frame.playerstate.stats[STAT_LEVEL];
	drawStruct.nYPos *= 2;
	drawStruct.nXPos = TIME_LEFT;
	drawStruct.szString = tongue_menu[T_MENU_LEVEL];
	re.DrawString( drawStruct );

	drawStruct.nXPos = TIME_RIGHT;
	Com_sprintf(dklev,sizeof(dklev),"%d",lev);
	drawStruct.szString = dklev;
	re.DrawString( drawStruct );


	drawStruct.nXPos = g_MultiScoreboard.sx;
	drawStruct.nYPos = g_MultiScoreboard.y+(g_MultiScoreboard.char_height/2);

	if (g_MultiScoreboard.ctf)
	{
		CTFInfo_t ctf_info1 = g_MultiScoreboard.ctf_info[0];
		CTFInfo_t ctf_info2 = g_MultiScoreboard.ctf_info[1];

		// keep the ctf scoreboard the same size
		tabwidth = (g_MultiScoreboard.char_width * SB_WIDTH);
		tabwidth /= 5;

		{
			// team 1 stat headers
			drawStruct.nXPos = CTF_X_POS(0,CTF_TAB_PLAYER);
			drawStruct.szString = tongue_scoreboard[T_SCOREBOARD_NAME];
			re.DrawString( drawStruct );

			drawStruct.nXPos = CTF_X_POS(0,CTF_TAB_KILLS);
			drawStruct.szString = tongue_scoreboard[T_SCOREBOARD_SCORE];
			re.DrawString( drawStruct );

			drawStruct.nXPos = CTF_X_POS(0,CTF_TAB_PING);
			drawStruct.szString = tongue_scoreboard[T_SCOREBOARD_PING];
			re.DrawString( drawStruct );

			// team 2 stat headers
			drawStruct.nXPos = CTF_X_POS(1,CTF_TAB_PLAYER);
			drawStruct.szString = tongue_scoreboard[T_SCOREBOARD_NAME];
			re.DrawString( drawStruct );

			drawStruct.nXPos = CTF_X_POS(1,CTF_TAB_KILLS);
			drawStruct.szString = tongue_scoreboard[T_SCOREBOARD_SCORE];
			re.DrawString( drawStruct );

			drawStruct.nXPos = CTF_X_POS(1,CTF_TAB_PING);
			drawStruct.szString = tongue_scoreboard[T_SCOREBOARD_PING];
			re.DrawString( drawStruct );
		}

		float headerTop = drawStruct.nYPos;
		float picTop = 0,picBottom1 = 0,picBottom2 = 0;
		picBottom1 = picBottom2 = headerTop + g_MultiScoreboard.char_height;

		// draw the scores!
		{
			// the team stats!
			int nStat1 = g_MultiScoreboard.sx + (tabwidth*0.5);
			int nStat2 = g_MultiScoreboard.sx + (tabwidth * 3.5);

			drawStruct.nYPos = g_MultiScoreboard.y+(g_MultiScoreboard.char_height/2);
			drawStruct.nYPos -= (g_MultiScoreboard.char_height*1.5);
			drawStruct.nXPos = nStat1;

			picTop = drawStruct.nYPos;

			buffer64 buf("%s - %d",tongue_ctf[T_CTF_RED_TEAM_NAME + ctf_info1.num - 1],ctf_info1.captures);
			drawStruct.szString = (const char *)buf;
			re.DrawString( drawStruct );

			drawStruct.nXPos = nStat2;
			buf.Set("%s - %d",tongue_ctf[T_CTF_RED_TEAM_NAME + ctf_info2.num - 1],ctf_info2.captures);
			drawStruct.szString = (const char *)buf;
			re.DrawString( drawStruct );

			// the player stats!
			drawStruct.nYPos = g_MultiScoreboard.y+(g_MultiScoreboard.char_height/2);
			drawStruct.nXPos = g_MultiScoreboard.sx;

			float theTop = drawStruct.nYPos + g_MultiScoreboard.char_height*1.5;
			g_MultiScoreboard.max_visible_scores = (viddef.height - theTop)/(g_MultiScoreboard.char_height * 1.25);
			g_MultiScoreboard.top_score = 0;

			int count[3];
			int team;
			memset(&count,0,3*sizeof(int));

			for (i = 0; i < g_MultiScoreboard.total_scores; i++)
			{
				cur_index = i + g_MultiScoreboard.top_score;
				score = &g_MultiScoreboard.scores [cur_index];
				team = score->team;

				// make sure we can put this guy somewhere...
				if ((team < 1) || (team > 2))
					continue;
				if (count[team] > g_MultiScoreboard.max_visible_scores)
					continue;

				count[team]++;

				//	fill name with spaces
				memset (name, 0x20, 128);
				cl_name = cl.clientinfo [score->client_index].name;
				for (j = 0; cl_name [j] != 0x00; j++)
					name [j] = cl_name [j];
				for (;j <= 24;j++)
					name[j] = NULL;

				// set up the y pos for this entry
				float multiplier = (count[team] == 1) ? 1.5 : 1.25;
				drawStruct.nYPos = g_MultiScoreboard.y + (g_MultiScoreboard.char_height/2) + count[team] * g_MultiScoreboard.char_height*multiplier;

				if (team == 1)
					picBottom1 = drawStruct.nYPos + g_MultiScoreboard.char_height;
				else
					picBottom2 = drawStruct.nYPos + g_MultiScoreboard.char_height;

				team--;		// make it a zero-based index now.
				SB_DrawString(CTF_X_POS(team,CTF_TAB_PLAYER),drawStruct.nYPos,name,CTF_X_POS(team,CTF_TAB_KILLS)-CTF_X_POS(team,CTF_TAB_PLAYER),g_MultiScoreboard.char_width);
//				drawStruct.nXPos = CTF_X_POS(team,CTF_TAB_PLAYER);
//				drawStruct.szString = name;
//				re.DrawString( drawStruct );

// SCG[1/16/00]: 				sprintf( out, "%5i", score->frags );
				Com_sprintf( out,sizeof(out), "%i", score->frags );
				drawStruct.nXPos = CTF_X_POS(team,CTF_TAB_KILLS);
				drawStruct.szString = out;
				re.DrawString( drawStruct );

// SCG[1/16/00]: 				sprintf( out, "%5i", score->ping );
				Com_sprintf( out,sizeof(out), "%i", score->ping );
				drawStruct.nXPos = CTF_X_POS(team,CTF_TAB_PING);
				drawStruct.szString = out;
				re.DrawString( drawStruct );
			}
		}

		// draw the cool color bands
		DRAWSTRUCT drawStructPic;
		memset(&drawStructPic,0,sizeof(drawStructPic));
		drawStructPic.pImage = NULL;
		drawStructPic.nFlags = DSFLAG_FLAT | DSFLAG_COLOR | DSFLAG_ALPHA; 
		drawStructPic.nTop    = picTop;													// y position

		// big block for team 1
		drawStructPic.nBottom = picBottom1 - picTop;									// height of nodule
		drawStructPic.rgbColor = CVector(cInfo[ctf_info1.num - 1].r,cInfo[ctf_info1.num - 1].g,cInfo[ctf_info1.num - 1].b);
		drawStructPic.fAlpha  = cInfo[ctf_info1.num - 1].a;			
		drawStructPic.nLeft   = CTF_X_POS(0,CTF_TAB_PLAYER) - BANNER_PAD;						// x position
		drawStructPic.nRight  = CTF_X_POS(1,CTF_TAB_PLAYER) - drawStructPic.nLeft - BANNER_PAD;	// set width of nodule			
		re.DrawPic( drawStructPic ); // do it

		// big block for team 2
		drawStructPic.nBottom = picBottom2 - picTop;									// height of nodule
		drawStructPic.rgbColor = CVector(cInfo[ctf_info2.num - 1].r,cInfo[ctf_info2.num - 1].g,cInfo[ctf_info2.num - 1].b);
		drawStructPic.fAlpha  = cInfo[ctf_info2.num - 1].a;			
		drawStructPic.nLeft   = CTF_X_POS(1,CTF_TAB_PLAYER) - BANNER_PAD;						// x position
		drawStructPic.nRight  = CTF_X_POS(2,CTF_TAB_PLAYER) - drawStructPic.nLeft - BANNER_PAD;	// set width of nodule			
		re.DrawPic( drawStructPic ); // do it

		// header highlights
		drawStructPic.nTop    = headerTop;												// y position
		drawStructPic.nBottom = g_MultiScoreboard.char_height;				// height of nodule

		// block for team 1
		drawStructPic.rgbColor = CVector(cInfo[ctf_info1.num - 1].r,cInfo[ctf_info1.num - 1].g,cInfo[ctf_info1.num - 1].b);
		drawStructPic.fAlpha  = cInfo[ctf_info1.num - 1].a;			
		drawStructPic.nLeft   = CTF_X_POS(0,CTF_TAB_PLAYER) - BANNER_PAD;						// x position
		drawStructPic.nRight  = CTF_X_POS(1,CTF_TAB_PLAYER) - drawStructPic.nLeft - BANNER_PAD;	// set width of nodule			
		re.DrawPic( drawStructPic ); // do it

		// block for team 2
		drawStructPic.rgbColor = CVector(cInfo[ctf_info2.num - 1].r,cInfo[ctf_info2.num - 1].g,cInfo[ctf_info2.num - 1].b);
		drawStructPic.fAlpha  = cInfo[ctf_info2.num - 1].a;			
		drawStructPic.nLeft   = CTF_X_POS(1,CTF_TAB_PLAYER) - BANNER_PAD;						// x position
		drawStructPic.nRight  = CTF_X_POS(2,CTF_TAB_PLAYER) - drawStructPic.nLeft - BANNER_PAD;	// set width of nodule			
		re.DrawPic( drawStructPic ); // do it

	}
	else		// ------------------------------------------ begin deathmatch / teamplay deathmatch ----------------------------------------
	{
		float bandTop(0),bandLeft(0),bandWidth(0);
		if (g_MultiScoreboard.details)
		{
			drawStruct.szString = tongue_scoreboard[T_SCOREBOARD_NAME];
			re.DrawString( drawStruct );
			tabstops += 4;

			drawStruct.nXPos = g_MultiScoreboard.sx + (tabwidth * tabstops) - (tabwidth/2);
			drawStruct.szString = tongue_scoreboard[T_SCOREBOARD_KILLS];
			re.DrawString( drawStruct );
			tabstops++;

			drawStruct.nXPos = g_MultiScoreboard.sx + (tabwidth * tabstops) - (tabwidth/2);
			drawStruct.szString = tongue_scoreboard[T_SCOREBOARD_PING];
			re.DrawString( drawStruct );
			tabstops++;

			drawStruct.nXPos = g_MultiScoreboard.sx + (tabwidth * tabstops) - (tabwidth/2);
			drawStruct.szString = tongue_scoreboard[T_SCOREBOARD_TIME];
			re.DrawString( drawStruct );
			tabstops++;

			drawStruct.nXPos = g_MultiScoreboard.sx + (tabwidth * tabstops) - (tabwidth/2);
			drawStruct.szString = tongue_scoreboard[T_SCOREBOARD_SHOTS];
			re.DrawString( drawStruct );
			tabstops++;

			drawStruct.nXPos = g_MultiScoreboard.sx + (tabwidth * tabstops) - (tabwidth/2);
			drawStruct.szString = tongue_scoreboard[T_SCOREBOARD_HITS];
			re.DrawString( drawStruct );
			tabstops++;

			drawStruct.nXPos = g_MultiScoreboard.sx + (tabwidth * tabstops) - (tabwidth/2);
			drawStruct.szString = tongue_scoreboard[T_SCOREBOARD_DIED];
			re.DrawString( drawStruct );

			bandTop = drawStruct.nYPos;
			bandLeft = g_MultiScoreboard.sx - 3;
			bandWidth = drawStruct.nXPos + tabwidth + 3 - bandLeft;

			g_MultiScoreboard.y += g_MultiScoreboard.char_height*1.5;
		}
		else
		{
			drawStruct.szString = tongue_scoreboard[T_SCOREBOARD_NAME];
			re.DrawString( drawStruct );
			tabstops += 3;

			drawStruct.nXPos = g_MultiScoreboard.sx + (tabwidth * tabstops) - (tabwidth/2);
			drawStruct.szString = tongue_scoreboard[T_SCOREBOARD_KILLS];
			re.DrawString( drawStruct );
			tabstops ++;

			drawStruct.nXPos = g_MultiScoreboard.sx + (tabwidth * tabstops) - (tabwidth/2);
			drawStruct.szString = tongue_scoreboard[T_SCOREBOARD_PING];
			re.DrawString( drawStruct );
			tabstops ++;

			drawStruct.nXPos = g_MultiScoreboard.sx + (tabwidth * tabstops) - (tabwidth/2);
			drawStruct.szString = tongue_scoreboard[T_SCOREBOARD_TIME];
			re.DrawString( drawStruct );

			bandTop = drawStruct.nYPos;
			bandLeft = g_MultiScoreboard.sx - 3;
			bandWidth = drawStruct.nXPos + tabwidth + 3 - bandLeft;

			g_MultiScoreboard.y += g_MultiScoreboard.char_height*1.5;
		}
					  
		// draw the cool color bands
		DRAWSTRUCT drawStructPic;
		memset(&drawStructPic,0,sizeof(drawStructPic));
		drawStructPic.pImage = NULL;
		drawStructPic.nFlags = DSFLAG_FLAT | DSFLAG_COLOR | DSFLAG_ALPHA; 
		drawStructPic.rgbColor = CVector(cInfo[episode_colors[g_MultiScoreboard.episode]].r,
										 cInfo[episode_colors[g_MultiScoreboard.episode]].g,
										 cInfo[episode_colors[g_MultiScoreboard.episode]].b);
		drawStructPic.fAlpha  = cInfo[episode_colors[g_MultiScoreboard.episode]].a;
		
		drawStructPic.nTop    = bandTop;													// y position
		drawStructPic.nLeft   = bandLeft;												//	 x position
		drawStructPic.nRight  = bandWidth;										// set width of nodule			
		drawStructPic.nBottom = g_MultiScoreboard.char_height + 3;										// height of nodule
		re.DrawPic( drawStructPic ); // do it
		// re.DrawFill(g_MultiScoreboard.sx - 4, g_MultiScoreboard.y + 4 , (g_MultiScoreboard.char_width * 52) + 4, 1, 55);
	// consolidation change: SCG 3-16-99
	//==================================

		
		// excessively long version of scoring calculations
		// this will be shortened!
		g_MultiScoreboard.max_visible_scores = (viddef.height - g_MultiScoreboard.y)/(g_MultiScoreboard.char_height * 1.5);
		g_MultiScoreboard.top_score = 0;
		dw = g_MultiScoreboard.char_width * SB_WIDTH+32;
		dh = g_MultiScoreboard.char_height * 2;
		bh = dh;
		bw = 22;
		hh = dh;
		hw = 36;
		max_kill_ratio = max_death_ratio = min_kill_ratio = min_death_ratio = 0;

		bool teamScoreShown[MAX_TEAMS];
		memset(&teamScoreShown,0,sizeof(teamScoreShown));
		// run through scoreboard once to get min/max info
		// for rank/comparison
		for(i=0; i< SB_MAX_CLIENTS;i++)
		{
			//    int             max_kill_ratio, max_death_ratio, min_kill_ratio, min_death_ratio;
			score = &g_MultiScoreboard.scores[i];
			if(score->time_on > 0)
				kill_ratio=score->frags / score->time_on;
			else
				kill_ratio=score->frags;
			if(max_kill_ratio < kill_ratio)
				max_kill_ratio = kill_ratio;
			else if(min_kill_ratio > kill_ratio)
				min_kill_ratio = kill_ratio;
			if(score->num_deaths > 0)
				death_ratio = score->frags / score->num_deaths;
			else
				death_ratio = score->frags;
			if(max_death_ratio < death_ratio)
				max_death_ratio = death_ratio;
			else if(min_death_ratio > death_ratio)
				min_death_ratio = death_ratio;
		}

		// The median has been determined, calculate levels as:
		// stat:  min                    MEDIAN                     max
		//        +-------------------------+-------------------------+
		// norm:  0.0    |    0.3    |     0.5     |    0.6    |    1.0
		// level: -2     |    -1     |      0      |     1     |      2
		// 
		char scoreString[16];
		for (i = 0; i < g_MultiScoreboard.total_scores && i < g_MultiScoreboard.max_visible_scores; i++)
		{
			cur_index = i + g_MultiScoreboard.top_score;
			score = &g_MultiScoreboard.scores [cur_index];
			// player stats
			if(score->time_on > 0)
				player_kill_ratio = score->frags/score->time_on;
			else
				player_kill_ratio = score->frags;
			if(score->num_deaths > 0)
				player_death_ratio = score->frags/score->num_deaths;
			else
				player_death_ratio = score->frags;

			// compare
			death_ratio = max_death_ratio - min_death_ratio;
			if(death_ratio > 0)
				player_death_ratio = player_death_ratio-min_death_ratio/death_ratio;
			else
				player_death_ratio = 0.5;   // default head
			if (player_death_ratio < 0.15)
				head_level = -2;
			else if(player_death_ratio < 0.415)
				head_level = -1;
			else if(player_death_ratio < 0.585)
				head_level = 0;
			else if(player_death_ratio < 74.5)
				head_level = 1;
			else
				head_level = 2;

			kill_ratio = max_kill_ratio - min_kill_ratio;
			if(kill_ratio > 0)
				player_kill_ratio = player_kill_ratio-min_kill_ratio / kill_ratio;
			else
				player_kill_ratio = score->frags;
			if(player_kill_ratio < 0.15)
				blood_level = -2;
			else if(player_kill_ratio < 0.415)
				blood_level = -1;
			else if(player_kill_ratio < 0.585)
				blood_level = 0;
			else if(player_kill_ratio < 74.5)
				blood_level = 1;
			else
				blood_level = 2;
	/*  DO NOT DELETE, WAITING ON NEW ARTWORK
			// Player score background
			re.DrawStretchPic( 
				g_MultiScoreboard.sx + 32, 
				g_MultiScoreboard.y, 
				dw / 2, 
				dh, 
				"pics/misc/scoreboard/baseb1.pcx", 
				RESOURCE_GLOBAL);
			re.DrawStretchPic( 
				g_MultiScoreboard.sx + (dw / 2) - 32, 
				g_MultiScoreboard.y, 
				dw / 2,
				dh, 
				"pics/misc/scoreboard/baseb2.pcx", 
				RESOURCE_GLOBAL);

			// Bloody
			switch(blood_level)
			{
			case -2:
				re.DrawStretchPic( g_MultiScoreboard.sx + 32 - bw, g_MultiScoreboard.y, bw, bh, "pics/misc/scoreboard/blood1.pcx", RESOURCE_GLOBAL);
				break;
			case -1:
				re.DrawStretchPic( g_MultiScoreboard.sx + 32 - bw, g_MultiScoreboard.y, bw, bh, "pics/misc/scoreboard/blood1.pcx", RESOURCE_GLOBAL);
				break;
			case 0:
				re.DrawStretchPic( g_MultiScoreboard.sx + 32 - bw, g_MultiScoreboard.y, bw, bh, "pics/misc/scoreboard/blood2.pcx", RESOURCE_GLOBAL);
				break;
			case 1:
				re.DrawStretchPic( g_MultiScoreboard.sx + 32 - bw, g_MultiScoreboard.y, bw, bh, "pics/misc/scoreboard/blood3.pcx", RESOURCE_GLOBAL);
				break;
			case 2:
				re.DrawStretchPic( g_MultiScoreboard.sx + 32 - bw, g_MultiScoreboard.y, bw, bh, "pics/misc/scoreboard/blood4.pcx", RESOURCE_GLOBAL);
				break;
			}

			// Bad Ass
			// Only Hiro is used right now        
			switch(head_level)
			{
			case -2:
				re.DrawStretchPic( g_MultiScoreboard.sx + dw-32, g_MultiScoreboard.y, bw, bh, "pics/misc/scoreboard/-2.pcx", RESOURCE_GLOBAL);
				break;
			case -1:
				re.DrawStretchPic( g_MultiScoreboard.sx + dw-32, g_MultiScoreboard.y, bw, bh, "pics/misc/scoreboard/hiro-1.pcx", RESOURCE_GLOBAL);
				break;
			case 0:
				re.DrawStretchPic( g_MultiScoreboard.sx + dw-32, g_MultiScoreboard.y, bw, bh, "pics/misc/scoreboard/mik0.pcx", RESOURCE_GLOBAL);
				break;
			case 1:
				re.DrawStretchPic( g_MultiScoreboard.sx + dw-32, g_MultiScoreboard.y, bw, bh, "pics/misc/scoreboard/hiro+1.pcx", RESOURCE_GLOBAL);
				break;
			case 2:
				re.DrawStretchPic( g_MultiScoreboard.sx + dw-32, g_MultiScoreboard.y, bw, bh, "pics/misc/scoreboard/+2.pcx", RESOURCE_GLOBAL);
				break;
			}
	*/		
			//	fill name with spaces
/*
			memset (name, 0x20, 128);
			cl_name = cl.clientinfo [score->client_index].name;
			for (j = 0; cl_name [j] != 0x00; j++)
				name [j] = cl_name [j];
			for (;j <= 24;j++)
				name[j] = NULL;
*/
			int Team = score->team - 1;
			if ((g_MultiScoreboard.teamplay & 2) && (!teamScoreShown[Team]))
			{
				if (!teamScoreShown[Team])
				{
					Com_sprintf(scoreString,sizeof(scoreString),"%-d",g_MultiScoreboard.teamscores[Team]);
					Com_sprintf(name,sizeof(name),"%s",cl.clientinfo[score->client_index].name);
					teamScoreShown[Team] = true;
				}
				else
				{
					Com_sprintf(scoreString,sizeof(scoreString),"");
					Com_sprintf(name,sizeof(name),"%s",cl.clientinfo[score->client_index].name);
				}
			}
			else
			{
				Com_sprintf(scoreString,sizeof(scoreString)," ");
				Com_sprintf(name,sizeof(name),"%s",cl.clientinfo[score->client_index].name);
			}
			drawStruct.nXPos = g_MultiScoreboard.sx;
			drawStruct.nYPos = g_MultiScoreboard.y+(g_MultiScoreboard.char_height/2);
			tabstops = 0;
			memset( out, 0, strlen( out ) );

			if (g_MultiScoreboard.details)
			{
				if (g_MultiScoreboard.teamplay)// & 2)
				{
					if (strlen(scoreString))
						SB_DrawString(drawStruct.nXPos,drawStruct.nYPos,scoreString,g_MultiScoreboard.char_width * 5,g_MultiScoreboard.char_width);					
					SB_DrawString(drawStruct.nXPos + g_MultiScoreboard.char_width * 5,drawStruct.nYPos,name,2.25 * tabwidth,g_MultiScoreboard.char_width);					
				}
				else
				{
					SB_DrawString(drawStruct.nXPos,drawStruct.nYPos,name,2.5 * tabwidth,g_MultiScoreboard.char_width);
				}
//#define STRING_WIDTH(str)				(re.StringWidth(str, scoreboard_font, -1))
//void SB_DrawString(int32 x, int32 y, const char *str, int32 max_pixel_width, int32 char_width) 
//				drawStruct.szString = name;
//				re.DrawString( drawStruct );
				tabstops += 4;
/*				if (strlen(scoreString))
				{
					SB_DrawString(drawStruct.nXPos,drawStruct.nYPos,name,g_MultiScoreboard.char_width * 5,g_MultiScoreboard.char_width);					
					SB_DrawString(drawStruct.nXPos + g_MultiScoreboard.char_width * 5,drawStruct.nYPos,name,2.25 * tabwidth,g_MultiScoreboard.char_width);					
				}
				else
				{
					SB_DrawString(drawStruct.nXPos,drawStruct.nYPos,name,2.5 * tabwidth,g_MultiScoreboard.char_width);
				}
*/				
				Com_sprintf( out,sizeof(out), "%i", score->frags );
				drawStruct.nXPos = g_MultiScoreboard.sx + (tabwidth * tabstops) - (tabwidth/2);
				drawStruct.szString = out;
				re.DrawString( drawStruct );
				tabstops++;

				Com_sprintf( out,sizeof(out), "%i", score->ping );
				drawStruct.nXPos = g_MultiScoreboard.sx + (tabwidth * tabstops) - (tabwidth/2);
				drawStruct.szString = out;
				re.DrawString( drawStruct );
				tabstops++;

				Com_sprintf( out,sizeof(out), "%i", score->time_on );
				drawStruct.nXPos = g_MultiScoreboard.sx + (tabwidth * tabstops) - (tabwidth/2);
				drawStruct.szString = out;
				re.DrawString( drawStruct );
				tabstops++;

				Com_sprintf( out,sizeof(out), "%i", score->shots_fired );
				drawStruct.nXPos = g_MultiScoreboard.sx + (tabwidth * tabstops) - (tabwidth/2);
				drawStruct.szString = out;
				re.DrawString( drawStruct );
				tabstops++;

				Com_sprintf( out,sizeof(out), "%i", score->shots_hit );
				drawStruct.nXPos = g_MultiScoreboard.sx + (tabwidth * tabstops) - (tabwidth/2);
				drawStruct.szString = out;
				re.DrawString( drawStruct );
				tabstops++;

				Com_sprintf( out,sizeof(out), "%i", score->num_deaths );
				drawStruct.nXPos = g_MultiScoreboard.sx + (tabwidth * tabstops) - (tabwidth/2);
				drawStruct.szString = out;
				re.DrawString( drawStruct );

				int team = score->team - 1;
				if (g_MultiScoreboard.teamplay && (team >= 0) && (team <= 7))
				{
					drawStructPic.rgbColor = CVector(cInfo[team].r, cInfo[team].g, cInfo[team].b);
					drawStructPic.fAlpha  = cInfo[team].a;
					
					drawStructPic.nTop    = drawStruct.nYPos;													// y position
					drawStructPic.nLeft   = g_MultiScoreboard.sx - 3;												//	 x position
					drawStructPic.nRight  = drawStruct.nXPos + tabwidth + 3 - drawStructPic.nLeft;					// set width of nodule			
					drawStructPic.nBottom = g_MultiScoreboard.char_height + 3;										// height of nodule
					re.DrawPic( drawStructPic ); // do it
					re.DrawPic( drawStructPic ); // do it
				}

				if (g_MultiScoreboard.teamplay)
					g_MultiScoreboard.y += g_MultiScoreboard.char_height + 3;
				else
					g_MultiScoreboard.y += g_MultiScoreboard.char_height*1.25;
			}
			else
			{
				if (g_MultiScoreboard.teamplay)// & 2)
				{
					if (strlen(scoreString))
						SB_DrawString(drawStruct.nXPos,drawStruct.nYPos,scoreString,g_MultiScoreboard.char_width * 5,g_MultiScoreboard.char_width);					
					SB_DrawString(drawStruct.nXPos + g_MultiScoreboard.char_width * 5,drawStruct.nYPos,name,2.25 * tabwidth,g_MultiScoreboard.char_width);					
				}
				else
				{
					SB_DrawString(drawStruct.nXPos,drawStruct.nYPos,name,2.5 * tabwidth,g_MultiScoreboard.char_width);
				}
//				SB_DrawString(drawStruct.nXPos,drawStruct.nYPos,name,2.5 * tabwidth,g_MultiScoreboard.char_width);
//				drawStruct.szString = name;
//				re.DrawString( drawStruct );
				tabstops += 3;
				
				Com_sprintf( out,sizeof(out), "%i", score->frags );
				drawStruct.nXPos = g_MultiScoreboard.sx + (tabwidth * tabstops) - (tabwidth/2);
				drawStruct.szString = out;
				re.DrawString( drawStruct );
				tabstops++;

				Com_sprintf( out, sizeof(out), "%i", score->ping );
				drawStruct.nXPos = g_MultiScoreboard.sx + (tabwidth * tabstops) - (tabwidth/2);
				drawStruct.szString = out;
				re.DrawString( drawStruct );
				tabstops++;

				Com_sprintf( out,sizeof(out), "%i", score->time_on );
				drawStruct.nXPos = g_MultiScoreboard.sx + (tabwidth * tabstops) - (tabwidth/2);
				drawStruct.szString = out;
				re.DrawString( drawStruct );
				tabstops++;

				int team = score->team - 1;
				if (g_MultiScoreboard.teamplay && (team >= 0) && (team <= 7))
				{
					drawStructPic.rgbColor = CVector(cInfo[team].r, cInfo[team].g, cInfo[team].b);
					drawStructPic.fAlpha  = cInfo[team].a;
					
					drawStructPic.nTop    = drawStruct.nYPos;													// y position
					drawStructPic.nLeft   = g_MultiScoreboard.sx - 3;												//	 x position
					drawStructPic.nRight  = drawStruct.nXPos + tabwidth + 3 - drawStructPic.nLeft;										// set width of nodule			
					drawStructPic.nBottom = g_MultiScoreboard.char_height + 3;										// height of nodule
					re.DrawPic( drawStructPic ); // do it
					re.DrawPic( drawStructPic ); // do it
				}

				if (g_MultiScoreboard.teamplay)
					g_MultiScoreboard.y += g_MultiScoreboard.char_height + 3;
				else
					g_MultiScoreboard.y += g_MultiScoreboard.char_height*1.25;
			}       
		}

		if (!g_MultiScoreboard.teamplay)
		{
			drawStructPic.nBottom = g_MultiScoreboard.y + 8 - drawStructPic.nTop;		// height of nodule
			re.DrawPic( drawStructPic ); // do it
		}
	}
   
	g_MultiScoreboard.up_last_frame = 1;
}
void sb_DrawSingle()
{
	int		tabwidth, tabstops;
	char	string[128];

	if (!(cl.frame.playerstate.stats[STAT_LAYOUTS] & LAYOUT_SCOREBOARD))
	{
		if( g_SingleScoreboard.nUpLastFrame )
		{
			dk_print_clear();
			g_SingleScoreboard.nUpLastFrame = 0;
		}
		return;
	}

	g_SingleScoreboard.nNumMonsters			= cl.frame.playerstate.stats[STAT_MONSTERS];		// SCG[12/20/99]: number of monsters
	g_SingleScoreboard.nNumMonstersKilled	= cl.frame.playerstate.stats[STAT_KILLS];			// SCG[12/20/99]: number of monsters killed
	g_SingleScoreboard.nNumSecrets			= cl.frame.playerstate.stats[STAT_SECRETS];			// SCG[12/20/99]: number of secrets
	g_SingleScoreboard.nNumSecretsFound		= cl.frame.playerstate.stats[STAT_FOUND_SECRETS];	// SCG[12/20/99]: number of secrets found
	g_SingleScoreboard.nDaikatanaExperience	= cl.frame.playerstate.stats[STAT_SWORD_LEVEL];		// SCG[12/20/99]: current daikatana level
	g_SingleScoreboard.fTime				= cl.frame.playerstate.stats[STAT_TIME];			// SCG[12/20/99]: time
	g_SingleScoreboard.fTimeTotal			= cl.frame.playerstate.stats[STAT_TOTAL_TIME];		// SCG[12/20/99]: total time

	tabwidth			= g_SingleScoreboard.nCharWidth * SB_WIDTH;
	tabwidth			= tabwidth / 4;
/*
	g_SingleScoreboard.nStartX	= 
		strlen( "Monsters" ) + 
		strlen( "Secrets" ) + 
		strlen( "Time" ) + 
		strlen( "Total Time" ) + ( tabwidth * 2 );
	if( g_SingleScoreboard.nDaikatanaExperience != -1 )
	{
		g_SingleScoreboard.nStartX	+= strlen( "Daikatana" ) + tabwidth;
	}
*/
/*	g_SingleScoreboard.nStartX = 
		STRING_WIDTH( "Monsters" ) + 
		STRING_WIDTH( "Secrets" ) +
		STRING_WIDTH( "Time" ) +
		STRING_WIDTH( "Total Time" );// + (tabwidth * 2 );
	if( g_SingleScoreboard.nDaikatanaExperience != -1 )
	{
		g_SingleScoreboard.nStartX	+= STRING_WIDTH( "Daikatana" );// + tabwidth;
	}
*/
	g_SingleScoreboard.nStartX = (tabwidth * 3);
	if( g_SingleScoreboard.nDaikatanaExperience != -1 )
	{
		g_SingleScoreboard.nStartX	+= tabwidth;
	}
	float barWidth = g_SingleScoreboard.nStartX + tabwidth;
	g_SingleScoreboard.nStartX = viddef.width - g_SingleScoreboard.nStartX;
	g_SingleScoreboard.nStartX *= 0.5;

//	g_SingleScoreboard.nCurrentY	= ( viddef.height * 0.5 ) - 32;
	g_SingleScoreboard.nCurrentY	= 64;

	DRAWSTRUCT drawStruct;
	drawStruct.nFlags	= DSFLAG_BLEND;
	drawStruct.pImage	= scoreboard_font;
	drawStruct.nXPos	= g_SingleScoreboard.nStartX;
	drawStruct.nYPos	= g_SingleScoreboard.nCurrentY + ( g_SingleScoreboard.nCharHeight / 2 );
/*
	DRAWSTRUCT drawStruct;
	drawStruct.nFlags	= DSFLAG_BLEND;
	drawStruct.pImage	= scoreboard_font;
	drawStruct.nXPos	= g_SingleScoreboard.nStartX;
	drawStruct.nYPos	= g_SingleScoreboard.nCurrentY + ( g_SingleScoreboard.nCharHeight / 2 );
*/
	tabstops = 0;

	// draw the cool color bands
	DRAWSTRUCT drawStructPic;
	memset(&drawStructPic,0,sizeof(drawStructPic));
	drawStructPic.pImage = NULL;
	drawStructPic.nFlags = DSFLAG_FLAT | DSFLAG_COLOR | DSFLAG_ALPHA; 
	drawStructPic.rgbColor = CVector(cInfo[episode_colors[g_SingleScoreboard.episode]].r,
									 cInfo[episode_colors[g_SingleScoreboard.episode]].g,
									 cInfo[episode_colors[g_SingleScoreboard.episode]].b);
	drawStructPic.fAlpha  = cInfo[episode_colors[g_SingleScoreboard.episode]].a;
	
	drawStructPic.nTop    = drawStruct.nYPos;													// y position
	drawStructPic.nBottom = g_SingleScoreboard.nCharHeight + 3;										// height of nodule
	drawStructPic.nLeft   = g_SingleScoreboard.nStartX - 3 + ( tabwidth * tabstops ) - ( tabwidth / 2 );// x position
	drawStructPic.nRight  = barWidth + 3;															// set width of nodule			
	re.DrawPic( drawStructPic ); // do it
	drawStructPic.nBottom += g_SingleScoreboard.nCharHeight*1.5;										// height of nodule
	re.DrawPic( drawStructPic ); // do it

	if( g_SingleScoreboard.nDaikatanaExperience != -1 )
	{
		// SCG[12/21/99]: Draw "Daikatana" text
		drawStruct.nXPos = g_SingleScoreboard.nStartX + ( tabwidth * tabstops ) - ( tabwidth / 2 );
		drawStruct.szString = "Daikatana";
		re.DrawString( drawStruct );
		tabstops++;
	}
	// SCG[12/21/99]: Draw "Monsters" text
	drawStruct.nXPos = g_SingleScoreboard.nStartX + ( tabwidth * tabstops ) - ( tabwidth / 2 );
	drawStruct.szString = tongue_menu[T_MENU_MONSTERS];//"Monsters";
	re.DrawString( drawStruct );
	tabstops++;
	// SCG[12/21/99]: Draw "Secrets" text
	drawStruct.nXPos = g_SingleScoreboard.nStartX + ( tabwidth * tabstops ) - ( tabwidth / 2 );
	drawStruct.szString = tongue_menu[T_MENU_SECRETS];//"Secrets";
	re.DrawString( drawStruct );
	tabstops++;
	// SCG[12/21/99]: Draw "Time" text
	drawStruct.nXPos = g_SingleScoreboard.nStartX + ( tabwidth * tabstops ) - ( tabwidth / 2 );
	drawStruct.szString = tongue_menu[T_MENU_TIME];//"Time";
	re.DrawString( drawStruct );
	tabstops++;
	// SCG[12/21/99]: Draw "Total Time" text
	drawStruct.nXPos = g_SingleScoreboard.nStartX + ( tabwidth * tabstops ) - ( tabwidth / 2 );
	drawStruct.szString = tongue_menu[T_MENU_TOTAL_TIME];//"Total Time";
	re.DrawString( drawStruct );
	tabstops++;

	tabstops = 0;
	drawStruct.nYPos += g_SingleScoreboard.nCharHeight * 1.5;

	// SCG[12/21/99]: Draw "Daikatana" value
	if( g_SingleScoreboard.nDaikatanaExperience != -1 )
	{
		drawStruct.nXPos = g_SingleScoreboard.nStartX + ( tabwidth * tabstops ) - ( tabwidth / 2 );
		Com_sprintf( string,sizeof(string), "%3.2f / 5", (float)g_SingleScoreboard.nDaikatanaExperience / 100.0);
		drawStruct.szString = string;
		re.DrawString( drawStruct );
		tabstops++;
	}

	// SCG[12/21/99]: Draw "Monsters" value
	drawStruct.nXPos = g_SingleScoreboard.nStartX + ( tabwidth * tabstops ) - ( tabwidth / 2 );
	Com_sprintf( string,sizeof(string), "%d / %d", g_SingleScoreboard.nNumMonstersKilled, g_SingleScoreboard.nNumMonsters );
	drawStruct.szString = string;
	re.DrawString( drawStruct );
	tabstops++;
	// SCG[12/21/99]: Draw "Secrets" value
	drawStruct.nXPos = g_SingleScoreboard.nStartX + ( tabwidth * tabstops ) - ( tabwidth / 2 );
	Com_sprintf( string,sizeof(string), "%d / %d", g_SingleScoreboard.nNumSecretsFound, g_SingleScoreboard.nNumSecrets );
	drawStruct.szString = string;
	re.DrawString( drawStruct );
	tabstops++;
	// SCG[12/21/99]: Draw "Time" value
	drawStruct.nXPos	= g_SingleScoreboard.nStartX + ( tabwidth * tabstops ) - ( tabwidth / 2 );
	int nMinutes		= ( (int)( g_SingleScoreboard.fTime / 60.0 ) % 60 );
	int nHours			= ( g_SingleScoreboard.fTime / 3600.0 );
	int nSeconds		= ( ( int )g_SingleScoreboard.fTime % 60 );
	Com_sprintf( string,sizeof(string), "%02d:%02d:%02d", nHours, nMinutes, nSeconds );
	drawStruct.szString = string;
	re.DrawString( drawStruct );
	tabstops++;

	// SCG[12/21/99]: Draw "Total Time" value
	drawStruct.nXPos	= g_SingleScoreboard.nStartX + ( tabwidth * tabstops ) - ( tabwidth / 2 );
	nMinutes			= ( (int)( g_SingleScoreboard.fTimeTotal / 60.0 ) % 60 );
	nHours				= ( g_SingleScoreboard.fTimeTotal / 3600 );
	nSeconds			= ( ( int )g_SingleScoreboard.fTimeTotal % 60 );
	Com_sprintf( string,sizeof(string), "%02d:%02d:%02d", nHours, nMinutes, nSeconds );
	drawStruct.szString = string;
	re.DrawString( drawStruct );
	tabstops++;

	g_SingleScoreboard.nUpLastFrame = 1;
}

int CIN_Running();

void sb_DrawScoreboard()
{
	if( CIN_Running())
	{
		return;
	}

	sb_ClearScreen ();

    if( scoreboard_font == NULL )
	{
// SCG[1/5/00]: 		scoreboard_font = re.LoadFont( "scorefont" );
		scoreboard_font = re.LoadFont( "scr_cent" );
	}

	if( g_SingleScoreboard.bActive == true )
	{
		sb_DrawSingle();
	}

	if( g_MultiScoreboard.bActive == true )
	{
		sb_DrawMulti();
	}
}

///////////////////////////////////////////////////////////////////////////////
//	sb_ParseScorebardMessage
//
//
///////////////////////////////////////////////////////////////////////////////

void sb_ParseScoreboardMessageMulti()
{
	int	i;
	playerInfo_t	*score;

	score = &g_MultiScoreboard.scores[0];

	// read the details flag
	g_MultiScoreboard.details = MSG_ReadByte(&net_message) ? true : false;	
	int episode = MSG_ReadByte(&net_message) - 1;
	if ((episode < 0) || (episode > 3))
		episode = 0;
	g_MultiScoreboard.episode = episode;
	g_MultiScoreboard.teamplay = MSG_ReadByte(&net_message);

	for (i = 0; i < g_MultiScoreboard.total_scores; i++, score++)
	{
		score->client_index	= MSG_ReadShort (&net_message);
		score->frags		= MSG_ReadShort (&net_message);
		score->level		= MSG_ReadShort	(&net_message);
		score->team			= MSG_ReadShort (&net_message);
		score->ping			= MSG_ReadShort (&net_message);
		score->time_on		= MSG_ReadShort (&net_message);
		score->shots_fired	= MSG_ReadShort (&net_message);
		score->shots_hit	= MSG_ReadShort (&net_message);
		score->num_deaths	= MSG_ReadShort (&net_message);
	}

	if (g_MultiScoreboard.teamplay)
	{
		for (i = 0; i < MAX_TEAMS; i++)
		{
			g_MultiScoreboard.teamscores[i] = MSG_ReadShort (&net_message);
		}
	}
}

void sb_ParseScoreboardMessageCTF()
{
	int	i;
	playerInfo_t	*score;

	score = &g_MultiScoreboard.scores[0];
	memset(score,0,sizeof(playerInfo_t));

	g_MultiScoreboard.details = MSG_ReadByte(&net_message) ? true : false;
	for (i = 0; i < g_MultiScoreboard.total_scores; i++, score++)
	{
		score->client_index	= MSG_ReadShort (&net_message);
		score->team			= MSG_ReadShort (&net_message);
		score->frags		= MSG_ReadShort (&net_message);
		score->level		= MSG_ReadShort	(&net_message);
		score->ping			= MSG_ReadShort (&net_message);
		score->time_on		= MSG_ReadShort (&net_message);
		score->shots_fired	= MSG_ReadShort (&net_message);
		score->shots_hit	= MSG_ReadShort (&net_message);
		score->num_deaths	= MSG_ReadShort (&net_message);
	}

	int numscores = MSG_ReadShort (&net_message);
	CTFInfo_t *ctf_info = NULL;
	for (i = 0; i < numscores; i++)
	{
		ctf_info = &g_MultiScoreboard.ctf_info[i];
		ctf_info->num		= MSG_ReadShort (&net_message);
		ctf_info->captures	= MSG_ReadShort (&net_message);
		ctf_info->score		= MSG_ReadShort (&net_message);
	}
}

#define SCOREBOARD_SINGLE	0x0
#define SCOREBOARD_COOP		0x1
#define SCOREBOARD_CTF		0x2
#define SCOREBOARD_MULTI	0x4

void sb_ParseScoreboardMessage()
{
	char nType;

	nType = MSG_ReadByte (&net_message);

	g_SingleScoreboard.bActive = false;
	g_MultiScoreboard.bActive = false;

	switch( nType )
	{
	case SCOREBOARD_SINGLE:
		{
			g_SingleScoreboard.bActive = true;
			int episode = MSG_ReadByte(&net_message) - 1;
			if ((episode < 0) || (episode > 3))
				episode = 0;
			g_SingleScoreboard.episode = episode;
		}
		break;
	case SCOREBOARD_COOP:
		g_MultiScoreboard.ctf = false;
		g_MultiScoreboard.total_scores = MSG_ReadByte (&net_message);
		g_MultiScoreboard.bActive = true;
		sb_ParseScoreboardMessageMulti();
		break;
	case SCOREBOARD_CTF:
		g_MultiScoreboard.ctf = true;
		g_MultiScoreboard.total_scores = MSG_ReadByte (&net_message);
		g_MultiScoreboard.bActive = true;
		sb_ParseScoreboardMessageCTF();
		break;
	case SCOREBOARD_MULTI:
		g_MultiScoreboard.ctf = false;
		g_MultiScoreboard.total_scores = MSG_ReadByte (&net_message);
		g_MultiScoreboard.bActive = true;
		sb_ParseScoreboardMessageMulti();
		break;
	}
}
