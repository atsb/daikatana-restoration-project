#ifndef _IONBLASTER_H
    #define _IONBLASTER_H

typedef struct
{
    short bounce_count;
	userEntity_t *owner;
	CVector lastImpactNorm;
} ionHook_t;

extern weaponInfo_t ionblasterWeaponInfo;

DllExport void weapon_ionblaster(userEntity_t *self);
DllExport void ammo_ionpack (userEntity_t *self);

short ionblaster_select(userEntity_t *self);
userInventory_t *ionblaster_give (userEntity_t *self, int ammoCount);
void ionblaster_ammo(userEntity_t *self);
void ionblaster_use(userInventory_s *ptr, userEntity_t *self);
void *ionblaster_command (struct userInventory_s *ptr, char *commandStr, void *data);

void ionblaster_touch(userEntity_t *self,userEntity_t *other,cplane_t *plane, csurface_t *surf);
void ionblaster_think (userEntity_t *self);
void ionblaster_remove(userEntity_t *self);
void ionblaster_diethink(userEntity_t *self);
 
#endif