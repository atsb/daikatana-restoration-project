// common.c -- misc functions used in client and server
#if _MSC_VER
#include "io.h"
#endif

#include "qcommon.h"
#include "console.h"

//	Nelno: include file for Daikatana specific stuff
#include "daikatana.h"
#include "vid.h"		//	Nelno:	yeah, I know this sucks

// added for Japanese version
#ifdef JPN
#include <windows.h>
#endif // JPN

#include <setjmp.h>


//#define PNT(x, y, z) CVector(x, y, z)

#define	MAXPRINTMSG	4096

#define MAX_NUM_ARGVS	50

#include "vertnormals.h"
CVertexNormals avertexnormals;


//	Nelno
char	*gamename;
char	*debugdir;

int		com_argc;
char	*com_argv[MAX_NUM_ARGVS+1];

int		realtime;

jmp_buf abortframe;		// an ERR_DROP occured, exit the entire frame


FILE	*log_stats_file;

//	Nelno
cvar_t	*dk_rates;

cvar_t	*host_speeds;
cvar_t	*log_stats;
cvar_t	*developer;
cvar_t	*console;
cvar_t	*timescale;
cvar_t	*fixedtime;
cvar_t	*logfile_active;	// 1 = buffer log, 2 = flush after each print
cvar_t	*showtrace;
cvar_t	*dedicated;
cvar_t	*showsync;	//	Nelno:	client-side animation debugging

// NETWORK LOGGING
// cvar_t   *g_cvarLogServer;

FILE	*logfile;



int		server_state;

// host_speeds times
int		time_before_game;
int		time_after_game;
int		time_before_ref;
int		time_after_ref;

int		progress_index;

//-----------------------------------------------------------------------------

#ifdef BUILD_PATH_TABLE


#define MAX_NUMBER_PATHTABLE_MAPS   100

static int nNumMaps = 0;
static int nMapCount = 0;
static char aMaps[MAX_NUMBER_PATHTABLE_MAPS][64];


static void PATHTABLE_AddMap( const char *szMap )
{
    strcpy( aMaps[nNumMaps], szMap );
    nNumMaps++;
}

static char *PATHTABLE_GetNextMap()
{
    char *szMap = NULL;
    if ( nMapCount < nNumMaps )
    {
        szMap = aMaps[nMapCount++];
    }

    return szMap;
}

static int PATHTABLE_ParsePathFile( const char *fileName )
{
	char szPath[128];
	strcpy( szPath, "./data/" );
	strcat( szPath, fileName );
	FILE *file = fopen( szPath, "rb" );
	if ( !file )
	{
		return FALSE;
	}

	char szLine[128];
    while ( 1 )
	{
		if ( fgets( szLine, 128, file ) == NULL )
		{
			break;
		}

		// skip blank lines
		if ( szLine[0] != '\n' && szLine[0] != '\r' )
		{
		    char szString[32];
            int nRetValue = sscanf( szLine, "%s", szString );
            if ( nRetValue > 0 )
            {
                PATHTABLE_AddMap( szString );
            }
		}
	}

    fclose( file );
	return TRUE;
}

int PATHTABLE_ExecuteNextMap()
{
    extern int SV_LevelExists( char *map );
    extern void SV_Map( qboolean attractloop, char *levelstring, qboolean loadgame );
    extern void SV_ChangeLevel( qboolean attractloop, char *levelstring, qboolean loadgame );

    int bNewMap = FALSE;
    if ( nMapCount == 0 )
    {
        bNewMap = TRUE;
    }

    char *szMap = PATHTABLE_GetNextMap();
	if ( !szMap )
    {
        return FALSE;
    }

    if ( !SV_LevelExists( szMap ) )
	{
    	return TRUE;
    }

    if ( bNewMap )
    {
        // start up the next map
        SV_Map( false, szMap, false );
    }
    else
    {
        SV_ChangeLevel( false, szMap, false );
    }

//    char szBuff[128];
//    sprintf( szBuff, "Executing %s.\n", szMap );
//    OutputDebugString( szBuff );

    return TRUE;
}



#include "dk_thread.h"

CThreadSync pathTableThread;

void PATHTABLE_StartThread()
{
    pathTableThread.Init( "PATHTABLEFILE", "PATHTABLEOBJECT" );
    pathTableThread.CreateFileMap_OpenRead_CreateEvent();
}

int PATHTABLE_GetThreadData()
{
    // return immediately if there are no maps loaded
    if ( nMapCount == 0 )
    {
        return 1;
    }

    if ( pathTableThread.WaitToRead(10) )
    {
        char szData[128];
        pathTableThread.ReadSharedData( szData );

		// Read from the shared data.
		int nValue = atoi( szData );

        pathTableThread.DoneReading();

        return nValue;
    }

    return 0;
}

#endif BUILD_PATH_TABLE



//-----------------------------------------------------------------------------

/*
============================================================================

CLIENT / SERVER interactions

============================================================================
*/

static int	rd_target;
static char	*rd_buffer;
static int	rd_buffersize;
static void	(*rd_flush)(int target, char *buffer);
void Com_BeginRedirect (int target, char *buffer, int buffersize, void (*flush)(int target, char *buffer))
{
	if (!target || !buffer || !buffersize || !flush)
		return;
	rd_target = target;
	rd_buffer = buffer;
	rd_buffersize = buffersize;
	rd_flush = flush;

	*rd_buffer = 0;
}

void Com_EndRedirect (void)
{
	rd_flush(rd_target, rd_buffer);

	rd_target = 0;
	rd_buffer = NULL;
	rd_buffersize = 0;
	rd_flush = NULL;
}


/*
=============
Com_Printf

Both client and server can use this, and it will output
to the apropriate place.
=============
*/
void Com_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	if (rd_target)
	{
		if ((strlen (msg) + strlen(rd_buffer)) > (rd_buffersize - 1))
		{
			rd_flush(rd_target, rd_buffer);
			*rd_buffer = 0;
		}
		strcat (rd_buffer, msg);
		return;
	}

	Con_Print (msg);
		
	// also echo to debugging console
	Sys_ConsoleOutput (msg);

	// logfile
	if (logfile_active && logfile_active->value)
	{
		char	name[MAX_QPATH];
		
		if (!logfile)
		{
			Com_sprintf (name, sizeof(name), "%s/dk_console.log", FS_Gamedir ());
			logfile = fopen (name, "w");
		}
		if (logfile)
			fprintf (logfile, "%s", msg);
		if (logfile_active->value > 1)
			fflush (logfile);		// force it to save every time
	}
}

/*
================
Com_DPrintf

A Com_Printf that only shows up if the "developer" cvar is set
================
*/
void Com_DPrintf (char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];
		
	if (!developer || !developer->value)
		return;			// don't confuse non-developers with techie stuff...

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);
	
	Com_Printf ("%s", msg);
}



/*
=============
Com_Error

Both client and server can use this, and it will
do the apropriate things.
=============
*/
void Com_Error (int code, char *fmt, ...)
{
	va_list		argptr;
	static char		msg[MAXPRINTMSG];
	static	qboolean	recursive;

	if (recursive)
		Sys_Error ("recursive error after: %s", msg);
	recursive = true;

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);
	
//#if !DEDICATED_ONLY
	//if (con.mode == CON_LOADING)
	//	con.mode = CON_NORMAL;
//#endif

	if (code == ERR_DISCONNECT)
	{
		CL_Drop ();
		recursive = false;
		longjmp (abortframe, -1);
	}
	else if (code == ERR_DROP)
	{
		Com_Printf ("********************\nERROR: %s\n********************\n", msg);
		SV_Shutdown (va("Server crashed: %s\n", msg), false, SV_UNLOAD_DLLS);
		CL_Drop ();
		recursive = false;
		longjmp (abortframe, -1);
	}
	else
	{
		SV_Shutdown (va("Server fatal crashed: %s\n", msg), false, SV_UNLOAD_DLLS);
		CL_Shutdown ();
	}

	if (logfile)
	{
		fclose (logfile);
		logfile = NULL;
	}

	Sys_Error ("%s", msg);
}


/*
=============
Com_Quit

Both client and server can use this, and it will
do the apropriate things.
=============
*/
void Com_Quit (void)
{
	SV_Shutdown ("Server quit\n", false, SV_UNLOAD_DLLS);
	CL_Shutdown ();

	Cmd_RemoveAllCommands();

	//DKLOG_Close();

	if (logfile)
	{
		fclose (logfile);
		logfile = NULL;
	}

	Sys_Quit ();
}


/*
==================
Com_ServerState
==================
*/
int Com_ServerState (void)
{
	return server_state;
}

/*
==================
Com_SetServerState
==================
*/
void Com_SetServerState (int state)
{
	server_state = state;
}

// command to tell memory manager to print stuff
void Mem_Stat_f (void)
{
	Mem_Heap_Walk(1);
}

// compact the memory
void Mem_Compact_f(void)
{
	Mem_Compact(1);
}

// do a memory dump
void Mem_Dump_f(void)
{
	Mem_Dump();
}

// inc sequence
void Mem_Seq_f(void)
{
	Mem_Inc_Sequence();
}

void Mem_Total_Long_f()
{
	Mem_Dump_Total(true);
}

void Mem_Total_Short_f()
{
	Mem_Dump_Total(false);
}

/*
==============================================================================

			MESSAGE IO FUNCTIONS

Handles byte ordering and avoids alignment errors
==============================================================================
*/



//
// writing functions
//

void MSG_WriteChar (sizebuf_t *sb, int c)
{
	byte	*buf;
	
#ifdef PARANOID
	if (c < -128 || c > 127)
		Com_Error (ERR_FATAL, "MSG_WriteChar: range error");
#endif

	buf = (byte *)SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteByte (sizebuf_t *sb, int c)
{
	byte	*buf;
	
#ifdef PARANOID
	if (c < 0 || c > 255)
		Com_Error (ERR_FATAL, "MSG_WriteByte: range error");
#endif

	buf = (byte *)SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteShort (sizebuf_t *sb, int c)
{
	byte	*buf;
	
#ifdef PARANOID
	if (c < ((short)0x8000) || c > (short)0x7fff)
		Com_Error (ERR_FATAL, "MSG_WriteShort: range error");
#endif

	buf = (byte *)SZ_GetSpace (sb, 2);
	buf[0] = c&0xff;
	buf[1] = c>>8;
}

void MSG_WriteLong (sizebuf_t *sb, int c)
{
	byte	*buf;
	
	buf = (byte *)SZ_GetSpace (sb, 4);
	buf[0] = c&0xff;
	buf[1] = (c>>8)&0xff;
	buf[2] = (c>>16)&0xff;
	buf[3] = c>>24;
}

void MSG_WriteFloat (sizebuf_t *sb, float f)
{
	union
	{
		float	f;
		int	l;
	} dat;
	
	
	dat.f = f;
	dat.l = LittleLong (dat.l);
	
	SZ_Write (sb, &dat.l, 4);
}

void MSG_WriteString (sizebuf_t *sb, const char *s)
{
	if (!s)
		SZ_Write (sb, "", 1);
	else
		SZ_Write (sb, s, strlen(s)+1);
}

void MSG_WriteCoord (sizebuf_t *sb, float f)
{
	MSG_WriteShort (sb, (int)(f*8));
}

void MSG_WritePos (sizebuf_t *sb, CVector &pos)
{
	MSG_WriteShort (sb, (int)(pos.x*8));
	MSG_WriteShort (sb, (int)(pos.y*8));
	MSG_WriteShort (sb, (int)(pos.z*8));
}

void MSG_WriteAngle (sizebuf_t *sb, float f)
{
	MSG_WriteByte (sb, (int)(f*256/360) & 255);
}

void MSG_WriteAngle16 (sizebuf_t *sb, float f)
{
	MSG_WriteShort (sb, ANGLE2SHORT(f));
}

void MSG_WriteDeltaUsercmd (sizebuf_t *buf, usercmd_t *from, usercmd_t *cmd)
{
	int		bits;
//
// send the movement message
//
	bits = 0;
	if (cmd->angles[0] != from->angles[0])
		bits |= CM_ANGLE1;
	if (cmd->angles[1] != from->angles[1])
		bits |= CM_ANGLE2;
	if (cmd->angles[2] != from->angles[2])
		bits |= CM_ANGLE3;
	if (cmd->forwardmove != from->forwardmove)
		bits |= CM_FORWARD;
	if (cmd->sidemove != from->sidemove)
		bits |= CM_SIDE;
	if (cmd->upmove != from->upmove)
		bits |= CM_UP;
	if (cmd->buttons != from->buttons)
		bits |= CM_BUTTONS;
	if (cmd->impulse != from->impulse)
		bits |= CM_IMPULSE;
    MSG_WriteByte (buf, bits);
	if (bits & CM_ANGLE1)
		MSG_WriteShort (buf, cmd->angles[0]);
	if (bits & CM_ANGLE2)
		MSG_WriteShort (buf, cmd->angles[1]);
	if (bits & CM_ANGLE3)
		MSG_WriteShort (buf, cmd->angles[2]);
	
	if (bits & CM_FORWARD)
		MSG_WriteShort (buf, cmd->forwardmove);
	if (bits & CM_SIDE)
	  	MSG_WriteShort (buf, cmd->sidemove);
	if (bits & CM_UP)
		MSG_WriteShort (buf, cmd->upmove);
 	if (bits & CM_BUTTONS)
	  	MSG_WriteByte (buf, cmd->buttons);
 	if (bits & CM_IMPULSE)
	    MSG_WriteByte (buf, cmd->impulse);
    MSG_WriteByte (buf, cmd->msec);
	MSG_WriteByte (buf, cmd->lightlevel);
}

void MSG_WriteDir (sizebuf_t *sb, CVector &dir)
{
	int best;
	
	if (!dir)
	{
		MSG_WriteByte (sb, 0);
		return;
	}

	// if there are no direction angles, don't go through all the bullshit
	if (!dir.x && !dir.y && !dir.z)
		best = NUMVERTEXNORMALS;
	else
        best = avertexnormals.GetClosestVertNormal(dir);

    MSG_WriteByte (sb, best);
}


void MSG_ReadDir (sizebuf_t *sb, CVector &dir)
{
	int		b;

	b = MSG_ReadByte (sb);

	// changed so that anything equal to or above NUMVERTEXNORMALS is set to (0,0,0)
	if (b >= NUMVERTEXNORMALS)
	{
//		Com_Error (ERR_DROP, "MSF_ReadDir: out of range");
		dir.Zero();
	}
	else
	{
		dir = avertexnormals.GetNormal(b);
	}
}


/*
==================
MSG_WriteDeltaEntity

Writes part of a packetentities message.
Can delta from either a baseline or a previous packet_entity
==================
*/

void MSG_WriteDeltaEntity (entity_state_t *from, entity_state_t *to, sizebuf_t *msg, int force)
{
	unsigned int		bits, bits2, cluster_bits, cluster_bits2;
	unsigned int		frame_sent_bits, sync;

	if (!to->number)
		Com_Error (ERR_FATAL, "Unset entity number");
	if (to->number >= MAX_EDICTS)
		Com_Error (ERR_FATAL, "Entity number >= MAX_EDICTS");

// send an update
	/*hr_sent_bits = */frame_sent_bits = sync = cluster_bits = cluster_bits2 = bits2 = bits = 0;

	// WAW[11/29/99]: This is causing A LARGE AMOUNT of data to be sent!!!
	// moved the code to after the bits are check.
	//if (to->number >= 256)
	//	bits |= U_NUMBER16;		// number8 is implicit otherwise

	if (to->origin.x != from->origin.x)
		bits |= U_ORIGIN1;
	if (to->origin.y != from->origin.y)
		bits |= U_ORIGIN2;
	if (to->origin.z != from->origin.z)
		bits |= U_ORIGIN3;

	if ( to->angles.x != from->angles.x )
		bits |= U_ANGLE1;		
	if ( to->angles.y != from->angles.y )
		bits |= U_ANGLE2;
	if ( to->angles.z != from->angles.z )
		bits |= U_ANGLE3;
		
	if ( to->skinnum != from->skinnum )
	{
		if ((unsigned)to->skinnum < 256)
			bits |= U_SKIN8;
		else if ((unsigned)to->skinnum < 0x10000)
			bits |= U_SKIN16;
		else
			bits |= (U_SKIN8|U_SKIN16);
	}

/*	*** NUKE THIS SOON ***
	if (to->entityID != from->entityID)
	{
		bits |= U_ENTITY_ID;
	}
*/
	///////////////////////////////////////////////////////////////////////////
	//	Nelno:	client now does animation, so don't send down frame changes from
	//	server to client unless certain parts of the frameInfo_t structure have
	//	changed.
	///////////////////////////////////////////////////////////////////////////

	//	make sure frameTime is reflected in flags
	to->frameInfo.frameFlags &= ~(FRAME_FPS10 | FRAME_FPS30);	// clear flags
	if (to->frameInfo.frameTime >= FRAMETIME_FPS10)
	{
		to->frameInfo.frameFlags |= FRAME_FPS10;
	}
	else if (to->frameInfo.frameTime <= FRAMETIME_FPS30 && to->frameInfo.frameTime > 0.0)
	{
		to->frameInfo.frameFlags |= FRAME_FPS30;
	}

	if (to->frameInfo.frameFlags & FRAME_FORCEINDEX)
	{
//		if (to->number == showsync->value)
//			Com_Printf ("FORCEINDEX is set!\n");
		//	FRAME_FORCEINDEX is true, so send frame down no matter what
		sync = true;
		to->frameInfo.frameFlags -= FRAME_FORCEINDEX;
	}
/*
	else if (to->frameInfo.frameFlags & FRAME_ONCE && !(to->frameInfo.frameState & FRSTATE_STARTANIM) &&
      ((to->frameInfo.frameState & FRSTATE_STOPPED || to->frameInfo.frameState & FRSTATE_LAST) ||
		(to->frame == to->frameInfo.startFrame)))// || to->frame == to->frameInfo.endFrame)))
	{
		//	sync frame if this is a FRAME_ONCE and we're restarting the
		//	same sequence
		sync = true;

		if (to->number == showsync->value)
      {
			Com_Printf ("FRAMEONCE forced sync ");
         if (to->frameInfo.frameState & FRSTATE_STOPPED)
            Com_Printf ("because FRSTATE_STOPPED\n");
         if (to->frameInfo.frameState & FRSTATE_LAST)
            Com_Printf ("because FRSTATE_STOPPED\n");
         if (to->frameInfo.startFrame == to->frame)
            Com_Printf ("because startFrame\n");
         if (to->frameInfo.endFrame == to->frame)
            Com_Printf ("because endFrame\n");
      }
	}
*/
	if (to->frameInfo.frameFlags != from->frameInfo.frameFlags)
	{
//		if (to->number == showsync->value)
//			Com_Printf ("frameFlags NOT equal!\n");
		frame_sent_bits |= FRSENT_FLAGS16;
		sync = true;
	}	
	if (to->frameInfo.startFrame != from->frameInfo.startFrame)
	{
		if (to->frameInfo.startFrame & 0xff00)
			frame_sent_bits |= FRSENT_STARTFRAME16;
		else
			frame_sent_bits |= FRSENT_STARTFRAME8;

//		if (to->number == showsync->value)
//			Com_Printf ("StartFrame changed!\n");

		sync = true;
	}
	if (to->frameInfo.endFrame != from->frameInfo.endFrame)
	{
		if (to->frameInfo.endFrame & 0xff00)
			frame_sent_bits |= FRSENT_ENDFRAME16;
		else
			frame_sent_bits |= FRSENT_ENDFRAME8;

//		if (to->number == showsync->value)
//			Com_Printf ("EndFrame changed!\n");

		sync = true;
	}
	if (to->frameInfo.startLoop != from->frameInfo.startLoop)
	{
		frame_sent_bits |= FRSENT_STARTLOOP16;
		sync = true;

//		if (to->number == showsync->value)
//			Com_Printf ("StartLoop changed!\n");

	}
	if (to->frameInfo.endLoop != from->frameInfo.endLoop)
	{
		frame_sent_bits |= FRSENT_ENDLOOP16;
		sync = true;

//		if (to->number == showsync->value)
//			Com_Printf ("EndLoop changed!\n");

	}
	if (to->frameInfo.loopCount != from->frameInfo.loopCount)
	{
		frame_sent_bits |= FRSENT_LOOPCOUNT8;
	}
	if (to->frameInfo.sound1Frame != from->frameInfo.sound1Frame)
	{
		if (to->frameInfo.sound1Frame & 0xff00)
			frame_sent_bits |= FRSENT_SOUND1FRAME16;
		else
			frame_sent_bits |= FRSENT_SOUND1FRAME8;
	}
	if (to->frameInfo.sound2Frame != from->frameInfo.sound2Frame)
	{
		if (to->frameInfo.sound2Frame & 0xff00)
			frame_sent_bits |= FRSENT_SOUND2FRAME16;
		else
			frame_sent_bits |= FRSENT_SOUND2FRAME8;
	}
	if (to->frameInfo.sound1Index != from->frameInfo.sound1Index)
	{
		frame_sent_bits |= FRSENT_SOUND1INDEX8;
	}
	if (to->frameInfo.sound2Index != from->frameInfo.sound2Index)
	{
		frame_sent_bits |= FRSENT_SOUND2INDEX8;
	}
	if (to->frameInfo.frameInc != from->frameInfo.frameInc)
	{
		frame_sent_bits |= FRSENT_FRAMEINC8;
	}

	// amw: 5.3.99 - steve's anti-skating values...
	if (to->frameInfo.modelAnimSpeed != from->frameInfo.modelAnimSpeed)
	{
		frame_sent_bits |= FRSENT_ANIMSPEED;
	}

	//	force frame syncing if level has just started because client
	//	can get a few frames in before physics starts running
	if (sync || force & DELTA_FORCE_FRAME)
	{
		if (to->frame & 0xff00)
			frame_sent_bits |= FRSENT_SYNC16;
		else
			frame_sent_bits |= FRSENT_SYNC8;
	}

	if (frame_sent_bits != 0)
	{
		//	tell the client we're going to send down some frame info
		if (frame_sent_bits & 0xffff00)
		{
			if (frame_sent_bits & 0xff0000)
				//	send down short int + 1 byte
				frame_sent_bits |= FRSENT_THIRDBYTE;
			
			//	frame_bits will send down a short int
			bits |= U_FRAME16;
		}
		else
			//	frame bits will send down a single byte
			bits |= U_FRAME8;
	}

	///////////////////////////////////////////////////////////////////////////
	//
	///////////////////////////////////////////////////////////////////////////

	if ( to->effects != from->effects )
	{
		if (to->effects < 256)
			bits |= U_EFFECTS8;
		else if (to->effects < 0x8000)
			bits |= U_EFFECTS16;
		else
			bits |= U_EFFECTS8|U_EFFECTS16;
	}
	
	if ( to->effects2 != from->effects2 )
	{
		if (to->effects2 < 256)
			bits2 |= U_EFFECTS2_8;
		else if (to->effects2 < 0x8000)
			bits2 |= U_EFFECTS2_16;
		else
			bits2 |= U_EFFECTS2_8|U_EFFECTS2_16;
	}
	
	if ( to->flags != from->flags )
	{
		if (to->flags < 256)
			bits2 |= U_FLAGS_8;
		else if (to->flags < 0x8000)
			bits2 |= U_FLAGS_16;
		else
			bits2 |= U_FLAGS_8|U_FLAGS_16;
	}
	
	if ( to->renderfx != from->renderfx )
	{
		if (to->renderfx < 256)
			bits |= U_RENDERFX8;
		else if (to->renderfx < 0x8000)
			bits |= U_RENDERFX16;
		else
			bits |= U_RENDERFX8|U_RENDERFX16;
	}
	
	if ( to->solid != from->solid )
		bits |= U_SOLID;
	
	// event is not delta compressed, just 0 compressed
	if ( to->event  )
		bits |= U_EVENT;
	
	if ( to->modelindex != from->modelindex )
		bits |= U_MODEL;

	if ( to->sound != from->sound )
		bits |= U_SOUND;

	if (to->volume   != from->volume ||
		to->dist_min != from->dist_min ||
		to->dist_max != from->dist_max ||
		to->snd_flags != from->snd_flags)	
		bits2 |= U_SOUNDINFO;

	if (to->old_origin.x != from->old_origin.x || 
		to->old_origin.y != from->old_origin.y || 
		to->old_origin.z != from->old_origin.z )
		bits |= U_OLDORIGIN;		// FIXME: only when newly seen!!!

	//	Nelno:	send down changes in render_scale
	if (to->render_scale.x != from->render_scale.x || 
		to->render_scale.y != from->render_scale.y || 
		to->render_scale.z != from->render_scale.z)
		bits |= U_RENDERX;		

	if (to->alpha != from->alpha)
      bits |= U_ALPHA;

	//	Nelno:	send more accurate bounding box info down for client-side prediction, 6 bytes
	if ((to->iflags & IF_SV_SENDBBOX) &&
		(	to->mins.x != from->mins.x || 
			to->mins.y != from->mins.y || 
			to->mins.z != from->mins.z ||
			to->maxs.x != from->maxs.x || 
			to->maxs.y != from->maxs.y || 
			to->maxs.z != from->maxs.z  ))
		bits |= U_BBOX;

	///////////////////////////////////////////////////////////////////////////
	//	the rest of the bit fields are in the second dword
	///////////////////////////////////////////////////////////////////////////

	//	send down REAL mins/maxs... 6 floats... 
	if ((to->renderfx & (RF_BEAM_MOVING | RF_SPOTLIGHT)) &&
		(to->mins.x != from->mins.x || 
		 to->mins.y != from->mins.y || 
		 to->mins.z != from->mins.z ||
		 to->maxs.x != from->maxs.x || 
		 to->maxs.y != from->maxs.y || 
		 to->maxs.z != from->maxs.z))
	{
		bits2 |= U_MINSMAXS;
	}
	//	send down particle volume mins/maxs... 6 floats... ONLY when we're forcing a delta update (i.e. sending baselines etc)
//	else if ((to->renderfx & (RF_SNOW | RF_RAIN | RF_DRIP)) && 
	else if ((to->renderfx & RF_PARTICLEVOLUME) && 
			(force > DELTA_DONT_FORCE) &&
			(to->mins.x != from->mins.x || 
			 to->mins.y != from->mins.y || 
			 to->mins.z != from->mins.z ||
			 to->maxs.x != from->maxs.x || 
			 to->maxs.y != from->maxs.y || 
			 to->maxs.z != from->maxs.z))
	{
		// send down mins maxs for particle volumes
		bits2 |= U_MINSMAXS;
	}
	// if we are a bmodel send down our mins/maxs
	if ((to->solid == 31) &&					  //&&& WARNING: this ultimately needs to be removed
			(to->mins.x != from->mins.x || 
			 to->mins.y != from->mins.y || 
			 to->mins.z != from->mins.z ||
			 to->maxs.x != from->maxs.x || 
			 to->maxs.y != from->maxs.y || 
			 to->maxs.z != from->maxs.z))
	{
		// send down as origin
		CVector	vTemp;
		vTemp = to->mins + to->maxs;
		vTemp.Multiply( 0.5 );
		to->origin = vTemp;
//		to->origin = (to->mins + to->maxs) * 0.5;
		bits |= U_ORIGIN1;
		bits |= U_ORIGIN2;
		bits |= U_ORIGIN3;
	}

	// modelindex2 is the weapon model
	if (to->modelindex2 != from->modelindex2)
		bits |= U_MODEL2;
	if (to->modelindex3 != from->modelindex3)
		bits |= U_MODEL3;
	if (to->modelindex4 != from->modelindex4)
		bits |= U_MODEL4;

	///////////////////////////////////////////////////////////////////////////
	//	determine if any cluster info should be sent down
	///////////////////////////////////////////////////////////////////////////
	if (to->numClusters != from->numClusters)
		bits2 |= U_NUMCLUSTERS;

#if 0	// amw: 5.11.99 - save for when we send down angles for hierarchical parts
	if (to->numClusters > 0) 
	{
		// if we're doing a baseline update...send it all
		if ((force & DELTA_FORCE) || (bits2 & U_NUMCLUSTERS))
		{
			// if this is set, all data for all clusters is sent
			cluster_bits = CDSENT_ALL_BITS;
		}
		else
		{
			// amw: this code block (the for loop) probably isn't necessary..
			// if the indexes to the cluster def table don't match, send everything
			for (int idx=0;idx<to->numClusters;idx++)
			{
				if (to->cDef[idx].clusterIdx != from->cDef[idx].clusterIdx)
				{
					// if this is set, all data for all clusters is sent
					cluster_bits = CDSENT_ALL_BITS;
					break;
				}
			}
			
			// check to see if individual angles have changed
			if (!(cluster_bits & CDSENT_ALL_BITS))
			{
				if (to->cDef[0].angles.x != from->cDef[0].angles.x)
					cluster_bits |= CDSENT_ANGLES_0_X;
				if (to->cDef[0].angles.y != from->cDef[0].angles.y)
					cluster_bits |= CDSENT_ANGLES_0_Y;
				if (to->cDef[0].angles.z != from->cDef[0].angles.z)
					cluster_bits |= CDSENT_ANGLES_0_Z;

				if (to->numClusters > 1)
				{
					if (to->cDef[1].angles.x != from->cDef[1].angles.x)
						cluster_bits |= CDSENT_ANGLES_1_X; 
					if (to->cDef[1].angles.y != from->cDef[1].angles.y)
						cluster_bits |= CDSENT_ANGLES_1_Y;
					if (to->cDef[1].angles.z != from->cDef[1].angles.z)
						cluster_bits |= CDSENT_ANGLES_1_Z;
				}

				// make sure if one of the 3rd cluster values is sent
				// that we flag cluster_bits appropriately
				if (to->numClusters > 2)
				{
					if (to->cDef[2].angles.x != from->cDef[2].angles.x)
						cluster_bits2 |= CDSENT_ANGLES_2_X; 
					if (to->cDef[2].angles.y != from->cDef[2].angles.y)
						cluster_bits2 |= CDSENT_ANGLES_2_Y;
					if (to->cDef[2].angles.z != from->cDef[2].angles.z)
						cluster_bits2 |= CDSENT_ANGLES_2_Z;

					if (cluster_bits2)
						cluster_bits |= CDSENT_3RDCLUSTER;
				}
			}
		}
		// set the appropriate bit in 'bits' so we know to send and read cluster data
		if (cluster_bits || cluster_bits2)
			bits |= U_CLUSTERDATA;
	}
#endif


///*
//	if ((to->renderfx & RF_BEAM_MOVING) &&
//		(to->mins.x != from->mins.x || to->mins.y != from->mins.y || to->mins.z != from->mins.z))
//		bits2 |= U_BEAM_DIR;
//*/

	if (to->angle_delta.x != from->angle_delta.x)
		bits2 |= U_ANGLEDELTA_X;
	if (to->angle_delta.y != from->angle_delta.y)
		bits2 |= U_ANGLEDELTA_Y;
	if (to->angle_delta.z != from->angle_delta.z)
		bits2 |= U_ANGLEDELTA_Z;
	if (to->origin_hack != from->origin_hack)
		bits2 |= U_ORIGINHACK;

	if( to->color.x != from->color.x ||
		to->color.y != from->color.y ||
		to->color.z != from->color.z )
		bits2 |= U_COLOR;

	//
	// write the message
	//
	if (!bits && !bits2 && !force)
		return;		// nothing to send!

	// WAW[11/29/99]: Moved this to below the bail out check.
	if (to->number >= 256)
		bits |= U_NUMBER16;		// number8 is implicit otherwise

	//*************************************************************************
	//*************************************************************************
	//	write the info to the message buffer
	//*************************************************************************
	//*************************************************************************

	//	Nelno:	added second 32-bit integer for more bit fields... damn
	if (bits2 & 0xffffffff)
	{
		bits |= U_MOREBITS4 | U_MOREBITS3 | U_MOREBITS2 | U_MOREBITS1;

		if (bits2 & 0xff000000)
			bits2 |= U_MOREBITS7 | U_MOREBITS6 | U_MOREBITS5;
		else if (bits2 & 0x00ff0000)
			bits2 |= U_MOREBITS6 | U_MOREBITS5;
		else if (bits2 & 0x0000ff00)
			bits2 |= U_MOREBITS5;
		else if (bits2 & 0x000000ff)
			bits |= U_MOREBITS4;
	}
	else if (bits & 0xff000000)
		bits |= U_MOREBITS3 | U_MOREBITS2 | U_MOREBITS1;
	else if (bits & 0x00ff0000)
		bits |= U_MOREBITS2 | U_MOREBITS1;
	else if (bits & 0x0000ff00)
		bits |= U_MOREBITS1;

	if (bits2 & 0xffffffff)
	{
		//	two dwords to send
		//	write the first dword
		MSG_WriteByte (msg,	bits & 255);
		MSG_WriteByte (msg,	(bits >> 8) & 255 );
		MSG_WriteByte (msg,	(bits >> 16) & 255 );
		MSG_WriteByte (msg,	(bits >> 24) & 255 );
		
		if (bits2 & 0xff000000)
		{
			//	second dword
			MSG_WriteByte (msg,	(bits2) & 255 );
			MSG_WriteByte (msg,	(bits2 >> 8) & 255 );
			MSG_WriteByte (msg,	(bits2 >> 16) & 255 );
			MSG_WriteByte (msg,	(bits2 >> 24) & 255 );
		}
		else if (bits2 & 0x00ff0000)
		{
			//	second dword
			MSG_WriteByte (msg,	(bits2) & 255 );
			MSG_WriteByte (msg,	(bits2 >> 8) & 255 );
			MSG_WriteByte (msg,	(bits2 >> 16) & 255 );
		}
		else if (bits2 & 0x0000ff00)
		{
			//	second dword
			MSG_WriteByte (msg,	(bits2) & 255 );
			MSG_WriteByte (msg,	(bits2 >> 8) & 255 );
		}
		else if (bits2 & 0x000000ff)
		{
			//	second dword
			MSG_WriteByte (msg,	(bits2) & 255 );
		}
	}
	else
	{
		//	only one dword to send
		//	write the first byte of the bit-fields
		MSG_WriteByte (msg,	bits & 255);

		if (bits & 0xff000000)
		{
			MSG_WriteByte (msg,	(bits>>8)&255 );
			MSG_WriteByte (msg,	(bits>>16)&255 );
			MSG_WriteByte (msg,	(bits>>24)&255 );
		}
		else if (bits & 0x00ff0000)
		{
			MSG_WriteByte (msg,	(bits>>8)&255 );
			MSG_WriteByte (msg,	(bits>>16)&255 );
		}
		else if (bits & 0x0000ff00)
		{
			MSG_WriteByte (msg,	(bits>>8)&255 );
		}
	}

	//----------

	if (bits & U_NUMBER16)
		MSG_WriteShort (msg, to->number);
	else
		MSG_WriteByte (msg,	to->number);

	//	Nelno:	changed modelindex to write shorts
	if (bits & U_MODEL)
		MSG_WriteShort (msg, to->modelindex);

	if ((bits & U_SKIN8) && (bits & U_SKIN16))		//used for laser colors
		MSG_WriteLong (msg, to->skinnum);
	else if (bits & U_SKIN8)
		MSG_WriteByte (msg, to->skinnum);
	else if (bits & U_SKIN16)
		MSG_WriteShort (msg, to->skinnum);


	if ( (bits & (U_EFFECTS8|U_EFFECTS16)) == (U_EFFECTS8|U_EFFECTS16) )
		MSG_WriteLong (msg, to->effects);
	else if (bits & U_EFFECTS8)
		MSG_WriteByte (msg, to->effects);
	else if (bits & U_EFFECTS16)
		MSG_WriteShort (msg, to->effects);

	if ( (bits2 & (U_EFFECTS2_8|U_EFFECTS2_16)) == (U_EFFECTS2_8|U_EFFECTS2_16) )
		MSG_WriteLong (msg, to->effects2);
	else if (bits2 & U_EFFECTS2_8)
		MSG_WriteByte (msg, to->effects2);
	else if (bits2 & U_EFFECTS2_16)
		MSG_WriteShort (msg, to->effects2);

	if ( (bits2 & (U_FLAGS_8|U_FLAGS_16)) == (U_FLAGS_8|U_FLAGS_16) )
		MSG_WriteLong (msg, to->flags);
	else if (bits2 & U_FLAGS_8)
		MSG_WriteByte (msg, to->flags);
	else if (bits2 & U_FLAGS_16)
		MSG_WriteShort (msg, to->flags);

	if ( (bits & (U_RENDERFX8|U_RENDERFX16)) == (U_RENDERFX8|U_RENDERFX16) )
		MSG_WriteLong (msg, to->renderfx);
	else if (bits & U_RENDERFX8)
		MSG_WriteByte (msg, to->renderfx);
	else if (bits & U_RENDERFX16)
		MSG_WriteShort (msg, to->renderfx);
/* *** NUKE THIS SOON ***
	if (bits & U_ENTITY_ID)
		MSG_WriteShort (msg, to->entityID);
*/
	if (bits & U_ORIGIN1)
		MSG_WriteCoord (msg, to->origin.x);		
	if (bits & U_ORIGIN2)
		MSG_WriteCoord (msg, to->origin.y);
	if (bits & U_ORIGIN3)
		MSG_WriteCoord (msg, to->origin.z);

	if (bits & U_ANGLE1)
		MSG_WriteFloat(msg, to->angles.x);
	if (bits & U_ANGLE2)
		MSG_WriteFloat(msg, to->angles.y);
	if (bits & U_ANGLE3)
		MSG_WriteFloat(msg, to->angles.z);

	if (bits & U_OLDORIGIN)
	{
		MSG_WriteCoord (msg, to->old_origin.x);
		MSG_WriteCoord (msg, to->old_origin.y);
		MSG_WriteCoord (msg, to->old_origin.z);
	}

	if (bits & U_SOUND)
		MSG_WriteShort (msg, to->sound);

	// NOTE: if this is changed after world spawn this should be sent as a byte
	if (bits2 & U_SOUNDINFO)
	{
		MSG_WriteByte(msg,(char)(to->volume*255.0f));
		MSG_WriteByte(msg,((int)to->dist_min)>>3);
		MSG_WriteByte(msg,((int)to->dist_max)>>5);
		MSG_WriteByte(msg,to->snd_flags);
	}

	if (bits & U_EVENT)
		MSG_WriteByte (msg, to->event);
	if (bits & U_SOLID)
		MSG_WriteShort (msg, to->solid);

	if (bits & U_RENDERX)
	{
		//	Nelno FIXME:	do we really need to send floats here?
		MSG_WriteFloat (msg, to->render_scale.x);
		MSG_WriteFloat (msg, to->render_scale.y);
		MSG_WriteFloat (msg, to->render_scale.z);
	}

	if (bits & U_BBOX)
	{
		//	assume mins are negative and mask to a byte
		MSG_WriteByte (msg, ((int) -to->mins.x) & 0x000000ff);
		MSG_WriteByte (msg, ((int) -to->mins.y) & 0x000000ff);
		MSG_WriteByte (msg, ((int) -to->mins.z) & 0x000000ff);
		MSG_WriteByte (msg, ((int) to->maxs.x) & 0x000000ff);
		MSG_WriteByte (msg, ((int) to->maxs.y) & 0x000000ff);
		MSG_WriteByte (msg, ((int) to->maxs.z) & 0x000000ff);
	}

   if (bits & U_ALPHA)
      MSG_WriteFloat(msg,to->alpha);

	if (bits2 & U_MINSMAXS)
	{
		MSG_WriteFloat (msg, to->mins.x);
		MSG_WriteFloat (msg, to->mins.y);
		MSG_WriteFloat (msg, to->mins.z);
		MSG_WriteFloat (msg, to->maxs.x);
		MSG_WriteFloat (msg, to->maxs.y);
		MSG_WriteFloat (msg, to->maxs.z);
	}

	if (bits2 & U_BEAM_DIR)
	{
		//	Nelno:	sends down a byte that specifies one of the pre-defined vertex normals
		//	not quite as accurate, but I guess we'll find out if it doesn't work very well..
		MSG_WriteDir (msg, to->mins);
	}

	////////////////////////////////////////////////////////////////////////////
	//	Nelno:	write whatever frameInfo stuff we need to send
	////////////////////////////////////////////////////////////////////////////

	if (bits & (U_FRAME8 | U_FRAME16))
	{
		if (bits & U_FRAME16)
		{
			// amw: 5.3.99 - casting ints to shorts fucks up the data..
			// and then reading them on the client side gives you
			// mangled bitmasked data.. it's compensated
			// on the client side.  we really need to write a 
			// MSG_WriteWord()

			//	send down two bytes of frame_sent_bits
			MSG_WriteShort (msg, (short) (frame_sent_bits & 0xffff));
			if (frame_sent_bits & FRSENT_THIRDBYTE)
				//	send down third byte
				MSG_WriteByte (msg, (byte) (frame_sent_bits >> 16));
		}
		else
			//	send down first byte only
			MSG_WriteByte (msg, (byte) (byte) (frame_sent_bits & 0x0ff));

		if (frame_sent_bits & FRSENT_SYNC8)
		{
//			Com_Printf ("server: synced frame, %i.\n", to->frame);
			MSG_WriteByte (msg, to->frame);
		}
		else if (frame_sent_bits & FRSENT_SYNC16)
		{
//			Com_Printf ("server: synced frame, %i\n", to->frame);
			MSG_WriteShort (msg, to->frame);
		}

		if (frame_sent_bits & FRSENT_FLAGS16)
			MSG_WriteShort (msg, to->frameInfo.frameFlags);

		if (frame_sent_bits & FRSENT_STARTFRAME8)
			MSG_WriteByte (msg, to->frameInfo.startFrame);
		else if (frame_sent_bits & FRSENT_STARTFRAME16)
			MSG_WriteShort (msg, to->frameInfo.startFrame);

		if (frame_sent_bits & FRSENT_ENDFRAME8)
			MSG_WriteByte (msg, to->frameInfo.endFrame);
		else if (frame_sent_bits & FRSENT_ENDFRAME16)
			MSG_WriteShort (msg, to->frameInfo.endFrame);

		if (frame_sent_bits & FRSENT_SOUND1FRAME8)
			MSG_WriteByte (msg, to->frameInfo.sound1Frame);
		else if (frame_sent_bits & FRSENT_SOUND1FRAME16)
			MSG_WriteShort (msg, to->frameInfo.sound1Frame);

		if (frame_sent_bits & FRSENT_SOUND2FRAME8)
			MSG_WriteByte (msg, to->frameInfo.sound2Frame);
		else if (frame_sent_bits & FRSENT_SOUND2FRAME16)
			MSG_WriteShort (msg, to->frameInfo.sound2Frame);

		if (frame_sent_bits & FRSENT_SOUND1INDEX8)
			MSG_WriteByte (msg, to->frameInfo.sound1Index);

		if (frame_sent_bits & FRSENT_SOUND2INDEX8)
			MSG_WriteByte (msg, to->frameInfo.sound2Index);

		if (frame_sent_bits & FRSENT_FRAMEINC8)
			MSG_WriteByte (msg, to->frameInfo.frameInc);

		if (frame_sent_bits & FRSENT_STARTLOOP16)
			MSG_WriteShort (msg, to->frameInfo.startLoop);

		if (frame_sent_bits & FRSENT_ENDLOOP16)
			MSG_WriteShort (msg, to->frameInfo.endLoop);

		if (frame_sent_bits & FRSENT_LOOPCOUNT8)
			MSG_WriteByte (msg, to->frameInfo.loopCount);

		// amw: can we scale this and send less data?
		if (frame_sent_bits & FRSENT_ANIMSPEED)
			MSG_WriteFloat (msg, to->frameInfo.modelAnimSpeed);

	}	

	if (bits & U_MODEL2)
		MSG_WriteLong(msg, to->modelindex2);
	if (bits & U_MODEL3)
		MSG_WriteShort(msg, to->modelindex3);
	if (bits & U_MODEL4)
		MSG_WriteShort(msg, to->modelindex4);


	///////////////////////////////////////////////////////////////////////////
	//	write all cluster info, if needed
	///////////////////////////////////////////////////////////////////////////
	if (bits2 & U_NUMCLUSTERS)
	{
//		_ASSERTE(to->numClusters <= MAX_MESH_CLUSTERS);
		MSG_WriteShort(msg, to->numClusters);
	}

	if (bits & U_CLUSTERDATA)
	{
		MSG_WriteByte(msg, cluster_bits);

		// the 3rd cluster defined had data that has changed
		if (cluster_bits & CDSENT_3RDCLUSTER)
			MSG_WriteByte(msg, cluster_bits2);

		// send all info down
		if (cluster_bits & CDSENT_ALL_BITS)
		{
			for (int i=0;i<to->numClusters;i++)
			{
				MSG_WriteByte(msg, to->cDef[i].clusterIdx);
				MSG_WriteAngle(msg, to->cDef[i].angles.x);
				MSG_WriteAngle(msg, to->cDef[i].angles.y);
				MSG_WriteAngle(msg, to->cDef[i].angles.z);
			}
		}
		else
		{
			// first cluster
			if (cluster_bits & CDSENT_ANGLES_0_X)
				MSG_WriteAngle(msg, to->cDef[0].angles.x);
			if (cluster_bits & CDSENT_ANGLES_0_Y)
				MSG_WriteAngle(msg, to->cDef[0].angles.y);
			if (cluster_bits & CDSENT_ANGLES_0_Z)
				MSG_WriteAngle(msg, to->cDef[0].angles.z);
			// second cluster
			if (cluster_bits & CDSENT_ANGLES_1_X)
				MSG_WriteAngle(msg, to->cDef[1].angles.x);
			if (cluster_bits & CDSENT_ANGLES_1_Y)
				MSG_WriteAngle(msg, to->cDef[1].angles.y);
			if (cluster_bits & CDSENT_ANGLES_1_Z)
				MSG_WriteAngle(msg, to->cDef[1].angles.z);
			// third cluster
			if (cluster_bits2 & CDSENT_ANGLES_2_X)
				MSG_WriteAngle(msg, to->cDef[2].angles.x);
			if (cluster_bits2 & CDSENT_ANGLES_2_Y)
				MSG_WriteAngle(msg, to->cDef[2].angles.y);
			if (cluster_bits2 & CDSENT_ANGLES_2_Z)
				MSG_WriteAngle(msg, to->cDef[2].angles.z);
		}
	}

//	///////////////////////////////////////////////////////////////////////////
//	//	write all hierarchical info, if needed
//	///////////////////////////////////////////////////////////////////////////
//
//	if (bits2 & U_HIERARCHICAL)
//	{
//		_ASSERTE(0);	// make sure this shit isn't being used
//		
//		//	send the hr_sent_bits flag to the client MAXIMUM OF ONE BYTE!!!
//		MSG_WriteByte (msg, hr_sent_bits);
//
//		if (hr_sent_bits & HRSENT_PARENTNUM8)
//			MSG_WriteByte (msg, to->hrInfo.parentNumber);
//		else if (hr_sent_bits & HRSENT_PARENTNUM16)
//			MSG_WriteShort (msg, to->hrInfo.parentNumber);
//
//		if (hr_sent_bits & HRSENT_HRFLAGS8)
//			MSG_WriteByte (msg, to->hrInfo.hrFlags);
//		else if (hr_sent_bits & HRSENT_HRFLAGS16)
//			MSG_WriteShort (msg, to->hrInfo.hrFlags);
//
//		if (hr_sent_bits & HRSENT_HRTYPE)
//			MSG_WriteByte (msg, to->hrInfo.hrType);
//
//		if (hr_sent_bits & HRSENT_SURFINDEX)
//			MSG_WriteByte (msg, to->hrInfo.surfaceIndex);
//
//		if (hr_sent_bits & HRSENT_VERTINDEX8)
//			MSG_WriteByte (msg, to->hrInfo.vertexIndex);
//		else if (hr_sent_bits & HRSENT_VERTINDEX16)
//			MSG_WriteShort (msg, to->hrInfo.vertexIndex);
//	}

	if (bits2 & U_ANGLEDELTA_X)
		MSG_WriteAngle (msg, to->angle_delta.x);
	if (bits2 & U_ANGLEDELTA_Y)
		MSG_WriteAngle (msg, to->angle_delta.y);
	if (bits2 & U_ANGLEDELTA_Z)
		MSG_WriteAngle (msg, to->angle_delta.z);
	if (bits2 & U_ORIGINHACK)
	{
		MSG_WriteCoord (msg, to->origin_hack.x);		
		MSG_WriteCoord (msg, to->origin_hack.y);		
		MSG_WriteCoord (msg, to->origin_hack.z);		
	}

	if( bits2 & U_COLOR )
	{
		MSG_WriteFloat (msg, to->color.x);
		MSG_WriteFloat (msg, to->color.y);
		MSG_WriteFloat (msg, to->color.z);
	}
}


//============================================================

//
// reading functions
//

void MSG_BeginReading (sizebuf_t *msg)
{
	msg->readcount = 0;
}

// returns -1 if no more characters are available
int MSG_ReadChar (sizebuf_t *msg_read)
{
	int	c;
	
	if (msg_read->readcount+1 > msg_read->cursize)
		c = -1;
	else
		c = (signed char)msg_read->data[msg_read->readcount];
	msg_read->readcount++;
	
	return c;
}

int MSG_ReadByte (sizebuf_t *msg_read)
{
	int	c;
	
	if (msg_read->readcount+1 > msg_read->cursize)
		c = -1;
	else
		c = (unsigned char)msg_read->data[msg_read->readcount];
	msg_read->readcount++;
	
	return c;
}

int MSG_ReadShort (sizebuf_t *msg_read)
{
	int	c;
	
	if (msg_read->readcount+2 > msg_read->cursize)
		c = -1;
	else		
		c = (short)(msg_read->data[msg_read->readcount]
		+ (msg_read->data[msg_read->readcount+1]<<8));
	
	msg_read->readcount += 2;
	
	return c;
}

int MSG_ReadLong (sizebuf_t *msg_read)
{
	int	c;
	
	if (msg_read->readcount+4 > msg_read->cursize)
		c = -1;
	else
		c = msg_read->data[msg_read->readcount]
		+ (msg_read->data[msg_read->readcount+1]<<8)
		+ (msg_read->data[msg_read->readcount+2]<<16)
		+ (msg_read->data[msg_read->readcount+3]<<24);
	
	msg_read->readcount += 4;
	
	return c;
}

float MSG_ReadFloat (sizebuf_t *msg_read)
{
	union
	{
		byte	b[4];
		float	f;
		int	l;
	} dat;
	
	if (msg_read->readcount+4 > msg_read->cursize)
		dat.f = -1;
	else
	{
		dat.b[0] =	msg_read->data[msg_read->readcount];
		dat.b[1] =	msg_read->data[msg_read->readcount+1];
		dat.b[2] =	msg_read->data[msg_read->readcount+2];
		dat.b[3] =	msg_read->data[msg_read->readcount+3];
	}
	msg_read->readcount += 4;
	
	dat.l = LittleLong (dat.l);

	return dat.f;	
}

char *MSG_ReadString (sizebuf_t *msg_read)
{
	static char	string[2048];
	int		l,c;
	
	l = 0;
	do
	{
		c = MSG_ReadChar (msg_read);
		if (c == -1 || c == 0)
			break;
		string[l] = c;
		l++;
	} while (l < sizeof(string)-1);
	
	string[l] = 0;
	
	return string;
}

char *MSG_ReadStringLine (sizebuf_t *msg_read)
{
	static char	string[2048];
	int		l,c;
	
	l = 0;
	do
	{
		c = MSG_ReadChar (msg_read);
		if (c == -1 || c == 0 || c == '\n')
			break;
		string[l] = c;
		l++;
	} while (l < sizeof(string)-1);
	
	string[l] = 0;
	
	return string;
}

#define ONE_OVER_8  0.125

float MSG_ReadCoord (sizebuf_t *msg_read)
{
	return MSG_ReadShort(msg_read) * ONE_OVER_8;
}

void MSG_ReadPos (sizebuf_t *msg_read, CVector &pos)
{
	pos.x = MSG_ReadShort(msg_read) * ONE_OVER_8;
	pos.y = MSG_ReadShort(msg_read) * ONE_OVER_8;
	pos.z = MSG_ReadShort(msg_read) * ONE_OVER_8;
}

#define  THREESIXTY_OVER_256 1.40625

float MSG_ReadAngle (sizebuf_t *msg_read)
{
//	return MSG_ReadChar(msg_read) * (360.0/256);	//amw - changed.. is this wrong???
	return MSG_ReadByte(msg_read) * THREESIXTY_OVER_256;	// mdm99.07.19 (360.0/256);
}

float MSG_ReadAngle16 (sizebuf_t *msg_read)
{
	return SHORT2ANGLE(MSG_ReadShort(msg_read));
}

///////////////////////////////////////////////////////////////////////////////
//
//  copies from to move, then puts the data in move
//
///////////////////////////////////////////////////////////////////////////////
void MSG_ReadDeltaUsercmd (sizebuf_t *msg_read, usercmd_t *from, usercmd_t *move)
{
	int bits;
	memcpy (move, from, sizeof(*move));
	bits = MSG_ReadByte (msg_read);
		
// read current angles
	if (bits & CM_ANGLE1)
		move->angles[0] = MSG_ReadShort (msg_read);
	if (bits & CM_ANGLE2)
		move->angles[1] = MSG_ReadShort (msg_read);
	if (bits & CM_ANGLE3)
		move->angles[2] = MSG_ReadShort (msg_read);
		
// read movement
	if (bits & CM_FORWARD)
		move->forwardmove = MSG_ReadShort (msg_read);
	if (bits & CM_SIDE)
		move->sidemove = MSG_ReadShort (msg_read);
	if (bits & CM_UP)
		move->upmove = MSG_ReadShort (msg_read);
	
// read buttons
	if (bits & CM_BUTTONS)
		move->buttons = MSG_ReadByte (msg_read);
	if (bits & CM_IMPULSE)
		move->impulse = MSG_ReadByte (msg_read);
// read time to run command
	move->msec = MSG_ReadByte (msg_read);
// read the light level
	move->lightlevel = MSG_ReadByte (msg_read);
}

void MSG_ReadData (sizebuf_t *msg_read, void *data, int len)
{
	int		i;

	for (i=0 ; i<len ; i++)
		((byte *)data)[i] = MSG_ReadByte (msg_read);
}


//===========================================================================

void SZ_Init (sizebuf_t *buf, byte *data, int length)
{
	memset (buf, 0, sizeof(*buf));
	buf->data = data;
	buf->maxsize = length;
}

void SZ_Clear (sizebuf_t *buf)
{
	buf->cursize = 0;
	buf->overflowed = false;
}

void *SZ_GetSpace (sizebuf_t *buf, int length)
{
	void	*data;
	
	if (buf->cursize + length > buf->maxsize)
	{
		if (!buf->allowoverflow)
		{
			_ASSERTE(FALSE);
			Com_Error (ERR_FATAL, "SZ_GetSpace: overflow without allowoverflow set"); 
		}
		
		if (length > buf->maxsize)
			Com_Error (ERR_FATAL, "SZ_GetSpace: %i is > full buffer size", length);
			
		Com_DPrintf ("SZ_GetSpace: overflow\n");
		SZ_Clear (buf); 
		buf->overflowed = true;
	}

	data = buf->data + buf->cursize;
	buf->cursize += length;
	
	return data;
}

void SZ_Write (sizebuf_t *buf, const void *data, int length)
{
	memcpy (SZ_GetSpace(buf,length),data,length);		
}

void SZ_Print (sizebuf_t *buf, char *data)
{
	int		len;
	
	len = strlen(data)+1;

	if (buf->cursize)
	{
		if (buf->data[buf->cursize-1])
			memcpy ((byte *)SZ_GetSpace(buf, len),data,len); // no trailing 0
		else
			memcpy ((byte *)SZ_GetSpace(buf, len-1)-1,data,len); // write over trailing 0
	}
	else
		memcpy ((byte *)SZ_GetSpace(buf, len),data,len);
}


//============================================================================



//============================================================================


/*
================
COM_CheckParm

Returns the position (1 to argc-1) in the program's argument list
where the given parameter apears, or 0 if not present
================
*/
int COM_CheckParm (char *parm)
{
	int		i;
	
	for (i=1 ; i<com_argc ; i++)
	{
		if (!strcmp (parm,com_argv[i]))
			return i;
	}
		
	return 0;
}

int EXEGetArgc(void)
{
	return com_argc;
}

char *EXEGetArgv(int arg)
{
	if (arg < 0 || arg >= com_argc || !com_argv[arg])
		return "";
	return com_argv[arg];
}

void COM_ClearArgv (int arg)
{
	if (arg < 0 || arg >= com_argc || !com_argv[arg])
		return;
	com_argv[arg] = "";
}


///*
//================
//COM_InitArgv
//================
//*/
//void COM_InitArgv (int argc, char **argv)
//{
//	int		i, j;
//
//	for (i = 0; i < MAX_NUM_ARGVS; i++)
//		com_argv [i] = "";
//
//	if (argc > MAX_NUM_ARGVS)
//		Com_Error (ERR_FATAL, "argc > MAX_NUM_ARGVS");
//
//	com_argc = argc;
//	for (i=0 ; i<argc ; i++)
//	{
//		if (!argv[i] || strlen(argv[i]) >= MAX_TOKEN_CHARS )
//			com_argv[i] = "";
//		else
//		{
//			com_argv[i] = argv[i];
//			
//			//	Nelno:	convert arg to lowercase
//			for (j = 0; j < strlen (com_argv [i]); j++)
//				if (com_argv [i][j] > 0x40 && com_argv [i][j] < 0x5B)
//					com_argv [i][j] -= 0x20;
//		}
//	}
//}

// mdm99.03.04 - Straight from Quake2

/*
================
COM_InitArgv
================
*/
void COM_InitArgv (int argc, char **argv)
{
	int		i;
	if (argc > MAX_NUM_ARGVS)
		Com_Error (ERR_FATAL, "argc > MAX_NUM_ARGVS");
	com_argc = argc;
	for (i=0 ; i<argc ; i++)
	{
		if (!argv[i] || strlen(argv[i]) >= MAX_TOKEN_CHARS )
			com_argv[i] = "";
		else
			com_argv[i] = argv[i];
	}
}

/*
================
COM_AddParm

Adds the given string at the end of the current argument list
================
*/
void COM_AddParm (char *parm)
{
	if (com_argc == MAX_NUM_ARGVS)
		Com_Error (ERR_FATAL, "COM_AddParm: MAX_NUM_ARGS");
	com_argv[com_argc++] = parm;
}




/// just for debugging
int	memsearch (byte *start, int count, int search)
{
	int		i;
	
	for (i=0 ; i<count ; i++)
		if (start[i] == search)
			return i;
	return -1;
}


char *CopyString (const char *in)
{
	char	*out;
	
	out = (char *)X_Malloc (strlen(in)+1, MEM_TAG_COPYSTRING);
	strcpy (out, in);
	return out;
}



void Info_Print (char *s)
{
	char	key[512];
	char	value[512];
	char	*o;
	int		l;

	if (*s == '\\')
		s++;
	while (*s)
	{
		o = key;
// Encompass MarkMa 040599
#ifdef	JPN	// JPN
		while (*s)	{
			if(*s == '\\')	break;
			if(IsDBCSLeadByte(*s))
				*o++ = *s++;
			*o++ = *s++;
		}
#else	// JPN
		while (*s && *s != '\\')
			*o++ = *s++;
#endif	// JPN
// Encompass MarkMa 040599

		l = o - key;
		if (l < 20)
		{
			memset (o, ' ', 20-l);
			key[20] = 0;
		}
		else
			*o = 0;
		Com_Printf ("%s", key);

		if (!*s)
		{
			Com_Printf ("MISSING VALUE\n");
			return;
		}

		o = value;
		s++;
// Encompass MarkMa 040599
#ifdef	JPN	// JPN
		while (*s)	{
			if(*s == '\\')	break;
			if(IsDBCSLeadByte(*s))
				*o++ = *s++;
			*o++ = *s++;
		}
#else	// JPN
		while (*s && *s != '\\')
			*o++ = *s++;
#endif	// JPN
 // Encompass MarkMa 040599

		*o = 0;

		if (*s)
			s++;
		Com_Printf ("%s\n", value);
	}
}


/*
====================
COM_BlockSequenceCheckByte

For proxy protecting
====================
*/
byte	COM_BlockSequenceCheckByte (byte *base, int length, int sequence)
{
	int		checksum;
	byte	buf[64];
	int		n;
	byte	*p;
	float	temp[2];

    // mdm 98.01.28 - in quake 2 this was the size of the struct that was actually 
	//	being passed down the pipe.  now that we've changed to a class i don't
	//	think we wanted the size of the class, but actually the size of the data
	//	being passed down.  maybe it makes no difference, but i'm sick of this bug.
	// mdm 98.01.28 - okay it makes no difference.  but i like it this way better.
	// mdm 98.01.29 - wow, always trust your first instinct.  this was the problem,
	//	however if was the fact that avertexnormals was changed from a vec3_t to a
	//	CVector.  i knew it was here all along, but damn what a long journey.
//	n = sequence % (NUMVERTEXNORMALS * sizeof(CVector));
	
    // mdm 98.02.12 - fixed cash's overwrite
	n = sequence % ((NUMVERTEXNORMALS-1) * sizeof(float[3]));

	// read two floats into temp array and ensure proper byte order
	memcpy ((byte *)temp, (byte *)avertexnormals.GetFloatArray() + n - (n % sizeof(float)), 
			sizeof(temp));
	temp[0] = LittleFloat (temp[0]);
	temp[1] = LittleFloat (temp[1]);
	p = ((byte *)temp) + (n % sizeof(float));
	
	if (length > 60)
		length = 60;
	memcpy (buf, base, length);
	buf[length] = (sequence & 0xff) ^ *p++;
	buf[length+1] = *p++;
	buf[length+2] = ((sequence>>8) & 0xff) ^ *p++;
	buf[length+3] = *p;
	length += 4;

	checksum = LittleLong(Com_BlockChecksum (buf, length));

	checksum &= 0xff;

#if 0
	// jwu to dump avertexnormals array
	// debug and release version produce different results for avertexnormals
	static int once = 0;
	if (!once)
	{
		int normalcheck = Com_BlockChecksum((byte*)avertexnormals.GetFloatArray(), sizeof(avertexnormals));
		once = 1;
//		log_printf("avertexnormals checksum = %d\n", normalcheck);
		Com_Printf("avertexnormals checksum = %d\n", normalcheck);
		for(int index =0; index < 255 ; index++)
		{
//			log_printf("mN[%3d].Set (%+1.6ff, %+1.6ff, %+1.6ff);\n", index,
//				avertexnormals.GetNormal(index).X(),
//				avertexnormals.GetNormal(index).Y(),
//				avertexnormals.GetNormal(index).Z());
			Com_Printf("mN[%3d].Set (%+1.6ff, %+1.6ff, %+1.6ff);\n", index,
				avertexnormals.GetNormal(index).X(),
				avertexnormals.GetNormal(index).Y(),
				avertexnormals.GetNormal(index).Z());
		}
	}

	char    packet[2048];
	memset(packet, 0, sizeof(packet));
	char BDC[] =
{'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	for(int i=0;i<length;i++)
	{
		packet[i*2  ]=BDC[(buf[i] & 0xf0)>>4];
		packet[i*2+1]=BDC[(buf[i] & 0x0f)];
	}
//	log_printf("seq=%d cc=%d size=%d\n%s\n\n",sequence,checksum,test_size,packet);
#endif

	return checksum;
}

// this came from a site that hacked the quake 2 net protocol
// interesting ...  it even helped me find our problem!

////// MakeChecksum for v3.13/v3.14//
//byte	COM_BlockSequenceCheckByte (byte *base, int length, int sequence)
//{
//	int offset;
//	unsigned char *p, tbuf[64];   
//	
//	offset = (sequence % 0x798);
//	p = (unsigned char *)avertexnormals + offset;   
//	if (length > 60) 
//		length = 60;
//	memcpy(tbuf, base, length);   
//	tbuf[length] = p[0] ^ sequence;
//	tbuf[length+1] = p[1];   
//	tbuf[length+2] = p[2] ^ (sequence >> 8);
//	tbuf[length+3] = p[3];
//	
//	return Com_BlockChecksum(tbuf, length+4) & 0xFF;
//}


//========================================================

void Key_Init (void);
void SCR_EndLoadingPlaque (void);

/*
=============
Com_Error_f

Just throw a fatal error to
test error shutdown procedures
=============
*/
void Com_Error_f (void)
{
	Com_Error (ERR_FATAL, "%s", GetArgv(1));
}

///////////////////////////////////////////////////////////////////////////////
//	host_rates_f
//
//	toggle frame rate display on and off
///////////////////////////////////////////////////////////////////////////////

void	host_rates_f (void)
{
	if (dk_rates == NULL)
		return;

	if (GetArgc () == 1)
	{
		//	no value specified, so just toggle
		if (dk_rates->value == 0)
			dk_rates->value = 1;
		else
			dk_rates->value = 0;
	}
	else
	{
		//	set value explicitly
		Cvar_ForceSet ("dk_rates", GetArgv (1));
	}

	if (dk_rates->value == 0)
		Com_Printf ("Frame rate display enabled.\n");
	else
		Com_Printf ("Frame rate display disabled.\n");

//#if !DEDICATED_ONLY
	//dk_InitRates (viddef.width, 64);
//#endif
}

///////////////////////////////////////////////////////////////////////
// Cmd_LogServer_f()
//
// Description:
//    Sets and starts, or resets the network logging client. Uses the 
//    log_server cvar as the destination log server address. 
// 
// Parms:
//    log_server cvar from the game, no direct parameters
//
// Return:
//    none, g_CommonSocket is set in udpclient
//
// Bugs:
//    doesn't support concurrent server use.
void Cmd_LogServer_f(void)
{
//   g_cvarLogServer = Cvar_Get("log_server", "", CVAR_ARCHIVE);
//   UDP_Log(6, "cmd net_log - start logging");

}
// End Cmd_LogServer_f()
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Logic[4/4/99]: precaculating sine/cosine tables to expedite 
//  particle effects

/*
double  *costable;
double  *sintable;

#ifndef M_PI
    #define M_PI                            3.14159265358979323846
#endif
// Logic[4/4/99]: using doubles for lookup tables, this PI_OVER_ONEIGHTY has more granularity
#ifndef PI_OVER_ONEEIGHTY   
    #define PI_OVER_ONEEIGHTY               0.01745329251994329576
#endif
#ifndef DEG2RAD
    #define DEG2RAD( a )                    ( (a) * PI_OVER_ONEEIGHTY )
#endif
#ifndef TRIGTABLE_SIZE
    #define TRIGTABLE_SIZE                  4096
#endif
void BuildTrigTables(void)
{
    unsigned long   i;
    double          inc, d;
    if(costable)
        free(costable);
    if(sintable)
        free(sintable);

    costable = (double *)X_Malloc(TRIGTABLE_SIZE * sizeof(double), MEM_TAG_MISC);
    sintable = (double *)X_Malloc(TRIGTABLE_SIZE * sizeof(double), MEM_TAG_MISC);

    inc = 360.0 / (double)TRIGTABLE_SIZE;
    d=0;
    for(i = 0; i < TRIGTABLE_SIZE; i++)
    {
        costable[i] = cos(DEG2RAD(d));
        sintable[i] = sin(DEG2RAD(d));
        d+=inc;
    }
}
*/
//
///////////////////////////////////////////////////////////////////////
//short CL_LoadLastConfiguration(void);

///////////////////////////////////////////////////////////////////////////////
//	Qcommon_Init
//	
//	Nelno:	main initialization routine.  Called when Q2 starts up for the first time
///////////////////////////////////////////////////////////////////////////////
void Qcommon_Init (int argc, char **argv)
{
	char	*s;
	int		i;


	//	define name of physics dll
//unix - physics.dll / physics.so
#if _MSC_VER
#if defined _M_IX86
	gamename = "physics.dll";
	debugdir = "debug";
#elif defined _M_ALPHA
	gamename = "physicsa.dll";
	debugdir = "debugaxp";
#endif
#else
	gamename = "physics.so";
	debugdir = "debug";
#endif

    //	Nelno:	stop sound buffer from overflowing on initial inits
	dk_Initializing = true;

	if (setjmp (abortframe) )
		Sys_Error ("Error during initialization");

	// init the memory manager
	InitMemoryManager();

    
    // Logic[4/4/99]: Build sin/cos tables
//    BuildTrigTables();

	// prepare enough of the subsystems to handle
	// cvar and command buffer management
	COM_InitArgv (argc, argv);

	Swap_Init ();
	Cbuf_Init ();

	Cmd_Init ();
	Cvar_Init ();

	Key_Init ();

#ifdef BUILD_PATH_TABLE
    dedicated = Cvar_Get( "dedicated", "1", 1 );
	PATHTABLE_StartThread();
#endif BUILD_PATH_TABLE

	if (COM_CheckParm ("-dedicated"))
		dedicated = Cvar_Get ("dedicated", "1", 1);
    else
        dedicated = Cvar_Get ("dedicated", "0", 1);

	// we need to add the early commands twice, because
	// a basedir or cddir needs to be set before execing
	// config files, but we want other parms to override
	// the settings of the config files
	Cbuf_AddEarlyCommands (false);
	Cbuf_Execute ();

	FS_InitFilesystem ();

    if (!dedicated->value)
    {
	    Cbuf_AddText ("exec daikatana.cfg\n");
		Cbuf_Execute ();
    
	    // Logic[4/30/99]: Eliminate all these extra cfg's (per John, CPL tourney)
        // Cbuf_AddText ("exec autoexec.cfg\n");
    	
/*
		//	Nelno:	load username.cfg instead of config.cfg
	    dk_GetUserName ();
#ifndef	LOCKED_DEMO
	    sprintf (config_str, "exec %s.cfg\n", dk_userName);
	    Cbuf_AddText (config_str);
#else
	    Cbuf_AddText ("default.cfg");
#endif*/
    } 
    else 
    {
        Cvar_Get("deathmatch", "1", 1); // Logic[4/30/99]: deathmatch wasn't set automatically with -dedicated?
        Cbuf_AddText("exec server.cfg\n");
    }

	// Nelno:	override dlls directory
	if (i = COM_CheckParm ("-dlldir"))
// SCG[1/16/00]: 		sprintf (dk_dllDir, "%s", com_argv [i + 1]);
		Com_sprintf (dk_dllDir, sizeof(dk_dllDir),"%s", com_argv [i + 1]);
	else
// SCG[1/16/00]: 		sprintf (dk_dllDir, "dlls");
		Com_sprintf (dk_dllDir,sizeof(dk_dllDir), "dlls");

	Cbuf_AddEarlyCommands (true);
	Cbuf_Execute ();

	//
	// init commands and vars
	//
	Cmd_AddCommand ("mem_stat", Mem_Stat_f);
	Cmd_AddCommand ("mem_compact", Mem_Compact_f);
	Cmd_AddCommand ("mem_dump", Mem_Dump_f);
	Cmd_AddCommand ("mem_total_long", Mem_Total_Long_f);
	Cmd_AddCommand ("mem_total_short", Mem_Total_Short_f);
	Cmd_AddCommand ("mem_seq", Mem_Seq_f);
    Cmd_AddCommand ("error", Com_Error_f);
    Cmd_AddCommand ("host_rates", host_rates_f);
    Cmd_AddCommand ("net_log", Cmd_LogServer_f);
	//	Nelno:	for showing real framerate
	dk_rates = Cvar_Get ("dk_rates", "0", CVAR_NOSET);
	showsync = Cvar_Get ("showsync", "0", 0);

	host_speeds = Cvar_Get ("host_speeds", "0", 0);
	log_stats = Cvar_Get ("log_stats", "0", 0);
	developer = Cvar_Get ("developer", "0", CVAR_ARCHIVE);
	console = Cvar_Get ("console", "0", 0);
	timescale = Cvar_Get ("timescale", "1", 0);
	fixedtime = Cvar_Get ("fixedtime", "0", 0);

// default to write log file to qconsole.log for console output in debug mode
#ifdef NDEBUG
	logfile_active = Cvar_Get ("logfile", "0", 0);
#else
	logfile_active = Cvar_Get ("logfile", "0", 0);
#endif // NDEBUG


	showtrace = Cvar_Get ("showtrace", "0", 0);
//unix - DEDICATED_ONLY
#ifdef DEDICATED_ONLY
	dedicated = Cvar_Get ("dedicated", "1", CVAR_NOSET);
#else
	dedicated = Cvar_Get ("dedicated", "0", CVAR_NOSET);
#endif

    cvar_t* cvarGibDamage;
	cvarGibDamage = Cvar_Get( "gib_damage", "40", CVAR_ARCHIVE );


	s = va("%d %s %s %s", VERSION, CPUSTRING, __DATE__, BUILDSTRING);
	Cvar_Get ("version", s, CVAR_SERVERINFO|CVAR_NOSET);

	if (dedicated->value)
		Cmd_AddCommand ("quit", Com_Quit);

	Sys_Init ();

	NET_Init ();
	Netchan_Init ();

	SV_Init ();
	CL_Init ();

//	g_cvarLogOutput = Cvar_Get( "LogOutput", "1", CVAR_ARCHIVE );

//	cvar_t* cvarLogFilePath;
//	cvarLogFilePath = Cvar_Get( "LogFilePath", "", CVAR_ARCHIVE );
//	DKLOG_Initialize( cvarLogFilePath->string );

//	DKLOG_Write( LOGTYPE_USERIO, 0.0, "QCommon_Init()" );

    // ISP: 06-25-99 build path table stuff
    cvar_t *cvarBuildPathTable = NULL;
#ifdef BUILD_PATH_TABLE

    cvarBuildPathTable = Cvar_Get( "BuildPathTable", "1", 0 );
    char *szPathTableFile = "";
    for ( i = 1; i < EXEGetArgc() - 1; i++ )
    {
		if ( !EXEGetArgv(i) || !EXEGetArgv(i + 1) )
        {
            continue;
        }

        if ( stricmp( "-PathTableFile", EXEGetArgv(i) ) == 0 )
		{
			if ( EXEGetArgv(i + 1) )
			{
				szPathTableFile = EXEGetArgv(i + 1);
			}
		}
    }

    if ( strlen( szPathTableFile ) == 0 )
    {
        strcpy( szPathTableFile, "PathTbl.dat" );
    }
    PATHTABLE_ParsePathFile( szPathTableFile );

#else 

    cvarBuildPathTable = Cvar_Get( "BuildPathTable", "0", 0 );

#endif BUILD_PATH_TABLE



/*
#ifndef	LOCKED_DEMO
	Com_Printf ("Executed %s.cfg.\n", dk_userName);
#endif
*/
	// add + commands from command line
	if (!Cbuf_AddLateCommands ())
	{	// if the user didn't give any commands, run default action
		if (!dedicated->value)
		{
/*
#ifdef	LOCKED_DEMO
			Cbuf_AddText ("d1\n");
			Cvar_ForceSet ("developer", "0");
#else

#if !DEDICATED_ONLY
			//	Nelno:	start demo loop if invulnerable.dat is found
			if (_access ("./data/invulnerable.dat", 0) != -1)
			{
				Cbuf_AddText ("d1\n");
				Com_Printf ("Started demo loop.\n");
			}
#ifndef	BOOT_MENU
			else
			{
				; //Cbuf_AddText ("d1\n");
			}

#else
			else
			{
				//	go to menu
				Cbuf_AddText ("menu_main\n");
			}
#endif
#endif
*/
			Cbuf_AddText ("menu_main\n");
		}
		else
		{
			Cbuf_AddText ("dedicated_start");
		}
//#endif
		Cbuf_Execute ();
	}
	else
	{	// the user asked for something explicit
		// so drop the loading plaque
		SCR_EndLoadingPlaque ();
	}

	//	Nelno:	init daikatana resources
	dk_InitResourcePath ();

	dk_Initializing = false;

	Com_Printf ("\n------- Daikatana Initialized -------\n\n");	
}

/*
=================
Qcommon_Frame
=================
*/

//unix - DEDICATED_ONLY
//#if !DEDICATED_ONLY
extern	viddef_t	viddef;//	Nelno:	yeah, I know this sucks
//#endif

char *SV_MapName();
char *CL_MapName();
char *szSVMapName;
char *szCLMapName;

void M_Menu_Main_f();
void Qcommon_Frame (int msec)
{
	char	*s;
	int		time_before, time_between, time_after;

	if (setjmp (abortframe) )
	{
		if( Cvar_VariableValue( "console" ) == 0 )
		{
			//M_Menu_Main_f();
		}
		return;			// an ERR_DROP was thrown
	}


#ifdef _DEBUG
#ifdef _MSC_VER
	// ISP - while debugging in single player mode, make this app sleep,
	// a hack
	//extern cvar_t *deathmatch;
	extern qboolean in_appactive;
    if(!dedicated || !dedicated->value)
	    if ( !in_appactive )//&& deathmatch->value == 0.0f )
	    {
		    // release mouse capture
		    extern void IN_Frame (void);
		    IN_Frame();
		    return;
	    }
#endif _MSC_VER
#endif _DEBUG


#ifdef BUILD_PATH_TABLE

    int nValue = PATHTABLE_GetThreadData();
    if ( nValue == 1 )
    {
        if ( !PATHTABLE_ExecuteNextMap() )
        {
            Com_Quit();
        }
    }

	extern void SV_RunGameFrame();
    SV_RunGameFrame();



#else BUILD_PATH_TABLE



	if (dk_rates != NULL && dk_rates->value != 0)
	{
		dk_StartTimer ();
	}

	if ( log_stats->modified )
	{
		log_stats->modified = false;
		if ( log_stats->value )
		{
			if ( log_stats_file )
			{
				fclose( log_stats_file );
				log_stats_file = 0;
			}
			log_stats_file = fopen( "stats.log", "w" );
			if ( log_stats_file )
			{
				fprintf( log_stats_file, "entities,dlights,parts,frame time\n" );
			}
		}
		else
		{
			if ( log_stats_file )
			{
				fclose( log_stats_file );
				log_stats_file = 0;
			}
		}
	}

	if (fixedtime->value)
	{
		msec = fixedtime->value;
	}
	else 
	if (timescale->value)
	{
		msec *= timescale->value;
		if (msec < 1)
		{
			msec = 1;
		}
	}

	if (showtrace->value)
	{
		extern	int c_traces, c_brush_traces;
		extern	int	c_pointcontents;

		Com_Printf ("%4i traces  %4i points\n", c_traces, c_pointcontents);
		c_traces = 0;
		c_brush_traces = 0;
		c_pointcontents = 0;
	}

	do
	{
		s = Sys_ConsoleInput();
		if (s)
		{
			Cbuf_AddText (va("%s\n",s));
		}
	} while (s);

    Cbuf_Execute();

    if (host_speeds->value)
	{
		time_before = Sys_Milliseconds ();
	}

	SV_Frame (msec);

	if (host_speeds->value)
	{
		time_between = Sys_Milliseconds ();		
	}

#ifndef DEDICATED_ONLY
//	szSVMapName = SV_MapName();
//	szCLMapName = CL_MapName();
#endif
	
	//CL_Frame (msec);

	if (host_speeds->value)
	{
		time_after = Sys_Milliseconds ();		
	}

	if (host_speeds->value)
	{
		int			all, sv, gm, cl, rf;

		all = time_after - time_before;
		sv = time_between - time_before;
		cl = time_after - time_between;
		gm = time_after_game - time_before_game;
		rf = time_after_ref - time_before_ref;
		sv -= gm;
		cl -= rf;
//		Com_Printf ("all:%3i sv:%3i gm:%3i cl:%3i rf:%3i\n", all, sv, gm, cl, rf);
		log_printf ("all:%3i sv:%3i gm:%3i cl:%3i rf:%3i\n", all, sv, gm, cl, rf);
	}	

	if (dk_rates != NULL && dk_rates->value != 0)
	{
		dk_StopTimer ();
//unix - DEDICATED_ONLY
//#if !DEDICATED_ONLY
		//dk_ShowRates (viddef.width, 64);
//#endif
	}

#endif BUILD_PATH_TABLE

}

/*
=================
Qcommon_Shutdown
=================
*/
void Qcommon_Shutdown (void)
{
	// hi, i'm the lone function
	ShutdownMemoryManager();
}


