#include "world.h"

/////////////////////////////////////////////////////////////////////
//	exports
/////////////////////////////////////////////////////////////////////

DllExport	void	func_anim (userEntity_t *self);

/////////////////////////////////////////////////////////////////////
//	defines
/////////////////////////////////////////////////////////////////////

#define	VISIBLE			1		//	brush starts visible

/////////////////////////////////////////////////////////////////////
//	typedefs
/////////////////////////////////////////////////////////////////////

typedef	struct	animHook_s
{
	float	damage;
} animHook_t;

#define	ANIMHOOK(x) (int)&(((animHook_t *)0)->x)
field_t anim_hook_fields[] = 
{
	{"damage",	ANIMHOOK(damage),	F_FLOAT},
	{NULL, 0, F_INT}
};

/////////////////////////////////////////////////////////////////////
//	prototypes
/////////////////////////////////////////////////////////////////////

////////////////////////////////////
//	func_anim_use_target
//
//
////////////////////////////////////

void	func_anim_use_target (userEntity_t *self)
{
	userEntity_t	*targ;

	if (!self->target)
		return;

	targ = com->FindTarget (self->target);

	if (targ)
	{
		//	Q2FIXME:	activator?!?!?
		targ->use (targ, self, gstate->activator);

		//	make self invisible and non-solid
		self->modelName = NULL;
		self->movetype = MOVETYPE_NONE;
		self->solid = SOLID_NOT;
	}
}


////////////////////////////////////
//	func_anim_use
//
//
////////////////////////////////////

void	func_anim_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	gstate->Con_Printf ("using %s\n", self->targetname);

	//	make self visible
	if (!self->modelName)
	{
		self->modelName = self->message;

		self->movetype = MOVETYPE_PUSH;
		self->solid = SOLID_BSP;

		gstate->SetModel (self, self->modelName);
	}

	if (self->delay <= 0.0)
		self->delay = 0.05;

	self->nextthink = gstate->time + self->delay;
	self->think = func_anim_use_target;
}

////////////////////////////////////
//	func_anim
//
//
////////////////////////////////////

void func_anim (userEntity_t *self)
{
	animHook_t		*hook;
	int				i;

	// get memory for userHook
	self->userHook = gstate->X_Malloc(sizeof(animHook_t),MEM_TAG_HOOK);
	hook = (animHook_t *) self->userHook;

	self->className = "func_rotate";

	//	default 0.1 second delay before using target
	self->delay = 0.1;

	for (i = 0; self->epair [i].key; i++)
	{
		if (!stricmp (self->epair [i].key, "dmg"))
			hook->damage = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "target"))
			self->target = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "killtarget"))
			self->killtarget = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "delay"))
				self->delay = atof (self->epair [i].value);
	}

	self->message = self->modelName;

	if (self->spawnflags & VISIBLE)
	{
		self->solid = SOLID_BSP;
		self->movetype = MOVETYPE_PUSH;
	}
	else
	{
		self->solid    = SOLID_NOT;
		self->movetype = MOVETYPE_NONE;
		
		self->modelName = NULL;
	}
	
    // these two are required
	gstate->SetOrigin(self, self->s.origin[0], self->s.origin[1], self->s.origin[2]);
	gstate->SetModel(self, self->modelName);

	self->use = func_anim_use;
}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_func_anim_register_func()
{
	gstate->RegisterFunc("func_anim_use_target",func_anim_use_target);
	gstate->RegisterFunc("func_anim_use",func_anim_use);
}

