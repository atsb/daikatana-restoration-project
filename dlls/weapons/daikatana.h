#ifndef _DAIKATANA_H
    #define _DAIKATANA_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------
#define DK_MELEE_MAX_HITS		2
//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
enum 
{
	SND_CH_MIN,
	SND_CH_SWIPE = SND_CH_MIN,
	SND_CH_HIT,
	SND_CH_SW_CLANK,
	SND_CH_ARMOR_HIT,
	SND_CH_BODY_HIT,
	SND_CH_MAX
};

typedef struct {
	weapon_t weapon;                       // weapon_t must be first entry

	int		lastSnd[SND_CH_MAX];
	short	meleeNum;
	short	hitNum;
	int		levels;
	float	doneTime;

	short	attacking;
	short	chain;
} daikatana_t;

typedef struct {
	short	damageFrame;						// anim frame to do damage
	short	followThrough;						// frame when the followThrough is done

	float	startF;								// beginning of swipe arc
	float	startR;
	float	startU;

	float	damageF;							// end of swipe arc
	float	damageR;
	float	damageU;

	short	markRoll;							// for the scorch mark!

	float	speed;
	int		sound;								// preferred sound or -1 for random
	int		nextAnim;
} daikatanaHitInfo_t;


typedef struct {
   short anim;
   daikatanaHitInfo_t hitInfo[DK_MELEE_MAX_HITS];
} daikatanaInfo_t;

#define EMPTY_HIT -1,{{-1,-1,NULL_FRU,NULL_FRU,-1,-1,-1,-1}}
#define NULL_FRU 0,0,0

typedef struct
{
	int first;
	int last;
	float vol;
} sound_info_t;

typedef struct
{
	short		sound;
	float		vol;
} amb_info_t;
//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t daikatana_WeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
userInventory_t *daikatana_give(userEntity_t *self, int ammoCount);
short daikatana_select(userEntity_t *self);
void *daikatana_command(struct userInventory_s *ptr, char *commandStr, void *data);
void daikatana_use(userInventory_s *ptr, userEntity_t *self);
void daikatana_think(userEntity_t *self);
void daikatana_touch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);
void daikatana_attack ( userEntity_t *self, daikatanaHitInfo_t &hit);
void daikatana_playSound(userEntity_t *self, int type, int snd);
long daikatana_calcExpBonus( userEntity_t *self, userEntity_t *other);
int calcDKLevel(userEntity_t *self);
void daikatana_levelUp(userEntity_t *self, int newLevel);
void daikatana_ambient(userEntity_t *self);
void daikatana_marks(userEntity_t *self, trace_t &trace, daikatanaHitInfo_t &hit);


#endif