///////////////////////////////////////////////////////////////////////////////
//	p_inventory.cpp
//
//	Inventory list functions.  Replacement for old Quake 1 based stuff.
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER
#include	<crtdbg.h>
#endif
#include	"p_global.h"
#include	"p_user.h"
#include  "p_inventory.h"

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define P_MAX_INVENTORY_ICONS  3

///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

//static	invenList_t	*entityInventories [MAX_EDICTS];

// cek[1-31-00]: won't work in multi 
///////////////////////////////////////////////////////////////////////////////
//	prototypes
///////////////////////////////////////////////////////////////////////////////
invenItem_t * P_FindPreviousInventoryItem(userEntity_t *self, invenList_t	*list);
invenItem_t * P_FindNextInventoryItem(userEntity_t *self, invenList_t	*list);


/*
///////////////////////////////////////////////////////////////////////////////
//	P_InventoryPrecache
//
//	stub
///////////////////////////////////////////////////////////////////////////////

  void	P_InventoryPrecache (char *className, char *iconName, char *modelName)
  {
  return;
  }
*/

///////////////////////////////////////////////////////////////////////////////
//	P_InventoryNew
//
///////////////////////////////////////////////////////////////////////////////

invenList_t	*P_InventoryNew (mem_type memType)
{
	invenList_t	*list;
	//	mem_type	memType = MEM_PERMANENT;	//	make this passed so monsters can use MEM_MALLOC
	MEM_TAG tag = (memType == MEM_PERMANENT) ? MEM_TAG_INVENTORY : MEM_TAG_COM_INV;
	
	list = (invenList_t *)gi.X_Malloc(sizeof (invenList_t), tag);
	//	list = (invenList_t *) com_malloc (sizeof (invenList_t), memType);	//	new (memType) invenList_t;
	if (!list)
		return	NULL;
	
	list->head = NULL;
	list->tail = NULL;
	list->curItem = NULL;
	list->selectedItem = NULL;
	list->memType = memType;
	
	return	list;
}

///////////////////////////////////////////////////////////////////////////////
//	P_InventoryAddItem
//
///////////////////////////////////////////////////////////////////////////////

int	P_InventoryAddItem (userEntity_t *self, invenList_t *list, userInventory_t *data)
{
	invenItem_t	*item;
	
	// if this is an invalid inventory list, return an error
	if (!list)
		return FALSE;
	
	_ASSERTE (data->memType == list->memType);
	
	MEM_TAG tag = (list->memType == MEM_PERMANENT) ? MEM_TAG_INVENTORY : MEM_TAG_COM_INV;
	
	item = (invenItem_t *)gi.X_Malloc(sizeof (invenItem_t), tag);	//	new (list->memType) invenItem_t;
	//	item = (invenItem_t *) com_malloc (sizeof (invenItem_t), list->memType);	//	new (list->memType) invenItem_t;
	item->data = data;
	
	if (list->head == NULL)
	{
		//	empty list
		item->prev = NULL;
		item->next = NULL;
		list->selectedItem = NULL;
		
		list->head = item;
		list->tail = item;
		
		
		
		// if this is an empty list, check to see if this a 'sack' inventory item
		if ((self->flags & FL_CLIENT) && (item->data->flags & ITF_INVSACK))
		{
			list->selectedItem = item;          // mark as selected by default

			self->client->needsUpdate |= HUD_UPDATE_INVEN;
		}
		
		return	true;
	}
	
	list->tail->next = item;
	item->prev = list->tail;
	item->next = NULL;
	list->tail = item;
	
	// if an item hasn't been selected yet, check for 'sack' type
	if (!list->selectedItem)
	{
		if (item->data->flags & ITF_INVSACK)
		{
			list->selectedItem = item;          // mark as selected by default
		}
	}

	if( (item->data->flags & ITF_INVSACK) && ( self != NULL ) && ( self->flags & FL_CLIENT ) )
	{
		self->client->needsUpdate |= HUD_UPDATE_INVEN;
	}
	return	true;
}

///////////////////////////////////////////////////////////////////////////////
//	P_InventoryFree
//
///////////////////////////////////////////////////////////////////////////////

void	P_InventoryFree (invenList_t *list)
{
	invenItem_t	*item, *nextItem;
	
	for (item = list->head; item; item = nextItem)
	{
		nextItem = item->next;
		
		gi.X_Free(item);
		//		com_free (item);
	}
	
	gi.X_Free(list);
	//	com_free (list);
}

///////////////////////////////////////////////////////////////////////////////
//	P_InventoryFindItem
//
///////////////////////////////////////////////////////////////////////////////

userInventory_t	*P_InventoryFindItem (invenList_t *list, char *name)
{
	invenItem_t		*item;
	userInventory_t	*data;
	
	if (!list)
		return	NULL;
	
	for (item = list->head; item != NULL; item = item->next)
	{
		data = item->data;
		
		//	all inventory items should have data and a name!
		_ASSERTE (data);
		_ASSERTE (data->name);
		
		if (!stricmp (data->name, name))
			return	data;
	}
	
	return	NULL;
}

///////////////////////////////////////////////////////////////////////////////
//	P_InventoryFirstItem
//
///////////////////////////////////////////////////////////////////////////////

userInventory_t *P_InventoryFirstItem (invenList_t *list)
{
	if (list == NULL || list->head == NULL)
		return	NULL;
	
	list->curItem = list->head;
	
	return	list->head->data;
}

///////////////////////////////////////////////////////////////////////////////
//	P_InventoryNextItem
//
///////////////////////////////////////////////////////////////////////////////

userInventory_t *P_InventoryNextItem (invenList_t *list)
{
	if (list == NULL || list->head == NULL)
		return	NULL;
	
	if (list->curItem == NULL)
		return	NULL;
	
	list->curItem = list->curItem->next;
	
	if (list->curItem == NULL)
		return	NULL;
	
	return	list->curItem->data;
}

///////////////////////////////////////////////////////////////////////////////
//	P_InventoryDeleteItem
//
///////////////////////////////////////////////////////////////////////////////

int	_P_InventoryDeleteItem (userEntity_t *self, invenList_t *list, userInventory_t *data)
{
	invenItem_t	*item;
	
	if (list == NULL || list->head == NULL)
		return	false;
	
	//	find item with matching data
	for (item = list->head; item; item = item->next)
	{
		_ASSERTE (item->data);
		
		if (item->data == data)
			break;
	}
	
	if (item == NULL)  //	reached end of list without finding data
		return	false;
	
	if (item == list->head && item == list->tail)
	{
		//	only item in the list
		list->head = NULL;
		list->tail = NULL;
		
		if (list->selectedItem == item)
			list->selectedItem = NULL;
	}
	else if (item == list->head)
	{
		// check for next selectable item in the list BEFORE changing links
		if (list->selectedItem == item)
			list->selectedItem = P_FindNextInventoryItem(self,list);
		
		//	first item in the list
		list->head = item->next;
		list->head->prev = NULL;
	}
	else if (item == list->tail)
	{
		// check for previous selectable item in the list BEFORE changing links
		if (list->selectedItem == item)
			list->selectedItem = P_FindPreviousInventoryItem(self,list);
		
		//	last item in the list
		list->tail = item->prev;
		list->tail->next = NULL;
	}
	else
	{
		// check for next selectable item in the list BEFORE changing links
		if (list->selectedItem == item)
			list->selectedItem = P_FindNextInventoryItem(self,list);
		
		//	item is in the middle of the list
		item->prev->next = item->next;
		item->next->prev = item->prev;
	}
	
	gi.X_Free (item);
	
	//	update inventory if it is being displayed
	if( ( self != NULL ) && ( self->flags & FL_CLIENT ) )
	{
		self->client->needsUpdate |= HUD_UPDATE_INVEN;
	}
//	if (self && self->flags & FL_CLIENT)
//		P_InventoryUpdateClient (self, false);
	
	return	true;
}

int	P_InventoryDeleteItem (userEntity_t *self, invenList_t *list, userInventory_t *data)
{
	if ((coop->value) && (data && (data->flags & ITF_COOP_REMOVEALL)))
	{
		userEntity_t *p = &serverState.g_edicts[1];
		userInventory_t *item;
		for(int i=0;i<3;i++,p++)
		{
			if (p == self)
				continue;

			if (p && p->inuse && p->inventory)
			{
				item = P_InventoryFindItem(p->inventory,data->name);
				if (item)
					_P_InventoryDeleteItem (p, p->inventory, item);
			}
		}
	}

	// delete the one we're supposed to...
	return _P_InventoryDeleteItem (self, list, data);
}

///////////////////////////////////////////////////////////////////////////////
//	P_InventoryCreateItem
//
//	creates an inventory item.  Size must be passed so that varying inventory
//	structures can be used instead of just userInventory_t, such as weapon_t
///////////////////////////////////////////////////////////////////////////////

userInventory_t	*P_InventoryCreateItem (invenList_t *list, char *name, invenUse_t use, invenCommand_t command, 
										int modelIndex, unsigned long flags, short structSize)
{
	userInventory_t	*data;
	
	if( list == NULL )
	{
		return NULL;
	}

	MEM_TAG tag = (list->memType == MEM_PERMANENT) ? MEM_TAG_INVENTORY : MEM_TAG_COM_INV;
	
	//	MUST use same memory type as list, otherwise some items could
	//	get auto-freed by com_heap routines and corrupt the list!
	
	data = (userInventory_t *)gi.X_Malloc(structSize, tag);
	if (!data)
		return	NULL;
	
	data->memType = list->memType;
	
	//	strdup the name so we don't have to worry about what type of string
	//	pointer got passed
	data->name = (char *)gi.X_Malloc(strlen (name) + 1, tag);
	strcpy (data->name, name);
	
	data->use = use;
	data->command = command;
	
	//	FIXME:	might need to store model name if model indexes change
	//			between levels due to different number of models being
	//			cached, or being cached in different order, etc.
	//			However, I don't think they will change because GL doesn't
	//			reload them if they already exist in the model list.
	/*
	//	strdup the model name
	//	data->modelName = (char *) com_malloc (strlen (modelName) + 1, list->memType);
	strcpy (data->modelName, name);
	*/
	data->modelIndex = modelIndex;
	data->flags = flags;
	
	Com_sprintf(data->modelName,sizeof(data->modelName),"%s",serverState.gi->GetConfigString(CS_MODELS + data->modelIndex));
	return	data;
}





invenItem_t * P_FindPreviousInventoryItem(userEntity_t *self, invenList_t	*list)
// search for previous item from current selected
{
	invenItem_t *search_item = NULL;
	
	if (!list || !list->selectedItem) // current item selected exists?
		return (NULL);
	
	
	if (list->selectedItem == list->head)      // first item?
		search_item = list->tail;                // flip, start looking at last item
	else
		search_item = list->selectedItem->prev;
	
	while (search_item && search_item != list->selectedItem)
	{
		if (search_item->data->flags & ITF_INVSACK)  // found it
			return (search_item); 
		
		if (search_item == list->head)     // flip?
			search_item = list->tail;
		else
			search_item = search_item->prev; // check previous item
	}
	
	if (search_item == list->selectedItem)  // no item found?
		search_item = NULL;
	
	return (search_item);
}


invenItem_t * P_FindNextInventoryItem(userEntity_t *self, invenList_t	*list)
// search for next item from current selected
{
	invenItem_t *search_item;
	
	if (!list || !list->selectedItem) // current item selected exists?
		return (NULL);
	
	if (list->selectedItem == list->tail)      // last item?
		search_item = list->head;                // flip, start looking at first item
	else
		search_item = list->selectedItem->next;
	
	while (search_item && search_item != list->selectedItem)
	{
		if (search_item->data->flags & ITF_INVSACK)  // found it
			return (search_item); 
		
		if (search_item == list->tail)     // flip?
			search_item = list->head;
		else
			search_item = search_item->next; // check previous item
	}
	
	if (search_item == list->selectedItem)  // no item found?
		search_item = NULL;
	
	return (search_item);
}




/* ----------------------------- Inventory Commands ------------------------ */

/*
#define	MAX_DISPLAY_ITEMS	3

  
	///////////////////////////////////////////////////////////////////////////////
	//	P_NumUniqueItems
	//
	//	returns the number of unique items in the sendList
	///////////////////////////////////////////////////////////////////////////////
	
	  int	P_NumUniqueItems (invenItem_t *sendList [MAX_DISPLAY_ITEMS])
	  {
	  int			i, j, numUnique = 0;
	  invenItem_t	*loggedItems [MAX_DISPLAY_ITEMS];
	  
		memset (loggedItems, 0x00, sizeof (invenItem_t *) * MAX_DISPLAY_ITEMS);
		
		  for (i = 0; i < MAX_DISPLAY_ITEMS; i++)
		  {
		  for (j = 0; j < MAX_DISPLAY_ITEMS; j++)
		  {
		  if (sendList [i] == loggedItems [j])
		  break;
		  }
		  
			if (j == MAX_DISPLAY_ITEMS)
			{
			loggedItems [numUnique] = sendList [i];
			numUnique++;
			}
			}
			
			  return	numUnique;
			  }
*/



byte P_InventoryItemCount(invenList_t *list, char *item_name)
{
	invenItem_t     *item;
	userInventory_t	*data;
	byte             count;
	
	if (!list)
		return (0);
	
	count = 0;
	
	for (item = list->head;item != NULL;item = item->next)
	{
		data = item->data;
		
		if (data && data->name && !stricmp (data->name, item_name)) // found match? 
			count++;
	}
	
	return (count);
}





void P_InventoryUpdateClient (userEntity_t *self, int bHideDisplay)
{
	int			i;
	invenList_t	*list;
	invenItem_t	*sendItem [P_MAX_INVENTORY_ICONS];	//	max # items can be seen at once
	
	//Com_Printf("P_InventoryUpdateClient(%d)\n",bHideDisplay);
	
	list = self->inventory;
	
	if( ( list == NULL ) || ( list->head == NULL ) )
//	if( list == NULL )
		return;
	
	sendItem[1] = list->selectedItem;  // set middle slot as selected item
	
	sendItem[0] = P_FindPreviousInventoryItem(self,list);  // previous item
	sendItem[2] = P_FindNextInventoryItem(self,list);      // next item

	// SCG[02/13/00]: Changed per Romero's request
	if( sendItem[0] == sendItem[2] )
	{
		sendItem[2] = NULL;
	}

	if( ( sendItem[0] == NULL ) && ( sendItem[1] == NULL ) && ( sendItem[2] == NULL ) )
	{
		gi.WriteByte(SVC_INVENTORY);
		gi.WriteByte(1); // display on/off
		gi.WriteByte(1); // clear values
		gi.unicast (self, true);
		self->client->inventoryTime = 10.0f; // reset
		return;
	}
	
	//	send the inventory message
	gi.WriteByte(SVC_INVENTORY);
	
	gi.WriteByte(bHideDisplay); // display on/off
	gi.WriteByte(0);
	
	if (!bHideDisplay)          // only pass down info if showing display
	{
		for (i = 0; i < P_MAX_INVENTORY_ICONS; i++)
		{
			if (sendItem[i] != NULL)
				gi.WriteShort (sendItem[i]->data->modelIndex);
			else
				gi.WriteShort (0);  			  //	no item
		}
		
		self->client->inventoryTime = p_realtime + INVMODE_DISPLAYTIME; // # seconds for display

		if (serverState.InventorySetMode)
			serverState.InventorySetMode(self,INV_MODE_ITEMS,false);
	}
	else
		self->client->inventoryTime = 0.0f; // reset
	
	gi.unicast (self, true);
}


///////////////////////////////////////////////////////////////////////////////
//	P_InventoryNext
//
//	user command to select the next item in inventory
///////////////////////////////////////////////////////////////////////////////

void	P_InventoryNext (userEntity_t *self)
{
	invenList_t	*list = self->inventory;
	invenItem_t *search_item;
	
	
	if (!list || !list->head || !list->tail || !list->selectedItem)
		return;
	
	search_item = P_FindNextInventoryItem(self,list);
	
	if (search_item != NULL)             // found one?
	{					 
		list->selectedItem = search_item;  // set it
	}
	
	self->client->needsUpdate |= HUD_UPDATE_INVEN;
//	P_InventoryUpdateClient (self, false);
}



///////////////////////////////////////////////////////////////////////////////
//	P_InventoryPrev
//
//	user command to select the previous item in inventory
///////////////////////////////////////////////////////////////////////////////

void	P_InventoryPrev (userEntity_t *self)
{
	invenList_t	*list = self->inventory;
	invenItem_t *search_item;
	
	
	if (!list || !list->head || !list->tail || !list->selectedItem)
		return;
	
	search_item = P_FindPreviousInventoryItem(self,list);
	
	if (search_item != NULL)						 // found one? 
	{
		list->selectedItem = search_item;  // set it
	}
	
	self->client->needsUpdate |= HUD_UPDATE_INVEN;
//	P_InventoryUpdateClient (self, false);
}



///////////////////////////////////////////////////////////////////////////////
//	P_InventoryUse
//
//
///////////////////////////////////////////////////////////////////////////////

void	P_InventoryUse (userEntity_t *self)
{
	userInventory_t	*data;
	invenList_t		*list = self->inventory;
	
	if (!list || !list->head || !list->tail || !list->selectedItem)
		return;
	
	data = list->selectedItem->data;
	
	//	use the item
	if (data->use)
	{
		data->use (data, self);
	}
}



///////////////////////////////////////////////////////////////////////////////
//	P_InventoryOperate (opens or closes inventory)
///////////////////////////////////////////////////////////////////////////////
// cek[1-31-00] added bHideDisplay...this will not work in multiplayer mode
void P_InventoryOperate(userEntity_t *self, int bHideDisplay)
{
	self->client->needsUpdate |= HUD_UPDATE_INVEN;
//	P_InventoryUpdateClient (self, bHideDisplay);
}






#define	MAX_DISPLAY_WEAPONS	6
weaponInfo_t *com_FindWeaponInSlot (int display_order);		// com_weapons.cpp

void	P_WeaponUpdateClient(userEntity_t *self, int hud_status)
// send weapon icon and ammo count information to client
{
	
	//Com_Printf("P_WeaponUpdateClient(%d)\n",hud_status);
	
	if (!self || !self->inventory)
		return;
	
	if (!(self->flags & FL_CLIENT))  // must be client for update
		return;
	
	
	if (!hud_status) // if wanting to force the weapon hud off?
	{
		//	send the inventory message
		gi.WriteByte (SVC_WEAPON_ICON);
		gi.WriteByte(0);                 // this forces the interface hud off (true = hide)
		gi.unicast (self, true);
		
		return;	// our work is done here
	}

	int i, current_weapon = -1;

	weaponInfo_t	*winfo[MAX_DISPLAY_WEAPONS];
	weapon_t        *weaponList[MAX_DISPLAY_WEAPONS];
	memset(&weaponList[0],NULL,sizeof(weaponList)); // initialize
	memset(&winfo[0],NULL,sizeof(winfo)); // initialize

	for (i = 0; i < MAX_DISPLAY_WEAPONS; i++)
	{
		winfo[i] = com_FindWeaponInSlot(i);
		if (!winfo[i])
			continue;

		weaponList[i] = (weapon_t *)P_InventoryFindItem(self->inventory,winfo[i]->weaponName);
		if (weaponList[i] && (weaponList[i] == (weapon_t *)self->curWeapon))
			current_weapon = i;
	}

	if( current_weapon == -1 )
	{
		return;
	}
	
	// send the inventory message
	gi.WriteByte (SVC_WEAPON_ICON);
	gi.WriteByte(1);  // show hud on

	short model, count;
	for (i = 0; i < MAX_DISPLAY_WEAPONS; i++)
	{
		model = 0;
		count = -1;

		if (weaponList[i] != NULL)
		{
			model = weaponList[i]->modelIndex;
			
			if (weaponList[i]->ammo != NULL)
			{
				if (!strcmpi(weaponList[i]->ammo->name,"ammo_none"))  // melee weapon?
					count = -1;                                  // doesn't use ammo
				else
					count = weaponList[i]->ammo->count;          // ammo count
			}
			else
				count = 0;
		}
		else
		{
			if (winfo[i] != NULL)
			{
				ammo_t *ammo = (ammo_t *)P_InventoryFindItem(self->inventory,winfo[i]->ammoName);
				if (ammo)
				{
					count = ammo->count;
				}
				// else use defaults set above
			}
		}

		gi.WriteShort(model);  // model index
		gi.WriteShort(count);  // ammo
	}

	gi.WriteShort(current_weapon); // current weapon selected
	
	gi.unicast (self, true);
	
	self->client->inventoryTime = p_realtime + INVMODE_DISPLAYTIME; // # seconds for display
	if (serverState.InventorySetMode)
		serverState.InventorySetMode(self,INV_MODE_WEAPONS,false);
}

/*
void	P_WeaponUpdateClient(userEntity_t *self, int hud_status)
// send weapon icon and ammo count information to client
{
	
	//Com_Printf("P_WeaponUpdateClient(%d)\n",hud_status);
	
	if (!self || !self->inventory)
		return;
	
	if (!(self->flags & FL_CLIENT))  // must be client for update
		return;
	
	
	if (!hud_status) // if wanting to force the weapon hud off?
	{
		//	send the inventory message
		gi.WriteByte (SVC_WEAPON_ICON);
		gi.WriteByte(0);                 // this forces the interface hud off (true = hide)
		gi.unicast (self, true);
		
		return;	// our work is done here
	}
	
	int	             i, current_weapon;
	int              slot;
	weapon_t        *weaponList [6];
	weapon_t        *temp_weap; 
	weapon_t        *special_melee;
	userInventory_t *item;
	
	memset(&weaponList[0],NULL,sizeof(weaponList)); // initialize
	
	current_weapon = -1;
	special_melee  = NULL;
	
	item = P_InventoryFirstItem(self->inventory);
	
	while (item)
	{
		if ( item->flags & ITF_WEAPON)
		{
			temp_weap = (weapon_t *) item;
			temp_weap->command((userInventory_t *) temp_weap, "display_order", &slot);
			
			//Com_Printf("%s, slot %d\n",temp_weap->name,slot);
			
			// check for special melee weapon (ie: gashands)
			// note: no 3rd person model for gashands, so use disruptor glove's model by
			// not overwriting weaponList's slot 0
			if (slot == -1) 
			{
				special_melee = (weapon_t *)item;	// store off  (not currently used, but may in the future)
				slot = 99;													// change slot to 0 so special weapon will show active in first weapon window
			}
			else
			{
				// add weapon into appropriate display slot
				weaponList[slot] = temp_weap;
			}
			
			// is this our current weapon?  store slot
			if (self->curWeapon && !stricmp (item->name, self->curWeapon->name))
				current_weapon = slot; 
		}
		
		item = P_InventoryNextItem (self->inventory);
	}
	
	if( current_weapon == -1 )
	{
		return;
	}
	
	// send the inventory message
	gi.WriteByte (SVC_WEAPON_ICON);
	gi.WriteByte(1);  // show hud on
	
	for (i = 0; i < MAX_DISPLAY_WEAPONS; i++)
	{
		if (weaponList[i] != NULL)
		{
			gi.WriteShort(weaponList[i]->modelIndex);  // model index
			
			if (weaponList[i]->ammo != NULL)
			{
				if (!strcmpi(weaponList[i]->ammo->name,"ammo_none"))  // melee weapon?
					gi.WriteShort(-1);                                  // doesn't use ammo
				else
					gi.WriteShort(weaponList[i]->ammo->count);          // ammo count
			}
			else
				gi.WriteShort(0); // ammo	count of 0
		}
		else
		{
			//	no item
			gi.WriteShort( 0);  // model index
			gi.WriteShort(-1);  // ammo
		}
	}
	
	gi.WriteShort(current_weapon); // current weapon selected
	
	gi.unicast (self, true);
	
	self->client->inventoryTime = p_realtime + INVMODE_DISPLAYTIME; // # seconds for display
	if (serverState.InventorySetMode)
		serverState.InventorySetMode(self,INV_MODE_WEAPONS,false);
}
*/

int P_InventoryCount(userEntity_t *self)
{
	userInventory_t *item = P_InventoryFirstItem(self->inventory);
	int count = 0;
	while (item)
	{
		if (item->flags & ITF_INVSACK)
			count++;
		item = P_InventoryNextItem (self->inventory);
	}
	return count;
}

/*
typedef struct userInventory_s
{
	char			*name;
	invenUse_t		use;
	invenCommand_t	command;
	int				modelIndex;
	unsigned long	flags;
	mem_type		memType;
	void			*dummy4;
} userInventory_t;
typedef struct	keyItem_s
{
	//	!!!! FIELDS BELOW MUST BE IDENTICAL TO userInventory_t!!!!
	userInventory_t	invRec;
	//	!!!! FIELDS ABOVE MUST BE IDENTICAL TO userInventory_t!!!!
	char	target[64];	// entity this key/book is used upon
	char	netname[64];	// entity net name	
	int		soundIndex;		// sound to play when used
} keyItem_t;
*/
int P_GetIndexForFunc( void *func );
void* P_GetFuncForIndex( int index );
void P_WriteFunc( FILE *f, void *p, char *name )
{
	int index = -1;
	if (p)
	{
		index = P_GetIndexForFunc( p );
		if( index != -1 )
		{
			gi.Con_Dprintf("Saved index for function: %s\n", name );
		}
		else
		{
			gi.Con_Dprintf("Could not find index for function: %s\n", name );
		}
	}
	fwrite (&index, sizeof( int ), 1, f);
}

bool P_ReadFunc( FILE *f, byte **p)
{
	int index = 0;
	if (!fread( &index, sizeof( int ), 1, f ))
		return false;


	if (index != -1)
	{
		*p = (byte *)P_GetFuncForIndex(index);
	}
	else
	{
		*p = NULL;
	}

	return true;
}

#define WRITE(buffer,value,size)		{memcpy(buffer,value,size);buffer += size;}
#define WRITESTRING(buffer,value,size)	{WRITE(buffer,&size,sizeof(int));if (size) {WRITE(buffer,value,size)} }	

#define READ(buffer,value,size)			{memcpy(value,buffer,size);buffer += size;}
#define READSTRING(buffer,value,size)	{READ(buffer,&size,sizeof(int));if (size) {READ(buffer,value,size)} }	

#define INC(buffer, bytes)		(buffer += bytes)
void P_WriteItem(void **buf, userInventory_t *item, int index)
{
	if (index == -1)
	{
		FILE *f = (FILE *)(*buf);
		int size = 0;
		size = strlen(item->name);
		fwrite(&size,sizeof(int),1,f);

		if (size)
		{
			fwrite(item->name,sizeof(char),size,f);
		}

		P_WriteFunc(f,item->use,"item_use");
		P_WriteFunc(f,item->command,"item_command");
//		fwrite(&item->modelIndex,sizeof(int),1,f);
		fwrite(item->modelName,sizeof(item->modelName),1,f);
		fwrite(&item->flags,sizeof(unsigned long),1,f);

		if (item->flags & ITF_KEY_ITEM)
		{
			keyItem_t *keyItem = (keyItem_t *)item;
			fwrite(keyItem->target,sizeof(keyItem->target),1,f);
			fwrite(keyItem->netname,sizeof(keyItem->netname),1,f);
//			fwrite(&keyItem->soundIndex,sizeof(keyItem->soundIndex),1,f);
			fwrite(keyItem->soundName,sizeof(keyItem->soundName),1,f);
		}
	}
	else
	{
		client_save_inv_t* _save = (client_save_inv_t* )*buf;//(*buf);//(*(buf + sizeof(client_save_inv_t)*index));
		client_save_inv_t* save = &_save[index];
//		client_save_inv_t* save = (client_save_inv_t *)(*(buf + index));
		memset(save,0,sizeof(client_save_inv_t));

		save->valid = 1;
		Com_sprintf(save->name,sizeof(save->name),item->name);
		save->use = P_GetIndexForFunc(item->use);
		save->command = P_GetIndexForFunc(item->command);
		strcpy(save->model,item->modelName);
		save->flags = item->flags;

		// write the key item specific stuff
		if (item->flags & ITF_KEY_ITEM)
		{
			keyItem_t *key = (keyItem_t *)item;
			strcpy(save->targetName,key->target);
			strcpy(save->netname,key->netname);
			strcpy(save->sound,key->soundName);
		}
	}
}

short P_ReadItem( void **buf, userEntity_t *self, int index)
{
	char *name			= NULL;
	byte *use,*command;
	int	modelIndex = 0;
	unsigned long flags = 0;
	char	target[64];	// entity this key/book is used upon
	char	netname[64];	// entity net name	
	int		soundIndex = 0;		// sound to play when used
	int size = 0;
	char sound[MAX_OSPATH];
	char model[MAX_OSPATH];
	memset(sound,0,sizeof(sound));
	memset(model,0,sizeof(model));

	if (index == -1)
	{
		FILE *f = (FILE *)(*buf);

		if (!fread(&size,sizeof(int),1,f))
			return 0;

		if (size)
		{
			name = (char *)malloc(size+1);
			memset(name,0,size+1);
			if (!fread(name,sizeof(char),size,f))
				return 0;
		}

		// read the use func
		if (!P_ReadFunc(f,&use))
			return 0;

		// read the command func
		if (!P_ReadFunc(f,&command))
			return 0;

		// read the model
		if (!fread(&model,sizeof(model),1,f))
			return 0;
		if (strlen(model))
			modelIndex = serverState.ModelIndex(model);

		// read the item flags
		if (!fread(&flags,sizeof(unsigned long),1,f))
			return 0;

		size = sizeof(userInventory_t);

		// read some specific things for key items
		if (flags & ITF_KEY_ITEM)
		{
			if (!fread(target,sizeof(target),1,f))
				return 0;

			if (!fread(netname,sizeof(netname),1,f))
				return 0;

			if (!fread(sound,sizeof(sound),1,f))
				return 0;

			if (strlen(sound))
				soundIndex = serverState.SoundIndex(sound);
			size = sizeof(keyItem_t);
		}
	}
	else
	{
		client_save_inv_t* _save = (client_save_inv_t* )*buf;//(*buf);//(*(buf + sizeof(client_save_inv_t)*index));
		client_save_inv_t* save = &_save[index];
		if (!save->valid)
			return 0;

		char _name[64];
		strcpy(_name,save->name);
		size = strlen(_name);
		if (size)
		{
			name = (char *)malloc(size+1);
			strcpy(name,_name);
		}
		else
			name = "";

		use = (byte *)P_GetFuncForIndex(save->use);
		command = (byte *)P_GetFuncForIndex(save->command);
		strcpy(model,save->model);
		if (strlen(model))
			modelIndex = serverState.ModelIndex(model);
		flags = save->flags;
		strcpy(target,save->targetName);
		strcpy(netname,save->netname);
		strcpy(sound,save->sound);
		if (strlen(sound))
			soundIndex = serverState.SoundIndex(sound);

		if (flags & ITF_KEY_ITEM)
			size = sizeof(keyItem_t);
		else
			size = sizeof(userInventory_t);
	}

	if (size)
	{
		userInventory_t	*item;
		item = P_InventoryCreateItem (self->inventory, name, (invenUse_t )use, (invenCommand_t )command, modelIndex, flags, size);
		if (item)
		{
			P_InventoryAddItem(self,self->inventory,item);
			if (item->flags & ITF_KEY_ITEM)
			{
				keyItem_t *key = (keyItem_t *)item;
				strcpy(key->target,target);
				strcpy(key->netname,netname);
				strcpy(key->soundName,sound);
				key->soundIndex = soundIndex;
			}
		}
	}

	if (name)
		free(name);

	return 1;
}

void P_InventorySave( void **buf, userEntity_t *self, int aryEntries )
{
	if (!buf)
		return;

	userInventory_t *item;
	if (!aryEntries)
	{
		FILE *f = (FILE *)(*buf);
		if (!f)
			return;

		int count = P_InventoryCount(self);
		fwrite(&count,sizeof(int),1,f);
		if (!count)
			return;

		item = P_InventoryFirstItem(self->inventory);
		while (item)
		{
			if (item->flags & ITF_INVSACK)
				P_WriteItem(buf,item,-1);

			item = P_InventoryNextItem (self->inventory);
		}
	}
	else
	{
		memset(*buf,0,aryEntries * sizeof(client_save_inv_t));
		item = P_InventoryFirstItem(self->inventory);
		int index = 0;
		for (int i = 0; ((i < aryEntries) && item); i++, item = P_InventoryNextItem(self->inventory))
		{
			if (item->flags & (ITF_INVSACK|ITF_SPECIAL))
				P_WriteItem(buf,item,index++);
		}
	}
}

void P_InventoryLoad( void **buf, userEntity_t *self, int aryEntries )
{
	if (!buf)
		return;

	if (!aryEntries)
	{
		FILE *f = (FILE *)(*buf);
		if (!f)
			return;

		int count = 0;
		fread(&count,sizeof(int),1,f);
		if (!count)
			return;

		for (int i = 0; i < count; i++)
		{
			if (!P_ReadItem(buf,self,-1))
			{
				serverState.Con_Dprintf("Error reading entity %s inventory.\n",self->className);
				return;
			}
		}
	}
	else
	{
		int count = 0;
		for (int i = 0; i < aryEntries; i++)
		{
			if (!P_ReadItem(buf,self,i))
				break;
		}
	}
}

/*
void P_WriteItem(FILE *f, userInventory_t *item)
{
	int size = 0;
	size = strlen(item->name);
	fwrite(&size,sizeof(int),1,f);

	if (size)
	{
		fwrite(item->name,sizeof(char),size,f);
	}

	P_WriteFunc(f,item->use,"item_use");
	P_WriteFunc(f,item->command,"item_command");
	fwrite(&item->modelIndex,sizeof(int),1,f);
	fwrite(&item->flags,sizeof(unsigned long),1,f);

	if (item->flags & ITF_KEY_ITEM)
	{
		keyItem_t *keyItem = (keyItem_t *)item;
		fwrite(&keyItem->target,sizeof(keyItem->target),1,f);
		fwrite(&keyItem->netname,sizeof(keyItem->netname),1,f);
		fwrite(&keyItem->soundIndex,sizeof(keyItem->soundIndex),1,f);
	}
}

void P_ReadItem( FILE *f, userEntity_t *self)
{
	int size = 0;
	char *name = NULL;
	byte *use,*command;
	int modelIndex;
	unsigned long flags;
	char	target[64];	// entity this key/book is used upon
	char	netname[64];	// entity net name	
	int		soundIndex;		// sound to play when used

	fread(&size,sizeof(int),1,f);
	if (size)
	{
		name = (char *)malloc(size+1);
		memset(name,0,size+1);
		fread(name,sizeof(char),size,f);
	}
	P_ReadFunc(f,&use);
	P_ReadFunc(f,&command);
	fread(&modelIndex,sizeof(int),1,f);
	fread(&flags,sizeof(unsigned long),1,f);

	size = sizeof(userInventory_t);
	if (flags & ITF_KEY_ITEM)
	{
		fread(&target,sizeof(target),1,f);
		fread(&netname,sizeof(netname),1,f);
		fread(&soundIndex,sizeof(int),1,f);
		size = sizeof(keyItem_t);
	}

	userInventory_t	*item;
	item = P_InventoryCreateItem (self->inventory, name, (invenUse_t )use, (invenCommand_t )command, modelIndex, flags, size);
	if (item)
		P_InventoryAddItem(self,self->inventory,item);
}

void P_InventorySave( FILE *f, userEntity_t *self )
{
	int count = P_InventoryCount(self);
	fwrite(&count,sizeof(int),1,f);
	if (!count)
		return;

	userInventory_t *item = P_InventoryFirstItem(self->inventory);
	while (item)
	{
		if (item->flags & ITF_INVSACK)
			P_WriteItem(f,item);

		item = P_InventoryNextItem (self->inventory);
	}

}

void P_InventoryLoad( FILE *f, userEntity_t *self )
{
	int count = 0;
	fread(&count,sizeof(int),1,f);
	if (!count)
		return;

	for (int i = 0; i < count; i++)
	{
		P_ReadItem(f,self);
	}
}
*/


void inventory_AddCommands(void)
{
	// cek[1-31-00]: obsolete
//	gi.AddCommand ("inven",          P_InventoryOperate);
//	gi.AddCommand ("inventory_next", P_InventoryNext);
//	gi.AddCommand ("inventory_prev", P_InventoryPrev);
//	gi.AddCommand ("inventory_use",  P_InventoryUse);
}

