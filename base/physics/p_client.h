#ifndef	_CLIENT_H
#define	_CLIENT_H


void	Client_SetCameraState (userEntity_t *ent, int state, int flags);
void	Client_SetClientAngles (userEntity_t *ent, CVector &angle);
void	Client_SetInputEntity (userEntity_t *clientEntity, userEntity_t *inputEntity);
void	Client_SetViewEntity (userEntity_t *clientEntity, userEntity_t *viewEntity);
void	Client_Think (edict_t *ent, usercmd_t *ucmd);
void	Client_BeginServerFrame (edict_t *ent);
void	Client_EndServerFrame (edict_t *ent);
void	Client_EndServerFrames (void);

#endif