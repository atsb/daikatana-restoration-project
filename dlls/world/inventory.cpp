#include "world.h"
//#include "ai_frames.h"// SCG[1/23/00]: not used
#include "ai_info.h"
#include "sidekick.h"

#define ACTION_WEAPONS_DISABLE	0x00000001
#define ACTION_ITEMS_DISABLE	0x00000002
#define ACTION_SIDEKICK_DISABLE	0x00000004

#define ACTION_WEAPONS_ENABLE	0x00000001
#define ACTION_ITEMS_ENABLE		0x00000002
#define ACTION_SIDEKICK_ENABLE	0x00000004

#define MAX_SKILL_SELECTION

// client.cpp
short isLevelUp(userEntity_t *self);
int AI_IsAlive( userEntity_t *self );

#define IS_PAUSED				(gstate->GetCvar("paused") > 0.0f)
void Inventory_weap_update(userEntity_t *self, qboolean bOn)
{
	if (gstate->gi->TestUnicast(self,true))
	{
		gstate->WeaponDisplayUpdateClient(self,bOn);
	}
	else
	{
		self->client->needsUpdate |= HUD_UPDATE_WEAP;
	}
}

void Inventory_inv_update(userEntity_t *self, qboolean bOn)
{
	if (gstate->gi->TestUnicast(self,true))
	{
		gstate->InventoryUpdateClient(self, !bOn);
	}
	else
	{
		self->client->needsUpdate |= HUD_UPDATE_INVEN;
	}
}

void Inventory_Update(  userEntity_t *self )
{
	if (!AI_IsAlive(self) || IS_PAUSED)
		return;
	gclient_t	*pClient = self->client;

	if( pClient == NULL )
	{
		return;
	}

	pClient->resp.nInventoryDisableFlag = 0;
//	gstate->Con_Dprintf("Update! - Inventory mode %d\n",pClient->resp.nCurrentMode);
	switch( pClient->resp.nCurrentMode )
	{
	case INV_MODE_WEAPONS:
		pClient->resp.nInventoryEnableFlag = ACTION_WEAPONS_ENABLE;
		pClient->resp.nInventoryDisableFlag |= ACTION_ITEMS_DISABLE;
		pClient->resp.nInventoryDisableFlag |= ACTION_SIDEKICK_DISABLE;
		break;
	case INV_MODE_ITEMS:
		pClient->resp.nInventoryEnableFlag = ACTION_ITEMS_ENABLE;
		pClient->resp.nInventoryDisableFlag |= ACTION_WEAPONS_DISABLE;
		pClient->resp.nInventoryDisableFlag |= ACTION_SIDEKICK_DISABLE;
		break;
	case INV_MODE_SIDEKICK1:
	case INV_MODE_SIDEKICK2:
		pClient->resp.nInventoryEnableFlag = ACTION_SIDEKICK_ENABLE;
		pClient->resp.nInventoryDisableFlag |= ACTION_WEAPONS_DISABLE;
		pClient->resp.nInventoryDisableFlag |= ACTION_ITEMS_DISABLE;
		break;
	}

	if( ( pClient->resp.nInventoryDisableFlag & ACTION_WEAPONS_DISABLE ) && pClient->resp.nInvWeaponMode )
	{
//		pClient->needsUpdate |= HUD_UPDATE_WEAP;
		Inventory_weap_update(self,false);
		pClient->resp.nInvWeaponMode = 0; 
	}

	if( ( pClient->resp.nInventoryDisableFlag & ACTION_ITEMS_DISABLE ) && pClient->resp.nInvItemMode )
	{
//		gstate->InventoryOperate( self, TRUE );
		Inventory_inv_update(self,false);
		pClient->resp.nInvItemMode = 0; 
	}

	if( ( pClient->resp.nInventoryDisableFlag & ACTION_SIDEKICK_DISABLE ) && pClient->resp.nInvSidekickMode )
	{
		SIDEKICK_DisableCommanding( self );
		pClient->resp.nInvSidekickMode = 0; 
	}

	if( pClient->resp.nInventoryEnableFlag & ACTION_WEAPONS_ENABLE )
	{
//		pClient->needsUpdate |= HUD_UPDATE_WEAP;
		Inventory_weap_update(self,true);
		pClient->resp.nInvWeaponMode = 1;
	}
	else if( pClient->resp.nInventoryEnableFlag & ACTION_ITEMS_ENABLE )
	{
//		gstate->InventoryOperate( self, FALSE );
		Inventory_inv_update(self,true);
		pClient->resp.nInvItemMode = 1;
	}
	else if( pClient->resp.nInventoryEnableFlag & ACTION_SIDEKICK_ENABLE )
	{
		SIDEKICK_SwapCommanding( self );
		gstate->CommandSelectorNext( self );
		gstate->CommandSelectorPrev( self );
		pClient->resp.nInvSidekickMode = 1;
	}
}

void Inventory_SetMode( userEntity_t *self, int nMode, qboolean bPerformAction )
{
	if (!AI_IsAlive(self) || IS_PAUSED)
		return;

	gclient_t	*pClient = self->client;

	if( pClient == NULL )
	{
		return;
	}

	pClient->resp.nCurrentMode = nMode;

	if( bPerformAction == TRUE )
	{
		Inventory_Update( self ); 
	}
}

int Inventory_sack_count(userEntity_t *self)
{
	if (!self->inventory)
		return 0;

	int count = 0;
	invenItem_t     *item;
	userInventory_t	*data;
	invenList_t *list = (invenList_t *)self->inventory;

	for (item = list->head;item != NULL;item = item->next)
	{
		data = item->data;
		
		if (data && data->flags & ITF_INVSACK) // found match? 
			count++;
	}

	return count;
}

void Inventory_Open_f( userEntity_t *self )
{
	if (!AI_IsAlive(self) || IS_PAUSED)
		return;

	gclient_t	*pClient = self->client;

	if( pClient == NULL )
	{
		return;
	}

	if( pClient->resp.nCurrentMode == INV_MODE_SKILLS )
	{
		return;
	}

	pClient->resp.nCurrentMode++;
//	gstate->Con_Dprintf("Inventory mode %d\n",pClient->resp.nCurrentMode);

	if( pClient->resp.nCurrentMode == INV_MODE_ITEMS )
	{
		if (!Inventory_sack_count(self))
		{
			pClient->resp.nCurrentMode++;
//			gstate->Con_Dprintf("No items! - Inventory mode %d\n",pClient->resp.nCurrentMode);
		}
	}

	if( pClient->resp.nCurrentMode == INV_MODE_SIDEKICK1 )
	{
		if( AIINFO_GetSuperfly() == NULL )
		{
			pClient->resp.nCurrentMode++;
//			gstate->Con_Dprintf("No Superfly! - Inventory mode %d\n",pClient->resp.nCurrentMode);
		}
	}

	if( pClient->resp.nCurrentMode == INV_MODE_SIDEKICK2 )
	{
		if( AIINFO_GetMikiko() == NULL )
		{
			pClient->resp.nCurrentMode++;
//			gstate->Con_Dprintf("No Mikiko! - Inventory mode %d\n",pClient->resp.nCurrentMode);
		}
	}

	if( pClient->resp.nCurrentMode > INV_MODE_SIDEKICK2 )
	{
		pClient->resp.nCurrentMode = INV_MODE_WEAPONS;
//		gstate->Con_Dprintf("Wrapping! - Inventory mode %d\n",pClient->resp.nCurrentMode);
	}

	Inventory_Update( self );
}

void Inventory_Next_f( userEntity_t *self )
{
	if (!AI_IsAlive(self) || IS_PAUSED)
		return;

	gclient_t	*pClient = self->client;

	if( pClient == NULL )
	{
		return;
	}

	if (isLevelUp(self) == 1)
	{
		gstate->CommandSelectorNext( self );
		return;
	}

	switch( pClient->resp.nCurrentMode )
	{
	case INV_MODE_WEAPONS:
		gstate->CBuf_AddText( "weapnext" );
		break;
	case INV_MODE_ITEMS:
		gstate->InventoryNext( self );
		break;
	case INV_MODE_SIDEKICK1:
	case INV_MODE_SIDEKICK2:
	case INV_MODE_SKILLS:
		gstate->CommandSelectorNext( self );
		pClient->resp.nUseableSelection++;
		if( pClient->resp.nUseableSelection > 5 ) pClient->resp.nUseableSelection = 0;
		break;
	}
}

void Inventory_Prev_f( userEntity_t *self )
{
	if (!AI_IsAlive(self) || IS_PAUSED)
		return;

	gclient_t	*pClient = self->client;

	if( pClient == NULL )
	{
		return;
	}

	if (isLevelUp(self) == 1)
	{
		gstate->CommandSelectorPrev( self );
		return;
	}

	switch( pClient->resp.nCurrentMode )
	{
	case INV_MODE_WEAPONS:
		gstate->CBuf_AddText( "weapprev" );
		break;
	case INV_MODE_ITEMS:
		gstate->InventoryPrev( self );
		break;
	case INV_MODE_SIDEKICK1:
	case INV_MODE_SIDEKICK2:
	case INV_MODE_SKILLS:
		gstate->CommandSelectorPrev( self );
		pClient->resp.nUseableSelection--;
		if( pClient->resp.nUseableSelection < 0 ) pClient->resp.nUseableSelection = 5;
		break;
	}
}

void Inventory_Use_f( userEntity_t *self )
{
	if (!AI_IsAlive(self) || IS_PAUSED)
		return;

	gclient_t	*pClient = self->client;

	if( pClient == NULL )
	{
		return;
	}

	if (isLevelUp(self) == 1)
	{
		gstate->CommandSelectorApply( self );
		return;
	}

	switch( pClient->resp.nCurrentMode )
	{
	case INV_MODE_WEAPONS:
		break;
	case INV_MODE_ITEMS:
		gstate->InventoryUse( self );
		break;
	case INV_MODE_SIDEKICK1:
		gstate->CommandSelectorApply( self );
		break;
	case INV_MODE_SIDEKICK2:
		gstate->CommandSelectorApply( self );
		break;
	case INV_MODE_SKILLS:
		gstate->CommandSelectorApply( self );
		break;
	}
}



