#ifndef _NIGHTMARE_H
    #define _NIGHTMARE_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------

#define NIGHTMARE_MAX_ATTACKS	10
#define REAPER_TOP_FRAME		(14.0)
#define REAP_MAX_LOOKS        32.0        // 16.0
#define REAP_MIN_DIST         50

typedef struct lookInfo_s
{
    float dist;             // distance to obstruction from this angle
    float dist3;            // average dist of prev, current, and next angle
    CVector forward;         // forward vector for this angle
} lookInfo_t;

typedef struct
{
	userEntity_t *ent;
	userEntity_t *pent;
} markedEnt_t;

typedef struct
{
	markedEnt_t		shooter;
	markedEnt_t		marked[NIGHTMARE_MAX_ATTACKS];		// 'marked' players
	int				curDeath;
	freezeEntity_t  frozen;
	CVector			vangle;
	short			reaped;
} nmControllerHook_t;

typedef struct
{
	userEntity_t	*shooter;
	userEntity_t	*controller;
	short			killPent;
	short			glow;
	short			blood;
	float			fadeTime;
	float			roll;
} pentHook_t;



//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t nightmareWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
DllExport void weapon_nightmare(userEntity_t *self);

userInventory_t *nightmareGive (userEntity_t *self, int ammoCount);
short nightmareSelect(userEntity_t *self);
void *nightmareCommand(struct userInventory_s *ptr, char *commandStr, void *data);
void nightmareUse(userInventory_s *ptr, userEntity_t *self);
void nightmareShootFunc(userEntity_t *self);
void nightmareControllerRemove(userEntity_t *self);
void nightmareControllerTracePent(userEntity_t *self);
void nightmareControllerSearch(userEntity_t *self);
void nightmareControllerBeatDown(userEntity_t *self);
void nightmareControllerUpkeep(userEntity_t *self);
void tracePentagram(markedEnt_t *markedEnt, userEntity_t *self, userEntity_t *shooter, userEntity_t *controller);
void doPentagram(userEntity_t *self);
void flameSpawn(userEntity_t *self);
void nightmareAlmostDeath(userEntity_t *reaper);
void nightmareScaredStiff(userEntity_t *self);
void timeToDie(userEntity_t *self);
void initReaperAttack(userEntity_t *self);
void nightmareDeath(userEntity_t *self);
void removeReaper(userEntity_t *self);
void findUnobstructedDirection(userEntity_t *ent,CVector &forward);

#endif