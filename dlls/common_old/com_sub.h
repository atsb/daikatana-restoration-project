#ifndef _SUB_H
#define _SUB_H

#include "user.h"
#include "hooks.h"
#include "com_vector.h"

//////////////////////////////////////
//	defines
//////////////////////////////////////

#define	DAMAGE_NONE			0
#define	DAMAGE_EXPLOSION	1
#define	DAMAGE_INERTIAL		2
#define	DAMAGE_SQUISH		3
#define	DAMAGE_DROWN		4
#define	DAMAGE_LAVA			5
#define	DAMAGE_SLIME 		6
#define	DAMAGE_FALLING		7
#define	DAMAGE_POISON		8
#define	DAMAGE_TELEFRAG		9
#define	DAMAGE_TRIGGER		10
#define	DAMAGE_SIDEWINDER	11

//////////////////////////////////////
// standard weapon and ammo structures
//////////////////////////////////////

typedef struct
{
	// all these fields need to remain the same for compatibility between weapons
	char	*name;
	void	(*use)(struct userInventory_s *ptr, userEntity_t *user);
	void	*(*command)(struct userInventory_s *ptr, char *commandStr, void *data);
	void	*precache;
	void	*dummy2;
	void	*dummy3;
	void	*dummy4;

	// ammount of ammo left
	int		count;
} ammo_t;

typedef struct
{
	// all these fields need to remain the same for compatibility between weapons
	char    *name;
	void    (*use)(struct userInventory_s *ptr, userEntity_t *user);
	void	*(*command)(struct userInventory_s *ptr, char *commandStr, void *data);
	void	*precache;
	void	*dummy3;
	void	*dummy4;

	// pointer to the ammo inventory
	ammo_t	*ammo;
} weapon_t;

////////////////////////////////////////////////////////////////
// defines
////////////////////////////////////////////////////////////////

#define	PITCH	0
#define	YAW		1
#define	ROLL	2

////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////

extern	Vector			forward, right, up;
extern	userTrace_t		trace;
extern	serverState_t	*gstate;
extern	vec3_t			zero_vector;

////////////////////////////////////////////////////////////////
// prototypes
////////////////////////////////////////////////////////////////

float	Sub_AngleMod (float ang);
float	Sub_VecToYaw (Vector &vec);
void	Sub_VecToAngles (Vector &vec1, Vector &vec2);
float	Sub_YawDiff (float facing_yaw, float to_targ_yaw);
void	Sub_BestDelta (float start_angle, float end_angle, float *best_delta, float *angle_diff);

void	Sub_GetMinsMaxs (userEntity_t *self, Vector &org, Vector &mins, Vector &maxs);
void	Sub_ChangeYaw (userEntity_t *ent);
void	Sub_ChangePitch (userEntity_t *ent);

void	Sub_SetMovedir(userEntity_t *self);
void	Sub_SpawnMarker (userEntity_t *self, Vector &org, char *model_name, float time);

void	Sub_Damage (userEntity_t *target, userEntity_t *inflictor, userEntity_t *attacker, float damage, int damage_type);
int		Sub_RadiusDamage (userEntity_t *inflictor, userEntity_t *attacker, userEntity_t *ignore, float damage, int damage_type);
int		Sub_ValidTouch (userEntity_t *self, userEntity_t *other);
void	Sub_UseTargets (userEntity_t *self);
void	Sub_Health (userEntity_t *self, float give, float max_health);
void	Sub_Respawn (userEntity_t *self);

// routines for vec3_t (Quake engine) vector types
void	Sub_ZeroVector (vec3_t v);
void	Sub_SetVector (vec3_t v, float f1, float f2, float f3);
void	Sub_MultVector (vec3_t v, float f1, vec3_t v2);
void	Sub_PrintVector(char *str, Vector &vec);
void	Sub_CopyVector (vec3_t v, vec3_t v2);

int				Sub_Visible (userEntity_t *self, userEntity_t *ent);
userEntity_t	*Sub_FindEntity (char *name);
userEntity_t	*Sub_FindClosestEntity (userEntity_t *self, char *name);
userEntity_t	*Sub_FindTarget (char *name);
userEntity_t	*Sub_DeathmatchSpawnPoint (char *className);
userEntity_t	*Sub_SelectSpawnPoint (char *className, char *target);


void	Sub_ClientPrint (userEntity_t *self, char *msg);
void	Sub_AmbientVolume (userEntity_t *self, Vector &org, int volume);
void	Sub_SetRespawn (userEntity_t *self, float time, int reset_ambient, int ambient_vol);

void	Sub_CalcBoosts (userEntity_t *self);
void	Sub_UpdateClientAmmo (userEntity_t *self);

void	Sub_Showbbox (userEntity_t *self, Vector &org, Vector &mins, Vector &maxs);
void	Sub_GetFrames (userEntity_t *self, char *seq_name, int *startFrame, int *endFrame);

userEntity_t	*Sub_ClientVisible (userEntity_t *self);

void	Sub_Warning (char *fmt, ...);
void	Sub_Error (char *fmt, ...);

void	Sub_SetClientModels (userEntity_t *self, userEntity_t *head, userEntity_t *torso, userEntity_t *legs, userEntity_t *weapon);
float	Sub_AngleDiff (float ang1, float ang2);
void	Sub_Normal (Vector v1, Vector v2, Vector v3, Vector &out);
void	Sub_DrawLine (userEntity_t *self, userEntity_t *target, Vector start, Vector dir);

userEntity_t	*Sub_SpawnFakeEntity ();
void			Sub_RemoveFakeEntity (userEntity_t *ent);

void	Sub_Poison (userEntity_t *target, float damage, float time, float interval);
void	Sub_TeleFrag (userEntity_t *self, float damage);
int		Sub_GenID (void);

#endif