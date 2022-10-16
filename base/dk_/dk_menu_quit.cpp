#include "dk_shared.h"

#include "dk_misc.h"
#include "dk_buffer.h"
#include "dk_array.h"

#include "dk_menup.h"
#include "dk_menu.h"
#include "dk_menu_controls.h"

#include "keys.h"

#include "l__language.h"


///////////////////////////////////////////////////////////////////////////////////
//  Quit Menu
///////////////////////////////////////////////////////////////////////////////////

#define QUIT_YES_X 190
#define QUIT_YES_Y 230
#define QUIT_NO_X 290
#define QUIT_NO_Y QUIT_YES_Y

#define QUIT_HEADER_X 200
#define QUIT_HEADER_Y (QUIT_YES_Y - 30)


CMenuSubQuit::CMenuSubQuit()
{
}

void CMenuSubQuit::Enter()
{
    //initialize our buttons.
    yes.Init(QUIT_YES_X, QUIT_YES_Y, menu_font, menu_font_bright, button_font);
    yes.InitGraphics(BUTTONSMALL_UP_STRING, BUTTONSMALL_DOWN_STRING, BUTTONSMALL_DISABLE_STRING);
    yes.SetText(tongue_menu[T_MENU_YES], true);

    no.Init(QUIT_NO_X, QUIT_NO_Y, menu_font, menu_font_bright, button_font);
    no.InitGraphics(BUTTONSMALL_UP_STRING, BUTTONSMALL_DOWN_STRING, BUTTONSMALL_DISABLE_STRING);
    no.SetText(tongue_menu[T_MENU_NO], true);

	message = rand() % 13;
	rare = (frand() < 0.1);
}


void CMenuSubQuit::Animate(int32 elapsed_time)
{
    //animate our buttons.
    yes.Animate(elapsed_time);
    no.Animate(elapsed_time);
}


void CMenuSubQuit::PlaceEntities()
{
    yes.PlaceEntities();
    no.PlaceEntities();
}

char* regular_messages[] =
{
	// added for Japanese version
#ifdef JPN
	"終了しますか？",
	"終了しますか？",
	"終了しますか？",
	"終了しますか？",
	"終了しますか？",
	"終了しますか？",
	"終了しますか？",
	"終了しますか？",
	"終了しますか？",
	"終了しますか？",
	"終了しますか？",
	"終了しますか？",
	"終了しますか？"
#else
	"Ya thought you were a big boy, but you're|really just a baby boy, aren't you?",
	"Are you gonna quit this game just like|you quit everything else in life?",
	"Does the pretty harp music and the scary|crickets make you cry and quit?",
	"Does all this time travel really boggle|you down, confuse you and make you quit?",
	"Did the scary monsters and mean robots|make you run to your mommy?",
	"Just remember: before you actually finish this game,|make sure you find the sandalwood box.  Quit now?",
	"Can't take much more of this hardcore action, eh?  |Gotta run like a little baby and go to bed, right?",
	"If you leave now, I'll start working on|summoning a Shoggoth to come tear you apart|while you sleep.  Is that ok?",
	"Hey, guess what?  When you leave,|I'm going to start smacking all the monsters around|so they're REALLY MAD next time you play, ok?",
	"While you're gone, I'm going to keep|working on the Grand Unified Theory, ok?",
	"Are you broken?",
	"Hey, watch out for the Dark Troopers in the|San Francisco episode, ok?",
	"You're not much of a Kraken, are you?"
#endif
};

char* rare_messages[] = 
{
#ifdef JPN
	"終了しますか？",
	"終了しますか？",
	"終了しますか？",
	"終了しますか？"
#else 
	"While you're away I think I'll link into some internet|sites and surf for a while.  Is that okay with you?",
	"Get out of me?",
	"Suck it down(tm)?",
	"Has the monkey landed yet?"
#endif
};

void SplitString(char *src, buffer128 &left, buffer128 &right, char splitchar)
{
	char *chr = strrchr(src,splitchar);
	if (chr)
	{
		*chr = NULL;
		left = src;
		right = chr+1;
	}
	else
	{
		left = NULL;
		right = src;
	}
}

void CMenuSubQuit::GetQuitMessage(buffer128 &buf1, buffer128 &buf2, buffer128 &buf3)
{
	char selected[256];
	memset(selected,0,256);

	if (rare)
	{
		strcpy(selected,rare_messages[message % 4]);
	}
	else
	{
		strcpy(selected,regular_messages[message]);
	}

	SplitString(selected,buf2,buf3,'|');
	if (strlen(buf2))
	{
		strcpy(selected,buf2);
		SplitString(selected,buf1,buf2,'|');
	}

/*	char *right = strchr(selected,'|');
	if (right)
	{
		*right = NULL;
		buf1 = selected;
		buf2 = right+1;
	}
	else
	{
		buf2 = selected;
	}*/
}

void CMenuSubQuit::DrawForeground()
{
	buffer128 buf1,buf2,buf3;
	// draw header (to clarify what the hell the yes/no buttons are prompting for)
#ifdef TONGUE_ENGLISH
	GetQuitMessage(buf1,buf2,buf3);	
#else
	buf3.Set(tongue_menu[T_MENU_QUIT_CONFIRM]);
#endif

	DKM_DrawString(275, QUIT_HEADER_Y - 30, buf1, NULL, true, true);  // highlight title
	DKM_DrawString(275, QUIT_HEADER_Y - 15, buf2, NULL, true, true);  // highlight title
	DKM_DrawString(275, QUIT_HEADER_Y, buf3, NULL, true, true);  // highlight title

	yes.DrawForeground();
	no.DrawForeground();
}


void CMenuSubQuit::DrawBackground()
{
    yes.DrawBackground();
    no.DrawBackground();
}

void DoQuit(qboolean bQuit);

bool CMenuSubQuit::Keydown(int32 key)
{
    switch (key)
    {
        case 'y' : // confirm quit of game.
			DoQuit(true);
            return true;
            break;

        case 'n' :
        case K_ESCAPE : // deconfirm quit of game.
			DoQuit(false);
            return true;
            break;
    }

    return false;
}


bool CMenuSubQuit::MousePos(int32 norm_x, int32 norm_y)
{
  bool ret = false;

  if (yes.MousePos(norm_x, norm_y) == true) ret = true;
  if (no.MousePos(norm_x, norm_y) == true) ret = true;

  return ret;
}


bool CMenuSubQuit::MouseDown(int32 norm_x, int32 norm_y)
{
  if (yes.MouseDown(norm_x, norm_y) == true) return true;
  if (no.MouseDown(norm_x, norm_y) == true) return true;

  return false;
}


bool CMenuSubQuit::MouseUp(int32 norm_x, int32 norm_y)
{
  if (yes.MouseUp(norm_x, norm_y) == true)
  {
	DoQuit(true);
	return true;
  }

  if (no.MouseUp(norm_x, norm_y) == true)
  {
	  DoQuit(false);
      return true;
  }

  return false;
}


CMenuSubResume::CMenuSubResume()
{

}


void CMenuSubResume::Enter()
{
  DKM_Shutdown();
}

