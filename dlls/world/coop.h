#ifndef _COOP_H
#define _COOP_H

bool CoopIsMainPlayer( userEntity_t *self );

userEntity_t *CoopFindMainPlayer();

userEntity_t *CoopFindPlayersBot( userEntity_t *self );

void CoopSelectCharacter( userEntity_t *self, char *userinfo );

void CoopTransferCharacter( userEntity_t *t, userEntity_t *f );

bool CoopIsOtherClose( userEntity_t *self, userEntity_t *other, float maxdist = 64.0f );

bool CoopIsAllOthersClose( userEntity_t *self, float maxdist, char **name1, char **name2 );

void CoopPlacePlayer( userEntity_t *self );

void CoopLevelStart();

void CoopFindPlayers(userEntity_t **hiro, userEntity_t **mikiko, userEntity_t **sfly, bool bAllowAI = false);
void COOP_LevelExit();
void COOP_ClientDisconnect(userEntity_t *self);
void COOP_AddInventoryItem(userEntity_t *self, userInventory_t *data);

userEntity_t *COOP_GetSpawnPoint(userEntity_t *self);
void COOP_SpawnHacks(userEntity_t *self);

#endif // _COOP_H