////////////////////////////////////////////////////////////////////////
// Ultimate Gas Hands Implementation
////////////////////////////////////////////////////////////////////////

#include "weapons.h"
#include "gashands.h"
#include "disruptor.h"

////////////////////////////////////////////////////////////////////////
// weapon info
enum {
    // Model 
    MDL_START,

    // sound
    SND_START,
    SND_HIT,
	SND_HIT1,
	SND_GH_LOOP,
    
    // animation
    ANIM_START,
	ANIM_SHOOT_2
};

weaponInfo_t gashandsWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_GASHANDS),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_NO_AMMO),
    // name
    "weapon_gashands",
    tongue_weapons[T_WEAPON_GASHANDS],
    "ammo_none",
    tongue_weapons[T_WEAPON_NO_AMMO],
    "iconname",

    // models
    {
        "models/e1/w_gashand.dkm",
		"models/e1/a_gashand.dkm",
        "models/e1/wa_gashand.dkm",
        "models/e1/we_gashand.dkm",
        NULL
    },
    
	// sounds
	{
		SND_NOSOUND,
		"e1/we_gasstart.wav",
		"e1/we_gasstopa.wav",
		"e1/we_gasloop.wav",
		SND_NOSOUND,
		SND_NOSOUND,
		SND_NOSOUND,
		"e1/we_gasclangc.wav",
		"e1/we_gasclangc.wav",
		"e1/we_gasloopa.wav",
		NULL
	},

	// frame names
	{
		MDL_WEAPON_STD,"ready",0,0,
		-1,"away",0,0,
		-1,"shootb",0,0,
		-1,"amba",0,0,
		ANIM_NONE,//doesn't exist-1,"ambb",0,0,
		ANIM_NONE,//doesn't exist-1,"ambc",0,0,
		-1,"shootc",0,0,
		NULL
	},

	// commands
	{ 
		wpcmds[WPCMDS_GASHANDS][WPCMDS_GIVE], (void (*)(struct edict_s *))gashandsGive,
		wpcmds[WPCMDS_GASHANDS][WPCMDS_SELC], (void (*)(struct edict_s *))gashandsSelect,
		NULL
	},

	gashandsGive,
	gashandsSelect,
	gashandsCommand,
	gashandsUse,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(gashands_t),

	100,
	100,        //	ammo_max
	1,          //	ammo_per_use
	1,          //	ammo_display_divisor
	0,          //	initial ammo
	0,          //	speed
	100.0,      //	range
	20.0,       //	damage
	120.0,      //	lifetime

	WFL_NODROP|WFL_PLAYER_ONLY,

	WEAPONTYPE_GLOVE,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{ 
		tongue_deathmsg_weapon1[T_DEATHMSGWEAPON_GASHANDS],
		NULL,
		ATTACKER_VICTIM
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon1,T_DEATHMSGWEAPON_GASHANDS),
	ATTACKER_VICTIM,

	WEAPON_GASHANDS,
	0,
	gashandsThink
};
static weaponInfo_t *winfo=&gashandsWeaponInfo;

void stopGasHands(userEntity_t *self)
{
	winfoStopLooped(self);
	self->client->pers.fGasHandsTime = 0;

	userInventory_t *gashands = gstate->InventoryFindItem(self->inventory,winfo->weaponName);
	if (gashands)
		gstate->InventoryDeleteItem(self, self->inventory,gashands);
    disruptorSelect(self);
}

void gashandsThink(userEntity_t *self)
{
    if((self->EntityType != WEAPON_GASHANDS) || ( !deathmatch->value && (self->owner->client->pers.fGasHandsTime <= 0)) )
	{
		stopGasHands(self->owner);
        return;
	}

	if (gstate->level->intermissiontime || gstate->bCinematicPlaying)
	{
		if (self->owner->s.sound)
		{
			winfoStopLooped(self->owner);
		}
		return;
	}

    gashands_t      *gashands;
    if(self->owner && (gashands = (gashands_t *)self->owner->curWeapon) )
    {
		if (!self->owner->s.sound && (self->s.frame < winfoFirstFrame(ANIM_READY_STD) || (self->s.frame >= winfoLastFrame(ANIM_READY_STD))) )
		    winfoPlayLooped(self->owner,SND_GH_LOOP,1.0f);

        if(!deathmatch->value && (self->owner->client->pers.fGasHandsTime <= 0))	// never runs out in deathmatch (unless you die)
        {
			stopGasHands(self->owner);
            return;
        }
    }

	if (self->owner && (frand() > 0.7))
	{
		AngleToVectors(self->owner->s.angles,forward,right,up);
		CVector org = self->s.origin;
		org += 4 * forward;
		org += 4 * right;
		org += self->owner->client->ps.pmove.pm_flags & PMF_DUCKED ? zero_vector : (21 * up);
		clientSmoke(org,.4,10,3,4);
	}

	if (!deathmatch->value)
	{
		self->owner->client->pers.fGasHandsTime -= 0.1;
	}
}

////////////////////////////////////////////////////////////////////////
// userInventory_t *give (userEntity_t *self, int ammoCount)
//
//  Description:
//      Adds the weapon to the entity's inventory.
//
//  Parameters:
//      userEntity_t *self      a pointer back to this ent
//      int ammouCount          initial ammo
//
//  Returns:
//      pointer to an inventory item
//  
userInventory_t *gashandsGive (userEntity_t *self, int ammoCount)
{
	if (!(self->flags & FL_CLIENT))
		return NULL;

    userInventory_t *newGasHands = weaponGive(self,winfo,ammoCount);
	if (newGasHands)
	{
		if (ammoCount != -1)
		{
			if (!deathmatch->value)
				self->client->pers.fGasHandsTime += winfo->lifetime;
			else
				self->client->pers.fGasHandsTime = 0;

			weaponSelect(self,winfo);
		}

	}

    return newGasHands;
}


////////////////////////////////////////////////////////////////////////
// short gashands_select(userEntity_t *self)
short gashandsSelect(userEntity_t *self)
{
	short rt = weaponSelect(self,winfo);

    return rt;
}

void *gashandsCommand(struct userInventory_s *inv, char *commandStr, void *data)
{
	if (!stricmp(commandStr,"change"))
	{
		userEntity_t *self = (userEntity_t *)data;
		if (self)
			winfoStopLooped(self);
	}

    return winfoGenericCommand(winfo,inv,commandStr,data);
}

void gashandsUse(userInventory_s *ptr, userEntity_t *self)
{
    playerHook_t        *phook;
    gashands_t          *gashands;

    if(self && self->userHook && self->curWeapon)
    {
		short animindex = (rand() & 1) ? ANIM_SHOOT_STD : ANIM_SHOOT_2;
        phook = (playerHook_t *)self->userHook;
        gashands = (gashands_t *)self->curWeapon;
        winfoAnimate(winfo,self,animindex,FRAME_ONCE|FRAME_WEAPON,0.05);
        winfoStopLooped(self);
        winfoPlay(self, SND_SHOOT_STD, 1.0f);
        phook->fxFrameNum = -2;//winfoFirstFrame(ANIM_SHOOT_STD)+3;
        phook->fxFrameFunc=gashandsSmackMyBitchUp;
        gashands->weapon.lastFired = gstate->time;
    }
}

void gashandsStartSound(userEntity_t *self)
{
    winfoPlayLooped(self,SND_GH_LOOP,1.0f);
}

void gashandsSmackMyBitchUp(userEntity_t *self)
{
    playerHook_t    *phook;
    trace_t tr;
    CVector vecStart, vecEnd, vecDir, vecDistance, vecColor, vecForward, vecRight, vecUp;

    if(self && self->userHook)
    {
		weapon_PowerSound(self);
		WEAPON_FIRED(self);
        phook = (playerHook_t *)self->userHook;
		phook->fxFrameFunc = NULL;
//        phook->fxFrameFunc = gashandsStartSound;
//        phook->fxFrameNum = -1;//winfoFirstFrame(ANIM_SHOOT_STD)+3;
        vecDir = self->client->v_angle;
        vecDir.AngleToVectors(vecForward, vecRight, vecUp);
        vecStart = self->s.origin+CVector(0,0,16);
		if( self->client->ps.pmove.pm_flags & PMF_DUCKED )
		{
			vecStart.z -= 25;
		}
        vecEnd = vecStart + vecForward * winfo->range;
        tr = gstate->TraceLine_q2(vecStart, vecEnd, self, MASK_SHOT);
        vecEnd = tr.endpos;
        if(tr.ent && (tr.fraction < 1.0))
        {
            vecDistance = vecEnd - vecStart;
           // if( (vecDistance.Length() < 40 && !tr.ent->takedamage) || tr.ent->takedamage)
            {
                vecColor.Set(0.0f, 0.0f, 1.0f);
                gstate->WriteByte(SVC_TEMP_ENTITY);
                gstate->WriteByte(TE_LIGHT);
                gstate->WritePosition(self->s.origin);
                gstate->WritePosition(vecColor);
                gstate->WriteFloat(350);
                gstate->MultiCast(self->s.origin, MULTICAST_PVS);

                gstate->WriteByte(SVC_TEMP_ENTITY);
                gstate->WriteByte(TE_SPARKS);
                gstate->WriteByte(5);
                gstate->WritePosition(vecEnd);
                gstate->WritePosition(vecDir);
                vecColor.Set(0.7f, 0.7f, 1.0f);
                gstate->WritePosition(vecColor);
	            gstate->WriteByte(1);
                gstate->MultiCast(self->s.origin, MULTICAST_PVS);
            }
            if(tr.ent->takedamage)
            {
				WEAPON_HIT(self,1);
				// cek[1-24-00]: set the attackweapon
				winfoAttackWeap;

                com->Damage(tr.ent, self, self, tr.endpos, vecDir, winfo->damage, DAMAGE_INERTIAL);
                winfoPlay(self, SND_HIT, 1.0f);
            } 
            else 
            {
                winfoPlay(self, SND_HIT+1, 1.0f);
            }
        }
    }
}

//-----------------------------------------------------------------------------
//	weapon_gashands
//-----------------------------------------------------------------------------
void weapon_gashands(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_gashands_register_func()
{
	gstate->RegisterFunc("gashandsThink",gashandsThink);
	gstate->RegisterFunc("gashandsUse",gashandsUse);
	gstate->RegisterFunc("gashandsGive",gashandsGive);
	gstate->RegisterFunc("gashandsSelect",gashandsSelect);
}
