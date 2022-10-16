#ifndef _RIPGUN_H
    #define _RIPGUN_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef struct
{
    weapon_t weapon;                       // weapon_t must be first entry

    float killtime;
    float lastframe;
    short stopfiring;
    userEntity_t *controller;
} ripgun_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t ripgunWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
DllExport void weapon_ripgun(userEntity_t *self);
DllExport void ammo_ripgun (userEntity_t *self);

userInventory_t *ripgun_give (userEntity_t *self, int ammoCount);
short ripgun_select(userEntity_t *self);
void *ripgun_command(struct userInventory_s *ptr, char *commandStr, void *data);
void ripgun_use(userInventory_s *ptr, userEntity_t *self);
void ripgun_shoot_func(userEntity_t *self);
void ripgun_shootThink(userEntity_t *self);
void ripgun_shootSlug(userEntity_t *self);
void ripgun_slug_think(userEntity_t *self);
void ripgun_shootTracer(userEntity_t *self,CVector &start,CVector &end);
void ripgun_removeController(userEntity_t *self);

#endif

