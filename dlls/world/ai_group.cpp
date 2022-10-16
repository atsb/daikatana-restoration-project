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
#include "ai_common.h"
#include "actorlist.h"
#include "collect.h"

/* ***************************** define types **************************** */

class CMonsterGroup
{
private:
	char	 szName[32];
	CPtrList *pMonsterList;

	userEntity_t *pLeader;

public:
	CMonsterGroup();
	~CMonsterGroup();

	void SetLeader( userEntity_t *pMonster )	{ pLeader = pMonster; }
	userEntity_t *GetLeader()					{ return pLeader; }
	
	void AddMonster( userEntity_t *pMonster );
	void RemoveMonster( userEntity_t *pMonster );
};


CPtrList monsterGroupList;

/* ***************************** Local Variables **************************** */
/* ***************************** Local Functions **************************** */
/* ***************************** Global Variables *************************** */


CMonsterGroup::CMonsterGroup()
{
	// allocate just two elements
	pMonsterList = new CPtrList( 2 );	
	pLeader = NULL;
}

CMonsterGroup::~CMonsterGroup()
{
	delete pMonsterList;
	pMonsterList = NULL;

	pLeader = NULL;
}

void CMonsterGroup::AddMonster( userEntity_t *pMonster )
{
	_ASSERTE( pMonsterList );

	pMonsterList->AddTail( pMonster );
}

void CMonsterGroup::RemoveMonster( userEntity_t *pMonster )
{
	_ASSERTE( pMonsterList );

	POSITION pos = pMonsterList->Find( pMonster);
	if ( pos )
	{
		pMonsterList->RemoveAt( pos );
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

