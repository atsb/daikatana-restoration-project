#ifndef _WIN32
#  error You should not be including this file on this platform
#endif
#ifndef __GLW_WIN_H__
#define __GLW_WIN_H__

typedef struct
{
	HINSTANCE	hInstance;
	void	*wndproc;
	HDC     hDC;			// handle to device context
	HWND    hWnd;			// handle to window
	HGLRC   hGLRC;			// handle to GL rendering context
	HINSTANCE hinstOpenGL;	// HINSTANCE for the OpenGL library
	qboolean minidriver;
	qboolean allowdisplaydepthchange;
	qboolean mcd_accelerated;

	PIXELFORMATDESCRIPTOR	pfd;
	FILE *log_fp;
} glwstate_t;
extern glwstate_t glw_state;
#endif
