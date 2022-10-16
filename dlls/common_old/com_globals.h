#ifndef	_COM_GLOBAL_H
#define	_COM_GLOBAL_H

void	com_InitFriction	(void);
void	com_InitHeap		(void);
void	com_InitList		(void);
void	com_InitMain		(void);
void	com_InitSub			(void);

//	defined in com_heap.cpp
extern	listHeader_t	*malloc_list;
extern	unsigned long	malloc_count;

//	defined in com_sub.cpp
extern	serverState_t	*gstate;
extern	common_export_t	com;
extern	Vector			forward, right, up;

#endif