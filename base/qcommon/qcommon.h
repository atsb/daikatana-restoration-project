#ifndef __QCOMMON_H__
#define __QCOMMON_H__

// qcommon.h -- definitions common between client and server, but not game.dll

#include    "dk_shared.h"

#define VERSION (__DATE__" "__TIME__)

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

//#define	LOCKED_DEMO	//	Daikatana will run in a locked demo mode if this is set.
						//	Menu will work, with New Game sub-menu disabled.
						//	Console will be disabled.
//#define	CeBIT_DEMO	//	For German CeBIT show.  Disables Episode 4 demos, sets
						//	sv_violence cvar to 0 by default

#define	BOOT_MENU		//	Daikatana will boot directly into menu, bypassing console
						//	This should eventually be the default after playing intro
						//	cinematics

#ifdef WIN32

#ifdef NDEBUG
#define BUILDSTRING "RELEASE"
#else
#define BUILDSTRING "DEBUG"
#endif

#ifdef _M_IX86
#define CPUSTRING   "x86"
#elif defined _M_ALPHA
#define CPUSTRING   "AXP"
#endif

#else   // !WIN32


#define BUILDSTRING "NON-WIN32"
#define CPUSTRING   "NON-WIN32"

#endif


#define NUM_RANDOM_BKG_SCREENS 4  // # of random background screens


///////////////////////////////////////////////////////////////////////////////
//  define name of physics dll
///////////////////////////////////////////////////////////////////////////////

extern  char    *gamename;
extern  char    *debugdir;

///////////////////////////////////////////////////////////////////////////////
//  globals by Nelno.  Wardrobe by Calvin Klein
///////////////////////////////////////////////////////////////////////////////

#define SV_UNLOAD_DLLS  true
#define SV_KEEP_DLLS    false

//============================================================================

typedef struct sizebuf_s
{
    qboolean    allowoverflow;  // if false, do a Com_Error
    qboolean    overflowed;     // set to true if the buffer size failed
    byte    *data;
    unsigned int     maxsize;
    unsigned int     cursize;
    unsigned int     readcount;
} sizebuf_t;

void SZ_Init (sizebuf_t *buf, byte *data, int length);
void SZ_Clear (sizebuf_t *buf);
void *SZ_GetSpace (sizebuf_t *buf, int length);
void SZ_Write (sizebuf_t *buf, const void *data, int length);
void SZ_Print (sizebuf_t *buf, char *data); // strcats onto the sizebuf

//============================================================================

struct usercmd_s;
struct entity_state_s;

void MSG_WriteChar (sizebuf_t *sb, int c);
void MSG_WriteByte (sizebuf_t *sb, int c);
void MSG_WriteShort (sizebuf_t *sb, int c);
void MSG_WriteLong (sizebuf_t *sb, int c);
void MSG_WriteFloat (sizebuf_t *sb, float f);
void MSG_WriteString (sizebuf_t *sb, const char *s);
void MSG_WriteCoord (sizebuf_t *sb, float f);
void MSG_WritePos (sizebuf_t *sb, CVector &pos);
void MSG_WriteAngle (sizebuf_t *sb, float f);
void MSG_WriteAngle16 (sizebuf_t *sb, float f);
void MSG_WriteDeltaUsercmd (sizebuf_t *sb, struct usercmd_s *from, struct usercmd_s *cmd);
void MSG_WriteDeltaEntity (struct entity_state_s *from, struct entity_state_s *to
                           , sizebuf_t *msg, qboolean force);
void MSG_WriteDir (sizebuf_t *sb, CVector &vector);


void    MSG_BeginReading (sizebuf_t *sb);

int     MSG_ReadChar (sizebuf_t *sb);
int     MSG_ReadByte (sizebuf_t *sb);
int     MSG_ReadShort (sizebuf_t *sb);
int     MSG_ReadLong (sizebuf_t *sb);
float   MSG_ReadFloat (sizebuf_t *sb);
char    *MSG_ReadString (sizebuf_t *sb);
char    *MSG_ReadStringLine (sizebuf_t *sb);

float   MSG_ReadCoord (sizebuf_t *sb);
void    MSG_ReadPos (sizebuf_t *sb, CVector &pos);
float   MSG_ReadAngle (sizebuf_t *sb);
float   MSG_ReadAngle16 (sizebuf_t *sb);
void    MSG_ReadDeltaUsercmd (sizebuf_t *sb, struct usercmd_s *from, struct usercmd_s *cmd);

void    MSG_ReadDir (sizebuf_t *sb, CVector &vector);

void    MSG_ReadData (sizebuf_t *sb, void *buffer, int size);

//============================================================================

extern  qboolean        bigendien;

#ifdef __BIG_ENDIAN__
#define BigShort(a)	(a)
#else
extern short	BigShort(short l);
#endif

#ifdef __BIG_ENDIAN__
#define BigLong(a)	(a)
#else
extern int		BigLong (int l);
#endif

#ifdef __BIG_ENDIAN__
#define BigFloat(a)	(a)
#else
extern float	BigFloat (float l);
#endif

#ifdef __BIG_ENDIAN__
extern short	LittleShort(short l);
#else
#define LittleShort(a)	(a)
#endif

#ifdef __BIG_ENDIAN__
extern int		LittleLong (int l);
#else
#define LittleLong(a)	(a)
#endif

#ifdef __BIG_ENDIAN__
extern float	LittleFloat (float l);
#else
#define LittleFloat(a)	(a)
#endif

/*
extern  short   BigShort (short l);
extern  short   LittleShort (short l);
extern  int     BigLong (int l);
extern  int     LittleLong (int l);
extern  float   BigFloat (float l);
extern  float   LittleFloat (float l);
*/

//============================================================================


int EXEGetArgc();
char *EXEGetArgv(int arg);  // range and null checked
void COM_ClearArgv (int arg);
int COM_CheckParm (char *parm);
void COM_AddParm (char *parm);

void COM_Init();
void COM_InitArgv (int argc, char **argv);

char *CopyString (const char *in);

//============================================================================

void Info_Print (char *s);


/* crc.h */

void CRC_Init(unsigned short *crcvalue);
void CRC_ProcessByte(unsigned short *crcvalue, byte data);
unsigned short CRC_Value(unsigned short crcvalue);
int CRC_Block (byte *start, int count);




/*
==============================================================

PROTOCOL

==============================================================
*/

// protocol.h -- communications protocols

#define PROTOCOL_VERSION    32

//=========================================

#define PORT_MASTER 27990
#define PORT_CLIENT 27991
#define PORT_SERVER 27992

//=========================================

#define UPDATE_BACKUP   16  // copies of entity_state_t to keep buffered
                            // must be power of two
#define UPDATE_MASK     (UPDATE_BACKUP-1)



//==================
// the svc_strings[] array in cl_parse.c should mirror this
//==================

//
// server to client
//
enum svc_ops_e
{
	svc_bad,

	// these ops are known to the game dll
	svc_muzzleflash,
	svc_muzzleflash2,
	svc_temp_entity,
	svc_layout,
	svc_inventory,
	svc_onscreen_icon,
	svc_clientdeath,
	svc_sidekick_update,
	//svc_sidekick_command,
	svc_selector,
	svc_weapon_icon,
	svc_xplevel,
	svc_savegem,
	svc_clientrespawn,

	// the rest are private to the client and server
	svc_nop,
	svc_disconnect,
	svc_reconnect,
	svc_sound,                  // <see code>
	svc_print,                  // [byte] id [string] null terminated string
	svc_stufftext,              // [string] stuffed into client's console buffer, should be \n terminated
	svc_serverdata,             // [long] protocol ...
	svc_configstring,           // [short] [string]
	svc_spawnbaseline,      
	svc_centerprint,            // [string] to put in center of the screen
	svc_download,               // [short] size [size bytes]
	svc_playerinfo,             // variable
	svc_packetentities,         // [...]
	svc_deltapacketentities,    // [...]
	svc_frame,
	svc_effect_notify,          //  Shawn:  added for Daikatana
	svc_setclientmodels,
	svc_syncentity,
	svc_scoreboard,
	svc_cliententity,           // Logic:   Client smarts, see libclient
	svc_crosshair,				// SCG[10/4/99]: For enabling/disabling the crosshair
	svc_end_of_game,
	svc_ammo_count_update,
	svc_sidekick_weapon_status,
	svc_start_music,
	svc_start_mp3,
	svc_stop_mp3,
	svc_reverb_preset,
	svc_start_stream,
	svc_client_string			// cek[6-20-00]: added for client-language based string messages
};

//==============================================

//
// client to server
//
enum clc_ops_e
{
    clc_bad,
    clc_nop,        
    clc_move,               // [[usercmd_t]
    clc_userinfo,           // [[userinfo string]
    clc_stringcmd          // [string] message
};

//==============================================

// plyer_state_t communication

#define PS_M_TYPE           (1<<0)
#define PS_M_ORIGIN         (1<<1)
#define PS_M_VELOCITY       (1<<2)
#define PS_M_TIME           (1<<3)
#define PS_M_FLAGS          (1<<4)
#define PS_M_GRAVITY        (1<<5)
#define PS_M_DELTA_ANGLES   (1<<6)

#define PS_VIEWOFFSET       (1<<7)
#define PS_VIEWANGLES       (1<<8)
#define PS_KICKANGLES       (1<<9)
#define PS_BLEND            (1<<10)
#define PS_FOV              (1<<11)
//#define   PS_WEAPONINDEX      (1<<12)
//#define   PS_WEAPONFRAME      (1<<13)

#define PS_VIEWENTITY       (1<<12)
#define PS_INPUTENTITY      (1<<13)

#define PS_RDFLAGS          (1<<14)

//==============================================

// user_cmd_t communication

// ms and light allways sent, the others are optional
#define CM_ANGLE1   (1<<0)
#define CM_ANGLE2   (1<<1)
#define CM_ANGLE3   (1<<2)
#define CM_FORWARD  (1<<3)
#define CM_SIDE     (1<<4)
#define CM_UP       (1<<5)
#define CM_BUTTONS  (1<<6)
#define CM_IMPULSE  (1<<7)

//==============================================

// a sound without an ent or pos will be a local only sound
#define SND_VOLUME      (1<<0)      // a byte
#define SND_ATTENUATION (1<<1)      // a byte
#define SND_POS         (1<<2)      // three coordinates
#define SND_ENT         (1<<3)      // a short 0-2: channel, 3-12: entity
#define SND_OFFSET      (1<<4)      // a byte, msec offset from frame start
#define SND_DISTANCE	(1<<5)		// two bytes

#define DEFAULT_SOUND_PACKET_VOLUME 1.0
#define DEFAULT_SOUND_PACKET_ATTENUATION 1.0

//==============================================

// entity_state_t communication

#define DELTA_DONT_FORCE    0x0000
#define DELTA_FORCE         0x0001
#define DELTA_FORCE_FRAME   0x0002

// try to pack the common update flags into the first byte
#define U_ORIGIN1		(1<<0)
#define U_ORIGIN2		(1<<1)
#define U_ANGLE2		(1<<2)
#define U_ANGLE3		(1<<3)
#define U_FRAME8		(1<<4)      //  Nelno:  set if any frameInfo changes
						            //  then server will send down additional flags denoting
						            //  exactly which frame params have changed
#define U_EVENT			(1<<5)
#define	U_FREED			(1<<5)		//	entity was freed by the server.  Only sent down with a U_REMOVE
									//	so it can have the same value as U_EVENT
#define U_REMOVE		(1<<6)      //	REMOVE this entity, don't add it
#define U_MOREBITS1		(1<<7)      //	read one additional byte
						
// second byte			
#define U_NUMBER16		(1<<8)      // NUMBER8 is implicit if not set
#define U_ORIGIN3		(1<<9)
#define U_ANGLE1		(1<<10)
#define U_MODEL			(1<<11)
#define U_RENDERFX8		(1<<12)     // fullbright, etc
#define U_CLUSTERDATA	(1<<13)		// the cluster data on a model has changed
#define U_EFFECTS8		(1<<14)     // autorotate, trails, etc
#define U_MOREBITS2		(1<<15)     // read one additional byte
						
// third byte			
#define U_SKIN8			(1<<16)
#define U_FRAME16		(1<<17)     // frame is a short
#define U_RENDERFX16	(1<<18)		// 8 + 16 = 32
#define U_EFFECTS16		(1<<19)     // 8 + 16 = 32

#define U_MODEL2		(1<<20) // weapons, flags, etc		
#define U_MODEL3		(1<<21)
#define U_MODEL4		(1<<22)

#define U_MOREBITS3 (1<<23)     // read one additional byte

// fourth byte
#define U_OLDORIGIN (1<<24)     // FIXME: get rid of this
#define U_SKIN16    (1<<25)
#define U_SOUND     (1<<26)
#define U_SOLID     (1<<27)

#define U_RENDERX   (1<<28)     //  Nelno:  added bit for render_scale 

#define U_BBOX      (1<<29)     //  Nelno:  for accurate bbox clipping on client...

#define U_ALPHA     (1<<30)
#define U_MOREBITS4 (1<<31)

///////////////////////////////////////////////////////////////////////////////
//  All these are in the second dword... these should be the most seldom sent fields...
///////////////////////////////////////////////////////////////////////////////

#define U_MINSMAXS      (1<<0)  //  6 floats!  Should ONLY be sent down for EF_SNOW and EF_RAIN
                                //  and only on a level spawn!!
                                //  FIXME:  do this all client side!
#define U_HIERARCHICAL  (1<<1)  //  sent each time a player respawns???
#define U_BEAM_DIR      (1<<2)  //  for RF_BEAM_MOVING.  
                                //  FIXME:  Probably should not be here...
#define U_EFFECTS2_8    (1<<3)
#define U_EFFECTS2_16   (1<<4)

#define U_FLAGS_8       (1<<5)
#define U_FLAGS_16      (1<<6)

#define U_MOREBITS5     (1<<7)
#define U_MOREBITS6     (1<<15)

#define U_ANGLEDELTA_X  (1<<20)
#define U_ANGLEDELTA_Y  (1<<21)
#define U_ANGLEDELTA_Z  (1<<22)

#define U_MOREBITS7     (1<<23)

#define U_SOUNDINFO		(1<<24)		// aditional sound information
#define U_NUMCLUSTERS	(1<<25)	// the number of clusters on a model has changed
#define U_ORIGINHACK	(1<<26)		// origin hack used by sound engine for bmodels (namely doors)
#define U_COLOR			(1<<27)		

//  Nelno:  frameInfo_t communication
//  these are set in frame_sent_bits to let the client know exactly what was
//  sent down, and what the size of it is

//	FIXME: amw 5.3.99 - we could gain a small net optimization by sorting
//	these in the order that most commonly occur 

#define FRSENT_SYNC8            0x00000001  //  sent 8-bit frame number
#define FRSENT_SYNC16           0x00000002  //  sent 16-bit frame number
#define FRSENT_FLAGS16          0x00000004  //  sent 16-bit frameFlags
#define FRSENT_STARTFRAME8      0x00000008  //  sent 8 bit startFrame
#define FRSENT_STARTFRAME16     0x00000010  //  sent 16 bit startFrame
#define FRSENT_ENDFRAME8        0x00000020  //  sent 8 bit endFrame
#define FRSENT_ENDFRAME16       0x00000040  //  sent 16 bit endFrame
#define FRSENT_SOUND1FRAME8     0x00000080  //  sent 8 bit sound1Frame

#define FRSENT_SOUND1FRAME16    0x00000100  //  sent 16 bit sound1Frame
#define FRSENT_SOUND2FRAME8     0x00000200  //  sent 8 bit sound2Frame
#define FRSENT_SOUND2FRAME16    0x00000400  //  sent 16 bit sound2Frame
#define FRSENT_SOUND1INDEX8     0x00000800  //  sent 8 bit sound1Index
#define FRSENT_SOUND2INDEX8     0x00001000  //  sent 8 bit sound2Index
#define FRSENT_FRAMEINC8        0x00002000  //  sent 8 bit frameInc
#define FRSENT_STARTLOOP16      0x00004000  //  sent 16 bit startLoop
#define FRSENT_THIRDBYTE        0x00008000  //  set if sending next two 

#define FRSENT_ENDLOOP16        0x00010000  //  sent 16 bit endLoop
#define FRSENT_LOOPCOUNT8       0x00020000  //  sent 8 bit loopCount
#define FRSENT_ANIMSPEED		0x00040000	//  sent the model animation speed


////  hierarchical sent bits flags
//#define HRSENT_PARENTNUM8       0x0001  //  parent number changed
//#define HRSENT_PARENTNUM16      0x0002  //  parent number changed
//#define HRSENT_HRFLAGS8         0x0004  //  hrFlags changed
//#define HRSENT_HRFLAGS16        0x0008  //  hrFlags changed
//#define HRSENT_HRTYPE           0x0010  //  hrType changed
//#define HRSENT_SURFINDEX        0x0020  //  surfaceIndex changed
//#define HRSENT_VERTINDEX8       0x0040  //  vertexIndex changed
//#define HRSENT_VERTINDEX16      0x0080


// cluster sent bits flags

#define CDSENT_ALL_BITS			0x0001  //  send all bits for all clusters
#define CDSENT_ANGLES_0_X		0x0002  //  send angles for cluster 0 -	pitch
#define CDSENT_ANGLES_0_Y       0x0004  //  send angles for cluster 0 - yaw
#define CDSENT_ANGLES_0_Z		0x0008  //  send angles for cluster 0 - roll
#define CDSENT_ANGLES_1_X       0x0010  //  send angles for cluster 1 - pitch
#define CDSENT_ANGLES_1_Y		0x0020  //  send angles for cluster 1 - yaw
#define CDSENT_ANGLES_1_Z		0x0040  //  send angles for cluster 1 - roll
#define CDSENT_3RDCLUSTER		0x0080	//  this bit is set if there is a third cluster coming over

#define CDSENT_ANGLES_2_X		0x0001  //  send angles for cluster 0 -	pitch
#define CDSENT_ANGLES_2_Y       0x0002  //  send angles for cluster 1 - yaw
#define CDSENT_ANGLES_2_Z		0x0004  //  send angles for cluster 2 - roll
#define CDSENT_RESERVED			0x0008  //  


/*
==============================================================

CMD

Command text buffering and command execution

==============================================================
*/

/*

Any number of commands can be added in a frame, from several different sources.
Most commands come from either keybindings or console line input, but remote
servers can also send across commands and entire text files can be execed.

The + command line options are also added to the command buffer.

The game starts with a Cbuf_AddText ("exec quake.rc\n"); Cbuf_Execute ();

*/

#define EXEC_NOW    0       // don't return until completed
#define EXEC_INSERT 1       // insert at current position, but don't run yet
#define EXEC_APPEND 2       // add to end of the command buffer

void Cbuf_Init();
// allocates an initial text buffer that will grow as needed

void Cbuf_AddText (const char *text);
// as new commands are generated from the console or keybindings,
// the text is added to the end of the command buffer.

void Cbuf_InsertText (char *text);
// when a command wants to issue other commands immediately, the text is
// inserted at the beginning of the buffer, before any remaining unexecuted
// commands.

void Cbuf_ExecuteText (int exec_when, char *text);
// this can be used in place of either Cbuf_AddText or Cbuf_InsertText

void Cbuf_AddEarlyCommands (qboolean clear);
// adds all the +set commands from the command line

qboolean Cbuf_AddLateCommands();
// adds all the remaining + commands from the command line
// Returns true if any late commands were added, which
// will keep the demoloop from immediately starting

void Cbuf_Execute();
// Pulls off \n terminated lines of text from the command buffer and sends
// them through Cmd_ExecuteString.  Stops when the buffer is empty.
// Normally called once per frame, but may be explicitly invoked.
// Do not call inside a command function!

void Cbuf_CopyToDefer();
void Cbuf_InsertFromDefer();
// These two functions are used to defer any pending commands while a map
// is being loaded

//===========================================================================

/*

Command execution takes a null terminated string, breaks it into tokens,
then searches for a command or variable that matches the first token.

*/

typedef void (*xcommand_t)();

void    Cmd_Init();

void    Cmd_AddCommand (char *cmd_name, xcommand_t function);

//  Nelno: added these to get rid of ClientCommand crap
void    Cmd_RemoveDLLCommands();
void    Cmd_AddCommandDLL (char *cmd_name, void (*function) (struct edict_s *ent));
int     Cmd_ExecuteDLLCommand (char *text, struct edict_s *ent);
//  End of nelnonic modifications

void	Cmd_RemoveAllCommands();	// ensure we free all the command memory

// called by the init functions of other parts of the program to
// register commands and functions to call for them.
// The cmd_name is referenced later, so it should not be in temp memory
// if function is NULL, the command will be forwarded to the server
// as a clc_stringcmd instead of executed locally
void    Cmd_RemoveCommand (char *cmd_name);

qboolean Cmd_Exists (char *cmd_name);
// used by the cvar code to check for cvar / command name overlap

char    *Cmd_CompleteCommand (char *partial);
// attempts to match a partial command for automatic command line completion
// returns NULL if nothing fits

int     GetArgc();
char    *GetArgv(int arg);
char    *GetArgs();
// The functions that execute commands get their parameters with these
// functions. Cmd_Argv () will return an empty string, not a NULL
// if arg > argc, so string operations are allways safe.

void    Cmd_TokenizeString (const char *text, qboolean macroExpand);
// Takes a null terminated string.  Does not need to be /n terminated.
// breaks the string up into arg tokens.

void    Cmd_ExecuteString (const char *text);
// Parses a single line of text into arguments and tries to execute it
// as if it was typed at the console

void    Cmd_ForwardToServer();
// adds the current command line as a clc_stringcmd to the client message.
// things like godmode, noclip, etc, are commands directed to the server,
// so when they are typed in at the console, they will need to be forwarded.

/*
==============================================================

CVAR

==============================================================
*/

/*

cvar_t variables are used to hold scalar or string variables that can be changed or displayed at the console or prog code as well as accessed directly
in C code.

The user can access cvars from the console in three ways:
r_draworder         prints the current value
r_draworder 0       sets the current value to 0
set r_draworder 0   as above, but creates the cvar if not present
Cvars are restricted from having the same names as commands to keep this
interface from being ambiguous.
*/

extern  cvar_t  *cvar_vars;

cvar_t *Cvar_Get (const char *var_name, const char *value, int flags);
// creates the variable if it doesn't exist, or returns the existing one
// if it exists, the value will not be changed, but flags will be ORed in
// that allows variables to be unarchived without needing bitflags

cvar_t  *Cvar_Set (const char *var_name, const char *value);
// will create the variable if it doesn't exist

cvar_t *Cvar_ForceSet (const char *var_name, const char *value);
// will set the variable even if NOSET or LATCH

cvar_t  *Cvar_FullSet (const char *var_name, const char *value, int flags);

void    Cvar_SetValue (const char *var_name, float value);
// expands value to a string and calls Cvar_Set

float   Cvar_VariableValue (const char *var_name);
// returns 0 if not defined or non numeric

char    *Cvar_VariableString (const char *var_name);
// returns an empty string if not defined

char    *Cvar_CompleteVariable (const char *partial);
// attempts to match a partial variable name for command line completion
// returns NULL if nothing fits

void    Cvar_GetLatchedVars();
// any CVAR_LATCHED variables that have been set will now take effect

qboolean Cvar_Command();
// called by Cmd_ExecuteString when Cmd_Argv(0) doesn't match a known
// command.  Returns true if the command was a variable reference that
// was handled. (print or change)

void    Cvar_WriteVariables (char *path);
// appends lines containing "set variable value" for all variables
// with the archive flag set to true.

void    Cvar_Init();

char    *Cvar_Userinfo();
// returns an info string containing all the CVAR_USERINFO cvars

char    *Cvar_Serverinfo();
// returns an info string containing all the CVAR_SERVERINFO cvars

extern  qboolean    userinfo_modified;
// this is set each time a CVAR_USERINFO variable is changed
// so that the client knows to send it to the server

/*
==============================================================

NET

==============================================================
*/

// net.h -- quake's interface to the networking layer

#define PORT_ANY    -1

#define MAX_MSGLEN      2800        // max length of a message
#define PACKET_HEADER   10          // two ints and a short

typedef enum {NA_LOOPBACK, NA_BROADCAST, NA_IP, NA_IPX, NA_BROADCAST_IPX} netadrtype_t;

typedef enum {NS_CLIENT, NS_SERVER} netsrc_t;

typedef struct
{
    netadrtype_t    type;

    byte    ip[4];
    byte    ipx[10];

    unsigned short  port;
} netadr_t;

void        NET_Init();
void        NET_Shutdown();

void        NET_Config (qboolean multiplayer);

qboolean    NET_GetPacket (netsrc_t sock, netadr_t *net_from, sizebuf_t *net_message);
void        NET_SendPacket (netsrc_t sock, int length, void *data, netadr_t to);

qboolean    NET_CompareAdr (netadr_t a, netadr_t b);
qboolean    NET_CompareBaseAdr (netadr_t a, netadr_t b);
qboolean    NET_IsLocalAddress (netadr_t adr);
char        *NET_AdrToString (netadr_t a);
qboolean    NET_StringToAdr (char *s, netadr_t *a);


//============================================================================

#define OLD_AVG     0.99        // total = oldtotal*OLD_AVG + new*(1-OLD_AVG)

#define MAX_LATENT  32

typedef struct
{
    qboolean    fatal_error;

    netsrc_t    sock;

    int         dropped;            // between last packet and previous

    int         last_received;      // for timeouts
    int         last_sent;          // for retransmits

    netadr_t    remote_address;
    int         qport;              // qport value to write when transmitting

// sequencing variables
    int         incoming_sequence;
    int         incoming_acknowledged;
    int         incoming_reliable_acknowledged; // single bit

    int         incoming_reliable_sequence;     // single bit, maintained local

    int         outgoing_sequence;
    int         reliable_sequence;          // single bit
    int         last_reliable_sequence;     // sequence number of last send

// reliable staging and holding areas
    sizebuf_t   message;        // writing buffer to send to server
    byte        message_buf[MAX_MSGLEN-16];     // leave space for header

// message is copied to this buffer when it is first transfered
    int         reliable_length;
    byte        reliable_buf[MAX_MSGLEN-16];    // unacked reliable message
} netchan_t;

extern  netadr_t    net_from;
extern  sizebuf_t   net_message;
extern  byte        net_message_buffer[MAX_MSGLEN];


void Netchan_Init();
void Netchan_Setup (netsrc_t sock, netchan_t *chan, netadr_t adr, int qport);

void Netchan_Transmit (netchan_t *chan, int length, byte *data);
void Netchan_OutOfBand (int net_socket, netadr_t adr, int length, byte *data);
void Netchan_OutOfBandPrint (int net_socket, netadr_t adr, char *format, ...);
qboolean Netchan_Process (netchan_t *chan, sizebuf_t *msg);

qboolean Netchan_CanReliable (netchan_t *chan);


/*
==============================================================

CMODEL

==============================================================
*/


#include "qfiles.h"

extern  userEpair_t *map_epairs;

int         CM_EpairsForEdict (const char **data, userEpair_t **epair);
void        CM_FreeEpairs (userEpair_t *epair);
char        *CM_KeyValue (userEpair_t *epair, char *key);
userEpair_t *CM_EpairsForClass (byte *buffer, char *className);
cmodel_t    *CM_LoadMap (char *name, qboolean clientload, unsigned *checksum);
cmodel_t    *CM_InlineModel (char *name);   // *1, *2, etc
void        CM_GetPaletteDirectory (char *name, char *palette);

int         CM_NumClusters();
int         CM_NumInlineModels();
char        *CM_EntityString();

// creates a clipping hull for an arbitrary box
int         CM_HeadnodeForBox (CVector &mins, CVector &maxs);


// returns an ORed contents mask
int         CM_PointContents (CVector &p, int headnode);
int         CM_TransformedPointContents (CVector &p, int headnode, CVector &origin, CVector &angles);

trace_t     CM_BoxTrace (CVector &start, CVector &end,
                          CVector &mins, CVector &maxs,
                          int headnode, int brushmask);
trace_t     CM_TransformedBoxTrace (CVector &start, CVector &end,
                          CVector &mins, CVector &maxs,
                          int headnode, int brushmask,
                          CVector &origin, CVector &angles);

byte        *CM_ClusterPVS (int cluster);
byte        *CM_ClusterPHS (int cluster);

int         CM_PointLeafnum (CVector& p);

// call with topnode set to the headnode, returns with topnode
// set to the first node that splits the box
int         CM_BoxLeafnums (CVector &mins, CVector &maxs, int *list,
                            int listsize, int *topnode);

int         CM_LeafContents (int leafnum);
int         CM_LeafCluster (int leafnum);
int         CM_LeafArea (int leafnum);

void        CM_SetAreaPortalState (int portalnum, qboolean open);
qboolean    CM_AreasConnected (int area1, int area2);

int         CM_WriteAreaBits (byte *buffer, int area);
qboolean    CM_HeadnodeVisible (int headnode, byte *visbits);

void        CM_WritePortalState (FILE *f);
void        CM_ReadPortalState (FILE *f);

qboolean	CM_inPVS( CVector &p1, CVector &p2 );

/*
==============================================================

PLAYER MOVEMENT CODE

Common between server and client so prediction matches

==============================================================
*/

void Pmove (pmove_t *pmove);

/*
==============================================================

FILESYSTEM

==============================================================
*/


void        FS_InitFilesystem();
void        FS_SetGamedir (char *dir);
char        *FS_Gamedir();
char		*FS_SaveGameDir(void);
char		*FS_ConfigDir();
char        *FS_NextPath (char *prevpath);
void        FS_ExecAutoexec();

int         FS_filelength (FILE *f);
int         FS_FOpenFile (const char *filename, FILE **file);
int         FS_FOpenTextFile (const char *filename, FILE **file);
void        FS_FCloseFile (FILE *f);
// note: this can't be called from another DLL, due to MS libc issues

int         FS_LoadFile (const char *path, void **buffer);
// a null buffer will just return the file length without loading
// a -1 length is not present

void        FS_Read (void *buffer, int len, FILE *f);
// properly handles partial reads

void        FS_FreeFile (void *buffer);

void        FS_CreatePath (char *path);

int         FS_Seek (FILE *f, long offset, int origin);
long        FS_Tell (FILE *f);
int         FS_Getc (FILE *f);
int			FS_Ungetc( int c, FILE *f );
char		*FS_Gets( char *string, int n, FILE *f );

int			FS_GetFilesOpened();
long		FS_GetBytesRead();

/*
==============================================================

MISC

==============================================================
*/


#define ERR_FATAL   0       // exit the entire game with a popup window
#define ERR_DROP    1       // print to console and disconnect from game
#define ERR_QUIT    2       // not an error, just a normal exit

#define EXEC_NOW    0       // don't return until completed
#define EXEC_INSERT 1       // insert at current position, but don't run yet
#define EXEC_APPEND 2       // add to end of the command buffer

#define PRINT_ALL       0
#define PRINT_DEVELOPER 1   // only print when "developer 1"

void        Com_BeginRedirect (int target, char *buffer, int buffersize, void (*rd_flush)(int target, char *buffer));
void        Com_EndRedirect();

void        Com_Printf (char *fmt, ...);
void        Com_DPrintf (char *fmt, ...);
void        Com_Error (int code, char *fmt, ...);
void        Com_Quit();
int         Com_ServerState();      // this should have just been a cvar...
void        Com_SetServerState (int state);

unsigned    Com_BlockChecksum (void *buffer, int length);
byte        COM_BlockSequenceCheckByte (byte *base, int length, int sequence);

float   frand();    // 0 ti 1
float   crand();    // -1 to 1

extern  cvar_t  *showsync;
extern  cvar_t  *developer;
extern  cvar_t  *dedicated;
extern  cvar_t  *host_speeds;
extern  cvar_t  *log_stats;

extern	cvar_t	*sv_frictionfactor;

//  Nelno:
extern  cvar_t  *dk_rates;
extern  cvar_t* cvarLogFilePath;

extern  FILE *log_stats_file;

// host_speeds times
extern  int     time_before_game;
extern  int     time_after_game;
extern  int     time_before_ref;
extern  int     time_after_ref;

#define	TAG_CLIENT	999
#define	TAG_BSP		1024	

void Qcommon_Init (int argc, char **argv);
void Qcommon_Frame (int msec);
void Qcommon_Shutdown();

// this is in the client code, but can be used for debugging from server
void SCR_DebugGraph (float value, int color);

/*
==============================================================


==============================================================
*/

void	Sys_Init();

void	Sys_AppActivate();
void	Sys_Minimize(void);

//void	Sys_UnloadGame();
void	Sys_UnloadPhysicsDLL();
void	*Sys_GetGameAPI (void *parms);
// loads the game dll and calls the api init function

char	*Sys_ConsoleInput();
void	Sys_ConsoleOutput (char *string);
void	Sys_SendKeyEvents();
void	Sys_Error (char *error, ...);
void	Sys_Quit();
char	*Sys_GetClipboardData();
void	Sys_CopyProtect();

/*
==============================================================

CLIENT / SERVER SYSTEMS

==============================================================
*/

extern int   progress_index;

void CL_Init();
void CL_Drop();
void CL_Shutdown();
void CL_Frame (int msec);
void SCR_BeginLoadingPlaque();
void SCR_UpdateScreen();

void SV_Init();
void SV_Shutdown (const char *finalmsg, qboolean reconnect, qboolean unload_dlls);
void SV_Frame (int msec);
#endif