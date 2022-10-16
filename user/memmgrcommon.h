#ifndef MEMMGR_COMMON_H
#define MEMMGR_COMMON_H

#include "memmgr.h"

typedef struct memory_import_s
{
#ifdef DEBUG_MEMORY
	void*	(*X_Malloc) (size_t size, MEM_TAG tag, char* file, int line);
#else
	void*	(*X_Malloc) (size_t size, MEM_TAG tag);
#endif
	void	(*X_Free) (void* mem);
} memory_import_t;

extern memory_import_t memmgr;

#endif