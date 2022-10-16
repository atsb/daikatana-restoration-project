///////////////////////////////////////////////////////////////////////////////
//	dk_inv.c
//
//	inventory management routines
///////////////////////////////////////////////////////////////////////////////

#include "server.h"
#include "qcommon.h"
#include "dk_inv.h"

// inventory subsystem

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

static	int					serverInventoryPrecacheCount;
static	int					serverInventoryCount;
static	serverInventory_t	serverInventory[MAX_EDICTS];

static	int					inventoryUpdate = 0;
static	int					clientItem = -1;

clientInventory_t clientInventory [MAX_INVENTORY];

///////////////////////////////////////////////////////////////////////////////
//	precache
///////////////////////////////////////////////////////////////////////////////

static	serverInventoryPrecache_t serverInventoryPrecache[MAX_INVENTORY];

///////////////////////////////////////////////////////////////////////////////
//	dll precache interface code
///////////////////////////////////////////////////////////////////////////////

static	serverInventoryPrecache_t	*InventoryFindPrecache(char *name)
{
    int i;

	for (i = 0; i < serverInventoryPrecacheCount; i++)
	{
		if (!strcmp(serverInventoryPrecache[i].className, name))
		{
			return(&serverInventoryPrecache[i]);
		}
	}

	return(NULL);
}


///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

void User_InventoryPrecache (char *name, char *iconName, char *modelName)
{
	// make sure we are initing
	if (sv.state != ss_loading && sv.state != ss_dead)
	{
		Com_Printf("Host already inited, %s not precached", name);
		return;
	}

	if (serverInventoryPrecacheCount >= MAX_INVENTORY)
	{
		Com_Printf("No more inventory items left, increase MAX_INVENTORY\n");
		return;
	}

	serverInventoryPrecache [serverInventoryPrecacheCount].className = strdup(name);
	serverInventoryPrecache [serverInventoryPrecacheCount].iconName  = iconName;	 //???
//	serverInventoryPrecache [serverInventoryPrecacheCount].sum       = inv_IconCRC(icon);

	// weapons have to have a model to draw on the screen
	if (modelName)
	{
		// User_PrecacheModel(modelName);
		serverInventoryPrecache [serverInventoryPrecacheCount].modelName = strdup(modelName);
	}
	else
	{
		serverInventoryPrecache [serverInventoryPrecacheCount].modelName = NULL;
	}

	serverInventoryPrecacheCount++;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/// dll interface code
/////////////////////////////////////////////////////////////////////////////////////////////


void *User_InventoryNew(void)
{
	int i;

	// see if there is a free one avail
	for (i = 0; i < serverInventoryCount; i++)
	{
		if (!serverInventory[i].in_use) break;
	}

	if (i == serverInventoryCount)
	{
	    if (serverInventoryCount >= MAX_EDICTS) 
		{
			Com_Printf("ERROR -- serverInventoryCount maxed out\n");
			return(NULL);
		}
	
		serverInventoryCount++;
	}

    // init this server inventory list
	serverInventory[i].in_use     = 1;
	serverInventory[i].invHead    = NULL;
	serverInventory[i].invTail    = NULL;

	return(&serverInventory[i]);
}



static void User_InventoryFree_r(serverInventoryList_t *p)
{
	if (p == NULL) return;

	User_InventoryFree_r(p->n);
	free(p);
}

void User_InventoryFree(void *ptr)
{
	serverInventory_t *inv = (serverInventory_t *) ptr;

	if (!ptr)
	{
		Com_Printf("User_InventoryFree: NULL pointer\n");
		return;
	}

	// clear out nodes
	User_InventoryFree_r(inv->invHead);

	inv->in_use  = 0;
	inv->invHead = NULL;
	inv->invTail = NULL;
}



userInventory_t *User_InventoryFindItem(void *ptr, char *name)
{
	serverInventory_t *inv = (serverInventory_t *) ptr;
	serverInventoryList_t *p;
	userInventory_t    *userInv;

	if (!ptr)
	{
		Com_Printf("User_InventoryFindItem: NULL pointer\n");
		return(NULL);
	}

	// go down clients inventory list looking for item
	for (p = inv->invHead; p != NULL; p = p->n)
	{
		// double check that name got set
		if (p->data)
		{
			userInv = (userInventory_t *) p->data;

			if (userInv->name)
			{
			    if (!Q_strcasecmp(userInv->name, name))
			    {
				    return(p->data);
				}
			}
			else
			{
				Com_Printf("WARNING -- Invalid inventory item.\n");
			}
		}
		else
		{
			Com_Printf("WARNING -- Invalid inventory item.\n");
		}
	}

	return(NULL);
}

userInventory_t *User_InventoryFirstItem(void *ptr)
{
	serverInventory_t *inv = (serverInventory_t *) ptr;

	if (!ptr)
	{
		Com_Printf("User_InventoryFirstItem: NULL pointer\n");
		return(NULL);
	}

	if (inv->invHead == NULL)
	{
		return(NULL);
	}

	// reset list walker pointer
	inv->invPtr = inv->invHead;

	return(inv->invPtr->data);
}



userInventory_t *User_InventoryNextItem(void *ptr)
{
	serverInventory_t *inv = (serverInventory_t *) ptr;

	if (!ptr)
	{
		Com_Printf("User_InventoryNextItem: NULL pointer\n");
		return(NULL);
	}

	// double check
	if (inv->invPtr == NULL) return(NULL);

	// advance
	inv->invPtr = inv->invPtr->n;

	if (inv->invPtr == NULL) return(NULL);
	return(inv->invPtr->data);
}



int User_InventoryAddItem(void *ptr, userInventory_t *item)
{
	serverInventory_t    *inv = (serverInventory_t *) ptr;
	serverInventoryList_t      *invNode;
	serverInventoryPrecache_t  *precache;


	if (!ptr)
	{
		Com_Printf("User_InventoryAddItem: NULL pointer\n");
		return(0);
	}

	if (!item->name)
	{
		Com_Printf("User_InventoryAddItem: Item must have a className\n");
		return(0);
	}

   precache = InventoryFindPrecache(item->name);

   if (!precache)
	{
		Com_Printf("User_InventoryAddItem: %s not precached\n", item->name);
		return(0);
	}

	// mdm99.03.18 - memmgr fails on no memory conditions, don't even need to check	
	// mdm99.03.18 - memmgr zero fills
	invNode = X_Malloc(sizeof(serverInventoryList_t), MEM_TAG_INVENTORY);

	invNode->data     = item;
	invNode->precache = precache;

	item->precache = precache;

	// create a new inventory node
	// see if we need to make this item the head
	if ((inv->invHead == NULL) && (inv->invTail == NULL))
	{
		inv->invHead = inv->invTail = invNode;
		return(1);
	}

	// ok add item to tail of list
	inv->invTail->n = invNode;
	invNode->p      = inv->invTail;
	inv->invTail    = invNode;

	return(1);
}


// void User_InventoryDeleteItem(void *ptr, void *item)
void User_InventoryDeleteItem(void *ptr, userInventory_t *item)
{
	serverInventory_t *inv         = (serverInventory_t *) ptr;
	// serverInventoryList_t *invNode = (serverInventoryList_t *) item;
	serverInventoryList_t *invNode;

	if (!ptr)
	{
		Com_Printf("User_InventoryDeleteItem: NULL pointer\n");
		return;
	}

	// find item to delete
	for (invNode = inv->invHead; invNode; invNode = invNode->n)
	{
		if (invNode->data == item) break;
	}

	if (!invNode)
	{
		Com_Printf("User_InventoryDeleteItem: Unable to find item in inventory\n");
		return;
	}

	// see if this the only node left in the list
	if ((invNode == inv->invHead) && (invNode == inv->invTail))
	{
		free(invNode);

		inv->invHead = NULL;
		inv->invTail = NULL;

		return;
	}

	// see if we are deleteing from fron head
	if (invNode == inv->invHead)
	{
		inv->invHead    = invNode->n;
		inv->invHead->p = NULL;

		free(invNode);
		return;
	}

	// see if we are deleting from end
	if (invNode == inv->invTail)
	{
		inv->invTail    = invNode->p;
		inv->invTail->n = NULL;

		free(invNode);
		return;
	}

	// ok we are deleteing from middle
	invNode->p->n = invNode->n;
	invNode->n->p = invNode->p;

	free(invNode);
}








#if 0

//////////////////////////////////////////////////////////////////////////////////////////
/// Client inventory drawing routines
//////////////////////////////////////////////////////////////////////////////////////////

static int inv_entersound;


static void InventoryQuery_f(void)
{
	// make sure user is connected to server
    if (cls.state != ca_connected)
    {
        Com_Printf("You must first connect to a server.\n");
        return;
	}

	if (!inventoryUpdate)
	{
		Com_Printf("Wait inventory is updating\n");
		return;
	}

	key_dest       = key_inv;
	inv_entersound = true;

}



void Inventory_Init(void)
{
	Cmd_AddCommand("inventory", InventoryQuery_f);
}




void inv_Keydown(int key)
{
	int oldItem;

	switch (key)
	{
	case K_ESCAPE:
		Com_Printf("exiting inventory system\n");
		key_dest = key_game;
		break;

	case K_LEFTARROW:
		// S_LocalSound ("misc/menu1.wav");
		oldItem = clientItem;
		clientItem--;

		while (clientItem != oldItem)
		{
			// see we we need to loop around
		    if (clientItem < 0)
		    {
			    clientItem = MAX_INVENTORY;
		    }

			if (clientInventory[clientItem].in_use) break;
			else clientItem--;
		}

		break;

	case K_RIGHTARROW:
		// S_LocalSound ("misc/menu1.wav");

		oldItem = clientItem;
		clientItem++;

		while (clientItem != oldItem)
		{
			// see if we need to loop around
			if (clientItem >= MAX_INVENTORY)
			{
				clientItem = 0;
			}

			if (clientInventory[clientItem].in_use) break;
			else clientItem++;
		}

		break;

	case K_ENTER:
		// S_LoaclSound("misc/menu1.wav");

		// send server a message selecting this item
		MSG_WriteByte(&cls.message, clc_inviconsel);
		MSG_WriteShort(&cls.message, clientItem);

		key_dest = key_game;
		break;

	}
}



void inv_Draw(void)
{
	qpic2_t temp;
	int i;
	clientInventory_t *item;

	if (key_dest != key_inv) return;
	if (clientItem < 0)      return;

	item = &clientInventory[clientItem];

	// ok convert item's 24 bit image to 8 bit image
	inv_RGBPalette(item);

	temp.width  = item->icon_width;
	temp.height = item->icon_height;
	temp.data   = item->icon_data8;

	Draw_TransPic2(10, 10, &temp);
	// Draw_Pic2(10, 10, &temp);
}

#endif