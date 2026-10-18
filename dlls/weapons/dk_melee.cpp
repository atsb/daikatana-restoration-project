//---------------------------------------------------------------------------
// DAIKATANA - Melee
//---------------------------------------------------------------------------
#include "weapons.h"
#include "dk_melee.h"

enum {
	// model defs
	MDL_START,

	// sound defs
	SND_START,
	SND_SWIPEA,
	SND_SWIPEB,
	SND_SWIPEC,
	SND_SWIPED,
	SND_SWIPEE,
	SND_SWIPEF,
	SND_SWHIT1,
	SND_SWHIT2,
	SND_SWHIT3,
	SND_SWHIT4,
	SND_SWHIT5,



	// special frame defs
	ANIM_START,					// a
	ANIM_SHOOT2,				// b 
	ANIM_SHOOT3,				// c -- low pitch
	ANIM_SHOOT4,				// d -- low pitch
	ANIM_SHOOT5,				// e -- sounds fast
//	ANIM_SHOOT6,
//	ANIM_SHOOT7,
//	ANIM_SHOOT8,
//	ANIM_SHOOT9,
//	ANIM_SHOOT10,
};

// weapon info def
weaponInfo_t dk_meleeWeaponInfo = {
	// names, net names, icon
	"weapon_dk_melee",
	tongue_weapons[T_WEAPON_DAIKATANA_MELEE],
	"ammo_none",
	tongue_weapons[T_WEAPON_NO_AMMO],
	"iconname",

	// model files
	{"models/global/w_daikatana.dkm",
	"models/e4/a_glock.dkm",
	"models/e4/wa_glock.dkm",
	NULL},

	// sound files
	{SND_NOSOUND,
    SND_NOSOUND, //"weapons/disruptorrod/disruptorready.wav",
	 SND_NOSOUND, //"weapons/disruptorrod/disruptoraway.wav",
	 SND_NOSOUND, //"weapons/shotcycler/shotcyclerfire.wav",
	 SND_NOSOUND, //"weapons/disruptorrod/disruptorambient1.wav",
    SND_NOSOUND,
    SND_NOSOUND,
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
	 NULL},

	// frame names
	{MDL_WEAPON_STD,"ready",0,0,
		-1,"away",0,0,
		-1,"ataka",0,0,
		-1,"amba",0,0,
      ANIM_NONE,
      ANIM_NONE,
		-1,"atakb",0,0,
		-1,"atakc",0,0,
		-1,"atakd",0,0,
		-1,"atake",0,0,
//		-1,"atakf",0,0,
//		-1,"atakg",0,0,
//		-1,"atakh",0,0,
//		-1,"ataki",0,0,
	0,NULL},

	// commands
	{ 
      wpcmds[WPCMDS_DAIKATANA_MELEE][WPCMDS_GIVE], (void (*)(struct edict_s *))d1_give,
	  wpcmds[WPCMDS_DAIKATANA_MELEE][WPCMDS_SELC], (void (*)(struct edict_s *))d1_select,
	  NULL
    },

	d1_give,
	d1_select,
	d1_command,
	d1_use,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(dk_melee_t),

	200,
	1000,		//	ammo_max
	1,			//	ammo_per_use
	10,			//	ammo_display_divisor
	0,			//	initial ammo
	0,			//	speed
	0,			//	range
	0,			//	damage
	0.0,		//	lifetime

	WFL_SPECIAL | WFL_DK_POWER,		//	flags
	WEAPONTYPE_PISTOL,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

	{NULL,
	NULL,
	NULL},
    WEAPON_DKMELEE
};

// local weapon info access
static weaponInfo_t *winfo=&dk_meleeWeaponInfo;

// anim, {damage frame, damage, sound}
static dk_meleeInfo_t meleeInfo[] = {
	{ANIM_SHOOT_STD,	{{8, 40, SND_SWIPEC, -1}, {-1, -1, -1, -1}}},
	{ANIM_SHOOT2,		{{8, 40, -1, -1}, {16, 40, -1, -1}}},
	{ANIM_SHOOT3,		{{13,40, SND_SWIPED, ANIM_SHOOT4}, {-1, -1, -1, -1}}},
	{ANIM_SHOOT4,		{{8, 40, -1, -1}, {16, 40, -1, -1}}},
	{ANIM_SHOOT5,		{{9, 40, -1, -1}, {18, 40, -1, -1}}},
};

void dk_attack ( userEntity_t *self, dk_meleeHitInfo_t &hit, float distance );
void dk_playSound(userEntity_t *self, int type, int snd);

typedef struct
{
	int first;
	int last;
} sound_info_t;

static sound_info_t dk_sound_info[] = 
{
	{SND_SWIPEA,SND_SWIPEF},			// SND_CH_SWIPE
	{SND_SWHIT1,SND_SWHIT5}				// SND_CH_HIT
};

//---------------------------------------------------------------------------
// d1_give()
//---------------------------------------------------------------------------
userInventory_t *d1_give(userEntity_t *self, int ammoCount)
{
   return(weaponGive (self, winfo, ammoCount));
}

//---------------------------------------------------------------------------
// d1_select()
//---------------------------------------------------------------------------
short d1_select(userEntity_t *self)
{
   return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// d1_command()
//---------------------------------------------------------------------------
void *d1_command(struct userInventory_s *inv, char *commandStr, void *data)
{
   winfoGenericCommand(winfo,inv,commandStr,data);

	return(NULL);
}

//---------------------------------------------------------------------------
// d1_use()
//---------------------------------------------------------------------------
void d1_use(userInventory_s *ptr, userEntity_t *self)
{
	dk_melee_t *dk_melee=(dk_melee_t *)self->curWeapon;
	playerHook_t *phook=(playerHook_t *)self->userHook;

	short index=(short)(rnd() * (sizeof(meleeInfo)/sizeof(meleeInfo[0])) -.1);

	winfoAnimate(winfo,self,meleeInfo[index].anim,FRAME_ONCE|FRAME_WEAPON,.05);
	dk_melee->meleeNum=index;
	dk_melee->hitNum=0;

	gstate->Con_Printf("dk attack %c\n",index + 'a');
	phook->fxFrameNum=winfoFirstFrame(meleeInfo[index].anim)+meleeInfo[index].hitInfo[0].damageFrame;
	phook->fxFrameFunc=d1_think;
}

//---------------------------------------------------------------------------
// think()
//---------------------------------------------------------------------------
void d1_think(userEntity_t *self)
{
	dk_melee_t *dk_melee=(dk_melee_t *)self->curWeapon;
	dk_meleeInfo_t *melee=&meleeInfo[dk_melee->meleeNum];
	dk_meleeHitInfo_t *hit=&melee->hitInfo[dk_melee->hitNum];
	playerHook_t *phook=(playerHook_t *)self->userHook;

	dk_attack(self,*hit,125);
	// evaluate next hit
	dk_melee->hitNum++;
	hit++;

	// is there another hit for this attack?
	if (dk_melee->hitNum==DK_MELEE_MAX_HITS || hit->damage==-1)
		phook->fxFrameFunc=NULL;
	else
		phook->fxFrameNum=winfoFirstFrame(melee->anim)+hit->damageFrame;

/*	if (hit->damage == -1)
	{
		if (hit->nextAnim != -1)
		{
			dk_melee->meleeNum = hit->nextAnim;
			dk_melee->hitNum = 0;
			phook->fxFrameNum=winfoFirstFrame(melee->anim)+hit->damageFrame;
		}
		else
			phook->fxFrameFunc = NULL;
	}
	else
		phook->fxFrameNum=winfoFirstFrame(melee->anim)+hit->damageFrame;*/
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_dk_melee_register_func()
{
	gstate->RegisterFunc("dk_meleeThink",d1_think);
	gstate->RegisterFunc("dk_meleeuse",d1_use);
}

void dk_attack ( userEntity_t *self, dk_meleeHitInfo_t &hit, float distance )
{
	if (hit.damage == -1)
		return;

    playerHook_t *phook=(playerHook_t *)self->userHook;
    trace_t         tr;

	// power boost!!!!
    weapon_PowerSound(self);

	// handle the sword swipes
	dk_playSound(self,SND_CH_SWIPE,hit.sound);

    CVector dir = self->client->v_angle;
    dir.AngleToVectors (forward, right, up);
    CVector start = self->s.origin + forward * 8.0 + up * 16;    
	CVector end = start + forward * distance;

    tr = gstate->TraceLine_q2 (start, end, self, MASK_SHOT);
    if ( tr.fraction!= 1.0 )
    {
        //Make sure we are damaging something that is meant to be damaged
        if ( tr.ent && tr.ent->takedamage != DAMAGE_NO )
        {
            dir = tr.endpos - start;
            dir.Normalize ();
            com->Damage (tr.ent, self, self, tr.endpos, dir, hit.damage, DAMAGE_INERTIAL);
			dir = tr.endpos;
			dir.z -= 100;
            weaponBloodSplat (tr.ent, tr.endpos, dir, tr.endpos, 96.0);
        }
        else if ( tr.ent && tr.ent->solid == SOLID_BSP || tr.ent->takedamage == DAMAGE_NO )
        {
			dk_playSound(self,SND_CH_HIT,-1);
	        weaponScorchMark( tr, SM_BULLET );
			clientSparks(tr.endpos,tr.plane.normal,40,CVector(0.7,0.7,0.8));
        }
    }
}

//					type			 != -1 to force a sound
void dk_playSound(userEntity_t *self, int type, int snd)
{
	dk_melee_t *dk_melee=(dk_melee_t *)self->curWeapon;
	if (!dk_melee)
		return;

	if ((type < SND_CH_MIN) || (type >= SND_CH_MAX))
	{
		memset(&dk_melee->lastSnd,0,sizeof(dk_melee->lastSnd));
		return;
	}

	if (snd != -1)
	{
		winfoPlay(self,snd,1.0);
		dk_melee->lastSnd[type] = snd;
	}
	else
	{
		int index;
		int lf = dk_sound_info[type].last - dk_sound_info[type].first;
		if (lf > 0)
		{
			index = dk_sound_info[type].first + (int)((lf + 0.995) * frand());
			if (index == dk_melee->lastSnd[type])
				index = dk_sound_info[type].first + (index + 1) % lf;
		}
		else
			index = dk_sound_info[type].first;

		winfoPlay(self,index,1.0);
		dk_melee->lastSnd[type] = index;
	}
}

