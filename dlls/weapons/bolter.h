#ifndef _BOLTER_H
    #define _BOLTER_H

typedef struct
{
    weapon_t weapon;

    // bolter fields
    int     reload_count;  // can fire twice before reloading
} bolterWeapon_t;

typedef struct
{
    float           remove_time;

    // Content friction hook
    frictionHook_t  *fhook;
} boltHook_t;

extern weaponInfo_t bolterWeaponInfo;

DllExport void weapon_bolter(userEntity_t *self);
DllExport void ammo_bolts (userEntity_t *self);

short           bolter_select (userEntity_t *self);
userInventory_t *bolter_give (userEntity_t *self, int ammoCount);
void            bolter_use (userInventory_s *ptr, userEntity_t *self);
void            bolter_detonate(userEntity_t *self);
void *bolter_command (struct userInventory_s *ptr, char *commandStr, void *data);

int     bolter_load_init (void);
void    bolter_level_init (void);

#endif