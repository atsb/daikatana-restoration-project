#include "dk_system.h"

#ifndef WIN32
#include <dlfcn.h>
#endif

#include "world.h"
#include "dk_buffer.h"
#include "flag.h"


///////////////////////////////////////////////////////////////////////////////
//	cool functions for the explosion
///////////////////////////////////////////////////////////////////////////////
void spawnPolyExplosion(CVector &org, float scale, float lsize, CVector &color, short flags);
void spawn_small_sprite_explosion (userEntity_t *self, CVector &org, int play_sound);
void spawn_sprite_explosion (userEntity_t *self, CVector &org, int play_sound);

///////////////////////////////////////////////////////////////////////////////
//	pack functions
///////////////////////////////////////////////////////////////////////////////
void item_pack_return_think(userEntity_t *self);
void item_pack_explode(userEntity_t *self);
void item_pack_touch(userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf);
void item_pack_think(userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////
#define PACK_MAX_WAIT_TIME			5
#define PACK_BOOM_TIME				90
#define PACK_BEEP_TIME				80
#define PACK_BEEP_DURATION			10
#define PACK_BEEP_INTERVAL			1
#define PACK_RETURN_TIME			10
#define PACK_DAMAGE					1000
#define PACK_RANGE					400

#define PACK_SOUND_TICK				"global/a_ames.wav"
#define PACK_SOUND_BEAT				"artifacts/goldensoulwait.wav"
#define PACK_SOUND_ALARM			"global/e_alarmb.wav"		// Played when the player first steals the flag.
#define PACK_SOUND_PICKUP			"global/a_hpick.wav"
#define PACK_SOUND_VICTORY			"global/bossdeath6.wav"
#define PACK_SOUND_RETURNED			"global/a_hpick.wav"

///////////////////////////////////////////////////////////////////////////////
//	cvars
///////////////////////////////////////////////////////////////////////////////
cvar_t	*deathtag;
//cvar_t	*dt_limit;
//cvar_t	*dt_timelimit;
static cvar_t	*dt_teamcolor1;
static cvar_t	*dt_teamcolor2;
static cvar_t	*fraglimit;
static cvar_t	*timelimit;

///////////////////////////////////////////////////////////////////////////////
//	local string defines
///////////////////////////////////////////////////////////////////////////////
flag_model_info_t dt_flag_model_info_l = 
{
	"models/global/dt_bpack.dkm",
	NULL,
	NULL,	
	NULL,	
	NULL,	
	NULL,	
	NULL,	
	NULL,
	NULL,	
	NULL,
	0,
	4,6,5	// iplayerClass - 1
};

//=============================================================================
// void DT_InitData(void)
// 
// sets up the team-specific data for the deathtag game
//=============================================================================
void DT_InitData()
{
	// set up the default color choices
	teamConvert[CTF_TEAM1] = 1;
	teamConvert[CTF_TEAM2] = 2;

	memset(&flagNames,0,sizeof(flagNames));
	Com_sprintf(flagNames[CTF_TEAM1],sizeof(flagNames[CTF_TEAM1]),tongue_deathtag[TONGUE_DT_TEAM_PACK],teamInfo[teamConvert[CTF_TEAM1]].teamName);
	Com_sprintf(flagNames[CTF_TEAM2],sizeof(flagNames[CTF_TEAM2]),tongue_deathtag[TONGUE_DT_TEAM_PACK],teamInfo[teamConvert[CTF_TEAM2]].teamName);

	flag_info.bUseSkins = false;
	flag_info.modelInfo[CTF_TEAM1] = flag_info.modelInfo[CTF_TEAM2] = &dt_flag_model_info_l;
}

//=============================================================================
// void DT_InitCVars(void)
// 
// Initialize deathtag cvars!
//=============================================================================
void DT_InitCVars(void)
{
	deathtag		= gstate->cvar("deathtag", "0", CVAR_SERVERINFO|CVAR_LATCH);
//	dt_limit		= gstate->cvar("dt_limit","0", CVAR_SERVERINFO|CVAR_LATCH|CVAR_ARCHIVE);
//	dt_teamcolor1	= gstate->cvar("dt_team_color_1","1", CVAR_SERVERINFO|CVAR_LATCH|CVAR_ARCHIVE|CVAR_NOSET);
//	dt_teamcolor2	= gstate->cvar("dt_team_color_2","2", CVAR_SERVERINFO|CVAR_LATCH|CVAR_ARCHIVE|CVAR_NOSET);
//	dt_timelimit	= gstate->cvar("dt_timelimit","0", CVAR_SERVERINFO|CVAR_LATCH|CVAR_ARCHIVE);
	fraglimit		= gstate->cvar("fraglimit","", 0);
	timelimit		= gstate->cvar("timelimit","", 0);
}

//=============================================================================
// void DT_FlagReset( userEntity_t *self )
// 
// deathtag specific flag resetting stuff
//=============================================================================
void DT_FlagReset( userEntity_t *self )
{
	self->touch		= item_pack_touch;
	self->think		= item_pack_think;
	self->nextthink = gstate->time + 0.2;
	self->hacks		= -1;
	self->delay		= -1;
	self->health	= -1;
}

//=============================================================================
// DT_FlagSpawn( userEntity_t *self, int team )
// 
// deathtag specific flag spawn stuff
//=============================================================================
void DT_FlagSpawn( userEntity_t *self, int team )
{
	ctfFlagHook_t *ihook = (ctfFlagHook_t *) self->userHook;
	if (!ihook)
		return;

	self->s.effects |= EF_ROTATE;

	ihook->soundAlarm		= gstate->SoundIndex( PACK_SOUND_ALARM );
	ihook->soundPickupIndex = gstate->SoundIndex( PACK_SOUND_PICKUP );
	ihook->soundVictory		= gstate->SoundIndex( PACK_SOUND_VICTORY );
	ihook->soundFlagReturned= gstate->SoundIndex( PACK_SOUND_RETURNED );
}

//=============================================================================
// void DT_PutClientInServer(userEntity_t *self)
// 
// puts a client in a deathtag game
//=============================================================================
userEntity_t *DT_PutClientInServer(userEntity_t *self)
{
	if (!deathtag->value)
		return NULL;

	TEAM_Verify(self,"dt_team",NULL);
	return TEAM_GetSpawnPoint(self);
}

//=============================================================================
// void DT_ClientDropFlag( userEntity_t *self, userEntity_t *flag )
// 
// let it be touched again!
//=============================================================================
void DT_ClientDropFlag( userEntity_t *self, userEntity_t *flag )
{
	flag->touch		= item_pack_touch;
}

//=============================================================================
// void DT_ClientDie( userEntity_t *self )
// 
// 
//=============================================================================
void DT_Client_Die (userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point)
{
	FLAG_Drop(self);
}

//=============================================================================
// void DT_ClientDisconnect( userEntity_t *self )
// 
// 
//=============================================================================
void DT_ClientDisconnect(userEntity_t *self)
{
	FLAG_Reset(FLAG_Carried_By(self));
	self->team = 0;
}

//=============================================================================
// int DT_GetScores(teamInfo_t *scorearr, int maxscores)
// 
// returns an array of score info for the game
//=============================================================================
int DT_GetScores(teamInfo_t *scorearr, int maxscores)
{
	if (!deathtag->value)
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
			if (ent->team == i)
				teamscore += ent->record.frags;
		}
		
		score->score = teamscore;
	}
	return cnt;
}

void CTFEndGameMessage(long message, int winner, int loser, int wscore, int lscore);
void DTEndGameMessage(long message, int winner, int loser, int wscore, int lscore)
{
	gstate->cs.BeginSendString();
	gstate->cs.SendSpecifierStr("%s%s",2);
	gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_deathtag,message));
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

//=============================================================================
// int DT_CheckRules()
// 
// checks the rules of the game and returns 1 if game is over, 0 otherwise.
//=============================================================================
int DT_CheckRules()
{
	if (!deathtag->value)
		return 0;

	// find the winning and losing teams!
	int winner(0),loser(0),wscore(0),lscore(0);

	teamInfo_t scorearr[2];
	int teams = DT_GetScores( scorearr, 2 );

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
			DTEndGameMessage(TONGUE_DT_SCORE_LIMIT,winner,loser,wscore,lscore);
			return 1;
		}
	}
	else
	{
		if (limit || timeup)
		{
			if (limit)	// cap limit...always a winner/loser
			{
				DTEndGameMessage(TONGUE_DT_SCORE_LIMIT,winner,loser,wscore,lscore);
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
// void DT_TeamStats(userEntity_t *self, int team, long &capPerc, long &teamPerc, long &flags)
// 
// fills out some info for placing in the score flaps on the HUD
//=============================================================================
void DT_TeamStats(userEntity_t *self, int team, long &capPerc, long &teamPerc, long &flags)
{
	flags = 0;

	// check if your team's pack is being carried
	userEntity_t *teamFlag = FLAG_Team(team);
	if (!teamFlag || (!FLAG_Carrier(team) && !(teamFlag->spawnflags & DROPPED_ITEM)))
		flags |= STAT_CTF_YOUR_FLAG_HOME;

	if (team == self->team)
	{
		flags |= STAT_CTF_YOUR_TEAM;
		// see if you're carrying a pack
		if (FLAG_Carried_By(self))
			flags |= STAT_CTF_YOU_HAVE_ENEMY_FLAG;
	}
	// send over the team conversion
	flags += ((teamConvert[team]) << STAT_CTF_TEAM_CONVERT_SHIFT);

	// send over the number of captures
	flags += ((ctfgame.score[team]) << STAT_CTF_TEAM_CAPTURES_SHIFT);

	teamPerc = 100;
	if (teamFlag && (teamFlag->hacks != -1))
	{
		if (self->team == teamFlag->team)
		{
			teamPerc = (long)((100*(teamFlag->hacks - gstate->time)) / PACK_BOOM_TIME);
		}
		else
		{
			teamPerc = 0;
		}
	}

	// how close are we to winning?
	if (fraglimit->value)
		capPerc = (long)( 100 * (ctfgame.score[team] / fraglimit->value));
	else
		capPerc = 0;
}

//========================================================================================================================
// pack entity functions
//========================================================================================================================
//=============================================================================
// void item_pack_return_think(userEntity_t *self)
// 
// puts the pack back where it belongs and we start all over again.
//=============================================================================
void item_pack_return_think(userEntity_t *self)
{
	ctfFlagHook_t *ihook = (ctfFlagHook_t *)self->userHook;
	if (ihook)
		gstate->StartEntitySound(self, CHAN_OVERRIDE, ihook->soundFlagReturned, 0.85f, 100000, 100000 );

	FLAG_RETURN_MESSAGE(tongue_ctf,T_CTF_FLAG_RETURNED,tongue_deathtag,TONGUE_DT_TEAM_PACK,self->team);
	FLAG_Reset(self);
}

//=============================================================================
// void item_pack_explode(userEntity_t *self)
// 
// pack explodes then gets set up for return
//=============================================================================
void item_pack_explode(userEntity_t *self)
{
	ctfFlagHook_t *ihook = (ctfFlagHook_t *)self->userHook;
	if (!ihook)
		return;

	// drop this pack if it's being carried
	if (ihook->pOwner)
		FLAG_Drop(ihook->pOwner);

	// spawn a cool looking explosion thinggy
	spawnPolyExplosion( self->s.origin, 1.4, 300, CVector(0.8,0.4,0.2), TE_POLYEXPLOSION_SND );
	spawn_sprite_explosion (self, self->s.origin, false);
	spawn_small_sprite_explosion(self, self->s.origin, false);

	// bust some people down
	int c;	
	CVector dir;
	userEntity_t *pent = &gstate->g_edicts[1];
	for (c = 0; c < maxclients->value; c++,pent++)
	{
		if (!pent || !pent->inuse)
			continue;

		dir = self->s.origin - pent->s.origin;
		if (dir.Length() < PACK_RANGE)
			com->Damage(pent,self,pent,pent->s.origin,dir, PACK_DAMAGE, DAMAGE_EXPLOSION);
	}

	// hide it until it respawns
	FLAG_Return(self);
	self->touch				= NULL;
	self->think				= item_pack_return_think;
	self->nextthink			= gstate->time + PACK_RETURN_TIME;
	self->solid				= SOLID_NOT;
	self->s.modelindex		= 0;
	gstate->LinkEntity(self);
}

//=============================================================================
// void item_pack_touch(userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
// 
// Lets players on the same team as the pack pick up the pack.  Sets the timers
// if necessary.  Players on the other team cause the pack to explode.
//=============================================================================
//tongue_deathtag[TONGUE_DT_TEAM_PACK]
void item_pack_touch(userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!com->ValidTouch (self, other))
		return;

	if (!(other->flags & FL_CLIENT))
		return;

	ctfFlagHook_t *ihook = (ctfFlagHook_t *)self->userHook;
	if (!ihook)
		return;

	if (other->team == self->team)					// other is touching their team's pack
	{
		gstate->StartEntitySound(other, CHAN_VOICE, ihook->soundPickupIndex, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX);
		if (!(self->spawnflags & DROPPED_ITEM))		// first time pick-up...set the timer
		{
			self->hacks = gstate->time + PACK_BOOM_TIME;
			self->delay = self->hacks - PACK_BEEP_DURATION;//gstate->time + PACK_BEEP_TIME;		// start ticking 10s before boom
			self->health = gstate->time;						// pickup time
			gstate->StartEntitySound(other, CHAN_OVERRIDE, ihook->soundAlarm, 0.85f, 1000.0f, 3000.0f);
		}

		// Let the other players know about it.
		// you grabbed the %s
		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_YOU_HAVE_GOT_FLAG),1);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_deathtag,TONGUE_DT_TEAM_PACK));
		gstate->cs.Continue(true);
		gstate->cs.SendCount(1);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,teamInfo[teamConvert[self->team]].teamNameID));
		gstate->cs.Unicast(other,CS_print_center,2.0);
		gstate->cs.EndSendString();

		//%s from your team has grabbed the %s
		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_GRABBED_FLAG),2);
		gstate->cs.SendString(other->netname);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_deathtag,TONGUE_DT_TEAM_PACK));
		gstate->cs.Continue(true);
		gstate->cs.SendCount(1);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,teamInfo[teamConvert[self->team]].teamNameID));
		FLAG_CS_Message(  other->team, other, 2.0f );
		gstate->cs.EndSendString();

		//%s has stolen your flag.
		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierID(RESOURCE_ID_FOR_STRING(tongue_deathtag,TONGUE_DT_O_TEAM_GRABBED),3);
		gstate->cs.SendString(other->netname);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,teamInfo[teamConvert[other->team]].teamNameID));
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_deathtag,TONGUE_DT_TEAM_PACK));
		gstate->cs.Continue(true);
		gstate->cs.SendCount(1);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,teamInfo[teamConvert[self->team]].teamNameID));
		FLAG_CS_Message( -other->team, other, 2.0f );
		gstate->cs.EndSendString();

		FLAG_Grab(self,other);
		self->touch = NULL;
		self->think = item_pack_think;
		ihook->pLastCarrier = other;
		return;
	}
	else											// other is touching the other team's pack
	{
		if (self->spawnflags & DROPPED_ITEM)		// this flag was dropped...kaboom!
		{
			item_pack_explode(self);
		}
		else										// harmlessly still in its area...just return
			return;
	}
}

//=============================================================================
// void item_pack_think(userEntity_t *self)
// 
// checks timers to see if the pack should explode or tick
//=============================================================================
void item_pack_think(userEntity_t *self)
{
	// move the pack along with its carrier
	userEntity_t *soundEnt = self;
	ctfFlagHook_t *ihook = (ctfFlagHook_t *)self->userHook;
	if (ihook && ihook->pOwner)
	{
		soundEnt = ihook->pOwner;
		self->s.origin = ihook->pOwner->s.origin;
		gstate->LinkEntity(self);
	}

	// should we make a tick?
	if ((self->delay != -1) && (gstate->time >= self->delay))
	{
		// play a tick
		gstate->StartEntitySound(soundEnt, CHAN_OVERRIDE, gstate->SoundIndex(PACK_SOUND_TICK), 1.0, 700.0f, 700.0f);

		if (soundEnt != self)
		{
			float dif = PACK_BEEP_DURATION;//(PACK_BOOM_TIME - PACK_BEEP_TIME);
			float vol = (dif - (self->hacks - gstate->time)) / dif;
			if (vol < 0) vol = 0;
			else if (vol > 1.0) vol = 1.0;

			if (vol < (1.0 / PACK_BEEP_DURATION))
			{
				gstate->cs.BeginSendString();
				gstate->cs.SendSpecifierID(RESOURCE_ID_FOR_STRING(tongue_deathtag,TONGUE_DT_TIME_ROUT),0);
				gstate->cs.Unicast(soundEnt,CS_print_center,2.0);
				gstate->cs.EndSendString();
			}
			gstate->StartEntitySound(soundEnt, CHAN_OVERRIDE, gstate->SoundIndex(PACK_SOUND_BEAT), vol, 700.0f, 700.0f);
		}
		self->delay = gstate->time + PACK_BEEP_INTERVAL;
	}

	if ((self->hacks != -1) && (gstate->time > self->hacks))
	{
		if (soundEnt != self)
		{
			gstate->cs.BeginSendString();
			gstate->cs.SendSpecifierID(RESOURCE_ID_FOR_STRING(tongue_deathtag,TONGUE_DT_TIME_OUT),0);
			gstate->cs.Unicast(soundEnt,CS_print_center,2.0);
			gstate->cs.EndSendString();
		}
		item_pack_explode(self);
		return;
	}

	self->think = item_pack_think;
	self->nextthink = gstate->time + 0.2;
}

//========================================================================================================================
// trigger_capture entity functions
//========================================================================================================================
//=============================================================================
// void DT_FlagCaptured(userEntity_t *flag, userEntity_t *self)
// 
// we have a capture.  Award points and set up the post-capture show.
//=============================================================================
void DT_FlagCaptured(userEntity_t *flag, userEntity_t *capturer, userEntity_t *trigger)
{
	if (!flag || !capturer || !trigger)
		return;

	CVector org = trigger->absmin;
	org += trigger->absmax;
	org.Multiply(0.5);

	FLAG_Drop(capturer, &org);
	flag->touch = NULL;		// no touching!!!  Let it blow up.

	// add in some clamping to the maximum wait until it blows up! PACK_MAX_WAIT_TIME PACK_BEEP_DURATION
	if ((flag->hacks - gstate->time) > PACK_MAX_WAIT_TIME)
	{
		flag->hacks = gstate->time + PACK_MAX_WAIT_TIME;
		flag->delay = flag->hacks - PACK_BEEP_DURATION;
	}

	// award points!
	ctfCaptureHook_t *hook = (ctfCaptureHook_t*)trigger->userHook;
	int points = 1;
	if (hook)
		points = hook->points;

	ctfFlagHook_t *ihook = (ctfFlagHook_t *)flag->userHook;
	if (ihook)
		gstate->StartEntitySound(capturer, CHAN_OVERRIDE, ihook->soundVictory, 1.0f, 100000.0f, 100000.0f);
	ctfgame.score[ capturer->team ] += points;

	// play the message
	buffer32 scoreString(tongue_ctf[T_CTF_SCORE],
		teamInfo[teamConvert[CTF_TEAM1]].teamName,
		ctfgame.score[CTF_TEAM1],
		teamInfo[teamConvert[CTF_TEAM2]].teamName,
		ctfgame.score[CTF_TEAM_BLUE]);

	buffer128 whoString;
	buffer128 pointString;
	if (points == 1)
	{
		// tell you what happened
		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierStr("%s%s%s",3);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_deathtag,TONGUE_DT_YOU_SCORED));
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_deathtag,TONGUE_DT_Y_POINT));
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_SCORE));
		gstate->cs.Continue(true);
		gstate->cs.SendCount(4);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[teamConvert[CTF_TEAM1]].teamNameID));
		gstate->cs.SendInteger(ctfgame.score[CTF_TEAM1]);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[teamConvert[CTF_TEAM2]].teamNameID));
		gstate->cs.SendInteger(ctfgame.score[CTF_TEAM2]);
		gstate->cs.Unicast(capturer,CS_print_center,2.0);
		gstate->cs.EndSendString();

		// tell your team what happened
		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierStr("%s%s%s",3);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_deathtag,TONGUE_DT_Y_TEAM_SCORED));
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_deathtag,TONGUE_DT_Y_POINT));
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_SCORE));
		gstate->cs.Continue(true);
		gstate->cs.SendCount(5);
		gstate->cs.SendString(capturer->netname);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[teamConvert[CTF_TEAM1]].teamNameID));
		gstate->cs.SendInteger(ctfgame.score[CTF_TEAM1]);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[teamConvert[CTF_TEAM2]].teamNameID));
		gstate->cs.SendInteger(ctfgame.score[CTF_TEAM2]);
		FLAG_CS_Message(capturer->team,capturer,2.0f);
		gstate->cs.EndSendString();

		// tell other team what happened
		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierStr("%s%s%s",3);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_deathtag,TONGUE_DT_O_TEAM_SCORED));
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_deathtag,TONGUE_DT_O_POINT));
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_SCORE));
		gstate->cs.Continue(true);
		gstate->cs.SendCount(7);
		gstate->cs.SendString(capturer->netname);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[capturer->team].teamNameID));
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[capturer->team].teamNameID));
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[teamConvert[CTF_TEAM1]].teamNameID));
		gstate->cs.SendInteger(ctfgame.score[CTF_TEAM1]);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[teamConvert[CTF_TEAM2]].teamNameID));
		gstate->cs.SendInteger(ctfgame.score[CTF_TEAM2]);
		FLAG_CS_Message(-capturer->team,capturer,2.0f);
		gstate->cs.EndSendString();
	}
	else
	{
		// tell you what happened
		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierStr("%s%s%s",3);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_deathtag,TONGUE_DT_YOU_SCORED));
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_deathtag,TONGUE_DT_Y_POINTS));
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_SCORE));
		gstate->cs.Continue(true);
		gstate->cs.SendCount(5);
		gstate->cs.SendInteger(points);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[teamConvert[CTF_TEAM1]].teamNameID));
		gstate->cs.SendInteger(ctfgame.score[CTF_TEAM1]);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[teamConvert[CTF_TEAM2]].teamNameID));
		gstate->cs.SendInteger(ctfgame.score[CTF_TEAM2]);
		gstate->cs.Unicast(capturer,CS_print_center,2.0);
		gstate->cs.EndSendString();

		// tell your team what happened
		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierStr("%s%s%s",3);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_deathtag,TONGUE_DT_Y_TEAM_SCORED));
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_deathtag,TONGUE_DT_Y_POINTS));
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_SCORE));
		gstate->cs.Continue(true);
		gstate->cs.SendCount(6);
		gstate->cs.SendString(capturer->netname);
		gstate->cs.SendInteger(points);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[teamConvert[CTF_TEAM1]].teamNameID));
		gstate->cs.SendInteger(ctfgame.score[CTF_TEAM1]);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[teamConvert[CTF_TEAM2]].teamNameID));
		gstate->cs.SendInteger(ctfgame.score[CTF_TEAM2]);
		FLAG_CS_Message(capturer->team,capturer,2.0f);
		gstate->cs.EndSendString();

		// tell other team what happened
		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierStr("%s%s%s",3);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_deathtag,TONGUE_DT_O_TEAM_SCORED));
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_deathtag,TONGUE_DT_O_POINTS));
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_SCORE));
		gstate->cs.Continue(true);
		gstate->cs.SendCount(8);
		gstate->cs.SendString(capturer->netname);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[capturer->team].teamNameID));
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[capturer->team].teamNameID));
		gstate->cs.SendInteger(points);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[teamConvert[CTF_TEAM1]].teamNameID));
		gstate->cs.SendInteger(ctfgame.score[CTF_TEAM1]);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, teamInfo[teamConvert[CTF_TEAM2]].teamNameID));
		gstate->cs.SendInteger(ctfgame.score[CTF_TEAM2]);
		FLAG_CS_Message(-capturer->team,capturer,2.0f);
		gstate->cs.EndSendString();
	}
}

//=============================================================================
// void	dt_trigger_capture_touch(userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
// 
// figures out if we have a capture!
//=============================================================================
void	dt_trigger_capture_touch(userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!com->ValidTouch (self, other))
		return;
	
	if (!(other->flags & FL_CLIENT))
		return;

	// see if the player has a flag!
	userEntity_t *teamflag = FLAG_Carried_By( other ); 
	if (!teamflag)
		return;

	// deathtag capture criteria (-1 means any team...)
	if ((self->team == -1) || (other->team == self->team))
	{
		DT_FlagCaptured(teamflag,other,self);
	}
}

//=============================================================================
// void DT_CaptureSpawn( userEntity_t *self, int team )
// 
// sets up the info for the trigger_capture
//=============================================================================
void DT_CaptureSpawn( userEntity_t *self, int team )
{
	if (!deathtag->value || ((team != -1) && ((team < CTF_TEAM1) || (team >= CTF_TEAMS))))
		return;

	self->touch			= dt_trigger_capture_touch;
	self->className		= "trigger_capture";
}

//DAMAGE_SQUISH|DAMAGE_TELEFRAG|
#define DAMAGE_PROTECT_FLAGS (DAMAGE_FALLING|DAMAGE_DROWN|DAMAGE_LAVA|DAMAGE_SLIME|DAMAGE_POISON|DAMAGE_COLD)
DllExport short dll_DT_CanDamage(userEntity_t *self, userEntity_t *attacker, unsigned long damage_flags)
{
	// wrong parms...keep on going with the damage
	if (!deathtag->value || !self || !attacker)
		return 1;

	// this only protects clients	------------- self is a client below...
	if (!(self->flags & FL_CLIENT))
		return 1;

	// no flag?  no protection...
	if (!FLAG_Carried_By(self))
		return 1;

	// attacker not a client?  let triggers and stuff hurt the player ---------- attacker is a client below
	if (!(attacker->flags & FL_CLIENT))
	{
		return 1;
	}
	else if (self == attacker)  // special case if self is attacker (drowning, falling, etc.)
	{
		if (damage_flags & DAMAGE_PROTECT_FLAGS)
			return 0;
	}

	// just return 1.  if self != attacker, we already know attacker is a client so we can return 1.
	// if self == attacker, we have un-protected damage...whoop up.
	return 1;
}
