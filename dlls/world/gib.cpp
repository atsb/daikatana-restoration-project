// ==========================================================================
//
//  File:
//  Contents:
//	routines for extracting and animating frames in actor entities
//  Author:
//
// ==========================================================================

#include "world.h"
//#include "FadeEnt.h"
#include "p_user.h"

/* ***************************** define types ****************************** */

#define	GIB_FOLLOW_SPEED	40.0
#define	GIB_MAX_SPIN		180.0
#define	GIB_GOAL_DIST		32.0
#define GIB_MAX_GIB			100
#define	MAX_GIBSSP  16       //  maximum gib pieces in single player mode
#define	MAX_GIBSMP   6       //  maximum gib pieces in multiplayer mode


#define GIB_DAMAGE_MULTIPLIER  25.0  // adjust this to adjust velocity/craziness of gibs!

typedef	struct	gibHook_s
{
	float		target_z;		//	what height the gib should float up to

	float		expire_time;	//	how long the gib should wait before shrinking away

	weapon_t	*weapon;		//	pointer to the nightmare weapon when user first runs over a gib
} gibHook_t;

#define	GIBHOOK(x) (int)&(((gibHook_t *)0)->x)
field_t gib_hook_fields[] = 
{
	{"target_z",	GIBHOOK(target_z),		F_FLOAT},
	{"expire_time",	GIBHOOK(expire_time),	F_FLOAT},
	{NULL, 0, F_INT}
};
void AI_Dprintf( char *fmt, ... );
///////////////////This is only for the GIB Generator not for gibs themself/////////////////////////
#define fGIB_ROBOTIC	0x0001
#define fGIB_BONE		0x0002
#define fGIB_NOBLOOD	0x0004
#define fGIB_STARTON	0x0008
#define fGIB_NOTOGGLE	0x0010

typedef struct 
{
    int onoff;									// Simple on/off
	int count;									// # of gib pieces
	int velocity;								// velocity for the gibs
	int spread;									// spread for the gibs
	int InternalFlags;							// Flags from epairs.
	float stoptime;								// Time for the entire SFX particle to generate what it is gonna do.. then quit until triggered again.
	float lifetime;								// time to live
	float scale;
	float smins;								//mins sound area
	float smaxs;								//maxs sound area
	float volume;								//volume
	CVector Target;								// Target Direction
} func_gib_hook_t;

#define	FUNC_GIB_HOOK(x) (int)&(((func_gib_hook_t *)0)->x)
field_t func_gib_hook_fields[] = 
{
	{"onoff",			FUNC_GIB_HOOK(onoff),			F_INT},
	{"count",			FUNC_GIB_HOOK(count),			F_INT},
	{"velocity",		FUNC_GIB_HOOK(velocity),		F_INT},
	{"spread",			FUNC_GIB_HOOK(spread),			F_INT},
	{"InternalFlags",	FUNC_GIB_HOOK(InternalFlags),	F_INT},
	{"stoptime",		FUNC_GIB_HOOK(stoptime),		F_FLOAT},
	{"lifetime",		FUNC_GIB_HOOK(lifetime),		F_FLOAT},
	{"scale",			FUNC_GIB_HOOK(scale),			F_FLOAT},
	{"Target",			FUNC_GIB_HOOK(Target),			F_VECTOR},
	{NULL, 0, F_INT}
};


int GIB_COUNT = 0;
/* ***************************** Local Variables *************************** */
/* ***************************** Local Functions *************************** */

void gib_set_bboxes(CVector * mins, CVector * maxs, byte gib_type);

/* **************************** Global Variables *************************** */
/* **************************** Global Functions *************************** */

/* ******************************* exports ********************************* */

//Gib Generator
DllExport void func_gib(userEntity_t *self);


// ----------------------------------------------------------------------------
//
// Name:		gib_remove
// Description:
// Input:
// Output:
// Note:
//				cleanup function for gib removal
//
// ----------------------------------------------------------------------------
void gib_remove(userEntity_t *self)
{
	gstate->RemoveEntity (self);
	if(GIB_COUNT)
		GIB_COUNT --;
}

// ----------------------------------------------------------------------------
//
// Name:		gib_goal_dir
// Description:
// Input:
// Output:
// Note:
//	returns a vector pointing towards goal which has a zero z 
//	component (direction in x/y plane only).
//
// ----------------------------------------------------------------------------
void gib_goal_dir( userEntity_t *self, userEntity_t *goal, CVector &dir )
{
	CVector	s_org, g_org;

	s_org = self->s.origin;
	s_org.z = 0;

	g_org = goal->s.origin;
	g_org.z = 0;

	dir = g_org - s_org;
	dir.Normalize();
}

     
// ----------------------------------------------------------------------------
//
// Name:		gib_follow
// Description:
// Input:
// Output:
// Note:
//	moves towards goal in the x/y plane, doesnt affect z velocity
//	returns distance to goal in x/y plane
//
// ----------------------------------------------------------------------------
float gib_follow( userEntity_t *self, userEntity_t *goal, float &speed )
{
	CVector	dir;
	gib_goal_dir (self, goal, dir);

	dir = dir * speed;
	
	self->velocity.x = dir.x;
	self->velocity.y = dir.y;

	speed = speed * self->button0;	// increase speed

	return	VectorXYDistance(self->s.origin, goal->s.origin);
}

// ----------------------------------------------------------------------------
//
// Name:		gib_shrink
// Description:
// Input:
// Output:
// Note:
//	gib has reached desired height and is spinning and shrinking into nothing
//	spinning grows faster and faster as gib shrinks
//
// ----------------------------------------------------------------------------
void gib_shrink( userEntity_t *self )
{
	gibHook_t	*gibHook = (gibHook_t *) self->userHook;

	gib_follow (self, self->goalentity, self->button1);

	//	gib has reached desired height and is spinning and shrinking into nothing
	self->s.render_scale = self->s.render_scale * self->delay;

	if ( self->s.render_scale.x < 0.1f )
	{
/*
		//	give 1 gib ammo to player for picking up the gore
		if (self->goalentity->inventory)
		{
			ammo = (ammo_t *) gstate->InventoryFindItem (self->goalentity->inventory, "ammo_gibs");
			if (ammo)
			{
				ammo_count = 3 * frand() + 1;
				ammo->command ((userInventory_t *)ammo, "give_ammo", &ammo_count);
			}
		}
*/
		self->remove (self);
	}
	else
	{
		self->think = gib_shrink;
		self->nextthink = gstate->time + 0.05f;//gstate->frametime;
	}
}
// ----------------------------------------------------------------------------
//
// Name:		ai_gib_sound
// Description: plays a sound for gib... it used to play a sound per gib.
// Input:
// Output:
// Note:
// creator: <nss>
//
// ----------------------------------------------------------------------------
void ai_gib_sound(userEntity_t *self)
{
	char			gib_sound[128];
	char			letter;

	if ((self->fragtype & FRAGTYPE_ROBOTIC)) 
	{
		letter = 97+(int)(rnd()*2.0);
//		sprintf(gib_sound,"global/m_gibsurf%c.wav",letter);
		Com_sprintf(gib_sound,sizeof(gib_sound),"global/m_gibsurf%c.wav",letter);
	}   
	else if (self->fragtype & FRAGTYPE_BONE)
	{
		letter = 97+(int)(rnd()*1.0);
//		sprintf(gib_sound,"global/m_gibbone%c.wav",letter);
		Com_sprintf(gib_sound,sizeof(gib_sound),"global/m_gibbone%c.wav",letter);

	}
	else
	{
		letter = 97+(int)(rnd()*4.0);
//		sprintf(gib_sound,"global/m_gibslop%c.wav",letter);
		Com_sprintf(gib_sound,sizeof(gib_sound),"global/m_gibslop%c.wav",letter);
	}
	gstate->StartEntitySound(self,CHAN_AUTO, gstate->SoundIndex(gib_sound), 0.75f, 300.0f,800.0f);
}


// ----------------------------------------------------------------------------
// NSS[12/6/99]:
// Name:		ai_smoked_sound
// Description: Plays a meaty fleshy nasty sound... This only happens when someone turns into meat soup...
// ...uh... ok so there are always gonna be gibs.  :)
// Input:
// Output:
// Note:
// ----------------------------------------------------------------------------
void ai_smoked_sound(userEntity_t *self)
{
	char			gib_sound[128];
	char			letter;
	memset( gib_sound, 0, 128 );
	if (!(self->fragtype & FRAGTYPE_ROBOTIC)&&!(self->fragtype & FRAGTYPE_BONE)) 
	{
		letter = 97+(int)(rnd()*5.0);
//		sprintf(gib_sound,"global/m_gibmeat%c.wav",letter);
		Com_sprintf(gib_sound,sizeof(gib_sound),"global/m_gibmeat%c.wav",letter);
	}   
	gstate->StartEntitySound(self,CHAN_AUTO, gstate->SoundIndex(gib_sound), 0.75f, 300.0f,800.0f);	
}



// ----------------------------------------------------------------------------
//
// Name:		gib_scale_spin
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
/*
void gib_scale_spin(userEntity_t *self)
{
	//	spin faster
	self->avelocity = self->avelocity * self->team;

	if (self->avelocity.x > GIB_MAX_SPIN)
	{
		self->avelocity.x = GIB_MAX_SPIN;
	}
	else 
	if (self->avelocity.x < - GIB_MAX_SPIN)
	{
		self->avelocity.x = - GIB_MAX_SPIN;
	}

	if (self->avelocity.y > GIB_MAX_SPIN)
	{
		self->avelocity.y = GIB_MAX_SPIN;
	}
	else 
	if (self->avelocity.y < - GIB_MAX_SPIN)
	{
		self->avelocity.y = - GIB_MAX_SPIN;
	}

	if (self->avelocity.z > GIB_MAX_SPIN)
	{
		self->avelocity.z = GIB_MAX_SPIN;
	}
	else 
	if (self->avelocity.z < - GIB_MAX_SPIN)
	{
		self->avelocity.z = - GIB_MAX_SPIN;
	}
}
*/
// ----------------------------------------------------------------------------
//
// Name:		gib_spin
// Description:
// Input:
// Output:
// Note:
//	gib has reached desired height and is spinning until expire_time is reached
//
// ----------------------------------------------------------------------------
/*
void gib_spin( userEntity_t *self )
{
	gibHook_t	*gibHook = (gibHook_t *) self->userHook;

	float dist = gib_follow (self, self->goalentity, self->button1);

	//	gib has reached desired height and is spinning 
	gib_scale_spin (self);

	if (gibHook->expire_time > gstate->time && dist > GIB_GOAL_DIST)
	{
		self->nextthink = gstate->time + THINK_TIME;
	}
	else
	{
		self->think = gib_shrink;
		self->nextthink = gstate->time + THINK_TIME;
	}
}
*/
// ----------------------------------------------------------------------------
//
// Name:		gib_rise
// Description:
// Input:
// Output:
// Note:
//	gib is rising to target_z height
//
// ----------------------------------------------------------------------------
/*
void gib_rise(userEntity_t *self)
{
	gibHook_t	*gibHook = (gibHook_t *) self->userHook;

//	gstate->Con_Printf ("gib_rise\n");
	
	gib_follow (self, self->goalentity, self->button1);

	gib_scale_spin (self);

	//	gib is rising up
	if (self->s.origin.z < gibHook->target_z && gstate->time < gibHook->expire_time)
	{
		self->nextthink = gstate->time + THINK_TIME;
	}
	else
	{
		gibHook->expire_time = gstate->time + 3.5;	//	spin time before shrinking away

		self->think = gib_spin;
		self->nextthink = gstate->time + THINK_TIME;
	}
}
*/
// ----------------------------------------------------------------------------
//
// Name:		gib_percent_above_below
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
// SCG[2/28/00]: not used
/*
void gib_percent_above_below (userEntity_t *self, int *contents, float *percent_submerged)
{
	CVector		top, bottom;
	float		bbox_height, water_height;				
	gibHook_t	*gibHook = (gibHook_t *) self->userHook;

	top = self->s.origin;
	top.z += self->s.maxs.z;

	bottom = self->s.origin;
	bottom.z += self->s.mins.z;

	bbox_height =(self->s.maxs.z - self->s.mins.z);

	water_height = gibHook->target_z - bottom.z;

	if (water_height <= 0)
	{
		*(float *) percent_submerged = 0.0;
	}
	else if (water_height >= bbox_height)
	{
		*(float *) percent_submerged = 1.0;	 
		// amw: set this here so it will float
		self->movetype = MOVETYPE_FLOAT;
	}
	else
	{
		*(float *) percent_submerged = 1 / (bbox_height / water_height);
		// amw: set this here so it will float
		self->movetype = MOVETYPE_FLOAT;
	}

	*(int *) contents = CONTENTS_EMPTY;
}
*/

// ----------------------------------------------------------------------------
//
// Name:		gib_touch_stopped
// Description:
// Input:
// Output:
// Note:
//	determines if the toucher can pick the gib up
//
// ----------------------------------------------------------------------------
void gib_touch_stopped( userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf )
{
	CVector		avel;
//	gibHook_t	*gibHook;
	weapon_t	*weapon;

	if (other->flags & (FL_CLIENT))// + FL_MONSTER))
	{
		weapon = (weapon_t *) other->curWeapon;

		if (weapon && weapon->name && !stricmp (weapon->name, "weapon_nightmare"))
		{
			//	FIXME: FL_MONSTER only picks up gibs if of TYPE_NHARRE
			//	FIXME: FL_CLIENT only picks up gibs if holding Nharre's Nightmare
			//	FIXME: FL_BOT only picks up gibs if Nharre's Nightmare is in inventory

			// cek: all this is cool but it just doesn't work right.  Just give em the gibs
			ammo_t *ammo = (ammo_t *) gstate->InventoryFindItem (other->inventory, "ammo_gibs");
			if (ammo)
			{
				ammoGiveTake_t take;
				take.count = 1;
				take.owner = other;
				ammo->command ((userInventory_t *)ammo, "give_ammo", &take);
			}
			self->touch = NULL;
/*
			self->userHook = gstate->X_Malloc(sizeof(gibHook_t),MEM_TAG_HOOK);
			gibHook = (gibHook_t *) self->userHook;

			//	float up to player's view_ofs height
			//	FIXME: make sure height is always the same above floor when Q2 bounding boxes are implemented
			gibHook->target_z = self->s.origin[2] + 32 + rnd () * 24 - 16;
			gibHook->expire_time = gstate->time + 2.0;

			self->delay = 0.5;			//	shrink rate
			self->team	= 1.09;			//	rate of spin increase
			self->button0 = 1.05;		//	forward velocity scale
			self->button1 = GIB_FOLLOW_SPEED;	//	speed
			self->s.effects = 0;	//	turn off blood trailing

			self->movetype = MOVETYPE_TOSS;	 // amw: the submerged function will take care of setting it to MOVETYPE_FLOAT
			//self->groundEntity = NULL;

			self->goalentity = other;

			avel.x = rnd () * 2.0 - 1.0;
			avel.y = rnd () * 2.0 + 2.0;
			if (rnd () < 0.5)
			{
				avel.y = -avel.y;
			}
			avel.z = rnd () * 2.0 - 1.0;
			avel.Normalize ();

			avel = avel * 30.0;
			self->avelocity = avel;

			self->touch = NULL;

			self->think = gib_rise;
			self->nextthink = gstate->time + 0.1;

			//float_init (self, 1.0, 2.0, 1.0, 0.9, 24.0, gib_percent_above_below);
			self->mass = 5.0;
			self->volume = 1.25;
			self->dissipate = 0.9;
			//self->velocity_cap = 24.0;  // old
            self->velocity_cap = 18.0;  // 3.8.99  
			self->submerged_func = gib_percent_above_below;
*/
		}
	}
}

// ----------------------------------------------------------------------------
//
// Name:		gib_BloodSplat
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------
void gib_BloodSplat (userEntity_t *self)
{
	CVector	end = self->s.origin + self->velocity;

	tr = gstate->TraceLine_q2(self->s.origin, end, self, MASK_MONSTERSOLID);
	if (tr.fraction < 1.0 && tr.ent && tr.ent->solid == SOLID_BSP)
	{
		com->BloodSplat (tr.ent, tr.endpos, &tr.plane);
	}
}

// ----------------------------------------------------------------------------
//
// Name:		gib_touch_moving
// Description:
// Input:
// Output:
// Note:
//	changes gib angular velocity each time it hits
//	put a blood splat on the wall, too.
//
// ----------------------------------------------------------------------------
void gib_touch_moving (userEntity_t *self, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	float speed = self->velocity.Length();

    //DKLOG_Write( LOGTYPE_MISC, 0.0f, "speed = %f", (float)speed );
   
	self->avelocity.x = (rnd() * speed * 2.0) - speed;
	self->avelocity.y = (rnd() * speed * 2.0) - speed;
	self->avelocity.z = (rnd() * speed * 2.0) - speed;
	
	if((rnd() < 0.025f) && (_stricmp(other->className,"worldspawn")==0))
	{
		ai_gib_sound(self);
	}

	if( other->solid == SOLID_BSP )
    {
		if( speed > 125.0f ) // make sure the gib is hauling ass before leaving blood
		{
			if( !( self->fragtype & FRAGTYPE_NOBLOOD ) ) // only if no-blood flag isn't set
			{
				if(rnd() < 0.15)
					gib_BloodSplat( self ); // splat the solid surface
			}                            
		}
    } 
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		gib_fadeout
// Description:
// Input:
// Output:
// Note:
//	fade out the gib before it is removed
//
// ----------------------------------------------------------------------------
#define GIB_FADE_STEP		0.15

void gib_fadeout( userEntity_t *self )
{
	// has the gib faded completely out?
	if (self->s.alpha <= 0.10 )
	{
		self->think = self->remove;
		//gstate->RemoveEntity(self);
	}
	else
	{
		// fade out the gib
		float Fader = 1.0f;
		if(GIB_COUNT > GIB_MAX_GIB*0.70f)//As we approach our max gib count let's accelerate the fade out.
			Fader = 3.0f * (float)((float)GIB_COUNT/(float)GIB_MAX_GIB);
		if (self->s.alpha > GIB_FADE_STEP)
		{
			self->s.alpha -= GIB_FADE_STEP*Fader;
		}
		else
		{
			self->s.alpha = 0.09;
		}
	}
	self->nextthink = gstate->time + 0.01;

}


// ----------------------------------------------------------------------------
// NSS[1/4/00]:
// Name:		gib_think
// Description:
// Input:
// Output:
// Note:
// routine for trailing blood (see also CL_DiminishingTrail() )
//
// ----------------------------------------------------------------------------
void gib_think( userEntity_t *self )
{
	float PercS = 0.0f;

	if(self->fragtype & FRAGTYPE_BONE)
		PercS = 3.0f;
	
	
	if(self->delay < gstate->time)
	{
		
		if (self->groundEntity)
		{
			if(!stricmp("wordlspawn",self->groundEntity->className))
			{
				self->nextthink = gstate->time + THINK_TIME;
				return;
			}

			self->solid = SOLID_TRIGGER;

			// this insures that doors won't collide with gibs
			//self->movetype = MOVETYPE_BOUNCE;	 // amw: the submerged function will take care of setting it to MOVETYPE_FLOAT
			//self->movetype = MOVETYPE_TOSS;
			//self->avelocity.Zero();
			//self->gravity = 2.0f;
			
			self->touch = gib_touch_stopped;
			self->think = gib_fadeout;

			// set up the fade out for the gibs
			self->s.renderfx |= RF_TRANSLUCENT;
			self->s.alpha = 1.0;

			// start fade out more quickly  3.4 dsn
			self->nextthink = gstate->time + 5.0 + rnd () * 10.0; 
			//gstate->LinkEntity (self);
			return;
		}
	}

	self->nextthink = gstate->time + THINK_TIME;
}



// ----------------------------------------------------------------------------
// NSS[1/4/00]:
// Name:		GibLimitDirection
// Description: Limits the Directional Path of the Gibs based on the direction of the attacker based off of ourself
// Input:CVector Dir
// Output:CVector NewDir
// Note:
// ----------------------------------------------------------------------------
void GibLimitDirection(CVector Dir, CVector *GibDir)
{
	
	CVector Angles;

	CVector NewDir;
	Dir.Negate();
	VectorToAngles(Dir,Angles);

	Angles.yaw		+= crand() * 45.0f;
	Angles.pitch	+= crand() * 45.0f;
	
	if(Angles.yaw > 360)
		Angles.yaw -= 360.0f;

	Angles.AngleToForwardVector(NewDir);
	GibDir->x = NewDir.x;
	GibDir->y = NewDir.y;
	GibDir->z = NewDir.z;
}



// ----------------------------------------------------------------------------
// NSS[1/4/00]:
// Name:		ai_throw_gib
// Description:
// Input:
// Output:
// Note:
//	throws a gib more or less in the direction of damage_vector
// ----------------------------------------------------------------------------
userEntity_t *ai_throw_gib( userEntity_t *self, char *model, CVector &offset,CVector Dir, byte gib_type)
{
	userEntity_t	*gib;
	CVector			dir;
	float			damage_mult,scale_mod[3];
	CVector			mins, maxs;
	char			model_type[128];
	CVector			GibDir;


	// NSS[1/4/00]:Setup the Traveling Direction
	GibLimitDirection(Dir,&GibDir);

	if(GIB_COUNT > GIB_MAX_GIB)
	{
		return NULL;
	}
	else
	{
		GIB_COUNT++;
	}
	if (!gib_enable->value ||  gstate->GetCvar("sv_violence"))										// Check to see if the monster is gib'able.
		return NULL;

	Com_sprintf(model_type,sizeof(model_type),"%s",model);
	if ((self->fragtype & FRAGTYPE_ROBOTIC))														// only if no-blood flag isn't set
	{
	    gib_set_bboxes(&mins, &maxs, gib_type);														// get bounding box size of gib, dependent on type
	}   
	else if (self->fragtype & FRAGTYPE_BONE)
	{
		Com_sprintf(model_type,sizeof(model_type),"models/global/g_bone.dkm");
		mins.Set(-2, -2, -1);  
		maxs.Set(2, 2, 1);
	    gib_set_bboxes(&mins, &maxs, gib_type);														// get bounding box size of gib, dependent on type
	}
	else
	{
		if(rnd() < 0.25)																			// NSS[12/6/99]:Sloppy seconds?
			gib_BloodSplat (self);																	// splat the solid surface
	    gib_set_bboxes(&mins, &maxs, gib_type);														// get bounding box size of gib, dependent on type
	}
	
	
	gib					= gstate->SpawnEntity ();
	gib->movetype		= MOVETYPE_BOUNCE;
	gib->fragtype		= self->fragtype;
	gib->s.iflags		|= IF_SV_SENDBBOX;																// NSS[1/4/00]:Prevent the bouncy bouncy thang from happening
	gib->svflags		|= SVF_DEADMONSTER;
	gib->s.modelindex	= gstate->ModelIndex (model_type);
	gib->solid			= SOLID_BBOX;
    gib->clipmask		= MASK_DEADSOLID;																// 1990309 JAR - don't let GIBS block player movement
	gib->owner			= self;
	gib->className		= "gib";
    gib->takedamage		= 0;
	gib->mass			= 2.0;
	gib->volume			= 1.0;
    gib->fragtype		= self->fragtype;
	gib->gravity		= 1.0f;
	gib->svflags		|= SVF_NOPUSHCOLLIDE;
	gib->s.effects2		= EF2_BLOODY_GIB;
	gib->flags			|= FL_NOSAVE;
	
	gstate->SetSize(gib, mins.x, mins.y, mins.z, maxs.x, maxs.y, maxs.z);

////////////////////////////////German Regulated Human Gibs Starts Here///////////////////////////////////

// SCG[6/5/00]: #ifdef TONGUE_GERMAN																					//Don't spill a damn drop of blood
#ifdef NO_VIOLENCE
	//gib->s.skinnum = gstate->GetModelSkinIndex (self->s.modelindex);
	gib->s.skinnum = gstate->ImageIndex(gstate->GetModelSkinName(self->s.modelindex));
	gib->s.effects2	|= EF2_IMAGESKIN;
#else
    if ((self->fragtype & FRAGTYPE_ROBOTIC))															// use the skin from the entity we blew up, if the monster is non-fleshy 
	{
		// NSS[7/12/00]:Oh My Fucking God!  What a cluster fucko'
		gib->s.skinnum = gstate->ImageIndex(gstate->GetModelSkinName(self->s.modelindex));
		//gstate->GetModelSkinIndex (self->s.modelindex);
		gib->s.effects2	|= EF2_IMAGESKIN;
	}
    if (!(self->fragtype & FRAGTYPE_NOBLOOD))															// 5.9  don't show blood trail on mechs
      gib->s.effects = EF_GIB;
#endif
////////////////////////////////German Regulated Human Gibs Ends Here/////////////////////////////////////

	if(self->fragtype & FRAGTYPE_ROBOTIC || self->fragtype & FRAGTYPE_BONE)							
	{
		gib->elasticity = 0.45;
	}
	else
	{
		gib->elasticity = 0.85;
	}

	// NSS[1/5/00]:Set offset here
	dir.x = self->absmin.x + (offset.x);
	dir.y = self->absmin.y + (offset.y);
	dir.z = self->absmin.z + (offset.z);
	gstate->SetOrigin2 (gib, dir);																	// set origin and link entity

	
	//Render Gibs at a scale based off of the monster's min max values.	
	//Add in a tad bit of random values to get random sized gibs.  <nss>
	// NSS[1/4/00]:
	if(self->fragtype & FRAGTYPE_ROBOTIC)
	{
		scale_mod[0]		=  rnd() * 0.060f;
		if(scale_mod[0] <0.038f)
			scale_mod[0]	= 0.038f;	
		scale_mod[1]		= (crand() * 0.015f) + scale_mod[0];
		scale_mod[2]		=  rnd() * 0.060f;
		if(scale_mod[2] <0.038f)
			scale_mod[2]	= 0.038f;	

	}
	else if (self->fragtype & FRAGTYPE_BONE)
	{
		scale_mod[0]		=  rnd() * 0.030f;
		if(scale_mod[0] <0.022f)
			scale_mod[0]	= 0.022f;	
		scale_mod[1]		=  rnd() * 0.030f;
		if(scale_mod[1] <0.022f)
			scale_mod[1]	= 0.022f;	}
	else
	{
		scale_mod[0]		=  rnd() * 0.060f;
		if(scale_mod[0] <0.038f)
			scale_mod[0]	= 0.038f;	
		scale_mod[1]		= (crand() * 0.010f) + scale_mod[0];
		scale_mod[2]		=  rnd() * 0.050f;
		if(scale_mod[2] <0.038f)
			scale_mod[2]	= 0.038f;	
	}
	float mass;
	if(self->mass > 300.0f)
	{
		mass = 300 / 10.0f;
	}
	else if (self->mass < 128.0f)
	{
		mass = 128 / 10.0f;
	}
	else
	{
		mass = self->mass / 10.0f;
	}
	
	if(self->fragtype & FRAGTYPE_BONE)
	{
		gib->s.render_scale.Set((self->absmax.x - self->absmin.x) * scale_mod[0], (self->absmax.y - self->absmin.y) * scale_mod[1], (self->absmax.z - self->absmin.z) * scale_mod[0]);
	}
	else
	{
		//gib->s.render_scale.Set((self->absmax.x - self->absmin.x) * scale_mod[0], (self->absmax.x - self->absmin.x) * scale_mod[0], (self->absmax.x - self->absmin.x) * scale_mod[2]);		
		gib->s.render_scale.Set(mass * scale_mod[0], mass * scale_mod[0], mass * scale_mod[2]);
	}

	// SCG[8/24/99]: Make sure we don't create a gib we can't see
	if( gib->s.render_scale.x == 0 || gib->s.render_scale.y == 0 || gib->s.render_scale.z == 0 )
	{
		gstate->RemoveEntity( gib );
		return NULL;
	}
	
	// Set the direction of the gib, and randomize it a bit to spread the gibs out...
	dir = GibDir;

	//Calculate the damage mod based off of how much damage done.
	//The damage_mod will range from 1 to 100 which is a % of the
	//velocity that will be randomly applied to the gibs.  <nss>
    float damage_mod;
	if(gstate->damage_inflicted > 100)
	{
		damage_mod = 1.0f;
	}
	else
	{
		damage_mod = (gstate->damage_inflicted/100.0f);
	}

	//Calculate the dammage based off of a % value and a little random values with a dash of a range.<nss>
	damage_mult =  damage_mod * (rnd()*3000.0f);
	if(damage_mult < 225)
	{
		damage_mult = 225.0f;
	}
	else if(damage_mult > 300.0f)
	{
		damage_mult = 300.0f;
	}

	// NSS[1/5/00]:Two different rates for fragment types.
	if(self->fragtype & FRAGTYPE_BONE || self->fragtype & FRAGTYPE_ROBOTIC)														
	{
		//Give each little gib an extra boost for a nice spread! <nss>
		dir = dir * damage_mult;
		dir.z *= 2.45f;
		dir.x *= 1.15f;
		dir.y *= 1.15f;
	}
	else
	{
		//Give each little gib an extra boost for a nice spread! <nss>
		dir = dir * damage_mult;
		dir.z *= 2.15f;
		dir.x *= 1.65f;
		dir.y *= 1.65f;
	}

	gib->velocity	= dir + self->velocity;
	gib->avelocity	= dir * 2.5;
    
	gib->think		= gib_think;
	gib->nextthink	= gstate->time + THINK_TIME;

	gib->touch		= gib_touch_moving;
	gib->remove		= gib_remove;

	gib->delay		= gstate->time + 1.5f;
	gstate->LinkEntity(gib);
	return	gib;
}



// ----------------------------------------------------------------------------
// NSS[1/4/00]:
// Name:AI_BloodThink
// Description:Think function for the blood cloud
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void AI_BloodThink(userEntity_t *self)
{
	RELIABLE_UNTRACK(self);
	gstate->RemoveEntity(self);
}


void AI_GibFest(userEntity_t *self, userEntity_t *attacker);

// ----------------------------------------------------------------------------
// NSS[1/4/00]:
// Name:AI_BloodThink
// Description:Think function for the blood cloud
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void AI_BloodGibThink(userEntity_t *self)
{
	//Create the Special FX for the spawning
	trackInfo_t tinfo;

	self->think = AI_BloodThink;
	
	AI_GibFest(self, self);
	
	if (deathmatch->value)
		self->nextthink	= gstate->time + 0.5f;
	else
		self->nextthink	= gstate->time + 0.5f;

	// clear this variable
	memset(&tinfo, 0, sizeof(tinfo));

	tinfo.fru.Zero();
	tinfo.ent				= self;
	tinfo.srcent			= self;
	tinfo.Long2				= self->fragtype;							// fragtype
	tinfo.altpos2.x			= (self->absmax.x - self->absmin.x)*1.15;	//bounding box 
	tinfo.altpos2.y			= (self->absmax.y - self->absmin.y)*1.15;
	tinfo.altpos2.z			= (self->absmax.z)*0.50;
	tinfo.altpos			= self->s.origin;
	tinfo.Long1				= ART_BLOODCLOUD;
	tinfo.flags				= TEF_FXFLAGS|TEF_SRCINDEX|TEF_LONG1|TEF_ALTPOS|TEF_ALTPOS2|TEF_LONG2;
	tinfo.fxflags			= TEF_ARTIFACT_FX|TEF_FX_ONLY;
	
	com->trackEntity(&tinfo,MULTICAST_ALL);

}



// ----------------------------------------------------------------------------
// NSS[1/4/00]:
// Name:AI_BloodCloud
// Description:Creates an entity with no model but a nice track entity that creates 
// a nice puff of bloody looking stuff.
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void AI_BloodCloud(userEntity_t *self, userEntity_t *Attacker)
{
	userEntity_t *blood;

	blood					= gstate->SpawnEntity ();
	blood->think			= AI_BloodGibThink;
	//blood->s.renderfx	    |= RF_NODRAW;
	blood->s.render_scale.Set(0.01,0.01,0.01);
	blood->fragtype			= self->fragtype;
	blood->mass				= self->mass;
	blood->s.modelindex		= self->s.modelindex;
	blood->movetype			= MOVETYPE_NONE;
	blood->solid			= SOLID_NOT;
	blood->s.origin			= self->s.origin;
	blood->enemy			= Attacker;
	blood->absmax			= Attacker->absmax;
	blood->absmin			= Attacker->absmin;

	blood->nextthink		= gstate->time + 0.2f;

	gstate->LinkEntity(blood);

}


// ----------------------------------------------------------------------------
// NSS[1/4/00]:
// Name:AI_GibFest
// Description:Sets up the model types and determines how many will be thrown
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void AI_GibFest(userEntity_t *self, userEntity_t *attacker)
{
	int GIB_TYPE		= GIB_TORSO;
	int OFFSET_LIMIT	= 9;
	int OFFSET			= 0;
	int Max_Gibs;
	float Mod			= 0;
	float Distance		= 0.0f;	
	
	CVector Dir,Offset,SizeBase;
	
	
	
	Mod = self->mass/500.0f;
	
	if(Mod < 0.35f)
		Mod = 0.35f;
	else if (Mod > 1.0)
		Mod = 1.0f;

	Dir = attacker->s.origin - self->s.origin;
	Dir.Normalize();

	Offset = (self->absmax - self->absmin);

	
	
	if (deathmatch->value || coop->value)
		Max_Gibs = (MAX_GIBSMP * Mod)*0.5;
	else
		Max_Gibs = (MAX_GIBSSP * Mod)*0.5;

	//Make The Squishing Noise
	ai_gib_sound( self);
	
	SizeBase.Set((self->s.maxs.x - self->s.mins.x),(self->s.maxs.y - self->s.mins.y), (self->s.maxs.z - self->s.mins.z));

	if(SizeBase.z > SizeBase.x)
	{
		Mod = SizeBase.z * (1/(float)Max_Gibs);
	}
	else
	{
		Mod = SizeBase.x * (1/Max_Gibs);	
	}

	for ( int i = 0; i < (Max_Gibs) ; i++ )
	{
		
		if(OFFSET >= OFFSET_LIMIT)
		{
			OFFSET	= GIB_TORSO;
		}
		
		if(SizeBase.z > SizeBase.x)
		{

			Offset.z	= (i+3)*Mod;
			Offset.x	= rnd() * SizeBase.x;
			Offset.y	= rnd() * SizeBase.y;
		}
		else
		{
			Offset.z	= rnd() * SizeBase.z;
			Offset.x	= (i+3)*Mod;
			Offset.y	= rnd() * SizeBase.y;			
		}
		
		ai_throw_gib(self, gib_models[GIB_TYPE + OFFSET],Offset, Dir, GIB_TYPE + OFFSET);
		OFFSET++;
	}

	if(self->fragtype & FRAGTYPE_BONE)
	{
		char letter = 97+(int)(rnd()*4.0);
		char gib_sound[32];
		Com_sprintf(gib_sound,sizeof(gib_sound),"global/m_gibbonecrk%c.wav",letter);
		gstate->StartEntitySound(self,CHAN_AUTO, gstate->SoundIndex(gib_sound), 0.55f, 300.0f,800.0f);	
	}
	else if (!(self->fragtype & FRAGTYPE_ROBOTIC))
	{
		ai_smoked_sound(self);
	}
}
void AI_ThinkFade(userEntity_t * self);
void AI_SetDeathBoundingBox(userEntity_t *self);
// ----------------------------------------------------------------------------
// NSS[1/30/00]:
// Name:AI_StartGibFest
// Description:Sets up the model types and determines how many will be thrown
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void AI_StartGibFest(userEntity_t *self, userEntity_t *attacker)
{
// SCG[6/5/00]: #ifdef TONGUE_GERMAN
#ifdef NO_VIOLENCE
	AI_SetDeathBoundingBox(self);
	gstate->LinkEntity( self );
	self->think		= AI_ThinkFade;
	self->nextthink = gstate->time + 0.0f;
#else
	// NSS[1/4/00]:Create a cloud of ass and blood
	AI_BloodCloud(self,attacker);
	AI_GibFest(self, attacker);
#endif 

}

// ----------------------------------------------------------------------------
// NSS[1/4/00]:Modified
// Name:gib_client_die
// Description:
// Input:
// Output:
// Note:
//
// ----------------------------------------------------------------------------

userEntity_t * gib_client_die(userEntity_t *self, userEntity_t *attacker)
// throw out delicious body parts when a main character dies
{
	userEntity_t * pCameraTarget;
	CVector mins, maxs;

	CVector Dir;

	Dir = attacker->s.origin - self->s.origin ;
	Dir.Normalize();

	mins = self->s.mins;
	maxs = self->s.maxs;

	float mins_x_half = mins.x * 0.50;
	float mins_y_half = mins.y * 0.50;
	float mins_z_half = mins.z * 0.50;

	float maxs_x_half = maxs.x * 0.50;
	float maxs_y_half = maxs.y * 0.50;
	float maxs_z_half = maxs.z * 0.50;


	//Make Noise
	ai_gib_sound( self);

	// NSS[1/4/00]:Create a cloud or ass and blood
	AI_BloodCloud(self, attacker);
	
	pCameraTarget = ai_throw_gib (self, gib_models [GIB_HEAD], CVector(0,0,maxs.z), Dir, GIB_HEAD);  // * 2 for head position
	
	// knock the eyes out of the head and launch far
	ai_throw_gib(self, gib_models[GIB_EYE],CVector( 0,-2,maxs.z + 3), Dir, GIB_EYE);
	ai_throw_gib(self, gib_models[GIB_EYE],CVector( 0, 2,maxs.z + 3), Dir, GIB_EYE);

	// keep chest close to where body dies
	ai_throw_gib(self, gib_models[GIB_CHEST], CVector(0, 0,maxs.z * 0.50), Dir, GIB_CHEST); // waist high

	ai_throw_gib(self, gib_models[GIB_ARM], CVector(0,mins_y_half,maxs_z_half), Dir, GIB_ARM);
	ai_throw_gib(self, gib_models[GIB_ARM], CVector(0,maxs_y_half,maxs_z_half), Dir, GIB_ARM);

	// throw bonus phalanges farrrrrr
	ai_throw_gib(self, gib_models[GIB_FOOT], CVector(mins_x_half,mins_y_half,maxs.z * 0.10), Dir, GIB_FOOT);
	ai_throw_gib(self, gib_models[GIB_FOOT], CVector(maxs_x_half,maxs_y_half,maxs.z * 0.10), Dir, GIB_FOOT);

	ai_throw_gib(self, gib_models[GIB_HAND], CVector(mins_x_half,mins_y_half,maxs.z * 0.60), Dir, GIB_HAND);
	ai_throw_gib(self, gib_models[GIB_HAND], CVector(mins_x_half,mins_y_half,maxs.z * 0.60), Dir, GIB_HAND);

	return (pCameraTarget); // return camera target
}


void gib_set_bboxes(CVector * mins, CVector * maxs, byte gib_type)
// return min/max bounding boxes for specific gib types
{
  switch (gib_type)
  {
    case GIB_TORSO : mins->Set(-2, -2, -2);  maxs->Set(2, 2, 2);  break;
    case GIB_LEG   : mins->Set(-1, -1, -1);  maxs->Set(1, 1, 1);  break;
    case GIB_FOOT  : mins->Set(-3, -3, -3);  maxs->Set(3, 3, 3);  break;
    case GIB_HAND  : mins->Set(-1, -1, -1);  maxs->Set(1, 1, 1);  break;

    case GIB_HEAD  : mins->Set(-1, -1, -3);  maxs->Set(1, 1, 3);  break;

    case GIB_CHEST : mins->Set(-3, -3, -3);  maxs->Set(3, 3, 3);  break;
    case GIB_EYE   : mins->Set(-1, -1, -1);  maxs->Set(1, 1, 1);  break;
    case GIB_ARM   : mins->Set(-1, -1, -1);  maxs->Set(1, 1, 1);  break;
    case GIB_MISC  : mins->Set(-1, -1, -1);  maxs->Set(1, 1, 1);  break;
    default        : mins->Set(-1, -1, -1);  maxs->Set(1, 1, 1);  break; 
  }
  
  // TEMPORARY
  //mins->Set(  0,  0,  0);
  //maxs->Set(  0,  0,  0);

}

int AI_GibLimit(userEntity_t *self, float damage);
bool check_for_gib(userEntity_t *self, int damage)
// check if entity is able to gib
{
  if ( (AI_GibLimit(self,damage) || (self->fragtype & FRAGTYPE_ALWAYSGIB)) && 
       sv_violence->value == 0)
	  return (true);
  else
	  return (false);
}

//NSS[11/8/99]:
///////////////////////////////////////////////////////////////////////////////
//				Below Functions to be used with fGib Only!					 //
///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
//	NSS[11/7/99]:
//	Name:fGib_ThrowGib
//	Description:This function actually throws the gibs around.
//	Input:userEntity_t *self
//	Output:NA
//	Note:
// ----------------------------------------------------------------------------
userEntity_t *fGib_ThrowGib( userEntity_t *self, char *model)
{
	userEntity_t	*gib;
	CVector			mins, maxs, angles, Forward, dir,Temp;
	char			model_type[128];
	int				gib_type = 6 + (int)rnd()*8;

	if(GIB_COUNT >= GIB_MAX_GIB)
		return NULL;
	
	

	if(gstate->GetCvar("sv_violence"))
		 return NULL;
	
	GIB_COUNT++;
	
	func_gib_hook_t *hook	= (func_gib_hook_t *)self->userHook;
	gib						= gstate->SpawnEntity ();
	gib->movetype			= MOVETYPE_BOUNCE;

// SCG[6/5/00]: #ifdef TONGUE_GERMAN
#ifdef NO_VIOLENCE
	//NSS[11/19/99]:
	//We can only use robotic gibs for the German type folk.
	//Wow... big brother or what??
//	sprintf(model_type,"%s","models/e1/m_cambot.dkm");
	Com_sprintf(model_type,sizeof(model_type),"%s","models/e1/m_cambot.dkm");
	// get bounding box size of gib, dependent on type
    gib_set_bboxes(&mins, &maxs, gib_type); 	
#else
	if(	hook->InternalFlags & fGIB_ROBOTIC)				// only if no-blood flag isn't set
	{
//		sprintf(model_type,"%s",model);
		Com_sprintf(model_type,sizeof(model_type),"%s",model);
	    gib_set_bboxes(&mins, &maxs, gib_type);			// get bounding box size of gib, dependent on type
	}   
	else if (hook->InternalFlags & fGIB_BONE)
	{
//		sprintf(model_type,"models/global/g_bone.dkm");
		Com_sprintf(model_type,sizeof(model_type),"models/global/g_bone.dkm");
		mins.Set(-.5, -5.0, -0.1);  
		maxs.Set(0.5, 5, 0.1);
		CVector SmokeandShit = self->s.origin;
		// Smoke FX
		CVector color;
		color.Set( 1, 1, 1 );
		gstate->particle (TE_PARTICLE_RISE, SmokeandShit, up, 2, color, 2, PARTICLE_SMOKE|PARTICLE_RANDOM_SPREAD);
	}
	else
	{
		if(rnd() < 0.10f)
		{
			gib_BloodSplat (self);							// splat the solid surface
		}
//		sprintf(model_type,"%s",model);
		Com_sprintf(model_type,sizeof(model_type),"%s",model);
	    gib_set_bboxes(&mins, &maxs, gib_type);			// get bounding box size of gib, dependent on type
	}
#endif 
    gstate->SetSize(gib, mins.x, mins.y, mins.z, maxs.x, maxs.y, maxs.z);

	gib->s.modelindex	= gstate->ModelIndex (model_type);
	gib->solid			= SOLID_BBOX;
    gib->clipmask		= MASK_DEADSOLID;	
	gib->owner			= self;
	gib->className		= "gib";
    gib->takedamage		= 0;
	gib->gravity		= 1.0f;
	gib->mass			= 10.0;
	gib->volume			= 1.0;
    gib->fragtype		= self->fragtype;
	gib->svflags		|= SVF_NOPUSHCOLLIDE;			// amw: causes gibs to not collide with doors etc.. **TMPORARY FIX**
	gib->s.effects2		= EF2_BLOODY_GIB;
	gib->flags			|= FL_NOSAVE;

    if (!(hook->InternalFlags & fGIB_NOBLOOD))
	{
      gib->s.effects = EF_GIB;
	}

	//<nss> meat will slop onto the floor while bone and metal should bounce around a bunch.
	if(hook->InternalFlags & fGIB_ROBOTIC || hook->InternalFlags & fGIB_BONE)
	{
		gib->elasticity = 0.45;
	}
	else
	{
		gib->elasticity = 0.85;
	}

    if (hook->InternalFlags & fGIB_ROBOTIC) // use the skin from the entity we blew up, if the monster is non-fleshy 
	{
		gib->s.skinnum = gstate->GetModelSkinIndex (self->s.modelindex);
	}

	gstate->SetOrigin2 (gib,self->s.origin);  // set origin and link entity
	

	//Render Gibs at a scale based off of the scale setting

	gib->s.render_scale.Set(hook->scale, hook->scale, hook->scale);

	// SCG[8/24/99]: Make sure we don't create a gib we can't see
	if( gib->s.render_scale.x == 0 || gib->s.render_scale.y == 0 || gib->s.render_scale.z == 0 )
	{
		gstate->RemoveEntity( gib );
		return NULL;
	}
	
	// Set the direction of the gib, and randomize it a bit to spread the gibs out...


	//NSS[11/8/99]:If we have a target then create a vector towards that.
	if(hook->Target)
	{
		dir = hook->Target;
		dir.VectorToAngles(angles);
		angles.yaw		+= (crand()*0.5) * hook->spread;
		angles.pitch	+= (crand()*0.2) * hook->spread;
		angles.AngleToForwardVector(Forward);
	}
	else  //No Target... Random Directions
	{
		angles = self->s.angles;
		angles.yaw		+= crand() * hook->spread;
		angles.pitch	+= crand() * hook->spread;
		angles.AngleToForwardVector(Forward);
	}
		
	//Set the direction and the velocity of the little giblet
	dir = Forward * (hook->velocity + ((hook->velocity*0.25)*crand()));

	//Setup the rest of the crap for the little gibber.
	gib->velocity	= dir;
	gib->avelocity	= dir;
	gib->think		= gib_think;
	gib->nextthink	= gstate->time + THINK_TIME;
	gib->touch		= gib_touch_moving;
	gib->remove		= gib_remove;
	gib->delay		= gstate->time + 1.5f;

	gstate->LinkEntity(gib);
	return	gib;

}




// ----------------------------------------------------------------------------
// NSS[2/8/00]:
// Name:		fai_gib_sound
// Description: fplays a sound for gib... it used to play a sound per gib.
// Input:userEntity_t *self, userEntity_t *fGib
// Output:NA
// Note:
// creator: <nss>
// ----------------------------------------------------------------------------
void fai_gib_sound(userEntity_t *self, userEntity_t *fGib)
{
	char			gib_sound[128];
	char			letter;
	func_gib_hook_t *hook =  (func_gib_hook_t *)fGib->userHook;

	if ((self->fragtype & FRAGTYPE_ROBOTIC)) 
	{
		letter = 97+(int)(rnd()*2.0);
		Com_sprintf(gib_sound,sizeof(gib_sound),"global/m_gibsurf%c.wav",letter);
	}   
	else if (self->fragtype & FRAGTYPE_BONE)
	{
		letter = 97+(int)(rnd()*1.0);
		Com_sprintf(gib_sound,sizeof(gib_sound),"global/m_gibbone%c.wav",letter);
	}
	else
	{
		letter = 97+(int)(rnd()*4.0);
		Com_sprintf(gib_sound,sizeof(gib_sound),"global/m_gibslop%c.wav",letter);
	}
	gstate->StartEntitySound(self,CHAN_AUTO, gstate->SoundIndex(gib_sound), hook->volume,hook->smins,hook->smaxs);
}





// ----------------------------------------------------------------------------
//	NSS[11/7/99]:
//	Name:fGib_SpawnGib
//	Description:This function actually spawns the gibs.
//	Input:userEntity_t *self
//	Output:NA
//	Note:
// ----------------------------------------------------------------------------
void fGib_SpawnGib(userEntity_t *self)
{
	CVector Spread;
	func_gib_hook_t *hook =  (func_gib_hook_t *)self->userHook;
	userEntity_t *gib;
	
	for ( int i = 0; i < hook->count; i++ )
	{		
		gib = fGib_ThrowGib(self, gib_models[GIB_TORSO+((int)rnd() * 8)]);
		if(i%2 && gib)//Make Noise
			fai_gib_sound(gib,self);
	}
}


// ----------------------------------------------------------------------------
//	NSS[11/7/99]:
//	Name:fGib_Think
//	Description:This is the think function for the fGib generator
//	Input:userEntity_t *self
//	Output:NA
//	Note:
// ----------------------------------------------------------------------------
void fGib_Think(userEntity_t *self)
{
	func_gib_hook_t *hook =  (func_gib_hook_t *)self->userHook;

	//Do the gib spawning until we are done!
	if( hook->lifetime > gstate->time )
	{
		fGib_SpawnGib(self);
		self->nextthink = gstate->time + 0.8;
	}
	else//otherwise turn ourself off.
	{
		hook->onoff		= 0;
		self->think		= NULL;
		self->nextthink = -1;
	}
}


// ----------------------------------------------------------------------------
//	NSS[11/7/99]:
//	Name:fGib_Use
//	Description:This is called when something uses the fGib generator
//	Input:userEntity_t *self, userEntity_t *other, userEntity_t *activator
//	Output:NA
//	Note:
// ----------------------------------------------------------------------------
void fGib_Use(userEntity_t *self, userEntity_t *other, userEntity_t *activator)
{
	func_gib_hook_t *hook =  (func_gib_hook_t *)self->userHook;
	
	if(!(hook->InternalFlags & fGIB_NOTOGGLE) || !hook->onoff)
	{
		if(!hook->onoff)									//Turn On
		{
			hook->onoff = 1;
			self->think		= fGib_Think;
			hook->lifetime	= hook->stoptime + gstate->time;
			self->nextthink = gstate->time + 0.1;
		}
		else if(!(hook->InternalFlags & fGIB_NOTOGGLE))		//Turn off
		{
			hook->onoff		= 0;
			self->think		= NULL;
			self->nextthink = -1;
		}
	}
}


// ----------------------------------------------------------------------------
//	NSS[11/7/99]:
//	Name:fGib_ParseEpairs
//	Description:Parse through epair values setup by the mappers
//	Input:userEntity_t *self
//	Output:NA
//	Note:
// ----------------------------------------------------------------------------
void fGib_ParseEpairs(userEntity_t * self)
{
	int nSpawnValue;
	float dummy_float;

	self->flags     &= ~FL_MONSTER;
	self->flags     &= ~FL_BOT;

	func_gib_hook_t *hook =  (func_gib_hook_t *)self->userHook;
	//Setup default values
	hook->onoff				= 0;				//Start off
	hook->count				= 3;				//Number of gibs to start with
	hook->spread			= 10.0f;			//Degree of Spread for Gibs.
	hook->velocity			= 85;				//Velocity of Gibs
	hook->scale				= 1.00f;			//Scale of Gibs
	hook->stoptime			= 1.0f;				//Time for Gibs to emit.
	hook->smaxs				= 512.0f;			//max range for sound
	hook->smins				= 128.0f;			//min range for sound
	hook->volume			= 0.75f;			//volume for sound
	
	//Parse through the epair values	
	if (self->epair)
	{
	    int i = 0;
		while ( self->epair[i].key )
	    {
			//Get the values
			if( !stricmp(self->epair [i].key, "target"))//find the target and get the vector to the target
			{
				userEntity_t *ent = findEntityFromName(self->epair[i].value);
				if(ent != NULL)
				{
					hook->Target = ent->s.origin - self->s.origin;
					hook->Target.Normalize();
				}
				else
				{
					hook->Target = self->s.origin;
				}
			}
			else if ( _stricmp(self->epair[i].key, "spread") == 0 )
		    {
			    hook->spread = atoi(self->epair[i].value);
		    }
			else if ( _stricmp(self->epair[i].key, "count") == 0 )
		    {
			    hook->count = atoi(self->epair[i].value);
		    }
			else if ( _stricmp(self->epair[i].key, "velocity") == 0 )
			{
				hook->velocity = atoi(self->epair[i].value);
			}
			else if( _stricmp(self->epair[i].key, "stoptime") == 0 )
			{
				hook->stoptime = atof(self->epair[i].value);
			}
			else if( _stricmp(self->epair[i].key, "min") == 0 )
			{
				hook->smins = atof(self->epair[i].value);
			}
			else if( _stricmp(self->epair[i].key, "max") == 0 )
			{
				hook->smaxs = atof(self->epair[i].value);
			}
			else if( _stricmp(self->epair[i].key, "volume") == 0 )
			{
				hook->volume = atof(self->epair[i].value);
			}
			else if( _stricmp(self->epair[i].key, "scale") == 0)
			{
				dummy_float   = atof(self->epair[i].value);
				//Make sure we don't get a dumb number
				if(dummy_float < 0.01f)
				{
					dummy_float = 0.01f;
				}
				else if(dummy_float > 200.f)
				{
					dummy_float = 200.0f;
				}
				hook->scale =  dummy_float;
			}
			else if ( _stricmp( self->epair[i].key, "spawnflags" ) == 0 )
		    {
				hook->InternalFlags = nSpawnValue = atoi( self->epair[i].value );
				if(hook->InternalFlags & fGIB_ROBOTIC)
				{
					self->fragtype |= FRAGTYPE_ROBOTIC;					
				}
				else if (hook->InternalFlags & fGIB_ROBOTIC)
				{
					self->fragtype |= FRAGTYPE_BONE;
				}
			}
			i++;
	    }		

		//Same goes for Number of gibs
		if(hook->count > 10)
		{
			hook->count = 10;
		}
		else if(hook->count < 1)
		{
			hook->count = 1;
		}
		
		//And yes, again, same goes for velocity
		if(hook->velocity > 600)
		{
			hook->velocity = 600;
		}
		else if(hook->velocity < 1)
		{
			hook->velocity = 1;
		}
		
		self->use = fGib_Use;
		

		if(hook->InternalFlags & fGIB_STARTON)
		{
			self->think		= fGib_Think;
			hook->lifetime	= hook->stoptime + gstate->time;
			self->nextthink = gstate->time + 0.1;
		}
		else
		{
			hook->onoff		= 0;
			self->think		= NULL;
			self->nextthink = -1;
		}
	}
	else  // No epairs... something is fucked... let's get rid of ourself.
	{
		self->remove(self);
	}

}

// ----------------------------------------------------------------------------
//	NSS[11/7/99]:
//	Name:func_gib
//	Description:Gib'age generator for places where you want to make a mess
//	Input:userEntity_t *self
//	Output:NA
//	Note:
// ----------------------------------------------------------------------------
void func_gib(userEntity_t *self)
{
	CVector	temp, ang;
	
	self->userHook = gstate->X_Malloc(sizeof(func_gib_hook_t),MEM_TAG_COM_MISC);

	self->className     = "gib_generator";

	self->s.modelindex  = 0;
	
	self->movetype  = MOVETYPE_NONE;

	self->solid     = SOLID_NOT;

    self->think     = fGib_ParseEpairs;
	self->nextthink = gstate->time + 1.05 + (rnd()*1.5);

	self->velocity.Zero();

	gstate->LinkEntity( self );

	self->flags     &= ~FL_MONSTER;
	self->flags     &= ~FL_BOT;
	self->flags		|= FL_NOSAVE;
}


///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_gib_register_func()
{
	gstate->RegisterFunc("AI_BloodThink",AI_BloodThink);
	gstate->RegisterFunc("gib_touch_stopped",gib_touch_stopped);
	gstate->RegisterFunc("gib_touch_moving",gib_touch_moving);
	gstate->RegisterFunc("gib_shrink",gib_shrink);
//	gstate->RegisterFunc("gib_spin",gib_spin);
//	gstate->RegisterFunc("gib_rise",gib_rise);
	gstate->RegisterFunc("gib_fadeout",gib_fadeout);
	gstate->RegisterFunc("gib_think",gib_think);
//	gstate->RegisterFunc("gib_percent_above_below",gib_percent_above_below);
	gstate->RegisterFunc("fGib_Think",fGib_Think);
	gstate->RegisterFunc("fGib_ParseEpairs",fGib_ParseEpairs);
	gstate->RegisterFunc("fGib_Use",fGib_Use);
}
