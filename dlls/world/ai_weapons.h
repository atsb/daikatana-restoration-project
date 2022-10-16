#ifndef	AI_WEAPONS_H
#define	AI_WEAPONS_H

//////////////////////////////////////////////////////////////////////////////////
//	generalized functions for attacking monsters
//////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//	defines
////////////////////////////////////////////////////////////////////////////////
//NSS[10/28/99]:SSG Made these on the player weapons side.. I am just sharing the knowledge on the
// AI side of the universe.
enum
{
	SM_BLOOD,
	SM_BULLET,
	SM_SCORCH,
	SM_FIST,
	SM_EARTH,
	SM_CLAW,
	SM_COUNT
};
////////////////////////////////////////////////////////////////////////////////
//	typedefs
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//  Wyndrax Defines
//
///////////////////////////////////////////////////////////////////////////////
#define WYNDRAX_MAX_LIGHTNING    20        // max lightning spawnable by a wisp
#define WYNDRAX_MAX_ENT_LIST     10       // max # of ents to choose to zap
#define WYNDRAX_MAX_DUMMY        10

typedef struct
{
    float killtime;                                          // when to kill wisp
    short lightningCount;                                    // # lightning bolts it's spawned
    short dummyCount;
    CVector forward,up;
    short sinofs;
    float sinetime;
	float Personality;
    userEntity_t dummyList[WYNDRAX_MAX_DUMMY];
    userEntity_t *lightningList[WYNDRAX_MAX_LIGHTNING];      // list of lightning ents
} wyndraxHook_t;

typedef struct
{
    float killtime;
    userEntity_t *owner,*src,*dst,*altdst;
} lightningHook_t;

///////////////////////////////////////////////////////////////////////////////
//
//  ai weapon hooks
//
///////////////////////////////////////////////////////////////////////////////
typedef struct
{
	CVector Delta_Angles;
	float time_to_live;
	float delta_alpha;
	float delta_scale;
}ZapFlare_t;

typedef struct
{
	CVector Delta_Angles;
	float time_to_live;
	float damage;
	float rnd_damage;
}FlameSword_t;


typedef struct
{
	float time_to_live;
	float a_counter;
	float r_speed ;
} rocketHook_t;

typedef struct
{
	float time_to_live;
	float roll_speed;
	float pitch_speed;
	float yaw_speed;
	float damage;
	float bounce_count;
	float bounce_max;
	float speed_max;
} staveHook_t;

typedef struct
{
	int		delta_angles;
	int		delta_color;
	int		color_dir;
	float	scale_max;
	float	scale_min;
	float	time_to_live;
	float	damage;
	float	delta_scale;
	float	delta_alpha;
	float	c_fov;
	CVector kick_angles;
	userEntity_t *Psyclaw;
} psyclawHook_t;

typedef struct
{
	float	time_to_live;
	int		type;
	float	delta_angles;
	float	delta_scale;
} ThunderJizHook_t;

typedef struct
{
	weapon_t	weapon;
	int		base_damage;
	int		rnd_damage;
	int		distance;
	float	spread_x;
	float	spread_z;
	float	speed;
	
	CVector	ofs;
} ai_weapon_t;

typedef struct
{
	float			brightness;
	float			length;
	float			rotation_speed;
    float			search_speed;
	float			search_angle;
	float			search_max;
	float			search_min;
	userEntity_t*	flare;
} camlightHook_t;

////////////////////////////////////////////////////////////////////////////////
//	prototypes
////////////////////////////////////////////////////////////////////////////////

CVector			ai_fire_bullet (userEntity_t *self, userEntity_t *target, ai_weapon_t *weapon);

int				ai_check_projectile_attack (userEntity_t *self, userEntity_t *target, ai_weapon_t *weapon, float min_dist);

////////////////////////////////////////////////////////////////////////////////
//	monster weapons
////////////////////////////////////////////////////////////////////////////////

void			psiblast_fire (userInventory_s *ptr, userEntity_t *self);
void			melee_punch (userInventory_s *ptr, userEntity_t *self);
void			melee_swing (userInventory_s *ptr, userEntity_t *self);
void            melee_fist( userInventory_s *ptr, userEntity_t *self );
void			chaingun_fire (userInventory_s *ptr, userEntity_t *self);
void			uzigun_fire (userInventory_s *ptr, userEntity_t *self);
void			pistol_fire (userInventory_s *ptr, userEntity_t *self);
void			boargun_fire (userInventory_s *ptr, userEntity_t *self);
void			boar_rocket_attack( userInventory_s *ptr, userEntity_t *self );
void            shotgun_fire( userInventory_s *ptr, userEntity_t *self );
void			dart_fire (userInventory_s *ptr, userEntity_t *self);
void			dart_strafe_fire (userInventory_s *ptr, userEntity_t *self);
void			poison_bite_mild (userInventory_s *ptr, userEntity_t *self);
void			cryo_spray(userInventory_s *ptr, userEntity_t *self);
void			rocket_attack (userInventory_s *ptr, userEntity_t *self);
void			mp_rocket_left_attack (userInventory_s *ptr, userEntity_t *self);
void			mp_rocket_right_attack (userInventory_s *ptr, userEntity_t *self);
void			vermin_rocket_attack( userInventory_s *ptr, userEntity_t *self );
void			magic_arrow_attack (userInventory_s *ptr, userEntity_t *self);
void			laser_fire (userInventory_s *ptr, userEntity_t *self);
void			deathlaser_fire (userInventory_s *ptr, userEntity_t *self);
void			sludge_attack (userInventory_s *ptr, userEntity_t *self);
void			wack_back (userInventory_s *ptr, userEntity_t *self);
void			wack_left (userInventory_s *ptr, userEntity_t *self);
void			wack_right (userInventory_s *ptr, userEntity_t *self);
void			lightning_attack (userInventory_s *ptr, userEntity_t *self);
void			lightning_punch (userInventory_s *ptr, userEntity_t *self);
void			spear_attack (userInventory_s *ptr, userEntity_t *self);
void			throwing_axe_attack (userInventory_s *ptr, userEntity_t *self);
void			throwing_knife_attack (userInventory_s *ptr, userEntity_t *self);
void			laserbeam_attack( userInventory_s *ptr, userEntity_t *self );
void			light_fire(userInventory_s* ptr, userEntity_t* self);
void			venom_spit(userInventory_s* ptr, userEntity_t* self);
void			medusa_spit(userInventory_s* ptr, userEntity_t* self);
void			rock_throw(userInventory_s* ptr, userEntity_t* self);
void			psyclaw_attack (userInventory_s *ptr, userEntity_t *self);
void            nharre_summon (userInventory_s *ptr, userEntity_t *self);
void            nharre_summon_demon (userInventory_s *ptr, userEntity_t *self);
void            garroth_summon (userInventory_s *ptr, userEntity_t *self);
void            kage_slice( userInventory_s *ptr, userEntity_t *self );
void			stave_attack (userInventory_s *ptr, userEntity_t *self);
void			FlameSword_attack( userInventory_s *ptr, userEntity_t *self );

//              ********ThunderJiz Start********
void			ThunderJiz_attack( userInventory_s *ptr, userEntity_t *self );
//              ********ThunderJiz End********

//              ********Fireball Start********
void			fireball_attack( userInventory_s *ptr, userEntity_t *self );
void			fireball_touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf );
void			fireball_think( userEntity_t *self );
void			fireball_explode( userEntity_t *self );
//              ********Fireball End  ********

//              ********FrogSpit Start********
void			FrogSpit_Attack( userInventory_s *ptr, userEntity_t *self );
void			FrogSpit_Touch( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf );
void			FrogSpit_Think( userEntity_t *self );
//              ********FrogSpit End  ********

//              ********WyndraxWisp Start********
void			WyndraxWisp_Attack(userInventory_s *ptr, userEntity_t *self );
//              ********WyndraxZap End********

//              ********WyndraxZap Start********
void			WyndraxZap_Attack(userInventory_s *ptr, userEntity_t *self );
//              ********WyndraxZap Endt********

//				********Kage's Special Attacks*******
void			Kage_Bouncing_Blast( userInventory_s *ptr, userEntity_t *self );
//				********Kage's Special Attacks*******

//				********LaserGat Attack*******
void			lasergat_fire( userInventory_s *ptr, userEntity_t *self);
//				********LaserGat Attack*******

void			ai_fire_playerWeapon( userEntity_t *self, const CVector &org, const CVector &end, float spread_x, float spread_z, int fire_frame );
void			ai_fire_curWeapon( userEntity_t *self );
//userInventory_t	*ai_weapon_create (invenList_t *list, char *weapon_name, invenUse_t useFunc, invenCommand_t cmdFunc);
//void			*ai_weapon_command (struct userInventory_s *ptr, char *commandStr, void *data);

void			ai_init_weapons();
userInventory_t	*ai_init_weapon(userEntity_t *self, float base_damage, float rnd_damage,
								float spread_x, float spread_z, float speed, float dist, const CVector &ofs, 
								char *name, weaponUse_t use_func, unsigned long flags);

void punch_DoDamage( userEntity_t *self, userEntity_t *pEnemy, ai_weapon_t *weapon );

#endif