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
//  Keyboard Menu
//
///////////////////////////////////////////////////////////////////////////////////


#define KEY_HEADER_X (104) 

#define KEY_COL0_X ( 90) 
#define KEY_COL1_X (238)
#define KEY_COL2_X (360)



#define KEY_LINE_SPACING 15
#define key_line_y(i) (145 + (i) * KEY_LINE_SPACING)

#define KEY_NUM_VISIBLE_LINES (15)

#define KEY_LISTBOX_UP_X (280)
#define KEY_LISTBOX_UP_Y key_line_y(KEY_NUM_VISIBLE_LINES + 1.0)
#define KEY_LISTBOX_DOWN_X (KEY_LISTBOX_UP_X + 30)
#define KEY_LISTBOX_DOWN_Y KEY_LISTBOX_UP_Y

// box outline mins/maxs
//#define KEY_BORDER_LEFT (KEY_COL0_X - 10)
//#define KEY_BORDER_TOP key_line_y(-1)
#define KEY_BORDER_RIGHT (KEY_COL2_X + KEY_COL2_X - KEY_COL1_X + 2)
//#define KEY_BORDER_BOTTOM key_line_y(KEY_NUM_VISIBLE_LINES + 0.7)


// graphic separator between key command and key bindings
#define KEY_SEPARATOR_X   (230)
#define KEY_SEPARATOR_Y   (138)
#define KEY_SEPARATOR_Y2  (374)


#define KEY_DEFAULTS_X (KEY_HEADER_X)
#define KEY_DEFAULTS_Y (KEY_LISTBOX_DOWN_Y)

// confirm dlg
// the buttons
#define BUTTON_HEIGHT										(25)
#define BUTTON_SHORT_WIDTH									(63)
#define BUTTON_MED_WIDTH									(115)

// some cool stuff for centering buttons
#define BUTTON_SPACE(width,eawidth,max)						((width - (max * eawidth))/(max + 1))
#define BUTTON_L(left,width,num,max,eawidth)				(left + (num * BUTTON_SPACE(width,eawidth,max)) + ((num-1)*eawidth))
#define BUTTON_MED_LEFT(num,max)							(BUTTON_L(CONFIG_LEFT,FILE_LIST_WIDTH,num,max,BUTTON_MED_WIDTH))
#define CENTERED(left,width,cwidth)							(left + (0.5*width) - (0.5*cwidth))
#define CONFIRM_WIDTH										(360)
#define CONFIRM_HEIGHT										(90)
#define CONFIRM_BUTTON_TOP									(CONFIRM_BOTTOM - (BUTTON_HEIGHT + 10))
#define CONFIRM_BUTTON_LEFT(idx)							( BUTTON_L(CONFIRM_LEFT,(CONFIRM_RIGHT-CONFIRM_LEFT),idx,2,BUTTON_SHORT_WIDTH) )

#define CONFIG_LEFT											110
#define CONFIG_TOP											115
#define CONFIG_WIDTH										375
#define CONFIG_HEIGHT										300
#define CONFIRM_LEFT										(CENTERED(CONFIG_LEFT - 15,CONFIG_WIDTH,CONFIRM_WIDTH))
#define CONFIRM_RIGHT										(CONFIRM_LEFT + CONFIRM_WIDTH)
#define CONFIRM_TOP											(CENTERED(CONFIG_TOP,CONFIG_HEIGHT,CONFIRM_HEIGHT))
#define CONFIRM_BOTTOM										(CONFIRM_TOP + CONFIRM_HEIGHT)


//structure that holds info on the bindings that for the commands.
class command_binding {
public:
    command_binding(const char *name, const char *text);
	
//	void* command_binding::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MENU); }
//	void* command_binding::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MENU); }
//	void  command_binding::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  command_binding::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }
	
    //the name of the command that is displayed to the user.
    buffer64 command_name;
	
    //the name of the command that is sent to perform the action.
    buffer64 command_text;
	
    //the keys that are bound to this command.
    int32 keys[2];
};

static char message[256];
static char prompt[256];
static int	assigningKey;

command_binding *bindings[KCC_NUM_COMMANDS];


const char *CommandText(key_config_command command)
{
    if (command < KCC_FORWARD || command >= KCC_NUM_COMMANDS) return NULL;
    if (bindings[command] == NULL) return NULL;
	
    return bindings[command]->command_text;
}


const char *CommandName(key_config_command command)
{
    if (command < KCC_FORWARD || command >= KCC_NUM_COMMANDS) return NULL;
    if (bindings[command] == NULL) return NULL;
	
    return bindings[command]->command_name;
}


//searches for a printable name for the command from the list of standard game commands.
const char *FindCommandName(const char *command_text)
{
	if (!command_text)
		return "Custom";

    for (int32 i = 0; i < KCC_NUM_COMMANDS; i++)
    {
        //compare the command text to the keybinding.
        if (stricmp(command_text, bindings[i]->command_text) == 0)
        {
            //we found a match.
            return bindings[i]->command_name;
        }
    }
	
    //couldn't find a nice command name for this text.
    return "Custom";
}


//searches through the user's key bindings and fills in the key fields in the bindings array.
void FindKeyBindings()
{
    //go through each of our commands.
    for (int32 i = 0; i < KCC_NUM_COMMANDS; i++)
    {
        //the number of bindings we have found for this command.
        int32 num_found = 0;
		
        bindings[i]->keys[0] = -1;
        bindings[i]->keys[1] = -1;
		
        //search through the bindings
        for (int32 key = 0; key < 256; key++)
        {
            //don't check any mouse or joystick keys, except for mouse-wheel keys  12.17 dsn
            if (key >= 200 && key != K_PAUSE)
            {
				if (!(key == K_MWHEELUP || key == K_MWHEELDOWN || (key >= K_MOUSE1 && key <= K_MOUSE3)))
					continue;
            }
			
            if (keybindings[key] == NULL) 
				continue;
			
            //compare the binding with the command text.
            if (stricmp(bindings[i]->command_text, keybindings[key]) == 0)
            {
                //a match.
                bindings[i]->keys[num_found] = key;
                num_found++;
				
                if (num_found > 1) 
					break;
            }
        }
		
        //go to the next command.
    }
}


command_binding::command_binding(const char *name, const char *text)
{
    //put in default values.
    keys[0] = keys[1] = -1;
	
    //check our parameters.
    if (name == NULL) return;
    if (text == NULL) return;
	
    //fill in the command name and text.
    command_name = name;
    command_text = text;
}


CMenuSubKeyboard::CMenuSubKeyboard()
{
	// removed 1.12  dsn
	//border.Init(KEY_BORDER_LEFT, KEY_BORDER_TOP, KEY_BORDER_RIGHT, KEY_BORDER_BOTTOM, 2);
	separator.InitV(KEY_SEPARATOR_X, KEY_SEPARATOR_Y, KEY_SEPARATOR_Y2, 4);
	
	TopIndex(0);
	
	hilight_showing = true;
	hilight_row = hilight_col = 0;
	
	assigning_control = false;
	hilight_animation_time = 0;
	
	//initialize the command bindings.  (name, command text)
	
	bindings[KCC_FORWARD]			= new command_binding((const char *)tongue_menu[T_MENU_KCC_FORWARD],		"+forward");
	bindings[KCC_BACKWARD]			= new command_binding((const char *)tongue_menu[T_MENU_KCC_BACKWARD],		"+back");
	bindings[KCC_STEP_LEFT]			= new command_binding((const char *)tongue_menu[T_MENU_KCC_STEP_LEFT],		"+moveleft");
	bindings[KCC_STEP_RIGHT]		= new command_binding((const char *)tongue_menu[T_MENU_KCC_STEP_RIGHT],		"+moveright");
	bindings[KCC_ATTACK]			= new command_binding((const char *)tongue_menu[T_MENU_KCC_ATTACK],			"+attack");
	bindings[KCC_OPERATE]			= new command_binding((const char *)tongue_menu[T_MENU_KCC_OPERATE],		"+use");  // 3.17 dsn
	bindings[KCC_UP_JUMP]			= new command_binding((const char *)tongue_menu[T_MENU_KCC_UP_JUMP],		"+moveup");
	bindings[KCC_DOWN_CROUCH]		= new command_binding((const char *)tongue_menu[T_MENU_KCC_DOWN_CROUCH],	"+movedown");
	bindings[KCC_WEAPON_NEXT]		= new command_binding((const char *)tongue_menu[T_MENU_KCC_WEAPON_NEXT],	"weapnext");
	bindings[KCC_WEAPON_PREV]		= new command_binding((const char *)tongue_menu[T_MENU_KCC_WEAPON_PREV],	"weapprev");
	bindings[KCC_TURN_LEFT]			= new command_binding((const char *)tongue_menu[T_MENU_KCC_TURN_LEFT],		"+left");
	bindings[KCC_TURN_RIGHT]		= new command_binding((const char *)tongue_menu[T_MENU_KCC_TURN_RIGHT],		"+right");
	bindings[KCC_RUN]				= new command_binding((const char *)tongue_menu[T_MENU_KCC_RUN],			"+speed");
	bindings[KCC_SIDESTEP]			= new command_binding((const char *)tongue_menu[T_MENU_KCC_SIDESTEP],		"+strafe");
	bindings[KCC_LOOK_UP]			= new command_binding((const char *)tongue_menu[T_MENU_KCC_LOOK_UP],		"+lookup");
	bindings[KCC_LOOK_DOWN]			= new command_binding((const char *)tongue_menu[T_MENU_KCC_LOOK_DOWN],		"+lookdown");
	bindings[KCC_CENTER_VIEW]		= new command_binding((const char *)tongue_menu[T_MENU_KCC_CENTER_VIEW],	"centerview");
	bindings[KCC_MOUSE_LOOK]		= new command_binding((const char *)tongue_menu[T_MENU_KCC_MOUSE_LOOK],		"+mlook");
	bindings[KCC_KEYBOARD_LOOK]		= new command_binding((const char *)tongue_menu[T_MENU_KCC_KEYBOARD_LOOK], "+klook");
	bindings[KCC_SCREEN_UP]			= new command_binding((const char *)tongue_menu[T_MENU_KCC_SCREEN_UP],		"sizeup");
	bindings[KCC_SCREEN_DN]			= new command_binding((const char *)tongue_menu[T_MENU_KCC_SCREEN_DN],		"sizedown");
	bindings[KCC_SHOW_HUDS]			= new command_binding((const char *)tongue_menu[T_MENU_KCC_SHOW_HUDS],		"show_huds");
	// cek[1-5-00] result of consolidating the hud controls
	bindings[KCC_HUD_CYCLE]			= new command_binding((const char *)tongue_menu[T_MENU_HUD_CYCLE],			"hud_cycle");	
	bindings[KCC_HUD_NEXT]			= new command_binding((const char *)tongue_menu[T_MENU_HUD_NEXT],			"hud_next");
	bindings[KCC_HUD_PREV]			= new command_binding((const char *)tongue_menu[T_MENU_HUD_PREV],			"hud_prev");
	bindings[KCC_HUD_USE]			= new command_binding((const char *)tongue_menu[T_MENU_HUD_USE],			"hud_use");
	// controls for the sidekick hud
	bindings[KCC_SWAP_SIDEKICK]		= new command_binding((const char *)tongue_menu[T_MENU_KCC_SWAP_SIDEKICK], "sidekick_swap");	
	// cek[12-11-99] - sidekick commands to bind to keys
	bindings[KCC_COMMANDS_GET]		= new command_binding((const char *)tongue_menu[T_MENU_SIDEKICK_GET],		"sidekick_get");//"Pickup");
	bindings[KCC_COMMANDS_COME]		= new command_binding((const char *)tongue_menu[T_MENU_SIDEKICK_COME],		"sidekick_come");//"Follow");
	bindings[KCC_COMMANDS_STAY]		= new command_binding((const char *)tongue_menu[T_MENU_SIDEKICK_STAY],		"sidekick_stay");//"Stay");
	bindings[KCC_COMMANDS_ATTACK]	= new command_binding((const char *)tongue_menu[T_MENU_SIDEKICK_ATTACK],	"sidekick_attack");//"Attack");
	bindings[KCC_COMMANDS_BACKOFF]	= new command_binding((const char *)tongue_menu[T_MENU_SIDEKICK_BACK0FF],	"sidekick_backoff");//"NoAttack");
	
	bindings[KCC_COMMANDS_SAVE]		= new command_binding((const char *)tongue_menu[T_MENU_SAVE],				"save");
	bindings[KCC_COMMANDS_LOAD]		= new command_binding((const char *)tongue_menu[T_MENU_LOAD_GAME],			"load");
	// CEK[1-11-00]
	bindings[KCC_COMMANDS_TALK]		= new command_binding((const char *)tongue_menu[T_MENU_KCC_TALK],			"messagemode");
	bindings[KCC_COMMANDS_SAY_TEAM] = new command_binding((const char *)tongue_menu[T_MENU_KCC_SAY_TEAM],		"messagemode2");
	// SCG[1/13/00]: 
	bindings[KCC_SCOREBOARD]		= new command_binding((const char *)tongue_menu[T_MENU_KCC_SCOREBOARD],		"scoreboard");

	// cek[1-4-00] 
	buffer32 buf1,buf2;
	for (int i = KCC_WEAPON_SELECT_1; i <= KCC_WEAPON_SELECT_7; i++)
	{
		int selection = i - KCC_WEAPON_SELECT_1 + 1;
		buf1.Set(tongue_menu[T_MENU_KCC_WEAPON_SELECT],selection);
		buf2.Set("weapon_select_%d",selection);
		bindings[i]  = new command_binding((const char *)buf1, buf2);
	}
}	

void CMenuSubKeyboard::TopIndex(int32 index)
{
    if (index < 0)
        index = 0;   
    else 
		if (index + KEY_NUM_VISIBLE_LINES > KCC_NUM_COMMANDS)
			index = KCC_NUM_COMMANDS - KEY_NUM_VISIBLE_LINES;
		
		top_index = index;
		
		//check if the up button should be visible.
		listbox_up.Enable((top_index > 0));
		
		//check if the down button should be visible.
		if (top_index + KEY_NUM_VISIBLE_LINES >= KCC_NUM_COMMANDS)
			listbox_down.Enable(false);
		else
			listbox_down.Enable(true);
}


void CMenuSubKeyboard::StartHilightCountdown(int32 row, int32 column)
{
	//bad row/column? forget the whole thing.
	if (column < -1 || column > 1 || row < -1 || row >= KEY_NUM_VISIBLE_LINES)
		return;
	
	hilight_delay.Start(row, column);
}


void CMenuSubKeyboard::AbortHilightCountdown()
{
	hilight_delay.Abort();
}


//row 0 is the top row that is showing.
void CMenuSubKeyboard::Hilight(int32 row, int32 column)
{
	if (column < 0 || column > 1 || row < 0 || row >= KEY_NUM_VISIBLE_LINES)
	{
		//bad row/column. un-hilight everything.
		hilight_showing = false;
		return;
	}
	
	hilight_showing = true;
	hilight_row     = row;
	hilight_col     = column;
	
	HilightSize(0);    //size the hilight box.
}

#define KEY_ANIM_PERIOD    400.0f			// higher value = slower pulse
#define KEY_ANIM_AMPLITUDE  20.0f

#define KEY_HILIGHT_WIDTH  (112)



void CMenuSubKeyboard::HilightSize(int32 time)
{
	int x1, x2;
	
    //compute the change in the hilight box size.  //make a sin wave that goes 0-1-0-1-0
    float box_change = KEY_ANIM_AMPLITUDE * (1 - (cos(M_PI * 2 * time / KEY_ANIM_PERIOD) * 0.5f + 0.5f));
	
    
    if (hilight_col == 0)
    {
		x1 = KEY_COL1_X + box_change - 2;
		x2 = KEY_COL1_X + KEY_HILIGHT_WIDTH - box_change + 2;
    }
    else
    {
		x1 = KEY_COL2_X + box_change - 2;
		x2 = KEY_COL2_X + KEY_HILIGHT_WIDTH - box_change + 2;
    }
	
    hilight_box.Init(x1,                                // left
		key_line_y(hilight_row),           // top
		x2,                                // right
		key_line_y(hilight_row + 1),       // bottom
		2);                                // thickness
}



void CMenuSubKeyboard::AssignStart()
{
	if (hilight_showing == false) 
		return;
	
	assigningKey = -1;
	assigning_control = true;   //set our flag.
	hilight_animation_time = 0; //reset the animation time. 
	
	S_StartLocalSound(DKM_sounds[DKMS_BUTTON5]);  // play a sound.
}


void CMenuSubKeyboard::AssignAbort()
{
	assigning_control = false;
	assigningKey = -1;
	HilightSize(0);
	
	S_StartLocalSound(DKM_sounds[DKMS_BUTTON5]);  // play a sound.
}


void CMenuSubKeyboard::AssignDelete()
{
	HilightSize(0);
	assigningKey = -1;
	
	int32 command_num = top_index + hilight_row;               // get command number changing
	if (command_num < 0 || command_num >= KCC_NUM_COMMANDS)    // boundary check
		return;
	
	//get the key that is selected.
	int32 oldkey = (hilight_col == 0) ?
		bindings[top_index + hilight_row]->keys[0] :
	bindings[top_index + hilight_row]->keys[1];
	
	Key_SetBinding(oldkey, "");     // delete binding for this key
	FindKeyBindings();              // re-find bindings.
	S_StartLocalSound(DKM_sounds[DKMS_NO]);
	
	assigning_control = false;
	
	S_StartLocalSound(DKM_sounds[DKMS_BUTTON5]);  // play a sound. 
}


void CMenuSubKeyboard::AssignComplete(int32 key) 
{
    //whatever happens now, we are done assigning a key.
    assigning_control = false;
	assigningKey = -1;
	
    //fix the size of our hilight box.
    HilightSize(0);
	
    //get the command number we are chaning.
//    int32 command_num = top_index + hilight_row;
	
    //paranoia strikes:
    if (top_index + hilight_row < 0 || top_index + hilight_row >= KCC_NUM_COMMANDS) {
        return;
    }
	
    //ok, we have a key that we are going to assign to this command.
    //get the old key.
    int32 oldkey = (hilight_col == 0) ?
		bindings[top_index + hilight_row]->keys[0] :
	bindings[top_index + hilight_row]->keys[1];
	
    //the other key that is assigned to this command.
    int32 secondkey = (hilight_col != 0) ?
		bindings[top_index + hilight_row]->keys[0] :
	bindings[top_index + hilight_row]->keys[1];
	
    //check if the new key is different from the old one.
    if (key == oldkey || key == secondkey) {
        //nothing changing.
        return;
    }
	
    //unbind the old key.
    Key_SetBinding(oldkey, "");
	
    //set the new key.
    Key_SetBinding(key, bindings[top_index + hilight_row]->command_text);
	
    //we need to re-find our bindings.
    FindKeyBindings();
	
    S_StartLocalSound(DKM_sounds[DKMS_BUTTON5]);
}


void CMenuSubKeyboard::Enter()
{
    defaults.Init(KEY_DEFAULTS_X, KEY_DEFAULTS_Y, menu_font, menu_font_bright, button_font);
    defaults.InitGraphics(BUTTONMEDIUM_UP_STRING, BUTTONMEDIUM_DOWN_STRING, BUTTONMEDIUM_DISABLE_STRING);
    
    defaults.SetText(tongue_menu[T_MENU_DEFAULTS], true);
	
    listbox_up.Init(KEY_LISTBOX_UP_X, KEY_LISTBOX_UP_Y, menu_font, menu_font_bright, button_font);
    listbox_up.InitGraphics(TOENAIL_UP_OFF_STRING, TOENAIL_UP_ON_STRING, TOENAIL_UP_DISABLE_STRING);
    //listbox_up.SetText(TRIARROW_UP_STRING, true);
	listbox_up.SetText(ARROW_UP_STRING, true);
	
    listbox_down.Init(KEY_LISTBOX_DOWN_X, KEY_LISTBOX_DOWN_Y, menu_font, menu_font_bright, button_font);
    listbox_down.InitGraphics(TOENAIL_DOWN_OFF_STRING, TOENAIL_DOWN_ON_STRING, TOENAIL_DOWN_DISABLE_STRING);
    //listbox_down.SetText(TRIARROW_DOWN_STRING, true);
	listbox_down.SetText(ARROW_DOWN_STRING, true);
	
    FindKeyBindings();
	
    //refresh the hilight size.
    if (hilight_showing == true)
		Hilight(hilight_row, hilight_col);
	
    //set the top index.  allows the scroll buttons to be enabled/disabled
    TopIndex(top_index);
	assigningKey = -1;
	confirm_dlg.Init(CONFIRM_LEFT,CONFIRM_TOP,CONFIRM_RIGHT,CONFIRM_BOTTOM,2,CONFIRM_BUTTON_TOP,CONFIRM_BUTTON_LEFT(1),CONFIRM_BUTTON_LEFT(2));
}


void CMenuSubKeyboard::Animate(int32 elapsed_time)
{
	if (confirm_dlg.Showing())
	{
		confirm_dlg.Animate(elapsed_time);
		return;
	}
    //check if the hilight delay timer is running.
    if (hilight_delay.On() == true)
    {
        //check if the delay is past.
        if (hilight_delay.TimeUp(GetTickCount()) == true)
        {
            //hilight the correct row.
            Hilight(hilight_delay.Row(), hilight_delay.Col());
            
            //turn off the timer.
            hilight_delay.Abort();
        }
    }
	
    //if we are assigning a control, we make the hilight box throb.
    if (assigning_control == true)
    {
        //add time to our counter.
        hilight_animation_time += elapsed_time;
        
        //size the hilight box.
        HilightSize(hilight_animation_time);
    }
}


char *KeyString(int32 key)
{
	// configing this slot?  return a string with ???'s that will be displayed.
    if (key == 1000)
		return "     ???";
	
    if (key == -1)
		return (tongue_menu[T_MENU_NOKEYSETTING]);  // no bind setting
	
	
	switch (key)
    {
	case K_LEFTARROW   : return (tongue_menu[T_MENU_ARROWS_LEFT]);   break; 
	case K_UPARROW     : return (tongue_menu[T_MENU_ARROWS_UP]);     break;
	case K_DOWNARROW   : return (tongue_menu[T_MENU_ARROWS_DOWN]);   break;
	case K_RIGHTARROW  : return (tongue_menu[T_MENU_ARROWS_RIGHT]);  break;
	default            : return Key_KeynumToString(key);             break;
	}
}


void CMenuSubKeyboard::DrawBackground()
{
	if (confirm_dlg.Showing())
	{
		confirm_dlg.DrawBackground();
		return;
	}

    if (assigning_control == false)
    {
        defaults.DrawBackground();
		
        //draw the list box buttons.
        listbox_up.DrawBackground();
        listbox_down.DrawBackground();
    }
}


void CMenuSubKeyboard::DrawForeground()
{
	if (confirm_dlg.Showing())
	{
		confirm_dlg.DrawForeground();
		return;
	}

	if (assigning_control == false)
	{
		if (hilight_showing == true)
			DKM_DrawString(KEY_HEADER_X, key_line_y(-2), tongue_menu[T_MENU_SETDELETE], NULL, false, true);  // highlight title
		else
			DKM_DrawString(KEY_HEADER_X, key_line_y(-2), tongue_menu[T_MENU_SELECT], NULL, false, true);  // highlight title
		
		defaults.DrawForeground();
		
		//draw the list box buttons.
		listbox_up.DrawForeground();
		listbox_down.DrawForeground();
	}
	else
	{
		DKM_DrawString(KEY_HEADER_X, key_line_y(-2), tongue_menu[T_MENU_SETABORT], NULL, false, true);
	}
	
	//draw the main listing of commands.
	for (int32 i = 0; i <  KEY_NUM_VISIBLE_LINES; i++)
	{
		int32 index = i + top_index;
		
		//do an error check just for paranoia's sake.
		if (index >= KCC_NUM_COMMANDS)
			break;
		
		//draw the command name.
		DKM_DrawString(KEY_COL0_X, key_line_y(i), bindings[index]->command_name, NULL, false, false);
		
		//draw the 2 keys that are mapped to the command.
		int32 key1 = bindings[index]->keys[0];
		int32 key2 = bindings[index]->keys[1];
		
		//check if we are configing a key.
		if (assigning_control && i == hilight_row)
		{
			if (hilight_col == 0) key1 = 1000;
			if (hilight_col == 1) key2 = 1000;
		}
		
		bool draw_bright;
		
		//should we draw this key red?
		draw_bright = (i == hilight_delay.Row() && hilight_delay.Col() == 0);
		DKM_DrawString(KEY_COL1_X, key_line_y(i), KeyString(key1), NULL, false, draw_bright);
		
		draw_bright = (i == hilight_delay.Row() && hilight_delay.Col() == 1);
		DKM_DrawString(KEY_COL2_X, key_line_y(i), KeyString(key2), NULL, false, draw_bright);
	}   
}


void CMenuSubKeyboard::PlaceEntities()
{
	if (confirm_dlg.Showing())
	{
		confirm_dlg.PlaceEntities();
		return;
	}
    // removed 1.12  dsn
    //border.PlaceEntities();
    separator.PlaceEntitiesV();  //draw the separator.
	
    if (hilight_showing == true)
    {
        hilight_box.PlaceEntities();
    }
}



bool CMenuSubKeyboard::Keydown(int32 key)
{
	if (confirm_dlg.Showing())
	{
		if (confirm_dlg.Keydown(key)) 
		{
			ConfirmFinish();
			return true;
		}
		return false;
	}
	
	if (assigning_control == true)
	{
		//dont allow keys that arent from the keyboard.
		if (((key >= 200) && (key != K_PAUSE)) || (key == K_MOUSE1) )
		{
			// dont allow assigning of those keys in the keyboard menu, EXCEPT
			// for MWHEELUP and MWHEELDOWN settings here   12.17  dsn
			//      if (!(key == K_MWHEELUP || key == K_MWHEELDOWN  || (key >= K_MOUSE1 && key <= K_MOUSE3)))
			return true;
		}
		
		switch (key)
		{
		case K_ESCAPE :  // abort our assign process.
			AssignAbort();
			return true;
			
			//filter any keys that arent allowed to be configured.
		case '`' :
			return true;
		}
		
		if ( keybindings[key] && strlen(keybindings[key]) )		// already has a binding
		{
			int32 oldkey = (hilight_col == 0) ? bindings[top_index + hilight_row]->keys[0] : bindings[top_index + hilight_row]->keys[1];		
			int32 secondkey = (hilight_col != 0) ? bindings[top_index + hilight_row]->keys[0] : bindings[top_index + hilight_row]->keys[1];

			if ((key == oldkey) || (key == secondkey))
			{
				AssignComplete(key);  // finish the assignment.
			}
			else
			{
				Com_sprintf(message,sizeof(message),tongue_menu[T_MENU_KEY_ASSIGNED],KeyString(key),FindCommandName(keybindings[key]));
				Com_sprintf(prompt,sizeof(prompt),tongue_menu[T_MENU_KEY_CONTINUE]);
				assigningKey = key;
				confirm_dlg.Show(message,prompt);
			}
		}
		else
		{
			AssignComplete(key);  // finish the assignment.
		}
		return true;
    }
	
    // check for the escape key, which will always exit the menu.
    if (key == K_ESCAPE)
    {
		//leave the menu.
		//DKM_EnterMain();
		if( Cvar_VariableValue( "console" ) == 0 )
		{
			return false;
		}
		DKM_Shutdown();       // exit entire interface  1.10 dsn
		
		return true;
    }
	
    if (hilight_showing == true)
    {
		switch (key)
		{
        case K_BACKSPACE :
        case K_DEL :
			{
				AssignDelete(); // delete the selected key
				return true;
			}
			
		case K_MWHEELUP:
        case K_UPARROW :
			{
				// if hilighting something at the top, scroll up.
				if (hilight_row < 1)
					TopIndex(top_index - 1);
				else
					Hilight(hilight_row - 1, hilight_col);      //move the hilight up a row.
				
				S_StartLocalSound(DKM_sounds[DKMS_BUTTON4]);  // play sound
				return true;
			}
			
		case K_MWHEELDOWN:
        case K_DOWNARROW :
			{
				//if we are hilighting on the bottom row, scroll down.
				if (hilight_row >= KEY_NUM_VISIBLE_LINES - 1)
					TopIndex(top_index + 1);
				else
					Hilight(hilight_row + 1, hilight_col);      // move the highlight down
				
				S_StartLocalSound(DKM_sounds[DKMS_BUTTON4]);  // play sound
				return true;
			}
			
        case K_RIGHTARROW :
			{
				if (hilight_col == 0)
					Hilight(hilight_row, 1);                    // move right column
				
				S_StartLocalSound(DKM_sounds[DKMS_BUTTON4]);
				return true;
			}
			
        case K_LEFTARROW :                              // move left column
			{
				if (hilight_col == 1)
					Hilight(hilight_row, 0);
				
				S_StartLocalSound(DKM_sounds[DKMS_BUTTON4]);  // play sound
				return true;
			}
			
        case K_ENTER :
			{
				AssignStart();                                // start assign of selected key
				return true;
			}
			
        case K_PGDN :
			{
				// get the control to be hilighted.
				int32 index = top_index + hilight_row + 5;
				
				bound_max(index, KCC_NUM_COMMANDS - 1);             // force boundaries
				
				//check if that index is shown on the screen.
				if (index >= top_index + KEY_NUM_VISIBLE_LINES)
				{
					//it is off the bottom.  figure out how many lines we need to scroll down to reach it.
					int32 newtop = index - KEY_NUM_VISIBLE_LINES + 1; 
					
					TopIndex(newtop);                                 // scroll to top index.
					Hilight(KEY_NUM_VISIBLE_LINES - 1, hilight_col);  // put selection on bottom column.
				}
				else
					Hilight(index - top_index, hilight_col);          // move hilight to command.
				
				S_StartLocalSound(DKM_sounds[DKMS_BUTTON4]);      // play sound
				return true;
			}
			
        case K_PGUP :
			{
				// get the control to be hilighted.
				int32 index = top_index + hilight_row - 5;
				bound_min(index, 0);                                // force boundaries
				
				// check if that index is shown on the screen.
				if (index < top_index)
				{
					// it is off the top.  set the new top index to the index we want to see.
					TopIndex(index);
					Hilight(0, hilight_col);                          // hilight top row.
				}
				else          
					Hilight(index - top_index, hilight_col);          // hilight that command.
				
				S_StartLocalSound(DKM_sounds[DKMS_BUTTON4]);        // play sound
				return true;
			}
		}
		
		return false;
    }
	
	
    switch (key)
    {
	case K_UPARROW:
		TopIndex(top_index - 1);
		return true;
		
	case K_DOWNARROW:
		TopIndex(top_index + 1);
		return true;
		
	case K_PGUP:
		TopIndex(top_index - KEY_NUM_VISIBLE_LINES / 2);
		return true;
		
	case K_PGDN:
		TopIndex(top_index + KEY_NUM_VISIBLE_LINES / 2);
		return true;
		
	case K_ENTER:
		Hilight(0, 0);
		return true;
    }
	
    return false;
}


bool CMenuSubKeyboard::MousePos(int32 norm_x, int32 norm_y)
{
	if (confirm_dlg.Showing())
	{
		if (confirm_dlg.MousePos(norm_x,norm_y)) return true;
		return false;
	}

	bool ret = false;
	
	if (defaults.MousePos(norm_x, norm_y) == true) 
		ret = true;
	
	if (assigning_control == false)
	{
		if (listbox_up.MousePos(norm_x, norm_y) == true) 
			ret = true;
		
		if (listbox_down.MousePos(norm_x, norm_y) == true) 
			ret = true;
/*		
		// check if mouse over one of the 2 columns of key assignments.
		int32 select_row = 
            list_box_hit_test(KEY_COL1_X - 3, 
            key_line_y(0) - 3, 
            KEY_BORDER_RIGHT - 7, 
            key_line_y(KEY_NUM_VISIBLE_LINES), 
            norm_x, 
            norm_y, 
            KEY_NUM_VISIBLE_LINES);
		
		if (select_row != -1)
		{
			// over a key / get the column
			int32 column = (norm_x >= KEY_COL2_X - 3) ? 1 : 0;
			
			//check if already have something hilighted.
			if (hilight_showing == true)
				StartHilightCountdown(select_row, column);  // start the timer to select a new item.
			else
				Hilight(select_row, column);                // hilight the item now.
		}
		else 
		{
			StartHilightCountdown(-1, -1);  // tell the timer that we have nothing to select.
			AbortHilightCountdown();        // abort the current timer.
		}
*/
	}
	
	return ret;
}


bool CMenuSubKeyboard::MouseDown(int32 norm_x, int32 norm_y)
{
	if (confirm_dlg.Showing())
	{
		if (confirm_dlg.MouseDown(norm_x,norm_y)) return true;
		return false;
	}

    if (defaults.MouseDown(norm_x, norm_y) == true) 
      return true;

/*    
    if (assigning_control == true)  // assigning this mouse click to a button?   12.18  dsn
    {
		AssignComplete((int)K_MOUSE1);  //finish the assignment.
		return true;
    }
    else*/
    {
		if (listbox_up.MouseDown(norm_x, norm_y) == true) 
			return true;
		
		if (listbox_down.MouseDown(norm_x, norm_y) == true) 
			return true;
    }
	
    //check if we are over one of the 2 columns of key assignments.
    int32 select_row = 
		list_box_hit_test(KEY_COL1_X - 3, 
		key_line_y(0) - 3, 
		KEY_BORDER_RIGHT - 7, 
		key_line_y(KEY_NUM_VISIBLE_LINES), 
		norm_x, 
		norm_y, 
		KEY_NUM_VISIBLE_LINES);
	
    if (select_row != -1)
    {
		// clicked on a key./ get column clicked on.
		int32 column = (norm_x >= KEY_COL2_X - 3) ? 1 : 0;
		
		Hilight(select_row, column);  // hilight this item.
		AssignStart();                // set assign flag
    }
	
    return false;
}

bool CMenuSubKeyboard::ConfirmFinish()
{
	if (confirm_dlg.Showing())
	{
		bool yes = false;
		if (confirm_dlg.Result(yes))
		{
			if ((assigningKey != -1) && yes)
			{
				AssignComplete(assigningKey);  // finish the assignment.
				assigningKey = -1;
			}
			return true;
		}
	}

	return false;
}

bool CMenuSubKeyboard::MouseUp(int32 norm_x, int32 norm_y)
{
	if (confirm_dlg.Showing())
	{
		if (confirm_dlg.MouseUp(norm_x,norm_y))
		{
			ConfirmFinish();
			return true;
		}
		return false;
	}
    
    if (defaults.MouseUp(norm_x, norm_y) == true)
    {
		// reset the default keys here     1.4  dsn
		Cbuf_AddText ("exec default_keys.cfg\n");
		Cbuf_Execute();
		FindKeyBindings();
		
		return true;
    }
	
    if (assigning_control == false)
    {
		if (listbox_up.MouseUp(norm_x, norm_y) == true)
		{
			TopIndex(top_index - 1); // clicked the listbox up button.
			return true;
		}
		
		if (listbox_down.MouseUp(norm_x, norm_y) == true)
		{
			TopIndex(top_index + 1);  // clicked the listbox down button.
			return true;
		}
    }
	
    return false;
}



CMenuSubKeyboard::~CMenuSubKeyboard()
// destructor 
{
	for (int32 i = 0; i < KCC_NUM_COMMANDS; i++)
	{
		if (bindings[i] != NULL)
			delete (bindings[i]);  // be free, memory!
	}
}
