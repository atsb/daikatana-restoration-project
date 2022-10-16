#ifndef	PROJECTILE_FX_H
#define	PROJECTILE_FX_H

// function prototype of all projectile tracker functions
typedef	void (*projectile_fx_func) (trackParamData_t *trackData, int pointContents);

// make an array of all functions for each type of tracked projectile
typedef struct projectile_fx_s
{
	projectile_fx_func			launch;
	projectile_fx_func			fly;
	projectile_fx_func			special;
	projectile_fx_func			die;
} projectile_fx_t;
#define EMPTY_PROJECTILE_FX		{NULL,NULL,NULL,NULL}

projectile_fx_s &GetProjectile_Fx(int idx);

#endif