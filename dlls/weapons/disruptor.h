#ifndef _DISRUPTOR_H
    #define _DISRUPTOR_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------
    #define MAX_STUN     10

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef struct
{
    userEntity_t *ent;
    float time;

    freezeEntity_t freezeInfo;
} stunInfo_t;

typedef struct
{
    weapon_t weapon;                       // weapon_t must be first entry

    stunInfo_t stun[MAX_STUN];
} disruptor_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t disruptorWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
DllExport void weapon_disruptor(userEntity_t *self);

userInventory_t *disruptorGive(userEntity_t *self, int ammoCount);
short disruptorSelect(userEntity_t *self);
void *disruptorCommand(struct userInventory_s *ptr, char *commandStr, void *data);
void disruptorUse(userInventory_s *ptr, userEntity_t *self);
void disruptorSmashFunc(userEntity_t *self);
userInventory_t *disruptorGive (userEntity_t *self, int ammoCount);
#endif