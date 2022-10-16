#ifndef _SLUGGER_H
    #define _SLUGGER_H

typedef struct
{
    weapon_t    weapon;

    int     mode;
}weaponPulse_t;


typedef struct
{
    float   remove_time;
    float   last_touch_time;

    CVector last_pos;
    int     iRingCount;
    frictionHook_t  *fhook;
} corditeHook_t;

extern weaponInfo_t sluggerWeaponInfo;
extern weaponInfo_t corditeWeaponInfo;

DllExport void weapon_slugger(userEntity_t *self);
DllExport void ammo_slugger(userEntity_t *self);
DllExport void ammo_cordite (userEntity_t *self);

userInventory_t *slugger_give (userEntity_t *self, int ammoCount);
short   slugger_select (userEntity_t *self);
void    slugger_select_finish (userEntity_t *self);
void    slugger_use (struct userInventory_s *ptr, userEntity_t *self);
void *weapon_command (struct userInventory_s *ptr, char *commandStr, void *data);
void slugger_shootShrapnel(userEntity_t *self);
void    ammoTouchSlugger (userEntity_t *self, userEntity_t *other, cplane_t *plance, csurface_t *surf);

#endif