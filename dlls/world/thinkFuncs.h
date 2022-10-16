#ifndef	THINKFUNC_H_
#define THINKFUNC_H_

///////////////////////////////////////////////////////////////////////////////////
//	exported functions
///////////////////////////////////////////////////////////////////////////////////

userEntity_t	*thinkFunc_add (userEntity_t *owner, think_t think_func, float time);
void			thinkFunc_remove (userEntity_t *ent);
void			thinkFunc_set_think (userEntity_t *ent, think_t think_func, float time);
void			thinkFunc_run_thinks (void);
void			thinkFunc_remove_thinks (void);
void			thinkFunc_init (void);

#endif