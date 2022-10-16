#include "world.h"
#include "ai_utils.h"
//#include "ai_weapons.h"// SCG[1/23/00]: not used
#include "ai_frames.h"
#include "nodelist.h"
#include "ai_func.h"
//#include "MonsterSound.h"// SCG[1/23/00]: not used

/* ***************************** define types ****************************** */
DllExport void cine_gharroth( userEntity_t *self );
DllExport void cine_pgharroth( userEntity_t *self );
DllExport void cine_hiro( userEntity_t *self );
DllExport void cine_phiro( userEntity_t *self );
DllExport void cine_mikiko( userEntity_t *self );
DllExport void cine_smikiko( userEntity_t *self );
DllExport void cine_mikikofly( userEntity_t *self );
DllExport void cine_priest( userEntity_t *self );
DllExport void cine_superfly( userEntity_t *self );
DllExport void cine_toshiro( userEntity_t *self );
DllExport void cine_tatsuo( userEntity_t *self );
DllExport void cine_kage( userEntity_t *self );
DllExport void cine_casseti( userEntity_t *self );
DllExport void cine_fmg( userEntity_t *self );
DllExport void cine_osaka( userEntity_t *self );
DllExport void cine_usagi( userEntity_t *self );
DllExport void cine_gusagi( userEntity_t *self );
DllExport void cine_suagis( userEntity_t *self );
DllExport void cine_ninja( userEntity_t *self );
DllExport void cine_inshiro( userEntity_t *self );
DllExport void cine_fatworker( userEntity_t *self );
DllExport void cine_thinworker( userEntity_t *self );
DllExport void cine_drug1( userEntity_t *self );
DllExport void cine_drug2( userEntity_t *self );
DllExport void cine_charon( userEntity_t *self );
DllExport void cine_mwguard( userEntity_t *self );


#define MODEL_FILENAME_LENGTH	MAX_QPATH


// NSS[12/6/99]:Added this to trap cinematic character thinking.
void cinematic_think(userEntity_t *self)
{
	AI_Dprintf("%s: Vel:%f  Frame:%d\n",self->className,self->velocity.Length(),self->s.frame);
	AI_TaskThink(self);
	AI_Dprintf("%s: Vel:%f  Frame:%d\n",self->className,self->velocity.Length(),self->s.frame);
}


// NSS[12/6/99]:Added this to trap cinematic character thinking.
void cinematic_parse_epairs(userEntity_t *self)
{
	AI_ParseEpairs(self);
	self->think = cinematic_think;
	self->nextthink = gstate->time + 0.1f;

}


void cinematic_character_init( userEntity_t *self, char *pModelName, char *pMonsterName )
{
	playerHook_t *hook	= AI_InitMonster( self, TYPE_CINE_ONLY );

	self->className		= pMonsterName;
	
	char szModelName[MODEL_FILENAME_LENGTH];
	strncpy( szModelName, pModelName, MODEL_FILENAME_LENGTH );

	strncat( szModelName, gstate->mapName, 4 );
	strncat( szModelName, ".dkm", 4 );

    self->s.modelindex = gstate->ModelIndex( szModelName );

	ai_get_sequences(self);

	self->s.mins.Set( -12, -12, -24 );
	self->s.maxs.Set( 12, 12, 30 );

	///////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////
	
	hook->fnStartAttackFunc = NULL;
	hook->fnAttackFunc		= NULL;

	hook->think_time		= THINK_TIME;
	
	self->takedamage		= DAMAGE_NO;
	
	self->think				= cinematic_parse_epairs;
	self->nextthink			= gstate->time + 0.2;

	self->mass				= 400;

	hook->ai_flags			&= ~AI_ALWAYS_USENODES;

	self->clipmask			= MASK_MONSTERSOLID;

	gstate->LinkEntity(self);

	AI_InitNodeList( self );

	self->flags				= ( FL_CINEMATIC | FL_BOT | FL_PUSHABLE );
}

void cine_gharroth( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_ghar_", "cine_gharroth" );
}

void cine_pgharroth( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_pghar_", "cine_pgharroth" );
}

void cine_hiro( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_hiro_", "cine_hiro" );
}

void cine_phiro( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_phiro_", "cine_phiro" );
}

void cine_mikiko( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_mikiko_", "cine_mikiko" );
}

void cine_smikiko( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_smikiko_", "cine_smikiko" );
}

void cine_mikikofly( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_mikikofly_", "cine_mikikofly" );
}

void cine_priest( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_priest_", "cine_priest" );
}

void cine_superfly( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_super_", "cine_superfly" );
}

void cine_toshiro( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_tosh_", "cine_toshiro" );
}

void cine_tatsuo( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_tatsuo_", "cine_tatsuo" );
}

void cine_kage( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_kage_", "cine_kage" );
}

void cine_casseti( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_casseti_", "cine_casseti" );
}

void cine_fmg( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_fmg_", "cine_fmg" );
}

void cine_osaka( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_osaka_", "cine_osaka" );
}

void cine_usagi( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_usagi_", "cine_usagi" );
}

void cine_gusagi( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_gusagi_", "cine_gusagi" );

	self->s.alpha = 0.60;
	self->s.renderfx = RF_TRANSLUCENT;
	self->s.color.Set( 0.8, 0.8, 0.8 );
}

void cine_suagis( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_suagis_", "cine_suagis" );
}

void cine_ninja( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_ninja_", "cine_ninja" );
}

void cine_inshiro( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_inshiro_", "cine_inshiro" );
}

void cine_fatworker( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_fat_", "cine_fatworker" );
}

void cine_thinworker( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_thin_", "cine_thinworker" );
}

void cine_drug1( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_drug1_", "cine_drug1" );
}

void cine_drug2( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_drug2_", "cine_drug2" );
}

void cine_charon( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_char_", "cine_charon" );
}

void cine_mwguard( userEntity_t *self )
{
	cinematic_character_init( self, "models/cinematic/c_mwguard_", "cine_mwguard" );
}

