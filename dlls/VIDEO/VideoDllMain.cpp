// ----------------------------------------------------------------------------------------
// Bassed on the DDrawXcl.cpp: DDraw Exclusive Mode Video Playback test/sample app
//
// Note: As of 10/23/98 DirectShow is still pretty buggy and the code doesn't work
//		 properly on all machines.  MS hasn't released the NT 4.0 version of DirectShow
//		 so it doesn't work under NT either.
//
//		 Oh, I wasn't succesful in getting the Video to restart when alt-tabed away and,
//		 naturally, the sample programs all punt on this and just abort the playback.
//
//		 Also, error cleanup is handled very well.
//
//  This file is built as a DLL with three fns exported:
//		Video_Init ()
//		Video_Play () 
//	 	Video_Cleanup ()        
// ----------------------------------------------------------------------------------------

#include <streams.h>
#include <windows.h>
#include <stdio.h>
#include <DVDEvCod.h>
#include "VidPlay.h"
#include "DDrawObj.h"

#define TIMER_ID        1
#define TIMER_RATE      100

#define ION_DLLEXPORT  __declspec( dllexport)

#define SafeRelease(x) if (x) { (x)->Release(); x=0; }
#define SafeDelete(x) if (x) { delete (x); x=0; }


TCHAR               gszDirection[100] ;
HINSTANCE           ghInstance ;
HWND                g_hWnd ;
int				 	g_nWidth 	= 0 ;
int 				g_nHeight 	= 0 ;
int 				g_nBitsPerPixel = 0 ;
int 				g_nPausedRefCnt = 0 ;
bool 				g_bVideoModeSwitched = false ;
bool 				g_bDoModeSwitch = true ;
bool 				g_bPaused = false ;

CBaseVideoPlayer   *gpPlayer ;
CDDrawObject       *gpDDrawObj ;
UINT                guTimerID ;
VIDEO_TYPE          geVideoType ;
BOOL                gbAppActive ;

static BOOL          g_bCrippledOS = FALSE;
static OSVERSIONINFO g_osviVerInfo;


LRESULT CALLBACK MovieWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) ;
LRESULT OnPlaybackEvent(HWND hWnd, WPARAM wParam, LPARAM lParam) ;
void    OnEndOfPlayback(HWND hWnd) ;
void    SetVideoPosition(BOOL bSetBallPosition) ;
BOOL    createPlayer(VIDEO_TYPE eType) ;
bool 	video_SetVideoMode (int nWidth, int nHeight, int nBitsPerPixel) ;



BOOL IsVideoTypeKnown (void)
{
	return (Unspecified != geVideoType) ;
}


VIDEO_TYPE GetVideoType (void)
{
	return geVideoType ;
}



ION_DLLEXPORT bool Video_Init (int nWidth, int nHeight, int nBitsPerPixel, LPDIRECTDRAW pDDraw)
{
	CoInitialize (NULL) ;

	// Create a DDraw object and init it
	//
	gpDDrawObj = new CDDrawObject (g_hWnd) ;
	if (NULL == gpDDrawObj)
	{
		DbgTerminate () ;
		return false ;
	}

    // -- Regsister window's class...
    WNDCLASS classInfo ;
    if (!GetClassInfo (ghInstance, "VideoOutput", &classInfo))
    {
        WNDCLASS wc;
        wc.style 		= CS_NOCLOSE;
        wc.lpfnWndProc  = (WNDPROC)MovieWndProc ;
        wc.cbClsExtra 	= 0;
        wc.cbWndExtra	= 0;
        wc.hInstance 	= ghInstance ;
        wc.hIcon 		= 0;
        wc.hCursor 		= 0;
        wc.hbrBackground= 0;
        wc.lpszMenuName = 0;
        wc.lpszClassName= "VideoOutput";
        RegisterClass (&wc) ;
    }
      
    g_hWnd = CreateWindow ("VideoOutput", "Video Window", WS_POPUP|WS_VISIBLE,0,0,4,4, g_hWnd,0,ghInstance,0);
    if (!g_hWnd) 
        return (false);


	g_nWidth 		= nWidth ;
    g_nHeight	    = nHeight ;
    g_nBitsPerPixel = nBitsPerPixel ;

	if (pDDraw)
	{
    	OutputDebugString ("Using existing DDraw object.\n") ;
    	if (!gpDDrawObj->Initialize (pDDraw))
        	return (FALSE);
	}
	// -- Setup DDraw and change video resolution...
    //
	HRESULT hr = gpDDrawObj->StartExclusiveMode (g_hWnd, nWidth, nHeight, nBitsPerPixel) ;

  	if (FAILED (hr))
  	{
        MessageBox (g_hWnd, 
                TEXT ("Can't create a DirectDraw object."), 
                TEXT ("Error"), MB_OK | MB_ICONSTOP) ;
		return false ;
  	}

    g_bVideoModeSwitched = true ;

	return(true);
}


ION_DLLEXPORT bool Video_Cleanup ()
{
	if (gpDDrawObj && gpDDrawObj->IsInExclusiveMode ())
		gpDDrawObj->StopExclusiveMode (g_hWnd) ;

    SafeDelete (gpPlayer) ;
    SafeDelete (gpDDrawObj) ;

	if (g_hWnd)
	{
	    DestroyWindow (g_hWnd) ;
		g_hWnd = 0 ;
	}
    
    //UnregisterClass ("VideoOutput", ghInstance) ;
	CoUninitialize () ;
	return true;
}




// Video_Play (): Switches to fulscreen exclusive mode, sets up for selected media
// playback, gets the video size and positions video at the center, starts playing
// and sets a timer to tell MovieWndProc() every 1/10 second.
//
ION_DLLEXPORT bool Video_Play (const char* pszFilename, VIDEO_TYPE eType)
{
	HRESULT hr ;
	DbgLog ((LOG_TRACE, 5, TEXT ("Video_Play () entered"))) ;

	// -- Make sure file exists...
    FILE* hFile ;
    hFile = fopen (pszFilename, "r") ;
    if (!hFile)
    {
    	OutputDebugString ("ERROR: Video file not found!\n") ;
        return (false);
        
    }
    fclose (hFile) ;

	// -- Create the player if necessary...
	if (!createPlayer (eType)  || NULL == gpPlayer)
	{
		DbgLog ((LOG_ERROR, 0, TEXT ("ERROR: Couldn't create %s player"),
	 			DVD == eType ? "DVD" : "File")) ;
		return FALSE ;
	}

	gpPlayer->SetFileName (pszFilename) ;

	if (!IsVideoTypeKnown ())
	{
		MessageBox (g_hWnd, 
			TEXT ("No playback option (DVD/File) has been specified through the menu option yet.\nCan't run test."), 
			TEXT ("Sorry"), MB_OK | MB_ICONINFORMATION) ;
		return 0 ;
	}

	//
	// Make sure that DShow components are installed etc so that
	// the graph building can be init-ed
	//
	if (!gpPlayer->Initialize ()) 
	{
		MessageBox (g_hWnd, 
			TEXT ("DShow components couldn't be initialized.\n\nCan't run test.\nPlease check DxMedia installation"), 
			TEXT ("Sorry"), MB_OK | MB_ICONSTOP) ;
		return 0 ;
	}

	// Build video playback graph
	//
	hr = gpPlayer->BuildGraph (g_hWnd, gpDDrawObj->GetDDObject (), gpDDrawObj->GetDDPrimary ()) ;
	if (FAILED (hr))
	{
		gpDDrawObj->StopExclusiveMode (g_hWnd) ;  // get out of exclusive mode
		return false ;
	}

	// Specify the overlay callback interface for OverlayMixer to notify us
	//
	hr = gpPlayer->SetOverlayCallback (gpDDrawObj->GetCallbackInterface ()) ;
	ASSERT (SUCCEEDED (hr)) ;

	//
	// Pause the video playback graph to get it ready to play
	//
	hr = gpPlayer->Pause () ;
	if (FAILED (hr))
	{
		gpPlayer->ClearGraph () ;           // remove graph
		gpDDrawObj->StopExclusiveMode (g_hWnd) ;  // get out of exclusive mode
		return false ;
	}

	//
	// Get the color key info from the Player object and pass it to the DDraw object
	//
	DWORD   dwVideoColorKey ;
	gpPlayer->GetColorKey (&dwVideoColorKey) ;
	gpDDrawObj->SetColorKey (dwVideoColorKey) ;

	//
	// Get the video width and height, center it and pass the coordinates to 
	// the player and the DDraw object
	//
	SetVideoPosition (TRUE) ;

	//
	// Create the first screen on back buffer and then flip
	//
#ifndef NOFLIP
	gpDDrawObj->UpdateAndFlipSurfaces () ;
#endif // NOFLIP

	//
	// Play video now...
	//
	if (!gpPlayer->Play ()) 
	{
		gpPlayer->ClearGraph () ;           // remove graph
		gpDDrawObj->StopExclusiveMode (g_hWnd) ;  // get out of exclusive mode
		return false ;
	}

	//
	// Now set a timer based on which we'll update the buffers and flip
	//
	guTimerID = SetTimer (g_hWnd, TIMER_ID, TIMER_RATE, NULL) ;
	ASSERT (0 != guTimerID) ;

	// We are done with starting the playback.  MovieWndProc will stop the playback on
	// playback event messages or user hitting Esc key as well the timer based
	// actions will be taken in WM_TIMER handler there.

	//if (StartPlay (gg_hWnd))  // playback started successfully
	gbAppActive = TRUE ;   // if we are playing, we must be active


	OutputDebugString ("v-- Entering Video_Play's message loop...\n") ;

    // Main message loop:
    MSG      msg ;
    while (GetMessage (&msg, NULL, 0, 0))
    {
        TranslateMessage (&msg) ;
        DispatchMessage (&msg) ;
    }

	OutputDebugString ("^-- Leaving Video_Play's message loop...\n") ;

	return(true);
} // Video_Play



BOOL createPlayer (VIDEO_TYPE eType)
{
	if (geVideoType == eType)  // same type as before
	{
		if (gpPlayer)          // we have already have the player
			return TRUE     ;  // we'll use the same one; everything is OK
	}
	else                       // video type has changed
	{
		if (gpPlayer)          // we created a player before...
		{
			delete gpPlayer ;  // release it now
			gpPlayer = NULL ;
		}
	}

	// If we are here, we need to create a new player of the specified type
	if (DVD == eType)
		gpPlayer = new CDVDPlayer ;
	else if (File == eType)
		gpPlayer = new CFilePlayer ;
	else  // what then??
	{
		ASSERT (FALSE) ;
		return NULL ;
	}

	geVideoType = eType ;   // this our current video type

	return TRUE ;
}




// OnEndOfPlayback(): Releases everything on end of playback (but checks to
// avoid doing it too many times) as it "may be" called a little more than
// we would like to.
//
void OnEndOfPlayback (HWND hWndVideo)
{
	OutputDebugString ("OnEndOfPlayback() entered\n")  ;

	if (0 != guTimerID)  // if any timer is still set
	{
		BOOL bRes = KillTimer (hWndVideo, TIMER_ID) ;  // don't need that timer anymore
		ASSERT (bRes) ;
		guTimerID = 0 ;  // timer released
	}

	if (gpPlayer  &&  gpPlayer->IsGraphReady ())
	{
		OutputDebugString ("Turn off color keying before stopping the graph\n")  ;
		gpDDrawObj->SetOverlayState (FALSE) ;   // don't paint color key in video's position

		gpPlayer->Stop () ;
        
        if (gpDDrawObj)
        {
			gpDDrawObj->ClearSurfaces () ;
#ifndef NOFLIP
			gpDDrawObj->UpdateAndFlipSurfaces () ;  // flip the surface so that video doesn't show anymore
#endif // NOFLIP
        }
	
		// Remove the overlay callback interface for OverlayMixer
		HRESULT hr = gpPlayer->SetOverlayCallback (NULL) ;
		ASSERT (SUCCEEDED (hr)) ;

		gpPlayer->ClearGraph () ;
	}
    
//*temp	if (gpDDrawObj && gpDDrawObj->IsInExclusiveMode ())
//*temp		gpDDrawObj->StopExclusiveMode (hWndVideo) ;
    
    // -- Break us out of our loop in Video_Play ()...
    //
    if (hWndVideo)
		PostMessage (hWndVideo, WM_CLOSE, 0, 0) ;    

SafeDelete (gpPlayer) ;
}



ION_DLLEXPORT void video_Pause (bool bPause)
{
    if (bPause)
    {
		g_nPausedRefCnt++ ;
		if (gpPlayer)
			gpPlayer->Pause () ;
    }
    else
    {
		g_nPausedRefCnt-- ;
		if (gpPlayer)
	    	gpPlayer->Play () ;    
    }
}	


ION_DLLEXPORT void video_OnActivate (bool bActivating)
{
	if (bActivating)
	{
    	OutputDebugString ("video_OnActivate activating\n") ;
        video_Pause (false) ;
	}
    else
    {
    	OutputDebugString ("video_OnActivate DE-activating\n") ;
		gpDDrawObj->ClearSurfaces () ;
        video_Pause (true) ;
    }
}	


LRESULT CALLBACK MovieWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HDC          hDC ;
	static PAINTSTRUCT  ps ;

	switch (message)
	{
#if 1
	case WM_ACTIVATEAPP:
		gbAppActive = (BOOL) wParam ;
		DbgLog ((LOG_TRACE, 2, TEXT ("Got a WM_ACTIVATEAPP message with Active = %s"),
				gbAppActive ? "TRUE" : "FALSE")) ;

		// -- Can't restore video playback until MS fixes there shit, so for now, abort playback...
		if (!gbAppActive)
        	OnEndOfPlayback (hWnd) ;  // should stop playback now
       	//video_OnActivate (gbAppActive ? true : false) ;
		break;
#endif        

#if 0
	case WM_ACTIVATE:
        gbAppActive = (LOWORD(wParam) == WA_INACTIVE) ? FALSE : TRUE ;
		OutputDebugString ("WM_ACTIVATE: ") ;
        OutputDebugString (gbAppActive ? "true\n" : "false\n") ;
        
		if (!gbAppActive)
        	OnEndOfPlayback (hWnd) ;  // should stop playback now
       	//video_OnActivate (gbAppActive ? true : false) ;
        return (DefWindowProc (hWnd, message, wParam, lParam));
		break ;
#endif
	case WM_SETCURSOR:
    	SetCursor (NULL) ;
		break;
        
	case WM_TIMER:
		DbgLog ((LOG_TRACE, 4, TEXT ("Got a WM_TIMER message with ID = %ld (Active = %s)"), 
			wParam, gbAppActive ? "T" : "F")) ;
		if (TIMER_ID != wParam  || !gbAppActive)
	        // this is not the timer we have set  or the app isn't active anymore...
			// (...but we should have turned time off then!!)
			break ;     // don't do anything

		// We could do some status update here that could be used by the
		// UpdateAndFlipSurfaces() call below.
		//
#ifndef NOFLIP
		gpDDrawObj->UpdateAndFlipSurfaces () ;
#endif // NOFLIP
		break;

	case WM_CHAR:
		switch (wParam)
		{
			case ',':
			case '<':
				break ;

			case '>':
            case '.':
				break ;
		};
        break;

        
	case WM_PLAY_EVENT:
		DbgLog ((LOG_TRACE, 4, TEXT ("Got a WM_PLAY_EVENT message with wParam = %ld"), wParam)) ;
		if (1 == OnPlaybackEvent (hWnd, wParam, lParam))  // playback ended
			OnEndOfPlayback (hWnd) ;     // do the necessary things
		break ;

	case WM_SIZE_CHANGE:
		DbgLog ((LOG_TRACE, 4, TEXT ("Got a WM_SIZE_CHANGE message"))) ;
		SetVideoPosition (FALSE) ;
		break ;

	case WM_KEYUP:
		DbgLog ((LOG_TRACE, 4, TEXT ("Got a WM_KEYUP message with wParam = %ld"), wParam)) ;
		switch (wParam)
		{
			case VK_ESCAPE:
				OnEndOfPlayback (hWnd) ;
				break ;

			case VK_SPACE:
				if (gpPlayer)
				{
					g_bPaused = !g_bPaused ;
					video_Pause (g_bPaused) ;
				}
				break ;

			default:
				break ;
		} // switch
		break ;

	case WM_DESTROY:
		OnEndOfPlayback (hWnd) ;  // must stop playback before quitting
		PostQuitMessage (0);
		break;

	default:
		return (DefWindowProc (hWnd, message, wParam, lParam));
	}

	return 0 ;
}




// OnPlaybackEvent(): Handles playback events, specially completed/stopped-on-error.
//
// Returns 0 if playback is continuing.
// Returns 1 if playback is over for any reason.
//
LRESULT OnPlaybackEvent (HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	DbgLog ((LOG_TRACE, 5,
			TEXT ("App's OnPlaybackEvent(0x%lx, 0x%lx) entered"),
			wParam, lParam)) ;

	IMediaEventEx  *pME = (IMediaEventEx *) lParam ;
    
	if (NULL == pME  || Playing != gpPlayer->GetState ())
	{
		DbgLog ((LOG_TRACE, 1, TEXT ("Either pME = NULL or not playing anymore. Skip everything."))) ;
		return 0 ;  // or 1 ??
	}

	LONG     lEvent ;
	LONG     lParam1, lParam2 ;

	//  Because the message mode for IMediaEvent may not be set before
	//  we get the first event it's important to read all the events
	//  pending when we get a window message to say there are events pending.
	//  GetEvent() returns E_ABORT when no more event is left.
	//
	while (SUCCEEDED (pME->GetEvent (&lEvent, &lParam1, &lParam2, 0))) // no wait
	{
		switch (lEvent)
		{
			//
			// First the DVD related events
			//
			case EC_DVD_STILL_ON:
				DbgLog ((LOG_TRACE, 5, TEXT ("Playback Event: EC_DVD_STILL_ON"))) ;
				break ;

			case EC_DVD_STILL_OFF:
				DbgLog ((LOG_TRACE, 5, TEXT ("Playback Event: EC_DVD_STILL_OFF"))) ;
				break ;

			case EC_DVD_DOMAIN_CHANGE:
				DbgLog ((LOG_TRACE, 5, TEXT ("Playback Event: EC_DVD_DOMAIN_CHANGE, %ld"), lParam1)) ;
				switch (lParam1)
				{
				case DVD_DOMAIN_FirstPlay:  // = 1
				case DVD_DOMAIN_Stop:       // = 5
					break ;

				case DVD_DOMAIN_VideoManagerMenu:  // = 2
				case DVD_DOMAIN_VideoTitleSetMenu: // = 3
					// Inform the app to update the menu option to show "Resume" now
					break ;

				case DVD_DOMAIN_Title:      // = 4
					// Inform the app to update the menu option to show "Menu" again
					break ;

				default: // hmmmm...
					break ;
				}
				break ;

			case EC_DVD_BUTTON_CHANGE:
				DbgLog ((LOG_TRACE, 5, TEXT ("DVD Event: Button Changed to %d out of %d"),
					lParam2, lParam1));
				break;

			case EC_DVD_TITLE_CHANGE:
				DbgLog ((LOG_TRACE, 5, TEXT ("Playback Event: EC_DVD_TITLE_CHANGE"))) ;
				break ;

			case EC_DVD_CHAPTER_START:
				DbgLog ((LOG_TRACE, 5, TEXT ("Playback Event: EC_DVD_CHAPTER_START"))) ;
				break ;

			case EC_DVD_CURRENT_TIME:
				DbgLog ((LOG_TRACE, 5, TEXT ("Playback Event: EC_DVD_CURRENT_TIME"))) ;
				break ;


			// Then the general DirectShow related events
			//
			case EC_COMPLETE:
				DbgLog ((LOG_TRACE, 5, TEXT ("Playback Event: Playback complete"))) ;
				//MessageBeep (MB_OK) ; // just to inform that the playback is over

				// Remember to free the event params
				pME->FreeEventParams (lEvent, lParam1, lParam2) ;

				// We don't do the release part here. That will be done in MovieWndProc()
				// after return from this function.
				return 1 ;  // playback over

			case EC_USERABORT:
			case EC_ERRORABORT:
				DbgLog ((LOG_TRACE, 5, TEXT ("Playback Event: 0x%lx"), lEvent)) ;
				MessageBeep (MB_ICONEXCLAMATION) ; // to inform that the playback has errored out

				// Remember to free the event params
				pME->FreeEventParams (lEvent, lParam1, lParam2) ;

				// We don't do the release part here. That will be done in MovieWndProc()
				// after return from this function.
				return 1 ;  // playback over

			default:
				DbgLog ((LOG_TRACE, 5, TEXT ("Ignored unknown playback event: 0x%lx"), lEvent)) ;
				break ;
		}

		// Remember to free the event params
		//
		DbgLog ((LOG_TRACE, 5, TEXT ("Going to call FreeEventParams()"))) ;
		pME->FreeEventParams (lEvent, lParam1, lParam2) ;
		DbgLog ((LOG_TRACE, 5, TEXT ("FreeEventParams() done"))) ;

	}  // end of while (GetEvent()) loop

	return 0 ;
}


typedef enum _AM_TRANSFORM
{
	AM_SHRINK,
	AM_STRETCH
} AM_TRANSFORM ;

void TransformRect (RECT *prRect, double dPictAspectRatio, AM_TRANSFORM transform)
{
	double dWidth, dHeight, dNewWidth, dNewHeight ;
	double dResolutionRatio = 0.0, dTransformRatio = 0.0 ;

	ASSERT (transform == AM_SHRINK || transform == AM_STRETCH) ;

	dNewWidth = dWidth = prRect->right - prRect->left ;
	dNewHeight = dHeight = prRect->bottom - prRect->top ;

	dResolutionRatio = dWidth / dHeight ;
	dTransformRatio = dPictAspectRatio / dResolutionRatio ;

	// shrinks one dimension to maintain the coorect aspect ratio
	if (transform == AM_SHRINK)
	{
		if (dTransformRatio > 1.0)
		{
			dNewHeight = dNewHeight / dTransformRatio ;
		}
		else if (dTransformRatio < 1.0)
		{
			dNewWidth = dNewWidth * dTransformRatio ;
		}
	}
	// stretches one dimension to maintain the coorect aspect ratio
	else if (transform == AM_STRETCH)
	{
		if (dTransformRatio > 1.0)
		{
			dNewWidth = dNewWidth * dTransformRatio ;
		}
		else if (dTransformRatio < 1.0)
		{
			dNewHeight = dNewHeight / dTransformRatio ;
		}
	}

	if (transform == AM_SHRINK)
	{
		ASSERT (dNewHeight <= dHeight) ;
		ASSERT (dNewWidth <= dWidth) ;
	}
	else
	{
		ASSERT (dNewHeight >= dHeight) ;
		ASSERT (dNewWidth >= dWidth) ;
	}

	// cut or add equal portions to the changed dimension

	prRect->left += (LONG)(dWidth - dNewWidth)/2 ;
	prRect->right = prRect->left + (LONG)dNewWidth ;

		prRect->top += (LONG)(dHeight - dNewHeight)/2 ;
	prRect->bottom = prRect->top + (LONG)dNewHeight ;
}


//
// SetVideoPosition(): Gets the original video size and positions it at the center.
// 
void SetVideoPosition (BOOL bSetBallPosition)
{
	DbgLog ((LOG_TRACE, 5, TEXT ("App's SetVideoPosition() entered"))) ;

	DWORD  dwVideoWidth, dwVideoHeight ;
	DWORD  dwARX, dwARY ;

	gpPlayer->GetNativeVideoData (&dwVideoWidth, &dwVideoHeight, &dwARX, &dwARY) ;
	DbgLog ((LOG_TRACE, 5, TEXT ("Native video size: %lu x %lu, Aspect Ratio: %lu x %lu"), 
			dwVideoWidth, dwVideoHeight, dwARX, dwARY)) ;

	// Update output size to make it aspect ratio corrected
	RECT  rectCorrected ;
	SetRect (&rectCorrected, 0, 0, dwVideoWidth, dwVideoHeight) ;
	TransformRect (&rectCorrected, (double)dwARX / (double)dwARY, AM_STRETCH) ;
	DbgLog ((LOG_TRACE, 5, TEXT ("Updated video size: %ld x %ld"), 
			RECTWIDTH (rectCorrected), RECTHEIGHT (rectCorrected))) ;

	RECT   ScrnRect ;
	gpDDrawObj->GetScreenRect (&ScrnRect) ;
	DbgLog ((LOG_TRACE, 5, TEXT ("Screen size is %ld x %ld"), 
			RECTWIDTH (ScrnRect), RECTHEIGHT (ScrnRect))) ;

	DWORD  dwVideoTop ;
	DWORD  dwVideoLeft ;
	if (RECTWIDTH (rectCorrected)  <= RECTWIDTH (ScrnRect)  &&  // video width less than screen
		RECTHEIGHT (rectCorrected) <= RECTHEIGHT (ScrnRect))    // video height less than screen
	{
		dwVideoLeft = (RECTWIDTH (ScrnRect)  - RECTWIDTH (rectCorrected)) / 2 ;
		dwVideoTop  = (RECTHEIGHT (ScrnRect) - RECTHEIGHT (rectCorrected)) / 2 ;
	}
	else                            // video width more than screen
	{
		rectCorrected = ScrnRect ;
		TransformRect (&rectCorrected, (double)dwARX / (double)dwARY, AM_SHRINK) ;
		dwVideoLeft = rectCorrected.left ;
		dwVideoTop  = rectCorrected.top ;
	}
	gpDDrawObj->SetVideoPosition (dwVideoLeft, dwVideoTop, 
					 			  RECTWIDTH (rectCorrected), RECTHEIGHT (rectCorrected)) ;

	gpPlayer->SetVideoPosition (dwVideoLeft, dwVideoTop, 
								RECTWIDTH (rectCorrected), RECTHEIGHT (rectCorrected)) ;
	DbgLog ((LOG_TRACE, 5, TEXT ("Video is %ld x %ld at (%ld x %ld)"), 
			RECTWIDTH (rectCorrected), RECTHEIGHT (rectCorrected), dwVideoLeft, dwVideoTop)) ;

	char szBuff [256] ;
    wsprintf (szBuff, "Video is %ld x %ld at (%ld x %ld)\n",
			RECTWIDTH (rectCorrected), RECTHEIGHT (rectCorrected), dwVideoLeft, dwVideoTop) ;
    OutputDebugString (szBuff) ;        
}




extern "C" int APIENTRY DllMain (HINSTANCE hInstance, DWORD  dwReason, LPVOID  lpReserved)
{
    BOOL bRC = TRUE;

    if (DLL_PROCESS_ATTACH == dwReason)
    {
        ghInstance = hInstance;

        g_osviVerInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        GetVersionEx(&g_osviVerInfo);

        if (VER_PLATFORM_WIN32_WINDOWS == g_osviVerInfo.dwPlatformId)
            g_bCrippledOS = TRUE;
    }

    return (TRUE);
}




