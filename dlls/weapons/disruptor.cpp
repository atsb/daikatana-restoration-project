//---------------------------------------------------------------------------
// DISRUPTOR GLOVE
//---------------------------------------------------------------------------
#include "weapons.h"
#include "disruptor.h"
#include "gashands.h"

#define USE_TRIVERTS    0
#define USE_MINSMAX     1

#if USE_TRIVERTS
void my_hr_TriNormal (CVector &v1, CVector &v2, CVector &v3, CVector &out);
void my_CrossProduct(CVector &v1, CVector &v2, CVector &cross);
#endif

enum
{
    // model defs
    MDL_START,
    MDL_ZAP,

    // sound defs
    SND_START,
    SND_ZAP,
    SND_HIT,
    SND_HITWORLD,

    // special frame defs
    ANIM_START,
    ANIM_BITCHSLAP
};

// weapon info def
weaponInfo_t disruptorWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_DISRUPTOR),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_NO_AMMO),
    // names, net names, icon
    "weapon_disruptor",
	tongue_weapons[T_WEAPON_DISRUPTOR],
	"ammo_none",
	tongue_weapons[T_WEAPON_NO_AMMO],
	"iconname",
		
		// model files
    {
		"models/e1/w_tglove.dkm",
		"models/e1/a_tazer.dkm",
		"models/e1/wa_glock.dkm",
		"models/e1/we_ionexpl.sp2",
		NULL
	},
	
    // sound files
    {
		SND_NOSOUND, //"e1/we_dgloveambc.wav",
		"e1/we_dgloveready.wav",
		"e1/we_dgloveaway.wav",
		"e1/we_dgloveshoota.wav",
		"e1/we_dgloveamba.wav",
		"e1/we_dgloveambb.wav",
		SND_NOSOUND,
		SND_NOSOUND,
		"e1/we_dglovehita.wav",
		"e1/we_dglovehitc.wav",
		NULL
	},
	
    // frame names
    {
		MDL_WEAPON_STD,"ready",0,0,
		-1,"away",0,0,
		-1,"shoota",0,0,
		-1,"amba",0,0,
		-1,"ambb",0,0,
		ANIM_NONE,//doesn't exist-1,"ambc",0,0,
		-1,"shootb",0,0,
		0,
		NULL
	},
	
    // commands
    { 
		wpcmds[WPCMDS_DISRUPTOR][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_1,
		wpcmds[WPCMDS_DISRUPTOR][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_1,
		NULL
    },
	
    weapon_give_1,
    weapon_select_1,
    disruptorCommand,
    disruptorUse,
    winfoSetSpawn,
    winfoAmmoSetSpawn,
    sizeof(disruptor_t),
	
    100,
    100,        //	ammo_max
    1,          //	ammo_per_use
    1,          //	ammo_display_divisor
    10,     //	initial ammo
    0,          //	speed
    100.0,      //	range
    20.0,       //	damage
    0.0,        //	lifetime
	
    WFL_NODROP,
	
    WEAPONTYPE_GLOVE,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	
/*	{ 
		tongue_deathmsg_weapon1[T_DEATHMSGWEAPON_DISRUPTOR],
		NULL,
		VICTIM_ATTACKER
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon1,T_DEATHMSGWEAPON_DISRUPTOR),
	VICTIM_ATTACKER,

	WEAPON_DISRUPTOR,
	0,
	NULL
};

// local weapon info access
static weaponInfo_t *winfo=&disruptorWeaponInfo;

//---------------------------------------------------------------------------
// give()
//---------------------------------------------------------------------------
userInventory_t *disruptorGive (userEntity_t *self, int ammoCount)
{
	if (self->inventory && gstate->InventoryFindItem(self->inventory,winfo->weaponName))
		return NULL;

    return(weaponGive(self,winfo,ammoCount));
}

//---------------------------------------------------------------------------
// disruptor_select()
//---------------------------------------------------------------------------
short disruptorSelect(userEntity_t *self)
{
	userInventory_t *curWeapon = self->curWeapon;

	if (self->inventory && (curWeapon && stricmp(curWeapon->name,"weapon_gashands")))
	{
		if (gstate->InventoryFindItem(self->inventory,"weapon_gashands"))
			return(gashandsSelect(self));
	}

    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// command()
//---------------------------------------------------------------------------
void *disruptorCommand(struct userInventory_s *inv, char *commandStr, void *data)
{
    winfoGenericCommand(winfo,inv,commandStr,data);

    return(NULL);
}

//---------------------------------------------------------------------------
// use()
//---------------------------------------------------------------------------
void disruptorUse(userInventory_s *ptr, userEntity_t *self)
{
    playerHook_t *phook=(playerHook_t *)self->userHook;
    disruptor_t *disruptor=(disruptor_t *)self->curWeapon;
	int	frame;

    if(rand() & 1)
		frame = ANIM_SHOOT_STD;
    else
		frame = ANIM_BITCHSLAP;

    winfoAnimate(winfo,self,frame,FRAME_ONCE|FRAME_WEAPON,0.05);

    winfoPlay(self, SND_SHOOT_STD, 1.0f);
    phook->fxFrameNum =	-2;
    phook->fxFrameFunc=disruptorSmashFunc;
    disruptor->weapon.lastFired=gstate->time;
}

//---------------------------------------------------------------------------
// smash_func()
//---------------------------------------------------------------------------
void disruptorSmashFunc(userEntity_t *self)
{
    CVector start,end,dir, dist, vecColor;
    trace_t tr;
//	short	snd;

    playerHook_t *phook=(playerHook_t *)self->userHook;
	if (!phook)
		return;

	WEAPON_FIRED(self);

	// don't call this function more than once
	phook->fxFrameFunc = NULL;
	
    // hit anything?
    dir=self->client->v_angle;
    dir.AngleToVectors(forward,right,up);
	int zoffset = 21;

	if( self->client->ps.pmove.pm_flags & PMF_DUCKED )
	{
		zoffset -= 25;
	}

	weapon_PowerSound(self);

	start=self->s.origin+CVector(0,0,zoffset);

    end=start + forward*winfo->range;
	long mask = MASK_SHOT;
//	mask &= ~CONTENTS_CLEAR;
    tr=gstate->TraceLine_q2(start,end,self,mask);

	if ((self->flags & FL_BOT) && (self->enemy))
	{
		tr.fraction = 0.8;
		tr.endpos = self->enemy->s.origin;
		tr.ent = self->enemy;
	}
    end=tr.endpos;
    dist = tr.endpos - self->s.origin;

    if (tr.ent && (tr.fraction < 1.0))
	{
		weaponScorchMark( tr, SM_FIST );
        if((dist.Length() < 40 && !tr.ent->takedamage) || tr.ent->takedamage)
        {
            vecColor.Set(0.0, 0.0, 1.0f);
            gstate->WriteByte(SVC_TEMP_ENTITY);
            gstate->WriteByte(TE_LIGHT);
            gstate->WritePosition(self->s.origin);
            gstate->WritePosition(vecColor);
            gstate->WriteFloat(350);
            gstate->MultiCast(self->s.origin, MULTICAST_PVS);

            gstate->WriteByte(SVC_TEMP_ENTITY);
            gstate->WriteByte(TE_SPARKS);
            gstate->WriteByte(5);
            gstate->WritePosition(end);
            gstate->WritePosition(dir);
            vecColor.Set(0.5f, 0.5f, 1.0f);
            gstate->WritePosition(vecColor);
            gstate->WriteByte(1);
            gstate->MultiCast(self->s.origin, MULTICAST_PVS);
        }
        if (tr.ent->takedamage) // && tr.ent->deadflag == DEAD_NO )
        {
			WEAPON_HIT(self,1);
            winfoPlay(self, SND_HIT, 1.0f);
			// cek[1-24-00]: set the attackweapon
			winfoAttackWeap;

            if(deathmatch->value)
                com->Damage(tr.ent,self,self,tr.endpos,dir, winfo->damage, DAMAGE_INERTIAL);
            else
                com->Damage(tr.ent,self,self,tr.endpos,dir, winfo->damage * 0.5, DAMAGE_INERTIAL);
        } else {
                winfoPlay(self, SND_HITWORLD, 1.0f);
        }
	}
}

//-----------------------------------------------------------------------------
//	weapon_disruptor
//-----------------------------------------------------------------------------
void weapon_disruptor(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_disruptor_register_func()
{
	gstate->RegisterFunc("disruptorUse",disruptorUse);
	gstate->RegisterFunc("disruptorGive",disruptorGive);
	gstate->RegisterFunc("disruptorSelect",disruptorSelect);
}
