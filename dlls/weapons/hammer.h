#ifndef _HAMMER_H
    #define _HAMMER_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------
    #define HAMMER_UPSTOP_FRAMES	18
    #define HAMMER_UPSTOP_FRAME		(winfoFirstFrame(ANIM_SHOOT_STD)+HAMMER_UPSTOP_FRAMES)
    #define HAMMER_DAMAGE_FRAME		(winfoFirstFrame(ANIM_SHOOT_STD)+24)

    #define HAMMER_QUAKE_DIST		450
	#define HAMMER_DAMAGE_RADIUS	((winfo->range) ? winfo->range : 128)
    #define HAMMER_QUAKE_TIME		6

    #define HAMMER_MAX_FROZEN		128

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef struct
{
    weapon_t weapon;                       // weapon_t must be first entry
    float smashtime;
    float power;
} hammer_t;

typedef struct
{
    float   killtime;
    CVector origin;
	userEntity_t *frozen[HAMMER_MAX_FROZEN]; // frozen entities, NULL terminated if < HAMMER_MAX_FROZEN
                                        // this is used to interrupt a hades hammer attack
	CHitCounter		hits;
} hammerHook_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t hammerWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
DllExport void weapon_hammer(userEntity_t *self);

// Logic [7/22/99]: kickass dupe symbols in the object and library
userInventory_t *hammerGive (userEntity_t *self, int ammoCount);
short hammerSelect(userEntity_t *self);
void *hammerCommand(struct userInventory_s *ptr, char *commandStr, void *data);
void hammerUse(userInventory_s *ptr, userEntity_t *self);
void touch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);
void startLaser(userEntity_t *self);
void hammerThink(userEntity_t *self);
void doEarthquake(userEntity_t *self);
void Hammer_impact(userEntity_t *self, trace_t &trace,float fscale);
void hammer_radiusDamage(userEntity_t *hammer, userEntity_t *player, float damage, float radius, qboolean bDiminishing);

#endif