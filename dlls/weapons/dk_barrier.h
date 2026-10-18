#ifndef _DK_BARRIER_H
#define _DK_BARRIER_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef struct {
   weapon_t weapon;                       // weapon_t must be first entry
} dk_barrier_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t dk_barrierWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
userInventory_t *d4_give(userEntity_t *self, int ammoCount);
short d4_select(userEntity_t *self);
void *d4_command(struct userInventory_s *ptr, char *commandStr, void *data);
void d4_use(userInventory_s *ptr, userEntity_t *self);
void d4_think(userEntity_t *self);
void d4_touch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);

#endif