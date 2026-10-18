/*
 * in_sdl3.cpp -- SDL3 keyboard, mouse and gamepad input backend.
 *
 * The legacy input API and command/movement logic remain intact.  Only the
 * device/event layer has been replaced with SDL3, so gameplay code continues
 * to see the original Quake key numbers and usercmd_t movement paths.
 */

#include <stdint.h>
#include <string.h>
#include <math.h>

#include <SDL3/SDL.h>

#include "client.h"
#include "dk_menu.h"
#include "dk_gce_main.h"
#include "input.h"

extern void AppActivate(int fActive, int minimize);

extern cvar_t* vid_fullscreen;

#define JOY_ABSOLUTE_AXIS   0x00000000U
#define JOY_RELATIVE_AXIS   0x00000010U
#define JOY_MAX_AXES        6
#define JOY_AXIS_X          0
#define JOY_AXIS_Y          1
#define JOY_AXIS_Z          2
#define JOY_AXIS_R          3
#define JOY_AXIS_U          4
#define JOY_AXIS_V          5

enum _ControlList
{
    AxisNada = 0,
    AxisForward,
    AxisLook,
    AxisSide,
    AxisTurn,
    AxisUp
};

static uint32_t dwAxisMap[JOY_MAX_AXES];
static uint32_t dwControlMap[JOY_MAX_AXES];
static uint32_t joy_raw_values[JOY_MAX_AXES];
static uint32_t *pdwRawValue[JOY_MAX_AXES];

cvar_t *in_mouse;
cvar_t *in_joystick;

cvar_t *joy_name;
cvar_t *joy_advanced;
cvar_t *joy_advaxisx;
cvar_t *joy_advaxisy;
cvar_t *joy_advaxisz;
cvar_t *joy_advaxisr;
cvar_t *joy_advaxisu;
cvar_t *joy_advaxisv;
cvar_t *joy_forwardthreshold;
cvar_t *joy_sidethreshold;
cvar_t *joy_pitchthreshold;
cvar_t *joy_yawthreshold;
cvar_t *joy_forwardsensitivity;
cvar_t *joy_sidesensitivity;
cvar_t *joy_pitchsensitivity;
cvar_t *joy_yawsensitivity;
cvar_t *joy_upthreshold;
cvar_t *joy_upsensitivity;

static bool joy_avail;
static bool joy_advancedinit;
static bool joy_haspov;
static uint32_t joy_oldbuttonstate;
static uint32_t joy_oldpovstate;
static int joy_numbuttons;

static SDL_Gamepad *joy_gamepad;
static SDL_JoystickID joy_instance_id;

static SDL_Window *g_sdl_window;
static bool g_sdl_initialized;

static bool mouseactive;
static bool mouseinitialized;
qboolean in_appactive;
static int mouse_buttons = 3;
static int mouse_oldbuttonstate;
static int mouse_x;
static int mouse_y;
static int old_mouse_x;
static int old_mouse_y;
static int mx_accum;
static int my_accum;

qboolean mlooking;
cvar_t *m_filter;
cvar_t *v_centermove;
cvar_t *v_centerspeed;

static const SDL_GamepadButton joy_buttons[] =
{
    SDL_GAMEPAD_BUTTON_SOUTH,
    SDL_GAMEPAD_BUTTON_EAST,
    SDL_GAMEPAD_BUTTON_WEST,
    SDL_GAMEPAD_BUTTON_NORTH,
    SDL_GAMEPAD_BUTTON_BACK,
    SDL_GAMEPAD_BUTTON_GUIDE,
    SDL_GAMEPAD_BUTTON_START,
    SDL_GAMEPAD_BUTTON_LEFT_STICK,
    SDL_GAMEPAD_BUTTON_RIGHT_STICK,
    SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,
    SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
    SDL_GAMEPAD_BUTTON_MISC1,
    SDL_GAMEPAD_BUTTON_MISC2,
    SDL_GAMEPAD_BUTTON_MISC3,
    SDL_GAMEPAD_BUTTON_MISC4,
    SDL_GAMEPAD_BUTTON_MISC5,
    SDL_GAMEPAD_BUTTON_MISC6
};

#define JOY_MAPPED_BUTTONS ((int)(sizeof(joy_buttons) / sizeof(joy_buttons[0])))

static unsigned IN_EventTime(void)
{
    /* Key_Event timestamps must use the same clock as sys_frame_time. */
    return Sys_Milliseconds();
}

static int IN_MapScancode(SDL_Scancode scancode)
{
    if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z)
        return 'a' + ((int)scancode - (int)SDL_SCANCODE_A);

    if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_9)
        return '1' + ((int)scancode - (int)SDL_SCANCODE_1);

    switch (scancode)
    {
    case SDL_SCANCODE_0: return '0';
    case SDL_SCANCODE_MINUS: return '-';
    case SDL_SCANCODE_EQUALS: return '=';
    case SDL_SCANCODE_LEFTBRACKET: return '[';
    case SDL_SCANCODE_RIGHTBRACKET: return ']';
    case SDL_SCANCODE_SEMICOLON: return ';';
    case SDL_SCANCODE_APOSTROPHE: return '\'';
    case SDL_SCANCODE_GRAVE: return '`';
    case SDL_SCANCODE_BACKSLASH:
    case SDL_SCANCODE_NONUSBACKSLASH: return '\\';
    case SDL_SCANCODE_COMMA: return ',';
    case SDL_SCANCODE_PERIOD: return '.';
    case SDL_SCANCODE_SLASH: return '/';
    case SDL_SCANCODE_KP_MULTIPLY: return '*';
    case SDL_SCANCODE_SPACE: return K_SPACE;
    case SDL_SCANCODE_TAB: return K_TAB;
    case SDL_SCANCODE_RETURN: return K_ENTER;
    case SDL_SCANCODE_ESCAPE: return K_ESCAPE;
    case SDL_SCANCODE_BACKSPACE: return K_BACKSPACE;
    case SDL_SCANCODE_UP: return K_UPARROW;
    case SDL_SCANCODE_DOWN: return K_DOWNARROW;
    case SDL_SCANCODE_LEFT: return K_LEFTARROW;
    case SDL_SCANCODE_RIGHT: return K_RIGHTARROW;
    case SDL_SCANCODE_LALT:
    case SDL_SCANCODE_RALT: return K_ALT;
    case SDL_SCANCODE_LCTRL:
    case SDL_SCANCODE_RCTRL: return K_CTRL;
    case SDL_SCANCODE_LSHIFT:
    case SDL_SCANCODE_RSHIFT: return K_SHIFT;
    case SDL_SCANCODE_F1: return K_F1;
    case SDL_SCANCODE_F2: return K_F2;
    case SDL_SCANCODE_F3: return K_F3;
    case SDL_SCANCODE_F4: return K_F4;
    case SDL_SCANCODE_F5: return K_F5;
    case SDL_SCANCODE_F6: return K_F6;
    case SDL_SCANCODE_F7: return K_F7;
    case SDL_SCANCODE_F8: return K_F8;
    case SDL_SCANCODE_F9: return K_F9;
    case SDL_SCANCODE_F10: return K_F10;
    case SDL_SCANCODE_F11: return K_F11;
    case SDL_SCANCODE_F12: return K_F12;
    case SDL_SCANCODE_PAUSE: return K_PAUSE;
    case SDL_SCANCODE_INSERT: return K_INS;
    case SDL_SCANCODE_DELETE: return K_DEL;
    case SDL_SCANCODE_HOME: return K_HOME;
    case SDL_SCANCODE_END: return K_END;
    case SDL_SCANCODE_PAGEUP: return K_PGUP;
    case SDL_SCANCODE_PAGEDOWN: return K_PGDN;
    case SDL_SCANCODE_KP_7: return K_KP_HOME;
    case SDL_SCANCODE_KP_8: return K_KP_UPARROW;
    case SDL_SCANCODE_KP_9: return K_KP_PGUP;
    case SDL_SCANCODE_KP_4: return K_KP_LEFTARROW;
    case SDL_SCANCODE_KP_5: return K_KP_5;
    case SDL_SCANCODE_KP_6: return K_KP_RIGHTARROW;
    case SDL_SCANCODE_KP_1: return K_KP_END;
    case SDL_SCANCODE_KP_2: return K_KP_DOWNARROW;
    case SDL_SCANCODE_KP_3: return K_KP_PGDN;
    case SDL_SCANCODE_KP_ENTER: return K_KP_ENTER;
    case SDL_SCANCODE_KP_0: return K_KP_INS;
    case SDL_SCANCODE_KP_PERIOD: return K_KP_DEL;
    case SDL_SCANCODE_KP_DIVIDE: return K_KP_SLASH;
    case SDL_SCANCODE_KP_MINUS: return K_KP_MINUS;
    case SDL_SCANCODE_KP_PLUS: return K_KP_PLUS;
    default: break;
    }

    return 0;
}

static void IN_KeyEvent(const SDL_KeyboardEvent *event)
{
    int key;

    if (event == NULL)
        return;

    key = IN_MapScancode(event->scancode);
    if (!key)
        return;

    if (event->down && event->scancode == SDL_SCANCODE_RETURN &&
        (event->mod & SDL_KMOD_ALT))
    {
        if (vid_fullscreen != NULL)
            Cvar_SetValue("vid_fullscreen", !vid_fullscreen->value);
        return;
    }

    Key_Event(key, event->down ? true : false, IN_EventTime());
}

void IN_MLookDown(void)
{
    mlooking = true;
}

void IN_MLookUp(void)
{
    mlooking = false;
    if (!freelook->value && lookspring->value)
        IN_CenterView();
}

static void IN_CloseGamepad(void)
{
    int i;

    if (joy_gamepad != NULL)
    {
        for (i = 0; i < JOY_MAPPED_BUTTONS; i++)
        {
            if (joy_oldbuttonstate & (1U << i))
                Key_Event((i < 4) ? (K_JOY1 + i) : (K_AUX1 + i), false, IN_EventTime());
        }

        for (i = 0; i < 4; i++)
        {
            if (joy_oldpovstate & (1U << i))
                Key_Event(K_AUX29 + i, false, IN_EventTime());
        }

        SDL_CloseGamepad(joy_gamepad);
        joy_gamepad = NULL;
        joy_instance_id = 0;
    }

    joy_oldbuttonstate = 0;
    joy_oldpovstate = 0;
    joy_avail = false;
    joy_haspov = false;
    joy_numbuttons = 0;
    for (i = 0; i < JOY_MAX_AXES; i++)
        joy_raw_values[i] = 32768;
}

static bool IN_OpenGamepadByID(SDL_JoystickID instance_id)
{
    SDL_Gamepad *gamepad;
    const char *name;

    if (joy_gamepad != NULL)
        return true;

    gamepad = SDL_OpenGamepad(instance_id);
    if (gamepad == NULL)
        return false;

    joy_gamepad = gamepad;
    joy_instance_id = SDL_GetGamepadID(gamepad);
    joy_avail = true;
    joy_haspov = true;
    joy_numbuttons = JOY_MAPPED_BUTTONS;
    joy_oldbuttonstate = 0;
    joy_oldpovstate = 0;
    joy_advancedinit = false;

    name = SDL_GetGamepadName(gamepad);
    if (name != NULL)
    {
        Com_Printf("\nSDL3 gamepad detected: %s\n\n", name);
        if (joy_name != NULL && strcmp(joy_name->string, "joystick") == 0)
            Cvar_Set("joy_name", name);
    }

    return true;
}

static bool IN_OpenFirstGamepad(void)
{
    SDL_JoystickID *ids;
    int count;
    bool result;

    ids = SDL_GetGamepads(&count);
    if (ids == NULL || count <= 0)
    {
        if (ids != NULL)
            SDL_free(ids);
        return false;
    }

    result = IN_OpenGamepadByID(ids[0]);
    SDL_free(ids);
    return result;
}

static void IN_UpdateGamepadAxes(void)
{
    Sint16 value;

    if (!joy_gamepad)
        return;

    value = SDL_GetGamepadAxis(joy_gamepad, SDL_GAMEPAD_AXIS_LEFTX);
    joy_raw_values[JOY_AXIS_X] = (uint32_t)((int)value + 32768);

    value = SDL_GetGamepadAxis(joy_gamepad, SDL_GAMEPAD_AXIS_LEFTY);
    joy_raw_values[JOY_AXIS_Y] = (uint32_t)((int)value + 32768);

    value = SDL_GetGamepadAxis(joy_gamepad, SDL_GAMEPAD_AXIS_RIGHTX);
    joy_raw_values[JOY_AXIS_R] = (uint32_t)((int)value + 32768);

    value = SDL_GetGamepadAxis(joy_gamepad, SDL_GAMEPAD_AXIS_RIGHTY);
    joy_raw_values[JOY_AXIS_U] = (uint32_t)((int)value + 32768);

    /* SDL3 exposes triggers as 0..32767 rather than centered axes. Keep them
     * neutral in the legacy six-axis map so advanced configurations do not
     * accidentally interpret an unpressed trigger as full negative travel. */
    joy_raw_values[JOY_AXIS_Z] = 32768;
    joy_raw_values[JOY_AXIS_V] = 32768;
}

static void IN_HandleGamepadRemoved(SDL_JoystickID instance_id)
{
    if (joy_gamepad != NULL && joy_instance_id == instance_id)
        IN_CloseGamepad();
}

static void IN_PumpEventsInternal(void)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            Com_Quit();
            break;

        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            IN_KeyEvent(&event.key);
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        {
            int index;
            int mask;
            switch (event.button.button)
            {
            case SDL_BUTTON_LEFT:   index = 0; break;
            case SDL_BUTTON_RIGHT:  index = 1; break;
            case SDL_BUTTON_MIDDLE: index = 2; break;
            default: index = -1; break;
            }
            if (index >= 0)
            {
                mask = 1 << index;
                if (!(mouse_oldbuttonstate & mask))
                    Key_Event(K_MOUSE1 + index, true, IN_EventTime());
                mouse_oldbuttonstate |= mask;
            }
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_UP:
        {
            int index;
            int mask;
            switch (event.button.button)
            {
            case SDL_BUTTON_LEFT:   index = 0; break;
            case SDL_BUTTON_RIGHT:  index = 1; break;
            case SDL_BUTTON_MIDDLE: index = 2; break;
            default: index = -1; break;
            }
            if (index >= 0)
            {
                mask = 1 << index;
                if (mouse_oldbuttonstate & mask)
                    Key_Event(K_MOUSE1 + index, false, IN_EventTime());
                mouse_oldbuttonstate &= ~mask;
            }
            break;
        }

        case SDL_EVENT_MOUSE_WHEEL:
            if (event.wheel.y > 0.0f)
            {
                Key_Event(K_MWHEELUP, true, IN_EventTime());
                Key_Event(K_MWHEELUP, false, IN_EventTime());
            }
            else if (event.wheel.y < 0.0f)
            {
                Key_Event(K_MWHEELDOWN, true, IN_EventTime());
                Key_Event(K_MWHEELDOWN, false, IN_EventTime());
            }
            break;

        case SDL_EVENT_WINDOW_FOCUS_LOST:
            IN_Activate(false);
            AppActivate(false, false);
            break;

        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            IN_Activate(true);
            AppActivate(true, false);
            break;

        case SDL_EVENT_GAMEPAD_ADDED:
            if (in_joystick != NULL && in_joystick->value)
                IN_OpenGamepadByID(event.gdevice.which);
            break;

        case SDL_EVENT_GAMEPAD_REMOVED:
            IN_HandleGamepadRemoved(event.gdevice.which);
            break;

        default:
            break;
        }
    }
}

void IN_PumpEvents(void)
{
    if (!g_sdl_initialized)
        return;
    IN_PumpEventsInternal();
}

void IN_SetSDLWindow(SDL_Window *window)
{
    g_sdl_window = window;
}

void IN_ActivateMouse(void)
{
    if (!mouseinitialized || g_sdl_window == NULL || !in_mouse->value)
    {
        mouseactive = false;
        return;
    }

    /* Always reassert the gameplay mouse state.  SDL/window managers can
       restore a visible cursor or drop the grab during a menu transition,
       even though our local mouseactive flag is still set. */
    if (!SDL_GetWindowMouseGrab(g_sdl_window))
    {
        if (!SDL_SetWindowMouseGrab(g_sdl_window, true))
            Com_Printf("SDL3: failed to grab mouse window: %s\n", SDL_GetError());
    }

    if (!SDL_GetWindowRelativeMouseMode(g_sdl_window))
    {
        if (!SDL_SetWindowRelativeMouseMode(g_sdl_window, true))
        {
            Com_Printf("SDL3: failed to enable relative mouse mode: %s\n", SDL_GetError());
            mouseactive = false;
            return;
        }

        SDL_WarpMouseInWindow(g_sdl_window, (float)viddef.width * 0.5f,
                              (float)viddef.height * 0.5f);

        old_mouse_x = old_mouse_y = 0;
        mouse_x = mouse_y = 0;
        mx_accum = my_accum = 0;
    }

    SDL_HideCursor();
    mouseactive = true;
}

qboolean IN_HandleSetCursor(void)
{
    if (g_sdl_window == NULL)
        return false;

    if (!mouseactive)
        return false;

    SDL_HideCursor();
    return true;
}

void IN_DeactivateMouse(void)
{
    if (!mouseinitialized)
        return;

    if (g_sdl_window != NULL)
    {
        SDL_SetWindowRelativeMouseMode(g_sdl_window, false);
        SDL_SetWindowMouseGrab(g_sdl_window, false);
    }

    mouseactive = false;

    /* In an active fullscreen game/menu state the OS cursor must remain
       hidden.  Show it only when the application has actually lost focus or
       when running windowed. */
    if (vid_fullscreen != NULL && vid_fullscreen->value != 0.0f && in_appactive)
        SDL_HideCursor();
    else
        SDL_ShowCursor();
}

static void IN_StartupMouse(void)
{
    cvar_t *cv;

    cv = Cvar_Get("in_initmouse", "1", CVAR_NOSET);
    if (!cv->value)
        return;

    mouseinitialized = true;
    mouse_buttons = 3;
}

static void IN_MouseMoveMenu(void)
{
    float x;
    float y;

    if (g_sdl_window == NULL)
        return;

    SDL_GetMouseState(&x, &y);
    DKM_MousePos((int)x, (int)y);
}

static void IN_MouseMove(usercmd_t *cmd)
{
    float fmx;
    float fmy;
    int mx;
    int my;
    static int zerocount;

    if (cls.key_dest == key_menu)
    {
        IN_MouseMoveMenu();
        return;
    }

    if (!mouseactive)
        return;

    fmx = 0.0f;
    fmy = 0.0f;
    if (g_sdl_window != NULL)
        SDL_GetRelativeMouseState(&fmx, &fmy);

    mx = (int)fmx;
    my = (int)fmy;

    if (CIN_GCE_MouseMove(mx, my) == 1)
        return;

    DKM_MouseMove(mx, my);

    if ((mx == 0) && (my == 0))
    {
        zerocount++;
        if (zerocount == 1)
        {
            mx = old_mouse_x;
            my = old_mouse_y;
        }
    }
    else
    {
        zerocount = 0;
    }

    if (!mx && !my)
        return;

    if (m_filter->value)
    {
        mouse_x = (mx + old_mouse_x) / 2;
        mouse_y = (my + old_mouse_y) / 2;
    }
    else
    {
        mouse_x = mx;
        mouse_y = my;
    }

    old_mouse_x = mx;
    old_mouse_y = my;

    mouse_x = (int)(mouse_x * sensitivity->value);
    mouse_y = (int)(mouse_y * sensitivity->value);

    if ((in_strafe.state & 1) || (lookstrafe->value && mlooking))
        cmd->sidemove += m_side->value * mouse_x;
    else
        cl.viewangles.y -= m_yaw->value * mouse_x;

    if ((mlooking || freelook->value) && !(in_strafe.state & 1))
        cl.viewangles.x += m_pitch->value * mouse_y;
    else
        cmd->forwardmove -= m_forward->value * mouse_y;
}

static uint32_t *RawValuePointer(int axis)
{
    if (axis < 0 || axis >= JOY_MAX_AXES)
        return &joy_raw_values[JOY_AXIS_Z];
    return &joy_raw_values[axis];
}

static void Joy_AdvancedUpdate_f(void)
{
    int i;
    uint32_t dwTemp;

    for (i = 0; i < JOY_MAX_AXES; i++)
    {
        dwAxisMap[i] = AxisNada;
        dwControlMap[i] = JOY_ABSOLUTE_AXIS;
        pdwRawValue[i] = RawValuePointer(i);
    }

    if (joy_advanced->value == 0.0)
    {
        dwAxisMap[JOY_AXIS_X] = AxisTurn;
        dwAxisMap[JOY_AXIS_Y] = AxisForward;
    }
    else
    {
        if (strcmp(joy_name->string, "joystick") != 0)
            Com_Printf("\n%s configured\n\n", joy_name->string);

        dwTemp = (uint32_t)joy_advaxisx->value;
        dwAxisMap[JOY_AXIS_X] = dwTemp & 0x0000000fU;
        dwControlMap[JOY_AXIS_X] = dwTemp & JOY_RELATIVE_AXIS;

        dwTemp = (uint32_t)joy_advaxisy->value;
        dwAxisMap[JOY_AXIS_Y] = dwTemp & 0x0000000fU;
        dwControlMap[JOY_AXIS_Y] = dwTemp & JOY_RELATIVE_AXIS;

        dwTemp = (uint32_t)joy_advaxisz->value;
        dwAxisMap[JOY_AXIS_Z] = dwTemp & 0x0000000fU;
        dwControlMap[JOY_AXIS_Z] = dwTemp & JOY_RELATIVE_AXIS;

        dwTemp = (uint32_t)joy_advaxisr->value;
        dwAxisMap[JOY_AXIS_R] = dwTemp & 0x0000000fU;
        dwControlMap[JOY_AXIS_R] = dwTemp & JOY_RELATIVE_AXIS;

        dwTemp = (uint32_t)joy_advaxisu->value;
        dwAxisMap[JOY_AXIS_U] = dwTemp & 0x0000000fU;
        dwControlMap[JOY_AXIS_U] = dwTemp & JOY_RELATIVE_AXIS;

        dwTemp = (uint32_t)joy_advaxisv->value;
        dwAxisMap[JOY_AXIS_V] = dwTemp & 0x0000000fU;
        dwControlMap[JOY_AXIS_V] = dwTemp & JOY_RELATIVE_AXIS;
    }
}

void IN_StartupJoystick(void)
{
    cvar_t *cv;

    joy_avail = false;
    joy_gamepad = NULL;

    cv = Cvar_Get("in_initjoy", "1", CVAR_NOSET);
    if (!cv->value)
        return;

    if (!IN_OpenFirstGamepad())
        return;
}

void IN_Commands(void)
{
    uint32_t buttonstate;
    uint32_t povstate;
    int i;

    if (in_joystick->value && joy_gamepad == NULL)
        IN_OpenFirstGamepad();

    if (!joy_avail || joy_gamepad == NULL || !in_joystick->value)
        return;

    buttonstate = 0;
    for (i = 0; i < JOY_MAPPED_BUTTONS; i++)
    {
        if (SDL_GetGamepadButton(joy_gamepad, joy_buttons[i]))
            buttonstate |= (1U << i);
    }

    for (i = 0; i < joy_numbuttons; i++)
    {
        if ((buttonstate & (1U << i)) && !(joy_oldbuttonstate & (1U << i)))
            Key_Event((i < 4) ? (K_JOY1 + i) : (K_AUX1 + i), true, 0);
        if (!(buttonstate & (1U << i)) && (joy_oldbuttonstate & (1U << i)))
            Key_Event((i < 4) ? (K_JOY1 + i) : (K_AUX1 + i), false, 0);
    }
    joy_oldbuttonstate = buttonstate;

    if (joy_haspov)
    {
        povstate = 0;
        if (SDL_GetGamepadButton(joy_gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP))
            povstate |= 0x01;
        if (SDL_GetGamepadButton(joy_gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT))
            povstate |= 0x02;
        if (SDL_GetGamepadButton(joy_gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN))
            povstate |= 0x04;
        if (SDL_GetGamepadButton(joy_gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT))
            povstate |= 0x08;

        for (i = 0; i < 4; i++)
        {
            if ((povstate & (1U << i)) && !(joy_oldpovstate & (1U << i)))
                Key_Event(K_AUX29 + i, true, 0);
            if (!(povstate & (1U << i)) && (joy_oldpovstate & (1U << i)))
                Key_Event(K_AUX29 + i, false, 0);
        }
        joy_oldpovstate = povstate;
    }
}

static void IN_JoyMove(usercmd_t *cmd)
{
    float speed;
    float aspeed;
    float fAxisValue;
    int i;

    if (joy_advancedinit != true)
    {
        Joy_AdvancedUpdate_f();
        joy_advancedinit = true;
    }

    if (!joy_avail || !joy_gamepad || !in_joystick->value)
        return;

    IN_UpdateGamepadAxes();

    if ((in_speed.state & 1) ^ (int)cl_run->value)
        speed = 2.0f;
    else
        speed = 1.0f;

    aspeed = speed * cls.frametime;

    for (i = 0; i < JOY_MAX_AXES; i++)
    {
        fAxisValue = (float)*pdwRawValue[i];
        fAxisValue -= 32768.0f;
        fAxisValue /= 32768.0f;

        switch (dwAxisMap[i])
        {
        case AxisForward:
            if ((joy_advanced->value == 0.0) && mlooking)
            {
                if (fabs(fAxisValue) > joy_pitchthreshold->value)
                {
                    if (m_pitch->value < 0.0)
                        cl.viewangles.x -= (fAxisValue * joy_pitchsensitivity->value) * aspeed * cl_pitchspeed->value;
                    else
                        cl.viewangles.x += (fAxisValue * joy_pitchsensitivity->value) * aspeed * cl_pitchspeed->value;
                }
            }
            else if (fabs(fAxisValue) > joy_forwardthreshold->value)
            {
                cmd->forwardmove += (fAxisValue * joy_forwardsensitivity->value) * speed * cl_forwardspeed->value;
            }
            break;

        case AxisSide:
            if (fabs(fAxisValue) > joy_sidethreshold->value)
                cmd->sidemove += (fAxisValue * joy_sidesensitivity->value) * speed * cl_sidespeed->value;
            break;

        case AxisUp:
            if (fabs(fAxisValue) > joy_upthreshold->value)
                cmd->upmove += (fAxisValue * joy_upsensitivity->value) * speed * cl_upspeed->value;
            break;

        case AxisTurn:
            if ((in_strafe.state & 1) || (lookstrafe->value && mlooking))
            {
                if (fabs(fAxisValue) > joy_sidethreshold->value)
                    cmd->sidemove -= (fAxisValue * joy_sidesensitivity->value) * speed * cl_sidespeed->value;
            }
            else if (fabs(fAxisValue) > joy_yawthreshold->value)
            {
                if (dwControlMap[i] == JOY_ABSOLUTE_AXIS)
                    cl.viewangles.y += (fAxisValue * joy_yawsensitivity->value) * aspeed * cl_yawspeed->value;
                else
                    cl.viewangles.y += (fAxisValue * joy_yawsensitivity->value) * speed * 180.0f;
            }
            break;

        case AxisLook:
            if (mlooking && fabs(fAxisValue) > joy_pitchthreshold->value)
            {
                if (dwControlMap[i] == JOY_ABSOLUTE_AXIS)
                    cl.viewangles.x += (fAxisValue * joy_pitchsensitivity->value) * aspeed * cl_pitchspeed->value;
                else
                    cl.viewangles.x += (fAxisValue * joy_pitchsensitivity->value) * speed * 180.0f;
            }
            break;

        default:
            break;
        }
    }
}

void IN_Init(void)
{
    m_filter = Cvar_Get("m_filter", "0", CVAR_ARCHIVE);
    in_mouse = Cvar_Get("in_mouse", "1", CVAR_ARCHIVE);

    in_joystick = Cvar_Get("in_joystick", "0", CVAR_ARCHIVE);
    joy_name = Cvar_Get("joy_name", "joystick", 0);
    joy_advanced = Cvar_Get("joy_advanced", "0", 0);
    joy_advaxisx = Cvar_Get("joy_advaxisx", "0", 0);
    joy_advaxisy = Cvar_Get("joy_advaxisy", "0", 0);
    joy_advaxisz = Cvar_Get("joy_advaxisz", "0", 0);
    joy_advaxisr = Cvar_Get("joy_advaxisr", "0", 0);
    joy_advaxisu = Cvar_Get("joy_advaxisu", "0", 0);
    joy_advaxisv = Cvar_Get("joy_advaxisv", "0", 0);
    joy_forwardthreshold = Cvar_Get("joy_forwardthreshold", "0.15", 0);
    joy_sidethreshold = Cvar_Get("joy_sidethreshold", "0.15", 0);
    joy_upthreshold = Cvar_Get("joy_upthreshold", "0.15", 0);
    joy_pitchthreshold = Cvar_Get("joy_pitchthreshold", "0.15", 0);
    joy_yawthreshold = Cvar_Get("joy_yawthreshold", "0.15", 0);
    joy_forwardsensitivity = Cvar_Get("joy_forwardsensitivity", "-1", 0);
    joy_sidesensitivity = Cvar_Get("joy_sidesensitivity", "-1", 0);
    joy_upsensitivity = Cvar_Get("joy_upsensitivity", "-1", 0);
    joy_pitchsensitivity = Cvar_Get("joy_pitchsensitivity", "1", 0);
    joy_yawsensitivity = Cvar_Get("joy_yawsensitivity", "-1", 0);

    v_centermove = Cvar_Get("v_centermove", "0.15", 0);
    v_centerspeed = Cvar_Get("v_centerspeed", "500", 0);

    Cmd_AddCommand("+mlook", IN_MLookDown);
    Cmd_AddCommand("-mlook", IN_MLookUp);
    Cmd_AddCommand("joy_advancedupdate", Joy_AdvancedUpdate_f);

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        Com_Printf("SDL3 input init failed: %s\n", SDL_GetError());
        mouseinitialized = false;
        return;
    }

    /* FPS input must never leave a hardware/Windows cursor visible while
       relative mouse mode is active. */
    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_CURSOR_VISIBLE, "0");

    g_sdl_initialized = true;

    if (g_sdl_window == NULL)
        Com_Printf("SDL3 input: no SDL window is available yet\n");

    IN_StartupMouse();
    IN_StartupJoystick();
}

void IN_Shutdown(void)
{
    IN_CloseGamepad();
    IN_DeactivateMouse();

    g_sdl_window = NULL;
    g_sdl_initialized = false;
    SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void IN_Activate(qboolean active)
{
    if (active)
    {
        in_appactive = true;
        mouseactive = false;
    }
    else
    {
        IN_DeactivateMouse();
        in_appactive = false;
    }
}

void IN_Frame(void)
{
    if (!mouseinitialized)
        return;

    if (in_mouse == NULL || in_appactive == false)
    {
        IN_DeactivateMouse();
        return;
    }

    if (Cvar_VariableValue("paused"))
    {
        IN_DeactivateMouse();
        SDL_HideCursor();
        return;
    }

    if (!cl.refresh_prepped ||
        cls.key_dest == key_console ||
        cls.key_dest == key_menu)
    {
        if (Cvar_VariableValue("vid_fullscreen") == 0)
        {
            IN_DeactivateMouse();
            return;
        }
    }

    IN_ActivateMouse();

    /* Fullscreen gameplay is an exclusive mouse state: keep the SDL window
       grabbed and the cursor hidden even if the OS/window manager touched
       the pointer during the preceding menu transition. */
    if (vid_fullscreen != NULL && vid_fullscreen->value != 0.0f &&
        cls.key_dest == key_game && g_sdl_window != NULL)
    {
        if (!SDL_GetWindowMouseGrab(g_sdl_window))
            SDL_SetWindowMouseGrab(g_sdl_window, true);
        if (!SDL_GetWindowRelativeMouseMode(g_sdl_window))
            SDL_SetWindowRelativeMouseMode(g_sdl_window, true);
        SDL_HideCursor();
    }
}

void IN_Move(usercmd_t *cmd)
{
    IN_MouseMove(cmd);
    if (in_appactive)
        IN_JoyMove(cmd);
}

void IN_ClearStates(void)
{
    mx_accum = 0;
    my_accum = 0;
    mouse_oldbuttonstate = 0;
}

void IN_MenuMouseSetup(void)
{
    if (g_sdl_window == NULL)
        return;

    IN_DeactivateMouse();
    SDL_SetWindowRelativeMouseMode(g_sdl_window, false);
    SDL_GetRelativeMouseState(NULL, NULL);
    SDL_SetWindowMouseGrab(g_sdl_window, true);
    SDL_WarpMouseInWindow(g_sdl_window, (float)viddef.width * 0.5f,
                          (float)viddef.height * 0.5f);
    old_mouse_x = old_mouse_y = 0;
    mouse_x = mouse_y = 0;
    mx_accum = my_accum = 0;
    SDL_HideCursor();
}

void IN_MenuMouseRelease(void)
{
    if (g_sdl_window != NULL)
    {
        SDL_SetWindowMouseGrab(g_sdl_window, false);
        SDL_SetWindowRelativeMouseMode(g_sdl_window, false);
        SDL_GetRelativeMouseState(NULL, NULL);
    }
    old_mouse_x = old_mouse_y = 0;
    mouse_x = mouse_y = 0;
    mx_accum = my_accum = 0;

    /* Never expose the Windows cursor while the game is fullscreen. */
    if (vid_fullscreen != NULL && vid_fullscreen->value != 0.0f)
        SDL_HideCursor();
    else
        SDL_ShowCursor();
}
