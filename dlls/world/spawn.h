#ifndef _SPAWN_H
#define _SPAWN_H

void SPAWN_CallInitFunction( userEntity_t *self, const char *szAIClassName );
int SPAWN_FindNearSpawnPoint( userEntity_t *self, CVector &spawnPoint );
void SPAWN_AI( userEntity_t *self, const char *szAIClassName );
void SPAWN_AI( userEntity_t *self, const char *szClassName, const char *szUniqueId, const CVector &spawnPoint, 
			   const CVector &facingAngle, int bRespawnAfterDeath = FALSE, char *szScriptAction = NULL, char *szSkin = NULL );

void SPAWN_Respawn( userEntity_t *self );
void SPAWN_RespawnThink( userEntity_t *self );

void SPAWN_Remove( const char *szUniqueId );


#endif _SPAWN_H