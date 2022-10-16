//#include <windows.h>
#include <stdio.h>
#include <math.h>

#include	"p_user.h"
#include	"hooks.h"

#include	"common.h"
#include	"com_globals.h"

//////////////////////////////////////////////////////////////////////////////////////////
// prototypes for memory allocation routines
//////////////////////////////////////////////////////////////////////////////////////////
/*
void			*com_malloc (unsigned long size, char mem_type);
void			com_free (void *ptr);
void			com_freemost (void);
void			com_freeall (void);
*/

//////////////////////////////////////////////////////////////////////////////////////////
// global variables
//////////////////////////////////////////////////////////////////////////////////////////

listHeader_t	*malloc_list;
unsigned long	malloc_count;

extern	serverState_t	*gstate;

/////////////////////////////////////////////////////////////////////////////////////
//	Memory handling functions
//
//	Routines for managing a global list of malloc'd memory
//
//	This list is initialized when the first DLL is loaded and everything
//	remaining in the list is freed when either a level exits or Daikatana exits
//
//	Any memory allocated in Daikatana should be malloc'd using these routines
//	to ensure that all the memory is freed under Win 95.
//
//
/////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////
//	com_count_mem_type
//
//	returns the number of pointers the list
//	that point to malloc'd memory of type mem_type
//////////////////////////////

unsigned long	com_count_mem_type (char mem_type)
{
	listNode_t		*l;
	unsigned long	count = 0;

	if (malloc_list->head)
	{
		for (l = malloc_list->head; l; l = com.list_node_next (l))
		{
			if (l->type == mem_type)
				count++;
		}
	}

	return	count;
}

//////////////////////////////
//	com_malloc
//
//	mallocs up a single ptr and
//	saves it in malloc_list
//////////////////////////////

void	*com_malloc (unsigned long size, char mem_type)
{
	void	*ptr;

	ptr = malloc (size);

	com.list_add_node (ptr, malloc_list, mem_type);

	malloc_count++;

	return	ptr;
}

//////////////////////////////
//	com_free
// 
//	removes a single ptr from the
//	list and frees its memory
//////////////////////////////

void	com_free (void *ptr)
{
	if (!ptr)
		return;

	com.list_remove_node (ptr, malloc_list);

	malloc_count--;

	free (ptr);
}

//////////////////////////////
//	com_free_most
// 
//	removes all memory not of type MEM_PERMANENT
//////////////////////////////

void	com_free_most (void)
{
	listNode_t	*node, *next_node;
	void		*ptr;
	int			i, count, freed;

	gstate->Con_Dprintf ("COMMON.DLL, dll_ExitLevel: malloc_count = %i\n", malloc_count);
	gstate->Con_Dprintf ("  %i MEM_HUNK\n  %i MEM_MALLOC\n  %i MEM_PERMANENT\n", 
		com_count_mem_type (MEM_HUNK), com_count_mem_type (MEM_MALLOC), com_count_mem_type (MEM_PERMANENT)); 

	if (malloc_list)
	{
		freed = 0;
		node = malloc_list->head;
		count = malloc_list->count;

		// traverse entire list, if not MEM_PERM, remove node
		for (i = 0; node && i < count; i++)
		{
			next_node = node->next;

			if (node->type != MEM_PERMANENT)
			{
				next_node = node->next;

				ptr = node->ptr;
				com.list_delete_node (malloc_list, node);

				free (ptr);

				freed++;
				malloc_count--;
			}

			node = next_node;
		}
	}
	
	gstate->Con_Dprintf ("com_free_most: freed %i pointers\n", i);
}

//////////////////////////////
//	com_free_type
// 
//	removes all memory of type mem_type
//////////////////////////////

void	com_free_type (char mem_type)
{
	listNode_t	*node, *next_node;
	void		*ptr;
	int			i, count, freed;

	gstate->Con_Dprintf ("COMMON.DLL, dll_ExitLevel: malloc_count = %i\n", malloc_count);
	gstate->Con_Dprintf ("  %i MEM_HUNK\n  %i MEM_MALLOC\n  %i MEM_PERMANENT\n", 
		com_count_mem_type (MEM_HUNK), com_count_mem_type (MEM_MALLOC), com_count_mem_type (MEM_PERMANENT)); 

	if (malloc_list)
	{
		freed = 0;
		node = malloc_list->head;
		count = malloc_list->count;

		// traverse entire list, if not MEM_PERM, remove node
		for (i = 0; node && i < count; i++)
		{
			next_node = node->next;

			if (node->type == mem_type)
			{
				next_node = node->next;

				ptr = node->ptr;
				com.list_delete_node (malloc_list, node);

				free (ptr);

				freed++;
				malloc_count--;
			}

			node = next_node;
		}
	}
	
	gstate->Con_Dprintf ("com_free_type: freed %i pointers\n", i);
}

//////////////////////////////
//	com_free_all
// 
//	removes malloc_list and
//	frees all pointers in each node
//////////////////////////////

void	com_free_all (void)
{
	void	*ptr;
	int		i;

	i = 0;

	if (malloc_list)
	{
		while (malloc_list->head)
		{
			ptr = malloc_list->tail->ptr;
			
			com.list_delete_node (malloc_list, malloc_list->tail);
			
			free (ptr);
			i++;
			malloc_count--;
		}
	}

	gstate->Con_Dprintf ("com_free_all: freed %i pointers\n", i);
}

///////////////////////////////////////////////////
// com_malloc_hook
//
// allocates and initializes memory for a userhook
//
// mallocing as MEM_PERMANENT will cause the hook to remain
// between level changes
//
// HUNK memory is taken from Quake's memory pool
// and is deallocated when a level ends.
// MALLOC and PERMANENT memory is malloc'd from Windows memory.
///////////////////////////////////////////////////

void	*com_malloc_hook (unsigned long size, int mem_type)
{
	void	*temp;

	if (mem_type == MEM_HUNK)
		//	Q2FIX: no Hunk_Alloc in Quake2
//		temp = gstate->Hunk_Alloc (size);
		temp = com_malloc (size, MEM_MALLOC);
	else if (mem_type == MEM_MALLOC)
		temp = com_malloc (size, MEM_MALLOC);
	else if (mem_type == MEM_PERMANENT)
		temp = com_malloc (size, MEM_PERMANENT);
	else
		gstate->Error ("Tried to alloc an unknown memory type");

	memset (temp, 0x00, size);

	return	temp;
}

///////////////////////////////////////////////////////////////////////////////
//	com_InitHeap
//
//	set up pointers to com_heap.cpp functions
///////////////////////////////////////////////////////////////////////////////

void	com_InitHeap (void)
{
	com.malloc = com_malloc;
	com.free = com_free;
	com.free_most = com_free_most;
	com.free_all = com_free_all;
	com.free_type = com_free_type;
	com.count_mem_type = com_count_mem_type;
	com.malloc_hook = com_malloc_hook;
}
