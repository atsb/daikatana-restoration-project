// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================

//#include <windows.h>
#include "dk_system.h"
#if _MSC_VER
#include <crtdbg.h>
#endif
#include <stdio.h>
#include "SWMRG.H"		      // The header file
#include "dk_shared.h"

/* ***************************** define types ****************************** */

typedef struct _logFile
{
	int nLogType;
	char *szFileName;
} LOGFILE;

/* ***************************** Local Variables *************************** */

static int	bDebugWindowReady	= FALSE;
static int	bFileLogsReady		= FALSE;

static SWMRG	g_SWMRG;
static HANDLE	SharedFile	= NULL;
static LPVOID	SharedMem	= NULL;
static LPSTR	String		= NULL;

static char	logPath[256];
static FILE	*aLogFiles[LOGTYPE_MAX];

/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */

cvar_t *g_cvarLogOutput = NULL;

/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */


LOGFILE logFileDefines[] = 
{
	{ LOGTYPE_GRAPHICS, "log_graphics.log" },
	{ LOGTYPE_AI,		"log_ai.log" },
	{ LOGTYPE_WEAPONS,	"log_weapons.log" },
	{ LOGTYPE_AUDIO,	"log_audio.log" },
	{ LOGTYPE_PHYSICS,	"log_physics.log" },
	{ LOGTYPE_SCRIPT,	"log_script.log" },
	{ LOGTYPE_NETWORK,	"log_network.log" },
	{ LOGTYPE_USERIO,	"log_userio.log" },
	{ LOGTYPE_MEMORY,	"log_memory.log" },
    { LOGTYPE_MISC,     "log_misc.log" }
};


//*****************************************************************************
//							LOCAL FUNCTIONS
//*****************************************************************************

// ----------------------------------------------------------------------------
//
// Name:		DKLOG_FileInitialize
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void DKLOG_FileInitialize( char *logFilePath )
{
	if ( bFileLogsReady == TRUE )
	{
		return;
	}

	if ( strlen(logFilePath) > 0 )
	{
		int bError = FALSE;
		// open files for logging to files
		char logFileName[256];
		for ( int i = 0; i < LOGTYPE_MAX; i++ )
		{
			strcpy( logFileName, logFilePath );
			strcat( logFileName, "\\" );
			strcat( logFileName, logFileDefines[i].szFileName );
			aLogFiles[i] = fopen( logFileName, "a" );
			if ( aLogFiles[i] == NULL )
			{
				bError = TRUE;
				break;
			}
		}

		if ( bError == TRUE )
		{
			for ( int j = 0; j < i; j++ )
			{
				if ( aLogFiles[j] )
				{
					fclose( aLogFiles[j] );
					aLogFiles[j] = NULL;
				}
			}
			return;
		}

		bFileLogsReady = TRUE;
	}
	else
	{
		for ( int i = 0; i < LOGTYPE_MAX; i++ )
		{
			aLogFiles[i] = NULL;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		DKLOG_FileClose
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void DKLOG_FileClose()
{
	for ( int i = 0; i < LOGTYPE_MAX; i++ )
	{
		if ( aLogFiles[i] )
		{
			fclose( aLogFiles[i] );
			aLogFiles[i] = NULL;
		}
	}
	bFileLogsReady = FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		DKLOG_DebugWindowInitialize
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static int DKLOG_DebugWindowInitialize()
{
	if ( bDebugWindowReady == TRUE )
	{
		return TRUE;
	}

	// initialize file mapping for output to debug window
	SharedFile = OpenFileMapping( FILE_MAP_WRITE, TRUE, "DKDEBUG_BUFFER" );
    if (!SharedFile) 
	{
		return FALSE;
    }

    SharedMem = MapViewOfFile( SharedFile, FILE_MAP_WRITE, 0, 0, 512 );
    if (!SharedMem) 
	{
		return FALSE;
    }

	String = (LPSTR)SharedMem;

	// Initialize the single-writer/multiple-reader 
	// guard synchronization object. This must be done 
	// before any thread attempts to use it.
	SWMRGInitialize(&g_SWMRG, "DKDEBUG", 1);

	bDebugWindowReady = TRUE;

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		DKLOG_DebugWindowClose
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void DKLOG_DebugWindowClose()
{
	bDebugWindowReady = FALSE;
	SWMRGDelete(&g_SWMRG);
}

// ----------------------------------------------------------------------------
//
// Name:		DKLOG_WriteToDebugWindow
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void DKLOG_WriteToDebugWindow( char *newString )
{
	_ASSERTE( bDebugWindowReady == TRUE );
	_ASSERTE( SharedMem );

	// Wait until safe to write: no writers and no readers.
	if ( SWMRGWaitToWrite(&g_SWMRG, 1) == TRUE )
	{
		int nStrLen = strlen(&newString[5]) + 5;
		memcpy( String, newString, nStrLen );
		String[nStrLen] = '\0';

		// Inform the other writers/readers that we are done.
		SWMRGDoneWriting(&g_SWMRG);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		DKLOG_WriteToFile
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void DKLOG_WriteToFile( int nLogType, char *msg )
{
	_ASSERTE( nLogType >= LOGTYPE_GRAPHICS && nLogType < LOGTYPE_MAX );

	if ( aLogFiles[nLogType] )
	{
		int nStrLen = strlen(msg);
		fwrite( msg, nStrLen, 1, aLogFiles[nLogType] );

		if ( !(msg[nStrLen - 1] == '\n') )
		{
			fprintf( aLogFiles[nLogType], "\n" );
		}
	}
}

//*****************************************************************************
//							GLOBAL FUNCTIONS
//*****************************************************************************
#ifdef _DEBUG
// ----------------------------------------------------------------------------
//
// Name:		DKLOG_Initialize
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int DKLOG_Initialize( char *logFilePath )
{
	DKLOG_FileInitialize( logFilePath );
	return DKLOG_DebugWindowInitialize();
}

// ----------------------------------------------------------------------------
//
// Name:		DKLOG_Close
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void DKLOG_Close()
{
	DKLOG_FileClose();
	DKLOG_DebugWindowClose();
}

// ----------------------------------------------------------------------------
//
// Name:		DKLOG_Write
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void DKLOG_Write( int nLogType, float fCurrentTime, char *fmt, ... )
{
	if ( !g_cvarLogOutput )
	{
		return;
	}

	char line[256] = { "\0" };
	va_list	argptr;

	int nLogOutput = g_cvarLogOutput->value;

	// set first byte to the log type if sending to debug window or TCP/IP
	if ( bDebugWindowReady && (nLogOutput & LOG_TO_DBWIN) )
	{
		va_start(argptr, fmt);
		vsprintf((char *)&line[5], fmt, argptr);
		va_end(argptr);

		line[0] = (unsigned char)nLogType;
		memcpy( &line[1], &fCurrentTime, sizeof(fCurrentTime) );

		DKLOG_WriteToDebugWindow( line );
	}

	if ( bFileLogsReady && (nLogOutput & LOG_TO_FILE) )
	{
		// pack log type and time at the beginning of data
		// 1.21 dsn  had to reverse the order of concating b/c arguments getting lost
		
        char temp_fmt[256];

        va_start(argptr, fmt);
		vsprintf(temp_fmt, fmt, argptr);
		va_end(argptr);
        
		sprintf( line, "%4.1f\t%s", fCurrentTime, temp_fmt );
		
		DKLOG_WriteToFile( nLogType, line );
	}
	
	if ( bDebugWindowReady && (nLogOutput & LOG_TO_NETWORK) )
	{
		va_start(argptr, fmt);
		vsprintf((char *)&line[5], fmt, argptr);
		va_end(argptr);

		line[0] = (unsigned char)nLogType;
		memcpy( &line[1], &fCurrentTime, sizeof(fCurrentTime) );

		int nStrLen = (strlen((char *)&line[5]) + 5);
		UDP_Send( nStrLen, (unsigned char *)line);
	}
}

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

#endif // _DEBUG