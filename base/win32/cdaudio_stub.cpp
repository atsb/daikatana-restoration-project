/*
 * Daikatana x64 CD-audio compatibility shim.
 *
 * The original cd_win.cpp links against 32-bit Miles mss32.dll and therefore
 * cannot participate in a Win64 process. Game audio is supplied by the SDL3
 * audio engine; these functions preserve the historical CD-audio entry points
 * without introducing a 32-bit dependency into the x64 executable.
 */
#include <windows.h>
#include "qcommon.h"

int CDAudio_Init(void)
{
    return 0;
}

void CDAudio_Shutdown(void)
{
}

void CDAudio_Play(int track, qboolean looping)
{
    (void)track;
    (void)looping;
}

void CDAudio_Stop(void)
{
}

void CDAudio_Update(void)
{
}

void CDAudio_Activate(qboolean active)
{
    (void)active;
}


/*
 * Win32 window-message compatibility entry point.
 *
 * The SDL3 audio path does not use legacy MCI/CD-audio notifications, but
 * vid_dll.cpp still dispatches MM_MCINOTIFY to this historical entry point.
 * Keep the symbol available on Win64 without bringing back cd_win.cpp or
 * its 32-bit Miles dependency.
 */
LONG CDAudio_MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    (void)hWnd;
    (void)uMsg;
    (void)wParam;
    (void)lParam;
    return 0;
}
