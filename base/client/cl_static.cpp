///////////////////////////////////////////////////////////////////////////////
//	cl_static.c
//
//	Nelno:	this is not implemented yet.  Will be done after E3 for torches,
//			etc. when there is more time to flesh this out.
//
//	handles drawing of static entites.
//	Static entites are entites which only exist on the client.  They are
//	sent down by the server, then removed from the server client list.  The
//	client tracks the entities, determines if they are in the PVS, and sends
//	them to the renderer if they should be drawn.
//
//	Static ents cannot be removed until a level restarts.
//
//	Uses a pseudo-linked list to keep from traversing an array, however,
//	the static entities are allocated from a static array to avoid mallocing.
//
//	Static entities can be animated by the client code.
//
//	TODO:	what happens when a client connects to a server that is already
//			running?  How does the new client get the static entities sent
//			to it?  Does the server need to keep a static entity list?
//			You know, this kind of sucks.
//
//	To create a static entity:
//
//	On the server:
//		spawn the entity normally
//		set EF_STATIC
//		send entity down, then remove it from entity list (what about mem
//			that was allocated in the DLLs, like userHook?  need a routine
//			to ensure that memory gets freed when ent is removed...)
//
//	On the client:
//		check for EF_STATIC flag on entities
//		call static_AddEntity to add entities with EF_STATIC
//		do not do anything else to these ents
//		call static_AddToRefresh to set up all static entities
//			and send them to the refresh dll
//		remove all static ents at the end or beginning of a new level
//
///////////////////////////////////////////////////////////////////////////////

#include	"client.h"

///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

typedef	struct	staticEnt_s
{
	entity_state_t	ent;
	
	int				areanum;

} staticEnt_t;

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define	MAX_STATIC_ENTS	64

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

static	staticEnt_t	staticList	[MAX_STATIC_ENTS];
static	int			static_free;	//	current free spot for next ent

///////////////////////////////////////////////////////////////////////////////
//	static_InitList
//
///////////////////////////////////////////////////////////////////////////////

void	static_InitList (void)
{
	memset (staticList, 0x00, sizeof (staticList));
	
	static_free = 0;
}


///////////////////////////////////////////////////////////////////////////////
//	static_AddToRefresh
//
//	goes through the list of static entities and determines which ones need
//	to be sent to the renderer.
///////////////////////////////////////////////////////////////////////////////

void	static_AddToRefresh (void)
{

}

///////////////////////////////////////////////////////////////////////////////
//	static_AddStaticEntity
//
//	Adds an entity to the list of static entities.
//
//	When the server wants to make an entity static it sets the EF_STATIC
//	flag on the entity, sends the entity down, and removes the entity.  
//	When the client recieves an entity with EF_STATIC it calls this routine
//	to add it to the static entity list and then continues to parse the 
//	remaining entities.
///////////////////////////////////////////////////////////////////////////////

void	static_AddStaticEntity (entity_state_t *ent)
{
//	staticEnt_t	*se;

	if (static_free == MAX_STATIC_ENTS)
	{
		Com_Printf ("static_AddStaticEntity: No free slots.\n");
		return;
	}
}