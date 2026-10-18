/*
** GLW_IMP.C
**
** SDL3 implementation of the OpenGL platform layer.  SDL owns the native
** window, OpenGL context, fullscreen mode and buffer presentation.  The
** renderer itself remains the original fixed-function OpenGL renderer.
*/
#include <assert.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "../ref_gl/gl_local.h"
#include "glw_win.h"

qboolean QGL_Init(const char *dllname);
void QGL_Shutdown(void);

qboolean RImp_InitGL(void);

glwstate_t glw_state;

extern cvar_t *vid_fullscreen;
extern cvar_t *gl_mode;
extern cvar_t *gl_bitdepth;
extern cvar_t *r_msaa_samples;

static qboolean VerifyDriver(void)
{
    const char *renderer;

    renderer = (const char *)qglGetString(GL_RENDERER);
    if (renderer == NULL)
        return false;

    if (SDL_strcasecmp(renderer, "gdi generic") == 0 && !glw_state.mcd_accelerated)
        return false;

    return true;
}

static void RImp_SetGLAttributes(int msaa_samples)
{
    int color_bits;

    SDL_GL_ResetAttributes();
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    color_bits = (gl_bitdepth != NULL && gl_bitdepth->value != 0.0f) ?
        (int)gl_bitdepth->value : 32;

    if (color_bits <= 16)
    {
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    }
    else
    {
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    }

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 0);
    SDL_GL_SetAttribute(SDL_GL_STEREO,
                        ri.Cvar_Get("cl_stereo", "0", 0)->value != 0.0f ? 1 : 0);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, msaa_samples > 0 ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, msaa_samples > 0 ? msaa_samples : 0);
}

static qboolean RImp_SetFullscreenMode(int width, int height)
{
    SDL_DisplayID display;
    SDL_DisplayMode **modes;
    const SDL_DisplayMode *selected;
    int mode_count;
    int i;

    /*
     * The old Win32 renderer requested the exact video mode selected by
     * gl_mode.  The selected dimensions must be the actual exclusive
     * fullscreen display dimensions.  SDL's fullscreen mode list is therefore
     * the authority here; never silently substitute a desktop-sized window.
     */
    display = SDL_GetDisplayForWindow(glw_state.window);
    if (display == 0)
        display = SDL_GetPrimaryDisplay();
    if (display == 0)
        return false;

    modes = SDL_GetFullscreenDisplayModes(display, &mode_count);
    if (modes == NULL)
        return false;

    selected = NULL;
    for (i = 0; i < mode_count; ++i)
    {
        const SDL_DisplayMode *candidate = modes[i];
        if (candidate == NULL)
            continue;

        if (candidate->w == width &&
            candidate->h == height)
        {
            selected = candidate;
            break;
        }
    }

    if (selected == NULL)
    {
        SDL_free(modes);
        ri.Con_Printf(PRINT_ALL,
                      "...exact fullscreen mode %dx%d is not available on display\n",
                      width, height);
        return false;
    }

    if (!SDL_SetWindowFullscreenMode(glw_state.window, selected))
    {
        SDL_free(modes);
        return false;
    }

    ri.Con_Printf(PRINT_ALL,
                  "...fullscreen mode %dx%d @ %.2f Hz (density %.2fx)\n",
                  selected->w, selected->h, selected->refresh_rate,
                  selected->pixel_density);

    SDL_free(modes);
    return true;
}

/*
** RImp_GetBitDepth
*/
int RImp_GetBitDepth(void)
{
    int r, g, b, a;

    r = g = b = a = 0;
    SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &r);
    SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &g);
    SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &b);
    SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &a);
    return r + g + b + a;
}

/*
** RImp_GetZDepth
*/
int RImp_GetZDepth(void)
{
    int depth = 0;
    SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depth);
    return depth;
}

/*
** RImp_SetMode
*/
int RImp_SetMode(int *pwidth, int *pheight, int mode, qboolean fullscreen)
{
    int width, height;
    const char *win_fs[] = { "W", "FS" };

    ri.Con_Printf(PRINT_ALL, "Initializing OpenGL display\n");
    ri.Con_Printf(PRINT_ALL, "...setting mode %d:", mode);

    if (!ri.Vid_GetModeInfo(&width, &height, mode))
    {
        ri.Con_Printf(PRINT_ALL, " invalid mode\n");
        return rserr_invalid_mode;
    }

    ri.Con_Printf(PRINT_ALL, " %d %d %s\n", width, height, win_fs[fullscreen]);

    if (glw_state.window == NULL)
        return rserr_invalid_mode;

    if (fullscreen)
    {
        ri.Con_Printf(PRINT_ALL, "...setting SDL fullscreen mode\n");

        if (!RImp_SetFullscreenMode(width, height))
        {
            ri.Con_Printf(PRINT_ALL, "...SDL fullscreen mode selection failed\n");
            ri.Con_Printf(PRINT_ALL, "...falling back to windowed mode\n");

            if (!SDL_SetWindowFullscreenMode(glw_state.window, NULL) ||
                !SDL_SetWindowFullscreen(glw_state.window, false))
            {
                return rserr_invalid_mode;
            }

            SDL_SetWindowSize(glw_state.window, width, height);
            SDL_SyncWindow(glw_state.window);
            gl_state.fullscreen = false;
            *pwidth = width;
            *pheight = height;
            return rserr_invalid_fullscreen;
        }

        if (!SDL_SetWindowFullscreen(glw_state.window, true))
        {
            ri.Con_Printf(PRINT_ALL, "...SDL_SetWindowFullscreen failed: %s\n", SDL_GetError());
            return rserr_invalid_mode;
        }

        SDL_SyncWindow(glw_state.window);
        gl_state.fullscreen = true;
    }
    else
    {
        ri.Con_Printf(PRINT_ALL, "...setting windowed mode\n");

        if (!SDL_SetWindowFullscreen(glw_state.window, false))
        {
            ri.Con_Printf(PRINT_ALL, "...SDL_SetWindowFullscreen(false) failed: %s\n", SDL_GetError());
            return rserr_invalid_mode;
        }

        SDL_SetWindowFullscreenMode(glw_state.window, NULL);
        SDL_SetWindowSize(glw_state.window, width, height);
        SDL_SyncWindow(glw_state.window);
        gl_state.fullscreen = false;
    }

    *pwidth = width;
    *pheight = height;

    ri.Vid_NewWindow(width, height);
    return rserr_ok;
}

/*
** RImp_Shutdown
*/
void RImp_Shutdown(void)
{
    if (ri.Vid_SetSDLWindow != NULL)
        ri.Vid_SetSDLWindow(NULL);

    if (glw_state.context != NULL)
    {
        SDL_GL_MakeCurrent(glw_state.window, NULL);
        SDL_GL_DestroyContext(glw_state.context);
        glw_state.context = NULL;
    }

    if (glw_state.window != NULL)
    {
        SDL_DestroyWindow(glw_state.window);
        glw_state.window = NULL;
    }

    if (glw_state.log_fp)
    {
        fclose(glw_state.log_fp);
        glw_state.log_fp = NULL;
    }

    if (glw_state.sdl_video_initialized)
    {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        glw_state.sdl_video_initialized = false;
    }

    gl_state.fullscreen = false;
}

/*
** RImp_Init
*/
qboolean RImp_Init(void *unused_hinstance, void *unused_wndproc)
{
    int width, height, mode;
    qboolean fullscreen;
    cvar_t *stereo;
    int requested_msaa;
    int msaa_candidates[4];
    int candidate_count;
    int i;

    (void)unused_hinstance;
    (void)unused_wndproc;

    memset(&glw_state, 0, sizeof(glw_state));
    glw_state.allowdisplaydepthchange = false;
    glw_state.mcd_accelerated = true;

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
        ri.Con_Printf(PRINT_ALL, "RImp_Init() - SDL video init failed: %s\n", SDL_GetError());
        return false;
    }
    glw_state.sdl_video_initialized = true;

    /* Populate the mode table from the actual display before resolving
       gl_mode.  This lets a saved modern mode (for example 1920x1080) be
       restored directly instead of being silently reset to 640x480. */
    if (ri.Vid_RefreshModeList != NULL)
        ri.Vid_RefreshModeList();

    stereo = ri.Cvar_Get("cl_stereo", "0", 0);
    gl_state.stereo_enabled = (stereo->value != 0.0f);

    requested_msaa = (r_msaa_samples != NULL) ? (int)r_msaa_samples->value : 0;
    if (requested_msaa != 2 && requested_msaa != 4 && requested_msaa != 8)
        requested_msaa = 0;

    candidate_count = 0;
    if (requested_msaa >= 8)
    {
        msaa_candidates[candidate_count++] = 8;
        msaa_candidates[candidate_count++] = 4;
        msaa_candidates[candidate_count++] = 2;
    }
    else if (requested_msaa >= 4)
    {
        msaa_candidates[candidate_count++] = 4;
        msaa_candidates[candidate_count++] = 2;
    }
    else if (requested_msaa >= 2)
    {
        msaa_candidates[candidate_count++] = 2;
    }
    msaa_candidates[candidate_count++] = 0;

    mode = (int)gl_mode->value;
    if (!ri.Vid_GetModeInfo(&width, &height, mode))
    {
        mode = 3;
        if (!ri.Vid_GetModeInfo(&width, &height, mode))
        {
            RImp_Shutdown();
            return false;
        }
    }

    fullscreen = (vid_fullscreen->value != 0.0f);
    glw_state.window = NULL;
    glw_state.context = NULL;
    glw_state.msaa_samples = 0;

    for (i = 0; i < candidate_count; ++i)
    {
        qboolean candidate_fullscreen = fullscreen;

        RImp_SetGLAttributes(msaa_candidates[i]);

        glw_state.window = SDL_CreateWindow(
            "Daikatana - Restoration Edition",
            width,
            height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);

        if (glw_state.window == NULL)
        {
            ri.Con_Printf(PRINT_ALL,
                          "RImp_Init() - SDL_CreateWindow failed for MSAA %d: %s\n",
                          msaa_candidates[i], SDL_GetError());
            if (msaa_candidates[i] != 0)
                continue;
            RImp_Shutdown();
            return false;
        }

        if (!SDL_SetWindowResizable(glw_state.window, false))
            ri.Con_Printf(PRINT_ALL,
                          "RImp_Init() - SDL_SetWindowResizable failed: %s\n",
                          SDL_GetError());

        if (!candidate_fullscreen)
        {
            SDL_SetWindowPosition(glw_state.window,
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED);
        }
        else if (!RImp_SetFullscreenMode(width, height))
        {
            ri.Con_Printf(PRINT_ALL,
                          "RImp_Init() - requested fullscreen mode unavailable\n");
            candidate_fullscreen = false;
            SDL_SetWindowFullscreenMode(glw_state.window, NULL);
        }

        glw_state.context = SDL_GL_CreateContext(glw_state.window);
        if (glw_state.context != NULL)
        {
            fullscreen = candidate_fullscreen;
            break;
        }

        ri.Con_Printf(PRINT_ALL,
                      "RImp_Init() - SDL_GL_CreateContext failed for MSAA %d: %s\n",
                      msaa_candidates[i], SDL_GetError());
        SDL_DestroyWindow(glw_state.window);
        glw_state.window = NULL;
    }

    if (glw_state.context == NULL)
    {
        RImp_Shutdown();
        return false;
    }

    {
        int multisample_buffers = 0;
        int multisample_samples = 0;

        SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &multisample_buffers);
        SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &multisample_samples);

        if (!multisample_buffers ||
            (multisample_samples != 2 && multisample_samples != 4 && multisample_samples != 8))
            multisample_samples = 0;

        glw_state.msaa_samples = multisample_samples;

        if (r_msaa_samples != NULL)
        {
            r_msaa_samples->value = (float)multisample_samples;
            r_msaa_samples->modified = false;
        }

        if (glw_state.msaa_samples > 0)
            ri.Con_Printf(PRINT_ALL, "...using %dx MSAA\n", glw_state.msaa_samples);
        else if (requested_msaa > 0)
            ri.Con_Printf(PRINT_ALL, "...MSAA unavailable; using multisampling disabled\n");
    }

    glw_state.color_bits = RImp_GetBitDepth();
    glw_state.depth_bits = RImp_GetZDepth();
    gl_state.fullscreen = fullscreen;
    {
        int stereo_enabled = 0;
        SDL_GL_GetAttribute(SDL_GL_STEREO, &stereo_enabled);
        gl_state.stereo_enabled = stereo_enabled != 0;
    }

    /* Show the window before entering exclusive fullscreen.  SDL3 treats the
     * fullscreen transition as an asynchronous window-state change on some
     * platforms, so synchronize only after the fullscreen request. */
    SDL_ShowWindow(glw_state.window);
    if (fullscreen)
    {
        if (!SDL_SetWindowFullscreen(glw_state.window, true))
        {
            ri.Con_Printf(PRINT_ALL,
                          "RImp_Init() - SDL fullscreen activation failed: %s\n",
                          SDL_GetError());
            SDL_SetWindowFullscreenMode(glw_state.window, NULL);
            fullscreen = false;
            gl_state.fullscreen = false;
            SDL_SetWindowSize(glw_state.window, width, height);
        }
    }

    SDL_SyncWindow(glw_state.window);
    SDL_RaiseWindow(glw_state.window);

    if (fullscreen)
    {
        int pixel_width = 0;
        int pixel_height = 0;
        if (SDL_GetWindowSizeInPixels(glw_state.window, &pixel_width, &pixel_height))
        {
            if (pixel_width != width || pixel_height != height)
            {
                ri.Con_Printf(PRINT_ALL,
                              "RImp_Init() - fullscreen drawable is %dx%d, requested %dx%d\n",
                              pixel_width, pixel_height, width, height);
            }
        }
    }

    if (ri.Vid_SetSDLWindow != NULL)
        ri.Vid_SetSDLWindow(glw_state.window);

    return true;
}

qboolean RImp_InitGL(void)
{
    if (glw_state.window == NULL || glw_state.context == NULL)
        return false;

    if (!VerifyDriver())
    {
        ri.Con_Printf(PRINT_ALL, "RImp_InitGL() - no hardware acceleration detected\n");
        return false;
    }

    {
        GLint red, green, blue, alpha, depth;
        const GLubyte *vendor;
        const GLubyte *actual_renderer;
        const GLubyte *version;

        red = green = blue = alpha = depth = 0;
        qglGetIntegerv(GL_RED_BITS, &red);
        qglGetIntegerv(GL_GREEN_BITS, &green);
        qglGetIntegerv(GL_BLUE_BITS, &blue);
        qglGetIntegerv(GL_ALPHA_BITS, &alpha);
        qglGetIntegerv(GL_DEPTH_BITS, &depth);
        vendor = qglGetString(GL_VENDOR);
        actual_renderer = qglGetString(GL_RENDERER);
        version = qglGetString(GL_VERSION);

        ri.Con_Printf(PRINT_ALL, "GL_VENDOR: %s\n", vendor ? (const char *)vendor : "<unknown>");
        ri.Con_Printf(PRINT_ALL, "GL_RENDERER: %s\n", actual_renderer ? (const char *)actual_renderer : "<unknown>");
        ri.Con_Printf(PRINT_ALL, "GL_VERSION: %s\n", version ? (const char *)version : "<unknown>");
        ri.Con_Printf(PRINT_ALL, "GL framebuffer: R%d G%d B%d A%d Z%d\n",
                      (int)red, (int)green, (int)blue, (int)alpha, (int)depth);
    }
    return true;
}

/*
** RImp_BeginFrame
*/
void RImp_BeginFrame(float camera_separation)
{
    if (gl_bitdepth->modified)
    {
        gl_bitdepth->modified = false;
    }

    if (camera_separation < 0 && gl_state.stereo_enabled)
        qglDrawBuffer(GL_BACK_LEFT);
    else if (camera_separation > 0 && gl_state.stereo_enabled)
        qglDrawBuffer(GL_BACK_RIGHT);
    else
        qglDrawBuffer(GL_BACK);
}

/*
** RImp_CheckError
*/
void RImp_CheckError(void)
{
    int err;

    if (gl_no_error_check->value)
        return;

    err = qglGetError();

    if (err != GL_NO_ERROR)
    {
        switch (err)
        {
        case GL_INVALID_ENUM: assert(err != GL_INVALID_ENUM); break;
        case GL_INVALID_VALUE: assert(err != GL_INVALID_VALUE); break;
        case GL_INVALID_OPERATION: assert(err != GL_INVALID_OPERATION); break;
        case GL_STACK_OVERFLOW: assert(err != GL_STACK_OVERFLOW); break;
        case GL_STACK_UNDERFLOW: assert(err != GL_STACK_UNDERFLOW); break;
        case GL_OUT_OF_MEMORY: assert(err != GL_OUT_OF_MEMORY); break;
        default: assert(err == GL_NO_ERROR); break;
        }
    }
}

/*
** RImp_AppActivate
**
** SDL3 owns the video window.  Preserve the renderer callback semantics
** without reaching directly into Win32 window management.
*/
void RImp_AppActivate(qboolean active)
{
    if (glw_state.window == NULL)
        return;

    if (active)
    {
        SDL_RestoreWindow(glw_state.window);
        SDL_ShowWindow(glw_state.window);
        SDL_RaiseWindow(glw_state.window);
    }
    else
    {
        if (vid_fullscreen != NULL && vid_fullscreen->value)
            SDL_MinimizeWindow(glw_state.window);
    }
}

/*
** RImp_EndFrame
*/
void RImp_EndFrame(void)
{
    RImp_CheckError();

    if (SDL_strcasecmp(gl_drawbuffer->string, "GL_BACK") == 0)
    {
        if (!SDL_GL_SwapWindow(glw_state.window))
            ri.Sys_Error(ERR_FATAL, "RImp_EndFrame() - SDL_GL_SwapWindow() failed: %s\n", SDL_GetError());
    }
}
