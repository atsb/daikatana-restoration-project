#include	<stdio.h>
#include	<math.h>
//#include	<windows.h>
#include	"dk_system.h"

//	header files in daikatana\user
#include	"p_user.h"
#include	"hooks.h"

#include	"p_global.h"

qboolean com_TeamCheck( userEntity_t *attacker, userEntity_t *target)
{
	// only clients can be on teams
	if(!(attacker->flags & FL_CLIENT) || !(target->flags & FL_CLIENT) )
	{
		return 0;
	}

	cvar_t *ctf = serverState.cvar("ctf","0",CVAR_SERVERINFO|CVAR_LATCH);
	cvar_t *deathtag = serverState.cvar("deathtag","0",CVAR_SERVERINFO|CVAR_LATCH);
	// make sure that deathmatch, teamplay or coop are set ctf is always teamplay
	bool bIsTeamPlay = (deathmatch->value && dm_teamplay->value) || (ctf->value || coop->value || deathtag->value);
	if(!bIsTeamPlay)
	{
		return 0;
	}

	// on the same team
	if (coop->value > 0.0f)			// if we get here, both are guaranteed to be clients...hence on the same team
	{
		return 1;
	}
	else
	{
		if(attacker->team == target->team)
		{
			return 1;
		}
	}
	return 0;
}

qboolean com_CanHurt( userEntity_t *attacker, userEntity_t *target )
{
	if (!attacker || !target)
		return TRUE;

	if (attacker == target)
		return TRUE;

	if (!(attacker->flags & FL_CLIENT) || !(target->flags & FL_CLIENT))	// if they're not both clients, we can always hurt!
		return TRUE;

	if (com_TeamCheck(attacker,target))		// on same team!
	{	
		if (!dm_friendly_fire->value)
			return FALSE;
	}

	return TRUE;
}