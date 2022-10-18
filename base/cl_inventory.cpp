#include	<windows.h>
#include	"client.h"
#include	"cl_inventory.h"
//#include  "physics.h"

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

invenIcon_t	inventoryIcons [CL_MAX_INVENTORY_ICONS];
int   numInventoryIcons;
int   inventory_mode;
int   inventory_selected;
float inventory_alpha;


void	inventory_init (void)
{
	//	reset inventory display times
	numInventoryIcons  = 0;
	inventory_mode     = INVMODE_OFF;  // by default
	inventory_selected = 0;
	inventory_alpha    = 0.0f;

	// reset hud defaults
	invmode_hud.alpha     = 0.0f;
	invmode_hud.direction = 0;
	invmode_hud.fade_time = 0.0f;
}


///////////////////////////////////////////////////////////////////////////////
//	inventory_ParseInventory
//
//	reads an svc_inventory message from the message buffer and sets up the
//	inventory list accordingly
///////////////////////////////////////////////////////////////////////////////

void	inventory_ParseInventory (void)
{
	int	i, modelIndex;
	qboolean bHideDisplay;

	bHideDisplay = MSG_ReadByte(&net_message); // hide display?
	qboolean bClear = MSG_ReadByte(&net_message);

	//Com_Printf("inventory_ParseInventory = %d\n",bHideDisplay);

	if (!bHideDisplay)  // if displaying the inventory contents
	{
		if (inventory_mode != INVMODE_ITEM_ICONS) // not open yet?
			S_StartLocalSound("global/inventory_open.wav"); //play open/close sound
		else
			S_StartLocalSound("global/inventory_update.wav"); //play up/down sound

		numInventoryIcons = 3;

		for (i = 0; i < numInventoryIcons; i++)
		{
			modelIndex = MSG_ReadShort(&net_message);
				
			inventoryIcons[i].modelindex = modelIndex;

			if (modelIndex != 0)
				inventoryIcons [i].model = cl.model_draw [modelIndex];
			else
				inventoryIcons [i].model = NULL;
		}

		inventory_mode = INVMODE_ITEM_ICONS; // set new hud mode

//		invmode_hud_setfade(1); // set up fade info
		if (scr_viewsize->value >= SCR_VIEWSIZE_NOSKILLS)
		{	
			invmode_hud.off_time = cl.time + 5000;
		}

		invmode_hud.direction = 1;       // alpha direction is towards fade out
		invmode_hud.fade_time = cl.time;  // start fade NOW / mode is reset when alpha reaches 0
	}
	else if (bClear && (inventory_mode == INVMODE_ITEM_ICONS))
	{
		for (i = 0; i < numInventoryIcons; i++)
			inventoryIcons [i].model = NULL;
	}
	// SCG[12/14/99]: control placed in world.dll/inventory.cpp
/*
	else
	{
		if (!check_weapon_hud()) // activate the weapon hud?
		{
			inventory_init(); // reset all vars
		}
	}
*/
}

void CL_InitWeaponIcons()
{
	memset(&inventoryIcons,0,sizeof(inventoryIcons));
	for (int i = 0; i < numInventoryIcons; i++)
	{
		inventoryIcons[i].modelindex = CL_ModelIndex( inventoryIcons[i].modelname );
		if (inventoryIcons[i].modelindex != 0 )
		{
			inventoryIcons[i].model = cl.model_draw[inventoryIcons[i].modelindex];
		}
		else
		{
			memset( &inventoryIcons[i], 0, sizeof( invenIcon_t ) );
			inventoryIcons[i].count = -1;
		}
	}
}

void CL_ParseWeaponIcons(void)
// read incomving svc_weapon_icon message from server
{
	int	i, modelIndex, count;
	byte hud_status;

	//Com_Printf("CL_ParseWeaponIcons()\n");

	hud_status = MSG_ReadByte(&net_message); // display on/off

	if (!hud_status) // force hud off?
	{
		invmode_hud.direction = -1;       // alpha direction is towards fade out
		invmode_hud.fade_time = cl.time;  // start fade NOW / mode is reset when alpha reaches 0

		return; // abort
	}

	numInventoryIcons = 6; // show 6 a total of 6 weapons

	for (i = 0; i < numInventoryIcons; i++)
	{
		modelIndex = MSG_ReadShort(&net_message);	// model index

		inventoryIcons[i].modelindex = modelIndex;

		if (modelIndex != 0)
		{
			inventoryIcons[i].model = cl.model_draw[modelIndex];
			strncpy( inventoryIcons[i].modelname, cl.configstrings[CS_MODELS+modelIndex], MAX_QPATH );
		}
		else
		{
			inventoryIcons[i].model = NULL;
		}

		inventoryIcons[i].count = MSG_ReadShort(&net_message); // ammo count
/*		count = MSG_ReadShort(&net_message);
		if (count == -1)
		{
			if (inventoryIcons[i].count <= 0)
				inventoryIcons[i].count = count;
		}
		else 
			inventoryIcons[i].count = count;*/
	}

	inventory_selected = MSG_ReadShort(&net_message); // which # is active


	// set flag indicating to draw weapon hud/icons
	inventory_mode = INVMODE_WEAPON_ICONS;            // set new hud mode

	if (scr_viewsize->value >= SCR_VIEWSIZE_NOSKILLS)
	{	
		invmode_hud.off_time = cl.time + 5000;
	}

	invmode_hud.direction = 1;       // alpha direction is towards fade out
	invmode_hud.fade_time = cl.time;  // start fade NOW / mode is reset when alpha reaches 0
}
             
void CL_ParseAmmoUpdate()
{
	byte index = MSG_ReadByte(&net_message);
	short count = MSG_ReadShort(&net_message);
	if ((index < 0) || (index >= 6)) 
		return;

	inventoryIcons[index].count = count;
}