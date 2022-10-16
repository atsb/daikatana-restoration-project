#ifndef _WORLD_H
#define _WORLD_H

#include "p_user.h"
#include "hooks.h"
#include "common.h"
#include "memmgrcommon.h"
#include "ai_save.h"	
#include "l__language.h"

//	for files that don't include windows.h
#ifndef	TRUE
#define	TRUE	1
#endif
#ifndef	FALSE
#define	FALSE	0
#endif

#define		DEMO_MODE	1

///////////////////////////////////////////////////////////////////////////////
//	enums
///////////////////////////////////////////////////////////////////////////////


extern	char	*gib_models [];

enum
{

	//GIB_TORSO,
	//GIB_SHOULDER,
	//GIB_LEG,
	//GIB_ARM,
	//GIB_CHUNK,
    
    GIB_TORSO,
    GIB_LEG,
    GIB_FOOT,
    GIB_HAND,
    GIB_HEAD,
    GIB_CHEST,
    GIB_EYE,
    GIB_ARM,
    GIB_MISC,

	GIB_WOOD1,
	GIB_WOOD2,
	GIB_GLASS1,
	GIB_GLASS2,
	GIB_METAL1,
	GIB_METAL2,
	GIB_ROBOT1,
	GIB_ROBOT2,
	GIB_ROBOT3,
	GIB_ROBOT4,
	GIB_ROCK1,
	GIB_ROCK2,
	GIB_ROCK3,
	GIB_NUM_MODELS
};

#ifdef _DEBUG
extern	char	*debug_models [];
#endif

enum
{
	DEBUG_CURNODE,
	DEBUG_LASTNODE,
	DEBUG_NODE,
	DEBUG_MARKER,
	DEBUG_JUMPTARG,
	DEBUG_GOAL,
	DEBUG_ARROW,
	DEBUG_PATHNODE,
	DEBUG_NUM_MODELS
};

typedef enum 
{
	DF_NONE		= 0x00,
	DF_ACCEL	= 0x01,
	DF_DUST		= 0x02,
	DF_BOUNCE	= 0x04,
	DF_ROCKGIBS	= 0x08,
	DF_WOODGIBS	= 0x10,
	DF_METALGIBS= 0x20,
	DF_QUAKE	= 0x40
} DOOR_FLAG;

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define	THINK_TIME	0.1

#define	MAX_DECO_ANIM_SEQS	5		// maximum number of deco animation sequences

#define	MAX_GIBS   12           //  maximum gib pieces

#define	PLAYER_GIB_POINT	50

#define	BODY_GIB_POINT		40

///////////////////////////////////////////////////////////////////////////////
//	externs
///////////////////////////////////////////////////////////////////////////////

extern	CVector			forward, right, up;
extern	CVector			zero_vector;
extern	userTrace_t		trace;
extern	trace_t			tr;
extern	serverState_t	*gstate;
extern	common_export_t	*com;
extern  common_export_t *memory_com;

extern	cvar_t	*maxclients;
extern	cvar_t	*p_gravity;
extern	cvar_t	*deathmatch;
extern	cvar_t	*coop;
extern	cvar_t	*skill;
extern	cvar_t	*ai_debug;
extern	cvar_t	*sv_episode;
extern	cvar_t	*sv_violence;
extern	cvar_t  *sv_sidekickweapons;
extern	cvar_t  *sv_aiwander;
extern	cvar_t	*sv_demomode;
extern  cvar_t	*sv_frictionfactor;
extern	cvar_t	*ai_scriptedit;
extern	cvar_t	*ai_scriptrunning;

extern  cvar_t  *allow_friendly_fire;

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
extern	cvar_t	*dm_footsteps;
extern  cvar_t  *dm_allow_hook;
extern  cvar_t  *dm_item_respawn;
extern	cvar_t	*dm_instagib;
extern	cvar_t	*unlimited_saves;

extern  cvar_t  *gib_damage_point;
extern	cvar_t	*gib_enable;

extern cvar_t  *modelname;
extern cvar_t  *skinname;
extern cvar_t  *skincolor;
extern cvar_t  *character;
extern cvar_t  *p_cheats;
extern cvar_t *cvarBuildPathTable;

extern cvar_t *sv_cinematics;

extern cvar_t	*kickview;

///////////////////////////////////////////////////////////////////////////////
//	prototypes
///////////////////////////////////////////////////////////////////////////////

float debris_vel(userEntity_t *self, float distxy, float distz, float up_vel);
void  Kill(userEntity_t *self);

///////////////////////////////////////////////////////////////////////////////
//	typedefs

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
// this structure is appended to door entities with userHook
typedef struct
{
	int		state;
	int		count;
	int		max_count;
	int		already_linked;
	int		continuous_mode;

	float	wait;
	float	base_health;
	float	speed;
	float	distance;
	float	move_finished;
	float	damage;
	float	ignore_time;

	// for func_trains that rotate
	float	x_speed;
	float	y_speed;
	float	z_speed;
	float	x_distance;
	float	y_distance;
	float	z_distance;
	float	end_x_rotation_time;
	float	end_y_rotation_time;
	float	end_z_rotation_time;
	float	end_x_angle;
	float	end_y_angle;
	float	end_z_angle;
	float	end_move_time;

	// for secrets only
	float	t_width;
	float	t_length;
	float	lip;			//&&& AMW 6.26.98 - record the lip on the button/door

	CVector			pos1;
	CVector			pos2;
	CVector			final_dest;
	CVector			size;
	CVector			old_origin;
	CVector			mangle;
	CVector			pos1_offs;		// default offsets of pos1 and pos2 from the entities origin
	CVector			pos2_offs;
	CVector			destangle;		//&&& AMW 7.8.98 - for accelerated doors
	CVector			velocityInc;	//&&& AMW 7.9.98 - for accelerated doors

	void	(*done_think)(userEntity_t *self);
	//void	(*die_think)(userEntity_t *self);
	//void	(*use_think)(userEntity_t *self);

	userEntity_t	*linked_door;
	userEntity_t	*trigger_field;
//	userEntity_t 	*movetarget;
	userEntity_t	*activator;
	userEntity_t	*portal;

	char			*nodetarget;	// a button might have a node target for a func_nodetrain
	char			*pAiScript;		// a button can have an ai script to play when pressed
	char			*pCineScript;	// a button can have a cinescript to play when pressed
					
	int				sound_top;
	int				sound_bottom;
	int				sound_up;
	int				sound_down;
					
	int				bitflags;		// various bitflags for door/button/train behavior
	int				steps;			// # of steps for acceleration
	int				bounceSteps;	// # of bounces remaining
					
	int				no_more_move;	//	TRUE if hook->wait == -1

	qboolean		bStruggle;
	char			*misctarget;	
} doorHook_t;

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

// this structure is appended to hosportal entities
typedef struct
{
	byte			style;          // 0 = large, 1 = medium, 2 = small
	int				max_juice;      // maximum juice available for suckdown
	int				current_juice;  // amount of juice remaining
	float			fx_start_time;
	userEntity_t	*activator;  // pointer to player
	CVector			vOrigin;
	int				sound_use;        // sound index of activation wav
	int				sound_use_done;   // sound index of depleted hosportal wav
	int				sound_particles;  // sound index of particle fx
	int				sound_healthup;   // sound index of health give 
	int				sound_use_out;    // sound index of activation of depleted hosportal
	int				sound_recharged;  // sound index of recharged hosportal

	think_t			use_fx;
} hosportalHook_t;

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

// this structure is appended to healthtree entities
typedef struct
{
    int   max_fruit;      // maximum fruit available for suckdown
    int   current_fruit;  // amount of fruit remaining
    int   fruit_value;    // health value per fruit

    float recharge_rate;  // rate at which to recharge
    
    float ignore_rate;    // ignore delay
    float ignore_timer;   // ignore timer

	int sound_use1;       // sound index of activation wav 1
    int sound_use2;       // sound index of activation wav 2
    int sound_regen;      // sound index of fruit regen wav
	
} healthtreeHook_t;

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

// func_nodetrain hook

typedef struct nodeTrainHook_s
{
	doorHook_t	dh;			// for compatibility with movement functions

	/////////////////////////////////////////////
	// node system stuff
	/////////////////////////////////////////////

	MAPNODE_PTR 	pCurNode;			// pointer to the node this train is closest to
	MAPNODE_PTR 	pTargetNode;		// pointer to the node this train is heading to
	PATHNODE_PTR	path;				// the current path that the bot is following, NULL when no path
	PATHNODE_PTR	path_end;			// the last node in the current path
	int				path_length;		// the length of the path in nodes

} nodeTrainHook_t;

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

// func_talking_head hook
/*
typedef struct talkingHeadHook_s
{
	int				waveIndex[4];		// wave index (4 max)
	int				lastIndex;			// last index played
	bool			bRandom;			// select a random index to play
	bool			bPlaying;			// currently playing a wave file
	long			startTime;			// time that wave started playing
	float			frames;				// number of frames to play

} talkingHeadHook_t;
*/
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

// effect_lightning hook
typedef struct lightningEffectHook_s
{
	userEntity_t	*attractorHead;		// first attractor in the list (attractors contain other links)
	userEntity_t	*curAttractor;		// current attractor.. next is curAttractor->userhook->attractorLink
	CVector			color;				// Color of the lightning.
	int				totalAttractors;	// total number of attractors linked to emitter
	int				sound[3];			// sound to play when lightning strikes	
	float			delay;				// time between strikes (default 2.0)
	float			duration;			// duration of lightning strikes (default 0.5)
	float			dmg;				// damage if a PLAYER gets hit (default 0.0)
	float			scale;				// scale of the lightning model (default 4.0)
	float			chance;				// pct chance a player will be hit (default 0.10)"},
	float			gndchance;			// if GROUND STRIKES is checked, chance for ground hit (default 0.20)
	float			modulation;			// % of modulation lightning will have applied to it.  Scalar value.
} lightningEffectHook_t;

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

// effect_steam hook
typedef struct steamEffectHook_s
{
	int				sound;			    // sound index
    CVector         vec;                // vector distance/direction
	float			delay;				// time between jets
	float			duration;			// duration of steam jet
	float			dmg;				// damage if a PLAYER gets hit
	float			scale;				// scale of the particle model
    int             flags;              // bit flags
} steamEffectHook_t;

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

// target_attractor hook
typedef struct attractorHook_s
{
	userEntity_t	*emitter;			// link back to the emitter
	userEntity_t	*prevAttractor;		// previous attractor in the list (first is NULL)
	userEntity_t	*nextAttractor;		// next attractor in the list (last is NULL)
	int				sound;				// sound to play when lightning strikes
	int				triggerIndex;		// priority index.. 0 is first.. etc etc
} attractorHook_t;

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

// effect entity hook
typedef struct effectEntityHook_s
{
	userEntity_t	*emitter;			// link back to the emitter
	userEntity_t	*dstEnt;			// destination entity
	CVector			dstPos;				// destination position
	float			killtime;			// time for the bolt to cease
	float			dmg;				// damage done by effect
	qboolean		bCheckTraceDamage;	// check the trace damage
} effectEntityHook_t;

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

// target_monster_spawn hook
typedef struct monsterSpawnHook_s
{
	int				sound;
	char			*monsterClass;
	char			*aiStateStr;
} monsterSpawnHook_t;

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

// func_floater hook
/*
typedef struct
{
	int				water_level;
	char			*frameName;
	userEntity_t	*last_touch;
	float			speed;
} floatHook_t;
*/
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

// func_debris hook
typedef struct
{
	char	*modelName;
	char	*fly_sound;
	char	*hit_sound;
	int		has_rotated;
	float	damage;
	float	last_water;
	float	change_time;
	float	rotation_scale;
	float	mass_scale;
	float   floor_distance;
	CVector	last_pos;
	CVector	hit_water_pos;
	CVector	dest;
	CVector	avel;
} debrisHook_t;

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

// trigger hook
typedef struct
{
	float			wait;
	float			base_health;
	float			next_trigger_time;
	float			damage;
	float			speed;

	int				bEnabled;				// allows a trigger_hurt to be turned on and off
	int				count;

	int				triggerSound;
	char			*console_msg;

	char			*model_1;
	char			*model_2;
	char			*model_3;

	userEntity_t	*activator;

	CVector			mangle;
	CVector			v_angle;

	char			*pCineScript;
	char			*pAiScript;

	void	(*done_think)(userEntity_t *self);

	char			*mp3file;		// SCG[11/24/99]: John wants mp3 files
} trigHook_t;

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

// rock hook (func_wall_explode random debris)
typedef struct
{
	float	remove_time;
	float	last_touch_time;
	float	last_water;
	float	change_time;

	float	rotation_scale;
	float	mass_scale;

	CVector	last_pos;
	CVector	hit_water_pos;
} rockHook_t;

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

// fog trigger hook
typedef struct
{
	float	cur_fog_value;
} fogHook_t;

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

// particlefield hook
typedef struct
{
	CVector	dest;
	CVector	dest1;
	CVector	dest2;
	CVector	impact_vel;
	char	*touch_sound;
	char	*trigger_sound;
	char	*amb_sound;
	float	damage;
	float	attack_finished;
	float	hold_time;
	int		count;
	CVector	color;
	CVector	last_color;
	userEntity_t	*other;
	think_t			think_use;
} particleHook_t;

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

// func_timer hook
typedef struct
{
	float	wait;
	float	random;
	float	delay;
	float	pausetime;
	userEntity_t *other;
	userEntity_t *activator;
} timerHook_t;

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

// target_lightramp
typedef struct
{
	float	timestamp;
	float	speed;
	char	*message;
} lightrampHook_t;

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

typedef	struct	lavaballHook_s
{
	CVector			targ_origin;

	float			time_min;
	float			time_rng;

	float			z_min;
	float			z_rng;

	float			damage;
} lavaballHook_t;

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

//<nss> Modified for better debris functionality
typedef	struct funcExploHook_s
{
	int				damage;			//Amount of dammage for explosion
	int				count;			//quantity of chunks to blow out
	int				rndcount;		//random quantity of chunks to blow out plus count
	int				Sounds[5];		//Sound indexes
	float			gravity;		//how much gravity to apply to chunks	
	float			scale;			//how big chunks should be
	float			speed;			//how fast chunks should fly
	float			personality;	//random value generated for debris personality
	float			alpha;			//Alpha level for debris
	char			*pCineScript;	//CineScript pointer
	char			*pAiScript;		//AIScript pointer
	userEntity_t	*target;			//Direction debris should go towards
	userEntity_t	*nextTarget;	//Next Target that should be triggered
} funcExploHook_t;

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

// used for animated deco's
typedef struct tagANIMSEQ
{
	short start;
	short end;
	unsigned short flags;
} ANIMSEQ, *LPANIMSEQ;


typedef	struct decoHook_s
{
	int				damage;
	userEntity_t	*inflictor;
	userEntity_t	*attacker;
	int				damage_done;
	CVector			point;
	int				explo_count;
	int				explo_max;
	int				frame;						// current frame
	int				animseq;					// current animation sequence
	ANIMSEQ			seq[MAX_DECO_ANIM_SEQS];	// kinda big... but necessary for speed
} decoHook_t;

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

typedef struct effectHook_s
{
	int				spawn_type;					// generic particle type.. TE_PARTICLE_RISE, TE_PARTICLE_FALL, TE_PARTICLE_FLOAT
	CVector			dir;						// direction for particles to go
	float			speed;						// speed of particles
//	int				color;						// color of particles
	CVector			color;
	int				count;						// number of particles to spawn per frame
	int				particle_type;				// type of particles (PARTICLE_SIMPLE thru PARTICLE_ICE)
	float			length;						// total time in seconds for effect to last
	float			frametime;					// time between spawn thinks
	float			endtime;					// calculated field - the last time the effect should be displayed
	char			*sound;						// sound to play when particles are spawned
} effectHook_t;

typedef	struct	itemInfo_s
{
	short	respawnFlags;
	int		health;
	int		respawnTime;
	int		frame_start;
	int		frame_end;
	int		netNameID;

	float	frame_time;
	float	render_scale;

	unsigned	long	flags;

	char	*className;
	char	*netName;
	char	*episodeIndependentModelName;

	char	*soundPickup;
	char	*soundRespawn;
	char	*soundAmbient;
	char	*soundSpecial;

	float	soundAmbientVolume;
	float	soundDist_min;
	float	soundDist_max;

	touch_t	touchFunc;

	
	CVector	mins;
	CVector	maxs;

} itemInfo_t;

typedef	struct	itemHook_s
{
	float		  respawnTime;
	float		  health;
    userEntity_t  *link;

	itemInfo_t	*info;	

	int			soundPickupIndex;
	int			soundRespawnIndex;
	int			soundSpecialIndex;

	//	store some flags here that define who can pick this item up
	unsigned long	flags;
} itemHook_t;

// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////
//
// SCG[11/19/99]: ///////////////////////////////////////////////////////////////////////////////

void	ParseAnimationSequences(char *str, LPANIMSEQ lpSeq);

typedef enum deathtype
{
	NONE,
	FELL,
	DROWNED,
	LAVA,
	SLIME,
	POISON,
	KILLBRUSH,
	TELEFRAG,
	CRUSHED


} DEATHTYPE;

extern	DEATHTYPE client_deathtype;


void AlertClientOfSidekickDeath(byte gibbed);


// artifacts.cpp
extern void item_savegem_use(userEntity_t *self);

// gib.cpp
extern void AI_StartGibFest(userEntity_t *self, userEntity_t *attacker);
extern userEntity_t * gib_client_die(userEntity_t *self, userEntity_t *Attacker);
extern userEntity_t	* ai_throw_gib(userEntity_t *self, char *model, CVector &offset, CVector Dir, byte gib_type = GIB_MISC);
extern bool check_for_gib(userEntity_t *self, int damage);
extern void ai_gib_sound(userEntity_t *self);

extern void recalc_level(userEntity_t* self);
extern void award_exps(userEntity_t *self,userEntity_t *other, int manual_bonus);
extern void xplevelup(userEntity_t *self);
extern int GetArmorPercentage(int armor_val, int item_flags);
extern userEntity_t * findEntityFromName(char *name);

void DecrementMonsterCount();
void IncrementMonsterCount();

// player spawn flags for default weapon selection
#define PLAYER_SF_OVERRIDE_WEAPONS		0x0001
#define PLAYER_SF_WEAPON_1				0x0002
#define PLAYER_SF_WEAPON_2				0x0004
#define PLAYER_SF_WEAPON_3				0x0008
#define PLAYER_SF_WEAPON_4				0x0010
#define PLAYER_SF_WEAPON_5				0x0020
#define PLAYER_SF_WEAPON_6				0x0040
#define PLAYER_SF_WEAPON_7				0x0080

extern int SinglePlayerCheat();

#endif