
#include	<windows.h>

#include	"client.h"
#include	"cl_sidekick.h"
#include  "cl_inventory.h"

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

int sidekickcommand_selected;


///////////////////////////////////////////////////////////////////////////////
//	functions
///////////////////////////////////////////////////////////////////////////////

void sidekickcommand_Init(void)
{
	//	reset vars
	sidekickcommand_selected = 0;
}


void CL_Sidekick_Update(void)
{
	byte sidekick_whom   = MSG_ReadByte (&net_message);  // retrieve which sidekick
	int  sidekick_status = MSG_ReadShort(&net_message);  // activate or deactivate
	int  val             = MSG_ReadShort(&net_message);  // retrieve percentage value (100%)
	
	//Com_Printf("CL_Sidekick_Update, whom=%d, status=%d, perc=%d\n",sidekick_whom,sidekick_status,val);

	if( sidekick_status != SIDEKICK_STATUS_DEACTIVATE )
	{
		inventory_mode = INVMODE_SIDEKICK;
	}

	// exclusive messages only
	switch (sidekick_status)
	{
    case SIDEKICK_STATUS_ACTIVATE :
		cl.sidekicks_active |= sidekick_whom;
		break;
		
    case SIDEKICK_STATUS_DEACTIVATE :
		cl.sidekicks_active &= ~sidekick_whom;
		break;
		
    case SIDEKICK_STATUS_HEALTH :
		if (sidekick_whom & SIDEKICK_MIKIKO)
			cl.sidekick_mikiko_health = val;
		else
			cl.sidekick_superfly_health = val;
		break;
		
    case SIDEKICK_STATUS_ARMOR :
		if (sidekick_whom & SIDEKICK_MIKIKO)
			cl.sidekick_mikiko_armor = val;
		else
			cl.sidekick_superfly_armor = val;
		break;
		
	case SIDEKICK_STATUS_COMMANDING :
		cl.sidekick_commanding = sidekick_whom;
		break;
	}
	
}





void sidekicks_active_change(alphafx_t * flap, int old_status, int flag)
// adjust sidekick info for activity changes
{
	// check if sidekick has left PVS for first time
	if (old_status & flag)                // previously was in PVS
	{
		if (!(cl.sidekicks_active & flag))  // now gone...
		{
			// set up fader values
			flap->fade_time = 1.0f;           // display active
			flap->direction = -1;             // fading out
		}
	}
	// check if mikiko has entered PVS for first time
	else if (!(old_status & flag))             // previously not in PVS
	{
		if (cl.sidekicks_active & flag)     // now active...
		{
			// set up fader values
			flap->fade_time = 1.0f;           // display active
			flap->direction = 1;              // fading in
		}
	}
}




void sidekicks_active_update(int old_status)
// compare status of previous sidekicks actives versus current active
// sets special fx for interface graphics (alpha faders)
{
	if (old_status != cl.sidekicks_active)             // actives differ?
	{
		sidekicks_active_change(&flap_mikiko,   old_status, SIDEKICK_MIKIKO);
		sidekicks_active_change(&flap_superfly, old_status, SIDEKICK_SUPERFLY); 
	}
	
}