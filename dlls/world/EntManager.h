#ifndef _ENTITYMANAGER_H_
#define _ENTITYMANAGER_H_

extern CTypedPtrArray<CPtrArray, userEntity_t *> entityManager;

__inline void ENTITYMANAGER_AddEntity( userEntity_t *pEntity )
{
    entityManager.Add( pEntity );
}

__inline userEntity_t *ENTITYMANAGER_GetEntity( int nIndex )
{
    return entityManager[nIndex];
}

__inline void ENTITYMANAGER_RemoveEntity( int nIndex )
{
    entityManager[nIndex] = NULL;
}

#endif _ENTITYMANAGER_H_