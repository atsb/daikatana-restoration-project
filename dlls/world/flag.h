#ifndef _FLAG_H
#define _FLAG_H

#include "ctf.h"
#include "deathtag.h"
// remember, team is 1-based.  so 3 gives two teams...
#define MAX_FLAGS					3
#define MAX_SKINS					9	
#define SPAWNFLAG_TEAM1	0X01
#define SPAWNFLAG_TEAM2 0x02

// hook structures
typedef	struct ctfFlagHook_s
{
	int				team;
	CVector			org_origin;
	CVector			org_angles;
	int				org_modelindex;

	int				soundAlarm;
	int				soundPickupIndex;
	int				soundVictory;
	int				soundFlagReturned;

	userEntity_t	*pOwner;
	userEntity_t	*pLastCarrier;
	userEntity_t	*trigger_capture;
} ctfFlagHook_t;

typedef struct ctfCaptureHook_s
{
	userEntity_t	*flag;
	bool			flagHere;
	int				points;
} ctfCaptureHook_t;

typedef struct ctfTeamInfo_s
{
	long		teamNameID;
	char		*teamName;
	char		*hiroSkin;
	char		*mikikoSkin;
	char		*superflySkin;
} ctfTeamInfo_t;

// contains model/skin info for each flag
typedef struct flag_model_info_s
{
	char							*model;
	char							*skins[MAX_SKINS];	
	int								numskins;
	int								frameInfo[3];
} flag_model_info_t;

// info about the flags used in the game
typedef struct flag_info_s
{
	bool							bUseSkins;
	flag_model_info_t				*modelInfo[CTF_TEAMS];	// pointer to completed flag_model_info_t structures
} flag_info_t;

extern int teamConvert[CTF_TEAMS];
extern char flagNames[CTF_TEAMS][32];
extern const ctfTeamInfo_s teamInfo[];
extern flag_info_t flag_info;
extern char *strTeamStart[CTF_TEAMS]; 



// helpful utility functions for flags!
#define FLAG_RETURN_MESSAGE(list1, index1, list2, index2, team)	gstate->cs.BeginSendString();				\
gstate->cs.SendSpecifierID(RESOURCE_ID_FOR_STRING(list1, index1), 1);										\
gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(list2, index2));												\
gstate->cs.Continue(true);																					\
gstate->cs.SendCount(1);																					\
gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_ctf,teamInfo[teamConvert[team]].teamNameID));			\
FLAG_CS_Message(0,NULL,2.0);																				\
gstate->cs.EndSendString();

void						FLAG_Init();

void						FLAG_Return(userEntity_t *self);
void						FLAG_Reset(userEntity_t *self);
void						FLAG_OwnerFix();
void						FLAG_SpawnSetup( userEntity_t *self, int team );
void						FLAG_Grab(userEntity_t *flag, userEntity_t *other, bool bShowOnPlayer = false);
userEntity_t				*FLAG_Drop(userEntity_t *self, CVector *origin_override = NULL);
int							FLAG_ModelIndex(userEntity_t *flag, userEntity_t *plr);
void						FLAG_Message( int team, userEntity_t *notthis, float time, const char *fmt, ... );
void						FLAG_CS_Message( int team, userEntity_t *notthis, float time );
void						FLAG_AwardPoints( int team, userEntity_t *skip, userEntity_t *InSightOf, int points );

userEntity_t				*FLAG_Carrier(int team);
userEntity_t				*FLAG_Carried_By(userEntity_t *self);
userEntity_t				*FLAG_Team(int team);
userEntity_t				*FLAG_ClosestCapture(userEntity_t *self);

userEntity_t				*FLAG_CaptureSpawn( unsigned long spawnflags, int points, const CVector &origin, 
											    const CVector &angles, const CVector &min, const CVector &max );

// team management functions
void						TEAM_GetPlayerCount( int& tc1, int& tc2 );
void						TEAM_Set(userEntity_t *self, int teamChoice, bool killIfChanged = true, char *userInfo = NULL);
void						TEAM_Verify(userEntity_t *self, char *cvar, char *userInfo);
void						TEAM_SetSkin(userEntity_t *self, char *userinfo);
userEntity_t				*TEAM_GetSpawnPoint( userEntity_t *self );
void						TEAM_SetOvertime( qboolean bSet );
qboolean					TEAM_Overtime( void );

#endif