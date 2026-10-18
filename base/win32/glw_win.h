#ifndef __GLW_WIN_H__
#define __GLW_WIN_H__

#include <SDL3/SDL.h>

/*
 * SDL3 owns the window and OpenGL context.  This state intentionally contains
 * no Win32 window, device-context, pixel-format, or WGL handles.
 */
typedef struct
{
    SDL_Window *window;
    SDL_GLContext context;
    qboolean sdl_video_initialized;
    qboolean mcd_accelerated;
    qboolean allowdisplaydepthchange;
    int color_bits;
    int depth_bits;
    int msaa_samples;
    FILE *log_fp;
} glwstate_t;

extern glwstate_t glw_state;

#endif
