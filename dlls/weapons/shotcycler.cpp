//---------------------------------------------------------------------------
// SHOTCYCLER
//---------------------------------------------------------------------------
#include "weapons.h"
#include "shotcycler.h"

enum
{
    // model defs
    MDL_START,
	MDL_AMMO,
	MDL_AMMO2,
    MDL_MUZZLEFLASH,
    MDL_SHELL,

    // sound defs
    SND_START,
    
    // special frame defs
    ANIM_START
};

// weapon info def
weaponInfo_t shotcyclerWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_SHOTCYCLER),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_SHOTCYCLER_AMMO),
	// names, net names, icon
	"weapon_shotcycler",
	tongue_weapons[T_WEAPON_SHOTCYCLER],
	"ammo_shells",
	tongue_weapons[T_WEAPON_SHOTCYCLER_AMMO],
	"iconname",

	// model files
	{
		"models/e1/w_shotcycler.dkm",
		"models/e1/a_shot.dkm",
		"models/e1/wa_shot6.dkm",
		"models/e1/we_shotcy.sp2",
		"models/e1/we_shotshell.dkm",
		NULL
	},

	// sound files
	{
		SND_NOSOUND,
		"e1/we_shotcyclerready.wav",
		"e1/we_shotcycleraway.wav",
		"e1/we_shotcyclershoota.wav",
		"e1/we_shotcycleramba.wav",
		"e1/we_shotcyclershootb.wav", // just here so its pre-cached
		"e1/we_shotcyclershella.wav",
		"e1/we_shotcyclershellb.wav",
		"e1/we_shotcyclershellc.wav",
		"e1/we_shotcyclershelld.wav",
		"e1/we_shotcyclershelle.wav",
		"e1/we_shotcyclershellf.wav",
		NULL
	},

	// frame names
	{
		MDL_WEAPON_STD,"ready",0,0,
		-1,"away",0,0,
		-1,"shoot",0,0,
		-1,"ambc",0,0,
		ANIM_NONE,
		ANIM_NONE,
		ANIM_NONE,
		// -1,"ambb",0,0,
		0,
		NULL
	},

	// commands
	{ 
		wpcmds[WPCMDS_SHOTCYCLER][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_4,
		wpcmds[WPCMDS_SHOTCYCLER][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_4,
		NULL
	},

	shotcyclerGive,
	shotcyclerSelect,
	shotcyclerCommand,
	shotcyclerUse,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(shotcycler_t),

	400,
	100,        //	ammo_max
	1,          //	ammo_per_use
	1,          //	ammo_display_divisor
	60,         //	initial ammo
	0,          //	speed
	2000.0,     //	range
	10.0,       //	damage
	0.0,        //	lifetime

	WFL_FORCE_SWITCH|WFL_CROSSHAIR,      //	flags

	WEAPONTYPE_RIFLE,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon1[T_DEATHMSGWEAPON_SHOTCYCLER],
		NULL,
		ATTACKER_VICTIM
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon1,T_DEATHMSGWEAPON_SHOTCYCLER),
	ATTACKER_VICTIM,


	WEAPON_SHOTCYCLER,
	0,
	NULL
};

// local weapon info access
static weaponInfo_t *winfo=&shotcyclerWeaponInfo;

// ISP [09-22-99] keep track of damaged units
class CDamagedUnitInfo
{
private:    
    int             nNumDamagedUnits;
    userEntity_t    *aDamagedUnits[2];      // max of 2 entities maybe shot in a single blast
    float           aTotalDamagePerUnit[2]; 

public:
    CDamagedUnitInfo();
    ~CDamagedUnitInfo();

    void Init();

    int GetNumDamagedUnits()        { return nNumDamagedUnits; }
    void AddDamagedUnit( userEntity_t *pUnit, float fDamage );

    userEntity_t *GetDamagedUnit( int nIndex );
    float GetTotalDamage( int nIndex );

};


CDamagedUnitInfo::CDamagedUnitInfo()
{
    Init();
}

CDamagedUnitInfo::~CDamagedUnitInfo()
{
}

void CDamagedUnitInfo::Init()
{
    nNumDamagedUnits = 0;
    aDamagedUnits[0] = NULL;
    aDamagedUnits[1] = NULL;
    aTotalDamagePerUnit[0] = 0.0f;
    aTotalDamagePerUnit[1] = 0.0f;
}

void CDamagedUnitInfo::AddDamagedUnit( userEntity_t *pUnit, float fDamage )
{
    if ( nNumDamagedUnits == 0 )
    {
        aDamagedUnits[nNumDamagedUnits] = pUnit;
        aTotalDamagePerUnit[nNumDamagedUnits] += fDamage;
        nNumDamagedUnits++;
    }
    else
    if ( nNumDamagedUnits == 1 )
    {
        if ( aDamagedUnits[0] == pUnit )
        {
            aTotalDamagePerUnit[0] += fDamage;
        }
        else
        {
            aDamagedUnits[nNumDamagedUnits] = pUnit;
            aTotalDamagePerUnit[nNumDamagedUnits] += fDamage;
            nNumDamagedUnits++;
        }
    }
    else
    if ( nNumDamagedUnits == 2 )
    {
        if ( aDamagedUnits[0] == pUnit )
        {
            aTotalDamagePerUnit[0] += fDamage;
        }
        else
        if ( aDamagedUnits[1] == pUnit )
        {
            aTotalDamagePerUnit[1] += fDamage;
        }
    }

}

userEntity_t *CDamagedUnitInfo::GetDamagedUnit( int nIndex )
{
    _ASSERTE( nIndex >= 0 && nIndex < 2 );

    return aDamagedUnits[nIndex];
}

float CDamagedUnitInfo::GetTotalDamage( int nIndex )
{
    _ASSERTE( nIndex >= 0 && nIndex < 2 );

    return aTotalDamagePerUnit[nIndex];
}
/*
static short cycler_shot_frames[] =
{
	0,5,10,15,20,25,42
};
*/
static short cycler_shot_frames[] =
{
//	3,8,13,18,23,28,42
	2,8,14,20,26,32,42
};
///////////////////////////////////////////////////////////////////////////////
//
//  remove_controller - removes the weapon controller
//
///////////////////////////////////////////////////////////////////////////////
void remove_controller(userEntity_t *self)
{
    untrackLight(self);
    gstate->RemoveEntity(self);
}


//---------------------------------------------------------------------------
// give()
//---------------------------------------------------------------------------
userInventory_t *shotcyclerGive(userEntity_t *self, int ammoCount)
{
    return(weaponGive (self, winfo, ammoCount));
}

//---------------------------------------------------------------------------
// shotcycler_select()
//---------------------------------------------------------------------------
short shotcyclerSelect(userEntity_t *self)
{
    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// command()
//---------------------------------------------------------------------------
void *shotcyclerCommand(struct userInventory_s *inv, char *commandStr, void *data)
{

    winfoGenericCommand(winfo,inv,commandStr,data);

    return(NULL);
}

//---------------------------------------------------------------------------
// use()
//---------------------------------------------------------------------------
void shotcyclerUse(userInventory_s *ptr, userEntity_t *self)
{
    playerHook_t *phook=(playerHook_t *)self->userHook;
	if (!phook)
		return;

    if ( !weaponHasAmmo(self,true) )
        return;

	if (self->flags & FL_CLIENT)
	{
		float fFrametime = 0.045 + (phook->attack_boost * 0.005);

		winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,fFrametime);
		shotcycler_t *cycler=(shotcycler_t *)self->curWeapon;
		cycler->shot = 0;

		phook->fxFrameFunc=shotcyclerShootFunc;
		phook->fxFrameNum=winfoFirstFrame(ANIM_SHOOT_STD)+cycler_shot_frames[0];
	}
	else if (self->flags & FL_BOT)
	{
		shootThink(self,true);
	}
}

void shotcyclerShootFunc(userEntity_t *self)
{
	if (!EntIsAlive(self))
		return;

    playerHook_t *phook=(playerHook_t *)self->userHook;
	if (!phook)
		return;

    shotcycler_t *cycler=(shotcycler_t *)self->curWeapon;
	if (!cycler)
	{
		phook->fxFrameFunc = NULL;
		phook->fxFrameNum = -1;
		return;
	}

	short fire = (weaponHasAmmo(self,false) && (cycler->shot <= 5)) ? 1 : 0;
	shootThink(self, fire);

	if (cycler->shot > 5)
	{
		float fFrametime = 0.045 + (phook->attack_boost * 0.005);
		if (phook->attack_boost)
		{
			float add = (phook->attack_boost == 1) ? 1.5 : 1.0;	// make it so one boost has an effect...
			fFrametime = fFrametime / ((phook->attack_boost+add) * 0.5);
		}
		userEntity_t *weap = (userEntity_t *)self->client->ps.weapon;
		if (weap)
		{
			int num = winfoLastFrame(ANIM_SHOOT_STD) - weap->s.frame;
			phook->attack_finished = gstate->time + num * fFrametime;
		}
		else
			phook->attack_finished = gstate->time + 1.0;

		phook->fxFrameFunc = NULL;
		phook->fxFrameNum = -1;
		return;
	}
	
	phook->fxFrameFunc = shotcyclerShootFunc;
	phook->fxFrameNum = cycler_shot_frames[++cycler->shot] + winfoFirstFrame(ANIM_SHOOT_STD);
}

//---------------------------------------------------------------------------
// shootThink()
//---------------------------------------------------------------------------
void shootThink(userEntity_t *self, short fire)
{
    CVector vecOrg, vecForward, vecRight, vecUp, vecDir;
	CVector kick_angles(-1,0,0);
	float kick_velocity=-70;

    if ( self->deadflag != DEAD_NO )
    {
        return;
    }

	if (fire)
	{
		weapon_kick(self, kick_angles, kick_velocity, 200, 200);
		shotcyclerShoot(self);

		weaponMuzzleFlash(	self,self,winfo,FLASH_SHOTCYCLER);
	}

    if ( !fire )
    {   
		playerHook_t *phook = (playerHook_t *)self->userHook;
		if (phook)
		{
			phook->fxFrameFunc = NULL;
			phook->fxFrameNum = -1;
		}
        gstate->StartEntitySound(self, CHAN_WEAPON, gstate->SoundIndex("e1/we_shotcyclershootb.wav"), 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
        return;
    } 
	else 
	{
        shotShell(self, &shotcyclerWeaponInfo, 4, 6, 6 , 1.0, CVector(12,5,15));
        winfoPlay(self, SND_SHOOT_STD,1.0f);
    }

	weapon_kick(self, kick_angles, kick_velocity, 200, 200);

    if(self->flags & FL_CLIENT)
    {
        // set this weapon being just fired
        _ASSERTE( self->input_entity );
        playerHook_t *pOwnerHook = (playerHook_t*)self->input_entity->userHook;
        _ASSERTE( pOwnerHook );
        pOwnerHook->bJustFired = 1;
                
        vecDir = self->client->v_angle;
        vecDir.AngleToVectors(vecForward, vecRight, vecUp);
        vecOrg = self->s.origin + weaponHandVector(self, vecForward * 35, vecRight);
    }
    else
    {
        // set this weapon being just fired
        _ASSERTE( self );
        playerHook_t *pOwnerHook = (playerHook_t*)self->userHook;
        _ASSERTE( pOwnerHook );
        pOwnerHook->bJustFired = 1;
    }
}


//---------------------------------------------------------------------------
// shoot()
//---------------------------------------------------------------------------
void shotcyclerShoot(userEntity_t *self)
{
    //userEntity_t    *went=(userEntity_t *)self->client->ps.weapon;// SCG[1/24/00]: not used
    playerHook_t    *phook=(playerHook_t *)self->userHook;
    CVector         vecEndPos, vecTracePellet, vecDir, vecSpread, vecShotOffset, vecStart;
    CVector         vecForward, vecRight, vecUp, vecPelletEnd;
    int             iPellets;
    float           fDistance;//, totalDamage=0;
    trace_t         tr, trShot;
    
    CDamagedUnitInfo  damagedUnitInfo;

    if ( !phook )
    {
        return;
    }

    phook->fired++;

    if ( !weaponHasAmmo(self,true) )  // 3.5 dsn
	{
		phook->fxFrameFunc = NULL;
		phook->fxFrameNum = -1;
	    return;
	}

    weaponUseAmmo(self,false);
    // weapon_trace_attack(self, winfo->damage, 10, winfo->range, 0);

	self->client->v_angle.AngleToVectors(forward, right, up);
	vecShotOffset = (winfo->fWeaponOffsetX1 * right) + 
				    (winfo->fWeaponOffsetY1 * forward) + (winfo->fWeaponOffsetZ1 * up);

//    vecShotOffset.Set(winfo->fWeaponOffsetX1, winfo->fWeaponOffsetY1, winfo->fWeaponOffsetZ1);

    trShot      = weaponTraceShot(self, vecShotOffset, 0);
    vecEndPos   = trShot.endpos;
    vecSpread.Set(0.09f, 0.09f ,0.0f);
    vecStart    = self->s.origin + vecShotOffset;
    vecDir      = vecEndPos - vecStart;
    fDistance   = vecDir.Length() + 64.0f;

	float tDamage = (deathmatch->value) ? winfo->damage : 0.75 * winfo->damage;
    for(iPellets = 0; iPellets < 10; iPellets++)
    {
        vecDir = vecEndPos - vecStart;
        vecDir.Normalize();
        vecDir.VectorToAngles(vecDir);
        vecDir.AngleToVectors(vecForward, vecRight, vecUp);
        vecDir = vecForward + vecRight * crnd () * vecSpread.x + vecUp * crnd () * vecSpread.y;
        vecPelletEnd = vecStart + vecDir * fDistance;
        tr = gstate->TraceLine_q2(vecStart, vecPelletEnd, self, MASK_SHOT);
        if(tr.fraction < 1.0)
        {
            vecDir = vecEndPos - vecStart;
            vecDir.Normalize();
            if(tr.ent && tr.ent->takedamage)
            {
                damagedUnitInfo.AddDamagedUnit( tr.ent, tDamage * 0.1f );
            }
        }
    }

	// [cek 11-10-99] these used to be above but they were generating waaayyy too much traffic
	if (tr.ent)
	{
		if(tr.ent->takedamage)
		{
			weaponBloodSplat(tr.ent, vecStart, vecEndPos, tr.endpos, 96.0);
		}
		else if ((tr.ent->solid == SOLID_BSP) || (!tr.ent->takedamage))
		{
			weaponScorchMark(tr,SM_MULTIBULLET);
		}
	}

    int nNumDamagedUnits = damagedUnitInfo.GetNumDamagedUnits();
    for ( int i = 0; i < nNumDamagedUnits; i++ )
    {
        userEntity_t *pDamagedUnit = damagedUnitInfo.GetDamagedUnit( i );
        float fDamage = damagedUnitInfo.GetTotalDamage( i );
		// cek[1-24-00]: set the attackweapon
		winfoAttackWeap;

        com->Damage( pDamagedUnit, self, self, tr.endpos, vecDir, fDamage, DAMAGE_INERTIAL);
    }

	WEAPON_HIT(self,nNumDamagedUnits);
}

//-----------------------------------------------------------------------------
//	weapon_shotcycler
//-----------------------------------------------------------------------------
void weapon_shotcycler(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

//-----------------------------------------------------------------------------
//	ammo_shells
//-----------------------------------------------------------------------------
void ammo_shells (userEntity_t *self)
{
    winfoAmmoSetSpawn (winfo, self, 24, 30.0, NULL);
	ammo_set_sound(self, gstate->SoundIndex("global/i_scyclerammo.wav"));
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_shotcycler_register_func()
{
	gstate->RegisterFunc("shotcyclershootThink",shootThink);
	gstate->RegisterFunc("shotcyclerremove_controller",remove_controller);
	gstate->RegisterFunc("shotcyclerUse",shotcyclerUse);
	gstate->RegisterFunc("shotcyclerGive",shotcyclerGive);
	gstate->RegisterFunc("shotcyclerSelect",shotcyclerSelect);
}
