#ifndef _kineticore_H
    #define _kineticore_H

    #define	MAX_BOUNCE		   3
    #define	BOUNCE_DAMAGE	   (10 / MAX_BOUNCE)

typedef struct  kcoreHook_s
{
    int     bounce_count;
    float   expire_time;
	userEntity_t *owner;
} kcoreHook_t;

DllExport void weapon_kineticore(userEntity_t *self);
DllExport void ammo_kineticore (userEntity_t *self);

void            kineticore_use(userInventory_s *ptr, userEntity_t *self);
short           kineticore_select (userEntity_t *self);
userInventory_t *kineticore_give (userEntity_t *self, int ammoCount);
void            kineticore_reload (userEntity_t *self);
void            kineticore_ammo(userEntity_t *self);
void *kineticore_command (struct userInventory_s *ptr, char *commandStr, void *data);
void spawnIce(userEntity_t *self);
void iceThink(userEntity_t *self);
void iceMelt(userEntity_t *self);
void iceTouch(userEntity_t *self,userEntity_t *other,cplane_t *plane,csurface_t *surf);

extern weaponInfo_t kineticoreWeaponInfo;

#endif