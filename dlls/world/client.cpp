// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================

#if _MSC_VER
#include <crtdbg.h>
#endif

#include "world.h"

#include "ai_common.h"
//#include "ai_move.h"
#include "ai_frames.h"

#include "nodelist.h"
#include "triggers.h"
#include "client.h"
#include "bodylist.h"
#include "chasecam.h"
#include "actorlist.h"
//#include "GrphPath.h"// SCG[1/23/00]: not used
#include "client_init.h"
#include "MonsterSound.h"
#include "dk_buffer.h"
#include "collect.h"
#include "ai_utils.h"
#include "SequenceMap.h"
#include "ai.h"
#include "ai_func.h"
#include "Sidekick.h"
#include "ai_info.h"
#include "coop.h"
#include "flag.h"

//#include "spawn.h"// SCG[1/23/00]: not used
#include "ai_save.h"

long exp_level[] = 
#include "levels.h"


int calcStatLevel( userEntity_t *self );
typedef	(*func_t)(edict_t *);
/* ***************************** define types ****************************** */

//#define _DEBUG_CLIENT

/////////////////////////////////////////////////////////////////////////////////////////////////////
//	think states
//	
//	these values describe the set of routines that have control of an entity's thinking process
//	they are not flags -- hook->ai_state can only equal one of these values at a time.
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define TH_STAND		0x00000001
#define TH_CROUCH		0x00000002
#define TH_RUNFORWARD	0x00000004
#define TH_RUNBACKWARD	0x00000008
#define TH_JUMP			0x00000010
#define TH_ATTACK		0x00000020

#define BOOST_BLINKTIME   (5.0)

static qboolean do_motd = FALSE;

void SetTeam(userEntity_t *ent)
{
    playerHook_t *hook = (playerHook_t *)ent->userHook;
    int pc;
    char s[64];

    pc = 0;

    strncpy(s, Info_ValueForKey(ent->client->pers.userinfo, "skinname"), 64);        
    if(strstr(s,"iko"))
        pc = PLAYERCLASS_MIKIKO;
    else if (strstr(s, "fly"))
        pc = PLAYERCLASS_SUPERFLY;
    else if (strstr(s, "iro"))
        pc = PLAYERCLASS_HIRO;
    if(hook)
        hook->iPlayerClass = pc;

	if (!(ctf->value || deathtag->value || (deathmatch->value && dm_teamplay->value)))
		ent->team = (atoi(Info_ValueForKey (ent->client->pers.userinfo, "skincolor")));
}


typedef struct _clientTransitionInfo
{
	char *szFromSequence;
	char *szToSequence;
	char *szTransitionSequence;	// sequence to use when going from szFromSequence to szToSequence
} CLIENT_TRANSITIONINFO;

static CLIENT_TRANSITIONINFO aTransitionSequences[] = 
{
	{ "run",	"aamb",		"rstopl" },
	{ "runa",	"aamba",	"rstopla" },
	{ "runb",	"aambb",	"rstoplb" },
	{ "aamb",	"camb",		"cin" },
	{ "aamba",	"camba",	"cina" },
	{ "aambb",	"cambb",	"cinb" },
	{ "camb",	"aamb",		"cout" },
	{ "camba",	"aamba",	"couta" },
	{ "cambb",	"aambb",	"coutb" },
};
static int nNumTransitionSequences = sizeof(aTransitionSequences) / sizeof(aTransitionSequences[0]);


/* ***************************** Local Variables *************************** */
static char szAnimation[16];

static CVector lastPlayerMovedPosition;

/* ***************************** Local Functions *************************** */

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
void MONITOR_Stop(userEntity_t *self); //NSS[11/2/99]:

/* ******************************* exports ********************************* */


//void ClientEndServerFrame (edict_t *ent);// SCG[1/23/00]: not used

////////////////////////////////////////////////////////////////
//	exports
////////////////////////////////////////////////////////////////

//	client interface to server
DllExport	void	dll_PutClientInServer(userEntity_t *self);	//	Q2FIXME: still need to export???
DllExport	void	dll_ClientDisconnect(userEntity_t *self);

//	Q2 stuff
DllExport	void	dll_ClientBegin (userEntity_t *self, int loadgame);
DllExport	int		dll_ClientConnect (userEntity_t *self, void *userinfo, int loadgame);
DllExport	void	dll_ClientThink (userEntity_t *self, usercmd_t *ucmd, pmove_t *pm);
DllExport	void	dll_ClientUserinfoChanged(edict_t *ent, char *userinfo);
//DllExport	void	dll_ClientCommand (edict_t *ent);// SCG[1/23/00]: not used
DllExport	void	dll_ClientBeginServerFrame (userEntity_t *self);
DllExport	void	dll_SetStats (userEntity_t *ent);

DllExport	void	info_player_deathmatch (userEntity_t *self);
DllExport	void	info_player_start (userEntity_t *self);
DllExport	void	info_player_coop (userEntity_t *self);
DllExport	void	info_player_team1 (userEntity_t *self);
DllExport	void	info_player_team2 (userEntity_t *self);
DllExport	void	info_player_intermission (userEntity_t *self);

void PlayIntroCinematic();

void Client_Save( FILE *f, struct edict_s *ent );
void Client_Load( FILE *f, struct edict_s *ent );
////////////////////////////////////////////////////////////////
//	globals
////////////////////////////////////////////////////////////////

int				cur_clients;
DEATHTYPE		client_deathtype;

userEntity_t	*pm_passent;


#define	FATIGUE_MAX		5000

////////////////////////////////////////////////////////////////
// structure for passing death info to weapons
////////////////////////////////////////////////////////////////

typedef	struct	killerInfo_s
{
	userEntity_t	*attacker;
	userEntity_t	*victim;
} killerInfo_t;

killerInfo_t	killerInfo;

/////////////////////////////////////////////////////////////////
//	prototypes
/////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
//  PlayClientSound
//
//  Description:
//      Stub routine for difference player class sounds until the
//      faster, cheaper, cleaner method is ready to go
#pragma message("// SCG[10/27/99]: patch" )
void PlayClientSound(userEntity_t *self, char *soundfile)
{
    char path[256]={"\0"};
    char modelname[64]={"\0"};

	if( self->client == NULL )
	{
		return;
	}

	if (deathmatch->value || coop->value)
		strncpy(modelname, self->client->pers.body_info.modelname, 64);
	else
		strncpy(modelname, Info_ValueForKey(self->client->pers.userinfo, "modelname"), 64);

    if(strstr(modelname, "mikiko"))
        Com_sprintf(path, sizeof(path),"mikiko/%s", soundfile);
    else if(strstr(modelname, "hiro"))
        Com_sprintf(path, sizeof(path),"hiro/%s", soundfile);
    else if(strstr(modelname, "superfly"))
        Com_sprintf(path, sizeof(path),"superfly/%s", soundfile);
    if(self->input_entity)
        gstate->StartEntitySound(self->input_entity, CHAN_BODY, gstate->SoundIndex(path), 0.85f, ATTN_NORM_MIN,ATTN_NORM_MAX);
    else
        gstate->StartEntitySound(self, CHAN_BODY, gstate->SoundIndex(path), 0.85f, ATTN_NORM_MIN,ATTN_NORM_MAX);
}
///////////////////////////////////////////////////////////////////////


//*****************************************************************************
//*****************************************************************************
//	Client Animation Routines
//*****************************************************************************
//*****************************************************************************

// ----------------------------------------------------------------------------
//
// Name:		Client_InitClientModel
// Description:
// Input:
// Output:
// Note:
//	loads the client model
//
// ----------------------------------------------------------------------------
void Client_InitClientModel ( userEntity_t *self )
{

//  testing purposes only
//	char *modelName = "models/global/m_superfly.dkm";
//	char *modelName = "models/global/m_mikiko.dkm";
//	char *modelName = "models/global/m_hiro.dkm";
    char m_name[MAX_QPATH];
//    char temp_name[MAX_QPATH];
//    int skincolor, character;

	/////////////////////////////////////////////////////////
	//	make sure we're drawing the models
	/////////////////////////////////////////////////////////
	self->s.renderfx &= ~RF_NODRAW;

//    modelname = gstate->cvar("modelname", DEFAULT_CHARACTER_MODEL, CVAR_USERINFO | CVAR_ARCHIVE);
//    self->s.modelindex = gstate->ModelIndex (modelname->string); // set on cvar "modelname"
//	self->modelName = modelname->string;                         // set on cvar "modelname"
    
    // pull model name from userinfo 
	memset(m_name, 0, sizeof(m_name));  // reset
    strncpy(m_name, Info_ValueForKey(self->client->pers.userinfo, "modelname"), sizeof(m_name)-1);

//#ifdef DM_DEMO
    // verify that the model is one of the main character models, for NOW
    if (!strstr(m_name,"models/global/m_hiro.dkm"))
      if (!strstr(m_name,"models/global/m_mikiko.dkm"))
        if (!strstr(m_name,"models/global/m_superfly.dkm"))
          strcpy(m_name,"models/global/m_hiro.dkm");                    // default model name 
//#endif



//#define ANDREW_DEBUG
#ifdef ANDREW_DEBUG
	strcpy(m_name, "models/global/m_hiro2.dkm");
#endif

    self->s.modelindex = gstate->ModelIndex(m_name);
	self->modelName = m_name;

    self->s.skinnum = CHARACTER_MODEL_SKINNUM; // special case to notify AddEntityPackets of character skin to be applied
    
	if ( self->pMapAnimationToSequence )
    {
        delete self->pMapAnimationToSequence;
        self->pMapAnimationToSequence = NULL;
    }

	SEQUENCEMAP_AllocSequenceMap( self );

	if ( !ai_get_sequences(self) )
	{
		// initialize the animation sound data
		FRAMEDATA_ReadFile( "sounds/player/mikiko/m_mikiko.csv", self );
	}
/*
	// initialize the cluster definition records
	clusterDef_t	*pDef = &self->s.cDef[0];

	pDef->clusterIdx = 1;
	pDef->angles.Set(15.0f,15.0f,17.0f);

	pDef++;

	pDef->clusterIdx = 2;
	pDef->angles.Set(25.0f,25.0f,27.0f);

	pDef++;

	pDef->clusterIdx = 3;
	pDef->angles.Set(35.0f,35.0f,37.0f);

	self->s.numClusters = 3;
*/
	// amw: right now zero means don't look at anyone
	self->s.numClusters = 0 ;
}

// ----------------------------------------------------------------------------
//
// Name:		Client_Breathe
// Description:
// Input:
// Output:
// Note:
//	sets up idlescale according to the player's fatigue
//
//	FIXME: stop idle angle from being reset each time v_ipitch_cycle is sent
//
// ----------------------------------------------------------------------------
void Client_Breathe (userEntity_t *self, userEntity_t *other, float kick, int damage)
{
//	playerHook_t	*ihook = AI_GetPlayerHook( self->input_entity );// SCG[1/23/00]: not used

	// max pitch rate of 5
/*
	gstate->ClientCmd (self, "v_iyaw_level 0\n");

//	if (!(ihook->fatigue % 250))
//	{
		pitch_rate = (float) ihook->fatigue / (FATIGUE_MAX / 12.0) + 1;
		sprintf (msg, "v_ipitch_cycle %.2f\n", pitch_rate);
		gstate->ClientCmd (self, msg);
		gstate->Con_Dprintf ("%s\n", msg);
//	}

	// max pitch change of 2
	pitch_change = (float) ihook->fatigue / (FATIGUE_MAX / 4.0) + 1;
	sprintf (msg, "v_ipitch_level %.2f\n", pitch_change);
	gstate->ClientCmd (self, msg);
	gstate->Con_Dprintf ("%s\n", msg);
*/
}

///////////////////////////////////////////////////////////////////////////////
//	Client animation routines
///////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------------
//
// Name:		client_stand_frames
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void client_stand_frames(userEntity_t *self)
{
	playerHook_t	*hook = (playerHook_t *)self->userHook;
//	playerHook_t	*ihook = AI_GetPlayerHook( self->input_entity );// SCG[1/23/00]: not used

	if (hook->th_state == TH_STAND)
	{
		return;
	}

	frameData_t *pSequence = FRAMES_GetSequence( self->input_entity, "aambb" );
	AI_StartSequence(self, pSequence, FRAME_LOOP);

	hook->th_state = TH_STAND;
}

// ----------------------------------------------------------------------------
//
// Name:		client_death_frames
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void client_death_frames(userEntity_t *self)
{
//	playerHook_t	*ihook = AI_GetPlayerHook( self->input_entity );// SCG[1/23/00]: not used
//	playerHook_t	*hook = (playerHook_t *)self->userHook;// SCG[1/23/00]: not used

	if (self->deadflag != DEAD_NO)
		return;

	frameData_t *pSequence = FRAMES_GetSequence( self->input_entity, "dieb" );
	AI_StartSequence(self->input_entity, pSequence, FRAME_LOOP);
}

// ****************************************************************************
//						Transition animations
// ****************************************************************************

__inline static int CLIENT_IsEndAnimation( userEntity_t *self )
{
	if ( self->s.frame >= self->s.frameInfo.endFrame || self->s.frame == self->s.frameInfo.startFrame )
	{
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		CLIENT_StartTransition
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void CLIENT_StartTransition( userEntity_t *self, int nTransitionIndex )
{
	userEntity_t *ie = self->input_entity;

	if ( CLIENT_IsEndAnimation( ie ) )
	{
		frameData_t *pSequence = FRAMES_GetSequence( ie, aTransitionSequences[nTransitionIndex].szTransitionSequence );

		AI_StartSequence( ie, pSequence );
	}

	playerHook_t *ihook = AI_GetPlayerHook( ie );
	ihook->bInTransition = TRUE;
	ihook->nTransitionIndex = nTransitionIndex;
	ihook->fTransitionTime = gstate->time + 1.0f;
}

// ----------------------------------------------------------------------------
//
// Name:		CLIENT_EndTransition
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void CLIENT_EndTransition( userEntity_t *self )
{
	userEntity_t *ie = self->input_entity;
	playerHook_t *ihook = AI_GetPlayerHook( ie );
	ihook->bInTransition = FALSE;
}

__inline static void CLIENT_EndTransition( playerHook_t *ihook )
{
	ihook->bInTransition = FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		CLIENT_UpdateFrame
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void CLIENT_UpdateFrame( userEntity_t *self )
{
	// increment frames according to length of last frame
	self->s.frameInfo.next_frameTime = self->s.frameInfo.next_frameTime + gstate->frametime;
	
	if (self->s.frameInfo.next_frameTime > self->s.frameInfo.frameTime)
	{
		float temp = floor (self->s.frameInfo.next_frameTime / self->s.frameInfo.frameTime);
		int add = (int)(self->s.frameInfo.frameInc * temp);
		int new_frame = self->s.frame + add;

		if ( self->s.frameInfo.frameInc > 0 )
		{
			///////////////////////////////////////////////////////////////////////
			self->s.frame += add;

			// determine if sequence needs to loop back to start
			if (self->s.frame > self->s.frameInfo.endFrame)
			{
				if (self->s.frameInfo.frameFlags & FRAME_ONCE)
				{
					// we've gone through once, so set frames to static (no updates)
					self->s.frame = self->s.frameInfo.endFrame;
				}
				else
				{
					//	looping through entire sequence
					self->s.frame = self->s.frameInfo.startFrame;
				}
			}
		}
		else
		{
			// reverse animation
			self->s.frame += add;

			// determine if sequence needs to loop back to start
			if (self->s.frame < self->s.frameInfo.endFrame)
			{
				if (self->s.frameInfo.frameFlags & FRAME_ONCE)
				{
					// we've gone through once, so set frames to static (no updates)
					self->s.frame = self->s.frameInfo.endFrame;
				}
				else
				{
					//	looping through entire sequence
					self->s.frame = self->s.frameInfo.startFrame;
				}
			}
		}
					
		self->s.frameInfo.next_frameTime = self->s.frameInfo.next_frameTime - (temp * self->s.frameInfo.frameTime);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		CLIENT_Transition
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void CLIENT_Transition( userEntity_t *self )
{
	_ASSERTE( self );
	userEntity_t *ie = self->input_entity;
	_ASSERTE( ie );
	playerHook_t *ihook = AI_GetPlayerHook( ie );
	_ASSERTE( ihook );

	if ( CLIENT_IsEndAnimation( ie ) )
	{
		if ( _stricmp( ihook->cur_sequence->animation_name, aTransitionSequences[ihook->nTransitionIndex].szFromSequence ) == 0 )
		{
			// now, start the transition sequence

			frameData_t *pSequence = FRAMES_GetSequence( ie, aTransitionSequences[ihook->nTransitionIndex].szTransitionSequence );
			AI_StartSequence( ie, pSequence );
		}
		else
		if ( _stricmp( ihook->cur_sequence->animation_name, aTransitionSequences[ihook->nTransitionIndex].szTransitionSequence ) == 0 )
		{
			// now, start the end sequence

			frameData_t *pSequence = FRAMES_GetSequence( ie, aTransitionSequences[ihook->nTransitionIndex].szToSequence );
			AI_StartSequence( ie, pSequence, FRAME_LOOP );

			CLIENT_EndTransition( ihook );
		}
	}

	if ( gstate->time > ihook->fTransitionTime )
	{
		CLIENT_EndTransition( ihook );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		CLIENT_HandleTransition
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static int CLIENT_HandleTransition( userEntity_t *self, const char *szFromSequence, const char *szToSequence )
{
	_ASSERTE( self );

	for ( int i = 0; i < nNumTransitionSequences; i++ )
	{
		if ( _stricmp( aTransitionSequences[i].szFromSequence, szFromSequence ) == 0 && 
			 _stricmp( aTransitionSequences[i].szToSequence, szToSequence ) == 0
		   )
		{
			CLIENT_StartTransition( self, i );
			return TRUE;
		}    	
	}

	return FALSE;
}

// ****************************************************************************

// ----------------------------------------------------------------------------
//
// Name:		CLIENT_SelectAnimationPerWeaponType
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
__inline static void CLIENT_SelectAnimationPerWeaponType( userEntity_t *self, char *pszAnimation )
{
	_ASSERTE( self );
	_ASSERTE( pszAnimation );

///	strcpy( pszAnimation, "atak" );

	weaponInfo_t *pWeaponInfo = (weaponInfo_t *)self->winfo;
	if ( !pWeaponInfo )
	{
		return;
	}
	if( self->waterlevel > 2)
	{
		sprintf(pszAnimation,"swim");
		return;
	}
	int nStrLength = strlen( pszAnimation );
	switch( pWeaponInfo->nWeaponType )
	{
		case WEAPONTYPE_GLOVE:
		{
			break;
		}
		case WEAPONTYPE_PISTOL:
		{
			pszAnimation[nStrLength] = 'a';
			pszAnimation[nStrLength+1] = '\0';
			break;
		}
		case WEAPONTYPE_RIFLE:
		{
			pszAnimation[nStrLength] = 'b';
			pszAnimation[nStrLength+1] = '\0';
			break;
		}
		case WEAPONTYPE_SHOULDER:
		{
			pszAnimation[nStrLength] = 'b';//'c';
			pszAnimation[nStrLength+1] = '\0';
			break;
		}
		default:
			break;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		CLIENT_FireCurrentWeapon
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static int CLIENT_FireCurrentWeapon( userEntity_t *self )
{
	_ASSERTE( self );

	if ( !self->input_entity->curWeapon )
	{
		return FALSE;
	}

	playerHook_t *ihook = AI_GetPlayerHook( self->input_entity );
	if ( ihook->attack_finished > gstate->time )
	{
		return FALSE;
	}

	//	if self == input_entity (not possessing) then fire immediately
	//	otherwise, monster weapon is fired in client_animation_think
	//	on correct frames
	// cek[12-2-99] no shooting when cinematics play
//	if (self->input_entity == self)
	if ((self->input_entity == self) && !gstate->bCinematicPlaying)
	{
		ihook->weapon_fired = self->input_entity->curWeapon;
		self->input_entity->curWeapon->use (self->input_entity->curWeapon, self->input_entity);

        weapon_t *weapon = (weapon_t *) self->input_entity->curWeapon;

		if(weapon)//NSS[11/30/99]:
		{
			if ( weapon->winfo->nWeaponType == WEAPONTYPE_RIFLE || weapon->winfo->nWeaponType == WEAPONTYPE_SHOULDER )
			{
				AI_SetJustFired( ihook );
			}


			// send down ammo count to client for display on status bar
// SCG[1/23/00]: not used
			//			if (weapon->ammo)
//			{
//				ammo_t *ammo = (ammo_t *) weapon->ammo;
				//	Q2FIXME:	change to Q2 network functions
				/*
				gstate->WriteByte (self, MSG_ONE, SVC_AMMOCOUNT);
				gstate->WriteByte (self, MSG_ONE, (int)ammo->count);
				*/
//			}
		}
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		CLIENT_StartJump
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void CLIENT_StartJump( userEntity_t *self, int nState )
{
	playerHook_t *hook = (playerHook_t *)self->userHook;
	playerHook_t *ihook = AI_GetPlayerHook( self->input_entity );
	
	if (self->deadflag != DEAD_NO)
	{
		return;
	}

	//////////////////////////////////////////////////////////////////////
	//	make sure that the input entity can jump at all
	//////////////////////////////////////////////////////////////////////
	if (self->flags & FL_FREEZE || ihook->max_jump_dist <= 0.0)
	{
		return;
	}

	userEntity_t *ie = self->input_entity;
	// make sure we make the input entity jump!
	if ( ie->groundEntity )
	{
		ihook->jump_time = gstate->time;

		// JAR first jump z vel is > 1 (like 250) don't even bother, if it's a ground ent what's the difference?
		// if ( ie->velocity.z < 1.0f )
		// {
			float boostfrac = (!deathmatch->value) ? 0.15 : 0.3;
			ie->velocity.z = ihook->upward_vel + (hook->acro_boost * ihook->upward_vel * boostfrac);
			
			// check if running into a wall
			if ( ie->velocity.x != 0.0f || ie->velocity.y != 0.0f )
			{
				float fSpeed = ihook->run_speed * 0.1f;
				CVector forwardVector = ie->velocity;
				forwardVector.Normalize();
				
				CVector endPos = ie->s.origin + (forwardVector * fSpeed);
// SCG[12/10/99]: 				tr = gstate->TraceLine_q2( self->s.origin, endPos, ie, CHOOSE_CLIP(ie) );
				tr = gstate->TraceLine_q2( self->s.origin, endPos, ie, ie->clipmask );
				if ( tr.fraction < 1.0f )
				{
					// blocked by something, have client just jump up
					ie->velocity.x = 0.0f;
					ie->velocity.y = 0.0f;
				}
			}
		// }

		szAnimation[0] = '\0';
		if ( nState & TH_RUNFORWARD || nState & TH_RUNBACKWARD )
		{
			if ( nState & TH_ATTACK )
			{
				strcpy( szAnimation, "bjump" );
				CLIENT_FireCurrentWeapon( self );
			}
			else
			if ( nState & TH_CROUCH )
			{
				strcpy( szAnimation, "bjump" );
			}
			else
			{
				strcpy( szAnimation, "bjump" );
			}
		}
		else
		if ( nState & TH_STAND )
		{
			if ( nState & TH_ATTACK )
			{
				strcpy( szAnimation, "ajump" );
				CLIENT_FireCurrentWeapon( self );
			}
			else
			if ( nState & TH_CROUCH )
			{
				strcpy( szAnimation, "ajump" );
			}
			else
			{
				strcpy( szAnimation, "ajump" );
			}
		}
		_ASSERTE( szAnimation[0] != '\0' );

        CLIENT_SelectAnimationPerWeaponType( self, szAnimation );
	
		if ( !ihook->cur_sequence || _stricmp( ihook->cur_sequence->animation_name, szAnimation ) != 0 )
		{
			frameData_t *pSequence = FRAMES_GetSequence( ie, szAnimation );
			AI_StartSequence( ie, pSequence, FRAME_LOOP);
		}

		if(self->waterlevel <= 2)
		{
			if(hook && hook->acro_boost > 0)
			{
				float volume = (hook->acro_boost < 5) ? hook->acro_boost * 0.1 : 1.0;
				gstate->StartEntitySound(self,CHAN_WEAPON,gstate->SoundIndex("global/Acroboost.wav"),volume,ATTN_NORM_MIN, ATTN_NORM_MAX);
			}

			float r = rnd ();
			if (r < 0.33)
				PlayClientSound(self, "jump1.wav");
			else if (r < 0.66)
				PlayClientSound(self, "jump2.wav");
			else
				PlayClientSound(self, "jump3.wav");
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		CLIENT_StartAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void CLIENT_StartAttack( userEntity_t *self, int nState )
{
	if (self->deadflag != DEAD_NO)
	{
		return;
	}

	if ( !CLIENT_FireCurrentWeapon( self ) )
	{
		return;
	}

	szAnimation[0] = '\0';
	if ( nState & TH_RUNFORWARD || nState & TH_RUNBACKWARD )
	{
		if ( nState & TH_JUMP )
		{
			strcpy( szAnimation, "bjump" );
		}
		else
		if ( nState & TH_CROUCH )
		{
			strcpy( szAnimation, "cwalk" );
		}
		else
		{
			strcpy( szAnimation, "run" );
		}
	}
	else
	if ( nState & TH_STAND )
	{
		if ( nState & TH_JUMP )
		{
			strcpy( szAnimation, "ajump" );
		}
		else
		if ( nState & TH_CROUCH )
		{
			strcpy( szAnimation, "camb" );
		}
		else
		{
			strcpy( szAnimation, "aamb" );
		}
	}
	_ASSERTE( szAnimation[0] != '\0' );

	CLIENT_SelectAnimationPerWeaponType( self, szAnimation );

	userEntity_t *ie = self->input_entity;
	playerHook_t *ihook = AI_GetPlayerHook( ie );
	if ( !ihook->cur_sequence || _stricmp( ihook->cur_sequence->animation_name, szAnimation ) != 0 )
	{
		frameData_t *pSequence = FRAMES_GetSequence( ie, szAnimation );
		AI_StartSequence( ie, pSequence, FRAME_LOOP );
	}

}

// ----------------------------------------------------------------------------
//
// Name:		CLIENT_StartCrouch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void CLIENT_StartCrouch( userEntity_t *self, int nState )
{
	if (self->deadflag != DEAD_NO)
	{
		return;
	}

	// 0 => not moving, 1 => moving forward, 2 => moving backward
	int nMovingDirection = 0;

	szAnimation[0] = '\0';
	if ( nState & TH_RUNFORWARD || nState & TH_RUNBACKWARD )
	{
		if ( nState & TH_JUMP )
		{
			strcpy( szAnimation, "cwalk" );
		}
		else
		if ( nState & TH_ATTACK )
		{
			_ASSERTE( FALSE );	
		}
		else
		{
			strcpy( szAnimation, "cwalk" );
		}

		if ( nState & TH_RUNFORWARD )
		{
			nMovingDirection = 1;
		}
		else
		{
			nMovingDirection = 2;
		}
	}
	else
	if ( nState & TH_STAND )
	{
		if ( nState & TH_JUMP )
		{
			strcpy( szAnimation, "camb" );
		}
		else
		if ( nState & TH_ATTACK )
		{
			_ASSERTE( FALSE );
		}
		else
		{
			strcpy( szAnimation, "camb" );
		}
	}
	_ASSERTE( szAnimation[0] != '\0' );

	CLIENT_SelectAnimationPerWeaponType( self, szAnimation );

	userEntity_t *ie = self->input_entity;
	playerHook_t *ihook = AI_GetPlayerHook( ie );
	if ( !ihook->cur_sequence || _stricmp( ihook->cur_sequence->animation_name, szAnimation ) != 0 )
	{
		int bTransition = FALSE;
		if ( ihook->cur_sequence )
		{
			bTransition = CLIENT_HandleTransition( self, ihook->cur_sequence->animation_name, szAnimation );
		}
		
		if ( !bTransition )
		{
			frameData_t *pSequence = FRAMES_GetSequence( ie, szAnimation );

			if ( nMovingDirection <= 1 )
			{
				AI_StartSequence( ie, pSequence, FRAME_LOOP );
			}
			else
			{
				// going backwards
				AI_StartSequenceInReverse( ie, pSequence, FRAME_LOOP );
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		CLIENT_StartRunForward
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void CLIENT_StartRunForward( userEntity_t *self, int nState )
{
	if (self->deadflag != DEAD_NO)
	{
		return;
	}

	szAnimation[0] = '\0';
	if ( nState & TH_JUMP )
	{
		_ASSERTE( FALSE );
	}
	else
	if ( nState & TH_ATTACK )
	{
		_ASSERTE( FALSE );
	}
	else
	if ( nState & TH_CROUCH && nState & TH_JUMP )
	{
		_ASSERTE( FALSE );
	}
	else
	if ( nState & TH_CROUCH )
	{
		_ASSERTE( FALSE );
	}
	else
	{
		strcpy( szAnimation, "run" );
	}
	_ASSERTE( szAnimation[0] != '\0' );

	CLIENT_SelectAnimationPerWeaponType( self, szAnimation );

	userEntity_t *ie = self->input_entity;
	playerHook_t *ihook = AI_GetPlayerHook( ie );
	if ( !ihook->cur_sequence || 
		 nState != ihook->th_state ||
		 _stricmp( ihook->cur_sequence->animation_name, szAnimation ) != 0 
	   )
	{
		frameData_t *pSequence = FRAMES_GetSequence( ie, szAnimation );
		// going forwards
		AI_StartSequence( ie, pSequence, FRAME_LOOP );

		//gstate->Con_Dprintf( "Forwards.\n" );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		CLIENT_StartRunBackward
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void CLIENT_StartRunBackward( userEntity_t *self, int nState )
{
	if (self->deadflag != DEAD_NO)
	{
		return;
	}

	szAnimation[0] = '\0';
	if ( nState & TH_JUMP )
	{
		_ASSERTE( FALSE );
	}
	else
	if ( nState & TH_ATTACK )
	{
		_ASSERTE( FALSE );
	}
	else
	if ( nState & TH_CROUCH && nState & TH_JUMP )
	{
		_ASSERTE( FALSE );
	}
	else
	if ( nState & TH_CROUCH )
	{
		_ASSERTE( FALSE );
	}
	else
	{
		strcpy( szAnimation, "run" );
	}
	_ASSERTE( szAnimation[0] != '\0' );

	CLIENT_SelectAnimationPerWeaponType( self, szAnimation );

	userEntity_t *ie = self->input_entity;
	playerHook_t *ihook = AI_GetPlayerHook( ie );
	if ( !ihook->cur_sequence || 
		 nState != ihook->th_state ||
		 _stricmp( ihook->cur_sequence->animation_name, szAnimation ) != 0 
	   )
	{
		frameData_t *pSequence = FRAMES_GetSequence( ie, szAnimation );
		// going backwards
		AI_StartSequenceInReverse( ie, pSequence, FRAME_LOOP );

		//gstate->Con_Dprintf( "Backwards.\n" );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		CLIENT_StartStand
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
static void CLIENT_StartStand( userEntity_t *self, int nState )
{
	if (self->deadflag != DEAD_NO)
	{
		return;
	}

	szAnimation[0] = '\0';
	if ( nState & TH_JUMP )
	{
		_ASSERTE( FALSE );
	}
	else
	if ( nState & TH_ATTACK )
	{
		_ASSERTE( FALSE );
	}
	else
	if ( nState & TH_CROUCH && nState & TH_JUMP )
	{
		_ASSERTE( FALSE );
	}
	else
	if ( nState & TH_CROUCH )
	{
		_ASSERTE( FALSE );
	}
	else
	{
		strcpy( szAnimation, "aamb" );
	}
	_ASSERTE( szAnimation[0] != '\0' );

	CLIENT_SelectAnimationPerWeaponType( self, szAnimation );

	userEntity_t *ie = self->input_entity;
	playerHook_t *ihook = AI_GetPlayerHook( ie );
	if ( !ihook->cur_sequence || _stricmp( ihook->cur_sequence->animation_name, szAnimation ) != 0 )
	{
		int bTransition = FALSE;
		if ( ihook->cur_sequence )
		{
			bTransition = CLIENT_HandleTransition( self, ihook->cur_sequence->animation_name, szAnimation );
		}
		
		if ( !bTransition )
		{
			frameData_t *pSequence = FRAMES_GetSequence( ie, szAnimation );
			AI_StartSequence( ie, pSequence, FRAME_LOOP );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		CLIENT_DetermineStates
// Description:
// Input:
// Output:
// Note:
//				if the client was in a transition sequence, then any actions
//				by the player should terminate the transition immediately!!
//
// ----------------------------------------------------------------------------
int CLIENT_DetermineStates( userEntity_t *self )
{
	userEntity_t *ie = self->input_entity;
	playerHook_t *ihook = AI_GetPlayerHook( ie );

	if( ihook == NULL )
	{
		return 0;
	}

	gclient_t *client = self->client;

	int nState = 0;

	//if ( client->ps.pmove.pm_flags & PMF_JUMP_HELD && ie->groundEntity )
	if ( client->ps.pmove.pm_flags & PMF_JUMP_HELD )
	{
  		if ( ie->groundEntity && ihook->can_jump_again)
		{
			nState |= TH_JUMP;
			CLIENT_EndTransition( ihook );
			ihook->can_jump_again = false;
		}
	}
	else
	{
		ihook->jump_time = 0;
		ihook->can_jump_again = true;
	}

	if ( client->latched_buttons & BUTTON_ATTACK )
	{
		nState |= TH_ATTACK;
		CLIENT_EndTransition( ihook );
	}
	if ( client->ps.pmove.pm_flags & PMF_DUCKED )
	{
		nState |= TH_CROUCH;

		if ( ihook->bInTransition && !(ihook->th_state & TH_CROUCH) )
		{
			CLIENT_EndTransition( ihook );
		}
	}
	else
	{
		
	}

	if ( fabs(ie->velocity.x) <= 1.0f && fabs(ie->velocity.y) <= 1.0f )
	{
		nState |= TH_STAND;
	}
	else
	{
		playerHook_t *hook = AI_GetPlayerHook( ie );
        if ( nState & TH_CROUCH )
        {
            hook->fSoundOutDistance = 0.0f;
        }
        else
        {
            float fVelocity = ie->velocity.Length();
            if ( fVelocity < hook->run_speed * 0.5f )
            {
                // must be walking
                hook->fSoundOutDistance = 200.0f;
            }
            else
            {
                hook->fSoundOutDistance = 600.0f;
            }
        }

        // see if the entity is going forwards or backwards
		float fVelocityYaw = VectorToYaw( ie->velocity );
		float fAngleDiff = fabs( fVelocityYaw - ie->s.angles.yaw );
		if ( fAngleDiff < 145.0f || fAngleDiff > 215.0f )
		{
			nState |= TH_RUNFORWARD;
		}
		else
		{
			nState |= TH_RUNBACKWARD;
		}

		CLIENT_EndTransition( ihook );
	}

	return nState;
}

// ----------------------------------------------------------------------------
//
// Name:		client_animation_think
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void client_animation_think (userEntity_t *self)
{
	userEntity_t *ie = self->input_entity;
	playerHook_t *ihook = AI_GetPlayerHook( ie );

	if( ihook == NULL )
	{
		return;
	}

	///////////////////////////////////////////////////////////////////////////
	// frame updates
	///////////////////////////////////////////////////////////////////////////

	if (ie->deadflag == DEAD_NO)
	{

#ifdef _DEBUG_CLIENT
		CSequenceList *pSequenceList = SEQUENCEMAP_Lookup( self->s.modelindex );
		if ( pSequenceList )
		{
			pSequenceList->Verify();
		}
#endif 

		int nState = CLIENT_DetermineStates( self );
		if ( ihook->bInTransition )
		{
			CLIENT_Transition( self );
			nState = 0;
		}
		else
		{
			if ( nState & TH_JUMP )
			{
				CLIENT_StartJump( self, nState );
			}
			else
			if ( nState & TH_ATTACK )
			{
				CLIENT_StartAttack( self, nState );
			}
			else
			if ( nState & TH_CROUCH )
			{
				if ( nState != ihook->th_state )
				{
					CLIENT_StartCrouch( self, nState );
				}
			}
			else
			if ( nState & TH_RUNFORWARD )
			{
				if ( nState != ihook->th_state )
				{
					CLIENT_StartRunForward( self, nState );
				}
			}
			else
			if ( nState & TH_RUNBACKWARD )
			{
				if ( nState != ihook->th_state )
				{
					CLIENT_StartRunBackward( self, nState );
				}
			}
			else
			if ( nState & TH_STAND )
			{
				//if ( nState != ihook->th_state )
				//{
					CLIENT_StartStand( self, nState );
				//}
			}
			else
			{
				
			}
		}
		ihook->th_state = nState;

		///////////////////////////////////////////////////////////////////////
		//	if input_entity != client, then we are in possesion,
		//	so handle attacks a bit different... use monster weapon
		//	in correct frame, instead of right when attack is pressed
		///////////////////////////////////////////////////////////////////////
		if (ie != self)
		{
			if (ihook->th_state == TH_ATTACK)
			{
				if (self->input_entity->s.frameInfo.frameState & FRSTATE_PLAYSOUND1)
				{
					self->input_entity->s.frameInfo.frameState -= FRSTATE_PLAYSOUND1;

					ie->curWeapon->use (ie->curWeapon, ie);
				}
				else 
				if (self->input_entity->s.frameInfo.frameState & FRSTATE_PLAYSOUND2)
				{
					self->input_entity->s.frameInfo.frameState -= FRSTATE_PLAYSOUND2;

					ie->curWeapon->use (ie->curWeapon, ie);
				}
			}
			//	plays any sounds for current animation
			else
			{
				if (self->input_entity->s.frameInfo.frameState & FRSTATE_PLAYSOUND1)
				{
					self->input_entity->s.frameInfo.frameState -= FRSTATE_PLAYSOUND1;

					if (ihook->sound1)
					{
						gstate->StartEntitySound (ie, CHAN_BODY, gstate->SoundIndex(ihook->sound1), 0.85f, ATTN_NORM_MIN,ATTN_NORM_MAX);
					}
				}
				else 
				if (self->input_entity->s.frameInfo.frameState & FRSTATE_PLAYSOUND2)
				{
					self->input_entity->s.frameInfo.frameState -= FRSTATE_PLAYSOUND2;

					if (ihook->sound2)
					{
						gstate->StartEntitySound (ie, CHAN_BODY, gstate->SoundIndex(ihook->sound2), 0.85f, ATTN_NORM_MIN,ATTN_NORM_MAX);
					}
				}
			}
		}
	}
	else 
	if (ie->deadflag == DEAD_DYING)
	{
		ie->view_ofs.z -= 2;
		if (ie->view_ofs.z < -8)
		{
			ie->view_ofs.z = -8;
		}

		//	FIXME: remove when death frames are present for character
		self->deadflag = DEAD_DEAD;

/*
		if (ihook->legs)
		{
			ihook = AI_GetPlayerHook( ihook->legs );
		}
*/
		if (self->input_entity->s.frameInfo.frameState & FRSTATE_STOPPED)
		{
			self->input_entity->s.frameInfo.frameState -= FRSTATE_STOPPED;

			// um... SCG: 2-26-99
//			ie->deadflag == DEAD_DEAD;
			ie->deadflag = DEAD_DEAD;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		Client_DropInventory
// Description:
// Input:
// Output:
// Note:
//	makes client drop their weapon and ammo in deathmatch
//
// ----------------------------------------------------------------------------
void Client_DropInventory(userEntity_t *self)
{
	weaponInfo_t	*winfo;
	userEntity_t	*weapon;    //, *ammo;
	weapon_t		*invWeapon;
	playerHook_t	*hook = (playerHook_t *)self->userHook;
    int             ammo_count = 0;
    ammo_t          *ammo = NULL;

	if (!self->inventory)
		return;

	if (!coop->value)
	{
		//	drop the client's current weapon with current amount of ammo
		if (self->winfo && self->client->ps.weapon)
		{
			// no model should be drawn on/with the client model
			self->s.modelindex2 = 0;

			winfo = (weaponInfo_t *) self->winfo;
			// is there any ammo?
			ammo = (ammo_t *) gstate->InventoryFindItem (self->inventory, winfo->ammoName);

			if ( ammo )
			{
				//	find out how much of this ammo we have
				ammo->command ((userInventory_t *) ammo, "count", &ammo_count);
			}

			// cek[7-19-00]: if there is ammo or it doesn't use ammo....drop it.
			if ( ammo_count || !winfo->ammo_per_use)
			{
				
				if(! (winfo->flags & WFL_NODROP) )
				{
					weapon = gstate->SpawnEntity ();
					weapon->className = winfo->weaponName;
					weapon->netname = winfo->weaponNetName;
					weapon->spawnflags = (DROPPED_ITEM|DROPPED_PLAYER_ITEM);

					//	set origin
					weapon->s.origin = self->s.origin;
					//	give some velocity
					weapon->velocity.x = rnd() * 400.0 - 200.0;
					weapon->velocity.y = rnd() * 400.0 - 200.0;
					weapon->velocity.z = rnd() * 250.0 + 250.0;

					winfo->spawn_func (winfo, weapon, -1.0, NULL);

					//	find out how much ammo player has for this weapon,
					//	and store it in the weapon pickup item
					invWeapon = (weapon_t *) self->curWeapon;
					weapon->hacks_int = invWeapon->ammo->count;

				}
			}
		}
	}
	//	clear everything having to do with weapons
	// cek[1-12-00]: does something totally different than it used to
//	Client_FreeWeapon (self);
	Client_InitWeapons (self);
}

// ----------------------------------------------------------------------------
//
// Name:		client_warp_appear
// Description:
// Input:
// Output:
// Note:
//	Makes client stretch out of nothing when appearing from
//	a teleporter
//
//	client is immobile until reaching full size
//
// ----------------------------------------------------------------------------
void client_warp_appear(userEntity_t *self)
{
	playerHook_t	*hook = (playerHook_t *)self->userHook;
	float			scale_up = 1.2, scale_down = 0.8;

	self->s.render_scale.x *= scale_up;
	self->s.render_scale.y *= scale_up;
	self->s.render_scale.z *= scale_down;

	if ( self->s.render_scale.x > 1.0 )
	{
		//	Restore the player's input now that they've reached full size
		//	Q2FIXME:	change to Q2 network functions
		//	gstate->WriteByte (self, MSG_ONE, SVC_RESTOREINPUT);

		self->s.render_scale.Set(1.0, 1.0, 1.0);
		self->solid = SOLID_BBOX;
		self->movetype = MOVETYPE_WALK;

		self->think = NULL;
		self->nextthink = -1;

		return;
	}

	self->think = client_warp_appear;
	self->nextthink = gstate->time + THINK_TIME;
}

//*****************************************************************************
//*****************************************************************************
//	Client Commands
//*****************************************************************************
//*****************************************************************************

///////////////////////////////////////////////////////////////////////////////
//	Client_NoTarget
///////////////////////////////////////////////////////////////////////////////
void Client_NoClip( userEntity_t *self )
{
	char	*msg;

	if( SinglePlayerCheat() == FALSE )
	{
		if( p_cheats->value == 0 )
		{
			if( deathmatch->value )
			{
				gstate->cprintf( self, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n" );
			}
			return;
		}
	}

	if( self->movetype == MOVETYPE_NOCLIP )
	{
		self->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	}
	else
	{
		self->movetype = MOVETYPE_NOCLIP;
		msg = "noclip ON\n";
	}

	gstate->cprintf( self, PRINT_HIGH, msg );
}

///////////////////////////////////////////////////////////////////////////////
//	Client_NoTarget
///////////////////////////////////////////////////////////////////////////////

void	Client_NoTarget (userEntity_t *self)
{
	if( SinglePlayerCheat() == FALSE )
	{
		if( gstate->GetCvar( "cheats" ) == 0 )
		{
			return;
		}
	}

	if (self->flags & FL_NOTARGET)
	{
		self->flags &= ~FL_NOTARGET;
		gstate->Con_Printf ("Notarget OFF.\n");
	}
	else
	{
		self->flags |= FL_NOTARGET;
		gstate->Con_Printf ("Notarget ON.\n");
	}
}

///////////////////////////////////////////////////////////////////////////////
//	Client_LlamaCheat
//
//	toggles invulnerability on and off
///////////////////////////////////////////////////////////////////////////////
void SIDEKICK_ToggleGodMode();
void	Client_LlamaCheat (userEntity_t *self)
{
	playerHook_t	*hook = (playerHook_t *)self->userHook;

	if( hook == NULL )
	{
		return;
	}

	if( SinglePlayerCheat() == FALSE )
	{
		if( gstate->GetCvar( "cheats" ) == 0 )
		{
			if (deathmatch->value)
			{
				gstate->cprintf (self, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
			}
			return;
		}
	}

	// make sure the client has been connected to the server

	if (!self->className)
		return;

	if (hook->dflags & DFL_LLAMA)
	{
		gstate->sprint (self, "God mode OFF.\n");
		hook->dflags &= ~DFL_LLAMA;
	}
	else
	{
		gstate->sprint (self, "God mode ON.\n");
		hook->dflags |= DFL_LLAMA;
	}

    SIDEKICK_ToggleGodMode();
}

///////////////////////////////////////////////////////////////////////////////
//	Client_RampageCheat
//
//	toggles Red Neck Rampage (R) mode
///////////////////////////////////////////////////////////////////////////////

void	Client_RampageCheat (userEntity_t *self)
{
	playerHook_t	*hook = (playerHook_t *)self->userHook;

	// make sure the client has been connected to the server
	if (!self->className)
		return;

	if( SinglePlayerCheat() == FALSE )
	{
		if( gstate->GetCvar( "cheats" ) == 0 )
		{
			if (deathmatch->value)
			{
				gstate->cprintf (self, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
			}
			return;
		}
	}


   	if (hook->dflags & DFL_RAMPAGE)
	{
		gstate->sprint (self, "Rampage mode OFF.\n");
		hook->dflags &= ~DFL_RAMPAGE;
	}
	else
	{
		gstate->sprint (self, "Rampage mode ON.\n");
		hook->dflags |= DFL_RAMPAGE;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	Client_HealthCheat
//
//	gives client 999 health
///////////////////////////////////////////////////////////////////////////////

void	Client_HealthCheat (userEntity_t *self)
{
	// make sure the client has been connected to the server
	if (!self->className)
		return;

	if( SinglePlayerCheat() == FALSE )
	{
		if( gstate->GetCvar( "cheats" ) == 0 )
		{
			if (deathmatch->value)
			{
				gstate->cprintf (self, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
			}
			return;
		}
	}


	float health = atof( gstate->GetArgv( 1 ) );
	if( !health )
	{
		health = 10;
	}

	self->health += health;
}

///////////////////////////////////////////////////////////////////////////////
//	Client_ShowStats
//
//	toggles stats display on and off
///////////////////////////////////////////////////////////////////////////////

void	Client_ShowStats (userEntity_t *self)
{
	playerHook_t	*hook = (playerHook_t *)self->userHook;

	if (hook->dflags & DFL_SHOWSTATS)
		hook->dflags = hook->dflags - DFL_SHOWSTATS;
	else
		hook->dflags = hook->dflags | DFL_SHOWSTATS;
}

///////////////////////////////////////////////////////////////////////////////
//	key_twist
//
///////////////////////////////////////////////////////////////////////////////

void	key_twist (userEntity_t *key)
{
	key->s.angles.z += 18.0;
	key->hacks_int++;
	if (key->hacks_int < 5)
	{
		key->nextthink = gstate->time + 0.1;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	Client_SpawnKey
//
///////////////////////////////////////////////////////////////////////////////

void	Client_SpawnKey (userEntity_t *button, userInventory_t *item)
{
	userEntity_t	*key;
	CVector			buttonOrigin;
	float			dist, largest;
	int				dir, i;

	if (stricmp (button->className, "func_button"))
		return;

	key = gstate->SpawnEntity ();
	key->movetype = MOVETYPE_NONE;
	key->solid = SOLID_NOT;
	
	key->s.modelindex = item->modelIndex;
	key->s.frame = 2;	//	third frame is specifcally oriented for placing
						//	keys in buttons so that we can avoid gimbal lock!
	key->s.frameInfo.frameFlags = FRAME_FORCEINDEX | FRAME_STATIC;
	
	//	find center of button
	buttonOrigin = button->s.mins + button->s.maxs;
	VectorMA(button->s.origin, buttonOrigin, 0.5, buttonOrigin);

	//	find the best axis of movement
	for (dir = largest = i = 0; i < 3; i++)
	{
		if (fabs(button->movedir[i]) > largest)
		{
			dir = i;
			largest = fabs(button->movedir[i]);
		}
	}

	//	place key at edge of button bounding box, along button->movedir
	dist = (button->s.maxs[dir] - button->s.mins[dir]) * 0.5;
	VectorMA(buttonOrigin, button->movedir, -dist, key->s.origin);

	//	rotate key to angle of movedir
	key->s.angles.x = 270;

	//	We ALL love gimbal lock!
	VectorToAngles(button->movedir, key->s.angles);

	gstate->LinkEntity (key);

	key->hacks_int = 0;
	key->think = key_twist;
	key->nextthink = gstate->time + 0.1;
}

///////////////////////////////////////////////////////////////////////////////
//	Client_Use
//
//	checks for a useable object in front of self and activates it's use function
//	if it has one and is close enough
//
//  FIXME: change to TraceBox when Q2 is merged
///////////////////////////////////////////////////////////////////////////////
#define STRUGGLE_DELAY		(1.0)
void SpawnDelay( userEntity_t *self, userEntity_t *other, userEntity_t *activator );
void Client_PlayUseWorldSound( userEntity_t *pClient )
{
	// SCG[10/29/99]: make sure the three sounds in worldspawn appear consecutively, or this will not work!
//	int sound_index = rand() % 3;
	playerHook_t *hook = AI_GetPlayerHook(pClient);
	if (!hook)
		return;

	if (hook->fNextStruggleTime <= gstate->time)
	{
		int sound_index = 0;
		switch(hook->iPlayerClass)
		{
		case PLAYERCLASS_SUPERFLY:
			sound_index = gstate->SoundIndex( "superfly/s_grunt1.wav" );
			break;
		case PLAYERCLASS_MIKIKO:
			sound_index = gstate->SoundIndex( "mikiko/m_grunt1.wav" );
			break;
		case PLAYERCLASS_HIRO:
			sound_index = gstate->SoundIndex( "hiro/grunt1.wav" );
			break;
		default:
			return;
		};

		gstate->StartEntitySound( pClient, CHAN_AUTO, sound_index, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );
		hook->fNextStruggleTime = gstate->time + STRUGGLE_DELAY;
	}
}

void Client_PlayUseStruggleSound( userEntity_t *pClient, userEntity_t *pTarget )
{
	// SCG[11/13/99]: Play the struggle sound
	doorHook_t	*pDoorHook = ( doorHook_t * ) pTarget->userHook;
	if( pDoorHook && pDoorHook->bStruggle )
	{
		playerHook_t *hook = AI_GetPlayerHook(pClient);
		if (!hook)
			return;

		if (hook->fNextStruggleTime <= gstate->time)
		{
			int sound_index = rand() % 3;
			char sound[80];
			switch(hook->iPlayerClass)
			{
			case PLAYERCLASS_SUPERFLY:
				sound_index = 1+ rand() % 5;
				Com_sprintf(sound,sizeof(sound),"superfly/s_struggle%d.wav",sound_index);
				sound_index = gstate->SoundIndex( sound );
				break;
			case PLAYERCLASS_MIKIKO:
				sound_index = 1 + rand() % 5;
				Com_sprintf(sound,sizeof(sound),"mikiko/m_struggle%d.wav",sound_index);
				sound_index = gstate->SoundIndex(sound);
				break;
			case PLAYERCLASS_HIRO:
				sound_index += gstate->SoundIndex( "hiro/grunt11.wav" );
				break;
			default:
				return;
			};

			gstate->StartEntitySound( pClient, CHAN_AUTO, sound_index, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );	
			hook->fNextStruggleTime = gstate->time + 2*STRUGGLE_DELAY;
		}
	}
}

/*
void Client_UseAction( userEntity_t *pClient, userEntity_t *pTarget, userInventory_t *pKeyItem )
{
	if( pKeyItem == NULL )
	{
		return;
	}

	if( pKeyItem->flags & ITF_USEONCE )
	{
		gstate->InventoryDeleteItem( pClient, pClient->inventory, pKeyItem );
	}

	// see if this key unlocks whatever requires it
	if( pKeyItem->flags & ITF_UNLOCK )
	{
		//	spawn a key sticking out of the door
		Client_SpawnKey( pTarget, pKeyItem );

		// find the entity which this one acts upon
		if( pTarget->target )
		{
			userEntity_t *pOther = com->FindTarget( pTarget->target );
			// it could target more than one door or entity
			while( pOther )
			{
				pOther->targetname	= NULL;		// now a door opens without a button
				pOther->keyname		= NULL;		// now it doesn't require a key
				pOther = com->FindTarget( pTarget->target );	// find the next one if multiple doors
			}	

		}
		// now remove the keyname requirement from the original entity
		pTarget->keyname = NULL;
	}
}
*/

qboolean Client_FuncButtonUse( userEntity_t *pClient, userEntity_t *pTarget )
{
	return com_DoKeyFunctions( pTarget, pClient, tongue_world[T_MUST_HAVE_KEY] );
/*
	userInventory_t *pKeyItem;

	// SCG[11/13/99]: Key check
	if( pTarget->keyname )
	{
		if( pClient->inventory && ( pClient->flags & FL_CLIENT ) )
		{
			pKeyItem = gstate->InventoryFindItem( pClient->inventory, pTarget->keyname );
			if( pKeyItem )
			{
				if( pKeyItem->flags & ITF_USEONCE )
				{
					gstate->InventoryDeleteItem( pClient, pClient->inventory, pKeyItem );
				}

				return TRUE;
			}
		}
	}
	else
	{
		return TRUE;
	}

	return FALSE;
*/
}

qboolean Client_FuncDoorUse( userEntity_t *pClient, userEntity_t *pTarget )
{
//	userInventory_t *pKeyItem;

	if( pTarget->targetname != NULL )
	{
		return FALSE;
	}

	return com_DoKeyFunctions( pTarget, pClient, tongue_world[T_MUST_HAVE_KEY] );
/*
	// SCG[11/13/99]: Key check
	if( pTarget->keyname )
	{
		if( pClient->inventory && ( pClient->flags & FL_CLIENT ) )
		{
			pKeyItem = gstate->InventoryFindItem( pClient->inventory, pTarget->keyname );
			if( pKeyItem )
			{
				if( pKeyItem->flags & ITF_USEONCE )
				{
					gstate->InventoryDeleteItem( pClient, pClient->inventory, pKeyItem );
				}

				return TRUE;
			}
		}
	}
	else
	{
		return TRUE;
	}

	return FALSE;
*/
}

qboolean Client_MiscUse( userEntity_t *pClient, userEntity_t *pTarget )
{
	return com_DoKeyFunctions( pTarget, pClient, tongue_world[T_MUST_HAVE_KEY] );
/*
	userInventory_t *pKeyItem;

	// SCG[11/13/99]: Key check
	if( pTarget->keyname )
	{
		if( pClient->inventory && ( pClient->flags & FL_CLIENT ) )
		{
			pKeyItem = gstate->InventoryFindItem( pClient->inventory, pTarget->keyname );
			if( pKeyItem )
			{
				if( pKeyItem->flags & ITF_USEONCE )
				{
					gstate->InventoryDeleteItem( pClient, pClient->inventory, pKeyItem );
				}

				return TRUE;
			}
		}
	}
	else
	{
		return TRUE;
	}

	return FALSE;
*/
}

void Client_Use (userEntity_t *self)
{
	CVector			org, end, start;
	trace_t			tr;
	CVector			mins(-8, -8, -8);
	CVector			maxs(8, 8, 8);
	qboolean		bCheckKeyFlags = FALSE, bUseTestsPassed = FALSE, bPlayStruggleSound = FALSE;
	userInventory_t	*pKeyItem = NULL;
	
	///////////////////////////////////////////////////////////////////////
	//	if in camera mode (looking at a monitor), go back to 1st person
	///////////////////////////////////////////////////////////////////////

	if ((self->flags & FL_CAMERAMODE)&&(self->delay < gstate->time))
	{
		//NSS[11/2/99]: New func_monitor stopping code here
		MONITOR_Stop(self);
		//Put a little more delay on there to prevent a 'flashback' due to high keyboard polling rates
		self->delay = gstate->time + 0.75f;
		return;
	}
	else if(self->flags & FL_CAMERAMODE)
	{
		return;
	}

	
	// make sure the client has been connected to the server
	if (!self->className)
		return;

	end = self->input_entity->client->v_angle;
	end.AngleToVectors (forward, right, up);

	start = self->input_entity->s.origin + CVector(0, 0, self->viewheight);
	end = start + forward * 80;
	tr = gstate->TraceBox_q2 (start, mins, maxs, end, self->input_entity, CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_CLEAR | CONTENTS_NOSHOOT | CONTENTS_DEADMONSTER);

	// is an entity standing in front of us?
	if (tr.ent)
	{
		// SCG[11/5/99]: entities that have target names or need keys cannot be used
		// SCG[2/5/00]: make sure the player cannot use triggers, only cinematic characters can use them.
		// SCG[2/5/00]: The reason we do not put a check at the beginning of each trigger is that some triggers
		// SCG[2/5/00]: are indirectly "used" by the player through intermediary objects.  We just want to
		// SCG[2/5/00]: block stuff directly used by the player.
		if( tr.ent->use && strstr( tr.ent->className, "trigger_" ) == NULL )
		{
			qboolean bNeedsKey, bHasTarget, bUseFuncButton, bUseFuncDoor, bUseFuncDoorRotate;
			bNeedsKey			= ( tr.ent->keyname != 0 );
			bHasTarget			= ( tr.ent->targetname != 0 );
			bUseFuncButton		= !stricmp( tr.ent->className, "func_button" );
			bUseFuncDoor		= ( !stricmp( tr.ent->className, "func_door" ) && !bHasTarget );
			bUseFuncDoorRotate	= ( !stricmp( tr.ent->className, "func_door_rotate" ) && !bHasTarget );

			// SCG[11/13/99]: Use the item
			if( bUseFuncDoor || bUseFuncDoorRotate )	bUseTestsPassed = Client_FuncDoorUse( self, tr.ent );
			else if( bUseFuncButton )					bUseTestsPassed = Client_FuncButtonUse( self, tr.ent );
			else if( bHasTarget == 0 )					bUseTestsPassed = Client_MiscUse( self, tr.ent );

			// SCG[11/13/99]: Use the item
			if( bUseTestsPassed )
			{
				if( tr.ent->delay )	SpawnDelay( tr.ent, self, self );
				else				tr.ent->use( tr.ent, self, self );
			}
			else 
			{
				Client_PlayUseStruggleSound( self, tr.ent );
			}
		}
		else if( !stricmp( tr.ent->className, "worldspawn" ) && ( tr.fraction != 1.0 ) )
		{
			Client_PlayUseWorldSound( self );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	Client_CheckInertialDamage
//
//	called each frame to check if the players velocity has decreased suddenly
//
//	treats xy velocity as separate from z velocity so that vertical velocity
//	is not compounded with horizontal velocity
//
//	if the client is possessing another input entity, then the client's current
//	powerup status overrides the input entity's
//
//	works with input_entity
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void	Client_CheckInertialDamage (userEntity_t *self)
{
	float			cur_vel, v_diff, damage, r;
	int				took_damage, landed;
	playerHook_t	*hook = (playerHook_t *)self->userHook;
	playerHook_t	*ihook = AI_GetPlayerHook( self->input_entity );
	CVector			v_temp;

	if( hook == NULL )
	{
		return;
	}

	float			z_damage_vel = 450.0; //NSS[11/16/99]:Lowered the damage velocity start value
	//NSS[11/27/99]:Added the 1 to speed boost to remvoe the 0 value
	float			xy_damage_vel = (ATTR_RUNSPEED + ((ATTR_RUNSPEED / ATTR_MAX_LEVELS) * ((float) hook->speed_boost)+1)) * 2;
    float			damage_div = 0.5;  // Logic[6/24/99]: cursed div! leverage FPU!

	// when acro boosted, ignore Z inertial damage, when speed boosted, ignore XY inertial damage
	bool			bSpeedBoosted =  (bool) (hook->speed_boost > hook->base_speed);
	bool			bAcroBoosted = 	 (bool)	(hook->acro_boost > hook->base_acro);

	landed = took_damage = FALSE;

	v_temp = self->input_entity->velocity;
    if(deathmatch->value && !dm_falling_damage->value)
        return;

#if 0 //DELETE SOON ???
    // in deathmatch, if falling damage is enabled, the velocity must be 2x
    // normal before damage is taken
/*    if(deathmatch->value)
    {
        z_damage_vel = z_damage_vel * 2;
        xy_damage_vel = xy_damage_vel * 2;
    }
*/
#endif
    ///////////////////////////////////////////////////////////////////////////
	//	Vertical velocity 
	///////////////////////////////////////////////////////////////////////////
    

	// get the current z velocity
	cur_vel = fabs (self->input_entity->velocity [2]);
	// find the difference between current velocity and last velocity
	v_diff = ihook->last_zvel - cur_vel;

	if (v_diff > z_damage_vel)
	{
		// change in velocity is > z_damage_velocity

		// find amount of health to subtract
		//NSS[11/16/99]:Increased the amount to 12.5%
		//NSS[11/19/99]:Decreased the amount to 6.25%  (50%)
		damage = floor ((v_diff - z_damage_vel) * 0.0625);
		// scale down damage based on acro skill
		damage = damage * ((10.0 - (float)hook->acro_boost) * 0.1);
		
		// amw: if we're acro boosted, don't take damage from z velocity changes
		if (damage && !bAcroBoosted)
		{
			client_deathtype = FELL;
			//NSS[11/16/99]:I commented this out.. it should be across the board.
            //if (deathmatch->value)
            // damage = damage * 0.5;  // take half damage in deathmatch?  why?
			//NSS[11/18/99]:Adding for regular pain sounds if we haven't met the > 10 damage base requested by John
			unsigned long dmg_flags = DAMAGE_FALLING | DAMAGE_NO_BLOOD;
			if(damage >10)
				dmg_flags|=DAMAGE_PAINOVERRIDE;

			com->Damage (self->input_entity, 
                         self->input_entity, 
                         self->input_entity, 
                         zero_vector, zero_vector, damage, dmg_flags);  // 3.8 dsn

			took_damage = TRUE;
		}
		else landed = TRUE;
	}
	else if (v_diff > 300.0)
		landed = TRUE;

	// save current z velocity as last
	ihook->last_zvel = cur_vel;

	///////////////////////////////////////////////////////////////////////////
	//	Horizontal velocity
	///////////////////////////////////////////////////////////////////////////

	//	get the current xy velocity
	v_temp.z = 0;
	cur_vel = v_temp.Length();

	//	find the change in velocity from last check
	v_diff = ihook->last_xyvel - cur_vel;

	if (v_diff > xy_damage_vel)
	{
		// change in xy velocity is > xy_damage_vel

		// find amount of health to subtract
		//damage = floor ((v_diff - xy_damage_vel) * damage_div);

		// scale down damage based on acro skill
		damage = floor ((v_diff - xy_damage_vel) * 0.0625);
		damage = damage * ((8.0 - (float)hook->acro_boost) * 0.08);

		// amw: if we're speed boosted, don't take damage from xy velocity changes
		if (damage && !bSpeedBoosted)
		{
			client_deathtype = FELL;
			com->Damage (self->input_entity, 
                         self->input_entity, 
                         self->input_entity, zero_vector, zero_vector, damage, DAMAGE_FALLING | DAMAGE_NO_BLOOD | DAMAGE_PAINOVERRIDE);

			took_damage = TRUE;
		}
	}

	// save current xy velocity and last xy velocity
	ihook->last_xyvel = cur_vel;

	// if the player took any damage then play a hurt sound
	if (took_damage)
	{
		// FIXME: play sounds for input_entities other than client
		if (self->input_entity == self && damage > 10)
		{
			if (rnd () < 0.5)
                PlayClientSound(self, "landhurt2.wav");
    //            gstate->sound (self->input_entity, CHAN_BODY, "player/playerlandhurt2.wav", 255, ATTN_NORM);
			else
                PlayClientSound(self, "landhurt1.wav");
			// gstate->sound (self->input_entity, CHAN_BODY, "player/playerlandhurt.wav", 255, ATTN_NORM);
		}
	}
	else if (landed)
	{
		// FIXME: play sounds for input_entities other than client
		if (self->input_entity == self)
		{
			r = rnd ();
			if (r < 0.33)
                PlayClientSound(self, "land1.wav");
				// gstate->sound (self->input_entity, CHAN_BODY, "player/playerland1.wav", 255, ATTN_NORM);
			else if (r < 0.66)
                PlayClientSound(self, "land2.wav");
				// gstate->sound (self->input_entity, CHAN_BODY, "player/playerland2.wav", 255, ATTN_NORM);
			else
                PlayClientSound(self, "land3.wav");
				// gstate->sound (self->input_entity, CHAN_BODY, "player/playerland3.wav", 255, ATTN_NORM);
			
            // playerland4.wav is a water land.
            //else
	        //  gstate->sound (self->input_entity, CHAN_BODY, "player/playerland4.wav", 255, ATTN_NORM);
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
//NSS[11/27/99]: Not finished!!!!
//	Burning_Fx_Think
//  Yes... feel the burn...be the burn... see the burn.
//
//////////////////////////////////////////////////////////////////////////////
void Burning_Fx_Think(userEntity_t *self)
{
	
	if(self->delay < gstate->time)
	{
		playerHook_t *hook	= (playerHook_t *)self->owner->userHook;

		if(hook)
		{
			hook->ai_flags &= ~AI_ISBURNING;
		}
		RELIABLE_UNTRACK(self);
		self->remove (self);
	}

}


///////////////////////////////////////////////////////////////////////////////
//NSS[11/27/99]:Not finished!!!!
//	Client_Lava_FX
//  Causes smoke and Fire and Ass to spray all over the place.
//
//////////////////////////////////////////////////////////////////////////////
void Client_Lava_FX(userEntity_t *self)
{
	//Setup the client stuff here
	playerHook_t *hook	= (playerHook_t *)self->userHook;
	if(hook)
	{
		if(!(hook->ai_flags & AI_ISBURNING))
		{
			hook->ai_flags		|= AI_ISBURNING;
	
			//Create the new entity here
			userEntity_t *ent	= gstate->SpawnEntity();
			ent->think			= Burning_Fx_Think;
			ent->owner			= self;
			ent->s.modelindex	= gstate->ModelIndex("models/global/e_fireb.sp2");
			ent->delay			= gstate->time + 3.0f;  //Burn for 3 seconds
			ent->nextthink		= gstate->time + 0.1f;  //
			ent->s.render_scale.Set(3.0f,3.0f,3.0f);
			gstate->LinkEntity(ent);
			
			//setup track entity stuff here
			trackInfo_t tinfo;
			memset(&tinfo, 0, sizeof(tinfo));
			tinfo.ent		= ent;
			tinfo.srcent	= ent;
			tinfo.flags		|= TEF_FXFLAGS|TEF_MODELINDEX|TEF_SRCINDEX;
			tinfo.fxflags	|= TEF_ADDFX|TEF_FIRE|TEF_SMOKETRAIL;
			tinfo.modelindex=gstate->ModelIndex("models/global/e_fireb.sp2");
			com->trackEntity(&tinfo,MULTICAST_ALL);
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
//	Client_CheckWaterDamage
//
//	check's for water damage to input_entity
//	uses input_entity's envirosuit/etc info
//
//	FIXME:	Doesn't support input_entity correctly yet
//			need to make all entities get same self->waterlevel info
//////////////////////////////////////////////////////////////////////////////
#define FREEZE_FRAME_DECREMENT			(0.01)
#define FREEZE_INTERVAL					(2.0)
#define FREEZE_DAMAGE					(5.0)

void	Client_CheckWaterDamage (userEntity_t *self)
{
	playerHook_t	*hook = (playerHook_t *)self->userHook;
	playerHook_t	*ihook = AI_GetPlayerHook( self->input_entity );
	float			time_under;
	CVector			temp;
	qboolean		bIsSuperfly, bIsMikiko, bIsHiro;

	bIsSuperfly = bIsMikiko = bIsHiro = false;

	if( hook == NULL || ihook == NULL )
	{
		return;
	}

	if(_stricmp(hook->cur_sequence->animation_name,"swim") != 0 && self->waterlevel > 2)
	{
		char pszAnimation[128];
		CLIENT_SelectAnimationPerWeaponType( self, pszAnimation );
		AI_ForceSequence(self,pszAnimation,FRAME_LOOP);
	}

	//NSS[11/29/99]: Took out the early out with god mode because we need to reset water damage if in god mode.
    if ( self->input_entity->health <= 0 || self->input_entity->movetype == MOVETYPE_NOCLIP || (hook->dflags & DFL_LLAMA))
	{
		ihook->drown_time = gstate->time + 12;	// don't need air
		return;
	}

	if( !strcmp( self->className, "Superfly" ) )
	{
		bIsSuperfly = true;
	}
	else if( !strcmp( self->className, "Mikiko" ) )
	{
		bIsMikiko = true;
	}
	else
	{
		bIsHiro = true;
	}

	// cek[11-26-99] make the trident work as an underwater breather!
	short holdingTrident = 0;
	if (self && self->client && self->client->ps.weapon)
	{
		userEntity_t *weap = (userEntity_t *)self->client->ps.weapon;
		if (!stricmp(weap->className,"weapon_trident"))
			holdingTrident = 1;
	}

	bool bNoDrown = (ihook->envirosuit_time > 0 ) || (holdingTrident) || ((ihook->items & IT_MANASKULL) && ihook->invulnerability_time);
	if (bNoDrown)//holdingTrident || (ihook->envirosuit_time > 0 ))
		ihook->drown_time = gstate->time + 12;

	if (!bNoDrown)	// no suit and not holding trident
	{
		// check for freezing in episode 3 (not in dm)
		if (!deathmatch->value)
		{
			if ((self->input_entity->watertype & CONTENTS_WATER) && (gstate->episode == 3) && (self->input_entity->waterlevel > 0))
			{
				if ((gstate->time >= ihook->freezeTime) && (gstate->time >= ihook->freezeStart))
					ihook->freezeLevel += 0.15;
			}
			else
				ihook->freezeStart = gstate->time + 4;
		}
		else
		{
			ihook->freezeStart = gstate->time + 4;
		}

		///////////////////////////////////////////////////////////////////////
		//	Not wearing an EnviroSuit
		///////////////////////////////////////////////////////////////////////
		if (self->input_entity->waterlevel < 3)
		{
			//	not completely under water
			if (self == self->input_entity)
			{
				//	play player sounds if not possessing
				//	FIXME:	make possession state clearer...
                if (ihook->drown_time < gstate->time)
                    PlayClientSound(self, "breathe2.wav");
				else if (hook->drown_time < gstate->time + 9)
				{
				  if (rnd () < 0.5)
                    PlayClientSound(self, "breathe2.wav");
				  else
                    PlayClientSound(self, "choke1.wav");
				}
			}

			ihook->drown_time = gstate->time + 12;
		}
		else
		{
			//	completely under water
			if (ihook->drown_time <= gstate->time && ihook->pain_finished < gstate->time)
			{
				//	drowning damage
				time_under = gstate->time - ihook->drown_time;

				client_deathtype = DROWNED;
				com->Damage (self->input_entity, self->input_entity, self->input_entity, zero_vector, 
							 zero_vector, time_under * 0.75, DAMAGE_DROWN | DAMAGE_NO_BLOOD | DAMAGE_IGNORE_ARMOR );
				ihook->pain_finished = gstate->time + 1;

				if (self->input_entity->watertype & CONTENTS_WATER)
				{
					if (self == self->input_entity)
					{
						int sound_index;
						if (time_under > 8)
						{
							{
								sound_index = rand() % 3;
								if( bIsSuperfly ) sound_index += gstate->SoundIndex( "superfly/waterchoke2.wav" );
								else if( bIsMikiko ) sound_index += gstate->SoundIndex( "mikiko/waterchoke1.wav" );
								else if( bIsHiro ) sound_index += gstate->SoundIndex( "hiro/waterdeath1.wav" );
							}

							gstate->StartEntitySound( self, CHAN_AUTO, sound_index, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );

							self->input_entity->s.event = EV_SUCKITDOWN;
						}
						else
						{
							self->input_entity->s.event = EV_DROWNING;
							{
								if( bIsSuperfly ) sound_index = gstate->SoundIndex( "superfly/waterchoke2.wav" );
								else if( bIsMikiko ) sound_index = gstate->SoundIndex( "mikiko/waterchoke1.wav" );
								else if( bIsHiro ) sound_index = gstate->SoundIndex( "hiro/waterdeath1.wav" );
							}

							gstate->StartEntitySound( self, CHAN_AUTO, sound_index, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );
						}
					}
				}
			}
			else if ((int)((ihook->drown_time - gstate->time) * 5.0) % 5 == 0)
				//	let out a bubble every 1/2 second
				self->input_entity->s.event = EV_BUBBLES;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	//	check to see if we just got out of a liquid
	///////////////////////////////////////////////////////////////////////////
	
	if (self->input_entity->waterlevel == 0 && self->input_entity->client->old_waterlevel > 0)
	{
		//	just exited a liquid
		if (self == self->input_entity)
            PlayClientSound(self, "exitwater.wav");
			// gstate->sound (self->input_entity, CHAN_BODY, "player/playerexitwater.wav", 255, ATTN_NORM);


		return;
	}

    // Logic[5/10/99]: E3 Nitro Hack
    if(self->s.renderfx & RF_IN_NITRO)
    {
//        if(gstate->time >= self->nextthink)
            com->Damage(self, self, self, zero_vector, zero_vector, 32000, DAMAGE_LAVA | DAMAGE_NO_BLOOD | DAMAGE_FREEZE | DAMAGE_COLD);
			self->s.color.Set(.3,.3,.8);
    }
	if (self->input_entity->watertype & CONTENTS_LAVA)
	{
		//NSS[11/22/99]:Added envirosuit stuff here.
		//	do damage from lava
		if (ihook->pain_finished < gstate->time && (ihook->envirosuit_time <= 0 && !(ihook->items & IT_ENVIROSUIT)))
		{
			client_deathtype = LAVA;
			//Client_Lava_FX(self);
			
			com->Damage (self->input_entity, self->input_entity, self->input_entity, zero_vector, 
						 zero_vector, 10 * self->input_entity->waterlevel, DAMAGE_LAVA | DAMAGE_NO_BLOOD);
			ihook->pain_finished = gstate->time + 0.2;
		}
		else if( ihook->envirosuit_time > 0 && (ihook->items & IT_ENVIROSUIT))
		{
			//Decrement envirosuit time while in nasty stuff.  It does not wear off if out of slime or lava. (Per John request)
			ihook->envirosuit_time--;
		}

	}
	else if (self->input_entity->watertype & CONTENTS_SLIME)
	{
		//NSS[11/22/99]:The last segment of the slime contents if statement should have been a NOT... 
		if (ihook->pain_finished < gstate->time && (ihook->envirosuit_time <= 0 && !(ihook->items & IT_ENVIROSUIT)))
		{
			//	do damage from slime
			client_deathtype = SLIME;
			
			com->Damage (self->input_entity, self->input_entity, self->input_entity, zero_vector, zero_vector, 
					 4 * self->input_entity->waterlevel, DAMAGE_SLIME | DAMAGE_NO_BLOOD);
			//	1 second until next damage
			ihook->pain_finished = gstate->time + 1;
		}
		else if( ihook->envirosuit_time > 0 && (ihook->items & IT_ENVIROSUIT))
		{
			//Decrement envirosuit time while in nasty stuff.  It does not wear off if out of slime or lava. (Per John request)
			ihook->envirosuit_time--;
		}
	}
	else 	//NSS[11/26/99]:Check to determine if we have envirosuit but no trident and we are underwater
	if(self->input_entity->waterlevel > 2 && (ihook->items & IT_ENVIROSUIT) && (ihook->envirosuit_time > 0)  && (!holdingTrident))
	{
			//Decrement envirosuit time while in nasty stuff.  It does not wear off if out of slime or lava. (Per John request)
			ihook->envirosuit_time--;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	Client_GetSpawnPoint
//
///////////////////////////////////////////////////////////////////////////////

userEntity_t *Client_GetSpawnPoint( char *name )
{
	userEntity_t *spot = NULL;
	char	classname[64];

	if (deathmatch->value && deathmatch->value != 99 && deathmatch->value != 98)
	{
		Com_sprintf( classname, sizeof(classname),"%s_deathmatch", name );
		spot = com->DeathmatchSpawnPoint( classname );
	}
	if (!spot && (ctf->value || deathtag->value))
	{
		Com_sprintf( classname, sizeof(classname),"%s_team1", name );
		spot = com->DeathmatchSpawnPoint( classname );

		Com_sprintf( classname, sizeof(classname),"%s_team2", name );
		if( spot && !com->DeathmatchSpawnPoint( classname ) )
		{
			spot = NULL;		// Need two team
		}
	}
	if (!spot && deathmatch->value)
	{
		gstate->Con_Printf("No %s found in entity list.\n",classname);
//		gstate->Error ("Check for deathmatch spawnpoints.");
		return NULL;
	}
	if ( !spot )
	{
		Com_sprintf( classname, sizeof(classname),"%s_start", name );
		spot = com->SelectSpawnPoint( classname, gstate->game->spawnpoint );
	}

	return	spot;
}

///////////////////////////////////////////////////////////////////////////////
//	Client_Respawn
//
//	respawns the player after death
//	FIXME: sometimes player can't respawn... not fully dead?
///////////////////////////////////////////////////////////////////////////////
int	Client_Respawn (userEntity_t *self)
{
	userEntity_t	*spot;
	CVector			temp;
	playerHook_t	*hook = (playerHook_t *)self->userHook;

	// add a dead body to the world only if the client wasn't gibbed
	if (!(self->s.renderfx & RF_NODRAW))
		bodylist_add (self);

	if (ctf->value || deathtag->value)
		spot = TEAM_GetSpawnPoint( self );
	else if (coop->value)
		spot = COOP_GetSpawnPoint(self);
	else
		spot = Client_GetSpawnPoint( "info_player" );

	if (!spot)
	{
		gstate->Con_Printf ("No open spawn position for %s!\n", self->netname);
		return FALSE;
	}

	// kill scoreboard (always...its used in single player too...)
    self->client->showflags &= ~(SHOW_SCORES | SHOW_SCORES_NOW);
    if(deathmatch->value || coop->value)
    {
		// WAW[11/18/99]: Reset the client attributes.
//cek[1-5-00]  romero doesn't want this        Client_InitAttributes (self);

	    hook->power_boost = hook->base_power;
	    hook->attack_boost = hook->base_attack;
	    hook->speed_boost = hook->base_speed;
	    hook->acro_boost = hook->base_acro;
	    hook->vita_boost = hook->base_vita;

        com->CalcBoosts(self);		
    }

	self->s.origin = spot->s.origin;
	self->s.angles = spot->s.angles;
	self->s.event = EV_ENTITY_FORCEMOVE;	// amw: fixes ghosting when respawning

    // ISP: 3-16-99  never null out the first character in an animation sequence
	//				 bad things will happen if first character is NULLed out 
	// 03.16.99  force new setting of animation seq
    //hook->cur_sequence->animation_name[0] = NULL;
	hook->cur_sequence = NULL;

	Client_InitGClient (self, spot->s.origin, spot->s.angles);

	// alert client side of respawn
	gstate->WriteByte(SVC_CLIENTRESPAWN);
	gstate->UniCast(self,true);


	Client_InitUserEntity (self);
	gstate->LinkEntity (self);


	Client_InitTimers (self);

	if (!coop->value)
	{
		Client_InitInventory (self);
	}
	Client_InitWeapons (self);
	Client_InitHookMisc (self);
	Client_InitCamera (self);

	if (ctf->value)
		CTFClient_Respawn(self);

	spawn_tele_gib( self->s.origin, self );

	self->flags &= ~FL_FREEZE;
	Client_InitClientModel ( self );
	//&&&FIXME - fix this hack... it's sending down 4 model index values...
//	com->SetClientModels (self, self, self, self, self);
    
	Client_InitAnims (self);
	CLIENT_StartStand (self, TH_STAND);

//	if (!coop->value)
		Client_InitDefaultWeapon(self);

	func_t func = (func_t)com->FindSpawnFunction("dll_weapon_reinit_3p_models");
	if (func)
		func(self);

	// SCG[2/6/00]: clear velocity
	hook->last_xyvel = 0;
	hook->last_zvel = 0;

	recalc_level(self);
	COOP_SpawnHacks(self);
	return	TRUE;
}
void	camera_next_close_monster (userEntity_t *self);
///////////////////////////////////////////////////////////////////////////////
//	Client_DeathThink
//
//	called each frame if the player is dead
///////////////////////////////////////////////////////////////////////////////

void Client_DeathThink (userEntity_t *self)
{
	playerHook_t	*hook = (playerHook_t *)self->userHook;
	CVector			ang, dir;

	// pressed fire or force respawn is set
	if (((self->client->latched_buttons & BUTTON_ATTACK) || (self->client->latched_buttons & BUTTON_USE) || 
		 (deathmatch->value&&dm_force_respawn->value && (gstate->time > self->hacks))) && 
		 self->delay < gstate->time) // and our time is up
	{
		//	if in camera mode, turn it off
		if (hook->camera)
			camera_stop (self, false);

		if (deathmatch->value || coop->value )
		{
			// set flag to keep trying respawns if necessary
			hook->dflags = hook->dflags | DFL_RESPAWN;
			
			Client_Respawn (self);
		}
		else
		{
			gstate->SetCvar ("p_frametime", "1.0");

        	char			command_str [128];
			//	RESTARTING MAP IN SINGLE-PLAYER
			Client_FreeInventory (self);

			// restart the entire server
			Com_sprintf (command_str, sizeof(command_str),"load save1" );
			gstate->CBuf_AddText(command_str);
			self->client->latched_buttons = 0;
		}

		return;
	}
	else if (hook->dflags & DFL_RESPAWN && !(deathmatch->value&&dm_force_respawn->value))
	{
		// failed to find an open respawn spot during last frame, so try again
		Client_Respawn (self);
	}
	/////////////////////////////////////////////////////////////
	//	follow client's killer if visible
	/////////////////////////////////////////////////////////////

	if (!(deathmatch->value || coop->value))
	{
		if(self->hacks < gstate->time && self->delay < gstate->time)
		{
			camera_next_close_monster (self);
			
			self->hacks = gstate->time + 10.0f;
			if(self->deadflag != DEAD_PUNK)
				self->deadflag = DEAD_DEAD;
		}
//		if (com->Visible (self, self->enemy))
//		{
			//dir = self->enemy->s.origin - self->s.origin;
			//dir.Normalize ();
			//VectorToAngles( dir, ang );

			//camera_set_viewangle (self, ang);
//		}
	}
}


void AlertClientOfSidekickDeath(byte gibbed)
// a sidekick has died, so the game is over for the main player
{
  userEntity_t *client_entity = &gstate->g_edicts[1];	
  playerHook_t *phook = AI_GetPlayerHook( client_entity );


	//	don't allow restart for a few seconds
	client_entity->delay = gstate->time + 4.0f;
	client_entity->client->latched_buttons = 0; // reset buttons
	phook->jump_time = gstate->time + 4.0;      // don't allow jump

	com->InformClientDeath(client_entity,gibbed); // to reset client-side interfaces

	// NSS[12/6/99]:Oops...
	if( ( client_entity->inuse != 0 ) && ( client_entity->deadflag == DEAD_NO ) )
	{
		client_entity->deadflag = DEAD_SIDEKICK;
		AI_StopCurrentSequence( client_entity ); // stop animation dead
		char szAnimation[16];
		AI_SelectAmbientAnimation( client_entity, szAnimation );
		AI_ForceSequence(client_entity,szAnimation,FRAME_LOOP);
		Client_Die (client_entity,client_entity,client_entity,1, client_entity->s.origin);
	}
}

///////////////////////////////////////////////////////////////////////////////
//	Client_WarpThink
//
//	called when client is warping to move from node to node
//
//	self->enemy is node the client is heading for
//
//	FIXME: Move all this SETANGLE crap to client side!!
///////////////////////////////////////////////////////////////////////////////

//	info_warp_destination spawnflags
// SCG[6/27/00]: #define		INFOWARP_SKIPTO		1
// SCG[6/27/00]: #define		INFOWARP_GLOW		2

void	Client_WarpThink (userEntity_t *self)
{
/*
	float			dist;
	playerHook_t	*hook = (playerHook_t *)self->userHook;
	trigHook_t		*thook = (trigHook_t *) self->enemy->userHook;
	userEntity_t	*target, *went;
	CVector			temp, ang;

	dist = VectorDistance (self->s.origin, self->enemy->s.origin);

	if (dist < 32.0 || self->teleport_time <= gstate->time)
	{
		if (self->enemy->target)
			target = com->FindTarget (self->enemy->target);

		if (!self->enemy->target || !target)
		{
			///////////////////////////////////////////////////////////////////
			//	End of warp path:
			//	if the goal has no target or the target entity is not found
			//	then it is the final destination so restore the player to normal
			///////////////////////////////////////////////////////////////////

			// have client ramp field of view back down to 90
//			Client_SetFOVRamp (self, 120, 90);

			gstate->SetCameraState (self, false, 0);
			Client_ClearForceAngles (self);

//			self->flags -= FL_INWARP;
			self->flags &= ~FL_INWARP;

			self->solid = SOLID_NOT;
			self->movetype = MOVETYPE_TOSS;

			self->s.origin = self->enemy->s.origin;
			gstate->LinkEntity (self);

			/////////////////////////////////////////////////////////
			//	start drawing models again
			//	set render_scale for all model parts to 0.1
			//	"unstick" all models from self, so stretching effect
			//	looks cooler
			/////////////////////////////////////////////////////////
			self->s.renderfx -= (self->s.renderfx & RF_NODRAW);

			//	turn on weapon model
			went = (userEntity_t *) self->client->ps.weapon;
			if (went)
				went->s.renderfx &= ~RF_NODRAW;

			self->s.render_scale.Set(0.1, 0.1, 7.4);

			//	set client's angles to direction of warp exit
			ang = CVector(0, thook->mangle.y, 0);
			self->flags |= FL_FIXANGLES;
			gstate->SetClientAngles(self, ang);

			ang.AngleToVectors (forward, right, up);

			temp = self->velocity;
			temp = temp.Length() * -0.25 * forward;
			temp.z = 0;
			self->velocity = temp;
			self->viewheight = 22.0;

			//	turn of glow if glowing
			self->s.effects -= (self->s.effects & EF_DIMLIGHT);

			//	so exit node from teleporter/warp gets placed at actual exit!
			hook->last_origin = self->s.origin;

			com->CalcBoosts (self);

			// missing sound
			//gstate->sound (self, CHAN_BODY, "misc/warpend2.wav", 255, ATTN_NORM);

			////////////////////////////////////////////////////////////
			//	gib anything within clients bounding box
			////////////////////////////////////////////////////////////

			com->TeleFrag (self, 65535);

			////////////////////////////////////////////////////////////
			//	set up a think to handle rescaling of client
			////////////////////////////////////////////////////////////

			self->think = client_warp_appear;
			self->nextthink = gstate->time + THINK_TIME;

			return;
		}
		else
		{
			///////////////////////////////////////////////////////////////////
			//	Go to a new path:
			//	reached a goal with a target, so go towards its target
			//	save the target for checking in the client's prethink
			///////////////////////////////////////////////////////////////////

			self->s.origin = self->enemy->s.origin;
			gstate->LinkEntity (self);

			if (target->spawnflags & INFOWARP_GLOW)
				//	turn light on if spawnflag is set
				self->s.effects |= EF_DIMLIGHT;
			else
				//	turn light off if not set on this path corner
				self->s.effects -= (self->s.effects & EF_DIMLIGHT);

			//	if destination is not visible, then go straight to next path
			if (target->spawnflags & INFOWARP_SKIPTO)
			{
				// put player right on path corner
				self->s.origin = target->s.origin;
				gstate->LinkEntity (self);

				// make player face target
				temp = self->s.origin - target->s.origin;
				temp.Length();
				VectorToAngles( temp, ang );


				gstate->SetClientAngles (self, ang);
				self->flags |= FL_FIXANGLES;

				self->teleport_time = gstate->time;
			}

			self->enemy = warp_setvalues (self, target);
		}
	}
	else
	{
		///////////////////////////////////////////////////////////////////////
		//	in transit to another info_warp_destination.  Compute view angles
		//	changes
		///////////////////////////////////////////////////////////////////////
		target = self->enemy;

		//	force player's velocity to max out
		temp = target->s.origin - self->s.origin;
		temp.Normalize();
		temp = temp * hook->run_speed;
		self->velocity = temp;
	}
*/
}

///////////////////////////////////////////////////////////////////////////////
//	Client_AnimateWeapon
//
//	THIS CODE SHOULD REFLECT ANY CHANGES TO com->FrameUpdate
//
//	animates the client's viewmodel each frame
//	skips frames if frame rate drops below 20
///////////////////////////////////////////////////////////////////////////////

void	Client_AnimateWeapon (userEntity_t *self)
{
	userEntity_t	*weapon=(userEntity_t *)self->client->ps.weapon;
	playerHook_t	*hook = (playerHook_t *)self->userHook;
	float			temp;
	int curframe;
	int				add, new_frame;

	if (!weapon)
		return;

	// no frame changes if FRAME_STATIC is set
	if (!(weapon->s.frameInfo.frameFlags & FRAME_STATIC))//damnthee
	{
		// increment frames according to length of last frame
		weapon->s.frameInfo.next_frameTime = weapon->s.frameInfo.next_frameTime + gstate->frametime;

		if (weapon->s.frameInfo.next_frameTime > weapon->s.frameInfo.frameTime)
		{
			temp = floor (weapon->s.frameInfo.next_frameTime / weapon->s.frameInfo.frameTime);
			add = (int)(weapon->s.frameInfo.frameInc * temp);

			curframe = weapon->s.frame;
			new_frame = curframe+add;

			///////////////////////////////////////////////////////////////////////
			//	set/clear FRSTATE_LAST flag if on last frame
			///////////////////////////////////////////////////////////////////////

//			if (weapon->s.frameInfo.endFrame <= new_frame && weapon->s.frameInfo.endFrame > curframe)
//				weapon->s.frameInfo.frameState |= FRSTATE_LAST;

			// call special function on specific frame
			if ((hook->fxFrameFunc) && (weapon->s.frame<=hook->fxFrameNum) && (new_frame>=hook->fxFrameNum))
				hook->fxFrameFunc(self);

			///////////////////////////////////////////////////////////////////////
			//	set FRSTATE_PLAYSOUND flags if correct frame
			///////////////////////////////////////////////////////////////////////

            if (weapon->s.frameInfo.sound1Frame >= curframe && weapon->s.frameInfo.sound1Frame < new_frame)
				weapon->s.frameInfo.frameState |= FRSTATE_PLAYSOUND1;

			if (weapon->s.frameInfo.sound2Frame >= curframe &&  weapon->s.frameInfo.sound2Frame < new_frame)
				weapon->s.frameInfo.frameState |= FRSTATE_PLAYSOUND2;

			weapon->s.frame += add;
			curframe += add;

			// are we currently looping within a larger sequence?
			if (weapon->s.frameInfo.frameFlags & FRAME_REPEAT)
			{
				// determine if sequence needs to loop back to loopStart
				if (curframe > weapon->s.frameInfo.endLoop)
				{
					weapon->s.frame = weapon->s.frameInfo.startLoop;
					weapon->s.frameInfo.loopCount--;
					// if count is <= 0, stop the looping of this partial sequence
					if (weapon->s.frameInfo.loopCount <= 0)
					{
						weapon->s.frameInfo.frameFlags -= FRAME_REPEAT;
						weapon->s.frameInfo.startLoop = weapon->s.frameInfo.startFrame;
						weapon->s.frameInfo.endLoop = weapon->s.frameInfo.endFrame;
					}
				}
			}
			// going through an entire sequence
			else
			{
				// determine if sequence needs to loop back to start
				if (weapon->s.frame > weapon->s.frameInfo.endFrame)
				{
					if (weapon->s.frameInfo.frameFlags & FRAME_ONCE)
					{
						// we've gone through once, so set frames to static (no updates)
//						weapon->s.frameInfo.frameState |= FRSTATE_STOPPED;
						weapon->s.frameInfo.frameFlags |= FRAME_STATIC;
						weapon->s.frameInfo.frameFlags -= FRAME_ONCE;

						weapon->s.frame = weapon->s.frameInfo.endFrame;

						//	if a weapon switch, start bringing new weapon up
						if (weapon->s.frameInfo.frameFlags & FRAME_WEAPONSWITCH)
							hook->weapon_next (self);
					}
					else
						weapon->s.frame = weapon->s.frameInfo.startFrame;
				}
			}

			weapon->s.frameInfo.next_frameTime = weapon->s.frameInfo.next_frameTime - (temp * weapon->s.frameInfo.frameTime);
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		CLIENT_HandleStopGoingFurther
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int CLIENT_HandleStopGoingFurther( userEntity_t *self )
{
 	_ASSERTE( self );
	if ( (self->movetype == MOVETYPE_NOCLIP) || (self->movetype == MOVETYPE_BOUNCE))
	{
		return FALSE;
	}

	playerHook_t *hook = AI_GetPlayerHook( self );
	// NSS[3/1/00]:	Check your fucking pointers!
	if(hook == NULL)
		return FALSE;
	
	NODELIST_PTR pNodeList = hook->pNodeList;
	// NSS[3/1/00]:	Check your fucking pointers!
	if(hook->pNodeList == NULL)
		return FALSE;
	
	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	MAPNODE_PTR pCurrentNode = NULL;
	if ( pNodeList->nCurrentNodeIndex >= 0 )
	{
		pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
	}
	if ( !pCurrentNode )
	{
		pCurrentNode = NODE_GetClosestNode(self);
	}

	if ( !pCurrentNode )
	{
		return FALSE;
	}

	int bRetValue = FALSE;
	float fDistance = VectorDistance( self->s.origin, pCurrentNode->position );
	if ( AI_IsCloseDistance2( self, fDistance ) && 
         pCurrentNode->node_type & NODETYPE_STOPSIDEKICK &&
         NODE_IsTriggered( pCurrentNode ) == FALSE )
    {
        int nOrder = 1;
        userEntity_t *pFirstSidekick = AIINFO_GetFirstSidekick();
	    if ( AI_IsAlive( pFirstSidekick ) )
	    {
            if ( SIDEKICK_DoStopGoingFurther( pFirstSidekick, pCurrentNode, nOrder ) )
            {
                nOrder++;
                bRetValue = TRUE;
            }
	    }

        userEntity_t *pSecondSidekick = AIINFO_GetSecondSidekick();
	    if ( AI_IsAlive( pSecondSidekick ) )
	    {
            if ( SIDEKICK_DoStopGoingFurther( pSecondSidekick, pCurrentNode, nOrder ) )
            {
                bRetValue = TRUE;
            }
        }

        if ( bRetValue == TRUE )
        {
            NODE_SetTriggered( pCurrentNode );
        }
    }

    return bRetValue;
        
}

// ----------------------------------------------------------------------------
//
// Name:		CLIENT_HandleTeleportAndComeNear
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int CLIENT_HandleTeleportAndComeNear( userEntity_t *self )
{
 	_ASSERTE( self );
	if ( (self->movetype == MOVETYPE_NOCLIP) || (self->movetype == MOVETYPE_BOUNCE))
	{
		return FALSE;
	}

	playerHook_t *hook = AI_GetPlayerHook( self );
	// NSS[3/1/00]:	Check your fucking pointers!
	if(hook == NULL)
		return FALSE;
	
	NODELIST_PTR pNodeList = hook->pNodeList;
	// NSS[3/1/00]:	Check your fucking pointers!
	if(hook->pNodeList == NULL)
		return FALSE;
	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	MAPNODE_PTR pCurrentNode = NULL;
	if ( pNodeList->nCurrentNodeIndex >= 0 )
	{
		pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
	}
	if ( !pCurrentNode )
	{
		pCurrentNode = NODE_GetClosestNode(self);
	}

	if ( !pCurrentNode )
	{
		return FALSE;
	}

	int bRetValue = FALSE;
	float fDistance = VectorDistance( self->s.origin, pCurrentNode->position );
	if ( AI_IsCloseDistance2( self, fDistance ) && 
         pCurrentNode->node_type & NODETYPE_TELEPORTSIDEKICK &&
         NODE_IsTriggered( pCurrentNode ) == FALSE )
    {
        int nOrder = 1;
        userEntity_t *pFirstSidekick = AIINFO_GetFirstSidekick();
	    if ( AI_IsAlive( pFirstSidekick ) )
	    {
            if ( SIDEKICK_DoTeleportAndComeNear( pFirstSidekick, pCurrentNode, nOrder ) )
            {
                nOrder++;
                bRetValue = TRUE;
            }
	    }

        userEntity_t *pSecondSidekick = AIINFO_GetSecondSidekick();
	    if ( AI_IsAlive( pSecondSidekick ) )
	    {
            if ( SIDEKICK_DoTeleportAndComeNear( pSecondSidekick, pCurrentNode, nOrder ) )
            {
                bRetValue = TRUE;
            }
        }

        if ( bRetValue == TRUE )
        {
            NODE_SetTriggered( pCurrentNode );
        }
    }

    return bRetValue;
        
}

///////////////////////////////////////////////////////////////////////////////
//	Client_PreThink
//
//	called each frame before the physics are run
///////////////////////////////////////////////////////////////////////////////

void Client_PreThink (userEntity_t *self)
{
	userEntity_t	*went = (userEntity_t *) self->client->ps.weapon;
	int				pc1;
	weapon_t		*weap = (weapon_t *) self->curWeapon;
	CVector			temp;
	playerHook_t	*hook = (playerHook_t *)self->userHook;
	playerHook_t	*ihook = (playerHook_t *)self->input_entity->userHook;
	short			loopambient=false;
    float           speed;

	if( hook == NULL || ihook == NULL )
	{
		return;
	}

	if (went && (went->flags & FL_LOOPAMBIENT_ANIM))
	{
		loopambient=true;
	}

	client_deathtype = NONE;

	/////////////////////////////////////////////////////////////////
	// handle ambient weapon anims
	/////////////////////////////////////////////////////////////////

    if(went && went->inuse && (gstate->time >= hook->attack_finished))
    {
        speed = self->velocity.Length();
        if(speed)
        {
            if(went->s.frameInfo.frameFlags & FRAME_STATIC)
            {
                weap->command ((userInventory_t *)weap->ammo,"bob_off",self);
            }
            //    weap->command ((userInventory_t *)weap->ammo,"bob_off",self);    
/*            
            // Play bob animation?
            if(speed > 200) // running
                weap->command ((userInventory_t *)weap->ammo,"bob_run",self);
            else    // walking
                weap->command ((userInventory_t *)weap->ammo,"bob_walk",self);
*/
        } 
        else if ( (went->s.frameInfo.frameFlags & FRAME_STATIC) ) 
        {
            if ((self->velocity.x==0 && self->velocity.y==0 && self->velocity.z==0) || loopambient)
            {
                if (gstate->time >= hook->ambient_time)
                {
                    weap->command ((userInventory_t *)weap->ammo,"ambient",self);
					
                    // if ambient_time wasn't changed, assume the command func doesn't handle
                    // ambients ... so, don't call it anymore for this weapon
                    if (gstate->time >= hook->ambient_time)
						hook->ambient_time=99999999;
                }
            } 
			else 
			{ 
                if (gstate->time >= hook->ambient_time)          // this is so ambient doesn't always start
				{
                    hook->ambient_time=gstate->time+2.0+(rnd()*6);     // right when you stop moving
				}
            }
        }
        /*
        else
        {
            // stop bob animation
            weap->command ((userInventory_t *)weap->ammo,"bob_off",self);
        }
        */
    }


	/////////////////////////////////////////////////////////////////
	// move camera
	/////////////////////////////////////////////////////////////////

	if (hook->camera && _stricmp(hook->camera->className, "func_monitor"))
	{
		cameraHook_t	*chook = (cameraHook_t *) hook->camera->userHook;

		if (chook && chook->pre_think)
			chook->pre_think (hook->camera);
	}

	/////////////////////////////////////////////////////////////////
	//	check to see if client on a warp path
	//	frame updates don't happen in a warp
	/////////////////////////////////////////////////////////////////

	if (self->flags & FL_INWARP)
		return;

	/////////////////////////////////////////////////////////////////
	//	animate the client's player models and weapon
	///////////////////////////////////////////////////////////////

/*
	//	animate the input entity's models
	com->FrameUpdate (self);
*/

	//////////////////////////////////////////////////////////////////
	//	shake client view
	//////////////////////////////////////////////////////////////////
	if( self->client )
	{
		if (hook->shake_time > gstate->time)
		{
			self->client->v_angle.x += (int)(rnd()* 20) - 10;
			self->client->v_angle.y += (int)(rnd()* 20) - 10;
		}
	}


	//	drowning can kill player, so don't go on
	if (self->deadflag == DEAD_DEAD)
	{
		return;
	}

	//////////////////////////////////////////////////////////////////
	//	fog stuff
	//	FIXME: someone broke this fog stuff
	//////////////////////////////////////////////////////////////////
	if (gstate->fogactive)
	{
		// check for water and turn fog off if we are in it
		// or on if we have just come out

		temp = self->s.origin + self->view_ofs;
		temp.z += self->velocity.z * 0.04;
		pc1 = gstate->PointContents( temp );
	}

	//////////////////////////////////////////////////////////////////
	//	node placement
	//////////////////////////////////////////////////////////////////

    if ( !deathmatch->value )
    {
	    float fMoveDistance = ihook->last_origin.Distance( self->input_entity->s.origin );
	    if( ihook->pNodeList == NULL )
		{
			int shit = 1;
		}

		if ( fMoveDistance > 0.0f && ( ihook->pNodeList != NULL ) )
	    {
            node_place_node( self->input_entity, ihook->pNodeList, self->input_entity->s.origin );

            if ( !CLIENT_HandleStopGoingFurther( self ) )
            {
                CLIENT_HandleTeleportAndComeNear( self );
            }
	    }
        else
        {
            // store the last moved position
            lastPlayerMovedPosition = self->input_entity->s.origin;
        }

        ihook->last_origin = self->input_entity->s.origin;

        // determine if the player/bot has left the ground between nodes
        // used for determining when to link nodes back
        if (self->input_entity->groundEntity == NULL)
	        ihook->bOffGround = TRUE;
    }

    // Health check
    if(hook && self->health > hook->base_health)
    {
        if(gstate->time -1 > hook->fLastHealthTick)
        {
            self->health -= 1.0;
            hook->fLastHealthTick = gstate->time;
        }
    }

}
///////////////////////////////////////////////////////////////////////////////
//	Client_CheckPowerUps
//
//	called each frame before the physics are run
///////////////////////////////////////////////////////////////////////////////
void Client_CheckPowerUps (userEntity_t *self)
{
	playerHook_t	*hook = (playerHook_t *)self->userHook;
	int				recalc = FALSE;

	if( hook == NULL )
	{
		return;
	}

	// exp_flags - set for each item when picked up and cleared when the item's expire message is sent
	// items - set when an item is picked up and cleared when the item becomes inactive
	// xx_time - set when an item is picked up to number of seconds till item expires

	float &freezeLevel = hook->freezeLevel;
	if (freezeLevel > 0)
	{
		if (gstate->time > hook->freezeTime)
		{
			// clamp it so we can't have any insanity...
			if (freezeLevel > 1) freezeLevel = 1;

			// blue flash so the client knows he's freezing...
			com->FlashClient(self, 0.4 * freezeLevel,0.2 ,0.2 ,1.0 , 0.05f );

			gstate->damage_inflicted = 0;

			// do some damage
			float freeze = freezeLevel * FREEZE_DAMAGE;
			client_deathtype = NONE;		// will give a failed at life and negative frag...
			com->Damage (self, self, self, zero_vector, zero_vector, freeze, DAMAGE_NO_BLOOD|DAMAGE_IGNORE_ARMOR|DAMAGE_COLD );

//			gstate->Con_Dprintf("Freezelevel: %f - dmg: %f\n",freezeLevel,gstate->damage_inflicted);
			if ( gstate->damage_inflicted > 0)
			{
				// play some sounds
				int sound_index = rand() & 1;	// 0 or 1!
				switch(hook->iPlayerClass)
				{
				default:
					if(sound_index)
						sound_index = gstate->SoundIndex( "hiro/icehurt2.wav" );
					else
						sound_index = gstate->SoundIndex( "hiro/icehurt1.wav" );
					break;
				case PLAYERCLASS_MIKIKO:
					if(sound_index)
						sound_index = gstate->SoundIndex( "mikiko/icehurt2.wav" );
					else
						sound_index = gstate->SoundIndex( "mikiko/icehurt1.wav" );
					break;
				case PLAYERCLASS_SUPERFLY:
					if(sound_index)
						sound_index = gstate->SoundIndex( "superfly/icehurt2.wav" );
					else
						sound_index = gstate->SoundIndex( "superfly/icehurt1.wav" );
					break;
				};
				if (sound_index)
					gstate->StartEntitySound( self, CHAN_AUTO, sound_index, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );
			}
			hook->freezeTime = gstate->time + FREEZE_INTERVAL;
		}

		// set up the color
		float r,g,b;
		r=g= 0.7-0.6*freezeLevel;
		if (r < 0.3) {r=g=0.3;}
		b = 0.4 + freezeLevel;
		if (b > .8) b = .8;
		self->s.color.Set(r,g,b);

		// update the info
		if (!( (gstate->episode == 3) && (self->input_entity->waterlevel > 1) ))
			freezeLevel -= FREEZE_FRAME_DECREMENT;
	}
	else
		freezeLevel = 0;

	if ((freezeLevel == 0) && (self->s.color.Length() > 0))
	{
		self->s.color.Set(0,0,0);
	}
	////////////////////////////////////////
	// poison
	////////////////////////////////////////
        //play sound and message after last poisoning is complete -KRH
	if( ( hook->exp_flags & EXP_POISON ) && hook->poison_time <= 0.2)
	{
		//	poison expiration sound and message
		gstate->StartEntitySound (self, CHAN_VOICE, gstate->SoundIndex("global/a_poisonfade.wav"), 1.0f, ATTN_NORM_MIN,ATTN_NORM_MAX);

		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierID(RESOURCE_ID_FOR_STRING(tongue_world,T_POWERUPS_POISON_EXPIRE),0);
		gstate->cs.Unicast(self,CS_print_center,2.0);
		gstate->cs.EndSendString();

		hook->exp_flags &= ~EXP_POISON;
	}

	if( hook->items & IT_POISON )
	{
		//	do damage
		if (hook->poison_next_damage_time <= 0)
		{
			//NSS[11/18/99]:Flash the client's screen green... they be a poisoned little bugger!
			CVector Color;
			Color.Set(0.10,0.65,0.10);
			com->FlashClient(self, 0.35f,Color.x ,Color.y ,Color.z , 0.05f );

			gstate->cs.BeginSendString();
			gstate->cs.SendSpecifierID(RESOURCE_ID_FOR_STRING(tongue_world,T_PLAYER_POISON),0);
			gstate->cs.Unicast(self,CS_print_center,2.0);
			gstate->cs.EndSendString();

			client_deathtype = POISON;
			com->Damage (self, self, self, zero_vector, zero_vector, hook->poison_damage, DAMAGE_POISON | DAMAGE_NO_BLOOD);
			hook->poison_next_damage_time = hook->poison_interval;
		}
		//	poison expire
		if (hook->poison_time <= 0)
		{
			hook->items &= ~IT_POISON;
			hook->poison_time = 0.0f;
			hook->poison_interval = 0.0f;
			hook->poison_next_damage_time = 0.0f;
		}
		else
		{
			hook->poison_time -= 0.1f;
//			hook->poison_interval -= 0.1;
			hook->poison_next_damage_time -= 0.1f;
		}

	}

	////////////////////////////////////////
	// envirosuit
	////////////////////////////////////////

	if( hook->items & IT_ENVIROSUIT )
	{
		//	envirosuit expires
		if (hook->envirosuit_time <= 0)
		{
			gstate->InventoryDeleteItem(self, self->inventory, gstate->InventoryFindItem(self->inventory, "item_envirosuit"));
			hook->items &= ~IT_ENVIROSUIT;
		}
		else if(hook->envirosuit_time == 30)
		{
			gstate->StartEntitySound (self, CHAN_VOICE, gstate->SoundIndex("artifacts/envirosuitfade.wav"), 0.85f, ATTN_NORM_MIN,ATTN_NORM_MAX);
			hook->envirosuit_time--;
		}
		// NSS[3/2/00]:
		if(hook->envirosuit_time > 1 &&  !((int)hook->envirosuit_time%15) && self->waterlevel > 2)
			gstate->StartEntitySound (self, CHAN_VOICE, gstate->SoundIndex("artifacts/envirosuituse.wav"), 0.85f, ATTN_NORM_MIN,ATTN_NORM_MAX);
	}

	////////////////////////////////////////
	// invulnerability
	////////////////////////////////////////

	if (hook->items & IT_MANASKULL)
	{
		if( hook->invulnerability_time <= 3.0 && ( hook->exp_flags & EXP_MANASKULL ) )
		{
			//	megashield expiration sound and message
			gstate->cs.BeginSendString();
			gstate->cs.SendSpecifierID(RESOURCE_ID_FOR_STRING(tongue_world,T_POWERUPS_MANASKULL_EXPIRE),0);
			gstate->cs.Unicast(self,CS_print_center,2.0);
			gstate->cs.EndSendString();
			gstate->StartEntitySound (self, CHAN_AUTO, gstate->SoundIndex("artifacts/manaskullfade.wav"), 1.0f, ATTN_NORM_MIN,ATTN_NORM_MAX);
			hook->exp_flags &= ~EXP_MANASKULL;
		}

		if( hook->invulnerability_time < 0 )
		{
			hook->items &= ~IT_MANASKULL;
		}
		else
			hook->invulnerability_time -= 0.1;
	}
	////////////////////////////////////////
	// wraithorb
	////////////////////////////////////////

	if (hook->items & IT_WRAITHORB)
	{
		userEntity_t *pWeapon = NULL;
		
		self->s.renderfx |= RF_TRANSLUCENT;
		
		// make the clients weapons translucent also
		if (self->client && self->client->ps.weapon)
		{
			pWeapon = (userEntity_t*)self->client->ps.weapon;
			pWeapon->s.renderfx |= RF_TRANSLUCENT;
//#pragma message("TODO AMW: remove this depthhack hack")
//			pWeapon->s.renderfx &= ~RF_DEPTHHACK;
		}


		//	FIXME:	make wratthorb work
		if( hook->wraithorb_time >= 0 )
		{
			float speed = self->velocity.Length();

			// set translucency for player based on velocity
			// set to correct player model
			if (speed < 1.0)
			{
				if (self->s.alpha > 0.05)	// as we're standing still, go invisible
					self->s.alpha -= 0.03;
			}
			else
			{
				if (self->s.alpha < 0.20)	// when moving, go more visible
					self->s.alpha += 0.03;
				else if ((self->s.alpha > 0.20) && (self->s.alpha <= 1.0))
					self->s.alpha -= 0.02;
			}
			
			if (pWeapon)
				pWeapon->s.alpha = self->s.alpha;

			// shift into predator mode if we're translucent enough
			if (self->s.alpha < 0.22)
			{
				self->s.renderfx |= RF_PREDATOR;
				if (pWeapon)
					pWeapon->s.renderfx |= RF_PREDATOR;
			}
			hook->wraithorb_time -= 0.1;
		}
		else
		{
            // bring translucency back up...
            if (self->s.alpha < 1.0f)
                self->s.alpha += 0.02;
			
            if (self->s.alpha > 1.0f)
                self->s.alpha = 1.0f;
            
            if (pWeapon)
                pWeapon->s.alpha = self->s.alpha;

            if (self->s.alpha > 0.22)
            {
				self->s.renderfx &= (~RF_PREDATOR);
				if (pWeapon)
					pWeapon->s.renderfx &= (~RF_PREDATOR);
            }

			//	wraithorb expires when translucency is back to full
            if (1.0f == self->s.alpha)
            {
    			hook->items = hook->items - (hook->items & IT_WRAITHORB);
    			self->s.renderfx &= ~RF_TRANSLUCENT;
    			self->s.renderfx &= ~RF_PREDATOR;
    			if (pWeapon)
    			{
    				pWeapon->s.renderfx &= ~RF_TRANSLUCENT;
    				pWeapon->s.renderfx &= ~RF_PREDATOR;
//    #pragma message("TODO AMW: remove this depthhack hack")
//    				pWeapon->s.renderfx |= RF_DEPTHHACK;
    			}
			    hook->exp_flags -= (hook->exp_flags & IT_WRAITHORB);
			}
			// added this so the warning plays before it is a moot point, independent of time setting -KRH
			if ((0.3f < self->s.alpha) && (0.32 >= self->s.alpha))
            {
                //	wraithorb expiration sound and message
                gstate->StartEntitySound (self, CHAN_AUTO, gstate->SoundIndex("artifacts/wraithorbburnout.wav"), 1.0f, ATTN_NORM_MIN,ATTN_NORM_MAX);

				gstate->cs.BeginSendString();
				gstate->cs.SendSpecifierID(RESOURCE_ID_FOR_STRING(tongue_world,T_POWERUPS_WRAITHORB_EXPIRE),0);
				gstate->cs.Unicast(self,CS_print_center,2.0);
				gstate->cs.EndSendString();
            }
		}
	}

	/////////////////////////////////////////////
	// stat boosters
	/////////////////////////////////////////////

	if( hook->items & IT_POWERBOOST )
	{
// SCG[1/4/00]: 		if( ( hook->power_boost_time <= ( gstate->time + BOOST_BLINKTIME ) ) && ( hook->exp_flags & EXP_POWERBOOST ) )
		if( ( hook->power_boost_time <= BOOST_BLINKTIME ) && ( hook->exp_flags & EXP_POWERBOOST ) )
		{
			hook->exp_flags &= ~EXP_POWERBOOST;     // reset expiring flag
			com->Boost_Icons(self, BOOSTICON_BLINK, ICON_POWERBOOST, 0); // start blinking icon
		}

// SCG[1/4/00]: 		if (hook->power_boost_time <= gstate->time)
		if( hook->power_boost_time <= 0 )
		{
			hook->power_boost_time = 0;	// SCG[1/11/00]: Make sure we don't go below zero.
			hook->items &= ~IT_POWERBOOST;
			hook->power_boost = hook->base_power;
			com->Boost_Icons(self, BOOSTICON_DELETE, ICON_POWERBOOST, 0); // remove skill icon
			recalc = TRUE;
		}
		else
			hook->power_boost_time -= 0.1;	// SCG[1/4/00]: 
	}

	if( hook->items & IT_ATTACKBOOST )
	{
// SCG[1/4/00]: 		if( ( hook->attack_boost_time <= ( gstate->time + BOOST_BLINKTIME ) ) && ( hook->exp_flags & EXP_ATTACKBOOST ) )
		if( ( hook->attack_boost_time <= BOOST_BLINKTIME ) && ( hook->exp_flags & EXP_ATTACKBOOST ) )
		{
			hook->exp_flags &= ~EXP_ATTACKBOOST;     // reset expiring flag
			com->Boost_Icons(self, BOOSTICON_BLINK, ICON_ATTACKBOOST, 0); // start blinking icon
		}

// SCG[1/4/00]: 		if( hook->attack_boost_time <= gstate->time )
		if( hook->attack_boost_time <= 0 )
		{
			hook->attack_boost_time = 0;	// SCG[1/11/00]: Make sure we don't go below zero.
			hook->items &= ~IT_ATTACKBOOST;
			hook->attack_boost = hook->base_attack;
		    com->Boost_Icons(self, BOOSTICON_DELETE, ICON_ATTACKBOOST, 0); // remove skill icon
			recalc = TRUE;
		}
		else
			hook->attack_boost_time -= 0.1;	// SCG[1/4/00]: 
	}

	if( hook->items & IT_SPEEDBOOST )
	{
// SCG[1/4/00]: 		if( ( hook->speed_boost_time <= ( gstate->time + BOOST_BLINKTIME ) ) && ( hook->exp_flags & EXP_SPEEDBOOST ) )
		if( ( hook->speed_boost_time <= BOOST_BLINKTIME ) && ( hook->exp_flags & EXP_SPEEDBOOST ) )
		{
			hook->exp_flags &= ~EXP_SPEEDBOOST;     // reset expiring flag
		    com->Boost_Icons(self, BOOSTICON_BLINK, ICON_SPEEDBOOST, 0); // start blinking icon
		}

// SCG[1/4/00]: 		if( hook->speed_boost_time <= gstate->time )
		if( hook->speed_boost_time <= 0 )
		{
			hook->speed_boost_time = 0;	// SCG[1/11/00]: Make sure we don't go below zero.
			hook->items &= ~IT_SPEEDBOOST;
			hook->speed_boost = hook->base_speed;
			com->Boost_Icons(self, BOOSTICON_DELETE, ICON_SPEEDBOOST, 0); // remove skill icon
			recalc = TRUE;
		}
		else
			hook->speed_boost_time -= 0.1;	// SCG[1/4/00]: 
	}

	if( hook->items & IT_ACROBOOST )
	{
// SCG[1/4/00]: 		if( ( hook->acro_boost_time <= ( gstate->time + BOOST_BLINKTIME ) ) && ( hook->exp_flags & EXP_ACROBOOST ) )
		if( ( hook->acro_boost_time <= BOOST_BLINKTIME ) && ( hook->exp_flags & EXP_ACROBOOST ) )
		{
			hook->exp_flags &= ~EXP_ACROBOOST;     // reset expiring flag
		    com->Boost_Icons(self, BOOSTICON_BLINK, ICON_ACROBOOST, 0); // start blinking icon
		}

// SCG[1/4/00]: 		if( hook->acro_boost_time <= gstate->time )
		if( hook->acro_boost_time <= 0 )
		{
			hook->acro_boost_time = 0;	// SCG[1/11/00]: Make sure we don't go below zero.
			hook->items &= ~IT_ACROBOOST;
			hook->acro_boost = hook->base_acro;
 		    com->Boost_Icons(self, BOOSTICON_DELETE, ICON_ACROBOOST, 0); // remove skill icon
			recalc = TRUE;
		}
		else
			hook->acro_boost_time -= 0.1;	// SCG[1/4/00]: 
	}

	if( hook->items & IT_VITABOOST )
	{
// SCG[1/4/00]: 		if( ( hook->vita_boost_time <= ( gstate->time + BOOST_BLINKTIME ) ) && ( hook->exp_flags & EXP_VITABOOST ) )
		if( ( hook->vita_boost_time <= BOOST_BLINKTIME ) && ( hook->exp_flags & EXP_VITABOOST ) )
		{
			hook->exp_flags &= ~EXP_VITABOOST;     // reset expiring flag
		    com->Boost_Icons(self, BOOSTICON_BLINK, ICON_VITABOOST, 0); // start blinking icon
		}

// SCG[1/4/00]: 		if( hook->vita_boost_time <= gstate->time )
		if( hook->vita_boost_time <= 0 )
		{
			hook->vita_boost_time = 0;	// SCG[1/11/00]: Make sure we don't go below zero.
			hook->items &= ~IT_VITABOOST;
			hook->vita_boost = hook->base_vita;
			com->Boost_Icons(self, BOOSTICON_DELETE, ICON_VITABOOST, 0); // remove skill icon
			recalc = TRUE;
		}
		else
			hook->vita_boost_time -= 0.1;	// SCG[1/4/00]: 
	}

	if ( recalc )
	{
	    com->CalcBoosts( self );
	}
}

///////////////////////////////////////////////////////////////////////////////
//	Client_PostThink
//	
//	called each frame before the physics are run
///////////////////////////////////////////////////////////////////////////////

void Client_PostThink (userEntity_t *self)
{
	playerHook_t	*hook;
	
	if(!self)
	{
		// this routine is fucked by garbage data, whine to the developer, bail out and try to move on
		gstate->Con_Dprintf ("Client_PostThink():%s,%i: bad parameter 1 == NULL\n", __FILE__, __LINE__);
		return; 
	}

	// Don't branch to camera think code if hook is screwed
	hook = (playerHook_t *)self->userHook;
	if(!hook)
	{
		gstate->Con_Dprintf ("Client_PostThink():%s,%i: self->userHook == NULL\n", __FILE__, __LINE__);
	} 
	else 
	{
		// camera posthink
		if (hook->camera && _stricmp(hook->camera->className, "func_monitor"))
		{
			cameraHook_t	*chook = (cameraHook_t *) hook->camera->userHook;
			// Fix ME: chook MIGHT be NULL
			if(chook)
			{
				if (chook->post_think)
					chook->post_think (hook->camera);
			} 
		}
	}

	if (self->flags & FL_INWARP)
		return;

	if (self->deadflag != DEAD_NO)
		return;

	Client_CheckPowerUps (self);

/// moved to friction hook
//	hook->last_water_type = self->watertype;
//	hook->last_water_level = self->waterlevel;
}


//=============================================================================
// void award_exps(userEntity_t *self,userEntity_t *other, int manual_bonus)
// 
// 
//=============================================================================
void award_exps(userEntity_t *self,userEntity_t *other, int manual_bonus)
// self-entity is awarded experience point bonus
{
	if (!(self->flags & FL_CLIENT))
		return;

	int exp_bonus;

	userEntity_t	*pSidekickMikiko, *pSidekickSuperfly;

	pSidekickMikiko = AIINFO_GetMikiko();
	pSidekickSuperfly = AIINFO_GetSuperfly();

	// WAW[11/18/99]: Only players and sidekicks are awarded exp.
	if ((self < &gstate->g_edicts[1]) || (self > &gstate->g_edicts[(int)(maxclients->value)]))
	{
		// OK are we a sidekick ?
//		if ((self != AIINFO_GetMikiko()) && (self != AIINFO_GetSuperfly()))
		// SCG[12/4/99]: Lets be efficient...
		if ((self != pSidekickMikiko) && (self != pSidekickSuperfly))
			return;
	}
		
	if (manual_bonus)             // force bonus?
		exp_bonus = manual_bonus;
	else if ((other >= &gstate->g_edicts[1]) && (other <= &gstate->g_edicts[(int)(maxclients->value)]) && !coop->value)	
	{
		// WAW[11/18/99]: Special experiance for killing a client.
		int level = calcStatLevel( other );
		exp_bonus = 100 * (level + 1); // For right now... Basicly that will be 300 for regular players average 4 kills to level up
	}
	else
	{
		// standard calculation for killing a monster ('other')
		playerHook_t*	hook = (playerHook_t*)other->userHook;
		
		exp_bonus = hook->base_health * 0.10 * (float)gstate->episode;  // 10% of opponent's base health
	}
	
	// cek[12-14-99] make sure we're supposed to be using the skill system in deathmatch!
	if (deathmatch->value)
	{
		cvar_t *dm_use_skill_system = gstate->cvar("dm_use_skill_system", "1", CVAR_ARCHIVE);
		if (dm_use_skill_system->value)
		{
			self->record.exp += exp_bonus;  // add exps
#pragma message("cek[7-18-00]: remove ----------------------------------------------------------------------------------------------------")
			gstate->Con_Dprintf("%d exp awarded to %s.  Total now %d.\n",exp_bonus,self->netname,self->record.exp);
			recalc_level(self);
		}
	}
	else
	{
		self->record.exp += exp_bonus;  // add exps
		recalc_level(self);
	}
	
	// give em some gibs in episode 3 for the nightmare...
	if (exp_bonus && (gstate->episode == 3) && (self->flags & FL_CLIENT) && self->inventory)
	{
		ammo_t *ammo = (ammo_t *) gstate->InventoryFindItem (self->goalentity->inventory, "ammo_gibs");
		if (ammo)
		{
			ammoGiveTake_t take;
			take.count = 1;
			take.owner = self;
			ammo->command ((userInventory_t *)ammo, "give_ammo", &take);
		}
	}
}




//=============================================================================
// int calcLevel( long exp )
// 
// Returns the experiance level based on the expenaced passed.
//=============================================================================
extern cvar_t	*dm_levellimit;

int calcLevel( long exp )
{
	int level;
	int maxlevel = (deathmatch->value ? (int)dm_levellimit->value:MAX_EXP_LEVELS);
	if (maxlevel <= 0)
		maxlevel = MAX_EXP_LEVELS;

	long *lv = &exp_level[1];
	for(level = 1;level < maxlevel;level++,lv++)
	{
		if (exp < *lv)
			return level-1;
	}
	return maxlevel;
}

//=============================================================================
// int calcStatLevel( userEntity_t *self )
// 
// Returns the level of a players stats.
//=============================================================================
int calcStatLevel( userEntity_t *self )
{
	if (!(self->flags & FL_CLIENT) || !self->userHook)
		return 0;

	playerHook_t*	hook = (playerHook_t*)self->userHook;

	return (hook->base_power + hook->base_attack + hook->base_speed +
			hook->base_acro + hook->base_vita);
}


//=============================================================================
// int fixStats( userEntity_t *self )
// 
// 
//=============================================================================
int fixStats( userEntity_t *self )
{
	if (!(self->flags & FL_CLIENT) || !self->userHook)
		return 0;

	playerHook_t*	hook = (playerHook_t*)self->userHook;

	int level = calcLevel( self->record.exp );
	int slevel = calcStatLevel( self );

	if (slevel > level)
	{
		int stats[ 5 ];
		stats[0] = hook->base_power;
		stats[1] = hook->base_attack;
		stats[2] = hook->base_speed;
		stats[3] = hook->base_acro;
		stats[4] = hook->base_vita;
		int i,h,v;
		do
		{
			h = 0;
			v = stats[0];
			for(i=1;i<5;i++)
			{
				if (stats[i] > v)
				{
					h = i;
					v = stats[i];
				}
			}
			if (v == 0)
				break;	// done.
			stats[h]--;
			slevel--;
		} while( slevel > level );
		hook->base_power	= stats[0];
		hook->base_attack	= stats[1];
		hook->base_speed	= stats[2];
		hook->base_acro		= stats[3];
		hook->base_vita		= stats[4];
		com->CalcBoosts (self);
	}
	return calcStatLevel( self );
}
		
//=============================================================================
// void CLIENT_XPLevel(userEntity_t *self)
// 
// 
//=============================================================================
void CLIENT_XPLevel(userEntity_t *self)
{
	// only send levelups if the client can go up a level
	if ( !(self->flags & FL_CLIENT) || (calcStatLevel( self ) >= (gstate->episode + 1) * 5) )
		return;

	if (!gstate->gi->TestUnicast(self,true))
	{
		self->client->needsUpdate |= HUD_UPDATE_XPLEVEL;
	}
	else
	{
		gstate->WriteByte(SVC_XPLEVEL);  //	send the 'level up' message to client
		gstate->UniCast(self,true);      // broadcast
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//  recalc_level
//
//  calculates and set the level of the player passed in
//
///////////////////////////////////////////////////////////////////////////////
void Inventory_SetMode( userEntity_t *self, int nMode, qboolean bPerformAction );
void recalc_level(userEntity_t* self)
{
	if (!(self->flags & FL_CLIENT))
		return;
	// WAW[11/18/99]: Just gave up and rewrote it!
	playerHook_t*	hook = (playerHook_t*)self->userHook;

	int level = calcLevel( self->record.exp );
	int slevel = calcStatLevel( self );

	if (slevel > level)
		slevel = fixStats( self );	// Cheating has occured some where... But just fix the damage.
	self->record.level = level;
	if (slevel < level)
	{
		Inventory_SetMode( self, INV_MODE_SKILLS, true );
		CLIENT_XPLevel(self); // Tell the client to do it again.
	}
}

//=============================================================================
// void isLevelUp(userEntity_t *self)
// 
// returns:
//			-1	if your experience level is less than your current level (cheating)
//			0	if things are just right
//			1	if your experience level is greater than your current level (levelup)
//=============================================================================
short isLevelUp(userEntity_t *self)
{
	if (!(self->flags & FL_CLIENT))
		return 0;

	int level = calcLevel( self->record.exp );
	int slevel = calcStatLevel( self );

	if ( slevel < level )
		return 1;
	else if ( slevel > level )
		return -1;
	else
		return 0;
}

//=============================================================================
// void xplevelup(userEntity_t *self)
// 
// 
//=============================================================================
void xplevelup(userEntity_t *self)
// command to apply point to a specific skill
{
	if (!(self->flags & FL_CLIENT) || !self->userHook)
		return;

	// WAW[11/18/99]: Put level cheating protection and various other things.
	int level = calcLevel( self->record.exp );
	int slevel = calcStatLevel( self );
	if (slevel > level)
		slevel = fixStats( self );	// Cheating has occured some where... But just fix the damage.

	if (slevel < level)
	{
		// OK we are fine to go.
		int skill = atoi(gstate->GetArgv(1));  // current skill selection

		// increase skill stat
		playerHook_t*	hook = (playerHook_t*)self->userHook;
		
		switch (skill)
		{
		case 0: hook->base_power  += 1.0f;  break;
		case 1:	hook->base_attack += 1.0f;	break;
		case 2:	hook->base_speed  += 1.0f;	break;
		case 3:	hook->base_acro   += 1.0f;	break;
		case 4:	hook->base_vita   += 1.0f;	break;
		}
		slevel = calcStatLevel( self );

		if (slevel < level)
			CLIENT_XPLevel(self); // Tell the client to do it again.
		
		com->CalcBoosts (self);
	}

	Inventory_SetMode( self, INV_MODE_SKILLS + 1, true );
}


///////////////////////////////////////////////////////////////////////////////
//
//	Client_PlayDeathTaunt
//	
///////////////////////////////////////////////////////////////////////////////
#define TAUNT_DELAY				(0.5)
void Client_PlayDeathTaunt( userEntity_t *self )
{
#ifndef TONGUE_ENGLISH
	return;
#endif
	userEntity_t	*pAttacker = gstate->attacker;
	char			*pFilename = NULL;
	int				nSoundIndex;

	// SCG[11/1/99]: Make sure the attacker is valid
	// cek[1-11-00]: no taunting if you kill yourself.
	if( pAttacker && (pAttacker != self) )
	{
		char *weapName = gstate->attackWeap;
		if (!weapName)
			return;

		if (strcmp(weapName, "weapon_daikatana") == 0)
		{
			pFilename = "global/dmsg_daikatana.wav";
		}
		else
		{
			switch(gstate->episode)
			{
			case 1:
				if( strcmp( weapName, "weapon_disruptor" ) == 0 )			pFilename = "e1/dmsg_1_1.wav";
				else if( strcmp( weapName, "weapon_gashands" ) == 0 )		pFilename = "e1/dmsg_1_7.wav";
				else if( strcmp( weapName, "weapon_ionblaster" ) == 0 )		pFilename = "e1/dmsg_1_2.wav";
				else if( strcmp( weapName, "weapon_c4" ) == 0 )				pFilename = "e1/dmsg_1_3.wav";
				else if( strcmp( weapName, "weapon_shotcycler" ) == 0 )		pFilename = "e1/dmsg_1_4.wav";
				else if( strcmp( weapName, "weapon_sidewinder" ) == 0 )		pFilename = "e1/dmsg_1_5.wav";
				else if( strcmp( weapName, "weapon_shockwave" ) == 0 )		pFilename = "e1/dmsg_1_6.wav";
				break;
			case 2:
				if( strcmp( weapName, "weapon_discus" ) == 0 )				pFilename = "e2/dmsg_2_1.wav";
				else if( strcmp( weapName, "weapon_venomous" ) == 0 )		pFilename = "e2/dmsg_2_2.wav";
				else if( strcmp( weapName, "weapon_sunflare" ) == 0 )		pFilename = "e2/dmsg_2_3.wav";
				else if( strcmp( weapName, "weapon_hammer" ) == 0 )			pFilename = "e2/dmsg_2_4.wav";
				else if( strcmp( weapName, "weapon_trident" ) == 0 )		pFilename = "e2/dmsg_2_5.wav";
				else if( strcmp( weapName, "weapon_zeus" ) == 0 )			pFilename = "e2/dmsg_2_6.wav";
				break;
			case 3:
				if( strcmp( weapName, "weapon_silverclaw" ) == 0 )			pFilename = "e3/dmsg_3_1.wav";
				else if( strcmp( weapName, "weapon_bolter" ) == 0 )			pFilename = "e3/dmsg_3_2.wav";
				else if( strcmp( weapName, "weapon_stavros" ) == 0 )		pFilename = "e3/dmsg_3_3.wav";
				else if( strcmp( weapName, "weapon_ballista" ) == 0 )		pFilename = "e3/dmsg_3_4.wav";
				else if( strcmp( weapName, "weapon_wyndrax" ) == 0 )		pFilename = "e3/dmsg_3_5.wav";
				else if( strcmp( weapName, "weapon_nightmare" ) == 0 )		pFilename = "e3/dmsg_3_6.wav";
				break;
			case 4:
				if( strcmp( weapName, "weapon_glock" ) == 0 )				pFilename = "e4/dmsg_4_1.wav";
				else if( strcmp( weapName, "weapon_slugger" ) == 0 )		pFilename = "e4/dmsg_4_2.wav";
				else if( strcmp( weapName, "weapon_kineticore" ) == 0 )		pFilename = "e4/dmsg_4_3.wav";
				else if( strcmp( weapName, "weapon_ripgun" ) == 0 )			pFilename = "e4/dmsg_4_4.wav";
				else if( strcmp( weapName, "weapon_novabeam" ) == 0 )		pFilename = "e4/dmsg_4_5.wav";
				else if( strcmp( weapName, "weapon_metamaser" ) == 0 )		pFilename = "e4/dmsg_4_6.wav";
				break;
			default:
				pFilename = NULL;
			}
		}

	// cek[1-25-00]: added for delayed deathmatch taunts
		if( pFilename != NULL )
		{
			playerHook_t *aHook = AI_GetPlayerHook( pAttacker );
			playerHook_t *tHook = AI_GetPlayerHook( self );

			nSoundIndex = gstate->SoundIndex( pFilename );

			bool doAttacker = atoi(Info_ValueForKey(pAttacker->client->pers.userinfo,"dm_taunts")) > 0;
			bool doSelf = atoi(Info_ValueForKey(self->client->pers.userinfo,"dm_taunts")) > 0;

			if (aHook && doAttacker)
			{
				aHook->ftauntTime = gstate->time + TAUNT_DELAY;
				aHook->ntauntIndex = nSoundIndex;
			}

			if (tHook && doSelf)
			{
				tHook->ftauntTime = gstate->time + TAUNT_DELAY;
				tHook->ntauntIndex = nSoundIndex;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	Client_DeathMessage
//
//	called from com->Damage when a player is killed
///////////////////////////////////////////////////////////////////////////////
void Client_DeathMessage (userEntity_t *self)
{
	playerHook_t	*ahook, *hook;
 

	// set up killer info for passing to weapon command
	killerInfo.attacker = gstate->attacker;
	killerInfo.victim = self;

	if( deathmatch->value > 0 )
	{
		// in deathmatch, just trust that the attacker is alright...make sure there is one though.
		if (!gstate->attacker)
			gstate->attacker = self;

		Client_PlayDeathTaunt( self );
	}
	else
	{
		if ( !AI_IsAlive(gstate->attacker) )
		{
			gstate->attacker = self;
		}
	}
	hook = AI_GetPlayerHook(self);
    ahook = AI_GetPlayerHook( gstate->attacker );

	if (gstate->attacker)
	{
		if (client_deathtype)
		{
			self->record.frags--;
			hook->died++;

			gstate->cs.BeginSendString();
			if (client_deathtype == 1)
				gstate->cs.SendSpecifierStr("%s%s",2);
			else
				gstate->cs.SendSpecifierStr("%s %s",2);

			gstate->cs.SendString(com->ProperNameOf(self));
			gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_deathmsg_self,client_deathtype - 1));
			gstate->cs.Unicast(self,CS_print_center, 2.0);
			gstate->cs.Multicast(CS_print_normal);
			gstate->cs.EndSendString();

			// play the telefrag sound...only in english!
#ifdef TONGUE_ENGLISH
			if (client_deathtype == TELEFRAG)
			{
				hook->ftauntTime = gstate->time + TAUNT_DELAY;
				hook->ntauntIndex = gstate->SoundIndex( "global/dmsg_telefragged.wav" );;
			}
#endif
		}
		// player killed himself
		else 
		if ( gstate->attacker == self && 
			 (gstate->attacker->flags & FL_CLIENT || gstate->attacker->flags & FL_BOT) )
		{
			// lose a frag, stooopid
			self->record.frags--;
			hook->died++;

			// lose some of your experience too dumbass
			//self->record.exp -= 0.10 * self->record.exp;
			//recalc_level(self);

			if ((gstate->attackWeap) && (gstate->attacker->inventory))
			{
				userInventory_t *inv = gstate->InventoryFindItem(gstate->attacker->inventory,gstate->attackWeap);
				if (inv)
					inv->command (inv, "suicide_msg", &killerInfo);
			}
/*
			if (ahook->weapon_fired)
			{
				if (ahook->weapon_fired->command)
					ahook->weapon_fired->command (ahook->weapon_fired, "suicide_msg", &killerInfo);
			}*/
		}
        // team mate killed player, deduct one frag
		//cek[1-8-00] friendly fire logic was backwards.  1 means you can hurt teammates.
        else if (com->TeamCheck(self, gstate->attacker) && (ctf->value || coop->value || (deathmatch->value || dm_friendly_fire->value))) 
        {
            gstate->attacker->record.frags--;
            hook->died++;

            //gstate->attacker->record.exp -= 0.20 * self->record.exp;
        }
		// player was killed by another player
		else 
		if ( gstate->attacker != self && 
			 (gstate->attacker->flags & FL_CLIENT || gstate->attacker->flags & FL_BOT) )
		{
			//gstate->attacker->record.exp += (0.10 * self->record.exp) + 10;
			gstate->attacker->record.frags++;
			
			hook->died++;
			ahook->killed++;

			// lose exp for dying
			//self->record.exp -= 0.10 * self->record.exp;
			//recalc_level(self);
			//recalc_level(gstate->attacker);

			if ((gstate->attackWeap) && (gstate->attacker->inventory))
			{
				userInventory_t *inv = gstate->InventoryFindItem(gstate->attacker->inventory,gstate->attackWeap);
				if (inv)
					inv->command (inv, "death_msg", &killerInfo);
			}

/*			ahook = AI_GetPlayerHook( gstate->attacker );
			if (ahook->weapon_fired)
			{
				if (ahook->weapon_fired->command)
					ahook->weapon_fired->command (ahook->weapon_fired, "death_msg", &killerInfo);
			}
*/
		}
		// player killed himself with his own c4
		else 
		if ( gstate->attacker->owner == self && gstate->attacker->owner && 
			 (gstate->attacker->owner->flags & FL_CLIENT || gstate->attacker->owner->flags & FL_BOT) )
		{
			// lose a frag, stooopid
			self->record.frags--;
			hook->died++;

			gstate->cs.BeginSendString();
			gstate->cs.SendSpecifierStr("%s %s",2);
			gstate->cs.SendString(com->ProperNameOf(self));
			gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_deathmsg_self,T_DEATHMSGSELF_C4));
			gstate->cs.Unicast(self,CS_print_center, 2.0);
			gstate->cs.Multicast(CS_print_normal);
			gstate->cs.EndSendString();

		}
		// player was killed by a player owned object  (ala c4)
		else 
		if ( gstate->attacker->owner != self && gstate->attacker->owner && 
			 (gstate->attacker->owner->flags & FL_CLIENT || gstate->attacker->owner->flags & FL_BOT) )
		{
			// reset ahook
			ahook = AI_GetPlayerHook( gstate->attacker->owner );
			
			//gstate->attacker->owner->record.exp += (0.10 * self->record.exp) + 10;
			gstate->attacker->owner->record.frags++;
			
			hook->died++;
			ahook->killed++;

			// lose exp for dying
			//self->record.exp -= 0.10 * self->record.exp;
			//recalc_level(self);
			//recalc_level(gstate->attacker->owner);
			
			char msg[80];
			Com_sprintf(msg, sizeof(msg),"%s was blown away by %s.\n", self->netname, gstate->attacker->owner->netname); // TTD: move to l_english.h
			gstate->bprint(msg);
		}
	}
}

void Client_RemoveWeapon(userEntity_t *self)
{
	if (!self || !self->client->ps.weapon)
		return;

	userEntity_t *went=(userEntity_t *)self->client->ps.weapon;
	RELIABLE_UNTRACK(went);
	went->s.sound = 0;
	went->think = NULL;
	went->s.modelindex2 = self->s.modelindex2 = 0;	// detatch it first!
	went->s.renderfx = RF_NODRAW;
	went->owner = NULL;
	gstate->RemoveEntity(went);
	//self->client->ps.weapon = NULL;
}

void camera_first_dead_sidekick(userEntity_t *self);
///////////////////////////////////////////////////////////////////////////////
//	Client_Die
//
//	called from com->Damage when a player is killed
///////////////////////////////////////////////////////////////////////////////

void Client_IceDie(userEntity_t *self)
{
	playerHook_t	*hook = (playerHook_t *)self->userHook;
	if (!hook)
		return;

	char *str;
	int num;
	int min;
	switch(hook->iPlayerClass)
	{
	case PLAYERCLASS_HIRO:
		num = 5;
		min = 1;
		str = "icedeath%d.wav";
		break;
	case PLAYERCLASS_MIKIKO:
		num = 6;
		min = 2;
		str = "ice%d.wav";
		break;
	case PLAYERCLASS_SUPERFLY:
		num = 4;
		min = 1;
		str = "ice%d.wav";
		break;
	default:
		return;
	};

	num = min + rand() % num;
	char sound[32];
	Com_sprintf(sound,sizeof(sound),str,num);
	PlayClientSound(self, sound);
}

void Client_Die (userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point)
{
	playerHook_t	*hook = (playerHook_t *)self->userHook;
	userEntity_t	*pCameraTarget = NULL;
	frameData_t		*pSequence;
	CVector        z;
	byte           gibbed = false;

	z.Zero();
   
	// cek[11-24-99] untrack trackents attached to the players current weapon and to the player
	if (self)
	{
		if (self->client)
		{			
			Client_RemoveWeapon(self);
		}
		self->s.sound = 0;		// cek[12-2-99] stop looped sounds when player dies!
		RELIABLE_UNTRACK(self);
	}
	
	//NSS[11/30/99]:We don't deserve health... we let our pals die!
	if(self->deadflag == DEAD_SIDEKICK && self->health > 0.0f)
	{
		self->health	= 0;
		self->delay		= gstate->time + 1.0f;  //NSS[11/30/99]:Setup a small delay so we can see our buddy dead...then go watch our death.
		self->hacks		= 0.0f;					//Reset our timer.
		self->solid		= SOLID_BBOX;			// make the player dead but solid as a rock
		self->movetype	= MOVETYPE_PUSH;		// get pushy since we're frozen
		self->deadflag	= DEAD_FROZEN;			// Set our flags
		self->flags		|= FL_FREEZE;			// 
		camera_first_dead_sidekick(self);		// Go find the sidekick that died
		self->hacks = gstate->time + 10.0f;		// make sure we have a nice delay before hunting down something else
		if(self->deadflag != DEAD_PUNK)
			self->deadflag = DEAD_DEAD;
		gstate->LinkEntity(self);		
		return;
	}
	
	//	drop weapon and print death message
	if (self->deadflag == DEAD_NO || self->deadflag == DEAD_SIDEKICK)
	{
		// cek[1-4-00]: unwraith the dead player
		if (hook)
			hook->items &= ~IT_WRAITHORB;

    	self->s.renderfx &= ~RF_TRANSLUCENT;
    	self->s.renderfx &= ~RF_PREDATOR;

		// SCG[11/16/99]: Here's an idea! Let's not check this unless we're
		// SCG[11/16/99]: actually in coop mode!!!
		// WAW[11/16/99]: OK But got to check the ctf and not the coop :-)
		if( ctf->value )
			CTFClient_Die(self,inflictor,attacker,damage,point);
		else if (deathtag->value)
			DT_Client_Die(self,inflictor,attacker,damage,point);

		Client_DeathMessage (self);  // show death message
		Client_DropInventory (self); //	free inventory
		// WAW[11/18/99]: Give the attacker experiance, but don't do it if they killed themselfs!
		if (deathmatch->value && (attacker != self))
		{
			// award player exp if there is no inflictor or if the inflictor is not the daikatana
			if (!inflictor || (inflictor && stricmp(inflictor->className,"weapon_daikatana")))
				award_exps(attacker,self,0);
		}
	}
	
	//NSS[11/6/99]: Lower the Z axis and widden the yx axis
	AI_SetDeathBoundingBox(self);

	// NSS[2/12/00]:Make user non-clipable...
	self->svflags |= SVF_DEADMONSTER;

	//Have to call this after resetting the bounding box
	gstate->LinkEntity(self);
	// lock the angles of the player so he can't rotate or spin
	self->s.angles.x = 0.0f;
	self->s.angles.z = 0.0f;

	// if we're displaying bounding boxes and the client dies, display it for 10 secs
	if (gstate->GetCvar ("p_showboxes") != 0)
	{
		com->DrawBoundingBox( self, gstate->time + 10.0 );
		// turn off the bounding box stuff
		gstate->SetCvar("p_showboxes","0");
	}

	//	delay before a player can respawn
	hook->jump_time = gstate->time + 0.5;

	if (self->deadflag == DEAD_NO || self->deadflag == DEAD_SIDEKICK )
	{
		//	player was just killed
		if (gstate->damage_flags & DAMAGE_COLD)
		{
			Client_IceDie(self);
		}
		else if (self->waterlevel > 2)
		{
			int sound_index = rand() % 2;
			sound_index += gstate->SoundIndex( "hiro/waterland4.wav" );

			gstate->StartEntitySound( self, CHAN_AUTO, sound_index, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );

			//            PlayClientSound(self, "waterdeath.wav");
		}
		// gstate->sound (self, CHAN_VOICE, "player/playerwaterdeath.wav", 255, ATTN_NORM);
		else if (self->health < -40)
		{
			PlayClientSound(self, "udeath.wav");
		}
		else
		{
			switch (rand() % 4)
			{
			case 0 : PlayClientSound(self, "death1.wav"); break;
			case 1 : PlayClientSound(self, "death2.wav"); break;
			case 2 : PlayClientSound(self, "death3.wav"); break;
			case 3 : PlayClientSound(self, "death4.wav"); break;
			}
		}

		//If we are in violence mode start the camera... we could end up in gib_cam mode.
		if (!hook->camera && sv_violence->value == 0)
		{
			camera_start (self, false);
		}
		else
		{
			camera_start (self, false);
			camera_mode (self, CAMERAMODE_BESTCHASE, false);
		}

		if (deathmatch->value || coop->value)
		{
			// show scoreboard here
			self->client->showflags |= (SHOW_SCORES | SHOW_SCORES_NOW);	
		}

		if ( !( self->flags & FL_FREEZE ) && check_for_gib(self,damage) )
		{

			// dismember this dead bastard if violence level is set
			if(sv_violence->value == 0)
			{
				//	make sure we're not drawing the player model
				self->s.renderfx |= RF_NODRAW;

				// NSS[2/13/00]:Testing
				pCameraTarget = self;
					
				gib_client_die(self, inflictor);

				self->solid         = SOLID_NOT; //	do not clip any longer!
				self->groundEntity  = NULL;
				self->movetype      = MOVETYPE_BOUNCE;
				self->svflags      |= SVF_DEADMONSTER;
				self->armor_val     = 0;
				self->armor_abs     = 0;
				self->s.modelindex = 0;
				gibbed = true;
			}
			else
			{
				pSequence = FRAMES_GetSequence( self, "diea" );
				AI_ForceSequence(self, pSequence);
			}
			self->deadflag = DEAD_PUNK;	
		}
		else
		{
			// set up frozen state
			if ( self->flags & FL_FREEZE )
			{ 
				AI_StopCurrentSequence( self ); // stop animation dead

				self->solid = SOLID_BBOX; // make the player dead but solid as a rock
				self->movetype = MOVETYPE_PUSH; // get pushy since we're frozen
			}
			else
			{
				frameData_t *pSequence = FRAMES_GetSequence( self, "diea" );
				AI_StartSequence(self, pSequence);
			}

			pCameraTarget = self;

			self->deadflag = DEAD_DYING;
		}
		if(pCameraTarget && sv_violence->value == 0)
		{
			camera_set_target (hook->camera, pCameraTarget);
			camera_mode (self, CAMERAMODE_GIBCAM, false);
		}

		if (!deathmatch->value && !coop->value)
		{
			//	don't allow restart for a few seconds
			self->delay = gstate->time + 4.0;
		}
		else
		{
			//	don't allow restart for a bit
	
			
			self->delay = gstate->time + 2.0;
			self->hacks = gstate->time + 8.0;
		}
		self->solid         = SOLID_NOT; //	do not clip any longer!
		self->client->latched_buttons = 0;
	}
	else if (self->deadflag == DEAD_DYING)
	{
		if ((sv_violence->value == 0) && !( self->flags & FL_FREEZE ))
		{
			// dismember this dead bastard
			pCameraTarget = gib_client_die(self, attacker);

			if(pCameraTarget)
			{
				camera_set_target (hook->camera, pCameraTarget);
				camera_mode (self, CAMERAMODE_GIBCAM, false);
			}
			gibbed = true;

			self->solid         = SOLID_NOT; //	do not clip any longer!
			self->groundEntity  = NULL;
			self->movetype      = MOVETYPE_BOUNCE;
			self->svflags      |= SVF_DEADMONSTER;
			self->armor_val     = 0;
			self->armor_abs     = 0;
			self->s.modelindex = 0;
			self->deadflag = DEAD_PUNK;
		}
		else
		{
			if ( !(self->flags & FL_FREEZE) )
			{ 
				frameData_t *pSequence = FRAMES_GetSequence( self, "diea" );
				AI_StartSequence(self, pSequence);

				self->deadflag = DEAD_PUNK;
			}
		}
	}
	else if (self->deadflag == DEAD_FROZEN)
	{
		self->solid = SOLID_BBOX; // frozen solid
		self->movetype = MOVETYPE_PUSH; // push technology
	}
	else if( (self->deadflag != DEAD_PUNK) && damage  )
	{
		self->solid         = SOLID_NOT; //	do not clip any longer!
		self->groundEntity  = NULL;
		self->movetype      = MOVETYPE_BOUNCE;
		self->svflags      |= SVF_DEADMONSTER;
		self->armor_val     = 0;
		self->armor_abs     = 0;		

		if(sv_violence->value == 0 && !( self->flags & FL_FREEZE ))
		{
			gib_client_die(self, attacker);
			self->s.modelindex = 0;
			gibbed = true;
		}
		self->deadflag = DEAD_PUNK;
		return;
	}

	com->InformClientDeath(self, gibbed);
}

///////////////////////////////////////////////////////////////////////////////
//	Client_Pain
//
//	initializes the client data structures
///////////////////////////////////////////////////////////////////////////////
void Client_Pain( userEntity_t *self, userEntity_t *other, float kick, int damage )
{
	playerHook_t *hook = (playerHook_t *)self->userHook;
	char buffer[32];
	char final_sound_name[32];
	char modelname[64]={"\0"};
	int Max = 10;
	int r;

	

	if(other->flags & FL_BOT)
	{
		AI_Dprintf("Sidekicks are busting the lunch out of you!\n");
	}


	if ( hook->pain_finished > gstate->time )
	{
		return;
	}
	
	if(damage > 35.0f)
		Com_sprintf(buffer,sizeof(buffer),"death");
	else
		Com_sprintf(buffer,sizeof(buffer),"pain");	
    
	if(self->deadflag == DEAD_NO)
    {
		if ((hook->dflags & DFL_LLAMA) || (gstate->damage_inflicted <= 0))
		{
			float volume = (damage * 0.01);
			if (volume < 0.25) volume = 0.25;
			else if (volume > 1) volume = 1;

			hook->pain_finished = gstate->time + 1.0;
			if (hook->items & IT_MANASKULL)
			{
				gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("artifacts/manaskullpickup.wav"), volume, ATTN_NORM_MIN, ATTN_NORM_MAX );
			}
			return;
		}

		// do special things for lava and slime
		if (gstate->damage_flags & DAMAGE_LAVA)
		{
			//NSS[11/29/99]:Bloody scream while in lava
			if (hook->sound_time <= gstate->time)
			{
				hook->sound_time = gstate->time + 3.0;
				if (hook->iPlayerClass == PLAYERCLASS_MIKIKO)
				{
					gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("mikiko/death8.wav"), 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );
				}
				else if (hook->iPlayerClass == PLAYERCLASS_SUPERFLY)
				{
					gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("superfly/death7.wav"), 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );
				}
				else
				{
					gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("hiro/death8.wav"), 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );
				}
			}
		}
		else if (gstate->damage_flags & DAMAGE_SLIME)
		{
			//NSS[11/29/99]:Bloody scream while in lava
			if (hook->sound_time <= gstate->time)
			{
				hook->sound_time = gstate->time + 1.5;
				if (hook->iPlayerClass == PLAYERCLASS_MIKIKO)
				{
					gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("mikiko/death7.wav"), 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );
				}
				else if (hook->iPlayerClass == PLAYERCLASS_SUPERFLY)
				{
					gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("superfly/death3.wav"), 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );
				}
				else
				{
					gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("hiro/death3.wav"), 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );
				}
			}
		}
		else
		{
			//NSS[10/25/99]:This will now randomly select sounds for the cocks.  This is also a new optimized function.

			if (deathmatch->value || coop->value)
				strncpy(modelname, self->client->pers.body_info.modelname, 64);
			else
				strncpy(modelname, Info_ValueForKey(self->client->pers.userinfo, "modelname"), 64);
			
			if(strstr(modelname, "mikiko"))
					Max = 8;
			else if (strstr(modelname, "superfly"))
					Max = 7;

			
			//NSS[11/7/99]:Added check for kickview enable
  			if(self->client && gstate->GetCvar("kickview") && !(hook->dflags & DFL_LLAMA))
			{
				//NSS[10/25/99]:Kickview from damage applied to client is done here.
				//This will make the player's view bust away from the direction of impacting damage.
				CVector Target_Angles,Forward;
				CVector vKick = other->s.origin - self->s.origin;
				vKick.Normalize();
				vKick.Negate();
				VectorToAngles(vKick,Target_Angles);
				Target_Angles = self->client->v_angle - Target_Angles;
				Target_Angles.AngleToForwardVector(Forward);
				if(damage > 100.0f)
					damage = 100.0f;
				float Damage_View = (damage * 0.01);
				Forward.Multiply( ( damage * Damage_View ) );
				Forward.yaw = Forward.yaw *-1;
				com->KickView( self, Forward, 0, damage,450.0f);
			}

			r = (int)(floor(rnd () * Max) + 1);
			Com_sprintf(final_sound_name,sizeof(final_sound_name),"%s%d.wav",buffer,r);

			//NSS[11/13/99]:Only play the sounds if we are not in LLAMA mode
			if( self->waterlevel < 3 && !(hook->dflags & DFL_LLAMA))
			{
				PlayClientSound(self, final_sound_name);
			}
		}

		hook->pain_finished = gstate->time + 1.0;
    }
	else if( (damage > 0.0f && self->delay < gstate->time)|| self->deadflag == DEAD_DEAD || self->deadflag == DEAD_DYING)
	{
		Client_Die (self,other,other,damage, self->s.origin);
	}

}

///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////

void	give_weapons (userEntity_t *self)
{
// only show ECTS weapons
//#define __ECTS__

//	int		episode;// SCG[1/23/00]: not used
	//	Q2FIXME:	world doesn't have epairs set up right or something
//	return;

//	episode = gstate->GetCvar ("sv_episode");// SCG[1/23/00]: not used

// SCG[9/28/99]: removed per Romero's request
/*
	if (episode == 1)
	{
#ifndef __ECTS__
		gstate->sprint (self->owner, "Giving Disruptor Rod.\n");
		gstate->CBuf_AddText ("weapon_disruptor\n");
#endif

		gstate->sprint (self->owner, "Giving ION Blaster.\n");
		gstate->CBuf_AddText ("weapon_ionblaster\n");

#ifndef __ECTS__
		gstate->sprint (self->owner, "Giving C4 Vizatergo.\n");
		gstate->CBuf_AddText ("weapon_c4\n");

		gstate->sprint (self->owner, "Giving Shotcycler.\n");
		gstate->CBuf_AddText ("weapon_shotcycler\n");
#endif

		gstate->sprint (self->owner, "Giving Sidewinder.\n");
		gstate->CBuf_AddText ("weapon_sidewinder\n");

		gstate->sprint (self->owner, "Giving Shockwave.\n");
		gstate->CBuf_AddText ("weapon_shockwave\n");

		gstate->CBuf_AddText ("select_ionblaster\n");
	}
	else if (episode == 2)
	{
#ifndef __ECTS__
		gstate->sprint (self->owner, "Giving Discus of Daedalus.\n");
		gstate->CBuf_AddText ("weapon_discus\n");
#endif

		gstate->sprint (self->owner, "Giving Sunflare.\n");
		gstate->CBuf_AddText ("weapon_sunflare\n");

#ifndef __ECTS__
		gstate->sprint (self->owner, "Giving Hammer.\n");
		gstate->CBuf_AddText ("weapon_hammer\n");
#endif

		gstate->sprint (self->owner, "Giving Venomous.\n");
		gstate->CBuf_AddText ("weapon_venomous\n");

		gstate->sprint (self->owner, "Giving Poseidon's Trident.\n");
		gstate->CBuf_AddText ("weapon_trident\n");

#ifndef __ECTS__
		gstate->sprint (self->owner, "Giving Hand Of Zeus.\n");
		gstate->CBuf_AddText ("weapon_zeus\n");

		gstate->sprint (self->owner, "Giving Midas' Golden Grasp.\n");
		gstate->CBuf_AddText ("weapon_midas\n");
#endif

		gstate->CBuf_AddText ("select_discus\n");
	}
	else if (episode == 3)
	{
#ifndef __ECTS__
		gstate->sprint (self->owner, "Giving Silver Claw.\n");
		gstate->CBuf_AddText ("weapon_silverclaw\n");
#endif

		gstate->sprint (self->owner, "Giving Bolter.\n");
		gstate->CBuf_AddText ("weapon_bolter\n");

#ifndef __ECTS__
		gstate->sprint (self->owner, "Giving Stavros' Stave.\n");
		gstate->CBuf_AddText ("weapon_stavros\n");

		gstate->sprint (self->owner, "Giving Ballista.\n");
		gstate->CBuf_AddText ("weapon_ballista\n");
#endif

		gstate->sprint (self->owner, "Giving Wyndrax Wisp.\n");
		gstate->CBuf_AddText ("weapon_wyndrax\n");

		gstate->sprint (self->owner, "Giving Nharre's Nightmare.\n");
		gstate->CBuf_AddText ("weapon_nightmare\n");

		gstate->CBuf_AddText ("select_bolter\n");
	}
	else if (episode == 4)
	{
		gstate->sprint (self->owner, "Giving Glock.\n");
		gstate->CBuf_AddText ("weapon_glock\n");

#ifndef __ECTS__
		gstate->sprint (self->owner, "Giving Ripgun.\n");
		gstate->CBuf_AddText ("weapon_ripgun\n");
#endif

		gstate->sprint (self->owner, "Giving Slugger.\n");
		gstate->CBuf_AddText ("weapon_slugger\n");

#ifndef __ECTS__

		gstate->sprint (self->owner, "Giving Kineticore.\n");
		gstate->CBuf_AddText ("weapon_kcore\n");
#endif

		gstate->sprint (self->owner, "Giving Novabeam.\n");
		gstate->CBuf_AddText ("weapon_novabeam\n");

#ifndef __ECTS__
		gstate->sprint (self->owner, "Giving Metamaser.\n");
		gstate->CBuf_AddText ("weapon_metamaser\n");
#endif

		gstate->CBuf_AddText ("select_glock\n");
	}

#ifndef __ECTS__
	gstate->sprint (self->owner, "Giving Flashlight.\n");
	gstate->CBuf_AddText ("weapon_flashlight\n");
#endif

#ifdef	__ECTS__
	//	start invulnerable
	playerHook_t *hook = AI_GetPlayerHook( self->owner );
	hook->dflags |= DFL_LLAMA;
#endif
*/
	self->remove (self);
}

///////////////////////////////////////////////////////////////////////////////
//	dll_ClientUserInfoChanged
//
//	called whenever the player updates a userinfo variable.
//
//	The game can override any of the settings in place
//	(forcing skins or names, etc) before copying it off.
///////////////////////////////////////////////////////////////////////////////

void	dll_ClientUserinfoChanged(edict_t *ent, char *userinfo)
{
	char	*s;
	char    *m;
	int		c;
	int		playernum;

	if (!ent->client)
		return;

    playernum = ent - gstate->g_edicts - 1; // get player number

	s = Info_ValueForKey (userinfo, "name");  // set name

	if (ent->netname)
	{
		gstate->X_Free(ent->netname);
		ent->netname = NULL;
	}

	if( !(s == NULL || s[0] == NULL ) )
	{
		StripInvalidChars(s,16);  // insure valid name

		//	MEM_MALLOC is okay because before it gets freed at level end,
		//	it gets copied over to client_persistDAta [x].netName
		ent->netname = (char *)gstate->X_Malloc(16, MEM_TAG_COM_MISC);
		strncpy (ent->netname, s, 16);
		ent->netname [15] = 0x00;
		strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);
	}

	
	if (deathmatch->value)
	{
		if (ctf->value || deathtag->value || dm_teamplay->value)
		{
			if (dm_teamplay->value)
			{
				TEAM_Verify(ent,"team",userinfo);
			}
			TEAM_SetSkin(ent, userinfo);
			// save off the userinfo in case we want to check something later
			strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);
			return;
		}

		// in dm, set and never changed
		c = ent->client->pers.body_info.character;
		m = ent->client->pers.body_info.modelname;
		// changed by setskin...
		s = ent->client->pers.body_info.skinname;
	}
	else
	{
		if (coop->value)
		{
			CoopSelectCharacter( ent, (char *)userinfo );
			strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);
			return;
		}
		else
		{
			Info_SetValueForKey(userinfo, "character", "0"); 
			Info_SetValueForKey(userinfo, "modelname", DEFAULT_CHARACTER_MODEL); 
			Info_SetValueForKey(userinfo, "skinname",  DEFAULT_HIRO_SKIN);
			Info_SetValueForKey(userinfo, "skincolor", "0");
		}

		c = atoi(Info_ValueForKey (userinfo, "character"));  // get character
		s = Info_ValueForKey      (userinfo, "skinname");    // get skin
		m = Info_ValueForKey      (userinfo, "modelname");   // get model  3.9 dsn
	}

	// save off the userinfo in case we want to check something later
	strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);

	// send the configstring
	gstate->ConfigString (CS_PLAYERSKINS + playernum, va("%s\\%s\\%s\\%d", ent->netname, m, s, c) );
    SetTeam(ent);
}

///////////////////////////////////////////////////////////////////////////////
//	PlayIntroCinematic
//
//	parses the worldspawn epairs and looks for the cinematic_intro key
///////////////////////////////////////////////////////////////////////////////

void CIN_RemoveAllEntities();
void PlayIntroCinematic() 
{
    //get the worldspawn entity.
    if (gstate == NULL) return;
    userEntity_t *worldspawn = gstate->FirstEntity();
    if (worldspawn == NULL || worldspawn->epair == NULL ) return;

	if (sv_demomode->value != 0)
		return;

	// SCG[9/30/99]: We're doing a changelevel, so don't play the intro.
	// SCG[2/15/00]: Added check for loadgame.
	if( gstate->nLevelTransitionType == 1 )
	{
		return;
	}

    //get loop through all the epairs.
    for (int i = 0; worldspawn->epair[i].key != NULL; i++) {
        //get the epair.
        userEpair_t *epair = &worldspawn->epair[i];
        if (epair->key == NULL) return;

        //check if this is the key we want.
        if (stricmp(epair->key, "cinematic_intro") == 0) 
		{
			//get the value.
			const char *intro_name = epair->value;
			if (intro_name == NULL) return;
			if( ( sv_cinematics->value != 0 ) && ( coop->value == 0 ) && ( deathmatch->value == 0 ) && ( gstate->nLevelTransitionType != 3 ) )
			{
				//make the command to stuff into the buffer.
				buffer256 command("cin_load_and_play %s\n", intro_name);

				gstate->CBuf_AddText(command);
			}
			else
			{
				strncpy( gstate->szCinematicName, intro_name, 32 );
				CIN_RemoveAllEntities();
			}
        }
    }
}

#define	SIDEKICK_PERS_MIKIKO	2
#define	SIDEKICK_PERS_SUPERFLY	3

invenUse_t artifact_inv_use(char *name);
invenCommand_t artifact_inv_command(char *name);
void AddInventoryItem( userEntity_t *pEnt, char *pClassName, char *pModelFileName, unsigned long flags = 0 )
{
	int nModelIndex = gstate->ModelIndex( pModelFileName );

	userInventory_t	*pInvItem = gstate->InventoryCreateItem( pEnt->inventory,
		pClassName,
		artifact_inv_use(pClassName),
		artifact_inv_command(pClassName),
		nModelIndex,
		( ITF_SPECIAL | ITF_INVSACK | flags),
		sizeof( userInventory_t ) );

	gstate->InventoryAddItem( pEnt, pEnt->inventory, pInvItem ); 
}

void AddEpisode1Special( userEntity_t *pEnt, client_persistant_s *pPersistant )
{
//	if( pPersistant->nInventoryFlags & ITEM_KEYCARD_CELL )	AddInventoryItem( pEnt, "item_keycard_cell", "models/e1/a1_clcrd.dkm" );
}

void AddEpisode2Special( userEntity_t *pEnt, client_persistant_s *pPersistant )
{
	if( pPersistant->nInventoryFlags & ITEM_ANTIDOTE )	AddInventoryItem( pEnt, "item_antidote", "models/global/a_antidote.dkm" );
	if( pPersistant->nInventoryFlags & ITEM_DRACHMA )	AddInventoryItem( pEnt, "item_drachma", "models/e2/a2_drachma.dkm" , ITF_USEONCE | ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_HORN )		AddInventoryItem( pEnt, "item_horn", "models/e2/a2_horn.dkm" , ITF_USEONCE | ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_RUNE_A )	AddInventoryItem( pEnt, "item_rune_a", "models/e2/c_runea.dkm" , ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_RUNE_E )	AddInventoryItem( pEnt, "item_rune_e", "models/e2/c_runee.dkm" , ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_RUNE_G )	AddInventoryItem( pEnt, "item_rune_g", "models/e2/c_runeg.dkm" , ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_RUNE_I )	AddInventoryItem( pEnt, "item_rune_i", "models/e2/c_runei.dkm" , ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_RUNE_S )	AddInventoryItem( pEnt, "item_rune_s", "models/e2/c_runes.dkm" , ITF_COOP_REMOVEALL);
}

void AddEpisode3Special( userEntity_t *pEnt, client_persistant_s *pPersistant )
{
	if( pPersistant->nInventoryFlags & ITEM_WYNDRAX_KEY )		AddInventoryItem( pEnt, "item_wyndrax_key", "models/e3/a3_ltkey.dkm" , ITF_USEONCE | ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_SPELLBOOK )			AddInventoryItem( pEnt, "item_spellbook", "models/e3/a3_bookw.dkm" , ITF_USEONCE | ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_CRYPT_KEY )			AddInventoryItem( pEnt, "item_crypt_key", "models/e3/a3_crkey.dkm" , ITF_USEONCE | ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_PURIFIER_SHARD1 )	AddInventoryItem( pEnt, "item_purifier_shard1", "models/e3/purifier_1.dkm" , ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_PURIFIER_SHARD2 )	AddInventoryItem( pEnt, "item_purifier_shard2", "models/e3/purifier_2.dkm" , ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_PURIFIER_SHARD3 )	AddInventoryItem( pEnt, "item_purifier_shard3", "models/e3/purifier_3.dkm" , ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_HEX_KEYSTONE )		AddInventoryItem( pEnt, "item_hex_keystone", "models/e3/a_hex.dkm" , ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_QUAD_KEYSTONE )		AddInventoryItem( pEnt, "item_quad_keystone", "models/e3/a_quad.dkm" , ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_TRIGON_KEYSTONE )	AddInventoryItem( pEnt, "item_trigon_keystone", "models/e3/a_tri.dkm" , ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_RING_OF_FIRE )		AddInventoryItem( pEnt, "item_ring_of_fire", "models/e3/a_ring_fire.dkm" );
	if( pPersistant->nInventoryFlags & ITEM_RING_OF_UNDEAD )	AddInventoryItem( pEnt, "item_ring_of_undead", "models/e3/a_ringund.dkm" );
	if( pPersistant->nInventoryFlags & ITEM_RING_OF_LIGHTNING )	AddInventoryItem( pEnt, "item_ring_of_lightning", "models/e3/a_ringlig.dkm" );
}

void AddEpisode4Special( userEntity_t *pEnt, client_persistant_s *pPersistant )
{
	if( pPersistant->nInventoryFlags & ITEM_ENVIROSUIT )			AddInventoryItem( pEnt, "item_envirosuit", "models/e4/a_envsuit.dkm" );
	if( pPersistant->nInventoryFlags & ITEM_CONTROL_CARD_YELLOW )	AddInventoryItem( pEnt, "item_control_card_yellow", "models/e4/a4_clcyl.dkm" , ITF_USEONCE |ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_CONTROL_CARD_GREEN )	AddInventoryItem( pEnt, "item_control_card_green", "models/e4/a4_clcgr.dkm" , ITF_USEONCE |ITF_COOP_REMOVEALL);

	if( pPersistant->nInventoryFlags & ITEM_SULPHUR )	AddInventoryItem( pEnt, "item_sulphur", "models/e4/a_sulphur.dkm" , ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_CHARCOAL )	AddInventoryItem( pEnt, "item_charcoal", "models/e4/a_charcoal.dkm" , ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_SALTPETER )	AddInventoryItem( pEnt, "item_saltpeter", "models/e4/a_saltp.dkm" , ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_BOTTLE )	AddInventoryItem( pEnt, "item_bottle", "models/e4/a_bottle.dkm" , ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_BOMB )		AddInventoryItem( pEnt, "item_bomb", "models/e4/a_bomb.dkm" , ITF_USEONCE |ITF_COOP_REMOVEALL);
}

void FetchSpecialData( userEntity_t *pEnt, client_persistant_s	*pPersistant )
{
//	if( pPersistant->nInventoryFlags & ITEM_MEGASHIELD )			AddInventoryItem( pEnt, "item_megashield", "models/global/a_mshield.dkm" );
//	if( pPersistant->nInventoryFlags & ITEM_INVINCIBILITY )			AddInventoryItem( pEnt, "item_invincibility", "models/global/a_invincibility.dkm" );
	if( pPersistant->nInventoryFlags & ITEM_CONTROL_CARD_RED )		AddInventoryItem( pEnt, "item_control_card_red", "models/e4/a4_clcbl.dkm" , ITF_USEONCE |ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_CONTROL_CARD_BLUE )		AddInventoryItem( pEnt, "item_control_card_blue", "models/e4/a4_clcbl.dkm" , ITF_USEONCE |ITF_COOP_REMOVEALL);
	if( pPersistant->nInventoryFlags & ITEM_SAVEGEM_1 )				AddInventoryItem( pEnt, "item_savegem", "models/global/a_savegem.dkm" );
	if( pPersistant->nInventoryFlags & ITEM_SAVEGEM_2 )				AddInventoryItem( pEnt, "item_savegem", "models/global/a_savegem.dkm" );
	if( pPersistant->nInventoryFlags & ITEM_SAVEGEM_3 )				AddInventoryItem( pEnt, "item_savegem", "models/global/a_savegem.dkm" );

	switch( gstate->episode )
	{
	case 1:
		AddEpisode1Special( pEnt, pPersistant );
		break;
	case 2:
		AddEpisode2Special( pEnt, pPersistant );
		break;
	case 3:
		AddEpisode3Special( pEnt, pPersistant );
		break;
	case 4:
		AddEpisode4Special( pEnt, pPersistant );
		break;
	}
}

typedef struct
{
    weapon_t    weapon;

    int     mode;
}weaponPulse_t;
void DoAmmoInit( userEntity_t *pEnt, int nIndex, int count, char *szClassName )
{
	ammoGiveTake_t give;
	give.count = count;
	give.owner = pEnt;

	weapon_t		*weap = (weapon_t *)gstate->InventoryFindItem(pEnt->inventory, szClassName);
	if (weap)
	{
		if ( (gstate->episode == 4) && !stricmp(szClassName,"weapon_slugger") )
		{
			// get the ammo from the inventory
			ammo_t *ammoc = (ammo_t *)gstate->InventoryFindItem (pEnt->inventory, "ammo_cordite");
			ammo_t *ammos = (ammo_t *)gstate->InventoryFindItem (pEnt->inventory, "ammo_slugger");

			if (ammoc)
			{
				give.count = (count >> 16) & 0X00007FFF;
				ammoc->command((userInventory_t *)ammoc, "give_ammo", &give);			
			}

			if (ammos)
			{
				give.count = count & 0X0000FFFF;
				ammos->command((userInventory_t *)ammos, "give_ammo", &give);			
			}
		}
		else if (weap->ammo)
		{
			weap->ammo->command((userInventory_t *)weap->ammo, "give_ammo", &give);
		}
	}
	else
	{
		weaponInfo_t *pWeaponInfo = com->FindRegisteredWeapon( szClassName );
		if (pWeaponInfo)
		{
			weap = (weapon_t *) pWeaponInfo->give_func(pEnt,count);
			gstate->InventoryDeleteItem(pEnt,pEnt->inventory,(userInventory_t *)weap);
		}
	}
}

void DoWeaponInit( userEntity_t *pEnt, client_persistant_s *pPersistant, int nIndex, unsigned int nFlags, char *szClassName )
{
	if (pPersistant->nWeaponFlags & nFlags)
	{
		weaponInfo_t	*pWeaponInfo;

		if ( pWeaponInfo = com->FindRegisteredWeapon( szClassName ) )
		{
			// give the weapon
			userInventory_t *weap;
			if ((gstate->episode == 1) && (nFlags & PERS_WEAP_WEAPON8))		// don't refill the gashands!!!
			{
				weap = pWeaponInfo->give_func( pEnt, -1 );
			}
			else
			{
				weap = pWeaponInfo->give_func( pEnt, 0);//pPersistant->nAmmo[nIndex] );
			}

			// arm it if found and current.
			if( weap && (pPersistant->nCurrentWeaponFlag == ( pPersistant->nCurrentWeaponFlag & nFlags )) ) 
			{
				//pWeaponInfo->select_func( pEnt );
				pEnt->curWeapon = weap;
				userEntity_t *weapon = (userEntity_t *)pEnt->client->ps.weapon;
				if (weapon)
				{
					if (weapon->inuse)
						gstate->RemoveEntity(weapon);
					pEnt->client->ps.weapon = NULL;
				}
			}	
		}
	}

	if (pPersistant->nAmmo[nIndex])
	{
		DoAmmoInit(pEnt,nIndex,pPersistant->nAmmo[nIndex],szClassName);
	}
}

void AddEpisode1Weapons( userEntity_t *pEnt, client_persistant_s *pPersistant )
{
	unsigned int	nWeaponFlags = pPersistant->nWeaponFlags;
/*
	if( nWeaponFlags & PERS_WEAP_WEAPON1 ) DoWeaponInit( pEnt, pPersistant, 0, PERS_WEAP_WEAPON1, "weapon_disruptor" );
	if( nWeaponFlags & PERS_WEAP_WEAPON2 ) DoWeaponInit( pEnt, pPersistant, 1, PERS_WEAP_WEAPON2, "weapon_ionblaster" );
	if( nWeaponFlags & PERS_WEAP_WEAPON3 ) DoWeaponInit( pEnt, pPersistant, 2, PERS_WEAP_WEAPON3, "weapon_c4" );
	if( nWeaponFlags & PERS_WEAP_WEAPON4 ) DoWeaponInit( pEnt, pPersistant, 3, PERS_WEAP_WEAPON4, "weapon_shotcycler" );
	if( nWeaponFlags & PERS_WEAP_WEAPON5 ) DoWeaponInit( pEnt, pPersistant, 4, PERS_WEAP_WEAPON5, "weapon_sidewinder" );
	if( nWeaponFlags & PERS_WEAP_WEAPON6 ) DoWeaponInit( pEnt, pPersistant, 5, PERS_WEAP_WEAPON6, "weapon_shockwave" );
	if( nWeaponFlags & PERS_WEAP_WEAPON8 ) DoWeaponInit( pEnt, pPersistant, 0, PERS_WEAP_WEAPON8, "weapon_gashands" );
*/
	DoWeaponInit( pEnt, pPersistant, 0, PERS_WEAP_WEAPON1, "weapon_disruptor" );
	DoWeaponInit( pEnt, pPersistant, 1, PERS_WEAP_WEAPON2, "weapon_ionblaster" );
	DoWeaponInit( pEnt, pPersistant, 2, PERS_WEAP_WEAPON3, "weapon_c4" );
	DoWeaponInit( pEnt, pPersistant, 3, PERS_WEAP_WEAPON4, "weapon_shotcycler" );
	DoWeaponInit( pEnt, pPersistant, 4, PERS_WEAP_WEAPON5, "weapon_sidewinder" );
	DoWeaponInit( pEnt, pPersistant, 5, PERS_WEAP_WEAPON6, "weapon_shockwave" );
	DoWeaponInit( pEnt, pPersistant, 0, PERS_WEAP_WEAPON8, "weapon_gashands" );
}

void AddEpisode2Weapons( userEntity_t *pEnt, client_persistant_s *pPersistant )
{
	unsigned int	nWeaponFlags = pPersistant->nWeaponFlags;
/*
	if( nWeaponFlags & PERS_WEAP_WEAPON1 ) DoWeaponInit( pEnt, pPersistant, 0, PERS_WEAP_WEAPON1, "weapon_discus" );
	if( nWeaponFlags & PERS_WEAP_WEAPON2 ) DoWeaponInit( pEnt, pPersistant, 1, PERS_WEAP_WEAPON2, "weapon_venomous" );
	if( nWeaponFlags & PERS_WEAP_WEAPON3 ) DoWeaponInit( pEnt, pPersistant, 2, PERS_WEAP_WEAPON3, "weapon_sunflare" );
	if( nWeaponFlags & PERS_WEAP_WEAPON4 ) DoWeaponInit( pEnt, pPersistant, 3, PERS_WEAP_WEAPON4, "weapon_hammer" );
	if( nWeaponFlags & PERS_WEAP_WEAPON5 ) DoWeaponInit( pEnt, pPersistant, 4, PERS_WEAP_WEAPON5, "weapon_trident" );
	if( nWeaponFlags & PERS_WEAP_WEAPON6 ) DoWeaponInit( pEnt, pPersistant, 5, PERS_WEAP_WEAPON6, "weapon_zeus" );
*/
	DoWeaponInit( pEnt, pPersistant, 0, PERS_WEAP_WEAPON1, "weapon_discus" );
	DoWeaponInit( pEnt, pPersistant, 1, PERS_WEAP_WEAPON2, "weapon_venomous" );
	DoWeaponInit( pEnt, pPersistant, 2, PERS_WEAP_WEAPON3, "weapon_sunflare" );
	DoWeaponInit( pEnt, pPersistant, 3, PERS_WEAP_WEAPON4, "weapon_hammer" );
	DoWeaponInit( pEnt, pPersistant, 4, PERS_WEAP_WEAPON5, "weapon_trident" );
	DoWeaponInit( pEnt, pPersistant, 5, PERS_WEAP_WEAPON6, "weapon_zeus" );
}

void AddEpisode3Weapons( userEntity_t *pEnt, client_persistant_s *pPersistant )
{
	unsigned int	nWeaponFlags = pPersistant->nWeaponFlags;
/*
	if( nWeaponFlags & PERS_WEAP_WEAPON1 ) DoWeaponInit( pEnt, pPersistant, 0, PERS_WEAP_WEAPON1, "weapon_silverclaw" );
	if( nWeaponFlags & PERS_WEAP_WEAPON2 ) DoWeaponInit( pEnt, pPersistant, 1, PERS_WEAP_WEAPON2, "weapon_bolter" );
	if( nWeaponFlags & PERS_WEAP_WEAPON3 ) DoWeaponInit( pEnt, pPersistant, 2, PERS_WEAP_WEAPON3, "weapon_stavros" );
	if( nWeaponFlags & PERS_WEAP_WEAPON4 ) DoWeaponInit( pEnt, pPersistant, 3, PERS_WEAP_WEAPON4, "weapon_ballista" );
	if( nWeaponFlags & PERS_WEAP_WEAPON5 ) DoWeaponInit( pEnt, pPersistant, 4, PERS_WEAP_WEAPON5, "weapon_wyndrax" );
	if( nWeaponFlags & PERS_WEAP_WEAPON6 ) DoWeaponInit( pEnt, pPersistant, 5, PERS_WEAP_WEAPON6, "weapon_nightmare" );
*/
	DoWeaponInit( pEnt, pPersistant, 0, PERS_WEAP_WEAPON1, "weapon_silverclaw" );
	DoWeaponInit( pEnt, pPersistant, 1, PERS_WEAP_WEAPON2, "weapon_bolter" );
	DoWeaponInit( pEnt, pPersistant, 2, PERS_WEAP_WEAPON3, "weapon_stavros" );
	DoWeaponInit( pEnt, pPersistant, 3, PERS_WEAP_WEAPON4, "weapon_ballista" );
	DoWeaponInit( pEnt, pPersistant, 4, PERS_WEAP_WEAPON5, "weapon_wyndrax" );
	DoWeaponInit( pEnt, pPersistant, 5, PERS_WEAP_WEAPON6, "weapon_nightmare" );
}

void AddEpisode4Weapons( userEntity_t *pEnt, client_persistant_s *pPersistant )
{
	unsigned int	nWeaponFlags = pPersistant->nWeaponFlags;
/*
	if( nWeaponFlags & PERS_WEAP_WEAPON1 ) DoWeaponInit( pEnt, pPersistant, 0, PERS_WEAP_WEAPON1, "weapon_glock" );
	if( nWeaponFlags & PERS_WEAP_WEAPON2 ) DoWeaponInit( pEnt, pPersistant, 1, PERS_WEAP_WEAPON2, "weapon_slugger" );
	if( nWeaponFlags & PERS_WEAP_WEAPON3 ) DoWeaponInit( pEnt, pPersistant, 2, PERS_WEAP_WEAPON3, "weapon_kineticore" );
	if( nWeaponFlags & PERS_WEAP_WEAPON4 ) DoWeaponInit( pEnt, pPersistant, 3, PERS_WEAP_WEAPON4, "weapon_ripgun" );
	if( nWeaponFlags & PERS_WEAP_WEAPON5 ) DoWeaponInit( pEnt, pPersistant, 4, PERS_WEAP_WEAPON5, "weapon_novabeam" );
	if( nWeaponFlags & PERS_WEAP_WEAPON6 ) DoWeaponInit( pEnt, pPersistant, 5, PERS_WEAP_WEAPON6, "weapon_metamaser" );
*/
	DoWeaponInit( pEnt, pPersistant, 0, PERS_WEAP_WEAPON1, "weapon_glock" );
	DoWeaponInit( pEnt, pPersistant, 1, PERS_WEAP_WEAPON2, "weapon_slugger" );
	DoWeaponInit( pEnt, pPersistant, 2, PERS_WEAP_WEAPON3, "weapon_kineticore" );
	DoWeaponInit( pEnt, pPersistant, 3, PERS_WEAP_WEAPON4, "weapon_ripgun" );
	DoWeaponInit( pEnt, pPersistant, 4, PERS_WEAP_WEAPON5, "weapon_novabeam" );
	DoWeaponInit( pEnt, pPersistant, 5, PERS_WEAP_WEAPON6, "weapon_metamaser" );
}

void FetchPersistantWeaponData( userEntity_t *pEnt, client_persistant_s	*pPersistant )
{
	if( pPersistant->nWeaponFlags & PERS_WEAP_WEAPON7 ) DoWeaponInit( pEnt, pPersistant, 0, PERS_WEAP_WEAPON7, "weapon_daikatana" );

	switch( gstate->episode )
	{
	case 1:
		AddEpisode1Weapons( pEnt, pPersistant );
		break;
	case 2:
		AddEpisode2Weapons( pEnt, pPersistant );
		break;
	case 3:
		AddEpisode3Weapons( pEnt, pPersistant );
		break;
	case 4:
		AddEpisode4Weapons( pEnt, pPersistant );
		break;
	}
}

void FetchSidekickEntData( edict_t *pClientEnt, edict_t *pSidekickEnt, int nType )
{
	client_persistant_s	*pPersistant;
	edict_t				*pEnt = pSidekickEnt;
	playerHook_t		*pHook = ( playerHook_t * ) pEnt->userHook;

	if( pEnt == NULL )
	{
		return;
	}

	if( nType == SIDEKICK_PERS_SUPERFLY )
	{
		pPersistant = pClientEnt->client->pers.pPersSuperfly;
	}
	else if( nType == SIDEKICK_PERS_MIKIKO )
	{
		pPersistant = pClientEnt->client->pers.pPersMikiko;
	}
	else
	{
		return;
	}

	if( pPersistant == NULL )
	{
		return;
	}

	if( pPersistant->nHealth )
	{
		pEnt->health					= pPersistant->nHealth;

		pEnt->armor_val					= pPersistant->nArmorVal;
		pEnt->armor_abs					= pPersistant->nArmorAbs;

		pEnt->team						= pPersistant->nTeam;

		pHook->items					= pPersistant->nItems;
		pHook->exp_flags				= pPersistant->nExpFlags;
		pHook->invulnerability_time		= pPersistant->fInvulnerabilityTime;
		pHook->envirosuit_time			= pPersistant->fEnvirosuitTime;
		pHook->wraithorb_time			= pPersistant->fWraithorbTime;
// SCG[1/4/00]: 		pHook->oxylung_time			= pPersistant->fOxylungTime;

		pHook->base_power				= pPersistant->fBasePower;
		pHook->base_attack				= pPersistant->fBaseAttack;
		pHook->base_speed				= pPersistant->fBaseSpeed;
		pHook->base_acro				= pPersistant->fBaseAcro;
		pHook->base_vita				= pPersistant->fBaseVita;
									
		pHook->power_boost				= pPersistant->fBoostPower;
		pHook->attack_boost				= pPersistant->fBoostAttack;
		pHook->speed_boost				= pPersistant->fBoostSpeed;
		pHook->acro_boost				= pPersistant->fBoostAcro;
		pHook->vita_boost				= pPersistant->fBoostVita;

		pHook->power_boost_time			= pPersistant->fTimePower;
		pHook->attack_boost_time		= pPersistant->fTimeAttack;
		pHook->speed_boost_time			= pPersistant->fTimeSpeed;
		pHook->acro_boost_time			= pPersistant->fTimeAcro;
		pHook->vita_boost_time			= pPersistant->fTimeVita;

		pHook->poison_time				= pPersistant->fPoisonTime;
		pHook->poison_next_damage_time	= pPersistant->fPoisonNextDamage;
		pHook->poison_damage			= pPersistant->fPoisonDamage;
		pHook->poison_interval			= pPersistant->fPoisonInterval;

		pEnt->s.angles					= pPersistant->vAngles;

		pHook->ai_flags					= pPersistant->nAIFlags;

		FetchPersistantWeaponData( pEnt, pPersistant );
	}
}

void FetchClientEntData( edict_t *pEnt )
{
	client_persistant_s	*pPersistant;
	playerHook_t		*pHook = ( playerHook_t * ) pEnt->userHook;

	if( pEnt == NULL )
	{
		return;
	}

	pPersistant = &pEnt->client->pers;

	if( pPersistant == NULL )
	{
		return;
	}

	if( pPersistant->nHealth )
	{
		if (!deathmatch->value)	// WAW[11/17/99]: These values get reset between levels.
		{
			pEnt->health				= pPersistant->nHealth;

			pEnt->armor_val				= pPersistant->nArmorVal;
			pEnt->armor_abs				= pPersistant->nArmorAbs;
		}
		pEnt->team					= pPersistant->nTeam;

		pEnt->record				= pPersistant->record;

		pHook->items				= pPersistant->nItems;
		pHook->exp_flags			= pPersistant->nExpFlags;
		pHook->invulnerability_time	= pPersistant->fInvulnerabilityTime;
		pHook->envirosuit_time		= pPersistant->fEnvirosuitTime;
		pHook->wraithorb_time		= pPersistant->fWraithorbTime;
// SCG[1/4/00]: 		pHook->oxylung_time			= pPersistant->fOxylungTime;

		pHook->base_power			= pPersistant->fBasePower;
		pHook->base_attack			= pPersistant->fBaseAttack;
		pHook->base_speed			= pPersistant->fBaseSpeed;
		pHook->base_acro			= pPersistant->fBaseAcro;
		pHook->base_vita			= pPersistant->fBaseVita;
		
		if (!deathmatch->value)	// WAW[11/17/99]: These values get reset between levels.
		{
			pHook->power_boost			= pPersistant->fBoostPower;
			pHook->attack_boost			= pPersistant->fBoostAttack;
			pHook->speed_boost			= pPersistant->fBoostSpeed;
			pHook->acro_boost			= pPersistant->fBoostAcro;
			pHook->vita_boost			= pPersistant->fBoostVita;

			pHook->power_boost_time		= pPersistant->fTimePower;
			pHook->attack_boost_time	= pPersistant->fTimeAttack;
			pHook->speed_boost_time		= pPersistant->fTimeSpeed;
			pHook->acro_boost_time		= pPersistant->fTimeAcro;
			pHook->vita_boost_time		= pPersistant->fTimeVita;

			// SCG[11/7/99]: Blech.  Do this only if we're going inbetween sublevels
			if( (gstate->nLevelTransitionType == 1) || (gstate->nLevelTransitionType == 3) )
			{
				pEnt->s.angles = pPersistant->vAngles;
				gstate->SetClientAngles( pEnt, pPersistant->vAngles );

				// only client 0 has this data!
				if ( (pEnt - gstate->g_edicts) == 1 )
				{
					if ( pPersistant->nTotalKills[gstate->subMap] ||
						 pPersistant->nFoundSecrets[gstate->subMap] ||
						 pPersistant->nTime[gstate->subMap] ||
						 pPersistant->nTotalMonsters[gstate->subMap] ||
						 pPersistant->nTotalSecrets[gstate->subMap]
						 )
					{
						gstate->numMonstersKilled	=  pPersistant->nTotalKills[gstate->subMap];
						gstate->numSecretsFound		=  pPersistant->nFoundSecrets[gstate->subMap];
						gstate->level->time			=  pPersistant->nTime[gstate->subMap];
						gstate->numMonsters			=  pPersistant->nTotalMonsters[gstate->subMap];
						gstate->numSecrets			=  pPersistant->nTotalSecrets[gstate->subMap];
					}
				}
			}
			else
			{
				// only clear out the found/killed numbers!  the others are already determined by now.
				gstate->numMonstersKilled	= 0;
				gstate->numSecretsFound		= 0;
//				gstate->level->time			= 0;
//				gstate->numMonsters			= 0;
//				gstate->numSecrets			= 0;
			}

			if( gstate->nLevelTransitionType != 2 )
			{
				FetchPersistantWeaponData( pEnt, pPersistant );
//				FetchSpecialData( pEnt, pPersistant );	// cek[2-22-00]
			}
		}
		else // WAW[12/13/99]: In deathmatch clear out specifics.
		{
			pEnt->record.deaths = 0;
			pEnt->record.frags	= 0;
			pEnt->record.dkexp	= 0;
		}			
	}
}

void InitClientPersistant (gclient_t *client)
{
//	gitem_t		*item;

	memset (&client->pers, 0, sizeof(client->pers));

	client->pers.nHealth	= 100;
	client->pers.bConnected	= true;
}


void InitClientResp (gclient_t *client)
{
	memset (&client->resp, 0, sizeof(client->resp));
	client->resp.enterframe = gstate->level->framenum;
	client->resp.nCurrentMode = INV_MODE_WEAPONS;
	client->resp.nInvWeaponMode = 1; 
//	client->resp.coop_respawn = client->pers;
}

float AI_Determine_Room_Height( userEntity_t *self, int Max_Mid, int type);

userEntity_t *Client_GetSidekickSpawnpoint( char *className )
{
	userEntity_t *pEntity = gstate->FirstEntity();

	while( pEntity )
	{
		if( pEntity->className )
		{
			if( !stricmp( pEntity->className, className ) )
			{
				if( ( pEntity->targetname == NULL ) && ( gstate->game->spawnpoint[0] == NULL ) )
					return pEntity;

				if( pEntity->targetname )
				{
					if( !stricmp( pEntity->targetname, gstate->game->spawnpoint ) )
					{
						return	pEntity;
					}
				}	
			}
		}

		pEntity = gstate->NextEntity( pEntity );
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//	dll_PutClientInServer
//
//	initializes the client data structures at connection time
///////////////////////////////////////////////////////////////////////////////
#define SPAWNED_SUPERFLY_WITH_CLIENT	0x01
#define SPAWNED_MIKIKO_WITH_CLIENT		0x02

void dll_PutClientInServer(userEntity_t *self)
{
	userEntity_t		*spot, *sidekickspot;
	userEntity_t		*pSuperfly = NULL;
	userEntity_t		*pMikiko = NULL;
	playerHook_t		*hook;
	gclient_t			*client;
	client_persistant_t	saved;
	client_respawn_t	resp;

	///////////////////////////////////////////////////////////////////////////
	//	find the spawn point
	///////////////////////////////////////////////////////////////////////////
	if (ctf->value)
		spot = CTFPutClientInServer(self);
	else if (deathtag->value)
		spot = DT_PutClientInServer(self);
	else if (coop->value)
		spot = COOP_GetSpawnPoint(self);
	else
	{
		if (deathmatch->value && dm_teamplay->value)
		{
			TEAM_Verify(self,"team",NULL);
		}
		spot = Client_GetSpawnPoint( "info_player" );
	}

	if (!spot)
	{
		if (deathmatch->value)
		{
			if (ctf->value || deathtag->value)
				com->Error ("Missing teamplay spawn points.");
			else
				com->Error ("No info_player_deathmatch spawn points.");
		}
		else if (coop->value)
			com->Error ("No info_player_coop spawn points.");
		else		
			com->Error ("No info_player_start spawn points.");
	}
	else
	{
		self->s.origin = spot->s.origin;
		self->s.angles = spot->s.angles;
		self->client->fix_angles	= spot->s.angles;
		self->client->v_angle		= spot->s.angles;
		self->client->oldviewangles	= spot->s.angles;
		self->client->ps.viewangles	= spot->s.angles;
		self->flags |= FL_FIXANGLES;
	}

	Client_InitGClient (self, self->s.origin, self->s.angles);
	
	if( gstate->nLevelTransitionType != 1 )
	{
		self->client->pers.vAngles = spot->s.angles;
	}

	self->spawnflags = spot->spawnflags;

	int index = self - gstate->g_edicts - 1;
	client = self->client;

	// nLevelTransitionType == 0 for map transition
	// nLevelTransitionType == 1 for sub-level transition
	// nLevelTransitionType == 2 for episode transition
	if( ( gstate->nLevelTransitionType != 1 ) && ( gstate->nLevelTransitionType != 3 ) )
	{
		// SCG[1/31/00]: cleat out stats
		for( int i = 0; i < MAX_STATS; i++ )
		{
			client->ps.stats[i]				= 0;
		}

		for( i = 0; i < MAX_SUBMAPS; i++ )
		{
			client->pers.nTotalKills[i]		= 0;
			client->pers.nTotalMonsters[i]	= 0;
			client->pers.nFoundSecrets[i]	= 0;
			client->pers.nTotalSecrets[i]	= 0;
			client->pers.nTime[i]			= 0;
		}

		// zap the inventory stuff
		if (gstate->nLevelTransitionType == 2)
		{
			client->pers.nWeaponFlags = client->pers.nCurrentWeaponFlag = 0;
			memset(client->pers.inventoryCrap,0,sizeof(client->pers.inventoryCrap));
		}

	}

	//	allocate hook structure
//	self->userHook = new playerHook_t;
	self->userHook = gstate->X_Malloc( sizeof( playerHook_t ), MEM_TAG_HOOK );
	hook = (playerHook_t *)self->userHook;
	hook->force_rate.Set( 0,0,0 );
    SetTeam( self );

	// SCG[11/20/99]: set load/save routines
	self->save = Client_Save;
	self->load = Client_Load;

//	self->flags |= FL_FIXANGLES;
//	gstate->SetClientAngles(self, spot->s.angles );

	//	generic initialization routines
	Client_InitUserEntity (self);

	// SCG[10/22/99]: this clears the timers for poisoning, powerups, temporary 
	// SCG[10/22/99]: attribute boosts and other misc. timers
	if( gstate->nLevelTransitionType != 1 )
	{
		Client_InitTimers( self );
	}

	// SCG[10/22/99]: this sets all of the base attributes for the player
	// SCG[10/22/99]: ( health, attributes, etc.. )
	Client_InitAttributes( self );

	// SCG[10/22/99]: this initializes misc. hook flags and stats
	Client_InitHookMisc( self );

	// SCG[10/22/99]: this clears all of the weapon info (could be the cause
	// SCG[10/22/99]: of the weapon frames getting hosed after a sub-level change )
	Client_InitWeapons( self );

	// this is done below...
/*
    if( deathmatch->value )
	{
		// SCG[10/22/99]: clears deathmatch stats.
		Client_InitStats( self );
	}
*/

	// SCG[10/22/99]: this clears out the inventory if one exists and allocates a new one
	Client_InitInventory( self );

	// SCG[10/22/99]: restores persistant data between levels
	// make sure new games get wiped
	if ( !((gstate->nLevelTransitionType == 2) && (gstate->episode == 1)) )
	{
		FetchClientEntData( self );
		void *inv = (void *)self->client->pers.inventoryCrap;
		com->InventoryLoad( &inv, self, MAX_PERS_INVENTORY );	// cek[2-22-00]
	}

	// SCG[10/22/99]: resets camera to normal
	Client_InitCamera( self );

	Client_InitClientModel( self );

	// SCG[10/22/99]: inits AI stuff (move speed, attack dist, goalentity, etc.. )
	Client_InitAIDefaults( self );

	// SCG[10/22/99]: inits the node systems (air, ground, track... )
    if( !deathmatch->value )
	{
	    Client_InitNodeSystem( self );
	}

	// WAW[11/17/99]: Itit the CTF Stuff.
//	if (ctf->value)
//		CTFPutClientInServer(self);

	SetTeam( self );

	// WAW[11/18/99]: Special handling for the deathmatch.  The persistant information is set up 
	// at a diffent time.   Here we just check to see if all persistant attributes are zero
	// if they are then we can initailize the client attributes and set the persistant attributes 
	// to thoughs.
	if (deathmatch->value)// || (coop->value && (gstate->nLevelTransitionType == 2) && (gstate->episode == 1)) )
	{
		Client_InitAttributes(self);
		client_persistant_s	*pers = &self->client->pers;
		pers->fBasePower	= pers->fBoostPower		= hook->base_power;
		pers->fBaseAttack	= pers->fBoostAttack	= hook->base_attack;
		pers->fBaseSpeed	= pers->fBoostSpeed		= hook->base_speed;
		pers->fBaseAcro		= pers->fBoostAcro		= hook->base_acro;
		pers->fBaseVita		= pers->fBoostVita		= hook->base_vita;
		int level = calcStatLevel( self );
		// Give that many exp points.
		self->record.exp = exp_level[ level ];

		// reset the daikatana
		self->record.dkexp = 0;

        com->CalcBoosts(self);		
		pers->nHealth = self->health = hook->base_health;
	}

    self->client->showflags &= ~(SHOW_SCORES | SHOW_SCORES_NOW);
	///////////////////////////////////////////////////////////////////////////
	//	init animations
	///////////////////////////////////////////////////////////////////////////
	
	Client_InitAnims( self );

	Client_InitDefaultWeapon( self );

	CLIENT_StartStand( self, TH_STAND );

	gstate->Con_Dprintf("Player position: %f %f %f\n", self->s.origin.x, self->s.origin.y, self->s.origin.z);

	// SCG[11/29/99]: Spawn sidekicks in single player mode
	if( ( deathmatch->value == 0 ) && ( coop->value == 0 ) )
	{
		CVector vOrigin, vAngles;
		unsigned int nSidekickMask = gstate->game->serverflags & SFL_SIDEKICK_MASK;

		pSuperfly = AIINFO_GetSuperfly();
		if( pSuperfly == NULL )
		{
			pSuperfly = AIINFO_GetMikikofly();
		}
		pMikiko = AIINFO_GetMikiko();

		playerHook_t *pSuperflyHook = NULL;
		playerHook_t *pMikikoHook = NULL;

		if( pSuperfly != NULL )
		{
			pSuperflyHook = ( playerHook_t * ) pSuperfly->userHook;
		}
		if( pMikiko != NULL )
		{
			pMikikoHook = ( playerHook_t * ) pMikiko->userHook;
		}

		// SCG[2/2/00]: When doing a level transition, make sure the player origin
		// SCG[2/2/00]: is a combnation of the difference from the triggers origin 
		// SCG[2/2/00]: and where he hit the trigger on the last level plus the origin
		// SCG[2/2/00]: taken from the spawnpoint on this level.
		if( gstate->nLevelTransitionType == 1 )
		{
			if( ( spot != NULL ) && !( spot->spawnflags & 0x01 ) )
			{
				self->s.origin += self->client->pers.vOffset;
				self->s.origin.z += 8; // SCG[2/2/00]: Keep from spawning in the ground...
			}
		}

		// SCG[11/29/99]: Spawn Superfly
		if( 
			( ( gstate->nLevelTransitionType == 1 ) && ( nSidekickMask & ( SFL_SPAWN_SUPERFLY | SFL_SPAWN_MIKIKOFLY ) ) ) || 
			( ( gstate->nLevelTransitionType == 3 ) && ( self->client->pers.nSidekicksSpawnedWithClient & SPAWNED_SUPERFLY_WITH_CLIENT ) ) || 
			( pSuperfly != NULL )
			)
		{

			if( self->client->pers.pPersSuperfly != NULL )
			{
				if( coop->value == 0 && deathmatch->value == 0 ) 
				{
					if( ( nSidekickMask & SFL_SPAWN_MIKIKOFLY ) || ( ( pSuperflyHook!= NULL ) && ( pSuperflyHook->bCarryingMikiko ) ) )
					{
						sidekickspot = Client_GetSidekickSpawnpoint( "info_mikikofly_start" );
					}
					else
					{
						sidekickspot = Client_GetSidekickSpawnpoint( "info_superfly_start" );
					}
				}
				else
				{
					sidekickspot = Client_GetSpawnPoint( "info_superfly" );
				}
				if( sidekickspot != NULL )
				{
					vOrigin = sidekickspot->s.origin;
					vAngles = sidekickspot->s.angles;
				}
				else
				{
					vOrigin = self->client->pers.pPersSuperfly->vOffset;
					vAngles = self->client->pers.pPersSuperfly->vAngles;
				}

				if( pSuperfly == NULL )
				{
					if( nSidekickMask & SFL_SPAWN_MIKIKOFLY )
					{
						pSuperfly = SIDEKICK_SpawnMikikoFly( vOrigin, vAngles );
					}
					else
					{
						pSuperfly = SIDEKICK_SpawnSuperfly( vOrigin, vAngles );
					}
					pSuperfly->s.origin = vOrigin;
				}
				else
				{
					pSuperfly->s.origin = vOrigin;
					pSuperfly->s.modelindex = gstate->ModelIndex( pSuperfly->modelName );
				}

				if( pSuperfly != NULL )
				{
					FetchSidekickEntData( self, pSuperfly, SIDEKICK_PERS_SUPERFLY );
					pSuperfly->flags &= ~FL_NOSAVE;

				}

				gstate->Con_Dprintf ("Superfly position: %f %f %f\n", pSuperfly->s.origin.x, pSuperfly->s.origin.y, pSuperfly->s.origin.z);
				gstate->LinkEntity( pSuperfly );
			}

			pSuperflyHook = ( playerHook_t * ) pSuperfly->userHook;
			if( pSuperflyHook != NULL )
			{
				pSuperflyHook->dflags &= ~DFL_LLAMA;
			}

			self->client->pers.nSidekicksSpawnedWithClient |= SPAWNED_SUPERFLY_WITH_CLIENT;
		}
		else
		{
			self->client->pers.nSidekicksSpawnedWithClient &= ~SPAWNED_SUPERFLY_WITH_CLIENT;
		}

		// SCG[11/29/99]: Spawn Mikiko
		if( 
			( ( gstate->nLevelTransitionType == 1 ) && ( nSidekickMask & SFL_SPAWN_MIKIKO ) ) || 
			( ( gstate->nLevelTransitionType == 3 ) && ( self->client->pers.nSidekicksSpawnedWithClient & SPAWNED_MIKIKO_WITH_CLIENT ) ) || 
			( pMikiko != NULL  )
			)
		{
			if( self->client->pers.pPersMikiko != NULL )
			{
				if( coop->value == 0 && deathmatch->value == 0 ) 
				{
					sidekickspot = Client_GetSidekickSpawnpoint( "info_mikiko_start" );
				}
				else
				{
					sidekickspot = Client_GetSpawnPoint( "info_mikiko" );
				}

				if( sidekickspot != NULL )
				{
					vOrigin = sidekickspot->s.origin;
					vAngles = sidekickspot->s.angles;
				}
				else
				{
					vOrigin = self->client->pers.pPersMikiko->vOffset;
					vAngles = self->client->pers.pPersMikiko->vAngles;
				}

				if( pMikiko == NULL )
				{
					pMikiko = SIDEKICK_SpawnMikiko( vOrigin, vAngles );
					pMikiko->s.origin = vOrigin;
				}
				else
				{
					pMikiko->s.origin = vOrigin;
					pMikiko->s.modelindex = gstate->ModelIndex( pMikiko->modelName );
				}

				if( pMikiko != NULL )
				{ 
					FetchSidekickEntData( self, pMikiko, SIDEKICK_PERS_MIKIKO );
					pMikiko->flags &= ~FL_NOSAVE;
				}
				gstate->Con_Dprintf ("Mikiko position: %f %f %f\n", pMikiko->s.origin.x, pMikiko->s.origin.y, pMikiko->s.origin.z);
				gstate->LinkEntity( pMikiko );
			}

			pMikikoHook = ( playerHook_t * ) pMikiko->userHook;
			if( pMikikoHook != NULL )
			{
				pMikikoHook->dflags &= ~DFL_LLAMA;
			}

			self->client->pers.nSidekicksSpawnedWithClient |= SPAWNED_MIKIKO_WITH_CLIENT;
		}
		else
		{
			self->client->pers.nSidekicksSpawnedWithClient &= ~SPAWNED_MIKIKO_WITH_CLIENT;
		}
	}

	// WAW[11/9/99]: Coop only, see if we need to destroy a bot and/or override the starting spot.
	// WAW[12/8/99]: Moved to the coop file.
	// ===========================================================================================
//	if (coop->value)
//		CoopPlacePlayer(self);
	//================

	// gotta do it.. this guy has to be able to get into the game!
	if( deathmatch->value != 0 )
	{
		spawn_tele_gib( self->s.origin, self );
	}

	alist_add( self );

	//	add one to running total of clients
	cur_clients++;

	hook->ID = com->GenID();

	//	clear any messages that might remain between level changes
	gstate->clearxy();

	//	link entity into BSP
	gstate->LinkEntity( self );	//	must be done after solid types are set!

	// SCG[10/22/99]: TODO: add forced weapon change here to change the weapon to
	// SCG[10/22/99]: that stored in the persistant data.

	// WAW[12/8/99]: Fix the stats -- incase inventory is off.
	fixStats(self);
	com->CalcBoosts(self);

	// SCG[12/12/99]: Add transient entities
	userEntity_t *pEnt;

	func_t func;

	if( gstate->nLevelTransitionType == 1 )
	{
		for( int i = 0; gstate->game->transients[i].className[0] != NULL; i++ )
		{
			pEnt = gstate->SpawnEntity();
			pEnt->s.origin = self->s.origin + gstate->game->transients[i].vOffset;
			pEnt->className = gstate->game->transients[i].className;
			pEnt->spawnflags = gstate->game->transients[i].spawnflags;
			func = ( func_t ) com->FindSpawnFunction( pEnt->className );
			if( func == NULL )
			{
				continue;
			}
			func( pEnt );		
			pEnt->health = gstate->game->transients[i].health;
		}
	}
/*
	// SCG[1/19/00]: check for savegems and inform client if necessary
	int nNumSavegems = gstate->InventoryItemCount( self->inventory,"item_savegem" );
	if( nNumSavegems != 0 )
	{
		gstate->WriteByte( SVC_SAVEGEM );
		gstate->WriteByte( false );
		gstate->WriteByte( nNumSavegems );
		gstate->UniCast( self, true );
	}
*/
	// SCG[1/19/00]: Update boost icons if necessary
	playerHook_t *pHook = ( playerHook_t * ) self->userHook;
	if( pHook != NULL )
	{
		if( pHook->power_boost_time )	com->Boost_Icons( self, BOOSTICON_ADD, ICON_POWERBOOST, pHook->power_boost_time );
		if( pHook->attack_boost_time )	com->Boost_Icons( self, BOOSTICON_ADD, ICON_ATTACKBOOST, pHook->attack_boost_time );
		if( pHook->speed_boost_time )	com->Boost_Icons( self, BOOSTICON_ADD, ICON_SPEEDBOOST, pHook->speed_boost_time );
		if( pHook->acro_boost_time )	com->Boost_Icons( self, BOOSTICON_ADD, ICON_ACROBOOST, pHook->acro_boost_time );
		if( pHook->vita_boost_time )	com->Boost_Icons( self, BOOSTICON_ADD, ICON_VITABOOST, pHook->vita_boost_time );
	}

	func = (func_t)com->FindSpawnFunction("dll_weapon_reinit_3p_models");
	if (func)
		func(self);

	if( ( coop->value == 0 ) && ( deathmatch->value == 0 ) )
	{
		if( ( gstate->nLevelTransitionType != 1 ) && ( gstate->nLevelTransitionType != 3 ) )
		{
			if( spot )
			{
				self->s.origin				= spot->s.origin;
				self->s.angles				= spot->s.angles;
				self->client->fix_angles	= spot->s.angles;
				self->client->v_angle		= spot->s.angles;
				self->client->oldviewangles	= spot->s.angles;
				self->client->ps.viewangles	= spot->s.angles;
				self->flags |= FL_FIXANGLES;
			}
		}
		else
		{
			self->client->fix_angles	= self->s.angles;
			self->client->v_angle		= self->s.angles;
			self->client->oldviewangles	= self->s.angles;
			self->client->ps.viewangles	= self->s.angles;
			self->flags |= FL_FIXANGLES;
		}
	}

	self->client->fix_angles.z		= 0;
	self->client->v_angle.z			= 0;
	self->client->oldviewangles.z	= 0;
	self->client->ps.viewangles.z	= 0;
	self->flags |= FL_FIXANGLES;

	recalc_level(self);
	CTFGiveControlCard(self);
	COOP_SpawnHacks(self);

	self->client->do_autosave = TRUE;
}

///////////////////////////////////////////
//	dll_ClientDisconnect
//
//	called when a client disconnects from the server
///////////////////////////////////////////

void dll_ClientDisconnect(userEntity_t *self)
{
	//playerHook_t	*hook = (playerHook_t *)self->userHook;
    playerHook_t    *hook = (playerHook_t *)self->userHook;
	gstate->bprintf (PRINT_HIGH, "%s has left the game.\n", self->client->pers.netname);

	if(!deathmatch->value)
        Client_FreeNodeSystem (self);
	
	if (coop->value)// && CoopIsMainPlayer(self))
	{
		COOP_ClientDisconnect(self);
	}
//	else if (coop->value)
//	{
//		COOP_ClientDisconnect(self);
//	}
	else if (ctf->value)			// WAW[11/16/99]: Might have a flag... Need to drop it if we do.
		CTFClientDisconnect( self );
	else if (deathtag->value)
		DT_ClientDisconnect(self);

	alist_remove( self );

	gstate->UnlinkEntity (self);
	self->s.modelindex = 0;
	self->solid = SOLID_NOT;
	gstate->LinkEntity (self);

	self->className = "disconnected";
	self->inuse = false;

	cur_clients--;
}

///////////////////////////////////////////////////////////////////////////////
//	dll_ClientConnect
//
//////////////////////////////////////////////////////////////////////////////

int		dll_ClientConnect (userEntity_t *self, void *userinfo, int loadgame)
{
	gclient_t			*client;
	client_persistant_t	saved;

	//	Q2FIXME:	check banned ip list and password in userinfo

	//	Q2FIXME:	check loadgame, init respawn and persistent data

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if( self->inuse == false )
	{
		// clear the respawning variables
		InitClientResp( self->client );

		client = self->client;
		saved = client->pers;
		if( client->pers.nHealth <= 0 )
		{
			InitClientPersistant( self->client );
		}
		client->pers = saved;

	}
	if (ctf->value)
		CTFClientConnect( self, (char *)userinfo );

	if (deathmatch->value)
	{
		// initialize character,model and skin. In deathmatch, prevent character or model changes after game start...
		char *s = Info_ValueForKey      ((char *)userinfo, "skinname");    // get skin
		char *m = Info_ValueForKey      ((char *)userinfo, "modelname");   // get model  3.9 dsn
		strncpy(self->client->pers.body_info.modelname,m,sizeof(self->client->pers.body_info.modelname)-1);
		strncpy(self->client->pers.body_info.skinname, s,sizeof(self->client->pers.body_info.skinname)-1);
		char *temp = Info_ValueForKey ((char *)userinfo, "character");
		if (strlen(temp))
			self->client->pers.body_info.character = atoi(temp);

		// send the string in the motd cvar to the client.
		if (gstate->GetCvar("dedicated") != 0)
		{
			do_motd = TRUE;
		}
	}

	//	Q2FIXME:	call ClientUserinfoChanged
	dll_ClientUserinfoChanged (self, (char *) userinfo);

	//	Print connect message if there is already someone connected
	//	Q2FIXME:	init game sturcture
	if (maxclients->value > 1)
		gstate->Con_Printf ("%s connected\n", self->netname);

	//	Q2FIXME:	were is level variable?  Make this part of serverState_t
	//	Q2FIXME:	is incrementing level.players really necessary?
	gstate->level->players++;

//	self->client->pers.connected = true;
	return	true;
}

///////////////////////////////////////////////////////////////////////////////
//	dll_ClientBegin
//
//	called when a client finishes connecting to the server
//	calls ClientEndServerFrame...  
///////////////////////////////////////////////////////////////////////////////

void	Client_MoveToIntermission (userEntity_t *ent);


///////////////////////////////////////////////////////////////////////////////
//
//  G_InitEdict - copied from quake2
//
///////////////////////////////////////////////////////////////////////////////

void G_InitEdict (edict_t *e)
{
	e->inuse = true;
	e->className = "noclass";
	e->gravity = 1.0;

	// FIXME: this should clear all record stats
	// clear frags
	e->record.frags = 0;
	e->record.exp = 0;
	e->record.deaths = 0;
	e->record.level = 0;
	e->record.dkexp = 0;

	//	force frame syncing for new entities so re-used entities get their frame
	//	index sent for sure!
	e->s.frameInfo.frameFlags |= FRAME_FORCEINDEX;
}

///////////////////////////////////////////////////////////////////////////////
//
//  ClientBeginDeathmatch - called when starting deathmatch
//
///////////////////////////////////////////////////////////////////////////////
void ClientBeginDeathmatch (userEntity_t* self)
{
	// clear out this edict
	G_InitEdict( self );
	
	// from quake 2
	InitClientResp (self->client);

	// initialize character,model and skin. In deathmatch, prevent character or model changes after game start...
	char *s = Info_ValueForKey      ((char *)self->client->pers.userinfo, "skinname");    // get skin
	char *m = Info_ValueForKey      ((char *)self->client->pers.userinfo, "modelname");   // get model  3.9 dsn
	strncpy(self->client->pers.body_info.modelname,m,sizeof(self->client->pers.body_info.modelname)-1);
	strncpy(self->client->pers.body_info.skinname, s,sizeof(self->client->pers.body_info.skinname)-1);
	char *temp = Info_ValueForKey ((char *)self->client->pers.userinfo, "character");
	if (strlen(temp))
		self->client->pers.body_info.character = atoi(temp);

	if (do_motd)
	{
		cvar_t *motd = gstate->cvar("motd","",0);
		char string[256];
		Com_sprintf(string,sizeof(string),motd->string);
		char *ptr = string;
		while( ptr = strchr(ptr,'|') )
		{
			*ptr = '\n';
		}
		gstate->centerprint(self, 5.0, "motd:%s\n", string);
		do_motd = false;
	}
	// locate ent at a spawn point
	dll_PutClientInServer (self);

	if (gstate->fogactive)
	{
		gstate->WriteByte (SVC_EFFECT_NOTIFY);				// wake up, here's a message!
		gstate->WriteLong (ET_FOG);							// its about the fog!
		gstate->WriteByte (1);								// fog on?
		gstate->WriteFloat (gstate->fogcolor[0]);			// red
		gstate->WriteFloat (gstate->fogcolor[1]);			// green
		gstate->WriteFloat (gstate->fogcolor[2]);			// blue
		gstate->WriteFloat (gstate->fogstart);				// start
		gstate->WriteFloat (gstate->fogend);				// end
		gstate->WriteFloat (gstate->fogskyend);				// sky end
		gstate->WriteByte (0);								// fade increment
		gstate->UniCast (self, true);						// send it dude!
	}

	// spawn effect from quake 2
//	// send effect
//	gstate->WriteByte (svc_muzzleflash);
//	gstate->WriteShort (self-g_edicts);
//	gstate->WriteByte (MZ_LOGIN);
//	gstate->multicast (self->s.origin, MULTICAST_PVS);

	gstate->bprintf (PRINT_HIGH, "%s joined the game.\n", self->client->pers.netname);

	// we don't have this either
//	// make sure all view stuff is valid
//	ClientEndServerFrame (self);
}

void	dll_ClientBegin (userEntity_t *self, int loadgame)
{
	//playerHook_t	*hook = (playerHook_t *)self->userHook;
	self->client = gstate->game->clients + (self - gstate->g_edicts - 1);

	// turn off the bounding box debugging stuff
	gstate->SetCvar("p_showboxes","0");

	if (deathmatch->value)
	{
		ClientBeginDeathmatch(self);
		dll_ClientUserinfoChanged(self,self->client->pers.userinfo);
		return;
	}

	if( self->inuse == TRUE )
	{
		// the client has cleared the client side viewangles upon
		// connecting to the server, which is different than the
		// state when the game is saved, so we need to compensate
		// with deltaangles
		for( int i = 0; i < 3 ;i++ )
		{
			self->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(self->client->ps.viewangles[i]);
		}
	}
	else
	{
		G_InitEdict( self );
		InitClientResp( self->client );
		dll_PutClientInServer( self );

        //adam: added to play intro cinematic for this level.
        PlayIntroCinematic();
	}

	if (gstate->level->intermissiontime)
	{
		Client_MoveToIntermission (self);
	}
	else
	{
#pragma message ("// SCG[10/23/99]: Add teleport fog here" )
	}
	
	//	Q2FIXME:	ClientEndServerFrame == PlayerPostThink?!?!?
	//	make sure all view stuff is valid

	//	Q2FIXME:	CALL THIS FROM PHYSICS.DLL!!!!!
	//	ClientEndServerFrame (self);
	//Client_PostThink (self);

	if (gstate->fogactive)
	{
		gstate->WriteByte (SVC_EFFECT_NOTIFY);				// wake up, here's a message!
		gstate->WriteLong (ET_FOG);							// its about the fog!
		gstate->WriteByte (1);								// fog on?
		gstate->WriteFloat (gstate->fogcolor[0]);			// red
		gstate->WriteFloat (gstate->fogcolor[1]);			// green
		gstate->WriteFloat (gstate->fogcolor[2]);			// blue
		gstate->WriteFloat (gstate->fogstart);				// start
		gstate->WriteFloat (gstate->fogend);				// end
		gstate->WriteFloat (gstate->fogskyend);				// sky end
		gstate->WriteByte (0);								// fade increment
		gstate->UniCast (self, true);						// send it dude!
	}

    AIINFO_SetPlayer( self );
	dll_ClientUserinfoChanged(self,self->client->pers.userinfo);
}

///////////////////////////////////////////////////////////////////////////////
//	dll_ClientBeginServerFrame
//
//	called when a client finishes connecting to the server
//	calls ClientEndServerFrame...  
///////////////////////////////////////////////////////////////////////////////

void	dll_ClientBeginServerFrame (userEntity_t *self)
{
	//////////////////////////////////////////////////////////////////
	//	if dead, do special stuff
	//////////////////////////////////////////////////////////////////

	if (self->deadflag == DEAD_PUNK || self->deadflag == DEAD_DEAD || self->deadflag == DEAD_SIDEKICK)
	{
		Client_DeathThink (self);
		return;
	}

	if (!(self->flags & FL_INWARP))
	{
		Client_CheckInertialDamage (self);

		Client_CheckWaterDamage (self);
	}
}

/* ------------------------------ ClientThink ------------------------------- */

///////////////////////////////////////////////////////////////////////////////
//	Client_SetFOVRamp
//
///////////////////////////////////////////////////////////////////////////////

/*
void	Client_SetFOVRamp (userEntity_t *self, float rate, float desired)
{
	playerHook_t	*hook = (playerHook_t *)self->userHook;

	hook->fov_rate = rate;
	if( hook->fov_rate > 160 )
	{
		hook->fov_rate = 160;
	}
	if( hook->fov_rate < 60 )
	{
		hook->fov_rate = 60;
	}

	hook->fov_desired = desired;
	if( hook->fov_desired > 160 )
	{
		hook->fov_desired = 160;
	}
	if( hook->fov_desired < 60 )
	{
		hook->fov_desired = 60;
	}

	hook->fov_last_change = gstate->Sys_Milliseconds ();
}

///////////////////////////////////////////////////////////////////////////////
//	Client_FOVRamp
//
//	handles ramping of client FOV
///////////////////////////////////////////////////////////////////////////////

void	Client_FOVRamp (userEntity_t *self)
{
	playerHook_t	*hook = (playerHook_t *)self->userHook;
	float			change, change_mult;
	int				ms, time;

	if (self->client->ps.fov != hook->fov_desired)
	{
		ms = gstate->Sys_Milliseconds ();
		time = ms - hook->fov_last_change;
		hook->fov_last_change = ms;

		change_mult = (float) time * 0.001;

		if (self->client->ps.fov < hook->fov_desired)
		{
			change = fabs (hook->fov_rate) * change_mult;
			self->client->ps.fov += change;

			if (self->client->ps.fov > hook->fov_desired)
				self->client->ps.fov = hook->fov_desired;
		}
		else
		{
			change = - fabs (hook->fov_rate) * change_mult;
			self->client->ps.fov += change;

			if( self->client->ps.fov > 160 )
			{
				self->client->ps.fov;
			}

			if (self->client->ps.fov < hook->fov_desired)
				self->client->ps.fov = hook->fov_desired;
		}
	}
}
*/
///////////////////////////////////////////////////////////////////////////////
//	Client_ClearForceAngles
//
///////////////////////////////////////////////////////////////////////////////

void	Client_ClearForceAngles (userEntity_t *self)
{
	self->client->ps.pmove.pm_type = PM_NORMAL;
	self->flags -= (self->flags & FL_FORCEANGLES);
}

///////////////////////////////////////////////////////////////////////////////
//	Client_SetForceAngles
//
///////////////////////////////////////////////////////////////////////////////

void	Client_SetForceAngles (userEntity_t *self)
{
	playerHook_t	*hook = (playerHook_t *)self->userHook;

	hook->angles_last_change = gstate->Sys_Milliseconds ();

	self->flags |= FL_FORCEANGLES;
}

///////////////////////////////////////////////////////////////////////////////
//	Client_ChangeAngles
//
//	turns client to look at hook->final_view_angles at hook->final_view_rate
//	if self->flags & FL_FORCEANGLES
///////////////////////////////////////////////////////////////////////////////

void	Client_ChangeAngles (userEntity_t *self)
{
	playerHook_t	*hook = (playerHook_t *)self->userHook;
	int				ms, time;
	CVector			angles;
	float			change_mult, rate, temp;

	if (!(self->flags & FL_FORCEANGLES))
		return;

	ms = gstate->Sys_Milliseconds ();
	time = ms - hook->angles_last_change;
	hook->angles_last_change = ms;

	change_mult = (float) time * 0.001;

	angles = self->client->ps.viewangles;
	angles.x = AngleMod (angles.x);
	angles.y = AngleMod (angles.y);
	angles.z = AngleMod (angles.z);
	
	if (hook->force_rate.x == 0)
	{
		angles.x = hook->force_angles.x;
	}
	else 
	if (angles.x != hook->force_angles.x)
	{
		rate = hook->force_rate.x * change_mult;
		if (rate > 0 && rate < 0.125)
			rate = 0.125;
		else 
		if (rate < 0 && rate > -0.125)
			rate = -0.125;

		temp = angles.x + rate;

		if ((temp <= hook->force_angles.x && angles.x > hook->force_angles.x) ||
			(temp >= hook->force_angles.x && angles.x < hook->force_angles.x))
		{
			angles.x = hook->force_angles.x;
			hook->force_rate.x=0;
		}
		else
		{ 
			angles.x = temp;
		}
	}

	if (hook->force_rate.y == 0)
	{
		angles.y = hook->force_angles.y;
	}
	else 
	if (angles.y != hook->force_angles.y)
	{
		rate = hook->force_rate.y * change_mult;
		if (rate > 0 && rate < 0.125)
			rate = 0.125;
		else 
		if (rate < 0 && rate > -0.125)
			rate = -0.125;

		temp = angles.y + rate;

		if ((temp <= hook->force_angles.y && angles.y > hook->force_angles.y) ||
			(temp >= hook->force_angles.y && angles.y < hook->force_angles.y))
		{
			angles.y = hook->force_angles.y;
			hook->force_rate.y=0;
		}
		else
		{ 
			angles.y = temp;
		}
	}

	if (hook->force_rate.z == 0)
		angles.z = hook->force_angles.z;
	else 
	if (self->flags & FL_INWARP)
		//	spin roll while in a warp
		angles.z += 180 * change_mult;
	else 
	if (angles.z != hook->force_angles.z)
	{
		rate = hook->force_rate.z * change_mult;
		if (rate > 0 && rate < 0.125)
			rate = 0.125;
		else 
		if (rate < 0 && rate > -0.125)
			rate = -0.125;

		temp = angles.z + rate;

		if ((temp <= hook->force_angles.z && angles.z > hook->force_angles.z) ||
			(temp >= hook->force_angles.z && angles.z < hook->force_angles.z))
		{
			angles.z = hook->force_angles.z;
			hook->force_rate.z=0;
		}
		else 
		{
			angles.z = temp;
		}
	}

	gstate->SetClientAngles (self, angles);
}

//---------------------------------------------------------------------------
// Client_SetAutoAim()
//---------------------------------------------------------------------------
void Client_SetAutoAim( userEntity_t *self )
{
    #define AIM_NUMRAYS  5

    userEntity_t *firstEntity=gstate->FirstEntity();
    playerHook_t *phook=(playerHook_t *)self->userHook;

	if( phook == NULL )
	{
		return;
	}

    CVector angles,forward,right,up;
    CVector start,end,shootpos,newstart;

    trace_t trace;
    CVector min,max;
	static CVector min1(4,4,4);
	static CVector max1(-4,-4,-4);
//    short i,numrays,count;
	short numrays;
    float xofs[AIM_NUMRAYS]={0, 1, 0, -1,  0};
    float yofs[AIM_NUMRAYS]={0, 0, 1,  0, -1};
    float aimsize;

    // do autoaim?
    aimsize = gstate->GetCvar("autoaim");
    if ( aimsize )
    {
        numrays = AIM_NUMRAYS;
    }
    else
    {
        numrays = 1;
    }

    // set trace size
/*
    min.Set( -aimsize, -aimsize, -aimsize );
    max.Set( aimsize, aimsize, aimsize );
*/
	min.x = min.y = min.z = (aimsize) ? -10 : 0;
	max.x = max.y = max.z = (aimsize) ? 10 : 0;

	if( self->client )
	{    
		angles = self->client->v_angle;
	}
    angles.AngleToVectors(forward,right,up);

	start = self->s.origin;
	VectorMA(start, forward, 2000, end);
	trace = gstate->TraceBox_q2(start,min,max,end,self,MASK_SHOT);

    if ( (trace.ent != firstEntity) && trace.ent && (trace.ent->s.flags & SFL_TARGETABLE) ) // && !trace.startsolid)
	{
		shootpos = (trace.ent->absmax + trace.ent->absmin) * 0.5;
	}
	else
	{
		trace.ent = NULL;
	}
/*
    for ( i = 0; i < numrays; i++ )
    {
        start = self->s.origin + (xofs[i]*aimsize/2*right) + (yofs[i]*aimsize/2*up) + CVector(0,0,12);
        end = start+forward*2000;

        count = 0;
        newstart = start;
        do 
        {
            // first trace is for monsters (ignores world)
            trace=gstate->TraceBox_q2(newstart,min,max,end,self,CONTENTS_MONSTERCLIP|CONTENTS_MONSTER|CONTENTS_PLAYERCLIP);
            newstart = trace.endpos + forward;
            count++;
        } while ((trace.fraction < 1) && (trace.ent != firstEntity) && (!(trace.ent->flags & SFL_TARGETABLE)) && count<4);

        if (count > 1)
        {
            trace.startsolid=0;
        }

        if ( (trace.ent != firstEntity) && trace.ent && (trace.ent->s.flags & SFL_TARGETABLE) ) // && !trace.startsolid)
        {
            userEntity_t *oldent;

            oldent = (userEntity_t *)trace.ent;
            shootpos = oldent->s.origin + CVector(0,0,12);     //(oldent->s.maxs[1]-oldent->s.mins[1])/2);

            count = 0;
            newstart = start;
            do 
            {
                // second trace is for world, traces to monster with smaller bounding box
                trace=gstate->TraceBox_q2(newstart,min1,max1,shootpos,self,CONTENTS_SOLID|CONTENTS_MONSTERCLIP|CONTENTS_MONSTER|CONTENTS_PLAYERCLIP);
                newstart = trace.endpos + forward;
                count++;
            } while (trace.fraction < 1 && (trace.ent != firstEntity) && (trace.ent != oldent) && (!(trace.ent->flags & SFL_TARGETABLE)) && count<4);

            if (oldent==(userEntity_t *)trace.ent)
            {
                break;
            }
            else
            {
                trace.ent=NULL;
            }   
        }
        else
        {
            trace.ent=NULL;
        }
    }
*/
   // found nothing
   if ( (trace.ent == firstEntity) || !trace.ent ) // || trace.startsolid)
   {
      phook->autoAim.ent=NULL;
      return;
   }

   // found something!
   phook->autoAim.shootpos = shootpos;
   phook->autoAim.ent = trace.ent;
}

///////////////////////////////////////////////////////////////////////////////
//	dll_ClientThink
//
//	This will be called once for each client frame, which will
//	usually be a couple times for each server frame.
//
//	only used here to get player button input
///////////////////////////////////////////////////////////////////////////////

void	dll_ClientThink (userEntity_t *self, usercmd_t *ucmd, pmove_t *pm)
{
	playerHook_t	*ihook = AI_GetPlayerHook( self->input_entity );
	gclient_t		*client;
	//float			r;

	//AI_Dprintf("v_aY:%f f_aY:%f ov_aY:%f\n",self->client->v_angle.yaw,self->client->fix_angles.yaw,self->client->oldviewangles.yaw);
	if( ihook == NULL )
	{
		return;
	}

	// we have a live client that is a deadmonster...if we're in coop, check the bbox and if the box is empty, 
	// go ahead and remove the deadmonster thing.
	if ((self->svflags & SVF_DEADMONSTER) && AI_IsAlive(self) && coop->value)
	{
		trace_t t;
        tr = gstate->TraceBox_q2( self->s.origin, self->s.mins, self->s.maxs, self->s.origin, self, MASK_SHOT );
	    if ( tr.fraction >= 1.0f && !tr.allsolid && !tr.startsolid)
			self->svflags &= ~SVF_DEADMONSTER;
	}

	if (self->client && self->client->pers.nCurrentWeaponFlag && self->curWeapon && !self->client->ps.weapon)
	{
		self->client->pers.nCurrentWeaponFlag = 0;
		weaponInfo_t	*pWeaponInfo;

		if ( pWeaponInfo = com->FindRegisteredWeapon( self->curWeapon->name ) )
		{
			pWeaponInfo->select_func( self );
		}
	}

	if ( (ihook->dflags & DFL_LLAMA) || (ihook->items & IT_MANASKULL) )
	{
		self->s.effects2 |= EF2_INVULNERABLE;
	}
	else
	{
		self->s.effects2 &= ~EF2_INVULNERABLE;
	}

	// SCG[1/5/00]: set cheat flags
	if( SinglePlayerCheat() == FALSE )
	{
		if( gstate->GetCvar( "cheats" ) )
		{
			self->client->ps.rdflags &= ~RDF_NOCHEATING;
		}
		else
		{
			self->client->ps.rdflags |= RDF_NOCHEATING;
		}
	}
	else
	{
		self->client->ps.rdflags &= ~RDF_NOCHEATING;
	}

	if (self->client->do_autosave && !gstate->bCinematicPlaying && !ihook->camera)
	{
		self->client->do_autosave = FALSE;

		if (!deathmatch->value)
			gstate->gi->AutoSave();
	}

    AI_DecreaseJustFired( ihook );

	gstate->level->current_entity = self;
	client = self->client;
/*
	if (gstate->level->intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
		if ( (gstate->level->time > gstate->level->intermissiontime + 5.0
			&& (ucmd->buttons & BUTTON_ANY) ) ||
			(deathmatch->value && (gstate->level->time > gstate->level->intermissiontime + 30)) )
		{
			self->client->showflags &= ~(SHOW_SCORES_NOW|SHOW_SCORES);
			gstate->level->exitintermission = true;
		}

		self->client->ps.rdflags &= ~RDF_LETTERBOX;

		return;
	}
	else */if (ctf->value)
	{
		if (CTFClientThink(self))
			return;
	}
	
	// setup jump flag
	if (pm->cmd.upmove >=10)
	{
		client->ps.pmove.pm_flags |= PMF_JUMP_HELD;
	}
	else
	{
		client->ps.pmove.pm_flags &= ~PMF_JUMP_HELD;
	}

	CLIENT_UpdateFrame( self->input_entity );

	if (self->flags & FL_INWARP)
	{
		Client_WarpThink (self);
//		Client_FOVRamp (self);
		Client_ChangeAngles (self);
	}
	else
	{
//		Client_FOVRamp (self);
		Client_ChangeAngles (self);
        if ( !deathmatch->value )
        {
            Client_SetAutoAim(self);
        }

        if (client->latched_buttons & BUTTON_USE)
        {
			// check for repeated entity-use with the use/operate button held down
			Client_Use(self);
        }

		//////////////////////////////////////////////////////////////////
		//	choose correct animation based on movement
		//////////////////////////////////////////////////////////////////
		client_animation_think (self);
	}

	// check if the side kicks needs to move away
    if ( !deathmatch->value )   // Logic[7/29/99]: no sidekicks in deathmatch games
    {
	    for ( int i = 0; i < pm->numtouch; i++ )
	    {
		    userEntity_t *pEnt = pm->touchents[i];
		    if ( _stricmp( pEnt->className, "SuperFly" ) == 0 || _stricmp( pEnt->className, "Mikiko" ) == 0 )
		    {
			    SIDEKICK_HandleMoveAway( pEnt, self );
		    }
	    }
    }

	// see if we have a taunt to play
	if (deathmatch->value && (ihook->ntauntIndex) && (ihook->ftauntTime) && (gstate->time >= ihook->ftauntTime))
	{
		gstate->StartEntitySound( self, CHAN_VOICE, ihook->ntauntIndex, 1.0, 100, 100 );
		ihook->ntauntIndex = 0;
		ihook->ftauntTime = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	dll_SetStats
//
//	sets stats to be set down for the passed client for display on status bar
//	menus, etc.
///////////////////////////////////////////////////////////////////////////////

long daikatana_exp_levels[] =
{
	0,
	250,
	750,
	1500,
	3000
/*
	0,
	500,
	1000,
	2000,
	4000
*/
};

#define SK_INVULN(other)			( (other->flags & FL_BOT) && (other->takedamage == DAMAGE_NO) )
void SetMikikoStats(userEntity_t *ent, userEntity_t *other)
{
	playerHook_t *sidekick_hook;
	if (ctf->value)
	{
		ent->client->ps.stats[STAT_SIDEKICKS_EXIST] |= SIDEKICK_MIKIKO;
		CTFTeamStats(ent, 1, ent->client->ps.stats[STAT_MIKIKO_HEALTH],
							 ent->client->ps.stats[STAT_MIKIKO_ARMOR],
							 ent->client->ps.stats[STAT_MIKIKO_FLAGS]);
	}
	else if (deathtag->value)
	{
		ent->client->ps.stats[STAT_SIDEKICKS_EXIST] |= SIDEKICK_MIKIKO;
		DT_TeamStats(ent, 1, ent->client->ps.stats[STAT_MIKIKO_HEALTH],
							 ent->client->ps.stats[STAT_MIKIKO_ARMOR],
							 ent->client->ps.stats[STAT_MIKIKO_FLAGS]);
	}
	else
	{
		if (other && other->userHook)
		{
			ent->client->ps.stats[STAT_SIDEKICKS_EXIST] |= SIDEKICK_MIKIKO;
			
			sidekick_hook = AI_GetPlayerHook(other);
			
			// check invisibility
			if (sidekick_hook->items & IT_WRAITHORB)
			{
				ent->client->ps.stats[STAT_MIKIKO_FLAGS] |= STAT_FLAG_INVISIBLE;
			}
			else
			{
				ent->client->ps.stats[STAT_MIKIKO_FLAGS] &= ~STAT_FLAG_INVISIBLE;
			}
						
			// check invulnerability
			if ( SK_INVULN(other) || (sidekick_hook->items & IT_MANASKULL) || (sidekick_hook->dflags & DFL_LLAMA) )
			{
				ent->client->ps.stats[STAT_MIKIKO_FLAGS] |= STAT_FLAG_INVULNERABLE;
			}
			else
			{
				ent->client->ps.stats[STAT_MIKIKO_FLAGS] &= ~STAT_FLAG_INVULNERABLE;
			}

			ent->client->ps.stats[STAT_MIKIKO_HEALTH]  = 100 * (other->health / sidekick_hook->base_health);
			ent->client->ps.stats[STAT_MIKIKO_ARMOR ]  = GetArmorPercentage(other->armor_val, sidekick_hook->items);
		}
		else  // she doesn't exist, so reset
		{
			ent->client->ps.stats[STAT_SIDEKICKS_EXIST] &= ~SIDEKICK_MIKIKO;
			ent->client->ps.stats[STAT_MIKIKO_FLAGS]   = 0;
			ent->client->ps.stats[STAT_MIKIKO_HEALTH]  = 0;
			ent->client->ps.stats[STAT_MIKIKO_ARMOR ]  = 0;
		}
	}
}

void SetSflyStats(userEntity_t *ent, userEntity_t *other)
{
	playerHook_t *sidekick_hook;
	if (ctf->value)
	{
		ent->client->ps.stats[STAT_SIDEKICKS_EXIST] |= SIDEKICK_SUPERFLY;
		CTFTeamStats(ent, 2, ent->client->ps.stats[STAT_SUPERFLY_HEALTH],
							 ent->client->ps.stats[STAT_SUPERFLY_ARMOR],
							 ent->client->ps.stats[STAT_SUPERFLY_FLAGS]);
	}
	else if (deathtag->value)
	{
		ent->client->ps.stats[STAT_SIDEKICKS_EXIST] |= SIDEKICK_SUPERFLY;
		DT_TeamStats(ent, 2, ent->client->ps.stats[STAT_SUPERFLY_HEALTH],
							 ent->client->ps.stats[STAT_SUPERFLY_ARMOR],
							 ent->client->ps.stats[STAT_SUPERFLY_FLAGS]);
	}
	else
	{
		if (other && other->userHook)
		{
			ent->client->ps.stats[STAT_SIDEKICKS_EXIST] |= SIDEKICK_SUPERFLY;
			
			sidekick_hook = AI_GetPlayerHook(other);
			
			// check invisibility
			if (sidekick_hook->items & IT_WRAITHORB)
			{
				ent->client->ps.stats[STAT_SUPERFLY_FLAGS] |= STAT_FLAG_INVISIBLE;
			}
			else
			{
				ent->client->ps.stats[STAT_SUPERFLY_FLAGS] &= ~STAT_FLAG_INVISIBLE;
			}
						
			// check invulnerability
			if ( SK_INVULN(other) || (sidekick_hook->items & IT_MANASKULL) || (sidekick_hook->dflags & DFL_LLAMA) )
			{
				ent->client->ps.stats[STAT_SUPERFLY_FLAGS] |= STAT_FLAG_INVULNERABLE;
			}
			else
			{
				ent->client->ps.stats[STAT_SUPERFLY_FLAGS] &= ~STAT_FLAG_INVULNERABLE;
			}

			ent->client->ps.stats[STAT_SUPERFLY_HEALTH]  = 100 * (other->health / sidekick_hook->base_health);
			ent->client->ps.stats[STAT_SUPERFLY_ARMOR ]  = GetArmorPercentage(other->armor_val, sidekick_hook->items);;
		}
		else // he does'nt exist, so reset
		{
			ent->client->ps.stats[STAT_SIDEKICKS_EXIST] &= ~SIDEKICK_SUPERFLY;
			ent->client->ps.stats[STAT_SUPERFLY_FLAGS]   = 0;
			ent->client->ps.stats[STAT_SUPERFLY_HEALTH]  = 0;
			ent->client->ps.stats[STAT_SUPERFLY_ARMOR ]  = 0;
		}
	}
}

void dll_SetStats( userEntity_t *ent )
{
	userEntity_t	*sidekick_ent;
//	playerHook_t	*sidekick_hook;
	playerHook_t	*hook = AI_GetPlayerHook( ent );
	int				lval;
	
	if( hook == NULL )
	{
		return;
	}
	
	if (ent->curWeapon)
	{
		ent->curWeapon->command(ent->curWeapon,"ammo_count",&lval);
		ent->client->ps.stats[STAT_AMMO] = lval;
	}
	else
	{
		ent->client->ps.stats[STAT_AMMO] = 0;
	}
	
	ent->client->ps.stats[STAT_ARMOR]  = floor(ent->armor_val + 0.5);
	ent->client->ps.stats[STAT_HEALTH] = floor(ent->health + 0.5);

	// this submap's info
	ent->client->ps.stats[STAT_KILLS] = gstate->numMonstersKilled;
	ent->client->ps.stats[STAT_MONSTERS] = gstate->numMonsters;
	ent->client->ps.stats[STAT_FOUND_SECRETS] = gstate->numSecretsFound;
	ent->client->ps.stats[STAT_SECRETS] = gstate->numSecrets;
	if( gstate->level->intermissiontime == 0 )
	{
		ent->client->ps.stats[STAT_TIME] = gstate->level->time;
		ent->client->ps.stats[STAT_TOTAL_TIME] = gstate->level->time;
	}

	// other maps' info
	if (coop->value)
	{
		userEntity_t *hiro,*mikiko,*sfly;
		CoopFindPlayers(&hiro, &mikiko, &sfly, false);
		if (hiro)
		{
			hiro->client->pers.nTotalMonsters[gstate->subMap]	= gstate->numMonsters;
			hiro->client->pers.nTotalKills[gstate->subMap]		= gstate->numMonstersKilled;
			hiro->client->pers.nTotalSecrets[gstate->subMap]	= gstate->numSecrets;
			hiro->client->pers.nFoundSecrets[gstate->subMap]	= gstate->numSecretsFound;
			hiro->client->pers.nTime[gstate->subMap]			= gstate->level->time;

			for (int i = 0; i < MAX_SUBMAPS; i++)
			{
				if (i == gstate->subMap)
					continue;

				ent->client->ps.stats[STAT_KILLS]			+= hiro->client->pers.nTotalKills[i];
				ent->client->ps.stats[STAT_MONSTERS]		+= hiro->client->pers.nTotalMonsters[i];
				ent->client->ps.stats[STAT_FOUND_SECRETS]	+= hiro->client->pers.nFoundSecrets[i];
				ent->client->ps.stats[STAT_SECRETS]			+= hiro->client->pers.nTotalSecrets[i];
				if( gstate->level->intermissiontime == 0 )
				{
					ent->client->ps.stats[STAT_TOTAL_TIME]		+= hiro->client->pers.nTime[i];
				}
			}
		}
	}
	else
	{
		ent->client->pers.nTotalMonsters[gstate->subMap]	= gstate->numMonsters;
		ent->client->pers.nTotalKills[gstate->subMap]		= gstate->numMonstersKilled;
		ent->client->pers.nTotalSecrets[gstate->subMap]		= gstate->numSecrets;
		ent->client->pers.nFoundSecrets[gstate->subMap]		= gstate->numSecretsFound;
		ent->client->pers.nTime[gstate->subMap]				= gstate->level->time;

		for (int i = 0; i < MAX_SUBMAPS; i++)
		{
			if (i == gstate->subMap)
				continue;

			ent->client->ps.stats[STAT_KILLS]			+= ent->client->pers.nTotalKills[i];
			ent->client->ps.stats[STAT_MONSTERS]		+= ent->client->pers.nTotalMonsters[i];
			ent->client->ps.stats[STAT_FOUND_SECRETS]	+= ent->client->pers.nFoundSecrets[i];
			ent->client->ps.stats[STAT_SECRETS]			+= ent->client->pers.nTotalSecrets[i];

			if( gstate->level->intermissiontime == 0 )
			{
				ent->client->ps.stats[STAT_TOTAL_TIME]		+= ent->client->pers.nTime[i];
			}
		}
	}

	// put save gem count into the stats field.
	if (deathmatch->value == 0)
		ent->client->ps.stats[STAT_SAVE_GEMS] = gstate->InventoryItemCount(ent->inventory,"item_savegem");
	else
		ent->client->ps.stats[STAT_SAVE_GEMS] = 0;

	// WAW[11/18/99]: Since we are now using exp in deathmatch.
	lval = ent->record.level;
	if (!deathmatch->value && (lval > ((gstate->episode + 1) * 5)))
		lval = (gstate->episode + 1) * 5;
	ent->client->ps.stats[STAT_LEVEL]  = lval;//ent->record.level;
	ent->client->ps.stats[STAT_EXP]    = ent->record.exp;
	if( deathmatch->value == 0 )
		ent->client->ps.stats[STAT_FRAGS]  = NO_FRAGS_USED;
	else
		ent->client->ps.stats[STAT_FRAGS]  = ent->record.frags;
	
	// if the player has a skill boost affect, use that value
	// else use the base skill level
	
	lval = hook->acro_boost ? hook->acro_boost : hook->base_acro;
	ent->client->ps.stats[STAT_JUMP] = lval;
	
	lval = hook->speed_boost ? hook->speed_boost : hook->base_speed;
	ent->client->ps.stats[STAT_SPEED] = lval;
	
	lval = hook->attack_boost ? hook->attack_boost : hook->base_attack;
	ent->client->ps.stats[STAT_ATTACK] = lval;
	
	lval = hook->power_boost ? hook->power_boost : hook->base_power;
	ent->client->ps.stats[STAT_POWER] = lval;
	
	lval = hook->vita_boost ? hook->vita_boost : hook->base_vita;
	ent->client->ps.stats[STAT_MAX_HEALTH] = lval;
	
	ent->client->ps.stats[STAT_INVISIBLE]      = (hook->items & IT_WRAITHORB);
	ent->client->ps.stats[STAT_INVULNERABLE] = (hook->items & IT_MANASKULL) || (hook->dflags & DFL_LLAMA);
	
	//	set up this to work in deathmatch
	if( (gstate->episode > 1 ) || deathmatch->value )
	{
		int dkExp = ent->record.dkexp;

		short numLevs = sizeof(daikatana_exp_levels)/sizeof(daikatana_exp_levels[0]);
		int i;
		for (i = numLevs-1; i >= 0; i--)
		{
			if (dkExp >= daikatana_exp_levels[i])
				break;
		}

		if (i >= 4)
			ent->client->ps.stats[STAT_SWORD_LEVEL] = 500;
		else
		{
			float levs = (float)(daikatana_exp_levels[i+1] - daikatana_exp_levels[i]);
			float frac = (float)(dkExp - daikatana_exp_levels[i]) / levs;
			ent->client->ps.stats[STAT_SWORD_LEVEL] = 100 * ((float)i + 1.0 + frac);
		}
	}
	else
	{
		ent->client->ps.stats[STAT_SWORD_LEVEL] = -1;
	}

	//	Send info for layouts
	ent->client->ps.stats [STAT_LAYOUTS] = 0;
	
//	if ( (ent->client->showflags & (SHOW_SCORES_NOW | SHOW_SCORES)) && deathmatch->value)
	if( ( ent->client->showflags & ( SHOW_SCORES_NOW | SHOW_SCORES ) ) )
	{
		ent->client->ps.stats [STAT_LAYOUTS] |= LAYOUT_SCOREBOARD;
	}
	
	if (ent->client->showflags & SHOW_ALLHUDS)
	{
		ent->client->ps.stats[STAT_LAYOUTS] |= LAYOUT_SHOWALLHUDS; // show all hud info
	}
	
	
	// update sidekick info for client stats   8.20
	// SCG[10/5/99]: We really don't want to do this if we are in deathmatch mode
	if (coop->value)
	{
		userEntity_t *ent_hiro,*ent_mikiko,*ent_sfly;
		CoopFindPlayers(&ent_hiro, &ent_mikiko, &ent_sfly);

		ent->client->ps.stats[STAT_LAYOUTS] |= LAYOUT_COOP;
		if (!ent_hiro && !ent_mikiko && !ent_sfly)
			return;

		if (ent == ent_hiro)
		{
			SetMikikoStats(ent,ent_mikiko ? ent_mikiko : AIINFO_GetMikiko());
			SetSflyStats(ent,ent_sfly ? ent_sfly : AIINFO_GetSuperfly());
		}
		else if (ent == ent_mikiko)
		{
			SetMikikoStats(ent,ent_hiro ? ent_hiro : NULL);
			SetSflyStats(ent, ent_sfly ? ent_sfly : AIINFO_GetSuperfly());
		}
		else if (ent == ent_sfly)
		{
			SetMikikoStats(ent,ent_mikiko ? ent_mikiko : AIINFO_GetMikiko());
			SetSflyStats(ent, ent_hiro ? ent_hiro : NULL);
		}
	}
	else if (ctf->value)
	{
		ent->client->ps.stats[STAT_LAYOUTS] |= LAYOUT_CTF;
		SetMikikoStats(ent,NULL);
		SetSflyStats(ent,NULL);
	}
	else if (deathtag->value)
	{
		ent->client->ps.stats[STAT_LAYOUTS] |= LAYOUT_DT;
		SetMikikoStats(ent,NULL);
		SetSflyStats(ent,NULL);
	}
	else if( deathmatch->value == 0 )
	{
		// MIKIKO
		SetMikikoStats(ent,AIINFO_GetMikiko());
/*		if ( (sidekick_ent = AIINFO_GetMikiko()) )
		{
			ent->client->ps.stats[STAT_SIDEKICKS_EXIST] |= SIDEKICK_MIKIKO;
			
			sidekick_hook = AI_GetPlayerHook(sidekick_ent);
			
			ent->client->ps.stats[STAT_MIKIKO_HEALTH]  = 100 * (sidekick_ent->health / sidekick_hook->base_health);
			ent->client->ps.stats[STAT_MIKIKO_ARMOR ]  = GetArmorPercentage(sidekick_ent->armor_val, sidekick_hook->items);
		}
		else  // she doesn't exist, so reset
		{
			ent->client->ps.stats[STAT_MIKIKO_HEALTH]  = 0;
			ent->client->ps.stats[STAT_MIKIKO_ARMOR ]  = 0;
		}
*/		
		// SUPERFLY// NSS[12/6/99]:Added Mikikofly
		sidekick_ent = AIINFO_GetSuperfly();
		if (!sidekick_ent)
			sidekick_ent = AIINFO_GetMikikofly();

		SetSflyStats(ent,sidekick_ent);

/*		if ( (sidekick_ent  = AIINFO_GetSuperfly()) || (sidekick_ent = AIINFO_GetMikikofly()))
		{
			ent->client->ps.stats[STAT_SIDEKICKS_EXIST] |= SIDEKICK_SUPERFLY;
			
			sidekick_hook = AI_GetPlayerHook(sidekick_ent);
			
			ent->client->ps.stats[STAT_SUPERFLY_HEALTH]  = 100 * (sidekick_ent->health / sidekick_hook->base_health);
			ent->client->ps.stats[STAT_SUPERFLY_ARMOR ]  = GetArmorPercentage(sidekick_ent->armor_val, sidekick_hook->items);;
		}
		else // he does'nt exist, so reset
		{
			ent->client->ps.stats[STAT_SUPERFLY_HEALTH]  = 0;
			ent->client->ps.stats[STAT_SUPERFLY_ARMOR ]  = 0;
		}
*/
	}
}

/*------------------------------ spawn functions ----------------------------*/

///////////////////////////////////////////////////////////////////////////////
//	info_player_start
//
//	single player spawn point
///////////////////////////////////////////////////////////////////////////////

void info_player_start (userEntity_t *self)
{
	int	i;

	for (i = 0; self->epair [i].key; i++)
	{
		if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;
	}
	
	gstate->Con_Printf ("info_player_start\n");
}

///////////////////////////////////////////////////////////////////////////////
//	info_player_coop
//
//	co-op spawn point
///////////////////////////////////////////////////////////////////////////////

void info_player_coop (userEntity_t *self)
{
	int	i;

	for (i = 0; self->epair [i].key; i++)
	{
		if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	info_player_deathmatch
//
//	deathmatch spawn point
///////////////////////////////////////////////////////////////////////////////

void info_player_deathmatch (userEntity_t *self)
{
	int	i;

	for (i = 0; self->epair [i].key; i++)
	{
		if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	info_player_team1
//
//	ctf team 1 spawn point
///////////////////////////////////////////////////////////////////////////////

void info_player_team1 (userEntity_t *self)
{
	int	i;

	for (i = 0; self->epair [i].key; i++)
	{
		if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	info_player_team2
//
//	ctf team 2 spawn point
///////////////////////////////////////////////////////////////////////////////

void info_player_team2 (userEntity_t *self)
{
	int	i;

	for (i = 0; self->epair [i].key; i++)
	{
		if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	intermission_find_target
//
///////////////////////////////////////////////////////////////////////////////

void	intermission_find_target (userEntity_t *self)
{
	CVector			dir;
	userEntity_t	*target;

	target = com->FindTarget (self->target);

	if (!target)
	{
		self->s.angles = self->movedir;
	}
	else
	{
		//	get vector to target and convert it to angles
		dir - target->s.origin - self->s.origin;
		dir.Normalize();
		VectorToAngles( dir, self->s.angles );
		//VectorSubtract (target->s.origin, self->s.origin, dir);
		//VectorNormalize (dir);
		//VectorToAngles (dir, self->s.angles);
	}

	self->nextthink = -1;
}

///////////////////////////////////////////////////////////////////////////////
//	info_player_intermission
//
///////////////////////////////////////////////////////////////////////////////

void info_player_intermission (userEntity_t *self)
{
	com->SetMovedir (self);

	if (self->target)
	{
		self->think = intermission_find_target;
		self->nextthink = gstate->time + 0.1;
	}
}


// ----------------------------------------------------------------------------
//
// Name:		CLIENT_IssueCommandToSidekicks
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void CLIENT_IssueCommandToSidekicks( userEntity_t *self )
{
	if (coop->value || deathmatch->value)
		return;

	_ASSERTE( self );
	playerHook_t *hook = (playerHook_t *)self->userHook;

    char *szCommand = gstate->GetArgv( 0 );
    if ( _stricmp( szCommand, "superfly" ) == 0 )
    {
        userEntity_t *pSuperfly = AIINFO_GetSuperfly();
        AIINFO_SetLastSelectedSidekick( pSuperfly );
        com->Sidekick_Update(self,TYPE_SUPERFLY,SIDEKICK_STATUS_COMMANDING,0); // update client
    }   
    else
    if ( _stricmp( szCommand, "mikiko" ) == 0 )
    {
        userEntity_t *pMikiko = AIINFO_GetMikiko();
        AIINFO_SetLastSelectedSidekick( pMikiko );
				com->Sidekick_Update(self,TYPE_MIKIKO,SIDEKICK_STATUS_COMMANDING,0); // update client
    }
    else
    {
		if (self->client->buttons & BUTTON_SHIFT)
		{
			userEntity_t *pMikiko = AIINFO_GetMikiko();
			if (pMikiko && AI_IsAlive(pMikiko))
			{
				SIDEKICK_ParseCommand( pMikiko, szCommand );
			}
			userEntity_t *pSuperfly = AIINFO_GetSuperfly();
			if (pSuperfly && AI_IsAlive(pSuperfly))
			{
				SIDEKICK_ParseCommand( pSuperfly, szCommand );
			}
		}
		else
		{
			if ( AIINFO_GetNumSidekicks() > 1 )
			{
				userEntity_t *pLastSelectedSidekick = AIINFO_GetLastSelectedSidekick();

				
				if (!pLastSelectedSidekick && !AI_IsAlive(pLastSelectedSidekick))
				{
  							SIDEKICK_SwapCommanding(self); // as backup, make sure one ONE is selected
								pLastSelectedSidekick = AIINFO_GetLastSelectedSidekick();
							}

				if ( AI_IsAlive( pLastSelectedSidekick ) )
				{
					SIDEKICK_ParseCommand( pLastSelectedSidekick, szCommand );
				}
			}
			else
			{
				userEntity_t *pFirstSidekick = AIINFO_GetFirstSidekick();
				if ( AI_IsAlive( pFirstSidekick ) )
				{
					SIDEKICK_ParseCommand( pFirstSidekick, szCommand );
				}
			}
		}
    }
}

// ----------------------------------------------------------------------------

CVector &CLIENT_GetLastPlayerMovedPosition()
{
    return lastPlayerMovedPosition;
}

// ----------------------------------------------------------------------------
//
// Name: Client_Save
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void Client_Save( FILE *f, struct edict_s *ent )
{
	AI_SavePlayerHook( f, ent );
}

// ----------------------------------------------------------------------------
//
// Name: Client_Load
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void Client_Load( FILE *f, struct edict_s *ent )
{
	AI_LoadPlayerHook( f, ent );

	// SCG[11/21/99]: ent->client->ps.weapon is a pointer that get's saved
	// SCG[11/21/99]: during the client save and should be null when we load
	// SCG[11/21/99]: since weapons will get setup later.
	ent->client->ps.weapon = NULL;

	// SCG[11/21/99]: this clears all of the weapon info (could be the cause
	// SCG[11/21/99]: of the weapon frames getting hosed after a sub-level change )
//	Client_InitWeapons( ent );

    if( !deathmatch->value )
	{
	    Client_InitNodeSystem( ent );
	}

	// SCG[11/21/99]: this clears out the inventory if one exists and allocates a new one
	Client_InitInventory( ent );

		// cek[2-22-00] zap the inventory info in the persistant thingy
	if (ent->client)
	{
		memset(ent->client->pers.inventoryCrap,0,sizeof(ent->client->pers.inventoryCrap));
	}

	Client_InitClientModel( ent );

	// SCG[11/21/99]: inits AI stuff (move speed, attack dist, goalentity, etc.. )
	Client_InitAIDefaults( ent );

	// SCG[12/7/99]: we need to do weapon initialization
	FetchClientEntData( ent );
/*
	// SCG[2/23/00]: restore stats to gstate
	gstate->numMonstersKilled	= ent->client->pers.nTotalKills[gstate->subMap];
	gstate->numSecretsFound		= ent->client->pers.nFoundSecrets[gstate->subMap];
	gstate->level->time			= ent->client->pers.nTime[gstate->subMap];
	gstate->numMonsters			= ent->client->pers.nTotalMonsters[gstate->subMap];
	gstate->numSecrets			= ent->client->pers.nTotalSecrets[gstate->subMap];
*/
	ent->client->pers.pPersSuperfly = ent->client->pers.pPersMikiko = NULL;

	playerHook_t *pHook = AI_GetPlayerHook(ent);
	if (pHook && ent->client)
	{
		if (( pHook->power_boost_time ) || 
			( pHook->attack_boost_time ) || 
			( pHook->speed_boost_time ) || 
			( pHook->acro_boost_time ) || 
			( pHook->vita_boost_time ))

		ent->client->needsUpdate |= HUD_UPDATE_BOOST;
	}

	recalc_level(ent);

	CLIENT_StartStand( ent, TH_STAND );

	alist_add( ent );
	if (coop->value)
	{
		ent->svflags |= SVF_DEADMONSTER;
	}
}
///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void mpath_hook_save( FILE *f, struct edict_s *ent );
void mpath_hook_load( FILE *f, struct edict_s *ent );

void world_client_register_func()
{
	gstate->RegisterFunc("client_warp_appear",client_warp_appear);
	gstate->RegisterFunc("key_twist",key_twist);
	gstate->RegisterFunc("intermission_find_target",intermission_find_target);
	gstate->RegisterFunc("AI_SavePlayerHook",AI_SavePlayerHook);
	gstate->RegisterFunc("AI_LoadPlayerHook",AI_LoadPlayerHook);
	gstate->RegisterFunc("AI_SaveMonsterPlayerHook",AI_SaveMonsterPlayerHook);
	gstate->RegisterFunc("AI_LoadMonsterPlayerHook",AI_LoadMonsterPlayerHook);
	gstate->RegisterFunc("Client_Save",Client_Save);
	gstate->RegisterFunc("Client_Load",Client_Load);
	gstate->RegisterFunc("mpath_hook_save",mpath_hook_save);
	gstate->RegisterFunc("mpath_hook_load",mpath_hook_load);
}
