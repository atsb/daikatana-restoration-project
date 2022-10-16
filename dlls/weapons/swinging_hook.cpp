/*-------------------------------------------------------------------
 Grappling hook for Daikatana

  Inspired by an Pericle Minole's swinging hook, who is probably
  the first person to do a hook responding realisticly to game
  physics. 

  Since this stuff may be affected by user-modified libs, be sure
  to be extra carefull with parameters, esp entity data. Be sure to
  advise the user if bad data is passed if the cvar DEVELOEPR is 1
  and handle things as gracefully as possible.
-------------------------------------------------------------------*/
/*
#include <windows.h>
#include "dk_dll.h"
#include "dk_shared.h"
#include "p_user.h"
#include "common.h"
#include "HOOKS.H"
*/
#include "weapons.h"

#define HOOK_ACTIVE		         0x00000001  
#define HOOK_ATTACHED	         0x00000002  
#define HOOK_REEL_IN	            0x00000004  
#define HOOK_REEL_OUT	         0x00000008
#define HOOK_ATTACKING           0x00000010
#define HOOK_ATTACK_NOOSE        0x00000020
#define HOOK_ATTACK_HELLRAISER   0x00000040
#define HOOK_MAX_GRAPPLES        128         // for grapple list

// grapple_s
// tracking hooks in the game
typedef struct grapplelist_s
{
   userEntity_t   *grapple;
   userEntity_t   *owner;
   userEntity_t   *victim;
} grapplelist_t;


// char	            *dll_Description = "SwingingHook.dll: A grappling hook with realistic physics.\n";
// serverState_t     *gstate;
cvar_t            *hook_speed;
cvar_t            *hook_min_len;
cvar_t            *hook_max_len;
cvar_t            *hook_rate;
cvar_t            *hook_prediction;
// common_export_t   *com;
grapplelist_t         grappleList[HOOK_MAX_GRAPPLES];

// userEpair_t eclassStr[] = {NULL};
// CVector up(0,0,1);

void hook_Noose(userEntity_t *,userEntity_t *ent, CVector);
void hook_HellRaiser(userEntity_t *attackinghook, userEntity_t *victim);

void G_ProjectSource (CVector point, CVector distance, CVector forward, CVector right, CVector & result)
{
	result.x = point.x + forward.x * distance.x + right.x * distance.y;
	result.y = point.y + forward.y * distance.x + right.y * distance.y;
	result.z = point.z + forward.z * distance.x + right.z * distance.y + distance.z;
}

/*-------------------------------------------------------------------
	Project Source in reverse

  The hook should look like it is originating from the free
  hand.
-------------------------------------------------------------------*/
void hook_ProjectSource_Reverse (gclient_t *client, CVector & point, CVector & distance, CVector & forward, CVector & right, CVector  &result)
{
	CVector	v3NewHand;

	// VectorCopy (distance, v3NewHand);
   v3NewHand = distance;
		v3NewHand.y *= -1;
	G_ProjectSource (point, v3NewHand, forward, right, result);
}

/*-------------------------------------------------------------------
	Disable hook, reset prediction mode if prediction mode is 
	not set.
-------------------------------------------------------------------*/
void hook_Drop (userEntity_t *ent_grapple)
{
   int i;
	// Somehow things got out of hand, bail out, notify if enabled!
	if(!ent_grapple)
	{
		gstate->Con_Dprintf ("hook_Drop():%s,%i: ent_grapple == NULL\n", __FILE__, __LINE__);
		return; 
	}

   // Remove the hook from the grapple list
   for(i=0;i<HOOK_MAX_GRAPPLES;i++)
   {
      if(grappleList[i].grapple != ent_grapple)
         continue;
      memset(&grappleList[i], 0, sizeof(grapplelist_t));
   }

	// If the owner is in attack mode, and is not dead
	if(ent_grapple->owner)
	{
		ent_grapple->owner->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
	
		ent_grapple->owner->client->hookstate = 0;
	}

	// FIXME: add a disconnect sound for the hook
	
	// removes hook
   if(!ent_grapple->goalentity)
   {
      gstate->Con_Dprintf("hook_Drop():%s:%i: ent_grapple->goalentity == NULL\n", __FILE__, __LINE__);
   } else {
      com->untrackEntity(ent_grapple->goalentity,NULL,MULTICAST_ALL);
      ent_grapple->remove(ent_grapple->goalentity);
   }
   ent_grapple->remove(ent_grapple);
	gstate->RemoveEntity(ent_grapple);
}


/*-------------------------------------------------------------------
	Manage hook behavior
-------------------------------------------------------------------*/
void hook_Behavior(userEntity_t *grapple)
{
	CVector	offset, start;
	CVector	forward, right;
	CVector	chainvec;		
   CVector  novector(0,0,0);
	float chainlen;			
	CVector velpart;			// player's velocity component moving to or away from hook
	float force;			// restrainment force
	qboolean chain_moving;
		
	// First and foremost: if, for some reason, this entity has been removed, 
	// bail out gracefully. If DEVELOPER=1, then whine about the bad parm.
	if(!grapple)
	{
		gstate->Con_Dprintf ("hook_Behavior():%s,%i: grapple == NULL\n", __FILE__, __LINE__);
		return; 
	}

	chain_moving = false;
	chainlen = 0;
	force = 0;

	// decide when to disconnect the hook
	if(!grapple->owner)
	{
		hook_Drop(grapple);
		return;
	}
	if ( (!(grapple->owner->client->hookstate & HOOK_ACTIVE)) ||// if hook has been retracted
	     (grapple->enemy->solid == SOLID_NOT) ||			// if target is no longer solid (i.e. hook broke glass; exploded barrels, gibs) 
	     (grapple->owner->deadflag) ||						// if player died
	     (grapple->owner->s.event == EV_PLAYER_TELEPORT) || // if player goes through teleport
		 ( (gstate->time - 5 > grapple->health) && (grapple->owner->client->hookstate & HOOK_ATTACKING) )  // if it's a noose/crucifier, it's got 5 seconds
		)	
	{
		hook_Drop(grapple);
		return;
	}

//	VectorCopy (grapple->enemy->velocity,grapple->velocity);
	grapple->velocity = grapple->enemy->velocity;

	if(grapple->owner->client->hookstate & HOOK_ATTACKING)
	{
		// offensive hook
		if(gstate->time - 1 > grapple->volume)
		{
         if(grapple->hchild)
         {
			   com->Damage(grapple->owner, grapple, grapple->hchild, grapple->enemy->s.origin, chainvec, 5, DAMAGE_DROWN);
			   grapple->volume = gstate->time;
         }
		}
		
	}

	// grow the length of the chain
	if ((grapple->owner->client->hookstate & HOOK_REEL_OUT) && (grapple->client->hookLength < hook_max_len->value))
	{
		grapple->client->hookLength += hook_rate->value;
		if (grapple->client->hookLength > hook_max_len->value) grapple->client->hookLength = hook_max_len->value;
		chain_moving = true;
	}

	// shrink the length of the chain
    if ((grapple->owner->client->hookstate & HOOK_REEL_IN) && (grapple->client->hookLength > hook_min_len->value))
	{
		grapple->client->hookLength -= hook_rate->value;
		if (grapple->client->hookLength < hook_min_len->value) grapple->client->hookLength = hook_min_len->value;
		chain_moving = true;
	}

	// derive start point of chain
//	AngleVectors (grapple->owner->client->v_angle, forward, right, NULL);
	grapple->owner->client->v_angle.AngleToVectors(forward, right, novector);
	offset.Set( 8, 8, grapple->owner->viewheight-8);
	hook_ProjectSource_Reverse (grapple->owner->client, grapple->owner->s.origin, offset, forward, right, start);

	// get info about chain
   chainvec.Subtract(grapple->s.origin, start);
	chainlen = chainvec.Length();

	// if player's location is beyond the chain's reach
	if (chainlen > grapple->client->hookLength)	
	{	 
		// determine player's velocity component of chain vector
		// VectorScale (chainvec, _DotProduct (grapple->owner->velocity, chainvec) / _DotProduct (chainvec, chainvec), velpart);
		// velpart.Scale(chainvec, grapple->owner->velocity.DotProduct(chainvec) / chainvec.DotProduct(chainvec));
      velpart = chainvec *  (grapple->owner->velocity.DotProduct(chainvec) / chainvec.DotProduct(chainvec));
		// restrainment default force 
		force = (chainlen - grapple->client->hookLength) * 5;

		// if player's velocity heading is away from the hook
		if (grapple->owner->velocity.DotProduct(chainvec) < 0)
		{
			// if chain has streched for 25 units
			if (chainlen > grapple->client->hookLength + 25)
				// remove player's velocity component moving away from hook
				// _VectorSubtract(grapple->owner->velocity, velpart, grapple->owner->velocity);
            grapple->owner->velocity.Subtract(grapple->owner->velocity, velpart);

		}
		else  // if player's velocity heading is towards the hook
		{
			if (velpart.Length() < force)
				force -= velpart.Length();
			else		
				force = 0;
		}
	}
	else
		force = 0;

	// disable prediction while suspended in air by hook
	// if server console variable hook_prediction is set 
	if (!(grapple->owner->client->ps.pmove.pm_flags & PMF_ON_GROUND))
	{
		if (hook_prediction->value)
			grapple->owner->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	}	
	else
		grapple->owner->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;

	chainvec.Normalize();
	VectorMA (grapple->owner->velocity, chainvec, force, grapple->owner->velocity);
	
	

	grapple->nextthink = gstate->time + 0.1;
}

/*-------------------------------------------------------------------
	hook_Attack(userEntity_t *attacking_hook, userEntity_t *other)

  If the hook latches an enemy, there are three attack options.
  First, find the furthest: a wall or a ceiling. If neither are
  within 2000 units, then hook the target like a regular grapple.
  
  If the ceiling is further than the wall, hang the poor guy from
  the ceiling and let the attacker(s) beat on him like a pinata, 
  Play some disturbing gurgling and gagging sound FX. Play a 
  "hanging" animation.

  If the wall is further, then the hook splits to two spikes, and
  crucifies the target, who will bleed on the wall, permitting
  the attacker(s) to have their way with him. Play some real 
  screaming bloody murder pain sound FX, and spawn blood FX on the
  wall behind the target's hands.

  Hang time (pardon the pun) should be set to something like
  5 seconds, unless the player dies, in which case the body should
  remain hung or crucified for full effect.
-------------------------------------------------------------------*/
void hook_Attack(userEntity_t *attacking_hook, userEntity_t *victim)
{
	trace_t        trace;
	CVector         traceup;
   bool           noosable, hellraiserable; 
   int            i;

	// parameter check: already performed in hook_Touch(), this data
	// is verified.

	traceup.x     = victim->s.origin.x;
	traceup.y     = victim->s.origin.y;
	traceup.z     = victim->s.origin.z + 2000;
   noosable       = false;
   hellraiserable = false;

   // Determine if the victim can hang
	// trace=gstate->trace_q2(victim->s.origin, NULL, NULL, traceup, victim, MASK_SOLID);
	trace=gstate->TraceLine_q2(victim->s.origin, traceup, NULL, MASK_SOLID);
	if (trace.surface && !(trace.surface->flags & SURF_SKY))
      noosable = true;
   
   // Can the hellraiser, secret attack be used?
   // Costly, but this is not run every frame.
   for(i=0;i<HOOK_MAX_GRAPPLES;i++)
   {
      if(grappleList[i].victim != victim)
         continue;
      if(grappleList[i].owner != attacking_hook->owner)
      {
         // goodie! The victim is already latched by another
         // player, time for the hellraiser team attack!
         hellraiserable = true;
         break;
      }
   }

   // exception to team attack, if the victim is out
   // in the open, no noose is available --HELLRAISERHOOK!
   if(noosable && !hellraiserable)
	{
		// Let him Hang Until he is dead! (actually for 5 seconds
		// until the poor slob is either gibbed, chokes to death
		// or escapes.

		// start a new grapple, from the targets neck to the
		// ceiling
		hook_Noose(attacking_hook, victim, up);
   } else {
      // Spawn several random chains from within the victim,
      // launch them and then thrash the poor soul violently
      hook_HellRaiser(attacking_hook, victim);
   }

}

/*-------------------------------------------------------------------
	Touch

  Found a surface, if it's a player or a monster, use the alternate
  hook attack! 

  This is called from outside the lib, be wary of data passed!
-------------------------------------------------------------------*/
void hook_Touch (userEntity_t *grapple_projectile, userEntity_t *other, cplane_t *plane, csurface_t *surf)
{
	CVector	offset, start;
	CVector	forward, right;
	CVector	chainvec;		// chain's vector
   CVector  novector(0,0,0);

	// Parameter check:
	// this is also called from outside this lib, check relevant parameters so that
	// the user can continue, even if another lib has altered things unexpectedly. 
	// Also optionally notify the user of the problem if DEVELOPER is 1
	if(!grapple_projectile)
	{
		gstate->Con_Dprintf ("hook_Touch():%s,%i: grapple_projectile == NULL\n", __FILE__, __LINE__);
		return; 
	}
	if(!grapple_projectile->owner)
	{
		gstate->Con_Dprintf ("hook_Touch():%s,%i: grapple_projectile->owner == NULL\n", __FILE__, __LINE__);
		return; 
	}
	if(!grapple_projectile->owner->client)
	{
		gstate->Con_Dprintf ("hook_Touch():%s,%i: grapple_projectile->owner->client == NULL\n", __FILE__, __LINE__);
		return; 
	}
	if(!other)
	{
		gstate->Con_Dprintf ("hook_Touch():%s,%i: other == NULL\n", __FILE__, __LINE__);
		return; 
	}
	if(!plane)
	{
		gstate->Con_Dprintf ("hook_Touch():%s,%i: plane == NULL\n", __FILE__, __LINE__);
		return; 
	}
	if(!surf)
	{
		gstate->Con_Dprintf ("hook_Touch():%s,%i: surf == NULL\n", __FILE__, __LINE__);
		return; 
	}

	// starting point for launching the hook
	// AngleVectors (grapple_projectile->owner->client->v_angle, forward, right, NULL);
	grapple_projectile->owner->client->v_angle.AngleToVectors(forward, right, novector);
	offset.Set( 8, 8, grapple_projectile->owner->viewheight-8);
	hook_ProjectSource_Reverse (grapple_projectile->owner->client, grapple_projectile->owner->s.origin, offset, forward, right, start);

	// _VectorSubtract(grapple_projectile->s.origin,start,chainvec);
	chainvec.Subtract(grapple_projectile->s.origin, start);
   grapple_projectile->client->hookLength = chainvec.Length();	

	// swinging from the sky? no way
	if (surf && (surf->flags & SURF_SKY))
	{
		hook_Drop(grapple_projectile);
		return;
	}

	if (other->solid == SOLID_BBOX)
	{
		if ((other->svflags & SVF_MONSTER) || (other->client) || other->flags & FL_BOT)
			gstate->StartEntitySound (grapple_projectile, CHAN_AUTO, gstate->SoundIndex("e3/we_bolterhitmetal.wav"), 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
		// OK, nailed the sucker, need to find ceiling or wall for a gruesome kill!
		hook_Attack(grapple_projectile, other);
		// hook_Drop(grapple_projectile);
		// return;
	}
	
	if (other->solid == SOLID_BSP)
	{
		// create a puff of smoke where the grapple impacts
		
		gstate->WriteByte (SVC_TEMP_ENTITY);
		gstate->WriteByte (TE_SMOKE);
		gstate->WritePosition (grapple_projectile->s.origin);
		gstate->WriteFloat(1.0);
		gstate->WriteFloat(2.0);
		gstate->WriteFloat(2.0);
		gstate->WriteFloat(5.0);
		gstate->MultiCast (grapple_projectile->s.origin, MULTICAST_PVS);
		
		gstate->StartEntitySound(grapple_projectile, CHAN_AUTO, gstate->SoundIndex("e3/we_bolterhitmetal.wav"), 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);
		grapple_projectile->avelocity.Zero();
	}
	
	// VectorCopy (other->velocity, grapple_projectile->velocity);
   grapple_projectile->velocity = other->velocity;
	if(grapple_projectile->owner)
	{
		grapple_projectile->owner->client->hookstate |= HOOK_ATTACHED;
	}
	grapple_projectile->enemy = other;
	grapple_projectile->touch = NULL;
	grapple_projectile->think = hook_Behavior;
	grapple_projectile->nextthink = gstate->time + 0.1;
}



/*-------------------------------------------------------------------
	Hook en route to latch onto something,

  This is the default think function while the grapple is in the
  air, before it latches or drops. Check parms here.
-------------------------------------------------------------------*/
void hook_Fly (userEntity_t *ent)
{
    CVector chainvec;		// chain's vector
	float chainlen;			// length of extended chain

	// since this is a think function, running 10 times / second,
	// check parameters on each pass. Another user lib could have
	// done bad things to variables in the interim. Handle problems
	// and notify the user if cvar DEVELOPER is 1
	if(!ent)
	{
		gstate->Con_Dprintf ("hook_Fly():%s,%i: ent == NULL\n", __FILE__, __LINE__);
		return; 
	}
	// ent->owner info is passed down the execution path
	if(!ent->owner || !ent->client)
	{
		gstate->Con_Dprintf ("hook_Fly():%s,%i: ent->owner || ent->client == NULL\n", __FILE__, __LINE__);
		return; 
	}
	// owner sure as hell better be a client!
	if(!ent->owner->client)
	{
		gstate->Con_Dprintf ("hook_Fly():%s,%i: ent->owner->client == NULL\n", __FILE__, __LINE__);
		return; 
	}


	// get info about chain
	// _VectorSubtract (ent->s.origin, ent->owner->s.origin, chainvec);
   chainvec = ent->s.origin - ent->owner->s.origin;
	chainlen =chainvec.Length();
	
	// if HOOK_ACTIVE is not flagged, kill the hook entity and return
	if ( (!(ent->owner->client->hookstate & HOOK_ACTIVE)) || (chainlen > hook_max_len->value) )
	{
		hook_Drop(ent);
		return;
	}

	// ok, maintain the chain between the owner and the grapple itself
	

	ent->nextthink = gstate->time + 0.1;
}


/*-------------------------------------------------------------------
	Hook Fired
-------------------------------------------------------------------*/
void hook_Fire (userEntity_t *ent)
{
	userEntity_t   *newhook;
   userEntity_t   *hookchain;
	trackInfo_t    tinfo;
   int            i;
	CVector	      offset, start;
	CVector	      forward, right, novector(0,0,0);

   // test
   UDP_Log(2, "Swinging Hook fired!");

	// parameter check
	// alread clean, this is only called from within this lib, verified
	// in grapple_Cmd

	// keep data clean
	memset(&tinfo, 0, sizeof(trackInfo_t));
	newhook = NULL;

	// AngleVectors (ent->client->v_angle, forward, right, NULL);
   ent->client->v_angle.AngleToVectors(forward, right, novector);
	offset.Set(8, 8, ent->viewheight-8);
	hook_ProjectSource_Reverse (ent->client, ent->s.origin, offset, forward, right, start);

	newhook = gstate->SpawnEntity();
	// VectorCopy (start, newhook->s.origin);
   newhook->s.origin = start;
	// VectorCopy (forward, newhook->movedir);
   newhook->movedir = forward;
	// VectorToAngles (forward, newhook->s.angles);
   forward.VectorToAngles(newhook->s.angles);

   // VectorScale (forward, hook_speed->value, newhook->velocity);
   // newhook->velocity.Scale(forward, hook_speed->value);
   newhook->velocity = forward * hook_speed->value;

	// VectorSet(newhook->avelocity,0,0,-800);
   newhook->avelocity.Set(0,0,-800);
	newhook->movetype = MOVETYPE_FLYMISSILE;

	newhook->clipmask = MASK_SHOT;
	newhook->solid = SOLID_BBOX;
	// VectorClear (newhook->s.mins);
	// VectorClear (newhook->s.maxs);
   newhook->s.mins.Zero();
   newhook->s.maxs.Zero();

	newhook->s.modelindex = gstate->ModelIndex ("models/e1/we_swrocket.dkm");
	newhook->owner = ent;

	newhook->touch = hook_Touch;
	newhook->think = hook_Fly;
	newhook->nextthink = gstate->time + 0.1;
	
	gstate->LinkEntity (newhook);
	gstate->StartEntitySound(newhook, CHAN_AUTO, gstate->SoundIndex("e2/we_discshoota.wav"), 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);

   // Add it to the grapple list
   for(i=0;i<HOOK_MAX_GRAPPLES;i++)
   {
      if(grappleList[i].grapple != NULL)
         continue;
      grappleList[i].grapple = newhook;
      grappleList[i].owner = ent;
   }

	// set up chain track entity
   // The client will receive this entity and track 
   // the chain between the client and the hook itself
	hookchain = gstate->SpawnEntity();
   hookchain->className = "hook_chain";
	hookchain->movetype = MOVETYPE_NONE;
	hookchain->solid = SOLID_NOT;
	hookchain->s.renderfx = RF_TRACKENT | RF_BEAM; 
   hookchain->s.effects = EF_GIB; // not really, just force the ent to the client
	hookchain->s.frameInfo.frameFlags = FRAME_FORCEINDEX;
	hookchain->s.frame = 1; 
	// hookchain->s.skinnum = 0xa1a2a3a4;
	hookchain->s.alpha = 1;
	hookchain->owner = ent;
   
   // hookchain->s.modelindex = 1;
	hookchain->s.modelindex = 0;//  gstate->ModelIndex ("models/e1/we_swrocket.dkm");
   // VectorCopy(newhook->s.origin, hookchain->s.origin);
   hookchain->s.origin = newhook->s.origin;
	gstate->LinkEntity (hookchain);
   newhook->goalentity = hookchain;
	
	tinfo.renderfx = RF_GRAPPLE_CHAIN | RF_BEAM;
	// tinfo.renderfx = RF_BEAM|RF_TRANSLUCENT;
	tinfo.ent = hookchain;
	tinfo.srcent = ent;
	tinfo.dstent = newhook;
	tinfo.modelindex = gstate->ModelIndex ("models/e1/we_swrocket.dkm");
   tinfo.fru.x = 0;
   tinfo.fru.y = -8;
   tinfo.fru.z = -8;
  	tinfo.flags = TEF_SRCINDEX|TEF_DSTINDEX | TEF_FRU;
   
	com->trackEntity(&tinfo, MULTICAST_ALL);

}


/*-------------------------------------------------------------------
	Hook command
-------------------------------------------------------------------*/
void grapple_Cmd(edict_s *ent)
{
	char *s;
	int *hookstate;

	// First and foremost: if, for some reason, this entity has been removed, 
	// bail out gracefully. If DEVELOPER=1, then whine about the bad parm.
	if(!ent)
	{
		gstate->Con_Dprintf ("grapple_Cmd:%s,%i: ent == NULL\n", __FILE__, __LINE__);
		return; 
	}
	if(!ent->client)
	{
		gstate->Con_Dprintf ("grapple_Cmd:%s,%i: ent->client == NULL\n", __FILE__, __LINE__);
		return; 
	}

    // take care of hook cvars
    if(! hook_speed)
    {
        hook_speed = gstate->cvar ("hook_speed", "1000", CVAR_ARCHIVE);
	    hook_min_len = gstate->cvar ("hook_min_len", "40", CVAR_ARCHIVE);
	    hook_max_len = gstate->cvar ("hook_max_len", "2000", CVAR_ARCHIVE);
	    hook_rate = gstate->cvar ("hook_rate", "40", CVAR_ARCHIVE);
	    hook_prediction = gstate->cvar ("hook_prediction", "0", CVAR_ARCHIVE);
    }

	// FIXME: need to make this a cvar
	// no grapple in dm
	if (deathmatch->value) 
		return;

	// parameters for grappling hook command
	s = gstate->GetArgv(1);

	hookstate = &ent->client->hookstate;

	if ((ent->solid != SOLID_NOT) && (ent->deadflag == DEAD_NO) &&
		(!(*hookstate & HOOK_ACTIVE)) && (Q_stricmp(s, "fire") == 0))
	{
		// flags hook as being active 
		*hookstate |= HOOK_ACTIVE; // | HOOK_REEL_IN;   

		hook_Fire (ent);
		return;
	}

	if  (*hookstate & HOOK_ACTIVE)
	{
		// release hook	
		if (Q_stricmp(s, "fire") == 0)
		{
			*hookstate |= ~HOOK_ACTIVE;
			return;
		}

		// deactivate chain growth or shrink
		if (Q_stricmp(s, "stop") == 0)
		{
			*hookstate -= *hookstate & (HOOK_REEL_OUT | HOOK_REEL_IN);
			return;
		}

		// activate chain growth
		if (Q_stricmp(s, "out") == 0)
		{
			*hookstate |= HOOK_REEL_OUT;
			*hookstate -= *hookstate & HOOK_REEL_IN;
			return;
		}

		// activate chain shrinking
		if (Q_stricmp(s, "in") == 0)
		{
			*hookstate |= HOOK_REEL_IN;		
			*hookstate -= *hookstate & HOOK_REEL_OUT;
		}
	}
}

/*-------------------------------------------------------------------
	DLL Load
-------------------------------------------------------------------*/
/*
void	dll_ServerLoad (serverState_t *state)
{
	gstate = state;
	hook_speed = gstate->cvar ("hook_speed", "1000", CVAR_ARCHIVE);
	hook_min_len = gstate->cvar ("hook_min_len", "40", CVAR_ARCHIVE);
	hook_max_len = gstate->cvar ("hook_max_len", "2000", CVAR_ARCHIVE);
	hook_rate = gstate->cvar ("hook_rate", "40", CVAR_ARCHIVE);
	hook_prediction = gstate->cvar ("hook_prediction", "0", CVAR_ARCHIVE);
   g_cvarLogServer = gstate->cvar("logstate", "", CVAR_ARCHIVE);
}

void dll_LevelLoad(void)
{

	gstate->Con_Printf ("* Loaded Grappling Hook DLL\n");
	gstate->AddCommand("grapple", grapple_Cmd);
}

int dll_Version(int size) {
    if (size == IONSTORM_DLL_INTERFACE_VERSION) {
        return  TRUE;
    }
    else {
        return  FALSE;
    }
}

void	dll_LevelExit (void)
{
}

void 	dll_ServerKill (void)
{
}

void	dll_ServerInit (void)
{
   com = (common_export_t *)gstate->common_exports;
   
}

DllExport int	dll_Entry (HINSTANCE hParent, DWORD dwReasonForCall, PVOID pvData)
{
	int			size;
	// debug info
	switch (dwReasonForCall)
	{
		case	QDLL_VERSION:
			size = *(int *) pvData;

			return	dll_Version (size);
			break;
		
		case 	QDLL_QUERY:
			* (userEpair_t **) pvData = eclassStr;

			return	TRUE;
			break;

		case	QDLL_DESCRIPTION:
			* (char **) pvData = dll_Description;

			return	TRUE;
			break;

		case	QDLL_SERVER_INIT:
			dll_ServerInit ();

			return	TRUE;
			break;

		case	QDLL_SERVER_KILL:
			dll_ServerKill ();

			return	TRUE;
			break;

		case	QDLL_SERVER_LOAD:
			dll_ServerLoad ((serverState_t *) pvData);

			return	TRUE;
			break;

		case	QDLL_LEVEL_LOAD:
			dll_LevelLoad ();

			return	TRUE;
			break;

		case	QDLL_LEVEL_EXIT:
			dll_LevelExit ();

			return	TRUE;
			break;

	}

	return	FALSE;
}

*/
/*-------------------------------------------------------------------
	hook_Noose(userEntity_t *attackinghook, userEntity_t *victim, CVector forward)

  Called by hook_Attack. It is used to spawn a new hook from the
  victim, thereby hanging him or otherwise latching him. The hook is
  launched in a particular direction (specified by forward). The new
  hook expires after 5 seconds.
-------------------------------------------------------------------*/
void hook_Noose(userEntity_t *attackinghook, userEntity_t *victim, CVector forward)
{
	userEntity_t *newhook;
   userEntity_t *hookchain;
	trackInfo_t tinfo;
	CVector	offset, start;
	CVector	right;
	int *hookstate;
   int i;
	playerHook_t	*playerhook = (playerHook_t *) victim->userHook; // nothing to do with a grappling hook!

   memset(&tinfo, 0, sizeof(trackInfo_t));
   
	// VectorSet(offset, 8, 8, victim->viewheight+16);
   offset.Set(8, 8, victim->viewheight+16);

	hook_ProjectSource_Reverse (victim->client, victim->s.origin, offset, forward, right, start);

	newhook = gstate->SpawnEntity();
	// VectorCopy (start, newhook->s.origin);
   newhook->s.origin = start;
	// VectorCopy (forward, newhook->movedir);
   newhook->movedir = forward;
	// VectorToAngles (forward, newhook->s.angles);
   forward.VectorToAngles(newhook->s.angles);
	// VectorScale (forward, hook_speed->value, newhook->velocity);
   // newhook->velocity.Scale(forward, hook_speed->value);
   newhook->velocity = forward * hook_speed->value;
	// VectorSet(newhook->avelocity,0,0,-800);
   newhook->avelocity.Set(0,0,-800);
	newhook->movetype = MOVETYPE_FLYMISSILE;
	newhook->clipmask = MASK_SHOT;
	newhook->solid = SOLID_BBOX;
	// VectorClear (newhook->s.mins);
   newhook->s.mins.Zero();
	// VectorClear (newhook->s.maxs);
   newhook->s.maxs.Zero();

	newhook->s.modelindex = gstate->ModelIndex ("models/e1/we_swrocket.dkm");
	newhook->owner = victim;
	// newhook->frags = 1;		// not a hook, a noose!
	newhook->health = gstate->time;
	newhook->volume = gstate->time;
	newhook->touch = hook_Touch;
	newhook->think = hook_Fly;
	newhook->nextthink = gstate->time + 0.1;
	newhook->hchild = attackinghook->owner;
	gstate->LinkEntity (newhook);
	gstate->StartEntitySound(newhook, CHAN_AUTO, gstate->SoundIndex("e2/we_discshoota.wav"), 1.0f, ATTN_NORM_MIN, ATTN_NORM_MAX);


	hookstate = &victim->client->hookstate;
	*hookstate |= HOOK_ACTIVE | HOOK_REEL_IN | HOOK_ATTACKING;   
		
	// set up chain track entity
   // The client will receive this entity and track 
   // the chain between the client and the hook itself
	hookchain = gstate->SpawnEntity();
   hookchain->className = "hook_chain";
	hookchain->movetype = MOVETYPE_NONE;
	hookchain->solid = SOLID_NOT;
	hookchain->s.renderfx = RF_TRACKENT | RF_BEAM; 
   hookchain->s.effects = EF_GIB; // not really, just force the ent to the client
	hookchain->s.frameInfo.frameFlags = FRAME_FORCEINDEX;
	hookchain->s.frame = 1; 
	// hookchain->s.skinnum = 0xa1a2a3a4;
	hookchain->s.alpha = 1;
	hookchain->owner = victim;
   
   // hookchain->s.modelindex = 1;
	hookchain->s.modelindex = gstate->ModelIndex ("models/e1/we_swrocket.dkm");
   // VectorCopy(newhook->s.origin, hookchain->s.origin);
   hookchain->s.origin = newhook->s.origin;

	gstate->LinkEntity (hookchain);
   newhook->goalentity = hookchain;
   // Add it to the grapple list
   for(i=0;i<HOOK_MAX_GRAPPLES;i++)
   {
      if(grappleList[i].grapple != NULL)
         continue;
      grappleList[i].grapple = newhook;
      grappleList[i].owner = attackinghook->owner;
      grappleList[i].victim = victim;
   }

	tinfo.renderfx = RF_BEAM;
	// tinfo.renderfx = RF_BEAM|RF_TRANSLUCENT;
	tinfo.ent = hookchain;
	tinfo.srcent = victim;
	tinfo.dstent = newhook;
	tinfo.modelindex = gstate->ModelIndex ("models/e1/we_swrocket.dkm");
   tinfo.fru.x = 8;
   tinfo.fru.y = 8;
   tinfo.fru.z = -8;
  	tinfo.flags = TEF_SRCINDEX|TEF_DSTINDEX | TEF_FRU;
   
  
	com->trackEntity(&tinfo, MULTICAST_ALL);


}

///////////////////////////////////////////////////////////////////////
// hook_HellRaiser(attackinghook, victim)
//
// New and interesting ways to gibletize your friends in deathmatch.
// This nasty implants the grapple in the victim, which then launches 
// several hooks in various directions, doing some damage over a 5
// second period. If the victim's health reaches 0 (either from 
// damage inflicted or as the result of an attacker beating the poor
// bastard down) AND the victim hasn't gibbed already, the HellRaiser
// attack gibs the victim
///////////////////////////////////////////////////////////////////////
void hook_HellRaiser(userEntity_t *attackinghook, userEntity_t *victim)
{
   CVector forward;
   int iHookCount, i;
   
   iHookCount = 0;
   memset(&forward, 0, sizeof(CVector));

   iHookCount = int(rnd() * 10) + 5; // ew, this is gonna hurt
   
   for(i=0; i<iHookCount; i++)
   {
      // which way to launch from the player?
      forward.x = (rnd() - 0.5) * 2;
      forward.y = (rnd() - 0.5) * 2;
      forward.z = (rnd() - 0.5) * 2;
      hook_Noose(attackinghook, victim, forward);
   }
   
}

///////////////////////////////////////////////////////////////////////////////
//
//  register weapon functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void weapon_hook_register_func()
{
	gstate->RegisterFunc("hook_Touch",hook_Touch);
	gstate->RegisterFunc("hook_Behavior",hook_Behavior);
	gstate->RegisterFunc("hook_Fly",hook_Fly);
}