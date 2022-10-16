//---------------------------------------------------------------------------
// HAMMER
//---------------------------------------------------------------------------
#include "weapons.h"
#include "hammer.h"

enum
{
    // model defs
    MDL_START,
    MDL_QUAKERING,

    // sound defs
    SND_START,
    SND_HIT1,
    SND_HIT2,
    SND_HIT3,
    SND_HIT4,
    SND_HIT5,
    SND_HIT6,
    SND_HIT7,
	SND_RAISE,
	SND_DROP,

    // special frame defs
    ANIM_START
//   ANIM_QUAKERING,
};

// weapon info def
weaponInfo_t hammerWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_HAMMER),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_NO_AMMO),
	// names, net names, icon
	"weapon_hammer",
	tongue_weapons[T_WEAPON_HAMMER],
	"ammo_hammer",
	tongue_weapons[T_WEAPON_NO_AMMO],
	"iconname",

	// model files
	{
		"models/e2/w_hammer.dkm",
		"models/e2/a_hammer.dkm",
		MDL_NOMODEL,
		"models/e2/we_hammer.sp2",
		NULL
	},

	// sound files
	{
		SND_NOSOUND,
		"e2/we_hammerready.wav", 
		"e2/we_hammeraway.wav", 
		SND_NOSOUND, 
		SND_NOSOUND, 
		SND_NOSOUND, 
		SND_NOSOUND, 
		"global/e_explodea.wav",
		"global/e_exploded.wav",
		"global/e_explodee.wav",
		"global/e_explodef.wav",
		"global/e_explodeg.wav",
		"global/e_explodel.wav",
		"global/e_explodem.wav",
		"e2/we_hammerR.wav",
		"e2/we_hammerD.wav",
		NULL
	},

	// frame names
	{
		MDL_WEAPON_STD,"ready",0,0,
		-1,"away",0,0,
		-1,"shoota",0,0,
		-1,"amba",0,0,
		ANIM_NONE,
		ANIM_NONE,
		0,
		NULL
	},

	// commands
	{  
		wpcmds[WPCMDS_HAMMER][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_4,
		wpcmds[WPCMDS_HAMMER][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_4,
		NULL
	},

	weapon_give_4,
	weapon_select_4,
	hammerCommand,
	hammerUse,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(hammer_t),

	400,
	0,        //	ammo_max
	0,        //	ammo_per_use
	1,        //	ammo_display_divisor
	0,      //	initial ammo
	0,        //	speed
	0,        //	range
	100,        //	damage
	0.0,      //	lifetime

	WFL_SPECIAL|WFL_PLAYER_ONLY,  // flags

	WEAPONTYPE_RIFLE,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon2[T_DEATHMSGWEAPON_HAMMER],
		NULL,
		ATTACKER_VICTIM
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon2,T_DEATHMSGWEAPON_HAMMER),
	ATTACKER_VICTIM,

	WEAPON_HAMMER,
	0,
	NULL
};

// local weapon info access
static weaponInfo_t *winfo=&hammerWeaponInfo;
#define HAMMER_FX_TIME  .75

//---------------------------------------------------------------------------
// give()
//---------------------------------------------------------------------------
userInventory_t *hammerGive(userEntity_t *self, int ammoCount)
{
    return(weaponGive(self,winfo,ammoCount));
}

//---------------------------------------------------------------------------
// hammer_select()
//---------------------------------------------------------------------------
short hammerSelect(userEntity_t *self)
{
    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// command()
//---------------------------------------------------------------------------
void *hammerCommand(struct userInventory_s *inv, char *commandStr, void *data)
{
    userEntity_t *self=(userEntity_t *)data;
    playerHook_t *phook=(playerHook_t *)self->userHook;

    if ( !stricmp(commandStr,"ambient") )
    {
		return NULL;
        hammer_t *hammer=(hammer_t *)self->curWeapon;

        if ( gstate->time > hammer->smashtime )
            winfoAnimate(winfo,self,ANIM_AMBIENT_STD,FRAME_ONCE|FRAME_WEAPON|FRAME_NODELAY,.05);
        else
            phook->ambient_time=gstate->time+winfoNumFrames(ANIM_AMBIENT_STD)*.05+2.0+(rnd()*6);
    }
    else
        winfoGenericCommand(winfo,inv,commandStr,data);

    return(NULL);
}

//---------------------------------------------------------------------------
// use()
//---------------------------------------------------------------------------
void hammerUse(userInventory_s *ptr, userEntity_t *self)
{
    playerHook_t *phook=(playerHook_t *)self->userHook;
    userEntity_t *ent,*weapon=(userEntity_t *)self->client->ps.weapon;
    hammer_t *hammer=(hammer_t *)self->curWeapon;

    phook->attack_finished=gstate->time+0.2;

    if ( gstate->time >= hammer->smashtime )
    {
        ent=gstate->SpawnEntity();
        ent->className="hammer_controller";
		ent->flags |= FL_NOSAVE;
        ent->owner=self;
        ent->svflags |= SVF_SHOT;
        winfoSetModel(ent,MDL_WEAPON_STD);
        ent->s.render_scale.Set(.001,.001,.001);
        gstate->SetSize(ent,-1,-1,-1,1,1,1);

        ent->think=hammerThink;
        ent->nextthink=gstate->time+.1;

		ent->userHook = gstate->X_Malloc(sizeof(hammerHook_t),MEM_TAG_HOOK);

        hammer->power=0;
        entAnimate(weapon,winfoFirstFrame(ANIM_SHOOT_STD),winfoFirstFrame(ANIM_SHOOT_STD),FRAME_ONCE,.05);
        weapon->s.renderfx |= RF_SHAKE;
    }

    hammer->smashtime=gstate->time+.5;
}

//---------------------------------------------------------------------------
// think()
//---------------------------------------------------------------------------
void hammerThink(userEntity_t *self)
{
    userEntity_t    *owner=self->owner;
    userEntity_t    *weapon=(userEntity_t *)owner->client->ps.weapon;
    playerHook_t    *phook=(playerHook_t *)owner->userHook;
    hammerHook_t    *hook=(hammerHook_t *)self->userHook;
    hammer_t        *hammer=(hammer_t *)owner->curWeapon;
//    short           killquake=false;// SCG[1/24/00]: not used
    int             iHitSound;
    CVector         vecCrackEarth;


    if(!hammer || !hook)
        return;
    // time to smash hammer on the ground
    if ( hammer->smashtime > 0 )
    {
        if ( gstate->time >= hammer->smashtime )
        {
            entAnimate(weapon,weapon->s.frame,winfoLastFrame(ANIM_SHOOT_STD),FRAME_ONCE,.025);
            phook->attack_finished=gstate->time+(winfoLastFrame(ANIM_SHOOT_STD)-weapon->s.frame)*.025 + 0.05;
            hammer->smashtime=-1;

            hammer->weapon.lastFired=gstate->time;
			weapon_PowerSound(owner);
			WEAPON_FIRED(owner);
        }
        else
        {
            // slowly bring hammer up
            weapon->s.frame++;
            if ( weapon->s.frame > HAMMER_UPSTOP_FRAME+1 )
			{
				// play that grunt thinggy
				weapon->s.frame = HAMMER_UPSTOP_FRAME;
			}
			else
			{
				if (!((weapon->s.frame - winfoFirstFrame(ANIM_SHOOT_STD)) % 5))
					winfoPlay(self, SND_RAISE, 1.0f);
			}
            hammer->power=weapon->s.frame-winfoFirstFrame(ANIM_SHOOT_STD);
            entAnimate(weapon,weapon->s.frame,weapon->s.frame,FRAME_ONCE,.05);
        }
    }

    // do damage
    if ( weapon->s.frame >= HAMMER_DAMAGE_FRAME-1 )
    {
		winfoPlay(self,SND_DROP,1.0);
        CVector dir,start,end;
        trace_t tr;

        weapon->s.renderfx &= ~RF_SHAKE;

        dir=owner->client->v_angle;
        dir.AngleToVectors(forward,right,up);
        start=owner->s.origin+up*4;
        end=start+forward*50;
        tr=gstate->TraceBox_q2(start,self->s.mins,self->s.maxs,end,self,MASK_SHOT);

        // direct hit - full damage * frac
        if ( tr.ent && tr.ent->inuse && tr.ent->takedamage)     // direct hit?
		{
			hook->hits.AddHit(self->owner,tr.ent);
			// cek[1-24-00]: set the attackweapon
			winfoAttackWeap;

            e2_com_Damage(tr.ent,self->owner,self->owner,tr.endpos,dir,winfo->damage*(hammer->power/HAMMER_UPSTOP_FRAMES),DAMAGE_INERTIAL);
		}

        // earthquake? or radius damage?
        if ( (hammer->power >= HAMMER_UPSTOP_FRAMES) && self->owner->groundEntity )       // earthquake?
        {
			// do some damage to the player...make this be the 'ammo' for doing a quake
			// cek[1-24-00]: set the attackweapon
			winfoAttackWeap;
			com->Damage(owner,self,owner,zero_vector,owner->s.origin,20,DAMAGE_NONE);

			// radius damage - half damage
			// cek[1-24-00]: set the attackweapon
//			winfoAttackWeap;
//            e2_com_RadiusDamage(self,self->owner,self->owner,winfo->damage*0.5,DAMAGE_EXPLOSION, HAMMER_DAMAGE_RADIUS, &hook->hits);
			hammer_radiusDamage(self,self->owner,winfo->damage,HAMMER_DAMAGE_RADIUS,false);

            hook->origin = owner->s.origin;
            hook->killtime=gstate->time+HAMMER_QUAKE_TIME;
			hook->frozen[0]=NULL;
            self->think=doEarthquake;
            self->think(self);
				
            // show wave effect
            CVector angle(90,0,0);
            CVector pos = self->s.origin;
            vecCrackEarth = self->s.origin;
            vecCrackEarth.z -= 100;
            tr = gstate->TraceLine_q2(self->s.origin, vecCrackEarth, NULL, MASK_SOLID);
            if(tr.fraction < 1.0)
            {
                // found the ground
				Hammer_impact(self,tr,1);
            }
            for ( int i=0; i<6; i++ )
            {

                pos.z = self->absmin.z-10+rnd()*8;//-4self->s.origin.z-10+rnd()*8-4;
                winfoClientScaleImage(winfo,pos,angle,.2,60,MDL_QUAKERING,.4,HAMMER_FX_TIME+i*(HAMMER_FX_TIME*.1));
                iHitSound = rand() % 7;
                winfoPlay(self, SND_HIT1 + iHitSound, 1.0f);
                
            }

            trackLight(self,250,.98,.52,.01,TEF_LIGHT);
            owner->velocity.z += 450;
        }
        else        // just radius damage
        {
			if (self->owner && self->owner->client && self->client->v_angle)
				vecCrackEarth = self->owner->client->v_angle;
			else
				vecCrackEarth = self->owner->s.angles;
			AngleToVectors(vecCrackEarth,forward);
            vecCrackEarth = self->s.origin;
            vecCrackEarth += 100 * forward;
            tr = gstate->TraceLine_q2(self->s.origin, vecCrackEarth, NULL, MASK_SOLID);
			float frac = hammer->power/HAMMER_UPSTOP_FRAMES;
            if(tr.fraction < 1.0)
            {
                // found the ground
				Hammer_impact(self,tr,0.5*frac);
            }
            for ( int i=0; i<3; i++ )
            {
                iHitSound = rand() % 7;
                winfoPlay(self, SND_HIT1 + iHitSound, 1.0f);   
            }

			// radius damage - half damage * frac
			// cek[1-24-00]: set the attackweapon
//			winfoAttackWeap;
  //          e2_com_RadiusDamage(self,self->owner,self->owner,winfo->damage*frac,DAMAGE_EXPLOSION, HAMMER_DAMAGE_RADIUS, &hook->hits);
			hammer_radiusDamage(self,self->owner,winfo->damage*frac,HAMMER_DAMAGE_RADIUS,true);
            self->remove(self);
            return;
        }
    }

    // keep controller with player
    self->s.origin = self->owner->s.origin;
    self->s.angles = self->owner->s.angles;
    gstate->LinkEntity(self);

    self->nextthink=gstate->time+.1;
}

void hammer_radiusDamage(userEntity_t *hammer, userEntity_t *player, float damage, float radius, qboolean bDiminishing)
{
	if (!player || !hammer || !hammer->userHook)
		return;

    hammerHook_t *hook=(hammerHook_t *)hammer->userHook;

	winfoAttackWeap;
	if (bDiminishing)
	{
        e2_com_RadiusDamage(hammer,player,player,damage,DAMAGE_EXPLOSION, radius, &hook->hits);
	}
	else
	{
		userEntity_t	*head;
		CVector			org;
		for (head = gstate->FirstEntity(); head != NULL; head = gstate->NextEntity(head))
		{
			// find the distance to this entity
			if (head != player && head->takedamage)
			{
				if (head->solid == SOLID_BSP)
				{
					org = head->s.mins + head->s.maxs;
					VectorMA( head->s.origin, org, 0.5, org);
				}
				else
				{
					org = head->s.origin;
				}
				CVector distance = org-player->s.origin;

				if (distance.Length() > radius)
				{
					continue;
				}

				if (!com->Visible(player,head))
					continue;

				hook->hits.AddHit(player,head);
				e2_com_Damage (head, hammer, player, org, zero_vector, damage, DAMAGE_EXPLOSION);
			}
		}
	}
}

//---------------------------------------------------------------------------
// doEarthquake()
//---------------------------------------------------------------------------
void doEarthquake(userEntity_t *self)
{
    hammerHook_t    *hook=(hammerHook_t *)self->userHook;
    userEntity_t    *head;
    CVector         temp, vecKickView, vecDir;
    float           dist,severity,move_severity;
    
	if (!hook)
		return;

    untrackLight(self);

    // time to quit?
    if ( gstate->time >= hook->killtime )
    {
        self->remove(self);
        return;
    }

    // shake the ground and do damage
    for ( head=gstate->FirstEntity(); head; head=gstate->NextEntity(head) )
    {
        if ( 
			( !head->inuse ) 
			|| ( head == self ) 
			|| ( head->solid != SOLID_BBOX ) 
			|| ( head->s.effects & EF_GIB ) 
			|| ( head->movetype == MOVETYPE_FLY )
			|| ( !validateTarget(self,head,0,TARGET_DM,TARGET_ALWAYS_IF_ALIVE,TARGET_DM,TARGET_ALWAYS))

			)
		{
            continue;
		}

        // distance from explosion
        temp = head->s.origin - hook->origin;
        dist= temp.Length()*.7;

        if ( dist > HAMMER_QUAKE_DIST )
            continue;

        severity=(HAMMER_QUAKE_DIST-dist)*.25;
        severity *= .25 + (hook->killtime - gstate->time) / HAMMER_QUAKE_TIME;
		severity *= (0.01*winfo->damage);		// damage/100 -- scale to use CSV
        move_severity=severity;

        if ( head->flags & FL_CLIENT && head->client )
        {
            vecKickView.Set((crand() * -.5) * (severity * 0.05f), (crand() * -.5) * (severity * 0.01f), (crand() * -.5) * (severity * 0.05f));
            com->KickView(head, vecKickView, 0, 50, 100);
        }
        else
            move_severity *= 4;

/*	REPLACED WITH RADIUS DAMAGE
        // do damage
		if (head->groundEntity && (head->velocity.z < 20))
		{
			vecDir = head->s.origin - self->owner->s.origin;
			vecDir.Normalize();
			if ( head != self->owner )
			{
				hook->hits.AddHit(self->owner,head);
				// cek[1-24-00]: set the attackweapon
				winfoAttackWeap;

				e2_com_Damage(head, 
							self, 
							self->owner, 
							head->s.origin, 
							vecDir, 
							severity*.01,
							DAMAGE_NO_BLOOD|DAMAGE_INERTIAL);
			}
		}
*/
        if ( head->groundEntity && !(head->flags & FL_CINEMATIC) && (head->flags & (FL_CLIENT|FL_BOT|FL_MONSTER)))
        {
            head->groundEntity = NULL;
            head->velocity.x += (rnd() - .5) * (move_severity*1.25);
            head->velocity.y += (rnd() - .5) * (move_severity*1.25);
            head->velocity.z += (rnd() - .5) * (move_severity*1.25);
        }
    }

    self->nextthink=gstate->time+.1;
}

void Hammer_impact(userEntity_t *self, trace_t &trace,float fscale)
{
	csurface_t *surf = trace.surface;
	if (surf)
	{
		if (fscale > 0.1)
		{
			int count = 6+frand()*6;
			if (!(deathmatch->value || coop->value))
			{
				CVector scale;
				scale.Set(2+1*frand(),2+.5*frand(),1+.5*frand());
				scale.Multiply(fscale);
				AngleToVectors(self->owner->s.origin,forward,right,up);

				debrisInfo_t di;
				memset(&di,0,sizeof(di));
				if (surf->flags & (SURF_WOOD|SURF_METAL|SURF_STONE|SURF_STONE|SURF_GLASS|SURF_SNOW|SURF_ICE|SURF_SAND))
				{
					di.surf				= surf;
					di.bRandom			= 0;			
				}
				else
				{
					di.surf				= NULL;
					di.type				= DEBRIS_ROCK1;	
					di.bRandom			= 1;			
				}
				di.count			= count;
				di.org				= trace.endpos+ 5*trace.plane.normal;
				di.dir				= trace.plane.normal;
				di.scale			= scale;
				di.scale_delta		= 0.25;					
				di.speed			= 750;
				di.spread			= 170;
				di.alpha			= 1.0;
				di.alpha_decay		= 0.25;
				di.gravity			= 1.0;
				di.delay			= 5;
				di.damage			= 0;
				di.owner			= NULL;

				di.bSound			= 1;						
				di.soundSource		= self;					
				di.minAttn			= ATTN_NORM_MAX;
				di.maxAttn			= ATTN_NORM_MAX	* 3; // these are big rox!

				di.particles		= PARTICLE_SIMPLE;
				di.pColor			= CVector(0.4,0.2,0.1);
				di.pScale			= 2.0;

				spawn_surface_debris(di);
			}
		}

		if (surf->flags & SURF_WOOD)
		{
			weaponScorchMark( trace, SM_EARTH );
		}
		else if (surf->flags & SURF_METAL)
		{
		}
		else if (surf->flags & SURF_STONE)
		{
			weaponScorchMark( trace, SM_EARTH );
		}
		else
		{
			weaponScorchMark( trace, SM_EARTH );
		}
	}
}

//-----------------------------------------------------------------------------
//	weapon_hammer
//-----------------------------------------------------------------------------
void weapon_hammer(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_hammer_register_func()
{
	gstate->RegisterFunc("hammerThink",hammerThink);
	gstate->RegisterFunc("hammerdoEarthquake",doEarthquake);
	gstate->RegisterFunc("hammerUse",hammerUse);
	gstate->RegisterFunc("hammerGive",hammerGive);
	gstate->RegisterFunc("hammerSelect",hammerSelect);
}
