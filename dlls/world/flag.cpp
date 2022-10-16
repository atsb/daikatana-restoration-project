#include "dk_system.h"

#ifndef WIN32
#include <dlfcn.h>
#endif

#include "world.h"
//#include "ai_info.h"// SCG[1/23/00]: not used
//#include "ai_utils.h"// SCG[1/23/00]: not used
//#include "ai_func.h"// SCG[1/23/00]: not used
//#include "Sidekick.h"// SCG[1/23/00]: not used
#include "dk_buffer.h"

#include "ctf.h"
#include "flag.h"

///////////////////////////////////////////////////////////////////////////////
//	client management stuff
///////////////////////////////////////////////////////////////////////////////
int	Client_Respawn (userEntity_t *self);
void Client_Die (userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point);
void Client_InitStats (userEntity_t *self);
void Client_InitAttributes (userEntity_t *self);
int calcStatLevel( userEntity_t *self );
extern long exp_level[];

///////////////////////////////////////////////////////////////////////////////
//	exports
///////////////////////////////////////////////////////////////////////////////
DllExport void item_flag_team1 (userEntity_t *self);
DllExport void item_flag_team2 (userEntity_t *self);

DllExport void trigger_capture_flag1 (userEntity_t *self);
DllExport void trigger_capture_flag2 (userEntity_t *self);

DllExport void trigger_capture (userEntity_t *self);
///////////////////////////////////////////////////////////////////////////////
//	local variables
///////////////////////////////////////////////////////////////////////////////
#define MAX_CAPS			8
static userEntity_t			*flags[CTF_TEAMS]; 
static userEntity_t			*caps[CTF_TEAMS][MAX_CAPS];

int flag_stand_frame = 3;
#define FLAG_MIN_BOX				CVector(-10, -10,-10)
#define FLAG_MAX_BOX				CVector( 10,   8, 10)

///////////////////////////////////////////////////////////////////////////////
//	extern variables
///////////////////////////////////////////////////////////////////////////////
// here to convert from team1/2 to the index into the teamInfo array above.
// if 1 is red and 2 is purple, we have 0,1,7.
int teamConvert[CTF_TEAMS] = 
{
	0,1,2
};

char flagNames[CTF_TEAMS][32];

const ctfTeamInfo_s teamInfo[] = 
{
	{-1,NULL,NULL,NULL,NULL},
	{T_CTF_RED_TEAM_NAME					,tongue_ctf[T_CTF_RED_TEAM_NAME]		,"skins/hiro_bod_8.wal"		,"skins/miko_bod_8.wal"		,"skins/sfly_bod_8.wal"},		
	{T_CTF_BLUE_TEAM_NAME					,tongue_ctf[T_CTF_BLUE_TEAM_NAME]		,"skins/hiro_bod_3.wal"		,"skins/miko_bod_3.wal"		,"skins/sfly_bod_3.wal"},		
	{T_CTF_CHROME_TEAM_NAME					,tongue_ctf[T_CTF_CHROME_TEAM_NAME]		,"skins/hiro_bod_4.wal"		,"skins/miko_bod_4.wal"		,"skins/sfly_bod_4.wal"},		
	{T_CTF_METAL_TEAM_NAME					,tongue_ctf[T_CTF_METAL_TEAM_NAME]		,"skins/hiro_bod_1.wal"		,"skins/miko_bod_1.wal"		,"skins/sfly_bod_1.wal"},	
	{T_CTF_GREEN_TEAM_NAME					,tongue_ctf[T_CTF_GREEN_TEAM_NAME]		,"skins/hiro_bod_2.wal"		,"skins/miko_bod_2.wal"		,"skins/sfly_bod_2.wal"},		
	{T_CTF_ORANGE_TEAM_NAME					,tongue_ctf[T_CTF_ORANGE_TEAM_NAME]		,"skins/hiro_bod_5.wal"		,"skins/miko_bod_5.wal"		,"skins/sfly_bod_5.wal"},	
	{T_CTF_PURPLE_TEAM_NAME					,tongue_ctf[T_CTF_PURPLE_TEAM_NAME]		,"skins/hiro_bod_6.wal"		,"skins/miko_bod_6.wal"		,"skins/sfly_bod_6.wal"},	
	{T_CTF_YELLOW_TEAM_NAME					,tongue_ctf[T_CTF_YELLOW_TEAM_NAME]		,"skins/hiro_bod_7.wal"		,"skins/miko_bod_7.wal"		,"skins/sfly_bod_7.wal"}		
};
#define TEAMSKINMAX 8

char *strTeamStart[CTF_TEAMS] = 
{
	"info_player_start",
	"info_player_team1",
	"info_player_team2"
};

flag_info_t flag_info;
qboolean	bIsOvertime;

///////////////////////////////////////////////////////////////////////////////
//	functions
///////////////////////////////////////////////////////////////////////////////
//---------------------------- helpful utility functions for flags! -----------------------------
void TEAM_Join_f(userEntity_t *self);
//=============================================================================
// void FLAG_Init()
// 
// sets up vars, data and commands
//=============================================================================
void FLAG_Init()
{	
	// init the cvars
	CTFInitCVars();
	DT_InitCVars();

	if (ctf->value)
		CTFInitData();
	else if (deathtag->value)
		DT_InitData();
	else if (dm_teamplay->value)
	{

	}
	else
		return;

	// set up any commands for the ctf game
	gstate->AddCommand("join",TEAM_Join_f);
	gstate->AddCommand(tongue_menu[T_MENU_JOIN_SERVER],TEAM_Join_f);

	memset(&flags[0],0,CTF_TEAMS * sizeof(flags[0]));
	// ctf structure.
	memset( &ctfgame,0,sizeof(ctfgame));
	memset( caps,0,sizeof(caps));

	// reset overtime
	TEAM_SetOvertime(FALSE);
}

//=============================================================================
// void FLAG_Grab(userEntity_t *flag, userEntity_t *other, bool bShowOnPlayer)
// 
// give a player the flag
//=============================================================================
void FLAG_Grab(userEntity_t *flag, userEntity_t *other, bool bShowOnPlayer)
{
	ctfFlagHook_t	*ihook = (ctfFlagHook_t *) flag->userHook;
	if (!ihook)
		return;

	if (ihook->pOwner && (ihook->pOwner != other))
		ihook->pOwner->s.modelindex4 = ihook->pOwner->s.modelindex3 = 0;

	if (other)
	{
		flag->think = NULL;
		FLAG_ModelIndex( flag, other );
		flag->solid = SOLID_NOT;
		flag->s.modelindex = 0;
		flag->spawnflags |= DROPPED_ITEM;
	}
	else
	{
		flag->solid			= SOLID_TRIGGER;
		flag->s.modelindex	= ihook->org_modelindex;
		flag->s.frame		= flag_stand_frame;
	}
	ihook->pOwner			= other;
	gstate->LinkEntity( flag );
}

//=============================================================================
// userEntity_t *FLAG_Drop(userEntity_t *self, CVector *origin_override)
// 
// places the flag in the world
//=============================================================================
userEntity_t *FLAG_Drop(userEntity_t *self, CVector *origin_override)
{
	userEntity_t *flag = FLAG_Carried_By(self);
	if (flag)
	{
		ctfFlagHook_t *ihook = (ctfFlagHook_t *) flag->userHook;
		FLAG_Grab(flag,NULL);
		flag->solid				= SOLID_TRIGGER;
		flag->s.origin			= origin_override ? *origin_override : self->s.origin;
		flag->s.angles.yaw		= self->s.angles.yaw;
		flag->s.angles.pitch	= 0.0f;		// No pitch.
		flag->s.angles.roll		= 0.0f;
		flag->s.modelindex		= ihook->org_modelindex;

		if (!origin_override)
		{
			flag->velocity.x		= rnd() * 400.0 - 200.0;
			flag->velocity.y		= rnd() * 400.0 - 200.0;
			flag->velocity.z		= rnd() * 250.0 + 250.0;
		}

		if (ctf->value)
			CTFClientDropFlag(self,flag);
		else if (deathtag->value)
			DT_ClientDropFlag(self,flag);

		gstate->LinkEntity( self );
	}

	return flag;
}

//=============================================================================
// int	FLAG_ModelIndex(userEntity_t *flag, userEntity_t *plr)
// 
// sets up the modelindex info for flag carrying
//=============================================================================
int	FLAG_ModelIndex(userEntity_t *flag, userEntity_t *plr)
{
	if (!flag || !plr || !plr->userHook)
		return 0;

	int frame = 0;
	ctfFlagHook_t	*ihook = (ctfFlagHook_t *) flag->userHook;
	playerHook_t *pHook;
	if (pHook = (playerHook_t *)plr->userHook)
		frame = flag_info.modelInfo[plr->team]->frameInfo[pHook->iPlayerClass - 1];

	plr->s.modelindex3 = flag->s.number;//flag->s.skinnum;
	plr->s.modelindex4 = ((frame << 13) + ihook->org_modelindex);
	// frame model#
	//   3    13
	return plr->s.modelindex4;
}

//=============================================================================
// userEntity_t *FLAG_ClosestCapture(userEntity_t *self)
// 
// returns the closest capture pad this entity's team can score at.
//=============================================================================
#define ABS_ORIGIN(ent)		((ent->absmin + ent->absmax) * 0.5)
userEntity_t *FLAG_ClosestCapture(userEntity_t *self)
{
	if ((self->team < CTF_TEAM1) || (self->team >= CTF_TEAMS))
		return NULL;

	CVector org = self->s.origin;
	float dist = 99999;
	int i = 0;
	userEntity_t *result = NULL;
	userEntity_t *cap = NULL;

	for (i = 0; i < MAX_CAPS; i++)
	{
		if (cap=caps[self->team][i])
		{
			float fdist = (org - ABS_ORIGIN(cap)).Length();
			if (!result || (fdist < dist))
			{
				result = cap;
				dist = fdist;
			}
		}
	}

	// either can capture at caps in index 0...check that too
	for (i = 0; i < MAX_CAPS; i++)
	{
		if (cap=caps[0][i])
		{
			float fdist = (org - ABS_ORIGIN(cap)).Length();
			if (!result || (fdist < dist))
			{
				result = cap;
				dist = fdist;
			}
		}
	}

	return result;
}

//=============================================================================
// userEntity_t *FLAG_Team(int team)
// 
// returns the specified team's flag
//=============================================================================
userEntity_t *FLAG_Team(int team)
{
	return ((team >= CTF_TEAM1) && (team < CTF_TEAMS)) ? flags[team] : NULL;
}

//=============================================================================
// userEntity_t *FLAG_Carrier(int team)
// 
// returns the entity carrying the specified team's flag
//=============================================================================
userEntity_t *FLAG_Carrier(int team)
{
	userEntity_t *flag = FLAG_Team(team);
	if (!flag)
		return NULL;


	ctfFlagHook_t *ihook = (ctfFlagHook_t *) flag->userHook;
	return (ihook) ? ihook->pOwner : NULL;
}

//=============================================================================
// userEntity_t *FLAG_Carried_By(userEntity_t *self)
// 
// returns the flag carried by the specified entity
//=============================================================================
userEntity_t *FLAG_Carried_By(userEntity_t *self)
{
	if (!self)
		return NULL;

	userEntity_t **ppent = &flags[CTF_TEAM1];
	int i;
	for(i=CTF_TEAM1;i<CTF_TEAMS;i++,ppent++)
	{
		if (!ppent || !(*ppent) || !(*ppent)->userHook)
			continue;

		if (self == ((ctfFlagHook_t *)((*ppent)->userHook))->pOwner)
			return *ppent;
	}
	return NULL;
}

//=============================================================================
// void FLAG_Message( int team, userEntity_t *notthis, float time, const char *fmt, ... )
// 
// centerprints a message to all players on 'team' or all players if team = 0
//=============================================================================
#define TEAM_CHECK(pl, team)	( (team == 0) || ((team > 0) && (team == pl->team)) || ((team < 0) && (-team != ent->team)) )
void FLAG_Message( int team, userEntity_t *notthis, float time, const char *fmt, ... )
{
	// Construct the message.
	char buf[512];
	va_list		argptr;
	va_start (argptr,fmt);
	vsprintf (buf, fmt,argptr);
	va_end (argptr);
	// Send the message to the people on the team.
	userEntity_t *ent;
	int c;
	for (c = 0; c < maxclients->value; c++)
	{
		ent = &gstate->g_edicts [c + 1];
		if (ent && (ent != notthis) && ent->inuse && TEAM_CHECK(ent,team))
		{
			if (gstate->gi->TestUnicast(ent,true))
			{
				gstate->centerprint( ent, time, buf );
			}
		}
	}
}

void FLAG_CS_Message( int team, userEntity_t *notthis, float time )
{
	// Send the message to the people on the team.
	userEntity_t *ent;
	int c;
	for (c = 0; c < maxclients->value; c++)
	{
		ent = &gstate->g_edicts [c + 1];
		if (ent && (ent != notthis) && ent->inuse && TEAM_CHECK(ent,team))
		{
			if (gstate->gi->TestUnicast(ent,true))
			{
				gstate->cs.Unicast(ent,CS_print_center,time);
			}
		}
	}
}

//=============================================================================
// void FLAG_AwardPoints( int team, userEntity_t *skip, userEntity_t *InSightOf, int points )
// 
// awards points to members of the selected team.  
// team = 0 : all players
// team > 0 : all players on (team)
// team < 0 : all players on (-team)
// skip		: do not award points to this entity
// InSightOf: if not NULL, points are awarded to an ent only if they can see this ent
//=============================================================================
void FLAG_AwardPoints( int team, userEntity_t *skip, userEntity_t *InSightOf, int points )
{
	userEntity_t *ent = NULL;
	for (int c = 0; c < maxclients->value; c++)
	{
		ent = &gstate->g_edicts [c + 1];
		if (ent && (ent != skip) && ent->inuse && TEAM_CHECK(ent,team))
		{
			if (InSightOf && !com->Visible(InSightOf,ent))
				continue;

			ent->record.frags += points;
		}
	}
}

//=============================================================================
// void FLAG_OwnerFix()
// 
// make sure only the owner of a flag is shown with it
//=============================================================================
void FLAG_OwnerFix()
{
	// Make sure only the owner has the flag.
	int c;
	int cnt = 0;
	userEntity_t *pent = &gstate->g_edicts[1];
	userEntity_t *flag;
	for (c = 0; c < maxclients->value; c++,pent++)
	{
		if (pent && pent->s.modelindex4)
		{
			flag = FLAG_Carried_By( pent );
			if (flag && (flag->userHook) && (pent == ((ctfFlagHook_t *)flag->userHook)->pOwner))
				FLAG_ModelIndex( flag, pent );
			else
				pent->s.modelindex4 = 0;
		}
	}
}

//=============================================================================
// void FLAG_Return(userEntity_t *self)
// 
// puts flag back to its spawn origin
//=============================================================================
void FLAG_Return(userEntity_t *self)
{
	ctfFlagHook_t *ihook		= (ctfFlagHook_t *)self->userHook;
	if (!ihook)
		return;

	self->s.origin		= ihook->org_origin;
}

//=============================================================================
// void FLAG_Reset(userEntity_t *self)
// 
// resets the flag to its condition when spawned
//=============================================================================
void FLAG_Reset(userEntity_t *self)
{
	if (!self)
		return;

	ctfFlagHook_t *ihook		= (ctfFlagHook_t *)self->userHook;
	if (!ihook)
		return;

	FLAG_Grab(self,NULL);
	FLAG_OwnerFix();

	self->spawnflags	&= ~(DROPPED_ITEM);
	self->solid			= SOLID_TRIGGER;
	self->movetype		= MOVETYPE_TOSS;
	self->s.renderfx	= RF_MINLIGHT;
	self->clipmask		= MASK_PLAYERSOLID;
	self->flags			= FL_ITEM;
	self->svflags		|= ( SVF_ITEM | SVF_NOPUSHCOLLIDE );	// amw: causes items in a cabinet to not collide with cabinet doors etc..

	self->s.effects		|= EF_RESPAWN;
	self->touch			= NULL;

	self->s.mins		= FLAG_MIN_BOX;
	self->s.maxs		= FLAG_MAX_BOX;

	self->s.origin		= ihook->org_origin;
	self->s.angles		= ihook->org_angles;
	self->s.modelindex	= ihook->org_modelindex;

	self->touch			= NULL;
	self->prethink		= NULL;
	self->think			= NULL;
	self->nextthink		= -1;

	self->postthink		= NULL;

	ihook->pOwner		= NULL;
	ihook->pLastCarrier = NULL;

	self->s.frame		= flag_stand_frame;

	if (ctf->value)
		CTFFlagReset(self);
	else if (deathtag->value)
		DT_FlagReset(self);

	gstate->LinkEntity( self );
}

DllExport int dll_FLAG_GetScores(teamInfo_t *scorearr, int maxscores)
{
	if (deathtag->value)
		return DT_GetScores(scorearr,maxscores);
	else if (ctf->value)
		return CTFGetScores(scorearr,maxscores);
	else
		return 0;
}

DllExport int dll_FLAG_CheckRules()
{
	if (deathtag->value)
	{
		return DT_CheckRules();
	}
	else if (ctf->value)
	{
		return CTFCheckRules();
	}
	else
		return 0;
}

//===========================================================================================================================================
// item_flag_* functions

//=============================================================================
// FLAG_SpawnSetup
//
// sets up the generic parameters shared among all uses of flags
//=============================================================================
void FLAG_SpawnSetup( userEntity_t *self, int team )
{
	if (!ctf->value && !deathtag->value)
		return;

	// look through the epairs to find the color for this flag
	int teamChoice = team;

	// check for some epairs
	if (self->epair)
	{
		char *key, *val;
		for (int i = 0; self->epair[i].key; i++)
		{
			key = self->epair[i].key;
			val = self->epair[i].value;

			if (!stricmp(key,"flagcolor"))
			{
				teamChoice = atoi(val);
			}
		}
	}

	if ((teamChoice < 1) || (teamChoice > TEAMSKINMAX))
		teamChoice = team;

	teamConvert[team]		= teamChoice;
	self->netname			= flagNames[team];
	self->userHook			= (ctfFlagHook_t*)gstate->X_Malloc(sizeof(ctfFlagHook_t),MEM_TAG_HOOK);
	ctfFlagHook_t *ihook	= (ctfFlagHook_t *)self->userHook;
							
	flags[team]				= self;
	self->team				= team;
	ihook->team				= team;
	ihook->pOwner			= NULL;

	// set up the model and skin
	self->modelName			= flag_info.modelInfo[team]->model;//strItemFlagModelName[ team ];
	self->s.modelindex		= gstate->ModelIndex (self->modelName);
//	self->s.render_scale.Set(0.5,0.5,0.5);
	if (flag_info.bUseSkins)
	{
		int skin = teamConvert[team];
		if ((skin > 0) && (skin < MAX_SKINS) && (skin <= flag_info.modelInfo[team]->numskins))
		{
			self->s.skinnum		= gstate->ImageIndex( flag_info.modelInfo[team]->skins[teamConvert[team]] );
			self->s.effects2	|= EF2_IMAGESKIN;
		}
	}

//	self->save				= flag_hook_save;
//	self->load				= flag_hook_load;

	ihook->org_origin		= self->s.origin;
	ihook->org_angles		= self->s.angles;
	ihook->org_modelindex	= self->s.modelindex;

	if (ctf->value)
		CTFFlagSpawn( self, team );
	else if (deathtag->value)
		DT_FlagSpawn(self,team);

	FLAG_Reset(self);
}

//=============================================================================
// item_flag_team#
//
// spawn functions for flags
//=============================================================================
void item_flag_team1( userEntity_t *self )
{
	FLAG_SpawnSetup(self,CTF_TEAM1);
}
void item_flag_team2( userEntity_t *self )
{
	FLAG_SpawnSetup(self,CTF_TEAM2);
}

//===========================================================================================================================================
// trigger_capture_flag# functions

//=============================================================================
// trigger_capture_flag#
// 
// generic spawn function for a trigger_capture
//=============================================================================
void trigger_capture_flag1 (userEntity_t *self)
{
	CVector org = self->absmin;
	org += self->absmax;
	org.Multiply(0.5);

	FLAG_CaptureSpawn(SPAWNFLAG_TEAM1,1,org,self->s.angles,CVector(-32,-32,-32),CVector(32,32,32));
}

void trigger_capture_flag2 (userEntity_t *self)
{
	CVector org = self->absmin;
	org += self->absmax;
	org.Multiply(0.5);

	FLAG_CaptureSpawn(SPAWNFLAG_TEAM2,1,org,self->s.angles,CVector(-32,-32,-32),CVector(32,32,32));
}

void trigger_capture (userEntity_t *self)
{
	if (!(ctf->value || deathtag->value))
		return;

	self->userHook	= (ctfCaptureHook_t*)gstate->X_Malloc(sizeof(ctfCaptureHook_t),MEM_TAG_HOOK);
	ctfCaptureHook_t *hook = (ctfCaptureHook_t*)self->userHook;

	// find out what teams this capture area is for
	bool isTeam1 = (self->spawnflags & SPAWNFLAG_TEAM1) ? true : false;
	bool isTeam2 = (self->spawnflags & SPAWNFLAG_TEAM2) ? true : false;

	if ((isTeam1 && isTeam2) || (!isTeam1 && !isTeam2))		// if both or neither, use either team!
		self->team = -1;
	else if (isTeam1)
		self->team = CTF_TEAM1;
	else
		self->team = CTF_TEAM2;

	int index = self->team;
	if (self->team == -1) index = 0;
	for (int j = 0; j < MAX_CAPS; j++)
	{
		if (!caps[index][j])
		{
			caps[index][j] = self;
			break;
		}
	}

	// check for some epairs
	if (self->epair)
	{
		char *key, *val;
		for (int i = 0; self->epair[i].key; i++)
		{
			key = self->epair[i].key;
			val = self->epair[i].value;

			if (!stricmp(key,"points"))
			{
				hook->points = atof(val);
			}
		}
	}
	if (!hook->points)
		hook->points = 1;

	self->solid			= SOLID_TRIGGER;
	self->movetype		= MOVETYPE_NONE;
	self->svflags		|= SVF_NOCLIENT;

	gstate->SetModel (self, self->modelName);

	// call game-type specific setup stuff
	if (ctf->value)
		CTFCaptureSpawn( self, self->team );
	else if (deathtag->value)
		DT_CaptureSpawn(self,self->team);

	gstate->LinkEntity (self);
}

userEntity_t *FLAG_CaptureSpawn( unsigned long spawnflags, int points, const CVector &origin, const CVector &angles, const CVector &min, const CVector &max )
{
	userEntity_t *self = gstate->SpawnEntity();
	self->className = "trigger_capture";
	self->spawnflags = spawnflags;

	trigger_capture(self);

	ctfCaptureHook_t *hook = (ctfCaptureHook_t*)self->userHook;
	if (hook)
		hook->points = points;

	self->s.origin	= origin;
	self->s.angles	= angles;
	self->s.mins	= min;
	self->s.maxs	= max;
	
	return self;
}


void flag_register_func()
{
//	gstate->RegisterFunc("flag_hook_save",flag_hook_save);
//	gstate->RegisterFunc("flag_hook_load",flag_hook_load);
}

//=======================================================================================================================================
// team management stuff
//=======================================================================================================================================
ctfgame_t ctfgame;		// The state of the capture the flag game.

//=============================================================================
// void TEAM_GetPlayerCount( int& tc1, int& tc2 )
//
// team counts are placed in tc1 and tc2
//=============================================================================
void TEAM_GetPlayerCount( int& tc1, int& tc2 )
{
	userEntity_t *player;
	int n;
	tc1 = 0;
	tc2 = 0;
	for (n = 1; n <= maxclients->value; n++)
	{
		player = &gstate->g_edicts[n];
		if (!player->inuse)
			continue;
		if (player->team == CTF_TEAM1)
			tc1++;
		else if (player->team == CTF_TEAM2)
			tc2++;
	}
}

//=============================================================================
// void TEAM_SetSkin(userEntity_t *self, char *userinfo)
//
// skins the entity
//=============================================================================
void TEAM_SetSkin(userEntity_t *self, char *userinfo)
{
	if (! (ctf->value || deathtag->value || dm_teamplay->value))
		return;

	int c = self->client->pers.body_info.character;
	char *skinname = NULL;
	if (ctf->value || deathtag->value)
	{
		if ((self->team < CTF_TEAM1) || (self->team >= CTF_TEAMS))
		{
//			TEAM_Set(self,-1,true,userinfo);
			return;
		}

		switch(c)
		{
		default:
		case 0: // HIRO
			skinname = teamInfo[teamConvert[self->team]].hiroSkin;
			break;
		case 1: // MIKIKO
			skinname = teamInfo[teamConvert[self->team]].mikikoSkin;
			break;
		case 2: // SUPERFLY
			skinname = teamInfo[teamConvert[self->team]].superflySkin;
			break;
		}
		if (!skinname)
			return;
	}
	else
	{
		if ((self->team < 1) || (self->team > TEAMSKINMAX))
		{
//			TEAM_Set(self,-1,true,userinfo);
			return;
		}

		switch(c)
		{
		default:
		case 0: // HIRO
			skinname = teamInfo[self->team].hiroSkin;
			break;
		case 1: // MIKIKO
			skinname = teamInfo[self->team].mikikoSkin;
			break;
		case 2: // SUPERFLY
			skinname = teamInfo[self->team].superflySkin;
			break;
		}
		if (!skinname)
			return;

	}

	char *m = self->client->pers.body_info.modelname;

	// copy the skinname to the body info...
    strncpy(self->client->pers.body_info.skinname, skinname,sizeof(self->client->pers.body_info.skinname)-1);

    int playernum = self - gstate->g_edicts - 1; // get player number
	gstate->ConfigString (CS_PLAYERSKINS + playernum, va("%s\\%s\\%s\\%d", self->netname, m, skinname, c) );
}

//=============================================================================
// void TEAM_Set(userEntity_t *self, int teamChoice, bool killIfChanged, char *userInfo)
//
// the passed entity is put on the given team
//=============================================================================
void TEAM_Set(userEntity_t *self, int teamChoice, bool killIfChanged, char *userInfo)
{
	playerHook_t *hook = (playerHook_t *)self->userHook;

	if (deathtag->value || ctf->value)
	{
		// see if we should auto-change the team
		if (teamChoice <= CTF_TEAM_AUTO)	// For now the SELECT is not implemented.
		{
			int t1,t2;
			TEAM_GetPlayerCount(t1,t2);
			// Choose a team based the number of players on each team.
			if (t1 < t2)
				teamChoice = CTF_TEAM1;
			else if (t1 > t2)
				teamChoice = CTF_TEAM2;
			else
			{
				// OK the teams are even.... Get Team scores and give the guy to the looozzer....
				if (ctfgame.score[ CTF_TEAM1 ] < ctfgame.score[ CTF_TEAM2 ])
					teamChoice = CTF_TEAM1;
				else if (ctfgame.score[ CTF_TEAM1 ] > ctfgame.score[ CTF_TEAM2 ])
					teamChoice = CTF_TEAM2;
				else
					teamChoice = (rand() % 2) + CTF_TEAM1;	// Give up and put them on a random team.
			}
		}
	}
	else
	{
		if ((teamChoice < 1) || (teamChoice > TEAMSKINMAX))
		{
			self->team = self->client->pers.nTeam = 0;
			teamChoice = 1;
		}
	}

	if (self->team != teamChoice)
	{
		self->team = self->client->pers.nTeam = teamChoice;
		TEAM_SetSkin( self, userInfo );
		if (self->netname && strlen(self->netname))
		{
			gstate->cs.BeginSendString();
			gstate->cs.SendSpecifierID(RESOURCE_ID_FOR_STRING(tongue_ctf,T_CTF_PLAYER_JOINED),2);
			gstate->cs.SendString(self->netname);

			if (ctf->value || deathtag->value)
			{
				gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,teamInfo[teamConvert[teamChoice]].teamNameID));
			}
			else
			{
				gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,teamInfo[teamChoice].teamNameID));
			}

			FLAG_CS_Message(0,NULL,2.0f);
			gstate->cs.EndSendString();
		}

		// kill off the player and reset stuff
		if (killIfChanged)
		{
			self->health = 0;
			com->Damage(self,self,self,zero_vector,zero_vector,10000,0);
//			Client_Respawn (self);
			Client_InitStats(self);

			Client_InitAttributes(self);
			client_persistant_s	*pers = &self->client->pers;
			if (hook)
			{
				pers->fBasePower	= hook->base_power;
				pers->fBaseAttack	= hook->base_attack;
				pers->fBaseSpeed	= hook->base_speed;
				pers->fBaseAcro		= hook->base_acro;
				pers->fBaseVita		= hook->base_vita;
			}

			memset(&self->record,0,sizeof(self->record));

			self->record.exp = exp_level[ calcStatLevel( self ) ];
		}
	}
}

//=============================================================================
// void TEAM_Verify(userEntity_t *self, char *userInfo)
//
// ensures that the entity's team info is correct
//=============================================================================
void TEAM_Verify(userEntity_t *self, char *cvar, char *userInfo)
{
	_ASSERTE(cvar);

	if (!userInfo)
		userInfo = self->client->pers.userinfo;

	char *val = Info_ValueForKey (userInfo, cvar);
	int	team = atoi(val);

	if (ctf->value || deathtag->value)
	{
		if ((self->team >= CTF_TEAM1) && (self->team < CTF_TEAMS))
			return;
	}
	else
	{
//		if ((self->team >= 1) && (self->team <= TEAMSKINMAX))
		if ((self->team == team) && (self->team >= 1) && (self->team <= TEAMSKINMAX))
			return;
	}

	if (deathtag->value || ctf->value)
	{
		if ((team < CTF_TEAM_SELECT) || (team >= CTF_TEAMS))
			team = CTF_TEAM_AUTO;
	}
	else
	{
		if ((team < 1) || (team > TEAMSKINMAX))
			team = 1;
	}

	TEAM_Set(self,team,false,userInfo);
}

//=============================================================================
// void TEAM_Join_f(userEntity_t *self)
//
// console command to get team info or change a player's team
//=============================================================================
void TEAM_Join_f(userEntity_t *self)
{
	if (!(ctf->value || deathtag->value))
		return;

	if (gstate->GetArgc() != 2)
	{
		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierID(RESOURCE_ID_FOR_STRING(tongue_ctf, T_CTF_ON_TEAM),2);
		gstate->cs.SendInteger(self->team);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,teamInfo[teamConvert[self->team]].teamNameID));
		gstate->cs.Unicast(self,CS_print_center,2.0);
		gstate->cs.EndSendString();
		return;
	}
	else
	{
		int team = atoi(gstate->GetArgv(1));
		if ((team < CTF_TEAM1) || (team >= CTF_TEAMS))
		{
			gstate->cs.BeginSendString();
			gstate->cs.SendSpecifierID(RESOURCE_ID_FOR_STRING(tongue_ctf, T_CTF_INVALID_TEAM),1);
			gstate->cs.SendInteger(self->team);
			gstate->cs.Unicast(self,CS_print_center,2.0);
			gstate->cs.EndSendString();
			return;
		}
		else if (team == self->team)
		{
			gstate->cs.BeginSendString();
			gstate->cs.SendSpecifierID(RESOURCE_ID_FOR_STRING(tongue_ctf, T_CTF_ALREADY_ON_TEAM),1);
			gstate->cs.SendInteger(team);
			gstate->cs.Unicast(self,CS_print_center,2.0);
			gstate->cs.EndSendString();
			return;
		}

		TEAM_Set(self,team,true);
	}
}

//=============================================================================
// userEntity_t *TEAM_GetSpawnPoint( userEntity_t *self )
// 
// 
//=============================================================================
userEntity_t *TEAM_GetSpawnPoint( userEntity_t *self )
{
	return com->DeathmatchSpawnPoint (strTeamStart[ self->team ]);
}

// some overtime functions.  make them visible just in case I decide to do anything crazy with them...
void TEAM_SetOvertime( qboolean bSet )
{
	if (bSet)
	{
		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierStr("%s%s%s",3);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, T_CTF_TIME_EXPIRED));
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, T_CTF_TIE));
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf, T_CTF_OVERTIME));
		FLAG_CS_Message(0, NULL, 2.0);
		gstate->cs.EndSendString();
	}
	bIsOvertime = bSet;
}

qboolean TEAM_Overtime( void )
{
	return bIsOvertime;
}
