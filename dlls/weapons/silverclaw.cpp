//---------------------------------------------------------------------------
// SILVERCLAW
//---------------------------------------------------------------------------
#include "weapons.h"
#include "silverclaw.h"

//---------------------------------------------------------------------------
// weapon specific info
//---------------------------------------------------------------------------

#define OFS_SLASH             5
#define OFS_DAMAGE            8

enum
{
    // model defs
    MDL_START,
    MDL_SLASH,
    MDL_SLASH2,
    MDL_SLASH3,

    // sound defs
    SND_START,
    SND_SHOOT2,
    SND_SHOOT3,
    SND_SHOOT4,
    SND_HITSTONE,
    SND_HITFLESH,

    // frame defs
    ANIM_START,
    ANIM_SHOOT2,
    ANIM_SHOOT3,
    ANIM_SLASH,
    ANIM_SLASH2,
    ANIM_SLASH3
};

// weapon info def
weaponInfo_t silverclawWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_SILVERCLAW),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_NO_AMMO),
	// names, net names, icon
	"weapon_silverclaw",
	tongue_weapons[T_WEAPON_SILVERCLAW],
	"ammo_none",
	tongue_weapons[T_WEAPON_NO_AMMO],
	"iconname",

    // model files
	{
		"models/e3/w_silverclaw.dkm",
		"models/e3/a_claw.dkm",
		MDL_NOMODEL,
		"models/e3/we_sclawslash.dkm",
		"models/e3/we_sclawslash2.dkm",
		"models/e3/we_sclawslash3.dkm",
		NULL
	},

    // sound files
	{
		SND_NOSOUND,
		"e3/we_sclawready.wav",
		"e3/we_sclawaway.wav",
		"e3/we_sclawshoota.wav",
		"e3/we_sclawamba.wav",
		SND_NOSOUND,
		SND_NOSOUND,
		"e3/we_sclawshootb.wav",
		"e3/we_sclawshootc.wav",
		"e3/we_sclawshootd.wav",
		"e3/we_sclawhit1.wav",
		"e3/we_sclawhit2.wav",
		NULL
	},

	// frame names
	{
		MDL_WEAPON_STD,"ready",0,0,
		-1,"away",0,0,
		-1,"shoota",0,0,
		-1,"amba",0,0,
		-1,"ambb",0,0,
		-1,"ambc",0,0,
		-1,"shootb",0,0,
		-1,"shootc",0,0,
		MDL_SLASH,"stand",0,0,
		MDL_SLASH2,"stand",0,0,
		MDL_SLASH3,"stand",0,0,
		0,
		NULL
	},

	// commands
	{ 
		wpcmds[WPCMDS_SILVERCLAW][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_1,
		wpcmds[WPCMDS_SILVERCLAW][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_1,
		NULL
	},

	weapon_give_1,
	weapon_select_1,
	silverclaw_command,
	silverclaw_use,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(silverclaw_t),

	100,
	0,          //	ammo_max
	0,          //	ammo_per_use
	1,          //	ammo_display_divisor
	0,          //	initial ammo
	0,          //	speed
	100.0,       //	range
	25.0,       //	damage
	0.0,        //	lifetime

	0,  //	flags

	WEAPONTYPE_GLOVE|WFL_NODROP,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon3[T_DEATHMSGWEAPON_SILVERCLAW],
		NULL,
		VICTIM_ATTACKER
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon3,T_DEATHMSGWEAPON_SILVERCLAW),
	VICTIM_ATTACKER,

    WEAPON_SILVERCLAW,
    0,
    NULL
};

// local weapon info access
static weaponInfo_t *winfo=&silverclawWeaponInfo;

//---------------------------------------------------------------------------
// give()
//---------------------------------------------------------------------------
userInventory_t *silverclaw_give(userEntity_t *self, int ammoCount)
{
	if (self->inventory && gstate->InventoryFindItem(self->inventory,winfo->weaponName))
		return NULL;

    return(weaponGive (self, winfo, ammoCount));
}

//---------------------------------------------------------------------------
// silverclaw_select()
//---------------------------------------------------------------------------
short silverclaw_select(userEntity_t *self)
{
    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// command()
//---------------------------------------------------------------------------
void *silverclaw_command(struct userInventory_s *ptr, char *commandStr, void *data)
{
    winfoGenericCommand(winfo,ptr,commandStr,data);

    return(NULL);
}

//---------------------------------------------------------------------------
// use()
//---------------------------------------------------------------------------
void silverclaw_use(userInventory_s *ptr, userEntity_t *self)
{
    silverclaw_t *silverclaw=(silverclaw_t *)self->curWeapon;
    playerHook_t *phook=(playerHook_t *)self->userHook;
    short index;
    char shootList[]={ANIM_SHOOT_STD,ANIM_SHOOT2,ANIM_SHOOT3};
    char animList[]={ANIM_SLASH,ANIM_SLASH2,ANIM_SLASH3};

    index=(short)(rnd()*2.9);
    silverclaw->slashanim=animList[index];
    silverclaw->shootanim=shootList[index];
    winfoAnimate(winfo,self,silverclaw->shootanim,FRAME_ONCE|FRAME_WEAPON,.05);

    // set fx func for damage
    phook->fxFrameNum=winfoFirstFrame(silverclaw->shootanim)+OFS_SLASH;
    phook->fxFrameFunc=silverclaw_think;

    silverclaw->weapon.lastFired=gstate->time;
	WEAPON_FIRED(self);
	weapon_PowerSound(self);
}

//---------------------------------------------------------------------------
// kill_slash()
//---------------------------------------------------------------------------
void kill_slash(userEntity_t *self)
{
//    silverclaw_t *silverclaw=(silverclaw_t *)self->owner->curWeapon;// SCG[1/24/00]: not used
    CVector pos;

    pos=self->owner->s.origin + CVector(16,4,26);
}

//---------------------------------------------------------------------------
// think()
//---------------------------------------------------------------------------
void silverclaw_think(userEntity_t *self)
{
    silverclaw_t *silverclaw=(silverclaw_t *)self->curWeapon;
    playerHook_t *phook=(playerHook_t *)self->userHook;
    CVector org;

    trace_t trace;
    CVector start,end,dir;
    short snd;

    // close attack
    switch ( silverclaw->shootanim )
    {
	default:
    case ANIM_SHOOT_STD:    snd=SND_SHOOT_STD;   break;
    case ANIM_SHOOT2:       snd=SND_SHOOT2;      break;
    case ANIM_SHOOT3:       snd=SND_SHOOT3;      break;
    }
    winfoPlay(self,snd,1.0f);

    dir = self->client->v_angle;
    dir.AngleToVectors(forward,right,up);
    start = self->s.origin + up * 12.0f;

	if( self->client->ps.pmove.pm_flags & PMF_DUCKED )
	{
		start.z -= 25;
	}

    end = start + forward * silverclawWeaponInfo.range;
    trace = gstate->TraceLine_q2(start, end, self, MASK_SHOT);

	if ((self->flags & FL_BOT) && (self->enemy))
	{
		trace.fraction = 0.8;
		trace.endpos = self->enemy->s.origin;
		trace.ent = self->enemy;
	}
    // hit anything?
    if ( trace.fraction!=1 )
    {
        float damage=winfo->damage;

        if ( trace.ent->deadflag != DEAD_NO )
            damage=50;

        if ( trace.ent->takedamage != DAMAGE_NO )
        {
            winfoPlay(self,SND_HITFLESH,1.0f);
			// cek[1-24-00]: set the attackweapon
			winfoAttackWeap;
			WEAPON_HIT(self,1);
            com->Damage(trace.ent,self,self,trace.endpos,forward,damage,DAMAGE_INERTIAL);
            phook->hit++;
        }

        if ( trace.ent->solid==SOLID_BSP || trace.ent->takedamage==DAMAGE_NO )
        {
//			weaponScorchMark(trace,SM_CLAW);
			silverclaw_marks(self,trace);
            winfoPlay(self,SND_HITSTONE,1.0f);
			end = start + forward * (silverclawWeaponInfo.range*trace.fraction);
			dir = start - end;
			dir.Normalize();
			CVector Color;
			Color.Set(1.0,1.0,1.0);
			clientSparks(end,dir,10, Color);
        }
    }

	phook->fxFrameFunc=NULL;
}

void silverclaw_marks(userEntity_t *self, trace_t &trace)
{
	if (trace.contents & CONTENTS_CLEAR || !trace.ent || trace.ent->solid != SOLID_BSP || trace.ent->takedamage != DAMAGE_NO)
		return;

	if (trace.surface && (trace.surface->flags & SURF_FLOWING))
		return;

    silverclaw_t *silverclaw=(silverclaw_t *)self->curWeapon;

	AngleToVectors(self->client->v_angle,forward,right,up);
	right = -right;

	// set up the orientation of this sprite, based on the swipe
	CVector ang = zero_vector;
	switch ( silverclaw->shootanim )
	{
	default:
	case ANIM_SHOOT_STD:    ang.yaw = 220;		break;
	case ANIM_SHOOT2:       ang.yaw = 135;     break;
	case ANIM_SHOOT3:       ang.yaw = 0;		break;
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
	gstate->WriteByte( SM_CLAW );
	gstate->MultiCast( zero_vector, MULTICAST_ALL );
}

//-----------------------------------------------------------------------------
//	weapon_silverclaw
//-----------------------------------------------------------------------------
void weapon_silverclaw(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_silverclaw_register_func()
{
	gstate->RegisterFunc("silverclaw_use",silverclaw_use);
	gstate->RegisterFunc("silverclaw_think",silverclaw_think);
	gstate->RegisterFunc("silverclaw_select",silverclaw_select);
	gstate->RegisterFunc("silverclaw_give",silverclaw_give);
}
