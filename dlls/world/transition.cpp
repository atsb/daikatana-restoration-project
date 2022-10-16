// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#if _MSC_VER
#include <crtdbg.h>
#endif
#include "world.h"
#include "transition.h"
#include "csv.h"
#include "dk_io.h"
#include "ai_frames.h"
#include "ai_utils.h"

/* ***************************** define types ****************************** */
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* ******************************* exports ********************************* */

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
CTransitionSequences::CTransitionSequences()
{
	nNumTransitions = 0;
	pTransitions = NULL;
}

CTransitionSequences::CTransitionSequences( int nNum )
{
	nNumTransitions = nNum;
	pTransitions = (TRANSITIONINFO*)memmgr.X_Malloc(sizeof(TRANSITIONINFO)*nNumTransitions,MEM_TAG_MISC);
}

CTransitionSequences::~CTransitionSequences()
{
	nNumTransitions = 0;
	memmgr.X_Free(pTransitions);
	pTransitions = NULL;
}

TRANSITION_PTR CTransitionSequences::Allocate( int nNum )
{
	nNumTransitions = nNum;
	pTransitions = (TRANSITIONINFO*)memmgr.X_Malloc(sizeof(TRANSITIONINFO)*nNumTransitions,MEM_TAG_MISC);
	return pTransitions;
}


// ----------------------------------------------------------------------------
//
// Name:		SetTransitionSequence
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CTransitionSequences::SetTransitionSequence( int nIndex, frameData_t *pSequence1, frameData_t *pSequence2, frameData_t *pSequence3 )
{
	_ASSERTE( pTransitions );
	_ASSERTE( nIndex >= 0 && nIndex < nNumTransitions );
	
	pTransitions[nIndex].pFromSequence		 = pSequence1;
	pTransitions[nIndex].pToSequence		 = pSequence2;
	pTransitions[nIndex].pTransitionSequence = pSequence3;
}

// ----------------------------------------------------------------------------
//
// Name:		FindTransitionSequence
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
frameData_t *CTransitionSequences::FindTransitionSequence( frameData_t *pFromSequenceTest, frameData_t *pToSequenceTest )
{
	for ( int i = 0; i < nNumTransitions; i++ )
	{
		if ( pTransitions[i].pFromSequence == pFromSequenceTest &&
			 pTransitions[i].pToSequence == pToSequenceTest )
		{
			return pTransitions[i].pTransitionSequence;
		}
	}

	return NULL;
}

// ****************************************************************************
//								TRANSITION functions
// ****************************************************************************

// ----------------------------------------------------------------------------
//
// Name:		TRANSITION_ReadFile
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int TRANSITION_ReadFile( const char *szFileName, userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

    int bSuccess = TRUE;

	CCSVFile *pCsvFile = NULL;
	int nRetCode = CSV_OpenFile( szFileName, &pCsvFile );
//	if ( nRetCode == ERROR_NONE ) //SCG: 2-26-99
	if ( nRetCode == DKIO_ERROR_NONE )
	{
		int nNumTransitions = 0;
		
		char szLine[2048];
		char szElement[64];
		// skip first line
		CSV_GetNextLine( pCsvFile, szLine );
		while ( CSV_GetNextLine( pCsvFile, szLine ) != EOF )
		{
			int nNumElements = 0;

			int nRetValue = CSV_GetFirstElement( pCsvFile, szLine, szElement );
			if ( nRetValue > 0 )
			{
				nNumElements++;
				while ( (nRetValue = CSV_GetNextElement( pCsvFile, szLine, szElement )) != EOLN )
				{
					nNumElements++;	
				}
			
				nNumTransitions++;
			}
		}

		CSV_SeekToBeginning( pCsvFile );

		CTransitionSequences *pTransitionSequences = new CTransitionSequences( nNumTransitions );

		nNumTransitions = 0;
		frameData_t *pFromSequence = NULL;
		frameData_t *pToSequence = NULL;
		frameData_t *pTransitionSequence = NULL;
		
		// skip first line
		CSV_GetNextLine( pCsvFile, szLine );
		while ( CSV_GetNextLine( pCsvFile, szLine ) != EOF )
		{
			int nRetValue = CSV_GetFirstElement( pCsvFile, szLine, szElement );
			if ( nRetValue > 0 )
			{
				_ASSERTE( nRetValue > 0 );
				pFromSequence = FRAMES_GetSequence( self, szElement );
				_ASSERTE( pFromSequence );

				nRetValue = CSV_GetNextElement( pCsvFile, szLine, szElement );
				_ASSERTE( nRetValue > 0 );
				pToSequence = FRAMES_GetSequence( self, szElement );
				_ASSERTE( pToSequence );

				nRetValue = CSV_GetNextElement( pCsvFile, szLine, szElement );
				_ASSERTE( nRetValue > 0 );
				pTransitionSequence = FRAMES_GetSequence( self, szElement );
				_ASSERTE( pTransitionSequence );

				pTransitionSequences->SetTransitionSequence( nNumTransitions, pFromSequence, pToSequence, pTransitionSequence );
				nNumTransitions++;
			}
		}

		hook->pTransitions = pTransitionSequences;

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
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
