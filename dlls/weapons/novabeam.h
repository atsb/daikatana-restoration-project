#ifndef _NOVABEAM_H
    #define _NOVABEAM_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef struct
{
    weapon_t weapon;                       // weapon_t must be first entry

    short first_think;
    float killbeam;
    short endeffect;
    short changekill;
    userEntity_t *beam;
	float burst;
} novabeam_t;

typedef struct
{
	CHitCounter hits;
} novaHook_t;
//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t novabeamWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
DllExport void weapon_novabeam(userEntity_t *self);
DllExport void ammo_novabeam (userEntity_t *self);

userInventory_t *novabeam_give (userEntity_t *self, int ammoCount);
short novabeam_select(userEntity_t *self);
void *novabeam_command(struct userInventory_s *ptr, char *commandStr, void *data);
void novabeam_use(userInventory_s *ptr, userEntity_t *self);
void novabeam_startLaser(userEntity_t *self);
void novabeam_touch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);
void novabeam_think(userEntity_t *self);

#endif