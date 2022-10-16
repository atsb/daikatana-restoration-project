#include "qcommon.h"
#include "memmgr.h"

#define min(a, b)		(((a) < (b)) ? (a) : (b)) 
#define isalpha(a)		((a) > 31 && (a) < 127)

typedef unsigned char byte;

#undef X_Malloc

/////////////////////////////////
// defines
/////////////////////////////////

#define ALIGN				4				// alignment size
#define LARGE_BLOCK_HEADER	0xCEEDDEEC
#define TEMP_BLOCK_HEADER	0xFEEFFEEF

#ifdef DEBUG_MEMORY
// SCG[8/2/99]: #define MEMMGR_LARGE_HEAP_SIZE	(48*1024*1024)	// size of memory pool  
//#define MEMMGR_LARGE_HEAP_SIZE	(32*1024*1024)	// size of memory pool  
#else
// SCG[8/2/99]: #define MEMMGR_LARGE_HEAP_SIZE	(48*1024*1024)	// size of memory pool  
//#define MEMMGR_LARGE_HEAP_SIZE	(32*1024*1024)	// size of memory pool  
#endif

#define NO_MEMORY_MANAGER   0
#define USE_Z_MALLOC		1


// SCG[11/4/99]: 
/////////////////////////////////
// quake zone memory management 
/////////////////////////////////

#define	Z_MAGIC		0x1d1d


typedef struct zhead_s
{
	struct zhead_s	*prev, *next;
	short	magic;
	short	tag;			// for group free
	int		size;
} zhead_t;

zhead_t		z_chain;
int		z_count, z_bytes;

/////////////////////////////////
// structs
/////////////////////////////////

typedef struct tagHeader
{
	long	lCheck;					// contains checknumber
	long	lSize;					// size of block
	MEM_TAG Tag;					// memory tag
#ifdef DEBUG_MEMORY
	unsigned int	nSeqNum;		// sequence number
	char	szFile[255];			// filename memory was alloc'd from
	int		nLine;					// line memory was alloc'd on
#endif
	struct tagHeader*	pNext;		// next block
	struct tagHeader*   pFreePrev;	// next free block
	struct tagHeader*	pFreeNext;	// prev free block
	byte	pMem[1];				// start of memory
} Header;

/////////////////////////////////
// globals
/////////////////////////////////

//static bool     bMemMgrError = false;
static bool		bActive = false;
//static void*	pLargeHeap;		// large memory pool
//static int		iHeaderSize;	// size of header
//static Header*	pLargeHead;		// memory head
//static void*	pLargeTail;		// memory end
//static Header*	pLargeFree;		// free list
//static Header*	pTempHead;		// temp memory
//static long		lLargeTotal;	// total used
//static long		lLargeMax;		// max used
//static long		lTempTotal;
#ifdef DEBUG_MEMORY
static unsigned int	nSequence;		// current sequence
#endif

memory_import_t	memmgr;

static char* szTags[] = 
{
	"FREE",			// free memory
	"TEMP",			// temp storage that shouldn't be around long
	"KEYBIND",		// key bindings and aliases
	"MODEL",		// model data
	"ARGV",			// argument memory
	"CVAR",			// cvars
	"FILE",			// file buffers and file system structs
	"CLIENT",		// clients structure
	"ENTITY",		// entities
	"SOUND",		// sound files
    "CIN",			// pics loaded by the cinematic
	"MISC",			// miscellanous memory
	"LEVEL",		// level memory
	"GAME",			// game memory (edicts, etc)
	"BSP",			// the map
	"BMODEL",		// brush models
	"SPRITE",		// sprite data
	"HOOK",			// user/player/monster hooks
	"MENU",			// menu memory
	"INVENTORY",	// inventory
	"COM_EPAIR",	// epairs allocated by com_malloc
	"COM_ENTITY",	// entities alloc by com_malloc
	"COM_INV",		// inventory - com_malloc
	"COM_MISC",		// misc memory to be freed at level change
	"EPAIR",		// normal epairs
	"GLPOLY",		// glpoly
	"COMMAND",		// commands
	"ALIAS",		// command aliases
	"COPYSTRING",	// copied strings - replaces strdup
	"AI",			// ai data
	"NODE",			// nodes
	"IMAGE",		// images
	"LAST"
};

//#if !NO_MEMORY_MANAGER
#if !USE_Z_MALLOC


/////////////////////////////////
// InitMemoryManager
//
// Allocates the base memory, and sets up the
// mem manager global variables.  Must be 
// called before any allocs.
//
////////////////////////////////

int InitMemoryManager()
{
	// malloc block of memory
	pLargeHeap = malloc(MEMMGR_LARGE_HEAP_SIZE);

	// return error if we can't alloc it
	if (!pLargeHeap)
	{
		Com_Error(ERR_FATAL,"MEMMGR: Couldn't allocate memory pool!\n");
		return -1;
	}

	// setup header size
	iHeaderSize = (int)((byte*)&pLargeHead->pMem[0]-(byte*)pLargeHead);

	// setup initial pointers
	pLargeHead = pLargeFree = (Header*)pLargeHeap;
	pTempHead = NULL;

	// init first block
	pLargeHead->lCheck 		= LARGE_BLOCK_HEADER;
	pLargeHead->lSize 		= MEMMGR_LARGE_HEAP_SIZE - iHeaderSize;
	pLargeHead->Tag			= MEM_TAG_FREE;
	pLargeHead->pNext 		= NULL;
	pLargeHead->pFreeNext	= NULL;
	pLargeHead->pFreePrev	= NULL;
	
	// set tail
	pLargeTail = (void*)((byte*)pLargeHead+MEMMGR_LARGE_HEAP_SIZE);

#ifdef DEBUG_MEMORY
	// setup Sequence
	nSequence = 0;
#endif

	// clear profiling variables
	lLargeTotal = lLargeMax = 0;

	memmgr.X_Malloc = X_Malloc;
	memmgr.X_Free = X_Free;

	bActive = true;

    bMemMgrError = false;

	z_chain.next = z_chain.prev = &z_chain;

	return 0;
}

////////////////////////////////////
// ShutdownMemoryManager
//
// frees all memory, reports leaks, and other
// general clean up.
//
////////////////////////////////////

int ShutdownMemoryManager()
{
	bActive = false;
	
	// free memory block back to os
	free(pLargeHeap);

	// free any temp memory
	Header* pNext;
	for (Header* pMem = pTempHead;pMem;pMem = pNext)
	{
		pNext = pMem->pNext;
		free(pMem);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//  Removes pMem from Free list and relinks
//
///////////////////////////////////////////////////////////////////////////////

__inline void Mem_Remove_From_Large_Free_List(Header* pMem) 
{
	// fix free list;
	if (pMem->pFreePrev) pMem->pFreePrev->pFreeNext = pMem->pFreeNext;
	if (pMem->pFreeNext) pMem->pFreeNext->pFreePrev = pMem->pFreePrev;

	// relink head
	if (pMem == pLargeFree) pLargeFree = pMem->pFreeNext;

	pMem->pFreeNext = NULL;
	pMem->pFreePrev = NULL;
}

__inline Header* Alloc_Large_Block(int size)
{
	Header* pMem;	
	Header* pNewMem = NULL;
	
	// find a free block big enough
	for (pMem = pLargeFree;pMem;pMem = pMem->pFreeNext)
	{
		// big enough?
		if (pMem->lSize >= size)
		{
			if (!pNewMem || pNewMem->lSize > pMem->lSize)
			{
				pNewMem = pMem;
			}
		}
	}

	// no block
	if (!pNewMem) return NULL;

	// set up memory to use
	pMem = pNewMem;

	// see if we can split the block
	if ((pMem->lSize-size) >= (iHeaderSize+ALIGN))
	{
		// find the pointer to the new block
		pNewMem = (Header*)((byte*)&pMem->pMem[0]+size);

		// setup initial state
		pNewMem->lCheck = LARGE_BLOCK_HEADER;
		pNewMem->lSize 	= pMem->lSize - size - iHeaderSize;
		pNewMem->Tag	= MEM_TAG_FREE;
		pNewMem->pNext 	= pMem->pNext;
		pNewMem->pFreeNext = pMem->pFreeNext;
		pNewMem->pFreePrev = pMem->pFreePrev;

		// fix old block
		pMem->lSize = size;
		pMem->pNext = pNewMem;

		// fix free list
		if (pMem->pFreePrev) pMem->pFreePrev->pFreeNext = pNewMem;
		if (pMem->pFreeNext) pMem->pFreeNext->pFreePrev = pNewMem;
	
		pMem->pFreePrev = NULL;
		pMem->pFreeNext = NULL;

		// relink head if necessary
		if (pMem == pLargeFree) pLargeFree = pNewMem;
	}
	else
	{
		Mem_Remove_From_Large_Free_List(pMem);
	}

	return pMem;
}

__inline Header* Alloc_Temp_Block(int size)
{
	Header* pMem;	

	pMem = (Header*)malloc(size+iHeaderSize);

	if (pMem)
	{
		// setup initial state
		pMem->lCheck	= TEMP_BLOCK_HEADER;
		pMem->lSize 	= size;
		pMem->pNext 	= pTempHead;
		pMem->pFreeNext = NULL;
		pMem->pFreePrev = NULL;

		pTempHead = pMem;
	
		lTempTotal += size;
	}

	return pMem;
}

/////////////////////////////////////////
// X_Malloc
//
// grab a piece of memory from the manager
//
/////////////////////////////////////////

#ifdef DEBUG_MEMORY
void* X_Malloc(size_t size, MEM_TAG tag, char* file, int line)
#else
void* X_Malloc(size_t size, MEM_TAG tag)
#endif
{
	Header* pMem;	
	bool	failed = false;

	if (!bActive) return NULL;

	// align size to boundary
	if (size % ALIGN)
	{
		size += ALIGN - (size%ALIGN);
	}

tryagain:
	pMem = Alloc_Large_Block(size);
	
	// no memory free!  life sucks!
	if (!pMem)
	{
		if (!failed)
		{
			failed = true;
			Mem_Compact(0);
			goto tryagain;
		}
		else
		{
//			pMem = Alloc_Temp_Block(size);
//			if (!pMem)
			{
				Com_DPrintf("********************\n");
				Com_DPrintf("Alloc of %lu bytes failed.\n",size);
				Com_Error(ERR_DROP,"MEMMGR: Unable to allocate memory!\n");
				return NULL;
			}
		}
	}

	// now all we have is the new block
	pMem->Tag = tag;

#ifdef DEBUG_MEMORY
	// put debug info in header
	strcpy(pMem->szFile,file);
	pMem->nLine = line;
	pMem->nSeqNum = nSequence;
#endif

	// zero fill the block
	memset(pMem->pMem,0,pMem->lSize);

	// count the block
	lLargeTotal += pMem->lSize;
	if (lLargeTotal >= lLargeMax)
		lLargeMax = lLargeTotal;

	// send the block back
	return (&pMem->pMem[0]);
}

////////////////////////////////////////////
// Mem_Heap_Walk
// 
// Walk the heap and check for bad stuff
// 
////////////////////////////////////////////

int Mem_Heap_Walk(int stats)
{
    // ISP [8-12-99] make sure this function gets called only once
    // if an error occurs
    if ( bMemMgrError == true )
    {
        return 0;
    }

	Header* pMem;
	Header* pPrev;
	long	lMemTotal = 0;
	long	lMemFree = 0;
	long	lMemUsed = 0;
	int		iBlkTotal = 0;
	int		iBlkFree = 0;
	int		iBlkUsed = 0;
	long	lMemWaste = 0;
	long	lTempTotal = 0;
	int		iTempBlk = 0;
	long	lTempWaste = 0;
	
	for (pMem = pLargeHead;pMem;pMem = pMem->pNext)
	{
		if (pMem->lCheck != LARGE_BLOCK_HEADER)
		{
			bMemMgrError = true;
            Com_Error(ERR_DROP,"MEMMGR: Bad Block in Large Heap!\n");
			return -1;
		}

		lMemTotal += pMem->lSize + iHeaderSize;
		iBlkTotal++;
		if (pMem->Tag == MEM_TAG_FREE)
		{
			lMemFree += pMem->lSize;
			iBlkFree++;
		}
		else
		{
			lMemUsed += pMem->lSize;
			iBlkUsed++;
		}
		lMemWaste += iHeaderSize;
	
		// set previous memory only useful for debugging
		pPrev = pMem;
	}

	if (lMemTotal != MEMMGR_LARGE_HEAP_SIZE)
	{
		Com_Error(ERR_DROP,"MEMMGR: Memory Mismatch!  Total was %lu\n",lMemTotal);
		return -1;
	}	

	for (pMem = pTempHead;pMem;pMem = pMem->pNext)
	{
		if (pMem->lCheck != TEMP_BLOCK_HEADER)
		{
			Com_Error(ERR_DROP,"MEMMGR: Bad Block in Temp Heap!\n");
			return -1;
		}

		lTempTotal += pMem->lSize + iHeaderSize;
		iTempBlk++;
		lTempWaste += iHeaderSize;
	
		// set previous memory only useful for debugging
		pPrev = pMem;
	}

	if (stats)
	{
		Com_DPrintf("Used:  %lu bytes in %i blocks (%02.1f%%)\n",lMemUsed,iBlkUsed,((float)lMemUsed/(float)lMemTotal)*100.0f);
		Com_DPrintf("Free:  %lu bytes in %i blocks (%02.1f%%)\n",lMemFree,iBlkFree,((float)lMemFree/(float)lMemTotal)*100.0f);
		Com_DPrintf("Total: %lu bytes in %i blocks (%lu bytes wasted)\n",lMemTotal,iBlkTotal,lMemWaste);
		Com_DPrintf("Temp:  %lu bytes in %i blocks (%lu bytes wasted)\n",lTempTotal,iTempBlk,lTempWaste);
		Com_DPrintf("Using: %lu bytes  (Max: %lu bytes)\n",lMemUsed+lTempTotal,lLargeMax);
	}
		
	return 0;
}

///////////////////////////////////////////
// X_Free
//
// frees the block 
//
///////////////////////////////////////////

void X_Free(void* mem)
{
    Header* pMem;

	if (!bActive) return;

	// do we even have valid memory?
	if (!mem) return;

    // get header for block
    pMem = (Header*)((byte*)mem-iHeaderSize);
                        
    // check the checksum
	if (pMem->lCheck == LARGE_BLOCK_HEADER)
	{
		// make sure we aren't freeing freed memory
		if (pMem->Tag == MEM_TAG_FREE)
		{
    		Com_DPrintf("X_Free: mem already freed!\n");
			return;
		}

#ifdef _DEBUG
		if( *pMem->pMem )
		{
			// clear out the freed block
			memset(pMem->pMem,0,pMem->lSize);
		}
#endif

		// remove from total
		lLargeTotal -= pMem->lSize;

		// add to top of free list
		pMem->pFreePrev = NULL;
		pMem->pFreeNext = pLargeFree;
		if( pLargeFree )
		{
			pLargeFree->pFreePrev = pMem;
			pLargeFree = pMem;
		}

		// set the block to freed
		pMem->Tag = MEM_TAG_FREE;

		// see if we can combine 
		while (pMem->pNext && pMem->pNext->Tag == MEM_TAG_FREE)
		{
			// reset size
			pMem->lSize += pMem->pNext->lSize + iHeaderSize;
			// remove from free list
			Mem_Remove_From_Large_Free_List(pMem->pNext);
			// reset next marker
			pMem->pNext = pMem->pNext->pNext;
		}
	}
	else if (pMem->lCheck == TEMP_BLOCK_HEADER)
	{
		Header* pTemp;

		pTemp = pMem;
		for (pMem = pTempHead;pMem;pMem = pMem->pNext)
		{
			if (pMem->pNext == pTemp)
			{
				pMem->pNext = pTemp->pNext;
				break;
			}
		}
	
		if (pTemp == pTempHead) pTempHead = pTemp->pNext;

		// remove from total
		lLargeTotal -= pTemp->lSize;
		lTempTotal -= pTemp->lSize;

		free(pTemp);
	}
	else
    {
		if ((mem > pLargeHead) && (mem < pLargeTail))
		{
    		Com_DPrintf("X_Free: bad block!\n");
			return;
		}
		else
		{
			// not ours must be before we got here
			free(mem);
			return;
		}
    }
}

void Mem_Compact(int stats)
{
	Header* pMem = NULL;
	int merged = 0;
	
	// walk the free list looking to combine
	for (pMem = pLargeFree;pMem;pMem = pMem->pFreeNext)
	{
		// see if we can combine 
		while (pMem->pNext && pMem->pNext->Tag == MEM_TAG_FREE)
		{
			// reset size
			pMem->lSize += pMem->pNext->lSize + iHeaderSize;
			// remove from free list
			Mem_Remove_From_Large_Free_List(pMem->pNext);
			// reset next marker
			pMem->pNext = pMem->pNext->pNext;
			// increase merged count
			merged++;
		}
	}

	if (stats)
	{
		Com_DPrintf("Compact merged %d blocks.\n",merged);
	}
}

void Mem_Free_Tag(MEM_TAG tag)
{
	Header* pMem;

	for (pMem = pLargeHead;pMem;pMem = pMem->pNext)
	{
		if (pMem->Tag == tag)
		{
			X_Free(pMem->pMem);
		}
	}

	for (pMem = pTempHead;pMem;pMem = pMem->pNext)
	{
		if (pMem->Tag == tag)
		{
			X_Free(pMem->pMem);
		}
	}

	Mem_Compact(0);
}

void Mem_Dump()
{
	Header* pMem;
	FILE* f;
	char buf[255];
	char file[255];
	int i, len;

	if (GetArgc() > 1)
	{
		sprintf(file,"%s",GetArgv(1));
	}
	else
	{
		strcpy(file,"c:/dk_memory_log.txt");
	}

	f = fopen(file,"w+");
	if (!f)
	{
		Com_Printf("Unable to open %s\n",file);
		return;
	}
	
	for (pMem = pLargeHead;pMem;pMem = pMem->pNext)
	{
		if (pMem->lCheck != LARGE_BLOCK_HEADER)
		{
			sprintf(buf,"BAD BLOCK!\n");
			break;
		}
		else
		{
#ifdef DEBUG_MEMORY			
			sprintf(buf,"%02X - [%s] File: %s  Line: %d  Size: %lu\n",pMem->nSeqNum,szTags[pMem->Tag],pMem->szFile,pMem->nLine,pMem->lSize);
#else
			sprintf(buf,"[%s] Size: %lu\n",szTags[pMem->Tag],pMem->lSize);
#endif
			if (pMem->Tag == MEM_TAG_FREE)
			{
				// write out data
				len = min(pMem->lSize,16);
				sprintf(buf,"%s  Data: <",buf);
				// write text
				for (i=0;i<len;i++)
				{
					if (isalpha(pMem->pMem[i]))
						sprintf(buf,"%s%c",buf,pMem->pMem[i]);
					else
						sprintf(buf,"%s ",buf);
				}
				sprintf(buf,"%s>",buf);
				// write hex
				for (i=0;i<len;i++)
					sprintf(buf,"%s %02X",buf,pMem->pMem[i]);
				sprintf(buf,"%s\n",buf);
			}
		}

		// write it out
		fwrite(buf,strlen(buf),1,f);
	}

	Com_Printf("Wrote Memory Dump %s\n",file);

	fclose(f);
}

void Mem_Inc_Sequence()
{
#ifdef DEBUG_MEMORY
	nSequence++;
	Com_DPrintf("Sequence is now %lu\n",nSequence);
#endif
}

void Mem_Dump_Total(bool bLong)
{
	long	lMemTotal = 0;
	Header* pMem;
	long	lTagTotal[MEM_TAG_LAST];
	long 	TagNum[MEM_TAG_LAST];
	bool	change = true;
	long	temp;
	int		i;
	
	// clear stats
	for (i=0;i<MEM_TAG_LAST;i++)
	{
		TagNum[i] = i;
		lTagTotal[i] = 0;	
	}

	for (pMem = pLargeHead;pMem;pMem = pMem->pNext)
	{
		if (pMem->lCheck != LARGE_BLOCK_HEADER)
		{
			Com_Error(ERR_DROP,"MEMMGR: Bad Block in Large Heap!\n");
			return;
		}

		if (pMem->Tag != MEM_TAG_FREE)
		{
			lTagTotal[pMem->Tag] += pMem->lSize;
			lMemTotal += pMem->lSize;
		}
	}

	for (pMem = pTempHead;pMem;pMem = pMem->pNext)
	{
		if (pMem->lCheck != TEMP_BLOCK_HEADER)
		{
			Com_Error(ERR_DROP,"MEMMGR: Bad Block in Temp Heap!\n");
			return;
		}

		if (pMem->Tag != MEM_TAG_FREE)
		{
			lTagTotal[pMem->Tag] += pMem->lSize;
			lMemTotal += pMem->lSize;
		}
	}

	// sort the results
	while (change)
	{
		change = false;
		for (i=0;i<(MEM_TAG_LAST-1);i++)
		{
			if (lTagTotal[i] < lTagTotal[i+1])
			{
				change = true;
				// swap tag num
				temp = TagNum[i];
				TagNum[i] = TagNum[i+1];
				TagNum[i+1] = temp;
				// swap tag total
				temp = lTagTotal[i];
				lTagTotal[i] = lTagTotal[i+1];
				lTagTotal[i+1] = temp;
			}
		}
	}

	if (bLong)
	{
		// done now display output
		for (i=0;i<MEM_TAG_LAST;i++)
		{
			if (lTagTotal[i]) Com_DPrintf("%-20s %luk\n",szTags[TagNum[i]],lTagTotal[i]/1024);
		}
		Com_DPrintf("\nTotal: %luk\n",lMemTotal/1024);
	}
	else
	{
		// write short display
		Com_DPrintf("Tot: %luk  %s %luk  %s %luk  %s %luk  %s %luk\n",
			lMemTotal/1024,szTags[TagNum[0]],lTagTotal[0]/1024,szTags[TagNum[1]],lTagTotal[1]/1024,
			szTags[TagNum[2]],lTagTotal[2]/1024,szTags[TagNum[3]],lTagTotal[3]/1024);
	}

	Com_DPrintf(" Free: %luk\n", ( MEMMGR_LARGE_HEAP_SIZE - lMemTotal ) / 1024 );
}

//#else // #if !NO_MEMORY_MANAGER
#else // #if !USE_Z_MALLOC

static unsigned long total_count;

int InitMemoryManager()
{
	memmgr.X_Malloc = X_Malloc;
	memmgr.X_Free = X_Free;

	z_chain.next = z_chain.prev = &z_chain;

	bActive = true;

	_CrtSetDbgFlag( _CRTDBG_DELAY_FREE_MEM_DF );

	total_count = 0;
	return 0;
}

int ShutdownMemoryManager()
{
    bActive = false;

/*
	zhead_t	*z, *next;

	for( z = z_chain.next ; z != &z_chain ; z = next )
	{
		next = z->next;
		Z_Free ((void *)(z+1));
	}
*/
	// SCG[11/16/99]: Done for debugging purposes, in case one
	// SCG[11/16/99]: of these fails with "damage to normal block" or 
	// SCG[11/16/99]: similar.
	Mem_Free_Tag( MEM_TAG_TEMP );
	Mem_Free_Tag( MEM_TAG_KEYBIND );
	Mem_Free_Tag( MEM_TAG_MODEL );
	Mem_Free_Tag( MEM_TAG_ARGV );
	Mem_Free_Tag( MEM_TAG_CVAR );
	Mem_Free_Tag( MEM_TAG_FILE );
	Mem_Free_Tag( MEM_TAG_CLIENT );
	Mem_Free_Tag( MEM_TAG_ENTITY );
	Mem_Free_Tag( MEM_TAG_SOUND );
    Mem_Free_Tag( MEM_TAG_CIN );
	Mem_Free_Tag( MEM_TAG_MISC );
	Mem_Free_Tag( MEM_TAG_LEVEL );
	Mem_Free_Tag( MEM_TAG_GAME );
	Mem_Free_Tag( MEM_TAG_BSP );
	Mem_Free_Tag( MEM_TAG_BMODEL );
	Mem_Free_Tag( MEM_TAG_SPRITE );
//	Mem_Free_Tag( MEM_TAG_HOOK );
	Mem_Free_Tag( MEM_TAG_MENU );
	Mem_Free_Tag( MEM_TAG_INVENTORY );
//	Mem_Free_Tag( MEM_TAG_COM_EPAIR );
//	Mem_Free_Tag( MEM_TAG_COM_ENTITY );
//	Mem_Free_Tag( MEM_TAG_COM_INV );
//	Mem_Free_Tag( MEM_TAG_COM_MISC );
	Mem_Free_Tag( MEM_TAG_EPAIR );
	Mem_Free_Tag( MEM_TAG_GLPOLY );
	Mem_Free_Tag( MEM_TAG_COMMAND );
	Mem_Free_Tag( MEM_TAG_ALIAS );
	Mem_Free_Tag( MEM_TAG_COPYSTRING );
	Mem_Free_Tag( MEM_TAG_NODE );
	Mem_Free_Tag( MEM_TAG_IMAGE );
	Mem_Free_Tag( MEM_TAG_LAST );
//	Mem_Free_Tag( MEM_TAG_AI );

    _CrtDumpMemoryLeaks();
	return 0;
}

#ifdef DEBUG_MEMORY
void* X_Malloc(size_t size, MEM_TAG tag, char* file, int line)
#else
void* X_Malloc(size_t size, MEM_TAG tag)
#endif
{
    if( !bActive )
        return NULL;
    
	return Z_TagMalloc( size, tag );
}

int Mem_Heap_Walk(int stats)
{
	return 0;
}

void X_Free(void* mem)
{
    if (!bActive) return;
	
	if (!mem) return;

	Z_Free( mem );
}

void Mem_Compact(int stats)
{
}

void Mem_Free_Tag(MEM_TAG tag)
{
	Z_FreeTags( tag );
}

void Mem_Dump()
{
}

void Mem_Inc_Sequence()
{
}

void Mem_Dump_Total(bool bLong)
{
	long	lMemTotal = 0;
	long	lTagTotal[MEM_TAG_LAST];
	long 	TagNum[MEM_TAG_LAST];
	bool	change = true;
	long	temp;
	int		i;
	zhead_t	*z, *next;

	// clear stats
	for( i = 0; i < MEM_TAG_LAST; i++ )
	{
		TagNum[i] = i;
		lTagTotal[i] = 0;	
	}

	for( z = z_chain.next ; z != &z_chain ; z = next )
	{
		next = z->next;

		if (z->tag != MEM_TAG_FREE)
		{
			lTagTotal[z->tag] += z->size;
			lMemTotal += z->size;
		}
	}

	// sort the results
	while( change )
	{
		change = false;
		for (i=0;i<(MEM_TAG_LAST-1);i++)
		{
			if (lTagTotal[i] < lTagTotal[i+1])
			{
				change = true;
				// swap tag num
				temp = TagNum[i];
				TagNum[i] = TagNum[i+1];
				TagNum[i+1] = temp;
				// swap tag total
				temp = lTagTotal[i];
				lTagTotal[i] = lTagTotal[i+1];
				lTagTotal[i+1] = temp;
			}
		}
	}

	if( bLong )
	{
		// done now display output
		for( i = 0; i < MEM_TAG_LAST; i++ )
		{
			if( lTagTotal[i] ) Com_DPrintf( "%-20s %luk\n",szTags[TagNum[i]], lTagTotal[i] / 1024 );
		}

		Com_DPrintf( "\nTotal: %luk\n", lMemTotal / 1024);
	}
	else
	{
		// write short display
		Com_DPrintf( "Tot: %luk  %s %luk  %s %luk  %s %luk  %s %luk\n",
			lMemTotal / 1024,
			szTags[ TagNum[0] ],
			lTagTotal[0] / 1024,
			szTags[ TagNum[1] ],
			lTagTotal[1] / 1024,
			szTags[ TagNum[2] ],
			lTagTotal[2] / 1024,
			szTags[ TagNum[3] ],
			lTagTotal[3] / 1024);
	}

	Com_DPrintf( "Total count: %luk\n", total_count / 1024 );

	Com_DPrintf( "Unaccounted for: %luk\n", ( total_count / 1024 ) - ( lMemTotal / 1024 ) );
}

#endif

// SCG[11/4/99]: Quake2 Stuff!
/*
==============================================================================

						ZONE MEMORY ALLOCATION

just cleared malloc with counters now...

==============================================================================
*/

/*
========================
Z_Free
========================
*/
void Z_Free (void *ptr)
{
	zhead_t	*z;

	z = ((zhead_t *)ptr) - 1;

	if( z == 0 )
	{
		return;
	}

	_ASSERTE( z->magic == Z_MAGIC );

	if (z->magic != Z_MAGIC)
	{
		Com_Error (ERR_FATAL, "Z_Free: bad magic");
	}

	z->prev->next = z->next;
	z->next->prev = z->prev;

	z_count--;
	z_bytes -= z->size;

	total_count -= z->size;
	free (z);
}


/*
========================
Z_Stats_f
========================
*/
void Z_Stats_f (void)
{
	Com_Printf ("%i bytes in %i blocks\n", z_bytes, z_count);
}

/*
========================
Z_FreeTags
========================
*/
void Z_FreeTags (int tag)
{
	zhead_t	*z, *next;

	if( z_chain.next == NULL )
	{
		return;
	}
	for (z=z_chain.next ; z && z != &z_chain ; z=next)
	{
		next = z->next;
		if (z->tag == tag)
			Z_Free ((void *)(z+1));
	}
}

/*
========================
Z_TagMalloc
========================
*/
void *Z_TagMalloc (int size, int tag)
{
	zhead_t	*z;
	
	size = size + sizeof(zhead_t);
	z = ( zhead_t * ) malloc(size);
	if (!z)
		Com_Error (ERR_FATAL, "Z_Malloc: failed on allocation of %i bytes",size);
	memset (z, 0, size);
	z_count++;
	z_bytes += size;
	z->magic = Z_MAGIC;
	z->tag = tag;
	z->size = size;

	z->next = z_chain.next;
	z->prev = &z_chain;
	z_chain.next->prev = z;
	z_chain.next = z;

	total_count += size;

	return (void *)(z+1);
}

/*
========================
Z_Malloc
========================
*/
void *Z_Malloc (int size)
{
	return Z_TagMalloc (size, 0);
}
