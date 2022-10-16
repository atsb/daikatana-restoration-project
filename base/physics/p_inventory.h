#ifndef	_INVENTORY_H
#define	_INVENTORY_H

// externs
//extern int inv_open_status;


// prototypes
invenList_t	*     P_InventoryNew (mem_type memType);
void	            P_InventoryFree (invenList_t *list);
userInventory_t	* P_InventoryFindItem (invenList_t *list, char *name);
userInventory_t * P_InventoryFirstItem (invenList_t *list);
userInventory_t * P_InventoryNextItem (invenList_t *list);
userInventory_t * P_InventoryNextItem (invenList_t *list);
int	              P_InventoryAddItem (userEntity_t *self, invenList_t *list, userInventory_t *data);
int	              P_InventoryDeleteItem (userEntity_t *self, invenList_t *list, userInventory_t *data);
userInventory_t	* P_InventoryCreateItem (invenList_t *list, char *name, invenUse_t use, invenCommand_t command, 
										int modelIndex, unsigned long flags, short structSize);
void              P_InventoryUpdateClient(userEntity_t *self, qboolean bHideDisplay);
byte              P_InventoryItemCount (invenList_t *list, char *name);
void              P_WeaponUpdateClient(userEntity_t *self, int hud_status);

void              inventory_AddCommands(void);


#endif