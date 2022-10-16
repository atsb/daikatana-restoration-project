#ifndef _CLIENT_H
#define _CLIENT_H

extern	int	cur_clients;

void	Client_NoClip		(userEntity_t *self);
void	Client_NoTarget		(userEntity_t *self);
void	Client_LlamaCheat	(userEntity_t *self);
void	Client_RampageCheat (userEntity_t *self);
void	Client_HealthCheat	(userEntity_t *self);
void	Client_ShowStats	(userEntity_t *self);
void	Client_Use			(userEntity_t *self);
void	path_to_rocket		(userEntity_t *self);

void	Client_PreThink		(userEntity_t *self);
void	Client_PostThink	(userEntity_t *self);

//void	Client_SetFOVRamp		(userEntity_t *self, float rate, float desired);
void	Client_SetForceAngles	(userEntity_t *self);
void	Client_ClearForceAngles	(userEntity_t *self);

void	Client_Die (userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, 
					int damage, CVector &point);
void	Client_Pain (userEntity_t *self, userEntity_t *other, float kick, int damage);
void	Client_Attack (userEntity_t *self);
//void	client_begin_stand (userEntity_t *self);
void	Client_FreeInventory (userEntity_t *self);

void	Client_InitClientModel ( userEntity_t *self );
//void	info_player_start (userEntity_t *self);
//void	info_player_deathmatch (userEntity_t *self);

userEntity_t *Client_GetSpawnPoint( char *name );
void CLIENT_IssueCommandToSidekicks( userEntity_t *self );

CVector &CLIENT_GetLastPlayerMovedPosition();

#endif