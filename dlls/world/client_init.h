#ifndef	CLIENT_INIT_H
#define	CLIENT_INIT_H

void Client_FreeInventory (userEntity_t *self);
void Client_InitInventory (userEntity_t *self);
void Client_InitTimers (userEntity_t *self);
void Client_InitAttributes (userEntity_t *self);
void Client_FreeWeapon (userEntity_t *self);
void Client_InitWeapons (userEntity_t *self);
void Client_InitUserEntity (userEntity_t *self);
void Client_InitHookMisc (userEntity_t *self);
void Client_InitAnims (userEntity_t *self);
void Client_InitStats (userEntity_t *self);
void Client_InitCamera (userEntity_t *self);
void Client_InitNodeSystem (userEntity_t *self);
void Client_InitSidekicks( userEntity_t *self );
void Client_InitAIDefaults (userEntity_t *self);
void Client_FreeNodeSystem (userEntity_t *self);
void Client_InitGClient (userEntity_t *self, CVector &origin, CVector &angles);
void Client_InitDefaultWeapon(userEntity_t* self);
	
void Client_MoveToIntermission (userEntity_t *ent);
void Client_BeginIntermission( char *nextMap, qboolean bShowStats );
void Client_SavePersistentData (void);
void Client_RestorePersistentData (userEntity_t *self);

int Client_Resurrect (userEntity_t *self);

#endif