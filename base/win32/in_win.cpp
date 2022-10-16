// in_win.c -- windows 95 mouse and joystick code
// 02/21/97 JCB Added extended DirectInput code to support external controllers.
#include "client.h"
#include "winquake.h"
#include "dk_menu.h"

#include "dk_gce_main.h"

extern	unsigned	sys_msg_time;
// joystick defines and variables
// where should defines be moved?
#define JOY_ABSOLUTE_AXIS	0x00000000		// control like a joystick
#define JOY_RELATIVE_AXIS	0x00000010		// control like a mouse, spinner, trackball
#define	JOY_MAX_AXES		6				// X, Y, Z, R, U, V
#define JOY_AXIS_X			0
#define JOY_AXIS_Y			1
#define JOY_AXIS_Z			2
#define JOY_AXIS_R			3
#define JOY_AXIS_U			4
#define JOY_AXIS_V			5
enum _ControlList
{
	AxisNada = 0, AxisForward, AxisLook, AxisSide, AxisTurn, AxisUp
};
DWORD	dwAxisFlags[JOY_MAX_AXES] =
{
	JOY_RETURNX, JOY_RETURNY, JOY_RETURNZ, JOY_RETURNR, JOY_RETURNU, JOY_RETURNV
};

//value _ControlList enum value that says what action the different joystick axes map to.
DWORD	dwAxisMap[JOY_MAX_AXES];
//either JOY_ABSOLUTE_AXIS or JOY_RELATIVE_AXIS.
DWORD	dwControlMap[JOY_MAX_AXES];
//temporary place to store the raw input values for the different joystick axes.
PDWORD	pdwRawValue[JOY_MAX_AXES];

cvar_t	*in_mouse;
cvar_t	*in_joystick;

// none of these cvars are saved over a session
// this means that advanced controller configuration needs to be executed
// each time.  this avoids any problems with getting back to a default usage
// or when changing from one controller to another.  this way at least something
// works.
cvar_t	*joy_name;
cvar_t	*joy_advanced;
cvar_t	*joy_advaxisx;
cvar_t	*joy_advaxisy;
cvar_t	*joy_advaxisz;
cvar_t	*joy_advaxisr;
cvar_t	*joy_advaxisu;
cvar_t	*joy_advaxisv;
cvar_t	*joy_forwardthreshold;
cvar_t	*joy_sidethreshold;
cvar_t	*joy_pitchthreshold;
cvar_t	*joy_yawthreshold;
cvar_t	*joy_forwardsensitivity;
cvar_t	*joy_sidesensitivity;
cvar_t	*joy_pitchsensitivity;
cvar_t	*joy_yawsensitivity;
cvar_t	*joy_upthreshold;
cvar_t	*joy_upsensitivity;
bool joy_avail, joy_advancedinit, joy_haspov;
DWORD		joy_oldbuttonstate, joy_oldpovstate;
int			joy_id;
DWORD		joy_flags;
DWORD		joy_numbuttons;
static JOYINFOEX	ji;
qboolean	in_appactive;
// forward-referenced functions
void IN_StartupJoystick (void);
void Joy_AdvancedUpdate_f (void);
void IN_JoyMove (usercmd_t *cmd);
/*
============================================================
  MOUSE CONTROL
============================================================
*/
// mouse variables
cvar_t	*m_filter;
qboolean	mlooking;

void IN_MLookDown (void) { 
    mlooking = true; 
}

void IN_MLookUp (void) {
    mlooking = false;
    if (!freelook->value && lookspring->value) {
		IN_CenterView ();
    }
}

int			mouse_buttons;
int			mouse_oldbuttonstate;
POINT		current_pos;
int			mouse_x, mouse_y, old_mouse_x, old_mouse_y, mx_accum, my_accum;
int			old_x, old_y;
qboolean	mouseactive;	// false when not focus app
qboolean	restore_spi;
qboolean	mouseinitialized;
int		originalmouseparms[3], newmouseparms[3] = {0, 0, 1};
qboolean	mouseparmsvalid;
int			window_center_x, window_center_y;
RECT		window_rect;
/*
===========
IN_ActivateMouse
Called when the window gains focus or changes in some way
===========
*/
void IN_ActivateMouse (void)
{
	int		width, height;
	if (!mouseinitialized)
		return;
	if (!in_mouse->value)
	{
		mouseactive = false;
		return;
	}
	if (mouseactive)
		return;
	mouseactive = true;

	if (mouseparmsvalid)
		restore_spi = SystemParametersInfo (SPI_SETMOUSE, 0, newmouseparms, 0);

	width = GetSystemMetrics (SM_CXSCREEN);
	height = GetSystemMetrics (SM_CYSCREEN);
	GetWindowRect ( cl_hwnd, &window_rect);
	if (window_rect.left < 0)
		window_rect.left = 0;
	if (window_rect.top < 0)
		window_rect.top = 0;
	if (window_rect.right >= width)
		window_rect.right = width-1;
	if (window_rect.bottom >= height-1)
		window_rect.bottom = height-1;

	window_center_x = (window_rect.right + window_rect.left)/2;
	window_center_y = (window_rect.top + window_rect.bottom)/2;

	SetCursorPos (window_center_x, window_center_y);

	old_x = window_center_x;
	old_y = window_center_y;
	SetCapture ( cl_hwnd );
	ClipCursor (&window_rect);
	while (ShowCursor (FALSE) >= 0)
		;
}
/*
===========
IN_DeactivateMouse
Called when the window loses focus
===========
*/
void IN_DeactivateMouse (void)
{
	if (!mouseinitialized)
		return;
	if (!mouseactive)
		return;
	if (restore_spi)
		SystemParametersInfo (SPI_SETMOUSE, 0, originalmouseparms, 0);
	mouseactive = false;
	ClipCursor (NULL);
	ReleaseCapture ();
	while (ShowCursor (TRUE) < 0)
		;
}
/*
===========
IN_StartupMouse
===========
*/
void IN_StartupMouse (void)
{
	cvar_t		*cv;
	cv = Cvar_Get ("in_initmouse", "1", CVAR_NOSET);
	if ( !cv->value ) 
		return; 
	mouseinitialized = true;
	mouseparmsvalid = SystemParametersInfo (SPI_GETMOUSE, 0, originalmouseparms, 0);
	mouse_buttons = 3;
}
/*
===========
IN_MouseEvent
===========
*/
void IN_MouseEvent (int mstate)
{
	int		i;
	if (!mouseinitialized)
		return;

// perform button actions
	for (i=0 ; i<mouse_buttons ; i++)
	{
		if ( (mstate & (1<<i)) &&
			!(mouse_oldbuttonstate & (1<<i)) )
		{
			Key_Event (K_MOUSE1 + i, true, sys_msg_time);
		}
		if ( !(mstate & (1<<i)) &&
			(mouse_oldbuttonstate & (1<<i)) )
		{
				Key_Event (K_MOUSE1 + i, false, sys_msg_time);
		}
	}	
		
	mouse_oldbuttonstate = mstate;
}

/*
===========
IN_MouseMove
===========
*/
//this function is called when the mouse is not captured, so we are unable to
//get relative movements.  Instead, we will get the absolute position of the cursor.
void IN_MouseMoveMenu() {
    //the coordinate of the mouse in the interface.
    int x, y;

    //get coordinates of the top-left of the screen.
    POINT tl;
    tl.x = 0;
    tl.y = 0;
    ClientToScreen(cl_hwnd, &tl);

    // find mouse movement
    GetCursorPos(&current_pos);

    //keep the position inside the window.
    //subtract the window origin from the cursor position.
    x = current_pos.x - tl.x;
    y = current_pos.y - tl.y;

    DKM_MousePos(x, y);       
}

void IN_MouseMove (usercmd_t *cmd)
{
    int     mx, my;
    static int zerocount = 0;

    if (!mouseactive) {
        //check if we are in the menu
        if (DKM_InMenu() == 1) {
            IN_MouseMoveMenu();
        }

        return;
    }

    // find mouse movement
    GetCursorPos (&current_pos);

    mx = current_pos.x - window_center_x;
    my = current_pos.y - window_center_y;

    //give the script editor a chance to intercept the mouse input.
    if (CIN_GCE_MouseMove(mx, my) == 1) {
        //the script editor used the input.
        SetCursorPos(window_center_x, window_center_y);
        return;
    }

    //tell the menu that the mouse is moving.
	DKM_MouseMove(mx, my);

    // hack to fix q2 mouse thread

    if( ( mx == 0 ) && ( my == 0 ) )
    {
        zerocount ++;
        if( zerocount == 1 )
        {
            mx = old_mouse_x;
            my = old_mouse_y;
        }
    }
    else
    {
        zerocount = 0;
    }

    // end hack

    if (!mx && !my)
        return;

    if (m_filter->value)
    {
        mouse_x = (mx + old_mouse_x) * 0.5;
        mouse_y = (my + old_mouse_y) * 0.5;
    }
    else
    {
        mouse_x = mx;
        mouse_y = my;
    }

    old_mouse_x = mx;
    old_mouse_y = my;

    mouse_x *= sensitivity->value;
    mouse_y *= sensitivity->value;

// add mouse X/Y movement to cmd
    if ((in_strafe.state & 1) || (lookstrafe->value && mlooking)) 
	{
        cmd->sidemove += m_side->value * mouse_x;
    }
    else 
	{
        cl.viewangles.y -= m_yaw->value * mouse_x;
    }

    if ((mlooking || freelook->value) && !(in_strafe.state & 1)) 
	{
        cl.viewangles.x += m_pitch->value * mouse_y;
    }
    else 
	{
        cmd->forwardmove -= m_forward->value * mouse_y;
    }

    // force the mouse to the center, so there's room to move
    SetCursorPos (window_center_x, window_center_y);
}

/*
=========================================================================
VIEW CENTERING
=========================================================================
*/
cvar_t	*v_centermove;
cvar_t	*v_centerspeed;
/*
===========
IN_Init
===========
*/
void IN_Init (void)
{
	// mouse variables
	m_filter				= Cvar_Get ("m_filter",					"0",		CVAR_ARCHIVE);
    in_mouse				= Cvar_Get ("in_mouse",					"1",		CVAR_ARCHIVE);
	// joystick variables
	in_joystick				= Cvar_Get ("in_joystick",				"0",		CVAR_ARCHIVE);
	joy_name				= Cvar_Get ("joy_name",					"joystick",	0);
	joy_advanced			= Cvar_Get ("joy_advanced",				"0",		CVAR_ARCHIVE);
	joy_advaxisx			= Cvar_Get ("joy_advaxisx",				"0",		CVAR_ARCHIVE);
	joy_advaxisy			= Cvar_Get ("joy_advaxisy",				"0",		CVAR_ARCHIVE);
	joy_advaxisz			= Cvar_Get ("joy_advaxisz",				"0",		CVAR_ARCHIVE);
	joy_advaxisr			= Cvar_Get ("joy_advaxisr",				"0",		CVAR_ARCHIVE);
	joy_advaxisu			= Cvar_Get ("joy_advaxisu",				"0",		CVAR_ARCHIVE);
	joy_advaxisv			= Cvar_Get ("joy_advaxisv",				"0",		CVAR_ARCHIVE);
	joy_forwardthreshold	= Cvar_Get ("joy_forwardthreshold",		"0.15",		CVAR_ARCHIVE);
	joy_sidethreshold		= Cvar_Get ("joy_sidethreshold",		"0.15",		CVAR_ARCHIVE);
	joy_upthreshold  		= Cvar_Get ("joy_upthreshold",			"0.15",		CVAR_ARCHIVE);
	joy_pitchthreshold		= Cvar_Get ("joy_pitchthreshold",		"0.15",		CVAR_ARCHIVE);
	joy_yawthreshold		= Cvar_Get ("joy_yawthreshold",			"0.15",		CVAR_ARCHIVE);
	joy_forwardsensitivity	= Cvar_Get ("joy_forwardsensitivity",	"-1",		CVAR_ARCHIVE);
	joy_sidesensitivity		= Cvar_Get ("joy_sidesensitivity",		"-1",		CVAR_ARCHIVE);
	joy_upsensitivity		= Cvar_Get ("joy_upsensitivity",		"-1",		CVAR_ARCHIVE);
	joy_pitchsensitivity	= Cvar_Get ("joy_pitchsensitivity",		"1",		CVAR_ARCHIVE);
	joy_yawsensitivity		= Cvar_Get ("joy_yawsensitivity",		"-1",		CVAR_ARCHIVE);
	// centering
	v_centermove			= Cvar_Get ("v_centermove",				"0.15",		0);
	v_centerspeed			= Cvar_Get ("v_centerspeed",			"500",		0);
	Cmd_AddCommand ("+mlook", IN_MLookDown);
	Cmd_AddCommand ("-mlook", IN_MLookUp);
	Cmd_AddCommand ("joy_advancedupdate", Joy_AdvancedUpdate_f);
	IN_StartupMouse ();
	IN_StartupJoystick ();
}
/*
===========
IN_Shutdown
===========
*/
void IN_Shutdown (void)
{
	IN_DeactivateMouse ();
}
/*
===========
IN_Activate
Called when the main window gains or loses focus.
The window may have been destroyed and recreated
between a deactivate and an activate.
===========
*/
void IN_Activate (qboolean active)
{
	in_appactive = active;
	mouseactive = !active;		// force a new window check or turn off
}
/*
==================
IN_Frame
Called every frame, even if not generating commands
==================
*/
void IN_Frame (void)
{
	if (!mouseinitialized)
		return;
	if (in_mouse == NULL || in_appactive == false )
	{
		IN_DeactivateMouse ();
		return;
	}
	if( Cvar_VariableValue( "paused" ) )
	{
		IN_DeactivateMouse ();
		ShowCursor( FALSE );
		return;
	}

	if ( !cl.refresh_prepped
		|| cls.key_dest == key_console
		|| cls.key_dest == key_menu)
	{
		// temporarily deactivate if in fullscreen
		if (Cvar_VariableValue ("vid_fullscreen") == 0)
		{
			IN_DeactivateMouse ();
			return;
		}
	}
	IN_ActivateMouse ();
}
/*
===========
IN_Move
===========
*/
void IN_Move(usercmd_t *cmd)
{
	IN_MouseMove (cmd);
	if (ActiveApp)
		IN_JoyMove (cmd);
}
/*
===================
IN_ClearStates
===================
*/
void IN_ClearStates (void)
{
	mx_accum = 0;
	my_accum = 0;
	mouse_oldbuttonstate = 0;
}
/*
=========================================================================
JOYSTICK
=========================================================================
*/
/* 
=============== 
IN_StartupJoystick 
=============== 
*/  
void IN_StartupJoystick (void) 
{ 
	int			numdevs;
	JOYCAPS		jc;
	MMRESULT	mmr;
	cvar_t		*cv;
 	// assume no joystick
	joy_avail = false; 
	// abort startup if user requests no joystick
	cv = Cvar_Get ("in_initjoy", "1", CVAR_NOSET);
	if ( !cv->value ) 
		return; 
 
	// verify joystick driver is present
	if ((numdevs = joyGetNumDevs ()) == 0)
	{
//		Com_Printf ("\njoystick not found -- driver not present\n\n");
		return;
	}
	// cycle through the joystick ids for the first valid one
	for (joy_id=0 ; joy_id<numdevs ; joy_id++)
	{
		memset (&ji, 0, sizeof(ji));
		ji.dwSize = sizeof(ji);
		ji.dwFlags = JOY_RETURNCENTERED;
		if ((mmr = joyGetPosEx (joy_id, &ji)) == JOYERR_NOERROR)
			break;
	} 
	// abort startup if we didn't find a valid joystick
	if (mmr != JOYERR_NOERROR)
	{
		Com_Printf ("\njoystick not found -- no valid joysticks (%x)\n\n", mmr);
		return;
	}
	// get the capabilities of the selected joystick
	// abort startup if command fails
	memset (&jc, 0, sizeof(jc));
	if ((mmr = joyGetDevCaps (joy_id, &jc, sizeof(jc))) != JOYERR_NOERROR)
	{
		Com_Printf ("\njoystick not found -- invalid joystick capabilities (%x)\n\n", mmr); 
		return;
	}
	// save the joystick's number of buttons and POV status
	joy_numbuttons = jc.wNumButtons;
	joy_haspov = (jc.wCaps & JOYCAPS_HASPOV) != 0;
	// old button and POV states default to no buttons pressed
	joy_oldbuttonstate = joy_oldpovstate = 0;
	// mark the joystick as available and advanced initialization not completed
	// this is needed as cvars are not available during initialization
	joy_avail = true; 
	joy_advancedinit = false;
	Com_Printf ("\njoystick detected\n\n"); 
}
/*
===========
RawValuePointer
===========
*/
PDWORD RawValuePointer (int axis)
{
	switch (axis)
	{
	case JOY_AXIS_X:
		return &ji.dwXpos;
	case JOY_AXIS_Y:
		return &ji.dwYpos;
	case JOY_AXIS_Z:
		return &ji.dwZpos;
	case JOY_AXIS_R:
		return &ji.dwRpos;
	case JOY_AXIS_U:
		return &ji.dwUpos;
	case JOY_AXIS_V:
		return &ji.dwVpos;
	}

    static unsigned long bad_axis = 0;
    return &bad_axis;
}
/*
===========
Joy_AdvancedUpdate_f
===========
*/
void Joy_AdvancedUpdate_f (void)
{
	// called once by IN_ReadJoystick and by user whenever an update is needed
	// cvars are now available
	int	i;
	DWORD dwTemp;
	// initialize all the maps
	for (i = 0; i < JOY_MAX_AXES; i++)
	{
		dwAxisMap[i] = AxisNada;
		dwControlMap[i] = JOY_ABSOLUTE_AXIS;
		pdwRawValue[i] = RawValuePointer(i);
	}
	if( joy_advanced->value == 0.0)
	{
		// default joystick initialization
		// 2 axes only with joystick control
		dwAxisMap[JOY_AXIS_X] = AxisTurn;
		// dwControlMap[JOY_AXIS_X] = JOY_ABSOLUTE_AXIS;
		dwAxisMap[JOY_AXIS_Y] = AxisForward;
		// dwControlMap[JOY_AXIS_Y] = JOY_ABSOLUTE_AXIS;
	}
	else
	{
		if (strcmp (joy_name->string, "joystick") != 0)
		{
			// notify user of advanced controller
			Com_Printf ("\n%s configured\n\n", joy_name->string);
		}
		// advanced initialization here
		// data supplied by user via joy_axisn cvars

        //Adam: god i hate documenting code i didnt write....
        //the joy_advaxis* vars contain a number, the lower 4 bits contain the _ControlList value
        //that the physical axis is mapped to.  Bit 5 is set if the joystick axis is relative.  

		dwTemp = (DWORD) joy_advaxisx->value;
		dwAxisMap[JOY_AXIS_X] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_X] = dwTemp & JOY_RELATIVE_AXIS;

		dwTemp = (DWORD) joy_advaxisy->value;
		dwAxisMap[JOY_AXIS_Y] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_Y] = dwTemp & JOY_RELATIVE_AXIS;

		dwTemp = (DWORD) joy_advaxisz->value;
		dwAxisMap[JOY_AXIS_Z] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_Z] = dwTemp & JOY_RELATIVE_AXIS;

		dwTemp = (DWORD) joy_advaxisr->value;
		dwAxisMap[JOY_AXIS_R] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_R] = dwTemp & JOY_RELATIVE_AXIS;

		dwTemp = (DWORD) joy_advaxisu->value;
		dwAxisMap[JOY_AXIS_U] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_U] = dwTemp & JOY_RELATIVE_AXIS;

		dwTemp = (DWORD) joy_advaxisv->value;
		dwAxisMap[JOY_AXIS_V] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_V] = dwTemp & JOY_RELATIVE_AXIS;
	}
	// compute the axes to collect from DirectInput
	joy_flags = JOY_RETURNCENTERED | JOY_RETURNBUTTONS | JOY_RETURNPOV;
	for (i = 0; i < JOY_MAX_AXES; i++)
	{
		if (dwAxisMap[i] != AxisNada)
		{
			joy_flags |= dwAxisFlags[i];
		}
	}
}
/*
===========
IN_Commands
===========
*/
void IN_Commands (void)
{
	int		i, key_index;
	DWORD	buttonstate, povstate;
	if (!joy_avail)
	{
		return;
	}
	
	// loop through the joystick buttons
	// key a joystick event or auxillary event for higher number buttons for each state change
	buttonstate = ji.dwButtons;
	for (i=0 ; i < joy_numbuttons ; i++)
	{
		if ( (buttonstate & (1<<i)) && !(joy_oldbuttonstate & (1<<i)) )
		{
			key_index = (i < 4) ? K_JOY1 : K_AUX1;
			Key_Event (key_index + i, true, 0);
		}
		if ( !(buttonstate & (1<<i)) && (joy_oldbuttonstate & (1<<i)) )
		{
			key_index = (i < 4) ? K_JOY1 : K_AUX1;
			Key_Event (key_index + i, false, 0);
		}
	}
	joy_oldbuttonstate = buttonstate;
	if (joy_haspov)
	{
		// convert POV information into 4 bits of state information
		// this avoids any potential problems related to moving from one
		// direction to another without going through the center position
		povstate = 0;
		if(ji.dwPOV != JOY_POVCENTERED)
		{
			if (ji.dwPOV == JOY_POVFORWARD)
				povstate |= 0x01;
			if (ji.dwPOV == JOY_POVRIGHT)
				povstate |= 0x02;
			if (ji.dwPOV == JOY_POVBACKWARD)
				povstate |= 0x04;
			if (ji.dwPOV == JOY_POVLEFT)
				povstate |= 0x08;
		}
		// determine which bits have changed and key an auxillary event for each change
		for (i=0 ; i < 4 ; i++)
		{
			if ( (povstate & (1<<i)) && !(joy_oldpovstate & (1<<i)) )
			{
				Key_Event (K_AUX29 + i, true, 0);
			}
			if ( !(povstate & (1<<i)) && (joy_oldpovstate & (1<<i)) )
			{
				Key_Event (K_AUX29 + i, false, 0);
			}
		}
		joy_oldpovstate = povstate;
	}
}
/* 
=============== 
IN_ReadJoystick
=============== 
*/  
bool IN_ReadJoystick (void)
{
	memset (&ji, 0, sizeof(ji));
	ji.dwSize = sizeof(ji);
	ji.dwFlags = joy_flags;
	if (joyGetPosEx (joy_id, &ji) == JOYERR_NOERROR)
	{
		return true;
	}
	else
	{
		// read error occurred
		// turning off the joystick seems too harsh for 1 read error,\
		// but what should be done?
		// Com_Printf ("IN_ReadJoystick: no response\n");
		// joy_avail = false;
		return false;
	}
}
/*
===========
IN_JoyMove
===========
*/
void IN_JoyMove(usercmd_t *cmd) {
    float   speed, aspeed;
    float   fAxisValue;
    int     i;
    // complete initialization if first time in
    // this is needed as cvars are not available at initialization time
    if (joy_advancedinit != true) {
        Joy_AdvancedUpdate_f();
        joy_advancedinit = true;
    }
    // verify joystick is available and that the user wants to use it
    if (!joy_avail || !in_joystick->value) {
        return; 
    }
 
    // collect the joystick data, if possible
    if (IN_ReadJoystick () != true) {
        return;
    }

    if ((in_speed.state & 1) ^ (int)cl_run->value) {
        speed = 2;
    }
    else {
        speed = 1;
    }

    aspeed = speed * cls.frametime;
    // loop through the axes
    for (i = 0; i < JOY_MAX_AXES; i++) {
        // get the floating point zero-centered, potentially-inverted data for the current axis
        fAxisValue = (float) *pdwRawValue[i];
        // move centerpoint to zero
        fAxisValue -= 32768.0;
        // convert range from -32768..32767 to -1..1 
        fAxisValue /= 32768.0;
        switch (dwAxisMap[i]) {
            case AxisForward:
                if ((joy_advanced->value == 0.0) && mlooking) 
				{
                    // user wants forward control to become look control
                    if (fabs(fAxisValue) > joy_pitchthreshold->value) 
					{
                        // if mouse invert is on, invert the joystick pitch value
                        // only absolute control support here (joy_advanced is false)
                        if (m_pitch->value < 0.0) 
						{
                            cl.viewangles.x -= (fAxisValue * joy_pitchsensitivity->value) * aspeed * cl_pitchspeed->value;
                        }
                        else 
						{
                            cl.viewangles.x += (fAxisValue * joy_pitchsensitivity->value) * aspeed * cl_pitchspeed->value;
                        }
                    }
                }
                else 
				{
                    // user wants forward control to be forward control
                    if (fabs(fAxisValue) > joy_forwardthreshold->value) 
					{
                        cmd->forwardmove += (fAxisValue * joy_forwardsensitivity->value) * speed * cl_forwardspeed->value;
                    }
                }
                break;
            case AxisSide:
                if (fabs(fAxisValue) > joy_sidethreshold->value) {
                    cmd->sidemove += (fAxisValue * joy_sidesensitivity->value) * speed * cl_sidespeed->value;
                }
                break;
            case AxisUp:
                if (fabs(fAxisValue) > joy_upthreshold->value) {
                    cmd->upmove += (fAxisValue * joy_upsensitivity->value) * speed * cl_upspeed->value;
                }
                break;
            case AxisTurn:
                if ((in_strafe.state & 1) || (lookstrafe->value && mlooking)) {
                    // user wants turn control to become side control
                    if (fabs(fAxisValue) > joy_sidethreshold->value) {
                        cmd->sidemove -= (fAxisValue * joy_sidesensitivity->value) * speed * cl_sidespeed->value;
                    }
                }
                else {
                    // user wants turn control to be turn control
                    if (fabs(fAxisValue) > joy_yawthreshold->value) 
					{
                        if(dwControlMap[i] == JOY_ABSOLUTE_AXIS) 
						{
                            cl.viewangles.y += (fAxisValue * joy_yawsensitivity->value) * aspeed * cl_yawspeed->value;
                        }
                        else 
						{
                            cl.viewangles.y += (fAxisValue * joy_yawsensitivity->value) * speed * 180.0;
                        }
                    }
                }
                break;
            case AxisLook:
                if (mlooking) {
                    if (fabs(fAxisValue) > joy_pitchthreshold->value) {
                        // pitch movement detected and pitch movement desired by user
                        if(dwControlMap[i] == JOY_ABSOLUTE_AXIS) {
                            cl.viewangles.x += (fAxisValue * joy_pitchsensitivity->value) * aspeed * cl_pitchspeed->value;
                        }
                        else {
                            cl.viewangles.x += (fAxisValue * joy_pitchsensitivity->value) * speed * 180.0;
                        }
                    }
                }
                break;
            default:
                break;
        }
    }
}