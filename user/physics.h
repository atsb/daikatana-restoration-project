#define Quake_DllExport    __declspec( dllexport )
#define DllExport	extern "C"	__declspec( dllexport )
#ifndef	DllExport_Plus
#define DllExport_Plus __declspec(dllexport)
#endif

//unix - DllExport
#if !_MSC_VER
#undef Quake_DllExport
#undef DllExport_Plus
#undef DllExport
#define Quake_DllExport
#define DLLExport_Plus
#define DllExport extern "C"
#endif

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define	GAME_API_VERSION	1

// edict->svflags

#define	SVF_NOCLIENT			0x00000001	//	don't send entity to clients, even if it has effects
#define	SVF_DEADMONSTER			0x00000002	//	treat as CONTENTS_DEADMONSTER for collision
#define	SVF_MONSTER				0x00000004	//	treat as CONTENTS_MONSTER for collision
#define	SVF_NOSPLASH			0x00000008	//	no default water transition sound
#define	SVF_NOTHUD				0x00000010	//	no default landing sound
#define	SVF_OWNERONLY			0x00000020  //	only owner sees this entity
#define	SVF_DEBUGBREAK			0x00000040	//	for debugging a particular entity in server code
#define	SVF_SHOT				0x00000080	//	don't clip against ents with clipmask == MASK_SHOT
#define	SVF_NOTOWNER			0x00000100  //	everyone but owner sees this (for nightmare weapon)
#define	SVF_SYNCTOCLIENT		0x00000200	//	entity will have its physics updated during client thinks
											//	this will only happen until the next server think, then flag is cleared
#define	SVF_NEWSPAWN			0x00000400	//	set when on a newly spawned entity, cleared  after first pass through physics
											//	stops entities who just spawned from causing a splash sound
#define	SVF_PUSHED				0x00000800	
#define SVF_NOPUSHCOLLIDE		0x00001000  //  set primarily on items that should not collide with b-models
#define SVF_AWARDEDEXP			0x00002000	//  Have given EXP for this entity.
#define	SVF_ITEM				0x00004000  // SCG[3/9/00]: last minute hack for trigger tracing so sidekicks can pick up items the player points to

#define	MAX_ENT_CLUSTERS		16

enum svc_ops_enum
{
	SVC_BAD,

	// these ops are known to the game dll
	SVC_MUZZLEFLASH,
	SVC_MUZZLEFLASH2,
	SVC_TEMP_ENTITY,
	SVC_LAYOUT,
	SVC_INVENTORY,
	SVC_ONSCREEN_ICON,         // 3.2 dsn
	SVC_CLIENTDEATH,           // 3.4
	SVC_SIDEKICK_UPDATE,       // 5.25
	//SVC_SIDEKICK_COMMAND,
  SVC_SELECTOR,
	SVC_WEAPON_ICON,          
	SVC_XPLEVEL,               // experience level up
  SVC_SAVEGEM,
  SVC_CLIENTRESPAWN,         

	// the rest are private to the client and server, NOT!
	SVC_NOP,
	SVC_DISCONNECT,
	SVC_RECONNECT,
	SVC_SOUND,					// <see code>
	SVC_PRINT,					// [byte] id [string] null terminated string
	SVC_STUFFTEXT,				// [string] stuffed into client's console buffer, should be \n terminated
	SVC_SERVERDATA,				// [long] protocol ...
	SVC_CONFIGSTRING,			// [short] [string]
	SVC_SPAWNBASELINE,		
	SVC_CENTERPRINT,			// [string] to put in center of the screen
	SVC_DOWNLOAD,				// [short] size [size bytes]
	SVC_PLAYERINFO,				// variable
	SVC_PACKETENTITIES,			// [...]
	SVC_DELTAPACKETENTITIES,	// [...]
	SVC_FRAME,
	SVC_EFFECT_NOTIFY,			//	Shawn:	added for Daikatana
	SVC_SETCLIENTMODELS,		//	Nelno:	send down edict numbers for a client body parts
	SVC_SYNCENTITY,				//	Nelno:	entity is syncing with client for one server frame
	SVC_SCOREBOARD,
	SVC_CLIENTENTITY,			//  Logic: Client gets some brains, see libclient source for info
	SVC_CROSSHAIR,				// SCG[10/4/99]: For enabling/disabling the crosshair
	SVC_END_OF_GAME,			// SCG[1/18/00]: 
	SVC_AMMO_COUNT_UPDATE,
	SVC_SIDEKICK_WEAPON_STATUS,
	SVC_START_MUSIC,
	SVC_START_MP3,
	SVC_STOP_MP3,
	SVC_REVERB_PRESET,
	SVC_START_STREAM
};

#define SCOREBOARD_SINGLE	0x0		// SCG[1/5/00]: 
#define SCOREBOARD_COOP		0x1		// SCG[1/5/00]: 
#define SCOREBOARD_CTF		0x2		// SCG[1/5/00]: 
#define SCOREBOARD_MULTI	0x4		// SCG[1/5/00]: 

///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

//	edict->solid values

typedef enum
{
	SOLID_NOT,			// no interaction with other objects
	SOLID_TRIGGER,		// only touch when inside, after moving
	SOLID_BBOX,			// touch on edge
	SOLID_BSP			// bsp clip, touch on edge
} solid_t;

#define	SOLID_SLIDEBOX	SOLID_BBOX

// Shawn: added for getting frame information
typedef struct frameinfo_s
{
	char	name[16];
	int		flags;
} frameinfo_t;

// link_t is only used for entity area links now
typedef struct link_s
{
	struct link_s	*prev, *next;
} link_t;


typedef struct edict_s edict_t;
typedef struct gclient_s gclient_t;


#ifndef GAME_INCLUDE

typedef struct gclient_s
{
	player_state_t	ps;		// communicated by server to clients
	int				ping;
	// the game dll can add anything it wants after
	// this point in the structure
} gclient_t;

///////////////////////////////////////////////////////////////////////////////
//	THIS IS THE SERVER VERSION OF edict_t.	DLLS SHOULD ALWAYS USE
//	THE LONGER userEntity_t AND NOT edict_t!!!!!!!!!!!!!!!!!!!!!!!
//	WHAT A FRICKIN MESS THIS CRAP IS...
///////////////////////////////////////////////////////////////////////////////

struct edict_s
{
	entity_state_t	s;
	struct gclient_s	*client;
	qboolean	inuse;
	int			linkcount;

	// FIXME: move these fields to a server private sv_entity_t
	link_t		area;				// linked to a division node or leaf
	
	int			num_clusters;		// if -1, use headnode instead
	int			clusternums[MAX_ENT_CLUSTERS];
	int			headnode;			// unused if num_clusters != -1
	int			areanum, areanum2;

	//================================

	int			svflags;			// SVF_NOCLIENT, SVF_DEADMONSTER, SVF_MONSTER, etc
//	CVector		mins, maxs;
	CVector		absmin, absmax, size;
	solid_t		solid;
	int			clipmask;
	edict_t		*owner;

	char		*className;

	player_record_t	record;			// player record - kills, deaths, stats, exp
	// the game dll can add anything it wants after
	// this point in the structure
};

#endif		// GAME_INCLUDE

///////////////////////////////////////////////////////////////////////////////
//	inventory typedefs
///////////////////////////////////////////////////////////////////////////////

typedef enum {
    MEM_HUNK,
    MEM_MALLOC,
    MEM_PERMANENT,

    HOOK_HUNK,
    HOOK_MALLOC,
    HOOK_PERMANENT,
} mem_type;

typedef void (*invenUse_t) (struct userInventory_s *ptr, struct edict_s *user);
typedef void *(*invenCommand_t) (struct userInventory_s *ptr, char *commandStr, void *data);
typedef	invenUse_t		weaponUse_t;
typedef	invenCommand_t	weaponCommand_t;
//typedef void (*weaponUse_t) (struct userInventory_s *ptr, struct edict_s *user);
//typedef void *(*weaponCommand_t) (struct userInventory_s *ptr, char *commandStr, void *data);

// cek[1-14-99] well, it seems someone decided to mix flags together.  The WFL_* flags are used when a weapon
// is added to the inventory.  the cool thing is that the ITF_* flags overlap these flags.  cool, eh?
#define	ITF_WEAPON			0x00010000	//	item is a weapon
#define	ITF_AMMO			0x00020000	//	item is ammo for a weapon
#define ITF_POWERUP			0x00040000	//  item is a powerup
#define ITF_USEONCE			0x00080000	//  item is only used once
#define ITF_UNLOCK			0x00100000	//  when this item is used, it unlocks the entity which requires it
							
#define	ITF_PROJECTILE		0x00200000		//	used by AI to determine weapon type
#define	ITF_TRACE		    0x00400000		//	used by AI to determine weapon type
#define	ITF_BULLET			0x00800000		//	used by AI to determine weapon type
#define	ITF_EXPLOSIVE		0x01000000		//	used by AI to determine weapon type
#define	ITF_SPLASH			0x02000000		//	smart monsters will try for splash damage

#define	ITF_SPECIAL			0x04000000		//	unique items
#define ITF_NOLEAD			0x08000000		//  make sure the weapon do not lead the target

#define ITF_INVSACK			0x10000000    //  can be stored in inventory 'sack'
#define ITF_COOP_REMOVEALL	0X20000000		// in coop mode, check all players for this item and remove it.
#define ITF_KEY_ITEM		0x40000000		// this item is a key_item
/*
#define	ITF_WEAPON	    0x00000001	//	item is a weapon
#define	ITF_AMMO	      0x00000002	//	item is ammo for a weapon
#define ITF_POWERUP	    0x00000004	//  item is a powerup
#define ITF_USEONCE	    0x00000008	//  item is only used once
#define ITF_UNLOCK	    0x00000010	//  when this item is used, it unlocks the entity which requires it

#define	ITF_PROJECTILE	0x00000020		//	used by AI to determine weapon type
#define	ITF_TRACE		    0x00000040		//	used by AI to determine weapon type
#define	ITF_BULLET		  0x00000080		//	used by AI to determine weapon type
#define	ITF_EXPLOSIVE	  0x00000100		//	used by AI to determine weapon type
#define	ITF_SPLASH		  0x00000200		//	smart monsters will try for splash damage

#define	ITF_SPECIAL		  0x00000400		//	unique items
#define ITF_NOLEAD		  0x00000800		//  make sure the weapon do not lead the target

#define ITF_INVSACK     0x00001000    //  can be stored in inventory 'sack'
*/

typedef struct userInventory_s
{
	char			*name;
	invenUse_t		use;
	invenCommand_t	command;
	int				modelIndex;
	char			modelName[MAX_OSPATH];
	unsigned long	flags;
	mem_type		memType;
	void			*dummy4;
} userInventory_t;

typedef struct	keyItem_s
{
	//	!!!! FIELDS BELOW MUST BE IDENTICAL TO userInventory_t!!!!
	userInventory_t	invRec;
	//	!!!! FIELDS ABOVE MUST BE IDENTICAL TO userInventory_t!!!!
	char	target[64];	// entity this key/book is used upon
	char	netname[64];	// entity net name	
	int		soundIndex;		// sound to play when used
	char	soundName[MAX_OSPATH];
} keyItem_t;

typedef	struct	invenItem_s
{
	userInventory_t		*data;

	struct	invenItem_s	*prev;
	struct	invenItem_s	*next;
} invenItem_t;

typedef struct
{
	invenItem_t		*head;
	invenItem_t		*tail;

	invenItem_t		*curItem;		//	for walking the list with FirstItem and NextItem
	invenItem_t		*selectedItem;	//	item currently selected by owner of invnetory

	mem_type		memType;
} invenList_t;

///////////////////////////////////////////////////////////////////////////////
//	hierarchical model structures
///////////////////////////////////////////////////////////////////////////////

//	used on the server
/*
typedef	struct	childInfo_s
{
	int					type;

	unsigned long int	flags;
	struct edict_s		*parent;

	//	pointer to function that handles movement
	void				(*func) (edict_t *child, struct childInfo_s *cinfo);

	///////////////////////////////////////////////////////////////////////////
	//	bmodel and old, simple "hierarchical" sticking
	///////////////////////////////////////////////////////////////////////////

	//	for hierarchial stickies
	CVector				dir;	//	direction from parent org to child org
	float				dist;	//	distance from parent org to child org
	CVector				ang_delta;	//	difference between parent and child angles

	//	for sticking to bmodels
	int		rotate_x;

	float   hyp_xy;
	float   hyp_yz;
	float   hyp_xz;

	CVector	last_angle;  //Other's angles previous frame
	CVector	last_origin; //Other's last origin values (see if we are a platform moving and rotating)

	///////////////////////////////////////////////////////////////////////////
	//	Q2KILL:	old, two-surface models 
	///////////////////////////////////////////////////////////////////////////

	int	forward_surfIndex;
	int	forward_vertIndex;				//	number of vertices into parent surface where master tri is

	int	right_surfIndex;
	int	right_vertIndex;				//	number of vertices into parent surface where master tri is

	float	pitch_delta;

	CVector	offset;

	///////////////////////////////////////////////////////////////////////////
	//	single-tri surface matching
	///////////////////////////////////////////////////////////////////////////

	int	surfIndex;
	int	origin_vertex;
	int	right_vertex;
	int	forward_vertex;
} childInfo_t;

//	list structure:	server version, uses edict_t
typedef	struct	childList_s
{
	struct	childList_s	*next;
	edict_t				*child;

	childInfo_t			*info;
} childList_t;
*/

// from ref.h
typedef struct entity_s entity_t;

///////////////////////////////////////////////////////////////////////////////
// functions provided by the main engine
///////////////////////////////////////////////////////////////////////////////

typedef struct game_import_t
{
	// special messages
	void	(*bprintf) (int printlevel, char *fmt, ...);
	void	(*Con_Dprintf)(char *fmt, ...);
	void	(*Con_Printf)(char *fmt, ...);
	void	(*cprintf) (edict_t *ent, int printlevel, char *fmt, ...);
	void	(*centerprintf) (edict_t *ent, char *fmt, ...);
	void	(*centerprint) (edict_t *ent, float msg_time, char *fmt, ...);

	// sound engine calls
	void	(*StartSound) (CVector &origin, edict_t *ent, int channel, int soundinedex, float volume, float dist_min, float dist_max, float timeofs);
    void    (*set_reverb_preset) (int nPresetID) ; // jas
	void	(*streamed_sound) (const char* name, float vol);

	void	(*StartMusic) (const char* name, int chan, edict_t *ent = NULL);
	void    (*StartMP3) (const char* name, int chan, float fvol, int play_count, qboolean bRestart = TRUE, edict_t *ent = NULL );	
	void    (*StopMP3) (int chan, edict_t *ent = NULL);	

	// config strings hold all the index strings, the lightstyles,
	// and misc data like the sky definition and cdtrack.
	// All of the current configstrings are sent to clients when
	// they connect, and changes are sent to all connected clients.
	void	(*configstring) (int num, char *string);

	void	(*Error) (char *fmt, ...);

	// new names can only be added during spawning
	// existing names can be looked up at any time
	int		(*modelindex) (const char *name);
	int		(*soundindex) (const char *name);
	int		(*imageindex) (const char *name);
	char	*(*GetConfigString) (int index);

	void	(*setmodel) (edict_t *ent, char *name);

	// collision detection
	trace_t	(*TraceLine) ( CVector &start, CVector &end, edict_t *passent, int contentmask);
	trace_t	(*TraceLineTrigger) ( CVector &start, CVector &end, edict_t *passent, int contentmask, unsigned int nAcceptFlag );

	trace_t	(*TraceBox) ( CVector &start, CVector &mins, CVector &maxs, CVector &end, edict_t *passent, int contentmask);
	int		(*pointcontents) (CVector &point);
	qboolean	(*inPVS) (CVector &p1, CVector &p2);
	qboolean	(*inPHS) (CVector &p1, CVector &p2);
	void		(*SetAreaPortalState) (int portalnum, qboolean open);
	qboolean	(*AreasConnected) (int area1, int area2);
	int			(*GetTraceCount)(void);

	// an entity will never be sent to a client or used for collision
	// if it is not passed to linkentity.  If the size, position, or
	// solidity changes, it must be relinked.
	void	(*linkentity) (edict_t *ent);
	void	(*unlinkentity) (edict_t *ent);		// call before removing an interactive edict
	int		(*BoxEdicts) (CVector &mins, CVector &maxs, edict_t **list,	int maxcount, int areatype);
	void	(*Pmove) (pmove_t *pmove);		// player movement code common with client prediction

	// network messaging
	void	(*multicast) (CVector &origin, multicast_t to);
	void	(*unicast) (edict_t *ent, qboolean reliable);
	void	(*WriteChar) (int c);
	void	(*WriteByte) (int c);
	void	(*WriteShort) (int c);
	void	(*WriteLong) (int c);
	void	(*WriteFloat) (float f);
	void	(*WriteString) (char *s);
	void	(*WritePosition) (CVector &pos);	// some fractional bits
	void	(*WriteDir) (CVector &pos);		// single byte encoded, very coarse
	void	(*WriteAngle) (float f);

#ifdef DEBUG_MEMORY
	void*	(*X_Malloc) (size_t size, MEM_TAG tag, char* file, int line);
#else
	void*	(*X_Malloc) (size_t size, MEM_TAG tag);
#endif
	void	(*X_Free) (void* mem);
	void	(*Mem_Free_Tag) (MEM_TAG tag);
	int		(*Mem_Heap_Walk) (int stats);

	// console variable interaction
	cvar_t	*(*cvar) (const char *var_name, const char *value, int flags);
	cvar_t	*(*cvar_set) (const char *var_name, const char *value);
	cvar_t	*(*cvar_forceset) (const char *var_name, const char *value);

	// ClientCommand and coneole command parameter checking
	int		(*GetArgc)();
	char	*(*GetArgv)(int n);
	char	*(*GetArgs)();

	// add commands to the server console as if they were typed in
	// for map changing, etc
	void	(*CBuf_AddText) (const char *text);

	void	(*DebugGraph) (float value, int color);

	void	(*RemoveDLLCommands)();
	void	(*AddCommand) (char *cmd_name, void (*function) (struct edict_s *ent));

	char	*dk_dlldir;
	char	*dk_gamedir;
	char	*dk_gamename;

	//	for getting the command-line parameters used to start Daikatana
	int		(*EXEGetArgc)();
	char	*(*EXEGetArgv)(int arg);

	void (*printxy)(int x, int y, char *fmt, ...);
    void (*clearxy)();

	void (*dk_GetFrameName) (int modelindex, int frameindex, char *name);
	int		(*dk_GetAnimSequences)	(int modelindex, frameData_t *pFrameData);

	int		(*dk_GetModelSkinIndex) (int modelIndex);

	char	*sv_prepped_refresh;		//	set to true when precached models have been loaded

	//	for viewmodel debugging command
	void	(*dk_ForceModelLoad) (int modelindex, char *name);
	void	(*dk_PreCacheModel) (int modelindex, char *name);
	int		(*Sys_Milliseconds)();

	//	for hierarchical model testing
	int		(*VertInfo) (edict_t *ent, int surfIndex, int vertIndex, CVector &vert);
	int		(*SurfIndex) (int modelIndex, char *surfName);
	int		(*TriVerts) (edict_t *ent, int surfIndex, int vertIndex, CVector &v1, CVector &v2, CVector &v3);

	//	PAK File IO routines
	int		(*FS_FileLength)	(FILE *F);
	void	(*FS_Close)			(FILE *F);
	int		(*FS_Open)			(const char *filename, FILE **file);
	void	(*FS_Read)			(void *buffer, int len, FILE *f);
	int		(*FS_LoadFile)		(const char *path, void **buffer);
	void	(*FS_FreeFile)		(void *buffer);
	int		(*FS_Seek)			(FILE *f, long offset, int origin);
	long	(*FS_Tell)			(FILE *f);
	int		(*FS_Getc)			(FILE *f);
	int		(*FS_Ungetc)		(int c, FILE *f);

	char*	(*FS_Gets)			(char *string, int n, FILE *f);
	long	(*FS_GetBytesRead)	();

    //functions exported from the exe for in game cinematic playback.
    void	(*CIN_CamLocation)(CVector &pos, CVector &angles);
    void	*(*CIN_GCE_Interface)();
    void	(*CIN_WorldPlayback)(void *funcs);

	void	(*SV_EmitSyncEntity)	(edict_t *client_entity, edict_t *ent);

	// SCG[11/4/99]: quake type memory allocation
	// managed memory allocation
	void	*(*TagMalloc) (int size, int tag);
	void	(*TagFree) (void *block);
	void	(*FreeTags) (int tag);

	short	(*IsPlayerLocal)		(int playerNum);

	// these functions simply perform the test that network writes and sends perform on the buffer
	qboolean (*TestMulticast)		(multicast_t to);
	qboolean (*TestUnicast)			(edict_t *ent, qboolean reliable);
} game_import_t;

///////////////////////////////////////////////////////////////////////////////
// functions exported by the game subsystem
///////////////////////////////////////////////////////////////////////////////

typedef struct game_export_s
{
	int			apiversion;

	void		(*SetServerTime)(float time);
	// each new level entered will cause a call to SpawnEntities
	void		(*SpawnEntities) (char *mapname, char *entstring, char *spawnpoint, qboolean bLoadgame );

	// Read/Write Game is for storing persistant cross level information
	// about the world state and the clients.
	// WriteGame is called every time a level is exited.
	// ReadGame is called on a loadgame.
	void		(*WriteGame) (char *filename, char autosave);
	void		(*ReadGame) (char *filename);

	void		(*WriteHeader) (char *filename, char *comment, qboolean autoSave);

	// ReadLevel is called after the default map information has been
	// loaded with SpawnEntities, so any stored client spawn spots will
	// be used when the clients reconnect.

	void		(*WriteLevel) (char *filename);
	void		(*ReadLevel) (char *filename);

	qboolean	(*ClientConnect) (edict_t *ent, char *userinfo, qboolean loadgame);
	void		(*ClientBegin) (edict_t *ent, qboolean loadgame);
	void		(*ClientUserinfoChanged) (edict_t *ent, char *userinfo);
	void		(*ClientDisconnect) (edict_t *ent);
	void		(*ClientCommand) (edict_t *ent);
	void		(*ClientThink) (edict_t *ent, usercmd_t *cmd);

	void		(*RunFrame)();

	void		(*RegisterFunc)(char* name, void* func); 

	// ServerCommand will be called when an "sv <command>" command is issued on the
	// server console.
	// The game can issue gi.argc() / gi.argv() commands to get the rest
	// of the parameters
	void		(*ServerCommand)();

	//
	// global variables shared between game and server
	//

	// The edict array is allocated in the game dll so it
	// can vary in size from one game to another.
	// 
	// The size will be fixed when ge->Init() is called
	struct		edict_s		*edicts;
	int			edict_size;
	int			num_edicts;		// current number, <= max_edicts
	int			max_edicts;

	//	Nelno's additions
	
	void		(*LevelLoad)();			//	called each time a new level loads
	void		(*LevelExit)();			//	called each time a level ends, before next level is loaded
	void		(*InitDLLs)();				//	called after all DLLs have loaded the first time
	void		(*UnloadDLLs)();			//	called when all DLLs are unloaded
	void		(*InitChangelevel)();		//	called when a "changelevel" command is issued
	
	void		(*LoadNodes)( char *pMapName );
	void		(*RegisterWorldFuncs)();

	void		(*EntityLoadCleanup)( int nIndex );
} game_export_t;
