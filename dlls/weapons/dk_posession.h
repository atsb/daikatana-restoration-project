#ifndef _DK_POSESSION_H
#define _DK_POSESSION_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef struct {
   weapon_t weapon;                       // weapon_t must be first entry
} dk_posession_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t dk_posessionWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
userInventory_t *d3_give (userEntity_t *self, int ammoCount);
short d3_select(userEntity_t *self);
void *d3_command(struct userInventory_s *ptr, char *commandStr, void *data);
void d3_use(userInventory_s *ptr, userEntity_t *self);
void d3_think(userEntity_t *self);
void d3_touch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);

#endif