#include "dk_shared.h"

#include "dk_misc.h"
#include "dk_buffer.h"
#include "dk_array.h"

#include "dk_menup.h"
#include "dk_menu_controls.h"

#include "keys.h"
#include "client.h"

#include "l__language.h"


// defines
#define SCREEN_WIDTH										370
#define SCREEN_HEIGHT										300
#define SCREEN_LEFT											100
#define SCREEN_TOP											105
#define SCREEN_RIGHT										(SCREEN_LEFT + SCREEN_WIDTH)
#define SCREEN_BOTTOM										(SCREEN_TOP + SCREEN_HEIGHT)

#define BUTTON_HEIGHT										(25)
#define BUTTON_SPACING										(18)

#define CAPTION_TOP											(SCREEN_TOP + 5)

#define COLUMN(idx)											( SCREEN_LEFT + 20 + (185 * (idx)) )
#define ROW(idx)											( SEPARATOR_TOP + 20 + (150 * (idx)) )

#define IS_TOP(ep)											( (ep == 1) || (ep == 2) )
#define IS_LEFT(ep)											( (ep == 1) || (ep == 3) )

#define POS_X(ep)											( IS_LEFT(ep) ? COLUMN(0) : COLUMN(1) )
#define POS_Y(ep, idx)										( (idx * BUTTON_SPACING) + (IS_TOP(ep) ? ROW(0) : ROW(1)) )

// the separator line
#define SEPARATOR_LEFT										( 100 )
#define SEPARATOR_RIGHT										( 470 )
#define SEPARATOR_TOP										( SCREEN_TOP + 17 )

// the back button
#define BACK_LEFT											( SCREEN_LEFT )
#define BACK_TOP											( SCREEN_TOP )
	
#define CAPTION_OFFSET										( 17 )

/*
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

	bool Active();
protected:
	bool					bActive;
	CInterfaceButtonRadio	weapons[24];
	CInterfaceButton		back;
};
*/
#define	FLAG_UNUSED			0xFFFFFFFF

CMenuSubWeapons::CMenuSubWeapons():
bActive(false),
weapCvar(NULL)
{
    //place the separator line.
    separator.Init( SEPARATOR_LEFT, SEPARATOR_TOP, SEPARATOR_RIGHT , 2);
}

void CMenuSubWeapons::Activate(char *cvar)
{
	Enter();
	weapCvar = cvar;
}

#define SLOT(ep, num)	( 1 << (6*(ep - 1) + num) )

void CMenuSubWeapons::Enter()
{
	bActive = true;

	// the back button
	back.Init(BACK_LEFT, BACK_TOP, menu_font, menu_font_bright, button_font);
	back.InitGraphics(BUTTONBACK_UP_STRING, BUTTONBACK_DOWN_STRING, BUTTONBACK_DISABLE_STRING);
	back.SetText("", true);

	// set up the buttons
	int episode = 1;

	// episode 1
	InitWeaponBtn(episode, 0, SLOT(episode,1), tongue_weapons[T_WEAPON_IONBLASTER_2]);
	InitWeaponBtn(episode, 1, SLOT(episode,2), tongue_weapons[T_WEAPON_C4_2]);
	InitWeaponBtn(episode, 2, SLOT(episode,3), tongue_weapons[T_WEAPON_SHOTCYCLER_2]);
	InitWeaponBtn(episode, 3, SLOT(episode,4), tongue_weapons[T_WEAPON_SIDEWINDER_2]);
	InitWeaponBtn(episode, 4, SLOT(episode,5), tongue_weapons[T_WEAPON_SHOCKWAVE_2]);
	InitWeaponBtn(episode, 5, FLAG_UNUSED, "");

	// episode 2
	episode = 2;
	InitWeaponBtn(episode, 0, SLOT(episode,0), tongue_weapons[T_WEAPON_DISCUS_2]);
	InitWeaponBtn(episode, 1, SLOT(episode,1), tongue_weapons[T_WEAPON_VENOMOUS_2]);
	InitWeaponBtn(episode, 2, SLOT(episode,2), tongue_weapons[T_WEAPON_SUNFLARE_2]);
	InitWeaponBtn(episode, 3, SLOT(episode,3), tongue_weapons[T_WEAPON_HAMMER_2]);
	InitWeaponBtn(episode, 4, SLOT(episode,4), tongue_weapons[T_WEAPON_TRIDENT_2]);
	InitWeaponBtn(episode, 5, SLOT(episode,5), tongue_weapons[T_WEAPON_ZEUS_2]);

	// episode 3
	episode = 3;
	InitWeaponBtn(episode, 0, SLOT(episode,1), tongue_weapons[T_WEAPON_BOLTER_2]);
	InitWeaponBtn(episode, 1, SLOT(episode,2), tongue_weapons[T_WEAPON_STAVROS_2]);
	InitWeaponBtn(episode, 2, SLOT(episode,3), tongue_weapons[T_WEAPON_BALLISTA_2]);
	InitWeaponBtn(episode, 3, SLOT(episode,4), tongue_weapons[T_WEAPON_WISP_2]);
	InitWeaponBtn(episode, 4, SLOT(episode,5), tongue_weapons[T_WEAPON_NIGHTMARE_2]);
	InitWeaponBtn(episode, 5, FLAG_UNUSED, "");
	
	// episode 4
	episode = 4;
	InitWeaponBtn(episode, 0, SLOT(episode,0), tongue_weapons[T_WEAPON_GLOCK_2]);
	InitWeaponBtn(episode, 1, SLOT(episode,1), tongue_weapons[T_WEAPON_SLUGGER_2]);
	InitWeaponBtn(episode, 2, SLOT(episode,2), tongue_weapons[T_WEAPON_KINIETICORE_2]);
	InitWeaponBtn(episode, 3, SLOT(episode,3), tongue_weapons[T_WEAPON_RIPGUN_2]);
	InitWeaponBtn(episode, 4, SLOT(episode,4), tongue_weapons[T_WEAPON_NOVABEAM_2]);
	InitWeaponBtn(episode, 5, SLOT(episode,5), tongue_weapons[T_WEAPON_METAMASER_2]);
}

void CMenuSubWeapons::Leave()
{
	bActive = false;
	weapCvar = NULL;
}

void CMenuSubWeapons::Animate(int32 elapsed_time)
{

}

void CMenuSubWeapons::DrawForeground()
{
	if (!bActive || !weapCvar)
	{
		Leave();
		return;
	}

	back.DrawForeground();

	int i;
	// print some captions
	char string[32];
	for (i = 1; i <= 4; i++)
	{
		Com_sprintf(string,sizeof(string),"%s %d:",tongue_menu[T_MENU_EPISODE],i);
		DKM_DrawString( POS_X(i), POS_Y(i,0) - CAPTION_OFFSET, string, NULL, false, true );
	}

	// draw all active buttons 
	weaponStruct_t *weap;
	for (weap = &weapons[0],i = 0; i < 24; i++,weap++)
	{
		if (weap->flag != FLAG_UNUSED)
		{
			SetRadioStateFlag( weap->button, weapCvar, weap->flag ,true);
			weap->button.DrawForeground();
		}
	}
}

void CMenuSubWeapons::DrawBackground()
{
	if (!bActive || !weapCvar)
	{
		Leave();
		return;
	}

	back.DrawBackground();

	// draw all active buttons
	weaponStruct_t *weap;
	int i;
	for (weap = &weapons[0],i = 0; i < 24; i++,weap++)
	{
		if (weap->flag != FLAG_UNUSED)
		{
			weap->button.DrawBackground();
		}
	}
}

void CMenuSubWeapons::PlaceEntities()
{
	back.PlaceEntities();
	separator.PlaceEntities();
}

bool CMenuSubWeapons::Keydown(int32 key)
{
	return false;
}

bool CMenuSubWeapons::MousePos(int32 norm_x, int32 norm_y)
{
	if (!bActive || !weapCvar)
	{
		Leave();
		return true;
	}

	bool result = false;
	if (back.MousePos(norm_x, norm_y))
		result = true;

	weaponStruct_t *weap;
	int i;
	for (weap = &weapons[0],i = 0; i < 24; i++,weap++)
	{
		if (weap->flag != FLAG_UNUSED)
		{
			if (weap->button.MousePos(norm_x, norm_y)) 
			{
				result = true;
			}
		}
	}
	return result;
}

bool CMenuSubWeapons::MouseDown(int32 norm_x, int32 norm_y)
{
	if (!bActive || !weapCvar)
	{
		Leave();
		return true;
	}

	if (back.MouseDown(norm_x, norm_y)) return true;

	bool result = false;
	weaponStruct_t *weap;
	int i;
	for (weap = &weapons[0],i = 0; i < 24; i++,weap++)
	{
		if (weap->flag != FLAG_UNUSED)
		{
			if (weap->button.MouseDown(norm_x, norm_y)) return true;
		}
	}

	return false;
}

bool CMenuSubWeapons::MouseUp(int32 norm_x, int32 norm_y)
{
	if (!bActive || !weapCvar)
	{
		Leave();
		return true;
	}

	if (back.MouseUp(norm_x, norm_y)) 
	{
		Leave();
		return true;
	}

	bool result = false;
	weaponStruct_t *weap;
	int i;
	for (weap = &weapons[0],i = 0; i < 24; i++,weap++)
	{
		if (weap->flag != FLAG_UNUSED)
		{
			if (weap->button.MouseUp(norm_x, norm_y))
			{
				ToggleCvarFlag( weapCvar, weap->flag );
				return true;
			}
		}
	}

	return false;
}

bool CMenuSubWeapons::Active()
{
	return bActive;
}

void CMenuSubWeapons::InitWeaponBtn(int episode, int num, unsigned long flag, const char *str)
{
	int idx = (episode - 1) * 6 + num;

	if (idx < 0 || idx > 24)
		return;

	weaponStruct_t *weapon = &weapons[idx];
	weapon->flag = flag;
	if (flag != FLAG_UNUSED)
	{
		weapon->button.Init(POS_X(episode), POS_Y(episode, num), menu_font, menu_font_bright, str);
	}
}
