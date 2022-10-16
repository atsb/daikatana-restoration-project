///////////////////////////////////////////////////////////////////////////////
//
//  someday this needs to completely replace snd_loc.h
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  headers for precompiling
//
///////////////////////////////////////////////////////////////////////////////

#include	<float.h>
#include	"client.h"
#include	"..\audio\AudioEngineImports.h"
#include	"mss.h"		// miles audio

///////////////////////////////////////////////////////////////////////////////
//
//  structs for miles audio system
//
///////////////////////////////////////////////////////////////////////////////

typedef struct sfxcache_s
{
	int 		length;
	int 		loopstart;
	int 		speed;			// not needed, because converted on load?
	int 		width;
	int 		stereo;
	byte		data[1];		// variable sized
//	byte		*data;		// variable sized
} sfxcache_t;

typedef struct sfx_s
{
	char 		name[MAX_QPATH];
	int			registration_sequence;
	sfxcache_t	*cache;
} sfx_t;

typedef struct dma_s
{
	int			channels;
	int			samples;				// mono samples in buffer
	int			submission_chunk;		// don't mix less than this #
	int			samplepos;				// in mono samples
	int			samplebits;
	int			speed;
	byte		*buffer;
} dma_t;

typedef struct stream_s
{
	HSTREAM		handle;
	HANDLE		hfile;
} stream_t;

typedef struct mp3_s
{
	HSTREAM		hMP3;
	char		song[MAX_QPATH];
} mp3_t;

///////////////////////////////////////////////////////////////////////////////
//
//  defines for audio
//
///////////////////////////////////////////////////////////////////////////////

// only begin attenuating sound volumes when outside the FULLVOLUME range
#define		SOUND_FULLVOLUME	80			
#define		SOUND_LOOPATTENUATE	0.003

#define		MAX_PROVIDERS		10			// max 3d providers, there are currently 6
#define		MAX_CHANNELS		64			// this is the max we will possibly support
#define		MAX_STREAMS			5			// ouch, if we ever hit this cd access will suck
//#define		MAX_MUSIC_CHANNELS	2			// two channels
#define		MAX_MP3_CHANNELS	8			// seven channels // SCG[10/18/99]: make that eight...

///////////////////////////////////////////////////////////////////////////////
//
//  class forward decls
//
///////////////////////////////////////////////////////////////////////////////

class CSample;

///////////////////////////////////////////////////////////////////////////////
//
//  external variables
//
///////////////////////////////////////////////////////////////////////////////

// -- defined in DllMain.c...
extern client_state_t*  pcl ;
extern entity_state_t*	pcl_parse_entities ;
extern client_static_t* pcls  ;

// from S_Calls.cpp
extern HDIGDRIVER	miles_driver;
extern int			max_channels;						// this is the max currently active
extern	CVector		listener_origin;
extern	CVector		listener_forward;
extern	CVector		listener_right;
extern	CVector		listener_up;
extern	dma_t		dma;
extern cvar_t*		s_volume;
//extern cvar_t*		s_nosound;
extern cvar_t*		s_khz;
extern cvar_t*		s_primary;
extern cvar_t*		s_3d_provider;
extern cvar_t*		s_attn_max;
extern cvar_t*		s_attn_min;
extern qboolean		using_3d;
extern H3DPOBJECT	listener;
extern stream_t		streams[MAX_STREAMS];
extern HPROVIDER	provider_3d;
extern bool			using_eax;

extern "C" const int kAudioEngineVersionNum;
extern const char* szAudioEngineDesc ;

///////////////////////////////////////////////////////////////////////////////
//
//  external functions called between cpp's
//
///////////////////////////////////////////////////////////////////////////////

// from support.cpp
void		S_AddLoopSounds (void);
void		S_Spatialize(CSample* samp);
qboolean	Sound_Init(void);			// init audio engine
void		Sound_Shutdown(void);		// kill audio engine
HPROVIDER	Find3dProvider(char* provider);

