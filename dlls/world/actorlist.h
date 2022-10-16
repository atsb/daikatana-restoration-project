#ifndef _ACTORLIST_H
#define _ACTORLIST_H

///////////////////////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////////////////////

typedef struct	alistNode_s
{
	userEntity_t	*ent;
	
	alistNode_s		*next;
} alistNode_t;

typedef	struct	alist_s
{
	alistNode_s		*head;
	alistNode_s		*tail;
	alistNode_s		*current;
	int			count;
} alist_t;

extern	alist_t	*client_list;
extern	alist_t	*monster_list;

///////////////////////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////////////////////

alist_t			*alist_init();
alist_t         *alist_destroy( alist_t *pList );
userEntity_t	*alist_FirstEntity( alist_t *alist );
userEntity_t	*alist_NextEntity( alist_t *alist );
void			alist_build( alist_t *alist, int include_flag );

void alist_add( userEntity_t *self );
void alist_remove( userEntity_t *self );

#endif
