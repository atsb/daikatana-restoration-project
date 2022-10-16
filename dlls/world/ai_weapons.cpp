// ==========================================================================
//
//  File:
//  Contents:
//  Author:
//
// ==========================================================================

//////////////////////////////////////////////////////////////////////////////////
//	ai_weapons:
//
//	Each monster has at least one weapon in its inventory.  These weapons have
//	to be useable by both monsters and clients (for possesion).  In order for
//	a client to use a weapon, the direction of attack must be based on self->client->v_angle.
//
//	For monsters, self->v_angle is set in ai_fire_curWeapon, which then calls
//	the weapon's use function.  Clients only need to directly call the use function.
//
//	Each weapon's use function converts self->client->v_angle to a normalized directional
//	vector and then calculates the aiming vector based on that weapon's spread_x
//	and spread_z and the origin of the firing entity.
//
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
//	generalized functions for monster attacks
//////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER
#include <crtdbg.h>
#endif
#include "world.h"
#include "ai_common.h"
#include "ai_utils.h"
#include "ai_weapons.h"
#include "thinkFuncs.h"
#include "ai_frames.h"
#include "misc.h"
#include "weapondefs.h"
#include "ai_aim.h"
#include "spawn.h"
#include "ai_func.h"

extern float sin_tbl[];
extern float cos_tbl[];
/* ***************************** define types ****************************** */

//#define	DEBUGINFO	1
#define	MARKERS

typedef struct
{
    float   remove_time;
    float   last_touch_time;

    CVector last_pos;
//	int     iRingCount;
//	frictionHook_t  *fhook;
} corditeHook_t;

#define	CORDITEHOOK(x) (int)&(((corditeHook_t *)0)->x)
field_t cordite_hook_fields[] = 
{
	{"remove_time",		CORDITEHOOK(remove_time),		F_FLOAT},
	{"last_touch_time",	CORDITEHOOK(last_touch_time),	F_FLOAT},
	{"last_pos",		CORDITEHOOK(last_pos),			F_VECTOR},
	{NULL, 0, F_INT}
};

typedef struct meteorHook_s
{
    short bounceCount;
    float killtime;
    short colorScale;
} meteorHook_t;

#define	METEORHOOK(x) (int)&(((meteorHook_t *)0)->x)
field_t meteor_hook_fields[] = 
{
	{"bounceCount",	METEORHOOK(bounceCount),	F_SHORT},
	{"killtime",	METEORHOOK(killtime),		F_FLOAT},
	{"colorScale",	METEORHOOK(colorScale),		F_SHORT},
	{NULL, 0, F_INT}
};

#define	WYNDRAXHOOK(x) (int)&(((wyndraxHook_t *)0)->x)
field_t wyndrax_hook_fields[] = 
{
	{"killtime",		WYNDRAXHOOK(killtime),			F_FLOAT},
	{"lightningCount",	WYNDRAXHOOK(lightningCount),	F_SHORT},
	{"lightningList",	WYNDRAXHOOK(lightningList),		F_EDICT},
	{"dummyCount",		WYNDRAXHOOK(dummyCount),		F_SHORT},
	{"dummyList",		WYNDRAXHOOK(dummyList),			F_EDICT},
	{"forward",			WYNDRAXHOOK(forward),			F_VECTOR},
	{"up",				WYNDRAXHOOK(up),				F_VECTOR},
	{"sinofs",			WYNDRAXHOOK(sinofs),			F_SHORT},
	{"sinetime",		WYNDRAXHOOK(sinetime),			F_FLOAT},
	{"Personality",		WYNDRAXHOOK(Personality),		F_FLOAT},
	{NULL, 0, F_INT}
};

#define	LIGHTNINGHOOK(x) (int)&(((lightningHook_t *)0)->x)
field_t lightning_hook_fields[] = 
{
	{"killtime",	LIGHTNINGHOOK(killtime),	F_FLOAT},
	{"owner",		LIGHTNINGHOOK(owner),		F_EDICT},
	{"src",			LIGHTNINGHOOK(src),			F_EDICT},
	{"dst",			LIGHTNINGHOOK(dst),			F_EDICT},
	{"altdst",		LIGHTNINGHOOK(altdst),		F_EDICT},
	{NULL, 0, F_INT}
};

#define	ROCKETHOOK(x) (int)&(((rocketHook_t *)0)->x)
field_t rocket_hook_fields[] = 
{
	{"time_to_live",	ROCKETHOOK(time_to_live),	F_FLOAT},
	{"a_counter",		ROCKETHOOK(a_counter),		F_FLOAT},
	{"r_speed",			ROCKETHOOK(r_speed),		F_FLOAT},
	{NULL, 0, F_INT}
};

#define	STAVEHOOK(x) (int)&(((staveHook_t *)0)->x)
field_t stave_hook_fields[] = 
{
	{"time_to_live",	STAVEHOOK(time_to_live),	F_FLOAT},
	{"roll_speed",		STAVEHOOK(roll_speed),		F_FLOAT},
	{"pitch_speed",		STAVEHOOK(pitch_speed),		F_FLOAT},
	{"yaw_speed",		STAVEHOOK(yaw_speed),		F_FLOAT},
	{"damage",			STAVEHOOK(damage),			F_FLOAT},
	{"bounce_count",	STAVEHOOK(bounce_count),	F_FLOAT},
	{"bounce_max",		STAVEHOOK(bounce_max),		F_FLOAT},
	{"speed_max",		STAVEHOOK(speed_max),		F_FLOAT},
	{NULL, 0, F_INT}
};

#define	PSYCLAWHOOK(x) (int)&(((psyclawHook_t *)0)->x)
field_t psyclaw_hook_fields[] = 
{
	{"delta_angles",	PSYCLAWHOOK(delta_angles),	F_INT},
	{"delta_color",		PSYCLAWHOOK(delta_color),	F_INT},
	{"color_dir",		PSYCLAWHOOK(color_dir),		F_INT},
	{"scale_max",		PSYCLAWHOOK(scale_max),		F_FLOAT},
	{"scale_min",		PSYCLAWHOOK(scale_min),		F_FLOAT},
	{"time_to_live",	PSYCLAWHOOK(time_to_live),	F_FLOAT},
	{"damage",			PSYCLAWHOOK(damage),		F_FLOAT},
	{"delta_scale",		PSYCLAWHOOK(delta_scale),	F_FLOAT},
	{"delta_alpha",		PSYCLAWHOOK(delta_alpha),	F_FLOAT},
	{"c_fov",			PSYCLAWHOOK(c_fov),			F_FLOAT},
	{"kick_angles",		PSYCLAWHOOK(kick_angles),	F_VECTOR},
	{"Psyclaw",			PSYCLAWHOOK(Psyclaw),		F_EDICT},
	{NULL, 0, F_INT}
};

#define	THUNDERJIZHOOK(x) (int)&(((ThunderJizHook_t *)0)->x)
field_t thunder_jiz_hook_fields[] = 
{
	{"time_to_live",	THUNDERJIZHOOK(time_to_live),	F_FLOAT},
	{"type",			THUNDERJIZHOOK(type),			F_INT},
	{"delta_angles",	THUNDERJIZHOOK(delta_angles),	F_FLOAT},
	{"delta_scale",		THUNDERJIZHOOK(delta_scale),	F_FLOAT},
	{NULL, 0, F_INT}
};

#define	CAMLIGHTHOOK(x) (int)&(((camlightHook_t *)0)->x)
field_t camlight_hook_fields[] = 
{
	{"brightness",		CAMLIGHTHOOK(brightness),		F_FLOAT},
	{"length",			CAMLIGHTHOOK(length),			F_FLOAT},
	{"rotation_speed",	CAMLIGHTHOOK(rotation_speed),	F_FLOAT},
	{"search_speed",	CAMLIGHTHOOK(search_speed),		F_FLOAT},
	{"search_angle",	CAMLIGHTHOOK(search_angle),		F_FLOAT},
	{"search_max",		CAMLIGHTHOOK(search_max),		F_FLOAT},
	{"search_min",		CAMLIGHTHOOK(search_min),		F_FLOAT},
	{"flare",			CAMLIGHTHOOK(flare),			F_EDICT},
	{NULL, 0, F_INT}
};

#define	ZAPFLARE(x) (int)&(((ZapFlare_t *)0)->x)
field_t zapflare_hook_fields[] = 
{
	{ "Delta_Angles",	ZAPFLARE(Delta_Angles),	F_VECTOR },
	{ "time_to_live",	ZAPFLARE(time_to_live),	F_FLOAT },
	{ "delta_alpha",	ZAPFLARE(delta_alpha),	F_FLOAT },
	{ "delta_scale",	ZAPFLARE(delta_scale),	F_FLOAT },
	{NULL, 0, F_INT}
};

#define	FLAMESWORD(x) (int)&(((FlameSword_t *)0)->x)
field_t flamesword_hook_fields[] = 
{
	{ "Delta_Angles",	FLAMESWORD(Delta_Angles),	F_VECTOR },
	{ "time_to_live",	FLAMESWORD(time_to_live),	F_FLOAT },
	{ "damage",			FLAMESWORD(damage),			F_FLOAT },
	{ "rnd_damage",		FLAMESWORD(rnd_damage),		F_FLOAT },
	{NULL, 0, F_INT}
};

void  meteor_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, meteor_hook_fields, sizeof( meteorHook_t ) );
}

void  meteor_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, meteor_hook_fields, sizeof( meteorHook_t ) );
}

void wyndrax_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, wyndrax_hook_fields, sizeof( wyndraxHook_t ) );
}

void wyndrax_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, wyndrax_hook_fields, sizeof( wyndraxHook_t ) );
}

void lightning_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, lightning_hook_fields, sizeof( lightningHook_t ) );
}

void lightning_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, lightning_hook_fields, sizeof( lightningHook_t ) );
}

void rocket_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, rocket_hook_fields, sizeof( rocketHook_t ) );
}

void rocket_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, rocket_hook_fields, sizeof( rocketHook_t ) );
}

void stave_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, stave_hook_fields, sizeof( staveHook_t ) );
}

void stave_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, stave_hook_fields, sizeof( staveHook_t ) );
}

void psyclaw_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, psyclaw_hook_fields, sizeof( psyclawHook_t ) );
}

void psyclaw_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, psyclaw_hook_fields, sizeof( psyclawHook_t ) );
}

void thunder_jiz_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, thunder_jiz_hook_fields, sizeof( ThunderJizHook_t) );
}

void thunder_jiz_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, thunder_jiz_hook_fields, sizeof( ThunderJizHook_t ) );
}

void camlight_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, camlight_hook_fields, sizeof( camlightHook_t ) );
}

void camlight_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, camlight_hook_fields, sizeof( camlightHook_t ) );
}

void zapflare_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, zapflare_hook_fields, sizeof( ZapFlare_t ) );
}

void zapflare_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, zapflare_hook_fields, sizeof( ZapFlare_t ) );
}

void flamesword_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, flamesword_hook_fields, sizeof( FlameSword_t ) );
}

void flamesword_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, flamesword_hook_fields, sizeof( FlameSword_t ) );
}

void cordite_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, cordite_hook_fields, sizeof( corditeHook_t ) );
}

void cordite_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, cordite_hook_fields, sizeof( corditeHook_t ) );
}

/* ***************************** Local Variables *************************** */

static int  trace_damage;

/* ***************************** Local Functions *************************** */

userEntity_t	*ai_fire_projectile ( userEntity_t *self, userEntity_t *target, ai_weapon_t *weapon, char *model, touch_t touch, unsigned long fx, AIMDATA_PTR aim_data );

int AI_IsVisible( userEntity_t *self, userEntity_t *targ );
int AI_IsInFOV( userEntity_t *self, userEntity_t *targ );

userEntity_t *DoFlash( userEntity_t *self, CVector& vec, ai_weapon_t *weapon );
void flash_think( userEntity_t *self );
void uzigun_think( userEntity_t *self );

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */
//void smallMeteorSpawn(userEntity_t *self);

userEntity_t *spawnZap(userEntity_t *owner,CVector src, userEntity_t *dst, CVector dest_origin, float killtime);
void removeZap(userEntity_t *self);
//void AI_Drop_Markers (CVector &Target, float delay);


/* ******************************* exports ********************************* */


//*****************************************************************************
//					Initialization functions
//*****************************************************************************
//-----------------------------------------------------------------------------
//	weaponScorchMark() -//NSS[10/28/99]:
//-----------------------------------------------------------------------------
void weaponScorchMark2(userEntity_t *self, userEntity_t *world,cplane_t *plane)
{
    
	if(world == NULL || plane == NULL)
		return;
	if(_stricmp(world->className,"worldspawn"))
		return;
	trace_t trace;
    CVector end,start;
	
	//NSS[10/28/99]:Changed the projection of the next spot.
	float vLength = self->velocity.Length() * 0.1f;	
    end = self->velocity;
	end.Normalize();
	start = (end * (-1*vLength)) + self->s.origin;
	end = (end * vLength) + self->s.origin;
	
    trace=gstate->TraceLine_q2(self->s.origin,end,self,MASK_SHOT);
	
    gstate->WriteByte(SVC_TEMP_ENTITY);
    gstate->WriteByte(TE_SCORCHMARK);
    gstate->WritePosition(trace.endpos);
    gstate->WriteShort(plane->planeIndex);
    gstate->WriteShort(world->s.modelindex);                //	write model index so that we can stick to
    //	bmodels correctly
    gstate->WriteShort(world->s.number);                    // entity number for bmodels
    gstate->WriteFloat( 1 );
    gstate->WriteFloat( 0 );
    gstate->WriteByte( SM_SCORCH );
    gstate->MultiCast(trace.endpos,MULTICAST_PVS);
}

//---------------------------------------------------------------------------
// spawnPolyExplosion()
//---------------------------------------------------------------------------
void spawnPolyExplosion(CVector &org, float scale, float lsize, CVector &color, short flags)
{
    CVector angles(0,0,0);

    // send explosion message
    gstate->WriteByte(SVC_TEMP_ENTITY);
    if ( flags & PEF_SOUND )
    {
        gstate->WriteByte(TE_POLYEXPLOSION_SND);
    }
	else
    {
        gstate->WriteByte(TE_POLYEXPLOSION);
    }

    gstate->WritePosition(org);
    gstate->WritePosition(angles);
    gstate->WriteShort((unsigned short)(scale*1024));
    gstate->WriteFloat(lsize);
    if ( lsize )
    {
        gstate->WritePosition(color);
    }
    gstate->MultiCast(org,MULTICAST_PVS);
}

//---------------------------------------------------------------------------
// spawnPolyExplosion()
//---------------------------------------------------------------------------
void spawnPolyExplosion(CVector &org, CVector &vec, float scale, float lsize, CVector &color, short flags)
{
    CVector angles(0,0,0);

    if(&vec == NULL)
	{
		spawnPolyExplosion( org, scale, lsize, color, flags);
		return;
	}

	// handle flags
    if ( (flags & PEF_ANGLES) )
    {
        angles=vec;
    }
    if ( (flags & PEF_NORMAL) )
    {
        VectorToAngles(vec, angles);

        org.x += vec.x * 4;
        org.y += vec.y * 4;
        org.z += vec.z * 4;
    }

    // send explosion message
    gstate->WriteByte(SVC_TEMP_ENTITY);
    if ( flags & PEF_SOUND )
    {
        gstate->WriteByte(TE_POLYEXPLOSION_SND);
    }
    else
    {
        gstate->WriteByte(TE_POLYEXPLOSION);
    }

    gstate->WritePosition(org);
    gstate->WritePosition(angles);
    gstate->WriteShort((unsigned short)(scale*1024));
    gstate->WriteFloat(lsize);
    if ( lsize )
    {
        gstate->WritePosition(color);
    }
    gstate->MultiCast(org,MULTICAST_PVS);
}



// ----------------------------------------------------------------------------
// <nss>
// Name:		ZapFlareThink
// Description:Flare for Wyndrax's attack
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void ZapFlareThink(userEntity_t *self)
{
	lightningHook_t *hook = (lightningHook_t *)self->userHook;
	if(hook->killtime < gstate->time)
	{
		gstate->RemoveEntity(self);
	}
	self->s.alpha = self->s.alpha * 0.75;
	self->s.render_scale = self->s.render_scale * 0.85;
	self->s.angles.roll += 25.0f;
	self->nextthink		= gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		spawnZapFlare
// Description:The first of the two attacks for Wyndrax or any other weapon that
// needs a cool flare fx
// Input:userEntity_t *self, CVector *Origin
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t * spawnZapFlare(userEntity_t *self, CVector *Origin, float life, float size, char *flarename)
{
	userEntity_t *Flare;
    lightningHook_t *hook;
	
	
	Flare = gstate->SpawnEntity();
	Flare->s.render_scale.Set(size,size, size);
	
	Flare->className		= "flare_sprite";
	Flare->s.renderfx		|= (SPR_ALPHACHANNEL);
	Flare->s.modelindex		= gstate->ModelIndex (flarename);
	Flare->movetype			= MOVETYPE_NONE;
	Flare->solid			= SOLID_NOT;
	Flare->owner			= self;
	Flare->s.alpha			= 0.75f;
	Flare->s.frame			= 0;
	Flare->s.origin.x		= Origin->x;
	Flare->s.origin.y		= Origin->y;
	Flare->s.origin.z		= Origin->z;
	Flare->s.frame			= 0;
    Flare->think			= ZapFlareThink;
    Flare->nextthink		= gstate->time + 0.1f;
	Flare->clipmask			= MASK_SHOT;
	Flare->userHook			= gstate->X_Malloc(sizeof(lightningHook_t),MEM_TAG_HOOK);
	hook=(lightningHook_t *)Flare->userHook;
	
	// SCG[11/24/99]: Save game stuff
	Flare->save = lightning_hook_save; 
	Flare->load = lightning_hook_load;
    
	hook->killtime			=gstate->time+life;
	
	gstate->LinkEntity(Flare);
	
	return Flare;
}


// ----------------------------------------------------------------------------
// <nss>
// Name:		ZapFlareRotateThink
// Description:Flare for Wyndrax's attack
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void ZapFlareRotateThink(userEntity_t *self)
{
	ZapFlare_t *hook	= (ZapFlare_t *)self->userHook;
	if(hook->time_to_live < gstate->time)
	{
		gstate->RemoveEntity(self);
	}
	self->s.alpha = self->s.alpha * hook->delta_alpha;
	self->s.render_scale = self->s.render_scale * 0.85;
	self->s.angles = self->s.angles + hook->Delta_Angles;
	self->nextthink		= gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		spawnZapFlareRotate
// Description: This is a generic flare routine which can be used to create nice
// flare FX for monster's firing weapons and such.
// Input:userEntity_t *self, CVector *Origin
// Output:NA
// Note:Changing the scale values so that they are not uniform will result in sharper
//edges.  Life is the base for the depletion rate of scale and alpha level
// ----------------------------------------------------------------------------
userEntity_t * spawnZapFlareRotate(userEntity_t *self, CVector *Origin, CVector Rotate,CVector Size,float life, char *flarename)
{
	userEntity_t *Flare;
    ZapFlare_t *hook;
	
	
	Flare = gstate->SpawnEntity();
	Flare->s.render_scale = Size;
	
	Flare->className		= "flare_sprite";
	Flare->s.renderfx		|= (SPR_ALPHACHANNEL|SPR_ORIENTED);
	Flare->s.modelindex		= gstate->ModelIndex (flarename);
	Flare->movetype			= MOVETYPE_NONE;
	Flare->solid			= SOLID_NOT;
	Flare->owner			= self;
	Flare->s.alpha			= 0.75f;
	Flare->s.frame			= 0;
	Flare->s.origin.x		= Origin->x;
	Flare->s.origin.y		= Origin->y;
	Flare->s.origin.z		= Origin->z;
	Flare->s.frame			= 0;
    Flare->think			= ZapFlareRotateThink;
    Flare->nextthink		= gstate->time + 0.1f;
	Flare->clipmask			= MASK_SHOT;
	Flare->s.angles			= self->s.angles;
	Flare->userHook			= gstate->X_Malloc(sizeof(ZapFlare_t),MEM_TAG_HOOK);
	hook					=( ZapFlare_t *)Flare->userHook;
    hook->Delta_Angles		= Rotate;
	hook->time_to_live		= gstate->time+life;

	// SCG[11/24/99]: Save game stuff
	Flare->save = zapflare_hook_save;
	Flare->load = zapflare_hook_load;

	if(life > 1.0f)
	{
		hook->delta_alpha = 0.95f;
		hook->delta_scale = 0.85f;
	}
	else
	{
		hook->delta_alpha = life;
		hook->delta_scale = life;
	}
	
	
	gstate->LinkEntity(Flare);
	
	return Flare;
}


//---------------------------------------------------------------------------
// trackLight()
//---------------------------------------------------------------------------
void AddTrackLight(userEntity_t *ent,float size,float r,float g,float b,long fxflags)
{
 	//Setup special effects for the fireball
    trackInfo_t tinfo;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=ent;
    tinfo.srcent=ent;
    tinfo.fru.Zero();
    tinfo.lightColor.x = r;//R
    tinfo.lightColor.y = g;//G
    tinfo.lightColor.z = b;//B
    tinfo.lightSize= size;
    tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX;
    tinfo.fxflags = fxflags;
    com->trackEntity(&tinfo,MULTICAST_PVS);
}

// ----------------------------------------------------------------------------
//
// Name:		ai_weapon_command
// Description:
//		standard command procedure for a monster weapon
//
//		default commands:
//			base_damage - sets the damage weapon does
//			rnd_damage	- the amount of random damage a weapon does
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void *ai_weapon_command( struct userInventory_s *ptr, char *commandStr, void *data )
{
	ai_weapon_t	*weapon = (ai_weapon_t *) ptr;
	
	if ( !stricmp(commandStr, "base_damage") )
	{
		weapon->base_damage = *(int *) data;
	}
	else 
	if ( !stricmp(commandStr, "rnd_damage") )
	{
		weapon->rnd_damage = *(int *) data;
	}
	else 
	if ( !stricmp(commandStr, "spread_x") )
	{
		weapon->spread_x = *(float *) data;
	}
	else 
	if ( !stricmp(commandStr, "spread_z") )
	{
		weapon->spread_z = *(float *) data;
	}
	else 
	if ( !stricmp(commandStr, "speed") )
	{
		weapon->speed = *(float *) data;
	}
	else 
	if ( !stricmp(commandStr, "distance") )
	{
		weapon->distance = *(float *) data;
	}
	else 
	if ( !stricmp(commandStr, "ofs") )
	{
		weapon->ofs = *(CVector *) data;
	}

	return NULL;

}

// ----------------------------------------------------------------------------
//
// Name:		ai_weapon_create
// Description:
//				creates an inventory object for a monster weapon
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userInventory_t	*ai_weapon_create( invenList_t *inventory, char	*weaponName, invenUse_t useFunc, invenCommand_t cmdFunc )
{
	ai_weapon_t	*weapon;
	int			structSize;

	_ASSERTE (inventory);

	structSize = sizeof (ai_weapon_t);
	weapon = (ai_weapon_t *) gstate->InventoryCreateItem (inventory, weaponName, (invenUse_t)useFunc, 
							(invenCommand_t)cmdFunc, -1, ITF_WEAPON, structSize); 

	// cek[8-3-00]: check das pointer
	if (!weapon)
		return NULL;
	
	weapon->base_damage = 10;
	weapon->rnd_damage = 0;

	return (userInventory_t *) weapon;
}


// ----------------------------------------------------------------------------
//
// Name:		ai_init_weapon
// Description:
//				set up a monster weapon with all of its attributes
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userInventory_t	*ai_init_weapon( userEntity_t *self, float base_damage, float rnd_damage,
						float spread_x, float spread_z, float speed, float dist, const CVector &ofs, 
						char *name, weaponUse_t use_func, unsigned long flags )
{
	_ASSERTE( self->inventory );

	ai_weapon_t *weapon = (ai_weapon_t *) ai_weapon_create( self->inventory, name, use_func, ai_weapon_command );

	weapon->base_damage	= base_damage;
	weapon->rnd_damage	= rnd_damage;
	weapon->spread_x	= spread_x;
	weapon->spread_z	= spread_z;
	weapon->speed		= speed;
	weapon->distance	= dist;
	weapon->ofs			= ofs;
	weapon->weapon.flags = flags;

	return	(userInventory_t *) weapon;
}

//*****************************************************************************
//					utility functions used for monster weapons
//*****************************************************************************

// ----------------------------------------------------------------------------
//
// Name:		ai_check_projectile_attack
// Description:
// Input:
// Output:
// Note:
//	aims a projectile attack and determines if it will hit the desired target
//
//	returns true if a trace from the attacker to the target's predicted position
//	is not obstructed
//
//	if a trace from the attacker to the target's predicted position is obstructed,
//	then true is returned if the trace went farther than min_dist units, otherwise
//	false is returned.
//
// ----------------------------------------------------------------------------
int	ai_check_projectile_attack( userEntity_t *self, userEntity_t *target, 
								ai_weapon_t *weapon, float min_dist )
{
	if ( min_dist == 0 )
	{
		//	explosive weapons have a radius == damage
		min_dist = weapon->base_damage + 32;
	}

	AIMDATA_PTR pAimData = ai_aim_curWeapon( self, weapon );

	float dist = VectorDistance( self->s.origin, target->s.origin );

	CVector end = pAimData->org + pAimData->dir * dist;
	tr = gstate->TraceLine_q2( pAimData->org, end, self, MASK_SHOT );

	if ( tr.fraction == 1.0 )
	{
		//	trace made it all the way to predicted position
		return TRUE;
	}
	else 
	if ( tr.ent == target )
	{
		//	trace hit the target
		return TRUE;
	}
	else 
	if ( (tr.ent->flags & FL_MONSTER) &&
	     AI_GetPlayerHook( self ) && AI_GetPlayerHook( tr.ent ) &&
	     (AI_GetPlayerHook( self )->dflags & DFL_EVIL) == (AI_GetPlayerHook( tr.ent )->dflags & DFL_EVIL) )
	{
		// trace hit a monster of equal evilness, so don't shoot
		return FALSE;
	}
	else 
	if ( tr.ent->flags & (FL_CLIENT + FL_BOT) )
	{
		// trace hit a client or bot, so fire anyway
		return TRUE;
	}
	else 
	if ( tr.fraction * dist > min_dist )
	{
		//	check to see if trace travelled far enough before impact
		return TRUE;
	}

	return FALSE;
}



// ----------------------------------------------------------------------------
//
// Name:		ai_fire_projectile
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *ai_fire_projectile( userEntity_t *self, userEntity_t *target, ai_weapon_t *weapon,
								  char *model, touch_t touch, unsigned long fx, AIMDATA_PTR aim_data )
{
	userEntity_t	*temp;
	CVector			org, end, dir, ang, p_org;

	AIMDATA_PTR pAimData = aim_data ? aim_data : ai_aim_curWeapon( self, weapon );

	temp = gstate->SpawnEntity();
	temp->movetype	= MOVETYPE_FLYMISSILE;
	temp->solid		= SOLID_BBOX;
	temp->s.effects = fx;
	temp->owner		= self;
	temp->s.frame	= 0;

	// SCG[11/19/99]: Don't save projectiles
	temp->flags |= FL_NOSAVE;

	temp->clipmask	= MASK_SHOT;

	temp->s.modelindex = gstate->ModelIndex (model);
	
	gstate->SetSize(temp, 0, 0, 0, 0, 0, 0);

	gstate->SetOrigin2( temp, pAimData->org );

	dir = pAimData->dir * weapon->speed;
	temp->velocity	= dir;

	//	align model in direction of travel
	VectorToAngles( pAimData->dir, ang );
	temp->s.angles	= ang;

	temp->touch		= touch;
	temp->think		= NULL;
	temp->nextthink = -1;

	return	temp;
}

//*****************************************************************************
//					monster weapons functions
//*****************************************************************************

// ----------------------------------------------------------------------------
//
// Name:		ai_muzzle_flash_think
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_muzzle_flash_think( userEntity_t *self )
{
	CVector	org, ang, dir;

	self->s.frame++;
	if (self->s.frame > 3)
		self->s.frame = 0;
/*
	//	track without locking to a surface
	if (!self->message)
	{
NoSurface:
		ang = self->owner->s.angles;
		self->s.angles = ang;
		ang.AngleToVectors (forward, right, up);

		org = self->owner->s.origin;
		org = org + forward * self->view_ofs[1] + right * self->view_ofs[0] + up * self->view_ofs[2];

		gstate->SetOrigin2 (self, org.vec ());

		gstate->Con_Printf ("No surface for muzzle flash!\n");
	}
	else
	{
		//	lock to surface self->message
		i = gstate->SurfInfo (self->owner->modelIndex, self->message);
		
		//	ooh, Nelno bad!
		if (i <= 0) goto NoSurface;

		gstate->VertInfo (self->owner, i, self->owner->s.frame, 0, vert [0]);
		gstate->VertInfo (self->owner, i, self->owner->s.frame, 1, vert [1]);
		gstate->VertInfo (self->owner, i, self->owner->s.frame, 2, vert [2]);
		gstate->VertInfo (self->owner, i, self->owner->s.frame, 3, vert [3]);

		self->s.origin = vert[3];
		self->s.origin [2] -= 24.0;

//		com->Normal (vert [0], vert [1], vert [2], dir);
//		VectorToAngles( dir, ang );

		ang = self->owner->s.angles;
		self->s.angles = ang;
		ang.AngleToVectors(forward, right, up);

		org = self->s.origin + forward * 30.0;
		gstate->SetOrigin2 (self, org.vec ());
	}
*/
	if ( gstate->time >= self->delay )
	{
//////
//////		if (self->owner)
/////			gstate->hr_RemoveChild (self->owner, self, false);
		gstate->RemoveEntity(self);
	}
	else
	{
		self->think = ai_muzzle_flash_think;
		self->nextthink = gstate->time + THINK_TIME;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		ai_muzzle_flash
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_muzzle_flash( userEntity_t *owner, const CVector &ofs, float time, 
					  char *surf1, char* surf2, unsigned long flags )
{
	//	amw: this code uses the old hierarchical shit and it's going away
	//
	return;

	userEntity_t *flash = gstate->SpawnEntity ();
	flash->movetype = MOVETYPE_NONE;
	flash->solid = SOLID_NOT;
	flash->owner = owner;
	flash->delay = gstate->time + time;
	flash->message = surf1;
	flash->className = "muzzle_flash";

	gstate->SetModel (flash, "models/global/e_muzflash.dkm");
	gstate->SetSize (flash, 0, 0, 0, 0, 0, 0);
	
//	flash->view_ofs = ofs;

///////////	gstate->hr_InitSurface (owner, flash, surf1, 0, flags);

	ai_muzzle_flash_think (flash);
}

//////////////////////////////////////////////////////////////////////////////////////
//	monster weapon types
//
//	usuable just like player weapons when possessing an enemy
//////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
//
// Name:		ai_fire_playerWeapon
// Description:
// Input:
// Output:
// Note:
//	fires curWeapon in direction from org to end
//
// ----------------------------------------------------------------------------
void ai_fire_playerWeapon( userEntity_t *self, const CVector &org, const CVector &end, 
						   float spread_x, float spread_z, int fire_frame )
{
	CVector			old_angles, dir, ang;
	playerHook_t	*hook = AI_GetPlayerHook( self );
	ai_weapon_t		*weapon = (ai_weapon_t *) self->curWeapon;

	if ( !self->curWeapon )
	{
		return;
	}

	//	save current view angles
	if( self->client )
	{
		old_angles = self->client->v_angle;
	}

	//	set self->client->v_angle to direction of attack
	ai_aim_playerWeapon(self, weapon, fire_frame * 0.1);

	if ( fire_frame >= 0 )
	{
		hook->fxFrameNum = self->s.frame + fire_frame - 1;
	}

	// amw: moved here so that we can use the ai weapons with other entities that don't have playerHook_t's
	// play the sound
	if ( hook->sound1 && !AI_IsSoundDisabled() )
	{
		gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex(hook->sound1), 
                                 1.0f, hook->fMinAttenuation, hook->fMaxAttenuation );
	}

	//	fire the weapon
	self->curWeapon->use( self->curWeapon, self );
}

// ----------------------------------------------------------------------------
//
// Name:		ai_fire_curWeapon
// Note:
//	fires curWeapon in direction from org to end
//	polymorphed version for non-player weapon firing
//
// ----------------------------------------------------------------------------
void ai_fire_curWeapon( userEntity_t *self )
{
	ai_weapon_t *weapon = (ai_weapon_t *) self->curWeapon;

	// check for weapon
	if ( !weapon ) 
	{
		return;
	}

	//	fire the weapon
	self->curWeapon->use(self->curWeapon, self);
}

/* ************************************ psiblast ************************************** */

// ----------------------------------------------------------------------------
//
// Name:		psiblast_firethink
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void psiblast_firethink( userEntity_t *self )
{
	CVector mins(-16, -16, -24);
	CVector maxs(16, 16, 32);

	tr = gstate->TraceBox_q2(self->s.origin, self->s.origin, mins, maxs, self, MASK_SOLID);
	if ( tr.ent )
	{
		if ( tr.ent->takedamage != DAMAGE_NO )
		{
			com->Damage( tr.ent, self->owner, self->owner, self->s.origin, zero_vector, rnd() * 5 + 5, DAMAGE_INERTIAL );
			if ( tr.ent->flags & FL_CLIENT )
			{
				playerHook_t *hook = AI_GetPlayerHook( tr.ent );
				hook->shake_time = gstate->time + 0.25;
			}
		}
	}

	self->s.render_scale = self->s.render_scale * 1.25;

	if ( self->delay <= gstate->time )
	{
		gstate->RemoveEntity(self);
	}
	else
	{
		self->think = psiblast_firethink;
		self->nextthink = gstate->time + 0.1;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		psiblast_fire
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void psiblast_fire (userInventory_s *ptr, userEntity_t *self)
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	ai_weapon_t	*weapon = (ai_weapon_t *) ptr;

	hook->attack_finished = gstate->time + 1.0;

	// FIXME: need to get real psiblast
	userEntity_t *blast = ai_fire_projectile( self, self->enemy, weapon, "models/e1/we_bolt.dkm",
	                                          NULL, 0, NULL );

	blast->movetype = MOVETYPE_NOCLIP;
	blast->solid = SOLID_NOT;
	
	blast->s.render_scale.Set(1.0, 1.0, 1.0);
	
	blast->delay = gstate->time + 2.0;

	psiblast_firethink( blast );
}

/* ************************************ melee ************************************** */

// ----------------------------------------------------------------------------
//
// Name:		ai_trace_attack
// Description:
// Input:
// Output:
// Note:
//			Standard melee attack.
//			Fires from pAimData->org to pAimData->end.  Attacks reach max of weapon->distance units.
//
// ----------------------------------------------------------------------------
userEntity_t *ai_trace_attack( userEntity_t *self, ai_weapon_t *weapon )
{
	AIMDATA_PTR pAimData = ai_aim_curWeapon(self, weapon);

	tr = gstate->TraceLine_q2( pAimData->org, pAimData->end, self, MASK_SHOT );
	
	//Melee PUNK code goes here!  If they are dead let's not bother with trace bullshit
	//and just punk the fuck out of the body.
	if(self->enemy)
	{
		if (self->enemy->deadflag != DEAD_PUNK && self->enemy->deadflag == DEAD_DEAD)
		{
			CVector dir = pAimData->end - pAimData->org;
			dir.Normalize();
			trace_damage = (float) weapon->base_damage + rnd() * float (weapon->rnd_damage);
			com->Damage( self->enemy, self, self, tr.endpos, dir, trace_damage, DAMAGE_NONE );
			//AI_AddNewGoal(self,GOALTYPE_WANDER);
			return NULL;
		}
	}
	

	if (tr.fraction!= 1.0)
	{
		if (tr.ent->takedamage != DAMAGE_NO)
		{
			CVector dir = pAimData->end - pAimData->org;
			dir.Normalize();

			trace_damage = (float) weapon->base_damage + rnd() * float (weapon->rnd_damage);
			com->Damage( tr.ent, self, self, tr.endpos, dir, trace_damage, DAMAGE_NONE );

			return	tr.ent;
		}
		else 
		if (tr.ent->solid == SOLID_BSP || tr.ent->takedamage == DAMAGE_NO)
		{
			return	tr.ent;
		}
	}

	return	NULL;
}

// ----------------------------------------------------------------------------
//
// Name:		melee_punch
// Description:
// Input:
// Output:
// Note:
//	"fires" a standard melee attack
//	which reaches hook->attack_dist
//
//	only plays hook->sound2 if target is hit
//
//	hook->sound1 can be used in the monster's code to
//	play a swinging sound
//
// ----------------------------------------------------------------------------
void melee_punch( userInventory_s *ptr, userEntity_t *self )
{
	playerHook_t	*hook = AI_GetPlayerHook( self );
	ai_weapon_t		*weapon = (ai_weapon_t *) ptr;

	if ( ai_trace_attack (self, weapon) )
	{
		if ( hook->sound2 && !AI_IsSoundDisabled() )
		{
			gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex(hook->sound2),
                                      1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		melee_swing
// Description:
// Input:
// Output:
// Note:
//	"fires" a standard melee attack
//	which reaches hook->attack_dist
//
//	plays hook->sound1
//
// ----------------------------------------------------------------------------
void melee_swing( userInventory_s *ptr, userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	ai_weapon_t	*weapon = (ai_weapon_t *) ptr;

	ai_trace_attack( self, weapon );
}


/* ************************************ chaingun ************************************** */

// ----------------------------------------------------------------------------
//
// Name:		ai_fire_bullet
// Description:
// Input:
// Output:
// Note:
//	Spread is the distance to right or left of the target
//
//	Fires from pAimData->org to pAimData->end (ignores weapon->distance)
//
// ----------------------------------------------------------------------------
CVector ai_fire_bullet( userEntity_t *self, userEntity_t *target, ai_weapon_t *weapon )
{
	AIMDATA_PTR pAimData = ai_aim_curWeapon( self, weapon );

	tr = gstate->TraceLine_q2( pAimData->org, pAimData->end, self, MASK_SHOT );
    
    //com->DrawLine (self, pAimData->org, pAimData->end, 0.1);

    CVector dir = pAimData->end - pAimData->org;
	dir.Normalize();
	int skill = (gstate->GetCvar("skill")+1);
	

	if ( tr.ent && tr.ent->takedamage != DAMAGE_NO && rnd() > (0.50/skill))
	{
		float fActualDamage = (float)weapon->base_damage + rnd() * float(weapon->rnd_damage);
		if(self->owner)
			com->Damage( tr.ent, self->owner, self->owner, tr.endpos, dir, fActualDamage, DAMAGE_INERTIAL );
		else
			com->Damage( tr.ent, self, self, tr.endpos, dir, fActualDamage, DAMAGE_INERTIAL );
		// play random ricochet sound
		if(rnd() < 0.85f)
		{
		char soundbuf[32];
		char n = 97 + (rnd()*4);
//		sprintf(soundbuf, "global/e_bulflesh%c.wav",n );
		Com_sprintf(soundbuf, sizeof(soundbuf),"global/e_bulflesh%c.wav",n );
		gstate->StartEntitySound(target, CHAN_AUTO, gstate->SoundIndex(soundbuf),0.85f, 128.0f, 256.0f);
		}
	}
	else 
	if ( tr.fraction != 1.0 )
	{
      // draw sparks where bullet misses 
	  gstate->WriteByte (SVC_TEMP_ENTITY);
	  gstate->WriteByte (TE_LASER_SPARKS);
	  gstate->WriteByte (20);
	  gstate->WritePosition (tr.endpos);
	  gstate->WriteDir (tr.plane.normal);
	  gstate->WriteByte (1);
	  gstate->MultiCast (tr.endpos, MULTICAST_PVS);
      
      
	  if(target->deadflag == DEAD_DEAD)
	  {
		com->Damage( target, self, self, target->s.origin, dir, 10, DAMAGE_INERTIAL );
	  }
	  // play random ricochet sound
      if(rnd() < 0.75f)
	  {
		char soundbuf[32];
		char n = 97 + (rnd()*8);
//		sprintf(soundbuf, "global/e_ricochet%c.wav",n );
		Com_sprintf(soundbuf, sizeof(soundbuf), "global/e_ricochet%c.wav",n );
		gstate->StartEntitySound(target, CHAN_AUTO, gstate->SoundIndex(soundbuf),0.85f, 64.0f, 256.0f);
	  }
    } 
	return tr.endpos;
}

// ----------------------------------------------------------------------------
//
// Name:		ai_fire_pellet
// Description: Fires a pellet, which is like a bullet but returns
//              TRUE rather than starting a ricochet sound.
// Input:
// Output:
// Note:
//	Spread is the distance to right or left of the target
//
//	Fires from pAimData->org to pAimData->end (ignores weapon->distance)
//
// ----------------------------------------------------------------------------
int ai_fire_pellet( userEntity_t *self, userEntity_t *target, ai_weapon_t *weapon, int spark_fx )
{
	AIMDATA_PTR pAimData = ai_aim_curWeapon( self, weapon );

	tr = gstate->TraceLine_q2( pAimData->org, pAimData->end, self, MASK_SHOT );

	CVector dir = pAimData->end - pAimData->org;
	dir.Normalize();

	if ( tr.ent && tr.ent->takedamage != DAMAGE_NO )
	{
		float fActualDamage = (float)weapon->base_damage + rnd() * float(weapon->rnd_damage);
		com->Damage( tr.ent, self, self, tr.endpos, dir, fActualDamage, DAMAGE_INERTIAL );
	}
	else 
	if ( tr.fraction != 1.0 )
	{
		if ( spark_fx )
		{
			// draw sparks where bullet misses 
			gstate->WriteByte (SVC_TEMP_ENTITY);
			gstate->WriteByte (TE_LASER_SPARKS);
			gstate->WriteByte (20);
			gstate->WritePosition (tr.endpos);
			gstate->WriteDir (tr.plane.normal);
			gstate->WriteByte (1);
			gstate->MultiCast (tr.endpos, MULTICAST_PVS);
		}
		return TRUE;
	}

	return FALSE;
}

void AI_Adjust_Offset(userEntity_t *ent,CVector *Offset)
{

	CVector N_Offset,fAng,forward,right,up, my_forward,my_right,my_up;

	N_Offset.Set(Offset->x,Offset->y,Offset->z);

	fAng = ent->owner->s.angles;
	fAng.AngleToVectors(forward, right, up);
	
	right = -right;

	// change matrix orientation
	my_forward.x = forward.x; 
	my_forward.y = right.x;  
	my_forward.z = up.x;
	my_right.x = forward.y; 
	my_right.y = right.y; 
	my_right.z = up.y;
	my_up.x = forward.z; 
	my_up.y = right.z; 
	my_up.z = up.z;

	//Apply rotation
	ent->s.origin.x = DotProduct( N_Offset, my_forward) + ent->owner->s.origin.x;
	ent->s.origin.y = DotProduct( N_Offset, my_right ) + ent->owner->s.origin.y;
	ent->s.origin.z = DotProduct( N_Offset, my_up ) + ent->owner->s.origin.z;

	//Apply new facing
	ent->s.angles.yaw = ent->owner->s.angles.yaw;
	ent->s.angles.pitch = ent->owner->s.angles.pitch;
	ent->s.angles.roll = ent->owner->s.angles.roll;
}

// ----------------------------------------------------------------------------
// NSS[11/11/99]:
// Name:		chaingun_think
// Description:
// Input:
// Output:
// Note:
//	self->owner is the entity that is shooting
//
// ----------------------------------------------------------------------------
void chaingun_think( userEntity_t *self)
{
	if ( !self->owner || !self->owner->enemy )
	{
		
		RELIABLE_UNTRACK(self);	
		gstate->RemoveEntity(self);
		return;
	}

//	playerHook_t *hook = AI_GetPlayerHook( self->owner );// SCG[1/23/00]: not used
	ai_weapon_t *weapon = (ai_weapon_t *) self->owner->curWeapon;

	//Fire at the player
	if(self->owner)
	{
		if(self->owner->enemy)
			ai_fire_bullet(self->owner, self->owner->enemy, weapon);
	}
	else
	{
		ai_fire_bullet(self->owner, self->owner->enemy, weapon);
	}

	self->delay++;
	
	if (self->delay >= 5)
	{
		self->owner->s.renderfx &= ~RF_MUZZLEFLASH;
		//remove the tracked entities
		RELIABLE_UNTRACK(self);
		if(self->owner)
			RELIABLE_UNTRACK(self->owner);
		gstate->RemoveEntity(self);
		return;
	}
	else
	{
		self->nextthink = gstate->time + 0.01f;
	}
}


// ----------------------------------------------------------------------------
//
// Name:		chaingun_fire
// Description:
// Input:
// Output:
// Note:
//	fires the chaingunner's chaingun
//	which fires several shots one after another
//
// ----------------------------------------------------------------------------
void chaingun_fire( userInventory_s *ptr, userEntity_t *self )
{
	CVector Dir;
	ai_weapon_t *weapon = (ai_weapon_t *) self->curWeapon;

	//userEntity_t *chain = ai_fire_projectile( self, self->enemy, weapon, "models/global/e_sflred.sp2",NULL, 0, NULL );

	userEntity_t *chain = gstate->SpawnEntity();
	
	
	gstate->SetSize(chain, 0, 0, 0, 0, 0, 0);
	chain->movetype	= MOVETYPE_NONE;
	
	chain->owner	= self;
	chain->s.frame	= 0;

	chain->s.modelindex = gstate->ModelIndex ("models/global/e_sflred.sp2");
	chain->s.render_scale.Set(0.001f, 0.001f, 0.001f);
	chain->solid		= SOLID_NOT;
	chain->curWeapon	= self->curWeapon;
	//chain->s.modelindex = NULL;
	
	chain->think = chaingun_think;
	
	chain->nextthink = gstate->time + 0.01f;
	
	chain->delay = 0;
	
	chain->enemy  = self->enemy;

	//RF_MUZZLEFLASH uses the modelindex3 set at the creature's spawn time.
	//i.e. self->s.modelindex3 = gstate->ModelIndex("models/global/me_mflash.dkm");
	chain->owner->s.renderfx |= RF_MUZZLEFLASH;


	AIMDATA_PTR pAimData = ai_aim_curWeapon( chain, weapon );
	gstate->SetOrigin2( chain, pAimData->org );

	gstate->LinkEntity(chain);


	CVector end  = ai_fire_bullet(chain, self->enemy, weapon);
	//Tracer Bullshit
    trackInfo_t tinfo;

    VectorToAngles(chain->velocity,Dir);	
	// clear this variable
    memset(&tinfo, 0, sizeof(tinfo));
    
	if( _stricmp( self->className, "monster_sealcommando" ) == 0)
	{
		tinfo.fru.Zero();
		tinfo.ent			= chain->owner;
		tinfo.srcent		= chain->owner;
		
		tinfo.altangle		= Dir;
		tinfo.altpos		= end;
		
		playerHook_t *hook = AI_GetPlayerHook( self );
		//Two handed
		if( hook->cur_sequence && strstr( hook->cur_sequence->animation_name, "ataka" ) )
		{
		
			tinfo.modelindex	= gstate->ModelIndex("models/global/we_mflash.dkm");
			tinfo.modelindex2	= gstate->ModelIndex("models/global/we_mflash.dkm");

			tinfo.scale			= 1.05f;
			tinfo.scale2		= 1.05f;
			
//			sprintf(tinfo.HardPoint_Name,"hr_muzzle1");
//			sprintf(tinfo.HardPoint_Name2,"hr_muzzle2");
			Com_sprintf(tinfo.HardPoint_Name,sizeof(tinfo.HardPoint_Name),"hr_muzzle1");
			Com_sprintf(tinfo.HardPoint_Name2,sizeof(tinfo.HardPoint_Name2),"hr_muzzle2");
		
			tinfo.flags = TEF_FXFLAGS|TEF_SRCINDEX|TEF_ALTANGLE|TEF_ALTPOS|TEF_MODELINDEX|TEF_MODELINDEX2|TEF_SCALE|TEF_SCALE2|TEF_HARDPOINT|TEF_HARDPOINT2;
			tinfo.fxflags = TEF_TRACER|TEF_MODEL2|TEF_MODEL|TEF_ATTACHMODEL|TEF_FX_ONLY;
		}
		else   // One handed (left hand)
		{
			tinfo.modelindex	= gstate->ModelIndex("models/global/we_mflash.dkm");

			tinfo.scale			= 1.05f;

//			sprintf(tinfo.HardPoint_Name,"hr_muzzle2");
			Com_sprintf(tinfo.HardPoint_Name,sizeof(tinfo.HardPoint_Name),"hr_muzzle2");
			tinfo.flags = TEF_FXFLAGS|TEF_SRCINDEX|TEF_ALTANGLE|TEF_ALTPOS|TEF_MODELINDEX|TEF_SCALE|TEF_HARDPOINT;
			tinfo.fxflags = TEF_TRACER|TEF_MODEL2|TEF_MODEL|TEF_ATTACHMODEL|TEF_FX_ONLY;
		
		}
		// Makes sure we are alpha'd
		tinfo.renderfx = SPR_ALPHACHANNEL;
		//decrease the amount of time for this function since we could have 2 at a time.
		self->delay = 2.0f;
	}
	else
	{
		tinfo.ent=chain;
		tinfo.srcent=chain;
		tinfo.fru.Zero();
		tinfo.altangle = Dir;
		tinfo.altpos = end;


		tinfo.flags = TEF_FXFLAGS|TEF_SRCINDEX|TEF_ALTANGLE|TEF_ALTPOS;
		tinfo.fxflags = TEF_TRACER;
	}
	com->trackEntity(&tinfo,MULTICAST_PVS);
}






void flash_think( userEntity_t *self )
{
	if ( !self->owner || !self->owner->enemy )
	{
		//remove the tracked entities
		RELIABLE_UNTRACK(self);
		gstate->RemoveEntity(self);
		return;
	}

	playerHook_t *hook = AI_GetPlayerHook( self->owner );
//	ai_weapon_t *weapon = (ai_weapon_t *) self->owner->curWeapon;// SCG[1/23/00]: not used

	self->delay++;

	if(self->s.alpha == 0.0f)
		self->s.alpha = 1.0f;
	else
		self->s.alpha = 0.0f;

	self->nextthink = gstate->time + 0.01f;

	if( hook->cur_sequence )
	{
		if( strstr( hook->cur_sequence->animation_name, "ataka" ) )
		{
			// set invisible before frame X, remove after.
			if( self->owner->s.frame < 79 )
				self->s.alpha = 0.0f;
			if( self->owner->s.frame > 94 )
			{
				RELIABLE_UNTRACK(self);
				gstate->RemoveEntity(self);
			}

		}
		if( strstr( hook->cur_sequence->animation_name, "atakb" ) )
		{
			// set invisible before frame X, remove after.
			if( self->owner->s.frame < 119 )
				self->s.alpha = 0.0f;
			if( self->owner->s.frame > 134 )
			{
				RELIABLE_UNTRACK(self);
				gstate->RemoveEntity(self);
			}
		}
	}
	else
		gstate->RemoveEntity(self);
}

userEntity_t *DoFlash( userEntity_t *self, CVector& vec, ai_weapon_t *weapon )
{
	userEntity_t *uzi;
	uzi = gstate->SpawnEntity();
	uzi->movetype	= MOVETYPE_NONE;
	uzi->solid		= SOLID_NOT;
	uzi->owner	= self;
	uzi->s.frame	= 0;
	uzi->curWeapon = self->curWeapon;
	
	uzi->s.modelindex = gstate->ModelIndex ("models/global/we_mflash.dkm");
	
	uzi->s.render_scale.Set( 1.0f, 1.0f, 1.0f);
	gstate->SetSize(uzi, -5, -5, -10, 5, 5, 10);
	
	uzi->s.alpha = 0.0f;
	
	uzi->s.renderfx |= RF_TRACKENT | RF_TRANSLUCENT;
	
	uzi->health = weapon->base_damage;
	
	//uzi->think = uzigun_think;
	
	uzi->nextthink = gstate->time + 0.01f;
	
	uzi->delay = 0;
	
	uzi->enemy  = self->enemy;
	
	uzi->clipmask = MASK_SHOT;
    uzi->svflags  = SVF_SHOT;         //	won't clip against any ents with clipmask == MASK_SHOT

	AI_Adjust_Offset( uzi, &vec );

	uzi->s.angles = uzi->owner->s.angles;

	AddTrackLight(uzi,120,0.60f,0.40f,0.40f,TEF_LIGHT);

	return uzi;
}

// ************************************ UziGun ***************************************
// ----------------------------------------------------------------------------
//
// Name:		uzigun_think
// Description:
// Input:
// Output:
// Note:
//	self->owner is the entity that is shooting
//
// ----------------------------------------------------------------------------
void uzigun_think( userEntity_t *self )
{	
	if ( !self->owner || !self->owner->enemy )
	{
		//remove the tracked entities
		RELIABLE_UNTRACK(self);
		gstate->RemoveEntity(self);
		return;
	}

//	playerHook_t *hook = AI_GetPlayerHook( self->owner );// SCG[1/23/00]: not used
	ai_weapon_t *weapon = (ai_weapon_t *) self->owner->curWeapon;

	userEntity_t *temp = self->owner;

	ai_fire_bullet(self->owner, self->owner->enemy, weapon);

	self->delay++;

	if(self->s.alpha == 0.0f)
		self->s.alpha = 1.0f;
	else
		self->s.alpha = 0.0f;
	
	self->nextthink = gstate->time + 0.01f;

	if( self->owner->s.frame >= 80 )
		gstate->RemoveEntity(self);
}




// ----------------------------------------------------------------------------
//
// Name:		pre_uzigun_think
// Description:the pre-process think for the uzigun
// Input:
// Output:
// Note:
//	this function really only sets the origin offset for the shot
//
// ----------------------------------------------------------------------------
void pre_uzigun_think(userEntity_t *self)
{
//	uzigun_think(self,20,-4,27);
	if(self->owner)
		self->s.angles = self->owner->s.angles;
}




// ----------------------------------------------------------------------------
//
// Name:		uzigun_fire
// Description:
// Input:
// Output:
// Note:
//	fires the uzigunner's uzigun
//	which fires several shots one after another
//
// ----------------------------------------------------------------------------
void uzigun_fire( userInventory_s *ptr, userEntity_t *self )
{

	ai_weapon_t *weapon = (ai_weapon_t *) self->curWeapon;

	userEntity_t *uzi;
	uzi = gstate->SpawnEntity();
	uzi->movetype	= MOVETYPE_NONE;
	uzi->solid		= SOLID_NOT;
	uzi->owner	= self;
	uzi->s.frame	= 0;
	uzi->curWeapon = self->curWeapon;
	
	uzi->s.modelindex = gstate->ModelIndex ("models/global/we_mflash.dkm");
	
	uzi->s.render_scale.Set(1.0f, 2.0f, 2.0f);
	gstate->SetSize(uzi, -5, -5, -10, 5, 5, 10);
	
	uzi->s.alpha = 1.0f;
	
	uzi->s.renderfx |= RF_TRACKENT | RF_TRANSLUCENT;
	
	uzi->health = (float)weapon->base_damage + ((float)weapon->rnd_damage * rnd());
	
	uzi->think = uzigun_think;
	
	uzi->nextthink = gstate->time + 0.01f;
	
	uzi->delay = 0;
	
	uzi->enemy  = self->enemy;
	
	uzi->clipmask = MASK_SHOT;
    uzi->svflags  = SVF_SHOT;         //	won't clip against any ents with clipmask == MASK_SHOT

	CVector Offset;

	Offset.x = 20;
	Offset.y = -6;
	Offset.z = 25;

	AI_Adjust_Offset( uzi, &Offset);

	uzi->s.angles = uzi->owner->s.angles;

	gstate->LinkEntity(uzi);

	AddTrackLight(uzi,120,0.60f,0.40f,0.40f,TEF_LIGHT);
}

/* ************************************ boargun ************************************** */

// ----------------------------------------------------------------------------
//
// Name:		boargun_think
// Description:
// Input:
// Output:
// Note:
//	a thinkFunc for boargun firing
//	self is a fake entity
//	self->owner is the entity that is shooting
//
// ----------------------------------------------------------------------------
void boargun_think( userEntity_t *self )
{
	if ( !self->owner || !self->owner->enemy )
	{
		gstate->RemoveEntity(self);
		return;
	}

//	playerHook_t *hook = AI_GetPlayerHook( self->owner );// SCG[1/23/00]: not used
	ai_weapon_t *weapon = (ai_weapon_t *) self->owner->curWeapon;

	userEntity_t *temp = self->owner;

	self->delay++;

	if(self->s.alpha == 0.0f)
		self->s.alpha = 1.0f;
	else
		self->s.alpha = 0.0f;
	
	self->nextthink = gstate->time + 0.05f;

	if( self->owner->s.frame > 91 )
		self->s.renderfx = RF_TRACKENT | RF_TRANSLUCENT;

	if( self->owner->s.frame >= 97 )
		gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//
// Name:		boargun_fire
// Description:
// Input:
// Output:
// Note:
//	fires the battle boar's machine gun
//	which fires 2 machine guns, several shots one after another
//
// ----------------------------------------------------------------------------
void boargun_fire( userInventory_s *ptr, userEntity_t *self )
{
	ai_weapon_t *weapon = (ai_weapon_t *) self->curWeapon;

	ai_fire_bullet(self, self->enemy, weapon);

	userEntity_t *boargun;
	boargun = gstate->SpawnEntity();
	boargun->movetype	= MOVETYPE_NONE;
	boargun->solid		= SOLID_NOT;
	boargun->owner	= self;
	boargun->s.frame	= 0;
	boargun->curWeapon = self->curWeapon;
	boargun->s.modelindex = gstate->ModelIndex ("models/global/we_mflash.dkm");
	boargun->s.render_scale.Set(1.0f, 2.0f, 2.0f);
	gstate->SetSize(boargun, -5, -5, -10, 5, 5, 10);
	boargun->s.alpha = 1.0f;
	boargun->s.renderfx |= RF_TRACKENT | RF_TRANSLUCENT | RF_NODRAW;
	boargun->health = (float)weapon->base_damage + ((float)weapon->rnd_damage * rnd());
	boargun->think = boargun_think;
	boargun->nextthink = gstate->time + 0.01f;
	boargun->delay = 0;
	boargun->enemy  = self->enemy;
	boargun->clipmask = MASK_SHOT;
    boargun->svflags  = SVF_SHOT;         //	won't clip against any ents with clipmask == MASK_SHOT

	CVector Offset;
	Offset.x = 15;
	Offset.y = -9; // X plr relative
	Offset.z = 14; // Y plr relative

	AI_Adjust_Offset( boargun, &Offset);
	boargun->s.angles = boargun->owner->s.angles;
	gstate->LinkEntity(boargun);

	userEntity_t *boargun2;
	boargun2 = gstate->SpawnEntity();
	boargun2->movetype	= MOVETYPE_NONE;
	boargun2->solid		= SOLID_NOT;
	boargun2->owner		= self;
	boargun2->s.frame	= 0;
	boargun2->curWeapon = self->curWeapon;
	boargun2->s.modelindex = gstate->ModelIndex ("models/global/we_mflash.dkm");
	boargun2->s.render_scale.Set(1.0f, 2.0f, 2.0f);
	gstate->SetSize(boargun2, -5, -5, -10, 5, 5, 10);
	boargun2->s.alpha = 1.0f;
	boargun2->s.renderfx |= RF_TRACKENT | RF_TRANSLUCENT | RF_NODRAW;
	boargun2->health = weapon->base_damage  + (weapon->rnd_damage * rnd());
	boargun2->think = boargun_think;
	boargun2->nextthink = gstate->time + 0.01f;
	boargun2->delay = 0;
	boargun2->enemy  = self->enemy;
	boargun2->clipmask = MASK_SHOT;
    boargun2->svflags  = SVF_SHOT;         //	won't clip against any ents with clipmask == MASK_SHOT

	CVector Offset2;
	Offset2.x = 15;
	Offset2.y = 9;  // X plr relative
	Offset2.z = 14; // Y plr relative

	AI_Adjust_Offset( boargun2, &Offset2);
	boargun2->s.angles = boargun2->owner->s.angles;
	gstate->LinkEntity(boargun2);

	AddTrackLight(boargun,120,0.60f,0.40f,0.40f,TEF_LIGHT);
	AddTrackLight(boargun2,120,0.60f,0.40f,0.40f,TEF_LIGHT);
}

/* ************************************ poison bite ************************************** */

// ----------------------------------------------------------------------------
//
// Name:		poison_bite_mild
// Description:
// Input:
// Output:
// Note:
//	"fires" a standard melee attack
//	which reaches hook->attack_dist
//
//	only plays hook->sound2 if target is hit
//
//	hook->sound1 can be used in the monster's code to
//	play a swinging sound
//
//	if target was hit, it is poisoned with a mild poison:
//	60 seconds, 1 hit every 3 seconds
//
// ----------------------------------------------------------------------------
void poison_bite_mild( userInventory_s *ptr, userEntity_t *self )
{
	playerHook_t *hook = AI_GetPlayerHook( self );
	ai_weapon_t	*weapon = (ai_weapon_t *) ptr;
	userEntity_t *target;
	float dist = hook->attack_dist;

	if ( dist > 64.0 )
	{
		dist = 64.0;
	}

	target = ai_trace_attack( self, weapon );
	if ( target )
	{
		com->Poison( target, 1.0, 15.0, 3.0 );
		if ( hook->sound2 && !AI_IsSoundDisabled() )
		{
			gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex(hook->sound2),
                                      1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
		}
	}
}

/* ************************************ dart ************************************** */

// ----------------------------------------------------------------------------
//
// Name:		dart_touch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void dart_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	com->Damage(other, self, self->owner, self->s.origin, zero_vector, self->health, DAMAGE_INERTIAL);
	com->Poison(other, 1.0, 60.0, 3.0);

	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//
// Name:		dart_fire_think
// Description:
// Input:
// Output:
// Note:
//	a for dart firing
//
// ----------------------------------------------------------------------------
void dart_fire_think( userEntity_t *self )
{										 
//	playerHook_t *hook = AI_GetPlayerHook( self->owner );// SCG[1/23/00]: not used
	ai_weapon_t	*weapon = (ai_weapon_t *) self->owner->curWeapon;
	userEntity_t *dart;

	dart = ai_fire_projectile( self->owner, self->owner->enemy, weapon, NULL, /*"models/dart.mdl",*/
	                           dart_touch, 0, NULL );
	dart->think = NULL;
	dart->nextthink = -1;
	dart->health = (float) weapon->base_damage * rnd() + (float) weapon->rnd_damage;

	self->delay++;

	if ( self->delay >= (self->s.frameInfo.endFrame - self->s.frameInfo.startFrame) / 4 )
	{
		thinkFunc_remove(self);
		return;
	}
	else
	{
		thinkFunc_set_think( self, dart_fire_think, 0.1 );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		dart_fire
// Description:
// Input:
// Output:
// Note:
//	fires multiple poison darts
//
// ----------------------------------------------------------------------------
void dart_fire( userInventory_s *ptr, userEntity_t *self )
{
//	ai_weapon_t *weapon = (ai_weapon_t *) ptr;// SCG[1/23/00]: not used

	userEntity_t *think_ent = thinkFunc_add( self, dart_fire_think, 0.1 );
	
	//	use self->delay as a counter for the chaingun
	think_ent->delay = 0;

	dart_fire_think( think_ent );
}

// ----------------------------------------------------------------------------
//
// Name:		dart_strafe_fire_think
// Description:
// Input:
// Output:
// Note:
//	a thinkFunc for dart firing
//
// ----------------------------------------------------------------------------
void dart_strafe_fire_think( userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self->owner );// SCG[1/23/00]: not used
	ai_weapon_t	*weapon = (ai_weapon_t *) self->owner->curWeapon;

	float yaw = self->owner->s.angles.yaw;
	self->owner->s.angles.yaw = (self->owner->s.angles.yaw - 12) + (self->delay * 6);

	userEntity_t *dart = ai_fire_projectile( self->owner, self->owner->enemy, weapon, NULL/*"models/dart.mdl"*/,
	                                         dart_touch, 0, NULL );
	dart->think = NULL;
	dart->nextthink = -1;
	dart->health = (float) weapon->base_damage * rnd() + (float) weapon->rnd_damage;

	self->owner->s.angles.yaw = yaw;
	self->delay++;

	if ( self->delay >= (self->s.frameInfo.endFrame - self->s.frameInfo.startFrame) / 4 )
	{
		thinkFunc_remove( self );
		return;
	}
	else
	{
		thinkFunc_set_think( self, dart_strafe_fire_think, 0.1 );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		dart_strafe_fire
// Description:
// Input:
// Output:
// Note:
//	fires multiple poison darts
//
// ----------------------------------------------------------------------------
void dart_strafe_fire( userInventory_s *ptr, userEntity_t *self )
{
//	ai_weapon_t *weapon = (ai_weapon_t *) ptr;// SCG[1/23/00]: not used

	userEntity_t *think_ent = thinkFunc_add( self, dart_strafe_fire_think, 0.1 );
	
	//	use self->delay as a counter for the chaingun
	think_ent->delay = 0;

	dart_strafe_fire_think( think_ent );
}

// ************************************ cryo spray ************************************** 

// ----------------------------------------------------------------------------
//
// Name:		cryo_touch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void cryo_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	com->Damage(other, self, self->owner, self->s.origin, zero_vector, self->health, DAMAGE_INERTIAL);
	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//
// Name:		cryo_think
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void cryo_think( userEntity_t *self )
{
	if ( self->delay <= gstate->time )
	{
		gstate->RemoveEntity(self);
	}
	else
	{
		self->nextthink = gstate->time + 0.1;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// Name:		cryo_spray
// Description:
//  sprays a stream of particles forward from the "muzzle" of the ai's
//	weapons
//
///////////////////////////////////////////////////////////////////////////////
void cryo_spray( userInventory_s* inv, userEntity_t* self )
{
	CVector org, ang;

	// get the weapon origin
	ai_weapon_t* weapon = (ai_weapon_t*)inv;
	weapon->speed = 250.0f;

	AIMDATA_PTR pAimData = ai_aim_curWeapon(self, weapon);

	//	calculate angle to target from actual firing position of projectile
	ang = self->s.angles;
	ang.AngleToVectors(forward, right, up);
	org = self->s.origin + weapon->ofs.x * right + weapon->ofs.y * forward + weapon->ofs.z * up;

	// send down particle spray
	gstate->WriteByte(SVC_TEMP_ENTITY);
	gstate->WriteByte(TE_CRYO_SPRAY);
	gstate->WritePosition(org);
// SCG[10/26/99]: monsters do not get a client struct
//	gstate->WritePosition(self->client->v_angle);				// send as a position to keep the angle
	gstate->WritePosition(self->s.angles);				// send as a position to keep the angle
	gstate->MultiCast(self->s.origin,MULTICAST_PVS);

	// fire the model with the touch function
	userEntity_t* model = gstate->SpawnEntity ();
	model->s.render_scale.Set(1.0f,1.0f,1.0f);
	model->s.frame		= 0;
	model->s.modelindex = gstate->ModelIndex ("models/global/w_tribullet.dkm");
	model->s.angles		= ang;
	model->s.alpha		= 0.0f;
	model->s.renderfx	|= RF_TRANSLUCENT;
	model->movetype		= MOVETYPE_FLYMISSILE;
	model->solid		= SOLID_BBOX;
	model->owner		= self;
	model->clipmask		= MASK_SHOT;
	model->velocity		= pAimData->dir * weapon->speed;
	model->health		= weapon->base_damage  + (weapon->rnd_damage * rnd());
	model->delay		= gstate->time + 0.8f;
	model->think		= cryo_think;
	model->touch		= cryo_touch;
	model->nextthink	= gstate->time + gstate->frametime;
	gstate->SetOrigin2(model,pAimData->org);
}



/* ************************************ rocket ************************************** */

// ----------------------------------------------------------------------------
//
// Name:		rocket_explode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void rocket_explode( userEntity_t *self,  cplane_t *plane, userEntity_t *other )
{
   //Need to do something about this.
	gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/e_explodeb.wav"), 
                              1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );

	//Make the explosion
    CVector color(.85,.35,.15);
    spawnPolyExplosion(self->s.origin, plane->normal, 1, 250, color, PEF_NORMAL|PEF_SOUND);
	spawn_sprite_explosion( self, self->s.origin, true );

	com->RadiusDamage( self, self->owner, self->owner, self->health, DAMAGE_EXPLOSION, 128.0 );

	RELIABLE_UNTRACK(self);
	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//
// Name:		rocket_think
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void rocket_think( userEntity_t *self )
{
	rocketHook_t * rhook = (rocketHook_t*) self->userHook;
	
	self->s.frame++;
	if ( self->s.frame > 2 )
	{
		self->s.frame = 0;
	}

	// the boar rocket attack doesn't make a hook...
	if (!rhook)
	{
		RELIABLE_UNTRACK(self);
		gstate->RemoveEntity(self);
		return;
	}

	if ( rhook->time_to_live <= gstate->time )
	{
		RELIABLE_UNTRACK(self);
		gstate->RemoveEntity(self);
	}
	else
	{
		self->nextthink = gstate->time + 0.1;
	}
	if(rhook->a_counter > 1.0)
	{
		rhook->a_counter--;
		self->velocity.Normalize();
		self->velocity =  self->velocity * (rhook->r_speed / rhook->a_counter);
	}
	
}

// ----------------------------------------------------------------------------
//
// Name:		rocket_touch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void rocket_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	if(self->owner != other->owner || other != self->owner)
	{
		rocket_explode( self, plane, other );
	}
}

// ----------------------------------------------------------------------------
//
// Name:		rocket_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void rocket_attack( userInventory_s *ptr, userEntity_t *self )
{
	CVector Offset, Target_Angles, Forward;
	ai_weapon_t	*weapon = (ai_weapon_t *) ptr;
	userEntity_t *rocket = ai_fire_projectile( self, self->enemy, weapon, "models/global/we_flarered.sp2",
	                                          rocket_touch, 0, NULL);

	rocket->userHook = gstate->X_Malloc(sizeof(rocketHook_t),MEM_TAG_HOOK);

	// SCG[11/24/99]: Save game stuff
	rocket->save = rocket_hook_save;
	rocket->load = rocket_hook_load;

	rocketHook_t * rhook = (rocketHook_t*) rocket->userHook;
	rhook->a_counter = 7.0f;
	rocket->s.render_scale.Set(0.01, 0.01, 0.01);

	rocket->health = (float)weapon->base_damage + ((float)weapon->rnd_damage * rnd());
	rocket->touch = rocket_touch;
	
	rhook->r_speed = weapon->speed;

	rocket->velocity.Normalize();
	
	rocket->velocity =  rocket->velocity * (rhook->r_speed / rhook->a_counter);

	rhook->time_to_live = gstate->time + 4.0f;

	rocket->think = rocket_think;
	rocket->nextthink = gstate->time + 0.01f;

	
	//This is to get a ruff estimate for the offset
	//THIS IS NOT HOW WE DRAW THE MODEL THAT IS DONE
	//THROUGH THE TRACK ENTITY.  IF THE MODEL DOES NOT
	//HAVE A HARDPOINT THEN IT WILL BE CENTERED OFF OF
	//THIS ORIGIN. IF IT DOES HAVE A HARDPOINT THEN THERE
	//WILL BE A CALCULATION BASED OFF OF THIS ORIGIN AND THE
	//HARDPOINT.
	Offset = self->enemy->s.origin - self->s.origin;
	Offset.Normalize();

	VectorToAngles(Offset,Target_Angles);

	Target_Angles.yaw -= 55;
	Target_Angles.pitch -= 65;

	Target_Angles.AngleToForwardVector(Forward);

	Target_Angles.yaw -= 115;
	Target_Angles.pitch += 25;

	Target_Angles.AngleToForwardVector(Offset);

	rocket->s.origin = (Forward * 25)+self->s.origin;


	// Smoke FX
	CVector color;
	color.Set( 0.65, 0.65, 0.65 );
	Offset = (Offset*45)+ self->s.origin;
	
	gstate->particle (TE_PARTICLE_RISE, Offset, up, 1, color,15, PARTICLE_SMOKE|PARTICLE_RANDOM_SPREAD);
	
	
	// link rocket to the world
	gstate->LinkEntity( rocket );

	Offset = self->enemy->s.origin - rocket->s.origin;
	Offset.Normalize();
	//VectorToAngles(Offset,Target_Angles);
	//rocket->s.angles = Target_Angles;

	
	//Attach the Sprite Light&Glow stuff to the rocket(add track entity)
	//This will also actually draw the model relative from the hard point.
    trackInfo_t tinfo;
    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=rocket;
    tinfo.srcent=rocket;
    tinfo.fru.Zero();
    tinfo.lightColor.x = 0.75;//R
    tinfo.lightColor.y = 0.45;//G
    tinfo.lightColor.z = 0.15;//B
    tinfo.lightSize= 145;
	
	tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_MODELINDEX|TEF_SCALE|TEF_EXTRAINDEX|TEF_MODELINDEX2|TEF_SCALE2|TEF_HARDPOINT2;
    tinfo.fxflags = TEF_LIGHT|TEF_SPRITE|TEF_MODEL|TEF_SMOKETRAIL;
	tinfo.renderfx = SPR_ALPHACHANNEL;
	tinfo.scale = 1.45f;
	tinfo.modelindex = gstate->ModelIndex ("models/global/e_sflorange.sp2");

	tinfo.modelindex2 = gstate->ModelIndex ("models/e4/me_missile.dkm");  //model to show for TEF_MODEL
	tinfo.scale2 = 2.00f;												//Scale for TEF_MODEL
	
//	sprintf(tinfo.HardPoint_Name2,"hr_muzzle");							//set the hardpoint name
	Com_sprintf(tinfo.HardPoint_Name2,sizeof(tinfo.HardPoint_Name2),"hr_muzzle");							//set the hardpoint name
	
	tinfo.extra = self;

	com->trackEntity(&tinfo,MULTICAST_ALL);

    gstate->StartEntitySound( rocket, CHAN_AUTO, gstate->SoundIndex("e4/m_rockgangataka.wav"), 
                              1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
}




// ----------------------------------------------------------------------------
//
// Name:		boar_rocket_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void boar_rocket_attack( userInventory_s *ptr, userEntity_t *self )
{
	CVector Offset, Target_Angles, Forward;
	ai_weapon_t	*weapon = (ai_weapon_t *) ptr;

	userEntity_t *rocket = ai_fire_projectile( self, self->enemy, weapon, "models/global/we_flarered.sp2",
	                                          rocket_touch, 0, NULL);

	rocket->s.render_scale.Set(0.01, 0.01, 0.01);

	rocket->health = weapon->base_damage  + (weapon->rnd_damage * rnd());
	rocket->touch = rocket_touch;

	rocket->delay = gstate->time + 5.0f;

	rocket->think = rocket_think;
	rocket->nextthink = gstate->time + 5.0f;

	//This is to get a ruff estimate for the offset
	//THIS IS NOT HOW WE DRAW THE MODEL THAT IS DONE
	//THROUGH THE TRACK ENTITY.  IF THE MODEL DOES NOT
	//HAVE A HARDPOINT THEN IT WILL BE CENTERED OFF OF
	//THIS ORIGIN. IF IT DOES HAVE A HARDPOINT THEN THERE
	//WILL BE A CALCULATION BASED OFF OF THIS ORIGIN AND THE
	//HARDPOINT.
	
	rocket->s.origin.z += 15.0f;

	// link rocket to the world
	gstate->LinkEntity( rocket );

	//Attach the Sprite Light&Glow stuff to the rocket(add track entity)
	//This will also actually draw the model relative from the hard point.
    trackInfo_t tinfo;
    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=rocket;
    tinfo.srcent=rocket;
    tinfo.fru.Zero();
    tinfo.lightColor.x = 0.75;//R
    tinfo.lightColor.y = 0.15;//G
    tinfo.lightColor.z = 0.15;//B
    tinfo.lightSize= 115;
	
	tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_MODELINDEX|TEF_SCALE|TEF_EXTRAINDEX|TEF_MODELINDEX2|TEF_SCALE2|TEF_HARDPOINT2;
    tinfo.fxflags = TEF_LIGHT|TEF_SPRITE|TEF_MODEL|TEF_SMOKETRAIL;
	tinfo.renderfx = SPR_ALPHACHANNEL;
	tinfo.scale = 0.75f;
	tinfo.modelindex = gstate->ModelIndex ("models/global/e_sflred.sp2");

	tinfo.modelindex2 = gstate->ModelIndex ("models/e1/we_swrocket.dkm");  //model to show for TEF_MODEL
	tinfo.scale2 = 0.65f;													//Scale for TEF_MODEL
	
//	sprintf(tinfo.HardPoint_Name2,"hr_muzzle3");							//set the hardpoint name
	Com_sprintf(tinfo.HardPoint_Name2,sizeof(tinfo.HardPoint_Name2),"hr_muzzle3");							//set the hardpoint name
	
	tinfo.extra = self;

	com->trackEntity(&tinfo,MULTICAST_ALL);

    gstate->StartEntitySound( rocket, CHAN_AUTO, gstate->SoundIndex("global/e_firetravelb.wav"), 
                              1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
}

// ----------------------------------------------------------------------------
// NSS[1/21/00]:
// Name:		mp_rocket_attack
// Description: The reduced rocket attack(2 functions combined into really 1)
// Input:userInventory_s *ptr, userEntity_t *self, CVector Offset, char *buffer
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void mp_rocket_attack(userInventory_s *ptr, userEntity_t *self, CVector Offset, char *hardpoint)
{

	CVector Target_Angles, Forward;
	ai_weapon_t	*weapon = (ai_weapon_t *) ptr;
	
	userEntity_t *rocket	= ai_fire_projectile( self, self->enemy, weapon, "models/global/e_flare4x.sp2",rocket_touch, 0, NULL);
	rocket->userHook		= gstate->X_Malloc(sizeof(rocketHook_t),MEM_TAG_HOOK);

	rocketHook_t *rhook		= (rocketHook_t *)rocket->userHook;


	// SCG[11/24/99]: Save game stuff
	rocket->save			= rocket_hook_save;
	rocket->load			= rocket_hook_load;

	
	rocket->velocity.Normalize();
	rhook->r_speed			= weapon->speed;
	rhook->a_counter		= 4.0f;
	rocket->velocity		= rocket->velocity * (rhook->r_speed / rhook->a_counter);
	rocket->health			= weapon->base_damage  + (weapon->rnd_damage * rnd());
	rocket->touch			= rocket_touch;
	rhook->time_to_live		= gstate->time + 4.0f;
	rocket->think			= rocket_think;
	rocket->nextthink		= gstate->time + 0.01f;
	rocket->s.alpha			= 0.10f;

	rocket->s.render_scale.Set(0.02f,0.02f,0.02f);

	
	AI_Adjust_Offset( rocket, &Offset);

	// link rocket to the world
	gstate->LinkEntity( rocket );


	// Smoke FX
	CVector color;
	color.Set( 0.65, 0.65, 0.65 );
	gstate->particle (TE_PARTICLE_RISE, rocket->s.origin, up, 1, color,15, PARTICLE_SMOKE|PARTICLE_RANDOM_SPREAD);

    trackInfo_t tinfo;
    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=rocket;
    tinfo.srcent=rocket;
    tinfo.fru.Zero();
    tinfo.lightColor.x	= 0.75;//R
    tinfo.lightColor.y	= 0.45;//G
    tinfo.lightColor.z	= 0.15;//B
    tinfo.lightSize		= 145;
	
	tinfo.flags			= TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_MODELINDEX|TEF_SCALE|TEF_EXTRAINDEX|TEF_MODELINDEX2|TEF_SCALE2|TEF_HARDPOINT2;
    tinfo.fxflags		= TEF_LIGHT|TEF_SPRITE|TEF_MODEL|TEF_SMOKETRAIL;
	tinfo.renderfx		= SPR_ALPHACHANNEL;
	tinfo.scale			= 1.0f;
	tinfo.modelindex	= gstate->ModelIndex ("models/global/e_sflorange.sp2");
	tinfo.modelindex2	= gstate->ModelIndex ("models/e4/me_missile.dkm");		//model to show for TEF_MODEL
	tinfo.scale2		= 0.75f;														//Scale for TEF_MODEL
	
	Com_sprintf(tinfo.HardPoint_Name2,sizeof(tinfo.HardPoint_Name2),hardpoint);							//set the hardpoint name
	
	tinfo.extra = self;

	com->trackEntity(&tinfo,MULTICAST_ALL);






    gstate->StartEntitySound( rocket, CHAN_AUTO, gstate->SoundIndex("e4/m_rockgangataka.wav"), 
                              1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );

}


// ----------------------------------------------------------------------------
// NSS[1/22/00]:
// Name:		mp_rocket_left_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void mp_rocket_left_attack( userInventory_s *ptr, userEntity_t *self )
{
	CVector Offset;
	char Buffer[32];
	Offset.x = 0;	// Z Plr Relative
	Offset.y = 8;	// X Plr Relative
	Offset.z = 30;	// Y Plr Relative
	Com_sprintf(Buffer,sizeof(Buffer),"hr_muzzle1");
	mp_rocket_attack( ptr, self, Offset, Buffer);
}

// ----------------------------------------------------------------------------
// NSS[1/22/00]:
// Name:		mp_rocket_right_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void mp_rocket_right_attack( userInventory_s *ptr, userEntity_t *self )
{
	CVector Offset;
	char Buffer[32];
	Offset.x = 0;	// Z Plr Relative
	Offset.y = -12;	// X Plr Relative
	Offset.z = 30;	// Y Plr Relative
	Com_sprintf(Buffer,sizeof(Buffer),"hr_muzzle1");
	mp_rocket_attack( ptr, self, Offset, Buffer);

}

// ----------------------------------------------------------------------------
//
// Name:		vermin_rocket_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void vermin_rocket_attack( userInventory_s *ptr, userEntity_t *self )
{
	CVector Offset, Target_Angles, Forward;
	ai_weapon_t	*weapon = (ai_weapon_t *) ptr;
	userEntity_t *rocket = ai_fire_projectile( self, self->enemy, weapon, "models/e4/me_missile.dkm",
		rocket_touch, 0, NULL);

	rocket->userHook = gstate->X_Malloc(sizeof(rocketHook_t),MEM_TAG_HOOK);

	// SCG[11/24/99]: Save game stuff
	rocket->save = rocket_hook_save;
	rocket->load = rocket_hook_load;

	rocketHook_t * rhook = (rocketHook_t*) rocket->userHook;
	rhook->a_counter = 7.0f;
	rocket->s.render_scale.Set(1, 1, 1);

	rocket->health = (float)weapon->base_damage + ((float)weapon->rnd_damage * rnd());
	rocket->touch = rocket_touch;
	
	rhook->r_speed = weapon->speed;

	rocket->velocity.Normalize();
	
	rocket->velocity =  rocket->velocity * (rhook->r_speed / rhook->a_counter);

	rhook->time_to_live = gstate->time + 4.0f;

	rocket->think = rocket_think;
	rocket->nextthink = gstate->time + 0.01f;

	Offset.x = 0;	// Z Plr Relative
	Offset.y = 0;	// X Plr Relative
	Offset.z = 20;	// Y Plr Relative

	AI_Adjust_Offset( rocket, &Offset);

	// link rocket to the world
	gstate->LinkEntity( rocket );

	// Smoke FX
	CVector color;
	color.Set( 0.65, 0.65, 0.65 );
	
	gstate->particle (TE_PARTICLE_RISE, Offset, up, 1, color,15, PARTICLE_SMOKE|PARTICLE_RANDOM_SPREAD);

    trackInfo_t tinfo;
    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=rocket;
    tinfo.srcent=rocket;
    tinfo.fru.Zero();
    tinfo.lightColor.x = 0.75;//R
    tinfo.lightColor.y = 0.45;//G
    tinfo.lightColor.z = 0.15;//B
    tinfo.lightSize= 145;
	
	tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_MODELINDEX|TEF_SCALE|TEF_EXTRAINDEX|TEF_MODELINDEX2|TEF_SCALE2|TEF_HARDPOINT2;
    tinfo.fxflags = TEF_LIGHT|TEF_SPRITE|/*TEF_MODEL|*/TEF_SMOKETRAIL;
	tinfo.renderfx = SPR_ALPHACHANNEL;
	tinfo.scale = 1.45f;
	tinfo.modelindex = gstate->ModelIndex ("models/global/e_sflorange.sp2");
	
//	sprintf(tinfo.HardPoint_Name2,"hr_muzzle");							//set the hardpoint name
	Com_sprintf(tinfo.HardPoint_Name2,sizeof(tinfo.HardPoint_Name2),"hr_muzzle");							//set the hardpoint name
	
	tinfo.extra = self;

	com->trackEntity(&tinfo,MULTICAST_ALL);

    gstate->StartEntitySound( rocket, CHAN_AUTO, gstate->SoundIndex("e4/m_rockgangataka.wav"), 
                              1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
}

/* ********************************* magic arrow ************************************** */

// ----------------------------------------------------------------------------
//
// Name:		magic_arrow_think
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void magic_arrow_think( userEntity_t *self )
{
	//remove the tracked entities
	RELIABLE_UNTRACK(self);
	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//
// Name:		magic_arrow_touch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void magic_arrow_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	CVector vel = self->velocity;
	com->Damage( other, self, self->owner, self->s.origin, vel, self->health, DAMAGE_INERTIAL );

	if( !strcmp( "worldspawn", other->className ))
		gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("global/e_arrowimp.wav"), 
			1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);

	RELIABLE_UNTRACK(self);
	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//
// Name:		magic_arrow_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void magic_arrow_attack( userInventory_s *ptr, userEntity_t *self )
{
	ai_weapon_t	*weapon = (ai_weapon_t *) ptr;

	userEntity_t *arrow = ai_fire_projectile( self, self->enemy, weapon, "models/e3/we_bolt.dkm",
	                                          magic_arrow_touch, 0, NULL );
	arrow->s.effects = EF_BOLTTRAIL;
	arrow->s.renderfx |= (SPR_ALPHACHANNEL | RF_TRANSLUCENT);

	arrow->s.render_scale.Set(0.5, 0.5, 0.5);

	arrow->health = (float)weapon->base_damage + ((float)weapon->rnd_damage * rnd());

	arrow->delay = gstate->time + 3.0;

	arrow->think = magic_arrow_think;
	arrow->nextthink = gstate->time + 10.0;

	// link arrow to world
	gstate->LinkEntity( arrow );

	
	//Attach the Sprite Light&Glow stuff (add track entity)
    trackInfo_t tinfo;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=arrow;
    tinfo.srcent=arrow;
    tinfo.fru.Zero();
    tinfo.lightColor.x = 0.65;//R
    tinfo.lightColor.y = 0.35;//G
    tinfo.lightColor.z = 0.35;//B
    tinfo.lightSize= 175;
	
	tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_MODELINDEX;
    tinfo.fxflags = TEF_LIGHT|TEF_SPRITE;
	tinfo.renderfx = SPR_ALPHACHANNEL;
	tinfo.scale = 1.0f;
	tinfo.modelindex = gstate->ModelIndex ("models/global/we_flarered.sp2");

	com->trackEntity(&tinfo,MULTICAST_PVS);
}

/* ********************************* sludge ************************************** */

// ----------------------------------------------------------------------------
//
// Name:		sludge_think
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sludge_think( userEntity_t *self )
{
	//remove the tracked entities
	RELIABLE_UNTRACK(self);
	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//NSS[11/15/99]:
// Name:		sludge_touch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sludge_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	CVector vel = self->velocity;
	com->Damage( other, self, self->owner, self->s.origin, vel, self->health, DAMAGE_INERTIAL );
	// Smoke FX
	CVector color;
	color.Set( 0.35, 0.65, 0.15 );
	gstate->particle (TE_PARTICLE_RISE, self->s.origin, up, 1, color,15, PARTICLE_SMOKE|PARTICLE_RANDOM_SPREAD);
	
	self->lastAIFrame++;
	//Setup our inverted plane
	vel.Normalize();
	vel.Negate();
	
	self->avelocity = 360*vel;


	if( self->lastAIFrame > 1 && self->owner != other )
	{
		//remove the tracked entities
		RELIABLE_UNTRACK(self);
		gstate->RemoveEntity(self);
	}
}


// ----------------------------------------------------------------------------
//NSS[11/15/99]:
// Name:		sludge_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void sludge_attack( userInventory_s *ptr, userEntity_t *self )
{
	ai_weapon_t	*weapon = (ai_weapon_t *) ptr;

	userEntity_t *sludge = ai_fire_projectile( self, self->enemy, weapon, "models/e1/me_sludge.dkm",
	                                           sludge_touch, 0, NULL );

	sludge->movetype = MOVETYPE_BOUNCEMISSILE;
	
	sludge->s.render_scale.Set(0.85, 0.85, 0.85);

	sludge->health = (float)weapon->base_damage + ((float)weapon->rnd_damage * rnd());

	sludge->avelocity.Set(0.0f,0.0f,35.0f);

	sludge->delay = gstate->time + 3.0;

	sludge->think = sludge_think;
	sludge->nextthink = gstate->time + 3.0;
	sludge->lastAIFrame = 0;
	
	//Attach the Sprite Light&Glow stuff (add track entity)
    trackInfo_t tinfo;  

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=sludge;
    tinfo.srcent=sludge;
    tinfo.fru.Zero();
    tinfo.lightColor.x = 0.5;//R
    tinfo.lightColor.y = 1.0;//G
    tinfo.lightColor.z = 0.5;//B
    tinfo.lightSize= 300;
	
	tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_MODELINDEX|TEF_SCALE;
    tinfo.fxflags = TEF_LIGHT|TEF_SPRITE|TEF_SMOKETRAIL|TEF_SLUDGE_FX;
	tinfo.renderfx = SPR_ALPHACHANNEL;
	tinfo.scale = 2.0f;
	tinfo.modelindex = gstate->ModelIndex ("models/global/e_sflgreen.sp2");
	com->trackEntity(&tinfo,MULTICAST_PVS);
}

/* ******************************** laser ****************************** */

// ----------------------------------------------------------------------------
//NSS[11/15/99]:
// Name:		laser_touch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void laser_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
    if ( self->owner == other )
    {
	    return;
	}

    //Can we damage the object we hit?
    if ( other->takedamage != DAMAGE_NO )
    {
        // make sure we pass the bolt's OWNER as the attacker!
        // pass zero_vector as dir because DAMAGE_INFLICTOR_VEL will override it
		com->Damage( other, self, self->owner, self->s.origin, zero_vector, self->health, DAMAGE_INERTIAL );
    }
    else
    {
        CVector normal,angles;
        CVector color(.9,.3,.2);

        if ( plane )
        {
		    normal = plane->normal;
        }
		else
        {
            normal = self->velocity;
            normal.Normalize();
            normal = normal*-1;
        }

        // do sparks
        gstate->WriteByte(SVC_TEMP_ENTITY);
        gstate->WriteByte(TE_BOLTER_SPARKS);
        gstate->WriteByte(100);
        gstate->WritePosition(self->s.origin);
        gstate->WriteDir(normal);
        gstate->WriteDir(CVector(0.5f, 0.5f, 1.0f));
        gstate->MultiCast(self->s.origin,MULTICAST_PVS);

		gstate->StartEntitySound(self, CHAN_AUTO, gstate->SoundIndex("global/we_zapb.wav"), 
                                     1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	}
    
	// unhook flare
	self->userHook = NULL;

    // remove laser
	RELIABLE_UNTRACK(self);
	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//NSS[11/15/99]:
// Name:		laser_think
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void laser_think( userEntity_t *self )
{
	RELIABLE_UNTRACK(self);
	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
// NSS[1/26/00]:
// Name:		laser_fire_now
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void laser_fire_now( userInventory_s *ptr, userEntity_t *self, char *buf, float scale )
{
	ai_weapon_t *weapon = (ai_weapon_t *) ptr;

	// create the laser (fire straight unless firing with tracking but non-leading shots)
	userEntity_t *laser = ai_fire_projectile( self, self->enemy, weapon, "models/e1/me_mater.sp2",
	                                          laser_touch, 0, 
											  (weapon->weapon.flags & ITF_NOLEAD) ? NULL : ai_aim_straight( self, weapon ) );
	laser->s.render_scale.Set(0.001, 0.001, 0.001);
	laser->s.alpha		= 0.7f;
	laser->health		= (float)weapon->rnd_damage * rnd() + (float)weapon->base_damage;
	laser->delay		= gstate->time + 3.0;
	laser->think		= laser_think;
	laser->nextthink	= gstate->time + 10.0;

	// link laser to world
	gstate->LinkEntity( laser );


	//Attach the Sprite Light&Glow stuff (add track entity)
    trackInfo_t tinfo;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=laser;
    tinfo.srcent=laser;
    tinfo.fru.Zero();
    tinfo.lightColor.x = 0.75;//R
    tinfo.lightColor.y = 0.35;//G
    tinfo.lightColor.z = 0.35;//B
    tinfo.lightSize= 165;
	
	tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_MODELINDEX|TEF_SCALE|TEF_EXTRAINDEX|TEF_MODELINDEX2|TEF_SCALE2|TEF_HARDPOINT2;
    tinfo.fxflags = TEF_LIGHT|TEF_SPRITE|TEF_MODEL;
	tinfo.renderfx = SPR_ALPHACHANNEL;
	tinfo.scale = 0.30f;
	tinfo.modelindex = gstate->ModelIndex ("models/e1/me_mater.sp2");

	tinfo.modelindex2 = gstate->ModelIndex ("models/e1/me_mater.dkm");  //model to show for TEF_MODEL
	tinfo.scale2 = scale;												//Scale for TEF_MODEL
	
	Com_sprintf(tinfo.HardPoint_Name2,sizeof(tinfo.HardPoint_Name2),buf);							//set the hardpoint name
	
	tinfo.extra = self;

	com->trackEntity(&tinfo,MULTICAST_PVS);
	
	gstate->StartEntitySound(laser, CHAN_AUTO, gstate->SoundIndex("global/we_zapa.wav"), 
           1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);


}


// ----------------------------------------------------------------------------
// NSS[1/26/00]:
// Name:		laser_fire
// Description:fire laser for the inmater
// Input:NA
// Output: userInventory_s *ptr, userEntity_t *self
// Note:
// ----------------------------------------------------------------------------
void laser_fire( userInventory_s *ptr, userEntity_t *self)
{
	char buffer[32];
	Com_sprintf(buffer,sizeof(buffer),"hr_muzzle");
	laser_fire_now(ptr, self, buffer, 0.50 );
}

// ----------------------------------------------------------------------------
// NSS[1/26/00]:
// Name:		lasergat_fire
// Description:fire laser for the lasergat
// Input:NA
// Output: userInventory_s *ptr, userEntity_t *self
// Note:
// ----------------------------------------------------------------------------
void lasergat_fire( userInventory_s *ptr, userEntity_t *self)
{
	char buffer[32];
	Com_sprintf(buffer,sizeof(buffer),"hr_muzzle1");
	laser_fire_now(ptr, self, buffer, 0.25 );
}

/* ******************************** death laser ****************************** */

// ----------------------------------------------------------------------------
//
// Name:		deathlaser_touch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void deathlaser_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
    if ( self->owner == other || self->owner == other->owner ) // don't hit owner or its other owned entities
    {
	    return;
	}

    //Can we damage the object we hit?
    if ( other->takedamage != DAMAGE_NO )
    {
        // make sure we pass the bolt's OWNER as the attacker!
        // pass zero_vector as dir because DAMAGE_INFLICTOR_VEL will override it
		com->RadiusDamage( self, self->owner, self->owner, self->health, DAMAGE_INERTIAL, 96.0 );
		//com->radiusDamage( other, self, self->owner, self->s.origin, zero_vector, self->health, DAMAGE_INERTIAL );
    }
    CVector normal,angles;
    CVector color(.9,.3,.2);

    if ( plane )
    {
		normal = plane->normal;
    }
	else
    {
        normal = self->velocity;
        normal.Normalize();
        normal = normal*-1;
    }

    CVector N_Origin = self->s.origin;
	N_Origin.z += 15.0f;
	// do sparks
    gstate->WriteByte(SVC_TEMP_ENTITY);
    gstate->WriteByte(TE_BOLTER_SPARKS);
    gstate->WriteByte(30);

    gstate->WritePosition(N_Origin);
    gstate->WriteDir(normal);
    gstate->WriteDir(CVector(0.9f, 0.6f, 0.1f));
    gstate->MultiCast(self->s.origin,MULTICAST_PVS);

	// play touch sound
	if (rnd() > .8)
	{
		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/we_zapa.wav"), 
                                  1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
	}
	else
	{
		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/we_zapb.wav"), 
                                  1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
	}
   
    // remove laser
	RELIABLE_UNTRACK(self);
	gstate->RemoveEntity(self);

}

// ----------------------------------------------------------------------------
//
// Name:		deathlaser_think
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void deathlaser_think( userEntity_t *self )
{
	// remove flare
	RELIABLE_UNTRACK(self);
    
	// unhook flare
	self->userHook = NULL;

    // remove laser
	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//
// Name:		deathlaser_fire
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void deathlaser_fire( userInventory_s *ptr, userEntity_t *self )
{
	ai_weapon_t	*weapon = (ai_weapon_t *) ptr;

	// create the laser
	userEntity_t *laser = ai_fire_projectile( self, self->enemy, weapon, "models/e1/we_dsbolt.dkm",
	                                          deathlaser_touch, 0, NULL );
	laser->s.render_scale.Set(3.0, 1.5, 1.5);
	laser->s.renderfx	|= RF_FULLBRIGHT|RF_TRANSLUCENT;
	laser->s.alpha		= 0.7f;
	laser->health		= (float)weapon->rnd_damage * rnd() + (float)weapon->base_damage;
	laser->delay		= gstate->time + 1.0;
	laser->think		= deathlaser_think;
	laser->nextthink	= gstate->time + 3.0;

	// link laser to world
	gstate->LinkEntity( laser );


    // create track light
	trackInfo_t tinfo;
    memset(&tinfo, 0, sizeof(tinfo));
    tinfo.ent			= laser;
    tinfo.srcent		= laser;
	tinfo.dstent		= laser;
    tinfo.lightColor.Set( 0.8f, 0.7f, 0.2f );
    tinfo.lightSize		= 150;
    tinfo.fru.Zero();
	tinfo.flags			= TEF_SRCINDEX|TEF_DSTINDEX|TEF_MODELINDEX|TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_FRU;
    tinfo.fxflags		= TEF_LIGHT | TEF_SPRITE;
	tinfo.scale			= 0.85;
	tinfo.modelindex	= gstate->ModelIndex("models/e1/we_dsboltf.sp2");

    com->trackEntity(&tinfo,MULTICAST_PVS);

	// muzzle flash for owner
	self->s.renderfx	|= RF_MUZZLEFLASH;
}

/* ********************************* laserbeam ************************************** */

// ----------------------------------------------------------------------------
//
// Name:		laserbeam_think
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void laserbeam_think( userEntity_t *self )
{
	CVector dir, last_dir, start, end;

	// jump out quickly if the beam is done firing
	if ( gstate->time >= self->health )
	{
		gstate->RemoveEntity(self);
		return;
	}

	// calculate the weapon firing hardpoint
	ai_weapon_t	*weapon = (ai_weapon_t *) self->owner->curWeapon;
	if (weapon)
	{
		// get the current direction the weapon (monster) is facing
		// generate forward vectors etc for the monster
		CVector angles = self->owner->s.angles; //client->v_angle);
		angles.AngleToVectors(forward, right, up);

		start = self->owner->s.origin + weapon->ofs.x * right + weapon->ofs.y * forward + weapon->ofs.z * up;
	}
	else
	{
		start = self->s.origin;
	}
	
    self->s.old_origin	= self->s.origin;
	self->s.origin		= start;
    self->s.angles		= self->owner->s.angles;

	// force it to redraw
	self->s.frameInfo.frameFlags = FRAME_FORCEINDEX;

	// if an enemy is defined, then shoot towards it.. otherwise fire straight out
	if ( self->enemy )
	{
		last_dir = self->movedir;

		end = self->enemy->s.origin;
		end.z += (self->enemy->viewheight > 8) ? (self->enemy->viewheight-8) : 0;	// adjust for view height so it doesn't shoot into the ground

		dir = end - self->s.origin;
		dir.Normalize ();
		self->movedir = dir;

		//if (dir != last_dir)
		{
			CVector angles;
			VectorToAngles(dir, angles);
			// this tells the laser what angle to draw at
			self->s.angles = angles;
		}
	}
	else
	{
		dir = forward;
	}

	// project the point out toward the enemy
	end = start + dir * 1024.0f;

	// set who we hit..
	tr = gstate->TraceLine_q2( start, end, self->owner, MASK_SHOT );
	if ( tr.ent )
	{
		// this probably is invalid actually...
		CVector vel = self->velocity;
		com->Damage( tr.ent, self, self->owner, end, vel, 2.0f, DAMAGE_INERTIAL);
		com->FlashClient( tr.ent, 0.10, 0.98, 0.98, 0.18 );
	}
	
	// the trace stops at the bounding box.. this ensures the beam hits the actual player model
	end = tr.endpos + dir * 32.0f;

	// specify the end point for the laser
	self->s.old_origin = end;

	// update the think functions
	self->think		= laserbeam_think;
	self->nextthink = gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
//
// Name:		spawn_laserbeam_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void spawn_laserbeam_attack( userEntity_t *self )
{
	// spawn bolt and set up the data fields
	userEntity_t *laserbeam = gstate->SpawnEntity();

	laserbeam->className	= "ai_laserbeam";
	laserbeam->movetype		= MOVETYPE_NONE;
	laserbeam->solid		= SOLID_NOT;
	laserbeam->s.renderfx	|= RF_BEAM | RF_TRANSLUCENT;

	laserbeam->owner		= self;
	laserbeam->enemy		= self->enemy;

	//	set the beam diameter - starting radius is encoded in upper 8 bits
	//	ending radius in the lower 8 bits -- this is a hack to make the network
	//	messages be a little faster
	laserbeam->s.frame = 1;
	laserbeam->s.frame = (laserbeam->s.frame << 8) + laserbeam->s.frame;

	// initialize the origin (it's updated in the think function)
	laserbeam->s.origin = self->s.origin;

	// link it into the world
	gstate->LinkEntity(laserbeam);

	// set up the functions to update the laserbeam
	laserbeam->think		= laserbeam_think;
	laserbeam->nextthink	= gstate->time + 0.1f;
	
	// health used for length of the time the laser shoots
	laserbeam->health		= gstate->time + 0.2f;
}

// ----------------------------------------------------------------------------
//
// Name:		laserbeam_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void laserbeam_attack( userInventory_s *ptr, userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
//	ai_weapon_t	 *weapon = (ai_weapon_t *) ptr;// SCG[1/23/00]: not used

	spawn_laserbeam_attack( self );
}

// ************************* light for cambot *********************************

// ----------------------------------------------------------------------------
//
// Name:		light_think
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void light_think( userEntity_t *self )
{
	trackInfo_t		tinfo;
	userEntity_t	*ignore;
	trace_t			tr;
	CVector			start, end, point, dir;
	static			CVector	lmins(-4, -4, -4);
	static			CVector lmaxs(4, 4, 4);
	camlightHook_t*	hook = (camlightHook_t*)self->userHook;

	// if my great master is dead, kill me too
	if ( !self->owner || !self->owner->inuse || self->owner->deadflag || gstate->bCinematicPlaying)
	{
		// remove light
		RELIABLE_UNTRACK(self);
		gstate->RemoveEntity(self);
		return;
	}

	self->s.frameInfo.frameFlags |= FRAME_FORCEINDEX;

	// calculate the weapon firing hardpoint
	ai_weapon_t	*weapon = (ai_weapon_t *) self->owner->curWeapon;
	if ( weapon )
	{
		// find out which way we are facing
		CVector angles = self->owner->s.angles; 
		angles.AngleToVectors(forward, right, up);

		start = self->owner->s.origin + weapon->ofs.x * right + weapon->ofs.y * forward + weapon->ofs.z * up;
	}
	else
	{
		start = self->s.origin;
	}
	
    self->s.old_origin	= self->s.origin;
	self->s.origin		= start;
    self->s.angles		= self->owner->s.angles;
	playerHook_t *phook = AI_GetPlayerHook( self->owner );

	if ( phook->power_boost == 1 )
	{
		
		if(phook->attack_boost_time == 0)
		{
			RELIABLE_UNTRACK(self);
			//Clear out tinfo
			memset(&tinfo, 0, sizeof(tinfo));
			tinfo.ent=self->owner;
			tinfo.srcent=self->owner;
			tinfo.fru.Zero();
			tinfo.lightColor.x = 0.65;//R
			tinfo.lightColor.y = 0.15;//G
			tinfo.lightColor.z = 0.15;//B
			tinfo.lightSize= 225.00;			
			tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_MODELINDEX|TEF_SCALE|TEF_HARDPOINT2;
			tinfo.fxflags = TEF_LIGHT|TEF_SPRITE|TEF_FX_ONLY;
			tinfo.renderfx = SPR_ALPHACHANNEL;
			tinfo.scale = 3.0f;
			tinfo.modelindex = gstate->ModelIndex ("models/global/e_sflred.sp2");
//			sprintf(tinfo.HardPoint_Name2,"hr_light");
			Com_sprintf(tinfo.HardPoint_Name2,sizeof(tinfo.HardPoint_Name2),"hr_light");
   			// send it down....
			com->trackEntity( &tinfo, MULTICAST_PVS );
			phook->attack_boost_time = 1;
		}
		if(self->owner)
		{
			if(self->owner->enemy)
			{
				dir = self->owner->enemy->s.origin - self->s.origin;
				dir.Normalize();
				//self->movedir = dir;
			}
		}
		// change to red
		self->s.color.Set( 0.80f,0.10f , 0.10f );
	}
	else
	{
		if(phook->attack_boost_time == 1)
		{
			RELIABLE_UNTRACK(self);
			//Clear out tinfo
			memset(&tinfo, 0, sizeof(tinfo));
			tinfo.ent=self->owner;
			tinfo.srcent=self->owner;
			tinfo.fru.Zero();
			tinfo.lightColor.x = 0.65;//R
			tinfo.lightColor.y = 0.65;//G
			tinfo.lightColor.z = 0.15;//B
			tinfo.lightSize= 225.00;			

			tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_MODELINDEX2|TEF_SCALE2|TEF_HARDPOINT2;
			tinfo.fxflags = TEF_LIGHT|TEF_MODEL|TEF_ATTACHMODEL|TEF_FX_ONLY;
			tinfo.renderfx = SPR_ALPHACHANNEL;
			tinfo.scale2 = 3.0f;
			tinfo.modelindex2 = gstate->ModelIndex ("models/e1/me_cambotf.sp2");
//			sprintf(tinfo.HardPoint_Name2,"hr_light");
			Com_sprintf(tinfo.HardPoint_Name2,sizeof(tinfo.HardPoint_Name2),"hr_light");
   			// send it down....
			com->trackEntity( &tinfo, MULTICAST_PVS );
			phook->attack_boost_time = 0;
			self->owner->enemy = NULL;
		}
		// normal yellow color
		self->s.color.Set( 0.60f,0.60f , 0.10f );

		// get the direction the owner is facing
		CVector	ang = self->owner->s.angles;

		// angle light down
		ang.pitch = 45;

		// pan light
		ang.yaw += hook->search_angle;

		// update search angle
		hook->search_angle += hook->search_speed;

		// check for max
		if (hook->search_angle > hook->search_max)
		{
			hook->search_angle = hook->search_max;
			hook->search_speed *= -1.0;
		}

		// check for min
		if (hook->search_angle < hook->search_min)
		{
			hook->search_angle = hook->search_min;
			hook->search_speed *= -1.0;
		}

		AngleToVectors( ang, dir );

	}

	// set the endpoint of the light based on direction and distance
	end = start + (dir * hook->length);		

	// ignore the enemy player
	ignore = self->enemy;
/*
	while(1)
	{
		tr = gstate->TraceLine_q2 (start, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

		if (!tr.ent)
 			break;

		//	if we hit something that's not a monster or player or is immune to lasers, we're done
		if (!(tr.ent->flags & (FL_CLIENT + FL_BOT + FL_MONSTER)))
			break;

		ignore = tr.ent;
		start = tr.endpos;
	}
*/
	tr = gstate->TraceLine_q2( start, end, ignore, ( CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER ) );

	self->s.render_scale = tr.endpos;
	
	self->think		= light_think;
	self->nextthink = gstate->time + 0.1;
}

// ----------------------------------------------------------------------------
//
// Name:		light_spawn
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void light_spawn( userEntity_t* self )
{
	CVector			ang, temp;
	trackInfo_t		tinfo;
	userEntity_t*	light = NULL;
	camlightHook_t*	hook = NULL;

	// spawn light 
	light = gstate->SpawnEntity();
	// setup the light data
	light->movetype			= MOVETYPE_NONE;
	light->solid			= SOLID_NOT;
	light->s.modelindex		= gstate->ModelIndex("models/e1/me_cambotf.sp2");		// must be non-zero
	light->s.alpha			= 0.00;
	// set the position
	light->s.origin			= self->s.origin;
	light->view_ofs			= self->view_ofs;
	// add a spotlight to this thing
	light->s.renderfx		|= (RF_SPOTLIGHT|SPR_ALPHACHANNEL);
	// set the spotlight color
	light->s.color.Set( 0.60f,0.60f ,0.10f );
	
	// set the spotlight diameter
	light->s.frame	= 2;
	// set owner and enemy
	light->owner	= self;
	light->enemy	= self->enemy;
	// setup thinking
	light->think	 = light_think;
	light->nextthink = gstate->time + 0.1;
	// put this entity in the world
	gstate->LinkEntity(light);

	// allocate user hook
	hook = (camlightHook_t*)gstate->X_Malloc(sizeof(camlightHook_t),MEM_TAG_HOOK);
	hook->search_angle	= -45;
	hook->search_speed	= 8;
	hook->search_max	= 64;
	hook->search_min	= -45;
	hook->length		= 600;
	hook->brightness    = 100;
	hook->rotation_speed = 20;

	// link hook
	light->userHook = hook;

	// SCG[11/24/99]: Save game stuff
	light->save = camlight_hook_save;
	light->load = camlight_hook_load;


	//Clear out tinfo
	memset(&tinfo, 0, sizeof(tinfo));
    tinfo.ent=self;
    tinfo.srcent=self;
    tinfo.fru.Zero();
    tinfo.lightColor.x = 0.65;//R
    tinfo.lightColor.y = 0.65;//G
    tinfo.lightColor.z = 0.15;//B
    tinfo.lightSize= 225.00;	
	
	tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_MODELINDEX2|TEF_SCALE2|TEF_HARDPOINT2;
    tinfo.fxflags = TEF_LIGHT|TEF_MODEL|TEF_ATTACHMODEL|TEF_FX_ONLY;
	tinfo.renderfx = SPR_ALPHACHANNEL;
	tinfo.scale2 = 3.0f;
	tinfo.modelindex2 = gstate->ModelIndex ("models/e1/me_cambotf.sp2");
//	sprintf(tinfo.HardPoint_Name2,"hr_light");
	Com_sprintf(tinfo.HardPoint_Name2,sizeof(tinfo.HardPoint_Name2),"hr_light");
   	// send it down....
	com->trackEntity( &tinfo, MULTICAST_PVS );



}

// ----------------------------------------------------------------------------
//
// Name:		light_fire
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void light_fire(userInventory_s *ptr, userEntity_t *self)
{
//	ai_weapon_t	 *weapon = (ai_weapon_t *) ptr;// SCG[1/23/00]: not used
	
	// spawn the light
	light_spawn(self);
}


/* ********************************* wack ************************************** */

// ----------------------------------------------------------------------------
//
// Name:		wack_back
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void wack_back( userInventory_s *ptr, userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	ai_weapon_t	 *weapon = (ai_weapon_t *) ptr;
	userEntity_t *hit = NULL;

	AIMDATA_PTR pAimData = ai_wack_aim( self, weapon );

	tr = gstate->TraceLine_q2( pAimData->org, pAimData->end, self, MASK_SHOT );
	if ( tr.fraction!= 1.0 )
	{
		if ( tr.ent->takedamage != DAMAGE_NO )
		{
			CVector dir = pAimData->end - pAimData->org;
			dir.Normalize();

			trace_damage = (float) weapon->base_damage + rnd() * float (weapon->rnd_damage);
			com->Damage( tr.ent, self, self, tr.endpos, dir, trace_damage, DAMAGE_NONE );

			hit = tr.ent;
		}
		else 
		if ( tr.ent->solid == SOLID_BSP || tr.ent->takedamage == DAMAGE_NO )
		{
			hit = tr.ent;
		}
	}

	if ( hit )
	{
		if ( hit->flags & (FL_CLIENT | FL_MONSTER | FL_BOT) )
		{
			CVector vel = pAimData->dir * (trace_damage * 6);
			vel.z = 0;

			vel = vel + hit->velocity;
			hit->velocity = vel;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		wack_left
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void wack_left( userInventory_s *ptr, userEntity_t *self )
{
//	playerHook_t	*hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	ai_weapon_t		*weapon = (ai_weapon_t *) ptr;
	userEntity_t *hit = NULL;

	AIMDATA_PTR pAimData = ai_wack_aim( self, weapon );

	tr = gstate->TraceLine_q2( pAimData->org, pAimData->end, self, MASK_SHOT );
	if ( tr.fraction!= 1.0 )
	{
		if ( tr.ent->takedamage != DAMAGE_NO )
		{
			CVector dir = pAimData->end - pAimData->org;
			dir.Normalize();

			trace_damage = (float) weapon->base_damage + rnd() * float (weapon->rnd_damage);
			com->Damage( tr.ent, self, self, tr.endpos, dir, trace_damage, DAMAGE_NONE );

			hit = tr.ent;
		}
		else 
		if ( tr.ent->solid == SOLID_BSP || tr.ent->takedamage == DAMAGE_NO )
		{
			hit = tr.ent;
		}
	}

	if (hit)
	{
		if (hit->flags & (FL_CLIENT | FL_MONSTER | FL_BOT))
		{
			CVector vel;
			CVector ang = self->s.angles;
			ang.AngleToVectors(vel, up, right);
			vel =  -(right * trace_damage * 6);
			vel.z = 0;

			vel = vel + hit->velocity;
			hit->velocity = vel;
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		wack_right
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void wack_right( userInventory_s *ptr, userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	ai_weapon_t	 *weapon = (ai_weapon_t *) ptr;
	userEntity_t *hit = NULL;

	AIMDATA_PTR pAimData = ai_wack_aim( self, weapon );

	tr = gstate->TraceLine_q2( pAimData->org, pAimData->end, self, MASK_SHOT );
	if ( tr.fraction!= 1.0 )
	{
		if ( tr.ent->takedamage != DAMAGE_NO )
		{
			CVector dir = pAimData->end - pAimData->org;
			dir.Normalize();

			trace_damage = (float) weapon->base_damage + rnd() * float (weapon->rnd_damage);
			com->Damage( tr.ent, self, self, tr.endpos, dir, trace_damage, DAMAGE_NONE );

			hit = tr.ent;
		}
		else 
		if ( tr.ent->solid == SOLID_BSP || tr.ent->takedamage == DAMAGE_NO )
		{
			hit = tr.ent;
		}
	}

	if ( hit )
	{
		if ( hit->flags & (FL_CLIENT | FL_MONSTER | FL_BOT) )
		{
			CVector vel;
			CVector ang = self->s.angles;
			ang.AngleToVectors(vel, up, right);
			vel = (right * trace_damage * 6);
			vel.z = 0;

			vel = vel + hit->velocity;
			hit->velocity = vel;
		}
	}
}

/* ********************************* spear ************************************** */
void throwing_knife_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf );
// ----------------------------------------------------------------------------
//
// Name:		spear_think
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void spear_think( userEntity_t *self )
{
	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//
// Name:		spear_touch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void spear_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	CVector vel = self->velocity;
	// did we hit the ground?  if so.. stick in the wall for 5 seconds and then delete
	if ( !stricmp (other->className, "worldspawn") )
	{
		// we don't want to collide with these
		self->movetype = MOVETYPE_NONE;
		self->solid	= SOLID_NOT;

		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/m_armorhite.wav"), 
			1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	}
	else
	{
		// change the knife to MOVETYPE_TOSS and let it bounce to the ground
		self->movetype = MOVETYPE_BOUNCE;
		com->Damage( other, self, self->owner, self->s.origin, vel, self->health, DAMAGE_INERTIAL );
		// rotate the knife so it falls on it's flag side
		self->s.angles.x = 0;
		self->s.angles.y = 90;

		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/e_bulfleshc.wav"), 
			1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	}

	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//
// Name:		spear_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void spear_attack( userInventory_s *ptr, userEntity_t *self )
{
	ai_weapon_t	 *weapon = (ai_weapon_t *) ptr;

	userEntity_t *spear = ai_fire_projectile( self, self->enemy, weapon, "models/e2/me_spear.dkm",
	                                          throwing_knife_touch, 0, NULL );

	spear->s.render_scale.Set( 1.0, 1.0, 1.0 );
	spear->health = (float)weapon->base_damage + ((float)weapon->rnd_damage * rnd());

	spear->delay = gstate->time + 3.0;

	spear->think = spear_think;
	spear->nextthink = gstate->time + 3.0;
}


/* ********************************* lightning attacks ********************************* */

// ----------------------------------------------------------------------------
//
// Name:		ai_lightning_think
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void ai_lightning_think( userEntity_t *self )
{
	RELIABLE_UNTRACK(self);
	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//
// Name:		spawn_lightning_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void spawn_lightning_attack( userEntity_t *self, userEntity_t *target, const CVector &spawnPt )
{
	trackInfo_t	tinfo;

	// clear this variable
	memset(&tinfo, 0, sizeof(tinfo));

	// spawn bolt and set up the data fields
	userEntity_t *bolt = gstate->SpawnEntity();
	
	bolt->className		= "ai_lightning_bolt";
	bolt->movetype		= MOVETYPE_NONE;
	bolt->solid			= SOLID_NOT;
	bolt->s.renderfx	= RF_TRACKENT|RF_LIGHTNING;    // drawn by client
	bolt->s.frameInfo.frameFlags = FRAME_FORCEINDEX;

	bolt->s.frame	= 8.0;					// beam diameter
	bolt->s.skinnum = 0xa1a2a3a4;					// beam color
	bolt->s.alpha	= 0.6;
	bolt->owner		= self;
	bolt->s.origin	= spawnPt;

	// link it into the world
	gstate->LinkEntity(bolt);

	// set up the functions to update the lightning bolt
	// nice blue light
	tinfo.lightColor.Set( 0.2, 0.8, 0.9 );
   	tinfo.fru.Zero();

	bolt->think			= ai_lightning_think;
	bolt->nextthink		= gstate->time + 0.5;

	// msg client to track this entity
	tinfo.lightSize		= 240;
	tinfo.ent			= bolt;
	tinfo.srcent		= bolt;
	tinfo.altangle.Set(0.25,0.45,0.85); //Set color of bolt
	tinfo.altpos2.x		= 2.0f;			//Size of Bolt
	tinfo.altpos2.y		= 1.0f;			//Modulation of Bolt
	tinfo.dstent		= target;
	tinfo.renderfx		= RF_LIGHTNING|RF_TRANSLUCENT;
	tinfo.flags			= TEF_SRCINDEX|TEF_DSTINDEX|TEF_FRU|TEF_LIGHT|TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_ALTPOS2|TEF_ALTANGLE;
   	// send it down....
	com->trackEntity( &tinfo, MULTICAST_PVS );
}

// ----------------------------------------------------------------------------
//
// Name:		ValkyrieZapThink
// Description:Think function for the wisps themselves
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void ValkyrieZapThink(userEntity_t *self)
{
    wyndraxHook_t *hook=(wyndraxHook_t *)self->userHook;
	CVector Offset,Target_Angles,Forward;
//	short randomZap=true;

    CVector Angles;
	CVector dir = self->enemy->s.origin - self->s.origin;
	dir.Normalize();
	
	VectorToAngles(dir,Angles);
	self->s.angles = Angles;	
	
	// time to die?
    if (gstate->time >= hook->killtime)
    {
		removeZap(self);
        return;
    }
//	float fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );// SCG[1/23/00]: not used
	
	// zap player!
	if(hook->dummyCount < 4)
	{		
		//Calculate offset for staff that rotates with model... 
		//Just as good as hard points. <nss>
		
		Offset = self->enemy->s.origin - self->s.origin;
		Offset.Normalize();
		VectorToAngles(Offset,Target_Angles);
		//Target_Angles.yaw +=45;
		Target_Angles.yaw -= 5;
		Target_Angles.AngleToForwardVector(Forward);
		Offset = (Forward * 40)+self->s.origin;
		//Offset.z +=15.0f;
		Offset.z -= 40.0f;

		spawnZap(self,Offset,self->enemy,hook->forward,0.25);

		Offset = self->enemy->s.origin - self->s.origin;
		Offset.Normalize();
		VectorToAngles(Offset,Target_Angles);
		//Target_Angles.yaw +=45;
		Target_Angles.yaw -= 5;
		Target_Angles.AngleToForwardVector(Forward);
		Offset = (Forward * 24)+self->s.origin;
		//Offset.z +=15.0f;
		Offset.z -= 40.0f;

		spawnZapFlare(self,&Offset,0.15, 2.15f, "models/global/e_flblue.sp2");

		hook->dummyCount+=2;
		
	}
    self->nextthink = gstate->time + 0.1;
}

// ----------------------------------------------------------------------------
//
// Name:		lightning_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void lightning_attack( userInventory_s *ptr, userEntity_t *self )
{
    wyndraxHook_t *hook;
	userEntity_t *Zap;
	
	Zap = gstate->SpawnEntity();
	Zap->movetype	= MOVETYPE_NONE;
	Zap->solid		= SOLID_NOT;
	Zap->owner		= self;
	Zap->s.frame	= 0;

	Zap->s.origin = self->s.origin;
	Zap->s.origin.z +=24;
	Zap->className		= "Wyndrax_Zap";
	Zap->enemy			= self->enemy;
	Zap->s.frame		= 0;
    Zap->think			= ValkyrieZapThink;
    Zap->nextthink		= gstate->time + 0.1f;
    Zap->remove			= removeZap;
	Zap->clipmask		= MASK_SHOT;

	Zap->userHook = gstate->X_Malloc(sizeof(wyndraxHook_t),MEM_TAG_HOOK);
    hook=(wyndraxHook_t *)Zap->userHook;

	// SCG[11/24/99]: Save game stuff
	Zap->save = wyndrax_hook_save;
	Zap->load = wyndrax_hook_load;

	gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e3/m_wwisplightning.wav"), 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );	

	hook->killtime		= gstate->time + 0.55f;
	//Store the origin of the player.
	hook->forward		= self->enemy->s.origin;

	gstate->LinkEntity(Zap);
	AI_Dprintf("Zap Entity alive\n");
}

// ----------------------------------------------------------------------------
//
// Name:		lightning_punch
// Description:
// Input:
// Output:
// Note:
//	"fires" a standard melee attack and a lightning bolt
//	which reaches hook->attack_dist
//
//	only plays hook->sound2 if target is hit
//
//	hook->sound1 can be used in the monster's code to
//	play a swinging sound
//
// ----------------------------------------------------------------------------
void lightning_punch( userInventory_s *ptr, userEntity_t *self )
{
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used
	ai_weapon_t	 *weapon = (ai_weapon_t *) ptr;
	trackInfo_t tinfo;

	// clear this variable
	memset( &tinfo, 0, sizeof(tinfo) );

	userEntity_t *hit = ai_trace_attack (self, weapon);
	if ( hit )
	{
	
		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e3/we_wwispcorditec.wav"), 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );	
/***********temporary code until new track entity stuff is in**********************/

		// spawn dummy entity and set up the data fields
		userEntity_t *dummy = gstate->SpawnEntity();
		
		dummy->className = "dummy";
		dummy->movetype = MOVETYPE_NONE;
		dummy->solid = SOLID_NOT;
		dummy->s.renderfx = RF_TRACKENT;    // drawn by client
		dummy->s.frameInfo.frameFlags = FRAME_FORCEINDEX;

		dummy->s.modelindex = gstate->ModelIndex ("models/global/we_mflash.dkm");

		dummy->s.frame = 8.0f;					// beam diameter
		dummy->s.skinnum = 0xa1a2a3a4;					// beam color
		dummy->s.alpha = 0.6;
		dummy->owner = self;
		dummy->s.origin = self->s.origin;

		dummy->s.render_scale.Set( 0.01f, 0.01f, 0.01f );

		// link it into the world
		gstate->LinkEntity(dummy);

		// set up the functions to turn off the dynamic lighting in a half second's time
		dummy->think = ai_lightning_think;
		dummy->nextthink = gstate->time + 0.5;

/**********************************************************************************/

		// add the dynamic light around the bitch
		tinfo.flags=0;
		tinfo.ent=dummy;	// self;
		tinfo.srcent=dummy;		// self;
		tinfo.fru.Zero();
		tinfo.lightColor.Set( 0.2f, 0.2f, 0.9f );
		tinfo.lightSize= -240;
		tinfo.flags = TEF_SRCINDEX|TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS;
		tinfo.fxflags = TEF_LIGHT;
		com->trackEntity(&tinfo,MULTICAST_PVS);
		
		// this means it hit something besides the worldspawn entity
		if ( hit->flags & (FL_CLIENT | FL_MONSTER | FL_BOT) )
		{
			CVector forward;
			CVector ang = self->s.angles;
			ang.AngleToForwardVector (forward);
			CVector vel = (forward * trace_damage * 30);
			vel.z = 40 + (trace_damage);	 // send 'em in the air a bit

			vel = vel + hit->velocity;
			hit->velocity = vel;
		}
	}
}

/* ********************************* axe attacks ********************************* */

// ----------------------------------------------------------------------------
//
// Name:		throwing_axe_think
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void throwing_axe_think( userEntity_t *self )
{
	float elapsedTime = gstate->time - self->delay;
	
	// start fading out
	if ( elapsedTime >= 4.0 )
	{
		if ( elapsedTime >= 5.0 )
		{
			gstate->RemoveEntity(self);
		}
		else
		{
			self->s.alpha = 5.0 - elapsedTime;  // gives us a value from 1.0 to 0.0
		}
		self->nextthink = gstate->time + 0.1;
	}
	else
	{
		if ( (self->movetype == MOVETYPE_BOUNCE) && (self->groundEntity != NULL) )
		{
			// we don't want to collide with these
			self->movetype = MOVETYPE_NONE;
			self->solid	= SOLID_NOT;
		}
		
		// don't call so often before the 5 seconds
		self->nextthink = gstate->time + 0.2;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		throwing_axe_touch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void throwing_axe_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	CVector vel = self->velocity;

	// stop the axe from moving and rotating
	self->velocity.Set(0.0, 0.0, 0.0);
	self->avelocity.Set(0.0, 0.0, 0.0);

	// no more touching please
	self->touch = NULL;

	// set up the time this is happening so we can fade it out etc
	self->delay = gstate->time;
	self->s.renderfx |= RF_TRANSLUCENT;
	self->s.alpha = 1.0;

	// did we hit the ground?  if so.. stick in the wall for 5 seconds and then delete
	if ( !stricmp (other->className, "worldspawn") )
	{
		CVector forward;
		self->s.angles.x = 0;	// we want it facing straight ahead for the following calculation
		AngleToVectors(self->s.angles, forward);
		self->s.angles.x = 300;  // now rotate it so it "sticks" in the wall
		forward.Multiply( -12.0f );
		self->s.origin.Add( forward );
		// we don't want to collide with these
		self->movetype = MOVETYPE_NONE;
		self->solid	= SOLID_NOT;

		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/m_bodyhitc.wav"), 
			1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	}
	else
	{
		// change the axe to MOVETYPE_TOSS and let it bounce to the ground
		self->movetype = MOVETYPE_BOUNCE;
		com->Damage( other, self, self->owner, self->s.origin, vel, self->health, DAMAGE_INERTIAL );
		// rotate the axe so it falls on it's flag side
		self->s.angles.x = 0;
		self->s.angles.y = 90;

		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/e_bulfleshc.wav"), 
			1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	}

	// the axe dies/disappears after 5 seconds...
	self->think = throwing_axe_think;
	self->nextthink = gstate->time + 0.2;
}

// ----------------------------------------------------------------------------
//
// Name:		throwing_axe_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void throwing_axe_attack( userInventory_s *ptr, userEntity_t *self )
{
	ai_weapon_t *weapon = (ai_weapon_t *) ptr;

	userEntity_t *pAxe = ai_fire_projectile( self, self->enemy, weapon, "models/e3/me_axe.dkm",
	                                         throwing_axe_touch, 0, NULL );
	if ( pAxe )
	{
		// rotate this axe pitch-wise
		pAxe->avelocity.Set( 300.0, 0.0, 0.0 );
		pAxe->s.render_scale.Set( 1.0, 1.0, 1.0 );
		
		pAxe->health = pAxe->health = (float)weapon->base_damage + ((float)weapon->rnd_damage * rnd());

		pAxe->think = throwing_axe_think;
		pAxe->nextthink = gstate->time + 3.0;

		gstate->StartEntitySound(pAxe, CHAN_AUTO, gstate->SoundIndex("e3/m_dwaraxfly.wav"), 
			1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	}
}

/* ********************************* knife attacks ********************************* */

// ----------------------------------------------------------------------------
//
// Name:		throwing_knife_think
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void throwing_knife_think( userEntity_t *self )
{
	float elapsedTime = gstate->time - self->delay;
	
	// start fading out
	if ( elapsedTime >= 4.0 )
	{
		if ( elapsedTime >= 5.0 )
		{
			gstate->RemoveEntity(self);
		}
		else
		{
			self->s.alpha = 5.0 - elapsedTime;  // gives us a value from 1.0 to 0.0
		}
		self->nextthink = gstate->time + 0.1;
	}
	else
	{
		if ( (self->movetype == MOVETYPE_BOUNCE) && (self->groundEntity != NULL) )
		{
			// we don't want to collide with these
			self->movetype = MOVETYPE_NONE;
			self->solid	= SOLID_NOT;
		}
		
		// don't call so often before the 5 seconds
		self->nextthink = gstate->time + 0.2;
	}
}

// ----------------------------------------------------------------------------
//
// Name:		throwing_knife_touch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void throwing_knife_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	CVector vel = self->velocity;

	// stop the knife from moving and rotating
	self->velocity.Set(0.0, 0.0, 0.0);
	self->avelocity.Set(0.0, 0.0, 0.0);

	// no more touching please
	self->touch = NULL;

	// set up the time this is happening so we can fade it out etc
	self->delay = gstate->time;
	self->s.renderfx |= RF_TRANSLUCENT;
	self->s.alpha = 1.0;

	// did we hit the ground?  if so.. stick in the wall for 5 seconds and then delete
	if ( !stricmp (other->className, "worldspawn") )
	{
		// we don't want to collide with these
		self->movetype = MOVETYPE_NONE;
		self->solid	= SOLID_NOT;

		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/m_armorhite.wav"), 
			1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	}
	else
	{
		// change the knife to MOVETYPE_TOSS and let it bounce to the ground
		self->movetype = MOVETYPE_BOUNCE;
		com->Damage( other, self, self->owner, self->s.origin, vel, self->health, DAMAGE_INERTIAL );
		// rotate the knife so it falls on it's flag side
		self->s.angles.x = 0;
		self->s.angles.y = 90;

		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/e_bulfleshc.wav"), 
			1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	}

	// the knife dies/disappears after 5 seconds...
	self->think = throwing_knife_think;
	self->nextthink = gstate->time + 0.2;
}

// ----------------------------------------------------------------------------
//
// Name:		throwing_knife_attack
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void throwing_knife_attack( userInventory_s *ptr, userEntity_t *self )
{
	ai_weapon_t *weapon = (ai_weapon_t *) ptr;

	userEntity_t *pKnife = ai_fire_projectile( self, self->enemy, weapon, "models/e2/me_thief.dkm",
	                                         throwing_axe_touch, 0, NULL );
	if ( pKnife )
	{
		// rotate this knife pitch-wise
		pKnife->avelocity.Set( 300.0, 0.0, 0.0 );
		pKnife->s.render_scale.Set( 1.0, 1.0, 1.0 );
		
		pKnife->health = (float)weapon->base_damage + ((float)weapon->rnd_damage * rnd());

		pKnife->think = throwing_knife_think;
		pKnife->nextthink = gstate->time + 3.0;

//		gstate->StartEntitySound(pKnife, CHAN_AUTO, gstate->SoundIndex("e3/m_dwaraxfly.wav"), 
//			1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
	}
}

/* ********************************* psyclaw attacks ********************************* */
// ----------------------------------------------------------------------------
//NSS[11/8/99]:
// Name:		psyclaw_attack_think
// Description: Think function for psyclaw weirdo attack
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void psyclaw_attack_think( userEntity_t *self )
{
	psyclawHook_t *hook=(psyclawHook_t *)self->userHook;
	CVector C1,C2,Color;
	if(self->s.render_scale.x > hook->scale_max)
		hook->delta_scale = 0.90;	
	else if(self->s.render_scale.x < hook->scale_min)
		hook->delta_scale = 1.10;	

	hook->delta_color += hook->color_dir;
	C1.Set(0.45,-0.15,1.0);
	C2.Set(0,1.0,0);
	Color = Color.Interpolate(C1,C2,hook->delta_color * 0.04f);
	self->s.color = Color;
	if(hook->delta_color >= 25)
		hook->color_dir = -8;
	else if(hook->delta_color <=0)
		hook->color_dir = 8;

	self->s.render_scale = self->s.render_scale * hook->delta_scale;
	
	self->nextthink = gstate->time + 0.1;
}
// ----------------------------------------------------------------------------
//NSS[11/8/99]:
// Name:		psyclaw_warp
// Description: Think function for warping the client's view
// Input: userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void psyclaw_warp(userEntity_t *self)
{
	psyclawHook_t *hook		= (psyclawHook_t *)self->userHook;
	userEntity_t *client	= self->owner;
	CVector	C1,C2,Color;
	float fade_out;
	
	//Do we need to restore the client's information?
	if(hook->time_to_live < gstate->time)
	{
		client->flags		&= ~FL_WARPEDVIEW;
		client->client->ps.fov = hook->c_fov;
		client->client->ps.kick_angles = hook->kick_angles;
		if(hook->Psyclaw)
		{
			playerHook_t *phook = AI_GetPlayerHook( self->owner );
			phook->attack_dist  = hook->delta_alpha;
		}
		//Clear the looped sound
		gstate->StartEntitySound( client, CHAN_LOOP, gstate->SoundIndex("global/e_atmospheref.wav"),0.0f, 0, 0 );
		gstate->RemoveEntity(self);
	}
	else//Let's get nasty
	{
		//If we have less than 3 seconds left on the FX let's start to fade back 'into' reality
		if( ( hook->time_to_live - gstate->time ) < 3.0f )
		{
			fade_out = (hook->time_to_live - gstate->time)/3;
			//Restore the clients fov to nermal
			if(client->client->ps.fov > hook->c_fov)
			{
				client->client->ps.fov -= 0.50f;
			}
			else if (client->client->ps.fov < hook->c_fov)
			{
				client->client->ps.fov += 0.50f;
			}
		}
		else//Otherwise keep the nasty settings
		{
			C1 = client->velocity * (0.55*crand());
			client->velocity.x = C1.x; 
			client->velocity.y = C1.y; 
			fade_out = 1.0f;
			client->client->ps.fov += 5.0f * sin_tbl[hook->delta_angles];
		}
		//Calculations for color fading and kick angles are done here.
		client->client->ps.kick_angles.roll += (6.0f*fade_out) * cos_tbl[hook->delta_angles];
		hook->delta_angles++;
		hook->delta_color += hook->color_dir;
		C1.Set(0.25,-0.15,1.0);
		C2.Set(0,0.85,0);
		Color = Color.Interpolate(C1,C2,hook->delta_color * 0.03f);
		com->FlashClient(client, (0.35f*fade_out),Color.x ,Color.y ,Color.z , 10.0f );
		if(hook->delta_angles > 12)
			hook->delta_angles = 0;
		if(hook->delta_color >= 25)
			hook->color_dir = -5;
		else if(hook->delta_color <=0)
			hook->color_dir = 5;
	}
	self->nextthink = gstate->time + 0.1;
}

// ----------------------------------------------------------------------------
//NSS[11/8/99]:
// Name:		psyclaw_touch
// Description: Touch function for psyclaw weirdo attack
// Input: userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void psyclaw_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	psyclawHook_t *hook=(psyclawHook_t *)self->userHook;
	playerHook_t *phook;

	if(self->owner)
	{
		phook = AI_GetPlayerHook( self->owner );
	}
	//Are we the client and do they already have a warped view fx applied to them?
	if((other->flags & FL_CLIENT) && !(other->flags & FL_WARPEDVIEW)&&(self->owner))
	{
		if(phook && self->owner)
		{
			hook->delta_alpha   = phook->attack_dist;
			phook->attack_dist	= 100.0f;
			hook->Psyclaw		= self->owner;
			other->flags		|= FL_WARPEDVIEW;
			self->s.modelindex	= 0;
			self->solid			= SOLID_NOT;
			self->think			= psyclaw_warp; 
			self->touch			= NULL;
			self->owner			= other;
			hook->kick_angles	= other->client->ps.kick_angles;
			hook->c_fov			= other->client->ps.fov;
			hook->time_to_live	= gstate->time + 8.0f;
			hook->delta_angles	= 0;
			hook->color_dir		= 5;
			hook->delta_color	= 0;
			self->velocity.Zero();
			self->nextthink = gstate->time + 0.1;
			com->Damage( other, self, self->owner, self->s.origin, self->velocity,hook->damage, DAMAGE_INERTIAL );
			RELIABLE_UNTRACK(self);
			gstate->StartEntitySound( other, CHAN_LOOP, gstate->SoundIndex("global/e_atmospheref.wav"),0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );
			return;
		}
		else if(self->owner)
		{
			com->Damage( other, self, self->owner, self->s.origin, self->velocity,hook->damage, DAMAGE_INERTIAL );
		}
		RELIABLE_UNTRACK(self);
		gstate->RemoveEntity(self);
	}

	if(self->owner)
		com->Damage( other, self, self->owner, self->s.origin, self->velocity,hook->damage, DAMAGE_INERTIAL );
	RELIABLE_UNTRACK(self);
	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//NSS[11/8/99]:
// Name:		psyclaw_attack
// Description:psyclaw for the Doombat
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void psyclaw_attack( userInventory_s *ptr, userEntity_t *self )
{
	ai_weapon_t	*weapon = (ai_weapon_t *) ptr;
	CVector Dir,angles;
	userEntity_t *pPsyclaw	= ai_fire_projectile( self, self->enemy, weapon, "models/e1/me_psyclaw.dkm", psyclaw_touch, 0, NULL );
	userEntity_t *BigOne	= pPsyclaw;
	pPsyclaw->avelocity.Set( 0.0, 160.0, 120.0 );
	pPsyclaw->s.render_scale.Set(1.0f,1.0f,1.0f);

	pPsyclaw->health	= (float)weapon->base_damage + ((float)weapon->rnd_damage * rnd());
	pPsyclaw->delay		= gstate->time + 5.0;
	pPsyclaw->think		= psyclaw_attack_think;
	pPsyclaw->nextthink = gstate->time + gstate->frametime;
	pPsyclaw->s.alpha	= 0.45f;
	pPsyclaw->owner		= self;
	pPsyclaw->s.renderfx= RF_TRANSLUCENT;
	angles				= self->s.angles;
	angles.AngleToForwardVector(Dir);
	Dir					= Dir * 40.0f;
	pPsyclaw->s.origin	= self->s.origin + Dir;
	pPsyclaw->s.origin.z+=15.0f;
	
	gstate->SetSize( pPsyclaw,-5, -5, -5, 5, 5, 5);
	gstate->LinkEntity( pPsyclaw );

	//NSS[10/27/99]: Let's create a user hook specificly for the meteor
	pPsyclaw->userHook = gstate->X_Malloc(sizeof(psyclawHook_t),MEM_TAG_HOOK);

	// SCG[11/24/99]: Save game stuff
	pPsyclaw->save = psyclaw_hook_save;
	pPsyclaw->load = psyclaw_hook_load;

    psyclawHook_t *hook=(psyclawHook_t *)pPsyclaw->userHook;

	hook->time_to_live	= gstate->time + 8.0f;
	hook->delta_alpha	= 0.55f;
	hook->delta_scale	= 1.45f;
	hook->scale_max		= 3.0f;
	hook->scale_min		= 1.15f;
	hook->damage		= weapon->base_damage + rnd()*weapon->rnd_damage;
	
	//Setup special effects for the psyclaw's whacky special FX
    trackInfo_t tinfo;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

	tinfo.fru.Zero();
	tinfo.ent			= pPsyclaw;
    tinfo.srcent		= pPsyclaw;
    tinfo.flags			= TEF_FXFLAGS|TEF_SRCINDEX|TEF_SCALE|TEF_MODELINDEX;
    tinfo.fxflags		= TEF_LIGHT|TEF_SPRITE;
   	tinfo.renderfx		= SPR_ALPHACHANNEL;
	tinfo.scale			= 1.0f;
	tinfo.modelindex	= gstate->ModelIndex ("models/global/e_sflgreen.sp2");
	com->trackEntity(&tinfo,MULTICAST_PVS);

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

	tinfo.fru.Zero();
	tinfo.ent			= self;
    tinfo.srcent		= self;
    tinfo.lightColor.x	= -1.0;//R
    tinfo.lightColor.y	= 1.0;//G
    tinfo.lightColor.z	= -1.0;//B
    tinfo.lightSize		= 100;
    tinfo.flags			= TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_LENGTH;
    tinfo.fxflags		= TEF_LIGHT|TEF_LIGHT_SHRINK;
	tinfo.length		= 2.0f;
	com->trackEntity(&tinfo,MULTICAST_PVS);


	pPsyclaw = ai_fire_projectile( self, self->enemy, weapon, "models/e1/me_psyclaw.dkm", psyclaw_touch, 0, NULL );

	pPsyclaw->avelocity.Set( 0.0, 220.0, 160.0 );
	pPsyclaw->s.render_scale.Set(0.80f,0.80f,0.80f);

	pPsyclaw->health	= 0;
	pPsyclaw->delay		= gstate->time + 0.5;
	pPsyclaw->think		= psyclaw_attack_think;
	pPsyclaw->nextthink = gstate->time + gstate->frametime;
	pPsyclaw->s.alpha	= 0.45f;
	pPsyclaw->owner		= BigOne;
	pPsyclaw->s.renderfx= RF_TRANSLUCENT;
	angles				= self->s.angles;
	angles.AngleToForwardVector(Dir);
	Dir					= Dir * 40.0f;
	pPsyclaw->s.origin	= self->s.origin + Dir;
	pPsyclaw->s.origin.z+=15.0f;
	
	gstate->SetSize( pPsyclaw,0, 0, 0, 0, 0, 0);
	gstate->LinkEntity( pPsyclaw );

	//NSS[10/27/99]: Let's create a user hook specificly for the meteor
	pPsyclaw->userHook = gstate->X_Malloc(sizeof(psyclawHook_t),MEM_TAG_HOOK);

	// SCG[11/24/99]: Save game stuff
	pPsyclaw->save = psyclaw_hook_save;
	pPsyclaw->load = psyclaw_hook_load;

    hook=(psyclawHook_t *)pPsyclaw->userHook;

	hook->time_to_live	= gstate->time + 8.0f;
	hook->delta_alpha	= 0.55f;
	hook->delta_scale	= 1.45f;
	hook->scale_max		= 2.0f;
	hook->scale_min		= 0.80f;
	hook->damage		= weapon->base_damage + rnd()*weapon->rnd_damage;
	hook->color_dir		= -8.0f;
	hook->delta_color	= 22.0f;
    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

	tinfo.fru.Zero();
	tinfo.ent			= self;
    tinfo.srcent		= self;
    tinfo.lightColor.x	= -1.0;//R
    tinfo.lightColor.y	= 1.0;//G
    tinfo.lightColor.z	= -1.0;//B
    tinfo.lightSize		= 100;
    tinfo.flags			= TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_LENGTH;
    tinfo.fxflags		= TEF_LIGHT|TEF_LIGHT_SHRINK;
	tinfo.length		= 2.0f;
	com->trackEntity(&tinfo,MULTICAST_PVS);

}


/* ********************************* pistol ********************************* */

// ----------------------------------------------------------------------------
//
// Name:		pistol_fire
// Description: single fire bullet             
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void pistol_fire( userInventory_s *ptr, userEntity_t *self )
{
	ai_weapon_t *weapon = (ai_weapon_t *) self->curWeapon;

	//self->s.renderfx |= RF_MUZZLEFLASH;
	
	ai_fire_bullet (self, self->enemy, weapon);
}


/* ******************************** shotgun ********************************* */

void shotgun_think( userEntity_t *self )
{
	if(self->owner)
	{
		//remove the tracked entities
		RELIABLE_UNTRACK(self->owner);
	}

	gstate->RemoveEntity(self);
}
// NSS[1/22/00]:
void shotgun_fire( userInventory_s *ptr, userEntity_t *self )
{
	ai_weapon_t *weapon = (ai_weapon_t *) self->curWeapon;
//	playerHook_t *hook = AI_GetPlayerHook( self );// SCG[1/23/00]: not used

    trackInfo_t tinfo;

	AIMDATA_PTR pAimData = ai_aim_curWeapon( self, weapon );

	tr = gstate->TraceLine_q2( pAimData->org, pAimData->end, self, MASK_SHOT );

	CVector dir = pAimData->end - pAimData->org;
	dir.Normalize();

	// if distance <= 256, damage = fActualDamage;
	// if distance > 256,  damage = fActualDamage * percent of dist from 256 to weapon->distance
	if ( tr.ent && tr.ent->takedamage != DAMAGE_NO )
	{
		float fActualDamage;
		fActualDamage = (float)weapon->base_damage + rnd() * float(weapon->rnd_damage);

		float distance = VectorDistance( self->s.origin, self->enemy->s.origin );
		float max = 256.0f;
		if( distance > 256 )
			fActualDamage = fActualDamage * (1.0f - ( (distance - max) / (weapon->distance - max)));

		com->Damage( tr.ent, self, self, tr.endpos, dir, fActualDamage, DAMAGE_INERTIAL );
	}


	userEntity_t *gunFlash;
	gunFlash = gstate->SpawnEntity();
	
	gunFlash->movetype	= MOVETYPE_NONE;
	gunFlash->solid		= SOLID_NOT;
	gunFlash->owner		= self;
	gunFlash->s.frame	= 0;
	gunFlash->think		= shotgun_think;
	gunFlash->nextthink = gstate->time + 0.20f;

	gstate->LinkEntity(gunFlash);

	// NSS[1/21/00]:Setup a track entity for the muzzleflash
	// clear this variable
	memset(&tinfo, 0, sizeof(tinfo));

	tinfo.fru.Zero();
	tinfo.ent			= self;
	tinfo.srcent		= self;
	tinfo.modelindex2	= gstate->ModelIndex("models/global/we_mflash.dkm");
	tinfo.scale			= 2.05f;
	tinfo.flags			= TEF_FXFLAGS|TEF_SRCINDEX|TEF_MODELINDEX2|TEF_SCALE|TEF_HARDPOINT2;
	tinfo.fxflags		= TEF_MODEL|TEF_ATTACHMODEL|TEF_FX_ONLY;
	tinfo.renderfx		= SPR_ALPHACHANNEL;
	Com_sprintf(tinfo.HardPoint_Name2,sizeof(tinfo.HardPoint_Name2),"hr_muzzle1");
	
	com->trackEntity(&tinfo,MULTICAST_PVS);	
}

/* ******************************** venom spit ****************************** */

// maximum length of time that the venom spit lives, in seconds
#define VENOM_SPIT_LIFETIME 5.0

// ----------------------------------------------------------------------------
//
// Name:		venom_spit_think
// Description: what the venom projectile is thinking
// Input:
// Output:
// Note:        called after delay
//
// ----------------------------------------------------------------------------

void venom_spit_think( userEntity_t *self )
{
	RELIABLE_UNTRACK(self);
	gstate->RemoveEntity(self);
}


// ----------------------------------------------------------------------------
//
// Name:		venom_spit_touch
// Description: what happens when the venom projectile touches an entity
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void venom_spit_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	if(self->owner != other->owner)
	{
		com->Damage( other, self, self->owner, self->s.origin, self->velocity, self->health, DAMAGE_INERTIAL );
		//Only poison the player not mikiko or superfly... 
		if(other->flags & FL_CLIENT)
		{
			com->Poison( other, 1.0, 15.0, 3.0 );
		}
	}
	RELIABLE_UNTRACK(self);
	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//
// Name:		venom_spit
// Description: fires a spit o' venom in your eye
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void venom_spit(userInventory_s *ptr, userEntity_t *self)
{
	ai_weapon_t *weapon = (ai_weapon_t *) self->curWeapon;
	userEntity_t *spit = ai_fire_projectile( self, self->enemy, weapon, "models/e3/me_rotspit.dkm",
		venom_spit_touch, 0, NULL );
	
	spit->s.render_scale.Set(0.1, 0.1, 0.1);
	gstate->SetSize(spit, -3, -3, -3, 3, 3, 3);
	spit->s.effects = EF_POISON;
	spit->s.alpha	= 1.0;
	spit->owner		= self;
	spit->health = (float)weapon->base_damage + ((float)weapon->rnd_damage * rnd());
	
	spit->delay = gstate->time + VENOM_SPIT_LIFETIME;
	spit->s.origin.z += 10.0f;
	spit->think = venom_spit_think;
	spit->nextthink = gstate->time + VENOM_SPIT_LIFETIME;
	
	gstate->LinkEntity(spit);
	trackInfo_t tinfo;
	memset(&tinfo, 0, sizeof(tinfo));
	
	//Create the negative light FX to simulate the funky jiz light
	tinfo.ent			= spit;
	tinfo.srcent		= spit;
	tinfo.fru.Zero();
	tinfo.flags			= TEF_FXFLAGS|TEF_SRCINDEX;
	tinfo.fxflags		= TEF_THUNDERSKEET_FX;
	
	com->trackEntity(&tinfo,MULTICAST_PVS);
	gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e3/e_firespitf.wav"),0.45f, ATTN_NORM_MIN, ATTN_NORM_MAX );
}

/* ******************************** rock throw ****************************** */

// maximum length of time that the rock lives, in seconds
#define ROCK_THROW_LIFETIME 3.0

// ----------------------------------------------------------------------------
//
// Name:		rock_throw_think
// Description: what the rock projectile is thinking
// Input:
// Output:
// Note:        called after delay
//
// ----------------------------------------------------------------------------

void rock_throw_think( userEntity_t *self )
{
	gstate->RemoveEntity(self);
}


// ----------------------------------------------------------------------------
//
// Name:		rock_throw_touch
// Description: what happens when the venom projectile touches an entity
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void rock_throw_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	com->Damage( other, self, self->owner, self->s.origin, self->velocity, self->health, DAMAGE_INERTIAL );

	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//
// Name:		rock_throw
// Description: tosses a rock at the enemy
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void rock_throw(userInventory_s *ptr, userEntity_t *self)
{
	ai_weapon_t *weapon = (ai_weapon_t *) self->curWeapon;
	userEntity_t *rock = ai_fire_projectile( self, self->enemy, weapon, "models/global/e_rock3.dkm",
	                                         rock_throw_touch, 0, NULL );
	float xydist = VectorXYDistance( self->s.origin, self->enemy->s.origin );

	// find time to hit enemy
	float t = xydist / weapon->speed;

	// calculate upward velocity from time to hit enemy
	rock->velocity.z = (self->enemy->s.origin.z - (self->s.origin.z + weapon->ofs.z)) / t -
	                   0.5f * -p_gravity->value * t;
	// Note: It would look cooler if the rock was aimed at the enemy's view location,
	// but Shawn told me to aim at the enemy's origin (more consistent, I guess). --zjb

	rock->movetype = MOVETYPE_TOSS;
	rock->s.render_scale.Set(1.0, 1.0, 1.0);
	rock->s.alpha = 1.0;

	rock->health = (float)weapon->base_damage + ((float)weapon->rnd_damage * rnd());

	rock->delay = gstate->time + ROCK_THROW_LIFETIME;

	rock->think = rock_throw_think;
	rock->nextthink = gstate->time + ROCK_THROW_LIFETIME;
}


/* ******************************** medusa spit ****************************** */

// maximum length of time that Medusa's spit lives, in seconds
#define MEDUSA_SPIT_LIFETIME 3.0

// ----------------------------------------------------------------------------
//
// Name:		medusa_spit_think
// Description: what the spit is thinking
// Input:
// Output:
// Note:        called after delay
//
// ----------------------------------------------------------------------------

void medusa_spit_think( userEntity_t *self )
{
	gstate->RemoveEntity(self);
}


// ----------------------------------------------------------------------------
//
// Name:		medusa_spit_touch
// Description: what happens when the spit touches an entity
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

void medusa_spit_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	com->Damage( other, self, self->owner, self->s.origin, self->velocity, self->health, DAMAGE_INERTIAL );

	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//
// Name:		medusa_spit
// Description: spit happens
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void medusa_spit(userInventory_s *ptr, userEntity_t *self)
{
	ai_weapon_t *weapon = (ai_weapon_t *) self->curWeapon;
	userEntity_t *spit = ai_fire_projectile( self, self->enemy, weapon, "models/e1/me_sludge.dkm",
		venom_spit_touch, 0, NULL );
	
	spit->s.render_scale.Set(0.15f, 0.15f, 0.15f);
	gstate->SetSize(spit, -3, -3, -3, 3, 3, 3);
	spit->s.effects = EF_POISON;
	spit->s.alpha	= 1.0;
	spit->owner		= self;
	spit->health = (float)weapon->base_damage + ((float)weapon->rnd_damage * rnd());
	
	spit->delay = gstate->time + VENOM_SPIT_LIFETIME;
	spit->s.origin.z += 10.0f;
	spit->think = venom_spit_think;
	spit->nextthink = gstate->time + VENOM_SPIT_LIFETIME;
	
	gstate->LinkEntity(spit);

	//Create the Special FX for the spawning
	trackInfo_t tinfo;
	// clear this variable
	memset(&tinfo, 0, sizeof(tinfo));

	tinfo.ent			=spit;
	tinfo.srcent		=spit;
	tinfo.fru.Zero();
	tinfo.Long1			= ART_FROGINATOR_SPIT;
	tinfo.flags			= TEF_FXFLAGS|TEF_SRCINDEX|TEF_LONG1|TEF_ALTPOS|TEF_ALTPOS2;
	tinfo.fxflags		= TEF_ARTIFACT_FX|TEF_FX_ONLY;
	// NSS[2/16/00]:Greenish
	tinfo.altpos.Set(0.30f,0.85f,0.05f);	// First Color (smaller Particles)
	tinfo.altpos2.Set(0.20f,0.55f,0.01f);	//Second Color (larger Particles)

	com->trackEntity(&tinfo,MULTICAST_ALL);	
}


/* ************************************ fireball ************************************** */
// ----------------------------------------------------------------------------
//
// Name:		fireball_explode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void fireball_explode( userEntity_t *self)
{
	
	
	spawn_sprite_explosion( self, self->s.origin, true );

	if(self->s.render_scale.x == 1.0f &&  self->s.render_scale.y == 1.0f  && self->s.render_scale.z == 1.0f )
	{
		CVector Flippy;
		Flippy = self->s.origin;
		Flippy.x += 40.0f;
		spawn_sprite_explosion( self, Flippy, true );	
		com->RadiusDamage( self, self->owner, self->owner, self->health*0.30, DAMAGE_EXPLOSION, 64.0 );
		Flippy = self->s.origin;
		Flippy.x -= 40.0f;
		spawn_sprite_explosion( self, Flippy, true );
		com->RadiusDamage( self, self->owner, self->owner, self->health*0.30, DAMAGE_EXPLOSION, 64.0 );
		Flippy = self->s.origin;
		Flippy.y -= 40.0f;
		spawn_sprite_explosion( self, Flippy, true );	
		com->RadiusDamage( self, self->owner, self->owner, self->health*0.30, DAMAGE_EXPLOSION, 64.0 );
		Flippy = self->s.origin;
		Flippy.y += 40.0f;
		spawn_sprite_explosion( self, Flippy, true );	
		com->RadiusDamage( self, self->owner, self->owner, self->health*0.30, DAMAGE_EXPLOSION, 64.0 );
		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/e_explodef.wav"), 
								 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/e_explodeq.wav"), 
								 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/e_exploded.wav"), 
								 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
	}
	else
	{
		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/e_explodeh.wav"), 
								 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
	}
	
	//do radius dammage
	com->RadiusDamage( self, self->owner, self->owner, self->health, DAMAGE_EXPLOSION, 64.0 );

	//remove the tracked entities
	RELIABLE_UNTRACK(self);

	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//
// Name:		fireball_think
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void fireball_think( userEntity_t *self )
{
	self->s.frame++;
	if ( self->s.frame > 4 )
	{
		self->s.frame = 0;
	}

	if ( self->delay <= gstate->time )
	{
		gstate->RemoveEntity(self);
	}
	else
	{
		self->nextthink = gstate->time + 0.1;
	}
	
	self->s.origin.pitch++;
}

// ----------------------------------------------------------------------------
//
// Name:		fireball_touch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void fireball_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	fireball_explode( self );
}

// ----------------------------------------------------------------------------
//
// Name:		fireball_attack
// Description:Fireball for the Doombat
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void fireball_attack( userInventory_s *ptr, userEntity_t *self )
{
	ai_weapon_t	*weapon = (ai_weapon_t *) ptr;

	userEntity_t *fireball = ai_fire_projectile( self, self->enemy, weapon, "models/e3/we_fball.dkm",
	                                             fireball_touch, 0, NULL );

	//Setup the trail and the Light
	fireball->s.effects = EF_BOLTTRAIL;

	fireball->health = (float)weapon->base_damage + ((float)weapon->rnd_damage * rnd());

	fireball->delay = gstate->time + 5.0;

	fireball->think = fireball_think;
	fireball->nextthink = gstate->time + gstate->frametime;

	fireball->owner = self;
	fireball->avelocity.Set( 0.0, 0.0, 200.0 );
	
	AIMDATA_PTR pAimData = ai_aim_curWeapon( self, weapon );
	gstate->SetSize(fireball, 0, 0, 0, 0, 0, 0);
	gstate->SetOrigin2( fireball, pAimData->org );
	gstate->LinkEntity(fireball);

	//Setup special effects for the fireball
    trackInfo_t tinfo;

	
	if( strstr( self->className, "monster_knight1" ) )
	{
		// clear this variable
		memset(&tinfo, 0, sizeof(tinfo));

		tinfo.ent=fireball;
		tinfo.srcent=fireball;
		tinfo.fru.Zero();
		tinfo.lightColor.x = .95;//R
		tinfo.lightColor.y = .35;//G
		tinfo.lightColor.z = .15;//B
		tinfo.lightSize= 150;
		tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX;
		tinfo.fxflags = TEF_LIGHT;
		tinfo.scale2 = 20.0f;

		com->trackEntity(&tinfo,MULTICAST_PVS);

		fireball->s.render_scale.Set( 1.0f, 1.0f, 1.0f );
	}
	else if( strstr( self->className, "monster_dragon" ) )
	{
		CVector Dir;
		float Speed = fireball->velocity.Length();
		fireball->s.origin.z += 96.0f;
		Dir = self->enemy->s.origin - fireball->s.origin ;
		Dir.Normalize();
		fireball->velocity = Dir * Speed;
		

		fireball->s.render_scale.Set( 1.0f, 1.0f, 1.0f );
		fireball->s.alpha = 0.30;
		fireball->s.color.Set(0.85f,0.25f,0.05f);
		fireball->avelocity.roll = 200.0f;
		// clear this variable
		memset(&tinfo, 0, sizeof(tinfo));

		tinfo.ent					=fireball;
		tinfo.srcent				=fireball;
		tinfo.fru.Zero();
		tinfo.Long1					= ART_DRAGON_FIREBALL;
		tinfo.flags					= TEF_FXFLAGS|TEF_SRCINDEX|TEF_LONG1|TEF_ALTPOS|TEF_ALTPOS2|TEF_DSTPOS;
		tinfo.fxflags				= TEF_ARTIFACT_FX|TEF_FX_ONLY;
		tinfo.dstpos.Set( Dir );				//Used for the direction
		com->trackEntity(&tinfo,MULTICAST_ALL);
	}
	else
	{
		// clear this variable
		memset(&tinfo, 0, sizeof(tinfo));

		tinfo.ent=fireball;
		tinfo.srcent=fireball;
		tinfo.fru.Zero();
		tinfo.lightColor.x = .95;//R
		tinfo.lightColor.y = .35;//G
		tinfo.lightColor.z = .15;//B
		tinfo.lightSize= 150;
		tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX;
		tinfo.fxflags = TEF_LIGHT;
		tinfo.scale2 = 20.0f;
		fireball->s.render_scale.Set(0.15f,0.15f,0.15f);
	}

    gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/e_firetraveld.wav"), 
                              1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
}





/* ************************************ FrogSpit ************************************** */

// ----------------------------------------------------------------------------
//
// Name:		FrogSpit_Attack
// Description:Initiating Sequence to a Spittle Attack!
// Input:userInventory_s *ptr, userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void FrogSpit_Attack( userInventory_s *ptr, userEntity_t *self )
{	
	ai_weapon_t *weapon = (ai_weapon_t *) self->curWeapon;
	userEntity_t *spit = ai_fire_projectile( self, self->enemy, weapon, "models/e1/me_sludge.dkm",
		venom_spit_touch, 0, NULL );
	
	spit->s.render_scale.Set(0.15f, 0.15f, 0.15f);
	gstate->SetSize(spit, -3, -3, -3, 3, 3, 3);
	spit->s.effects = EF_POISON;
	spit->s.alpha	= 1.0;
	spit->owner		= self;
	spit->health = (float)weapon->base_damage + ((float)weapon->rnd_damage * rnd());
	
	spit->delay = gstate->time + VENOM_SPIT_LIFETIME;
	spit->s.origin.z += 10.0f;
	spit->think = venom_spit_think;
	spit->nextthink = gstate->time + VENOM_SPIT_LIFETIME;
	
	gstate->LinkEntity(spit);

	//Create the Special FX for the spawning
	trackInfo_t tinfo;
	// clear this variable
	memset(&tinfo, 0, sizeof(tinfo));

	tinfo.ent			=spit;
	tinfo.srcent		=spit;
	tinfo.fru.Zero();
	tinfo.Long1			= ART_FROGINATOR_SPIT;
	tinfo.flags			= TEF_FXFLAGS|TEF_SRCINDEX|TEF_LONG1|TEF_ALTPOS|TEF_ALTPOS2;
	tinfo.fxflags		= TEF_ARTIFACT_FX|TEF_FX_ONLY;
	// NSS[2/16/00]:Greenish
	tinfo.altpos.Set(0.30f,0.85f,0.05f);	// First Color (smaller Particles)
	tinfo.altpos2.Set(0.20f,0.55f,0.01f);	//Second Color (larger Particles)

	com->trackEntity(&tinfo,MULTICAST_ALL);	
}


/* ************************************ Wyndrax's Wisps ************************************** */

//---------------------------------------------------------------------------
// directLine()  Something that might not need to be here <check later> <nss>
//---------------------------------------------------------------------------
int directLine(userEntity_t *src,userEntity_t *dst)
{
    CVector         org, end;

    end = ((dst->absmin + dst->absmax) * 0.5) + dst->view_ofs;
    org = src->s.origin + src->view_ofs;

    gstate->TraceLine (org, end, TRUE, src, &trace);

    if ( trace.fraction == 1.0 || trace.ent == dst )
    {
        org = org - trace.endpos;

        // return the distance to target
        return	org.Length();
    }

    return(0);
}

// ----------------------------------------------------------------------------
// NSS[10/29/99]:
// Name:        Wisp_Personality_Generator
// Description
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
float Wisp_Personality_Generator(float P)
{
	float New;

	//These buggers are pretty happy who they are...but sometimes they like changing.
	if(rnd() > 0.75f)
	{
		//But when they do they get a new personality...
		New = rnd();
		if(New < 0.50f)
		{
			New = 0.50f;
		}
		if(P < 0.0f)
		{
			New *= -1;
		}
	}
	else
	{
		New = P;
	}
	
	return New;
}


// ----------------------------------------------------------------------------
// NSS[10/29/99]:
// Name:		sineMove 
// Description:A modified version of the previous sineMove
// Input:	userEntity_t *self  --> what we want to move around crazy like
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void sineMove(userEntity_t *self)
{
    wyndraxHook_t *hook=(wyndraxHook_t *)self->userHook;
    CVector vec,Dir;
	int count;
	float fDistance = VectorDistance(self->enemy->s.origin,self->s.origin);
	
	Dir = self->enemy->s.origin - self->s.origin;

	Dir.z += 10 + (rnd()*50);
	
	Dir.Normalize();

	Dir = Dir * 150;

	count = hook->sinofs;
	if(hook->Personality < 0.0)
	{
		count = 11 - count;
	}
	
	if(abs((int)Dir.x) > abs((int)Dir.y))
	{
		Dir.y += cos_tbl[count] * (100*hook->Personality);
		
	}
	else
	{
		Dir.x += cos_tbl[count] * (-100*hook->Personality);
	}
	
	
	Dir.z += sin_tbl [count] * (100*hook->Personality);
   
    //This code prevents the wisps from getting over the player's head.
	if(fDistance < 64.0f)
	{
		Dir.x = Dir.x * -1;
		Dir.y = Dir.y * -1;
	}	
	else if(fDistance < 100.0)
	{
		Dir.x = 0.0f;
		Dir.y = 0.0f;
	}
		
	self->velocity = Dir;

    hook->sinofs++;
    if ( hook->sinofs > 11 )
	{
        hook->sinofs=0;
		hook->Personality = Wisp_Personality_Generator(hook->Personality);
	}
}

// ----------------------------------------------------------------------------
//NSS[10/12/99]:
// Name:		removeLightning 
// Description:Function to remove lightning
// Input:	userEntity_t *self  --> us... the pure energy bolt of ass stomping
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void removeLightning(userEntity_t *self)
{
    wyndraxHook_t *whook=(wyndraxHook_t *)self->curWeapon;
    lightningHook_t *hook=(lightningHook_t *)self->userHook;

    // find ent in lightning list
    for ( int i=0; i<WYNDRAX_MAX_LIGHTNING; i++ )
        if ( whook->lightningList[i]==self )
        {
            whook->lightningList[i]=NULL;
            whook->lightningCount--;
            break;
        }

        // dummy ent?
    if ( hook->dst>=whook->dummyList && hook->dst<=&whook->dummyList[WYNDRAX_MAX_DUMMY] )
        hook->dst->inuse=false;

    RELIABLE_UNTRACK(self);
    gstate->RemoveEntity(self);
}


// ----------------------------------------------------------------------------
//NSS[10/12/99]:
// Name:		lightningThink 
// Description:think for lightning
// Input:	userEntity_t *self  --> us... the pure energy bolt of ass stomping
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void lightningThink(userEntity_t *self)
{
    lightningHook_t *hook=(lightningHook_t *)self->userHook;
	wyndraxHook_t *whook = (wyndraxHook_t *)hook->owner->userHook;

	_ASSERTE( hook );
    CVector dir;
	char buffer[30];
	int Letter_Offset;

	// damage dst if dst's owner doesn't have a like owner
    bool isDummy = (hook->dst>=whook->dummyList && hook->dst<=&whook->dummyList[WYNDRAX_MAX_DUMMY] );
    if (hook && whook &&  (hook->dst != self->owner) && !isDummy)
    {
        dir = hook->src->s.origin - self->s.origin;
		com->Damage(hook->dst,self,self->owner,hook->dst->s.origin,dir,2.0,DAMAGE_INERTIAL | DAMAGE_SPARKLES | DAMAGE_CHECK_INVENTORY);
    }
	//Generate random noise
	Letter_Offset = (int)((rnd()*3.0f)+97.0f);
//	sprintf(buffer,"e3/we_wwispcordite%c.wav",Letter_Offset);
	Com_sprintf(buffer,sizeof(buffer),"e3/we_wwispcordite%c.wav",Letter_Offset);
	//Make noise
	gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex(buffer),0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );
   
     // kill self?
    if ( gstate->time >= hook->killtime || !directLine(hook->src,hook->dst) ||  hook->src->deadflag!=DEAD_NO || hook->dst->deadflag!=DEAD_NO )
    {
		removeLightning(self);
        return;
    }

    // keep lightning linked to src
    self->s.origin = hook->src->s.origin;
    gstate->LinkEntity(self);
	self->nextthink=gstate->time+.1;
}


// ----------------------------------------------------------------------------
//NSS[10/12/99]:
// Name:		spawnLightning
// Description:Lightning spawn function
// Input:	userEntity_t *owner --->owner spawning the lightning
//			userEntity_t *src	--->point of expulsion
//			userEntity_t *dst	--->point of contact
//			float killtime		--->time to live time to die
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *spawnLightning(userEntity_t *owner,userEntity_t *src, userEntity_t *dst, float killtime, CVector Color)
{
    wyndraxHook_t *whook=(wyndraxHook_t *)owner->userHook;
    lightningHook_t *hook;
    userEntity_t *bolt;
    trackInfo_t tinfo;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    // find an open slot in wyndrax hook
    // (it's filled in at bottom of func)
	int i = 0;
    for (i=0; i<WYNDRAX_MAX_LIGHTNING; i++ )
        if ( !whook->lightningList[i] )
            break;

	if (i == WYNDRAX_MAX_LIGHTNING)
		return NULL;
        // spawn bolt
    bolt=gstate->SpawnEntity();
    bolt->className="wyndrax bolt";
    bolt->movetype=MOVETYPE_NONE;
    bolt->solid=SOLID_NOT;
    bolt->s.renderfx=RF_TRACKENT|RF_LIGHTNING;          // drawn by client
    bolt->s.effects=EF_GIB;           // not really GIBS, makes server send to clients
    bolt->s.frame=4;                  // beam diameter
    bolt->s.frameInfo.frameFlags = FRAME_FORCEINDEX;
    bolt->s.skinnum=0xa1a2a3a4;       // beam color
    bolt->s.alpha=.6;
    bolt->s.origin = src->s.origin;
    bolt->owner=owner->owner;           // basically, the player
    gstate->LinkEntity(bolt);

    bolt->think=lightningThink;
    bolt->nextthink=gstate->time+.1;
    bolt->remove=removeLightning;
    bolt->curWeapon=(userInventory_t *)owner->userHook;

	bolt->userHook = gstate->X_Malloc(sizeof(lightningHook_t),MEM_TAG_HOOK);

	// SCG[11/24/99]: Save game stuff
	bolt->save = lightning_hook_save;
	bolt->load = lightning_hook_load;
	
    hook=(lightningHook_t *)bolt->userHook;
    hook->killtime=gstate->time;
	hook->killtime += killtime;
    hook->owner=owner;
    hook->src=src;
    hook->dst=dst;

    //NSS[11/12/99]:
	// msg client to track this entity
    tinfo.ent			= bolt;
    tinfo.srcent		= src;
    tinfo.dstent		= dst;
	tinfo.altangle		= Color;
	tinfo.altpos2.x		= 4.0; //Size of bolt
	tinfo.altpos2.y		= 1.0f;//Modulation
	tinfo.flags = TEF_SRCINDEX|TEF_FXFLAGS|TEF_ALTANGLE|TEF_ALTPOS2;
    tinfo.renderfx = RF_LIGHTNING|RF_TRANSLUCENT;

    if (dst>=whook->dummyList && dst<=&whook->dummyList[WYNDRAX_MAX_DUMMY] )
    {
        tinfo.flags |= TEF_DSTPOS;
        tinfo.dstpos = dst->s.origin;
    }
    else
    {
        tinfo.flags |= TEF_DSTINDEX|TEF_FRU;
        tinfo.fru.Zero();
    }
    com->trackEntity(&tinfo,MULTICAST_PVS);

    // wisp keeps track of all lightning bolts it's spawned
    whook->lightningList[i]=bolt;
    whook->lightningCount++;

    return(bolt);
}

// ----------------------------------------------------------------------------
//
// Name:		removeWisp
// Description:Make sure we remove all attached lightning and then the wisp
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void removeWisp(userEntity_t *self)
{
    wyndraxHook_t *hook=(wyndraxHook_t *)self->userHook;
	playerHook_t *owner_hook = (playerHook_t *)self->owner->userHook;
    
	//<NSS> This is a hack, but since all monsters at this point are using the entire
	//playerHook_t structure I am going to use vars that I KNOW monsters will never use
	//instead of creating more shit to throw into the playerHook_t struct.  What we
	//really need to do is have a monsterHook_t structure with generic vars in them and
	//all vars with NO purpose get axed!  This should happen AFTER the demo.
	
	//Kill the wisp counter.
	//First make sure Wyndrax isn't dead	
	if(owner_hook)
	{
		owner_hook->base_power--;
	}

	// remove any lightning with this wisp as the src
    for ( int i=0; i<WYNDRAX_MAX_LIGHTNING; i++ )
	{
        if ( hook->lightningList[i] )
        {
            lightningHook_t *lhook=(lightningHook_t *)hook->lightningList[i]->userHook;
			if(lhook)
			{
				if (!stricmp(hook->lightningList[i]->className,"wyndrax bolt"))
				{
					if ( lhook->src==self )
						hook->lightningList[i]->remove(hook->lightningList[i]);
				}
			}
        }
	}
	gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e3/we_wwispaway.wav"),0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );
    RELIABLE_UNTRACK(self);
	gstate->RemoveEntity(self);
	
}


// ----------------------------------------------------------------------------
// NSS[10/29/99]:
// Name:		wisFade
// Description:Process to make the wisp fade out into nothing
// Input: userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void wispFade(userEntity_t *self)
{
	self->s.alpha -= 0.05;

	self->s.render_scale.x -= 0.10f;
	self->s.render_scale.y -= 0.10f;
	//For weird fade out into 'dense' space effect.
	if(self->s.alpha < 0.50)
	{
		self->s.render_scale.z -= 0.20f;
	}
	else
	{
		self->s.render_scale.z += 0.10f;
	}
	sineMove(self);
	if(self->s.alpha < 0.001f)
	{
		removeWisp(self);
	}

	self->nextthink=gstate->time+.1;
}


// ----------------------------------------------------------------------------
// NSS[10/29/99]:
// Name:		wisPain
// Description:If we manage to hit one of these bad motha's they can die
// Input:userEntity_t *self, userEntity_t *attacker, float kick, int damage
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------

void wispPain(userEntity_t *self, userEntity_t *attacker, float kick, int damage)
{
	wyndraxHook_t *hook=(wyndraxHook_t *)self->userHook;
	
	if(self->owner != attacker)
	{
		//reduce the health by the amount of damage done
		self->health -= damage;
		if(self->health <=0)
		{
			//zero out our velocity;
			self->velocity.Zero();
			
			//change our think to the wispFade function
			self->think = wispFade;
			//In case we take anymore damage let's reset our damage related shit
			self->pain = NULL;
			self->takedamage = DAMAGE_NO;

			if(hook->Personality < 0.0f)
			{
				hook->Personality = -1.0f;
			}
			else
			{
				hook->Personality = 1.0f;
			}

			self->nextthink=gstate->time+.1;

		}
	}
}

// ----------------------------------------------------------------------------
// NSS[10/29/99]:
// Name:		WyndraxWispTouch
// Description:Touch function for the wisps
// Input:userEntity_t *self -->The wisp
//		userEntity_t *other -->The thing we touched
//		cplane_t *plane     -->The plane
//      csurface_t *surf    -->The surface of the plane
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void wyndraxWispTouch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
    wyndraxHook_t *hook=(wyndraxHook_t *)self->userHook;
    CVector vec;

    // calc reflection vector
    if(plane)
	{
		float dp=hook->forward.x*plane->normal[0] + hook->forward.y*plane->normal[1] + hook->forward.z*plane->normal[2];
	    CVector normal(plane->normal);
	    normal=normal*dp;
	   vec=hook->forward-2*normal;

		// put this in angles
		VectorToAngles(vec,vec);
		self->s.angles = vec;

		// calc new forward / up vectors
		vec=self->s.angles;
		vec.AngleToVectors(forward,right,up);
		hook->forward=forward;
		hook->up=up;
		hook->sinetime=gstate->time+.2;
	}
}

// ----------------------------------------------------------------------------
// NSS[10/29/99]:
// Name:		wyndraxWispThink
// Description:Think function for the wisps themselves
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void wyndraxWispThink(userEntity_t *self)
{
    wyndraxHook_t *hook=(wyndraxHook_t *)self->userHook;
    short randomZap=true;
	CVector C1,C2,Color;
	C1.Set(0.10,0.20,0.75);
	C2.Set(0.20,0.10,0.85);

    
	CVector Angles;
	CVector dir = self->enemy->s.origin - self->s.origin;
	dir.Normalize();
	
	VectorToAngles(dir,Angles);
	self->s.angles = Angles;	
	
	// time to die?
    if ( gstate->time >= hook->killtime || !(AI_IsAlive(self->owner)))
    {
 		//change our think to the wispFade function
		self->think = wispFade;
		//In case we take anymore damage let's reset our damage related shit
		self->pain = NULL;
		self->takedamage = DAMAGE_NO;

		if(hook->Personality < 0.0f)
		{
			hook->Personality = -1.0f;
		}
		else
		{
			hook->Personality = 1.0f;
		}

		self->nextthink=gstate->time+.1;
        return;
    }

    // move in sine wave
    if ( gstate->time >= hook->sinetime )
        sineMove(self);

    
	float fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );
	// zap player?
	//Randomly zap the player for now and make sure we don't exceed the max wisps.
	//We should be able to see the player and should be fairly close to the player.
    if ( (rnd() > 0.55f) && (hook->lightningCount < WYNDRAX_MAX_LIGHTNING) && (fDistance < 200.0f) && AI_IsVisible(self,self->enemy))
    {
		Color = Color.Interpolate(C1,C2,hook->sinofs/12);
		spawnLightning(self,self,self->enemy,0.2f, Color);
    }

    // randomly zap environment?
    if ( randomZap && hook->dummyCount < WYNDRAX_MAX_DUMMY )
    {
        // randomly spawn lightning to points in the world
        CVector vec,ang,start,end;
        trace_t tr;

        vec=CVector(crand(),crand(),crand());
        VectorToAngles(vec, ang);
        ang.AngleToVectors(forward,right,up);
        start=self->s.origin;
        end=start+forward*1000;
        tr=gstate->TraceLine_q2(start, end,self,MASK_SHOT);

        if ( tr.ent==gstate->FirstEntity() )
        {
            userEntity_t *dummy;

            // find empty dummy ent
            for ( int i=0; i<WYNDRAX_MAX_DUMMY; i++ )
                if ( !hook->dummyList[i].inuse )
                    break;

                // setup dummy ent
            dummy=&hook->dummyList[i];
            dummy->deadflag=DEAD_NO;
            dummy->inuse=true;
            dummy->s.origin = tr.endpos;
			if(dummy != self->enemy)
			{
				Color = Color.Interpolate(C1,C2,hook->sinofs/12);
				spawnLightning(self,self,dummy,0.2f, Color);
			}
        }
    }

    self->nextthink=gstate->time+.1;
}




// ----------------------------------------------------------------------------
// NSS[10/29/99]:
// Name:		WyndraxWisp_Attack
// Description:Initiating Wyndrax Launch Wisp!
// Input:userInventory_s *ptr, userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void WyndraxWisp_Attack(userInventory_s *ptr, userEntity_t *self )
{
    ai_weapon_t *invWeapon = (ai_weapon_t *) self->curWeapon;
	
	playerHook_t *phook=(playerHook_t *)self->userHook;
    userEntity_t *wisp;
    wyndraxHook_t *hook;
    CVector vec;

	wisp = ai_fire_projectile( self, self->enemy, invWeapon, "models/e3/we_wisp.dkm", wyndraxWispTouch, EF_WISP_TRAIL, NULL );
	wisp->s.origin.Set(self->s.origin.x-10,self->s.origin.y+10,self->s.origin.z+22);

	wisp->className		= "monster_wisp";
	wisp->enemy			= self->enemy;
	wisp->s.frame		= 0;
    wisp->think			= wyndraxWispThink;
    wisp->nextthink		= gstate->time+.1;
    wisp->remove		= removeWisp;
    wisp->movetype		= MOVETYPE_BOUNCEMISSILE;
    wisp->solid			= SOLID_BBOX;
	wisp->owner			= self;
	wisp->clipmask		= MASK_SOLID;

	// lets link the thing
	gstate->LinkEntity(wisp);

	//Setup the size and scale
    gstate->SetSize(wisp,-1,-1,-1,1,1,1);
	wisp->s.render_scale.Set(2.0f, 2.0f, 2.0f);


	//Set alpha, scale, and bounding region
	wisp->s.alpha = 1.0f;
 	
	//Set the health
	wisp->takedamage = DAMAGE_YES;
	wisp->health = 10.0;
	wisp->pain = wispPain;
	
	
	//Setup initial velocity towards the player	
	CVector Dir;
	Dir = self->enemy->s.origin - self->s.origin;
	Dir.Normalize();
	wisp->velocity = Dir *(500 + rnd() * 500);

	wisp->userHook = gstate->X_Malloc(sizeof(wyndraxHook_t),MEM_TAG_HOOK);

	// SCG[11/24/99]: Save game stuff
	wisp->save = wyndrax_hook_save;
	wisp->load = wyndrax_hook_load;

    hook=(wyndraxHook_t *)wisp->userHook;
    
	
	//Let's setup a lifetime for the Wisps... say like 20 seconds
	hook->killtime=gstate->time + 20.0;
    
	//Setup static information pertaining to wisp's movement etc..
	hook->sinofs=0;
    hook->sinetime=0;
	hook->Personality = rnd();
	if( rnd() > 0.50f)
	{
		hook->Personality *= -1;
	}


    vec=self->s.angles;
    vec.AngleToVectors(forward,right,up);
    hook->forward=forward;
    hook->up=up;
	gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e3/we_wwispshoota.wav"),0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );
	
	//Increment the wisp counter
	phook->base_power++;


	//Attach the Sprite Light&Glow stuff (add track entity)
    trackInfo_t tinfo;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=wisp;
    tinfo.srcent=wisp;
    tinfo.fru.Zero();
    tinfo.lightColor.x = 0.35;//R
    tinfo.lightColor.y = 0.35;//G
    tinfo.lightColor.z = 0.75;//B
    tinfo.lightSize= 175.00;
	
	tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_MODELINDEX|TEF_SCALE;
    tinfo.fxflags = TEF_LIGHT|TEF_SPRITE;
	tinfo.renderfx = SPR_ALPHACHANNEL;
	tinfo.scale = 1.0f + (rnd()*0.5);
	tinfo.modelindex = gstate->ModelIndex ("models/global/e_sflblue.sp2");
	
	com->trackEntity(&tinfo,MULTICAST_PVS);
}

/* ************************************ Wyndrax's Lightning ************************************** */

// ----------------------------------------------------------------------------
// NSS[10/29/99]:
// Name:		ZapThink 
// Description:think for lightning
// Input:	userEntity_t *self  --> us... the pure energy bolt of ass stomping
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void ZapThink(userEntity_t *self)
{
    lightningHook_t *hook=(lightningHook_t *)self->userHook;
    CVector dir;
	//char buffer[30];
//	int Letter_Offset;

	
	// damage dst if dst's owner doesn't have a like owner
    if ( hook->dst != self->owner && self->owner != NULL)
    {
        dir = hook->src->s.origin - self->s.origin;
		//(userEntity_t *inflictor, userEntity_t *attacker, userEntity_t *ignore,	float damage, unsigned long damage_flags, float fRadius );
		
		// cek[12-1-99] parm 2 is attacker (wyndrax) and 3 is ignore (wyndrax)
		com->RadiusDamage(hook->dst,self->owner,self->owner,5.0,DAMAGE_INERTIAL | DAMAGE_SPARKLES | DAMAGE_CHECK_INVENTORY,90.0f);
//		com->RadiusDamage(hook->dst,self,self->owner->owner,5.0,DAMAGE_INERTIAL | DAMAGE_SPARKLES | DAMAGE_CHECK_INVENTORY,90.0f);
		//com->Damage(hook->dst,self,self->owner,hook->dst->s.origin,dir,2.0,DAMAGE_INERTIAL | DAMAGE_SPARKLES);
    }
	//Generate random noise
//	Letter_Offset = (int)((rnd()*3.0f)+97.0f);
    //Make noise
	//gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex(buffer),1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );

     // kill self?
    if ( gstate->time >= hook->killtime || !directLine(hook->src,hook->dst) ||  hook->src->deadflag!=DEAD_NO || hook->dst->deadflag!=DEAD_NO )
    {
        RELIABLE_UNTRACK(self);
		
		//Kill Target Entity
		gstate->RemoveEntity(hook->dst);
		//Kill Lightning
//		gstate->RemoveEntity(self);
		removeLightning(self);

		AI_Dprintf("Lightning Entity Remove!\n");
        return;
    }
 	self->nextthink=gstate->time+.1;
}


// ----------------------------------------------------------------------------
//
// Name:		spawnZap
// Description:Lightning spawn function
// Input:	userEntity_t *owner --->owner spawning the lightning
//			userEntity_t *src	--->point of expulsion
//			userEntity_t *dst	--->point of contact
//			float killtime		--->time to live time to die
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
userEntity_t *spawnZap(userEntity_t *owner,CVector src, userEntity_t *dst, CVector dest_origin, float killtime)
{
    wyndraxHook_t *whook=(wyndraxHook_t *)owner->userHook;
    lightningHook_t *hook;
    userEntity_t *bolt;
    trackInfo_t tinfo;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));
	AI_Dprintf("Lightning alive\n");
    // find an open slot in wyndrax hook
    // (it's filled in at bottom of func)
    for ( int i=0; i<WYNDRAX_MAX_LIGHTNING; i++ )
        if ( !whook->lightningList[i] )
            break;
	
	if (i == WYNDRAX_MAX_LIGHTNING)
		return NULL;

    // spawn bolt
    bolt=gstate->SpawnEntity();
    bolt->className="wyndrax bolt";
    bolt->movetype=MOVETYPE_NONE;
    bolt->solid=SOLID_NOT;
    bolt->s.renderfx=RF_TRACKENT|RF_LIGHTNING;          // drawn by client
    bolt->s.effects=EF_GIB;           // not really GIBS, makes server send to clients
    bolt->s.frame=4;                  // beam diameter
    bolt->s.frameInfo.frameFlags = FRAME_FORCEINDEX;
    bolt->s.skinnum=0xa1a2a3a4;       // beam color
    bolt->s.alpha=.6;
    bolt->s.origin = src;
    bolt->owner=owner->owner;           // basically, Wyndrax
    gstate->LinkEntity(bolt);

    bolt->think=ZapThink;
    bolt->nextthink=gstate->time+.1;
    bolt->remove=removeLightning;
    bolt->curWeapon=(userInventory_t *)owner->userHook;

	bolt->userHook = gstate->X_Malloc(sizeof(lightningHook_t),MEM_TAG_HOOK);

	// SCG[11/24/99]: Save game stuff
	bolt->save = lightning_hook_save;
	bolt->load = lightning_hook_load;

    hook=(lightningHook_t *)bolt->userHook;
    hook->killtime=gstate->time+killtime;
    hook->owner=owner;
    hook->src=owner;
    
	//Create temp entity to zap...
	hook->dst= gstate->SpawnEntity();
	hook->dst->svflags &= SVF_NOCLIENT;
	hook->dst->s.origin = dst->s.origin;
	gstate->LinkEntity(hook->dst);

	hook->altdst = dst;

	// msg client to track this entity
    tinfo.ent=bolt;
    tinfo.srcent=bolt;
    tinfo.dstent=dst;
    tinfo.dstpos = dest_origin;
	tinfo.altangle.Set(0.25,0.45,0.85); //Set color
	
	tinfo.altpos2.x	= 12.0; //Size of bolt
	tinfo.altpos2.y	= 0.75f;//Modulation

	tinfo.flags = TEF_SRCINDEX|TEF_FXFLAGS|TEF_ALTANGLE|TEF_DSTPOS|TEF_ALTPOS2;
    tinfo.renderfx = RF_LIGHTNING|RF_TRANSLUCENT;
   
	com->trackEntity(&tinfo,MULTICAST_PVS);

//	tinfo.lightColor.Set(0.35,0.45,0.75);
//	tinfo.lightSize = 200.0f;
//	tinfo.flags = TEF_SRCINDEX|TEF_DSTPOS | TEF_LENGTH|TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS;
//	tinfo.fxflags = TEF_LIGHT;

    
	// wisp keeps track of all lightning bolts it's spawned
    whook->lightningList[i]=bolt;
    whook->lightningCount++;

    return(bolt);
}


// ----------------------------------------------------------------------------
//
// Name:		removeZap
// Description:Make sure we remove all attached lightning and then the wisp
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void removeZap(userEntity_t *self)
{
    wyndraxHook_t *hook=(wyndraxHook_t *)self->userHook;
//	playerHook_t *owner_hook = (playerHook_t *)self->owner->userHook;// SCG[1/23/00]: not used
    
	// remove any lightning with this wisp as the src
    for ( int i=0; i<WYNDRAX_MAX_LIGHTNING; i++ )
	{
        if ( hook->lightningList[i] )
        {
            lightningHook_t *lhook=(lightningHook_t *)hook->lightningList[i]->userHook;
			if (lhook)
			{
				if ( lhook->src==self )
				{
					RELIABLE_UNTRACK(hook->lightningList[i]);
					//com->untrackEntity(hook->lightningList[i],NULL,MULTICAST_ALL_R);
					gstate->RemoveEntity(hook->lightningList[i]);
					//hook->lightningList[i]->remove(hook->lightningList[i]);
					AI_Dprintf("Lightning Entity Remove!\n");
				}
			}
        }
	}
   	AI_Dprintf("Zap Entity Remove!\n");

	RELIABLE_UNTRACK(self);
	gstate->RemoveEntity(self);
}



// ----------------------------------------------------------------------------
//
// Name:		wyndraxZapThink
// Description:Think function for the wisps themselves
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void wyndraxZapThink(userEntity_t *self)
{
    wyndraxHook_t *hook=(wyndraxHook_t *)self->userHook;
	CVector Offset,Target_Angles,Forward;
//	short randomZap=true;

    CVector Angles;
	CVector dir = self->enemy->s.origin - self->s.origin;
	dir.Normalize();
	
	VectorToAngles(dir,Angles);
	self->s.angles = Angles;	
	
	// time to die?
    if (gstate->time >= hook->killtime)
    {
		removeZap(self);
        return;
    }
//	float fDistance = VectorDistance( self->s.origin, self->enemy->s.origin );// SCG[1/23/00]: not used
	
	// zap player!
	if(hook->dummyCount < 4)
	{
		//Calculate offset for hand that rotates with model... 
		//Just as good as hard points. <nss>
		Offset = self->enemy->s.origin - self->s.origin;
		Offset.Normalize();
		VectorToAngles(Offset,Target_Angles);
		Target_Angles.yaw -= 35;
		Target_Angles.AngleToForwardVector(Forward);
		Offset = (Forward * 15)+self->s.origin;
		Offset.z +=10.0f;
		//Spawn the lightning
		spawnZap(self,Offset,self->enemy,hook->forward,0.25);
		//We are close enough to the hand(a little high)
		Offset.z -=3.0f;
		//Spawn flare.
		spawnZapFlare(self,&Offset,0.15,2.15f,"models/global/e_flblue.sp2");
		
		//Calculate offset for staff that rotates with model... 
		//Almost as good as hard points. <nss>
		Offset = self->enemy->s.origin - self->s.origin;
		Offset.Normalize();
		VectorToAngles(Offset,Target_Angles);
		Target_Angles.yaw +=45;
		Target_Angles.AngleToForwardVector(Forward);
		Offset = (Forward * 40)+self->s.origin;
		Offset.z +=15.0f;
		spawnZap(self,Offset,self->enemy,hook->forward,0.25);
		Offset = self->enemy->s.origin - self->s.origin;
		Offset.Normalize();
		VectorToAngles(Offset,Target_Angles);
		Target_Angles.yaw +=45;
		Target_Angles.AngleToForwardVector(Forward);
		Offset = (Forward * 24)+self->s.origin;
		Offset.z +=15.0f;
		spawnZapFlare(self,&Offset,0.15, 2.15f, "models/global/e_flblue.sp2");

		hook->dummyCount+=2;
		
	}
    self->nextthink=gstate->time+.1;
}


// ----------------------------------------------------------------------------
//
// Name:		WyndraxZap_Attack
// Description:The first of the two attacks for Wyndrax
// Input:userInventory_s *ptr, userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void WyndraxZap_Attack(userInventory_s *ptr, userEntity_t *self )
{
    wyndraxHook_t *hook;
	userEntity_t *Zap;
	

	Zap = gstate->SpawnEntity();
	Zap->movetype	= MOVETYPE_NONE;
	Zap->solid		= SOLID_NOT;
	Zap->owner		= self;
	Zap->s.frame	= 0;
	
	


	Zap->s.origin = self->s.origin;
	Zap->s.origin.z +=24;
	Zap->className		= "Wyndrax_Zap";
	Zap->enemy			= self->enemy;
	Zap->s.frame		= 0;
    Zap->think			= wyndraxZapThink;
    Zap->nextthink		= gstate->time + 0.1f;
    Zap->remove			= removeZap;
	Zap->clipmask		= MASK_SHOT;

	Zap->userHook = gstate->X_Malloc(sizeof(wyndraxHook_t),MEM_TAG_HOOK);

	// SCG[11/24/99]: Save game stuff
	Zap->save = wyndrax_hook_save;
	Zap->load = wyndrax_hook_load;

    hook=(wyndraxHook_t *)Zap->userHook;
	

	hook->killtime		= gstate->time + 0.55f;
	//Store the origin of the player.
	hook->forward		= self->enemy->s.origin;

	gstate->LinkEntity(Zap);
	AI_Dprintf("Zap Entity alive\n");
}




// ----------------------------------------------------------------------------
//
// Name:        weapon_DoDamage
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void punch_DoDamage( userEntity_t *self, userEntity_t *pEnemy, ai_weapon_t *weapon )
{
	CVector dir = pEnemy->s.origin - self->s.origin;
	dir.Normalize();

	float fDamage = (float) weapon->base_damage + rnd() * float (weapon->rnd_damage);
	com->Damage( pEnemy, self, self, pEnemy->s.origin, dir, fDamage, 0);
}


/* ************************************ Stavros' Stave ************************************** */
//NSS[10/29/99]:
//---------------------------------------------------------------------------
//meteor_explode()
//NSS[10/27/99]: The remove function for the fragments
//---------------------------------------------------------------------------
void meteor_explode( userEntity_t *self, cplane_t *plane, userEntity_t *other )
{
	staveHook_t *hook=(staveHook_t *)self->userHook;
	
	//Make the explosion
    CVector color(.85,.35,.15);
    spawnPolyExplosion(self->s.origin, plane->normal, 1, 250, color, PEF_NORMAL|PEF_SOUND);
	spawn_sprite_explosion( self, self->s.origin, true );

	//Make the scorch mark
	weaponScorchMark2(self,other,plane);

	//do radius dammage
	com->RadiusDamage( self, self->owner, self->owner, self->health, DAMAGE_EXPLOSION|DAMAGE_CHECK_INVENTORY, hook->damage );

	//remove the tracked entities
	RELIABLE_UNTRACK(self);

	//Stop the looping sound
	gstate->StartEntitySound( self, CHAN_LOOP, gstate->SoundIndex("global/e_firetravela.wav"), 0.0f, 0.0f, 0.0f );

	gstate->RemoveEntity(self);
}

//---------------------------------------------------------------------------
//Meteor_touch()
//NSS[10/27/99]:modified the touch function for fragments
//---------------------------------------------------------------------------
void meteor_touch(userEntity_t *self,userEntity_t *other,cplane_t *plane,csurface_t *surf)
{
	staveHook_t *hook=(staveHook_t *)self->userHook;
	if(other != self && other != self->owner && hook->bounce_count >= hook->bounce_max )
		meteor_explode( self, plane, other );
	else
		hook->bounce_count++;
}

//---------------------------------------------------------------------------
//Meteor_think()
//NSS[10/27/99]:Went ahead and added a check to make sure it gets destroyed 
//no matter what.
//---------------------------------------------------------------------------
void meteor_think(userEntity_t *self)
{
	//NSS[10/27/99]:Why set the renderfx to 0?
	staveHook_t *hook=(staveHook_t *)self->userHook;
	
	if(hook->time_to_live < gstate->time || self->velocity.Length() < 10.0f)
	{
		RELIABLE_UNTRACK(self);
		gstate->RemoveEntity(self);
	}
	else
	{
		self->s.angles.roll		+= hook->roll_speed;
		self->s.angles.pitch	+= hook->pitch_speed;	
		self->s.angles.yaw		+= hook->yaw_speed;
	}
	self->nextthink = gstate->time + 0.1;
}


//---------------------------------------------------------------------------
//MeteorFragmentSpawn()
//NSS[10/27/99]:Spawn function for fragments
//---------------------------------------------------------------------------
void MeteorFragmentSpawn(CVector vDirectional, CVector Origin, int Damage, userEntity_t * owner)
{
	userEntity_t	*temp;
	CVector			org, end, dir, ang, p_org;

	temp = gstate->SpawnEntity();
	temp->movetype	= MOVETYPE_BOUNCE;
	temp->solid		= SOLID_BBOX;
	temp->s.effects = 0;
	temp->owner		= owner;
	temp->s.frame	= 0;

	temp->clipmask	= MASK_SHOT;
	temp->svflags = SVF_SHOT;         //	won't clip against any ents with clipmask == MASK_SHOT
	temp->s.modelindex = gstate->ModelIndex( "models/e3/we_meteor.dkm" );
	
	gstate->SetSize(temp, -5, -5,-5, 5, 5, 5);
	

	Origin.x	+=	crand() * 25.0f;
	Origin.y	+=	crand() * 25.0f;
	Origin.z	+=	crand() * 25.0f;
	
	gstate->SetOrigin2( temp, Origin );

	temp->velocity = vDirectional;

	//	align model in direction of travel
	VectorToAngles( temp->velocity, ang );
	temp->s.angles	= ang;

	temp->touch		= meteor_touch;

	temp->think = meteor_think;
	temp->nextthink = gstate->time + gstate->frametime;

	float Scale_Base = (rnd()* 0.35) + .30;
	temp->s.render_scale.Set(Scale_Base+(rnd()*0.20f),Scale_Base+(rnd()*0.20f),Scale_Base+(rnd()*0.20f));
	
	gstate->LinkEntity( temp );

	//Attach the Sprite Light&Glow and fire stuff 
    trackInfo_t tinfo;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=temp;
    tinfo.srcent=temp;
    tinfo.fru.Zero();
    tinfo.lightColor.x = 0.85;//R
    tinfo.lightColor.y = 0.35;//G
    tinfo.lightColor.z = 0.15;//B
    tinfo.lightSize= 125.00;	
	
	tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_MODELINDEX|TEF_SCALE;
    tinfo.fxflags = TEF_LIGHT|TEF_SPRITE|TEF_STAVROS;
	tinfo.renderfx = SPR_ALPHACHANNEL;
	tinfo.scale = 1.2f * (Scale_Base * 0.65f);
	tinfo.modelindex = gstate->ModelIndex ("models/e3/we_fglow.sp2");
	
	com->trackEntity(&tinfo,MULTICAST_PVS);


	//NSS[10/27/99]: Let's create a user hook specificly for the meteor
	temp->userHook = gstate->X_Malloc(sizeof(staveHook_t),MEM_TAG_HOOK);

	// SCG[11/24/99]: Save game stuff
	temp->save = stave_hook_save;
	temp->load = stave_hook_load;

    staveHook_t *hook=(staveHook_t *)temp->userHook;

	//set a +/- random value for pitch and roll
	hook->pitch_speed = crand() * 30.0f;
	hook->roll_speed = crand() * 30.0f;

	//Base Damage on how big they are.
	hook->damage = (Scale_Base/0.25f)*Damage;

	//Set a self destruct time(6 seconds roughly)
	hook->time_to_live = gstate->time + 12.0f;

	//Set the current bounce count
	hook->bounce_count = 0;

	//Set the random maximum bounce count
	hook->bounce_max = 2 + rnd()*3;
	
//	gstate->StartEntitySound( temp, CHAN_LOOP, gstate->SoundIndex("global/e_travela.wav"), 1.0f, 128.0f, 256.0f );

}


// ----------------------------------------------------------------------------
//NSS[10/27/99]:
// Name:         stave_destroy
// Description:This will remove the main Huge meteor and will make little tiny baby
// meteors... awe.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void stave_destroy( userEntity_t * self, cplane_t *plane, userEntity_t *other)
{
	staveHook_t *hook=(staveHook_t *)self->userHook;
	CVector Reflect,Target_Angles,Forward;
	float Magnitude = self->velocity.Length();
	
	//get the current velocity
	Reflect = self->velocity;
	Reflect.Normalize();
	//Invert it(point backwards)
	Reflect.Negate();
	
	
	int Frags = 4+(int)(rnd()*3);
	//Spawn multiple fragments
	for(int i=0; i < Frags ;i++)
	{
		//Convert the angles
		VectorToAngles(Reflect,Target_Angles);

		//Let's add about a 30 degree spread from the origin of the returning vector
		Target_Angles.yaw += crand()*45;
		Target_Angles.pitch += crand()*45;

		Target_Angles.AngleToForwardVector(Forward);

		Forward = Forward * (Magnitude*1.85);		
		
		MeteorFragmentSpawn( Forward, self->s.origin, hook->damage, self->owner);
	}
	

	//gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/e_explode1.wav"), 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );

	//Make the explosion
    CVector color(0.85,0.35,0.15);
	spawnPolyExplosion(self->s.origin, plane->normal,2, 450, color, PEF_NORMAL|PEF_SOUND);
	spawn_sprite_explosion( self, self->s.origin, true );
	//Make the scorch mark
	weaponScorchMark2(self,other,plane);

	//do radius dammage
	com->RadiusDamage( self, self->owner, self->owner, self->health, DAMAGE_EXPLOSION|DAMAGE_CHECK_INVENTORY, hook->damage );

	//remove the tracked entities
	RELIABLE_UNTRACK(self);

	//Stop the looping sound
	gstate->StartEntitySound( self, CHAN_LOOP, gstate->SoundIndex("global/e_firetravele.wav"), 0.0f, 0.0f, 0.0f );
	
	gstate->RemoveEntity(self);
	
}


//---------------------------------------------------------------------------
// touch()
//---------------------------------------------------------------------------
void stave_touch(userEntity_t *self,userEntity_t *other,cplane_t *plane,csurface_t *surf)
{
	if(other != self && other != self->owner)
		stave_destroy( self, plane, other );
}

//---------------------------------------------------------------------------
// think()
//NSS[10/27/99]:Went ahead and added a check to make sure it gets destroyed 
//no matter what.
//---------------------------------------------------------------------------
void stave_think(userEntity_t *self)
{
	staveHook_t *hook=(staveHook_t *)self->userHook;
	
	if(hook->time_to_live < gstate->time)
	{
		gstate->RemoveEntity(self);
	}
	else
	{
		//Keep it turning
		self->s.angles.roll		+= hook->roll_speed;
		self->s.angles.pitch	+= hook->pitch_speed;	
		self->s.angles.yaw		+= hook->yaw_speed;

		//Warp in handling here
		if(self->s.render_scale.x < 1.0f)
		{
			//We are warping in here thus change the size to make the appearance of depth
			self->s.render_scale.x += 0.05f;
			self->s.render_scale.y += 0.05f;
			self->s.render_scale.z += 0.05f;

			//As we change size let's slow the turn rate down
			if(hook->roll_speed > 5.0f)
			{
				hook->roll_speed	-= 15.0f;
				hook->pitch_speed	-= 15.0f;
				hook->yaw_speed		-= 15.0f;
			}
			
			if( self->velocity.Length() < hook->speed_max)
			{
				if (self->velocity.Length() < hook->speed_max * 0.20f)
					self->velocity = self->velocity * 1.18f;				
				else
					self->velocity = self->velocity * 1.35;	
			}
		}
		
	}
	
	
	self->nextthink = gstate->time + 0.1;
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		StavrosZapFlareRotateThink
// Description:Flare for Wyndrax's attack
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void StavrosFlareThink(userEntity_t *self)
{
	ZapFlare_t *hook	= (ZapFlare_t *)self->userHook;
	if(hook->time_to_live < gstate->time)
	{
		//remove the tracked entities
		RELIABLE_UNTRACK(self);
		gstate->RemoveEntity(self);
	}
	if(self->s.render_scale.x > 0.80)
	{
		hook->delta_scale = - 0.07f;
	}
	else if(self->s.render_scale.x < 0.10)
	{
		hook->delta_scale = 0.0f;
	}
	self->s.render_scale.x	+= hook->delta_scale;
	self->s.render_scale.y	+= hook->delta_scale;
	self->s.render_scale.z	+= hook->delta_scale;
	self->s.angles			+= hook->Delta_Angles;
	
	self->nextthink		= gstate->time + 0.1f;

}

//---------------------------------------------------------------------------
// Stave Attack
//NSS[10/27/99]: Modified Stave Attack for Stavros
//ehum.. added a few things here and there.  :)
//---------------------------------------------------------------------------
void stave_attack( userInventory_s *ptr, userEntity_t *self )
{
	ai_weapon_t	*weapon = (ai_weapon_t *) ptr;
	CVector Offset, Target_Angles, Forward, Scale, Rotate;
	trackInfo_t tinfo;

	userEntity_t *pStave = ai_fire_projectile( self, self->enemy, weapon, "models/e3/we_meteor.dkm",
	                                             stave_touch, 0, NULL );

	//We are warping in from inner space through a worm hole created by
	//electromagnetic shockwaves rippling time and the fabric of space.
	//The meteor should start off looking small and then end up looking 
	//large
	
	///////////////////////Meteor Code Starts Here///////////////////////
	pStave->s.render_scale.Set(0.2f,0.2f,0.2f);
	pStave->health = weapon->base_damage;
	pStave->s.renderfx=0;
	pStave->svflags = SVF_SHOT;         //	won't clip against any ents with clipmask == MASK_SHOT
	pStave->think = stave_think;
	pStave->nextthink = gstate->time + gstate->frametime;

	pStave->s.frame = 0;
	gstate->SetSize(pStave,-12,-12,-12,12,12,12);

	pStave->owner	= self;	

	Offset = self->enemy->s.origin - self->s.origin;
	Offset.Normalize();

	VectorToAngles(Offset,Target_Angles);

	Target_Angles.yaw	+= 35;
	Target_Angles.pitch -= 45;

	Target_Angles.AngleToForwardVector(Forward);

	pStave->s.origin = (Forward * 25)+self->s.origin;
	pStave->s.origin.z += 25.0f;

	//We want to start off nice and slow
	pStave->velocity.Normalize();
	pStave->velocity =  pStave->velocity * (weapon->speed*0.05);

	//Link the bitch	
	gstate->LinkEntity( pStave );

	//Attach the Sprite Light&Glow and fire stuff 


    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent			=pStave;
    tinfo.srcent		=pStave;
    tinfo.fru.Zero();
    tinfo.lightColor.x	= 0.55;//R
    tinfo.lightColor.y	= 0.35;//G
    tinfo.lightColor.z	= 0.15;//B
    tinfo.lightSize		= 100.00;
	
	tinfo.flags			= TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_MODELINDEX|TEF_SCALE;
    tinfo.fxflags		= TEF_LIGHT|TEF_SPRITE|TEF_STAVROS;
	tinfo.renderfx		= SPR_ALPHACHANNEL;
	tinfo.scale			= 2.75f;
	tinfo.modelindex	= gstate->ModelIndex ("models/e3/we_fglow.sp2");
	
	com->trackEntity(&tinfo,MULTICAST_PVS);
	
	
	//NSS[10/27/99]: Let's create a user hook specificly for the meteor
	pStave->userHook = gstate->X_Malloc(sizeof(staveHook_t),MEM_TAG_HOOK);

	// SCG[11/24/99]: Save game stuff
	pStave->save = stave_hook_save;
	pStave->load = stave_hook_load;

    staveHook_t *hook=(staveHook_t *)pStave->userHook;

	//Save the max velocity
	hook->speed_max = weapon->speed;

	//set a +/- random value for pitch and roll
	hook->pitch_speed	= crand() * 40.0f;
	hook->roll_speed	= crand() * 40.0f;
	hook->yaw_speed		= crand() * 40.0f;
	
	//Let's set the damage based off of the CSV file
	hook->damage = weapon->base_damage;

	//Set a self destruct time(6 seconds roughly)
	hook->time_to_live = gstate->time + 12.0f;

	////////////////////Special FX for launching Starts Here////////////////////
	
	CVector *Origin = &pStave->s.origin;
	Scale.Set(0.1f,0.1f,0.1f);
	Rotate.Set(0.0f,0.0f,15.0f);
	//spawnZapFlareRotate(userEntity_t *self, CVector *Origin, CVector Rotate,CVector Size,float life, char *flarename)
	userEntity_t *ent	= spawnZapFlareRotate(self, Origin,Rotate,Scale, 0.9f, "models/e3/we_blackhole.sp2");
    ent->s.alpha		= 0.75f;
	ent->s.frame		= 10;
	ent->think			=  StavrosFlareThink;

	ZapFlare_t *zHook	= (ZapFlare_t *) ent->userHook;
	zHook->delta_scale	= 0.35f;

	// clear this variable
    memset(&tinfo, 0, sizeof(tinfo));
	//Create the negative light FX to simulate the wormhole'ish thingy going down

    tinfo.ent			= ent;
    tinfo.srcent		= ent;
    tinfo.fru.Zero();
    tinfo.lightColor.x	= -1.0;//R
    tinfo.lightColor.y	= -.5;//G
    tinfo.lightColor.z	= -.5;//B
    tinfo.lightSize		= 300.0;
	tinfo.length		= 05.0f;			//Set the rate to shrink the light
	
	tinfo.flags			= TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_LENGTH;
    tinfo.fxflags		= TEF_LIGHT_SHRINK;
	
	com->trackEntity(&tinfo,MULTICAST_PVS);

	gstate->StartEntitySound( pStave, CHAN_LOOP, gstate->SoundIndex("global/e_torchd.wav"),0.85f, 128.0f, 512.0f );	

}


/* ************************************ ThunderSkeet's JiZ ************************************** */


//---------------------------------------------------------------------------
// touch()
//---------------------------------------------------------------------------
void ThunderSpray_Touch(userEntity_t *self,userEntity_t *other,cplane_t *plane,csurface_t *surf)
{
	if(other != self && other != self->owner)
	{
		//do radius dammage
		printf("BANG!\n");
		com->RadiusDamage( self, self, self, 40.0f, DAMAGE_EXPLOSION, 256.0f );
		//remove the tracked entities
		RELIABLE_UNTRACK(self);
		gstate->RemoveEntity(self);
	}
}


// ----------------------------------------------------------------------------
// NSS[10/29/99]:
// Name:	ThunderSpray_Think
// Description:The think function for the ThunderSpray  "Spray them Down!"
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void ThunderSpray_Think( userEntity_t *self)
{
	CVector Forward;
	ThunderJizHook_t *hook=(ThunderJizHook_t *)self->userHook;
	if(hook->time_to_live < gstate->time)
	{
		gstate->RemoveEntity(self);
	}
	else
	{
		if(self->s.render_scale.x < 0.30f)
		{
			hook->delta_scale = 0.25f;
		}
		else if(self->s.render_scale.x > 0.85f)
		{
			if(hook->type)
				gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/e_warploopb.wav"), 0.25f, 128.0f, 512.0f );	
			hook->delta_scale = -0.25f;
		}
		//We are warping in here thus change the size to make the appearance of depth
		self->s.render_scale.x += hook->delta_scale;
		self->s.render_scale.y += hook->delta_scale;
		self->s.render_scale.z += hook->delta_scale;	

	}
	
	CVector Dir = self->velocity;
	Dir.Normalize();

	//Sine Movement for the glob of green goo
	if(hook->type)
	{
		//if(Dir.x > Dir.y)
		//{
		//	self->velocity.y += amplitude * sin_tbl [(int)hook->delta_angles];
		//}
		//else
		//{
			self->velocity.x += 10 * sin_tbl [(int)hook->delta_angles];
			self->velocity.y += 5 * sin_tbl [(int)hook->delta_angles];
		//}
		//self->velocity.z += amplitude/2 * cos_tbl[(int)hook->delta_angles];

	}
	else
	{
	//	if(Dir.x > Dir.y)
	//	{
	//		self->velocity.y += amplitude * cos_tbl [(int)hook->delta_angles];
	//	}
	//	else
	//	{
			self->velocity.x += 10 * cos_tbl [(int)hook->delta_angles];
			self->velocity.y += 5 * cos_tbl [(int)hook->delta_angles];
	//	}
		//self->velocity.z += amplitude * sin_tbl[(int)hook->delta_angles];
	}
	
	hook->delta_angles += 1;
	if(hook->delta_angles > 12)
		hook->delta_angles = 0;

	self->nextthink = gstate->time + 0.1;
}



// ----------------------------------------------------------------------------
// NSS[10/29/99]:
// Name:	ThunderJiz
// Description:This is the ThunderSkeet's jiz spray attack
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void ThunderJiz_attack( userInventory_s *ptr, userEntity_t *self )
{
	playerHook_t *phook = AI_GetPlayerHook( self );
	ai_weapon_t	*weapon = (ai_weapon_t *) ptr;
	CVector Offset, Target_Angles,Forward;
	trackInfo_t tinfo;
	userEntity_t *spray;
	
	weapon->speed = 128.0f + (rnd()*64.0f);
	spray = ai_fire_projectile( self, self->enemy, weapon, "models/global/e_flyellow.sp2",ThunderSpray_Touch, 0, NULL );
	//spray code starts here
	spray->movetype		= MOVETYPE_BOUNCEMISSILE;
	spray->owner		= self;
	spray->s.frame		= 0;
	spray->clipmask		= CONTENTS_PLAYERCLIP|CONTENTS_SOLID;
	spray->s.renderfx	|= (SPR_ALPHACHANNEL);
	spray->health		= (float)weapon->base_damage + ((float)weapon->rnd_damage * rnd());

	gstate->SetSize(spray, -1, -1, -1, 1, 1, 1);
	
	//Set the origin for the main shit stream exit hole thingy.
	//Offset = self->enemy->s.origin - self->s.origin;
	//Offset.Normalize();

	//VectorToAngles(Offset,Target_Angles);

	//if(phook->acro_boost)
	//{
	//	Target_Angles.yaw	+= 25;
	//}
	//else
	//{
	//	Target_Angles.yaw	-= 25;
	//}

	//Target_Angles.AngleToForwardVector(Forward);
	//Offset = (Forward * (20.0f+(rnd()*4.0f))) + self->s.origin;
	Offset = self->s.origin;
	Offset.z	-= 40.0f;

	gstate->SetOrigin2( spray, Offset );

	spray->s.render_scale.Set(1.4f,1.4f,1.4f);

	spray->think		= ThunderSpray_Think;
	spray->nextthink	= gstate->time + 0.2f;

	spray->s.alpha		= 0.45f;

	//Link the bitch	
	gstate->LinkEntity( spray );
	
	//NSS[10/27/99]: Let's create a user hook specificly for the ThunderJiz
	spray->userHook = gstate->X_Malloc(sizeof(ThunderJizHook_t),MEM_TAG_HOOK);

	// SCG[11/24/99]: Save game stuff
	spray->save = thunder_jiz_hook_save;
	spray->load = thunder_jiz_hook_load;

	ThunderJizHook_t *hook=(ThunderJizHook_t *)spray->userHook;

	hook->type			= (int)phook->acro_boost;
	hook->time_to_live	= gstate->time + 15.0f;
	hook->delta_scale	= -0.25;

	
	Offset = spray->s.origin;


	// clear this variable
	memset(&tinfo, 0, sizeof(tinfo));
	
	//Create the negative light FX to simulate the funky jiz light
	tinfo.ent			= spray;
	tinfo.srcent		= spray;
	tinfo.fru.Zero();
	tinfo.lightColor.x	= 0.15;//R
	tinfo.lightColor.y	= 0.95;//G
	tinfo.lightColor.z	= 0.35;//B
	tinfo.lightSize		= 400.0;
	tinfo.length		= 5.0f;			//Set the rate to shrink the light
	
	tinfo.flags			= TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_LENGTH;
	tinfo.fxflags		= TEF_LIGHT_SHRINK|TEF_THUNDERSKEET_FX;
	
	com->trackEntity(&tinfo,MULTICAST_PVS);

	if(phook->acro_boost)
		gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/e_warploopb.wav"), 0.25f, 128.0f, 512.0f );

}

int AI_ComputeBestAwayYawPoint(userEntity_t *self, CVector &flyAwayPoint, int Distance, int Resolution);
// ----------------------------------------------------------------------------
// NSS[12/17/99]:
// Name:        nharre_summon
// Description: The new and improved summoning function for Nharre.
// Input: userInventory_s *ptr, userEntity_t *self
// Output:NA
// Note:
// 
// ----------------------------------------------------------------------------
void nharre_summon( userInventory_s *ptr, userEntity_t *self )
{
	// summons a buboid from one of several set points in the map
	CVector AwayPoint;
	char Buffer[32];
	
	//AI_ComputeBestAwayYawPoint(self, AwayPoint,256.0f, 0.15f);
	SPAWN_FindNearSpawnPoint( self, AwayPoint );

	int Type = (int)(rnd()*3);

	// NSS[12/17/99]:Get the monster
	switch(Type)
	{
		case 0:
			{
				Com_sprintf(Buffer,sizeof(Buffer),"monster_buboid");
				break;
			}
		case 1:
			{
				Com_sprintf(Buffer,sizeof(Buffer),"monster_doombat");
				break;
			}
		default:
			{
				Com_sprintf(Buffer,sizeof(Buffer),"monster_plague_rat");
				break;
			}
	}

	// NSS[12/17/99]:Spawn the monster
	userEntity_t *pNewEntity = com->SpawnDynamicEntity( self, Buffer, true );
    if ( pNewEntity )
    {
	    int nEpairsCount = 0;
		while ( pNewEntity->epair[nEpairsCount].key )
		{
			
			if(self->epair)
			{
				if(!(_stricmp(self->epair[nEpairsCount].key,"spawnname")))
				{
					pNewEntity->epair[nEpairsCount].value = NULL;		
				}
			}
			if(self->epair)
			{
				if(!(_stricmp(self->epair[nEpairsCount].key,"deathtarget")))
				{
					pNewEntity->epair[nEpairsCount].value = NULL;		
				}
			}
			nEpairsCount++;
		}
		SPAWN_CallInitFunction( pNewEntity, Buffer );
		IncrementMonsterCount();
		pNewEntity->s.origin = AwayPoint;
		
		CVector Scale, Rotate, Dir, Location;

		Dir.Set(0.0,0.0,1.0);
		
		Scale.Set (1.0f,8.0f,10.0f);
		Rotate.Set(25.0f,15.0f,45.0f);
		Location = pNewEntity->s.origin;

		spawnZapFlareRotate(pNewEntity, &Location,Rotate, Scale,0.7f,"models/global/e_flred.sp2");
		
		gstate->WriteByte(SVC_TEMP_ENTITY);
		gstate->WriteByte(TE_SMOKE);
		gstate->WritePosition(Location);
		gstate->WriteFloat(10);
		gstate->WriteFloat(10);
		gstate->WriteFloat(5);
		gstate->WriteFloat(50);
		gstate->MultiCast(Location,MULTICAST_PVS);	

		if ( self->enemy )
		{
			AI_AddNewGoal( pNewEntity, GOALTYPE_KILLENEMY, self->enemy );
			CVector Dir,Angles;
			Dir = self->enemy->s.origin - pNewEntity->s.origin;
			Dir.Normalize();
			VectorToAngles(Dir,Angles);
			pNewEntity->s.angles.Set(0.0f,0.0f,0.0f);
			pNewEntity->s.angles.yaw = Angles.yaw;
			if(Type == 1)
			{
				AI_ForceSequence(pNewEntity,"flya",FRAME_LOOP);
			}
		}
		else
		{
			AI_AddNewGoal( pNewEntity, GOALTYPE_WANDER );
			if(Type == 1)
			{
				AI_ForceSequence(pNewEntity,"flya",FRAME_LOOP);
			}
		}
		
		// NSS[12/17/99]:Specific function for Buboid.
		if(Type == 0)
		{
			AI_AddNewGoal( pNewEntity, GOALTYPE_BUBOID_GETOUTOFCOFFIN );
		}
	}
}


// ----------------------------------------------------------------------------
//
// Name:        nharre_summon_demon
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

#define REAP_MAX_LOOKS        32.0        // 16.0
#define REAP_MIN_DIST         50
#define REAPER_ATTACK_SOUND   "e3/we_reaperattack2.wav"
#define REAPER_APPEAR_SOUND   "e3/we_reaperappear2.wav"
#define REAPER_WIND_SOUND     "e3/we_nharrewind.wav"
#define REAPER_DAMAGE         50

typedef struct lookInfo_s
{
    float dist;             // distance to obstruction from this angle
    float dist3;            // average dist of prev, current, and next angle
    CVector forward;         // forward vector for this angle
} lookInfo_t;

userEntity_t *reapervictim = NULL;
freezeEntity_t reaperfreezevictim;

void initReaperAttack(userEntity_t *self);
CVector findUnobstructedDirection(userEntity_t *ent);
void nightmareDeath(userEntity_t *self);
void nightmareScaredStiff(userEntity_t *self);
void nightmareAlmostDeath(userEntity_t *reaper);
void freezeEntity(userEntity_t *ent,freezeEntity_t *freeze);
void unfreezeEntity(userEntity_t *ent,freezeEntity_t *freeze);
void removeReaper(userEntity_t *self);


void nharre_summon_demon( userInventory_s *ptr, userEntity_t *self )
{
    userEntity_t *ent;

    CVector pos=self->s.origin;

    if ( !self->enemy || !AI_IsVisible(self, self->enemy) )
        return;

	reapervictim = self->enemy;

    // spawn grim raper
    ent=gstate->SpawnEntity();
    ent->owner=self;
    ent->movetype=MOVETYPE_NONE;
    ent->solid=SOLID_BBOX;
    ent->className="reaper";
    ent->s.modelindex=gstate->ModelIndex("models/e3/we_nnreaper.dkm");
	ent->s.render_scale.Set(0.001,0.001,0.001); // make him hide till we want him
    ent->s.renderfx=0;//RF_MINLIGHT|RF_FULLBRIGHT;
    ent->s.alpha=1.0;
    ent->remove=removeReaper;
    ent->curWeapon=NULL;

    initReaperAttack(ent);
    AddTrackLight(ent,300,.8,.4,.2,TEF_LIGHT);
}

//-----------------------------------------------------------------------------
//	light_think
//-----------------------------------------------------------------------------
void flame_light_think(userEntity_t *self)
{
	const float REAPER_TOP_FRAME = 14.0f;

	if ( !self->owner || !self->owner->inuse || (self->owner->s.frame >= (REAPER_TOP_FRAME + 5)) )
	{
		gstate->RemoveEntity(self);
		return;
	}

	self->s.render_scale.x += 15*crand();
	self->s.render_scale.y += 15*crand();
	self->nextthink = gstate->time + 0.2;
}

//-----------------------------------------------------------------------------
//	flamespawn
//-----------------------------------------------------------------------------
void FlameSpawn(userEntity_t *self)
{
	userEntity_t *light;
	// spawn light 

	light = gstate->SpawnEntity();
	// setup the light data
	trace_t tr;
	CVector start = self->absmin;
	CVector end = start;
	end.z -= 2000;

	// find the start position
	userEntity_t *ignore = self;
	while(1)
	{
		tr = gstate->TraceLine_q2 (start, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

		if (!tr.ent)
 			break;

		//	if we hit something that's not a monster or player or is immune to lasers, we're done
		if (!(tr.ent->flags & (FL_CLIENT + FL_BOT + FL_MONSTER)))
			break;

		ignore = tr.ent;
		start = tr.endpos;
	}
	light->s.origin = tr.endpos;

	// now, find the end position
	ignore = self->owner;
	start = self->absmin;
	end = start;
	end.z += 2000;
	while(1)
	{
		tr = gstate->TraceLine_q2 (start, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

		if (!tr.ent)
 			break;

		//	if we hit something that's not a monster or player or is immune to lasers, we're done
		if (!(tr.ent->flags & (FL_CLIENT + FL_BOT + FL_MONSTER)))
			break;

		ignore = tr.ent;
		start = tr.endpos;
	}

	light->s.render_scale = tr.endpos;

	light->movetype			= MOVETYPE_NONE;
	light->solid			= SOLID_NOT;
	light->s.modelindex		= gstate->ModelIndex("models/e1/me_cambotf.sp2");		// must be non-zero
	light->s.alpha			= 0.01;
	// set the position
	light->view_ofs			= self->view_ofs;
	// add a spotlight to this thing 
	light->s.renderfx		|= (RF_SPOTLIGHT|RF_TRANSLUCENT);//|RF_LIGHTFLARE);
	// set the spotlight color
	light->s.color.Set( 0.90f,0.20f ,0.10f );

	VectorToAngles(CVector(0,0,1),light->s.angles);
	
	// set the spotlight diameter
	light->s.frame	= 10;
	// set owner and enemy
	light->owner	= self;
	// setup thinking
	light->think	 = flame_light_think;
	light->nextthink = gstate->time + 0.1;
	// put this entity in the world
	gstate->LinkEntity(light);

	// make a cool scorch mark
	CVector dst = self->absmin;
	dst.z -= 2000;
	tr = gstate->TraceLine_q2( self->s.origin, dst, self, MASK_MONSTERSOLID);
	if (tr.ent)
	{
		gstate->WriteByte( SVC_TEMP_ENTITY );
		gstate->WriteByte( TE_SCORCHMARK );
		gstate->WritePosition( tr.endpos );
		gstate->WriteShort( tr.plane.planeIndex );
		gstate->WriteShort( tr.ent->s.modelindex );
		gstate->WriteShort( tr.ent->s.number);
		gstate->WriteFloat( 2 );
		gstate->WriteFloat( 0 );
		gstate->WriteByte( SM_EARTH );
		gstate->MultiCast( zero_vector, MULTICAST_ALL );
	}
}

//---------------------------------------------------------------------------
// initReaperAttack()
//---------------------------------------------------------------------------
void initReaperAttack(userEntity_t *self)
{
    CVector pos,angle;

    // ignore players in a warp or dead
    if ( !reapervictim || (reapervictim->flags & FL_INWARP) || (reapervictim->deadflag != DEAD_NO) )
    {
		reapervictim = NULL;
        return;
    }

    // get vector towards an open area of the map
    forward = findUnobstructedDirection(reapervictim);

	// play sounds second-to-last and last frames
    self->s.frameInfo.sound1Frame = 43 - 1;
    self->s.frameInfo.sound2Frame = 43;

	// set think function
	self->think = nightmareAlmostDeath;
	self->nextthink = gstate->time + 0.5;

    // set reaper position
    pos=reapervictim->s.origin+forward*100;
    self->s.origin = pos;

    // set reaper direction
    angle=-forward;
    VectorToAngles(angle,angle);
    self->s.angles = angle;

    self->s.renderfx=0;
    // link 'n lower
    gstate->LinkEntity(self);

    // freeze victim
	nightmareScaredStiff(reapervictim);
    freezeEntity(reapervictim,&reaperfreezevictim);

	reaperfreezevictim.old_owner = reapervictim->owner;
    reapervictim->owner=self->owner;

	gstate->StartEntitySound(self,CHAN_AUTO,gstate->SoundIndex(REAPER_APPEAR_SOUND),0.85f,ATTN_NORM_MIN,ATTN_NORM_MAX);
	gstate->StartEntitySound(self,CHAN_AUTO,gstate->SoundIndex(REAPER_WIND_SOUND),0.85f,ATTN_NORM_MIN,ATTN_NORM_MAX);

    // client gets extra attention
    if ( reapervictim->flags & FL_CLIENT )
    {
        playerHook_t *phook=(playerHook_t *)reapervictim->userHook;
        float delta,diff;

        pos=self->s.origin;
        angle=pos-reapervictim->s.origin;
        VectorToAngles(angle,angle);
        angle.x = 360-25;

        phook->force_angles=angle;

        BestDelta(reapervictim->s.angles[0],angle.x,&delta,&diff);
        phook->force_rate.x=200*delta;

        BestDelta(reapervictim->s.angles[1],angle.y,&delta,&diff);
        phook->force_rate.y=200*delta;

        phook->force_rate.z=200;

        gstate->SetClientAngles(reapervictim,angle);
    }
    else
    {
        // setup current anim state to show pain
        reapervictim->s.frameInfo.frameState |= FRSTATE_STOPPED;
    }
}


//---------------------------------------------------------------------------
// findUnobstructedDirection()
//---------------------------------------------------------------------------
CVector findUnobstructedDirection(userEntity_t *ent)
{
    lookInfo_t look[(int)REAP_MAX_LOOKS];

    CVector pos,end,angles,vec;
    short i,next,prev,bestangle;
    float rot,bestdist;

    // get distance from each angle
    rot=0;
    pos=ent->s.origin;
    pos.z += 10;

    angles=ent->s.angles;
    angles.x=0;

    for ( i=0; i<REAP_MAX_LOOKS; i++ )
    {
        angles.AngleToVectors(forward,right,up);
        look[i].forward=forward;

        end=pos+forward*300;
        gstate->TraceLine(pos,end,true,ent,&trace);
        vec=pos-trace.endpos;
        look[i].dist=vec.Length();

        rot += 360.0/REAP_MAX_LOOKS;
        angles.y += rot;
        if ( angles.y >= 360 )
            angles.y -= 360;
    }   

    // find best unobstructed angle
    bestangle=-1;
    bestdist=-1;
    for ( i=0; i<REAP_MAX_LOOKS; i++ )
    {
        prev=i-1;
        if ( prev < 0 )
            prev=REAP_MAX_LOOKS-1;

        next=i+1;
        if ( next==REAP_MAX_LOOKS )
            next=0;

        look[i].dist3=look[i].dist;      //(look[prev].dist+look[i].dist+look[next].dist)/3;
        if ( look[prev].dist >= REAP_MIN_DIST && look[i].dist >= REAP_MIN_DIST && look[next].dist >= REAP_MIN_DIST )
        {
            // best angle?
            if ( look[i].dist3 > bestdist )
            {
                bestangle=i;
                bestdist=look[i].dist3;
            }
        }
    }

    // hmm... reaper's getting pretty claustrophobic right about now...
    // find the largest avg distance, period!
    if ( bestangle==-1 )
	{
        for ( i=0; i<REAP_MAX_LOOKS; i++ )
        {
            prev=i-1;
            if ( prev < 0 )
                prev=REAP_MAX_LOOKS-1;

            next=i+1;
            if ( next==REAP_MAX_LOOKS )
                next=0;

            // best angle?
            if ( look[i].dist > bestdist )
            {
                bestangle=i;
                bestdist=look[i].dist3;
            }
        }
	}

    return look[bestangle].forward;
}


//---------------------------------------------------------------------------
// death()
//---------------------------------------------------------------------------
void nightmareAlmostDeath(userEntity_t *reaper)
{
	FlameSpawn( reaper );

	// rocks would be nice, but I'd have to duplicate the code
	// from the weapons DLL to get them

    trackInfo_t tinfo;
	memset(&tinfo,0,sizeof(tinfo));

	tinfo.ent=reaper;
    tinfo.srcent=reaper;

	// for the flame
	tinfo.renderfx = SPR_ALPHACHANNEL;
	tinfo.scale = 5.0;	// this needs updated as stuff happens!
	tinfo.scale2 = 0.8;	// alpha!
	tinfo.modelindex = gstate->ModelIndex ("models/global/we_nharref.sp2");

	// negative light technology
	tinfo.lightColor.Set(1,-1,-1);
	tinfo.lightSize = 250;
	tinfo.length = 3.0;
	
	// init the reaper!
	tinfo.Long1 |= TEF_REAPER_INIT;

    tinfo.flags=TEF_SRCINDEX|TEF_FXFLAGS|TEF_LIGHTSIZE|TEF_LIGHTCOLOR|TEF_LENGTH|TEF_MODELINDEX|TEF_SCALE|TEF_LONG1|TEF_SCALE2;
	tinfo.fxflags |= TEF_LIGHT_SHRINK|TEF_ALPHA_SPRITE|TEF_REAPER;
    com->trackEntity(&tinfo,MULTICAST_ALL);

	reaper->s.render_scale.Set(1,1,1); // make him hide till we want him

    // set frame / think info
    reaper->s.frame=0;
    reaper->s.frameInfo.startFrame=0;
    reaper->s.frameInfo.endFrame=43;
    reaper->s.frameInfo.frameFlags=FRAME_ONCE | FRAME_FORCEINDEX;
    reaper->s.frameInfo.frameInc=1;
    reaper->s.frameInfo.frameTime=FRAMETIME_FPS10;
    reaper->s.frameInfo.frameState=FRSTATE_STARTANIM;
    reaper->s.frameInfo.next_frameTime=0;
    reaper->s.frameInfo.sound1Frame = 42;
    reaper->s.frameInfo.sound2Frame = 43;

    reaper->think=nightmareDeath;
    reaper->nextthink=gstate->time+0.1;
}


//---------------------------------------------------------------------------
// death()
//---------------------------------------------------------------------------
void nightmareDeath(userEntity_t *self)
{
    short lastframe=43;
    CVector dir;

	// update our anim stuff and set the sound playing flags
    com->FrameUpdate(self);

    // is player that summoned reaper dead?
    if ( !self->owner || !self->owner->inuse ||
	     stricmp(self->owner->className,"freed") == 0 ||
	     self->owner->deadflag != DEAD_NO )
	{
		// that breaks the spell
		if ( reapervictim )
		{
			unfreezeEntity(reapervictim,&reaperfreezevictim);
			reapervictim->owner = reaperfreezevictim.old_owner;
		}

		self->think = NULL;
		self->remove( self );
		return;
    }

    // remove anyone that has died before reaper's had chance to smash him!
    if ( reapervictim && reapervictim->deadflag != DEAD_NO )
	{
		reapervictim=NULL;
	}

	// make monster show pain
	if ( reapervictim && reapervictim->deadflag==DEAD_NO && !(reapervictim->flags & FL_CLIENT) )
	{
//		playerHook_t *hook=(playerHook_t *)reapervictim->userHook;// SCG[1/23/00]: not used

		com->FrameUpdate(reapervictim);
		if ( reapervictim->s.frameInfo.frameState & FRSTATE_STOPPED && rnd()>.75 )
		{
			reapervictim->s.frameInfo.frameState=0;
			reapervictim->s.frameInfo.frameFlags|=FRAME_ONCE;
		}
	}

    // do damage near end of attack
    if ( self->s.frameInfo.frameState & FRSTATE_PLAYSOUND1 && reapervictim )
    {
        self->s.frameInfo.frameState &= ~FRSTATE_PLAYSOUND1;

		gstate->StartEntitySound(self,CHAN_AUTO,gstate->SoundIndex(REAPER_ATTACK_SOUND),0.85f,ATTN_NORM_MIN,ATTN_NORM_MAX);
        unfreezeEntity(reapervictim,&reaperfreezevictim);
		reapervictim->owner = reaperfreezevictim.old_owner;
        if ( reapervictim->flags & FL_CLIENT )
        {
            CVector pushdir,vel;

            pushdir=reapervictim->s.origin-self->s.origin;
            pushdir.Normalize();

            vel=reapervictim->velocity;
            vel=pushdir*1500;
            reapervictim->velocity = vel;

            reapervictim->groundEntity = NULL;
        }

        //	Nelno:	calculate vector from reaper to victim for good gibbage spread
        //	0, 0, -24 moves origin of reaper down a bit so gibs fly upwards
        dir = reapervictim->s.origin - (self->s.origin + CVector(0, 0, -24));
        dir.Normalize ();

		com->Damage(reapervictim,self,self,reapervictim->s.origin,dir,REAPER_DAMAGE,DAMAGE_EXPLOSION);
    }
    // remove self at end of attack
    else if ( self->s.frameInfo.frameState & FRSTATE_PLAYSOUND2 || !reapervictim )
    {
        self->s.frameInfo.frameState &= ~FRSTATE_PLAYSOUND2;

		self->remove(self);
		return;
    }

    self->nextthink=gstate->time+.1;
}


//-----------------------------------------------------------------------------
//  play a really cool scream sound!
//-----------------------------------------------------------------------------
void nightmareScaredStiff(userEntity_t *self)
{
    char path[256]={"\0"};
    char modelname[64]={"\0"};

	if( self->client == NULL )
	{
		return;
	}

    strncpy(modelname, Info_ValueForKey(self->client->pers.userinfo, "modelname"), 64);

    if(strstr(modelname, "mikiko"))
	{
//        sprintf(path, "mikiko/death8.wav");
        Com_sprintf(path, sizeof(path),"mikiko/death8.wav");
	}
    else if(strstr(modelname, "hiro"))
	{
//        sprintf(path, "hiro/death8.wav");
        Com_sprintf(path, sizeof(path),"hiro/death8.wav");
	}
    else if(strstr(modelname, "superfly"))
	{
//        sprintf(path, "superfly/death4.wav");
        Com_sprintf(path, sizeof(path),"superfly/death4.wav");
	}

    if(self->input_entity)
        gstate->StartEntitySound(self->input_entity, CHAN_BODY, gstate->SoundIndex(path), 1.0f, 256,1024);
    else
        gstate->StartEntitySound(self, CHAN_BODY, gstate->SoundIndex(path), 1.0f, 256,1024);
}


//---------------------------------------------------------------------------
// freezeEntity()
//---------------------------------------------------------------------------
void freezeEntity(userEntity_t *ent,freezeEntity_t *freeze)
{

    // save func pointers
    freeze->old_prethink=ent->prethink;
    freeze->old_think=ent->think;
    freeze->old_pain=ent->pain;
    freeze->old_die=ent->die;

    if ( ent->flags & FL_CLIENT )
    {
        // special client shit here
        playerHook_t *hook=(playerHook_t *)ent->userHook;

        gstate->SetCameraState(ent,true,CAMFL_LOCK_X+CAMFL_LOCK_Y+CAMFL_LOCK_Z);
        hook->angles_last_change=gstate->Sys_Milliseconds();
/*
        if ( hook->dflags & DFL_HIERARCHICAL )
        {
            if ( hook->legs )
                hook->legs->svflags |= SVF_NOTOWNER;
            if ( hook->torso )
                hook->torso->svflags |= SVF_NOTOWNER;
            if ( hook->head )
                hook->head->svflags |= SVF_NOTOWNER;
            if ( hook->weapon )
                hook->weapon->svflags |= SVF_NOTOWNER;
        }
        else
*/            ent->s.renderfx |= RF_NODRAW;

        ent->flags |= FL_FORCEANGLES;
        ent->viewheight=22;

        freeze->old_movetype=ent->movetype;
        freeze->old_velocity.Zero();
        freeze->old_avelocity.Zero();
		freeze->old_clipmask = ent->clipmask;

        ent->velocity.Zero();;
        ent->avelocity.Zero();
        ent->movetype=MOVETYPE_FLY;
    }
    else
    {
        // save old state
//      freeze->old_prethink=ent->prethink;
//      freeze->old_think=ent->think;
//      freeze->old_pain=ent->pain;
//      freeze->old_die=ent->die;
        freeze->old_movetype=ent->movetype;
        freeze->old_velocity = ent->velocity;
        freeze->old_avelocity = ent->avelocity;
		freeze->old_clipmask = ent->clipmask;

        // set new values
        ent->pain=NULL;
        ent->die=NULL;
        ent->prethink=NULL;
        ent->think=NULL;

//	  AI_StartFreeze( ent );         // mike: trouble compiling

        ent->velocity.Zero();
        ent->avelocity.Zero();
        ent->movetype=MOVETYPE_NONE;
        ent->clipmask=MASK_SOLID;
    }
}

//---------------------------------------------------------------------------
// unfreezeEntity()
//---------------------------------------------------------------------------
void unfreezeEntity(userEntity_t *ent,freezeEntity_t *freeze)
{
    playerHook_t *phook=(playerHook_t *)ent->userHook;

    // restore func pointers
    ent->prethink=freeze->old_prethink;
    ent->think=freeze->old_think;
    ent->pain=freeze->old_pain;
    ent->die=freeze->old_die;
	ent->clipmask = freeze->old_clipmask;

    if ( ent->flags & FL_CLIENT )
    {
        // special client shit here
        ent->client->ps.pmove.pm_type = PM_NORMAL;
        ent->flags &= ~FL_FORCEANGLES;
        gstate->SetCameraState(ent,false,0);

/*
        if ( phook->dflags & DFL_HIERARCHICAL )
        {
            if ( phook->legs )
                phook->legs->svflags &= ~SVF_NOTOWNER;
            if ( phook->torso )
                phook->torso->svflags &= ~SVF_NOTOWNER;
            if ( phook->head )
                phook->head->svflags &= ~SVF_NOTOWNER;
            if ( phook->weapon )
                phook->weapon->svflags &= ~SVF_NOTOWNER;
        }
        else
*/            ent->s.renderfx &= ~RF_NODRAW;

        ent->velocity = freeze->old_velocity;
        ent->avelocity = freeze->old_avelocity;
        ent->movetype= (movetype_t) freeze->old_movetype;
    }
    else
    {
        // restore old state
//      ent->prethink=freeze->old_prethink;
//      ent->think=freeze->old_think;
//      ent->pain=freeze->old_pain;
//      ent->die=freeze->old_die;

//	  AI_EndFreeze( ent );        // mike: trouble compiling..

        ent->movetype= (movetype_t ) freeze->old_movetype;
        ent->velocity = freeze->old_velocity;
        ent->avelocity = freeze->old_avelocity;
    }
}


//---------------------------------------------------------------------------
// removeReaper()
//---------------------------------------------------------------------------
void removeReaper(userEntity_t *self)
{
    RELIABLE_UNTRACK(self);
    gstate->RemoveEntity(self);
}


void garroth_summon( userInventory_s *ptr, userEntity_t *self )
{
	if ( !self->enemy )
	{
		return;
	}

	// summons a buboid near the player
	userEntity_t *pNewEntity = com->SpawnDynamicEntity( self, "monster_buboid", true );
    if ( pNewEntity )
    {
		int i = 0;
		while(pNewEntity->epair[i].key)
		{
			if(_stricmp(pNewEntity->epair[i].key,"deathtarget")==0)
			{
				pNewEntity->epair[i].value = NULL;
			}
			i++;
		}

		SPAWN_CallInitFunction( pNewEntity, "monster_buboid" );
		IncrementMonsterCount();
		// get vector towards an open area of the map
		forward = findUnobstructedDirection(self->enemy);

		pNewEntity->deathtarget = NULL;


		

		// set buboid position
		pNewEntity->s.origin = self->enemy->s.origin + 100.0f * forward;

		AI_AddNewGoal( pNewEntity, GOALTYPE_KILLENEMY, self->enemy );
	}
}


// ----------------------------------------------------------------------------
//
// Name:		kage_slice
// Description:
// Input:
// Output:
// Note:
//	"fires" a standard melee attack
//	which reaches hook->attack_dist
//
//	only plays hook->sound2 if target is hit
//
//	hook->sound1 can be used in the monster's code to
//	play a swinging sound
//
// ----------------------------------------------------------------------------
void kage_slice( userInventory_s *ptr, userEntity_t *self )
{
	playerHook_t	*hook = AI_GetPlayerHook( self );
	ai_weapon_t		*weapon = (ai_weapon_t *) ptr;

	AIMDATA_PTR pAimData = ai_aim_curWeapon(self, weapon);

	tr = gstate->TraceLine_q2( pAimData->org, pAimData->end, self, MASK_SHOT );

	if (tr.fraction!= 1.0)
	{
		if (tr.ent->takedamage != DAMAGE_NO)
		{
			CVector dir = pAimData->end - pAimData->org;
			dir.Normalize();

			if ( tr.ent->health > 1 )
			{
				trace_damage = tr.ent->health - 1;
			}
			else
			{
				trace_damage = (float) weapon->base_damage + rnd() * float (weapon->rnd_damage);
			}

			com->Damage( tr.ent, self, self, tr.endpos, dir, trace_damage, DAMAGE_NONE );

			if ( hook->sound2 && !AI_IsSoundDisabled() )
			{
				gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex(hook->sound2),
										  1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
			}
		}
	}
}
// ----------------------------------------------------------------------------
//NSS[11/4/99]:
// Name:		FlameSword_explode
// Description: Explode the ball  (erp!)
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void FlameSword_explode( userEntity_t *self)
{
	FlameSword_t *hook   = (FlameSword_t *) self->userHook;

	gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/e_explodeh.wav"),0.75f, ATTN_NORM_MIN, ATTN_NORM_MAX );
	
	spawn_sprite_explosion( self, self->s.origin, true );

	//do radius dammage
	com->RadiusDamage( self, self->owner, self->owner, hook->damage + rnd()*hook->rnd_damage, DAMAGE_EXPLOSION,128.0f);

	//remove the tracked entities
	RELIABLE_UNTRACK(self);

	//Remove the Entity
	gstate->RemoveEntity(self);
	
}


// ----------------------------------------------------------------------------
//NSS[11/4/99]:
// Name:		FlameSword_think
// Description: Think function for FlameSword Fireball
// Input:  userEntity_t *self 
// Output: NA
// Note:
//
// ----------------------------------------------------------------------------
void FlameSword_think( userEntity_t *self )
{
	if ( self->delay <= gstate->time )
	{
		FlameSword_explode( self );
	}
	else
	{
		self->nextthink = gstate->time + 0.1;
	}
	
	self->s.origin.pitch++;
}

// ----------------------------------------------------------------------------
//NSS[11/4/99]:
// Name:		FlameSword_touch
// Description: Touch function (generic)
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void FlameSword_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	FlameSword_explode( self );
}

// ----------------------------------------------------------------------------
// NSS[11/4/99]:
// Name:		FlameSword_attack
// Description:Fireball for the Doombat
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void FlameSword_attack( userInventory_s *ptr, userEntity_t *self )
{
	ai_weapon_t	*weapon = (ai_weapon_t *) ptr;

	userEntity_t *FlameSword = ai_fire_projectile( self, self->enemy, weapon, "models/e3/we_fglow.sp2", FlameSword_touch, 0, NULL );

	FlameSword->health = weapon->base_damage;

	FlameSword->delay = gstate->time + 5.0;

	FlameSword->think = FlameSword_think;
	FlameSword->nextthink = gstate->time + gstate->frametime;

	FlameSword->owner = self;

	FlameSword->s.render_scale.Set(0.1,0.1,0.1);
	
	AIMDATA_PTR pAimData = ai_aim_curWeapon( self, weapon );
	gstate->SetSize(FlameSword, -2, -2, -2, 2, 2, 2);
	gstate->SetOrigin2( FlameSword, pAimData->org );
	gstate->LinkEntity(FlameSword);

	FlameSword->userHook = gstate->X_Malloc(sizeof(FlameSword_t),MEM_TAG_HOOK);

	// SCG[11/24/99]: Save game stuff
	FlameSword->save = flamesword_hook_save;
	FlameSword->load = flamesword_hook_load;

	FlameSword_t *hook   = (FlameSword_t *) FlameSword->userHook;

	hook->damage		= weapon->base_damage;
	hook->rnd_damage		= weapon->rnd_damage;
	hook->time_to_live	= gstate->time + 10.0f;

	//Setup special effects for the fireball
    trackInfo_t tinfo;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

   tinfo.fru.Zero();
	tinfo.ent			=FlameSword;
    tinfo.srcent		=FlameSword;
    tinfo.lightColor.x	= .95;//R
    tinfo.lightColor.y	= .25;//G
    tinfo.lightColor.z	= .15;//B
    tinfo.lightSize		= 50;
    tinfo.flags			= TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_SCALE|TEF_SCALE2|TEF_MODELINDEX|TEF_MODELINDEX2;
    tinfo.fxflags		= TEF_LIGHT|TEF_SPRITE|TEF_STAVROS|TEF_MODEL;
   	tinfo.renderfx		= SPR_ALPHACHANNEL;
	tinfo.scale			= 0.55f;
	tinfo.scale2		= 0.45;
	tinfo.modelindex	= gstate->ModelIndex ("models/e3/we_fglow.sp2");
	tinfo.modelindex2	= gstate->ModelIndex ("models/e3/we_fball.dkm");
	
	com->trackEntity(&tinfo,MULTICAST_PVS);
    gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/e_firetraveld.wav"), 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );
}


/* ************************************ cordite ************************************** */

void cordite_explode( userEntity_t *self);
void cordite_think( userEntity_t *self );
void cordite_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf );

// ----------------------------------------------------------------------------
//
// Name:		cordite_attack
// Description: cordite attack that uses grenade code from the ripgun
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void cordite_attack( userInventory_s *ptr, userEntity_t *self )
{
	ai_weapon_t   *weapon = (ai_weapon_t *) ptr;
    corditeHook_t *hook;

	userEntity_t *cordite = ai_fire_projectile( self, self->enemy, weapon, "models/e4/we_ripgren.dkm",
	                                            cordite_touch, 0, NULL );

	cordite->movetype = MOVETYPE_BOUNCEMISSILE;

	cordite->s.effects |= EF_SMOKETRAIL;
	cordite->health = weapon->base_damage;
	cordite->delay = gstate->time + 0.1;

	cordite->userHook = gstate->X_Malloc(sizeof(corditeHook_t),MEM_TAG_HOOK);

	// SCG[11/24/99]: Save game stuff
	cordite->save = cordite_hook_save;
	cordite->load = cordite_hook_load;

    hook = (corditeHook_t *) cordite->userHook;
	hook->remove_time = gstate->time + 3.0;
	hook->last_touch_time = gstate->time;

	cordite->think = cordite_think;
	cordite->nextthink = gstate->time + gstate->frametime;

	cordite->owner = self;

    cordite->s.angles.x = self->s.angles.x;
    cordite->elasticity = 0.6;
    cordite->avelocity.x = rnd() > 0.5 ? rnd()*100 : -rnd()*100;
    cordite->avelocity.y = rnd() > 0.5 ? rnd()*100 : -rnd()*100;
    cordite->avelocity.z = rnd() > 0.5 ? rnd()*100 : -rnd()*100;
	
	AIMDATA_PTR pAimData = ai_aim_curWeapon( self, weapon );
	gstate->SetSize(cordite, 0, 0, 0, 0, 0, 0);
	gstate->SetOrigin2( cordite, pAimData->org );
	gstate->LinkEntity(cordite);

    gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/e_firetraveld.wav"), 
                              1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );
}


// ----------------------------------------------------------------------------
//
// Name:		cordite_explode
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void cordite_explode( userEntity_t *self)
{
	gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e4/we_ripgunexplode.wav"), 
                             0.45f, ATTN_NORM_MIN, ATTN_NORM_MAX );

	spawn_sprite_explosion( self, self->s.origin, true );

    com->FlashClient(self,0.8,0.8,0.5,0.3);

	//do radius damage
	com->RadiusDamage( self, self->owner, self->owner, self->health, DAMAGE_EXPLOSION, 64.0 );

	//remove the tracked entities
	RELIABLE_UNTRACK(self);

	gstate->RemoveEntity(self);
}

// ----------------------------------------------------------------------------
//
// Name:		cordite_think
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void cordite_think( userEntity_t *self )
{
    corditeHook_t       *hook = (corditeHook_t *) self->userHook;
    int                 iPointContents = gstate->PointContents(self->s.origin);

    if( iPointContents & CONTENTS_WATER)
    {
        self->velocity.x = self->velocity.x * 0.5;
        self->velocity.y = self->velocity.y * 0.5;
        if(fabs(self->velocity.z) > 50)
            self->velocity.z = self->velocity.z * 0.5;
    }

    if ( gstate->time >= hook->remove_time )
	{
        cordite_explode (self);
	}
    else if ( gstate->time >= hook->remove_time - 0.5f && gstate->time < hook->remove_time)
    {
        self->s.effects &= ~EF_SMOKETRAIL;
        self->nextthink = gstate->time + 0.1;
    }
    else
    {
//		vecDistance = self->s.origin - self->owner->s.origin;
//		if(hook && hook->iRingCount < 3)
//		{
//			self->velocity.VectorToAngles(vecForwardAngles);
//			CL_ScaleAlpha(self->s.origin, vecForwardAngles, 0.1,
//			              =number of frames=, gstate->ModelIndex(=model name=),
//			              0.76, 0.01, 55, CVector(1.0f, 0.8f, 1.0f));
//			hook->iRingCount++;
//		}
//		else 
		if(self->movetype != MOVETYPE_BOUNCE)
        {
            self->movetype = MOVETYPE_BOUNCE;
        }
        self->nextthink = gstate->time + 0.1;
        self->think = cordite_think;
	}
}


// ----------------------------------------------------------------------------
//
// Name:		cordite_touch
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

const char *cordite_touch_sounds[] = {
    "e4/we_ripgunhita.wav",
    "e4/we_ripgunhitb.wav",
    "e4/we_ripgunhitc.wav",
    "e4/we_ripgunhitd.wav",
    "e4/we_ripgunhite.wav",
    "e4/we_ripgunhitf.wav"
};

void cordite_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	if ( other->flags & FL_CLIENT || other->flags & FL_BOT || other->flags & FL_MONSTER )
	{
		cordite_explode (self);
	}
	else
	{
	    corditeHook_t *hook = (corditeHook_t *) self->userHook;
//		CVector temp = hook->last_pos - self->s.origin;// SCG[1/23/00]: not used

		hook->last_pos = self->s.origin;    

		// we just touched less than 1/10 of a second ago, so don't play bounce noise again
		if ( gstate->time < hook->last_touch_time + 0.1 )
			return;

		gstate->StartEntitySound(self,CHAN_AUTO,
		                         gstate->SoundIndex(cordite_touch_sounds[(int)(rnd() * 6)]),
		                         1.0f,64,2048);

		if ( self->velocity.Length() > 0.01 )
		{
			self->avelocity.x = rnd() * (rnd() > 0.5 ? 100 : -100);
			self->avelocity.y = rnd() * (rnd() > 0.5 ? 100 : -100);
			self->avelocity.z = rnd() * (rnd() > 0.5 ? 100 : -100);
		}

		hook->last_touch_time = gstate->time;
	}
}


// ----------------------------------------------------------------------------
// NSS[1/13/00]:
// Name:BB_Explode
// Description:Explode function for the Bouncing Blast Attack
// Input: userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void BB_Explode(userEntity_t *self)
{
	FlameSword_t *hook   = (FlameSword_t *) self->userHook;

	gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("e4/we_ripgunexplode.wav"), 0.45f, ATTN_NORM_MIN, ATTN_NORM_MAX );

	spawn_sprite_explosion( self, self->s.origin, true );

    com->FlashClient(self,0.8,0.8,0.5,0.3);

	//do radius damage
	com->RadiusDamage( self, self->owner, self->owner,hook->damage + (rnd()*hook->rnd_damage), DAMAGE_EXPLOSION, 64.0 );

	//remove the tracked entities
	RELIABLE_UNTRACK(self);

	gstate->RemoveEntity(self);
}


// ----------------------------------------------------------------------------
// NSS[1/13/00]:
// Name:BB_Think
// Description:Think function for the Bouncing Blast Attack
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void BB_Think(userEntity_t *self)
{
	FlameSword_t *hook   = (FlameSword_t *) self->userHook;
	if(self->delay < gstate->time || hook->time_to_live > 1)
	{
		//remove the tracked entities
		BB_Explode (self);
	}

	self->nextthink = gstate->time + 0.1f;

}

// ----------------------------------------------------------------------------
// NSS[1/13/00]:
// Name:BB_TouchGround
// Description:Touch function for the Bouncing Blast Attack
// Input: userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void BB_TouchGround(userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	FlameSword_t *hook   = (FlameSword_t *) self->userHook;
	if ( (other->flags & FL_CLIENT || other->flags & FL_BOT || other->flags & FL_MONSTER) &&  (self->owner != other->owner))
	{
		BB_Explode (self);
	}
	else
	{
		if(hook->time_to_live < 3)
		{
			self->think = BB_Think;
			hook->time_to_live++;
		}
		else
		{
			BB_Explode (self);
		}
	}
}





// ----------------------------------------------------------------------------
// NSS[1/15/00]:
// Name:BB_Hunt
// Description:Some will hunt some will protect
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void BB_Hunt(userEntity_t *self)
{
	CVector Dir;
	float fDistance,fDistanceFromOwner;
	FlameSword_t *hook   = (FlameSword_t *) self->userHook;

	fDistance = 0;
	fDistanceFromOwner = 0;

	if(self->enemy)
	{
		fDistance		   = VectorXYDistance(self->s.origin,self->enemy->s.origin);
	}
	if(self->owner)
	{
		fDistanceFromOwner = VectorXYDistance(self->s.origin,self->owner->s.origin);
	}

	Dir = self->enemy->s.origin - self->s.origin;
	Dir.Normalize();

	if(fDistance > 64.0f)
	{
		if(fDistanceFromOwner < 200.0f && AI_Determine_Room_Height(self,500,2) > 100)
		{
			Dir.z = Dir.z * 0.50f;
			self->velocity = Dir * 256.0f;
		}
		else
		{
			AI_FaceTowardPoint(self,self->enemy->s.origin);
			if(AI_Determine_Room_Height(self,500,1) < 48.0f)
			{
				Dir.z = 0.0f;
			}
			self->velocity = Dir * 450.0f;
		}
	}
	else
	{
		Dir.x = Dir.x * (crand() * 0.20);
		Dir.y = Dir.y * (crand() * 0.20);
		Dir.z = Dir.z * (crand() * 0.20);
		self->touch			= BB_TouchGround;
		hook->time_to_live	= 0;
		self->velocity		= Dir * 356.0f;
	}

	if(self->delay < gstate->time)
	{
		BB_Explode (self);		
	}

	self->nextthink = gstate->time + 0.1f;

}



// ----------------------------------------------------------------------------
// NSS[1/15/00]:
// Name:BB_Protect
// Description:The swirls... 
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
/*void BB_Protect(userEntity_t *self)
{
	CVector Dir;
	float fDistance,fDistanceFromOwner,fDistance;
	FlameSword_t *hook   = (FlameSword_t *) self->userHook;

	if(self->enemy)
	{
		fDistance		   = VectorXYDistance(self->s.origin,self->enemy->s.origin);
	}
	if(self->owner)
	{
		fDistanceFromOwner = VectorXYDistance(self->s.origin,self->owner->s.origin);
	}

	
	
	
	
	
	Dir = self->enemy->s.origin - self->s.origin;
	Dir.Normalize();

	if(fDistance > 64.0f)
	{
		if(fDistanceFromOwner < 200.0f)
		{
			Dir.z = Dir.z * 0.50f;
			self->velocity = Dir * 156.0f;
		}
		else
		{
			self->velocity.Zero();
		}
	}
	else
	{
		self->gravity		= 2.0f;
		self->touch			= BB_TouchGround;
		hook->time_to_live	= 0;
		self->velocity		= Dir * 356.0f;
	}

	if(self->delay < gstate->time)
	{
		BB_Explode (self);		
	}

	self->nextthink = gstate->time + 0.1f;

}
*/




// ----------------------------------------------------------------------------
// NSS[1/13/00]:
// Name:BB_TouchCeiling
// Description:Touch function for the Bouncing Blast Attack
// Input: userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void BB_TouchCeiling(userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
//	FlameSword_t *hook   = (FlameSword_t *) self->userHook;// SCG[1/23/00]: not used
	if ( (other->flags & FL_CLIENT || other->flags & FL_BOT || other->flags & FL_MONSTER) &&  (self->owner != other->owner))
	{
		BB_Explode (self);
	}
	else
	{
		// NSS[1/15/00]:Protector or Hunter?
		//if(rnd() < 0.50f)
		//{
		self->think = BB_Hunt;

		//}
		//else
		//{
		//	self-think	= BB_Protect;
		//}
	}
}


void Kage_FindNearSpawnPoint( userEntity_t *self, CVector &spawnPoint )
{
	// search the eight directions around the self to find a spawn point
	float delta_angle = 30.0f * self->hacks_int;
	CVector angle = self->s.angles;
	angle.pitch = -5.0f;
	angle.yaw   = delta_angle;

	CVector vector;
	angle.AngleToForwardVector( vector );
	CVector endPoint;

	VectorMA( self->s.origin, vector, 64.0f, endPoint );
	endPoint.z += 16.0f;
	spawnPoint = endPoint;
}


// ----------------------------------------------------------------------------
// <nss>
// Name:		KagespawnZapFlareRotate
// Description: This is a generic flare routine which can be used to create nice
// flare FX for monster's firing weapons and such.
// Input:userEntity_t *self, CVector *Origin
// Output:NA
// Note:Changing the scale values so that they are not uniform will result in sharper
//edges.  Life is the base for the depletion rate of scale and alpha level
// ----------------------------------------------------------------------------
userEntity_t * KagespawnZapFlareRotate(userEntity_t *self, CVector *Origin, CVector Rotate,CVector Size,float life, char *flarename)
{
	userEntity_t *Flare;
    ZapFlare_t *hook;
	
	
	Flare = gstate->SpawnEntity();
	Flare->s.render_scale = Size;
	
	Flare->className		= "flare_sprite";
	Flare->s.renderfx		|= (SPR_ALPHACHANNEL);
	Flare->s.modelindex		= gstate->ModelIndex (flarename);
	Flare->movetype			= MOVETYPE_NONE;
	Flare->solid			= SOLID_NOT;
	Flare->owner			= self;
	Flare->s.alpha			= 0.75f;
	Flare->s.frame			= 0;
	Flare->s.origin.x		= Origin->x;
	Flare->s.origin.y		= Origin->y;
	Flare->s.origin.z		= Origin->z;
	Flare->s.frame			= 0;
    Flare->think			= ZapFlareRotateThink;
    Flare->nextthink		= gstate->time + 0.1f;
	Flare->clipmask			= MASK_SHOT;
	Flare->s.angles			= self->s.angles;
	Flare->userHook			= gstate->X_Malloc(sizeof(ZapFlare_t),MEM_TAG_HOOK);
	hook					=( ZapFlare_t *)Flare->userHook;
    hook->Delta_Angles		= Rotate;
	hook->time_to_live		= gstate->time+life;

	// SCG[11/24/99]: Save game stuff
	Flare->save = zapflare_hook_save;
	Flare->load = zapflare_hook_load;

	if(life > 1.0f)
	{
		hook->delta_alpha = 0.95f;
		hook->delta_scale = 0.85f;
	}
	else
	{
		hook->delta_alpha = life;
		hook->delta_scale = life;
	}
	
	
	gstate->LinkEntity(Flare);
	
	return Flare;
}


// ----------------------------------------------------------------------------
// NSS[1/13/00]:
// Name:Kage_Bouncing_Blast
// Description:One of Kage's special attacks
// Input:userInventory_s *ptr , userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void Kage_Bouncing_Blast( userInventory_s *ptr, userEntity_t *self )
{
	// summons a buboid from one of several set points in the map
	CVector AwayPoint;
	char Buffer[32];
	
	Kage_FindNearSpawnPoint( self, AwayPoint );

	int Type = (int)(rnd()*3);

	// NSS[12/17/99]:Get the monster
	Com_sprintf(Buffer,sizeof(Buffer),"monster_ghost");

	// NSS[12/17/99]:Spawn the monster
	userEntity_t *pNewEntity = com->SpawnDynamicEntity( self, Buffer, true );
    if ( pNewEntity )
    {
	    int nEpairsCount = 0;
		while ( pNewEntity->epair[nEpairsCount].key )
		{
			
			if(self->epair)
			{
				if(!(_stricmp(self->epair[nEpairsCount].key,"spawnname"))||!(_stricmp(self->epair[nEpairsCount].key,"deathtarget")))
				{
					pNewEntity->epair[nEpairsCount].value = NULL;		
				}
			}
			nEpairsCount++;
		}
		SPAWN_CallInitFunction( pNewEntity, Buffer );
		pNewEntity->s.origin = AwayPoint;
		// NSS[6/7/00]:Make sure we CANNOT do any kind of deathtarget thang.
		if ( pNewEntity->deathtarget != NULL )
		{
			pNewEntity->deathtarget = NULL;
		}

		IncrementMonsterCount();		

		CVector Scale, Rotate, Dir, Location;

		Dir.Set(0.0,0.0,1.0);
		
		Scale.Set (0.75f,5.0f,10.0f);
		Rotate.Set(10.0f,5.0f,5.0f);
		Location = pNewEntity->s.origin;

		KagespawnZapFlareRotate(pNewEntity, &Location,Rotate, Scale,1.50f,"models/global/e_flred.sp2");

		Scale.Set (10.0f,0.75f,5.0f);
		Rotate.Set(5.0f,5.0f,10.0f);
		Location = pNewEntity->s.origin;
		Location.z += 18.0f;
		KagespawnZapFlareRotate(pNewEntity, &Location,Rotate, Scale,1.10f,"models/global/e_flred.sp2");
		
		gstate->WriteByte(SVC_TEMP_ENTITY);
		gstate->WriteByte(TE_SMOKE);
		gstate->WritePosition(Location);
		gstate->WriteFloat(10);
		gstate->WriteFloat(10);
		gstate->WriteFloat(5);
		gstate->WriteFloat(50);
		gstate->MultiCast(Location,MULTICAST_PVS);	

		if ( self->enemy )
		{
			AI_AddNewGoal( pNewEntity, GOALTYPE_KILLENEMY, self->enemy );
			CVector Dir,Angles;
			Dir = self->enemy->s.origin - pNewEntity->s.origin;
			Dir.Normalize();
			VectorToAngles(Dir,Angles);
			pNewEntity->s.angles.Set(0.0f,0.0f,0.0f);
			pNewEntity->s.angles.yaw = Angles.yaw;
			pNewEntity->owner = self;
		}
		else
		{
			AI_AddNewGoal( pNewEntity, GOALTYPE_WANDER );
			if(Type == 1)
			{
				AI_ForceSequence(pNewEntity,"flya",FRAME_LOOP);
			}
		}
	}
}



// ----------------------------------------------------------------------------
//
// Name:
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_ai_weapons_register_func()
{
	gstate->RegisterFunc("cryo_touch",cryo_touch);
	gstate->RegisterFunc("ai_muzzle_flash_think",ai_muzzle_flash_think);
	gstate->RegisterFunc("psiblast_firethink",psiblast_firethink);
	gstate->RegisterFunc("chaingun_think",chaingun_think);
	gstate->RegisterFunc("uzigun_think",uzigun_think);
	gstate->RegisterFunc("boargun_think",boargun_think);
	gstate->RegisterFunc("rocket_think",rocket_think);
	gstate->RegisterFunc("magic_arrow_think",magic_arrow_think);
	gstate->RegisterFunc("sludge_think",sludge_think);
	gstate->RegisterFunc("laser_think",laser_think);
	gstate->RegisterFunc("deathlaser_think",deathlaser_think);
	gstate->RegisterFunc("laserbeam_think",laserbeam_think);
	gstate->RegisterFunc("light_think",light_think);
	gstate->RegisterFunc("spear_think",spear_think);
	gstate->RegisterFunc("ai_lightning_think",ai_lightning_think);
	gstate->RegisterFunc("throwing_axe_think",throwing_axe_think);
	gstate->RegisterFunc("psyclaw_attack_think",psyclaw_attack_think);
	gstate->RegisterFunc("venom_spit_think",venom_spit_think);
	gstate->RegisterFunc("rock_throw_think",rock_throw_think);
	gstate->RegisterFunc("medusa_spit_think",medusa_spit_think);
	gstate->RegisterFunc("fireball_think",fireball_think);
	gstate->RegisterFunc("lightningThink",lightningThink);
	gstate->RegisterFunc("wispFade",wispFade);
	gstate->RegisterFunc("wyndraxWispThink",wyndraxWispThink);
	gstate->RegisterFunc("stave_think",stave_think);
	gstate->RegisterFunc("wispPain",wispPain);
	gstate->RegisterFunc("nharre_summon",nharre_summon);
	gstate->RegisterFunc("nharre_summon_demon",nharre_summon_demon);
	gstate->RegisterFunc("nightmareDeath",nightmareDeath);
	gstate->RegisterFunc("nightmareAlmostDeath",nightmareAlmostDeath);
	gstate->RegisterFunc("garroth_summon",garroth_summon);
	gstate->RegisterFunc("kage_slice",kage_slice);
	gstate->RegisterFunc("ThunderSpray_Think",ThunderSpray_Think);
	gstate->RegisterFunc("meteor_think",meteor_think);
	
	//FlameSword Register Functions
	gstate->RegisterFunc("FlameSword_attack",FlameSword_attack);
	gstate->RegisterFunc("FlameSword_touch",FlameSword_touch);
	gstate->RegisterFunc("FlameSword_think",FlameSword_think);
	gstate->RegisterFunc("FlameSword_explode",FlameSword_explode);

	gstate->RegisterFunc("BB_Think",BB_Think);
	gstate->RegisterFunc("BB_Hunt",BB_Hunt);
	gstate->RegisterFunc("BB_TouchCeiling",BB_TouchCeiling);
	gstate->RegisterFunc("BB_TouchGround",BB_TouchGround);

	gstate->RegisterFunc("cordite_think",cordite_think);
	gstate->RegisterFunc("cordite_touch",cordite_touch);

	gstate->RegisterFunc("meteor_hook_save",meteor_hook_save);
	gstate->RegisterFunc("meteor_hook_load",meteor_hook_load);
	gstate->RegisterFunc("wyndrax_hook_save",wyndrax_hook_save);
	gstate->RegisterFunc("wyndrax_hook_load",wyndrax_hook_load);
	gstate->RegisterFunc("lightning_hook_save",lightning_hook_save);
	gstate->RegisterFunc("lightning_hook_load",lightning_hook_load);
	gstate->RegisterFunc("rocket_hook_save",rocket_hook_save);
	gstate->RegisterFunc("rocket_hook_load",rocket_hook_load);
	gstate->RegisterFunc("stave_hook_save",stave_hook_save);
	gstate->RegisterFunc("stave_hook_load",stave_hook_load);
	gstate->RegisterFunc("psyclaw_hook_save",psyclaw_hook_save);
	gstate->RegisterFunc("psyclaw_hook_load",psyclaw_hook_load);
	gstate->RegisterFunc("thunder_jiz_hook_save",thunder_jiz_hook_save);
	gstate->RegisterFunc("thunder_jiz_hook_load",thunder_jiz_hook_load);
	gstate->RegisterFunc("camlight_hook_save",camlight_hook_save);
	gstate->RegisterFunc("camlight_hook_load",camlight_hook_load);
	gstate->RegisterFunc("zapflare_hook_save",zapflare_hook_save);
	gstate->RegisterFunc("zapflare_hook_load",zapflare_hook_load);
	gstate->RegisterFunc("flamesword_hook_save",flamesword_hook_save);
	gstate->RegisterFunc("flamesword_hook_load",flamesword_hook_load);
	gstate->RegisterFunc("cordite_hook_save",cordite_hook_save);
	gstate->RegisterFunc("cordite_hook_load",cordite_hook_load);
}
