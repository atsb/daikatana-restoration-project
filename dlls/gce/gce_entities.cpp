#include "p_user.h"
#include "keys.h"
#include <ctype.h>

#include "dk_cin_types.h"
#include "dk_gce_interface.h"
#include "gce_main.h"
#include "gce_script.h"
#include "gce_hud.h"
#include "gce_entities.h"
#include "gce_position.h"
#include "gce_headscript.h"
#include "dk_point.h"

#include "dk_gce_script.h"
#include "dk_gce_entities.h"
#include "dk_gce_entity_attributes.h"

#define entities_header() if (cin == NULL || server == NULL) {return;}

//the array of entities we are editing.
array<CScriptEntity> entities;

//true if we are getting the position with the position hud mode feature.
static bool getting_position = false;
//true if we are recording a headscript
static bool getting_headscript = false;
//when we are getting the position, we use these vars to keep track of the task we will be modifying
//in our entities array.
static int create_task_entity_num;
static int create_task_task_num;
static task_type create_task_type;

//alot of the commands take as parameters an entity and a task number.  CheckEntityTaskNumber() 
//fills in these vars with the values found.
static int task_entity_num;
static int task_task_num;

//returns our array of sounds.
const array<CScriptEntity> &EntitiesCurrent() {
    return entities;
}

//copies the given array so we can edit it.
void EntitiesEdit(const array<CScriptEntity> &entities) {
    //copy the given array.
    ::entities = entities;
}

void EntitiesSave() {
    entities_header();

    //send the entities back to the script module.
    ScriptEntitiesModified(entities);
}

bool CheckEntityTaskNumber() {
    //check our arguments.
    if (server->GetArgc() < 3) return false;
    if (!isdigit(server->GetArgv(1)[0])) return false;
    if (!isdigit(server->GetArgv(2)[0])) return false;

    //get the entity number
    task_entity_num = atoi(server->GetArgv(1));
    
    //get the task number
    task_task_num = atoi(server->GetArgv(2));

    //get the entity.
    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return false;

    //get the task
    CScriptEntityTask *task = entity->GetTasks().Item(task_task_num);
    if (task == NULL) return false;

    return true;
}

CScriptEntityTask *GetTaskFromCommand() {
    if (CheckEntityTaskNumber() == false) return NULL;

    //get the entity.
    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return NULL;

    //get the task
    CScriptEntityTask *task = entity->GetTasks().Item(task_task_num);
    return task;
}

void EntitiesAdd() {
    entities_header();

    //get the arguments
    if (server->GetArgc() != 2) return;
    const char *name = server->GetArgv(1);
    if (name[0] == '\0' || name[0] == ' ') return;

	// verify that the entity is present
//	if ((!name)||(CET_NONE==cin->GetEntityType(name)))
//		return;

	//make a new entity.
    CScriptEntity *entity = new CScriptEntity();

    //set its name.
    entity->Name(name);

    //put it into the array.
    entities.Add(entity);

    //sort the array.
    entities.Sort(CScriptEntity::Compare);
}

void EntitiesDelete() {
    entities_header();

    //get our argument.
    if (server->GetArgc() != 2) return;
    const char *arg = server->GetArgv(1);
    if (!isdigit(arg[0])) return;

    //get the index we want to delete.
    int index = atoi(arg);

    //remove the item.
    entities.Remove(index);
}

void EntitiesAddTask() {
    entities_header();

    //get our argument.
    if (server->GetArgc() != 2) return;
    const char *arg = server->GetArgv(1);
    if (!isdigit(arg[0])) return;

    //get the index we want to add a task to
    int index = atoi(arg);

    //get the entity
    CScriptEntity *entity = entities.Item(index);
    if (entity == NULL) return;

    //make a new task.
    CScriptEntityTask *task = new CScriptEntityTask();

    //add the task to the entity.
    entity->Add(task);

    //init the use_entity_name to the caller
    task->UseEntityName(entity->Name());
}

void EntitiesTaskTime() {
    entities_header();

    //get the task.
    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;

    //get the time.
    if (server->GetArgc() != 4) return;
    float time = atof(server->GetArgv(3));

    //set the time
    task->Time(time);
}

void EntitiesTaskInsert() {
    entities_header();
    
    //get the entity and task indexes.
    if (CheckEntityTaskNumber() == false) return;

    //get the entity.
    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;

    //get the array of tasks.
    task_list &tasks = entity->GetTasks();

    //make a new task.
    CScriptEntityTask *task = new CScriptEntityTask();

    //add the task at the given index.
    tasks.Add(task, task_task_num);
}

void EntitiesTaskDelete() {
    entities_header();
    
    //get the entity and task indexes.
    if (CheckEntityTaskNumber() == false) return;

    //get the entity.
    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;

    //get the array of tasks.
    task_list &tasks = entity->GetTasks();

    //delete the task from the entity.
    tasks.Remove(task_task_num);
}

void EntitiesAddEntityPath(int selected_entity) {
    entities_header();

    entity_t seg_entity;
    task_type type;
    bool fHaveStartPos;

    //get the entity.
    CScriptEntity *entity = entities.Item(selected_entity);
    if (entity == NULL) return;

    int i;

    //get the array of tasks.
    task_list &tasks = entity->GetTasks();

    // draw all the paths
    CVector startPos, endPos;

    i=0;
    while (i<tasks.Num())
    {
        type = tasks.Item(i)->Type();

        switch (type)
        {
            case TT_TELEPORT:
            
            fHaveStartPos = true;
            startPos = tasks.Item(i)->Destination();
            
            break;

            case TT_MOVE_TO:
            case TT_MOVE_AND_TURN:
            
            if (fHaveStartPos)
            {
                endPos = tasks.Item(i)->Destination();

                seg_entity.oldorigin = startPos;
                seg_entity.origin = endPos;
                seg_entity.flags = RF_TRANSLUCENT | RF_BEAM;
                seg_entity.frame = 2;
                seg_entity.alpha = 1.0f;
        
                cin->beam_AddGCESplines (&seg_entity, 1, -1);

                startPos = endPos;
            }
            
            break;
        }

        i++;
    }
}

void EntitiesTaskMakeMoveTo() {
    entities_header();

    if (CheckEntityTaskNumber() == false) return;

    //remember what task type we will create
    create_task_type = TT_MOVE_TO;
    create_task_entity_num = task_entity_num;
    create_task_task_num = task_task_num;
    
    //start the get-position proceedure.
    getting_position = true;
    PositionPrompt("Move to entity desintation.");
    PositionSaveCommand("gce_entities_task_make_save\n");
    PositionAbortCommand("gce_entities_task_make_abort\n");

    //change hud modes.
    HUDModePosition();

    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;
    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;
	task->UniqueID( entity->UniqueID() );
}

void EntitiesTaskMakeTeleport() {
    entities_header();

    if (CheckEntityTaskNumber() == false) return;

    //remember what task type we will create
    create_task_type = TT_TELEPORT;
    create_task_entity_num = task_entity_num;
    create_task_task_num = task_task_num;
    
    //start the get-position proceedure.
    getting_position = true;
    PositionPrompt("Move to entity desintation and angles.");
    PositionSaveCommand("gce_entities_task_make_save\n");
    PositionAbortCommand("gce_entities_task_make_abort\n");

    //change hud modes.
    HUDModePosition();

    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;
    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;
	task->UniqueID( entity->UniqueID() );
}

void EntitiesTaskMakeHeadScript() {
    entities_header();

    if (CheckEntityTaskNumber() == false) return;

    //remember what task type we will create
    create_task_type = TT_HEAD_SCRIPT;
    create_task_entity_num = task_entity_num;
    create_task_task_num = task_task_num;
    
    //start the head script recording proceedure.
    getting_headscript = true;

    //change hud modes.
    HUDModeHeadScript();

    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;
    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;
	task->UniqueID( entity->UniqueID() );
}

void EntitiesTaskMakePlayAnimation() {
    entities_header();

    //get the task
    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;

    //change the task type.
    task->PlayAnimation("amba");

    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;
	task->UniqueID( entity->UniqueID() );
}

void EntitiesTaskMakeSetIdleAnimation() {
    entities_header();

    //get the task
    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;

    //change the task type.
    task->SetIdleAnimation("amba");

    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;
	task->UniqueID( entity->UniqueID() );
}

void EntitiesTaskMakePlaySound() {
    entities_header();

    //get the task
    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;

    //change the task type.
    task->PlaySound("", 0.0f);

    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;
	task->UniqueID( entity->UniqueID() );
}

void EntitiesTaskMakeTurn() {
    entities_header();

    if (CheckEntityTaskNumber() == false) return;

    //remember what task type we will create.
    create_task_type = TT_TURN;
    create_task_entity_num = task_entity_num;
    create_task_task_num = task_task_num;
    
    //start the get-position proceedure.
    getting_position = true;
    PositionPrompt("Face the desired angles.");
    PositionSaveCommand("gce_entities_task_make_save\n");
    PositionAbortCommand("gce_entities_task_make_abort\n");

    //change hud modes.
    HUDModePosition();
}

void EntitiesTaskMakeMoveAndTurn() {
    entities_header();

    if (CheckEntityTaskNumber() == false) return;

    //remember what task type we will create.
    create_task_type = TT_MOVE_AND_TURN;
    create_task_entity_num = task_entity_num;
    create_task_task_num = task_task_num;
    
    //start the get-position proceedure.
    getting_position = true;
    PositionPrompt("Move and look in desired position and angles.");
    PositionSaveCommand("gce_entities_task_make_save\n");
    PositionAbortCommand("gce_entities_task_make_abort\n");

    //change hud modes.
    HUDModePosition();

    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;
    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;
	task->UniqueID( entity->UniqueID() );
}

void EntitiesTaskMakeWait() {
    entities_header();

    //get the task
    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;

    //change the task type.
    task->Wait(1.0f);

    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;
	task->UniqueID( entity->UniqueID() );
}

void EntitiesTaskMakeAttributeBackup() {
    entities_header();

    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;

    //change the task type.
    task->AttributeBackup();

    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;
	task->UniqueID( entity->UniqueID() );
}

void EntitiesTaskMakeAttributeRestore() {
    entities_header();

    //get the task
    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;

    //change the task type.
    task->AttributeRestore();

    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;
	task->UniqueID( entity->UniqueID() );
}

void EntitiesTaskMakeAttributeYawSpeed() {
    entities_header();

    //get the task
    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;

    //get the entity.
    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;

    //get the current value of the entity's run speed.
    entity_attributes attributes;
    if (cin->EntityAttributes(entity->Name(), entity->UniqueID(), attributes) == false) return;

    //set the task type.
    task->AttributeYawSpeed(attributes.yaw_speed);
	task->UniqueID( entity->UniqueID() );
}

void EntitiesTaskMakeAttributeRunSpeed() {
    entities_header();

    //get the task
    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;

    //get the entity.
    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;

    //get the current value of the entity's run speed.
    entity_attributes attributes;
    if (cin->EntityAttributes(entity->Name(), entity->UniqueID(), attributes) == false) return;

    //set the task type.
    task->AttributeRunSpeed(attributes.run_speed);
	task->UniqueID( entity->UniqueID() );
}

void EntitiesTaskMakeAttributeWalkSpeed() {
    entities_header();

    //get the task
    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;

    //get the entity.
    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;

    //get the current value of the entity's walk speed.
    entity_attributes attributes;
    if (cin->EntityAttributes(entity->Name(), entity->UniqueID(), attributes) == false) return;

    //set the task type.
    task->AttributeWalkSpeed(attributes.walk_speed);
	task->UniqueID( entity->UniqueID() );
}

void EntitiesTaskMakeStartRunning() {
    entities_header();

    //get the task
    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;

    //get the entity.
    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;

    //set the task type.
    task->StartRunning();
	task->UniqueID( entity->UniqueID() );
}

void EntitiesTaskMakeStartWalking() {
    entities_header();

    //get the task
    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;

    //get the entity.
    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;

    //set the task type.
    task->StartWalking();
	task->UniqueID( entity->UniqueID() );
}

void EntitiesSetEntityID()
{
    entities_header();

	int entity_num = atoi( server->GetArgv(1) );
    //get the entity.
    CScriptEntity *entity = entities.Item(entity_num);
    if (entity == NULL) return;

	char *id = server->GetArgv(3);
	_ASSERTE( id != NULL );
	entity->UniqueID( id );

	CScriptEntityTask *task;

    for( int i = 0; 1; i++ )
	{
		task = entity->GetTasks().Item(i);
	    if (task == NULL) break;
		task->UniqueID( id );
	}
}

void EntitiesSetEntityName()
{
    entities_header();

	int entity_num = atoi( server->GetArgv(1) );
    //get the entity.
    CScriptEntity *entity = entities.Item(entity_num);
    if (entity == NULL) return;

	char *name = server->GetArgv(3);

	entity->Name( name );
}

void EntitiesTaskAttribute() {
    entities_header();

    //get the task
    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;

    //get the entity.
    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;

    //check the type of task.
    switch (task->Type()) {
		case TT_MOVE_TO:
        case TT_PLAY_ANIMATION:
        case TT_SET_IDLE_ANIMATION: {
            //set the animation name for the task.
            task->AnimationName(server->GetArgv(3));
            break;
        }
		case TT_PLAY_SOUND: {
			// set the sound name for the task
            task->SoundName(server->GetArgv(3));
			task->SoundDuration(atof(server->GetArgv(4)));
			break;
		}
        case TT_TRIGGER_BRUSH_USE: {

            const char *use_entity_name = server->GetArgv(3);

            // verify that the entity is present
            if (use_entity_name)
                task->UseEntityName(use_entity_name);

            break;
        }
        default: {
            //for all other task types, interpret the argument as a floating point number.
            float attribute_value = atof(server->GetArgv(3));

            //set the attribute for the task.
            task->AttributeValue(attribute_value);
        }
    }
	task->UniqueID( entity->UniqueID() );
}

void EntitiesTaskTriggerBrushUse() {
    entities_header();

    //get the task
    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;

    //get the entity.
    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;

    //set the task type
    task->TriggerBrushUse();
	task->UniqueID( entity->UniqueID() );
}

void EntitiesTaskPositionSave() {
    entities_header();

    //change back to entity mode
    HUDModeEntities();

    if (getting_position == false) return;

    //get the position and angles from the position module
    CVector position = PositionLoc();

    //subtract off the 22 unit offset added to the player position to get the camera position.
    position.z -= 22;

    CVector angles = PositionAngles();

    //get the entity we are modifying.
    CScriptEntity *entity = entities.Item(create_task_entity_num);
    if (entity == NULL) return;

    //get the task
    CScriptEntityTask *task = entity->GetTasks().Item(create_task_task_num);
    if (task == NULL) return;

    //set the task type.
    switch (create_task_type) {
        case TT_MOVE_TO:
            task->MoveTo(position);
            break;
        case TT_TURN:
            task->Turn(angles);
            break;
        case TT_MOVE_AND_TURN:
            task->MoveAndTurn(position, angles);
            break;
        case TT_TELEPORT:
            task->Teleport(position, angles);
            break;
        default:
            break;
    }
}

void EntitiesTaskPositionAbort() {
    entities_header();
    if (getting_position == false) return;

    //change back to entity mode
    HUDModeEntities();
}

void EntitiesTaskHeadScriptSave() {
    entities_header();
    if (getting_headscript == false) return;

    //change back to entity mode
    HUDModeEntities();

    //reset our flag.
    getting_headscript = false;

    //get the entity we are modifying.
    CScriptEntity *entity = entities.Item(create_task_entity_num);
    if (entity == NULL) return;

    //get the task
    CScriptEntityTask *task = entity->GetTasks().Item(create_task_task_num);
    if (task == NULL) return;

    //get the head script that was recorded.
    const CEntityHeadScript *headscript = &HeadscriptRecorded();
    
    //set the task type.
    task->HeadScript(headscript);
}

void EntitiesTaskHeadScriptAbort() {
    entities_header();
    if (getting_headscript == false) return;

    //change back to entity mode
    HUDModeEntities();

    //reset our flag.
    getting_headscript = false;
}

void EntitiesMakeSpawnEntity()
{
    entities_header();

    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;

    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;

	char *id = server->GetArgv(3);
	_ASSERTE( id != NULL );

	entity->UniqueID( id );
	CVector dest, dir;
	cin->LastCameraPos( dest, dir );
	task->SpawnEntity( id, dest, dir );
}

void EntitiesMakeRemoveEntity()
{
    entities_header();

    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;

    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;

	char *id = server->GetArgv(3);
	_ASSERTE( id != NULL );
	entity->UniqueID( id );
	task->RemoveEntity( entity->UniqueID() );
}

CScriptShot *SelectedShot();

void EntitiesMakeClearGoals()
{
    entities_header();

    CScriptEntityTask *task = GetTaskFromCommand();
    if (task == NULL) return;

    CScriptEntity *entity = entities.Item(task_entity_num);
    if (entity == NULL) return;

	char *id = server->GetArgv(3);
	_ASSERTE( id != NULL );
	entity->UniqueID( id );

	CScriptShot *shot = SelectedShot();

	float total_time = shot->TotalTime();

	task->ClearGoals( entity->UniqueID(), total_time );
}

