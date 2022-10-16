
#ifndef _AI_COMMON_H
#define _AI_COMMON_H

class CAction;

///////////////////////////////////////////////////////////////////////////////////////////////////// 
//	ai.h
//
//	defines and structures used by all ai_ files
/////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_CLOSEST_HIDENODES	4
#define MAX_CLOSEST_SNIPENODES	4

////////////////////////////////////////////////////////////////////////////
//		attack mode for specific monster attacks
////////////////////////////////////////////////////////////////////////////
#define	SPECIFICATTACKMODE_NONE			0
#define	SPECIFICATTACKMODE_STARTJUMP	1
#define	SPECIFICATTACKMODE_JUMP			2
#define	SPECIFICATTACKMODE_LAND			3
#define	SPECIFICATTACKMODE_PUSHOFF		4

////////////////////////////////////////////////////////////////////////////
//		current moving environment
////////////////////////////////////////////////////////////////////////////
#define MOVING_GROUND			0
#define MOVING_WATER			1
#define MOVING_AIR				2

////////////////////////////////////////////////////////////////////////////
//	Monster Spawn flags
////////////////////////////////////////////////////////////////////////////
#define SPAWN_NONE				0x00000000
#define SPAWN_WANDER			0x00000001
#define SPAWN_PATHFOLLOW		0x00000002
#define SPAWN_NODE_WANDER		0x00000004
#define SPAWN_RANDOM_WANDER		0x00000008
#define SPAWN_IGNORE_PLAYER		0x00000010
#define SPAWN_SNIPE				0x00000020
#define SPAWN_DO_NOT_FLY		0x00000040
#define SPAWN_DO_NOT_MOVE		0x00000080
#define SPAWN_DEATHTARGET		0x00000100
#define SPAWN_TAKECOVER			0x00000200
#define SPAWN_ALWAYSGIB         0x00000400
// the last flag IONRadiant has a checkbox for (!!!) -- zjb
#define SPAWN_CEILING         0x00000800

////////////////////////////////////////////////////////////////////////////
// Use abilities flags
////////////////////////////////////////////////////////////////////////////
#define CANUSE_NONE			0x00000000
#define CANUSE_DOOR			0x00000001
#define CANUSE_ELEVATOR		0x00000002
#define CANUSE_TRAIN		0x00000004
#define CANUSE_PLATFORM		0x00000008
#define CANUSE_LADDER		0x00000010
#define CANUSE_ALL			0xffffffff

//////////////////////////////////////////////////////////////////////////////
////	NODE definitions
//////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////
//	these flags modify think behaviour
//
//	multiple flags can be set at once
/////////////////////////////////////////////////////////////////////////////////////////////////////

#define	AI_CONTINUE			0x00000001		//	go to continue_think when finished with current action
#define AI_SPECIAL			0x00000002		//	go to special_think when finished with current action
#define	AI_END_ATTACK		0x00000004		//	attack needs to be aborted at end of sequence for some reason
#define	AI_JUMP				0x00000008		//	set when an entity jumps.  when set an entities think will not be
											//	reset because it is assumed that it has been set in ai_jump
#define	AI_TURN				0x00000010		//	set when an entity turns.  when set an entities think will not be
											//	reset because it is assumed that it has been set in ai_turn
#define	AI_ACTION			0x00000020		//	performing a monster_path_corner_action
#define AI_IGNORE_PLAYER	0x00000040		//  ignore player, unless attacked
#define AI_SNIPE			0x00000080		//  this monster is currently in a sniping mode
#define	AI_CANDODGE			0x00000100		//  this monster is able to dodge
#define AI_ALWAYS_USENODES	0x00000200		//  this monster will move using nodes, even if he can see the enemy


// player commands to sidekicks
#define AI_ATTACK           0x00000400		
#define AI_NOATTACK         0x00000800
#define AI_STAY             0x00001000
#define AI_FOLLOW           0x00002000
#define AI_COMEHERE         0x00004000
#define AI_SEENPLAYER		0x00008000		// Used for the wander stuff after the player is dead.
#define AI_DROWNING			0x00010000		// Used to determine if the AI is drowning or not.
#define AI_CANSWIM			0x00020000		// USed to let monsters who are not amphibious actually try to chase after the player in water
#define AI_ISBURNING		0x00040000		// Flag to determine if the creature is on fire or not.
#define AI_SIDEKICKKILL		0x00080000		// NSS[2/4/00]:When told to attack something if you are pointing at something then they attack it until it is dead or they are.

#define	REACHED_GOAL_DIST	32.0

#define STRAFE_LEFT			0
#define STRAFE_RIGHT		1
#define STRAFE_UPLEFT		2
#define STRAFE_UPRIGHT		3
#define STRAFE_DOWNLEFT		4
#define STRAFE_DOWNRIGHT	5

//#define	STRAFE_RIGHT	1
//#define	STRAFE_LEFT 	2

#define	TURN_RIGHT		1
#define	TURN_LEFT		2


//-----------------------------------------------------------------------------


typedef enum enum_TASKTYPE
{
	TASKTYPE_NONE,
    TASKTYPE_IDLE,
	TASKTYPE_WANDER,
	TASKTYPE_ATTACK,
	TASKTYPE_MOVE,
	TASKTYPE_CHASE,
	TASKTYPE_CHASEATTACK,
	TASKTYPE_CHASEATTACKSTRAFING,
	TASKTYPE_FOLLOWWALKING,
	TASKTYPE_FOLLOWRUNNING,
    TASKTYPE_FOLLOWSIDEKICKWALKING,
    TASKTYPE_FOLLOWSIDEKICKRUNNING,
	TASKTYPE_JUMPUP,
	TASKTYPE_JUMPFORWARD,
	TASKTYPE_SWITCH,
	TASKTYPE_HIDE,
	TASKTYPE_SNIPE,
	TASKTYPE_AMBUSH,
	TASKTYPE_RUNAWAY,
    TASKTYPE_BRIEFCOVER,
	TASKTYPE_MOVEAWAY,
	TASKTYPE_WAITFORTRAINTOCOME,
	TASKTYPE_WAITFORTRAINTOSTOP,
	TASKTYPE_WAITFORPLATFORMTOCOME,
	TASKTYPE_WAITFORPLATFORMTOSTOP,
	TASKTYPE_WAITFORDOORTOOPEN,
	TASKTYPE_WAITFORDOORTOCLOSE,
	TASKTYPE_MOVEINTOPLATFORM,
	TASKTYPE_MOVEOUTOFPLATFORM,
	TASKTYPE_PATROL,
	TASKTYPE_PAIN,
	TASKTYPE_DIE,
	TASKTYPE_FREEZE,
	TASKTYPE_MOVETOLOCATION,
	TASKTYPE_MOVETOEXACTLOCATION,
    TASKTYPE_WALKTOLOCATION,
	TASKTYPE_GOINGAROUNDOBSTACLE,
	TASKTYPE_FLYTOLOCATION,
	TASKTYPE_SWOOPATTACK,
	TASKTYPE_TOWARDINTERMEDIATEPOINT,
	TASKTYPE_AIRRANGEDATTACK,
	TASKTYPE_AIRMELEEATTACK,
	TASKTYPE_SWIMTOLOCATION,
	TASKTYPE_FACEANGLE,
	TASKTYPE_WAIT,
	TASKTYPE_WAITFORNOCOLLISION,
	TASKTYPE_BACKUPALLATTRIBUTES,
	TASKTYPE_RESTOREALLATTRIBUTES,
	TASKTYPE_MODIFYTURNATTRIBUTE,
	TASKTYPE_MODIFYRUNATTRIBUTE,
	TASKTYPE_MODIFYWALKATTRIBUTE,

    TASKTYPE_PLAYANIMATION,
    TASKTYPE_SETIDLEANIMATION,

	TASKTYPE_RANDOMWANDER,

	TASKTYPE_STARTUSINGWALKSPEED,
	TASKTYPE_STARTUSINGRUNSPEED,

	TASKTYPE_ACTIVATESWITCH,
	TASKTYPE_MOVETOENTITY,
    TASKTYPE_MOVETOOWNER,
	TASKTYPE_STAND,
	TASKTYPE_TAKECOVER,
	TASKTYPE_TAKECOVER_ATTACK,

	TASKTYPE_STAY,
	TASKTYPE_FLYINGAROUNDOBSTACLE,
	TASKTYPE_PATHFOLLOW,
	TASKTYPE_DODGE,
	TASKTYPE_SIDESTEP,

	TASKTYPE_GOINGAROUNDENTITY,
	TASKTYPE_MOVETOSNIPE,
	TASKTYPE_STRAFE,
	TASKTYPE_CHASEEVADE,			// try to zig zag to avoid enemy's fire
	TASKTYPE_TAKEOFF,
	TASKTYPE_DROP,
	TASKTYPE_TOWARDINTERMEDIATEPOINT2,

	TASKTYPE_CHASESIDESTEPLEFT,
	TASKTYPE_CHASESIDESTEPRIGHT,
	TASKTYPE_CHARGETOWARDENEMY,
	TASKTYPE_RETREATTOOWNER,
	TASKTYPE_SEQUENCETRANSITION,
	TASKTYPE_FINISHCURRENTSEQUENCE,
	TASKTYPE_JUMPTOWARDPOINT,
	TASKTYPE_SHOTCYCLERJUMP,
	TASKTYPE_MOVEUNTILVISIBLE,
	TASKTYPE_USEPLATFORM,
	TASKTYPE_MOVEDOWN,
	TASKTYPE_USE,
	TASKTYPE_COWER,
	TASKTYPE_UPLADDER,
	TASKTYPE_DOWNLADDER,
	TASKTYPE_WAITONLADDERFORNOCOLLISION,

	TASKTYPE_PLAYSOUNDTOEND,
	TASKTYPE_TELEPORT,
    TASKTYPE_PLAYANIMATIONANDSOUND,
    TASKTYPE_PLAYSOUND,
    TASKTYPE_MOVETOENTITYUNTILVISIBLE,
    
    TASKTYPE_ACTION_PLAYANIMATION,
	TASKTYPE_ACTION_PLAYPARTIALANIMATION,
	TASKTYPE_ACTION_PLAYSOUND,
	TASKTYPE_ACTION_SPAWN,
	TASKTYPE_ACTION_SETMOVINGANIMATION,
	TASKTYPE_ACTION_SETSTATE,
	TASKTYPE_ACTION_DIE,
	TASKTYPE_ACTION_STREAMSOUND,
	TASKTYPE_ACTION_SENDMESSAGE,
	TASKTYPE_ACTION_SENDURGENTMESSAGE,
	TASKTYPE_ACTION_CALL,
	TASKTYPE_ACTION_RANDOMSCRIPT,
	TASKTYPE_ACTION_COMENEAR,
	TASKTYPE_ACTION_REMOVE,
	TASKTYPE_ACTION_LOOKAT,
	TASKTYPE_ACTION_STOPLOOK,
    TASKTYPE_ACTION_ATTACK,
    TASKTYPE_ACTION_PRINT,

    TASKTYPE_WAITUNTILNOOBSTRUCTION,
    TASKTYPE_MOVETORETREATNODE,
    TASKTYPE_STOPENTITY,

	// monster specific ai should be placed after this
	TASKTYPE_SPECIAL,

	TASKTYPE_INMATER_GOTOPRISONER,			// inmater specific tasks
	TASKTYPE_INMATER_KILLPRISONER,
	TASKTYPE_INMATER_WAITATPRISONER,

	TASKTYPE_PRISONER_WANDER,
	TASKTYPE_PRISONER_COWER,
//	TASKTYPE_PRISONER_INMATERHASITINME,

	TASKTYPE_SLUDGEMINION_SCOOP,
	TASKTYPE_SLUDGEMINION_DUMP,

	TASKTYPE_BUBOID_GETOUTOFCOFFIN,
	TASKTYPE_BUBOID_MELT,
	TASKTYPE_BUBOID_MELTED,
	TASKTYPE_BUBOID_UNMELT,

	TASKTYPE_ROTWORM_JUMPSPRING,
	TASKTYPE_ROTWORM_JUMPFLY,
	TASKTYPE_ROTWORM_JUMPBITE,

	TASKTYPE_CAMBOT_PATHFOLLOW,
	TASKTYPE_CAMBOT_FOLLOWPLAYER,

//	TASKTYPE_LABWORKER_WORK,

	//////////////////////////////
	//     WYNDRAX  TASKS      //
	//////////////////////////////
	TASKTYPE_WYNDRAX_FIND_WISPMASTER,	//<nss>Find main wisp entity
	TASKTYPE_WYNDRAX_COLLECT_WISPS,		//<nss>Collecte wipss once at entity
	TASKTYPE_WYNDRAX_CHARGE_UP,			//<nss>Charge our ass up !
	
	//////////////////////////////
	//     CHAINGANG TASKS      //
	//////////////////////////////
	TASKTYPE_CHAINGANG_ATTACK_THINK,//<nss> Main Attack State change function
	TASKTYPE_CHAINGANG_ATTACK,		//<nss>	Attack sequence for both tongue and jaws
	TASKTYPE_CHAINGANG_CHASE,		//<nss>	Chase the player down!
	TASKTYPE_CHAINGANG_GROUND,		//<nss> Land the Griffon(find landing spot and get within 'x' distance)
	TASKTYPE_CHAINGANG_AIR,			//<nss> Take off sequence which places the monster into the air
	TASKTYPE_CHAINGANG_INITIATE_AIR,//<nss> Initiate the Air sequence.
	TASKTYPE_CHAINGANG_INITIATE_GROUND,//<nss> Initiate the Ground sequence.
	TASKTYPE_CHAINGANG_MOVETOLOCATION,//<nss> 
	TASKTYPE_CHAINGANG_DODGE,		//<nss>Sets up a decent dodge to point
	
	//////////////////////////////
	//        FROG TASKS        //
	//////////////////////////////
	TASKTYPE_FROG_ATTACK_THINK,		//<nss> Main Attack State change function
	TASKTYPE_FROG_ATTACK,			//<nss>	Attack sequence for both tongue and jaws
	TASKTYPE_FROG_JUMP,				//<nss>	Jump at Player!
	TASKTYPE_FROG_CHASE,			//<nss>	Chase the player down!

	//////////////////////////////
	//      GRIFFON TASKS       //
	//////////////////////////////
	TASKTYPE_GRIFFON_ATTACK_THINK,	//<nss> Main Attack State change function
	TASKTYPE_GRIFFON_CHASE,			//<nss>	Chase down the player!
	TASKTYPE_GRIFFON_ATTACK,		//<nss>	Attack sequence for both ground and air
	TASKTYPE_GRIFFON_JUMP,			//<nss>	Jump at Player!
	TASKTYPE_GRIFFON_FLYAWAY,		//<nss>	Generic Gly Away sequence
	TASKTYPE_GRIFFON_GROUND,		//<nss>	Land the Griffon
	TASKTYPE_GRIFFON_FLY,			//<nss>	Griffon will take off and fly
	TASKTYPE_GRIFFON_LAND,			//<nss> Landing Sequence
	
	//////////////////////////////
	//      DOOMBAT TASKS       //
	//////////////////////////////
	TASKTYPE_DOOMBAT_HOVER,			//<nss>	Hover before shooting fireball
	TASKTYPE_DOOMBAT_CHASE,			//<nss>	Chase down the player!
	TASKTYPE_DOOMBAT_ATTACK_THINK,	//<nss> Main Attack State change function
	TASKTYPE_DOOMBAT_KAMAKAZI,		//<nss> Bust open on the player's skull
	TASKTYPE_DOOMBAT_ATTACK,		//<nss> Claw or Shoot
	TASKTYPE_DOOMBAT_FLYAWAY,		//<nss> Get out of there!

	//////////////////////////////
	//      GHOST TASKS         //
	//////////////////////////////
	TASKTYPE_GHOST_WAKEUP,			//<nss>	WAKEUP Mr. Ghost man!
	TASKTYPE_GHOST_SPIRAL,			//<nss>	Spiral out and up
	TASKTYPE_GHOST_DEATH,			//<nss>	Fade out 
	TASKTYPE_GHOST_ATTACK,			//<nss> Hack and Slash
	TASKTYPE_GHOST_CHASE,			//<nss> Wow.. like uh.. less than 4 days to master and STILL adding.. .uuugh.

	//////////////////////////////
	//      DRAGON TASKS        //
	//////////////////////////////
	TASKTYPE_DRAGON_HOVER,			//<nss>	Hover before shooting fireball
	TASKTYPE_DRAGON_ATTACK_THINK,	//<nss> Main Attack State change function
	TASKTYPE_DRAGON_ATTACK,			//<nss> Shoot the fireball 
	TASKTYPE_DRAGON_FLYAWAY,		//<nss> Fly away to give the player a chance...

	//////////////////////////////
	//      HARPY TASKS         //
	//////////////////////////////
	TASKTYPE_HARPY_ATTACK_THINK,	// Main Attack State change function
	TASKTYPE_HARPY_ATTACK,			// Attack sequence for both tongue and jaws
	TASKTYPE_HARPY_CHASE,			// Chase the player down!
	TASKTYPE_HARPY_GROUND,			// Land the Griffon(find landing spot and get within 'x' distance)
	TASKTYPE_HARPY_AIR,				// Take off sequence which places the monster into the air
	TASKTYPE_HARPY_INITIATE_AIR,	// Initiate the Air sequence.
	TASKTYPE_HARPY_INITIATE_GROUND,	// Initiate the Ground sequence.
	TASKTYPE_HARPY_MOVETOLOCATION,
	TASKTYPE_HARPY_DODGE,			// Sets up a decent dodge to point

	////////////////////////////////
	//       MEDUSA TASKS         //
	////////////////////////////////
	TASKTYPE_MEDUSA_CHASE,      //<zjb> chase, but retreat to a retreat node if there's no way
	TASKTYPE_MEDUSA_RATTLE,     //<zjb> rattle before gazing
	TASKTYPE_MEDUSA_GAZE,       //<zjb> gaze at player
	TASKTYPE_MEDUSA_STOPGAZING, //<zjb> stop gazing at player

	////////////////////////////////
	//    THUNDERSKEET TASKS      //
	////////////////////////////////
	TASKTYPE_THUNDERSKEET_HOVER,		//NSS[10/26/99]:Generic tasks for Thunderskeet
	TASKTYPE_THUNDERSKEET_DARTTOWARDENEMY,
	TASKTYPE_THUNDERSKEET_FLYAWAY,
	TASKTYPE_THUNDERSKEET_ATTACK,
	
	////////////////////////////////
	//       SKEETER TASKS        //
	////////////////////////////////
	TASKTYPE_SKEETER_HOVER,				//NSS[10/26/99]: Generic tasks for Skeeter
	TASKTYPE_SKEETER_DARTTOWARDENEMY,
	TASKTYPE_SKEETER_FLYAWAY,
	TASKTYPE_SKEETER_PREHATCH,
	TASKTYPE_SKEETER_HATCH,
	TASKTYPE_SKEETER_ATTACK,

	//////////////////////////////
	//     LYCANTHIR TASKS      //
	//////////////////////////////
	TASKTYPE_LYCANTHIR_RESURRECT_WAIT, //<zjb> wait for time to pass before resurrection
	TASKTYPE_LYCANTHIR_RESURRECT,      //<zjb> come to life
	TASKTYPE_BUBOID_RESURRECT_WAIT,
	TASKTYPE_BUBOID_RESURRECT, 

	//////////////////////////////
	//     NHARRE TASKS		    //
	//////////////////////////////
	TASKTYPE_NHARRE_TELEPORT,			// NSS[12/13/99]:Added the teleport task for Nharre
	
	//NSS[11/1/99]:Added moveto
	TASKTYPE_DEATHSPHERE_CHARGEWEAPON,
	TASKTYPE_DEATHSPHERE_MOVETOLOCATION,

	TASKTYPE_PROTOPOD_WAIT_TO_HATCH,
	TASKTYPE_PROTOPOD_HATCH,

	TASKTYPE_COLUMN_AWAKEN_WAIT,
	TASKTYPE_COLUMN_AWAKEN,

	TASKTYPE_SKINNYWORKER_HIDE,
	TASKTYPE_SKINNYWORKER_BACKWALL,			// backup against the wall
	TASKTYPE_SKINNYWORKER_LOOKOUT,			// look out for the enemy
	
    TASKTYPE_MISHIMAGUARD_RELOAD,			// NSS
	TASKTYPE_PSYCLAW_JUMPUP,				// NSS[2/25/00]:

    TASKTYPE_SMALLSPIDER_RUNAWAY,

	// bot specific tasks
	TASKTYPE_BOT_DIE,
	TASKTYPE_BOT_ROAM,
	TASKTYPE_BOT_PICKUPITEM,
	TASKTYPE_BOT_ENGAGEENEMY,
	TASKTYPE_BOT_CHASEATTACK,
	TASKTYPE_BOT_STRAFELEFT,
	TASKTYPE_BOT_STRAFERIGHT,
	TASKTYPE_BOT_CHARGETOWARDENEMY,

	////////////////////////////////
	//      SIDEKICK TASKS        //
	////////////////////////////////

	TASKTYPE_SIDEKICK_DIE,
    TASKTYPE_SIDEKICK_PICKUPITEM,
    TASKTYPE_SIDEKICK_EVADE,

    TASKTYPE_SIDEKICK_PROCESS,				// NSS[12/13/99]:This task checks the environment for baddies etc..
	TASKTYPE_SIDEKICK_STOP,					// NSS[12/13/99]:Just freakin' stop ok?

	TASKTYPE_SIDEKICK_RANDOMWALK,
    TASKTYPE_SIDEKICK_LOOKAROUND,
    TASKTYPE_SIDEKICK_LOOKUP,
    TASKTYPE_SIDEKICK_LOOKDOWN,
    TASKTYPE_SIDEKICK_SAYSOMETHING,
    TASKTYPE_SIDEKICK_KICKSOMETHING,
    TASKTYPE_SIDEKICK_WHISTLE,
    TASKTYPE_SIDEKICK_ANIMATE,
	
	
	TASKTYPE_SIDEKICK_ATTACK,				// NSS[12/13/99]:

    TASKTYPE_SIDEKICK_STOPGOINGFURTHER,
    TASKTYPE_SIDEKICK_TELEPORTANDCOMENEAR,

    TASKTYPE_SIDEKICK_COMEHERE,
    TASKTYPE_SIDEKICK_TELEPORT,

	// NSS[1/15/00]:All of Kage's new attacks
	TASKTYPE_KAGE_CHARGEHEALTH,
	TASKTYPE_KAGE_CREATEPROTECTORS,
	TASKTYPE_KAGE_SMOKESCREEN,
	TASKTYPE_KAGE_WAIT,
	TASKTYPE_KAGE_RETURN,

	// test tasks
	TASKTYPE_TESTMAP_ROAM,

    TASKTYPE_MAX,

} TASKTYPE;

//	NOTE!!	If you add a GOALTYPE here, also add the string equivalent for it
//			to GOALTYPE_STR below!!!

typedef enum enum_GOALTYPE
{
	GOALTYPE_NONE,
    GOALTYPE_IDLE,
	GOALTYPE_KILLENEMY,
	GOALTYPE_WANDER,
	GOALTYPE_PATROL,
	GOALTYPE_HIDE,
	GOALTYPE_PICKUPITEM,
	GOALTYPE_MOVETOLOCATION,
	GOALTYPE_AMBUSH,
	GOALTYPE_RUNAWAY,
	GOALTYPE_SNIPE,
	GOALTYPE_FOLLOW,
	GOALTYPE_FLYTOLOCATION,
	GOALTYPE_SWIMTOLOCATION,
	GOALTYPE_FACEANGLE,
	GOALTYPE_WAIT,
	GOALTYPE_BACKUPALLATTRIBUTES,
	GOALTYPE_RESTOREALLATTRIBUTES,
	GOALTYPE_MODIFYTURNATTRIBUTE,
	GOALTYPE_MODIFYRUNATTRIBUTE,
	GOALTYPE_MODIFYWALKATTRIBUTE,

    GOALTYPE_PLAYANIMATION,
    GOALTYPE_SETIDLEANIMATION,

	GOALTYPE_RANDOMWANDER,

	GOALTYPE_STARTUSINGWALKSPEED,
	GOALTYPE_STARTUSINGRUNSPEED,
	GOALTYPE_MOVETOENTITY,
    GOALTYPE_MOVETOEXACTLOCATION,

	GOALTYPE_STAY,				//	for sidekicks: wait at current location until given a new follow mode
	GOALTYPE_PATHFOLLOW,
	GOALTYPE_TAKECOVER,
	GOALTYPE_SCRIPTACTION,
	GOALTYPE_WHENUSED_SCRIPTACTION,
	GOALTYPE_USE,

	GOALTYPE_GENERALUSE,

    GOALTYPE_PLAYSOUNDTOEND,

	// monster specific goal should be placed here
	GOALTYPE_INMATER_KILLPRISONER,

	GOALTYPE_PRISONER_WANDER,

	GOALTYPE_COLUMN_AWAKEN,

	GOALTYPE_BUBOID_GETOUTOFCOFFIN,
	GOALTYPE_BUBOID_MELT,

	GOALTYPE_ROTWORM_DROPFROMCEILING,
	GOALTYPE_ROTWORM_JUMPATTACK,

	GOALTYPE_CAMBOT_PATHFOLLOW,
	GOALTYPE_CAMBOT_FOLLOWPLAYER,

	GOALTYPE_PROTOPOD_HATCH,

	GOALTYPE_MEDUSA_GAZE,

	GOALTYPE_SKEETER_HATCH,

	GOALTYPE_LYCANTHIR_RESURRECT, // Play dead! Good doggie!
	GOALTYPE_BUBOID_RESURRECT,
	
    //GOALTYPE_SKINNYWORKER_WORK_A,
    GOALTYPE_SKINNYWORKER_HIDE,

	// sidekick specific goals should be placed here
	GOALTYPE_SIDEKICK_WAITHERE,
	GOALTYPE_SIDEKICK_FOLLOWOWNER,
	GOALTYPE_SIDEKICK_ATTACK,
    GOALTYPE_SIDEKICK_STOPGOINGFURTHER,
    GOALTYPE_SIDEKICK_TELEPORTANDCOMENEAR,

	GOALTYPE_WYNDRAX_RECHARGE,		//<nss> Wyndrax need's more wisp souls to suck down... 
	GOALTYPE_WYNDRAX_POWERUP,		//<nss> Wyndrax need's lightning charge... 
	// bot specific goals
	GOALTYPE_BOT_ROAM,
	GOALTYPE_BOT_PICKUPITEM,
	GOALTYPE_BOT_ENGAGEENEMY,

	GOALTYPE_KAGE_SPARKUP,			// NSS[1/15/00]: Charge up and send out the protectors
	GOALTYPE_KAGE_YINYANG,			// NSS[1/15/00]: Frontal flip on the players forehead.

	// test goals
	GOALTYPE_TESTMAP_ROAM,

} GOALTYPE;


extern	char	*GOALTYPE_STR [];

//-----------------------------------------------------------------------------

// 8 bytes
typedef struct aiDataAnimation_s 
{
    int type;
    int number;
} aiDataAnimation;


typedef struct aiData
{
	userEntity_t	*pEntity;
	int				nValue;			// nMapNodeIndex, or other integer value (i.e. index)
	float			fValue;			// stores multiple values (i.e. seconds, attributes, etc...)
	CVector			destPoint;
    frameData_t		*pAnimSequence;
	char			*pString;
	CAction			*pAction;
	void			*pVoid;			// general pointer storage

	float gDYaw;					//goal change in Yaw
	float cDYaw;					//current Yaw

} AIDATA, *AIDATA_PTR;


typedef	struct task
{
	TASKTYPE	nTaskType;
	AIDATA		taskData;		//	saved data for this task, depending on the
								//	type of task being performed
	struct task *pNext;

} TASK, *TASK_PTR;


typedef struct taskQueue
{
	int		 nNumTasks;
	TASK_PTR pStartList;
	TASK_PTR pEndList;

} TASKQUEUE, *TASKQUEUE_PTR;

typedef struct goal
{
	int			bFinished;		// has this goal's been satified

	GOALTYPE	nGoalType;
	AIDATA		goalData;		// goal data depends on the type of goal
	
	TASKQUEUE_PTR	pTasks;			// tasks that need to be done in order to satisfy 
								// current goal

	struct goal *pNext;

} GOAL, *GOAL_PTR;

typedef struct goalStack
{
	int		 nNumGoals;
	GOAL_PTR pTopOfStack;

} GOALSTACK, *GOALSTACK_PTR;


#endif _AI_COMMON_H
