///////////////////////////////////////////////////////////////////////////////
//	Hiro Miyamoto
//
//	Used for scripts
///////////////////////////////////////////////////////////////////////////////

#include	"world.h"
#include	"ai_utils.h"
//#include	"ai_move.h"// SCG[1/23/00]: not used
//#include	"ai_weapons.h"// SCG[1/23/00]: not used
//#include	"ai_frames.h"// SCG[1/23/00]: not used
#include	"nodelist.h"
#include	"ai.h"
#include	"ai_func.h"
#include	"Sidekick.h"

///////////////////////////////////////////////////////////////////////////////
//	exports
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//	defines
///////////////////////////////////////////////////////////////////////////////

#define	NPC_MAX_TORSO_TWIST	45.0
#define	NPC_MAX_HEAD_TWIST	35.0
#define	NPC_MAX_HEAD_TILT	60.0

///////////////////////////////////////////////////////////////////////////////
//	typedefs
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	globals
///////////////////////////////////////////////////////////////////////////////

//void	SIDEKICK_Update( userEntity_t *self );// SCG[1/23/00]: not used
//void	SIDEKICK_Avoid(userEntity_t *self);// SCG[1/23/00]: not used
//void	SIDEKICK_StartAttack(userEntity_t *self);// SCG[1/23/00]: not used

///////////////////////////////////////////////////////////////////////////////
//	hiro_SpawnScriptActor
//
//	creates a Hiro actor for the specified client
//
//	The client is made insubstantial with all parts set to RF_NODRAW and
//	FL_FREEZE set to disable input
//
//	A new NPC entity with a Hiro model is spawned at the passed origin and
//	given a goal of GOALTYPE_IDLE
//
//	FIXME:	duplicate real client's weapon model and copy over any structures
//			that need to be...
///////////////////////////////////////////////////////////////////////////////

void	hiro_SpawnScriptActor (userEntity_t *self, const CVector &origin, const CVector &angles)
{
	self->s.solid = SOLID_NOT;
//&&& amw 4.12.99 - no longer valid-	
//	hr_hide_parts (self);
	gstate->LinkEntity (self);

	self->flags |= FL_FREEZE|FL_BOT;


	userEntity_t *hiro = SIDEKICK_SpawnHiro( origin, angles );
	if ( !hiro )
	{
		com->Warning( "Failed to spawn Hiro.");
		return;
	}

	hiro->owner = self;

	playerHook_t *hiroHook = AI_GetPlayerHook( hiro );
	node_find_cur_node( hiro, hiroHook->pNodeList );

	hiro->think = AI_TaskThink;
	hiro->nextthink = gstate->time + 0.2f;

/*
	self->think = AI_TaskThink;
	self->nextthink = gstate->time + 0.2;
*/
}

void hiro_SpawnScriptDummy(const CVector &origin, const CVector &angles) 
{
	userEntity_t *hiro = SIDEKICK_SpawnHiro( origin, angles );
	if ( !hiro )
	{
		com->Warning( "Failed to spawn Hiro.");
		return;
	}

    //set the pitch and roll of hiro to zero.
    hiro->s.angles.x = 0.0f;
    hiro->s.angles.z = 0.0f;
    hiro->ideal_ang.x = 0.0f;
    hiro->ideal_ang.z = 0.0f;

	hiro->s.solid = SOLID_NOT;
	hiro->flags |= FL_FREEZE;

    playerHook_t *hiroHook = AI_GetPlayerHook( hiro );
    node_find_cur_node( hiro, hiroHook->pNodeList );

    hiro->think = NULL;
}

///////////////////////////////////////////////////////////////////////////////
//	hiro_RemoveScriptActor
//
///////////////////////////////////////////////////////////////////////////////

void	hiro_RemoveScriptActor (userEntity_t *actor)
{
	userEntity_t	*client;

	client = actor->owner;
	client->flags &= ~FL_FREEZE;

    client->s.origin = actor->s.origin;
	client->s.angles = actor->s.angles;
	client->ideal_ang = actor->s.angles;
	client->groundEntity = NULL;

//&&& amw 4.12.99 - no longer valid-
//	hr_show_parts (client);
	client->s.solid = SOLID_BBOX;
	gstate->LinkEntity (client);

//&&& amw 4.12.99 - no longer valid-
//    hr_remove_parts(actor, true);

	actor->remove (actor);
    
}

void hiro_RemoveScriptDummy(userEntity_t *actor) {
//&&& amw 4.12.99 - no longer valid-
//    hr_remove_parts(actor, true);

	actor->remove (actor);
}


#if 0
/*
///////////////////////////////////////////////////////////////////////////////
//	hiro_init
//	
//	initialization code for Hiro.  
///////////////////////////////////////////////////////////////////////////////

void hiro_init (userEntity_t *self)
{
	playerHook_t *hook = AI_InitMonster (self, TYPE_CLIENT);

//	self->remove (self);
//	return;

	self->className		= "Hiro";
	self->netname		= tongue_world[T_HIRO_MIYAMOTO];
    //name that is used to search for the entity during script operations.
    self->scriptname    = "hiro";

	hook->dflags = DFL_GOOD;
	self->flags &= ~FL_MONSTER;
	self->flags |= FL_BOT;
	//self->flags = FL_MONSTER;

	self->ang_speed.Set( 90, 720, 90 );

	self->s.modelindex = 0;
	gstate->SetSize( self, -16.0, -16.0, -24.0, 16.0, 16.0, 32.0 );

	gstate->LinkEntity (self);

	///////////////////////////////////////////////////////////////////////////
	// set up pointers to this creature's ai functions
	///////////////////////////////////////////////////////////////////////////

	hook->upward_vel	= 270.0;
	hook->run_speed		= 350.0;
	hook->walk_speed	= 63.0;
	hook->attack_speed	= 350.0;
	hook->max_jump_dist = ai_max_jump_dist( hook->run_speed, hook->upward_vel );

	hook->attack_dist	= 2048;

	self->health		= 100;
	hook->base_health	= 100;
	hook->pain_chance	= 10;
	self->mass			= 1.5;

	hook->follow_dist		= 256.0;
	hook->walk_follow_dist	= 128.0;

	hook->fnStartAttackFunc = SIDEKICK_StartAttack;
	hook->fnAttackFunc		= SIDEKICK_Attack;
	hook->fnFindTarget		= AI_FindNearestMonsterBot;

	self->prethink		= SIDEKICK_Update;

	///////////////////////////////////////////////////////////////////////////
	//
	///////////////////////////////////////////////////////////////////////////

	hr_init_models (self);

	///////////////////////////////////////////////////////////////////////////
	//	give hiro a weapon
	///////////////////////////////////////////////////////////////////////////
	
	hook->nAttackType = ATTACK_GROUND_RANGED;
	hook->nAttackMode = ATTACKMODE_NORMAL;
}

*/
#endif 0
