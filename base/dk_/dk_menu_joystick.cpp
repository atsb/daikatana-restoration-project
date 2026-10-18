#include "dk_shared.h"

#include "dk_misc.h"
#include "dk_buffer.h"
#include "dk_array.h"

#include "dk_menup.h"
#include "dk_menu_controls.h"

#include "keys.h"
#include "client.h"

#include "l__language.h"

///////////////////////////////////////////////////////////////////////////////////
//
//  Joystick Menu
//
///////////////////////////////////////////////////////////////////////////////////


#define JOY_HEADER_X												100
#define JOY_HEADER_Y												110
#define JOY_HEADER_Y2												126
																
#define JOY_ACTION_LEFT												100
#define JOY_ACTION_TOP												135
#define JOY_ACTION_RIGHT											340
#define JOY_ACTION_BOTTOM											233

#define joy_line_y(i)												(JOY_ACTION_TOP + ((i) + 1) * 16)
#define JOY_COL0_X													(JOY_ACTION_LEFT + 0)
#define JOY_COL1_X													((JOY_ACTION_RIGHT + JOY_ACTION_LEFT) / 2 - 20)
																	
#define JOY_SPACING													15
#define JOY_OPTS_LEFT												(JOY_HEADER_X)
#define JOY_OPTS_TOP												(joy_line_y(1))

#define JOY_USE_JOY_TOP												(JOY_OPTS_TOP + JOY_SPACING)

#define JOY_AXIS_PICKER_TOP											(joy_line_y(4))
#define JOY_AXIS_PICKER_WIDTH										(150)

#define SLIDER_SPACING												(40)
#define JOY_THRESHOLD_TOP											(joy_line_y(8))
#define JOY_SENSITIVITY_TOP											(JOY_THRESHOLD_TOP + SLIDER_SPACING)

#define JOY_NUM_COMMANDS_VISIBLE									14

#define JOY_COMMAND_LEFT											(JOY_ACTION_RIGHT + 2)
#define JOY_COMMAND_TOP												(JOY_ACTION_TOP + 16)
#define JOY_COMMAND_RIGHT											(JOY_COMMAND_LEFT + 134)
#define JOY_COMMAND_BOTTOM											(joy_line_y(JOY_NUM_COMMANDS_VISIBLE))

#define JOY_COMMAND_UP_X											(JOY_COMMAND_LEFT + 40)
#define JOY_COMMAND_UP_Y											(JOY_COMMAND_BOTTOM + 10)
#define JOY_COMMAND_DOWN_X											(JOY_COMMAND_UP_X + 30)
#define JOY_COMMAND_DOWN_Y											(JOY_COMMAND_UP_Y)

#define sign(val) (((val) < 0) ? -1.0f : 1.0f)



//((sel >= K_MOUSE1) && (sel <= K_MOUSE3))//
#define SELECTION_VALID(sel)		((sel >= K_JOY1) && (sel <= K_AUX32))


CMenuSubJoystick::CMenuSubJoystick() 
{
	command_box.Init(JOY_COMMAND_LEFT, JOY_COMMAND_TOP - 2, JOY_COMMAND_RIGHT, JOY_COMMAND_BOTTOM + 2, 2);
    joy_binding = tongue_menu[T_MENU_NONE_STRING];
	num_command_items = 0;
	num_command_items = KCC_NUM_COMMANDS;
	in_initjoy = Cvar_Get("in_initjoy","0",CVAR_NOSET);
	in_joystick = Cvar_Get("in_joystick","0",CVAR_ARCHIVE);

    pick_axis.AddString(tongue_menu[T_MENU_JOY_FORWARD]);//"Forward");
    pick_axis.AddString(tongue_menu[T_MENU_JOY_SIDE]);//"Side");
    pick_axis.AddString(tongue_menu[T_MENU_JOY_UP]);//"Up");
    pick_axis.AddString(tongue_menu[T_MENU_JOY_PITCH]);//"Pitch");
    pick_axis.AddString(tongue_menu[T_MENU_JOY_YAW]);//"Yaw");

	joy_forwardthreshold	= Cvar_Get ("joy_forwardthreshold",		"0.15",		0);
	joy_forwardsensitivity	= Cvar_Get ("joy_forwardsensitivity",	"-1",		0);

	joy_sidethreshold		= Cvar_Get ("joy_sidethreshold",		"0.15",		0);
	joy_sidesensitivity		= Cvar_Get ("joy_sidesensitivity",		"-1",		0);

	joy_upthreshold  		= Cvar_Get ("joy_upthreshold",			"0.15",		0);
	joy_upsensitivity		= Cvar_Get ("joy_upsensitivity",		"-1",		0);

	joy_pitchthreshold		= Cvar_Get ("joy_pitchthreshold",		"0.15",		0);
	joy_pitchsensitivity	= Cvar_Get ("joy_pitchsensitivity",		"1",		0);

	joy_yawthreshold		= Cvar_Get ("joy_yawthreshold",			"0.15",		0);
	joy_yawsensitivity		= Cvar_Get ("joy_yawsensitivity",		"-1",		0);
}

void CMenuSubJoystick::PositionCommandHilight(int32 index)
{
    if (index < 0 || index >= JOY_NUM_COMMANDS_VISIBLE) 
      return;
		 
    command_hilight_box.Init(JOY_COMMAND_LEFT + 2, joy_line_y(index), JOY_COMMAND_RIGHT - 2, joy_line_y(index + 1), 2);
}

void CMenuSubJoystick::SetSliderVals()
{
	int32 picked = pick_axis.CurrentIndex();
	switch(picked)
	{
	case 0:		// forward
		threshold_slider.Position(joy_forwardthreshold->value);
		sensitivity_slider.Position(joy_forwardsensitivity->value);
		break;

	case 1:		// side
		threshold_slider.Position(joy_sidethreshold->value);
		sensitivity_slider.Position(joy_sidesensitivity->value);
		break;

	case 2:		// up
		threshold_slider.Position(joy_upthreshold->value);
		sensitivity_slider.Position(joy_upsensitivity->value);
		break;

	case 3:		// pitch
		threshold_slider.Position(joy_pitchthreshold->value);
		sensitivity_slider.Position(joy_pitchsensitivity->value);
		break;

	case 4:		// yaw
		threshold_slider.Position(joy_yawthreshold->value);
		sensitivity_slider.Position(joy_yawsensitivity->value);
		break;

	default:
		break;
	}

}

void CMenuSubJoystick::SetThreshold()
{
	float value = threshold_slider.Position();
	int32 picked = pick_axis.CurrentIndex();
	switch(picked)
	{
	case 0:		// forward
		Cvar_SetValue("joy_forwardthreshold",value);
//		joy_forwardthreshold->value = value;
		break;

	case 1:		// side
		Cvar_SetValue("joy_sidethreshold",value);
//		joy_sidethreshold->value = value;
		break;

	case 2:		// up
		Cvar_SetValue("joy_upthreshold",value);
//		joy_upthreshold->value = value;
		break;

	case 3:		// pitch
		Cvar_SetValue("joy_pitchthreshold",value);
//		joy_pitchthreshold->value = value;
		break;

	case 4:		// yaw
		Cvar_SetValue("joy_yawthreshold",value);
//		joy_yawthreshold->value = value;
		break;

	default:
		break;
	}
}

void CMenuSubJoystick::SetSensitivity()
{
	float value = sensitivity_slider.Position();
	int32 picked = pick_axis.CurrentIndex();
	switch(picked)
	{
	case 0:		// forward
		Cvar_SetValue("joy_forwardsensitivity",value);
//		joy_forwardsensitivity->value = value;// * sign(joy_forwardsensitivity->value);
		break;

	case 1:		// side
		Cvar_SetValue("joy_sidesensitivity",value);
//		joy_sidesensitivity->value = value;// * sign(joy_sidesensitivity->value);
		break;

	case 2:		// up
		Cvar_SetValue("joy_upsensitivity",value);
//		joy_upsensitivity->value = value;// * sign(joy_upsensitivity->value);
		break;

	case 3:		// pitch
		Cvar_SetValue("joy_pitchsensitivity",value);
//		joy_pitchsensitivity->value = value;// * sign(joy_pitchsensitivity->value);
		break;

	case 4:		// yaw
		Cvar_SetValue("joy_yawsensitivity",value);
//		joy_yawsensitivity->value = value;// * sign(joy_yawsensitivity->value);
		break;

	default:
		break;
	}

}

void CMenuSubJoystick::Enter() 
{
    //place the command box up and down buttons.
    command_up.Init(JOY_COMMAND_UP_X, JOY_COMMAND_UP_Y, menu_font, menu_font_bright, button_font);
    command_up.InitGraphics(TOENAIL_UP_OFF_STRING, TOENAIL_UP_ON_STRING, TOENAIL_UP_DISABLE_STRING);
	command_up.SetText(ARROW_UP_STRING, true);

    command_down.Init(JOY_COMMAND_DOWN_X, JOY_COMMAND_DOWN_Y, menu_font, menu_font_bright, button_font);
    command_down.InitGraphics(TOENAIL_DOWN_OFF_STRING, TOENAIL_DOWN_ON_STRING, TOENAIL_DOWN_DISABLE_STRING);
	command_down.SetText(ARROW_DOWN_STRING, true);
	selection = -1;
	command_hilight = -1;

    use_joystick.Init(JOY_OPTS_LEFT, JOY_USE_JOY_TOP, menu_font, menu_font_bright, tongue_menu[T_MENU_JOY_USE]);
	pick_axis.Init(JOY_OPTS_LEFT,JOY_AXIS_PICKER_TOP, JOY_AXIS_PICKER_WIDTH, menu_font, menu_font_bright, button_font, tongue_menu[T_MENU_JOY_CONFIGURE_AXIS]);

    threshold_slider.Init(JOY_OPTS_LEFT, JOY_THRESHOLD_TOP, button_font, SLIDER_BACKGROUND_CHAR, SLIDER_TAB_CHAR);
    //sensitivityX.InitExp(42, -2, 1);
    threshold_slider.InitLinear(42, 0.0f, 1.0f);  // try linear scale instead of exponential
 
    // Y Axis
    sensitivity_slider.Init(JOY_OPTS_LEFT, JOY_SENSITIVITY_TOP, button_font, SLIDER_BACKGROUND_CHAR, SLIDER_TAB_CHAR);
    //sensitivityY.InitExp(42, -2, 1);
    sensitivity_slider.InitLinear(42, -2.0f, 2.0f);  // try linear scale instead of exponential

	SetSliderVals();
}

void CMenuSubJoystick::TopIndex(int32 index)
{
	if (!SELECTION_VALID(selection)) return;
    //assume the whole command list isn't even showing for now.
    command_up.Enable(false);
    command_down.Enable(false);
    top_index = 0;


    //assume we were passed a valid index.
    top_index = index;

    //assume we see the down button for now.
    command_down.Enable(true);

    //make sure we arent scrolling down too far.
    if (top_index >= num_command_items - JOY_NUM_COMMANDS_VISIBLE)
    {
        top_index  = num_command_items - JOY_NUM_COMMANDS_VISIBLE;

        //we are at the bottom of the list, down show the down button.
        command_down.Enable(false);
    }

    bound_min(top_index, 0);

    //if the top index is > 0, we should show the up button.
    if (top_index > 0) 
	{
        command_up.Enable(true);
    }
    else 
	{
        command_up.Enable(false);
    }
}

void CMenuSubJoystick::SelectCommand(int32 index)
{
    if (!SELECTION_VALID(selection))
      return;

	if ((index >= -1) && (index < KCC_NUM_COMMANDS))//( !( (index < -1) || (index >= KCC_NUM_COMMANDS) || (selection < MA_BUTTON1) || (selection >= MA_NUM_ACTIONS) ) )
	{
		if (index == -1)
		{
			S_StartLocalSound(DKM_sounds[DKMS_NO]);
			Key_SetBinding(selection, "");
		}
		else
		{
			Key_SetBinding(selection, CommandText((key_config_command)index));
		}
	}
}

int32 CMenuSubJoystick::HitCommand(int32 norm_x, int32 norm_y) 
{
    //check if it is the box at all.
    if (norm_x < JOY_COMMAND_LEFT || norm_x > JOY_COMMAND_RIGHT || 
        norm_y < joy_line_y(0) || norm_y > joy_line_y(JOY_NUM_COMMANDS_VISIBLE)) 
    {
        //hit nothing.
        return -1;
    }

    //get the percentage down we are at.
    float percent = float(norm_y - joy_line_y(0)) / float(joy_line_y(JOY_NUM_COMMANDS_VISIBLE) - joy_line_y(0));

    //get the index of the command we selected.
    int32 index = int32(percent * JOY_NUM_COMMANDS_VISIBLE);

    //make sure its a valid index.
    if (index < 0 || index + top_index >= num_command_items)
    {
        //not a valid index.
        return -1;
    }

    //return the index we clicked
    return index + top_index;
}

const char *CMenuSubJoystick::FindBinding()
{
	if (SELECTION_VALID(selection))
	{
		if (keybindings[selection] == NULL || keybindings[selection][0] == '\0')
			return tongue_menu[T_MENU_NONE_STRING];

		return FindCommandName(keybindings[selection]);
	}
	return NULL;
}

void CMenuSubJoystick::Animate(int32 elapsed_time) 
{
    threshold_slider.Animate(elapsed_time);
    sensitivity_slider.Animate(elapsed_time);

	if (SELECTION_VALID(selection))
	{
		command_box.Animate(elapsed_time);
		command_up.Animate(elapsed_time);
		command_down.Animate(elapsed_time);
	}

    if (command_list_delay.On()) 
	{
	    int32 current_time = GetTickCount();
        //check if time is up.
        if (command_list_delay.TimeUp(current_time)) 
		{
            //hilight the command.
            command_hilight = command_list_delay.Row();
        }
    }
}

void CMenuSubJoystick::DrawBackground() 
{
    threshold_slider.DrawBackground();
    sensitivity_slider.DrawBackground();

	use_joystick.DrawBackground();
	pick_axis.DrawBackground();
	if (SELECTION_VALID(selection))
	{
		command_box.DrawBackground();
		command_up.DrawBackground();
		command_down.DrawBackground();
	}
}

void CMenuSubJoystick::DrawForeground() 
{
	// HEADER
	DKM_DrawString(JOY_HEADER_X, JOY_HEADER_Y, tongue_menu[T_MENU_JOY_HEADER], NULL, false, true); // highlighted

	// SLIDERS
	buffer32 buf1("%s %4.3f",tongue_menu[T_MENU_JOY_THRESHOLD], threshold_slider.Position());
	DKM_DrawString(JOY_OPTS_LEFT, JOY_THRESHOLD_TOP - 16, buf1, NULL, false, false);
    threshold_slider.DrawForeground();

	buf1.Set("%s %3.2f",tongue_menu[T_MENU_JOY_SENSITIVITY], sensitivity_slider.Position());
    DKM_DrawString(JOY_OPTS_LEFT, JOY_SENSITIVITY_TOP - 16, buf1, NULL, false, false);
    sensitivity_slider.DrawForeground();

	bool usej =  ((in_initjoy->value > 0) && (in_joystick->value > 0) ) ? 1 : 0;
	use_joystick.Check(usej);
	use_joystick.DrawForeground();
	pick_axis.DrawForeground();

	if (SELECTION_VALID(selection))
	{
		buffer32 buf("Joystick %d",selection - K_JOY1 + 1);
		DKM_DrawString(JOY_COL0_X, joy_line_y(0), buf, NULL, false, false);
		DKM_DrawString(JOY_COL1_X, joy_line_y(0), FindBinding(), NULL, false, false);
		command_box.DrawForeground();
		command_up.DrawForeground();
		command_down.DrawForeground();

        for (int32 i = 0; i < JOY_NUM_COMMANDS_VISIBLE; i++)
        {
            //get the index of the command we will display.
            int32 index = i + top_index;
            if (index >= KCC_NUM_COMMANDS) 
				break;

            //decide if this item should be drawn bright.
            bool draw_bright = (command_list_delay.Row() == i);

            //print the string.
            DKM_DrawString(JOY_COMMAND_LEFT + 5, joy_line_y(i), CommandName((key_config_command)index), NULL, false, draw_bright);
        }
		DKM_DrawString(JOY_HEADER_X, JOY_HEADER_Y2, tongue_menu[T_MENU_JOY_HEADER2], NULL, false, true); // highlighted
	}
	else if (usej)
	{
		DKM_DrawString(JOY_COL0_X, joy_line_y(0), "--", NULL, false, false);
		DKM_DrawString(JOY_COL1_X, joy_line_y(0), "--", NULL, false, false);
		DKM_DrawString(JOY_HEADER_X, JOY_HEADER_Y2, tongue_menu[T_MENU_JOY_HEADER1], NULL, false, true); // highlighted
	}
	else
	{
	}
}

void CMenuSubJoystick::PlaceEntities() 
{
    threshold_slider.PlaceEntities();
    sensitivity_slider.PlaceEntities();

	use_joystick.PlaceEntities();
	pick_axis.PlaceEntities();
	if (SELECTION_VALID(selection))
	{
		command_box.PlaceEntities();
		command_up.PlaceEntities();
		command_down.PlaceEntities();
	}

    //check if have a hilight in the command box.
    if (command_hilight != -1)
    {
        PositionCommandHilight(command_hilight);
        command_hilight_box.PlaceEntities();
    }
}

bool CMenuSubJoystick::Keydown(int32 key) 
{
	if (SELECTION_VALID(key))
	{
		selection = key;
		num_command_items = KCC_NUM_COMMANDS;
		return true;
	}

	if (SELECTION_VALID(selection))
	{
		switch(key)
		{
		case K_DEL:
            SelectCommand(-1);
            return true;

        case K_ESCAPE:
			selection = -1;
			command_hilight = -1;
            command_list_delay.Start(-1);
            command_list_delay.Abort();

		case K_MWHEELUP:
		case K_UPARROW:
            TopIndex(top_index - 1);
			return true;

		case K_MWHEELDOWN:
		case K_DOWNARROW:
            TopIndex(top_index + 1);
			return true;
		};
	}

    switch (key) 
	{
        case K_ESCAPE:
			selection = -1;
			//leave the menu.
			//DKM_EnterMain();
			if( Cvar_VariableValue( "console" ) == 0 )
			{
				return false;
			}

			DKM_Shutdown();       // exit entire interface  1.10 dsn
			return true;

    }

    return false;
}

bool CMenuSubJoystick::MousePos(int32 norm_x, int32 norm_y) 
{
    if (threshold_slider.MousePos(norm_x,norm_y)) return true;
    if (sensitivity_slider.MousePos(norm_x,norm_y)) return true;

	if (use_joystick.MousePos(norm_x,norm_y)) return true;
	if (pick_axis.MousePos(norm_x,norm_y)) return true;

	if (SELECTION_VALID(selection))
	{
		if (command_up.MousePos(norm_x,norm_y)) return true;
		if (command_down.MousePos(norm_x,norm_y)) return true;

        int32 command = HitCommand(norm_x, norm_y);

        //set the command hilight if the cursor was on a command.
        if (command != -1) 
		{
            if (command_hilight == -1) 
			{
                //hilight now.
                command_hilight = command - top_index;
            }
            else 
			{
                //hilight after a delay.
                command_list_delay.Start(command - top_index);
            }
        }
        else 
		{
            //tell the delay that there is nothing under the mouse.
            command_list_delay.Start(-1);

            //abort the command list delay, in case it is on.
            command_list_delay.Abort();
        }
	}

    return false;
}

bool CMenuSubJoystick::MouseDown(int32 norm_x, int32 norm_y) 
{
    if (threshold_slider.MouseDown(norm_x,norm_y)) return true;
    if (sensitivity_slider.MouseDown(norm_x,norm_y)) return true;

	if (use_joystick.MouseDown(norm_x,norm_y)) return true;
	if (pick_axis.MouseDown(norm_x,norm_y)) return true;
	if (SELECTION_VALID(selection))
	{
		if (command_up.MouseDown(norm_x,norm_y)) return true;
		if (command_down.MouseDown(norm_x,norm_y)) return true;
	}

    return false;
}

bool CMenuSubJoystick::MouseUp(int32 norm_x, int32 norm_y) 
{
    if (threshold_slider.MouseUp(norm_x,norm_y))
	{
		SetThreshold();
		return true;
	}

    if (sensitivity_slider.MouseUp(norm_x,norm_y))
	{
		SetSensitivity();
		return true;
	}

	if (use_joystick.MouseUp(norm_x,norm_y))
	{
		bool usej =  ((in_initjoy->value > 0) && (in_joystick->value > 0) ) ? 1 : 0;
		if (usej)
		{
			Cvar_SetValue("in_joystick",0.0f);
//			in_initjoy->value = 0.0f;
//			in_joystick->value = 0.0f;

			// stop selecting stuff.
			selection = -1;
			command_hilight = -1;
            command_list_delay.Start(-1);
            command_list_delay.Abort();
		}
		else
		{
			Cvar_SetValue("in_joystick",1.0f);
			Cvar_SetValue("in_initjoy",1.0f);
//			in_initjoy->value = 1.0f;
//			in_joystick->value = 1.0f;
		}
		
		return true;
	}

	if (pick_axis.MouseUp(norm_x,norm_y))
	{
		SetSliderVals();
		return true;
	}

	if (SELECTION_VALID(selection))
	{
		if (command_up.MouseUp(norm_x,norm_y))
		{
            TopIndex(top_index - 1);
			return true;
		}

		if (command_down.MouseUp(norm_x,norm_y))
		{
            TopIndex(top_index + 1);
			return true;
		}

        int32 command = HitCommand(norm_x, norm_y);

        //set the command hilight if the cursor was on a command.
        if (command != -1) 
		{
			SelectCommand(command);
            if (command_hilight == -1) 
			{
                //hilight now.
                command_hilight = command - top_index;
            }

            else 
			{
                //hilight after a delay.
                command_list_delay.Start(command - top_index);
            }
        }
        else 
		{
            //tell the delay that there is nothing under the mouse.
            command_list_delay.Start(-1);

            //abort the command list delay, in case it is on.
            command_list_delay.Abort();
        }
	}

    return false;
}






