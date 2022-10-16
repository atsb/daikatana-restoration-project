#ifndef _SIDEWINDER_H
#define _SIDEWINDER_H

extern weaponInfo_t sidewinderWeaponInfo;

DllExport void weapon_sidewinder(userEntity_t *self);
DllExport void ammo_rockets (userEntity_t *self);

short sidewinder_select(userEntity_t *self);
userInventory_t *sidewinder_give (userEntity_t *self, int ammoCount);
void sidewinder_use(userInventory_t *ptr, userEntity_t *self);
void *sidewinder_command (struct userInventory_s *ptr, char *commandStr, void *data);

int sidewinder_load_init(void);
void sidewinder_level_init(void);

#endif