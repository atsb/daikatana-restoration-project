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
#include "ai_utils.h"

/* ***************************** define types ****************************** */

class CAIInfo
{
private:
	userEntity_t *pPlayer;
    userEntity_t *pFirstSidekick;
    userEntity_t *pSecondSidekick;
    int nNumSidekicks;

    userEntity_t *pLastSelectedSidekick;

public:
	CAIInfo()
	{
        Init();
    }
	~CAIInfo() {}
 	
	void Init()
    {
        pPlayer         = NULL;
        pFirstSidekick  = NULL;
        pSecondSidekick = NULL;
        pLastSelectedSidekick = NULL;
        nNumSidekicks   = 0;
    }

    void SetPlayer( userEntity_t *pNewEntity )	{ pPlayer = pNewEntity; }
	userEntity_t *GetPlayer()	                { return pPlayer; }

	void AddSidekick( userEntity_t *pNewEntity )
    { 
        if ( pNewEntity == pFirstSidekick || pNewEntity == pSecondSidekick )
        {
            return;
        }
        
        if ( !AI_IsAlive(pFirstSidekick) )
        {
            pFirstSidekick = pNewEntity; 
        }
        else
        if ( !AI_IsAlive(pSecondSidekick) )
        {
            pSecondSidekick = pNewEntity;
        }
        else
        {
            _ASSERTE( FALSE );
        }
        nNumSidekicks++;
    }
    void DeleteSidekick( userEntity_t *pEntity )
    {
        if ( pFirstSidekick == NULL && pSecondSidekick == NULL )
        {
            return;
        }
        if ( pEntity != pFirstSidekick && pEntity != pSecondSidekick )
        {
            return;
        }

        if ( pEntity == pFirstSidekick )
        {
			if( pSecondSidekick != NULL )
			{
	            pFirstSidekick = pSecondSidekick;
	            pSecondSidekick = NULL;
			}

            nNumSidekicks--;

            if ( nNumSidekicks == 0 )
            {
                pFirstSidekick = NULL;
            }
        }
        
        if ( pEntity == pSecondSidekick )
        {
            pSecondSidekick = NULL;
            nNumSidekicks--;
        }

    }

    int GetNumSidekicks()               { return nNumSidekicks; }
	userEntity_t *GetFirstSidekick()    { return pFirstSidekick; }
	userEntity_t *GetSecondSidekick()	{ return pSecondSidekick; }

    userEntity_t *GetSuperfly()
    {
        if ( pFirstSidekick && pFirstSidekick->className && pFirstSidekick->inuse )
        {
			if( strstr( pFirstSidekick->className, "Superfly" ) )
	            return pFirstSidekick;
        }

        if ( pSecondSidekick && pSecondSidekick->className && pSecondSidekick->inuse )
        {
			if( strstr( pSecondSidekick->className, "Superfly" ) )
	            return pSecondSidekick;
        }

		// SCG[2/24/00]: When loading a savegame, it is possible for the sidekick to be around, but inactive.
		if( pFirstSidekick != NULL )
		{
			if ( ( pFirstSidekick->inuse == 0 ) || 
				( ( strcmp( pFirstSidekick->className, "Superfly" ) != 0 ) &&
				( strcmp( pFirstSidekick->className, "Mikiko" ) != 0 ) )
				)
			{
				DeleteSidekick( pFirstSidekick );
			}
		}
		if( pSecondSidekick != NULL ) 
		{
			if( ( pSecondSidekick->inuse == 0 ) || 
				( ( strcmp( pSecondSidekick->className, "Superfly" ) != 0 ) &&
				( strcmp( pSecondSidekick->className, "Mikiko" ) != 0 ) )
				)
			{
				DeleteSidekick( pSecondSidekick );
			}
		}

        return NULL;
    }
    userEntity_t *GetMikikofly()
    {
		userEntity_t *pSuperfly = GetSuperfly();
		if(pSuperfly != NULL)
		{
			playerHook_t *hook = (playerHook_t *)pSuperfly->userHook;
			if(hook != NULL && hook->bCarryingMikiko)
			{
				return pSuperfly;
			}
		}
		return NULL;
	}

    userEntity_t *GetMikiko()
    {
        if ( pFirstSidekick && pFirstSidekick->className && pFirstSidekick->inuse )
        {
	        if ( strstr( pFirstSidekick->className, "Mikiko" ) )
		        return pFirstSidekick;
        }

        if ( pSecondSidekick && pSecondSidekick->className && pSecondSidekick->inuse )
        {
	        if ( strstr( pSecondSidekick->className, "Mikiko" ) )
		        return pSecondSidekick;
        }

		// SCG[2/24/00]: When loading a savegame, it is possible for the sidekick to be around, but inactive.
		if( pFirstSidekick != NULL )
		{
			if ( ( pFirstSidekick->inuse == 0 ) || 
				( ( strcmp( pFirstSidekick->className, "Superfly" ) != 0 ) &&
				( strcmp( pFirstSidekick->className, "Mikiko" ) != 0 ) )
				)
			{
				DeleteSidekick( pFirstSidekick );
			}
		}
		if( pSecondSidekick != NULL ) 
		{
			if( ( pSecondSidekick->inuse == 0 ) || 
				( ( strcmp( pSecondSidekick->className, "Superfly" ) != 0 ) &&
				( strcmp( pSecondSidekick->className, "Mikiko" ) != 0 ) )
				)
			{
				DeleteSidekick( pSecondSidekick );
			}
		}

        return NULL;
    }

    void SetLastSelectedSidekick( userEntity_t *pEntity )
    {
        pLastSelectedSidekick = pEntity;
    }
    userEntity_t *GetLastSelectedSidekick() { return pLastSelectedSidekick; }
};

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* ******************************* exports ********************************* */

CAIInfo aiInfo;

// ----------------------------------------------------------------------------

void AIINFO_Init()
{
    aiInfo.Init();
}

void AIINFO_AddAllSidekicks()
{
	userEntity_t *pEntity;
	for( pEntity = gstate->FirstEntity (); pEntity; pEntity = gstate->NextEntity( pEntity ) )
	{
		if( strcmp( pEntity->className, "Superfly" ) == 0 )
		{
			aiInfo.AddSidekick( pEntity );
		}
		else if( strcmp( pEntity->className, "Mikiko" ) == 0 )
		{
			aiInfo.AddSidekick( pEntity );
		}
		else if( strcmp( pEntity->className, "MikikoFly" ) == 0 )
		{
			aiInfo.AddSidekick( pEntity );
		}

	}
}

void AIINFO_SetPlayer( userEntity_t *pNewEntity )	
{ 
    aiInfo.SetPlayer( pNewEntity );
}
userEntity_t *AIINFO_GetPlayer()
{ 
    return aiInfo.GetPlayer();
}

void AIINFO_AddSidekick( userEntity_t *pNewEntity )
{ 
    aiInfo.AddSidekick( pNewEntity );
}
void AIINFO_DeleteSidekick( userEntity_t *pEntity )
{
    aiInfo.DeleteSidekick( pEntity );
}
userEntity_t *AIINFO_GetFirstSidekick()
{ 
    return aiInfo.GetFirstSidekick();
}
userEntity_t *AIINFO_GetSecondSidekick()
{ 
    return aiInfo.GetSecondSidekick();
}
userEntity_t *AIINFO_GetSuperfly()
{
    return aiInfo.GetSuperfly();
}
userEntity_t *AIINFO_GetMikikofly()
{
    return aiInfo.GetMikikofly();
}
userEntity_t *AIINFO_GetMikiko()
{
    return aiInfo.GetMikiko();
}

int AIINFO_GetNumSidekicks()
{
    return aiInfo.GetNumSidekicks();
}

void AIINFO_SetLastSelectedSidekick( userEntity_t *pEntity )
{
    aiInfo.SetLastSelectedSidekick( pEntity );
}

userEntity_t *AIINFO_GetLastSelectedSidekick()
{
    return aiInfo.GetLastSelectedSidekick();
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
