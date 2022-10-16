#ifndef _VENOMOUS_H
    #define _VENOMOUS_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------
    #define VEN_BITE_DIST      150
    #define VEN_START_SPEED    600
    #define VEN_FRICTION_RATE  3

    #define VEN_BITE1_FRAME    15
    #define VEN_BITE2_FRAME    21

    #define MAX_VEN_APPEAR_FRAMES       3
    #define MAX_VEN_STAND_FRAMES        9
    #define MAX_VEN_DISSIPATE_FRAMES    3

    #define VEN_MAX_BLOW       6

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef enum venomousMode_s
{
    VEN_MODE_APPEAR,
    VEN_MODE_STAND,
    VEN_MODE_DISSIPATE
} venomousMode_t;

typedef struct
{
    weapon_t weapon;                       // weapon_t must be first entry

    int     iAmbAnim;
} venomous_t;

typedef struct
{
    float			killtime;
    CVector			forward;
    short			mode;
    short			count;
    short			sAltFire;
	short			onGround;
	userEntity_t	*owner;
} venomousHook_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t venomousWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
DllExport void weapon_venomous(userEntity_t *self);
DllExport void ammo_venomous(userEntity_t *self);

userInventory_t *venomousGive(userEntity_t *self, int ammoCount);
short venomousSelect(userEntity_t *self);
void *venomousCommand(struct userInventory_s *ptr, char *commandStr, void *data);
void venomousUse(userInventory_s *ptr, userEntity_t *self);
void venomousBite(userEntity_t *self);
userEntity_t *venomousSpitPoison(userEntity_t *self,CVector &pos,short num,float speed);

void poisonThink(userEntity_t *self);
void poisonTouch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);
void venomousShoot(userEntity_t *self);

#endif