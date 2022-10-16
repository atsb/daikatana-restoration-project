//---------------------------------------------------------------------------
// FLASHLIGHT
//---------------------------------------------------------------------------
#include "weapons.h"
#include "flashlight.h"

enum
{
    // model defs
    MDL_START,

    // sound defs
    SND_START,

    // special frame defs
    ANIM_START,
};

// weapon info def
weaponInfo_t flashlightWeaponInfo = {
    // names, net names, icon
    "weapon_flashlight",
    tongue_weapons[T_WEAPON_FLASHLIGHT],
    "ammo_flashlight",
    tongue_weapons[T_WEAPON_NO_AMMO],
    "iconname",

    // model files
    {"models/e2/w_sflare.dkm",
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
        NULL},

    // frame names
    {MDL_WEAPON_STD,"ready",0,0,
        -1,"away",0,0,
        -1,"shoota",0,0,
        -1,"amba",0,0,
        ANIM_NONE,
        ANIM_NONE,
        0,NULL},

    // commands
    {  
      wpcmds[WPCMDS_FLASHLIGHT][WPCMDS_GIVE], (void (*)(struct edict_s *))flashlight_give,
      wpcmds[WPCMDS_FLASHLIGHT][WPCMDS_SELC], (void (*)(struct edict_s *))flashlight_select,
      NULL
    },

    flashlight_give,
    flashlight_select,
    flashlight_command,
    flashlight_use,
    winfoSetSpawn,
    winfoAmmoSetSpawn,
    sizeof(flashlight_t),

    200,
    1000,   //	ammo_max
    1,      //	ammo_per_use
    10,     //	ammo_display_divisor
    0,      //	initial ammo
    0,      //	speed
    0,      //	range
    0,      //	damage
    0.0,    //	lifetime

    WFL_AI_CANNOT_USE,      //	flags

    WEAPONTYPE_PISTOL,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

	{NULL,
        NULL,
        NULL},
    WEAPON_FLASHLIGHT
};

// local weapon info access
static weaponInfo_t *winfo=&flashlightWeaponInfo;

//---------------------------------------------------------------------------
// give()
//---------------------------------------------------------------------------
userInventory_t *flashlight_give(userEntity_t *self, int ammoCount)
{
    flashlight_t *flashlight;
    CVector color(1,1,1);                        // default color

    flashlight=(flashlight_t *)weaponGive(self,winfo,ammoCount*FLASHLIGHT_POWER_DIV*.6);
    if ( flashlight )
        flashlight->color = color;

    return((userInventory_t *)flashlight);
}

//---------------------------------------------------------------------------
// flashlight_select()
//---------------------------------------------------------------------------
short flashlight_select(userEntity_t *self)
{
    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// command()
//---------------------------------------------------------------------------
void *flashlight_command(struct userInventory_s *inv, char *commandStr, void *data)
{
    flashlight_t *flashlight=(flashlight_t *)inv;
    userEntity_t *self=(userEntity_t *)data;
    playerHook_t *phook=(playerHook_t *)self->userHook;

    if ( !stricmp(commandStr,"ammo_count") )
    {
        if ( flashlight->weapon.ammo )
        {
            flashlight->weapon.ammo->command((userInventory_t *)flashlight->weapon.ammo,"count",data);
            *(int *)data /= FLASHLIGHT_POWER_DIV;
        }
    }
    else if ( !stricmp(commandStr,"set_color") )
    {
        CVector *color=(CVector *)data;

        flashlight->color = *color;
    }
    else
        winfoGenericCommand(winfo,inv,commandStr,data);

    return(NULL);
}

//---------------------------------------------------------------------------
// use()
//---------------------------------------------------------------------------
void flashlight_use(userInventory_s *ptr, userEntity_t *self)
{
    playerHook_t *phook=(playerHook_t *)self->userHook;
    userEntity_t *ent;
    flashlight_t *flashlight=(flashlight_t *)self->curWeapon;

    phook->attack_finished=gstate->time;

    if ( gstate->time >= flashlight->killtime )
    {
        ent=gstate->SpawnEntity();
        ent->className="flashlight";
        ent->owner=self;
        ent->s.renderfx |= RF_TRACKENT;
        ent->svflags |= SVF_SHOT;
        winfoSetModel(ent,MDL_WEAPON_STD);
        ent->s.render_scale.Set(.001,.001,.001);
        gstate->SetSize(ent,-1,-1,-1,1,1,1);

        ent->think=flashlight_think;
        ent->nextthink=gstate->time+.1;
        ent->remove=flashlight_Remove;

        flashlight->killtime=gstate->time+.1;
        flashlight_think(ent);
    }

    flashlight->killtime=gstate->time+.2;
}

//---------------------------------------------------------------------------
// think()
//---------------------------------------------------------------------------
void flashlight_think(userEntity_t *self)
{
    flashlight_t *flashlight=(flashlight_t *)self->owner->curWeapon;
    CVector start,end,dir;
    float power;

    // determine battery strength (from ammo)
    weaponUseAmmo(self->owner,false);
    power=(float)(weaponHasAmmo(self->owner,false));
    power /= winfo->ammo_display_divisor;
    power=(float)(weaponHasAmmo(self->owner,false)/winfo->ammo_display_divisor) / 50.0;

    // turn light off?
    if ( gstate->time >= flashlight->killtime || !power )
    {
        self->remove (self);
        return;
    }

    // don't let size/brightness drop as fast as percentage
    if ( power < 1 )
        power += (1-power) * .5;
    else
        power=1;

    // keep light controller with player
    self->s.origin = self->owner->s.origin;
    self->s.old_origin = self->owner->s.old_origin;
    self->s.angles = self->owner->s.angles;
    gstate->LinkEntity(self);

    // show light
    trackInfo_t tinfo;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=self;
    tinfo.srcent=self->owner;
    tinfo.fru.Zero();
    tinfo.lightColor.x = power*flashlight->color.x;
    tinfo.lightColor.y = power*flashlight->color.y;
    tinfo.lightColor.z = power*flashlight->color.z;
    tinfo.lightSize=power;
    tinfo.flags = TEF_SRCINDEX|TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FRU|TEF_FXFLAGS;
    tinfo.fxflags = TEF_FLASHLIGHT;
    com->trackEntity(&tinfo,MULTICAST_ALL);
//   trackLight(self,power,power*flashlight->color[0],power*flashlight->color[1],power*flashlight->color[2],TEF_FLASHLIGHT);

    self->nextthink=gstate->time+.1;
}

//-----------------------------------------------------------------------------
//	flashlightRemove
//-----------------------------------------------------------------------------
void flashlight_Remove(userEntity_t *self)
{
    untrackLight(self);
    gstate->RemoveEntity(self);
}

//-----------------------------------------------------------------------------
//	weapon_flashlight
//-----------------------------------------------------------------------------
void weapon_flashlight(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_flashlight_register_func()
{
	gstate->RegisterFunc("flashlight_think",flashlight_think);
	gstate->RegisterFunc("flashlight_use",flashlight_use);
	gstate->RegisterFunc("flashlightRemove",flashlight_Remove);
	gstate->RegisterFunc("flashlight_give",flashlight_give);
	gstate->RegisterFunc("flashlight_select",flashlight_select);
}