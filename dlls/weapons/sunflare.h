#ifndef _SUNFLARE_H
    #define _SUNFLARE_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------
    #define SUNFLARE_MAX_FLAMES         200

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------

typedef struct
{
	short flames;
	CHitCounter hits;
} sfHook_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t sunflareWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
DllExport void weapon_sunflare(userEntity_t *self);

userInventory_t *sunflareGive (userEntity_t *self, int ammoCount);
short sunflareSelect(userEntity_t *self);
void *sunflareCommand(struct userInventory_s *ptr, char *commandStr, void *data);
void sunflareUse(userInventory_s *ptr, userEntity_t *self);
void sunflareShootFunc(userEntity_t *self);
void sunflareRestartEffect(userEntity_t *self);
void sunflareExplode(userEntity_t *entSelf,userEntity_t *other);
void sunflareThink(userEntity_t *self);
void sunflareTouch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);
void flame_think2(userEntity_t *self);
void flame_doflames(userEntity_t *self);
void flame_die(userEntity_t *self);
void flame_damage(userEntity_t *self);
void weapThink(userEntity_t *entWeapon);
#endif