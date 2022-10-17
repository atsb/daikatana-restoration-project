// server.h
//define	PARANOID			// speed sapping error checking
/*
#include "qcommon.h"
#include "game.h"
*/

#include "qcommon.h"
#include "physics.h"

#define TONGUE_SAVE_ERR_LOCAL "You must be running a local game to save!"
#define TONGUE_SAVE_ERR_INGAME "You must be in a game to save!"
#define TONGUE_SAVE_ERR_DM "You can not save in a deathmatch!"
#define TONGUE_SAVE_ERR_CINE "You can not save during a cinematic!"

//=============================================================================
#define	MAX_MASTERS	8				// max recipients for heartbeat packets
typedef enum {
	ss_dead,			// no map loaded
	ss_loading,			// spawning level edicts
	ss_game,			// actively running
	ss_cinematic,
	ss_demo,
	ss_pic
} server_state_t;
// some qc commands are only valid before the server has finished
// initializing (precache commands, static sounds / objects, etc)
typedef struct
{
	server_state_t	state;			// precache commands are only valid during load
	qboolean	attractloop;		// running cinematics and demos for the local system only
	qboolean	loadgame;			// client begins should reuse existing entity
	unsigned	time;				// allways sv.framenum * 100 msec
	int			framenum;
	char		name[MAX_QPATH];			// map name, or cinematic name
	struct cmodel_s		*models[MAX_MODELS];
	char		configstrings[MAX_CONFIGSTRINGS][MAX_CONFIGSTR_LEN];
	entity_state_t	baselines[MAX_EDICTS];
	// the multicast buffer is used to send a message to a set of clients
	// it is only used to marshall data until SV_Multicast is called
	sizebuf_t	multicast;
	byte		multicast_buf[MAX_MSGLEN];
	// demo server information
	FILE		*demofile;
	qboolean	timedemo;		// don't time sync
	
	char		prepped_refresh;	//	Nelno: added for model precaching
} server_t;
#define EDICT_NUM(n) ((edict_t *)((byte *)ge->edicts + ge->edict_size*(n)))
#define NUM_FOR_EDICT(e) ( ((byte *)(e)-(byte *)ge->edicts ) / ge->edict_size)

///////////////////////////////////////////////////////////////////////////////
//	Nelno: changed name from client_state_t for compatibility with client.h
///////////////////////////////////////////////////////////////////////////////

typedef enum
{
	cs_free,		// can be reused for a new connection
	cs_zombie,		// client has been disconnected, but don't reuse
					// connection for a couple seconds
	cs_connected,	// has been assigned to a client_t, but not in game yet
	cs_spawned		// client is fully in game
} cl_state_t;

typedef struct
{
	int					areabytes;
	byte				areabits[MAX_MAP_AREAS/8];		// portalarea visibility bits
	player_state_t		ps;
	int					num_entities;
	int					first_entity;		// into the circular sv_packet_entities[]
} client_frame_t;

#define	LATENCY_COUNTS	16
#define	RATE_MESSAGES	10

typedef struct client_s
{
	cl_state_t		state;
	char			userinfo[MAX_INFO_STRING];		// name, etc

	int				lastframe;			// for delta compression
	usercmd_t		lastcmd;			// for filling in big drops

	int				commandMsec;		// every seconds this is reset, if user
										// commands exhaust it, assume time cheating

	int				frame_latency[LATENCY_COUNTS];
	int				ping;

	int				message_size[RATE_MESSAGES];	// used to rate drop packets
	int				rate;
	int				surpressCount;		// number of messages rate supressed

	edict_t			*edict;				// EDICT_NUM(clientnum+1)
	char			name[32];			// extracted from userinfo, high bits masked

    
    char            modelname[MAX_QPATH]; // model name         2.18.99
    char            skinname[MAX_QPATH];  // skin name
    int             skincolor;            // skin color
    int             character;            // character model (0=Hiro, 1=Mikiko, 2=Superfly)

	int				messagelevel;		// for filtering printed messages
	// The datagram is written to by sound calls, prints, temp ents, etc.
	// It can be harmlessly overflowed.
	sizebuf_t		datagram;
	byte			datagram_buf[MAX_MSGLEN];
	client_frame_t	frames[UPDATE_BACKUP];	// updates can be delta'd from here
	byte			*download;			// file being downloaded
	int				downloadsize;		// total bytes (can't use EOF because of paks)
	int				downloadcount;		// bytes sent
	
	int				lastmessage;		// sv.framenum when packet was last received
	int				lastconnect;
	
	netchan_t		netchan;
} client_t;
// a client can leave the server in one of four ways:
// dropping properly by quiting or disconnecting
// timing out if no valid messages are received for timeout.value seconds
// getting kicked off by the server operator
// a program error, like an overflowed reliable buffer
//=============================================================================

// MAX_CHALLENGES is made large to prevent a denial
// of service attack that could cycle all of them
// out before legitimate users connected
#define	MAX_CHALLENGES	1024

typedef struct
{
	netadr_t	adr;
	int			challenge;
	int			time;
} challenge_t;

typedef struct
{
	qboolean	initialized;	// sv_init has completed
	int			realtime;		// allways increasing, no clamping, etc
	char		mapcmd[MAX_TOKEN_CHARS];	// ie: *intro.cin+base 
	int			spawncount;			// incremented each server start
									// used to check late spawns
	client_t	*clients;			// [maxclients->value];
	int			num_client_entities;	// maxclients->value*UPDATE_BACKUP*MAX_PACKET_ENTITIES
	int			next_client_entities;		// next client_entity to use
	entity_state_t	*client_entities;	// [num_client_entities]
	int			last_heartbeat;

	challenge_t	challenges[MAX_CHALLENGES];	// to prevent invalid IPs from connecting

	// serverrecord values
	FILE		*demofile;
	sizebuf_t	demo_multicast;
	byte		demo_multicast_buf[MAX_MSGLEN];
} server_static_t;
//=============================================================================
extern	netadr_t	net_from;
extern	sizebuf_t	net_message;
extern	netadr_t	master_adr[MAX_MASTERS];	// address of the master server
extern	server_static_t	svs;				// persistant server info
extern	server_t		sv;					// local server
extern	cvar_t		*sv_paused;
extern	cvar_t		*maxclients;
extern	cvar_t		*sv_noreload;			// don't reload level state when reentering
											// development tool
extern	cvar_t		*sv_enforcetime;
extern	client_t	*sv_client;
extern	edict_t		*sv_player;
extern	cvar_t		*sv_episode;
extern  cvar_t    *sv_mapnum;
extern  cvar_t    *sv_savegamedir;
//===========================================================
//
// sv_main.c
//
void SV_FinalMessage (const char *message, qboolean reconnect);
void SV_DropClient (client_t *drop);
int SV_ModelIndex (const char *name);
int SV_SoundIndex (const char *name);
int SV_ImageIndex (const char *name);
void SV_WriteClientdataToMessage (client_t *client, sizebuf_t *msg);
void SV_ExecuteUserCommand (const char *s);
void SV_InitOperatorCommands (void);
void SV_SendServerinfo (client_t *client);
void SV_UserinfoChanged (client_t *cl);
void Master_Heartbeat (void);
void Master_Packet (void);

//GAMESPY
void basic_callback(char *outbuf, int maxlen, void *userdata);
void info_callback(char *outbuf, int maxlen, void *userdata);
void rules_callback(char *outbuf, int maxlen, void *userdata);
void players_callback(char *outbuf, int maxlen, void *userdata);

//
// sv_init.c
//
void SV_InitGame (void);
void SV_Map (qboolean attractloop, char *levelstring, qboolean loadgame);

//	Nelno:	for changing levels without killing DLLs
void SV_ChangeLevel (qboolean attractloop, char *levelstring, qboolean loadgame);
void SV_Unload (void);

//
// sv_phys.c
//
void SV_PrepWorldFrame (void);
//
// sv_send.c
//
typedef enum {RD_NONE, RD_CLIENT, RD_PACKET} redirect_t;
#define	SV_OUTPUTBUF_LENGTH	(MAX_MSGLEN - 16)

extern	char	sv_outputbuf[SV_OUTPUTBUF_LENGTH];

void SV_FlushRedirect (int sv_redirected, char *outputbuf);

void SV_DemoCompleted (void);
void SV_SendClientMessages (void);
qboolean SV_TestMulticast(multicast_t to);
void SV_Multicast (CVector &origin, multicast_t to);

void SV_ReverbPreset (int nPresetID);
void SV_StartStream (const char* name, float vol);
void SV_StartMusic(const char* name, int chan, edict_t *ent = NULL);
void SV_StartMP3( const char* name, int channel, float fvol, int play_count, qboolean bRestart, edict_t *ent = NULL );
void SV_StopMP3(int channel, edict_t *ent = NULL);
void SV_StartSound (CVector &origin, edict_t *entity, int channel,int soundindex, float volume,
	float dist_min,float dist_max, float timeofs);

void SV_ClientPrintf (client_t *cl, int level, char *fmt, ...);
void SV_BroadcastPrintf (int level, char *fmt, ...);
void SV_BroadcastCommand (char *fmt, ...);
//
// sv_user.c
//
void SV_Nextserver (void);
void SV_ExecuteClientMessage (client_t *cl);
//
// sv_ccmds.c
//
void SV_ReadLevelFile (void);
void SV_Status_f (void);
//
// sv_ents.c
//
void SV_EmitSyncEntity (edict_t *client_entity, edict_t *ent);
void SV_WriteFrameToClient (client_t *client, sizebuf_t *msg);
void SV_RecordDemoMessage (void);
void SV_BuildClientFrame (client_t *client);
void SV_Error (char *error, ...);

//
// sv_game.c
//
extern	game_export_t	*ge;

void	SV_InitNewGame (void);			//	shuts down dlls and start a new map 
void	SV_InitNewMap (void);			//	start a new map, don't shut down dlls
void	SV_ShutdownDLLs (void);
void	SV_UnloadDLLs (void);
void	SV_InitEdict (edict_t *e);
void SV_SetStats(edict_t *ent);

//
// sv_ccmds.c
//
//============================================================
//
// high level object sorting to reduce interaction tests
//
void SV_ClearWorld (void);
// called after the world model has been loaded, before linking any entities
void SV_UnlinkEdict (edict_t *ent);
// call before removing an entity, and before trying to move one,
// so it doesn't clip against itself
void SV_LinkEdict (edict_t *ent);
// Needs to be called any time an entity changes origin, mins, maxs,
// or solid.  Automatically unlinks if needed.
// sets ent->v.absmin and ent->v.absmax
// sets ent->leafnums[] for pvs determination even if the entity
// is not solid
int SV_AreaEdicts (CVector &mins, CVector &maxs, edict_t **list, int maxcount, int areatype);
// fills in a table of edict pointers with edicts that have bounding boxes
// that intersect the given area.  It is possible for a non-axial bmodel
// to be returned that doesn't actually intersect the area on an exact
// test.
// returns the number of pointers filled in
// ??? does this allways return the world?
//===================================================================
//
// functions that interact with everything apropriate
//
int SV_PointContents (CVector &p);
// returns the CONTENTS_* value from the world at the given point.
// Quake 2 extends this to also check entities, to allow moving liquids
trace_t SV_TraceLine( CVector &start, CVector &end, edict_t *passedict, int contentmask);
trace_t SV_TraceLineTrigger( CVector &start, CVector &end, edict_t *passedict, int contentmask, unsigned int nAcceptFlag);
trace_t SV_TraceBox( CVector &start, CVector &mins, CVector &maxs, CVector &end, edict_t *passedict, int contentmask);
// mins and maxs are relative
// if the entire move stays in a solid volume, trace.allsolid will be set,
// trace.startsolid will be set, and trace.fraction will be 0
// if the starting point is in a solid, it will be allowed to move out
// to an open area
// passedict is explicitly excluded from clipping checks (normally NULL)

int SV_GetTraceCount(void);
// returns to the physics dll or wherever, how many trace's occurred during the last frame

///////////////////////////////////////////////////////////////////////////////
//	dk_server.c
//
//	server functions specific to daikatana

///////////////////////////////////////////////////////////////////////////////

//void	dk_LoadServerModel	(int modelnum, char *name);
void	dk_GetFrameName		(int modelindex, int frameindex, char *name);
int		dk_GetAnimSequences	(int modelindex, frameData_t *pFrameData);
int		dk_GetModelSkinIndex(int modelIndex);
char*		dk_GetModelSkinName(int modelIndex);
void	dk_ForceModelLoad	(int modelindex, char *name);
void	dk_PreCacheModel	(int modelindex, char *name);
int		dk_VertInfo			(edict_t *ent, int surfIndex, int vertIndex, CVector &vert);
int		dk_SurfIndex		(int modelIndex, char *surfName);
int		dk_TriVerts			(edict_t *ent, int sturIndex, int vertIndex, CVector &v1, CVector &v2, CVector &v3);
