#ifndef _DEATHTAG_H
#define _DEATHTAG_H

extern  cvar_t	*deathtag;

void				DT_InitData();
void				DT_InitCVars(void);
					
void				DT_FlagReset( userEntity_t *self );
void				DT_FlagSpawn( userEntity_t *self, int team );

userEntity_t		*DT_PutClientInServer(userEntity_t *self);
void				DT_ClientDropFlag( userEntity_t *self, userEntity_t *flag );
void				DT_Client_Die (userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point);
void				DT_ClientDisconnect(userEntity_t *self);

int					DT_GetScores(teamInfo_t *scorearr, int maxscores);
int					DT_CheckRules();
void				DT_TeamStats(userEntity_t *self, int team, long &capPerc, long &teamPerc, long &flags);

void				DT_CaptureSpawn( userEntity_t *self, int team );

#endif