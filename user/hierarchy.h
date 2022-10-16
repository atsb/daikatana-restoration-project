#ifndef HIERARCHY_H_
#define HIERARCHY_H_

///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define	MAX_TORSO_TWIST	60.0
#define	MAX_HEAD_TWIST	35.0
#define	MAX_HEAD_TILT	30.0

///////////////////////////////////////////////////////////////////////////////
//	function prototypes
///////////////////////////////////////////////////////////////////////////////

void	hr_init_frametable (userEntity_t *part);
void	hr_init_model_offsets (userEntity_t *master, userEntity_t *legs, userEntity_t *torso,
							   userEntity_t *head, userEntity_t *weapon);
void	hr_init_models (userEntity_t *self);
void	hr_remove_parts (userEntity_t *self, int force_removal);

void	hr_yaw_follow (userEntity_t *master, userEntity_t *slave, float max_twist);
void	hr_torso_pitch (userEntity_t *master, userEntity_t *slave, float max_tilt);
void	hr_head_follow (userEntity_t *self, userEntity_t *head);
void	hr_snap_legs (userEntity_t *self, userEntity_t *legs);

void	hr_sequence( userEntity_t *self, frameData_t *pSequence );
void    hr_sequence(userEntity_t *self, const char *animationName);
void	hr_stand (userEntity_t *self);
void	hr_stand_legs (userEntity_t *self);
void	hr_attack_torso (userEntity_t *self);
void	hr_run (userEntity_t *self);
void	hr_attack_run (userEntity_t *self);
void	hr_jump (userEntity_t *self);
void	hr_walk (userEntity_t *self);
void	hr_attack (userEntity_t *self);
void	hr_die (userEntity_t *self);

userEntity_t	*hr_gib (userEntity_t *self);

void	hr_hide_parts (userEntity_t *self);
void	hr_show_parts (userEntity_t *self);

DllExport	void	hr_set_weapon_model (userEntity_t *self);

#endif