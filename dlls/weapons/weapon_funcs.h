#ifndef _WEAPONFUNCS_H
#define _WEAPONFUNCS_H

#include	"../world/chasecam.h"
#include	"weapondefs.h"

//unix - DllExport
#if _MSC_VER
#define DllExport	extern "C"	__declspec( dllexport )
#else
#define DLLExport_Plus
#define DllExport extern "C"
#endif

//////////////////////////////
// globals
//////////////////////////////

extern int mike;
extern float mikef;

extern float sin_tbl[];
extern float cos_tbl[];

extern weaponInfo_t *weaponList[WP_NUM_EPISODES+1][WP_WEAPONS_PER_EP];

extern CVector std_angles;

extern autoAim_t autoAim;

enum
{
    SELECT_TARGET_PATH = 1,
    SELECT_TARGET_RANGE
};

//////////////////////////////
// prototypes
//////////////////////////////
void        weapon_PowerSound   (userEntity_t *self);
float       crnd                (void);
float       lim_rand            (int limit);
void        weapon_animate      (userEntity_t *self, int start, int end, float frametime);
int         weapon_trace_attack ( userEntity_t *self, int damage, int num_bullets, int distance, short flags);
//unix - fixed incorrect declaration of weapon_touch
//void        weapon_touch        (userEntity_t *self, userEntity_t *other);
void    	weapon_touch (userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf);
ammo_t      *weapon_check_ammo  (userEntity_t *self, char *ammo_name, int ammo_count);

void        spawn_sprite_explosion (userEntity_t *self, CVector &org, int play_sound);
void        spawn_ring_explosion (userEntity_t *self, CVector &org, int play_sound);
void        spawn_smoke_trail (userEntity_t *self, CVector &org, int play_sound);

void spawnPolyExplosion(CVector &org, float scale, float lsize, CVector &color, short flags);
void spawnPolyExplosion(CVector &org,CVector &vec,float scale,float lsize,CVector &color,short flags);
void weaponRicochet(CVector &vecPos);
void weaponGunShot(CVector &vecOrg, CVector &vecDir);

#define TARGET_ALWAYS			0X1000		// always a valid target
#define TARGET_ALWAYS_IF_ALIVE  0X2000		// always valid if not dead
#define TARGET_NEVER			0x0000		// never a valid target
#define TARGET_SINGLE			0X0001		// valid in single player
#define TARGET_DM				0X0002		// valid in deathmatch
#define TARGET_COOP				0X0004		// valid in coop
#define TARGET_TEAM				0x0008		// a teammate of self is a valid target (ONLY FOR C/B)
#define TARGET_SELF				0X0010		// target == self is valid (only for C/B)
#define TARGET_OWNER			0X0020		// target == self->owner is valid (only vor C/B)
short validateTarget(userEntity_t *self, userEntity_t *target, int ifVisible, int Cflags, int Mflags, int Bflags, int Oflags);
userEntity_t *selectTarget(userEntity_t *entSelf, CVector &vecDeviation, int, userEntity_t *);
////////////////////////////////////////////////////////////////////////////////////
//	weapon_list functions
////////////////////////////////////////////////////////////////////////////////////

weaponListHeader_t  *weapon_list_init       (weaponListHeader_t *list);
void                weapon_list_add         (userEntity_t *self, weaponListHeader_t *list);
void                weapon_list_delete_node (weaponListHeader_t *list, weaponListNode_t *node);
void                weapon_list_remove      (userEntity_t *self, weaponListHeader_t *list);
userEntity_t        *weapon_list_ent        (weaponListNode_t *node);
weaponListNode_t    *weapon_list_node_next  (weaponListNode_t *node);
weaponListNode_t    *weapon_list_node_prev  (weaponListNode_t *node);

////////////////////////////////////////////////////////////////////////////////////
//	projectile functions
////////////////////////////////////////////////////////////////////////////////////

void            weapon_face_velocity        (userEntity_t *self);
userEntity_t    *weapon_spawn_projectile    (userEntity_t *self, CVector &org, float xy_vel, float xangle, char *className, qboolean fAutoAim = TRUE);
trace_t weaponTraceShot(userEntity_t *entShooter, CVector &vecShotOffset, float fSpeed );

//---------------------------------------------------------------------------
// general functions
//---------------------------------------------------------------------------
#define WEAPON_VALID_ADD(ent)	( ent && (ent->flags & FL_CLIENT) && ent->userHook )
#define WEAPON_HIT(ent,num)		if (WEAPON_VALID_ADD(ent)) { ((playerHook_t *)ent->userHook)->hit+=num; }
#define WEAPON_FIRED(ent)		if (WEAPON_VALID_ADD(ent)) { ((playerHook_t *)ent->userHook)->fired++; }

int weaponDllInit(weaponInfo_t *winfo);
void weaponLevelInit(weaponInfo_t *winfo);
weapon_t *weaponCreateInv(invenList_t *inventory, weaponInfo_t *winfo, usew_t use_func, command_t cmd_func, short struct_size);
weapon_t *weaponCreateInv2(weaponInfo_t *winfo, select_t use_func,command_t cmd_func,short struct_size);
ammo_t *ammoCreate(invenList_t *inventory, weaponInfo_t *winfo);
ammo_t *ammoAdd(userEntity_t *self,int amount,weaponInfo_t *winfo);
void ammo_set_sound(userEntity_t *self, int sound);
void *weaponGenericCommand(userInventory_t *inv,char *commandStr,void *data);
void *winfoGenericCommand(weaponInfo_t *winfo,userInventory_t *inv,char *commandStr,void *data);
void *ammoGenericCommand(userInventory_t *ptr,char *commandStr,void *data);
CVector weaponHandVector(userEntity_t *ent,CVector &fpos,CVector &hpos);
userEntity_t *selectWeapon(userEntity_t *owner,char *weaponName,char *modelName, byte enable_crosshair);
void weaponEntRemove(userEntity_t *self);
void weaponEntThink(userEntity_t *self);
void frameUpdate(userEntity_t *self);
void new_weapon_animate (userEntity_t *self, int start, int end, float frametime);
void winfoAnimDelay(weaponInfo_t *winfo,userEntity_t *self,short animNum,float delay);
void winfoAnimate(weaponInfo_t *winfo,userEntity_t *ent,short anim,unsigned short frameflags,float frametime);
void entAnimate(userEntity_t *ent,short first,short last,unsigned short frameflags,float frametime);
short unlimitedAmmo(void);
//short weaponUseAmmo(userEntity_t *self,int count,short autochange);
short weaponHasAmmo(userEntity_t *self,short autochange,short max_shots = 1);  // 3.5 dsn added max_shots, default is 1
short weaponUseAmmo(userEntity_t *self,short autochange);
void weaponAutoChange(userEntity_t *self);
userInventory_t *weaponGive(userEntity_t *self,weaponInfo_t *winfo,short ammoCount);
short weaponSelect(userEntity_t *self,weaponInfo_t *new_winfo);
void weaponSelectFinish(userEntity_t *self);
qboolean entityHuman(userEntity_t *self);
void winfoClientExplosion(weaponInfo_t *winfo,CVector &pos,CVector &angle,float scale,short model,short anim,short flags,short light,CVector &color);
void winfoClientScaleAlpha(weaponInfo_t *winfo,CVector &pos,CVector &angle,float scale,short model,short anim,
                           float alpha_scale,float alpha_end,short light,CVector &color);
void winfoClientScaleRotate(weaponInfo_t *winfo,CVector &pos,float start_scale,short end_scale,float step_scale,int model);
void winfoClientScaleAlphaRotate(weaponInfo_t *winfo,CVector &pos,CVector &angle,CVector &rot,float scale_start,float scale_end,short model,
								float alpha_start, float dietime);
void trackLight(userEntity_t *ent,float size,float r,float g,float b,int fxflags);
void trackLightOfs(userEntity_t *ent,float size,float r,float g,float b,CVector &ofs,int fxflags);
void winfoClientScaleImage(weaponInfo_t *winfo,CVector &pos,CVector &angle,float scale_start,float scale_end,short model,float alpha,float killtime);
void untrackLight(userEntity_t *ent);
void trackAddLight(userEntity_t *ent,float size,float r,float g,float b,int fxflags);
void trackRemoveLight(userEntity_t *ent,int flags);
void trackSetAnim(userEntity_t *ent,int modelindex,short numframes,float scale,int fxflags,float frametime);
void trackSetAnimLocal(userEntity_t *ent,int modelindex,short numframes,float scale,int fxflags,float frametime);
void trackAddEffect(userEntity_t *ent,int modelindex,int fxflags);
void trackRemoveEffect(userEntity_t *ent,int fxflags);
//void trackModFlags(userEntity_t *ent,int flags);
void freezeEntity(userEntity_t *ent,freezeEntity_t *freeze);
void unfreezeEntity(userEntity_t *ent,freezeEntity_t *freeze);
void flashClients(userEntity_t *src,float r,float g,float b,float a, float vel=0);
void winfoStopLooped(userEntity_t *self);
void clientSmoke(CVector &pos,float scale,float amount,float spread,float maxvel);
void clientBeamSparks(CVector &pos,float amount,float scale);
void clientSparks(CVector &pos,CVector &dir,int amount,CVector &color);
void clientBlood(CVector &pos,CVector &dir,int amount,CVector &color);
void clientFireSmoke(CVector &pos, float burntime, char firstFrame);
void entBackstep(userEntity_t *ent, float dist);
void entBackstep(userEntity_t *ent,CVector &normal,float dist);
short entStuck(userEntity_t *ent);
CVector turnToTarget(userEntity_t *entSelf, userEntity_t *entTarget, float fTurnRate);
//userEntity_t *checkAutoAim(userEntity_t *self,CVector &shootat);

void weaponMuzzleFlashForMonstersAndBots (userEntity_t *self, playerHook_t *hook);
void weaponTrackMuzzle(trackInfo_t& tinfo, userEntity_t *self, userEntity_t *player, weaponInfo_t *winfo, bool full = false);
void weaponUntrackProjectile(userEntity_t *proj);
void weaponTrackProjectile(trackInfo_t &tinfo, userEntity_t *proj, userEntity_t *player, weaponInfo_t *winfo, short ID, short projFlags, bool full = false);
/*
void weaponMuzzleFlash(userEntity_t *track, userEntity_t *player, weaponInfo_t *winfo, char *flash, 
					   short numFrames, float frametime, float scale, float lightSize, CVector lightColor, 
					   short animFlags, long fxFlags, float alpha = 0, CVector FRU = CVector(0,0,0));*/
void weaponMuzzleFlash(userEntity_t *track, userEntity_t *player, weaponInfo_t *winfo, long flashIdx); 

//	pickup item spawning
int weaponShouldSwitch (weapon_t *new_weapon, weapon_t *old_weapon);
void weaponTouchDefault (userEntity_t *self, userEntity_t *other, cplane_t *plance, csurface_t *surf);
void winfoSetSpawn(weaponInfo_t *winfo,userEntity_t *self,float respawn_time,touch_t touch);
void ammoTouchDefault (userEntity_t *self, userEntity_t *other, cplane_t *plance, csurface_t *surf);
void winfoAmmoSetSpawn(weaponInfo_t *winfo,userEntity_t *self,int count,float respawn_time,touch_t touch);

//	wall damage
void weaponBloodSplat (userEntity_t *target, CVector &start, CVector &end, CVector &impactPos, float dist);
void weaponScorchMark( trace_t &trace, byte cType );
void weaponScorchMark2(userEntity_t *self, userEntity_t *world,cplane_t *plane);

void weapon_kick(userEntity_t *player, CVector kick_angles, float kick_velocity, short fViewVelocity, short fViewVelocityReturn);

void weapprev_Command(userEntity_t *self);
void weapnext_Command(userEntity_t *self);
void weap_Command    (userEntity_t *self);

// bouncing shell
//void shotShell(userEntity_t *entPlayer, weaponInfo_t *pWeaponInfo, int nShellModelIndex, int nShellSoundStartIndex, int nMaxShellSounds, float scale, float z_offset );
void shotShell(userEntity_t *entPlayer, weaponInfo_t *pWeaponInfo, int nShellModelIndex, int nShellSoundStartIndex, int nMaxShellSounds, float scale, CVector &offset );

int EntIsAlive( userEntity_t *self );

void e2_com_Damage (  userEntity_t *target, 
					  userEntity_t *inflictor, 
					  userEntity_t *attacker, 
					  CVector &point_of_impact, 
					  CVector &damage_vec, 
					  float damage, 
					  unsigned long damage_flags );

qboolean winfoCheckMaxCount(userEntity_t *self, weaponInfo_t *winfo);

//--------------------------------------------------------------------------
// stuff for debris
//--------------------------------------------------------------------------

typedef struct debrisInfo_s
{
	csurface_t		*surf;			// specify this
	int				type;			// or this
	short			bRandom;		// only affects debris when 'type' is used
	int				count;
	CVector			org;
	CVector			dir;
	CVector			scale;
	float			scale_delta;	// used to modulate scale.  should be [0,1]
	float			speed;
	float			spread;
	float			alpha;
	float			alpha_decay;	// how fast the decay happens.  this is subtracted every 0.3 
	float			gravity;
	float			delay;
	float			damage;			// does it hurt?  also makes it solid
	userEntity_t	*owner;			// who gets credit for the damage?

	short			bSound;			// play sounds with debris?
	userEntity_t	*soundSource;	// stuff used for sounds
	float			minAttn;		// min attenuation for sound (<1 uses default)
	float			maxAttn;		// max attenuation for sound (<1 uses default)

	short			particles;		// attach some cool particles to the effect! (-1 for no particles!)
	CVector			pColor;			// color of partles!
	float			pScale;			// particle scale
} debrisInfo_t;

typedef struct
{
	float			damage;
	userEntity_t	*owner;
} weaponDebrisHook_t;

enum 
{
	DEBRIS_WOOD = 0,
	DEBRIS_WOOD1 = DEBRIS_WOOD,
	DEBRIS_WOOD2,
	DEBRIS_GLASS,
	DEBRIS_GLASS1 = DEBRIS_GLASS,
	DEBRIS_GLASS2,
	DEBRIS_METAL,
	DEBRIS_METAL1 = DEBRIS_METAL,
	DEBRIS_METAL2,
	DEBRIS_ROBOT,
	DEBRIS_ROBOT1 = DEBRIS_ROBOT,
	DEBRIS_ROBOT2,
	DEBRIS_ROBOT3,
	DEBRIS_ROBOT4,
	DEBRIS_ROCK,
	DEBRIS_ROCK1 = DEBRIS_ROCK,
	DEBRIS_ROCK2,
	DEBRIS_ROCK3,
	DEBRIS_BONE,
	DEBRIS_ICE,
	DEBRIS_SAND,
	DEBRIS_MAX
};
void spawn_surface_debris(debrisInfo_t &di, bool bForceDeathmatch = false);

// SCG[11/24/99]: Save game stuff
void Weapon_SaveHook( FILE *f, struct edict_s *ent, field_t *hook_fields, int nSize );
void Weapon_LoadHook( FILE *f, struct edict_s *ent, field_t *hook_fields, int nSize );

// hit counter for weapons that give and keep on givin
#define MAX_HITS				32
class CHitCounter
{
public:
								CHitCounter(userEntity_t *_self = NULL):self(_self) {memset(&hits,0,sizeof(hits));}
								~CHitCounter();
	void						AddHit(userEntity_t *_self, userEntity_t *_hit);
protected:
	userEntity_t				*self;
	userEntity_t				*hits[MAX_HITS];
};
int	counted_com_RadiusDamage (userEntity_t *inflictor, userEntity_t *attacker, userEntity_t *ignore, CHitCounter *hit, float damage, unsigned long damage_flags, float fRadius);
int	e2_com_RadiusDamage (userEntity_t *inflictor, userEntity_t *attacker, userEntity_t *ignore, float damage, unsigned long damage_flags, float fRadius, CHitCounter *hit = NULL);

#define IS_MONSTER_TYPE(ent,Mtype)		((ent->userHook) ? (((playerHook_t *)ent->userHook)->type == Mtype) : FALSE)
#define IS_FLESHY		0x0001
#define IS_ARMOR		0x0002
#define IS_SWORD		0x0004
short Weapon_is_fleshy(userEntity_t *self);

#define WEAPON_DISPLAY_UPDATE(self)		if (self && (self->flags & FL_CLIENT) && self->client) {self->client->needsUpdate |= HUD_UPDATE_WEAP;}
#endif // _WEAPON_FUNC_H