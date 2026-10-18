///////////////////////////////////////////////////////////////////////////////
// 	p_sidekick.cpp
//	  -- sidekick command functions
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER
#include	<crtdbg.h>
#endif
#include	"p_global.h"
#include	"p_user.h"

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define NUM_SIDEKICKCOMMANDS          (7)

///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

int sidekickcommand_selected = 0;

///////////////////////////////////////////////////////////////////////////////
//	prototypes
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	functions
///////////////////////////////////////////////////////////////////////////////
void P_SidekickCommandPrev(userEntity_t *self)
// scroll up/back in command list
{
	if (--sidekickcommand_selected == -1)
    sidekickcommand_selected = NUM_SIDEKICKCOMMANDS - 1;

  //	send the sidekickcommand Prev message 
	gi.WriteByte(SVC_SIDEKICK_COMMAND);
  gi.WriteByte(sidekickcommand_selected);  // selection index
	gi.WriteByte(0);				                 // activate/apply
	gi.unicast (self, true);

  self->client->inventoryTime = p_realtime + INVMODE_DISPLAYTIME; // # seconds for display
}


void P_SidekickCommandNext(userEntity_t *self)
// scroll down/forward in command list
{
	if (++sidekickcommand_selected == NUM_SIDEKICKCOMMANDS)
    sidekickcommand_selected = 0;

  //	send the sidekickcommand Next message 
	gi.WriteByte(SVC_SIDEKICK_COMMAND);
  gi.WriteByte(sidekickcommand_selected);	 // selection index
	gi.WriteByte(0);				                 // activate/apply
	gi.unicast (self, true);

  self->client->inventoryTime = p_realtime + INVMODE_DISPLAYTIME; // # seconds for display
}


void P_SidekickCommandApply(userEntity_t *self)
{
  //	send the sidekickcommand Apply message 
	gi.WriteByte(SVC_SIDEKICK_COMMAND);
	gi.WriteByte(sidekickcommand_selected);  // selection index
  gi.WriteByte(1);												 // activate/apply
	gi.unicast (self, true);

  self->client->inventoryTime = p_realtime + INVMODE_DISPLAYTIME; // # seconds for display
}



void sidekickcommand_AddCommands(void)
{
  gi.AddCommand ("command_prev" ,  P_SidekickCommandPrev);
	gi.AddCommand ("command_next" ,  P_SidekickCommandNext);
	gi.AddCommand ("command_apply",  P_SidekickCommandApply);
}
