#ifndef MEMMGR_H
#define MEMMGR_H

#ifdef _DEBUG
//#define DEBUG_MEMORY
#endif

#define	TAG_GAME	765		// clear when unloading the dll
#define	TAG_LEVEL	766		// clear when loading a new level
#define	TAG_HOOK	767		// also clear when loading a new level

typedef enum
{
	MEM_TAG_FREE,						// free memory
	MEM_TAG_TEMP,						// temp storage that shouldn't be around long
	MEM_TAG_KEYBIND,					// key bindings and aliases
	MEM_TAG_MODEL,						// model data
	MEM_TAG_ARGV,						// argument memory
	MEM_TAG_CVAR,						// cvars
	MEM_TAG_FILE,						// file buffers and file system structs
	MEM_TAG_CLIENT,						// clients structure
	MEM_TAG_ENTITY,						// entities
	MEM_TAG_SOUND,						// sound files
    MEM_TAG_CIN,						// pics loaded by the cinematic
	MEM_TAG_MISC,						// miscellanous memory
	MEM_TAG_LEVEL,						// level memory
	MEM_TAG_GAME,						// game memory (edicts, etc)
	MEM_TAG_BSP,						// the map
	MEM_TAG_BMODEL,						// brush models
	MEM_TAG_SPRITE,						// sprite data
	MEM_TAG_HOOK,						// user/player/monster hooks
	MEM_TAG_MENU,						// menu memory
	MEM_TAG_INVENTORY,					// inventory
	MEM_TAG_COM_EPAIR,					// epairs allocated by com_malloc
	MEM_TAG_COM_ENTITY,					// entities alloc by com_malloc
	MEM_TAG_COM_INV,					// inventory - com_malloc
	MEM_TAG_COM_MISC,					// misc memory to be freed at level change
	MEM_TAG_EPAIR,						// normal epairs
	MEM_TAG_GLPOLY,						// gl poly structure - freed at level unload
	MEM_TAG_COMMAND,					// commands
	MEM_TAG_ALIAS,						// command aliases
	MEM_TAG_COPYSTRING,					// copied strings - replaces strdup
	MEM_TAG_AI,							// ai
	MEM_TAG_NODE,						// node
	MEM_TAG_IMAGE,						// images
	MEM_TAG_LAST						// always should be last
} MEM_TAG;

// prototypes
int InitMemoryManager();
int ShutdownMemoryManager();
const char* GetMemoryError();
int Mem_Heap_Walk(int stats);
void Mem_Compact(int stats);
void Mem_Free_Tag(MEM_TAG tag);
void Mem_Dump();
void Mem_Inc_Sequence();
void Mem_Dump_Total(bool bLong);

#ifdef DEBUG_MEMORY
void* X_Malloc(size_t size, MEM_TAG tag, char* file, int line);
#else
void* X_Malloc(size_t size, MEM_TAG tag);
#endif
void X_Free(void* mem);

void* X_MallocDebug(size_t size, MEM_TAG tag, const char *filename );

#ifdef DEBUG_MEMORY
#define X_Malloc(a, b) X_Malloc((a), (b), __FILE__, __LINE__)
#endif

void Z_Free (void *ptr);
void Z_Stats_f (void);
void Z_FreeTags (int tag);
void *Z_TagMalloc (int size, int tag);
void *Z_Malloc (int size);

#endif // MEMMGR_H

