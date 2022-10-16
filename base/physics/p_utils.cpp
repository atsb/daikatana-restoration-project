///////////////////////////////////////////////////////////////////////////////
//	p_utils.c
//
//	misc utility functions for physics.dll
///////////////////////////////////////////////////////////////////////////////

#include "p_global.h"
#include "p_user.h"
//#include "hooks.h"
#include "memmgr.h"

//static	short	p_entityID = 0;	//	just increments on each entity spawn

///////////////////////////////////////////////////////////////////////////////
//	P_InitEdict
//
///////////////////////////////////////////////////////////////////////////////

void P_InitEdict (edict_t *e)
{
	e->inuse = true;
	e->className = "noclass";
	e->gravity = 1.0;
	e->mass = 1.0;
	e->volume = 2.0;
	e->elasticity = 0.75;
	e->dissipate = 0.9;
	e->s.alpha = 1.0;
	e->velocity_cap = 32.0;
	e->submerged_func = P_PercentSubmerged;
	e->s.render_scale.Set( 1.0f, 1.0f, 1.0f );
	e->s.number = e - g_edicts;
	e->svflags = SVF_NEWSPAWN;

/*	*** NUKE THIS SOON ***
	e->s.entityID = p_entityID++;
*/
	//	force frame syncing for new entities so re-used entities get their frame
	//	index sent for sure!
	e->s.frameInfo.frameFlags |= FRAME_FORCEINDEX;

	//	ensure that userHook always starts out null
	e->userHook = NULL;
	//	set default removal function for an entity
	e->remove = P_FreeEdict;

	// make sure the unique ID is null
	e->szUniqueID = NULL;
}

///////////////////////////////////////////////////////////////////////////////
//	P_SpawnEdict
//
//	Either finds a free edict, or allocates a new one.
//	Try to avoid reusing an entity that was recently freed, because it
//	can cause the client to think the entity morphed into something else
//	instead of being removed and recreated, which can cause interpolated
//	angles and bad trails.
///////////////////////////////////////////////////////////////////////////////

edict_t *P_SpawnEdict (void)
{
	int			i;
	edict_t		*e;

	e = &g_edicts[(int)maxclients->value+1];
	for ( i=maxclients->value+1 ; i<globals.num_edicts ; i++, e++)
	{
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if (!e->inuse && ( e->freetime < 2 || level.time - e->freetime > 0.5 ) )
		{
			P_InitEdict (e);
			return e;
		}
	}
	
	if (i == game.maxentities)
		gi.Error ("ED_Alloc: no free edicts");
		
	globals.num_edicts++;
	P_InitEdict (e);
	return e;
}

///////////////////////////////////////////////////////////////////////////////
//	P_EntPtr
//
//	returns a pointer to entity # entnum
///////////////////////////////////////////////////////////////////////////////

edict_t	*P_EntPtr (int entnum)
{
	edict_t *e;

	e = &g_edicts[entnum];

	return(e);
}

///////////////////////////////////////////////////////////////////////////////
//	P_EntNum
//
//	returns the corresponding number of entity ent
///////////////////////////////////////////////////////////////////////////////

int	P_EntNum (edict_t *ent)
{
	edict_t	*cur_ent;
	int		i;

	cur_ent = g_edicts;

	for (i = 0; i < globals.num_edicts; i++, cur_ent++)
	{
		if (cur_ent == ent)
			return	i;
	}

	return	globals.num_edicts + 1;
}

///////////////////////////////////////////////////////////////////////////////
//	P_FreeEdict
//
//	Marks the edict as free, saves the gclient_t pointer, initializes the 
//	edict to 0, restores the client pointer and sets a few other things
//
//	if the edict is a hierarchical child, it is removed from all parent lists
//	that it is in
//
//	and edict's self->remove function points to this by default
///////////////////////////////////////////////////////////////////////////////

void P_InventoryFree( invenList_t *list );

void P_FreeEdict (edict_t *ent)
{
	gclient_t	*client;

	if( ent == NULL )
	{
		return;
	}

	if( stricmp( ent->className, "path_corner_train" ) == 0 )
	{
		gi.Con_Dprintf( "Error: removing entity: %s\n", ent->className );
	}

	// clear the unique ID for this entity
	if (serverState.UNIQUEID_Remove)
		serverState.UNIQUEID_Remove(ent);

	//	Nelno:	free userHook memory
	if (ent->userHook != NULL)
	{
		gi.X_Free(ent->userHook);
//		com_free (ent->userHook);
		ent->userHook = NULL;
	}

	if( ent->inventory != NULL )
	{
		P_InventoryFree( ent->inventory );
		ent->inventory = NULL;
	}

	//	if this is a fake entity, remove it
	if (ent->flags & FL_FAKEENTITY)
	{
		P_RemoveFakeEntity (ent);
		return;
	}

	//&&& AMW 6.11.98 - rebuild the teamchain list if this entity is part of the list
	if (ent->teamchain || ent->teammaster)
	{
		// is this a child?
		if (ent->flags & FL_TEAMSLAVE)
		{
			userEntity_t	*cur = NULL, *last = NULL;

			// run through the list and find 'ent' - start with the parent/teammaster
			cur = ent->teammaster;

			while (cur)
			{
				if (cur == ent)				// we found 'ent' in the teamchain
				{
					break;
				}
				last = cur;					// save this one
				cur = cur->teamchain;		// go to the next entity in the list
			}

			// re-connect the entities before and after 'ent'
			if (last)
			{
				if (cur)
				{
					last->teamchain = cur->teamchain;	// re-connect the gap
				}
				else
				{
					last->teamchain = NULL;	// this was the last entity in the list
				}
			}
		}
		else  // what do we do here if the parent is freed?  at least reset the other entities
		{
			// if this is true, some kind of error has occured.. a child is not flagged as a TEAMSLAVE
			if (ent != ent->teammaster)
			{
//				gstate->Con_Printf ("error deleting parent with NO attached children [%s]\n",(ent->targetname) ? ent->targetname:"N/A");
			}
			else
			{
//				gstate->Con_Printf ("Deleting parent with attached children [%s]\n",(ent->targetname) ? ent->targetname:"N/A");

				userEntity_t	*cur = NULL, *next = NULL;

				// run through the list, start with the parent/teammaster and reset all the children
				cur = ent->teammaster->teamchain;

				while (cur)
				{
					next = cur->teamchain;			// save off the next pointer
		
					cur->teamchain = NULL;			// remove the link from this entity
					cur->teammaster = NULL;         // remove the master link from this entity
					cur->flags &= ~FL_TEAMSLAVE;	// turn off the slave flag

					cur = next;						// go to the next entity in the list
				}
			}
		}
	}

	gi.unlinkentity (ent);		// unlink from world

	//	Nelno:	save the pointer to gclient_t so edicts ALWAYS have one
	client = ent->client;

	memset (ent, 0, sizeof(edict_t));

	ent->client = client;

	ent->className = "freed";
	ent->freetime = level.time;
	ent->inuse = false;
}

///////////////////////////////////////////////////////////////////////////////
//	P_FirstEntity
//
//	returns the first entity in the entity list
///////////////////////////////////////////////////////////////////////////////

userEntity_t *P_FirstEntity(void)
{
	return	(g_edicts);
}

///////////////////////////////////////////////////////////////////////////////
//	P_NextEntity
//
//	returns the entity after ent in the entity list
//
//	WARNING:	skips entities that are not in use -- you can't find empty
//				client slots with this function
///////////////////////////////////////////////////////////////////////////////

userEntity_t *P_NextEntity (edict_t *ent)
{
	edict_t		*cur_ent;
	int			i;

	for (i = P_EntNum (ent) + 1; i < globals.num_edicts; i++, cur_ent++)
	{
		cur_ent = &g_edicts [i];
		if (!cur_ent->inuse)
			continue;

		return	(userEntity_t *) cur_ent;
	}

	return	NULL;
}

///////////////////////////////////////////////////////////////////////////////
//	P_TouchTriggers
//
//	Nelno:	forms a list of all the triggers with any part of their volume 
//			inside the area specified by ent->mins/ent->maxs.
//			For each entity in the list, calls its touch functions
///////////////////////////////////////////////////////////////////////////////

void	com_UseTargets (userEntity_t *self, userEntity_t *other, userEntity_t *activator);
void	P_TouchTriggers (edict_t *ent)
{
	int			i, num;
	edict_t		*touch [MAX_EDICTS], *hit;

	// dead things don't activate triggers!
	if ((ent->client || (ent->svflags & SVF_MONSTER)) && (ent->health <= 0))
		return;


	num = gi.BoxEdicts (ent->absmin, ent->absmax, touch, MAX_EDICTS, AREA_TRIGGERS);
	
	
	//NSS[11/20/99]:We need to figure out how we will prevent script
	//characters from hitting triggers.
	//playerHook_t *hook	= (playerHook_t *)ent->userHook;
	//if(hook)
	//{
	//	if(hook->bInScriptMode)
	//		return;
	//}

	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for (i=0 ; i<num ; i++)
	{
		hit = touch[i];
		if (!hit->inuse)
			continue;
		if (!hit->touch)
			continue;
		if (hit == ent)
			continue;
		if( strcmp( ent->className, "noclass" ) == 0 )
		{
			continue;
		}
		if( strcmp( ent->className, "freed" ) == 0 )
		{
			continue;
		}

		hit->touch (hit, ent, NULL, NULL);

		//NSS[11/1/99]:Only items should call this section and monsters should not trigger them
		if( (hit->flags & FL_ITEM) && (ent->flags & (FL_BOT|FL_CLIENT)) )
		{
			// SCG[10/4/99]: Items can now trigger things...
			if((hit->target != NULL ) || ( hit->killtarget != NULL ) )
			{
				com_UseTargets( hit, ent, ent );
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
//	P_TouchSolids
//
//	Call after linking a new trigger in during gameplay
//	to force all entities it covers to immediately touch it
///////////////////////////////////////////////////////////////////////////////

void	P_TouchSolids (edict_t *ent)
{
	int			i, num;
	edict_t		*touch[MAX_EDICTS], *hit;

	num = gi.BoxEdicts (ent->absmin, ent->absmax, touch, MAX_EDICTS, AREA_SOLID);

	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for (i=0 ; i<num ; i++)
	{
		hit = touch[i];
		if (!hit->inuse)
			continue;
		if (ent->touch)
			ent->touch (hit, ent, NULL, NULL);
		if (!ent->inuse)
			break;
	}
}


///	Nelno: move to COMMON.DLL

///////////////////////////////////////////////////////////////////////////////
//	KillBox
//
//	Kills all entities that would touch the proposed new positioning
//	of ent.  Ent should be unlinked before calling this!
///////////////////////////////////////////////////////////////////////////////

qboolean KillBox (edict_t *ent)
{
	trace_t		tr;

	while (1)
	{
		tr = gi.TraceBox (ent->s.origin, ent->s.mins, ent->s.maxs, ent->s.origin, NULL, MASK_PLAYERSOLID);
		if (!tr.ent)
			break;

		// nail it
//		T_Damage (tr.ent, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION);

		// if we didn't kill it, fail
		if (tr.ent->solid)
			return false;
	}

	return true;		// all clear
}

///////////////////////////////////////////////////////////////////////////////
//	FAKE ENTITIES
//
//	for spawning "fake" entities.
//
//	these are just malloc'd structures that appear to be regular entities
//	except that they have an FL_FAKENTITY flag set and they are not part of
//	the server/physics entity list.
//
//	They are useful in place of entities that do not need physics run on
//	them -- they will never see the physics loop at all.
//
//	fake entities are always malloc'd as TAG_LEVEL, so they cannot persist
//	between maps/levels.
//
//	fake entities are not chained or linked in any way, either to the real
//	entity list or any other fake entities
///////////////////////////////////////////////////////////////////////////////

userEntity_t	*P_SpawnFakeEntity (void)
{
	userEntity_t *fake;
	
	fake = (userEntity_t *) gi.X_Malloc (sizeof (userEntity_t), MEM_TAG_ENTITY);
	fake->client = (gclient_t *) gi.X_Malloc (sizeof (gclient_t), MEM_TAG_ENTITY);	

	P_InitEdict (fake);

	return	fake;
}

///////////////////////////////////////////////////////////////////////////////
//	P_RemoveFakeEntity
//
//	FIXME:	remove userHook memory!!!
///////////////////////////////////////////////////////////////////////////////

void	P_RemoveFakeEntity (userEntity_t *fake)
{
	gi.X_Free (fake->client);
	gi.X_Free (fake);
}

