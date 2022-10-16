//#pragma once

#ifndef _DK_MENUP_H_
#define _DK_MENUP_H_

#include "qcommon.h"
#include "ref.h"

#include "dk_menu_controls.h"

#include "../GOA/CEngine/goaceng.h"

typedef struct gl_driver_info_s
{
	buffer64	szFilename;
	buffer32	szVendor;
	buffer32	szRenderer;
	buffer32	szVersion;
	buffer256	szExtensions;
} gl_driver_info_t;

//extern float use_shadows;
extern int32 vid_menu_mode;	// resolution
extern float vid_brightness;	// brightness
extern int32 texture_quality;
extern int32 tesselation_quality;

extern int32 current_gl_driver;

extern array_obj<gl_driver_info_t>	gl_driver_info;
extern int							gl_driver_info_count;

extern bool is_fullscreen;
extern float use_shadows;	// shadows
extern short modulate;

extern char * DKM_sounds[];


typedef enum
{
// SCG[11/29/99]:   DKMS_LOOP,
  DKMS_MENU_MP3_1,
  DKMS_MENU_MP3_2,
  DKMS_ENTER,
  DKMS_DESCEND,
  DKMS_ASCEND,
  //DKMS_QUIT, //NSS[11/5/99]:Added because someone else commented out the Quit sound and it broke things!
  DKMS_NO,
  DKMS_BUTTON3,
  DKMS_BUTTON4,
  DKMS_BUTTON5,
  DKMS_BUTTON6,
  DKMS_BUTTON7,
  DKMS_EXIT_MENU,
  DMKS_NUMSOUNDS
};




//private types and classes used for the menu.
class CMenu;
    class CMenuMain;
    class CMenuSub;
        class CMenuSubQuit;
        class CMenuSubResume;
        class CMenuSubDemos;
        class CMenuSubAdvanced;
        class CMenuSubOptions;
        class CMenuSubJoystick;
        class CMenuSubKeyboard;
        class CMenuSubMouse;
        class CMenuSubVideo;
        class CMenuSubSound;
        class CMenuSubSavegame;
        class CMenuSubLoadgame;
        class CMenuSubMultiplayer;
        class CMenuSubNewgame;



//an enum for each bitmap
typedef enum {
    //the cursor images.
    DKB_CURSOR_00 = 0,
    DKB_CURSOR_01,
    DKB_CURSOR_02,
    DKB_CURSOR_03,
    DKB_CURSOR_04,
    DKB_CURSOR_05,
    DKB_CURSOR_06,
    DKB_CURSOR_07,
    DKB_CURSOR_08,
    DKB_CURSOR_09,

    //the background images.
    DKB_BACK00,
    DKB_BACK01,
    DKB_BACK02,
    DKB_BACK10,
    DKB_BACK11,
    DKB_BACK12,

    DKB_NUM
} dk_interface_bmp;


//a structure to keep track of our bitmaps.
typedef struct interface_bmp_desc_s
{
    //the name of the pic.
    char *name;
    resource_t	resource;
} interface_bmp_desc;



//the different possible status's for each button
typedef enum
{
    BS_START, 
    BS_HILIGHT_FORWARD,
    BS_HILIGHT_BACKWARD,
    BS_HILIGHT, 
    BS_SELECT_FORWARD,
    BS_SELECT_BACKWARD,
    BS_SELECTED
} dk_button_status;


//functions we use.
void DKM_NewMousePos();
void DKM_Animate();
bool in_main_game(void);
void M_PopMenu();
float CalcFov(float fov_x, float width, float height);
void PlayInterfaceSound(const char* file_name);


extern void *menu_font;           //the font.
extern void *menu_font_bright;    //the bright font.
extern void *button_font;         //the button graphic font.

void DKM_DrawString(int32 norm_x, int32 norm_y, const char *str, void *font, bool center, bool hilighted, float alpha = 0.0);
void DKM_DrawString(int32 norm_x, int32 norm_y, const char *str, int32 norm_max_pixel_width, bool bright);

//shuts down the menu.
void DKM_Shutdown();

//sets a string that is output to the execute buffer when the menu is done shutting down.  Takes
//printf style arguments.
void DKM_ShutdownString(const char *format_string, ...);

//enters the main menu, closing any open sub-menu.
void DKM_EnterMain();

//the different menus call this to get entity structures to fill in.
entity_t *NewEntity();

//the amount of times needed for the various animations.

#define ANIMATION_SELECT_TIME (350)
#define ANIMATION_HILIGHT_TIME (ANIMATION_SELECT_TIME / 2)

#define ANIMATION_BANNER_TURN (100)
#define ANIMATION_BANNER_WIGGLE (1000)

#define SHUTDOWN_SEQUENCE_TIME (ANIMATION_SELECT_TIME * 3.0) // give it time to shutdown

#define CURSOR_TOTAL_TIME (400)
#define CURSOR_NUM_FRAMES (9)

#define INPUTFIELD_WIDE_CHAR (13)
#define INPUTFIELD_SHORT_CHAR (12)

#define SLIDER_BACKGROUND_CHAR (10)
#define SLIDER_TAB_CHAR (11)
#define SLIDER_WIDTH (210)

#define BUTTONSMALL_DISABLE_STRING "\x001"
#define BUTTONSMALL_UP_STRING "\x002"
#define BUTTONSMALL_DOWN_STRING "\x003"
#define BUTTONMEDIUM_DISABLE_STRING "\x004"
#define BUTTONMEDIUM_UP_STRING "\x005"
#define BUTTONMEDIUM_DOWN_STRING "\x006"
#define BUTTONLONG_DISABLE_STRING "\x007"
#define BUTTONLONG_UP_STRING "\x008"
#define BUTTONLONG_DOWN_STRING "\x009"
#define BUTTONFAT_DISABLE_STRING "\x01a\x01b"
#define BUTTONFAT_UP_STRING "\x01c\x01d"
#define BUTTONFAT_DOWN_STRING "\x01e\x01f"
#define BUTTONBACK_DISABLE_STRING "\x020"
#define BUTTONBACK_UP_STRING "\x021"
#define BUTTONBACK_DOWN_STRING "\x022"

//the different commands we can set in the keyboard, mouse, or joystick config menu
typedef enum
{
    KCC_FORWARD,
    KCC_BACKWARD,
    KCC_STEP_LEFT,
    KCC_STEP_RIGHT,
    KCC_ATTACK,
    KCC_UP_JUMP,
    KCC_OPERATE,       // 12.14 dsn
    KCC_DOWN_CROUCH,
    KCC_WEAPON_NEXT,
    KCC_WEAPON_PREV,
    KCC_TURN_LEFT,
    KCC_TURN_RIGHT,
    KCC_RUN,
    KCC_SIDESTEP,
    KCC_LOOK_UP,
    KCC_LOOK_DOWN,
    KCC_CENTER_VIEW,
    KCC_MOUSE_LOOK,
    KCC_KEYBOARD_LOOK,
    KCC_SCREEN_UP,
    KCC_SCREEN_DN,
    KCC_SHOW_HUDS,
/*    KCC_INV_OPEN,
    KCC_INV_USE,
//    KCC_INV_DROP,
    KCC_INV_NEXT,
    KCC_INV_PREV,*/
	KCC_HUD_CYCLE,
	KCC_HUD_NEXT,
	KCC_HUD_PREV,
	KCC_HUD_USE,

	KCC_SWAP_SIDEKICK,

//	KCC_COMMANDS_NEXT,
//    KCC_COMMANDS_PREV,
//	  KCC_COMMANDS_ACT,

	// cek[12-11-99]
	KCC_COMMANDS_GET,
	KCC_COMMANDS_COME,
	KCC_COMMANDS_STAY,
	KCC_COMMANDS_ATTACK,
	KCC_COMMANDS_BACKOFF,

	KCC_COMMANDS_SAVE,
	KCC_COMMANDS_LOAD,

	// cek[1-11-00]
	KCC_COMMANDS_TALK,
	KCC_COMMANDS_SAY_TEAM,
	// SCG[1/13/00]: 
	KCC_SCOREBOARD,

	KCC_WEAPON_SELECT_1,
	KCC_WEAPON_SELECT_2,
	KCC_WEAPON_SELECT_3,
	KCC_WEAPON_SELECT_4,
	KCC_WEAPON_SELECT_5,
	KCC_WEAPON_SELECT_6,
	KCC_WEAPON_SELECT_7,

    KCC_NUM_COMMANDS
} key_config_command;

//gets the command text for the standard game command.
const char *CommandText(key_config_command command);
//gets the printable command name for the standard game command.
const char *CommandName(key_config_command command);
//searches for a printable name for the command from the list of standard game commands.
const char *FindCommandName(const char *command_text);

//used to set boolean state (1/0) cvars.
void ToggleRadioState(const char *cvar_name);
//reads the value of the cvar, and sets the check state for the radio button control.
void SetRadioState(CInterfaceButtonRadio &control, const char *cvar_name, bool inverse = false);
void Cvar_ForceSetValue (const char *var_name, float value);
void ToggleCvarFlag(char *cvar_name, unsigned int flag);
void SetRadioStateFlag(CInterfaceButtonRadio &control, const char *cvar_name, unsigned int flag, bool inverse = false) ;

//macro that does list box hit test, returns the index that was selected, or -1 if we are off the box.
//num is the number of visible lines from top to bottom.  x and y are the current position of the mouse.
#define list_box_hit_test(left, top, right, bottom, x, y, num) \
    (((x) > (left) && (x) < (right) && (y) > (top) && (y) < (bottom)) ? (compute_select_index((top), (y), (bottom), (num))) : -1)

//macro used to compute selected index given the top, bottom, and current y, plus the number of indexes spanned.
#define compute_select_index(top, y, bottom, num) bound_max_inline(bound_min_inline(int32(float((y) - (top))/float((bottom) - (top)) * (num)), 0), (num) - 1)

//this class can be used to set up delays in our hilight operations.
class hilight_delay_control {
  public:
    hilight_delay_control();
    
  protected:
    //the amount of time we wait, in ms.
    int32 time;

    //true if the timer is counting.
    bool timer_on;

    //the time that the timer was started.
    int32 timer_start;

    //the row and column that will be hilighted when the time is up.
    int32 row;
    int32 col;

  public:
    //checks to make sure the given row/column isn't already set up and counting down.
    void Start(int32 row, int32 col = 0, int32 delay = 100);
    void Abort();

    int32 Row();
    int32 Col();

    bool On();

    //returns true if the timer has expired.
    bool TimeUp(int32 cur_time);
};

//base class for all menu screens classes.
class CMenu {
  public:
    CMenu();
    virtual ~CMenu();

//	void* CMenu::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MENU); }
//	void* CMenu::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MENU); }
//	void  CMenu::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CMenu::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

    //called when the menu is first entered.
    virtual void Enter();
    //called when the menu is exited.
    virtual void Exit();

    //moves things based on the time elapsed.
    virtual void Animate(int32 elapsed_time);

    //draws the menu
    virtual void DrawBackground();
    virtual void DrawForeground();
    //add's the menu's 3d entities to the list.
    virtual void PlaceEntities();

    //called when the mouse is moved.  returns true if this menu handled the movement.
    virtual bool MousePos(int32 norm_x, int32 norm_y);

    //called when the mouse is clicked down and released. returns true if the
    //click was handled.
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    virtual bool MouseUp(int32 norm_x, int32 norm_y);

    //accepts keyboard input.  
    //returns true if the key was handled.
    virtual bool Keydown(int32 key);
    virtual bool Keyup(int32 key);
};

//class for the main menu with the 14 buttons.
class CMenuMain : public CMenu {
  public:
    CMenuMain();
    ~CMenuMain();

	void Init();
	void GameShutdown();
  void ResetSubMenu(dk_main_button menu_num);

    //main draw function.
    void Draw();

  protected:
    //the 14 buttons.
    CMenuSub *sub_menu[DKMB_NUM_BUTTONS];

    //the button that is currently selected.
    dk_main_button selection;

    //does whatever processing is required to get the given button out of selected mode
    void CheckUnselect(dk_main_button button);

    //renders the 3d stuff.
    void RenderView();

    //puts the different 3-d entities in the scene.
    void PlaceButtons();
    
    //selects one of our sub-menus.
    void Select(dk_main_button button);

    //array of entities that will be returned by NewEntity.
    entity_t entities[128];
    int32 num_entities;

    //true if we are closing the interface.
    bool shutting_down;
    //the time spent in our shutdown sequence.
    int32 shutdown_time;
    //a string we will write to the console upon leaving.
    CBuffer<2048> console_shutdown_string;

  public:
    //sets the console shutdown string.
    void ShutdownString(const char *string);

    //starts the shutdown sequence.
    void Shutdown();

    //returns a pointer to an entity structure that will be drawn.
    entity_t *NewEntity();

    //enters the specified sub-menu.
    void Enter(dk_main_button button);

    virtual void Enter();
    virtual void Exit();
    virtual void Animate(int32 elapsed_time);
    virtual void PlaceEntities();
    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    virtual bool MouseUp(int32 norm_x, int32 norm_y);
    virtual bool Keydown(int32 key);
    virtual bool Keyup(int32 key);

    void ShuttingDown(int32 elapsed_time);

	bool init_done;
};

//base class for all menus off of the sub-menu
class CMenuSub : public CMenu {
  public:
    CMenuSub();

    //vars we need to keep track of the animations for the button.
    dk_button_status status;

    //the state that the animation should eventually go.
    dk_button_status dest_status;

    //the amount of time we have spent in this state.
    int32 time;

    bool enabled;

    //returns the correct frame to display for the current time and status
    int32 Frame();
};

//classes for the submenus.
class CMenuSubQuit : public CMenuSub {
  public:
    CMenuSubQuit();
    virtual void Enter();
    virtual void Animate(int32 elapsed_time);
    virtual void DrawForeground();
    virtual void DrawBackground();
    virtual void PlaceEntities();
    virtual bool Keydown(int32 key);
    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    virtual bool MouseUp(int32 norm_x, int32 norm_y);

  protected:
    //our yes and no buttons.
    CInterfaceButton		yes;
    CInterfaceButton		no;

	void					GetQuitMessage(buffer128 &buf1, buffer128 &buf2, buffer128 &buf3);
	int						message;
	short					rare;
};

/*
class CMenuSubHelp : public CMenuSub {
  public:
    CMenuSubHelp();
    virtual void DrawForeground();
    virtual bool Keydown(int32 key);
};
*/

class CMenuSubResume : public CMenuSub
{
  public:
    CMenuSubResume();
    virtual void Enter();
};



class CMenuSubDemos : public CMenuSub
{
  public:
    CMenuSubDemos();
    virtual void Enter();
    virtual void Animate(int32 elapsed_time);
    virtual void DrawForeground();
    virtual void DrawBackground();
    virtual void PlaceEntities();
    virtual bool Keydown(int32 key);
    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    virtual bool MouseUp(int32 norm_x, int32 norm_y);
//  protected:
};

// cek[12-13-99] implementation in dk_menu_demos for now...
class CMenuSubCredits : public CMenuSub
{
public:
    CMenuSubCredits();
    virtual void Enter();
	virtual void Animate(int32 elapsed_time);
    virtual void DrawForeground();
    virtual void DrawBackground();
    virtual void PlaceEntities();
    virtual bool Keydown(int32 key);
    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    virtual bool MouseUp(int32 norm_x, int32 norm_y);
protected:
	int32					msTime;

	CInterfaceConfirmDlg	confirm_dlg;
	CInterfaceButton		planetDK_button;
	bool					ConfirmFinish();
};

// cek[12-3-99] this is actually the config loader
class CMenuSubAdvanced : public CMenuSub
{
public:
    CMenuSubAdvanced();
    virtual void Enter();
	virtual void CMenuSubAdvanced::Animate(int32 elapsed_time);
    virtual void PlaceEntities();
    virtual void DrawForeground();
    virtual void DrawBackground();
    virtual bool Keydown(int32 key);
    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    virtual bool MouseUp(int32 norm_x, int32 norm_y);

protected:
	bool					RetrieveData();
	void					DrawFileLines();
	void					DoLoad();
	void					DoSave();
	void					DoDelete(short type = DEL_START);
	enum					{DEL_START,DEL_ABORT,DEL_DEL};

	CInterfaceFileBox		files;

	CInterfaceInputField	file_field;

    CInterfaceLine			separator;
    CInterfaceButton		load;
    CInterfaceButton		save;
	CInterfaceButton		delBtn;

	bool					ConfirmFinish();
	CInterfaceConfirmDlg	confirm_dlg;
	CInterfaceStatusText	status;

	cvar_t					*cl_curconfig;
	int32					curSelection;
	char					cfg_file[MAX_OSPATH];
};


class CMenuSubOptions : public CMenuSub {
  public:
    CMenuSubOptions();
    virtual void Enter();
    virtual void Animate(int32 elapsed_time);
    virtual void DrawForeground();
    virtual void DrawBackground();
    virtual void PlaceEntities();
    virtual bool Keydown(int32 key);
    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    virtual bool MouseUp(int32 norm_x, int32 norm_y);
  protected:
 
    void DrawStatusBarPic(void);

    //radio buttons for the different on/off options.
    CInterfaceButtonRadio always_run_radio;
    CInterfaceButtonRadio lookspring_radio;
    CInterfaceButtonRadio lookstrafe_radio;
    CInterfaceButtonRadio auto_target_radio;
    CInterfaceButtonRadio weapon_visible;
	CInterfaceButtonRadio subtitles_radio;
	CInterfaceButtonRadio no_alttab_radio;
	CInterfaceButtonRadio weapon_bob_radio;
	CInterfaceButtonRadio weapon_autoswitch_radio;
	CInterfaceButtonRadio play_intro_radio;
	CInterfaceButtonRadio bloody_game_radio;
	CInterfaceButtonRadio dm_taunts_radio;
	CInterfaceButtonRadio unlimited_saves;
	CInterfaceButtonRadio dm_help;


    //picker for crosshair graphic
    CInterfacePicker crosshair_picker;
	void DrawCrosshair();

    CInterfaceSlider sbar_alpha;  // slider for setting alpha on status bar
    bool mouseIsDown;

    //picker for episode for weapon ordering
    //CInterfacePicker weapon_episode_picker;
};

class CMenuSubJoystick : public CMenuSub {
public:
    CMenuSubJoystick();
    virtual void Enter();
    virtual void Animate(int32 elapsed_time);
    virtual void DrawBackground();
    virtual void DrawForeground();
    virtual void PlaceEntities();
    virtual bool Keydown(int32 key);
    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    virtual bool MouseUp(int32 norm_x, int32 norm_y);

protected:
	void PositionControls();
	int32 ConfiguringAxisNum();
	unsigned int BoundAdvAxisFlags();

	void LoadAdvBindings();
	void SaveAdvBindings();

	CInterfacePicker		config_axis;
    CInterfaceButtonRadio	advanced_radio;
    CInterfaceButtonRadio	absolute_radio;
	bool advAbsolute;

	// advanced cvars
	cvar_t *joy_advanced;
	cvar_t *joy_advaxisx;
	cvar_t *joy_advaxisy;
	cvar_t *joy_advaxisz;
	cvar_t *joy_advaxisr;
	cvar_t *joy_advaxisu;
	cvar_t *joy_advaxisv;

	cvar_t *joy_forwardthreshold;	
	cvar_t *joy_forwardsensitivity;	

	cvar_t *joy_sidethreshold;		
	cvar_t *joy_sidesensitivity;	

	cvar_t *joy_upthreshold;  		
	cvar_t *joy_upsensitivity;

	cvar_t *joy_pitchthreshold;		
	cvar_t *joy_pitchsensitivity;

	cvar_t *joy_yawthreshold;
	cvar_t *joy_yawsensitivity;		

	void TopIndex(int32 index);
	void PositionCommandHilight(int32 index);
	void SelectCommand(int32 index);
	const char *FindBinding();
	void SetThreshold();
	void SetSensitivity();
	void SetSliderVals();

	int32 top_index;
	int32 num_command_items;

	buffer64 joy_binding;
	int32 selection;

    //box for command list
    CInterfaceBox command_box;
    //box for command list hilight
    CInterfaceBox command_hilight_box;
    //up and down buttons for command box.
    CInterfaceButton command_up;
    CInterfaceButton command_down;

	CInterfaceButtonRadio use_joystick;
	CInterfacePicker pick_axis;

	CInterfaceSlider threshold_slider;
	CInterfaceSlider sensitivity_slider;

    int32 HitCommand(int32 norm_x, int32 norm_y);
    int32 command_hilight;
    hilight_delay_control command_list_delay;

	cvar_t *in_initjoy;
	cvar_t *in_joystick;
};

class CMenuSubKeyboard : public CMenuSub {
  public:
    CMenuSubKeyboard();
    ~CMenuSubKeyboard();
    virtual void Enter();
    virtual void Animate(int32 elapsed_time);
    virtual void DrawBackground();
    virtual void DrawForeground();
    virtual void PlaceEntities();
    virtual bool Keydown(int32 key);
    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    virtual bool MouseUp(int32 norm_x, int32 norm_y);

  protected:
	bool ConfirmFinish();
	CInterfaceConfirmDlg	confirm_dlg;
    //our "restore defaults" button.
    CInterfaceButton defaults;
    //the up and down buttons for the listbox.
    CInterfaceButton listbox_up;
    CInterfaceButton listbox_down;

	CInterfaceScroller scroller;

    //the border around the listbox.
    //CInterfaceBox border;   // removed  1.12 dsn

    //border around the hilighted item
    CInterfaceBox hilight_box;

    //the separator between the keyboard names and binds
    CInterfaceLine separator;

    //the item at the top of the list of controls.
    int32 top_index;

    //sets the index of the item that should show at the top of the visible portion of the listbox.
    void TopIndex(int32 num);

    //starts countdown until the given row and column is hilighted.
    void StartHilightCountdown(int32 row, int32 column);
    void AbortHilightCountdown();

    //var to keep track of the hilight delay.
    hilight_delay_control hilight_delay;

    //puts the hilight box over the specified row/column
    void Hilight(int32 row, int32 column);
    //sizes the hilight box.
    void HilightSize(int32 time);

    //true if we have something hilighted.
    bool hilight_showing;
    //the row/column that is hilighted.
    int32 hilight_row, hilight_col;

    //starts/aborts/completes the control assignment process
    void AssignStart();
    void AssignAbort();
    void AssignComplete(int32 key);
    void AssignDelete();

    //true if we are setting a control, waiting for the user to press a key.
    bool assigning_control;
    //time elapsed in our hilight box's animation.
    int32 hilight_animation_time;
};

/*
//each mouse action has a name.
typedef enum {
    //MA_X_AXIS,
    //MA_Y_AXIS,
    MA_BUTTON1,
    MA_BUTTON2,
    MA_BUTTON3,

    MA_NUM_ACTIONS,
    MA_NONE,
} mouse_action;
*/

typedef enum
{
	MA_BUTTON1,
	MA_BUTTON2,
	MA_BUTTON3,
	MA_MWHEELUP,
	MA_MWHEELDN,
	
	MA_NUM_ACTIONS,
	MA_NONE
} mouse_action;


class CMenuSubMouse : public CMenuSub {
  public:
    CMenuSubMouse();
    virtual void Enter();
    virtual void Animate(int32 elapsed_time);
    virtual void DrawBackground();
    virtual void DrawForeground();
    virtual void PlaceEntities();
    virtual bool Keydown(int32 key);
    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    virtual bool MouseUp(int32 norm_x, int32 norm_y);

  protected:
    //box for the mouse actions.
    CInterfaceBox action_box;
    //selection box for the action box.
    CInterfaceBox action_hilight_box;

    //box for command list
    CInterfaceBox command_box;
    //box for command list hilight
    CInterfaceBox command_hilight_box;
    //up and down buttons for command box.
    CInterfaceButton command_up;
    CInterfaceButton command_down;

    //the checkbox for reversing mouse axis
    CInterfaceButtonRadio reverse_axis;
    CInterfaceButtonRadio mouse_look;
    CInterfaceButtonRadio look_strafe;
	CInterfaceButtonRadio mouse_filter;
    
    //the sensitivity sliders
    CInterfaceSlider sensitivityX;
    CInterfaceSlider sensitivityY;
    CInterfaceSlider speed;

    //the strings we will display to indicate what the mouse actions are bound to.
    //buffer64 mouse_bindings[5];
    buffer64 mouse_bindings[MA_NUM_ACTIONS];

    void FindBindings();

    //returns the action that is under the given x-y coordinate.
    mouse_action HitAction(int32 norm_x, int32 norm_y);
    //returns the line in the command list under the given coordinate, or -1 if the coordinate is off the list.
    int32 HitCommand(int32 norm_x, int32 norm_y);

    //puts the hilight box around the given item.
    void PositionActionHilight(int32 index);
    void PositionCommandHilight(int32 index);

    //the item that we clicked on and selected.
    mouse_action selection;

    //the item that the cursor is over.
    mouse_action action_hilight;
    
    //the command that the cursor is over.
    int32 command_hilight;

    //delay controls for the action and command lists.
    hilight_delay_control action_list_delay;
    hilight_delay_control command_list_delay;

    //the index that is at the top of the command list.
    int32 top_index;

    //the number of items that are currently in the command list.
    int32 num_command_items;

    //selects the given action.
    void SelectAction(mouse_action action);

    //selects the command at the given index.
    void SelectCommand(int32 index);

    //makes the command list display the given index at the top.
    void TopIndex(int32 index);
};

typedef enum {
//    VM_SOFTWARE,
    VM_OPENGL,
//    VM_GL_3DFX,
//    VM_GL_DEFAULT,
//    VM_GL_POWERVR,

    VM_NUM_MODES
} video_mode;

//#define gl_mode(mode) (mode == VM_GL_3DFX || mode == VM_GL_DEFAULT || mode == VM_GL_POWERVR)
#define gl_mode(mode) (mode == VM_OPENGL )

class CMenuSubVideo : public CMenuSub {
  public:
    CMenuSubVideo();
    virtual void Enter();
    virtual void Exit();
    virtual void Animate(int32 elapsed_time);
    virtual void DrawBackground();
    virtual void DrawForeground();
    virtual void PlaceEntities();
    virtual bool Keydown(int32 key);
    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    virtual bool MouseUp(int32 norm_x, int32 norm_y);

  protected:
    //the settings when we entered the menu.
    video_mode	start_mode;
    int			start_driver;
    int32		start_res;
    bool		start_fullscreen;
    int32		start_gl_shadows;
    int32		start_gl_quality;
	int32		start_tessellate_quality;
    float		start_gl_gamma;

    //modifies the refresh settings variables.
    void ApplyChanges();

    //the apply changes button.
    CInterfaceButton apply_changes;
    //returns true if we should display the apply changes button.
    bool ShowApplyChanges();

    //the current video mode.
    video_mode cur_mode;

    // SCG[6/28/99]: the current video driver.
	int cur_driver;

    //true if we have the mode string hilighted.
    bool mode_hilighted;

    // SCG[6/28/99]: true if we have the driver string hilighted.
    bool driver_hilighted;

    //a box to use to hilight the mode.
    CInterfaceBox mode_hilight;

    // SCG[6/28/99]: a box to use to hilight the driver.
    CInterfaceBox driver_hilight;

    //determines the current video mode by examining system variables.
    void GetMode();

    //determines the current video driver
    void GetDriver();

    // SCG[6/28/99]: true if we are picking a driver
	bool picking_driver;
    // SCG[6/28/99]: a box to surround the list of video drivers
    CInterfaceBox driver_list_box;
    // SCG[6/28/99]: hilight box for the driver list.
    CInterfaceBox driver_list_hilight_box;
    // SCG[6/28/99]: moves the hilight box to the given line.
    void DriverListHilight(int32 line);
    // SCG[6/28/99]: the driver that is hilighted in the list. -1 means none.
    int32 driver_list_hilight;

    //true if we are picking a mode.
    bool picking_mode;
    //a box to surround the list of video modes
    CInterfaceBox mode_list_box;
    //hilight box for the mode list.
    CInterfaceBox mode_list_hilight_box;
    //moves the hilight box to the given line.
    void ModeListHilight(int32 line);
    //the mode that is hilighted in the list. -1 means none.
    int32 mode_list_hilight;

    //the separator between the driver name and the options.
    CInterfaceLine separator;

    //makes the menu draw a new mode, but doesnt actually change any system settings.
    void SelectMode(video_mode mode);

    //slider to pick the resolution.
    CInterfaceSlider resolution_slider;

    //slider to pick the screen size.
    CInterfaceSlider screen_size_slider;

    //slider to pick the brightness
    CInterfaceSlider brightness_slider;

    //controls for the software mode
    //CInterfaceButtonRadio soft_fullscreen;
    //CInterfaceButtonRadio soft_stipple_alpha;

    //controls for gl modes.
    CInterfaceSlider gl_texture_slider;

    CInterfaceSlider gl_tessellation_slider;

    CInterfaceButtonRadio btn_shadows;
	CInterfaceButtonRadio btn_envMap;
    CInterfaceButtonRadio btn_fullscreen;
    CInterfaceButtonRadio btn_bulletholes;
	CInterfaceButtonRadio btn_showFog;
	CInterfaceButtonRadio btn_snow_rain;

	CInterfacePicker picker_modulate;

    //values for the buttons.
    bool fullscreen;
    int32 gl_shadows;
};

class CMenuSubSound : public CMenuSub {
  public:
    CMenuSubSound();
	~CMenuSubSound();
    virtual void Enter();
    virtual void Exit();
    virtual void Animate(int32 elapsed_time);
    virtual void DrawBackground();
    virtual void DrawForeground();
    virtual void PlaceEntities();
    virtual bool Keydown(int32 key);
    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    virtual bool MouseUp(int32 norm_x, int32 norm_y);

  protected:
    CInterfaceSlider sfx_volume_slider;
    CInterfaceSlider music_volume_slider;
    CInterfaceSlider cine_volume_slider;
	CInterfaceButtonRadio radio3d;

    void SetSFXVolumeFromSlider(int32 norm_x);
    void SetMusicVolumeFromSlider();
	void SetCineVolumeFromSlider();

	int32 sfx_volume_old;	// store previous mouse x to check sound fx volume change
	int32 cine_volume_preview_time;
};

#define GAME_STAT_HEALTH		0
#define GAME_STAT_ARMOR			1
#define GAME_STAT_LEVEL			2

#define GAME_INFO_CAPTION		0
#define GAME_INFO_HIRO			1
#define GAME_INFO_SUPERFLY		2
#define GAME_INFO_MIKIKO		3

#define MAX_NAME_LEN			10
#define MAX_STAT_LEN			8
#define MAX_TITLE				35
typedef struct
{
	char		name[MAX_NAME_LEN];
	char		stats[3][MAX_STAT_LEN];
} game_info_t;

/*
class CMenuSubSavegame : public CMenuSub
{
public:
	CMenuSubSavegame();
	virtual void Enter();
	virtual void Animate(int32 elapsed_time);
	virtual void DrawForeground();
	virtual void DrawBackground();
	virtual void PlaceEntities();
	virtual bool Keydown(int32 key);
	virtual bool MousePos(int32 norm_x, int32 norm_y);
	virtual bool MouseDown(int32 norm_x, int32 norm_y);
	virtual bool MouseUp(int32 norm_x, int32 norm_y);
protected:

	CInterfaceLine		separator;
	CInterfaceBox		screen_shot_box;
	CInterfaceButton	save_button;
};

*/
class CMenuSubSavegame : public CMenuSub
{
public:
    CMenuSubSavegame();
    virtual void Enter();
    virtual void Exit();
    virtual void Animate(int32 elapsed_time);
    virtual void DrawForeground();
    virtual void DrawBackground();
    virtual void PlaceEntities();
    virtual bool Keydown(int32 key);
    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    virtual bool MouseUp(int32 norm_x, int32 norm_y);

protected:
	CInterfaceStatusText	status;
	CInterfaceFileBox		files;
	CInterfaceInputField	file_field;
	CInterfaceLine			separator;
	CInterfaceLine			separator2;
	CInterfaceBox			screen_shot_box;
	CInterfaceButton		save;
	bool					RetrieveData();
	int32					curSelection;

	game_info_t				game_info[4];

	int						monsters;
	int						tmonsters;

	int						secrets;
	int						tsecrets;

	int						episode;
	char					title[MAX_TITLE];
	void					FillGameInfo();
	char					save_file[MAX_OSPATH];
};


class CMenuSubLoadgame : public CMenuSub
{
public:
    CMenuSubLoadgame();
    virtual void Enter();
    virtual void Animate(int32 elapsed_time);
    virtual void DrawForeground();
    virtual void DrawBackground();
    virtual void PlaceEntities();
    virtual bool Keydown(int32 key);
    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    virtual bool MouseUp(int32 norm_x, int32 norm_y);

protected:
	void					GetPic();

	CInterfaceFileBox		files;
	CInterfaceLine			separator;
	CInterfaceLine			separator2;
	CInterfaceBox			screen_shot_box;
	CInterfaceButton		load;
	DRAWSTRUCT				picDrawStruct;

	game_info_t				game_info[4];
	int						monsters;
	int						tmonsters;

	int						secrets;
	int						tsecrets;

	int					time;
	int					total;

	int						episode;
	char					title[MAX_TITLE];
	void					FillGameInfo();
	void					DoLoad();

	bool					RetrieveData();
	char					cur_file[MAX_OSPATH];
	int32					curSelection;
	bool					bCoop;
};

// weapon selection menu screen.  used from within the multiplayer menu.
typedef struct weaponStruct_s
{
	CInterfaceButtonRadio	button;
	unsigned int			flag;
} weaponStruct_t;

class CMenuSubWeapons : public CMenuSub
{
public:
    CMenuSubWeapons();
    virtual void Enter();
	virtual void Leave();
    virtual void Animate(int32 elapsed_time);
    virtual void DrawForeground();
    virtual void DrawBackground();
    virtual void PlaceEntities();
    virtual bool Keydown(int32 key);
    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    virtual bool MouseUp(int32 norm_x, int32 norm_y);

	void Activate(char *cvar);
	bool Active();
protected:
	char					*weapCvar;
	bool					bActive;
	weaponStruct_t			weapons[24];
	CInterfaceButton		back;
    CInterfaceLine			separator;                    // separator line

	void InitWeaponBtn(int episode, int num, unsigned int flag, const char *str);
};

//the different modes of the multiplayer menu.
typedef enum
{
  MM_JOIN = 0,
  MM_HOST,
  MM_SETUP
} multiplayer_mode;


typedef enum
{
  GAMECONNECT_NONE = 0,
  GAMECONNECT_LAN,
  GAMECONNECT_INTERNET
} gameconnect_type;


typedef enum {
    CS_288,
    CS_336,
    CS_ISDN,
    CS_DUALISDN,
    CS_T1,
    CS_CUSTOM
} connection_speed;

//an enumerated state list used to walk through the process of refreshing the local
//server list.
typedef enum {
    MJRS_NONE,
    MJRS_START,
    MJRS_DRAW_MESSAGE,
    MJRS_DO_PINGS
} multi_join_refresh_state;

class CMenuSubMultiplayer : public CMenuSub
{
public:
    CMenuSubMultiplayer();
	~CMenuSubMultiplayer();	// WAW[11/23/99]: Added for gamespy
    virtual void Enter();
	virtual void Exit();
    virtual void Animate(int32 elapsed_time);
    virtual void DrawBackground();
    virtual void DrawForeground();
    virtual void PlaceEntities();
    virtual bool Keydown(int32 key);
    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    virtual bool MouseUp(int32 norm_x, int32 norm_y);
	
	virtual void DrawModeJoin();	// WAW[11/23/99]: Added.

	virtual void DrawGameModeDeathmatch();
    virtual void DrawGameModeCTF();
    virtual void DrawGameModeDeathtag();
	virtual void DrawGameModeCooperative();
	virtual void PlaceEntitiesDeathmatch();
	virtual void PlaceEntitiesCTF();
	virtual void PlaceEntitiesDeathtag();
	virtual void PlaceEntitiesCooperative();
	virtual bool MousePosDeathmatch(int32 norm_x, int32 norm_y);
	virtual bool MousePosCTF(int32 norm_x, int32 norm_y);
	virtual bool MousePosDeathtag(int32 norm_x, int32 norm_y);
	virtual bool MousePosCooperative(int32 norm_x, int32 norm_y);
	virtual bool MouseDownDeathmatch(int32 norm_x, int32 norm_y);
	virtual bool MouseDownCTF(int32 norm_x, int32 norm_y);
	virtual bool MouseDownDeathtag(int32 norm_x, int32 norm_y);
	virtual bool MouseDownCooperative(int32 norm_x, int32 norm_y);
	virtual bool MouseUpDeathmatch(int32 norm_x, int32 norm_y);
	virtual bool MouseUpCTF(int32 norm_x, int32 norm_y);
	virtual bool MouseUpDeathtag(int32 norm_x, int32 norm_y);
	virtual bool MouseUpCooperative(int32 norm_x, int32 norm_y);

  protected:
	CInterfaceButtonRadio	instagib_radio;
	CInterfaceButton		BtnConfigureWeapons;
	CMenuSubWeapons			weapons;
	int32					lastClick;

	bool					DoJoin();
	char					manual_server[64];
	void					DoRefresh();
	DRAWSTRUCT				picDrawStruct;
	DRAWSTRUCT				picGamespy;
    multiplayer_mode current_mode;               // current subscreen 
	multiplayer_mode selected_mode;				// WAW[11/24/99]: Hold for the mode when goes back to setup.
    gameconnect_type gameconnect;                // type of game connect

    CInterfaceButton LAN_button;                 // LAN connection button
    CInterfaceButton internet_button;            // internet connection button

    CInterfaceButton join_server_button;         // join multiplayer game button.
    CInterfaceButton host_server_button;         // host multiplayer game button.
    CInterfaceButton character_setup_button;     // the player Setup button


	bool					ConfirmFinish();
	CInterfaceConfirmDlg	confirm_dlg;
    CInterfaceButton mplayer_start_button;     
    CInterfaceButton mplayer_community_button;
	int mplayer_exec_mode;

    void SetCurrentMode(multiplayer_mode mode);  //sets the current mode.

    CInterfaceLine separator;                    // separator line
    
    buffer256 player_name;                       // current setting for the player name.
    CInterfaceInputField player_name_field;      // player name input field.
    
    buffer256 team_name;                         // current setting for the player name.
    CInterfaceInputField team_name_field;        // player name input field.

    connection_speed current_connection_speed;   // current connection speed.
    CInterfacePicker connect_speed_picker;       // connection rate 

    //  connection speed radio buttons
    CInterfaceButtonRadio connect_speed_288;
    CInterfaceButtonRadio connect_speed_336;
    CInterfaceButtonRadio connect_speed_ISDN;
    CInterfaceButtonRadio connect_speed_DualISDN;
    CInterfaceButtonRadio connect_speed_T1;
    CInterfaceButtonRadio connect_speed_custom;

    void SetPlayerName(const char *name);        // sets the player name 
    void SetTeamName(const char *name);          // sets the team name

    void load_character_model(void);                    // load/init model
    void register_character_model(bool bResetRotation); // set model/skin names

    void * character_model;
    void * character_skin;
    CInterfacePicker modelname_picker; // to select model type
    CInterfacePicker skincolor_picker; // to select skin color

    int32 character_model_timer;
    int character_model_frame;
    int character_model_frame_start;
    int character_model_frame_end;
    float character_model_rotation;

	CInterfaceButton button_find_internet;		// SCG[12/1/99]: 
	CInterfaceButton button_find_local;			// SCG[12/1/99]:	
	CInterfaceButton button_start_multiplayer;	// SCG[12/1/99]: 
    
    //
    //controls used in the start game screen.
    //
    CInterfacePicker game_mode_picker;

    buffer256 server_name;
    CInterfaceInputField server_name_field;

    CInterfaceButton go_button;  // button used in the start menu to actually begin gameplay. 
	CInterfaceButton hostback_button;	// WAW[12/3/99]: Go back to the setup screen.
	CInterfaceButton joinback_button;	// WAW[12/3/99]: Go back to the setup screen.

	CInterfacePicker team_choice_picker;
	CInterfacePicker join_team_choice_picker;

	CInterfacePicker team1_color_picker;
    CInterfacePicker team2_color_picker;
	bool VerifyTeamColorPickers(CInterfacePicker& changing, CInterfacePicker &other, int old_val);

    CInterfacePicker ctf_limit_picker;
    CInterfacePicker score_limit_picker;
    CInterfacePicker frag_limit_picker;
    CInterfacePicker time_limit_picker;
    CInterfacePicker coop_time_limit_picker;
    CInterfacePicker coop_character_color_picker;
    CInterfacePicker coop_episode_picker;
    CInterfacePicker max_players_picker;
    CInterfacePicker level_picker;
    CInterfacePicker level_limit_picker;
    CInterfacePicker skill_level_picker;
    CInterfacePicker episode_picker;
	array<buffer64>	 episode_map_names;

    CInterfaceButtonRadio weapon_stay_radio;
    CInterfaceButtonRadio item_respawn_radio;
    CInterfaceButtonRadio spawn_farthest_radio;
    CInterfaceButtonRadio allow_exiting_radio;
    CInterfaceButtonRadio same_map_radio;
    CInterfaceButtonRadio force_respawn_radio;
    CInterfaceButtonRadio falling_damage_radio;
    CInterfaceButtonRadio allow_powerups_radio;
    CInterfaceButtonRadio allow_health_radio;
    CInterfaceButtonRadio allow_armor_radio;
    CInterfaceButtonRadio infinite_ammo_radio;
    CInterfaceButtonRadio fast_switch_radio;
    CInterfaceButtonRadio footsteps_radio;
    CInterfaceButtonRadio shotshells_radio;
	CInterfaceButtonRadio skill_system_radio;

//	CInterfaceButtonRadio weapon_allow[6];

    CInterfaceButtonRadio teamplay_radio;
    CInterfaceButtonRadio friendly_fire_radio;

//	CInterfaceButtonRadio allow_hook_radio;
//	CInterfaceButtonRadio instant_powerups_radio;
//	CInterfaceButtonRadio fixed_fov_radio;

    //a control array to hold all of our radio buttons.
    CInterfaceControlArray flag_control_array;

	// SCG[10/29/99]: End deathmatch controls

	// SCG[10/29/99]: Begin deathtag controls
	// SCG[10/29/99]: End deathtag controls

	// SCG[10/29/99]: Begin coop controls
    CInterfaceButton coop_character_setup_button;
    CInterfacePicker coop_level_picker;
	// SCG[10/29/99]: End coop controls

    
    void FillLevelPicker(int32 episode_num);        // fill in the level name picker
    void SetServerName(const char *name);           // sets the server name.
    void StartServer();                             // starts the server with the current settings.

    const char *GetCurrentMapName();                // gets the map name of level/episode

    //controls used in the join game screen.

    CInterfaceInputField join_name_field;           // ip join input field
    buffer256 join_name_string;                     // string displayed in join field.

    CInterfaceButton play_button;                   // play button
    //CInterfaceButton add_button;                  // add button for address book
    CInterfaceButton refresh_button;                // refresh button for server list

    //buttons to use to scroll the local games list.
    CInterfaceButton local_games_up_button;
    CInterfaceButton local_games_down_button;
    
    CInterfaceBox local_games_border;               // border box around server list

    //the local game that is displayed at the top of the list of local games.
    int32 local_list_top_index;
    int32 local_list_hilight_index;
    //WAW[11/24/99] buttons to use to scroll the internet games list.
    int32 net_list_top_index;
    int32 net_list_hilight_index;

    void SetJoinString(const char *str);
    
    multi_join_refresh_state refresh_state;         // track state during the refresh process.

	void fillEpisodePicker( const char *gametype ); // WAW[11/19/99]: Added to read out of the "multiplayermaps.csv" file
	void setGameMode();
	void FillDeathmatchInfo(bool bSetVars = false);
	void FillCTFInfo(bool bSetVars = false);
	void FillDeathtagInfo(bool bSetVars = false);
	void FillCoopInfo(bool bSetVars = false);

	// WAW[11/23/99]: Added support for GAMESPY
	// ========================================
	enum serverliststate_e
	{
		SERVERLISTSTATE_NONE,
		SERVERLISTSTATE_IDLE,
		SERVERLISTSTATE_STARTED,
		SERVERLISTSTATE_LISTXFER,
		SERVERLISTSTATE_LANLIST,
		SERVERLISTSTATE_QUERYING
	};
	serverliststate_e		serverlist_state;
    GServerList				serverlist;	
	int						serverlist_progress;
	int						buttonmove_timer;
	void SetJoinListButtons();
	int MaxJoinLines();
	void MoveJoinDown();
	void MoveJoinUp();

	int GetListTopIndex() { return (gameconnect == GAMECONNECT_INTERNET ? net_list_top_index:local_list_top_index); }
	void SetListTopIndex(int val) 
	{ 
		if (gameconnect == GAMECONNECT_INTERNET) net_list_top_index = val;
		else local_list_top_index = val;
	}
	
	int GetListHilightIndex() { return (gameconnect == GAMECONNECT_INTERNET ? net_list_hilight_index:local_list_hilight_index); }
	void SetListHilightIndex(int val) 
	{ 
		if (gameconnect == GAMECONNECT_INTERNET) net_list_hilight_index = val;
		else local_list_hilight_index = val;
	}

	void RefreshInternetList();	// WAW[12/3/99]: RefreshInternetList

	bool ctf_selected;
	void SetTeamChoicePicker();	// WAW[12/9/99]: for ctf_teamchoice.
	void SetTeamChoice( int val );

	// coop menu skill selection
    void DrawSkillPic(char * filename, int x, int y);
    CInterfaceButton start[12];
    CInterfaceButtonRadio skill_easy;
    CInterfaceButtonRadio skill_medium;
    CInterfaceButtonRadio skill_hard;
	int GameModeIndex();
public:
	void Commit();		// WAW[12/9/99]: Commit the changes to the CVars.
	void ServerListCallBack( int msg, void *param1, void *param2 );
	// ========================================
};




class CMenuSubNewgame : public CMenuSub {
  public:
    CMenuSubNewgame();
    virtual void Enter();
    virtual void Animate(int32 elapsed_time);
    virtual void DrawForeground();
    virtual void DrawBackground();
    virtual void PlaceEntities();
    virtual bool Keydown(int32 key);
    virtual bool MousePos(int32 norm_x, int32 norm_y);
    virtual bool MouseDown(int32 norm_x, int32 norm_y);
    virtual bool MouseUp(int32 norm_x, int32 norm_y);

  protected:
    void DrawSkillPic(char * filename, int x, int y);

    CInterfaceButton start[12];
};


#endif // _DK_MENUP_H_

