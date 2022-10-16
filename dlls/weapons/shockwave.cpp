//---------------------------------------------------------------------------
// SHOCKWAVE
//---------------------------------------------------------------------------
#include "weapons.h"
#include "shockwave.h"

enum
{
    // model defs
    MDL_START,
    MDL_SPHERE,
    MDL_SHOCKRING,
    MDL_SPHEREEXPLODE,
    MDL_SHOTRING,
    MDL_MUZZLEFLASH,
    MDL_SHOCKWALL,

    // sound defs
    SND_START,
    SND_EXPLODE,
//   SND_GLOBEEXPAND,
//   SND_RING,

    // frame defs
    ANIM_START,
    ANIM_SPHERE,
    ANIM_SPHEREEXPLODE,
    ANIM_SHOTRING
};

// weapon info def
weaponInfo_t shockwaveWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_SHOCKWAVE),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_SHOCKWAVE_AMMO),
	// names, net names, icon
	"weapon_shockwave",
	tongue_weapons[T_WEAPON_SHOCKWAVE],
	"ammo_shocksphere",
	tongue_weapons[T_WEAPON_SHOCKWAVE_AMMO],
	"iconname",

	// model files
	{
		"models/e1/w_shockwave.dkm",
		"models/e1/a_shokwv.dkm",
		"models/e1/wa_shokwv.dkm",
		// "models/e1/we_shockorb.sp2",
		"models/e1/we_3dshock.dkm",
		"models/e1/we_shockring.sp2",
		"models/e1/we_shockexp.sp2",
		"models/global/we_shotring.sp2",
		"models/e1/we_mfswave.sp2",
		"models/e1/we_shockwall.sp2",
		NULL
	},

	// sound files
	{
		"e1/we_shockwaveamba.wav",
		"e1/we_shockwaveready.wav",
		"e1/we_shockwaveaway.wav",
		"e1/we_shockwaveshoota.wav",
		SND_NOSOUND,
		SND_NOSOUND,
		SND_NOSOUND,
		"e1/we_shockwaveexp.wav",
		//       "e1/we_globeexpand4.wav",
		//       "e1/we_ringsound.wav",
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
		ANIM_NONE,
		MDL_SPHEREEXPLODE,"sp2",0,0,
		MDL_SHOTRING,"sp2",0,0,
		MDL_SHOCKWALL, "sp2", 0, 0,
		0,
		NULL
	},

	// commands
	{ 
		wpcmds[WPCMDS_SHOCKWAVE][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_6,
		wpcmds[WPCMDS_SHOCKWAVE][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_6,
		NULL
	},

	weapon_give_6,
	weapon_select_6,
	shockwaveCommand,
	shockwaveUse,
	winfoSetSpawn,
	winfoAmmoSetSpawn,
	sizeof(weapon_t),

	600,
	10,                        // ammo_max
	1,                         // ammo_per_use
	1,                         // ammo_display_divisor
	4,                          //	initial ammo
	700.0,                     // speed
	2000.0,                    // range
	SWAVE_MIN_DAMAGE,          // damage
	3.0,                       // lifetime

	WFL_FORCE_SWITCH|WFL_PLAYER_ONLY,          //WFL_LOOPAMBIENT_SND,       // flags

	WEAPONTYPE_SHOULDER,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon1[T_DEATHMSGWEAPON_SHOCKWAVE],
		NULL,
		ATTACKER_VICTIM
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon1,T_DEATHMSGWEAPON_SHOCKWAVE),
	ATTACKER_VICTIM,


	WEAPON_SHOCKWAVE,
	0,
	NULL
};

// local weapon info access
static weaponInfo_t *winfo=&shockwaveWeaponInfo;

//---------------------------------------------------------------------------
// weapon_give()
//---------------------------------------------------------------------------
userInventory_t *shockwaveGive (userEntity_t *self, int ammoCount)
{
    return(weaponGive(self,winfo,ammoCount));
}

//---------------------------------------------------------------------------
// weapon_select()
//---------------------------------------------------------------------------
short shockwaveSelect(userEntity_t *self)
{
    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// weapon_command()
//---------------------------------------------------------------------------
void *shockwaveCommand(struct userInventory_s *ptr, char *commandStr, void *data)
{
    winfoGenericCommand(winfo,ptr,commandStr,data);

    return(NULL);
}

//---------------------------------------------------------------------------
// weapon_use()
//---------------------------------------------------------------------------
void shockwaveUse(userInventory_s *ptr, userEntity_t *self)
{
    CVector vec;
    playerHook_t *phook=(playerHook_t *)self->userHook;

    // any ammo?
    if ( !weaponHasAmmo(self,true) )
        return;

    weaponUseAmmo (self, true);

    winfoPlay(self,SND_SHOOT_STD,1.0f);
    winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,.05);
    
    phook->fxFrameNum=winfoFirstFrame(ANIM_SHOOT_STD)+32;
    phook->fxFrameFunc=shockwaveShootFunc;

    phook->fired++;

}

void shockorbThink(userEntity_t *self)
{
    CVector         vecColor(.25,.25,1), vecShotRingDistance, vecWaterVelocity, vecShotRingForward;
    float           fShotRingDistance;
    shockwaveHook_t *hook=(shockwaveHook_t *)self->userHook;
	
    if ( gstate->PointContents(self->s.origin) & MASK_WATER )
	{
		if(self->velocity.Length() > 100)
		{
			vecWaterVelocity = self->velocity;
			vecWaterVelocity.Normalize();
			vecWaterVelocity = vecWaterVelocity * 100;
			self->velocity = vecWaterVelocity;
		}
		if(hook)
		{
			hook->sBounceCount=9999;
		}
	}
    if(self->velocity.Length() < 1.0f)
    {
        // kaboom!
        winfoPlayAttn(self,SND_EXPLODE,1.0f,2048,4096);		// should be able to hear this forever...
        shockwaveExplode(self,1);
        self->remove (self);
    } else {
        self->nextthink=gstate->time+0.1f;
        if(hook)
        {
            
            //weapon_face_velocity(self);
            vecShotRingDistance = self->s.origin - hook->vecLastShotRingOrigin;
            fShotRingDistance = vecShotRingDistance.Length();
            if(fShotRingDistance > 75 && hook->iShotRingCount <= 5)
            {
                hook->vecLastShotRingOrigin = self->s.origin;
                hook->iShotRingCount++;
                vecShotRingForward = self->velocity;
                vecShotRingForward.VectorToAngles(vecShotRingForward);

                winfoClientScaleAlpha(winfo,self->s.origin,vecShotRingForward,.5,MDL_SHOTRING, ANIM_SHOTRING,.96,.01,200,vecColor);     
            } else {
                self->velocity.z-=50;
            }
        }
    }
    return;
}
//---------------------------------------------------------------------------
// shoot_func()
//---------------------------------------------------------------------------
void shockwaveShootFunc(userEntity_t *self)
{
    weapon_t *invWeapon=(weapon_t *)self->curWeapon;
    userEntity_t *shot/*,*weapon=(userEntity_t *)self->client->ps.weapon*/;// SCG[1/24/00]: not used
    CVector vec, vecKickAngles(2,0,0);
    playerHook_t *phook=(playerHook_t *)self->userHook;
	if (!phook)
		return;
    
    float zvel,size,scale;

    phook->fxFrameFunc=NULL;

    // adjust z velocity (based on shot angle)
    vec=self->client->v_angle;
    vec.AngleToVectors(forward,right,up);
    zvel=(1.0-fabs(forward.z)) * winfo->speed;
    if ( forward.z < 0 )
        zvel = -zvel;

    // spawn projectile
    CVector offset( winfo->fWeaponOffsetX1, winfo->fWeaponOffsetY1, winfo->fWeaponOffsetZ1 );
    
    shot=weapon_spawn_projectile(self,offset, winfo->speed, 0,"projectile_shockwave", FALSE);

    // set shot attributes
    shot->movetype=MOVETYPE_BOUNCEMISSILE;  // Logic[4/8/99]: MOVETYPE_BOUNCE just doesn't give us that ominous bouncing orb effect
    shot->s.effects=0;
    shot->s.renderfx=0;     //RF_TRANSLUCENT;
    // set func ptrs
    shot->touch=(touch_t)shockwaveTouch;
    shot->think = shockorbThink;
    shot->nextthink=gstate->time+.1;
    // more attrs
    size=15;     //SHOCKWAVE_SIZE_START;
    // scale=SHOCKWAVE_SCALE_START;
    scale = 15.0;
    shot->userHook = gstate->X_Malloc(sizeof(shockwaveHook_t),MEM_TAG_HOOK);
    shot->avelocity.Set(45, 90, 180);
    winfoSetModel(shot,MDL_SPHERE);
    gstate->SetSize(shot,-size,-size,-size,size,size,size);        // final s/b 24
    //gstate->SetOrigin(shot,shot->s.origin[0],shot->s.origin[1],shot->s.origin[2]);
    shot->s.render_scale.Set(scale,scale,scale);

//    if ( weapon )
//        trackSetAnim(weapon,gstate->ModelIndex(winfoModel(MDL_MUZZLEFLASH)),1,.285,TEF_ANIMATE,.05);

/*	weaponMuzzleFlash(	self,self,winfo,					// thing tracked, player, weapon info
						"models/e1/we_mfswave.sp2",		// flash model (modelindex)
						1,0.05,								// model frame count, frame time
						0.285,								// scale
						150 + 50*frand(),					// light value (may be 0)
						CVector(1,1,1),				// light color (ignored if no lightsize)
						TEF_ANIM2_THIRD|TEF_ANIM2_FIRST,					// any of TEF_AMIN2_* animation flags
						0);						// additional fxflags (TEF_NOROTATE)
*/
	weaponMuzzleFlash(	self,self,winfo,FLASH_SHOCKWAVE);

    invWeapon->lastFired=gstate->time;
    // kick the player view
    weapon_kick(self, vecKickAngles, -200, 200, 200);
}



//---------------------------------------------------------------------------
// shockwave_touch()
//---------------------------------------------------------------------------
void shockwaveTouch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf)
{
    shockwaveHook_t *hook=(shockwaveHook_t *)self->userHook;
    CVector ang,pos,normal, vecPush, vecOrigin, vecVel, vecExtra, vecKick;
    userEntity_t *head;
    CVector temp, vecAngle;
    float mindist=99999,dist, perc;

    if(other->takedamage != DAMAGE_NO)
    {
        // com->Damage(head,inflictor,attacker,head->s.origin,pushdir,damage_points,DAMAGE_EXPLOSION|DAMAGE_DIR_TO_INFLICTOR)
        vecPush=other->s.origin-self->s.origin;
        vecPush.Normalize();
		// cek[1-24-00]: set the attackweapon
		winfoAttackWeap;

        com->Damage(other, self, self->owner, other->s.origin, vecPush, winfo->damage * 3, DAMAGE_EXPLOSION|DAMAGE_DIR_TO_INFLICTOR);
		hook->hits.AddHit(self->owner,other);
        winfoPlayAttn(self,SND_EXPLODE,1.0f,2048,4096);
        shockwaveExplode(self,1);

        self->remove (self);
		return;
    }
    if(hook)
    {
        hook->sBounceCount++;
        hook->iShotRingCount = 9999;
        // shockwave_explode(self,hook->sBounceCount);
//        shockwave_explode(self,1);

		// cek[1-24-00]: set the attackweapon
		winfoAttackWeap;

		counted_com_RadiusDamage(self,self->owner,NULL,&hook->hits,0.75*winfo->damage,0,300);
		hook->hits.AddHit(self->owner,other);

        if(plane)
        {
            VectorToAngles(plane->normal, vecAngle);
            gstate->WriteByte(SVC_TEMP_ENTITY);
            gstate->WriteByte(TE_SHOCKORB_BOUNCE);
            gstate->WritePosition(self->s.origin);
            gstate->WritePosition(vecAngle);
            gstate->WriteShort(gstate->ModelIndex(winfoModel(MDL_SHOCKWALL)));
            gstate->MultiCast(self->s.origin,MULTICAST_PVS);

            // Logic[4/12/99]: Hideous... definately need a better way to do this!
			// And for the blast, knock people around (*HARD*)
            head = gstate->FirstEntity ();
            while ( head )
            {
                // find the distance to this entity
                if ( head != self && head->takedamage && (head->flags & (FL_CLIENT|FL_MONSTER|FL_BOT)) && !(head->flags & FL_CINEMATIC))
                {
                    vecOrigin = head->s.origin + (head->s.mins + head->s.maxs) * 0.5;
                    dist = vecOrigin.Distance(self->s.origin);
                    
                    vecKick.x = ((rnd() - 0.5) * 15);
                    vecKick.y = ((rnd() - 0.5) * 15);
                    vecKick.z = ((rnd() - 0.5) * 5);
                    com->KickView(head, vecKick, 0, 50, 100);

                    if ( (dist<=1000) && head->groundEntity )
                    {
                        perc=(1000-dist)*0.001;
                        vecPush=head->s.origin-self->s.origin;
                        vecPush.Normalize();
                        if ( vecPush.z < .4 && vecPush.z > -0.1 )
                            vecPush.z = .4;

                        vecVel=head->velocity;
                        vecExtra=vecPush*2000*perc;
                        vecVel=vecVel+vecExtra;
						vecVel.Multiply(0.30);		// cek[1-11-00] tone down a bit.
                        head->velocity = vecVel;
                        head->groundEntity = NULL;
                    }
                }

                head = gstate->NextEntity (head);
            }

        }
        if(hook->sBounceCount > 5)
        {
            if ( plane )
            {
                for ( head=gstate->FirstEntity(); head; head=gstate->NextEntity(head) )
                {
                    if ( !head->inuse || (head->flags & FL_CINEMATIC) || !(head->flags & (FL_CLIENT|FL_MONSTER|FL_BOT)))
                        continue;

                    // distance from explosion
                    temp = head->s.origin - self->s.origin;
                    dist=temp.Length()*.7;
                    if ( dist > SHOCKRING_MAX_DIST )
                        continue;

                    if ( head->groundEntity )
                    {
                        head->groundEntity = NULL;
                        head->velocity.x += (rnd() - 0.5) * 200;//hook->severity;
                        head->velocity.y += (rnd() - 0.5) * 200;//hook->severity;
                        //	head->velocity [2] = /*hook->severity*/ 200 * (1.0 / head->mass);
                    }

                    if ( head->flags & FL_CLIENT && head->client )
                    {
                        float severity=(SHOCKRING_MAX_DIST-dist)*.2;

                        // as wave dies down, it shakes less
                        if ( mindist >= SHOCKRING_MAX_DIST-150 )
                            severity *= (SHOCKRING_MAX_DIST-mindist)/150;

                        vecKick.x = ((rnd() - 0.5) * (severity * 0.02));
                        vecKick.y = ((rnd() - 0.5) * (severity * 0.01));
                        vecKick.z = ((rnd() - 0.5) * (severity * 0.1));
                        com->KickView(head, vecKick, 0, 50, 100);
                        /*
                        head->client->kick_angles.x += ((rnd() - 0.5) * (severity / 50));
                        head->client->kick_angles.y += ((rnd() - 0.5) * (severity / 100));
                        head->client->kick_angles.z += ((rnd() - 0.5) * (severity / 10));
                        */
                    }
                }
                
                
                self->s.origin.x += plane->normal.x*40;
                self->s.origin.y += plane->normal.y*40;
                self->s.origin.z += plane->normal.z*40;
                self->velocity = self->velocity * 1.5;
            }

            winfoPlayAttn(self,SND_EXPLODE,1.0f,2048,4096);
            shockwaveExplode(self,1);

            self->remove (self);
        }
    }
    return;
}

//---------------------------------------------------------------------------
// shockringThink()
//---------------------------------------------------------------------------
void shockringThink(userEntity_t *self)
{
    playerHook_t *phook=(playerHook_t *)self->owner->userHook;
    shockwaveHook_t *hook=(shockwaveHook_t *)self->userHook;
	if (!hook || !phook)
		return;

    userEntity_t *head;
    shockRing_t *ring;
    CVector temp;
    float perc,mindist=99999,maxdist=-1,dist;
    short loop;

    // deal with all shockrings
    for ( loop=0; loop<hook->ringnum; loop++ )
    {
        ring=&hook->ring[loop];
		// SCG[1/24/00]: Check yer pointers
		if( ring == NULL )
		{
			continue;
		}
        if(ring->max < SHOCKRING_MAX_DIST)
            shockwaveDamage(self,self->owner,self,winfo->damage,0.0,0,ring->min,ring->max,0.0);

        if ( ring->min < mindist )
            mindist=ring->min;
        if ( ring->max > maxdist )
            maxdist=ring->max;

        ring->min=ring->max-20;
        perc=(gstate->time-ring->starttime)/SHOCKRING_TIME;
        ring->max=perc*SHOCKRING_MAX_DIST;

        // track light at first shockring
        if ( !loop )
        {
            perc=1.0-(ring->max/SHOCKRING_MAX_DIST);
            if ( perc > 0 )
            {
                perc *= .5;
                trackLight(self,500,.2*perc,.2*perc,1*perc,TEF_LIGHT);
            }
        }
    }

    // shake the ground a little???
    for ( head=gstate->FirstEntity(); head; head=gstate->NextEntity(head) )
    {
        if ( !head->inuse || (head->flags & FL_CINEMATIC) || !(head->flags & (FL_CLIENT|FL_MONSTER|FL_BOT)))
            continue;

        // distance from explosion
        temp = head->s.origin - self->s.origin;
        dist=temp.Length()*.7;

        if ( dist > SHOCKRING_MAX_DIST )
            continue;

        if ( head->groundEntity )
        {
            head->groundEntity = NULL;
            head->velocity.x += (rnd() - 0.5) * 200;//hook->severity;
            head->velocity.y += (rnd() - 0.5) * 200;//hook->severity;
            //	head->velocity [2] = /*hook->severity*/ 200 * (1.0 / head->mass);
        }

        if ( head->flags & FL_CLIENT && head->client )
        {
            float severity=(SHOCKRING_MAX_DIST-dist)*.2;

            // as wave dies down, it shakes less
            if ( mindist >= SHOCKRING_MAX_DIST-150 )
                severity *= (SHOCKRING_MAX_DIST-mindist)/150;

            head->client->kick_angles.x += ((rnd() - 0.5) * (severity / 50));
            head->client->kick_angles.y += ((rnd() - 0.5) * (severity / 100));
            head->client->kick_angles.z += ((rnd() - 0.5) * (severity / 10));
        }
    }

    // time for next shockring?
    if ( gstate->time >= hook->spawntime )
        shockwaveExplode(self,hook->ringnum+1);

    // kill explosion controller?
    if ( ring->max >= SHOCKRING_MAX_DIST )
    {
        untrackLight(self);
        self->remove (self);
        return;
    }

    self->nextthink=gstate->time+.05;
}

//---------------------------------------------------------------------------
// shockwave_explode()
//---------------------------------------------------------------------------
void shockwaveExplode(userEntity_t *self,short explosionNum)
{
    userEntity_t *ent;
    shockwaveHook_t *hook;
    shockRing_t *ring;
    CVector vtemp, vecColor(0.25f, 0.25f, 1.0f);

    vtemp=CVector(-1,-1,-1);

    winfoClientExplosion(winfo,self->s.origin,vtemp,1.5,MDL_SPHEREEXPLODE,ANIM_SPHEREEXPLODE,CLE_FADEALPHA,0,vecColor);
//    winfoClientScaleRotate(winfo,self->s.origin,1,24,SHOCKRING_TIME,MDL_SHOCKRING);
    winfoClientScaleRotate(winfo,self->s.origin,1,14,SHOCKRING_TIME,MDL_SHOCKRING);

    // spawn single entity that keeps track of explosion
    if ( explosionNum==1 )
    {
        ent=gstate->SpawnEntity();
        ent->className="shockwave_center";
		ent->flags |= FL_NOSAVE;
        ent->think=shockringThink;
        ent->nextthink=gstate->time+.05;
        ent->s.renderfx=RF_NODRAW|RF_TRACKENT;
        ent->owner=self->owner;
        ent->s.origin = self->s.origin;
        gstate->LinkEntity(ent);

        //ent->userHook = new shockwaveHook_t;
		ent->userHook = gstate->X_Malloc(sizeof(shockwaveHook_t),MEM_TAG_HOOK);
        hook=(shockwaveHook_t *)ent->userHook;

        flashClients(self,.8,.8,.8,.3);
    }
    else
        hook=(shockwaveHook_t *)self->userHook;

	if (!hook)
		return;

    hook->ringnum=explosionNum;
    ring=&hook->ring[explosionNum-1];
    ring->min=0;
    ring->max=15;
    ring->starttime=gstate->time;

    if ( hook->ringnum < SHOCKWAVE_NUM_WAVES )
        hook->spawntime=gstate->time+.5;
    else
        hook->spawntime=gstate->time+99999;

}

//---------------------------------------------------------------------------
// shockwave_damage
//
// - entities are only damaged if they are within curmin & curmax
// - damage is based on distance from explosion
//---------------------------------------------------------------------------
int shockwaveDamage(userEntity_t *inflictor, userEntity_t *attacker, userEntity_t *ignore, float maxdamage,float maxdist, int damage_type, float curmin, float curmax, float push_extra)
{
    userEntity_t    *head;
    float           damage_points,dist,perc;
    CVector         org,pushdir,vel,extra;
    int             hit_something;

    maxdist=1000;
    push_extra=1500;

    if ( curmin < 0 )
        curmin=0;

    hit_something = FALSE;
    head = gstate->FirstEntity ();

    while ( head )
    {
        // find the distance to this entity
        if ( (head != ignore) && (head->takedamage != DAMAGE_NO))
        {
            org = head->s.origin + (head->s.mins + head->s.maxs) * 0.5;
            dist = org.Distance(inflictor->s.origin);

            if ( dist>=curmin && dist<=curmax )
            {
                perc=(maxdist-dist)/maxdist;
                damage_points=perc*maxdamage;

                if ( damage_points < 0 )
                    damage_points = 0;

                // half damage from own weapon
                if ( head==attacker )
                    damage_points=damage_points*.5;

                hit_something=TRUE;

                pushdir=head->s.origin-inflictor->s.origin;
                pushdir.Normalize();
                if ( pushdir.z < .4 && pushdir.z > -0.1 )
                    pushdir.z = .4;

                // determine damage
                if ( gstate->inPVS(head->s.origin,inflictor->s.origin) )
                {
                    if ( !com->CanDamage(head,inflictor) )
                        damage_points *= .9;
                }
                else if ( gstate->inPVS(head->s.origin,inflictor->s.origin) )
                    damage_points *= .5;
                else
                    damage_points *= .05;

/*
            if (!com->CanDamage(head,inflictor))
            {
               damage_points *= .025;
               push_extra *= .025;
            }
*/

				// cek[1-24-00]: set the attackweapon
				winfoAttackWeap;

                com->Damage(head,inflictor,attacker,head->s.origin,pushdir,damage_points,DAMAGE_EXPLOSION|DAMAGE_DIR_TO_INFLICTOR);
				shockwaveHook_t *hook=(shockwaveHook_t *)inflictor->userHook;
				if (hook)
					hook->hits.AddHit(attacker,head);

				if (!(head->flags & FL_CINEMATIC) && (head->flags & (FL_CLIENT|FL_MONSTER|FL_BOT)))
				{
					vel=head->velocity;
					extra=pushdir*push_extra;
					extra=extra*perc;
					vel=vel+extra;
					head->velocity = vel;

					head->groundEntity = NULL;
				}
            }
        }

        head = gstate->NextEntity (head);
    }

    return	hit_something;
}

//-----------------------------------------------------------------------------
//	weapon_shockwave
//-----------------------------------------------------------------------------
void weapon_shockwave(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

//-----------------------------------------------------------------------------
//	ammo_shocksphere
//-----------------------------------------------------------------------------
void ammo_shocksphere (userEntity_t *self)
{
    winfoAmmoSetSpawn (winfo, self, 1, 30.0, NULL);
	ammo_set_sound(self, gstate->SoundIndex("global/i_swaveammo.wav"));
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_shockwave_register_func()
{
	gstate->RegisterFunc("shockorbThink",shockorbThink);
	gstate->RegisterFunc("shockringThink",shockringThink);
	gstate->RegisterFunc("shockwaveTouch",shockwaveTouch);
	gstate->RegisterFunc("shockwaveUse",shockwaveUse);
	gstate->RegisterFunc("shockwaveGive",shockwaveGive);
	gstate->RegisterFunc("shockwaveSelect",shockwaveSelect);
}
