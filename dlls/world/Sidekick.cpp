// ==========================================================================
//
//  File:       Sidekick.cpp
//  Contents:
//  Author:
//
// ==========================================================================

#if _MSC_VER
#include <crtdbg.h>
#endif
#include "world.h"
#include "ai_move.h"
#include "ai_frames.h"
#include "nodelist.h"
#include "actorlist.h"
#include "ai_func.h"
#include "MonsterSound.h"
#include "ai.h"
#include "spawn.h"
#include "Sidekick.h"
#include "ai_info.h"
#include "SequenceMap.h"
#include "ai_utils.h"
#include "items.h"
#include "ai_weapons.h"
#include "weapondefs.h"
#include "ai_think.h"
#include "csv.h"
#include "dk_io.h"
#include "chasecam.h"
#include "client.h"

/* ***************************** define types ****************************** */

//#define USE_STRAFE
#define DO_AMBIENTS

#define HIRO_CLASSNAME		"Hiro"
#define MIKIKO_CLASSNAME	"Mikiko"
#define SUPERFLY_CLASSNAME	"Superfly"
#define MIKIKOFLY_CLASSNAME	"MikikoFly"

#define SUPERFLY_CARRYING_MIKIKO_MODEL  "models/global/m_mikiko.dkm"

#define SIDEKICK_MELEE_ATTACK_DISTANCE	128.0f
#define SIDEKICK_WIDTH_FACTOR   0.75f
#define PROJECTILE_EFFECTIVE_DISTANCE   512.0f

#ifdef _DEBUG
void AI_Drop_Markers (CVector &Target, float delay);
#endif
/* ***************************** Local Variables *************************** */

// NSS[2/24/00]:Wait timers for commands.
float Superflywait,MikikoWait;
const static float fAngleAwayFromOwner      = 75.0f;
const static float fNormalDistanceFromOwner = 96.0f;
const static float fCloseDistanceToOwner    = 65.0f;
const static float fOneHalfFrameDistance    = 60.0f;

static float SuperFlyWaitIdle;				// NSS[3/4/00]:Variable used to determine how long the sidkick has been waiting in one place.
static float MikikoWaitIdle;

static float SuperflyLastTalk;
static float MikikoLastTalk;

static DEATHTYPE sidekick_deathtype = NONE;
static int bInGodMode = FALSE;


// ----------------------------------------------------------------------------

#define E1_DISRUPTOR    0
#define E1_IONBLASTER	1
#define E1_SHOTCYCLER	2
#define E1_NUMWEAPONS   3

#define E2_DISCUS       0
#define E2_VENOMOUS     1
#define E2_TRIDENT		2
#define E2_NUMWEAPONS   3

#define E3_SILVERCLAW   0
#define E3_BOLTER       1
#define E3_BALLISTA     2
#define E3_NUMWEAPONS   3

#define E4_GLOCK        0
#define E4_RIPGUN       1
#define E4_SLUGGER      2
#define E4_NUMWEAPONS   3


// NSS[12/5/99]:Maximum nuimber of messages to hold in the que 
#define MAX_SIDEKICK_MESSAGES 128


//NSS[12/4/99]:Communication buffers for sidekicks
SideKickCom_t SuperflyCom[MAX_SIDEKICK_MESSAGES];
SideKickCom_t MikikoCom[MAX_SIDEKICK_MESSAGES];






//NSS[12/4/99]:Had to create a cross reference table because someone had no fore sight and obviously didn't understand naming conventions.
SideMp3_t SuperFlyVoices[]=
{
	{4,0,HEALTH_RESPONSE1,	"sid_s_01.mp3"},
	{4,0,HEALTH_RESPONSE2,	"sid_s_02.mp3"},
	{4,0,HEALTH_RESPONSE3,	"sid_s_03.mp3"},
	{4,0,HEALTH_RESPONSE4,	"sid_s_04.mp3"},
	{4,0,HEALTH_RESPONSE5,	"sid_s_05.mp3"},
	{4,0,HIRO_HEALTH_G50,		"sid_s_06.mp3"},
	{4,0,HIRO_HEALTH_L50,		"sid_s_07.mp3"},
	{4,0,HIRO_HEALTH_L15,		"sid_s_08.mp3"},
	{4,0,IMPATIENT,			"sid_s_09.mp3"},
	{4,0,IMPATIENT,			"sid_s_10.mp3"},
	{8,0,IMPATIENT,			"sid_s_amblong_bustskulls1.mp3"},
#ifndef DAIKATANA_DEMO
	{8,0,IMPATIENT,			"sid_s_amblong_geez1.mp3"},
	{8,0,IMPATIENT,			"sid_s_amblong_gottahurry1.mp3"},
	{8,0,IMPATIENT,			"sid_s_amblong_heylets1.mp3"},
	{8,0,IMPATIENT,			"sid_s_amblong_onefoot1.mp3"},
	{8,0,IMPATIENT,			"sid_s_amblong_pizza1.mp3"},
	{20,0,IMPATIENT,			"sid_s_amblong_snore1.mp3"},
	{20,0,IMPATIENT,			"sid_s_amblong_snore2.mp3"},
	{8,0,IMPATIENT,			"sid_s_amblong_thisplace1.mp3"},
	{8,0,IMPATIENT,			"sid_s_amblong_whistle1.mp3"},
#endif
	{4,0,LOOKING_UP,			"sid_s_11.mp3"},
	{4,0,LOOKING_UP,			"sid_s_12.mp3"},
	{4,0,LOOKING_UP,			"sid_s_amb_hahahmm1.mp3"},
#ifndef DAIKATANA_DEMO
	{4,0,LOOKING_UP,			"sid_s_amb_hello1.mp3"},
	{4,0,LOOKING_UP,			"sid_s_amb_hello2.mp3"},
	{4,0,LOOKING_UP,			"sid_s_amb_heyhiro1.mp3"},
#endif
	{4,0,LOOKING_SIDEWAYS,	"sid_s_13.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_s_14.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_s_amb_checkthat1.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_s_amb_cracka1.mp3"},
#ifndef DAIKATANA_DEMO
	{4,0,LOOKING_SIDEWAYS,	"sid_s_amb_hatching1.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_s_amb_whassup1.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_s_amb_whassup2.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_s_amb_whassup3.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_s_amb_thatlooks1.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_s_amb_thatsgross1.mp3"},
#endif
	{6,0,RANDOM_WALK,			"sid_s_amb_gonnawander1.mp3"},
	{6,0,RANDOM_WALK,			"sid_s_amb_heywhats1.mp3"},
	{6,0,RANDOM_WALK,			"sid_s_amb_hirolisten1.mp3"},
	{6,0,RANDOM_WALK,			"sid_s_amb_placerocks1.mp3"},
#ifndef DAIKATANA_DEMO
	{6,0,RANDOM_WALK,			"sid_s_amb_somethings1.mp3"},
	{6,0,RANDOM_WALK,			"sid_s_amb_wanderin1.mp3"},
	{6,0,RANDOM_WALK,			"sid_s_amb_whatdid1.mp3"},
	{6,0,RANDOM_WALK,			"sid_s_amb_whatsgoin1.mp3"},
	{6,0,RANDOM_WALK,			"sid_s_amb_youhear1.mp3"},
#endif
	{4,0,HITBY_HIRO,			"sid_s_15.mp3"},
	{6,0,HITBY_HIRO,			"sid_s_hiroshot_assdust1.mp3"},
	{6,0,HITBY_HIRO,			"sid_s_hiroshot_bustyou1.mp3"},
	{6,0,HITBY_HIRO,			"sid_s_hiroshot_idiotpile1.mp3"},
#ifndef DAIKATANA_DEMO
	{6,0,HITBY_HIRO,			"sid_s_hiroshot_jacking1.mp3"},
	{6,0,HITBY_HIRO,			"sid_s_hiroshot_knobby1.mp3"},
	{6,0,HITBY_HIRO,			"sid_s_hiroshot_punchteeth1.mp3"},
	{6,0,HITBY_HIRO,			"sid_s_hiroshot_smackdown1.mp3"},
	{6,0,HITBY_HIRO,			"sid_s_hiroshot_stupidload1.mp3"},
	{6,0,HITBY_HIRO,			"sid_s_hiroshot_takeeyes1.mp3"},
	{6,0,HITBY_HIRO,			"sid_s_hiroshot_watchit1.mp3"},
#endif
	{4,0,HITBY_SIDEKICK,		"sid_s_16.mp3"},
	{4,0,PUSHEDBY_HIRO,		"sid_s_push_youlike1.mp3"},
	{4,0,PUSHEDBY_HIRO,		"sid_s_push_asktomove1.mp3"},
	{4,0,PUSHEDBY_HIRO,		"sid_s_push_getoffme1.mp3"},
#ifndef DAIKATANA_DEMO
	{4,0,PUSHEDBY_HIRO,		"sid_s_push_heyman1.mp3"},
	{4,0,PUSHEDBY_HIRO,		"sid_s_push_heythats1.mp3"},
#endif
	{4,0,STARTING_ATTACK,		"sid_s_17.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_18.mp3"},
	{4,0,STARTING_ATTACK,		"Super_32.mp3"},
	{4,0,STARTING_ATTACK,		"Super_34.mp3"},
#ifndef DAIKATANA_DEMO
	{4,0,STARTING_ATTACK,		"Super_37.mp3"},
	{4,0,STARTING_ATTACK,		"Super_41.mp3"},
	{4,0,STARTING_ATTACK,		"Super_50.mp3"},
	{4,0,STARTING_ATTACK,		"Super_51.mp3"},
	{4,0,STARTING_ATTACK,		"Super_54.mp3"},
	{4,0,STARTING_ATTACK,		"Super_56.mp3"},
	{4,0,STARTING_ATTACK,		"Super_59.mp3"},
	{4,0,STARTING_ATTACK,		"Super_60.mp3"},
	{4,0,STARTING_ATTACK,		"e1_super_31.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_blanket1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_daddytime1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_gulpit1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_jackedup1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_lemmebust1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_maddaddy1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_pity1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_punktime1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_ruin1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_tearcrap1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_s_atk_timefor1.mp3"},
#endif
	{8,0,HIRO_DIES,			"sid_s_hirodies_asscapped1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_bitchslapped1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_blowndown1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_busted1.mp3"},
#ifndef DAIKATANA_DEMO
	{8,0,HIRO_DIES,			"sid_s_hirodies_dead1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_demolished1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_goodbye1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_goodbye2.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_goodbye3.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_hiroisdead1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_jawbounce1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_nutskicked1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_skullsplit1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_sucks1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_tongue1.mp3"},
	{8,0,HIRO_DIES,			"sid_s_hirodies_wander1.mp3"},
#endif
	{4,0,NO_AMMO,				"sid_s_19.mp3"},
	{4,0,NO_AMMO,				"sid_s_20.mp3"},
	{4,0,LOW_HEALTH,			"sid_s_21.mp3"},
	{4,0,LOW_HEALTH,			"sid_s_22.mp3"},
	{4,0,START_CROUCH,		"sid_s_23.mp3"},
	{4,0,START_CROUCH,		"sid_s_24.mp3"},
	{4,0,LOOK_4_HIRO,			"sid_s_25.mp3"},
	{4,0,LOOK_4_HIRO,			"sid_s_26.mp3"},
	{4,0,RUN_AWAY,			"sid_s_27.mp3"},
	{4,0,RUN_AWAY,			"sid_s_28.mp3"},
	{4,0,RUN_AWAY,			"sid_s_hiroruns_backhere1.mp3"},
#ifndef DAIKATANA_DEMO
	{4,0,RUN_AWAY,			"sid_s_hiroruns_hottie1.mp3"},
	{4,0,RUN_AWAY,			"sid_s_hiroruns_laterass1.mp3"},
	{4,0,RUN_AWAY,			"sid_s_hiroruns_runafter1.mp3"},
#endif
	{4,0,QUESTION_HEALTH,		"sid_s_q_01a.mp3"},
	{4,0,ANSWER_1,			"sid_s_29.mp3"},
	{4,0,QUESTION_1,			"sid_s_30.mp3"},
	{4,0,ANSWER_2,			"sid_s_31.mp3"},
	{4,0,PICKUP_ITEM,			"sid_s_32.mp3"},
	{4,0,EVADE,				"death3.wav"},
	{4,0,EVADE,				"death7.wav"},
	{4,0,JUMP,				"jump1.wav"},
	{4,0,JUMP,				"jump2.wav"},
	{4,0,JUMP,				"jump3.wav"},
	{4,0,JUMP,				"jump4.wav"},
	{4,0,JUMP,				"jump5.wav"},
	{4,0,LANDING,				"Land1.wav"},
	{4,0,LANDING,				"Land2.wav"},
	{4,0,LANDING,				"Land3.wav"},
	{4,0,LANDING,				"Land4.wav"},
	{4,0,LANDING,				"Land5.wav"},
	{4,0,NO,					"cmd_su_no_01.mp3"},
	{4,0,NO,					"cmd_su_no_02.mp3"},
	{4,0,NO,					"cmd_su_no_03.mp3"},
	{4,0,CANTREACH,			"cmd_su_pu_01_n2.mp3"},
	{4,0,CANTREACH,			"cmd_su_pu_02_n2.mp3"},
	{4,0,CANTREACH,			"cmd_su_pu_03_n2.mp3"},
	{4,0,NO_TALK,""}
};


SideMp3_t MikikoVoices[]=
{
	{4,0,HEALTH_RESPONSE1,	"sid_m_01.mp3"},
	{4,0,HEALTH_RESPONSE2,	"sid_m_02.mp3"},
	{4,0,HEALTH_RESPONSE3,	"sid_m_03.mp3"},
	{4,0,HEALTH_RESPONSE4,	"sid_m_04.mp3"},
	{4,0,HEALTH_RESPONSE5,	"sid_m_05.mp3"},
	{4,0,HIRO_HEALTH_G50,		"sid_m_06.mp3"},
	{4,0,HIRO_HEALTH_L50,		"sid_m_07.mp3"},
	{4,0,HIRO_HEALTH_L15,		"sid_m_08.mp3"},
	{8,0,IMPATIENT,			"sid_m_09.mp3"},
	{8,0,IMPATIENT,			"sid_m_10.mp3"},
	{8,0,IMPATIENT,			"sid_m_amblong_getgoing1.mp3"},
	{8,0,IMPATIENT,			"sid_m_amblong_gettingold1.mp3"},
#ifndef DAIKATANA_DEMO
	{8,0,IMPATIENT,			"sid_m_amblong_hungry1.mp3"},
	{8,0,IMPATIENT,			"sid_m_amblong_letsmove1.mp3"},
	{8,0,IMPATIENT,			"sid_m_amblong_movealong1.mp3"},
	{8,0,IMPATIENT,			"sid_m_amblong_moving1.mp3"},
	{8,0,IMPATIENT,			"sid_m_amblong_notime1.mp3"},
	{8,0,IMPATIENT,			"sid_m_amblong_thirsty1.mp3"},
	{8,0,IMPATIENT,			"sid_m_amblong_waiting1.mp3"},
#endif
	{4,0,LOOKING_UP,			"sid_m_11.mp3"},
	{4,0,LOOKING_UP,			"sid_m_12.mp3"},
	{4,0,LOOKING_UP,			"sid_m_amb_hearsometh1.mp3"},
	{4,0,LOOKING_UP,			"sid_m_amb_smellthat1.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_m_13.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_m_14.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_m_amb_clockthose1.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_m_amb_feelingshabs1.mp3"},
#ifndef DAIKATANA_DEMO
	{4,0,LOOKING_SIDEWAYS,	"sid_m_amb_hello1.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_m_amb_heyhiro1.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_m_amb_hmm2.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_m_amb_hmm3.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_m_amb_whatshappen1.mp3"},
	{4,0,LOOKING_SIDEWAYS,	"sid_m_amb_whatare1.mp3"},
#endif
	{4,0,RANDOM_WALK,			"sid_m_amb_hmm1.mp3"},
	{4,0,RANDOM_WALK,			"sid_m_amb_interesting1.mp3"},
	{4,0,RANDOM_WALK,			"sid_m_amb_somethingsover1.mp3"},
	{4,0,RANDOM_WALK,			"sid_m_amb_nice1.mp3"},
#ifndef DAIKATANA_DEMO
	{4,0,RANDOM_WALK,			"sid_m_amb_smellinteresting1.mp3"},
	{4,0,RANDOM_WALK,			"sid_m_amb_somethingsmells1.mp3"},
	{4,0,RANDOM_WALK,			"sid_m_amb_whatis1.mp3"},
	{4,0,RANDOM_WALK,			"sid_m_amb_whatsover1.mp3"},
#endif
	{4,0,HITBY_HIRO,			"sid_m_15.mp3"},
	{4,0,HITBY_HIRO,			"sid_m_hiroshot_bunyip1.mp3"},
	{4,0,HITBY_HIRO,			"sid_m_hiroshot_chucku1.mp3"},
	{4,0,HITBY_HIRO,			"sid_m_hiroshot_hobbit1.mp3"},
#ifndef DAIKATANA_DEMO
	{4,0,HITBY_HIRO,			"sid_m_hiroshot_jackedup1.mp3"},
	{4,0,HITBY_HIRO,			"sid_m_hiroshot_mansteak1.mp3"},
	{4,0,HITBY_HIRO,			"sid_m_hiroshot_nooselick1.mp3"},
	{4,0,HITBY_HIRO,			"sid_m_hiroshot_nubmunch1.mp3"},
	{4,0,HITBY_HIRO,			"sid_m_hiroshot_whatdoyou1.mp3"},
#endif
	{4,0,HITBY_SIDEKICK,		"sid_m_16.mp3"},
	{4,0,PUSHEDBY_HIRO,		"sid_m_push_stoppushing1.mp3"},
	{4,0,PUSHEDBY_HIRO,		"sid_m_push_watchit1.mp3"},
	{4,0,PUSHEDBY_HIRO,		"sid_m_push_watchwhere1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_m_17.mp3"},
	{4,0,STARTING_ATTACK,		"sid_m_18.mp3"},
	{4,0,STARTING_ATTACK,		"sid_m_atk_ampedup1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_m_atk_completed1.mp3"},
#ifndef DAIKATANA_DEMO
	{4,0,STARTING_ATTACK,		"sid_m_atk_igotthis1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_m_atk_lemmeatem1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_m_atk_skullcrack1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_m_atk_suckitdown1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_m_atk_tearitdown1.mp3"},
	{4,0,STARTING_ATTACK,		"sid_m_atk_timefora1.mp3"},
#endif
	{4,0,NO_AMMO,				"sid_m_19.mp3"},
	{4,0,NO_AMMO,				"sid_m_20.mp3"},
	{4,0,LOW_HEALTH,			"sid_m_21.mp3"},
	{4,0,LOW_HEALTH,			"sid_m_22.mp3"},
	{4,0,START_CROUCH,		"sid_m_23.mp3"},
	{4,0,START_CROUCH,		"sid_m_24.mp3"},
	{4,0,LOOK_4_HIRO,			"sid_m_25.mp3"},
	{4,0,LOOK_4_HIRO,			"sid_m_26.mp3"},
	{4,0,RUN_AWAY,			"sid_m_27.mp3"},
	{4,0,RUN_AWAY,			"sid_m_28.mp3"},
	{4,0,RUN_AWAY,			"sid_m_hiroruns_alone1.mp3"},
#ifndef DAIKATANA_DEMO
	{4,0,RUN_AWAY,			"sid_m_hiroruns_getback1.mp3"},
	{4,0,RUN_AWAY,			"sid_m_hiroruns_manbeast1.mp3"},
	{4,0,RUN_AWAY,			"sid_m_hiroruns_runningaway1.mp3"},
#endif
	{4,0,PICKUP_ITEM,			"sid_m_29.mp3"},
	{4,0,PICKUP_ITEM,			"sid_m_30.mp3"},
	{4,0,QUESTION_HEALTH,		"sid_m_q_01.mp3"},
	{4,0,QUESTION_1,			"sid_m_q_02.mp3"},
	{4,0,QUESTION_2,			"sid_m_q_04.mp3"},
	{4,0,ANSWER_1,			"sid_m_a_03.mp3"},
	{4,0,ANSWER_2,			"sid_m_a_05.mp3"},
	{4,0,EVADE,				"death2.wav"},
	{4,0,EVADE,				"death8.wav"},
	{4,0,EVADE,				"death7.wav"},
	{4,0,JUMP,				"jump1.wav"},
	{4,0,JUMP,				"jump2.wav"},
	{4,0,JUMP,				"jump3.wav"},
	{4,0,JUMP,				"jump4.wav"},
	{4,0,JUMP,				"jump5.wav"},
	{4,0,LANDING,				"Land1.wav"},
	{4,0,LANDING,				"Land2.wav"},
	{4,0,LANDING,				"Land3.wav"},
	{4,0,LANDING,				"Land4.wav"},
	{4,0,LANDING,				"Land5.wav"},
	{4,0,NO,					"cmd_mi_no_01.mp3"},
	{4,0,NO,					"cmd_mi_no_03.mp3"},
	{4,0,CANTREACH,			"cmd_mi_pu_01_n2.mp3"},
	{4,0,CANTREACH,			"cmd_mi_pu_02_n2.mp3"},
	{4,0,CANTREACH,			"cmd_mi_pu_03_n2.mp3"},
	{4,0,NO_TALK,""}
};


// store the best weapon to use against each monsters
typedef struct sidekickAttackInfo
{
	int nFirstWeaponChoice;
    int nSecondWeaponChoice;
    int nThirdWeaponChoice;

} sidekickAttackInfo_t;

static sidekickAttackInfo_t aSidekickCombatData[MAX_MONSTERS_IN_LIST] =
{
    { 0,   1,   2 },		//	*EMPTY*				= 0
    { 0,   1,   2 },		//	TYPE_CLIENT			= 1		
    { 0,   1,   2 },		//	TYPE_MIKIKO			= 2
    { 0,   1,   2 },		//	TYPE_SUPERFLY		= 3
    { 0,   1,   2 },		//	TYPE_BOT			= 4	
    { 0,   1,   2 },		//	*EMPTY*				= 5
    { 0,   1,   2 },		//	*EMPTY*				= 6
    { 0,   1,   2 },		//	*EMPTY*				= 7
    { 0,   1,   2 },		//	*EMPTY*				= 8
    { 0,   1,   2 },		//	*EMPTY*				= 9
	//Weapon selection scheme
	// NSS[12/11/99]: Date of last update
	//  Episode 1
	{ 0,   1,   2 },		//	TYPE_ROBOCROC		= 10	
	{ 1,   1,   2 },		//	TYPE_SL_SKEET		= 11
    { 1,   1,   2 },		//	TYPE_TH_SKEET		= 12
	{ 0,   1,   2 },		//	TYPE_FROGINATOR		= 13
	{ 0,   1,   2 },		//	TYPE_VERMIN			= 14
	{ 0,   1,   2 },        //	TYPE_SLUDGEMINION	= 15
	{ 0,   1,   2 },		//  TYPE_SURGEON		= 16
    { 0,   1,   2 },		//	TYPE_INMATER		= 17
    { 0,   1,   2 },		//	TYPE_PRISONER		= 18
	{ 0,   1,   2 },		//	TYPE_BATTLEBOAR		= 19
	{ 0,   1,   2 },		//	TYPE_RAGEMASTER		= 20
    { 0,   1,   2 },		//	TYPE_TRACKATTACK	= 21
    { 0,   1,   2 },        //  TYPE_MISHIMAGUARD   = 22
    { 1,   1,   2 },		//	TYPE_LASERGAT		= 23
	{ 1,   1,   2 },		//	TYPE_CAMBOT			= 24
    { 0,   1,   2 },		//	TYPE_PSYCLAW		= 25
    { 1,   1,   2 },		//	TYPE_DEATHSPHERE	= 26
    { 0,   1,   2 },		//	TYPE_PROTOPOD		= 27
    { 0,   0,   0 },		//	TYPE_SKINNYWORKER	= 28
    { 0,   0,   0 },		//	TYPE_FATWORKER	    = 29
	
	//  Episode 2 // NSS[12/11/99]:Only Two weapons
	{ 0,   1,   2 },		//	TYPE_FERRYMAN		= 30	
	{ 0,   1,   2 },		//	TYPE_SKELETON		= 31
	{ 0,   1,   2 },		//	TYPE_SPIDER			= 32
	{ 0,   1,   2 },		//	TYPE_THIEF			= 33
	{ 0,   1,   2 },		//	TYPE_SATYR			= 34
	{ 0,   1,   2 },		//	TYPE_CENTURION		= 35
	{ 0,   1,   2 },		//	TYPE_HARPY			= 36
	{ 0,   0,   0 },		//	TYPE_COLUMN			= 37
    { 0,   1,   2 },		//	TYPE_SIREN			= 38
	{ 0,   1,   2 },		//	TYPE_GRIFFON		= 39
	{ 0,   1,   2 },		//	TYPE_CERBERUS		= 40
	{ 0,   1,   2 },		//	TYPE_MEDUSA			= 41
	{ 0,   1,   2 },		//	TYPE_CYCLOPS		= 42
	{ 0,   1,   2 },		//	TYPE_MINOTAUR		= 43
    { 0,   1,   2 },		//	TYPE_SMALLSPIDER    = 44
	{ 0,   1,   2 },		//	TYPE_KMINOS			= 45
    { 0,   1,   2 },		//	*EMPTY*				= 46
    { 0,   1,   2 },		//	*EMPTY*				= 47
    { 0,   1,   2 },		//	*EMPTY*				= 48
    { 0,   1,   2 },		//	*EMPTY*				= 49

	//  Episode 3
	{ 0,   1,   2 },		//	TYPE_DRAGONEGG		= 50	
	{ 0,   1,   2 },		//	TYPE_PRIEST			= 51
	{ 0,   1,   1 },		//	TYPE_ROTWORM		= 52
	{ 0,   1,   1 },		//	TYPE_PLAGUERAT		= 53
	{ 0,   1,   2 },		//	TYPE_BUBOID			= 54
	{ 0,   1,   1 },		//	TYPE_DOOMBAT		= 55
	{ 0,   1,   2 },		//	TYPE_FLETCHER		= 56
    { 0,   1,   2 },		//	TYPE_DWARF			= 57
    { 0,   0,   0 },		//	TYPE_LYCANTHIR		= 58
	{ 0,   1,   2 },		//	TYPE_CELESTRIL		= 59
    { 0,   1,   2 },		//	TYPE_WYNDRAX		= 60
	{ 0,   1,   2 },		//	TYPE_BABYDRAGON		= 61
    { 0,   1,   2 },		//	TYPE_NHARRE			= 62
	{ 0,   1,   2 },		//	TYPE_GARROTH		= 63
	{ 0,   1,   2 },		//	TYPE_KNIGHT1		= 64
	{ 0,   1,   2 },		//	TYPE_KNIGHT2		= 65
	{ 0,   1,   2 },		//	TYPE_WIZARD			= 66
	{ 0,   1,   2 },		//	TYPE_DRAGON			= 67
	{ 0,   1,   2 },		//	TYPE_STAVROS		= 68
    { 0,   1,   2 },		//	*EMPTY*				= 69
								
	//  Episode 4
	{ 0,   1,   2 },		//	TYPE_SEAGULL		= 70	
    { 0,   1,   2 },		//	TYPE_GOLDFISH		= 71
	{ 0,   1,   2 },		//	TYPE_SHARK			= 72
	{ 0,   1,   2 },		//	TYPE_SQUID			= 73
    { 0,   1,   2 },		//	TYPE_BLACKPRIS		= 74
	{ 0,   1,   2 },		//	TYPE_WHITEPRIS		= 75
	{ 0,   1,   2 },		//	TYPE_LABMONKEY		= 76
	{ 0,   1,   2 },		//	TYPE_FEMGANG		= 77
	{ 0,   1,   2 },		//	TYPE_PIPEGANG		= 78
	{ 0,   1,   2 },		//	TYPE_ROCKETGANG 	= 79
    { 0,   1,   2 },		//	TYPE_UZIGANG		= 80
	{ 0,   1,   2 },		//	TYPE_CHAINGANG		= 81
    { 0,   1,   2 },		//	TYPE_SDIVER			= 82
    { 0,   1,   2 },		//	TYPE_CRYOTECH		= 83
	{ 0,   1,   2 },		//	TYPE_NAVYSEAL		= 84
	{ 0,   1,   2 },		//	TYPE_SEALGIRL		= 85
	{ 0,   1,   2 },		//	TYPE_SEALCOMMANDO	= 86
	{ 0,   1,   2 },		//	TYPE_SEALCAPTAIN	= 87
	{ 0,   1,   2 },		//	TYPE_ROCKETMP		= 88
	{ 0,   1,   2 },		//	TYPE_KAGE			= 89
};


//#define INDEX_COMMAND_YES           0
//#define INDEX_COMMAND_NO            1
//#define INDEX_COMMAND_STAY          2
//#define INDEX_COMMAND_FOLLOW        3
//#define INDEX_COMMAND_ATTACK        4
//#define INDEX_COMMAND_NOATTACK      5
//#define INDEX_COMMAND_COMEHERE      6
//#define INDEX_COMMAND_PICKUP        7
//#define INDEX_COMMAND_LAST          8

#define SIDEKICK_COMMAND_YES            1
#define SIDEKICK_COMMAND_NO             2
#define SIDEKICK_COMMAND_STAY           3
#define SIDEKICK_COMMAND_FOLLOW         4
#define SIDEKICK_COMMAND_ATTACK         5
#define SIDEKICK_COMMAND_NOATTACK       6
#define SIDEKICK_COMMAND_COMEHERE       7
#define SIDEKICK_COMMAND_PICKUP         8
#define SIDEKICK_COMMAND_PICKUP_BAD     9
#define SIDEKICK_COMMAND_PICKUP_NO		 10
#define SIDEKICK_COMMAND_COMMAND       11
#define SIDEKICK_COMMAND_LAST          12

/*
static char *aSidekickCommandString[SIDEKICK_COMMAND_LAST][2] = 
{
		// command    filename substring
    { "",         ""          },
    { "yes",      "yes"       },
    { "no",       "no"        },
    { "stay",     "stay"      },
    { "follow",   "follow"    },
    { "attack",   "attack"    },
    { "noattack", "noattack", },
    { "comehere", "come"      },
    { "pickup",   "get"       },
    { "pickup",   "get_bad"   },
    { "pickup",   "get_no"    },
    { "",         "command"   }
};
*/
static char *aSidekickCommandString[SIDEKICK_COMMAND_LAST][2] = 
{
		// command    filename substring
    { "",					""          },
    { "yes",				"yes"       },
    { "no",					"no"        },
    { "sidekick_stay",		"stay"      },
    { "follow",				"follow"    },
    { "sidekick_attack",	"attack"    },
    { "sidekick_backoff",	"noattack", },
    { "sidekick_come",		"come"      },
    { "sidekick_get",		"get"       },
    { "pickup",   "get_bad"   },
    { "pickup",   "get_no"    },
    { "",         "command"   }
};

#define MAX_NUMBER_AMBIENTS     8

typedef struct ambientInfo
{
    float fWaitTime;
    float fRandomFactor;

} AMBIENT_INFO, *AMBIENT_INFO_PTR;

static AMBIENT_INFO aAmbientInfo[] = 
{
    { 30.0f,  0.05f },         // TASKTYPE_SIDEKICK_RANDOMWALK,
    { 25.0f,  0.05f },         // TASKTYPE_SIDEKICK_LOOKAROUND,
    { 20.0f,  0.05f },         // TASKTYPE_SIDEKICK_LOOKUP,
    { 20.0f,  0.05f },         // TASKTYPE_SIDEKICK_LOOKDOWN,
    { 25.0f,  0.05f },         // TASKTYPE_SIDEKICK_SAYSOMETHING,
    { 40.0f,  0.05f },         // TASKTYPE_SIDEKICK_KICKSOMETHING,
    { 30.0f,  0.05f },         // TASKTYPE_SIDEKICK_WHISTLE,
    { 50.0f,  0.05f },         // TASKTYPE_SIDEKICK_ANIMATE,

};

//static int nNumAmbients = sizeof(aAmbientInfo) / sizeof(aAmbientInfo[0]);

static float aNextAmbientTimes[2][MAX_NUMBER_AMBIENTS] = 
{
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },

};


// ****************************************************************************
// stuff to notify the player of the sidekicks' fighting status!
#define MK_NO_UPDATE			0x0f
#define MK_NO_WEAPON			0
#define MK_LOW_AMMO				1		//
#define MK_GOOD_AMMO			2		
#define MK_ROCKIN_AMMO			3

#define SF_NO_UPDATE			0xf0
#define SF_NO_WEAPON			0
#define SF_LOW_AMMO				16
#define SF_GOOD_AMMO			32
#define SF_ROCKIN_AMMO			48

static byte saveFlags = 0x55;			// 01010101


void SIDEKICK_UpdateWeaponStatus(userEntity_t *player, byte mk_flags, byte sf_flags)
{
	if (coop->value || deathmatch->value)
		return;

	if ((mk_flags | sf_flags) == 0xff)
		return;

	byte flags = 0;
	if (mk_flags != MK_NO_UPDATE)
	{
		flags |= mk_flags;
	}
	else
	{
		flags |= saveFlags & 0x0f;
	}

	if (sf_flags != SF_NO_UPDATE)
	{
		flags |= sf_flags;
	}
	else
	{
		flags |= saveFlags & 0xf0;
	}

	if (flags != saveFlags)
	{
		if (!gstate->gi->TestUnicast(player,true))
			return;

		AI_Dprintf("Update Sidekick Ammo Status!\n");
		saveFlags = flags;
		gstate->WriteByte(SVC_SIDEKICK_WEAPON_STATUS);
		gstate->WriteByte(flags);
		gstate->UniCast(player,true);
	}
}
/* ***************************** Local Functions *************************** */

int SIDEKICK_HandleFollowing( userEntity_t *self );
userEntity_t *SIDEKICK_FindClosestEnemy( userEntity_t *self );
void SIDEKICK_FreeInventory( userEntity_t *self );
void SIDEKICK_DropInventory( userEntity_t *self );
int AI_HasGoalInQueue( userEntity_t *self, GOALTYPE SearchGoal );
void SIDEKICK_InitWeapons( userEntity_t *self );
static void SIDEKICK_Init( userEntity_t *self, int nType );
void SIDEKICK_PreThink( userEntity_t *self );
void SIDEKICK_PostThink( userEntity_t *self );
void SIDEKICK_StartDie( userEntity_t *self, userEntity_t *inflictor, 
		  				       userEntity_t *attacker, int damage, CVector &point );
void SIDEKICK_Pain( userEntity_t *self, userEntity_t *other, float kick, int damage);
void SIDEKICK_Attack( userEntity_t *self );
void SIDEKICK_BeginAttack( userEntity_t *self );
void SIDEKICK_Start( userEntity_t *self, int nType );
userEntity_t *SIDEKICK_Spawn( userEntity_t *self, int nType );
int SIDEKICK_FindClearSpotFrom( userEntity_t *self,CVector &fromPoint, CVector &clearPoint);

int SIDEKICK_AttackOwnerFacingEnemy(userEntity_t *self);

userEntity_t *SIDEKICK_FindEnemy( userEntity_t *self );
void SIDEKICK_FindOwnerThink( userEntity_t *self );

void SIDEKICK_CommandAcknowledge(userEntity_t *self, int command_flag, qboolean PVS_required = false);
void PlaySidekickMP3(userEntity_t *self,char *sound);

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
/* ******************************* exports ********************************* */
DllExport void SIDEKICK_Alert( userEntity_t *owner, userEntity_t *target );
DllExport void info_superfly_start( userEntity_t *self );
DllExport void info_mikikofly_start( userEntity_t *self ); //NSS[11/30/99]:Added for mikikofly mesh mess
DllExport void info_mikiko_start( userEntity_t *self );
DllExport void trigger_mikiko_spawn( userEntity_t *self );
DllExport void trigger_superfly_spawn( userEntity_t *self );

void SIDEKICK_Think( userEntity_t *self );

void SIDEKICK_PermissionGiven( userEntity_t *self );
void SIDEKICK_PermissionDenied( userEntity_t *self );

void SIDEKICK_PickedUpItem( userEntity_t *self, const char *szItemName );

userInventory_t *SIDEKICK_ChooseBestWeaponForE1( userEntity_t *self, userEntity_t *pEnemy = NULL );
userInventory_t *SIDEKICK_ChooseBestWeaponForE2( userEntity_t *self, userEntity_t *pEnemy = NULL );
userInventory_t *SIDEKICK_ChooseBestWeaponForE3( userEntity_t *self, userEntity_t *pEnemy = NULL );
userInventory_t *SIDEKICK_ChooseBestWeaponForE4( userEntity_t *self, userEntity_t *pEnemy = NULL );

int SIDEKICK_MoveTowardPoint( userEntity_t *self, CVector &destPoint, 
						      int bMovingTowardNode = FALSE, int bFaceTowardDest = TRUE );
int SIDEKICK_MoveTowardPointWhileAttacking( userEntity_t *self, CVector &destPoint, 
						                    int bMovingTowardNode = FALSE, int bFaceTowardDest = TRUE );
int SIDEKICK_ReadAmbientInfo( userEntity_t *self, const char *szFileName );

/*
void FetchSidekickEntData( edict_t *pClientEnt, edict_t *pSidekickEnt, int nType );
*/

int SideKick_TalkAmbient(userEntity_t *self, enum Sidekick_Talk VoiceType, int OverRideTime);
int Check_Que(userEntity_t *other,int Type, float Offset);
int SIDEKICK_SendMessage(userEntity_t *other, int command, int An_Int, float A_Float, float B_Float, userEntity_t *sender,int Flag);

// NSS[1/19/00]:Some defines for functions
int SIDEKICK_IsClearShot(userEntity_t *self, userEntity_t *pEntity, CVector &OrgStart, CVector &OrgEnd);
int SIDEKICK_HasTaskInQue( userEntity_t *self,TASKTYPE Task);
int SIDEKICK_OwnerIsTooCloseToEnemy(userEntity_t *self);
int SIDEKICK_FindGoodShootingSpot(userEntity_t *self, userEntity_t *enemy, CVector *Position);

void FetchSidekickEntData( edict_t *pClientEnt, edict_t *pSidekickEnt, int nType );
float SIDEKICK_DeterminePathLength(userEntity_t *self, userEntity_t *target);

// ----------------------------------------------------------------------------
//								LOCAL functions
// ----------------------------------------------------------------------------
#define BOUND_MIN(val,min)		if (val < min) val = min
void AI_SaveSidekick( FILE *f, struct edict_s *ent )
{
	AI_SavePlayerHook( f, ent );

// SCG[2/24/00]: copy persistant data
	playerHook_t *pHook = AI_GetPlayerHook( ent );
	client_persistant_t	*pPersistant = &ent->client->pers;
	pPersistant->nHealth				= ent->health;
	pPersistant->record					= ent->record;
	pPersistant->nArmorVal				= ent->armor_val;
	pPersistant->nArmorAbs				= ent->armor_abs;
	pPersistant->nTeam					= ent->team;
	pPersistant->nItems					= pHook->items;
	pPersistant->nExpFlags				= pHook->exp_flags;
	pPersistant->fInvulnerabilityTime	= pHook->invulnerability_time;
	pPersistant->fEnvirosuitTime		= pHook->envirosuit_time;
	pPersistant->fWraithorbTime			= pHook->wraithorb_time;
	pPersistant->fBasePower				= pHook->base_power;
	pPersistant->fBaseAttack			= pHook->base_attack;
	pPersistant->fBaseSpeed				= pHook->base_speed;
	pPersistant->fBaseAcro				= pHook->base_acro;
	pPersistant->fBaseVita				= pHook->base_vita;
	pPersistant->fBoostPower			= pHook->power_boost;
	pPersistant->fBoostAttack			= pHook->attack_boost;
	pPersistant->fBoostSpeed			= pHook->speed_boost;
	pPersistant->fBoostAcro				= pHook->acro_boost;
	pPersistant->fBoostVita				= pHook->vita_boost;
	pPersistant->fTimePower				= pHook->power_boost_time;
	pPersistant->fTimeAttack			= pHook->attack_boost_time;
	pPersistant->fTimeSpeed				= pHook->speed_boost_time;
	pPersistant->fTimeAcro				= pHook->acro_boost_time;
	pPersistant->fTimeVita				= pHook->vita_boost_time;
	pPersistant->vAngles				= ent->s.angles;
	pPersistant->nAIFlags				= pHook->ai_flags;
	BOUND_MIN(pPersistant->fTimePower				,0);
	BOUND_MIN(pPersistant->fTimeAttack				,0);
	BOUND_MIN(pPersistant->fTimeSpeed				,0);
	BOUND_MIN(pPersistant->fTimeAcro				,0);
	BOUND_MIN(pPersistant->fTimeVita				,0);
	BOUND_MIN(pPersistant->fInvulnerabilityTime		,0);
	BOUND_MIN(pPersistant->fEnvirosuitTime			,0);
	BOUND_MIN(pPersistant->fWraithorbTime			,0);

	com->SavePersInventory(ent);
	memcpy( &ent->client->pers, pPersistant, sizeof( client_persistant_t ) );

	// SCG[2/24/00]: save client
	com->AI_SaveHook( f, ent->client, sizeof( gclient_s ) );
}

#define	SIDEKICK_PERS_MIKIKO	2
#define	SIDEKICK_PERS_SUPERFLY	3

void SidekickLoadThink( userEntity_t *self )
{
	playerHook_t *pHook = AI_GetPlayerHook( self );
	userEntity_t *pPlayer = NULL;

	unsigned int	nSidekickType;

	if( pHook->type == TYPE_SUPERFLY )
	{
		nSidekickType = SIDEKICK_PERS_SUPERFLY;
	}
	else if( pHook->type == TYPE_MIKIKO )
	{
		nSidekickType = SIDEKICK_PERS_MIKIKO;
	}

	userEntity_t *pEntity = alist_FirstEntity( client_list );
	while ( pEntity != NULL )
	{
		if( pEntity->className != NULL )
		{
			if ( _stricmp( pEntity->className, "player" ) == 0 )
			{
				pPlayer = pEntity;
				break;
			}
		}

		pEntity = alist_NextEntity( client_list );
	}

	if( ( pHook != NULL ) && ( pPlayer != NULL ) )
	{
		pHook->owner = pPlayer;
		if (nSidekickType == SIDEKICK_PERS_MIKIKO)
		{
			if( pPlayer->client->pers.pPersMikiko )
			{
				memcpy( &self->client->pers, pPlayer->client->pers.pPersMikiko, sizeof( client_persistant_t ) );
			}
			else
			{
				pPlayer->client->pers.pPersMikiko = &self->client->pers;
			}
		}
		else
		{
			if( pPlayer->client->pers.pPersSuperfly )
			{
				memcpy( &self->client->pers, pPlayer->client->pers.pPersSuperfly, sizeof( client_persistant_t ) );
			}
			else
			{
				pPlayer->client->pers.pPersSuperfly = &self->client->pers;
			}
		}
		FetchSidekickEntData( pPlayer, self, nSidekickType );

		AI_AddNewGoal( self, GOALTYPE_FOLLOW, pHook->owner );

		self->think = SIDEKICK_FindOwnerThink;
		self->nextthink = gstate->time + 0.1f;
	}
	else
	{
		self->think = SidekickLoadThink;
		self->nextthink = gstate->time + 1.0f;
	}
}

void AI_LoadSidekick( FILE *f, struct edict_s *ent )
{
	userEntity_t *pPlayer = NULL;
	userEntity_t *pSuperfly = AIINFO_GetSuperfly();
	userEntity_t *pMikiko = AIINFO_GetMikiko();
	unsigned int	nSidekickType;

	AI_LoadPlayerHook( f, ent );

	// SCG[2/6/00]: Allocate memory for the client
	gclient_t *client = ( gclient_t * ) gstate->X_Malloc( sizeof( gclient_t ), MEM_TAG_TEMP );
	com->AI_LoadHook( f, client, sizeof( gclient_s ) );

	ent->s.origin.z += 8;

	playerHook_t *pLoadHook = AI_GetPlayerHook( ent );

	ent->userHook = NULL;

	if( ( pLoadHook->type == TYPE_SUPERFLY ) || ( pLoadHook->type == TYPE_MIKIKOFLY ) )
	{
		if( pLoadHook->bCarryingMikiko )
		{
			SIDEKICK_Start( ent, TYPE_MIKIKOFLY );
		}
		else
		{
			SIDEKICK_Start( ent, TYPE_SUPERFLY );
		}
		nSidekickType = SIDEKICK_PERS_SUPERFLY;
	}
	else if( pLoadHook->type == TYPE_MIKIKO )
	{
		SIDEKICK_Start( ent, TYPE_MIKIKO );
		nSidekickType = SIDEKICK_PERS_MIKIKO;
	}

	// NSS[3/3/00]:Prevents the 'phatom weapons' from appearing.
	ent->s.modelindex2 = 0;

	ent->client = client;

	playerHook_t *pHook = AI_GetPlayerHook( ent );

	client_persistant_t	*pPersistant	= &ent->client->pers;
	pHook->stateFlags					= pLoadHook->stateFlags;
	pHook->items						= pPersistant->nItems;
	pHook->exp_flags					= pPersistant->nExpFlags;
	pHook->invulnerability_time			= pPersistant->fInvulnerabilityTime;
	pHook->envirosuit_time				= pPersistant->fEnvirosuitTime;
	pHook->wraithorb_time				= pPersistant->fWraithorbTime;
	pHook->base_power					= pPersistant->fBasePower;
	pHook->base_attack					= pPersistant->fBaseAttack;
	pHook->base_speed					= pPersistant->fBaseSpeed;
	pHook->base_acro					= pPersistant->fBaseAcro;
	pHook->base_vita					= pPersistant->fBaseVita;
	pHook->power_boost					= pPersistant->fBoostPower;
	pHook->attack_boost					= pPersistant->fBoostAttack;
	pHook->speed_boost					= pPersistant->fBoostSpeed;
	pHook->acro_boost					= pPersistant->fBoostAcro;
	pHook->vita_boost					= pPersistant->fBoostVita;
	pHook->power_boost_time				= pPersistant->fTimePower;
	pHook->attack_boost_time			= pPersistant->fTimeAttack;
	pHook->speed_boost_time				= pPersistant->fTimeSpeed;
	pHook->acro_boost_time				= pPersistant->fTimeAcro;
	pHook->vita_boost_time				= pPersistant->fTimeVita;
	ent->s.angles						= pPersistant->vAngles;
	pHook->ai_flags						= pPersistant->nAIFlags;
	ent->armor_val						= pPersistant->nArmorVal;
	ent->armor_abs						= pPersistant->nArmorAbs;
	
	

	gstate->X_Free( pLoadHook );

	ent->think = SidekickLoadThink;
	ent->nextthink = gstate->time + 1.0f;

}

void SIDEKICK_SetGodMode()
{
    bInGodMode = TRUE;

    userEntity_t *pFirstSidekick = AIINFO_GetFirstSidekick();
    if ( pFirstSidekick )
    {
        pFirstSidekick->takedamage = DAMAGE_NO;
    }

    userEntity_t *pSecondSidekick = AIINFO_GetSecondSidekick();
    if ( pSecondSidekick )
    {
        pSecondSidekick->takedamage = DAMAGE_NO;
    }

	gstate->Con_Dprintf( "Sidekick(s) in god mode." );
}

void SIDEKICK_ClearGodMode()
{
    bInGodMode = FALSE;

    userEntity_t *pFirstSidekick = AIINFO_GetFirstSidekick();
    if ( pFirstSidekick )
    {
        pFirstSidekick->takedamage = DAMAGE_YES;
    }

    userEntity_t *pSecondSidekick = AIINFO_GetSecondSidekick();
    if ( pSecondSidekick )
    {
        pSecondSidekick->takedamage = DAMAGE_YES;
    }

	gstate->Con_Dprintf( "Sidekick(s) no longer in god mode." );
}

int SIDEKICK_IsInGodMode()
{
    return bInGodMode;
}

void SIDEKICK_ToggleGodMode()
{
    if ( SIDEKICK_IsInGodMode() )
    {
        SIDEKICK_ClearGodMode();
    }
    else
    {
        SIDEKICK_SetGodMode();
    }
}

// ----------------------------------------------------------------------------

int SIDEKICK_IsCarryingMikiko( playerHook_t *hook )
{
    return hook->bCarryingMikiko;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_IsStaying
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsStaying( playerHook_t *hook )
{
    if ( hook->ai_flags & AI_STAY )
    {
        return TRUE;
    }

    return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_SetAttackTime
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
 void SIDEKICK_SetAttackTime( playerHook_t *hook, float fTime )
{
    hook->fAttackTime = fTime;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_IsAboutToFire
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsAboutToFire( playerHook_t *hook )
{
    if ( gstate->time >= hook->attack_finished || gstate->time >= hook->attack_finished + 0.2f )
    {
        return TRUE;
    }

    return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_IsSpecialWeapon
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsSpecialWeapon( weapon_t *pWeapon )
{
    _ASSERTE( pWeapon );

    int nCurrentEpisode = GetCurrentEpisode();
    if ( nCurrentEpisode == 2 &&
         ( _stricmp( pWeapon->name, "weapon_discus" ) == 0 ||
           _stricmp( pWeapon->name, "weapon_venomous" ) == 0 )
       )
    {
        return TRUE;
    }

    return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_CanUseWeapon
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_CanUseWeapon( weapon_t *pWeapon )
{
    _ASSERTE( pWeapon );
    if ( pWeapon->ammo->count > 0 || SIDEKICK_IsSpecialWeapon( pWeapon ) )
    {
        return TRUE;
    }

    return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:        AI_GetCurrentWeaponRange
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float AI_GetCurrentWeaponRange( userEntity_t *self )
{
	weapon_t *pWeapon = (weapon_t *) self->curWeapon;

    if( pWeapon == NULL )
	{
		return 0;
	}

    float fRange = pWeapon->winfo->range;
    
	if ( pWeapon && self->enemy )
    {
	    int nCurrentEpisode = GetCurrentEpisode();
        if ( nCurrentEpisode == 2 && 
             self->health > 50 && _stricmp( pWeapon->name, "weapon_discus" ) == 0 )
        {
            float fDistance = VectorDistance(self->s.origin,self->enemy->s.origin);
			if(fDistance <= SIDEKICK_MELEE_ATTACK_DISTANCE)
			{
				fRange = SIDEKICK_MELEE_ATTACK_DISTANCE - 4.0f;
			}
        }
        else
        if ( nCurrentEpisode == 2 && pWeapon->ammo->count == 0 &&
             _stricmp( pWeapon->name, "weapon_venomous" ) == 0 )
        {
            float fDistance = VectorDistance(self->s.origin,self->enemy->s.origin);
			if(fDistance <= SIDEKICK_MELEE_ATTACK_DISTANCE || pWeapon->ammo->count == 0)
			{
				fRange = SIDEKICK_MELEE_ATTACK_DISTANCE - 4.0f;
			}
        }
    }
	fRange = pWeapon->winfo->range;

    return fRange;

}


// ----------------------------------------------------------------------------
// NSS[12/11/99]:
// Name:		SIDEKICK_IsFacingEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsFacingEnemy( userEntity_t *self, userEntity_t *pEnemy, float maxdist, float maxyaw, float maxpitch)
{
	_ASSERTE( self );
    _ASSERTE( pEnemy );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	// replace default (invalid) distance
	if ( maxdist == -1.0f )
	{
		weapon_t *pWeapon	= (weapon_t *) self->curWeapon;
		weaponInfo_t *winfo = (weaponInfo_t *)pWeapon->winfo;
		if ( winfo )
		{
			maxdist = winfo->range;
		}
		else
		{
			maxdist = 256.0f;
		}
	}

	CVector dir = pEnemy->s.origin - self->s.origin;
	dir.Normalize();
	CVector angleTowardEnemy;
	VectorToAngles( dir, angleTowardEnemy );
	
	float fTowardEnemyY = AngleMod(angleTowardEnemy.y);
	float fFacingAngleY = AngleMod(self->s.angles.yaw);

	float fYawDiff = fabs( fTowardEnemyY - fFacingAngleY );
	if ( fYawDiff < maxyaw || fYawDiff > (360.0f - maxyaw) )
    {
	    float fTowardEnemyX = AngleMod(angleTowardEnemy.x);
	    float fFacingAngleX = AngleMod(self->s.angles.pitch);
	    float fPitchDiff = fabs( fTowardEnemyX - fFacingAngleX );
		float fDistance = VectorDistance( self->s.origin, pEnemy->s.origin );
        if ( ( fPitchDiff < maxpitch || fPitchDiff > (360.0f - maxpitch) ) && fDistance < maxdist)
	    {
		    return TRUE;
	    }
    }
	return FALSE;
}


// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_GetAmbientTimes
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float *SIDEKICK_GetNextAmbientTimes( userEntity_t *self )
{
    float *pNextAmbientTimes = NULL;
    if ( AIINFO_GetFirstSidekick() == self )
    {
        pNextAmbientTimes = aNextAmbientTimes[0];
    }
    else
    {
        pNextAmbientTimes = aNextAmbientTimes[1];
    }

    return pNextAmbientTimes;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_SetNextAmbientTime
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SetNextAmbientTime( userEntity_t *self, TASKTYPE nTaskType )
{
    _ASSERTE( nTaskType >= TASKTYPE_SIDEKICK_RANDOMWALK && nTaskType <= TASKTYPE_SIDEKICK_ANIMATE );

    int nIndex = nTaskType - TASKTYPE_SIDEKICK_RANDOMWALK;

    float *pNextAmbientTimes = SIDEKICK_GetNextAmbientTimes( self );

    pNextAmbientTimes[nIndex] = gstate->time + aAmbientInfo[nIndex].fWaitTime;

}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_IsRandomWalkTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float SIDEKICK_GetRandomWalkDistance( playerHook_t *hook )
{
    return hook->follow_dist * 2.0f;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_IsRandomWalkTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsRandomWalkTask( TASKTYPE nTaskType )
{
    if ( nTaskType == TASKTYPE_SIDEKICK_RANDOMWALK )
    {
        return TRUE;
    }

    return FALSE;
}

// ----------------------------------------------------------------------------
// NSS[2/1/00]:
// Name:        SIDEKICK_IsAmbientTask
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsAmbientTask( TASKTYPE nTaskType )
{
    switch(nTaskType)
	{
		case TASKTYPE_SIDEKICK_RANDOMWALK:
		case TASKTYPE_SIDEKICK_LOOKAROUND:
		case TASKTYPE_SIDEKICK_LOOKUP:
		case TASKTYPE_SIDEKICK_LOOKDOWN:
		case TASKTYPE_SIDEKICK_SAYSOMETHING:
		case TASKTYPE_SIDEKICK_KICKSOMETHING:
		case TASKTYPE_SIDEKICK_WHISTLE:
		case TASKTYPE_SIDEKICK_ANIMATE:
		{
		  return TRUE;
		}
	}	
    return FALSE;
}


// ----------------------------------------------------------------------------
// NSS[1/13/00]:
// Name:        SIDEKICK_CheckMonsterType
// Description: Checks the current weapon and the enemy and makes sure that the
// two are compatible.  
// Input: userEntity_t *self, userEntity_t *monster
// Output:TRUE or FALSE
// Note:
// ----------------------------------------------------------------------------
int SIDEKICK_CheckMonsterType(userEntity_t *self, userEntity_t *monster)
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
//	userEntity_t *client_entity = &gstate->g_edicts[1];	// SCG[1/23/00]: not used
	weapon_t *pWeapon = (weapon_t *) self->curWeapon;
	
	// NSS[1/13/00]:Specific monsters to run from when we have a melee weapon
	if (pWeapon)
	{
		if((!_stricmp(pWeapon->name,"weapon_silverclaw"))||(!_stricmp(pWeapon->name,"weapon_disruptor")))
		{
			if(self->enemy)
			{
				playerHook_t *mhook = AI_GetPlayerHook(self->enemy);
				if(!mhook)
				{
					return TRUE;
				}
				switch(mhook->type)
				{
					case TYPE_SLUDGEMINION:
					case TYPE_INMATER:
					case TYPE_RAGEMASTER:
					case TYPE_DEATHSPHERE:
					case TYPE_BATTLEBOAR:
					case TYPE_PSYCLAW:
					case TYPE_SL_SKEET:
					case TYPE_TH_SKEET:
					case TYPE_LASERGAT:
					case TYPE_DOOMBAT:			
					case TYPE_DRAGON:
					case TYPE_GARROTH:
					case TYPE_NHARRE:
					case TYPE_STAVROS:
					case TYPE_KNIGHT1:
					case TYPE_WYNDRAX:
					case TYPE_KNIGHT2:
					case TYPE_COLUMN:// NSS[1/13/00]:Always run from Columns
					case TYPE_CERBERUS:
					case TYPE_MEDUSA:
					{
						return FALSE;
					}
				}
			}
		}

		// NSS[1/13/00]:Always run from Columns
		if(self->enemy)
		{
			playerHook_t *mhook = AI_GetPlayerHook(self->enemy);
			if(mhook && (mhook->type == TYPE_COLUMN || mhook->type == TYPE_MEDUSA || mhook->type == TYPE_CERBERUS))
				return FALSE;
				
		}
		
	}
	// NSS[2/1/00]:We don't have a weapon or we made it through the loop, let's use regular processing
	return TRUE;
}



// ----------------------------------------------------------------------------
// NSS[1/13/00]:
// Name:        SIDEKICK_IsOkToFight
// Description:Determine if the entity has enough health to survive a fight
// Input:userEntity_t *self 
// Output:TRUE or FALSE
// Note:
// ----------------------------------------------------------------------------
int SIDEKICK_IsOkToFight( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	weapon_t *pWeapon = (weapon_t *) self->curWeapon;

	if(self->enemy && (self->enemy->flags & FL_EXPLOSIVE))
		return FALSE;
	
	if(!self->enemy && !(hook->ai_flags & AI_SIDEKICKKILL))
	{
		return FALSE;
	}
	
	if ( (hook->ai_flags & AI_NOATTACK) || self->health < 15 || SIDEKICK_IsCarryingMikiko( hook ) || !pWeapon )
    {
		return FALSE;
    }
	if ( !SIDEKICK_CanUseWeapon( pWeapon ) )
	{
		return FALSE;
	}

	
	return SIDEKICK_CheckMonsterType(self, self->enemy);
}



// ----------------------------------------------------------------------------

int SIDEKICK_IsInAttackMode( playerHook_t *hook )
{
    
	return hook->bInAttackMode;
}

// ----------------------------------------------------------------------------
// NSS[12/6/99]:
// Name:        SIDEKICK_SetAttackMode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SetAttackMode( userEntity_t *self, userEntity_t *pEnemy )
{
    _ASSERTE( self );
    _ASSERTE( pEnemy );
    
	playerHook_t *hook = AI_GetPlayerHook(self);
    
	
	if(hook)
	{
		// NSS[1/19/00]:Set our enemy;
		self->enemy = pEnemy;
		
		// NSS[1/16/00]:Make sure we have the right kind of weapon
		SIDEKICK_ChooseBestWeapon( self );

		
		// NSS[12/15/99]:Make sure we are 'ok' to fight.
		if (!hook->bCarryingMikiko && !SIDEKICK_HasTaskInQue(self,TASKTYPE_SIDEKICK_ATTACK) && SIDEKICK_IsOkToFight( self ))
		{
				AI_AddNewTaskAtFront(self,TASKTYPE_SIDEKICK_ATTACK);
		}
		//else if (!SIDEKICK_IsOkToFight( self ) && !SIDEKICK_IsEvading(self))
		else if (!SIDEKICK_IsOkToFight( self ) && !SIDEKICK_HasTaskInQue(self,TASKTYPE_SIDEKICK_EVADE))
		{
			AI_AddNewTaskAtFront(self,TASKTYPE_SIDEKICK_EVADE,self->enemy);
		}
	}
}

// ----------------------------------------------------------------------------

void SIDEKICK_ClearAttackMode( userEntity_t *self )
{
    _ASSERTE( self );
    playerHook_t *hook = AI_GetPlayerHook( self );

		int bOkToResetGoal = TRUE;

		// if the owner is too far away, run toward the owner always keep a close 
		// distance to owner
		userEntity_t *pOwner = hook->owner;
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		if ( pCurrentGoal )
		{
			TASK_PTR pCurrentTask = GOAL_GetCurrentTask( pCurrentGoal );
			if ( pCurrentTask )
			{
				TASKTYPE nTaskType = TASK_GetType( pCurrentTask );
				if ( nTaskType == TASKTYPE_SIDEKICK_EVADE )
				{
					bOkToResetGoal = FALSE;
				}
			}
		}
    
		if ( bOkToResetGoal && AI_IsAlive( pOwner ) && !SIDEKICK_IsStaying( hook ) )
		{
			GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
			if ( nGoalType == GOALTYPE_FOLLOW )
			{
				GOAL_ClearTasks( pCurrentGoal );
				AI_AddInitialTasksToGoal( self, pCurrentGoal, TRUE );
			}
			else
			{
				GOALSTACK_ClearAllGoals( pGoalStack );
				AI_AddNewGoal( self, GOALTYPE_FOLLOW, pOwner );
			}
		}

		SIDEKICK_ResetAmbientTimes( self );
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_ComputeAttackAnimationTime
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float SIDEKICK_ComputeAttackAnimationTime( userEntity_t *self )
{
    weaponInfo_t *winfo = (weaponInfo_t *)self->winfo;
    _ASSERTE( winfo );

    playerHook_t *hook = AI_GetPlayerHook( self );

    float fFrameTime = 0.03f;
    float fAnimTime = winfoNumFrames(ANIM_SHOOT_STD)*fFrameTime + 0.1f;
    if ( hook->attack_boost )
    {
        fFrameTime = fFrameTime / ((hook->attack_boost+1) * 0.3);
        fAnimTime = (winfoNumFrames(ANIM_SHOOT_STD)*fFrameTime) + 0.1f;
    }

    return fAnimTime;
}


// ----------------------------------------------------------------------------
// NSS[12/14/99]:
// Name:        SIDEKICK_InAttackRange
// Description:Are we within range?
// Input:userEntity_t *self, float fDistance, userEntity_t *enemy
// Output:TRUE/FALSE
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_InAttackRange(userEntity_t *self, float fDistance, userEntity_t *enemy)
{
	
	float fWeaponRange = AI_GetCurrentWeaponRange( self );
	
	if(!enemy)
	{
		if(self->enemy)
		{
			enemy = self->enemy;
		}
		else
		{
			return TRUE;
		}
	}

	if((fWeaponRange > fDistance || fDistance < 94.0f) && SIDEKICK_IsClearShot(self,enemy,self->s.origin,enemy->s.origin))
		return TRUE;
	
	return FALSE;
	// hook->fnInAttackRange( self, fDistance, enemy )
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_ComputeAnimationTime
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float SIDEKICK_ComputeAnimationTime( playerHook_t *hook )
{
	_ASSERTE( hook );

	if ( !hook->cur_sequence )
	{
		return 0.0f;
	}

	frameData_t *pCurrentSequence = hook->cur_sequence;
    _ASSERTE( pCurrentSequence );

	float fTime = (pCurrentSequence->last - pCurrentSequence->first + 1) * 0.1f;

	return fTime;
}


// ----------------------------------------------------------------------------
// NSS[12/16/99]:
// Name:		Sidekick_Touch
// Description: A touch function for the Sidekicks to keep them in check with one another.
// Input:	userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf
// Output: NA
// Note:
//
// ----------------------------------------------------------------------------
void Sidekick_Touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	if(other)
	{
		playerHook_t *ohook = AI_GetPlayerHook( other );
		if(ohook)
		{
			if ( (ohook->type == TYPE_MIKIKO || ohook->type == TYPE_SUPERFLY) && (self->velocity.Length() == 0.0f))
			{
				SIDEKICK_HandleMoveAway( self, other );
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_Init
// Description:
// Input:
// Output:
// Note:
//				self->s.origin should be set before calling this function
//
// ----------------------------------------------------------------------------
void SIDEKICK_Init( userEntity_t *self, int nType )
{
	_ASSERTE( self );
	int MikikoFly = 0;
	playerHook_t *hook = AI_GetPlayerHook( self );
	//	set the size
	gstate->SetSize( self, -16.0f, -16.0f, -24.0f, 16.0f, 16.0f, 32.0f );
	Superflywait		= 0.0f;	// NSS[3/4/00]:These two are used to determine the time between commands.
	MikikoWait			= 0.0f;
	SuperFlyWaitIdle	= 0.0f;	// NSS[3/4/00]:These two are used to determine the time in which they are in an IDLE state.
	MikikoWaitIdle		= 0.0f;

	SuperflyLastTalk	= gstate->time + 10.0f;
	MikikoLastTalk		= gstate->time + 10.0f;
	
	memset(SuperflyCom,0,sizeof(SuperflyCom));
	memset(SuperflyCom,0,sizeof(MikikoCom));
	
	if ( nType == TYPE_MIKIKO )
	{
		self->className = MIKIKO_CLASSNAME;

		self->netname	= tongue_world[T_MIKIKO_IBIHARA];
		self->scriptname = MIKIKO_CLASSNAME;
	}
	else
	if ( nType == TYPE_SUPERFLY || nType == TYPE_MIKIKOFLY)
	{
		//NSS[11/30/99]:
		
		
		if(nType == TYPE_MIKIKOFLY)
		{
			MikikoFly = 1;
			hook->type = TYPE_SUPERFLY;			//Not my idea, but hey some of the code was already there why not use it.
			self->className = SUPERFLY_CLASSNAME;
		}	
		else
		{
			self->className	= SUPERFLY_CLASSNAME;
		}

		self->netname	= tongue_world[T_SUPERFLY_JOHNSON];
		self->scriptname = SUPERFLY_CLASSNAME;
	}
	else
	if ( nType == TYPE_CLIENT )
	{
	    self->className = HIRO_CLASSNAME;

		self->netname	= tongue_world[T_HIRO_MIYAMOTO];
		self->scriptname = HIRO_CLASSNAME;
	}

	self->save = AI_SaveSidekick;
	self->load = AI_LoadSidekick;

	// init model information
	/////////////////////////////////////////////////////////
	//	make sure we're drawing the models
	/////////////////////////////////////////////////////////
	self->s.renderfx	&= ~RF_NODRAW;

	char *szModelName;
	if(MikikoFly)
	{
		szModelName = AIATTRIBUTE_GetModelName( MIKIKOFLY_CLASSNAME );	
	}
	else
	{
		szModelName = AIATTRIBUTE_GetModelName( self->className );
	}

    _ASSERTE( szModelName );
    self->s.modelindex  = gstate->ModelIndex( szModelName );
	self->modelName		= szModelName;

	// initialize the animation frame data
	if (!ai_get_sequences(self))
	{
		char *szCSVFileName = AIATTRIBUTE_GetCSVFileName( self->className );
        _ASSERTE( szCSVFileName );
        FRAMEDATA_ReadFile( szCSVFileName, self );
	}

	// amw: right now zero means don't look at anyone
	self->s.numClusters     = 0;

	// init animation info
	self->s.frameInfo.frameInc			= 1;
	self->s.frameInfo.frameTime			= FRAMETIME_FPS20;
	self->s.frameInfo.next_frameTime	= gstate->time + self->s.frameInfo.frameTime;
	
	
	//	server and physics
	self->movetype			= MOVETYPE_WALK;
	self->solid				= SOLID_BBOX;
//	self->clipmask			= MASK_NPCSOLID;
	self->clipmask			= MASK_MONSTERSOLID;
	self->flags				|= FL_BOT;// | FL_PUSHABLE;
	self->elasticity		= 1.0;
	self->mass				= 300.0;
	self->max_speed			= SIDEKICK_RUNSPEED;
	self->svflags			|= SVF_MONSTER;
	self->svflags			&= ~SVF_DEADMONSTER;
	self->s.effects			= EF_GIB;				// this makes server send client info to the clients //???
// SCG[12/7/99]: 	self->s.renderfx		= RF_CLUSTERBASED;		// makes this model draw with cluster based animation
	self->s.iflags			= IF_SV_SENDBBOX;		// send the real BBOX size down to client for prediction	

	self->ang_speed.Set( 90.0f, 90.0f, 90.0f );
	AI_ZeroVelocity( self );

	self->s.effects			= 0;
	self->s.origin.z		+= 0.125;		// make sure off ground

	//	set up skins
	self->s.skinnum			= 0;
	self->s.frame			= 0;

	self->viewheight		= 22.0f;
	self->health			= ATTR_HEALTH;
	self->takedamage		= DAMAGE_AIM;
	self->deadflag			= DEAD_NO;

	//	thinks
	self->prethink			= SIDEKICK_PreThink;
	self->postthink			= SIDEKICK_PostThink;
	self->pain				= SIDEKICK_Pain;
	self->die				= SIDEKICK_StartDie;

	self->goalentity		= self;
	self->enemy				= NULL;

	self->PickedUpItem		= SIDEKICK_PickedUpItem;

	self->touch				= Sidekick_Touch;
	
	ai_setfov( self, 270.0f );


	hook->dflags &= ~DFL_LLAMA;	// SCG[1/5/00]: 

	hook->upward_vel		= 125.0;
	hook->forward_vel		= 350.0;
	hook->max_jump_dist		= ai_max_jump_dist(hook->upward_vel, hook->forward_vel);

	hook->attack_dist		= 2000.0;
	hook->run_speed			= SIDEKICK_RUNSPEED;
	hook->walk_speed		= SIDEKICK_WALKSPEED;
	hook->attack_speed		= SIDEKICK_RUNSPEED;
	hook->jump_chance		= 0;
	hook->back_away_dist	= 0;
	hook->active_distance	= 8192;

	hook->think_time		= THINK_TIME;

	hook->fnStartAttackFunc = NULL;//SIDEKICK_BeginAttack;
	hook->fnAttackFunc		= NULL;//SIDEKICK_Attack;

	hook->bOkToAttack		= TRUE;
	hook->nTargetFrequency	= 5;

	hook->bInTransition		= FALSE;
	hook->nTransitionIndex	= 0;
	hook->fTransitionTime	= 0.0f;
	hook->pTransitions		= NULL;

	// misc hook info
	hook->fnFindTarget		= NULL;//SIDEKICK_FindEnemy;
	hook->dflags	        &= ~(DFL_EVIL | DFL_NEUTRAL | DFL_BERSERK);
	hook->dflags			= DFL_RANGEDATTACK + DFL_CANSTRAFE + DFL_GOOD;

	hook->fov_desired		= 90.0;
	hook->fov_rate			= 0.0;

	// initialize timer info
	hook->strafe_time		= gstate->time;
	hook->strafe_dir		= STRAFE_LEFT;

	//	poison stuff
	hook->poison_time				= 0;
	hook->poison_next_damage_time	= 0;
	hook->poison_damage				= 0;
	hook->poison_interval			= 0;

	//	power ups
	hook->invulnerability_time	= 0;
	hook->envirosuit_time		= 0;
	hook->wraithorb_time		= 0;
	hook->oxylung_time			= 0;

	//	temporary attribute boosts
	hook->power_boost_time		= 0;
	hook->attack_boost_time		= 0;
	hook->speed_boost_time		= 0;
	hook->acro_boost_time		= 0;
	hook->vita_boost_time		= 0;

	//	other timer fields
	hook->pain_finished			= 0;
	hook->drown_time			= gstate->time + 12.0;
	hook->attack_finished		= 0;
	hook->jump_time				= 0;
	hook->sound_time			= 0;
	hook->path_time				= 0;
	hook->think_time			= 0.1;
	hook->shake_time			= 0;
	hook->ambient_time			= 0;

	//	clear flags
	hook->exp_flags				= 0;

	//	Nelno:	never finished implementing this...
	hook->fatigue				= 0;

	hook->fnInAttackRange		= SIDEKICK_InAttackRange;

	// now initialize inventory
	if ( self->inventory )
	{
		//	client already has an inventory, so free it and set ptr to NULL
		SIDEKICK_FreeInventory(self);
	}

    self->inventory = gstate->InventoryNew(MEM_MALLOC);

	hook->follow_dist		= 424.0;
	hook->walk_follow_dist	= 100.0;

	self->think			    = SIDEKICK_FindOwnerThink;		//	waits for owner to connect
	AI_SetNextThinkTime( self, 0.2f );

	hook->fSoundOutDistance = 0.0f;

    hook->nAttackMode       = ATTACKMODE_NORMAL;

    hook->pWantItem         = NULL;
    hook->bInAttackMode     = FALSE;
    hook->bAttackPlayer     = FALSE;
	hook->bCarryingMikiko   = MikikoFly;
    hook->bJustFired        = 0;
	hook->type				= nType;		//NSS[11/30/99]:Added

	hook->dflags  |= DFL_CANUSELADDER|DFL_CANUSEDOOR;// NSS[2/12/00]:They can use both

	AIATTRIBUTE_SetInfo( self );

    // update client on sidekick's info
    if ( hook->type == TYPE_SUPERFLY || hook->type == TYPE_MIKIKOFLY)
    {
		com->Sidekick_Update(self,SIDEKICK_SUPERFLY,SIDEKICK_STATUS_HEALTH,100 * (self->health / hook->base_health)); // update health
    }
    else
    {
	    com->Sidekick_Update(self,SIDEKICK_MIKIKO,SIDEKICK_STATUS_HEALTH,100 * (self->health / hook->base_health)); // update health
    }

	SIDEKICK_FindOwner( self );

	//NSS[11/27/99]:Flag which will allow the sidekicks to swim around and follow you into water until they start taking damage.
	hook->ai_flags |=AI_CANSWIM;
	hook->ai_flags |=AI_FOLLOW;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_InitWeapons
// Description:
// Input:
// Output:
// Note:
//			Inits all variables related to client weapons
//
// ----------------------------------------------------------------------------
void SIDEKICK_InitWeapons( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	self->curWeapon				= NULL;

	hook->fxFrameNum			= 0;
	hook->fxFrameFunc			= NULL;

	hook->last_button0			= 0;
//	hook->viewmodel_start_frame = 0;
//	hook->viewmodel_end_frame	= 0;
//	hook->viewmodel_frame_time	= 0;

	hook->weapon_fired			= NULL;
	hook->weapon_next			= NULL;

	// clear out armor also
	self->armor_val				= 0;
	self->armor_abs				= 0.0f;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_FreeWeapon
// Description:
// Input:
// Output:
// Note:
//			untracks the client's weapon and removes the entity associated with it
//
// ----------------------------------------------------------------------------
void SIDEKICK_FreeWeapon( userEntity_t *self )
{
	self->curWeapon = NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_InitDefaultWeapon
// Description:
// Input:
// Output:
// Note:
//			Give client default weapon
//
// ----------------------------------------------------------------------------
void SIDEKICK_InitDefaultWeapon( userEntity_t* self )
{
	_ASSERTE( self );
	// give client a weapon if deathmatch
	_ASSERTE( self->inventory );
	
	weaponInfo_t *winfo = NULL;
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	int nCurrentEpisode = GetCurrentEpisode();
	switch ( nCurrentEpisode )
	{
		case 1:
		{
			
			if(hook && !hook->bCarryingMikiko)
			{
				winfo = com->FindRegisteredWeapon( "weapon_disruptor" );
				if(winfo)
				{
					winfo->give_func( self, winfo->initialAmmo );
					winfo->select_func( self );
				}

				// NSS[1/10/00]:Added server var to allow sidekicks to have all weapons upon start
				if(gstate->GetCvar("sv_sidekickweapons"))
				{
					winfo = com->FindRegisteredWeapon( "weapon_ionblaster" );
					if(winfo)
					{
						winfo->give_func( self, winfo->initialAmmo );
						winfo->select_func( self );
					}
					winfo = com->FindRegisteredWeapon( "weapon_shotcycler" );
					if(winfo)
					{
						winfo->give_func( self, winfo->initialAmmo );
						winfo->select_func( self );
					}
				}

			}
			break;
		}	

		case 2:
		{
			
				// NSS[1/10/00]:Added server var to allow sidekicks to have all weapons upon start
				if(gstate->GetCvar("sv_sidekickweapons"))
				{
					if(hook && !hook->bCarryingMikiko)
					{
						winfo = com->FindRegisteredWeapon( "weapon_discus" );
						if(winfo)
						{
							winfo->give_func( self, winfo->initialAmmo );
							winfo->select_func( self );
						}

						winfo = com->FindRegisteredWeapon( "weapon_venomous" );
						if(winfo)
						{
							winfo->give_func( self, winfo->initialAmmo );
							winfo->select_func( self );
						}

						winfo = com->FindRegisteredWeapon( "weapon_trident" );
						if(winfo)
						{
							winfo->give_func( self, winfo->initialAmmo );
							winfo->select_func( self );
						}

					}
				}
            break;
		}
		case 3:
		{
			// NSS[1/10/00]:Added server var to allow sidekicks to have all weapons upon start
			if(hook && !hook->bCarryingMikiko && gstate->GetCvar("sv_sidekickweapons"))
			{
				winfo = com->FindRegisteredWeapon( "weapon_silverclaw" );
				if(winfo)
				{
					winfo->give_func( self, winfo->initialAmmo );
					winfo->select_func( self );
				}

				winfo = com->FindRegisteredWeapon( "weapon_bolter" );
				if(winfo)
				{
					winfo->give_func( self, winfo->initialAmmo );
					winfo->select_func( self );
				}
			}
            break;
		}
		case 4:
		{
			// NSS[1/10/00]:Added server var to allow sidekicks to have all weapons upon start
			if(gstate->GetCvar("sv_sidekickweapons"))
			{
				if(hook && !hook->bCarryingMikiko)
				{
					winfo = com->FindRegisteredWeapon( "weapon_glock" );
					winfo->give_func( self, winfo->initialAmmo );
					winfo->select_func( self );
					winfo = com->FindRegisteredWeapon( "weapon_ripgun" );
					winfo->give_func( self, winfo->initialAmmo );
					winfo->select_func( self );
					winfo = com->FindRegisteredWeapon( "weapon_slugger" );
					winfo->give_func( self, winfo->initialAmmo );
					winfo->select_func( self );

				}
			}
            break;
		}
		default:
        {
			break;
        }
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_FreeInventory
// Description:
// Input:
// Output:
// Note:
//			Frees inventory when a player restarts (single player) or respawns 
//			(coop and deathmatch)
//
// ----------------------------------------------------------------------------
void SIDEKICK_FreeInventory( userEntity_t *self )
{
	_ASSERTE( self );

	if ( !self->inventory )
	{
		return;
	}

	gstate->InventoryFree(self->inventory);
	self->inventory = NULL;

	//	make sure there are no pointers to weapons left
	SIDEKICK_InitWeapons(self);

}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_DropInventory
// Description:
// Input:
// Output:
// Note:
//			makes client drop their weapon and ammo in deathmatch
//
// ----------------------------------------------------------------------------
void SIDEKICK_DropInventory( userEntity_t *self )
{
	_ASSERTE( self );

	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	if ( !self->inventory )
	{
		return;
	}

	weaponInfo_t *winfo = (weaponInfo_t *) self->winfo;
	//	drop the client's current weapon with current amount of ammo
	if ( winfo )
	{
		// no model should be drawn on/with the client model
		self->s.modelindex2 = 0;
        
		weapon_t *pWeapon = (weapon_t *) self->curWeapon;
        if ( pWeapon && pWeapon->ammo->count > 0 && !(winfo->flags & WFL_NODROP) )
        {
		    userEntity_t *pNewWeapon = gstate->SpawnEntity();
		    pNewWeapon->className	= winfo->weaponName;
		    pNewWeapon->netname		= winfo->weaponNetName;
		    pNewWeapon->spawnflags	= (DROPPED_ITEM|DROPPED_PLAYER_ITEM);

		    //	set origin
		    pNewWeapon->s.origin = self->s.origin;
		    //	give some velocity
		    pNewWeapon->velocity.x = rnd() * 300.0f - 200.0f;
		    pNewWeapon->velocity.y = rnd() * 300.0f - 200.0f;
		    pNewWeapon->velocity.z = rnd() * 200.0f + 200.0f;

		    winfo->spawn_func( winfo, pNewWeapon, -1.0f, NULL );

		    //	find out how much ammo player has for this weapon,
		    //	and store it in the weapon pickup item
		    pNewWeapon->hacks_int = pWeapon->ammo->count;
	    }
    }

	//	clear everything having to do with weapons
	SIDEKICK_FreeWeapon(self);
	SIDEKICK_InitWeapons(self);

}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_InitStats
// Description:
// Input:
// Output:
// Note:
//			Inits deathmatch statistics
//
// ----------------------------------------------------------------------------
void SIDEKICK_InitStats( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	hook->killed	= 0;
	hook->died		= 0;
	hook->fired		= 0;
	hook->hit		= 0;

}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_FreeNodeSystem
// Description:
// Input:
// Output:
// Note:
//	frees data associated with node system
//
// ----------------------------------------------------------------------------
void SIDEKICK_FreeNodeSystem( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	hook->pNodeList = AI_DeleteNodeList( hook->pNodeList );
	
	//	by default, players should not have either of these...
	PATHLIST_KillPath(hook->pPathList);
	hook->pPathList = PATHLIST_Destroy(hook->pPathList);

}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_CheckPowerUps
// Description:
//	called each frame before the physics are run
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_CheckPowerUps( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	_ASSERTE( hook );

	int	recalc = FALSE;

	// exp_flags - set for each item when picked up and cleared when the item's expire message is sent
	// items - set when an item is picked up and cleared when the item becomes inactive
	// xx_time - set when an item is picked up to number of seconds till item expires

	////////////////////////////////////////
	// poison
	////////////////////////////////////////
	if ( hook->exp_flags & EXP_POISON && hook->poison_time <= -3.0 )
	{
		//	poison expiration sound and message
		gstate->StartEntitySound(self, CHAN_VOICE, gstate->SoundIndex("artifacts/poisonfade.wav"), 
                                 1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
		sidekick_deathtype = POISON;
		//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_POISON_EXPIRE]);
		hook->exp_flags &= ~EXP_POISON;
	}
	if ( hook->items & IT_POISON )
	{
		//	do damage
		if ( hook->poison_next_damage_time <= 0 )
		{
			gstate->Con_Printf("poisoning\n");
			com->Damage(self, self, self, zero_vector, zero_vector, hook->poison_damage, DAMAGE_POISON | DAMAGE_NO_BLOOD);
			hook->poison_next_damage_time = hook->poison_interval;
		}
		//	poison expire
		if (hook->poison_time < 0)
		{
			hook->items &= ~IT_POISON;
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
	if ( hook->envirosuit_time <= gstate->time + 3.0 && (hook->exp_flags & EXP_ENVIROSUIT) )
	{
		//	envirosuit expiration sound and message
		gstate->StartEntitySound(self, CHAN_VOICE, gstate->SoundIndex("artifacts/envirosuit/envirosuitfade.wav"), 
                                 1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
		//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_ENVIROSUIT_EXPIRE]);
		hook->exp_flags = hook->exp_flags - (hook->exp_flags & EXP_ENVIROSUIT);
	}
	if ( hook->items & IT_ENVIROSUIT )
	{
		//	envirosuit expires
		if (hook->envirosuit_time < gstate->time)
		{
			hook->items -= (hook->items & IT_ENVIROSUIT);
		}
	}

	////////////////////////////////////////
	// megashield
	////////////////////////////////////////
	if ( hook->invulnerability_time <= gstate->time + 3.0 && (hook->exp_flags & EXP_MEGASHIELD) )
	{
		//	megashield expiration sound and message
		//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_MEGASHIELD_EXPIRE]);
		gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("artifacts/megashield/megafade.wav"), 
                                 1.0f, hook->fMinAttenuation, hook->fMaxAttenuation);
		hook->exp_flags = hook->exp_flags - (hook->exp_flags & EXP_MEGASHIELD);
	}
	if ( hook->items & IT_MEGASHIELD )
	{
		//	megashield expires
		if (hook->invulnerability_time < gstate->time)
		{
			hook->items -= (hook->items & IT_MEGASHIELD);
		}
	}
	/////////////////////////////////////////////
	// stat boosters
	/////////////////////////////////////////////
	if ( hook->items & IT_POWERBOOST )
	{
		if (hook->power_boost_time <= gstate->time + 5.0 && hook->exp_flags & EXP_POWERBOOST)
		{
			//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_BOOST_POWER_EXPIRE]);
			hook->exp_flags -= (hook->exp_flags & (EXP_POWERBOOST));
		}

		if ( hook->power_boost_time <= gstate->time )
		{
			hook->items -= (hook->items & IT_POWERBOOST);
			hook->power_boost = 0; //hook->base_power;
			recalc = TRUE;
		}
	}
	if ( hook->items & IT_ATTACKBOOST )
	{
		if ( hook->attack_boost_time <= gstate->time + 5.0 && hook->exp_flags & EXP_ATTACKBOOST )
		{
			//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_BOOST_ATTACK_EXPIRE]);
			hook->exp_flags -= (hook->exp_flags & (EXP_ATTACKBOOST));
		}

		if ( hook->attack_boost_time <= gstate->time )
		{
			hook->items -= (hook->items & IT_ATTACKBOOST);
			hook->attack_boost = 0; //hook->base_attack;
			recalc = TRUE;
		}
	}
	if ( hook->items & IT_SPEEDBOOST )
	{
		if (hook->speed_boost_time <= gstate->time + 5.0 && hook->exp_flags & EXP_SPEEDBOOST)
		{
			//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_BOOST_SPEED_EXPIRE]);
			hook->exp_flags -= (hook->exp_flags & (EXP_SPEEDBOOST));
		}

		if ( hook->speed_boost_time <= gstate->time )
		{
			hook->items -= (hook->items & IT_SPEEDBOOST);
			hook->speed_boost = 0; //hook->base_speed;
			recalc = TRUE;
		}
	}
	if ( hook->items & IT_ACROBOOST )
	{
		if ( hook->acro_boost_time <= gstate->time + 5.0 && hook->exp_flags & EXP_ACROBOOST )
		{																						  
			//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_BOOST_ACRO_EXPIRE]);
			hook->exp_flags -= (hook->exp_flags & (EXP_ACROBOOST));
		}

		if ( hook->acro_boost_time <= gstate->time )
		{
			hook->items -= (hook->items & IT_ACROBOOST);
			hook->acro_boost = 0; //hook->base_acro;
			recalc = TRUE;
		}
	}
	if ( hook->items & IT_VITABOOST )
	{
		if (hook->vita_boost_time <= gstate->time + 5.0 && hook->exp_flags & EXP_VITABOOST)
		{
			//gstate->sprint(self, "%s.\n",tongue_world[T_POWERUPS_BOOST_VITA_EXPIRE]);
			hook->exp_flags -= (hook->exp_flags & (EXP_VITABOOST));
		}

		if ( hook->vita_boost_time <= gstate->time )
		{
			hook->items -= (hook->items & IT_VITABOOST);
			hook->vita_boost = 0; //hook->base_vita;
			recalc = TRUE;
		}
	}

	if ( recalc )
    {
        com->CalcBoosts( self );
    }
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_PreThink
// Description:
// Input:
// Output:
// Note:
//	called each frame before the physics are run
//
// ----------------------------------------------------------------------------
void SIDEKICK_PreThink( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	client_deathtype = NONE;

	//if(!(self->flags & FL_HCHILD))
	//	self->flags |= FL_HCHILD;
    
	if(hook)
		AI_ClearExecutedTasks( hook );

	/////////////////////////////////////////////////////////////////
	//	check to see if client on a warp path
	//	frame updates don't happen in a warp
	/////////////////////////////////////////////////////////////////

	if ( self->flags & FL_INWARP )
	{
		return;
	}

	//	animate the input entity's models
	com->FrameUpdate(self);

	//	fire player weapons that are based on frames
	/*if ( hook->fxFrameFunc )
	{
		hook->fxFrameFunc( self );
	}*/

	if ( !AI_IsAlive( self ) ) // 10.6 changed to 'NOT alive'
	{
		return;
	}

	hook->last_origin = self->s.origin;

	// determine if the player/bot has left the ground between nodes
	// used for determining when to link nodes back
	if ( self->groundEntity == NULL )
	{
		hook->bOffGround = TRUE;
	}

}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_NitroDeath
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
// Logic[5/10/99]: E3 Nitro Hack
void SIDEKICK_NitroDeath(userEntity_t *self)
{
    com->Damage(self, self, self, zero_vector, zero_vector, 32000.0f, DAMAGE_LAVA | DAMAGE_NO_BLOOD);
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_PostThink
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_PostThink( userEntity_t *self )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used


	if ( self->flags & FL_INWARP )
	{
		return;
	}

	if ( !AI_IsAlive( self ) ) // 10.6 changed to 'NOT alive'
	{
		return;
	}

	//SIDEKICK_CheckPowerUps( self );

  // Logic[5/10/99]: E3 NITRO HACK
  if ( self->s.renderfx & RF_IN_NITRO )
  {
    if ( self->think != SIDEKICK_NitroDeath )
    {
      self->think = SIDEKICK_NitroDeath;
      AI_SetNextThinkTime( self, 3.0f );
    }
  }
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_FindOwner
// Description:
// Input:
// Output:
// Note:
//				waits until a player logs on, then assigns that player as owner 
//              this sidekick
//
// ----------------------------------------------------------------------------
void SIDEKICK_FindOwner( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_SetOwner( self, NULL );

	int nGoalType = GOALTYPE_NONE;
    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
    if ( pCurrentGoal )
    {
        GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
    }

	userEntity_t *head = alist_FirstEntity( client_list );
	while ( head != NULL )
	{
		// NSS[12/5/99]:Making changes for Coop.. find the first Client
		//if ( AI_IsAlive( head ) && _stricmp( head->className, "player" ) == 0 )
		if ( AI_IsAlive( head ) && (head->flags & FL_CLIENT) )
		{
			float fDistance = VectorDistance( self->s.origin, head->s.origin );
			if ( (fDistance < hook->active_distance && AI_IsVisible(self, head)) ||
                 (AI_IsLineOfSight( self, head) && fDistance < 256.0f) )
			{
				AI_SetOwner( self, head );

                if ( !SIDEKICK_IsStaying( hook ) )
                {
                    if ( nGoalType == GOALTYPE_FOLLOW )
                    {		        
    			        AI_AddInitialTasksToGoal( self, pCurrentGoal, TRUE );
                    }
                    else
                    {
                	    AI_AddNewGoal( self, GOALTYPE_FOLLOW, hook->owner );
                    }
			    }
            }
		}

		head = alist_NextEntity( client_list );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_FindOwnerThink
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_FindOwnerThink( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	// NSS[2/10/00]:Make sure we have some form of a sequence first!
	if(!hook->cur_sequence)
	{
		// NSS[2/10/00]:We got no sequence thus force one down the hole!
		char szAnimation[16];
		AI_SelectAmbientAnimation( self, szAnimation );
		frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
		AI_ForceSequence(self, pSequence,FRAME_LOOP);
	}
	SIDEKICK_FindOwner( self );




	if( hook->owner == NULL )
	{
		self->think		= SIDEKICK_FindOwnerThink;
	}
	else
	{
   		SIDEKICK_InitDefaultWeapon( self );

		self->think		= SIDEKICK_Think;
	}

	AI_SetNextThinkTime( self, 0.1f );
}

// ----------------------------------------------------------------------------
// NSS[3/1/00]:
// Name:		SIDEKICK_CanPathToOrIsCloseToEnemy
// Description: This will determine all of the possible variables determining
// if the enemy is reachable, shootable, and visible.  
// Input:userEntity_t self, userEntity_t *enemy
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_CanPathToOrIsCloseToEnemy(userEntity_t *self, userEntity_t *enemy)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	int response	= FALSE;
	int path		= AI_FindPathToEntity(self,enemy,TRUE);
	float xyDistance= VectorXYDistance(self->s.origin,enemy->s.origin); 
	float zDistance = VectorZDistance(self->s.origin,enemy->s.origin); 
	float fDistance = SIDEKICK_DeterminePathLength(self, enemy);
	if(hook->pPathList && hook->pPathList->nPathLength && fDistance < 1024.0f && fDistance != -1)
	{
		response = TRUE;
	}
	else if(hook->pPathList && hook->pPathList->nPathLength && fDistance != -1 && zDistance < 64.0f && xyDistance < 256.0f || zDistance < 64.0f && xyDistance < 256.0f && AI_IsVisible(self,enemy))
	{
		response = TRUE;
	}
	else if((enemy->movetype == MOVETYPE_FLY || enemy->movetype == MOVETYPE_HOVER || enemy->movetype == MOVETYPE_SWIM || enemy->movetype == MOVETYPE_NONE) && zDistance < 1024.0f && AI_IsVisible(self,enemy) && SIDEKICK_IsClearShot(self,enemy,self->s.origin,enemy->s.origin))
	{
		response = TRUE;
	}
	return response;
}

// ----------------------------------------------------------------------------
// NSS[3/1/00]:
// Name:		SIDEKICK_CanPathToOrIsCloseToEnemy
// Description: This will determine all of the possible variables determining
// if the enemy is reachable, shootable, and visible.  
// Input:userEntity_t self, userEntity_t *enemy
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_CanPathToOrIsCloseToOwner(userEntity_t *self, userEntity_t *owner)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	int response	= FALSE;
	int path		= AI_FindPathToEntity(self,owner,TRUE);
	float xyDistance= VectorXYDistance(self->s.origin,owner->s.origin); 
	float zDistance = VectorZDistance(self->s.origin,owner->s.origin); 
	float fDistance = SIDEKICK_DeterminePathLength(self,owner);
	if(hook->pPathList && hook->pPathList->nPathLength > 2 && fDistance != -1)
	{
		response = TRUE;
	}
	else if(hook->pPathList && hook->pPathList->nPathLength && fDistance != -1 && zDistance < 64.0f && xyDistance < 256.0f || zDistance < 64.0f && xyDistance < 256.0f && AI_IsVisible(self,owner))
	{
		response = TRUE;
	}
	return response;
}


// ----------------------------------------------------------------------------
// NSS[5/19/00]:
// Name:  SIDEKICK_IsOkToMoveStraight
// Description:This is a hack so that we do not change the fundamental ai_isoktomovestraight
// function but we can make changes here for various minor functions which need 'special' attention.
// Input:userEntity_t *self, CVector &destPoint, float fXYDistance, float fZDistance
// Output:TRUE/FALSE
// ----------------------------------------------------------------------------
int SIDEKICK_IsOkToMoveStraight( userEntity_t *self, CVector &destPoint, float fXYDistance, float fZDistance )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	if ( AI_IsFlyingUnit( self ) || AI_IsInWater( self ) || hook->ai_flags & AI_DROWNING || self->flags & FL_CINEMATIC)
    {
        if ( AI_IsLineOfSight( self, destPoint ) )
        {
            return TRUE;
        }
    }
    else
    {
		CVector Mins = self->s.mins;
		CVector QuarterDest,Dir;
		float fSpeed = self->velocity.Length() * 0.125;
		Dir = destPoint - self->s.origin;
		Dir.Normalize();
		QuarterDest = self->s.origin + (Dir * fSpeed);
		Mins.z += 8.0f;
		tr = gstate->TraceBox_q2( self->s.origin,Mins,self->s.maxs,QuarterDest, self,MASK_SOLID);
	    if ( hook->ai_flags & AI_ALWAYS_USENODES )
	    {
		    if ( (tr.fraction >= 1.0f && fZDistance < MAX_JUMP_HEIGHT) ||
			     (tr.fraction >= 0.8f && ((1.2f-tr.fraction) * (fXYDistance+fZDistance)) < 32.0f) )
		    {
			    if ( AI_IsGroundBelowBetween( self, self->s.origin, destPoint ) )
			    {
				    return TRUE;
			    }
		    }
	    }
	    else
	    {
		    if ( (tr.fraction >= 1.0f && fZDistance < MAX_JUMP_HEIGHT) || 
			     (tr.fraction >= 0.8f && ((1.2f-tr.fraction) * (fXYDistance+fZDistance)) < 32.0f) )
		    {
			    if ( AI_IsGroundBelowBetween( self, self->s.origin, destPoint ) )
			    {
				    return TRUE;
			    }
		    }
	    }
    }

	return FALSE;
}


// ----------------------------------------------------------------------------
// <nss>
// Name:		Markers_Think
// Description:Think function for Markers
// Input:userEntity_t *self
// Output:<nothing>
// Note:  
// Created:<nss>
// ----------------------------------------------------------------------------
void SIDEKICK_Markers_Think (userEntity_t *self)
{
	if(self->delay < gstate->time)
		self->remove(self);
	self->nextthink = gstate->time + 0.01f;
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		AI_Drop_Markers
// Description:Drops a Marker at the CVector Target origin and leaves it for
// 20 seconds
// Input:CVector Target
// Output:<nothing>
// Note:  
// Created:<nss>
// ----------------------------------------------------------------------------
void SIDEKICK_Drop_Markers (CVector &Target, float delay)
{
	if(gstate->GetCvar("ai_debug_info") != 0.0f)
	{
		userEntity_t *Flare;
		
		Flare = gstate->SpawnEntity ();

		Flare->owner = Flare;
		Flare->solid = SOLID_NOT;
		Flare->movetype = MOVETYPE_NONE;
		Flare->className = "drop_sprite";

		Flare->s.renderfx |= (SPR_ALPHACHANNEL);

		Flare->s.modelindex = gstate->ModelIndex ("models/global/e_flare4x.sp2");
		
		Flare->s.render_scale.Set( .50, .50, .50);
		Flare->s.alpha = 0.50f;
		Flare->s.color.Set(1.0f,0.50f,0.50f);
		Flare->delay = gstate->time + delay;
		Flare->think = SIDEKICK_Markers_Think;

		gstate->SetSize (Flare, -2, -2, -4, 2, 2, 4);
		gstate->SetOrigin2 (Flare, Target);
		gstate->LinkEntity (Flare);

		Flare->s.frame = 0;
		Flare->nextthink = gstate->time + 0.01f;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_ParseCommand
// Description:
// Input:
// Output:
// Note:
//	searches through the passed string for meaningful commands
//	if a valid command is found, it's bit is set in commandFlag
//	commandFlag is returned
//
//	this does a lot of string compares, so it should not be used except
//	when the player issues a command from the console or a key binding
//
// ----------------------------------------------------------------------------
//#define SIDEKICK_COMMAND_YES            0x00000001
//#define SIDEKICK_COMMAND_NO             0x00000002
//#define SIDEKICK_COMMAND_STAY           0x00000004
//#define SIDEKICK_COMMAND_FOLLOW         0x00000008
//#define SIDEKICK_COMMAND_ATTACK         0x00000010
//#define SIDEKICK_COMMAND_NOATTACK       0x00000020
//#define SIDEKICK_COMMAND_COMEHERE       0x00000040
//#define SIDEKICK_COMMAND_PICKUP         0x00000080
int FindClientInPVS(userEntity_t *self);
// NSS[2/24/00]:Hack hack hack... don't want to use any more values in the userEntity or hook structure... got WAY too many as it is.
void SIDEKICK_ParseCommand( userEntity_t *self, char *str )
{
    playerHook_t *hook = AI_GetPlayerHook( self );

	if (!hook || !hook->owner || hook->bAttackPlayer == TRUE )
		return;

	unsigned long nCommandType = 0;

	
	// NSS[2/26/00]:Prevents too many commands coming at once.
	if(AI_IsSuperfly(hook) && gstate->time > Superflywait)
	{
		Superflywait = gstate->time + 1.10f;
	}
	else if(AI_IsMikiko(hook) && gstate->time > MikikoWait)
	{
		MikikoWait = gstate->time + 1.10f;
	}
	else
	{
		return;
	}

	userEntity_t *pOwner = hook->owner;
	if(!AI_IsAlive( pOwner ) || self->groundEntity == NULL)
		return;

	  
	// NSS[2/5/00]:If sidekicks cannot see you and we are not in developer mode then don't respond or do a damned thing!
	if(!FindClientInPVS(self) && !gstate->GetCvar ("developer"))
	{	
		return;
	}
	
	if ( _stricmp( str, aSidekickCommandString[SIDEKICK_COMMAND_YES][0] ) == 0 )
    {
        nCommandType = SIDEKICK_COMMAND_YES;
    }
    else
    if ( _stricmp( str, aSidekickCommandString[SIDEKICK_COMMAND_NO][0] ) == 0 )
    {
        nCommandType = SIDEKICK_COMMAND_NO;
    }
    else
    if ( _stricmp( str, aSidekickCommandString[SIDEKICK_COMMAND_STAY][0] ) == 0 )
    {
        nCommandType = SIDEKICK_COMMAND_STAY;
    }
    else
    if ( _stricmp( str, aSidekickCommandString[SIDEKICK_COMMAND_FOLLOW][0] ) == 0 )
    {
        nCommandType = SIDEKICK_COMMAND_FOLLOW;
    }
    else
    if ( _stricmp( str, aSidekickCommandString[SIDEKICK_COMMAND_ATTACK][0] ) == 0 )
    {
        nCommandType = SIDEKICK_COMMAND_ATTACK;
    }
    else
    if ( _stricmp( str, aSidekickCommandString[SIDEKICK_COMMAND_NOATTACK][0] ) == 0 )
    {
        nCommandType = SIDEKICK_COMMAND_NOATTACK;
    }
    else
    if ( _stricmp( str, aSidekickCommandString[SIDEKICK_COMMAND_COMEHERE][0] ) == 0 )
    {
        nCommandType = SIDEKICK_COMMAND_COMEHERE;
    }
    else
    if ( _stricmp( str, aSidekickCommandString[SIDEKICK_COMMAND_PICKUP][0] ) == 0 )
    {
        nCommandType = SIDEKICK_COMMAND_PICKUP;
    }

	
    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	  GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
    GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );

	if ( nCommandType == SIDEKICK_COMMAND_YES )
    {
        SIDEKICK_PermissionGiven( self );  
    }
    else
    if ( nCommandType == SIDEKICK_COMMAND_NO )
    {
        SIDEKICK_PermissionDenied( self );
    }
    else
    if ( nCommandType == SIDEKICK_COMMAND_STAY )
	{
		hook->ai_flags &= ~AI_FOLLOW;
		hook->ai_flags |= AI_STAY;

		if ( nGoalType == GOALTYPE_STAY )
		{
		  AI_AddInitialTasksToGoal( self, pCurrentGoal, TRUE );
		}
		else
		{
		  //	get rid of all goals
		  GOALSTACK_ClearAllGoals( pGoalStack );
		  AI_AddNewGoal( self, GOALTYPE_STAY );
		}
		SIDEKICK_CommandAcknowledge(self,SIDEKICK_COMMAND_STAY); // respond to given command
	}
	else 
	if ( nCommandType == SIDEKICK_COMMAND_FOLLOW )
	{
		hook->ai_flags &= ~AI_STAY;
		hook->ai_flags |= AI_FOLLOW;

		if ( nGoalType == GOALTYPE_FOLLOW )
		{
			AI_AddInitialTasksToGoal( self, pCurrentGoal, TRUE );            
		}
		else
		{
			//	get rid of all goals
			GOALSTACK_ClearAllGoals( pGoalStack );
			AI_AddNewGoal( self, GOALTYPE_FOLLOW, hook->owner );
		}

	}
	else 
	if ( nCommandType == SIDEKICK_COMMAND_ATTACK )
	{
		hook->ai_flags &= ~AI_NOATTACK;
		hook->ai_flags |= AI_ATTACK;
		hook->ai_flags |=AI_SIDEKICKKILL;
		
	
		if(hook->bCarryingMikiko)
		{
			SideKick_TalkAmbient(self, NO,1);
			SIDEKICK_SendMessage(self, sCom_Speak,NO,gstate->time+2.0f,0.0f,self,COM_Read);
			return;
		}
		
		if(SIDEKICK_AttackOwnerFacingEnemy(self) && SIDEKICK_CheckMonsterType(self,self->enemy) )
		{
			// NSS[2/29/00]:If we can kill and is ok to fight
			if(SIDEKICK_IsOkToFight( self ) && SIDEKICK_CanPathToOrIsCloseToEnemy(self,self->enemy))
			{
				hook->ai_flags |=AI_SIDEKICKKILL;
				if (!SIDEKICK_HasTaskInQue(self,TASKTYPE_SIDEKICK_ATTACK) )
				{
					AI_AddNewTaskAtFront(self,TASKTYPE_SIDEKICK_ATTACK);
				}
				return;
			}
			else
			{
				SideKick_TalkAmbient(self, NO,1);
				hook->ai_flags &= ~AI_SIDEKICKKILL;
				self->enemy = NULL;
				SIDEKICK_SendMessage(self, sCom_Speak,NO,gstate->time+1.0f,0.0f,self,COM_Read);
				return;
			}
		}
		else
		{
			hook->ai_flags &= ~AI_SIDEKICKKILL;
		}

		SIDEKICK_CommandAcknowledge(self,SIDEKICK_COMMAND_ATTACK); // respond to given command
	}
    else
    if ( nCommandType == SIDEKICK_COMMAND_NOATTACK )
    {
		hook->ai_flags &= ~AI_ATTACK;
        hook->ai_flags |= AI_NOATTACK;

        SIDEKICK_CommandAcknowledge(self,SIDEKICK_COMMAND_NOATTACK); // respond to given command

		// NSS[12/7/99]:Adding the backoff stuff
		if(hook->owner)
		{
			// try to evade from fighting
			SIDEKICK_HandleMoveAway(self,hook->owner);
			hook->ai_flags &= ~AI_STAY;
		}
    }
    else
    if ( nCommandType == SIDEKICK_COMMAND_COMEHERE )
    {
        
		hook->ai_flags &= ~AI_STAY;
        hook->ai_flags |= AI_FOLLOW;

        hook->ai_flags |= AI_COMEHERE;
        
        // find a point in directly front of the owner
        userEntity_t *pOwner = hook->owner;
		if(pOwner)
		{
			if(!AI_IsAlive(pOwner) || (pOwner->deadflag != DEAD_NO))
				return;
	      
		} 
        else// NSS[12/7/99]:No owner get out.
		{
			return;
		}
		CVector vector;
        pOwner->s.angles.AngleToForwardVector( vector );

        CVector position;
        VectorMA( pOwner->s.origin, vector, 64.0f, position );
        
        CVector groundPoint;
		if(SIDEKICK_FindClearSpotFrom( self, pOwner->s.origin , groundPoint ))
        {
			if(!AI_FindPathToPoint(self,groundPoint))
			{
				float fZDistance	= VectorZDistance(self->s.origin,groundPoint);
				float fXYDistance	= VectorXYDistance(self->s.origin,groundPoint);
				if(!AI_IsOkToMoveStraight( self, groundPoint, fXYDistance, fZDistance ))
				{
   					if(!Check_Que(self,CANTREACH,1.5f))
					{
						SideKick_TalkAmbient(self, CANTREACH,1);
						SIDEKICK_SendMessage(self, sCom_Speak,NO,gstate->time+1.5f,0.0f,self,COM_Read);
					}				
					return;
				}
			}
#ifdef _DEBUG
			AI_Drop_Markers(groundPoint,2.0f);
#endif
			if(nGoalType == GOALTYPE_STAY)
			{
				if(hook->owner)
				{
					GOALSTACK_ClearAllGoals( pGoalStack );
					AI_AddNewGoal( self, GOALTYPE_FOLLOW, hook->owner );
				    GOALSTACK_PTR pGoalStackT = AI_GetCurrentGoalStack(hook);// NSS[3/10/00]:This is a double check
					GOAL_PTR pCurrentGoalT = GOALSTACK_GetCurrentGoal(pGoalStackT);
					if(pCurrentGoalT->nGoalType == GOALTYPE_FOLLOW)
						AI_AddNewTaskAtFront( self, TASKTYPE_SIDEKICK_COMEHERE, groundPoint );
				}
			}
			else
			{
				GOAL_ClearTasks( pCurrentGoal );
				AI_AddNewTaskAtFront( self, TASKTYPE_SIDEKICK_COMEHERE, groundPoint );
			}
        }
        else
        {
   			if(!Check_Que(self,CANTREACH,3.0f))
			{
				SideKick_TalkAmbient(self, CANTREACH,1);
				SIDEKICK_SendMessage(self, sCom_Speak,NO,gstate->time+3.0f,0.0f,self,COM_Read);
				return;
			}			
        }

  		// NSS[2/9/00]:Checking health and soon to be ammo if low health or ammo say so and exit out.(move to seperate function)
		if(self->health < hook->base_health*0.20)
		{
			if(!Check_Que(self,LOW_HEALTH,70.0f))
			{
				SideKick_TalkAmbient(self, HEALTH_RESPONSE5,1);
				SIDEKICK_SendMessage(self, sCom_Speak,LOW_HEALTH,gstate->time+15.0f,0.0f,self,COM_Read);
				return;
			}
		}
		else if(AI_IsMikiko(hook))// NSS[2/29/00]:Otherwise check for no ammo...
		{
			if(!(saveFlags & MK_GOOD_AMMO) && !(saveFlags & MK_ROCKIN_AMMO))
			{
				if(!Check_Que(self,NO_AMMO,70.0f))
				{
					SideKick_TalkAmbient(self, NO_AMMO,1);
					SIDEKICK_SendMessage(self, sCom_Speak,NO_AMMO,gstate->time+15.0f,0.0f,self,COM_Read);
					return;
				}			
			}
		}
		else if(AI_IsSuperfly(hook) && !hook->bCarryingMikiko)
		{
			if(!(saveFlags & SF_GOOD_AMMO) && !(saveFlags & SF_ROCKIN_AMMO))
			{
				if(!Check_Que(self,NO_AMMO,70.0f))
				{
					SideKick_TalkAmbient(self, NO_AMMO,1);
					SIDEKICK_SendMessage(self, sCom_Speak,NO_AMMO,gstate->time+15.0f,0.0f,self,COM_Read);
					return;
				}		
			}
		}
		SIDEKICK_CommandAcknowledge(self,SIDEKICK_COMMAND_COMEHERE); // respond to given command
	}
    else
    if ( nCommandType == SIDEKICK_COMMAND_PICKUP )
    {

		userEntity_t *pItem = ITEM_GetTargetedItem( self );
        if ( pItem )
        {
            int ret = SIDEKICK_CanPickupItem( self, pItem, TRUE );
			if ( ret == TRUE)
            {
				/*TASK_PTR pTask = GOAL_GetCurrentTask( pCurrentGoal );
                AIDATA_PTR pAIData = TASK_GetData( pTask );
                TASKTYPE nTaskType = TASK_GetType( pTask );
                if ( !(nTaskType == TASKTYPE_SIDEKICK_PICKUPITEM && pAIData->pEntity == pItem) )
                {
                    AI_AddNewTaskAtFront( self, TASKTYPE_SIDEKICK_PICKUPITEM, pItem );
                }*/
				float fZDistance	= VectorZDistance(self->s.origin,pItem->s.origin);
				float fXYDistance	= VectorXYDistance(self->s.origin,pItem->s.origin);

				if(!AI_FindPathToPoint(self,pItem->s.origin) || (fXYDistance < 64.0f && fZDistance <= 32.0f))
				{
					if(!SIDEKICK_IsOkToMoveStraight( self, pItem->s.origin, fXYDistance, fZDistance ))
					{
   						if(!Check_Que(self,CANTREACH,1.5f))
						{
							SideKick_TalkAmbient(self, CANTREACH,1);
							SIDEKICK_SendMessage(self, sCom_Speak,NO,gstate->time+1.5f,0.0f,self,COM_Read);
						}				
						return;
					}
				}
                if(!AI_HasGoalInQueue(self,GOALTYPE_PICKUPITEM))
				{
					if(strstr(pItem->className,"misc_"))
					{
						AI_AddNewGoal(self,GOALTYPE_USEHOSPORTAL,pItem);
					}
					else
					{
						AI_AddNewGoal(self,GOALTYPE_PICKUPITEM,pItem);
					}

                }
				SIDEKICK_CommandAcknowledge(self,SIDEKICK_COMMAND_PICKUP); // respond to given command
            }
            else if ( ret == FALSE)
            {
				SIDEKICK_CommandAcknowledge(self, SIDEKICK_COMMAND_PICKUP_NO); // respond to given command
            }
			else
			{
				if(!Check_Que(self,CANTREACH,1.5f))
				{
					SideKick_TalkAmbient(self, CANTREACH,1);
					SIDEKICK_SendMessage(self, sCom_Speak,CANTREACH,gstate->time+1.5f,0.0f,self,COM_Read);
					return;
				}			
			}
        }
		else
		{
			if(!Check_Que(self,NO,1.5f))
			{
				SideKick_TalkAmbient(self, NO,1);
				SIDEKICK_SendMessage(self, sCom_Speak,CANTREACH,gstate->time+1.5f,0.0f,self,COM_Read);
				return;
			}			
		}
    }
}




void SIDEKICK_CommandAcknowledge(userEntity_t *self, int command_type, qboolean PVS_required)
// play a sound in response to a command given (which will also show subtitle text)
//NSS[12/2/99]:
// file name format: [initial]_cmd_[command abbrev.]_[#].mp3
//
{
	char buf[256];
	char filepath[256];
	playerHook_t *hook = AI_GetPlayerHook( self );

	if( hook == NULL )
	{
		return;
	}
	if( hook->owner == NULL )
	{
		SIDEKICK_FindOwner( self );
	}
	if( hook->owner == NULL )
	{
		return;
	}

	if(Check_Que(self,NO_TALK,1.0f))
	{
		return;
	}
	if (!command_type) 
		return;

	if (PVS_required)                                            // owner/sidekick must be in PVS?
		if (!gstate->inPVS(self->s.origin,hook->owner->s.origin))  
			return;                                                  // not so, so don't speak


	if ( AI_IsMikiko(hook) )
		strcpy(filepath,"sounds/voices/mikiko/cmd_mi_");
	else
	if ( AI_IsSuperfly(hook) )
		strcpy(filepath,"sounds/voices/superfly/cmd_su_");

	//NSS[12/2/99]:Nice reference
	//#define SIDEKICK_COMMAND_YES            1
	//#define SIDEKICK_COMMAND_NO             2
	//#define SIDEKICK_COMMAND_STAY           3
	//#define SIDEKICK_COMMAND_FOLLOW         4
	//#define SIDEKICK_COMMAND_ATTACK         5
	//#define SIDEKICK_COMMAND_NOATTACK       6
	//#define SIDEKICK_COMMAND_COMEHERE       7
	//#define SIDEKICK_COMMAND_PICKUP         8
	//#define SIDEKICK_COMMAND_PICKUP_BAD     9
	//#define SIDEKICK_COMMAND_PICKUP_NO	  10
	//#define SIDEKICK_COMMAND_COMMAND		  11
	//#define SIDEKICK_COMMAND_LAST			  12
	
	//NSS[12/2/99]:Setup the commands to play
	switch(command_type)
	{
		case SIDEKICK_COMMAND_COMEHERE:
		{
			Com_sprintf(buf,sizeof(buf),"%sco_0%d.mp3",filepath,(int)(rnd()*3)+1);
			break;
		}
		case SIDEKICK_COMMAND_STAY:
		{
			Com_sprintf(buf,sizeof(buf),"%sst_0%d.mp3",filepath,(int)(rnd()*3)+1);
			break;			
		}
		case SIDEKICK_COMMAND_ATTACK:
		{
			Com_sprintf(buf,sizeof(buf),"%sat_0%d.mp3",filepath,(int)(rnd()*3)+1);
			break;						
		}
		case SIDEKICK_COMMAND_NOATTACK:
		{
			Com_sprintf(buf,sizeof(buf),"%sba_0%d.mp3",filepath,(int)(rnd()*3)+1);
			break;								
		}
		case SIDEKICK_COMMAND_PICKUP:
		{
			Com_sprintf(buf,sizeof(buf),"%spu_0%d.mp3",filepath,(int)(rnd()*3)+1);
			break;								
		}
		case SIDEKICK_COMMAND_PICKUP_NO:
		{
			Com_sprintf(buf,sizeof(buf),"%spu_0%d_n1.mp3",filepath,(int)(rnd()*3)+1);
			break;								
		}
		case SIDEKICK_COMMAND_YES:
		{
			Com_sprintf(buf,sizeof(buf),"%sye_0%d.mp3",filepath,(int)(rnd()*3)+1);
			break;								
		}
		case SIDEKICK_COMMAND_NO:
		{

			Com_sprintf(buf,sizeof(buf),"%sno_0%d.mp3",filepath,(int)(rnd()*2)+2);
			break;								
		}
		
		default://NSS[12/2/99]:Got a command not in here?  Let's not play it.
		{
			buf[0] = NULL;
			break;
		}
	}
	//NSS[12/2/99]:Play it!
	if(buf[0] != NULL)
		PlaySidekickMP3(self,buf);
		//gstate->StartMP3(buf, 2, 0.65, 1 );

}


// ----------------------------------------------------------------------------
// NSS[1/31/00]:
// Name:		SIDEKICK_Die
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_Die( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	userEntity_t *client_entity = &gstate->g_edicts[1];	
	ai_frame_sounds( self );

	frameData_t *pCurrentSequence = hook->cur_sequence;
	

	if ( pCurrentSequence && !strstr( pCurrentSequence->animation_name, "die" ) )
	{
		AI_ForceSequence(self, "diea");
		self->deadflag = DEAD_DYING;
	}
	else if (! pCurrentSequence)
	{
		self->deadflag  = DEAD_DEAD;
		self->die		= NULL;
		self->prethink	= NULL;
		self->postthink = NULL;
		self->think		= NULL;

		if ( hook->pGoals )
		{
			GOALSTACK_Delete( hook->pGoals );
			hook->pGoals = NULL;
		}
		if ( hook->pScriptGoals )
		{
			GOALSTACK_Delete( hook->pScriptGoals );
			hook->pScriptGoals = NULL;
		}

		if (self->waterlevel > 2)
		{
			int sound_index = rand() % 2;
			sound_index += gstate->SoundIndex( "hiro/waterland4.wav" );

			gstate->StartEntitySound( self, CHAN_AUTO, sound_index, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );
		}

		UNIQUEID_Remove( hook->szScriptName );
		free( hook->szScriptName );
		hook->szScriptName = NULL;

		alist_remove( self );

		gstate->LinkEntity( self );

		AI_Dprintf( "Finished TASKTYPE_SIDEKICK_DIE.\n" );

		if( ( client_entity->inuse != 0 ) && ( client_entity->deadflag == DEAD_NO ) )
		{
			AlertClientOfSidekickDeath(false); // game over  (false == not gibbed)
		}
	}

	if ( self->deadflag == DEAD_DYING )
	{
		
		if ( !strstr( pCurrentSequence->animation_name, "die" ) ||
			(self->s.frameInfo.frameState & FRSTATE_LAST ||
			self->s.frameInfo.frameState & FRSTATE_STOPPED) )
		{
			self->deadflag = DEAD_DEAD;
			self->die		= NULL;
			self->prethink	= NULL;
			self->postthink = NULL;
			self->think		= NULL;

			if ( hook->pGoals )
			{
				GOALSTACK_Delete( hook->pGoals );
				hook->pGoals = NULL;
			}
			if ( hook->pScriptGoals )
			{
				GOALSTACK_Delete( hook->pScriptGoals );
				hook->pScriptGoals = NULL;
			}

			if (self->waterlevel > 2)
			{
				int sound_index = rand() % 2;
				sound_index += gstate->SoundIndex( "hiro/waterland4.wav" );

				gstate->StartEntitySound( self, CHAN_AUTO, sound_index, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );
			}

			UNIQUEID_Remove( hook->szScriptName );
			free( hook->szScriptName );
			hook->szScriptName = NULL;

			alist_remove( self );

			gstate->LinkEntity( self );

			AI_Dprintf( "Finished TASKTYPE_SIDEKICK_DIE.\n" );

			if( ( client_entity->inuse != 0 ) && ( client_entity->deadflag == DEAD_NO ) )
			{
				AlertClientOfSidekickDeath(false); // game over  (false == not gibbed)
			}
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_StartDie
// Description:
//			called from com->Damage when a player is killed
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartDie( userEntity_t *self, userEntity_t *inflictor, 
		  				       userEntity_t *attacker, int damage, CVector &point )
{
    userEntity_t *client_entity = &gstate->g_edicts[1];	
    playerHook_t *phook = AI_GetPlayerHook( client_entity );

    playerHook_t *hook = AI_GetPlayerHook( self );
    userEntity_t *pCameraTarget = NULL;

	int Chan = 0;
	
	if(hook && AI_IsSidekick(hook))
	{
		Chan = 4;
		
	}
	else
	{
		Chan = 3;
	}

	gstate->StopMP3(Chan);

	if(hook && hook->owner)
	{
		char deathmsg[80];
		Com_sprintf(deathmsg, sizeof(deathmsg),"%s%s",com->ProperNameOf(self), tongue_deathmsg_self[NONE]);
		gstate->centerprint(hook->owner,2.0,deathmsg);
	}

	if ( self->deadflag == DEAD_NO )
	{
		SIDEKICK_DropInventory( self );
	}
	
	// NSS[1/31/00]:Episode3 water death
	if((self->watertype & CONTENTS_WATER) && attacker == self && inflictor == self && (GetCurrentEpisode() == 3))
	{
		self->flags |= FL_FREEZE;
		self->s.color.Set(0.0,0.0,0.65);
		self->s.alpha = 0.75;
		self->s.renderfx |= SPR_ALPHACHANNEL;
	}

    
	// self->solid			= SOLID_NOT;    //	do not clip any longer!
    self->solid         = SOLID_BBOX;
    self->groundEntity	= NULL;
    self->movetype		= MOVETYPE_BOUNCE;
    self->svflags		|= SVF_DEADMONSTER;	//&&& AMW 7/17/98 - as per quake2
    self->armor_val		= 0;
    self->armor_abs		= 0;

	// lock the angles of the entity so he can't rotate or spin
	self->s.angles.x	= 0.0f;
	self->s.angles.z	= 0.0f;

	
	if( ( client_entity->inuse != 0 ) && ( client_entity->deadflag == DEAD_NO ) )
	{
		if ( ( phook != NULL ) && ( phook->camera ) )  // if previous camera in use, stop it
		{
			camera_stop(client_entity, false);
		}

		camera_start (client_entity, false);	// prepare camera
	}

	if ( self->deadflag == DEAD_NO )
	{
		if ( self->waterlevel > 2 )
        {
		    SIDEKICK_PlaySound(self, "waterdeath.wav");
		}
		else 
		if ( self->health < -40 )
        {
		    SIDEKICK_PlaySound(self, "udeath.wav");
		}
		else
		{
			switch (rand() % 4)
            {
			    case 0 : SIDEKICK_PlaySound(self, "death1.wav"); break;
				case 1 : SIDEKICK_PlaySound(self, "death2.wav"); break;
				case 2 : SIDEKICK_PlaySound(self, "death3.wav"); break;
				case 3 : SIDEKICK_PlaySound(self, "death4.wav"); break;
			}
		}

		if ( !( self->flags & FL_FREEZE ) && check_for_gib(self, damage) && sv_violence == 0)
		{
			//	make sure we're not drawing the player model
			self->s.renderfx |= RF_NODRAW;

			// dismember this dead bastard
			//NSS[11/23/99]:Violence stuff (0 = violence anything else = none)
			//if(sv_violence == 0)
			//{
			pCameraTarget = gib_client_die(self, inflictor);
			//}

            //If we are not dead yet
			if( ( client_entity->inuse != 0 ) && ( client_entity->deadflag == DEAD_NO ) )
			{
				if(pCameraTarget)
				{
					camera_set_target (phook->camera, pCameraTarget);
					camera_mode (client_entity, CAMERAMODE_GIBCAM, false);
				}
			}

            self->die		= NULL;
            self->prethink	= NULL;
            self->postthink = NULL;
            self->think		= NULL;
            self->deadflag  = DEAD_DEAD;

			if ( hook->pGoals )
			{
				GOALSTACK_Delete( hook->pGoals );
				hook->pGoals = NULL;
			}
			if ( hook->pScriptGoals )
			{
				GOALSTACK_Delete( hook->pScriptGoals );
				hook->pScriptGoals = NULL;
			}
			
			UNIQUEID_Remove( hook->szScriptName );
			free( hook->szScriptName );
			hook->szScriptName = NULL;

			alist_remove( self );
			self->remove(self);          // remove if gibbed
			gstate->LinkEntity( self );

//			if(client_entity->deadflag == DEAD_NO)
//			{
//				AlertClientOfSidekickDeath(false); // game over (true == gibbed)
//			}

			return;
		}
		else
		{
            // set up frozen state
			if ( self->flags & FL_FREEZE )
			{ 
				AI_StopCurrentSequence( self ); // stop animation dead

				self->solid    = SOLID_BBOX;    // make the player dead but solid as a rock
				self->movetype = MOVETYPE_BOUNCE; // get pushy since we're frozen
				self->deadflag = DEAD_FROZEN;
			}
			else
			{
				AI_ForceSequence(self, "diea");

		        // die task need to be inserted since the die task gets triggered from outside 
		        // the AI framework
		        GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
		        GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
		        if ( !pCurrentGoal )
		        {
			        AI_AddNewGoal( self, GOALTYPE_IDLE );
		        }
		        AI_AddNewTaskAtFront( self, TASKTYPE_SIDEKICK_DIE );
			}

			pCameraTarget = self; // set the camera to this entity
	
			self->deadflag = DEAD_DYING;
		}

		//if we are not dead yet.
		if( ( client_entity->inuse != 0 ) && ( client_entity->deadflag == DEAD_NO ) )
		{
			camera_set_target (phook->camera, pCameraTarget);
			camera_mode (client_entity, CAMERAMODE_GIBCAM, false);
		}
	}
	else
	if (self->deadflag == DEAD_FROZEN)
	{
		self->solid    = SOLID_BBOX;       // frozen solid
		self->movetype = MOVETYPE_BOUNCE; // push technology

        pCameraTarget = self;

        //NSS[11/23/99]:IF we are not dead yet.
		if( ( client_entity->inuse != 0 ) && ( client_entity->deadflag == DEAD_NO ) )
		{
			camera_set_target (phook->camera, pCameraTarget);
			camera_mode (client_entity, CAMERAMODE_GIBCAM, false);
		}

        self->die		= NULL;
        self->prethink	= NULL;
        self->postthink = NULL;
        self->think		= NULL;
        self->deadflag  = DEAD_DEAD;

		if ( hook->pGoals )
		{
			GOALSTACK_Delete( hook->pGoals );
			hook->pGoals = NULL;
		}
		if ( hook->pScriptGoals )
		{
			GOALSTACK_Delete( hook->pScriptGoals );
			hook->pScriptGoals = NULL;
		}
		
		UNIQUEID_Remove( hook->szScriptName );
		free( hook->szScriptName );
		hook->szScriptName = NULL;

		alist_remove( self );
		gstate->LinkEntity( self );

	}
	//NSS[11/30/99]:There can only be one!  :P
	if( ( client_entity->inuse != 0 ) && ( client_entity->deadflag == DEAD_NO ) )
	{
		AlertClientOfSidekickDeath(false); // game over  (false == not gibbed)
	}
	
	AI_SetNextThinkTime ( self, 0.1f  );
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 5.0f  );
}








// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_Pain
// Description:
//			initializes the client data structures
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_Pain( userEntity_t *self, userEntity_t *pAttacker, float kick, int damage )
{
	_ASSERTE( self );

	char soundname[MAX_QPATH];

	playerHook_t *hook = AI_GetPlayerHook( self );
	userEntity_t *pCameraTarget = NULL;
	userEntity_t *pOtherSidekick = SIDEKICK_GetOtherSidekick( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );


	// NSS[12/5/99]:Check to see if Hiro is the one doing the smacking... make sure we will still be alive to speak.
	if(hook && ((self->health-damage) > 0))
	{
		// NSS[12/6/99]:First thing we do is clear any command to stay... if something is busting us down let's kick ass!
		if(pAttacker == hook->owner)
		{
			if(rnd() < 0.50 || damage >10.0f)
			{
				if(!Check_Que(self,HITBY_HIRO,3.0f))
				{
					SideKick_TalkAmbient(self, HITBY_HIRO,1);
					SIDEKICK_SendMessage(self, sCom_Speak,HITBY_HIRO,gstate->time+3,0.0f,self,COM_Read);
				}
			}
		}
		else if(pAttacker == pOtherSidekick)
		{
			if(rnd() < 0.50 || damage > 10.0f)
			{
				if(!Check_Que(self,HITBY_SIDEKICK,3.0f))
				{
					SideKick_TalkAmbient(self, HITBY_SIDEKICK,1);
					SIDEKICK_SendMessage(self, sCom_Speak,HITBY_SIDEKICK,gstate->time+3,0.0f,self,COM_Read);
				}
				
			}
		}
		else// NSS[12/7/99]:A monster is hitting us... 
		{
			if(SIDEKICK_HasTaskInQue(self,TASKTYPE_SIDEKICK_EVADE))
			{
				if(pCurrentTask)
				{
					if(pCurrentTask->nTaskType == TASKTYPE_SIDEKICK_STOP)
					{
						AI_RemoveCurrentTask(self,FALSE);
					}
				}
			}
		}
		// NSS[2/12/00]:Test to see what is hurting us
		if(!(pAttacker->flags & (FL_BOT|FL_CLIENT|FL_MONSTER)))
		{
			// NSS[2/12/00]:If it is a door smashing us let's try and get out of its way
			if(pAttacker->className && (strstr(pAttacker->className,"door")||strstr(pAttacker->className,"train")) )
			{
				if (!SIDEKICK_HasTaskInQue(self,TASKTYPE_MOVEAWAY))
				{
					AI_AddNewTaskAtFront( self, TASKTYPE_MOVEAWAY, pAttacker );
				}
				else
				{
					CVector dir;
					self->s.angles.AngleToForwardVector( dir );
					dir.Negate();
					AI_FindNewCourse(self, &dir);
				}
			}
		}
	}

	if(pCurrentTask)
	{
		if (TASK_GetType( pCurrentTask ) == TASKTYPE_WAITONLADDERFORNOCOLLISION )
		{
			self->movetype = MOVETYPE_WALK;
		}
		
		if( TASK_GetType( pCurrentTask ) != TASKTYPE_SIDEKICK_ATTACK)
		{
			if(pAttacker)
			{
				if((pAttacker->flags & FL_MONSTER) && !hook->bCarryingMikiko && !SIDEKICK_HasTaskInQue(self,TASKTYPE_SIDEKICK_ATTACK) && SIDEKICK_IsOkToFight( self )&& SIDEKICK_CheckMonsterType(self,pAttacker))
				{
					AI_AddNewTaskAtFront(self,TASKTYPE_SIDEKICK_ATTACK);
				}
			}
		}
	}

	// NSS[12/6/99]:Remove any stay or no attack command.
	hook->ai_flags &= ~AI_STAY;
	hook->ai_flags &= ~AI_NOATTACK;
	hook->ai_flags |= AI_ATTACK;
	hook->ai_flags |= AI_FOLLOW;

	weapon_t *pWeapon = (weapon_t *) self->curWeapon;

	if(pWeapon)
	{
		if(SIDEKICK_CanUseWeapon( pWeapon ))
		{
			hook->bInAttackMode = TRUE;
		}	
	}

	AI_SetNextThinkTime( self, 0.1f );
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 3.0f );
	AI_SetMovingCounter( hook, 0 );

	if ( self->health < 0.0f )
	{
		if (!( self->flags & FL_FREEZE )&&check_for_gib(self,damage)) // able to gib?// NSS[3/8/00]:Medusa was gibbing us and this was the problem added the check for FL_FREEZE
		{

			userEntity_t *client_entity = &gstate->g_edicts[1];	
			playerHook_t *phook = AI_GetPlayerHook( client_entity );


			if( ( client_entity->inuse != 0 ) && ( client_entity->deadflag == DEAD_NO ) )
			{
				if (phook->camera)  // if previous camera in use, stop it
					camera_stop(client_entity, false);

				camera_start (client_entity, false);	// prepare camera
			}


			self->s.renderfx = RF_NODRAW;

			// dismember this dead bastard
			pCameraTarget = gib_client_die(self,pAttacker);

			if( client_entity->inuse != 0 )
			{
				camera_set_target (phook->camera, pCameraTarget);
				camera_mode (client_entity, CAMERAMODE_GIBCAM, false);
			}

			if ( hook->pRespawnData )
			{
				self->think = SPAWN_RespawnThink;
				AI_SetNextThinkTime( self, 5.0f );
			}
			else
			{
				self->deadflag	= DEAD_DEAD;
				self->pain		= NULL;
				self->use		= NULL;
				self->die		= NULL;
				self->think		= NULL;
				self->prethink	= NULL;
				self->postthink	= NULL;

				if ( hook->pGoals )
				{
					GOALSTACK_Delete( hook->pGoals );
					hook->pGoals = NULL;
				}

				if ( hook->pScriptGoals )
				{
					GOALSTACK_Delete( hook->pScriptGoals );
					hook->pScriptGoals = NULL;
				}

				UNIQUEID_Remove( hook->szScriptName );
				free( hook->szScriptName );
				hook->szScriptName = NULL;

				alist_remove( self );
				self->remove( self );

				if( ( client_entity->inuse != 0 ) && ( client_entity->deadflag == DEAD_NO ) )
				{
					AlertClientOfSidekickDeath(false); // game over  (true == gibbed)
				}
			}
		}

		return;
	}
	else
	{
		// play PAIN sounds
		// NSS[1/31/00]:Added water and lava stuff
		if((self->watertype & CONTENTS_WATER) && pAttacker == self &&  (GetCurrentEpisode() == 3))
		{
			if(rnd() < 0.15)
			{
				Com_sprintf(soundname,sizeof(soundname),"icehurt%c.wav",('1' + (rand() % 2)));
				SIDEKICK_PlaySound(self,soundname);
				if(self->s.color.x <= 0.0f || self->s.color.y <= 0.0f)
				{
					self->s.color.x = 1.0f;
					self->s.color.y = 1.0f;
					self->s.color.z = 0.25f;
				}
				if(self->s.color.z < 1.0)
				{
					self->s.color.z +=0.10;
					if(self->s.color.x > 0.10)
					{
						self->s.color.x -=0.10;
						self->s.color.y -=0.10;
					}
				}
			}
			

		}
		else if((self->watertype & CONTENTS_WATER) && pAttacker == self )
		{
			if(rnd() < 0.15)
			{
				if(hook->type == TYPE_MIKIKO)
					Com_sprintf(soundname,sizeof(soundname),"waterchoke%c.wav",('1' + ((rand() % 2)+1)));
				else
					Com_sprintf(soundname,sizeof(soundname),"waterchoke%c.wav",('1' + ((rand() % 2)+2)));
				SIDEKICK_PlaySound(self,soundname);
			}
		}
		else if((self->watertype & (CONTENTS_SLIME|CONTENTS_LAVA)) && pAttacker == self )
		{
			if(rnd() < 0.15)
			{
				Com_sprintf(soundname,sizeof(soundname),"death%c.wav",('1' + ((rand() % 3)+5)));
				SIDEKICK_PlaySound(self,soundname);
			}
		}
		else
		{
			Com_sprintf(soundname,sizeof(soundname),"pain%c.wav",('1' + (rand() % 4)));
			SIDEKICK_PlaySound(self,soundname);
		}
		
		
		
	}

	// still in a pain function from last hit?
	if ( hook->pain_finished >= gstate->time || self == pAttacker )
	{
		return;
	}

    // only attack monsters and do not attack player and other sidekicks
    _ASSERTE( pAttacker );
	if ( pAttacker->flags & FL_MONSTER )
	{
		int new_enemy = FALSE;
		playerHook_t *ahook = AI_GetPlayerHook( pAttacker );

		if ( pAttacker->flags & FL_MONSTER )
		{
			new_enemy = TRUE;
		}
		else
		//	bots only attack entities with opposite alignment
		if ( hook->dflags & DFL_EVIL && (ahook->dflags & (DFL_GOOD | DFL_NEUTRAL)) )
		{
			new_enemy = TRUE;
		}
		else 
		if ( hook->dflags & DFL_GOOD &&	(ahook->dflags & (DFL_EVIL | DFL_NEUTRAL)) )
		{
			new_enemy = TRUE;
		}
		//	or anything if they are berserk
		else 
		if ( hook->dflags & DFL_BERSERK )
		{
			new_enemy = TRUE;
		}

		if ( !ahook->pNodeList )
		{
			new_enemy = FALSE;
		}
		else 
		if ( ahook->pNodeList->nCurrentNodeIndex < 0 )
		{
			new_enemy = FALSE;
		}

		if ( new_enemy )
		{
            SIDEKICK_SetAttackMode( self, pAttacker );
		}
	}

   	int nRandomValue = (int)(Random() * 99.9);
	if ( nRandomValue < hook->pain_chance )
	{
        char szAnimation[16];
        strcpy( szAnimation, "hit" );
		AI_SelectAnimationPerWeaponType( self, szAnimation );
		weaponInfo_t *pWeaponInfo = (weaponInfo_t *)self->winfo;
		if(pWeaponInfo)
		{
			if(pWeaponInfo->nWeaponType == WEAPONTYPE_GLOVE)
			{
				int nStrLength = strlen( szAnimation );
				szAnimation[nStrLength] = 'a';
				szAnimation[nStrLength+1] = '\0';
			}
		}
		else
		{
				int nStrLength = strlen( szAnimation );
				szAnimation[nStrLength] = 'a';
				szAnimation[nStrLength+1] = '\0';		
		}
		frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
        if ( pSequence )
        {
		    AI_ForceSequence(self, pSequence, FRAME_ONCE);

		    AI_Dprintf( "Starting TASKTYPE_PAIN.\n" );
		    ai_frame_sounds( self );
		    AI_AddNewTaskAtFront( self, TASKTYPE_PAIN );
        }
	}
    hook->pain_finished = gstate->time + (self->s.frameInfo.endFrame - self->s.frameInfo.startFrame) * self->s.frameInfo.frameTime;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SetAttackSequence
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SetAttackSequence( userEntity_t *self )
{
	_ASSERTE( self );

	if ( self->curWeapon )
	{
//		playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
		char szAnimation[16];
		AI_SelectRunningAnimation( self, szAnimation );
        AI_ForceSequence( self, szAnimation, FRAME_LOOP );
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_IsGap
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsGap( userEntity_t *self, CVector &forwardVector )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    float fSpeed = AI_ComputeMovingSpeed( hook );	
	float fDistance = fSpeed * 0.2f;

    CVector nextPoint;
    VectorMA( self->s.origin, forwardVector, fDistance, nextPoint );
	if ( !AI_IsGroundBelowBetween( self, self->s.origin, nextPoint ) )
	{
		return TRUE;
	}
	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_MoveTowardPoint
// Description:
//				move straight toward a point
// Input:
// Output:
//				TRUE	=> if reached the point
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_MoveTowardPoint( userEntity_t *self, CVector &destPoint, 
						int bMovingTowardNode /* = FALSE */, int bFaceTowardDest /* = TRUE */ )
{
	_ASSERTE( self );
	
	if ( AI_IsInWater( self ) )
    {
        return AI_SwimTowardPoint( self, destPoint, bMovingTowardNode );
    }

	// if this unit is going outside of its bounding box, then terminate current goal
	if ( AI_IsInsideBoundingBox( self ) == FALSE )
	{
		AI_RemoveCurrentGoal( self );
		return FALSE;
	}

	playerHook_t *hook = AI_GetPlayerHook( self );
	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return FALSE;
	}
	
	// we can just check the x,y distance since this function assumes that the 
	// dest point is visible from self
	float fXYDistance = VectorXYDistance( self->s.origin, destPoint );
	float fZDistance = VectorZDistance( self->s.origin, destPoint );
	if ( AI_IsCloseDistance2(self, fXYDistance ) && fZDistance < 32.0f )
	{
		AI_UpdateCurrentNode( self );
        return TRUE;
	}

	if ( bFaceTowardDest )
	{
        AI_FaceTowardPoint( self, destPoint );
	}

	float fSpeed = AI_ComputeMovingSpeed( hook );

    if ( !self->groundEntity )
    {
		if ( AI_IsCloseDistance2( self, fXYDistance ) )
		{
			AI_UpdateCurrentNode( self );
            return TRUE;
		}

        if ( self->waterlevel > 0 && self->waterlevel < 3 )
	    {
            float fHeight = 8.0f + ((float)self->waterlevel * 8.0f);
            if ( fZDistance > fHeight  )
            {
                // must be stuck trying to go out of the water, try to jump
	            self->velocity.z = hook->upward_vel*0.50;
            }
            else
            {
                forward = destPoint - self->s.origin;
                forward.Normalize();

                //	scale speed based on current frame's move_scale
	            float fScale = FRAMES_ComputeFrameScale( hook );
	            AI_SetVelocity( self, forward, (fSpeed * fScale) );
            }

            AI_UpdateCurrentNode( self );

            return FALSE;
        }
        else
        {
            AI_HandleOffGround( self, destPoint, fSpeed );
        }

		return FALSE;
	}

    if ( self->groundEntity && self->waterlevel == 2 )
    {
        if ( fZDistance > 24.0f  )
        {
            // must be stuck trying to go out of the water, try to jump
	        self->velocity.z = hook->upward_vel*0.5f;
            self->groundEntity = NULL;
            AI_UpdateCurrentNode( self );
            return FALSE;
        }
    }

	forward = destPoint - self->s.origin;

	// should never be in water, but just in case
	int bOnFlatSurface = AI_IsOnFlatSurface( self );
	if ( self->waterlevel < 3 && bOnFlatSurface && self->movetype != MOVETYPE_HOP )
	{
		forward.z = 0.0f;
	}
	forward.Normalize();

	if ( AI_HandleCollisionWithEntities( self, destPoint, fSpeed ) )
	{
		AI_UpdateCurrentNode( self );
        return FALSE;
	}

	if ( AI_HandleGroundObstacle( self, fSpeed, bMovingTowardNode ) )
	{
		AI_UpdateCurrentNode( self );
        return FALSE;
	}

	float fOneFrameDistance = fSpeed * 0.125f;
    if ( !bMovingTowardNode && AI_IsGap( self, destPoint, fOneFrameDistance ) )
	{
		AI_StopEntity( self );
        return FALSE;
	}

	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	AI_SetVelocity( self, forward, (fSpeed * fScale) );

	if ( AI_HandleGettingStuck( self, destPoint, fSpeed ) )
	{
		AI_UpdateCurrentNode( self );
        return FALSE;
	}

	hook->last_origin = self->s.origin;

	AI_UpdateCurrentNode( self );

    AI_HandleCrouching( self );

	ai_frame_sounds( self );
	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_MoveTowardPointWhileAttacking
// Description:
//				move straight toward a point
// Input:
// Output:
//				TRUE	=> if reached the point
//				FALSE	=> otherwise
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_MoveTowardPointWhileAttacking( userEntity_t *self, CVector &destPoint, 
						                    int bMovingTowardNode /* = FALSE */, int bFaceTowardDest /* = TRUE */ )
{
	_ASSERTE( self );
	
	if ( AI_IsInWater( self ) )
    {
        return AI_SwimTowardPoint( self, destPoint, bMovingTowardNode );
    }

	playerHook_t *hook = AI_GetPlayerHook( self );
	
	// we can just check the x,y distance since this function assumes that the 
	// dest point is visible from self
	float fXYDistance = VectorXYDistance( self->s.origin, destPoint );
	float fZDistance = VectorZDistance( self->s.origin, destPoint );
	if ( AI_IsCloseDistance2( self, fXYDistance ) && fZDistance < 32.0f )
	{
        AI_UpdateCurrentNode( self );
        return TRUE;
	}

	userEntity_t *pEnemy = self->enemy;
    if ( AI_IsAlive( pEnemy ) && AI_IsLineOfSight( self, pEnemy ) )
    {
        SIDEKICK_FaceTowardPoint( self, pEnemy->s.origin );
	}
    else
    {
        if ( bFaceTowardDest )
        {
            SIDEKICK_FaceTowardPoint( self, destPoint );
        }
    }

	float fSpeed = AI_ComputeMovingSpeed( hook );

	if ( !self->groundEntity && self->waterlevel < 3 )
	{
		if ( AI_IsCloseDistance2( self, fXYDistance ) )
		{
			AI_UpdateCurrentNode( self );
            return TRUE;
		}

		return FALSE;
	}

	forward = destPoint - self->s.origin;

	// should never be in water, but just in case
	int bOnFlatSurface = AI_IsOnFlatSurface( self );
	if ( self->waterlevel < 3 && bOnFlatSurface )
	{
		forward.z = 0.0f;
	}
	forward.Normalize();

	if ( AI_HandleGroundObstacle( self, fSpeed, bMovingTowardNode ) )
	{
        AI_UpdateCurrentNode( self );
		return FALSE;
	}

	/*if ( SIDEKICK_IsGap( self, forward ) )
	{
		AI_StopEntity( self );
        return FALSE;
	}*/

	if ( !bMovingTowardNode && AI_HandleGap( self, destPoint, fSpeed, fXYDistance, fZDistance ) )
	{
		AI_UpdateCurrentNode( self );
        return FALSE;
	}

	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	AI_SetVelocity( self, forward, (fSpeed * fScale) );

//	if ( AI_HandleGettingStuck( self, destPoint, fSpeed ) )
//	{
//		return FALSE;
//	}

	hook->last_origin = self->s.origin;

	AI_UpdateCurrentNode( self );

    AI_HandleCrouching( self );

	ai_frame_sounds( self );

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_MoveWhileAttacking
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_MoveWhileAttacking( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    if ( AI_IsInWater( self ) )
    {
        return AI_Swim( self );
    }
	
	if ( hook->pPathList->pPath && hook->pPathList->pPath->nNodeIndex >= 0 )
	{
		_ASSERTE( pGroundNodes );

		MAPNODE_PTR pNode = NODE_GetNode( pGroundNodes, hook->pPathList->pPath->nNodeIndex );
        _ASSERTE( pNode );
		
		CVector destPoint = pNode->position;
		if ( SIDEKICK_MoveTowardPointWhileAttacking( self, destPoint, TRUE ) == TRUE )
		{
			// we're already at this node, so delete the first one
			PATHLIST_DeleteFirstInPath( hook->pPathList );

			if ( hook->pPathList->pPath )
			{
				MAPNODE_PTR pNextNode = NODE_GetNode( pGroundNodes, hook->pPathList->pPath->nNodeIndex );
				if ( AI_HandleUse( self, pGroundNodes, pNode, pNextNode ) )
				{
					return TRUE;
				}
			}

			SIDEKICK_MoveWhileAttacking( self );
		}

		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_ModifyStrafeDirectionAfterCollision
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_ModifyStrafeDirectionAfterCollision( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	switch ( hook->strafe_dir )
	{
		case STRAFE_LEFT:
		{
			hook->strafe_dir = STRAFE_RIGHT;
			break;
		}
		case STRAFE_RIGHT:
		{
			hook->strafe_dir = STRAFE_LEFT;
			break;
		}
		case STRAFE_UPLEFT:
		{
			hook->strafe_dir = STRAFE_DOWNRIGHT;
			break;
		}
		case STRAFE_UPRIGHT:
		{
			hook->strafe_dir = STRAFE_DOWNLEFT;
			break;
		}
		case STRAFE_DOWNLEFT:
		{
			hook->strafe_dir = STRAFE_UPRIGHT;
			break;
		}
		case STRAFE_DOWNRIGHT:
		{
			hook->strafe_dir = STRAFE_UPLEFT;
			break;
		}
		default:
		{
			break;
		}
	}

    hook->strafe_time = gstate->time + 3.0f;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_HandleCollision
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_HandleCollision( userEntity_t *self, float fSpeed, CVector &forwardDirection )
{
    if ( SIDEKICK_IsGap( self, forwardDirection ) )
    {
        SIDEKICK_ModifyStrafeDirectionAfterCollision( self );
        return;    
    }

	playerHook_t *hook = AI_GetPlayerHook( self );

	float fDist = fSpeed * 0.1f;
	if ( self->velocity.Length() > 0 )
	{
		fDist = fSpeed * 0.2f;
	}
	if ( fDist < 32.0f )
	{
		fDist = 32.0f;
	}

	int nWallType = AI_CheckAirTerrain( self, forwardDirection, fDist );
	switch ( nWallType )
	{
		case TER_OBSTRUCTED:
		{
			AI_Dprintf( "Strafing along the wall.\n" );

			CVector dir;
			AI_ComputeAlongWallVector( self, forwardDirection, terrain.wall_normal, dir );
			forwardDirection = dir;
			
			SIDEKICK_ModifyStrafeDirectionAfterCollision( self );
			break;
		}
		case TER_WALL:
		{
			AI_Dprintf( "Strafing along the wall.\n" );

			CVector dir;
			AI_ComputeAlongWallVector( self, forwardDirection, terrain.wall_normal, dir );
			forwardDirection = dir;

			SIDEKICK_ModifyStrafeDirectionAfterCollision( self );
			break;
		}
		
		case TER_HIGH_OBS:
		{
			gstate->Con_Dprintf( "Detected high obstruction.\n" );
			
			// if it is a ceiling then, glide along it
			if ( terrain.wall_normal.z < -0.7f )
			{
				forwardDirection.z = 0.0f;
				forwardDirection.Normalize();
			}
			else
			{
				// duck under
				forwardDirection.x = 0.0f;
				forwardDirection.y = 0.0f;
				forwardDirection.z = -1.0f;
			}
			break;
		}
		case TER_WALL_RIGHT:
		{
			gstate->Con_Dprintf( "Right side Blocked.\n" );

			// right side is blocked
			// rotate the wall normal to the left so that we can move around this 
			// obstacle
			CVector dir( terrain.wall_normal.y, -terrain.wall_normal.x, forwardDirection.z );
			dir.Normalize();
			forwardDirection = dir;
			
			SIDEKICK_ModifyStrafeDirectionAfterCollision( self );
			break;
		}
		case TER_WALL_LEFT:
		{
			gstate->Con_Dprintf( "Left side Blocked.\n" );
			
			// left side is blocked
			// rotate the wall normal to the right so that we can move around this 
			// obstacle
			CVector dir( -terrain.wall_normal.y, terrain.wall_normal.x, forwardDirection.z );
			dir.Normalize();
			forwardDirection = dir;
			
			SIDEKICK_ModifyStrafeDirectionAfterCollision( self );
			break;
		}		
		case TER_LOW_OBS:
		{
			// go up
			gstate->Con_Dprintf( "Detected low obstruction.\n" );

			if ( terrain.wall_normal.z > 0.7f )
			{
				forwardDirection.z = 0.0f;
				forwardDirection.Normalize();
			}
			else
			{
				forwardDirection.x = 0.0f;
				forwardDirection.y = 0.0f;
				forwardDirection.z = 1.0f;
			}
			break;
		}

		default:
		{
			break;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_HandleCollisionWhileAttacking
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_HandleCollisionWhileAttacking( userEntity_t *self, userEntity_t *pEnemy, 
                                                    float fSpeed, CVector &forwardDirection )
{
    _ASSERTE( self );
    _ASSERTE( pEnemy );

    SIDEKICK_HandleCollision( self, fSpeed, forwardDirection );
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SelectAnimationWhileAttacking
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SelectAnimationWhileAttacking( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

    if ( self->velocity.Length() > 0.0f )
    {
        frameData_t *pCurrentSequence = hook->cur_sequence;
        if ( !strstr( pCurrentSequence->animation_name, "run" ) )
        {
		    char szAnimation[16];
		    AI_SelectRunningAnimation( self, szAnimation );
            AI_ForceSequence( self, szAnimation, FRAME_LOOP );
        }
    }
    else
    {
        float fWeaponRange = AI_GetCurrentWeaponRange( self );
        if ( fWeaponRange >= SIDEKICK_MELEE_ATTACK_DISTANCE && AI_IsEndAnimation(self))
        {
            frameData_t *pCurrentSequence = hook->cur_sequence;
			weapon_t *pWeapon = (weapon_t *) self->curWeapon;
            //if ( !strstr( pCurrentSequence->animation_name, "amb" ) && _stricmp( pWeapon->name, "weapon_discus" ) )
            //{
                char szAnimation[16];
	            //AI_SelectAnimationPerWeaponType( userEntity_t *self, char *pszAnimation )
//				sprintf(szAnimation,"atak");
				Com_sprintf(szAnimation,sizeof(szAnimation),"atak");
				AI_SelectAnimationPerWeaponType( self, szAnimation );

                AI_ForceSequence(self, szAnimation, FRAME_LOOP);
            //}
        }
    }
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_StickCloseToPlayer
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_StickCloseToPlayer( userEntity_t *self )
{
	userEntity_t *pEnemy = self->enemy;
	_ASSERTE( AI_IsAlive( pEnemy ) );

    playerHook_t *hook = AI_GetPlayerHook( self );
    userEntity_t *pOwner = hook->owner;

    float fDistance = VectorDistance( self->s.origin, pOwner->s.origin );
    float fSpeed = AI_ComputeMovingSpeed( hook );	

	// make this sidekick stay at certain angle from the player relative to where
    // the enemy and player is located and at a certain distance from the player
    CVector ownerVectorTowardEnemy;
	CVector ownerAngleTowardEnemy;
	ownerVectorTowardEnemy = pEnemy->s.origin - pOwner->s.origin;
	ownerVectorTowardEnemy.Normalize();
	VectorToAngles( ownerVectorTowardEnemy, ownerAngleTowardEnemy );
    ownerAngleTowardEnemy.pitch = 0.0f;   // pitch should be zero, a sidekick should be
                                          // at about the same plane as the player
    
    float fAngle = fAngleAwayFromOwner;
    int nWhichSide = AI_WhichSide( pOwner->s.origin, pEnemy->s.origin, self->s.origin );
    if ( nWhichSide == ON_RIGHT )
    {
        fAngle = -fAngleAwayFromOwner;
    }
 	ownerAngleTowardEnemy.y = AngleMod( ownerAngleTowardEnemy.y + fAngle );
    
    CVector forwardVectorFromOwner;
    ownerAngleTowardEnemy.AngleToForwardVector( forwardVectorFromOwner );

	CVector destPoint;
	VectorMA( pOwner->s.origin, forwardVectorFromOwner, fNormalDistanceFromOwner, destPoint );

	tr = gstate->TraceLine_q2( self->s.origin, destPoint, self, MASK_SOLID );
	if ( tr.fraction < 1.0f )
    {
        float fDistanceToDestFromSelf = VectorDistance( self->s.origin, destPoint );
        CVector forwardVectorFromSelf = destPoint - self->s.origin;
        forwardVectorFromSelf.Normalize();
        
        float fDistanceToCollidedPoint = (fDistanceToDestFromSelf * tr.fraction) - 32.0f;
        VectorMA( self->s.origin, forwardVectorFromSelf, fDistanceToCollidedPoint, destPoint );

        float fDistanceToDestFromOwner = VectorDistance( pOwner->s.origin, destPoint );
        if ( fDistanceToDestFromOwner < fCloseDistanceToOwner )
        {
            // do not move
            destPoint = self->s.origin;
        }
    }

    CVector groundPoint;
	CVector movingVector;
    if ( AI_FindGroundPoint( self, destPoint, groundPoint ) )
    {
	    movingVector = groundPoint - self->s.origin;
	    movingVector.Normalize();
    }
    else
    {
        // do not move
    }

    float fDistanceToDestPoint = VectorDistance( self->s.origin, groundPoint );
    if ( fDistanceToDestPoint < fOneHalfFrameDistance )
    {
        movingVector = pOwner->velocity;
        movingVector.Normalize();
    }

    if ( movingVector.Length() > 0.0f )
    {
	    SIDEKICK_HandleCollision( self, fSpeed, movingVector );

	    if ( SIDEKICK_IsGap( self, movingVector ) )
        {
            AI_StopEntity( self );
			return FALSE;
        }
        else
        {
            // scale speed based on current frame's move_scale
	        float fScale = FRAMES_ComputeFrameScale( hook );
            AI_SetVelocity( self, movingVector, (fSpeed * fScale) );
        }
    }

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_MoveCloserTowardEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_MoveCloserTowardEnemy( userEntity_t *self, userEntity_t *pEnemy )
{
    playerHook_t *hook = AI_GetPlayerHook( self );

    _ASSERTE( pEnemy );

    float fSpeed = AI_ComputeMovingSpeed( hook );	
	
	CVector vectorTowardEnemy;
	CVector angleTowardEnemy;
	vectorTowardEnemy = pEnemy->s.origin - self->s.origin;
	vectorTowardEnemy.Normalize();
	VectorToAngles( vectorTowardEnemy, angleTowardEnemy );
	
	float fYAngle = 45.0f;
    if ( rnd() > 0.8f )
    {
        fYAngle += 15.0f;
    }

	switch ( hook->strafe_dir )
	{
		case STRAFE_LEFT:
		{
			angleTowardEnemy.y += fYAngle;
			break;
		}
		case STRAFE_RIGHT:
		{
			angleTowardEnemy.y -= fYAngle;
			break;
		}
		case STRAFE_UPLEFT:
		{
			angleTowardEnemy.y += fYAngle;
			break;
		}
		case STRAFE_UPRIGHT:
		{
			angleTowardEnemy.y -= fYAngle;
			break;
		}
		case STRAFE_DOWNLEFT:
		{
			angleTowardEnemy.y += fYAngle;
			break;
		}
		case STRAFE_DOWNRIGHT:
		{
			angleTowardEnemy.y -= fYAngle;
			break;
		}
		default:
		{
			break;
		}
	}
	
    angleTowardEnemy.y = AngleMod( angleTowardEnemy.y );
	
	CVector forwardVector;
	angleTowardEnemy.AngleToForwardVector( forwardVector );
	forwardVector.Normalize();

	SIDEKICK_HandleCollisionWhileAttacking( self, pEnemy, fSpeed, forwardVector );

    if ( SIDEKICK_IsGap( self, forwardVector ) )
    {
        AI_StopEntity( self );
        return TRUE;
    }

	// scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
    AI_SetVelocity( self, forwardVector, (fSpeed * fScale) );

    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_MoveTowardEntity
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_MoveTowardEntity( userEntity_t *self, userEntity_t *pEntity )
{
    _ASSERTE( self );
    _ASSERTE( pEntity );

    playerHook_t *hook = AI_GetPlayerHook( self );

	float fXYDistance = VectorXYDistance( self->s.origin, pEntity->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pEntity->s.origin );
	if ( AI_IsOkToMoveStraight( self, pEntity->s.origin, fXYDistance, fZDistance ) )
	{
		SIDEKICK_MoveTowardPointWhileAttacking( self, pEntity->s.origin, FALSE );
    }
    else
    {
        // move toward owner until can no longer see the enemy
		if ( !AI_IsPathToEntityClose( self, pEntity ) )
        {
            if ( !AI_FindPathToEntity( self, pEntity ) )
            {
                return;
            }
		}

		if ( SIDEKICK_MoveWhileAttacking( self ) == FALSE )
		{
			PATHLIST_KillPath( hook->pPathList );
		}
    }
        	
	hook->nMoveCounter++;

    SIDEKICK_SelectAnimationWhileAttacking( self );
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_UpdateMeleeAttackMovement
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_UpdateMeleeAttackMovement( userEntity_t *self )
{
	userEntity_t *pEnemy = self->enemy;
	playerHook_t *hook = AI_GetPlayerHook( self );
    userEntity_t *pOwner = hook->owner;
	
	
	
	if ( !AI_IsAlive( pEnemy ) )
	{
		return FALSE;
	}

	AI_SetStateRunning(hook);
    
	SIDEKICK_SelectAnimationWhileAttacking( self );

    // if the owner is hidden from the enemy then move closer to him
    float fDistanceToOwner = fNormalDistanceFromOwner;
    int bCanOwnerSeeEnemy = AI_IsLineOfSight( pOwner, pEnemy );
    if ( !bCanOwnerSeeEnemy )
    {
        fDistanceToOwner = fCloseDistanceToOwner;
    }

    int bCanSeeOwner = AI_IsLineOfSight( self, pOwner );
    int bCanSeeEnemy = AI_IsLineOfSight( self, pEnemy );

    if ( !bCanSeeOwner && AI_IsAlive(self))
    {
		SIDEKICK_MoveTowardEntity( self, pOwner );
        return TRUE;
    }
    
    // this sidekick can see the owner now
    
    if ( !bCanOwnerSeeEnemy && bCanSeeEnemy )
    {
		SIDEKICK_MoveTowardEntity( self, pOwner );
        return TRUE;
    }

    if ( !SIDEKICK_IsOwnerClose( self ) )
    {
        if ( SIDEKICK_StickCloseToPlayer( self ) == FALSE )
		{
			return FALSE;
		}
    }
    else
    {
		float fDistance = VectorDistance( self->s.origin, pEnemy->s.origin );
		if ( fDistance < AI_GetCurrentWeaponRange( self ) )
		{
            AI_ZeroVelocity( self );
		    // NSS[12/14/99]:Give them a little movement
			if(AI_IsEnemyTargetingMe(self,self->enemy) && (rnd() < 0.25))
			{
				AI_AddNewTaskAtFront(self,TASKTYPE_SIDESTEP);
			}

        }
        else
        {
			// SCG[10/20/99]: this was causing a problem
            if( SIDEKICK_MoveTowardPointWhileAttacking( self, pEnemy->s.origin ) == FALSE )
			{
				return FALSE;
			}
        }
    }


    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_ShouldStrafe
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define STRAFE_CHECK_DISTANCE   128.0f

int SIDEKICK_ShouldStrafe( userEntity_t *self, userEntity_t *pEnemy )
{
	CVector vectorTowardEnemy;
	CVector angleTowardEnemy;
	vectorTowardEnemy = pEnemy->s.origin - self->s.origin;
	vectorTowardEnemy.Normalize();
	VectorToAngles( vectorTowardEnemy, angleTowardEnemy );
	
	float fYAngle = 85.0f;
    float fDistance = VectorDistance( self->s.origin, pEnemy->s.origin );
    float fRange = AI_GetCurrentWeaponRange( self );
	if ( fDistance < fRange * 0.35f )
    {
        fYAngle = 95.0f;
    }
    else
    if ( fDistance > fRange * 0.75f )
    {
        fYAngle = 45.0f;
    }
    
    angleTowardEnemy.y = fYAngle;
	
	CVector forwardVector;
	angleTowardEnemy.AngleToForwardVector( forwardVector );
	forwardVector.Normalize();

	CVector mins(self->s.mins.x + 8.0f, self->s.mins.y + 8.0f, self->s.mins.z + 16.0f );
	CVector maxs(self->s.maxs.x - 8.0f, self->s.maxs.y - 8.0f, self->s.maxs.z );

	int bCanStrafe = TRUE;
    CVector endPoint;
	VectorMA( self->s.origin, forwardVector, STRAFE_CHECK_DISTANCE, endPoint );
	if ( AI_IsGroundBelowBetween( self, self->s.origin, endPoint ) )
	{
		tr = gstate->TraceBox_q2( self->s.origin, mins, maxs, endPoint, self, MASK_SOLID );
		if ( tr.fraction < 1.0f )
		{
            bCanStrafe = FALSE;
        }
    }
    else
    {
        bCanStrafe = FALSE;
    }

    if ( bCanStrafe == TRUE )
    {
        float fOppositeAngle = 360.0f - fYAngle;
        CVector oppositeAngle = angleTowardEnemy;
        oppositeAngle.y = fOppositeAngle;

	    CVector oppositeVector;
	    oppositeAngle.AngleToForwardVector( oppositeVector );
	    oppositeVector.Normalize();

//	    endPoint;// SCG[1/23/00]: not used
	    VectorMA( self->s.origin, oppositeVector, STRAFE_CHECK_DISTANCE, endPoint );
	    if ( AI_IsGroundBelowBetween( self, self->s.origin, endPoint ) )
	    {
		    tr = gstate->TraceBox_q2( self->s.origin, mins, maxs, endPoint, self, MASK_SOLID );
		    if ( tr.fraction >= 1.0f )
		    {
                bCanStrafe = FALSE;
            }
        }
        else
        {
            bCanStrafe = FALSE;
        }
    }

    return bCanStrafe;

}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_AttackWhileStrafing
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_AttackWhileStrafing( userEntity_t *self, userEntity_t *pEnemy )
{
    _ASSERTE( self );
    _ASSERTE( pEnemy );

    if ( self->groundEntity == NULL && self->waterlevel < 3 )
    {
        return TRUE;
    }

    playerHook_t *hook = AI_GetPlayerHook( self );

    int bCanSeeEnemy = AI_IsCompletelyVisible( self, pEnemy, SIDEKICK_WIDTH_FACTOR );
    if ( bCanSeeEnemy == NOT_VISIBLE )
    {
        // end this state
        SIDEKICK_SetAttackTime( hook, gstate->time );
        return TRUE;
    }

    float fSpeed = AI_ComputeMovingSpeed( hook );	
	
	CVector vectorTowardEnemy;
	CVector angleTowardEnemy;
	vectorTowardEnemy = pEnemy->s.origin - self->s.origin;
	vectorTowardEnemy.Normalize();
	VectorToAngles( vectorTowardEnemy, angleTowardEnemy );
	
	float fYAngle = 85.0f;
    float fDistance = VectorDistance( self->s.origin, pEnemy->s.origin );
    float fRange = AI_GetCurrentWeaponRange( self );
	if ( fDistance < fRange * 0.35f )
    {
        fYAngle = 95.0f;
    }
    else
    if ( fDistance > fRange * 0.75f )
    {
        fYAngle = 45.0f;
    }

    if ( rnd() > 0.8f )
    {
        fYAngle += 15.0f;
    }

	switch ( hook->strafe_dir )
	{
		case STRAFE_LEFT:
		{
			angleTowardEnemy.y += fYAngle;
			break;
		}
		case STRAFE_RIGHT:
		{
			angleTowardEnemy.y -= fYAngle;
			break;
		}
		case STRAFE_UPLEFT:
		{
			angleTowardEnemy.y += fYAngle;
			break;
		}
		case STRAFE_UPRIGHT:
		{
			angleTowardEnemy.y -= fYAngle;
			break;
		}
		case STRAFE_DOWNLEFT:
		{
			angleTowardEnemy.y += fYAngle;
			break;
		}
		case STRAFE_DOWNRIGHT:
		{
			angleTowardEnemy.y -= fYAngle;
			break;
		}
		default:
		{
			break;
		}
	}
	
    angleTowardEnemy.y = AngleMod( angleTowardEnemy.y );
	
	CVector forwardVector;
	angleTowardEnemy.AngleToForwardVector( forwardVector );
	forwardVector.Normalize();

	SIDEKICK_HandleCollisionWhileAttacking( self, pEnemy, fSpeed, forwardVector );

    if ( SIDEKICK_IsGap( self, forwardVector ) )
    {
        AI_StopEntity( self );
        return TRUE;
    }

	// scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
    AI_SetVelocity( self, forwardVector, (fSpeed * fScale) );

    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_TakeCover
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_TakeCover( userEntity_t *self, userEntity_t *pEnemy )
{
	_ASSERTE( self );
    _ASSERTE( pEnemy );
	playerHook_t *hook = AI_GetPlayerHook( self );

	SIDEKICK_MoveWhileAttacking( self );

	if ( !hook->pPathList->pPath )
	{
		//	set up a path consisting of one node
		MAPNODE_PTR pRetreatNode = NODE_FurthestFrom( hook->pNodeList, pEnemy->s.origin );
		if ( !pRetreatNode )
		{
            // no longer in backaway
            SIDEKICK_SetAttackTime( hook, gstate->time );
			return TRUE;
		}

		PATHLIST_AddNodeToPath( hook->pPathList, pRetreatNode );

        SIDEKICK_MoveWhileAttacking( self );
	}

    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_StayStill
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_StayStill( userEntity_t *self, userEntity_t *pEnemy )
{
	_ASSERTE( self );
    _ASSERTE( pEnemy );
	playerHook_t *hook = AI_GetPlayerHook( self );

    AI_ZeroVelocity( self );

    int bCanSeeEnemy = AI_IsCompletelyVisible( self, pEnemy, SIDEKICK_WIDTH_FACTOR );
    if ( bCanSeeEnemy != NOT_VISIBLE )
    {
        // end this state
        SIDEKICK_SetAttackTime( hook, gstate->time );
        return TRUE;
    }

    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_BackAway
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_BackAway( userEntity_t *self, userEntity_t *pEnemy )
{
	_ASSERTE( self );
    _ASSERTE( pEnemy );
	playerHook_t *hook = AI_GetPlayerHook( self );

    int bCanSeeEnemy = AI_IsCompletelyVisible( self, pEnemy, SIDEKICK_WIDTH_FACTOR );
    if ( bCanSeeEnemy == NOT_VISIBLE )
    {
        // no longer in backaway
        SIDEKICK_SetAttackTime( hook, gstate->time );
        return TRUE;
    }

	SIDEKICK_MoveWhileAttacking( self );

	if ( !hook->pPathList->pPath )
	{
		//	set up a path consisting of one node
		MAPNODE_PTR pRetreatNode = NODE_FurthestFrom( hook->pNodeList, pEnemy->s.origin );
		if ( !pRetreatNode )
		{
            // no longer in backaway
            SIDEKICK_SetAttackTime( hook, gstate->time );
			return TRUE;
		}

		PATHLIST_AddNodeToPath( hook->pPathList, pRetreatNode );

        SIDEKICK_MoveWhileAttacking( self );
	}

    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_ShouldCharge
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_ShouldCharge( userEntity_t *self, userEntity_t *pEnemy )
{
	_ASSERTE( self );
    
    if ( !AI_IsAlive( pEnemy ) )
    {
        return FALSE;
    }

    int bCanSeeEnemy = AI_IsCompletelyVisible( self, pEnemy, SIDEKICK_WIDTH_FACTOR );
    if ( bCanSeeEnemy == NOT_VISIBLE )
    {
        return TRUE;
    }

    int bShouldCharge = TRUE;
    float fDistance = VectorDistance( self->s.origin, pEnemy->s.origin );
    float fRange = AI_GetCurrentWeaponRange( self );
    if ( fRange < SIDEKICK_MELEE_ATTACK_DISTANCE )
	{
        bShouldCharge = FALSE;
    }
	else
	{
        if ( fDistance < fRange * 0.5f )
        {
            bShouldCharge = FALSE;
        }
	}

    return bShouldCharge;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_Charge
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_Charge( userEntity_t *self, userEntity_t *pEnemy )
{
    _ASSERTE( self );
    _ASSERTE( pEnemy );

    playerHook_t *hook = AI_GetPlayerHook( self );

    if ( !SIDEKICK_ShouldCharge( self, pEnemy ) )
    {
        // no longer in charge mode
        SIDEKICK_SetAttackTime( hook, gstate->time );
        return TRUE;
    }

    int bCanSeeEnemy = AI_IsCompletelyVisible( self, pEnemy, SIDEKICK_WIDTH_FACTOR );
    if ( bCanSeeEnemy == COMPLETELY_VISIBLE )
    {
        SIDEKICK_AttackWhileStrafing( self, pEnemy );
        return TRUE;
    }

	float fXYDistance = VectorXYDistance( self->s.origin, pEnemy->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pEnemy->s.origin );
	if ( AI_IsOkToMoveStraight( self, pEnemy->s.origin, fXYDistance, fZDistance ) )
	{
		SIDEKICK_MoveTowardPointWhileAttacking( self, pEnemy->s.origin, FALSE );
    }
    else
    {
        // move toward owner until can no longer see the enemy
		if ( !AI_IsPathToEntityClose( self, pEnemy ) )
        {
            if ( !AI_FindPathToEntity( self, pEnemy ) )
            {
                SIDEKICK_SetAttackTime( hook, gstate->time );
                return FALSE;
            }
		}

		if ( SIDEKICK_MoveWhileAttacking( self ) == FALSE )
		{
			PATHLIST_KillPath( hook->pPathList );
		}
    }
        	
	hook->nMoveCounter++;

    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_Jump
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_Jump( userEntity_t *self, userEntity_t *pEnemy )
{
    _ASSERTE( self );
    _ASSERTE( pEnemy );

    if ( self->groundEntity != NULL )
    {
        playerHook_t *hook = AI_GetPlayerHook( self );

	    char szAnimation[16];
	    AI_SelectJumpingAnimation( self, szAnimation );
	    AI_ForceSequence( self, szAnimation );

	    self->velocity.z += hook->upward_vel;

	    self->groundEntity = NULL;

        SIDEKICK_SetAttackTime( hook, gstate->time );
        return TRUE;
    }

    return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_UpdateRangeAttackMovement
// Description:
// Input:
// Output:
// Note:
//              try to stay close to the owner
//
// ----------------------------------------------------------------------------
#ifdef USE_STRAFE

int SIDEKICK_UpdateRangeAttackMovement( userEntity_t *self )
{
	userEntity_t *pEnemy = self->enemy;
	if ( !pEnemy )
	{
		return FALSE;
	}

    AI_ZeroVelocity( self );

	playerHook_t *hook = AI_GetPlayerHook( self );

    float fSpeed = AI_ComputeMovingSpeed( hook );	
	
	CVector vectorTowardEnemy;
	CVector angleTowardEnemy;
	vectorTowardEnemy = pEnemy->s.origin - self->s.origin;
	vectorTowardEnemy.Normalize();
	VectorToAngles( vectorTowardEnemy, angleTowardEnemy );
	
	float fYAngle = 85.0f;
    float fDistance = VectorDistance( self->s.origin, pEnemy->s.origin );
    float fRange = AI_GetCurrentWeaponRange( self );
	if ( fDistance < fRange * 0.25f )
    {
        fYAngle = 95.0f;
    }

    if ( rnd() > 0.8f )
    {
        fYAngle += 15.0f;
    }

	switch ( hook->strafe_dir )
	{
		case STRAFE_LEFT:
		{
			angleTowardEnemy.y += fYAngle;
			break;
		}
		case STRAFE_RIGHT:
		{
			angleTowardEnemy.y -= fYAngle;
			break;
		}
		case STRAFE_UPLEFT:
		{
			angleTowardEnemy.y += fYAngle;
			break;
		}
		case STRAFE_UPRIGHT:
		{
			angleTowardEnemy.y -= fYAngle;
			break;
		}
		case STRAFE_DOWNLEFT:
		{
			angleTowardEnemy.y += fYAngle;
			break;
		}
		case STRAFE_DOWNRIGHT:
		{
			angleTowardEnemy.y -= fYAngle;
			break;
		}
		default:
		{
			break;
		}
	}
	
    angleTowardEnemy.y = AngleMod( angleTowardEnemy.y );
	
	CVector forwardVector;
	angleTowardEnemy.AngleToForwardVector( forwardVector );
	forwardVector.Normalize();

	SIDEKICK_HandleCollisionWhileAttacking( self, pEnemy, fSpeed, forwardVector );

    if ( SIDEKICK_IsGap( self, forwardVector ) )
    {
        AI_StopEntity( self );
        return FALSE;
    }

	// scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
    AI_SetVelocity( self, forwardVector, (fSpeed * fScale) );

    return TRUE;
}

#else USE_STRAFE

int SIDEKICK_UpdateRangeAttackMovement( userEntity_t *self )
{
	userEntity_t *pEnemy = self->enemy;
	if ( !AI_IsAlive( pEnemy ) )
	{
		return FALSE;
	}

   // AI_ZeroVelocity( self );

    playerHook_t *hook = AI_GetPlayerHook( self );
    userEntity_t *pOwner = hook->owner;

    // if the owner is hidden from the enemy then move closer to him
// SCG[1/23/00]: not used
//    float fDistanceToOwner = fNormalDistanceFromOwner;
//    int bCanOwnerSeeEnemy = AI_IsLineOfSight( pOwner, pEnemy );
//    if ( !bCanOwnerSeeEnemy )
//    {
//        fDistanceToOwner = fCloseDistanceToOwner;
//    }

	SIDEKICK_MoveTowardEntity( self, pOwner );
	
	SIDEKICK_SelectAnimationWhileAttacking( self );
	
	return TRUE;
}
#endif USE_STRAFE



// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_UpdateAttackMovement
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_UpdateAttackMovement( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
//	userEntity_t *client_entity = &gstate->g_edicts[1];	// SCG[1/23/00]: not used
 	
	userEntity_t *pOwner = hook->owner;

	weapon_t *pWeapon = (weapon_t *) self->curWeapon;
	if ( !pWeapon || !SIDEKICK_CanUseWeapon( pWeapon ) )
	{
        SIDEKICK_ChooseBestWeapon( self );
        pWeapon = (weapon_t *) self->curWeapon;
        if ( !pWeapon || !SIDEKICK_CanUseWeapon( pWeapon ) )
        {
            // try to evade from fighting
            GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
            GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
            GOAL_ClearTasks( pCurrentGoal );
            AI_AddNewTaskAtFront( self, TASKTYPE_SIDEKICK_EVADE, self->enemy );
            return FALSE;
        }		
	}

	float fRange = AI_GetCurrentWeaponRange( self );
    if ( fRange <= 156.0f )
	{
        if ( SIDEKICK_UpdateMeleeAttackMovement( self ) == FALSE )
        {
            return FALSE;
        }
    }
	else
	{
        // ranged attack weapon
        if ( SIDEKICK_UpdateRangeAttackMovement( self ) == FALSE )
        {
            return FALSE;
        }
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
// NSS[1/20/00]:
// Name:		SIDEKICK_IsClearShot
// Description: Will determine if the Sidekick has a clear shot or not.
// Input: userEntity_t *self, userEntity_t *pEntity
// Output: TURE or FALSE
// Note:
// ----------------------------------------------------------------------------
int SIDEKICK_IsClearShot(userEntity_t *self, userEntity_t *pEntity, CVector &OrgStart, CVector &OrgEnd)
{
	CVector Delta_Origin,Destination;
	int Hit = 0;
	int single_trace = 0;
	float x =5;
	float z =5;
	

	// NSS[1/20/00]:Had to make this a little more modular for other purposes
	Delta_Origin.Set(OrgStart.x,OrgStart.y,OrgStart.z);
	Destination.Set(OrgEnd.x, OrgEnd.y, OrgEnd.z);
	
	weapon_t *pWeapon = (weapon_t *) self->curWeapon;

	int nCurrentEpisode = GetCurrentEpisode();
	
	if(pWeapon)
	{

		float fWeaponRange	= AI_GetCurrentWeaponRange( self );
		float fDistance		= VectorXYDistance(Delta_Origin,Destination);

		// NSS[1/20/00]:Make sure we can hit it first.
		if(fWeaponRange < fDistance)
		{
			return FALSE;
		}

		switch(nCurrentEpisode)
		{
			case 1:
			{
				//Reference
				//gstate->SetSize (ionBlast, -8, -8, -8, 8, 8, 8);
				if( _stricmp(pWeapon->name,"weapon_disruptor") == 0 )			
				{
					single_trace = 1;
				}
				else if(_stricmp(pWeapon->name,"weapon_ionblaster")== 0)
				{
					x = 16;
					z = 16;
				}
				break;
			}
			case 2:
			{
				//Reference
				//gstate->SetSize(discus,-5,-2,-5,5,2,5);
				if(_stricmp(pWeapon->name,"weapon_discus") == 0)
				{
					x = 8;
					z = 8;
				}
				else
				if(_stricmp(pWeapon->name,"weapon_venomous") == 0)
				{
					x = 16;
					z = 16;
				}
				else
				if(_stricmp(pWeapon->name,"weapon_trident") == 0)
				{
					x = 16;
					z = 16;
				}
				break;
			}
			case 3:
			{
				//Reference
				//gstate->SetSize(bolt,-4,-4,-4,4,4,4);
				if( _stricmp(pWeapon->name,"weapon_silverclaw") == 0 )
				{
					single_trace = 1;
				}
				else if(_stricmp(pWeapon->name,"weapon_bolter") == 0)
				{
					x = 16;
					z = 16;
				}

				break;
			}
			case 4:
			{
				if( _stricmp(pWeapon->name,"weapon_glock") == 0 )
				{
					single_trace = 1;
				}
				break;
			}
		}


		if(!single_trace)
		{
			Delta_Origin.x += x;
			Delta_Origin.z += z;
			tr = gstate->TraceLine_q2( Delta_Origin, Destination, self, MASK_MONSTERSOLID );
			if(tr.fraction < 1.0f && tr.ent != pEntity)
			{
				Hit++;
			}
			Delta_Origin.x -= x;
			Delta_Origin.z += z;
			tr = gstate->TraceLine_q2( Delta_Origin, Destination, self, MASK_MONSTERSOLID );
			if(tr.fraction < 1.0f && tr.ent != pEntity)
			{
				Hit++;
			}
			Delta_Origin.x += x;
			Delta_Origin.z -= z;
			tr = gstate->TraceLine_q2( Delta_Origin, Destination, self, MASK_MONSTERSOLID );
			if(tr.fraction < 1.0f && tr.ent != pEntity)
			{
				Hit++;
			}
			Delta_Origin.x -= x;
			Delta_Origin.z -= z;
			tr = gstate->TraceLine_q2( Delta_Origin, Destination, self, MASK_MONSTERSOLID );
			if(tr.fraction < 1.0f && tr.ent != pEntity)
			{
				Hit++;
			}
		}
		else
		{
			tr = gstate->TraceLine_q2( Delta_Origin, Destination, self, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_CLEAR|CONTENTS_DEADMONSTER );
			if(tr.fraction < 1.0f && tr.ent != pEntity)
			{
				Hit++;
			}	
		}

		if(!Hit)
		{
			return TRUE;
		}
	}
	return FALSE;
}





// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_IsOkToUseRangedWeapon
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsOkToUseRangeWeapon( userEntity_t *self )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

    userEntity_t *pEnemy = self->enemy;
    _ASSERTE( pEnemy );

    int bOkToUseRangeWeapon = FALSE;

//	float fDistance = VectorDistance( self->s.origin, pEnemy->s.origin );// SCG[1/23/00]: not used

    userInventory_t *pCurrentWeapon = self->curWeapon;
    _ASSERTE( pCurrentWeapon );

    int nCurrentEpisode = GetCurrentEpisode();
	switch ( nCurrentEpisode )
	{
		case 1:
		{
            if ( _stricmp(pCurrentWeapon->name, "weapon_ionblaster") == 0 )
			{
                bOkToUseRangeWeapon = TRUE;
            }
			else if( _stricmp(pCurrentWeapon->name, "weapon_shotcycler") == 0 )
			{
				bOkToUseRangeWeapon = TRUE;	
			}
			break;
		}	
		case 2:
		{
            bOkToUseRangeWeapon = TRUE;
            break;
		}
		case 3:
		{
			if ( (_stricmp(pCurrentWeapon->name, "weapon_bolter") == 0 || 
			      _stricmp(pCurrentWeapon->name, "weapon_ballista") == 0))
			{
                bOkToUseRangeWeapon = TRUE;
            }
            break;
		}
		case 4:
		{
            bOkToUseRangeWeapon = TRUE;
            break;
		}
		default:
        {
            break;
        }
	}

    return bOkToUseRangeWeapon;

}
// ----------------------------------------------------------------------------
// NSS[12/15/99]:
// Name:		SIDEKICK_SelectBestAttackAnimation
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SelectBestAttackAnimation(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	weapon_t *pWeapon = (weapon_t *) self->curWeapon;
	AI_Dprintf("SideKickAFrame:%d\n",self->s.frame);
	if(hook)
	{
		if(_stricmp( pWeapon->name, "weapon_discus" ) == 0 && self->velocity.Length() < 10.0f)
		{
			if ( (!hook->cur_sequence || !strstr( hook->cur_sequence->animation_name, "atak" ) ||  AI_IsEndAnimation( self )) && pWeapon->ammo->count > 0 || AI_IsJustFired(hook ))
			{
				frameData_t *pSequence = FRAMES_GetSequence( self, "atak" );
    			AI_ForceSequence( self, pSequence);
			}
		}
		else
		{
			SIDEKICK_SelectAnimationWhileAttacking( self );
		}
	}
}


// ----------------------------------------------------------------------------
// NSS[12/16/99]:
// Name:		SIDEKICK_RipgunThink
// Description: A solution to the chaingun problem and sidekicks...
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_RipgunThink(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	weapon_t *pWeapon = (weapon_t *) self->curWeapon;
    if ( pWeapon )
    {
	

		if(_stricmp( pWeapon->name, "weapon_ripgun" ) == 0 || _stricmp( pWeapon->name, "weapon_shotcycler" ) == 0)
		{
			if(hook)
			{
				self->nextthink = gstate->time + 0.10f;
		
				if(hook->attack_finished < gstate->time)
				{
					self->think = SIDEKICK_Think;
				}
				else
				{
					ai_fire_curWeapon( self );
				}
				return;
			}
		}
	}
	self->think = SIDEKICK_Think;	
	self->nextthink = gstate->time + 0.10f;
}




// ----------------------------------------------------------------------------
// NSS[12/15/99]:
// Name:		SIDEKICK_UseCurrentWeapon
// Description: This function actually uses the current weapon the sidekick has chosen
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_UseCurrentWeapon( userEntity_t *self )
{
	_ASSERTE( self );
	_ASSERTE( self->enemy );
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *pEnemy = self->enemy;

	if( self->enemy == NULL || hook == NULL )
	{
		return;
	}

	if (!SIDEKICK_IsFacingEnemy( self, pEnemy,-1.0f, 90.0f, 90.0f ) )
	{
		AI_FaceTowardPoint(self,pEnemy->s.origin);
	//	return;
	}
	

	weapon_t *pWeapon = (weapon_t *) self->curWeapon;
    if ( pWeapon )
    {
        if ( SIDEKICK_CanUseWeapon( pWeapon ) )
        {
            float fWeaponRange = AI_GetCurrentWeaponRange( self );
			float fDistance = VectorDistance( self->s.origin, pEnemy->s.origin );

			// NSS[1/17/00]:Call the frame functions if required.
			if ( hook->fxFrameFunc )
			{
				hook->fxFrameFunc( self );
			}

            if ( fWeaponRange <= 124.0f)
            {
		        if ( fDistance <= fWeaponRange )
		        {
                    if ( !hook->cur_sequence || 
                         !strstr( hook->cur_sequence->animation_name, "atak" ) || 
                         AI_IsEndAnimation( self ) )
                    {
						frameData_t *pSequence = FRAMES_GetSequence( self, "atak" );
    	                AI_ForceSequence( self, pSequence );
                    }
                    else
                    {
	                    
						
						if ( SIDEKICK_IsClearShot( self, pEnemy , self->s.origin, pEnemy->s.origin ) && 
                             (AI_IsReadyToAttack1( self ) || AI_IsReadyToAttack2( self )) )
		                {
			                CVector vectorTowardEnemy = pEnemy->s.origin - self->s.origin;

			                vectorTowardEnemy.Normalize();
			                
			                CVector angleTowardEnemy;
			                VectorToAngles(vectorTowardEnemy, angleTowardEnemy);
			                
			                self->client->v_angle = self->s.angles;
			                self->client->v_angle.pitch = angleTowardEnemy.pitch;
			                self->client->v_angle.roll = angleTowardEnemy.roll;

			                hook->weapon_fired = self->curWeapon;
                           	// NSS[12/9/99]:So far not many good sounds for Mikiko's battle cries... 
							if(!Check_Que(self,STARTING_ATTACK,10.0f))
							{
								SideKick_TalkAmbient(self, STARTING_ATTACK,1);
								SIDEKICK_SendMessage(self, sCom_Speak,STARTING_ATTACK,gstate->time+10.0f,0.0f,self,COM_Read);
							}

							// NSS[12/20/99]:If we made it this far we should be pretty still.
							AI_ZeroVelocity( self );

							ai_fire_curWeapon( self );

                            AI_SetJustFired( hook );
							
                        }
						else if(AI_IsEndAnimation(self))
						{
							AI_RemoveCurrentTask(self,FALSE);
						}
                    }
                }
            }
            else
            {
	            // using ranged weapon
                if ( gstate->time >= hook->attack_finished )
	            {
           		    if ( fDistance <= fWeaponRange  )
				    {
						if ( SIDEKICK_IsClearShot( self, pEnemy, self->s.origin,pEnemy->s.origin ) )
						{                        
							if ( (AI_IsReadyToAttack1( self ) || AI_IsReadyToAttack2( self )) && pWeapon->ammo->count > 0 )
							{
								CVector vectorTowardEnemy = pEnemy->s.origin - self->s.origin;
								vectorTowardEnemy.Normalize();
								
								CVector angleTowardEnemy;
								VectorToAngles(vectorTowardEnemy, angleTowardEnemy);
								
								self->client->v_angle       = angleTowardEnemy;
								self->client->v_angle.pitch = angleTowardEnemy.pitch;
								self->client->v_angle.roll  = angleTowardEnemy.roll;

								hook->weapon_fired = self->curWeapon;
								
								if (!Check_Que(self,STARTING_ATTACK,10.0f))
								{
									SideKick_TalkAmbient(self, STARTING_ATTACK,1);
									SIDEKICK_SendMessage(self, sCom_Speak,STARTING_ATTACK,gstate->time+10.0f,0.0f,self,COM_Read);							
								}                    
								ai_fire_curWeapon( self );

								// NSS[12/20/99]:If we made it this far we should be pretty still.
								AI_ZeroVelocity( self );
	
								if(_stricmp( pWeapon->name, "weapon_ripgun" ) == 0 )
								{
									self->think = SIDEKICK_RipgunThink;
									hook->attack_finished = gstate->time + 0.50;
									self->nextthink = gstate->time + 0.10f;
								}
								else if(_stricmp( pWeapon->name, "weapon_shotcycler" ) == 0 )
								{
									self->think = SIDEKICK_RipgunThink;
									hook->attack_finished = gstate->time + 0.15;
									self->nextthink = gstate->time + 0.10f;
								}
								else
								{
									hook->attack_finished = gstate->time + SIDEKICK_ComputeAttackAnimationTime( self );
									AI_RemoveCurrentTask(self,FALSE);
								}

								AI_SetJustFired( hook );
							}
							
							if(pWeapon->ammo->count > 0)
							{
								SIDEKICK_SelectBestAttackAnimation(self);
							}
						}
					}
                }
            }
        }
    }
}



// ----------------------------------------------------------------------------
// NSS[1/17/00]:
// Name:		SIDEKICK_FindGoodShootingSpot
// Description: Will attempt to find a good shooting spot..if so it will move there.
// Input:userEntity_t *self, userEntity_t *enemy, CVector *Position
// Output: TRUE --> Spot found, FALSE---> Nope... just can't find one.
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_FindGoodShootingSpot(userEntity_t *self, userEntity_t *enemy, CVector *Position)
{
	CVector Origin, Delta_Origin,Enemy_Origin, Angles, Dir, New_Origin;
	CVector mins,maxs;
	playerHook_t *hook = AI_GetPlayerHook(self);
	
	float	Delta_Yaw;
	int		Done = 1;
	float	Range = 200.0f;
	
	if(!hook)
	{
		return FALSE;
	}

	
	// NSS[1/20/00]:Determine if it is melee or ranged
	weapon_t *pWeapon = (weapon_t *) self->curWeapon;
	if(pWeapon)
	{
		float fWeaponRange	= AI_GetCurrentWeaponRange( self );
		if(fWeaponRange < Range)
		{
			Range = fWeaponRange*0.85;
		}
	}
	
	// NSS[1/20/00]:Check for Melee weapons
	if(Range < 200)
	{
		Origin = enemy->s.origin;
	}
	else
	{
		// NSS[1/20/00]:Base it off of the owner if he/she is alive.
		if(AI_IsAlive(hook->owner))
		{
			Origin.Set(hook->owner->s.origin.x,hook->owner->s.origin.y,hook->owner->s.origin.z);
		}
		else
		{
			Origin = self->s.origin;
		}
	}

	Enemy_Origin.Set(enemy->s.origin.x,enemy->s.origin.y,enemy->s.origin.z);

	if(hook->type == TYPE_MIKIKO)
	{
		Delta_Yaw = 25.0f;
	}
	else
	{
		Delta_Yaw = 335.0f;		
	}

	
	while(Done)
	{
		Angles.yaw = Delta_Yaw;
		Angles.pitch = Angles.roll = 0.0f;

		Angles.AngleToForwardVector(Dir);

		Delta_Origin = Origin + (Range * Dir);

		Delta_Origin.z +=16.0f;

		if(SIDEKICK_IsClearShot(self,enemy,Delta_Origin,Enemy_Origin) && AI_FindPathToPoint( self, Delta_Origin ))
		{
			Position->x = Delta_Origin.x;
			Position->y = Delta_Origin.y;
			Position->z = Delta_Origin.z;
			return TRUE;
		}
		
		if(hook->type == TYPE_MIKIKO)
		{
			if(Delta_Yaw == 25.0f || Delta_Yaw == 205)
				Delta_Yaw += 130.0f;
			else
				Delta_Yaw += 50.0f;
			if(Delta_Yaw > 360)
			{
				Done = 0;
			}
		}
		else// NSS[1/19/00]:Superfly
		{
			if(Delta_Yaw == 335.0f || Delta_Yaw == 205)
				Delta_Yaw -= 130.0f;
			else
				Delta_Yaw -= 50.0f;			
			if(Delta_Yaw < 0)
			{
				Done = 0;
			}
		}

		
	}
	if(hook->type == TYPE_MIKIKO)
	{
		Delta_Yaw = 25.0f;
	}
	else// NSS[1/19/00]:Superfly
	{
		Delta_Yaw = 335.0f;		
	}	
	return FALSE;
}


// ----------------------------------------------------------------------------
// NSS[1/17/00]:
// Name:		SIDEKICK_IsOwnerGoingThisWay
// Description:Determines if the owner is going in the general direction of the 
// monster we are targeting
// Input:userEntity_t *self
// Output:TRUE FALSE
// Note:
// ----------------------------------------------------------------------------
int SIDEKICK_IsOwnerGoingThisWay(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	userEntity_t *owner = hook->owner;
	// NSS[1/17/00]:Store this for later.
	float FOV = hook->fov;

	if(owner)
	{
		hook->fov = 90.0f;
		if(AI_IsInFOV(self,self->enemy) && self->velocity.Length() > 20.0f)
		{
			hook->fov = FOV;
			return TRUE;
		}
		
	}
	return FALSE;
}

// ----------------------------------------------------------------------------
// NSS[12/13/99]:
// Name:		SIDEKICK_Attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_Attack( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook	= AI_GetPlayerHook( self );
	CVector Dir, Angles;
	weapon_t *pWeapon = (weapon_t *) self->curWeapon;


	if (!self->enemy ||  !AI_IsAlive( self->enemy ))
	{
		self->enemy = NULL;
		// killed the enemy, no longer in attack mode
		SIDEKICK_ClearAttackMode( self );
		// NSS[12/13/99]: Now this is a task... finally.
		AI_RemoveCurrentTask(self,FALSE);
		return;
	}

	Dir = self->enemy->s.origin - self->s.origin;
	Dir.Normalize();

	VectorToAngles(Dir,Angles);

	self->s.angles.yaw = Angles.yaw;	
	
	// NSS[12/13/99]:Moved this out of the think process
	SIDEKICK_ChooseBestWeapon( self );

	if( SIDEKICK_IsOkToFight( self ) )
	{
		if(hook)
		{
			userEntity_t *owner = hook->owner;
			
			if(!SIDEKICK_IsClearShot( self, self->enemy ,self->s.origin,self->enemy->s.origin))
			{
				if(SIDEKICK_FindGoodShootingSpot(self,self->enemy, &Dir))
				{
					AI_SetStateRunning(hook);
					float fDistance = VectorXYDistance(self->s.origin,Dir);
					float fZDistance = VectorZDistance(self->s.origin,Dir);
					if(fDistance < 48.0f && AI_IsOkToMoveStraight(self,Dir,fDistance,fZDistance))
					{
						if(!AI_MoveTowardPoint( self, self->enemy->s.origin, FALSE ))
						{
							AI_AddNewTaskAtFront(self,TASKTYPE_MOVETOLOCATION,Dir);		
						}
					}					
					else
					{
						AI_AddNewTaskAtFront(self,TASKTYPE_MOVETOLOCATION,Dir);
					}
					return;
				}
				else
				{
					// NSS[1/17/00]:If we can't find a place to shoot at the monster don't do a damned thing.
					// let the player do it...or move to help the sidekick adjust him/herself.
					if(hook->owner && !SIDEKICK_HasTaskInQue(self,TASKTYPE_SIDEKICK_STOP))
						AI_AddNewTaskAtFront(self,TASKTYPE_SIDEKICK_STOP, 2.0f);
					else
					{
						SIDEKICK_ClearAttackMode(self);
					}
				}
				return;
			}
			
			// NSS[1/20/00]:Always running when there is fighting going on.
			AI_SetStateRunning(hook);

			
			//if ( SIDEKICK_UpdateAttackMovement( self ) == FALSE)
			//{
			//	return;
			//}
		
			// NSS[12/20/99]:Actually do something with the weapon now.
			SIDEKICK_UseCurrentWeapon( self );
		}
		else
		{
			AI_Dprintf("Problemo's no hook and sidekick is in attack mode!!!!\n");
		}
	}
	else
	{
		self->enemy = NULL;
		AI_RemoveCurrentTask(self,FALSE);
		//AI_Dprintf("Problemo's no enemy and sidekick is in attack mode!!!!\n");
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_BeginAttack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_BeginAttack( userEntity_t *self )
{
	_ASSERTE( self );
	_ASSERTE( self->enemy );
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_SetStateAttacking( hook );

    // clear all tasks when beginning to attack
    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
    GOAL_ClearTasks( pCurrentGoal );

#ifdef USE_STRAFE
	hook->strafe_dir = rand() % (STRAFE_DOWNRIGHT+1);
#endif USE_STRAFE

    // NSS[12/5/99]:Code to make sidekicks scream that battle cry they scream so well.
	if(!Check_Que(self,STARTING_ATTACK,15.0f) && rnd()< 0.35f)
	{
  		if(SideKick_TalkAmbient(self,STARTING_ATTACK,0))
			SIDEKICK_SendMessage(self, sCom_Speak,STARTING_ATTACK,gstate->time+15.0f,0.0f,self,COM_Read);
	}
	
	SIDEKICK_ChooseBestWeapon( self );
    SIDEKICK_SetAttackSequence( self );
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_Start
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_Start( userEntity_t *self, int nType )
{
	_ASSERTE( self );

	// cek[1-19-00] prevent sidekick spawning in coop mode
	if (coop->value || deathmatch->value)
	{
		gstate->RemoveEntity(self);
		return;
	}

	playerHook_t *hook = AI_InitMonster( self, nType );
	self->s.flags	&= ~SFL_TARGETABLE;
	_ASSERTE( hook );

	// SCG[10/25/99]: called specifically since only sidekicks and real clients 
	// SCG[10/25/99]: should be referencing the client structure..  This 
	// SCG[10/25/99]: takes place of the code Nelno wrote that alloc'd a client
	// SCG[10/25/99]: for every entity (monsters, worldspawn, etc...)
	self->client = ( gclient_t * ) gstate->X_Malloc( sizeof( gclient_t ), MEM_TAG_TEMP );
	memset( self->client, 0, sizeof( gclient_t ) );

	gstate->LinkEntity( self );

	AI_DetermineMovingEnvironment( self );

	SIDEKICK_Init( self, nType );
	SIDEKICK_InitWeapons( self );

	char szAnimation[16];
    AI_SelectAmbientAnimation( self, szAnimation );
	frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
	AI_StartSequence(self, pSequence, FRAME_LOOP);

	gstate->LinkEntity( self );

    int nNumSidekicks = AIINFO_GetNumSidekicks();
    if ( nNumSidekicks == 1 )
    {
        SIDEKICK_ReadAmbientInfo( self, "SidekickAmbient.csv" );
    }
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_Spawn
// Description:
// Input:
// Output:
// Note:
//          do not spawn more than one kind of sidekick at a time
//
// ----------------------------------------------------------------------------
userEntity_t *SIDEKICK_Spawn( userEntity_t *self, int nType )
{
	// cek[1-19-00] prevent sidekick spawning in coop mode
	if (coop->value)
		return NULL;

    if ( nType == TYPE_SUPERFLY )
    {
	    userEntity_t *pSuperFly = AIINFO_GetSuperfly();
        if ( pSuperFly )
        {
            com->Warning( "Superfly already exists!" );
            return NULL;
        }
    }
	else
	if (nType == TYPE_MIKIKOFLY)
	{
	    userEntity_t *pSuperFly = AIINFO_GetMikikofly();
        if ( pSuperFly )
        {
            com->Warning( "Mikikofly already exists!" );
            return NULL;
        }	
	}
    else
    if ( nType == TYPE_MIKIKO )
    {
        userEntity_t *pMikiko = AIINFO_GetMikiko();
        if ( pMikiko )
        {
            com->Warning( "Mikiko already exists!" );
            return NULL;
        }
    }
    CVector spawnPoint;
	if ( !SPAWN_FindNearSpawnPoint( self, spawnPoint ) )
	{
		com->Warning( "Did not find an appropriate spawn point.\n");
		return NULL;
	}

	userEntity_t *pSidekick = gstate->SpawnEntity();
	pSidekick->s.origin = spawnPoint;
	SIDEKICK_Start( pSidekick, nType );

	return pSidekick;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_Spawn
// Description:
// Input:
// Output:
// Note:
//          do not spawn more than one kind of sidekick at a time
//
// ----------------------------------------------------------------------------
userEntity_t *SIDEKICK_Spawn( int nType, const CVector &origin, const CVector &angle )
{
	// cek[1-19-00] prevent sidekick spawning in coop mode
	if (coop->value)
		return NULL;

    if ( nType == TYPE_SUPERFLY )
    {
	    userEntity_t *pSuperFly = AIINFO_GetSuperfly();
        if ( pSuperFly )
        {
            com->Warning( "Superfly already exists!" );
            return NULL;
        }
    }
	else
	if (nType == TYPE_MIKIKOFLY)
	{
	    userEntity_t *pSuperFly = AIINFO_GetMikikofly();
        if ( pSuperFly )
        {
            com->Warning( "Mikikofly already exists!" );
            return NULL;
        }	
	}
    else
    if ( nType == TYPE_MIKIKO )
    {
        userEntity_t *pMikiko = AIINFO_GetMikiko();
        if ( pMikiko )
        {
            com->Warning( "Mikiko already exists!" );
            return NULL;
        }
    }

	userEntity_t *pSidekick = gstate->SpawnEntity();
	pSidekick->s.origin     = origin;
	pSidekick->s.angles     = angle;
	pSidekick->ideal_ang    = angle;

	SIDEKICK_Start( pSidekick, nType );

	return pSidekick;
}

// ----------------------------------------------------------------------------
//							GLOBAL functions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_Alert
// Description:
// Input:
// Output:
// Note:
//	called by damage function when a client attacks a monster or bot
//
//	FIXME:	make this a general function that alerts all entities on the
//			same team.
//
// ----------------------------------------------------------------------------
void SIDEKICK_Alert( userEntity_t *owner, userEntity_t *target )
{
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_PlaySound
// Description:
//      Stub routine for difference player class sounds until the
//      faster, cheaper, cleaner method is ready to go
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_PlaySound( userEntity_t *self, const char *soundfile )
{
	char buf[256];

	_ASSERTE( self );

	playerHook_t *hook = AI_GetPlayerHook( self );

	_ASSERTE( hook );

	if ( AI_IsMikiko(hook) )  // mikiko sound?
	{
//		sprintf(buf,"mikiko/%s",soundfile);
		Com_sprintf(buf,sizeof(buf),"mikiko/%s",soundfile);
	}
	else if ( AI_IsSuperfly(hook) ) // superfly sound?
	{
//		sprintf(buf,"superfly/%s",soundfile);
		Com_sprintf(buf,sizeof(buf),"superfly/%s",soundfile);
	}
	else
//		sprintf(buf,"hiro/%s",soundfile);
		Com_sprintf(buf,sizeof(buf),"hiro/%s",soundfile);

	gstate->StartEntitySound(self, CHAN_BODY, gstate->SoundIndex(buf), 1.0f, ATTN_NORM_MIN,ATTN_NORM_MAX);
}

// ----------------------------------------------------------------------------
// NSS[12/10/99]:
// Name:		SIDEKICK_SelectWeapon
// Description: Generic Weapon selection code based off of a simple rating system for monsters
// Input:userEntity_t *self, userEntity_t *pEnemy, weapon_t **aWeapon, ammo_t **aAmmo
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
userInventory_t *SIDEKICK_SelectWeapon( userEntity_t *self, userEntity_t *pEnemy, weapon_t **aWeapon, ammo_t **aAmmo )
{
    _ASSERTE( aWeapon );
    _ASSERTE( aAmmo );

    weapon_t *pWeapon = NULL;
	playerHook_t *hook = AI_GetPlayerHook(self);
	// NSS[2/14/00]:MikikoFly does not use weapons!
	if(hook->type == TYPE_MIKIKOFLY)
	{
		return NULL;
	}
	int nFirstChoice,nSecondChoice,nThirdChoice;

	if ( pEnemy && AI_IsAlive(pEnemy) )
    {        
		playerHook_t *pEnemyHook = AI_GetPlayerHook( pEnemy );
		if(pEnemyHook)
		{
			nFirstChoice	= aSidekickCombatData[pEnemyHook->type].nFirstWeaponChoice;
			nSecondChoice	= aSidekickCombatData[pEnemyHook->type].nSecondWeaponChoice;
			nThirdChoice	= aSidekickCombatData[pEnemyHook->type].nThirdWeaponChoice;
		}
		else// NSS[1/20/00]:Not a valid monster get standard selection
		{
			nFirstChoice	= 0;
			nSecondChoice	= 1;
			nThirdChoice	= 2;		
		}
	}
	else // NSS[1/20/00]:No enemy, get the standard selection
	{
		nFirstChoice	= 0;
		nSecondChoice	= 1;
		nThirdChoice	= 2;
	}
	// NSS[3/1/00]:Make sure this is a valid number
	if(nFirstChoice > 2)
		nFirstChoice = 0;
	if(nSecondChoice > 2)
		nSecondChoice = 1;
	if(nThirdChoice > 2)
		nThirdChoice =2;


	weapon_t *pWeapon1	= aWeapon[nFirstChoice];
	weapon_t *pWeapon2	= aWeapon[nSecondChoice];
	weapon_t *pWeapon3	= aWeapon[nThirdChoice];

	if ( !pWeapon1 || !pWeapon1->ammo || !SIDEKICK_CanUseWeapon( pWeapon1 ) || pWeapon2 || pWeapon3)
    {
		if ( !pWeapon2 || !pWeapon2->ammo || !SIDEKICK_CanUseWeapon( pWeapon2 ) || pWeapon3)
		{
			if(!pWeapon3 || !pWeapon3->ammo || !SIDEKICK_CanUseWeapon( pWeapon3))
			{
				if(!pWeapon2 || !pWeapon2->ammo || !SIDEKICK_CanUseWeapon( pWeapon2))
				{
					if(!pWeapon1 || !pWeapon1->ammo || !SIDEKICK_CanUseWeapon(pWeapon1))
					{
						pWeapon = NULL;
					}
					else
					{
						pWeapon = pWeapon1;
					}
				}
				else
				{
					pWeapon = pWeapon2;
				}
			}
			else
			{
				pWeapon = pWeapon3;
			}
		}
		else
		{
			pWeapon= pWeapon2;
		}
    }
	else
	{
		pWeapon = pWeapon1;
	}

	return (userInventory_t*)pWeapon;
}

// ----------------------------------------------------------------------------
// NSS[12/10/99]:
// Name:		SIDEKICK_OwnerIsTooCloseToEnemy
// Description: For weapon selection...don't want to select a weapon that is explosive
// if the owner is right next to the monster.
// Input: userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------

int SIDEKICK_OwnerIsTooCloseToEnemy(userEntity_t *self)
{
	float fDistance;
	playerHook_t *hook = AI_GetPlayerHook(self);

	if(hook)
	{
		if(hook->owner && self->enemy)
		{
			fDistance = VectorDistance(self->enemy->s.origin,hook->owner->s.origin);
			if(fDistance < 156.0f)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

// ----------------------------------------------------------------------------
// NSS[12/10/99]:
// Name:		SIDEKICK_IsTooCloseToOwner
// Description: For weapon selection...don't want to select a weapon that is explosive
// if the owner is right next to the monster.
// Input: userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsTooCloseToOwner(userEntity_t *self)
{
	float fDistance;
	playerHook_t *hook = AI_GetPlayerHook(self);

	if(hook)
	{
		if(hook->owner)
		{
			fDistance = VectorDistance(hook->owner->s.origin,self->s.origin);
			if(fDistance < 96.0f)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}


// ----------------------------------------------------------------------------
// NSS[12/10/99]:
// Name:		SIDEKICK_IsTooCloseToEnemy
// Description: For weapon selection...don't want to select a weapon that is explosive
// if the owner is right next to the monster.
// Input: userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsTooCloseToEnemy(userEntity_t *self)
{
	float fDistance;
	if(self->enemy)
	{
		fDistance = VectorDistance(self->enemy->s.origin,self->s.origin);
		if(fDistance < 156.0f)
		{
			return TRUE;
		}
	}
	return FALSE;
}


// ----------------------------------------------------------------------------
// NSS[12/10/99]:
// Name:		SIDEKICK_IsTooFarFromEnemy
// Description: For weapon selection...don't want to select a weapon that spreads too much at a distance
// Input: userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsTooFarFromEnemy(userEntity_t *self)
{
	float fDistance;
	playerHook_t *hook = AI_GetPlayerHook(self);

	if(hook)
	{
		if(hook->owner && self->enemy)
		{
			fDistance = VectorDistance(self->enemy->s.origin,self->s.origin);
			if(fDistance > 400.0f)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

// ----------------------------------------------------------------------------

userInventory_t *SIDEKICK_ChooseBestWeaponForE1( userEntity_t *self, userEntity_t *pEnemy /* = NULL */ )
{
	weapon_t *aWeapon[E1_NUMWEAPONS];
    ammo_t *aAmmo[E1_NUMWEAPONS];

	
	//	clear the ammo and weapons arrays
	memset(aWeapon, 0x00, E1_NUMWEAPONS * sizeof(weapon_t*));
	memset(aAmmo, 0x00, E1_NUMWEAPONS * sizeof(ammo_t*));

	///////////////////////////////////////////////////////////
	//	determine which weapons we have
	///////////////////////////////////////////////////////////

	//	search through the whole inventory list in one pass
	_ASSERTE( self->inventory );
	userInventory_t *inv = gstate->InventoryFirstItem( self->inventory );
	while ( inv )
	{
		if( inv->flags & ITF_WEAPON )
		{
			if ( inv->name )
			{
				//	look for weapons
				if ( _stricmp(inv->name, "weapon_disruptor") == 0 )
				{
					aWeapon[E1_DISRUPTOR] = (weapon_t *)inv;

					// disruptor glove does not need ammo
					if ( aWeapon[E1_DISRUPTOR]->ammo )
					{
						aWeapon[E1_DISRUPTOR]->ammo->count = 1;
					}
				}
				else 
				if ( _stricmp(inv->name, "weapon_ionblaster") == 0 )
				{
					aWeapon[E1_IONBLASTER] = (weapon_t *)inv;
				}
				else 
				if ( _stricmp(inv->name, "weapon_shotcycler") == 0 && !SIDEKICK_IsTooCloseToOwner(self) && !SIDEKICK_IsTooFarFromEnemy(self))
				{
					aWeapon[E1_SHOTCYCLER] = (weapon_t *)inv;
				}
				else
				if ( _stricmp(inv->name, "ammo_disruptor") == 0 )
				{
					aAmmo[E1_DISRUPTOR] = (ammo_t*)inv;
				}
				else
				if ( _stricmp(inv->name, "ammo_ionpack") == 0 )
				{
					aAmmo[E1_IONBLASTER] = (ammo_t*)inv;
				}
				else
				if ( _stricmp(inv->name, "ammo_shells") == 0 )
				{
					aAmmo[E1_SHOTCYCLER] = (ammo_t*)inv;
				}
			}
		}		
		inv = gstate->InventoryNextItem( self->inventory );
	}

    if ( aAmmo[E1_DISRUPTOR] )
    {
        
    }

    userInventory_t *pWeapon = SIDEKICK_SelectWeapon( self, pEnemy, aWeapon, aAmmo );

    return pWeapon;
}

// ----------------------------------------------------------------------------

userInventory_t *SIDEKICK_ChooseBestWeaponForE2( userEntity_t *self, userEntity_t *pEnemy /* = NULL */ )
{
	weapon_t *aWeapon[E1_NUMWEAPONS];
    ammo_t *aAmmo[E1_NUMWEAPONS];

	//	clear the ammo and weapons arrays
	memset(aWeapon, 0x00, E1_NUMWEAPONS * sizeof(weapon_t*));
	memset(aAmmo, 0x00, E1_NUMWEAPONS * sizeof(ammo_t*));

	///////////////////////////////////////////////////////////
	//	determine which weapons we have
	///////////////////////////////////////////////////////////

	//	search through the whole inventory list in one pass
	_ASSERTE( self->inventory );
	userInventory_t *inv = gstate->InventoryFirstItem( self->inventory );
	while ( inv )
	{	
		// SCG[11/5/99]: save some cycles, lets only do this for weapons..
		if( inv->flags & ITF_WEAPON )
		{
			if ( inv->name )
			{
				//	look for weapons
				if ( _stricmp(inv->name, "weapon_discus") == 0 )
				{
					aWeapon[E2_DISCUS] = (weapon_t *)inv;
				}
				else 
				if ( _stricmp(inv->name, "weapon_venomous") == 0 && !SIDEKICK_OwnerIsTooCloseToEnemy(self))
				{
					aWeapon[E2_VENOMOUS] = (weapon_t *)inv;
				}
				else 
				if ( _stricmp(inv->name, "weapon_trident") == 0 && !SIDEKICK_OwnerIsTooCloseToEnemy(self))
				{
					aWeapon[E2_TRIDENT] = (weapon_t *)inv;
				}
				else
				if ( _stricmp(inv->name, "ammo_discus" ) == 0 )
				{
					aAmmo[E2_DISCUS] = (ammo_t*)inv;
				}
				else
				if ( _stricmp(inv->name, "ammo_venomous") == 0 )
				{
					aAmmo[E2_VENOMOUS] = (ammo_t*)inv;
				}
				else
				if (_stricmp(inv->name, "ammo_trident") == 0)
				{
					aAmmo[E2_TRIDENT] = (ammo_t*)inv;
				}
			}
		}
		
		inv = gstate->InventoryNextItem( self->inventory );
	}

    userInventory_t *pWeapon = SIDEKICK_SelectWeapon( self, pEnemy, aWeapon, aAmmo );

    return pWeapon;
}

// ----------------------------------------------------------------------------

userInventory_t *SIDEKICK_ChooseBestWeaponForE3( userEntity_t *self, userEntity_t *pEnemy /* = NULL */ )
{
	weapon_t *aWeapon[E3_NUMWEAPONS];
    ammo_t *aAmmo[E3_NUMWEAPONS];

	//	clear the ammo and weapons arrays
	memset(aWeapon, 0x00, E3_NUMWEAPONS * sizeof(weapon_t*));
	memset(aAmmo, 0x00, E1_NUMWEAPONS * sizeof(ammo_t*));

	///////////////////////////////////////////////////////////
	//	determine which weapons we have
	///////////////////////////////////////////////////////////

	//	search through the whole inventory list in one pass
	_ASSERTE( self->inventory );
	userInventory_t *inv = gstate->InventoryFirstItem( self->inventory );
	while ( inv )
	{
		if( inv->flags & ITF_WEAPON )
		{
			if ( inv->name )
			{
				//	look for weapons
				if ( _stricmp(inv->name, "weapon_silverclaw") == 0 )
				{
					aWeapon[E3_SILVERCLAW] = (weapon_t *)inv;

					// silverclaw does not need ammo
					if ( aWeapon[E3_SILVERCLAW]->ammo )
					{
						aWeapon[E3_SILVERCLAW]->ammo->count = 1;
					}
				}
				else 
				if ( _stricmp(inv->name, "weapon_bolter") == 0 )
				{
					aWeapon[E3_BOLTER] = (weapon_t *)inv;
				}
				else // NSS[12/20/99]:Added distance check for both the owner and self to make sure we don't bust ourself up.
				if ( _stricmp(inv->name, "weapon_ballista") == 0 && !SIDEKICK_OwnerIsTooCloseToEnemy(self)&& !SIDEKICK_IsTooCloseToEnemy(self))
				{
					aWeapon[E3_BALLISTA] = (weapon_t *)inv;
				}
				else
				if ( _stricmp(inv->name, "ammo_bolts") == 0 )
				{
					aAmmo[E3_BOLTER] = (ammo_t*)inv;
				}
				else
				if ( _stricmp(inv->name, "ammo_ballista") == 0 )
				{
					aAmmo[E3_BALLISTA] = (ammo_t*)inv;
				}
			}
		}		
		inv = gstate->InventoryNextItem( self->inventory );
	}

    userInventory_t *pWeapon = SIDEKICK_SelectWeapon( self, pEnemy, aWeapon, aAmmo );

    return pWeapon;
}

// ----------------------------------------------------------------------------

userInventory_t *SIDEKICK_ChooseBestWeaponForE4( userEntity_t *self, userEntity_t *pEnemy /* = NULL */ )
{
	weapon_t *aWeapon[E4_NUMWEAPONS];
    ammo_t *aAmmo[E4_NUMWEAPONS];

	//	clear the ammo and weapons arrays
	memset(aWeapon, 0x00, E4_NUMWEAPONS * sizeof(weapon_t*));
	memset(aAmmo, 0x00, E1_NUMWEAPONS * sizeof(ammo_t*));

	///////////////////////////////////////////////////////////
	//	determine which weapons we have
	///////////////////////////////////////////////////////////

	//	search through the whole inventory list in one pass
	_ASSERTE( self->inventory );
	userInventory_t *inv = gstate->InventoryFirstItem( self->inventory );
	while ( inv )
	{
		if( inv->flags & ITF_WEAPON )
		{
			if ( inv->name )
			{
				//	look for weapons
				if ( _stricmp(inv->name, "weapon_glock") == 0 )
				{
					aWeapon[E4_GLOCK] = (weapon_t *)inv;
				}
				else 
				if ( _stricmp(inv->name, "weapon_ripgun") == 0 )
				{
					aWeapon[E4_RIPGUN] = (weapon_t *)inv;
				}
				else 
				if ( _stricmp(inv->name, "weapon_slugger") == 0 && !SIDEKICK_IsTooCloseToOwner(self) && !SIDEKICK_IsTooFarFromEnemy(self))
				{
					aWeapon[E4_SLUGGER] = (weapon_t *)inv;
				}
				else
				if ( _stricmp(inv->name, "ammo_bullets") == 0 )
				{
					aAmmo[E4_GLOCK] = (ammo_t*)inv;
				}
				else
				if ( _stricmp(inv->name, "ammo_ripgun") == 0 )
				{
					aAmmo[E4_RIPGUN] = (ammo_t*)inv;
				}
				else
				if ( _stricmp(inv->name, "ammo_slugger") == 0 )
				{
					aAmmo[E4_SLUGGER] = (ammo_t*)inv;
				}
			}
		}
		
		inv = gstate->InventoryNextItem( self->inventory );
	}

    userInventory_t *pWeapon = SIDEKICK_SelectWeapon( self, pEnemy, aWeapon, aAmmo );

    return pWeapon;
}

// ----------------------------------------------------------------------------

void SIDEKICK_ChooseBestWeapon( userEntity_t *self )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

    userEntity_t *pEnemy = self->enemy;

	userInventory_t *pCurrentWeapon = NULL;
    int nCurrentEpisode = GetCurrentEpisode();
	switch ( nCurrentEpisode )
	{
		case 1:
		{
			pCurrentWeapon = SIDEKICK_ChooseBestWeaponForE1( self, pEnemy );
			break;
		}	
		case 2:
		{
			pCurrentWeapon = SIDEKICK_ChooseBestWeaponForE2( self, pEnemy );
            break;
		}
		case 3:
		{
			pCurrentWeapon = SIDEKICK_ChooseBestWeaponForE3( self, pEnemy );
            break;
		}
		case 4:
		{
			pCurrentWeapon = SIDEKICK_ChooseBestWeaponForE4( self, pEnemy );
            break;
		}
		default:
        {
			pCurrentWeapon = NULL;
            break;
        }
	}

    if ( pCurrentWeapon )
    {
		if( ( self->curWeapon != pCurrentWeapon ) || ( self->winfo == NULL ) )
		{
			ai_weapon_t	*pAIWeapon = (ai_weapon_t *)pCurrentWeapon;

			weapon_t *pWeapon = &(pAIWeapon->weapon);
			weaponInfo_t *winfo = pWeapon->winfo;
			_ASSERTE( winfo );

			winfo->select_func( self );

			self->curWeapon = pCurrentWeapon;
		}
    }
    else
    {
        self->curWeapon = NULL;
    }
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_ChooseMeleeWeapon
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_ChooseMeleeWeapon( userEntity_t *self )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	_ASSERTE( self->inventory );

	userInventory_t *pCurrentWeapon = NULL;
    int nCurrentEpisode = GetCurrentEpisode();
	switch ( nCurrentEpisode )
	{
		case 1:
		{
	        pCurrentWeapon = gstate->InventoryFindItem( self->inventory, "weapon_disruptor" );
			break;
		}	
		case 2:
		{
            pCurrentWeapon = gstate->InventoryFindItem( self->inventory, "weapon_discus" );
            break;
		}
		case 3:
		{
            pCurrentWeapon = gstate->InventoryFindItem( self->inventory, "weapon_silverclaw" );
            break;
		}
		case 4:
		{
            pCurrentWeapon = gstate->InventoryFindItem( self->inventory, "weapon_glock" );
            break;
		}
		default:
        {
            pCurrentWeapon = gstate->InventoryFindItem( self->inventory, "weapon_disruptor" );
            break;
        }
	}

    if ( pCurrentWeapon )
    {
        ai_weapon_t	*pAIWeapon = (ai_weapon_t *)pCurrentWeapon;

        weapon_t *pWeapon = &(pAIWeapon->weapon);
        weaponInfo_t *winfo = pWeapon->winfo;
        _ASSERTE( winfo );

        winfo->select_func( self );

        self->curWeapon = pCurrentWeapon;

        return TRUE;
    }
    else
    {
        self->curWeapon = NULL;
    }

    return FALSE;   
}




// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_CanPickupWeapon
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_CanPickupWeapon( userEntity_t *pItem )
{
    if ( !ITEM_IsWeapon( pItem ) )
    {
        return FALSE;
    }
    	
    int bRetValue = FALSE;


	int nCurrentEpisode = GetCurrentEpisode();
	switch ( nCurrentEpisode )
	{
		case 1:
		{
			if ( _stricmp(pItem->className, "weapon_disruptor") == 0 ||
			     _stricmp(pItem->className, "weapon_ionblaster") == 0 ||
			     _stricmp(pItem->className, "weapon_shotcycler") == 0 )
			{
				
				bRetValue = TRUE;
			}
			break;
		}	
		case 2:
		{
			if ( _stricmp(pItem->className, "weapon_discus") == 0 ||
			     _stricmp(pItem->className, "weapon_venomous") == 0 )
			{
				bRetValue = TRUE;
			}
            break;
		}
		case 3:
		{
			if ( _stricmp(pItem->className, "weapon_silverclaw") == 0 ||
			     _stricmp(pItem->className, "weapon_bolter") == 0 ||
			     _stricmp(pItem->className, "weapon_ballista") == 0 )
			{
                bRetValue = TRUE;
			}
            break;
		}
		case 4:
		{
			if ( _stricmp(pItem->className, "weapon_glock") == 0 ||
			     _stricmp(pItem->className, "weapon_ripgun") == 0 ||
			     _stricmp(pItem->className, "weapon_slugger") == 0 )
			{
			    bRetValue = TRUE;
            }
            break;
		}
		default:
        {
            bRetValue = TRUE;
            break;
        }
	}
    return bRetValue;
}

// ----------------------------------------------------------------------------
// NSS[1/31/00]:
// Name:        SIDEKICK_CanPickupAmmo
// Description:This will first determine if we can pickup that type of ammo and then will
// make sure that we aren't already full in our inventory.
// Input:userEntity_t *pItem, userEntity_t *self
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_CanPickupAmmo( userEntity_t *pItem, userEntity_t *self )
{
	int bRetValue = TRUE;
    int nCurrentEpisode = GetCurrentEpisode();
	char Buffer[64];

	
	if ( !ITEM_IsAmmo( pItem ) )
    {
        return FALSE;
    }
    	
	switch ( nCurrentEpisode )
	{
		case 1:
		{
			if ( _stricmp(pItem->className, "ammo_ionpack") == 0 )
			{
				Com_sprintf(Buffer,sizeof(Buffer),"weapon_ionblaster");
			}
			else if (_stricmp(pItem->className, "ammo_shells") == 0 )
			{
				Com_sprintf(Buffer,sizeof(Buffer),"weapon_shotcycler");
			}
			else
			{
				bRetValue = FALSE;
			}
			break;
		}	
		case 2:
		{
			if ( _stricmp(pItem->className, "ammo_venomous") == 0 )
			{
				Com_sprintf(Buffer,sizeof(Buffer),"weapon_venomous");
			}
			else if ( _stricmp(pItem->className, "ammo_trident") == 0 )
			{
				Com_sprintf(Buffer,sizeof(Buffer),"weapon_trident");
			}
			else
			{
				bRetValue = FALSE;
			}

			break;
		}
		case 3:
		{
			if ( _stricmp(pItem->className, "ammo_bolts") == 0)
			{
				Com_sprintf(Buffer,sizeof(Buffer),"weapon_bolter");
			}
			else if(_stricmp(pItem->className, "ammo_ballista") == 0 )
			{
                Com_sprintf(Buffer,sizeof(Buffer),"weapon_ballista");
			}
			else
			{
				bRetValue = FALSE;
			}

			break;
		}
		case 4:
		{
			if ( _stricmp(pItem->className, "ammo_bullets") == 0)
			{
				Com_sprintf(Buffer,sizeof(Buffer),"weapon_glock");
			}
			else if(_stricmp(pItem->className, "ammo_ripgun") == 0)
			{
				Com_sprintf(Buffer,sizeof(Buffer),"weapon_ripgun");
			}
			else if(_stricmp(pItem->className, "ammo_slugger") == 0 )
			{
			    Com_sprintf(Buffer,sizeof(Buffer),"weapon_slugger");
            }
			else
			{
				bRetValue = FALSE;
			}
			break;
		}
		default:
        {   
			bRetValue = FALSE;
            break;
        }
	}
	
	// NSS[1/31/00]:If we can actually use this weapon
	if(bRetValue)
	{
		weapon_t *pWeapon = (weapon_t *)gstate->InventoryFindItem( self->inventory, Buffer );
		if(pWeapon)
		{
			// NSS[1/31/00]:As long as we aren't maxed out.
			if(pWeapon->ammo->count >= pWeapon->winfo->ammo_max)
			{
				bRetValue = FALSE;
			}
		}
		else
		{
			bRetValue = FALSE;
		}
	}
	
	return bRetValue;
}

// ----------------------------------------------------------------------------
// NSS[12/14/99]:
// Name:		SIDEKICK_DetermineNeedWeapon
// Description: Will determine if the sidekick can pickup a specific special weapon type.
// Input: userEntity_t *self, char *Weapon_Name
// Output:TRUE/FALSE
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_DetermineNeedWeapon( userEntity_t *self, char *Weapon_Name)
{
	userInventory_t *inv = gstate->InventoryFirstItem( self->inventory );
	
	weapon_t *item = (weapon_t *)gstate->InventoryFindItem(self->inventory,Weapon_Name);
	if (!item)
		return TRUE;

	if (item->ammo->count >= item->winfo->ammo_max)
		return FALSE;

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_CanPickupItem
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_CanPickupItem( userEntity_t *self, userEntity_t *pItem, int bForcePickup /* = FALSE */ )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
    _ASSERTE( AI_IsSidekick( hook ) );

    int RetValue = FALSE;
	if ( !ITEM_IsItem( pItem ) )
    {
        return FALSE;
    }
	float XYDistance	= VectorXYDistance(self->s.origin,pItem->s.origin);
	float ZDistance		= VectorZDistance(self->s.origin,pItem->s.origin);

	// NSS[2/29/00]:If we cannot find a path to the item.
	if(!AI_FindPathToEntity(self,pItem,TRUE))
	{
		if(!AI_IsVisible(self,pItem))
		{
			return -1;
		}	 
		else if(!(XYDistance < 255.0f && ZDistance < 32.0f))
		{
			return -1;
		}
		
	}

	if(pItem->nSidekickFlag & MIKIKO_NO_PICKUP && hook->type == TYPE_MIKIKO && !bForcePickup)
	{
		return FALSE;
	}
	else if(pItem->nSidekickFlag & SUPERFLY_NO_PICKUP && (hook->type == TYPE_SUPERFLY || hook->type == TYPE_MIKIKOFLY) && !bForcePickup)
	{
		return FALSE;
	}	
	
	if ( strstr( pItem->className, "item_health" ) )
	{
        if ( self->health < (hook->base_health - (hook->base_health*.05)) )
        {
            RetValue = TRUE;
        }
	}
	else
	if ( _stricmp( pItem->className, "item_goldensoul" ) == 0 )
	{
        if ( self->health <= 100 || bForcePickup)
        {
            RetValue = TRUE;
        }
	}
    else
    if ( strstr( pItem->className, "weapon" ) )
	{
        // all weapons should be flagged anyway, if not
       	// NSS[2/14/00]:MikikoFly does not use weapons!
		if(hook->type == TYPE_MIKIKOFLY)
		{
			return FALSE;
		}
		// NSS[3/28/00]:Do a small filter to make sure we need this weapon type.
		if(!SIDEKICK_DetermineNeedWeapon( self, pItem->className))
			return FALSE;

		RetValue = SIDEKICK_CanPickupWeapon( pItem );
	}
	else
	if ( strstr( pItem->className, "ammo" ) )
	{
      	// NSS[2/14/00]:MikikoFly does not use weapons!
		if(hook->type == TYPE_MIKIKOFLY)
		{
			return FALSE;
		}
 
		RetValue = SIDEKICK_CanPickupAmmo( pItem, self );
	}
	else
	if ( strstr( pItem->className, "armor" ) )
    {
	    if ((self->health * ((float)pItem->armor_abs/100.0)) > (self->armor_val * self->armor_abs))
	    {
			RetValue = TRUE;
	    }
    }
	else
	if ( _stricmp( pItem->className, "item_power_boost" ) == 0 ||
		 _stricmp( pItem->className, "item_acro_boost" ) == 0 ||
		 _stricmp( pItem->className, "item_attack_boost" ) == 0 ||
		 _stricmp( pItem->className, "item_speed_boost" ) == 0 ||
		 _stricmp( pItem->className, "item_vita_boost" ) == 0 )
	{
        // no power ups for
        return FALSE;
	}
	else if (((!_stricmp(pItem->className,"misc_lifewater")) || (!_stricmp(pItem->className,"misc_hosportal")) || (!_stricmp(pItem->className,"misc_healthtree"))) )
	{
		if(!_stricmp(pItem->className,"misc_healthtree"))
		{
			healthtreeHook_t	*ihook = (healthtreeHook_t *) pItem->userHook;
			if(ihook && hook)
			{
				// NSS[5/24/00]:As long as we have fruit and low health then we can still use this item.
				if(ihook->current_fruit && (self->health < (hook->base_health - (hook->base_health*.05))) ) 
				{
					RetValue = TRUE;
				}			
			}
		}
		else
		{
			hosportalHook_t	*ihook = (hosportalHook_t *) pItem->userHook;
			if(ihook && hook)
			{
				// NSS[5/19/00]:If the hosportal has some juice left in it and we need health then let's do it.
				if(ihook->current_juice > 0 && (self->health < (hook->base_health - (hook->base_health*.05))) ) 
				{
					RetValue = TRUE;
				}
			}
		}
	}

	return RetValue;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnHiro
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *SIDEKICK_SpawnHiro( userEntity_t *self )
{
	return SIDEKICK_Spawn( self, TYPE_CLIENT );
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnSuperfly
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *SIDEKICK_SpawnSuperfly( userEntity_t *self )
{
	return SIDEKICK_Spawn( self, TYPE_SUPERFLY );
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnMikiko
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *SIDEKICK_SpawnMikiko( userEntity_t *self )
{
	return SIDEKICK_Spawn( self, TYPE_MIKIKO );
}


// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnMikikoFly
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *SIDEKICK_SpawnMikikoFly( userEntity_t *self )
{
	return SIDEKICK_Spawn( self, TYPE_MIKIKOFLY );
}


// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnHiro2
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SpawnHiro2( userEntity_t *self )
{
	SIDEKICK_Spawn( self, TYPE_CLIENT );
}


// ----------------------------------------------------------------------------
// NSS[2/15/00]:
// Name:		SIDEKICK_TeleportSetup
// Description:Setups the teleport ihnformation then assigns the task
// Input:userEntity_t *self (where self is the player), MONSTER_TYPE Type
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_TeleportSetup(userEntity_t *self, MONSTER_TYPE Type)
{
	userEntity_t *Sidekick;
	playerHook_t *hook; 
	
	if(Type ==  TYPE_SUPERFLY || Type == TYPE_MIKIKOFLY)
	{
		Sidekick	= AIINFO_GetSuperfly();
	}
	else
	{
		Sidekick	= AIINFO_GetMikiko();
	}
	
	if(Sidekick)
	{
		hook	= AI_GetPlayerHook(Sidekick);
		if(hook->type == Type)
		{
			AI_AddNewTaskAtFront(Sidekick,TASKTYPE_SIDEKICK_TELEPORT,self->s.origin);
		}
	}
}


// ----------------------------------------------------------------------------
// NSS[2/15/00]:
// Name:		SIDEKICK_TeleportSuperfly
// Description:Teleports Superfly close to the player
// Input:userEntity_t *self (where self is the player)
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_TeleportSuperfly( userEntity_t *self )
{
	SIDEKICK_TeleportSetup( self, TYPE_SUPERFLY );
}

// ----------------------------------------------------------------------------
// NSS[2/15/00]:
// Name:		SIDEKICK_TeleportMikiko
// Description:Teleports Mikikofly close to the player
// Input:userEntity_t *self (where self is the player)
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_TeleportMikiko( userEntity_t *self )
{
	SIDEKICK_TeleportSetup( self, TYPE_MIKIKO );

}

// ----------------------------------------------------------------------------
// NSS[2/15/00]:
// Name:		SIDEKICK_TeleportMikikoFly
// Description:Teleports Mikikofly close to the player
// Input:userEntity_t *self (where self is the player)
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_TeleportMikikoFly( userEntity_t *self )
{
	SIDEKICK_TeleportSetup( self, TYPE_MIKIKOFLY );
}




// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnSuperfly2
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SpawnSuperfly2( userEntity_t *self )
{
	SIDEKICK_Spawn( self, TYPE_SUPERFLY );
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnMikiko2
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SpawnMikiko2( userEntity_t *self )
{
	SIDEKICK_Spawn( self, TYPE_MIKIKO );

}

// ----------------------------------------------------------------------------
// NSS[12/16/99]:
// Name:		SIDEKICK_SpawnMikikoFly2
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SpawnMikikoFly2( userEntity_t *self )
{
	SIDEKICK_Spawn( self, TYPE_MIKIKOFLY );
}
// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnHiro
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *SIDEKICK_SpawnHiro( const CVector &origin, const CVector &angle )
{
	return SIDEKICK_Spawn( TYPE_CLIENT, origin, angle );
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnSuperfly
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *SIDEKICK_SpawnSuperfly( const CVector &origin, const CVector &angle )
{
	return SIDEKICK_Spawn( TYPE_SUPERFLY, origin, angle );
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnMikiko
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *SIDEKICK_SpawnMikiko( const CVector &origin, const CVector &angle )
{
	return SIDEKICK_Spawn( TYPE_MIKIKO, origin, angle );
}



// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_SpawnMikikoFly
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *SIDEKICK_SpawnMikikoFly( const CVector &origin, const CVector &angle )
{
	return SIDEKICK_Spawn( TYPE_MIKIKOFLY, origin, angle );
}



// ----------------------------------------------------------------------------
//
// Name:		info_mikikofly_start
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void info_mikikofly_start( userEntity_t *self )
{
	// SCG[2/11/00]: the model still needs to be registered, though!!!
	char *szModelName = AIATTRIBUTE_GetModelName( SUPERFLY_CLASSNAME );
	if( szModelName == NULL )
	{
		gstate->Error( "Could not get superfly model info" );
	}
	gstate->ModelIndex( szModelName );

	for( int i = 0; self->epair[i].key; i++ )
	{
		if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;
	}

	// SCG[2/6/00]: do not spawn on level transitions.( that is taken care of in dll_putclientinserver )
	// SCG[2/6/00]: this will allow us leave sidekicks behind on certain levels.
	if( ( gstate->nLevelTransitionType == 1 ) && ( self->targetname == NULL ) )
	{
		return;
	}

	if( self->targetname )
	{
		if( strcmp( gstate->game->spawnpoint, self->targetname ) )
		{
			return;
		}
	}

	// SCG[1/28/00]: Create a new entity for the sidekick
	userEntity_t *pSidekickEntity = gstate->SpawnEntity();
	if( pSidekickEntity == NULL )
	{
		return;
	}

	pSidekickEntity->s.origin = self->s.origin;
	pSidekickEntity->s.angles = self->s.angles;

	SIDEKICK_Start( pSidekickEntity, TYPE_MIKIKOFLY );
	
	gstate->Con_Printf( "info_superfly_start\n" );
}

// ----------------------------------------------------------------------------
//
// Name:		info_superfly_start
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void info_superfly_start( userEntity_t *self )
{
	// SCG[2/11/00]: the model still needs to be registered, though!!!
	char *szModelName = AIATTRIBUTE_GetModelName( SUPERFLY_CLASSNAME );
	if( szModelName == NULL )
	{
		gstate->Error( "Could not get superfly model info" );
	}
	gstate->ModelIndex( szModelName );

	for( int i = 0; self->epair[i].key; i++ )
	{
		if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;
	}

	// SCG[2/6/00]: do not spawn on level transitions.( that is taken care of in dll_putclientinserver )
	// SCG[2/6/00]: this will allow us leave sidekicks behind on certain levels.
	if( ( gstate->nLevelTransitionType == 1 ) && ( self->targetname == NULL ) )
	{
		return;
	}

	if( self->targetname )
	{
		if( strcmp( gstate->game->spawnpoint, self->targetname ) )
		{
			return;
		}
	}

	// SCG[1/28/00]: Create a new entity for the sidekick
	userEntity_t *pSidekickEntity = gstate->SpawnEntity();
	if( pSidekickEntity == NULL )
	{
		return;
	}

	pSidekickEntity->s.origin = self->s.origin;
	pSidekickEntity->s.angles = self->s.angles;

	SIDEKICK_Start( pSidekickEntity, TYPE_SUPERFLY );
	
	gstate->Con_Printf( "info_superfly_start\n" );
}
								  
// ----------------------------------------------------------------------------
//
// Name:		info_mikiko_start
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void info_mikiko_start( userEntity_t *self )
{
	// SCG[2/11/00]: the model still needs to be registered, though!!!
	char *szModelName = AIATTRIBUTE_GetModelName( MIKIKO_CLASSNAME );
	if( szModelName == NULL )
	{
		gstate->Error( "Could not get superfly model info" );
	}
	gstate->ModelIndex( szModelName );

	for( int i = 0; self->epair[i].key; i++ )
	{
		if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;
	}

	// SCG[2/6/00]: do not spawn on level transitions.( that is taken care of in dll_putclientinserver )
	// SCG[2/6/00]: this will allow us leave sidekicks behind on certain levels.
	if( ( gstate->nLevelTransitionType == 1 ) && ( self->targetname == NULL ) )
	{
		return;
	}

	if( self->targetname )
	{
		if( strcmp( gstate->game->spawnpoint, self->targetname ) )
		{
			return;
		}
	}

	// SCG[1/28/00]: Create a new entity for the sidekick
	userEntity_t *pSidekickEntity = gstate->SpawnEntity();
	if( pSidekickEntity == NULL )
	{
		return;
	}

	pSidekickEntity->s.origin = self->s.origin;
	pSidekickEntity->s.angles = self->s.angles;

	SIDEKICK_Start( pSidekickEntity, TYPE_MIKIKO );
	
	gstate->Con_Printf( "info_mikiko_start\n" );
}

// ----------------------------------------------------------------------------
// NSS[1/26/00]:
// Name:		SIDEKICK_IsNeedHealth
// Description:Do we need health?
// Input:userEntity_t *self
// Output:TRUE/FALSE
// ----------------------------------------------------------------------------
int SIDEKICK_IsNeedHealth( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	if(hook)
	{
		if ( self->health <  hook->base_health*0.50)
		{
			return TRUE;
		}	
	}
	return FALSE;
}



// ----------------------------------------------------------------------------
// NSS[2/29/00]:
// Name:		SIDEKICK_DetermineNeededItem
// Description:Determines if the Sidekick Needs something
// Input:userEntity_t *self
// Output:NA
// Note: I have made modifications to this... by far this is not how I would handle
// doing this whole item thang....
// ----------------------------------------------------------------------------
userEntity_t *SIDEKICK_DetermineNeededItem( userEntity_t *self )
{
	_ASSERTE( self->inventory );

	userEntity_t *pNeedItem = NULL;
	
	if ( SIDEKICK_IsNeedHealth( self ) == TRUE )
	{
		pNeedItem = SIDEKICK_GetClosestVisibleItem( self, NULL, 0.0f, ITEMTYPE_HEALTH);
	}

    if ( !pNeedItem )
    {
        pNeedItem = SIDEKICK_GetClosestVisibleWeaponNotInInventory( self );
    }

    if ( !pNeedItem )
    {
        pNeedItem = SIDEKICK_GetClosestVisibleAmmoNotInInventory( self );
    }
	
	// NSS[2/29/00]:Check to make sure we can get to it.
	if(AI_FindPathToEntity(self,pNeedItem,TRUE) && SIDEKICK_DeterminePathLength(self, pNeedItem) < 256.0f )
		return pNeedItem;
	else
		return NULL;
}


// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_DetermineWantItem
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *SIDEKICK_DetermineWantItem( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *pWantItem = NULL;
	if ( self->health < hook->base_health )
	{
		// find a goldensoul
		pWantItem  = SIDEKICK_GetClosestVisibleItem( self, NULL, 0.0f, ITEMTYPE_GOLDENSOUL);
		//pWantItem = SIDEKICK_GetClosestVisibleGoldenSoul( self );
		if ( pWantItem )
		{
			return pWantItem;
		}
	}

	if ( self->health < hook->base_health )
	{
		pWantItem  = SIDEKICK_GetClosestVisibleItem( self, NULL, 0.0f, ITEMTYPE_HEALTH);
		//pWantItem = SIDEKICK_GetClosestVisibleHealth( self );
		if ( pWantItem )
		{
			return pWantItem;
		}
	}

	if ( self->armor_val <= 75 )
	{
		// find a nearest armor	
		pWantItem  = SIDEKICK_GetClosestVisibleItem( self, NULL, 0.0f, ITEMTYPE_ARMOR);
		//pWantItem = SIDEKICK_GetClosestVisibleArmor( self );
		if ( pWantItem )
		{
			return pWantItem;
		}
	}
	else
	if ( self->armor_val <= 150 )
	{
		
		pWantItem  = SIDEKICK_GetClosestVisibleItem( self, "item_plasteel_armor", 0.0f, ITEMTYPE_ARMOR);
		//pWantItem = SIDEKICK_GetClosestVisibleArmor( self, "item_plasteel_armor" );

		if ( pWantItem )
		{
			return pWantItem;
		}
	}

	// look for stat boost
	if ( !(hook->items & IT_POWERBOOST) )
	{
		
		pWantItem  = SIDEKICK_GetClosestVisibleItem( self, "item_power_boost", 0.0f, ITEMTYPE_STATBOOST);
		//pWantItem = SIDEKICK_GetClosestVisibleStatBoost( self, "item_power_boost" );
		if ( pWantItem )
		{
			return pWantItem;
		}
	}
	if ( !(hook->items & IT_ATTACKBOOST) )
	{
		pWantItem  = SIDEKICK_GetClosestVisibleItem( self, "item_attack_boost", 0.0f, ITEMTYPE_STATBOOST);
		//pWantItem = SIDEKICK_GetClosestVisibleStatBoost( self, "item_attack_boost" );
		if ( pWantItem )
		{
			return pWantItem;
		}
	}
	if ( !(hook->items & IT_SPEEDBOOST) )
	{
		pWantItem  = SIDEKICK_GetClosestVisibleItem( self, "item_speed_boost", 0.0f, ITEMTYPE_STATBOOST);
		//pWantItem = SIDEKICK_GetClosestVisibleStatBoost( self, "item_speed_boost" );
		if ( pWantItem )
		{
			return pWantItem;
		}
	}
	if ( !(hook->items & IT_ACROBOOST) )
	{
		pWantItem  = SIDEKICK_GetClosestVisibleItem( self, "item_acro_boost", 0.0f, ITEMTYPE_STATBOOST);
		//pWantItem = SIDEKICK_GetClosestVisibleStatBoost( self, "item_acro_boost" );
		if ( pWantItem )
		{
			return pWantItem;
		}
	}
	if ( !(hook->items & IT_VITABOOST) )
	{
		pWantItem  = SIDEKICK_GetClosestVisibleItem( self, "item_vita_boost", 0.0f, ITEMTYPE_STATBOOST);
		//pWantItem = SIDEKICK_GetClosestVisibleStatBoost( self, "item_vita_boost" );
		if ( pWantItem )
		{
			return pWantItem;
		}
	}

	// now look for weapons
	_ASSERTE( self->inventory );

	int nCurrentEpisode = GetCurrentEpisode();
	switch ( nCurrentEpisode )
	{
		case 1:
		{
			weapon_t *pWeapon = (weapon_t *)gstate->InventoryFindItem( self->inventory, "weapon_shotcycler" );
			if ( !pWeapon )
			{
				// find a closest shotcyler
				pWantItem  = SIDEKICK_GetClosestVisibleItem( self, "weapon_shotcycler", 0.0f, ITEMTYPE_WEAPONS);
				//pWantItem = SIDEKICK_GetClosestVisibleWeapon( self, "weapon_shotcycler" );
				if ( pWantItem )
				{
					return pWantItem;
				}
			}
			pWeapon = (weapon_t *)gstate->InventoryFindItem( self->inventory, "weapon_ionblaster" );
			if ( !pWeapon )
			{
				// find a closest ionblaster
				pWantItem  = SIDEKICK_GetClosestVisibleItem( self, "weapon_ionblaster", 0.0f, ITEMTYPE_WEAPONS);
				//pWantItem = SIDEKICK_GetClosestVisibleWeapon( self, "weapon_ionblaster" );
				if ( pWantItem )
				{
					return pWantItem;
				}
			}

			break;
		}	
		case 2:
		{
			weapon_t *pWeapon = (weapon_t *)gstate->InventoryFindItem( self->inventory, "weapon_discus" );
			if ( !pWeapon )
			{
				// find a closest shotcyler
				pWantItem  = SIDEKICK_GetClosestVisibleItem( self, "weapon_discus", 0.0f, ITEMTYPE_WEAPONS);
				if ( pWantItem )
				{
					return pWantItem;
				}
			}
			pWeapon = (weapon_t *)gstate->InventoryFindItem( self->inventory, "weapon_venomous" );
			if ( !pWeapon )
			{
				// find a closest ionblaster
				pWantItem  = SIDEKICK_GetClosestVisibleItem( self, "weapon_venomous", 0.0f, ITEMTYPE_WEAPONS);
				if ( pWantItem )
				{
					return pWantItem;
				}
			}

			break;
		}
		case 3:
		{
			weapon_t *pWeapon = (weapon_t *)gstate->InventoryFindItem( self->inventory, "weapon_silverclaw" );
			if ( !pWeapon )
			{
				// find a closest shotcyler
				pWantItem  = SIDEKICK_GetClosestVisibleItem( self, "weapon_silverclaw", 0.0f, ITEMTYPE_WEAPONS);
				if ( pWantItem )
				{
					return pWantItem;
				}
			}
			pWeapon = (weapon_t *)gstate->InventoryFindItem( self->inventory, "weapon_bolter" );
			if ( !pWeapon )
			{
				// find a closest ionblaster
				pWantItem  = SIDEKICK_GetClosestVisibleItem( self, "weapon_bolter", 0.0f, ITEMTYPE_WEAPONS);
				if ( pWantItem )
				{
					return pWantItem;
				}
			}
			pWeapon = (weapon_t *)gstate->InventoryFindItem( self->inventory, "weapon_ballista" );
			if ( !pWeapon )
			{
				// find a closest ionblaster
				pWantItem  = SIDEKICK_GetClosestVisibleItem( self, "weapon_ballista", 0.0f, ITEMTYPE_WEAPONS);
				if ( pWantItem )
				{
					return pWantItem;
				}
			}
			break;
		}
		case 4:
		{
			weapon_t *pWeapon = (weapon_t *)gstate->InventoryFindItem( self->inventory, "weapon_glock" );
			if ( !pWeapon )
			{
				// find a closest shotcyler
				pWantItem  = SIDEKICK_GetClosestVisibleItem( self, "weapon_glock", 0.0f, ITEMTYPE_WEAPONS);
				if ( pWantItem )
				{
					return pWantItem;
				}
			}
			pWeapon = (weapon_t *)gstate->InventoryFindItem( self->inventory, "weapon_slugger" );
			if ( !pWeapon )
			{
				// find a closest ionblaster
				pWantItem  = SIDEKICK_GetClosestVisibleItem( self, "weapon_slugger", 0.0f, ITEMTYPE_WEAPONS);
				if ( pWantItem )
				{
					return pWantItem;
				}
			}
			pWeapon = (weapon_t *)gstate->InventoryFindItem( self->inventory, "weapon_ripgun" );
			if ( !pWeapon )
			{
				// find a closest ionblaster
				pWantItem  = SIDEKICK_GetClosestVisibleItem( self, "weapon_ripgun", 0.0f, ITEMTYPE_WEAPONS);
				if ( pWantItem )
				{
					return pWantItem;
				}
			}			
			break;
		}

		default:
			break;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_IsWeaponAvailable
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsWeaponAvailable( userEntity_t *self, char *szWeaponName )
{
	_ASSERTE( self );
	
	if ( !self->inventory )
	{
		return FALSE;
	}

	weapon_t *pWeapon = (weapon_t *)gstate->InventoryFindItem( self->inventory, szWeaponName );
	if ( !pWeapon )
	{
		return FALSE;
	}

	return TRUE;
}





//NSS[12/1/99]:Only for sidekicks
int AI_IsSideKickCloseToItem( playerHook_t *hook, float fDistance );
#define HEALTH	1
#define AMMO	2
#define ARMOR	3
// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_StartPickupItem
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartPickupItem( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_SIDEKICK_PICKUPITEM );

	AI_SetStateRunning(hook);
	if ( AI_StartMove(self) == FALSE )
	{
		return;
	}

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	_ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	userEntity_t *pItem = pAIData->pEntity;
	_ASSERTE( pItem );

	// compute the time that may take to get there
	float fDistance = 0;
	if (!AI_FindPathToPoint( self, pItem->s.origin ) )
	{
/*		CVector currentPos = self->s.origin;

		NODEHEADER_PTR pNodeHeader = NODE_GetNodeHeader( self );
		PATHNODE_PTR pPath = hook->pPathList->pPath;
		while ( pPath )
		{
			MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, pPath->nNodeIndex );
			fDistance += VectorXYDistance( currentPos, pNode->position );
			
			currentPos = pNode->position;

			pPath = pPath->next_node;
		}*/
	}
	// NSS[3/6/00]:Setup a quick check to make sure that we still need the item as we approach the item.
	pAIData->nValue = 0;
	if(strstr(pItem->className,"health"))
		pAIData->nValue = HEALTH;
	if(strstr(pItem->className,"ammo"))
		pAIData->nValue = AMMO;
	if(strstr(pItem->className,"armor"))
		pAIData->nValue = ARMOR;

	AI_Dprintf( "Starting TASKTYPE_SIDEKICK_PICKUPITEM.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, -1 );



}	






// ----------------------------------------------------------------------------
// NSS[12/16/99]:
// Name:		SideKick_SpecialMoveTowardPoint
// Description: Specific Move function for specific circumstances, one being picking up items
// Input: userEntity_t *self, CVector &dstpoint, int bMovingTowardNode, int bFaceTowardDest
// Output:T/F
// Note:NA
// ----------------------------------------------------------------------------
int SideKick_SpecialMoveTowardPoint( userEntity_t *self, CVector &destPoint, 
						int bMovingTowardNode /* = FALSE */, int bFaceTowardDest /* = TRUE */ )
{
	_ASSERTE( self );
	
	playerHook_t *hook = AI_GetPlayerHook( self );
	if ( !AI_CanMove( hook ) )
	{
		AI_RemoveCurrentTask( self );
		return FALSE;
	}
	
	// we can just check the x,y distance since this function assumes that the 
	// dest point is visible from self
	float fXYDistance = VectorXYDistance( self->s.origin, destPoint );
	float fZDistance = VectorZDistance( self->s.origin, destPoint );
	if ( fXYDistance < ((self->s.maxs.x - self->s.mins.x)*0.50f) && fZDistance < 32.0f )
	{
		
		AI_UpdateCurrentNode( self );
        return TRUE;
	}

	if ( bFaceTowardDest )
	{
        AI_FaceTowardPoint( self, destPoint );
	}

	float fSpeed = AI_ComputeMovingSpeed( hook );

    if ( !self->groundEntity )
    {
		if ( AI_IsCloseDistance2(self, fXYDistance ) )
		{
			AI_UpdateCurrentNode( self );
            return TRUE;
		}

        if ( self->waterlevel > 0 && self->waterlevel < 3 )
	    {
            float fHeight = 8.0f + ((float)self->waterlevel * 8.0f);
            if ( fZDistance > fHeight  )
            {
                // must be stuck trying to go out of the water, try to jump
	            self->velocity.z = hook->upward_vel*0.50f;
            }
            else
            {
                forward = destPoint - self->s.origin;
                forward.Normalize();

                //	scale speed based on current frame's move_scale
	            float fScale = FRAMES_ComputeFrameScale( hook );
	            AI_SetVelocity( self, forward, (fSpeed * fScale) );
            }

            AI_UpdateCurrentNode( self );

            return FALSE;
        }
        else
        {
            AI_HandleOffGround( self, destPoint, fSpeed );
        }

		return FALSE;
	}

    if ( self->groundEntity && self->waterlevel == 2 )
    {
        if ( fZDistance > 24.0f  )
        {
            // must be stuck trying to go out of the water, try to jump
	        self->velocity.z = hook->upward_vel *0.25f;
            AI_UpdateCurrentNode( self );
            return FALSE;
        }
    }

    userEntity_t *pAboveEntity = AI_IsEntityAbove( self );
    if ( pAboveEntity )
    {
        CVector freePoint;
        if ( AI_FindCollisionFreePoint( self, freePoint ) )
        {
            destPoint = freePoint;
            bMovingTowardNode = FALSE;
        }
    }

    userEntity_t *pBelowEntity = AI_IsEntityBelow( self );
    if ( pBelowEntity )
    {
        // since enemy is below, find a point respective of the enemy
        CVector freePoint;
        if ( AI_FindCollisionFreePoint( pBelowEntity, freePoint ) )
        {
        	forward = freePoint - self->s.origin;
            forward.Normalize();

	        float fDistance = VectorDistance( self->s.origin, freePoint );
	        float fZDistance = VectorZDistance( self->s.origin, freePoint );

	        // compute needed forward velocity to reach the point
	        float fUpwardVelocity = hook->upward_vel * 0.5f;
	        float fForwardVelocity = ai_jump_vel(self, fDistance, fZDistance, fUpwardVelocity);

	        if ( freePoint.z > self->s.origin.z )
	        {
		        fUpwardVelocity += fZDistance;
	        }
	        self->velocity = forward * fForwardVelocity;
	        self->velocity.z = fUpwardVelocity;

	        self->groundEntity = NULL;
            return FALSE;
        }
    }

	if ( AI_HandleDirectPath( self, destPoint, bMovingTowardNode ) )
	{
		AI_UpdateCurrentNode( self );
        return FALSE;
	}

	forward = destPoint - self->s.origin;

	// should never be in water, but just in case
	int bOnFlatSurface = AI_IsOnFlatSurface( self );

	forward.Normalize();

	if ( AI_HandleCollisionWithEntities( self, destPoint, fSpeed ) )
	{
		AI_UpdateCurrentNode( self );
        return FALSE;
	}

	if ( AI_HandleGroundObstacle( self, fSpeed, bMovingTowardNode ) )
	{
		AI_UpdateCurrentNode( self );
        return FALSE;
	}

	float fOneFrameDistance = fSpeed * 0.125f;

	//	scale speed based on current frame's move_scale
	float fScale = FRAMES_ComputeFrameScale( hook );
	AI_SetVelocity( self, forward, (fSpeed * fScale) );

	if ( AI_HandleGettingStuck( self, destPoint, fSpeed ) )
	{
		AI_UpdateCurrentNode( self );
        return FALSE;
	}

	hook->last_origin = self->s.origin;

	AI_UpdateCurrentNode( self );

    AI_HandleCrouching( self );

	ai_frame_sounds( self );
	return FALSE;
}


void SIDEKICK_RemoveItem(userEntity_t *Item);



// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// NSS[12/14/99]:
// Name:		SIDEKICK_DetermineNeedAmmo
// Description: Function that will determine if we still need ammo.
// Input: userEntity_t *self, char *Ammo_Name
// Output:TRUE/FALSE
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_DetermineNeedAmmo( userEntity_t *self, char *Ammo_Name)
{
	userInventory_t *inv = gstate->InventoryFirstItem( self->inventory );
	ammo_t* Ammo_Ptr;
	while ( inv )
	{
		if ( inv->name )
		{
			if ( _stricmp(inv->name, Ammo_Name) == 0 )
			{
				Ammo_Ptr = (ammo_t*)inv;
				if(Ammo_Ptr->winfo && Ammo_Ptr->winfo->ammo_max)
				{
					if(Ammo_Ptr->winfo->ammo_max > Ammo_Ptr->count)
						return TRUE;
				}
				return FALSE;
			}
		}
		inv = gstate->InventoryNextItem( self->inventory );
	}
	return FALSE;
}
// ----------------------------------------------------------------------------
// NSS[12/14/99]:
// Name:		SIDEKICK_PickupItem
// Description: General Pickup for Sidekicks
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_PickupItem( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	userEntity_t *pItem = pAIData->pEntity;
	_ASSERTE( pItem );
	
    int bItemVisible = ITEM_IsVisible( pItem );
    if ( !bItemVisible )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

    float fXYDistance = VectorXYDistance( self->s.origin, pItem->s.origin );
	float fZDistance = VectorZDistance( self->s.origin, pItem->s.origin );
	CVector destPoint;
	destPoint = pItem->s.origin;
	destPoint.z += 16.0f;
	switch(pAIData->nValue)
	{
		case HEALTH:
		{
			// NSS[3/6/00]:We might have run over some health on the way, check to make sure we still need it.
			if(self->health >= hook->base_health)
			{
				AI_RemoveCurrentGoal(self);
				return;
			}
			break;
		}
		case AMMO:
		{
			if(pItem && pItem->className && !SIDEKICK_DetermineNeedAmmo( self, pItem->className))
			{
				AI_RemoveCurrentGoal(self);
				return;
			}
			break;
		}
		case ARMOR:
		{
			// NSS[3/6/00]:Again, we might have run over some armor on our way, if we don't need it give up this task.
			if(self->armor_val >= pItem->health)
			{
				AI_RemoveCurrentGoal(self);
				return;			
			}
			break;
		}
	}

	if ( AI_IsSideKickCloseToItem( hook, fXYDistance ) && fZDistance < 32.0f && AI_IsOkToMoveStraight(self,destPoint,fXYDistance,fZDistance))
    {
		AI_AddNewTaskAtFront(self,TASKTYPE_MOVETOEXACTLOCATION,pItem);
    }
	else
	{
		int bMoved = FALSE;
		if ( hook->pPathList->nPathLength == 0 )
		{
			tr = gstate->TraceLine_q2( self->s.origin, pItem->s.origin, self, MASK_SOLID );
			if ( (tr.fraction >= 0.65f && fZDistance < MAX_JUMP_HEIGHT) || 
				 (tr.fraction >= 0.8f && ((1.2f-tr.fraction) * (fXYDistance+fZDistance)) < 32.0f) )
			{
				AI_MoveTowardPoint( self, pItem->s.origin, FALSE );
				bMoved = TRUE;
			}
		}
		if ( !bMoved )
		{
			if ( hook->pPathList->pPath )
			{
				if ( !AI_HandleUse( self ) )
				{
					if ( AI_Move( self ) == FALSE )
					{
						if ( AI_CanPath( hook ) && !AI_FindPathToPoint( self, pItem->s.origin ) )
						{
							AI_MoveTowardPoint( self, pItem->s.origin, FALSE );
							bMoved = TRUE;
							//AI_RestartCurrentGoal( self );
						}
					}					
				}
			}
			else
			{
				if ( AI_CanPath( hook ) && !AI_FindPathToPoint( self, pItem->s.origin ) )
				{
					AI_MoveTowardPoint( self, pItem->s.origin, FALSE );
					bMoved = TRUE;
					//AI_RestartCurrentGoal( self );
				}
			}
		}
	}
}

#define SIDEKICK_USE_NOTHING	0
#define SIDEKICK_USE_HOSPORTAL	1
#define SIDEKICK_USE_HEALTHTREE 2


// ----------------------------------------------------------------------------
// NSS[5/19/00]:
// Name:		SIDEKICK_StartUseItem
// Description: Setup everything before we go over to use a hosportal.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartUseItem( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	AI_SetStateRunning(hook);
	if ( AI_StartMove(self) == FALSE )
	{
		return;
	}

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	_ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	userEntity_t *pItem = pAIData->pEntity;
	_ASSERTE( pItem );

	// compute the time that may take to get there
	float fDistance = 0;
	
	AI_FindPathToPoint( self, pItem->s.origin );
	
	AI_Dprintf( "Starting TASKTYPE_SIDEKICK_USEITEM.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	AI_SetTaskFinishTime( hook, -1 );


	// NSS[5/24/00]:This here is what sets up our actions through this particular task... and there you have it.
	if(_stricmp(pAIData->pEntity->className,"misc_hosportal") == 0 || _stricmp(pAIData->pEntity->className,"misc_lifewater") == 0)
	{
		pAIData->nValue = SIDEKICK_USE_HOSPORTAL;
	}
	else if (_stricmp(pAIData->pEntity->className,"misc_healthtree") == 0)
	{
		pAIData->nValue = SIDEKICK_USE_HEALTHTREE;	
	}
	else
	{
		pAIData->nValue = SIDEKICK_USE_NOTHING;
	}
}	


// ----------------------------------------------------------------------------
// NSS[5/19/00]:
// Name:		SIDEKICK_UseItem
// Description: Move to the item and also use it
// Input:userEntity_t *self
// Output:NA
// Note:If you are looking through this code I feel for you... no really.. I do.
//
// ----------------------------------------------------------------------------
void SIDEKICK_UseItem(userEntity_t *self)
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	_ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	userEntity_t *pItem = pAIData->pEntity;
	_ASSERTE( pItem );


	float XYDistance, ZDistance;
	XYDistance = VectorXYDistance(self->s.origin, pItem->s.origin);
	ZDistance  = VectorZDistance(self->s.origin, pItem->s.origin);

	// NSS[5/19/00]:The case where we are actually close enough to use the hosportal.
	if(XYDistance < 65.0f && ZDistance <= 32.0f)
	{
		if(pAIData->nValue == SIDEKICK_USE_HOSPORTAL)
		{
			// NSS[3/6/00]:Setup a quick check to make sure that we still need the item as we approach the item.
			hosportalHook_t *ihook = (hosportalHook_t *)pItem->userHook;
			// NSS[5/19/00]:As long as we have enough juice to charge ourself up then lets hang out a bit and keep using it.
			if(ihook->current_juice > 0 && (self->health < (hook->base_health - (hook->base_health *0.025))) )
			{
				
				if(pItem->use && pAIData->fValue < gstate->time)
				{
					pItem->use(pItem,self,self);
					pAIData->fValue = gstate->time + 2.0f;
					if(pAIData->nValue == 0)
					{
						char szAnimation[16];
						AI_SelectAmbientAnimation( self, szAnimation );
						AI_ForceSequence(self, szAnimation, FRAME_LOOP);
					}
					pAIData->nValue = 1;
				}
				else if (pItem->use && pAIData->fValue > gstate->time)
				{
					//We are just waiting to be charged up to MAX HP.
					//What the hell... just make sure we look like we are using it.
					AI_FaceTowardPoint(self,pItem->s.origin);
				}
				else
				{
					AI_Dprintf("Major problem, the hosportal does not have a use function!!\n");
				}
			}
			else
			{
				AI_RemoveCurrentGoal(self);
			}
		}
		else if (pAIData->nValue == SIDEKICK_USE_HEALTHTREE)
		{
			healthtreeHook_t *ihook = (healthtreeHook_t *) pItem->userHook;
			if(ihook->current_fruit && (self->health < (hook->base_health - (hook->base_health *0.025))))
			{
				if(gstate->time >= ihook->ignore_timer  && pItem->use)
				{
					pItem->use(pItem,self,self);
				}
				else
				{
					AI_FaceTowardPoint(self,pItem->s.origin);
				}
			}
			else 
			{
				AI_RemoveCurrentGoal(self);
			}

		}
		else if ( pAIData->nValue == SIDEKICK_USE_NOTHING)
		{
			AI_Dprintf("The entity being pointed at has no code to deal with it!\n Removing this goal.");
			AI_RemoveCurrentGoal(self);
		}
	}
	else
	{
		int bMoved = FALSE;
		if ( hook->pPathList->nPathLength == 0 )
		{
			tr = gstate->TraceLine_q2( self->s.origin, pItem->s.origin, self, MASK_SOLID );
			if ( (tr.fraction >= 0.65f && ZDistance < MAX_JUMP_HEIGHT) || 
				 (tr.fraction >= 0.8f && ((1.2f-tr.fraction) * (XYDistance+ZDistance)) < 32.0f) )
			{
				AI_MoveTowardPoint( self, pItem->s.origin, FALSE );
				bMoved = TRUE;
			}
		}
		if ( !bMoved )
		{
			if ( hook->pPathList->pPath )
			{
				if ( !AI_HandleUse( self ) )
				{
					if ( AI_Move( self ) == FALSE )
					{
						if ( AI_CanPath( hook ) && !AI_FindPathToPoint( self, pItem->s.origin ) )
						{
							AI_MoveTowardPoint( self, pItem->s.origin, FALSE );
							bMoved = TRUE;
						}
					}					
				}
			}
			else
			{
				if ( AI_CanPath( hook ) && !AI_FindPathToPoint( self, pItem->s.origin ) )
				{
					AI_MoveTowardPoint( self, pItem->s.origin, FALSE );
					bMoved = TRUE;
				}
			}
		}	
	}
}



// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_PermissionDenied
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_PermissionDenied( userEntity_t *self )
{
    _ASSERTE( self );
    playerHook_t *hook = AI_GetPlayerHook( self );

    if ( hook->pWantItem )
    {
        ITEM_SetNoPickup( self, hook->pWantItem );
        hook->pWantItem = NULL;

        SIDEKICK_CommandAcknowledge(self,SIDEKICK_COMMAND_NO); // respond to given command
    }
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_PermissionGiven
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_PermissionGiven( userEntity_t *self )
{
    _ASSERTE( self );
    playerHook_t *hook = AI_GetPlayerHook( self );

    if ( hook->pWantItem )
    {
        AI_AddNewTaskAtFront( self, TASKTYPE_SIDEKICK_PICKUPITEM, hook->pWantItem );
		SIDEKICK_CommandAcknowledge(self,SIDEKICK_COMMAND_YES); // respond to given command
    }
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_AskPermissionForItem
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsOkToAskPermissionForItem( userEntity_t *self, userEntity_t *pItem )
{
    playerHook_t *hook = AI_GetPlayerHook( self );
	//NSS[11/30/99]:Added Mikikofly here
    if ( hook->type == TYPE_SUPERFLY || hook->type == TYPE_MIKIKOFLY)
    {
        if ( pItem->nSidekickFlag & SUPERFLY_ASKED_PERMISSION )
        {
            return FALSE;
        }
    }
    else
    if ( hook->type == TYPE_MIKIKO )
    {
        if ( pItem->nSidekickFlag & MIKIKO_ASKED_PERMISSION )
        {
            return FALSE;
        }
    }

    return TRUE;
}

// ----------------------------------------------------------------------------

void SIDEKICK_SetAskedPermissionForItem( userEntity_t *self, userEntity_t *pItem )
{
    playerHook_t *hook = AI_GetPlayerHook( self );
    if ( hook->type == TYPE_SUPERFLY || hook->type == TYPE_MIKIKOFLY)
    {
        pItem->nSidekickFlag |= SUPERFLY_ASKED_PERMISSION;
    }
    else
    if ( hook->type == TYPE_MIKIKO )
    {
        pItem->nSidekickFlag |= MIKIKO_ASKED_PERMISSION;
    }
}

// ----------------------------------------------------------------------------

void SIDEKICK_AskPermissionForItem( userEntity_t *self )
{
    playerHook_t *hook = AI_GetPlayerHook( self );
    _ASSERTE( hook->pWantItem );
    if ( SIDEKICK_IsOkToAskPermissionForItem( self, hook->pWantItem ) )
    {
 		AI_AddNewTaskAtFront( self, TASKTYPE_SIDEKICK_PICKUPITEM, hook->pWantItem );
    }
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_HandlePickupItems
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_HandlePickupItems( userEntity_t *self )
{
    playerHook_t *hook = AI_GetPlayerHook( self );

    if ( self->hacks < gstate->time && !SIDEKICK_IsStaying( hook ) )
    {
        hook->pWantItem = SIDEKICK_DetermineNeededItem( self );
        self->hacks = gstate->time + 1.0f;
		if ( !hook->pWantItem )
        {
            hook->pWantItem = SIDEKICK_DetermineWantItem( self );
        }

        if ( hook->pWantItem )
	    {
           	if(AI_FindPathToEntity(self,hook->pWantItem,TRUE) && SIDEKICK_DeterminePathLength(self,hook->pWantItem) < 256.0f)
			{
				SIDEKICK_AskPermissionForItem( self );
				return TRUE;
			}
        }
    }
    return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_PickedUpItem
// Description:
//				sidekick a bot picks up a new item, this function is called
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_PickedUpItem( userEntity_t *self, const char *szItemName )
{
    if ( _strnicmp( szItemName, "weapon", 6 ) == 0 )
	{
		SIDEKICK_ChooseBestWeapon( self );
	}
	else
	if ( _stricmp( szItemName, "item_plasteel_armor" ) == 0 ||
		 _stricmp( szItemName, "item_chromatic_armor" ) == 0 )
	{
	}									   
	else
	if ( _stricmp( szItemName, "item_power_boost" ) == 0 ||
		 _stricmp( szItemName, "item_acro_boost" ) == 0 ||
		 _stricmp( szItemName, "item_attack_boost" ) == 0 ||
		 _stricmp( szItemName, "item_speed_boost" ) == 0 )
	{
	}
	else
	if ( _stricmp( szItemName, "item_vita_boost" ) == 0 )
	{
	}
	else
	if ( _strnicmp( szItemName, "item_health", 11 ) == 0 )
	{
	}
	else
	if ( _stricmp( szItemName, "item_goldensoul" ) == 0 )
	{
	}
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_IsOwnerTargetingMe
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsOwnerTargetingMe( userEntity_t *self, userEntity_t *pOwner )
{
    _ASSERTE( AI_IsAlive( pOwner ) );

	CVector dir = self->s.origin - pOwner->s.origin;
	dir.Normalize();
	CVector angleTowardSelf;
	VectorToAngles( dir, angleTowardSelf );
	
	float fTowardSelfY = AngleMod(angleTowardSelf.y);
	float fFacingAngleY = AngleMod(pOwner->s.angles.yaw);

	float fYawDiff = fabs( fTowardSelfY - fFacingAngleY );
	if ( fYawDiff < 10.0f || fYawDiff > 350.0f )
    {
	    float fFacingAngleX = AngleMod(pOwner->s.angles.pitch);
	    float fTowardSelfX = AngleMod(angleTowardSelf.x);
	    float fPitchDiff = fabs( fTowardSelfX - fFacingAngleX );
        if ( fPitchDiff < 30.0f || fPitchDiff > 330.0f )
	    {
	        CVector facingVector;
            pOwner->s.angles.AngleToForwardVector( facingVector );

            CVector testPoint;
            VectorMA( pOwner->s.origin, facingVector, 1000.0f, testPoint );
            tr = gstate->TraceLine_q2( pOwner->s.origin, testPoint, pOwner, MASK_SOLID );
	        if ( tr.fraction < 1.0 && tr.ent == self )
            {
    		    return TRUE;
            }
	    }
    }

	return FALSE;
}
// ----------------------------------------------------------------------------
// NSS[12/12/99]:
// Name:        SIDEKICK_EnemyGoalFilter
// Description:This will determine if the enemy iis in an agressive mode or not.
// Input: userEntity_t *ent
// Output:int (yes/no) 1/0
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_EnemyGoalFilter(userEntity_t *ent)
{
	playerHook_t *Mhook;
	Mhook = AI_GetPlayerHook( ent );
	int Valid = 0;
	// NSS[12/11/99]:Monster has to be in attack mode in order for Sidekicks to really react to them.
	if(Mhook)
	{
		GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( Mhook );
		if(pGoalStack)
		{
			GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
			if(pCurrentGoal)
			{
				GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );					
				switch(nGoalType)
				{
					case GOALTYPE_KILLENEMY:
					case GOALTYPE_COLUMN_AWAKEN:
					case GOALTYPE_BUBOID_GETOUTOFCOFFIN:
					case GOALTYPE_BUBOID_MELT:
					case GOALTYPE_ROTWORM_DROPFROMCEILING:
					case GOALTYPE_ROTWORM_JUMPATTACK:
					case GOALTYPE_PROTOPOD_HATCH:
					case GOALTYPE_MEDUSA_GAZE:
					case GOALTYPE_SKEETER_HATCH:
					case GOALTYPE_LYCANTHIR_RESURRECT:
					case GOALTYPE_BUBOID_RESURRECT:
					case GOALTYPE_WYNDRAX_RECHARGE:
					case GOALTYPE_WYNDRAX_POWERUP:
					case GOALTYPE_TAKECOVER:
					case GOALTYPE_WANDER:
					case GOALTYPE_CAMBOT_FOLLOWPLAYER:
					{
						Valid = 1;
					}
				}
			}
		}
	}
	return Valid;	
}


// ----------------------------------------------------------------------------
// NSS[12/12/99]:
// Name:        SIDEKICK_WhoIsClosest
// Description:Function to determine who should be the 'controller' for doors etc..
// Input:userEntity_t *self,MAPNODE_PTR pCurrentNode
// Output:T/F
// Note:NA
// ----------------------------------------------------------------------------
int SIDEKICK_WhoIsClosest(userEntity_t *self,MAPNODE_PTR pCurrentNode)
{
	float fDistanceMe;
	float fDistanceThem;

	userEntity_t *pOtherSidekick = SIDEKICK_GetOtherSidekick( self );
	if(pOtherSidekick != NULL)
	{
		fDistanceMe		= VectorDistance(self->s.origin,pCurrentNode->position);
		fDistanceThem	= VectorDistance(pOtherSidekick->s.origin,pCurrentNode->position);
		if(fDistanceMe > fDistanceThem)
			return FALSE;
	}
	return TRUE;
}



// ----------------------------------------------------------------------------
// NSS[12/12/99]:
// Name:        SIDEKICK_AttackOwnerFacingEnemy
// Description:This is a new part of the attack command for the sidekicks.  It will
// trace out from the owner's view and find the enemy the owner is pointing at and 
// attack it.
// Input:userEntity_t *self
// Output:TRUE/FALSE (True-->going to attack/False-->can't find enemy)
// Note:NA
// ----------------------------------------------------------------------------
int SIDEKICK_AttackOwnerFacingEnemy(userEntity_t *self)
{

	playerHook_t *hook = AI_GetPlayerHook(self);
	if(hook)
	{
		if(hook->owner)
		{
			CVector Dir, Destination,Start;
			Start = hook->owner->s.origin;
			Start.z += 22.0f;
			// NSS[2/4/00]:Get 
			hook->owner->client->ps.viewangles.AngleToForwardVector(Dir);
			//hook->owner->s.angles.AngleToForwardVector(Dir);
			Destination = Start + 800*Dir;
			tr = gstate->TraceLine_q2( Start, Destination, hook->owner, MASK_SHOT);
			// NSS[2/4/00]:if we have hit something considered to be a monster solid and it thinks then kill it!
			
			SIDEKICK_Drop_Markers((Start + (Dir *(800*tr.fraction))),1.0f);
			if(tr.fraction < 1.0f && (tr.ent->flags & FL_MONSTER))
			{
				self->enemy = tr.ent;
				if(SIDEKICK_IsOkToFight(self))
				{
					return TRUE;
				}
				else
				{
					self->enemy = NULL;
				}
			}
		}
	}
	return FALSE;
}

// ----------------------------------------------------------------------------
// NSS[2/17/00]:
// Name:        SIDEKICK_DeterminePathLength
// Description:Adding this for mid-textures that can be seen through.  This will
// check the path length every time and determine how far we have to travel in order
// to get to our destination.
// Input:userEntity_t *self
// Output:userEntity_t *self
// Note:NA
// ----------------------------------------------------------------------------
float SIDEKICK_DeterminePathLength(userEntity_t *self, userEntity_t *target)
{
	// NSS[2/17/00]:Make it some stupid distance
	float distance = -1;
	playerHook_t *hook = AI_GetPlayerHook(self);
	if(hook)
	{
		if(hook->pPathList)
		{
			// update the current node for this entity
			NODELIST_PTR pNodeList = hook->pNodeList;
			if(pNodeList)
			{
				NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
				if(pNodeHeader)
				{
					if(hook->pPathList->pPath && hook->pPathList->pPath->next_node)
					{
						PATHNODE_PTR CurrentPath	=	hook->pPathList->pPath;
						if(CurrentPath)
						{
							MAPNODE_PTR pCurrentNode	= NODE_GetNode( pNodeHeader, CurrentPath->nNodeIndex );
							if(pCurrentNode)
							{
								MAPNODE_PTR pNextNode;
								
								while(CurrentPath)
								{
									pNextNode = NULL;
									if(CurrentPath->next_node)
									{
										pNextNode = NODE_GetNode( pNodeHeader, CurrentPath->next_node->nNodeIndex );
									}
									if(pNextNode != NULL)
									{
										// NSS[3/1/00]:Added this per John request to determine if our path is blocked by something
										tr = gstate->TraceLine_q2( pCurrentNode->position,pNextNode->position, NULL, MASK_SOLID);
										if(tr.fraction < 1.0f && tr.ent)
										{
											if(strstr(tr.ent->className,"door"))
											{
												if (!(pCurrentNode->node_type & (NODETYPE_DOORBUTTON | NODETYPE_DOOR) || ( pNextNode && (pNextNode->node_type & ( NODETYPE_DOORBUTTON | NODETYPE_DOOR)) )) )
												{
													return -1;
												}
											}
											else
											{
												return -1;
											}
										}
										distance += VectorDistance(pCurrentNode->position,pNextNode->position);		
									}
									else
									{
										// NSS[3/11/00]:Finally trace to the final target from the end Node to make sure we can reach it.
										tr = gstate->TraceLine_q2( pCurrentNode->position,target->s.origin, NULL, MASK_SOLID);
										if(tr.fraction < 1.0f)
										{
											return -1;
										}
									}
									pCurrentNode = pNextNode;
									CurrentPath = CurrentPath->next_node;
								}
							}
						}
					}
					else if (hook->pPathList->pPath && !hook->pPathList->pPath->next_node)
					{
						MAPNODE_PTR pCurrentNode	= NODE_GetNode( pNodeHeader, hook->pPathList->pPath->nNodeIndex);
						distance += VectorDistance(self->s.origin,pCurrentNode->position);
					}
				}
			}
		}
	}
	return distance;
}


// ----------------------------------------------------------------------------
// NSS[12/12/99]:
// Name:        SIDEKICK_FindEnemy_DeepLoop
// Description:Had to add a second layer to do processing after I get the enemy
// Input:userEntity_t *self
// Output:userEntity_t *self
// Note:NA
// ----------------------------------------------------------------------------
userEntity_t *SIDEKICK_FindEnemy_DeepLoop( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *Mhook;
	playerHook_t *hook = AI_GetPlayerHook( self );
    userEntity_t *pOwner = hook->owner;
	userEntity_t *Owner_Enemy,*SideKick_Enemy,*Self_Enemy,*SideKick,*ent;
	float fOwnerDis, fSideDis, fSelfDis,fDistance,wDistance;
	CVector	position;
    
	fOwnerDis = fSideDis = fSelfDis = 9999;

	Owner_Enemy = SideKick_Enemy = Self_Enemy = SideKick = ent = NULL;
	
	userEntity_t *pOtherSidekick = SIDEKICK_GetOtherSidekick( self );



    //userEntity_t *pMonster = alist_FirstEntity( monster_list );
	userEntity_t *pMonster = gstate->FirstEntity();
	
	//for (vm = gstate->FirstEntity (); vm && stricmp (vm->className, "viewthing"); vm = gstate->NextEntity (vm));
	weapon_t *pWeapon = (weapon_t *) self->curWeapon;

	if(pWeapon && pWeapon->winfo)
		wDistance = pWeapon->winfo->range;
	else
		wDistance = 0.0f;

	//Parse through all monsters
	while ( pMonster )
	{
		if(pMonster->flags & FL_MONSTER)
		{
			// NSS[12/2/99]:FirstCheck to see if the monster has an enemy.  We are reactional and not action oriented A.I. creatures. Then check to make
			// sure that the AI is in an Offensive mode, then check to make sure the offensive mode is adversely going to effect someone in the party.
			if ( pMonster->enemy  && AI_IsAlive(pMonster) && SIDEKICK_EnemyGoalFilter(pMonster))
			{
				fDistance = VectorDistance( self->s.origin, pMonster->s.origin );
				
				if ((SIDEKICK_IsClearShot(self,pMonster,self->s.origin,pMonster->s.origin) || !pWeapon || wDistance < fDistance) && fDistance < 1048.0f)
				{
					// NSS[1/22/00]:Make sure we don't see any really bad monsters... and if we do let's run!
					if(!SIDEKICK_CheckMonsterType(self,pMonster) && (AI_IsVisible(self,pMonster) || AI_IsVisible(pOwner,pMonster)) )
					{
						return pMonster;
					}

					// NSS[12/2/99]:Priority #1 Take care of ourself
					if(pMonster->enemy == self)
					{
						if (fDistance < fSelfDis )
						{
							Self_Enemy	= pMonster;
							fSelfDis	= fDistance;
						}
					}
					else// NSS[12/2/99]:Priority #2 Look to see if either the player or the other sidekick is in trouble
					if(pMonster->enemy->flags & FL_CLIENT)
					{
						if(fDistance < fOwnerDis)
						{
							Owner_Enemy	= pMonster;
							fOwnerDis	= fDistance;
							pOwner->enemy = pMonster;
						}
					}
					else// NSS[12/2/99]:Part of Priority #2
					{
						Mhook = AI_GetPlayerHook( pMonster->enemy );
						if(Mhook)
						{
							if((Mhook->type == TYPE_MIKIKO || Mhook->type == TYPE_SUPERFLY) && (Mhook->type != hook->type) )
							{
								if(fDistance < fSideDis)
								{
									// NSS[12/2/99]:Get the pointer to the monster and the other sidekick
									SideKick		= pMonster->enemy;
									SideKick_Enemy	= pMonster;
									fSideDis		= fDistance;

								}
							}
						}
					}
				}
			}
		}
		else if(pMonster->flags & FL_EXPLOSIVE)// NSS[1/28/00]:For sunflare, C4, Venomous
		{
			fDistance = VectorDistance( self->s.origin, pMonster->s.origin );
			if(fDistance < 256.0f)
			{
				AI_AddNewTaskAtFront(self,TASKTYPE_SIDEKICK_EVADE,pMonster);
				return pMonster;
			}
		}

		// NSS[12/2/99]:Continue through the list of monsters
		pMonster = gstate->NextEntity (pMonster);
        //pMonster = alist_NextEntity( monster_list );
	}
	
	
	// NSS[12/20/99]:Let's worry about ourself first.
	if(Self_Enemy != NULL)
	{
		ent = Self_Enemy;
		fDistance = VectorDistance(self->s.origin,Self_Enemy->s.origin);
		if(!SIDEKICK_IsClearShot(self,ent,self->s.origin,ent->s.origin))
		{
			if(SIDEKICK_FindGoodShootingSpot(self,ent, &position))
			{
				if(Owner_Enemy)
				{
					// NSS[3/11/00]:If it is a monster to run from or 
					if(!SIDEKICK_CheckMonsterType(self,Owner_Enemy) || (fDistance > VectorDistance(self->s.origin,Owner_Enemy->s.origin)) )
					{
						return Owner_Enemy;
					}
				}
				if(SIDEKICK_CanPathToOrIsCloseToEnemy(self, Self_Enemy))
				{
					return ent;
				}
			}
			else if(Owner_Enemy)
			{
					if(!SIDEKICK_CheckMonsterType(self,Owner_Enemy))
					{
						return Owner_Enemy;
					}
			}
			// NSS[2/27/00]:Did not find a thing reset ent
			ent = NULL;
		}
		else if(Owner_Enemy == NULL && SideKick_Enemy == NULL)
		{
			return ent;	
		}
	}
	
	// NSS[12/2/99]:Check to see if other sidekick is in trouble
	if(SideKick_Enemy != NULL)
	{
		// NSS[12/2/99]:Let's not be selfish, if we have more than 50 health and our buddy is in trouble let's help out.
		if((SideKick->health < self->health && self->health > 50.0f) || Self_Enemy == NULL)
		{
			ent = SideKick_Enemy;
			if(!SIDEKICK_IsClearShot(self,ent,self->s.origin,ent->s.origin))
			{
				if(SIDEKICK_FindGoodShootingSpot(self,ent, &position))
				{
					return ent;
				}
			}
			else
			{
				return ent;	
			}
		}
	}

	// NSS[12/20/99]: Finally let's help the player out.
	if(Owner_Enemy != NULL)
	{
		ent = Owner_Enemy;
		if(!SIDEKICK_IsClearShot(self,ent,self->s.origin,ent->s.origin))
		{
			if(SIDEKICK_FindGoodShootingSpot(self,ent, &position))
			{
				return ent;
			}
		}
		else
		{
			return ent;	
		}
	}

	if(!pWeapon && ent)
	{
		return ent;
	}
	return NULL;
}



// ----------------------------------------------------------------------------
// NSS[12/12/99]:
// Name:        SIDEKICK_FindEnemy
// Description:This is a new and improved version of find enemy for the sidekicks
// Input:userEntity_t *self
// Output:userEntity_t *self
// Note:NA
// ----------------------------------------------------------------------------
userEntity_t *SIDEKICK_FindEnemy( userEntity_t *self )
{
	userEntity_t *enemy;
	//playerHook_t *hook;
	
	enemy = NULL;
	enemy = SIDEKICK_FindEnemy_DeepLoop(self);

	if(enemy && !(enemy->flags & FL_EXPLOSIVE))
	{
		//hook = AI_GetPlayerHook(enemy);
		//if(hook)
		//{
			if( !SIDEKICK_CanPathToOrIsCloseToEnemy(self, enemy) )
			{
				enemy = NULL;
			}
			/*else			
			if(enemy->movetype == MOVETYPE_WALK || enemy->movetype == MOVETYPE_HOP)
			{
				float zDistance = VectorZDistance(self->s.origin, enemy->s.origin);
				// NSS[2/3/00]:Set it to make the OkToFight call.
				self->enemy = enemy;
				if(zDistance > 156.0f && !SIDEKICK_IsOkToFight(self))
				{
					enemy		= NULL;
				}
				// NSS[2/3/00]:Reset it... let the exterior calls figure this one out.
				self->enemy = NULL;
			}*/
		//}
	}
	// NSS[3/8/00]:Sidekicks are not affraid of the player's missiles.
	if(enemy && (enemy->flags & FL_EXPLOSIVE) && enemy->movetype == MOVETYPE_FLYMISSILE)
		return NULL;
	else
		return enemy;
}


// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_FindClosestEnemy
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define MONSTER_DETECTION_CHANCE    0.1f

userEntity_t *SIDEKICK_FindClosestEnemy( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
    userEntity_t *pOwner = hook->owner;
    if ( !pOwner )
    {
        return NULL;
    }

    float fClosestDistance = 1024.0f;
    userEntity_t *pClosestEnemy = NULL;

    float fClosestDetectedDistance = 4096.0f;
    userEntity_t *pDetectedEnemy = NULL;

    userEntity_t *pMonster = alist_FirstEntity( monster_list );
	while ( pMonster )
	{
		if ( pMonster->enemy && AI_IsAlive( pMonster ) && AI_IsLineOfSight( self, pMonster ) &&
             (pMonster->enemy == pOwner || pMonster->enemy == self) )
        {
            float fXYDistance = VectorXYDistance( self->s.origin, pMonster->s.origin );
            float fZDistance = VectorZDistance( self->s.origin, pMonster->s.origin );
		    if ( fXYDistance < fClosestDistance && fZDistance < 256.0f )
		    {
			    fClosestDistance = fXYDistance;
                pClosestEnemy = pMonster;
            }
            else
            {
                float fDistance = VectorDistance( self->s.origin, pMonster->s.origin );
                if ( fDistance < fClosestDetectedDistance && rnd() < MONSTER_DETECTION_CHANCE )
                {
                    pDetectedEnemy = pMonster;
                    fClosestDetectedDistance = fDistance;
                }
            }
        }

        pMonster = alist_NextEntity( monster_list );
	}

    if ( !pClosestEnemy )
    {
        pClosestEnemy = pDetectedEnemy;
    }

    return pClosestEnemy;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_TestRandomWalkPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define RANDOMWALK_DISTANCE			128.0f
#define CLOSER_RANDOMWALK_DISTANCE	96.0f

int SIDEKICK_TestRandomWalkPoint( userEntity_t *self, CVector &mins, CVector &maxs, CVector &vector, CVector &randomWalkPoint )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *pOwner = hook->owner;
    _ASSERTE( pOwner );

	CVector endPoint;
	VectorMA( self->s.origin, vector, RANDOMWALK_DISTANCE, endPoint );
	float fDistance = VectorDistance( pOwner->s.origin, endPoint );
	if ( fDistance < SIDEKICK_GetRandomWalkDistance( hook ) &&
	     AI_IsGroundBelowBetween( self, self->s.origin, endPoint ) )
	{
		tr = gstate->TraceBox_q2( self->s.origin, mins, maxs, endPoint, self, MASK_SOLID|CONTENTS_MONSTER );
		if ( tr.fraction >= 1.0f )
		{
            VectorMA( self->s.origin, vector, RANDOMWALK_DISTANCE - 24.0f, randomWalkPoint );
            _ASSERTE( randomWalkPoint.Length() > 0.0f );
			return TRUE;
		}
    }
	else
	{
		VectorMA( self->s.origin, vector, CLOSER_RANDOMWALK_DISTANCE, endPoint );
		float fDistance = VectorDistance( pOwner->s.origin, endPoint );
		if ( fDistance < SIDEKICK_GetRandomWalkDistance( hook ) &&
		     AI_IsGroundBelowBetween( self, self->s.origin, endPoint ) )
		{
			tr = gstate->TraceBox_q2( self->s.origin, mins, maxs, endPoint, self, MASK_SOLID|CONTENTS_MONSTER );
			if ( tr.fraction >= 1.0f )
			{
                VectorMA( self->s.origin, vector, CLOSER_RANDOMWALK_DISTANCE - 24.0f, randomWalkPoint );
                _ASSERTE( randomWalkPoint.Length() > 0.0f );
				return TRUE;
			}
		}
    }

	return FALSE;
}

int SIDEKICK_FindRandomWalkPoint( userEntity_t *self, CVector &randomWalkPoint )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *pOwner = hook->owner;
    _ASSERTE( pOwner );

	CVector mins(self->s.mins.x + 8.0f, self->s.mins.y + 8.0f, self->s.mins.z + 16.0f );
	CVector maxs(self->s.maxs.x - 8.0f, self->s.maxs.y - 8.0f, self->s.maxs.z );

	CVector vector = self->s.origin - pOwner->s.origin;
	vector.z = 0.0f;
	vector.Normalize();

	CVector angleTowardOwner;
	VectorToAngles( vector, angleTowardOwner );
    angleTowardOwner.pitch = 1.0f;

    int bFacingOwner = FALSE;
    CVector angle = self->s.angles;
	float fDistance = VectorDistance( self->s.origin, pOwner->s.origin );
    if ( fDistance < RANDOMWALK_DISTANCE )
    {
        float fYAngleDiff = AngleMod(self->s.angles.yaw - angleTowardOwner.yaw);
        if ( fYAngleDiff < 45.0f || fYAngleDiff > 315.0f )
        {
            bFacingOwner = TRUE;
            angle = angleTowardOwner;
        }
    }

    float fYAngle = angle.yaw;
    int nRandom = rand() % 5;
    if ( nRandom < 4 )
    {
        fYAngle = angle.yaw;
        if ( nRandom == 0 )
        {
            fYAngle = 45.0f;
        }
        else
        if ( nRandom == 1 )
        {
            fYAngle = 90.0f;
        }
        else
        if ( nRandom == 2 )
        {
            fYAngle = 315.0f;
        }
        else
        if ( nRandom == 3 )
        {
            fYAngle = 270.0f;
        }
    }
    else
    {
        nRandom = rand() % 8;
        fYAngle = (float)nRandom * 45.0f;

        if ( fYAngle == 0.0f && bFacingOwner == TRUE )
        {
            if ( rnd() > 0.5f )
            {
                fYAngle = 45.0f;
            }
            else
            {
                fYAngle = 315.0f;
            }
        }
    }

    float fRunFollowDistance = AI_GetRunFollowDistance(hook);
    if ( fDistance >= fRunFollowDistance - 64.0f && pOwner->deadflag == DEAD_NO)
    {
        float fNewYAngle = AngleMod( angle.yaw + fYAngle );
        float fYAngleDiff = AngleMod(fNewYAngle - angleTowardOwner.yaw);
        if ( fYAngleDiff > 150.0f && fYAngleDiff < 210.0f )
        {
            if ( rnd() > 0.5f )
            {
                fYAngle += 135.0f;
            }
            else
            {
                fYAngle += 225.0f;
            }
        }
    }

    angle.yaw = AngleMod( angle.yaw + fYAngle );
	angle.AngleToForwardVector( vector );

	return SIDEKICK_TestRandomWalkPoint( self, mins, maxs, vector, randomWalkPoint );
}


int SIDEKICK_FindRandomWalkPointUsingNodes( userEntity_t *self, CVector &randomWalkPoint )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    userEntity_t *pOwner = hook->owner;
    _ASSERTE( pOwner );

	// update the current node for this entity
	NODELIST_PTR pNodeList = hook->pNodeList;
	_ASSERTE( pNodeList );
	NODEHEADER_PTR pNodeHeader = pNodeList->pNodeHeader;
	_ASSERTE( pNodeHeader );

	MAPNODE_PTR pCurrentNode = NODE_GetClosestNode(self);
	if ( !pCurrentNode )
	{
		if ( pNodeList->nCurrentNodeIndex != -1 )
		{
			pCurrentNode = NODE_GetNode( pNodeHeader, pNodeList->nCurrentNodeIndex );
		}
	}

    int bFound = FALSE;
    if ( pCurrentNode )
    {
	    // find a random node that is within a certain distance from the starting position
	    int nNumLinks = 0;
	    int aNodeIndices[MAX_NODE_LINKS];
	    for ( int i = 0; i < pCurrentNode->nNumLinks; i++ )
	    {
		    MAPNODE_PTR pNode = NODE_GetNode( pNodeHeader, pCurrentNode->aLinks[i].nIndex );

		    float fDistance = VectorDistance( pOwner->s.origin, pNode->position );
		    if ( fDistance < SIDEKICK_GetRandomWalkDistance( hook ) )
		    {
			    aNodeIndices[nNumLinks] = pCurrentNode->aLinks[i].nIndex;
			    nNumLinks++;
		    }
	    }

	    MAPNODE_PTR pDestNode = NULL;
	    if ( nNumLinks > 0 )
	    {
		    srand( (unsigned)time( NULL ) );
   		    int nTargetNodeIndex = aNodeIndices[(rand() % nNumLinks)];

		    pDestNode = NODE_GetNode( pNodeHeader, nTargetNodeIndex );
		    _ASSERTE( pDestNode );

            randomWalkPoint = pDestNode->position;

            bFound = TRUE;
	    }
    }

    return bFound;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_ComputeRandomWalkPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_ComputeRandomWalkPoint( userEntity_t *self, CVector &randomWalkPoint )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    userEntity_t *pOwner = hook->owner;
    _ASSERTE( pOwner );

    if ( !SIDEKICK_FindRandomWalkPoint( self, randomWalkPoint ) )
    {
        return FALSE;
    }

    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StartRandomWalk
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartRandomWalk( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_RANDOMWALK );

    CVector position;
    if ( SIDEKICK_ComputeRandomWalkPoint( self, position ) == FALSE )
    {
        AI_RemoveCurrentTask( self );
        return;
    }
    _ASSERTE( position.Length() > 0.0f );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
    TASK_Set( pCurrentTask, position );

	AI_SetStateSlowWalking( hook );
	if ( AI_StartMove( self ) == FALSE )
	{
		TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_SIDEKICK_RANDOMWALK )
		{
			return;
		}
	}

    AI_Dprintf( "Starting TASKTYPE_SIDEKICK_RANDOMWALK.\n" );

	AI_SetOkToAttackFlag( hook, TRUE );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData );

    _ASSERTE( pAIData->destPoint.Length() > 0.0f );

	float fDistance = VectorDistance( self->s.origin, pAIData->destPoint );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 1.0f;
	AI_SetTaskFinishTime( hook, fTime );

	AI_SetMovingCounter( hook, 0 );

    SIDEKICK_RandomWalk( self );
}

void SIDEKICK_RandomWalk( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	_ASSERTE( pAIData );

	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	if ( AI_IsCloseDistance2( self, fXYDistance ) && fZDistance < 32.0f )
	{
		AI_RemoveCurrentTask( self );
        SIDEKICK_SetNextAmbientTime( self, TASKTYPE_SIDEKICK_RANDOMWALK );
		return;
	}
	
	AI_MoveTowardPoint(self,pAIData->destPoint,TRUE,TRUE);
	
}



// ----------------------------------------------------------------------------
// NSS[12/20/99]:
// Name:		SIDEKICK_StartAnimation
// Description:Generic StartAnimation Function for Sidekicks, will select the right
// animation based off of the current weapon being held.
// Input:userEntity_t *self, char *Buff (starting letters of desired animation)
// Output:0 = failed, 1 = Success
// Note:
// ----------------------------------------------------------------------------
int SIDEKICK_StartAnimation(userEntity_t *self, char *Buff)
{

	char szAnimation[16];
	playerHook_t *hook = AI_GetPlayerHook(self);
    
	if(AI_IsEndAnimation(self) || self->s.frame == 0)
	{
		// NSS[12/20/99]:Trap for crouching.
		if(AI_IsCrouching( hook ))
		{
//			sprintf(szAnimation,"cwalk");
			Com_sprintf(szAnimation,sizeof(szAnimation),"cwalk");
		}
		else
		{
			if(self->waterlevel > 2)
			{
				strcpy( szAnimation, "swim");
			}
			else
			{
				strcpy( szAnimation, Buff );
			}
		}

		AI_SelectAnimationPerWeaponType( self, szAnimation );
		frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
		if ( !pSequence )
		{
			AI_RemoveCurrentTask( self );
			return 0;
		}
		
		AI_ForceSequence( self, pSequence, FRAME_LOOP );
	}
	return 1;
}


// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StartLookAround
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartLookAround( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_LOOKAROUND );

	char szAnimation[16];
    strcpy( szAnimation, "lookaround" );
	AI_SelectAnimationPerWeaponType( self, szAnimation );
	frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
	if ( !pSequence )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	if ( AI_StartSequence( self, pSequence, FRAME_ONCE ) == FALSE )
	{
		return;
	}

    AI_Dprintf( "Starting TASKTYPE_SIDEKICK_LOOKAROUND.\n" );

	AI_SetOkToAttackFlag( hook, TRUE );

    float fAnimationTime = AI_ComputeAnimationTime( pSequence );
	AI_SetTaskFinishTime( hook, fAnimationTime );
    
    AI_SetNextThinkTime( self, 0.1f );
    
}

void SIDEKICK_LookAround( userEntity_t *self )
{
	_ASSERTE( self );

	if ( AI_IsEndAnimation( self ) )
	{
		AI_RemoveCurrentTask( self );
        SIDEKICK_SetNextAmbientTime( self, TASKTYPE_SIDEKICK_LOOKAROUND );
		return;
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StartLookUp
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartLookUp( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_LOOKUP );

	char szAnimation[16];
    strcpy( szAnimation, "lookup" );
	AI_SelectAnimationPerWeaponType( self, szAnimation );
	frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
	if ( !pSequence )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	if ( AI_StartSequence( self, pSequence, FRAME_ONCE ) == FALSE )
	{
		return;
	}

    AI_Dprintf( "Starting TASKTYPE_SIDEKICK_LOOKUP.\n" );

	AI_SetOkToAttackFlag( hook, TRUE );

    float fAnimationTime = AI_ComputeAnimationTime( pSequence );
	AI_SetTaskFinishTime( hook, fAnimationTime );

    AI_SetNextThinkTime( self, 0.1f );
}

void SIDEKICK_LookUp( userEntity_t *self )
{
	_ASSERTE( self );

	if ( AI_IsEndAnimation( self ) )
	{
		AI_RemoveCurrentTask( self );
        SIDEKICK_SetNextAmbientTime( self, TASKTYPE_SIDEKICK_LOOKUP );
		return;
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StartLookDown
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartLookDown( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_LOOKDOWN );

	char szAnimation[16];
    strcpy( szAnimation, "lookdown" );
	AI_SelectAnimationPerWeaponType( self, szAnimation );
	frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
	if ( !pSequence )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	if ( AI_StartSequence( self, pSequence, FRAME_ONCE ) == FALSE )
	{
		return;
	}

    AI_Dprintf( "Starting TASKTYPE_SIDEKICK_LOOKDOWN.\n" );

	AI_SetOkToAttackFlag( hook, TRUE );

    float fAnimationTime = AI_ComputeAnimationTime( pSequence );
	AI_SetTaskFinishTime( hook, fAnimationTime );

    AI_SetNextThinkTime( self, 0.1f );
}

void SIDEKICK_LookDown( userEntity_t *self )
{
	_ASSERTE( self );

	if ( AI_IsEndAnimation( self ) )
	{
		AI_RemoveCurrentTask( self );
        SIDEKICK_SetNextAmbientTime( self, TASKTYPE_SIDEKICK_LOOKDOWN );
		return;
	}
}

// ----------------------------------------------------------------------------
// NSS[12/12/99]:
// Name:        SIDEKICK_StartSaySomething
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartSaySomething( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );
	float distance;

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_SAYSOMETHING );

    GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
    _ASSERTE( pCurrentGoal );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

    if ( pAIData->pString )
    {
        if ( strstr( pAIData->pString, "mp3" ) )
        {        
			// NSS[12/12/99]:Added a cheesy attenuation hack for MP3 stuff.
			distance = 1.0f;
			if(hook)
			{
				userEntity_t *owner = hook->owner;
				if(owner)
				{
					float fDistance = VectorDistance(self->s.origin,owner->s.origin);
					if(fDistance > 512.0f)
					{
						if(fDistance > 1500.0f)
						{
							distance = 0.10f;
						}
						else
						{
							distance = (1500 - fDistance)/1000.0f;
						}
						// NSS[12/9/99]:Limit the min.
						if(distance < 0.10f)
							distance = 0.10f;
					}
				}
			}
			gstate->StartMP3( pAIData->pString, 2, distance, 1 );
        }
        else
        if ( strstr( pAIData->pString, "wav" ) )
        {
	        gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex(pAIData->pString), 
                                      1.0f, hook->fMinAttenuation, hook->fMaxAttenuation );
        }
    }

    AI_Dprintf( "Starting TASKTYPE_SIDEKICK_SAYSOMETHING.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_RemoveCurrentTask( self );
}



// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_SaySomething
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SaySomething( userEntity_t *self )
{
	_ASSERTE( self );	
	
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StartKickSomething
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartKickSomething( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_KICKSOMETHING );

	char szAnimation[16];
    strcpy( szAnimation, "aataka" );
	AI_SelectAnimationPerWeaponType( self, szAnimation );
	frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
	if ( !pSequence )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	if ( AI_StartSequence( self, pSequence, FRAME_ONCE ) == FALSE )
	{
		return;
	}

    AI_Dprintf( "Starting TASKTYPE_SIDEKICK_KICKSOMETHING.\n" );

	AI_SetOkToAttackFlag( hook, TRUE );

    float fAnimationTime = AI_ComputeAnimationTime( pSequence );
	AI_SetTaskFinishTime( hook, fAnimationTime );

    AI_SetNextThinkTime( self, 0.1f );
}

void SIDEKICK_KickSomething( userEntity_t *self )
{
	_ASSERTE( self );

	if ( AI_IsEndAnimation( self ) )
	{
		AI_RemoveCurrentTask( self );
        SIDEKICK_SetNextAmbientTime( self, TASKTYPE_SIDEKICK_KICKSOMETHING );
		return;
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StartWhistle
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartWhistle( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_WHISTLE );

	char szAnimation[16];
    strcpy( szAnimation, "aamb" );
	AI_SelectAnimationPerWeaponType( self, szAnimation );
	frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
	if ( !pSequence )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	if ( AI_StartSequence( self, pSequence, FRAME_ONCE ) == FALSE )
	{
		return;
	}

    AI_Dprintf( "Starting TASKTYPE_SIDEKICK_WHISTLE.\n" );

	AI_SetOkToAttackFlag( hook, TRUE );

    float fAnimationTime = AI_ComputeAnimationTime( pSequence );
	AI_SetTaskFinishTime( hook, fAnimationTime );

    AI_SetNextThinkTime( self, 0.1f );
}

void SIDEKICK_Whistle( userEntity_t *self )
{
	_ASSERTE( self );

	if ( AI_IsEndAnimation( self ) )
	{
		AI_RemoveCurrentTask( self );
        SIDEKICK_SetNextAmbientTime( self, TASKTYPE_SIDEKICK_WHISTLE );
		return;
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StartAnimate
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartAnimate( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_ANIMATE );

	char szAnimation[16];
    strcpy( szAnimation, "aamb" );
	AI_SelectAnimationPerWeaponType( self, szAnimation );
	frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
	if ( !pSequence )
	{
		AI_RemoveCurrentTask( self );
		return;
	}

	if ( AI_StartSequence( self, pSequence, FRAME_ONCE ) == FALSE )
	{
		return;
	}

    AI_Dprintf( "Starting TASKTYPE_SIDEKICK_ANIMATE.\n" );

	AI_SetOkToAttackFlag( hook, TRUE );

    float fAnimationTime = AI_ComputeAnimationTime( pSequence );
	AI_SetTaskFinishTime( hook, fAnimationTime );

    AI_SetNextThinkTime( self, 0.1f );
}

void SIDEKICK_Animate( userEntity_t *self )
{
	_ASSERTE( self );

	if ( AI_IsEndAnimation( self ) )
	{
		AI_RemoveCurrentTask( self );
        SIDEKICK_SetNextAmbientTime( self, TASKTYPE_SIDEKICK_ANIMATE );
		return;
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StartStopGoingFurther
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartStopGoingFurther( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    _ASSERTE( pGoalStack );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_STOPGOINGFURTHER );

	char szAnimation[16];
	AI_SelectAmbientAnimation( self, szAnimation );
	if ( AI_StartSequence( self, szAnimation, FRAME_LOOP ) == FALSE )
	{
		return;
 	}

	AI_Dprintf( "Starting TASKTYPE_SIDEKICK_STOPGOINGFURTHER.\n" );

	AI_ZeroVelocity( self );

	AI_SetNextThinkTime( self, 0.1f );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, -1.0f );
	AI_SetMovingCounter( hook, 0 );
}

void SIDEKICK_StopGoingFurther( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

    frameData_t *pCurrentSequence = hook->cur_sequence;
    if ( !strstr( pCurrentSequence->animation_name, "amb" ) )
    {
        char szAnimation[16];
	    AI_SelectAmbientAnimation( self, szAnimation );
	    AI_ForceSequence( self, szAnimation, FRAME_LOOP );
    }
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StartTeleportAndComeNear
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartTeleportAndComeNear( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_SetOkToAttackFlag( hook, TRUE );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( pGoalStack );
    _ASSERTE( GOALSTACK_GetCurrentTaskType(pGoalStack) == TASKTYPE_SIDEKICK_TELEPORTANDCOMENEAR );

	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal(pGoalStack);
	_ASSERTE( pCurrentGoal );
	GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
	if ( nGoalType == GOALTYPE_SIDEKICK_TELEPORTANDCOMENEAR )
	{
		GOAL_Satisfied( pCurrentGoal );
		AI_RemoveCurrentGoal( self );
	}

}

void SIDEKICK_TeleportAndComeNear( userEntity_t *self )
{
}
int AI_StartMoveForWalkingUnit( userEntity_t *self );

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StartComeHere
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartComeHere( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_SIDEKICK_COMEHERE );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	_ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

	float fDistance = AI_ComputeDistanceToPoint( self, pAIData->destPoint );
    float fRunFollowDistance = AI_GetRunFollowDistance(hook);
    float fWalkFollowDistance = AI_GetWalkFollowDistance(hook);

    //if ( fDistance > fRunFollowDistance && !AI_IsCrouching(hook))
    //{
    //    AI_SetStateRunning( hook );
    //}
    //else
    //{
        AI_SetStateWalking( hook );
    //}
	
    if ( AI_StartMove( self ) == FALSE )
	{
		if ( TASK_GetType( pCurrentTask ) != TASKTYPE_SIDEKICK_COMEHERE )
		{
			return;
		}
	}


	AI_Dprintf( "Starting TASKTYPE_SIDEKICK_COMEHERE.\n" );

	AI_SetNextThinkTime( self, 0.1f );

	AI_SetOkToAttackFlag( hook, TRUE );
	float fSpeed = AI_ComputeMovingSpeed( hook );
	float fTime = (fDistance / fSpeed) + 2.0f;
	AI_SetTaskFinishTime( hook, fTime);
	AI_SetMovingCounter( hook, 0 );

}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_ComeHere
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_ComeHere( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
	
	if(hook && hook->owner && AI_IsAlive(hook->owner))
		pAIData->destPoint = hook->owner->s.origin;
	
	float fXYDistance = VectorXYDistance( self->s.origin, pAIData->destPoint );
	float fZDistance = VectorZDistance( self->s.origin, pAIData->destPoint );
	if ( (AI_IsCloseDistance2( self, fXYDistance )|| fXYDistance < 32.0f) && fZDistance <= 32.0f )
	{
		AI_RemoveCurrentTask( self );
		return;
	}
	
	if(AI_IsEndAnimation(self))
	{
		float fDistance = AI_ComputeDistanceToPoint( self, pAIData->destPoint );
		float fRunFollowDistance = AI_GetRunFollowDistance(hook);
		float fWalkFollowDistance = AI_GetWalkFollowDistance(hook);

		if ( fDistance > fRunFollowDistance && !AI_IsCrouching(hook))
		{
			AI_SetStateRunning( hook );
		}
		else
		{
			AI_SetStateWalking( hook );
		}
		
		AI_StartMoveForWalkingUnit( self );
	}

	if ( AI_IsOkToMoveStraight( self, pAIData->destPoint, fXYDistance, fZDistance ) )
	{
		AI_MoveTowardPoint( self, pAIData->destPoint, FALSE );
	}
	else
	{
		if ( !AI_HandleUse( self ) )
		{
			if ( hook->pPathList->pPath )
			{
				if ( AI_Move( self ) == FALSE )
				{
					if ( !AI_FindPathToPoint( self, pAIData->destPoint ) )
					{
						AI_RestartCurrentGoal( self );
					}
				}					
			}
			else
			{
				if ( !AI_FindPathToPoint( self, pAIData->destPoint ) )
				{
					AI_RestartCurrentGoal( self );
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
// NSS[1/27/00]:
// Name:		SIDEKICK_FindClearSpotFrom
// Description: Finds a clear point from the point fromPoint... er.. yeah.
// Input:userEntity_t *self,CVector &fromPoint, CVector &clearPoint
// Output:TRUE/FALSE
// Note:
// ----------------------------------------------------------------------------
#define CLEAR_POINT_DISTANCE    64.0f

int SIDEKICK_FindClearSpotFrom( userEntity_t *self,CVector &fromPoint, CVector &clearPoint)
{
	playerHook_t *hook	= AI_GetPlayerHook(self);
	userEntity_t *owner =  hook->owner;
	// search the eight directions around the self to find a spawn point
	float delta_angle = 0.0f;
	float distance = 128.0f;
	CVector angle = self->s.angles;
	angle.pitch = -5.0f;


	CVector vector,Dir,bottomPoint,MidPoint;
	angle.AngleToForwardVector( vector );
	CVector endPoint;

	tr = gstate->TraceBox_q2(fromPoint , self->s.mins, self->s.maxs, fromPoint, self, self->clipmask );

	if ( tr.fraction >= 1.0f && !tr.allsolid && !tr.startsolid)
	{
		clearPoint = fromPoint;
		//If we made it then cool.
		clearPoint.z += 6.0f;
		return TRUE;
	}

	
	while(delta_angle < 360)
	{
		while(distance > 32.0f)
		{
			VectorMA( fromPoint, vector, distance, endPoint );
			endPoint.z += 2.0f;
			//if ( AI_IsGroundBelowBetween( self, fromPoint, endPoint,32.0 ) ||  (owner && owner->waterlevel > 2))
			//{
				endPoint.z += 6.0f;
				bottomPoint = endPoint;
				//Get a new midway point
				Dir = endPoint - fromPoint;
				Dir.Normalize();
				MidPoint = fromPoint + (Dir*(distance*0.75f));
				//trace from mid to new destination
				tr = gstate->TraceBox_q2(MidPoint , self->s.mins, self->s.maxs, endPoint, self, self->clipmask );

				if ( tr.fraction >= 1.0f && !tr.allsolid && !tr.startsolid)
				{
					//If we made it then cool.
					clearPoint = endPoint;
					return TRUE;
				}
				if(tr.ent)
				{
					AI_Dprintf("Collision With:%s",tr.ent->className);
				}
			//}
			//slowly decrement the distance
			distance -= 8.0f;
		}
		delta_angle += 45.0f;
		angle.yaw = AngleMod( angle.yaw + delta_angle );
		angle.AngleToForwardVector( vector );

		//reset the distance for the next angle
		distance = 128.0f;
	}
	
	return FALSE;
}


// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_StartTeleport
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StartTeleport( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	AI_SetOkToAttackFlag( hook, TRUE );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	_ASSERTE( GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_SIDEKICK_TELEPORT );

	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
    _ASSERTE( pCurrentTask );
	_ASSERTE( TASK_GetType( pCurrentTask ) == TASKTYPE_SIDEKICK_TELEPORT );

	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);
    
    CVector clearPoint;
    SIDEKICK_FindClearSpotFrom( self, pAIData->destPoint, clearPoint );

    self->s.origin = clearPoint;

	gstate->LinkEntity( self );

    AI_RemoveCurrentTask( self );
}

void SIDEKICK_Teleport( userEntity_t *self )
{
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_HandlePlayerSelectingSidekick
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_HandlePlayerSelectingSidekick( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
    
	if ( (hook->nTargetCounter % 2) == 0 )
    {
        // if there are two sidekicks, make the one that the player is pointing
        // at more noticeable than the other one, 
        // this is to differentiate which one the player is communicating with
        int bFullBright = FALSE;
        userEntity_t *pOwner = hook->owner;
        if ( AI_IsAlive( pOwner ) )
        {
            float fDistance = VectorDistance( self->s.origin, pOwner->s.origin );
            if ( fDistance < 512.0f && SIDEKICK_IsOwnerTargetingMe( self, pOwner ) )
            {
                AIINFO_SetLastSelectedSidekick( self );
                bFullBright = TRUE;
            }
        }

        if ( bFullBright )
        {
            self->s.renderfx |= RF_FULLBRIGHT;
        }
        else
        {
            self->s.renderfx &= ~RF_FULLBRIGHT;
        }
    }

}

// ----------------------------------------------------------------------------
// NSS[1/31/00]:
// Name:        PlaySidekickMP3
// Description: New Mp3 Playing routine to deal with the ultimate Cheese hack
// Input:userEntity_t *self, int Index
// Output:NA
// Note:See the defined structure information in sidekick.h and also the defines 
// at the top of this file.  
// ----------------------------------------------------------------------------
void PlaySidekickMP3(userEntity_t *self,char *sound)
{
	int Chan;
	float distance = 1.0f;
	userEntity_t *Sidekick;

	if(sound != NULL)
	{
		if(strstr(sound,"superfly") || strstr(sound,"mikikofly"))
		{
			Chan = 4;
			Sidekick =	AIINFO_GetSuperfly();
			// NSS[3/12/00]:Keep a marker on the last time they have spoken
			SuperflyLastTalk = gstate->time + 10.0f;
		}
		else
		{
			Chan = 3;
			Sidekick =	AIINFO_GetMikiko();
			// NSS[3/12/00]:Keep a marker on the last time they have spoken
			MikikoLastTalk = gstate->time + 10.0f;
		}
	}
	else
	{
		return;
	}
	
	playerHook_t *hook	= AI_GetPlayerHook( Sidekick );
	// NSS[2/17/00]:Testy testy
	if(!AI_IsSidekick(hook))
		return;
	// NSS[12/9/99]:Get the distance from the owner to give the effect of 'distance' in the volume since mp3's don't play sounds based off of their origin.
	if(hook != NULL)
	{
		userEntity_t *owner = hook->owner;
		if(owner != NULL)
		{
			// NSS[3/10/00]:If the owner is not alive then we want to here the sounds
			if(AI_IsAlive(owner))
			{
				float fDistance = VectorDistance(Sidekick->s.origin,owner->s.origin);
				if(fDistance > 164.0f)
				{
					if(fDistance >= 800.0f)
					{
						if(!gstate->inPHS(Sidekick->s.origin,hook->owner->s.origin))
							return;
						else
							distance = 0.10f;
					}
					else
					{
						// NSS[2/12/00]:If we are far away and cannot see owner then we cannot hear the owner
						if(fDistance > 300 && !gstate->inPHS(Sidekick->s.origin,hook->owner->s.origin))
							return;
						else
							distance = (1000 - fDistance)/1000.0f;
					}

					
					// NSS[12/9/99]:Limit the min.
					if(distance < 0.10f)
						distance = 0.10f;
				}
			}
		}
		else
		{
			return;
		}
	}
	
	if(!gstate->bCinematicPlaying)		
	{
		gstate->StopMP3(Chan);
		gstate->StartMP3(sound, Chan, distance, 1 );
	}
}




// ----------------------------------------------------------------------------
// NSS[1/31/00]:
// Name:        PlaySidekickVoice
// Description: Had to rewrite this again... this is a litle more modular now.
// Input:userEntity_t *self, int Index
// Output:NA
// Note:See the defined structure information in sidekick.h and also the defines 
// at the top of this file.  
// ----------------------------------------------------------------------------
void PlaySidekickVoice(userEntity_t *self,int Index, int VoiceType)
{
	char Buffer[32];
	char Sound[128];
	int Chan;
	int MP3 = 0;
	float distance = 1.0f;
	SideMp3_t VoiceIndex;
	playerHook_t *hook	= AI_GetPlayerHook( self );
	
	// NSS[12/9/99]:Get the distance from the owner to give the effect of 'distance' in the volume since mp3's don't play sounds based off of their origin.
	if(hook)
	{
		userEntity_t *owner = hook->owner;
		if(owner)
		{
			float fDistance = VectorDistance(self->s.origin,owner->s.origin);
			if(fDistance > 164.0f)
			{
				if(fDistance >= 800.0f)
				{
					if(!gstate->inPHS(self->s.origin,hook->owner->s.origin))
						return;
					else
						distance = 0.30f;
				}
				else
				{
					// NSS[2/12/00]:If we are far away and cannot see owner then we cannot hear the owner
					if(fDistance > 300 && !gstate->inPHS(self->s.origin,hook->owner->s.origin))
						return;
					else
						distance = (1000 - fDistance)/1000.0f;
				}

				
				// NSS[12/9/99]:Limit the min.
				if(distance < 0.30f)
					distance = 0.30f;

			}
		}
	}


	if(hook->type == TYPE_MIKIKO)
	{
		//NSS[12/4/99]:Setup the index reference
		VoiceIndex = MikikoVoices[Index];
		// NSS[1/31/00]:Added Wav capability
		if(strstr(VoiceIndex.File,".wav"))
			Com_sprintf(Buffer,sizeof(Buffer),"mikiko/");
		else
		{
			MP3 = 1;
			Com_sprintf(Buffer,sizeof(Buffer),"sounds/voices/mikiko/");
		}
		Chan = 2;
		// NSS[3/12/00]:Keep a marker on the last time they have spoken
		MikikoLastTalk = gstate->time + VoiceIndex.delay;
	}
	else
	{
		//NSS[12/4/99]:Setup the index reference
		VoiceIndex = SuperFlyVoices[Index];
		// NSS[1/31/00]:Added Wav capability
		if(strstr(VoiceIndex.File,".wav"))
			Com_sprintf(Buffer,sizeof(Buffer),"superfly/");
		else
		{
			MP3 = 1;
			Com_sprintf(Buffer,sizeof(Buffer),"sounds/voices/superfly/");
		}
		Chan = 3;
		// NSS[3/12/00]:Keep a marker on the last time they have spoken
		SuperflyLastTalk = gstate->time + VoiceIndex.delay;
	}
	
	//NSS[12/4/99]:Combine the two
	Com_sprintf(Sound,sizeof(Sound),"%s%s",Buffer,VoiceIndex.File);

	if(Sound[0] != NULL && !gstate->bCinematicPlaying)		//NSS[12/3/99]:If we have a sound and a cinematic is not playing then go for it.
	{
		if(MP3)
		{
			gstate->StopMP3(Chan);
			gstate->StartMP3(Sound, Chan, distance, 1 );
		}
		else
		{
			gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex(Sound), 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
		}
		VoiceIndex.last_time_used = gstate->time;			// NSS[12/4/99]:Keep track of the last time we played this sound.
	}
}

// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        SideKick_TalkAmbient
// Description: Uhh.. a section of ambient stuff to say... no real method to 
// this madness..
// Input:userEntity_t *self, int Tasktype
// Output:Returns the Index played.
// Note:Parse through either Mikiko or Superfly's ambient Voice sounds and 
// and determine which one will be best to play.
// OverRideTime specifies that they are answering a question or need to say something
// ----------------------------------------------------------------------------
static SideMp3_t List[10];					//NSS[12/4/99]:Create 5 slots to possibly be able to fill
static int Indexes [10];
int SideKick_TalkAmbient(userEntity_t *self, enum Sidekick_Talk VoiceType, int OverRide_Time)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	SideMp3_t *VoiceIndex;
	int i = 0;
	int index_found = 0;				//NSS[12/4/99]:How many we have found of this type
	int Best_Index	= 0;				//NSS[12/4/99]:Best index found.
	float Last_Time = 0;				//NSS[12/4/99]:Keep track of time... 
	
	memset(&List,0,sizeof(List));
	memset(&Indexes,0,sizeof(Indexes));

	if(hook->type == TYPE_MIKIKO)		//NSS[12/4/99]:Get the pointer to the Voices
	{
		 VoiceIndex = MikikoVoices;
	}
	else
	{
		VoiceIndex  = SuperFlyVoices;
	}
	
	//NSS[12/4/99]:Run through the voices until we find the one that matches the Voice Type
	while( VoiceIndex[i].SoundID != 0)
	{
		if(VoiceIndex[i].SoundID == VoiceType)
		{
			if(index_found < 10)
			{
				List[index_found]	= VoiceIndex[i];
				Indexes[index_found]= i;
				index_found++;
			}
			else
			{
				break;//Theoretically we should never get here unless someone jacks with the lists
			}
		}
		i++;
	}

	//NSS[12/4/99]:We have accumulated the Array of possible things to say now process it.
	switch(VoiceType)
	{
		// NSS[2/2/00]:Randomly choose one depending on how many there are.
		/*case EVADE:
		case STARTING_ATTACK:
		case NO:
		case CANTREACH:
		case IMPATIENT:
		case JUMP:
		case LANDING:
		{
			int Number = rnd()*index_found;
			Best_Index = Indexes[Number];
			break;
		}*/
		case HEALTH_RESPONSE1:
		case HEALTH_RESPONSE2:
		case HEALTH_RESPONSE3:
		case HEALTH_RESPONSE4:
		case HEALTH_RESPONSE5:
		{
			Best_Index = Indexes[index_found-1];
			break;
		}
		default:
		{
			int Number = rnd()*index_found;
			Best_Index = Indexes[Number];
			break;		
		}
		/*default:
		{
			for(i = 0; i < index_found; i++)
			{
				if(List[i].last_time_used >= Last_Time)
				{
					Best_Index = Indexes[i];						// NSS[12/4/99]:
					Last_Time = List[Best_Index].last_time_used;	// NSS[12/4/99]:
				}
			}		
		}*/
	}

	// NSS[12/4/99]:Now play it.. nice and simple.
	PlaySidekickVoice(self,Best_Index,VoiceType);




	return Best_Index;
}

// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        SIDEKICK_SendMessage
// Description:This allows the sidekicks to talk to one another
// Input:	userEntity_t *other, int command, int An_Int, float A_Float
// Output:
// Note:
//Other --> person getting the message.
// ----------------------------------------------------------------------------
int SIDEKICK_SendMessage(userEntity_t *other, int command, int An_Int, float A_Float, float B_Float, userEntity_t *sender,int Flag)
{
	int i;
	float time;
	SideKickCom_t *Kick_Com = NULL;
	SideKickCom_t *ParseCom = NULL;
	playerHook_t *hook = AI_GetPlayerHook( other );
	
	if( hook == NULL )
	{
		return 0;
	}

	if(hook->type == TYPE_MIKIKO)
	{
		ParseCom = MikikoCom;
	}
	else
	{
		ParseCom = SuperflyCom;
	}
	
	// NSS[12/4/99]:Get our point of reference
	time = gstate->time;
	
	// NSS[3/12/00]:First check to see if we have this message in the Queue...
	for(i = 0; i< MAX_SIDEKICK_MESSAGES;i++)
	{
		if(ParseCom[i].aInt ==  An_Int && ParseCom[i].aFloat > gstate->time)
		{
			return 0;
		}
		else if(ParseCom[i].aInt ==  An_Int && ParseCom[i].aFloat < gstate->time)
		{
			Kick_Com = &ParseCom[i];
			break;
		}
	}

	// NSS[3/12/00]:Next if we have made it this far, find an open spot.
	if(Kick_Com == NULL)
	{
		for(i = 0; i< MAX_SIDEKICK_MESSAGES;i++)
		{
			if(ParseCom[i].aFloat < gstate->time)
			{
				Kick_Com = &ParseCom[i];
				break;
			}
		}
	}


	if(!Kick_Com)
	{
		return 0;	
	}
	else
	{
		Kick_Com->bFloat		= B_Float;
		Kick_Com->aFloat		= A_Float;
		Kick_Com->aInt			= An_Int;
		Kick_Com->command		= command;
		Kick_Com->Location		= sender->s.origin;
		Kick_Com->flags			= Flag;
		Kick_Com->TimeStamp		= gstate->time;
	}
	return 1;
}



// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        Check_Que
// Description:This will check the message Que and determine if a message has been sent.
// in too short a period of time ago... 
// Input:userEntity_t *Other,int Type, float Offset
// Output:Int (Number of occurances found
// Note:
// If the offset has a value it will find messages of the same type and determine
// if it has been long enough to play it again.
// ----------------------------------------------------------------------------
int Check_Que(userEntity_t *other,int Type, float Offset)
{
	int i;
	int Number = 0;
	SideKickCom_t *Kick_Com;
	SideKickCom_t *Kick_Com_It;
	playerHook_t *hook = AI_GetPlayerHook( other );
	if(!hook)
		return 1;
	if(hook->type == TYPE_MIKIKO)
	{
		Kick_Com = MikikoCom;
		// NSS[3/12/00]:If superfly has just said something let's not talk...
		if(SuperflyLastTalk > gstate->time && Type != NO_TALK && Type != JUMP && Type != LANDING)
			return 1;
	}
	else
	{
		Kick_Com = SuperflyCom;
		if(MikikoLastTalk > gstate->time && Type != NO_TALK && Type != JUMP && Type != LANDING)
			return 1;
	}
	
	for(i = 0 ; i < MAX_SIDEKICK_MESSAGES; i++)
	{
		Kick_Com_It = &Kick_Com[i];
		if(Type != SPECIAL_TALK)
		{
			if(Offset)
			{
				// NSS[1/31/00]:Check to see if a message has been in the Queue for longer than Offset clicks.
				if(Kick_Com_It->aInt == Type && ((Kick_Com_It->aFloat+Offset) > gstate->time) )
				{
					Number++;
				}
			}
			else
			{
				// NSS[12/5/99]:Generic Check
				if(Kick_Com_It->aInt == Type )
				{
					Number++;
				}		
			}
			// NSS[1/31/00]:Generally, if we have something in here that is fairly new let's not speak at all.
			if(Kick_Com_It->TimeStamp <= gstate->time)
			{
				if((gstate->time - Kick_Com_It->TimeStamp) < 4.0f)
				{
					Number++;
				}
			}
		}
		else// NSS[1/31/00]:Handle Sidekick's special talking
		{
			if(Kick_Com_It->aFloat+Offset > gstate->time)
			{
				Number++;
			}			
		}
	}

	return Number;
}


// ----------------------------------------------------------------------------
// NSS[12/16/99]:
// Name:        SIDEKICK_FaceSomethingReal
// Description: Turns to face something that is something other than a wall
// Input: userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_FaceSomethingReal(userEntity_t *self)
{
	CVector Dir,Angles;

	float Roll_Max = 15.0f;
	float Pitch_Max = 20.0f;
	
	playerHook_t *hook = AI_GetPlayerHook( self );

	AngleToVectors(self->s.angles,Dir);
	Dir.Normalize();
	
	Dir = Dir * 128.0f;
	tr = gstate->TraceLine_q2( self->s.origin, Dir, self, MASK_SOLID);

	if(!SIDEKICK_HasTaskInQue(self,TASKTYPE_SIDEKICK_ATTACK))
	{
		//This means we are facing a wall...or some other very un-interesting cock let's not do this.
		if(tr.fraction < 1.0f && !SIDEKICK_HasTaskInQue(self,TASKTYPE_SIDEKICK_ATTACK))
		{
			// NSS[12/16/99]:If we have an owner let's face him.
			if(hook->owner )
			{
				Dir = hook->owner->s.origin - self->s.origin;
				Dir.Normalize();
				VectorToAngles(Dir,Angles);
				
				if(Angles.roll > Roll_Max || Angles.roll < (-1*Roll_Max))
				{
					if(Angles.roll > 0)
						Angles.roll = Roll_Max;
					else
						Angles.roll = (-1 * Roll_Max);
				}
				if(Angles.pitch > Pitch_Max || Angles.pitch > (-1 * Pitch_Max))
				{
					if(Angles.pitch > 0)
						Angles.pitch = Pitch_Max;
					else
						Angles.pitch = (-1 * Pitch_Max);				
				}

				self->s.angles.yaw = Angles.yaw;

			}
			else
			{
				self->s.angles.yaw +=15.0f;
			}
		}
	}
}

// ----------------------------------------------------------------------------
// NSS[12/6/99]:
// Name:        SIDEKICK_HandleAmbientTasks
// Description: Handles ambient tasks... kahduh... 
// Input: userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_HandleAmbientTasks( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	userEntity_t *pOtherSidekick = SIDEKICK_GetOtherSidekick( self );
    int bDoneSidekickThink = FALSE;

	if ( pCurrentGoal )
    {
        TASK_PTR pCurrentTask = GOAL_GetCurrentTask( pCurrentGoal );
        if ( pCurrentTask )
        {
	        CVector lastPosition = self->s.origin;
            TASKTYPE nCurrentTaskType = TASK_GetType( pCurrentTask );
            if ( SIDEKICK_IsAmbientTask( nCurrentTaskType ) && 
                 SIDEKICK_HandleFollowing( self ) == FALSE )
            {
                bDoneSidekickThink = TRUE;
                switch ( nCurrentTaskType )
	            {
                    case TASKTYPE_SIDEKICK_RANDOMWALK:
		            {
			            SIDEKICK_RandomWalk( self );
                        AI_SetNextThinkTime( self, 0.1f );
			            
						if(!Check_Que(self,RANDOM_WALK,15.0f) && (rnd() < 0.35f) && hook->owner && AI_IsAlive(hook->owner))
						{
								if(SideKick_TalkAmbient(self,RANDOM_WALK,1))
								{
									// NSS[12/6/99]:We send a message to both sidekicks so we are not saying the same shit at the same time.
									SIDEKICK_SendMessage(self, sCom_Speak,RANDOM_WALK,gstate->time+5.0f,0.0f,self,COM_Read);
									if(pOtherSidekick)
									{
										SIDEKICK_SendMessage(pOtherSidekick, sCom_Speak,RANDOM_WALK,gstate->time+10.0f,0.0f,self,COM_Read);
									}
								}
						}
						else
						{
							SIDEKICK_SendMessage(self, sCom_Speak,RANDOM_WALK,gstate->time+15.0f,0.0f,self,COM_Read);
						}
						break;
					}
                    case TASKTYPE_SIDEKICK_LOOKAROUND:
		            {			            
						if(!Check_Que(self,LOOKING_SIDEWAYS,30.0f) && (rnd() < 0.15f)&& hook->owner && AI_IsAlive(hook->owner))
						{
							SIDEKICK_FaceSomethingReal(self);
							self->velocity.Zero();
							if(rnd() < 0.25f)
							{
								if(SideKick_TalkAmbient(self,LOOKING_SIDEWAYS,1))
								{
									// NSS[12/6/99]:We send a message to both sidekicks so we are not saying the same shit at the same time.
									SIDEKICK_SendMessage(self, sCom_Speak,LOOKING_SIDEWAYS,gstate->time+30.0f,0.0f,self,COM_Read);
									if(pOtherSidekick)
									{
										SIDEKICK_SendMessage(pOtherSidekick, sCom_Speak,LOOKING_SIDEWAYS,gstate->time+30.0f,0.0f,self,COM_Read);
									}
								}
							}
							else
							{
								SIDEKICK_SendMessage(self, sCom_Speak,LOOKING_SIDEWAYS,gstate->time+30.0f,0.0f,self,COM_Read);
							}
						}
						SIDEKICK_LookAround( self );
                        AI_SetNextThinkTime( self, 0.1f );
			            break;
		            }
                    case TASKTYPE_SIDEKICK_LOOKUP:
		            {
			            SIDEKICK_FaceSomethingReal(self);
						self->velocity.Zero();
						if(!Check_Que(self,LOOKING_UP,30.0f) && (rnd() < 0.15f)&& hook->owner && AI_IsAlive(hook->owner))
						{
							if(SideKick_TalkAmbient(self,LOOKING_UP,0))
							{
								// NSS[12/6/99]:We send a message to both sidekicks so we are not saying the same shit at the same time.
								SIDEKICK_SendMessage(self, sCom_Speak,LOOKING_UP,gstate->time+30.0f,0.0f,self,COM_Read);
								if(pOtherSidekick)
								{
									SIDEKICK_SendMessage(pOtherSidekick, sCom_Speak,LOOKING_UP,gstate->time+30.0f,0.0f,self,COM_Read);
								}
							}
						}
						SIDEKICK_LookUp( self );
                        AI_SetNextThinkTime( self, 0.1f );
			            break;
		            }
                    case TASKTYPE_SIDEKICK_LOOKDOWN:
		            {
			            SIDEKICK_FaceSomethingReal(self);
						self->velocity.Zero();
						SIDEKICK_LookDown( self );
                        AI_SetNextThinkTime( self, 0.1f );
			            break;
		            }
                    case TASKTYPE_SIDEKICK_SAYSOMETHING:
		            {
			            
						SIDEKICK_SaySomething( self );
						self->velocity.Zero();
                        AI_SetNextThinkTime( self, 0.1f );
			            break;
		            }
                    case TASKTYPE_SIDEKICK_KICKSOMETHING:
		            {
			            SIDEKICK_FaceSomethingReal(self);
						self->velocity.Zero();
						SIDEKICK_KickSomething( self );
                        AI_SetNextThinkTime( self, 0.1f );
			            break;
		            }
                    case TASKTYPE_SIDEKICK_WHISTLE:
		            {
			            SIDEKICK_Whistle( self );
						self->velocity.Zero();
                        AI_SetNextThinkTime( self, 0.1f );
			            break;
		            }
                    case TASKTYPE_SIDEKICK_ANIMATE:
		            {
			            SIDEKICK_FaceSomethingReal(self);
						self->velocity.Zero();
						SIDEKICK_Animate( self );
                        AI_SetNextThinkTime( self, 0.1f );
			            break;
		            }
                    default:
                    {
                        _ASSERTE( FALSE );
                        break;
                    }
                }

                AI_HandleTaskFinishTime( self, nCurrentTaskType, lastPosition );
            }
            else
            {
                
            }
        }
    }

    return bDoneSidekickThink;
}


// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        SIDEKICK_CheckMessage
// Description:Does what it sounds like... checks the messages for sidekick type self.
// Input:	userEntity_t *self
// Output:	NA
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_CheckMessage(userEntity_t *self)
{
	enum Sidekick_Talk Message_Type;
	SideKickCom_t	*Kick_Com;
	SideKickCom_t	*Kick_Com_It;
	SideMp3_t		*Voices;
	playerHook_t *hook = AI_GetPlayerHook( self );
	
	int i,Index;
	float Delay = 0.0f;
	Index = -1;
	
	if(hook->type == TYPE_MIKIKO)
	{
		Kick_Com	= MikikoCom;
		Voices		= MikikoVoices;
	}
	else
	{
		Kick_Com	= SuperflyCom;
		Voices		= SuperFlyVoices;
	}
	Message_Type = NO_TALK;
	for(i=0;i < MAX_SIDEKICK_MESSAGES;i++)
	{
		Kick_Com_It = &Kick_Com[i];
		// NSS[12/4/99]:Unread message.
		if(Kick_Com_It->flags == COM_Unread)
		{
			switch(Kick_Com_It->command)
			{
				case sCom_Speak:
				{
					if(Kick_Com_It->aFloat < gstate->time)
					{
						switch(Kick_Com_It->aInt)
						{
							case QUESTION_HEALTH:
							{
								if(self->health >= 80)
									Message_Type = HEALTH_RESPONSE1;
								else if(self->health < 80 && self->health >= 60)
									Message_Type = HEALTH_RESPONSE2;
								else if(self->health < 60 && self->health >= 40)
									Message_Type = HEALTH_RESPONSE3;
								else if(self->health < 40 && self->health >= 20)
									Message_Type = HEALTH_RESPONSE4;
								else
									Message_Type = HEALTH_RESPONSE5;
								break;
							}
							case QUESTION_1:
							{
								Message_Type = ANSWER_1;
								break;
							}
							case QUESTION_2:
							{
								Message_Type = ANSWER_2;
								break;
							}
							// NSS[12/5/99]:Sync Commands go below this point
							case COM_WAIT:
							{
								AI_AddNewTaskAtFront(self,TASKTYPE_WAIT,Kick_Com_It->aFloat);
								break;
							}
							default:
							{
								Message_Type = (Sidekick_Talk)Kick_Com_It->aInt;
							}
						}
						// NSS[12/5/99]:If we have a message to say then do so.
						if(Message_Type)
						{
							Index = SideKick_TalkAmbient(self, Message_Type,1);
						}
						// NSS[12/5/99]:Mark the message as being read.
						Kick_Com_It->flags = COM_Read;
						// NSS[3/12/00]: Break out of this loop so we don't continue to say things.
						break;
					}
				}
			}
		}

		// NSS[3/12/00]:If we played something let's push back anything else waiting to be played.		
		if(Index != -1)
		{
			Delay = Voices[Index].delay;
			// NSS[3/12/00]:Now delay everything else based off of the delay of the message just sent.
			for(i=0;i < MAX_SIDEKICK_MESSAGES;i++)
			{
				Kick_Com_It = &Kick_Com[i];
				// NSS[12/4/99]:Unread message.
				if(Kick_Com_It->flags == COM_Unread)
				{
					if(Kick_Com_It->aFloat <= (gstate->time + Delay))
					{
						// NSS[3/12/00]:Add the time the MP3 will take to play plus 2 seconds for safety purposes.
						Kick_Com_It->aFloat = gstate->time + Delay + 2.0f;
					}
				}
			}
		}

		if(Kick_Com_It->flags == COM_Read)
		{
			// NSS[12/5/99]:Message timed out and should be removed
			if((gstate->time - Kick_Com_It->aFloat) > 240.0f)
			{
				Kick_Com_It->bFloat			= 0.0f;
				Kick_Com_It->aFloat			= 0.0f;
				Kick_Com_It->aInt			= 0;
				Kick_Com_It->command		= 0;
				Kick_Com_It->Location		= NULL;
				Kick_Com_It->flags			= 0;
				Kick_Com_It->TimeStamp		= 0.0f;
			}

		}
	}
}






//void SideKick_TalkAmbient(userEntity_t *self, int VoiceType);
// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        SIDEKICK_ChitChat
// Description: The main segment for conversation between Mikiko and SuperFly
// Input: userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_ChitChat(userEntity_t * self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	userEntity_t *pOtherSidekick = SIDEKICK_GetOtherSidekick( self );
	
	if(self && AI_IsVisible(self,hook->owner))
	{
		if(self->hacks*0.25 < gstate->time) //We have this look at the time thing a tad bit earlier.  We might not have to have this
		{
			if(pOtherSidekick)
			{
				// NSS[12/4/99]:First one to check is the Health question. Added in that they have to be close to one another
				if(pOtherSidekick->health < 100)
					if(!Check_Que(pOtherSidekick,QUESTION_HEALTH,1000.0f) ) 
						if( VectorDistance(self->s.origin,pOtherSidekick->s.origin) < 175.0f)
							if(!Check_Que(self,LOW_HEALTH,60.0f))
								if(!Check_Que(self,QUESTION_HEALTH,70.0f))
									if(rnd() < 0.25f)
									{
										if(SIDEKICK_SendMessage(pOtherSidekick, sCom_Speak,QUESTION_HEALTH,(gstate->time+2.0f),pOtherSidekick->health,self,COM_Unread))
										{
											SideKick_TalkAmbient(self, QUESTION_HEALTH,1);
											SIDEKICK_SendMessage(self, sCom_Speak,QUESTION_HEALTH,gstate->time+180.0f,0.0f,self,COM_Read);
										}
										return;
									}
									else
									{
										SIDEKICK_SendMessage(self, sCom_Speak,QUESTION_HEALTH,gstate->time+180,0.0f,self,COM_Read);
									}

				// NSS[12/5/99]:Really far away conversation doesn't take place too often.
				float RandomNumber = rnd();
				if(hook->owner)
				{
					if(VectorDistance(self->s.origin,hook->owner->s.origin) > 1000.0f && VectorDistance(self->s.origin,pOtherSidekick->s.origin) < 200.0f && AI_IsVisible(self,hook->owner))
					{
						if(!(Check_Que(pOtherSidekick,QUESTION_1,512.0f)) && !(Check_Que(self,QUESTION_1,512.0f) && (RandomNumber < 0.03f)))
						{
							if(SIDEKICK_SendMessage(pOtherSidekick, sCom_Speak,QUESTION_1,(gstate->time+4.0f),500.0f,self,COM_Unread))
							{
								SideKick_TalkAmbient(self, QUESTION_1,1);
								SIDEKICK_SendMessage(self, sCom_Speak,QUESTION_1,gstate->time+512.0f,0.0f,self,COM_Read);
								return;					
							}
						}
						else if(!Check_Que(self,QUESTION_1,512))
						{
							SIDEKICK_SendMessage(self, sCom_Speak,QUESTION_1,gstate->time+512.0f,500.0f,self,COM_Read);
						}

						if(!(Check_Que(pOtherSidekick,QUESTION_2,512)) && !(Check_Que(pOtherSidekick,QUESTION_1,512)) && !(Check_Que(self,QUESTION_1,512)) && hook->type == TYPE_MIKIKO && (RandomNumber < 0.03f))
						{
							if(SIDEKICK_SendMessage(pOtherSidekick, sCom_Speak,QUESTION_2,(gstate->time+4.0),500.0f,self,COM_Unread))
							{
								SideKick_TalkAmbient(self, QUESTION_2,1);
								SIDEKICK_SendMessage(self, sCom_Speak,QUESTION_2,gstate->time+512.0f,500.0f,self,COM_Read);
								return;
							}
						}
						else if(hook->type == TYPE_MIKIKO  && !Check_Que(self,QUESTION_2,512.0f))
						{
							SIDEKICK_SendMessage(self, sCom_Speak,QUESTION_2,gstate->time+512,0.0f,self,COM_Read);
						}
					}
				}
			}
			// NSS[12/5/99]:Sidekicks talking to Hiro about health goes here.
			if(hook->owner)
			{
				// NSS[12/5/99]:Health related conversation goes here...
				if(hook->owner->health < 100 && (rnd() < 0.10f))
				{
							
				}
			}
		}
	}
}


// ----------------------------------------------------------------------------
// NSS[12/9/99]:
// Name:        SIDEKICK_HandleDeadOwner
// Description:Handle when the owner dies... 
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_HandleDeadOwner(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);

	if(hook)
	{
		if(hook->owner)
		{
			userEntity_t *other = SIDEKICK_GetOtherSidekick(self);
			if(!AI_IsAlive(hook->owner) && AI_IsSuperfly(hook))
			{
				if(other && !AI_IsAlive(other))
					return 0;
				if(!Check_Que(self,HIRO_DIES,0.0f))
				{
					SideKick_TalkAmbient(self, HIRO_DIES,1);
					SIDEKICK_SendMessage(self, sCom_Speak,HIRO_DIES,gstate->time+10000.0f,0.0f,self,COM_Read);
				}
			}
		}
	}
	return 0;
}


// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        SIDEKICK_StartStop
// Description: Heh... always wanted to create a function named this. 
// Input:	userEntity_t *self
// Output:  na
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_StartStop(userEntity_t *self)
{
	char szAnimation[16];
	playerHook_t *hook = AI_GetPlayerHook( self );
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);

    
	AI_SelectAmbientAnimation( self, szAnimation );
    frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
    if(hook)
	{
		if( pSequence != NULL && hook->cur_sequence && hook->cur_sequence->animation_name && !strstr(hook->cur_sequence->animation_name,"amb"))
		{
			AI_ForceSequence(self, szAnimation, pSequence->flags );
		}
		else if(hook->cur_sequence && hook->cur_sequence->animation_name && strstr(hook->cur_sequence->animation_name,"amb") && AI_IsEndAnimation(self))
		{
			AI_ForceSequence(self, szAnimation, pSequence->flags );
		}
	}
	// NSS[12/13/99]:Hang out for a few seconds;
	// NSS[2/25/00]:If we pass in a time to wait...then wait that time frame otherwise it is a quick wait.
	if(pAIData->fValue && ((pAIData->fValue + gstate->time) < (gstate->time + 30.0f)))
	{
		pAIData->fValue += gstate->time;
	}
	else
	{
		pAIData->fValue = gstate->time + 0.25f;
	}
	pAIData->nValue = 0;
	
	// NSS[12/13/99]:Let's STOP
	self->velocity.Zero();

	AI_SetTaskFinishTime( hook, -1);
	
}


// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        SIDEKICK_Stop
// Description: Just stop ok?  
// Input:	userEntity_t *self
// Output:  na
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_Stop(userEntity_t *self)
{
	char szAnimation[16];
	float fDistance;
	float oDistance;
	playerHook_t *mHook;// NSS[1/23/00]:hook for our enemy
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	TASK_PTR pCurrentTask = GOALSTACK_GetCurrentTask( pGoalStack );
	AIDATA_PTR pAIData = TASK_GetData(pCurrentTask);	

	self->enemy = SIDEKICK_FindEnemy( self );
	// NSS[12/13/99]:Let's STOP
	self->velocity.Zero();
	

	if(self->enemy)
	{
		// NSS[1/17/00]:Handle getting out of the Stop function while attacking.
		if(SIDEKICK_IsOkToFight(self))
		{
			oDistance = VectorDistance(self->s.origin,hook->owner->s.origin);
			if(AI_IsVisible(self,self->enemy) || oDistance > fNormalDistanceFromOwner )
			{
				AI_RemoveCurrentTask(self,FALSE);
				return;
			}
		}
		else
		{
			mHook = AI_GetPlayerHook(self->enemy);
			fDistance = VectorDistance(self->s.origin,self->enemy->s.origin);
			if(fDistance < mHook->active_distance && AI_IsVisible(self,self->enemy)&&self->enemy->enemy == self)
			{
				AI_RemoveCurrentTask(self,FALSE);
				return;
			}
		}
		AI_FaceTowardPoint(self,self->enemy->s.origin);
	}

	if(AI_IsEndAnimation(self))
	{
		AI_SelectAmbientAnimation( self, szAnimation );
		frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
		if( pSequence != NULL)
		{
			AI_ForceSequence(self, szAnimation, pSequence->flags );
		}
	}

	// NSS[12/13/99]:End of our second.. .let's re-evaluate the situation.
	if(gstate->time > pAIData->fValue)
	{
		if(self->enemy && pAIData->nValue < 10 && !SIDEKICK_IsOkToFight(self))
		{
			fDistance = VectorDistance(self->s.origin,self->enemy->s.origin);
			if(fDistance > 500.0f && AI_IsVisible(self,self->enemy))// NSS[12/13/99]:Keep hangin'
			{
				pAIData->fValue = gstate->time + 0.25f;
				pAIData->nValue ++;
				return;
			}
		}
		AI_RemoveCurrentTask(self);
		return;
	}
}


// ----------------------------------------------------------------------------
// NSS[1/19/00]:
// Name:        SIDEKICK_HasPickedUpItem
// Description: Determine if the Sidekick has picked up an item only if it has
// TASKTYPE_SIDEKICK_PICKUPITEM in its Que.
// Input:	userEntity_t *self
// Output:  TRUE/FALSE
// Note:
// ----------------------------------------------------------------------------
int SIDEKICK_HasPickedUpItem( userEntity_t *self, int UseIt)
{
	playerHook_t *hook			= AI_GetPlayerHook( self );
	userEntity_t *owner			= hook->owner;

	GOALSTACK_PTR pGoalStack	= AI_GetCurrentGoalStack(hook);
	GOAL_PTR pCurrentGoal		= GOALSTACK_GetCurrentGoal( pGoalStack );

	TASK_PTR Task;
	
	// NSS[1/16/00]:Get the next task to be executed
	if(pCurrentGoal->pTasks->pStartList)
	{
		Task = pCurrentGoal->pTasks->pStartList;
		while(Task)
		{
			if(Task->nTaskType == TASKTYPE_SIDEKICK_PICKUPITEM)
			{
				AIDATA_PTR pAIData = TASK_GetData(Task);
				userEntity_t *pItem = pAIData->pEntity;
				// NSS[1/29/00]:For objects like health and such... 
				// NSS[1/29/00]:This is a hack because the sidekicks were standing on health(exact location) and not triggering it... don't have the time to figure out why the trigger shit is fucked.
				if(UseIt && pItem && pItem->touch)
				{
					tr = gstate->TraceLineTrigger( self->s.origin, self->s.origin, self, MASK_ALL, SVF_ITEM );
					if(tr.ent && tr.ent->flags & FL_ITEM)
						pItem->touch(pItem, self, NULL, NULL);
				}
				else if ( !pItem || !ITEM_IsVisible( pItem ) )
				{
					AI_RemoveCurrentTask( self );
					return TRUE;
				}
				return FALSE;
			}
			else
			{
				if(Task->pNext)
				{
					Task = Task->pNext;
				}
				else
				{
					Task = NULL;
					return FALSE;
				}
			}
		}
	}

	return FALSE;
}



// ----------------------------------------------------------------------------
// NSS[1/19/00]:
// Name:        SIDEKICK_HasTaskInQue
// Description: Determine if the Sidekick has 'Task' in its Que
// Input:	userEntity_t *self
// Output:  TRUE/FALSE
// Note:
// ----------------------------------------------------------------------------
int SIDEKICK_HasTaskInQue( userEntity_t *self, TASKTYPE SearchTask )
{
	playerHook_t *hook			= AI_GetPlayerHook( self );
	userEntity_t *owner			= hook->owner;

	GOALSTACK_PTR pGoalStack	= AI_GetCurrentGoalStack(hook);
	if(!pGoalStack)
		return FALSE;
	GOAL_PTR pCurrentGoal		= GOALSTACK_GetCurrentGoal( pGoalStack );
	if(!pCurrentGoal || !pCurrentGoal->pTasks)
		return FALSE;
	

	TASK_PTR Task;
	
	// SCG[2/10/00]: Early out if we can.  Also prevents from accessing NULL pointers
	if( 
		( pGoalStack->nNumGoals == 0 ) || 
		( pCurrentGoal == NULL ) || 
		( pCurrentGoal->pTasks == NULL ) )
	{
		return FALSE;
	}

	// NSS[1/16/00]:Get the next task to be executed
	if(pCurrentGoal->pTasks->pStartList)
	{
		Task = pCurrentGoal->pTasks->pStartList;
		while(Task)
		{
			if(Task->nTaskType == SearchTask)
			{
				return TRUE;
			}
			else
			{
				if(Task->pNext)
				{
					Task = Task->pNext;
				}
				else
				{
					Task = NULL;
					return FALSE;
				}
			}
		}
	}

	return FALSE;
}



// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        SIDEKICK_Process_Environment
// Description: Scan for unfriendlies and deal with them
// Input:	userEntity_t *self
// Output:  na
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_Process_Environment(userEntity_t *self)
{
	playerHook_t *hook			= AI_GetPlayerHook( self );
	userEntity_t *owner			= hook->owner;

	float fDistance = 0.0f;
	
	if(owner)
	{
		fDistance = VectorXYDistance( self->s.origin, owner->s.origin );
	}
	
	// NSS[12/11/99]: find an enemy that is close to the owner every so often
    userEntity_t *pEnemy = NULL;
	hook->nTargetCounter++;

	// NSS[1/20/00]:
	SIDEKICK_ChooseBestWeapon( self );

	// NSS[1/13/00]:Find the most threatening enemy.
	self->enemy = SIDEKICK_FindEnemy( self );
	
	if(self->enemy)
	{
	    // NSS[1/16/00]:Make sure we have the right kind of weapon
		SIDEKICK_ChooseBestWeapon( self );

		// NSS[12/15/99]:Make sure we are 'ok' to fight.
		if (!SIDEKICK_IsStaying(hook) && !hook->bCarryingMikiko && !SIDEKICK_HasTaskInQue(self,TASKTYPE_SIDEKICK_ATTACK) && SIDEKICK_IsOkToFight( self ))
		{
			AI_AddNewTaskAtFront(self,TASKTYPE_SIDEKICK_ATTACK);
		}
		else if ((!SIDEKICK_IsStaying(hook) && !SIDEKICK_IsOkToFight( self ) && !SIDEKICK_HasTaskInQue(self,TASKTYPE_SIDEKICK_EVADE)) && AI_IsVisible(self,self->enemy))
		{
			AI_AddNewTaskAtFront(self,TASKTYPE_SIDEKICK_EVADE,self->enemy);
		}
	}

	// NSS[12/20/99]:If we don't have an enemy targeting us or a 'friendly' then let's see if we need to process any
	// dead-owner related stuff.
	if(!self->enemy)
	{
		// NSS[12/9/99]:
		SIDEKICK_HandleDeadOwner(self);
	}

}

// ----------------------------------------------------------------------------
// NSS[1/3/00]:
// Name:		SideKick_IsVisible
// Description:Test whether a target is within fov and a line of sight
// Input:userEntity_t *self, userEntity_t *targ
// Output:TRUE/FALSE
// Note:
//
// ----------------------------------------------------------------------------
int SideKick_IsEnemyVisible( userEntity_t *self, userEntity_t *targ )
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	if ( !AI_IsAlive( targ ) || !AI_IsAlive( self ) )
	{
		return FALSE;
	}

	CVector	org, end;

	org.Add( self->s.origin, self->view_ofs );
	end.Add( targ->absmin, targ->absmax );
	end.Multiply( 0.5 );
	end.Add( targ->view_ofs );
	playerHook_t *pTargetHook = AI_GetPlayerHook( targ );
	
	//NSS[11/6/99]:Check to see if it even has a hook)
	if(pTargetHook)
	{
		//	use Potentially Visible Set to quickly exclude some entities
		tr = gstate->TraceLine_q2( org, end, self, MASK_SOLID);
		if ( tr.fraction >= 1.0f)
		{
			return TRUE;
		}
	}
	return	FALSE;
}


// ----------------------------------------------------------------------------
// NSS[3/2/00]:
// Name:        SIDEKICK_HandleManaSkull
// Description: This handles the manaskull thang!
// Input:	playerHook_t *hook
// Output:  NADA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_HandleManaSkull(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	if(hook)
	{
		if (hook->items & IT_MANASKULL)
		{
			if( hook->invulnerability_time <= 3.0 && ( hook->exp_flags & EXP_MANASKULL ) )
			{
				hook->exp_flags &= ~EXP_MANASKULL;
				gstate->StartEntitySound (self, CHAN_AUTO, gstate->SoundIndex("artifacts/manaskullfade.wav"), 1.0f, ATTN_NORM_MIN,ATTN_NORM_MAX);
			}

			if( hook->invulnerability_time < 0 )
			{
				hook->items  &= ~IT_MANASKULL;
			}
			else
				hook->invulnerability_time -= 0.1;
		}
	}
}

// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        SIDEKICK_HandleWraith
// Description: This handles the Wraith mode for Sidekicks
// Input:	userEntity_t *self
// Output:  NADA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_HandleWraith(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	if(hook)
	{
		if (hook->items & IT_WRAITHORB)
		{
			userEntity_t *pWeapon = NULL;
			
			self->s.renderfx |= RF_TRANSLUCENT;
			
			// make the clients weapons translucent also
			if (self->client && self->client->ps.weapon)
			{
				pWeapon = (userEntity_t*)self->client->ps.weapon;
				pWeapon->s.renderfx |= RF_TRANSLUCENT;
			}
			//	FIXME:	make wratthorb work
			if( hook->wraithorb_time >= 0 )
			{
				float speed = self->velocity.Length();

				// set translucency for player based on velocity
				// set to correct player model
				if (speed < 1.0)
				{
					if (self->s.alpha > 0.20)	// as we're standing still, go invisible
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
    				}
					hook->exp_flags -= (hook->exp_flags & IT_WRAITHORB);
				}
				// added this so the warning plays before it is a moot point, independent of time setting -KRH
				if ((0.3f < self->s.alpha) && (0.32 >= self->s.alpha))
				{
					//	wraithorb expiration sound and message
					gstate->StartEntitySound (self, CHAN_AUTO, gstate->SoundIndex("artifacts/wraithorburnout.wav"), 1.0f, ATTN_NORM_MIN,ATTN_NORM_MAX);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
// NSS[2/28/00]:
// Name:        SIDEKICK_GetAmmoCount
// Description: Gets the current Sidekicks ammo count
// Input:	userEntity_t *self
// Output:  NA
// ----------------------------------------------------------------------------
void SIDEKICK_GetAmmoCount(userEntity_t *self,byte *flag)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	if(hook && hook->owner)
	{
		weapon_t *pWeapon	= (weapon_t *) self->curWeapon;
		if(pWeapon && pWeapon->winfo)
		{
			weaponInfo_t *winfo = pWeapon->winfo;

			switch((int)winfo->EntityType)
			{
			case WEAPON_DISCUS:
				{
					if(AI_IsMikiko(hook))
						*flag = MK_ROCKIN_AMMO;
					else
						*flag = SF_ROCKIN_AMMO;
				}
				return;

			case WEAPON_DISRUPTOR:
			case WEAPON_SILVERCLAW:
				{
					if(AI_IsMikiko(hook))
						*flag = MK_LOW_AMMO;
					else
						*flag = SF_LOW_AMMO;
				}
				return;
			case WEAPON_VENEMOUS:
				{
					if (!pWeapon->ammo->count)
					{
						if(AI_IsMikiko(hook))
							*flag = MK_LOW_AMMO;
						else
							*flag = SF_LOW_AMMO;
					}
				}
				break;

			default:
				break;
			};

			float Ammo,Ammo_Max;
			Ammo_Max	= pWeapon->winfo->ammo_max;
			Ammo		= pWeapon->ammo->count;
			if(AI_IsMikiko(hook))
			{
				*flag	= MK_NO_WEAPON;
				if(Ammo >= Ammo_Max * 0.75f)
				{
					*flag = MK_ROCKIN_AMMO;
				}
				else if(Ammo >= Ammo_Max * 0.35f)
				{
					*flag = MK_GOOD_AMMO;
				}
				else if(Ammo != 0.0f)
				{
					*flag = MK_LOW_AMMO;
				}
			}
			else
			{
				*flag	= SF_NO_WEAPON;
				if(Ammo >= Ammo_Max * 0.75f)
				{
					*flag = SF_ROCKIN_AMMO;
				}
				else if(Ammo >= Ammo_Max * 0.40f)
				{
					*flag = SF_GOOD_AMMO;
				}
				else if(Ammo != 0.0f)
				{
					*flag = SF_LOW_AMMO;
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
// NSS[2/28/00]:
// Name:        SIDEKICK_CheckAmmoCount
// Description: Checks the Ammo for the sidekick and updates the hud.
// Input:	userEntity_t *self
// Output:  na
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_CheckAmmoCount(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	playerHook_t *ohook;

	byte flag_mk,flag_sf;
	
	flag_mk		= MK_NO_UPDATE;
	flag_sf		= SF_NO_UPDATE;

	userEntity_t *other = SIDEKICK_GetOtherSidekick(self);

	// NSS[2/28/00]:Superfly gets makiko's shiat!
	if(AI_IsSuperfly(hook) && other)
	{
		ohook = AI_GetPlayerHook( other );
		if(ohook && AI_IsSidekick(ohook))
		{
			SIDEKICK_GetAmmoCount(other,&flag_mk);
		}
		SIDEKICK_GetAmmoCount(self,&flag_sf);
	}
	else if (AI_IsSuperfly(hook))
	{
		SIDEKICK_GetAmmoCount(self,&flag_sf);	
	}
	else
	{
		SIDEKICK_GetAmmoCount(self,&flag_mk);	
	}

	// NSS[2/28/00]:Pak'em up and ship em' off!
	SIDEKICK_UpdateWeaponStatus(hook->owner, flag_mk,flag_sf);
}



// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        SIDEKICK_EndGameHandler
// Description: This function will take care of the sidekicks for good.
// this function is only called from Kage's code and will only be called if Kage
// has been activated.
// Input:	nothing
// Output:  can of kickass
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_EndGameHandler( void )
{
//	#ifndef Debug
	userEntity_t *Superfly,*Mikiko;

	Superfly	= AIINFO_GetSuperfly();
	Mikiko		= AIINFO_GetMikiko();

	gstate->RemoveEntity(Superfly);
	gstate->RemoveEntity(Mikiko);

//	#endif 
}


// ----------------------------------------------------------------------------
// NSS[12/4/99]:
// Name:        SIDEKICK_Think
// Description: Sidekicks version 6.66
// Input:	userEntity_t *self
// Output:  can of kickass
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_Think( userEntity_t *self )

{
	
	playerHook_t *hook = AI_GetPlayerHook( self );
	playerHook_t *ohook;
	CVector Mins;
	
	// NSS[3/23/00]:Hack for being frozen by medusa.
	if(!AI_IsAlive(self))
	{
		self->nextthink = gstate->time + 1.0f;
		return;
	}
	// NSS[3/14/00]:HACK HACK HACK HACK HACK HACK HACK HACK!!!!!!!!
	// NSS[3/14/00]:This is here because the physics engine is still completely fucked.
	// Sidekicks sometimes(rarely) get stuck into one another...this will allow superfly
	// the ability to get unstuck from mikiko if this happens.
	if( ((int)(gstate->time *10))%10)
	{
		Mins = self->s.mins;
		Mins.z *= 0.65f;
		tr = gstate->TraceBox_q2( self->s.origin, Mins, self->s.maxs, self->s.origin, self, self->clipmask );
		if(tr.ent && (tr.ent->flags & (FL_CLIENT|FL_BOT|FL_MONSTER)) && tr.ent != self)
		{
			if(!(self->svflags & SVF_DEADMONSTER))
			{
				self->svflags |= SVF_DEADMONSTER;
			}
		}
		else if((self->svflags & SVF_DEADMONSTER))
		{
			self->svflags &= ~SVF_DEADMONSTER;
		}
	}
	
	
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	// NSS[3/6/00]:Once again... the million'th hack to handle things that don't get restored properly from a load/save game.
	if(!pCurrentGoal)
	{
		if(hook && hook->owner && AI_IsAlive(hook->owner))
			AI_AddNewGoal(self,GOALTYPE_FOLLOW,hook->owner);
		else
			AI_AddNewGoal(self,GOALTYPE_IDLE);
		
		self->nextthink = gstate->time + 0.10f;
		return;
	}
	
	TASK_PTR pCurrentTask = GOAL_GetCurrentTask( pCurrentGoal );
	
	userEntity_t *client_entity = &gstate->g_edicts[1];	

	CVector AttackRate;

	// NSS[2/4/00]:Set the rate of attack based on the skill level
	AttackRate.Set(1.0f,2.0f,5.0f);

	// NSS[2/28/00]:If both sidekicks are around let the big boy handle the ammo thang!
	userEntity_t *other = SIDEKICK_GetOtherSidekick(self);
	// NSS[2/28/00]:Updates the status of the Sidekick's ammo
	if(AI_IsMikiko(hook) && other )
	{
		ohook = AI_GetPlayerHook( other );
		if(AI_IsSidekick(ohook))
			SIDEKICK_CheckAmmoCount(self);
	}
	else
	{
		SIDEKICK_CheckAmmoCount(self);	
	}
	
	// NSS[2/9/00]:Handle Wraith Mode
	SIDEKICK_HandleWraith(self);

	// NSS[3/2/00]:Handle The Mana Skull thing!
	SIDEKICK_HandleManaSkull(self);

	// NSS[1/31/00]:Restore color for the freezing water thing that will never be used... *sigh*
	if((hook->pain_finished <= gstate->time) && (GetCurrentEpisode() == 3) && self->s.color.z > 0.0f && !(self->watertype & CONTENTS_WATER))
	{
		self->s.color.z -=0.10f;
		if(self->s.color.z < 0.20f)
		{
			self->s.color.Set(0.0f,0.0f,0.0f);
		}
	}

	// NSS[12/11/99]:If we are not alive and we have reached this point
	// NSS[12/11/99]:We should go ahead and call our die function and then return out.
	if(!AI_IsAlive(self))
	{
		SIDEKICK_Die(self);
		self->nextthink = gstate->time + 0.1f;
		return;
	}
	// NSS[1/28/00]:Don't do a damned thing when cinematic's are playing
	if(gstate->bCinematicPlaying)
	{
		self->nextthink = gstate->time + 0.1f;
		return;
	}

	// NSS[12/9/99]:
	SIDEKICK_HandlePlayerSelectingSidekick( self );
    

	// NSS[12/5/99]:
	SIDEKICK_CheckMessage(self);

	// NSS[2/4/00]:If we are in search and destroy mode and our enemy is dead then reset
	if((hook->ai_flags & AI_SIDEKICKKILL) && self->enemy && !(AI_IsAlive(self->enemy)) )
	{
		self->enemy = NULL;
		hook->ai_flags &= ~AI_SIDEKICKKILL;
	}
	else if ((hook->ai_flags & AI_SIDEKICKKILL) && !self->enemy)
	{
		self->enemy = NULL;
		hook->ai_flags &= ~AI_SIDEKICKKILL;
	}
	if(SIDEKICK_HasTaskInQue(self,TASKTYPE_SIDEKICK_EVADE) || (self->enemy && !SIDEKICK_IsOkToFight(self)))
	{
		AI_SetStateRunning(hook);
	}

	// NSS[2/16/00]:
	if(self->waterlevel > 2)
	{
		if(hook->cur_sequence && hook->cur_sequence->animation_name  && !strstr(hook->cur_sequence->animation_name,"swim") )
			AI_ForceSequence(self,"swim",FRAME_LOOP);
	}
	
	// NSS[3/4/00]:If we are moving around then 
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// NSS[3/4/00]:Check to see if we are idle or not
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if(pCurrentTask && hook->owner && AI_IsAlive(hook->owner))
	{
		switch(pCurrentTask->nTaskType)
		{
			case TASKTYPE_SIDEKICK_LOOKAROUND:
			case TASKTYPE_SIDEKICK_LOOKUP:
			case TASKTYPE_SIDEKICK_LOOKDOWN:
			case TASKTYPE_IDLE:
			{
				float WaitTime;
				if(AI_IsSuperfly(hook))
					WaitTime = SuperFlyWaitIdle;
				else
					WaitTime = MikikoWaitIdle;

				// NSS[3/4/00]:Every 5th second let's check to see if we are able to play the impatient sounds.
				if(gstate->time > (WaitTime +  15.0f)) 
				{
   					if(!Check_Que(self,IMPATIENT,40.0f))
					{
						// NSS[3/4/00]:25% chance they will say something about sitting around.
						if(rnd() < 0.35f)
						{
							SideKick_TalkAmbient(self, IMPATIENT,1);
							if(AI_IsSuperfly(hook))
								SuperFlyWaitIdle	= gstate->time + 10 + (10*rnd());
							if(AI_IsMikiko(hook))
								MikikoWaitIdle		= gstate->time + 10 + (10*rnd());

						}
						SIDEKICK_SendMessage(self, sCom_Speak,IMPATIENT,(gstate->time+40.0f)+(rnd()*60.0f),0.0f,self,COM_Read);					}
				}
				// NSS[3/13/00]:
				SIDEKICK_ChitChat(self);
				break;
			}
			default:
			{
				if(AI_IsSuperfly(hook))
					SuperFlyWaitIdle	= gstate->time;
				if(AI_IsMikiko(hook))
					MikikoWaitIdle		= gstate->time;
			}
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// NSS[12/11/99]:Now process the real thinking/ following/ attacking from here down.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // NSS[12/13/99]:Sidekick specific task processing.
	// Any task that we might need to do any pre-processing on before it hits the generic
	// task think function we need to do so here.  Otherwise, it will go into a generic AI
	// think pattern.
	if(pCurrentTask)
	{
		switch(pCurrentTask->nTaskType)
		{
			case TASKTYPE_SIDEKICK_ATTACK:
			{
				SIDEKICK_Attack(self);
				
				if(!(hook->ai_flags & AI_SIDEKICKKILL) || (self->enemy && !(AI_IsAlive(self->enemy))) )
				{
					// NSS[2/4/00]:Clamper
					if(!((int)(gstate->time*10) % (int)AI_GetSkill(AttackRate)))
					{ 
						SIDEKICK_Process_Environment(self);
					}
				}
				break;
			}
			case TASKTYPE_SIDEKICK_STOP:
			{
				// NSS[12/13/99]:We are finished with all major tasks, let's look around a bit.
				SIDEKICK_Stop(self);
				break;
			}
			case TASKTYPE_CHASE:
			{
				if(SideKick_IsEnemyVisible(self,self->enemy))
					AI_TaskThink(self);
				else
					AI_RemoveCurrentTask(self,FALSE);
				break;
			}
			case TASKTYPE_MOVETOLOCATION:
			case TASKTYPE_WAITFORNOCOLLISION:
			{
				if(SIDEKICK_HasTaskInQue(self,TASKTYPE_SIDEKICK_ATTACK))
				{
					if(self->enemy)
					{
						if(AI_IsAlive(self->enemy))
						{
							if(SIDEKICK_IsClearShot(self,self->enemy,self->s.origin,self->enemy->s.origin))
							{
								AI_RemoveCurrentTask(self,FALSE);
								break;
							}
						}
						else
						{
							self->enemy = NULL;
							AI_RemoveCurrentTask(self,FALSE);
						}
					}
				}
				AI_TaskThink( self );
				break;
			}
			case TASKTYPE_SIDEKICK_EVADE:
			{
				if(self->enemy && SIDEKICK_IsOkToFight(self))
				{
					AI_RemoveCurrentTask(self,FALSE);
					break;
				}
				AI_TaskThink( self );
				break;
			}
			case TASKTYPE_WAIT:
			{
				if(strstr(hook->cur_sequence->animation_name,"run")||strstr(hook->cur_sequence->animation_name,"walk"))
				{
					char szAnimation[16];
					AI_SelectAmbientAnimation( self, szAnimation );
					AI_ForceSequence(self, szAnimation, FRAME_LOOP);
				}
			}
			case TASKTYPE_SIDEKICK_RANDOMWALK:
			case TASKTYPE_SIDEKICK_LOOKAROUND:
			case TASKTYPE_SIDEKICK_LOOKUP:
			case TASKTYPE_SIDEKICK_LOOKDOWN:
			case TASKTYPE_SIDEKICK_SAYSOMETHING:
			case TASKTYPE_SIDEKICK_KICKSOMETHING:
			case TASKTYPE_SIDEKICK_WHISTLE:
			case TASKTYPE_SIDEKICK_ANIMATE:
			case TASKTYPE_IDLE:
			case TASKTYPE_FOLLOWRUNNING:
			case TASKTYPE_FOLLOWWALKING:
			case TASKTYPE_FINISHCURRENTSEQUENCE:
			case TASKTYPE_SIDEKICK_COMEHERE:
			{
				
				if(hook->cur_sequence)
				{
					if(strstr(hook->cur_sequence->animation_name,"atak")||strstr(hook->cur_sequence->animation_name,"jump"))
					{
						char szAnimation[16];
						AI_SelectAmbientAnimation( self, szAnimation );
						AI_ForceSequence(self, szAnimation, FRAME_LOOP);
					}
				}
				// NSS[12/13/99]:We are finished with all major tasks, let's look around a bit.
				if(!(hook->ai_flags & AI_SIDEKICKKILL) || (self->enemy && !(AI_IsAlive(self->enemy))) )
				{
					// NSS[2/4/00]:Clamper
					if(!((int)(gstate->time*10) % (int)AI_GetSkill(AttackRate)))
					{ 
						SIDEKICK_Process_Environment(self);
					}
				}
				else if ((hook->ai_flags & AI_SIDEKICKKILL)&& AI_IsAlive(self->enemy))
				{
					if(!SIDEKICK_HasTaskInQue(self,TASKTYPE_SIDEKICK_ATTACK))
					{
						AI_AddNewTaskAtFront(self,TASKTYPE_SIDEKICK_ATTACK);
					}
				}

				// NSS[12/13/99]:handle ambient tasks, if we have one then break otherwise continue on to AI_Think
				if(SIDEKICK_HandleAmbientTasks( self ))
				{
					break;
				}
			}
			default:
			{	
				
				//Handle Generic AI Tasks here
				AI_TaskThink( self );
				break;
			}
		}
	}
	else
	{
		if(hook->owner)
		{
			AI_AddNewTask(self,TASKTYPE_FOLLOWRUNNING,hook->owner);
		}
		AI_AddNewTask(self,TASKTYPE_IDLE);
	}
	// NSS[12/13/99]:We always have a next think time!
	AI_SetNextThinkTime( self, 0.1f );
    // NSS[12/13/99]:If this happens then someone is an ass!
	_ASSERTE( self->nextthink > gstate->time );

}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_GetOtherSidekick
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *SIDEKICK_GetOtherSidekick( userEntity_t *self )
{
    userEntity_t *pOtherSidekick = NULL;
	int nNumSidekicks = AIINFO_GetNumSidekicks();
    if ( nNumSidekicks == 2 )
    {
        userEntity_t *pFirstSidekick = AIINFO_GetFirstSidekick();
        _ASSERTE( pFirstSidekick );
        userEntity_t *pSecondSidekick = AIINFO_GetSecondSidekick();
		if(!pSecondSidekick || !(pSecondSidekick->flags & FL_BOT))
			return NULL;

        pOtherSidekick = pSecondSidekick;
        if ( self == pSecondSidekick )
        {
            pOtherSidekick = pFirstSidekick;
        }
    }

	// NSS[3/14/00]:Make sure we are a sidekick
	if(pOtherSidekick && !(pOtherSidekick->flags & FL_BOT))
	{
		return NULL;
	}
	else
	{
		return pOtherSidekick;
	}
    
}


// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_CanDoRandomWalk
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_CanDoRandomWalk( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    if ( AI_IsOnTrain( self ) || SIDEKICK_IsStaying( hook ) )
    {
        return FALSE;
    }

    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_HandleAmbients
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_HandleAmbients( userEntity_t *self )
{
#ifdef DO_AMBIENTS

    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	// set the owner to the goal data
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
    if ( pCurrentGoal )
    {
        GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
        if ( nGoalType == GOALTYPE_SIDEKICK_STOPGOINGFURTHER ||
             nGoalType == GOALTYPE_SIDEKICK_TELEPORTANDCOMENEAR )
        {
			if(self->velocity.Length() <= 180.0f && (strstr(hook->cur_sequence->animation_name,"walk") || strstr(hook->cur_sequence->animation_name,"run")) )
			{	
				if(!strstr(hook->cur_sequence->animation_name,"hit"))
				{
					char szAnimation[16];
					AI_SelectAmbientAnimation( self, szAnimation );
					AI_ForceSequence(self, szAnimation, FRAME_LOOP);
				}
				else if(AI_IsEndAnimation(self))
				{
					char szAnimation[16];
					AI_SelectAmbientAnimation( self, szAnimation );
					AI_ForceSequence(self, szAnimation, FRAME_LOOP);		
				}
				AI_ZeroVelocity(self);	
			}
			return FALSE;
        }
    }

    float *pNextAmbientTimes = SIDEKICK_GetNextAmbientTimes( self );

    int nNumAmbients = 0;
    TASKTYPE aAmbientTasks[MAX_NUMBER_AMBIENTS];
    for ( int i = 0; i < MAX_NUMBER_AMBIENTS; i++ )
    {
        if ( gstate->time > pNextAmbientTimes[i] && 
             rnd() < aAmbientInfo[i].fRandomFactor )
        {
            aAmbientTasks[nNumAmbients++] = (TASKTYPE)(TASKTYPE_SIDEKICK_RANDOMWALK + i);
        }
    }

    if ( nNumAmbients > 0 )
    {
        int nRandom = rand() % nNumAmbients;

        if ( aAmbientTasks[nRandom] == TASKTYPE_SIDEKICK_RANDOMWALK )
        {
            if ( SIDEKICK_CanDoRandomWalk( self ) )
            {
                int nNumRandomWalk = (rand() % 3) + 1;
                for ( i = 0; i < nNumRandomWalk; i++ )
                {
					AI_AddNewTaskAtFront( self, TASKTYPE_SIDEKICK_RANDOMWALK );
                }
            }
        }
        else
        {
            AI_AddNewTaskAtFront( self, aAmbientTasks[nRandom] );
        }
        return TRUE;
    }

#endif DO_AMBIENTS
	//NSS[12/3/99]:If we are in ambients and we are still using the walking frame... let's not.
	if(self->velocity.Length() <= 180.0f && (strstr(hook->cur_sequence->animation_name,"walk") || strstr(hook->cur_sequence->animation_name,"run")) )
	{	
		if(!strstr(hook->cur_sequence->animation_name,"hit"))
		{
			char szAnimation[16];
			AI_SelectAmbientAnimation( self, szAnimation );
			AI_ForceSequence(self, szAnimation, FRAME_LOOP);
		}
		else if(AI_IsEndAnimation(self))
		{
			char szAnimation[16];
			AI_SelectAmbientAnimation( self, szAnimation );
			AI_ForceSequence(self, szAnimation, FRAME_LOOP);		
		}
		AI_ZeroVelocity(self);	
	}
	else if (self->velocity.Length() <= 48.0f && strstr(hook->cur_sequence->animation_name,"amb"))
	{
		AI_ZeroVelocity(self);	
	}
    return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_DetermineFollowing
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_DetermineFollowing( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *pOwner = hook->owner;
    _ASSERTE( pOwner );

    userEntity_t *pOtherSidekick = SIDEKICK_GetOtherSidekick( self );
    // NSS[2/10/00]:Test
	//hook->nFollowing = FOLLOWING_PLAYER;
	//return hook->nFollowing;

	int nNumSidekicks = AIINFO_GetNumSidekicks();
    if ( nNumSidekicks == 2 )
    {
        if(!pOtherSidekick)
		{
			hook->nFollowing = FOLLOWING_PLAYER;
			return hook->nFollowing;
		}
		if ( AI_IsSuperfly( hook ) && SIDEKICK_IsCarryingMikiko(hook) )
        {
            hook->nFollowing = FOLLOWING_PLAYER;
        }
        else
        {
            playerHook_t *pOtherSidekickHook = AI_GetPlayerHook( pOtherSidekick );
			if(pOtherSidekickHook)
			{
				if ( pOtherSidekickHook->nFollowing == FOLLOWING_NONE )
				{
					// make the sidekick that is closer to the player follow first
					float fThisSidekickDistance = VectorDistance( pOwner->s.origin, self->s.origin );
					float fOtherSidekickDistance = VectorDistance( pOwner->s.origin, pOtherSidekick->s.origin );
					if ( fThisSidekickDistance < fOtherSidekickDistance )
					{
						hook->nFollowing = FOLLOWING_PLAYER;
					}
					else
					{
						hook->nFollowing = FOLLOWING_SIDEKICK;
					}
				}
				else
				if ( pOtherSidekickHook->nFollowing == FOLLOWING_PLAYER )
				{
					hook->nFollowing = FOLLOWING_SIDEKICK;
				}
				else
				if ( pOtherSidekickHook->nFollowing == FOLLOWING_SIDEKICK )
				{
					hook->nFollowing = FOLLOWING_PLAYER;
				}
			}
        }
    }
    else
    {
        hook->nFollowing = FOLLOWING_PLAYER;
    }
	return hook->nFollowing;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_HandleFollowing
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_HandleFollowing( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    
	if (hook == NULL || SIDEKICK_IsStaying( hook ) || (self->enemy && AI_IsAlive(self->enemy)) || !(hook->ai_flags & AI_FOLLOW) )
    {
        return FALSE;
    }

	userEntity_t *pOwner = hook->owner;
    if(pOwner == NULL)
		return FALSE;

    hook->nFollowing = FOLLOWING_NONE;

	// set the owner to the goal data
	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
	_ASSERTE( pCurrentGoal );
	GOAL_Set( pCurrentGoal, pOwner );

	TASK_PTR pCurrentTask = GOAL_GetCurrentTask( pCurrentGoal );
    _ASSERTE( pCurrentTask );
    TASKTYPE nTaskType = TASK_GetType( pCurrentTask );
	
	if(!AI_IsAlive(pOwner))
		return FALSE;
    
	if ( AI_IsOnTrain( pOwner ) && !AI_IsOnTrain( self ) && AI_IsOnMovingTrain( pOwner ) )
	{
		float fZDistance = VectorZDistance( self->s.origin, pOwner->s.origin );
		if ( fZDistance < MAX_JUMP_HEIGHT )
		{
            if ( SIDEKICK_IsAmbientTask( nTaskType ) )
            {
                AI_RemoveCurrentTask( self, TASKTYPE_MOVETOOWNER, pOwner );
            }
            else
            {
                AI_AddNewTaskAtFront( self, TASKTYPE_MOVETOOWNER, pOwner );
            }
			return TRUE;
		}
	}	
	
	CVector lastPlayerMovedPosition = CLIENT_GetLastPlayerMovedPosition();
    float fRunFollowDistance,fWalkFollowDistance,fDistance,fPlayerMovedDistance,OwnerVelocity;
    int nFollowing = SIDEKICK_DetermineFollowing( self );
	
	fRunFollowDistance = AI_GetRunFollowDistance(hook);
	fWalkFollowDistance = AI_GetWalkFollowDistance(hook);
	
	lastPlayerMovedPosition = CLIENT_GetLastPlayerMovedPosition();
	fPlayerMovedDistance = VectorDistance( pOwner->input_entity->s.origin, lastPlayerMovedPosition );
	
	OwnerVelocity = pOwner->velocity.Length();
	
	if ( nFollowing == FOLLOWING_PLAYER )
    {
		lastPlayerMovedPosition = CLIENT_GetLastPlayerMovedPosition();
		fDistance = VectorDistance( self->s.origin, pOwner->s.origin );
	}
	else
	{
		userEntity_t *ent = SIDEKICK_GetOtherSidekick(self);
		if(ent)
		{
			pOwner = ent;
			fDistance = VectorDistance( self->s.origin, pOwner->s.origin );
		}
	}
	int IsOwnerVisible = AI_IsVisible(self,pOwner);
	if(!SIDEKICK_CanPathToOrIsCloseToOwner(self, pOwner))
		return FALSE;

	if ( (!IsOwnerVisible || fPlayerMovedDistance > 64.0f || OwnerVelocity > 20.0f || fDistance > (fWalkFollowDistance * 2.0f)) && pOwner->deadflag == DEAD_NO )
    {
       // if ( !SIDEKICK_IsAmbientTask( nTaskType ) )
        //{
            GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
            
            

			if ( (fDistance > fRunFollowDistance || SIDEKICK_HasTaskInQue(self,TASKTYPE_SIDEKICK_ATTACK)) && !AI_IsCrouching(hook))	
            {
                if ( nFollowing == FOLLOWING_PLAYER )
                {
                    if ( nGoalType == GOALTYPE_FOLLOW )
                    {
                        GOAL_ClearTasks( pCurrentGoal );
					}
                    AI_AddNewTaskAtFront( self, TASKTYPE_FOLLOWRUNNING, pOwner );
					//AI_ForceSequence(self, "runa");
                    return TRUE;
                }
                else
                if ( nFollowing == FOLLOWING_SIDEKICK && !AI_IsCrouching(hook))
                {
                    userEntity_t *pOtherSidekick = SIDEKICK_GetOtherSidekick( self );
                    _ASSERTE( pOtherSidekick );

                    if ( nGoalType == GOALTYPE_FOLLOW )
                    {
                        GOAL_ClearTasks( pCurrentGoal );
                    }
                    AI_AddNewTaskAtFront( self, TASKTYPE_FOLLOWSIDEKICKRUNNING, pOtherSidekick );
                    return TRUE;
                }
            }
            else
            if ( fDistance > fWalkFollowDistance || !IsOwnerVisible )
            {
                if ( nFollowing == FOLLOWING_PLAYER )
                {
                    if ( nGoalType == GOALTYPE_FOLLOW )
                    {
                        GOAL_ClearTasks( pCurrentGoal );
                    }
                    AI_AddNewTaskAtFront( self, TASKTYPE_FOLLOWWALKING, pOwner );
                    return TRUE;
                }
                else
                if ( nFollowing == FOLLOWING_SIDEKICK )
                {
                    userEntity_t *pOtherSidekick = SIDEKICK_GetOtherSidekick( self );
                    _ASSERTE( pOtherSidekick );

                    if ( nGoalType == GOALTYPE_FOLLOW )
                    {
                        GOAL_ClearTasks( pCurrentGoal );
                    }
					AI_AddNewTaskAtFront( self, TASKTYPE_FOLLOWSIDEKICKWALKING, pOtherSidekick );
                    return TRUE;
                }
	        //}
        }
    }

    return FALSE;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_TrackOwner
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_TrackOwner( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *pOwner = hook->owner;
	if ( pOwner && pOwner->deadflag == DEAD_NO && !self->enemy)
	{
        self->ideal_ang.pitch = 0.0f;
        com->ChangePitch( self );

	    CVector dir = pOwner->s.origin - self->s.origin;
	    dir.Normalize();
	    CVector angleTowardOwner;
	    VectorToAngles( dir, angleTowardOwner );
	    
	    float fTowardOwnerY = AngleMod(angleTowardOwner.y);
	    float fFacingAngleY = AngleMod(self->s.angles.yaw);

	    float fYawDiff = fabs( fTowardOwnerY - fFacingAngleY );
	    if ( fYawDiff > 60.0f && fYawDiff < 300.0f )
	    {
		    AI_FaceTowardPoint( self, pOwner->s.origin );
	    }
    }
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_ResetAmbientTimes
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_ResetAmbientTimes( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    if ( AI_IsSidekick( hook ) )
    {
        // reset the next ambient times
        float *pNextAmbientTimes = SIDEKICK_GetNextAmbientTimes( self );
    
        for ( int i = 0; i < MAX_NUMBER_AMBIENTS; i++ )
        {
            pNextAmbientTimes[i] = gstate->time + aAmbientInfo[i].fWaitTime;
        }
    }

}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_HandleIdle
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_HandleIdle( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    hook->nFollowing = FOLLOWING_NONE;

    if ( AI_IsSidekick( hook ) )
    {
	    if ( (hook->ai_flags & AI_IGNORE_PLAYER) )
	    {
		    return;
	    }

	    if ( !self->groundEntity  && self->waterlevel < 3)
	    {
		    return;
	    }

	    if ( !hook->owner )
	    {
		    SIDEKICK_FindOwner( self );
	    }
		    
	    userEntity_t *pOwner = hook->owner;
	    if ( pOwner && AI_IsAlive(pOwner) )
	    {
		    
			if ( SIDEKICK_HandleFollowing( self ) == TRUE )
            {
                return;
            }

            // track owner
            SIDEKICK_TrackOwner( self );

            if ( SIDEKICK_HandlePickupItems( self ) == FALSE )
            {
                // NSS[2/3/00]:Don't do ambient things if crouching
				if(!hook->bCarryingMikiko && !AI_IsCrouching( hook ) )
				{
					SIDEKICK_HandleAmbients( self );
				}
				else
				{
					// NSS[12/6/99]:As long as we aren't in the middle of a pain frame.					
					if(self->velocity.Length() <= 48.0f && (strstr(hook->cur_sequence->animation_name,"walk") || strstr(hook->cur_sequence->animation_name,"run") || strstr(hook->cur_sequence->animation_name,"amb")))
					{
						
						AI_ZeroVelocity(self);
						// NSS[2/3/00]:Don't do ambient things if crouching
						if(!strstr(hook->cur_sequence->animation_name,"hit") && !AI_IsCrouching( hook ))
						{
							char szAnimation[16];
							AI_SelectAmbientAnimation( self, szAnimation );
							AI_ForceSequence(self, szAnimation, FRAME_LOOP);
						}
						else if(AI_IsEndAnimation(self) && !AI_IsCrouching(hook))
						{
							char szAnimation[16];
							AI_SelectAmbientAnimation( self, szAnimation );
							AI_ForceSequence(self, szAnimation, FRAME_LOOP);		
						}
				
					}
				}
            }
        }
        else
        {
            self->ideal_ang.pitch = 0.0f;
            com->ChangePitch( self );
			// NSS[2/3/00]:Don't do ambient things if crouching
			if(!AI_IsCrouching( hook ))
				SIDEKICK_HandleAmbients( self );
        }
    }
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_FindNearUnobstructedPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
#define BETWEEN_SIDEKICK_DISTANCE   64.0f

int SIDEKICK_FindNearUnobstructedPoint( userEntity_t *self, CVector &point )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    userEntity_t *pOwner = hook->owner;
    if ( !pOwner )
    {
        return FALSE;
    }

    userEntity_t *pOtherSidekick = SIDEKICK_GetOtherSidekick( self );
    if ( !pOtherSidekick )
    {
        return FALSE;
    }

    CVector twoPoints[2];
    int nNumFreePoints = 0;
    
    CVector vectorTowardOwner = pOwner->s.origin - pOtherSidekick->s.origin;
    vectorTowardOwner.Normalize();

    CVector angleTowardOwner;
    VectorToAngles( vectorTowardOwner, angleTowardOwner );

    CVector angle = angleTowardOwner;    
	angle.pitch = -5.0f;

	CVector endPoint, vector;

	angle.yaw = AngleMod( angleTowardOwner.yaw + 60.0f );
	angle.AngleToForwardVector( vector );
	endPoint.x = pOtherSidekick->s.origin.x + (vector.x * BETWEEN_SIDEKICK_DISTANCE);
	endPoint.y = pOtherSidekick->s.origin.y + (vector.y * BETWEEN_SIDEKICK_DISTANCE);
	endPoint.z = pOtherSidekick->s.origin.z + (vector.z * BETWEEN_SIDEKICK_DISTANCE);
	CVector bottomPoint = endPoint;
	bottomPoint.z -= 1.0f;
	tr = gstate->TraceBox_q2( endPoint, self->s.mins, self->s.maxs, bottomPoint, self, self->clipmask );
	if ( tr.fraction >= 1.0f && !tr.allsolid && !tr.startsolid && !tr.ent )
	{
		bottomPoint = endPoint;
        bottomPoint.z -= 100.0f;
        tr = gstate->TraceLine_q2( endPoint, bottomPoint, self, MASK_SOLID );
        if ( tr.fraction < 1.0f )
        {
            twoPoints[nNumFreePoints] = endPoint;
            twoPoints[nNumFreePoints].z -= 100.0f * tr.fraction;
            twoPoints[nNumFreePoints].z += 24.0f;

            nNumFreePoints++;
        }
	}

	angle = angleTowardOwner;
    angle.pitch -= 5.0f;
    angle.yaw = AngleMod( angleTowardOwner.yaw - 60.0f );
	angle.AngleToForwardVector( vector );
	endPoint.x = pOtherSidekick->s.origin.x + (vector.x * BETWEEN_SIDEKICK_DISTANCE);
	endPoint.y = pOtherSidekick->s.origin.y + (vector.y * BETWEEN_SIDEKICK_DISTANCE);
	endPoint.z = pOtherSidekick->s.origin.z + (vector.z * BETWEEN_SIDEKICK_DISTANCE);
	bottomPoint = endPoint;
	bottomPoint.z -= 1.0f;
	tr = gstate->TraceBox_q2( endPoint, self->s.mins, self->s.maxs, bottomPoint, self, self->clipmask );
	if ( tr.fraction >= 1.0f && !tr.allsolid && !tr.startsolid && !tr.ent )
	{
		bottomPoint = endPoint;
        bottomPoint.z -= 100.0f;
        tr = gstate->TraceLine_q2( endPoint, bottomPoint, self, MASK_SOLID );
        if ( tr.fraction < 1.0f )
        {
            twoPoints[nNumFreePoints] = endPoint;
            twoPoints[nNumFreePoints].z -= 100.0f * tr.fraction;
            twoPoints[nNumFreePoints].z += 24.0f;

            nNumFreePoints++;
        }
	}

    int bRetValue = FALSE;
    if ( nNumFreePoints == 1 )
    {
        point = twoPoints[0];
        bRetValue = TRUE;
    }
    else
    if ( nNumFreePoints == 2 )
    {
        // find the closer of the two from the owner
        float fDistance1 = VectorDistance( pOwner->s.origin, twoPoints[0] );
        float fDistance2 = VectorDistance( pOwner->s.origin, twoPoints[1] );
        if ( fDistance1 < fDistance2 )
        {
            point = twoPoints[0];
        }
        else
        {
            point = twoPoints[1];
        }
        bRetValue = TRUE;
    }

    return bRetValue;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_FaceTowardPoint
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_FaceTowardPoint( userEntity_t *self, CVector &destPoint )
{
	_ASSERTE( self );

	CVector dir = destPoint - self->s.origin;
	dir.Normalize();
	CVector newIdealAngle;
	VectorToAngles( dir, newIdealAngle );

    self->ideal_ang = newIdealAngle;
    self->ideal_ang.pitch = 0.0f;

//	if ( newIdealAngle.pitch > 5.0f && newIdealAngle.pitch < 180.0f )
//    {
//        newIdealAngle.pitch *= 0.5f;
//    }
//    else
//    if ( newIdealAngle.pitch >= 180.0f && newIdealAngle.pitch < 355.0f )
//    {
//        float fTempPitch = 360.0f - newIdealAngle.pitch;
//        fTempPitch *= 0.5f;
//        newIdealAngle.pitch = 360.0f - fTempPitch;
//    }
//    self->ideal_ang = newIdealAngle;
//    if ( self->ideal_ang.pitch > 45.0f && self->ideal_ang.pitch < 315.0f )
//    {
//        if ( self->ideal_ang.pitch < 180.0f )
//        {
//            self->ideal_ang.pitch = 45.0f;
//        }
//        else
//        {
//            self->ideal_ang.pitch = 315.0f;
//        }
//    }
//
//	com->ChangePitch( self );

    com->ChangeYaw( self );
}

// ----------------------------------------------------------------------------
// NSS[2/8/00]:
// Name:        SIDEKICK_HandleMoveAway
// Description: Handles the sidekicks moveing away from one another.
// Input:userEntity_t *self(the one moving away), userEntity_t *other (the one colliding with us)
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_HandleMoveAway( userEntity_t *self, userEntity_t *other )
{
    playerHook_t *hook	= AI_GetPlayerHook( self );
	playerHook_t *ohook = AI_GetPlayerHook( other);

    //First we move out of their way.. if possible.
	if ((other->flags & (FL_CLIENT|FL_BOT)) )
    {
		if (!SIDEKICK_HasTaskInQue(self,TASKTYPE_MOVEAWAY))// && other->flags & FL_CLIENT)
        {
			if(other->flags & FL_CLIENT)
			{
				if(!Check_Que(self,PUSHEDBY_HIRO,60.0f))
				{
					SideKick_TalkAmbient(self, PUSHEDBY_HIRO,1);
					SIDEKICK_SendMessage(self, sCom_Speak,PUSHEDBY_HIRO,gstate->time+60.0f,0.0f,self,COM_Read);
					return;
				}
			}
			AI_AddNewTaskAtFront( self, TASKTYPE_MOVEAWAY, other );
        }
    }
	else if(other->flags & FL_BOT)
	{
		//Second if we are unable to then we need to tell them to stop 
		if(!SIDEKICK_HasTaskInQue(self,TASKTYPE_SIDEKICK_STOP))
		{
			AI_AddNewTaskAtFront( self, TASKTYPE_SIDEKICK_STOP, other );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_IsOwnerClose
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_IsOwnerClose( userEntity_t *self )
{
    playerHook_t *hook = AI_GetPlayerHook( self );

 	userEntity_t *pOwner = hook->owner;
    _ASSERTE( pOwner );

    float fRunFollowDistance = AI_GetRunFollowDistance( hook );

    float fDistance = VectorDistance( self->s.origin, pOwner->s.origin );
	if ( fDistance > (fRunFollowDistance * 4.0f) || 
		 ( fDistance > (fRunFollowDistance * 2.0f) && !AI_IsLineOfSight( self, pOwner ) )
	   )
	{
        return FALSE;
    }

    return TRUE;
}


// ----------------------------------------------------------------------------
// NSS[1/23/00]: New Function to find a good retreat Node
// Name:		SIDEKICK_FindFurthestNodeFrom
// Description:A majorly re-worked function similar to NODE_FindFurthestFrom but
// greatly enhanced.  This function searches multiple layers within the node system
// to determine the best Node to retreat to.  This function DOES use recursion.
// Input: NodeTraverse *NodeT
// Output: TRUE/FALSE and NodeT->pCurrentNode is the best destination if returned True.
// Note: NodeTraverse Struct layout
// NODELIST_PTR pNodeList --> node list to parse through
// CVector &Origin		  --> Origin to Determine distance between nodes.
// int NodeIndex		  --> current Node we are looking at.
// float Distance		  --> current Distance away
// int NodeDepth          --> current NodeDepth
// float MaximumDistance  --> Maximum Distance to traverse from the origin
// float CurrentDistance  --> Current distance traversed from the origin
// ----------------------------------------------------------------------------
int	SIDEKICK_FindFurthestNodeFrom (NodeTraverse *NodeT)
{
	int i;
	float fDistance;
	float CurrentDistance;
	MAPNODE_PTR pBestLinkNode,pLinkNode;
	NODEHEADER_PTR pNodeHeader;
	
	// NSS[1/24/00]:Check our nodelist and current index before doing anything
	if( NodeT->pNodeList == NULL || (NodeT->pNodeList->nCurrentNodeIndex == -1) )
	{
		return FALSE;
	}

	// NSS[1/24/00]:Setup all node pointers and the header
	pBestLinkNode		= NULL;
	pNodeHeader			= NodeT->pNodeList->pNodeHeader;
	NodeT->pCurrentNode	= &(pNodeHeader->pNodes[NodeT->CurrentNodeIndex]);

	// NSS[1/24/00]:Make sure we are traversing a valid node
	if(!NodeT->pCurrentNode)
	{
		return FALSE;
	}
	
	if( (NodeT->NodeDepth >= NodeT->MaximumNodeDepth) || (NodeT->CurrentDistance >= NodeT->MaximumDistance) )
	{
		return TRUE;
	}

	for ( i = 0; i < NodeT->pCurrentNode->nNumLinks; i++ )
	{
		pLinkNode = &(pNodeHeader->pNodes[NodeT->pCurrentNode->aLinks[i].nIndex]);
		
		if ( pLinkNode->nNumLinks > 1 )
		{
			fDistance = VectorXYDistance(pLinkNode->position, NodeT->origin);
			if ( fDistance > NodeT->CurrentDistance )
			{
				if(NodeT->CurrentDistance > NodeT->GreatestDistance)
				{
					NodeT->GreatestDistance = NodeT->CurrentDistance;
					NodeT->pBestNode		= pLinkNode;
				}

				// NSS[2/1/00]:Store this itterations currentdistance value
				CurrentDistance			= NodeT->CurrentDistance;
				// NSS[2/1/00]:Setup the next itterations CurrentDistance, index, and depth;
				NodeT->CurrentDistance	= fDistance;
				NodeT->CurrentNodeIndex	= pLinkNode->nIndex;
				NodeT->NodeDepth		++;
				if(SIDEKICK_FindFurthestNodeFrom(NodeT))
				{
					return TRUE;
				}
				else
				{
					NodeT->CurrentDistance	= CurrentDistance;
					NodeT->CurrentNodeIndex	= NodeT->pCurrentNode->nIndex;
					NodeT->NodeDepth		--;
				}
			}
		}
		else//Handle single linked nodes
		{
			fDistance = NodeT->CurrentDistance + VectorXYDistance( NodeT->pCurrentNode->position, NodeT->origin );
			if(fDistance > NodeT->MaximumDistance)
			{
				NodeT->CurrentDistance = fDistance;
				return TRUE;
			}
		}
	}
	return FALSE;
}

// ----------------------------------------------------------------------------
// NSS[1/23/00]:Revamped version of Sidekick's evade function
// Name:		SIDEKICK_StartEvade
// Description:Run away!
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_StartEvade( userEntity_t *self )
{
	_ASSERTE( self );
	
	NodeTraverse	NodeT;

	int				Trap = 0;	

	playerHook_t *hook			= AI_GetPlayerHook( self );
	// NSS[1/23/00]:Check our hook
	if(!hook)
	{
		AI_Dprintf("Problemo's no hook and Sidekick is trying to evade!!\n");
		AI_RemoveCurrentTask(self,FALSE);
	}
	
	AI_SetStateRunning( hook );
    
	// NSS[1/23/00]:Check our enemy pointer
	//If we don't have an enemy get out.  If our enemy is dead and doesn't have the explosive flag set get out.  Otherwise if 
	//our enemy is dead but has the explosive flag set then get the fuck outta there.
	if ( !self->enemy || (!AI_IsAlive( self->enemy ) && !(self->enemy->flags & FL_EXPLOSIVE)))
    {
	    AI_RemoveCurrentTask( self );
	    return;
    }

	// NSS[1/31/00]:Added evade sounds to the sidekicks.
	// NSS[2/23/00]:We should move out of the way from missiles but not scream about them... 
	if(!Check_Que(self,EVADE,15.0f) && !(self->enemy->flags & FL_CLIENT) && !strstr(self->enemy->className,"projectile_missile"))
	{
		SideKick_TalkAmbient(self, EVADE,1);
		SIDEKICK_SendMessage(self, sCom_Speak,EVADE,gstate->time+5.0,0.0f,self,COM_Read);
	}

	if(hook->pNodeList)
	{
		
		// NSS[1/24/00]:Setup Start Point
		NodeT.origin				= self->enemy->s.origin;
		NodeT.NodeDepth				= 0;
		NodeT.CurrentDistance		= VectorXYDistance(self->s.origin,self->enemy->s.origin);
		NodeT.CurrentNodeIndex		= hook->pNodeList->nCurrentNodeIndex;
		NodeT.GreatestDistance		= 0.0f;
		NodeT.pNodeList				= hook->pNodeList;
		NodeT.MaximumNodeDepth		= 5;
		NodeT.pBestNode				= NULL;
		if(self->enemy->flags & FL_EXPLOSIVE)
		{
			NodeT.MaximumDistance	= 256.0f;
		}
		else
		{
			float fDistanceFrom		= VectorDistance(self->s.origin, self->enemy->s.origin);
			NodeT.MaximumDistance	= 700.0f + fDistanceFrom;
		}
		
		if(!SIDEKICK_FindFurthestNodeFrom (&NodeT))
		{
			// NSS[1/22/00]:If not then get the HideNode
			
			if(!NodeT.pBestNode)
			{
				
				MAPNODE_PTR pRetreatNode = NODE_GetClosestCompleteHideNode( hook->pNodeList->pNodeHeader, self, self->enemy );
				if(!pRetreatNode)
				{
					Trap = 1;
				}
				else
				{
					NodeT.pBestNode = pRetreatNode;
				}
			}
			
		}
	}

	if(NodeT.pBestNode)
	{
		if(AI_IsCloseDistance2(self,VectorXYDistance(self->s.origin,NodeT.pBestNode->position)))
		{
			Trap = 1;
		}
	}
	else
	{
		AI_Dprintf("Sidekick Trying to Evade but finding no path or nodes to use!!!\n");
		Trap = 1;
	}
	// NSS[2/1/00]:This trap prevents jitter in animations due to the fact that the sidekick might
	// get in a situation where they just can't run anywhere else...if so don't run
	if(!Trap)
	{
		frameData_t *pCurrentSequence = hook->cur_sequence;
		if ( !strstr( pCurrentSequence->animation_name, "run" ))
		{
			char szAnimation[16];
			AI_SelectRunningAnimation( self, szAnimation );
			AI_ForceSequence(self, szAnimation, FRAME_LOOP);
		}
	}

	AI_SetNextThinkTime	( self, 0.1f );
	AI_SetOkToAttackFlag( hook, FALSE );
	AI_SetTaskFinishTime( hook, 2.0f );
	AI_SetMovingCounter	( hook, 0 );
}


// ----------------------------------------------------------------------------
// NSS[1/23/00]:Revamped SidekickEvade function
// Name:		SIDEKICK_Evade
// Description:Main loop for finding the next node to traverse towards.
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_Evade( userEntity_t *self )
{
	NodeTraverse				NodeT;
	playerHook_t *hook			= AI_GetPlayerHook( self );
    float fDistance				= 0.0f;

	if(!hook)
	{
		AI_Dprintf("Problemo's no hook and Sidekick is trying to evade!!\n");
		AI_RemoveCurrentTask(self,FALSE);
		return;
	}
	
	// NSS[1/23/00]:Check our enemy
	if(self->enemy)
	{
		fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );
	}
	else
	{
		AI_RemoveCurrentTask(self,FALSE);
		return;
	}
	

	AI_SetStateRunning( hook );
	if( fDistance > 700.0f && !AI_IsVisible(self,self->enemy) )
	{
		AI_AddNewTaskAtFront(self,TASKTYPE_SIDEKICK_STOP);
	}
	
	if (AI_IsEndAnimation(self))
    {
        char szAnimation[16];
        AI_SelectRunningAnimation( self, szAnimation );
        AI_ForceSequence(self, szAnimation, FRAME_LOOP);
    }
	
	if(hook->pNodeList)
	{
		
		// NSS[1/24/00]:Setup Start Point
		NodeT.origin				= self->enemy->s.origin;
		NodeT.NodeDepth				= 0;
		NodeT.CurrentDistance		= VectorXYDistance(self->s.origin,self->enemy->s.origin);
		NodeT.CurrentNodeIndex		= hook->pNodeList->nCurrentNodeIndex;
		NodeT.GreatestDistance		= 0.0f;
		NodeT.pNodeList				= hook->pNodeList;
		NodeT.MaximumNodeDepth		= 5;
		NodeT.pBestNode				= NULL;
		if(self->enemy->flags & FL_EXPLOSIVE)
		{
			NodeT.MaximumDistance	= 256.0f;
		}
		else
		{
			float fDistanceFrom		= VectorDistance(self->s.origin, self->enemy->s.origin);
			NodeT.MaximumDistance	= 700.0f + fDistanceFrom;
		}
		
		if(!SIDEKICK_FindFurthestNodeFrom (&NodeT))
		{
			// NSS[1/22/00]:If not then get the HideNode	
			if(!NodeT.pBestNode)
			{
				MAPNODE_PTR pRetreatNode = NODE_GetClosestCompleteHideNode( hook->pNodeList->pNodeHeader, self, self->enemy );
				if(!pRetreatNode)
				{
					if(hook->owner )
					{
						// NSS[2/7/00]:Run home to da-da
						if(!AI_IsCrouching(hook))
						{
							AI_AddNewTaskAtFront( self,TASKTYPE_FOLLOWRUNNING,hook->owner);
						}
						else
						{
							AI_AddNewTaskAtFront( self,TASKTYPE_FOLLOWWALKING,hook->owner);
						}

					}
					else
					{
						//	nowhere to run to, hang out a bit and get the shit kicked out of us... 
						AI_AddNewTaskAtFront( self,TASKTYPE_SIDEKICK_STOP,hook->owner->s.origin  );					
					}
					return;
				}
				NodeT.pBestNode = pRetreatNode;
			}
			
		}
	}

	if(NodeT.pBestNode)
	{
		if(AI_IsCloseDistance2(self,VectorXYDistance(self->s.origin,NodeT.pBestNode->position)))
		{
			if(hook->owner)
			{
				// NSS[2/7/00]:Run home to da-da
				if(!AI_IsCrouching(hook))
					AI_AddNewTaskAtFront( self,TASKTYPE_FOLLOWRUNNING,hook->owner);
				else
					AI_AddNewTaskAtFront( self,TASKTYPE_FOLLOWWALKING,hook->owner);
			}
			else
			{
				//	nowhere to run to, hang out a bit and get the shit kicked out of us... 
				AI_AddNewTaskAtFront( self,TASKTYPE_SIDEKICK_STOP,hook->owner->s.origin  );					
			}
		
		}
		else
		{
			if(hook->owner)
			{
				float oDistance;
				oDistance = VectorDistance(self->s.origin,hook->owner->s.origin);
				if(oDistance > 768.0f)
				{
					// NSS[2/7/00]:Run home to da-da
					if(!AI_IsCrouching(hook))
						AI_AddNewTaskAtFront( self,TASKTYPE_FOLLOWRUNNING,hook->owner);
					else
						AI_AddNewTaskAtFront( self,TASKTYPE_FOLLOWWALKING,hook->owner);
				}
				else
				{
					AI_AddNewTaskAtFront(self,TASKTYPE_MOVETOLOCATION, NodeT.pBestNode->position);
				}
			}
			else
			{
				AI_AddNewTaskAtFront(self,TASKTYPE_MOVETOLOCATION, NodeT.pBestNode->position);
			}
		}
	}
	else
	{
		AI_Dprintf("Sidekick Trying to Evade but finding no path or nodes to use!!!\n");
		AI_AddNewTaskAtFront( self, TASKTYPE_SIDEKICK_STOP );	
	}
	AI_SetTaskFinishTime( hook, 1.0f );
}


// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_DoStopGoingFurther
// Description:
//              int nOrder => 1 = first time
//                            2 = second time
//                              this var is used so that sound is played only once
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_DoStopGoingFurther( userEntity_t *self, CVector &position, 
                                 char *szSound, char *szAnimation, int nOrder /* = 1 */ )
{
 	_ASSERTE( self );

	if ( !AI_IsAlive(self) )
	{
		return FALSE;
	}

	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if ( GOALSTACK_GetCurrentGoalType( pGoalStack ) == GOALTYPE_SIDEKICK_STOPGOINGFURTHER )
    {
        return FALSE;
    }

    AI_AddNewGoal( self, GOALTYPE_SIDEKICK_STOPGOINGFURTHER );

    TASKTYPE nTaskType = TASKTYPE_NONE;
    if ( szSound && strlen(szSound) > 0 && szAnimation && strlen(szAnimation) > 0 && nOrder == 1 )
    {
    	AIDATA aiData;
        frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
        if ( pSequence )
        {
            aiData.pAnimSequence = pSequence;
        }
        aiData.pString = szSound;
        
        AI_AddNewTask( self, TASKTYPE_PLAYANIMATIONANDSOUND, &aiData );

        nTaskType = TASKTYPE_PLAYANIMATIONANDSOUND;
    }
    else
    {
        if ( szAnimation && strlen(szAnimation) > 0 )
        {
    	    frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
            AI_AddNewTask( self, TASKTYPE_PLAYANIMATION, pSequence );

            if ( nTaskType == TASKTYPE_NONE )
            {
                nTaskType = TASKTYPE_PLAYANIMATION;
            }
        }
	    if ( szSound && strlen(szSound) > 0 && nOrder == 1 )
        {
            PlaySidekickMP3(self,szSound);//AI_AddNewTask( self, TASKTYPE_PLAYSOUND, szSound );

            //if ( nTaskType == TASKTYPE_NONE )
            //{
            //  nTaskType = TASKTYPE_PLAYSOUND;
            //}
        }
    }

	if ( position.Length() > 0.0f )
    {
        AI_SetStateRunning( hook );
        AI_AddNewTask( self, TASKTYPE_MOVETOLOCATION, position );
        
        if ( nTaskType == TASKTYPE_NONE )
        {
            nTaskType = TASKTYPE_MOVETOLOCATION;
        }
    }

    AI_AddNewTask( self, TASKTYPE_SIDEKICK_STOPGOINGFURTHER );

    if ( nTaskType == TASKTYPE_PLAYANIMATIONANDSOUND )
    {
        AI_StartPlayAnimationAndSound( self );
    }
    else
    if ( nTaskType == TASKTYPE_PLAYANIMATION )
    {
	    AI_StartPlayAnimation( self );
    }
    else
    if ( nTaskType == TASKTYPE_MOVETOLOCATION )
    {
        AI_StartMoveToLocation( self );
    }

    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_DoStopGoingFurther
// Description:
//              int nOrder => 1 = first time
//                            2 = second time
//                              this var is used so that sound is played only once
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_DoStopGoingFurther( userEntity_t *self, MAPNODE_PTR pCurrentNode, int nOrder /* = 1 */ )
{
	if ( !AI_IsAlive(self) || !pCurrentNode )
	{
		return FALSE;
	}

    CVector position = pCurrentNode->node_data.vector;
    char *szSound = pCurrentNode->target;
    char *szAnimation = pCurrentNode->targetname;

    return SIDEKICK_DoStopGoingFurther( self, position, szSound, szAnimation, nOrder );

}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_DoTeleportAndComeNear
// Description:
//              int nOrder => 1 = first time
//                            2 = second time
//                              this var is used so that sound is played only once
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_DoTeleportAndComeNear( userEntity_t *self, CVector &position, 
                                    char *szSound, char *szAnimation, int nOrder /* = 1 */ )
{
 	_ASSERTE( self );

	if ( !AI_IsAlive(self) )
	{
		return FALSE;
	}
	playerHook_t *hook = AI_GetPlayerHook( self );

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
	if ( GOALSTACK_GetCurrentGoalType( pGoalStack ) == GOALTYPE_SIDEKICK_TELEPORTANDCOMENEAR )
    {
        return FALSE;
    }

    AI_AddNewGoal( self, GOALTYPE_SIDEKICK_TELEPORTANDCOMENEAR );

    TASKTYPE nTaskType = TASKTYPE_NONE;
    
    userEntity_t *pSecondSidekick = AIINFO_GetSecondSidekick();
    if ( self == pSecondSidekick )
    {
        AI_AddNewTask( self, TASKTYPE_WAIT, 1.0f );
        nTaskType = TASKTYPE_WAIT;
    }

	if ( position.Length() > 0.0f )
    {
        AI_AddNewTask( self, TASKTYPE_SIDEKICK_TELEPORT, position );
        
        if ( nTaskType == TASKTYPE_NONE )
        {
            nTaskType = TASKTYPE_SIDEKICK_TELEPORT;
        }
    }

	if ( position.Length() > 0.0f )
    {
        userEntity_t *pOwner = hook->owner;
        if ( AI_IsAlive( pOwner ) )
        {
            AI_AddNewTask( self, TASKTYPE_MOVETOENTITYUNTILVISIBLE, pOwner );
        
            if ( nTaskType == TASKTYPE_NONE )
            {
                nTaskType = TASKTYPE_MOVETOENTITYUNTILVISIBLE;
            }
        }
    }

    if ( szSound && strlen(szSound) > 0 && szAnimation && strlen(szAnimation) > 0 && nOrder == 1 )
    {
    	AIDATA aiData;
        frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
        if ( pSequence )
        {
            aiData.pAnimSequence = pSequence;
        }
        aiData.pString = szSound;
        
        AI_AddNewTask( self, TASKTYPE_PLAYANIMATIONANDSOUND, &aiData );

        if ( nTaskType == TASKTYPE_NONE )
        {
            nTaskType = TASKTYPE_PLAYANIMATIONANDSOUND;
        }
    }
    else
    {
        if ( szAnimation && strlen(szAnimation) > 0 )
        {
    	    frameData_t *pSequence = FRAMES_GetSequence( self, szAnimation );
            AI_AddNewTask( self, TASKTYPE_PLAYANIMATION, pSequence );

            if ( nTaskType == TASKTYPE_NONE )
            {
                nTaskType = TASKTYPE_PLAYANIMATION;
            }
        }
	    if ( szSound && strlen(szSound) > 0 && nOrder == 1 )
        {
            AI_AddNewTask( self, TASKTYPE_PLAYSOUND, szSound );

            if ( nTaskType == TASKTYPE_NONE )
            {
                nTaskType = TASKTYPE_PLAYSOUND;
            }
        }
    }

    AI_AddNewTask( self, TASKTYPE_SIDEKICK_TELEPORTANDCOMENEAR );

    if ( nTaskType == TASKTYPE_WAIT )
    {
        AI_StartWait( self );
    }
    else
    if ( nTaskType == TASKTYPE_SIDEKICK_TELEPORT )
    {
        SIDEKICK_StartTeleport( self );
    }
    else
    if ( nTaskType == TASKTYPE_MOVETOENTITYUNTILVISIBLE )
    {
        AI_StartMoveToEntityUntilVisible( self );
    }
    else
    if ( nTaskType == TASKTYPE_PLAYANIMATION )
    {
	    AI_StartPlayAnimation( self );
    }
    else
    if ( nTaskType == TASKTYPE_PLAYSOUND )
    {
        AI_StartPlaySound( self );
    }
    else
    if ( nTaskType == TASKTYPE_MOVETOLOCATION )
    {
        AI_StartMoveToLocation( self );
    }
    
    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_DoTeleportAndComeNear
// Description:
//              int nOrder => 1 = first time
//                            2 = second time
//                              this var is used so that sound is played only once
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_DoTeleportAndComeNear( userEntity_t *self, MAPNODE_PTR pCurrentNode, int nOrder /* = 1 */ )
{
	if ( !AI_IsAlive(self) || !pCurrentNode )
	{
		return FALSE;
	}

    CVector position = pCurrentNode->node_data.vector;
    char *szSound = pCurrentNode->target;
    char *szAnimation = pCurrentNode->targetname;

    return SIDEKICK_DoTeleportAndComeNear( self, position, szSound, szAnimation, nOrder );

}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_HandleSuperflyTalk
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_HandleSuperflyTalk( userEntity_t *self, MAPNODE_PTR pCurrentNode )
{
 	_ASSERTE( self );
    _ASSERTE( pCurrentNode );

	if ( !self || !pCurrentNode )
	{
		return FALSE;
	}

	playerHook_t *hook = AI_GetPlayerHook( self );
    _ASSERTE( AI_IsSidekick( hook ) );

    if ( !AI_IsSuperfly( hook ) )
    {
        return FALSE;
    }

	int bRetValue = FALSE;
	
	userEntity_t *owner = hook->owner;

	if(owner && (pCurrentNode->node_type & NODETYPE_SUPERFLYTALK) )
	{
		float fDistance = VectorDistance( self->s.origin, owner->s.origin);
		if(!Check_Que(self,NO_TALK,1.0f) && fDistance < 800.0f && AI_IsVisible(self,owner))
		{
			fDistance = VectorDistance( self->s.origin, pCurrentNode->position );
			if ( AI_IsCloseDistance2(self, fDistance ) && NODE_IsTriggered( pCurrentNode ) == FALSE )
			{
				NODE_SetTriggered( pCurrentNode );
				if ( pCurrentNode->target )
				{
					AI_AddNewTaskAtFront( self, TASKTYPE_SIDEKICK_SAYSOMETHING, pCurrentNode->target );
				}

				bRetValue = TRUE;
			}
		}
	}

    return bRetValue;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_HandleMikikoTalk
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_HandleMikikoTalk( userEntity_t *self, MAPNODE_PTR pCurrentNode )
{
 	_ASSERTE( self );
    _ASSERTE( pCurrentNode );

	if ( !self || !pCurrentNode )
	{
		return FALSE;
	}

	playerHook_t *hook = AI_GetPlayerHook( self );
    _ASSERTE( AI_IsSidekick( hook ) );

    if ( !AI_IsMikiko( hook ) )
    {
        return FALSE;
    }

	// NSS[3/12/00]:If we can't talk then let's not do a damned thing.
	if(Check_Que(self,NO_TALK,1.0f))
	{
		return FALSE;
	}

	int bRetValue = FALSE;
	userEntity_t *owner = hook->owner;
	if(owner  && ( pCurrentNode->node_type & NODETYPE_MIKIKOTALK))
	{
		float fDistance = VectorDistance( self->s.origin, owner->s.origin);
		if(!Check_Que(self,SPECIAL_TALK,5.0f) && fDistance < 800.0f && AI_IsVisible(self,owner))
		{
			fDistance = VectorDistance( self->s.origin, pCurrentNode->position );
			if ( AI_IsCloseDistance2( self, fDistance ) && NODE_IsTriggered( pCurrentNode ) == FALSE )
			{
				NODE_SetTriggered( pCurrentNode );
				if ( pCurrentNode->target )
				{
					AI_AddNewTaskAtFront( self, TASKTYPE_SIDEKICK_SAYSOMETHING, pCurrentNode->target );
				}

				bRetValue = TRUE;
			}
		}
	}
    return bRetValue;
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_ReadAmbientInfo
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_ReadAmbientInfo( userEntity_t *self, const char *szFileName )
{
    int bSuccess = TRUE;

	CCSVFile *pCsvFile = NULL;
	int nRetCode = CSV_OpenFile( szFileName, &pCsvFile );
	if ( nRetCode == DKIO_ERROR_NONE )
	{
		int nNumSequences = 0;
		
		char szLine[2048];
		char szElement[64];
		// skip first line
		CSV_GetNextLine( pCsvFile, szLine );

        int i = 0;
		while ( CSV_GetNextLine( pCsvFile, szLine ) != EOF )
		{
			int nRetValue = CSV_GetFirstElement( pCsvFile, szLine, szElement );
			if ( nRetValue > 0 )
			{
                float fValue = 0.0f;
                if ( (nRetValue = CSV_GetNextElement( pCsvFile, szLine, szElement )) != EOLN )
                {
                    int nRetValue = sscanf( szElement, "%f", &fValue );
                    if ( nRetValue > 0 )
                    {
                        aAmbientInfo[i].fWaitTime = fValue;
                    }
                }
                
                if ( (nRetValue = CSV_GetNextElement( pCsvFile, szLine, szElement )) != EOLN )
                {
                    int nRetValue = sscanf( szElement, "%f", &fValue );
                    if ( nRetValue > 0 )
                    {
                        aAmbientInfo[i].fRandomFactor = fValue;
                    }
                }

                i++;
			}

		}

		CSV_CloseFile( pCsvFile );
	}
	else
	{
		bSuccess = FALSE;
	}		

	return bSuccess;
}


// ----------------------------------------------------------------------------
//                      SIDEKICK attacking player code
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_AttackPlayerUpdateMeleeAttackMovement
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_AttackPlayerUpdateMeleeAttackMovement( userEntity_t *self )
{
	userEntity_t *pEnemy = self->enemy;
	if ( !AI_IsAlive( pEnemy ) )
	{
		return FALSE;
	}

    playerHook_t *hook = AI_GetPlayerHook( self );

    int bCanSeeEnemy = AI_IsLineOfSight( self, pEnemy );

	float fDistance = VectorDistance( self->s.origin, pEnemy->s.origin );
	if ( fDistance < AI_GetCurrentWeaponRange( self ) )
	{
        AI_ZeroVelocity( self );
    }
    else
    {
        SIDEKICK_MoveTowardPointWhileAttacking( self, pEnemy->s.origin );
    }

    SIDEKICK_SelectAnimationWhileAttacking( self );

    return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_AttackPlayerUpdateRangeAttackMovement
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_AttackPlayerUpdateRangeAttackMovement( userEntity_t *self )
{
	userEntity_t *pEnemy = self->enemy;
	if ( !AI_IsAlive( pEnemy ) )
	{
		return FALSE;
	}

    playerHook_t *hook = AI_GetPlayerHook( self );

    int nPrevAttackMode = hook->nAttackMode;

    int bCanSeeEnemy = AI_IsCompletelyVisible( self, pEnemy, SIDEKICK_WIDTH_FACTOR );
    if ( gstate->time > hook->fAttackTime )
    {
        if ( SIDEKICK_IsAboutToFire( hook ) )
        {
            if ( bCanSeeEnemy == NOT_VISIBLE )
            {
                hook->nAttackMode = ATTACKMODE_SIDEKICK_CHARGE;
                SIDEKICK_SetAttackTime( hook, gstate->time + (rnd() * 2.0f) + 2.0f ); 
            }
            else
            {
                if ( rnd() > 0.5f )
                {
                    if ( SIDEKICK_ShouldStrafe( self, pEnemy ) )
                    {
                        hook->nAttackMode = ATTACKMODE_SIDEKICK_STRAFE;
                        SIDEKICK_SetAttackTime( hook, gstate->time + (rnd() * 4.0f) + 4.0f );
                    }
                    else
                    {
                        hook->nAttackMode = ATTACKMODE_SIDEKICK_BACKAWAY;
                        SIDEKICK_SetAttackTime( hook, gstate->time + (rnd() * 4.0f) + 3.0f );
                    }
                }
                else
                {
                    hook->nAttackMode = ATTACKMODE_SIDEKICK_STAYSTILL;
                    SIDEKICK_SetAttackTime( hook, gstate->time + 0.5f );
                }
            }
        }
        else
        if ( AI_IsJustFired( hook ) )
        {
            if ( rnd() > 0.5f && bCanSeeEnemy == NOT_VISIBLE )
            {
                if ( rnd() > 0.5f && SIDEKICK_ShouldCharge( self, pEnemy ) )
                {
                    hook->nAttackMode = ATTACKMODE_SIDEKICK_CHARGE;
                    SIDEKICK_SetAttackTime( hook, gstate->time + (rnd() * 2.0f) + 2.0f ); 
                }
                else
                {
                    if ( SIDEKICK_ShouldStrafe( self, pEnemy ) )
                    {
                        hook->nAttackMode = ATTACKMODE_SIDEKICK_STRAFE;
                        SIDEKICK_SetAttackTime( hook, gstate->time + (rnd() * 4.0f) + 4.0f );
                    }
                    else
                    {
                        hook->nAttackMode = ATTACKMODE_SIDEKICK_BACKAWAY;
                        SIDEKICK_SetAttackTime( hook, gstate->time + (rnd() * 4.0f) + 3.0f );
                    }
                }
            }
            else
            {
                if ( rnd() > 0.5f )
                {
                    hook->nAttackMode = ATTACKMODE_SIDEKICK_BACKAWAY;
                    SIDEKICK_SetAttackTime( hook, gstate->time + (rnd() * 4.0f) + 3.0f );
                }
                else
                {
                    hook->nAttackMode = ATTACKMODE_SIDEKICK_TAKECOVER;
                    SIDEKICK_SetAttackTime( hook, gstate->time + (rnd() * 4.0f) + 5.0f );
                }
            }
        }
        else
        {
            if ( bCanSeeEnemy == NOT_VISIBLE )
            {
                hook->nAttackMode = ATTACKMODE_SIDEKICK_STAYSTILL;
                SIDEKICK_SetAttackTime( hook, gstate->time + 0.5f );
            }
            else
            {
                int nRandom = rand() % 4;
                switch ( nRandom )
                {
                    case 0:
                    {
                        hook->nAttackMode = ATTACKMODE_SIDEKICK_TAKECOVER;
                        SIDEKICK_SetAttackTime( hook, gstate->time + (rnd() * 4.0f) + 7.0f );
                        break;
                    }
                    case 1:
                    {
                        if ( SIDEKICK_ShouldStrafe( self, pEnemy ) )
                        {
                            hook->nAttackMode = ATTACKMODE_SIDEKICK_STRAFE;
                            SIDEKICK_SetAttackTime( hook, gstate->time + (rnd() * 4.0f) + 4.0f );
                        }
                        else
                        {
                            hook->nAttackMode = ATTACKMODE_SIDEKICK_BACKAWAY;
                            SIDEKICK_SetAttackTime( hook, gstate->time + (rnd() * 4.0f) + 3.0f );
                        }
                        break;
                    }
                    case 2:
                    {
                        hook->nAttackMode = ATTACKMODE_SIDEKICK_BACKAWAY;
                        SIDEKICK_SetAttackTime( hook, gstate->time + (rnd() * 4.0f) + 3.0f );
                        break;
                    }
                    case 3:
                    {
                        if ( SIDEKICK_ShouldCharge( self, pEnemy ) )
                        {
                            hook->nAttackMode = ATTACKMODE_SIDEKICK_CHARGE;
                            SIDEKICK_SetAttackTime( hook, gstate->time + (rnd() * 2.0f) + 2.0f );
                        }
                        else
                        {
                            hook->nAttackMode = ATTACKMODE_SIDEKICK_BACKAWAY;
                            SIDEKICK_SetAttackTime( hook, gstate->time + (rnd() * 4.0f) + 3.0f );
                        }
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }
    }

    if ( hook->nAttackMode != nPrevAttackMode )
    {
        PATHLIST_KillPath(hook->pPathList);
    }

    if ( hook->nAttackMode == ATTACKMODE_SIDEKICK_STRAFE )
    {
        SIDEKICK_AttackWhileStrafing( self, pEnemy );
    }
    else
    if ( hook->nAttackMode == ATTACKMODE_SIDEKICK_TAKECOVER )
    {
        SIDEKICK_TakeCover( self, pEnemy );
    }
    else
    if ( hook->nAttackMode == ATTACKMODE_SIDEKICK_STAYSTILL )
    {
        SIDEKICK_StayStill( self, pEnemy );
    }
    else
    if ( hook->nAttackMode == ATTACKMODE_SIDEKICK_BACKAWAY )
    {
        SIDEKICK_BackAway( self, pEnemy );
    }
    else
    if ( hook->nAttackMode == ATTACKMODE_SIDEKICK_CHARGE )
    {
        SIDEKICK_Charge( self, pEnemy );
    }
    else
    {
        _ASSERTE( FALSE );
    }

    playerHook_t *pEnemyHook = AI_GetPlayerHook( pEnemy );
    if ( pEnemy->flags & FL_CLIENT )
    {
        pEnemyHook = AI_GetPlayerHook( pEnemy->input_entity );
    }
    if ( bCanSeeEnemy != NOT_VISIBLE && AI_IsJustFired( pEnemyHook ) )
    {
        if ( pEnemy->flags & FL_CLIENT )
        {
            if ( pEnemyHook->autoAim.ent == self )
            {
                SIDEKICK_Jump( self, pEnemy );
            }
        }
        else
        {
            if ( rnd() < 0.25f || AI_IsEnemyLookingAtMe( self, pEnemy ) )
            {
                SIDEKICK_Jump( self, pEnemy );
            }
        }
    }
            
    SIDEKICK_SelectAnimationWhileAttacking( self );

    return TRUE;

}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_AttackPlayerUpdateAttackMovement
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
int SIDEKICK_AttackPlayerUpdateAttackMovement( userEntity_t *self )
{
	_ASSERTE( self );
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

	userEntity_t *pEnemy = self->enemy;
	_ASSERTE( AI_IsAlive( pEnemy ) );

	float fRange = AI_GetCurrentWeaponRange( self );
    if ( fRange < SIDEKICK_MELEE_ATTACK_DISTANCE )
	{
        if ( SIDEKICK_AttackPlayerUpdateMeleeAttackMovement( self ) == FALSE )
        {
            return FALSE;
        }
    }
	else
	{
        // ranged attack weapon
        if ( SIDEKICK_AttackPlayerUpdateRangeAttackMovement( self ) == FALSE )
        {
            return FALSE;
        }
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
//
// Name:		SIDEKICK_AttackPlayer
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_AttackPlayer( userEntity_t *self )
{
	_ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *pEnemy = self->enemy;
    if ( !AI_IsAlive( pEnemy ) )
	{
		// killed the enemy, no longer in attack mode
		return;
	}

	AI_FaceTowardPoint( self, pEnemy->s.origin );
	
    int bOkToFight = TRUE;
    if ( SIDEKICK_IsOkToFight( self ) )
    {
	    weapon_t *pWeapon = (weapon_t *) self->curWeapon;
	    if ( !pWeapon || !SIDEKICK_CanUseWeapon( pWeapon ) )
	    {
            SIDEKICK_ChooseBestWeapon( self );
            pWeapon = (weapon_t *) self->curWeapon;
            if ( !pWeapon || !SIDEKICK_CanUseWeapon( pWeapon ) )
            {
                bOkToFight = FALSE;
            }
        }
    }
    else
    {
        bOkToFight = FALSE;
    }
                    
    if ( bOkToFight == FALSE )
    {
        // try to evade from fighting
        GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
        GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
        
		//if(GOALSTACK_GetCurrentTaskType( pGoalStack ) == TASKTYPE_SIDEKICK_EVADE))
		//{
			
		//}
		//else
		//{
			GOAL_ClearTasks( pCurrentGoal );
			AI_AddNewTaskAtFront( self, TASKTYPE_SIDEKICK_EVADE, pEnemy );
			return;
		//}
    }

	if ( SIDEKICK_AttackPlayerUpdateAttackMovement( self ) == FALSE )
	{
		return;
	}

	SIDEKICK_UseCurrentWeapon( self );
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_AttackPlayerThink
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_AttackPlayerThink( userEntity_t *self )
{
	_ASSERTE( self );

    if ( !AI_IsAlive( self ) )
    {
        AI_TaskThink( self );
        return;
    }

	playerHook_t *hook = AI_GetPlayerHook( self );

	userEntity_t *pEnemy = self->enemy;
    if ( !AI_IsAlive( pEnemy ) )
	{
		// killed the enemy, no longer in attack mode
        AI_ZeroVelocity( self );
        SIDEKICK_SelectAnimationWhileAttacking( self );
		return;
	}

	GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack(hook);
	GOAL_PTR pCurrentGoal = GOALSTACK_GetCurrentGoal( pGoalStack );
    if ( !pCurrentGoal )
    {
        pCurrentGoal = AI_AddNewGoal( self, GOALTYPE_GENERALUSE );
    }
    _ASSERTE( pCurrentGoal );

    AI_DecreaseJustFired( hook );

    GOALTYPE nGoalType = GOAL_GetType( pCurrentGoal );
    if ( nGoalType == GOALTYPE_GENERALUSE && GOAL_GetNumTasks( pCurrentGoal ) > 0 )
    {
	    AI_FaceTowardPoint( self, pEnemy->s.origin );
	    
	    SIDEKICK_UseCurrentWeapon( self );

        AI_TaskThink( self );
    }
    else
    {
        SIDEKICK_AttackPlayer( self );
        AI_SetNextThinkTime( self, 0.1f );
    }

    _ASSERTE( self->nextthink > gstate->time );

    if ( AI_IsJustFired( hook ) )
    {
        // end this state
        SIDEKICK_SetAttackTime( hook, gstate->time );
    }

}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_SetAttackPlayer
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_SetAttackPlayer( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

    if ( !AI_IsAlive( self ) || SIDEKICK_IsCarryingMikiko( hook ) )
    {
        return;
    }

    userEntity_t *pPlayer = AIINFO_GetPlayer();
    _ASSERTE( AI_IsAlive( pPlayer ) );

    AI_ClearTeam( self );

    hook->owner         = NULL;
    hook->bAttackPlayer = TRUE;
    self->enemy         = pPlayer;

	self->think		    = SIDEKICK_AttackPlayerThink;

	AI_SetStateAttacking( hook );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    GOALSTACK_ClearAllGoals( pGoalStack );
    AI_AddNewGoal( self, GOALTYPE_GENERALUSE );

	hook->strafe_dir = rand() % (STRAFE_DOWNRIGHT+1);

	SIDEKICK_ChooseBestWeapon( self );
    SIDEKICK_SetAttackSequence( self );

	AI_SetNextThinkTime( self, 0.1f );

}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_StopAttackPlayer
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void SIDEKICK_StopAttackPlayer( userEntity_t *self )
{
    _ASSERTE( self );
	playerHook_t *hook = AI_GetPlayerHook( self );

    GOALSTACK_PTR pGoalStack = AI_GetCurrentGoalStack( hook );
    GOALSTACK_ClearAllGoals( pGoalStack );

    hook->bAttackPlayer = TRUE;
    self->enemy         = NULL;

	SIDEKICK_FindOwner( self );

    if ( hook->owner )
    {
	    self->think	= SIDEKICK_Think;
    }
    else
    {
        self->think = SIDEKICK_StopAttackPlayer;
    }

	AI_SetNextThinkTime( self, 0.1f );
}

// ----------------------------------------------------------------------------
//                          Test functions
// ----------------------------------------------------------------------------
void SIDEKICK_TestAttack( userEntity_t *self )
{
    userEntity_t *pSidekick = AIINFO_GetFirstSidekick();

    // find enemy, then attack
	userEntity_t *pFirstMonster = alist_FirstEntity( monster_list );
	if ( pFirstMonster )
    {
        SIDEKICK_SetAttackMode( self, pFirstMonster );
    }
}

void SIDEKICK_TestAttackPlayer( userEntity_t *self )
{
    userEntity_t *pSidekick = AIINFO_GetFirstSidekick();
    
    if ( pSidekick )
    {
        SIDEKICK_SetAttackPlayer( pSidekick );
    }
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_DisableCommanding
// Description: 
// Note:        
//              
// ----------------------------------------------------------------------------
void SIDEKICK_DisableCommanding( userEntity_t *self )
{
    userEntity_t *pMikiko, *pSuperfly, *pLastSelectedSidekick;

    pMikiko   = AIINFO_GetMikiko();
    pSuperfly = AIINFO_GetSuperfly();

    pLastSelectedSidekick = AIINFO_GetLastSelectedSidekick();

	if( pLastSelectedSidekick == pMikiko )
	{
		com->Sidekick_Update( self, TYPE_MIKIKO, SIDEKICK_STATUS_DEACTIVATE, 0 );
	}
	else if( pLastSelectedSidekick == pSuperfly )
	{
		com->Sidekick_Update( self, TYPE_SUPERFLY, SIDEKICK_STATUS_DEACTIVATE, 0 );
	}

}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_SwapCommanding
// Description: Swap command control between sidekicks
// Note:        If Mikiko is the current sidekick, swap control to Superfly
//              (and vice versa)
// ----------------------------------------------------------------------------
void Inventory_SetMode( userEntity_t *self, int nMode, qboolean bPerformAction );
void SIDEKICK_SetCommanding( userEntity_t *self, userEntity_t *pSK, int mode )
{
	if (mode == INV_MODE_SIDEKICK1)		// select superfly
	{
		Inventory_SetMode( self, INV_MODE_SIDEKICK1, FALSE );// SCG[1/24/00]: set the mode for the global inventory status
		AIINFO_SetLastSelectedSidekick(pSK);  // he's in command now
		com->Sidekick_Update(self,TYPE_SUPERFLY,SIDEKICK_STATUS_COMMANDING,0); // update client
		SIDEKICK_CommandAcknowledge(pSK,SIDEKICK_COMMAND_COMMAND,true); // respond to given command

		gstate->CommandSelectorNext( self );
		gstate->CommandSelectorPrev( self );
	}
	else if (mode == INV_MODE_SIDEKICK2)	// select mikiko
	{
		Inventory_SetMode( self, INV_MODE_SIDEKICK2, FALSE );// SCG[1/24/00]: set the mode for the global inventory status
		AIINFO_SetLastSelectedSidekick(pSK);      // she's in command now
		com->Sidekick_Update(self,TYPE_MIKIKO,SIDEKICK_STATUS_COMMANDING,0); // update client
		SIDEKICK_CommandAcknowledge(pSK,SIDEKICK_COMMAND_COMMAND,true); // respond to given command

		gstate->CommandSelectorNext( self );
		gstate->CommandSelectorPrev( self );
	}
}

void SIDEKICK_SwapCommanding( userEntity_t *self )
{
    // if mikiko is last selected sidekick,
    //	 if superfly exists and is alive,
    //     set superfly to last selected sidekick
    //     send status message to client of swap
    //   end
    // else
    //   if superfly is the last selected sidekick
    //     if mikiko exists and is alive
    //       set mikiko to last selected sidekick
    //       send status message to client of swap
    //     end
    //   else
    //     no sidekicks exist to swap between

	if (gstate->GetCvar("paused") > 0.0f)
		return;

    userEntity_t *pMikiko, *pSuperfly, *pLastSelectedSidekick;
	
    pMikiko   = AIINFO_GetMikiko();														 // grab 'er
    pSuperfly = AIINFO_GetSuperfly();                          // grab 'im
	
    pLastSelectedSidekick = AIINFO_GetLastSelectedSidekick();	 // grab last in command
	
    if (!pLastSelectedSidekick) // no sidekick in command?  should have been set earlier.. 
    {
        // just to be sure, check for available sidekick and set as last selected
		
		if (pMikiko && AI_IsAlive(pMikiko))
		{
/*
			Inventory_SetMode( self, INV_MODE_SIDEKICK2, FALSE );// SCG[1/24/00]: set the mode for the global inventory status
			AIINFO_SetLastSelectedSidekick(pMikiko);      // she's in command now
			com->Sidekick_Update(self,TYPE_MIKIKO,SIDEKICK_STATUS_COMMANDING,0); // update client
			SIDEKICK_CommandAcknowledge(pMikiko,SIDEKICK_COMMAND_COMMAND,true); // respond to given command

			gstate->CommandSelectorNext( self );
			gstate->CommandSelectorPrev( self );*/
			SIDEKICK_SetCommanding(self, pMikiko, INV_MODE_SIDEKICK2);
		}
		else if (pSuperfly && AI_IsAlive(pSuperfly))
		{
/*
			Inventory_SetMode( self, INV_MODE_SIDEKICK1, FALSE );// SCG[1/24/00]: set the mode for the global inventory status
			AIINFO_SetLastSelectedSidekick(pSuperfly);  // he's in command now
			com->Sidekick_Update(self,TYPE_SUPERFLY,SIDEKICK_STATUS_COMMANDING,0); // update client
			SIDEKICK_CommandAcknowledge(pSuperfly,SIDEKICK_COMMAND_COMMAND,true); // respond to given command

			gstate->CommandSelectorNext( self );
			gstate->CommandSelectorPrev( self );*/
			SIDEKICK_SetCommanding(self, pSuperfly, INV_MODE_SIDEKICK1);
		}
    }
    else
    {
		if (pMikiko && pMikiko == pLastSelectedSidekick)  // mikiko exists and is last selected?
		{
			if (pSuperfly && AI_IsAlive(pSuperfly) )        // superfly exists and is alive?
			{
/*
				Inventory_SetMode( self, INV_MODE_SIDEKICK1, FALSE );// SCG[1/24/00]: set the mode for the global inventory status
				AIINFO_SetLastSelectedSidekick(pSuperfly);    // set superfly as commanding sidekick
				com->Sidekick_Update(self,TYPE_SUPERFLY,SIDEKICK_STATUS_COMMANDING,0); // update client
				SIDEKICK_CommandAcknowledge(pSuperfly,SIDEKICK_COMMAND_COMMAND,true); // respond to given command

				gstate->CommandSelectorNext( self );
				gstate->CommandSelectorPrev( self );*/
				SIDEKICK_SetCommanding(self, pSuperfly, INV_MODE_SIDEKICK1);
			}
			else
			{
				// there is no superfly but we still have to activate the sidekick command HUD just as normal
/*
				Inventory_SetMode( self, INV_MODE_SIDEKICK2, FALSE );// SCG[1/24/00]: set the mode for the global inventory status
				AIINFO_SetLastSelectedSidekick(pMikiko);      // she's in command now
				com->Sidekick_Update(self,TYPE_MIKIKO,SIDEKICK_STATUS_COMMANDING,0); // update client
				SIDEKICK_CommandAcknowledge(pMikiko,SIDEKICK_COMMAND_COMMAND,true); // respond to given command

				gstate->CommandSelectorNext( self );
				gstate->CommandSelectorPrev( self );*/
				SIDEKICK_SetCommanding(self, pMikiko, INV_MODE_SIDEKICK2);
			}
		}
		else
		{
			if (pSuperfly && pSuperfly == pLastSelectedSidekick) // superfly is last selected?
			{
				if (pMikiko && AI_IsAlive(pMikiko) )               // mikiko exists and alive?
				{
/*
					Inventory_SetMode( self, INV_MODE_SIDEKICK2, FALSE );// SCG[1/24/00]: set the mode for the global inventory status 
					AIINFO_SetLastSelectedSidekick(pMikiko);         // set mikiko as commanding sidekick
					com->Sidekick_Update(self,TYPE_MIKIKO,SIDEKICK_STATUS_COMMANDING,0); // update client
					SIDEKICK_CommandAcknowledge(pMikiko,SIDEKICK_COMMAND_COMMAND,true); // respond to given command

					gstate->CommandSelectorNext( self );
					gstate->CommandSelectorPrev( self );*/
					SIDEKICK_SetCommanding(self, pMikiko, INV_MODE_SIDEKICK2);
				}
				else
				{
					// there is no mikiko but we still have to activate the sidekick command HUD just as normal
					SIDEKICK_SetCommanding(self, pSuperfly, INV_MODE_SIDEKICK1);
				}    
			}
		}
    }
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_Remove
// Description: Removes sidkekick and performs cleanup
// Note:		Utility function for SIDEKICK_ShutdownSuperfly and SIDEKICK_ShutdownMikiko
//
// ----------------------------------------------------------------------------
void SIDEKICK_Remove( userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );

	self->solid    = SOLID_BBOX;
	self->movetype = MOVETYPE_BOUNCE;

    self->die		= NULL;
    self->prethink	= NULL;
    self->postthink = NULL;
    self->think		= NULL;

	if ( hook->pGoals )
	{
		GOALSTACK_Delete( hook->pGoals );
		hook->pGoals = NULL;
	}

	if ( hook->pScriptGoals )
	{
		GOALSTACK_Delete( hook->pScriptGoals );
		hook->pScriptGoals = NULL;
	}
	
	UNIQUEID_Remove( hook->szScriptName );
	free( hook->szScriptName );
	hook->szScriptName = NULL;

	alist_remove( self );
	gstate->RemoveEntity( self );
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_ShutdownSuperfly
// Description: Removes Superfly and performs cleanup
//
// ----------------------------------------------------------------------------
void SIDEKICK_ShutdownSuperfly()
{
	userEntity_t	*pSuperfly = AIINFO_GetSuperfly();

	_ASSERTE( pSuperfly );
	if( pSuperfly != NULL )
	{
		SIDEKICK_Remove( pSuperfly );
	}
}

// ----------------------------------------------------------------------------
//
// Name:        SIDEKICK_ShutdownMikiko
// Description: Removes Mikiko and performs cleanup
//
// ----------------------------------------------------------------------------
void SIDEKICK_ShutdownMikiko()
{
	userEntity_t	*pMikiko = AIINFO_GetMikiko();

	_ASSERTE( pMikiko );
	if( pMikiko != NULL )
	{
		SIDEKICK_Remove( pMikiko );
	}
}

// ----------------------------------------------------------------------------
//
// Name:        trigger_superfly_spawn_use
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void trigger_superfly_spawn_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator )
{
	SIDEKICK_Spawn( TYPE_SUPERFLY, self->s.origin, self->s.angles );

	gstate->RemoveEntity( self );
}

// ----------------------------------------------------------------------------
//
// Name:        trigger_mikiko_spawn_use
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void trigger_mikiko_spawn_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator )
{
	SIDEKICK_Spawn( TYPE_MIKIKO, self->s.origin, self->s.angles );

	gstate->RemoveEntity( self );
}

// ----------------------------------------------------------------------------
//
// Name:        trigger_superfly_spawn
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void trigger_superfly_spawn( userEntity_t *self )
{
	self->solid		= SOLID_NOT;
	self->movetype	= MOVETYPE_NONE;
	self->svflags	|= SVF_NOCLIENT;
	self->use		= trigger_superfly_spawn_use;
	gstate->SetModel (self, self->modelName);
	self->modelName = NULL;
	gstate->LinkEntity( self );
	
	CVector origin;
	origin.x = self->absmin.x + ( self->absmax.x - self->absmin.x );
	origin.y = self->absmin.y + ( self->absmax.y - self->absmin.y );
	origin.z = self->absmin.z + ( self->absmax.z - self->absmin.z );
	self->s.origin = origin;
}

// ----------------------------------------------------------------------------
//
// Name:        trigger_mikiko_spawn
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void trigger_mikiko_spawn( userEntity_t *self )
{
	self->solid		= SOLID_NOT;
	self->movetype	= MOVETYPE_NONE;
	self->svflags	|= SVF_NOCLIENT;
	self->use		= trigger_mikiko_spawn_use;
	gstate->SetModel (self, self->modelName);
	self->modelName = NULL;
	gstate->LinkEntity( self );
	
	CVector origin;
	origin.x = self->absmin.x + ( self->absmax.x - self->absmin.x );
	origin.y = self->absmin.y + ( self->absmax.y - self->absmin.y );
	origin.z = self->absmin.z + ( self->absmax.z - self->absmin.z );
	self->s.origin = origin;
}

// ----------------------------------------------------------------------------
// NSS[3/6/00]:
// Name:        SIDEKICK_Stats
// Description: Displays the satistics for the sidekick.
// Input:userEntity_t *self(sidekick)
// Output:
// ----------------------------------------------------------------------------
void SIDEKICK_Stats(userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook(self);
	if(hook)
	{
		weapon_t *pWeapon	= (weapon_t *) self->curWeapon;
		if(pWeapon && pWeapon->winfo)
		{
			float Ammo,Ammo_Max;
			Ammo_Max	= pWeapon->winfo->ammo_max;
			Ammo		= pWeapon->ammo->count;
			AI_Dprintf("Health:%f Armor:%f Weapon:%s Ammo:%d AmmoMax:%d\n",self->health,self->armor_val,pWeapon->name,Ammo,Ammo_Max);
		}
		else
		{
			AI_Dprintf("Health:%f Armor:%f\n",self->health,self->armor_val);
		}
	}
}

// ----------------------------------------------------------------------------
// NSS[3/6/00]:
// Name:        SIDEKICK_Stats_SF
// Description: Get the statistics for superfly/mikikofly
// Input:userEntity_t *self(client)
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_Stats_SF(userEntity_t *self)
{
	userEntity_t *ent = AIINFO_GetSuperfly();
	if(!ent)
		ent = AIINFO_GetMikikofly();
	if(ent)
	{
		SIDEKICK_Stats(ent);
	}
}


// ----------------------------------------------------------------------------
// NSS[3/6/00]:
// Name:        SIDEKICK_Stats_MK
// Description: Get the statistics for mikiko
// Input:userEntity_t *self(client)
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void SIDEKICK_Stats_MK(userEntity_t *self)
{
	userEntity_t *ent = AIINFO_GetMikiko();
	if(ent)
	{
		SIDEKICK_Stats(ent);
	}
}


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_sidekick_register_func()
{
	gstate->RegisterFunc("SIDEKICK_FindOwnerThink",SIDEKICK_FindOwnerThink);
	gstate->RegisterFunc("SIDEKICK_NitroDeath",SIDEKICK_NitroDeath);
	gstate->RegisterFunc("SIDEKICK_Think",SIDEKICK_Think);
	gstate->RegisterFunc("SIDEKICK_AttackPlayerThink",SIDEKICK_AttackPlayerThink);
	gstate->RegisterFunc("SIDEKICK_StopAttackPlayer",SIDEKICK_StopAttackPlayer);
	gstate->RegisterFunc("SIDEKICK_Pain",SIDEKICK_Pain);
	gstate->RegisterFunc("SIDEKICK_StartDie",SIDEKICK_StartDie);
	gstate->RegisterFunc("SIDEKICK_PreThink",SIDEKICK_PreThink);
	gstate->RegisterFunc("SIDEKICK_PostThink",SIDEKICK_PostThink);
	gstate->RegisterFunc("SIDEKICK_InAttackRange",SIDEKICK_InAttackRange);
	gstate->RegisterFunc("AI_SaveSidekick",AI_SaveSidekick);
//	gstate->RegisterFunc("AI_LoadSidekick",AI_LoadSidekick);
	gstate->RegisterFunc("AI_LoadSidekick",AI_LoadSidekick);
	gstate->RegisterFunc("Sidekick_Touch",Sidekick_Touch);
	gstate->RegisterFunc("SIDEKICK_RipgunThink",SIDEKICK_RipgunThink);	

	gstate->RegisterFunc("trigger_superfly_spawn_use",trigger_superfly_spawn_use);
	gstate->RegisterFunc("trigger_mikiko_spawn_use",trigger_mikiko_spawn_use);
}
