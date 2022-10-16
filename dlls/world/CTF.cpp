// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#include "dk_system.h"

#ifndef WIN32
#include <dlfcn.h>
#endif

#include "world.h"
#include "ai_utils.h"
#include "dk_buffer.h"
#include "flag.h"


///////////////////////////////////////////////////////////////////////////////
//	NAMES OF THINGS
///////////////////////////////////////////////////////////////////////////////
#define CTF_SOUND_ALARM				"global/e_alarmb.wav"		// Played when the player first steals the flag.
#define CTF_SOUND_FLAG_PICKUP		"global/a_hpick.wav"
#define CTF_SOUND_VICTORY			"global/bossdeath6.wav"
#define CTF_SOUND_FLAG_RETURNED		"global/a_hpick.wav"

///////////////////////////////////////////////////////////////////////////////
//	Other Global defines.
///////////////////////////////////////////////////////////////////////////////
#define FLAG_RETURN_TIME			60.0
#define DEBUG_MSG(str)					gstate->Con_Dprintf("%s\n",str)
#define MAX_SPAWNPOINTS		16

///////////////////////////////////////////////////////////////////////////////
//	typedef
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	point bonus stuff
///////////////////////////////////////////////////////////////////////////////
#define DEFEND_BASE_BONUS				1
#define DEFEND_FLAG_BONUS				1
#define KILL_ENEMY_CARRIER_BONUS		2
#define DEFEND_FLAG_CARRIER_BONUS		1
#define PUT_FLAG_BACK_BONUS				1
#define CAPTURER_BONUS					5
#define TEAM_CAPTURE_BONUS				5
#define CAPTURE_DEFEND_BASE_BONUS		1

///////////////////////////////////////////////////////////////////////////////
//	cvars
///////////////////////////////////////////////////////////////////////////////
cvar_t	*ctf;
static cvar_t	*fraglimit;
static cvar_t	*timelimit;

///////////////////////////////////////////////////////////////////////////////
//	local vars and stuff
///////////////////////////////////////////////////////////////////////////////
userEntity_t	*entFlags[CTF_TEAMS];

static char *strItemFlagName[ CTF_TEAMS] =
{
	"item_flag_team",
	"item_flag_team1",
	"item_flag_team2"
};


static char *strTriggerCaptureName[ CTF_TEAMS ] =
{
	NULL,
	"trigger_capture_flag1",
	"trigger_capture_flag2"
};

static char *strControlCardName[ CTF_TEAMS ] =
{
	NULL,
	"item_control_card_red",
	"item_control_card_blue"
};

static char *strControlCardModelName[ CTF_TEAMS ] = 
{
	NULL,
	"models/global/ctf_clcrd.dkm",
	"models/global/ctf_clcbl.dkm",
};

flag_model_info_t ctf_flag_model_info_l = 
{
	"models/global/a_ctf_flagl.dkm",
	NULL,
	"skins/ctf_flaglred.wal",	
	"skins/ctf_flaglblu.wal",	
	"skins/ctf_flaglchr.wal",	
	"skins/ctf_flaglchr2.wal",	
	"skins/ctf_flaglgrn.wal",	
	"skins/ctf_flaglorang.wal",
	"skins/ctf_flaglpurp.wal",	
	"skins/ctf_flaglyel.wal",
	8,
	4,6,5
};

void CTFFlagSpawn( userEntity_t *self, int team );
userEntity_t *CTFFlagSpawn( int team, const CVector &origin, const CVector &angles );
//================================================================================
// All the goodness starts here
//=============================================================================
// int GetAllEntities( const char *name, userEntity_t **entarr, int max )
// 
// 
//=============================================================================
int GetAllEntities( const char *name, userEntity_t **entarr, int max )
{
	userEntity_t *ent;
	int cnt = 0;

	if (*name == '*')
	{
		name++;
		int len = strlen( name );
		for(ent = gstate->FirstEntity(); ent; ent = gstate->NextEntity( ent ))
		{
			if (ent->className && !_strnicmp( ent->className, name, len ))
			{
				entarr[ cnt++ ] = ent;
				if (cnt >= max)
					break;
			}
		}
		return cnt;
	}

	for(ent = gstate->FirstEntity(); ent; ent = gstate->NextEntity( ent ))
	{
		if (ent->className && !stricmp( ent->className, name))
		{
			entarr[ cnt++ ] = ent;
			if (cnt >= max)
				break;
		}
	}
	return cnt;
}

//=============================================================================
// static userEntity_t *findFarthestDM( userEntity_t *ent )
// 
// 
//=============================================================================
static userEntity_t *findFarthestDM( userEntity_t *ent )
{
	userEntity_t *ans = NULL;
	float dist = 0.0f;
	float ndist = 0.0f;
	userEntity_t *entarr[ MAX_SPAWNPOINTS ];
	int cnt = GetAllEntities( "info_player_deathmatch", entarr, MAX_SPAWNPOINTS );
	int i;
	for(i=0;i<cnt-1;i++)
	{
		 ndist = VectorDistance2( entarr[i]->s.origin, ent->s.origin );
		 if (ndist > dist)
		 {
			 dist = ndist;
			 ans = entarr[i];
		 }
	}
	return ans;
}

//=============================================================================
// static int findClosest( userEntity_t *ent, userEntity_t **entarr, int max )
// 
// 
//=============================================================================
static int findClosest( userEntity_t *ent, userEntity_t **entarr, int max )
{
	int ans = -1;
	userEntity_t **ppe;
	float dist = 1e15f;
	float ndist;
	int i;
	for(i=0,ppe = entarr;i<max;i++,ppe++)
	{
		if (*ppe)
		{
			ndist = VectorDistance2( (*ppe)->s.origin, ent->s.origin );
			if (ndist < dist)
			{
				dist = ndist;
				ans = i;
			}
		}
	}
	return ans;
}

	
void CTFInitData()
{
	// set up the default color choices
	teamConvert[CTF_TEAM1] = 1;
	teamConvert[CTF_TEAM2] = 2;

	memset(&flagNames,0,sizeof(flagNames));
	Com_sprintf(flagNames[CTF_TEAM1],sizeof(flagNames[CTF_TEAM1]),tongue_ctf[T_CTF_FLAG_NAME],teamInfo[teamConvert[CTF_TEAM1]].teamName);
	Com_sprintf(flagNames[CTF_TEAM2],sizeof(flagNames[CTF_TEAM2]),tongue_ctf[T_CTF_FLAG_NAME],teamInfo[teamConvert[CTF_TEAM2]].teamName);

	flag_info.bUseSkins = true;
	flag_info.modelInfo[CTF_TEAM1] = flag_info.modelInfo[CTF_TEAM2] = &ctf_flag_model_info_l;
}
//=============================================================================
// void CTFInitCVars(void)
// 
// 
//=============================================================================
void CTFInitCVars(void)
{
	ctf				= gstate->cvar("ctf", "0", CVAR_SERVERINFO|CVAR_LATCH);
	fraglimit		= gstate->cvar("fraglimit","", 0);
	timelimit		= gstate->cvar("timelimit","", 0);
/*	ctf_forcejoin = gi.cvar("ctf_forcejoin", "", 0);
	competition = gi.cvar("competition", "0", CVAR_SERVERINFO);
	matchlock = gi.cvar("matchlock", "1", CVAR_SERVERINFO);
	electpercentage = gi.cvar("electpercentage", "66", 0);
	matchtime = gi.cvar("matchtime", "20", CVAR_SERVERINFO);
	matchsetuptime = gi.cvar("matchsetuptime", "10", 0);
	matchstarttime = gi.cvar("matchstarttime", "20", 0);
	admin_password = gi.cvar("admin_password", "", 0);
	warp_list = gi.cvar("warp_list", "q2ctf1 q2ctf2 q2ctf3 q2ctf4 q2ctf5", 0);
*/
}

void CTFLevelStart()
{
	if (!ctf->value)
		return;
}

void CTFTeamStats(userEntity_t *self, int team, long &capPerc, long &teamPerc, long &flags)
{
	flags = 0;

	// check if your team's flag has a carrier
	userEntity_t *teamFlag = FLAG_Team(team);
	if (!teamFlag || (!FLAG_Carrier(team) && !(teamFlag->spawnflags & DROPPED_ITEM)))
		flags |= STAT_CTF_YOUR_FLAG_HOME;

	if (team == self->team)
	{
		flags |= STAT_CTF_YOUR_TEAM;
		// see if you're carrying a flag
		if (FLAG_Carried_By(self))
			flags |= STAT_CTF_YOU_HAVE_ENEMY_FLAG;
	}

	// send over the team conversion
	flags += ((teamConvert[team]) << STAT_CTF_TEAM_CONVERT_SHIFT);

	// send over the number of captures
	flags += ((ctfgame.score[team]) << STAT_CTF_TEAM_CAPTURES_SHIFT);

	// send down the team info
	int team1,team2;
	TEAM_GetPlayerCount(team1,team2);
	if (team1+team2 > 0)
	{
		if (team == CTF_TEAM1)
			teamPerc = (100*team1) / (team1+team2);
		else if (team == CTF_TEAM2)
			teamPerc = (100*team2) / (team1+team2);
		else
			capPerc = teamPerc = 0;
	}
	else
		capPerc = teamPerc = 0;

	// how close are we to winning?
	if (fraglimit->value)
		capPerc = (long)( 100 * (ctfgame.score[team] / fraglimit->value));
	else
		capPerc = 0;
}

//DllExport int dll_CTFGetScores(teamInfo_t *scorearr, int maxscores)
int CTFGetScores(teamInfo_t *scorearr, int maxscores)
{
	if (!ctf->value)
		return 0;

	int cnt = 0;
	int i,c;

	for(i=CTF_TEAM1;i<CTF_TEAMS;i++,cnt++)
	{
		if (cnt >= maxscores)
			break;
		teamInfo_t *score = (scorearr++);

		score->captures = ctfgame.score[i];
		score->teamNum = teamConvert[i];//teamInfo[teamConvert[i]].teamName;
		
		int teamscore = 0;
		userEntity_t *ent;
		for (c = 0; c < maxclients->value; c++)
		{
			ent = &gstate->g_edicts [c + 1];
			if (!ent->inuse)
				continue;

			if (ent->team == i)
				teamscore += ent->record.frags;
		}
		
		score->score = teamscore;
	}
	return cnt;
}

//DllExport int dll_CTFCheckRules()

void CTFEndGameMessage(long message, int winner, int loser, int wscore, int lscore)
{
	gstate->cs.BeginSendString();
	gstate->cs.SendSpecifierStr("%s%s",2);
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,message));
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_TEAM_WON));
	gstate->cs.Continue(true);
	gstate->cs.SendCount(5);
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,teamInfo[winner].teamNameID));
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,teamInfo[winner].teamNameID));
	gstate->cs.SendInteger(wscore);
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,teamInfo[loser].teamNameID));
	gstate->cs.SendInteger(lscore);
	FLAG_CS_Message(0,NULL,2.0);
	gstate->cs.EndSendString();
}

int CTFCheckRules()
{
	if (!ctf->value)
		return 0;

	// find the winning and losing teams!
	int winner(0),loser(0),wscore(0),lscore(0);

	teamInfo_t scorearr[2];
	int teams = CTFGetScores( scorearr, 2 );

	if (scorearr[0].captures > scorearr[1].captures)
	{
		wscore = scorearr[0].captures;
		lscore = scorearr[1].captures;
		winner = 1;
		loser = 2;
	}
	else if (scorearr[1].captures > scorearr[0].captures)
	{
		wscore = scorearr[1].captures;
		lscore = scorearr[0].captures;
		winner = 2;
		loser = 1;
	}

	// have we reached an end-game condition?
	bool limit = (fraglimit->value && (wscore >= fraglimit->value));
	bool timeup = ( (timelimit->value) &&  (gstate->level->time > timelimit->value*60) );

	if (TEAM_Overtime())
	{
		if (winner && loser)
		{
			CTFEndGameMessage(T_CTF_LIMIT_REACHED,winner,loser,wscore,lscore);
			return 1;
		}
	}
	else
	{
		if (limit || timeup)
		{
			if (limit)	// cap limit...always a winner/loser
			{
				CTFEndGameMessage(T_CTF_LIMIT_REACHED,winner,loser,wscore,lscore);
			}
			else		// time limit...can be a tie
			{
				if (winner && loser)
				{
					CTFEndGameMessage(T_CTF_TIME_EXPIRED,winner,loser,wscore,lscore);
				}
				else
				{
					TEAM_SetOvertime(TRUE);
					return 0;
				}
			}
			return 1;
		}
	}

	return 0;
}

//=============================================================================
// void CTFInitialize()
// 
// Starts up all the object needed for capture the flag (if not in the map)
// Also will change spawn points to team points if nessary.
//=============================================================================

bool CTFInitialize()
{
	userEntity_t *entarr[ MAX_SPAWNPOINTS ];
	int cnt;

	if (!ctf->value)
		return FALSE;
	if (!deathmatch->value)
	{
		gstate->SetCvar( "ctf", "0" );
		return FALSE;
	}

	// Clear out all flags.
	int i,j;


	for (i=0;i<CTF_TEAMS;i++)
		entFlags[i] = NULL;
	// Find all the flags...
	char buf[128];
	for(i=CTF_TEAM1;i<CTF_TEAMS;i++)
	{
		Com_sprintf(buf,sizeof(buf),"item_flag_team%d",i);
		entFlags[i] = com->FindEntity( buf );
	}
	// If we don't have two teams... then we better deal 
	if (!entFlags[CTF_TEAM1] || !entFlags[CTF_TEAM2])
	{
		// OK this is probably not a capture the flag level.   But thats all right, we'll fake it.
		// What we will do is use the spawn points to place the flag and teams and stuff.
		
		userEntity_t	*e1 = NULL, *e2 = NULL;
		bool			e1del = false;
		bool			e2del = false;
		// OK Here is how it will work... First find one possible object.
		if (!(e1 = entFlags[CTF_TEAM1]))
		{
			if (!(e1 = com->FindEntity( strTriggerCaptureName[ CTF_TEAM1 ] )))
				e1 = com->FindEntity( strTeamStart[ CTF_TEAM1 ] );
		}
		if (!(e2 = entFlags[CTF_TEAM2]))
		{
			if (!(e2 = com->FindEntity( strTriggerCaptureName[ CTF_TEAM2 ] )))
				e2 = com->FindEntity( strTeamStart[ CTF_TEAM2 ] );
		} 
		if (!e1 && !e2)
		{
			// OK - Use the two deathmatch that are the farthest away from each other.
			cnt = GetAllEntities( "info_player_deathmatch", entarr, MAX_SPAWNPOINTS );
			if (cnt < 2)
			{
				// Give up this is never going to work.
				gstate->Error("This map cannot become a Capture the flag map");
				return FALSE;
			}
			float dist = 0.0f;
			float ndist;
			for(i=0;i<cnt-1;i++)
			{
				for(j=i+1;j<cnt;j++)
				{
					 ndist = VectorDistance2( entarr[i]->s.origin, entarr[j]->s.origin );
					 if (ndist > dist)
					 {
						 dist = ndist;
						 e1 = entarr[i];
						 e2 = entarr[j];
					 }
				}
			}
			if (cnt >= 6) 
				e1del = e2del = true;
		}
		else if (!e1 && e2)
			e1 = findFarthestDM( e2 );
		else if (e1 && !e2)
			e2 = findFarthestDM( e1 );
		if (!e1 || !e2)
		{
			// Give up this is never going to work.
			gstate->Error("This map cannot become a Capture the flag map");
			return FALSE;
		}

		// TEMP!!!!
		entFlags[CTF_TEAM1] = CTFFlagSpawn( CTF_TEAM1, e1->s.origin, e1->s.angles );
		entFlags[CTF_TEAM2] = CTFFlagSpawn( CTF_TEAM2, e2->s.origin, e2->s.angles );

		if (e1del)
			e1->remove(e1);
		if (e2del)
			e2->remove(e1);

		// TODO: Build the flags that we don't have here!

		// OK, now see if we have team starting points.  If we do not then divy up the deathmatch start points to the other players.
		if (!com->FindEntity( strTeamStart[ CTF_TEAM1 ] ) || !com->FindEntity( strTeamStart[ CTF_TEAM2 ] )) // Just to make things simple, divie up the spawn points if either one isn't available.
		{
			cnt = GetAllEntities( "info_player_deathmatch", entarr, MAX_SPAWNPOINTS );
			// Use the team choose method to divide up spawn points.
			int team = CTF_TEAM1;
			while(1)
			{
				i = findClosest( entFlags[team], entarr, cnt );
				if (i < 0)
					break;
				e1 = entarr[i];
				e1->className = strTeamStart[ team ];
				entarr[i] = NULL;
				team = (team == CTF_TEAM1 ? CTF_TEAM2:CTF_TEAM1);
			}
		}
	}

	userEntity_t *triggers[CTF_TEAMS-1];
	memset(&triggers,0,sizeof(triggers));
	GetAllEntities("trigger_capture",triggers,CTF_TEAMS-1);
	if (!triggers[CTF_TEAM1-1])
	{
		ctfgame.flagReturns[CTF_TEAM1] = CTFCaptureSpawn( CTF_TEAM1, entFlags[CTF_TEAM1]->s.origin, entFlags[CTF_TEAM1]->s.angles, CVector(-20, -20,-20),CVector(20, 20,20));		
	}

	if (!triggers[CTF_TEAM2-1])
	{
		ctfgame.flagReturns[CTF_TEAM2] = CTFCaptureSpawn( CTF_TEAM2, entFlags[CTF_TEAM2]->s.origin, entFlags[CTF_TEAM2]->s.angles, CVector(-20, -20,-20),CVector(20, 20,20));
	}
	return TRUE;
}


//=============================================================================
// void CTFClientConnect( userEntity_t *self, char * userinfo )
// 
// 
//=============================================================================
void CTFClientConnect( userEntity_t *self, char * userinfo )
{
//	self->team = (atoi(Info_ValueForKey (userinfo, "ctf_team")));
}

void item_flag_thinkreturn( userEntity_t *self )
{
	if ( (gstate->time > self->hacks) || (gstate->PointContents(self->s.origin) & CONTENTS_LAVA) )
	{
		self->hacks = 0.0;
		ctfFlagHook_t	*ihook = (ctfFlagHook_t *) self->userHook;

		FLAG_Reset(self);
		self->s.effects |= EF_RESPAWN;	//	tell client to start respawn effect

		if (ihook && ihook->soundFlagReturned)
			gstate->StartEntitySound(self, CHAN_OVERRIDE, ihook->soundFlagReturned, 0.85f, 100000, 100000 );

		FLAG_RETURN_MESSAGE(tongue_ctf,T_CTF_FLAG_RETURNED,tongue_ctf,T_CTF_FLAG_NAME,self->team);
		return;
	}

	self->nextthink = gstate->time + 1.0;
}

//=============================================================================
// void CTFClientDropFlag( userEntity_t *self, userEntity_t *flag )
// 
// 
//=============================================================================
void CTFClientDropFlag( userEntity_t *self, userEntity_t *flag )
{
	flag->think		= item_flag_thinkreturn;
	flag->nextthink	= gstate->time + 1.0;
	flag->hacks		= gstate->time + FLAG_RETURN_TIME;
}

//=============================================================================
// void CTFClientDie( userEntity_t *self )
// 
// 
//=============================================================================
void CTFClient_Die (userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point)
{
	// award some bonus points
	if (attacker && self && (attacker != self) && (attacker->team != self->team) && (self->flags & FL_CLIENT) && (attacker->flags & FL_CLIENT) )
	{
		userEntity_t *attCap	= FLAG_ClosestCapture(attacker);	// closest capture pad for attacker's team
		userEntity_t *attFlag	= FLAG_Team(attacker->team);		// attacker team's flag
		userEntity_t *selfCarr	= FLAG_Carrier(self->team);			// carrier of self team's flag
		userEntity_t *attCarr	= FLAG_Carrier(attacker->team);		// carrier of attacker team's flag

		// if both players see the attacker's score pad, give attacker DEFEND_BASE_BONUS
		if (attCap && com->Visible(attacker,attCap) && com->Visible(self,attCap))
		{
			DEBUG_MSG("Base defense bonus");
			attacker->record.frags += DEFEND_BASE_BONUS;
		}

		// if both players see the attacker's flag and the flag is not being carried, give attacker DEFEND_FLAG_BONUS
		if (!attCarr && attFlag && com->Visible(self,attFlag) && com->Visible(attacker,attFlag))
		{
			DEBUG_MSG("Flag defense bonus");
			attacker->record.frags += DEFEND_FLAG_BONUS;
		}

		// if self had attacker's flag, give attacker KILL_ENEMY_CARRIER_BONUS
		if (attCarr == self)
		{
			DEBUG_MSG("Kill enemy carrier bonus");
			attacker->record.frags += KILL_ENEMY_CARRIER_BONUS;
		}

		// if attacker's flag carrier is visible, give attacker DEFEND_FLAG_CARRIER_BONUS
		if (selfCarr && (selfCarr != attacker) && com->Visible(attacker,selfCarr))
		{
			DEBUG_MSG("Flag carrier defense bonus");
			attacker->record.frags += DEFEND_FLAG_CARRIER_BONUS;
		}
	}

	FLAG_Drop(self);
}

void CTFClientDisconnect(userEntity_t *self)
{
	FLAG_Drop(self);
	self->team = 0;
}





#define	MAX_INTERMISSION_POINTS	16
/*
userEntity_t *FindIntermissionSpot(int num = -1)
{
	int numspots;
	userEntity_t *spot[MAX_INTERMISSION_POINTS];
	userEntity_t *ent;

	numspots = GetAllEntities( "info_player_intermission", spot, MAX_INTERMISSION_POINTS );
	if (numspots == 0)
	{
		if ((spot[0] = com->FindEntity ("info_player_start")) != NULL)
			numspots = 1;
		else if ((spot[0] = com->FindEntity ("info_player_deathmatch")) != NULL)
			numspots = 1;
	}
	if (num < 0)
		ent = spot [rand () % numspots];
	else if (num >= numspots)
		ent = spot[ numspots - 1 ];
	else 
		ent = spot[ num ];
	return ent;
}
*/

void CTFStartTeamMenu( userEntity_t *self )
{
//	userEntity_t *spot = FindIntermissionSpot(0);

	self->team = 0;
//	self->s.origin = spot->s.origin;
//	self->s.angles = spot->s.angles;
//	self->client->ps.viewangles = spot->s.angles;
	self->client->ps.pmove.origin[0] = gstate->level->intermission_origin[0]*8;
	self->client->ps.pmove.origin[1] = gstate->level->intermission_origin[1]*8;
	self->client->ps.pmove.origin[2] = gstate->level->intermission_origin[2]*8;
	self->client->ps.viewoffset.Zero();
	self->client->ps.blend[3] = 0;
//	self->client->ps.gunindex = 0;
	self->solid		= SOLID_NOT;
	self->movetype	= MOVETYPE_NONE;
	self->flags		|= FL_FREEZE;
	self->svflags	|= SVF_NOCLIENT;
}

bool CTFClientThink( userEntity_t *self )
{
	return FALSE; // Continue to think.
//	self->flags		|= FL_FREEZE;
}

//=============================================================================
// void CTFGiveControlCard( userEntity_t *self )
// 
// 
//=============================================================================
void CTFGiveControlCard( userEntity_t *self )
{
	if (!ctf->value || (self->team < CTF_TEAM1) || (self->team >= CTF_TEAMS))
		return;

	userInventory_t	*invitem;

	
	if (gstate->InventoryFindItem (self->inventory, strControlCardName[self->team]))
		return;

	invitem = gstate->InventoryCreateItem( self->inventory, strControlCardName[ self->team ],
 				NULL,
				NULL,
				gstate->ModelIndex (strControlCardModelName[ self->team ]), 
				ITF_SPECIAL | ITF_INVSACK, 
				sizeof( userInventory_t ));
	if (!invitem)
		return;

	gstate->InventoryAddItem (self, self->inventory, invitem); 
}

void CTFInitClient( userEntity_t *self )
{
	if (!ctf->value)
		return;

//	CTFGiveControlCard(self);
}

//=============================================================================
// void CTFPutClientInServer(userEntity_t *self)
// 
// called when a client enters a ctf game.  sets the player's team based on
// the value of ctf_team.  Also determines the player's starting spawn point
//=============================================================================
userEntity_t *CTFPutClientInServer(userEntity_t *self)
{
	if (!ctf->value)
		return NULL;

	TEAM_Verify(self,"ctf_team",NULL);
	CTFGiveControlCard(self);
	return TEAM_GetSpawnPoint(self);
}

void CTFClient_Respawn(userEntity_t *self)
{
	CTFGiveControlCard(self);
}

//=============================================================================
// void CTFFlagCaptured( userEntity_t *self, userEntity_t *capturer )
// 
// 
//=============================================================================
void CTFFlagCaptured( userEntity_t *self, userEntity_t *capturer, userEntity_t *trigger )
{
	ctfFlagHook_t	*ihook = (ctfFlagHook_t *) self->userHook;
	// Award points.
	ctfgame.score[ capturer->team ] ++;

	// Reset flag.
	FLAG_Reset(self);
	self->s.effects |= EF_RESPAWN;	//	tell client to start respawn effect

	// We want to here the sound ALL OVER the map.
	gstate->StartEntitySound(capturer, CHAN_OVERRIDE, ihook->soundVictory, 1.0f, 100000.0f, 100000.0f);


	// "You have captured the %s!"
	gstate->cs.BeginSendString();
	gstate->cs.SendSpecifierStr("%s%s",2);
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_YOU_CAPTURED));
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_SCORE));
	gstate->cs.Continue(true);
	gstate->cs.SendCount(5);
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, T_CTF_FLAG_NAME));
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[teamConvert[CTF_TEAM1]].teamNameID));
	gstate->cs.SendInteger(ctfgame.score[CTF_TEAM1]);
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[teamConvert[CTF_TEAM2]].teamNameID));
	gstate->cs.SendInteger(ctfgame.score[CTF_TEAM2]);
	gstate->cs.Continue(true);
	gstate->cs.SendCount(1);
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[teamConvert[self->team]].teamNameID));
	gstate->cs.Unicast(capturer,CS_print_center,2.0);
	gstate->cs.EndSendString();

	// "%s from your team has captured the %s!"
	gstate->cs.BeginSendString();
	gstate->cs.SendSpecifierStr("%s%s",2);
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_YOUR_TEAM_CAPTURED));
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_SCORE));
	gstate->cs.Continue(true);
	gstate->cs.SendCount(6);
	gstate->cs.SendString(capturer->netname);
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, T_CTF_FLAG_NAME));
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[teamConvert[CTF_TEAM1]].teamNameID));
	gstate->cs.SendInteger(ctfgame.score[CTF_TEAM1]);
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[teamConvert[CTF_TEAM2]].teamNameID));
	gstate->cs.SendInteger(ctfgame.score[CTF_TEAM2]);
	gstate->cs.Continue(true);
	gstate->cs.SendCount(1);
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[teamConvert[self->team]].teamNameID));
	FLAG_CS_Message(capturer->team,capturer,2.0f);
	gstate->cs.EndSendString();

	// "%s from the %s team has captured your flag!"
	gstate->cs.BeginSendString();
	gstate->cs.SendSpecifierStr("%s%s",2);
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_OTHER_TEAM_CAPTURED));
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_SCORE));
	gstate->cs.Continue(true);
	gstate->cs.SendCount(6);
	gstate->cs.SendString(capturer->netname);
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[capturer->team].teamNameID));
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[teamConvert[CTF_TEAM1]].teamNameID));
	gstate->cs.SendInteger(ctfgame.score[CTF_TEAM1]);
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[teamConvert[CTF_TEAM2]].teamNameID));
	gstate->cs.SendInteger(ctfgame.score[CTF_TEAM2]);
	FLAG_CS_Message(-capturer->team,capturer,2.0f);
	gstate->cs.EndSendString();

	// award the bonus points
	capturer->record.frags += CAPTURER_BONUS;
	FLAG_AwardPoints(capturer->team,NULL,NULL,TEAM_CAPTURE_BONUS);

	// give defenders in sight of the capture trigger a bonus too... promote base defense!
	if (trigger)
		FLAG_AwardPoints(capturer->team,capturer,trigger,CAPTURE_DEFEND_BASE_BONUS);
}

void	item_flag_touch (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!com->ValidTouch (self, other))
		return;

//	playerHook_t	*ohook = AI_GetPlayerHook( other );
	ctfFlagHook_t	*ihook = (ctfFlagHook_t *) self->userHook;

	// Only Players may interact with the game.
	if (stricmp( "player", other->className ))
		return;

	userEntity_t *otherflag = FLAG_Carried_By(other);

	if (other->team == self->team)
	{
		if (self->spawnflags & DROPPED_ITEM)
		{
			// TODO: Let Everyone know that the flag has been returned.
			// 
			// Return the flag to it's start now.
			FLAG_RETURN_MESSAGE(tongue_ctf,T_CTF_FLAG_RETURNED,tongue_ctf,T_CTF_FLAG_NAME,self->team);
			gstate->StartEntitySound(other, CHAN_OVERRIDE, ihook->soundFlagReturned, 0.85f, 100000, 100000 );
			DEBUG_MSG("Flag replaced bonus");
			other->record.frags += PUT_FLAG_BACK_BONUS;
			FLAG_Reset(self);
		}
		return;	
	}

	// Pick up the flag if we don't have a flag.... This might be the case if we convert up to three teams.
	// 
	if (otherflag)
		return;		// Cannot have two flags.

	gstate->StartEntitySound(other, CHAN_VOICE, ihook->soundPickupIndex, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	if (!(self->spawnflags & DROPPED_ITEM))
		gstate->StartEntitySound(other, CHAN_OVERRIDE, ihook->soundAlarm, 0.85f, 1000.0f, 3000.0f);

	FLAG_Grab(self,other);

	// Let the other players know about it.
	// you grabbed the %s
	gstate->cs.BeginSendString();
	gstate->cs.SendSpecifierID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_YOU_HAVE_GOT_FLAG),1);
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_FLAG_NAME));
	gstate->cs.Continue(true);
	gstate->cs.SendCount(1);
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,teamInfo[teamConvert[self->team]].teamNameID));
	gstate->cs.Unicast(other,CS_print_center,2.0);
	gstate->cs.EndSendString();

	//%s from your team has grabbed the %s
	gstate->cs.BeginSendString();
	gstate->cs.SendSpecifierID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_GRABBED_FLAG),2);
	gstate->cs.SendString(other->netname);
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_FLAG_NAME));
	gstate->cs.Continue(true);
	gstate->cs.SendCount(1);
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,teamInfo[teamConvert[self->team]].teamNameID));
	FLAG_CS_Message(  other->team, other, 2.0f );
	gstate->cs.EndSendString();

	//%s has stolen your flag.
	gstate->cs.BeginSendString();
	gstate->cs.SendSpecifierID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_STOLEN_FLAG),1);
	gstate->cs.SendString(other->netname);
	FLAG_CS_Message( -other->team, other, 2.0f );
	gstate->cs.EndSendString();

}

void CTFFlagReset( userEntity_t *self )
{
	self->touch			= item_flag_touch;
}

void CTFFlagSpawn( userEntity_t *self, int team )
{
	ctfFlagHook_t *ihook = (ctfFlagHook_t *) self->userHook;
	if (!ihook)
		return;

	ihook->soundAlarm		= gstate->SoundIndex( CTF_SOUND_ALARM );
	ihook->soundPickupIndex = gstate->SoundIndex( CTF_SOUND_FLAG_PICKUP );
	ihook->soundVictory		= gstate->SoundIndex( CTF_SOUND_VICTORY );
	ihook->soundFlagReturned= gstate->SoundIndex( CTF_SOUND_FLAG_RETURNED );
}

userEntity_t *CTFFlagSpawn( int team, const CVector &origin, const CVector &angles )
{
	if ((team < CTF_TEAM1) || (team >= CTF_TEAMS))
		return NULL;

	userEntity_t *self = gstate->SpawnEntity();
	self->className = strItemFlagName[team];

	self->s.origin = origin;
	self->s.angles = angles;
	CTFFlagSpawn( self, team );
	return self;
}

///////////////////////////////////////////////////////////////////////////////
// trigger_capture_team*
///////////////////////////////////////////////////////////////////////////////
void	ctf_trigger_capture_touch(userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!com->ValidTouch (self, other))
		return;
	
 	if (stricmp( "player", other->className ))
		return;

	userEntity_t *otherflag = FLAG_Carried_By( other ); 
	if (!otherflag)
		return;

	if (other->team == self->team)
	{
		// bFast specifies whether or not the team's flag must be in place to capture
//		bool bFast = true;

//		if (bFast)
//		{
//			CTFFlagCaptured( otherflag, other, self );
//		}
//		else
//		{
			userEntity_t *ourflag = FLAG_Team(other->team);
			if (!ourflag || (ourflag->spawnflags & DROPPED_ITEM))
				return;

			CTFFlagCaptured( otherflag, other, self );
//		}
	}
}

void CTFCaptureSpawn( userEntity_t *self, int team )
{
	if (!ctf->value || (team < CTF_TEAM1) || (team >= CTF_TEAMS))
		return;

	self->solid			= SOLID_TRIGGER;
	self->movetype		= MOVETYPE_NONE;
	self->svflags		|= SVF_NOCLIENT;
	self->team			= team;

	gstate->SetModel (self, self->modelName);
	gstate->LinkEntity (self);


	self->touch			= ctf_trigger_capture_touch;
	self->className		= "trigger_capture";
}

userEntity_t *CTFCaptureSpawn( int team, const CVector &origin, const CVector &angles, const CVector &min, const CVector &max )
{
	if (!ctf->value || (team < CTF_TEAM1) || (team >= CTF_TEAMS))
		return NULL;

	unsigned long sf;
	if (team == CTF_TEAM1)
		sf = SPAWNFLAG_TEAM1;
	else
		sf = SPAWNFLAG_TEAM2;

	return FLAG_CaptureSpawn(sf,1,origin,angles,min,max);
/*
	userEntity_t *self = gstate->SpawnEntity();

	self->className = strTriggerCaptureName[team];

	self->s.origin	= origin;
	self->s.angles	= angles;
	self->s.mins	= min;
	self->s.maxs	= max;
	CTFCaptureSpawn(self,team);

	return self;*/
}
