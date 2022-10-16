#ifndef _AIINFO_H_
#define _AIINFO_H_

void AIINFO_Init();
void AIINFO_SetPlayer( userEntity_t *pNewEntity );
userEntity_t *AIINFO_GetPlayer();
void AIINFO_AddSidekick( userEntity_t *pNewEntity );
void AIINFO_DeleteSidekick( userEntity_t *pEntity );
userEntity_t *AIINFO_GetFirstSidekick();
userEntity_t *AIINFO_GetSecondSidekick();
userEntity_t *AIINFO_GetSuperfly();
userEntity_t *AIINFO_GetMikiko();
userEntity_t *AIINFO_GetMikikofly();
int AIINFO_GetNumSidekicks();
void AIINFO_SetLastSelectedSidekick( userEntity_t *pEntity );
userEntity_t *AIINFO_GetLastSelectedSidekick();

#endif _AIINFO_H_
