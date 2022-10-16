//#include <windows.h>
#if _MSC_VER
#include <crtdbg.h>
#endif
#include <stdio.h>
#include <math.h>

#include	"p_user.h"

#include	"p_global.h"

/////////////////////////////////////////////////////////////////////////////////////
// list functions
//
// manages a doubly linked list that points to any type of data
/////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////
// com_list_ptr
//
// returns the entity pointed to
// by node
///////////////////////////////

void	*com_list_ptr (listNode_t *node)
{
	return	node->ptr;
}

///////////////////////////////
// com_list_node_next
//
// returns a pointer to the node
// after node
///////////////////////////////

listNode_t	*com_list_node_next (listNode_t *node)
{
	return	node->next;
}

///////////////////////////////
// com_list_prev
//
// returns a pointer to the node
// preceding node
///////////////////////////////

listNode_t	*com_list_node_prev (listNode_t *node)
{
	return	node->prev;
}

///////////////////////////////
// com_list_init
//
// initialize a linked list
///////////////////////////////

listHeader_t	*com_list_init (listHeader_t *list)
{
	// mdm 99.07.07 - alloc does not return if it fails
	list = (listHeader_t *) gi.X_Malloc (sizeof (listHeader_t), MEM_TAG_MISC);
	list->head = NULL;
	list->tail = NULL;
	list->count = 0;

	return	list;
}

///////////////////////////////
// com_list_new_node
//
// create a new node and return
// a pointer to it
///////////////////////////////

listNode_t	*com_list_new_node (void)
{
	listNode_t	*node;

	// alloc does not fail and zero fills memory
	node = (listNode_t *) gi.X_Malloc (sizeof (listNode_t), MEM_TAG_MISC);

	return	node;
}

///////////////////////////////
// com_list_add_node
//
// add a node to the list which
// points to self
///////////////////////////////

void	com_list_add_node (void *ptr, listHeader_t *list, char mem_type)
{
	listNode_t	*node;

	if (!list)
	{
		gi.Error ("Attempted to add to uninitialized weapon list!");
	}

	// create a new node
	node = com_list_new_node ();
	
	// point this node to self
	node->ptr = ptr;
	node->type = mem_type;

	if (list->head == NULL)
	{
		// list is empty
		list->head = node;
		list->tail = node;
		node->prev = NULL;
		node->next = NULL;
	}
	else
	{
		// one or more nodes in list
		
		// make the tail to point to the new node
		list->tail->next = node;

		// point new node back to tail
		node->prev = list->tail;
		// new node is last in list, so point next to NULL
		node->next = NULL;

		// make the new node the tail
		list->tail = node;
	}

	list->count++;
}

///////////////////////////////
// com_list_delete_node
//
// remove node from list
///////////////////////////////

void	com_list_delete_node (listHeader_t *list, listNode_t *node)
{
	if (!node)
	{
		gi.Error ("Attempted to remove a null node from a list!");
	}

	if (node == list->head)
	{
		// node is the head of the list
		list->head = node->next;
	}
	if (node == list->tail)
	{
		// node is the tail of the list
		list->tail = node->prev;
	}

	if (node->next)
	{
		node->next->prev = node->prev;
	}
	if (node->prev)
	{
		node->prev->next = node->next;
	}

	gi.X_Free(node);
//	free(node);
	list->count--;
}

///////////////////////////////
// com_list_remove_node
//
// remove all nodes from the list
// which point to self
///////////////////////////////

void	com_list_remove_node (void *ptr, listHeader_t *list)
{
	listNode_t	*cur_node, *last_node;

	if (!list || !list->head)
	{
		_ASSERTE(FALSE);
		gi.Error ("Attempted to remove from empty list!");
	}

	cur_node = list->head;

	while (cur_node)
	{
		_ASSERTE ((int)cur_node->next != 0xcdcdcdcd);
		_ASSERTE ((int)cur_node->next != 0xdcdcdcdc);
		_ASSERTE ((int)cur_node->next != 0xdddddddd);

		if (cur_node->ptr == ptr)
		{
			last_node = cur_node;
			cur_node = com_list_node_next (cur_node);
			com_list_delete_node (list, last_node);
		}
		else
		{
			cur_node = com_list_node_next (cur_node);
		}
	}
}

//////////////////////////////////
//	com_list_kill
//
//	frees up memory for all nodes
//	and the list header
//////////////////////////////////

void	com_list_kill (listHeader_t *list)
{
	if (!list)
	{
		return;
	}

	while (list->head != NULL)
	{
		com_list_delete_node (list, list->tail);
	}

	gi.X_Free(list);
//	free (list);
}

///////////////////////////////////////////////////////////////////////////////
//	com_InitList
//
//	set up pointers to com_list.cpp functions
///////////////////////////////////////////////////////////////////////////////

void	com_InitList (void)
{
	com.list_init = com_list_init;
	com.list_kill = com_list_kill;
	com.list_new_node = com_list_new_node;
	com.list_add_node = com_list_add_node;
	com.list_delete_node = com_list_delete_node;
	com.list_remove_node = com_list_remove_node;
	com.list_ptr = com_list_ptr;
	com.list_node_next = com_list_node_next;
	com.list_node_prev = com_list_node_prev;
}