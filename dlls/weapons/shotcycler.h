#ifndef _SHOTCYCLER_H
#define _SHOTCYCLER_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------
#define SS_SHOT_START_FRAME      3           // ANIM_SHOOT_STD frame to start shooting
#define SS_MAX_SHOTS             6           // number of shots
#define SS_SHOT_DELAY            .3          // delay between shots

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef struct {
   weapon_t		weapon;							// weapon_t must be first entry

   short		shot;							// tracks the current shot.
} shotcycler_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t shotcyclerWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
DllExport void weapon_shotcycler(userEntity_t *self);
DllExport void ammo_shells (userEntity_t *self);


userInventory_t *shotcyclerGive (userEntity_t *self, int ammoCount);
short shotcyclerSelect(userEntity_t *self);
void *shotcyclerCommand(struct userInventory_s *ptr, char *commandStr, void *data);
void shotcyclerUse(userInventory_s *ptr, userEntity_t *self);
void shotcyclerShootFunc(userEntity_t *self);
void shootThink(userEntity_t *self, short fire);
void shotcyclerShoot(userEntity_t *self);

#endif