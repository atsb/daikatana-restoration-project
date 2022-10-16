#include "p_global.h"
#include "p_user.h"
#include "hooks.h"
#include "p_inventory.h"

void *DLL_FindFunction(char *funcName);

static	char	*ignore_List [100];
static	int		ignore_Count;

///////////////////////////////////////////////////////////////////////////////
//	Ignore_FreeList
//
///////////////////////////////////////////////////////////////////////////////

void	Ignore_FreeList (void)
{
	int		i;

	for (i = 0; i < ignore_Count; i++)
		gi.X_Free ((void *) ignore_List [i]);
}

///////////////////////////////////////////////////////////////////////////////
//	Ignore_AddName
//
///////////////////////////////////////////////////////////////////////////////

void	Ignore_AddName (char *line, int char_count, int is_comment)
{
	int		i;

	//	strip spaces from the end of the name
	for (i = char_count; i > 0; i--)
	{
		if (line [i] == ' ')
		{
			line [i] = 0x00;
			char_count--;
		}
		else 
			break;
	}

	if (is_comment == false && char_count > 0)
	{
		//	end of line, so copy current line 
		line [char_count] = 0x00;
		ignore_List [ignore_Count] = (char *) gi.X_Malloc(strlen (line) + 1, MEM_TAG_TEMP );
		strcpy (ignore_List [ignore_Count], line);
		ignore_Count++;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	Ignore_CheckCommandLine
//
///////////////////////////////////////////////////////////////////////////////


char *Ignore_CheckCommandLine (void)
{
	int	i;

    for (i = 1; i < gi.EXEGetArgc() - 1; i++)
    {
		if (!gi.EXEGetArgv(i) || !gi.EXEGetArgv(i + 1))
            continue;      

        if (!stricmp ("-ignore", gi.EXEGetArgv(i)))
		{
			if (gi.EXEGetArgv(i + 1))
				return	gi.EXEGetArgv (i + 1);
		}
	}

	return	NULL;
}
	
///////////////////////////////////////////////////////////////////////////////
//	Ignore_LoadList
//
///////////////////////////////////////////////////////////////////////////////

void	Ignore_LoadList (void)
{
	FILE	*file;
	char	*fname;
	int		file_start, file_len;
	char	line [256];
	char	ch;
	int		char_count, is_comment;

	//	determine if there is a commandline override for the ignore file
	
//	sprintf (fname, "ignore.txt", gi.dk_gamedir);
	fname = Ignore_CheckCommandLine ();
	if (fname)
		file_len = gi.FS_Open (fname, &file);
	else
		file_len = gi.FS_Open ("ignore.txt", &file);
	
	if (file_len < 0)
		return;

	file_start = gi.FS_Tell (file);
	gi.FS_Seek (file, file_start, SEEK_SET);

	char_count = ignore_Count = 0;
	is_comment = false;

	while (file_len > 0)
	{
		ch = gi.FS_Getc (file);
		if (char_count == 0 && (ch == '/' || ch == ';'))
			is_comment = true;

		if (ch != '\r' && ch != '\n')
		{
			//	don't add CR/LF to endm
			line [char_count] = ch;
			char_count++;
		}

		if (ch == '\n')
		{
			Ignore_AddName (line, char_count, is_comment);

			char_count = 0;
			is_comment = false;
		}

		file_len--;
	}

	//	make sure we don't leave out the last line if there is no
	//	carriage return
	if (char_count > 0)
	{
		Ignore_AddName (line, char_count, is_comment);
	}

	gi.FS_Close (file);
}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

static	char *Ignore_strstr (char *string, char *subString)
{
	char	sub [1024];
	char	str [1024];
	int		i;

	if (strlen (subString) > 1024 || strlen (subString) > strlen (string) || strlen (string) > 1024)
		return	NULL;

	for (i = 0; i < strlen (subString) + 1; i++)
	{
		if (i >= 'a' && i <= 'z')
			sub [i] = subString [i] - 0x20;
		else
			sub [i] = subString [i];
	}

	for (i = 0; i < strlen (string) + 1; i++)
	{
		if (i >= 'a' && i <= 'z')
			str [i] = string [i] - 0x20;
		else
			str [i] = string [i];
	}

	return	strstr (str+1, sub);
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

static int Ignore_ShouldIgnore (char *str)
{
	int i;

	for (i = 0; i < ignore_Count; i++)
	{
		if (ignore_List [i][0] = '*')
		{
			if (Ignore_strstr (str, ignore_List [i] + 1) != NULL)
				return	TRUE;
		}
		else if (!stricmp(str, ignore_List[i]))
			return	TRUE;
	}

	return	FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

static char *Sent_EpairEdict(const char *data, edict_t *ent)
{
	userEpair_t  tpair[100];
	int          tpair_count;
//	qboolean	init;
	char		keyname[256];
	char       *com_token;
	int			n, i;

//	init = false;
	tpair_count = 0;

	// go through all the dictionary pairs
	while (1)
	{	
		com_token = COM_Parse ((char **)&data);

		// see if we are done
		if (com_token[0] == '}') break;

		if (!data)
		{
			Sys_Error ("Sent_ParseEdict: EOF without closing brace");
		}
		
		// make sure we have room left in tpair
		if (tpair_count >= 100)
		{
			Sys_Error("Sent_EpairEdict: more than 100 epairs found.");
		}

        // FIXME: change light to _light to get rid of this hack
        if (!strcmp(com_token, "light"))
		{
			// hack for single light def
	        strcpy (com_token, "light_lev");
		}

		strcpy (keyname, com_token);

		// another hack to fix heynames with trailing spaces
		n = strlen(keyname);
		while (n && keyname[n-1] == ' ')
		{
			keyname[n-1] = 0;
			n--;
		}

	// parse value	
		com_token = COM_Parse ((char **)&data);

		if (!data)
			Sys_Error ("ED_ParseEntity: EOF without closing brace");

		if (com_token[0] == '}')
			Sys_Error ("ED_ParseEntity: closing brace without data");

//		init = true;	

// keynames with a leading underscore are used for utility comments,
// and are immediately discarded by quake
		if (keyname[0] == '_')
		{
			// amw - I need the _color key on some of the entities to specify color
			if (stricmp(keyname,"_color") != 0)
				continue;
		}
		// fill in epair
		tpair[tpair_count].key   = (char *) gi.X_Malloc(strlen(keyname) + 1, MEM_TAG_COM_EPAIR);
		if( tpair [tpair_count].key == NULL )
			continue;
		tpair[tpair_count].value = (char *) gi.X_Malloc(strlen(com_token) + 1, MEM_TAG_COM_EPAIR);
		if( tpair [tpair_count].value == NULL )
			continue;

		strcpy(tpair[tpair_count].key, keyname);
		strcpy(tpair[tpair_count].value, com_token);

		tpair_count++;
	}

	// ok set up epair in entity
	ent->epair = (userEpair_t *) gi.X_Malloc((tpair_count + 1) * sizeof(userEpair_t), MEM_TAG_COM_EPAIR);

	// copy pointers
	for (i = 0; i < tpair_count; i++)
	{
		ent->epair[i].key   = tpair[i].key;
		ent->epair[i].value = tpair[i].value;
	}

	// make sure end ends in NULL
	ent->epair[tpair_count].key   = NULL;
	ent->epair[tpair_count].value = NULL;

	return (char *)data;
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

static char *Sent_GetValue(userEpair_t *epair, char *key)
{
	int i;

	for (i = 0; epair[i].key; i++)
	{
		if (!Q_strcasecmp(epair[i].key, key) )
		{
			return(epair[i].value);
		}
	}

	return(NULL);
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

static void Sent_BasicEpair(edict_t *ent)
{
	int i;
	char *key, *val;

	for (i = 0; ent->epair[i].key; i++)
	{
		key = ent->epair[i].key;
		val = ent->epair[i].value;

	    if (!Q_strcasecmp(key, "origin"))
	    {
		    sscanf(val, "%f%f%f", 
			    &ent->s.origin.x,
			    &ent->s.origin.y,
			    &ent->s.origin.z);
	    }

	    // angle
	    else 
		if (!Q_strcasecmp(key, "angle"))
	    {
		    ent->s.angles.x = 0.0;
		    ent->s.angles.y = atof(val);
		    ent->s.angles.z = 0.0;
	    }

	    // spawnflags
	    else 
		if (!Q_strcasecmp(key, "spawnflags"))
		{
		    ent->spawnflags = atol(val);
		}
		// model, for brush models
	    else 
		if (!Q_strcasecmp(key, "model"))
			ent->modelName = val;

		else 
		if (!Q_strcasecmp (key, "target"))
			ent->target = val;

		else 
		if (!Q_strcasecmp (key, "targetname"))
			ent->targetname = val;

		else 
		if (!Q_strcasecmp (key, "nodetargetname"))
			ent->nodeTargetName = val;

		//&&& AMW 6.11.98 - link entities to their parent entities if defined
		else 
		if (!Q_strcasecmp (key, "parenttarget"))  
			ent->parentname	= val;
		else 
		if (!Q_strcasecmp (key, "parentname"))  
			ent->parentname	= val;
		
	    //&&& AMW 6.12.98 - generic linking/grouping key
		else 
		if (!Q_strcasecmp(key, "group"))
			ent->groupname = val;

		// TEMPORARY HACK... EVENTUALLY NEEDS TO BE REMOVED
		else 
		if (!Q_strcasecmp(key, "team"))
			ent->groupname = val;

	    //&&& AMW 6.12.98 - generic linking/grouping key
		else 
		if (!Q_strcasecmp(key, "triggerindex"))
			ent->triggerIndex = atoi(val);

	    //&&& AMW 7.8.98 - generic setting for health
		else 
		if (!Q_strcasecmp(key, "health"))
			ent->health = atoi(val);

	    //&&& AMW 7.14.98 - generic setting for mass
		else 
		if (!Q_strcasecmp(key, "mass"))
			ent->mass = atof(val);

/*	*** NUKE THIS SOON ***
	    // jas 9-18-98 - send sound definition...
		else 
		if (!Q_strcasecmp(key, "sounddef"))
		{ 
			ISoundDef* pDef = g_SoundDefs.Lookup (val) ;
			if (pDef)
	            ent->s.soundDefID = pDef->m_nIndex ;
		}
*/
		// mdm 98.01.19 - bad john, we do that elsewhere
//	    // jas 9-28-98 - send sound...
//		else 
//		if (!Q_strcasecmp(key, "sound"))
//		{ 
//			ent->s.sound = gi.soundindex (val);
//		}

        //check for script name key.  Also mirror Insoo's uniqueid here.
        else 
		if ((Q_strcasecmp(key, "scriptname") == 0) ||
            (Q_strcasecmp(key, "uniqueid") == 0)) {
            //set the script name.
            ent->scriptname = val;
        }

		//check for spawn name key.
        else 
		if (Q_strcasecmp(key, "spawnname") == 0) {
            //set the script name.
            ent->spawnname = val;
        }

		//check for key name key.
        else 
		if (Q_strcasecmp(key, "keyname") == 0) {
            //set the script name.
            ent->keyname = val;
        }
					
		//	Nelno:	don't fill in sky any longer?
/*
		else if (!Q_strcasecmp(key, "sky"))
		{
			strcpy(sv.skyname, val);
		}
*/
	}
}

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

#define	SPAWNFLAG_NOT_COOP			0x00001000
typedef	(*func_t)(edict_t *);

void ParseWeaponData( userEntity_t *pEnt, client_persistant_t *pPersistant, userInventory_t *pInv )
{
	ammo_t			*pAmmo;
	weapon_t		*pWeapon;
	unsigned int	nWeaponFlags, nAmmoIndex;

	// SCG[11/13/99]: Get a pointer to the weapon
	pWeapon = ( weapon_t * ) pInv;

	// SCG[11/29/99]: Check for the daikatana
	if( strcmp( pWeapon->name, "weapon_daikatana" ) == 0 )
	{
		pPersistant->nWeaponFlags |= PERS_WEAP_WEAPON7;
		if (pEnt->curWeapon == pInv)
			pPersistant->nCurrentWeaponFlag = PERS_WEAP_WEAPON7;
		return;
	}

	// SCG[2/14/00]: check for gashands
	if( strcmp( pWeapon->name, "weapon_gashands" ) == 0 )
	{
		pPersistant->nWeaponFlags |= PERS_WEAP_WEAPON8;
		if (pEnt->curWeapon == pInv)
			pPersistant->nCurrentWeaponFlag = PERS_WEAP_WEAPON8;
		return;
	}

	unsigned int count = 0;
	if ( (serverState.episode == 4) && (!stricmp(pWeapon->name,"weapon_slugger")) )
	{
		// cordite | slugs
		// get the ammo from the inventory
		ammo_t *ammoc = (ammo_t *)serverState.InventoryFindItem (pEnt->inventory, "ammo_cordite");
		ammo_t *ammos = (ammo_t *)serverState.InventoryFindItem (pEnt->inventory, "ammo_slugger");
		if (ammoc && ammoc->count && (pEnt->flags & FL_CLIENT))
		{
			count = ammoc->count << 16;
		}
		if (ammos && ammos->count)
		{
			count += ammos->count;
		}
	}
	else
	{
		// SCG[11/13/99]: Get a pointer to the ammo
		pAmmo = ( ammo_t * ) pWeapon->ammo;

		if (pAmmo)
			count = pAmmo->count;
	}

	// SCG[11/13/99]: Set the weapon flag based on pWeapon->winfo->display_order
	nWeaponFlags = (1 << pWeapon->winfo->display_order);

	// SCG[11/13/99]: Set the ammo flag based on pWeapon->winfo->display_order
	nAmmoIndex = pWeapon->winfo->display_order;

	// SCG[11/8/99]: Set weapon info
	pPersistant->nWeaponFlags |= nWeaponFlags;

	// SCG[11/8/99]: Set ammo info
	pPersistant->nAmmo[nAmmoIndex] = count;//pAmmo->count;


	if( pEnt->curWeapon == pInv )
	{
		pPersistant->nCurrentWeaponFlag = nWeaponFlags;
	}
}

byte P_InventoryItemCount(invenList_t *list, char *item_name);
void P_InventorySave( void **buf, userEntity_t *self, int aryEntries );

void ParseSpecialData( userEntity_t *pEnt, client_persistant_t *pPersistant, userInventory_t *pInv )
{
	unsigned int	nInventoryFlags = 0;

	if( strcmp( pInv->name, "item_megashield" ) == 0 )				{ nInventoryFlags = ITEM_MEGASHIELD; }
	if( strcmp( pInv->name, "item_invincibility" ) == 0 )			{ nInventoryFlags = ITEM_INVINCIBILITY; }
return;

	if( strcmp( pInv->name, "item_savegem" ) == 0 )					
	{ 
		int nSavegemCount = P_InventoryItemCount( pEnt->inventory, "item_savegem" );
		for( int i = 0; i < nSavegemCount; i++ )
		{
			nInventoryFlags |= ITEM_SAVEGEM_1 << i;
		}
	}

	switch( serverState.episode )
	{
	case 1:
		break;
	case 2:
		if( strcmp( pInv->name, "item_antidote" ) == 0 )			{ nInventoryFlags = ITEM_ANTIDOTE; break; }
		if( strcmp( pInv->name, "item_drachma" ) == 0 )				{ nInventoryFlags = ITEM_DRACHMA; break; }
		if( strcmp( pInv->name, "item_horn" ) == 0 )				{ nInventoryFlags = ITEM_HORN; break; }
		if( strcmp( pInv->name, "item_rune_a" ) == 0 )				{ nInventoryFlags = ITEM_RUNE_A; break; }
		if( strcmp( pInv->name, "item_rune_e" ) == 0 )				{ nInventoryFlags = ITEM_RUNE_E; break; }
		if( strcmp( pInv->name, "item_rune_g" ) == 0 )				{ nInventoryFlags = ITEM_RUNE_G; break; }
		if( strcmp( pInv->name, "item_rune_i" ) == 0 )				{ nInventoryFlags = ITEM_RUNE_I; break; }
		if( strcmp( pInv->name, "item_rune_s" ) == 0 )				{ nInventoryFlags = ITEM_RUNE_S; break; }
		break;
	case 3:
		if( strcmp( pInv->name, "item_wyndrax_key" ) == 0 )			{ nInventoryFlags = ITEM_WYNDRAX_KEY; break; }
		if( strcmp( pInv->name, "item_spellbook" ) == 0 )			{ nInventoryFlags = ITEM_SPELLBOOK; break; }
// SCG[11/17/99]: Removed per John's request
//		if( strcmp( pInv->name, "item_evilbook" ) == 0 )			{ nInventoryFlags = ITEM_EVILBOOK; break; }
		if( strcmp( pInv->name, "item_crypt_key" ) == 0 )			{ nInventoryFlags = ITEM_CRYPT_KEY; break; }
		if( strcmp( pInv->name, "item_purifier_shard1" ) == 0 )		{ nInventoryFlags = ITEM_PURIFIER_SHARD1; break; }
		if( strcmp( pInv->name, "item_purifier_shard2" ) == 0 )		{ nInventoryFlags = ITEM_PURIFIER_SHARD2; break; }
		if( strcmp( pInv->name, "item_purifier_shard3" ) == 0 )		{ nInventoryFlags = ITEM_PURIFIER_SHARD3; break; }
		if( strcmp( pInv->name, "item_hex_keystone" ) == 0 )		{ nInventoryFlags = ITEM_HEX_KEYSTONE; break; }
		if( strcmp( pInv->name, "item_quad_keystone" ) == 0 )		{ nInventoryFlags = ITEM_QUAD_KEYSTONE; break; }
		if( strcmp( pInv->name, "item_trigon_keystone" ) == 0 )		{ nInventoryFlags = ITEM_TRIGON_KEYSTONE; break; }
// SCG[11/17/99]: Removed per John's request
//		if( strcmp( pInv->name, "item_ice_boots" ) == 0 )			{ nInventoryFlags = ITEM_ICE_BOOTS; break; }
		if( strcmp( pInv->name, "item_ring_of_fire" ) == 0 )		{ nInventoryFlags = ITEM_RING_OF_FIRE; break; }
		if( strcmp( pInv->name, "item_ring_of_undead" ) == 0 )		{ nInventoryFlags = ITEM_RING_OF_UNDEAD; break; }
		if( strcmp( pInv->name, "item_ring_of_lightning" ) == 0 )	{ nInventoryFlags = ITEM_RING_OF_LIGHTNING; break; }
		break;
	case 4:
		if( strcmp( pInv->name, "item_envirosuit" ) == 0 )			{ nInventoryFlags = ITEM_ENVIROSUIT; break; }
		if( strcmp( pInv->name, "item_control_card_red" ) == 0 )	{ nInventoryFlags = ITEM_CONTROL_CARD_RED; break; }
		if( strcmp( pInv->name, "item_control_card_blue" ) == 0 )	{ nInventoryFlags = ITEM_CONTROL_CARD_BLUE; break; }
		if( strcmp( pInv->name, "item_control_card_yellow" ) == 0 )	{ nInventoryFlags = ITEM_CONTROL_CARD_YELLOW; break; }
		if( strcmp( pInv->name, "item_control_card_green" ) == 0 )	{ nInventoryFlags = ITEM_CONTROL_CARD_GREEN; break; }

		if( strcmp( pInv->name, "item_sulphur" ) == 0 )				{ nInventoryFlags = ITEM_SULPHUR; break; }
		if( strcmp( pInv->name, "item_charcoal" ) == 0 )			{ nInventoryFlags = ITEM_CHARCOAL; break; }
		if( strcmp( pInv->name, "item_saltpeter" ) == 0	)			{ nInventoryFlags = ITEM_SALTPETER; break; }
		if( strcmp( pInv->name, "item_bottle" ) == 0 )				{ nInventoryFlags = ITEM_BOTTLE; break; }
		if( strcmp( pInv->name, "item_bomb" ) == 0 )				{ nInventoryFlags = ITEM_BOMB; break; }
		break;
	}

	pPersistant->nInventoryFlags |= nInventoryFlags;
}

void SavePersistantInventory( userEntity_t *pEnt )
{
	client_persistant_t	*pPersistant;
	playerHook_t		*pHook;
	pHook = ( playerHook_t * ) pEnt->userHook;

	if( pHook == NULL )
	{
		return;
	}

	pPersistant = &pEnt->client->pers;

	_ASSERTE( pEnt->inventory );

	// SCG[11/13/99]: clear out the weapon flags;
	// cek[2-15-00]: need to clear the flags...or we can have things that got removed 'show up' again.
	pPersistant->nWeaponFlags = 0;
	pPersistant->nCurrentWeaponFlag = 0;
	pPersistant->nInventoryFlags = 0;
	memset(&pPersistant->nAmmo,0,sizeof(pPersistant->nAmmo));

	userInventory_t *pInv = P_InventoryFirstItem( pEnt->inventory );
	while( pInv )
	{
		if( pInv->flags & ITF_WEAPON )
		{
			// SCG[11/13/99]: weapon data
			ParseWeaponData( pEnt, pPersistant, pInv );
		}
/*
		else if( pInv->flags & ( ITF_SPECIAL | ITF_INVSACK ) )
		{
			// SCG[11/13/99]: keys and bomb parts
//			ParseSpecialData( pEnt, pPersistant, pInv );//cek[2-22-00]
		}
*/
		pInv = P_InventoryNextItem( pEnt->inventory );
	}

	// do it again, this time with ammo...
	pInv = P_InventoryFirstItem( pEnt->inventory );
	while( pInv )
	{
		if( pInv->flags & ITF_AMMO )
		{
			ammo_t *ammo = (ammo_t *)pInv;
			if (!pPersistant->nAmmo[ammo->winfo->display_order])
				pPersistant->nAmmo[ammo->winfo->display_order] = ammo->count;
		}
		pInv = P_InventoryNextItem( pEnt->inventory );
	}

	void *inv = (void *)pPersistant->inventoryCrap;
	P_InventorySave( &inv, pEnt, MAX_PERS_INVENTORY);
}


/*
==================
SaveClientData

Some information that should be persistant, like health, 
is still stored in the edict structure, so it needs to
be mirrored out to the client structure before all the
edicts are wiped.
==================
*/
#define BOUND_MIN(val,min)		if (val < min) val = min

userEntity_t *com_FindEntity( const char *name );
void SaveClientData (void)
{
	int					i, j, nCount = 1;
	edict_t				*pEntArray[3];
//	client_persistant_t	*pPersistantArray[3];
	client_persistant_t	*pPersistant;
	playerHook_t		*pHook;
	CVector				vDir;

	pEntArray[0] = pEntArray[1] = pEntArray[2] = NULL;	// WAW[11/29/99]: Needs to be NULLed out due to network stuff.

	for( i = 0; i < game.maxclients; i++ )
	{
		pEntArray[0] = &g_edicts[1 + i];
	
		if( !deathmatch->value && !coop->value )
		{
			nCount = 3;
			pEntArray[1] = com_FindEntity( "Superfly" );
			pEntArray[2] = com_FindEntity( "Mikiko" );
		}

		for( j = 0; j < nCount; j++ )
		{
			if (!pEntArray[j])
			{
				continue;
			}

			if (!pEntArray[j]->inuse)
			{
				continue;
			}

			pHook = ( playerHook_t * ) pEntArray[j]->userHook;

			if( pHook == NULL )
			{
				continue;
			}

			pPersistant = &pEntArray[j]->client->pers;

			pPersistant->nHealth				= pEntArray[j]->health;
												
			pPersistant->record					= pEntArray[j]->record;
												
			pPersistant->nArmorVal				= pEntArray[j]->armor_val;
			pPersistant->nArmorAbs				= pEntArray[j]->armor_abs;
												
			pPersistant->nTeam					= pEntArray[j]->team;
												
			pPersistant->nItems					= pHook->items;
			pPersistant->nExpFlags				= pHook->exp_flags;
			pPersistant->fInvulnerabilityTime	= pHook->invulnerability_time;
			pPersistant->fEnvirosuitTime		= pHook->envirosuit_time;
			pPersistant->fWraithorbTime			= pHook->wraithorb_time;

			pPersistant->fBasePower				= pHook->base_power;
			pPersistant->fBaseAttack			= pHook->base_attack;
			pPersistant->fBaseSpeed				= pHook->base_speed;
			pPersistant->fBaseAcro				= pHook->base_acro;
			pPersistant->fBaseVita				= pHook->base_vita;
												
			pPersistant->fBoostPower			= pHook->power_boost;
			pPersistant->fBoostAttack			= pHook->attack_boost;
			pPersistant->fBoostSpeed			= pHook->speed_boost;
			pPersistant->fBoostAcro				= pHook->acro_boost;
			pPersistant->fBoostVita				= pHook->vita_boost;
												
			pPersistant->fTimePower				= pHook->power_boost_time;
			pPersistant->fTimeAttack			= pHook->attack_boost_time;
			pPersistant->fTimeSpeed				= pHook->speed_boost_time;
			pPersistant->fTimeAcro				= pHook->acro_boost_time;
			pPersistant->fTimeVita				= pHook->vita_boost_time;
												
			pPersistant->fPoisonTime			= pHook->poison_time;
			pPersistant->fPoisonNextDamage		= pHook->poison_next_damage_time;
			pPersistant->fPoisonDamage			= pHook->poison_damage;
			pPersistant->fPoisonInterval		= pHook->poison_interval;

			pPersistant->vAngles				= pEntArray[j]->s.angles;

			pPersistant->nAIFlags				= pHook->ai_flags;

			BOUND_MIN(pPersistant->fTimePower				,0);
			BOUND_MIN(pPersistant->fTimeAttack				,0);
			BOUND_MIN(pPersistant->fTimeSpeed				,0);
			BOUND_MIN(pPersistant->fTimeAcro				,0);
			BOUND_MIN(pPersistant->fTimeVita				,0);
			BOUND_MIN(pPersistant->fInvulnerabilityTime		,0);
			BOUND_MIN(pPersistant->fEnvirosuitTime			,0);
			BOUND_MIN(pPersistant->fWraithorbTime			,0);

			SavePersistantInventory( pEntArray[j] );

/*
			// some stuff in the player persistant for monsters/secrets/etc.
			if ((j == 0) && (serverState.numMonsters || serverState.numMonstersKilled || serverState.numSecrets || serverState.numSecretsFound))
			{
				pPersistant->nTotalMonsters[serverState.subMap] = serverState.numMonsters;
				pPersistant->nTotalKills[serverState.subMap] = serverState.numMonstersKilled;

				pPersistant->nTotalSecrets[serverState.subMap] = serverState.numSecrets;
				pPersistant->nFoundSecrets[serverState.subMap] = serverState.numSecretsFound;

				pPersistant->nTime[serverState.subMap] = serverState.level->time;
			}
			else if (j != 0)
			{
				pPersistant->nTotalMonsters[serverState.subMap] = 0;
				pPersistant->nTotalKills[serverState.subMap] = 0;
				pPersistant->nTotalSecrets[serverState.subMap] = 0;
				pPersistant->nFoundSecrets[serverState.subMap] = 0;
				pPersistant->nTime[serverState.subMap] = 0;
			}
*/
		}
		// SCG[10/26/99]: sidekick persistant info is stored with the client because 
		// SCG[10/26/99]: the sidekicks (and their persistant info) get erased
		// SCG[10/26/99]: before we have a chance to restore it upon level load.
		// SCG[10/26/99]: The data alloc'd here is removed after the persistant data is restored.
		if( !deathmatch->value && !coop->value )
		{
			if( ( pEntArray[0] != NULL ) && ( pEntArray[0]->client != NULL ) )
			{
				// SCG[11/30/99]: Superfly origin
				if( pEntArray[1] != NULL )
				{
					pEntArray[1]->client->pers.vOffset = pEntArray[1]->s.origin;
				}

				// SCG[11/30/99]: Mikiko origin
				if( pEntArray[2] != NULL )
				{
					pEntArray[2]->client->pers.vOffset = pEntArray[2]->s.origin;
				}

				// SCG[11/30/99]: Superfly persistant data
				if( ( pEntArray[1] != NULL ) && ( pEntArray[1]->client != NULL ) )
				{
					if( pEntArray[0]->client->pers.pPersSuperfly == NULL )
					{
						pEntArray[0]->client->pers.pPersSuperfly = 
							( client_persistant_t * ) gi.X_Malloc( sizeof( client_persistant_t ), MEM_TAG_TEMP );
					}
					memcpy( pEntArray[0]->client->pers.pPersSuperfly, 
						&pEntArray[1]->client->pers, sizeof( client_persistant_t ) );
				}

				// SCG[11/30/99]: Mikiko persistant data
				if( ( pEntArray[2] != NULL ) && ( pEntArray[2]->client != NULL ) )
				{
					if( pEntArray[0]->client->pers.pPersMikiko == NULL )
					{
						pEntArray[0]->client->pers.pPersMikiko = 
							( client_persistant_t * ) gi.X_Malloc( sizeof( client_persistant_t ), MEM_TAG_TEMP );
					}
					memcpy( pEntArray[0]->client->pers.pPersMikiko, 
						&pEntArray[2]->client->pers, sizeof( client_persistant_t ) );
				}
			}
		}
	}
}

cvar_t *dm_weap_noallow;
#define ARY_OFFSET(ep)				( (ep - 1) * 6 )
#define FLAGS(ep)					( (((unsigned long)dm_weap_noallow->value) >> (6*(ep - 1))) & 0x0000003F )
#define ALLOWED(ep,item)			( ( FLAGS(ep) & (1 << (item)) ) ? FALSE : TRUE )
static char *weapons[] =
{
	"",
	"weapon_ionblaster",
	"weapon_c4viz",
	"weapon_shotcycler",
	"weapon_sidewinder",
	"weapon_shockwave",

	"weapon_discus",
	"weapon_venomous",
	"weapon_sunflare",
	"weapon_hammer",
	"weapon_trident",
	"weapon_zeus",

	"",
	"weapon_bolter",
	"weapon_stavros",
	"weapon_ballista",
	"weapon_wyndrax",
	"weapon_nightmare",

	"weapon_glock",
	"weapon_slugger",
	"weapon_kineticore",
	"weapon_ripgun",
	"weapon_novabeam",
	"weapon_metamaser"
};

static char *ammo[] =
{
	"",
	"ammo_ionpack",
	"ammo_c4",
	"ammo_shells",
	"ammo_rockets",
	"ammo_shocksphere",

	"",
	"ammo_venomous",
	"",
	"",
	"ammo_tritips",
	"ammo_zeus",

	"",
	"ammo_bolts",
	"ammo_stavros",
	"ammo_ballista",
	"ammo_wisp",
	"",

	"ammo_bullets",
	"ammo_slugger",
	"ammo_kineticore",
	"ammo_ripgun",
	"ammo_novabeam",
	""
};

static int allow_all = -2;
char *get_className(int episode, char *className)
{
	if ( !deathmatch->value || (episode < 1) || (episode > 4) || !className)
		return className;

	bool bIsAmmo = (strnicmp(className,"ammo_",5) == 0);
	bool bIsWeap = (strnicmp(className,"weapon_",7) == 0);

	if ( !bIsAmmo && !bIsWeap)
		return className;

	if ( allow_all == -2 )
	{
		allow_all = 0;
	}

	if (allow_all == 1)
		return className;
	else if (allow_all == -1)
		return "";

	char **compare;
	if (bIsAmmo)
		compare = ammo;
	else
		compare = weapons;

	char *testName = className;
	if (bIsAmmo && (episode == 4))
	{
		if (!stricmp(className,"ammo_cordite"))
			testName = "ammo_slugger";
	}

	int i;
	int item = -1;
	int start = ARY_OFFSET(episode);
	int end = ARY_OFFSET(episode + 1);
	for (i = start; i < end; i++)
	{
		if (!stricmp(testName,compare[i]))
		{
			item = i - start;
			break;
		}
	}

	if (item == -1)
		return className;

	if (ALLOWED(episode, item))
	{
		return className;
	}
	else
	{
		for (i = item + start - 1; i >= start; i--)
		{
			if (ALLOWED(episode, i - start) && (strlen(compare[i]) > 0))
				return compare[i];
		}

		for (i = item + 1; i < end; i++)
		{
			if (ALLOWED(episode, i - start) && (strlen(compare[i]) > 0))
				return compare[i];
		}
	}

	return "";
}

void SpawnEntities( char *mapname, char *str, char *spawnpoint, qboolean bLoadgame )
{
	edict_t		*ent;
	char		*com_token;
	func_t		func;
	int			entity_count = 0;
	int			removed = 0;

	SaveClientData();
	
	// set up the submap
	char lastchar = tolower(*(mapname + strlen(mapname) - 1));
	int subMap = lastchar - 97;
	if ((subMap < 0) || (subMap >= MAX_SUBMAPS)) subMap = 0;
	serverState.subMap = subMap;

	// clear these out
	serverState.numMonsters = 0;
	serverState.numSecrets = 0;
	serverState.numMonstersKilled = 0;
	serverState.numSecretsFound = 0;

	gi.Mem_Free_Tag(MEM_TAG_LEVEL);

	gclient_t *plr = &game.clients[0];

	int		length = strlen(mapname);
	char	test = mapname[length - 1];
	if( bLoadgame )
	{
		serverState.nLevelTransitionType = 3;// loadgame
	}
	else if( ( ( mapname[1] != level.mapname[1] ) || ( level.mapname[0] == NULL ) ) && ( bLoadgame == false ) )
	{
		serverState.nLevelTransitionType = 2;// episode transition
	}
	else if( ( test != 'a' ) ||
		( ( test == 'a' ) && ( strncmp( mapname, level.mapname, length - 1 ) == 0 ) ) )
	{
		serverState.nLevelTransitionType = 1;// level transition
	}
	else
	{
		serverState.nLevelTransitionType = 0;// map transition
	}

	memset (&level, 0, sizeof(level));
	memset (g_edicts, 0, game.maxentities * sizeof (g_edicts[0]));

	memset (level.mapname, 0x00, sizeof (level.mapname));
	strncpy (level.mapname, mapname, sizeof(level.mapname)-1);

	serverState.g_edicts = g_edicts;

//	unsigned int saveMonsters = 0;
//	unsigned int saveSecrets = 0;

	if (serverState.nLevelTransitionType != 1)
	{
//		saveMonsters = 0;
//		saveSecrets = 0;
		serverState.numMonstersKilled = 0;
		serverState.numSecretsFound = 0;
	}
	else
	{
//		saveMonsters = plr->pers.nTotalMonsters[subMap];
//		saveSecrets = plr->pers.nTotalSecrets[subMap];
		serverState.numMonstersKilled = plr->pers.nTotalKills[subMap];
		serverState.numSecretsFound = plr->pers.nFoundSecrets[subMap];
	}


	// set client fields on player ents
	for( int i = 0; i < game.maxclients; i++ )
	{
		g_edicts[i + 1].client = game.clients + i;
	}

	ent = &g_edicts[0];

/*
	for (i = 0; i < game.maxentities; i++, ent++)
	{
		ent->s.render_scale.Set( 1.0f, 1.0f, 1.0f );

		if (i == 0)
		{
			ent->className = "virgin worldspawn";
			continue;
		}
		else if (i <= maxclients->value)
		{
			ent->className = "virgin client";
			continue;
		}

		ent->className = "virgin entity";

		ent->client = (gclient_t *) gi.X_Malloc (sizeof(gclient_t), MEM_TAG_GAME);
	}
*/

	ent = NULL;

	// see if we need to load up a file indicating which
	// entity not to load.  This is for nelno debugging only.
	Ignore_LoadList ();

	cvar_t *ctf			= gi.cvar( "ctf", "", 0 );
	cvar_t *coop		= gi.cvar( "coop", "", 0 );
	cvar_t *deathtag	= gi.cvar( "deathtag", "", 0 );
	dm_weap_noallow		= gi.cvar( "dm_weap_noallow","0",CVAR_SERVERINFO|CVAR_LATCH);

	// SCG[3/7/00]: 
	qboolean bRemovedForMultiplayer;
	// parse entites 
	for (;;)
	{
		// SCG[3/7/00]: 
		bRemovedForMultiplayer = FALSE;
		com_token = COM_Parse (&str);
		if (!str)
			break;
		if (com_token [0] != '{')
			gi.Error ("SpawnEntities: found %s when expecting {",com_token);
/*
		if ((com_token = COM_Parse(&str)) == NULL) break;

		if (com_token[0] != '{')
		{
			//	Nelno: don't puke, please
			gi.error ("Sent_LoadServerEntities: found %s when expecting {",com_token);
		}
*/
		if( !ent ) 
		{
			ent = g_edicts;
		}
		else
		{
			ent = P_SpawnEdict ();
		}

		entity_count++;

		// turn {} into a epair string
		str = Sent_EpairEdict (str, ent);

		// set classname
//		ent->className = Sent_GetValue(ent->epair, "classname");
		ent->className = get_className(serverState.episode,Sent_GetValue(ent->epair,"classname"));
#ifdef _DEBUG
			if ( stricmp(ent->className,Sent_GetValue(ent->epair,"classname")) )
				serverState.Con_Dprintf("Entity %s changed to %s\n",Sent_GetValue(ent->epair,"classname"),ent->className);
#endif
		//NSS[11/20/99]:Uhhhh WE HAVE to check for classnames FIRST
		if (!ent->className)
		{
			gi.Error ("Entity %i has no classname\n", entity_count);

			P_FreeEdict (ent);

			continue;
		}			
		if (Ignore_ShouldIgnore (ent->className) == FALSE)
		{
			if (!ent->className)
			{
				gi.Error ("Entity %i has no classname\n", entity_count);

				P_FreeEdict (ent);

				continue;
			}

			// fill in basic values
			Sent_BasicEpair(ent);
/*
			if (!stricmp(ent->className,"trigger_console"))
			{
				int foo = 1;
			}
*/
			// SCG[1/17/00]: This is most disgusting...
			// SCG[1/17/00]: Only spawn stuff with coop 1
			char *pCoopOnlyStr = Sent_GetValue(ent->epair, "coop");

			if( pCoopOnlyStr != NULL )
			{
				if( ( ( atoi( pCoopOnlyStr ) == 1 ) && ( coop->value == 0 ) ) ||
					( ( atoi( pCoopOnlyStr ) == 0 ) && ( coop->value == 1 ) ) )
				{
					bRemovedForMultiplayer = TRUE;
				}
			}

			char *pCtfOnlyStr = Sent_GetValue(ent->epair, "ctf");

			if( pCtfOnlyStr != NULL )
			{
				if( ( ( atoi( pCtfOnlyStr ) == 1 ) && ( ctf->value == 0 ) ) ||
					( ( atoi( pCtfOnlyStr ) == 0 ) && ( ctf->value == 1 ) ) )
				{
					bRemovedForMultiplayer = TRUE;
				}
			}

			char *pDTagOnlyStr = Sent_GetValue(ent->epair, "deathtag");

			if( pDTagOnlyStr != NULL )
			{
				if( ( ( atoi( pDTagOnlyStr ) == 1 ) && ( deathtag->value == 0 ) ) ||
					( ( atoi( pDTagOnlyStr ) == 0 ) && ( deathtag->value == 1 ) ) )
				{
					bRemovedForMultiplayer = TRUE;
				}
			}

			char *pDedicatedOnlyStr = Sent_GetValue(ent->epair, "dedicated");

			if( pDedicatedOnlyStr != NULL )
			{
				if( ( ( atoi( pDedicatedOnlyStr ) == 1 ) && ( dedicated->value == 0 ) ) ||
					( ( atoi( pDedicatedOnlyStr ) == 0 ) && ( dedicated->value == 1 ) ) )
				{
					bRemovedForMultiplayer = TRUE;
				}
			}

			char *pViolenceRemoveStr = Sent_GetValue(ent->epair, "violence");

			if( pViolenceRemoveStr != NULL )
			{
				if( ( ( atoi( pViolenceRemoveStr ) == 1 ) && ( sv_violence->value == 0 ) ) ||
					( ( atoi( pViolenceRemoveStr ) == 0 ) && ( sv_violence->value == 1 ) ) )
				{
					bRemovedForMultiplayer = TRUE;
				}
			}

			if( bRemovedForMultiplayer )
			{
				if( strcmp( ent->className, "func_areaportal" ) == 0 )
				{
					for( i = 0; ent->epair[i].key != NULL; i++ )
					{
						if( stricmp( ent->epair[i].key, "style" ) == 0 )
						{
							ent->hacks_int = atoi( ent->epair [i].value );
						}
					}

					// SCG[3/7/00]: open it up
					gi.SetAreaPortalState( ent->hacks_int, TRUE );
				}
				P_FreeEdict (ent);
				removed++;
				continue;
			}

			// find spawn function for this class
			func = (func_t) DLL_FindFunction(ent->className);

			if (func == NULL)
			{
				// if this is the worldspawn classname then this
				// is a fatal error, otherwise we can let it slide
				if (!stricmp(ent->className, "worldspawn"))
					gi.Error ("Worldspawn entity MUST have a spawn function.");
				else
					gi.Con_Dprintf("No spawn function found for %s.\n", ent->className);
			}
			else
			{
				if (ent != g_edicts)
				{
					
					//int MaxClients = Cvar_VariableValue( "maxclients" );
					
					// check for entities that shouldn't be spawned in deathmatches
					if (deathmatch->value && deathmatch->value != 98)
					{
						//	don't spawn entities in deathmatch that shouldn't be spawned
						if (ent->spawnflags & SPAWNFLAG_NOT_DEATHMATCH || (ent->className && strstr (ent->className, "monster_") == ent->className))
						{
							if( strcmp( ent->className, "func_areaportal" ) == 0 )
							{
								for( i = 0; ent->epair[i].key != NULL; i++ )
								{
									if( stricmp( ent->epair[i].key, "style" ) == 0 )
									{
										ent->hacks_int = atoi( ent->epair [i].value );
									}
								}

								// SCG[3/7/00]: open it up
								gi.SetAreaPortalState( ent->hacks_int, TRUE );
							}
							P_FreeEdict (ent);	
							removed++;

							continue;
						}
						// check for entities that shouldn't be spawned for deathmatch and max player levels
						else 
						{	
							// we don't store this in the edict_t record so retrieve it this way
							char *pMaxPlayerStr = Sent_GetValue(ent->epair, "maxplayers");

							if (pMaxPlayerStr && (maxclients->value != atoi(pMaxPlayerStr)))
							{
								P_FreeEdict (ent);	
								removed++;
								continue;
							}
						}
					}
					else
					{
						// check for entities that shouldn't be spawned for current skill level
						if ( ((skill->value == 0) && (ent->spawnflags & SPAWNFLAG_NOT_EASY)) ||
							 ((skill->value == 1) && (ent->spawnflags & SPAWNFLAG_NOT_MEDIUM)) ||
							 ((skill->value == 2) && (ent->spawnflags & SPAWNFLAG_NOT_HARD))
							)
						{
							P_FreeEdict (ent);	
							removed++;

							continue;
						}
					}

					ent->spawnflags &= ~(SPAWNFLAG_NOT_EASY | SPAWNFLAG_NOT_MEDIUM | SPAWNFLAG_NOT_HARD | SPAWNFLAG_NOT_COOP | SPAWNFLAG_NOT_DEATHMATCH);
				}

				// call spawn function
				func (ent);

				//	if this is a MOVETYPE_TOSS, MOVETYPE_BOUNCE, MOVETYPE_BOUNCEMISSILE or MOVETYPE_FLY
				//	add an epsilon to it so that it won't clip into the world if it was placed directly
				//	on the floor
#define	DIST_EPSILON	(0.03125)            // MIKE: didn't compile without this, so, I added it...
												 // don't know if it's the correct #define (there were several
												 // _EPSILON #defs) but, it compiles...

				if( ( ent->movetype == MOVETYPE_TOSS ) || 
					( ent->movetype == MOVETYPE_BOUNCE ) || 
					( ent->movetype == MOVETYPE_BOUNCEMISSILE ) || 
					( ent->movetype == MOVETYPE_FLY ) || 
					( ent->movetype == MOVETYPE_HOVER ) ||
					( ent->movetype == MOVETYPE_FLOAT ) || 
					( ent->movetype == MOVETYPE_WALK ) || 
					( ent->movetype == MOVETYPE_WHEEL ) || 
					( ent->movetype == MOVETYPE_HOP ) || 
					( ent->movetype == MOVETYPE_SWIM ) )
					ent->s.origin.z += DIST_EPSILON;
// this messes up area portals..  just need to make sure that everything that needs
// this called (like sound_ambient) is doing so...
#if 0
				gi.linkentity (ent);
#endif
			}
		}
		else 
		{
			//gi.Con_Dprintf("Ignoring %s.\n", ent->className);
			ent->remove (ent);
		}
	}

//	if (saveMonsters)
//		serverState.numMonsters = saveMonsters;

//	if (saveSecrets)
//		serverState.numSecrets = saveSecrets;

	Ignore_FreeList ();
}
