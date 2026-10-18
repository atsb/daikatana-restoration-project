#ifndef _DK_MELEE_H
#define _DK_MELEE_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------
#define DK_MELEE_MAX_HITS     2

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
enum 
{
	SND_CH_MIN,
	SND_CH_SWIPE = SND_CH_MIN,
	SND_CH_HIT,
	SND_CH_MAX
};

typedef struct {
	weapon_t weapon;                       // weapon_t must be first entry

	int		lastSnd[SND_CH_MAX];
	short	meleeNum;
	short	hitNum;
} dk_melee_t;

typedef struct {
	short	damageFrame;
	float	damage;
	int		sound;								// preferred sound or -1 for random
	int		nextAnim;
} dk_meleeHitInfo_t;

typedef struct {
   short anim;
   dk_meleeHitInfo_t hitInfo[DK_MELEE_MAX_HITS];
} dk_meleeInfo_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t dk_meleeWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
userInventory_t *d1_give(userEntity_t *self, int ammoCount);
short d1_select(userEntity_t *self);
void *d1_command(struct userInventory_s *ptr, char *commandStr, void *data);
void d1_use(userInventory_s *ptr, userEntity_t *self);
void d1_think(userEntity_t *self);
void d1_touch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);

#endif