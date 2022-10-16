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
//  Video Menu
//
///////////////////////////////////////////////////////////////////////////////////

//#define FUCKEDPALETTETEST

#define VIDEO_LEFT											100
#define VIDEO_TOP											125
#define video_line_y(i)										(VIDEO_TOP + (i) * 18)
															
#define VIDEO_DRIVER_LIST_LENGTH							4
#define VIDEO_GLDRIVER_LIST_LENGTH							6
															
#define VIDEO_DRIVER_NAME_LEFT								(200)
#define VIDEO_DRIVER_NAME_RIGHT								(VIDEO_DRIVER_NAME_LEFT + 60)
															
#define VIDEO_DRIVER_HILIGHT_LEFT							(VIDEO_DRIVER_NAME_LEFT - 5)
#define VIDEO_DRIVER_HILIGHT_TOP							(video_line_y(0))
#define VIDEO_DRIVER_HILIGHT_RIGHT							(VIDEO_DRIVER_NAME_RIGHT + 1)
#define VIDEO_DRIVER_HILIGHT_BOTTOM							(video_line_y(1) + 1)


#define VIDEO_GLDRIVER_NAME_LEFT							(VIDEO_DRIVER_NAME_RIGHT + 8)
#define VIDEO_GLDRIVER_NAME_RIGHT							(VIDEO_GLDRIVER_NAME_LEFT + 200)
															
#define VIDEO_GLDRIVER_HILIGHT_LEFT							(VIDEO_GLDRIVER_NAME_LEFT - 5)
#define VIDEO_GLDRIVER_HILIGHT_TOP							(video_line_y(0) - 2)
#define VIDEO_GLDRIVER_HILIGHT_RIGHT						(VIDEO_GLDRIVER_NAME_RIGHT + 1)
#define VIDEO_GLDRIVER_HILIGHT_BOTTOM						(video_line_y(1) + 5)
													
#define video_driver_line_y(i)								(video_line_y(i + 2))
#define VIDEO_DRIVER_LIST_LEFT								(VIDEO_DRIVER_HILIGHT_LEFT)
#define VIDEO_DRIVER_LIST_TOP								(video_driver_line_y(-0.5) - 3)
//#define VIDEO_DRIVER_LIST_TOP (video_driver_line_y(0))
#define VIDEO_DRIVER_LIST_RIGHT								(VIDEO_DRIVER_HILIGHT_RIGHT)
#define VIDEO_DRIVER_LIST_BOTTOM							(video_driver_line_y( VIDEO_DRIVER_LIST_LENGTH ) + 4)

#define VIDEO_GLDRIVER_LIST_LEFT							(VIDEO_GLDRIVER_HILIGHT_LEFT)
#define VIDEO_GLDRIVER_LIST_TOP								(video_driver_line_y(-0.5))
#define VIDEO_GLDRIVER_LIST_RIGHT							(VIDEO_GLDRIVER_HILIGHT_RIGHT)
#define VIDEO_GLDRIVER_LIST_BOTTOM							(video_driver_line_y( VIDEO_GLDRIVER_LIST_LENGTH ) + 4)

#define VIDEO_SEPARATOR_TOP									(video_line_y(1.5))
#define VIDEO_SLIDER_LEFT									(VIDEO_LEFT)
#define VIDEO_SLIDER_WIDTH									(200)
#define VIDEO_SLIDER_HEIGHT									(16)
#define VIDEO_SLIDER_RIGHT									(VIDEO_SLIDER_LEFT + VIDEO_SLIDER_WIDTH)
#define VIDEO_SLIDER_VSPACING								(40)

#define VIDEO_RESOLUTION_TOP								(172)
#define VIDEO_BRIGHTNESS_TOP								(VIDEO_RESOLUTION_TOP + VIDEO_SLIDER_VSPACING) //165)

//#define VIDEO_SOFT_FULLSCREEN_TOP    (  0)
//#define VIDEO_SOFT_STIPPLE_TOP       (  0)

#define VIDEO_GL_TEXTURE_QUALITY_TOP						( VIDEO_BRIGHTNESS_TOP + VIDEO_SLIDER_VSPACING )

//#define VIDEO_GL_SHADOWS_TOP         (  0)
//#define VIDEO_GL_FULLSCREEN_TOP      (  0)

#define VIDEO_SCREEN_SIZE_TOP								(VIDEO_GL_TEXTURE_QUALITY_TOP + VIDEO_SLIDER_VSPACING)
															
#define VIDEO_TESSELLATE_TOP								( VIDEO_SCREEN_SIZE_TOP + VIDEO_SLIDER_VSPACING )

#define VIDEO_APPLY_LEFT									(180)
#define VIDEO_APPLY_TOP										(396)

#define VIDEO_OPTIONS_SPACING								(20)
#define VIDEO_OPTIONS_LEFT									(VIDEO_SLIDER_RIGHT + 20)
#define VIDEO_OPTIONS_TOP(idx)								(VIDEO_RESOLUTION_TOP + (VIDEO_OPTIONS_SPACING * idx))

#define VIDEO_FULLSCREEN_TOP								(VIDEO_OPTIONS_TOP(0))
#define VIDEO_SHADOWS_TOP									(VIDEO_OPTIONS_TOP(1))
#define VIDEO_ENVMAP_TOP									(VIDEO_OPTIONS_TOP(2))
#define VIDEO_BUlLETHOLES									(VIDEO_OPTIONS_TOP(3))
#define VIDEO_SHOW_FOG										(VIDEO_OPTIONS_TOP(4))
#define VIDEO_SNOW_RAIN										(VIDEO_OPTIONS_TOP(5))

#define VIDEO_MODULATE_TOP									(VIDEO_OPTIONS_TOP(7))
#define VIDEO_MODULATE_WIDTH								(100)

#define VIDEO_FULLSCREEN_CVAR								"vid_fullscreen"
#define VIDEO_SHADOWS_CVAR									"gl_shadows"
#define VIDEO_ENVMAP_CVAR									"gl_envmap"
#define VIDEO_BULLETHOLES_CVAR								"gl_surfacesprites"
#define VIDEO_SHOW_FOG_CVAR									"gl_vfog"
#define VIDEO_SHOW_FOGENTS_CVAR								"gl_vfogents"

#define VIDEO_SNOW_RAIN_CVAR								"cl_particlevol"

//cvars that we need access to.
extern cvar_t *vid_ref;
extern cvar_t *vid_gamma;
extern cvar_t *scr_viewsize;
extern cvar_t *vid_fullscreen;

static cvar_t *gl_mode;
static cvar_t *gl_driver;
static cvar_t *gl_picmip;
//static cvar_t *gl_subdivide_size;
static cvar_t *gl_shadows_cvar;
static cvar_t *gl_ext_palettedtexture;
//static cvar_t *sw_mode;
//static cvar_t *sw_stipplealpha;

//the names we display for the different modes.
static char *video_mode_names[VM_NUM_MODES];

array_obj<gl_driver_info_t>	gl_driver_info;
int							gl_driver_info_count;

void DKM_LoadVideoSettings(bool setDefaults);
bool DKM_VideoSettingsChanged();
void DKM_UpdateVideoSettings();

//the different resolutions available
static char *video_resolutions[] =
{
  "320 x 240",
  "400 x 300",
  "512 x 384",
  "640 x 480",
  "800 x 600",
  "960 x 720",
  "1024 x 768",
  "1152 x 864",
  "1280 x 960",
  "1600 x 1200"
};

static int32 video_num_resolutions = sizeof(video_resolutions)/sizeof(char *);

gl_driver_info_t	driverOpenGLdefault;
gl_driver_info_t	driver3DFXMiniGL;
gl_driver_info_t	driverNECMiniGL;

CMenuSubVideo::CMenuSubVideo()
{
	driverOpenGLdefault.szFilename.Set( "opengl32" );
	driverOpenGLdefault.szRenderer.Set( "OpenGL Default" );
	driver3DFXMiniGL.szFilename.Set( "3dfxgl" );
	driver3DFXMiniGL.szRenderer.Set( "Voodoo Mini ICD" );
	driverNECMiniGL.szFilename.Set( "pvrgl" );
	driverNECMiniGL.szRenderer.Set( "PVR Mini ICD" );

	gl_driver_info.Add( driverOpenGLdefault );
	gl_driver_info.Add( driver3DFXMiniGL );
	gl_driver_info.Add( driverNECMiniGL );
	gl_driver_info_count = gl_driver_info.Num();
		
//cur_mode = VM_SOFTWARE;
	cur_mode = VM_OPENGL;

    //set the names of the modes.
//	video_mode_names[VM_SOFTWARE]   = tongue_menu_options[T_MENU_VIDEO_MODE_1];
	video_mode_names[VM_OPENGL]		= tongue_menu_options[T_MENU_VIDEO_MODE_2];
//    video_mode_names[VM_GL_3DFX]    = tongue_menu_options[T_MENU_VIDEO_MODE_2];
//    video_mode_names[VM_GL_DEFAULT] = tongue_menu_options[T_MENU_VIDEO_MODE_3];
//    video_mode_names[VM_GL_POWERVR] = tongue_menu_options[T_MENU_VIDEO_MODE_4];

//	mode_hilight.Init(VIDEO_DRIVER_HILIGHT_LEFT, VIDEO_DRIVER_HILIGHT_TOP, VIDEO_DRIVER_HILIGHT_RIGHT, VIDEO_DRIVER_HILIGHT_BOTTOM, 2);
//	mode_list_box.Init(VIDEO_DRIVER_LIST_LEFT, VIDEO_DRIVER_LIST_TOP, VIDEO_DRIVER_LIST_RIGHT, VIDEO_DRIVER_LIST_BOTTOM, 2);

	driver_list_box.Init(VIDEO_GLDRIVER_LIST_LEFT, VIDEO_GLDRIVER_LIST_TOP, VIDEO_GLDRIVER_LIST_RIGHT, VIDEO_GLDRIVER_LIST_BOTTOM, 1);
	driver_hilight.Init(VIDEO_GLDRIVER_HILIGHT_LEFT, VIDEO_GLDRIVER_HILIGHT_TOP, VIDEO_GLDRIVER_HILIGHT_RIGHT, VIDEO_GLDRIVER_HILIGHT_BOTTOM, 1);

#ifndef FUCKEDPALETTETEST
	separator.Init(VIDEO_LEFT, video_line_y(1.5), VIDEO_GLDRIVER_LIST_RIGHT, 2);
#endif

    picker_modulate.AddString("1");
    picker_modulate.AddString("2");
    picker_modulate.AddString("3");
    picker_modulate.AddString("4");
    picker_modulate.AddString("5");
    picker_modulate.AddString("6");
    picker_modulate.AddString("7");
    picker_modulate.AddString("8");
    picker_modulate.AddString("9");
}

void CMenuSubVideo::Exit()
{
	vid_menu_mode = resolution_slider.Position();
	bound_min(vid_menu_mode,3);

	vid_brightness = brightness_slider.Position();

	texture_quality = gl_texture_slider.Position();

	scr_viewsize->value = screen_size_slider.Position();
    
//	tesselation_quality = ( 256 - int32(gl_tessellation_slider.Position() ) + 64 );

	modulate = picker_modulate.CurrentIndex() + 1;

    GetDriver();
	current_gl_driver = cur_driver;
}

void CMenuSubVideo::Enter()
{

    //get pointers to our vars.
    gl_mode = Cvar_Get("gl_mode", "3", CVAR_ARCHIVE);  // ttd: change default to 0 soon

	// quick boundary check
	if (gl_mode->value >= video_num_resolutions)
		Cvar_SetValue("gl_mode", 0);                         // set to default
	else if (gl_mode->value < 0)											// ditto
		Cvar_SetValue("gl_mode", 0);


	// TEMPORARY PRESS DEMO PURPOSE RESTRICTION... fix permantently later
	if (gl_mode->value < 3)											
	Cvar_SetValue("gl_mode", 3);												  // 640x480 minimum


    gl_driver = Cvar_Get("gl_driver", "opengl32", 0);
    gl_picmip = Cvar_Get("gl_picmip", "0", CVAR_ARCHIVE);
//    gl_subdivide_size = Cvar_Get("gl_subdivide_size", "0", CVAR_ARCHIVE);
    gl_ext_palettedtexture = Cvar_Get("gl_ext_palettedtexture", "0", 0);
    //sw_mode = Cvar_Get("sw_mode", "0", 0);
    //sw_stipplealpha = Cvar_Get("sw_stipplealpha", "0", CVAR_ARCHIVE);
    gl_shadows_cvar = Cvar_Get("gl_shadows", "0", CVAR_ARCHIVE);


    apply_changes.Init(VIDEO_APPLY_LEFT, VIDEO_APPLY_TOP, menu_font, menu_font_bright, button_font);
    apply_changes.InitGraphics(BUTTONLONG_UP_STRING, BUTTONLONG_DOWN_STRING, BUTTONLONG_DISABLE_STRING);
	//apply_changes.InitGraphics(BUTTONMEDIUM_UP_STRING, BUTTONMEDIUM_DOWN_STRING, BUTTONMEDIUM_DISABLE_STRING);
	apply_changes.SetText(tongue_menu_options[T_MENU_VIDEO_APPLY_CHANGES],true);


    resolution_slider.Init(VIDEO_SLIDER_LEFT, VIDEO_RESOLUTION_TOP, button_font, SLIDER_BACKGROUND_CHAR, SLIDER_TAB_CHAR);
    //resolution_slider.InitLinear(video_num_resolutions, 0, video_num_resolutions - 1);


	// TEMPORARY PRESS DEMO PURPOSE RESTRICTION... fix permantently later
	resolution_slider.InitLinear(video_num_resolutions - 3, 3, video_num_resolutions - 1);



    brightness_slider.Init(VIDEO_SLIDER_LEFT, VIDEO_BRIGHTNESS_TOP, button_font, SLIDER_BACKGROUND_CHAR, SLIDER_TAB_CHAR);
    brightness_slider.InitLinear(81, 0.5, 1.3);

    gl_texture_slider.Init(VIDEO_SLIDER_LEFT, VIDEO_GL_TEXTURE_QUALITY_TOP, button_font, SLIDER_BACKGROUND_CHAR, SLIDER_TAB_CHAR);
    gl_texture_slider.InitLinear(4, 3, 0);

	// SCG[8/26/99]: poly tessellation slider
//	gl_tessellation_slider.Init(VIDEO_SLIDER_LEFT, VIDEO_TESSELLATE_TOP, button_font, SLIDER_BACKGROUND_CHAR, SLIDER_TAB_CHAR);
//    gl_tessellation_slider.InitLinear(4, 64, 256);

		// screen size slider
    screen_size_slider.Init(VIDEO_SLIDER_LEFT, VIDEO_SCREEN_SIZE_TOP, button_font, SLIDER_BACKGROUND_CHAR, SLIDER_TAB_CHAR);
    screen_size_slider.InitLinear(9, 40, 120); // steps, low value, high value


    //set up the radio buttons.
	btn_fullscreen.Init(VIDEO_OPTIONS_LEFT,VIDEO_FULLSCREEN_TOP,menu_font,menu_font_bright, tongue_menu_options[T_MENU_VIDEO_FULLSCREEN]);
	btn_shadows.Init(VIDEO_OPTIONS_LEFT,VIDEO_SHADOWS_TOP,menu_font,menu_font_bright, tongue_menu_options[T_MENU_VIDEO_SHADOWS]);
	btn_envMap.Init(VIDEO_OPTIONS_LEFT,VIDEO_ENVMAP_TOP,menu_font,menu_font_bright, tongue_menu[T_MENU_ENV_MAP]);
	btn_bulletholes.Init(VIDEO_OPTIONS_LEFT,VIDEO_BUlLETHOLES,menu_font,menu_font_bright, tongue_menu[T_MENU_BULLET_HOLES]);
	btn_showFog.Init(VIDEO_OPTIONS_LEFT,VIDEO_SHOW_FOG,menu_font,menu_font_bright, tongue_menu_options[T_MENU_VIDEO_SHOW_FOG]);
	btn_snow_rain.Init(VIDEO_OPTIONS_LEFT,VIDEO_SNOW_RAIN,menu_font,menu_font_bright, tongue_menu_options[T_MENU_VIDEO_SNOW_RAIN]);

	picker_modulate.Init(VIDEO_OPTIONS_LEFT,VIDEO_MODULATE_TOP, VIDEO_MODULATE_WIDTH, menu_font, menu_font_bright, button_font, tongue_menu[T_MENU_MODULATE]);

    //soft_fullscreen.Init(VIDEO_SLIDER_LEFT, VIDEO_SOFT_FULLSCREEN_TOP, menu_font, menu_font_bright, tongue_menu_options[T_MENU_VIDEO_FULLSCREEN]);
    //soft_stipple_alpha.Init(VIDEO_SLIDER_LEFT, VIDEO_SOFT_STIPPLE_TOP, menu_font, menu_font_bright, tongue_menu_options[T_MENU_VIDEO_STIPPLE_ALPHA]);

    //gl_shadows_b.Init(VIDEO_SLIDER_LEFT, VIDEO_GL_SHADOWS_TOP, menu_font, menu_font_bright, tongue_menu_options[T_MENU_VIDEO_SHADOWS]);
    //gl_fullscreen.Init(VIDEO_SLIDER_LEFT, VIDEO_GL_FULLSCREEN_TOP, menu_font, menu_font_bright, tongue_menu_options[T_MENU_VIDEO_FULLSCREEN]);

	// SCG[6/29/99]: get the current driver. has to come before GetMode()
    GetDriver();

    //get the current mode.
    GetMode();

//    picking_mode = false;
//    mode_hilighted = false;

    ModeListHilight(-1);

	picking_driver = false;
    driver_hilighted = false;
    DriverListHilight(-1);
}

//returns value in range of .5 to 1.3.  1.3 is brightest.  For some totally
//fucked up reason however, the range that is stored in the actual cvar is inverted, so that
//.5 is brightest.  We must invert the range when we read or write to the cvar.
float GetCurrentBrightness() {
    //move the range from 0.5->1.3 to 0.0->0.8
    float zero_offset = vid_gamma->value - 0.5f;
    //invert it.
    float inverted = 0.8f - zero_offset;

    //add back the offset
    return inverted + 0.5f;
}

void CMenuSubVideo::GetDriver() {
	gl_driver_info_t driver;

//#pragma message( "enumeration hack" )
	// SCG[5/26/00]: Should never get here because of manual driver additions in CMenuSubVideo::CMenuSubVideo()
/*
	if( gl_driver_info_count == 0 )
	{
		driver.szFilename.Set( gl_driver->string );
		if( strstr( gl_driver->string, "opengl32" ) )
		{
			driver.szRenderer.Set( "OpenGL Default" );
		}
		else
		{
			driver.szRenderer.Set( "Mini ICD" );
		}

		gl_driver_info.Add( driver );
		gl_driver_info_count = 1;
	}
*/
	cur_driver = 0;
	for( int i = 0; i < gl_driver_info_count; i++ )
	{
		driver = gl_driver_info.Item( i );
		if( !strcmp( driver.szFilename, gl_driver->string ) )
		{
			cur_driver = i;
		}
	}
}

void CMenuSubVideo::GetMode() {
    //get the name of the refresh dll.
/*
    if (stricmp(vid_ref->string, "gl") == 0) {
        //we are in a opengl mode.
        if (stricmp(gl_driver->string, "3dfxgl") == 0) {
            //3dfx driver
            cur_mode = VM_GL_3DFX;
        }
        else if (stricmp(gl_driver->string, "pvrgl") == 0) {
            //powerVR
            cur_mode = VM_GL_POWERVR;
        }
        else {
            //default
            cur_mode = VM_GL_DEFAULT;
        }
    }
*/


    if (stricmp(vid_ref->string, "gl") == 0) //{
        cur_mode = VM_OPENGL;
//	}
    //else {
        ////assume we are in the software renderer
        //cur_mode = VM_SOFTWARE;
    //}

    //get the position of the resolution slider.
    //if (cur_mode == VM_SOFTWARE)
    //{
    //    resolution_slider.Position(sw_mode->value);
    //}
    //else 
/*    if (gl_mode(cur_mode))
    {
	    // TEMPORARY PRESS DEMO PURPOSE RESTRICTION... fix permantently later
	    if (gl_mode->value < 3)											
        Cvar_SetValue("gl_mode", 3);									  // 640x480 minimum

      resolution_slider.Position(gl_mode->value);
    }

    //get screen size
    screen_size_slider.Position(scr_viewsize->value);

    //get brightness
    brightness_slider.Position(GetCurrentBrightness());

    //get the texture quality value.
    gl_texture_slider.Position(gl_picmip->value);

    //get the texture quality value.
	int32 tessellation_quality = ( 256 - int32(gl_subdivide_size->value + 0.001f) ) + 64;
    gl_tessellation_slider.Position(tessellation_quality);

    //save the values of the settings that have to be confirmed before changing.
    start_mode = cur_mode;
	start_driver = cur_driver;
    start_res = int32(resolution_slider.Position() + 0.001f);


    // TEMPORARY PRESS DEMO PURPOSE RESTRICTION... fix permantently later
		if (start_res < 3)
      start_res = 3;


    start_fullscreen = fullscreen = vid_fullscreen->value != 0.0f;
    //start_gl_8bit = gl_8bit = gl_ext_palettedtexture->value != 0.0f;
    
    //start_gl_shadows = gl_shadows = int32(gl_shadows_cvar->value);
    start_gl_shadows = gl_shadows = int32(use_shadows);
    
    start_gl_quality = int32(gl_picmip->value);
	start_tessellate_quality = tessellation_quality;

    start_gl_gamma = GetCurrentBrightness();
*/
	resolution_slider.Position(vid_menu_mode);
    brightness_slider.Position(vid_brightness);
    gl_texture_slider.Position(texture_quality);
    screen_size_slider.Position(scr_viewsize->value);
//    gl_tessellation_slider.Position(( 256 - int32(tesselation_quality) + 64 ));

	picker_modulate.SetCurrentString(modulate - 1);
}


void CMenuSubVideo::SelectMode(video_mode mode)
{
    cur_mode = mode;

    //get the position of the resolution slider.
    //if (cur_mode == VM_SOFTWARE)
    //{
    //    resolution_slider.Position(sw_mode->value);
    //}
    //else 
    if (gl_mode(cur_mode)) 
    {
	    // TEMPORARY PRESS DEMO PURPOSE RESTRICTION... fix permantently later
	    if (gl_mode->value < 3)											
        Cvar_SetValue("gl_mode", 3);												  // 640x480 minimum

      resolution_slider.Position(gl_mode->value);
    }
}

void CMenuSubVideo::ModeListHilight(int32 line) {
    mode_list_hilight = line;

    if (line != -1) {
        //move the hilight box.
        //mode_list_hilight_box.Init(VIDEO_DRIVER_LIST_LEFT + 3, video_driver_line_y(line) - 3, 
        //                           VIDEO_DRIVER_LIST_RIGHT - 3, video_driver_line_y(line + 1), 2);

		mode_list_hilight_box.Init(VIDEO_DRIVER_LIST_LEFT  + 2, video_driver_line_y(line) - 1 , 
									VIDEO_DRIVER_LIST_RIGHT - 2, video_driver_line_y(line + 2), 2);


    }
}

void CMenuSubVideo::DriverListHilight(int32 line) {
    driver_list_hilight = line;

    if (line != -1) {
        //move the hilight box.
//        driver_list_hilight_box.Init(VIDEO_GLDRIVER_LIST_LEFT + 3, video_driver_line_y(line) - 3, 
//                                   VIDEO_GLDRIVER_LIST_RIGHT - 3, video_driver_line_y(line + 1), 2);

//        driver_list_hilight_box.Init(VIDEO_GLDRIVER_LIST_LEFT  + 2, video_driver_line_y(line) - 2, 
//                                     VIDEO_GLDRIVER_LIST_RIGHT - 2, video_driver_line_y(line + 1) + 4, 2);
        driver_list_hilight_box.Init(VIDEO_GLDRIVER_LIST_LEFT, video_driver_line_y(line), 
                                     VIDEO_GLDRIVER_LIST_RIGHT, video_driver_line_y(line + 1), 2);


    }
}

bool CMenuSubVideo::ShowApplyChanges()
{
/*    if (start_mode != cur_mode) return true;
    if (start_driver != cur_driver) return true;
    if (start_res != int32(resolution_slider.Position() + 0.001f)) return true;
    if (start_fullscreen != fullscreen) return true;
    if (start_gl_shadows != gl_shadows) return true;
    if (start_gl_quality != int32(gl_texture_slider.Position() + 0.001f)) return true;
    if (start_tessellate_quality != int32(gl_tessellation_slider.Position() + 0.001f)) return true;

    //check if the current mode is a gl mode.
    if (gl_mode(cur_mode) && fabs(start_gl_gamma - brightness_slider.Position()) > 0.001f) return true;

    return false;*/
	current_gl_driver = cur_driver;
	return DKM_VideoSettingsChanged();
}

void CMenuSubVideo::Animate(int32 elapsed_time) {
    //soft_fullscreen.Animate(elapsed_time);
    //soft_stipple_alpha.Animate(elapsed_time);

    //gl_shadows_b.Animate(elapsed_time);
    //gl_fullscreen.Animate(elapsed_time);

    apply_changes.Animate(elapsed_time);
}


void CMenuSubVideo::DrawBackground()
{
//    if (picking_mode == false && picking_driver == false)
    if (picking_driver == false)
    {
        //draw the resolution slider.
        resolution_slider.DrawBackground();

        //draw screen size slider.
        screen_size_slider.DrawBackground();

        //draw the text that displays the current screen size selection now, because
        //the buttons on the right of the screen should draw on top of it.

        //int32 screen_size = int32((screen_size_slider.Position() + 0.001f - 40) / 10);
        //buffer32 screensize("%d%%",(screen_size+1) * 10);
        //int32    screen_size = int32((screen_size_slider.Position() + 0.001f - 40) / 10);

				/*
			  char       buf[64];
        DRAWSTRUCT drawStruct;

    		sprintf(buf,"pics/screensize/screensize_%3.3d.bmp",(int)scr_viewsize->value);
		
		    drawStruct.pImage = re.RegisterPic( buf, NULL, NULL, RESOURCE_INTERFACE );
		    drawStruct.nFlags = DSFLAG_PALETTE; // | DSFLAG_BLEND;
		    drawStruct.nXPos = 160;
		    drawStruct.nYPos = 120;
		    re.DrawPic( drawStruct );
				*/



        //draw brightness slider.
        brightness_slider.DrawBackground();

        //check our mode.
        if (gl_mode(cur_mode))
        {
            //draw texture quality slider.
            gl_texture_slider.DrawBackground();
//            gl_tessellation_slider.DrawBackground();
			btn_fullscreen.DrawBackground();
			btn_shadows.DrawBackground();
			btn_envMap.DrawBackground();
			btn_bulletholes.DrawBackground();
			picker_modulate.DrawBackground();
			btn_showFog.DrawBackground();
			btn_snow_rain.DrawBackground();
        }

        //draw apply changes button.
        if (ShowApplyChanges() == true)
        {
            apply_changes.DrawBackground();
        }
    }
}

void CMenuSubVideo::DrawForeground()
{
	gl_driver_info_t	driver;
	//DKM_DrawString(VIDEO_LEFT, video_line_y(0), "Driver", NULL, false, false);
	DKM_DrawString(VIDEO_LEFT, video_line_y(0), tongue_menu_options[T_MENU_VIDEO_DRIVER], NULL, false, false);

	DKM_DrawString(VIDEO_DRIVER_NAME_LEFT, video_line_y(0), video_mode_names[cur_mode], NULL, false, false);

	if( gl_mode( cur_mode ) ) 
	{
		driver = gl_driver_info.Item( cur_driver );
		DKM_DrawString(VIDEO_GLDRIVER_NAME_LEFT, video_line_y(0), ( const char * ) driver.szRenderer, NULL, false, false);
	}
/*
  if (picking_mode == true)
  {
    //draw the list of modes.
    for (int32 i = 0; i < VM_NUM_MODES; i++)
      DKM_DrawString(VIDEO_DRIVER_LIST_LEFT + 6, video_driver_line_y(i), video_mode_names[i], NULL, false, false);
  }
	else 
*/
	if( picking_driver == true )
	{
		for( int32 i = 0; i < gl_driver_info_count; i++ )
		{
			driver = gl_driver_info.Item( i );
			DKM_DrawString(VIDEO_GLDRIVER_LIST_LEFT + 6, video_driver_line_y(i), ( const char * ) driver.szRenderer, NULL, false, false);
		}
	}
    else 
    {
      //draw the resolution slider and labels.
      resolution_slider.DrawForeground();
      
      // header
//      DKM_DrawString(VIDEO_SLIDER_LEFT, VIDEO_RESOLUTION_TOP - VIDEO_SLIDER_HEIGHT, tongue_menu_options[T_MENU_VIDEO_RESOLUTION], NULL, false, false);
 
			// value
      int32 resolution = int32(resolution_slider.Position() + 0.001f);  //the selected resolution.

			// TEMPORARY PRESS DEMO PURPOSE RESTRICTION... fix permantently later
      if (resolution < 3)
        resolution = 3;


//      DKM_DrawString(VIDEO_SLIDER_RIGHT + 10, VIDEO_RESOLUTION_TOP, video_resolutions[resolution], NULL, false, false);

	  buffer64 strBuf("%s - %s",tongue_menu_options[T_MENU_VIDEO_RESOLUTION],video_resolutions[resolution]);
// Encompass MarkMa 041299
#ifdef JPN	// JPN - changed by yokoyama
      DKM_DrawString(VIDEO_SLIDER_LEFT, VIDEO_RESOLUTION_TOP /*-2*/ - VIDEO_SLIDER_HEIGHT, strBuf, NULL, false, false);
#else	// JPN
      DKM_DrawString(VIDEO_SLIDER_LEFT, VIDEO_RESOLUTION_TOP - VIDEO_SLIDER_HEIGHT, strBuf, NULL, false, false);
#endif	// JPN
// Encompass MarkMa 041299



      //draw brightness text.
      brightness_slider.DrawForeground();
//      DKM_DrawString(VIDEO_SLIDER_LEFT, VIDEO_BRIGHTNESS_TOP - VIDEO_SLIDER_HEIGHT, tongue_menu_options[T_MENU_VIDEO_BRIGHTNESS], NULL, false, false);

//      buffer64 buf("%d%%", int32(0.1f + 100 * brightness_slider.Position()));
//      DKM_DrawString(VIDEO_SLIDER_RIGHT + 10, VIDEO_BRIGHTNESS_TOP, buf, NULL, false, false);
	strBuf.Set("%s - %d%%",tongue_menu_options[T_MENU_VIDEO_BRIGHTNESS],int32(0.1f + 100 * brightness_slider.Position()));
// Encompass MarkMa 041299
#ifdef JPN	// JPN - changed by yokoyama
	DKM_DrawString(VIDEO_SLIDER_LEFT, VIDEO_BRIGHTNESS_TOP /*-2*/ - VIDEO_SLIDER_HEIGHT, strBuf, NULL, false, false);
#else	// JPN
	DKM_DrawString(VIDEO_SLIDER_LEFT, VIDEO_BRIGHTNESS_TOP - VIDEO_SLIDER_HEIGHT, strBuf, NULL, false, false);
#endif	// JPN
// Encompass MarkMa 041299
     
			//draw screen size slider and labels.
    screen_size_slider.DrawForeground();
//	strBuf.Set("%s - %d%%",tongue_menu_options[T_MENU_VIDEO_SCREEN_SIZE],(int)scr_viewsize->value);
	strBuf.Set("%s - %d%%",tongue_menu_options[T_MENU_VIDEO_SCREEN_SIZE],(int)screen_size_slider.Position());

	DKM_DrawString(VIDEO_SLIDER_LEFT, VIDEO_SCREEN_SIZE_TOP - VIDEO_SLIDER_HEIGHT, strBuf, NULL, false, false);
//        buffer32 screensize("%d%%",(int)scr_viewsize->value);

//        DKM_DrawString(VIDEO_SLIDER_RIGHT + 10, VIDEO_SCREEN_SIZE_TOP, screensize, NULL, false, false);


      //check what mode we are in.
      //if (cur_mode == VM_SOFTWARE)
      //{
        //software mode.
        //check if we are fullscreen.
        //soft_fullscreen.Check(fullscreen);
        //soft_fullscreen.DrawForeground();

        //check for stipple alpha
      //  soft_stipple_alpha.Check(sw_stipplealpha->value != 0);
      //  soft_stipple_alpha.DrawForeground();
      //}
      //else 
        if (gl_mode(cur_mode))
        {
			//gl mode.
//		    SetRadioState(btn_fullscreen, VIDEO_FULLSCREEN_CVAR);
			btn_fullscreen.Check(is_fullscreen);
			btn_fullscreen.DrawForeground();

//		    SetRadioState(btn_shadows, VIDEO_SHADOWS_CVAR);
			btn_shadows.Check(use_shadows > 0.0f);
			btn_shadows.DrawForeground();

		    SetRadioState(btn_envMap, VIDEO_ENVMAP_CVAR);
			btn_envMap.DrawForeground();

		    SetRadioState(btn_bulletholes, VIDEO_BULLETHOLES_CVAR);
			btn_bulletholes.DrawForeground();

		    SetRadioState(btn_showFog, VIDEO_SHOW_FOG_CVAR);
			btn_showFog.DrawForeground();

		    SetRadioState(btn_snow_rain, VIDEO_SNOW_RAIN_CVAR);
			btn_snow_rain.DrawForeground();

			//draw the texture quality scroll bar and labels.
			gl_texture_slider.DrawForeground();

			int32 quality = int32(gl_texture_slider.Position() + 0.001f);
			strBuf.Set("%s - %s",tongue_menu_options[T_MENU_VIDEO_TEXTURE_QUALITY],texture_quality_desc[quality]);
// Encompass MarkMa 041299
#ifdef JPN	// JPN - changed by yokoyama
			DKM_DrawString(VIDEO_SLIDER_LEFT, VIDEO_GL_TEXTURE_QUALITY_TOP /* -2 */- VIDEO_SLIDER_HEIGHT, strBuf, NULL, false, false);
#else	// JPN
			DKM_DrawString(VIDEO_SLIDER_LEFT, VIDEO_GL_TEXTURE_QUALITY_TOP - VIDEO_SLIDER_HEIGHT, strBuf, NULL, false, false);
#endif	// JPN
// Encompass MarkMa 041299
/*
			gl_tessellation_slider.DrawForeground();

			quality = ( 256 - int32(gl_tessellation_slider.Position() + 0.001f) ) + 64;
			quality /= 64;
			strBuf.Set("%s - %s",tongue_menu_options[T_MENU_OPTS_TESSELLATION],texture_quality_desc[quality - 1]);
			DKM_DrawString(VIDEO_SLIDER_LEFT, VIDEO_TESSELLATE_TOP - VIDEO_SLIDER_HEIGHT, strBuf, NULL, false, false);
*/
			picker_modulate.DrawForeground();
        }


        //draw apply changes button.
        if (ShowApplyChanges() == true)
            apply_changes.DrawForeground();
    }
}


void CMenuSubVideo::PlaceEntities() {
/*
    if (mode_hilighted == true || picking_mode == true) {
        mode_hilight.PlaceEntities();
    }
	else */if( driver_hilighted == true || picking_driver == true ) {
		driver_hilight.PlaceEntities();
	}

/*
    if (picking_mode == true) {
        mode_list_box.PlaceEntities();

        if (mode_list_hilight != -1) {
            mode_list_hilight_box.PlaceEntities();
        }
    }
	else */if( picking_driver == true ) {
		driver_list_box.PlaceEntities();

        if (driver_list_hilight != -1) {
            driver_list_hilight_box.PlaceEntities();
		}
	}
    else {
        //draw the separating line.

#ifndef FUCKEDPALETTETEST
        separator.PlaceEntities();
#endif

    if (gl_mode(cur_mode))
		picker_modulate.PlaceEntities();
    }
}

bool CMenuSubVideo::Keydown(int32 key) {
    switch (key) {
        case K_ESCAPE: {
/*
            if (picking_mode == true) {
                picking_mode = false;
            }
            else */if (picking_driver == true) {
                picking_driver = false;
            }
            else {
                //leave the menu.
                //DKM_EnterMain();
				if( Cvar_VariableValue( "console" ) == 0 )
				{
					return false;
				}
                DKM_Shutdown();       // exit entire interface  1.10 dsn
            }
            return true;
        }
    }

    return false;
}

bool CMenuSubVideo::MousePos(int32 norm_x, int32 norm_y) 
{
///*
    //check if we are over the driver name.
    if (norm_x > VIDEO_DRIVER_HILIGHT_LEFT && norm_x < VIDEO_DRIVER_HILIGHT_RIGHT &&
        norm_y > VIDEO_DRIVER_HILIGHT_TOP && norm_y < VIDEO_DRIVER_HILIGHT_BOTTOM)
    {
        //we are on the driver name.
        mode_hilighted = true;
        driver_hilighted = false;
    }
    else 
    if (norm_x > VIDEO_GLDRIVER_HILIGHT_LEFT && norm_x < VIDEO_GLDRIVER_HILIGHT_RIGHT  &&
        //norm_y > VIDEO_GLDRIVER_HILIGHT_TOP  && norm_y < VIDEO_GLDRIVER_HILIGHT_BOTTOM && ( cur_mode != VM_SOFTWARE ) )
				norm_y > VIDEO_GLDRIVER_HILIGHT_TOP  && norm_y < VIDEO_GLDRIVER_HILIGHT_BOTTOM)
	{
        driver_hilighted = true;
        mode_hilighted = false;
	}
    else {
        //we are not over the driver name.
        mode_hilighted = false;
        driver_hilighted = false;
    }
//*/
/*
    if (picking_mode == true) {
        //the mode list is down.
        int32 hilight = list_box_hit_test(VIDEO_DRIVER_LIST_LEFT, video_driver_line_y(0), 
                                        VIDEO_DRIVER_LIST_RIGHT, video_driver_line_y( VM_NUM_MODES ), 
                                        norm_x, norm_y, VM_NUM_MODES );

        ModeListHilight(hilight);
    }
    else */if (picking_driver == true) {
        //the mode list is down.
        int32 hilight = list_box_hit_test(VIDEO_GLDRIVER_LIST_LEFT, video_driver_line_y(0), 
                                        VIDEO_GLDRIVER_LIST_RIGHT, video_driver_line_y(gl_driver_info_count), 
                                        norm_x, norm_y, gl_driver_info_count);

        DriverListHilight(hilight);
    }
    else {
        bool ret = false;
        //the controls are showing.
        if (resolution_slider.MousePos(norm_x, norm_y) == true) ret = true;
        if (screen_size_slider.MousePos(norm_x, norm_y) == true) ret = true;
        if (brightness_slider.MousePos(norm_x, norm_y) == true) ret = true;

        //check our mode.
        //if (cur_mode == VM_SOFTWARE) {
            //software mode.
        //    if (soft_fullscreen.MousePos(norm_x, norm_y) == true) ret = true;
        //    if (soft_stipple_alpha.MousePos(norm_x, norm_y) == true) ret = true;
        //}
        //else 
          if (gl_mode(cur_mode)) {
            //gl mode.
            if (gl_texture_slider.MousePos(norm_x, norm_y) == true) ret = true;
//            if (gl_tessellation_slider.MousePos(norm_x, norm_y) == true) ret = true;
            if (btn_shadows.MousePos(norm_x, norm_y) == true) ret = true;
            if (btn_envMap.MousePos(norm_x, norm_y) == true) ret = true;
            if (btn_bulletholes.MousePos(norm_x, norm_y) == true) ret = true;
            if (btn_fullscreen.MousePos(norm_x, norm_y) == true) ret = true;
			if (btn_showFog.MousePos(norm_x, norm_y) == true) ret = true;
			if (btn_snow_rain.MousePos(norm_x, norm_y) == true) ret = true;
			if (picker_modulate.MousePos(norm_x,norm_y)) ret = true;
        }

        if (ShowApplyChanges() == true && apply_changes.MousePos(norm_x, norm_y) == true) ret = true;

        if (ret == true) return true;
    }

    return false;
}

bool CMenuSubVideo::MouseDown(int32 norm_x, int32 norm_y) {
    //check if we clicked on the mode
///*
    if (norm_x > VIDEO_DRIVER_HILIGHT_LEFT && norm_x < VIDEO_DRIVER_HILIGHT_RIGHT &&
        norm_y > VIDEO_DRIVER_HILIGHT_TOP && norm_y < VIDEO_DRIVER_HILIGHT_BOTTOM && !picking_driver)
    {
        //we are on the driver name.
        picking_mode = true;
        return true;
    }
	else 
//*/
  if (norm_x > VIDEO_GLDRIVER_HILIGHT_LEFT && norm_x < VIDEO_GLDRIVER_HILIGHT_RIGHT &&
//			norm_y > VIDEO_GLDRIVER_HILIGHT_TOP && norm_y < VIDEO_GLDRIVER_HILIGHT_BOTTOM && !picking_mode)
			norm_y > VIDEO_GLDRIVER_HILIGHT_TOP && norm_y < VIDEO_GLDRIVER_HILIGHT_BOTTOM )
    {
        //we are on the driver name.
        picking_driver = true;
        return true;
    }

/*
    if (picking_mode == true) {
        //check if we clicked on one of the modes.
        int32 select = list_box_hit_test(VIDEO_DRIVER_LIST_LEFT, video_driver_line_y(0), 
                                        VIDEO_DRIVER_LIST_RIGHT, video_driver_line_y(VM_NUM_MODES), 
                                        norm_x, norm_y, VM_NUM_MODES);

        //whatever happens, we're done picking the mode.
        picking_mode = false;

        if (select == -1) {
            //stop picking the mode.
            return false;
        }
        else {
            //they picked a new mode.
            SelectMode((video_mode)select);
            return true;
        }
    }
	else */if (picking_driver == true) {
        //check if we clicked on one of the modes.
        int32 select = list_box_hit_test(VIDEO_GLDRIVER_LIST_LEFT, video_driver_line_y(0), 
                                        VIDEO_GLDRIVER_LIST_RIGHT, video_driver_line_y(gl_driver_info_count), 
                                        norm_x, norm_y, gl_driver_info_count);

        //whatever happens, we're done picking the mode.
        picking_driver = false;

        if (select == -1) {
            //stop picking the mode.
            return false;
        }
        else {
            //they picked a new mode.
            cur_driver = select;
			if( cur_driver >= gl_driver_info_count )
			{
				cur_driver = gl_driver_info_count - 1;
			}
            return true;
        }
	}
    else {
        //the controls are showing.
        if (resolution_slider.MouseDown(norm_x, norm_y) == true) return true;
        if (screen_size_slider.MouseDown(norm_x, norm_y) == true) return true;
        if (brightness_slider.MouseDown(norm_x, norm_y) == true) return true;

        //check our mode.
        //if (cur_mode == VM_SOFTWARE)
        //{
            //software mode.
            //if (soft_fullscreen.MouseDown(norm_x, norm_y) == true) return true;
            //if (soft_stipple_alpha.MouseDown(norm_x, norm_y) == true) return true;
        //}
        //else
        if (gl_mode(cur_mode))
        {
            //gl mode.
            if (gl_texture_slider.MouseDown(norm_x, norm_y) == true) return true;
//            if (gl_tessellation_slider.MouseDown(norm_x, norm_y) == true) return true;
            if (btn_shadows.MouseDown(norm_x, norm_y) == true) return true;
            if (btn_envMap.MouseDown(norm_x, norm_y) == true) return true;
            if (btn_bulletholes.MouseDown(norm_x, norm_y) == true) return true;
            if (btn_fullscreen.MouseDown(norm_x, norm_y) == true) return true;
			if (btn_showFog.MouseDown(norm_x, norm_y) == true) return true;
			if (btn_snow_rain.MouseDown(norm_x, norm_y) == true) return true;
			if (picker_modulate.MouseDown(norm_x,norm_y)) return true;
        }

        if (ShowApplyChanges() == true && apply_changes.MouseDown(norm_x, norm_y) == true) return true;
    }

    return false;
}

bool CMenuSubVideo::MouseUp(int32 norm_x, int32 norm_y)
{
/*
    if (picking_mode == true)
    {
        //check if we clicked on one of the modes.
        int32 select = list_box_hit_test(VIDEO_DRIVER_LIST_LEFT, video_driver_line_y(0), 
                                        VIDEO_DRIVER_LIST_RIGHT, video_driver_line_y(VM_NUM_MODES), 
                                        norm_x, norm_y, VM_NUM_MODES);

        if (select != -1)
        {
            //they picked a new mode.
            SelectMode((video_mode)select);
            picking_mode = false;
            return true;
        }
    }
*/	
	if( picking_driver == true )
	{
        //check if we clicked on one of the modes.
        int32 select = list_box_hit_test(VIDEO_GLDRIVER_LIST_LEFT, video_driver_line_y(0), 
                                        VIDEO_GLDRIVER_LIST_RIGHT, video_driver_line_y(gl_driver_info_count), 
                                        norm_x, norm_y, gl_driver_info_count);

        if (select != -1)
        {
            //they picked a new mode.
			cur_driver = select;
			if( cur_driver >= gl_driver_info_count )
			{
				cur_driver = gl_driver_info_count - 1;
			}
            picking_driver = false;
            return true;
        }
	}
    else
    {
        //the controls are showing.
        if (resolution_slider.MouseUp(norm_x, norm_y) == true) 
		{
			Exit();
            return true;
        }
        if (screen_size_slider.MouseUp(norm_x, norm_y) == true) 
		{
            //set the value of the cvar that controls screen size.
            Cvar_SetValue("viewsize", screen_size_slider.Position());

            return true;
        }
        if (brightness_slider.MouseUp(norm_x, norm_y) == true) 
		{
			Exit();
            return true;
        }

        //check our mode.
        //if (cur_mode == VM_SOFTWARE) {
        //    //software mode.
        //    if (soft_fullscreen.MouseUp(norm_x, norm_y) == true) {
        //        fullscreen = !fullscreen;
        //        return true;
        //    }
        //    if (soft_stipple_alpha.MouseUp(norm_x, norm_y) == true) {
        //        Cvar_SetValue("sw_stipplealpha", ((sw_stipplealpha->value != 0.0f) ? 0.0f : 1.0f));
        //        return true;
        //    }
        //}
        //else 
        if (gl_mode(cur_mode))
        {
            //gl mode.
            if (gl_texture_slider.MouseUp(norm_x, norm_y) == true)
			{
				Exit();
				return true;
			}
/*
            if (gl_tessellation_slider.MouseUp(norm_x, norm_y) == true)
			{
				Exit();
				return true;
			}
*/            
            if (btn_shadows.MouseUp(norm_x, norm_y) == true)
            {
				//ToggleRadioState(VIDEO_SHADOWS_CVAR);
				use_shadows = (use_shadows > 0.0f) ? 0 : 1;
                return true;
            }
            if (btn_fullscreen.MouseUp(norm_x, norm_y) == true)
            {
				is_fullscreen = !is_fullscreen;
//				ToggleRadioState(VIDEO_FULLSCREEN_CVAR);
                return true;
            }

            if (btn_bulletholes.MouseUp(norm_x, norm_y) == true)
            {
				ToggleRadioState(VIDEO_BULLETHOLES_CVAR);
                return true;
            }
            if (btn_envMap.MouseUp(norm_x, norm_y) == true)
            {
				ToggleRadioState(VIDEO_ENVMAP_CVAR);
                return true;
            }
            
			if (btn_showFog.MouseUp(norm_x, norm_y) == true)
            {
				ToggleRadioState(VIDEO_SHOW_FOG_CVAR);
				ToggleRadioState(VIDEO_SHOW_FOGENTS_CVAR);
                return true;
            }

			if (btn_snow_rain.MouseUp(norm_x, norm_y) == true)
            {
				ToggleRadioState(VIDEO_SNOW_RAIN_CVAR);
                return true;
            }

			if (picker_modulate.MouseUp(norm_x, norm_y) == true)
			{
				Exit();
				//get the value from the picker.
//				int32 newvalue = picker_modulate.CurrentIndex();

				//set the cvar value.
//				Cvar_SetValue("gl_modulate", (float)newvalue + 1);

				return true;
			}
        }

        if (ShowApplyChanges() == true && apply_changes.MouseUp(norm_x, norm_y) == true) {
            //apply the changes to the video mode.
            ApplyChanges();

			if (in_main_game())
			    M_ForceMenuOff();

            return true;
        }
    }

    return false;
}

void CMenuSubVideo::ApplyChanges()
{
	DKM_UpdateVideoSettings();
	return;
    //set the refresh dll name.

//    if (cur_mode == VM_SOFTWARE) {
//        Cvar_Set("vid_ref", "soft");
//    }
//	else {
/*
	gl_driver_info_t	driver = gl_driver_info.Item( cur_driver );

    Cvar_Set("vid_ref", "gl");
    Cvar_Set("gl_driver", driver.szFilename );
*/

//	}

	// SCG[5/26/99]: Added cast to remove fractional componant of sw_mode/gl_mode
    //set the resolution.
    //if (cur_mode == VM_SOFTWARE) {
    //    Cvar_SetValue("sw_mode", (int32)resolution_slider.Position());
    //}
    //else 
    if (gl_mode(cur_mode))
    {
      Cvar_SetValue("gl_mode", (int32)resolution_slider.Position());

	    // TEMPORARY PRESS DEMO PURPOSE RESTRICTION... fix permantently later
	    if (gl_mode->value < 3)											
        Cvar_SetValue("gl_mode", 3);			  // 640x480 minimum
    }

    //set the fullscreen flag.
    Cvar_SetValue("vid_fullscreen", (fullscreen == true) ? 1.0f : 0.0f);

    //set misc software/gl settings.
    if (gl_mode(cur_mode))
    {
      use_shadows = (gl_shadows) ? 1.0f : 0.0f;

      //texture quality.
      Cvar_SetValue("gl_picmip", int32(gl_texture_slider.Position()));
//		  Cvar_SetValue( "gl_subdivide_size", ( 256 - int32(gl_tessellation_slider.Position() ) + 64 ) );
    }

    //set the gamma.
    VID_SetGamma(brightness_slider.Position());
}

