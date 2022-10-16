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
//  Options Menu
//
///////////////////////////////////////////////////////////////////////////////////


#define OPTIONS_LEFT											(100)
#define OPTIONS_SPACING											(20)
#define OPTIONS_LEFT2											(300)

#define OPTIONS_ALWAYS_RUN_LEFT									(OPTIONS_LEFT)
#define OPTIONS_ALWAYS_RUN_TOP									(105)

#define OPTIONS_LOOKSPRING_LEFT									(OPTIONS_LEFT)
#define OPTIONS_LOOKSPRING_TOP									(OPTIONS_ALWAYS_RUN_TOP + OPTIONS_SPACING)

#define OPTIONS_LOOKSTRAFE_LEFT									(OPTIONS_LEFT)
#define OPTIONS_LOOKSTRAFE_TOP									(OPTIONS_LOOKSPRING_TOP + OPTIONS_SPACING)

#define OPTIONS_AUTO_TARGET_LEFT								(OPTIONS_LEFT)
#define OPTIONS_AUTO_TARGET_TOP									(OPTIONS_LOOKSTRAFE_TOP + OPTIONS_SPACING)

#define OPTIONS_WEAPON_VISIBLE_LEFT								(OPTIONS_LEFT)
#define OPTIONS_WEAPON_VISIBLE_TOP								(OPTIONS_AUTO_TARGET_TOP + OPTIONS_SPACING)
																
#define OPTIONS_SUBTITLES_LEFT									(OPTIONS_LEFT)
#define OPTIONS_SUBTITLES_TOP									(OPTIONS_WEAPON_VISIBLE_TOP + OPTIONS_SPACING)

/*																
#define OPTIONS_NO_ALTTAB_LEFT									(OPTIONS_LEFT)												
#define OPTIONS_NO_ALTTAB_TOP									(OPTIONS_SUBTITLES_TOP + OPTIONS_SPACING)

#define OPTIONS_WEAPON_BOB_LEFT									(OPTIONS_LEFT)
#define OPTIONS_WEAPON_BOB_TOP									(OPTIONS_NO_ALTTAB_TOP + OPTIONS_SPACING)

#define OPTIONS_WEAPON_AUTOSWITCH_LEFT							(OPTIONS_LEFT)
#define OPTIONS_WEAPON_AUTOSWITCH_TOP							(OPTIONS_WEAPON_BOB_TOP + OPTIONS_SPACING)

#define OPTIONS_PLAY_INTRO_LEFT									(OPTIONS_LEFT)
#define OPTIONS_PLAY_INTRO_TOP									(OPTIONS_WEAPON_AUTOSWITCH_TOP + OPTIONS_SPACING)

#define OPTIONS_BLOODY_GAME_LEFT								(OPTIONS_LEFT)
#define OPTIONS_BLOODY_GAME_TOP									(OPTIONS_PLAY_INTRO_TOP + OPTIONS_SPACING)
*/

#define OPTIONS_NO_ALTTAB_LEFT									(OPTIONS_LEFT2)												
#define OPTIONS_NO_ALTTAB_TOP									(OPTIONS_ALWAYS_RUN_TOP)

#define OPTIONS_WEAPON_BOB_LEFT									(OPTIONS_LEFT2)
#define OPTIONS_WEAPON_BOB_TOP									(OPTIONS_NO_ALTTAB_TOP + OPTIONS_SPACING)

#define OPTIONS_WEAPON_AUTOSWITCH_LEFT							(OPTIONS_LEFT2)
#define OPTIONS_WEAPON_AUTOSWITCH_TOP							(OPTIONS_WEAPON_BOB_TOP + OPTIONS_SPACING)

#define OPTIONS_PLAY_INTRO_LEFT									(OPTIONS_LEFT2)
#define OPTIONS_PLAY_INTRO_TOP									(OPTIONS_WEAPON_AUTOSWITCH_TOP + OPTIONS_SPACING)

#define OPTIONS_DM_TAUNTS_LEFT									(OPTIONS_LEFT2)
#define OPTIONS_DM_TAUNTS_TOP									(OPTIONS_PLAY_INTRO_TOP + OPTIONS_SPACING)

#define OPTIONS_UNLIMITED_SAVES_LEFT							(OPTIONS_LEFT2)
#define OPTIONS_UNLIMITED_SAVES_TOP								(OPTIONS_DM_TAUNTS_TOP + OPTIONS_SPACING)

#define OPTIONS_DM_HELP_LEFT									(OPTIONS_LEFT)
#define OPTIONS_DM_HELP_TOP										(OPTIONS_UNLIMITED_SAVES_TOP + OPTIONS_SPACING)

#define OPTIONS_BLOODY_GAME_LEFT								(OPTIONS_LEFT2)
#define OPTIONS_BLOODY_GAME_TOP									(OPTIONS_UNLIMITED_SAVES_TOP + OPTIONS_SPACING)


#define OPTIONS_CROSSHAIR_LEFT									(220)
#define OPTIONS_CROSSHAIR_TOP									(245)
#define OPTIONS_CROSSHAIR_WIDTH									(130)

#define OPTIONS_CROSSHAIR_PIC_LEFT								(OPTIONS_CROSSHAIR_LEFT + OPTIONS_CROSSHAIR_WIDTH + 5)//(360)
#define OPTIONS_CROSSHAIR_PIC_TOP								(OPTIONS_CROSSHAIR_TOP + 13)//(170)
//#define OPTIONS_WEAPON_ORDER_LEFT (260)
//#define OPTIONS_WEAPON_ORDER_TOP (165)
//#define OPTIONS_WEAPON_ORDER_WIDTH (220)

#define OPTIONS_SBAR_ALPHA_X									(190)
#define OPTIONS_SBAR_ALPHA_Y									(390)
																
#define OPTIONS_SBAR_PIC_X										(224)
#define OPTIONS_SBAR_PIC_Y										(OPTIONS_SBAR_ALPHA_Y - 130)
																
#define OPTIONS_SBAR_STR_X										(206)
#define OPTIONS_SBAR_STR_Y										(OPTIONS_SBAR_ALPHA_Y - 20)
																
																
#define OPTIONS_ALWAYS_RUN_CVAR									"cl_run"
#define OPTIONS_LOOKSPRING_CVAR									"lookspring"
#define OPTIONS_LOOKSTRAFE_CVAR									"lookstrafe"
#define OPTIONS_AUTO_TARGET_CVAR								"autoaim"
#define OPTIONS_WEAPON_VISIBLE_CVAR								"weapon_visible"
#define OPTIONS_SUBTITLES_CVAR									"subtitles"
#define OPTIONS_NO_ALTTAB_CVAR									"win_noalttab"
#define OPTIONS_WEAPON_BOB_CVAR									"cl_bob"
#define OPTIONS_BLOODY_GAME_CVAR								"sv_violence"
#define OPTIONS_WEAPON_AUTOSWITCH_CVAR							"cl_autowweaponswitch"
#define OPTIONS_PLAY_INTRO_CVAR									"play_intro"
#define OPTIONS_DM_TAUNTS_CVAR									"dm_taunts"
#define OPTIONS_UNLIMITED_SAVES_CVAR							"unlimited_saves"
#define OPTIONS_DM_HELP_CVAR									"cl_dmhints"


extern cvar_t *statusbar_alpha;


CMenuSubOptions::CMenuSubOptions()
{
    crosshair_picker.AddString("None");
    //for the crosshair selections that aren't "none", we let the picker draw
    //a blank spot, and we draw our own graphic instead.
    crosshair_picker.AddString("1");
    crosshair_picker.AddString("2");
    crosshair_picker.AddString("3");
    crosshair_picker.AddString("4");

    //add the episode names to the episode picker for the weapon ordering
    /*
    weapon_episode_picker.AddString("Episode 1");
    weapon_episode_picker.AddString("Episode 2");
    weapon_episode_picker.AddString("Episode 3");
    weapon_episode_picker.AddString("Episode 4");
    */
}


void CMenuSubOptions::Enter()
{
	//initialize our controls.
    
	always_run_radio.Init (OPTIONS_ALWAYS_RUN_LEFT,     OPTIONS_ALWAYS_RUN_TOP,     menu_font, menu_font_bright, tongue_menu_options[T_MENU_ALWAYS_RUN]);
	lookspring_radio.Init (OPTIONS_LOOKSPRING_LEFT,     OPTIONS_LOOKSPRING_TOP,     menu_font, menu_font_bright, tongue_menu_options[T_MENU_LOOKSPRING]);
	lookstrafe_radio.Init (OPTIONS_LOOKSTRAFE_LEFT,     OPTIONS_LOOKSTRAFE_TOP,     menu_font, menu_font_bright, tongue_menu_options[T_MENU_LOOKSTRAFE]);
	auto_target_radio.Init(OPTIONS_AUTO_TARGET_LEFT,    OPTIONS_AUTO_TARGET_TOP,    menu_font, menu_font_bright, tongue_menu_options[T_MENU_AUTO_TARGETING]);
	weapon_visible.Init   (OPTIONS_WEAPON_VISIBLE_LEFT, OPTIONS_WEAPON_VISIBLE_TOP, menu_font, menu_font_bright, tongue_menu_options[T_MENU_WEAPON_VISIBLE]);
	subtitles_radio.Init  (OPTIONS_SUBTITLES_LEFT,      OPTIONS_SUBTITLES_TOP,      menu_font, menu_font_bright, tongue_menu_options[T_MENU_SUBTITLES]);

	crosshair_picker.Init(OPTIONS_CROSSHAIR_LEFT,    OPTIONS_CROSSHAIR_TOP, OPTIONS_CROSSHAIR_WIDTH, menu_font, menu_font_bright, button_font, tongue_menu_options[T_MENU_CROSSHAIR]);
	//weapon_episode_picker.Init(OPTIONS_WEAPON_ORDER_LEFT, OPTIONS_WEAPON_ORDER_TOP, OPTIONS_WEAPON_ORDER_WIDTH, menu_font, menu_font_bright, button_font, tongue_menu_options[T_MENU_WEAPON_ORDERING]);

	sbar_alpha.Init(OPTIONS_SBAR_ALPHA_X, OPTIONS_SBAR_ALPHA_Y, button_font, SLIDER_BACKGROUND_CHAR, SLIDER_TAB_CHAR);
	sbar_alpha.InitLinear(22, 0.0f, 1.0f);

	statusbar_alpha = Cvar_Get("statusbar_alpha", "0.60", CVAR_ARCHIVE);
	sbar_alpha.Position(statusbar_alpha->value); // set position according to cvar

	mouseIsDown = false; // track mouse for instant update on status bar slider

	no_alttab_radio.Init (OPTIONS_NO_ALTTAB_LEFT, OPTIONS_NO_ALTTAB_TOP, menu_font, menu_font_bright, tongue_menu[T_MENU_NO_ALTTAB]);
	weapon_bob_radio.Init (OPTIONS_WEAPON_BOB_LEFT, OPTIONS_WEAPON_BOB_TOP, menu_font, menu_font_bright, tongue_menu[T_MENU_WEAPON_BOB]);
	weapon_autoswitch_radio.Init (OPTIONS_WEAPON_AUTOSWITCH_LEFT,     OPTIONS_WEAPON_AUTOSWITCH_TOP, menu_font, menu_font_bright, tongue_menu[T_MENU_AUTOSWITCH]);

	play_intro_radio.Init (OPTIONS_PLAY_INTRO_LEFT, OPTIONS_PLAY_INTRO_TOP, menu_font, menu_font_bright, tongue_menu[T_MENU_PLAY_INTRO]);
	dm_taunts_radio.Init (OPTIONS_DM_TAUNTS_LEFT, OPTIONS_DM_TAUNTS_TOP, menu_font, menu_font_bright, tongue_menu[T_MENU_DM_TAUNTS]);

	unlimited_saves.Init (OPTIONS_UNLIMITED_SAVES_LEFT, OPTIONS_UNLIMITED_SAVES_TOP, menu_font, menu_font_bright, tongue_menu[T_MENU_UNLIMITED_SAVES]);

	dm_help.Init (OPTIONS_DM_HELP_LEFT, OPTIONS_DM_HELP_TOP, menu_font, menu_font_bright, tongue_menu[T_MENU_CTF_HELP]);
// SCG[6/5/00]: #ifndef TONGUE_GERMAN 
#ifndef NO_VIOLENCE
	bloody_game_radio.Init (OPTIONS_BLOODY_GAME_LEFT, OPTIONS_BLOODY_GAME_TOP, menu_font, menu_font_bright, tongue_menu[T_MENU_BLOODY]);
#endif
}



void CMenuSubOptions::Animate(int32 elapsed_time)
{
    sbar_alpha.Animate(elapsed_time);

    if (mouseIsDown) // still have the mouse button down while sliding alpha?
    {
      float value = sbar_alpha.Position();   //read the value from the slider.
      if (value > 1.0f)
        value = 1.0f;

      Cvar_SetValue("statusbar_alpha", value);
      DrawStatusBarPic();
    }

}

extern crosshair_t crosshairList[MAX_CROSSHAIRS][MAX_CROSSHAIR_TYPES];

void CMenuSubOptions::DrawCrosshair()
{
	int32 picked = crosshair_picker.CurrentIndex();
	if (!picked)
		return;

	picked -= 1;
	crosshair_t *ch = &crosshairList[picked][CROSSHAIR_NOTARGET];

	float	norm_to_screen_x = viddef.width / 640.0f;
	float	norm_to_screen_y = viddef.height / 480.0f;

	DRAWSTRUCT drawStruct;
	drawStruct.pImage = re.RegisterPic( ch->pic, NULL, NULL, RESOURCE_INTERFACE );
	drawStruct.nFlags = DSFLAG_BLEND;
	drawStruct.nXPos = OPTIONS_CROSSHAIR_PIC_LEFT * norm_to_screen_x;
	drawStruct.nYPos = OPTIONS_CROSSHAIR_PIC_TOP * norm_to_screen_y;
	drawStruct.fScaleX =  3 * norm_to_screen_x;
	drawStruct.fScaleY = 3 * norm_to_screen_y;
	drawStruct.fAlpha = 1.0;
	re.DrawPic( drawStruct );
}

void CMenuSubOptions::DrawStatusBarPic(void)
{
    if (statusbar_alpha->value >= 0.01f)
	{
		float	norm_to_screen_x = viddef.width / 640.0f;
		float	norm_to_screen_y = viddef.height / 480.0f;

		DRAWSTRUCT drawStruct;
		drawStruct.pImage = re.RegisterPic( "pics/statusbar/sbar_head.bmp", NULL, NULL, RESOURCE_INTERFACE );
		drawStruct.nFlags = DSFLAG_PALETTE | DSFLAG_ALPHA | DSFLAG_SCALE | DSFLAG_BLEND;
		drawStruct.nXPos = OPTIONS_SBAR_PIC_X * norm_to_screen_x;
		drawStruct.nYPos = OPTIONS_SBAR_PIC_Y * norm_to_screen_y;
		drawStruct.fScaleX = norm_to_screen_x;
		drawStruct.fScaleY = norm_to_screen_y;
		drawStruct.fAlpha = statusbar_alpha->value;
		re.DrawPic( drawStruct );
	}
}


void CMenuSubOptions::DrawForeground()
{
   //draw the radio buttons.

    always_run_radio.DrawForeground();
    lookspring_radio.DrawForeground();
    lookstrafe_radio.DrawForeground();
    auto_target_radio.DrawForeground();
    weapon_visible.DrawForeground();
	subtitles_radio.DrawForeground();

    no_alttab_radio.DrawForeground();
    weapon_bob_radio.DrawForeground();
    weapon_autoswitch_radio.DrawForeground();

	play_intro_radio.DrawForeground();
	dm_taunts_radio.DrawForeground();
	unlimited_saves.DrawForeground();
	dm_help.DrawForeground();

// SCG[6/5/00]: #ifndef TONGUE_GERMAN
#ifndef NO_VIOLENCE
	bloody_game_radio.DrawForeground();
#endif

    
	crosshair_picker.DrawForeground();
	DrawCrosshair();

    sbar_alpha.DrawForeground();
    DrawStatusBarPic();

	DKM_DrawString(OPTIONS_SBAR_STR_X, OPTIONS_SBAR_STR_Y, tongue_menu_options[T_MENU_OPTS_TRANSLUCENCY], NULL, false, false);

//    weapon_episode_picker.DrawForeground();

}

void CMenuSubOptions::DrawBackground()
{
    //get the current value of the crosshair setting.
    int32 crosshair_value = (int32)Cvar_VariableValue("crosshair");
    crosshair_picker.SetCurrentString(crosshair_value);

    //draw the radio buttons.  get the state of each one.
    SetRadioState(always_run_radio, OPTIONS_ALWAYS_RUN_CVAR);
    always_run_radio.DrawBackground();

    SetRadioState(lookspring_radio, OPTIONS_LOOKSPRING_CVAR);
    lookspring_radio.DrawBackground();

    SetRadioState(lookstrafe_radio, OPTIONS_LOOKSTRAFE_CVAR);
    lookstrafe_radio.DrawBackground();

    SetRadioState(auto_target_radio, OPTIONS_AUTO_TARGET_CVAR);
    auto_target_radio.DrawBackground();

    SetRadioState(weapon_visible,OPTIONS_WEAPON_VISIBLE_CVAR);
    weapon_visible.DrawBackground();

	SetRadioState(subtitles_radio, OPTIONS_SUBTITLES_CVAR);
    subtitles_radio.DrawBackground();

	SetRadioState(no_alttab_radio, OPTIONS_NO_ALTTAB_CVAR);
    no_alttab_radio.DrawBackground();

	SetRadioState(weapon_bob_radio, OPTIONS_WEAPON_BOB_CVAR);
    weapon_bob_radio.DrawBackground();

	SetRadioState(weapon_autoswitch_radio, OPTIONS_WEAPON_AUTOSWITCH_CVAR);
    weapon_autoswitch_radio.DrawBackground();

	SetRadioState(play_intro_radio, OPTIONS_PLAY_INTRO_CVAR);
    play_intro_radio.DrawBackground();

	SetRadioState(dm_taunts_radio, OPTIONS_DM_TAUNTS_CVAR);
    dm_taunts_radio.DrawBackground();

	SetRadioState(unlimited_saves, OPTIONS_UNLIMITED_SAVES_CVAR);
    unlimited_saves.DrawBackground();

	SetRadioState(dm_help, OPTIONS_DM_HELP_CVAR);
    dm_help.DrawBackground();

// SCG[6/5/00]: #ifndef TONGUE_GERMAN
#ifndef NO_VIOLENCE
	SetRadioState(bloody_game_radio, OPTIONS_BLOODY_GAME_CVAR,true);	// this flags is inverted
    bloody_game_radio.DrawBackground();
#endif

    crosshair_picker.DrawBackground();

    sbar_alpha.DrawBackground();

//    weapon_episode_picker.DrawBackground();
}

void CMenuSubOptions::PlaceEntities() {
    crosshair_picker.PlaceEntities();
//    weapon_episode_picker.PlaceEntities();
}

bool CMenuSubOptions::Keydown(int32 key) {
    switch (key) {
        case K_ESCAPE:
			if( Cvar_VariableValue( "console" ) == 0 )
			{
				return false;
			}
            //leave the menu.
            //DKM_EnterMain();
            DKM_Shutdown();       // exit entire interface  1.10 dsn
            return true;
    }

    return false;
}

bool CMenuSubOptions::MousePos(int32 norm_x, int32 norm_y) {
    bool ret = false;

    if (always_run_radio.MousePos(norm_x, norm_y)  == true)  ret = true;
    if (lookspring_radio.MousePos(norm_x, norm_y)  == true)  ret = true;
    if (lookstrafe_radio.MousePos(norm_x, norm_y)  == true)  ret = true;
    if (auto_target_radio.MousePos(norm_x, norm_y) == true)  ret = true;
    if (crosshair_picker.MousePos(norm_x, norm_y)  == true)  ret = true;
    if (weapon_visible.MousePos(norm_x, norm_y)    == true)  ret = true;
	if (subtitles_radio.MousePos(norm_x, norm_y)   == true)  ret = true;

    if (no_alttab_radio.MousePos(norm_x, norm_y)  == true)  ret = true;
    if (weapon_bob_radio.MousePos(norm_x, norm_y)    == true)  ret = true;
    if (weapon_autoswitch_radio.MousePos(norm_x, norm_y)    == true)  ret = true;
	if (play_intro_radio.MousePos(norm_x, norm_y)   == true)  ret = true;
	if (dm_taunts_radio.MousePos(norm_x, norm_y)   == true)  ret = true;
	if (unlimited_saves.MousePos(norm_x, norm_y)   == true)  ret = true;
	if (dm_help.MousePos(norm_x, norm_y)   == true)  ret = true;

// SCG[6/5/00]: #ifndef TONGUE_GERMAN
#ifndef NO_VIOLENCE
	if (bloody_game_radio.MousePos(norm_x, norm_y)   == true)  ret = true;
#endif
//    if (weapon_episode_picker.MousePos(norm_x, norm_y) == true) ret = true;

    if (sbar_alpha.MousePos(norm_x, norm_y) == true) ret = true;


    return ret;
}


bool CMenuSubOptions::MouseDown(int32 norm_x, int32 norm_y)
{
    if (always_run_radio.MouseDown(norm_x, norm_y)  == true)  return true;
    if (lookspring_radio.MouseDown(norm_x, norm_y)  == true)  return true;
    if (lookstrafe_radio.MouseDown(norm_x, norm_y)  == true)  return true;
    if (auto_target_radio.MouseDown(norm_x, norm_y) == true)  return true;
    if (crosshair_picker.MouseDown(norm_x, norm_y)  == true)  return true;
    if (weapon_visible.MouseDown(norm_x, norm_y)    == true)  return true;
    if (subtitles_radio.MouseDown(norm_x, norm_y)   == true)  return true;

    if (no_alttab_radio.MouseDown(norm_x, norm_y)  == true)  return true;
    if (weapon_bob_radio.MouseDown(norm_x, norm_y)    == true)  return true;
    if (weapon_autoswitch_radio.MouseDown(norm_x, norm_y)    == true)  return true;
	if (play_intro_radio.MouseDown(norm_x, norm_y)   == true)  return true;
	if (dm_taunts_radio.MouseDown(norm_x, norm_y)   == true)  return true;
	if (unlimited_saves.MouseDown(norm_x, norm_y)   == true)  return true;
	if (dm_help.MouseDown(norm_x, norm_y)   == true)  return true;

// SCG[6/5/00]: #ifndef TONGUE_GERMAN
#ifndef NO_VIOLENCE
	if (bloody_game_radio.MouseDown(norm_x, norm_y)   == true)  return true;
#endif
//    if (weapon_episode_picker.MouseDown(norm_x, norm_y) == true) return true;

    if (sbar_alpha.MouseDown(norm_x, norm_y) == true)
    {
      //float value = sbar_alpha.Position();   //read the value from the slider.
      //Cvar_SetValue("statusbar_alpha", value);
      //re.DrawPicPalette(OPTIONS_SBAR_PIC_X, OPTIONS_SBAR_PIC_Y,"pics/statusbar/future/sbar_head.bmp", statusbar_alpha->value, RESOURCE_EPISODE);

      mouseIsDown = true;
      return true;
    }


    return false;
}


bool CMenuSubOptions::MouseUp(int32 norm_x, int32 norm_y)
{
    mouseIsDown = false;

    if (always_run_radio.MouseUp(norm_x, norm_y) == true) 
    {
        ToggleRadioState(OPTIONS_ALWAYS_RUN_CVAR);
        return true;
    }
    
    if (lookspring_radio.MouseUp(norm_x, norm_y) == true)
    {
        ToggleRadioState(OPTIONS_LOOKSPRING_CVAR);
        return true;
    }
    
    if (lookstrafe_radio.MouseUp(norm_x, norm_y) == true)
    {
        ToggleRadioState(OPTIONS_LOOKSTRAFE_CVAR);
        return true;
    }
    
    if (auto_target_radio.MouseUp(norm_x, norm_y) == true)
    {
        ToggleRadioState(OPTIONS_AUTO_TARGET_CVAR);
        return true;
    }
    
    if (crosshair_picker.MouseUp(norm_x, norm_y) == true)
    {
        //get the value from the picker.
        int32 newvalue = crosshair_picker.CurrentIndex();

        //set the cvar value.
        Cvar_SetValue("crosshair", (float)newvalue);

        return true;
    }
   
    if (weapon_visible.MouseUp(norm_x, norm_y) == true)
    {
      ToggleRadioState(OPTIONS_WEAPON_VISIBLE_CVAR);
      return true;
    }

	if (subtitles_radio.MouseUp(norm_x, norm_y) == true)
    {
      ToggleRadioState(OPTIONS_SUBTITLES_CVAR);
      return true;
    }

	if (no_alttab_radio.MouseUp(norm_x, norm_y) == true)
    {
      ToggleRadioState(OPTIONS_NO_ALTTAB_CVAR);
      return true;
    }

	if (weapon_bob_radio.MouseUp(norm_x, norm_y) == true)
    {
      ToggleRadioState(OPTIONS_WEAPON_BOB_CVAR);
      return true;
    }

	if (weapon_autoswitch_radio.MouseUp(norm_x, norm_y) == true)
    {
      ToggleRadioState(OPTIONS_WEAPON_AUTOSWITCH_CVAR);
      return true;
    }
    
	if (play_intro_radio.MouseUp(norm_x, norm_y) == true)
    {
      ToggleRadioState(OPTIONS_PLAY_INTRO_CVAR);
      return true;
    }

	if (dm_taunts_radio.MouseUp(norm_x, norm_y) == true)
    {
      ToggleRadioState(OPTIONS_DM_TAUNTS_CVAR);
      return true;
    }

	if (unlimited_saves.MouseUp(norm_x, norm_y) == true)
    {
      ToggleRadioState(OPTIONS_UNLIMITED_SAVES_CVAR);
      return true;
    }

	if (dm_help.MouseUp(norm_x, norm_y) == true)
    {
      ToggleRadioState(OPTIONS_DM_HELP_CVAR);
      return true;
    }

// SCG[6/5/00]: #ifndef TONGUE_GERMAN
#ifndef NO_VIOLENCE
	if (bloody_game_radio.MouseUp(norm_x, norm_y) == true)
    {
      ToggleRadioState(OPTIONS_BLOODY_GAME_CVAR);
      return true;
    }
#endif
    /*
    if (weapon_episode_picker.MouseUp(norm_x, norm_y) == true) {
        //get the 
        int32 episode = weapon_episode_picker.CurrentIndex();

        return true;
    }
    */

    if (sbar_alpha.MouseUp(norm_x, norm_y) == true)
    {
      float value = sbar_alpha.Position();   //read the value from the slider.
      if (value > 1.0f)
        value = 1.0f;

      Cvar_SetValue("statusbar_alpha", value);
      return true;
    }

    return false;
}

