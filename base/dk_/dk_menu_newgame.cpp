#include "dk_shared.h"

#include "dk_misc.h"
#include "dk_buffer.h"
#include "dk_array.h"

#include "dk_menup.h"
#include "dk_menu_controls.h"

#include <io.h>

#include "keys.h"
#include "client.h"

#include "l__language.h"

//#define EPISODE_DEMO_TEST  // show episodes else show difficulty selections

///////////////////////////////////////////////////////////////////////////////////
//
//  New Game Menu
//
///////////////////////////////////////////////////////////////////////////////////



#ifdef EPISODE_DEMO_TEST

//#define NEWGAME_BUTTON_X (200)	// SCG[9/29/99]: changed for demo
#define NEWGAME_BUTTON_X (120)
#define NEWGAME_BUTTON_SPACING (35)
//#define NEWGAME_LEVEL_NAME_X (280)	// SCG[9/29/99]: changed for demo
#define NEWGAME_LEVEL_NAME_X (200)
#define newgame_button_y(i) (125 + (i) * NEWGAME_BUTTON_SPACING)
#define NUM_NEWGAME_BUTTONS 8

#define SKILL_RADIO_LEFT			( 400 )
#define SKILL_RADIO_EASY_TOP		( 125 )
#define SKILL_RADIO_MEDIUM_TOP		( SKILL_RADIO_EASY_TOP + 20 )
#define SKILL_RADIO_HARD_TOP		( SKILL_RADIO_MEDIUM_TOP + 20 )


typedef struct {
    char *episode_name;
    char *display_name;
    char *map_name;
} newgame_start;


newgame_start newgames[] =
{
  { "E1M1", "Marsh",            "map e1m1a\n" },
  { "E1M3", "Solitary",         "map e1m3a\n" },
  { "E2M2", "Catacomb",         "map e2m2a\n" },
  { "E2M5", "Lair of Medusa",   "map e2m5a\n" },
  { "E3M1", "Plague Village",   "map e3m1a\n" },
  { "E3M4", "Wyndrax Tower",    "map e3m4a\n" },
  { "E4M1", "The Rock",         "map e4m1a\n" },
  { "E4M2", "Beneath The Rock", "map e4m2a\n" }
};



//static int32 NUM_NEWGAMES = sizeof(newgames)/sizeof(newgame_start);

#else 


#define NUM_NEWGAME_BUTTONS 3

#define NEWGAME_HEADER_X 220
#define NEWGAME_HEADER_Y 120

#define newgame_button_x(i)    (100 + (i) * NEWGAME_BUTTON_SPACING)
#define NEWGAME_BUTTON_Y       (340)
#define NEWGAME_BUTTON_SPACING (130)

#define skill_pic_x(i) (newgame_button_x(i) - 12)
#define SKILL_PIC_Y    (NEWGAME_BUTTON_Y - 260)


// pictures of Hiro icon for each skill level
char * skill_pics[NUM_NEWGAME_BUTTONS] =
   {
     "pics/menu/skill_0.tga",   // easy
     "pics/menu/skill_1.tga",   // medium
     "pics/menu/skill_2.tga"    // hard
   };

#endif





CMenuSubNewgame::CMenuSubNewgame() {

}

void CMenuSubNewgame::Enter()
{

	//position the buttons.
    for (int32 i = 0; i < NUM_NEWGAME_BUTTONS; i++)
    {
        
        
        
#ifdef EPISODE_DEMO_TEST 
        start[i].Init(NEWGAME_BUTTON_X, newgame_button_y(i), menu_font, menu_font_bright, button_font);
				start[i].InitGraphics(BUTTONSMALL_UP_STRING, BUTTONSMALL_DOWN_STRING, BUTTONSMALL_DISABLE_STRING);
        start[i].SetText(newgames[i].episode_name, true);
#else 
        start[i].Init(newgame_button_x(i), NEWGAME_BUTTON_Y, menu_font, menu_font_bright, button_font);
        start[i].InitGraphics(BUTTONMEDIUM_UP_STRING, BUTTONMEDIUM_DOWN_STRING, BUTTONMEDIUM_DISABLE_STRING);
// SCG[12/2/99]:         start[i].SetText(tongue_menu[T_MENU_EASY + i], true);
        start[i].SetText(tongue_menu_options[T_MENU_NEWGAME_SKILL_EASY + i], true);
#endif
    }



#ifdef EPISODE_DEMO_TEST 

	skill_easy.Init( SKILL_RADIO_LEFT, SKILL_RADIO_EASY_TOP, menu_font, menu_font_bright, tongue_menu_options[T_MENU_NEWGAME_SKILL_EASY]);
	skill_medium.Init( SKILL_RADIO_LEFT, SKILL_RADIO_MEDIUM_TOP, menu_font, menu_font_bright, tongue_menu_options[	T_MENU_NEWGAME_SKILL_MEDIUM]);
	skill_hard.Init( SKILL_RADIO_LEFT, SKILL_RADIO_HARD_TOP, menu_font, menu_font_bright, tongue_menu_options[T_MENU_NEWGAME_SKILL_HARD]);

	skill_easy.Check( false );
	skill_medium.Check( false );
	skill_hard.Check( false );

	int value = ( int ) Cvar_VariableValue( "skill" );
	switch( value )
	{
	  case 0  : skill_easy.Check  ( true );	break;
	  case 1  : skill_medium.Check( true ); break;
	  case 2  : skill_hard.Check  ( true ); break;
	  default : skill_easy.Check  ( true ); break;
	}

#endif EPISODE_DEMO_TEST



}

void CMenuSubNewgame::Animate(int32 elapsed_time)
{
    for (int32 i = 0; i < NUM_NEWGAME_BUTTONS; i++)
    {
        start[i].Animate(elapsed_time);
    }
}


void CMenuSubNewgame::DrawForeground()
{
#ifndef JPN
#ifdef  DM_DEMO
    return;
#endif
#endif

    // draw header 
#ifndef EPISODE_DEMO_TEST
    DKM_DrawString(NEWGAME_HEADER_X, NEWGAME_HEADER_Y, tongue_difficulty[T_MENU_DIFFICULTY_HEADER], NULL, false, true);
#endif

    for (int32 i = 0; i < NUM_NEWGAME_BUTTONS; i++)
    {
      DrawSkillPic(skill_pics[i],skill_pic_x(i), SKILL_PIC_Y);  // draw picture of Hiro representing skill levels

      start[i].DrawForeground();

#ifdef EPISODE_DEMO_TEST
        // draw episode name
        DKM_DrawString(NEWGAME_LEVEL_NAME_X, newgame_button_y(i), newgames[i].display_name, NULL, false, false);
#endif EPISODE_DEMO_TEST

    }


#ifdef EPISODE_DEMO_TEST

	skill_easy.DrawForeground();
	skill_medium.DrawForeground();
	skill_hard.DrawForeground();

#endif EPISODE_DEMO_TEST


}


void CMenuSubNewgame::DrawBackground()
{
#ifndef JPN
#ifdef  DM_DEMO
    return;
#endif
#endif
    for (int32 i = 0; i < NUM_NEWGAME_BUTTONS; i++) 
    {
        start[i].DrawBackground();
    }


#ifdef EPISODE_DEMO_TEST

	skill_easy.DrawBackground();
	skill_medium.DrawBackground();
	skill_hard.DrawBackground();

#endif EPISODE_DEMO_TEST


}


void CMenuSubNewgame::PlaceEntities()
{
}


bool CMenuSubNewgame::Keydown(int32 key)
{
    switch (key)
    {
        case '`' :
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


bool CMenuSubNewgame::MousePos(int32 norm_x, int32 norm_y)
{
    //check each button.
    for (int32 i = 0; i < NUM_NEWGAME_BUTTONS; i++) 
    {
        if (start[i].MousePos(norm_x, norm_y) == true) return true;
    }   

#ifdef EPISODE_DEMO_TEST

    if (skill_easy.MousePos(norm_x, norm_y)  == true)  return true;
    if (skill_medium.MousePos(norm_x, norm_y)  == true)  return true;
    if (skill_hard.MousePos(norm_x, norm_y)  == true)  return true;

#endif EPISODE_DEMO_TEST

	
	return false;
}


bool CMenuSubNewgame::MouseDown(int32 norm_x, int32 norm_y)
{

//    dsn 2.1.  what in tarnation?
//
//    if (_access("./data/invulnerable.dat", 0) != -1) {
//        return false;
//    }

    for (int32 i = 0; i < NUM_NEWGAME_BUTTONS; i++)
    {
        if (start[i].MouseDown(norm_x, norm_y) == true) 
          return true;
    }


#ifdef EPISODE_DEMO_TEST

    if (skill_easy.MouseDown(norm_x, norm_y)  == true)  return true;
    if (skill_medium.MouseDown(norm_x, norm_y)  == true)  return true;
    if (skill_hard.MouseDown(norm_x, norm_y)  == true)  return true;

#endif EPISODE_DEMO_TEST


    return false;
}


bool CMenuSubNewgame::MouseUp(int32 norm_x, int32 norm_y)
//they clicked and released
{
    for (int32 i = 0; i < NUM_NEWGAME_BUTTONS; i++)
    {
        if (start[i].MouseUp(norm_x, norm_y) == true)
        {
            // set difficulty skill level
            Cvar_SetValue("skill", ( float ) i );
			Cvar_ForceSet( "deathmatch", "0" );
			Cvar_ForceSet( "deathtag", "0" );
			Cvar_ForceSet( "ctf", "0" );
			Cvar_ForceSet( "coop", "0" );

#ifdef DAIKATANA_DEMO
			DKM_ShutdownString("map e1m1a\n");
#else
	// NSS[10/2/00]:For OEM version of Daikatana
	#ifdef DAIKATANA_OEM
			DKM_ShutdownString("map e1m2a\n");
	#else
			if (Cvar_VariableValue("play_intro") == 0)
				DKM_ShutdownString("map e1m1a\n");
			else
				DKM_ShutdownString("map intro\n");
	#endif
#endif
            DKM_Shutdown();

            return (true);
        }
    }


#ifdef EPISODE_DEMO_TEST

    if (skill_easy.MouseUp(norm_x, norm_y)  == true)
    {
		  skill_easy.Check( true );
		  skill_medium.Check( false );
		  skill_hard.Check( false );
      Cvar_SetValue("skill", ( float ) 0 );

      return true;
    }

    if (skill_medium.MouseUp(norm_x, norm_y)  == true)
    {
		  skill_easy.Check( false );
		  skill_medium.Check( true );
		  skill_hard.Check( false );
      Cvar_SetValue("skill", ( float ) 1 );

      return true;
    }

    if (skill_hard.MouseUp(norm_x, norm_y)  == true)
    {
		  skill_easy.Check( false );
		  skill_medium.Check( false );
		  skill_hard.Check( true );
      Cvar_SetValue("skill", ( float ) 2 );

      return true;
    }

#endif EPISODE_DEMO_TEST


    return false;
}


void CMenuSubNewgame::DrawSkillPic(char * filename, int x, int y)
{

  float	norm_to_screen_x = viddef.width  / 640.0f;
  float	norm_to_screen_y = viddef.height / 480.0f;

  DRAWSTRUCT drawStruct;

  drawStruct.pImage  = re.RegisterPic( filename, NULL, NULL, RESOURCE_INTERFACE );
  drawStruct.nFlags  = DSFLAG_PALETTE | DSFLAG_ALPHA | DSFLAG_SCALE | DSFLAG_BLEND;
  drawStruct.nXPos   = x * norm_to_screen_x;
  drawStruct.nYPos   = y * norm_to_screen_y;
  drawStruct.fScaleX = norm_to_screen_x;
  drawStruct.fScaleY = norm_to_screen_y;
  drawStruct.fAlpha  = 0.90f;

  re.DrawPic( drawStruct );
}

