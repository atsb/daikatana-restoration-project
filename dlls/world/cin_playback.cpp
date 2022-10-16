#include "world.h"
#include "ai.h"
#include "ai_func.h"
#include "ai_frames.h"
#include "Sidekick.h"

#include "dk_cin_types.h"
#include "dk_cin_world_interface.h"
#include "dk_gce_entity_attributes.h"

#include "entity_pointer.h"
#include "ai_utils.h"

#include "ai_info.h"

cin_entity_type GetEntityType(const char *entity_name, const char *id);

void SetClientFOV( float fFOV );
void SetServerState(qboolean bCinematicPlaying, const char *pszCinematicName );
void QueueMoveTo(const char *entity_name, const char *id, const CVector &pos, const char *animation_name);
void QueueTurn(const char *entity_name, const char *id, const CVector &angles);
void QueueMoveAndTurn(const char *entity_name, const char *id, const CVector &pos, const CVector &angles);
void QueueWait(const char *entity_name, const char *id, float seconds);
void QueueStartRunning(const char *entity_name, const char *id);
void QueueStartWalking(const char *entity_name, const char *id, const char *animation_name );
void QueueAnimation(const char *entity_name, const char *id, const char *animation_name);
void QueueSetIdleAnimation(const char *entity_name, const char *id, const char *animation_name);
void QueueTriggerBrushUse(const char *entity_name, const char *id, const char *use_entity_name);
void QueuePlaySound(const char *entity_name, const char *id, const char *sound_name, const float sound_duration);

void AttributesBackup(const char *entity_name, const char *id);
void AttributesRestore(const char *entity_name, const char *id);
void AttributesTurnSpeed(const char *entity_name, const char *id, float yaw_speed);
void AttributesRunSpeed(const char *entity_name, const char *id, float run_speed);
void AttributesWalkSpeed(const char *entity_name, const char *id, float walk_speed);

void TriggerBrushUse(const char *entity_name, const char *id);

bool DoneWithTasks(const char *entity_name, const char *id);
bool EntityIndex(const char *entity_name, const char *id, int &pos);
bool GetEntityPosition(const char *entity_name, const char *id, CVector &pos, CVector &angles);
void TeleportEntity(const char *entity_name, const char *id, CVector &pos, CVector &angles, bool use_angles);
void SetEntityFacing(const char *entity_name, const char *id, CVector &angles);
bool EntityAttributes(const char *entity_name, const char *id, entity_attributes &attributes);
void SpawnHiroActor(int entity_num);
void RemoveHiroActor();
void SpawnHiroDummy(const CVector &origin, const CVector &angles);
void RemoveHiroDummy();
void CIN_RemoveAllEntities();

void SpawnCinEntity( const char *name, const char *id, const CVector& origin, const CVector& angles );
void RemoveCinEntity( const char *name, const char *id );

void ClearGoals( const char *name, const char *id );

void GetCOM() {
    if (gstate == NULL) return;
    memory_com = com = (common_export_t *)gstate->common_exports;
}

#define cin_header() if (gstate == NULL) return; if (com == NULL) {GetCOM(); if (com == NULL) return;}
#define cin_header_ret(val) if (gstate == NULL) return val; if (com == NULL) {GetCOM(); if (com == NULL) return val;}

void WorldCINInit() {
    if (gstate == NULL) return;

    cin_world_import_t import;

    import.GetEntityType			= GetEntityType;

	import.SetServerState			= SetServerState;

    import.QueueMoveTo				= QueueMoveTo;
    import.QueueTurn				= QueueTurn;
    import.QueueMoveAndTurn			= QueueMoveAndTurn;
    import.QueueWait				= QueueWait;
    import.QueueStartRunning		= QueueStartRunning;
    import.QueueStartWalking		= QueueStartWalking;
    import.QueueAnimation			= QueueAnimation;
    import.QueueSetIdleAnimation	= QueueSetIdleAnimation;
    import.QueueTriggerBrushUse		= QueueTriggerBrushUse;
    import.QueuePlaySound			= QueuePlaySound;

    import.AttributesBackup			= AttributesBackup;
    import.AttributesRestore		= AttributesRestore;
    import.AttributesTurnSpeed		= AttributesTurnSpeed;
    import.AttributesRunSpeed		= AttributesRunSpeed;
    import.AttributesWalkSpeed		= AttributesWalkSpeed;

    import.TriggerBrushUse			= TriggerBrushUse;

    import.DoneWithTasks			= DoneWithTasks;
    import.EntityIndex				= EntityIndex;
    import.GetEntityPosition		= GetEntityPosition;
    import.TeleportEntity			= TeleportEntity;
    import.SetEntityFacing			= SetEntityFacing;
    import.EntityAttributes			= EntityAttributes;

    import.SpawnHiroActor			= SpawnHiroActor;
    import.RemoveHiroActor			= RemoveHiroActor;

    import.SpawnHiroDummy			= SpawnHiroDummy;
    import.RemoveHiroDummy			= RemoveHiroDummy;

	import.SpawnEntity				= SpawnCinEntity;
	import.RemoveEntity				= RemoveCinEntity;
	import.RemoveAllEntities		= CIN_RemoveAllEntities;

	import.ClearGoals				= ClearGoals;

	import.SetClientFOV				= SetClientFOV;

    gstate->CIN_WorldPlayback(&import);
}

void SetClientFOV( float fFOV )
{
	gclient_t	*pClient;

	for( int i = 0; i < gstate->game->maxclients; i++ )
	{
		pClient = &gstate->game->clients[i];
		
		pClient->ps.fov = fFOV;
	}
}

userEntity_t *Client_GetSpawnPoint( char *name );

void SetServerState(qboolean bCinematicPlaying, const char *pszCinematicName )
{
	userEntity_t *pClient;

	if( bCinematicPlaying ) 
	{
		gstate->bCinematicPlaying = bCinematicPlaying;
		if( pszCinematicName != NULL )
		{
			strncpy( gstate->szCinematicName, pszCinematicName, 32 );
		}

	}
	else // SCG[2/4/00]: fix spawn angles after a cinematic is done playing
	{
		gstate->bCinematicPlaying = bCinematicPlaying;
		userEntity_t *pSpot = Client_GetSpawnPoint( "info_player" );
		if( ( coop->value == 0 ) && ( deathmatch->value == 0 ) && pSpot != NULL )
		{
			pClient = &gstate->g_edicts[1];
			pClient->client->fix_angles		= pSpot->s.angles;
			pClient->client->v_angle		= pSpot->s.angles;
			pClient->client->oldviewangles	= pSpot->s.angles;
			pClient->client->ps.viewangles	= pSpot->s.angles;
			pClient->s.angles				= pSpot->s.angles;
			pClient->flags |= FL_FIXANGLES;
		}
	}
}


void WorldCINKill() {
    if (gstate == NULL) return;

    //give the client a table of NULL pointers.
    cin_world_import_t import;
    memset(&import, 0, sizeof(cin_world_import_t));

    gstate->CIN_WorldPlayback(&import);
}


//finds the entity with the given name.
userEntity_t *FindEntity(const char *name)
{
    cin_header_ret(NULL);

    //if (com->FindScriptEntity == NULL) return NULL;  // function exists?
    if (com->FindEntity == NULL) return NULL;        // function exists?

    //look for the entity based on the script name.
    //return com->FindScriptEntity(name);
		
    return com->FindEntity(name);  // 8.12 dsn   changed to look for classname rather than 'scriptname'
}


//finds the entity with the given name, that is flagged as a bot, client, or monster.
userEntity_t *FindQueueEntity(const char *name) {
  userEntity_t *entity = FindEntity(name);
    if (entity == NULL) return NULL;

    //make sure it is the correct type.
    if ((entity->flags & (FL_CLIENT | FL_MONSTER | FL_BOT)) == 0) return NULL;

    return entity;
}

//finds the entity with the given name, that is flagged as a bot, client, or monster.
userEntity_t *FindQueueEntity(const char *name, const char *id) {
    userEntity_t *entity = UNIQUEID_Lookup( id );

    if (entity == NULL) entity = FindEntity(name);

    if (entity == NULL) return NULL;

    //make sure it is the correct type.
    if ((entity->flags & (FL_CLIENT | FL_MONSTER | FL_BOT)) == 0) return NULL;

    return entity;
}

cin_entity_type GetEntityType(userEntity_t *entity) {
    if (entity == NULL) return CET_NONE;

    //check the class name of the object first
    if (entity->className != NULL) {
        if (stricmp("trigger_once", entity->className) == 0 ||
            stricmp("trigger_changelevel", entity->className) == 0) 
        {
            //it's a trigger brush we can use.
            return CET_TRIGGER_BRUSH;
        }
    }

    //check the flags.
    if ((entity->flags & (FL_CLIENT | FL_MONSTER | FL_BOT)) != 0) {
        //this entity has a task queue.
        return CET_TASK_QUEUE;
    }

    //unknown type.
    return CET_NONE;
}

cin_entity_type GetEntityType(const char *entity_name, const char *id) {
    if (entity_name == NULL) return CET_NONE;

    //special case hiro, because sometimes there is no entity spawned with this scriptname, but
    //he is always available for the script.
//    if (stricmp(entity_name, "hiro") == 0) return CET_TASK_QUEUE;

    //get the entity.
	// check through FindEntity if it's specified through a model
	if( strstr( entity_name, ".dkm" ) )
	{
		userEntity_t *entity = FindEntity(entity_name);
		if (entity == NULL) return CET_NONE;

		return GetEntityType(entity);
	}
	// otherwise, it must be a spawned type
	else
	{
		return CET_TASK_QUEUE;
	}

}

void QueueMoveTo(const char *entity_name, const char *id, const CVector &pos, const char *animation_name ) {
    cin_header();
    if (entity_name == NULL) return;
    
    //get the entity with the given name.
    userEntity_t *ent = FindQueueEntity(entity_name, id);
    if (ent == NULL) return;

	if( *animation_name != NULL )
	{
		playerHook_t *pHook = ( playerHook_t * ) ent->userHook;
		frameData_t *pFrameData = FRAMES_GetSequence( ent, animation_name );
		if( pFrameData != NULL )
		{
			pHook->pMovingAnimation = pFrameData;
		}
	}

    //add a goal to the entity's stack.
    AI_AddNewScriptGoalAtBack(ent, GOALTYPE_MOVETOLOCATION, pos);
}

void QueueTurn(userEntity_t *ent, const char *id, const CVector &angles) {
    if (ent == NULL) return;
    if (com == NULL) return;
    if ((ent->flags & (FL_CLIENT | FL_MONSTER | FL_BOT)) == 0) return;

    CVector flat_angle;
    flat_angle.x = 0.0f;
    flat_angle.y = AngleMod(angles[YAW]);
    flat_angle.z = 0.0f;

    AI_AddNewScriptGoalAtBack(ent, GOALTYPE_FACEANGLE, flat_angle);
}

void QueueTurn(const char *entity_name, const char *id, const CVector &angles) {
    cin_header();
    if (entity_name == NULL) return;
    
    //get the entity with the given name.
    userEntity_t *ent = FindQueueEntity(entity_name, id);
    if (ent == NULL) return;

    QueueTurn(ent, id, angles);
}

void QueueMoveAndTurn(const char *entity_name, const char *id, const CVector &pos, const CVector &angles) {
    cin_header();
    if (entity_name == NULL) return;
    
    //get the entity with the given name.
    userEntity_t *ent = FindQueueEntity(entity_name, id);
    if (ent == NULL) return;

    //add a move goal to the entity's stack.
    AI_AddNewScriptGoalAtBack(ent, GOALTYPE_MOVETOLOCATION, pos);

    QueueTurn(ent, id, angles);
}

void QueueWait(const char *entity_name, const char *id, float seconds) {
    cin_header();
    if (entity_name == NULL) return;
    
    //get the entity.
    userEntity_t *ent = FindQueueEntity(entity_name, id);
    if (ent == NULL) return;

    //add the goal to the entity's stack.
    AI_AddNewScriptGoalAtBack(ent, GOALTYPE_WAIT, seconds);
}

void AttributesBackup(const char *entity_name, const char *id) {
    cin_header();
    if (entity_name == NULL) return;

    //get the entity.
    userEntity_t *ent = FindQueueEntity(entity_name, id);
    if (ent == NULL) return;

    //add the backup goal to the queue.
    AI_AddNewScriptGoalAtBack(ent, GOALTYPE_BACKUPALLATTRIBUTES);
}

void QueueStartRunning(const char *entity_name, const char *id) {
    cin_header();
    if (entity_name == NULL) return;

    //get the entity.
    userEntity_t *ent = FindQueueEntity(entity_name, id);
    if (ent == NULL) return;

    //add the start running goal to the queue.
    AI_AddNewScriptGoalAtBack(ent, GOALTYPE_STARTUSINGRUNSPEED);
}

void QueueStartWalking(const char *entity_name, const char *id, const char *animation_name ) {
    cin_header();
    if (entity_name == NULL) return;

    //get the entity.
    userEntity_t *ent = FindQueueEntity(entity_name, id);
    if (ent == NULL) return;

	if( *animation_name != NULL )
	{
		playerHook_t *pHook = ( playerHook_t * ) ent->userHook;
		frameData_t *pFrameData = FRAMES_GetSequence( ent, animation_name );
		if( pFrameData != NULL )
		{
			pHook->pMovingAnimation = pFrameData;
		}
	}

    //add the start walking goal to the queue.
    AI_AddNewScriptGoalAtBack(ent, GOALTYPE_STARTUSINGWALKSPEED);
}

void AttributesRestore(const char *entity_name, const char *id) {
    cin_header();
    if (entity_name == NULL) return;

    //get the entity.
    userEntity_t *ent = FindQueueEntity(entity_name, id);
    if (ent == NULL) return;

    //add the backup goal to the queue.
    AI_AddNewScriptGoalAtBack(ent, GOALTYPE_RESTOREALLATTRIBUTES);
}

void AttributesTurnSpeed(const char *entity_name, const char *id, float yaw_speed) {
    cin_header();
    if (entity_name == NULL) return;

    //get the entity.
    userEntity_t *ent = FindQueueEntity(entity_name, id);
    if (ent == NULL) return;

    //make a vector with the 3 new turn speeds.  Use the current pitch and roll speeds, replace the yaw speed.
    CVector new_turn_speed;
	new_turn_speed.x = ent->ang_speed.x;
	new_turn_speed.y = yaw_speed;
	new_turn_speed.z = ent->ang_speed.z;

    //add the backup goal to the queue.
    AI_AddNewScriptGoalAtBack(ent, GOALTYPE_MODIFYTURNATTRIBUTE, new_turn_speed);
}

void AttributesRunSpeed(const char *entity_name, const char *id, float run_speed) {
    cin_header();
    if (entity_name == NULL) return;

    //get the entity.
    userEntity_t *ent = FindQueueEntity(entity_name, id);
    if (ent == NULL) return;

    //add the backup goal to the queue.
    AI_AddNewScriptGoalAtBack(ent, GOALTYPE_MODIFYRUNATTRIBUTE, run_speed);
}

void AttributesWalkSpeed(const char *entity_name, const char *id, float walk_speed) {
    cin_header();
    if (entity_name == NULL) return;

    //get the entity.
    userEntity_t *ent = FindQueueEntity(entity_name, id);
    if (ent == NULL) return;

    //add the backup goal to the queue.
    AI_AddNewScriptGoalAtBack(ent, GOALTYPE_MODIFYWALKATTRIBUTE, walk_speed);
}

void TriggerBrushUse(const char *entity_name, const char *id) {
    cin_header();
    if (entity_name == NULL) return;

    //get the entity.
//  userEntity_t *entity = FindEntity(entity_name);
    userEntity_t *entity = UNIQUEID_Lookup( id );
    if (entity == NULL) entity = FindEntity(entity_name);
    if (entity == NULL) return;

    //get the type of the entity.
    cin_entity_type entity_type = GetEntityType(entity);
//    if (entity_type != CET_TRIGGER_BRUSH) return;

    //trigger the trigger brush.
    if (entity->use == NULL) return;
    entity->use(entity, NULL, NULL);
}

void QueueTriggerBrushUse(const char* user_entity_name, const char *id, const char *usee_entity_name) {
    cin_header();
    if (user_entity_name == NULL || usee_entity_name == NULL) return;

    //get the user entity.
//  userEntity_t *user_entity = FindEntity(user_entity_name);
    userEntity_t *user_entity = UNIQUEID_Lookup( id );
    if (user_entity == NULL) user_entity = FindEntity(user_entity_name);
    if (user_entity == NULL) return;

    //get the usee entity.
  //  userEntity_t *usee_entity = FindEntity(usee_entity_name);
    userEntity_t *usee_entity = UNIQUEID_Lookup(usee_entity_name);
    if (usee_entity == NULL) user_entity = FindEntity(usee_entity_name);
    if (usee_entity == NULL) return;

    //trigger the trigger brush.
    AI_AddNewScriptGoalAtBack(user_entity, GOALTYPE_USE, usee_entity);
}

bool DoneWithTasks(const char *entity_name, const char *id) {
    cin_header_ret(true);
    if (entity_name == NULL) return true;
    
    //get the entity with the given name.
//    userEntity_t *ent = FindEntity(entity_name);
    userEntity_t *ent = UNIQUEID_Lookup( id );
    if (ent == NULL) ent = FindEntity(entity_name);
    if (ent == NULL) return true;

    //check what type of entity we are dealing with.
    if ((ent->flags & (FL_CLIENT | FL_MONSTER | FL_BOT)) != 0) {
        if (ent->input_entity == NULL) return true;

        //get the entity's script goal stack.
        playerHook_t *hook = AI_GetPlayerHook( ent->input_entity );
        if (hook == NULL) return true;
        GOALSTACK_PTR goals = hook->pScriptGoals;
        if (goals == NULL) return true;

        //check how many goals the entity has.
        int num_goals = GOALSTACK_GetNumGoals(goals);
        if (num_goals < 1) return true;

        //if we have more than 1 goal, we are not done.
        if (num_goals > 1) return false;

        //we only have 1 goal, check if it is the idle goal.
        GOAL_PTR goal = GOALSTACK_GetCurrentGoal(goals);
        if (goal == NULL) return true;

        //check the type.
        if (GOAL_GetType(goal) == GOALTYPE_IDLE) return true;

        //we still have a non-idle goal.
        return false;
    }

    return true;
}

bool EntityIndex(const char *entity_name, const char *id, int &pos) {
    cin_header_ret(false);
    if (entity_name == NULL) false;
    
    //get the entity with the given name.
//    userEntity_t *ent = FindEntity(entity_name, id);
    userEntity_t *ent = UNIQUEID_Lookup( id );
    if (ent == NULL) ent = FindEntity(entity_name);
    if (ent == NULL) return false;

    //get the entity's number.
    pos = ent->s.number;

    return true;
}

bool GetEntityPosition(const char *entity_name, const char *id, CVector &pos, CVector &angles) {
    cin_header_ret(false);
    if (entity_name == NULL) false;
    
    //get the entity with the given name.
//    userEntity_t *ent = FindEntity(entity_name);
    userEntity_t *ent = UNIQUEID_Lookup( id );
    if (ent == NULL) ent = FindEntity(entity_name);
    if (ent == NULL) 
	{
		return false;
	}

    //get the entity's position.
    pos = ent->s.origin;
    angles = ent->s.angles;

    return true;
}

void TeleportEntity(const char *entity_name, const char *id, CVector &pos, CVector &angles, bool use_angles) {
    cin_header();
    if (entity_name == NULL) return;

    //find the entity with the given name.
//    userEntity_t *ent = FindEntity(entity_name);
    userEntity_t *ent = UNIQUEID_Lookup( id );
    if (ent == NULL) ent = FindEntity(entity_name);
    if (ent == NULL) 
		return;

	ent->flags |= FL_CINEMATIC;
    //set the entity's position.
    ent->s.origin = pos;
    ent->s.origin.z += 1.0;

    //set the angles.
    if (use_angles == true) 
	{
        ent->s.angles = angles;
        ent->ideal_ang = ent->s.angles;
    }

    //ent->groundEntity = NULL;
    gstate->LinkEntity (ent);

    //clear out the entity's script goal stack, if it has one.
    //make sure it is the correct type.
    if ((ent->flags & (FL_CLIENT | FL_MONSTER | FL_BOT)) == 0) return;

    //get the entity's script goal stack.
    if (ent->input_entity == NULL) return;
    playerHook_t *hook = AI_GetPlayerHook( ent->input_entity );
    if (hook == NULL) return;
    GOALSTACK_PTR goals = hook->pScriptGoals;
    if (goals == NULL) return;

    //clear out all the goals.
    GOALSTACK_ClearAllGoals(goals);
}

void SetEntityFacing(const char *entity_name, const char *id, CVector &angles) {
    cin_header();
    if (entity_name == NULL) return;

    //find the entity.
    userEntity_t *ent = FindQueueEntity(entity_name, id);
    if (ent == NULL) return;

    //set the entity's ideal angle.
    ent->ideal_ang = angles;
    ent->ideal_ang.x /= 3.0f;
}

bool EntityAttributes(const char *entity_name, const char *id, entity_attributes &attributes) {
    cin_header_ret(false);
    if (entity_name == NULL) return false;

    userEntity_t *ent = FindQueueEntity(entity_name, id);
/*
    if (ent == NULL) {
        //we need to special case hiro, because sometimes we need to get his attributes even
        //when he isnt there.
        if (stricmp(entity_name, "hiro") == 0) {
            //make up some attributes.
			attributes.walk_speed = SIDEKICK_WALKSPEED;
            attributes.run_speed = SIDEKICK_RUNSPEED;
            attributes.yaw_speed = 72;

            return true;
        }

        return false;
    }
*/

    //get his playerhook.
    if (ent->input_entity == NULL) return false;
    playerHook_t *hook = AI_GetPlayerHook( ent->input_entity );
    if (hook == NULL) return false;

    //get the attributes.
    attributes.run_speed = hook->run_speed;
    attributes.yaw_speed = ent->ang_speed[YAW];

    return true;
}

void SpawnHiroActor(int entity_num) {
    cin_header();
    if (gstate->game == NULL) return;
    if (gstate->g_edicts == NULL) return;
    if (entity_num < 0) return;

    //get the max number of entities.
    int max_entities = gstate->game->maxentities;
    if (entity_num >= max_entities) return;

    //get the client entity.
//    userEntity_t *client = &gstate->g_edicts[entity_num];
    
    //get the position of the client.
//    hiro_SpawnScriptActor(client, client->s.origin, client->s.angles);
    userEntity_t *pPlayer = FindEntity("player");
	if( pPlayer != NULL )
	{
		pPlayer->solid = SOLID_NOT;
		gstate->LinkEntity( pPlayer );
		pPlayer->flags |= FL_FREEZE;
		pPlayer->s.renderfx |= RF_NODRAW;
	}

	userEntity_t *pSuperFly = AIINFO_GetSuperfly();
	if( pSuperFly != NULL )
	{
		pSuperFly->solid = SOLID_NOT;
		gstate->LinkEntity( pSuperFly );
		pSuperFly->flags |= FL_FREEZE;
		pSuperFly->s.renderfx |= RF_NODRAW;
	}

	userEntity_t *pMikiko = AIINFO_GetMikiko();
	if( pMikiko != NULL )
	{
		pMikiko->solid = SOLID_NOT;
		gstate->LinkEntity( pMikiko );
		pMikiko->flags |= FL_FREEZE;
		pMikiko->s.renderfx |= RF_NODRAW;
	}

}

void RemoveHiroActor() {
    //get the actor.
//    userEntity_t *actor = FindEntity("hiro");
//    if (actor == NULL) return;

//    hiro_RemoveScriptActor(actor);
//    userEntity_t *client = &gstate->g_edicts[entity_num];
//	client->s.solid &= ~SOLID_NOT;
//	gstate->LinkEntity( client );
//	client->flags &= ~FL_FREEZE;
//	client->s.renderfx &= ~RF_NODRAW;
    userEntity_t *pPlayer = FindEntity("player");
	if( pPlayer != NULL )
	{
		pPlayer->solid = SOLID_BBOX;
		gstate->LinkEntity( pPlayer );
		pPlayer->flags &= ~FL_FREEZE;
		pPlayer->s.renderfx &= ~RF_NODRAW;
	}

	userEntity_t *pSuperFly = AIINFO_GetSuperfly();
	if( pSuperFly != NULL )
	{
		pSuperFly->solid = SOLID_BBOX;
		gstate->LinkEntity( pSuperFly );
		pSuperFly->flags &= ~FL_FREEZE;
		pSuperFly->s.renderfx &= ~RF_NODRAW;
	}

	userEntity_t *pMikiko = AIINFO_GetMikiko();
	if( pMikiko != NULL )
	{
		pMikiko->solid = SOLID_BBOX;
		gstate->LinkEntity( pMikiko );
		pMikiko->flags &= ~FL_FREEZE;
		pMikiko->s.renderfx &= ~RF_NODRAW;
	}
}

void SpawnHiroDummy(const CVector &origin, const CVector &angles) {
/*
    hiro_SpawnScriptDummy(origin, angles);
*/
}

void RemoveHiroDummy() {
/*
    //get the actor.
    userEntity_t *actor = FindEntity("hiro");
    if (actor == NULL) return;

    hiro_RemoveScriptDummy(actor);
*/
}

void QueueAnimation(const char *entity_name, const char *id, const char *animation_name) {
    cin_header();
    if (entity_name == NULL) return;
    if (animation_name == NULL) return;
    
    //get the entity with the given name.
    userEntity_t *ent = FindQueueEntity(entity_name, id);
    if (ent == NULL) return;

    //information needed to locate the desired animation.

/*
    if (AI_GetAnimationType(ent, animation_name, &animation_type, &animation_number) == false) {
        //couldn't find the data for this animation name.
        gstate->Con_Printf("Bad script animation specified for %s: %s.\n", entity_name, animation_name); 
        return;
    }

    //get the animation frame information.
    frameData_t *data = AI_DataForSequenceType(ent, animation_type, animation_number);
    if (data == NULL) return;
*/

    frameData_t *data = FRAMES_GetSequence( ent, animation_name );
	if ( data == NULL )
	{
		return;
	}

	AIDATA aiData;
    aiData.pAnimSequence = data;

    //make a new goal.
    GOAL_PTR animation_goal = AI_AddNewScriptGoalAtBack(ent, GOALTYPE_PLAYANIMATION, &aiData);
    if (animation_goal == NULL) return;

    // ISP [9-7-99] Does not need this anymore if ever???

//    //compute the length of time the animation will take.
//    //get the number of frames. then divide by the fps.
//    float seconds = data->last - data->first;
//    if ((data->flags & FRAME_FPS10) != 0) {
//        //10 fps
//        seconds *= 0.1f;
//    }
//    else if ((data->flags & FRAME_FPS30) != 0) {
//        //30 fps
//        seconds *= 0.0333f;
//    }
//    else {
//        //20 fps
//        seconds *= 0.05f;
//    }
//
//    //subtract off the time that is eaten up by the set animation goal itself.
//    seconds -= 0.1f;
//
//    //add a wait goal
//    QueueWait(entity_name, seconds);
}

void QueueSetIdleAnimation(const char *entity_name, const char *id, const char *animation_name) {
    cin_header();
    if (entity_name == NULL) return;

    //get the entity with the given name.
    userEntity_t *ent = FindQueueEntity(entity_name, id);
    if (ent == NULL) return;

/*
    //information needed to locate the desired animation.
    int animation_type;
    int animation_number;

    if (AI_GetAnimationType(ent, animation_name, &animation_type, &animation_number) == false) {
        //couldn't find the data for this animation name.
        gstate->Con_Printf("Bad script animation specified for %s: %s.\n", entity_name, animation_name); 
        return;
    }

    //get the animation frame information.
    frameData_t *data = AI_DataForSequenceType(ent, animation_type, animation_number);
    if (data == NULL) return;
*/

	frameData_t *data = FRAMES_GetSequence( ent, animation_name );
    //make the goal ai data.
    AIDATA goal_data;
    goal_data.pAnimSequence = data;

    //make a new goal.
    AI_AddNewScriptGoalAtBack(ent, GOALTYPE_SETIDLEANIMATION, &goal_data);
}

void QueuePlaySound(const char *entity_name, const char *id, const char *sound_name, const float sound_duration) {
    cin_header();
    if (entity_name == NULL) return;

    //get the entity with the given name.
    userEntity_t *ent = FindQueueEntity(entity_name, id);
    if (ent == NULL) return;

    //make the goal ai data.
    AIDATA goal_data;
    goal_data.pString = (char*)sound_name;
    goal_data.fValue = sound_duration;

    //make a new goal.
    AI_AddNewScriptGoalAtBack(ent, GOALTYPE_PLAYSOUNDTOEND, &goal_data);
}

void SPAWN_Remove( const char *szUniqueId );
void SPAWN_AI( userEntity_t *self, const char *szClassName, const char *szUniqueId, const CVector &spawnPoint, 
			   const CVector &facingAngle, int bRespawnAfterDeath, char *szScriptAction, char *szSkin );

void SpawnCinEntity( const char *name, const char *id, const CVector& origin, const CVector& angles )
{
	userEntity_t *ent = UNIQUEID_Lookup( id );

	if( ent != NULL )
	{
		int cinematic_entity_already_exists = 0;
		_ASSERTE( cinematic_entity_already_exists );
		return;
	}

	if( id[0] == NULL )
	{
		int entity_does_not_have_unique_id = 0;
		_ASSERTE( entity_does_not_have_unique_id );
	}

	SPAWN_AI( NULL, name, id, origin, angles, 0, NULL, NULL );

	cin_header();

	if (name == NULL) return;

	//find the entity with the given name.
	ent = UNIQUEID_Lookup( id );
	if (ent == NULL) 
		return;

	ent->flags |= FL_CINEMATIC;
	ent->groundEntity = NULL;
	gstate->LinkEntity (ent);

	if ((ent->flags & (FL_CLIENT | FL_MONSTER | FL_BOT)) == 0) return;

	//get the entity's script goal stack.
	if (ent->input_entity == NULL) return;
	playerHook_t *hook = AI_GetPlayerHook( ent->input_entity );
	if (hook == NULL) return;
	GOALSTACK_PTR goals = hook->pScriptGoals;
	if (goals == NULL) return;

	//clear out all the goals.
	GOALSTACK_ClearAllGoals(goals);
//	hook->szScriptName = ( char * ) gstate->X_Malloc( strlen( id + 1 ), MEM_TAG_AI );
	hook->szScriptName = ( char * ) strdup( id );
	if( ( hook->szScriptName != NULL )  )
	{
		strcpy( hook->szScriptName, id );
	}
}

// SCG[10/1/99]: Removes by classname
void alist_remove( userEntity_t *self );
void CIN_RemoveAllEntities()
{
	userEntity_t *pPlayer;
	userEntity_t *pEntity;

	for( pEntity = gstate->FirstEntity (); pEntity; pEntity= gstate->NextEntity( pEntity ) )
	{
		if( pEntity != NULL )
		{
			if( pEntity->flags & FL_CINEMATIC )
			{
				if ( AI_IsAlive( pEntity ) )
				{
    				playerHook_t *hook = AI_GetPlayerHook( pEntity );
					
					pEntity->pain		= NULL;
					pEntity->use		= NULL;
					pEntity->die		= NULL;
					pEntity->think		= NULL;
					pEntity->prethink	= NULL;
					pEntity->postthink  = NULL;

					if ( hook->pGoals )
					{
						GOALSTACK_Delete( hook->pGoals );
						hook->pGoals = NULL;
					}
					if ( hook->pScriptGoals )
					{
						GOALSTACK_Delete( hook->pScriptGoals );
						hook->pScriptGoals = NULL;
					}
					
					if( hook->szScriptName )
					{
						UNIQUEID_Remove( hook->szScriptName );
	//					gstate->X_Free( hook->szScriptName );
						free( hook->szScriptName );
						hook->szScriptName = NULL;
					}

					alist_remove( pEntity );
					pEntity->remove( pEntity );
				}
			}
		}
	}

    pPlayer = FindEntity( "player" );

	for( pEntity = gstate->FirstEntity (); pEntity; pEntity= gstate->NextEntity( pEntity ) )
	{
		for( int i = 0; ( pEntity->epair != NULL ) && ( pEntity->epair[i].key != NULL ); i++ )
		{
			// NSS[12/10/99]:It was not checking the epairs right... so I rewrote things.
			userEpair_t	*epair = &pEntity->epair[i];
			if( epair != NULL)
			{
				if( strcmp( epair->key, "cinetrigger" ) == 0 )
				{
					if( strcmp( epair->value, gstate->szCinematicName ) == 0 )
					{
						if( pEntity->use )
						{
							pEntity->use( pEntity, pPlayer, pPlayer );
						}
					}
				}
			
				if( strcmp( epair->key, "cinekill" ) == 0 )
				{
					if( strcmp( epair->value, gstate->szCinematicName ) == 0 )
					{
						gstate->RemoveEntity( pEntity );
					}
				}
			}
		}
	}
}

// SCG[10/1/99]: Removes by uniqueid
void RemoveCinEntity( const char *name, const char *id )
{
	SPAWN_Remove( id );
}

void ClearGoals( const char *name, const char *id )
{
    userEntity_t *ent = FindQueueEntity(name, id);
	if( ent == NULL ) return;

    playerHook_t *hook = AI_GetPlayerHook( ent->input_entity );
	if( hook == NULL ) return;

    GOALSTACK_PTR goals = hook->pScriptGoals;
    if (goals == NULL) return;

    GOALSTACK_ClearAllGoals( goals );
}
