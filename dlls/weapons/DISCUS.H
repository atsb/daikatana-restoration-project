#ifndef _DISCUS_H
    #define _DISCUS_H

DllExport void weapon_discus(userEntity_t *self);

extern weaponInfo_t discusWeaponInfo;

typedef struct
{
    userEntity_t	*owner;             // owner of discus
//   CVector v_angle;                  // current movement angle
    CVector			forward;
    float			droptime;                  // when to drop to the ground
    CVector			lastpos;                  // pos on last frame
    float			speed;
    short			sSeekTarget;           // seek or just bounce?
    userEntity_t    *target;
	int				clear;			// we clear to do some damage?
	short			reflected;		// uhh, did it reflect yet?
} discusHook_t;

typedef struct
{
    weapon_t weapon;

    short bladesout;
} discus_t;

void discusMelee(userEntity_t *self);
userInventory_t *discusGive(userEntity_t *self, int ammoCount);
short discusSelect(userEntity_t *self);
void discusSelectFinish(userEntity_t *self);
void *discusCommand (struct userInventory_s *ptr,char *commandStr,void *data);
void discusUse (userInventory_t *ptr, userEntity_t *self);
void discusShoot(userEntity_t *self);
void discusThink(userEntity_t *self);
void discusTouch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);
void dropDiscus(userEntity_t *self);
void discusPickup(userEntity_t *self, userEntity_t *other);
void spinDiscus(userEntity_t *self);
void catchDiscus(userEntity_t *self, userEntity_t *other);
void discusTrailPoint(userEntity_t *self,short type,CVector &position);
void discusRemove(userEntity_t *self);
void weapon_discus(userEntity_t *self);

#endif