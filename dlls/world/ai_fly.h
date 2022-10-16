#ifndef	AI_FLY_H_
#define	AI_FLY_H_


///////////////////////////////////////////////////////////////////////////////
//	function prototypes
///////////////////////////////////////////////////////////////////////////////

int		ai_should_fly (userEntity_t *self);
float	ai_fly_choose_turn (userEntity_t *self, Vector dest, Vector normal);
void	ai_flytogoal (userEntity_t *self, float speed, int move_dir);

#endif