#ifndef _CHASECAM_H
#define _CHASECAM_H

//////////////////////////////////////////////////////////////////////
//	globals
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//	enums and defines
//////////////////////////////////////////////////////////////////////

enum
{
	CAMERAMODE_DEFAULT = 0,
	CAMERAMODE_BESTCHASE = 1,
	CAMERAMODE_MONITOR = 2,
	CAMERAMODE_TEST1 = 3,
	CAMERAMODE_TEST2 = 4,
	CAMERAMODE_NEXT = 5,
	CAMERAMODE_GIBCAM = 99,
	CAMERAMODE_CINEMATIC = 100
};

//////////////////////////////////////////////////////////////////////
//	typedefs
//////////////////////////////////////////////////////////////////////

typedef	struct	cameraHook_s
{
	think_t			pre_think;	// routine called each frame for an
								// active camera
	think_t			post_think;	// routine called each frame for an
								// active camera
	int				mode;
	int				demonic;

	float			follow_dist;
	float			target_dist;
	float			next_change;
	float			min_change_time;
	float			speed;
	float			max_speed;

	CVector			view_angle;	// the current angle we are viewing from
	CVector			ideal_angle;// the final angle we want to view from

	CVector			target_angles;	//	last angles of target
	
	CVector			best_org;
	CVector			best_dir;

	// for taking over the target entity
	think_t			target_think;
	think_t			target_prethink;
	think_t			target_postthink;
	unsigned long	target_ai_flags;
	char			target_th_state;
	movetype_t		target_movetype;

	//	for spawning a fake target if current target gets removed
	CVector			last_target_pos;
	CVector			last_target_angles;
	CVector			last_target_v_angle;

	userEntity_t	*debug_target;
} cameraHook_t;

void	camera_set_viewangle (userEntity_t *client, CVector &ang);

DllExport void	camera_start				(userEntity_t *self, int showMsg);
DllExport void	camera_stop					(userEntity_t *self, int showMsg);

void	camera_toggle				(userEntity_t *self);
void	camera_set_target			(userEntity_t *camera, userEntity_t *target);
void	camera_next_monster			(userEntity_t *self);
void	camera_next_sidekick		(userEntity_t *self); // NSS[2/11/00]:
void	camera_prev_monster			(userEntity_t *self);
void	camera_mode					(userEntity_t *self, int cmode, int showMsg);
void	camera_debug				(userEntity_t *self);
void	camera_toggle_possession	(userEntity_t *self);
void	camera_toggle_gravity		(userEntity_t *self);
void	camera_set_debug_target		(userEntity_t *self);
void	camera_toggle_debug_target	(userEntity_t *self);
void	camera_kill_target			(userEntity_t *self);
void	camera_lock					(userEntity_t *self);
void	camera_force_enemy			(userEntity_t *self);
#endif