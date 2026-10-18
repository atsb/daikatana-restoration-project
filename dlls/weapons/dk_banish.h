#ifndef _DK_BANISH_H
#define _DK_BANISH_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef struct {
   weapon_t weapon;                       // weapon_t must be first entry
} dk_banish_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t dk_banishWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
userInventory_t *d6_give(userEntity_t *self, int ammoCount);
short d6_select(userEntity_t *self);
void *d6_command(struct userInventory_s *ptr, char *commandStr, void *data);
void d6_use(userInventory_s *ptr, userEntity_t *self);
void think(userEntity_t *self);
void touch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);

#endif