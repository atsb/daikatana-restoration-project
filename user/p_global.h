///////////////////////////////////////////////////////////////////////////////
//	global include file for physics DLL
///////////////////////////////////////////////////////////////////////////////

#ifndef _G_GLOBAL_H
#define _G_GLOBAL_H

#include	"dk_shared.h"
#include	"p_user.h"
// Logic[8/17/99]: this was directly pathed, major no-no!
#include	"../base/dk_/dk_log.h"

#include	"common.h"

#define	FRAMETIME		1.0F / (1000.0F / SV_FRAME_MILLISECONDS)

extern	cvar_t	*maxentities;
extern	cvar_t	*deathmatch;
extern	cvar_t	*coop;
extern	cvar_t	*dmflags;
extern	cvar_t	*skill;
extern	cvar_t	*fraglimit;
extern	cvar_t	*timelimit;
extern	cvar_t	*samelevel;
extern	cvar_t	*noexit;
extern	cvar_t	*g_select_empty;
extern	cvar_t	*g_unlimited_ammo;
extern	cvar_t	*dm_respawn;
extern	cvar_t	*dm_camping_time;
extern	cvar_t	*dm_camping_remind_time;
extern	cvar_t	*dm_camping_notify;
extern	cvar_t	*nomonsters;

extern	cvar_t	*p_gravity;
extern	cvar_t	*p_maxvelocity;

extern	cvar_t	*gun_x, *gun_y, *gun_z;
extern	cvar_t	*p_rollspeed;
extern	cvar_t	*p_rollangle;

extern	cvar_t	*run_pitch;
extern	cvar_t	*run_roll;
extern	cvar_t	*bob_up;
extern	cvar_t	*bob_pitch;
extern	cvar_t	*bob_roll;

extern	cvar_t	*p_cheats;
extern	cvar_t	*dedicated;
extern	cvar_t	*maxclients;

extern	cvar_t	*hr_testvert;
extern	cvar_t	*hr_shownormals;

extern	cvar_t	*p_runphysics;
extern	cvar_t	*p_speeds;
extern	cvar_t	*p_sync;
extern	cvar_t	*p_sendparticles;

//	for slow motion physics
extern	cvar_t	*p_frametime;
extern	float	p_frametime_scaled;
extern	int		p_realtime;

//	for auto-kicking spammers, ticks are in frames (10 per second)
extern	cvar_t	*p_spamkick;			//	auto-kick spammers if not 0
extern	cvar_t	*p_spamcount;		//	how many spams before user gets kicked
extern	cvar_t	*p_spamticks;		//	how long between messages from same use constitutes a spam
extern	cvar_t	*p_spamresetticks;	//	how long before resetting spam counter
extern	cvar_t	*sv_violence;

extern cvar_t  *allow_friendly_fire;

// memory tags to allow dynamic memory to be cleaned up
#define	TAG_GAME	765		// clear when unloading the dll
#define	TAG_LEVEL	766		// clear when loading a new level


extern	game_import_t	gi;
extern	game_export_t	globals;
extern	level_locals_t	level;
extern  game_locals_t   game;

extern	edict_t			*g_edicts;

// multiplayer options
//extern	cvar_t	*dm_frag_limit;
//extern	cvar_t	*dm_time_limit;
extern	cvar_t	*dm_skill_level;
extern	cvar_t	*dm_max_players;

// deathmatch flags
extern	cvar_t	*dm_weapons_stay;
extern	cvar_t	*dm_spawn_farthest;
extern	cvar_t	*dm_allow_exiting;
extern	cvar_t	*dm_same_map;
extern	cvar_t	*dm_force_respawn;
extern	cvar_t	*dm_falling_damage;
//extern	cvar_t	*dm_instant_powerups;
extern	cvar_t	*dm_allow_powerups;
extern	cvar_t	*dm_allow_health;
extern	cvar_t	*dm_allow_armor;
extern	cvar_t	*dm_infinite_ammo;
extern	cvar_t	*dm_fixed_fov;
extern	cvar_t	*dm_teamplay;
extern	cvar_t	*dm_friendly_fire;
extern  cvar_t  *dm_footsteps;
extern  cvar_t  *dm_allow_hook;

extern serverState_t serverState;

///////////////////////////////////////////////////////////////////////////////
//	dk_dll.c
///////////////////////////////////////////////////////////////////////////////

extern	void	(*dll_ClientDisconnect)		(userEntity_t *self);
extern	void	(*dll_ClientBegin)			(userEntity_t *self, int loadgame);
extern	int		(*dll_ClientConnect)		(userEntity_t *self, char *userinfo, int loadgame);
extern	void	(*dll_ClientUserinfoChanged)(edict_t *ent, char *userinfo);
extern	void	(*dll_ClientThink)			(userEntity_t *ent, usercmd_t *ucmd, pmove_t *pm);
extern	void	(*dll_ClientBeginServerFrame)	(userEntity_t *ent);
extern	void	(*dll_think_stub)			(userEntity_t *ent);
extern	void	(*dll_SetStats)				(userEntity_t *ent);
extern	void	(*dll_UpdateBoundingBoxes)	(userEntity_t *ent);
extern	void	(*dll_NPCAlert)				(userEntity_t *owner, userEntity_t *attacker);
extern	void	(*dll_BeginIntermission)	(char *nextMap);
extern	void	(*dll_LoadNodes)			( char *pMapName );
extern	void	(*dll_RegisterWorldFuncs)	();
extern	void	(*dll_EntityLoadCleanup)	( int nIndex );

extern  int		(*dll_FLAG_GetScores)		( teamInfo_t *scorearr, int maxscores ); // WAW[11/16/99]: Here because the #!?@! scoreboard is in physics.
extern	int		(*dll_FLAG_CheckRules)		();

extern	short	(*dll_DT_CanDamage)			(userEntity_t *self, userEntity_t *attacker, unsigned long damage_flags);

//	Nelno:	dll stuff
void	DLL_InitFunctions(void);		//	called once upon DLL load
void	DLL_LoadLevelFunctions (void);	//	called every each level load
void	DLL_ExitLevelFunctions (void);	//	called on each level exit
void	DLL_ExitFunctions (void);		//	called before unloading DLLs
void	DLL_LoadDLLs (char *dirName);	//	called to load DLLs
void	DLL_UnloadDLLs (void);			//	called to unload DLLs

///////////////////////////////////////////////////////////////////////////////
//	dk_ServerState.c
///////////////////////////////////////////////////////////////////////////////

void InitServerState(game_import_t *import);

///////////////////////////////////////////////////////////////////////////////
//	p_utils.c
///////////////////////////////////////////////////////////////////////////////

void		P_InitEdict (edict_t *e);
edict_t		*P_SpawnEdict (void);
edict_t		*P_EntPtr (int entnum);
int			P_EntNum (edict_t *ent);
void		P_FreeEdict (edict_t *e);
void		P_SaveEdict (FILE* f, edict_t* self);
void		P_LoadEdict (FILE* f, edict_t* self);

void		P_TouchTriggers (edict_t *ent);

userEntity_t	*P_FirstEntity(void);
userEntity_t	*P_NextEntity(edict_t *ent);

userEntity_t	*P_SpawnFakeEntity (void);
void			P_RemoveFakeEntity (userEntity_t *fake);

void	P_InitFuncList();
char*	P_GetFuncString(void* func);
void*	P_GetStringFunc(char *string);
void	P_RegisterFunc(char* name, void* func);

///////////////////////////////////////////////////////////////////////////////
//	p_phys.c
///////////////////////////////////////////////////////////////////////////////

int		P_Push (edict_t *pusher, CVector &move, CVector &amove);
void	P_RunEntity (edict_t *ent, int run_think);
void	P_CheckGround (edict_t *ent);
qboolean P_RunThink (edict_t *ent, int run_think);
void	P_MomentumPush (edict_t *e1, edict_t *e2, CVector &push_velocity);
trace_t P_PushEntity (edict_t *ent, CVector &push);
int		P_FlyMove (edict_t *ent, float time, int mask);

///////////////////////////////////////////////////////////////////////////////
//	p_save.c
///////////////////////////////////////////////////////////////////////////////

void	P_WriteGame (char *filename, char autosave);
void	P_ReadGame (char *filename);
void	P_WriteLevel (char *filename);
void	P_ReadLevel (char *filename);
void	P_WriteHeader( char *filename, char *comment, qboolean autoSave );

///////////////////////////////////////////////////////////////////////////////
//	p_main.c
///////////////////////////////////////////////////////////////////////////////

void P_RunFrame (void);


void	P_SetStats(edict_t *ent);

///////////////////////////////////////////////////////////////////////////////
//	p_float.c
///////////////////////////////////////////////////////////////////////////////

void	P_PercentSubmerged (userEntity_t *self, int *contents, float *percent_submerged);
float	P_PhysicsFloat (edict_t *self);

///////////////////////////////////////////////////////////////////////////////
//	com_main.cpp
///////////////////////////////////////////////////////////////////////////////

void	com_ServerLoad (void);
void	com_LevelLoad (void);
void 	com_LevelExit (void);
void 	com_ServerKill (void);

void	com_InitFriction	(void);
void	com_InitHeap		(void);
void	com_InitList		(void);
void	com_InitMain		(void);
void	com_InitSub			(void);
void	com_InitWeapons		(void);

///////////////////////////////////////////////////////////////////////////////
//	defined in com_heap.cpp
///////////////////////////////////////////////////////////////////////////////

extern	listHeader_t	*malloc_list;
extern	unsigned long	malloc_count;

void	*com_malloc(unsigned long size, mem_type type);
void	com_free (void *ptr);
void	com_free_most (void);
void	com_free_type (int mem_type);
void	com_free_all (void);
void	*com_malloc_hook (unsigned long size, mem_type type);

///////////////////////////////////////////////////////////////////////////////
//	defined in com_sub.cpp
///////////////////////////////////////////////////////////////////////////////

extern	common_export_t	com;
extern	CVector			forward, right, up;
extern	CVector			zero_vector;


// SCG[10/21/99]: 
void SaveClientData();

#endif
