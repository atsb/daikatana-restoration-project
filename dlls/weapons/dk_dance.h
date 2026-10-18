#ifndef _DK_DANCE_H
#define _DK_DANCE_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef struct {
   weapon_t weapon;                       // weapon_t must be first entry
} dk_dance_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t dk_danceWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
userInventory_t *d5_give(userEntity_t *self, int ammoCount);
short d5_select(userEntity_t *self);
void *d5_command(struct userInventory_s *ptr, char *commandStr, void *data);
void d5_use(userInventory_s *ptr, userEntity_t *self);
void d5_think(userEntity_t *self);
void d5_touch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);

#endif