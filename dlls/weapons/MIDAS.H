#ifndef _MIDAS_H
    #define _MIDAS_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef struct
{
    weapon_t weapon;                       // weapon_t must be first entry
} midas_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t midasWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
DllExport void weapon_midas(userEntity_t *self);
DllExport void ammo_pyrite (userEntity_t *self);

static userInventory_t *give (userEntity_t *self, int ammoCount);
static short midas_select(userEntity_t *self);
static void *command(struct userInventory_s *ptr, char *commandStr, void *data);
static void use(userInventory_s *ptr, userEntity_t *self);
static void touch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);
static void think(userEntity_t *self);

#endif