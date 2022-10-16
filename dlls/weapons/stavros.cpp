//---------------------------------------------------------------------------
// STAVROS STAVE
//---------------------------------------------------------------------------
#include "weapons.h"
#include "stavros.h"

void stavrosThink(userEntity_t *self);

enum
{
    // model defs
    MDL_START,
    MDL_METEOR,

    // sound defs
    SND_START,
    SND_FLYBY,
    SND_BOUNCE,

    // frame defs
    ANIM_START,
    ANIM_METEOR
};

// weapon info def
weaponInfo_t stavrosWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_STAVROS),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_STAVROS_AMMO),
	// names, net names, icon
	"weapon_stavros",
	tongue_weapons[T_WEAPON_STAVROS],
	"ammo_stavros",
	tongue_weapons[T_WEAPON_STAVROS_AMMO],
	"iconname",

    // model files
	{
		"models/e3/w_stavros.dkm",
		"models/e3/a_stav.dkm",
		"models/e3/wa_stav.dkm",
		"models/e3/we_meteor.dkm",
		NULL
	},

    // sound files
	{
		SND_NOSOUND,
		"e3/we_staveready.wav",
		"e3/we_staveaway.wav",
		"e3/we_stavefire.wav",
		SND_NOSOUND,
		SND_NOSOUND,
		SND_NOSOUND,
		"e3/we_staveflya.wav",
		"e3/we_stavebounce.wav",
		NULL
	},

    // frame names
	{
		MDL_WEAPON_STD,"ready",0,0,
		-1,"away",0,0,
		-1,"shoot",0,0,
		-1,"amba",0,0,
		ANIM_NONE,
		ANIM_NONE,
		MDL_METEOR,"sp2",0,0,
		0,
		NULL
	},

    // commands
	{
		wpcmds[WPCMDS_STAVROS][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_3,
		wpcmds[WPCMDS_STAVROS][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_3,
		NULL
	},

	weapon_give_3,
	weapon_select_3,
	stavrosCommand,
	stavrosUse,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(stavros_t),

	300,
	50,     //	ammo_max
	1,      //	ammo_per_use
	1,      //	ammo_display_divisor
	20,     //	initial ammo
	425.0,  //	speed
	4250.0, //	range
	100.0,  //	damage
	10.0,       //	lifetime

	WFL_FORCE_SWITCH|WFL_CROSSHAIR|WFL_PLAYER_ONLY,      //	flags

	WEAPONTYPE_RIFLE,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon3[T_DEATHMSGWEAPON_STAVROS],
		NULL,
		VICTIM_ATTACKER
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon3,T_DEATHMSGWEAPON_STAVROS),
	VICTIM_ATTACKER,

    WEAPON_STAVROS,
    0,
    NULL
};

// local weapon info access
static weaponInfo_t *winfo=&stavrosWeaponInfo;

//---------------------------------------------------------------------------
// give()
//---------------------------------------------------------------------------
userInventory_t *stavrosGive(userEntity_t *self, int ammoCount)
{
    return(weaponGive(self,winfo,ammoCount));
}

//---------------------------------------------------------------------------
// stavros_select()
//---------------------------------------------------------------------------
short stavrosSelect(userEntity_t *self)
{
    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// command()
//---------------------------------------------------------------------------
void *stavrosCommand(struct userInventory_s *ptr, char *commandStr, void *data)
{
    winfoGenericCommand(winfo,ptr,commandStr,data);

    return(NULL);
}

void stave_attack( userInventory_s *ptr, userEntity_t *self );
//---------------------------------------------------------------------------
// use()
//---------------------------------------------------------------------------
void stavrosUse(userInventory_s *ptr, userEntity_t *self)
{
    playerHook_t *phook=(playerHook_t *)self->userHook;
    userEntity_t *went=(userEntity_t *)self->client->ps.weapon;

    if ( !weaponHasAmmo(self,true) )
        return;

    weaponUseAmmo (self, false);

    winfoPlay(self,SND_SHOOT_STD,1.0f);
    winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,0.05);

//   trackAddEffect(went,0,TEF_STAVROS_START);

    phook->fxFrameNum=-1;//winfoFirstFrame(ANIM_SHOOT_STD)+10;
    phook->fxFrameFunc=NULL;//stavrosShoot;

	stave_attack( ptr, self );
}


//-----------------------------------------------------------------------------
//	weapon_stavros
//-----------------------------------------------------------------------------
void weapon_stavros(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}


// ----------------------------------------------------------------------------
// <nss>
// Name:		ZapFlareRotateThink
// Description:Flare for Wyndrax's attack
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void ZapFlareRotateThink(userEntity_t *self)
{
	ZapFlare_t *hook	= (ZapFlare_t *)self->userHook;
	if(hook->time_to_live < gstate->time)
	{
		gstate->RemoveEntity(self);
	}
	self->s.alpha = self->s.alpha * hook->delta_alpha;
	self->s.render_scale = self->s.render_scale * 0.85;
	self->s.angles = self->s.angles + hook->Delta_Angles;
	self->nextthink		= gstate->time + 0.1f;
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		spawnZapFlareRotate
// Description: This is a generic flare routine which can be used to create nice
// flare FX for monster's firing weapons and such.
// Input:userEntity_t *self, CVector *Origin
// Output:NA
// Note:Changing the scale values so that they are not uniform will result in sharper
//edges.  Life is the base for the depletion rate of scale and alpha level
// ----------------------------------------------------------------------------
userEntity_t * spawnZapFlareRotate(userEntity_t *self, CVector *Origin, CVector Rotate,CVector Size,float life, char *flarename)
{
	userEntity_t *Flare;
    ZapFlare_t *hook;
	
	
	Flare = gstate->SpawnEntity();
	Flare->flags |= FL_NOSAVE;
	Flare->s.render_scale = Size;
	
	Flare->className		= "flare_sprite";
	Flare->s.renderfx		|= (SPR_ALPHACHANNEL|SPR_ORIENTED);
	Flare->s.modelindex		= gstate->ModelIndex (flarename);
	Flare->movetype			= MOVETYPE_NONE;
	Flare->solid			= SOLID_NOT;
	Flare->owner			= self;
	Flare->s.alpha			= 0.75f;
	Flare->s.frame			= 0;
	Flare->s.origin.x		= Origin->x;
	Flare->s.origin.y		= Origin->y;
	Flare->s.origin.z		= Origin->z;
	Flare->s.frame			= 0;
    Flare->think			= ZapFlareRotateThink;
    Flare->nextthink		= gstate->time + 0.1f;
	Flare->clipmask			= MASK_SHOT;
	Flare->s.angles			= self->s.angles;
	Flare->userHook			= gstate->X_Malloc(sizeof(ZapFlare_t),MEM_TAG_HOOK);
	hook					=( ZapFlare_t *)Flare->userHook;
    hook->Delta_Angles		= Rotate;
	hook->time_to_live		= gstate->time+life;
	

	if(life > 1.0f)
	{
		hook->delta_alpha = 0.95f;
		hook->delta_scale = 0.85f;
	}
	else
	{
		hook->delta_alpha = life;
		hook->delta_scale = life;
	}
	
	
	gstate->LinkEntity(Flare);
	
	return Flare;
}


/* ************************************ Stavros' Stave ************************************** */
//NSS[10/29/99]:
//---------------------------------------------------------------------------
//meteor_explode()
//NSS[10/27/99]: The remove function for the fragments
//---------------------------------------------------------------------------
void meteor_explode( userEntity_t *self, cplane_t *plane, userEntity_t *other )
{
	staveHook_t *hook=(staveHook_t *)self->userHook;
	
	//remove the tracked entities
	winfoStopLooped(self);
	RELIABLE_UNTRACK(self);

	//Stop the looping sound
	gstate->StartEntitySound( self, CHAN_LOOP, gstate->SoundIndex("global/e_firetravela.wav"), 0.0f, 0.0f, 0.0f );

	//Make the explosion
	if (!plane || !other)
	{
		gstate->RemoveEntity(self);
		return;
	}
    CVector color(.85,.35,.15);
    spawnPolyExplosion(self->s.origin, plane->normal, 1, 250, color, PEF_NORMAL|PEF_SOUND);
	spawn_sprite_explosion( self, self->s.origin, true );

	//Make the scorch mark
	weaponScorchMark2(self,other,plane);

	//do radius dammage
	if (hook)
	{
		// cek[1-24-00]: set the attackweapon
		winfoAttackWeap;

		int num = com->RadiusDamage( self, hook->owner, self, self->health, DAMAGE_EXPLOSION, hook->damage );
		WEAPON_HIT(hook->owner,num);
	}

	gstate->RemoveEntity(self);
}

//---------------------------------------------------------------------------
//Meteor_touch()
//NSS[10/27/99]:modified the touch function for fragments
//---------------------------------------------------------------------------
void meteor_touch(userEntity_t *self,userEntity_t *other,cplane_t *plane,csurface_t *surf)
{
	winfoStopLooped(self);
	staveHook_t *hook=(staveHook_t *)self->userHook;
	if (!hook)
	{
		gstate->RemoveEntity(self);
		return;
	}

	if(other != self && other != self->owner && hook->bounce_count >= 1 )
		meteor_explode( self, plane, other );
	else
		hook->bounce_count++;
}

//---------------------------------------------------------------------------
//Meteor_think()
//NSS[10/27/99]:Went ahead and added a check to make sure it gets destroyed 
//no matter what.
//---------------------------------------------------------------------------
void meteor_think(userEntity_t *self)
{
	//NSS[10/27/99]:Why set the renderfx to 0?
	staveHook_t *hook=(staveHook_t *)self->userHook;
	
	if(!hook || hook->time_to_live < gstate->time)
	{
		//remove the tracked entities
		winfoStopLooped(self);
		RELIABLE_UNTRACK(self);
		gstate->RemoveEntity(self);
		return;
	}
	else
	{
		self->s.angles.roll		+= hook->roll_speed;
		self->s.angles.pitch	+= hook->pitch_speed;	
		self->s.angles.yaw		+= hook->yaw_speed;
	}
	self->nextthink = gstate->time + 0.1;
}


//---------------------------------------------------------------------------
//MeteorFragmentSpawn()
//NSS[10/27/99]:Spawn function for fragments
//---------------------------------------------------------------------------
void MeteorFragmentSpawn(CVector vDirectional, CVector Origin, int Damage, userEntity_t * owner)
{
	userEntity_t	*temp;
	CVector			org, end, dir, ang, p_org;

	temp = gstate->SpawnEntity();
	temp->elasticity = 1.0;
	temp->flags |= FL_NOSAVE;
	temp->svflags	= SVF_SHOT;
	temp->movetype	= MOVETYPE_BOUNCE;
	temp->solid		= SOLID_BBOX;
	temp->s.effects = EF_METEOR;
	temp->owner		= NULL;
	temp->s.frame	= 0;

	temp->clipmask	= MASK_SHOT;
	temp->health = 0.5 * winfo->damage;

	temp->s.modelindex = gstate->ModelIndex( "models/e3/we_meteor.dkm" );
	
	gstate->SetSize(temp, -8, -8,-16, 8, 8, 0);
	
	VectorToAngles(vDirectional,ang);
	AngleToVectors(ang,forward,right,up);

	ang.pitch += 45 * crand();
	ang.yaw += 45 * crand();
	AngleToVectors(ang,vDirectional);
/*
	Origin.x	+=	crand() * 25.0f;
	Origin.y	+=	crand() * 25.0f;
	Origin.z	+=	crand() * 25.0f;
*/
	Origin += 50 * crand() * up + 50 * crand() * right + 10 * forward;
	gstate->SetOrigin2( temp, Origin );

	temp->velocity = vDirectional;
	temp->velocity.Normalize();
	temp->velocity.Multiply(winfo->speed * 0.75);

	//	align model in direction of travel
	VectorToAngles( temp->velocity, ang );
	temp->s.angles	= ang;

	temp->touch		= meteor_touch;

	temp->think = meteor_think;
	temp->nextthink = gstate->time + gstate->frametime;

	float Scale_Base = (rnd()* 0.35) + .30;
	temp->s.render_scale.Set(Scale_Base+(rnd()*0.20f),Scale_Base+(rnd()*0.20f),Scale_Base+(rnd()*0.20f));
	
	gstate->LinkEntity( temp );

	//Attach the Sprite Light&Glow and fire stuff 
    trackInfo_t tinfo;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=temp;
    tinfo.srcent=temp;
    tinfo.fru.Zero();
    tinfo.lightColor.x = 0.85;//R
    tinfo.lightColor.y = 0.35;//G
    tinfo.lightColor.z = 0.15;//B
    tinfo.lightSize= 200.00;	
	
	tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_MODELINDEX|TEF_SCALE;
    tinfo.fxflags = TEF_LIGHT|TEF_SPRITE|TEF_STAVROS;
	tinfo.renderfx = SPR_ALPHACHANNEL;
	tinfo.scale = 1.2f * (Scale_Base * 0.65f);
	tinfo.modelindex = gstate->ModelIndex ("models/e3/we_fglow.sp2");
	
	com->trackEntity(&tinfo,MULTICAST_PVS);


	//NSS[10/27/99]: Let's create a user hook specificly for the meteor
	temp->userHook = gstate->X_Malloc(sizeof(staveHook_t),MEM_TAG_HOOK);
    staveHook_t *hook=(staveHook_t *)temp->userHook;

	//set a +/- random value for pitch and roll
	hook->pitch_speed = crand() * 60.0f;
	hook->roll_speed = crand() * 60.0f;

	//Base Damage on how big they are.
	hook->damage = 0.5 * winfo->range;//(Scale_Base/0.25f)*Damage;

	//Set a self destruct time(6 seconds roughly)
	hook->time_to_live = gstate->time + 6.0f;
	hook->owner = owner;

	//Set the current bounce count
	hook->bounce_count = 0;

	//Set the random maximum bounce count
	hook->bounce_max = 2 + rnd()*3;
	
//	gstate->StartEntitySound( temp, CHAN_LOOP, gstate->SoundIndex("global/e_travela.wav"), 1.0f, 128.0f, 256.0f );

}


// ----------------------------------------------------------------------------
//NSS[10/27/99]:
// Name:         stave_destroy
// Description:This will remove the main Huge meteor and will make little tiny baby
// meteors... awe.
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void stave_destroy( userEntity_t * self, cplane_t *plane, userEntity_t *other)
{
	staveHook_t *hook=(staveHook_t *)self->userHook;
	if (!hook)
	{
		gstate->RemoveEntity(self);
		return;
	}

	CVector Reflect,Target_Angles,Forward;
	float Magnitude = self->velocity.Length();
	
	if (plane)
	{
		Reflect = plane->normal;
	}
	else
	{
		//get the current velocity
		Reflect = self->velocity;
		Reflect.Normalize();
		//Invert it(point backwards)
		Reflect.Negate();
	}
	
	
	int Frags = 4+(int)(rnd()*3);
	if (deathmatch->value || coop->value)
		Frags = 0;
	//Spawn multiple fragments
	for(int i=0; i < Frags ;i++)
	{
		//Convert the angles
		VectorToAngles(Reflect,Target_Angles);

		//Let's add about a 30 degree spread from the origin of the returning vector
		Target_Angles.yaw += crand()*45;
		Target_Angles.pitch += crand()*45;

		Target_Angles.AngleToForwardVector(Forward);

		Forward = Forward * (Magnitude*1.85);		
		
		MeteorFragmentSpawn( Forward, self->s.origin, hook->damage, self->owner);
	}
	

	//gstate->StartEntitySound( self, CHAN_AUTO, gstate->SoundIndex("global/e_explode1.wav"), 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX );

	//Make the explosion
    CVector color(0.85,0.35,0.15);

	// WAW[11/17/99]: Deal with the plane being null.
	CVector *pln;
	if (plane && plane->normal) 
		pln = &plane->normal;
	else
		pln = &zero_vector;

	spawnPolyExplosion(self->s.origin, *pln,2, 450, color, PEF_NORMAL|PEF_SOUND);
	spawn_sprite_explosion( self, self->s.origin, true );
	//Make the scorch mark
	weaponScorchMark2(self,other,plane);

	//do radius dammage
	// cek[1-24-00]: set the attackweapon
	winfoAttackWeap;

	int num = com->RadiusDamage( self, self->owner, self->owner, self->health, DAMAGE_EXPLOSION, hook->damage );
	WEAPON_HIT(self->owner,num);

	//remove the tracked entities
	RELIABLE_UNTRACK(self);

	//Stop the looping sound
	gstate->StartEntitySound( self, CHAN_LOOP, gstate->SoundIndex("global/e_firetravele.wav"), 0.0f, 0.0f, 0.0f );
	
	gstate->RemoveEntity(self);
	
}


//---------------------------------------------------------------------------
// touch()
//---------------------------------------------------------------------------
void stave_touch(userEntity_t *self,userEntity_t *other,cplane_t *plane,csurface_t *surf)
{
	winfoStopLooped(self);
	if(other != self && other != self->owner)
		stave_destroy( self, plane, other );
}

//---------------------------------------------------------------------------
// think()
//NSS[10/27/99]:Went ahead and added a check to make sure it gets destroyed 
//no matter what.
//---------------------------------------------------------------------------
#define SPEEDUP_L1				1.75
#define SPEEDUP_L2				2.5
#define GROW_RATE				0.1
void stave_think(userEntity_t *self)
{
	staveHook_t *hook=(staveHook_t *)self->userHook;
	
	if(!hook || (hook->time_to_live < gstate->time))
	{
		winfoStopLooped(self);
		gstate->RemoveEntity(self);
	}
	else
	{
		//Keep it turning
		self->s.angles.roll		+= hook->roll_speed;
		self->s.angles.pitch	+= hook->pitch_speed;	
		self->s.angles.yaw		+= hook->yaw_speed;

		//Warp in handling here
		if(self->s.render_scale.x < 1.0f)
		{
			//We are warping in here thus change the size to make the appearance of depth
			self->s.render_scale.x += GROW_RATE;
			self->s.render_scale.y += GROW_RATE;
			self->s.render_scale.z += GROW_RATE;

			//As we change size let's slow the turn rate down
			if(hook->roll_speed > 5.0f)
			{
				hook->roll_speed	-= 15.0f;
				hook->pitch_speed	-= 15.0f;
				hook->yaw_speed		-= 15.0f;
			}
			
			if( self->velocity.Length() < hook->speed_max)
			{
				if (self->velocity.Length() < hook->speed_max * 0.20f)
					self->velocity = self->velocity * SPEEDUP_L1;				
				else
					self->velocity = self->velocity * SPEEDUP_L2;	
			}
		}
		
	}
	
	
	self->nextthink = gstate->time + 0.1;
}

// ----------------------------------------------------------------------------
// <nss>
// Name:		StavrosZapFlareRotateThink
// Description:Flare for Wyndrax's attack
// Input:userEntity_t *self
// Output:NA
// Note:
//
// ----------------------------------------------------------------------------
void StavrosFlareThink(userEntity_t *self)
{
	ZapFlare_t *hook	= (ZapFlare_t *)self->userHook;// SCG[1/24/00]: not used
	if(!hook || (hook->time_to_live < gstate->time))
	{
		//remove the tracked entities
		RELIABLE_UNTRACK(self);
		gstate->RemoveEntity(self);
	}
	if(self->s.render_scale.x > 0.80)
	{
		hook->delta_scale = - 0.07f;
	}
	else if(self->s.render_scale.x < 0.10)
	{
		hook->delta_scale = 0.0f;
	}
	self->s.render_scale.x	+= hook->delta_scale;
	self->s.render_scale.y	+= hook->delta_scale;
	self->s.render_scale.z	+= hook->delta_scale;
	self->s.angles			+= hook->Delta_Angles;
	
	self->nextthink		= gstate->time + 0.1f;

}

//---------------------------------------------------------------------------
// Stave Attack
//NSS[10/27/99]: Modified Stave Attack for Stavros
//ehum.. added a few things here and there.  :)
//---------------------------------------------------------------------------
void stave_attack( userInventory_s *ptr, userEntity_t *self )
{
	WEAPON_FIRED(self);
	CVector Offset, Target_Angles, Forward, Scale, Rotate;
	trackInfo_t tinfo;
	userEntity_t *pStave;

    CVector offset( winfo->fWeaponOffsetX1, winfo->fWeaponOffsetY1, winfo->fWeaponOffsetZ1 );
    pStave=weapon_spawn_projectile(self,offset,winfo->speed,0,"proj_stavros_meteor");
	//We are warping in from inner space through a worm hole created by
	//electromagnetic shockwaves rippling time and the fabric of space.
	//The meteor should start off looking small and then end up looking 
	//large
	
	///////////////////////Meteor Code Starts Here///////////////////////
	pStave->movetype	= MOVETYPE_FLYMISSILE;
	pStave->s.render_scale.Set(0.2f,0.2f,0.2f);
	pStave->health = winfo->damage;
	pStave->s.renderfx=0;
	pStave->s.modelindex = gstate->ModelIndex("models/e3/we_meteor.dkm");
	pStave->s.effects = EF_METEOR;

	pStave->touch = stave_touch;
	pStave->think = stave_think;
	pStave->nextthink = gstate->time + gstate->frametime;

	pStave->s.frame = 0;
	gstate->SetSize(pStave,-12,-12,-12,12,12,12);

	pStave->owner	= self;	

	Target_Angles = self->client->v_angle;
	Target_Angles.yaw	+= 35;
	Target_Angles.pitch -= 45;
	Target_Angles.AngleToForwardVector(Forward);

	pStave->s.origin = (Forward * 25)+self->s.origin;
	pStave->s.origin.z += 25.0f;

	//We want to start off nice and slow
	AngleToVectors(self->client->v_angle,forward,right,up);
	pStave->velocity = forward;
	pStave->velocity.Normalize();
	pStave->velocity =  pStave->velocity * (winfo->speed*0.05);
	VectorToAngles(forward,pStave->s.angles);

	//Link the bitch	
	gstate->LinkEntity( pStave );

	//Attach the Sprite Light&Glow and fire stuff 


    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent			=pStave;
    tinfo.srcent		=pStave;
    tinfo.fru.Zero();
    tinfo.lightColor.x	= 0.55;//R
    tinfo.lightColor.y	= 0.35;//G
    tinfo.lightColor.z	= 0.15;//B
    tinfo.lightSize		= 300.00;
	
	tinfo.flags			= TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_MODELINDEX|TEF_SCALE;
    tinfo.fxflags		= TEF_LIGHT|TEF_SPRITE|TEF_STAVROS;
	tinfo.renderfx		= SPR_ALPHACHANNEL;
	tinfo.scale			= 2.75f;
	tinfo.modelindex	= gstate->ModelIndex ("models/e3/we_fglow.sp2");
	
	com->trackEntity(&tinfo,MULTICAST_PVS);
	
	
	//NSS[10/27/99]: Let's create a user hook specificly for the meteor
	pStave->userHook = gstate->X_Malloc(sizeof(staveHook_t),MEM_TAG_HOOK);
    staveHook_t *hook=(staveHook_t *)pStave->userHook;

	//Save the max velocity
	hook->speed_max = winfo->speed;

	//set a +/- random value for pitch and roll
	hook->pitch_speed	= crand() * 40.0f;
	hook->roll_speed	= crand() * 40.0f;
	hook->yaw_speed		= crand() * 40.0f;
	
	//Let's set the damage based off of the CSV file (or, lets just get them from winfo!)
	hook->damage = winfo->range;

	//Set a self destruct time(6 seconds roughly)
	hook->time_to_live = gstate->time + 12.0f;

	////////////////////Special FX for launching Starts Here////////////////////
	
	CVector *Origin = &pStave->s.origin;
	Scale.Set(0.1f,0.1f,0.1f);
	Rotate.Set(0.0f,0.0f,15.0f);
	//spawnZapFlareRotate(userEntity_t *self, CVector *Origin, CVector Rotate,CVector Size,float life, char *flarename)
	userEntity_t *ent	= spawnZapFlareRotate(self, Origin,Rotate,Scale, 0.9f, "models/e3/we_blackhole.sp2");
    ent->s.alpha		= 0.75f;
	ent->s.frame		= 10;
	ent->think			=  StavrosFlareThink;

	ZapFlare_t *zHook	= (ZapFlare_t *) ent->userHook;
	zHook->delta_scale	= 0.35f;

	// clear this variable
    memset(&tinfo, 0, sizeof(tinfo));
	//Create the negative light FX to simulate the wormhole'ish thingy going down

    tinfo.ent			= ent;
    tinfo.srcent		= ent;
    tinfo.fru.Zero();
    tinfo.lightColor.x	= -1.0;//R
    tinfo.lightColor.y	= -.5;//G
    tinfo.lightColor.z	= -.5;//B
    tinfo.lightSize		= 300.0;
	tinfo.length		= 05.0f;			//Set the rate to shrink the light
	
	tinfo.flags			= TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_LENGTH;
    tinfo.fxflags		= TEF_LIGHT_SHRINK;
	
	com->trackEntity(&tinfo,MULTICAST_PVS);

	gstate->StartEntitySound( pStave, CHAN_LOOP, gstate->SoundIndex("global/e_torchd.wav"), 1.0f, 128.0f, 512.0f );	

}

//-----------------------------------------------------------------------------
//	ammo_stavros
//-----------------------------------------------------------------------------
void ammo_stavros (userEntity_t *self)
{
    winfoAmmoSetSpawn(winfo, self, 2, 30.0, NULL);

	if (!self->inuse || !stricmp(self->className,"freed"))
		return;

	self->s.sound = gstate->SoundIndex("global/e_torchd.wav"); 
	self->s.volume = 0.25;
	self->movetype = MOVETYPE_TOSS;
	self->s.dist_min = 64;		
	self->s.dist_max = 128;		
	self->s.effects2 |= EF2_STAVROS_AMMO;
	gstate->LinkEntity(self);
}


///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_stavros_register_func()
{
	gstate->RegisterFunc("stavrosstave_thinkk",stave_think);
	gstate->RegisterFunc("stavrosmeteor_think",meteor_think);
	gstate->RegisterFunc("stavrosUse",stavrosUse);
	gstate->RegisterFunc("stavrosStavrosFlareThink",StavrosFlareThink);
	gstate->RegisterFunc("stavrosZapFlareRotateThink",ZapFlareRotateThink);
	gstate->RegisterFunc("stavrosmeteor_touch",meteor_touch);
	gstate->RegisterFunc("stavrosstave_touch",stave_touch);
	gstate->RegisterFunc("stavrosGive",stavrosGive);
	gstate->RegisterFunc("stavrosSelect",stavrosSelect);
}

