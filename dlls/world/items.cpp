//////////////////////////////////////////////////////////////////////
//	items.cpp
//
//	routines for handling pick up items and power ups
//
//////////////////////////////////////////////////////////////////////

#include "world.h"
#include "items.h"
#include "collect.h"
#include "nodelist.h"
#include "GrphPath.h"
#include "client.h"
#include "ai_utils.h"
//#include "actorlist.h"// SCG[1/23/00]: not used
#include "sidekick.h"

///////////////////////////////////////////////////////////////////////////////
//	exports
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
//						CItems functions
// ----------------------------------------------------------------------------

CPtrList *pWeaponList		= NULL;
CPtrList *pAmmoList			= NULL;
CPtrList *pArmorList		= NULL;
CPtrList *pHealthList		= NULL;
CPtrList *pStatBoostList	= NULL;
CPtrList *pGoldenSoulList	= NULL;



// ----------------------------------------------------------------------------
//
// Name:        af_powerup
// Description:
// Input:
// Output:
// Note:
//  cheat for powering up attributes
//
// ----------------------------------------------------------------------------
int calcStatLevel( userEntity_t *self );

#define WRAP_BOOST(boost)		if (boost > 5) boost = 0
int AI_IsVisible( userEntity_t *self, userEntity_t *targ );// NSS[2/22/00]:
#ifdef _DEBUG
void AI_Drop_Markers (CVector &Target, float delay);
#endif
//#ifdef _DEBUG
void af_powerup( userEntity_t *self )
{
	extern long exp_level[];
	int				count;
	char			*arg;
	playerHook_t	*hook = AI_GetPlayerHook( self );

	if( SinglePlayerCheat() == FALSE )
	{
		if( gstate->GetCvar( "cheats" ) == 0 )
		{
			return;
		}
	}

	if (deathmatch->value)
		return;

	count = gstate->GetArgc ();

	if (count < 2 || count > 2)
	{
		gstate->bprint ("\ngreyskull [powerup]\n\ngives one level of boost to an attribute.");
		gstate->bprint ("[powerup] can be:\n  power\n  speed\n  attack\n  acro\n  vita\n  all\n");
	}

	arg = gstate->GetArgv (1);

	if (!_stricmp (arg, "power"))
		hook->base_power++;
	else 
    if (!_stricmp (arg, "speed"))
		hook->base_speed++;
	else 
    if (!_stricmp (arg, "attack"))
		hook->base_attack++;
	else 
    if (!_stricmp (arg, "acro"))
		hook->base_acro++;
	else 
    if (!_stricmp (arg, "vita"))
		hook->base_vita++;
	else 
    if (!_stricmp (arg, "all"))
	{
		hook->base_power++;
		hook->base_speed++;
		hook->base_attack++;
		hook->base_acro++;
		hook->base_vita++;		
	}

	WRAP_BOOST(hook->base_power);
	WRAP_BOOST(hook->base_speed);
	WRAP_BOOST(hook->base_attack);
	WRAP_BOOST(hook->base_acro);
	WRAP_BOOST(hook->base_vita);

	int level = calcStatLevel(self);
	self->record.exp = exp_level[level];
	com->CalcBoosts (self);
}

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//  cheat for powering down attributes
//
// ----------------------------------------------------------------------------
void af_powerdown( userEntity_t *self )
{
	extern long exp_level[];
	int				count;
	char			*arg;
	playerHook_t	*hook = AI_GetPlayerHook( self );
	
	if( SinglePlayerCheat() == FALSE )
	{
		if( gstate->GetCvar( "cheats" ) == 0 )
		{
			return;
		}
	}

	if (deathmatch->value)
		return;
		
	count = gstate->GetArgc ();

	if (count < 2 || count > 2)
	{
		gstate->bprint ("breakme [powerup]\n\nremoves one level of boost from an attribute.");
		gstate->bprint ("[powerup] can be:\n  power\n  speed\n  attack\n  acro\n  vita\n  all\n");
	}

	arg = gstate->GetArgv (1);

	if (!_stricmp (arg, "power") && hook->power_boost > 0)
		hook->power_boost--;
	else 
    if (!_stricmp (arg, "speed") && hook->speed_boost > 0)
		hook->speed_boost--;
	else 
    if (!_stricmp (arg, "attack") && hook->attack_boost > 0)
		hook->attack_boost--;
	else 
    if (!_stricmp (arg, "acro") && hook->acro_boost > 0)
		hook->acro_boost--;
	else 
    if (!_stricmp (arg, "vita") && hook->vita_boost > 0)
		hook->vita_boost--;
	else 
    if (!_stricmp (arg, "all"))
	{
		if (hook->power_boost > 0)
			hook->power_boost--;
		if (hook->speed_boost > 0)
			hook->speed_boost--;
		if (hook->attack_boost > 0)
			hook->attack_boost--;
		if (hook->acro_boost > 0)		
			hook->acro_boost--;
		if (hook->vita_boost > 0)
			hook->vita_boost--;		
	}

	int level = calcStatLevel(self);
	self->record.exp = exp_level[level];
	com->CalcBoosts (self);
}
//#endif


// ----------------------------------------------------------------------------
//
// Name:        ITEM_CanPath
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
bool ITEM_CanPath( CVector &point1, CVector &point2 )
{
	_ASSERTE( pGroundNodes );

	MAPNODE_PTR pNode1 = NODE_FindClosestNode( pGroundNodes, NODETYPE_GROUND, point1 );
	MAPNODE_PTR pNode2 = NODE_FindClosestNode( pGroundNodes, NODETYPE_GROUND, point2 );
	if ( !pNode1 || !pNode2 )
	{
		return FALSE;
	}

	tr = gstate->TraceLine_q2( pNode2->position, point2, NULL, MASK_SOLID );
	if ( tr.fraction >= 1.0f && PATH_ComputePath( pGroundNodes, pNode1, pNode2 ) )
	{
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		ITEM_Destroy
// Description:
// Input:
// Output:
// Note:
//				ASSUME: the items are valid throughout a level, even if it is 
//				picked up and are not visible.  It should remain intact in memory
//
// ----------------------------------------------------------------------------
void ITEM_Destroy()
{
	if ( pWeaponList )
	{
		delete pWeaponList;
        pWeaponList = NULL;
	}
	if ( pAmmoList )
	{
		delete pAmmoList;
        pAmmoList = NULL;
	}
	if ( pArmorList )
	{
		delete pArmorList;
        pArmorList = NULL;
	}
	if ( pHealthList )
	{
		delete pHealthList;
        pHealthList = NULL;
	}
	if ( pStatBoostList )
	{
		delete pStatBoostList;
        pStatBoostList = NULL;
	}
	if ( pGoldenSoulList )
	{
		delete pGoldenSoulList;
        pGoldenSoulList = NULL;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		ITEM_SetSpawnValue
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ITEM_SetSpawnValue( userEntity_t *self, int nSpawnValue )
{
	_ASSERTE( self );

	if ( nSpawnValue & ITEMSPAWN_SUPERFLY )
	{
	    self->nSidekickFlag &= ~SUPERFLY_NO_PICKUP;
    }
	if ( nSpawnValue & ITEMSPAWN_MIKIKO )
	{
        self->nSidekickFlag &= ~MIKIKO_NO_PICKUP;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		ITEM_ProcessSpawnFlag
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ITEM_ProcessSpawnFlag( userEntity_t *self, char *value )
{
	_ASSERTE( self );
	int nSpawnValue = -1;
	if ( _stricmp(value, "") != 0 )
	{
		nSpawnValue = atoi( value );
	}
	_ASSERTE( nSpawnValue != -1 );

	ITEM_SetSpawnValue( self, nSpawnValue );
}

// ----------------------------------------------------------------------------
//
// Name:		ITEM_ParseEpairs
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ITEM_ParseEpairs( userEntity_t *self )
{
	_ASSERTE( self );

	if ( self->className )
	{
		ai_debug_print( self, "%s\n", self->className );
	}

	if ( self->epair )
	{
	    int i = 0;
	    while ( self->epair[i].key )
	    {
		    if ( _stricmp( self->epair[i].key, "spawnflags" ) == 0 )
		    {
			    ITEM_ProcessSpawnFlag( self, self->epair[i].value );
		    }

		    i++;
	    }
    }
}

// ----------------------------------------------------------------------------
//
// Name:		ITEM_Initialize
// Description:
// Input:
// Output:
// Note:
//				ASSUME: the items are valid throughout a level, even if it is 
//				picked up and are not visible.  It should remain intact in memory
//
// ----------------------------------------------------------------------------
qboolean ITEM_Initialize()
{
    // clean if necessary
	ITEM_Destroy();
	
	// before adding any item to the list, make sure there is a path to it first

	userEntity_t *pClientSpawnPoint = Client_GetSpawnPoint( "info_player" );
	if(! pClientSpawnPoint )
	{
//		gstate->Error("You must have a player_info_spawn point defined before you can load a map. <nss>");
		return FALSE;
	}
	CVector point1 = pClientSpawnPoint->s.origin;

	pWeaponList = new CPtrList;
	pAmmoList = new CPtrList;
	pArmorList = new CPtrList;
	pHealthList = new CPtrList;
	pStatBoostList = new CPtrList;
	pGoldenSoulList = new CPtrList;

	if ( !pWeaponList || !pAmmoList || !pArmorList || !pHealthList || !pStatBoostList || !pGoldenSoulList )
	{
		_ASSERTE( FALSE );
		return FALSE;
	}

	userEntity_t *head = gstate->FirstEntity();
    while ( head )
    {
        int bItem = FALSE;
        if ( strstr( head->className, "weapon" ) )
		{
			if ( !deathmatch->value || ITEM_CanPath( point1, head->s.origin ) )
			{
				pWeaponList->AddTail( head );
			}
			else
			{
				AI_Dprintf( "Can not reach %s at (%d, %d, %d).\n", 
							head->className, 
							head->s.origin.x,
							head->s.origin.y,
							head->s.origin.z );
			}
            bItem = TRUE;
		}
		else
		if ( strstr( head->className, "ammo" ) )
		{
			if ( !deathmatch->value || ITEM_CanPath( point1, head->s.origin ) )
			{
				pAmmoList->AddTail( head );
			}
			else
			{
				AI_Dprintf( "Can not reach %s at (%d, %d, %d).\n", 
							head->className, 
							head->s.origin.x,
							head->s.origin.y,
							head->s.origin.z );
			}
            bItem = TRUE;
		}
		else
		if (strstr( head->className, "armor")) // NSS[12/16/99]:This should register all armor types...wasn't doing this before.
		{
			if ( !deathmatch->value || ITEM_CanPath( point1, head->s.origin ) )
			{
				pArmorList->AddTail( head );
			}
			else
			{
				AI_Dprintf( "Can not reach %s at (%d, %d, %d).\n", 
							head->className, 
							head->s.origin.x,
							head->s.origin.y,
							head->s.origin.z );
			}
            bItem = TRUE;
		}
		else
		if ( _stricmp( head->className, "item_power_boost" ) == 0 ||
			 _stricmp( head->className, "item_acro_boost" ) == 0 ||
			 _stricmp( head->className, "item_attack_boost" ) == 0 ||
			 _stricmp( head->className, "item_speed_boost" ) == 0 ||
			 _stricmp( head->className, "item_vita_boost" ) == 0 )
		{
			if ( !deathmatch->value || ITEM_CanPath( point1, head->s.origin ) )
			{
				pStatBoostList->AddTail( head );
			}
			else
			{
				AI_Dprintf( "Can not reach %s at (%d, %d, %d).\n", 
							head->className, 
							head->s.origin.x,
							head->s.origin.y,
							head->s.origin.z );
			}
            bItem = TRUE;
		}
		else
		if ( strstr( head->className, "item_health" ) )
		{
			if ( !deathmatch->value || ITEM_CanPath( point1, head->s.origin ) )
			{
				pHealthList->AddTail( head );
			}
			else
			{
				AI_Dprintf( "Can not reach %s at (%d, %d, %d).\n", 
							head->className, 
							head->s.origin.x,
							head->s.origin.y,
							head->s.origin.z );
			}
            bItem = TRUE;
		}
		else
		if ( _stricmp( head->className, "item_goldensoul" ) == 0 )
		{
			if ( !deathmatch->value || ITEM_CanPath( point1, head->s.origin ) )
			{
				pGoldenSoulList->AddTail( head );
			}
			else
			{
				AI_Dprintf( "Can not reach %s at (%d, %d, %d).\n", 
							head->className, 
							head->s.origin.x,
							head->s.origin.y,
							head->s.origin.z );
			}
            bItem = TRUE;
		}

        if ( bItem )
        {
            head->nSidekickFlag = (SUPERFLY_NO_PICKUP | MIKIKO_NO_PICKUP);
            ITEM_ParseEpairs( head );    
        }
        
        head = gstate->NextEntity(head);
    }

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        ITEM_IsVisible
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int ITEM_IsVisible( userEntity_t *self )
{
	if ( self->s.modelindex == 0 || self->solid == SOLID_NOT ||
         _stricmp( self->className, "freed" ) == 0 || _stricmp( self->className, "noclass" ) == 0 )
	{
		return FALSE;
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		ITEM_GetClosestWeapon
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *ITEM_GetClosestWeapon( userEntity_t *self, const char *szItemName )
{
    _ASSERTE( self );
	
	float fMinDistance = 1e20;

	userEntity_t *pClosestWeapon = NULL;
	POSITION pos = pWeaponList->GetHeadPosition();
    while (pos)
    {
		userEntity_t *pWeapon = (userEntity_t*)pWeaponList->GetNext(pos);
		_ASSERTE( pWeapon );

		if ( ITEM_IsVisible( pWeapon ) &&
			 _stricmp( pWeapon->className, szItemName ) == 0 )
		{
			float fDistance = VectorDistance( self->s.origin, pWeapon->s.origin );
			if ( fDistance < fMinDistance )
			{
				fMinDistance = fDistance;

				pClosestWeapon = pWeapon;
			}
		}
	}

	return pClosestWeapon;
}

// ----------------------------------------------------------------------------
//
// Name:		ITEM_GetClosestAmmo
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *ITEM_GetClosestAmmo( userEntity_t *self, const char *szItemName )
{
	_ASSERTE( self );
	_ASSERTE( pAmmoList );

    float fMinDistance = 1e20;

	userEntity_t *pClosestAmmo = NULL;
	POSITION pos = pAmmoList->GetHeadPosition();
    while (pos)
    {
		userEntity_t *pAmmo = (userEntity_t*)pAmmoList->GetNext(pos);
		_ASSERTE( pAmmo );

		if ( (deathmatch->value || ITEM_IsVisible(pAmmo)) && 
			 _stricmp( pAmmo->className, szItemName ) == 0 )
		{
			float fDistance = VectorDistance( self->s.origin, pAmmo->s.origin );
			if ( fDistance < fMinDistance )
			{
				fMinDistance = fDistance;

				pClosestAmmo = pAmmo;
			}
		}
	}

	return pClosestAmmo;
}

// ----------------------------------------------------------------------------
//
// Name:		ITEM_GetClosestArmor
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *ITEM_GetClosestArmor( userEntity_t *self )
{
    _ASSERTE( self );
	_ASSERTE( pArmorList );

	float fMinDistance = 1e20;

	userEntity_t *pClosestArmor = NULL;
	POSITION pos = pArmorList->GetHeadPosition();
    while (pos)
    {
		userEntity_t *pArmor = (userEntity_t*)pArmorList->GetNext(pos);
		_ASSERTE( pArmor );

		if ( deathmatch->value || ITEM_IsVisible(pArmor) )
		{
			float fDistance = VectorDistance( self->s.origin, pArmor->s.origin );
			if ( fDistance < fMinDistance )
			{
				fMinDistance = fDistance;

				pClosestArmor = pArmor;
			}
		}
	}

	return pClosestArmor;
}

// ----------------------------------------------------------------------------
//
// Name:		ITEM_GetClosestArmor
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *ITEM_GetClosestArmor( userEntity_t *self, const char *szItemName )
{
    _ASSERTE( self );
	_ASSERTE( pArmorList );

	float fMinDistance = 1e20;

	userEntity_t *pClosestArmor = NULL;
	POSITION pos = pArmorList->GetHeadPosition();
    while (pos)
    {
		userEntity_t *pArmor = (userEntity_t*)pArmorList->GetNext(pos);
		_ASSERTE( pArmor );

		if ( (deathmatch->value || ITEM_IsVisible(pArmor)) && 
			 _stricmp( pArmor->className, szItemName ) == 0 )
		{
			float fDistance = VectorDistance( self->s.origin, pArmor->s.origin );
			if ( fDistance < fMinDistance )
			{
				fMinDistance = fDistance;

				pClosestArmor = pArmor;
			}
		}
	}

	return pClosestArmor;
}

// ----------------------------------------------------------------------------
//
// Name:		ITEM_GetClosestHealth
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *ITEM_GetClosestHealth( userEntity_t *self )
{
	_ASSERTE( self );
	_ASSERTE( pHealthList );

    float fMinDistance = 1e20;

	userEntity_t *pClosestHealth = NULL;
	POSITION pos = pHealthList->GetHeadPosition();
    while (pos)
    {
		userEntity_t *pHealth = (userEntity_t*)pHealthList->GetNext(pos);
		if ( deathmatch->value || ITEM_IsVisible(pHealth) )
		{
			float fDistance = VectorDistance( self->s.origin, pHealth->s.origin );
			if ( fDistance < fMinDistance )
			{
				fMinDistance = fDistance;

				pClosestHealth = pHealth;
			}
		}
	}

	return pClosestHealth;
}

// ----------------------------------------------------------------------------
//
// Name:		ITEM_GetClosestStatBoost
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *ITEM_GetClosestStatBoost( userEntity_t *self, const char *szItemName )
{
    _ASSERTE( self );
	_ASSERTE( pStatBoostList );

	float fMinDistance = 1e20;

	userEntity_t *pClosestStatBoost = NULL;
	POSITION pos = pStatBoostList->GetHeadPosition();
    while (pos)
    {
		userEntity_t *pStatBoost = (userEntity_t*)pStatBoostList->GetNext(pos);
		_ASSERTE( pStatBoost );

		if ( (deathmatch->value || ITEM_IsVisible(pStatBoost)) && 
			 _stricmp( pStatBoost->className, szItemName ) == 0 )
		{
			float fDistance = VectorDistance( self->s.origin, pStatBoost->s.origin );
			if ( fDistance < fMinDistance )
			{
				fMinDistance = fDistance;

				pClosestStatBoost = pStatBoost;
			}
		}
	}

	return pClosestStatBoost;
}

// ----------------------------------------------------------------------------
//
// Name:		ITEM_GetClosestGoldenSoul
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *ITEM_GetClosestGoldenSoul( userEntity_t *self )
{
    _ASSERTE( self );
	_ASSERTE( pGoldenSoulList );

	float fMinDistance = 1e20;

	userEntity_t *pClosestGoldenSoul = NULL;
	POSITION pos = pGoldenSoulList->GetHeadPosition();
    while (pos)
    {
		userEntity_t *pGoldenSoul = (userEntity_t*)pGoldenSoulList->GetNext(pos);

		if ( deathmatch->value || ITEM_IsVisible(pGoldenSoul) )
		{
			float fDistance = VectorDistance( self->s.origin, pGoldenSoul->s.origin );
			if ( fDistance < fMinDistance )
			{
				fMinDistance = fDistance;

				pClosestGoldenSoul = pGoldenSoul;
			}
		}
	}

	return pClosestGoldenSoul;
}

// ----------------------------------------------------------------------------
//
// Name:		ITEM_GetNextClosestVisibleItem
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *ITEM_GetNextClosestVisibleItem( userEntity_t *self, const CVector &fromLocation, 
                        float fDistanceBetweenItems /* = DISTANCE_BETWEEN_ITEMS */ )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	userEntity_t *pClosestItem = NULL;

	float fMinDistance = 512.0f;

	POSITION pos = pWeaponList->GetHeadPosition();
    while (pos)
    {
		userEntity_t *pItem = (userEntity_t*)pWeaponList->GetNext(pos);
		if ( ITEM_IsVisible(pItem) && gstate->inPVS(self->s.origin, pItem->s.origin) )
		{
			weapon_t *pWeapon = (weapon_t *)gstate->InventoryFindItem( self->inventory, pItem->className );
			if ( !pWeapon )
			{
				// item is visible
				float fDistance = VectorDistance( self->s.origin, pItem->s.origin );
				float fDistanceToFromLocation = VectorDistance( fromLocation, pItem->s.origin );
				if ( fDistance < fMinDistance && fDistanceToFromLocation >= fDistanceBetweenItems )
				{
					tr = gstate->TraceLine_q2( self->s.origin, pItem->s.origin, self, MASK_SOLID );
					if ( tr.fraction >= 1.0f )
					{
						pClosestItem = pItem;
						fMinDistance = fDistance;
					}
				}
			}
		}
	}

	pos = pAmmoList->GetHeadPosition();
    while (pos)
    {
		userEntity_t *pItem = (userEntity_t*)pAmmoList->GetNext(pos);

		if ( ITEM_IsVisible(pItem) && gstate->inPVS(self->s.origin, pItem->s.origin) )
		{
			int bPickupAmmo = FALSE;
			ammo_t *pAmmo = (ammo_t*)gstate->InventoryFindItem( self->inventory, pItem->className );
			if ( pAmmo )
			{
				weaponInfo_t *pWeaponInfo = pAmmo->winfo;
				if ( pAmmo->count < pWeaponInfo->ammo_max )
				{
					bPickupAmmo = TRUE;
				}
			}
			else
			{
				bPickupAmmo = TRUE;
			}

			if ( bPickupAmmo )
			{
				// item is visible
				float fDistance = VectorDistance( self->s.origin, pItem->s.origin );
				float fDistanceToFromLocation = VectorDistance( fromLocation, pItem->s.origin );
				if ( fDistance < fMinDistance && fDistanceToFromLocation >= fDistanceBetweenItems )
				{
					tr = gstate->TraceLine_q2( self->s.origin, pItem->s.origin, self, MASK_SOLID );
					if ( tr.fraction >= 1.0f )
					{
						pClosestItem = pItem;
						fMinDistance = fDistance;
					}
				}
			}
		}
	}

	if ( self->armor_val < 100 )
	{
		pos = pArmorList->GetHeadPosition();
		while (pos)
		{
			userEntity_t *pItem = (userEntity_t*)pArmorList->GetNext(pos);
			if ( ITEM_IsVisible(pItem) && gstate->inPVS(self->s.origin, pItem->s.origin) )
			{
				// item is visible
				float fDistance = VectorDistance( self->s.origin, pItem->s.origin );
				float fDistanceToFromLocation = VectorDistance( fromLocation, pItem->s.origin );
				if ( fDistance < fMinDistance && fDistanceToFromLocation >= fDistanceBetweenItems )
				{
					tr = gstate->TraceLine_q2( self->s.origin, pItem->s.origin, self, MASK_SOLID );
					if ( tr.fraction >= 1.0f )
					{
						pClosestItem = pItem;
						fMinDistance = fDistance;
					}
				}
			}
		}
	}

	if ( self->health < 100 )
	{
		pos = pHealthList->GetHeadPosition();
		while (pos)
		{
			userEntity_t *pItem = (userEntity_t*)pHealthList->GetNext(pos);
			if ( ITEM_IsVisible(pItem) && gstate->inPVS(self->s.origin, pItem->s.origin) )
			{
				// item is visible
				float fDistance = VectorDistance( self->s.origin, pItem->s.origin );
				float fDistanceToFromLocation = VectorDistance( fromLocation, pItem->s.origin );
				if ( fDistance < fMinDistance && fDistanceToFromLocation >= fDistanceBetweenItems )
				{
					tr = gstate->TraceLine_q2( self->s.origin, pItem->s.origin, self, MASK_SOLID );
					if ( tr.fraction >= 1.0f )
					{
						pClosestItem = pItem;
						fMinDistance = fDistance;
					}
				}
			}
		}
	}

	pos = pStatBoostList->GetHeadPosition();
    while (pos)
    {
		userEntity_t *pItem = (userEntity_t*)pStatBoostList->GetNext(pos);
		if ( ITEM_IsVisible(pItem) && gstate->inPVS(self->s.origin, pItem->s.origin) )
		{
			// item is visible
			// now check if the boost was already picked up
			if ( (_stricmp( pItem->className, "item_power_boost" ) == 0 && !(hook->items & IT_POWERBOOST)) ||
				 (_stricmp( pItem->className, "item_attack_boost" ) == 0 && !(hook->items & IT_ATTACKBOOST)) ||
				 (_stricmp( pItem->className, "item_speed_boost" ) == 0 && !(hook->items & IT_SPEEDBOOST)) ||
				 (_stricmp( pItem->className, "item_acro_boost" ) == 0 && !(hook->items & IT_ACROBOOST)) ||
				 (_stricmp( pItem->className, "item_vita_boost" ) == 0 && !(hook->items & IT_VITABOOST)) )
			{
				float fDistance = VectorDistance( self->s.origin, pItem->s.origin );
				float fDistanceToFromLocation = VectorDistance( fromLocation, pItem->s.origin );
				if ( fDistance < fMinDistance && fDistanceToFromLocation >= fDistanceBetweenItems )
				{
					tr = gstate->TraceLine_q2( self->s.origin, pItem->s.origin, self, MASK_SOLID );
					if ( tr.fraction >= 1.0f )
					{
						pClosestItem = pItem;
						fMinDistance = fDistance;
					}
				}
			}
		}
	}

	if ( self->health < hook->base_health )
	{
		pos = pGoldenSoulList->GetHeadPosition();
		while (pos)
		{
			userEntity_t *pItem = (userEntity_t*)pGoldenSoulList->GetNext(pos);
			if ( ITEM_IsVisible(pItem) && gstate->inPVS(self->s.origin, pItem->s.origin) )
			{
				// item is visible
				float fDistance = VectorDistance( self->s.origin, pItem->s.origin );
				float fDistanceToFromLocation = VectorDistance( fromLocation, pItem->s.origin );
				if ( fDistance < fMinDistance && fDistanceToFromLocation >= fDistanceBetweenItems )
				{
					tr = gstate->TraceLine_q2( self->s.origin, pItem->s.origin, self, MASK_SOLID );
					if ( tr.fraction >= 1.0f )
					{
						pClosestItem = pItem;
						fMinDistance = fDistance;
					}
				}
			}
		}
	}

	return pClosestItem;
}

// ----------------------------------------------------------------------------
//
// Name:        ITEM_RemoveAll
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ITEM_RemoveAll()
{
    // clean if necessary
	ITEM_Destroy();
	
	userEntity_t *head = gstate->FirstEntity();
    while ( head )
    {
        if ( strstr( head->className, "weapon" ) ||
             strstr( head->className, "ammo" ) ||
		     strstr( head->className, "item_" ) )
		{
			head->remove( head );
        }

        head = gstate->NextEntity(head);
    }
}

// ----------------------------------------------------------------------------
//
// Name:        ITEM_IsOkToPickup
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int ITEM_IsOkToPickup( userEntity_t *self, userEntity_t *pItem )
{
	if ( strstr( self->className, "Superfly" ) ||
         strstr( self->className, "Mikiko" ) )
    {
        return SIDEKICK_CanPickupItem( self, pItem );
    }

    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        ITEM_SetNoPickup
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ITEM_SetNoPickup( userEntity_t *self, userEntity_t *pItem )
{
    if ( strstr( self->className, "superfly" ) )
    {
        pItem->nSidekickFlag |= SUPERFLY_NO_PICKUP;
    }
    else
    if ( strstr( self->className, "mikiko" ) )
    {
        pItem->nSidekickFlag |= MIKIKO_NO_PICKUP;
    }
}



// ----------------------------------------------------------------------------
//                      SIDEKICK specific functions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_GetClosestVisibleWeaponNotInInventory
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *SIDEKICK_GetClosestVisibleWeaponNotInInventory( userEntity_t *self, float fWithinDistance )
{
	_ASSERTE( self );
	_ASSERTE( pWeaponList );

    float fMinDistance = 1e20;

	userEntity_t *pClosestWeapon = NULL;
	POSITION pos = pWeaponList->GetHeadPosition();
    while (pos)
    {
		userEntity_t *pWeapon = (userEntity_t*)pWeaponList->GetNext(pos);
		tr = gstate->TraceLine_q2( self->s.origin, pWeapon->s.origin, self, MASK_SOLID );
		/*gstate->inPVS(self->s.origin, pWeapon->s.origin)*/
		if ( ITEM_IsVisible( pWeapon ) && 
              tr.fraction >= 0.65f &&
             ITEM_IsOkToPickup( self, pWeapon ) && 
             !gstate->InventoryFindItem( self->inventory, pWeapon->className ) )
		{
			float fDistance = VectorDistance( self->s.origin, pWeapon->s.origin );
			if ( fDistance < fMinDistance && fDistance < fWithinDistance )
			{
				tr = gstate->TraceLine_q2( self->s.origin, pWeapon->s.origin, self, MASK_SOLID );
				//tr = gstate->TraceBox_q2( self->s.origin, self->s.mins,self->s.maxs,  pWeapon->s.origin, self, MASK_SOLID);
				if ( tr.fraction >= 1.0f )
				{
				    fMinDistance = fDistance;

				    pClosestWeapon = pWeapon;
			    }
            }
		}
	}

	return pClosestWeapon;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_GetClosestVisibleAmmoNotInInventory
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *SIDEKICK_GetClosestVisibleAmmoNotInInventory( userEntity_t *self, float fWithinDistance )
{
	_ASSERTE( self );
	_ASSERTE( pAmmoList );

    float fMinDistance = 1e20;

	userEntity_t *pClosestAmmo = NULL;
	POSITION pos = pAmmoList->GetHeadPosition();
    while ( pos )
    {
		userEntity_t *pAmmo = (userEntity_t*)pAmmoList->GetNext(pos);
		if ( ITEM_IsVisible( pAmmo ) && 
             gstate->inPVS(self->s.origin, pAmmo->s.origin) &&
             ITEM_IsOkToPickup( self, pAmmo ) /*&& 
             !gstate->InventoryFindItem( self->inventory, pAmmo->className )*/ )
		{
			float fDistance = VectorDistance( self->s.origin, pAmmo->s.origin );
			if ( fDistance < fMinDistance && fDistance < fWithinDistance )
			{
				tr = gstate->TraceLine_q2( self->s.origin, pAmmo->s.origin, self, MASK_SOLID );
				//tr = gstate->TraceBox_q2( self->s.origin, self->s.mins,self->s.maxs,  pAmmo->s.origin, self, MASK_SOLID);
				if ( tr.fraction >= 1.0f )
				{
				    fMinDistance = fDistance;

				    pClosestAmmo = pAmmo;
			    }
            }
		}
	}

	return pClosestAmmo;
}



// ----------------------------------------------------------------------------
// NSS[1/11/00]:
// Name:		SIDEKICK_RemoveItem
// Description: If the item is part of the list remove it from the list.
// Input:userEntity_t *item
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_RemoveItem(userEntity_t *Item)
{
	CPtrList *List = NULL;
	POSITION pos;

	if ( strstr( Item->className, "weapon" ) )
	{
		List = pWeaponList;
	}
	else
	if ( strstr( Item->className, "ammo" ) )
	{
		List = pAmmoList;
	}
	else
	if (strstr( Item->className, "armor")) 
	{
		List = 	pArmorList;
	}
	else
	if ( _stricmp( Item->className, "item_power_boost" ) == 0 ||
		 _stricmp( Item->className, "item_acro_boost" ) == 0 ||
		 _stricmp( Item->className, "item_attack_boost" ) == 0 ||
		 _stricmp( Item->className, "item_speed_boost" ) == 0 ||
		 _stricmp( Item->className, "item_vita_boost" ) == 0 )
	{
		List = pStatBoostList;
	}
	else
	if ( strstr( Item->className, "item_health" ) )
	{
		List = pHealthList;
	}
	else
	if ( _stricmp( Item->className, "item_goldensoul" ) == 0 )
	{
		List = pGoldenSoulList;
	}
	
	// NSS[1/11/00]:If it actually was found as a valid item.
	if(List)
	{
		pos = List->Find(Item,List->GetHeadPosition());
		if(pos)
		{
			List->RemoveAt(pos);

		}
	}

}



// ----------------------------------------------------------------------------
// NSS[1/10/00]:Consolidating like 10 functions to 2
// Name:		SIDEKICK_GetEntityPos
// Description:Gets the first item of its type from the list
// Input:long Item_Type, POSITION &pos
// Output:userEntity_t *
// Note:NextFirst --->0=(get first...when you first start) 1=(get next item)
//
// ----------------------------------------------------------------------------
userEntity_t *SIDEKICK_GetEntityPos(long Item_Type, POSITION &pos, int NextFirst)
{
	CPtrList	*List;
	userEntity_t *ent = NULL;
	//POSITION pos = PositionNow;
	
	// NSS[1/11/00]:Get the right list pointer
	switch (Item_Type)
	{
		case ITEMTYPE_ARMOR:
		{
			List = pArmorList;
			break;
		}
		case ITEMTYPE_HEALTH:
		{
			List = pHealthList;
			break;
		}
		case ITEMTYPE_AMMO:
		{
			List = pAmmoList;
			break;
		}
		case ITEMTYPE_GOLDENSOUL:
		{
			List = pGoldenSoulList;
			break;
		}
		case ITEMTYPE_STATBOOST:
		{
			List = pStatBoostList;
			break;
		}
		case ITEMTYPE_WEAPONS:
		{
			List = pWeaponList;
			break;
		}
		default :
		{
			AI_Dprintf("No Item Type Defined.\n");
			return NULL;
		}
	}
	
	// NSS[1/11/00]:Now let's get the information from the list
	if(!NextFirst)
	{
		pos = List->GetHeadPosition();
	}
	if(pos)
	{
		ent = (userEntity_t*)List->GetNext(pos);
	}

	// NSS[1/10/00]:If pos is 0 then we are at the end of the list(no mo).
	if(!pos)
	{
		return NULL;
	}
	
	return ent;
}

float SIDEKICK_DeterminePathLength(userEntity_t *self,userEntity_t *target);
int AI_FindPathToEntity( userEntity_t *self, userEntity_t *destEntity, int bTestCanPath /* = TRUE */ );
// ----------------------------------------------------------------------------
// NSS[1/10/00]:Consolidating like 10 functions to 2
// Name:		SIDEKICK_GetClosestVisibleItem
// Description:Scans for the closest visible item type and if found returns a pointer to it.
// Input:userEntity_t *self, const char *szItemName, float fWithinDistance , long Item_Type
// Output:userEntity_t *
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *SIDEKICK_GetClosestVisibleItem( userEntity_t *self, const char *szItemName, float fWithinDistance , long Item_Type)
{
	_ASSERTE( self );
	_ASSERTE( pStatBoostList );

    float fMinDistance	= 1e20;
	float fDistance		= 0.0f;

	POSITION pos		= NULL;

	userEntity_t *ent			= SIDEKICK_GetEntityPos(Item_Type,pos,0);
	userEntity_t *pClosestEnt	= NULL;

	

	while (ent)
    {
		if ( ITEM_IsVisible( ent ) && AI_IsVisible(self,ent) && ITEM_IsOkToPickup( self, ent ))
		{
			if(szItemName!=NULL)
			{
				if(_stricmp( ent->className, szItemName ) != 0)
				{
					// NSS[1/10/00]:Keep on getting the entities until we are at the end of the list.
					ent	= SIDEKICK_GetEntityPos(Item_Type,pos,1);
					continue;
				}
			}
			
			fDistance = VectorDistance( self->s.origin, ent->s.origin );
			if ( fDistance < fMinDistance) //&& fDistance < fWithinDistance )
			{
				tr = gstate->TraceBox_q2( self->s.origin, self->s.mins,self->s.maxs,  ent->s.origin, self, MASK_SOLID);
				AI_FindPathToEntity(self,ent,TRUE);

				if ( tr.fraction >= 1.0f && SIDEKICK_DeterminePathLength(self,ent) < 300.0f)
				{
				    fMinDistance = fDistance;

				    pClosestEnt = ent;
			    }
            }
		}
		// NSS[1/10/00]:Keep on getting the entities until we are at the end of the list.
		ent	= SIDEKICK_GetEntityPos(Item_Type,pos,1);
	}
	return pClosestEnt;
}






// ----------------------------------------------------------------------------
//
// Name:		ITEM_IsItem
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int ITEM_IsItem( userEntity_t *self )
{
    if ( strstr( self->className, "weapon" ) ||
         strstr( self->className, "ammo" ) ||
		 strstr( self->className, "item_" ) || (strstr(self->className, "misc_")) )
	{
		return TRUE;
    }

    return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		ITEM_IsAmmo
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int ITEM_IsAmmo( userEntity_t *self )
{
	_ASSERTE( self );

    if ( strstr( self->className, "ammo" ) )
    {
        return TRUE;
    }

    return FALSE;
}

int ITEM_IsHealth( userEntity_t *self )
{
	_ASSERTE( self );

	if ( strstr( self->className, "item_health" ) ||
         strstr( self->className, "item_goldensoul" ) )
    {
        return TRUE;
    }

    return FALSE;
}

int ITEM_IsWeapon( userEntity_t *self )
{
	_ASSERTE( self );

    if ( strstr( self->className, "weapon" ) )
    {
        return TRUE;
    }

    return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_IsFacingEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int ITEM_IsFacingItem( userEntity_t *self, userEntity_t *pItem )
{

	if(self != NULL && pItem != NULL && AI_IsVisible(self,pItem))
	{
		CVector Start = self->s.origin;
		Start.z += 22.0f;
		CVector dir = pItem->s.origin - Start;
		dir.Normalize();
		CVector angleTowardItem;
		VectorToAngles( dir, angleTowardItem );

		//float fTowardItemY = AngleMod(angleTowardItem.y);
		//float fFacingAngleY = AngleMod(self->s.angles.yaw);
		float fTowardItemY = AngleMod(angleTowardItem.y);
		float fFacingAngleY = AngleMod(self->client->ps.viewangles.yaw);

		float fYawDiff = fabs( fTowardItemY - fFacingAngleY );
		if ( fYawDiff < 5.0f || fYawDiff > 355.0f )
		{
			//float fTowardItemX = AngleMod(angleTowardItem.x);
			//float fFacingAngleX = AngleMod(self->s.angles.pitch);
			float fTowardItemX = AngleMod(angleTowardItem.x);
			float fFacingAngleX = AngleMod(self->client->ps.viewangles.pitch);
		
			float fPitchDiff = fabs( fTowardItemX - fFacingAngleX );
			if ( fPitchDiff < 20.0f || fPitchDiff > 340.0f && AI_IsVisible(self,pItem))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
void SIDEKICK_Drop_Markers (CVector &Target, float delay);

// ----------------------------------------------------------------------------
// NSS[2/29/00]:
// Name:		ITEM_GetTargetedItem
// Description: Would be nice to have had this here awhile back
// Input: userEntity_t *self
// Output:NA
// Note:I wish we could trace to items... perhaps later... :(
// ----------------------------------------------------------------------------
userEntity_t *ITEM_GetItemOwnerIsPointingAt(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	CVector Dir, Destination,Start;
	if(hook)
	{
		if(hook->owner)
		{
			Start = hook->owner->s.origin;
			Start.z += 22.0f;
			//Get the view angles
			hook->owner->client->ps.viewangles.AngleToForwardVector(Dir);
			Destination = Start + 800*Dir;
			tr = gstate->TraceLineTrigger( Start, Destination, hook->owner, MASK_ALL, SVF_ITEM );
			// If we have hit something marked as an item then return it as the entity
			SIDEKICK_Drop_Markers((Start + (Dir *(800*tr.fraction))),1.25f);
			if(tr.fraction < 1.0f && (tr.ent->flags & FL_ITEM))
			{
				return tr.ent;
			}
			else
			{
				tr = gstate->TraceLine_q2( Start, Destination, hook->owner, MASK_ALL);
				if(tr.ent && ((!_stricmp(tr.ent->className,"misc_healthtree")) || (!_stricmp(tr.ent->className,"misc_lifewater")) || (!_stricmp(tr.ent->className,"misc_hosportal"))) )
				{
					return tr.ent;
				}
			}
		}
	}
	return NULL;
}

// ----------------------------------------------------------------------------
// NSS[2/29/00]:
// Name:		ITEM_ParseThroughItemArray
// Description:Ok how about we have 1 function to do this ???
// Input:CPtrList *pThingList, userEntity_t *object
// Output:userEntity_t *<entity>
// Note:Oh wow... 
//
// ----------------------------------------------------------------------------
userEntity_t *ITEM_ParseThroughItemArray(CPtrList *pThingList,userEntity_t *object)
{
	POSITION pos = pThingList->GetHeadPosition();
	while (pos)
	{
		userEntity_t *pItem = (userEntity_t*)pThingList->GetNext(pos);
		//userEntity_t *pItem = ITEM_GetItemOwnerIsPointingAt(self);
		if (ITEM_IsVisible(pItem))
		{
			// NSS[2/29/00]:Damned items are fucking SOLID_TRIGGER!!! ARRRG!
			if(object == pItem)
			{
				return pItem;
			}
		}
	}
	return NULL;
}

// ----------------------------------------------------------------------------
// NSS[2/29/00]:
// Name:		ITEM_GetTargetedItem
// Description:Ok look down below.. then look down below this function... a difference?
// Input: userEntity_t *self
// Output:userEntity_t *self
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *ITEM_GetTargetedItem( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	float fMinDistance = 1024.0f;

	userEntity_t *object = ITEM_GetItemOwnerIsPointingAt(self);
	userEntity_t *pItem  = NULL;
	
	if(object != NULL)
	{
		if(hook && hook->owner)
		{
			// NSS[2/29/00]:	CHECK FOR HEALTH
			pItem = ITEM_ParseThroughItemArray(pHealthList,object);
			if(pItem != NULL)
				return pItem;
			// NSS[2/29/00]:	CHECK FOR ARMOR
			pItem = ITEM_ParseThroughItemArray(pArmorList,object);
			if(pItem != NULL)
				return pItem;
			// NSS[2/29/00]:	CHECK FOR WEAPONS
			pItem = ITEM_ParseThroughItemArray(pWeaponList,object);
			if(pItem != NULL)
				return pItem;
			// NSS[2/29/00]:	CHECK FOR AMMO
			pItem = ITEM_ParseThroughItemArray(pAmmoList,object);
			if(pItem != NULL)
				return pItem;
			// NSS[2/29/00]:	CHECK FOR GOLDENSOUL
			pItem = ITEM_ParseThroughItemArray(pGoldenSoulList,object);

			// NSS[5/19/00]:	CHECK FOR HOSPORTAL OR FOUNTAIN
			if(((!_stricmp(object->className,"misc_healthtree")) || (!_stricmp(object->className,"misc_lifewater")) || (!_stricmp(object->className,"misc_hosportal"))))
			{
				pItem = object;
			}
		}
	}
	return pItem;
}

/*// NSS[2/29/00]:Let's try this again shall we?
// ----------------------------------------------------------------------------
//
// Name:		ITEM_GetTargetedItem
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *ITEM_GetTargetedItem( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	float fMinDistance = 1024.0f;

	POSITION pos = pHealthList->GetHeadPosition();
	while (pos)
	{
		userEntity_t *pItem = (userEntity_t*)pHealthList->GetNext(pos);
		if ( ITEM_IsVisible(pItem) && gstate->inPVS(self->s.origin, pItem->s.origin) )
		{
			// item is visible
			float fDistance = VectorDistance( self->s.origin, pItem->s.origin );
			if ( fDistance < fMinDistance && ITEM_IsFacingItem( self, pItem ) )
			{
				return pItem;
			}
		}
	}

	pos = pArmorList->GetHeadPosition();
	while (pos)
	{
		userEntity_t *pItem = (userEntity_t*)pArmorList->GetNext(pos);
		if ( ITEM_IsVisible(pItem) && gstate->inPVS(self->s.origin, pItem->s.origin) )
		{
			// item is visible
			float fDistance = VectorDistance( self->s.origin, pItem->s.origin );
			if ( fDistance < fMinDistance && ITEM_IsFacingItem( self, pItem ) )
			{
				return pItem;
			}
		}
	}

	pos = pWeaponList->GetHeadPosition();
    while (pos)
    {
		userEntity_t *pItem = (userEntity_t*)pWeaponList->GetNext(pos);
		if ( ITEM_IsVisible(pItem) && gstate->inPVS(self->s.origin, pItem->s.origin) )
		{
			// item is visible
			float fDistance = VectorDistance( self->s.origin, pItem->s.origin );
			if ( fDistance < fMinDistance && ITEM_IsFacingItem( self, pItem ) )
			{
				return pItem;
			}
		}
	}

	pos = pAmmoList->GetHeadPosition();
    while (pos)
    {
		userEntity_t *pItem = (userEntity_t*)pAmmoList->GetNext(pos);
		if ( ITEM_IsVisible(pItem) && gstate->inPVS(self->s.origin, pItem->s.origin) )
		{
			// item is visible
			float fDistance = VectorDistance( self->s.origin, pItem->s.origin );
			if ( fDistance < fMinDistance && ITEM_IsFacingItem( self, pItem ) )
			{
				return pItem;
			}
		}
	}

	pos = pStatBoostList->GetHeadPosition();
    while (pos)
    {
		userEntity_t *pItem = (userEntity_t*)pStatBoostList->GetNext(pos);
		if ( ITEM_IsVisible(pItem) && gstate->inPVS(self->s.origin, pItem->s.origin) )
		{
			// item is visible
			float fDistance = VectorDistance( self->s.origin, pItem->s.origin );
			if ( fDistance < fMinDistance && ITEM_IsFacingItem( self, pItem ) )
			{
				return pItem;
			}
		}
	}

	pos = pGoldenSoulList->GetHeadPosition();
	while (pos)
	{
		userEntity_t *pItem = (userEntity_t*)pGoldenSoulList->GetNext(pos);
		if ( ITEM_IsVisible(pItem) && gstate->inPVS(self->s.origin, pItem->s.origin) )
		{
			// item is visible
			float fDistance = VectorDistance( self->s.origin, pItem->s.origin );
			if ( fDistance < fMinDistance && ITEM_IsFacingItem( self, pItem ) )
			{
				return pItem;
			}
		}
	}

	return NULL;
}
*/
// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
