//---------------------------------------------------------------------------
// DAIKATANA - Melee
//---------------------------------------------------------------------------
#include "weapons.h"
#include "daikatana.h"

enum {
	// model defs
	MDL_START,
	MDL_RING,
	MDL_DAIKATANA_NOHANDS,

	// sound defs
	SND_START,
	SND_WIND1,
	SND_WIND2,
	SND_CHANT1,
	SND_CHANT2,
	SND_AMBIENT1,
	SND_AMBIENT2,
	SND_AMBIENT3,
	SND_EHIT1,
	SND_EHIT2,

	// special frame defs
	ANIM_START,					// a
	ANIM_SHOOT2,				// b 
	ANIM_SHOOT3,				// c -- low pitch
	ANIM_SHOOT4,				// d -- low pitch
	ANIM_SHOOT5,				// e -- sounds fast
	ANIM_SHOOT6,
	ANIM_SHOOT7,
	ANIM_SHOOT8,
	ANIM_SHOOT9
};

// weapon info def
weaponInfo_t daikatana_WeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_DAIKATANA),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_NO_AMMO),
	// names, net names, icon
	"weapon_daikatana",
	tongue_weapons[T_WEAPON_DAIKATANA],
	"ammo_none",
	tongue_weapons[T_WEAPON_NO_AMMO],
	"iconname",

	// model files
	{"models/global/w_daikatana.dkm",
	"models/global/a_daikatana.dkm",
	"models/e4/wa_glock.dkm",
	"models/global/we_dklevel.dkm",
	"models/global/ws_daikatana.dkm",
	NULL},

	// sound files
	{SND_NOSOUND,
	"global/we_swordwhoosha.wav",		// ready and away!
	"global/we_swordwhooshc.wav",		
	SND_NOSOUND,						
	SND_NOSOUND,						
	SND_NOSOUND,
	SND_NOSOUND,
	"global/e_windc.wav",				// wind for the level up
	"global/e_windb.wav",
	"global/we_dk_cnt_01.wav",			// chants for the level up
	"global/we_dk_cnt_02.wav",
	"global/we_dk_01.wav",				// ambients
	"global/we_dk_02.wav",
	"global/we_dk_03a.wav",
	"global/e_electronsprke.wav",
	"global/e_electronsprkh.wav"
	},

	// frame names
	{MDL_WEAPON_STD,"ready",0,0,
		-1,"away",0,0,
		-1,"ataka",0,0,
		-1,"amba",0,0,
      ANIM_NONE,
      ANIM_NONE,
		-1,"atakb",0,0,
		-1,"atakc",0,0,
		ANIM_NONE,//doesn't exist-1,"atakd",0,0,
		-1,"atake",0,0,
		-1,"atakf",0,0,
		-1,"atakg",0,0,
		ANIM_NONE,//doesn't exist-1,"atakh",0,0,
		ANIM_NONE,//doesn't exist-1,"ataki",0,0,
	0,NULL},

	// commands
	{ 
      wpcmds[WPCMDS_DAIKATANA][WPCMDS_GIVE], (void (*)(struct edict_s *))daikatana_give,
	  wpcmds[WPCMDS_DAIKATANA][WPCMDS_SELC], (void (*)(struct edict_s *))daikatana_select,
	  NULL
    },

	daikatana_give,
	daikatana_select,
	daikatana_command,
	daikatana_use,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(daikatana_t),

	200,
	1000,		//	ammo_max
	1,			//	ammo_per_use
	10,			//	ammo_display_divisor
	0,			//	initial ammo
	0,			//	speed
	100,		//	range
	10,			//	damage
	0.0,		//	lifetime

	WFL_NODROP,	//	flags
	WEAPONTYPE_PISTOL,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_daikatana[T_DEATHMSGWEAPON_DK_1],
		NULL,
		VICTIM_ATTACKER
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_daikatana,T_DEATHMSGWEAPON_DK_1),
	VICTIM_ATTACKER,

    WEAPON_DKMELEE,
	0,
	NULL
};

// local weapon info access
static weaponInfo_t *winfo=&daikatana_WeaponInfo;

#define DK_ATTACK_NONE		0X00000000
#define DK_ATTACKA			0X00000001
#define DK_ATTACKB			0x00000002
#define DK_ATTACKC			0x00000004
#define DK_ATTACKD			0x00000008
#define DK_ATTACKE			0x00000010
#define DK_ATTACKF			0x00000020
#define DK_ATTACKG			0x00000040
#define DK_ATTACKH			0x00000080
#define DK_ATTACKI			0x00000100
#define DK_ATTACKJ			0x00000200

#define DK_ATTACK_FLAGS		0x0FFFFFFF		// quick check for attack flags
// these flags override all other anim flags
#define DK_ATTACK_LAST		0X40000000		// this is the last swipe of this sequence
#define DK_ATTACK_MORE		0x80000000		// this attack has multiple swipes in it

#define DK_MAX_LEVELS			15
enum
{
	SND_SWIPEA,			// swipes
	SND_SWIPEB,
	SND_SWIPEC,			// low pitch
	SND_SWIPED,			// low pitch
	SND_SWIPEE,
	SND_SWIPEF,

	SND_SW_HIT1,			// hit a world surface
	SND_SW_HIT2,
	SND_SW_HIT3,
	SND_SW_HIT4,
	SND_SW_HIT5,

	SND_SW_ARMOR_HIT1,
	SND_SW_ARMOR_HIT2,
	SND_SW_ARMOR_HIT3,
	SND_SW_ARMOR_HIT4,
	SND_SW_ARMOR_HIT5,

	SND_SW_CLANK1,		// hits another sword!
	SND_SW_CLANK2,
	SND_SW_CLANK3,
	SND_SW_CLANK4,
	SND_SW_CLANK5,

	SND_SW_BODYHIT1,
	SND_SW_BODYHIT2,
	SND_SW_BODYHIT3,
	SND_SW_BODYHIT4,

	SND_SW_MAX
};

static char *daikatana_sounds[] =
{
	"global/we_swordwhoosha.wav",
	"global/we_swordwhooshb.wav",
	"global/we_swordwhooshc.wav",
	"global/we_swordwhooshd.wav",
	"global/we_swordwhooshe.wav",
	"global/we_swordwhooshf.wav",

	"global/m_swordhita.wav",
	"global/m_swordhitb.wav",
	"global/m_swordhitc.wav",
	"global/m_swordhitd.wav",
	"global/m_swordhite.wav",

	"global/m_armorhita.wav",
	"global/m_armorhitb.wav",
	"global/m_armorhitc.wav",
	"global/m_armorhitd.wav",
	"global/m_armorhite.wav",

	"global/we_swordwclanka.wav",
	"global/we_swordwclankb.wav",
	"global/we_swordwclankc.wav",
	"global/we_swordwclankd.wav",
	"global/we_swordwclanke.wav",

	"global/we_swordstaba.wav",
	"global/we_swordstabb.wav",
	"global/we_swordstabc.wav",
	"global/we_swordstabd.wav",
};

// defines to describe the position of the sword tip.
// will be normalized and multiplied by winfo->range!
#define FRU_FORWARD				winfo->range,0,0
#define FRU_RIGHT				0.25*winfo->range,winfo->range,0
#define FRU_LEFT				0.25*winfo->range,-winfo->range,0
#define FRU_UP					0,0,winfo->range
#define FRU_DOWN				0,0,-winfo->range
#define FRU_UPRIGHT				0.25*winfo->range,winfo->range,winfo->range
#define FRU_UPLEFT				0.25*winfo->range,-winfo->range,winfo->range
#define FRU_DNRIGHT				0.25*winfo->range,winfo->range,-0.25*winfo->range
#define FRU_DNLEFT				0.25*winfo->range,-winfo->range,-0.25*winfo->range

// a bit off center (to make hitting easier)
#define FRU_FWDLEFT				winfo->range,-0.25*winfo->range,0
#define FRU_FWDRIGHT			winfo->range,0.25*winfo->range,0

#define SCORCH_DOT				-1
#define SCORCH_HORIZ			0
#define SCORCH_DR				45
#define SCORCH_VERT				90
#define SCORCH_DL				135

#define DK_AFTER_A				(DK_ATTACKF|DK_ATTACKC|DK_ATTACKE|DK_ATTACKG)
#define DK_AFTER_B				(DK_ATTACKA|DK_ATTACKB|DK_ATTACKG)
#define DK_AFTER_C				(DK_ATTACKA|DK_ATTACKB|DK_ATTACKE)
#define DK_AFTER_D				(DK_ATTACK_LAST)
#define DK_AFTER_E				(DK_ATTACKA|DK_ATTACKC|DK_ATTACKF|DK_ATTACKG)
#define DK_AFTER_F				(DK_ATTACKA|DK_ATTACKB|DK_ATTACKE|DK_ATTACKG)
#define DK_AFTER_G				(DK_ATTACKB|DK_ATTACKC|DK_ATTACKG|DK_ATTACKE)
#define DK_AFTER_H				(DK_ATTACK_LAST)
#define DK_AFTER_I				(DK_ATTACK_LAST)

// anim, {damage frame, speed, sound,next anim}
static daikatanaInfo_t meleeInfo[] = {
	{ANIM_SHOOT_STD,{	{7, 11, FRU_RIGHT, FRU_FWDLEFT, SCORCH_HORIZ, 1.0, -1, DK_AFTER_A}}},		// A	R TO L
	{ANIM_SHOOT2,	{	{7, 11, FRU_UPRIGHT, FRU_FWDLEFT, SCORCH_DL, 1.0, -1, DK_ATTACK_MORE},		// B1	UR TO LL
						{18, 24, FRU_DNLEFT, FRU_FWDRIGHT, SCORCH_HORIZ, 1.0, -1, DK_AFTER_B}}},		// B2	L TO R
	{ANIM_SHOOT3,	{	{7, 11, FRU_UPLEFT, FRU_FWDRIGHT, SCORCH_DR, 1.0, -1, DK_AFTER_C}}},			// C	UL TO LR
	{EMPTY_HIT},																					// D
	{ANIM_SHOOT5,	{	{15, 22, NULL_FRU, NULL_FRU, SCORCH_DOT, 1.0, -1, DK_AFTER_E}}},			// E	JAB	
	{ANIM_SHOOT6,	{	{6, 10, FRU_LEFT, FRU_FWDRIGHT, SCORCH_HORIZ, 1.0, -1, DK_AFTER_F}}},			// F	L TO R
	{ANIM_SHOOT7,	{	{10, 16, FRU_UP, FRU_FORWARD, SCORCH_VERT, 1.0, -1, DK_AFTER_G}}},			// G	up - down
	{EMPTY_HIT},																					// H
	{EMPTY_HIT}																					// I
};

// first sound, last sound, volume
static sound_info_t daikatana_sound_info[] = 
{
	{SND_SWIPEA,SND_SWIPEF,0.4},				// SND_CH_SWIPE
	{SND_SW_HIT1,SND_SW_HIT5,0.7},				// SND_CH_HIT
	{SND_SW_CLANK1,SND_SW_CLANK5,0.7},			// SND_CH_SW_CLANK
	{SND_SW_ARMOR_HIT1,SND_SW_ARMOR_HIT5,0.7},	// SND_CH_ARMOR_HIT
	{SND_SW_BODYHIT1,SND_SW_BODYHIT4,0.3}		// SND_CH_BODY_HIT
};

static amb_info_t amb_info[] = 
{
	{-1,-1},
	{SND_AMBIENT1,0.35},
	{SND_AMBIENT2,0.55},
	{SND_AMBIENT3,0.55},
	{SND_AMBIENT3,0.65}
};

static long daikatana_exp_levels[] =
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

//#define DK_RESTART_LATENCY			(1.0 - 0.05 * (calcDKLevel(self) - 1))
#define DK_RESTART_LATENCY			((phook && phook->attack_boost) ? (1.0 - 0.1 * phook->attack_boost) : 1.0)
#define DK_DONE_LATENCY				(0.2)
// takes a set of animation flags and returns a random animation index
// from that set.
int daikatana_get_attack(userEntity_t *self, int flags)
{
	// count number of bits on in the flag
	int flags2 = flags;
	int i,count = 0;
	for (i = 0; i < 32; i++)
	{	
		count += flags & 1;
		flags >>= 1;
	}
	if (!count)
		return -1;

	// choose a random anim
	count = (int)(frand() * (count - 0.05)) + 1;
	for (i = 0; i < 32; i++)
	{
		if (flags2 & 1)
		{
			if (!(--count))
				return i;
		}
		flags2 >>= 1;
	}

	return -1;
}

//---------------------------------------------------------------------------
// daikatana_give()
//---------------------------------------------------------------------------
userInventory_t *daikatana_give(userEntity_t *self, int ammoCount)
{
   return(weaponGive (self, winfo, ammoCount));
}

//---------------------------------------------------------------------------
// daikatana_track() returns a trackInfo_t set up to track the daikatana
//---------------------------------------------------------------------------
short daikatana_track(trackInfo_t &tinfo, userEntity_t *self)
{
	if (!self)
		return 0;

	memset(&tinfo,0,sizeof(tinfo));
	weaponTrackMuzzle(tinfo,self,self,winfo,true);//(userEntity_t *)self->client->ps.weapon,self,winfo);
	tinfo.modelindex = gstate->ModelIndex("models/global/e_flare4+.sp2");
	tinfo.Long2 = gstate->ModelIndex(winfoModel(MDL_DAIKATANA_NOHANDS));
	tinfo.scale = 0.2;
	tinfo.scale2 = 0.3;
	tinfo.numframes = calcDKLevel(self);
	tinfo.lightColor.Set(0.2,0.2,1);		// fx color!
	tinfo.altpos2.Set(-1,-1,1+frand());		// neg. light color
	tinfo.flags |= TEF_SCALE|TEF_SCALE2|TEF_MODELINDEX|TEF_FXFLAGS|TEF_NUMFRAMES|TEF_LONG2|TEF_LIGHTCOLOR|TEF_ALTPOS2;
	tinfo.fxflags |= TEF_DAIKATANA|TEF_FX_ONLY;

	return 1;
}

void daikatana_ambient(userEntity_t *self)
{
	userEntity_t *weap = (userEntity_t *)self->client->ps.weapon;
	if (!weap)
		return;

	winfoStopLooped(self);
	short level = calcDKLevel(self);
	if ((level < 1) || (level > 5))
		return;

	amb_info_t inf = amb_info[level-1];
	if ((inf.sound != -1) && (inf.vol > 0))
		winfoPlayLooped(weap,inf.sound,inf.vol);
}

void daikatana_swipe(userEntity_t *self, daikatanaHitInfo_t &hit)
{
/*	short level = calcDKLevel(self);
	if (level > 1)
		winfoPlay(self,SND_AT1+level-2,1.0);
*/
	daikatana_playSound(self,SND_CH_SWIPE,hit.sound);
}
//---------------------------------------------------------------------------
// daikatana_select()
//---------------------------------------------------------------------------
short daikatana_select(userEntity_t *self)
{
	short rt = weaponSelect(self,winfo);
	if ((rt == WEAPON_CHANGED) || (rt == WEAPON_ACTIVE))
	{
		daikatana_t *dk_melee;
		if ( dk_melee=(daikatana_t *)gstate->InventoryFindItem(self->inventory,winfo->weaponName) )
		{
			dk_melee->attacking = dk_melee->chain = 0;
		}
	}
	return rt;
}

//---------------------------------------------------------------------------
// daikatana_command()
//---------------------------------------------------------------------------
void *daikatana_command(struct userInventory_s *inv, char *commandStr, void *data)
{
	// do some stuff when we change from the dk BUT STILL CALL THE DEFAULT!!
	if (!stricmp(commandStr,"change"))
	{
		userEntity_t *self=(userEntity_t *)data;
		if (!self)
			return winfoGenericCommand(winfo,inv,commandStr,data);

		userEntity_t *weap = (userEntity_t *)self->client->ps.weapon;
		if (weap)
			winfoStopLooped(weap);

		daikatana_t *dk_melee;
		if ( dk_melee=(daikatana_t *)gstate->InventoryFindItem(self->inventory,winfo->weaponName) )
		{
			dk_melee->attacking = dk_melee->chain = 0;
		}

		RELIABLE_UNTRACK(self);
	}

	return winfoGenericCommand(winfo,inv,commandStr,data);
}

//---------------------------------------------------------------------------
// daikatana_SetAttack()
// setup the daikatana's next attack anim (-1 is random)
//---------------------------------------------------------------------------
void daikatana_SetAttack(userEntity_t *self, int index)
{
	playerHook_t *phook=(playerHook_t *)self->userHook;
	daikatana_t *dk_melee=(daikatana_t *)self->curWeapon;
	if (!phook || !dk_melee)
		return;

	short numAttacks = sizeof(meleeInfo)/sizeof(meleeInfo[0]);
/*	if ( (index < 0) || (index >= numAttacks))
	{
		if (self && (self->flags & FL_CLIENT))
		{
			if ( self->client->v_angle.Pitch() < -40 )	// up, pick jab or bash
			{
				index =  (rand() & 1) ? 4 : 6;
			}
			else if ( self->client->v_angle.Pitch() > 40 )	// down, pick jab
			{
				index = 4;
			}
		}
		else
		{
			// pick an attack.  if index is invalid, randomize, otherwise, just use index
			index = (short)(frand() * (numAttacks - .05));
		}
	}
*/

	if ( (index < 0) || (index >= numAttacks))
		index = (short)(frand() * (numAttacks - .05));

	daikatanaInfo_t *melee = &meleeInfo[index];
	if (melee->anim == -1)		// empty -- pick another...
	{
		daikatana_SetAttack(self,-1);
		return;
	}

//	gstate->Con_Printf("dk attack %c, level %d\n",index + 'a',dk_melee->levels);

	dk_melee->meleeNum=index;
	dk_melee->hitNum=0;
	dk_melee->levels++;

	daikatanaHitInfo_t *hit = &melee->hitInfo[0];

	// set up the animation and frame funcs
	float speed = 0.04 - (0.004 * calcDKLevel(self));
	speed /= hit->speed;

/*	if(phook->attack_boost)
	{
		speed /= ((phook->attack_boost+1) * 0.5);
	}
*/
	if (speed < 0.02)
		speed = 0.02;
	// compute attack boost by hand...can't go overboard with the speed of this anim...

	float delay = 0.5 * speed * hit->damageFrame;
	if (delay < 0.1) delay = 0.1;

	winfoAnimate(winfo,self,melee->anim,FRAME_ONCE|FRAME_WEAPON|FRAME_NODELAY,speed);
	phook->fxFrameNum=winfoFirstFrame(melee->anim)+hit->damageFrame;

	dk_melee->doneTime = gstate->time + DK_DONE_LATENCY;			// give some time to hit the attack btn again...
	phook->attack_finished = gstate->time + delay;//speed * hit->damageFrame;
	phook->fxFrameFunc=daikatana_think;
	phook->ambient_time = gstate->time + 3;				// no ambs while animating!!!

	dk_melee->attacking = 1;
	dk_melee->chain = 0;
}

//---------------------------------------------------------------------------
// daikatana_followThrough()
// finishes the current swing anim and then spawns a new one.
//---------------------------------------------------------------------------
void daikatana_followThrough(userEntity_t *self)
{
	playerHook_t *phook=(playerHook_t *)self->userHook;
	if (!phook)
		return;

	userEntity_t *weap = (userEntity_t *)self->client->ps.weapon;
	daikatana_t *dk_melee=(daikatana_t *)self->curWeapon;

	// some sanity checking
	if ( (!dk_melee) || (!weap) || (stricmp(weap->className,"weapon_daikatana")) )
	{
		phook->fxFrameNum=-1;
		phook->fxFrameFunc = NULL;
		phook->attack_finished = gstate->time + DK_RESTART_LATENCY;
		if (dk_melee)
		{
			dk_melee->attacking = dk_melee->chain = 0;
			dk_melee->doneTime = gstate->time;
		}
		return;
	}

	daikatanaInfo_t *melee=&meleeInfo[dk_melee->meleeNum];
	daikatanaHitInfo_t *hit=&melee->hitInfo[dk_melee->hitNum];

	int index = daikatana_get_attack(self, hit->nextAnim);
	if (index == -1)
	{
		phook->fxFrameNum=-1;
		phook->fxFrameFunc = NULL;
		phook->attack_finished = gstate->time + DK_RESTART_LATENCY;
		dk_melee->doneTime = gstate->time + 0.05;
		dk_melee->attacking = dk_melee->chain = 0;
		return;
	}

//	gstate->Con_Printf("spawning dk attack %c\n",index + 'a');
	daikatana_SetAttack(self,index);	
}

//---------------------------------------------------------------------------
// think()
//---------------------------------------------------------------------------
void daikatana_think(userEntity_t *self)
{
	userEntity_t *weap = (userEntity_t *)self->client->ps.weapon;
	daikatana_t *dk_melee=(daikatana_t *)self->curWeapon;
	playerHook_t *phook=(playerHook_t *)self->userHook;

	if (!phook)
		return;

	if ( (!dk_melee) || (!weap) || (stricmp(weap->className,"weapon_daikatana")) )
	{
		phook->fxFrameNum=-1;
		phook->fxFrameFunc = NULL;
		phook->attack_finished = gstate->time + DK_RESTART_LATENCY;
		if (dk_melee)
		{
			dk_melee->attacking = dk_melee->chain = 0;
			dk_melee->doneTime = gstate->time;
		}
		return;
	}

	daikatanaInfo_t *melee=&meleeInfo[dk_melee->meleeNum];
	daikatanaHitInfo_t *hit=&melee->hitInfo[dk_melee->hitNum];

	daikatana_attack(self,*hit);

	// more attacks(swipes) in this anim -- this is forced to finish!
	if ((hit->nextAnim & DK_ATTACK_MORE))
	{
		dk_melee->hitNum++;
		hit++;
		dk_melee->doneTime = gstate->time + DK_DONE_LATENCY;			// give some time to hit the attack btn again...
		phook->attack_finished = gstate->time + 0.1;
		phook->fxFrameNum=winfoFirstFrame(melee->anim)+hit->damageFrame;
		phook->fxFrameFunc = daikatana_think;
		return;
	}
	
	// last attack?
	short maxLevels = 2 * calcDKLevel(self);
	if (!dk_melee->chain || (hit->nextAnim & DK_ATTACK_LAST) || (dk_melee->levels >= maxLevels) /* || (dk_melee->doneTime <= gstate->time)*/ || !(hit->nextAnim & DK_ATTACK_FLAGS))
	{
		dk_melee->attacking = dk_melee->chain = 0;
		dk_melee->meleeNum = -1;
		phook->fxFrameNum=-1;
		phook->fxFrameFunc = NULL;
		if (dk_melee->levels >= maxLevels)
			phook->attack_finished = gstate->time + DK_RESTART_LATENCY;
		else
			phook->attack_finished = gstate->time + 0.5 * DK_RESTART_LATENCY;			// give some time to hit the attack btn again...
		dk_melee->doneTime = gstate->time + 0.05;
		return;
	}

	// we're going to try to spawn another attack...finish this anim's followthrough
	dk_melee->doneTime = gstate->time + DK_DONE_LATENCY;			// give some time to hit the attack btn again...
	phook->attack_finished = gstate->time + 0.1;
	phook->fxFrameNum = winfoFirstFrame(melee->anim) + hit->followThrough;
	phook->fxFrameFunc = daikatana_followThrough;
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_daikatana_register_func()
{
	gstate->RegisterFunc("dk_meleeThink",daikatana_think);
	gstate->RegisterFunc("dk_meleeuse",daikatana_use);
}

//---------------------------------------------------------------------------
// daikatana_traceArc()
// improved tracing for the daikatana swings.  traces out a simple arc to 
// find the hit point.
//---------------------------------------------------------------------------
#define TRACE_BOX_SIZE					(16)
trace_t daikatana_traceArc(userEntity_t *self, CVector end, CVector start, bool bUseBox)
{
	trace_t result;
	CVector org = self->s.origin;
	CVector dir = self->client->v_angle;
	dir.AngleToVectors(forward,right,up);
	unsigned long mask = MASK_SHOT;
	mask &= ~CONTENTS_NOSHOOT;

	// SCG[1/24/00]: make sure that if we early out, we don't send bogus data!
	memset( &result, 0, sizeof( trace_t ) );

	if (end.Length() < 0.9)
	{
		start = org;
		end = start + forward * winfo->range;
		result = gstate->TraceLine_q2 (start, end, self, mask);
		return result;
	}
	else
	{
		CVector _start = start,_end = end, end2;
		CVector mins(-TRACE_BOX_SIZE,-TRACE_BOX_SIZE,-TRACE_BOX_SIZE), maxs(TRACE_BOX_SIZE,TRACE_BOX_SIZE,TRACE_BOX_SIZE);

		float step = (bUseBox) ? 0.2 : 0.1;

		for (float frac = 0.01; frac < 1.0; frac += step)
		{
			start = _start;
			end = _end;

			start.Normalize();
			start.Multiply(frac * winfo->range);
			end.Normalize();
			end.Multiply(frac * winfo->range);

			// interpolate to the mid point
			end2 = end.Interpolate(start,end,0.5);
			end2.Normalize();
			end2.Multiply(frac * winfo->range);

			// determine the mid-point
			org = self->s.origin;
			org += end2.x * forward;
			org += end2.y * right;
			org += end2.z * up;
			end2 = org;

			// starting point
			org = self->s.origin;
			org += start.x * forward;
			org += start.y * right;
			org += start.z * up;
			start = org;

			if (bUseBox)
				result=gstate->TraceBox_q2(start,mins,maxs,end2,self,mask);
			else
				result = gstate->TraceLine_q2 (start, end2, self, mask);
			if (result.fraction != 1.0)
			{
				return result;
			}

			// ending point
			org = self->s.origin;
			org += end.x * forward;
			org += end.y * right;
			org += end.z * up;
			end = org;

			if (bUseBox)
				result=gstate->TraceBox_q2(start,mins,maxs,end,self,mask);
			else
				result = gstate->TraceLine_q2 (end2, end, self, mask);
			if (result.fraction != 1.0)
			{
				return result;
			}
		}
	}
	return result;
}

//---------------------------------------------------------------------------
// daikatana_attack()
// a swing has reached its damage frame...see what we got.
//---------------------------------------------------------------------------
void daikatana_attack ( userEntity_t *self, daikatanaHitInfo_t &hit)
{
//    playerHook_t *phook=(playerHook_t *)self->userHook;// SCG[1/24/00]: not used
    trace_t         tr;

	// power boost!!!!
    weapon_PowerSound(self);

	WEAPON_FIRED(self);
	// handle the sword swipes
	daikatana_swipe(self,hit);

    CVector dir = self->client->v_angle;
    dir.AngleToVectors (forward, right, up);
    CVector start = self->s.origin + forward * 8.0 + up * 16;    
//	CVector end = start + forward * winfo->range;// SCG[1/24/00]: not used

	CVector damageFRU, startFRU;
	damageFRU.Set(hit.damageF,hit.damageR,hit.damageU);
	startFRU.Set(hit.startF,hit.startR,hit.startU);

	tr = daikatana_traceArc(self, damageFRU, startFRU, true);

    if ( tr.fraction!= 1.0 )
    {
        //Make sure we are damaging something that is meant to be damaged
        if ( tr.ent && (tr.ent->takedamage != DAMAGE_NO) )
        {
			userEntity_t *attackee = tr.ent;
			CVector dieOrg = attackee->s.origin;
            dir = tr.endpos - start;
            dir.Normalize ();

			float damage = winfo->damage + (calcDKLevel(self) - 1) * 10;

			// see if we're behind the target
			AngleToVectors(self->s.angles,forward);		// forward is player's fwd
			AngleToVectors(attackee->s.angles,right);	// right is player's right
			forward.z = right.z = 0;
			forward.Normalize();
			right.Normalize();

			short attackeeFleshy = Weapon_is_fleshy(attackee);

			short flags = attackee->flags;
			float dp = DotProduct(forward,right);
			if ( (dp >= 0.85) && (dp < 1) )
			{
				damage *= 2;
			}
			else if ( (dp <= -0.85) && (dp >= -1))		// reasonably facing the enemy
			{
				// do cool stuff if the attackee is wielding a daikatana.... 
				if ((attackeeFleshy & IS_SWORD) || (flags & FL_CLIENT) && (attackee->client))
				{
					userEntity_t *went;
					if ((flags & FL_CLIENT) && (went = (userEntity_t *)attackee->client->ps.weapon))
					{
						if (!stricmp(went->className,"weapon_daikatana"))
						{
							damage *= 0.5;
							daikatana_playSound(self,SND_CH_SW_CLANK,-1);
						}
					}
					else if ((attackeeFleshy & IS_SWORD))
						daikatana_playSound(self,SND_CH_SW_CLANK,-1);
				}
			}

			long exp_bonus = daikatana_calcExpBonus(self,attackee);

			// not even the mighty daikatana can bust down medusa or the column (or cerberus)
			if (!IS_MONSTER_TYPE(attackee,TYPE_COLUMN) && !IS_MONSTER_TYPE(attackee,TYPE_MEDUSA) && !IS_MONSTER_TYPE(attackee,TYPE_CERBERUS))
			{
				// cek[1-24-00]: set the attackweapon
				winfoAttackWeap;
				WEAPON_HIT(self,1);
				com->Damage (attackee, (edict_s *)self->client->ps.weapon, self, tr.endpos, dir, damage, 0);
			}

			// do cool stuff after the attack
			if (flags & (FL_CLIENT|FL_MONSTER|FL_BOT))
			{
				if (EntIsAlive(attackee) || (gstate->damage_inflicted <= 0))
				{
					if (attackee->armor_val || (attackeeFleshy & IS_ARMOR) )
						daikatana_playSound(self,SND_CH_ARMOR_HIT,-1);
					else
						daikatana_playSound(self,SND_CH_BODY_HIT,-1);
				}
				else
				{
					trackInfo_t tinfo;
					if (exp_bonus && daikatana_track(tinfo,self))
					{
						short newLevel,oldLevel = calcDKLevel(self);
						// award the experience
						self->record.dkexp += exp_bonus;  // add exps	
						if ((newLevel = calcDKLevel(self)) > oldLevel)
						{
							daikatana_levelUp(self,newLevel);
							tinfo.Long1 |= TEF_DK_LEVEL2;
							tinfo.numframes = newLevel;
							winfoPlay(self,SND_CHANT1 + (rand() & 1),1.0);
							daikatana_ambient(self);
						}

						// tell the sword to do fx level 1 (vanquish)
						tinfo.Long1 |= TEF_DK_LEVEL1;
						tinfo.altpos = dieOrg;
						tinfo.flags |= TEF_LONG1|TEF_ALTPOS;
						com->trackEntity(&tinfo,MULTICAST_ALL);
					}
					daikatana_playSound(self,SND_CH_BODY_HIT,-1);
				}
			}
			else
			{
				if (IS_MONSTER_TYPE(attackee,TYPE_GOLDFISH))
					daikatana_playSound(self,SND_CH_BODY_HIT,-1);
				else
					daikatana_playSound(self,SND_CH_HIT,-1);
			}
			
			if (sv_violence->value)
 				clientSparks(tr.endpos,tr.plane.normal,5,CVector(0.7,0.7,0.8));
			else
			{
				dir = tr.endpos;
				dir.z -= 100;
				weaponBloodSplat (tr.ent, tr.endpos, start, tr.endpos, 96.0);
			}
       }
        else if ( tr.ent && tr.ent->solid == SOLID_BSP)
        {
			winfoPlay(self,SND_EHIT1 + (rand() & 1),0.6);
			daikatana_playSound(self,SND_CH_HIT,-1);
			tr = daikatana_traceArc(self, damageFRU, startFRU, false);
			daikatana_marks(self,tr,hit);
			clientSparks(tr.endpos,tr.plane.normal,5,CVector(0.7,0.7,0.8));
        }
    }
}

//---------------------------------------------------------------------------
// daikatana_playSound()
// cool sound randomizer for the daikatana sounds.  
//---------------------------------------------------------------------------
//					type			 != -1 to force a sound
void daikatana_playSound(userEntity_t *self, int type, int snd)
{
	daikatana_t *dk_melee=(daikatana_t *)self->curWeapon;
	if (!dk_melee)
		return;

	if ((type < SND_CH_MIN) || (type >= SND_CH_MAX))
	{
		memset(&dk_melee->lastSnd,0,sizeof(dk_melee->lastSnd));
		return;
	}

	float vol = daikatana_sound_info[type].vol;
	if (snd != -1)
	{
		gstate->StartEntitySound(self,CHAN_AUTO,gstate->SoundIndex(daikatana_sounds[snd]),vol,ATTN_NORM_MIN,ATTN_NORM_MAX);
		dk_melee->lastSnd[type] = snd;
	}
	else
	{
		int index;
		int lf = daikatana_sound_info[type].last - daikatana_sound_info[type].first;
		if (lf > 0)
		{
			index = daikatana_sound_info[type].first + (int)((lf + 0.995) * frand());
			if (index == dk_melee->lastSnd[type])
				index = daikatana_sound_info[type].first + (index + 1) % lf;
		}
		else
			index = daikatana_sound_info[type].first;

		gstate->StartEntitySound(self,CHAN_AUTO,gstate->SoundIndex(daikatana_sounds[index]),vol,ATTN_NORM_MIN,ATTN_NORM_MAX);
		dk_melee->lastSnd[type] = index;
	}
}

//---------------------------------------------------------------------------
// daikatana_use()
// bust somebody down!
//---------------------------------------------------------------------------
void daikatana_use(userInventory_s *ptr, userEntity_t *self)
{
	daikatana_t *dk_melee=(daikatana_t *)self->curWeapon;
	playerHook_t *phook=(playerHook_t *)self->userHook;

	if (!dk_melee || !phook)
		return;


/*	if (gstate->time >= dk_melee->doneTime)
	{
		dk_melee->levels = 0;
		daikatana_SetAttack(self,-1);
	}
	else
		phook->attack_finished = gstate->time + 0.1;		// delay the finish of the attack...

	dk_melee->doneTime = gstate->time + DK_DONE_LATENCY;			// give some time to hit the attack btn again...*/

	if (!dk_melee->attacking)
	{
		dk_melee->levels = 0;
		daikatana_SetAttack(self,-1);
	}
	else
		dk_melee->chain = 1;
}

// ripped this straight out of client.cpp...cuz I need it!
int calcStatLevel( userEntity_t *self )
{
	playerHook_t*	hook = (playerHook_t*)self->userHook;
	
	return (hook->base_power + hook->base_attack + hook->base_speed +
			hook->base_acro + hook->base_vita);
}

//---------------------------------------------------------------------------
// int calcDKLevel(userEntity_t *self)
// finds the current dk level in the dk_exp_levels table (minimum of 1)
//---------------------------------------------------------------------------
int calcDKLevel(userEntity_t *self)
{
	if (!self)
		return 1;

#ifdef _DEBUG
	if (dk_level->value)
	{
		if ((dk_level->value >= 1) && (dk_level->value <= 5))
			return dk_level->value;
	}
#endif

	int dkExp = self->record.dkexp;

	short numLevs = sizeof(daikatana_exp_levels)/sizeof(daikatana_exp_levels[0]);

	for (int i = numLevs-1; i >= 0; i--)
	{
		if (dkExp >= daikatana_exp_levels[i])
			return i+1;
	}
	return 1;
}

//---------------------------------------------------------------------------
// daikatana_awardExperience(userEntity_t *self, userEntity_t *other)
//---------------------------------------------------------------------------
long daikatana_calcExpBonus( userEntity_t *self, userEntity_t *other)
{
	if (!self || !other || !self->userHook || !other->userHook || 
		!(other->flags & (FL_CLIENT|FL_MONSTER|FL_BOT)) || 
		!(self->flags & (FL_CLIENT|FL_MONSTER|FL_BOT)) ||
		(other->deadflag != DEAD_NO) ||
		((other->flags & FL_CLIENT) && (coop->value)))				// no exps for killing clients in coop!!
		return 0;

	long exp_bonus = 0;

	if ((other >= &gstate->g_edicts[1]) && (other <= &gstate->g_edicts[(int)(maxclients->value)]) && !coop->value)	
	{
		int level = calcStatLevel( other );
		exp_bonus = 100 * (level + 1);
	}
	else
	{
		// standard calculation for killing a monster ('other')
		playerHook_t*	hook = (playerHook_t*)other->userHook;
		
		exp_bonus = hook->base_health * 0.10;  // 10% of opponent's base health
	}

	return 0.5*exp_bonus;
}

void levelRingThink(userEntity_t *self)
{

	self->s.alpha += 0.03;
	if (self->s.alpha > 0.8)
		self->s.alpha = 0.8;


	if (self->s.render_scale.Length() <= 5)
	{
		self->hacks += 4;
		self->s.render_scale.y *= 0.95;
		self->s.render_scale.z *= 0.95;
	}
	else
	{
		self->s.render_scale.y *= 0.85;
		self->s.render_scale.z *= 0.85;
	}
	
	if (self->hacks > 64)
	{
		self->s.render_scale.y *= 0.65;
		self->s.render_scale.z *= 0.65;
	}

	if (self->hacks > 80)
	{
		gstate->RemoveEntity(self);
		return;
	}

	self->s.old_origin = self->s.origin;
	self->s.origin = self->owner->s.origin;
	self->s.origin.z += self->owner->s.mins.z;
	self->s.origin.z += self->hacks;

	self->nextthink = gstate->time + 0.05;
}

void spawnLevelRing(userEntity_t *self)
{
	userEntity_t *levelRing;
    levelRing = gstate->SpawnEntity ();
	levelRing->flags |= FL_NOSAVE;
    levelRing->owner = self;

    levelRing->movetype = MOVETYPE_FLY;
	levelRing->s.renderfx |= RF_TRANSLUCENT;
	levelRing->s.origin = self->s.origin;
	levelRing->s.old_origin = self->s.origin;
    levelRing->solid = SOLID_NOT;
    levelRing->elasticity = 1.0;
	winfoSetModel(levelRing, MDL_RING);
	levelRing->s.render_scale.Set(3,20,20);
	VectorToAngles(CVector(0,0,1),levelRing->s.angles);
	levelRing->avelocity.Set(0,0,500 + 250*frand());
	levelRing->s.color.Set(0.1,0.1,.8);
	levelRing->s.alpha = 0.5;
	levelRing->hacks = 0;

	levelRing->think = levelRingThink;
	levelRing->nextthink = gstate->time + 0.05;
	gstate->LinkEntity(levelRing);
	winfoPlay(levelRing,SND_WIND1 + (rand() & 1),0.4);
}

void levelUpThink(userEntity_t *self)
{
	if (self->team-- > 0)
	{
		spawnLevelRing(self->owner);
	}
	else if (self->hacks < gstate->time)
	{
		gstate->RemoveEntity(self);
		return;
	}

	self->s.old_origin = self->s.origin;
	self->s.origin = self->owner->s.origin;

	self->nextthink = gstate->time + 0.5;
}

void daikatana_levelUp(userEntity_t *self, int newLevel)
{
	userEntity_t *upEnt;
    upEnt = gstate->SpawnEntity ();
	upEnt->flags |= FL_NOSAVE;
    upEnt->owner = self;
    upEnt->modelName = NULL;
    upEnt->movetype = MOVETYPE_FLY;
	upEnt->s.render_scale.Set(0.001,0.001,0.001);
	upEnt->s.origin = self->s.origin;
	upEnt->s.old_origin = self->s.origin;
    upEnt->solid = SOLID_NOT;
    upEnt->elasticity = 1.0;
	upEnt->team = 2*newLevel;
	upEnt->think = levelUpThink;
	upEnt->nextthink = gstate->time + 0.1;
	upEnt->hacks = gstate->time + 10;			// kill time
	gstate->LinkEntity(upEnt);
}

void daikatana_marks(userEntity_t *self, trace_t &trace, daikatanaHitInfo_t &hit)
{
	if (trace.contents & CONTENTS_CLEAR || !trace.ent ||  trace.ent->solid != SOLID_BSP || trace.ent->takedamage != DAMAGE_NO)
		return;

	if (trace.surface && (trace.surface->flags & SURF_FLOWING))
		return;

	AngleToVectors(self->client->v_angle,forward,right,up);
	right = -right;

	// set up the orientation of this sprite, based on the swipe
	CVector ang = zero_vector;

	ang.yaw = hit.markRoll;

	if (ang.yaw == SCORCH_DOT)
	{
        weaponScorchMark( trace, SM_META );//SM_BULLET );
		return;
	}

	// add on the yaw of the surface you're hitting
	CVector ang2;
	VectorToAngles(trace.plane.normal,ang2);
	ang.yaw += ang2.yaw;
	if (ang.yaw >= 360) ang.yaw -= 360;

	// convert it to a vector so we can transform it
	AngleToVectors(ang,ang2);
	ang = ang2;

	// do the transformation
	ang2.x = DotProduct(ang,forward);//my_forward);
	ang2.y = DotProduct(ang,right);//my_right);
	ang2.z = DotProduct(ang,up);//my_up);

	// back to an angle
	VectorToAngles(ang2,ang);

	gstate->WriteByte( SVC_TEMP_ENTITY );
	gstate->WriteByte( TE_SCORCHMARK );
	gstate->WritePosition( trace.endpos );
	gstate->WriteShort( trace.plane.planeIndex );
	gstate->WriteShort( trace.ent->s.modelindex );
	gstate->WriteShort( trace.ent->s.number);
	gstate->WriteFloat( 0.5 );
	gstate->WriteFloat( ang.yaw );
	gstate->WriteByte( SM_DK_SLASH );
	gstate->MultiCast( zero_vector, MULTICAST_ALL );
}
