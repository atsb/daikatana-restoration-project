
#include "p_global.h"
#include "p_inventory.h"

#define MAX_FUNC	4096

typedef struct 
{
	char	name[64];
//	int		index;
	void	*func;
} func_t;

func_t func_list[MAX_FUNC];

///////////////////////////////////////////////////////////////////////////////
//
//  Initialize our function list
//
///////////////////////////////////////////////////////////////////////////////
static int	g_nIndex;

void P_InitFuncList()
{
	memset(&func_list,0,sizeof(func_t)*MAX_FUNC);
	g_nIndex = 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//  Register a function pointer for save/load
//
///////////////////////////////////////////////////////////////////////////////
void P_RegisterFunc( char* name, void* func )
{
	int i;
	
	for( i = 0; i < MAX_FUNC; i++ )
	{
		// SCG[10/5/99]: duplicate. not added
		if( strncmp( func_list[i].name, name, 64 ) == 0 )
		{
			return;
		}

		if( !func_list[i].func )
		{
			strncpy( func_list[i].name, name, 64 );
			func_list[i].func = func;
//			func_list[i].index = g_nIndex;
//			g_nIndex++;
			return;
		}
	}
	
	gi.Error( "Couldn't register function.\n" );
	_ASSERTE(FALSE);
}

char* P_GetStringForFunc( void *func )
{
	if( func != NULL )
	{
		for( int i = 0; i < MAX_FUNC; i++ )
		{
			if (func_list[i].func == func)
			{
				if( func_list[i].name[0] != NULL )
				{
//					gi.Con_Dprintf("Saved string for function: " );
					return func_list[i].name;
				}
				else
				{
//					gi.Error("No string for function: " );
					return NULL;
				}
			}
		}
	}

	return NULL;
}

int P_GetIndexForFunc( void *func )
{
	if( func != NULL )
	{
		for( int i = 0; i < MAX_FUNC; i++ )
		{
			if( func_list[i].func == func )
			{
//				if( func_list[i].index != -1 )
				{
//					return func_list[i].index;
					return i;
				}
/*				else
				{
					return -1;
				}
*/
			}
		}
	}

	return -1;
}

void* P_GetFuncForString(char *string)
{
	int	len = strlen( string );

	for( int i = 0; i < MAX_FUNC; i++ )
	{
		if( strncmp( func_list[i].name, string, len ) == 0 )
		{
			return func_list[i].func;
		}
	}

	return NULL;
}

void* P_GetFuncForIndex( int index )
{
	if( index != -1 )
	{
		return func_list[index].func;
	}
/*
	for( int i = 0; i < MAX_FUNC; i++ )
	{
		if( func_list[i].index == index )
		{
			return func_list[i].func;
		}
	}
*/
	return NULL;
}

// SCG[11/19/99]: ----------------------------------------------------------------
// SCG[11/19/99]: Save game crap
// SCG[11/19/99]: ----------------------------------------------------------------

#define	FOFS(x) (int)&(((edict_t *)0)->x)
#define	STOFS(x) (int)&(((spawn_temp_t *)0)->x)
#define	LLOFS(x) (int)&(((level_locals_t *)0)->x)
#define	CLOFS(x) (int)&(((gclient_t *)0)->x)

#define FFL_SPAWNTEMP		1

/*
typedef enum {
	F_INT, 
	F_BYTE, 
	F_SHORT, 
	F_FLOAT,
	F_LSTRING,			// string on disk, pointer in memory, TAG_LEVEL
	F_GSTRING,			// string on disk, pointer in memory, TAG_GAME
	F_VECTOR,
	F_ANGLEHACK,
	F_EDICT,			// index on disk, pointer in memory
	F_ITEM,				// index on disk, pointer in memory
	F_CLIENT,			// index on disk, pointer in memory

	F_FUNC,
	F_PRECORD,
	F_EPAIR,
	F_DATA,

	F_USERHOOK,

	F_IGNORE
} fieldtype_t;

typedef struct
{
	char	*name;
	int		ofs;
	fieldtype_t	type;
	int		flags;
} field_t;
*/
field_t fields[] = {
	{"client",					FOFS(client),					F_CLIENT},
	{"svflags",					FOFS(svflags),					F_INT},
	{"absmin",					FOFS(absmin),					F_VECTOR},
	{"absmax",					FOFS(absmax),					F_VECTOR},
	{"size",					FOFS(size),						F_VECTOR},
	{"solid",					FOFS(solid),					F_INT},
	{"clipmask",				FOFS(clipmask),					F_INT},
	{"owner",					FOFS(owner),					F_EDICT},
	{"className",				FOFS(className),				F_LSTRING},
	{"record",					FOFS(record),					F_PRECORD},
	{"freetime",				FOFS(freetime),					F_FLOAT},
#pragma message("// SCG[11/20/99]: fix epair save/loading" )
	{"epair",					FOFS(epair),					F_RESTORE_AS_NULL},
	{"enemy",					FOFS(enemy),					F_EDICT},
	{"goaletity",				FOFS(goalentity),				F_EDICT},
	{"groundEntity",			FOFS(groundEntity),				F_EDICT},
	{"groundEntity_linkcount",	FOFS(groundEntity_linkcount),	F_INT},
	{"view_entity",				FOFS(view_entity),				F_EDICT},
	{"input_entity",			FOFS(input_entity),				F_EDICT},
	{"inventory",				FOFS(inventory),				F_RESTORE_AS_NULL},
	{"spawnflags",				FOFS(spawnflags),				F_INT},
	{"delay",					FOFS(delay),					F_FLOAT},
	{"killtarget",				FOFS(killtarget),				F_LSTRING},
	{"modelName",				FOFS(modelName),				F_LSTRING},
	{"movetype",				FOFS(movetype),					F_INT},
	{"fragtype",				FOFS(fragtype),					F_INT},
	{"velocity",				FOFS(velocity),					F_VECTOR},
	{"avelocity",				FOFS(avelocity),				F_VECTOR},
	{"prethink",				FOFS(prethink),					F_FUNC},
	{"postthink",				FOFS(postthink),				F_FUNC},
	{"touch",					FOFS(touch),					F_FUNC},
	{"think",					FOFS(think),					F_FUNC},
	{"blocked",					FOFS(blocked),					F_FUNC},
	{"use",						FOFS(use),						F_FUNC},
	{"pain",					FOFS(pain),						F_FUNC},
	{"die",						FOFS(die),						F_FUNC},
	{"remove",					FOFS(remove),					F_FUNC},
	{"save",					FOFS(save),						F_FUNC},
	{"load",					FOFS(load),						F_FUNC},
	{"nextthink",				FOFS(nextthink),				F_FLOAT},
	{"armor_val",				FOFS(armor_val),				F_FLOAT},
	{"armor_abs",				FOFS(armor_abs),				F_FLOAT},
	{"health",					FOFS(health),					F_FLOAT},
	{"hacks",					FOFS(hacks),					F_FLOAT},
	{"hacks_int",				FOFS(hacks_int),				F_INT},
	{"takedamage",				FOFS(takedamage),				F_FLOAT},
	{"deadflag",				FOFS(deadflag),					F_INT},
	{"view_ofs",				FOFS(view_ofs),					F_VECTOR},
	{"button0",					FOFS(button0),					F_FLOAT},
	{"button1",					FOFS(button1),					F_FLOAT},
	{"button2",					FOFS(button2),					F_FLOAT},
	{"fixangle",				FOFS(fixangle),					F_FLOAT},
	{"idealpitch",				FOFS(idealpitch),				F_FLOAT},
	{"netname",					FOFS(netname),					F_LSTRING},
	{"flags",					FOFS(flags),					F_INT},
	{"team",					FOFS(team),						F_INT},
	{"teleport_time",			FOFS(teleport_time),			F_FLOAT},
	{"waterlevel",				FOFS(waterlevel),				F_INT},
	{"watertype",				FOFS(watertype),				F_INT},
	{"soundambientIndex",		FOFS(soundAmbientIndex),		F_INT},
	{"deathtarget",				FOFS(deathtarget),				F_LSTRING},
	{"target",					FOFS(target),					F_LSTRING},
	{"targetname",				FOFS(targetname),				F_LSTRING},
	{"parentname",				FOFS(parentname),				F_LSTRING},
	{"nodeTargetName",			FOFS(nodeTargetName),			F_LSTRING},
	{"groupname",				FOFS(groupname),				F_LSTRING},
	{"triggerIndex",			FOFS(triggerIndex),				F_INT},
	{"spawnname",				FOFS(spawnname),				F_LSTRING},
	{"keyname",					FOFS(keyname),					F_LSTRING},
	{"scriptname",				FOFS(scriptname),				F_LSTRING},
	{"movedir",					FOFS(movedir),					F_VECTOR},
	{"message",					FOFS(message),					F_LSTRING},
	{"gravity",					FOFS(gravity),					F_FLOAT},
	{"userHook",				FOFS(userHook),					F_RESTORE_AS_NULL},
	{"curWeapon",				FOFS(curWeapon),				F_RESTORE_AS_NULL},
	{"curItem",					FOFS(curItem),					F_RESTORE_AS_NULL},
	{"hchild",					FOFS(hchild),					F_RESTORE_AS_NULL},
	{"max_speed",				FOFS(max_speed),				F_FLOAT},
	{"gravity_dir",				FOFS(gravity_dir),				F_VECTOR},
	{"angle_delta",				FOFS(angle_delta),				F_VECTOR},
	{"ideal_ang",				FOFS(ideal_ang),				F_VECTOR},
	{"ang_speed",				FOFS(ang_speed),				F_VECTOR},
	{"ptr1",					FOFS(ptr1),						F_DATA},
	{"viewheight",				FOFS(viewheight),				F_INT},
	{"light_level",				FOFS(light_level),				F_INT},
	{"teamchain",				FOFS(teamchain),				F_EDICT},
	{"teammaster",				FOFS(teammaster),				F_EDICT},
	{"childOffset",				FOFS(childOffset),				F_VECTOR},
	{"transformedOffset",		FOFS(transformedOffset),		F_VECTOR},
	{"spawn_origin",			FOFS(spawn_origin),				F_VECTOR},
	{"elasticity",				FOFS(elasticity),				F_FLOAT},
	{"dissipate",				FOFS(dissipate),				F_FLOAT},
	{"mass",					FOFS(mass),						F_FLOAT},
	{"volume",					FOFS(volume),					F_FLOAT},
	{"velocity_cap",			FOFS(velocity_cap),				F_FLOAT},
	{"percent_submerged",		FOFS(percent_submerged),		F_FLOAT},
	{"submerged_func",			FOFS(submerged_func),			F_FUNC},
	{"groundSurface",			FOFS(groundSurface),			F_RESTORE_AS_NULL},
	{"pMapAnimationToSequence",	FOFS(pMapAnimationToSequence),	F_RESTORE_AS_NULL},
	{"winfo",					FOFS(winfo),					F_RESTORE_AS_NULL},
	{"oldWinfo",				FOFS(oldWinfo),					F_RESTORE_AS_NULL},
	{"lastAIFrame",				FOFS(lastAIFrame),				F_INT},
	{"nIndex",					FOFS(nIndex),					F_INT},
	{"PickedUpItem",			FOFS(PickedUpItem),				F_FUNC},
	{"EntityType",				FOFS(EntityType),				F_INT},
	{"nSidekickFlag",			FOFS(nSidekickFlag),			F_INT},
	{"szUniqueID",				FOFS(szUniqueID),				F_RESTORE_AS_NULL},
	{NULL, 0, F_INT}
};

// -------- just for savegames ----------
// all pointer fields should be listed here, or savegames
// won't work properly (they will crash and burn).
// this wasn't just tacked on to the fields array, because
// these don't need names, we wouldn't want map fields using
// some of these, and if one were accidentally present twice
// it would double swizzle (fuck) the pointer.

/*
field_t		savefields[] =
{
	{"className",				FOFS(className),				F_LSTRING},
	{"enemy",					FOFS(enemy),					F_LSTRING},
	{"goalentity",				FOFS(goalentity),				F_EDICT},
	{"groundEntity",			FOFS(groundEntity),				F_EDICT},
	{"view_entity",				FOFS(view_entity),				F_EDICT},
	{"input_entity",			FOFS(input_entity),				F_EDICT},
	{"killtarget",				FOFS(killtarget),				F_LSTRING},
	{"modelName",				FOFS(modelName),				F_LSTRING},
	{"prethink",				FOFS(prethink),					F_FUNC},
	{"postthink",				FOFS(postthink),				F_FUNC},
	{"touch",					FOFS(touch),					F_FUNC},
	{"think",					FOFS(think),					F_FUNC},
	{"blocked",					FOFS(blocked),					F_FUNC},
	{"use",						FOFS(use),						F_FUNC},
	{"pain",					FOFS(pain),						F_FUNC},
	{"die",						FOFS(die),						F_FUNC},
	{"remove",					FOFS(remove),					F_FUNC},
	{"save",					FOFS(save),						F_FUNC},
	{"load",					FOFS(load),						F_FUNC},
	{"netname",					FOFS(netname),					F_LSTRING},
	{"target",					FOFS(target),					F_LSTRING},
	{"targetname",				FOFS(targetname),				F_LSTRING},
	{"parentname",				FOFS(parentname),				F_LSTRING},
	{"nodeTargetName",			FOFS(nodeTargetName),			F_LSTRING},
	{"groupname",				FOFS(groupname),				F_LSTRING},
	{"spawnname",				FOFS(spawnname),				F_LSTRING},
	{"keyname",					FOFS(keyname),					F_LSTRING},
	{"scriptname",				FOFS(scriptname),				F_LSTRING},
	{"userHook",				FOFS(userHook),					F_RESTORE_AS_NULL},
	{"teamchain",				FOFS(teamchain),				F_EDICT},
	{"teammaster",				FOFS(teammaster),				F_EDICT},
	{"submerged_func",			FOFS(submerged_func),			F_FUNC},
	{"pMapAnimationToSequence",	FOFS(pMapAnimationToSequence),	F_GSTRING},
	{NULL, 0, F_INT}
};
*/

field_t		levelfields[] =
{
//	{"", LLOFS(changemap), F_LSTRING},

	{"", LLOFS(sight_client), F_EDICT},
	{"", LLOFS(sight_entity), F_EDICT},
	{"", LLOFS(sound_entity), F_EDICT},
	{"", LLOFS(sound2_entity), F_EDICT},

	{NULL, 0, F_INT}
};

field_t		clientfields[] =
{
//	{"", CLOFS(pers.weapon), F_ITEM},
//	{"", CLOFS(newweapon), F_ITEM},
	{NULL, 0, F_INT}
};

//=========================================================
userEntity_t *com_FindEntity( const char *name );
void P_WriteHeader( char *filename, char *comment, qboolean autoSave )
{
	FILE			*f;
	int				i,j;
	save_header_t	header;
	memset(&header,0,sizeof(header));

	f = fopen (filename, "wb");
	if (!f)
		gi.Error ("Couldn't open %s", filename);

	if (!autoSave)
	{
		if (!deathmatch->value)
		{
			edict_t	*pEntArray[3];
			pEntArray[0] = &g_edicts[1];		// hiro
		
			if( !coop->value )
			{
				pEntArray[1] = com_FindEntity( "Mikiko" );
				pEntArray[2] = com_FindEntity( "Superfly" );
				header.bCoop = false;
			}
			else
			{
				pEntArray[1] = &g_edicts[2];		// mikiko
				pEntArray[2] = &g_edicts[3];		// superfly
				header.bCoop = true;
			}

			if (pEntArray[0]->client)
			{
				client_persistant_s pers = pEntArray[0]->client->pers;

				// map and save specific stuff
				for (i = 0; i < MAX_SUBMAPS; i++)
				{
					if (i == serverState.subMap)
					{
						header.monsters		+= serverState.numMonstersKilled;
						header.tMonsters	+= serverState.numMonsters;
						header.secrets		+= serverState.numSecretsFound;
						header.tSecrets		+= serverState.numSecrets;
						header.total		+= serverState.level->time;
					}
					else
					{
						header.monsters		+= pers.nTotalKills[i];
						header.tMonsters	+= pers.nTotalMonsters[i];
						header.secrets		+= pers.nFoundSecrets[i];
						header.tSecrets		+= pers.nTotalSecrets[i];
						header.total		+= pers.nTime[i];
					}
				}
			}

			header.time = serverState.level->time;
			header.episode = serverState.episode;
			if (comment)
				Com_sprintf(header.caption,sizeof(header.caption),comment);

			Com_sprintf(header.mapTitle,sizeof(header.mapTitle),serverState.mapTitle);

			// do the single/coop specific stuff...
			edict_t *ent = NULL;
			for (j = 0; j < 3; j++)
			{
				if (!(ent = pEntArray[j]))
					continue;

				switch(j)
				{
				case 0:
					header.hiroHealth	= floor(ent->health + 0.5);
					header.hiroArmor	= floor(ent->armor_val + 0.5);
					header.hiroLevel	= ent->record.level;
					break;
				case 1:
					header.mHealth		= floor(ent->health + 0.5);
					header.mArmor		= floor(ent->armor_val + 0.5);
					if (coop->value)
						header.mLevel		= ent->record.level;
					break;
				case 2:
					header.sfHealth		= floor(ent->health + 0.5);
					header.sfArmor		= floor(ent->armor_val + 0.5);
					if (coop->value)
						header.sfLevel		= ent->record.level;
					break;
				};
			}
		}
	}
	else
	{
		header.bCoop = (coop->value > 0);
		header.episode = serverState.episode;
		Com_sprintf(header.caption,sizeof(header.caption),serverState.mapTitle);
		Com_sprintf(header.mapTitle,sizeof(header.mapTitle),"Last map change");
	}

	fwrite(&header,sizeof(header),1,f);
	fclose (f);

}

//=========================================================

void WriteField1 (FILE *f, field_t *field, byte *base)
{
	void		*p;
	int			len;
	int			index;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_RESTORE_AS_NULL:
	case F_BYTE:
	case F_SHORT: 
	case F_INT:
	case F_FLOAT:
	case F_ANGLEHACK:
	case F_VECTOR:
	case F_IGNORE:
		break;

	case F_LSTRING:
	case F_GSTRING:
		if ( *(char **)p )
			len = strlen(*(char **)p) + 1;
		else
			len = 0;
		*(int *)p = len;
		break;
	case F_EDICT:
		if ( *(edict_t **)p == NULL)
			index = -1;
		else
			index = *(edict_t **)p - g_edicts;
		*(int *)p = index;
		break;
	case F_CLIENT:
		if ( *(gclient_t **)p == NULL)
			index = -1;
		else
			index = *(gclient_t **)p - game.clients;
		*(int *)p = index;
		break;
	case F_ITEM:
/*
		if ( *(edict_t **)p == NULL)
			index = -1;
		else
			index = *(gitem_t **)p - itemlist;
		*(int *)p = index;
*/
		break;
	case F_FUNC:
		break;
	case F_PRECORD:
		break;
	case F_EPAIR:
		break;
	case F_DATA:
		break;
	default:
		gi.Error ("WriteEdict: unknown field type");
	}
}

void WriteField2( FILE *f, field_t *field, byte *base )
{
	int			len, index;
	void		*p;
	
	p = ( void * ) ( base + field->ofs );
	switch( field->type )
	{
	case F_LSTRING:
	case F_GSTRING:
		if ( *( char ** ) p )
		{
			len = strlen( *( char ** ) p ) + 1;
			fwrite( *( char ** ) p, len, 1, f);
		}
		break;
	case F_FUNC:
		if( *( char ** ) p )
		{
			index = P_GetIndexForFunc( *( char ** ) p );
/*
			if( index != -1 )
			{
				gi.Con_Dprintf("Saved index for function: %s\n", field->name );
			}
			else
			{
				gi.Con_Dprintf("Entity is has been assigned an unregistered function ( %s )\n", field->name );
			}
*/
			if( index == -1 )
			{
				gi.Con_Dprintf("Entity is has been assigned an unregistered function ( %s )\n", field->name );
			}
		}
		else
		{
			index = -1;
		}

		fwrite (&index, sizeof( int ), 1, f);
		break;
	}
}

void WriteEpair( FILE *f, edict_t *ent )
{
	int i;
	int nKeyLen;
	int nValueLen;

	if( ent->epair == NULL )
	{
		i = 0; 
		fwrite( &i, sizeof( int ), 1, f );
		return;
	}

	for( i = 0; ent->epair[i].key != NULL; i++ )
	{
	}

	fwrite( &i, sizeof( int ), 1, f );

	for( i = 0; ent->epair[i].key != NULL; i++ )
	{
		nKeyLen = strlen( ent->epair[i].key ) + 1;
		fwrite( &nKeyLen, sizeof( int ), 1, f );
		fwrite( ent->epair[i].key, nKeyLen, 1, f );

		nValueLen = strlen( ent->epair[i].value ) + 1;
		fwrite( &nValueLen, sizeof( int ), 1, f );
		fwrite( ent->epair[i].value, nValueLen, 1, f );
	}
}

void ReadEpair( FILE *f, edict_t *ent )
{
	int	i;
	int	nNumEpairs;
	int nKeyLen;
	int nValueLen;

	fread( &nNumEpairs, sizeof( nNumEpairs ), 1, f );
	if( nNumEpairs == 0 )
	{
		return;
	}

	ent->epair = ( userEpair_t * ) gi.X_Malloc( ( nNumEpairs + 1 ) * sizeof( userEpair_t ), MEM_TAG_COM_EPAIR );

	for( i = 0; i < nNumEpairs; i++ )
	{
		fread( &nKeyLen, sizeof( int ), 1, f );
		ent->epair[i].key = ( char * ) gi.X_Malloc( nKeyLen, MEM_TAG_COM_EPAIR );
		fread( ent->epair[i].key, nKeyLen, 1, f );

		fread( &nValueLen, sizeof( int ), 1, f );
		ent->epair[i].value = ( char * ) gi.X_Malloc( nValueLen, MEM_TAG_COM_EPAIR );
		fread( ent->epair[i].value, nValueLen, 1, f );
	}

	ent->epair[i].key = NULL;
	ent->epair[i].value = NULL;

}

void ReadField( FILE *f, field_t *field, byte *base )
{
	void		*p;
	int			len;
	int			index;

	p = ( void * ) ( base + field->ofs );
	switch( field->type )
	{
	case F_INT:
	case F_BYTE:
	case F_SHORT: 
	case F_FLOAT:
	case F_ANGLEHACK:
	case F_VECTOR:
	case F_IGNORE:
		break;

	case F_RESTORE_AS_NULL:
		*( char ** ) p = NULL;
		break;

	case F_LSTRING:
		len = *( int * ) p;
		if (!len)
			*( char ** ) p = NULL;
		else
		{
			*( char ** ) p = ( char * ) gi.X_Malloc (len, MEM_TAG_LEVEL);
			fread( *( char ** ) p, len, 1, f );
		}
		break;
	case F_GSTRING:
		len = *( int * ) p;
		if( !len )
			*( char ** ) p = NULL;
		else
		{
			*( char ** ) p = ( char * ) gi.X_Malloc( len, MEM_TAG_GAME );
			fread( *( char ** ) p, len, 1, f );
		}
		break;
	case F_EDICT:
		index = *( int * ) p;
		if( index == -1 )
			*( edict_t ** ) p = NULL;
		else
			*( edict_t ** ) p = &g_edicts[index];
		break;
	case F_CLIENT:
		index = *( int * ) p;
		if ( index == -1 )
			*( gclient_t ** ) p = NULL;
		else
			*( gclient_t ** ) p = &game.clients[index];
		break;
	case F_ITEM:
/*
		index = *(int *)p;
		if ( index == -1 )
			*(gitem_t **)p = NULL;
		else
			*(gitem_t **)p = &itemlist[index];
*/
		break;
	case F_FUNC:
		fread( &index, sizeof( len ), 1, f );
		if( index != -1 )
		{
			*( byte ** ) p = ( byte * ) P_GetFuncForIndex( index );
		}
		else
		{
//			gi.Con_Dprintf("Entity has been assigned an unregistered function ( %s )\n", field->name );
			*( byte ** ) p = ( byte * ) NULL;
		}
		break;
	case F_PRECORD:
		break;
	case F_EPAIR:
		break;
	case F_DATA:
		break;
	default:
		gi.Error ("ReadEdict: unknown field type");
	}
}

//=========================================================

/*
==============
WriteClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteClient (FILE *f, gclient_t *client)
{
	field_t		*field;
	gclient_t	temp;
	
	gi.Con_Dprintf("----------------------------\n");
	gi.Con_Dprintf("Saving: client\n");

	// all of the ints, floats, and vectors stay as they are
	temp = *client;

	// change the pointers to lengths or indexes
	for (field=clientfields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=clientfields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)client);
	}

	int nSize = sizeof( client_persistant_t );
	// SCG[3/15/00]: Save superfly persistant info
	if( client->pers.pPersSuperfly != NULL )
	{
		fwrite( &nSize, sizeof( nSize ), 1, f );
		fwrite( client->pers.pPersSuperfly, nSize, 1, f );
	}
	else
	{
		nSize = 0;
		fwrite( &nSize, sizeof( nSize ), 1, f );
	}

	nSize = sizeof( client_persistant_t );
	// SCG[3/15/00]: Save mikiko persistant info
	if( client->pers.pPersMikiko != NULL )
	{
		fwrite( &nSize, sizeof( nSize ), 1, f );
		fwrite( client->pers.pPersMikiko, nSize, 1, f );
	}
	else
	{
		nSize = 0;
		fwrite( &nSize, sizeof( nSize ), 1, f );
	}
}

/*
==============
ReadClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadClient (FILE *f, gclient_t *client)
{
	field_t		*field;

	fread (client, sizeof(*client), 1, f);

	for (field=clientfields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)client);
	}

	// SCG[3/15/00]: Load superfly persistant info
	int nSize;
	fread( &nSize, sizeof( nSize ), 1, f );
	if( nSize == sizeof( client_persistant_t ) )
	{
		client->pers.pPersSuperfly = ( client_persistant_t * ) gi.X_Malloc( sizeof( client_persistant_t ), MEM_TAG_GAME );
		fread( client->pers.pPersSuperfly, sizeof( client_persistant_t ), 1, f );
	}

	// SCG[3/15/00]: Load mikiko persistant info
	fread( &nSize, sizeof( nSize ), 1, f );
	if( nSize == sizeof( client_persistant_t ) )
	{
		client->pers.pPersMikiko = ( client_persistant_t * ) gi.X_Malloc( sizeof( client_persistant_t ), MEM_TAG_GAME );
		fread( client->pers.pPersMikiko, sizeof( client_persistant_t ), 1, f );
	}
}

#define SPAWNED_SUPERFLY_WITH_CLIENT	0x01
#define SPAWNED_MIKIKO_WITH_CLIENT		0x02
void P_CheckZapInventory(gclient_t *client)
{
	// we need to check for sidekicks here and if they do not exists, zero out the persistant flag
	// also, check changelevel flags for need to spawn sidekicks and change the same flags as appropriate.
	if( serverState.nLevelTransitionType == 1 )
	{
		userEntity_t *pSuperfly = com_FindEntity( "Superfly" );
		userEntity_t *pMikiko = com_FindEntity( "Mikiko" );
		
		unsigned int nSidekickMask = ( game.serverflags & SFL_SIDEKICK_MASK );
		if( ( pSuperfly == NULL ) && !( nSidekickMask & ( SFL_SPAWN_SUPERFLY | SFL_SPAWN_MIKIKOFLY ) ) )
		{
			client->pers.nSidekicksSpawnedWithClient &= ~SPAWNED_SUPERFLY_WITH_CLIENT;
		}

		if( ( pMikiko == NULL ) && !( nSidekickMask & SFL_SPAWN_MIKIKO ) )
		{
			client->pers.nSidekicksSpawnedWithClient &= ~SPAWNED_MIKIKO_WITH_CLIENT;
		}
	}
	
	if (serverState.nLevelTransitionType == 2)
	{
		client->pers.nWeaponFlags = client->pers.nCurrentWeaponFlag = 0;
		// SCG[5/4/00]: This made it to RC14, but did not get checked in SS database untill today.
		// SCG[5/4/00]: SS database label will be inaccurate for this file.
		client->pers.nSidekicksSpawnedWithClient = 0;// SCG[4/3/00]: 

		memset(client->pers.inventoryCrap,0,sizeof(client->pers.inventoryCrap));
		memset(&client->pers.nAmmo[0],0,sizeof(client->pers.nAmmo));// cek[5-4-00]: zap ammo count too.
	}
}

/*
============
P_WriteGame

This will be called whenever the game goes to a new level,
and when the user explicitly saves the game.

Game information include cross level data, like multi level
triggers, help computer info, and all client states.

A single player death will automatically restore from the
last save position.
============
*/
void P_WriteGame (char *filename, char autosave)
{
	FILE	*f;
	int		i;
	char	str[16];

	if (!autosave)
		SaveClientData ();

	f = fopen (filename, "wb");
	if (!f)
		gi.Error ("Couldn't open %s", filename);

	memset (str, 0, sizeof(str));
	strcpy (str, __DATE__);
	fwrite (str, sizeof(str), 1, f);

	fwrite (&game, sizeof(game), 1, f);
	for (i=0 ; i<game.maxclients ; i++)
	{
		if (autosave)
			P_CheckZapInventory(&game.clients[i]);

		WriteClient (f, &game.clients[i]);
	}

	fclose (f);
}

///////////////////////////////////////////////////////////////////////////////
//	P_ReadGame
//
///////////////////////////////////////////////////////////////////////////////

void P_ReadGame (char *filename)
{
	FILE	*f;
	int		i;
	char	str[16];

	gi.FreeTags (MEM_TAG_GAME);

	f = fopen (filename, "rb");
	if (!f)
		gi.Error ("Couldn't open %s", filename);

	fread (str, sizeof(str), 1, f);
	if (strcmp (str, __DATE__))
	{
		fclose (f);
		gi.Error ("Savegame from an older version.\n");
	}

	g_edicts =  ( edict_t * ) gi.X_Malloc( game.maxentities * sizeof( g_edicts[0] ), MEM_TAG_GAME );
	memset( g_edicts, 0, game.maxentities * sizeof( g_edicts[0] ) );
	globals.edicts = g_edicts;

	fread (&game, sizeof(game), 1, f);
	game.clients = ( gclient_t * ) gi.X_Malloc( game.maxclients * sizeof( game.clients[0] ), MEM_TAG_GAME );
	memset( game.clients, 0, game.maxclients * sizeof( game.clients[0] ) );

	game.transients = ( transient_t * ) gi.X_Malloc ( 8 * sizeof( transient_t ), MEM_TAG_GAME );
	memset( game.transients, 0, 8 * sizeof( transient_t ) );

	for (i=0 ; i<game.maxclients ; i++)
	{
		ReadClient (f, &game.clients[i]);
	}

	fclose (f);
}

//==========================================================


/*
==============
WriteEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
//void P_InventorySave( FILE *f, userEntity_t *self );
//void P_InventoryLoad( FILE *f, userEntity_t *self );
void P_InventorySave( void **buf, userEntity_t *self, int aryEntries );
void P_InventoryLoad( void **buf, userEntity_t *self, int aryEntries );

void WriteEdict (FILE *f, edict_t *ent)
{
	field_t		*field;
	edict_t		temp;

	// all of the ints, floats, and vectors stay as they are
	temp = *ent;

	// change the pointers to lengths or indexes
	for (field=fields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=fields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)ent);
	}

	WriteEpair( f, ent );

	// SCG[11/19/99]: Write the edict hooks
	if( ent->save )
	{
		ent->save( f, ent );
	}

	// cek[2-22-00]
	void *buf = (void *)f;
	P_InventorySave(&buf,ent,0);
}

/*
==============
WriteLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteLevelLocals (FILE *f)
{
	field_t		*field;
	level_locals_t		temp;

	// all of the ints, floats, and vectors stay as they are
	temp = level;

	// change the pointers to lengths or indexes
	for (field=levelfields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=levelfields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)&level);
	}
}


/*
==============
ReadEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadEdict (FILE *f, edict_t *ent)
{
	field_t			*field;
	unsigned long	lBytesRead;

	// SCG[11/21/99]: Free up pre-read entity data that ends up having to be reset later.
	if( ent->inventory != NULL )
	{
		P_InventoryFree( ent->inventory );
		ent->inventory = NULL;
	}

	// SCG[11/22/99]: Read the data
	lBytesRead = fread (ent, sizeof(*ent), 1, f);

	// SCG[11/21/99]: Read in the entity info
	for( field = fields; field->name; field++ )
	{
		ReadField( f, field, ( byte * ) ent );
	}

	memset (&ent->area, 0, sizeof(ent->area));
	gi.linkentity(ent);

	ReadEpair( f, ent );

	// SCG[11/19/99]: Read the edict hooks
	if( ent->load )
	{
		ent->load( f, ent );
	}

	// cek[2-22-00]
	void *buf = (void *)f;
	P_InventoryLoad(&buf,ent,0);
}

/*
==============
ReadLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadLevelLocals (FILE *f)
{
	field_t		*field;

	fread (&level, sizeof(level), 1, f);

	for (field=levelfields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)&level);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	P_WriteLevel
//
///////////////////////////////////////////////////////////////////////////////
void P_InitDLLs (void);

void P_WriteLevel (char *filename)
{
	int		i;
	edict_t	*ent;
	FILE	*f;
//	void	*base;

	f = fopen (filename, "wb");
	if (!f)
		gi.Error ("Couldn't open %s", filename);

	// write out edict size for checking
	i = sizeof(edict_t);
	fwrite (&i, sizeof(i), 1, f);

	// write out a function pointer for checking
/*
	base = (void *)InitGame;
	
	fwrite (&base, sizeof(base), 1, f);
*/
	// write out level_locals_t
	WriteLevelLocals (f);

	// write out all the configstrings
//	fwrite (sv.configstrings, sizeof(sv.configstrings), 1, f);

	// write out all the entities
	for (i=0 ; i<globals.num_edicts ; i++)
	{
		ent = &g_edicts[i];
		if( !ent->inuse )
		{
			continue;
		}
		if( ent->flags & FL_NOSAVE )
		{
			continue;
		}
		if( ( ent->deadflag != DEAD_NO ) && ( ent->flags & FL_MONSTER ) )
		{
			P_FreeEdict( ent );
			continue;
		}
		if( ent->className == NULL )
		{
			continue;
		}

		fwrite (&i, sizeof(i), 1, f);
		WriteEdict (f, ent);
	}
	i = -1;
	fwrite (&i, sizeof(i), 1, f);

	fclose (f);
}


///////////////////////////////////////////////////////////////////////////////
//	ReadLevel
//
//	SpawnEntities will allready have been called on the
//	level the same way it was when the level was saved.
//
//	That is necessary to get the baselines
//	set up identically.
//
//	The server will have cleared all of the world links before
//	calling ReadLevel.
///////////////////////////////////////////////////////////////////////////////

void P_ReadLevel (char *filename)
{
	int		entnum, lastentnum;
	FILE	*f;
	int		i;
//	void	*base;
	edict_t	*ent;

	f = fopen (filename, "rb");
	if (!f)
		gi.Error ("Couldn't open %s", filename);

	// free any dynamic memory allocated by loading the level
	// base state
	gi.Mem_Free_Tag(MEM_TAG_LEVEL);
	gi.Mem_Free_Tag(MEM_TAG_HOOK);
//	gi.Mem_Free_Tag(MEM_TAG_COM_EPAIR);
	gi.Mem_Free_Tag(MEM_TAG_COM_ENTITY);
	gi.Mem_Free_Tag(MEM_TAG_COM_MISC);
//	gi.Mem_Heap_Walk( 1 );

	// wipe all the entities
	memset( g_edicts, 0, game.maxentities * sizeof( g_edicts[0] ) );
	globals.num_edicts = maxclients->value+1;

	// check edict size
	fread (&i, sizeof(i), 1, f);
	if (i != sizeof(edict_t))
	{
		fclose (f);
		gi.Error ("ReadLevel: mismatched edict size");
	}

	// check function pointer base address
/*
	fread (&base, sizeof(base), 1, f);
	if (base != (void *)InitGame)
	{
		fclose (f);
		gi.Error ("ReadLevel: function pointers have moved");
	}
*/
	// load the level locals
	ReadLevelLocals (f);

	// load all the entities
	while (1)
	{
		if (fread (&entnum, sizeof(entnum), 1, f) != 1)
		{
			fclose (f);
			gi.Error ("ReadLevel: failed to read entnum");
		}
		if( entnum >= globals.max_edicts )
		{
			gi.Error ("ReadLevel: entnum too large!\n");
		}

		if (entnum == -1)
			break;
		if (entnum >= globals.num_edicts)
			globals.num_edicts = entnum+1;

		ent = &g_edicts[entnum];
		ReadEdict (f, ent);

		// let the server rebuild world links for this ent
//		if( !( ent->flags & FL_MONSTER ) )
		{
//			memset (&ent->area, 0, sizeof(ent->area));
//			gi.linkentity(ent);
		}
		lastentnum = entnum;
	}

	fclose (f);

	// wipe all the clients
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = &g_edicts[i+1];
		gi.unlinkentity (ent);
		// SCG[11/20/99]: Uh, this makes save games not work right...
//		ent->inuse = false;
		ent->client = game.clients + i;
		ent->client->pers.bConnected = false;
	}

//	// fire any cross-level triggers here
/*
	for (i=0 ; i<globals.num_edicts ; i++)
	{
		if (!g_edicts[i].className)
			continue;
		if (strcmp(g_edicts[i].className, "target_crosslevel_target") != 0)
			continue;
		g_edicts[i].nextthink = level.time + g_edicts[i].delay;
	}
*/
	for( i = 0; i < globals.num_edicts; i++ )
	{
		ent = &g_edicts[i];

		if (!ent->inuse)
			continue;

		// fire any cross-level triggers
		if (ent->className)
			if (strcmp(ent->className, "target_crosslevel_target") == 0)
				ent->nextthink = level.time + ent->delay;
	}
}

void SaveHook( FILE *f, void *pHook, int nSize )
{
	fwrite( pHook, nSize, 1, f );
}

void LoadHook( FILE *f, void *pHook, int nSize )
{
	fread( pHook, nSize, 1, f );
}

