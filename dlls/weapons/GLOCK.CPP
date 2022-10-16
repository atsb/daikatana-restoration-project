//---------------------------------------------------------------------------
// GLOCK 2020
//---------------------------------------------------------------------------
#include "weapons.h"
#include "glock.h"

typedef enum winfo_enum_t
{
    // model defs
    MDL_START,
    MDL_SHELL,
    MDL_MUZZLEFLASH,

    // sound defs
    SND_START,
    SND_SHELL,
    SND_SHELL2,
    SND_SHELL3,
    SND_RELOAD,
    SND_CLICK,

    // frame defs
    ANIM_START,
    ANIM_RELOAD,
    ANIM_SHELL,
    ANIM_MUZZLEFLASH
} winfo_enum_t;

// weapon info def
weaponInfo_t glockWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_GLOCK),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_GLOCK_AMMO),
	// names, net names, icon
	"weapon_glock",
	tongue_weapons[T_WEAPON_GLOCK],
	"ammo_bullets",
	tongue_weapons[T_WEAPON_GLOCK_AMMO],
	"iconname",

	// model files
	{
		"models/e4/w_glock.dkm",
		"models/e4/a_glock.dkm",
		"models/e4/wa_glock.dkm",
		"models/e4/we_glockshell.dkm",
		"models/e4/we_mfglock.sp2",
		NULL
	},

	// sound files
	{
		SND_NOSOUND,
		"e4/we_glockready.wav",
		"e4/we_glockaway.wav",
		"e4/we_glockshootb.wav",
		SND_NOSOUND,
		SND_NOSOUND,
		SND_NOSOUND,
		"e1/we_shotcyclershella.wav",
		"e1/we_shotcyclershellb.wav",
		"e1/we_shotcyclershellc.wav",
		//		"e4/we_glockshella.wav",
		//        "e4/we_glockshellb.wav",
		//        "e4/we_glockshellc.wav",
		"e4/we_glockreload.wav",
		"e4/we_glockclick.wav",
		NULL
	},

	// frame names
	{
		MDL_WEAPON_STD,"ready",0,0,
		-1,"away",0,0,
		-1,"shoota",0,0,
		-1,"amba",0,0,
		-1,"ambb",0,0,
		ANIM_NONE,
		-1,"reload",0,0,             // should be reload
		MDL_SHELL,"stand000",0,0,
		MDL_MUZZLEFLASH,"sp2",0,0,
		0,
		NULL
	},

	// commands
	{
		wpcmds[WPCMDS_GLOCK][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_1,
		wpcmds[WPCMDS_GLOCK][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_1,
		NULL
	},

	weapon_give_1,
	weapon_select_1,
	glock_command,
	glock_use,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(glockWeapon_t),

	100,
	200,        //	ammo_max
	1,          //	ammo_per_use
	1,          //	ammo_display_divisor
	10,         //	initial ammo
	0,          //	speed
	2000.0,     //	range
	20.0,       //	damage
	0.0,        //	lifetime

	WFL_FORCE_SWITCH|WFL_CROSSHAIR,          //	flags
	WEAPONTYPE_PISTOL,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon4[T_DEATHMSGWEAPON_GLOCK],
		NULL,
		ATTACKER_VICTIM
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon4,T_DEATHMSGWEAPON_GLOCK),
	ATTACKER_VICTIM,

	WEAPON_GLOCK,
	0,
	NULL
};

// local weapon info access
static weaponInfo_t *winfo=&glockWeaponInfo;

/////////////////////////////////////////////////////////////////////
// globals
/////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
// glock_give()
//---------------------------------------------------------------------------
userInventory_t *glock_give (userEntity_t *self, int ammoCount)
{
    glockWeapon_t *glock;

    glock=(glockWeapon_t *)weaponGive(self,winfo,ammoCount);
    if ( glock )
        glock->clip=10;

    return((userInventory_t *)glock);
}

//---------------------------------------------------------------------------
// glock_select()
//---------------------------------------------------------------------------
short   glock_select (userEntity_t *self)
{
    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// glock_command()
//---------------------------------------------------------------------------
void *glock_command (struct userInventory_s *ptr, char *commandStr, void *data)
{
    winfoGenericCommand(winfo,ptr,commandStr,data);
    return(NULL);
}

int glock_reload(userEntity_t *self)
{
	if (self->flags & FL_CLIENT)
	{
		playerHook_t *phook=(playerHook_t *)self->userHook;
		glockWeapon_t *glock=(glockWeapon_t *)self->curWeapon;

		if ( !glock->clip )
		{
			glock->clip=weaponHasAmmo(self,true);

			if ( glock->clip )
			{
				if ( glock->clip > 10 )
					glock->clip = 10;
				winfoPlay(self,SND_RELOAD,1.0f);
				winfoAnimate(winfo,self,ANIM_RELOAD,FRAME_ONCE|FRAME_WEAPON,.05);
			}
		}

		return glock->clip;
	}

	return 0;
}

//---------------------------------------------------------------------------
// glockUseAmmo()
//---------------------------------------------------------------------------
short glockUseAmmo(userEntity_t *self)
{
	if (self->flags & FL_CLIENT)
	{
		playerHook_t *phook=(playerHook_t *)self->userHook;
		glockWeapon_t *glock=(glockWeapon_t *)self->curWeapon;

		if ( !glock->clip )
		{
			if (!glock_reload(self))
			{
				winfoPlay(self,SND_CLICK,1.0f);
				phook->attack_finished=gstate->time + .3;
			}
/*			glock->clip=weaponHasAmmo(self,true);

			if ( glock->clip )
			{
				if ( glock->clip > 10 )
					glock->clip = 10;
				winfoPlay(self,SND_RELOAD,1.0f);
				winfoAnimate(winfo,self,ANIM_RELOAD,FRAME_ONCE|FRAME_WEAPON,.05);
			}
			else
			{
				winfoPlay(self,SND_CLICK,1.0f);
				phook->attack_finished=gstate->time + .3;
			}
*/
			return(0);
		}

		if ( weaponUseAmmo(self,true) )
			glock->clip--;

		return(1);
	}
	else
	{
		return weaponUseAmmo(self,false);
	}
}

//---------------------------------------------------------------------------
// glock_use()
//---------------------------------------------------------------------------
void    glock_use (userInventory_s *ptr, userEntity_t *self)
{
    weapon_t *invWeapon=(weapon_t *)self->curWeapon;
    userEntity_t *weapon=(userEntity_t *)self->client->ps.weapon;
    CVector         vecDir, vecMuzzle;
    playerHook_t    *phook=(playerHook_t *)self->userHook;
    trace_t         tr;

	// for sidekicks
	if (!weapon)
		weapon = self;

    if ( !glockUseAmmo(self) )
        return;

	winfoPlay(self,SND_SHOOT_STD,1.0f);
    winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,.05);

	weaponMuzzleFlash(	self,self,winfo,FLASH_GLOCK);

    vecMuzzle.Set(winfo->fWeaponOffsetX1, winfo->fWeaponOffsetY1, winfo->fWeaponOffsetZ1);
    shotShell(self, winfo, MDL_SHELL, SND_SHELL, 3, 1.0, CVector(12,5,45) );
    tr = weaponTraceShot(self, vecMuzzle, 0);
    vecMuzzle += self->s.origin;
    vecDir = tr.endpos - vecMuzzle;
    vecDir.Normalize();
    if(tr.ent && tr.ent->takedamage)
    {
		// cek[1-24-00]: set the attackweapon
		winfoAttackWeap;

		WEAPON_HIT(self,1);
        com->Damage(tr.ent, weapon, self, tr.endpos, vecDir, winfo->damage, DAMAGE_INERTIAL);
        vecDir = tr.endpos - vecMuzzle;
        weaponBloodSplat(tr.ent, vecMuzzle, tr.endpos, tr.endpos, vecDir.Length());
    } 
    else if (tr.ent && tr.ent->solid == SOLID_BSP)
    {
		// SCG[10/2/99]: gunshot now paired with bullet hole on client
//        weaponGunShot(tr.endpos, vecDir);
		weaponScorchMark( tr, SM_BULLET );
//        weaponRicochet(tr.endpos);
    }

	glock_reload(self);
    if(phook)
    {
        phook->fired++;
    }
}

//-----------------------------------------------------------------------------
//	weapon_glock
//-----------------------------------------------------------------------------
void weapon_glock(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

//---------------------------------------------------------------------------
//	ammo_bullets
//
//	pickup item spawn code
//---------------------------------------------------------------------------

void    ammo_bullets (userEntity_t *self)
{
    //	set className so we can use gstate->InventoryFindItem...
    self->className = winfo->ammoName;

    winfoAmmoSetSpawn (winfo, self, 10, 30.0, NULL);
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_glock_register_func()
{
	gstate->RegisterFunc("glock_use",glock_use);
	gstate->RegisterFunc("glock_give",glock_give);
	gstate->RegisterFunc("glock_select",glock_select);
}
