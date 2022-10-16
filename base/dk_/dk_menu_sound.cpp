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
//  Sound Menu
//
///////////////////////////////////////////////////////////////////////////////////

#define SOUND_SFX_CVAR				"s_volume"
#define SOUND_MUSIC_CVAR			"s_music"
#define SOUND_CINE_CVAR				"s_cine"
#define SOUND_CINE_PREVIEW			"DM2_Titans"
#define SOUND_CINE_PREVIEW_CHAN		CHAN_CINE_1

#define SOUND_LEFT											(180)
#define SOUND_SFX_LEFT										(SOUND_LEFT)
#define SOUND_SFX_TOP										(185)
#define SOUND_MUSIC_LEFT									(SOUND_LEFT)
#define SOUND_MUSIC_TOP										(SOUND_SFX_TOP + 60)
															
#define SOUND_CINE_LEFT										(SOUND_LEFT)
#define SOUND_CINE_TOP										(SOUND_MUSIC_TOP + 60)

#define SOUND_USE_3D_LEFT									(SOUND_LEFT)
#define SOUND_USE_3D_TOP									(SOUND_CINE_TOP + 40)


CMenuSubSound::~CMenuSubSound()
{

}

CMenuSubSound::CMenuSubSound()
{
	sfx_volume_old = 0;
}

void CMenuSubSound::Enter()
{
    //initialize the slider bars.
    sfx_volume_slider.Init(SOUND_SFX_LEFT, SOUND_SFX_TOP, button_font, SLIDER_BACKGROUND_CHAR, SLIDER_TAB_CHAR);
    sfx_volume_slider.InitLinear(101, 0, 100);

    music_volume_slider.Init(SOUND_MUSIC_LEFT, SOUND_MUSIC_TOP, button_font, SLIDER_BACKGROUND_CHAR, SLIDER_TAB_CHAR);
    music_volume_slider.InitLinear(101, 0, 100);

    cine_volume_slider.Init(SOUND_CINE_LEFT, SOUND_CINE_TOP, button_font, SLIDER_BACKGROUND_CHAR, SLIDER_TAB_CHAR);
    cine_volume_slider.InitLinear(101, 0, 100);

    //get the sfx volume cvar.
    cvar_t *sfx_volume_cvar   = Cvar_Get(SOUND_SFX_CVAR,   "0.5", CVAR_ARCHIVE);
    cvar_t *music_volume_cvar = Cvar_Get(SOUND_MUSIC_CVAR, "0.5", CVAR_ARCHIVE);
    cvar_t *cine_volume_cvar = Cvar_Get(SOUND_CINE_CVAR, "0.5", CVAR_ARCHIVE);

    //set the position of the sfx volume slider.
    sfx_volume_slider.Position(sfx_volume_cvar->value * 100.0f);
    music_volume_slider.Position(music_volume_cvar->value * 100.0f);
    cine_volume_slider.Position(cine_volume_cvar->value * 100.0f);

//cek[3-14-00]: disable	cvar_t *use3d = Cvar_Get("s_use3d","1",CVAR_ARCHIVE);
//cek[3-14-00]: disable	radio3d.Init (SOUND_USE_3D_LEFT, SOUND_USE_3D_TOP, menu_font, menu_font_bright, tongue_menu[T_MENU_3D_SOUND]);

	cine_volume_preview_time = 0;
}

void CMenuSubSound::Exit()
{
	if (cine_volume_preview_time)
	{
		S_StopMusic(SOUND_CINE_PREVIEW_CHAN);
		cine_volume_preview_time = 0;
	}
}

void CMenuSubSound::Animate(int32 elapsed_time) 
{
	if (cine_volume_preview_time)
	{
		cine_volume_preview_time -= elapsed_time;
		if (cine_volume_preview_time <= 0)
		{
			S_StopMusic(SOUND_CINE_PREVIEW_CHAN);
			cine_volume_preview_time = 0;
		}
	}
}

void CMenuSubSound::DrawBackground() 
{
//cek[3-14-00]: disable    SetRadioState(radio3d, "s_use3d");
//cek[3-14-00]: disable	radio3d.DrawBackground();
    sfx_volume_slider.DrawBackground();
    music_volume_slider.DrawBackground();
    cine_volume_slider.DrawBackground();
}

void CMenuSubSound::DrawForeground() 
{
//cek[3-14-00]: disable	radio3d.DrawForeground();
    //draw the sfx volume slider.
    DKM_DrawString(SOUND_SFX_LEFT, SOUND_SFX_TOP - 18, tongue_menu_options[T_MENU_VOLUME_SOUNDFX], NULL, false, false);

    sfx_volume_slider.DrawForeground();
    //draw value of sfx volume.
    int32 volume = (int32)(sfx_volume_slider.Position() + 0.001f);
    buffer16 volume_string("%d", volume);
    DKM_DrawString(SOUND_SFX_LEFT + SLIDER_WIDTH, SOUND_SFX_TOP + 2, volume_string, NULL, false, false);

    //draw music volume slider.
	DKM_DrawString(SOUND_MUSIC_LEFT, SOUND_MUSIC_TOP - 18, tongue_menu_options[T_MENU_VOLUME_MUSIC], NULL, false, false);
    music_volume_slider.DrawForeground();
    volume = (int32)(music_volume_slider.Position() + 0.001f);
    volume_string.Set("%d", volume);
    DKM_DrawString(SOUND_MUSIC_LEFT + SLIDER_WIDTH, SOUND_MUSIC_TOP + 2, volume_string, NULL, false, false);

    //draw cinematic volume slider.
	DKM_DrawString(SOUND_CINE_LEFT, SOUND_CINE_TOP - 18, tongue_menu_options[T_MENU_VOLUME_CINE], NULL, false, false);
    cine_volume_slider.DrawForeground();
    volume = (int32)(cine_volume_slider.Position() + 0.001f);
    volume_string.Set("%d", volume);
    DKM_DrawString(SOUND_CINE_LEFT + SLIDER_WIDTH, SOUND_CINE_TOP + 2, volume_string, NULL, false, false);

}

void CMenuSubSound::PlaceEntities() 
{
//cek[3-14-00]: disable	radio3d.PlaceEntities();
}

bool CMenuSubSound::Keydown(int32 key) 
{
    switch (key) {
        case K_ESCAPE:
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

bool CMenuSubSound::MousePos(int32 norm_x, int32 norm_y)
{
    bool ret = false;

//cek[3-14-00]: disable	if (radio3d.MousePos(norm_x,norm_y)) return true;

    if (sfx_volume_slider.MousePos(norm_x, norm_y) == true)
    {
        //set the fx volume
        SetSFXVolumeFromSlider(norm_x);
        ret = true;   
    }
    
    if (music_volume_slider.MousePos(norm_x, norm_y) == true) 
    {
      //set the music volume
      SetMusicVolumeFromSlider();
      ret = true;
    }

    if (cine_volume_slider.MousePos(norm_x, norm_y) == true) 
    {
      //set the music volume
      SetCineVolumeFromSlider();
      ret = true;
    }

    return ret;
}

bool CMenuSubSound::MouseDown(int32 norm_x, int32 norm_y) 
{
//cek[3-14-00]: disable	if (radio3d.MouseDown(norm_x,norm_y)) return true;
    if (sfx_volume_slider.MouseDown(norm_x, norm_y) == true) return true;
    if (music_volume_slider.MouseDown(norm_x, norm_y) == true) return true;
    if (cine_volume_slider.MouseDown(norm_x, norm_y) == true) return true;

    return false;
}


bool CMenuSubSound::MouseUp(int32 norm_x, int32 norm_y)
{
//cek[3-14-00]: disable
/*	if (radio3d.MouseUp(norm_x,norm_y))
	{
        ToggleRadioState("s_use3d");
		return true;
	}
*/
    if (sfx_volume_slider.MouseUp(norm_x, norm_y) == true)
    {
        //set the current setting.
        SetSFXVolumeFromSlider(norm_x);
        return true;
    }
    if (music_volume_slider.MouseUp(norm_x, norm_y) == true)
    {
        //set the current setting.
        SetMusicVolumeFromSlider();
        return true;
    }
    if (cine_volume_slider.MouseUp(norm_x, norm_y) == true)
    {
        //set the current setting.
        SetCineVolumeFromSlider();
        return true;
    }

    return false;
}


void CMenuSubSound::SetSFXVolumeFromSlider(int32 norm_x)
{
	//get the current setting.
	int32 volume = (int32)(sfx_volume_slider.Position() + 0.001f);

	//normalize the volume from 0.0 to 1.0
	float volume_normalized = (float)volume / 100.0f;

	if (sfx_volume_old != volume)  // new setting?
	{
		//set the s_volume cvar.
		Cvar_SetValue(SOUND_SFX_CVAR, volume_normalized);

		// play sound for volume feedback
		PlayInterfaceSound(DKM_sounds[DKMS_BUTTON3]);

		sfx_volume_old = volume; // store new setting
	}

}

void CMenuSubSound::SetMusicVolumeFromSlider()
{
	//get the current setting.
	int32 volume = (int32)(music_volume_slider.Position() + 0.001f);

	//normalize the volume from 0.0 to 1.0
	float volume_normalized = (float)volume / 100.0f;

	//set the s_volume cvar.
	Cvar_SetValue(SOUND_MUSIC_CVAR, volume_normalized);

	// force music volume update
	cvar_t *music_volume_cvar = Cvar_Get(SOUND_MUSIC_CVAR, "0.5", CVAR_ARCHIVE); 

	music_volume_cvar->modified = true;
}

void CMenuSubSound::SetCineVolumeFromSlider()
{
	//get the current setting.
	int32 volume = (int32)(cine_volume_slider.Position() + 0.001f);

	//normalize the volume from 0.0 to 1.0
	float volume_normalized = (float)volume / 100.0f;

	//set the s_volume cvar.
	Cvar_SetValue(SOUND_CINE_CVAR, volume_normalized);

	// force music volume update
	cvar_t *cine_volume_cvar = Cvar_Get(SOUND_MUSIC_CVAR, "0.5", CVAR_ARCHIVE); 

	cine_volume_cvar->modified = true;

	if (!cine_volume_preview_time)
	{
		S_StartMusic(SOUND_CINE_PREVIEW,SOUND_CINE_PREVIEW_CHAN);
	}
	cine_volume_preview_time = 5000;
}





