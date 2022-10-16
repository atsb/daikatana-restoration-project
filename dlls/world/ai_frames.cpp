// ==========================================================================
//
//  File:		ai_frames
//  Contents:
//	routines for extracting and animating frames in actor entities
//  Author:
//
// ==========================================================================
#include "world.h"
#include "ai_common.h"
#include "ai_frames.h"
//#include "floater.h"// SCG[1/23/00]: not used
#include "ai_func.h"
#include "SequenceMap.h"
#include "ai_utils.h"
#include "collect.h"

/* ***************************** define types ****************************** */

///////////////////////////////////////////////////////////////////////////////
//	Monster Model definitions
//
//	Note: this list is used so other parts of world.dll can get the model 
//  names (i.e. pre-caching certain models during loading in target_spawn_monster)
//  Also, each monster spawn function should utilize this table when setting
//  a monster's model index.
//
//	p.s.  I had a lot of fucking fun creating this table...NOT  -amw
///////////////////////////////////////////////////////////////////////////////

monsterName_t monsterList[MAX_MONSTERS_IN_LIST] =
{
    {"",						""},									//	*EMPTY*				= 0
    {"Hiro",					"models/global/m_hiro.dkm"},			//	TYPE_CLIENT			= 1		
    {"Mikiko",					"models/global/m_mikiko.dkm"},			//	TYPE_MIKIKO			= 2
    {"Superfly",				"models/global/m_superfly.dkm"},		//	TYPE_SUPERFLY		= 3
    {"",						""},									//	TYPE_BOT			= 4	
    {"",						""},									//	*EMPTY*				= 5
    {"",						""},									//	*EMPTY*				= 6
    {"",						""},									//	*EMPTY*				= 7
    {"",						""},									//	*EMPTY*				= 8
    {"",						""},									//	*EMPTY*				= 9

	//*** Episode 1 ***//
	
	{"monster_crox",			"models/e1/m_croco.dkm"},				//	TYPE_ROBOCROC		= 10	
	{"monster_slaughterskeet",	"models/e1/m_skeeter.dkm"},				//	TYPE_SL_SKEET		= 11
    {"monster_thunderskeet",	""},									//	TYPE_TH_SKEET		= 12
	{"monster_froginator",		"models/e1/m_frog.dkm"},				//	TYPE_FROGINATOR		= 13
	{"monster_venomvermin",		"models/e1/m_vermin.dkm"},				//	TYPE_VERMIN			= 14
	{"monster_sludgeminion",	"models/e1/m_sludgeminion.dkm"},        //	TYPE_SLUDGEMINION	= 15
	{"monster_surgeon",			"models/e1/m_mwsurgeon.dkm"},			//  TYPE_SURGEON		= 16
    {"monster_inmater",			"models/e1/m_inmater.dkm"},				//	TYPE_INMATER		= 17
    {"monster_prisoner",		"models/e1/m_priza.dkm"},				//	TYPE_PRISONER		= 18
	{"monster_battleboar",		"models/e1/m_bboar.dkm"},				//	TYPE_BATTLEBOAR		= 19
	{"monster_ragemaster",		"models/e1/m_ragemaster.dkm"},			//	TYPE_RAGEMASTER		= 20
// SCG[12/5/99]:     {"monster_trackattack",		NULL},									//	TYPE_TRACKATTACK	= 21
    {"monster_mishimaguard",    "models/e1/m_mwguard.dkm"},             //  TYPE_MISHIMAGUARD   = 22
    {"monster_lasergat",		""},									//	TYPE_LASERGAT		= 23
	{"monster_cambot",			"models/e1/m_cambot.dkm"},				//	TYPE_CAMBOT			= 24
    {"monster_psyclaw",			"models/e1/m_psyclaw.dkm"},				//	TYPE_PSYCLAW		= 25
    {"monster_deathsphere",		"models/e1/m_dsphere.dkm"},				//	TYPE_DEATHSPHERE	= 26
    {"monster_protopod",		"models/e1/m_protopod.dkm"},			//	TYPE_PROTOPOD		= 27
    {"monster_skinnyworker",	"models/e1/m_mwskny.dkm"},				//	TYPE_SKINNYWORKER	= 28
    {"monster_fatworker",	    "models/e1/m_mwfaty.dkm"},				//	TYPE_FATWORKER	    = 29
								
	//*** Episode 2 ***//		
								
	{"monster_ferryman",		"models/e2/m_ferryman.dkm"},			//	TYPE_FERRYMAN		= 30	
	{"monster_skeleton",		"models/e2/m_skeleton.dkm"},			//	TYPE_SKELETON		= 31
	{"monster_spider",			"models/e2/m_spider.dkm"},				//	TYPE_SPIDER			= 32
    {"monster_thief",			"models/e2/m_thief.dkm"},				//	TYPE_THIEF			= 33
	{"monster_satyr",			"models/e2/m_satyr.dkm"},				//	TYPE_SATYR			= 34
	{"monster_centurion",		"models/e3/m_centurion.dkm"},			//	TYPE_CENTURION		= 35
	{"monster_harpy",			"models/e2/m_harpy.dkm"},				//	TYPE_HARPY			= 36
	{"monster_column",			"models/e2/m_column.dkm"},				//	TYPE_COLUMN			= 37
    {"monster_siren",			""},									//	TYPE_SIREN			= 38
	{"monster_griffon",			"models/e2/m_griffon.dkm"},				//	TYPE_GRIFFON		= 39
	{"monster_cerberus",		"models/e2/m_cerberus.dkm"},			//	TYPE_CERBERUS		= 40
	{"monster_medusa",			"models/e3/m_medusa.dkm"},				//	TYPE_MEDUSA			= 41
// SCG[12/5/99]: 	{"monster_cyclops",			"models/e2/m_cyclops.dkm"},				//	TYPE_CYCLOPS		= 42
// SCG[2/13/00]: 	{"monster_minotaur",		"models/e2/m_minotaur.dkm"},			//	TYPE_MINOTAUR		= 43
    {"monster_smallspider",		"models/e2/m_spider.dkm"},				//	TYPE_SMALLSPIDER	= 44
// SCG[2/13/00]: 	{"monster_kingminos",		"models/e2/m_kminos.dkm"},				//	TYPE_KMINOS			= 45
    {"",						""},									//	*EMPTY*				= 46
    {"",						""},									//	*EMPTY*				= 47
    {"",						""},									//	*EMPTY*				= 48
    {"",						""},									//	*EMPTY*				= 49
								
	//*** Episode 3 ***//		
								
	{"monster_priest",			"models/e3/m_priest.dkm"},				//	TYPE_PRIEST			= 50	
	{"monster_buboid",			"models/e3/m_buboid.dkm"},				//	TYPE_BUBOID			= 51
	{"monster_doombat",			"models/e3/m_doombat.dkm"},				//	TYPE_DOOMBAT		= 52
	{"monster_rotworm",			"models/e3/m_rotworm.dkm"},				//	TYPE_ROTWORM		= 53
	{"monster_plague_rat",		"models/e3/m_prat.dkm"},				//	TYPE_PLAGUERAT		= 54
	{"monster_dwarf",			"models/e3/m_dwarf.dkm"},				//	TYPE_DWARF			= 55
	{"monster_fletcher",		"models/e3/m_fletcher.dkm"},			//	TYPE_FLETCHER		= 56
// SCG[2/13/00]:     {"monster_dragonegg",		""},									//	TYPE_DRAGONEGG		= 57
// SCG[12/5/99]:     {"monster_babydragon",		NULL},									//	TYPE_BABYDRAGON		= 58
	{"monster_lycanthir",		"models/e3/m_lycanthir.dkm"},			//	TYPE_LYCANTHIR		= 59
    {"",						""},									//	TYPE_CELESTRIL		= 60
	{"monster_wyndrax",			"models/e3/m_wyndrax.dkm"},				//	TYPE_WYNDRAX		= 61
    {"monster_nharre",			"models/e3/m_nharre.dkm"},				//	TYPE_NHARRE			= 62
	{"monster_garroth",			"models/e3/m_gharroth.dkm"},			//	TYPE_GARROTH		= 63
	{"monster_dragon",			"models/e3/m_dragon.dkm"},				//	TYPE_DRAGON			= 64
	{"monster_knight1",			"models/e3/m_knight1.dkm"},				//	TYPE_KNIGHT1		= 65
	{"monster_knight2",			"models/e3/m_knight2.dkm"},				//	TYPE_KNIGHT2		= 66
	{"monster_stavros",			"models/e3/m_stavros.dkm"},				//	TYPE_STAVROS		= 67
	{"",						""},									//	TYPE_WIZARD			= 68
    {"",						""},									//	*EMPTY*				= 69
								
	//*** Episode 4 ***//		
								
	{"monster_blackprisoner",	"models/e4/m_bpris.dkm"},				//	TYPE_BLACKPRIS		= 70	
    {"monster_whiteprisoner",	""},									//	TYPE_WHITEPRIS		= 71
	{"monster_chaingang",		"models/e4/m_chgang.dkm"},				//	TYPE_CHAINGANG		= 72
	{"monster_femgang",			"models/e4/m_femgang.dkm"},				//	TYPE_FEMGANG		= 73
    {"",						""},									//	TYPE_PIPEGANG		= 74
	{"monster_rocketdude",		"models/e4/m_rockgang.dkm"},			//	TYPE_ROCKETGANG		= 75
	{"monster_uzigang",			"models/e4/m_uzigng.dkm"},				//	TYPE_UZIGANG		= 76
	{"monster_labmonkey",		"models/e4/m_labmonkey.dkm"},			//	TYPE_LABMONKEY		= 77
	// mdm99.04.15 - this monster is actually episode 1, but if i move it the number is screwed.  lUshIE
	{"monster_cryotech",		"models/e1/m_cryotech.dkm"},			//	TYPE_CRYOTECH 		= 78
    {"",						""},									//	TYPE_NAVYSEAL		= 79
	{"monster_shark",			"models/e4/m_shark.dkm"},				//	TYPE_SHARK			= 80
    {"monster_squid",			""},									//	TYPE_SQUID			= 81
    {"",						""},									//	TYPE_GOLDFISH		= 82
	{"monster_sealcommando",	"models/e4/m_scomndo.dkm"},				//	TYPE_SEALCOMMANDO	= 83
	{"monster_sealcaptain",		"models/e4/m_sealcap.dkm"},				//	TYPE_SEALCAPTAIN	= 84
	{"monster_sealgirl",		"models/e4/m_sgirl.dkm"},				//	TYPE_SEALGIRL		= 85
	{"monster_rocketmp",		"models/e4/m_rocketmp.dkm"},			//	TYPE_ROCKETMP		= 86
	{"monster_kage",			"models/e4/m_kage.dkm"},				//	TYPE_KAGE			= 87
// SCG[12/5/99]: 	{"monster_sdiver",			"models/e4/m_sdiver.dkm"},				//	TYPE_SDIVER			= 88
	{"monster_piperat",			"models/e4/m_piperat.dkm"},			//	TYPE_PIPERAT		= 89
    //Non-Episode Based monsters<nss>
	{"monster_firefly",			"models/e1/d1_firefly.sp2"},			//	TYPE_FIREFLY		= 91
	{"monster_wisp",			"models/e3/we_wisp.sp2"},				//	TYPE_FIREFLY		= 92
	{"monster_mikiko",			"models/global/m_smikiko.dkm"},			//	TYPE_MIKIKOMON		= 95
	{"monster_ghost",			"models/e3/we_ghost.dkm"},				//	TYPE_GHOST			= 96
	{"",						""},									//  *EMPTY*				= 97
};

// UNDEFINED MONSTERS  i.e. I don't know which ones these are..

/*
"monster_tarantula",
"monster_thief",
"monster_gang1",
"monster_gang2",
*/

///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */
/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */


///////////////////////////////////////////////////////////////////////////////
//	Function implementation
///////////////////////////////////////////////////////////////////////////////



// ----------------------------------------------------------------------------
//
// Name:		CacheMonsterModel
// Description:
// Input:
// Output:
// Note:
//	Pre-cache a monster model given the monster's classname
//
//	return's the model index
//
// ----------------------------------------------------------------------------
int	CacheMonsterModel( char *className )
{
	for (int i = 0; i < MAX_MONSTERS_IN_LIST; i++)
	{
		// did we find the monster?
		if (!stricmp(monsterList[i].className, className))		
		{
			char *szModelName = AIATTRIBUTE_GetModelName( monsterList[i].className );
			_ASSERTE( szModelName );
			int modelIndex = gstate->ModelIndex( szModelName );
			//int modelIndex = gstate->ModelIndex( monsterList[i].modelName );
//			gstate->PreCacheModel( modelIndex, szModelName );
			return modelIndex;
		}
	}

	return -1;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_precache_global_models
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_precache_global_models (void)
{
	for (int i = 0; i < GIB_NUM_MODELS; i++)
	{
		gstate->ModelIndex (gib_models [i]);
	}

#ifdef _DEBUG
	for (i = 0; i < DEBUG_NUM_MODELS; i++)
	{
		gstate->ModelIndex (debug_models [i]);
	}
#endif
}

// ----------------------------------------------------------------------------
//
// Name:		GetMonsterInfo
// Description:
// Input:
//				szClassName			=> class name of the monster
// Output:
//				int					=> monster type
//				char* szModelName	=> name of the model
// Note:
//
// ----------------------------------------------------------------------------
int GetMonsterInfo( const char *szClassName, char *szModelName )
{
	for ( int i = 0; i < MAX_MONSTERS_IN_LIST; i++ )
	{
		if ( _stricmp(monsterList[i].className, szClassName) == 0 )
		{
			strcpy( szModelName, monsterList[i].modelName );
			
			return i;
		}
	}

	return -1;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_get_sequences
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int ai_get_sequences( userEntity_t *self )
{
	CSequenceList *pSequenceList = SEQUENCEMAP_Lookup( self->s.modelindex );
	if ( pSequenceList )
	{
		pSequenceList->MapAnimationNameToSequences( self );

		return TRUE;
	}
	else
	{
		int nNumSequences = com->GetFrameData( self, NULL );
		if ( nNumSequences > 0 )
		{
			// allocate all sequences
			pSequenceList = new CSequenceList( nNumSequences );
			frameData_t *pFrames = pSequenceList->GetSequences();
			com->GetFrameData( self, pFrames );

			pSequenceList->MapAnimationNameToSequences( self );
			SEQUENCEMAP_Add( self->s.modelindex, pSequenceList );
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_set_sequence_sounds
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void ai_set_sequence_sounds(userEntity_t *self,
                                   int frame1, char *sound1, 
                                   int frame2, char *sound2,
                                   int nAttackFrame1, int nAttackFrame2,
                                   float fAnimChance, float fSound2Chance )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	// frames on which we want to play a sound

	if (fSound2Chance > 0.0f && rnd() < fSound2Chance)
	{
		// occasionally switch up sound2 with frame1 by chance
		self->s.frameInfo.sound1Frame = -1;
		self->s.frameInfo.sound2Frame = frame1;
	}
	else
	{
		self->s.frameInfo.sound1Frame = frame1;
		self->s.frameInfo.sound2Frame = frame2;
	}

	self->s.frameInfo.nAttackFrame1 = nAttackFrame1;
	self->s.frameInfo.nAttackFrame2 = nAttackFrame2;

	//&&&FIXME: this sends messages to the client.. EXPENSIVE
	//self->s.frameInfo.sound1Index = gstate->SoundIndex (sound1);
	//self->s.frameInfo.sound2Index = gstate->SoundIndex (sound2);

	hook->sound1 = sound1;
	hook->sound2 = sound2;

	self->s.frameInfo.frameState &= ~FRSTATE_PLAYSOUND1;
	self->s.frameInfo.frameState &= ~FRSTATE_PLAYSOUND2;
	self->s.frameInfo.frameState &= ~FRSTATE_PLAYATTACK1;
	self->s.frameInfo.frameState &= ~FRSTATE_PLAYATTACK2;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartSequenceInReverse
// Description:
// Input:
// Output:
//				TRUE	=> started the requested sequence without
//				FALSE	=> have to wait until current sequence finishes or started a
//						   transition
// Note:
//				starts a sequence playing making sure the current sequence finishes
//				before starting a new one, but only for non-client AIs
//
// ----------------------------------------------------------------------------
int AI_StartSequenceInReverse( userEntity_t *self, frameData_t *pNewSequence, int nFrameLoopFlag )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !pNewSequence )
	{
		return FALSE;
	}

	if ( hook->type != TYPE_CLIENT && hook->type != TYPE_BOT )
	{
		// see if current animation has completed
		if ( hook->cur_sequence && self->s.frameInfo.frameState != 0 && !AI_IsEndAnimation( self ) )
		{
			AI_AddNewTaskAtFront( self, TASKTYPE_FINISHCURRENTSEQUENCE );
			return FALSE;
		}

		if ( hook->cur_sequence != pNewSequence )
		{
			CTransitionSequences *pTransitions = hook->pTransitions;
			if ( pTransitions )
			{
				frameData_t *pTransitionSequence = pTransitions->FindTransitionSequence( hook->cur_sequence, pNewSequence );
				if ( pTransitionSequence )
				{
					AI_AddNewTaskAtFront( self, TASKTYPE_SEQUENCETRANSITION, pTransitionSequence );
					return FALSE;
				}
			}
		}
	}

	hook->cur_sequence = pNewSequence;

	ai_set_sequence_sounds( self, 
							pNewSequence->soundframe1, 
							pNewSequence->sound1, 
							pNewSequence->soundframe2, 
							pNewSequence->sound2, 
							pNewSequence->nAttackFrame1,
							pNewSequence->nAttackFrame2,
							pNewSequence->fAnimChance,
							pNewSequence->fSound2Chance );

	self->s.frame					= pNewSequence->last;
	self->s.frameInfo.frameState	= 0;
	self->s.frameInfo.startFrame	= pNewSequence->last;
	self->s.frameInfo.endFrame		= pNewSequence->first;
	self->s.frameInfo.frameFlags	= FRAME_FORCEINDEX;
    self->s.frameInfo.frameFlags    |= nFrameLoopFlag;
	self->s.frameInfo.frameInc		= -1;
	self->s.frameInfo.modelAnimSpeed = pNewSequence->fModelAnimSpeed;

	//	don't reset animation if already in this sequence
	if (self->s.frame > self->s.frameInfo.startFrame || self->s.frame < self->s.frameInfo.endFrame)
	{
		self->s.frame = self->s.frameInfo.startFrame;
	}

	return TRUE;
}

int AI_StartSequenceInReverse( userEntity_t *self, const char *animName, int nFrameLoopFlag )
{
	frameData_t *pSequence = FRAMES_GetSequence( self, animName );
	if ( !pSequence )
	{
		gstate->Con_Dprintf("Missing %s frame data for %s.\n", animName, self->className);
		return FALSE;
	}

	return AI_StartSequenceInReverse( self, pSequence, nFrameLoopFlag );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ForceSequenceInReverse
// Description:
// Input:
// Output:
//				TRUE	=> started the requested sequence without
//				FALSE	=> have to wait until current sequence finishes or started a
//						   transition
// Note:
//
// ----------------------------------------------------------------------------
int AI_ForceSequenceInReverse( userEntity_t *self, frameData_t *pNewSequence, int nFrameLoopFlag )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !pNewSequence )
	{
		return FALSE;
	}

	hook->cur_sequence = pNewSequence;

	ai_set_sequence_sounds( self, 
							pNewSequence->soundframe1, 
							pNewSequence->sound1, 
							pNewSequence->soundframe2, 
							pNewSequence->sound2, 
							pNewSequence->nAttackFrame1,
							pNewSequence->nAttackFrame2,
							pNewSequence->fAnimChance,
							pNewSequence->fSound2Chance );

	self->s.frame					= pNewSequence->last;
	self->s.frameInfo.frameState	= 0;
	self->s.frameInfo.startFrame	= pNewSequence->last;
	self->s.frameInfo.endFrame		= pNewSequence->first;
	self->s.frameInfo.frameFlags	= FRAME_FORCEINDEX;
    self->s.frameInfo.frameFlags    |= nFrameLoopFlag;
	self->s.frameInfo.frameInc		= -1;
	self->s.frameInfo.modelAnimSpeed = pNewSequence->fModelAnimSpeed;

	//	don't reset animation if already in this sequence
	if (self->s.frame > self->s.frameInfo.startFrame || self->s.frame < self->s.frameInfo.endFrame)
	{
		self->s.frame = self->s.frameInfo.startFrame;
	}

	return TRUE;
}

int AI_ForceSequenceInReverse( userEntity_t *self, const char *animName, int nFrameLoopFlag )
{
	frameData_t *pSequence = FRAMES_GetSequence( self, animName );
	if ( !pSequence )
	{
		gstate->Con_Dprintf("Missing %s frame data for %s.\n", animName, self->className);
		return FALSE;
	}

	return AI_ForceSequenceInReverse( self, pSequence, nFrameLoopFlag );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartSequence
// Description:
// Input:
// Output:
//				TRUE	=> started the requested sequence without
//				FALSE	=> have to wait until current sequence finishes or started a
//						   transition
// Note:
//				starts a sequence playing making sure the current sequence finishes
//				before starting a new one, but only for non-client AIs
//
// ----------------------------------------------------------------------------
int AI_StartSequence( userEntity_t *self, frameData_t *pNewSequence, int nFrameLoopFlag, int bOverRide )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !pNewSequence )
	{
		return FALSE;
	}

	if ( hook->type != TYPE_CLIENT && hook->type != TYPE_BOT )
	{
		// see if current animation has completed
		if ( hook->cur_sequence && self->s.frameInfo.frameState != 0 && !AI_IsEndAnimation( self ) )
		{
			AI_AddNewTaskAtFront( self, TASKTYPE_FINISHCURRENTSEQUENCE );
			return FALSE;
		}

		if ( hook->cur_sequence != pNewSequence )
		{
			CTransitionSequences *pTransitions = hook->pTransitions;
			if ( pTransitions )
			{
				frameData_t *pTransitionSequence = pTransitions->FindTransitionSequence( hook->cur_sequence, pNewSequence );
				if ( pTransitionSequence )
				{
					AI_AddNewTaskAtFront( self, TASKTYPE_SEQUENCETRANSITION, pTransitionSequence );
					return FALSE;
				}
			}
		}
	}

	hook->cur_sequence = pNewSequence;

	ai_set_sequence_sounds( self, 
							pNewSequence->soundframe1, 
							pNewSequence->sound1, 
							pNewSequence->soundframe2, 
							pNewSequence->sound2, 
							pNewSequence->nAttackFrame1,
							pNewSequence->nAttackFrame2,
							pNewSequence->fAnimChance,
							pNewSequence->fSound2Chance );

	self->s.frame					= pNewSequence->first;
	self->s.frameInfo.frameState	= 0;
	self->s.frameInfo.startFrame	= pNewSequence->first;
	self->s.frameInfo.endFrame		= pNewSequence->last;
	self->s.frameInfo.frameFlags	= FRAME_FORCEINDEX;
    self->s.frameInfo.frameFlags    |= nFrameLoopFlag;
	self->s.frameInfo.frameInc		= 1;
	self->s.frameInfo.modelAnimSpeed = pNewSequence->fModelAnimSpeed;

	//	don't reset animation if already in this sequence
	if (bOverRide || self->s.frame < self->s.frameInfo.startFrame || self->s.frame > self->s.frameInfo.endFrame)
	{
		self->s.frame = self->s.frameInfo.startFrame;
	}

	return TRUE;
}

int AI_StartSequence( userEntity_t *self, const char *animName, int nFrameLoopFlag )
{
	frameData_t *pSequence = FRAMES_GetSequence( self, animName );
	if ( !pSequence )
	{
		gstate->Con_Dprintf("Missing %s frame data for %s.\n", animName, self->className);
		return FALSE;
	}

	return AI_StartSequence( self, pSequence, nFrameLoopFlag );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartSequence
// Description:
// Input:
//				nStartIndex	=> absolute index into the passed in sequence
//				nEndIndex	=> can be < 0 
//							   if < 0, sequence is played to the end of sequence
// Output:
//				TRUE	=> started the requested sequence without
//				FALSE	=> have to wait until current sequence finishes or started a
//						   transition
// Note:
//				starts a sequence playing making sure the current sequence finishes
//				before starting a new one, but only for non-client AIs
//
// ----------------------------------------------------------------------------
int AI_StartSequence( userEntity_t *self, frameData_t *pNewSequence, int nStartIndex, int nEndIndex, int nFrameLoopFlag )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !pNewSequence )
	{
		return FALSE;
	}

	if ( hook->type != TYPE_CLIENT && hook->type != TYPE_BOT )
	{
		// see if current animation has completed
		if ( hook->cur_sequence && self->s.frameInfo.frameState != 0 && !AI_IsEndAnimation( self ) )
		{
			AI_AddNewTaskAtFront( self, TASKTYPE_FINISHCURRENTSEQUENCE );
			return FALSE;
		}

		if ( hook->cur_sequence != pNewSequence )
		{
			CTransitionSequences *pTransitions = hook->pTransitions;
			if ( pTransitions )
			{
				frameData_t *pTransitionSequence = pTransitions->FindTransitionSequence( hook->cur_sequence, pNewSequence );
				if ( pTransitionSequence )
				{
					AI_AddNewTaskAtFront( self, TASKTYPE_SEQUENCETRANSITION, pTransitionSequence );
					return FALSE;
				}
			}
		}
	}

	hook->cur_sequence = pNewSequence;

	ai_set_sequence_sounds( self, 
							pNewSequence->soundframe1, 
							pNewSequence->sound1, 
							pNewSequence->soundframe2, 
							pNewSequence->sound2, 
							pNewSequence->nAttackFrame1,
							pNewSequence->nAttackFrame2,
							pNewSequence->fAnimChance,
							pNewSequence->fSound2Chance );

	//_ASSERTE( nStartIndex >= pNewSequence->first && nStartIndex <= pNewSequence->last );

	self->s.frame					= nStartIndex;
	self->s.frameInfo.frameState	= 0;
	self->s.frameInfo.startFrame	= nStartIndex;
	self->s.frameInfo.endFrame		= pNewSequence->last;
	if ( nEndIndex >= pNewSequence->first && nEndIndex <= pNewSequence->last )
	{
		self->s.frameInfo.endFrame	= nEndIndex;
	}
	self->s.frameInfo.frameFlags	= FRAME_FORCEINDEX;
    self->s.frameInfo.frameFlags    |= nFrameLoopFlag;
	self->s.frameInfo.frameInc		= 1;
	self->s.frameInfo.modelAnimSpeed = pNewSequence->fModelAnimSpeed;

	//	don't reset animation if already in this sequence
	if (self->s.frame < self->s.frameInfo.startFrame || self->s.frame > self->s.frameInfo.endFrame)
	{
		self->s.frame = self->s.frameInfo.startFrame;
	}

	return TRUE;
}

int AI_StartSequence( userEntity_t *self, const char *animName, int nStartIndex, int nEndIndex, int nFrameLoopFlag )
{
	frameData_t *pSequence = FRAMES_GetSequence( self, animName );
	if ( !pSequence )
	{
		gstate->Con_Dprintf("Missing %s frame data for %s.\n", animName, self->className);
		return FALSE;
	}

	return AI_StartSequence( self, pSequence, nStartIndex, nEndIndex, nFrameLoopFlag );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_ForceSequence
// Description:
// Input:
// Output:
// Note:
//				force a new sequence
//
// ----------------------------------------------------------------------------
int AI_ForceSequence( userEntity_t *self, frameData_t *pNewSequence, int nFrameLoopFlag, int bOverRide )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( !pNewSequence )
	{
		return FALSE;
	}

	hook->cur_sequence = pNewSequence;

	ai_set_sequence_sounds( self, 
							pNewSequence->soundframe1, 
							pNewSequence->sound1, 
							pNewSequence->soundframe2, 
							pNewSequence->sound2, 
							pNewSequence->nAttackFrame1,
							pNewSequence->nAttackFrame2,
							pNewSequence->fAnimChance,
							pNewSequence->fSound2Chance );

	self->s.frame					= pNewSequence->first;
	self->s.frameInfo.frameState	= 0;
	self->s.frameInfo.startFrame	= pNewSequence->first;
	self->s.frameInfo.endFrame		= pNewSequence->last;
	self->s.frameInfo.frameFlags	= FRAME_FORCEINDEX;
    self->s.frameInfo.frameFlags    |= nFrameLoopFlag;
	self->s.frameInfo.frameInc		= 1;
	self->s.frameInfo.modelAnimSpeed = pNewSequence->fModelAnimSpeed;

	//	don't reset animation if already in this sequence
	if (bOverRide || self->s.frame < self->s.frameInfo.startFrame || self->s.frame > self->s.frameInfo.endFrame)
	{
		self->s.frame = self->s.frameInfo.startFrame;
	}

	return TRUE;
}

int AI_ForceSequence( userEntity_t *self, const char *animName, int nFrameLoopFlag )
{
	frameData_t *pSequence = FRAMES_GetSequence( self, animName );
	playerHook_t *hook = AI_GetPlayerHook(self);
	if(hook)
	{
		if(hook->cur_sequence)
		{
			if(strlen(hook->cur_sequence->animation_name))
			{				
				if( (!_stricmp(hook->cur_sequence->animation_name,animName)) && (self->s.frameInfo.frameFlags & FRAME_LOOP) && (nFrameLoopFlag & FRAME_LOOP))
				{
					return TRUE;
				}
			}
		}
	}
	if ( !pSequence )
	{
		gstate->Con_Dprintf("Missing %s frame data for %s.\n", animName, self->className);
		return FALSE;
	}

	return AI_ForceSequence( self, pSequence, nFrameLoopFlag );
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StartTransition
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StartTransition( userEntity_t *self, frameData_t *pNewSequence )
{
	if ( !pNewSequence )
	{
		return;
	}

	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	hook->cur_sequence = pNewSequence;

	ai_set_sequence_sounds( self, 
							pNewSequence->soundframe1, 
							pNewSequence->sound1, 
							pNewSequence->soundframe2, 
							pNewSequence->sound2, 
							pNewSequence->nAttackFrame1,
							pNewSequence->nAttackFrame2,
							pNewSequence->fAnimChance,
							pNewSequence->fSound2Chance );

	self->s.frame					= pNewSequence->first;
	self->s.frameInfo.frameState	= FRSTATE_TRANSITION;
	self->s.frameInfo.startFrame	= pNewSequence->first;
	self->s.frameInfo.endFrame		= pNewSequence->last;
	self->s.frameInfo.frameFlags	= FRAME_FORCEINDEX;
	self->s.frameInfo.frameInc		= 1;
	self->s.frameInfo.loopCount		= 0;
	self->s.frameInfo.modelAnimSpeed = pNewSequence->fModelAnimSpeed;
}

// ----------------------------------------------------------------------------
//
// Name:		AI_StopCurrentSequence
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void AI_StopCurrentSequence( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	hook->cur_sequence				= NULL;
	self->s.frameInfo.frameFlags	= FRAME_FORCEINDEX;
	self->s.frameInfo.frameInc		= 1;
	self->s.frameInfo.loopCount		= 0;
	self->s.frameInfo.frameState	= 0;
}

// ----------------------------------------------------------------------------
//
// Name:        AI_ComputeAnimationTime
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int AI_ComputeAnimationTime( frameData_t *pSequence )
{
    _ASSERTE( pSequence );
    int nNumFrames = pSequence->last - pSequence->first + 1;

    float fTime = nNumFrames * 0.1f;

    _ASSERTE( fTime > 0.0f );
    return fTime;
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
