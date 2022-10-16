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

///////////////////////////////////////////////////////////////////////////////////
//  Demos Menu
///////////////////////////////////////////////////////////////////////////////////


CMenuSubDemos::CMenuSubDemos()
{

}


void CMenuSubDemos::Enter()
{

}


void CMenuSubDemos::Animate(int32 elapsed_time)
{

}

void CMenuSubDemos::DrawBackground()
{

}



void CMenuSubDemos::DrawForeground()
{

}


void CMenuSubDemos::PlaceEntities()
{

}


bool CMenuSubDemos::Keydown(int32 key)
{
	switch (key)
	{
    case '`' :
    case K_ESCAPE :
		if( Cvar_VariableValue( "console" ) == 0 )
		{
			return false;
		}
		DKM_Shutdown(); // exit entire interface
		
		return true;
	}
	
	return false;
}


bool CMenuSubDemos::MousePos(int32 norm_x, int32 norm_y)
{
  bool ret = false;

  return ret;
}



bool CMenuSubDemos::MouseDown(int32 norm_x, int32 norm_y)
{
  return false;
}


bool CMenuSubDemos::MouseUp(int32 norm_x, int32 norm_y)
{
  return false;
}

typedef struct credit_name_s
{
	char *name;
	bool highlight;
} credit_name_t;

#define END_NAME "-"

static credit_name_t credit_names[] =
{
	{"The Daikatana Team is...",true},
	{"" , false},
	{"Game Designer/Project Lead" , true},
	{"John Romero" , false},
	{"" , false},
	{"Producer" , true},
	{"Kelly \"Sleestak\" Hoerner" , false},
	{"" , false},
	{"Lead Programmer" , true},
	{"Shawn Green" , false},
	{"" , false},
	{"Programmers" , true},
	{"Charles \"Chalrez\" Kostick" , false},
	{"Noel \"AI_NINJA\" Stephens" , false},
	{"" , false},
	{"Level Designers" , true},
	{"Stevie \"KillCreek\" Case" , false},
	{"Christian \"Guf\" Cummings" , false},
	{"Larry \"Rocketman\" Herring" , false},
	{"Luke \"weasl\" Whiteside" , false},
	{"" , false},
	{"Lead Artist" , true},
	{"Eric Smith" , false},
	{"" , false},
	{"Artists" , true},
	{"Matthew Cox" , false},
	{"Gene \"Sgt Sacc Atak\" Kohler Jr." , false},
	{"Trent Martinez" , false},
	{"Chris \"Pixel Beast\" Perna" , false},
	{"Rayfield \"Pieslice\" Wright" , false},
	{"" , false},
	{"Sound Engineer" , true},
	{"Stan Neuvo" , false},
	{"" , false},
	{"Music" , true},
	{"Will Nevins" , false},
	{"" , false},
	{"Director of Development" , true},
	{"John \"Mr.\" Kavanagh" , false},
	{"",false},
	{"Eidos Producer", true},
	{"James Poole", false},
	{"" , false},
	{"Installation Engineer" , true},
	{"Lee Briggs" , false},
	{"" , false},
	{"Localization Manager" , true},
	{"Jeff Groteboer" , false},
	{"" , false},
	{"Director of QA" , true},
	{"Chad \"Ogre\" Barron" , false},
	{"" , false},
	{"QA Manager / Lead Tester" , true},
	{"Alex \"Nazgûl\" Quintana" , false},
	{"" , false},
	{"Eidos Lead Tester", true},
	{"Clayton Palma" , false},
	{"" , false},
	{"QA Team" , true},
	{"Billy R. \"Amadeus\" Browning" , false},
	{"Joe Forsythe" , false},
	{"Jay Anthony \"BuzzzKil\" Franke" , false},
	{"Dan Franklin" , false},
	{"Erik Kennedy" , false},
	{"Ron Lauron" , false},
	{"Chris Lovenguth", false},
	{"Colby McCracken" , false},
	{"Clint Nembhard" , false},
	{"Rob Swain" , false},
	{"Doug \"Thanatos\" Swearingen" , false},
	{"Beau Teora" , false},
	{"Kjell Vistad" , false},
	{END_NAME,false}
};

#define CREDITS_TOP						(132.0)
#define CREDITS_LIST_START				(CREDITS_TOP)
#define CREDITS_SPEED					(0.016)
#define CREDITS_SPACING					(16)
#define CREDITS_POS(x,time)				((x * CREDITS_SPACING) + CREDITS_LIST_BOTTOM - CREDITS_SPEED * (float)time)
#define CREDITS_UNDERLINE_POS(x,time)	(CREDITS_POS(x,time) + CREDITS_SPACING - 2)
#define CREDITS_CENTER_X				(270)
#define CREDITS_LIST_COUNT				(17)
#define CREDITS_LIST_BOTTOM				((CREDITS_LIST_COUNT * CREDITS_SPACING) + CREDITS_LIST_START)

#define CREDITS_FADE_LINES				(5)
#define CREDITS_FADE_TOP				(CREDITS_LIST_START + CREDITS_SPACING * CREDITS_FADE_LINES)
#define CREDITS_FADE_BOTTOM				(CREDITS_LIST_BOTTOM - CREDITS_SPACING * CREDITS_FADE_LINES)
#define CREDITS_ALPHA_NORM				(0.7)
#define CREDITS_ALPHA_TOP_SLOPE			(CREDITS_ALPHA_NORM / (CREDITS_FADE_TOP - CREDITS_LIST_START))
#define CREDITS_ALPHA_BOT_SLOPE			(-CREDITS_ALPHA_NORM / (CREDITS_LIST_BOTTOM - CREDITS_FADE_BOTTOM))


#define CREDITS_PLAY_TOP				(CREDITS_LIST_BOTTOM + 20)
#define CREDITS_PLAY_LEFT				(230)

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
#define CONFIRM_WIDTH										(350)
#define CONFIRM_HEIGHT										(110)
#define CONFIRM_BUTTON_TOP									(CONFIRM_BOTTOM - (BUTTON_HEIGHT + 10))
#define CONFIRM_BUTTON_LEFT(idx)							( BUTTON_L(CONFIRM_LEFT,(CONFIRM_RIGHT-CONFIRM_LEFT),idx,2,BUTTON_SHORT_WIDTH) )

#define CONFIG_LEFT											110
#define CONFIG_TOP											115
#define CONFIG_WIDTH										375
#define CONFIG_HEIGHT										300
#define CONFIRM_LEFT										(CENTERED(CONFIG_LEFT - 5,CONFIG_WIDTH,CONFIRM_WIDTH))
#define CONFIRM_RIGHT										(CONFIRM_LEFT + CONFIRM_WIDTH)
#define CONFIRM_TOP											(CENTERED(CONFIG_TOP,CONFIG_HEIGHT,CONFIRM_HEIGHT))
#define CONFIRM_BOTTOM										(CONFIRM_TOP + CONFIRM_HEIGHT)

CMenuSubCredits::CMenuSubCredits()
{

}

void CMenuSubCredits::Enter()
{
	confirm_dlg.Init(CONFIRM_LEFT,CONFIRM_TOP,CONFIRM_RIGHT,CONFIRM_BOTTOM,2,CONFIRM_BUTTON_TOP,CONFIRM_BUTTON_LEFT(1),CONFIRM_BUTTON_LEFT(2));
	planetDK_button.Init( 170, 110 ,menu_font, menu_font_bright, button_font );
	planetDK_button.InitGraphics( BUTTONLONG_UP_STRING, BUTTONLONG_DOWN_STRING, BUTTONLONG_DISABLE_STRING );
	planetDK_button.SetText( tongue_menu[T_MENU_PLANET_DK], true );
	msTime = 0;
}

void CMenuSubCredits::Animate(int32 elapsed_time)
{
	if (confirm_dlg.Showing())
	{
		confirm_dlg.Animate(elapsed_time);
		return;
	}
	planetDK_button.Animate(elapsed_time);
	msTime += elapsed_time;
}

void CMenuSubCredits::DrawForeground()
{
	if (confirm_dlg.Showing())
	{
		confirm_dlg.DrawForeground();
		return;
	}

	int top = 0;
	int drawn = 0;
	int i = 0;
	while (stricmp(credit_names[i].name,END_NAME))
	{
		int idx = i++;
		top = CREDITS_POS(idx,msTime);
		if ((top <= CREDITS_LIST_START) || (top > CREDITS_LIST_BOTTOM))
			continue;

		float alpha;
		if (top <= CREDITS_FADE_TOP)
		{
			alpha = (top - CREDITS_LIST_START) * CREDITS_ALPHA_TOP_SLOPE;
		}
		else if (top >= CREDITS_FADE_BOTTOM)
		{
			alpha = CREDITS_ALPHA_NORM + (top - CREDITS_FADE_BOTTOM) * CREDITS_ALPHA_BOT_SLOPE;
		}
		else
		{
			float mid = (CREDITS_LIST_BOTTOM + CREDITS_LIST_START) * 0.5;
			float dist = fabs(top - mid);
			if (dist < CREDITS_SPACING * 0.5)
				alpha = 1.0;
			else
				alpha = CREDITS_ALPHA_NORM;
		}
						
		drawn++;
		DKM_DrawString(CREDITS_CENTER_X,top,credit_names[idx].name, NULL, true,credit_names[idx].highlight, alpha);
	}

	// fire up the cinecredits!
	if (!drawn)
	{
#ifndef DAIKATANA_DEMO
		if (!in_main_game())
		{
			msTime = 0;
			DKM_ShutdownString("deathmatch 0\ncoop 0\nkillserver\nloading\nwait\nwait\nwait\nwait\nmap credits\n");
			DKM_Shutdown();
		}
		else
#endif
			msTime = 0;
	}
	planetDK_button.DrawForeground();
}

void CMenuSubCredits::DrawBackground()
{
	if (confirm_dlg.Showing())
	{
		confirm_dlg.DrawBackground();
		return;
	}

	planetDK_button.DrawBackground();
}

void CMenuSubCredits::PlaceEntities()
{
	if (confirm_dlg.Showing())
	{
		confirm_dlg.PlaceEntities();
		return;
	}

	planetDK_button.PlaceEntities();
}

bool CMenuSubCredits::Keydown(int32 key)
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
	
	if (planetDK_button.Keydown(key))
	{

		return true;
	}
	return false;
}

bool CMenuSubCredits::MousePos(int32 norm_x, int32 norm_y)
{
	if (confirm_dlg.Showing())
	{
		if (confirm_dlg.MousePos(norm_x,norm_y)) return true;
		return false;
	}

	if (planetDK_button.MousePos(norm_x, norm_y))
	{
		return true;
	}
	return false;
}

bool CMenuSubCredits::MouseDown(int32 norm_x, int32 norm_y)
{
	if (confirm_dlg.Showing())
	{
		if (confirm_dlg.MouseDown(norm_x,norm_y)) return true;
		return false;
	}

	if (planetDK_button.MouseDown(norm_x, norm_y))
	{
		return true;
	}
	return false;
}	

bool CMenuSubCredits::MouseUp(int32 norm_x, int32 norm_y)
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

	if (planetDK_button.MouseUp(norm_x, norm_y))
	{
		confirm_dlg.Show(tongue_menu[T_MENU_PLANET_DK_MSG],tongue_menu[T_MENU_CONTINUE]);			
		return true;
	}
	return false;
}

int LaunchBrowser(char *url);
bool CMenuSubCredits::ConfirmFinish()
{
	if (confirm_dlg.Showing())
	{
		bool yes = false;
		if (confirm_dlg.Result(yes))
		{
			if (yes)
			{
				Sys_Minimize();
				LaunchBrowser("www.planetdaikatana.com");
			}
			return true;
		}
	}

	return false;
}

