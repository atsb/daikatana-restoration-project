// ==========================================================================
//
//  File:		SequenceMap.cpp
//  Contents:
//  Author:
//
// ==========================================================================

#if _MSC_VER
#include <crtdbg.h>
#endif
#include "world.h"
#include "SequenceMap.h"
#include "collect.h"

/* ***************************** define types ****************************** */

/* ***************************** Local Variables *************************** */

static CMapIntegerToPtr mapModelToAnimations;

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* ******************************* exports ********************************* */
/* ***************************** Local Functions *************************** */


CSequenceList::CSequenceList()
{
	nNumSequences = 0;
	pSequences = NULL;
}

CSequenceList::CSequenceList( int nNewNum )
{
	nNumSequences = nNewNum;
	pSequences = AllocateSequences( nNewNum );
}

CSequenceList::~CSequenceList()
{
	DeleteSequences();
}

frameData_t *CSequenceList::AllocateSequences( int nNewNum )
{
	nNumSequences = nNewNum;
	
	//pSequences = new frameData_t[nNewNum];
	pSequences = (frameData_t*)gstate->X_Malloc(sizeof(frameData_t)*nNewNum,MEM_TAG_AI);

	return pSequences;
}

void CSequenceList::DeleteSequences()
{
	//delete [] pSequences;
	gstate->X_Free(pSequences);
	pSequences = NULL;

	nNumSequences = 0;
}

void CSequenceList::MapAnimationNameToSequences( userEntity_t *self )
{
	_ASSERTE( self->pMapAnimationToSequence->GetHashTableSize() > 0 );

	for ( int i = 0; i < nNumSequences; i++ )
	{
		frameData_t *pSequence = GetSequence( i );
		_ASSERTE( pSequence );
		_ASSERTE( strlen(pSequence->animation_name) > 0 );

		self->pMapAnimationToSequence->SetAt( pSequence->animation_name, pSequence );
	}
}

frameData_t *CSequenceList::GetSequence( const char *animName )
{
	for ( int i = 0; i < nNumSequences; i++ )
	{
		frameData_t *pSequence = GetSequence( i );
		_ASSERTE( pSequence );

		if ( _stricmp( pSequence->animation_name, animName ) == 0 )
		{
			return pSequence;
		}
	}

	return NULL;	
}

void CSequenceList::Verify()
{
	for ( int i = 0; i < nNumSequences; i++ )
	{
		frameData_t *pSequence = GetSequence( i );
		_ASSERTE( pSequence );
		_ASSERTE( strlen(pSequence->animation_name) > 0 );
	}
}

// ****************************************************************************
// ****************************************************************************

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

CSequenceList *SEQUENCEMAP_Lookup( int nModelIndex )
{
	void *pValue = NULL;
    mapModelToAnimations.Lookup(nModelIndex, pValue);
    CSequenceList *pSequenceList = (CSequenceList *)pValue;

    return pSequenceList;
}

void SEQUENCEMAP_Add( int nModelIndex, CSequenceList *pSequenceList )
{
	mapModelToAnimations.SetAt( nModelIndex, pSequenceList );
}

void SEQUENCEMAP_Remove( int nModelIndex )
{
	int nKey;

	mapModelToAnimations.LookupKey( nModelIndex, nKey );
	mapModelToAnimations.RemoveKey( nKey );
}

void SEQUENCEMAP_RemoveAll()
{
	if ( !mapModelToAnimations.IsEmpty() )
    {
	    int key;
	    void *value;
	    POSITION pos = mapModelToAnimations.GetStartPosition();
	    while ( pos )
	    {
		    mapModelToAnimations.GetNextAssoc( pos, key, value );

		    CSequenceList *pSequenceList = (CSequenceList *)value;
            if ( pSequenceList )
            {
	            delete pSequenceList;
            }
        }
        
        mapModelToAnimations.RemoveAll();
    }
}

void SEQUENCEMAP_AllocSequenceMap( userEntity_t *self )
{
    if ( !self->pMapAnimationToSequence )
    {
	    self->pMapAnimationToSequence = new CMapStringToPtr;
        if ( !self->pMapAnimationToSequence )
        {
            com->Error( "MEM allocation failed." );
        }
    }
}
