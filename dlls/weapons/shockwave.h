#ifndef _SHOCKWAVE_H
    #define _SHOCKWAVE_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------
    #define SHOCKWAVE_NUM_WAVES         6        // # of shockwave rings
    #define SHOCKWAVE_MAX_DAMAGE        100      // strongest ring's damage
    #define SWAVE_MIN_DAMAGE            150

    #define SHOCKRING_TIME              3.0
    #define SHOCKRING_MAX_DIST          350.0

    #define SHOCKWAVE_SIZE_START        8.0
    #define SHOCKWAVE_SIZE_END          16.0

    #define SHOCKWAVE_SCALE_START       .5
    #define SHOCKWAVE_SCALE_END         .5

    #define SHOCKWAVE_GROW_STEPS        30.0

    #define SHOCKRING_MAX_FRAMES        6
    #define SHOCKRING_MAX_SCALE         6

    #define NUM_SPHERE_FRAMES        10
    #define NUM_SHOCKRING_FRAMES     6

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef struct
{
    float min;
    float max;
    float starttime;
} shockRing_t;

typedef struct
{
    float       spawntime;
    short       ringnum;
    shockRing_t ring[SHOCKWAVE_NUM_WAVES];
    short       sBounceCount;
    CVector     vecLastShotRingOrigin;
    int         iShotRingCount;
	CHitCounter hits;
} shockwaveHook_t;

typedef struct
{
    short pc,lastpc;           // keep track of contents
    short bounce_count;        // # bounces
    short spawn;               //
    float explode_time;        // 
    float min,max;             // shockwave min/max damage radius
    float scale_change;
    float size_change;
} shockwaveHook_old_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t shockwaveWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
DllExport void weapon_shockwave(userEntity_t *self);
DllExport void ammo_shocksphere(userEntity_t *self);

short shockwaveSelect(userEntity_t *self);
userInventory_t *shockwaveGive (userEntity_t *self, int ammoCount);
void *shockwaveCommand(struct userInventory_s *ptr, char *commandStr, void *data);
void shockwaveUse(userInventory_s *ptr, userEntity_t *self);
void shockwaveTouch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);

int shockwaveDamage(userEntity_t *inflictor, userEntity_t *attacker, userEntity_t *ignore, float damage,float maxdist, int damage_type, float curmin, float curmax, float push_extra);
void shockwaveExplode(userEntity_t *self,short explosionNum);
void shockwaveShootFunc(userEntity_t *self);
void shockringThink(userEntity_t *pos);

#endif