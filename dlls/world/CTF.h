#ifndef _CTF_H
#define _CTF_H

typedef enum 
{
	CTF_TEAM_SELECT =	-1,
	CTF_TEAM_AUTO,
	CTF_TEAM1,
	CTF_TEAM2,

	CTF_TEAMS		// MAX TEAMS POSSIBLE.
} ctfTeam_t;

#define CTF_TEAM_RED	CTF_TEAM1
#define CTF_TEAM_BLUE	CTF_TEAM2

typedef struct ctfgame_s
{
	int		maxteams;
	int		score[CTF_TEAMS];	// Score for the team.
	int		total[ CTF_TEAMS];	// Set for the intermissions.

	userEntity_t *	flagReturns[ CTF_TEAMS ];

	float	last_flag_capture;
	int		last_capture_team;

	float	matchtime;	// time for match start/end (depends on state)
	int		lasttime;		// last time update
} ctfgame_t;

extern	ctfgame_t ctfgame;
extern  cvar_t	*ctf;

void CTFInitCVars(void);
void CTFInitData();
void CTFClientConnect( userEntity_t *self, char * userinfo );
void CTFClient_Die (userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point);
void CTFClientDisconnect(userEntity_t *self);
void CTFClient_Respawn(userEntity_t *self);
userEntity_t *CTFPutClientInServer(userEntity_t *self);
void CTFJoinTeam_f(userEntity_t *self);
void CTFFlagReset( userEntity_t *self );

userEntity_t *CTFCaptureSpawn( int team, const CVector &origin, const CVector &angles, const CVector &min, const CVector &max );
void CTFCaptureSpawn( userEntity_t *self, int team );
void CTFFlagSpawn( userEntity_t *self, int team );

void CTFGiveControlCard( userEntity_t *self );
void CTFClientDropFlag( userEntity_t *self, userEntity_t *flag );

bool CTFClientThink( userEntity_t *self );		// Returns true if we should do no more thinking.

bool CTFInitialize();

void CTFInitClient( userEntity_t *self );
void CTFLevelStart();
void CTFTeamStats(userEntity_t *self, int team, long &capPerc, long &teamPerc, long &flags);

int GetAllEntities( const char *name, userEntity_t **entarr, int max );

int CTFGetScores(teamInfo_t *scorearr, int maxscores);
int CTFCheckRules();


#endif // _CTF_H