#ifndef _ZEUS_H
    #define _ZEUS_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------
#define ZEUS_MAXBOLTS   40      // maximum bolts

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef struct
{
    weapon_t weapon;                       // weapon_t must be first entry
} zeus_t;

typedef struct
{
    userEntity_t    *entLeaderBolt;
    userEntity_t    *entBolt;
    userEntity_t    *entTarget;
} boltList_t;


#define ZEUS_MAX_TARGETS		20
typedef struct
{
	userEntity_t	*targets[ZEUS_MAX_TARGETS];
	int				targetCount;
	int				bolts;
	int				zaps;
} zHook_t;


//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t zeusWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
DllExport   void    weapon_zeus (userEntity_t *self);
DllExport   void    ammo_zeus (userEntity_t *self);

// common functions
userInventory_t *zeus_give (userEntity_t *self, int ammoCount);
short zeus_select(userEntity_t *self);
void *command(struct userInventory_s *ptr, char *commandStr, void *data);
void use(userInventory_s *ptr, userEntity_t *self);
void touch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);
userEntity_t *spawnBolt(userEntity_t *entSource, userEntity_t *entDest, userEntity_t *entOwner, userEntity_t *entLeaderBolt);
// zeus specific functions
void zeusStrike(userEntity_t *entSelf);
#endif