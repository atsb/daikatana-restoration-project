// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
#include <stdio.h>
#include <stdlib.h>
#include "dk_std.h"
#include "dk_shared.h"
#include "world.h"
#include "collect.h"
#include "EntManager.h"

/* ***************************** define types ****************************** */

//class CEntityInfo
//{
//private:
//	userEntity_t *pEntity;
//	int bInUse;
//
//public:
//	CEntityInfo()
//	{
//        pEntity = NULL;
//        bInUse = FALSE;
//    }
//    CEntityInfo( userEntity_t *pNewEntity )
//    {
//        pEntity = pNewEntity;
//        bInUse = TRUE;
//    }
//	~CEntityInfo()
//	{
//        pEntity = NULL;
//        bInUse = FALSE;
//    }
// 	
//	void SetInUsed()		{ bInUse = TRUE; }
//	void SetNoLongerUsed()	{ bInUse = FALSE; }
//	int IsInUse()			{ return bInUse; }
//	
//	void SetEntity( userEntity_t *pNewEntity )	
//    { 
//        pEntity = pNewEntity;
//        if ( pNewEntity )
//        {
//            bInUse = TRUE;
//        }
//        else
//        {
//            bInUse = FALSE;
//        }
//    }
//	userEntity_t *GetEntity()	{ return pEntity; }		
//};

/* ***************************** Local Variables *************************** */

CTypedPtrArray<CPtrArray, userEntity_t *> entityManager;


/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* ******************************* exports ********************************* */

// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
