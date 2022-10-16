
// q_shared.h -- included first by ALL program modules
#ifndef _Q_SHARED_H
#define _Q_SHARED_H

#define	SV_FRAME_MILLISECONDS	100	//	Quake 2 default is 100
#define	CL_FRAME_MILLISECONDS	100	//	number of milliseconds in a server frame
//	Quake 2 default is 100
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "dk_std.h"
#include <time.h>
#include "dk_point.h"
#include "logfile.h"
#include "memmgr.h"
#if _MSC_VER
#include <crtdbg.h>
#endif

bool UDP_SenderSocket(void);
int UDP_Send(int len, unsigned char *buf);
long NetworkLong(long hostlong);
long HostLong(long netlong);
int UDP_Log(int msgtype, char *fmt, ...);
void UDP_ShutDown(void);
extern int g_CommonSocket;
//#ifdef __cplusplus
//extern "C" {
//#endif 

/*
#ifndef DM_DEMO
#define DM_DEMO 1
#endif
*/

typedef enum
{
    PLAYERCLASS_HIRO = 1,
    PLAYERCLASS_SUPERFLY,
    PLAYERCLASS_MIKIKO
} playerclass_t;

//	epairs for entity info
typedef struct
{
    char  *key;
    char  *value;
} userEpair_t;

// destination class for gi.multicast()
typedef enum
{
    MULTICAST_ALL,
    MULTICAST_PHS,
    MULTICAST_PVS,
    MULTICAST_ALL_R,
    MULTICAST_PHS_R,
    MULTICAST_PVS_R
} multicast_t;


/*
==============================================================

MATHLIB

==============================================================
*/
// back by popular demand...
typedef float vec_t;
typedef vec_t vec3_t[3];
typedef vec_t vec5_t[5];

typedef	int	fixed4_t;
typedef	int	fixed8_t;
typedef	int	fixed16_t;

extern const CVector vec3_origin;
extern const CVector vec3_identity;

// microsoft's fabs seems to be ungodly slow...
//float Q_fabs (float f);
//#define	fabs(f) Q_fabs(f)
extern long Q_ftol( float f );


void ClearBounds( CVector &mins, CVector &maxs );
void AddPointToBounds( CVector &v, CVector &mins, CVector &maxs );
float frand();
float crand();

int Q_log2(int val);
int BoxOnPlaneSide( CVector &emins, CVector &emaxs, struct cplane_s *plane );
float anglemod(float a);



//=============================================

char *COM_SkipPath (char *pathname);
void COM_StripExtension (char *in, char *out);
void COM_FileBase (char *in, char *out);
void COM_FilePath (char *in, char *out);
void COM_DefaultExtension (char *path, char *extension);
char *COM_Parse (char **data_p);

// data is an in/out parm, returns a parsed out token
void Com_sprintf (char *dest, int size, char *fmt, ...);

void Com_PageInMemory (byte *buffer, int size);

//=============================================

// portable case insensitive compare
int Q_stricmp (char *s1, char *s2);
int Q_strcasecmp (char *s1, char *s2);
int Q_strncasecmp (char *s1, char *s2, int n);

//=============================================

#ifdef __BIG_ENDIAN__
    #define BigShort(a)	(a)
#else
	extern short    BigShort(short l);
#endif

#ifdef __BIG_ENDIAN__
    #define BigLong(a)	(a)
#else
	extern int      BigLong (int l);
#endif

#ifdef __BIG_ENDIAN__
    #define BigFloat(a)	(a)
#else
	extern float    BigFloat (float l);
#endif

#ifdef __BIG_ENDIAN__
	extern short    LittleShort(short l);
#else
    #define LittleShort(a)	(a)
#endif

#ifdef __BIG_ENDIAN__
	extern int      LittleLong (int l);
#else
    #define LittleLong(a)	(a)
#endif

#ifdef __BIG_ENDIAN__
	extern float    LittleFloat (float l);
#else
    #define LittleFloat(a)	(a)
#endif
/*
short	BigShort(short l);
short	LittleShort(short l);
int		BigLong (int l);
int		LittleLong (int l);
float	BigFloat (float l);
float	LittleFloat (float l);
*/

void    Swap_Init (void);
char    *va(char *format, ...);

//=============================================

//
// key / value info strings
//
#define	MAX_INFO_KEY		64
#define	MAX_INFO_VALUE		64
#define	MAX_INFO_STRING		512

char *Info_ValueForKey (char *s, char *key);
void Info_RemoveKey (char *s, char *key);
void Info_SetValueForKey (char *s, char *key, char *value);

void StripInvalidChars(char * name, int max_len = -1);
void swap_backslashes(char * buf);

/*
==============================================================

SYSTEM SPECIFIC

==============================================================
*/

extern  int curtime;        // time returned by last Sys_Milliseconds

int     Sys_Milliseconds (void);
void    Sys_Mkdir (char *path);

//// large block stack allocation routines
void    *Hunk_Begin (int maxsize);
void    *Hunk_Alloc (int size);
void    Hunk_Free (void *buf);
int     Hunk_End (void);

// directory searching
#define SFF_ARCH    0x01
#define SFF_HIDDEN  0x02
#define SFF_RDONLY  0x04
#define SFF_SUBDIR  0x08
#define SFF_SYSTEM  0x10

/*
** pass in an attribute mask of things you wish to REJECT
*/
char    *Sys_FindFirst (char *path, unsigned musthave, unsigned canthave );
char    *Sys_FindNext ( unsigned musthave, unsigned canthave );
void    Sys_FindClose (void);


// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error (char *error, ...);

void Com_Printf (char *msg, ...);

/*
==========================================================

CVARS (console variables)

==========================================================
*/

#ifndef CVAR
    #define	CVAR

        #define	CVAR_ARCHIVE	1	// set to cause it to be saved to vars.rc
        #define	CVAR_USERINFO	2	// added to userinfo  when changed
        #define	CVAR_SERVERINFO	4	// added to serverinfo when changed
        #define	CVAR_NOSET		8	// don't allow change from console at all,
// but can be set from thcommand line
        #define	CVAR_LATCH		16	// save changes until server restart

// nothing outside the Cvar_*() functions should modify these fields!
typedef struct cvar_s
{
    char        *name;
    char        *string;
    char        *latched_string;    // for CVAR_LATCH vars
    int         flags;
    qboolean    modified;   // set each time the cvar is changed
    float       value;
    struct cvar_s *next;
} cvar_t;

#endif		// CVAR

// logging variables
//extern cvar_t *g_cvarLogServer;
//extern cvar_t *g_cvarLogOutput;

#define DEFAULT_CHARACTER_MODEL "models/global/m_hiro.dkm"
#define DEFAULT_HIRO_SKIN       "skins/hiro_bod_1.wal"
#define DEFAULT_MIKIKO_SKIN     "skins/miko_bod_1.wal"
#define DEFAULT_SUPERFLY_SKIN   "skins/sfly_bod_1.wal"
#define CHARACTER_MODEL_SKINNUM 512

/*
==============================================================

COLLISION DETECTION

==============================================================
*/

#define	DIST_EPSILON	(0.03125)	//	bbox must be this far away from an object to be outside of it

// i hope this works
#define CONTENTS_EMPTY  0


// lower bits are stronger, and will eat weaker brushes completely
#define	CONTENTS_SOLID			0x00000001		// an eye is never valid in a solid
#define	CONTENTS_WINDOW			0x00000002		// translucent, but not watery
#define	CONTENTS_AUX			0x00000004
#define	CONTENTS_LAVA			0x00000008
#define	CONTENTS_SLIME			0x00000010
#define	CONTENTS_WATER			0x00000020
#define	CONTENTS_MIST			0x00000040
//#define	LAST_VISIBLE_CONTENTS	0x00000040
#define	CONTENTS_CLEAR			0x00000080
#define	CONTENTS_NOTSOLID		0x00000100
#define	CONTENTS_NOSHOOT		0x00000200
#define	CONTENTS_FOG			0x00000400
#define	CONTENTS_NITRO			0x00000800
#define	LAST_VISIBLE_CONTENTS	0x00000800

// remaining contents are non-visible, and don't eat brushes

#define	CONTENTS_AREAPORTAL		0x00008000

#define	CONTENTS_PLAYERCLIP		0x00010000
#define	CONTENTS_MONSTERCLIP	0x00020000

// currents can be added to any other contents, and may be mixed
#define	CONTENTS_CURRENT_0		0x00040000
#define	CONTENTS_CURRENT_90		0x00080000
#define	CONTENTS_CURRENT_180	0x00100000
#define	CONTENTS_CURRENT_270	0x00200000
#define	CONTENTS_CURRENT_UP		0x00400000
#define	CONTENTS_CURRENT_DOWN	0x00800000

#define	CONTENTS_ORIGIN			0x01000000	// removed before bsping an entity

#define	CONTENTS_MONSTER		0x02000000	// should never be on a brush, only in game
#define	CONTENTS_DEADMONSTER	0x04000000
#define	CONTENTS_DETAIL			0x08000000	// brushes to be added after vis leafs
#define	CONTENTS_TRANSLUCENT	0x10000000	// auto set if any surface has trans
#define	CONTENTS_LADDER			0x20000000

//	Nelno:	FL_BOT considers this solid
#define	CONTENTS_NPCCLIP		0x40000000

#define	SURF_LIGHT		0x00000001	// value will hold the light strength

#define SURF_FULLBRIGHT	0x00000002	// lightmaps are not generated for this surface

#define	SURF_SKY		0x00000004	// don't draw, but add to skybox
#define	SURF_WARP		0x00000008	// turbulent water warp
#define	SURF_TRANS33	0x00000010
#define	SURF_TRANS66	0x00000020
#define	SURF_FLOWING	0x00000040	// scroll towards angle
#define	SURF_NODRAW		0x00000080	// don't bother referencing the texture

#define	SURF_HINT		0x00000100	// make a primary bsp splitter
#define	SURF_SKIP		0x00000200	// completely ignore, allowing non-closed brushes

#define	SURF_WOOD 		0x00000400	//	The following surface flags affect game physics and sounds
#define	SURF_METAL		0x00000800
#define	SURF_STONE		0x00001000
#define	SURF_GLASS		0x00002000
#define	SURF_ICE		0x00004000
#define	SURF_SNOW		0x00008000

#define	SURF_MIRROR		0x00010000	//	a mirror surface.  Not implemented yet.
#define	SURF_TRANSTHING	0x00020000	//	???
#define	SURF_ALPHACHAN	0x00040000	//	Nelno: 32 bit alpha channel (GL) or mid-texture with transparency (software)
#define SURF_MIDTEXTURE	0x00080000

#define	SURF_PUDDLE		0x00100000	//	for puddle sounds when walking
#define	SURF_SURGE		0x00200000  //  for water that surges (slow surge)
#define	SURF_BIGSURGE	0x00400000  //  faster, bigger surge
#define	SURF_BULLETLIGHT	0x00800000	//	light comes out of bullet holes
#define	SURF_FOGPLANE	0x01000000	// SCG[3/30/99]: for volumetric fog
#define	SURF_SAND		0x02000000	//	for puddle sounds when walking

// content masks
#define	MASK_ALL				(-1)
#define	MASK_SOLID				(CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_CLEAR|CONTENTS_NOSHOOT)
#define	MASK_DEADSOLID			(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_CLEAR|CONTENTS_NOSHOOT)
#define	MASK_WATER				(CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME)
#define	MASK_OPAQUE				(CONTENTS_SOLID|CONTENTS_SLIME|CONTENTS_LAVA)
#define	MASK_SHOT				(CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_CLEAR|CONTENTS_DEADMONSTER)
#define MASK_CURRENT			(CONTENTS_CURRENT_0|CONTENTS_CURRENT_90|CONTENTS_CURRENT_180|CONTENTS_CURRENT_270|CONTENTS_CURRENT_UP|CONTENTS_CURRENT_DOWN)

//	players clip against CONTENTS_PLAYERCLIP
#define	MASK_PLAYERSOLID		(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_CLEAR|CONTENTS_NOSHOOT)

//	monsters clip against CONTENTS_PLAYERCLIP and CONTENTS_MONSTERCLIP
#define	MASK_MONSTERSOLID		(CONTENTS_SOLID|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_CLEAR|CONTENTS_NOSHOOT)

//	NPCs clip against CONTENTS_PLAYERCLIP and CONTENTS__NPCCLIP
#define	MASK_NPCSOLID			(CONTENTS_SOLID|CONTENTS_NPCCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_PLAYERCLIP|CONTENTS_CLEAR|CONTENTS_NOSHOOT)

//	chooses the best clip based on self->flags
#define CHOOSE_CLIP(a)	((a->flags & FL_CLIENT) ? MASK_PLAYERSOLID : ((a->flags & FL_MONSTER) ? MASK_MONSTERSOLID : MASK_NPCSOLID))

// gi.BoxEdicts() can return a list of either solid or trigger entities
// FIXME: eliminate AREA_ distinction?
#define	AREA_SOLID		1
#define	AREA_TRIGGERS	2


// plane_t structure
// !!! if this is changed, it must be changed in asm code too !!!
//	Nelno:	I do not believe this is true.  This is a Quake 1 comment
//			that is no longer valid!  Only changing the mplane_t in
//			r_model.h requires a corresponding change in the asm code.
//			cplane_t is not even referenced in ref_soft!
typedef struct cplane_s
{
    CVector normal;
    float   dist;
    byte    type;           // for fast side tests
    byte    signbits;       // signx + (signy<<1) + (signz<<1)
    byte    pad[2];
    unsigned    short   planeIndex; //	index of this plane into map_planes (cmodel.c) and 
                                    //	r_worldmodel->planes (ref_gl) so that plane indices 
                                    //	can be passed to client for wall damage
} cplane_t;

typedef struct cmodel_s
{
    CVector     mins;
    CVector     maxs;
    CVector     origin;             // for sounds or lights
    int         headnode;
} cmodel_t;

typedef struct csurface_s
{
    char        name[16];
    int         flags;
    int         value;
    int         index;
	unsigned short	color;	// SCG[7/8/99]: added for water color
} csurface_t;


// a trace is returned when a box is swept through the world
typedef struct
{
    qboolean    allsolid;   // if true, plane is not valid
    qboolean    startsolid; // if true, the initial point was in a solid area
    float       fraction;   // time completed, 1.0 = didn't hit anything
    CVector     endpos;     // final position
    cplane_t    plane;      // surface normal at impact
    csurface_t  *surface;   // surface hit
    int         contents;   // contents on other side of surface hit
    struct edict_s  *ent;       // not set by CM_*() functions
} trace_t;

// some stuff for the ctf scoreboard
typedef struct teamInfo_s
{
	int				teamNum;
	int				captures;
	int				score;
} teamInfo_t;


#define USE_INLINE_CODE
#ifdef USE_INLINE_CODE

//////////////////////////////////////////////////////////////////////////////////////
// BOX_ON_PLANE_SIDE - extensively used bsp routine

__inline int BOX_ON_PLANE_SIDE( CVector &emins, CVector &emaxs, struct cplane_s *plane)	
{
	float *eminsPt = &emins.x;
	float *emaxsPt = &emaxs.x;

	if (plane->type < 3) 
	{
		if (plane->dist <= eminsPt[plane->type])
		{
			return (1); 
		}
		else
		{
			if (plane->dist >= emaxsPt[plane->type]) 
			{
				return (2);
			}
			else
			{
				return (3);
			}
		}
	}
	else
	{
		return (BoxOnPlaneSide( emins, emaxs, plane));
	}
}

#else
#define BOX_ON_PLANE_SIDE(emins, emaxs, p)	\
	(((p)->type < 3)?						\
	(										\
	((p)->dist <= (emins)[(int)(p)->type])?	\
			1								\
		:									\
		(									\
		((p)->dist >= (emaxs)[(int)(p)->type])?\
				2							\
			:								\
				3							\
		)									\
	)										\
	:										\
	BoxOnPlaneSide( (emins), (emaxs), (p)))
#endif



// pmove_state_t is the information necessary for client side movement
// prediction
typedef enum 
{
    // can accelerate and turn
    PM_NORMAL,
    PM_SPECTATOR,
    // no acceleration or turning
    PM_DEAD,
    PM_GIB,     // different bounding box
    PM_FREEZE,
    PM_NOINPUT,  //	client input is ignored
	PM_FREEZEDEATH // same as PM_FREEZE but angle changes are ignored too
} pmtype_t;

// pmove->pm_flags
#define	PMF_DUCKED			0x0001
#define	PMF_JUMP_HELD		0x0002

//	Quake 2 3.14 Player Movement Flags
#define	PMF_ON_GROUND		0x0004
#define	PMF_TIME_WATERJUMP	0x0008	//	pm_time is waterjump
#define	PMF_TIME_LAND		0x0010	//	pm_time is time before rejump
#define	PMF_TIME_TELEPORT	0x0020	//	pm_time is non-moving time
#define PMF_NO_PREDICTION	0x0040	//	temporarily disables prediction (used for grappling hook)

#define	PMF_CAMERAMODE		0x0080	//	viewing from camera, not player
#define	PMF_CAMERA_LOCK_X	0x0100	//	no pitch input from player, set by camera
#define	PMF_CAMERA_LOCK_Y	0x0200	//	no yaw input from player, set by camera
#define	PMF_CAMERA_LOCK_Z	0x0400	//	no roll input from player, set by camera

#define PMF_USE_HELD        0x0800  //  use/operate button held
#define PMF_CAMERA_NOLERP	0x1000	//  NSS[10/26/99]:Do not lerp the camera

#define	PMF_CAMERA_LETTER	0x2000	// SCG[12/12/99]: letterbox mode for intermission.

// this structure needs to be communicated bit-accurate
// from the server to the client to guarantee that
// prediction stays in sync, so no floats are used.
// if any part of the game code modifies this struct, it
// will result in a prediction error of some degree.
typedef struct
{
    pmtype_t    pm_type;

    short       origin[3];      // 12.3
    short       velocity[3];    // 12.3
    unsigned short       pm_flags;       // ducked, jump_held, etc
    byte        pm_time;
    short       gravity;
    short       delta_angles[3];    // add to command angles to get view direction
                                    // changed by spawns, rotating objects, and teleporters
} pmove_state_t;


//
// button bits
//
#define	BUTTON_ATTACK		1
#define	BUTTON_USE			2
#define BUTTON_SHIFT		4
#define	BUTTON_ANY			128			// any key whatsoever


// usercmd_t is sent to the server each client frame
typedef struct usercmd_s
{
    byte    msec;
    byte    buttons;
    short   angles[3];
    short   forwardmove, sidemove, upmove;
    byte    impulse;        // remove?
    byte    lightlevel;     // light level the player is standing on
} usercmd_t;


#define	MAXTOUCH	32
typedef struct
{
    // state (in / out)
    pmove_state_t   s;

    // command (in)
    usercmd_t       cmd;
    qboolean        snapinitial;    // if s has been changed outside pmove

    // results (out)
    int         numtouch;
    struct edict_s  *touchents[MAXTOUCH];

    CVector     viewangles;         // clamped

    float       viewheight;

    CVector     mins, maxs;         // bounding box size

    struct edict_s      *groundEntity;
    struct csurface_s   *groundSurface;
    int         watertype;
    int         waterlevel;

    float       max_speed;          //	for changing running speed based on attributes

    // callbacks to test the world
    //trace_t (*trace)( CVector &start, CVector &mins, CVector &maxs, CVector &end );
    trace_t (*TraceBox)( CVector &start, CVector &mins, CVector &maxs, CVector &end );
    trace_t (*TraceLine)( CVector &start, CVector &end );
    int     (*pointcontents)( CVector &point );
} pmove_t;


// entity_state_t->effects
// Effects are things handled on the client side (lights, particles, frame animations)
// that happen constantly on the given entity.
// An entity that has effects will be sent to the client
// even if it has a zero index model.
#define	EF_ROTATE			0x00000001		// rotate (bonus items)
#define	EF_GIB				0x00000002		// leave a blood trail
#define EF_FIRE				0x00000004     // 
#define	EF_SLUDGE 			0x00000008		// redlight + trail...
#define	EF_BEAMTRAIL		0x00000010		// USE ME!!
#define	EF_MAGIC_ARROW		0x00000020		//	fletcher's arrow, was EF_GRENADE
#define	EF_DIMLIGHT    		0x00000040		//	Yellow dynamic light
#define	EF_METEOR			0x00000080     // stavros meteor, was EF_BFG
#define EF_COLOR_SHELL		0x00000100
#define EF_POWERSCREEN		0x00000200
#define	EF_ANIM01			0x00000400		// automatically cycle between frames 0 and 1 at 2 hz
#define	EF_ANIM23			0x00000800		// automatically cycle between frames 2 and 3 at 2 hz
#define EF_ANIM_ALL			0x00001000		// automatically cycle through all frames at 2hz
#define EF_ANIM_ALLFAST		0x00002000		// automatically cycle through all frames at 10hz
#define	EF_FLIES			0x00004000
//#define	EF_QUAD				0x00008000
//#define	EF_PENT				0x00010000

//	QUAKE 2 3.14 effects
#define	EF_TELEPORTER		0x00020000		// particle fountain
#define EF_FLAG1			0x00040000
#define EF_FLAG2			0x00080000
#define EF_WISP_TRAIL		0x00100000
#define EF_GREENGIB			0x00200000

//	Nelno:	added for Q1 DLL compatibility, not yet supported
#define	EF_BRIGHTLIGHT 		0x00400000		// now supported as movable dynamic lights
#define	EF_LAVATRAIL		0x00800000
#define	EF_RESPAWN			0x01000000		//	start a respawn effect for this item
#define EF_POISON           0x10000000
#define EF_SMOKETRAIL       0x20000000
#define EF_BOLTTRAIL        0x40000000
//
// mike: this wouldn't work for some reason ... something mucking with the high bit??
//#define EF_TRIDENTTRAIL       0x80000000     // <sniff> no more flags ... the final sign of armageddon ... 
// it's probably a y2k problem mike  -amw   :)

//  Shawn:  Added for Daiaktana effects
//  EFFECT TYPES
#define ET_FOG				0x00000001
//#define ET_FLARE			0x00000002	// SCG[8/10/99]: Not used
#define	ET_SNOW				0x00000004
#define ET_RAIN				0x00000008
#define ET_DRIP_BLOOD		0x00000010
#define ET_DRIP_SLUDGE		0x00000020
#define ET_DRIP_WATER		0x00000040

// well, we finally did it... 32 more effect flags added
#define EF2_TRIDENTTRAIL	0x00000001
#define EF2_SNOW			0x00000002
#define EF2_RAIN			0x00000004
#define EF2_DRIP			0x00000008
#define	EF2_FIRE			0x00000010
#define EF2_FOG				0x00000020
#define EF2_USEMEPLEASE     0x00000040
#define EF2_ROTATE_CLOCK    0x00000080 // rotate clockwise (EF_ROTATE is counter clockwise)
#define EF2_BOOSTFX         0x00000100 // boost particle fx
#define EF2_BOOSTFX_ACRO    0x00000200
#define EF2_BOOSTFX_VITA    0x00000400
#define EF2_BOOSTFX_POWER   0x00000800
#define EF2_BOOSTFX_SPEED   0x00001000
#define EF2_BOOSTFX_ATTACK  0x00002000
#define EF2_STEAM           0x00004000
#define EF2_IMAGESKIN       0x00008000		// skin loaded in the image precache list
#define EF2_RENDER_SCALE    0x00010000      // Logic[5/18/99]: Smoothly scale up/down render size, hijaak enity_state_t->alpha for target scale
#define	EF2_TRACKED_PROJ	0X00020000		// So the client projectile tracker can verify that it is tracking a projectile.
#define EF2_STAVROS_AMMO	0X00040000		// stavros fire and smoke fx
#define EF2_C4_BEEP			0x00080000
#define EF2_SUNFLARE_FLAME	0X00100000
#define EF2_BLOODY_GIB		0x00200000		// flag so that non-violent clients will treat this as nodraw!
#define EF2_INVULNERABLE	0x00400000

// entity_state_t->renderfx flags
#define	RF_MINLIGHT			0x00000001		//	allways have some light (viewmodel)
#define	RF_VIEWERMODEL		0x00000002		//	don't draw through eyes, only mirrors
#define	RF_WEAPONMODEL		0x00000004		//	only draw through eyes
#define	RF_FULLBRIGHT		0x00000008		//	allways draw full intensity -- MODELS ONLY
#define	SPR_ORIENTED		RF_FULLBRIGHT	//	sprite models only flag
#define RF_BEAM				0x00000010
#define	RF_TRANSLUCENT		0x00000020
#define	SPR_ALPHACHANNEL	RF_TRANSLUCENT
#define	RF_FRAMELERP		0x00000040		//
#define	RF_DEPTHHACK		0x00000080		//	for view weapon Z crunching
#define	RF_CUSTOMSKIN		0x00000100		//	skin is an index in image_precache
#define	RF_GLOW				0x00000200		//	pulse lighting for bonus items - MODELS ONLY
#define RF_LIGHTRAY     	0x00000400		//	volumetric lighting effect
#define	RF_SURFSKIN			0x00000800		//	skinnum references surface definition - MODELS ONLY
#define RF_ENTITYSKIN   	0x00001000		//	skinnum references entity number - MODELS ONLY
#define RF_TRACKENT	        0x00002000		//	track entity ... drawn via CL_AddTrackEnts()
#define	RF_LIGHTFLARE		0x00004000		//	for flaring light entities that render along a line from
//	light to viewer
#define RF_SPOTLIGHT		0x00008000		//	spotlights (volumetric)
#define RF_LIGHTNING		0x00010000		//	lightning bolt
#define RF_TRAILS			0x00020000		//	show trails behind this entity (model==modelindex2; if 0, uses modelindex)
#define RF_INITTRAILS		0x00040000		//	initialize trails for this entity

//	Nelno:	added for Q1 DLL compatibility
#define RF_NODRAW			0x00080000		//	don't render model
#define	RF_MUZZLEFLASH		0x00100000		//	draw bright light for one frame
// SCG[12/7/99]: #define	RF_CLUSTERBASED		0x00200000		//	cluster based model
#define	RF_ENVMAP			0x00200000		//	cluster based model
#define RF_SHAKE			0x00400000     // shakes entity (randomly adjusts position)
#define RF_NOVALASER		0x00800000     // novabeam laser (uses different texture)
#define RF_PREDATOR 		0x01000000     // predator invisibility effect
#define RF_BEAM_MOVING		0x02000000     // just like RF_BEAM, but it moves (render_scale[0]==length, mins[]==forward vector)
#define	RF_PARTICLEVOLUME	0x04000000	   // this is a particle volume entity.. see effects2 for the type
#define	RF_GRAPPLE_CHAIN	0x08000000	   // use a chain texture on bean for grapple hook
#define RF_TRACER			0x10000000     // bullet tracer... (show's bullet path)
#define RF_TRACK_REFDEF		0x20000000     // for RF_TRACKENTs, forces use of refdef. values (added so pent is visible when client is being killed)
#define RF_3P_NODRAW		0x40000000     // don't draw in third person (track entities)
#define RF_IN_NITRO			0x80000000		// SCG[5/3/99]: for nitro death

// shared flags
#define SFL_TARGETABLE		0x00000001     // entity can be targeted

// ----------------------------------------------------------------------------
// Artifact FX flags to be used in the LONG1 spot of t_info and with the
// TEF_ARTIFACT_FX selected. Well... this is now being used as another set of
// track entities seeing that we have run through yet another flag set.
// ----------------------------------------------------------------------------
#define ART_FIRE				0x00000001
#define ART_LIGHTNING			0x00000002
#define ART_UNDEAD				0x00000004
#define ART_BLACKBOX			0x00000008		// NSS[11/22/99]:Black Box Special FX
#define ART_FOUNTAIN			0x00000010		// NSS[11/29/99]:Hosportal Fountain(Episode 2) Special FX
#define ART_DAIKATANA			0x00000020		// NSS[11/29/99]:Mikiko_Monster Daikatana Special FX
#define ART_STAVROS_AMMO		0x00000040		//CEK[12-1-99] :Lavarocks
#define ART_BLOODCLOUD			0x00000080		// NSS[1/4/00]: Blood cloud track Entity
#define ART_KAGE_BOUNCE			0x00000100		// NSS[1/13/00]:Special FX for One of Kage's Special Attacks.
#define ART_BUBOID_MELT			0x00000200		// NSS[1/31/00]:Sepcial FX for Bubiod Melting
#define ART_FROGINATOR_SPIT		0x00000400		// NSS[2/16/00]:Sepcial FX for Froginator
#define ART_DRAGON_BREATH		0x00000800		// NSS[2/22/00]:Special FX for Dragon breath
#define ART_DRAGON_FIRE			0x00001000		// NSS[2/22/00]:Special FX for Dragon Fire
#define ART_DRAGON_FIREBALL		0x00002000		// NSS[2/22/00]:Special FX for Dragon FireBall

// ----------------------------------------------------------------------------



// track entity flags (tells what info is sent to client)
#define TEF_SRCINDEX		0x00000001		// srcindex included in msg
#define TEF_FRU				0x00000002		// forward/right/up ofs included in msg
#define TEF_DSTINDEX		0x00000004		// dst_index included in msg
#define TEF_LENGTH			0x00000008		// length included in msg
#define TEF_DSTPOS			0x00000010		// dstpos included in msg
#define TEF_LIGHTCOLOR		0x00000020		// light color
#define TEF_LIGHTSIZE		0x00000040		// light size
#define TEF_MODELINDEX		0x00000080		// model index (for muzzle flash)
#define TEF_NUMFRAMES		0x00000100		// num frames
#define TEF_SCALE			0x00000200		// scale (for animations)
#define TEF_FRAMETIME		0x00000400		// amount of time each frame should be shown
#define TEF_EXTRAINDEX		0x00000800		//
#define TEF_ALTPOS			0x00001000		//
#define TEF_ALTPOS2			0x00002000		//
#define TEF_ALTANGLE		0x00004000		//
#define TEF_FXFLAGS			0x00008000		//
#define TEF_MODELINDEX2		0x00010000		// Another model index number < nss >
#define TEF_SCALE2			0x00020000		// Another Scale number for the new model index number. < nss >
#define TEF_HARDPOINT		0x00040000		// 32 Chars MAX for the name of the hard point you want to spawn the entity FX from < nss >
#define TEF_HARDPOINT2		0x00080000		// 32 Chars MAX for the name of the second hard point you want to pass. < nss >
#define	TEF_LONG1			0x00100000		// Just extra Longs to be used if needed //NSS[11/7/99]:
#define TEF_LONG2			0x00200000		// Ditto //NSS[11/7/99]:

//		************READ THIS ABOUT TRACK ENTITIES***************
//		The server side entity HAS to have a modelindex in order
//		for a track entity to actually work.  If you must, set it
//		to a sprtie and give it a stupidly small scale value(i.e. 0.00001)
//		has worked for me so far in some cases.


// track entity FX flags
#define TEF_ADDFX			0x00000001		// add fx to entity that's already being tracked
#define TEF_REMOVEFX		0x00000002		// remove fx from entity already being tracked

#define TEF_LIGHT			0x00000004		// draws light with ent
#define TEF_FLASHLIGHT		0x00000008		// flashlight - (client trace, draws light)
#define TEF_ANIMATE			0x00000010		// show animation at ("hr_muzzle")
#define TEF_ARTIFACT_FX		0x00000020		// All artifacts that have an FX use this track entity flag
#define TEF_FIRE			0x00000040		// fire (sunflare)
#define TEF_DAIKATANA		0x00000080		// daikatana sword effects
#define TEF_NOPARTICLES		0x00000100		// don't add extra particles to this temp entity
#define TEF_TRACER			0x00000200		// do tracer effect (for slugger)
#define TEF_NOROTATE		0x00000400		// no rotation on TEF_ANIMATE
#define TEF_JETTRAIL		0x00000800		// Jet trail<nss>
#define TEF_SPRITE			0x00001000		// Attached a sprite or model with alpha blend set to .30, you will be able to pass the amount of alpha later.<nss>
#define TEF_MODEL			0x00002000      // A hack to get stuff to spawn at the model's set hardpoint.
#define TEF_COMPLEXPARTICLE 0x00004000      // A very modular particle generator that mappers have control over.
#define TEF_SHRINKINGMODEL	0x00008000		// A cool model that will shrink itself
#define TEF_SMOKETRAIL		0x00010000		// Standard smoke trail for tracked entities...
#define TEF_ANIMATE2		0x00020000		// animation attached to first or third person weapon hard point
#define TEF_LIGHT_SHRINK    0x00040000		// Same as light but shrinks at 'x' rate determined by the length value in tinfo.
#define TEF_STAVROS			0x00080000		// Stavros's particle FX for the Meteor
#define TEF_METABEAMS		0x00100000		// metamaser beam effect
#define TEF_THUNDERSKEET_FX 0x00200000		// ThunderSkeet FX
#define TEF_PROJ_FX			0x00400000		// projectile tracker effects
#define TEF_FX_ONLY			0x00800000		// will absolutely never cause the entity this trackent is attached to be drawn here.
#define TEF_FLAMESWORD		0x01000000		// Knight1's flamesword FX
#define TEF_ELECTRICSWORD	0x02000000		// Knight2's Lightning FX Battle axe thingy
#define TEF_REAPER			0X04000000		// nightmare reaper fx
#define	TEF_ALPHA_SPRITE	0x08000000		// just like TEF_SPRITE but allows customizable alpha

///////////NOTE ON TEF_MODEL2/////////////////
//TEF_MODEL2 is to be used ONLY in conjunction with TEF_MODEL.  Likewise you CANNOT use sprites with this nor can you use the flamesword
//TEF_MODEL2 you can only use: TEF_LIGHT,TEF_TRACER & TEF_SMOKETRAIL with it... perhaps others but LOOK before using!!!!!
//NSS[11/11/99]: Yes this is a minor hack
#define	TEF_MODEL2			0x10000000		// just like TEF_MODEL but used the modelIndex2 for the model
#define TEF_ATTACHMODEL		0x20000000		// used with either TEF_MODEL or TEF_MODEL2 to make them 'stick' to the hardpoint;
//When using TEF_MODEL2 with TEF_MODEL the modelIndex goes with TEF_MODEL and the modelIndex2 goes with TEF_MODEL2... if you use either by theirself,
//then they use this sequence:  TEF_MODEL or TEF_MODEL2 goes with modelIndex2(both do the same thing) ... TEF_SPRITE takes the modelIndex over if both are not used together.

#define TEF_SUNFLARE_FX		0X40000000		// move all the sunflare fx to the client
#define TEF_SLUDGE_FX		0X80000000		// Last Track Entity... but I had to do it.


// some stuff for the reaper
#define	TEF_REAPER_INIT		0X00000001		// initialization stuff (sparkles)
#define TEF_REAPER_SMOKE	0X00000002		// do some complex smoke

// some defines and enums necessary for tracking projectiles
#define TEF_PROJ_FLY		0x0001
#define TEF_PROJ_SPECIAL	0X0002
#define TEF_PROJ_LAUNCH		0x0004
#define TEF_PROJ_DIE		0X0008
#define TEF_PROJ_WATER		0X0010			// use to force water!
#define TEF_PROJ_IDS_MASK	0Xff00

#define TEF_PROJ_ID_SHIFT	8					// bits to shift the id!
#define TEF_ID_OF( x )			( (x) << TEF_PROJ_ID_SHIFT )
#define TEF_INDEX_OF( x )		( ((x) & TEF_PROJ_IDS_MASK) >> TEF_PROJ_ID_SHIFT )
enum
{
	TEF_PROJ_ID_FIRST = 0,
	TEF_PROJ_ID_KCORE,							// this will then be 0x0100
	TEF_PROJ_ID_SIDEWINDER,
	TEF_PROJ_ID_BALLISTA,
	TEF_PROJ_ID_IONBLASTER,
	TEF_PROJ_ID_VENOMOUS,
	TEF_PROJ_ID_TRIDENT,
	TEF_PROJ_ID_WISP,
	TEF_PROJ_ID_MAX
};


typedef enum
{
	FLASH_NONE,
	FLASH_ION,
	FLASH_GLOCK,
	FLASH_SLUGGER,
	FLASH_RIPGUN,
	FLASH_SHOTCYCLER,
	FLASH_SHOCKWAVE,
	MAX_FLASHES
} enum_muzzle_flashes;

// track entity effects that can be added and removed without altering other settings
#define TEF_EFFECTS			(TEF_LIGHT|TEF_FIRE|TEF_ALPHA_SPRITE)

// general msg flags that are removed after the msg is processed
#define TEF_MESSAGE_FLAGS	(TEF_SRCINDEX|TEF_FRU|TEF_DSTINDEX|TEF_LENGTH|TEF_DSTPOS|TEF_LIGHTCOLOR|TEF_LIGHTSIZE)

// player_state_t->refdef flags
#define	RDF_UNDERWATER		0x0001		//	warp the screen as apropriate
#define RDF_NOWORLDMODEL	0x0002		//	used for player configuration screen
#define	RDF_CLEARCOLORBUFF	0x0004
#define RDF_CLEARDEPTHBUFF	0x0008
#define	RDF_INFOG			0x0010
#define RDF_LETTERBOX		0x0020
#define RDF_HALTALLDRAWING	0x0040
#define RDF_NOCHEATING		0x0080
#define RDF_NOLERP			0x0100
#define RDF_COOP			0x0200		// hack to tell a client they're in coop mode.
#define RDF_NO_ICONS		0x0400
#define RDF_DEATHMATCH		0x0800
#define	RDF_CINESCREENHACK	0x1000
//
// muzzle flashes / player effects
//
// these should be or'd onto the numframes member of the track info strucure
#define	TEF_ANIM2_FIRST		0X0100			// show animation in first person
#define TEF_ANIM2_THIRD		0X0200			// show animation in third person
#define TEF_ANIM2_LOOP		0X0400			// animation will not turn itself off when the animation is complete
#define TEF_ANIM2_BILLBOARD 0x0800			// two sprites instead of one!
#define TEF_ANIM2_SMOKE		0X1000			// add some smoke, too
#define TEF_3POINT_ANIM		0x00000080		// show animation at 3 points ("hr_muzzle","hr_muzzle2","hr_muzzle3") yanked from above (now a long2 flag)

// daikatana effect flags (long1)
#define TEF_DK_LEVEL1		0x00000001
#define TEF_DK_LEVEL2		0x00000002
#define TEF_DK_LEVEL3		0x00000004
#define TEF_DK_LEVEL4		0x00000008
#define TEF_DK_LEVEL5		0x00000010

extern  CVector monster_flash_offset[];

//	Nelno: particle types, in dk_shared for sending with TE_PARTICLE
typedef enum
{
    PARTICLE_SIMPLE         = 0,
    PARTICLE_SNOW,
    PARTICLE_RAIN,
    PARTICLE_BLOOD,
    PARTICLE_BLOOD2,
    PARTICLE_BLOOD3,
    PARTICLE_BLOOD4,
    PARTICLE_BLOOD5,
    PARTICLE_BUBBLE,
    PARTICLE_SMOKE,
    PARTICLE_SPARKS,
    PARTICLE_BIG_SPARKS,
    PARTICLE_POISON,
    PARTICLE_BLUE_SPARKS,
    PARTICLE_ICE,
    PARTICLE_SPARKLE1,     // acrobatic boost, attack boost, power boost, speed boost, vitality boost
    PARTICLE_SPARKLE2,
    PARTICLE_DRIP_BLOOD,
    PARTICLE_DRIP_SLUDGE,
    PARTICLE_DRIP_WATER,
    PARTICLE_SPLASH1,
    PARTICLE_SPLASH2,
    PARTICLE_SPLASH3,
    PARTICLE_BEAM_SPARKS,
    PARTICLE_FIRE,
	PARTICLE_CRYOSPRAY,
    PARTICLE_SPARK_1,
    PARTICLE_SPARK_2,
    PARTICLE_FIREFLY,
	PARTICLE_CP1,
	PARTICLE_CP2,
	PARTICLE_CP3,
	PARTICLE_CP4,
    NUM_PARTICLETYPES,

    PARTICLE_RANDOM_SPREAD = 128   //	bit flag for use when sending down a TE_PARTICLE_xxxx type
    //	if set, particles origin's are generated randomly around specified origin
    //	DO NOT USE OTHERWISE, AND ALWAYS CLEAR BEFORE SETTING ACTUAL PARTICLE TYPE!!!
} particle_type_t;

// temp entity events
//
// Temp entity events are for things that happen
// at a location seperate from any existing entity.
// Temporary entity messages are explicitly constructed
// and broadcast.
typedef enum
{
    TE_GUNSHOT,                                                                // 0
    TE_BLOOD,
    TE_EXPLOSION1,
    TE_EXPLOSION2,
    TE_SPARKS,
    TE_SHIELD_SPARKS,
    TE_LASER_SPARKS,
    TE_BFG_LASER,
    TE_TRACK_ENTITY,
    TE_NOVAHIT,                 //	Nelno: novabeam hitting a wall
    TE_EXPLOSION_SMALL1,        //	sound                                           // 30
    TE_EXPLOSION_SMALL2,            //	no sound
    TE_BLUE_SPARKS,
    TE_EXPLOSION,
    TE_ALPHASCALE,
    TE_SCALEROTATE,
    TE_SMOKE,
    TE_BEAM_SPARKS,
    TE_BLOOD_FADE,
    TE_ICE_CRYSTALS,
    TE_TRACER,                                                                 // 40
    TE_PARTICLE_FALL,       //	particles will be affected by gravity
    TE_PARTICLE_RISE,       //	particles will rise up
    TE_PARTICLE_FLOAT,      //	particles will not fall or rise
							//	to send a TE_PARTICLE_xxxx message
							//	WritePos (origin)
							//	WriteDir (dir)
							//	WriteByte (speed) -- NOTE: real speed will be this value * 10!
							//	WriteByte (count)
							//	WriteByte (color)
							//  WriteFloat( color.x )
							//  WriteFloat( color.y )
							//  WriteFloat( color.z )
							//	WriteByte (type) -- NOTE: OR with PARTICLE_RANDOM_SPREAD for slightly random origins
    TE_FIRESMOKE,
    TE_SCALEIMAGE,                                                             // 45
    TE_SPARKLES,
    TE_BOLTER_SPARKS,
    TE_SUPERTRIDENT,         // for trident, effect when tips merge together
    TE_POLYEXPLOSION,        // polygonal explosion
    TE_POLYEXPLOSION_SND,	// polygonal explosion with sound					// 50
    TE_SURFSPRITE,           // for tagging a surface with a sprite
    TE_BLOODSPLAT,           // tag surface with blood
    TE_BULLETHOLE,           // tag surface with bullet hole
    TE_HEADSHOT,             // tag surface with bloody, brainy mess
    TE_SCORCHMARK,           // tag surface with a black scorch mark			// 55
    TE_EARTHCRACK,          // crack surface sprite
	TE_FISTMARK,			// fist damage sprite
    TE_RAINSPLASH,           // splash effect for rain 
    TE_TRAVELING_SPARKS,	// used by disruptor glove
    TE_SPIRAL_PARTICLES,     // create a bunch of spiral particles
    TE_DOUBLEHELIX_PARTICLES, // create a bunch of spiral particles
    TE_SHOCKORB_BOUNCE,     // Logic[4/8/99]: The server sends this message whenever the shock orb bounces
	TE_CRYO_SPRAY,		// spray of poison from cryotech wand
    TE_LIGHT,               // temporary light
    TE_KICKVIEW,            // client viewport kick effect, see cl_tent.cpp TempEvent_KickView() and cl_ents.cpp CL_CalcViewValues()
    TE_RICOCHET,            // ricochet sound
	TE_ALPHASCALEROTATE,
	TE_SPIRAL_PARTICLES2,	// adds more control and takes away some particle type/color change crap
    TEMP_EVENT_COUNT
} temp_event_t;

#define SPLASH_UNKNOWN		0
#define SPLASH_SPARKS		1
#define SPLASH_BLUE_WATER	2
#define SPLASH_BROWN_WATER	3
#define SPLASH_SLIME		4
#define	SPLASH_LAVA			5
#define SPLASH_BLOOD		6


#define SIDEKICK_MIKIKO   0x0001 
#define SIDEKICK_SUPERFLY 0x0002

#define SIDEKICK_STATUS_ACTIVATE    0
#define SIDEKICK_STATUS_DEACTIVATE  1
#define SIDEKICK_STATUS_HEALTH      2
#define SIDEKICK_STATUS_ARMOR       3
#define SIDEKICK_STATUS_COMMANDING  4


// sound channels
// channel 0 never willingly overrides
// other channels (1-7) allways override a playing sound on that channel
#define	CHAN_AUTO               0
#define	CHAN_WEAPON             1
#define	CHAN_VOICE              2
#define	CHAN_ITEM               3
#define	CHAN_BODY               4
#define CHAN_LOOP				5	// all sounds on this channel will loop.  use 0.0 volume to turn off
// modifier flags
#define	CHAN_NO_PHS_ADD			8	// send to all clients, not just ones in PHS (ATTN 0 will also do this)
#define	CHAN_RELIABLE			16	// send by reliable message, not datagram

// define music channels
#define CHAN_MUSIC_MENU			0
#define CHAN_MUSIC_MAP			1
#define CHAN_CINE_1				2
#define CHAN_CINE_2				3
#define CHAN_CINE_3				4
#define CHAN_CINE_4				5
#define CHAN_CINE_5				6
#define CHAN_CINE_6				7

// sound attenuation values
#define	ATTN_NONE               0	// full volume the entire level
#define	ATTN_NORM               1
#define	ATTN_IDLE               2
#define	ATTN_STATIC             3	// diminish very rapidly with distance

//NSS[11/11/99]:Changed the mins values... they were screwed!
#define ATTN_NORM_MIN			256.0f	
#define ATTN_NORM_MAX			648.0f	

#define SND_NONDIRECTIONAL		0x0001

// player_state->stats[] indexes
#define STAT_ARMOR				  0           
#define STAT_HEALTH				  1
#define STAT_LEVEL				  2
#define STAT_AMMO				    3
#define STAT_EXP            4

#define	STAT_JUMP				    5		//	0 - 5 for level
#define	STAT_SPEED				  6		//	0 - 5 for level
#define	STAT_ATTACK       	7		//	0 - 5 for level
#define	STAT_POWER		      8		//	0 - 5 for level
#define	STAT_MAX_HEALTH			9		//	0 - 5 for level

#define	STAT_SWORD_LEVEL	  10  //	power level of Daikatana

#define STAT_INVISIBLE      11	//  invisible
#define STAT_INVULNERABLE		12  //  invulnerable

#define STAT_KILLS				13
#define STAT_MONSTERS			14
#define STAT_FOUND_SECRETS		15
#define STAT_SECRETS			21
#define STAT_TIME				22
#define STAT_TOTAL_TIME			23
#define STAT_SAVE_GEMS			24
/*
#define STAT_HEALTH_ICON		16
#define	STAT_HEALTH_OLD				17
#define	STAT_AMMO_ICON			18
#define	STAT_AMMO_OLD				19
#define	STAT_ARMOR_ICON		20
#define	STAT_ARMOR_OLD				21
#define	STAT_SELECTED_ICON	22
#define	STAT_PICKUP_ICON		23
#define	STAT_PICKUP_STRING	24
#define	STAT_TIMER_ICON		25
#define	STAT_TIMER				26
#define	STAT_HELPICON			27
*/

#define STAT_SIDEKICKS_EXIST	16     // 5.25   npc's that exist
#define STAT_MIKIKO_HEALTH		17     //        health % of sidekick #1
#define STAT_MIKIKO_ARMOR		18     //        armor  % of sidekick #1
#define STAT_SUPERFLY_HEALTH	19     //        health % of sidekick #2
#define STAT_SUPERFLY_ARMOR		20     //        armor  % of sidekick #2

#define STAT_SUPERFLY_FLAGS		25		// for sf invis, invul
#define STAT_MIKIKO_FLAGS		26		// for mk invis, invul

#define	STAT_SELECTED_ITEM		28
#define	STAT_LAYOUTS			29
#define	STAT_FRAGS				30
#define	STAT_FLASHES			31		// cleared each frame, 1 = health, 2 = armor

#define	MAX_STATS				32

#define STAT_FLAG_INVULNERABLE	0X0001
#define STAT_FLAG_INVISIBLE		0X0002

#define	LAYOUT_SCOREBOARD		0x0001
#define LAYOUT_SHOWALLHUDS		0x0002
#define LAYOUT_CTF				0x0004
#define LAYOUT_DT				0X0008
#define LAYOUT_COOP				0x0010	

#define STAT_CTF_YOUR_FLAG_HOME			0x00000001
#define STAT_CTF_YOU_HAVE_ENEMY_FLAG	0x00000002
#define STAT_CTF_YOUR_TEAM				0x00000004
#define STAT_CTF_TEAM_CONVERT_SHIFT		(3)
#define STAT_CTF_TEAM_CAPTURES_SHIFT	(7)

#define NO_FRAGS_USED   -999


// dmflags->value flags
#define	DF_NO_HEALTH		1
#define	DF_NO_ITEMS			2
#define	DF_WEAPONS_STAY		4
#define	DF_NO_FALLING		8
#define	DF_INSTANT_ITEMS	16
#define	DF_SAME_LEVEL		32
#define DF_SKINTEAMS		64
#define DF_MODELTEAMS		128
#define DF_NO_FRIENDLY_FIRE	256
#define	DF_SPAWN_FARTHEST	512
#define DF_FORCE_RESPAWN	1024
#define DF_NO_ARMOR			2048
#define DF_ALLOW_EXIT		4096
#define DF_INFINITE_AMMO	8192
#define DF_QUAD_DROP		16384
#define DF_FIXED_FOV		32768

/*
==========================================================

  ELEMENTS COMMUNICATED ACROSS THE NET

==========================================================
*/

#define	ANGLE2SHORT(x)	((int)((x)*65536/360) & 65535)
#define	SHORT2ANGLE(x)	((x)*(360.0/65536))

#define	ANGLE2LONG(x)	((long int)((x) * 4294967296 / 360) & 4294967295)
#define	LONG2ANGLE(x)	((x)*(360.0 / 4294967296))

//extern double  *costable;
//extern double  *sintable;
//#define TRIGTABLE_SIZE                  4096    // Logic[4/4/99]: expedite with a shift operation
//#define SINTABLE(y) sintable[y&TRIGTABLE_SIZE-1]
//#define COSTABLE(y) costable[y&TRIGTABLE_SIZE-1]


//
// config strings are a general means of communication from
// the server to all connected clients.
// Each config string can be at most MAX_QPATH characters.
//
#define	CS_NAME				0
#define	CS_CDTRACK			1
#define	CS_SKY				2
#define	CS_STATUSBAR		3		// display program string
#define	CS_PALETTE			4

#define CS_CLOUDNAME		6

#define CS_SKY_2			7
#define CS_CLOUDNAME_2		8
#define CS_SKY_3			9
#define CS_CLOUDNAME_3		10
#define CS_SKY_4			11
#define CS_CLOUDNAME_4		12
#define CS_SKY_5			13
#define CS_CLOUDNAME_5		14


#define	CS_MAXCLIENTS		30
#define	CS_MAPCHECKSUM		31		// for catching cheater maps

#define	CS_MODELS			32
#define	CS_SOUNDS			(CS_MODELS+MAX_MODELS)
#define	CS_IMAGES			(CS_SOUNDS+MAX_SOUNDS)
#define	CS_LIGHTS			(CS_IMAGES+MAX_IMAGES)
#define	CS_ITEMS			(CS_LIGHTS+MAX_LIGHTSTYLES)
#define	CS_PLAYERSKINS		(CS_ITEMS+MAX_ITEMS)
#define	MAX_CONFIGSTRINGS	(CS_PLAYERSKINS+MAX_CLIENTS)


//==============================================


// entity_state_t->event values
// ertity events are for effects that take place reletive
// to an existing entities origin.  Very network efficient.
// All muzzle flashes really should be converted to events...
typedef enum
{
    EV_NONE,
    EV_ITEM_RESPAWN,
    EV_FOOTSTEP,
    EV_FALLSHORT,
    EV_FALL,
    EV_FALLFAR,
    EV_PLAYER_TELEPORT,
    EV_ENTITY_FORCEMOVE,        // special case where we want to move an entity with no prediction
    EV_FOOTSTEP_WOOD,
    EV_FOOTSTEP_METAL,
    EV_FOOTSTEP_STONE,
    EV_FOOTSTEP_GLASS,
    EV_FOOTSTEP_ICE,
    EV_FOOTSTEP_SNOW,
    EV_FOOTSTEP_SAND,
	EV_FOOTSTEP_LADDER_WOOD,
	EV_FOOTSTEP_LADDER_METAL,
    EV_FOOTSTEP_PUDDLE,
    EV_SPLASH_BIG,
    EV_SPLASH_MED,
    EV_SPLASH_SMALL,
    EV_DROWNING,
    EV_SUCKITDOWN,
    EV_BUBBLES,
	EV_SWIM,
    EV_NUM_EVENTS
} entity_event_t;



//the name of each main menu button.
typedef enum
{
    DKMB_NEWGAME = 0,
    DKMB_MULTIPLAYER,
    DKMB_LOADGAME,
    DKMB_SAVEGAME,
    DKMB_SOUND,
    DKMB_VIDEO,
    DKMB_MOUSE,
    DKMB_KEYBOARD,
    DKMB_JOYSTICK,
    DKMB_OPTIONS,
    DKMB_CONFIG,
//    DKMB_DEMOS,
	DKMB_CREDITS,
    DKMB_RESUME,
    DKMB_QUIT,

    DKMB_NUM_BUTTONS,
    DKMB_NONE
} dk_main_button;


#define DKMB_FIRST_MENU_IN  (DKMB_NEWGAME)



////////////////////////////////////////////////////////////////////////////////////
///	client-side frame animation
////////////////////////////////////////////////////////////////////////////////////
// frameFlag
#define		FRAME_LOOP			0x0001		// loop from startFrame to endFrame
#define		FRAME_ONCE			0x0002		// go from startFrame to endFrame then stop
#define		FRAME_STATIC		0x0004		// remain on one frame
#define		FRAME_REPEAT		0x0008		// loop through a partial sequence for loopCount times
#define		FRAME_TRANS			0x0010
#define		FRAME_NEEDTRANS		0x0020	//	this frame sequence needs a transition played at beginnig and end
#define		FRAME_FPS10			0x0040	//	10 fps, default is 20 
#define		FRAME_FPS30			0x0080	//	30 fps, default is 20 
#define		FRAME_WEAPONSWITCH	0x0100  //	go to next weapon
#define		FRAME_CLIENTSOUNDS	0x0200	//	play sounds on client if set
#define		FRAME_FORCEINDEX	0x0400	//	forces self->s.frame to be sent to client
#define     FRAME_WEAPONSWITCH2 0x0800  //  weapon switch 2, (requested switch during animation -- recall when done animating)

#define     FRAME_WEAPON       0x4000     // for animating weapon via winfoAnimate()... ent passed must be player
#define     FRAME_NODELAY      0x8000     // makes winfoAnimate() NOT set attack_finished (mainly for ambients)
//frameState
//	flags set by server/client animation code
#define		FRSTATE_LAST		0x0001	//	reached last frame in sequence
#define		FRSTATE_PLAYSOUND1	0x0002
#define		FRSTATE_PLAYSOUND2	0x0004
#define		FRSTATE_STOPPED		0x0008	//	no longer animating
#define		FRSTATE_STARTANIM	0x0010	//	no longer animating
#define		FRSTATE_TRANSITION	0x0020	//	playing a transition sequence
#define		FRSTATE_PLAYATTACK1	0x0040
#define		FRSTATE_PLAYATTACK2	0x0080


//	frameInfo.frameTime should always equal one of these defines
#define		FRAMETIME_FPS5 		0.2		//	not supported due to flag limitations, forced to 10 fps, see Nelno
#define		FRAMETIME_FPS10		0.1
#define		FRAMETIME_FPS15		0.066
#define		FRAMETIME_FPS20		0.05	//	not supported due to flag limitations, forced to 20 fps, see Nelno
#define		FRAMETIME_FPS25		0.04	//	not supported due to flag limitations, forced to 20 fps, see Nelno
#define		FRAMETIME_FPS30		0.033	
#define		FRAMETIME_FPS35		0.028	//	not supported due to flag limitations, forced to 30 fps, see Nelno
#define		FRAMETIME_FPS40		0.025	//	not supported due to flag limitations, forced to 30 fps, see Nelno

//	Nelno:	frameInfo_t, for client-based animation
//	some of these fields can be sent to the client on an entity-specific basis

typedef struct  frameInfo_s
{
    short   frameFlags;         //	don't sync s.frame if changed

    short   frameState;         //	bits are set in server animation code when 
                                //	corresponding criteria is met. 
                                //	NEVER COMMUNICATED FROM SERVER TO CLIENT
                                //	OR VICE VERSA!!

    short   startFrame;         //	sync client s.frame to server s.frame if changed
    short   endFrame;           //	sync client s.frame to server s.frame if changed

    short   startLoop;          //	sync client s.frame to server s.frame if changed
    short   endLoop;            //	sync client s.frame to server s.frame if changed
    short   loopCount;          //	don't sync s.frame, only send if change in loopCount > 1???

    short   sound1Frame;        //	don't sync s.frame
    short   sound2Frame;        //	don't sync s.frame

    short   sound1Index;        //	don't sync s.frame
    short   sound2Index;        //	don't sync s.frame

    char    frameInc;           //	don't sync s.frame???

    float   frameTime;          //	NEVER COMMUNICATED FROM SERVER TO CLIENT
                                //	OR VICE VERSA!!
    float   next_frameTime;     //	NEVER COMMUNICATED FROM SERVER TO CLIENT
                                //	OR VICE VERSA!!

    int     nAttackFrame1;      //	NEVER COMMUNICATED FROM SERVER TO CLIENT
                                //	OR VICE VERSA!!
    int     nAttackFrame2;      //	NEVER COMMUNICATED FROM SERVER TO CLIENT
                                //	OR VICE VERSA!!
    float   modelAnimSpeed;     //  Ash -- speed unscaled model should move for animation to not skate
} frameInfo_t;

//////////////////////////////////////////////////////////////////////////////////////
////	client-side hierarchical animation
////	
////	if any of this info changes on the server, it must be sent down to the client
//////////////////////////////////////////////////////////////////////////////////////
//
////	hierarchical types
//#define	HR_SURFACE				1	//	attaches child to a specific surface
//#define	HR_RELATIVE				2	//	attaches child based on initial position of child relative to parent
//#define	HR_BMODEL				3	//	not implemented from DK Quake 1 source yet -- might not be necessary...
//#define	HR_SURFACE_OLD			4	//	attaches child to a specific set of surfaces (same as HR_SURFACE but
////	less efficient -- should be phased out eventually)
//
/////////////////////////////////////////////////////////////////////////////////
////	hierarchical model typedefs
/////////////////////////////////////////////////////////////////////////////////
//
//#define	HRF_NOROLL			0x0001	//	sticking object's roll not tied to parent
//#define	HRF_NOPITCH			0x0002	//	sticking object's pitch not tied to parent
//#define	HRF_NOYAW			0x0004	//	sticking object's yaw not tied to parent
//#define	HRF_NOMOVE			0x0008	//	sticking objects movement not tied to parent
//#define	HRF_NOREMOVE 		0x0020	//	don't remove this this sticky if parent is removed
//#define	HRF_FLIPFORWARD		0x0080	//	flip normal of forward surface
//#define	HRF_FLIPRIGHT		0x0100	//	flip normal of right surface
//#define	HRF_OFFSET			0x0200	//	offest model by child->offset field
//#define	HRF_SERVER_ONLY		0x0400	//	hrInfo will not be sent to client
//#define	HRF_CLIENT_ONLY		0x0800	//	server will only place children at the origin of their parent
////	so they stay in the same PVS.  The client will do the final
////	placement of the entity based on hierarchical attributes.
////	This really should be the default so that dedicated servers
////	can work correctly (ie. no model loading on the server!)
//#define	HRF_REMOVE			0x4000	//	set when server has removed entity so client can remove it
////	from all child lists
//#define	HRF_RESET			0x8000	//	hrInfo changed, so reset everything on client
////	this flag should never need to be set by the server
//
////	part of entity_state structure
//typedef struct  hrInfo_s
//{
//    short int   parentNumber;       //	entity number of parent entity
//    unsigned short int hrFlags;     //	hierarchical flags
//    byte        hrType;             //	type of hierarchical model
//    byte        surfaceIndex;       //	index to surface in parent's model
//    byte        vertexIndex;        //	index to vertex in parent's model
//} hrInfo_t;


///////////////////////////////////////////////////////////////////////////
// clusterDef_t
//
// defines angles and other data for cluster based models
///////////////////////////////////////////////////////////////////////////

#define MAX_MESH_CLUSTERS	3

typedef struct clusterDef_s
{
	int		clusterIdx;				// index into the client/server cluster def table
	CVector angles;					// angles for this cluster
} clusterDef_t;


///////////////////////////////////////////////////////////////////////////////
//
//  player_record_t
//
//  holds all the player information such as kills, deaths, etc.
//
///////////////////////////////////////////////////////////////////////////////

typedef struct player_record_s
{
	int				frags;
	int				deaths;
	unsigned long	exp;
	short			level;
	unsigned long	dkexp;
} player_record_t;

///////////////////////////////////////////////////////////////////////////
// entity_state_t is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
///////////////////////////////////////////////////////////////////////////

    #define	IF_SV_SENDBBOX	0x00000001	//	Server flag:	send down real bounding box size when it changes
    #define	IF_CL_REALBBOX	0x00000001	//	Client flag:	use real mins/max sizes for prediction

typedef struct entity_state_s
{
    int				number;         //	edict index

    CVector			origin;
    CVector			angles;
    CVector			old_origin;     //	for lerping
	CVector			color;			// SCG[8/11/99]:  Added for effects.

    int				modelindex;
    int				modelindex2, modelindex3, modelindex4;  //	weapons, CTF flags, etc
    int				frame;
    int				skinnum;
					
    unsigned int    effects;        // using 32 bits, but expecting unsigned in creating delta's.. DOH!
    unsigned int    effects2;       // ditto
    unsigned int	renderfx;       // ditto
    float			alpha;          //	for RF_TRANSLUCENT
					
    int				solid;          //	for client side prediction, 8*(bits 0-4) is x/y radius
					                //	8j*(bits 5-9) is z down distance, 8(bits10-15) is z up
					                //	gi.linkentity sets this properly
					                //	Nelno: ONLY IF SVF_SENDBBOX IS NOT SET, DAMNIT!
    
	// sound engine variables
	unsigned char	snd_flags;		// flags for sound
	int				sound;          // for looping sounds, to guarantee shutoff
    float			volume;			// looping sound volume
	float			dist_min;		// minimum distance (volume = full)
	float			dist_max;		// maximum distance (volume = none)
	CVector			origin_hack;	// used by the sound engine for those funky bmodels

	int				event;          //	impulse events -- muzzle flashes, footsteps, etc
					                //	events only go out for a single frame, they
					                //	are automatically cleared each frame
					
    CVector			render_scale;   //	Nelno: added for dynamic model scaling
    CVector			mins, maxs;     //  Shawn:  Added for particle volumes

    frameInfo_t		frameInfo;
    

//	hrInfo_t		hrInfo;			// ******************** soon to go away **************************
    CVector			angle_delta;    //	angular offsets for hierarchical models - also
	
	int				numClusters;	
	clusterDef_t	cDef[MAX_MESH_CLUSTERS];		// cluster definitions MAX_MESH_CLUSTERS

    int				iflags;         //	independent flags, ie. client and server don't send these back and forth
									//	added for getting accurate bounding box info for client prediction

	unsigned int    flags;          //	general flags the client needs to know about


/*	*** NUKE THIS SOON ***
    int				soundDefID ;    //	JAS: definition id containing bunches of sound parameters
*/

/*	*** NUKE THIS SOON ***   
	short			entityID;       //	unique ID generated each time an entity is created
*/
} entity_state_t;

//==============================================


// player_state_t is the information needed in addition to pmove_state_t
// to rendered a view.  There will only be 10 player_state_t sent each second,
// but the number of pmove_state_t changes will be reletive to client
// frame rates
typedef struct
{
    pmove_state_t   pmove;      // for prediction

    // these fields do not need to be communicated bit-precise

    CVector     viewangles;     // for fixed views
    CVector     viewoffset;     // add to pmovestate->origin
    CVector     kick_angles;    // add to view direction to get render angles
                                // set by weapon kicks, pain effects, etc

    CVector     gunangles;
    CVector     gunoffset;

//    int         gunindex;      // mike: might be able to kill these, i'll look for references.
//    int         gunframe;      // index/frame are stored in weapon (below)

    void        *weapon;       // it's actually a (userEntity_t *)

    float       blend[4];       // rgba full screen effect

    float       fov;            // horizontal field of view

    int         rdflags;        // refdef flags

    //short       stats[MAX_STATS];       // fast status bar updates
    // mdm98.02.18 - need a long for the exp
	  long       stats[MAX_STATS];       // fast status bar updates
    int        skills_active;   // skills-in-use flags

    int         view_entity;
    int         input_entity;
} player_state_t;

//-------------------------------------------------------------
// CL_Explosion() flags
//-------------------------------------------------------------
    #define CLE_FADEALPHA        0x01

/*
==========================================================
    PARTICLE_VOLUME
==========================================================
*/

//	Umm.... these are the same as RF_ flags, that makes things fuck up.
/*
#define PV_TYPESNOW		0x01000000			// specifies the particle volume as a snow volume
#define PV_SNOW_SLOW	0x00000001			// slow 
#define PV_SNOW_FAST	0x00000002			// fast 
#define PV_SNOW_HEAVY	0x00000004			// heavy
#define PV_SNOW_LIGHT	0x00000008			// flurries
#define PV_SNOW_DRIFT1	0x00000010			// drift

#define PV_TYPERAIN		0x02000000			// specifies the particle volume as a rain volume
#define PV_RAIN_SLOW	0x00000001			// slow rain
#define PV_RAIN_FAST	0x00000002			// fast rain
#define PV_RAIN_0		0x00000004			// straight down
#define PV_RAIN_45		0x00000008			// 45 degree angle
*/

/*
==========================================================
    SPRITE FLAGS
==========================================================
*/

/*
//	OLDER FLAGS BEFORE CONSOLIDATION WITH RF_ FLAGS
#define SPR_ALPHACHANNEL		0x00000001			// entity has a alpha channel
#define SPR_ORIENTED			0x00000002			// entity is oriented
#define SPR_ROTATE_X			0x00000004			// entity is oriented
#define SPR_ROTATE_Y			0x00000008			// entity is oriented
#define SPR_ROTATE_Z			0x00000010			// entity is oriented
*/

//  Shawn:  global colors
/*
enum
{
    GC_BLACK,
    GC_BLUE,
    GC_GREEN,
    GC_CYAN,
    GC_RED,
    GC_PURPLE,
    GC_BROWN,
    GC_LYGRAY,
    GC_DKGRAY,
    GC_LTBLUE,
    GC_LTGRN,
    GC_LTCYAN,
    GC_LTRED,
    GC_LTPURPLE,
    GC_YELLOW,
    GC_WHITE,
    GC_NUMCOLORS
};
*/

//#ifdef __cplusplus
//}
//#endif 

// game.serverflags values
#define SFL_CROSS_TRIGGER_1		0x00000001
#define SFL_CROSS_TRIGGER_2		0x00000002
#define SFL_CROSS_TRIGGER_3		0x00000004
#define SFL_CROSS_TRIGGER_4		0x00000008
#define SFL_CROSS_TRIGGER_5		0x00000010
#define SFL_CROSS_TRIGGER_6		0x00000020
#define SFL_CROSS_TRIGGER_7		0x00000040
#define SFL_CROSS_TRIGGER_8		0x00000080
#define SFL_CROSS_TRIGGER_MASK	0x000000ff

#define SFL_SPAWN_SUPERFLY		0x00010000
#define SFL_SPAWN_MIKIKO		0x00020000
#define SFL_SPAWN_MIKIKOFLY		0x00040000
#define SFL_SIDEKICK_MASK		0x000f0000

#define RELIABLE_UNTRACK(ent)		com->untrackEntity(ent,NULL,MULTICAST_ALL);com->untrackEntity(ent,NULL,MULTICAST_ALL_R)

// for the save game header
typedef struct save_header_s
{
	bool				bCoop;
	unsigned int		monsters;
	unsigned int		secrets;
	unsigned int		tMonsters;
	unsigned int		tSecrets;

	float				hiroHealth;
	float				hiroArmor;
	short				hiroLevel;
	float				sfHealth;
	float				sfArmor;
	float				sfLevel;
	float				mHealth;
	float				mArmor;
	float				mLevel;

	long				time;
	long				total;

	short				episode;
	char				caption[256];
	char				mapTitle[256];
} save_header_t;


#endif

