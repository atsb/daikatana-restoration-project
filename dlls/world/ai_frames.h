#ifndef	_AI_FRAMES
#define	_AI_FRAMES

///////////////////////////////////////////////////////////////////////////////
//
// routines for managing frames in monsters/bots
//
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// monster numbers
///////////////////////////////////////////////////////////////////////////////

enum MONSTER_TYPE
{
	TYPE_CLIENT			= 1,		
	TYPE_MIKIKO			= 2,
	TYPE_SUPERFLY		= 3,
	TYPE_BOT			= 4,		//	deathmatch bot
	//
	// Episode 1
	//
	// !!! It is important to ORDER these for level of importance !!!
	// !!! monsters will look at the value for the type and will base !!!!
	// !!! their attacking through the other monster off of where each monster !!!!
	// !!! is in this enum.(the higher the number the more important and thus less likely a lower number will attack it)!!!

	TYPE_ROBOCROC		= 10,
	TYPE_SL_SKEET		= 11,
	TYPE_TH_SKEET		= 12,
	TYPE_FROGINATOR		= 13,
	TYPE_VERMIN			= 14,
	TYPE_SLUDGEMINION	= 15,
	TYPE_SURGEON		= 16,
	TYPE_INMATER		= 17,
	TYPE_PRISONER		= 18,
	TYPE_BATTLEBOAR		= 19,
	TYPE_RAGEMASTER		= 20,
	TYPE_TRACKATTACK	= 21,
	TYPE_MISHIMAGUARD   = 22,
	TYPE_LASERGAT		= 23,
	TYPE_CAMBOT			= 24,
	TYPE_PSYCLAW		= 25,
	TYPE_DEATHSPHERE	= 26,
	TYPE_PROTOPOD		= 27,
	TYPE_SKINNYWORKER	= 28,
	TYPE_FATWORKER	    = 29,
	//
	// Episode 2
	//
	// !!! It is important to ORDER these for level of importance !!!
	// !!! monsters will look at the value for the type and will base !!!!
	// !!! their attacking through the other monster off of where each monster !!!!
	// !!! is in this enum.(the higher the number the more important and thus less likely a lower number will attack it)!!!

	TYPE_FERRYMAN		= 30,
	TYPE_SKELETON		= 31,
	TYPE_SPIDER			= 32,
	TYPE_THIEF			= 33,
	TYPE_SATYR			= 34,
	TYPE_CENTURION		= 35,
	TYPE_HARPY			= 36,
	TYPE_COLUMN			= 37,
	TYPE_SIREN			= 38,
	TYPE_GRIFFON		= 39,
	TYPE_CERBERUS		= 40,
	TYPE_MEDUSA			= 41,
	TYPE_CYCLOPS		= 42,
	TYPE_MINOTAUR		= 43,
	TYPE_SMALLSPIDER    = 44,
    TYPE_KMINOS			= 45,

	//
	// Episode 3
	//
	// !!! It is important to ORDER these for level of importance !!!
	// !!! monsters will look at the value for the type and will base !!!!
	// !!! their attacking through the other monster off of where each monster !!!!
	// !!! is in this enum.(the higher the number the more important and thus less likely a lower number will attack it)!!!
	// Ordered
	TYPE_DRAGONEGG		= 50,
	TYPE_PRIEST			= 51,
	TYPE_ROTWORM		= 52,
	TYPE_PLAGUERAT		= 53,
	TYPE_BUBOID			= 54,
	TYPE_DOOMBAT		= 55,
	TYPE_FLETCHER		= 56,
	TYPE_DWARF			= 57,
	TYPE_LYCANTHIR		= 58,
	TYPE_CELESTRIL		= 59,
	TYPE_WYNDRAX		= 60,
	TYPE_BABYDRAGON		= 61,
	TYPE_NHARRE			= 62,
	TYPE_GARROTH		= 63,
	TYPE_KNIGHT1		= 64,
	TYPE_KNIGHT2		= 65,
	TYPE_WIZARD			= 66,
	TYPE_DRAGON			= 67,
	TYPE_STAVROS		= 68,

	//
	// Episode 4
	// 
	// !!! It is important to ORDER these for level of importance !!!
	// !!! monsters will look at the value for the type and will base !!!!
	// !!! their attacking through the other monster off of where each monster !!!!
	// !!! is in this enum.(the higher the number the more important and thus less likely a lower number will attack it)!!!
	//Ordered
	TYPE_SEAGULL		= 70,
	TYPE_GOLDFISH		= 71,
	TYPE_SHARK			= 72,
	TYPE_SQUID			= 73,
	TYPE_BLACKPRIS		= 74,
	TYPE_WHITEPRIS		= 75,
	TYPE_LABMONKEY		= 76,
	TYPE_FEMGANG		= 77,
	TYPE_PIPEGANG		= 78,
	TYPE_ROCKETGANG		= 80,
	TYPE_UZIGANG		= 81,
	TYPE_CHAINGANG		= 82,
	TYPE_SDIVER			= 83,
	TYPE_CRYOTECH 		= 84,
	TYPE_NAVYSEAL		= 85,
	TYPE_SEALGIRL		= 86,
	TYPE_SEALCOMMANDO	= 87,
	TYPE_SEALCAPTAIN	= 88,
	TYPE_ROCKETMP		= 89,
	TYPE_KAGE			= 90,


	// this type is onyl used in cinematics and should never be assigned to anything appearing in the game
	TYPE_CINE_ONLY		= 91,

	TYPE_FIREFLY		= 92,
	TYPE_WISP			= 93,
	TYPE_PIPERAT		= 94,
	TYPE_MIKIKOMON		= 95,
	TYPE_MIKIKOFLY		= 96,
	TYPE_GHOST			= 97,
	TYPE_DOPEFISH		= 98,

};

// string table filled with monster model dkm files
//extern char szMonsterModelName[90][2][32];

typedef struct monsterName_s
{
	char	className[24];
	char	modelName[40];
} monsterName_t;

#define MAX_MONSTERS_IN_LIST	99		// This NUMBER should be 1 above the list ALWAYS!!!!! <NSS>
extern monsterName_t monsterList[MAX_MONSTERS_IN_LIST];
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// prototypes
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int		CacheMonsterModel ( char *className );
//char*	GetMonsterModelName ( char *className );
int GetMonsterInfo( const char *szClassName, char *szModelName );

int		ai_get_frames			(userEntity_t *self, char *seq_name, int *startFrame, int *endFrame);
int		ai_get_sequence			(userEntity_t *self, char *name, frameData_t **sequence);
int		ai_get_sequences		(userEntity_t *self);
//void	ai_set_sequence_sounds	(userEntity_t *self, int frame1, char *sound1, int frame2, char *sound2, int nAttackFrame = 1);
//void	ai_set_trans			(userEntity_t *self, int start, int main, int main_num, int end);
//void	ai_post_trans			(userEntity_t *self, unsigned long flags);
//void	ai_set_sequence_data	(userEntity_t *self, frameData_t *data, int sf1, char *name1, int sf2, 
//						char *name2, int pret_type, int pret, int post_type, int postt, unsigned long int flags);
//void	ai_init_sequence		(userEntity_t *self, frameData_t *data, int seq_type, int seq_num, bool bOverride = FALSE);
//void	ai_set_frames			(userEntity_t *self, int cur_seq_type, int cur_seq_num);
//void	ai_force_sequence		(userEntity_t *self, int seq_type, int seq_num, int trans_pre_type, int trans_pre, 
//								int trans_post_type, int trans_post, int sf1, char *sname1, int sf2, char *sname2);

void	ai_die_sound			(userEntity_t *self);
void	ai_wander_sound			(userEntity_t *self);
void	ai_ambient_sound		( userEntity_t *self );

void	ai_sight_sound			(userEntity_t *self);
void	ai_pain_sound			(userEntity_t *self);
void	ai_frame_sounds			(userEntity_t *self);

void	ai_precache_global_models (void);

int AI_StartSequenceInReverse( userEntity_t *self, frameData_t *pNewSequence, int nFrameLoopFlag = FRAME_ONCE );
int AI_StartSequenceInReverse( userEntity_t *self, const char *animName, int nFrameLoopFlag );
int AI_StartSequence( userEntity_t *self, frameData_t *pSequence, int nFrameLoopFlag = FRAME_ONCE, int bOverRide = FALSE );
int AI_StartSequence( userEntity_t *self, const char *animName, int nFrameLoopFlag = FRAME_ONCE );
int AI_StartSequence( userEntity_t *self, frameData_t *pNewSequence, int nStartIndex, int nEndIndex, int nFrameLoopFlag = FRAME_ONCE );
int AI_StartSequence( userEntity_t *self, const char *animName, int nStartIndex, int nEndIndex, int nFrameLoopFlag = FRAME_ONCE );
int AI_ForceSequence( userEntity_t *self, frameData_t *pSequence, int nFrameLoopFlag = FRAME_ONCE, int bOverRide = FALSE );
int AI_ForceSequence( userEntity_t *self, const char *animName, int nFrameLoopFlag = FRAME_ONCE );

void AI_StartTransition( userEntity_t *self, frameData_t *pNewSequence );
void AI_StopCurrentSequence( userEntity_t *self );

int AI_ForceSequenceInReverse( userEntity_t *self, frameData_t *pNewSequence, int nFrameLoopFlag );
int AI_ForceSequenceInReverse( userEntity_t *self, const char *animName, int nFrameLoopFlag );

int AI_ComputeAnimationTime( frameData_t *pSequence );

//-----------------------------------------------------------------------------
// inline functions defined below
//-----------------------------------------------------------------------------

DllExport frameData_t *FRAMES_GetSequence( userEntity_t *self, const char *name );

#ifdef _DEBUG

float FRAMES_ComputeFrameScale( playerHook_t *hook );
void AI_ToggleSound();
int AI_IsSoundDisabled();

#else _DEBUG

#include "collect.h"

__inline float FRAMES_ComputeFrameScale( playerHook_t *hook )
{
	return 1.0f;
}

// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
extern int bDisableAISound;

__inline void AI_ToggleSound()
{
	if ( bDisableAISound )
	{
		bDisableAISound = FALSE;
	}
	else
	{
		bDisableAISound = TRUE;
	}
}

__inline int AI_IsSoundDisabled()
{
	return bDisableAISound;
}

#endif _DEBUG

#endif _AI_FRAMES
