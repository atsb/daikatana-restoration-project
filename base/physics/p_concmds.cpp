///////////////////////////////////////////////////////////////////////////////
//	p_concmds.c
//
//	console commands local to the physics DLL
///////////////////////////////////////////////////////////////////////////////

#include	"p_global.h"
#include	"p_user.h"

#include    "l__language.h"

///////////////////////////////////////////////////////////////////////////////
//	concmd_NoClip
//
//	toggles noclip movement for client
///////////////////////////////////////////////////////////////////////////////

/*
void concmd_NoClip (edict_t *ent)
{
	char	*msg;

	if( SinglePlayerCheat() == FALSE )
	{
		if( p_cheats->value == 0 )
		{
			if( deathmatch->value )
			{
				gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
			}
			return;
		}
	}

	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	}
	else
	{
		ent->movetype = MOVETYPE_NOCLIP;
		msg = "noclip ON\n";
	}

	gi.cprintf (ent, PRINT_HIGH, msg);
}
*/
///////////////////////////////////////////////////////////////////////////////
//	concmd_Scoreboard
///////////////////////////////////////////////////////////////////////////////

void	concmd_Scoreboard (edict_t *ent)
{
	long toggle;

	if (gi.GetArgc() > 1)
	{
		toggle = atol(gi.GetArgv(1));
		
		if (toggle)
		{
			ent->client->showflags |= (SHOW_SCORES | SHOW_SCORES_NOW);	//	send update immediately
		}
		else
		{
			gi.clearxy ();
			ent->client->showflags &= ~(SHOW_SCORES | SHOW_SCORES_NOW);
		}
	}
	else // toggle
	{
		if (ent->client->showflags & (SHOW_SCORES | SHOW_SCORES_NOW))
		{
			gi.clearxy ();
			ent->client->showflags &= ~(SHOW_SCORES | SHOW_SCORES_NOW);
		}
		else
			ent->client->showflags |= (SHOW_SCORES | SHOW_SCORES_NOW);	//	send update immediately
	}
}




void concmd_ShowHuds(edict_t *ent)
// command to show all the huds at once (skill, weapon, scoreboard)
{
	long toggle;

	if (gi.GetArgc() > 1)
	{
		toggle = atol(gi.GetArgv(1));
		
		if (toggle)
		{
			ent->client->showflags |= SHOW_ALLHUDS;	//	send update immediately
		}
		else
		{
			gi.clearxy ();
			ent->client->showflags &= ~SHOW_ALLHUDS;
		}
	}
	else // toggle
	{
		if (ent->client->showflags & SHOW_ALLHUDS)
		{
			gi.clearxy ();
			ent->client->showflags &= ~SHOW_ALLHUDS;
		}
		else
			ent->client->showflags |= SHOW_ALLHUDS;	//	send update immediately
	}

  if (deathmatch->value)                      //  in multiplayer, display scoreboard
    concmd_Scoreboard(ent); 
}




///////////////////////////////////////////////////////////////////////////////
//	concmd_CheckSpam
//
///////////////////////////////////////////////////////////////////////////////

int	concmd_CheckSpam (edict_t *ent)
{
	gclient_t	*client;
	char		msg [128];

	client = ent->client;

	//	if less than .5 seconds has passed, count this as a spam
	if (client->last_message_frame - level.framenum < p_spamticks->value)
	{
		client->spam_count++;

		if (client->spam_count > p_spamcount->value)
		{
			if (p_spamkick->value)
			{
				//	auto-kick the spammer
				gi.cprintf (ent, PRINT_CHAT, "%s.\n",tongue_world[T_PLAYER_SELF_SPAM_KICK]);
// SCG[1/16/00]: 				sprintf (msg, "kick %i\n", ent->s.number - 1);
				Com_sprintf (msg,sizeof(msg), "kick %i\n", ent->s.number - 1);
				gi.CBuf_AddText(msg);

				//	tell everyone else
				gi.bprintf (PRINT_HIGH, "%s %s.\n", ent->netname,tongue_world[T_PLAYER_SPAM_KICK]);

				return	true;
			}
		}
	}

	//	clear spam count if a message hasn't been sent for awhile
	if (client->last_message_frame - level.framenum > p_spamresetticks->value)
	{
		client->spam_count = 0;
	}

	client->last_message_frame = level.framenum;

	return	false;
}

///////////////////////////////////////////////////////////////////////////////
//	concmd_Say
//
///////////////////////////////////////////////////////////////////////////////

void concmd_Say (edict_t *ent)
{
	int			j;
	edict_t		*other;
	char		*p;
	char		text[2048];

	if (gi.GetArgc () < 2)
		return;

	if (concmd_CheckSpam (ent))
		//	client was kicked for spamming
		return;

	Com_sprintf (text, sizeof (text), "%s: ", com.ProperNameOf(ent));

	p = gi.GetArgs ();

	//	skip quotes
	if (*p == '"')
	{
		p++;
		p [strlen (p) - 1] = 0;
	}
	
	strcat (text, p);

	// don't let text be too long for malicious reasons
	if (strlen (text) > 150)
		text [150] = 0;

	strcat (text, "\n");

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, "%s", text);

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];

		if (!other->inuse || !other->client)
			continue;

		gi.cprintf(other, PRINT_CHAT, "\t%s", text);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	concmd_Tell
//
//	sends a chat message to a specific client based on name or number
//
//	format:  tell [name] [message]
//			 tell [number] [message]
///////////////////////////////////////////////////////////////////////////////

void concmd_Tell (edict_t *ent)
{
	edict_t		*other, *check;
	char		*p;
	char		text[2048];
	char		*name;
	int			i, is_number;

	if (gi.GetArgc () < 3)
		return;

	if (concmd_CheckSpam (ent))
		//	client was kicked for spamming
		return;

	name = gi.GetArgv (1);

	//	check for any non-number characters in name
	is_number = true;

	for (i = 0; i < strlen (name); i++)
		if (name [i] < '0' || name [i] > '9')
			is_number = false;

	other = NULL;

	if (is_number)
	{
		i = atoi (name);
		if (i < MAX_EDICTS)
		{
			other = &g_edicts [i + 1];

			if (!other || !other->inuse)
				other = NULL;
		}
	}
	else
	{
		for (i = 0; i < maxclients->value; i++)
		{
			check = &g_edicts [i + 1];
			if (check->netname && !(stricmp (check->netname, name)))
			{
				other = check;
				break;
			}
		}
	}

	if (!other)
	{
		gi.cprintf (ent, PRINT_HIGH, "%s %s.\n", name,tongue_world[T_TELL_CMD_UNKNOWN_PLAYER]);
		return;
	}

	Com_sprintf (text, sizeof (text), "%s: ", com.ProperNameOf(ent));

	p = gi.GetArgs ();

	//	skip past the name
	p = p + strlen (name) + 1;

	//	skip quotes
	if (*p == '"')
	{
		p++;
		p [strlen (p) - 1] = 0;
	}
	
	strcat (text, p);

	// don't let text be too long for malicious reasons
	if (strlen (text) > 150)
		text [150] = 0;

	strcat (text, "\n");

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, "%s", text);

	gi.cprintf(other, PRINT_CHAT, "%s", text);
}

///////////////////////////////////////////////////////////////////////////////
//	concmd_SayTeam
//
///////////////////////////////////////////////////////////////////////////////

void concmd_SayTeam (edict_t *ent)
{
	int		j;
	edict_t	*other;
	char	*p;
	char	text[2048];

	if (gi.GetArgc () < 2)
		return;

	Com_sprintf (text, sizeof (text), "[%s]: ", com.ProperNameOf(ent));

	p = gi.GetArgs ();

	//	skip quotes
	if (*p == '"')
	{
		p++;
		p [strlen (p) - 1] = 0;
	}
	
	strcat (text, p);

	// don't let text be too long for malicious reasons
	if (strlen (text) > 150)
		text [150] = 0;

	strcat (text, "\n");

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, "%s", text);

	if (deathmatch->value)
	{
		if (serverState.GetCvar("deathtag") || serverState.GetCvar("ctf") || serverState.GetCvar("dm_teamplay"))
		{
			for (j = 1; j <= game.maxclients; j++)
			{
				other = &g_edicts[j];

				if (!other->inuse || !other->client || (other->team != ent->team))
					continue;

				gi.cprintf(other, PRINT_CHAT, "\t%s", text);
			}
			return;
		}
	}

	// if we get here...just send it to everybody
	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];

		if (!other->inuse || !other->client)
			continue;

		gi.cprintf(other, PRINT_CHAT, "\t%s", text);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	concmd_AddCommands
///////////////////////////////////////////////////////////////////////////////

void	concmd_AddCommands (void)
{
//	gi.AddCommand ("noclip", concmd_NoClip);
	gi.AddCommand ("scoreboard", concmd_Scoreboard);
	
	if(!(serverState.GetCvar("dedicated") > 0))
		gi.AddCommand ("say", concmd_Say);
	gi.AddCommand ("tell", concmd_Tell);
	gi.AddCommand ("say_team", concmd_SayTeam);
	gi.AddCommand ("say_all", concmd_Say);
	gi.AddCommand ("show_huds", concmd_ShowHuds);
}
