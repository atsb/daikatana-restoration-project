///////////////////////////////////////////////////////////////////////////////
// 	p_selector.cpp
//	  -- command list selector functions
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER
#include	<crtdbg.h>
#endif
#include	"p_global.h"
#include	"p_user.h"

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define NUM_SIDEKICKCOMMANDS (5)

///////////////////////////////////////////////////////////////////////////////
//	functions
///////////////////////////////////////////////////////////////////////////////

#define CAN_COMMAND			(self->inuse && (self->health > 0) && (self->deadflag == DEAD_NO))
void P_CommandSelectorPrev(userEntity_t *self)
// scroll up/back in command list
{
	if (!CAN_COMMAND)
		return;
	//	send the command selector 'Prev' message 
	gi.WriteByte(SVC_SELECTOR);
	gi.WriteShort(-1);            // backward direction
	gi.WriteByte(0);				      // activate/apply = false
	gi.unicast (self, true);
	
	self->client->inventoryTime = p_realtime + INVMODE_DISPLAYTIME; // # seconds for display
}


void P_CommandSelectorNext(userEntity_t *self)
// scroll down/forward in command list
{
	if (!CAN_COMMAND)
		return;
	//	send the command selector 'Next' message 
	gi.WriteByte(SVC_SELECTOR);
	gi.WriteShort(1);	            // forward direction
	gi.WriteByte(0);				      // activate/apply = false
	gi.unicast (self, true);
	
	self->client->inventoryTime = p_realtime + INVMODE_DISPLAYTIME; // # seconds for display
}


void P_CommandSelectorApply(userEntity_t *self)
{
	if (!CAN_COMMAND)
		return;
	//	send the command selector 'Apply' message 
	gi.WriteByte(SVC_SELECTOR);
	gi.WriteShort(0);             // direction
	gi.WriteByte(1);						  // activate/apply = true
	gi.unicast (self, true);
	
	self->client->inventoryTime = p_realtime + INVMODE_DISPLAYTIME; // # seconds for display
}

void cmdselector_AddCommands(void)
{
	gi.AddCommand ("command_prev" ,  P_CommandSelectorPrev);
	gi.AddCommand ("command_next" ,  P_CommandSelectorNext);
	gi.AddCommand ("command_apply",  P_CommandSelectorApply);
}
