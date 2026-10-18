#ifndef _DK_PROTEUS_H
#define _DK_PROTEUS_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------
#define DK_PROTEUS_PHASEIN_TIME     6
#define DK_PROTEUS_PHASEOUT_TIME    6

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef struct {
   weapon_t weapon;                       // weapon_t must be first entry

   short active;
} dk_proteus_t;

typedef struct {
   dk_proteus_t *dk_proteus;
   short prevCameraMode;
   float killtime;
} dk_proteusHook_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t dk_proteusWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
userInventory_t *d2_give (userEntity_t *self, int ammoCount);
short d2_select(userEntity_t *self);
void *d2_command(struct userInventory_s *ptr, char *commandStr, void *data);
void d2_use(userInventory_s *ptr, userEntity_t *self);
void think_phaseIn(userEntity_t *self);
void think_lurking(userEntity_t *self);
void think_phaseOut(userEntity_t *self);
void touch_lurking(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);
void controllerRemove(userEntity_t *self);

#endif