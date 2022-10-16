////////////////////////////////////////////////////////////////
// Body list routines
//
// just a list of models that is used for the bodies of dead 
// players
////////////////////////////////////////////////////////////////

#include "world.h"
//#include "triggers.h"// SCG[1/23/00]: not used
//#include "client.h"// SCG[1/23/00]: not used
//#include "ai_frames.h"// SCG[1/23/00]: not used

#define		MAX_BODIES		8

static		userEntity_t	*pBodyList[MAX_BODIES];
static		float			bodySpawnTime[MAX_BODIES];

void PlayClientSound(userEntity_t *self, char *soundfile);


/////////////////////////
//  bodylist_init
/////////////////////////

void bodylist_init (userEntity_t *self)
{
	int				i;

	for (i = 0; i < MAX_BODIES; i++)
	{
		pBodyList[i] = gstate->SpawnEntity ();
		pBodyList[i]->className = "body";

		pBodyList[i]->svflags	= SVF_NOCLIENT;
		pBodyList[i]->modelName = NULL;
		pBodyList[i]->movetype	= MOVETYPE_NONE;
		pBodyList[i]->solid		= SOLID_NOT;
		pBodyList[i]->s.color.Set(0,0,0);

		// reset the spawn time.. the lowest (earliest) get chosen first
		bodySpawnTime[i]		= 0.0;	
	}
}

/////////////////////////
//	bodylist_get_index
/////////////////////////
int bodylist_get_index(userEntity_t *self)
{
	for (int i=0; i < MAX_BODIES; i++)
	{
		if (self == pBodyList[i])	
			return i;
	}

	// return the default body index
	return (0);
}

/////////////////////////
//	bodylist_freebody
/////////////////////////
void bodylist_freebody(userEntity_t *self)
{
	self->takedamage = DAMAGE_NO;		// do not take damage any longer
    self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;			// do not clip any longer
	self->svflags = SVF_NOCLIENT|SVF_DEADMONSTER;		// do not send this entity to the client
	self->s.renderfx |= RF_NODRAW;		// make sure we don't draw the model
    self->clipmask = -1;
	self->s.color.Set(0,0,0);
	self->think = NULL;
    gstate->LinkEntity(self);
	bodySpawnTime[bodylist_get_index(self)] = 0.0;	// ready for immediate use
}

/////////////////////////
//	bodylist_die
/////////////////////////
void bodylist_die (userEntity_t *self, userEntity_t *inflictor, userEntity_t *attacker, int damage, CVector &point)
{
	float di;

	di = gstate->damage_inflicted; // jar 19990305 save damage_inflicted 
	if (self->health < -40)
	{
		// play gib/splashing sound
//		gstate->sound (self, CHAN_VOICE, "player/udeath.wav", 255, ATTN_NORM);
		PlayClientSound(self,"udeath.wav");

//		if (damage >= PLAYER_GIB_POINT && sv_violence->value > 1)
        if (sv_violence->value == 0)
		{
			while(gstate->damage_inflicted < 100) 
				gstate->damage_inflicted = gstate->damage_inflicted * 5; // JAR 19990305 crank up the damage... bloodlust

			gib_client_die(self,inflictor);	// JAR 1990306 Common player body giblet tossing goodness

		}

		bodylist_freebody(self);
/*
		self->takedamage = DAMAGE_NO;		// do not take damage any longer
        self->movetype = MOVETYPE_NONE;
		self->solid = SOLID_NOT;			// do not clip any longer
		self->svflags = SVF_NOCLIENT|SVF_DEADMONSTER;		// do not send this entity to the client
		self->s.renderfx |= RF_NODRAW;		// make sure we don't draw the model
        self->clipmask = -1;
        gstate->LinkEntity(self);
		bodySpawnTime[bodylist_get_index(self)] = 0.0;	// ready for immediate use
*/
	}
    gstate->damage_inflicted = di;	// JAR 19990305 restore damage_inflicted
}


/////////////////////////
//	bodylist_get_oldest
/////////////////////////
int bodylist_get_oldest(void)
{
	float oldestTime = 999999.00;
	int	oldestIdx = 0;

	for (int i=0; i < MAX_BODIES; i++)
	{
		if (bodySpawnTime[i] < oldestTime)	// the lower the time, the older the body
		{
			oldestTime = bodySpawnTime[i];
            oldestIdx = i;
		}
	}

	// return the index of the oldest body entity
	return (oldestIdx);
}

/////////////////////////
//	bodylist_fade
/////////////////////////
void bodylist_fade(userEntity_t *self)
{
	if (self->s.alpha < 0.1)
	{
		bodylist_freebody(self);
		return;
	}

	self->s.alpha -= 0.02;
	self->nextthink = gstate->time + 0.1;
}

/////////////////////////
//	bodylist_add
/////////////////////////
void bodylist_add (userEntity_t *self, float fadeTime)
{
	if ((fadeTime == -1) && (deathmatch->value || coop->value))
	{
		fadeTime = 10;
	}

	int nBodyIdx = bodylist_get_oldest();

	userEntity_t *pBody = pBodyList[nBodyIdx];

	if (!pBody)
		return;

	// set up the spawn time so we can always choose the oldest entity
	bodySpawnTime[nBodyIdx] = gstate->time;

	// save the entity number
	int s_number = pBody->s.number;
	// copy over the entity state
	pBody->s = self->s;
    
	// restore the entity number
	pBody->s.number = s_number;
	pBody->s.event |= EV_ENTITY_FORCEMOVE;	// amw: fixes ghosting when respawning
    pBody->s.frameInfo.frameFlags = 0;
	// pBody->svflags	= self->svflags;
    pBody->svflags = SVF_DEADMONSTER;
	pBody->absmin	= self->absmin;
	pBody->absmax	= self->absmax;
	pBody->size		= self->size;

	// pBody->solid	= self->solid;
	pBody->clipmask = self->clipmask;
    // pBody->clipmask = MASK_PLAYERSOLID;
	pBody->owner	= self->owner;
	pBody->movetype = self->movetype;
    pBody->solid = SOLID_BBOX;
	pBody->die	= bodylist_die;
	pBody->takedamage = DAMAGE_YES;
    // Logic [8/5/99]: ensure there are no thinks running for bodies.
	// cek: unless you want them to fade away...
	if(fadeTime == -1)
		pBody->think = NULL;
	else
	{
		if (pBody->s.renderfx & RF_IN_NITRO)
		{
			pBody->s.renderfx &= ~RF_IN_NITRO;
			pBody->s.color.Set(0.3,0.3,0.8);
		}
		pBody->s.renderfx |= RF_TRANSLUCENT;
		pBody->think = bodylist_fade;
		pBody->nextthink = gstate->time + fadeTime;
		pBody->s.alpha = 1.0;
	}

	gstate->LinkEntity (pBody);

	// increment the body count
	nBodyIdx++;
	if (nBodyIdx >= MAX_BODIES)
		nBodyIdx = 0;
}	

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_bodylist_register_func()
{
	gstate->RegisterFunc("bodylist_die",bodylist_die);
	gstate->RegisterFunc("bodylist_fade",bodylist_fade);
}
