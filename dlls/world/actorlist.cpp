//////////////////////////////////////////////////////////
//	actor list 
//
//	builds a list of a specific entity type which can be 
//	used for searches without going through the entire entity
//	list (which could be as large as 600 entities)
//
//	alists are destroyed each time a level ends and rebuilt
//	in worldspawn_prethink
//
//	globals:
//		flags set.
//		set.  Used by ai routines for targetting
//	client_list - all entities with FL_CLIENT set, ie. human players
//		who have logged on to the server.  Used for events that only
//		concern human players.
//	monster_list - all entities with FL_MONSTER flag set.
//////////////////////////////////////////////////////////

#include "world.h"
#include "actorlist.h"
#include "ai_utils.h"
#include "ai_frames.h"
//#include "Sidekick.h"// SCG[1/23/00]: not used
#include "ai_info.h"

///////////////////////////////////////////////////////////////////////////////////////////////
// external globals
///////////////////////////////////////////////////////////////////////////////////////////////

alist_t	*client_list = NULL;		//	all entities with FL_CLIENT flag set
alist_t	*monster_list = NULL;		//	all entities with FL_MONSTER flag set


int alist_add( alist_t *alist, userEntity_t *ent );
int alist_remove( alist_t *alist, userEntity_t *ent, bool fatalize = false );

// ----------------------------------------------------------------------------
//
// Name:        alist_init
// Description:
// Input:
// Output:
// Note:
// returns an initialized (0 member) alist
//
// ----------------------------------------------------------------------------
alist_t	*alist_init()
{
//	alist_t	*alist = new alist_t;
	alist_t	*alist = (alist_t*)gstate->X_Malloc(sizeof(alist_t),MEM_TAG_AI);

	alist->head     = NULL;
	alist->current  = NULL;
	alist->tail     = NULL;
	alist->count    = 0;

	return alist;
}

// ----------------------------------------------------------------------------
//
// Name:        alist_destroy
// Description:
//              destroy alist and NULL out the think function
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
alist_t *alist_destroy( alist_t *pList )
{
	_ASSERTE( pList );

	// SCG[11/13/99]: Handle NULL's
	if( pList == NULL || pList->head == NULL )
	{
		return NULL;
	}

    // make sure this entity is not already in the list
	alistNode_t *node = pList->head;
	alistNode_t *pNextNode;
	while ( node )
	{
		pNextNode = node->next;

		userEntity_t *pEntity = node->ent;
        if ( pEntity )
        {
            pEntity->think = NULL;
        }
        alist_remove( pEntity );

        if( pNextNode == NULL )
		{
			break;
		}

        node = pNextNode;
	}

    gstate->X_Free( pList );

    return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:        alist_add
// Description:
//	adds an entity to the end of alist, unless that entity is already in the list
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int alist_add( alist_t *alist, userEntity_t *ent )
{
	alistNode_t	*node;

	if ( !alist )
	{
		com->Error( "Attempted to add to an empty alist!\n" );
		return FALSE;
	}

	// make sure this entity is not already in the list
	node = alist->head;
	while ( node )
	{
		if ( node->ent == ent )
		{
        	return FALSE;
		}
        
        node = node->next;
	}

	// allocate the new node
//	node = new alistNode_t;
	node = (alistNode_t*)gstate->X_Malloc(sizeof(alistNode_t),MEM_TAG_AI);
	node->ent = ent;
	node->next = NULL;

	alist->count++;

	// nothing in the list
	if ( alist->head == NULL )
	{
		alist->head = node;
		alist->current = node;
		alist->tail = node;
	}
	// one or more items in the list
	else
	{
		alist->tail->next = node;
		alist->tail = node;
	}

    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        alist_remove
// Description:
//	adds an entity to the end of alist
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void UNIQUEID_Remove( const char *szUniqueID );
int alist_remove( alist_t *alist, userEntity_t *ent, bool fatalize )
{
	alistNode_t	*node, *prev;

	// SCG[2/8/00]: Heh. We might need to do this...
	if( ent->epair != NULL )
	{
		int	i = 0;
		while( ent->epair[i].key != NULL )
		{
			if( _stricmp( ent->epair[i].key, "uniqueid" ) == 0 )
			{
				UNIQUEID_Remove( ent->epair[i].value );
			}
			i++;
		}
	}

	if ( !alist->head )
	{
		if ( fatalize == true )         // fatal exit to console upon error?
        {
            com->Error( "Attempted to remove from an empty alist!\n" );
        }
/*
        else
        {
            gstate->Con_Printf( "No entities to nuke." );
        }
*/		
        return FALSE;
	}

	node = alist->head;

	// are we deleting the first node in the list?
	if ( node->ent == ent )
	{
		prev = NULL;
		// move the head to the next entity
		alist->head = node->next;
	}
	else
	{
		// traverse to the node pointing to ent
		while ( node && node->ent != ent )
		{
			prev = node;
			node = node->next;
		}

		// ent is not in this list
		if ( !node )
		{
        	return FALSE;
        }

		//	point the previous node to skip over the one we're deleting
		prev->next = node->next;
	}		
	
	//	if this is the tail we're deleting, point the tail to 
	//	the previous node
	if ( node == alist->tail )
	{
    	alist->tail = prev;
	}
    //	if this is the current node we're deleting, point the 
	//	current node to the previous node
	if ( node == alist->current )
	{
    	alist->current = prev;
    }

	alist->count--;

	// deallocate the memory
	//delete node;
	gstate->X_Free(node);

	node = NULL;
    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        alist_FirstEntity
// Description:
// returns the head of the list and initializes alist->current to the head of the list
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *alist_FirstEntity( alist_t *alist )
{
	// make current point to the head
	alist->current = alist->head;

	if ( alist->current )
	{
    	return alist->current->ent;
	}

	return	NULL;
}

// ----------------------------------------------------------------------------
//
// Name:        alist_NextEntity
// Description:
// Input:
// Output:
// Note:
//	returns the ent in the next node of the list from alist->current, or null if it is the end of
//	the alist
//	
//	ALWAYS call alist_FirstEntity once before calling alist_NextEntity, or the current pointer
//	could point to anywhere in the list
//
// ----------------------------------------------------------------------------
userEntity_t *alist_NextEntity( alist_t *alist )
{
	if ( alist->current )
	{
		alist->current = alist->current->next;

		if (alist->current)
		{
        	return alist->current->ent;
	    }
    }

	return NULL;
}

// ----------------------------------------------------------------------------
//
// Name:        alist_build
// Description:
// Input:
// Output:
// Note:
//	builds a list of all entities whose flags have at least one of the same bits set as
//	include_flag
//
// ----------------------------------------------------------------------------
void alist_build( alist_t *alist, int include_flag )
{
	userEntity_t	*head;
	int				count = 0;

	head = gstate->FirstEntity();
	while ( head )
	{
		// if head has matching flags, add head to the list of entities
		if ( head->flags & include_flag )
		{
			alist_add( alist, head );
//			gstate->Con_Dprintf ("added %s to list\n", head->className);
			count++;
		}

		head = gstate->NextEntity( head );
	}

//	gstate->Con_Printf ("count = %i\n", count);
}

// ----------------------------------------------------------------------------
//
// Name:        alist_add
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void alist_add( userEntity_t *self )
{
    playerHook_t *hook = AI_GetPlayerHook( self );

    int bSidekick = AI_IsSidekick( hook );

    if ( hook->type == TYPE_CLIENT || hook->type == TYPE_BOT || bSidekick )
    {
        if ( alist_add( client_list, self ) && bSidekick )
        {
            AIINFO_AddSidekick( self );
        }
    }
    else
    {
        alist_add( monster_list, self );
    }
}

// ----------------------------------------------------------------------------
//
// Name:        alist_remove
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void alist_remove( userEntity_t *self )
{
    playerHook_t *hook = AI_GetPlayerHook( self );

    int bSidekick = AI_IsSidekick( hook );

    if(hook)
	{
		if ( ( client_list != NULL ) && 
			( ( hook->type == TYPE_CLIENT ) || 
			( hook->type == TYPE_BOT ) || 
			bSidekick ) )
		{
			if ( alist_remove( client_list, self ) && bSidekick )
			{
				AIINFO_DeleteSidekick( self );
			}
		}
		else
		{
			alist_remove( monster_list, self );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

