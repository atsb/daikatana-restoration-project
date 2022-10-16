#ifndef _METAMASER_H
    #define _METAMASER_H

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------
    #define META_FIRE_RATE	       0.5
    #define META_CHARGE_TIME	   1.4
    #define META_TRACK_TIME		   3.0
	#define	META_ATTACK_TIME		0.4
    #define META_MAX_TARGETS	   12
	#define META_MAX_ATTACKS	   4
    #define META_MAX_CHARGES	   120
    #define META_SEARCH_TIME	   0.1
	#define META_PAIN_TIME			1.5
	#define META_MAX_HEALTH			1000
	#define META_HEALTH_LEVEL		300
	#define META_DIE_TIME			5.0		// duration of death sequence
	#define META_RING_TIME			2.0		// lifetime of metarings
	#define META_RING_MAX_SCALE		17
	#define META_RING_INIT_RAD		25
	#define META_RING_MAXDIST		(META_RING_INIT_RAD * META_RING_MAX_SCALE)
	#define META_MAX_LASERS			4		// number of death lasers
	#define META_LASER_DAMAGE		0.2		// every 0.2 sec
	#define META_END_LASER_DAMAGE	1.0		// death lasers
	#define META_RING_DAMAGE		1.0		// rings!

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
typedef struct
{
    weapon_t weapon;                       // weapon_t must be first entry
} metamaser_t;

typedef struct
{
    userEntity_t *target;
    float end_track_time;
    float next_sound_time;
    float next_damage_time;
} metaTarget_t;

typedef struct 
{
	short index;
	bool live;
} laserTarget_t;

typedef struct
{
	userEntity_t *owner;
	float rmin;
	float rmax;
	float start_time;
} metaring_t;

typedef struct
{
    float remove_time;						// also controls die sequence...

    float last_touch_time;
    float last_water;
    float change_time;
    float rotation_scale;
    float mass_scale;

    CVector hit_water_pos;

    int charges;

    metaTarget_t targets[META_MAX_TARGETS];
	metaTarget_t acquired[META_MAX_ATTACKS];
	int numTargets;
	int numAcquired;
	int maxAcquired;
	float pain_end_time;					// also controls explosions
	float next_pain_level;
	userEntity_t *owner;

	trackInfo_t tinfo;
	userEntity_t *lasers[META_MAX_LASERS];

} metamaserHook_t;

//---------------------------------------------------------------------------
// globals
//---------------------------------------------------------------------------
extern weaponInfo_t metamaserWeaponInfo;

//---------------------------------------------------------------------------
// function prototypes
//---------------------------------------------------------------------------
DllExport void weapon_metamaser(userEntity_t *self);
DllExport void ammo_metamaser (userEntity_t *self);

userInventory_t *metamaser_give (userEntity_t *self, int ammoCount);
short metamaser_select(userEntity_t *self);
void *metamaser_command(struct userInventory_s *ptr, char *commandStr, void *data);
void metamaser_use(userInventory_s *ptr, userEntity_t *self);
void metamaser_touch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);
void metamaser_startLaser(userEntity_t *self);
void metamaser_think(userEntity_t *self);
void metamaser_shoot_func(userEntity_t *self);

void metamaser_waterFriction(userEntity_t *self);
void metamaser_initTargets(userEntity_t *self);
void metamaser_addTarget(userEntity_t *self, userEntity_t *other);
void metamaser_removeTarget(userEntity_t *self, userEntity_t *other);
void metamaser_remove(userEntity_t *self);
void metamaser_die(struct edict_s *self, struct edict_s *inflictor, struct edict_s *attacker, int damage, CVector &point);
void metamaser_pain(struct edict_s *self, struct edict_s *other, float kick, int damage);
void metamaser_track(userEntity_t *self);
void metamaser_search(userEntity_t *self);
void metamaser_wait(userEntity_t *self);
void metamaser_update_flags(userEntity_t *self, bool bNoTracks);
void metamaser_set_target(int index, long value, trackInfo_t *tinfo);
void metamaser_acquire(userEntity_t *self);
void metamaser_acquire_target(userEntity_t *self, userEntity_t *target);
void metamaser_unacquire_target(userEntity_t *self, userEntity_t *target);
void metamaser_diethink (userEntity_t *self);
void metamaser_spawn_laser_targets(userEntity_t * self, int count);
void metamaser_laser_target_think(userEntity_t *self);
void metamaser_spawn_metaring(userEntity_t *self);
void metaring_think(userEntity_t *self);
void metamaser_push_entity(userEntity_t *ent, CVector &push, float magnitude);

#endif