
// q_shared.h -- included first by ALL program modules
#ifndef _DK_DEFINES_H
#define _DK_DEFINES_H

#ifdef _WIN32
// unknown pragmas are SUPPOSED to be ignored, but....
#pragma warning(disable : 4244)     // MIPS
#pragma warning(disable : 4136)     // X86
#pragma warning(disable : 4051)     // ALPHA

#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4305)		// truncation from const double to float

#endif _WIN32

#if defined _M_IX86 && !defined C_ONLY
#define id386	1
#else
#define id386	0
#endif

#if defined _M_ALPHA && !defined C_ONLY
#define idaxp	1
#else
#define idaxp	0
#endif

typedef unsigned char 		byte;
typedef unsigned short      uint16;
typedef int                 int32;
typedef unsigned int        uint32;


#ifdef __cplusplus
typedef int	qboolean;
#else
typedef enum {false, true}	qboolean;
#endif


#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

//#ifdef __cplusplus
//extern "C" {
//#endif 

// angle indexes
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

#define	MAX_STRING_CHARS	1024	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	80		// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS		512		// max length of an individual token

#define MAX_CONFIGSTR_LEN	128		// cek[2-3-00]: increased length of config strings
#define	MAX_QPATH			64		// max length of a quake game pathname
#define	MAX_OSPATH			128		// max length of a filesystem pathname

//
// per-level limits
//
#define	MAX_CLIENTS			256		// absolute limit
#define	MAX_EDICTS			1024	// must change protocol to increase more
//#define	MAX_EDICTS			512	// must change protocol to increase more
#define	MAX_LIGHTSTYLES		256
#define	MAX_PARTICLEVOLUMES	256		// maximum number of particle volume brushes allowed in the world
//#define	MAX_MODELS			256		// these are sent over the net as bytes
#define	MAX_MODELS			512		// these are sent over the net as bytes
//#define	MAX_SOUNDS			256		// so they cannot be blindly increased.. hehe.. whatever :)
#define	MAX_SOUNDS			512		
#define	MAX_IMAGES			256
#define	MAX_ITEMS			256


// game print flags
#define	PRINT_LOW			0		// pickup messages
#define	PRINT_MEDIUM		1		// death messages
#define	PRINT_HIGH			2		// critical messages
#define	PRINT_CHAT			3		// chat messages



#define	ERR_FATAL			0		// exit the entire game with a popup window
#define	ERR_DROP			1		// print to console and disconnect from game
#define	ERR_DISCONNECT		2		// don't kill server

#define	PRINT_ALL			0
#define PRINT_DEVELOPER		1		// only print when "developer 1"
#define PRINT_ALERT			2		

#define RANDOMIZE()         srand((int) time(NULL))


/*
==============================================================

MATHLIB

==============================================================
*/

typedef	int	fixed4_t;
typedef	int	fixed8_t;
typedef	int	fixed16_t;

#define sqr(x)			        ((x)*(x))
#ifndef M_PI
#define M_PI		            3.14159265358979323846	// matches value in gcc v2 math.h
#endif
#ifndef PI_OVER_ONEEIGHTY   
    #define PI_OVER_ONEEIGHTY   0.01745329251994329576
#endif
#define ONEEIGHTY_OVER_PI		57.29577951f
#ifndef DEG2RAD
    #define DEG2RAD( a )                    ( (a) * PI_OVER_ONEEIGHTY )
#endif
#define	nanmask (255<<23)
#define	IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)

#ifndef M_PI2
    #define M_PI2               6.28318530717958647692
#endif

//#ifdef __cplusplus
//}
//#endif 


#ifndef _MSC_VER
#ifndef _ASSERTE
#define _ASSERTE(x)		
#endif
#endif

#endif _DK_DEFINES_H

