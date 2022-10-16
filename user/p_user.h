
#ifndef _GUSER_H
#define _GUSER_H

#if _MSC_VER
#pragma warning (disable : 4200 4244)
#endif

#include <stdarg.h>
#include <math.h>

#include "dk_shared.h"
#include "qfiles.h"

class CMapStringToPtr;
//#include "collect.h"

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible gclient_t and edict_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "physics.h"

//unix - avoid conflict with standard type use_t
#if !_MSC_VER
#undef use_t
#endif

// #include "ClientEntityManager.h"
typedef float vec_t;
//typedef float vec3_t[3];

#define	TRUE	1
#define	FALSE	0

// edict->movetype values
typedef enum
{
	MOVETYPE_NONE,			// never moves
	MOVETYPE_NOCLIP,		// origin and angles change with no interaction
	MOVETYPE_PUSH,			// no clip to world, push on box contact
	MOVETYPE_STOP,			// no clip to world, stops on box contact

	MOVETYPE_WALK,			// gravity
	MOVETYPE_STEP,			// gravity, special edge handling
	MOVETYPE_FLY,
	MOVETYPE_TOSS,			// gravity
	MOVETYPE_FLYMISSILE,	// extra size to monsters
	MOVETYPE_BOUNCE,
	MOVETYPE_FLOAT,			//	acts like MOVETYPE_BOUNCE until it hits a liquid, then mass/density takes over
	MOVETYPE_BOUNCEMISSILE,
	MOVETYPE_SWIM,			// water creatures
	MOVETYPE_WHEEL,			// creatures on wheel or on treads
	MOVETYPE_HOVER,			// flying creatures that can hover
	MOVETYPE_HOP,			// hopping creatures such as froginator
	MOVETYPE_TRACK,			// moving along a track such as a train or track attack
} movetype_t;										// if not in water, then acts like movetype_toss

// SCG[11/19/99]: 
typedef enum {
	F_INT, 
	F_BYTE, 
	F_SHORT, 
	F_FLOAT,
	F_LSTRING,			// string on disk, pointer in memory, TAG_LEVEL
	F_GSTRING,			// string on disk, pointer in memory, TAG_GAME
	F_VECTOR,
	F_ANGLEHACK,
	F_EDICT,			// index on disk, pointer in memory
	F_ITEM,				// index on disk, pointer in memory
	F_CLIENT,			// index on disk, pointer in memory

	F_FUNC,
	F_PRECORD,
	F_EPAIR,
	F_DATA,

	F_RESTORE_AS_NULL,

	F_IGNORE
} fieldtype_t;

typedef struct
{
	char	*name;
	int		ofs;
	fieldtype_t	type;
//	int		flags;	// SCG[1/23/00]: not used
} field_t;

#define CLIENT_ADDITIVE_VELOCITY	// the speed for client and bots are increased when both 
									// forward and sidestep are held at the same time


// edict->deadflag values

#define	DEAD_NO					0	//The player is still a happy little camper
#define	DEAD_DYING				1	//The player is in the middle of dying
#define	DEAD_DEAD				2	//The player is completely dead(has been gibbed)
#define	DEAD_FROZEN				3	//The player is frozen
#define DEAD_PUNK				4	//The player is dead but has not been gibbed
#define DEAD_SIDEKICK			5	//The sidekicks have died you need to die or something

#define	DAMAGE_NO				0
#define	DAMAGE_YES				1
#define	DAMAGE_AIM				2

//	edict->flags
//	Q2FIXME:	remove useless flags
#define	FL_FORCEANGLES			0x00000001	//	force the client to look a certain direction
#define	FL_FIXANGLES			0x00000002	//	force the client angles to change on next frame,
											//	cleared each time a frame is run for the client
#define	FL_FAKEENTITY			0x00000004	//	not a real server entity - malloced up as a temporary goal
#define	FL_CLIENT				0x00000008
#define	FL_NOPUSH 				0x00000010	//	entity cannot push another entity
#define	FL_MONSTER				0x00000020
#define	FL_SWIM   				0x00000040
#define	FL_NOTARGET				0x00000080
#define	FL_ITEM					0x00000100
#define	FL_FORCEMOVE			0x00000200	//  do not let other entities prevent this entity from moving
#define	FL_IGNORE_ARMOR			0x00000400	//	not all corners are valid
#define	FL_WATERJUMP			0x00000800	//	player jumping out of water
// SCG[12/7/99]: #define	FL_CLUSTERBASED			0x00001000	//	for cluster based hierarchical animation
#define	FL_CINEMATIC			0x00001000
#define	FL_BOT					0x00002000
#define FL_EXPLOSIVE			0x00004000
#define	FL_ATTACKHOLD    		0x00008000
#define	FL_HCHILD				0x00010000	//	a hierarchial child entity
#define	FL_HPARENT	 			0x00020000	//	a hierarchial parent entity
#define	FL_BODY					0x00040000
#define	FL_INWARP				0x00080000
#define	FL_CAMERAMODE			0x00100000
#define	FL_DEBUG				0x00200000
#define	FL_IGNOREUSETARGET		0x00400000
#define	FL_NORESPAWN			0x00800000	//	item will not respawn if set
#define	FL_POSSESSED			0x01000000
#define FL_PUSHABLE				0x02000000	//	can be pushed
#define	FL_LEDGEDROP			0x04000000	//	monster/bot can fall off of ledges
#define	FL_TEAMSLAVE			0x08000000	//	this is a linked entity to a parent
#define FL_LOOPAMBIENT_ANIM		0x10000000  //	constantly play weapon ambient
#define FL_WARPEDVIEW			0x20000000  //	psyclaw's weird warp view
#define FL_FREEZE				0x40000000  // used to set pmove.pm_type
#define FL_NOSAVE				0x80000000  //	be free little flag -- cek[11/17/99] not anymore!  this entity will not be saved (used mainly for the player weapon)

// Item flags

#define	IT_PLASTEEL			0x00000001
#define	IT_CHROMATIC		0x00000002
#define	IT_SILVER			0x00000004
#define	IT_GOLD				0x00000008
#define	IT_CHAINMAIL		0x00000010
#define	IT_BLACK_ADAMANT	0x00000020
#define	IT_KEVLAR			0x00000040
#define	IT_EBONITE			0x00000080
#define	IT_WRAITHORB		0x00000100
#define IT_MEGASHIELD		0x00000200
#define IT_GOLDENSOUL		0x00000400
#define IT_ANTIDOTE			0x00000800
#define IT_POWERBOOST		0x00001000
#define	IT_ATTACKBOOST		0x00002000
#define	IT_SPEEDBOOST		0x00004000
#define	IT_ACROBOOST		0x00008000
#define	IT_VITABOOST		0x00010000
#define	IT_JET_BOOTS		0x00020000
#define	IT_OXYLUNG			0x00040000
#define	IT_ICE_BOOTS		0x00080000
#define	IT_ENVIROSUIT		0x00100000
#define	IT_POISON    		0x00200000
#define IT_WOOD_CHEST		0x00400000
#define IT_BLACK_CHEST		0x00800000
#define IT_MANASKULL		0x01000000
#define IT_RINGOFIRE		0x02000000  //NSS[11/24/99]:Added all 3 rings
#define IT_RINGOFUNDEAD		0x04000000
#define IT_RINGOFLIGHTNING	0x08000000


typedef enum 
{
  ICON_NONE,

  ICON_POWERBOOST,
  ICON_ATTACKBOOST,
  ICON_SPEEDBOOST,
  ICON_ACROBOOST,
  ICON_VITABOOST

} onscreen_icon_enums;

typedef enum
{
	INV_MODE_WEAPONS,
	INV_MODE_ITEMS,
	INV_MODE_SIDEKICK1,
	INV_MODE_SIDEKICK2,
	INV_MODE_SKILLS
};


#define BOOSTICON_ADD    0
#define BOOSTICON_DELETE 1
#define BOOSTICON_BLINK  2


#define INVMODE_DISPLAYTIME	(6000)  // # of seconds * 1000


#define	SPAWNFLAG_NOT_EASY			0x00001000
#define	SPAWNFLAG_NOT_MEDIUM		0x00002000
#define	SPAWNFLAG_NOT_HARD			0x00004000
#define	SPAWNFLAG_NOT_DEATHMATCH	0x00008000


#define FRAGTYPE_NOBLOOD			0x00000001   // 1.5  dsn    if this entity leaves blood
#define FRAGTYPE_ROBOTIC			0x00000002   //             robotic gib, versus fleshy gib
#define FRAGTYPE_BONE				0x00000004   //				Bone Gib makes Bone Noises.
#define FRAGTYPE_SPECIALGIBTEXTURE	0x00000008   //             special texture used for gib model
#define FRAGTYPE_ALWAYSGIB			0x00000010   // 5.9         always gib
#define FRAGTYPE_NEVERGIB			0x00000020   //             never ever gib
// SCG[11/8/99]: Added for those nutty Germans!
#ifdef TOUNGE_GERMAN
#define FRAGTYPE_HUMANOID			FRAGTYPE_ROBOTIC
#else
#define FRAGTYPE_HUMANOID			0x00000020
#endif

#define DROPPED_ITEM			0x00008000
#define	DROPPED_PLAYER_ITEM		0x00010000

// sound channels
// channel 0 never willingly overrides
// other channels (1-7) allways override a playing sound on that channel

#define	CHAN_AUTO		0
#define	CHAN_WEAPON		1
#define	CHAN_VOICE		2
#define	CHAN_ITEM		3
#define	CHAN_BODY		4
#define  CHAN_WEAPON2   5

// modifier flags
#define	CHAN_NO_PHS_ADD	8	// send to all clients, not just ones in PHS (ATTN 0 will also do this)
#define	CHAN_RELIABLE	16	// send by reliable message, not datagram
#define	CHAN_OVERRIDE	CHAN_NO_PHS_ADD + CHAN_WEAPON	//	mother fucking shit fucking asshole shit fuck damn fuckers ARGH!!!!

#define	ATTN_NONE		 0
#define	ATTN_NORM		 1
#define	ATTN_IDLE		 2
#define	ATTN_STATIC		 3

// set when an item is picked up, cleared once its expire message is shown
#define	EXP_WRAITHORB	0x00000001
#define	EXP_MEGASHIELD	0x00000002
#define	EXP_ENVIROSUIT	0x00000004
#define EXP_POWERBOOST	0x00000008
#define EXP_SPEEDBOOST	0x00000010
#define EXP_ATTACKBOOST	0x00000020
#define EXP_ACROBOOST	0x00000040
#define EXP_VITABOOST	0x00000080
#define EXP_OXYLUNG		0x00000100
#define EXP_JETBOOTS	0x00000200
#define	EXP_POISON		0x00000400
#define EXP_MANASKULL	0x00000800

// door/plat/button states
#define STATE_TOP     0         // door is at top
#define STATE_BOTTOM  1         // door is at bottom
#define STATE_DOWN    2         // door is going down
#define STATE_UP      3         // door is going up

// base attribute levels used to calculate attribute modifications based on boost levels
#define	ATTR_JUMPSPEED		190.0
#define	ATTR_RUNSPEED		320.0
#define	ATTR_ATTACKMULT		1.0
#define	ATTR_HEALTH			100.0
#define	ATTR_POWERMULT		1.0
#define	ATTR_MAX_LEVELS		5

//	camera flags for SetCameraState
#define	CAMFL_LOCK_X		0x00000001
#define	CAMFL_LOCK_Y		0x00000002
#define	CAMFL_LOCK_Z		0x00000004
#define CAMFL_NOLERP		0x00000008 //NSS[10/26/99]:Added flag for no lerping

// frames
typedef struct
{
	int    frameIndex;    // frame number
	int    frameNext;     // next frame to goto
	char  *frameName;
} userFrame_t;

typedef struct
{
	int   frameIndex;
	int   frameNext;
	int   frameTrans;
} followFrame_t;






///////////////////////////////////////////////////////////////////////////
/// Frame Info
///////////////////////////////////////////////////////////////////////////

// model info

// userFrameInfo flags
#define MI_MODELALIAS  0x00000001
#define MI_MODELLW     0x00000002


typedef struct
{
	char           modelName[50];
	unsigned long  flags;
	int            numframes;
	int            numsurfs;
} userModelInfo_t;





// frame info
typedef struct
{
	char          frameName[50];
	unsigned long flags;

	// unsigned char bboxmin[3];
	// unsigned char bboxmax[3];
} userFrameInfo_t;

////////////////////////////////////////////////////////////////////////
//	used for trace function
////////////////////////////////////////////////////////////////////////

typedef struct
{
	int			allsolid;	        // if true, plane is not valid
	int			startsolid;        // if true, the initial point was in a solid area
	int			inopen;
	int			inwater;
	float		fraction;		    // time completed, 1.0 = didn't hit anything
	CVector		endpos;			// final position
	CVector		planeNormal;

	struct	edict_s		*ent;         // entity the surface is on
} userTrace_t;


///////////////////////////////////////////////////////////////////////////////
//	Stuff from Quake 2's g_local.h
///////////////////////////////////////////////////////////////////////////////


//
// this structure is cleared as each map is entered
// it is read/written to the level.sav file for savegames
//
typedef struct
{
	int			framenum;
	float		time;

	char		level_name[MAX_QPATH];	// the descriptive name (Outer Base, etc)
	char		mapname[MAX_QPATH];		// the server name (base1, etc)
	char		nextMap[MAX_QPATH];		// go here when fraglimit is hit

	// intermission state
	float		intermissiontime;		// time the intermission was started
//	char		*changemap;
	int			exitintermission;
	CVector		intermission_origin;
	CVector		intermission_angle;

	int			players;		// FIXME: count when needed, don't store

	edict_t		*sight_client;	// changed once each frame for coop games

	edict_t		*sight_entity;
	int			sight_entity_framenum;
	edict_t		*sound_entity;
	int			sound_entity_framenum;
	edict_t		*sound2_entity;
	int			sound2_entity_framenum;

	int			pic_health;

	int			total_secrets;
	int			found_secrets;

	int			total_goals;
	int			found_goals;

	int			total_monsters;
	int			killed_monsters;

	edict_t		*current_entity;	// entity running from G_RunFrame
	edict_t		*body_que;			// looped chain of dead bodies
} level_locals_t;


// spawn_temp_t is only used to hold entity field values that
// can be set from the editor, but aren't actualy present
// in edict_t during gameplay
typedef struct
{
	// world vars
	char		*sky;
	float		skyrotate;
	CVector		skyaxis;
	char		*nextmap;

	int			lip;
	int			distance;
	int			height;
	char		*noise;
	float		pausetime;
	char		*item;
	char		*gravity;

	float		minyaw;
	float		maxyaw;
	float		minpitch;
	float		maxpitch;
} spawn_temp_t;

typedef struct 
{
  char  modelname[MAX_QPATH]; // model name         2.18.99
  char  skinname[MAX_QPATH];  // skin name
  int   skincolor;            // skin color
  int   character;            // character model (0=Hiro, 1=Mikiko, 2=Superfly)
} body_info_t;

// SCG[11/17/99]: Begin savegame flags

#define PERS_WEAP_WEAPON1			0x00000001
#define PERS_WEAP_WEAPON2			0x00000002
#define PERS_WEAP_WEAPON3			0x00000004
#define PERS_WEAP_WEAPON4			0x00000008
#define PERS_WEAP_WEAPON5			0x00000010
#define PERS_WEAP_WEAPON6			0x00000020
#define PERS_WEAP_WEAPON7			0x00000040	// SCG[11/5/99]: always the daikatana
#define PERS_WEAP_WEAPON8			0x00000080	// SCG[11/5/99]: gashands

	// SCG[11/17/99]: Global
#define ITEM_MEGASHIELD				0x00000001
#define ITEM_INVINCIBILITY			0x00000002
// SCG[1/28/00]: cheeseball!
#define ITEM_SAVEGEM_1				0x00000004
#define ITEM_SAVEGEM_2				0x00000008
#define ITEM_SAVEGEM_3				0x00000010
#define ITEM_CONTROL_CARD_RED		0x00000020
#define ITEM_CONTROL_CARD_BLUE		0x00000040
									
	// SCG[11/17/99]: Episode 1

	// SCG[11/17/99]: Episode 2
#define ITEM_ANTIDOTE				0x00000100
#define ITEM_DRACHMA				0x00000200
#define ITEM_HORN					0x00000400
#define ITEM_RUNE_A					0x00000800
#define ITEM_RUNE_E					0x00001000
#define ITEM_RUNE_G					0x00002000
#define ITEM_RUNE_I					0x00004000
#define ITEM_RUNE_S					0x00008000
									
	// SCG[11/17/99]: Episode 3
#define ITEM_WYNDRAX_KEY			0x00000100
#define ITEM_SPELLBOOK				0x00000200
#define ITEM_CRYPT_KEY				0x00000400
#define ITEM_PURIFIER_SHARD1		0x00000800
#define ITEM_PURIFIER_SHARD2		0x00001000
#define ITEM_PURIFIER_SHARD3		0x00002000
#define ITEM_HEX_KEYSTONE			0x00004000
#define ITEM_QUAD_KEYSTONE			0x00008000
#define ITEM_TRIGON_KEYSTONE		0x00010000
#define ITEM_RING_OF_FIRE			0x00020000
#define ITEM_RING_OF_UNDEAD			0x00040000
#define ITEM_RING_OF_LIGHTNING		0x00080000

	// SCG[11/17/99]: Episode 4
#define ITEM_ENVIROSUIT				0x00000100	
#define ITEM_CONTROL_CARD_YELLOW	0x00000800
#define ITEM_CONTROL_CARD_GREEN		0x00001000
#define ITEM_SULPHUR				0x00002000
#define ITEM_CHARCOAL				0x00004000
#define ITEM_SALTPETER				0x00008000
#define ITEM_BOTTLE					0x00010000
#define ITEM_BOMB					0x00020000

// SCG[11/17/99]: End savegame flags

#define MAX_PERS_INVENTORY			20
typedef struct client_save_inv_s
{
	short		valid;
	char		name[64];
	int			use;
	int			command;
	char		model[MAX_OSPATH];
	int			flags;

	char		targetName[64];
	char		netname[64];
	char		sound[MAX_OSPATH];
} client_save_inv_t;

// client data that stays across multiple level loads
#define MAX_SUBMAPS	 8
typedef struct client_persistant_s
{
	char		userinfo[MAX_INFO_STRING];
	char		netname[16];
	char		sounddir[MAX_QPATH];		// player/male, etc
	//int			hand;
    body_info_t body_info;                  // model, skin, character, color

// SCG[10/11/99]: Old Quake2 stuff
/*
	// values saved and restored from edicts when changing levels
	int			health;
	int			max_health;

  int			selected_item;
	int			inventory[MAX_ITEMS];

	// ammo capacities
	int			max_bullets;
	int			max_shells;
	int			max_rockets;
	int			max_grenades;
	int			max_cells;
	int			max_slugs;
*/
// SCG[10/11/99]: Daikatana stuff

	qboolean		bConnected;

	player_record_t	record;

	float			nHealth;					// SCG[10/11/99]: Health
	float			nArmorVal;					// SCG[10/11/99]: Armor
	float			nArmorAbs;					// SCG[10/11/99]: Armor
				
	unsigned int	nTotalKills[MAX_SUBMAPS];	// SCG[10/11/99]: Total number of kills [3] because of submaps...
	unsigned int	nTotalSecrets[MAX_SUBMAPS];	// SCG[10/11/99]: Total number of secrets [3] because of submaps...

	unsigned int	nTotalMonsters[MAX_SUBMAPS];
	unsigned int	nFoundSecrets[MAX_SUBMAPS];
	unsigned int	nTime[MAX_SUBMAPS];
				
	unsigned int	nTeam;						// SCG[10/8/99]: player team number
												
	void			*pInventory;				// SCG[10/11/99]: inventory
												
	unsigned int	nCurrentWeaponFlag;			// SCG[11/22/99]: Flag for the currently selected weapon
	unsigned int	nInventoryFlags;			// SCG[11/4/99]: Inventory needs to be flag based
	unsigned char	nWeaponFlags;				// SCG[11/4/99]: Same as above
	unsigned int	nAmmo[7];					// SCG[11/4/99]: Keep track of ammo
												
	unsigned long	nItems;						// SCG[1/3/00]: 
	unsigned long	nExpFlags;					// SCG[1/4/00]: 
	float			fInvulnerabilityTime;		// SCG[1/4/00]: 
	float			fEnvirosuitTime;			// SCG[1/4/00]: 
	float			fWraithorbTime;				// SCG[1/4/00]: 
	float			fAntidotePoints;			// cek[2-1-00]: added for antidote
// SCG[1/4/00]: 	float			fOxylungTime;				// SCG[1/4/00]: 

	float			fBasePower;					// SCG[10/8/99]: base power
	float			fBaseAttack;				// SCG[10/8/99]: base attack
	float			fBaseSpeed;					// SCG[10/8/99]: base speed
	float			fBaseAcro;					// SCG[10/8/99]: base acro
	float			fBaseVita;					// SCG[10/8/99]: base vita
												
	float			fBoostPower;				// SCG[10/8/99]: artifact boosted power
	float			fBoostAttack;				// SCG[10/8/99]: artifact boosted attack
	float			fBoostSpeed;				// SCG[10/8/99]: artifact boosted speed
	float			fBoostAcro;					// SCG[10/8/99]: artifact boosted acro
	float			fBoostVita;					// SCG[10/8/99]: artifact boosted vita
												
	float			fTimePower;					// SCG[10/11/99]: artifact power time left
	float			fTimeAttack;				// SCG[10/11/99]: artifact attack time left
	float			fTimeSpeed;					// SCG[10/11/99]: artifact speed time left
	float			fTimeAcro;					// SCG[10/11/99]: artifact acro time left
	float			fTimeVita;					// SCG[10/11/99]: artifact vita time left
												
	float			fGasHandsTime;				// cek[2-4-00]
	CVector			vAngles;					// SCG[10/11/99]: keep track of angles when changing sub - levels.
												
	// SCG[10/28/99]: Sidekick stuff			
	CVector			vOffset;					// SCG[10/28/99]: Dir of sidekick

	struct client_persistant_s	*pPersSuperfly;	// SCG[10/26/99]: persistant info for Superfly sidekick
	struct client_persistant_s	*pPersMikiko;	// SCG[10/26/99]: persistant info for Mikiko sidekick
	
	int				nSidekicksSpawnedWithClient;// SCG[2/17/00]: 

	// SCG[2/19/00]: added shit for sidekicks
	int				nAIFlags;

	client_save_inv_t inventoryCrap[MAX_PERS_INVENTORY];
} client_persistant_t;

// client data that staus across deathmatch respawns
typedef struct
{
	int			enterframe;			// level.framenum the client entered the game
	int			score;				// frags, etc
	CVector		cmd_angles;			// angles sent over in the last command
	
	// SCG[1/24/00]: inventory shit.
	int			nCurrentMode;
	int			nLastActiveMode;
	int			nInventoryDisableFlag;
	int			nInventoryEnableFlag;
	int			nInvWeaponMode;
	int			nInvItemMode;
	int			nInvSidekickMode;
	int			nUseableSelection;
} client_respawn_t;

#define	CLF_BIGSPLASH	0x00000001	//	set if client entered water fast enough for a big splash
#define	CLF_MEDSPLASH	0x00000002	//	set if client entered water fast enough for a medium splash
#define	CLF_SMALLSPLASH	0x00000004	//	set if client entered water fast enough for a small splash

// this structure is cleared on each PutClientInServer(),
// except for 'client->pers'
typedef struct gclient_s
{
	// known to server
	player_state_t		ps;				// communicated by server to clients
	int					ping;

	// private to game
	client_persistant_t	pers;
	client_respawn_t	resp;
	pmove_state_t		old_pmove;	// for detecting out-of-pmove changes

	int					showflags;	//	Nelno:	flags for inventory, help, scores, etc.

	int					ammo_index;

	int					buttons;
	int					oldbuttons;
	int					latched_buttons;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int					damage_armor;		// damage absorbed by armor
	int					damage_parmor;		// damage absorbed by power armor
	int					damage_blood;		// damage taken out of health
	int					damage_knockback;	// impact damage
	CVector				damage_from;		// origin for vector calculation

	CVector				fix_angles;		//	Nelno:	for camera states
	CVector				kick_angles;	// weapon kicks
	CVector				kick_origin;
	float				v_dmg_roll, v_dmg_pitch, v_dmg_time;	// damage kicks
	float				fall_time, fall_value;		// for view drop on fall
	float				damage_alpha;
	float				bonus_alpha;
	CVector				damage_blend;
	float				flash_alpha;		//	for cordite
    float				flash_velocity;      // how fast to fade the flash
	CVector				flash_blend;	
	CVector				v_angle;			// aiming direction
	float				bobtime;			// so off-ground doesn't change it
	CVector				oldviewangles;
	CVector				oldvelocity;

	int					old_waterlevel;
	int					clFlags;			//	client-only flags -- only get cleared when they are checked
											//	which can be up to one server frame

	int					inventoryTime;		  //	auto-refreshing inventory display

	int					last_message_frame;	//	last frame on which client sent a text message
	int					spam_count;			//	number of message client has sent

	short				needsUpdate;

	// Logic 111298 - Grappling Hook stuff
	// FIXME: add some hook here to dynamically add vars to
	// this structure from other dll's
//	int					hookstate;
//	int					hookLength;

} gclient_t;

#define HUD_UPDATE_INVEN	0X0001
#define HUD_UPDATE_WEAP		0X0002
#define HUD_UPDATE_BOOST	0X0004
#define HUD_UPDATE_XPLEVEL	0X0008

#define	SHOW_SCORES			  0x0001
#define	SHOW_SCORES_NOW		0x0002
#define SHOW_ALLHUDS      0x0004

//
// this structure is left intact through an entire game
// it should be initialized at dll load time, and read/written to
// the server.ssv file for savegames
//
typedef struct
{
	char			className[64];
	float			health;
	unsigned int	spawnflags;
	CVector			vOffset;
} transient_t;

typedef struct
{
	char		helpmessage1[512];
	char		helpmessage2[512];
	qboolean	helpchanged;

	gclient_t	*clients;		// [maxclients]

	// can't store spawnpoint in level, because
	// it would get overwritten by the savegame restore
	char		spawnpoint[512];	// needed for coop respawns

	// store latched cvars here that we want to get at often
	int			maxclients;
	int			maxentities;

	// cross level triggers
	int			serverflags;

	// items
	int			num_items;
	// NSS[3/6/00]:
	float		SpeedM;
	float		VitalityM;
	float		PowerM;

	// SCG[12/12/99]: Transient entities (entities that cross levels
	transient_t	*transients;
} game_locals_t;

///////////////////////////////////////////////////////////////////////////////
//	function typedefs
///////////////////////////////////////////////////////////////////////////////

typedef void	(*think_t)	(struct edict_s *self);
typedef void	(*touch_t)	(struct edict_s *self, struct edict_s *other, cplane_t *plane, csurface_t *surf);
typedef	void	(*use_t)	(struct edict_s *self, struct edict_s *other, struct edict_s *activator);
typedef	void	(*pain_t)	(struct edict_s *self, struct edict_s *other, float kick, int damage    );
typedef	void	(*die_t)	(struct edict_s *self, struct edict_s *inflictor, struct edict_s *attacker, int damage, CVector &point);
typedef	void	(*blocked_t)(struct edict_s *self, struct edict_s *other);
typedef void	(*remove_t)	(struct edict_s *self);


//	this is the function used to determine how much of a floating object is submerged...  By switching this
//	function, object can be made to bob outside of water (like gibs floating in the air)
typedef void	(*submerged_t) (struct edict_s *self, int *contents, float *percent_submerged);
typedef enum    entity_type_s
{
    PROJECTILE_ION,
    WEAPON_DISRUPTOR,
    WEAPON_IONBLASTER,
    WEAPON_SHOTCYCLER,
    WEAPON_C4,
    WEAPON_DAIKATANA,
    WEAPON_DKBANISH,
    WEAPON_DKBARRIER,
    WEAPON_DKDANCE,
    WEAPON_DKMELEE,
    WEAPON_DKPOSESSION,
    WEAPON_DKPROTEUS,
    WEAPON_FLASHLIGHT,
    WEAPON_GLOCK,
    WEAPON_SIDEWINDER,
    WEAPON_SHOCKWAVE,
    WEAPON_GASHANDS,
    WEAPON_DISCUS,
    WEAPON_VENEMOUS,
    WEAPON_TRIDENT,
    WEAPON_SUNFLARE,
    WEAPON_ZEUS,
    WEAPON_HAMMER,
    WEAPON_BALLISTA,
    WEAPON_SILVERCLAW,
    WEAPON_NIGHTMARE,
    WEAPON_STAVROS,
    WEAPON_WYNDRAX,
    WEAPON_BOLTER,
    WEAPON_KINETICORE,
    WEAPON_METAMASER,
    WEAPON_RIPGUN,
    WEAPON_SLUGGER,
    WEAPON_NOVABEAM,
    WEAPON_TAZERHOOK,
    ZEUS_BOLT,
    PLAYER
} entity_type_t;


// sidekick flags for picking up items
#define SUPERFLY_NO_PICKUP          0x00000001
#define MIKIKO_NO_PICKUP            0x00000002
#define SUPERFLY_ASKED_PERMISSION   0x00000004
#define MIKIKO_ASKED_PERMISSION     0x00000008

///////////////////////////////////////////////////////////////////////////////
// this file is shared between katana and the dlls so be
// careful changing it
//
// if you change the fields of this structure then the
// dlls need to be recompiled
//
// will be reduced!!!!!!
///////////////////////////////////////////////////////////////////////////////

typedef struct edict_s
{
	// QUAKE2 REQUIRED FILEDS
	entity_state_t	s;
	struct gclient_s	*client;	// NULL if not a player
									// the server expects the first part
									// of gclient_s to be a player_state_t
									// but the rest of it is opaque

    qboolean	inuse;
	int			linkcount;

	// FIXME: move these fields to a server private sv_entity_t
	link_t		area;				// linked to a division node or leaf

	int			num_clusters;		// if -1, use headnode instead
	int			clusternums[MAX_ENT_CLUSTERS];
	int			headnode;			// unused if num_clusters != -1
	int			areanum, areanum2;

	//================================

	unsigned int svflags;
	CVector		absmin, absmax, size;
	solid_t		solid;
	int			clipmask;
	struct edict_s *owner;

	char		*className;

	player_record_t	record;			// player record - kills, deaths, stats, exp

	// END OF QUAKE2 REQUIRED FIELDS

	float		freetime;			// sv.time when the object was freed

	// this is an epair list of values that we
	// got out of the bsp file
	// These need to be parsed in the spawn functions.
	// NULL in key terminates this
	userEpair_t	*epair;

	// index into sv.models (model we are using)
	// needed here?  Shouldn't set model set this
	// for us?
//	int			modelIndex;

	//
	struct edict_s  *enemy;
	struct edict_s  *goalentity;

	// the entity under this one
	// used in pushmove code
	edict_t		*groundEntity;
	int			groundEntity_linkcount;

	struct edict_s  *view_entity;
	struct edict_s  *input_entity;

	// user's inventory
	// this pointer must be saved between levels
	invenList_t	*inventory;

	// these are flags that the editor can set and when
	// the entity gets spawned the dll code can do something
	// This field is not used by the engine/
    unsigned long  spawnflags;

	// delay for nelno
	float		delay;

	// kill target
	char		*killtarget;

	// name of the model file we are using
	char		*modelName;

	movetype_t	movetype;	//&&& AMW - changed to the enum type so we can view in the debugger
    int         fragtype;   // 1.5 dsn

	CVector		velocity;
	CVector		avelocity;

    // functions
	void (*prethink)	(struct edict_s *ent);
	void (*postthink)	(struct edict_s *ent);

	touch_t		touch;
	think_t		think;
	blocked_t	blocked;
	use_t		use;
	pain_t		pain;
	die_t		die;
	remove_t	remove;				// called when an entity needs to be removed

	void		(*save)	(FILE* f, struct edict_s* self);	// called to save the userHook
	void		(*load)	(FILE* f, struct edict_s* self);	// called to load the userHook

	float		nextthink;

//	float       exppoints;          // experience points
//	float       explevel;           // experience level
	float		armor_val;
	float		armor_abs;
	float		health;
	float		hacks;				// used for storage of various things that used to be hacked into frags
									// this is a float now beacuse most of the hacks assumed it was!
	int			hacks_int;			// um, there are people who need both types, god this is a mess
//	int			frags;

	float		takedamage;
	int			deadflag;
	CVector		view_ofs;
				
	float		button0;
	float		button1;
	float		button2;
				
	float		fixangle;
	float		idealpitch;
	char		*netname;			// if this entity is a user then name of user

	unsigned long    flags;
	int			team;

	float		teleport_time;
				
	int			waterlevel;
	int			watertype;
				
	int			soundAmbientIndex;	// for storing index for items, used to reset s.sound on respawn
	char		*deathtarget;
	char 		*target;
	char 		*targetname;
	char		*parentname;		//&&& AMW 6.9.98 -	added to allow linking entities to other entities
	char		*nodeTargetName;	//					a node targets this entity
				
	char		*groupname;			//&&& AMW 6.12.98 - added to allow grouping of entities for various purposes
	int			triggerIndex;
				
	char		*spawnname;			//&&& AMW 10.6.98 - entity classname to spawn when this entity dies

	char		*keyname;			//&&& AMW 10.25.98 - any entity can require a key to be used...
    
	//for finding the entity during script operations.
    char		*scriptname;

	CVector		movedir;

	char 		*message;

	// user's local gravity
	float		gravity;

	// user hook
	// This should only be used by spawn function!!!!
	void		*userHook;

	struct userInventory_s *curWeapon;
	struct userInventory_s *curItem;

	// hierarchial model list
	struct	edict_s	*hchild;

	float		max_speed;			// entity's maximum running speed
									
	CVector		gravity_dir;		// can be used to make individual entities "attracted" to something
									// not implemented in sv_phys yet
	// mdm99.04.06 - never used									
//	void		*stickyHook;		

	// SCG[8/11/99]: Not used
//	CVector		rotation_org;		//	origin for special-case rotations (player torso)
									//	should always be an offset from self->origin
									
	CVector		angle_delta;		//	angular offsets for hierarchical models - also 
									//  used for rotation speeds for items and decos
									
	CVector		ideal_ang;			//	replaces ideal_yaw, idealpitch and adds ideal_roll as ideal_ang [2]
	CVector		ang_speed;			//	replaces yaw_speed and adds pitch speed and roll speed
									
	void		*ptr1;				//	this is actually used as a temporary hook for warping objects

	// mdm99.04.06 - i removed these and everything still compiled.  *shrug*
//	void		*t1;
//	void		*t2;

	///////////////////////////////////////////////////////////////////////////
	//	added for Q2
	///////////////////////////////////////////////////////////////////////////

	int			viewheight;
	int			light_level;

	//	for linked moving entities...
	edict_t		*teamchain;
	edict_t		*teammaster;
	CVector     childOffset;		//  offset of the child from the parent
	CVector		transformedOffset;	// offset of the child if the parent has been rotated
	CVector		spawn_origin;		//  original position of entity when spawned

//	childList_t	*child_list;

	float		elasticity;			//	MOVETYPE_BOUNCE, MOVETYPE_BOUNCEMISSILE: scales velocity on impact 
	float		dissipate;			//	MOVETYPE_FLOAT: scales velocity while fabs (ent->velocity [2]) > velocity_cap
									//	1.0 = loses no velocity when it impacts a solid
									//	2.0 = doubles velocity 
									//	0.5 = halves velocity on impact
				
	float		mass;				//	affects floating, pushing (momentum)
									//	1.0 = player mass, mass * velocity = momentum
									//	standard physics stuff, except units are players
	float		volume;				//	affects floating (could be calced from bbox, but isn't for more control)
									//	1.0 = player volume
	float		velocity_cap;
	float		percent_submerged;	//	for tracking percentage of submersion so MOVETYPE_FLOAT can
									//	act just like MOVETYPE_BOUNCE out of water
				
	submerged_t	submerged_func;		//	function which determines contents and % submerged.
									//	normally assigned to gstate->PercentSubmerged
									//	switched to another function for nightmare gibs
	csurface_t	*groundSurface;		//	pointer to surface entity is standing on.  Not valid if groundEntity = NULL
	CMapStringToPtr	*pMapAnimationToSequence;
	void		*winfo,*oldWinfo;        // current weaponInfo_t and previous

	int lastAIFrame;	// Used in AI to synch attacks to animation

	// ISP: 3-10-99 added this callback function to let the entity know when it
	//		picks up a new item
	void (*PickedUpItem) (struct edict_s *ent, const char *szItemName );

    // Logic[4/26/99]: Something to reduce the ever increasing number of flags/hacks/special-case code messes
    // Logic[4/26/99]: adding an entity TYPE int (rather than a class name). 
    entity_type_t   EntityType;
	int         nIndex;             // ISP[052799]: index into the entity manager

    // ISP [8-3-99] sidekick flags
    unsigned long   nSidekickFlag;

	char *szUniqueID;

} userEntity_t;

// typedef struct edict_s userEntity_t;

//
// track entity record  (moved here from weapon_funcs.h)
//
class trackInfo_t {
public:
   __inline trackInfo_t();

   userEntity_t *ent;						// entity to be tracked by srcent
   userEntity_t *srcent,*dstent;			// track from src, to dst entity
   userEntity_t *extra;						// extra entity (special case)
   CVector fru;								// fwd, rt, up offsets from src (compressed into vec3_t)
   CVector dstpos;							// dst position (used if dstent==NULL)
   long flags;								// uses TEF_xxxx flags - tells what's sent to client
   long fxflags;							// uses TEF_xxxx flags - marks effects to do
   long renderfx;	
   float length;
   CVector lightColor;
   float lightSize;
   int modelindex;
   int modelindex2;
   short numframes;
   float scale;
   float scale2;
   float frametime;
   CVector altpos,altpos2;
   CVector altangle;					
   char HardPoint_Name[16];					//NSS[10/27/99]:
   char HardPoint_Name2[16];				//NSS[11/7/99]:
   long Long1;
   long Long2;					
};

trackInfo_t::trackInfo_t() {
//   renderfx = 0;
}


////////////////////////////////////////////////////////////////////////
//	server state
////////////////////////////////////////////////////////////////////////

typedef struct serverState_s {
	//	some commonly accessed vars we don't want to GetCvar on all the time
    float	frametime;			
	float	time;				//	time based on frame count, always increments of 0.1
	int		realtime;	//	actual system time in milliseconds of last physics frame
	char	*mapName;			//	current mapname
	int		fogactive;			//	fog on or off
	float	fogstart;
	float	fogend;
	float	fogskyend;
	int		foginc;
	float	fogcolor[3];
	int		numSecrets;			//  number of secrets in this map
	int		numSecretsFound;	//  number of secrets found in this map
	int		numMonsters;
	int		numMonstersKilled;
	int     cdtrack;
	CVector	gravity_dir;		//	direction of gravity pull
	char	*basedir;			//	base directory that Daikatana is running in

	int		nLevelTransitionType;
	char	spawn_target[64];	// SCG[9/25/99]: spawn target.  the global version of this did not work since the .dll
								// SCG[9/25/99]: is unloaded and then re-loaded when the level changes...

	char		mapTitle[256];	// map title from the worldspawn mapname epair
	int			episode;
	int			subMap;			
	qboolean	bCinematicPlaying;	// SCG[11/17/99]: 
	char		szCinematicName[32];
	// global colors
	// get updated at beginning of level
/*
	int	global_black;
	int	global_blue;
	int	global_green;
	int	global_cyan;
	int	global_red;
	int	global_purple;
	int	global_brown;
	int	global_ltgray;
	int	global_dkgray;
	int	global_ltblue;
	int	global_ltgrn;
	int	global_ltcyan;
	int	global_ltred;
	int	global_ltpurple;
	int	global_yellow;
	int	global_white;
*/

	// for nelno and noel
	userEntity_t *attacker;
	userEntity_t *inflictor;
	userEntity_t *activator;
	userEntity_t *targeter;			//	FIXME: still necessary?
	char		 *attackWeap;		//  cek[1-24-00] for weapon taunts

	float		damage_inflicted;	//	how much damage was done during last call to Sub_Damage
	CVector		damage_vector;		//	what direction the attack came from in last call to Sub_Damage
	unsigned long	damage_flags;	//	what type of damage was done in last call to Sub_Damage

	// interface functions
	void  (*Con_Printf)(char *fmt, ...);	// prints only to the server console
	void  (*Con_Dprintf)(char *fmt, ...);	// prints only to the sever console if developer != 0
	void  (*AddCommand) (char *cmd_name, void (*function) (struct edict_s *ent));
	void  (*LightStyle)(int style, char *val);
	void  (*SetOrigin)(userEntity_t *ent, float x, float y, float z);
	void  (*SetOrigin2)(userEntity_t *ent, CVector &pos);
	void  (*SetModel)(userEntity_t *ent, char *m);
	void  (*TraceLine)( CVector &start, CVector &end, int nomonsters, userEntity_t *passent, userTrace_t *trace);
	int   (*PointContents)( CVector &point);
	void  (*RemoveEntity)(userEntity_t *ent);

	void  (*SetSize)(userEntity_t *e,
		           float minx, float miny, float minz,
				   float maxx, float maxy, float maxz);

	userEntity_t *(*FirstEntity)();
	userEntity_t *(*NextEntity)(userEntity_t *start);
	userEntity_t *(*SpawnEntity)();

    void (*bprint)(char *fmt, ...);
    void (*sprint)(userEntity_t *ent, char *fmt, ...);
	void (*particle)(int te_type, CVector &org, CVector &dir, float speed, CVector& color, int count, int type);
    void (*centerprint)(userEntity_t *ent, float msg_time, char *fmt, ...);
    
	//	Q2FIXME:	looks like these no longer need ent passed each time
	//				instead call multicast or unicast first to prefix the message...
	//				can just integrate this into each function
#ifdef	QUAKE1_NETWORK_CODE
	void (*WriteFloat)(userEntity_t *ent, int dest, float c);  /// Nelno added
    void (*WriteByte)(userEntity_t *ent, int dest, int c);
    void (*WriteChar)(userEntity_t *ent, int dest, int c);
    void (*WriteShort)(userEntity_t *ent, int dest, int c);
    void (*WriteLong)(userEntity_t *ent, int dest, int c);
    void (*WriteAngle)(userEntity_t *ent, int dest, float angle);
    void (*WriteCoord)(userEntity_t *ent, int dest, float coord);
    void (*WriteString)(userEntity_t *ent, int dest, char *str);
    void (*WriteEntity)(userEntity_t *destEnt, int dest, userEntity_t *ent);
#endif

    void (*CBuf_AddText)(const char *cmd);
// SCG[1/22/00]: 	void (*ClientCmd)(userEntity_t *ent, char *cmd);

	float (*GetCvar)(char *varName);
	void  (*SetCvar)(char *varName, char *value);

	// debug
  void (*printxy)(int x, int y, char *fmt, ...);
  void (*clearxy)();


	// inventory create/delete methods
  invenList_t	*(*InventoryNew)(mem_type memType);
  void         (*InventoryFree)(invenList_t *list);

	// inventory search methods
	struct userInventory_s *	(*InventoryFindItem)(invenList_t *list, char *name);
	struct userInventory_s *	(*InventoryFirstItem)(invenList_t *list);
	struct userInventory_s *	(*InventoryNextItem)(invenList_t *list);
	int							(*InventoryAddItem)(userEntity_t *self, invenList_t *list, struct userInventory_s *item);
	int							(*InventoryDeleteItem)(userEntity_t *self, invenList_t *list, struct userInventory_s *item);
	struct userInventory_s *	(*InventoryCreateItem)(invenList_t *list, char *name, invenUse_t use,invenCommand_t command, int modelIndex, unsigned long flags, short structSize);
	byte						(*InventoryItemCount)(invenList_t *list, char *item_name);
	void						(*InventoryUpdateClient)(userEntity_t *self, int bHideDisplay);
	void						(*InventoryNext)(userEntity_t *self);
	void						(*InventoryPrev)(userEntity_t *self);
	void						(*InventoryUse)(userEntity_t *self);
	void						(*InventoryOperate)(userEntity_t *self, int bHideDisplay);
	void						(*InventorySetMode)(userEntity_t *self, int nMode, qboolean bPerformAction);

	void						(*CommandSelectorPrev)(userEntity_t *self);
	void						(*CommandSelectorNext)(userEntity_t *self);
	void						(*CommandSelectorApply)(userEntity_t *self);
	
	// NSS[1/11/00]:Added to be able to remove weapons and ammo from the linked lists in world.dll from weapons.dll
	void						(*SIDEKICK_RemoveItem)(userEntity_t *Item);

	// weapon icon display signal
	void						(*WeaponDisplayUpdateClient)(userEntity_t *self, int hud_status);

	// model info
// SCG[1/22/00]: 	int		(*ModelInfo)(int modelIndex, userModelInfo_t *modelInfo);
	int		(*GetModelSkinIndex) (int modelIndex);

	//	Q2FIXME:	remove FrameInfo when DLLs are all clear of it
// SCG[1/22/00]: 	int		(*FrameInfo)(int modelIndex, int frameIndex, userFrameInfo_t *frameInfo);
	void	(*GetFrameName)(int modelindex, int frameindex, char *name);
	void	(*ForceModelLoad) (int modelindex, char *name);
	void	(*PreCacheModel) (int modelindex, char *name);
	void	(*AmbientSound)(CVector &pos, char *samp, int vol, int attenuation, float fade);
// SCG[1/22/00]: 	void	(*MakeStatic)(userEntity_t *ent);

	//	Q2FIXME:	remove this crap
	void	(*DrawSpriteOverlay)(char *fileName, int x, int y);

	// functions to get to the parameter list of a function
	// only call these when a registered function has been called
	char *(*GetArgv)(int arg);
	int   (*GetArgc)();

	// error function
	void (*Error)(char *error, ...);

// SCG[1/22/00]:     void  (*SetFunction) (char *funcName, void *func, char *funcDesc);
// SCG[1/22/00]:     void *(*GetFunction) (char *funcName);
// SCG[1/22/00]:     void  (*ListFunction)(char *funcName);

	//	returns the transformed and translated location of a vertex in self's model
    int (*VertInfo)(userEntity_t *self, int surfaceIndex, int vertIndex, CVector &vert);

	//	returns the index of surface surfName
	int	(*SurfIndex) (int modelIndex, char *surfName);
	int	(*TriVerts) (edict_t *ent, int surfIndex, int vertIndex, CVector &v1, CVector &v2, CVector &v3);

	void (*SetClientViewEntity)(userEntity_t *client, userEntity_t *viewEntity);
	void (*SetClientInputEntity)(userEntity_t *client, userEntity_t *inputEntity);

	///////////////////////////////////////////////////////////////////////////
	//	New stuff for Quake 2
	//	FIXME:	reorder this stuff at some point when I have time
	//	FIXME:	at some point these should be cleaned out because there are
	//			a lot of duplicate functions, but for the initial merge and
	//			E3 deadline it's just A LOT easier if they stay
	///////////////////////////////////////////////////////////////////////////

	//	collision detection
	trace_t		(*TraceLine_q2) ( CVector &start, CVector &end, edict_t *passent, int contentmask);
	trace_t		(*TraceLineTrigger) ( CVector &start, CVector &end, edict_t *passent, int contentmask, unsigned int nAcceptFlag );
	trace_t		(*TraceBox_q2) ( CVector &start, CVector &mins, CVector &maxs, CVector &end, edict_t *passent, int contentmask);

	qboolean	(*inPVS) (CVector &p1, CVector &p2);
	qboolean	(*inPHS) (CVector &p1, CVector &p2);
	void		(*SetAreaPortalState) (int portalnum, qboolean open);
	qboolean	(*AreasConnected) (int area1, int area2);
	int			(*GetTraceCount)(void);

	//	an entity will never be sent to a client or used for collision
	//	if it is not passed to linkentity.  If the size, position, or
	//	solidity changes, it must be relinked.
	void	(*LinkEntity) (edict_t *ent);
	void	(*UnlinkEntity) (edict_t *ent);		// call before removing an interactive edict
	int		(*BoxEdicts) ( CVector &mins, CVector &maxs, edict_t **list,	int maxcount, int areatype);
	void	(*Pmove) (pmove_t *pmove);		// player movement code common with client prediction

#ifndef	QUAKE1_NETWORK_CODE
	void	(*MultiCast) ( CVector &origin, multicast_t to);
	void	(*UniCast) (edict_t *ent, qboolean reliable);
	void	(*WriteChar) (int c);
	void	(*WriteByte) (int c);
	void	(*WriteShort) (int c);
	void	(*WriteLong) (int c);
	void	(*WriteFloat) (float f);
	void	(*WriteString) (char *s);
	void	(*WritePosition) ( CVector &pos);	// some fractional bits
	void	(*WriteDir) ( CVector &pos);		// single byte encoded, very coarse
	void	(*WriteAngle) (float f);
#endif

	void	(*DebugGraph) (float value, int color);

	cvar_t	*(*cvar_forceset) (const char *var_name, const char *value);	// cvar_forceset in game_import_t
	cvar_t	*(*cvar) (const char *var_name, const char *value, int flags);
	cvar_t	*(*cvar_set) (const char *var_name, const char *value);

	// sound calls
	
	void (*StartEntitySound) (userEntity_t* ent, int channel, int sound_index, float volume, float dist_min, float dist_max);
	void (*StartOriginSound) (CVector& origin, int channel, int sound_index, float volume, float dist_min, float dist_max);
    void (*SetReverbPreset)(int nPresetID) ;    // jas 9/11/98 to allow trigger_change_sfx() to change reverb setting
	void (*StreamedSound)	(const char* name, float volume);	// mdm - for streaming audio
	void (*StartMusic) (const char* name, int chan, edict_t *ent = NULL);	
	void (*StartMP3) (const char* name, int chan, float fvol, int play_count, qboolean bRestart = TRUE, edict_t *ent = NULL );	
	void (*StopMP3) (int chan, edict_t *ent = NULL);	

	// config strings hold all the index strings, the lightstyles,
	// and misc data like the sky definition and cdtrack.
	// All of the current configstrings are sent to clients when
	// they connect, and changes are sent to all connected clients.
	void	(*ConfigString) (int num, char *string);

	// new names can only be added during spawning
	// existing names can be looked up at any time
	int		(*ModelIndex) (const char *name);
	int		(*SoundIndex) (const char *name);
	int		(*ImageIndex) (const char *name);

	void	(*bprintf) (int printlevel, char *fmt, ...);
	void	(*dprintf) (char *fmt, ...);
	void	(*cprintf) (edict_t *ent, int printlevel, char *fmt, ...);
	void	(*centerprintf) (edict_t *ent, char *fmt, ...);

	// managed memory allocation
#ifdef DEBUG_MEMORY
	void*	(*X_Malloc) (size_t size, MEM_TAG tag, char* file, int line);
#else
	void*	(*X_Malloc) (size_t size, MEM_TAG tag);
#endif
	void	(*X_Free) (void* mem);
	void	(*Mem_Free_Tag) (MEM_TAG tag);
	int		(*Mem_Heap_Walk) (int stats);

	int		(*Sys_Milliseconds) ();

	void	(*SetCameraState) (userEntity_t *ent, int state, int flags);

	//	Set's fix_angles in physics, p_client.c.  This forces the clients view angles
	//	only if FL_FIXANGLES is set, or if the camera state is set to true (enabled)
	void	(*SetClientAngles) (userEntity_t *ent, CVector &angles);

	int		(*PushMove) (userEntity_t *ent, CVector &move, CVector &amove);

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
	int		(*FS_Ungetc)		(int, FILE *f);
	char*	(*FS_Gets)			(char *string, int n, FILE *f);
	long	(*FS_GetBytesRead)	();

	///////////////////////////////////////////////////////////////////////////////
	//	pointers to structures in physics.dll, set up in dk_ServerState.c
	///////////////////////////////////////////////////////////////////////////////

	game_locals_t	*game;
	level_locals_t	*level;
	game_import_t	*gi;
	game_export_t	*globals;
	userEntity_t	*g_edicts;
	void			*common_exports;

    //functions exported from the exe for in game cinematic playback.
    void (*CIN_CamLocation)( CVector &pos, CVector &angles);
    void *(*CIN_GCE_Interface)();   //exe returns interface used by gce dll
    void (*CIN_WorldPlayback)(void *funcs);    //world dll sends functions to exe to call during playback.

	userEntity_t	*(*SpawnFakeEntity) ();
	void			(*RemoveFakeEntity) (userEntity_t *fake);

	void			(*PercentSubmerged) (userEntity_t *self, int *contents, float *percent_submerged);

	void			(*RegisterFunc)(char* name, void* func);

	void			(*UNIQUEID_Remove)		(userEntity_t *self);
    //hooks to the old common.dll memory allocation functions, which are now in physics.dll.  Dont use these,
    //instead use the global overloaded operator new and operator delete functions.
//    class CClientEntityManager    *ClientEntityManager;
//    void            (CClientEntityManager::*ClientConnect)(struct edict_s *);

} serverState_t;

////////////////////////////////////////////////////////////////////////////////
// icons for inventory items
////////////////////////////////////////////////////////////////////////////////


// icons for inventory items
/*
typedef struct userIcon_s
{
	int  width, height;
	char data [];
} userIcon_t;
*/

////
////
////  Memory allocation functions.  Memory is actually allocated and freed by the old common.dll
////  allocation functions that are now located in physics.dll.
////
////
//
//void *operator new(size_t size);
//void *operator new(size_t size, mem_type type);
//void operator delete(void *);
//
//#if !_MSC_VER
////unix - new []
//void *operator new[](size_t size, mem_type type);
////unix - delete[]
//void operator delete[](void *);
////unix - new[]
//void *operator new[](size_t size, mem_type type, size_t real_size);
//#endif
//
////this new operator is for grabbing a piece of memory that is larger than the size of the actual
////structure being allocated.
//void *operator new(size_t size, mem_type type, size_t real_size);

#endif
