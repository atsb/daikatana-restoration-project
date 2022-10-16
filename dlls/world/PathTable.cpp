// ==========================================================================
//
//  File:		NodeList.cpp
//  Contents:
//  Author:
//
// ==========================================================================
#include <stdio.h>
#if _MSC_VER
#include <crtdbg.h>
#endif
#include "world.h"
#include "nodelist.h"
#include "octree.h"
#include "dk_thread.h"

/* ***************************** define types ****************************** */
/* ***************************** Local Variables *************************** */
static CThreadSync pathTableThread;

/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* ******************************* exports ********************************* */



// ----------------------------------------------------------------------------
//
// Name:        PATHTABLE_ContinueThread
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void PATHTABLE_ContinueThread()
{
    _ASSERTE( cvarBuildPathTable );

    if ( cvarBuildPathTable->value > 0.0f )
    {
        putchar( '.' );
        if ( pathTableThread.WaitToWrite(10) )
        {
            pathTableThread.WriteToSharedData( "0" );
            pathTableThread.DoneWriting();
        }
    }
}

// ----------------------------------------------------------------------------
//
// Name:        PATHTABLE_ThreadFunc
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
unsigned int __stdcall PATHTABLE_ThreadFunc( void *pData ) 
{
    _ASSERTE( cvarBuildPathTable );

    printf( "Building Path Table for %s.map.\n", gstate->mapName );

    if ( pGroundNodes && pGroundNodes->nNumNodes > 0 )
    {
        NODE_ComputePathTable( pGroundNodes );
    }

    if ( pAirNodes && pAirNodes->nNumNodes > 0 )
    {
        NODE_ComputePathTable( pAirNodes );
    }

    if ( pTrackNodes && pTrackNodes->nNumNodes > 0 )
    {
        NODE_ComputePathTable( pTrackNodes );
    }

	if ( pPathOctree )
	{
		pPathOctree = OCTREE_Destruct( pPathOctree );
        pPathOctree = NULL;
    }

    pPathOctree = OCTREE_Construct();
    OCTREE_BuildOctree( pPathOctree );
    NODE_Write();

    putchar( '\n' );
    if ( pathTableThread.WaitToWrite(10) )
    {
        pathTableThread.WriteToSharedData( "1" );
        pathTableThread.DoneWriting();
    }

	return 0;
}

// ----------------------------------------------------------------------------
//
// Name:        PATHTABLE_BuildPathTable
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void PATHTABLE_BuildPathTable()
{
    // ISP: 06-28-99 building a path table 
    extern cvar_t *cvarBuildPathTable;
    if ( !cvarBuildPathTable )
    {
        cvarBuildPathTable = gstate->cvar( "BuildPathTable", "", 0 );
    }

    if ( cvarBuildPathTable->value > 0.0f )
    {
        pathTableThread.Init( "PATHTABLEFILE", "PATHTABLEOBJECT" );
        pathTableThread.OpenFileMapForWrite_OpenEvent();
        pathTableThread.CreateThread( PATHTABLE_ThreadFunc );
    }
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
