#include "world.h"
#include "light.h"

////////////////////////////////////////////////////////////////////////////////////
//	exports
////////////////////////////////////////////////////////////////////////////////////

DllExport	void	light(userEntity_t *self);
DllExport	void	light_walltorch (userEntity_t *self);
DllExport	void	light_spot (userEntity_t *self);
DllExport	void	light_strobe (userEntity_t *self);
DllExport	void	light_flare (userEntity_t *self);
//DllExport	void	light_flame (userEntity_t *self);// SCG[1/23/00]: not used
DllExport	void	light_e1 (userEntity_t *self);
DllExport	void	light_e2 (userEntity_t *self);
DllExport	void	light_e3 (userEntity_t *self);
DllExport	void	light_e4 (userEntity_t *self);

////////////////////////////////////////////////////////////////////////////////////
//	defines
////////////////////////////////////////////////////////////////////////////////////

#define	NO_SOUND	1
#define	START_OFF	1
#define	LIGHT_FLARE	2

////////////////////////////////////////////////////////////////////////////////////
//	globals
////////////////////////////////////////////////////////////////////////////////////

// SCG[11/20/99]: Save game stuff
#define	LIGHTHOOK(x) (int)&(((lightHook_t *)0)->x)
field_t light_hook_fields[] = 
{
	{"ambient_sound",	LIGHTHOOK(ambient_sound),	F_LSTRING},
	{"stylestr",		LIGHTHOOK(stylestr),		F_LSTRING},
	{"style",			LIGHTHOOK(style),			F_INT},
	{NULL, 0, F_INT}
};

void light_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, light_hook_fields, sizeof( lightHook_t ) );
}

void light_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, light_hook_fields, sizeof( lightHook_t ) );
}

////////////////////////////////////////////////////////////////////////////////////
// light_use
////////////////////////////////////////////////////////////////////////////////////

void	light_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	lightHook_t		*hook = (lightHook_t *) self->userHook;

	if (self->spawnflags & START_OFF)
	{
		gstate->ConfigString (CS_LIGHTS + hook->style, "m");
		self->spawnflags &= ~START_OFF;
		self->svflags &= ~SVF_NOCLIENT;		// make sure we send this entity to the client
	}
	else
	{
		gstate->ConfigString (CS_LIGHTS + hook->style, "a");
		self->spawnflags |= START_OFF;
		// if this is a light flare, don't send this entity to the client
		if (!stricmp(self->className, "light_flare"))
			self->svflags |= SVF_NOCLIENT;
	}
}

///////////////////////////////////////////////////////////////////////////////
// light_flare_use
///////////////////////////////////////////////////////////////////////////////

void	light_flare_use (userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
//	lightHook_t		*hook = (lightHook_t *) self->userHook;// SCG[1/23/00]: not used

	if (self->spawnflags & START_OFF)
	{
		self->spawnflags &= ~START_OFF;
		self->svflags &= ~SVF_NOCLIENT;		// make sure we send this entity to the client
	}
	else
	{
		self->spawnflags |= START_OFF;
		self->svflags |= SVF_NOCLIENT;		// don't send this entity to the client
	}
}


///////////////////////////////////////////////////////////////////////////////
// light_flare
///////////////////////////////////////////////////////////////////////////////

void light_flare (userEntity_t *self)
{
	int		i;
	char	*model;

	model = "models/global/e_flare2.sp2";

	for (i = 0; self->epair [i].key != NULL; i++)
	{
		if (!stricmp (self->epair [i].key, "model"))
			model = self->epair [i].value;

		else if (!stricmp(self->epair[i].key, "scale"))  // 1.20 dsn
		{
			sscanf( self->epair[i].value, "%f %f %f", 
				&self->s.render_scale.x, 
				&self->s.render_scale.y, 
				&self->s.render_scale.z );
        }
	}

	if( self->s.render_scale.x == 0 ) self->s.render_scale.x = 1;
	if( self->s.render_scale.y == 0 ) self->s.render_scale.y = 1;
	if( self->s.render_scale.z == 0 ) self->s.render_scale.z = 1;

	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	self->s.modelindex = gstate->ModelIndex (model);
	self->s.renderfx = SPR_ALPHACHANNEL | RF_TRANSLUCENT | RF_LIGHTFLARE;

	self->s.mins.Zero();
	self->s.maxs.Zero();

	self->use = light_flare_use;

	// if this is supposed to start off, don't send it to the client
	if (self->spawnflags & START_OFF)
		self->svflags |= SVF_NOCLIENT;
	
	gstate->LinkEntity (self);
}

///////////////////////////////////////////////////////////////////////////////
// light
///////////////////////////////////////////////////////////////////////////////

void light (userEntity_t *self)
{
	int				i;
	lightHook_t		*hook;
	int				style_defined = FALSE;
	char			*model;

	// if this light is inert we want to remove it to save edicts
//	if (!self->targetname && !(self->spawnflags & LIGHT_FLARE) || deathmatch->value )
	if (!self->targetname && !(self->spawnflags & LIGHT_FLARE))
	{	
		self->remove (self);
		return;
	}

	model = "models/global/e_flare2.sp2";

	self->userHook = (lightHook_t*)gstate->X_Malloc(sizeof(lightHook_t),MEM_TAG_HOOK);
//	self->userHook = new lightHook_t;
	hook = (lightHook_t *) self->userHook;

	hook->style = 0;

	// SCG[11/24/99]: Save game stuff
	self->save = light_hook_save;
	self->load = light_hook_load;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (!stricmp (self->epair [i].key, "delay"))
			self->delay = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "sound"))
			hook->ambient_sound = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "targetname"))
			self->targetname = self->epair [i].value;
		else if (!stricmp (self->epair [i].key, "style"))
			hook->style = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "lightstyle"))
		{
			style_defined = TRUE;
			hook->stylestr = self->epair [i].value;
		}
		else if (!stricmp (self->epair [i].key, "model"))
			model = self->epair [i].value;
		else if (!stricmp(self->epair[i].key, "scale"))  // 1.20 dsn
		{
			float fScale = atof (self->epair[i].value);
			self->s.render_scale.Set( fScale, fScale, fScale );
        }


		i++;
	}
		
	if( self->s.render_scale.x == 0 ) self->s.render_scale.x = 1;
	if( self->s.render_scale.y == 0 ) self->s.render_scale.y = 1;
	if( self->s.render_scale.z == 0 ) self->s.render_scale.z = 1;

	if (style_defined)
		gstate->ConfigString (CS_LIGHTS + hook->style, hook->stylestr);

	if (hook->ambient_sound)
		gstate->SoundIndex (hook->ambient_sound);

	if (hook->style >= 32)
	{
		self->use = light_use;
		if (self->spawnflags & START_OFF)
			gstate->ConfigString (CS_LIGHTS + hook->style, "a");
		else
			gstate->ConfigString (CS_LIGHTS + hook->style, "m");
	}

	if (self->spawnflags & LIGHT_FLARE)
	{
		self->solid = SOLID_NOT;
		self->movetype = MOVETYPE_NONE;
		self->s.modelindex = gstate->ModelIndex (model);
		self->s.renderfx = SPR_ALPHACHANNEL | RF_TRANSLUCENT;

		self->s.mins.Zero();
		self->s.maxs.Zero();
		
		// don't send it to the client if the light starts off
		if (self->spawnflags & START_OFF)
			self->svflags |= SVF_NOCLIENT;

		gstate->LinkEntity (self);

	}
	else
	{
		self->solid = SOLID_NOT;
		self->movetype = MOVETYPE_NONE;
		self->s.renderfx = RF_NODRAW;

		self->s.mins.Zero();
		self->s.maxs.Zero();
		
		// don't send it to the client if the light starts off
		if (self->spawnflags & START_OFF)
			self->svflags |= SVF_NOCLIENT;
		gstate->LinkEntity (self);
	}
}	

////////////////////////////////////
// light_spot
////////////////////////////////////

void	light_spot (userEntity_t *self)
{
	int				i;
	lightHook_t		*hook;

	self->userHook = gstate->X_Malloc(sizeof(lightHook_t),MEM_TAG_HOOK); 
	hook = (lightHook_t *) self->userHook;

	hook->style = 0;

	// SCG[11/24/99]: Save game stuff
	self->save = light_hook_save;
	self->load = light_hook_load;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (stricmp (self->epair [i].key, "delay") == 0)
			self->delay = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "sound") == 0)
			hook->ambient_sound = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "targetname") == 0)
			self->targetname = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "flag") == 0)
		{
			if (stricmp (self->epair [i].value, "NO_SOUND") == 0)
				self->spawnflags = (int)self->spawnflags | NO_SOUND;
		}
		else if (!stricmp (self->epair [i].key, "style"))
			hook->style = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "lightstyle"))
		{
			hook->stylestr = self->epair [i].value;
			gstate->LightStyle (hook->style, hook->stylestr);
		}

		i++;
	}

	if (hook->ambient_sound)
		gstate->SoundIndex (hook->ambient_sound);

	self->use = light_use;
}

////////////////////////////////////
// light_strobe_think
////////////////////////////////////
/*
void	light_strobe_think (userEntity_t *self)
{
	lightHook_t		*hook = (lightHook_t *) self->userHook;

	if (hook->state == FALSE)
	{
		//FIXME: change LightStyle
		hook->state = TRUE;
	}
	else
	{
		//FIXME: change LightStyle
		hook->state = FALSE;
	}

	if (hook->ambient_sound)
		gstate->sound (self, CHAN_AUTO, hook->ambient_sound, 255, ATTN_NORM);

	self->nextthink = gstate->time + self->delay;
	self->think = light_strobe_think;
}
*/
////////////////////////////////////
// light_strobe
////////////////////////////////////

void	light_strobe (userEntity_t *self)
{
	int				i;
	lightHook_t		*hook;

	self->userHook = gstate->X_Malloc(sizeof(lightHook_t), MEM_TAG_HOOK); 
	hook = (lightHook_t *) self->userHook;

	hook->style = 0;

	// SCG[11/24/99]: Save game stuff
	self->save = light_hook_save;
	self->load = light_hook_load;

	i = 0;
	while (self->epair [i].key != NULL)
	{
		if (stricmp (self->epair [i].key, "delay") == 0)
			self->delay = atof (self->epair [i].value);
		else if (stricmp (self->epair [i].key, "sound") == 0)
			hook->ambient_sound = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "targetname") == 0)
			self->targetname = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "flag") == 0)
		{
			if (stricmp (self->epair [i].value, "NO_SOUND") == 0)
				self->spawnflags = (int)self->spawnflags | NO_SOUND;
		}
		else if (!stricmp (self->epair [i].key, "style"))
			hook->style = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "lightstyle"))
		{
			hook->stylestr = self->epair [i].value;
			gstate->LightStyle (hook->style, hook->stylestr);
		}

		i++;
	}

	if (hook->ambient_sound)
		gstate->SoundIndex (hook->ambient_sound);
	
	self->use = light_use;
}

////////////////////////////////////
// light_walltorch
////////////////////////////////////

void	light_walltorch (userEntity_t *self)
{
//	int				i;
//	lightHook_t		*hook;
	float			fade = 1.0;

	gstate->RemoveEntity( self );
//	return;

/*
	self->userHook = gstate->X_Malloc(sizeof(lightHook_t), MEM_TAG_HOOK); 
	hook = (lightHook_t *) self->userHook;

	// SCG[11/24/99]: Save game stuff
	self->save = light_hook_save;
	self->load = light_hook_load;

	i = 0;
	while (self->epair [i].key != NULL)
	{
//		gstate->Con_Printf ("key = %s, value = %s\n", self->epair [i].key, self->epair [i].value);
		
		if (strcmp (self->epair [i].key, "ambient_sound") == 0)
			hook->ambient_sound = self->epair [i].value;
		else if (stricmp (self->epair [i].key, "targetname") == 0)
			self->targetname = self->epair [i].value;
		else if (strcmp (self->epair [i].key, "flag") == 0)
		{
			if (strcmp (self->epair [i].value, "NO_SOUND") == 0)
				self->spawnflags = (int)self->spawnflags | NO_SOUND;
		}
		else if (!stricmp (self->epair [i].key, "style"))
			hook->style = atof (self->epair [i].value);
		else if (!stricmp (self->epair [i].key, "lightstyle"))
		{
			hook->stylestr = self->epair [i].value;
			gstate->LightStyle (hook->style, hook->stylestr);
		}
		else if (!stricmp (self->epair [i].key, "fade"))
			fade = atof (self->epair [i].value);

		i++;
	}

	gstate->ModelIndex ("models/e3/d3_torch.dkm");
	gstate->SetModel (self, "models/e3/d3_torch.dkm");

	if (!((int)self->spawnflags & NO_SOUND))
	{
		if (hook->ambient_sound)
		{
			// FIXME: add ambientsound to 
			gstate->SoundIndex (hook->ambient_sound);
			gstate->AmbientSound (self->s.origin, hook->ambient_sound, 255, ATTN_STATIC, fade);
		}
		else
		{
			gstate->SoundIndex ("ambience/fire1.wav");
			gstate->AmbientSound (self->s.origin, "ambience/fire1.wav", 255, ATTN_STATIC, fade);
		}
	}

	// FIXME: add makestatic to 
	//	gstate->makestatic (self);
	self->use = light_use;
*/
};

///////////////////////////////////////////////////////////////////////////////
// light_flame_burn
//
///////////////////////////////////////////////////////////////////////////////

void light_flame_burn(userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other->takedamage = DAMAGE_YES)
	{
		if (other->flags & FL_CLIENT)
			client_deathtype = LAVA;
		com->Damage (other, self, self, other->s.origin, zero_vector, 2.0, DAMAGE_NONE | DAMAGE_NO_BLOOD);
	}
}

void light_flame_die( userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point )
{
	gstate->RemoveEntity( self->owner );
}

///////////////////////////////////////////////////////////////////////////////
// light_setup
//
///////////////////////////////////////////////////////////////////////////////
void light_flame_setup(userEntity_t *self, CVector& origin, char *model)
{
	userEntity_t	*light;
	lightHook_t		*hook;
	float			fade = 1.0;

	self->userHook = gstate->X_Malloc(sizeof(lightHook_t), MEM_TAG_HOOK); 
	hook = (lightHook_t *) self->userHook;

	for( int i = 0; self->epair [i].key != NULL; i++ )
	{
		if( !stricmp( self->epair [i].key, "scale" ) )
		{
			sscanf( self->epair[i].value, "%f %f %f", 
				&self->s.render_scale.x, 
				&self->s.render_scale.y, 
				&self->s.render_scale.z );
		}
		else if( !stricmp( self->epair [i].key, "sound" ) )
		{
			hook->ambient_sound = self->epair[i].value;
		}
		else if (!stricmp (self->epair [i].key, "fade"))
			fade = atof (self->epair [i].value);
	}

	if (hook->ambient_sound)
	{
		// FIXME: add ambientsound to 
		gstate->SoundIndex (hook->ambient_sound);
		gstate->AmbientSound (self->s.origin, hook->ambient_sound, 255, ATTN_STATIC, fade);
	}

	// SCG[11/24/99]: Save game stuff
	self->save = light_hook_save;
	self->load = light_hook_load;

	if( self->s.render_scale.x == 0 ) self->s.render_scale.x = 1;
	if( self->s.render_scale.y == 0 ) self->s.render_scale.y = 1;
	if( self->s.render_scale.z == 0 ) self->s.render_scale.z = 1;

	self->solid = SOLID_TRIGGER;
	self->movetype = MOVETYPE_NONE;
	self->s.renderfx = SPR_ALPHACHANNEL | SPR_ORIENTED | RF_TRANSLUCENT;
	self->s.effects = EF_ANIM_ALLFAST;
//	self->delay = self->s.render_scale[0];
	self->s.modelindex = gstate->ModelIndex (model);
	self->owner = gstate->SpawnEntity ();

	//	spawn the other flame facing 90 degrees
	light = self->owner;
	light->solid = SOLID_NOT;
	light->movetype = MOVETYPE_NONE;
	light->s.renderfx = SPR_ALPHACHANNEL | SPR_ORIENTED | RF_TRANSLUCENT;
	light->s.effects = EF_ANIM_ALLFAST;
	light->s.angles.x = self->s.angles.x;
	light->s.angles.y = self->s.angles.y + 90;
	light->s.angles.z = self->s.angles.z;
	light->s.origin = origin;
	light->s.render_scale = self->s.render_scale;
	light->s.mins.Set( -8, -8, -8 );
	light->s.maxs.Set( 8, 8, 8 );
	light->s.modelindex = self->s.modelindex;

	gstate->LinkEntity (self);
	gstate->LinkEntity (light);

	self->die = light_flame_die;
	self->touch = light_flame_burn;
}

#define LIGHT_TYPE_1	0x00000001
#define LIGHT_TYPE_2	0x00000002
#define LIGHT_TYPE_3	0x00000004
#define LIGHT_TYPE_4	0x00000008

void light_e1(userEntity_t *self)
{
	light_flame_setup(self, self->s.origin, "models/global/e2_firea.sp2");
}

void light_e2(userEntity_t *self)
{
	light_flame_setup(self, self->s.origin, "models/global/e2_firea.sp2");

	if (self->spawnflags & LIGHT_TYPE_1)
	{
	}
	else if (self->spawnflags & LIGHT_TYPE_2)
	{
	}
	else if (self->spawnflags & LIGHT_TYPE_3)
	{
	}
	else if (self->spawnflags & LIGHT_TYPE_4)
	{
	}
}

void light_e3(userEntity_t *self)
{
	light_flame_setup(self, self->s.origin, "models/global/e3_firea.sp2");
	if (self->spawnflags & LIGHT_TYPE_1)
	{
	}
	else if (self->spawnflags & LIGHT_TYPE_2)
	{
	}
	else if (self->spawnflags & LIGHT_TYPE_3)
	{
	}
}

void light_e4(userEntity_t *self)
{
	light_flame_setup(self, self->s.origin, "models/global/e4_firea.sp2");
	if (self->spawnflags & LIGHT_TYPE_1)
	{
	}
}

///////////////////////////////////////////////////////////////////////////////
//	light_setstyles
//
///////////////////////////////////////////////////////////////////////////////

void	light_setstyles (void)
{
	// 0 normal
	gstate->ConfigString (CS_LIGHTS + 0, "m");
	
	// 1 FLICKER (first variety)
	gstate->ConfigString (CS_LIGHTS + 1, "mmnmmommommnonmmonqnmmo");
	
	// 2 SLOW STRONG PULSE
	gstate->ConfigString (CS_LIGHTS + 2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");
	
	// 3 CANDLE (first variety)
	gstate->ConfigString (CS_LIGHTS + 3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");
	
	// 4 FAST STROBE
	gstate->ConfigString (CS_LIGHTS + 4, "mamamamamama");
	
	// 5 GENTLE PULSE 1
	gstate->ConfigString (CS_LIGHTS + 5,"jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	
	// 6 FLICKER (second variety)
	gstate->ConfigString (CS_LIGHTS + 6, "nmonqnmomnmomomno");
	
	// 7 CANDLE (second variety)
	gstate->ConfigString (CS_LIGHTS + 7, "mmmaaaabcdefgmmmmaaaammmaamm");
	
	// 8 CANDLE (third variety)
	gstate->ConfigString (CS_LIGHTS + 8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");
	
	// 9 SLOW STROBE (fourth variety)
	gstate->ConfigString (CS_LIGHTS + 9, "aaaaaaaazzzzzzzz");
	
	// 10 FLUORESCENT FLICKER
	gstate->ConfigString (CS_LIGHTS + 10, "mmamammmmammamamaaamammma");

	// 11 SLOW PULSE NOT FADE TO BLACK
	gstate->ConfigString (CS_LIGHTS + 11, "abcdefghijklmnopqrrqponmlkjihgfedcba");

	//	styles 12 - 16 are definable by individual lights

	// 63 testing
	gstate->ConfigString (CS_LIGHTS + 63, "a");
}


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_light_register_func()
{
	gstate->RegisterFunc("light_flame_burn",light_flame_burn);
	gstate->RegisterFunc("light_use",light_use);
	gstate->RegisterFunc("light_flare_use",light_flare_use);
	gstate->RegisterFunc("light_flame_die",light_flame_die);

	gstate->RegisterFunc("light_hook_save", light_hook_save );
	gstate->RegisterFunc("light_hook_load", light_hook_load );
}
