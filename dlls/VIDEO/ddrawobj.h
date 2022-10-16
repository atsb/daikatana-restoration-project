/**************************************************************************
*
*  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
*  PURPOSE.
*
*  Copyright (c) 1993 - 1998  Microsoft Corporation.  All Rights Reserved.
*
**************************************************************************/

// DDrawObj.h: DDraw Object class header file

// #define NOFLIP  1  /* for debugging */


#ifdef SHOW_BALL
#define BALL_RADIUS   40
#define BALL_STEP      4
#endif
//
// Some macros
//
#define RECTWIDTH(rect)   ((rect).right  - (rect).left)
#define RECTHEIGHT(rect)  ((rect).bottom - (rect).top)

// forward declaration
class COverlayCallback ;

//
// DDraw object class to paint color key, flip etc etc.
//
class CDDrawObject 
{
  public:   // public methods for Windows structure to call

    CDDrawObject (HWND hWndApp) ;
   ~CDDrawObject (void) ;

	BOOL 	Initialize 			  (LPDIRECTDRAW pDDraw=0) ;
    HRESULT StartExclusiveMode    (HWND hWndApp, int nWidth, int nHeight, int nBitDepth) ;
    HRESULT StopExclusiveMode     (HWND hWndApp) ;
    HRESULT UpdateAndFlipSurfaces (void) ;
    void    SetVideoPosition 	  (DWORD dwVideoLeft, DWORD dwVideoTop, 
            					   DWORD dwVideoWidth, DWORD dwVideoHeight) ;

    inline  void    SetColorKey(DWORD dwColorKey)   { m_dwVideoKeyColor = dwColorKey ; } ;
    inline  void    GetScreenRect(RECT *pRect)      { *pRect = m_RectScrn ; } ;
    inline  BOOL    IsInExclusiveMode(void)         { return m_bInExclMode ; } ;
    inline  LPDIRECTDRAW         GetDDObject(void)  { return m_pDDObject ; } ;
    inline  LPDIRECTDRAWSURFACE  GetDDPrimary(void) { return m_pPrimary ; } ;
    inline  void    SetOverlayState(BOOL bState)    { m_bOverlayVisible = bState ; } ;
    inline  IDDrawExclModeVideoCallback * GetCallbackInterface(void) { return m_pOverlayCallback ; } ;

	void    ClearSurfaces () ;
        
  private:  // private helper methods for the class' own use
	HRESULT clearSurface 			   (IDirectDrawSurface *pDDSurface, RECT* pRect=0, DWORD dwFillColor=RGB (0, 0, 0)) ;
	HRESULT fillSurface 			   (IDirectDrawSurface *pDDSurface) ;
	void    drawOnSurface 			   (LPDIRECTDRAWSURFACE pSurface) ;
	HRESULT convertColorRefToPhysColor (COLORREF rgb, DWORD *pdwPhysColor) ;
	inline  DWORD   getColorKey 	   (DWORD dwColorKey) 	   { return m_dwVideoKeyColor ; };
    inline  void    incCount		   (void)                  { m_iCount++ ; } ;
    inline  int     getCount		   (void)                  { return m_iCount ; } ;
    
  private:  // internal state info
    LPDIRECTDRAW         m_pDDObject ;   // DirectDraw interface
    LPDIRECTDRAWSURFACE  m_pPrimary ;    // primary surface
    LPDIRECTDRAWSURFACE  m_pBackBuff ;   // back buffer attached to primary
    
    BOOL     m_bInExclMode ;     // Are we in exclusive mode now?
    RECT     m_RectScrn ;        // whole screen as a rect
    RECT     m_RectVideo ;       // current video position as rect
    DWORD    m_dwScrnColor ;     // physical color for surface filling
    DWORD    m_dwVideoKeyColor ; // physical color for color keying video area
    int      m_iCount ;          // flip count

    BOOL     m_bFrontBuff ;      // draw on front (or back) buffer?
    LPTSTR   m_szFrontMsg ;      // front surface string ("Front Buffer")
    LPTSTR   m_szBackMsg ;       // back surface string ("Back Buffer")
    LPTSTR   m_szDirection ;     // Direction string for users

    BOOL     m_bOverlayVisible ; // is overlay visible?
    IDDrawExclModeVideoCallback *m_pOverlayCallback ;  // overlay callback handler interface

	bool	 m_bReleaseDDraw ;	 // True if this class creates the DDraw object, if DDraw passed in, we don't
    							 // want to free it.
#ifdef SHOW_BALL
  public:
    void    SetBallPosition(DWORD dwVideoLeft, DWORD dwVideoTop, 
                            DWORD dwVideoWidth, DWORD dwVideoHeight) ;
    void    MoveBallPosition(int iDirX, int iDirY) ;
  private:
    int      m_iBallCenterX ;    // X-coord of ball's center
    int      m_iBallCenterY ;    // Y-coord of ball's center
    HPEN     m_hPen ;            // pen for drawing outline of the ball
    HBRUSH   m_hBrush ;          // brush for filling the ball
#endif
} ;



//
// Overlay callback handler object class
//
class COverlayCallback : public CUnknown, public IDDrawExclModeVideoCallback
{
public:
    COverlayCallback(CDDrawObject *pDDrawObj, HWND hWndApp, HRESULT *phr) ;
   ~COverlayCallback() ;

    DECLARE_IUNKNOWN
 
    // IDDrawExclModeVideoCallback interface methods
    //
    STDMETHODIMP OnUpdateOverlay(BOOL  bBefore,
                                 DWORD dwFlags,
                                 BOOL  bOldVisible,
                                 const RECT *prcSrcOld,
                                 const RECT *prcDestOld,
                                 BOOL  bNewVisible,
                                 const RECT *prcSrcNew,
                                 const RECT *prcDestNew) ;

    STDMETHODIMP OnUpdateColorKey(COLORKEY const *pKey,
                                  DWORD    dwColor) ;

    STDMETHODIMP OnUpdateSize(DWORD dwWidth, DWORD dwHeight, 
                              DWORD dwARWidth, DWORD dwARHeight) ;
  private:
    CDDrawObject *m_pDDrawObj ;
    HWND          m_hWndApp ;
} ;