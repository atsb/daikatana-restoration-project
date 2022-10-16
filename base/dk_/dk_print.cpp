#include	"client.h"
#include	"daikatana.h"

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define MAX_DKCON  40

///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
	int  in_use;
	int  x, y;
	char str[100];
} dkcon_t;

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

static	dkcon_t	dkcon	[MAX_DKCON];
static	dkcon_t	frameprint_buffer	[MAX_DKCON];

static	int		dk_spinCount = 0;
static	char	*dk_spin = {"|/-\\|/-\\"};

///////////////////////////////////////////////////////////////////////////////
//	dk_print_test
///////////////////////////////////////////////////////////////////////////////

void dk_print_test (void)
{
	dk_printxy (5, 5, "This a test.");
}

///////////////////////////////////////////////////////////////////////////////
//	dk_init_print
///////////////////////////////////////////////////////////////////////////////

void dk_print_init (void)
{
	memset (dkcon, 0x00, sizeof(dkcon));
	memset (frameprint_buffer, 0x00, sizeof(frameprint_buffer));
}

///////////////////////////////////////////////////////////////////////////////
//	dk_printxy
///////////////////////////////////////////////////////////////////////////////

void dk_printxy (int x, int y, char *fmt, ...)
{
	va_list		argptr;
	int i;

	// if developer flag is not set then don't bother
//	if (!developer->value) return;

	// see if spot is already in use
	for (i = 0; i < MAX_DKCON; i++)
	{
		if ((dkcon [i].in_use) && (x == dkcon [i].x) && (y == dkcon [i].y))
		{
			break;
		}
	}

	if (i >= MAX_DKCON)
	{
		// find one not in use
		for (i = 0; i < MAX_DKCON; i++)
		{
			if (!dkcon [i].in_use) break;
		}
	}

	if (i >= MAX_DKCON)
	{
		// opps, no more spots left
		return;
	}

    dkcon [i].in_use = 1;
	dkcon [i].x = x;
	dkcon [i].y = y;

	va_start (argptr,fmt);
	vsprintf (dkcon[i].str, fmt, argptr);
	va_end (argptr);
}

///////////////////////////////////////////////////////////////////////////////
//	dk_print_clear
///////////////////////////////////////////////////////////////////////////////

void dk_print_clear (void)
{
	int i;

	for (i = 0; i < MAX_DKCON; i++)
		dkcon [i].in_use = 0;
}

///////////////////////////////////////////////////////////////////////////////
//	dk_printframe_xy
//
//	same as dk_printxy, except it prints from a buffer that is cleared each frame
///////////////////////////////////////////////////////////////////////////////

void dk_printframe_xy (int x, int y, char *fmt, ...)
{
	va_list		argptr;
	int i;

	// see if spot is already in use
	for (i = 0; i < MAX_DKCON; i++)
	{
		if ((frameprint_buffer [i].in_use) && (x == frameprint_buffer [i].x) && (y == frameprint_buffer [i].y))
		{
			break;
		}
	}

	if (i >= MAX_DKCON)
	{
		// find one not in use
		for (i = 0; i < MAX_DKCON; i++)
		{
			if (!frameprint_buffer [i].in_use) break;
		}
	}

	if (i >= MAX_DKCON)
	{
		// opps, no more spots left
		return;
	}

    frameprint_buffer [i].in_use = 1;
	frameprint_buffer [i].x = x;
	frameprint_buffer [i].y = y;

	va_start (argptr,fmt);
	vsprintf (frameprint_buffer[i].str, fmt, argptr);
	va_end (argptr);
}

///////////////////////////////////////////////////////////////////////////////
//	dk_printframe_clear
///////////////////////////////////////////////////////////////////////////////

void dk_printframe_clear (void)
{
	int i;

//	if (!developer->value) return;

	for (i = 0; i < MAX_DKCON; i++)
	{
		frameprint_buffer [i].in_use = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	dk_print_spinner
///////////////////////////////////////////////////////////////////////////////

void	dk_print_spinner (void)
{
	char	blah [2];
	dk_spinCount++;
	if (dk_spinCount > 7)
		dk_spinCount = 1;

// SCG[1/16/00]: 	sprintf (blah, "%c", dk_spin [dk_spinCount]);
	Com_sprintf (blah,sizeof(blah), "%c", dk_spin [dk_spinCount]);

	dk_printxy (0, 0, blah);
}


///////////////////////////////////////////////////////////////////////////////
//	dk_print_draw
///////////////////////////////////////////////////////////////////////////////

void dk_print_draw (void)
{
///////	int i, j, x;

	DRAWSTRUCT	drawStruct;

	for (int i = 0; i < MAX_DKCON; i++)
	{
		if (dkcon [i].in_use)
		{
			if (dkcon [i].y >= scr_con_current)
			{
				// SCG[3/29/99]: Consolidation change
//				for (j = 0, x = dkcon [i].x; j < strlen (dkcon [i].str); j++, x += 8)
//					re.DrawChar (x, dkcon [i].y, dkcon [i].str [j]);
					drawStruct.nFlags = DSFLAG_BLEND;
					drawStruct.szString = dkcon[i].str;
					drawStruct.nXPos = dkcon[i].x;
					drawStruct.nYPos = dkcon[i].y;
					drawStruct.pImage = NULL;
					re.DrawString( drawStruct );
			}
		}

		//	second, per-frame buffer
		if (frameprint_buffer [i].in_use)
		{
			if (frameprint_buffer [i].y >= scr_con_current)
			{
				// SCG[3/29/99]: Consolidation change
//				for (j = 0, x = frameprint_buffer [i].x; j < strlen (frameprint_buffer [i].str); j++, x += 8)
//					re.DrawChar (x, frameprint_buffer [i].y, frameprint_buffer [i].str [j]);
					drawStruct.nFlags = DSFLAG_BLEND;
					drawStruct.szString = frameprint_buffer[i].str;
					drawStruct.nXPos = frameprint_buffer[i].x;
					drawStruct.nYPos = frameprint_buffer[i].y;
					drawStruct.pImage = NULL;
					re.DrawString( drawStruct );
			}
		}
	}

	dk_printframe_clear ();
}