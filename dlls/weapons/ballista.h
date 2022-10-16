#ifndef _BALLISTA_H
    #define _BALLISTA_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------
    #define BALLISTA_MAX_SKEWER   1
    #define MAX_BALLISTAS         30
    #define BALLISTA_MAX_DROPPED  6

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef struct
{
    weapon_t weapon;                       // weapon_t must be first entry
} ballista_t;

typedef struct
{
	userEntity_t	*ent;
	int				old_flags;
	movetype_t		old_movetype;
	float			release_time;
	CVector			normal;
} ballistaSkewer_t;

typedef struct
{
	bool				stuck;
    float				killtime;
    CVector				velocity;
	CVector				angles;
	ballistaSkewer_t	skewered;
	short				unskeweredCount;
	userEntity_t		*lastSkewered;
	CHitCounter			hits;
} ballistaHook_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t ballistaWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
DllExport void weapon_ballista(userEntity_t *self);
DllExport void ammo_ballista(userEntity_t *self);

userInventory_t *ballistaGive(userEntity_t *self, int ammoCount);
short ballistaSelect(userEntity_t *self);
void *ballistaCommand(struct userInventory_s *ptr, char *commandStr, void *data);
void ballistaUse(userInventory_s *ptr, userEntity_t *self);

short hitMidsection(userEntity_t *self,userEntity_t *other);
void removeBallista(userEntity_t *self, bool bExplode, CVector normal);
void ballista_skewer(userEntity_t *self, userEntity_t *other);
void ballista_unskewer(userEntity_t *self);
void ballistaShoot(userEntity_t *self);
void ballistaThink(userEntity_t *self);
void ballistaTouch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);
void ballistaDie(userEntity_t *self);


#endif