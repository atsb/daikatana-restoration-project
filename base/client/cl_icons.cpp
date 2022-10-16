///////////////////////////////////////////////////////////////////////////////
//	cl_icons.cpp
//
//	onscreen icon setup
//
///////////////////////////////////////////////////////////////////////////////

//#include	<windows.h>

#include	"client.h"
#include	"cl_icons.h"
#include  "p_user.h"
#include  "l__language.h"

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define BOOST_NAME_LEFT      (0)
#define ICON_BLINK_DELAY     (500) // half second
#define ICON_HALF_CENTER     (36)	 // x offset

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

extern void *statbar_font; // font used for display

onscreen_icons_info_t onscreen_icons_info;  


//
// functions
//

void CL_Boost_Icons_Init(void)
// initialize global structure
{
	onscreen_icons_info.num_icons = 0;

	for (int i = 0; i < MAX_ONSCREEN_ICONS; i++)
		memset(&onscreen_icons_info.icons[i],0x00,sizeof(onscreen_icons_t));
}




void CL_Boost_Icons_Think(void)
// calculate/check display time
{
	int i;
	onscreen_icons_t icon;
	DRAWSTRUCT drawStruct;
	int sx, sy/*, len*/;
	char buffer[MAX_QPATH];

	if (onscreen_icons_info.num_icons == 0)       // look before you leap
		return;

	if (statbar_font == NULL)
		statbar_font = re.LoadFont("statbar_font"); // load/check font

	sx = BOOST_NAME_LEFT;    // x at left
	sy = viddef.height - 260; // y at top of screen

	for (i = 0;i < MAX_ONSCREEN_ICONS;i++)
	{
		icon = onscreen_icons_info.icons[i]; 

		if (icon.models[0] != NULL)   // model exists?
		{
			if (icon.blink)             // model is in blink mode?
			{
				if (icon.blink_time == 0) // start blink not set yet
				{
					onscreen_icons_info.icons[i].blink_time = cl.time + ICON_BLINK_DELAY;
					onscreen_icons_info.icons[i].alpha = 0.0; // start off inviso
				}
				else
				{
					if (cl.time > icon.blink_time)  // time to flip blink?
					{
						if (icon.alpha == 0.0f)                       // is it inviso?
						{
							onscreen_icons_info.icons[i].alpha = 1.0f;  // make it visible

							//S_StartSoundQuake2(CVector(0,0,0), 0, CHAN_AUTO, S_RegisterSound("global/e_teleportstart.wav"), 1, ATTN_NORM, 0);
							S_StartLocalSound("global/e_teleportstart.wav");
						}
						else
							onscreen_icons_info.icons[i].alpha = 0.0f;  // make it inviso

						// set new blink time
						onscreen_icons_info.icons[i].blink_time = cl.time + ICON_BLINK_DELAY; 
					}
				}
			}  

			// no skills interface visible?
			if (scr_viewsize->value >= SCR_VIEWSIZE_NOSKILLS) 
			{
				// draw text so players know what skill icons are which
// SCG[1/16/00]: 				sprintf(buffer,"%s",tongue_statbar[T_SKILLS_POWER + i]);
				Com_sprintf(buffer,sizeof( buffer), "%s",tongue_statbar[T_SKILLS_POWER + i]);

				// determine pixel width to center the string
				sx = ICON_HALF_CENTER - (re.StringWidth(buffer, statbar_font, -1) >> 1);

//				len = strlen(buffer);

				drawStruct.nFlags = DSFLAG_BLEND;
				drawStruct.szString = buffer;
				drawStruct.nXPos = sx;
				drawStruct.nYPos = sy;
				drawStruct.pImage = statbar_font;
				re.DrawString( drawStruct );
			}
		}

		sy += 36;  // increase y spacing
	}
}


void CL_Boost_Icons(void)
// retrieve net message from server to add to client icon list
{
	byte icon_action, icon_type;
	int num_icons, display_time;
	int model_index1, model_index2; 

	icon_action  = MSG_ReadByte(&net_message);  // retrieve action 
	icon_type    = MSG_ReadByte(&net_message);  // retrieve icon type
	display_time = MSG_ReadShort(&net_message); // retrieve display time

	num_icons = onscreen_icons_info.num_icons;

	// determine desired action 

	if (icon_action == BOOSTICON_ADD) // adding icon?
	{
		switch (icon_type)
		{
		case ICON_POWERBOOST  :
			model_index2 = CL_ModelIndex("models/global/a_pwrb.dkm"); // match artifacts.cpp model names
			model_index1 = CL_ModelIndex("models/global/a_pwrc.dkm");
			break;

		case ICON_ATTACKBOOST :
			model_index2 = CL_ModelIndex("models/global/a_atkb.dkm");
			model_index1 = CL_ModelIndex("models/global/a_atkc.dkm");
			break;

		case ICON_SPEEDBOOST  :
			model_index2 = CL_ModelIndex("models/global/a_spdb.dkm");
			model_index1 = CL_ModelIndex("models/global/a_spdc.dkm");
			break;

		case ICON_ACROBOOST   :
			model_index2 = CL_ModelIndex("models/global/a_acrb.dkm");
			model_index1 = CL_ModelIndex("models/global/a_acrc.dkm");
			break;

		case ICON_VITABOOST   :
			model_index2 = CL_ModelIndex("models/global/a_vtlb.dkm");
			model_index1 = CL_ModelIndex("models/global/a_vtlc.dkm");
			break;

		default :
			model_index2 = 0;  // no icon type found
			model_index1 = 0;
			break;
		}

		icon_type--; // to set 0 based
		_ASSERTE(icon_type >= 0 && icon_type < MAX_ONSCREEN_ICONS);     // check for bogus server value

		// set model 1 references, if one exists
		if (model_index1 > 0) 
		{
			onscreen_icons_info.icons[icon_type].models[0] = cl.model_draw[model_index1];
			onscreen_icons_info.icons[icon_type].models[1] = NULL;

			onscreen_icons_info.icons[icon_type].blink      = false;
			onscreen_icons_info.icons[icon_type].blink_time = 0;

			onscreen_icons_info.icons[icon_type].alpha      = 1.0f;

			// set model 2 references, if one exists
			if (model_index2 > 0) 
				onscreen_icons_info.icons[icon_type].models[1] = cl.model_draw[model_index2];
		}
		else
		{
			onscreen_icons_info.icons[num_icons].models[0] = NULL; // no go, dimaggio
			onscreen_icons_info.icons[num_icons].models[1] = NULL;
		}

		onscreen_icons_info.num_icons++;  // increment # of icons
	}
	else if (icon_action == BOOSTICON_DELETE) // delete/remove icon?
	{
		if (num_icons > 0)
			onscreen_icons_info.num_icons--;       // decrement # of current icons

		memset(&onscreen_icons_info.icons[icon_type - 1],0x00,sizeof(onscreen_icons_t));  // reset icon info
	}
	else if (icon_action == BOOSTICON_BLINK) // blink icon?
	{
		if (num_icons > 0)  // icons exists security
		{
			onscreen_icons_info.icons[icon_type - 1].blink = true;    // icon will be blinking
			onscreen_icons_info.icons[icon_type - 1].blink_time = 0;  // reset blink timer
		}
	}

}

