#include	"weapons.h"
#if _MSC_VER
#include "io.h"
#endif
#include "fcntl.h"

#include "nightmare.h"
#include "w_attribute.h"

//#include "..\World\ai_func.h"

// Grapple hack, no weapon info for the hook (not needed)
//extern void (grapple_Cmd)(userEntity_t *ent);


float sin_tbl[] = {0.017,0.515,0.874,0.999,0.857,0.484,-0.017,-0.515,-0.874,-0.999,-0.857,-0.484};
float cos_tbl[] = {0.999,0.857,0.484,-0.017,-0.515,-0.874,-0.999,-0.857,-0.484,0.017,0.515,0.874};

CVector vec3_origin_w(0,0,0);

#define	IS_INSTAGIB				(deathmatch->value && dm_instagib->value)
autoAim_t autoAim;
#define WSP_RAISE_TRACE    20
// NSS[3/9/00]:Added respawn.
void Weapon_SetRespawn(userEntity_t *self, float delay);
void weaponAmmoCountUpdate(userEntity_t *self, ammo_t *ammo);
///////////////////////////////////////////////////////////////////////
// Logic [7/22/99]: 
// <BEGIN RANTING COMMENTARY>
//  The follogin ai_ code was in zeus.cpp (and used in two other objects!)
//  so I think to myself, "what kind of dumbass would be lazy enough to 
//  make this mess? Now I gotta go and move this shit around and waste 
//  my time cleaning shit up." Of course, I am the kind of dumbass that
//  will try out code one place, and maybe "move it later", though this
//  wasn't the case this time. So PLEASE PLEASE PLEASE, don't write 
//  "test code" and leave it sitting in production. Clean it up and 
//  put it where it belongs to save others the extra hassle of trying 
//  to understand what the hell you were thinking, then doing the right
//  thing with your "test code". (I suspect The ThirdTurd crew did this)
//  I'll try to do the same so I don't come off as such a hypocrite :)
// </END RANTING COMMENTARY>

// SCG[11/24/99]: Shut the fuck up dumbass. You suck. You're code sucks.
// SCG[11/24/99]: You're a hypocrite. You're fired.

void Weapon_SaveHook( FILE *f, struct edict_s *ent, field_t *hook_fields, int nSize )
{
	field_t			*field;
	void			*pHook = gstate->X_Malloc( nSize, MEM_TAG_MISC );

//	gstate->Con_Dprintf( "Saving hook for: %s\n", ent->className );
	
	// SCG[11/21/99]: Allocate temp memory for hook
	memcpy( pHook, ent->userHook, nSize );

	// change the pointers to lengths or indexes
	for( field = hook_fields; field->name; field++ )
	{
		com->AI_WriteSaveField1( f, field, ( byte * ) pHook );
	}

	// write the block
	com->AI_SaveHook( f, pHook, nSize );

	// now write any allocated data following the edict
	for( field = hook_fields; field->name; field++ )
	{
		com->AI_WriteSaveField2( f, field, ( byte * ) ent->userHook );
	}

	// SCG[11/21/99]: Free temp memory
	gstate->X_Free( pHook );
}

void Weapon_LoadHook( FILE *f, struct edict_s *ent, field_t *hook_fields, int nSize )
{
	field_t		*field;

	ent->userHook = gstate->X_Malloc( nSize, MEM_TAG_HOOK );

	com->AI_LoadHook( f, ent->userHook, nSize );

	for( field = hook_fields; field->name; field++ )
	{
		com->AI_ReadField (f, field, (byte *)ent->userHook );
	}
}


#define	TOUCHHOOK(x) (int)&(((touchHook_t *)0)->x)
field_t touch_hook_fields[] = 
{
	{"give_func",		TOUCHHOOK(give_func),		F_FUNC},
	{"select_func",		TOUCHHOOK(select_func),		F_FUNC},
	{NULL, 0, F_INT}
};

void touch_hook_save( FILE *f, edict_t *ent )
{
	Weapon_SaveHook( f, ent, touch_hook_fields, sizeof( touchHook_t ) );
}

void touch_hook_load( FILE *f, edict_t *ent )
{
	Weapon_LoadHook( f, ent, touch_hook_fields, sizeof( touchHook_t) );
}

typedef void ( *ammo_spawn_func_t )( userEntity_t *self );
#define	AMMOTOUCHHOOK(x) (int)&(((ammoTouchHook_t *)0)->x)
field_t ammo_touch_hook_fields[] = 
{
	{"winfo",		AMMOTOUCHHOOK(winfo),	F_FUNC},
	{"count",		AMMOTOUCHHOOK(count),	F_INT},
	{NULL, 0, F_INT}
};

void ammo_touch_hook_save( FILE *f, edict_t *ent )
{
}

void ammo_touch_hook_load( FILE *f, edict_t *ent )
{
//	char *szClassName = ent->className;// SCG[1/24/00]: not used

	ammo_spawn_func_t spawnfunc = ( ammo_spawn_func_t ) com->FindSpawnFunction( ent->className );

	if( spawnfunc == NULL )
	{
		return;
	}

	// SCG[12/7/99]: Spawn a temp entity, steal it's hook, and then kill it.
	userEntity_t *pTempEntity = gstate->SpawnEntity();
	pTempEntity->className = ent->className;
	spawnfunc( pTempEntity );
	ammoTouchHook_t	*pHook = ( ammoTouchHook_t * ) pTempEntity->userHook;
	ent->userHook = ( void * ) pHook;
	pTempEntity->userHook = NULL;
	gstate->RemoveEntity( pTempEntity );
}


///////////////////////////////////////////////////////////////////////////////
//	ai_setfov
//
//	sets up a bots field of view parameter
//	set's hook->fov to cos (fov_degrees / 2)
///////////////////////////////////////////////////////////////////////////////
void    ai_setfov (userEntity_t *self, float fov_degrees)
{
    playerHook_t    *hook = (playerHook_t *) self->userHook;
    float           rad;

    // convert degrees to radians
    rad = (fov_degrees / 2.0) * (M_PI * 2 / 360.0);
    hook->fov = cos (rad);
}

// ai_infront

// removed, horribly defective and only used by ai_visible (q.v.) -- zjb

// ai_visible

// removed, was only used by nightmare and com_Visible is good enough to use instead -- zjb

// Logic [7/22/99]: ok, end of ai_ relocation code
///////////////////////////////////////////////////////////////////////

////////////////////////////////////////////
// crnd
//
// returns a random value between -1 and 1
///////////////////////////////////////////

float crnd(void)
{
    return 2*(rnd() - 0.5);
}

//////////////////////////////////////////////////////
// lim_rand
//
// Return a pseudo-random value between 0 and limit
//////////////////////////////////////////////////////

float lim_rand(int limit)
{

    return((float)(limit*(((float)rand())/0x7fff)));
}

void weapon_PowerSound(userEntity_t *self)
{
    playerHook_t    *hook = (playerHook_t *)self->userHook;

    if(hook && hook->power_boost > 0)
	{
		float volume = hook->power_boost * 0.2;
        gstate->StartEntitySound(self,CHAN_WEAPON,gstate->SoundIndex("global/a_powerquad.wav"),volume,ATTN_NORM_MIN, ATTN_NORM_MAX);
	}
}

void weaponRicochet(CVector &vecPos)
{
    gstate->WriteByte(SVC_TEMP_ENTITY);
    gstate->WriteByte(TE_RICOCHET);
    gstate->WritePosition(vecPos);
    gstate->MultiCast(vecPos, MULTICAST_PHS);
}


void weapon_no_ammo_alert(userEntity_t *self)
{
  // print message telling the player there's no ammo
  //gstate->centerprint(self, 2.0, "%s\n", tongue_weapons[T_WEAPON_NO_AMMO]);

  // play a global no-ammo sound
  gstate->StartEntitySound(self, CHAN_OVERRIDE, gstate->SoundIndex("global/we_noammo.wav"), 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
}


/*
void weaponGunShot(CVector &vecOrg, CVector &vecDir)
{

    gstate->WriteByte(SVC_TEMP_ENTITY);
    gstate->WriteByte(TE_GUNSHOT);
    gstate->WritePosition(vecOrg);
    gstate->WriteDir(vecDir);
    gstate->MultiCast(vecOrg, MULTICAST_PVS);
}
*/

////////////////////////////////////////////////////////////////////////
//  CVector weaponTraceShot(userEntity_t *entShooter, 
//                          CVector &vecShotOffSet, 
//                          float fSpeed )
//
//  Description:
//      Determines a vector describing a shot endpoint or a velocity if 
//      fSpeed is non-zero.
//
//  Parameters:
//      userEntity_t *entShooter        entity doing the shooting (to
//                                      accomodate autoaming.
//      CVector vecShotOffset           offset from entShooter origin
//                                      of the gun muzzle
//      fSpeed                          projectile speed, or 0 if the
//                                      trace needs to find an impact
//                                      point in the map
//
//  Return:
//      a vector describing the shot endpoint or projectile velocity if
//      fSpeed is non-zero.
//
trace_t weaponTraceShot(userEntity_t *entShooter, CVector &vecShotOffset, float fSpeed )
{
    playerHook_t    *playerHook;
    trace_t         tr;     // not a good thing to return a local variable
    CVector         vecStart, vecEnd, vecForward;
    CVector         vecShotDir;
    short autoaim;
    vecShotDir.Set(0,0,0);

    memset( &tr, 0x00, sizeof(trace_t) );
    weapon_PowerSound(entShooter);

    if( (playerHook = (playerHook_t *)entShooter->userHook) != NULL)
    {
        autoaim = (short)gstate->GetCvar("autoaim");
        if(playerHook->autoAim.ent && autoaim > 0)
        {
            vecStart = entShooter->s.origin + vecShotOffset;
            if(fSpeed)
            {
                vecShotDir = playerHook->autoAim.ent->s.origin - vecStart;
                vecShotDir.Normalize();
                vecShotDir = vecShotDir * fSpeed;
            } 
            else 
            {
                vecShotDir = playerHook->autoAim.ent->s.origin;
            }
            tr = gstate->TraceLine_q2(vecStart, vecShotDir, entShooter, MASK_SHOT);
        } 
        else 
        {
            // trace to crosshair
            if( (entShooter->flags & FL_CLIENT) && entShooter->client )
            {
				CVector vecOrigin;
				vecOrigin = entShooter->s.origin;
				if( entShooter->client->ps.pmove.pm_flags & PMF_DUCKED )
				{
					vecOrigin.z -= 25;
				}

                vecStart = entShooter->client->v_angle;
                vecStart.AngleToForwardVector(vecForward);
//                vecStart = entShooter->s.origin;
                vecStart = vecOrigin;
                vecStart.z += WSP_RAISE_TRACE;

                vecEnd = vecOrigin + vecForward * 4000;
                tr = gstate->TraceLine_q2(vecStart, vecEnd, entShooter, MASK_SHOT);
            }
            else
            {
                // TODO: ISP [09-30-99] depending of the difficulty setting set 
                // the accuracy level accordingly
                if ( entShooter->flags & FL_BOT )
                {
                    playerHook = (playerHook_t *)entShooter->userHook;

                    userEntity_t *pEnemy = entShooter->enemy;
                    if ( pEnemy )
                    {
                        CVector endPoint = pEnemy->s.origin;
                        endPoint.z += 12.0f;
                        if ( rnd() > 0.5f )
                        {
	                        CVector forward, right, up;
	                        entShooter->s.angles.AngleToVectors(forward, right, up);

                            float fSpreadX = 40.0f;
                            float fSpreadZ = 40.0f;
                            float sx = (fSpreadX * rnd());
                            if ( rnd() < 0.5f )
                            {
                                sx = -sx;
                            }
	                        float sz = (fSpreadZ * rnd());
                            if ( rnd() < 0.5f )
                            {
                                sz = -sz;
                            }
	                        endPoint = pEnemy->s.origin + (right * sx) + (up * sz);
                            endPoint.z += 12.0f;
                        }

                        tr.ent = pEnemy;
                        tr.endpos = endPoint;
                    }
                }
            }
        }
    }

    return tr;
}


// Logic[8/4/99]:  This whole fucking thing is dumb, special cased code sitting here
// for the shotcycler, ripgun and glock. It will die. Common shit sits in weaponTraceShot()

////////////////////////////////////////////
// weapon_trace_attack
// returns an int depending on what was hit
// 0 - nothing was hit
// 1 - living thing was hit
// 2 - bmodel/world was hit
////////////////////////////////////////////

int weapon_trace_attack ( userEntity_t *self, int damage, int num_bullets, int distance, short flags)
{
    playerHook_t *phook=(playerHook_t *)self->userHook;
    int             per_partical,hit_a_monster/*, iRicochet*/;// SCG[1/24/00]: not used
    CVector         forward, right, up, temp, end, spread, dir;
    trace_t         tr;

    temp = self->client->v_angle;
    weapon_PowerSound(self);
    // using auto aim?
//   if (tr.ent=checkAutoAim(self,tr.endpos))
    if ( tr.ent=phook->autoAim.ent )
    {
        tr.endpos = phook->autoAim.shootpos;
        temp=tr.endpos-self->s.origin;
        VectorToAngles(temp,temp);
    }

    temp.AngleToVectors (forward, right, up);

    temp = self->s.origin + forward * 8.0 + up * 16;    
    hit_a_monster=0;
//    iRicochet = 0;

    if ( damage && num_bullets )
    {
        ///////SPREAD RATIOS, IF MORE BULLETS MORE SPREAD//////
        if ( num_bullets >= 2 && num_bullets <= 4 )
            spread = CVector (.03, .03, 0.0);
        else if ( num_bullets >= 5 && num_bullets <= 7 )
            spread = CVector(.06,.06,0.0);
        else if ( num_bullets >= 8 && num_bullets <= 12 )
            spread = CVector(.09,.09,0.0);
        else if ( num_bullets >= 13 )
            spread = CVector(.12,.12,0.0);
        else
        {
        }  /// 1 shot////	

        //////// DAMAGE PER PARTICAL//////

        per_partical = damage / num_bullets;

        for ( int i =0; i <num_bullets; i++ )
        {
            // single shot hits target
            if ( num_bullets == 1 )
                end = temp + forward * distance;

            // hitMiddle? first shot hits target
            if ( (flags & WTA_HITMIDDLE) && !i )
                end = temp + forward * distance;
            else
                if ( num_bullets > 1 )           // other shots spread out
            {
                dir = forward + right * crnd () * spread.x + up * crnd () * spread.y;
                end = temp + dir * distance;
            }

            ///////TRACE ME/////
            tr = gstate->TraceLine_q2 (temp, end, self, MASK_SHOT);

            //See if we actually hit something
            if ( tr.fraction!= 1.0 )
            {
                //Make sure we are damaging something that is meant to be damaged
                if ( tr.ent && tr.ent->takedamage != DAMAGE_NO )
                {
                    dir = end - temp;
                    dir.Normalize ();
                    com->Damage (tr.ent, self, self, tr.endpos, dir, per_partical, DAMAGE_INERTIAL);
                    hit_a_monster=2;
//                    weaponBloodSplat (tr.ent, temp, end, tr.endpos, 96.0);
                }
                if ( tr.ent && tr.ent->solid == SOLID_BSP || tr.ent->takedamage == DAMAGE_NO )
                {
	                weaponScorchMark( tr, SM_BULLET );
//                    CVector v3dir = dir;// SCG[1/24/00]: not used
                    if ( flags & WTA_SPARKS )
                        clientBeamSparks(tr.endpos,40,1);        //dir,40,0x22);
                    hit_a_monster = 1;
                }
            }
        }
        // 19990217 JAR - moved client smoke here, was generating 10 messages/shot (bad for network with 6 shots in 0.5 seconds)
        if ( tr.fraction!= 1.0 )
        {
            clientSmoke(tr.endpos,.5,40,10,20);
            // send ricochet temp event
            weaponRicochet(tr.endpos);
        }

    }
    return	hit_a_monster;

}

////////////////////////////////////////////////////////////////
// weapon_check_ammo
//
// check for a valid pointer to an ammo type in self->curWeapon
// if pointer to ammo is valid, then the ammo is sent a command
// to decrement its rounds ammo_count ammount
////////////////////////////////////////////////////////////////

ammo_t  *weapon_check_ammo (userEntity_t *self, char *ammo_name, int ammo_count)
{
    return(NULL);
}

//---------------------------------------------------------------------------
// spawn_sprite_explosion()
//---------------------------------------------------------------------------
void spawn_sprite_explosion(userEntity_t *self, CVector &org, int play_sound)
{
    gstate->WriteByte(SVC_TEMP_ENTITY);

    //	FIXME:	make a separate te_explosion type for no sound	
    if ( play_sound )
        gstate->WriteByte (TE_EXPLOSION1);
    else
        gstate->WriteByte (TE_EXPLOSION1);    // amw - should this be TE_EXPLOSION2 ?

    gstate->WritePosition(org);
    gstate->MultiCast(org,MULTICAST_PVS);
}

//---------------------------------------------------------------------------
// spawnPolyExplosion()
//---------------------------------------------------------------------------
void spawnPolyExplosion(CVector &org, float scale, float lsize, CVector &color, short flags)
{
    CVector angles(0,0,0);

    // send explosion message
    gstate->WriteByte(SVC_TEMP_ENTITY);
    if ( flags & PEF_SOUND )
    {
        gstate->WriteByte(TE_POLYEXPLOSION_SND);
    }
	else
    {
        gstate->WriteByte(TE_POLYEXPLOSION);
    }

    gstate->WritePosition(org);
    gstate->WritePosition(angles);
    gstate->WriteShort((unsigned short)(scale*1024));
    gstate->WriteFloat(lsize);
    if ( lsize )
    {
        gstate->WritePosition(color);
    }
    gstate->MultiCast(org,MULTICAST_PVS);
}

//---------------------------------------------------------------------------
// spawnPolyExplosion()
//---------------------------------------------------------------------------
void spawnPolyExplosion(CVector &org, CVector &vec, float scale, float lsize, CVector &color, short flags)
{
    CVector angles(0,0,0);

    // handle flags
    if ( (flags & PEF_ANGLES) )
    {
        angles=vec;
    }
    if ( (flags & PEF_NORMAL) )
    {
        VectorToAngles(vec, angles);

        org.x += vec.x * 4;
        org.y += vec.y * 4;
        org.z += vec.z * 4;
    }

    // send explosion message
    gstate->WriteByte(SVC_TEMP_ENTITY);
    if ( flags & PEF_SOUND )
    {
        gstate->WriteByte(TE_POLYEXPLOSION_SND);
    }
    else
    {
        gstate->WriteByte(TE_POLYEXPLOSION);
    }

    gstate->WritePosition(org);
    gstate->WritePosition(angles);
    gstate->WriteShort((unsigned short)(scale*1024));
    gstate->WriteFloat(lsize);
    if ( lsize )
    {
        gstate->WritePosition(color);
    }
    gstate->MultiCast(org,MULTICAST_PVS);
}

/////////////////////////////////
//	weapon_face_velocity
/////////////////////////////////

void    weapon_face_velocity (userEntity_t *self)
{
   CVector temp, dir;

   dir = self->velocity;
   dir.Normalize ();

   VectorToAngles(dir, temp);
   self->s.angles = temp;
}

/////////////////////////////////
// weapon_spawn_projectile
//
//
//
/////////////////////////////////
// DEBUG, this should be an enum!
#ifndef LOGTYPE_WEAPONS
#define  LOGTYPE_WEAPONS 2
#endif
userEntity_t *weapon_spawn_projectile (userEntity_t *self, CVector &org, float xy_vel, float xangle, char *className, qboolean bAutoAim)
{

   playerHook_t *phook;
   userEntity_t *ent/*,*player=NULL*/,*autoaim;// SCG[1/24/00]: not used
   CVector vel,origin,pos,vpos,whv;
   CVector shootpos;
   trace_t    trace_crosshair;
	CVector vecOrigin;

   // Check Parms:
   if(!self)
      return NULL;
   if( (phook = (playerHook_t *)self->userHook) == NULL )
       if( (phook = (playerHook_t *)self->owner->userHook) == NULL )
           return NULL;

   weapon_PowerSound(self);
   // initialize
   ent = gstate->SpawnEntity ();

   ent->flags |= FL_NOSAVE;
   ent->owner = self;
   ent->movetype = MOVETYPE_TOSS;
   ent->solid= SOLID_BBOX;
   ent->className = className;

   ent->clipmask = MASK_SHOT;
   ent->svflags = SVF_SHOT;         //	won't clip against any ents with clipmask == MASK_SHOT

/*   // get direction of shot
	if ( self->flags & FL_CLIENT && gstate->GetCvar("autoaim") > 0 )
	{
		autoaim=phook->autoAim.ent;
		shootpos = phook->autoAim.shootpos;
	} 
	else 
	{
		autoaim=0;
	}

	if( self->client )
	{
		vel = self->client->v_angle;
	}
	else
	{
		vel = self->s.angles;
	}
*/
   // make this stuff more bot-friendly
	if (self->flags & FL_CLIENT)
	{
		if (gstate->GetCvar("autoaim") > 0)
		{
			vel = self->client->v_angle;
			autoaim=phook->autoAim.ent;
			shootpos = phook->autoAim.shootpos;
		}
		else
		{
			vel = self->client->v_angle;
			autoaim = NULL;
		}
	} 
	else if ((self->flags & FL_BOT) && self->enemy)
	{
		autoaim=self->enemy;
		shootpos = self->enemy->s.origin;
		bAutoAim = TRUE;
		VectorToAngles(shootpos - self->s.origin,vel);
	}
	else
	{
		vel = self->s.angles;
		autoaim = NULL;
	}


   // add to xangle (for tossing things)
   // cap at +- 89
   if ( xangle != 0.0 )
   {
       vel.x += xangle;
       vel.x = vel.x>89 ? 89 : vel.x;
       vel.x = vel.x<-89 ? -89 : vel.x;      
   }
   vel.AngleToVectors(forward,right,up);

    // set origin - offset from self->s.origin, with calculation for handedness
   origin=self->s.origin + weaponHandVector(self,forward*org.y,right*org.x);
   origin.z += org.z;
   ent->s.origin = origin;

   if ( autoaim && bAutoAim )
   {
      vel=shootpos-origin;
      VectorToAngles(vel,vel);
      vel.AngleToVectors(forward,right,up);
      if ( xangle != 0.0 )
      {
         vel.x += xangle;
         vel.x = vel.x>89 ? 89 : vel.x;
         vel.x = vel.x<-89 ? -89 : vel.x;      
      }
      vel=forward*xy_vel;
   } 
   else 
   {
      // trace to see what crosshair is pointing at
	   vecOrigin = self->s.origin;
		if( self->client && self->client->ps.pmove.pm_flags & PMF_DUCKED )
		{
			vecOrigin.z -= 20;
		}
      pos = vecOrigin;
      pos.z += WSP_RAISE_TRACE;
      vpos=pos+forward*2000;
      // pos=pos+weaponHandVector(self,zeroVector,zeroVector);
      trace_crosshair = gstate->TraceLine_q2(pos, vpos, self, MASK_SHOT);
      pos=trace_crosshair.endpos-origin;           // calc vector to whatever crosshair is over
      pos.Normalize(); // normalize the direction-vector from source to target (vector of length 1 in proper direction)
      // vel.Scale(pos, xy_vel); // scale for speed
      vel = pos * xy_vel;
   }
   
   // Attack boost increases weapon firing speads and projectile velocities
   if(phook->attack_boost)
       vel += vel * phook->attack_boost * 0.3;

   ent->velocity = vel; // set the entity's velocity vector to the scaled vector
       
   gstate->LinkEntity(ent);
   weapon_face_velocity(ent); // point the projectile in the direction it's going
   return(ent);
}


//---------------------------------------------------------------------------
// weaponCreateInv()
//---------------------------------------------------------------------------
//weapon_t *weaponCreateInv(weaponInfo_t *winfo,usew_t use_func,command_t cmd_func,short struct_size)
weapon_t *weaponCreateInv (invenList_t *inventory, weaponInfo_t *winfo, usew_t use_func, 
                           command_t cmd_func, short struct_size)
{
    weapon_t    *weapon;
    int         modelIndex;

    //	get modelindex for pickup/inventory item
    modelIndex = gstate->ModelIndex (winfoModel(MDL_ARTIFACT_STD));

    weapon = (weapon_t *) gstate->InventoryCreateItem (inventory, winfo->weaponName, (invenUse_t)use_func, 
                                                       (invenCommand_t)cmd_func, modelIndex, ITF_WEAPON | winfo->flags, struct_size); 

    if ( !weapon )
        return	NULL;

    //	weapon_t also has pointer to winfo structure
    weapon->winfo = winfo;

    return	weapon;
}

//---------------------------------------------------------------------------
// ammoCreate()
//---------------------------------------------------------------------------
ammo_t *ammoCreate (invenList_t *inventory, weaponInfo_t *winfo)
{
    ammo_t *ammo;
    int     modelIndex;

    //	get modelindex for pickup/inventory item
    if ( winfoModel(MDL_AMMO_STD) != MDL_NOMODEL )
        modelIndex = gstate->ModelIndex (winfoModel(MDL_AMMO_STD));
    else
        modelIndex=0;

    ammo = (ammo_t *) gstate->InventoryCreateItem (inventory, winfo->ammoName, NULL, 
                                                   (invenCommand_t)ammoGenericCommand, modelIndex, ITF_AMMO, sizeof (ammo_t)); 

    if ( !ammo )
        return	NULL;

    //	ammo_t also has counter and pointer to winfo structure
    ammo->count = 0;
    ammo->winfo = winfo;

    return	ammo;
}

//---------------------------------------------------------------------------
// ammoAdd()
//---------------------------------------------------------------------------
ammo_t *ammoAdd(userEntity_t *self,int amount,weaponInfo_t *winfo)
{
    ammo_t *ammo;

    // make sure that this ammo type isn't already in inventory
    if ( !(ammo=(ammo_t *)gstate->InventoryFindItem(self->inventory,winfo->ammoName)) )
    {
        // create ammo
        if ( !(ammo=ammoCreate (self->inventory, winfo)) )
        {
            gstate->Con_Printf("Unable to malloc %s.\n",winfo->ammoNetName);
            return(NULL);
        }

        // add the ammo to the player's inventory list
        if ( !gstate->InventoryAddItem(self, self->inventory,(userInventory_t *)ammo) )
        {
            gstate->Con_Printf("Could not add %s to inventory.\n",winfo->ammoNetName);
            return(NULL);
        }
    }

    // ammo type is in inventory, so add to count
//    if(!amount)
  //      amount = winfo->initialAmmo;

	ammoGiveTake_t give;
	give.count = amount;
	give.owner = self;
    ammo->command((userInventory_t *)ammo,"give_ammo",&give);
    return(ammo);
}

//---------------------------------------------------------------------------
// winfoGenericCommand()
//---------------------------------------------------------------------------
void *winfoGenericCommand(weaponInfo_t *winfo,userInventory_t *inv,char *commandStr,void *data)
{
    userEntity_t *self=(userEntity_t *)data;
    playerHook_t *phook=(playerHook_t *)self->userHook;
    weapon_t *weapon=(weapon_t *)inv;

    if ( !stricmp(commandStr, "death_msg") )
    {
		killerInfo_t *ki=(killerInfo_t *)data;

		// for dedicated servers, echo to the console.
		if (dedicated->value)
		{
			char msg[128];// SCG[1/24/00]: lets make it bigger to make sure

			char *text;
			text = gstate->cs.StringForID(winfo->deathmsgIndex);

			if ( winfo->attacker_victim)
			{
				Com_sprintf(msg, sizeof(msg),text, com->ProperNameOf(ki->attacker), com->ProperNameOf(ki->victim));
			}
			else
			{
				Com_sprintf(msg, sizeof(msg),text, com->ProperNameOf(ki->victim), com->ProperNameOf(ki->attacker));
			}

			gstate->Con_Printf("%s",msg);
		}

		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierID(winfo->deathmsgIndex,2);
		if (winfo->attacker_victim)
		{
			gstate->cs.SendString(com->ProperNameOf(ki->attacker));
			gstate->cs.SendString(com->ProperNameOf(ki->victim));
		}
		else
		{
			gstate->cs.SendString(com->ProperNameOf(ki->victim));
			gstate->cs.SendString(com->ProperNameOf(ki->attacker));
		}
		gstate->cs.Unicast(ki->victim,CS_print_center, 2.0);
		gstate->cs.Multicast(CS_print_normal);
		gstate->cs.EndSendString();

    }
    else if ( !stricmp (commandStr, "suicide_msg") )
    {
        killerInfo_t *ki=(killerInfo_t *)data;

		// for dedicated servers, echo to the console.
		if (dedicated->value)
		{
			char msg[128];

			Com_sprintf(msg, sizeof(msg),"%s %s", com->ProperNameOf(ki->victim), tongue_deathmsg_self[T_DEATHMSGSELF_KILLBRUSH]);
			gstate->Con_Printf("%s",msg);
		}

		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierStr("%s %s",2);
		gstate->cs.SendString(com->ProperNameOf(ki->victim));
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue_deathmsg_self,T_DEATHMSGSELF_KILLBRUSH));
		gstate->cs.Unicast(ki->victim,CS_print_center, 2.0);
		gstate->cs.Multicast(CS_print_normal);
		gstate->cs.EndSendString();
    }
    else if ( !stricmp(commandStr,"ammo_count") )
    {
        if ( weapon->ammo )
            weapon->ammo->command((userInventory_t *)weapon->ammo,"count",data);
    }
    else if ( !stricmp(commandStr, "change") )
    {
        winfoAnimate(winfo,(userEntity_t *)data,ANIM_AWAY_STD,FRAME_ONCE|FRAME_WEAPON,.05);
    }
    else if ( !stricmp(commandStr, "change_time") )
    {
        *(float *)data=(winfoNumFrames(ANIM_AWAY_STD)) * 0.05;
    }
    else if ( !stricmp(commandStr, "winfo") )
    {
        *(void **)data=(void *)winfo;
    }
    else if ( !stricmp(commandStr,"ambient") )
    {
		if (gstate->bCinematicPlaying)
			return NULL;

        float animtime=winfoNumFrames(ANIM_AMBIENT_STD)*.05;
        short animtype=0,animnum=0,num=0;

        if ( winfoAnim(ANIM_AMBIENT3_STD) != ANIM_NOANIM )
            num=3;
        else if ( winfoAnim(ANIM_AMBIENT2_STD) != ANIM_NOANIM )
            num=2;
        else if ( winfoAnim(ANIM_AMBIENT_STD) != ANIM_NOANIM )
            num=1;
        else
            return(NULL);

        animnum=rnd()*num;
        animtime=winfoNumFrames(ANIM_AMBIENT_STD+animnum)*.05;

        // play ambient anim
        if ( winfo->flags & WFL_LOOPAMBIENT_ANIM )
        {
            phook->ambient_time=gstate->time + 3600;
            animtype=FRAME_LOOP;
        }
        else
        {
            phook->ambient_time=gstate->time+animtime+2+(rnd()*6);
            animtype=FRAME_ONCE;
            if ( winfoSound(SND_AMBIENT_STD+animnum) != SND_NOSOUND )
                winfoPlay(self,SND_AMBIENT_STD+animnum,1.0f);
        }
        winfoAnimate(winfo,self,ANIM_AMBIENT_STD+animnum,animtype|FRAME_WEAPON|FRAME_NODELAY,.05);
    }
    else if ( !stricmp (commandStr, "rating") )       // return a value between 0 and 1000
        *(int *)data=winfo->rating;
    else 
      if (!stricmp(commandStr,"display_order") )
        *(int *)data=winfo->display_order;

    return(NULL);
}

//---------------------------------------------------------------------------
// ammoGenericCommand()
//---------------------------------------------------------------------------
void *ammoGenericCommand(userInventory_t *ptr,char *commandStr,void *data)
{
    ammo_t          *ammo=(ammo_t *)ptr;
    weaponInfo_t    *winfo;

    if ( !stricmp (commandStr, "give_ammo") )            // adds (*data) units of ammo
    {
		ammoGiveTake_t *give = (ammoGiveTake_t *)data;
		ammo->count += give->count;

//        ammo->count = ammo->count + *(int *)data;

        winfo = (weaponInfo_t *) ammo->winfo;
        //	limit amount of ammo
        if ( ammo->count > winfo->ammo_max )
            ammo->count = winfo->ammo_max;

		if (give->owner)
		{
			weaponAmmoCountUpdate(give->owner, ammo);
		}
    }
    else if ( !stricmp (commandStr, "take_ammo") )       // uses (*data) units of ammo
    {
        if ( ammo->count > 0 )
        {
//            short count=*(short *)data;
			ammoGiveTake_t *take = (ammoGiveTake_t *)data;
			short count = take->count;

            if ( ammo->count >= count )
                ammo->count = ammo->count - count;
            else
            {
                count=ammo->count;
                ammo->count=0;
            }

			if (take->owner)
			{
				weaponAmmoCountUpdate(take->owner, ammo);
			}

            return((void *)count);
        }
        else
            return(NULL);
    }
    else if ( !stricmp (commandStr, "use_ammo") )        // uses 1 unit of ammo
    {
        if ( ammo->count > 0 )
        {
            ammo->count--;
            return((void *)1);
        }
        else
            return(NULL);
    }
    else if ( !stricmp (commandStr, "count") )           // returns ammo count
    {
        if ( data )
            *(int *)data=ammo->count;
        return((void *)ammo->count);
    }

    return NULL;
}

//---------------------------------------------------------------------------
// weaponDllInit()
//---------------------------------------------------------------------------
int weaponDllInit(weaponInfo_t *winfo)
{
    return(sizeof(userEntity_t)+sizeof(serverState_t));
}

//---------------------------------------------------------------------------
// weaponLevelInit()
//---------------------------------------------------------------------------
void weaponLevelInit(weaponInfo_t *winfo)
{
    userEntity_t *ent;
    weaponFrame_t *fptr;
    weaponCmd_t *cmd;
    char **ptr;

/*
#ifdef _DEBUG    
	// add commands
    cmd=&winfo->command[0];
    while ( cmd->text )
    {
        gstate->AddCommand(cmd->text,cmd->func);
        
        //DKLOG_Write(LOGTYPE_MISC, 0.0f, "cmd->text=%s",cmd->text);
     
        cmd++;
    }
#else
    cmd = &winfo->command[1];
    gstate->AddCommand(cmd->text,cmd->func);
#endif    
*/
/*
#ifndef _DEBUG    
	if( gstate->GetCvar( "cheats" ) )
#endif
	{
		// add commands
		cmd=&winfo->command[0];
		while ( cmd->text )
		{
			gstate->AddCommand(cmd->text,cmd->func);
        
			//DKLOG_Write(LOGTYPE_MISC, 0.0f, "cmd->text=%s",cmd->text);
     
			cmd++;
		}
	}
*/
    cmd = &winfo->command[1];
    gstate->AddCommand(cmd->text,cmd->func);

	  //gstate->AddCommand("grapple", grapple_Cmd);     // unused
   
    // prev/next weapon commands
    gstate->AddCommand("weapprev", weapprev_Command);
    gstate->AddCommand("weapnext", weapnext_Command);
		gstate->AddCommand("weap_hud",     weap_Command);     // brings up weapon hud, no action
                                            
    //gstate->AddCommand("unholster", Cmd_Unholster); // unused

    // cache models
    ptr=&winfo->models[0];
    while ( *ptr )
    {
        if ( *ptr != MDL_NOMODEL )
            gstate->ModelIndex (*ptr);
        ptr++;
    }

    // cache sounds
    ptr=&winfo->sounds[0];
    while ( *ptr )
    {
        if ( *ptr != SND_NOSOUND )
            gstate->SoundIndex (*ptr);
        ptr++;
    }

    // get frame info
    ent=gstate->SpawnEntity();
    fptr=winfo->frames;
    while ( fptr->name )
    {
        dk2SetModel(ent,winfo->models[fptr->model]);

        if ( !strcmp(fptr->name,"sp2") )         // sprites
        {
            dsprite_t sprite;
            FILE *h;

            gstate->gi->FS_Open(winfo->models[fptr->model],&h);
            if ( h )
            {
                gstate->gi->FS_Read(&sprite,sizeof(dsprite_t)-sizeof(dsprframe_t),h);
                gstate->gi->FS_Close(h);

				//unix - byte-order fix
				sprite.ident = LittleLong (sprite.ident);
				sprite.version = LittleLong (sprite.version);
				sprite.numframes = LittleLong (sprite.numframes);

                fptr->first=0;
                fptr->last=sprite.numframes-1;
            }
            fptr++;
        }
        else                                   // models
            do
            {
                if ( fptr->name != ANIM_NOANIM )
				{
                    com->GetFrames(ent,fptr->name,&fptr->first,&fptr->last);

#ifdef _DEBUG
					if (!fptr->first && !fptr->last)
					{
						gstate->Con_Dprintf("%s animation %s does not exist.\n",winfo->weaponName,fptr->name);
					}
#endif
				}

                fptr++;
            } while ( fptr->model == -1 );
    }
    ent->remove (ent);

	// set the weapon attribute values from "weapons.csv" document
	WEAPONATTRIBUTE_SetInfo( winfo );
}

//---------------------------------------------------------------------------
// weaponHandVector()
//---------------------------------------------------------------------------
CVector weaponHandVector(userEntity_t *ent,CVector &fpos,CVector &hpos)
{
    userEntity_t *player=NULL;
    CVector dofs;

    if ( ent->flags & (FL_BOT | FL_MONSTER) )
    {
        //	bots are always right handed
        //	FIXME:	check for ducking bots!!
        return(fpos + hpos);
    }
    else
    {

/*
        // determine horizontal ofs from "hand" cavar
        ent->client->pers.hand=gstate->GetCvar("hand");

        if ( ent->client->pers.hand==LEFT_HANDED )
            hpos=-hpos;
        else
            if ( ent->client->pers.hand==CENTER_HANDED )
        {
            CVector temp( 0, 0, -4 );

            hpos = temp;
            fpos = temp;
        }
        */

        // determine vertical ofs from DUCK flag
        if ( ent->flags & ( FL_CLIENT | FL_BOT ) )
            player=ent;
        else 
        if ( ent->owner && ent->owner->flags & FL_CLIENT )
            player=ent->owner;
        //   else
        //      gstate->Con_Printf("weaponHandVector(): can't determine DUCK state from %s\n",ent->className);

        if ( player && player->client->ps.pmove.pm_flags & PMF_DUCKED )
            dofs=CVector(0,0,25);
        else
            dofs=zero_vector;

        return(fpos+hpos-dofs);
    }
}

short daikatana_track(trackInfo_t &tinfo, userEntity_t *self);
void daikatana_ambient(userEntity_t *self);
void sunflareAddFlame(userEntity_t *self);
void selectWeaponTrack(userEntity_t *self)
{
	if (!self || !self->client || !self->client->ps.weapon || !self->winfo)
		return;

	weaponInfo_t *winfo = (weaponInfo_t *)self->winfo;
	userEntity_t *ent = (userEntity_t *)self->client->ps.weapon;

    trackInfo_t tinfo;
	memset(&tinfo,0,sizeof(tinfo));
	// send 'track entity' msg to client
	// for now, send msg every time the weapon is selected, so demos will display weapons
	// daikatana does this stuff by itself...don't want it getting funked up...
	if (stricmp(winfo->weaponName,"weapon_daikatana"))
	{
		tinfo.ent=ent;
		tinfo.srcent=self;
		tinfo.extra=self;
		tinfo.fru.Zero();
		tinfo.dstent=0;
		tinfo.flags=TEF_SRCINDEX|TEF_FRU|TEF_EXTRAINDEX|TEF_FXFLAGS;
		tinfo.renderfx=RF_MINLIGHT|RF_WEAPONMODEL|RF_DEPTHHACK|RF_3P_NODRAW;
		if(!stricmp(winfo->weaponName, "weapon_gashands"))
			tinfo.renderfx |= RF_SHAKE;
//		com->trackEntity(&tinfo,MULTICAST_ALL);

		if (!stricmp(winfo->weaponName,"weapon_sunflare"))
		{
			sunflareAddFlame(ent);
		}
	}
	else
	{
		if (daikatana_track(tinfo,self))
		{
			com->trackEntity(&tinfo,MULTICAST_ALL_R);
		}
		daikatana_ambient(self);
	}
	
}

///////////////////////////////////////////////////////////////////////////////
//	InitClientWeaponModel()
//
//	sets the correct 3rd person weapon for a player model based on self->curWeapon
///////////////////////////////////////////////////////////////////////////////

void	InitClientWeaponModel (userEntity_t *self)
{
	weaponInfo_t	*winfo = NULL;

	int modelindex2 = self->s.modelindex2 & 0xffff0000;
//	modelindex2 &= 0Xff00;

	if (self->winfo)
	{
		selectWeaponTrack(self);

		winfo = (weaponInfo_t *) self->winfo;
		// SCG: 3-2-99
		// the disruptor glove does not have a client weapon model 
		// FIXME: could we avoid this by setting winfo->models[MDL_ARTIFACT_STD]
		// to NULL in the disruptor ini code?
		// the daikatana is drawn by a trackent.
		if( !stricmp( winfo->weaponName, "weapon_daikatana" ) || 
			!stricmp(winfo->weaponName,"weapon_disruptor") ||
			!stricmp(winfo->weaponName,"weapon_gashands") ||
			!stricmp( winfo->weaponName, "weapon_silverclaw" ) )
		{
			self->s.modelindex2 = modelindex2;
		}
		else
		{
			// set modelindex2... it is the model index for the weapon model
			self->s.modelindex2 = modelindex2 + gstate->ModelIndex(winfo->models[MDL_ARTIFACT_STD]);
		}
	}
	else
	{
		// no model should be drawn
		self->s.modelindex2 = modelindex2;
	}
}

DllExport void dll_weapon_reinit_3p_models(userEntity_t *self)
{
	return;

	if (!(deathmatch->value || coop->value))
		return;

	userEntity_t *ent;
	for (int c = 0; c < maxclients->value; c++)
	{
		ent = &gstate->g_edicts [c + 1];
		if (!ent->inuse)
			continue;

		InitClientWeaponModel(ent);
	}
}

//---------------------------------------------------------------------------
// selectWeapon()
//---------------------------------------------------------------------------
userEntity_t *selectWeapon(userEntity_t *owner,char *weaponName,char *modelName, byte enable_crosshair)
{
    weaponInfo_t *winfo=(weaponInfo_t *)owner->winfo;
    playerHook_t *phook=(playerHook_t *)owner->userHook;
    trackInfo_t tinfo;
    userEntity_t *ent;
    userInventory_t *inv;
    CVector vec,dir;
    short weapon_spawned=false;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    //	Nelno:	don't allow selecting a weapon when dead or in a warp
    if ( owner->flags & FL_INWARP || owner->deadflag != DEAD_NO )
        return	NULL;

    // set owner's current weapon
    if ( !(inv=gstate->InventoryFindItem(owner->inventory,weaponName)) )
        return(NULL);

    owner->curWeapon=inv;

	// SCG[10/24/99]: The following only needs to be done for clients
	if( owner->flags & ( FL_CLIENT ) )// | FL_BOT ) )
	{
		// spawn weapon entity (if not already there)
		if ( !owner->client->ps.weapon )
		{
			owner->client->ps.weapon=gstate->SpawnEntity();
			((userEntity_t *)owner->client->ps.weapon)->flags |= FL_NOSAVE;
			weapon_spawned=true;
		}

		// setup weapon entity
		ent=(userEntity_t *)owner->client->ps.weapon;
		_ASSERTE(ent->flags & FL_NOSAVE);
		ent->owner = owner;
		ent->movetype = MOVETYPE_NONE;
		ent->solid = SOLID_NOT;
		ent->className = weaponName;
		ent->s.effects=256;
		ent->s.modelindex=gstate->ModelIndex(modelName);
		ent->svflags = SVF_SHOT;         //SVF_OWNERONLY|SVF_SHOT;                      // only send to owner's machine
		ent->s.renderfx &= ~(RF_ENVMAP|RF_NODRAW);
		ent->s.renderfx |= RF_TRACKENT|RF_WEAPONMODEL|RF_DEPTHHACK;               // special draw
		if(!stricmp(weaponName, "weapon_gashands"))
			ent->s.renderfx |= RF_SHAKE;
		else
			ent->s.renderfx &= ~RF_SHAKE;
		ent->winfo=winfo;
		ent->EntityType = winfo->EntityType;
		ent->s.modelindex2 = owner->s.number + 10000;		// hacky hacky!  so we can tag which weapon model belongs to who
		owner->s.modelindex2 = ent->s.number << 16;

		if (!stricmp(weaponName, "weapon_daikatana"))
			ent->s.renderfx &= ~RF_DEPTHHACK;

		//   winfo->lastFired=0;           // init last time weapon was fired

		// not sure if this is needed or not ... only referenced in world/client.cpp
		ent->flags &= ~FL_LOOPAMBIENT_ANIM;
		if ( winfo && (winfo->flags & WFL_LOOPAMBIENT_ANIM) )
			ent->flags |= FL_LOOPAMBIENT_ANIM;

		ent->prethink=NULL;

		ent->s.origin = owner->s.origin;
		gstate->LinkEntity(ent);     
		if ( !weapon_spawned )
			RELIABLE_UNTRACK(ent);

		ent->think=weaponEntThink;
		ent->nextthink=gstate->time+.1;
		ent->remove=weaponEntRemove;

/*
		// send 'track entity' msg to client
		// for now, send msg every time the weapon is selected, so demos will display weapons
		// daikatana does this stuff by itself...don't want it getting funked up...
		if (stricmp(weaponName,"weapon_daikatana"))
		{
			tinfo.ent=ent;
			tinfo.srcent=owner;
			//tinfo.extra=phook->weapon;
			tinfo.extra=owner;
			tinfo.fru.Zero();
			tinfo.dstent=0;
			tinfo.flags=TEF_SRCINDEX|TEF_FRU|TEF_EXTRAINDEX|TEF_FXFLAGS;
		//		tinfo.fxflags=TEF_HR_MUZZLE_POS;
			tinfo.renderfx=RF_MINLIGHT|RF_WEAPONMODEL|RF_DEPTHHACK|RF_3P_NODRAW;
			if(!stricmp(weaponName, "weapon_gashands"))
				tinfo.renderfx |= RF_SHAKE;
			com->trackEntity(&tinfo,MULTICAST_ALL);
		}
*/
		if(!stricmp(weaponName, "weapon_daikatana"))
		{
			ent->s.renderfx |= RF_ENVMAP;
		}
		else
		{
			switch( gstate->episode )
			{
			case 1:
				if( !stricmp( weaponName, "weapon_gashands" ) ) { ent->s.renderfx |= RF_ENVMAP; }
				else if( !stricmp( weaponName, "weapon_disruptor" ) ) { ent->s.renderfx |= RF_ENVMAP; }
				else if( !stricmp( weaponName, "weapon_shockwave" ) ) { ent->s.renderfx |= RF_ENVMAP; }
				else if( !stricmp( weaponName, "weapon_c4")) { ent->s.renderfx |= RF_ENVMAP; }
				else if( !stricmp( weaponName, "weapon_shotcycler" ) ) { ent->s.renderfx |= RF_ENVMAP; }
				break;
			case 2:
				if( !stricmp( weaponName, "weapon_trident" ) ) { ent->s.renderfx |= RF_ENVMAP; }
				else if( !stricmp( weaponName, "weapon_zeus" ) ) { ent->s.renderfx |= RF_ENVMAP; }
				else if( !stricmp( weaponName, "weapon_hammer" ) ) { ent->s.renderfx |= RF_ENVMAP; }
				else if( !stricmp( weaponName, "weapon_discus" ) ) { ent->s.renderfx |= RF_ENVMAP; }
				break;
			case 3:
				if( !stricmp( weaponName, "weapon_silverclaw" ) ) { ent->s.renderfx |= RF_ENVMAP; }
				else if( !stricmp( weaponName, "weapon_wyndrax" ) ) { ent->s.renderfx |= RF_ENVMAP; }
				break;
			case 4:
				ent->s.renderfx |= RF_ENVMAP;
/*
				if(!stricmp(weaponName, "weapon_glock")) { ent->s.renderfx |= RF_ENVMAP; }
				else if(!stricmp(weaponName, "weapon_ripgun")) { ent->s.renderfx |= RF_ENVMAP; }
				else if(!stricmp(weaponName, "weapon_slugger")) { ent->s.renderfx |= RF_ENVMAP; }
				else if(!stricmp(weaponName, "weapon_kineticore")) { ent->s.renderfx |= RF_ENVMAP; }
				else if(!stricmp(weaponName, "weapon_novabeam")) { ent->s.renderfx |= RF_ENVMAP; }
				else if(!stricmp(weaponName, "weapon_metamaser")) { ent->s.renderfx |= RF_ENVMAP; }
*/
				break;
			}
		}

		ent->s.render_scale.Set(1,1,1);

		// init ambient_time
		phook->ambient_time=gstate->time+2.0+(rnd()*6);

		// SCG[10/4/99]: enable/disable the crosshair
		gstate->WriteByte( SVC_CROSSHAIR );
		gstate->WriteByte( enable_crosshair );
		gstate->UniCast( owner, true );

	}
	else 
	{
		ent = NULL;
	}

	// set up the 3rd person model
	InitClientWeaponModel (owner);

    return(ent);
}

//---------------------------------------------------------------------------
// weaponEntRemove()
//---------------------------------------------------------------------------
void weaponEntRemove(userEntity_t *self)
{
	RELIABLE_UNTRACK(self);
    gstate->RemoveEntity(self);
}

//---------------------------------------------------------------------------
// weaponEntThink()
//---------------------------------------------------------------------------
void weaponEntThink(userEntity_t *self)
{
	if (!self || !self->owner)
		return;

    playerHook_t    *hook=(playerHook_t *)self->owner->userHook;
    weaponInfo_t    *winfo;

	if ( !hook )
	{
		return;
	}

    short oldFrame;

    // update position
    self->s.origin = self->owner->s.origin;
    self->s.old_origin = self->owner->s.old_origin;

    // update animation
    oldFrame=self->s.frame;
    frameUpdate(self);

    // weapons call special function on specific frame
    if ( (hook->fxFrameFunc) && (oldFrame<=hook->fxFrameNum) && (self->s.frame>=hook->fxFrameNum) )
    {
	    hook->fxFrameFunc(self->owner);
	}


    // previous requesting to change the weapon denied            3.5  dsn
    if (self && self->s.frameInfo.frameFlags & FRAME_WEAPONSWITCH2 &&
        self->s.frameInfo.frameFlags & FRAME_STATIC)
    {
		self->s.frameInfo.frameFlags &= ~FRAME_WEAPONSWITCH2;
		if (self->owner)
			weaponAutoChange(self->owner);
    }

	weapon_t *weap=(weapon_t *)self->curWeapon;
	winfo = (weaponInfo_t *)self->winfo;
	if (self && self->owner && self->owner->flags & FL_CLIENT)
	{
		float autosw = atof(Info_ValueForKey(self->owner->client->pers.userinfo, "cl_autowweaponswitch"));
		if (weap && winfo && (autosw > 0) && (weap->ammo->count < winfo->ammo_per_use))
		{
			weaponAutoChange(self->owner);
		}
	}

    self->nextthink=gstate->time+.1;

    // call custom think functions, after everything else so the custom
    // think can use and/or override anything set here.

	if (winfo)
	{
		if (winfo->wentThink)
			winfo->wentThink(self);
	}
}

qboolean validEntity(userEntity_t *self)
{
	if (!self)
		return FALSE;

	int num = self - gstate->g_edicts;
	if ( (num < 0) || (num > gstate->game->maxentities) )
		return FALSE;
	else
		return TRUE;
}
//---------------------------------------------------------------------------
// frameUpdate()
//---------------------------------------------------------------------------
void frameUpdate(userEntity_t *self)
{
	if (!self || !self->inuse || !stricmp(self->className,"freed") || !validEntity(self->owner) || !self->owner->userHook || !EntIsAlive(self->owner))
		return;

    weaponInfo_t *winfo=(weaponInfo_t *)self->winfo;
    playerHook_t *hook;

    // update animation
    com->FrameUpdate(self);

    // anim finished?
    if ( self->s.frameInfo.frameState & FRSTATE_STOPPED )
    {
        // we're done...
        self->s.frameInfo.frameFlags |= FRAME_STATIC;
        self->s.frameInfo.frameFlags &= ~FRAME_ONCE;
        if ( (hook = (playerHook_t *)self->owner->userHook) && (hook->fxFrameFunc) && (hook->fxFrameNum == -1) )
        {
	        hook->fxFrameFunc(self->owner);
        }


        weaponHasAmmo(self->owner,(winfo->flags & WFL_FORCE_SWITCH)); // 3.5 dsn // i would put these here, but it interrupts selection of weapons

        // switching weapons?
        if ( self->s.frameInfo.frameFlags & FRAME_WEAPONSWITCH )
        {
            playerHook_t *hook=(playerHook_t *)self->owner->userHook;

            hook->weapon_next(self->owner);
            self->s.frameInfo.frameFlags &= ~FRAME_WEAPONSWITCH;
        }
        else if ( winfo && winfo->flags & WFL_LOOPAMBIENT_ANIM )      // a FRAME_ONCE anim is finished, loop ambient?
        {
            weapon_t *weap=(weapon_t *)self->owner->curWeapon;

            if ( weap )
                weap->command((userInventory_t *)weap->ammo,"ambient",self->owner);
        }
    }

	// yeah, do this all over again cuz weaponHasAmmo can make it break stuff
	if (!self || !stricmp(self->className,"freed") || !validEntity(self->owner) || !self->owner->userHook)
		return;
    // Logic[8/4/99]: 
    // In cases where we specify hook->fxFrameNum == -2, we want to run hook->fxFrameNum() 
    // halfway through the animation. Rather than making animations code-bound, animations
    // and their corresponding routines are data bound.
    if( (hook = (playerHook_t *)self->owner->userHook) && 
        (hook->fxFrameFunc) && 
        (hook->fxFrameNum == -2) && 
        (self->s.frame - self->s.frameInfo.startFrame  > ((self->s.frameInfo.endFrame - self->s.frameInfo.startFrame) * 0.5)) )
    {
        // half anim
        hook->fxFrameFunc(self->owner);
    }
    // must    ....     clear   ..... flags ....
    self->s.frameInfo.frameState &= ~(FRSTATE_STOPPED|FRSTATE_LAST);
}

//---------------------------------------------------------------------------
// weapon_animate() - still used by the older weapons ... will remove later!!
//---------------------------------------------------------------------------
void weapon_animate (userEntity_t *self, int start, int end, float frametime)
{
//    playerHook_t *hook=(playerHook_t *)self->userHook;// SCG[1/24/00]: not used
    userEntity_t *weapon;
    userEntity_t *went = (userEntity_t *) self->client->ps.weapon;

    weapon=(userEntity_t *)self->client->ps.weapon;
    if ( !weapon )
        return;

    //	no weapon frames for bots or monsters
    if ( !(self->flags & FL_CLIENT ) )
        return;

    went->s.frameInfo.startFrame = start;
    went->s.frameInfo.endFrame = end;
    went->s.frameInfo.frameTime = frametime;
    went->s.frameInfo.next_frameTime = 0;
    went->s.frameInfo.frameFlags = FRAME_ONCE;      // | FRAME_FORCEINDEX;	//	force frame change to be sent
    went->s.frameInfo.frameInc = 1;
    weapon->s.frame=start;
}

//---------------------------------------------------------------------------
// entAnimate()
//---------------------------------------------------------------------------
void entAnimate(userEntity_t *ent,short first,short last,unsigned short frameflags,float frametime)
{
    ent->s.frame=first;
    ent->s.frameInfo.startFrame=first;
    ent->s.frameInfo.endFrame=last;
    ent->s.frameInfo.frameFlags=frameflags | FRAME_FORCEINDEX;
    ent->s.frameInfo.frameInc=1;
    ent->s.frameInfo.frameTime=frametime;
    ent->s.frameInfo.frameState=FRSTATE_STARTANIM;
    ent->s.frameInfo.next_frameTime=0;
}

//---------------------------------------------------------------------------
// winfoAnimate()
//---------------------------------------------------------------------------
void winfoAnimate(weaponInfo_t *winfo,userEntity_t *ent,short anim,unsigned short frameflags,float frametime)
{
    float fFrameTime;
    if ( !ent )
        return;

    fFrameTime = frametime;
    if ( frameflags & FRAME_WEAPON )
    {
        playerHook_t *hook=(playerHook_t *)ent->userHook;
        // if FRAME_WEAPON, ent is the player

        if ( !(frameflags & FRAME_NODELAY) )
        {
            if(hook->attack_boost)
            {
				float add = (hook->attack_boost == 1) ? 1.5 : 1.0;	// make it so one boost has an effect...
                fFrameTime = fFrameTime / ((hook->attack_boost+add) * 0.5);
                hook->attack_finished = gstate->time + (winfoNumFrames(anim)*fFrameTime) + 0.1;
            }
            else
                hook->attack_finished = gstate->time + winfoNumFrames(anim)*fFrameTime + 0.1;
        }

        //	no weapon frames for bots or monsters
        if ( !(ent->flags & FL_CLIENT) )
            return;

        // now, ent is the weapon
        ent=(userEntity_t *)ent->client->ps.weapon;
    }

    frameflags &= ~(FRAME_WEAPON | FRAME_NODELAY);
    entAnimate(ent,winfoFirstFrame(anim),winfoLastFrame(anim),frameflags,fFrameTime);
}

//---------------------------------------------------------------------------
// unlimitedAmmo()
//---------------------------------------------------------------------------
short unlimitedAmmo(void)
{
	if (deathmatch->value)
	{
		return (gstate->GetCvar("dm_infinite_ammo") > 0.0f) ? 1 : 0;		
	}

    short unlimited_ammo;

    // cvar check is for 'runtime', #define is for 'builds' (like E3)
    // g_unlimted_ammo == -1, ignores #define
     if ( (unlimited_ammo=(short)gstate->GetCvar("g_unlimited_ammo")) == -1 )
        return(0);
    else
        unlimited_ammo |= WEAPON_UNLIMITED_AMMO;

    return(unlimited_ammo);
}

//---------------------------------------------------------------------------
// weaponHasAmmo()
//
//	returns amount of ammo if weapon has enough ammo for one shot 
//	(winfo->ammo_per_use) or 0 if it does not have enough ammo
//---------------------------------------------------------------------------
short   weaponHasAmmo(userEntity_t *self,short autochange,short max_shots)     // 3.5 dsn  added 'max_shots' - default is 1
{
    userInventory_t *weapon;
    weapon_t        *weap;
    weaponInfo_t    *winfo;

    if(!self)
        return 0;
    if ( !self->curWeapon )
        return(0);

    weapon=self->curWeapon;
    weap=(weapon_t *)weapon;
    winfo=(weaponInfo_t *)weap->winfo;

    if ( IS_INSTAGIB || (deathmatch->value && dm_infinite_ammo->value ))
        return 32767;

    // unlimited ammo?
    if ( unlimitedAmmo() )
        return	32767;

    if ( weap->ammo->count < (winfo->ammo_per_use * max_shots) )  // max shots defaults to 1, except in special cases
    {
        if ( autochange )
            weaponAutoChange(self);
        return	0;
    }

    return	weap->ammo->count;
}

void weaponAmmoCountUpdate(userEntity_t *self, ammo_t *ammo)
{
	if (!(self->flags & FL_CLIENT) || !ammo || !ammo->winfo)
		return;

	// if this bullshit happens, just tell the client to update the weapon display!
	if (!gstate->gi->TestUnicast(self,false))
	{
		WEAPON_DISPLAY_UPDATE(self);
		return;
	}

	weaponInfo_t *winfo = ammo->winfo;
	gstate->WriteByte(SVC_AMMO_COUNT_UPDATE);
	gstate->WriteByte(winfo->display_order);
	if (!strcmpi(ammo->name,"ammo_none"))  // melee weapon?
		gstate->WriteShort(-1);                                  // doesn't use ammo
	else
		gstate->WriteShort((short)ammo->count);          // ammo count

	gstate->UniCast(self,false);

}

//---------------------------------------------------------------------------
// weaponUseAmmo()
//
//	subtracts winfo->ammo_per_use from a self->curWeapon's ammo supply
//	if < ammo_per_use ammo is left and autochange == true, weapon is auto-changed
//	always returns 0 if there is not enough ammo to fire a shot, otherwise
//	it returns the ammount of ammo left after firing a single shot
//---------------------------------------------------------------------------
short   weaponUseAmmo(userEntity_t *self,short autochange)
{
    if ( !self->curWeapon )
        return(0);

    userInventory_t *weapon=self->curWeapon;
    weapon_t        *weap=(weapon_t *)weapon;
    weaponInfo_t    *winfo = (weaponInfo_t *) weap->winfo;
//    int             count;

    if ( IS_INSTAGIB || (deathmatch->value && dm_infinite_ammo->value ))
        return 32767;

    // unlimited ammo?
    if ( unlimitedAmmo() )
        return	32767;
    
    // assume this weapon has enough ammo to USE!  (because the switch weapon check occurs after the weapon
    // has started animating its fire frames   3.5 dsn  
    
    ammoGiveTake_t take;
	take.count = winfo->ammo_per_use;
	take.owner = self;
//    count = winfo->ammo_per_use;
    
    return (short)(weap->ammo->command ((userInventory_t *) weap->ammo, "take_ammo", &take));
}

//---------------------------------------------------------------------------
// weaponAutoChange()
//---------------------------------------------------------------------------

#define	MAX_WEAPONS	32
void weaponAutoChange(userEntity_t *self)
{
    weapon_t        *weapon = (weapon_t *) self->curWeapon;
    weapon_t        *prev, *next, *new_weapon;
    weapon_t        *weaponList [MAX_WEAPONS];
    weaponInfo_t    *winfo;
    int             best_weapon, best_rating, rating;
    int             cur_rating, best_lower_rating, best_lower_weapon;
    userInventory_t *item;
    int             total_weapons, i, ammo_count;
    //float           wp_change_order;

	if (!EntIsAlive(self))
		return;

	// SCG[1/24/00]: clear out weaponList..
	for( i = 0; i < MAX_WEAPONS; i++ )
	{
		weaponList[i] = NULL;
	}

    //	get a list of all the weapons in the player's inventory
    //	excluding their current weapon
    total_weapons  = 0;
    item = gstate->InventoryFirstItem (self->inventory);

    while ( item && total_weapons < MAX_WEAPONS )
    {
        if ( item->name && strstr (item->name, "weapon_") )
        {
            if ( stricmp (item->name, self->curWeapon->name) )
            {
                //	exclude current weapon
                weaponList [total_weapons] = (weapon_t *) item;
                total_weapons++;
            }
        }

        item = gstate->InventoryNextItem (self->inventory);
    }

    //	find the best weapon in the list
    //	also find the weapon with the highest rating below that of the
    //	current weapon
    best_rating = best_weapon = -1;
    best_lower_rating = best_lower_weapon = -1;
    weapon->command ((userInventory_t *) weapon, "rating", &cur_rating);
    for ( i = 0; i < total_weapons; i++ )
    {
        weaponList [i]->command ((userInventory_t *) weaponList [i], "rating", &rating);
        ammo_count = (int)weaponList [i]->ammo->command ((userInventory_t *) weaponList [i]->ammo, "count", &ammo_count);
        winfo = (weaponInfo_t *) weaponList [i]->winfo;

        if ( rating > best_rating && ((ammo_count >= winfo->ammo_per_use) || (winfo->flags & WFL_SELECT_EMPTY))  )
        {
            best_weapon = i;
            best_rating = rating;
        }

        if ( rating < cur_rating && rating > best_lower_rating  && ((ammo_count >= winfo->ammo_per_use) || (winfo->flags & WFL_SELECT_EMPTY)) )
        {
            best_lower_weapon = i;
            best_lower_rating = rating;
        }
    }

    if ( best_weapon >= 0 )
        next = weaponList [best_weapon];
    else
        next = NULL;

    if ( best_lower_weapon >= 0 )
        prev = weaponList [best_lower_weapon];
    else
        prev = NULL;

    new_weapon = NULL;

    new_weapon = prev ? prev : next;

	// [cek 11-26-99] This circumvents the 'normal' weapon select procedure.  Call the weapon's select,
	// which by default calls weaponsSelect...
    if ( new_weapon )
		(weaponInfo_t *)new_weapon->winfo->select_func(self);
//        weaponSelect (self, (weaponInfo_t *) new_weapon->winfo);
}

//---------------------------------------------------------------------------
// weaponGive()
//---------------------------------------------------------------------------
userInventory_t *weaponGive(userEntity_t *self,weaponInfo_t *winfo,short ammoCount)
{
	weapon_t		*weapon;
	userInventory_t	*invItem;
	playerHook_t	*phook=(playerHook_t *)self->userHook;

	// when the weapon_give function is called by the console command, ammoCount is invalid
	// checks for 'out of range' here.
	if( ammoCount < 0 || ammoCount > 500 )
	{
		ammoCount = 0;
	}

    //	Nelno:	don't giving a weapon when dead or in a warp
    if( self->flags & FL_INWARP || self->deadflag != DEAD_NO )
	{
        return	NULL;
	}

	// make sure a sidekick can't steal the player's weapons!
	if ((self->flags & FL_BOT) && ((winfo->flags & WFL_PLAYER_ONLY) || (phook->type == TYPE_MIKIKOFLY)))
		return NULL;

    // make sure weapon is not already in inventory	
    if( invItem = gstate->InventoryFindItem( self->inventory, winfo->weaponName ) )
    {
        //	just get the ammo from the weapon if we already have it
        weapon				= ( weapon_t * ) invItem;
        weapon->ammo		= ammoAdd ( self, ammoCount, winfo );
	    weapon->modelIndex	= gstate->ModelIndex( winfoModel( MDL_ARTIFACT_STD ) );

//		gstate->WeaponDisplayUpdateClient( self, true );  // 7.20
		WEAPON_DISPLAY_UPDATE(self);

		return( invItem );
	}

    // create new inventory item
    if( !( weapon =weaponCreateInv( self->inventory, winfo, winfo->use_func, winfo->cmd_func, winfo->weapon_t_size) ) )
    {
        gstate->Con_Printf( "Unable to malloc %s.\n", winfo->weaponNetName );
        return( NULL );
    }

    // add the weapon to the player's inventory list
    if( !gstate->InventoryAddItem( self, self->inventory, ( userInventory_t * ) weapon ) )
    {
        gstate->Con_Printf( "Could not add %s to inventory.\n",winfo->weaponNetName );
        return( NULL );
    }

//    weapon->flags		= ITF_WEAPON; //	set item flag
//    weapon->winfo		= winfo;
    weapon->ammo		= ammoAdd( self, ammoCount, winfo );           // give ammo with weapon
    phook->ambient_time	= gstate->time + 2.0 + ( rnd() * 6 );       // starting ambient time

//	gstate->WeaponDisplayUpdateClient( self, true );  // 7.20
	WEAPON_DISPLAY_UPDATE(self);

    return((userInventory_t *)weapon);
}

//---------------------------------------------------------------------------
// weaponSelect() - returns true if new weapon is active weapon
// returns: WEAPON_ACTIVE - weapon is already active
//          WEAPON_CHANGED - weapon is being changed
//          WEAPON_UNAVAILABLE - weapon not in inventory
//          WEAPON_ANIMATING - can't change due to animation
//---------------------------------------------------------------------------
short weaponSelect(userEntity_t *self,weaponInfo_t *new_winfo)
{
    playerHook_t    *hook=(playerHook_t *)self->userHook;
    weaponInfo_t    *winfo=NULL;                // current weapon's info -- new_winfo is new weapon's info
    weapon_t        *weapon;
    userInventory_t *invItem;
    userEntity_t    *went = NULL;
    short           useSameModel=false;
    int             ammo_count;

    // SCG[10/24/99]: Only check self->client.ps.weapon for client entities since nothing
	// SCG[10/24/99]: else has a client....
	if( self->flags & ( FL_CLIENT))// | FL_BOT ) )
	{
		went = (userEntity_t *) self->client->ps.weapon;

		// if(went && went->EntityType ==  WEAPON_GASHANDS)
		//        return(WEAPON_UNAVAILABLE);

		// I had a crash here on restarting a level, so I added a check for a NULL hook --zjb, 082099
		if ( !hook )
		{
			return(WEAPON_UNAVAILABLE);
		}

		// no changing weapons while it's attack     // modified 3.5 dsn
		if ( hook->attack_finished >= gstate->time )
		{
			if ( !(invItem = gstate->InventoryFindItem(self->inventory,new_winfo->weaponName)) )
				return(WEAPON_UNAVAILABLE);

			if (invItem == self->curWeapon )
				return(WEAPON_ACTIVE);


			// trying to check for a weapon change to an ammoless weapon
			if ( weapon=(weapon_t *)self->curWeapon )
				weapon->command(self->curWeapon,"winfo",&winfo);

			// get ammo_count of our reserved weapon slot, of the the NEW weapon we want to change to
			invItem->command( (userInventory_t *) invItem, "ammo_count", &ammo_count);

			// do we have ammo for the new weapon to switch to?
			if (weapon && new_winfo->ammo_per_use > 0)
			{ 
				// avoid this if WFL_SELECT_EMPTY is set cek[11-26-99]
				if ((ammo_count < new_winfo->ammo_per_use) && !(new_winfo->flags & WFL_SELECT_EMPTY) )
				{
					weapon_no_ammo_alert(self);  // no ammo message/sound

					return (WEAPON_UNAVAILABLE);
				}
			}

			// Logic[4/30/99]: This forces an autoswitch, not what we want..
			// set flag to inform recall when animating is complete
			// if(went)
			//    went->s.frameInfo.frameFlags |= FRAME_WEAPONSWITCH2;
			// cek[2-25-00]
			if ( !weapon || !(weapon->ammo->count < winfo->ammo_per_use))
				return(WEAPON_ANIMATING);
		}


		if ( !(invItem = gstate->InventoryFindItem(self->inventory,new_winfo->weaponName)) )
		{
			weapon_no_ammo_alert(self);  // no ammo message/sound

			return(WEAPON_UNAVAILABLE);
		}

		// get pointer to weaponInfo_t
		if ( weapon=(weapon_t *)self->curWeapon )
			weapon->command(self->curWeapon,"winfo",&winfo);
 
		// 3.5 dsn
		// trying to check for a weapon change to an ammoless weapon
      
		// get ammo_count of our reserved weapon slot, of the the NEW weapon we want to change to
		invItem->command( (userInventory_t *) invItem, "ammo_count", &ammo_count);

		// do we have ammo for the new weapon to switch to?
		if (weapon && new_winfo->ammo_per_use > 0)
		{ 
			if ((ammo_count < new_winfo->ammo_per_use) && !(new_winfo->flags & WFL_SELECT_EMPTY) )
			{
				weapon_no_ammo_alert(self);  // no ammo message/sound

				return (WEAPON_UNAVAILABLE);
			}
		}
    
		// does new weapon and old weapon use the same model? (ie: ripgun, daikatana)
		// if there's 'overlapping bits of SAME_WEAPON_MASK', both use the same weapon model
		if ( winfo && (winfo->flags & WFL_SAME_WEAPON_MASK) & (new_winfo->flags & WFL_SAME_WEAPON_MASK) )
			useSameModel=true;

		// save winfo pointers
		self->winfo = new_winfo;
		self->oldWinfo = winfo;

		// change weapon?
		if ( !useSameModel && weapon && went )
		{
			if ( invItem==self->curWeapon )
				return(WEAPON_ACTIVE);

			if ( winfo )              // testweapon doesn't have winfo
			{
				// some weapons need to know when they're being deselected
				weapon->command(self->curWeapon,"change",self);

				// special time calc... adds 1 sec to delay      
				hook->attack_finished=gstate->time+winfoNumFrames(ANIM_AWAY_STD)*.05;

				if ( winfoSound(SND_AWAY_STD) != SND_NOSOUND )
				winfoPlay(self,SND_AWAY_STD,1.0f);
			}

		    userInventory_t *curWeapon = self->curWeapon;
			int c_ammo_count = 0;
			if (curWeapon)
				curWeapon->command( (userInventory_t *) curWeapon, "ammo_count", &c_ammo_count);

			bool shouldSwitchFast = ((deathmatch->value) && (dm_fast_switch->value)) || (self->flags & FL_BOT) || 
									(!c_ammo_count && (!stricmp(winfo->weaponName,"weapon_discus")));

			//	make sure the client's frame flags tell us to go to next weapon -- bots always do the immediate thing
			if (shouldSwitchFast)
			{
				weaponSelectFinish(self);
			}
			else 
			{
				went->s.frameInfo.frameFlags |= FRAME_WEAPONSWITCH;

				//	set client's next weapon to this weapon
				hook->weapon_next=weaponSelectFinish;
			}
		}
		else
			weaponSelectFinish(self);

	}
	else if( self->flags & FL_BOT )
	{
		// save winfo pointers
		self->winfo = new_winfo;
		self->oldWinfo = winfo;
		weaponSelectFinish(self);
	}

	return(WEAPON_CHANGED);
}

void weaponSelectReady(userEntity_t *self)
{
    userEntity_t *player = self->owner;
	if (!player)
		return;

	weaponInfo_t *winfo=(weaponInfo_t *)player->winfo;
    playerHook_t *phook=(playerHook_t *)player->userHook;

	if (!winfo || !phook)
		return;

	self->s.renderfx &= ~RF_NODRAW;
//	self->s.frame = winfoFirstFrame(ANIM_READY_STD);
	self->think = weaponEntThink;
    self->nextthink=gstate->time+.1;

//    winfoAnimate(winfo,player,ANIM_READY_STD,FRAME_ONCE|FRAME_WEAPON,0.05);
//    if ( winfoSound(SND_READY_STD) != SND_NOSOUND )
//		gstate->StartEntitySound(player,CHAN_AUTO,gstate->SoundIndex(winfo->sounds[(short)(SND_READY_STD)]),1.0f,ATTN_NORM_MIN,ATTN_NORM_MAX);

    // ambient start immediately?
    if ( winfo->flags & WFL_LOOPAMBIENT_ANIM )
        phook->ambient_time=gstate->time;

    // start looping ambient sound?
    if ( winfoSound(SND_WEAPON_STD) != SND_NOSOUND )
	{
		player->s.sound = gstate->SoundIndex(winfo->sounds[(short)(SND_WEAPON_STD)]);
		player->s.volume = 1.0;
	}
    else
		winfoStopLooped(player);
}

//---------------------------------------------------------------------------
// weaponSelectFinish()
//---------------------------------------------------------------------------
void weaponSelectFinish(userEntity_t *self)
{
    weaponInfo_t *winfo=(weaponInfo_t *)self->winfo,*oldWinfo=(weaponInfo_t *)self->oldWinfo;
    userEntity_t *went = NULL;
//    playerHook_t *phook=(playerHook_t *)self->userHook;// SCG[1/24/00]: not used
    short useSameModel=false;

    if ( !winfo )
        return;

    if ( !selectWeapon( self, winfo->weaponName, winfoModel( MDL_WEAPON_STD ), ( winfo->flags & WFL_CROSSHAIR ) ? 1 : 0 ) )
	{
        return;
	}

	if( self->flags & (FL_CLIENT))//|FL_BOT) )
	{
	    went = (userEntity_t*)self->client->ps.weapon;
	}

    // does new weapon and old weapon use the same model? (ie: ripgun, daikatana)
    // if there's 'overlapping bits of SAME_WEAPON_MASK', both use the same weapon model
    if ( winfo && oldWinfo && (winfo->flags & WFL_SAME_WEAPON_MASK) & (oldWinfo->flags & WFL_SAME_WEAPON_MASK) )
        useSameModel=true;

    // do ready anim and sound
    if ( !useSameModel )
    {
		if (went)
			went->s.frame = winfoFirstFrame(ANIM_READY_STD);
		winfoAnimate(winfo,self,ANIM_READY_STD,FRAME_ONCE|FRAME_WEAPON,0.05);
		if ( winfoSound(SND_READY_STD) != SND_NOSOUND )
			gstate->StartEntitySound(self,CHAN_AUTO,gstate->SoundIndex(winfo->sounds[(short)(SND_READY_STD)]),1.0f,ATTN_NORM_MIN,ATTN_NORM_MAX);
		if (((deathmatch->value) && (dm_fast_switch->value)) || (self->flags & FL_BOT))
		{
			weaponSelectReady(self);
		}
		else 
		{
			if (went)
			{
				went->s.renderfx |= RF_NODRAW;
//				went->s.frameInfo.frameFlags |= FRAME_WEAPONSWITCH;
				went->think = weaponSelectReady;
				went->nextthink = gstate->time + 0.4;
			}
		}

	}
		
//	gstate->WeaponDisplayUpdateClient(self, true);  // 7.20
	WEAPON_DISPLAY_UPDATE(self);
}


//---------------------------------------------------------------------------
// entityHuman() - better way to determine human from mech???
//---------------------------------------------------------------------------
qboolean entityHuman(userEntity_t *self)
{
    char **ptr;
    char *mechEnts[] = {"monster_sludgeminion",
        "monster_ragemaster",
        "monster_battleboar",
        NULL};

    ptr=mechEnts;
    while ( *ptr )
    {
        if ( !stricmp(self->className,*ptr) )
            return(FALSE);

        ptr++;
    }

    return(TRUE);
}

//---------------------------------------------------------------------------
// winfoClientExplosion()
//---------------------------------------------------------------------------
void winfoClientExplosion(weaponInfo_t *winfo,CVector &pos,CVector &angle,float scale,short model,short anim,short flags,short light,CVector &color)
{
    gstate->WriteByte(SVC_TEMP_ENTITY);
    gstate->WriteByte(TE_EXPLOSION);
    gstate->WritePosition(pos);
    gstate->WriteAngle(angle.x);
    gstate->WriteAngle(angle.y);
    gstate->WriteAngle(angle.z);
    gstate->WriteFloat(scale);
    gstate->WriteByte(winfoNumFrames(anim));
    gstate->WriteLong(gstate->ModelIndex(winfoModel(model)));
    gstate->WriteByte(flags);
    gstate->WriteShort(light);
    if ( light )
        gstate->WritePosition(color);
    gstate->MultiCast(pos,MULTICAST_PVS);
}

//---------------------------------------------------------------------------
// winfoClientScaleImage()
//---------------------------------------------------------------------------
void winfoClientScaleImage(weaponInfo_t *winfo,CVector &pos,CVector &angle,float scale_start,float scale_end,short model,float alpha,float killtime)
{
    gstate->WriteByte(SVC_TEMP_ENTITY);
    gstate->WriteByte(TE_SCALEIMAGE);
    gstate->WritePosition(pos);
    gstate->WritePosition(angle);
    gstate->WriteFloat(scale_start);
    gstate->WriteFloat(scale_end);
    gstate->WriteShort(gstate->ModelIndex(winfoModel(model)));
    gstate->WriteFloat(alpha);
    gstate->WriteFloat(killtime);
    gstate->MultiCast(pos,MULTICAST_PVS);
}

//---------------------------------------------------------------------------
// winfoClientScaleAlpha()
//---------------------------------------------------------------------------
void winfoClientScaleAlpha(weaponInfo_t *winfo,CVector &pos,CVector &angle,float scale,short model,short anim,
                           float alpha_scale,float alpha_end,short light,CVector &color)
{
    gstate->WriteByte(SVC_TEMP_ENTITY);
    gstate->WriteByte(TE_ALPHASCALE);
    gstate->WritePosition(pos);
    gstate->WriteAngle(angle.x);
    gstate->WriteAngle(angle.y);
    gstate->WriteAngle(angle.z);
    gstate->WriteFloat(scale);
    gstate->WriteByte(winfoNumFrames(anim));
    gstate->WriteLong(gstate->ModelIndex(winfoModel(model)));
    gstate->WriteFloat(alpha_scale);
    gstate->WriteFloat(alpha_end);
    gstate->WriteShort(light);
    if ( light )
        gstate->WritePosition(color);
    gstate->MultiCast(pos,MULTICAST_PVS);
}
//---------------------------------------------------------------------------
// winfoClientScaleAlphaRotate()
//---------------------------------------------------------------------------
void winfoClientScaleAlphaRotate(weaponInfo_t *winfo,CVector &pos,CVector &angle,CVector &rot,float scale_start,float scale_end,short model,
								float alpha_start, float dietime)
{
    gstate->WriteByte(SVC_TEMP_ENTITY);
    gstate->WriteByte(TE_ALPHASCALEROTATE);
    gstate->WritePosition(pos);
    gstate->WriteFloat(angle.x);
    gstate->WriteFloat(angle.y);
    gstate->WriteFloat(angle.z);
    gstate->WriteFloat(rot.x);
    gstate->WriteFloat(rot.y);
    gstate->WriteFloat(rot.z);
    gstate->WriteFloat(scale_start);
    gstate->WriteFloat(scale_end);
    gstate->WriteLong(gstate->ModelIndex(winfoModel(model)));
    gstate->WriteFloat(alpha_start);
	gstate->WriteFloat(dietime);
    gstate->MultiCast(pos,MULTICAST_PVS);
}

//---------------------------------------------------------------------------
// winfoClientScaleRotate()
//---------------------------------------------------------------------------
void winfoClientScaleRotate(weaponInfo_t *winfo,CVector &pos,float start_scale,short end_scale,float step_scale,int model)
{
    gstate->WriteByte(SVC_TEMP_ENTITY);
    gstate->WriteByte(TE_SCALEROTATE);
    gstate->WritePosition(pos);
    gstate->WriteFloat(start_scale);
    gstate->WriteFloat(end_scale);
    gstate->WriteFloat(step_scale);
    gstate->WriteLong(gstate->ModelIndex(winfoModel(model)));
    gstate->MultiCast(pos,MULTICAST_PVS);
}

//---------------------------------------------------------------------------
// trackLight()
//---------------------------------------------------------------------------
void trackLight(userEntity_t *ent,float size,float r,float g,float b,int fxflags)
{
    trackInfo_t tinfo;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=ent;

// not sure why i was doing this... but at least some shots (ionblaster) have the player marked as the owner
// and any lights attached to the shot are displayed where the player is
//
//   if (ent->owner)
//      tinfo.srcent=ent->owner;
//   else
    tinfo.srcent=ent;
    tinfo.fru.Zero();
    tinfo.lightColor.Set( r, g, b );
    tinfo.lightSize=size;
    tinfo.flags = TEF_SRCINDEX|TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FRU|TEF_FXFLAGS;
    tinfo.fxflags = fxflags;
    com->trackEntity(&tinfo,MULTICAST_ALL);
}

//---------------------------------------------------------------------------
// trackLightOfs()
//---------------------------------------------------------------------------
void trackLightOfs(userEntity_t *ent,float size,float r,float g,float b,CVector &ofs,int fxflags)
{
    trackInfo_t tinfo;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=ent;

// not sure why i was doing this... but at least some shots (ionblaster) have the player marked as the owner
// and any lights attached to the shot are displayed where the player is
//
//   if (ent->owner)
//      tinfo.srcent=ent->owner;
//   else
    tinfo.srcent=ent;
    tinfo.fru = ofs;
    tinfo.lightColor.Set( r, g, b );
    tinfo.lightSize=size;
    tinfo.flags = TEF_SRCINDEX|TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FRU|TEF_FXFLAGS;
    tinfo.fxflags = fxflags;
    com->trackEntity(&tinfo,MULTICAST_ALL);
}

//---------------------------------------------------------------------------
// untrackLight()
//---------------------------------------------------------------------------
void untrackLight(userEntity_t *ent)
{
	RELIABLE_UNTRACK(ent);
}

//---------------------------------------------------------------------------
// trackAddLight() - add a light to an existing track entity
//---------------------------------------------------------------------------
void trackAddLight(userEntity_t *ent,float size,float r,float g,float b,int fxflags)
{
    trackInfo_t tinfo;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=ent;
// not sure why i was doing this... but at least some shots (ionblaster) have the player marked as the owner
// and any lights attached to the shot are displayed where the player is
//
//   if (ent->owner)
//      tinfo.srcent=ent->owner;
//   else
    tinfo.srcent=ent;
    tinfo.lightColor.Set( r, g, b );
    tinfo.lightSize=size;
    tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS;
    tinfo.fxflags = TEF_ADDFX|fxflags;
    com->trackEntity(&tinfo,MULTICAST_ALL);
}

//---------------------------------------------------------------------------
// trackRemoveLight() - remove a light to an existing track entity
//---------------------------------------------------------------------------
void trackRemoveLight(userEntity_t *ent,int fxflags)
{
    trackInfo_t tinfo;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=ent;
// not sure why i was doing this... but at least some shots (ionblaster) have the player marked as the owner
// and any lights attached to the shot are displayed where the player is
//
//   if (ent->owner)
//      tinfo.srcent=ent->owner;
//   else
    tinfo.srcent=ent;
    tinfo.flags = TEF_FXFLAGS;
    tinfo.fxflags = TEF_REMOVEFX|fxflags;
    com->trackEntity(&tinfo,MULTICAST_ALL);
}

//---------------------------------------------------------------------------
// trackSetAnim()
//---------------------------------------------------------------------------
void trackSetAnim(userEntity_t *ent,int modelindex,short numframes,float scale,int fxflags,float frametime)
{
    trackInfo_t tinfo;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=ent;
// not sure why i was doing this... but at least some shots (ionblaster) have the player marked as the owner
// and any lights attached to the shot are displayed where the player is
//
//   if (ent->owner)
//      tinfo.srcent=ent->owner;
//   else
    tinfo.srcent=ent;
    tinfo.modelindex=modelindex;
    tinfo.numframes=numframes;
    tinfo.scale=scale;
    tinfo.frametime=frametime;
    tinfo.flags = TEF_MODELINDEX|TEF_NUMFRAMES|TEF_SCALE|TEF_FRAMETIME|TEF_FXFLAGS;
    tinfo.fxflags = TEF_ADDFX|fxflags;
    com->trackEntity(&tinfo,MULTICAST_ALL);
}

//---------------------------------------------------------------------------
// trackSetAnim()
//---------------------------------------------------------------------------
void trackSetAnimLocal(userEntity_t *ent,int modelindex,short numframes,float scale,int fxflags,float frametime)
{
    trackInfo_t tinfo;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=ent;
    tinfo.srcent=ent->owner;
    tinfo.modelindex=modelindex;
    tinfo.numframes=numframes;
    tinfo.scale=scale;
    tinfo.frametime=frametime;
    tinfo.flags = TEF_MODELINDEX|TEF_NUMFRAMES|TEF_SCALE|TEF_FRAMETIME|TEF_FXFLAGS;
    tinfo.fxflags = TEF_ADDFX|fxflags;
    com->trackEntity(&tinfo,UNICAST_ENT);
}

//---------------------------------------------------------------------------
// trackAddEffect()
//---------------------------------------------------------------------------
void trackAddEffect(userEntity_t *ent,int modelindex,int fxflags)
{
    trackInfo_t tinfo;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.flags=0;
    tinfo.ent=ent;
// not sure why i was doing this... but at least some shots (ionblaster) have the player marked as the owner
// and any lights attached to the shot are displayed where the player is
//
//   if (ent->owner)
//      tinfo.srcent=ent->owner;
//   else
    tinfo.srcent=ent;
    tinfo.modelindex=modelindex;
    if ( modelindex )
        tinfo.flags |= TEF_MODELINDEX;
    tinfo.flags |= TEF_SRCINDEX|TEF_FXFLAGS;
    tinfo.fxflags=TEF_ADDFX|fxflags;
    com->trackEntity(&tinfo,MULTICAST_ALL);
}

//---------------------------------------------------------------------------
// trackRemoveEffect()
//---------------------------------------------------------------------------
void trackRemoveEffect(userEntity_t *ent,int fxflags)
{
    trackInfo_t tinfo;

    // clear this variable
    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.ent=ent;
// not sure why i was doing this... but at least some shots (ionblaster) have the player marked as the owner
// and any lights attached to the shot are displayed where the player is
//
//   if (ent->owner)
//      tinfo.srcent=ent->owner;
//   else
    tinfo.srcent=ent;
    tinfo.flags = TEF_FXFLAGS;
    tinfo.fxflags = TEF_REMOVEFX|fxflags;
    com->trackEntity(&tinfo,MULTICAST_ALL);
}

//---------------------------------------------------------------------------
// freezeEntity()
//---------------------------------------------------------------------------
void freezeEntity(userEntity_t *ent,freezeEntity_t *freeze)
{
    // save func pointers
    freeze->old_prethink=ent->prethink;
    freeze->old_think=ent->think;
    freeze->old_pain=ent->pain;
    freeze->old_die=ent->die;

    if ( ent->flags & FL_CLIENT )
    {
        // special client shit here
        playerHook_t *hook=(playerHook_t *)ent->userHook;

        gstate->SetCameraState(ent,true,CAMFL_LOCK_X+CAMFL_LOCK_Y+CAMFL_LOCK_Z);
        hook->angles_last_change=gstate->Sys_Milliseconds();
/*        if ( hook->dflags & DFL_HIERARCHICAL )
        {
            if ( hook->legs )
                hook->legs->svflags |= SVF_NOTOWNER;
            if ( hook->torso )
                hook->torso->svflags |= SVF_NOTOWNER;
            if ( hook->head )
                hook->head->svflags |= SVF_NOTOWNER;
            if ( hook->weapon )
                hook->weapon->svflags |= SVF_NOTOWNER;
        }
        else
            ent->s.renderfx |= RF_NODRAW;*/

        ent->flags |= FL_FORCEANGLES;
        ent->viewheight=32;

        freeze->old_movetype=ent->movetype;
        freeze->old_velocity.Zero();
        freeze->old_avelocity.Zero();

        ent->velocity.Zero();
        ent->avelocity.Zero();
        ent->movetype=MOVETYPE_FLY;
    }
    else
    {
        // save old state
//      freeze->old_prethink=ent->prethink;
//      freeze->old_think=ent->think;
//      freeze->old_pain=ent->pain;
//      freeze->old_die=ent->die;
        //freeze->old_movetype=ent->movetype;
        //freeze->old_velocity = ent->velocity;
        //freeze->old_avelocity = ent->avelocity;
		//freeze->old_clipmask = ent->clipmask;

        // set new values
        //ent->pain=NULL;
        //ent->die=NULL;
        //ent->prethink=NULL;
        //ent->think=NULL;
        playerHook_t *hook=(playerHook_t *)ent->userHook;
		if (hook)
			hook->nSpawnValue |= SPAWN_DO_NOT_MOVE;

//	  AI_StartFreeze( ent );         // mike: trouble compiling

        ent->velocity.Zero();
        ent->avelocity.Zero();
        //ent->movetype=MOVETYPE_NONE;
        //ent->clipmask=MASK_SOLID;
    }
}

//---------------------------------------------------------------------------
// unfreezeEntity()
//---------------------------------------------------------------------------
void unfreezeEntity(userEntity_t *ent,freezeEntity_t *freeze)
{
    playerHook_t *phook=(playerHook_t *)ent->userHook;


    if ( ent->flags & FL_CLIENT )
    {
		 // restore func pointers
		ent->prethink=freeze->old_prethink;
		ent->think=freeze->old_think;
		ent->pain=freeze->old_pain;
		ent->die=freeze->old_die;

		// special client shit here
        ent->client->ps.pmove.pm_type = PM_NORMAL;
        ent->flags &= ~FL_FORCEANGLES;
        gstate->SetCameraState(ent,false,0);

/*
        if ( phook->dflags & DFL_HIERARCHICAL )
        {
            if ( phook->legs )
                phook->legs->svflags &= ~SVF_NOTOWNER;
            if ( phook->torso )
                phook->torso->svflags &= ~SVF_NOTOWNER;
            if ( phook->head )
                phook->head->svflags &= ~SVF_NOTOWNER;
            if ( phook->weapon )
                phook->weapon->svflags &= ~SVF_NOTOWNER;
        }
        else
*/			ent->s.renderfx &= ~RF_NODRAW;

        ent->velocity = freeze->old_velocity;
        ent->avelocity = freeze->old_avelocity;
        ent->movetype= (movetype_t) freeze->old_movetype;
    }
    else
    {
        // restore old state
//      ent->prethink=freeze->old_prethink;
//      ent->think=freeze->old_think;
//      ent->pain=freeze->old_pain;
//      ent->die=freeze->old_die;

//	  AI_EndFreeze( ent );        // mike: trouble compiling..

        playerHook_t *hook=(playerHook_t *)ent->userHook;
		if (hook)
			hook->nSpawnValue &= ~SPAWN_DO_NOT_MOVE;
		//ent->movetype= (movetype_t ) freeze->old_movetype;
        //ent->velocity = freeze->old_velocity;
        //ent->avelocity = freeze->old_avelocity;
		//ent->clipmask = freeze->old_clipmask;
    }
}

//---------------------------------------------------------------------------
// flashClients()
//---------------------------------------------------------------------------
void flashClients(userEntity_t *src,float r,float g,float b,float a, float vel)
{
    userEntity_t    *ent;
	CVector			vDirection;
	float			fDist;

    ent=gstate->FirstEntity();          // first entity is world
    ent=gstate->NextEntity(ent);        // next 16 should be clients
    while ( ent )
    {
        if ( !(ent->flags & FL_CLIENT) )
            break;

        if ( !ent->inuse )
            continue;

		// cek[2-10-00]: here, how bout we not set the fov to friggin radians....eh?
//        ai_setfov(ent,90);

		vDirection = ent->s.origin - src->s.origin;

		fDist = fabs( vDirection.Length() );

//        if ( ai_visible(ent,src) )
        if ( fDist < 300.0 )
        {
            com->FlashClient( ent, a, r, g, b, vel );
        }

        ent=gstate->NextEntity(ent);
    }
}

//---------------------------------------------------------------------------
// winfoStopLooped()
//---------------------------------------------------------------------------
void winfoStopLooped(userEntity_t *self)
{
    self->s.sound = 0;
    // gstate->sound(self,CHAN_WEAPON,"null.wav",0,ATTN_NORM);
}

//---------------------------------------------------------------------------
// clientSmoke()
//---------------------------------------------------------------------------
void clientSmoke(CVector &pos,float scale,float amount,float spread,float maxvel)
{
	if (gstate->PointContents(pos) & MASK_WATER)
		return;

    gstate->WriteByte(SVC_TEMP_ENTITY);
    gstate->WriteByte(TE_SMOKE);
    gstate->WritePosition(pos);
    gstate->WriteFloat(scale);
    gstate->WriteFloat(amount);
    gstate->WriteFloat(spread);
    gstate->WriteFloat(maxvel);
    gstate->MultiCast(pos,MULTICAST_PVS);
}

//---------------------------------------------------------------------------
// clientBeamSparks()
//---------------------------------------------------------------------------
void clientBeamSparks(CVector &pos,float count,float scale)
{
    gstate->WriteByte(SVC_TEMP_ENTITY);
    gstate->WriteByte(TE_BEAM_SPARKS);
    gstate->WritePosition(pos);
    gstate->WriteFloat(count);
    gstate->WriteFloat(scale);
    gstate->MultiCast(pos,MULTICAST_PVS);
}

//---------------------------------------------------------------------------
// clientSparks()
//---------------------------------------------------------------------------
void clientSparks(CVector &pos,CVector &dir,int amount, CVector &color)
{
    gstate->WriteByte(SVC_TEMP_ENTITY);
    gstate->WriteByte(TE_SPARKS);
    gstate->WriteByte(amount);
    gstate->WritePosition(pos);
    gstate->WritePosition(dir);
    gstate->WritePosition(color);
    gstate->WriteByte(1);
    gstate->MultiCast(pos,MULTICAST_PVS);
}

//---------------------------------------------------------------------------
// clientBlood()
//---------------------------------------------------------------------------
void clientBlood(CVector &pos,CVector &dir,int amount, CVector &color)
{
    gstate->WriteByte(SVC_TEMP_ENTITY);
    gstate->WriteByte(TE_BLOOD);
    gstate->WriteByte(amount);
    gstate->WritePosition(pos);
    gstate->WritePosition(dir);
    gstate->WritePosition(color);
    gstate->WriteByte(1);
    gstate->MultiCast(pos,MULTICAST_PVS);
}

//---------------------------------------------------------------------------
// clientFireSmoke() - fire/smoke combo, used by sunflare
//---------------------------------------------------------------------------
void clientFireSmoke(CVector &pos, float burntime, char firstFrame)
{
    gstate->WriteByte(SVC_TEMP_ENTITY);
    gstate->WriteByte(TE_FIRESMOKE);
    gstate->WritePosition(pos);
    gstate->WriteFloat(burntime);
    gstate->WriteByte(firstFrame);
    gstate->MultiCast(pos,MULTICAST_PVS);
}

//---------------------------------------------------------------------------
// entBackstep()
//---------------------------------------------------------------------------
void entBackstep(userEntity_t *ent, float dist)
{
    CVector vec = ent->velocity;
    vec.Normalize();
    vec = ent->s.origin + vec * -dist;
    ent->s.origin = vec;
}

//---------------------------------------------------------------------------
// entBackstep()
//---------------------------------------------------------------------------
void entBackstep(userEntity_t *ent, CVector &normal, float dist)
{
    ent->s.origin.x += normal.x * dist;
    ent->s.origin.y += normal.y * dist;
    ent->s.origin.z += normal.z * dist;
}

//---------------------------------------------------------------------------
// entStuck()
//---------------------------------------------------------------------------
short entStuck(userEntity_t *ent)
{
    gstate->TraceLine(ent->s.origin,ent->s.origin,false,NULL,&trace);
    if ( trace.allsolid || trace.startsolid )
        return(true);
    else
        return(false);
}

//-----------------------------------------------------------------------------
//	weaponShouldSwitch
//
//	returns true if player should switch from old_weapon to new_weapon
//-----------------------------------------------------------------------------

int weaponShouldSwitch (weapon_t *new_weapon, weapon_t *old_weapon)
{
    int new_rating, old_rating;
    int new_ammo, old_ammo;

    if ( !old_weapon )
        return	TRUE;

    new_weapon->command ((userInventory_t *) new_weapon, "rating", &new_rating);
    old_weapon->command ((userInventory_t *) old_weapon, "rating", &old_rating);

    new_weapon->command ((userInventory_t *) new_weapon, "ammo_count", &new_ammo);
    old_weapon->command ((userInventory_t *) old_weapon, "ammo_count", &old_ammo);

	// this is stupid.  not all weapons require ammo.
	bool has_ammo;
	if (new_weapon->winfo->ammo_max > 0)
	{
		if (new_ammo)
			has_ammo = true;
		else
			has_ammo = false;
	}
	else
		has_ammo = true;

    if ( ((new_rating > old_rating) && has_ammo) || ((old_ammo == 0) && has_ammo) )
        return	TRUE;
    else
        return	FALSE;
}

//-----------------------------------------------------------------------------
//	weaponTouchDefault
//
//	called when a weapon pickup item is touched
//	sets item's modelindex to 0 so that it disappears
//	sets item's solid type to SOLID_NOT so that it cannot be touched
//	calls item's give function
//	sets weapon's think to respawn function
//-----------------------------------------------------------------------------
									
void    weaponTouchDefault (userEntity_t *self, userEntity_t *other, cplane_t *plance, csurface_t *surf)
{
    touchHook_t *hook = (touchHook_t *) self->userHook;
    weapon_t    *new_weapon;
    select_t    select_func;

    if ( !(other->flags & (FL_CLIENT | FL_BOT)) )
        return;
/*
	// check to see if this item is meant for a particular sidekick (single player only!)
	if (!deathmatch->value && !coop->value)
	{
		if (self->spawnflags & (SPAWNFLAG_SUPERFLY|SPAWNFLAG_MIKIKO))
		{
			short passed = 0;
			if (self->spawnflags & SPAWNFLAG_SUPERFLY)				// superfly can pick this up
			{
				passed = (!strcmp( other->className, "Superfly" )) ? 1 : 0;
			}

			if (!passed && (self->spawnflags & SPAWNFLAG_MIKIKO))
			{
				passed = (!strcmp( other->className, "Mikiko" )) ? 1 : 0;
			}

			if (!passed)
				return;
		}
	}
*/

	bool bWeaponsStay = ((deathmatch->value && dm_weapons_stay->value) || coop->value);

	// if we are touching sunflare, discus or metamaser, force bWeaponsStay to false...
	if ( !stricmp(self->className,"weapon_sunflare") || !stricmp(self->className,"weapon_discus") || !stricmp(self->className,"weapon_metamaser") )
		bWeaponsStay = false;

	// if weapons stay and we have this weapon treat coop as dm_weapons_stay
//	if ((dm_weapons_stay->value) && gstate->InventoryFindItem(other->inventory, self->className))
	if ( bWeaponsStay && gstate->InventoryFindItem(other->inventory, self->className))
	{
		// if the weapon was dropped we can still pick it up
		// WAW[12/2/99]: Deathmatch and coop mode.
		if (!(self->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && (coop->value || deathmatch->value))
			return;	// leave the weapon for others to pickup
	}

    //	call give function for this weapon
    new_weapon = (weapon_t *) hook->give_func (other, self->hacks_int);

	// if it wasn't given, don't pick it up!
	if (!new_weapon)
		return;

    //	save select func, because com->SetRespawn will remove self and it's userHook
    //	in single-player games
    select_func = hook->select_func;
	self->think = NULL;

	// clear out weapon and set respawn
//	if (((!deathmatch->value && !coop->value) && dm_weapons_stay->value) || (!dm_weapons_stay->value || (self->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM))))
	if ( !bWeaponsStay || (self->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) )
	{
		//	com->SetRespawn does the following:
		//	In deathmatch:
		//	set weapon to SOLID_NOT, set think function to com->Respawn
		//	reset ambient volume if item has one (not working in Q2 yet)
		//	stop weapons thrown out at death from respawning
		//com->SetRespawn (self, self->delay);
		// NSS[3/9/00]:
		Weapon_SetRespawn(self, self->delay	);
		//	make weapon invisible    
		self->s.modelindex = 0;
	}

    //	flash screen if this is a client
    if ( other->flags & FL_CLIENT )
        other->client->bonus_alpha = 0.25;

    // play the weapon pickup sound
    gstate->StartEntitySound(other, CHAN_OVERRIDE, gstate->SoundIndex("global/i_pickup6.wav"), 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);

	if (new_weapon)
	{
		if (other->flags & FL_CLIENT)
		{
			if (gstate->SIDEKICK_RemoveItem)
				gstate->SIDEKICK_RemoveItem(self);

			if (new_weapon->winfo)
			{
				//gstate->centerprint (other, 1.0, "%s %s.\n", tongue[T_PICKUP_WEAPON], new_weapon->winfo->weaponNetName);
				gstate->cs.BeginSendString();
				gstate->cs.SendSpecifierStr("%s %s.\n",2);
				gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue,T_PICKUP_WEAPON));
				gstate->cs.SendStringID(new_weapon->winfo->weaponNameIndex);
				gstate->cs.Unicast(other,CS_print_center, 1.0);
				gstate->cs.EndSendString();
			}

			short tryAutoSwitch = 1;
			if (other->client)
			{
				char autoweap[5];
				strncpy(autoweap, Info_ValueForKey(other->client->pers.userinfo, "cl_autowweaponswitch"), 5);        
				if (!strnicmp(autoweap,"0",1))
					tryAutoSwitch = 0;

			}

			//	weaponShouldSwitch returns true if player should switch to the new weapon
			if ( tryAutoSwitch && weaponShouldSwitch ((weapon_t *) new_weapon, (weapon_t *) other->curWeapon) )
				select_func (other);
		}

		if ( other->PickedUpItem )
		{
			other->PickedUpItem( other, new_weapon->name );
		}
	}
}


// some stuff to validate whether or not weapons/ammo should be spawned.
// NOTE:  after winfoSetSpawn or ammoSetSpawn return, self must be tested
// to make sure it is still valid if it is used elsewhere...
#define VALIDATE_EPISODE			0X0001
#define VALIDATE_FLAGS				0X0002
#define VALIDATE_ALL				0XFFFF

// check some things to make sure we should really spawn a weapon
short winfoValidate(weaponInfo_t *winfo, unsigned short flags )
{
	short passed = 0;
	int index = 0;
	short episode;
	if (flags & VALIDATE_EPISODE)
	{
		int i;
		episode = gstate->episode;
		if ((episode > WP_NUM_EPISODES) || (episode < 1) || !winfo)		// make sure this is even a valid episode first
			return 0;

		episode--;	// gstate->episode is 1-4, make it 0-3
		// is the weapon an episode weapon?
		for (i = 0; i < WP_WEAPONS_PER_EP; i++)
		{
			if (weaponList[episode][i] == winfo)
			{
				index = i;
				passed = 1;
				break;
			}
		}

		if (!passed)
		{
			// is this a global weapon?
			for (i = 0; i < WP_NUM_EPISODES; i++)
			{
				if (weaponList[WP_NUM_EPISODES][i] == winfo)
				{
					index = i;
					episode = WP_NUM_EPISODES;
					passed = 1;
					break;
				}
			}
		}
	}

	// only check the flags in deathmatch mode.
// SCG[1/24/00]: not used testflag was not used and the rest of ths statement was commented out
// SCG[1/24/00]: since nothing happens here, we'll ocmment out the whole thing.
//	if ((flags & VALIDATE_FLAGS) && passed && deathmatch->value)
//	{
//		long testflag = 1 << index;// SCG[1/24/00]: not used
//		long weapflags = (episode < WP_NUM_EPISODES) ? dm_episode_weapons->value : dm_global_weapons->value;
//		passed = (weapflags & testflag) ? 1 : 0;
//	}

	return passed;
}

void winfoDroppedItemThink(userEntity_t *self)
{
	self->touch = NULL;
	self->think = NULL;
	self->remove(self);
}

//-----------------------------------------------------------------------------
//	winfoSetSpawn
//
//	set up spawn info for a weapon
//
//	respawn_time:	time for weapon to respawn after being picked up
//	touch:			touch function to call when this item is touched
//					if NULL is passed this is set to weapon_TouchDefault
//-----------------------------------------------------------------------------
void winfoSetSpawn(weaponInfo_t *winfo,userEntity_t *self,float respawn_time,touch_t touch)
{
	if (IS_INSTAGIB)
	{
		gstate->RemoveEntity(self);
		return;
	}

	if (!winfoValidate(winfo,VALIDATE_ALL))
	{
		gstate->RemoveEntity(self);
		return;
	}

    touchHook_t *hook;

    //	malloc hook
    self->userHook = gstate->X_Malloc(sizeof(touchHook_t), MEM_TAG_HOOK);
    hook=(touchHook_t *)self->userHook;

	// SCG[11/24/99]: Save game stuff
	self->save = touch_hook_save;
	self->load = touch_hook_load;

    //	set give / select function
    hook->give_func=winfo->give_func;
    hook->select_func=winfo->select_func;

    //	set respawn value
    self->delay=respawn_time;

    winfoSetModel(self,MDL_ARTIFACT_STD);

    //	self->modelName stores the model name so it can be reset when
    //	the item respawns
    self->modelName=winfoModel(MDL_ARTIFACT_STD);

    //	set mins/maxs
    //	mins are all 0's so that we can use a model that is centered
    //	for both the pickup model and the inventory model
    self->s.mins.Set( -16, -16, 0 );
    self->s.maxs.Set( 16, 16, 24 );

    //	set up physics
    self->movetype = MOVETYPE_TOSS;
    self->solid = SOLID_TRIGGER;
    self->flags = 0;    //	cannot be pushed
    if(deathmatch->value)
    {
		entAnimate(self,3,3,FRAME_ONCE,0.05);

//        self->s.frame = 3;
        self->s.effects |= EF_ROTATE;
        self->s.mins.Set(0, 0, -16);
        self->s.origin.z += 16;
    }
	if (self->spawnflags & (SPAWNFLAG_SUPERFLY|SPAWNFLAG_MIKIKO))
		self->s.renderfx |= RF_FULLBRIGHT;//RF_GLOW;
	else
		self->s.renderfx |= RF_GLOW;

    self->className = winfo->weaponName;
    self->hacks_int = winfo->initialAmmo;   //	store ammount of ammo in frags
    self->netname   = winfo->weaponNetName;

	// set up a delay to destroy the item if it has been dropped
	if ((deathmatch->value) && (respawn_time < 0.0f))
	{
		self->think = winfoDroppedItemThink;
		self->nextthink = gstate->time + 60;
	}

    //	set up touch function
    if ( !touch )
        self->touch = weaponTouchDefault;
    else
        self->touch = touch;

	self->flags |= FL_ITEM;
	self->svflags |= SVF_ITEM;
    //	link into bsp
    gstate->LinkEntity(self);
}

// ----------------------------------------------------------------------------
// NSS[3/9/00]:
// Name:		Weapon_Respawn
// Description:The actual function which respawns the item.
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void	Weapon_Respawn (userEntity_t *self)
{
	if (deathmatch->value || !(deathmatch->value))
	{
		self->s.sound		= self->soundAmbientIndex;
		self->s.modelindex	= gstate->ModelIndex(self->modelName);
	
		self->solid			= SOLID_TRIGGER;
		self->nextthink		= -1;

		self->s.effects		|= EF_RESPAWN;	//	tell client to start respawn effect
		self->s.renderfx	|= RF_GLOW;

		gstate->LinkEntity(self);
		gstate->StartEntitySound( self, CHAN_AUTO,gstate->SoundIndex("global/a_itspwn.wav"),1.0f,256.0f, 512.0f);	
	}
}


// ----------------------------------------------------------------------------
// NSS[3/9/00]:
// Name:		Weapon_SetRespawn
// Description:Generic function to set the respawning of items.
// Input:userEntity_t *self
// Output:NA
// Note:
// ----------------------------------------------------------------------------
void Weapon_SetRespawn(userEntity_t *self, float delay)
{
	self->s.modelindex = 0;	//	set model to 0, modelName should still be valid, though
	self->solid = SOLID_NOT;
	
	//	shut off looping ambient sounds
	if (self->s.sound > 0)
	{
		self->soundAmbientIndex = self->s.sound;
		self->s.sound = 0;
	}
	else
		self->soundAmbientIndex = 0;


	if (deathmatch->value && delay > 0.0f)
	{
		// NSS[3/4/00]:Set the respawn value for the item.
		self->nextthink = gstate->time + delay;
		self->think = Weapon_Respawn;
	}
	else
		self->remove (self);
}


//-----------------------------------------------------------------------------
//	ammoTouchDefault
//
//	called when a weapon pickup item is touched
//	sets item's modelindex to 0 so that it disappears
//	sets item's solid type to SOLID_NOT so that it cannot be touched
//	calls item's give function
//	sets weapon's think to respawn function
//-----------------------------------------------------------------------------

void    ammoTouchDefault (userEntity_t *self, userEntity_t *other, cplane_t *plance, csurface_t *surf)
{
	playerHook_t *ohook = (playerHook_t *)other->userHook;
    ammoTouchHook_t *hook = (ammoTouchHook_t *) self->userHook;
	if (!hook || !ohook)
		return;

    //weapon_t        *new_weapon;
    ammo_t          *ammo = NULL;
    int             ammo_addcount = hook->count;
    weaponInfo_t    *winfo;

    if ( !(other->flags & (FL_CLIENT | FL_BOT)) )
        return;

	if ((other->flags & FL_BOT) && ((hook->winfo->flags & WFL_PLAYER_ONLY) || (ohook->type == TYPE_MIKIKOFLY)))
		return;

/*
	// check to see if this item is meant for a particular sidekick (single player only!)
	if (!deathmatch->value && !coop->value)
	{
		if (self->spawnflags & (SPAWNFLAG_SUPERFLY|SPAWNFLAG_MIKIKO))
		{
			short passed = 0;
			if (self->spawnflags & SPAWNFLAG_SUPERFLY)				// superfly can pick this up
			{
				passed = (!strcmp( other->className, "Superfly" )) ? 1 : 0;
			}

			if (!passed && (self->spawnflags & SPAWNFLAG_MIKIKO))
			{
				passed = (!strcmp( other->className, "Mikiko" )) ? 1 : 0;
			}

			if (!passed)
				return;
		}
	}
*/
	// get the ammo from the inventory
    ammo = (ammo_t *) gstate->InventoryFindItem (other->inventory, self->className);
    if (ammo && (winfo = (weaponInfo_t *)ammo->winfo))
    {
		ammo_addcount = (ammo->count + ammo_addcount <= winfo->ammo_max) ? (ammo_addcount) : (winfo->ammo_max - ammo->count);
    }

    //	add the ammo
	if (!ammo_addcount)
		return;

	ammo = ammoAdd (other, ammo_addcount, hook->winfo);

	if(!ammo)
		return;

	if ((other->flags & FL_CLIENT) && ammo->winfo )
	{
//		gstate->centerprint (other, 1.0, "%s %s.\n", tongue[T_PICKUP_WEAPON], ammo->winfo->ammoNetName);
		gstate->cs.BeginSendString();
		gstate->cs.SendSpecifierStr("%s %s.\n",2);
		gstate->cs.SendStringID(RESOURCE_ID_FOR_STRING(tongue,T_PICKUP_WEAPON));
		gstate->cs.SendStringID(ammo->winfo->ammoNameIndex);
		gstate->cs.Unicast(other, CS_print_center, 1.0);
		gstate->cs.EndSendString();
	}
    //	make ammo invisible
    self->s.modelindex = 0;

// this is done in com_setrespawn	self->s.sound = 0;
    // play the ammo pickup sound
	gstate->StartEntitySound(other, CHAN_OVERRIDE, hook->sound, 1.0, ATTN_NORM, ATTN_NORM_MAX);
    //	com->SetRespawn does the following:
    //	In deathmatch:
    //	set weapon to SOLID_NOT, set think function to com->Respawn
    //	reset ambient volume if item has one (not working in Q2 yet)
    //com->SetRespawn (self, self->delay);
	Weapon_SetRespawn(self, self->delay	);


    //	flash screen if this is a client
    if ( other->flags & FL_CLIENT )
	{
		if (gstate->SIDEKICK_RemoveItem)
			gstate->SIDEKICK_RemoveItem(self);
        other->client->bonus_alpha = 0.25;
	}

	  if ( other->PickedUpItem )
		  other->PickedUpItem( other, ammo->name );

	WEAPON_DISPLAY_UPDATE(other);
}

void ammo_set_sound(userEntity_t *self, int sound)
{
	if (!self->inuse || !stricmp(self->className,"freed"))
		return;

	ammoTouchHook_t *hook=(ammoTouchHook_t *)self->userHook;
	hook->sound = sound;
}
//-----------------------------------------------------------------------------
//	winfoAmmoSetSpawn
//
//	set up spawn info for weapon ammo
//
//	count:			amount of ammo this item gives
//	respawn_time:	time for weapon to respawn after being picked up
//	touch:			touch function to call when this item is touched
//					if NULL is passed this is set to weapon_TouchDefault
//-----------------------------------------------------------------------------
void winfoAmmoSetSpawn(weaponInfo_t *winfo,userEntity_t *self,int count,float respawn_time,touch_t touch)
{
	if (IS_INSTAGIB)
	{
		gstate->RemoveEntity(self);
		return;
	}

	if (!winfoValidate(winfo,VALIDATE_ALL))
	{
		gstate->RemoveEntity(self);
		return;
	}

    ammoTouchHook_t *hook;

    self->className = winfo->ammoName;

    //	malloc hook
    self->userHook = gstate->X_Malloc(sizeof(ammoTouchHook_t), MEM_TAG_HOOK);
    hook=(ammoTouchHook_t *)self->userHook;

    //	set give / select function
    hook->winfo = winfo;
    hook->count = count;
	hook->sound = gstate->SoundIndex("global/i_c4ammo.wav");

    //	set respawn value
    if (deathmatch->value && dm_item_respawn->value)  // 2.28 dsn
      self->delay = respawn_time;
    else
      self->delay = -1.0f; // no respawn

	// SCG[12/7/99]: Save game stuff
	self->save = ammo_touch_hook_save;
	self->load = ammo_touch_hook_load;

    winfoSetModel(self,MDL_AMMO_STD);

    //	self->modelName stores the model name so it can be reset when
    //	the item respawns
    self->modelName=winfoModel(MDL_AMMO_STD);

    //	set mins/maxs
    //	mins are all 0's so that we can use a model that is centered
    //	for both the pickup model and the inventory model
//    self->s.mins.Set(-8, -8, -4);
//    self->s.maxs.Set(8, 8, 8);
    self->s.mins.Set(-8, -8, 0);
    self->s.maxs.Set(8, 8, 24);

	//self->s.origin.z += 16.0f;

    //	set up physics
    self->movetype = MOVETYPE_TOSS;
    self->solid = SOLID_TRIGGER;
    self->flags = 0;    //	cannot be pushed
//    self->s.effects |= EF_ROTATE;
    self->s.renderfx |= RF_GLOW;

    //	set up touch function
    if ( !touch )
        self->touch = ammoTouchDefault;
    else
        self->touch = touch;

	self->flags |= FL_ITEM;
	self->svflags |= SVF_ITEM;
    //	link into bsp
    gstate->LinkEntity(self);
}

// initializes a trackinfo structure for use with the CL_WeaponMuzzlePoint function.
// will set all necessary flags and variables accordingly
void weaponTrackMuzzle(trackInfo_t& tinfo, userEntity_t *self, userEntity_t *player, weaponInfo_t *winfo, bool full)
{
	if (!self || !player)
		return;

	tinfo.ent = self;
	tinfo.srcent = player;
	tinfo.flags |= TEF_SRCINDEX;
	if (full)
	{
		if (winfo)
		{
			tinfo.modelindex2 = gstate->ModelIndex( winfoModel(MDL_ARTIFACT_STD) );			// 3rd person model
			tinfo.flags |= TEF_MODELINDEX2;
		}
		if (player->client && player->client->ps.weapon)
		{
			tinfo.extra = (edict_s *)player->client->ps.weapon;
			tinfo.flags |= TEF_EXTRAINDEX;
		}
	}
}

void weaponUntrackThink(userEntity_t *self)
{
	gstate->RemoveEntity(self);
}

void weaponUntrackProjectile(userEntity_t *proj)
{
	if (!proj)
		return;

	winfoStopLooped(proj);
	proj->s.effects2 &= ~EF2_TRACKED_PROJ;
	proj->s.renderfx |= RF_NODRAW|RF_TRACKENT;
	proj->think = weaponUntrackThink;
	proj->nextthink = gstate->time + 0.2;
	proj->touch = NULL;
	proj->pain = NULL;
	proj->die = NULL;
	proj->prethink = NULL;
	proj->postthink = NULL;
	proj->remove = NULL;
	proj->solid = SOLID_NOT;
	gstate->LinkEntity(proj);

	com->untrackEntity(proj,NULL,MULTICAST_ALL);
//	RELIABLE_UNTRACK(proj);
}

// initializes a trackinfo structure for tracking a projectile
void weaponTrackProjectile(trackInfo_t &tinfo, userEntity_t *proj, userEntity_t *player, weaponInfo_t *winfo, short ID, short projFlags, bool full)
{
	// the projectile tracker needs this..set it up
	if (!proj || !player || !winfo)
		return;

	// put in the verification.
	if (!(proj->s.effects2 & EF2_TRACKED_PROJ))
	{
		proj->s.effects2 |= EF2_TRACKED_PROJ;
		gstate->LinkEntity(proj);
	}

	weaponTrackMuzzle(tinfo,proj,player,winfo,full);
	tinfo.fxflags = TEF_PROJ_FX|TEF_FX_ONLY;
	tinfo.numframes = TEF_ID_OF(ID)|projFlags;
	tinfo.flags |= TEF_FXFLAGS|TEF_NUMFRAMES;
}

void weaponMuzzleFlash(userEntity_t *track, userEntity_t *player, weaponInfo_t *winfo, long flashIdx)
{
	cvar_t *w_vis = gstate->cvar("weapon_visible","1",CVAR_ARCHIVE);
	if (w_vis->value == 0)
		return;

	trackInfo_t tinfo;
    memset(&tinfo, 0, sizeof(tinfo));
	weaponTrackMuzzle(tinfo,track,player,winfo,true);
	tinfo.Long1 = flashIdx;
	tinfo.fxflags = TEF_ANIMATE2|TEF_LIGHT|TEF_FX_ONLY;
	tinfo.flags |= TEF_LONG1|TEF_FXFLAGS;

    com->trackEntity(&tinfo,MULTICAST_PVS);
}

//-----------------------------------------------------------------------------
//	weaponScorchMark()
//-----------------------------------------------------------------------------
void weaponScorchMark( trace_t &trace, byte cType )
{
	if (trace.contents & CONTENTS_CLEAR || !trace.ent || trace.ent->solid != SOLID_BSP || trace.ent->takedamage != DAMAGE_NO)
		return;

	if (trace.surface && (trace.surface->flags & SURF_FLOWING))
		return;

    gstate->WriteByte( SVC_TEMP_ENTITY );
    gstate->WriteByte( TE_SCORCHMARK );
    gstate->WritePosition( trace.endpos );
    gstate->WriteShort( trace.plane.planeIndex );
    gstate->WriteShort( trace.ent->s.modelindex );
    gstate->WriteShort( trace.ent->s.number);
    gstate->WriteFloat( 1 );
    gstate->WriteFloat( 0 );
    gstate->WriteByte( cType );
	if (cType == SM_BULLET)
		gstate->WritePosition(zero_vector);
    gstate->MultiCast( zero_vector, MULTICAST_ALL );
}

//-----------------------------------------------------------------------------
//	weaponScorchMark2() - didn't know what to call this...
//-----------------------------------------------------------------------------
void weaponScorchMark2(userEntity_t *self, userEntity_t *world,cplane_t *plane)
{
    trace_t trace;
    CVector end;

	if (!plane)
		return;		// WAW[11/16/99] No Scorch if no plane.

    end.x = self->s.origin.x + self->velocity.x;
    end.y = self->s.origin.y + self->velocity.y;
    end.z = self->s.origin.z + self->velocity.z;

    trace=gstate->TraceLine_q2(self->s.origin,end,self,MASK_SHOT);
	if (trace.contents & CONTENTS_CLEAR || !trace.ent ||  trace.ent->solid != SOLID_BSP || trace.ent->takedamage != DAMAGE_NO)
		return;

	if (trace.surface && (trace.surface->flags & SURF_FLOWING))
		return;

    gstate->WriteByte(SVC_TEMP_ENTITY);
    gstate->WriteByte(TE_SCORCHMARK);
    gstate->WritePosition(trace.endpos);
    gstate->WriteShort(plane->planeIndex);
    gstate->WriteShort(world->s.modelindex);                //	write model index so that we can stick to
    //	bmodels correctly
    gstate->WriteShort(world->s.number);                    // entity number for bmodels
    gstate->WriteFloat( 1 );
    gstate->WriteFloat( 0 );
    gstate->WriteByte( SM_SCORCH );
    gstate->MultiCast(zero_vector,MULTICAST_ALL);
}

void weaponBloodSplat (userEntity_t *target, CVector &start, CVector &end, CVector &impactPos, float dist)
{
// SCG[6/5/00]: #ifdef TONGUE_GERMAN
#ifdef NO_VIOLENCE
		return;
#endif

	cvar_t *sv_violence = gstate->cvar("sv_violence","0",CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE);
	if (sv_violence->value > 0.0)
		return;

    playerHook_t    *hook;
    trace_t         blood_trace;

    if ( target->flags & (FL_CLIENT | FL_BOT | FL_MONSTER) )
    {
        hook = (playerHook_t *) target->userHook;
        if ( hook->dflags & DFL_MECHANICAL )
            //	FIXME:	do an oil splat or something?
            return;
		
        //	Nelno:	testing wall damage
        //	trace again to find a place for the blood splat
        blood_trace = gstate->TraceLine_q2 (start, end, NULL, MASK_SOLID);
        if ( blood_trace.fraction < 1.0 )
        {
            if ( VectorDistance(blood_trace.endpos, impactPos) < 128.0 )
            {
                {
					gstate->WriteByte (SVC_TEMP_ENTITY);
					gstate->WriteByte (TE_BLOODSPLAT);
					gstate->WritePosition (blood_trace.endpos);
					gstate->WriteShort (blood_trace.plane.planeIndex);
					gstate->WriteShort (blood_trace.ent->s.modelindex); //	write model index so that we can stick to
					gstate->WriteShort (blood_trace.ent->s.number);     //	entity number for bmodels
					gstate->MultiCast(vec3_origin_w,MULTICAST_PVS);
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//	weaponMuzzleFlashForMonstersAndBots
//
//	places the muzzle flash flag on the correct entity for a monster or bot
//	normally this is just the monster entity, but if it is hierarchical the
//	flag must be placed on the weapon model
///////////////////////////////////////////////////////////////////////////////

void    weaponMuzzleFlashForMonstersAndBots (userEntity_t *self, playerHook_t *hook)
{
    if ( self->flags & (FL_MONSTER | FL_BOT) )
    {
/*
        if ( hook->dflags & DFL_HIERARCHICAL && hook->weapon )
            hook->weapon->s.renderfx |= RF_MUZZLEFLASH;
        else
*/            self->s.renderfx |= RF_MUZZLEFLASH;
    }
}

////////////////////////////////////////////////////////////////////////
//	weapon_kick(userEntity_t *player, CVector kick_angles, CVector kick_velocity)
//
//	Description:
//		Kicks the viewangles on the client, optionally differentiates 
//		forward, right and up velocity from current velocity.
//
//	Parameters:
//		player			the player experiencing the kick
//		kick_angles		adjustment to make
//		kick_velocity	forward, right and up velocity relative to player
//						--that is, player view-angles, since that's where
//						the weapon is pointing
//
//	Return:
//		nothing
//	
//	BUGS:
//		This function does *NOT* reset the view angles after it is called.
//		this must be done either in the next think routine or elsewhere
//		in the game.
//		This generates a net message since the playerstate kickangle is 
//		updated. If/when entity stuff is moved to the client side, then
//		weapon kicks can be triggered there without the network overhead.
//
void weapon_kick(userEntity_t *player, CVector kick_angles, float kick_velocity, short fViewVelocityTo, short fViewVelocityReturn)
{
	CVector vel, forward, right, up;

    // Bail if this is not a player!
    if(!(player->flags & FL_CLIENT) || !player->client)
		return;
	// Kick the view angles
    gstate->WriteByte(SVC_TEMP_ENTITY);
    gstate->WriteByte(TE_KICKVIEW);
    gstate->WritePosition(kick_angles);
    gstate->WriteFloat(0);
    gstate->WriteShort(fViewVelocityTo);
    gstate->WriteShort(fViewVelocityReturn);
    gstate->UniCast(player, true);

    // move the player
	vel=player->client->v_angle;
	vel.AngleToVectors(forward, right, up);
	vel=forward*kick_velocity;
	player->velocity+=vel;
}


void ManualNextWeapon(userEntity_t *self, int dir)
// used in conjunction with 'weapnext' and 'weapprev' bindings
{
    weapon_t        *weapon = (weapon_t *) self->curWeapon;
    weapon_t        *prev, *next, *new_weapon;
    weapon_t        *weaponList [MAX_WEAPONS];
    weaponInfo_t    *winfo;
    userInventory_t *item;

    int             best_weapon, best_rating, worst_weapon, worst_rating;
    int             rating;
    int             cur_rating, best_prev_rating, best_prev_weapon;
    int             best_next_rating, best_next_weapon;
    int             total_weapons, i, ammo_count;

    //	get a list of all the weapons in the player's inventory
    //	excluding their current weapon
    total_weapons  = 0;
    item = gstate->InventoryFirstItem (self->inventory);

    while ( item && total_weapons < MAX_WEAPONS )
    {
        if ( item->name && strstr (item->name, "weapon_") )
        {
			// if there is no current weapon, all weapons are valid.
            if (!weapon || (weapon && stricmp (item->name, weapon->name)) )
            {
                //	exclude current weapon
                weaponList [total_weapons] = (weapon_t *) item;
                total_weapons++;
            }
        }

        item = gstate->InventoryNextItem (self->inventory);
    }

    if (total_weapons == 0)  // no weapons to change to?
      return;
      
    best_prev_rating = -1;
    best_prev_weapon = -1;

    best_next_rating = 32767;
    best_next_weapon = -1;

    best_weapon = -1;
    best_rating = -1;

    worst_weapon = -1;
    worst_rating = 32767;

	if (weapon)
		weapon->command((userInventory_t *) weapon, "rating", &cur_rating);
	else
		cur_rating = 0;

    // finds: weapon with the prev rating below that of the current weapon
    //        weapon with the next rating above that of the current weapon
    //        weapon with the best rating
    //        weapon with the worst rating


    for (i = 0;i < total_weapons; i++)
    {
        weaponList[i]->command((userInventory_t *) weaponList [i], "rating", &rating);
		ammo_count = (int)weaponList [i]->ammo->command ((userInventory_t *) weaponList [i]->ammo, "count", &ammo_count);

        winfo = (weaponInfo_t *) weaponList [i]->winfo;

        if ((ammo_count >= winfo->ammo_per_use) || (winfo->flags & WFL_SELECT_EMPTY) ) // the weapon has ammo?
        {

          if (rating > best_rating) 
          {
            best_rating = rating;
            best_weapon = i;
          }

          if (rating < worst_rating)
          {
            worst_rating = rating;
            worst_weapon = i;
          }

          // is this weapon better than the next best weapon, found so far?
          if (rating > cur_rating && rating < best_next_rating)
          {
            best_next_weapon = i;        // store weapon that is the next best
            best_next_rating = rating;   // store rating
          }

          if (rating < cur_rating && rating > best_prev_rating)
          {
            best_prev_weapon = i;        // store weapon that is previous best
            best_prev_rating = rating;   // store rating
          }
        }
    }
 
    // set next best weapon
    if (best_next_weapon != -1)
      next = weaponList[best_next_weapon];
    else
      if (worst_weapon != -1)                      // flip over to bottom ratings
        next = weaponList[worst_weapon];
      else
        next = NULL;

    // set prev best weapon
    if (best_prev_weapon != -1)
      prev = weaponList [best_prev_weapon];
    else
      if (best_weapon != -1)
        prev = weaponList[best_weapon];     // flip over to the top ratings
      else
        prev = NULL;

    if (dir == 1)         // looking for 'next weapon'?
      new_weapon = next;
    else
      new_weapon = prev;  // looking for 'previous weapon'

    /*
    // what the fuck is this cvar?
    wp_change_order = gstate->GetCvar ("wp_change_order");

    if ( wp_change_order == 1 )
        new_weapon = next ? next : prev;
    else if ( wp_change_order == -1 )
        new_weapon = prev ? prev : next;
    */

	// [cek 11-14-99] This circumvents the 'normal' weapon select procedure.  Call the weapon's select,
	// which by default calls weaponsSelect...
    if ( new_weapon )
		(weaponInfo_t *)new_weapon->winfo->select_func(self);
//        weaponSelect (self, (weaponInfo_t *) new_weapon->winfo);
}



void weapprev_Command(userEntity_t *self)
{
  ManualNextWeapon(self,-1); // backward
}


void weapnext_Command(userEntity_t *self)
{
  ManualNextWeapon(self,1); // forward

}


void weap_Command(userEntity_t *self)
// activate/de-activate apon hud
{
  int hud_status = atoi(gstate->GetArgv(1));  // 0 = force off, 1 = force on

	gstate->WeaponDisplayUpdateClient(self, hud_status); // update client interface
}


///////////////////////////////////////////////////////////////////////////////
//
// bouncing shells
//
///////////////////////////////////////////////////////////////////////////////

static weaponInfo_t *g_pShellWeaponInfo;
static int			g_nShellSoundStartIndex, g_nMaxShellSounds;

///////////////////////////////////////////////////////////////////////
//  void shotShellTouch(userEntity_t *entShell)
//
//  Description:
//      Touch function for the casing
//
//  Parameters:
//      userEntity_t *entShell      passed by the engine. A pointer
//                                  to the shell itself
//
//  Return Value:   none
//
void shotShellTouch(userEntity_t *entShell, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
    CVector vecVel;
//    float   fSpeed;// SCG[1/24/00]: not used
    int     iSnd;

    if(entShell->velocity.x || entShell->velocity.y)
    {
        vecVel = entShell->velocity;

        //fSpeed = vecVel.Length();// SCG[1/24/00]: not used
        // play a casing hitting the ground sound

        iSnd = (int)(frand() * (g_nMaxShellSounds - 0.05));
        gstate->StartEntitySound(entShell,CHAN_AUTO,gstate->SoundIndex(g_pShellWeaponInfo->sounds[g_nShellSoundStartIndex + iSnd]),1.0f,ATTN_NORM_MIN,ATTN_NORM_MAX);

        // random angular velocity
//        entShell->avelocity.x = (frand() * 720);
//        entShell->avelocity.y = (frand() * 720);
//        entShell->avelocity.z = (frand() * 720);
    }
}

void shotShellThink(userEntity_t *entShell)
{
    entShell->s.alpha-=0.02;
	entShell->s.render_scale.Set(1,1,1);
//	entShell->s.angles.y = AngleMod(entShell->s.angles.y + 20);
//	entShell->s.angles.z = AngleMod(entShell->s.angles.z + 20);
    if((entShell->s.alpha < 0))// || (entShell->velocity.Length() < 10))
	{
//        int iSnd = (int)(frand() * (g_nMaxShellSounds - 0.05));
//        gstate->StartEntitySound(entShell,CHAN_AUTO,gstate->SoundIndex(g_pShellWeaponInfo->sounds[g_nShellSoundStartIndex + iSnd]),1.0f,ATTN_NORM_MIN,ATTN_NORM_MAX);
        entShell->remove(entShell);
	}
    else
        entShell->nextthink = gstate->time + 0.1;
}
///////////////////////////////////////////////////////////////////////
//  void shotShell(userEntity_t *entPlayer)
//
//  Description:
//      Spawns a shell casing, tosses it into the world, it makes some
//      nice casing bouncing sounds as it lands in the world, and
//      fades away like a gib.
//
//  Parameters: 
//      userEntity_t *entPlayer     the player shooting the shotcycler
//
//  Return Value: none
//
// SCG[7/13/99]: Make this useable by multiple weapons ( added modelname )
void shotShell(userEntity_t *entPlayer, weaponInfo_t *pWeaponInfo, int nShellModelIndex, int nShellSoundStartIndex, int nMaxShellSounds, float scale, CVector &offset )
{
    userEntity_t    *entShell;
    CVector         vecDir, vecForward, vecRight, vecUp;
    
    if(!dm_shotshells->value)
        return;
	
	g_pShellWeaponInfo = pWeaponInfo;
	g_nShellSoundStartIndex = nShellSoundStartIndex;
	g_nMaxShellSounds = nMaxShellSounds;

    entShell = gstate->SpawnEntity();
	entShell->className = "shot_shell";
//    entShell->s.modelindex = gstate->ModelIndex("models/e1/we_shotshell.dkm");
	entShell->flags |= FL_NOSAVE;
	entShell->s.render_scale.Set(scale,scale,scale);
	gstate->SetSize(entShell,-scale,-scale,-scale,scale,scale,scale);
    entShell->s.modelindex = gstate->ModelIndex(pWeaponInfo->models[nShellModelIndex]);
    entShell->movetype = MOVETYPE_BOUNCE;
	entShell->touch = shotShellTouch;
    entShell->s.renderfx = RF_TRANSLUCENT;
    entShell->s.alpha = 1.0;
    entShell->think = shotShellThink;
    entShell->nextthink = gstate->time + 0.1;
	entShell->owner = entPlayer;
    
    entShell->clipmask = MASK_SOLID;
    entShell->svflags = SVF_SHOT;
    entShell->solid = SOLID_BBOX;

    // calculate the direction / velocity the shell will travel (to the right of the player)
    vecDir = entPlayer->s.angles;//client->v_angle;
    vecDir.AngleToVectors(vecForward, vecRight, vecUp);

    // make the shells appear just in front and to the right of the player
    entShell->s.origin = entPlayer->s.origin + weaponHandVector(entPlayer, vecForward * offset.x, vecRight * offset.y);
    entShell->s.origin.z+=offset.z;//z_offset;

    // now apply velocity to the right of the player
    vecDir = vecRight * (frand() * 100 + 150);
    vecDir.z += 100;    // the shells pop up and out of the shotcycer
    entShell->velocity = vecDir;

    // random angular velocity
	entShell->s.angles.Set(90,0,0);
//    entShell->avelocity.x = (crand() * 200);
//    entShell->avelocity.y = (crand() * 200);
//    entShell->avelocity.z = (crand() * 200);
    
    gstate->LinkEntity(entShell);

}
///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
// global weapon registration functions
// e1 weapons
void weapon_c4_register_func();
void weapon_disruptor_register_func();
void weapon_gashands_register_func();
void weapon_ionblaster_register_func();
void weapon_shockwave_register_func();
void weapon_shotcycler_register_func();
void weapon_sidewinder_register_func();
// e2 weapons
void weapon_discus_register_func();
void weapon_hammer_register_func();
void weapon_sunflare_register_func();
void weapon_trident_register_func();
void weapon_venomous_register_func();
void weapon_zeus_register_func();
// e3 weapons
void weapon_ballista_register_func();
void weapon_bolter_register_func();
void weapon_nightmare_register_func();
void weapon_silverclaw_register_func();
void weapon_stavros_register_func();
void weapon_wyndrax_register_func();
// e4 weapons
void weapon_glock_register_func();
void weapon_kineticore_register_func();
void weapon_metamaser_register_func();
void weapon_novabeam_register_func();
void weapon_ripgun_register_func();
void weapon_slugger_register_func();
// global weapons
void weapon_daikatana_register_func();
//void weapon_dk_banish_register_func();
//void weapon_dk_barrier_register_func();
//void weapon_dk_dance_register_func();
//void weapon_dk_melee_register_func();
//void weapon_dk_posession_register_func();
// TODO:  dk_proteus
// misc "weapons"



// does some crazy stuff to figure out if we can damage the target.
// only the zeus can hurt medusa
void e2_com_Damage (  userEntity_t *target, 
					  userEntity_t *inflictor, 
					  userEntity_t *attacker, 
					  CVector &point_of_impact, 
					  CVector &damage_vec, 
					  float damage, 
					  unsigned long damage_flags )
{	
	if (attacker && (attacker->flags & FL_CLIENT))									// if we have an attacker that is a client
	{
		if (!target || (target && (IS_MONSTER_TYPE(target,TYPE_MEDUSA)||IS_MONSTER_TYPE(target,TYPE_CERBERUS))))	// and the target exists and is medusa
		{
			if (attacker->curWeapon)												// and the attacker has a weapon	
			{
				weapon_t *weapon = (weapon_t *)attacker->curWeapon;
				if (stricmp(weapon->name,"weapon_zeus") && IS_MONSTER_TYPE(target,TYPE_MEDUSA))					// if the weapon isn't zeus, get outa here
				{
					return;
				}
				if (stricmp(weapon->name,"weapon_trident") && IS_MONSTER_TYPE(target,TYPE_CERBERUS))					// if the weapon isn't zeus, get outa here
				{
					return;
				}
			}
			else
				return;
		}
	}

	gstate->damage_inflicted = 0;
	com->Damage(target,inflictor,attacker,point_of_impact,damage_vec,damage,damage_flags);
}

// yes, this is exactly com_radiusdamage (with some modifications)
int	e2_com_RadiusDamage (userEntity_t *inflictor, userEntity_t *attacker, userEntity_t *ignore, float damage, unsigned long damage_flags, float fRadius, CHitCounter *hit)
{
	userEntity_t	*head;
	float			damage_points;
	CVector			org, distance, v;
	int				hit_something;
	float			dist;

	hit_something = 0;
	head = gstate->FirstEntity();

	damage_flags &= ~(DAMAGE_DIR_TO_ATTACKER | DAMAGE_INFLICTOR_VEL);
	damage_flags |= DAMAGE_DIR_TO_INFLICTOR;

	for (head = gstate->FirstEntity(); head != NULL; head = gstate->NextEntity(head))
	{
		// find the distance to this entity
		if (head != ignore && head->takedamage)
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
            distance = org-inflictor->s.origin;
			dist = distance.Length();

			if (dist > fRadius)
			{
				//	cannot hit if more than damage units away
				continue;
			}

			damage_points = damage * (1.0 - ((dist*dist)/(fRadius*fRadius)));

			// do half damage on the owner (for rocket jumping)
			if (head == inflictor->owner)
			{
				damage_points *= 0.5;

				// cek[11-29-99] do even less if it's a sidewinder
				if (damage_flags & DAMAGE_SIDEWINDER)
					damage_points *= 0.65;
			}

			if (damage_points > 0)
			{
				if (hit)
					hit->AddHit(attacker,head);
				hit_something++;
				//	pass it zero_vector as dir because DAMAGE_DIR_TO_INFLICTOR flag will override dir anyway
				e2_com_Damage (head, inflictor, attacker, org, zero_vector, damage_points, damage_flags);
			}
		}
	}

	return	hit_something;
}

// yes, this is exactly com_radiusdamage (with some modifications)
int	counted_com_RadiusDamage (userEntity_t *inflictor, userEntity_t *attacker, userEntity_t *ignore, CHitCounter *hit, float damage, unsigned long damage_flags, float fRadius)
{
	userEntity_t	*head;
	float			damage_points;
	CVector			org, distance, v;
	int				hit_something;
	float			dist;

	hit_something = 0;
	head = gstate->FirstEntity();

	damage_flags &= ~(DAMAGE_DIR_TO_ATTACKER | DAMAGE_INFLICTOR_VEL);
	damage_flags |= DAMAGE_DIR_TO_INFLICTOR;

	for (head = gstate->FirstEntity(); head != NULL; head = gstate->NextEntity(head))
	{
		// find the distance to this entity
		if (head != ignore && head->takedamage)
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
            distance = org-inflictor->s.origin;
			dist = distance.Length();

			if (dist > fRadius)
			{
				//	cannot hit if more than damage units away
				continue;
			}

			damage_points = damage * (1.0 - ((dist*dist)/(fRadius*fRadius)));

			// do half damage on the owner (for rocket jumping)
			if (head == inflictor->owner)
			{
				damage_points *= 0.5;

				// cek[11-29-99] do even less if it's a sidewinder
				if (damage_flags & DAMAGE_SIDEWINDER)
					damage_points *= 0.65;
			}

			if (damage_points > 0)
			{
				hit_something++;
				if (hit)
					hit->AddHit(attacker,head);
				//	pass it zero_vector as dir because DAMAGE_DIR_TO_INFLICTOR flag will override dir anyway
				if (gstate->episode == 2)
					e2_com_Damage (head, inflictor, attacker, org, zero_vector, damage_points, damage_flags);
				else
					com->Damage (head, inflictor, attacker, org, zero_vector, damage_points, damage_flags);
//				gstate->Con_Dprintf("Counted rad damage: %f to %s\n",damage_points,head->netname);
			}
		}
	}

	return	hit_something;
}

// validates a target based on the passed flags
// ifVisible = if it passes all other tests, must be visible
// Cflags = client	(player/dm players)
// Bflags = bot		(sidekick)
// Mflags = monsters
// Oflags = stuff that takes damage that isn't a C/M/B
short validateTarget(userEntity_t *self, userEntity_t *target, int ifVisible, int Cflags, int Mflags, int Bflags, int Oflags)
{
	if (!self || !target || !target->inuse || !target->takedamage)
		return 0;

	short isDM = deathmatch->value;
	short isCOOP = coop->value;
	short isSingle = (isDM || isCOOP) ? 0 : 1;

	short passed = 0;

	// determine which set of flags we should use to test
	int flags = Oflags;
	if (target->flags & FL_CLIENT)
		flags = Cflags;
	else if (target->flags & FL_MONSTER)
		flags = Mflags;
	else if (target->flags & FL_BOT)
		flags = Bflags;

	// handle really easy cases like always and never
	if (flags & TARGET_ALWAYS)
		passed = 1;
	else if (flags & TARGET_ALWAYS_IF_ALIVE)
	{
		if (target->deadflag == DEAD_NO)
			passed = 1;
	}
	else if (flags == TARGET_NEVER)
		passed = 0;
	else
	{
		// check targeting self's owner
		if (self->owner == target)
		{
			if (!(flags & TARGET_OWNER))
				return 0;
			else
			{
				passed = 1;
				goto done;
			}
		}
		// if target is self and TARGET_SELF is not set, just exit 0
		if (self == target)
		{
			if (!(flags & TARGET_SELF) )
				return 0;
			else
			{
				passed = 1;
				goto done;
			}
		}

		// check the teams for clients and bots (only if friendly fire is on!)
		cvar_t *ctf = gstate->cvar("ctf","0",CVAR_SERVERINFO|CVAR_LATCH);
		if ( (ctf->value || (dm_teamplay->value && dm_friendly_fire->value)) &&		// only if teamplay and ff are on!
			 (self->flags & (FL_CLIENT|FL_BOT))	&&					// only for clients/bots
			 (target->flags & (FL_CLIENT|FL_BOT)) &&				// ditto
			 (com->TeamCheck(self,target)) &&						// same team
			 !(flags & TARGET_TEAM) )								// same team damage flag not set
					
			 return 0;

		passed = (isSingle && (flags & TARGET_SINGLE)) ||
				 (isDM && (flags & TARGET_DM)) ||
				 (isCOOP && (flags & TARGET_COOP));
	}

done:
	if (passed)
	{
		if (ifVisible)
			return com->Visible(self, target) ? 1 : 0;
		else
			return passed;
	}
	else
		return 0;
}
///////////////////////////////////////////////////////////////////////
//  selectTarget(userEntity_t *entSelf, CVector &vecDeviation, int iOption)
//
//  Description:
//      selectTarget() iterates through the entity list and chooses a 
//      a suitable target based on deviation and whether the target is
//      a client/monster/target and will take damage. The best target
//      for the selection type is returned.
//
//      If iOption is SELECT_TARGET_PATH, then a target that most
//      closely matches self's path is returned. If iOption is 
//      SELECT_TARGET_RANGE, then the closest entity to self is returned.
//
//  Parameters:
//      entSelf         Entity that is selecting a target
//      vecDeviation    3 floats describing total distance, and a 
//                      normalized vector difference of direction used to 
//                      make selections. E.g. vecDeviation(0.25, 0.25, 1000)
//                      and iOption = SELECT_TARGET_PATH, will only consider
//                      targets within 1000 units of entSelf, and only if 
//                      the difference between the normalized vector of 
//                      entSelf's velocity and the normalized vector 
//                      describing the difference between the entSelf's origin
//                      and the target's origin are < 0.25 for x and 0.25 for y. 
//                      In other words about a 45 degree conical spread for 1000
//                      units
//      iOption         SELECT_TARGET_PATH or SELECT_TARGET_RANGE
//
//  Return:
//      The entity best suited for targeting or NULL of no entity was
//      discovered.
//
//  Example:
//      entNear = selectTarget(self, CVector(0.25, 0.25, 1000), SELECT_TARGET_PATH)
//
//  ToDo:
//      Perhaps have a separate list of targetable entities in the game to expedite
//      the search currently handled by FindRadius().
//
userEntity_t *selectTarget(userEntity_t *entSelf, CVector &vecDeviation, int iOption = 0, userEntity_t *entStart = NULL)
{
    userEntity_t    *entReturn;
    userEntity_t    *entConsider;
    CVector         vecPath, vecGoalDir;
    trace_t         tr;
    float           fDeviationX, fDeviationY, fSmallestDeviation=9999;

    entReturn = NULL;
    entConsider = entStart; // start considering here

    if(iOption == SELECT_TARGET_PATH)
    {
        fSmallestDeviation = 9999.0f;
        if(entSelf->flags & FL_CLIENT)
        {
            vecPath = entSelf->s.angles;
            vecPath.AngleToForwardVector(vecPath);
        } else {
            vecPath = entSelf->velocity;
            vecPath.Normalize();
        }
        while((entConsider = com->FindRadius(entConsider, entSelf->s.origin, vecDeviation.z)) != NULL)
        {
            // qualify the target
            if( ((entConsider->takedamage) || 
                (entConsider->flags & FL_CLIENT) || 
                (entConsider->flags & FL_BOT) || 
                (entConsider->flags & FL_MONSTER)) && 
                entConsider != entSelf->owner && 
                entConsider != entSelf)
            {
                tr = gstate->TraceLine_q2(entSelf->s.origin, entConsider->s.origin, entSelf, MASK_SOLID);
                if(tr.fraction >= 1.0)  // traced all the way to the target successfully
                {
                    vecGoalDir = entConsider->s.origin - entSelf->s.origin;
                    vecGoalDir.Normalize();
                    vecGoalDir = vecPath - vecGoalDir;
                    fDeviationX = fabs(vecGoalDir.x);
                    fDeviationY = fabs(vecGoalDir.y);
                    if(fDeviationX < vecDeviation.x && fDeviationY < vecDeviation.y)
                    {
                        if(fSmallestDeviation > fDeviationX + fDeviationY)
                        {
                            fSmallestDeviation = fDeviationX + fDeviationY;
                            entReturn = entConsider;
                        }
                    }
                }
            }
        }
    }
    else if (iOption == SELECT_TARGET_RANGE)
    {
        while( (entConsider = com->FindRadius(entConsider, entSelf->s.origin, vecDeviation.z)) != NULL)
        {
            if( ((entConsider->takedamage) || 
                (entConsider->flags & FL_CLIENT) || 
                (entConsider->flags & FL_BOT) || 
                (entConsider->flags & FL_MONSTER)) && 
                entConsider != entSelf->owner && 
                entConsider != entSelf)
            {
                tr = gstate->TraceLine_q2(entSelf->s.origin, entConsider->s.origin, entSelf, MASK_SOLID);
                if(tr.fraction >= 1.0)
                {
                    vecGoalDir = entConsider->s.origin - entSelf->s.origin;
                    if(fSmallestDeviation > vecGoalDir.Length())
                    {
                        fSmallestDeviation = vecGoalDir.Length();
                        entReturn = entConsider;
                    }
                }
            }
        }
    }
    return entReturn;
}
//
///////////////////////////////////////////////////////////////////////


CVector turnToTarget(userEntity_t *entSelf, userEntity_t *entTarget, float fTurnRate)
{
    CVector     vecGoalDir, vecChangeDir, vecReturn;
    float       fVelocity;

    if(entSelf && entTarget)
    {
		CVector vecOrigin;
		if (entTarget->solid == SOLID_BSP)
		{
			vecOrigin = (entTarget->absmax + entTarget->absmin) * 0.5;
		}
		else
		{
			vecOrigin = entTarget->s.origin;
		}

		fVelocity = entSelf->velocity.Length();
//		vecGoalDir = entTarget->s.origin - entSelf->s.origin;
		vecGoalDir =vecOrigin - entSelf->s.origin;
		if(fTurnRate < 1.0f)
		{
			// triangulation
			vecChangeDir = entSelf->velocity - vecGoalDir;
			vecChangeDir = vecChangeDir * fTurnRate;
			vecGoalDir = entSelf->velocity - vecChangeDir;
			vecGoalDir.Normalize();
			vecReturn = vecGoalDir * fVelocity;
		} 
		else 
		{
			// home in directly
			vecGoalDir.Normalize();
			vecReturn = vecGoalDir * fVelocity;
		}
    }
	else
		vecReturn.Set(0,0,0);

    return vecReturn;
}

int EntIsAlive( userEntity_t *self )
{
	if( self == NULL )
	{
		return FALSE;
	}

	if ( self->deadflag == DEAD_NO && self->health > 0 && self->inuse &&
		 _stricmp( self->className, "freed" ) != 0 &&
		 _stricmp( self->className, "noclass" ) != 0 )
	{
		return TRUE;
	}

	if( ( self->solid == SOLID_BSP ) )// SCG[10/6/99]: If this is a bsp entity, return true.
	{
		return TRUE;
	}

	return FALSE;
}

qboolean winfoCheckMaxCount(userEntity_t *self, weaponInfo_t *winfo)
{
	// this condition is ok...we'll have an inventory when we're done...
	if (!self || !self->inventory)
		return TRUE;

	weapon_t *item = (weapon_t *)gstate->InventoryFindItem(self->inventory,winfo->weaponName);
	if (!item)
		return TRUE;

	if (item->ammo->count >= winfo->ammo_max)
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------------------------------
// cool debris spawn functions!
//------------------------------------------------------------------------------------------------------
#define MAX_DEBRIS			20
static short				debris_count=-1;
static userEntity_t			*debris_list[MAX_DEBRIS];

char *debris_models [] =   {"models/global/e_wood1.dkm",
							"models/global/e_wood2.dkm",
							"models/global/e_glass1.dkm",
							"models/global/e_glass2.dkm",
							"models/global/e_metal1.dkm",
							"models/global/e_metal2.dkm",
							"models/global/e_gibrobot1.dkm",
							"models/global/e_gibrobot2.dkm",
							"models/global/e_gibrobot3.dkm",
							"models/global/e_gibrobot4.dkm",
							"models/global/e_rock1.dkm",
							"models/global/e_rock2.dkm",
							"models/global/e_rock3.dkm",
							"models/global/g_bone.dkm",
							"models/e4/we_ice.sp2"};

short debris_mass[] =	{15,15,			// wood
						 15,15,			// glass
						 20,20,			// metal
						 20,20,20,20,	// robot(metal)
						 25,25,25,		// rock
						 15,			// bone
						 15				// ice
						};

char *debris_sounds[] = {	"global/e_woodbreak.wav",		// wood sounds 0,5
							"global/e_woodbreaksa.wav",
							"global/e_woodbreaksb.wav",
							"global/e_woodbreaksc.wav",
							"global/e_woodbreakse.wav",
							"global/e_glassbreaksa.wav",	// glass 5,6
							"global/e_glassbreaksb.wav",
							"global/e_glassbreaksc.wav",
							"global/e_glassbreaksd.wav",
							"global/e_glassbreakse.wav",
							"global/e_glassbreaksf.wav",
							"global/e_metalbreaksa.wav",	// metal,robot 11,6
							"global/e_metalbreaksb.wav",
							"global/e_metalbreaksc.wav",
							"global/e_metalbreaksd.wav",
							"global/e_metalbreakse.wav",
							"global/e_metalbreaksf.wav",
							"global/e_rocktumble1.wav",		// rock 17,3
							"global/e_rocktumble2.wav",
							"global/e_rocktumble3.wav",
							"global/e_gravelc.wav"
						};

#define DEBRIS_WOODSND_START	0
#define DEBRIS_WOODSND_CNT		5

#define DEBRIS_GLASSSND_START	5
#define DEBRIS_GLASSSND_CNT		6

#define DEBRIS_METALSND_START	11
#define DEBRIS_METALSND_CNT		6

#define DEBRIS_ROCKSND_START	17
#define DEBRIS_ROCKSND_CNT		3

#define DEBRIS_SAND_START		20
#define DEBRIS_SAND_CNT			1

void debris_sound(userEntity_t *self, int type, float minAttn, float maxAttn)
{
	if ((type < 0) || (type >= DEBRIS_MAX) || !self)
		return;

	int index;
	switch(type)
	{
	case DEBRIS_WOOD1:
	case DEBRIS_WOOD2:
		index = DEBRIS_WOODSND_START+(short)(frand()*(DEBRIS_WOODSND_CNT-0.05));
		break;

	case DEBRIS_GLASS1:
	case DEBRIS_GLASS2:
		index = DEBRIS_GLASSSND_START+(short)(frand()*(DEBRIS_GLASSSND_CNT-0.05));
		break;

	case DEBRIS_METAL1:
	case DEBRIS_METAL2:
	case DEBRIS_ROBOT1:
	case DEBRIS_ROBOT2:
	case DEBRIS_ROBOT3:
	case DEBRIS_ROBOT4:
		index = DEBRIS_METALSND_START+(short)(frand()*(DEBRIS_METALSND_CNT-0.05));
		break;

	case DEBRIS_ROCK1:
	case DEBRIS_ROCK2:
	case DEBRIS_ROCK3:
		index = DEBRIS_ROCKSND_START+(short)(frand()*(DEBRIS_ROCKSND_CNT-0.05));
		break;

	case DEBRIS_SAND: // todo: get a cool sound for this
		index = DEBRIS_SAND_START;
	default:
	case DEBRIS_BONE:
	case DEBRIS_ICE:
		index = -1;
	};
	if (index == -1)
		return;

	gstate->StartEntitySound(self,CHAN_AUTO,gstate->SoundIndex(debris_sounds[index]),1.0,minAttn,maxAttn);
}

int debris_get_index(csurface_t *surf)
{
	if (!surf || !(surf->flags & (SURF_WOOD|SURF_METAL|SURF_STONE|SURF_STONE|SURF_GLASS|SURF_SNOW|SURF_ICE|SURF_SAND)))
		return -1;

//	userEntity_t *result = gstate->SpawnEntity ();// SCG[1/24/00]: not used

	if (surf->flags & SURF_WOOD)
	{
		return DEBRIS_WOOD+(short)(frand()*(DEBRIS_GLASS-DEBRIS_WOOD-0.05));
	}
	else if (surf->flags & SURF_METAL)
	{
		return DEBRIS_METAL+(short)(frand()*(DEBRIS_ROBOT-DEBRIS_METAL-0.05));
	}
	else if (surf->flags & SURF_STONE)
	{
		return DEBRIS_ROCK+(short)(frand()*(DEBRIS_BONE-DEBRIS_ROCK-0.05));
	}
	else if (surf->flags & SURF_GLASS)
	{
		return DEBRIS_GLASS+(short)(frand()*(DEBRIS_METAL-DEBRIS_GLASS-0.05));
	}
	else if (surf->flags & (SURF_SNOW|SURF_ICE))
	{
		return DEBRIS_ICE;
	}
	else if (surf->flags & SURF_SAND)
	{
		return DEBRIS_SAND;
	}
	else
		return -1;
}

int debris_get_index(int type)
{
	if ((type < 0) || (type >= DEBRIS_MAX))
		return -1;

	switch(type)
	{
	case DEBRIS_WOOD1:
	case DEBRIS_WOOD2:
		return DEBRIS_WOOD+(short)(frand()*(DEBRIS_GLASS-DEBRIS_WOOD-0.05));

	case DEBRIS_GLASS1:
	case DEBRIS_GLASS2:
		return DEBRIS_GLASS+(short)(frand()*(DEBRIS_METAL-DEBRIS_GLASS-0.05));

	case DEBRIS_METAL1:
	case DEBRIS_METAL2:
		return DEBRIS_METAL+(short)(frand()*(DEBRIS_ROBOT-DEBRIS_METAL-0.05));

	case DEBRIS_ROBOT1:
	case DEBRIS_ROBOT2:
	case DEBRIS_ROBOT3:
	case DEBRIS_ROBOT4:
		return DEBRIS_ROBOT+(short)(frand()*(DEBRIS_ROCK-DEBRIS_ROBOT-0.05));

	default:
	case DEBRIS_ROCK1:
	case DEBRIS_ROCK2:
	case DEBRIS_ROCK3:
		return DEBRIS_ROCK+(short)(frand()*(DEBRIS_BONE-DEBRIS_ROCK-0.05));

	case DEBRIS_BONE:
	case DEBRIS_ICE:
	case DEBRIS_SAND:
		return type;
	};

	// SCG[1/24/00]: need a default return value!
	return -1;
}


int w_debris_remove(userEntity_t *self)
{
	int i;
	if (!self)		// remove the oldest entry (most faded)
	{
		float lowAlpha = 500000;
		int result = -1;
		for (i = 0; i < MAX_DEBRIS; i++)
		{
			if (debris_list[i])
			{
				if (debris_list[i]->health < lowAlpha)
				{
					result = i;
					lowAlpha = debris_list[i]->health;
				}
			}
		}

		if (result != -1)
		{
			RELIABLE_UNTRACK(debris_list[result]);
			gstate->RemoveEntity(debris_list[result]);
			debris_list[result] = NULL;
			debris_count--;
			return result;
		}
	}
	else			// remove specific entry
	{
		for (i = 0; i < MAX_DEBRIS; i++)
		{
			if (debris_list[i] == self)
			{
				debris_list[i] = NULL;
				debris_count--;
				return i;
			}
		}
	}
	return -1;
}

void w_debris_add(userEntity_t *self)
{
	int insert = -1;
	if (debris_count >= MAX_DEBRIS)
	{
		insert = w_debris_remove(NULL);
	}
	else
	{
		for (int i = 0; i < MAX_DEBRIS; i++)
		{
			if (!debris_list[i])
			{
				insert = i;
				break;
			}
		}
	}
	if (insert == -1)
		return;

	debris_list[insert] = self;
	debris_count++;
}
void w_debris_fall2(userEntity_t *self)
{
	CVector	vel;

	self->s.alpha -= self->hacks;

	if(self->s.alpha <= 0.01)
	{
		self->remove(self);
		return;
	}
	self->nextthink = gstate->time + 0.3f;
}

void w_debris_StartFall(userEntity_t *self)
{
	RELIABLE_UNTRACK(self);
	if(self->velocity.Length() > 10)
	{
		self->s.angles.yaw		+= self->mass+(rnd()*10);
		self->s.angles.roll		+= self->mass+(rnd()*10);
	}
	if(self->delay < gstate->time)
	{
		self->touch = NULL;
		self->think = w_debris_fall2;
	}
	self->nextthink = gstate->time + 0.1f;
}

void debris_touch(userEntity_t *self, userEntity_t *other,cplane_t *plane, csurface_t *surf)
{
	if (!self->userHook)
		return;

	weaponDebrisHook_t *hook=(weaponDebrisHook_t *)self->userHook;

	if ((other->takedamage) && (other->flags & (FL_CLIENT|FL_BOT|FL_MONSTER )))
	{
		trace_t tr;
		CVector dst = self->s.origin;
		dst.z -= 50;
		tr = gstate->TraceLine_q2( self->s.origin, dst, self, MASK_MONSTERSOLID);
		dst = tr.endpos - self->s.origin;
		if (dst.Length() > 5)
			com->Damage (other, self, hook->owner, self->s.origin, zero_vector, hook->damage, DAMAGE_NONE);
	}
}

void debris_remove(userEntity_t *self)
{
	if (!self)
		return;

	w_debris_remove(self);
	if (!stricmp(self->className,"freed"))
		return;

	RELIABLE_UNTRACK(self);
	gstate->RemoveEntity(self);
}

void debris_init (debrisInfo_t &di)
{
	if (di.type == -1)
		return;

	userEntity_t *debris;

	float speed = di.speed;
	float spread = di.spread;
	CVector ang,forward,origin;
	CVector scale = di.scale;
	float r,sp=0.1*spread;

	for (int i = 0; i < di.count; i ++)
	{
		debris = gstate->SpawnEntity ();
		if (!debris)
			return;

		debris->flags |= FL_NOSAVE;
		w_debris_add(debris);

		debris->mass = debris_mass[di.type];
		debris->className	= "debris";
		debris->s.frame		= 0;
		debris->flags		= 0;
		debris->takedamage	= DAMAGE_NO;
		debris->svflags		= SVF_SHOT|SVF_DEADMONSTER;
		debris->remove		= debris_remove;

		if (di.owner && (di.damage > 0) && (di.type != DEBRIS_SAND)) // sand is a special beast
		{
			debris->solid = SOLID_BBOX;
			debris->clipmask	= MASK_SOLID;
			debris->touch = debris_touch;
			debris->userHook = gstate->X_Malloc(sizeof(weaponDebrisHook_t),MEM_TAG_HOOK);
			weaponDebrisHook_t *hook=(weaponDebrisHook_t*)debris->userHook;
			hook->damage = di.damage;
			hook->owner = di.owner;
		}
		else
		{
			debris->clipmask	= MASK_SOLID;
			debris->solid		= SOLID_NOT;
			debris->touch = NULL;
		}

		float rf = di.scale_delta;
		scale.Set(scale.x *( 1 + rf*crand()), scale.y *( 1 + rf*crand()), scale.z *( 1 + rf*crand()));
		debris->s.render_scale = scale;
		debris->mass *= (scale.x+scale.y+scale.z)*0.33;
		debris->max_speed = speed;
		debris->gravity = di.gravity;

		VectorToAngles(di.dir,ang);
		ang.yaw += crand()*spread;
		ang.pitch += crand()*spread;
		ang.AngleToForwardVector(forward);
		forward.Normalize();

		if (di.type == DEBRIS_SAND)		// really just makes a complex particle generator that doesn't move!
		{
			di.particles = PARTICLE_SIMPLE;
			debris->movetype	= MOVETYPE_NONE;
			debris->s.renderfx = RF_TRANSLUCENT;
			debris->velocity.Set(0,0,0);
			debris->s.angles.yaw = 360*frand();
		}
		else
		{
			debris->movetype	= MOVETYPE_BOUNCE;
			debris->s.modelindex = gstate->ModelIndex (debris_models[di.type]);
			debris->s.renderfx = RF_TRANSLUCENT;
			debris->velocity = (speed*0.55 + (frand()*speed*0.45)) * forward;
			debris->s.angles.yaw = 360*frand();
		}

		debris->s.alpha = di.alpha;

		r = spread ? frand()*sp - ((float)sp/2) : 0;
		origin.x = di.org.x + r;
		r = spread ? frand()*sp - ((float)sp/2) : 0;
		origin.y = di.org.y + r;
		origin.z = di.org.z;
		scale += CVector(0.4,0.4,1.0);
		gstate->SetOrigin2(debris,origin);
		gstate->SetSize(debris,-scale.x,-scale.y,-scale.z ,scale.x,scale.y,scale.z);

		debris->health = gstate->time;
		debris->delay = gstate->time + di.delay;
		debris->hacks = di.alpha_decay;
		debris->think = w_debris_StartFall;
		debris->nextthink = gstate->time + 0.1;

		gstate->LinkEntity(debris);

		if ((di.particles != -1) && (di.particles < NUM_PARTICLETYPES) )// attach the track ent thinggy here.
		{
			trackInfo_t tinfo;
			memset(&tinfo, 0, sizeof(tinfo));

			tinfo.ent=debris;
			tinfo.srcent=debris;

			tinfo.length		= 0.8;						// alpha
			tinfo.lightColor	= di.pColor;				// color
			tinfo.lightSize		= 0.4;						// delta alpha
			tinfo.modelindex	= di.particles;
			tinfo.scale			= di.pScale;				// scale
			tinfo.frametime		= 2;						// emission frequency
			tinfo.altpos.x		= 10;						// particle count
			tinfo.altpos.y		= di.speed;					// speed

			tinfo.altangle.x	= 1;						// emission toggle
			tinfo.altangle.y	= 1;						// emission time
			tinfo.numframes     = di.spread;				// spread
			tinfo.altpos2		= di.dir;					// direction
			tinfo.modelindex2	= 0;						// random?
			tinfo.dstpos		= CVector(0,0,-3);			// gravity!
			tinfo.fxflags		= TEF_COMPLEXPARTICLE|TEF_FX_ONLY;
			
			tinfo.flags = TEF_LIGHTCOLOR|TEF_LIGHTSIZE|TEF_FXFLAGS|TEF_SRCINDEX|TEF_SCALE|TEF_ALTPOS|TEF_MODELINDEX|TEF_NUMFRAMES|TEF_LENGTH|TEF_FRAMETIME|TEF_DSTPOS|TEF_ALTPOS2|TEF_ALTANGLE|TEF_MODELINDEX2|TEF_HARDPOINT|TEF_DSTPOS;
    
			com->trackEntity(&tinfo,MULTICAST_ALL);
		}
	}
}

void spawn_surface_debris(debrisInfo_t &di, bool bForceDeathmatch)
{
	if (deathmatch->value && !bForceDeathmatch)
		return;

	if (debris_count == -1)
	{
		debris_count = 0;
		memset(&debris_list,0,sizeof(debris_list));
	}

	if ((!di.surf) && (di.type < 0) || (di.type >= DEBRIS_MAX))
		return;

	if (di.surf)							// do the surface-style spawn!
	{
		int count = di.count;
		di.count = 1;						// do em one at a time
		for (int i = 0; i < count; i++)
		{
			di.type = debris_get_index(di.surf);
			debris_init(di);
		}
	}
	else									// type specified
	{
		if (di.bRandom)
		{
			int count = di.count;
			di.count = 1;
			for (int i = 0; i < count; i++)
			{
				di.type = debris_get_index(di.type);
				debris_init(di);
			}
		}
		else
			debris_init(di);
	}

	if (di.bSound && di.soundSource)
	{
		float min = di.minAttn;
		float max = di.maxAttn;
		if (min < 1)
			min = ATTN_NORM_MIN;
		if (max< 1)
			max = ATTN_NORM_MAX;

		debris_sound(di.soundSource, di.type,min,max);
	}
}

short disruptorSelect( userEntity_t *self );
short ionblaster_select( userEntity_t *self );
short c4Select( userEntity_t *self );
short shotcyclerSelect( userEntity_t *self );
short sidewinder_select( userEntity_t *self );
short shockwaveSelect( userEntity_t *self );
short discusSelect( userEntity_t *self );
short venomousSelect( userEntity_t *self );
short sunflareSelect( userEntity_t *self );
short hammerSelect( userEntity_t *self );
short tridentSelect( userEntity_t *self );
short zeus_select( userEntity_t *self );
short silverclaw_select( userEntity_t *self );
short bolter_select( userEntity_t *self );
short stavrosSelect( userEntity_t *self );
short ballistaSelect( userEntity_t *self );
short wyndraxSelect( userEntity_t *self );
short nightmareSelect( userEntity_t *self );
short glock_select( userEntity_t *self );
short slugger_select( userEntity_t *self );
short kineticore_select( userEntity_t *self );
short ripgun_select( userEntity_t *self );
short novabeam_select( userEntity_t *self );
short metamaser_select( userEntity_t *self );
short daikatana_select( userEntity_t *self );

userInventory_t *disruptorGive( userEntity_t *self, int ammoCount );
userInventory_t *ionblaster_give( userEntity_t *self, int ammoCount );
userInventory_t *c4Give( userEntity_t *self, int ammoCount );
userInventory_t *shotcyclerGive( userEntity_t *self, int ammoCount );
userInventory_t *sidewinder_give( userEntity_t *self, int ammoCount );
userInventory_t *shockwaveGive( userEntity_t *self, int ammoCount );
userInventory_t *discusGive( userEntity_t *self, int ammoCount );
userInventory_t *venomousGive( userEntity_t *self, int ammoCount );
userInventory_t *sunflareGive( userEntity_t *self, int ammoCount );
userInventory_t *hammerGive( userEntity_t *self, int ammoCount );
userInventory_t *tridentGive( userEntity_t *self, int ammoCount );
userInventory_t *zeus_give( userEntity_t *self, int ammoCount );
userInventory_t *silverclaw_give( userEntity_t *self, int ammoCount );
userInventory_t *bolter_give( userEntity_t *self, int ammoCount );
userInventory_t *stavrosGive( userEntity_t *self, int ammoCount );
userInventory_t *ballistaGive( userEntity_t *self, int ammoCount );
userInventory_t *wyndraxGive( userEntity_t *self, int ammoCount );
userInventory_t *nightmareGive( userEntity_t *self, int ammoCount );
userInventory_t *glock_give( userEntity_t *self, int ammoCount );
userInventory_t *slugger_give( userEntity_t *self, int ammoCount );
userInventory_t *kineticore_give( userEntity_t *self, int ammoCount );
userInventory_t *ripgun_give( userEntity_t *self, int ammoCount );
userInventory_t *novabeam_give( userEntity_t *self, int ammoCount );
userInventory_t *metamaser_give( userEntity_t *self, int ammoCount );
userInventory_t *daikatana_give( userEntity_t *self, int ammoCount );


short( *weapon_select[4][7] )( userEntity_t *self ) = {
// SCG[12/7/99]: Episode 1
	{ disruptorSelect, ionblaster_select, c4Select, shotcyclerSelect, sidewinder_select, shockwaveSelect, daikatana_select, },
// SCG[12/7/99]: Episode 2
	{ discusSelect, venomousSelect, sunflareSelect, hammerSelect, tridentSelect, zeus_select, daikatana_select, },
// SCG[12/7/99]: Episode 3
	{ silverclaw_select, bolter_select, stavrosSelect, ballistaSelect, wyndraxSelect, nightmareSelect, daikatana_select, },
// SCG[12/7/99]: Episode 4
	{ glock_select, slugger_select, kineticore_select, ripgun_select, novabeam_select, metamaser_select, daikatana_select, },
};

userInventory_t *( *weapon_give[4][7] )( userEntity_t *self, int ammoCount ) = {
// SCG[12/7/99]: Episode 1
	{ disruptorGive, ionblaster_give, c4Give, shotcyclerGive, sidewinder_give, shockwaveGive, daikatana_give, },
// SCG[12/7/99]: Episode 2
	{ discusGive, venomousGive, sunflareGive, hammerGive, tridentGive, zeus_give, daikatana_give, },
// SCG[12/7/99]: Episode 3
	{ silverclaw_give, bolter_give, stavrosGive, ballistaGive, wyndraxGive, nightmareGive, daikatana_give, },
// SCG[12/7/99]: Episode 4
	{ glock_give, slugger_give, kineticore_give, ripgun_give, novabeam_give, metamaser_give, daikatana_give, },
};

#define VALID_EPISODE			((gstate->episode >= 1) && (gstate->episode <= 4 ))

short weapon_select_1( userEntity_t *self ) { return (VALID_EPISODE) ? weapon_select[gstate->episode - 1][0]( self ) : WEAPON_UNAVAILABLE; }
short weapon_select_2( userEntity_t *self ) { return (VALID_EPISODE) ? weapon_select[gstate->episode - 1][1]( self ) : WEAPON_UNAVAILABLE; }
short weapon_select_3( userEntity_t *self ) { return (VALID_EPISODE) ? weapon_select[gstate->episode - 1][2]( self ) : WEAPON_UNAVAILABLE; }
short weapon_select_4( userEntity_t *self ) { return (VALID_EPISODE) ? weapon_select[gstate->episode - 1][3]( self ) : WEAPON_UNAVAILABLE; }
short weapon_select_5( userEntity_t *self ) { return (VALID_EPISODE) ? weapon_select[gstate->episode - 1][4]( self ) : WEAPON_UNAVAILABLE; }
short weapon_select_6( userEntity_t *self ) { return (VALID_EPISODE) ? weapon_select[gstate->episode - 1][5]( self ) : WEAPON_UNAVAILABLE; }
short weapon_select_7( userEntity_t *self ) { return (VALID_EPISODE) ? weapon_select[gstate->episode - 1][6]( self ) : WEAPON_UNAVAILABLE; }

userInventory_t *weapon_give_1( userEntity_t *self, int ammoCount ) { return (VALID_EPISODE) ? weapon_give[gstate->episode - 1][0]( self, ammoCount ) : NULL; }
userInventory_t *weapon_give_2( userEntity_t *self, int ammoCount ) { return (VALID_EPISODE) ? weapon_give[gstate->episode - 1][1]( self, ammoCount ) : NULL; }
userInventory_t *weapon_give_3( userEntity_t *self, int ammoCount ) { return (VALID_EPISODE) ? weapon_give[gstate->episode - 1][2]( self, ammoCount ) : NULL; }
userInventory_t *weapon_give_4( userEntity_t *self, int ammoCount ) { return (VALID_EPISODE) ? weapon_give[gstate->episode - 1][3]( self, ammoCount ) : NULL; }
userInventory_t *weapon_give_5( userEntity_t *self, int ammoCount ) { return (VALID_EPISODE) ? weapon_give[gstate->episode - 1][4]( self, ammoCount ) : NULL; }
userInventory_t *weapon_give_6( userEntity_t *self, int ammoCount ) { return (VALID_EPISODE) ? weapon_give[gstate->episode - 1][5]( self, ammoCount ) : NULL; }
userInventory_t *weapon_give_7( userEntity_t *self, int ammoCount ) { return (VALID_EPISODE) ? weapon_give[gstate->episode - 1][6]( self, ammoCount ) : NULL; }

/*
short weapon_select_1( userEntity_t *self ) { return weapon_select[gstate->episode - 1][0]( self ); }
short weapon_select_2( userEntity_t *self ) { return weapon_select[gstate->episode - 1][1]( self ); }
short weapon_select_3( userEntity_t *self ) { return weapon_select[gstate->episode - 1][2]( self ); }
short weapon_select_4( userEntity_t *self ) { return weapon_select[gstate->episode - 1][3]( self ); }
short weapon_select_5( userEntity_t *self ) { return weapon_select[gstate->episode - 1][4]( self ); }
short weapon_select_6( userEntity_t *self ) { return weapon_select[gstate->episode - 1][5]( self ); }
short weapon_select_7( userEntity_t *self ) { return weapon_select[gstate->episode - 1][6]( self ); }

userInventory_t *weapon_give_1( userEntity_t *self, int ammoCount ) { return weapon_give[gstate->episode - 1][0]( self, ammoCount ); }
userInventory_t *weapon_give_2( userEntity_t *self, int ammoCount ) { return weapon_give[gstate->episode - 1][1]( self, ammoCount ); }
userInventory_t *weapon_give_3( userEntity_t *self, int ammoCount ) { return weapon_give[gstate->episode - 1][2]( self, ammoCount ); }
userInventory_t *weapon_give_4( userEntity_t *self, int ammoCount ) { return weapon_give[gstate->episode - 1][3]( self, ammoCount ); }
userInventory_t *weapon_give_5( userEntity_t *self, int ammoCount ) { return weapon_give[gstate->episode - 1][4]( self, ammoCount ); }
userInventory_t *weapon_give_6( userEntity_t *self, int ammoCount ) { return weapon_give[gstate->episode - 1][5]( self, ammoCount ); }
userInventory_t *weapon_give_7( userEntity_t *self, int ammoCount ) { return weapon_give[gstate->episode - 1][6]( self, ammoCount ); }
*/
void weapon_funcs_register_func()
{
	gstate->RegisterFunc("weaponEntRemove",weaponEntRemove);
	gstate->RegisterFunc("weaponEntThink",weaponEntThink);
	gstate->RegisterFunc("weaponTouchDefault",weaponTouchDefault);
	gstate->RegisterFunc("winfoDroppedItemThink",ammoTouchDefault);
	gstate->RegisterFunc("ammoTouchDefault",ammoTouchDefault);
	gstate->RegisterFunc("shotShellTouch",shotShellTouch);
	gstate->RegisterFunc("shotShellThink",shotShellThink);

	gstate->RegisterFunc( "touch_hook_save", touch_hook_save );
	gstate->RegisterFunc( "touch_hook_load", touch_hook_load );
	gstate->RegisterFunc( "ammo_touch_hook_save", ammo_touch_hook_save );
	gstate->RegisterFunc( "ammo_touch_hook_load", ammo_touch_hook_load );

	// register e1 weapon funcs
	weapon_c4_register_func();
	weapon_disruptor_register_func();
	weapon_gashands_register_func();
	weapon_ionblaster_register_func();
	weapon_shockwave_register_func();
	weapon_shotcycler_register_func();
	weapon_sidewinder_register_func();
	// register e2 weapon funcs
	weapon_discus_register_func();
	weapon_hammer_register_func();
	weapon_sunflare_register_func();
	weapon_trident_register_func();
	weapon_venomous_register_func();
	weapon_zeus_register_func();
	// register e3 weapon funcs
	weapon_ballista_register_func();
	weapon_bolter_register_func();
	weapon_nightmare_register_func();
	weapon_silverclaw_register_func();
	weapon_stavros_register_func();
	weapon_wyndrax_register_func();
	// register e4 weapon funcs
	weapon_glock_register_func();
	weapon_kineticore_register_func();
	weapon_metamaser_register_func();
	weapon_novabeam_register_func();
	weapon_ripgun_register_func();
	weapon_slugger_register_func();

	// register global weapon funcs
	weapon_daikatana_register_func();
	// register misc "weapon" funcs

	// SCG[12/7/99]: Consolidate weapon selection.
	gstate->RegisterFunc( "weapon_select_1", weapon_select_1 );
	gstate->RegisterFunc( "weapon_select_2", weapon_select_2 );
	gstate->RegisterFunc( "weapon_select_3", weapon_select_3 );
	gstate->RegisterFunc( "weapon_select_4", weapon_select_4 );
	gstate->RegisterFunc( "weapon_select_5", weapon_select_5 );
	gstate->RegisterFunc( "weapon_select_6", weapon_select_6 );
	gstate->RegisterFunc( "weapon_select_7", weapon_select_7 );

	gstate->RegisterFunc( "weapon_give_1", weapon_give_1 );
	gstate->RegisterFunc( "weapon_give_2", weapon_give_2 );
	gstate->RegisterFunc( "weapon_give_3", weapon_give_3 );
	gstate->RegisterFunc( "weapon_give_4", weapon_give_4 );
	gstate->RegisterFunc( "weapon_give_5", weapon_give_5 );
	gstate->RegisterFunc( "weapon_give_6", weapon_give_6 );
	gstate->RegisterFunc( "weapon_give_7", weapon_give_7 );
	gstate->RegisterFunc( "Weapon_Respawn", Weapon_Respawn );
	
}

CHitCounter::~CHitCounter()
{
/*
	if (!self)
		return;

	int count = 0;
	for (int i = 0; i < MAX_HITS; i++)
	{
		if (hits[i]) count++;
	}

	WEAPON_HIT(self,count);
*/
}

void CHitCounter::AddHit(userEntity_t *_self, userEntity_t *_hit)
{
	if (!self)
		self = _self;

	if (self != _self)
		return;

	userEntity_t *hit;
	for (int i = 0; i < MAX_HITS; i++)
	{
		hit = hits[i];
		if (hit == _hit) // don't add duplicates
			break;

		if (!hit)
		{
			hits[i] = _hit;
			WEAPON_HIT(self,1);
			break;
		}
	}
}

short Weapon_is_fleshy(userEntity_t *self)
{
	playerHook_t *hook = (playerHook_t *)self->userHook;
	if (!hook)
		return IS_FLESHY;

	switch(hook->type)
	{
	default:
	case TYPE_CLIENT			:
	case TYPE_MIKIKO			:
	case TYPE_SUPERFLY			:
	case TYPE_MIKIKOFLY			:
	case TYPE_BOT				:
	case TYPE_SURGEON			:
	case TYPE_PRISONER			:
	case TYPE_MISHIMAGUARD		:
	case TYPE_PSYCLAW			:
	case TYPE_SKINNYWORKER		:
	case TYPE_FATWORKER			:
	case TYPE_FERRYMAN			:
	case TYPE_SPIDER			:
	case TYPE_THIEF				:
	case TYPE_SATYR				:
	case TYPE_HARPY				:
	case TYPE_SIREN				:
	case TYPE_GRIFFON			:
	case TYPE_CERBERUS			:
	case TYPE_MEDUSA			:
	case TYPE_CYCLOPS			:
	case TYPE_MINOTAUR			:
	case TYPE_SMALLSPIDER		:
    case TYPE_KMINOS			:
	case TYPE_DRAGONEGG			:
	case TYPE_PRIEST			:
	case TYPE_ROTWORM			:
	case TYPE_PLAGUERAT			:
	case TYPE_BUBOID			:
	case TYPE_DOOMBAT			:
	case TYPE_LYCANTHIR			:
	case TYPE_CELESTRIL			:
	case TYPE_BABYDRAGON		:
	case TYPE_DRAGON			:
	case TYPE_SEAGULL			:
	case TYPE_GOLDFISH			:
	case TYPE_SHARK				:
	case TYPE_SQUID				:
	case TYPE_BLACKPRIS			:
	case TYPE_WHITEPRIS			:
	case TYPE_LABMONKEY			:
	case TYPE_PIPEGANG			:
	case TYPE_SDIVER			:
	case TYPE_CRYOTECH 			:
	case TYPE_NAVYSEAL			:
	case TYPE_SEALGIRL			:
	case TYPE_SEALCOMMANDO		:
	case TYPE_CINE_ONLY			:								
	case TYPE_FIREFLY			:
	case TYPE_WISP				:
	case TYPE_PIPERAT			:
		return IS_FLESHY;
	// E1	
	case TYPE_ROBOCROC			:
	case TYPE_SL_SKEET			:
	case TYPE_TH_SKEET			:
	case TYPE_FROGINATOR		:
	case TYPE_VERMIN			:
	case TYPE_SLUDGEMINION		:
	case TYPE_INMATER			:
	case TYPE_BATTLEBOAR		:
	case TYPE_RAGEMASTER		:
	case TYPE_TRACKATTACK		:
	case TYPE_LASERGAT			:
	case TYPE_CAMBOT			:
	case TYPE_DEATHSPHERE		:
	case TYPE_PROTOPOD			:
	// E2
	case TYPE_SKELETON			:
	case TYPE_CENTURION			:
	case TYPE_COLUMN			:
	// E3
	case TYPE_STAVROS			:
	case TYPE_FLETCHER			:
	case TYPE_DWARF				:
	case TYPE_WYNDRAX			:
	case TYPE_NHARRE			:
	case TYPE_GARROTH			:
	case TYPE_WIZARD			:
	// E4
	case TYPE_ROCKETGANG		:
	case TYPE_UZIGANG			:
	case TYPE_CHAINGANG			:
	case TYPE_SEALCAPTAIN		:
	case TYPE_ROCKETMP			:
		return IS_ARMOR;				
																
	case TYPE_KNIGHT1			:
	case TYPE_KNIGHT2			:
	case TYPE_KAGE				:
		return IS_ARMOR|IS_SWORD;

	case TYPE_MIKIKOMON			:
	case TYPE_FEMGANG			:
		return IS_FLESHY|IS_SWORD;
	};
}