
#include "dk_shared.h"

#include "dk_misc.h"
#include "dk_buffer.h"
#include "dk_array.h"

#include "dk_menup.h"
#include "dk_menu_controls.h"

#include "client.h"
#include "keys.h"

#include "l__language.h"


///////////////////////////////////////////////////////////////////////////////////
//
//  Mouse Menu
//
///////////////////////////////////////////////////////////////////////////////////

#define MOUSE_HEADER_X											100
#define MOUSE_HEADER_Y											110

#define MOUSE_ACTION_LEFT										100
#define MOUSE_ACTION_TOP										115
#define MOUSE_ACTION_RIGHT										340
#define MOUSE_ACTION_BOTTOM										233

#define mouse_line_y(i)											(MOUSE_ACTION_TOP + ((i) + 1) * 16)
#define MOUSE_COL0_X											(MOUSE_ACTION_LEFT + 0)
#define MOUSE_COL1_X											((MOUSE_ACTION_RIGHT + MOUSE_ACTION_LEFT) / 2 - 20)

#define MOUSE_SPACING											20
#define MOUSE_REVERSE_Y											(mouse_line_y(MA_NUM_ACTIONS) + 5)
#define MOUSE_MOUSELOOK_Y										(MOUSE_REVERSE_Y + MOUSE_SPACING)
#define MOUSE_LOOKSTRAFE_Y										(MOUSE_MOUSELOOK_Y + MOUSE_SPACING)
#define MOUSE_FILTER_Y											(MOUSE_LOOKSTRAFE_Y + MOUSE_SPACING)

#define SLIDER_SPACING											(40)
#define MOUSE_SENSITIVITY_X										(MOUSE_ACTION_LEFT)
#define MOUSE_SENSITIVITY_X_Y									(MOUSE_FILTER_Y + SLIDER_SPACING)//322
#define MOUSE_SENSITIVITY_Y_Y									(MOUSE_SENSITIVITY_X_Y + SLIDER_SPACING)//362
#define MOUSE_SENSITIVITY_W										(235)
#define MOUSE_SPEED_TOP											(MOUSE_SENSITIVITY_Y_Y + SLIDER_SPACING)

#define MOUSE_NUM_COMMANDS_VISIBLE								14 // 16

//#define MOUSE_COMMAND_LEFT (MOUSE_ACTION_RIGHT + 5)
#define MOUSE_COMMAND_LEFT										(MOUSE_ACTION_RIGHT + 2)
#define MOUSE_COMMAND_TOP										(MOUSE_ACTION_TOP + 16)
#define MOUSE_COMMAND_RIGHT										(MOUSE_COMMAND_LEFT + 134) // 140
#define MOUSE_COMMAND_BOTTOM									(mouse_line_y(MOUSE_NUM_COMMANDS_VISIBLE))

#define MOUSE_COMMAND_UP_X										(MOUSE_COMMAND_LEFT + 40)
#define MOUSE_COMMAND_UP_Y										(MOUSE_COMMAND_BOTTOM + 10)
#define MOUSE_COMMAND_DOWN_X									(MOUSE_COMMAND_UP_X + 30)
#define MOUSE_COMMAND_DOWN_Y									(MOUSE_COMMAND_UP_Y)

#define sign(val) (((val) < 0) ? -1.0f : 1.0f)

// global vars

extern cvar_t *lookstrafe;
extern cvar_t *freelook;
extern cvar_t *m_pitch;

#define MOUSE_FILTER_CVAR										"m_filter"

/*
typedef enum
{
	MA_BUTTON1,
	MA_BUTTON2,
	MA_BUTTON3,
	MA_MWHEELUP,
	MA_MWHEELDN,
	
	MA_NUM_ACTIONS,
	MA_NONE,
} mouse_action;
*/
static int32 mouse_button_list[MA_NUM_ACTIONS] = 
{
	K_MOUSE1,
	K_MOUSE2,
	K_MOUSE3,
	K_MWHEELUP,
	K_MWHEELDOWN,
};

static char *mouse_string_list[MA_NUM_ACTIONS] = 
{
	tongue_menu[T_MENU_MOUSE_BUTTON1],
	tongue_menu[T_MENU_MOUSE_BUTTON2],
	tongue_menu[T_MENU_MOUSE_BUTTON3],
	tongue_menu[T_MENU_WHEEL_UP],
	tongue_menu[T_MENU_WHEEL_DN],
};



CMenuSubMouse::CMenuSubMouse()
{
  //place the border objects.
	command_box.Init(MOUSE_COMMAND_LEFT, MOUSE_COMMAND_TOP - 2, MOUSE_COMMAND_RIGHT, MOUSE_COMMAND_BOTTOM + 2, 2);

  //for (int32 i = 0; i < 5; i++)
  for (int32 i = 0; i < MA_NUM_ACTIONS; i++)
	  mouse_bindings[i] = tongue_menu[T_MENU_NONE_STRING];

  action_hilight = MA_NONE;
  selection = MA_NONE;

  num_command_items = 0;

  TopIndex(0);
}



void CMenuSubMouse::FindBindings()
{
	for (int32 i = 0; i < MA_NUM_ACTIONS; i++)
    {
		int32 key = mouse_button_list[i];

		//check if anything is set to this mouse button.
		if (keybindings[key] == NULL || keybindings[key][0] == '\0')
		{
			mouse_bindings[MA_BUTTON1 + i] = tongue_menu[T_MENU_NONE_STRING];
			continue;
		}

        mouse_bindings[MA_BUTTON1 + i] = FindCommandName(keybindings[key]);
    }

}

void CMenuSubMouse::SelectAction(mouse_action action)
{
    selection = action;
    num_command_items = 0;
    command_hilight = -1;

    if (selection == MA_NONE) 
      return;

    num_command_items = KCC_NUM_COMMANDS;
}


void CMenuSubMouse::SelectCommand(int32 index)
{
    if (selection == MA_NONE) 
      return;

	if ( !( (index < -1) || (index >= KCC_NUM_COMMANDS) || (selection < MA_BUTTON1) || (selection >= MA_NUM_ACTIONS) ) )
	{
		if (index == -1)
		{
			S_StartLocalSound(DKM_sounds[DKMS_NO]);
			Key_SetBinding(mouse_button_list[selection], "");
		}
		else
		{
			Key_SetBinding(mouse_button_list[selection], CommandText((key_config_command)index));
		}
	}
    //update our info on the bindings that are set.
    FindBindings();

    SelectAction(selection);
}


void CMenuSubMouse::TopIndex(int32 index)
{
    //assume the whole command list isn't even showing for now.
    command_up.Enable(false);
    command_down.Enable(false);
    top_index = 0;

    if (selection == MA_NONE) return;

    //assume we were passed a valid index.
    top_index = index;

    //assume we see the down button for now.
    command_down.Enable(true);

    //make sure we arent scrolling down too far.
    if (top_index >= num_command_items - MOUSE_NUM_COMMANDS_VISIBLE)
    {
        top_index  = num_command_items - MOUSE_NUM_COMMANDS_VISIBLE;

        //we are at the bottom of the list, down show the down button.
        command_down.Enable(false);
    }

    bound_min(top_index, 0);

    //if the top index is > 0, we should show the up button.
    if (top_index > 0) {
        command_up.Enable(true);
    }
    else {
        command_up.Enable(false);
    }
}


mouse_action CMenuSubMouse::HitAction(int32 norm_x, int32 norm_y)
{
    //check if it is on any of them.
    if (norm_x < MOUSE_COL0_X || norm_x > MOUSE_ACTION_RIGHT - 5 || 
        norm_y < mouse_line_y(0) || norm_y > mouse_line_y(MA_NUM_ACTIONS)) {
        //we're not even on the box
        return MA_NONE;
    }

    //it is in one of them.  get the percentage down we are.
    float percent = float(norm_y - mouse_line_y(0)) / float(mouse_line_y(MA_NUM_ACTIONS) - mouse_line_y(0));

    int32 index = int32(percent * MA_NUM_ACTIONS);

    bound_min(index, 0);
//    bound_max(index, MA_BUTTON3);
    bound_max(index, MA_NUM_ACTIONS);

    return (mouse_action)index;
}

//returns the line in the command list under the given coordinate, or -1 if the coordinate is off the list.
int32 CMenuSubMouse::HitCommand(int32 norm_x, int32 norm_y) {
    //check if it is the box at all.
    if (norm_x < MOUSE_COMMAND_LEFT || norm_x > MOUSE_COMMAND_RIGHT || 
        norm_y < mouse_line_y(0) || norm_y > mouse_line_y(MOUSE_NUM_COMMANDS_VISIBLE)) 
    {
        //hit nothing.
        return -1;
    }

    //get the percentage down we are at.
    float percent = float(norm_y - mouse_line_y(0)) / float(mouse_line_y(MOUSE_NUM_COMMANDS_VISIBLE) - mouse_line_y(0));

    //get the index of the command we selected.
    int32 index = int32(percent * MOUSE_NUM_COMMANDS_VISIBLE);

    //make sure its a valid index.
    if (index < 0 || index + top_index >= num_command_items)
    {
        //not a valid index.
        return -1;
    }

    //return the index we clicked
    return index + top_index;
}

void CMenuSubMouse::PositionActionHilight(int32 index)
{
    if (index < 0 || index >= MA_NUM_ACTIONS) return;

    //action_hilight_box.Init(MOUSE_COL1_X - 3, mouse_line_y(index) - 2, MOUSE_ACTION_RIGHT - 6, mouse_line_y(index + 1), 2);
		action_hilight_box.Init(MOUSE_COL1_X - 2, mouse_line_y(index),  MOUSE_ACTION_RIGHT , mouse_line_y(index + 1), 2);
}


void CMenuSubMouse::PositionCommandHilight(int32 index)
{
    if (index < 0 || index >= MOUSE_NUM_COMMANDS_VISIBLE) 
      return;
		 
    command_hilight_box.Init(MOUSE_COMMAND_LEFT + 2, mouse_line_y(index), MOUSE_COMMAND_RIGHT - 2, mouse_line_y(index + 1), 2);
}


void CMenuSubMouse::Enter()
{
    //set up the slider controls.
    
    // X Axis
    sensitivityX.Init(MOUSE_SENSITIVITY_X, MOUSE_SENSITIVITY_X_Y, button_font, SLIDER_BACKGROUND_CHAR, SLIDER_TAB_CHAR);
    //sensitivityX.InitExp(42, -2, 1);
    sensitivityX.InitLinear(42, 0.001f, 0.07f);  // try linear scale instead of exponential
 
    // Y Axis
    sensitivityY.Init(MOUSE_SENSITIVITY_X, MOUSE_SENSITIVITY_Y_Y, button_font, SLIDER_BACKGROUND_CHAR, SLIDER_TAB_CHAR);
    //sensitivityY.InitExp(42, -2, 1);
    sensitivityY.InitLinear(42, 0.001f, 0.07f);  // try linear scale instead of exponential

    // speed
    speed.Init(MOUSE_SENSITIVITY_X, MOUSE_SPEED_TOP, button_font, SLIDER_BACKGROUND_CHAR, SLIDER_TAB_CHAR);
    speed.InitLinear(19, 1.0f, 20.0f);  

    // set sensitivity slider positions
    if (lookstrafe->value != 0)
      sensitivityX.Position(fabs(m_side->value));  //they are strafing.
    else 
      sensitivityX.Position(fabs(m_yaw->value));   //they are turning 
 
    if (freelook->value != 0)
      sensitivityY.Position(fabs(m_pitch->value));    //they are looking
    else
      sensitivityY.Position(fabs(m_forward->value));  //they are moving

	speed.Position(Cvar_VariableValue("sensitivity"));

    //place the command box up and down buttons.
    command_up.Init(MOUSE_COMMAND_UP_X, MOUSE_COMMAND_UP_Y, menu_font, menu_font_bright, button_font);
    command_up.InitGraphics(TOENAIL_UP_OFF_STRING, TOENAIL_UP_ON_STRING, TOENAIL_UP_DISABLE_STRING);
	command_up.SetText(ARROW_UP_STRING, true);

    command_down.Init(MOUSE_COMMAND_DOWN_X, MOUSE_COMMAND_DOWN_Y, menu_font, menu_font_bright, button_font);
    command_down.InitGraphics(TOENAIL_DOWN_OFF_STRING, TOENAIL_DOWN_ON_STRING, TOENAIL_DOWN_DISABLE_STRING);
	command_down.SetText(ARROW_DOWN_STRING, true);

    //set up the reverse axis radio button.
	  reverse_axis.Init(MOUSE_ACTION_LEFT, MOUSE_REVERSE_Y, menu_font, menu_font_bright, tongue_menu[T_MENU_MOUSE_REVERSEAXIS]);
    mouse_look.Init(MOUSE_ACTION_LEFT, MOUSE_MOUSELOOK_Y, menu_font, menu_font_bright, tongue_menu[T_MENU_MOUSE_MOUSELOOK]);
    look_strafe.Init(MOUSE_ACTION_LEFT, MOUSE_LOOKSTRAFE_Y, menu_font, menu_font_bright, tongue_menu[T_MENU_MOUSE_LOOKSTRAFE]);

    mouse_filter.Init(MOUSE_ACTION_LEFT, MOUSE_FILTER_Y, menu_font, menu_font_bright, tongue_menu[T_MENU_MOUSE_SMOOTH]);

    action_hilight = MA_NONE;
    selection = MA_NONE;

    FindBindings();
}


void CMenuSubMouse::Animate(int32 elapsed_time)
{
    sensitivityX.Animate(elapsed_time);
    sensitivityY.Animate(elapsed_time);
    speed.Animate(elapsed_time);

    command_up.Animate(elapsed_time);
    command_down.Animate(elapsed_time);

    //get the current time.
    int32 current_time = GetTickCount();

    //check if our hilight delay timers are running.
    if (action_list_delay.On() == true) {
        //check if the time is up.
        if (action_list_delay.TimeUp(current_time) == true) {
            //hilight the action.
            action_hilight = (mouse_action)action_list_delay.Row();
        }
    }

    if (command_list_delay.On() == true) {
        //check if time is up.
        if (command_list_delay.TimeUp(current_time) == true) {
            //hilight the command.
            command_hilight = command_list_delay.Row();
        }
    }
}

void CMenuSubMouse::DrawForeground()
{
	bool draw_bright;
	for (int i = 0; i < MA_NUM_ACTIONS; i++)
	{
		DKM_DrawString(MOUSE_COL0_X, mouse_line_y(i), mouse_string_list[i], NULL, false, false);
		draw_bright = (action_list_delay.Row() == i);
		DKM_DrawString(MOUSE_COL1_X, mouse_line_y(i), mouse_bindings[MA_BUTTON1 + i], NULL, false, draw_bright);
	}

    // reverse y-axis button
    reverse_axis.Check(m_pitch->value < 0);
    reverse_axis.DrawForeground();

    // mouselook button
    mouse_look.Check((freelook->value) ? 1 : 0);
    mouse_look.DrawForeground();

    // lookstrafe button
    look_strafe.Check((lookstrafe->value) ? 1 : 0); 
    look_strafe.DrawForeground();

	mouse_filter.DrawForeground();

    // sensitivty sliders
    sensitivityX.DrawForeground();
    sensitivityY.DrawForeground();
    speed.DrawForeground();
          
	buffer32 buf("%s - %4.3f",tongue_menu[T_MENU_MOUSE_SENSITIVITY_X], sensitivityX.Position());
	DKM_DrawString(MOUSE_SENSITIVITY_X, MOUSE_SENSITIVITY_X_Y - 16, buf, NULL, false, false);

	buf.Set("%s - %4.3f",tongue_menu[T_MENU_MOUSE_SENSITIVITY_Y], sensitivityY.Position());
	DKM_DrawString(MOUSE_SENSITIVITY_X, MOUSE_SENSITIVITY_Y_Y - 16, buf, NULL, false, false);

	buf.Set("%s - %d",tongue_menu[T_MENU_MOUSE_SPEED], (int)speed.Position());
	DKM_DrawString(MOUSE_SENSITIVITY_X, MOUSE_SPEED_TOP - 16, buf, NULL, false, false);

    //check if we have a selection
    if (selection != MA_NONE)
    {
        //we have an action selected.
        
		//DKM_DrawString(MOUSE_ACTION_LEFT, MOUSE_ACTION_TOP - 10, "Select a command, delete to clear.", NULL, false, false);
		DKM_DrawString(MOUSE_HEADER_X, MOUSE_HEADER_Y, tongue_menu[T_MENU_MOUSE_SELECT_ACTION], NULL, false, true);

        //draw as many commands as will fit.
        for (int32 i = 0; i < MOUSE_NUM_COMMANDS_VISIBLE; i++)
        {
            //get the index of the command we will display.
            int32 index = i + top_index;
            if (index >= KCC_NUM_COMMANDS) 
              break;

            //decide if this item should be drawn bright.
            bool draw_bright = (command_list_delay.Row() == i);

            //print the string.
            DKM_DrawString(MOUSE_COMMAND_LEFT + 5, mouse_line_y(i), CommandName((key_config_command)index), NULL, false, draw_bright);
        }

        //check if the command list up/down buttons should be drawn.
        command_up.DrawForeground();
        command_down.DrawForeground();
    }
    //check if we have a hilight
    else
    {
		//DKM_DrawString(MOUSE_ACTION_LEFT, MOUSE_ACTION_TOP - 10, "Select an action.", NULL, false, false);
        //DKM_DrawString(MOUSE_ACTION_LEFT, MOUSE_ACTION_TOP - 10, tongue_menu[T_MENU_MOUSE_SELECTACTION], NULL, false, false);
        
        // draw header
		  DKM_DrawString(MOUSE_HEADER_X, MOUSE_HEADER_Y, tongue_menu[T_MENU_MOUSE_HEADER], NULL, false, true); // highlighted
    }

      /*
      // TEMP: show settings
      float value1 = sensitivityX.Position();
      char buf[256];

      if (lookstrafe->value != 0)              //they are strafing.
      {
        sprintf(buf,"m_side = %f / %f",value1,sign(m_side->value) * value1);
        DKM_DrawString(MOUSE_SENSITIVITY_X + 135, MOUSE_SENSITIVITY_X_Y - 15, buf,   NULL, false, false);
      }
      else         //they are turning
      {
        //sprintf(buf,"m_yaw = %f / %f",value1,sign(m_yaw->value) * value1);
        sprintf(buf,"m_yaw = %f",value1);
        DKM_DrawString(MOUSE_SENSITIVITY_X + 135, MOUSE_SENSITIVITY_X_Y - 15, buf,   NULL, false, false);
      };
      */

}


void CMenuSubMouse::DrawBackground()
{
    sensitivityX.DrawBackground();
    sensitivityY.DrawBackground();
    speed.DrawBackground();

    SetRadioState(mouse_filter,MOUSE_FILTER_CVAR);
    mouse_filter.DrawBackground();

    if (selection != MA_NONE)
    {
        //check if the command list up/down buttons should be drawn.
        command_up.DrawBackground();
        command_down.DrawBackground();
    }
}


void CMenuSubMouse::PlaceEntities()
{
    //check if we have a selection.
    if (selection != MA_NONE)
    {
        //we have a selection.
        PositionActionHilight(selection);
        action_hilight_box.PlaceEntities();

        //the box around the command list.
        command_box.PlaceEntities();

        //check if have a hilight in the command box.
        if (command_hilight != -1)
        {
            PositionCommandHilight(command_hilight);
            command_hilight_box.PlaceEntities();
        }
    }
    //check if we have a hilight
    else if (action_hilight != MA_NONE)
    {
        PositionActionHilight(action_hilight);
        action_hilight_box.PlaceEntities();
    }

}


bool CMenuSubMouse::Keydown(int32 key)
{
	if (selection != MA_NONE)
	{
		switch(key)
		{
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
         /*
         case K_UPARROW: {
            //check if we have an action selected.
            if (selection == MA_NONE) {
                //check if we have a hilight.
                if (action_hilight == MA_NONE) {
                    //hilight the top one.
                    action_hilight = MA_X_AXIS;
                }
                //move the hilight in the action box.
                else if (action_hilight > MA_X_AXIS) {
                    //go up one action.
                    action_hilight = mouse_action(action_hilight - 1);
                }
            }
            else {
                //move the hilight in the command box.
                if (command_hilight <= 0) {
                    //try to scroll the list up.
                    TopIndex(top_index - 1);
                }
                else {
                    //move the hilight up.
                    command_hilight--;
                }
            }
            return true;
        }
        case K_DOWNARROW: {
            //check if we have an action selected.
            if (selection == MA_NONE) {
                //check if we have a hilight.
                if (action_hilight == MA_NONE) {
                    //hilight the top one.
                    action_hilight = MA_X_AXIS;
                }
                //move the hilight in the action box.
                else if (action_hilight < MA_BUTTON3) {
                    //go down one action.
                    action_hilight = mouse_action(action_hilight + 1);
                }
            }
            else {
                if (command_hilight == -1) {
                    //hilight the first one.
                    command_hilight = 0;
                }
                //move the hilight in the command box.
                else if (command_hilight >= MOUSE_NUM_COMMANDS_VISIBLE - 1 || command_hilight >= num_command_items) {
                    //try to scroll the list down.
                    TopIndex(top_index + 1);
                }
                else {
                    //move the hilight down.
                    command_hilight++;
                }
            }
            return true;
        }
        case K_ENTER: {
            //check if we have a selection 
            if (selection == MA_NONE) {
                //check if an action is hilighted.
                if (action_hilight != MA_NONE) {
                    //select the hilighted action.
                    SelectAction(action_hilight);
                    
                    TopIndex(top_index);
                    return true;
                }
            }
            //check if we have a command hilighted.
            else if (command_hilight != -1) {
                //select this command.
                SelectCommand(command_hilight + top_index);

                //unselect our action.
                SelectAction(MA_NONE);
                
                return true;
            }
            break;
        }
        case K_BACKSPACE:
        */

        case K_DEL:
            if (selection != MA_NONE) {
                //clear out the action.
                SelectCommand(-1);

                //deselect our action.
                SelectAction(MA_NONE);
                return true;
            }
            break;
        case K_ESCAPE: 
			{
			int32 old_selection = selection;
            //get rid of our selection.
            SelectAction(MA_NONE);

			if( Cvar_VariableValue( "console" ) == 0 )
			{
				return false;
			}

            //check if we have a selection.
            if (old_selection == MA_NONE) {
                //leave the menu.
                //DKM_EnterMain();
                DKM_Shutdown();       // exit entire interface  1.10 dsn
                return true;
            }
            return true;
        }
    }

    return false;
}

bool CMenuSubMouse::MousePos(int32 norm_x, int32 norm_y)
{
    bool ret = false;
    
    if (reverse_axis.MousePos(norm_x, norm_y) == true) ret = true;
    if (mouse_look.MousePos(norm_x, norm_y) == true)   ret = true;
    if (look_strafe.MousePos(norm_x, norm_y) == true)   ret = true;
    if (mouse_filter.MousePos(norm_x, norm_y) == true)   ret = true;

    if (sensitivityX.MousePos(norm_x, norm_y) == true) ret = true;
    if (sensitivityY.MousePos(norm_x, norm_y) == true) ret = true;
    if (speed.MousePos(norm_x, norm_y) == true) ret = true;

    if (selection != MA_NONE)
    {
        if (command_up.MousePos(norm_x, norm_y) == true) ret = true;
        if (command_down.MousePos(norm_x, norm_y) == true) ret = true;
    }

    if (ret == false)
    {
        
        //check if the mouse position is over one of the actions.
        mouse_action hit = HitAction(norm_x, norm_y);
        if (hit != MA_NONE)
        {
            //hilight the item that the cursor is on.
            if (action_hilight == MA_NONE)  //hilight immediately.
              action_hilight = hit;
            else                            //hilight after a delay.
                action_list_delay.Start(hit);

            return true;
        }
        else
        {
            //tell the delay that there is nothing under the mouse.
            action_list_delay.Start(-1);

            //abort the action list delay, in case it is on.
            action_list_delay.Abort();
        }
        

        //check if it is over a command.
        if (selection != MA_NONE)
        {
            int32 command = HitCommand(norm_x, norm_y);

            //set the command hilight if the cursor was on a command.
            if (command != -1) {
                if (command_hilight == -1) {
                    //hilight now.
                    command_hilight = command - top_index;
                }
                else {
                    //hilight after a delay.
                    command_list_delay.Start(command - top_index);
                }
            }
            else {
                //tell the delay that there is nothing under the mouse.
                command_list_delay.Start(-1);

                //abort the command list delay, in case it is on.
                command_list_delay.Abort();
            }
        }
    }

    return ret;
}

bool CMenuSubMouse::MouseDown(int32 norm_x, int32 norm_y)
{
    //if (selection == MA_Y_AXIS)
    //{
    if (reverse_axis.MouseDown(norm_x, norm_y) == true) return true;
    if (mouse_look.MouseDown(norm_x, norm_y) == true)   return true;
    if (look_strafe.MouseDown(norm_x, norm_y) == true)   return true;
    if (mouse_filter.MouseDown(norm_x, norm_y) == true)   return true;


    if (sensitivityX.MouseDown(norm_x, norm_y) == true) 
      return true;
    if (sensitivityY.MouseDown(norm_x, norm_y) == true) 
      return true;
    if (speed.MouseDown(norm_x, norm_y) == true) 
      return true;
    
    if (selection != MA_NONE)
    {
        if (command_up.MouseDown(norm_x, norm_y) == true) return true;
        if (command_down.MouseDown(norm_x, norm_y) == true) return true;
    }

    //check if they clicked on one of the actions.
    mouse_action clicked = HitAction(norm_x, norm_y);
    if (clicked != MA_NONE)
    {
        //they clicked on an action.
        SelectAction(clicked);

        TopIndex(top_index);
        return true;
    }

    //check if they clicked on a command.
    if (selection != MA_NONE) {
        int32 command = HitCommand(norm_x, norm_y);

        if (command != -1) {
            //we clicked an item.
            SelectCommand(command);
            return true;
        }
    }

    return false;
}


bool CMenuSubMouse::MouseUp(int32 norm_x, int32 norm_y)
{
    bool ret = false;

    //if (selection == MA_Y_AXIS)
    //{
        if (reverse_axis.MouseUp(norm_x, norm_y) == true)
        {
            //they clicked the reverse axis button.  invert the value of the m_pitch cvar.
            Cvar_SetValue("m_pitch", -m_pitch->value);
            return true;
        }


        if (mouse_look.MouseUp(norm_x, norm_y) == true)
        {
            //they clicked the mouselook button.
            Cvar_SetValue("freelook", (freelook->value) ? 0 : 1);
            return true;
        }

        if (look_strafe.MouseUp(norm_x, norm_y) == true)
        {
            //they clicked the mouselook button.
            Cvar_SetValue("lookstrafe", (lookstrafe->value) ? 0 : 1);
            return true;
        }

        if (mouse_filter.MouseUp(norm_x, norm_y) == true)
        {
            //they clicked the mouse filter button.
			ToggleRadioState(MOUSE_FILTER_CVAR);
            return true;
        }



    //}
    
    //if (selection == MA_X_AXIS || selection == MA_Y_AXIS)
    //{
        if (sensitivityX.MouseUp(norm_x, norm_y) == true)
        {
            //they changed the sensitivity.
            //read the value from the slider.
            float value = sensitivityX.Position();
            //check the function of the x axis.
            if (lookstrafe->value != 0)
            {
                //they are strafing.
                Cvar_SetValue("m_side", sign(m_side->value) * value);
            }
            else 
			{
                //they are turning
                Cvar_SetValue("m_yaw", sign(m_yaw->value) * value);
            }            
            return true;   
        }

        if (sensitivityY.MouseUp(norm_x, norm_y) == true)
        {
            //they changed the sensitivity.
            //read the value from the slider.
            float value = sensitivityY.Position();

                //check the function of the y axis.
                if (freelook->value != 0)
                {
                    //they are looking
                    Cvar_SetValue("m_pitch", sign(m_pitch->value) * value);
                }
                else
                {
                    //they are moving
                    Cvar_SetValue("m_forward", sign(m_forward->value) * value);
                }
            
            return true;   
        }

        if (speed.MouseUp(norm_x, norm_y) == true)
        {
			Cvar_SetValue("sensitivity",speed.Position());
			return true;
		}

    //}

    if (selection != MA_NONE)
    {
        if (command_up.MouseUp(norm_x, norm_y) == true) {
            //the want to scroll up.
            TopIndex(top_index - 1);
            return true;
        }
        if (command_down.MouseUp(norm_x, norm_y) == true) {
            //they want to scroll down.
            TopIndex(top_index + 1);
            return true;
        }
    }

    return ret;
}

