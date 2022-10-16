// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================
///////////////////////////////////////////////////////////////////////////////////
//	thinkFuncs
//
//	manages a list of think functions associated with an entity thinklist->self->owner
//	thinkList->self is a fake entity and its owner is the real entity associated with
//	it.
//
//	spawns a fake entity for the think func.  Useful because it
//	duplicates all the entity fields.  However, it can't be used
//	for entities that you wish to be visible, because the engine
//	doesn't know anything about a fake entity
///////////////////////////////////////////////////////////////////////////////////

#include	"world.h"
//#include	"common.h"// SCG[1/23/00]: not used

/* ***************************** define types ****************************** */
typedef	struct	thinkFunc_s
{
	think_t			think_func;
	userEntity_t	*self;
	float			nextthink;

	thinkFunc_s		*next;
} thinkFunc_t;

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

///////////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////////

static	thinkFunc_t	*thinkList	= NULL;

///////////////////////////////////////////////////////////////////////////////////
//	functions
///////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
//
// Name:		thinkFunc_add
// Description:
//	adds a new think func
//
//	tf->self is a fake entity
//	returns self
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *thinkFunc_add( userEntity_t *owner, think_t think_func, float time )
{
	thinkFunc_t	*tf;

	tf = (thinkFunc_t*)gstate->X_Malloc(sizeof(thinkFunc_t), MEM_TAG_MISC);
	tf->self = com->SpawnFakeEntity ();
	tf->self->owner = owner;
	tf->think_func = think_func;
	tf->nextthink = gstate->time + time;

	if (!thinkList)
	{
		thinkList = tf;
		tf->next = NULL;

		return	tf->self;
	}

	//	no longer needed with fake entities	
/*
	//	make sure this entity doesn't already have a thinkFunc
	//	if it does, then don't add this one to the list

	for (l = thinkList; l; l = l->next)
	{
		if (owner == l->self->owner)
		{
			//	this entity has a think, so free new memory and set think_func 
			//	and time for the thinkFunc that already exists, then exit
			l->think_func = think_func;
			l->nextthink = gstate->time + time;

			com->free (tf->self);
			com->free (tf);

			return;
		}
	}
*/
	tf->next = thinkList;
	thinkList = tf;

	return	tf->self;
}

// ----------------------------------------------------------------------------
//
// Name:		thinkFunc_remove
// Description:
//	removes all thinkFuncs with self == ent
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void thinkFunc_remove( userEntity_t *ent )
{
	thinkFunc_t	*tf, *prev, *next;

	tf = prev = thinkList;

	//	first thinkFunc in list?
	if ( tf->self == ent )
	{
		thinkList = tf->next;
		
		com->RemoveFakeEntity (tf->self);
		gstate->X_Free(tf);
		return;
	}

	while (tf)
	{
		next = tf->next;
		
		if (tf->self == ent)
		{
			prev->next = next;
	
			com->RemoveFakeEntity (tf->self);
			gstate->X_Free(tf);
		}
		else
		{
			prev = tf;
		}

		tf = next;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		thinkFunc_set_think
// Description:
//	sets the function and think time
//	of a thinkFunc whose self = ent
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void thinkFunc_set_think( userEntity_t *ent, think_t think_func, float time )
{
	thinkFunc_t	*tf = thinkList;

	while ( tf )
	{
		if ( tf->self == ent )
		{
			tf->nextthink = gstate->time + time;
			tf->think_func = think_func;
		}

		tf = tf->next;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		thinkFunc_run_think
// Description:
//	executes the think function for a
//	single thinkFunc
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void thinkFunc_run_think( thinkFunc_t *tf )
{
    float thinktime;

	if ( tf->think_func )
	{
		thinktime = tf->nextthink;

//		gstate->Con_Printf ("run_think: thinktime = %.2f, ", thinktime);

		//	if nexthink is negative or zero or should not occur yet, then exit
	    if ( thinktime <= 0 || thinktime > gstate->time )
		{
			return;
		}
        
	    tf->nextthink = 0;

        tf->think_func (tf->self);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		thinkFunc_run_thinks
// Description:
//	executes the functions for all
//	the thinkFuncs
//
//	uses global thinkList
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void thinkFunc_run_thinks()
{
	if ( !thinkList )
	{
		return;
	}

	thinkFunc_t *tf = thinkList;
	while ( tf )
	{
		//	save next because thinkFuncs can remove themselves during thinks
		thinkFunc_t *next = tf->next;
		
		thinkFunc_run_think (tf);

		tf = next;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		thinkFunc_remove_thinks
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void thinkFunc_remove_thinks()
{
	thinkFunc_t	*cur, *next;

	cur = next = thinkList;
	while (cur)
	{
		next = cur->next;

		com->RemoveFakeEntity (cur->self);
		gstate->X_Free(cur);
		cur = next;
	}

	thinkList = NULL;
/*
	for (cur = next = thinkList; next; cur = next)
	{
		next = cur->next;

		com->RemoveFakeEntity (cur->self);
		com->free (cur);
		cur = NULL;
	}
*/
}

// ----------------------------------------------------------------------------
//
// Name:		thinkFunc_init
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void thinkFunc_init()
{
	thinkList = NULL;
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
