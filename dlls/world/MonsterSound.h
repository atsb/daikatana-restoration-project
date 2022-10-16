
#ifndef _MONSTERSOUNDS_H
#define _MONSTERSOUNDS_H

//-----------------------------------------------------------------------------

typedef struct _string
{
	char szString[64];
	_string *pNext;

} STRING, *STRING_PTR;

typedef struct _nameList
{
	int nNumStrings;
	STRING_PTR pStrings;

} STRINGLIST, *STRINGLIST_PTR;

STRINGLIST_PTR STRINGLIST_Construct();
STRINGLIST_PTR STRINGLIST_Destruct( STRINGLIST_PTR pStringList );
char *STRINGLIST_GetString( STRINGLIST_PTR pStringList, char *szString );
char *STRINGLIST_AddString( STRINGLIST_PTR pStringList, char *szString );
__inline int STRINGLIST_GetNumStrings( STRINGLIST_PTR pStringList )
{
	return pStringList->nNumStrings;
}

extern STRINGLIST_PTR pSoundList;

//-----------------------------------------------------------------------------

int FRAMEDATA_ReadFile( const char *szFileName, userEntity_t *self );
int FRAMEDATA_ViewthingReadFile( userEntity_t *self, const char *szFileName, frameData_t *aFrameData );
void FRAMEDATA_SetModelAnimSpeeds( userEntity_t *self );

#ifndef COLLECTION_H
#include "collect.h"
#endif COLLECTION_H

void SIGHTSOUND_FreeSounds();
void SIGHTSOUND_AddSound( int nType, char *szSound );
CPtrArray *SIGHTSOUND_GetArray( int nType );

#endif _MONSTERSOUNDS_H
