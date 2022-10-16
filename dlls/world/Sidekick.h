#ifndef _SIDEKICK_H
#define _SIDEKICK_H

#define SIDEKICK_WALKSPEED		200.0f
#define SIDEKICK_RUNSPEED		400.0f

#define HIRO_MODELNAME		"models/global/m_hiro.dkm"
#define MIKIKO_MODELNAME	"models/global/m_mikiko.dkm"
#define SUPERFLY_MODELNAME	"models/global/m_superfly.dkm"

#define HIRO_CSV_NAME		"sounds/hiro/m_hiro.csv"
#define MIKIKO_CSV_NAME		"sounds/mikiko/m_mikiko.csv"
#define SUPERFLY_CSV_NAME	"sounds/superfly/m_superfly.csv"

#define SUPERFLY_CLIENT		2
#define MIKIKO_CLIENT		3



#define ITEMTYPE_ARMOR		0x00000001
#define ITEMTYPE_HEALTH		0x00000002
#define ITEMTYPE_AMMO		0x00000004
#define ITEMTYPE_GOLDENSOUL	0x00000008
#define ITEMTYPE_STATBOOST	0x00000010
#define ITEMTYPE_WEAPONS	0x00000020


//NSS[12/4/99]:Curent things sidekicks can tell one another
#define sCom_Speak		0x00000001	//Tell them to respond to something they say
#define sCom_GetItem	0x00000002	//Tell them an item is in their FOV that they might need.
#define sCom_Synch		0x00000004	//Tell the Sidekick to chill out for a second... the other is doing something.




// NSS[12/5/99]:Message Que flags
#define COM_Unread 1
#define COM_Read   2


enum Sidekick_Talk
{
	NO_TALK,
	HEALTH_RESPONSE1,
	HEALTH_RESPONSE2,
	HEALTH_RESPONSE3,
	HEALTH_RESPONSE4,
	HEALTH_RESPONSE5,
	HIRO_HEALTH_G50,
	HIRO_HEALTH_L50,
	HIRO_HEALTH_L15,
	IMPATIENT,		
	LOOKING_UP,		
	LOOKING_SIDEWAYS,
	HITBY_HIRO,		
	HITBY_SIDEKICK,	
	STARTING_ATTACK,
	NO_AMMO,		
	LOW_HEALTH,		
	START_CROUCH,	
	LOOK_4_HIRO,	
	RUN_AWAY,		
	PICKUP_ITEM,	
	QUESTION_HEALTH,
	QUESTION_1,		
	QUESTION_2,		
	ANSWER_1,		
	ANSWER_2,		
	ANSWER_3,		
	EVADE,			
	JUMP,			
	LANDING,	
	RANDOM_WALK,			// new addition
	PUSHEDBY_HIRO,			// new addition
	HIRO_DIES,			// new addition
	NO,					//General No response.
	CANTREACH,				//No response when can't reach
	SPECIAL_TALK = 98,
	COM_WAIT
};


/*
#define HEALTH_RESPONSE1	1
#define HEALTH_RESPONSE2	2
#define HEALTH_RESPONSE3	3
#define HEALTH_RESPONSE4	4
#define HEALTH_RESPONSE5	5
#define HIRO_HEALTH_G50		6
#define HIRO_HEALTH_L50		7
#define HIRO_HEALTH_L15		8
#define IMPATIENT			9
#define LOOKING_UP			10
#define LOOKING_SIDEWAYS	11
#define HITBY_HIRO			12
#define HITBY_SIDEKICK		13
#define STARTING_ATTACK		14
#define NO_AMMO				15
#define LOW_HEALTH			16
#define START_CROUCH		17
#define LOOK_4_HIRO			18
#define RUN_AWAY			19
#define PICKUP_ITEM			20
#define QUESTION_HEALTH		21
#define QUESTION_1			22
#define QUESTION_2			23
#define ANSWER_1			24
#define ANSWER_2			25
#define ANSWER_3			26
#define ATTACK				27
#define EVADE				28
#define JUMP				29
#define LANDING				30
#define NO					31		//General No response.
#define CANTREACH			32		//No response when can't reach

// NSS[12/5/99]:Sync Commands
#define SPECIAL_TALK		98  // For node speak and sidekicks
#define COM_WAIT			99	//Wait command.
*/






void SIDEKICK_SetGodMode();
void SIDEKICK_ClearGodMode();
int SIDEKICK_IsInGodMode();
void SIDEKICK_ToggleGodMode();

void SIDEKICK_TrackOwner( userEntity_t *self );

int SIDEKICK_DetermineFollowing( userEntity_t *self );
userEntity_t *SIDEKICK_GetOtherSidekick( userEntity_t *self );

void SIDEKICK_SelectAnimationPerWeaponType( userEntity_t *self, char *pszAnimation );
void SIDEKICK_PlaySound( userEntity_t *self, const char *soundfile );
void SIDEKICK_ChooseBestWeapon( userEntity_t *self );

void SIDEKICK_FindOwner( userEntity_t *self );

userEntity_t *SIDEKICK_SpawnHiro( userEntity_t *self );
userEntity_t *SIDEKICK_SpawnSuperfly( userEntity_t *self );
userEntity_t *SIDEKICK_SpawnMikikoFly( userEntity_t *self );
userEntity_t *SIDEKICK_SpawnMikiko( userEntity_t *self );
void SIDEKICK_SpawnHiro2( userEntity_t *self );
void SIDEKICK_SpawnSuperfly2( userEntity_t *self );
void SIDEKICK_SpawnMikiko2( userEntity_t *self );
void SIDEKICK_SpawnMikikoFly2( userEntity_t *self );


// NSS[2/15/00]:
void SIDEKICK_TeleportMikiko( userEntity_t *self );
void SIDEKICK_TeleportMikikoFly( userEntity_t *self );
void SIDEKICK_TeleportSuperfly( userEntity_t *self );

userEntity_t *SIDEKICK_SpawnHiro( const CVector &origin, const CVector &angle );
userEntity_t *SIDEKICK_SpawnSuperfly( const CVector &origin, const CVector &angle );
userEntity_t *SIDEKICK_SpawnMikikoFly( const CVector &origin, const CVector &angle );
userEntity_t *SIDEKICK_SpawnMikiko( const CVector &origin, const CVector &angle );

void SIDEKICK_ParseCommand( userEntity_t *self, char *str );
int SIDEKICK_HandlePickupItems( userEntity_t *self );

void SIDEKICK_ResetAmbientTimes( userEntity_t *self );
void SIDEKICK_HandleIdle( userEntity_t *self );
int SIDEKICK_FindNearUnobstructedPoint( userEntity_t *self, CVector &point );
void SIDEKICK_FaceTowardPoint( userEntity_t *self, CVector &destPoint );
void SIDEKICK_HandleMoveAway( userEntity_t *self, userEntity_t *pPlayer );

int SIDEKICK_IsOwnerClose( userEntity_t *self );
int SIDEKICK_CanPickupItem( userEntity_t *self, userEntity_t *pItem, int bForcePickup = FALSE );

int SIDEKICK_IsOkToFight( userEntity_t *self );

int SIDEKICK_HandleAmbients( userEntity_t *self );

int SIDEKICK_DoStopGoingFurther( userEntity_t *self, CVector &position, 
                                 char *szSound, char *szAnimation, int nOrder /* = 1 */ );
int SIDEKICK_DoStopGoingFurther( userEntity_t *self, MAPNODE_PTR pCurrentNode, int nOrder = 1 );
int SIDEKICK_DoTeleportAndComeNear( userEntity_t *self, CVector &position, 
                                    char *szSound, char *szAnimation, int nOrder /* = 1 */ );
int SIDEKICK_DoTeleportAndComeNear( userEntity_t *self, MAPNODE_PTR pCurrentNode, int nOrder = 1 );

int SIDEKICK_HandleSuperflyTalk( userEntity_t *self, MAPNODE_PTR pCurrentNode );
int SIDEKICK_HandleMikikoTalk( userEntity_t *self, MAPNODE_PTR pCurrentNode );



void SIDEKICK_TestAttack( userEntity_t *self );
void SIDEKICK_TestAttackPlayer( userEntity_t *self );


void SIDEKICK_SwapCommanding( userEntity_t *self );
void SIDEKICK_DisableCommanding( userEntity_t *self );

void SIDEKICK_ShutdownMikiko();
void SIDEKICK_ShutdownSuperfly();

int SIDEKICK_StartAnimation(userEntity_t *self, char *Buff);
int SIDEKICK_WhoIsClosest(userEntity_t *self,MAPNODE_PTR pCurrentNode);
int SIDEKICK_HasTaskInQue( userEntity_t *self, TASKTYPE SearchTask );

int SIDEKICK_FindClearSpotFrom( userEntity_t *self,CVector &fromPoint, CVector &clearPoint);
// NSS[3/6/00]:
void SIDEKICK_Stats_SF(userEntity_t *self);
void SIDEKICK_Stats_MK(userEntity_t *self);
//NSS[12/4/99]:Created so I can have some form of communication between sidekicks.
typedef struct 
{
	CVector			Location;		//Vector information holder
	unsigned long	flags;			//Flags for internal keepings
	int				command;		//Command
	int				aInt;			//Int value for anything
	float			aFloat;			//Float value for anything (being used mainly for time to live
	float			bFloat;			//Yet another generic float value to be used.
	float			TimeStamp;		//When the message was sent.

} SideKickCom_t;


//NSS[12/4/99]:Created to cross reference sounds
typedef struct
{	
	float delay;						//Delay until you say another sound.
	float last_time_used;				//Which one is talking
	enum Sidekick_Talk SoundID;			//Sound Identifier (can have multiple sounds with the same identifier)
	char File[48];
} SideMp3_t;







#endif _SIDEKICK_H
