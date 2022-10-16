//---------------------------------------------------------------------------
// BOLTER
//---------------------------------------------------------------------------
#include "weapons.h"
#include "bolter.h"

enum
{
    // model defs
    MDL_START,
    MDL_BOLT,
    MDL_BOLTEXP,

    // sound defs
    SND_START,
    SND_HIT,
    SND_HITMETAL,
    SND_HITSTONE,
    SND_HITWOOD,

    // frame defs
    ANIM_START,
    ANIM_RELOAD,
    ANIM_BOLTEXP
};

// weapon info def
weaponInfo_t bolterWeaponInfo = 
{
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_BOLTER),
	RESOURCE_ID_FOR_STRING(tongue_weapons,T_WEAPON_BOLTER_AMMO),
    // names, net names, icon
    "weapon_bolter",
    tongue_weapons[T_WEAPON_BOLTER],
    "ammo_bolts",
    tongue_weapons[T_WEAPON_BOLTER_AMMO],
    "iconname",

    // model files
    {"models/e3/w_bolter.dkm",
        "models/e3/a_bolter.dkm",
        "models/e3/wa_bolt.dkm",
        "models/e3/we_bolt.dkm",
        "models/e3/we_boltexp.dkm",
        NULL},

    // sound files
    {SND_NOSOUND,
        "e3/we_bolterready.wav",
        "e3/we_bolteraway.wav",
        "e3/we_bolterfire.wav",
        "e3/we_bolteramba.wav",
        SND_NOSOUND,
        SND_NOSOUND,
        "e3/we_bolterhit.wav",
        "e3/we_bolterhitmetal.wav",
        "e3/we_bolterhitstone.wav",
        "e3/we_bolterhitwood.wav",
        NULL},

    // frame names
    {MDL_WEAPON_STD,"ready",0,0,
        -1,"away",0,0,
        -1,"shoot",0,0,
        -1,"amba",0,0,
        -1,"ambb",0,0,
        ANIM_NONE,
        -1,"amba",0,0,
        MDL_BOLTEXP,"explode",0,0,
        0,NULL},

    // commands
    { 
      wpcmds[WPCMDS_BOLTER][WPCMDS_GIVE], (void (*)(struct edict_s *))weapon_give_2,
      wpcmds[WPCMDS_BOLTER][WPCMDS_SELC], (void (*)(struct edict_s *))weapon_select_2,
      NULL
    },

    weapon_give_2,
    weapon_select_2,
    bolter_command,
    bolter_use,
    winfoSetSpawn,
    winfoAmmoSetSpawn,
    sizeof(bolterWeapon_t),

    200,
    100,      //	ammo_max
    1,        //	ammo_per_use
    1,        //	ammo_display_divisor
    30,       //	initial ammo
    800.0,    //	speed
    2400.0,   //	range
    30.0,     //	damage
    10.0,      //	lifetime

    WFL_FORCE_SWITCH|WFL_CROSSHAIR,        //	flags

	WEAPONTYPE_RIFLE,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,

/*	{
		tongue_deathmsg_weapon3[T_DEATHMSGWEAPON_BOLTER],
		NULL,
		VICTIM_ATTACKER
	},*/

	RESOURCE_ID_FOR_STRING(tongue_deathmsg_weapon3,T_DEATHMSGWEAPON_BOLTER),
	VICTIM_ATTACKER,

    WEAPON_BOLTER,
    0,
    NULL
};

// local weapon info access
static weaponInfo_t *winfo=&bolterWeaponInfo;

listHeader_t    *bolt_list;

//---------------------------------------------------------------------------
// bolter_give()
//---------------------------------------------------------------------------
userInventory_t *bolter_give (userEntity_t *self, int ammoCount)
{
    bolterWeapon_t *bolter;

    if ( !bolt_list )
        bolt_list=com->list_init(bolt_list);

    bolter=(bolterWeapon_t *)weaponGive(self,winfo,ammoCount);

	if ( bolter )
	{
	    bolter->reload_count = 2;
	}

    return((userInventory_t *)bolter);
}

//---------------------------------------------------------------------------
// bolter_select()
//---------------------------------------------------------------------------
short bolter_select (userEntity_t *self)
{
    return(weaponSelect(self,winfo));
}

//---------------------------------------------------------------------------
// bolter_command()
//---------------------------------------------------------------------------
void *bolter_command (struct userInventory_s *ptr, char *commandStr, void *data)
{
    userEntity_t    *self=(userEntity_t *)data;
//    playerHook_t    *phook=(playerHook_t *)self->userHook;// SCG[1/24/00]: not used

    char *buf;

    if ( !stricmp (commandStr, "bolter") )
    {
        //Return the value of bolter so we know we have it readied
        buf = (char*) data;
        sprintf(buf,"bolter");
    }
/*
   else if (!stricmp(commandStr,"ambient"))
   {
      if (weaponHasAmmo(self,false))
        {
         short animlist[]={ANIM_AMBIENT_STD,ANIM_AMBIENT2};
         short animnum;

         animnum=animlist[(int)(rnd()*1.9)];

         winfoAnimate(winfo,self,animnum,FRAME_ONCE|FRAME_WEAPON|FRAME_NODELAY,.05);
         phook->ambient_time=gstate->time+winfoNumFrames(animnum)*.05+2.0+(rnd()*6);

         winfoPlay(self,SND_AMBIENT_STD,0.1f);
      }
   }
*/
    else
        winfoGenericCommand(winfo,ptr,commandStr,data);

    return(NULL);
}

//---------------------------------------------------------------------------
//	bolt_remove ()
//
//	called when a bolt is removed if bolt's ent->remove is set to this.  
//	Removes it from all lists.
//	Free's its userHook memory
//	Does not actually remove the entity, since this function can be, and 
//	usually is, called from RemoveEntity in p_utils.c
//---------------------------------------------------------------------------

void    bolt_remove (userEntity_t *self)
{
    boltHook_t      *hook = (boltHook_t *) self->userHook;

	if (hook)
	{
		com->list_remove_node (self, bolt_list);

		//	free this bolt's friction hook
		com->friction_remove (hook->fhook);
	}

    //	userHook gets freed in RemoveEntity
    gstate->RemoveEntity (self);
}

//---------------------------------------------------------------------------
// bolt_remove_oldest()
//---------------------------------------------------------------------------
void    bolt_remove_oldest (userEntity_t *self)
{
    // point to the first entity in the bolt_list
    self = (userEntity_t *) bolt_list->head->ptr;

    //	Nelno:	this will free all of the bolt's hooks, etc.
    self->remove (self);
}

//---------------------------------------------------------------------------
// bolt_think()
//---------------------------------------------------------------------------
void    bolt_think (userEntity_t *self)
{
    boltHook_t      *hook = (boltHook_t *) self->userHook;
	if (!hook)
		return;

    // are we sticking to something?
    if ( self->flags & FL_HCHILD )
    {
        // remove the bolts if the entity their stuck to 
        // is dead or dying

        // FIXME:what will this do if sticky is stuck to another sticky???
        if ( self->owner->deadflag != DEAD_NO )
        {
            self->think = self->remove; //	Nelno:	this will now remove itself from lists, etc.
            self->nextthink = gstate->time + 0.05;
            return;
        }
    }

    if ( hook->remove_time <= gstate->time )
    {
        self->think = bolt_remove;
        self->nextthink = gstate->time + 0.05;
        return;
    }

    self->think = bolt_think;
    self->nextthink = gstate->time + 0.05;
}

void boltFade(userEntity_t *self)
{
    boltHook_t  *hook = (boltHook_t *) self->userHook;

	if( gstate->time >= hook->remove_time )
	{
		if( (self->s.alpha -= 0.05) < 0.1)
		{
			self->remove(self);
		}
//		else
//		{
//			self->nextthink = gstate->time + 0.2;
//			gstate->LinkEntity(self);    
//		}
	}

	self->nextthink = gstate->time + 0.2;
    
}

//---------------------------------------------------------------------------
// bolt_touch()
//---------------------------------------------------------------------------
void    bolt_touch (userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf)
{
//    boltHook_t  *hook = (boltHook_t *) self->userHook;// SCG[1/24/00]: not used

    if ( self->owner == other )
        return;

    if (surf && (surf->flags & SURF_SKY))		
    {
        self->think = bolt_remove;
        self->nextthink = gstate->time + 0.05;
        return;
    }

    self->s.renderfx -= (self->s.renderfx & RF_FULLBRIGHT);

    //Can we damage the object we hit?
    if (other->takedamage != DAMAGE_NO )
    {
        // make sure we pass the bolt's OWNER as the attacker!
        //	pass zero_vector as dir because DAMAGE_INFLICTOR_VEL will override it
		// cek[1-24-00]: set the attackweapon
		winfoAttackWeap;

		WEAPON_HIT(self->owner,1);
        com->Damage (other, self, self->owner, self->s.origin, zero_vector, winfo->damage, DAMAGE_INERTIAL | DAMAGE_INFLICTOR_VEL);
        self->remove(self);       
    }
    else
    {
		if ((other && other->s.solid == 31) || (other->takedamage != DAMAGE_NO) || IS_MONSTER_TYPE(other,TYPE_NHARRE))// cek[11-29-99] 31 means this is a bmodel..don't stick [12-7-99] but still hurt it.  :)
		{
			self->nextthink = gstate->time + 0.1;
			self->think = bolt_remove;
			self->touch = NULL;
//			self->remove(self);
			return;
		}
		CVector normal,angles;
        CVector color(.9,.3,.2);

        if ( plane )
            normal=plane->normal;
        else
        {
            normal=self->velocity;
            normal.Normalize();
            normal=normal * -1;
        }


		if (surf)
		{
			if (surf->flags & SURF_WOOD)
			{
				winfoPlay(self,SND_HITWOOD,1.0f);
			}
			else if (surf->flags & SURF_METAL)
			{
				// play the metal sound and do some sparks
				winfoPlay(self,SND_HITMETAL,1.0f);
				gstate->WriteByte(SVC_TEMP_ENTITY);
				gstate->WriteByte(TE_SPARKS);
				gstate->WriteByte(5);
				gstate->WritePosition(self->s.origin);
				gstate->WritePosition(normal);
				gstate->WritePosition(CVector(0.8f, 0.9f, 1.0f));
				gstate->WriteByte(0);
				gstate->MultiCast(self->s.origin, MULTICAST_PVS);
			}
			else if (surf->flags & SURF_STONE)
			{
				// play the stone sound and make some debris
				winfoPlay(self,SND_HITSTONE,1.0f);
			}
			else
			{
				winfoPlay(self,SND_HIT,1.0f);
			}
		}
		else
		{
			winfoPlay(self,SND_HIT,1.0f);
		}

        self->touch = NULL;
        self->nextthink = gstate->time + 0.5;
        self->think = boltFade;
        self->velocity.Zero();
        self->s.effects &= ~EF_BOLTTRAIL;
        // self->remove (self);
        self->s.renderfx = RF_TRANSLUCENT;
		self->solid = SOLID_NOT;
		self->svflags |= SVF_DEADMONSTER;
		gstate->LinkEntity(self);
    }

}

//---------------------------------------------------------------------------
// bolt_flight_think()
//---------------------------------------------------------------------------
void    bolt_flight_think (userEntity_t *self)
{
    boltHook_t  *hook = (boltHook_t *) self->userHook;
	if (!hook)
		return;

    if ( gstate->time >= hook->remove_time )
    {
        self->think = bolt_remove;
        self->nextthink = gstate->time + 0.05;

        return;
    }

    com->friction_apply (self, hook->fhook);

    self->think = bolt_flight_think;
    self->nextthink = gstate->time + 0.05;
}

//---------------------------------------------------------------------------
// bolter_use()
//---------------------------------------------------------------------------
void    bolter_use (struct userInventory_s *ptr, userEntity_t *self)
{
//    playerHook_t        *phook = (playerHook_t *) self->userHook;// SCG[1/24/00]: not used
    boltHook_t          *hook;
    bolterWeapon_t  *bolter = (bolterWeapon_t *) self->curWeapon;
    CVector             org;
    userEntity_t        *bolt;
//    ammo_t              *ammo = bolter->weapon.ammo;// SCG[1/24/00]: not used
    int                 ammo_count;

    bolter->reload_count--;

    // start the firing frame sequence
    if ( bolter->reload_count == 1 )
    {
        // do we have any ammo left?
        if ( !weaponHasAmmo (self, true) )
            return;

        winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,.05);
        //phook->attack_finished += .5;       // wait additional .5 sec
    }
    else if ( bolter->reload_count == 0 )
    {
        // do we have any ammo left?
        if ( !weaponHasAmmo(self,true) )
            return;

        winfoAnimate(winfo,self,ANIM_SHOOT_STD,FRAME_ONCE|FRAME_WEAPON,.05);
        //phook->attack_finished += .5;       // wait additional .5 sec
    }
    else
    {
        // find out how much ammo we have
        ammo_count=weaponUseAmmo(self,true);

        if ( ammo_count >= 2 )
            bolter->reload_count = 2;
        else
            bolter->reload_count = ammo_count;

//		// only reload if we have some ammo
//		if (ammo_count > 0)
//		{
//         winfoAnimate(winfo,self,ANIM_RELOAD,FRAME_ONCE|FRAME_WEAPON,.05);
//		}

//      winfoPlay(self,SND_LOAD,1.0f);

        return;
    }

    winfoPlay(self,SND_SHOOT_STD,1.0f);

    /////////////////////////////////////////////////////////////////
    // create projectile
    /////////////////////////////////////////////////////////////////

    // spawn a new bolter bolt
    CVector offset( winfo->fWeaponOffsetX1, winfo->fWeaponOffsetY1, winfo->fWeaponOffsetZ1 );
    bolt=weapon_spawn_projectile(self,offset,winfo->speed,0,"projectile_bolter");

    // set up the userHook for the bolter bolt
//    bolt->userHook = new boltHook_t;
	bolt->userHook = gstate->X_Malloc(sizeof(boltHook_t),MEM_TAG_HOOK);
    hook = (boltHook_t *) bolt->userHook;

    // set up frictionHook
    hook->fhook = com->friction_init (self, hook->fhook);
    com->friction_set_physics (self, hook->fhook, 1.0, 1.0, 1000.0, 0);

    // set up touch and think functions for the bolt
    bolt->remove = (remove_t)bolt_remove;
    bolt->owner=self;
    bolt->s.effects=EF_BOLTTRAIL;
    bolt->s.render_scale.Set(2,2,2);
    gstate->SetSize(bolt,-4,-4,-4,4,4,4);

    // add this bolt to the global list of bolter bolts
    com->list_add_node (bolt, bolt_list, 0);

    bolt->remove = bolt_remove; //	cleanup function for bolt

    winfoSetModel(bolt,MDL_BOLT);
    bolt->movetype = MOVETYPE_FLYMISSILE;
    bolt->s.renderfx |= RF_FULLBRIGHT;

    bolt->think = bolt_flight_think;
    bolt->nextthink = gstate->time + .1;
    bolt->touch = (touch_t)bolt_touch;

    hook->remove_time = gstate->time + winfo->lifetime;

    bolter->weapon.lastFired=gstate->time;
	WEAPON_FIRED(self);
}

//---------------------------------------------------------------------------
// bolter_exit_level()
//---------------------------------------------------------------------------
void    bolter_exit_level (userEntity_t *self)
{
    listNode_t      *cur_node;
    userEntity_t    *bolt;

    // traverse the list of bolts and remove all of their userhooks,
    // friction hooks and sticky hooks
    cur_node = bolt_list->head;

    while ( cur_node )
    {
        bolt = (userEntity_t *) cur_node->ptr;

        // go to the next node
        cur_node = com->list_node_next (cur_node);

        //	remove bolt from list, free hooks and free edict
        self->remove (bolt);
    }
}

//-----------------------------------------------------------------------------
//	weapon_bolter
//-----------------------------------------------------------------------------
void weapon_bolter(userEntity_t *self)
{
    winfoSetSpawn(winfo,self,30.0,NULL);
}

//-----------------------------------------------------------------------------
//	ammo_bolts
//-----------------------------------------------------------------------------
void ammo_bolts (userEntity_t *self)
{
    winfoAmmoSetSpawn(winfo, self, 50, 30.0, NULL);
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_bolter_register_func()
{
	gstate->RegisterFunc("bolt_remove",bolt_remove);
	gstate->RegisterFunc("bolt_think",bolt_think);
	gstate->RegisterFunc("boltFade",boltFade);
	gstate->RegisterFunc("bolt_flight_think",bolt_flight_think);
	gstate->RegisterFunc("bolter_use",bolter_use);
	gstate->RegisterFunc("bolt_touch",bolt_touch);
	gstate->RegisterFunc("bolter_give",bolter_give);
	gstate->RegisterFunc("bolter_select",bolter_select);
}
