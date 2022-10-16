#ifndef _TRIDENT_H
    #define _TRIDENT_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------
    #define TRI_TIP_SIZE       3
    #define TRI_EXPAND_TIME    .18

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef enum
{
    TRITIP_LEFT,
    TRITIP_MIDDLE,
    TRITIP_RIGHT
} tridentTip_t;

typedef struct
{
    weapon_t weapon;                       // weapon_t must be first entry
} trident_t;

typedef struct
{
    // used by all tips
    tridentTip_t tiptype;
    CVector forward,right,up;

    // only used by middle tip
    float boostTime;
    userEntity_t *rt,*left;
    short supercharged;

	short inWater;
	short supermsgSent;

    // only used by outer tips
    float movedir;
    float changeDirTime;
    userEntity_t *middle;
} tridentHook_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t tridentWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
DllExport void weapon_trident(userEntity_t *self);
DllExport void ammo_tritips (userEntity_t *self);

userInventory_t *tridentGive (userEntity_t *self, int ammoCount);
short tridentSelect(userEntity_t *self);
void *tridentCommand(struct userInventory_s *ptr, char *commandStr, void *data);
void tridentUse(userInventory_s *ptr, userEntity_t *self);
void tridentShootFunc(userEntity_t *self);
userEntity_t *tipShoot(userEntity_t *self,tridentTip_t tiptype);
void tipThink(userEntity_t *self);
void tipTouch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);
void tipSetOuterDirection(userEntity_t *self);
void tipRemove(userEntity_t *entTip);
void tipDie(userEntity_t *self);

#endif