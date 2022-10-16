#ifndef _STAVROS_H
    #define _STAVROS_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef struct
{
    weapon_t weapon;                       // weapon_t must be first entry
} stavros_t;

typedef struct meteorHook_s
{
    short bounceCount;
    float killtime;
    short colorScale;
} meteorHook_t;

typedef struct
{
	CVector Delta_Angles;
	float time_to_live;
	float delta_alpha;
	float delta_scale;
}ZapFlare_t;

typedef struct
{
	userEntity_t *owner;
	float time_to_live;
	float roll_speed;
	float pitch_speed;
	float yaw_speed;
	float damage;
	float bounce_count;
	float bounce_max;
	float speed_max;
} staveHook_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t stavrosWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
DllExport void weapon_stavros(userEntity_t *self);
DllExport void ammo_stavros (userEntity_t *self);

userInventory_t *stavrosGive (userEntity_t *self, int ammoCount);
short stavrosSelect(userEntity_t *self);
void *stavrosCommand(struct userInventory_s *ptr, char *commandStr, void *data);
void stavrosUse(userInventory_s *ptr, userEntity_t *self);

void stavrosShoot(userEntity_t *self);
void stavrosThink(userEntity_t *self);
void stavrosTouch(userEntity_t *self,userEntity_t *other,cplane_t *plane,csurface_t *surf);
void smallMeteorSpawn(userEntity_t *self);
void smallMeteorThink(userEntity_t *self);
void smallMeteorTouch(userEntity_t *self,userEntity_t *other,cplane_t *plane,csurface_t *surf);

#endif