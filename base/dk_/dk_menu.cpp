#include <io.h>
#include "dk_shared.h"
#include "qfiles.h"
#include "qcommon.h"

#include "dk_misc.h"
#include "dk_buffer.h"
#include "dk_array.h"

#include "dk_menu.h"
#include "dk_menup.h"
//#include "dk_model.h"
#include "winquake.h"
#include "client.h"
#include "dk_bitmap.h"
#include "dk_beams.h"
#include "server.h"

//#include "l__language.h"





char * DKM_sounds[] =
{ 
    "menu_01",
    "menu_02",
	"menus/Enter Menu_001.wav",
	"menus/600ms rotate descend_001.wav",
	"menus/600ms rotate ascend_001.wav",
	//    "menus/playerchoke3.wav", no longer needed -KRH
	"menus/no_001.wav",
	"menus/button_003.wav",
	"menus/button_004.wav",
	"menus/button_005.wav",
	"menus/button_006.wav",
	"menus/button_007.wav",
	"menus/Exit Menu_001.wav"
};

#define VERSION_STR			"V. 1.2"
#define	VERSION_X			430
#define VERSION_Y			416

//we keep track of the mouse position.
static int32 dkm_mouse_x = 320;
static int32 dkm_mouse_y = 240;


static int32 animation_select_time;
static int32 animation_hilight_time;
static int32 shutdown_sequence_time;
static bool DKM_blasting;


//true if we are drawing the menu up on the screen.
static bool dkm_in_menu = false;

// default first submenu
static dk_main_button dkm_last_menu_in = DKMB_FIRST_MENU_IN;

int32 vid_menu_mode;	// resolution
float vid_brightness;	// brightness
int32 texture_quality;
int32 tesselation_quality;

bool is_fullscreen;
float use_shadows = false;	// shadows
short modulate;

// for tracking mode changes
int32 start_vid_menu_mode;
int32 start_texture_quality;
int32 start_tesselation_quality;
float start_vid_brightness;
float start_use_shadows;
//cek[3-14-00]: disablefloat start_use_3d;

bool start_is_fullscreen;
short start_modulate;

int32 start_gl_driver;
int32 current_gl_driver;

// local prototypes
bool UsedSideButton(int i);


//returns 1 if we are in the menu, 0 otherwise.
bool DKM_InMenu() {
    return dkm_in_menu;
}

static int32 cursor_offset_x = 6;
static int32 cursor_offset_y = 8;


static bool dkm_play_enter_sound = true;

//extern char	dk_userName[];
void CL_WriteConfiguration (char *name);
bool enable_save();

void PlayInterfaceSound(const char* file_name) 
{	
	S_StartLocalSound(file_name);
}


static interface_bmp_desc bitmaps[DKB_NUM] =
{
    {"pics/interface/cursor_01.tga", RESOURCE_INTERFACE},  
    {"pics/interface/cursor_02.tga", RESOURCE_INTERFACE}, 
    {"pics/interface/cursor_03.tga", RESOURCE_INTERFACE},
    {"pics/interface/cursor_04.tga", RESOURCE_INTERFACE},
    {"pics/interface/cursor_05.tga", RESOURCE_INTERFACE},
    {"pics/interface/cursor_06.tga", RESOURCE_INTERFACE},
    {"pics/interface/cursor_07.tga", RESOURCE_INTERFACE},
    {"pics/interface/cursor_08.tga", RESOURCE_INTERFACE},
    {"pics/interface/cursor_09.tga", RESOURCE_INTERFACE},
    {"pics/interface/cursor_10.tga", RESOURCE_INTERFACE},
	
    {"pics/interface/back00.bmp", RESOURCE_INTERFACE},
    {"pics/interface/back01.bmp", RESOURCE_INTERFACE},
    {"pics/interface/back02.bmp", RESOURCE_INTERFACE},
    {"pics/interface/back10.bmp", RESOURCE_INTERFACE},
    {"pics/interface/back11.bmp", RESOURCE_INTERFACE},
    {"pics/interface/back12.bmp", RESOURCE_INTERFACE}
};



//the interface palette.
static byte interface_palette[768];
//true if we have loaded the interface palette
bool palette_loaded = false;

//the models we use.
void *button_model = NULL;
void *box_model = NULL;

//the font.
void *menu_font = NULL;
//the bright font.
void *menu_font_bright = NULL;
//the button graphic font.
void *button_font = NULL;

//the main menu object.
static CMenuMain menu;

//the area of the screen covered by the buttons can be represented by a parallelogram.
//all are normalized to 640x480
//the left and right of the parallelogram.
static int32 norm_button_left = 506;
static int32 norm_button_right = 593;
//the top and bottom scanlines of the left vertical parallelogram edge.
static int32 norm_button_left_top = 76;
static int32 norm_button_left_bottom = 436;
//the top and bottom scanlines of the right vertical parallelogram edge.
static int32 norm_button_right_top = 55;
static int32 norm_button_right_bottom = 462;

//we keep track of elapsed time from frame to frame.
static int32 last_frame_time = 0;

#ifdef JPN
bool fDBCS = false;
int TrailByte;

void DbcsCharInput(int chCharCode1, int chCharCode2)
{
	if (chCharCode1) {
		fDBCS = true;
		TrailByte = chCharCode2;
		menu.Keydown(chCharCode1);
		fDBCS = false;
	}
	else
		menu.Keydown(chCharCode2);
}
#endif // JPN 
//when we first enter the menu, we need to initialize last_frame_time
static bool last_frame_time_init = false;


bool in_main_game(void)
// is the player in the main game?
{
	return (cls.state == ca_active);
}


//given a x coordinate, computes the top and bottom vertical scan that lies over the buttons.
//returns true if the x coordinate is within the parallelogram, false otherwise.
//the given x is in normalized 640 width, and top and bottom are returned for normalized 480 height.
bool DKM_ParallelogramColumn(int32 norm_x, int32 &norm_top, int32 &norm_bottom) {
    //check if the x is on the buttons.
    if (norm_x < norm_button_left) return false;
    if (norm_x > norm_button_right) return false;
	
    //get the slope of the top line.  static so it only gets computed once.
    static float slope_top = float(norm_button_right_top - norm_button_left_top) / float(norm_button_right - norm_button_left);
    //get the intercept of the top line. 
    static float int_top = norm_button_left_top - slope_top * norm_button_left;
	
    //get slope and intercept of the bottom line.
    static float slope_bottom = float(norm_button_right_bottom - norm_button_left_bottom) / float(norm_button_right - norm_button_left);
    static float int_bottom = norm_button_left_bottom - slope_bottom * norm_button_left;
	
    //get the y values for our x coordinate.
    norm_top = int32(slope_top * norm_x + int_top);
    norm_bottom = int32(slope_bottom * norm_x + int_bottom);
	
    return true;
}

//given a y coordinate and the top and bottom of a line, returns the button that the y value falls on.
//the given x and y are in screen coordinates, not normalized to 640x480
dk_main_button DKM_ButtonNum(int32 norm_x, int32 norm_y) {
    //get the top and bottom of the vertical scan at the given x.
    int32 norm_top, norm_bottom;
    if (DKM_ParallelogramColumn(norm_x, norm_top, norm_bottom) == false)
    {
        //we didnt hit a button.
        return DKMB_NONE;
    }
	
    //just divide the top to bottom span into as many pieces as we have buttons
    dk_main_button button = dk_main_button(int32(float(norm_y - norm_top) / float(norm_bottom - norm_top) * DKMB_NUM_BUTTONS));
	
// changed by yokoyama for Japanese version
#ifndef JPN
#ifdef DM_DEMO
	if (!UsedSideButton((int)button))
		return DKMB_NONE;          //we didnt hit a button.
#endif // DM_DEMO
#endif         
	
    //do some error checking.
    if (button < 0) return (dk_main_button)0;
    if (button >= DKMB_NUM_BUTTONS) 
		return (dk_main_button)(DKMB_NUM_BUTTONS - 1);
	
    return button;
}

void DKM_SetMenuPalette()
{
    re.SetInterfacePalette(&interface_palette[0]);
}


void DKM_PrecacheSounds(void)
// register the sounds before using
{
	// pre-cache menu sounds
//	S_BeginRegistration(); 
	
	for (int i = DKMS_ENTER;i < DMKS_NUMSOUNDS;i++)
	{
		S_RegisterSound(DKM_sounds[i]);
	}
	
//	S_EndRegistration(0);
}

void DKM_RegisterSounds(void)
{
	S_BeginRegistration(); 
	
	for (int i = DKMS_ENTER;i < DMKS_NUMSOUNDS;i++)
	{
		S_RegisterSound(DKM_sounds[i]);
	}
	
	S_EndRegistration(0);
}

void DKM_InitFonts( void )
{
    menu_font			= re.LoadFont("int_font");
    menu_font_bright	= re.LoadFont("int_font_bright");
    button_font			= re.LoadFont("int_buttons");
}

void FS_AddGameDirectory (char *dir);
float GetCurrentBrightness();

void DKM_LoadVideoSettings(bool setDefaults)
{
    cvar_t * temp_cvar;
    temp_cvar = Cvar_Get("gl_mode", "0", CVAR_ARCHIVE);
	/*start_vid_menu_mode = */vid_menu_mode = temp_cvar->value;

	for( int i = 0; i < gl_driver_info_count; i++ )
	{
		gl_driver_info_t driver = gl_driver_info.Item( i );
		if( !strcmp( driver.szFilename, Cvar_VariableString( "gl_driver" ) ) )
		{
			current_gl_driver = i;
			break;
		}
	}

	vid_brightness = GetCurrentBrightness();

    temp_cvar = Cvar_Get("gl_picmip", "0", CVAR_ARCHIVE);
	/*start_texture_quality = */texture_quality = temp_cvar->value;

//    temp_cvar = Cvar_Get("gl_subdivide_size", "0", CVAR_ARCHIVE);
//	/*start_tesselation_quality = */tesselation_quality = temp_cvar->value;//( 256 - int32(temp_cvar->value + 0.001f) ) + 64;

    temp_cvar = Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
	/*start_is_fullscreen = */is_fullscreen = (temp_cvar->value > 0.0f);

    temp_cvar = Cvar_Get("gl_modulate", "0", CVAR_ARCHIVE);
	/*start_modulate = */modulate = temp_cvar->value;

    temp_cvar = Cvar_Get("gl_shadows", "0", CVAR_ARCHIVE);
    /*start_use_shadows = */use_shadows = temp_cvar->value;

	if (setDefaults)
	{
		start_gl_driver = current_gl_driver;
		start_vid_menu_mode = vid_menu_mode;
		start_texture_quality = texture_quality;
		start_tesselation_quality = tesselation_quality;
		start_vid_brightness = vid_brightness;

		start_is_fullscreen = is_fullscreen;
		start_modulate = modulate;
		start_use_shadows = use_shadows;
	}
}
extern cvar_t      *vid_ref;           // Name of Refresh DLL loaded

bool DKM_VideoSettingsChanged()
{
	return (vid_menu_mode != start_vid_menu_mode) || 
		(texture_quality != start_texture_quality) ||
		(tesselation_quality != start_tesselation_quality) || 
		(modulate != start_modulate) ||
		(start_is_fullscreen != is_fullscreen) || 
		(fabs(start_vid_brightness - vid_brightness) > 0.001) ||
		(start_use_shadows != use_shadows) ||
		( start_gl_driver != current_gl_driver );
}

void DKM_UpdateVideoSettings()
{
	cvar_t *temp_cvar;

	// update the mode
    temp_cvar = Cvar_Get("gl_mode", "0", CVAR_ARCHIVE);
	Cvar_SetValue("gl_mode",vid_menu_mode);

	// update the brightness
    VID_SetGamma(vid_brightness);

	// update the texture quality
	Cvar_SetValue("gl_picmip",texture_quality);
    temp_cvar = Cvar_Get("gl_picmip", "0", CVAR_ARCHIVE);

	// update the fog quality
//	Cvar_SetValue("gl_subdivide_size",tesselation_quality);
//    temp_cvar = Cvar_Get("gl_subdivide_size", "0", CVAR_ARCHIVE);

	// update fullscreen/windowed
	Cvar_SetValue("vid_fullscreen",is_fullscreen);
    temp_cvar = Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);

	// update modulate
	Cvar_SetValue("gl_modulate",modulate);
    temp_cvar = Cvar_Get("gl_modulate", "0", CVAR_ARCHIVE);

	// shadows
    Cvar_SetValue("gl_shadows", use_shadows);

	vid_ref->modified = DKM_VideoSettingsChanged();

// SCG[5/26/00]: 
	gl_driver_info_t driver = gl_driver_info.Item( current_gl_driver );
    Cvar_Set( "gl_driver", driver.szFilename );
	
	// set currents as default
	DKM_LoadVideoSettings(true);
}

void DKM_Enter(dk_main_button force_menu)
// interface loads up here!
{
	if (force_menu != DKMB_FIRST_MENU_IN)  // override default menu?
	{
		menu.ResetSubMenu(dkm_last_menu_in); // reset all aspects of previous menu in
		
		dkm_last_menu_in = force_menu;
	}

	if( in_main_game() == true && ((Cvar_VariableValue("unlimited_saves") != 0) || (cl_savegem_count > 0)) )
	{
		re.GetSavegameScreenShot();
	}

/*
	cvar_t *savegamedir = Cvar_Get( "sv_savegamedir", 0, 0 );
	if( ( savegamedir != NULL ) && ( savegamedir->string != NULL ) )
	{
		if( savegamedir->modified == 1 )
		{
			FS_AddGameDirectory( savegamedir->string );
			savegamedir->modified = 0;
		}
	}
*/
	if (stricmp(FS_SaveGameDir(), FS_Gamedir()))
		FS_AddGameDirectory( FS_SaveGameDir() );

	// SCG[11/28/99]: make sure the game pauses when you enter the menu
	if (!deathmatch->value && !Cvar_VariableValue("coop"))
		Cvar_Set ("paused", "1");

	ShowCursor( FALSE );
	
	// mdm99.06.28 - background change
	// mdm99.07.14 - real music API added
	S_StopMusic(CHAN_MUSIC_MENU);
	
	if (S_StopAllSounds)    
		S_StopAllSounds () ;
	
    //check if we have loaded our palette.
    if (palette_loaded == false)
    {
        //load the palette.
        //try and read it out of one of the background bitmaps.
        if (LoadBitmapPalette("pics/interface/back00.bmp", interface_palette) == true) {
            //we got the palette.
            palette_loaded = true;
        }
        else
        {
            //set some default values into the palette.
            for (int32 i = 0; i < 256; i++) {
                interface_palette[i * 3 + 0] = i;
                interface_palette[i * 3 + 1] = i;
                interface_palette[i * 3 + 2] = i;
            }
        }
    }
	
    //set the palette.
	DKM_SetMenuPalette();
	
    //we are entering the menu.
    dkm_in_menu = true;
	
    dkm_mouse_x = 320;
    dkm_mouse_y = 240;
	
    // set animation timers
    animation_select_time  = ANIMATION_SELECT_TIME;
    animation_hilight_time = ANIMATION_HILIGHT_TIME;
    shutdown_sequence_time = SHUTDOWN_SEQUENCE_TIME;
	
    DKM_blasting = true; // no animation on first in
	
    //cache all of our bitmaps.
    for (int32 i = 0; i < DKB_NUM; i++) {
        re.RegisterPic(bitmaps[i].name, NULL, NULL, bitmaps[i].resource);
    }
	
    //register the button model.
    button_model = re.RegisterModel("models/interface/ib_button.dkm", RESOURCE_INTERFACE);
    box_model = re.RegisterModel("models/interface/iu_unitbox.dkm", RESOURCE_INTERFACE);
	
    //move the windows cursor to the middle of our window.
    //get the rectangle of the window.
    POINT tl, br;
    tl.x = tl.y = 0;
    br.x = viddef.width;
    br.y = viddef.height;
    ClientToScreen(cl_hwnd, &tl);
    ClientToScreen(cl_hwnd, &br);
	
    //set the cursor position.  add 320,240 to the position of our window.
    SetCursorPos(tl.x + viddef.width / 2 , tl.y + viddef.height / 2);
	
    //make the mouse confined to our window.
    RECT window_rect;
    window_rect.left = tl.x;
    window_rect.top = tl.y;
    window_rect.right = br.x;
    window_rect.bottom = br.y;
    ClipCursor(&window_rect);
	
    //tell the main menu that we are entering.
    // 1.10 dsn   moved below -- enter first menu AFTER the fonts/buttons have been loaded
    //menu.Enter();
	
    //get our fonts.
	DKM_InitFonts();
	
	// set up our video settings so we can change on exit
	DKM_LoadVideoSettings(true);
	
//cek[3-14-00]: disable	start_use_3d = Cvar_VariableValue("s_use3d");
    // turn off shadows
//    Cvar_SetValue("gl_shadows", 0.0f);

    if (in_main_game() == false)
		DKM_RegisterSounds();        // pre-cache only if not already in a map
	
    //tell the main menu that we are entering.
    menu.Enter();
	
    DKM_blasting = false; // done blasting in, rabbit
}

void CL_Snd_Restart_f (void);

void DKM_Leave() 
{
    //make sure we haven't already left the menu
    if (dkm_in_menu == false) return;
	
    //purge the interface resources.
    re.PurgeResources(RESOURCE_INTERFACE);
	
	// SCG[11/28/99]: make sure the game unpauses when you exit the menu
	Cvar_Set ("paused", "0");
    
	dkm_in_menu = false;

	// SCG[12/8/99]: Remove the savegame screenshot from memory if we got one.
	re.ClearSavegameScreenShot();

    //free the mouse.
    ClipCursor(NULL);
	
    //tell the main menu we are leaving.
    menu.Exit();
	
	// update the video settings
//	DKM_UpdateVideoSettings();

    // restore shadows flag
//    Cvar_SetValue("gl_shadows", use_shadows);
    //when we enter the menu again, reset our time tracking vars.
    last_frame_time_init = false;
	
    //set the palette.
    re.SetInterfacePalette(NULL);
    
    // -- make sure that any looping sounds are stopped...
	// mdm99.06.28 - background change
	// mdm99.07.14 - real music api added
	S_StopMusic(CHAN_MUSIC_MENU);
	if (S_StopAllSounds)
		S_StopAllSounds ();

//cek[3-14-00]: disable	
/*	if (start_use_3d != Cvar_VariableValue("s_use3d"))
	{
		CL_Snd_Restart_f ();
	}
*/
	// start up the map music
	S_StartMusic(NULL,CHAN_MUSIC_MAP);
	// SCG[11/21/99]: Write the configuration
	// cek[12-5-99]: Write to currentconfig cvar
	CL_WriteConfiguration("current");//Cvar_VariableString("currentconfig"));
//	CL_WriteConfiguration (dk_userName);
}

//called every frame with the amount the mouse moved since last frame.
void DKM_MouseMove(int32 mx, int32 my) {
    //add the displacements to our position.
    dkm_mouse_x += mx;
    dkm_mouse_y += my;
	
    //keep the cursor on the screen.
    bound_min(dkm_mouse_x, 0);
    bound_max(dkm_mouse_x, viddef.width - 1);
    bound_min(dkm_mouse_y, 0);
    bound_max(dkm_mouse_y, viddef.height - 1);
	
    //do mouse movement processing.
    DKM_NewMousePos();
}

void DKM_MousePos(int32 screen_x, int32 screen_y) {
    dkm_mouse_x = screen_x;
    dkm_mouse_y = screen_y;
	
    //keep the cursor on the screen.
    bound_min(dkm_mouse_x, 0);
    bound_max(dkm_mouse_x, viddef.width - 1);
    bound_min(dkm_mouse_y, 0);
    bound_max(dkm_mouse_y, viddef.height - 1);
	
    //do mouse movement processing.
    DKM_NewMousePos();
}

//the given allowed pixel width 
int32 DKM_FontNumCharsInWidth(const char *string, int32 norm_allowed_pixel_width, int32 start_pos) {
    if (string == NULL) return 0;
	
    if (menu_font != NULL) {
        //use our proportional font.
        return re.FontNumCharsInWidth(string, menu_font, norm_allowed_pixel_width, start_pos);
    }
	
    //get the length of the string.
    int32 len = strlen(string);
	
    //make sure the starting position isnt after the end of the string.
    if (start_pos >= len) return 0;
	
    //the maximum possible number of chars that will fit in the given width.
    int32 max_chars = norm_allowed_pixel_width / 8;
	
    //get the number of characters not counting ones before the starting position.
    int32 num_avail_chars = len - start_pos;
	
    //see if we have more available characters than will fit.
    if (num_avail_chars > max_chars) {
        //we have enough characters in the string to fill up the available width.
        return max_chars;
    }
	
    //we dont have enough characters to fill the available width;
    return num_avail_chars;
}

int32 DKM_StringWidth(const char *string, int32 max_chars_counted) {
    if (string == NULL) return 0;
	
    if (menu_font != NULL) {
        //use our proportional font.
        return re.StringWidth(string, menu_font, max_chars_counted);
    }
	
    //get the length of the string.
    int32 len = strlen(string);
	
    //make sure we dont count more than the maximum number of characters 
    if (max_chars_counted != -1) {
        bound_max(len, max_chars_counted);
    }
	
    //the width is the number of characters times the width, 8
    return len * 8;
}

void DKM_DrawString(int32 norm_x, int32 norm_y, const char *str, void *font, bool center, bool hilighted, float alpha) 
{
    if (str == NULL) return;
	
    //decide which font we will use.
    void *use_font;
	
    //if a font was passed in, use that.
    if (font != NULL) {
        use_font = font;
    }
    else {
        //use our menu font, but check if we are supposed to hilight it.
        use_font = (hilighted == false) ? menu_font : menu_font_bright;
    }
	
    //get the current x and y stretch factors.
    float norm_to_screen_x = viddef.width / 640.0f;
    float norm_to_screen_y = viddef.height / 480.0f;
	
    //get screen coordinates for the text.
    int32 screen_x = norm_to_screen_x * norm_x;
    int32 screen_y = norm_to_screen_y * norm_y;
	
    if (use_font != NULL) {
        //check if we need to center the text.
        if (center == true) {
            screen_x -= norm_to_screen_x * re.StringWidth(str, use_font, -1) / 2;
        }
		
		//        re.DrawString(str, screen_x, screen_y, use_font, norm_to_screen_x, norm_to_screen_y);
		//==================================
		// consolidation change: SCG 3-15-99
		DRAWSTRUCT drawStruct;
		drawStruct.nFlags = DSFLAG_SCALE | DSFLAG_BLEND;
		if (alpha)
		{
			drawStruct.fAlpha = alpha;
			drawStruct.nFlags |= DSFLAG_ALPHA;
		}
		drawStruct.szString = str;
		drawStruct.nXPos = screen_x;
		drawStruct.nYPos = screen_y;
		drawStruct.pImage = use_font;
		drawStruct.fScaleX = norm_to_screen_x;
		drawStruct.fScaleY = norm_to_screen_y;

		re.DrawString( drawStruct );
		// consolidation change: SCG 3-15-99
		//==================================
        return;
    }
	
    if (center == true) {
        screen_x -= 4 * strlen(str);
    }
	
    for (int32 pos = 0; *str != '\0'; str++, pos++) {
        //draw the character.
        re.DrawChar(screen_x, screen_y, str[0]);
        
        //move our x over.
        screen_x = int32((norm_x + pos * 8) * norm_to_screen_x);
    }
    
}

//given x and y are in normalized coordinates. max_pixel_width is not normalized.
void DKM_DrawString(int32 norm_x, int32 norm_y, const char *str, int32 norm_max_pixel_width, bool bright) {
    //get the width of the given string.
    int32 width = DKM_StringWidth(str, -1);
    
    //check if the width is greater than the space we have.
    if (width > norm_max_pixel_width) {
        //get the width of the "..." we will put at the end of the string.
        int32 dots_width = DKM_StringWidth("...", -1);
		
        //get the number of characters of the given string that will fit before the dots.
        int32 num_chars = DKM_FontNumCharsInWidth(str, norm_max_pixel_width - dots_width, 0);
        bound_max(num_chars, 250);
		
        //copy that many characters from the string to a new buffer.
        char final_str[256];
        strncpy(final_str, str, num_chars);
		
        //put the ... at the end.
        strcpy(&final_str[num_chars], "...");
		
        //draw the string.
        DKM_DrawString(norm_x, norm_y, final_str, NULL, false, bright);
    }
    else {
        //draw the whole string.
        DKM_DrawString(norm_x, norm_y, str, NULL, false, bright);
    }
    
}

//shuts down the menu.
void DKM_Shutdown()
{
    menu.Shutdown();
}

//sets a string that is output to the execute buffer when the menu is done shutting down.
void DKM_ShutdownString(const char *format, ...) {
    //get our parameter list.
    va_list args;
    va_start(args, format);
	
    //print to a buffer.
    char string[2048];
    _vsnprintf(string, 2048, format, args);
	
    menu.ShutdownString(string);
}

void DKM_EnterMain() {
    menu.Enter();
}

void DKM_Draw() {
    //get the elapsed time since last frame.
    int32 elapsed_time;
	
    if (last_frame_time_init == false) {
        //haven't initialized our previous time.
        last_frame_time_init = true;
		
        last_frame_time = GetTickCount();
        elapsed_time = 0;
    }
    else {
        //get the current time.
        int32 cur_time = GetTickCount();
		
        //subtract the current time from the time last frame.
        elapsed_time = cur_time - last_frame_time;
		
        bound_max(elapsed_time, 50);
		
        //remember the current time for next time.
        last_frame_time = cur_time;
    }
	
    //animate the menu.
    menu.Animate(elapsed_time);
	
    menu.Draw();
	
    menu.ShuttingDown(elapsed_time);
}

void DKM_Key(int32 key, qboolean down) {
    //check if this is a mouse message.
    if (key == K_MOUSE1) {
        if (down != false) {
            DKM_MouseDown();
        }
        else {
            DKM_MouseUp();
        }
        return;
    }
	
    if (down != false) {
        menu.Keydown(key);
    }
}

void DKM_NewMousePos() {
    //convert the current mouse position to normalized 640x480.
    int32 norm_x = int32(dkm_mouse_x * 640.0f / viddef.width);
    int32 norm_y = int32(dkm_mouse_y * 480.0f / viddef.height);
	
    menu.MousePos(norm_x, norm_y);
}

void DKM_MouseDown() {
    //convert the current mouse position to normalized 640x480.
    int32 norm_x = int32(dkm_mouse_x * 640.0f / viddef.width);
    int32 norm_y = int32(dkm_mouse_y * 480.0f / viddef.height);
	
    menu.MouseDown(norm_x, norm_y);
}

void DKM_MouseUp() {
    //convert the current mouse position to normalized 640x480.
    int32 norm_x = int32(dkm_mouse_x * 640.0f / viddef.width);
    int32 norm_y = int32(dkm_mouse_y * 480.0f / viddef.height);
	
    menu.MouseUp(norm_x, norm_y);
}

void DKM_GameShutdown()
{
	menu.GameShutdown();
}

void DKM_Init()
{
	menu.Init();
}

///////////////////////////////////////////////////////////////////////////////////
//
//  The base menu class
//
///////////////////////////////////////////////////////////////////////////////////

CMenu::CMenu() {
}

CMenu::~CMenu() {
}

//called when the menu is first entered.
void CMenu::Enter() {
	
}

//called when the menu is exited.
void CMenu::Exit() {
	
}

void CMenu::Animate(int32 elapsed_time) {
	
}

//draws the menu
void CMenu::DrawForeground() {
	
}

void CMenu::DrawBackground() {
	
}

void CMenu::PlaceEntities() {
    return;
}

//called when the mouse is moved
bool CMenu::MousePos(int32 norm_x, int32 norm_y) {
    return false;
}

bool CMenu::MouseDown(int32 norm_x, int32 norm_y) {
    return false;
}

bool CMenu::MouseUp(int32 norm_x, int32 norm_y) {
    return false;
}

//accepts keyboard input.  
//returns true if the key was handled.
bool CMenu::Keydown(int32 key) {
    return false;
}

bool CMenu::Keyup(int32 key) {
    return false;
}

///////////////////////////////////////////////////////////////////////////////////
//
//  Main Menu class.
//
///////////////////////////////////////////////////////////////////////////////////

CMenuMain::CMenuMain()
{
	//initialize our state.
	selection = DKMB_NONE;
	
	//init our shutdown vars.
	shutting_down = false;
	shutdown_time = 0;
	
	init_done = false;
}


void CMenuMain::Init()
{
	DKM_ToggleEnterSound(true);
	
	//initialize our button pointers.
	sub_menu[0]  = new CMenuSubNewgame();
	sub_menu[1]  = new CMenuSubMultiplayer();
	sub_menu[2]  = new CMenuSubLoadgame();
	sub_menu[3]  = new CMenuSubSavegame();
	sub_menu[4]  = new CMenuSubSound();
	sub_menu[5]  = new CMenuSubVideo();
	sub_menu[6]  = new CMenuSubMouse();
	sub_menu[7]  = new CMenuSubKeyboard();
	sub_menu[8]  = new CMenuSubJoystick();
	sub_menu[9]  = new CMenuSubOptions();
	sub_menu[10] = new CMenuSubAdvanced();
	sub_menu[11] = new CMenuSubCredits();//new CMenuSubDemos();
	sub_menu[12] = new CMenuSubResume();
	sub_menu[13] = new CMenuSubQuit();
	
	init_done = true;
}


void CMenuMain::ResetSubMenu(dk_main_button menu_num)
{
	if (sub_menu[menu_num])  // init'd?
	{
		sub_menu[menu_num]->status      = BS_START;
		sub_menu[menu_num]->dest_status = BS_START;
		
		sub_menu[menu_num]->enabled     = true;
		sub_menu[menu_num]->time        = 0;
	}
	
}



CMenuMain::~CMenuMain()
{
}

void CMenuMain::GameShutdown()
{
	for (int32 i = 0; i < DKMB_NUM_BUTTONS; i++)
	{
		delete sub_menu[i];
	}
}


void CMenuMain::Enter()
{
	if (!init_done) 
		Init();
	
	if (dkm_play_enter_sound)                   
		PlayInterfaceSound(DKM_sounds[DKMS_ENTER]);
	
	DKM_ToggleEnterSound(true);  // reset
	
	// pause map music
	S_StopMusic(CHAN_MUSIC_MAP);
	
	//S_StartMusic("e1b",CHAN_MUSIC_MENU);
//	S_StartSound(CVector(0,0,0), 1, CHAN_LOOP, S_RegisterSound(DKM_sounds[DKMS_LOOP]), 0.25f, 0.0f, 0.0f);
//	int offset = ( ( int )rand() * 1000 ) % 2;


// AJR 04/18/2000: Only play one menu song for demo
#ifdef DAIKATANA_DEMO
	S_StartMusic( DKM_sounds[DKMS_MENU_MP3_1], CHAN_MUSIC_MENU);
#else
	S_StartMusic( DKM_sounds[DKMS_MENU_MP3_1 + (rand() & 1)], CHAN_MUSIC_MENU);
#endif
    //Select(DKMB_NONE);
	Select(dkm_last_menu_in);
	
	shutting_down = false;
	console_shutdown_string = "";
}



void CMenuMain::Exit()
{
    Select(DKMB_NONE);
}



char *button_skins[DKMB_NUM_BUTTONS + 1] = {  // + 1 for 'unused' skin
    "skins/ib_singleplay.bmp",	//"skins/ib_newgame.bmp",		// DKMB_NEWGAME
	"skins/ib_multi.bmp",		// DKMB_MULTIPLAYER
	"skins/ib_loadgame.bmp",	// DKMB_LOADGAME
	"skins/ib_savegame.bmp",	// DKMB_SAVEGAME
	"skins/ib_sound.bmp",		// DKMB_SOUND
	"skins/ib_video.bmp",		// DKMB_VIDEO
	"skins/ib_mouse.bmp",		// DKMB_MOUSE
	"skins/ib_keyboard.bmp",	// DKMB_KEYBOARD
	"skins/ib_joystick.bmp",	// DKMB_JOYSTICK
	"skins/ib_options.bmp",		// DKMB_OPTIONS
	"skins/ib_config.bmp",		// DKMB_CONFIG
	"skins/ib_credits.bmp",		//"skins/ib_demos.bmp",		// DKMB_DEMOS
	"skins/ib_resume.bmp",		// DKMB_RESUME
	"skins/ib_quit.bmp",		// DKMB_QUIT
	"skins/ib_blank.bmp"		// DKMB_NUM_BUTTONS
};



bool UsedSideButton(int i)
// quick flag for demo mode to see if this button is used
{
	
	
#ifdef DM_DEMO
	if (i == DKMB_NEWGAME  ||
		i == DKMB_LOADGAME ||
		i == DKMB_SAVEGAME ||
		i == DKMB_JOYSTICK ||
		i == DKMB_CONFIG   ||
		i == DKMB_DEMOS) //    ||
		//i == DKMB_HELP) 
		return (false);
#endif

// added for Japanese version
#ifdef DAIKATANA_DEMO
#ifndef JPN
	if (i == DKMB_NEWGAME  ||
		i == DKMB_LOADGAME ||
		i == DKMB_SAVEGAME ||
//		i == DKMB_JOYSTICK ||
//		i == DKMB_CONFIG   ||
		i == DKMB_CREDITS) //    ||
		//i == DKMB_HELP) 
		return (false);
#else

	if (/*i == DKMB_MULTIPLAYER  ||*/
//		i == DKMB_LOADGAME ||
//		i == DKMB_SAVEGAME ||
//		i == DKMB_JOYSTICK ||
//		i == DKMB_CONFIG   ||
		i == DKMB_CREDITS) //    ||
		//i == DKMB_HELP) 
		return (false);
#endif // JPN
// SCG[6/16/00]: This breaks things.  Commenting this out makes the U.S. version do what it's
// SCG[6/16/00]: supposed to do and the japanese version do what it's supposed to do as well as
// SCG[6/16/00]: the demo for each respective version..
/*
#else // DAIKATANA_DEMO
#ifndef JPN
	if (i == DKMB_NEWGAME  ||
		i == DKMB_LOADGAME ||
		i == DKMB_SAVEGAME)// ||
//		i == DKMB_JOYSTICK ||
//		i == DKMB_CONFIG   ||
//		i == DKMB_CREDITS) //    ||
		//i == DKMB_HELP) 
		return (false);
#else

//	if (i == DKMB_MULTIPLAYER  ||
//		i == DKMB_LOADGAME ||
//		i == DKMB_SAVEGAME ||
//		i == DKMB_JOYSTICK ||
//		i == DKMB_CONFIG   ||
//		i == DKMB_CREDITS) //    ||
		//i == DKMB_HELP) 
//		return (false);
#endif // JPN
*/
#endif // DAIKATANA_DEMO
	
	
	return (true);
};


void CMenuMain::PlaceButtons() {
    //positioning data for the buttons.
    static CVector button_top_origin(35, -2.8, 8.35);
    static CVector button_top_direction(0, 210, 0);
    static float button_height = 1.165f;
    static float button_scale = 1.09f;
	
    //the current z value we will draw the model at.
    float button_z = button_top_origin.z;
	
    for (int32 i = 0; i < DKMB_NUM_BUTTONS; i++)
    {
        //get an entity structure we can fill in.
        entity_t *button = NewEntity();
		
        //assign the model pointer.
        button->model = button_model;
/*		
        // check for special skin replacement, if this is the resume button
        if (i == DKMB_RESUME)
        {
			// if the player is in the game, show 'resume' button
			button->skin = re.RegisterSkin(button_skins[i], RESOURCE_INTERFACE); 
			if (in_main_game() == true)
			{
				button->color.Set( 0.0, 0.0, 0.0 );
			}
			else // else dim the button
			{
				button->color.Set( 0.25, 0.25, 0.25 );
			}
        }
        else 
        {
			// check for special skin replacement, if this is the resume button
			if (i == DKMB_SAVEGAME)
			{
				button->skin = re.RegisterSkin(button_skins[i], RESOURCE_INTERFACE); 
				// if the player is in the game and has savegems, show 'savegame' button
				if (in_main_game() == true && cl_savegem_count > 0)
				{
					button->color.Set( 0.0, 0.0, 0.0 );
				}
				else // else dim the button
				{
					button->color.Set( 0.25, 0.25, 0.25 );
				}
			}
			else
			{
				button->skin = re.RegisterSkin(button_skins[i], RESOURCE_INTERFACE);
				//put the skin on the button.
				if (UsedSideButton(i))
				{
					button->color.Set( 0.0, 0.0, 0.0 );
				}
				else
				{
					button->color.Set( 0.25, 0.25, 0.25 );
				}
			}
        }
*/		
        // check for special skin replacement, if this is the resume button
        if (i == DKMB_RESUME)
        {
			// if the player is in the game, show 'resume' button
			button->skin = re.RegisterSkin(button_skins[i], RESOURCE_INTERFACE); 
			if (in_main_game() == true)
			{
				button->color.Set( 0.0, 0.0, 0.0 );
			}
			else // else dim the button
			{
				button->color.Set( 0.25, 0.25, 0.25 );
			}
        }
        else if (i == DKMB_SAVEGAME)
		{
			button->skin = re.RegisterSkin(button_skins[i], RESOURCE_INTERFACE); 
			// if the player is in the game and has savegems, show 'savegame' button
			if ( enable_save() )//(in_main_game() == true) && (cl_savegem_count > 0) && !(cl.refdef.rdflags & RDF_LETTERBOX) && (svs.clients[0].edict->client->ps.stats[STAT_HEALTH] > 0))
			{
				button->color.Set( 0.0, 0.0, 0.0 );
			}
			else // else dim the button
			{
				button->color.Set( 0.25, 0.25, 0.25 );
			}
		}
		else
		{
			button->skin = re.RegisterSkin(button_skins[i], RESOURCE_INTERFACE);
			//put the skin on the button.
			if (UsedSideButton(i))
			{
				button->color.Set( 0.0, 0.0, 0.0 );
			}
			else
			{
				button->color.Set( 0.25, 0.25, 0.25 );
			}
		}
        //make the button fullbright.
        button->flags = RF_FULLBRIGHT;
		
        //set the origin.
        button->origin.x = button_top_origin.x;
        button->origin.y = button_top_origin.y;
        button->origin.z = button_z;
		
        //set the angles.
        button->angles = button_top_direction;
		
        //set the scale
        button->render_scale.Set( button_scale, button_scale, button_scale );
		
        //set the frame
        button->frame = sub_menu[i]->Frame();
		
        //move the z down a little.
        button_z -= button_height;
    }
}



void CMenuMain::ShutdownString(const char *string) 
{
    console_shutdown_string = string;
}


void CMenuMain::Shutdown()
{
    shutting_down = true;
    shutdown_time = 0;
	
    //play our exit sound.
    //PlayInterfaceSound("menus/Exit Menu_001.wav");
	
    //exit all submenus
    Enter(DKMB_NONE);
}


entity_t *NewEntity()
{
    return menu.NewEntity();
}

entity_t *CMenuMain::NewEntity() {
    //increment the number of entities.
    num_entities++;
	
    //return the next entity struct to be used.
    return &entities[num_entities - 1];
}

void CMenuMain::RenderView() 
{
    if (button_model == NULL) return;
	
    if (DKM_InMenu() == false) {
        return;
    }
	
    refdef_t refdef;
    memset(&refdef, 0, sizeof(refdef));
	
	//    refdef.vieworg[2] = 100;
    refdef.x = scr_vrect.x;
    refdef.y = scr_vrect.y;
	//    refdef.width = 640;//viddef.width;
	//    refdef.height = 480;//viddef.height;
    refdef.width = viddef.width;
    refdef.height = viddef.height;
    refdef.fov_x = 40.0f;
    refdef.fov_y = CalcFov(refdef.fov_x, refdef.width, refdef.height);
    refdef.time = cl.time*0.001;
    refdef.areabits = cl.frame.areabits;
    refdef.rdflags = RDF_NOWORLDMODEL | RDF_CLEARDEPTHBUFF;
    refdef.beamList = &beamList;
	
    //initialize our entity list.
    num_entities = 0;
	
    //initialize the entities we are going to draw.
    menu.PlaceEntities();
	
    //add the entities.
    refdef.num_entities = num_entities;
    refdef.entities = &entities[0];
	
    //render the frame.
    re.RenderFrame (&refdef);
}

void CMenuMain::Draw() {
    //draw the background images.
	int		width_256/*, width_128*/;
	int		/*pic1_x,*/ pic2_x/*, pic3_x*/;
	int		height;
	float	x_scale, y_scale;
	DRAWSTRUCT drawStruct;
	
	x_scale = (viddef.width / 640.0);
	y_scale = (viddef.height / 480.0);
	
	width_256 = (int)(256.0 * x_scale);
//	width_128 = (int)(128.0 * x_scale);
	
//	pic1_x = 0;
	pic2_x = width_256;
//	pic3_x = pic2_x + width_256;
	
	height = (int)(256.0 * y_scale);
	
	//==================================
	// consolidation change: SCG 3-11-99
	
	drawStruct.nFlags = DSFLAG_SCALE|DSFLAG_PALETTE;
	drawStruct.fScaleX = x_scale;
	drawStruct.fScaleY = y_scale;
	
	drawStruct.nYPos = 0;
	
	int index = 0;
	for( int i = 0; i < 2; i++ )
	{
		drawStruct.nXPos = 0;
		for( int j = 0 ;j < 3; j++ )
		{
			drawStruct.pImage = re.RegisterPic( bitmaps[DKB_BACK00 + index++].name, NULL, NULL, RESOURCE_INTERFACE );
			re.DrawPic( drawStruct );
			drawStruct.nXPos += width_256;
		}
		drawStruct.nYPos += height;
	}
	
	// consolidation change: SCG 3-11-99
	//==================================
	
    //draw the background of the selected menu.
    if (selection != DKMB_NONE) {
        sub_menu[selection]->DrawBackground();
    }
	
    //draw the models.
    RenderView();
	
    //draw the foreground of the selected menu.
    if (selection != DKMB_NONE) {
        sub_menu[selection]->DrawForeground();
    }
	
    //get the cursor pic we will use this frame.
    int32 cursor = int32(float(abs(last_frame_time) % CURSOR_TOTAL_TIME) / CURSOR_TOTAL_TIME * CURSOR_NUM_FRAMES);
	
    //draw the cursor.
	//    re.DrawPic (dkm_mouse_x - cursor_offset_x, dkm_mouse_y - cursor_offset_y, bitmaps[dk_interface_bmp(DKB_CURSOR_00 + cursor)].name, RESOURCE_INTERFACE);
	drawStruct.pImage = re.RegisterPic( bitmaps[dk_interface_bmp(DKB_CURSOR_00 + cursor)].name, NULL, NULL, RESOURCE_INTERFACE );
	drawStruct.nFlags = DSFLAG_BLEND | DSFLAG_SCALE;
	drawStruct.nXPos = dkm_mouse_x - cursor_offset_x;
	drawStruct.nYPos = dkm_mouse_y - cursor_offset_y;
	drawStruct.fScaleX = x_scale;
	drawStruct.fScaleY = y_scale;
	re.DrawPic( drawStruct );

	// draw the version string
	DKM_DrawString(VERSION_X,VERSION_Y,VERSION_STR,NULL,false,false,0.2);
}

void CMenuMain::PlaceEntities() {
    //put the buttons in the correct position.
    PlaceButtons();
	
	
    //tell the selected sub-menu to place it's entities.
    if (selection != DKMB_NONE) {
        sub_menu[selection]->PlaceEntities();
    }
}

bool CMenuMain::MousePos(int32 norm_x, int32 norm_y)
{
    if (shutting_down == true) return true;
	
    //check the sub-menu
    if (selection != DKMB_NONE) {
        sub_menu[selection]->MousePos(norm_x, norm_y);
        //even if the sub-menu handled the move, we
        //still process the new mouse position.
    }
	
    //get the current time.
//    int32 cur_time = GetTickCount();
	
    //the button that the mouse is over.
    dk_main_button button_hit = DKM_ButtonNum(norm_x, norm_y);
	
	
	
    // if the Resume button is not active, don't show animiation with browse
    if (button_hit == DKMB_RESUME)
		if (!in_main_game())
			return false;
		
	// if the Save button is not active, don't show animiation with browse
	if (button_hit == DKMB_SAVEGAME)
		if ( !enable_save() )//in_main_game() || !cl_savegem_count || (cl.refdef.rdflags & RDF_LETTERBOX) || (svs.clients[0].edict->client->ps.stats[STAT_HEALTH] <= 0)) // not in the game or no savegems
			return false;                           // bail
			
		
	//if the button dest status is BS_START, set it to BS_HILIGHT.
	if (button_hit != DKMB_NONE &&  
		sub_menu[button_hit]->dest_status == BS_START && 
		sub_menu[button_hit]->enabled == true)
	{
				
				
// changed by yokoyama for Japanese version
#ifndef JPN
#ifdef DM_DEMO
		if (UsedSideButton((int)button_hit))
#endif
#endif         
		{
			
			
			//change the destination status.
			sub_menu[button_hit]->dest_status = BS_HILIGHT;
			
			//get the current status of the button.
			dk_button_status status = sub_menu[button_hit]->status;
			
			//check what state we are in now.
			if (status == BS_START || status == BS_HILIGHT_BACKWARD)
			{
				//change to hilight forward status.
				sub_menu[button_hit]->status = BS_HILIGHT_FORWARD;
				//remember the time we entered the state.
				sub_menu[button_hit]->time = 0;
			}
		}
	}
	
	//move other buttons back to the start status if we have to.
	for (int32 i = 0; i < DKMB_NUM_BUTTONS; i++)
	{
		//make sure this isnt the one we are on now.
		if (i == button_hit) continue;
		
#ifndef JPN
#ifdef DM_DEMO
		if (!UsedSideButton(i))
			continue;
#endif
#endif         
		
		
		
		//check the dest status of this button.
		if (sub_menu[i]->dest_status == BS_HILIGHT)
		{
			
			//make the dest status BS_START
			sub_menu[i]->dest_status = BS_START;
			
			//check our current status.
			dk_button_status status = sub_menu[i]->status;
			if (status == BS_HILIGHT || status == BS_HILIGHT_FORWARD) {
				//change to the BS_HILIGHT_BACKWARD status.
				sub_menu[i]->status = BS_HILIGHT_BACKWARD;
				//remember the time we started.
				sub_menu[i]->time = 0;
			}
		}
	}
	
	return button_hit != DKMB_NONE;
}


bool CMenuMain::Keyup(int32 key)
{
    if (shutting_down == true) return true;
	
    return false;
}

bool CMenuMain::Keydown(int32 key) 
{
    if (shutting_down == true) return true;
	
    //check the sub-menu
    if (selection != DKMB_NONE) {
        if (sub_menu[selection]->Keydown(key) == true) {
            //the sub-menu handled the key.
            return true;
        }
    }
	
    dk_main_button selected_menu = DKMB_NONE;
	
    switch (key)
    {
	case '`' :         // console
		if( Cvar_VariableValue( "console" ) == 0 )
		{
			return false;
		}
		Shutdown ();
		return true;

	case K_ESCAPE :    // escape key
		if( (Cvar_VariableValue( "console" ) == 0) && !in_main_game() )
		{
			return false;
		}
		Shutdown ();
		return true;
		
//	case 'n' : selected_menu = DKMB_NEWGAME;     break;
	case 'g' : selected_menu = DKMB_NEWGAME;     break;
	case 'l' : selected_menu = DKMB_LOADGAME;    break;
	case 's' : 
		{
			if( enable_save() )//in_main_game() && cl_savegem_count > 0 && !(cl.refdef.rdflags & RDF_LETTERBOX) && (svs.clients[0].edict->client->ps.stats[STAT_HEALTH] > 0)) 
				selected_menu = DKMB_SAVEGAME; 
			break;
		}
	case 'j' : selected_menu = DKMB_JOYSTICK;    break;
	case 'd' : selected_menu = DKMB_CREDITS;     break;
	case 'c' : selected_menu = DKMB_CONFIG;      break;
	case 'r' : if( in_main_game() == true ) selected_menu = DKMB_RESUME; break;
	case 'p' : selected_menu = DKMB_MULTIPLAYER; break;
	case 'u' : selected_menu = DKMB_SOUND;       break;
	case 'v' : selected_menu = DKMB_VIDEO;       break;
	case 'm' : selected_menu = DKMB_MOUSE;       break;
	case 'k' : selected_menu = DKMB_KEYBOARD;    break;
	case 'o' : selected_menu = DKMB_OPTIONS;     break;
	case 'q' : selected_menu = DKMB_QUIT;        break;
		
	default:
		return false;
    }
	
    //make sure we have a menu to select.
    if (selected_menu == DKMB_NONE) 
		return true;
	
    //make sure the selected menu button isnt animating.
    if (sub_menu[selected_menu]->status == BS_SELECT_BACKWARD) 
		return true;
	
    //enter the menu
    Enter(selected_menu);
	
    return true;
}


bool CMenuMain::MouseUp(int32 norm_x, int32 norm_y)
{
    if (shutting_down == true) return true;
	
    if (selection != DKMB_NONE)
    {
        if (sub_menu[selection]->MouseUp(norm_x, norm_y) == true)
        {
            //the sub-menu handled the event.
            return true;
        }
    }
	
    return false;
}


bool CMenuMain::MouseDown(int32 norm_x, int32 norm_y)
{
    if (shutting_down == true) return true;
	
    //check if we are editing text in an edit field.
    CInterfaceInputField *current_field = CurrentlyEditingField();
    if (current_field != NULL) {
        //check if the field would want the click.
        if (current_field->MouseDown(norm_x, norm_y) == false) {
            //abort editing this field.
            current_field->StopEditing();
        }
    }
	
    //check the sub-menu
    if (selection != DKMB_NONE) {
        if (sub_menu[selection]->MouseDown(norm_x, norm_y) == true) {
            //the sub-menu handled the click.
            return true;
        }
    }
	
    //get the button that the mouse on top of.
    dk_main_button button_hit = DKM_ButtonNum(norm_x, norm_y);
    if (button_hit == DKMB_NONE) {
        return false;
    }
    //make sure the submenu is working.
    if (sub_menu[button_hit]->enabled == false) {
        return false;
    }
    //make sure the button is not in an animation.
    if (sub_menu[button_hit]->status == BS_SELECT_BACKWARD) {
        return false;
    }
	
	
    // special case: check to see if Resume button can be active
    if (button_hit == DKMB_RESUME)
	{
		if (!in_main_game())
		{
			return false;
		}
	}
		
		// special case: check to see if Save button can be active  // TTD: CHECK SAVEGEMS
	if (button_hit == DKMB_SAVEGAME)
	{
		if ( !enable_save() )//in_main_game() || !cl_savegem_count || (cl.refdef.rdflags & RDF_LETTERBOX) || (svs.clients[0].edict->client->ps.stats[STAT_HEALTH] <= 0)) // not in game or no savegems
		{
			return false;                           // bail
		}
	}
			
	//select our new button.
	Select(button_hit);
	
	//if any other buttons are selected, we need to send them back to the selected status
	for (int32 i = 0; i < DKMB_NUM_BUTTONS; i++)
	{
		if (i == button_hit) continue;
		
#ifndef JPN
#ifdef DM_DEMO
		if (!UsedSideButton(i))
			continue;
#endif
#endif
		//unselect this button.
		CheckUnselect((dk_main_button)i);
	}
	
	//call DKM_NewMousePos, so hilight detection can occur for any buttons we just deselected.
	MousePos(norm_x, norm_y);
	
	return true;
}


void CMenuMain::CheckUnselect(dk_main_button button)
{
    // if blasting out of interface, no animate
    if (DKM_blasting == true)
		return;
	
	//check the dest status.
	if (sub_menu[button]->dest_status == BS_SELECTED)
	{
		//send it back to the hilight status.
		sub_menu[button]->dest_status = BS_HILIGHT;
		
		//check our current status.
		dk_button_status status = sub_menu[button]->status;
		if (status == BS_SELECTED || status == BS_SELECT_FORWARD)
		{
            //go to select_backward
            sub_menu[button]->status = BS_SELECT_BACKWARD;
            //remember the time.
            sub_menu[button]->time = 0;
			
            if (animation_select_time) // if we're not zooming out of the interface, play .wav file
				PlayInterfaceSound(DKM_sounds[DKMS_DESCEND]);
		}
		//else, we are in hilight forward status, dont do anything.
	}
	
}


void CMenuMain::Enter(dk_main_button button)
{
    Select(button);
}


//called when a new button is selected
void CMenuMain::Select(dk_main_button button)
{
    //check if the selection is changing.
    if (button == selection) 
		return;
	
    // exiting by other method than the QUIT & NEWGAME submenus?
    if (button == DKMB_NONE && selection != DKMB_QUIT)
    {
		// let's haul ass out of the menu system by cranking up animation/shutdown delay timers
		animation_select_time  = 0;
		animation_hilight_time = 0;
		shutdown_sequence_time = 0;
		
		DKM_blasting = true; // no animation on exit
    }
	
    //leave our old state.
    if (selection != DKMB_NONE)
    {
        //tell the menu we are exiting it.
        sub_menu[selection]->Exit();
        //put the button out of selected status.
        CheckUnselect(selection);
    }
	
    //set our state
    selection = button;
    
    //enter the new menu
    if (selection != DKMB_NONE)
    {
        if (selection != DKMB_QUIT &&    // if user hits 'no' on quit menu
            selection != DKMB_RESUME)    // or the resume button
			dkm_last_menu_in = selection;  // keep track of last menu entered    1.10 dsn
		
        sub_menu[selection]->Enter();
		
        
        if (DKM_blasting == true) // blasting in/out? show no animations
        {
			sub_menu[selection]->dest_status = BS_SELECTED;
			sub_menu[selection]->status = BS_SELECTED;            // force to selected
			sub_menu[selection]->time = 0;                        // save time
        }
        else
        {
			//set this button so that it starts flipping around.
			sub_menu[selection]->dest_status = BS_SELECTED;
			
			dk_button_status status = sub_menu[selection]->status;
			//change the status of the button.
			if (status == BS_START || status == BS_HILIGHT_BACKWARD || status == BS_HILIGHT_FORWARD || 
				status == BS_HILIGHT || status == BS_SELECT_BACKWARD) 
			{
				//move it towards the selected state.
				sub_menu[selection]->status = BS_SELECT_FORWARD;
				
				//save the time.
				sub_menu[selection]->time = 0;
				
				PlayInterfaceSound(DKM_sounds[DKMS_ASCEND]);
			}
        }
    }
}


void CMenuMain::ShuttingDown(int32 elapsed_time)
{
    //check if we are in our shutdown sequence.
    if (menu.shutting_down == true)
    {
        //increment the time.
        shutdown_time += elapsed_time;
		
        //check if we are done with the shutdown sequence.
        if (shutdown_time >= shutdown_sequence_time)
        {
            //send our shutdown string to the console.
            Cbuf_AddText(console_shutdown_string);
			
            M_PopMenu();
            DKM_Leave();
            cls.key_dest = key_game;
        }
    }
}


void CMenuMain::Animate(int32 elapsed_time)
{
    //go through each of the buttons.
    for (int32 i = 0; i < DKMB_NUM_BUTTONS; i++)
    {
		
#ifndef JPN
#ifdef DM_DEMO
        if (!UsedSideButton(i))
			continue;
#endif
#endif		
		
		
        //get the button.
        CMenuSub *button = sub_menu[i];
		
        if (i == selection)
        {
            //let the sub-menu animate its controls and stuff.
            button->Animate(elapsed_time);
        }
		
        //add the elapsed time to the button's time.
        button->time += elapsed_time;
		
        //check our current status.
        if (button->status == BS_HILIGHT_FORWARD)
        {
            //we are going towards hilight.
            //check if we have enough time to get there.
            if (button->time >= animation_hilight_time)
            {
                //we have enough time to go to the hilight status.
                button->status = BS_HILIGHT;
				
                //check if we are continuing.
                if (button->dest_status == BS_SELECTED)
                {
                    //continue to the select forward state.
                    button->status = BS_SELECT_FORWARD;
					
                    //subtract off the time.
                    button->time -= animation_hilight_time;
					
                    //we will do the check for BS_SELECT_FORWARD below.
                }
            }
        }
		
        if (button->status == BS_SELECT_FORWARD)
        {
            //we are going towards select.
            //check if we are enough time to get there.
            if (button->time >= animation_select_time)
            {
                //we have enough time to go to the hilight status.
                button->status = BS_SELECTED;
            }
        }
		
        if (button->status == BS_SELECT_BACKWARD)
        {
            //we are going towards hilight
            //check if we have enough time to get there.
            if (button->time >= animation_select_time) 
            {
                //we have enough time.
                button->status = BS_HILIGHT;
				
                //check if we are continuing.
                if (button->dest_status == BS_START)
                {
                    //continue to hilight backward state.
                    button->status = BS_HILIGHT_BACKWARD;
					
                    //subtract off the time.
                    button->time -= animation_select_time;
					
                    //we will the check for BS_HILIGHT_BACKWARD below.
                }
            }
        }
		
        if (button->status == BS_HILIGHT_BACKWARD)
        {
            //we are going towards start
            //check if we have enough time.
            if (button->time >= animation_hilight_time)
            {
                //we have enough time.
                button->status = BS_START;
            }
        }
    }
	
    
	
}


void ToggleRadioState(const char *cvar_name) {
    if (cvar_name == NULL || cvar_name[0] == '\0') return;
	
    //get the value of the cvar.
    float cur_value = Cvar_VariableValue(cvar_name);
	
    //set it to the opposite value.
    if (cur_value != 0.0f) {
        Cvar_ForceSet(cvar_name, "0");
    }
    else {
        Cvar_ForceSet(cvar_name, "1");
    }
}


void SetRadioState(CInterfaceButtonRadio &control, const char *cvar_name, bool inverse) 
{
    if (cvar_name == NULL || cvar_name[0] == '\0') return;
	
    //get the value.
    float cur_value = Cvar_VariableValue(cvar_name);
	
    //set the check state.
	bool check = (cur_value != 0.0f);
	if (inverse) check = !check;
    control.Check(check);
}

void Cvar_ForceSetValue (const char *var_name, float value)
{
	char	val[32];

	if (value == (int)value)
		Com_sprintf (val, sizeof(val), "%i",(int)value);
	else
		Com_sprintf (val, sizeof(val), "%f",value);
	Cvar_ForceSet (var_name, val);
}

void ToggleCvarFlag(char *cvar_name, unsigned long flag)
{
    if (cvar_name == NULL || cvar_name[0] == '\0') return;
	
    //get the value of the cvar.
    unsigned long cur_value = (unsigned long)Cvar_VariableValue(cvar_name);
	
	Cvar_ForceSetValue(cvar_name,cur_value ^= flag);
}

void SetRadioStateFlag(CInterfaceButtonRadio &control, const char *cvar_name, unsigned long flag, bool inverse) 
{
    if (cvar_name == NULL || cvar_name[0] == '\0') return;
	
    //get the value.
    unsigned long cur_value = Cvar_VariableValue(cvar_name);
	
    //set the check state.
	bool check = ((cur_value & flag) != 0);
	if (inverse) check = !check;
    control.Check(check);
}
///////////////////////////////////////////////////////////////////////////////////
//
//  Sub-Main Menu base class.
//
///////////////////////////////////////////////////////////////////////////////////

CMenuSub::CMenuSub() {
    status = BS_START;
    time = 0;
    dest_status = BS_START;
	
    enabled = true;
}

#define FRAME_START 0
#define FRAME_HILIGHT 1
#define FRAME_SELECTED 15

int32 CMenuSub::Frame() {
    //check the status.
    float percent;
    switch (status) {
	case BS_START: {
		return FRAME_START;
				   }
	case BS_HILIGHT_FORWARD: {
		percent = float(time) / (animation_hilight_time + 1);
		bound_min(percent, 0.0f);
		bound_max(percent, 1.0f);
		return int32(FRAME_START + (FRAME_HILIGHT - FRAME_START) * percent);
							 }
	case BS_HILIGHT_BACKWARD: {
		percent = float(time) / (animation_hilight_time + 1);
		bound_min(percent, 0.0f);
		bound_max(percent, 1.0f);
		percent = 1.0f - percent;
		return int32(FRAME_START + (FRAME_HILIGHT - FRAME_START) * percent);
							  }
	case BS_HILIGHT: {
		return FRAME_HILIGHT;
					 }
	case BS_SELECT_FORWARD: {
		percent = float(time) / (animation_select_time + 1);
		bound_min(percent, 0.0f);
		bound_max(percent, 1.0f);
		return int32(FRAME_HILIGHT + (FRAME_SELECTED - FRAME_HILIGHT) * percent);
							}
	case BS_SELECT_BACKWARD: {
		percent = float(time) / (animation_select_time + 1);
		bound_min(percent, 0.0f);
		bound_max(percent, 1.0f);
		percent = 1.0f - percent;
		return int32(FRAME_HILIGHT + (FRAME_SELECTED - FRAME_HILIGHT) * percent);
							 }
	case BS_SELECTED: {
		return FRAME_SELECTED;
					  }
    }
	
    return 0;
}

void DKM_ToggleEnterSound(bool toggle)
{
	dkm_play_enter_sound = toggle;
} 

void DoQuit(qboolean bQuit)
{
	if (bQuit)
	{
		DKM_ShutdownString("quit\n");
		DKM_Shutdown();
	}
	else
	{
		if (dkm_last_menu_in == DKMB_QUIT)
		{
			dkm_last_menu_in = DKMB_NEWGAME;
			DKM_Shutdown();
		}
		else
		{
			PlayInterfaceSound(DKM_sounds[DKMS_NO]);
			DKM_ToggleEnterSound(false); // don't play entry sound next time

			//menu.Enter();
			DKM_EnterMain();
		}
	}
}
