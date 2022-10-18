
#include	<windows.h>

#include	"client.h"
#include	"cl_selector.h"
#include  "cl_sidekick.h"
#include  "cl_inventory.h"

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	functions
///////////////////////////////////////////////////////////////////////////////
extern cvar_t *sv_episode;
qboolean CL_ValidateSelectorPos(int selection)
{
	int episode = atoi( CM_KeyValue (map_epairs, "episode"));//floor(sv_episode->value + 0.5);				// make sure its good!

	unsigned long val = 0;
	switch (selection)
	{
	case 0 : val = cl.frame.playerstate.stats[STAT_POWER];         break; // power
	case 1 : val = cl.frame.playerstate.stats[STAT_ATTACK];        break; // attack
	case 2 : val = cl.frame.playerstate.stats[STAT_SPEED];         break; // speed
	case 3 : val = cl.frame.playerstate.stats[STAT_JUMP];          break; // acro
	case 4 : val = cl.frame.playerstate.stats[STAT_MAX_HEALTH];    break; // vitality
	default: return FALSE;
	}

	if (cl.refdef.rdflags & RDF_DEATHMATCH)
	{
		return (val >= 5) ? FALSE : TRUE;
	}
	else
	{
		int max = episode + 1;
		if (max > 5) max = 5;
		return ( val >= max ) ? FALSE : TRUE;

	}

}

void CL_ParseSelectorCommand(void)
// read incoming svc_selector message from server
{
	int  cmd_direction;
	byte cmd_activate;
	char buf_command[256];
	
	
	cmd_direction = MSG_ReadShort(&net_message); // direction
	cmd_activate  = MSG_ReadByte (&net_message);  // activating/applying command?
	
	//Com_Printf("CL_ParseSelectorCommand(%d,%d)\n",cmd_direction,cmd_activate);
	
	
	// branch, depending on mode
	
	if (xplevelup_mode)        // adjusting skill points on level up?
	{
		if (cmd_activate)        // applying skill point?
		{
			if (CL_ValidateSelectorPos(xplevelup_selection))
			{
				if (scr_viewsize->value >= SCR_VIEWSIZE_NOSKILLS &&    // is screen size disabling skill display
					!override_showhuds)                                // and not showing all huds?
				{
					skill_hud.direction = -1;      // start fade
					skill_hud.fade_time = cl.time; // fade NOW
				}
				
				// send command to server and pass selected skill point
				
				Com_sprintf(buf_command,sizeof(buf_command),"xplevelup %d\n",xplevelup_selection);
				Cbuf_AddText(buf_command);
				
				// reset vars
				xplevelup_mode      = false;
				xplevelup_selection = 0;
				xplevelup_timer     = 0;
				xplevelup_blink     = false;
				
				S_StartLocalSound("global/skill_increase.wav");  // play apply/de-activate sound
			}
			else
			{
				for (int i = 0; i < MAX_SKILLS; i++)
				{
					if (CL_ValidateSelectorPos(i))
					{
						xplevelup_selection = i;
						break;
					}
				}
			}
		}
		else                              // moving selector up/down
		{
			int i = 0;
			do
			{
				xplevelup_selection += cmd_direction;  // apply direction to current selection
				
				if (xplevelup_selection == MAX_SKILLS) // flip 
					xplevelup_selection = 0;
				else if (xplevelup_selection == -1) // boundary check
					xplevelup_selection = (MAX_SKILLS - 1);     // flip
			} while ((i++ < MAX_SKILLS) && (!CL_ValidateSelectorPos(xplevelup_selection)));
				
				S_StartLocalSound("global/skill_selector.wav");  // play up/down sound
		}
		
	}
	else
	{
		// assume sideick command handling
		if (cl.refdef.rdflags & RDF_COOP)
			return;
		
		if (cl.sidekicks_active) // any sidekicks available?
		{
			if (cmd_activate)  // activating sidekick command?
			{
				// activating sidekick command and interface already open?
				if (inventory_mode == INVMODE_SIDEKICK)  
				{
					// need to make sure these commands are equivalent to those found in world.dll
					switch (sidekickcommand_selected)
					{
					case 0 : Com_sprintf(buf_command,sizeof(buf_command),"sidekick_get\n");				break;
					case 1 : Com_sprintf(buf_command,sizeof(buf_command),"sidekick_come\n");			break;
					case 2 : Com_sprintf(buf_command,sizeof(buf_command),"sidekick_stay\n");            break;
					case 3 : Com_sprintf(buf_command,sizeof(buf_command),"sidekick_attack\n");          break;
					case 4 : Com_sprintf(buf_command,sizeof(buf_command),"sidekick_backoff\n");         break;
					default: memset( buf_command, 0, 256 ); break;
					}

					Cbuf_AddText(buf_command); // send command to server
					
					// play activate sound
					S_StartLocalSound("global/skill_increase.wav");
				}
				else // else just open the interface if 'apply' button is hit
				{
					S_StartLocalSound("global/skill_start.wav"); // play initial sound
				}
			}
			else
			{
				// not activating -- adjust sidekick selector
				sidekickcommand_selected += cmd_direction;
				
				if (sidekickcommand_selected == -1)                     // boundary check
					sidekickcommand_selected = NUM_SIDEKICKCOMMANDS - 1;  // flip
				else
					if (sidekickcommand_selected == NUM_SIDEKICKCOMMANDS) // boundary check
						sidekickcommand_selected = 0;                       // flip 
					
					S_StartLocalSound("global/skill_selector.wav"); // play up/down sound
			}
			
			
			// set flag indicating to draw sidekick command 
			inventory_mode = INVMODE_SIDEKICK;                    // set new hud mode
			
			if (scr_viewsize->value >= SCR_VIEWSIZE_NOSKILLS)
			{	
				invmode_hud.off_time = cl.time + 5000;
			}

			invmode_hud.direction = 1;       // alpha direction is towards fade out
			invmode_hud.fade_time = cl.time;  // start fade NOW / mode is reset when alpha reaches 0
			
			// any sidekick in 'command'?
			if (!cl.sidekick_commanding)
			{
				if (cl.sidekicks_active & SIDEKICK_MIKIKO)
					cl.sidekick_commanding = 2; // ttd: need global here!
				else
					if (cl.sidekicks_active & SIDEKICK_SUPERFLY)
						cl.sidekick_commanding = 3; // ttd: need global here!
			}
		}
		
	}
	
}


