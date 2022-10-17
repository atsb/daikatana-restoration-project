#ifndef _WYNDRAX_H
    #define _WYNDRAX_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------
    #define WYNDRAX_MAX_LIGHTNING    4       // max lightning spawnable by a wisp
    #define WYNDRAX_MAX_ENT_LIST     10       // max # of ents to choose to zap
    #define WYNDRAX_MAX_DUMMY        4

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef struct
{
    weapon_t weapon;                       // weapon_t must be first entry
} wyndrax_t;

typedef struct
{
    float killtime;                                          // when to kill wisp
    short lightningCount;                                    // # lightning bolts it's spawned
    userEntity_t *lightningList[WYNDRAX_MAX_LIGHTNING];      // list of lightning ents
    short dummyCount;
    userEntity_t dummyList[WYNDRAX_MAX_DUMMY];
    CVector forward,up;
    short sinofs;
    float sinetime;
	float Personality;
	CHitCounter hits;
} wyndraxHook_t;


typedef struct
{
    float				killtime;
    userEntity_t		*owner,*src,*dst;
	short				isDummy;
} lightningHook_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t wyndraxWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
DllExport void weapon_wyndrax(userEntity_t *self);
DllExport void ammo_wisp (userEntity_t *self);

userInventory_t *wyndraxGive (userEntity_t *self, int ammoCount);
short wyndraxSelect(userEntity_t *self);
void *wyndraxCommand(struct userInventory_s *ptr, char *commandStr, void *data);
void wyndraxUse(userInventory_s *ptr, userEntity_t *self);
void wyndraxShootFunc(userEntity_t *self);
void wyndraxTouch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);
void wyndraxThink(userEntity_t *self);
void sineMove(userEntity_t *self);
void removeWisp(userEntity_t *self);
userEntity_t *spawnLightning(userEntity_t *owner,userEntity_t *src, userEntity_t *dst, float killtime, short dummy);
void lightningThink(userEntity_t *self);
void removeLightning(userEntity_t *self);
int directLine(userEntity_t *src,userEntity_t *dst);

#endif