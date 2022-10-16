// ----------------------------------------------------------------------------------------
//	This file compiles to an exe and is used to test the IONVideo.dll
// ----------------------------------------------------------------------------------------

#include <streams.h>
#include <windows.h>

#include <DVDEvCod.h>
#include "..\VidPlay.h"
#include "..\DDrawObj.h"
#include "VideoTest.h"

#define DEFINE_VIDEO_GLOBALS
#include "..\VideoPlayback.h"

HWND                g_hWnd ;
HINSTANCE           ghInstance ;
HANDLE hDLL = 0 ;



bool LoadVideoLib (const char* pszVideoDllFilename)
{
    hDLL = ::LoadLibrary (pszVideoDllFilename);

    if (0 == hDLL)
    {
        OutputDebugString ("ERROR: loading Video DLL\n") ;
        return (false);
    }

    Video_Init		   = (Video_Init_t) 	    GetProcAddress  (hDLL, "Video_Init");
    Video_Cleanup 	   = (Video_Cleanup_t)      GetProcAddress  (hDLL, "Video_Cleanup");
    Video_Play 		   = (Video_Play_t)  	    GetProcAddress  (hDLL, "Video_Play");

    if (Video_Init == 0 || Video_Cleanup == 0 || Video_Play == 0)
    {
    	OutputDebugString ("INVALID Video DLL!\n") ;
        return(false);
    }

	return(true);
}	


int APIENTRY WinMain (HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    MSG      msg ;

    ghInstance = hInstance ;

    if (!LoadVideoLib ("IONVideo.dll"))
    {
        MessageBox (0,
            TEXT ("Video DLL was not found or is Invalid!"),
            TEXT ("Error"), MB_OK | MB_ICONINFORMATION) ;
        return(0);
    }
    
    if (!Video_Init (800, 600, 16, 0))
    {
        DbgTerminate () ;
        return FALSE ;
    }

	bool bPlayedOk ;
    bPlayedOk = Video_Play (lpCmdLine, File) ;
    //bPlayedOk = Video_Play ("d:\\W97intr.mpg", File) ;
    //bPlayedOk = Video_Play ("d:\\dk.mpg", File) ;

    Video_Cleanup () ;

    if (!bPlayedOk) 
    {
    	MessageBox (NULL, "No file name specified or it wasn't found!", "Error", MB_OK | MB_ICONINFORMATION) ;
    }	

    return (msg.wParam) ;
}


