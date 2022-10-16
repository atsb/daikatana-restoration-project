#include "world.h"
#include "nodelist.h"
//#include "GrphPath.h"// SCG[1/23/00]: not used
#include "ai_func.h"
#include "common.h"

void Client_SpawnKey( userEntity_t *button, userInventory_t *item );
short item_bomb_message(userEntity_t *self);

int com_DoKeyFunctions( userEntity_t *self, userEntity_t *other, char *message )
{
	char	outmessage[128];

	if( self->keyname )
	{
		userEntity_t *pKey = com->FindEntity( self->keyname );
		if( other->inventory && ( other->flags & FL_CLIENT ) )
		{
			userInventory_t *pKeyItem = gstate->InventoryFindItem( other->inventory, self->keyname );

			// did the player have the key?
			if( pKeyItem == NULL )
			{
				if( ( self->hacks < gstate->time ) && ( ( message != NULL ) || ( stricmp( self->keyname,"item_bomb" ) == 0 ) ) )
				{
					if( pKey != NULL )
					{
#ifdef JPN
 #ifdef MAN // for MAN
						Com_sprintf( outmessage, sizeof( outmessage ), "%s %s.\n", message, pKey->netname );
 #else
  #ifdef TIW// for TIW
						Com_sprintf( outmessage, sizeof( outmessage ), "%s %s.\n", message, pKey->netname );
  #else // for JPN
						Com_sprintf( outmessage, sizeof( outmessage ), "%s %s.\n", pKey->netname, message );
  #endif // TIW
 #endif // MAN
#else
						Com_sprintf( outmessage, sizeof( outmessage ), "%s %s.\n", message, pKey->netname );
#endif // JPN
					}
					else if (!stricmp(self->keyname,"item_bomb"))		// cek[2-13-00]: hacky hacky
					{
						item_bomb_message(other);
						return FALSE;
					}
					else	
					{
						// cek[2-16-00]
						// argh.  this used to assume that the item existed in the map right now...doesn't work if the key
						// is on another map.  self->keyname isn't translated so this must change.
						userEntity_t *pNewKey = com->SpawnDynamicEntity( self, self->keyname, FALSE );
						if (pNewKey)
						{
#ifdef JPN
 #ifdef MAN // for MAN
							Com_sprintf( outmessage, sizeof( outmessage ), "%s %s.\n", message, pNewKey->netname );
 #else
  #ifdef TIW// for TIW
							Com_sprintf( outmessage, sizeof( outmessage ), "%s %s.\n", message, pNewKey->netname );
  #else // for JPN
							Com_sprintf( outmessage, sizeof( outmessage ), "%s %s.\n", pNewKey->netname, message );
  #endif // TIW
 #endif // MAN
#else
							Com_sprintf( outmessage, sizeof( outmessage ), "%s %s.\n", message, pNewKey->netname );
#endif // JPN
							gstate->RemoveEntity(pNewKey);
						}
						else
						{
#ifdef JPN
 #ifdef MAN // for MAN
							Com_sprintf( outmessage, sizeof( outmessage ), "%s %s.\n", message, self->keyname );
 #else
  #ifdef TIW// for TIW
							Com_sprintf( outmessage, sizeof( outmessage ), "%s %s.\n", message, self->keyname );
  #else // for JPN
							Com_sprintf( outmessage, sizeof( outmessage ), "%s %s.\n", self->keyname, message );
  #endif // TIW
 #endif // MAN
#else
							Com_sprintf( outmessage, sizeof( outmessage ), "%s %s.\n", message, self->keyname );	// well, I tried..
#endif // JPN
						}
					}
					gstate->centerprint( other, 2.0, outmessage );
				}

				return FALSE;
			}
			else 
			{
				if( pKeyItem->flags & ITF_USEONCE )
				{
					gstate->InventoryDeleteItem( other, other->inventory, pKeyItem );
					self->keyname = NULL;
				}

				// see if this key unlocks whatever requires it
				if( pKeyItem->flags & ITF_UNLOCK )
				{
					//	spawn a key sticking out of the door
					Client_SpawnKey( self, pKeyItem );

					// find the entity which this one acts upon
					if( self->target )
					{
						userEntity_t *pOther = com->FindTarget( self->target );
						// it could target more than one door or entity
						while( pOther )
						{
							pOther->targetname	= NULL;		// now a door opens without a button
							pOther->keyname		= NULL;		// now it doesn't require a key
							pOther = com->FindTarget( self->target );	// find the next one if multiple doors
						}	

					}

					// now remove the keyname requirement from the original entity
					self->keyname = NULL;
				}
			}
		}
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//	com_FindParent
//
///////////////////////////////////////////////////////////////////////////////
void com_FindParent (userEntity_t *self)
{
	userEntity_t	*parent, *cur, *last;

	if (self->parentname)
	{
		// search for the entity with a targetName == parentName
		parent = com->FindTarget (self->parentname);
		if (parent)
		{
			// make sure a child can not take on it's own list of entities
			if (parent->teammaster)
			{
	 			gstate->Con_Printf ("Parent [%s] is already a child !! No link created for [%s] at %s.\n",  
					self->parentname, 
					self->className,
					com->vtos(self->s.origin));
			}
			else
			{
			
				// step through the teamlist and attach 'self' to the end
				cur = parent;
				while (cur)
				{
					last = cur;
					cur = cur->teamchain;		// go to next entity
				}
				last->teamchain = self;			// link the end to the current entity
				self->teamchain = NULL;			// just to make sure
				self->teammaster = parent;		// point back to the parent
				self->flags |= FL_TEAMSLAVE;	// let it be known this is a child entity

				self->spawn_origin = self->s.origin;

				// now update the child if the parent (perhaps a train) has moved since it was spawned
				if (!VectorEqual(parent->spawn_origin, parent->s.origin))
				{
					CVector newOrigin;
					newOrigin = self->spawn_origin + ( parent->s.origin - parent->spawn_origin );
					gstate->SetOrigin2 (self, newOrigin);
					self->s.old_origin = self->s.origin;
				}

				// initialize all the offsets from the parent
				self->childOffset = self->s.origin - parent->s.origin;
				self->transformedOffset = self->childOffset;
				self->s.angle_delta = self->s.angles - parent->s.angles;
			}
		}
		else
		{
			gstate->Con_Printf ("Child entity [%s] link parent [%s] not found\n",(self->targetname) ? self->targetname:"N/A",self->parentname);
		}
	}

	// reset the think functions
	self->nextthink = -1;
	self->think = NULL;
}

//---------------------------------------------------------------------------
// com_SpawnEffect()
//
// make this a function in common
//
//---------------------------------------------------------------------------
void com_SpawnEffect(CVector &pos,float scale,float amount,float spread,float maxvel)
{
	gstate->WriteByte(SVC_TEMP_ENTITY);
	gstate->WriteByte(TE_SMOKE);
	gstate->WritePosition(pos);
	gstate->WriteFloat(scale);
	gstate->WriteFloat(amount);
	gstate->WriteFloat(spread);
	gstate->WriteFloat(maxvel);
	gstate->MultiCast(pos,MULTICAST_PVS);
}

/////////////////////////////////////////////////////////
// com_GenerateDustCloud()
//
// generates a random dust cloud around a given entity
//
/////////////////////////////////////////////////////////
void com_GenerateDustCloud( userEntity_t *self, int count )
{
	CVector centerPt, spawnPos, mins, maxs, temp;

	com->GetMinsMaxs (self, centerPt, mins, maxs);
	temp.Set(1.0, 1.0, 1.0);

	mins = mins - temp;
	maxs = maxs + temp;

//	gstate->Con_Printf ("min: %s   max: %s\n", com->vtos(mins.vec()), com->vtos(maxs.vec()));

	for (int i = 0; i < count; i++)
	{
		spawnPos.x = (rand() % (int)(maxs.x)) * self->movedir.x + centerPt.x;
		spawnPos.y = (rand() % (int)(maxs.y)) * self->movedir.y + centerPt.y;
		spawnPos.z = centerPt.z ;//self->absmax[2];  // spawn these on the ground
		
		com_SpawnEffect( spawnPos, rand() % 4, rand() % 50, rand() % 6 + 2, rand() % 40);
//		gstate->Con_Printf ("spawnPos: %s\n", com->vtos(spawnPos.vec()));
	}
	return;
}

/////////////////////////////////////////////////////////
//
// com_PlayEntityScript()
//
// fire off and cine or ai scripts attached to this entity
//
/////////////////////////////////////////////////////////
void CIN_RemoveAllEntities();
void com_PlayEntityScript(userEntity_t *pActivator, char *pCineScript, char *pAiScript)
{
	// are there scripts defined?
	if (pCineScript || pAiScript)
	{
		// run the cinescriptor script
		if (pCineScript)
		{
			if( ( sv_cinematics->value != 0 ) && ( coop->value == 0 ) && ( deathmatch->value == 0 ) )
			{
				char buf[256];
//				sprintf( buf, "cin_load_and_play %s\n", pCineScript );
				Com_sprintf( buf, sizeof(buf),"cin_load_and_play %s\n", pCineScript );
				gstate->CBuf_AddText( buf );
			}
			else
			{
				if( pCineScript == NULL )
				{
					gstate->Con_Printf("pCineScript == NULL\n");
				}
				else
				{
					strncpy( gstate->szCinematicName, pCineScript, 32 );
					CIN_RemoveAllEntities();
				}
			}
		}
		else if (pAiScript)
		{
			// script activation code
			AI_AddScriptActionGoal( pActivator, pAiScript );
		}
	}
}


/////////////////////////////////////////////////////////
// com_SpawnEarthQuake()
//
// generates a random dust cloud around a given entity
//
/////////////////////////////////////////////////////////

void com_SpawnEarthQuake( userEntity_t *self, float fRadius, float fScale, float fDamage )
{
	userEntity_t	*ent;
	CVector			vDist, vKick;
	float			fLength, fIntensity, fIntensityScale;

	fIntensityScale = fScale * 0.05f;
    for( ent = gstate->FirstEntity(); ent; ent = gstate->NextEntity( ent ) )
	{
        if ( 
			( !ent->inuse ) 
			|| ( ent == self ) 
			|| ( ent->solid != SOLID_BBOX ) 
			|| ( ent->s.effects & EF_GIB ) 
			|| ( ent->movetype == MOVETYPE_FLY )

			)
		{
            continue;
		}

		vDist = ent->s.origin - self->s.origin;
		fLength = vDist.Length();

		if( fLength > fRadius )
		{
			continue;
		}

		fIntensity = ( ( fRadius - fLength ) * 0.01 );

        if( ( ent->flags & FL_CLIENT ) && ( ent->client ) )
        {
			if( fDamage != 0.0 )
			{
	            com->Damage( ent, self, self, self->s.origin, CVector( 0, 0, 0 ), ( fDamage * 0.1 ) * fIntensityScale, DAMAGE_NO_BLOOD );
			}
			vKick.Set( ( crand() * 0.08 ), ( crand() * 0.08 ), ( crand() * 0.08 ) );
			vKick = -vKick;
            vKick.Multiply( ( fIntensity * fIntensityScale ) );
            com->KickView( ent, vKick, 0, 50, 100 );
        }
	}
}

///////////////////////////////////////////////////////////////////////////////
//	com_CalcChildPositions
//
//	if a parent train stops or re-adjusts it's origin outside of the physics
//  system, this function should be called..
//
///////////////////////////////////////////////////////////////////////////////

void	com_CalcChildPositions (userEntity_t *self)
{
    userEntity_t	*child;
	CVector			forward, right, up, offset, new_org;
	qboolean		bForceMove = FALSE;

	// get a pointer to the child list
	child = self->teamchain;

	if (!child)
		return;

	if (self->s.event == EV_ENTITY_FORCEMOVE)
		bForceMove = TRUE;


	// calculate the transformation matrix
	AngleToVectors(self->s.angles, forward, right, up);

    while (child)				// while we have valid pointers
    {
		// transform the offset from the parent
		offset = child->childOffset;
		// negate Y
		offset.y = -offset.y;

		new_org.x = DotProduct(offset, forward) + self->s.origin.x;
		new_org.y = DotProduct(offset, right) + self->s.origin.y;
		new_org.z = DotProduct(offset, up) + self->s.origin.z;

		// update the child origin
		child->s.origin = new_org;
		child->s.old_origin = child->s.origin;

		child->s.origin_hack = child->s.origin;

		// update the transformed origin difference
		child->transformedOffset = child->s.origin - self->s.origin;

		// update the angles of the child
		child->s.angles = self->s.angles + child->s.angle_delta;

		// make sure they are between 0-359
		child->s.angles.x = AngleMod ( child->s.angles.x );
		child->s.angles.y = AngleMod ( child->s.angles.y );
		child->s.angles.z = AngleMod ( child->s.angles.z );

		// are we teleporting the parent?  if so, teleport the child entities also
		if (bForceMove)
		{
			child->s.event = EV_ENTITY_FORCEMOVE;

			// func_dynalights require special handling
			if (!stricmp (child->className, "func_dynalight"))
			{
				if (child->spawnflags &	0x01)	// is the light on?
				{
					// turn it off for a frame while it teleports
					func_dynalight_off(child);
					// now set it to come back on 
					child->think = func_dynalight_on;
					child->nextthink = gstate->time + 0.01;
				}
			}
		
		}
		gstate->LinkEntity( child );
		// get next child
	    child = child->teamchain;
    }
}



///////////////////////////////////////////////////////////////////////////////
//	com_CalcMoveFinished
//
//	makes sure a door and all its linked doors have completely closed before
//	toggling an associated func_areaportal
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	com_CalcMoveFinished
//
//	makes sure a door and all its linked doors have completely closed before
//	toggling an associated func_areaportal
///////////////////////////////////////////////////////////////////////////////

void areaportal_set (userEntity_t *self, int state);
void	com_CalcMoveFinished (userEntity_t *self)
{
	doorHook_t		*hook = (doorHook_t *) self->userHook;
	doorHook_t		*lhook;
	int				all_closed = TRUE;
	userEntity_t	*linked_door;

	self->nextthink = -1.0;

	self->s.origin_hack = self->s.origin;

	// NSS[12/6/99]:If we have been freed then don't do a damned thing
	if(!_stricmp(self->className,"freed"))
		return;
	if (hook->done_think) hook->done_think (self);

	//	if this door is linked to a portal, and it is about to close the portal
	//	make sure that ALL linked doors are closed before this door
	//	can be reactivated, and before it can toggle an areaportal

	if( hook->portal != NULL ) 
	{
		if( ( hook->portal->light_level == AREAPORTAL_OPEN ) && 
			( hook->state == STATE_BOTTOM ) )
		{
			if( self->owner != NULL )
			{
				for( linked_door = self->owner; linked_door; linked_door = lhook->linked_door )
				{
					lhook = ( doorHook_t * ) linked_door->userHook;

					if( lhook->state != STATE_BOTTOM )
					{
						areaportal_set( hook->portal, AREAPORTAL_OPEN );
						if( lhook->portal )
						{
							areaportal_set( lhook->portal, AREAPORTAL_OPEN );
						}
						//	not all linked doors have closed, so come back here next frame and try again
						self->think = com_CalcMoveFinished;
						self->nextthink = gstate->time + 0.1;
						return;
					}
				}
			}

			areaportal_set( hook->portal, AREAPORTAL_CLOSED );
			if( lhook->portal )
			{
				areaportal_set( lhook->portal, AREAPORTAL_CLOSED );
			}
		}
		else if( ( hook->portal->light_level == AREAPORTAL_CLOSED ) &&
			( ( hook->state == STATE_UP ) || ( hook->state == STATE_TOP ) ) )
		{
			areaportal_set( hook->portal, AREAPORTAL_OPEN );
			self->think = com_CalcMoveFinished;
			self->nextthink = gstate->time + 0.1;
		}
	}
/*
	if (hook->portal && hook->portal->light_level == AREAPORTAL_OPEN && hook->state == STATE_BOTTOM)
	{
		if (self->owner)
		{
			for (linked_door = self->owner; linked_door; linked_door = lhook->linked_door)
			{
				lhook = (doorHook_t *) linked_door->userHook;

				if (lhook->state != hook->state)
				{
					//	not all linked doors have closed, so come back here next frame and try again
					self->think = com_CalcMoveFinished;
					self->nextthink = gstate->time + 0.1;

//					gstate->Con_Printf ("Door could not close portal\n");
					return;
				}
			}
		}

//		gstate->Con_Printf ("Door closed portal\n");
		hook->portal->use (hook->portal, self, self);
	}
*/
}

///////////////////////////////////////////////////////////////////////////////
//	com_CalcMoveDone
//
//	After moving, set origin to exact final destination
//	Set velocity to 0
///////////////////////////////////////////////////////////////////////////////

void	com_CalcMoveDone (userEntity_t *self)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;
	CVector		move;

	if( hook == NULL )
	{
		return;
	}

	//	move to final destination
	move = hook->final_dest - self->s.origin;
	//VectorSubtract( hook->final_dest.vec(), self->s.origin, move );

	gstate->PushMove (self, move, zero_vector);

	//&&& AMW 6.26.98 - re-orient the linked entities also so they end up in
	// the right place
	if (self->teamchain && !(self->flags & FL_TEAMSLAVE))
	{
		// update the positions of the child entities
		com_CalcChildPositions (self);
	}

	//	zero velocity
	self->velocity.Zero();
	//com->ZeroVector (self->velocity);		//&&& AMW 6.26.98 - this is NOT good.. it stops the train inbetween path_corners

////	self->think = com_CalcMoveFinished;
////	self->nextthink = gstate->time + 0.1;
#pragma message( "// SCG[11/24/99]: FIXME: this should not be happening!" )
	if( _stricmp( self->className, "freed" ) == 0 )
	{
		return;
	}

	gstate->LinkEntity( self );

	com_CalcMoveFinished(self);

}

///////////////////////////////////////////////////////////////////////////////
//	com_CalcAbsolutePosition
//
//	Set origin to exact final destination
//	Set velocity to 0
///////////////////////////////////////////////////////////////////////////////

void	com_CalcAbsolutePosition (userEntity_t *self)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	//	move to final destination
	self->s.origin = hook->final_dest;
	self->s.old_origin = self->s.origin;

	gstate->LinkEntity( self );

	//&&& AMW 6.26.98 - re-orient the linked entities also so they end up in
	// the right place
	if (self->teamchain && !(self->flags & FL_TEAMSLAVE))
	{
		// update the positions of the child entities

		com_CalcChildPositions (self);
	}

	//	zero velocity
	self->velocity.Zero();		//&&& AMW 6.26.98 - this is NOT good.. it stops the train inbetween path_corners

	com_CalcMoveFinished(self);

//elf->think = train_next; //com_CalcMoveFinished;
//elf->nextthink = gstate->time + 0.01;

}


///////////////////////////////////////////////////////////////////////////////
//	com_CalcMove
//
//	calculate self.velocity and self.nextthink to reach dest from
//	self.origin traveling at speed
///////////////////////////////////////////////////////////////////////////////

void	com_CalcMove (userEntity_t *self, const CVector &tdest, float tspeed, think_t func)
{
	CVector		vdestdelta;
	float		len, traveltime;
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	if (!tspeed)
		return;

	hook->done_think = func;
	hook->final_dest = tdest;
	self->think = com_CalcMoveDone;

	if (tdest.x == self->s.origin.x && tdest.y == self->s.origin.y && tdest.z == self->s.origin.z)
	{
		self->velocity.Zero();
		self->nextthink = gstate->time + 0.1;
		return;
	}

	// set destdleta to the vector needed to move
	vdestdelta = tdest - self->s.origin;

	// calculate length of vector
	len = vdestdelta.Length();

	// divide by speed to get time to read dest
	traveltime = len / tspeed;

	if (traveltime < 0.1)
	{
		self->velocity.Zero();
		self->nextthink = gstate->time + 0.1;
		return;
	}

	// set netthink to trigger a thnk when dest is reached
	self->nextthink = gstate->time + traveltime;
	hook->end_move_time = self->nextthink;

	// scale the destdelta vector by the tme spent traveling to get velocity
	vdestdelta = vdestdelta * (1 / traveltime);
	self->velocity = vdestdelta;
}

///////////////////////////////////////////////////////////////////////////////
//	com_CalcAngleMoveDone
//
//	After moving, set origin to exact final destination
//	Set velocity to 0
///////////////////////////////////////////////////////////////////////////////

void	com_CalcAngleMoveDone (userEntity_t *self)
{
	doorHook_t	*hook = (doorHook_t *) self->userHook;
	CVector		amove;

	if (!(self->flags & FL_HCHILD))
	{
		amove = hook->final_dest - self->s.angles;
		//VectorSubtract( hook->final_dest.vec(), self->s.angles, amove );
		gstate->PushMove (self, zero_vector, amove);
	}

	if (hook->bitflags & DF_QUAKE)
	{
		com_SpawnEarthQuake( self, self->mass * 100, 1.0, 0.0 );
	}

	self->avelocity.Zero();
	//com->ZeroVector (self->avelocity);

	self->think = com_CalcMoveFinished;
	self->nextthink = gstate->time + 0.1;

}

///////////////////////////////////////////////////////////////////////////////
//	com_AcceleratedAngleMoveDone
//
///////////////////////////////////////////////////////////////////////////////

void	com_CalcAcceleratedAngleMove (userEntity_t *self)
{
	CVector		avelocity;

	doorHook_t	*hook = (doorHook_t *) self->userHook;

	hook->steps--;

	avelocity = self->avelocity;

	avelocity = avelocity + hook->velocityInc;

 	self->nextthink = gstate->time + 0.1;
	self->think = com_CalcAcceleratedAngleMove;

	// adjust the last step
	if ((hook->steps == 1) && !(hook->bounceSteps & 1))
	{
		avelocity = hook->final_dest - self->s.angles;
		avelocity = avelocity * 10.0;
	}

	if (hook->steps <= 0)
	{
		// odd is up..
		hook->bounceSteps--;

		if (hook->bitflags & DF_DUST) 
		{
			com_GenerateDustCloud( self, self->mass * 100 / 35 );
		}

		if (hook->bitflags & DF_QUAKE)
		{
			com_SpawnEarthQuake( self, self->mass * 100, 1.0, 0.0 );
		}

		if ((hook->bitflags & DF_BOUNCE) && (hook->bounceSteps > 0)) 
		{
			// calculate a bouncing path back up
			self->avelocity.x = -(hook->speed * self->movedir.x * 0.1); 
			self->avelocity.y = -(hook->speed * self->movedir.y * 0.1);
			self->avelocity.z = -(hook->speed * self->movedir.z * 0.1);

			self->nextthink = gstate->time + 0.01;

			hook->steps = 4;
		}
		else
		{
			com_CalcAngleMoveDone (self);
		}
	}
	else
	{
		self->avelocity = avelocity;
	}
}


///////////////////////////////////////////////////////////////////////////////
//	com_CalcAngleMove
//
//	calculate self.velocity and self.nextthink to reach dest from
//	self.origin traveling at speed
///////////////////////////////////////////////////////////////////////////////

void	com_CalcAngleMove (userEntity_t *self, const CVector &destangle, float tspeed, think_t func)
{
	CVector		destdelta;
	float		len, traveltime, velocityInc;
	doorHook_t	*hook = (doorHook_t *) self->userHook;

	if (!tspeed)
		return;

	// save the dest angles we're moving to
	hook->destangle = destangle;

	// set destdelta to the vector needed to move
	destdelta = destangle - self->s.angles;

	// calculate length of vector
	len = destdelta.Length();

	// set the final destination
	hook->final_dest = self->s.angles + destdelta;

	// simulate physics and the object dropping or accelerating
	if (hook->bitflags & DF_BOUNCE || hook->bitflags & DF_ACCEL)
	{
		hook->done_think = func;
		
		traveltime = len / (tspeed / 2);

		hook->steps = traveltime / 0.1;

		if (hook->steps > 0)
		{
			velocityInc = tspeed / hook->steps;
			self->nextthink = gstate->time + 0.1;
			self->think = com_CalcAcceleratedAngleMove;

			// set the velocities to move
			if (destdelta.x)
			{
				self->avelocity.x = (destdelta.x > 0) ? velocityInc : -velocityInc;
			}
			else 
			if (destdelta.y)
			{
				self->avelocity.y = (destdelta.y > 0) ? velocityInc : -velocityInc;
			}
			else 
			if (destdelta.z)
			{
				self->avelocity.z = (destdelta.z > 0) ? velocityInc : -velocityInc;
			}

			// set the velocity accelerator
			hook->velocityInc = self->avelocity;
		}
		else
		{
			com_CalcAngleMoveDone (self);
		}
	}
	else  // the 'normal' method
	{

		// divide by speed to get time to reach dest
		traveltime = len / tspeed;

		hook->done_think = func;

		if (traveltime)
		{
			// set nextthink to trigger a think when dest is reached
			self->nextthink = gstate->time + traveltime;
			self->think = com_CalcAngleMoveDone;

			// scale the destdelta vector by the time spent traveling to get velocity
			destdelta = destdelta * (1 / traveltime);
			self->avelocity = destdelta;
		}
		else
		{
			com_CalcAngleMoveDone (self);
		}
	}
}



///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_common_register_func()
{
	gstate->RegisterFunc("func_dynalight_on",func_dynalight_on);
	gstate->RegisterFunc("com_CalcMoveFinished",com_CalcMoveFinished);
	gstate->RegisterFunc("com_CalcMoveDone",com_CalcMoveDone);
	gstate->RegisterFunc("com_CalcAcceleratedAngleMove",com_CalcAcceleratedAngleMove);
	gstate->RegisterFunc("com_CalcAngleMoveDone",com_CalcAngleMoveDone);
	gstate->RegisterFunc("com_FindParent",com_FindParent);
}
