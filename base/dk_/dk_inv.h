#ifndef _DKINV_H
#define _DKINV_H

#define MAX_INVENTORY 200

///////////////////////////////////////////////////////////////////////////////
//	client side inventory
///////////////////////////////////////////////////////////////////////////////

typedef struct clientInventory_s
{
	int				in_use;
	char			*className;

	float			lastPalUpdate;			// last time palette was updated
	// icon stuff
	int				icon_width;
	int				icon_height;
	unsigned short	icon_crc;
	unsigned char	*icon_data24;
	unsigned char	*icon_data8;
} clientInventory_t;

///////////////////////////////////////////////////////////////////////////////
//	inventory precache, this holds inventory name, id, picture and other static data
///////////////////////////////////////////////////////////////////////////////

typedef struct serverInventoryPrecache_s
{
	char			*className;				// unique
	char			*iconName;				// image that user sees for this item (24 bit)
	unsigned short	sum;					// checksum of image
	char			*modelName;           
} serverInventoryPrecache_t;

///////////////////////////////////////////////////////////////////////////////
//	user's inventory
///////////////////////////////////////////////////////////////////////////////

typedef struct serverInventoryList_s
{
	userInventory_t						*data;
	struct  serverInventoryPrecache_s	*precache;    // pointer into precache
	struct  serverInventoryList_s		*p, *n;
} serverInventoryList_t;


typedef struct
{
	int    in_use;

    serverInventoryList_t    *invHead,  *invTail;

	// used to walk list
	serverInventoryList_t    *invPtr;
} serverInventory_t;

///////////////////////////////////////////////////////////////////////////////
//	prototypes
///////////////////////////////////////////////////////////////////////////////

void	Inventory_TimeRequest (void);
void	Inventory_Init (void);

void *User_InventoryNew(void);
static void User_InventoryFree_r(serverInventoryList_t *p);
void User_InventoryFree(void *ptr);
userInventory_t *User_InventoryFindItem(void *ptr, char *name);
userInventory_t *User_InventoryFirstItem(void *ptr);
userInventory_t *User_InventoryNextItem(void *ptr);
int User_InventoryAddItem(void *ptr, userInventory_t *item);
void User_InventoryDeleteItem(void *ptr, userInventory_t *item);
void User_InventoryPrecache (char *name, char *iconName, char *modelName);

///////////////////////////////////////////////////////////////////////////////
//	externs
///////////////////////////////////////////////////////////////////////////////

extern	clientInventory_t	clientInventory[MAX_INVENTORY];

#endif