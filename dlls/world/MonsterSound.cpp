// ==========================================================================
//
//  File:		MonsterSound.cpp
//  Contents:
//  Author:
//
// ==========================================================================

#include <algorithm>

#if _MSC_VER
#include <crtdbg.h>
#endif
#include "world.h"
#include "ai_common.h"
#include "ai_frames.h"
#include "ai_func.h"
//#include "SequenceMap.h"// SCG[1/23/00]: not used
#include "collect.h"
#include "filebuffer.h"
//#include "decode.h"// SCG[1/23/00]: not used
#include "dk_io.h"
#include "csv.h"
#include "ai_utils.h"
#include "MonsterSound.h"

/* ***************************** define types ****************************** */

using namespace std;

#define CSV_SIGHT_SKIP_ELEMENTS 8

class CSightSounds
{
public:
    CSightSounds();
    ~CSightSounds();

	void FreeSounds();
    void AddSound( int nType, char *szSound, float chance );
    CPtrArray *GetArray( int nType );
	float *GetChances( int nType );

private:
    CPtrArray aSightSounds[MAX_MONSTERS_IN_LIST];
	float *apfSightSoundChances[MAX_MONSTERS_IN_LIST];
};

/* ***************************** Local Variables *************************** */

static CSightSounds sightSounds;

//static int bUsingFile = TRUE;// SCG[1/23/00]: not used
//static CFileBuffer *pFileBuffer = NULL;// SCG[1/23/00]: not used
//static FILE *file = NULL;// SCG[1/23/00]: not used

/* **************************** Global Variables *************************** */

STRINGLIST_PTR pSoundList = NULL;

/* **************************** Global Functions *************************** */
/* ******************************* exports ********************************* */
/* ***************************** Local Functions *************************** */


CSightSounds::CSightSounds()
{
}

CSightSounds::~CSightSounds()
{
	FreeSounds();
}

void CSightSounds::FreeSounds()
{
    for ( int i = 0; i < MAX_MONSTERS_IN_LIST; i++ )
    {
        int nNumSounds = aSightSounds[i].GetSize();
        if ( nNumSounds > 0 )
        {
            for ( int j = 0; j < nNumSounds; j++ )
            {
                char *pSound = (char *)aSightSounds[i][j];
                if ( pSound )
                {
                    gstate->X_Free(pSound);
                }
            }

			gstate->X_Free(apfSightSoundChances[i]);
        
            aSightSounds[i].RemoveAll();
        }
    }
}

void CSightSounds::AddSound( int nType, char *szSound, float chance )
{
    _ASSERTE( nType >= 0 && nType < MAX_MONSTERS_IN_LIST );
    
	char *pSound = (char *)gstate->X_Malloc( strlen(szSound) + 1, MEM_TAG_MISC );
    if ( !pSound )
    {
        com->Error( "Mem allocation failed." );
    }

	int numcurrentsounds = aSightSounds[nType].GetSize();
	if ( (numcurrentsounds & 7) == 0 ) // check and reallocate on multiples of 8 (including 0)
	{
		// reallocate chances
		float *pNewChanceArray = (float *)gstate->X_Malloc( (numcurrentsounds + 8) * sizeof(float), MEM_TAG_MISC );
		float *pOldChanceArray = apfSightSoundChances[nType];

		if ( !pNewChanceArray )
		{
	        com->Error( "Mem allocation failed." );
		}

		// move the previous chances if necessary
		if ( numcurrentsounds )
		{
			memmove( pNewChanceArray, pOldChanceArray, numcurrentsounds * sizeof(float));
			gstate->X_Free(pOldChanceArray);
		}

		apfSightSoundChances[nType] = pNewChanceArray;
	}

	apfSightSoundChances[nType][numcurrentsounds] = chance;

    strcpy( pSound, szSound );
    aSightSounds[nType].Add( pSound );
}

CPtrArray *CSightSounds::GetArray( int nType )
{
    return &(aSightSounds[nType]);
}

float *CSightSounds::GetChances( int nType )
{
	return apfSightSoundChances[nType];
}

// ----------------------------------------------------------------------------
void SIGHTSOUND_FreeSounds()
{
	sightSounds.FreeSounds();
}

void SIGHTSOUND_AddSound( int nType, char *szSound, float chance )
{
    sightSounds.AddSound( nType, szSound, chance );
}

CPtrArray *SIGHTSOUND_GetArray( int nType )
{
    return sightSounds.GetArray( nType );
}

float *SIGHTSOUND_GetChances( int nType )
{
    return sightSounds.GetChances( nType );
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Name:		STRINGLIST_Construct
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
STRINGLIST_PTR STRINGLIST_Construct()
{
//	STRINGLIST_PTR pStringList = new STRINGLIST;
	STRINGLIST_PTR pStringList = (STRINGLIST_PTR)gstate->X_Malloc(sizeof(STRINGLIST),MEM_TAG_MISC);
    if ( !pStringList )
    {
        com->Error( "Mem allocation failed." );
    }
	pStringList->nNumStrings = 0;
	pStringList->pStrings = NULL;

	return pStringList;
}

// ----------------------------------------------------------------------------
//
// Name:		STRINGLIST_Destruct
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
STRINGLIST_PTR STRINGLIST_Destruct( STRINGLIST_PTR pStringList )
{
	if ( pStringList )
	{
		STRING_PTR pStrings = pStringList->pStrings;
		while ( pStrings != NULL )
		{
			STRING_PTR pTemp = pStrings;
			pStrings = pStrings->pNext;

			//delete pTemp;
			gstate->X_Free(pTemp);
		}

		//delete pStringList;
		gstate->X_Free(pStringList);
	}

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		STRINGLIST_GetString
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
char *STRINGLIST_GetString( STRINGLIST_PTR pStringList, char *szString )
{
	STRING_PTR pStrings = pStringList->pStrings;
	while ( pStrings != NULL )
	{
		if ( _stricmp( pStrings->szString, szString ) == 0 )
		{
			return pStrings->szString;
		}

		pStrings = pStrings->pNext;
	}

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		STRINGLIST_AddString
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
char *STRINGLIST_AddString( STRINGLIST_PTR pStringList, char *szString )
{
//	STRING_PTR pString = new STRING;
	STRING_PTR pString = (STRING_PTR)gstate->X_Malloc(sizeof(STRING),MEM_TAG_MISC);
	strcpy( pString->szString, szString );
	pString->pNext = NULL;

	if ( pStringList->pStrings )
	{
		STRING_PTR pStrings = pStringList->pStrings;
		STRING_PTR pPrevString = pStrings;
		while ( pStrings != NULL )
		{
			pPrevString = pStrings;
			pStrings = pStrings->pNext;
		}
		
		pPrevString->pNext = pString;
	}
	else
	{
		pStringList->pStrings = pString;
	}

	pStringList->nNumStrings++;

	return pString->szString;
}



// ****************************************************************************
//								FRAMEDATA functions
// ****************************************************************************


// ----------------------------------------------------------------------------
//
// Name:		FRAMEDATA_Init
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void FRAMEDATA_Init( frameData_t *pFrameData )
{
	pFrameData->sound1			= NULL;
	pFrameData->soundframe1		= pFrameData->first + 1;
	pFrameData->sound2			= NULL;
	pFrameData->soundframe2		= -1;

	pFrameData->flags			= FRAME_ONCE;

	pFrameData->nAttackFrame1	= pFrameData->first + 1;
	pFrameData->nAttackFrame2	= -1;
	pFrameData->fModelAnimSpeed = 0.0f;
	pFrameData->fAnimChance     = 1.0f;
	pFrameData->fSound2Chance   = 0.0f;
}

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

__inline char *FRAMEDATA_GetSoundName1( frameData_t *pFrameData )
{
	return pFrameData->sound1;
}
__inline void FRAMEDATA_SetSoundName1( frameData_t *pFrameData, char *szSoundName )
{
	pFrameData->sound1 = szSoundName;
}
__inline int FRAMEDATA_GetSoundFrame1( frameData_t *pFrameData )
{
	return pFrameData->soundframe1;
}
__inline void FRAMEDATA_SetSoundFrame1( frameData_t *pFrameData, int nIndex )
{
	if ( nIndex >= 0 )
	{
		pFrameData->soundframe1 = pFrameData->first + nIndex;
	}
}
__inline char *FRAMEDATA_GetSoundName2( frameData_t *pFrameData )
{
	return pFrameData->sound2;
}
__inline void FRAMEDATA_SetSoundName2( frameData_t *pFrameData, char *szSoundName )
{
	pFrameData->sound2 = szSoundName;
}
__inline int FRAMEDATA_GetSoundFrame2( frameData_t *pFrameData )
{
	return pFrameData->soundframe2;
}
__inline void FRAMEDATA_SetSoundFrame2( frameData_t *pFrameData, int nIndex )
{
	if ( nIndex >= 0 )
	{
		pFrameData->soundframe2 = pFrameData->first + nIndex;
	}
}
__inline unsigned long FRAMEDATA_GetFrameDataFlag( frameData_t *pFrameData )
{
	return pFrameData->flags;
}
__inline void FRAMEDATA_SetFrameDataFlag( frameData_t *pFrameData, unsigned long nFlags )
{
	pFrameData->flags = nFlags;
}

__inline int FRAMEDATA_GetAttackFrame1( frameData_t *pFrameData )
{
	return pFrameData->nAttackFrame1;
}
__inline void FRAMEDATA_SetAttackFrame1( frameData_t *pFrameData, int nIndex )
{
	if ( nIndex >= 0 )
	{
		pFrameData->nAttackFrame1 = pFrameData->first + nIndex;
	}
}

__inline int FRAMEDATA_GetAttackFrame2( frameData_t *pFrameData )
{
	return pFrameData->nAttackFrame2;
}
__inline void FRAMEDATA_SetAttackFrame2( frameData_t *pFrameData, int nIndex )
{
	if ( nIndex >= 0 )
	{
		pFrameData->nAttackFrame2 = pFrameData->first + nIndex;
	}
}

__inline float FRAMEDATA_GetAnimChance( frameData_t *pFrameData )
{
	return pFrameData->fAnimChance;
}
__inline void FRAMEDATA_SetAnimChance( frameData_t *pFrameData, float fChance )
{
	pFrameData->fAnimChance = fChance;
}

__inline float FRAMEDATA_GetSound2Chance( frameData_t *pFrameData )
{
	return pFrameData->fSound2Chance;
}
__inline void FRAMEDATA_SetSound2Chance( frameData_t *pFrameData, float fChance )
{
	pFrameData->fSound2Chance = fChance;
}


// ----------------------------------------------------------------------------
//
// Name:		FRAMEDATA_GetFlag
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static int FRAMEDATA_GetFlag( char *szFlag )
{
	if ( _stricmp( "FRAME_LOOP", szFlag ) == 0 )
	{
		return FRAME_LOOP;
	}
	else
	if ( _stricmp( "FRAME_ONCE", szFlag ) == 0 )
	{
		return FRAME_ONCE;
	}

	return -1;
}

// ----------------------------------------------------------------------------
//
// Name:		FRAMEDATA_ParseData
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define FRAMEDATA_SOUND1			0
#define FRAMEDATA_FRAME1			1
#define FRAMEDATA_SOUND2			2
#define FRAMEDATA_FRAME2			3
#define FRAMEDATA_FLAG				4
#define FRAMEDATA_ATTACKFRAME1		5
#define FRAMEDATA_ATTACKFRAME2		6
#define FRAMEDATA_UNPARSEDANIMSPEED 7
#define FRAMEDATA_ANIMPERCENT       8
#define FRAMEDATA_SOUND2PERCENT     9

static void FRAMEDATA_ParseData( userEntity_t *self, frameData_t *pFrameData, char *szData, int nCurrentDataIndex )
{
	_ASSERTE( self );
	_ASSERTE( pFrameData );

	char szString[128];
	int nValue = -1;
	float fValue = 0.0f;

	switch ( nCurrentDataIndex )
	{
		case FRAMEDATA_SOUND1:
		{
			int nRetValue = sscanf( szData, "%s", szString );
			if ( nRetValue <= 0 )
			{
				return;
			}

			char *szSoundName = STRINGLIST_GetString( pSoundList, szString );
			if ( !szSoundName )
			{
				szSoundName = STRINGLIST_AddString( pSoundList, szString );	
			}
			FRAMEDATA_SetSoundName1( pFrameData, szSoundName );
			break;
		}
		case FRAMEDATA_FRAME1:
		{
			int nRetValue = sscanf( szData, "%d", &nValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			FRAMEDATA_SetSoundFrame1( pFrameData, nValue );
			break;
		}
		case FRAMEDATA_SOUND2:
		{
			int nRetValue = sscanf( szData, "%s", szString );
			if ( nRetValue <= 0 )
			{
				return;
			}

			char *szSoundName = STRINGLIST_GetString( pSoundList, szString );
			if ( !szSoundName )
			{
				szSoundName = STRINGLIST_AddString( pSoundList, szString );	
			}
			FRAMEDATA_SetSoundName2( pFrameData, szSoundName );
			break;
		}
		case FRAMEDATA_FRAME2:
		{
			int nRetValue = sscanf( szData, "%d", &nValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			FRAMEDATA_SetSoundFrame2( pFrameData, nValue );
			break;
		}
		case FRAMEDATA_FLAG:
		{
			int nRetValue = sscanf( szData, "%s", szString );
			if ( nRetValue <= 0 )
			{
				return;
			}

			int nFlag = FRAMEDATA_GetFlag( szString );
			FRAMEDATA_SetFrameDataFlag( pFrameData, nFlag );
			break;
		}

		case FRAMEDATA_ATTACKFRAME1:
		{
			int nRetValue = sscanf( szData, "%d", &nValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			FRAMEDATA_SetAttackFrame1( pFrameData, nValue );
			break;
		}
		case FRAMEDATA_ATTACKFRAME2:
		{
			int nRetValue = sscanf( szData, "%d", &nValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			FRAMEDATA_SetAttackFrame2( pFrameData, nValue );
			break;
		}

		case FRAMEDATA_UNPARSEDANIMSPEED:
			// we don't actually set anim speed
		break;

		case FRAMEDATA_ANIMPERCENT:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			FRAMEDATA_SetAnimChance(pFrameData, min(1.0f, max(0.0f, fValue * 0.01f)));
		}
		break;

		case FRAMEDATA_SOUND2PERCENT:
		{
			int nRetValue = sscanf( szData, "%f", &fValue );
			if ( nRetValue <= 0 )
			{
				return;
			}

			// erase soundframe2, since only sound1 is really being used
			pFrameData->soundframe2 = -1;

			FRAMEDATA_SetSound2Chance(pFrameData, min(1.0f, max(0.0f, fValue * 0.01f)));
		}
		break;

		default:
		break;
	}

}

// ----------------------------------------------------------------------------
//
// Name:		FRAMEDATA_InitSounds
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void FRAMEDATA_InitSounds( userEntity_t *self )
{
	_ASSERTE( self );

	char *key;
	void *value;
	POSITION pos = self->pMapAnimationToSequence->GetStartPosition();
	while ( pos )
	{
		self->pMapAnimationToSequence->GetNextAssoc( pos, key, value );

		_ASSERTE( strlen( key ) > 0 );
		frameData_t *pFrameData = (frameData_t*)value;
        if ( pFrameData )
        {
		    FRAMEDATA_SetSoundName1( pFrameData, NULL );
		    FRAMEDATA_SetSoundName2( pFrameData, NULL );
		    FRAMEDATA_SetSoundFrame1( pFrameData, 1 );
		    FRAMEDATA_SetFrameDataFlag( pFrameData, FRAME_ONCE );
		    FRAMEDATA_SetAttackFrame1( pFrameData, 1 );
		    FRAMEDATA_SetAttackFrame2( pFrameData, -1 );
			FRAMEDATA_SetAnimChance( pFrameData, 1.0f );
			FRAMEDATA_SetSound2Chance( pFrameData, 0.0f );
	    }
    }
}

// ----------------------------------------------------------------------------
//
// Name:		FRAMEDATA_ReadFile
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int FRAMEDATA_ReadFile( const char *szFileName, userEntity_t *self )
{
	_ASSERTE( self );

	_ASSERTE( self->pMapAnimationToSequence->GetHashTableSize() > 0 );

    if ( !szFileName || strlen( szFileName ) == 0 )
    {
        return FALSE;
    }

    FRAMEDATA_InitSounds( self );

    int bSuccess = TRUE;

	CCSVFile *pCsvFile = NULL;
	int nRetCode = CSV_OpenFile( szFileName, &pCsvFile );
	if ( nRetCode == DKIO_ERROR_NONE )
	{
		int nNumSequences = 0;
		
		char szLine[2048];
		char szElement[64];
		// skip first line
		CSV_GetNextLine( pCsvFile, szLine );
		while ( CSV_GetNextLine( pCsvFile, szLine ) != EOF )
		{
			int nRetValue = CSV_GetFirstElement( pCsvFile, szLine, szElement );
			if ( nRetValue > 0 )
			{
				frameData_t *pFrameData = FRAMES_GetSequence( self, szElement );
				if ( pFrameData )
				{
					FRAMEDATA_Init( pFrameData );

					int nNumElements = 0;
					while ( (nRetValue = CSV_GetNextElement( pCsvFile, szLine, szElement )) != EOLN )
					{
						FRAMEDATA_ParseData( self, pFrameData, szElement, nNumElements );
						nNumElements++;	
					}
			
                    _ASSERTE( strlen(pFrameData->animation_name) > 0 );
					nNumSequences++;
				}
                else
                {
                    // do sight sounds
                    if ( strstr( szElement, "sight" ) )
                    {
                        if ( (nRetValue = CSV_GetNextElement( pCsvFile, szLine, szElement )) != EOLN )
                        {
                            if ( strlen( szElement ) > 0 )
                            {
                                playerHook_t *hook = AI_GetPlayerHook( self );
								char szChanceElement[64]; // there's always the element of chance
								float chance = 1.0f;
								int i;

								for ( i=0; i < CSV_SIGHT_SKIP_ELEMENTS; i++ )
								{
									if ( CSV_GetNextElement( pCsvFile, szLine, szChanceElement ) == EOLN )
									{
										break;
									}
								}

								// Finished CSV scan?
								if ( i == CSV_SIGHT_SKIP_ELEMENTS )
								{
									// you must defeat sheng long to stand a chance
									if ( !sscanf(szChanceElement, "%f", &chance) )
									{
										chance = 1.0f;
									}
								}

                                SIGHTSOUND_AddSound( hook->type, szElement, chance * 0.01f );
                            }
                        }
                    }
                }
			}
		}

		CSV_CloseFile( pCsvFile );
	}
	else
	{
		bSuccess = FALSE;
	}		

	return bSuccess;
}

// ----------------------------------------------------------------------------
//
// Name:		FRAMEDATA_ReadFile
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int FRAMEDATA_ViewthingReadFile( userEntity_t *self, const char *szFileName, frameData_t *aFrameData )
{
	_ASSERTE( self );

    int bSuccess = TRUE;

	CCSVFile *pCsvFile = NULL;
	int nRetCode = CSV_OpenFile( szFileName, &pCsvFile );
	if ( nRetCode == DKIO_ERROR_NONE )
	{
		char szLine[2048];
		char szElement[64];

		int i = 0;
		while (1)
		{
			frameData_t *pFrameData = &aFrameData[i];
			if ( !pFrameData || pFrameData->animation_name[0] == 0x00)
			{
				break;
			}

			// start from the start of buffer
			CSV_SeekToBeginning( pCsvFile );
			
			// skip first line
			CSV_GetNextLine( pCsvFile, szLine );
			while ( CSV_GetNextLine( pCsvFile, szLine ) != EOF )
			{
				int nRetValue = CSV_GetFirstElement( pCsvFile, szLine, szElement );
				if ( nRetValue > 0 && stricmp( szElement, pFrameData->animation_name ) == 0 )
				{
					FRAMEDATA_Init( pFrameData );

					int nNumElements = 0;
					while ( (nRetValue = CSV_GetNextElement( pCsvFile, szLine, szElement )) != EOLN )
					{
						FRAMEDATA_ParseData( self, pFrameData, szElement, nNumElements );
						nNumElements++;
						
						//if ( pFrameData->sound1 && strlen( pFrameData->sound1 ) > 0 )
						//{
						//	gstate->SoundIndex(pFrameData->sound1);
						//}
						//if ( pFrameData->sound2 && strlen( pFrameData->sound2 ) > 0 )
						//{
						//	gstate->SoundIndex(pFrameData->sound2);
						//}
						
					}
					
                    break;
				}
			}

			i++;
		}

		CSV_CloseFile( pCsvFile );
	}
	else
	{
		bSuccess = FALSE;
	}		

	return bSuccess;
}

// ----------------------------------------------------------------------------
//
// Name:		FRAMEDATA_SetModelAnimSpeeds
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void FRAMEDATA_SetModelAnimSpeeds( userEntity_t *self )
{
    _ASSERTE( self );

	// fill in the model anim speed
    playerHook_t *hook = AI_GetPlayerHook( self );

	char *key;
	void *value;
	POSITION pos = self->pMapAnimationToSequence->GetStartPosition();
	while ( pos )
	{
		self->pMapAnimationToSequence->GetNextAssoc( pos, key, value );

		_ASSERTE( strlen( key ) > 0 );
		frameData_t *pFrameData = (frameData_t*)value;
        if ( pFrameData )
        {
            if ( strstr( pFrameData->animation_name, "walk" ) )
            {
                pFrameData->fModelAnimSpeed = hook->walk_speed;
            }
            else
            if ( strstr( pFrameData->animation_name, "run" ) )
            {
                pFrameData->fModelAnimSpeed = hook->run_speed;
            }
            else
            {
                pFrameData->fModelAnimSpeed = 0.0f;
            }
	    }
    }
}

// ----------------------------------------------------------------------------
//							Sounds for AI
// ----------------------------------------------------------------------------

int bDisableAISound = FALSE;

// ----------------------------------------------------------------------------
//
// Name:		ai_pain_sound
// Description:
// Input:
// Output:
// Note:
//	plays the correct pain sound for a dying creature
//
// ----------------------------------------------------------------------------
void ai_pain_sound( userEntity_t *self )
{
	_ASSERTE( self );
	if ( bDisableAISound )
	{
		return;
	}

	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );
	userEntity_t *pClient = AI_CanClientHear( self );
	if ( pClient )
	{
		frameData_t *pSequence = FRAMES_GetSequence( self, "pain" );
		if ( pSequence )
		{
			gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex(pSequence->sound1), 
                                     1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		ai_die_sound
// Description:
// Input:
// Output:
// Note:
//	plays the correct death sound for a dying creature
//
// ----------------------------------------------------------------------------
void ai_die_sound( userEntity_t *self )
{
	_ASSERTE( self );
	if ( bDisableAISound )
	{
		return;
	}

	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	userEntity_t *pClient = AI_CanClientHear( self );
	if ( pClient )
	{
		frameData_t *pSequence = FRAMES_GetSequence( self, "diea" );
		if ( pSequence )
		{
			gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex(pSequence->sound1), 
                                     1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		ai_sight_sound
// Description:
// Input:
// Output:
// Note:
//	plays an appropriate sight sound based on actor type
//
// ----------------------------------------------------------------------------
void ai_sight_sound( userEntity_t *self )
{
	_ASSERTE( self );
	if ( bDisableAISound || self->deadflag != DEAD_NO)
	{
		return;
	}

	userEntity_t *pClient = AI_CanClientHear( self );
	if ( pClient )
	{
    	playerHook_t *hook = AI_GetPlayerHook( self );

    	CPtrArray *pSoundArray = SIGHTSOUND_GetArray( hook->type );
		float *pChances = SIGHTSOUND_GetChances( hook->type );
        int nNumSightSounds = pSoundArray->GetSize();
		if ( nNumSightSounds > 0 )
		{
			float chanceleft = 1.0f;
			int i = 0;

			while ( rnd() > pChances[i] / chanceleft )
			{
				chanceleft -= pChances[i];

				// cap chance remaining to 0.1%
				if ( chanceleft < 0.0001f )
					chanceleft = 0.0001f;

				if ( i + 1 < nNumSightSounds )
				{
					i++;
				}
				else
				{
					break;
				}
			}

            char *pSound = (char *)(*pSoundArray)[i];
            _ASSERTE( pSound );

			gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex(pSound), 
                                      1.0f, hook->fMinAttenuation, hook->fMaxAttenuation );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		ai_wander_sound
// Description:
// Input:
// Output:
// Note:
//	plays an appropriate wandering sound based on actor type
//
// ----------------------------------------------------------------------------
void ai_wander_sound( userEntity_t *self )
{
	_ASSERTE( self );

	if ( bDisableAISound )
	{
		return;
	}

	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );
	if ( hook->sound_time >= gstate->time )
	{
		return;
	}

	userEntity_t *pClient = AI_CanClientHear( self );
	if ( pClient )
	{
		frameData_t *pSequence = FRAMES_GetSequence( self, "amba" );
		if ( pSequence )
		{
			gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex(pSequence->sound1), 
                                     1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
		}
		hook->sound_time = gstate->time + 10.0 + rnd() * 30.0;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		ai_ambient_sound
// Description:
// Input:
// Output:
// Note:
//	plays an appropriate ambient sound based on actor type
//
// ----------------------------------------------------------------------------
void ai_ambient_sound( userEntity_t *self )
{
	_ASSERTE( self );
	if ( bDisableAISound )
	{
		return;
	}

	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );
	if ( hook->sound_time >= gstate->time )
	{
		return;
	}

	userEntity_t *pClient = AI_CanClientHear( self );
	if ( pClient )
	{
		if ( self->s.frameInfo.frameState & FRSTATE_PLAYSOUND1 )
		{
			self->s.frameInfo.frameState &= ~FRSTATE_PLAYSOUND1;

			if ( hook->sound1 )
			{
				gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex(hook->sound1), 
                                         1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
				hook->sound_time = gstate->time + 10.0 + rnd() * 30.0;
			}
		}
		else 
		if ( self->s.frameInfo.frameState & FRSTATE_PLAYSOUND2 )
		{
			self->s.frameInfo.frameState &= ~FRSTATE_PLAYSOUND2;
			if ( hook->sound2 )
			{
				gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex(hook->sound2), 
                                         1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
				hook->sound_time = gstate->time + 10.0 + rnd() * 30.0;
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		AI_PlaySurfaceSounds
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_PlaySurfaceSounds( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    if ( !self->groundEntity || !self->groundSurface )
    {
    	return;
    }

    int nSurfaceFlags = self->groundSurface->flags;
	char szSound[64] = { "\0" };

    int nRandom = rand() % 4;
	if(self->waterlevel <= 2)
	{
		if ( nSurfaceFlags & SURF_WOOD )
		{
			switch ( nRandom )
			{
				case 0: strcpy( szSound, "global/p_stpwd1.wav" ); break;
				case 1: strcpy( szSound, "global/p_stpwd2.wav" ); break;
				case 2: strcpy( szSound, "global/p_stpwd3.wav" ); break;
				case 3: strcpy( szSound, "global/p_stpwd4.wav" ); break;
				default: break;
			}
		}
		else 
		if ( nSurfaceFlags & SURF_METAL )
		{
			switch ( nRandom )
			{
				case 0: strcpy( szSound, "global/p_stpmt1.wav" ); break;
				case 1: strcpy( szSound, "global/p_stpmt2.wav" ); break;
				case 2: strcpy( szSound, "global/p_stpmt3.wav" ); break;
				case 3: strcpy( szSound, "global/p_stpmt4.wav" ); break;
				default: break;
			}
		}
		else 
		if ( nSurfaceFlags & SURF_STONE )
		{
			switch ( nRandom )
			{
				case 0: strcpy( szSound, "global/p_stpst1.wav" ); break;
				case 1: strcpy( szSound, "global/p_stpst2.wav" ); break;
				case 2: strcpy( szSound, "global/p_stpst3.wav" ); break;
				case 3: strcpy( szSound, "global/p_stpst4.wav" ); break;
				default: break;
			}
		}
		else 
		if ( nSurfaceFlags & SURF_ICE )
		{
			switch ( nRandom )
			{
				case 0: strcpy( szSound, "global/p_stpic1.wav" ); break;
				case 1: strcpy( szSound, "global/p_stpic2.wav" ); break;
				case 2: strcpy( szSound, "global/p_stpic3.wav" ); break;
				case 3: strcpy( szSound, "global/p_stpic4.wav" ); break;
				default: break;
			}
		}
		else 
		if ( nSurfaceFlags & SURF_SNOW )
		{
			switch ( nRandom )
			{
				case 0: strcpy( szSound, "global/p_stpsn1.wav" ); break;
				case 1: strcpy( szSound, "global/p_stpsn2.wav" ); break;
				case 2: strcpy( szSound, "global/p_stpsn3.wav" ); break;
				case 3: strcpy( szSound, "global/p_stpsn4.wav" ); break;
				default: break;
			}
		}
		else
		if ( nSurfaceFlags & SURF_PUDDLE || nSurfaceFlags & SURF_SURGE || nSurfaceFlags & SURF_BIGSURGE )
		{
			switch ( nRandom )
			{
				case 0: strcpy( szSound, "global/p_stppu1.wav" ); break;
				case 1: strcpy( szSound, "global/p_stppu2.wav" ); break;
				case 2: strcpy( szSound, "global/p_stppu3.wav" ); break;
				case 3: strcpy( szSound, "global/p_stppu4.wav" ); break;
				default: break;
			}
		}
		else
		if ( nSurfaceFlags & SURF_SAND )
		{
			switch ( nRandom )
			{
				case 0: strcpy( szSound, "global/p_stpsd1.wav" ); break;
				case 1: strcpy( szSound, "global/p_stpsd2.wav" ); break;
				case 2: strcpy( szSound, "global/p_stpsd3.wav" ); break;
				case 3: strcpy( szSound, "global/p_stpsd4.wav" ); break;
				default: break;
			}
		}
		else
		{
			switch ( nRandom )
			{
				case 0: strcpy( szSound, "global/p_stp1.wav" ); break;
				case 1: strcpy( szSound, "global/p_stp2.wav" ); break;
				case 2: strcpy( szSound, "global/p_stp3.wav" ); break;
				case 3: strcpy( szSound, "global/p_stp4.wav" ); break;
				default: break;
			}
		}
	}
	else//NSS[11/13/99]:We are in water let's play the underwater swimming noise
	{
			switch ( nRandom )
			{
				case 0: strcpy( szSound, "hiro/swim1.wav" ); break;
				case 1: strcpy( szSound, "hiro/swim2.wav" ); break;
				case 2: strcpy( szSound, "hiro/swim3.wav" ); break;
				case 3: strcpy( szSound, "hiro/swim4.wav" ); break;
				default: break;
			}
			
	}
    if ( strlen(szSound) > 0 )
    {
		float SoundLevel = 1.0;
		if(self->waterlevel <= 2)
		{
			SoundLevel = 0.65f;
		}
		gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex(szSound), 
                                 SoundLevel, hook->fMinAttenuation, hook->fMaxAttenuation);
    }

}

// ----------------------------------------------------------------------------
//
// Name:		ai_frame_sounds
// Description:
//			plays sounds based on frame flags set in ai_prethink
// Input:
// Output:
// Note:
//          for sidekicks, play sounds per surface
//
// ----------------------------------------------------------------------------
void ai_frame_sounds( userEntity_t *self )
{
	_ASSERTE( self );
	if ( AI_IsSoundDisabled() )
	{
		return;
	}

	userEntity_t *pClient = AI_CanClientHear( self );
	if ( pClient )
	{
		playerHook_t *hook = AI_GetPlayerHook( self );
		if ( self->s.frameInfo.frameState & FRSTATE_PLAYSOUND1 )
		{
			self->s.frameInfo.frameState &= ~FRSTATE_PLAYSOUND1;
			if ( AI_IsSidekick( hook ) && AI_IsStateMoving( hook ) )
            {
                AI_PlaySurfaceSounds( self );
            }
            else
            {
			    if ( hook->sound1 )
			    {
                    gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex(hook->sound1), 
                                             1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
			    }
            }
		}
		else 
		if ( self->s.frameInfo.frameState & FRSTATE_PLAYSOUND2 )
		{
			self->s.frameInfo.frameState &= ~FRSTATE_PLAYSOUND2;
			if ( AI_IsSidekick( hook ) && AI_IsStateMoving( hook ) )
            {
                AI_PlaySurfaceSounds( self );
            }
            else
            {
			    if ( hook->sound2 )
			    {
				    gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex(hook->sound2), 
                                             1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
			    }
            }
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		FRAMES_GetSequence
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
frameData_t *FRAMES_GetSequence( userEntity_t *self, char *name )
{
	if(!self->pMapAnimationToSequence)
		return NULL;
	else
		return (frameData_t *)(*self->pMapAnimationToSequence)[name];
}

// ----------------------------------------------------------------------------
//	following functions are defined as inline in release build
//	the compile time was too long in debug build, so they are defined here
// ----------------------------------------------------------------------------

#ifdef _DEBUG

float FRAMES_ComputeFrameScale( playerHook_t *hook )
{
	return 1.0f;
}

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_ToggleSound()
{
	if ( bDisableAISound )
	{
		bDisableAISound = FALSE;
	}
	else
	{
		bDisableAISound = TRUE;
	}
}

int AI_IsSoundDisabled()
{
	return bDisableAISound;
}

#endif _DEBUG


// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

