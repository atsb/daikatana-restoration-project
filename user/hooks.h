#ifndef _HOOKS_H
#define _HOOKS_H

//	interface to common.dll, CVector type and subroutines
#include "common.h"	
#include "ai_common.h"


class CWhenUsedCommand;

////////////////////////////////////////////////////////////////////////////////////
///	defines
////////////////////////////////////////////////////////////////////////////////////
#define MAC_NUM_INTERMEDIATE_POINTS		5

////////////////////////////////////////////////////////////////////////////
//		Movement state
////////////////////////////////////////////////////////////////////////////
typedef enum enum_MOVEMENTSTATE
{
	MOVEMENT_GROUND,
	MOVEMENT_WATER,
	MOVEMENT_AIR,
	MOVEMENT_TRAIN,
	MOVEMENT_PLATFORM,
	MOVEMENT_LADDER,
} MOVEMENTSTATE;


typedef enum enum_ATTACKTYPE
{
	ATTACK_NONE,
	ATTACK_GROUND_MELEE,
	ATTACK_GROUND_RANGED,
	ATTACK_AIR_MELEE,
	ATTACK_AIR_SWOOP_MELEE,
	ATTACK_AIR_RANGED,
	ATTACK_AIR_SWOOP_RANGED,
	ATTACK_WATER_MELEE,
	ATTACK_WATER_RANGED,
	ATTACK_GROUND_CHASING,

} ATTACKTYPE;


typedef enum enum_ATTACKMODE
{
	ATTACKMODE_NORMAL,
	ATTACKMODE_SWOOPSTRAFE,
	ATTACKMODE_SNIPING,		//	stays in place unless enemy is very close (within 2 nodes)

	ATTACKMODE_MELEE,
	ATTACKMODE_RANGED,
	ATTACKMODE_JUMP,
	ATTACKMODE_CHASE,
    ATTACKMODE_TAKECOVER,

    // sidekick specific stuff
    ATTACKMODE_SIDEKICK_STRAFE,
    ATTACKMODE_SIDEKICK_TAKECOVER,
    ATTACKMODE_SIDEKICK_STAYSTILL,
    ATTACKMODE_SIDEKICK_BACKAWAY,
    ATTACKMODE_SIDEKICK_CHARGE,
    ATTACKMODE_SIDEKICK_JUMP,

} ATTACKMODE;

#define LEFT_SIDE_CLEAR		0x00000001
#define RIGHT_SIDE_CLEAR	0x00000002

////////////////////////////////////////////////////////////////////////////////////
///	task management
////////////////////////////////////////////////////////////////////////////////////

//	stack structure
typedef	struct	taskStack_s
{
	struct	taskStack_s	*next;

	int		task_type;
	void	*task_data;		//	saved data for this task, depending on the
						//	type of task being performed
} taskStack_t;


////////////////////////////////////////////////////////////////////////////////////
///	for intQuota (how smart a creature is)
////////////////////////////////////////////////////////////////////////////////////

#define		INT_PSTEED		1
#define		INT_LOW			2
#define		INT_MEDIUM		3
#define		INT_HIGH		4
#define		INT_NELNO		5

////////////////////////////////////////////////////////////////////////////////////
///	for sequence_max, how many of each sequence type exist
////////////////////////////////////////////////////////////////////////////////////

enum	sequence_nums
{
	SEQUENCE_NONE = -1,
	SEQUENCE_STAND,
	SEQUENCE_WANDER,
	SEQUENCE_RUN,
	SEQUENCE_WALK,
	SEQUENCE_ATAK,
	SEQUENCE_DEATH,
	SEQUENCE_SIGHT,
	SEQUENCE_PAIN,
	SEQUENCE_TURN,
	SEQUENCE_JUMP,
	SEQUENCE_FLY,
	SEQUENCE_SWIM,
	SEQUENCE_TRANS,
	SEQUENCE_WALKTOSTAND,
	SEQUENCE_STANDTOWALK,
	SEQUENCE_WALKTORUN,
	SEQUENCE_RUNTOWALK,
	SEQUENCE_SPECIAL,
	SEQUENCE_ATAKSTART,
	SEQUENCE_ATAKEND,
	SEQUENCE_DODGE,
	SEQUENCE_SHIELD,
	SEQUENCE_MAX
};

////////////////////////////////////////////////////////////////////////////////////
//	stateFlags
////////////////////////////////////////////////////////////////////////////////////

#define SFL_IDLE			0x00000000
#define SFL_WALKING			0x00000001
#define	SFL_RUNNING			0x00000002	//	entity should use running frames and run_speed
#define	SFL_ATTACKING		0x00000004	//	entity should use running frames and attack_speed
#define SFL_CROUCHING       0x00000008
#define SFL_SLOWWALKING     0x00000010  // half the walk speed

////////////////////////////////////////////////////////////////////////////////////
///	flags for client entities
////////////////////////////////////////////////////////////////////////////////////

#define	DFL_LLAMA			0x00000001	//	invulnerability cheat
#define DFL_RAMPAGE			0x00000002	//	super damage cheat
#define	DFL_SHOWSTATS		0x00000004
#define	DFL_RESPAWN			0x00000008	//	set when no respawn points are free
#define	DFL_RANGEDATTACK	0x00000010	//	entity can attack from a distance
#define	DFL_CANSTRAFE		0x00000020	
#define	DFL_TURNED			0x00000040	//	entity just came out of ai_turn
#define	DFL_JUMPATTACK		0x00000080	//	entity can try to jump at enemy from a distance
#define	DFL_BERSERK			0x00000100	//	attacks anything that moves, even same type
#define	DFL_EVIL			0x00000200	//	always attacks DFL_GOOD
#define	DFL_NEUTRAL			0x00000400	//	attacks anything, but only when attacked
#define	DFL_GOOD			0x00000800	//	always attacks DFL_EVIL
#define	DFL_TIMEFOLLOW		0x00001000	//	entity will try to follow using time stamps
#define	DFL_FOLLOWATTACK	0x00002000	//	entity is following while attacking
//#define	DFL_HIERARCHICAL	0x00004000	//	entity is made of multiple models
#define	DFL_FLYSWOOP		0x00008000	//	entity will fly by adjusting height to match goal
#define	DFL_FACEENEMY		0x00010000	//	entity will turn to face self->enemy, default is to face self->goalentity
#define	DFL_FACENONE 		0x00020000	//	entity will turn not turn to face anything
#define	DFL_FACEPITCH		0x00040000	//	entity will adjust pitch to face goal
#define	DFL_FACEOWNER		0x00080000	//	entity will turn to face hook->owner
#define	DFL_ORIENTTOFLOOR	0x00100000	//	entity's pitch/roll will be adjusted to match ground normal
#define	DFL_MECHANICAL		0x00200000	//	entity is a freaking robot
#define DFL_AMPHIBIOUS		0x00400000	//	entity changes movetype based on waterlevel
#define DFL_CANUSELADDER	0x00800000	//	entity can use a ladder
#define DFL_CANUSEDOOR		0x01000000	//	entity can use a door

///////////////////////////////////////////////////////////////////////////////
//	command flags:	these flags are used to communicate state changes to the
//					NPCs.  See NPC_ParseCommand and NPC_SetCommandStates
///////////////////////////////////////////////////////////////////////////////

#define	COMMAND_AGRESSIVE	0x00000001
#define	COMMAND_MODERATE	0x00000002
#define	COMMAND_PASSIVE		0x00000004
#define	COMMAND_REACTIVE	0x00000008
#define	COMMAND_DEFENSIVE	0x00000010

#define	COMMAND_WAIT		0x00000020
#define	COMMAND_FOLLOW		0x00000040
#define	COMMAND_HIDE		0x00000080
#define COMMAND_DIE			0x00000100

//	masks for command flags
#define	COMMANDS_COMBAT		(COMMAND_AGRESSIVE | COMMAND_MODERATE | COMMAND_PASSIVE | COMMAND_REACTIVE | COMMAND_DEFENSIVE)
#define	COMMANDS_FOLLOW		(COMMAND_FOLLOW | COMMAND_WAIT | COMMAND_HIDE)

///////////////////////////////////////////////////////////////////////////////
//	goal flags:	these flags modify the operation of some goal and task types
///////////////////////////////////////////////////////////////////////////////

#define	GFL_AGRESSIVE		0x00000001	//	always attack
#define	GFL_MODERATE		0x00000002	//	attack owner's target
#define	GFL_PASSIVE			0x00000004	//	never ever attack
#define	GFL_REACTIVE		0x00000008	//	attack when attacked
#define	GFL_DEFENSIVE		0x00000010	//	attack when owner is attacked

//	mask for goal flags
#define	GMASK_COMBAT			(GFL_AGRESSIVE | GFL_MODERATE | GFL_PASSIVE | GFL_REACTIVE | GFL_DEFENSIVE)
#define	GMASK_NOTARGET			(GFL_MODERATE | GFL_PASSIVE | GFL_REACTIVE | GFL_DEFENSIVE)	//	never actively acquire targets
#define	GMASK_ACQUIRETARGET		(GFL_AGRESSIVE)	//	actively acquire targets
#define	GMASK_MONSTERDEFAULT	(GFL_AGRESSIVE)
#define	GMASK_NPCDEFAULT		(GFL_MODERATE)


///////////////////////////////////////////////////////////////////////////////
/// dynamic mapping stuff
///////////////////////////////////////////////////////////////////////////////

#define	MAX_NODE_LINKS	6

// ----------------------------------------------------------------------------

typedef struct link
{
	short nDistance;
	short nIndex;
} LINK, *LINK_PTR;

typedef struct nodeData
{
	CVector vector;			// position or angles
	
} NODEDATA, *NODEDATA_PTR;

typedef	struct	mapNode
{
	CVector			position;					// location of this node in the world

	int				nIndex;
	int				nNumLinks;
	LINK			aLinks[MAX_NODE_LINKS];		// links to other nodes reachable from current node

	int				node_type;
	NODEDATA		node_data;

	char			*target;
	char			*targetname;
	void			*pUserHook;
	
	//nodeTime_t		*node_time;

	userEntity_t	*marker;					// links to other nodes reachable from current node
    int             bTriggered;         // used by NODETYPE_STOPSIDEKICK and NODETYPE_TELEPORTSIDEKICK

} MAPNODE, *MAPNODE_PTR;

typedef struct	nodeHeader
{
	int		bUsePathTable;
	
	int		nNumPathTableNodes;
	short	**pPathTable;			// size is (nNumNodes * nNumNodes)

	int		nNumNodes;				// number of nodes in use
	int		nTotalNumNodes;			// total number of nodes allocated
	MAPNODE_PTR pNodes;

} NODEHEADER, *NODEHEADER_PTR;

typedef	struct	nodeList_s
{
	NODEHEADER_PTR	pNodeHeader;
	int				nLastNodeIndex;
	int				nCurrentNodeIndex;
} NODELIST, *NODELIST_PTR;


////////////////////////////////////////////////////////////////////////////////////
/// pathNodes
////////////////////////////////////////////////////////////////////////////////////

// list of nodes in a shortest path

typedef	struct	pathNode_s
{
	int		nNodeIndex;
	struct	pathNode_s	*next_node;
	struct	pathNode_s	*pNextFreeNode;
} PATHNODE, *PATHNODE_PTR;

typedef	struct	pathList_s
{
	struct	pathNode_s	*pPath;		//	list of nodes entity is using to follow
	struct	pathNode_s	*pPathEnd;	//	last node in the path

	int					nPathLength;
	int					nMaxPathLength;
} PATHLIST, *PATHLIST_PTR;

////////////////////////////////////////////////////////////////////////////////////
///	frame tables -- what sounds to perform during a specific sequence and what
///					frames to play them on
////////////////////////////////////////////////////////////////////////////////////


//typedef	struct	sequenceTable_s
//{
//	frameData_t	*stand	[9];
//	frameData_t *wander [9];
//	frameData_t *sight	[9];
//	frameData_t	*run	[9];
//	frameData_t	*walk	[9];
//	frameData_t	*atak	[9];
//	frameData_t	*death	[9];
//	frameData_t	*pain	[9];
//	frameData_t	*turn	[9];
//	frameData_t	*jump	[9];
//	frameData_t	*fly	[9];
//	frameData_t	*swim	[9];
//	frameData_t	*trans	[9];
//	frameData_t	*walktostand [1];
//	frameData_t	*standtowalk [1];
//	frameData_t	*walktorun [1];
//	frameData_t	*runtowalk [1];
//	frameData_t	*special[9];
//
//	frameData_t *atakstart[1];
//	frameData_t *atakend[1];
//	frameData_t *dodge[2];
//	frameData_t *shield[2];
//} sequenceTable_t;

//typedef	struct	vm_sequenceTable_s
//{
//	frameData_t	*shoot	[9];
//	frameData_t	*ready	[9];
//	frameData_t	*goaway	[9];
//	frameData_t	*reload	[9];
//	frameData_t	*ambient[9];
//	frameData_t	*running[9];
//} vm_sequenceTable_t;


////////////////////////////////////////////////////////////////////////////////////
// attribute backup
////////////////////////////////////////////////////////////////////////////////////
typedef struct _savedAttribte
{
	CVector turnSpeed;		// in angles per second
	float fRunSpeed;
	float fWalkSpeed;
} SAVEDATTRIBUTE, *SAVEDATTRIBUTE_PTR;


typedef struct _box
{
	CVector mins;
	CVector maxs;
} BOX;

typedef struct autoAim_s 
{
   userEntity_t *ent;
   CVector shootpos;
} autoAim_t;


// ----------------------------------------------------------------------------
// transition animation info for models

typedef struct _transitionInfo
{
	frameData_t *pFromSequence;
	frameData_t *pToSequence;
	frameData_t *pTransitionSequence;
} TRANSITIONINFO, *TRANSITION_PTR;

class CTransitionSequences
{
public:
	CTransitionSequences();
	CTransitionSequences( int nNum );
	~CTransitionSequences();

//	void* operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

	TRANSITION_PTR Allocate( int nNum );
	
	int GetNumTransitions()						{ return nNumTransitions; }
	TRANSITION_PTR GetTransitionSequences()		{ return pTransitions; }
	
	void SetTransitionSequence( int nIndex, frameData_t *pSequence1, frameData_t *pSequence2, frameData_t *pSequence3 );

	frameData_t *FindTransitionSequence( frameData_t *pSequence1, frameData_t *pSequence2 );

private:
	int nNumTransitions;
	TRANSITION_PTR pTransitions;
};

////////////////////////////////////////////////////////////////////////////////////
// ISP: 04-05-99
class CRespawnData
{
public:
	CRespawnData();
	CRespawnData( const CVector &newSpawnPoint, char *szNewScriptAction, int bRespawnFlag );

//	void* operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

	void SetSpawnPoint( const CVector &newSpawnPoint )
	{ spawnPoint = newSpawnPoint; }
	CVector &GetSpawnPoint()	{ return spawnPoint; }
	void SetScriptAction( char *szNewScriptAction )
	{ szScriptAction = szNewScriptAction; }
	char *GetScriptAction()		{ return szScriptAction; }
	void SetRespawnFlag( int nFlag )
	{ bRespawnAfterDeath = nFlag; }
	int GetRespawnFlag()		{ return bRespawnAfterDeath; }

private:
	CVector spawnPoint;
	char *szScriptAction;
	int bRespawnAfterDeath;
};

extern	common_export_t* memory_com;

typedef enum enum_FOLLOWING
{
    FOLLOWING_NONE,
    FOLLOWING_PLAYER,
    FOLLOWING_SIDEKICK,
    FOLLOWING_ENTITY,
} FOLLOWING;

////////////////////////////////////////////////////////////////////////////////////
//          the following class is used to keep track of ai function calls to
//          trap possible stack overflows
////////////////////////////////////////////////////////////////////////////////////

#define MAX_NUM_TASKS   10

class CTaskTracker
{
public:
    CTaskTracker()  { nNumTotalTasks = 0; }
    ~CTaskTracker() {};

    int GetNumTotalTasks()       { return nNumTotalTasks; }
    int GetNumForTaskType( TASKTYPE nTaskType )
    {
        for ( int i = 0; i < nNumTotalTasks; i++ )
        {
            if ( nTaskType == aTasks[i] )
            {
                return aNumCalls[i];
            }
        }

        return 0;
    }

    void Clear()        { nNumTotalTasks = 0; }
    void AddTask( TASKTYPE nTaskType )
    {
        int bFound = FALSE;
        for ( int i = 0; i < nNumTotalTasks; i++ )
        {
            if ( nTaskType == aTasks[i] )
            {
                aNumCalls[i]++;
                bFound = TRUE;
                break;
            }
        }

        if ( bFound == FALSE && nNumTotalTasks < MAX_NUM_TASKS )
        {
            aTasks[nNumTotalTasks] = nTaskType;
            aNumCalls[nNumTotalTasks] = 1;
            nNumTotalTasks++;
        }
    }

private:
    int nNumTotalTasks;
    
    int aNumCalls[MAX_NUM_TASKS];
    TASKTYPE aTasks[MAX_NUM_TASKS];
};


////////////////////////////////////////////////////////////////////////////////////
/// player hook
////////////////////////////////////////////////////////////////////////////////////

class playerHook_t 
{ 
public:
   	//unix - fixed inline constructor
	//__inline playerHook_t();
	playerHook_t() 
	{
		nFollowing	= FOLLOWING_NONE;
		//memset(this,0,sizeof(playerHook_t));
	}

//	void* playerHook_t::operator new (size_t size) { return memory_com->X_Malloc(size,MEM_TAG_HOOK); }
//	void* playerHook_t::operator new[] (size_t size) { return memory_com->X_Malloc(size,MEM_TAG_HOOK); }
//	void  playerHook_t::operator delete (void* ptr) { memory_com->X_Free(ptr); }
//	void  playerHook_t::operator delete[] (void* ptr) { memory_com->X_Free(ptr); }

	// bot's have this alternate owner field because Quake doesn't clip
	// objects against their owner
	userEntity_t		*owner;

	//////////////////////////////////////////////
	//	stats
	//////////////////////////////////////////////

	float		base_health;
	char		pain_chance;
	int			intQuota;			// how smart this monster/bot is 

	int			active_distance;	// distance from a client before
									// becoming inactive;
	short		back_away_dist;		// distance from goal when entity will back away
	char		type;				// what monster is this?  For avoiding stricmps

	float		upward_vel;			// used for jumping, set in FindLedge
	float		forward_vel;		// FIXME: obsolete?

	float		run_speed;
	float		walk_speed;
	float		attack_dist;		// distance away before bot will attack
	float		jump_attack_dist;	// used for monsters that can jump attack
	float		fov;				// field of view
	int			max_jump_dist;
	char		jump_chance;		//	percentage chance that this entity will use a jump attack (0-100)
	float		attack_speed;
	float		fly_speed;
	float		fly_goal_offset;	//	how far above/below its goal a flying creature desires to be
	int			ID;					//	this entity's ID number (only for bots and players, used for time stamp node following)
	float		skill;				//	skill level of this bot, 0 - 100.  Affects combat tactics, hit chance, firing rate, etc.
	float       speak;              //  distance that a bot can speak to other bots  <nss>

	//////////////////////////////////////////////
	// timer fields
	//////////////////////////////////////////////

	float		pain_finished;
	float		drown_time;
	float		attack_finished;
	float		jump_time;
	// time remaining until entity can make a sound
	float		sound_time;
	float		path_time;
	float		think_time;
	float		shake_time;
	float		ambient_time;              // delay before next ambient

	/////////////////////////////////////////////
	// powerup flags and timers
	/////////////////////////////////////////////

	float		invulnerability_time;
	float		envirosuit_time;
	float		wraithorb_time;
	float		oxylung_time;
	
	/////////////////////////////////////////////
	//	poisoning
	/////////////////////////////////////////////

	float		poison_time;					//	time when poison expires
	float		poison_next_damage_time;		//	next time when poison will damage
	float		poison_damage;					//	amount of damage poison will do
	float		poison_interval;				//	how often poison does damage

    // 19990308 JAR - boosts are always used with floats when making calculations
	// stat boosts
	float       base_power;
	float       base_attack;
	float       base_speed;
	float       base_acro;
	float       base_vita;

	float       power_boost;
	float		attack_boost;
	float       speed_boost;
	float       acro_boost;
	float       vita_boost;

	// expiration times for temporary attribute powerups
	float		power_boost_time;
	float		attack_boost_time;
	float		speed_boost_time;
	float		acro_boost_time;
	float		vita_boost_time;

	/////////////////////////////////////////////
	// flags
	/////////////////////////////////////////////

	unsigned	long	dflags;		//	DLL specific flags (ie. not needed by the engine)
	unsigned	long	items;
	unsigned	long	exp_flags;	//	expiration flags for items
	unsigned	long	stateFlags;	//	some flags that specify states for this creature, like walking, running, etc.
	unsigned	long	ai_flags;	//	the current ai action a bot is performing -- OLD AI
	char				th_state;	//	think states -- OLD AI and client animation
	unsigned	long	goalFlags;	//	modifies how some tasks and goals are performed.  Mainly for NPCs.

	// for toggling fog in water
	int			fog_on;
	int			fog_value;

	// for track velocity changes
	float		last_zvel;
	float		last_xyvel;

	/////////////////////////////////////////////
	// node system stuff
	/////////////////////////////////////////////

	NODELIST_PTR	pNodeList;			// pointer to the node list this bot is using
	PATHLIST_PTR	pPathList;

	// location of entity during its think, not necessarily
	// it's last location before the Quake engine moved it
	CVector		last_origin;
	
	//////////////////////////////////////////////
	// wall following stuff
	//////////////////////////////////////////////

	// used by wall following code so that entities keep
	// turning the same way to go around an obstruction
	float		turn_yaw;

	//////////////////////////////////////////////
	// animation and frame stuff
	//////////////////////////////////////////////

	char		*sound1;
	char		*sound2;

	int				cur_seq_type;			//	type of sequence currently playing
	int				cur_seq_num;			//	number of sequence currently playing
	frameData_t		*cur_sequence;

	/////////////////////////////////////////////////////
	//	vars for animating client viewmodel (weapon)
	/////////////////////////////////////////////////////

	short	fxFrameNum;
	void	(*fxFrameFunc)(userEntity_t *self);


	////////////////////////////////////////////////////
	// weapons and armor
	////////////////////////////////////////////////////
	
	int				last_button0; // for weapons that are fired when button0 is released

	int				viewmodel_start_frame;			// first frame to play in weapon firing sequence
	int				viewmodel_end_frame;			// last frame to play in weapon firing sequence
	float			viewmodel_frame_time;			// time between weapon fire frames

	userInventory_t	*weapon_fired;
	think_t			weapon_next;					// routine to call after current weapon is put away

	/////////////////////////////////////////////////
	// for keeping track of deathmatch stats
	/////////////////////////////////////////////////
	int				killed;
	int				died;
	int				fired;
	int				hit;

	/////////////////////////////////////////////////
	// function pointers
	/////////////////////////////////////////////////

	think_t		begin_jump;
	think_t		begin_stand;
	think_t		begin_follow;
	think_t		begin_path_follow;
	think_t		begin_wander;
	think_t		begin_attack;
	think_t		begin_seek;
	think_t		begin_pain;
	think_t		wander_sound;
	think_t		sight_sound;
	think_t		begin_turn;
	think_t		begin_path_wander;
	think_t		begin_action;
	think_t		continue_think;
	think_t		special_think;
	think_t		temp_think;
	think_t		begin_node_wander;
	think_t		begin_node_retreat;
	think_t		sight;					//	if not NULL, called when an entity sights an enemy
	think_t		begin_time_stamp_follow;
	think_t		follow_attack;			//	attack to perform while moving towards a specific goal
	think_t		begin_follow_attack;	//	sets up frames for starting a following attack

	userEntity_t *(*find_target)(userEntity_t *self);

	void	(*begin_transition)(userEntity_t *self, frameData_t *data, int seq_type, int seq_num, think_t think);

	think_t		fnAttackFunc;
	think_t		fnStartAttackFunc;
	think_t		fnTakeCoverFunc;

	think_t		fnStartJumpUp;
	think_t		fnJumpUp;

	think_t		fnStartJumpForward;
	think_t		fnJumpForward;

	int			(*fnChasingAttack)(userEntity_t *self);

	userEntity_t	*(*fnFindTarget)(userEntity_t *self);
	think_t		fnStartCower;
	think_t		fnCower;
	think_t     fnStartIdle;

	int			(*fnInAttackRange)(userEntity_t *self, float dist, userEntity_t *enemy);

	// for breathing effect after running
	int				fatigue;

	////////////////////////////////////////////////
	// warping 
	////////////////////////////////////////////////

	CVector			force_angles;
	CVector			force_rate;
	float			ideal_pitch;
	int				angles_last_change;

	////////////////////////////////////////////////
	//	camera and possession stuff
	////////////////////////////////////////////////

	userEntity_t	*camera;	// pointer to the camera this ent is
								// viewing from, usually NULL
	userEntity_t	*demon;    	// pointer to the entity possessing this ent

	////////////////////////////////////////////////
	//	combat strafing
	////////////////////////////////////////////////

	char			strafe_dir;
	float			strafe_time;

	/////////////////////////////////////////////////
	//	hierarchical model pieces
	/////////////////////////////////////////////////

/*
	userEntity_t	*head;
	userEntity_t	*torso;
	userEntity_t	*legs;
	userEntity_t	*weapon;
*/

	GOALSTACK_PTR	pGoals;

	GOALSTACK_PTR	pScriptGoals;

	///	FIXME: reorder when recompiling all DLLs
	char			turn_dir;

	CVector			ground_org;

	char			*caller [10];			//	who made the last 10 calls to ai_set_think

	//	ambient motion stuff for Mikiko and Superfly
	float			follow_dist;			//	distance away that Superfly, Mikiko will run after player
	float			walk_follow_dist;		//	distance away that Superfly, Mikiko will walk after player

	//////////////////////////////////////////////////
	//	field of view ramping
	//////////////////////////////////////////////////
	float			fov_rate;				//	rate of change/second
	float			fov_desired;			//	desired fov
	unsigned long	fov_last_change;		//	last time of change

	int				nMoveCounter;
	int				nTargetCounter;			//	frame counter for checking for targets
	int				nTargetFrequency;		//	number of frames between target checks
	CVector			startPosition;

	ATTACKTYPE		nAttackType;
	ATTACKTYPE		nCurrentAttackType;
	ATTACKMODE		nAttackMode;
	int				nSpecificAttackMode;		// attack mode used for different AI's

	SAVEDATTRIBUTE	savedAttribute;

	MOVEMENTSTATE	nMovementState;

	int				nWaitCounter;			// counter to keep track of how long an AI unit is in a current state

	int				nCanUseFlag;			// keeps track of that this entity can use
	BOX				boundedBox;

	int				nSpawnValue;			// initial spawn flags
	
	float           fTenUpdatesDistance;
	int				nMovingCounter;

    autoAim_t       autoAim;

	float			fTaskFinishTime;
	int				bOkToAttack;			// can the AI unit attack the enemy while in a specific task

    //adam:  the animation type and number that will be used as the entity's idle animation.
    //int idle_animation_type;
    //int idle_animation_number;

	float			fAttackTime;			// time allowed for each attacks, can not attack more than this
	movetype_t		nOriginalMoveType;

	FOLLOWING		nFollowing;		        // used by sidekicks indicating what they are following 
	float			fNextPathTime;			// unit can compute path when gstate->time > fNextPathTime

	unsigned long	nFlags;					// can be used for anything - ex. if protopod has hatched or not

	int				nTransitionIndex;
	float			fTransitionTime;		// transition sequence time limit

	CTransitionSequences *pTransitions;

    // 19990307 JAR - teamplay variables, player class id for stats, etc...
    int             iPlayerClass;           // PLAYERCLASS_HIRO, PLAYERCLASS_MIKIKO, PLAYERCLASS_SUPERFLY, PLAYERCLASS_whateverelsewewantoadd
    float           fLastHealthTick;        // tick health -1 / sec back to base

	CRespawnData	*pRespawnData;
	
	frameData_t		*pMovingAnimation;
	char			*szScriptName;

	CWhenUsedCommand *pWhenUsedCommand;

	userEntity_t	*pEntityToLookAt;

    float           fSoundOutDistance;       // the distance the sound travels from what this entity is doing

    // ISP [8-3-99] sidekick asked to have this item
    userEntity_t    *pWantItem;

    float           fMinAttenuation;
    float           fMaxAttenuation;

    unsigned long   bInScriptMode   : 1,
                    bBounded        : 1,
                    bMoving         : 1,
                    bInTransition   : 1,
                    bDucking        : 1,
                    bInAttackMode   : 1,
                    bOffGround      : 1,    // flag to tell if bot/player left ground between nodes
                    bJumped         : 1,    // flag to tell if player jumped between nodes
                    bStuck          : 1,    // TRUE if entity was stuck/blocked during last move attempt
                    bAttackPlayer   : 1,    // sidekick only flag
                    bCarryingMikiko : 1,    // Superfly only flag
                    bJustFired      : 2,
                    bEmpty          : 20;

    CTaskTracker taskTracker;

	// cek[1-25-00]: added for delayed deathmatch taunts
	float			ftauntTime;
	int				ntauntIndex;

	float			freezeLevel;
	float			freezeTime;
	float			freezeStart;

	// cek[2-12-00]: added to fix stupid hack in client jump code.
	bool			can_jump_again;
	float			fNextStruggleTime;
};

//////////////////////////////////////////////////////////////////////////
// hardcoded monster paths
//////////////////////////////////////////////////////////////////////////

typedef	struct
{
	char *target[4];
	char *action[4];
	char *pathtarget;
	char *szScriptName;
} mpathHook_t;

#endif
