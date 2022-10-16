#ifndef _ITEMS_H
#define _ITEMS_H

/* these are conflicting defines of enums in p_user.h
// E1 weapons
#define WEAPON_DISRUPTOR		1
#define WEAPON_IONBLASTER		2
#define WEAPON_C4				3
#define WEAPON_SHOTCYCLER		4
#define WEAPON_SIDEWINDER		5
#define WEAPON_SHOCKWAVE		6

// E2
#define WEAPON_DISCUS			11
#define WEAPON_SUNFLARES		12
#define WEAPON_VENOMOUS			13
#define WEAPON_TRIDENT			14
#define WEAPON_MIDAS			15
#define WEAPON_EYEOFZEUS		16
#define WEAPON_HAMMER			17

// E3
#define WEAPON_SILVERCLAW		21
#define WEAPON_BOLTER			22
#define WEAPON_STAVROSSTAVE		23
#define WEAPON_BALLISTA			24
#define WEAPON_WISP				25
#define WEAPON_NIGHTMARE		26

// E4
#define WEAPON_GLOCK			31
#define WEAPON_RIPGUN			32
#define WEAPON_SLUGGER			33
#define WEAPON_KINETICORE		34
#define WEAPON_NOVABEAM			35
#define WEAPON_METAMASER		36
*/
#define HEALTH_CAN				41
#define HEALTH_VASE				42
#define HEALTH_WOODENCRATE		43
#define HEALTH_MEDIKIT			44

#define ARMOR_PLASTEEL			45
#define ARMOR_CHROMATIC			46
#define ARMOR_SILVER			47
#define ARMOR_GOLD				48
#define ARMOR_CHAINMAIL			49
#define ARMOR_BLACKADAMANT		50
#define ARMOR_KEVLAR			51
#define ARMOR_EBONITE			52

#define STATBOOST_ATTACKPOWER	53
#define STATBOOST_ATTACKSPEED	54
#define STATBOOST_MAXHEALTH		55
#define STATBOOST_JUMPING		56
#define STATBOOST_SPEED			57

#define WRAITHORB				60
#define MEGASHIELD				61
#define GOLDENSOUL				62
#define ANTIDOTE				63

#define WOODEN_CHEST			64
#define BLACK_CHEST				65

////////////////////////////////////////////////////////////////////////////
//	ITEM Spawn flags
////////////////////////////////////////////////////////////////////////////
#define ITEMSPAWN_NONE			0x00000000
#define ITEMSPAWN_SUPERFLY  	0x00000001
#define ITEMSPAWN_MIKIKO		0x00000002

// ----------------------------------------------------------------------------

#define DISTANCE_BETWEEN_ITEMS		32.0f

void ITEM_Destroy();
qboolean ITEM_Initialize();
int ITEM_IsVisible( userEntity_t *self );
userEntity_t *ITEM_GetClosestWeapon( userEntity_t *self, const char *szItemName );
userEntity_t *ITEM_GetClosestAmmo( userEntity_t *self, const char *szItemName );
userEntity_t *ITEM_GetClosestArmor( userEntity_t *self );
userEntity_t *ITEM_GetClosestArmor( userEntity_t *self, const char *szItemName );
userEntity_t *ITEM_GetClosestHealth( userEntity_t *self );
userEntity_t *ITEM_GetClosestStatBoost( userEntity_t *self, const char *szItemName );
userEntity_t *ITEM_GetClosestGoldenSoul( userEntity_t *self );
userEntity_t *ITEM_GetNextClosestVisibleItem( userEntity_t *self );
userEntity_t *ITEM_GetNextClosestVisibleItem( userEntity_t *self, const CVector &fromLocation, float fDistance = DISTANCE_BETWEEN_ITEMS );
void ITEM_RemoveAll();

int ITEM_IsOkToPickup( userEntity_t *self, userEntity_t *pItem );
void ITEM_SetNoPickup( userEntity_t *self, userEntity_t *pItem );



#define SIDEKICK_ITEM_SEARCH_DISTANCE    512.0f
userEntity_t *SIDEKICK_GetClosestVisibleItem( userEntity_t *self, const char *szItemName, float fWithinDistance , long Item_Type);

//userEntity_t *SIDEKICK_GetClosestVisibleHealth( userEntity_t *self, float fWithinDistance = SIDEKICK_ITEM_SEARCH_DISTANCE );
//userEntity_t *SIDEKICK_GetClosestVisibleWeapon( userEntity_t *self, const char *szItemName, float fWithinDistance = SIDEKICK_ITEM_SEARCH_DISTANCE );
//userEntity_t *SIDEKICK_GetClosestVisibleWeapon( userEntity_t *self, float fWithinDistance = SIDEKICK_ITEM_SEARCH_DISTANCE );
userEntity_t *SIDEKICK_GetClosestVisibleWeaponNotInInventory( userEntity_t *self, float fWithinDistance = SIDEKICK_ITEM_SEARCH_DISTANCE );
userEntity_t *SIDEKICK_GetClosestVisibleAmmoNotInInventory( userEntity_t *self, float fWithinDistance = SIDEKICK_ITEM_SEARCH_DISTANCE );
//userEntity_t *SIDEKICK_GetClosestVisibleGoldenSoul( userEntity_t *self, float fWithinDistance = SIDEKICK_ITEM_SEARCH_DISTANCE );
//userEntity_t *SIDEKICK_GetClosestVisibleArmor( userEntity_t *self, float fWithinDistance = SIDEKICK_ITEM_SEARCH_DISTANCE );
//userEntity_t *SIDEKICK_GetClosestVisibleArmor( userEntity_t *self, const char *szItemName, float fWithinDistance = SIDEKICK_ITEM_SEARCH_DISTANCE );
//userEntity_t *SIDEKICK_GetClosestVisibleStatBoost( userEntity_t *self, const char *szItemName, float fWithinDistance = SIDEKICK_ITEM_SEARCH_DISTANCE );

int ITEM_IsItem( userEntity_t *self );
int ITEM_IsAmmo( userEntity_t *self );
int ITEM_IsHealth( userEntity_t *self );
int ITEM_IsWeapon( userEntity_t *self );

userEntity_t *ITEM_GetTargetedItem( userEntity_t *self );

#endif _ITEMS_H
