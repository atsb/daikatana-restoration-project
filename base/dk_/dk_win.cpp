#include	<windows.h>
#include	<stdio.h>
#include	"daikatana.h"

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

char	dk_userName	[DK_MAX_USERNAME];
int		dk_Initializing = 0;

static	double	dk_LastFrameRate;
static	double	dk_TotalFrames;
static	double	dk_TotalFrameTime;
static	double	dk_AvgFrameRate;

static	double	dk_AvgFrameRate50;	//	average of last 50 frames
static	int		dk_TotalFrames50;
static	double	dk_TotalFrameTime50;

static	double	dk_LowRate;
static	double	dk_HighRate;

static	double	dk_Frequency;

static	unsigned int	dk_FrameTime;
static	unsigned int	dk_StartTime;
static	unsigned int	dk_StopTime;

static	int				dk_UseQuery;
static	LARGE_INTEGER	dk_QueryStart;
static	LARGE_INTEGER	dk_QueryStop;
static	LARGE_INTEGER	dk_QueryFreq;

///////////////////////////////////////////////////////////////////////////////
//	functions
///////////////////////////////////////////////////////////////////////////////

void	dk_GetUserName (void)
{
	unsigned	int	size = DK_MAX_USERNAME;
	if (!GetUserName (dk_userName, &size))
	{
		sprintf (dk_userName, "config");
	}
}

///////////////////////////////////////////////////////////////////////////////
//	dk_StartTime
//
///////////////////////////////////////////////////////////////////////////////

void	dk_StartTimer (void)
{
	if (dk_UseQuery)
		QueryPerformanceCounter (&dk_QueryStart);
	else
		dk_StartTime = timeGetTime ();
}

///////////////////////////////////////////////////////////////////////////////
//	dk_InitRates
//
///////////////////////////////////////////////////////////////////////////////

void	dk_InitRates (int maxx, int y)
{
	int				x = maxx - (8 * 18);

	QueryPerformanceFrequency (&dk_QueryFreq);

	if (dk_QueryFreq.LowPart != 0)
	{
		dk_UseQuery = TRUE;
		dk_Frequency = (double) dk_QueryFreq.LowPart;
	}
	else
	{
		dk_Frequency = 1000.0;
		dk_UseQuery = FALSE;
	}

	dk_LastFrameRate = 0;
	dk_TotalFrameTime = 0;
	dk_TotalFrames = 0;
	dk_AvgFrameRate = 0;
	dk_FrameTime = 0;

	dk_AvgFrameRate50 = 0;
	dk_TotalFrameTime50 = 0;
	dk_TotalFrames50 = 0;

	dk_LowRate = 100000;
	dk_HighRate = 0;

	dk_StartTimer ();

	dk_printxy (x, y, "");
	dk_printxy (x, y + 8, "");
	dk_printxy (x, y + 16, "");
	dk_printxy (x, y + 24, "");
	dk_printxy (x, y + 32, "");
}

///////////////////////////////////////////////////////////////////////////////
//	dk_StopTime
//
///////////////////////////////////////////////////////////////////////////////

void	dk_StopTimer (void)
{
	unsigned int	test_time;

	if (dk_UseQuery)
	{
		QueryPerformanceCounter (&dk_QueryStop);

		test_time = dk_QueryStop.LowPart - dk_QueryStart.LowPart;

		if (test_time > 100)
			dk_FrameTime = test_time;
	}
	else
	{
		dk_StopTime = timeGetTime ();

		dk_FrameTime = dk_StopTime - dk_StartTime;
	}
	
	dk_TotalFrameTime += (double) dk_FrameTime;
	dk_TotalFrames += 1;

	dk_TotalFrameTime50 += (double) dk_FrameTime;
	dk_TotalFrames50 += 1;
}

///////////////////////////////////////////////////////////////////////////////
//	dk_ShowRates
//
//	FIXME:	don't hardcode x value for printxy
///////////////////////////////////////////////////////////////////////////////

void	dk_ShowRates (int maxx, int y)
{
	int		x = maxx - (8 * 18);

	if (dk_FrameTime != 0)
		dk_LastFrameRate = 1.0 / ((double) dk_FrameTime / dk_Frequency);
	else
		dk_LastFrameRate = 0;

	dk_AvgFrameRate = 1.0 / ((dk_TotalFrameTime / dk_TotalFrames) / dk_Frequency);

	dk_printxy (x, y, "Last frame: %.2f\n", dk_LastFrameRate);
	dk_printxy (x, y + 8, "Average   : %.2f\n", dk_AvgFrameRate);

	if (dk_TotalFrames50 == 50)
	{
		dk_AvgFrameRate50 = 1.0 / ((dk_TotalFrameTime50 / dk_TotalFrames50) / dk_Frequency);
		dk_printxy (x, y + 16, "Average50 : %.2f\n", dk_AvgFrameRate);

		dk_TotalFrameTime50 = 0;
		dk_TotalFrames50 = 0;
	}

	if (dk_LastFrameRate < dk_LowRate)
	{
		dk_LowRate = dk_LastFrameRate;
		dk_printxy (x, y + 24, "LowRate   : %.2f\n", dk_LowRate);
	}

	if (dk_LastFrameRate > dk_HighRate)
	{
		dk_HighRate = dk_LastFrameRate;
		dk_printxy (x, y + 32, "HighRate  : %.2f\n", dk_HighRate);
	}

//	dk_printxy (x, y + 24, "FrameTime : %i\n", dk_FrameTime);
//	dk_printxy (x, y + 32, "Start     : %u\n", dk_QueryStart.LowPart);
//	dk_printxy (x, y + 40, "Stop      : %u\n", dk_QueryStop.LowPart);
}
