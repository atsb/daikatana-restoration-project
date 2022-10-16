#ifndef _SILVERCLAW_H
    #define _SILVERCLAW_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef struct
{
    weapon_t weapon;                       // weapon_t must be first entry

    short starttrack;
    short slashanim;
    short shootanim;
} silverclaw_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t silverclawWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
DllExport void weapon_silverclaw(userEntity_t *self);

userInventory_t *silverclaw_give (userEntity_t *self, int ammoCount);
short silverclaw_select(userEntity_t *self);
void silverclaw_use(userInventory_s *ptr, userEntity_t *self);
void *silverclaw_command(struct userInventory_s *ptr, char *commandStr, void *data);
void silverclaw_think(userEntity_t *self);
void kill_slash(userEntity_t *self);
void silverclaw_marks(userEntity_t *self, trace_t &trace);

#endif