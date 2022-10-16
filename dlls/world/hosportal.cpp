#include "world.h"
#include "ai_utils.h"


// export
DllExport void misc_hosportal(userEntity_t *self);
DllExport void misc_fountain(userEntity_t *self);

// SCG[10/29/99]: Gets the hook and makes sure it's valid
#define hosportal_header()	{ pHosportalHook = ( hosportalHook_t * ) self->userHook; if( pHosportalHook == NULL ) return; }

void add_hosportal_use_fx( userEntity_t *self );
void hosportal_recharge_think( userEntity_t *self );
void hosportal_give_health_think( userEntity_t *self );
void hosportal_use_charging( userEntity_t *self, userEntity_t *other, userEntity_t *activator );
void hosportal_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator );

int AI_IsInFOV( userEntity_t *self, userEntity_t *targ );

// defines
#define HOS_IGNORE_RATE ( 2.0 )
#define HOS_FX_DELAY    ( 5.0 )    // 5.0

#define FRAME_JUICEAVAIL   1   // frame # to show juice is available (green)
#define FRAME_NOJUICEAVAIL 0   // frame # to show juice is not available (red)

#define	IS_INSTAGIB				(deathmatch->value && dm_instagib->value)

// SCG[11/19/99]: Save game stuff
#define	HOSPORTALHOOK(x) (int)&(((hosportalHook_t *)0)->x)
field_t hosportal_hook_fields[] = 
{
	{"style",			HOSPORTALHOOK(style),			F_BYTE},
	{"max_juice",		HOSPORTALHOOK(max_juice),		F_INT},
	{"current_juice",	HOSPORTALHOOK(current_juice),	F_INT},
	{"fx_start_time",	HOSPORTALHOOK(fx_start_time),	F_FLOAT},
	{"activator",		HOSPORTALHOOK(activator),		F_EDICT},
	{"vOrigin",			HOSPORTALHOOK(vOrigin),			F_VECTOR},
	{"sound_use",		HOSPORTALHOOK(sound_use),		F_INT},
	{"sound_use_done",	HOSPORTALHOOK(sound_use_done),	F_INT},
	{"sound_particles",	HOSPORTALHOOK(sound_particles),	F_INT},
	{"sound_healthup",	HOSPORTALHOOK(sound_healthup),	F_INT},
	{"sound_use_out",	HOSPORTALHOOK(sound_use_out),	F_INT},
	{"sound_recharged",	HOSPORTALHOOK(sound_recharged),	F_INT},
	{"use_fx",			HOSPORTALHOOK(use_fx),			F_FUNC},
	{NULL, 0, F_INT}
};

void hosportal_hook_save( FILE *f, edict_t *ent )
{
	AI_SaveHook( f, ent, hosportal_hook_fields, sizeof( hosportalHook_t ) );
}

void hosportal_hook_load( FILE *f, edict_t *ent )
{
	AI_LoadHook( f, ent, hosportal_hook_fields, sizeof( hosportalHook_t ) );
}

void add_hosportal_use_fx( userEntity_t *self )
// show cool fx while the hosportal is in use
{
	hosportalHook_t *pHosportalHook;

	// SCG[10/29/99]: This gets the pHosportalHook pointer and checks it's validity
	hosportal_header();

	CVector mins( -16,-16,-20 );
	CVector maxs( 16,16,32 );

	// create spiral particle effect around each hand
	gstate->WriteByte( SVC_TEMP_ENTITY );
	gstate->WriteByte( TE_SPIRAL_PARTICLES ); 
	gstate->WritePosition( self->s.origin );
	gstate->WritePosition( mins );
	gstate->WritePosition( maxs );
	gstate->WriteByte( HOS_FX_DELAY );	// display for # seconds
	gstate->MultiCast( self->s.origin, MULTICAST_PVS );

	// SCG[11/1/99]: We need to keep track of the FX time.
	pHosportalHook->fx_start_time = gstate->time + HOS_FX_DELAY - 3;

	// sound fx for particles
	gstate->StartEntitySound( self, CHAN_AUTO, pHosportalHook->sound_particles, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );
}


void fountain_prethink(userEntity_t *self)
{
	//NSS[11/29/99]:Untrack Hosportal
	if(self->hacks < gstate->time)
	{
		RELIABLE_UNTRACK(self);
		self->prethink = NULL;
	}

}

void add_fountain_use_fx( userEntity_t *self )
{
	//NSS[11/29/99]:Get Header
	hosportalHook_t *pHosportalHook;
	hosportal_header();
	
	trackInfo_t tinfo;
	// clear this variable
	memset(&tinfo, 0, sizeof(tinfo));

	tinfo.ent			=self;
	tinfo.srcent		=self;
	tinfo.fru.Zero();
	tinfo.length		= 0;	
	tinfo.Long1			= ART_FOUNTAIN;
	tinfo.flags			= TEF_FXFLAGS|TEF_SRCINDEX|TEF_LENGTH|TEF_LONG1;
	tinfo.fxflags		= TEF_ARTIFACT_FX|TEF_FX_ONLY;
		
	com->trackEntity(&tinfo,MULTICAST_ALL);
	
	//NSS[11/29/99]:function to untrack this entity 
	self->prethink = fountain_prethink;

	pHosportalHook->fx_start_time = gstate->time + HOS_FX_DELAY - 3;
	//NSS[11/29/99]:time to untrack 
	self->hacks =  gstate->time + HOS_FX_DELAY - 3.2;

	// cek[12-14-99] add some sound
	gstate->StartEntitySound( self, CHAN_AUTO, pHosportalHook->sound_particles, 0.85f, ATTN_NORM_MIN, ATTN_NORM_MAX );
}

void hosportal_recharge_think( userEntity_t *self )
{
	hosportalHook_t *pHosportalHook;

	// SCG[10/29/99]: This gets the pHosportalHook pointer and checks it's validity
	hosportal_header();

	if( pHosportalHook->current_juice < pHosportalHook->max_juice )     // any juice depleted from hosportal?
	{
		// SCG[10/29/99]: Give the hosportal health
		pHosportalHook->current_juice++;

		// SCG[10/29/99]: Set the next think time
		if( coop->value == 1 )
		{
			self->nextthink = gstate->time + 1;
		}
		else
		{
			self->nextthink = gstate->time + 0.1;
		}
	}
	else
	{
		// SCG[10/29/99]: Play the recharged sound
		gstate->StartEntitySound(self, CHAN_AUTO, pHosportalHook->sound_recharged, 0.5, ATTN_NORM_MIN, ATTN_NORM_MAX);

		// SCG[10/29/99]: Set the frame to show that the hosportal has been charged
		self->s.frame = FRAME_JUICEAVAIL;
		self->s.frameInfo.frameFlags |= FRAME_FORCEINDEX;

		// SCG[10/29/99]: We seem to be charged, no need to think
		self->think = NULL;

		// SCG[10/29/99]: reset the use function
		self->use = hosportal_use;
	}

	// SCG[10/29/99]: set the next think
	self->nextthink = gstate->time + 0.1;
}

void hosportal_reset( userEntity_t *self )
{
	hosportalHook_t *pHosportalHook;

	// SCG[10/29/99]: This gets the pHosportalHook pointer and checks it's validity
	hosportal_header();

	// SCG[10/29/99]: set the think to the recharge think
	self->think = hosportal_recharge_think;
	// SCG[10/29/99]: set the next think
	self->nextthink = gstate->time + 0.1;
	// SCG[10/29/99]: set the use function
	self->use = hosportal_use_charging;

	// SCG[10/29/99]: clear the activator
	pHosportalHook->activator = NULL;

	self->s.frame = FRAME_NOJUICEAVAIL;
	self->s.frameInfo.frameFlags |= FRAME_FORCEINDEX;
}

void hosportal_give_health_think( userEntity_t *self )
{
	hosportalHook_t *pHosportalHook;

	// SCG[10/29/99]: This gets the pHosportalHook pointer and checks it's validity
	hosportal_header();

	// SCG[10/29/99]: Get the player hook
	playerHook_t	*pPlayerHook = ( playerHook_t * ) pHosportalHook->activator->userHook;

	// SCG[10/29/99]: make sure it's valid
	if( pPlayerHook == NULL )
	{
		hosportal_reset( self );
		return;
	}

	// SCG[10/29/99]: Reset is the activator is no longer valid
	if( pHosportalHook->activator == NULL || pHosportalHook->activator->deadflag != DEAD_NO )
	{
		hosportal_reset( self );
		return;
	}

	if( AI_IsInFOV( pHosportalHook->activator, self ) == FALSE )
	{
		hosportal_reset( self );
		return;
	}

	// SCG[10/29/99]: reset if we are at full health
	if( pHosportalHook->activator->health >= pPlayerHook->base_health )
	{
		gstate->StartEntitySound(self, CHAN_AUTO, pHosportalHook->sound_recharged, 0.5, ATTN_NORM_MIN, ATTN_NORM_MAX);

		hosportal_reset( self );
		return;
	}

	// SCG[10/29/99]: Play the sound for hosportal out of juice
	if( pHosportalHook->current_juice <= 0 )
	{
		gstate->StartEntitySound(self, CHAN_AUTO, pHosportalHook->sound_healthup, 0.5, ATTN_NORM_MIN, ATTN_NORM_MAX);

		hosportal_reset( self );
		return;
	}

	// SCG[10/29/99]: Check the distance of the player and make sure we can give them health
	CVector vLength = pHosportalHook->vOrigin - pHosportalHook->activator->s.origin;
	float	fDist = fabs( vLength.Length() );

	// SCG[10/29/99]: If the activator is too far from the hosportal, stop giving the acivator health
	// SCG[10/29/99]: and go into recharge mode.
	if( fDist > 64 && (pHosportalHook->activator->flags & FL_CLIENT))
	{
		hosportal_reset( self );
		return;
	}

	// SCG[11/1/99]: Restart the FX if we need to
	if( gstate->time >= pHosportalHook->fx_start_time )
	{
		pHosportalHook->use_fx( self );
	}

	// SCG[10/29/99]: Give the player health
	com->Health( pHosportalHook->activator, 1.0f, pPlayerHook->base_health );

	pHosportalHook->current_juice--;

	// SCG[10/29/99]: Set the next think time
	self->nextthink = gstate->time + 0.2;
}

void hosportal_use_charging( userEntity_t *self, userEntity_t *other, userEntity_t *activator )
{
	hosportalHook_t *pHosportalHook;

	// SCG[10/29/99]: This gets the pHosportalHook pointer and checks it's validity

	hosportal_header();

	// SCG[10/29/99]: we should play a sound here that tells the 
	// SCG[10/29/99]: activator that he/she cannot use the hosportal yet.
	gstate->StartEntitySound(self, CHAN_AUTO, pHosportalHook->sound_recharged, 0.5, ATTN_NORM_MIN, ATTN_NORM_MAX);
}

void hosportal_use( userEntity_t *self, userEntity_t *other, userEntity_t *activator )
{
	hosportalHook_t *pHosportalHook = ( hosportalHook_t * ) self->userHook;

	// SCG[10/29/99]: This gets the pHosportalHook pointer and checks it's validity
	hosportal_header();

	// SCG[10/29/99]: Get the player hook
	playerHook_t	*pPlayerHook = ( playerHook_t * ) activator->userHook;

	// SCG[10/29/99]: make sure it's valid
	if( pPlayerHook == NULL )
	{
		return;
	}

	// SCG[10/29/99]: Check the distance of the player and make sure we can give them health
	CVector vLength = pHosportalHook->vOrigin - activator->s.origin;
	float	fDist = fabs( vLength.Length() );

	// SCG[10/29/99]: If the activator is too far from the hosportal, stop giving the acivator health
	// SCG[10/29/99]: and go into recharge mode.
	if( fDist > 64 && (activator->flags & FL_CLIENT))
	{
		return;
	}

	if( AI_IsInFOV( activator, self ) == FALSE )
	{
		return;
	}

	// SCG[10/29/99]: Do nothing if we are at full health
	if( activator->health >= pPlayerHook->base_health )
	{
		gstate->StartEntitySound(self, CHAN_AUTO, pHosportalHook->sound_recharged, 0.5, ATTN_NORM_MIN, ATTN_NORM_MAX);

		return;
	}

	// SCG[10/29/99]: Get a pointer to the activator
	pHosportalHook->activator = activator;

	// SCG[10/29/99]: Set the think function
	self->think = hosportal_give_health_think;

	// SCG[10/29/99]: The initial think gets set to the next server frame, 
	// SCG[10/29/99]: but after that it will be every second
	self->nextthink = 0.1;

	// SCG[10/29/99]: Set the use to hosportal_use_charging so if anyone tries to use it
	// SCG[10/29/99]: it will play a sound.
	self->use = hosportal_use_charging;

	// SCG[10/29/99]: Show effects
	//NSS[11/29/99]:Added the hook use in place of directly calling the original hosportal fx.
	pHosportalHook->use_fx( self );
}

///////////////////////
//	misc_hosportal
///////////////////////
#define ALLOW_HEALTH	0x01
void init_hosportal(userEntity_t *self)
{
	hosportalHook_t	*hook;
	CVector			temp;

	self->userHook = gstate->X_Malloc(sizeof(hosportalHook_t),MEM_TAG_HOOK);
	hook = (hosportalHook_t *) self->userHook;

	self->save = hosportal_hook_save;
	self->load = hosportal_hook_load;

	// 4.27 dsn  moved below, based on style type
	//self->s.modelindex = gstate->ModelIndex( "models/e1/hosportal.dkm" );
		
	self->mass	  = 1.0;
	self->gravity = 1.0;

	// defaults
	self->health = 100;

	hook->style         = 0;     // large hosportal
	hook->max_juice     = 100;   // maximum energy
 
	hook->current_juice = hook->max_juice;     

	if( hook->current_juice > 0 )
	{
		self->s.frame = FRAME_JUICEAVAIL;     // green, juice avail
	}
	else
	{
		self->s.frame = FRAME_NOJUICEAVAIL;   // red, no juice avail
	}

	self->s.frameInfo.frameFlags |= FRAME_FORCEINDEX; // force stay on this frame, always

	// if the model has been rotated, rotate the bounding box also
	if( self->s.angles[0] || self->s.angles[1] || self->s.angles[2] )
	{
		com->RotateBoundingBox( self );
	}
     
	self->movetype = MOVETYPE_NONE; // 4.27.99  shockwave was moving hosportals
	self->solid = SOLID_BBOX; // alias models (b-models use SOLID_BSP)

	gstate->SetOrigin( self, self->s.origin[0], self->s.origin[1], self->s.origin[2] );

	if( !self->health )                        // indestructable?
	{
		self->takedamage = DAMAGE_NO;
	}
	else
	{
		self->takedamage = DAMAGE_YES;
	}

	hook->activator	= NULL; 
	self->use		= hosportal_use;     // "use" function

	// SCG[10/29/99]: Get the origin of the brush
	hook->vOrigin.x = self->absmin.x;
	hook->vOrigin.y = self->absmin.y;
	hook->vOrigin.z = self->absmin.z;
	hook->vOrigin.x += ( self->absmax.x - self->absmin.x ) / 2;
	hook->vOrigin.y += ( self->absmax.y - self->absmin.y ) / 2;
	hook->vOrigin.z += ( self->absmax.z - self->absmin.z ) / 2;

	//NSS[11/22/99]:Added to prevent bouncey bouncy ontop of bbox things.
	self->s.iflags |= IF_SV_SENDBBOX;
}

void misc_hosportal(userEntity_t *self)
{
	if (IS_INSTAGIB || (deathmatch->value && !dm_allow_health->value))
		return;

	int				i;

	init_hosportal( self );

	hosportalHook_t	*hook = (hosportalHook_t *) self->userHook;
	if (!hook)	// WAW[11/29/99]: Certain Deathmatch situtaions might make this NULL.
		return;

  // pull variables from entity info
	for( i = 0; self->epair[i].key; i++ )
	{
		if( stricmp( self->epair[i].key, "style" ) == 0 )
		{
			hook->style = atoi( self->epair[i].value );
		}
		else if( stricmp( self->epair [i].key, "health") == 0)
		{
			self->health = atoi (self->epair[i].value);
		}
		else if( stricmp( self->epair[i].key, "max_juice" ) == 0)
		{
			hook->max_juice = atoi( self->epair[i].value );
		} 
	}

	self->className	= "misc_hosportal";

	// force sounds
	hook->sound_use       = gstate->SoundIndex( "global/h_use.wav" );
	hook->sound_use_done  = gstate->SoundIndex( "global/h_use_done.wav" );
	hook->sound_particles = gstate->SoundIndex( "global/h_hfx.wav" );
	hook->sound_healthup  = gstate->SoundIndex( "global/h_healthup.wav" );
	hook->sound_use_out   = gstate->SoundIndex( "global/h_use_out.wav" );
	hook->sound_recharged = gstate->SoundIndex( "global/h_recharged.wav" );

  // set model names (based on large, medium, small hosportals)
	switch( hook->style )
	{
		case 0 : self->s.modelindex = gstate->ModelIndex( "models/e1/hosportal1.dkm" ); 
				gstate->SetSize(self,-16.0,-16.0,-24.0,  16.0,16.0,36.0); break; // large
			break;  
		case 1 : self->s.modelindex = gstate->ModelIndex( "models/e1/hosportal2.dkm" );
				gstate->SetSize(self,-16.0,-16.0,-24.0,  16.0,16.0,24.0); break; // medium

		case 2 : self->s.modelindex = gstate->ModelIndex( "models/e1/hosportal3.dkm" );
				gstate->SetSize(self,-16.0,-16.0,-24.0,  16.0,16.0,24.0); break; // small
	}

	hook->use_fx = add_hosportal_use_fx;

	gstate->LinkEntity (self);   // let's exist!
}

void misc_fountain(userEntity_t *self)
{
	if (IS_INSTAGIB || (deathmatch->value && !dm_allow_health->value))
		return;

	int				i;

	init_hosportal( self );

	hosportalHook_t	*hook = (hosportalHook_t *) self->userHook;

  // pull variables from entity info
	for( i = 0; self->epair[i].key; i++ )
	{
		if( stricmp( self->epair[i].key, "style" ) == 0 )
		{
			hook->style = atoi( self->epair[i].value );
		}
		else if( stricmp( self->epair [i].key, "health") == 0)
		{
			self->health = atoi (self->epair[i].value);
		}
		else if( stricmp( self->epair[i].key, "max_juice" ) == 0)
		{
			hook->max_juice = atoi( self->epair[i].value );
		} 
	}

	self->s.modelindex = gstate->ModelIndex( "models/e2/a2_hlthfnt.dkm" );
	gstate->SetSize(self, -16.0, -16.0, -24.0, 16.0, 16.0, 8.0);

	self->className	= "misc_lifewater";

	hook->use_fx = add_fountain_use_fx;
	hook->sound_particles = gstate->SoundIndex("global/e_pondwaterb.wav");
	gstate->LinkEntity (self);   // let's exist!
}

///////////////////////////////////////////////////////////////////////////////
//
//  register world functions for save/load
//
///////////////////////////////////////////////////////////////////////////////
void world_hosportal_register_func()
{
	gstate->RegisterFunc("hosportal_recharge_think", hosportal_recharge_think );
	gstate->RegisterFunc("hosportal_give_health_think", hosportal_give_health_think );
	gstate->RegisterFunc("hosportal_use_charging", hosportal_use_charging );
	gstate->RegisterFunc("hosportal_use", hosportal_use );
	gstate->RegisterFunc("hosportal_hook_save", hosportal_hook_save );
	gstate->RegisterFunc("hosportal_hook_load", hosportal_hook_load );
	gstate->RegisterFunc("add_hosportal_use_fx", add_hosportal_use_fx );
	gstate->RegisterFunc("add_fountain_use_fx", add_fountain_use_fx );
}

