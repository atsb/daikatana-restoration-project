#ifndef _FLASHLIGHT_H
    #define _FLASHLIGHT_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------
    #define FLASHLIGHT_POWER_DIV     10

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef struct
{
    weapon_t weapon;                       // weapon_t must be first entry

    CVector color;
    float killtime;
} flashlight_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t flashlightWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
DllExport void weapon_flashlight(userEntity_t *self);

userInventory_t *flashlight_give (userEntity_t *self, int ammoCount);
short flashlight_select(userEntity_t *self);
void *flashlight_command(struct userInventory_s *ptr, char *commandStr, void *data);
void flashlight_use(userInventory_s *ptr, userEntity_t *self);
void flashlight_touch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);
void flashlight_startLaser(userEntity_t *self);
void flashlight_think(userEntity_t *self);
void flashlight_Remove(userEntity_t *self);

#endif