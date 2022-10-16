#ifndef __RW_WIN_H__
#define __RW_WIN_H__
#include <windows.h>
#include <ddraw.h>
typedef struct
{
	HINSTANCE		hInstance;
	void			*wndproc;
	HDC				hDC;				// global DC we're using
	HWND			hWnd;				// HWND of parent window
	HDC				hdcDIBSection;		// DC compatible with DIB section
	HBITMAP			hDIBSection;		// DIB section
	unsigned char	*pDIBBase;			// DIB base pointer, NOT used directly for rendering!
	HPALETTE		hPal;				// palette we're using
	HPALETTE		hpalOld;			// original system palette
	COLORREF		oldsyscolors[20];	// original system colors
	HINSTANCE		hinstDDRAW;			// library instance for DDRAW.DLL
	LPDIRECTDRAW 	lpDirectDraw;		// pointer to DirectDraw object
	LPDIRECTDRAWSURFACE lpddsFrontBuffer;	// video card display memory front buffer
	LPDIRECTDRAWSURFACE lpddsBackBuffer;	// system memory backbuffer
	LPDIRECTDRAWSURFACE lpddsOffScreenBuffer;	// system memory backbuffer
	LPDIRECTDRAWPALETTE	lpddpPalette;		// DirectDraw palette
	qboolean		palettized;			// true if desktop is paletted
	qboolean		modex;
	qboolean		initializing;
	RECT			view_rect;
	RECT			screen_rect;
} swwstate_t;
extern swwstate_t sww_state;
/*
** DIB code
*/
qboolean DIB_Init( unsigned char **ppbuffer, int *ppitch );
void     DIB_Shutdown( void );
void     DIB_SetPalette( const unsigned char *palette );
qboolean DDRAW_Init( unsigned char **ppbuffer, int *ppitch );
void     DDRAW_Shutdown( void );
void     DDRAW_SetPalette( const unsigned char *palette );
#endif
