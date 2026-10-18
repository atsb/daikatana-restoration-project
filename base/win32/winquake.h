// winquake.h: Win32-specific Quake header file
#pragma warning( disable : 4229 )  // mgraph gets this
#include <windows.h>
#include <dsound.h>
#define	WINDOW_STYLE	(WS_OVERLAPPED|WS_BORDER|WS_CAPTION|WS_VISIBLE)

extern	HINSTANCE	global_hInstance;
extern LPDIRECTSOUND pDS;
extern LPDIRECTSOUNDBUFFER pDSBuf;
extern DWORD gSndBufSize;
extern HWND			cl_hwnd;
extern qboolean		ActiveApp, Minimized;

