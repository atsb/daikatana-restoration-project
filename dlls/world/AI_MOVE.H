#ifndef	_AI_MOVE
#define	_AI_MOVE

///////////////////////////////////////////////////////////////////////////////////
//
// Basic movement routines for monsters/bots
//
///////////////////////////////////////////////////////////////////////////////////

#define	DEBUGINFO	1

#define	TER_CLEAR				0
#define TER_GRADEUP				1
#define TER_STEPSUP				2
#define TER_FAR_WALL			3

#define	TER_OBSTRUCTED			5		// obstructed 
#define TER_WALL				6
#define TER_WALL_PARTIAL		7		// not used
#define TER_HIGH_OBS			8		// high obstruction, should lower to get through
#define	TER_WALL_RIGHT			10		// wall blocking to right, but not to left
#define	TER_WALL_LEFT			11		// wall blocking to left, but not to right
#define TER_LOW_OBS				12		// low obstruction, low enough to jump over
#define TER_CEILING				13

#define BLOCKED_LEFT_BY_ENTITY		14
#define BLOCKED_RIGHT_BY_ENTITY		15
#define BLOCKED_ABOVE_BY_ENTITY		16
#define BLOCKED_BELOW_BY_ENTITY		17
#define BLOCKED_BOTH_BY_ENTITY		18	//This is really above, below, left and right but I didn't want to change the name just to make it say all and change all the occurances of it within the code<nss>

// for movetogoal, swimtogoal
#define	MOVE_FORWARD		1
#define	MOVE_BACKWARD		2
#define	MOVE_UP				3
#define	MOVE_DOWN			4
#define	MOVE_STRAFE			5
#define	MOVE_STRAFEFORWARD	6
#define	MOVE_STRAFEBACKWARD	7

#define	MOVE_SPEED	380

#define	yaw_rate	22.5


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// typedefs
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef	struct
{
	int	num_ledges;
	int	ledge_height	[64];
} ledgeList_t;

typedef	struct
{
	float	ledge_z_dist;
	float	ledge_xy_dist;
} ledgeData_t;

//////////////////////////////////////////////////////////////////////////
/// terrain and navigation stuff
/////////////////////////////////////////////////////////////////////////

// terrain checking
typedef struct
{
	float	wall_dist;
	int  	wall_type;
	float	wall_yaw;
	float	wall_pitch;
	CVector	wall_normal;
	int		water_type;

	int		left_blocked;
	int		right_blocked;

	float	left_dist;
	float	right_dist;
	
/*
	float	left_grade;
	float	right_grade;

	float	left_grade_bottom;
	float	right_grade_bottom;

	float	left_grade_top;
	float	right_grade_top;
*/
} terInfo_t;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// globals
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern	ledgeList_t	ledge_list;  // stores status of ledge searches up to 1000 units wide
extern	ledgeData_t	ledge_data;
extern	terInfo_t	terrain;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// proto-toto-woto-typey-wipeys
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// general
//-----------------------------------------------------------------------------
int		ai_check_gap			(userEntity_t *self, terInfo_t ter, float dist);
int		ai_terrain_type			(userEntity_t *self, CVector &dir, float dist);


userEntity_t *ai_spawn_goal		(userEntity_t *self);


//userEntity_t *ai_find_target_client_bot	(userEntity_t *self);
//userEntity_t *ai_find_target_monster_bot	(userEntity_t *self);
//userEntity_t *ai_find_target_berserk		(userEntity_t *self);
//
//int			ai_choose_wander_goal	(userEntity_t *self);
//int			ai_should_follow		(userEntity_t *self);
//
//void		ai_prethink				(userEntity_t *self);
//void		ai_resume_think			(userEntity_t *self);
//
//void		ai_path_wander			(userEntity_t *self);
//void		ai_begin_path_wander	(userEntity_t *self);
//
//void		ai_wander				(userEntity_t *self);
//void		ai_begin_wander			(userEntity_t *self);
//
//void		ai_node_wander			(userEntity_t *self);
//void		ai_begin_node_wander	(userEntity_t *self);
//
//int			ai_path_follow_check_goal	(userEntity_t *self, float dist, int visible);
//void		ai_path_follow				(userEntity_t *self);
//void		ai_begin_path_follow		(userEntity_t *self);

//int			ai_time_follow_check_goal	(userEntity_t *self, float dist, int visible);
//void		ai_time_stamp_follow		(userEntity_t *self);
//void		ai_begin_time_stamp_follow	(userEntity_t *self);

//void		ai_follow		(userEntity_t *self);
//void		ai_begin_follow	(userEntity_t *self);
//
//void		ai_stand		(userEntity_t *self);
//void		ai_begin_stand	(userEntity_t *self);
//
//void		ai_jump			(userEntity_t *self);
//void		ai_begin_jump	(userEntity_t *self);
//
//void		ai_jump_up			(userEntity_t *self);
//void		ai_begin_jump_up	(userEntity_t *self);
//
//void		ai_pain			(userEntity_t *self);
//void		ai_begin_pain( userEntity_t *self, userEntity_t *other, float kick, int damage );
//
//void		ai_die			(userEntity_t *self);
//void		ai_begin_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, const CVector &point );
//
//void		ai_turn			(userEntity_t *self);
//void		ai_begin_turn	(userEntity_t *self);

//-----------------------------------------------------------------------------
// water 
//-----------------------------------------------------------------------------
//int		ai_check_water			(userEntity_t *self, terInfo_t ter, float dist);
//void	ai_water_best_direction (userEntity_t *self);
//void	ai_check_water_jump		(userEntity_t *self);
//void	ai_swimtogoal			(userEntity_t *self, float speed, int move_dir);
void	ai_water_level			(userEntity_t *self);
void	ai_water_damage			(userEntity_t *self);

//-----------------------------------------------------------------------------
// ground
//-----------------------------------------------------------------------------
int		ai_jump_obstructed		(userEntity_t *self, float dist, float vz);
//int		ai_find_ledge			(userEntity_t *self, float start_dist);
int		ai_find_ledge_list		(userEntity_t *self, float start_dist);

int	ai_choose_ledge( userEntity_t *self, userEntity_t *goal, float start_dist );


//-----------------------------------------------------------------------------
// air
//-----------------------------------------------------------------------------





//-----------------------------------------------------------------------------
//		monster initialization functions
//-----------------------------------------------------------------------------

//void			ai_parse_epairs (userEntity_t *self);
//playerHook_t	*ai_init_monster(userEntity_t *self, int type);

void	ai_get_spawn_node	(userEntity_t *self);
void	ai_teleported		(userEntity_t *self);


//-----------------------------------------------------------------------------


//void			ai_begin_transition	(userEntity_t *self, int startFrame, int endFrame, think_t think);

//void			ai_rocket_jump			(userEntity_t *self);
//float			ai_best_direction		(userEntity_t *self);
//int			ai_path_clear			(userEntity_t *self, const CVector &end);
//int			ai_on_plat				(userEntity_t *self);
//MAPNODE_PTR	ai_choose_node			(userEntity_t *self);
//int			ai_choose_roam_goal		(userEntity_t *self);



#endif
