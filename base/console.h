
//
// console
//

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

// 19990311 - JAR: changed to support variable message display
#define	NUM_CON_TIMES 6

#define	CON_TEXTSIZE	32768

#define	CON_QUICK	0
#define	CON_NORMAL	1
#define	CON_LOADING	2		//	console is full screen, raises when ca_active



extern cvar_t      *console_screen_idx;


typedef struct
{
	qboolean	initialized;

	char	text[CON_TEXTSIZE];
	int		current;		// line where next message will be printed
	int		x;				// offset in current line for next print
	int		display;		// bottom of console displays this line

	int		ormask;			// high bit mask for colored characters

	int 	linewidth;		// characters across screen
	int		totallines;		// total lines in console scrollback

	float	cursorspeed;

	int		vislines;

	float	times[NUM_CON_TIMES];	// cls.realtime time the line was generated
								// for transparent notify lines

	//	Nelno:	quick or normal console mode
	int		mode;		
} console_t;

extern	console_t	con;

void Con_DrawCharacter (int cx, int line, int num);

void Con_CheckResize (void);
void Con_Init (void);
void Con_DrawConsole (float frac);
void Con_Print (char *txt);
void Con_CenteredPrint (char *text);
void Con_Clear_f (void);
void Con_DrawNotify (void);
void Con_ClearNotify (void);
void Con_ToggleConsole_f (void);
void Con_ToggleQuickConsole_f (void);
void Con_ConsoleOff (void);
