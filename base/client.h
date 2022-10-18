// client.h -- primary header for client

//define	PARANOID			// speed sapping error checking

#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "ref.h"

extern	refexport_t	re;		// interface to refresh .dll

#include "vid.h"
#include "screen.h"
#include "sound.h"
#include "input.h"
#include "keys.h"
#include "console.h"
#include "cdaudio.h"
// #include "ClientEntityManager.h"

//////////////////////////////////////////////////////////////////////////////
//	defines
//////////////////////////////////////////////////////////////////////////////
#define ONE_DIV_255 0.00392156862745098039215686274509804
#define MAX_TRAILS	5


#define SCR_VIEWSIZE_NOSKILLS   (110)
#define SCR_VIEWSIZE_NOSTATUS   (120)
#define SCR_VIEWSIZE_MAXSETTING (SCR_VIEWSIZE_NOSTATUS) // set to highest #define

#define MAX_SKILLS 5

#define DRAW_PROGRESS_DEFAULT 1
#define DRAW_PROGRESS_LOADING 2



enum
{
	CFL_HPARENT	= 0x0001,	//  entity has children
	CFL_HCHILD	= 0x0002,	//  entity has a parent
	CFL_RESPAWN	= 0x0004,	//	entity is respawning
	CFL_FREED	  = 0x0008	//	entity was freed on server
};
//	Nelno:	for client side tracing.  This may not work for non-local clients.
//			Needs to be tested.

typedef	struct
{
	qboolean	allsolid;	// if true, plane is not valid
	qboolean	startsolid;	// if true, the initial point was in a solid area
	float		fraction;	// time completed, 1.0 = didn't hit anything
	CVector		endpos;		// final position
	cplane_t	plane;		// surface normal at impact
	csurface_t	*surface;	// surface hit
	int			contents;	// contents on other side of surface hit
	int			ent;		// number of edict that was hit
} client_trace_t;

///////////////////////////////////////////////////////////////////////////////
//	hierarchical model structures
///////////////////////////////////////////////////////////////////////////////
#if 0
//	used on the client, uses centity_t
typedef	struct	cl_childInfo_s
{
	int					type;

	unsigned short	flags;
	struct centity_s	*parent;

	//	pointer to function that handles movement
	void				(*func) (const struct centity_s *child, const struct cl_childInfo_s *cinfo);

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

	float	pitch_delta;

	///////////////////////////////////////////////////////////////////////////
	//	single-tri surface matching
	///////////////////////////////////////////////////////////////////////////

	int	surfIndex;
	int	origin_vertex;
	int	right_vertex;
	int	forward_vertex;
} cl_childInfo_t;


//	list structure:	client version, uses centity_t
typedef	struct	cl_childList_s
{
	struct	cl_childList_s	*next;
	struct  centity_s		*child;
	
	cl_childInfo_t			*info;
} cl_childList_t;

#endif

//=============================================================================

typedef struct frame_s
{
	qboolean		valid;			// cleared if delta parsing was invalid
	int				serverframe;
	int				servertime;		// server time the message is valid for (in msec)
	int				deltaframe;
	byte			areabits[MAX_MAP_AREAS/8];		// portalarea visibility bits
	player_state_t	playerstate;
	int				num_entities;
	int				parse_entities;	// non-masked index into cl_parse_entities array
} frame_t;

typedef struct centity_s
{
	entity_state_t	baseline;		// delta from this if not from a previous frame
	entity_state_t	current;
	entity_state_t	prev;			// will always be valid, but might just be a copy of current

	int			serverframe;		// if not current, this ent isn't in the frame

	int			trailcount;							//	for diminishing grenade trails
	CVector		lerp_origin;						//	for trails and hierarhical models
	CVector		lerp_angles;						//	for hierarchical models
	CVector     trailPosList[MAX_TRAILS+1];			//	for RF_TRAILS
    CVector     trailAngleList[MAX_TRAILS+1];
	int			fly_stoptime;

	int				flags;						//	FL_ flags (needed for hierarchical system)

	//	if a hierarchical parent or chilid this is a pointer to this 
	//	entity in the list of renderer entities
	entity_t	*render_ent;

	CVector		origin_hack;

	float		fFrameLerp;
} centity_t;


/*
typedef struct clientinfo_s
{
	char	name[MAX_QPATH];
	struct image_s	*skin;
	struct image_s	*icon;
	char	iconname[MAX_QPATH];
	struct model_s	*model;
	struct model_s	*weaponmodel;
} clientinfo_t;
*/

typedef struct clientinfo_s
{
	char	name[MAX_QPATH];
	void *skin;
  void *model;                    
	int   character;         
} clientinfo_t;




#define	CMD_BACKUP		64	// allow a lot of command backups for very fast systems

//
// the client_state_t structure is wiped completely at every
// server map change
//
typedef struct client_state_s
{
	int			timeoutcount;

	int			timedemo_frames;
	int			timedemo_start;

	qboolean	refresh_prepped;	// false if on new level or new ref dll
	qboolean	sound_prepped;		// ambient sounds can start
	qboolean	force_refdef;		// vid has changed, so we can't use a paused refdef

	int			parse_entities;		// index (not anded off) into cl_parse_entities[]

	usercmd_t	cmd;
	usercmd_t	cmds[CMD_BACKUP];	// each mesage will send several old cmds
	int			cmd_time[CMD_BACKUP];	// time sent, for calculating pings
	short		predicted_origins[CMD_BACKUP][3];	// for debug comparing against server

	float		predicted_step;				// for stair up smoothing
	unsigned	predicted_step_time;

	CVector		predicted_origin;	// generated by CL_PredictMovement
	CVector		predicted_angles;
	CVector		prediction_error;

	frame_t		frame;				// received from server
	int			surpressCount;		// number of messages rate supressed
	frame_t		frames[UPDATE_BACKUP];

	// the client maintains its own idea of view angles, which are
	// sent to the server each frame.  It is cleared to 0 upon entering each level.
	// the server sends a delta each frame which is added to the locally
	// tracked view angles to account for standing on rotating objects,
	// and teleport direction changes
	CVector		viewangles;

	int			time;			// this is the time value that the client
								// is rendering at.  allways <= cls.realtime
	float		lerpfrac;		// between oldframe and frame

	refdef_t	refdef;

	CVector		v_forward, v_right, v_up;	// set when refdef.angles is set

	//
	// transient data from server
	//
	char		layout[1024];		// general 2D overlay
	int			inventory[MAX_ITEMS];

	//
	// non-gameserver infornamtion
	// FIXME: move this cinematic stuff into the cin_t structure
	FILE		*cinematic_file;
	int			cinematictime;		// cls.realtime for first cinematic frame
	int			cinematicframe;
	char		cinematicpalette[768];
	qboolean	cinematicpalette_active;

	//
	// server state information
	//
	qboolean	attractloop;		// running the attract loop, any key will menu
	int			servercount;	// server identification for prespawns
	char		gamedir[MAX_QPATH];
	int			playernum;

	char		configstrings[MAX_CONFIGSTRINGS][MAX_CONFIGSTR_LEN];

	//
	// locally derived information from server state
	//
	void *model_draw[MAX_MODELS];
	struct cmodel_s	*model_clip[MAX_MODELS];

	void *sound_precache[MAX_SOUNDS];
	void *image_precache[MAX_IMAGES];

	clientinfo_t	clientinfo[MAX_CLIENTS];
	clientinfo_t	baseclientinfo;

//	class CClientEntityManager    *ClientEntityManager;

	byte sidekicks_active;
	byte sidekick_commanding; 

	int  sidekick_mikiko_health;
	int  sidekick_mikiko_armor;
	int  sidekick_superfly_health;
	int  sidekick_superfly_armor;

} client_state_t;

extern	client_state_t	cl;


//==================================================================
//
//the client_static_t structure is persistant through an arbitrary number
//of server connections
//
//==================================================================


typedef enum {
	ca_uninitialized,
	ca_disconnected, 	// not talking to a server
	ca_connecting,		// sending request packets to the server
	ca_connected,		// netchan_t established, waiting for svc_serverdata
	ca_active			// game views should be displayed
} connstate_t;

typedef enum {
	dl_none,
	dl_model,
	dl_sound,
	dl_skin,
	dl_single
} dltype_t;		// download type

typedef enum {key_game, key_console, key_message, key_menu} keydest_t;

typedef struct client_static_s
{
	connstate_t	state;
	keydest_t	key_dest;

	int			framecount;
	int			realtime;			// allways increasing, no clamping, etc
	float		frametime;			// seconds since last frame

// screen rendering information
	float		disable_screen;		// showing loading plaque between levels
									// or changing rendering dlls
									// if time gets > 30 seconds ahead, break it
	int			disable_servercount;	// when we receive a frame and cl.servercount
									// > cls.disable_servercount, clear disable_screen

// connection information
	char		servername[MAX_OSPATH];	// name of server from original connect
	float		connect_time;		// for connection retransmits

	int			quakePort;			// a 16 bit value that allows quake servers
									// to work around address translating routers
	netchan_t	netchan;
	int			serverProtocol;		// in case we are doing some kind of version hack

	int			challenge;			// from the server to use for connecting
	FILE		*download;		// file transfer from server
	char		downloadtempname[MAX_OSPATH];
	char		downloadname[MAX_OSPATH];
	int			downloadnumber;
	dltype_t	downloadtype;
	int			downloadpercent;

// demo recording info must be here, so it isn't cleared on level change
	qboolean	demorecording;
	qboolean	demowaiting;	// don't record until a non-delta message is received
	FILE		*demofile;
} client_static_t;

extern client_static_t	cls;

typedef struct load_info_s
{
	int total_time;
	int sound_time;
	int bsp_time;
	int models_time;
	int images_time;
	int clients_time;

	int total_files;
	int sound_files;
	int bsp_files;
	int models_files;
	int images_files;
	int clients_files;

	long total_bytes;
	long sound_bytes;
	long bsp_bytes;
	long models_bytes;
	long images_bytes;
	long clients_bytes;
} load_info_t;

extern load_info_t load_info;



#define XPLEVELUP_BLINKTIME  400

extern int      xplevelup_mode;
extern int      xplevelup_selection;
extern float    xplevelup_timer;
extern qboolean xplevelup_blink;


#define INVMODE_ALPHA_DEFAULT     (0.75)
#define INVMODE_FADETIME        (5250.0)  // seconds * 1000

#define FLAP_ALPHA_RATE                 (0.04)
#define FLAP_ALPHA_RATE_DOUBLE          (0.02)
#define FLAP_ALPHA_RATE_DOUBLEDOUBLE    (0.01)


typedef struct alphafx_s
{
	float   alpha;			 // graphic alpha 
	int     direction;  // up 1, -1 down, 0 none
	float   fade_time;    // show display, or timer
	float	off_time;		// time to turn off
} alphafx_t;


extern alphafx_t  flap_mikiko;
extern alphafx_t  flap_superfly;
extern alphafx_t  invmode_hud;
extern alphafx_t  skill_hud;

//=============================================================================

//
// cvars
//

extern	cvar_t	*cl_checkkick;		//	checks for kick.dat in data dir, if found, kicks player
extern	float	cl_checkkicktime;	//	timer for checking kick file

extern	cvar_t	*cl_showflares;
extern	cvar_t	*cl_flarecount;				//	Nelno:	 1 = show # of flares in PVS
extern	cvar_t	*cl_particlevol;
extern	cvar_t	*cl_stereo_separation;
extern	cvar_t	*cl_stereo;

extern	cvar_t	*cl_gun;
extern	cvar_t	*cl_add_blend;
extern	cvar_t	*cl_add_lights;
extern	cvar_t	*cl_add_particles;
extern	cvar_t	*cl_add_entities;
extern	cvar_t	*cl_predict;
//extern	cvar_t	*cl_footsteps;
extern	cvar_t	*cl_noskins;
extern	cvar_t	*cl_autoskins;

extern	cvar_t	*dm_footsteps;
extern  cvar_t  *deathmatch;

extern	cvar_t	*cl_add_flares;				//  Shawn:  Added for Daikatana
extern	cvar_t	*cl_add_comparticles;		//  Shawn:  Added for Daikatana
extern	cvar_t	*cl_add_fog;				//  Shawn:  Added for Daikatana

extern	cvar_t	*cl_upspeed;
extern	cvar_t	*cl_forwardspeed;
extern	cvar_t	*cl_sidespeed;

extern	cvar_t	*cl_yawspeed;
extern	cvar_t	*cl_pitchspeed;

extern	cvar_t	*cl_run;
extern  cvar_t  *weapon_visible;
extern  cvar_t  *subtitles;

extern	cvar_t	*cl_anglespeedkey;

extern	cvar_t	*cl_shownet;
extern	cvar_t	*cl_showmiss;
extern	cvar_t	*cl_showclamp;

extern	cvar_t	*lookspring;
extern	cvar_t	*lookstrafe;
extern	cvar_t	*sensitivity;

extern	cvar_t	*m_pitch;
extern	cvar_t	*m_yaw;
extern	cvar_t	*m_forward;
extern	cvar_t	*m_side;

extern  cvar_t  *modelname;
extern  cvar_t  *skinname; // 2.15  dsn
extern  cvar_t  *skincolor;
extern  cvar_t  *character;

extern	cvar_t	*freelook;

extern	cvar_t	*cl_lightlevel;	// FIXME HACK

extern	cvar_t	*cl_paused;
extern	cvar_t	*cl_timedemo;

extern  cvar_t  *cl_cspeeds;
extern  cvar_t  *fov;

extern  cvar_t  *cl_bob;

extern	int		cl_episode_num;			  //	cl_main.cpp
extern	int		cl_last_episode_num;	//	cl_main.cpp
extern  byte  cl_death_gib;         //	cl_main.cpp
extern  byte  cl_savegem_count;     //  cl_main.cpp

extern  float cl_gameover_time;
extern  float cl_gameover_alpha;
extern  float cl_theend_time;
extern  float cl_theend_alpha;

extern	char	cl_mapname [32];

extern  cvar_t	*cl_antiskate;

//  Shawn:  Added for Daikatana
typedef struct cpvolume_s
{
	int		key;
	int		pv_active;
	int		pv_max;
} cpvolume_t;


#define	PARTICLE_GRAVITY	-250

typedef	void (*partFunc_t) (struct particle_s *p);

//  Shawn:  Moved here for Daikatana
typedef struct particle_s
{
	struct cpvolume_s	*volume;	
	struct particle_s	*next;

	float		time;

	CVector		org;
	CVector		lastorg;
	CVector		vel;
	CVector		accel;
	CVector		color_rgb;
	float		colorvel;
	float		alpha;
	float		alphavel;

	int			type;
	float		die_time;

	float		pscale;			//	scale size of particle
	float		scalevel;

	partFunc_t	pFunc;			//	movement function for this particle (allows complex particle physics)
	void		*pHook;			//	extra data storage for complex particle physics
	short		flags;            //
} cparticle_t;

//  Shawn:  Added for Daikatana
extern cparticle_t	*active_particles, *free_particles;

//  Shawn:  Added for Daikatnaa
extern cparticle_t	particles[MAX_PARTICLES];
extern int			cl_numparticles;

//  Shawn:  Added for Daikatana
extern cparticle_t	*active_comparticles, *free_comparticles;

//  Shawn:  Added for Daikatnaa
extern cparticle_t	comparticles[MAX_PARTICLES];
extern int			cl_numcomparticles;


typedef struct cdlight_s
{
	int		key;				// so entities can reuse same entry
	CVector	color;
	CVector	origin;
	float	radius;
	float	die;				// stop lighting after this time
	float	decay;				// drop this each second
	float	minlight;			// don't add when contributing less
} cdlight_t;

extern	centity_t	cl_entities[MAX_EDICTS];
extern	cdlight_t	cl_dlights[MAX_DLIGHTS];

//  Shawn:  Added for Daikatana
typedef struct cflare_s
{
	int		key;				// so entities can reuse same entry
	CVector	color;
	CVector	origin;
	CVector	angles;
	float	radius;
	float	die;				// stop lighting after this time
	float	decay;				// drop this each second
	float	falloff;			// rate of falloff from center to edge
} cflare_t;

// #define TE_MAX_TRAILS      20
#define TE_MAX_TRAILS      60
#define TE_TRAIL_INUSE     0x0001
#define TE_TRAIL_USESTART  0x0002
#define TE_TRAIL_REMOVE    0x0004
#define TE_TRAIL_SAVEVEC   0x0008
#define TE_TRAIL_WAIT      0x0010

typedef struct te_trail_s {
   float alpha;
   short next;
   short flags;
   CVector start,end;
} te_trail_t;

typedef struct te_tracker_s {
	char type;						// (1=normal ent   2=laser)                     // will have #defines soon
	long flags;						// flags
	long fxflags;					// fxflags
	long renderfx;					// renderfx flags
	int track_index;				// index of ent src_index should track
	int src_index;					// index of ent to track track_index
	int extra_index;				// index of special entity
	float forward;					// forward offset
	float right;					// right offset
	float up;						// up offset
	int dst_index;					// index of dst ent to track to
	CVector dstpos;					// dstpos to track to (if dst_index isn't used)
	float length;					// length of something (varies)
	short lightning;				// index into lightningInfo
	CVector lightColor;				// color of light
	float lightSize;				// size of light (radius)
	int modelindex;					// model index (for muzzle flash)
	int modelindex2;				// model index2 (for Whatever)
	short numframes;				// number of frames
	short curframe;					// current frame
	float nextsunsmoke;				// 
	float scale;					// scale (of animation)
	float scale2;					// scale2 (of animation)
	float frametime;				// time to display each frame
	float curframetime;				// counts down to 0, then resets to frametime
	float ss_start;					// stavros stave fx: start time
	float ss_angle;					// stavros stave fx: old angle
	CVector ss_ofs;					// stavros stave fx: old position
	CVector ss_fwd;					// stavros stave fx: old position
	CVector ss_rt;					// stavros stave fx: old position
	CVector ss_up;					// stavros stave fx: old position
	CVector altpos;					//
	CVector altpos2;				//
	CVector altangle;				//
	CVector trailLastVec;			//
	char  HardPoint[16];			// HardPoint Name
	char  HardPoint2[16];			// Second HardPoint Name
	short trailCount;				//
	short trailHead;				//
	float trailNextPoint;			// time when next point is added to trail
	long  Long1;					//		
	long  Long2;					//
   te_trail_t trail[TE_MAX_TRAILS];
   short freeMe;					// free this trackent after processing this frame!
} te_tracker_t;

// crosshair auto-aim info
typedef struct autoAim_s {
   float xofs,yofs;
   entity_state_t *ent;
   short item;
} autoAim_t;



///////////////////////////////////////////////////////////////////////
// Logic[3/25/99]:  Adding a structure which contains a function 
//                  and some profiling data.
#define MAX_DISPATCH_FUNCTIONS      64

#ifndef PROFILE_DISPATCH        // comment this block out to disable profiling
    #define PROFILE_DISPATCH
#endif

typedef struct cl_TrackEntFunction_s
{
    void        (*pvfDispatchFunction)(void);
    
    // The following data is for profiling support
    int         iTimeStartFunction;
    int         iTimeCompleteFunction;
    int         iAccumulatedTimeForFunction;
    int         iFrame;
    int         iRunCountThisFrame;
    int         iAccumulatedRunCount;
} cl_TempEventFunction_t;

void CL_InitDispatchFunctionList(void);
extern cl_TempEventFunction_t   cl_TempEventFunctionList[TEMP_EVENT_COUNT];

//
///////////////////////////////////////////////////////////////////////

extern autoAim_t autoAim;

// SCG[8/10/99]: Not used
//extern	cflare_t	cl_flares[MAX_FLARES];			//  Shawn:  Added for Daikatana

extern foginfo_t	r_foginfo;

// the cl_parse_entities must be large enough to hold UPDATE_BACKUP frames of
// entities, so that when a delta compressed message arives from the server
// it can be un-deltad from the original 
//#define	MAX_PARSE_ENTITIES	1024
// SCG[12/21/99]: hmmm... this is 4 times the quake2 max number of entities.
// SCG[12/21/99]: Sould we be doing 4 times our entities? (i.e. 4096 entities!?!?)
// SCG[12/21/99]: I'll double it...
#define	MAX_PARSE_ENTITIES	2048
extern	entity_state_t	cl_parse_entities[MAX_PARSE_ENTITIES];

//=============================================================================

extern	netadr_t	net_from;
extern	sizebuf_t	net_message;

int DrawString (int x, int y, char *s);
int DrawAltString (int x, int y, char *s);	// toggle high bit
qboolean	CL_CheckOrDownloadFile (char *filename);


void CL_AddNetgraph (void);
void CL_TeleporterParticles (entity_state_t *ent);
void CL_ParticleEffect (CVector &org, CVector &dir, int color, int count);
//void CL_ParticleEffect2 (CVector &org, CVector &dir, int color, int count);	// SCG[8/10/99]: This function is not used
// Logic[3/26/99]: void CL_IceCrystals(CVector &org, CVector &dir, int count);
void CL_IceCrystals(void);
void CL_SpiralParticles (CVector &org, CVector &mins, CVector &maxs, int msec);
void CL_SpiralParticles2 (CVector &org, CVector &mins, CVector &maxs, CVector &color, float speed, particle_type_t pt, int msec);
void CL_DoubleHelixParticles (CVector &org, CVector &mins, CVector &maxs, int msec);

// Logic[3/26/99]: void CL_BloodSplash (CVector &org, CVector &dir, int color, int count);				
void CL_BloodSplash (void);
// Logic[3/26/99]: void CL_BloodFade(CVector &org, CVector &dir, int color, int count);				//	Nelno
void CL_BloodFade(void);

// Logic[3/26/99]: void CL_SparkleSplash (CVector &org, CVector &dir, int color, int count);			//	Nelno
void CL_SparkleSplash ( void );			//	Nelno
void CL_ParticleEffectSparks(CVector vecOrigin, CVector vecDir, CVector vecColor, int iDamage);
void CL_BloodSpurt (CVector &org, CVector &dir, int color, int count);				//	Nelno
void CL_NovaEffect(CVector &org,CVector &fwd,CVector &normal);
void CL_ParticleEffect_Complex (CVector &org, CVector &dir, CVector color, int count, int type);	//	Nelno
void CL_ParticleEffect_Complex2 (CVector &org, CVector &dir, int speed, CVector& color, int count, int type, float gravity);	//	Nelno
// Logic[3/26/99]: void CL_SuperTrident(CVector &org, CVector &dir, int color, int count, int type);
void CL_SuperTrident(void);

//=================================================

void CL_GenerateVolumeParticles (int key, int type, CVector &mins, CVector &maxs, float dist, int flags);
void CL_RemoveParticleVolume (int key);
void CL_ClearEffects (void);
void CL_ClearTEnts (void);
// void CL_BlasterTrail (CVector &start, CVector &end);
void CL_QuadTrail (CVector &start, CVector &end);
void CL_RailTrail (CVector &start, CVector &end);
void CL_BubbleTrail (CVector &start, CVector &end);
// void CL_FlagTrail (CVector &start, CVector &end, float color);

// added this
void CL_PlasmaTrail (CVector &start, CVector &end);
void CL_BoomerTrail (CVector &start, CVector &end);
void CL_SunflareSmoke(CVector &origin, float scale, float count, int spread, float maxvel);

int CL_ParseEntityBits (unsigned *bits, unsigned *bits2);
void CL_ParseDelta (entity_state_t *from, entity_state_t *to, int number, int bits, int bits2);
void CL_ParseFrame (void);

void CL_ParseTEnt (void);
void CL_ParseConfigString (void);
void CL_ParseMuzzleFlash (void);
void CL_ParseMuzzleFlash2 (void);
void SmokeAndFlash(CVector &origin);

void CL_SetLightstyle (int i);

void CL_RunDLights (void);
void CL_RunLightStyles (void);

void CL_AddEntities (void);
void CL_AddDLights (void);
void CL_AddTEnts (void);
void CL_AddLightStyles (void);

void CL_ParseEffect (void);			//  Shawn:  Added for Daikatana
void CL_ParseFog (void);			//  Shawn:  Added for Daikatana
void CL_ParseFlare (void);			//  Shawn:  Added for Daikatana
void CL_RunFlares (void);			//  Shawn:  Added for Daikatana
void CL_RunFog (void);				//  Shawn:  Added for Daikatana
void CL_AddFlares (void);			//  Shawn:  Added for Daikatana
void CL_SetFogInfo (qboolean on, int inc, float r, float g, float b, float start, float end, float skyend);			//  Shawn:  Added for Daikatana

//=================================================

void CL_PrepRefresh (void);
void CL_RegisterSounds (void);

void CL_Quit_f (void);
void CL_SampleModel_f (void);

void IN_Accumulate (void);

void CL_ParseLayout (void);


//
// cl_main
//
extern	refexport_t	re;		// interface to refresh .dll

//	Nelno:	added for earlier debug output during initialization
void	CL_InitConsole (void);
//void CL_Init (void);

void CL_UnloadAll_f(void);
void CL_Loading_Info_f();

void CL_Disconnect (void);
void CL_Disconnect_f (void);
void CL_GetChallengePacket (void);
void CL_PingServers_f (void);
void CL_Snd_Restart_f (void);

void CL_ParseXPLevel(void);
void CL_ParseSaveGem(void);
void CL_InformClientDeath(void);
void CL_InformClientTheEnd(void);

//
// cl_input
//
typedef struct kbutton_s
{
	int			down[2];		// key nums holding it down
	unsigned	downtime;		// msec timestamp
	unsigned	msec;			// msec down this frame
	int			state;
} kbutton_t;

extern	kbutton_t	in_mlook, in_klook;
extern 	kbutton_t 	in_strafe;
extern 	kbutton_t 	in_speed;

void CL_InitInput (void);
void CL_SendCmd (void);
void CL_SendMove (usercmd_t *cmd);

void CL_ClearState (void);

void CL_ReadPackets (void);

int  CL_ReadFromServer (void);
void CL_WriteToServer (usercmd_t *cmd);
//void CL_BaseMove (usercmd_t *cmd);

void IN_CenterView (void);

//float CL_KeyState (kbutton_t *key);
char *Key_KeynumToString (int keynum);

//
// cl_demo.c
//
void CL_WriteDemoMessage (void);
void CL_Stop_f (void);
void CL_Record_f (void);

//
// cl_parse.c
//
#ifdef _DEBUG
extern	char *svc_strings[256];
#endif

void CL_ParseServerMessage (void);
void CL_LoadClientinfo (clientinfo_t *ci, char *s);
#ifdef _DEBUG
void SHOWNET(char *s);
#endif /* DEBUG */
void CL_ParseClientinfo (int player);

//
// cl_view.c
//
extern int gun_frame;
extern void *gun_model;

void V_Init (void);
void V_RenderView( float stereo_separation );
void V_AddEntity (entity_t *ent);
int CL_ModelIndex (char *name);
entity_t *V_GetEntity();
void V_AddParticle (CVector &org, int color, float alpha, particle_type_t type, float);
void V_AddParticleRGB( CVector &org, int color, float alpha, particle_type_t type, CVector & color_rgb, float);
void V_AddLight (CVector &org, float intensity, float r, float g, float b);
void V_AddLightStyle (int style, float r, float g, float b);

void V_AddFlare (CVector &org, CVector &ang, float intensity, float falloff, float r, float g, float b);	//  Shawn:  Added for Daikatana
void V_AddComParticle (CVector &org, int color, float alpha, particle_type_t type, float pscale, short flags, CVector &color_rgb);


//
// cl_tent.c
//
void CL_RegisterTEntSounds (void);
void CL_RegisterTEntModels (void);
void CL_SmokeAndFlash(CVector &origin);
void DK_MovingFlareSnd(CVector &pos);

// Logic[5/3/99]: View rotations/translations and other funky fx
typedef enum view_special
{
    VIEW_NONE,
    VIEW_DRUNK,
    VIEW_COUNT
} view_special_t;
typedef struct kickview_s
{
    CVector     vecKickAngles;          // destination angles
    CVector     vecAngleOffset;         // angle offset for this frame
    float       fFOV;                   // destination FOV
    float       fFOVOffset;             // FOV offset for this frame
    float       fBaseFOV;               // player's base FOV setting
    int         iKickVelocityTo;        // how fast to get TO effect (angles and FOV)
    int         iKickVelocityReturn;    // how fast to return from effect
    int         iStartTime;             // baseline for effect calculations
    float       fToInc;                   // calculated on message receipt (expedite calculations)
    float       fFromInc;
    float       fMult;
} kickview_t;
extern kickview_t g_KickView;
//
// cl_pred.c
//
void		CL_InitPrediction (void);
void		CL_PredictMove (void);
void		CL_CheckPredictionError (void);

trace_t CL_TraceLine(CVector &start, CVector &end, int passent_num, int clip_mask);
trace_t	CL_TraceBox(CVector &start, CVector &mins, CVector &maxs, CVector &end, int passent_num, int clip_mask);

//
// cl_fx.c
//
cdlight_t *CL_AllocDlight (int key);
void CL_BigTeleportParticles (CVector &org);
void CL_RocketTrail (CVector &start, CVector &end, centity_t *old);
void CL_MeteorTrail(CVector &start, CVector &end, centity_t *old, int flags);
//void CL_PoisonSpray(CVector &start,CVector &end,CVector &dir,float speed,short spread);
void CL_PoisonSpray(CVector &start,CVector &end,float speed,short spread);
void CL_AddTrails(CVector &pos,centity_t *cent);
void CL_BeamTrail(CVector &vecPos,centity_t *cent);
void CL_DiminishingTrail (CVector &start, CVector &end, centity_t *old, int flags);
// void CL_FlyEffect (centity_t *ent, CVector &origin);
//void CL_BoostEffects(centity_t *ent, CVector &origin, int fx_flag);
void CL_BoostEffects(CVector &origin, int fx_flag);
void CL_BfgParticles (entity_t *ent);
void CL_AddParticles (void);
void CL_EntityEvent (entity_state_t *ent);
void CL_AddComParticles (void);		//  Shawn:  Added for Daikatana
cparticle_t *CL_SpawnComParticle(void);
cparticle_t *CL_SpawnParticle();
void CL_GenerateComplexParticle( CVector &origin, CVector &Dir, CVector &Color, CVector &Alpha, float scale, float count, int spread, float maxvel,int Type,int Gravity);//<nss>


//
// menus
//

void M_Init (void);
void M_Keydown (int key);
void M_Draw (void);
void M_Menu_Main_f (void);
void M_Menu_Main2_f(void);
void M_ForceMenuOff (void);
void M_AddToServerList (netadr_t adr, char *info);


bool DKM_InMenu(void);

//
// cl_inv.c
//
void CL_ParseInventory (void);
void CL_KeyInventory (int key);
void CL_DrawInventory (void);

//
// cl_pred.c
//
void CL_PredictMovement (void);

#if id386
void x86_TimerStart( void );
void x86_TimerStop( void );
void x86_TimerInit( unsigned long smallest, unsigned longest );
unsigned long *x86_TimerGetHistogram( void );
#endif

//	Nelno:	added this for per-level menu resources
//	Menu.c
int	M_GetMenuPic (char *name, char *r_path, int *w, int *h);


// cl_icons.cpp

// dk_model
int	dk_GetSpecificAnimSequences(int modelindex, char *fName, frameData_t *pFrameData, char *seqName);