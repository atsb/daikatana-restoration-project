#include "p_user.h"

#include "dk_cin_types.h"
#include "dk_gce_interface.h"

#include "dk_array.h"
#include "dk_buffer.h"
#include "dk_point.h"

#include "dk_cin_types.h"
#include "dk_gce_entities.h"

#include "gce_main.h"
#include "gce_fields.h"
#include "gce_entities.h"
#include "gce_script.h"
#include "gce_hud.h"

typedef enum {
    FE_SAVE,
    FE_ADD,
    FE_DELETE,

    FE_ENTITY_SCROLL_UP,
    FE_ENTITY_SCROLL_DOWN,

    FE_TASK_ADD,
    FE_TASK_INSERT,
    FE_TASK_DELETE,

    FE_TASK_SCROLL_UP,
    FE_TASK_SCROLL_DOWN,

    FE_TASK_ASAP,
    FE_TASK_TIME,

    FE_TASK_ATTRIBUTE,
    FE_ANIMATION_NAME,
    FE_SOUND_NAME,
    FE_SOUND_DURATION,


    //tasks available to entities with a task queue.
    FE_TASK_MAKE_MOVETO,
    FE_TASK_MAKE_TURN,
    FE_TASK_MAKE_MOVEANDTURN,
    FE_TASK_MAKE_WAIT,
    FE_TASK_MAKE_ATTRIBUTE_BACKUP,
    FE_TASK_MAKE_ATTRIBUTE_RESTORE,
    FE_TASK_MAKE_ATTRIBUTE_RUNSPEED,
    FE_TASK_MAKE_ATTRIBUTE_WALKSPEED,
    FE_TASK_MAKE_ATTRIBUTE_YAWSPEED,
    FE_TASK_MAKE_TELEPORT,
    FE_TASK_MAKE_HEADSCRIPT,
    FE_TASK_MAKE_START_RUNNING,
    FE_TASK_MAKE_START_WALKING,
    FE_TASK_MAKE_PLAY_ANIMATION,
    FE_TASK_MAKE_SET_IDLE_ANIMATION,
    FE_TASK_MAKE_PLAY_SOUND,

    //tasks available for trigger brush entities and entities with a task queue.
    FE_TASK_MAKE_TRIGGER_BRUSH_USE,
    FE_USE_ENTITY_NAME,

	FE_TASK_MAKE_SPAWN_ENTITY,
	FE_TASK_MAKE_REMOVE_ENTITY,

	FE_TASK_SPAWN_ENTITY_NAME,

	FE_TASK_SPAWN_ENTITY_CLASSNAME,

	FE_TASK_MAKE_CLEARGOALS,

    FE_NUM_FIELDS,
} fields_entities;

int num_fields_entities = FE_NUM_FIELDS;

field_description field_desc_entities[] = {
    {FT_BUTTON, 14, "Save Entities"},
    {FT_STRING, 25, "Add Entity: "},
    {FT_BUTTON, 14, "Delete Entity"},

    {FT_BUTTON, 2, "^"},
    {FT_BUTTON, 2, "V"},

    {FT_BUTTON, 10, "Add Task"},
    {FT_BUTTON, 10, "Insert"},
    {FT_BUTTON, 10, "Delete"},

    {FT_BUTTON, 2, "^"},
    {FT_BUTTON, 2, "V"},

    {FT_BUTTON, 4, "ASAP"},
    {FT_FLOAT, 6, "Time: "},

    {FT_FLOAT, 6, "Attribute: "},
    {FT_STRING, 15, "Attribute: "},
    {FT_STRING, 15, "Attribute: "},
    {FT_FLOAT, 6, "Attribute: "},

    {FT_BUTTON, 14, "Move To"},
    {FT_BUTTON, 14, "Turn"},
    {FT_BUTTON, 14, "Move and Turn"},
    {FT_BUTTON, 14, "Wait"},
    {FT_BUTTON, 14, "Attr. Backup"},
    {FT_BUTTON, 14, "Attr. Restore"},
    {FT_BUTTON, 14, "Set Run Speed"},
    {FT_BUTTON, 14, "Set Walk Speed"},
    {FT_BUTTON, 14, "Set Yaw Speed"},
    {FT_BUTTON, 14, "Teleport"},
    {FT_BUTTON, 14, "Head Script"},
    {FT_BUTTON, 14, "Start Running"},
    {FT_BUTTON, 14, "Start Walking"},
    {FT_BUTTON, 14, "Play Animation"},
    {FT_BUTTON, 14, "Idle Animation"},
    {FT_BUTTON, 14, "Play Sound"},

    {FT_BUTTON, 14, "Use"},
    {FT_STRING, 14, "Attribute: "},

    {FT_BUTTON, 14, "Spawn Entity"},
    {FT_BUTTON, 14, "Remove Entity"},

    {FT_STRING, 16, "Entity ID:"},
    {FT_STRING, 25, "Entity Classname: "},

    {FT_BUTTON, 14, "Clear Goals"},
};

//when we are in entity mode, we use this info to keep track of what is visible.
static int entities_num_visible_lines = 0;
static int entities_top_index = 0;
static int entities_selected_row = -1;

//if we have an entity selected, we used these vars to display a table of tasks.
static int tasks_num_visible_lines = 0;
static int tasks_top_index = 0;
static int tasks_selected_row = -1;

int HUD_GetSelectedEntity()
{
    int selected_entity = -1;

    if (-1!=entities_selected_row)
        selected_entity = entities_selected_row + entities_top_index;

    return selected_entity;
}

bool field_mask_entities(int field) {
    if (cin == NULL) return false;

    if (field == FE_SAVE) return true;
    if (field == FE_ADD) return true;

    //get the array of entities we are editing.
    const array<CScriptEntity> &entities = EntitiesCurrent();

    //check the scroll buttons for the entity list.
    if (field == FE_ENTITY_SCROLL_DOWN) {
        if (entities.Num() - entities_top_index > 10) return true;
    }
    if (field == FE_ENTITY_SCROLL_UP) {
        if (entities_top_index > 0) return true;
    }

    if (entities_selected_row != -1) {
        //get the entity we have selected.
        const CScriptEntity *entity = entities.Item(entities_selected_row + entities_top_index);
        if (entity == NULL) return false;

        //get the entity name.
        const char *entity_name = entity->Name();
        if (entity_name == NULL) return false;

        if (field == FE_DELETE) return true;

        //get the entity array.
        if (field == FE_TASK_ADD) return true;

        //get the entity's task list.
        const task_list &tasks = entity->Tasks();

        //check the task list scroll buttons.
        if (field == FE_TASK_SCROLL_DOWN) {
            if (tasks.Num() - tasks_top_index > 10) return true;
        }
        if (field == FE_TASK_SCROLL_UP) {
            if (tasks_top_index > 0) return true;
        }

		if (field == FE_TASK_SPAWN_ENTITY_NAME) return true;
		if (field == FE_TASK_SPAWN_ENTITY_CLASSNAME) return true;

        if (tasks_selected_row != -1) {
			if (field == FE_TASK_INSERT) return true;
            if (field == FE_TASK_DELETE) return true;
        
            if (field == FE_TASK_ASAP) return true;
            if (field == FE_TASK_TIME) return true;

            //get the task
            const CScriptEntityTask *task = tasks.Item(tasks_selected_row + tasks_top_index);
            if (task == NULL) return false;

            //get the entity type.
            cin_entity_type entity_type = cin->GetEntityType(entity_name, entity->UniqueID());

            if (entity_type == CET_TASK_QUEUE) {
                if (field == FE_TASK_MAKE_MOVETO) return true;
                if (field == FE_TASK_MAKE_TURN) return true;
                if (field == FE_TASK_MAKE_MOVEANDTURN) return true;
                if (field == FE_TASK_MAKE_WAIT) return true;
                if (field == FE_TASK_MAKE_ATTRIBUTE_BACKUP) return true;
                if (field == FE_TASK_MAKE_ATTRIBUTE_RESTORE) return true;
                if (field == FE_TASK_MAKE_ATTRIBUTE_YAWSPEED) return true;
                if (field == FE_TASK_MAKE_ATTRIBUTE_RUNSPEED) return true;
                if (field == FE_TASK_MAKE_ATTRIBUTE_WALKSPEED) return true;
                if (field == FE_TASK_MAKE_TELEPORT) return true;
                if (field == FE_TASK_MAKE_HEADSCRIPT) return true;
                if (field == FE_TASK_MAKE_START_RUNNING) return true;
                if (field == FE_TASK_MAKE_START_WALKING) return true;
                if (field == FE_TASK_MAKE_PLAY_ANIMATION) return true;
                if (field == FE_TASK_MAKE_SET_IDLE_ANIMATION) return true;
                if (field == FE_TASK_MAKE_PLAY_SOUND) return true;
                if (field == FE_TASK_MAKE_TRIGGER_BRUSH_USE) return true;
                if (field == FE_TASK_MAKE_SPAWN_ENTITY) return true;
                if (field == FE_TASK_MAKE_REMOVE_ENTITY) return true;
                if (field == FE_TASK_MAKE_CLEARGOALS) return true;
            }
            else if (entity_type == CET_TRIGGER_BRUSH) {
                if (field == FE_TASK_MAKE_TRIGGER_BRUSH_USE) return true;
            }

            //get the task type.
            task_type type = task->Type();

            //check the type.
            if (type == TT_ATTRIBUTE_RUN_SPEED || 
                type == TT_ATTRIBUTE_WALK_SPEED ||
                type == TT_ATTRIBUTE_YAW_SPEED ||
                type == TT_WAIT) 
            {
                if (field == FE_TASK_ATTRIBUTE) return true;
            }

            if( ( type == TT_PLAY_ANIMATION ) || 
				( type == TT_SET_IDLE_ANIMATION ) || 
				( type == TT_MOVE_TO ) ) 
			{
                if (field == FE_ANIMATION_NAME) return true;
            }

            if (type == TT_PLAY_SOUND) {
                if (field == FE_SOUND_NAME) return true;
                if (field == FE_SOUND_DURATION) return true;
            }

            if (type == TT_TRIGGER_BRUSH_USE){
                if (field == FE_USE_ENTITY_NAME) return true;
            }
        }

    }

    return false;
}


void edit_start_entities(int field) {
    if (cin == NULL) return;

    //get the array of entities we are editing.
    const array<CScriptEntity> &entities = EntitiesCurrent();

    if (field == FE_SAVE) {
        FieldEdit(field_desc_entities[field]);
    }
    else if (field == FE_ADD) {
        FieldEdit(field_desc_entities[field], "");
    }
    else if (field == FE_DELETE) {
        FieldEdit(field_desc_entities[field]);
    }
    else if (field == FE_ENTITY_SCROLL_UP) {
        FieldEdit(field_desc_entities[field]);
    }
    else if (field == FE_ENTITY_SCROLL_DOWN) {
        FieldEdit(field_desc_entities[field]);
    }
    else if (entities_selected_row != -1) {
        //get the current entity.
        const CScriptEntity *entity = EntitiesCurrent().Item(entities_selected_row);
        if (entity == NULL) return;

        //get the entity name.
        const char *entity_name = entity->Name();
        if (entity_name == NULL) return;

        if (field == FE_TASK_ADD) {
            FieldEdit(field_desc_entities[field]);
        }

        else if (field == FE_TASK_SCROLL_UP) {
            FieldEdit(field_desc_entities[field]);
        }
        else if (field == FE_TASK_SCROLL_DOWN) {
            FieldEdit(field_desc_entities[field]);
        }

		if (field == FE_TASK_SPAWN_ENTITY_NAME) FieldEdit(field_desc_entities[field], entity->UniqueID());
		if (field == FE_TASK_SPAWN_ENTITY_CLASSNAME) FieldEdit(field_desc_entities[field], entity->Name());

        //get the current field.
        if (tasks_selected_row != -1) {
            const CScriptEntityTask *task = entity->Tasks().Item(tasks_selected_row + tasks_top_index);
            if (task == NULL) return;

            if (field == FE_TASK_INSERT) FieldEdit(field_desc_entities[field]);
            if (field == FE_TASK_DELETE) FieldEdit(field_desc_entities[field]);
            
            if (field == FE_TASK_TIME) FieldEdit(field_desc_entities[field], task->Time());
            if (field == FE_TASK_ATTRIBUTE) FieldEdit(field_desc_entities[field], task->AttributeValue());
            if (field == FE_ANIMATION_NAME) FieldEdit(field_desc_entities[field], task->AnimationName());
            if (field == FE_USE_ENTITY_NAME) FieldEdit(field_desc_entities[field], task->UseEntityName());
            if (field == FE_SOUND_NAME) FieldEdit(field_desc_entities[field], task->SoundName());
            if (field == FE_SOUND_DURATION) FieldEdit(field_desc_entities[field], task->SoundDuration());


            //get the entity type.
            cin_entity_type entity_type = cin->GetEntityType(entity_name, entity->UniqueID());
            
            if (entity_type == CET_TASK_QUEUE) {
                if (field == FE_TASK_MAKE_MOVETO) FieldEdit(field_desc_entities[field]);
                if (field == FE_TASK_MAKE_TURN) FieldEdit(field_desc_entities[field]);
                if (field == FE_TASK_MAKE_MOVEANDTURN) FieldEdit(field_desc_entities[field]);
                if (field == FE_TASK_MAKE_WAIT) FieldEdit(field_desc_entities[field]);
                if (field == FE_TASK_MAKE_ATTRIBUTE_BACKUP) FieldEdit(field_desc_entities[field]);
                if (field == FE_TASK_MAKE_ATTRIBUTE_RESTORE) FieldEdit(field_desc_entities[field]);
                if (field == FE_TASK_MAKE_ATTRIBUTE_YAWSPEED) FieldEdit(field_desc_entities[field]);
                if (field == FE_TASK_MAKE_ATTRIBUTE_RUNSPEED) FieldEdit(field_desc_entities[field]);
                if (field == FE_TASK_MAKE_ATTRIBUTE_WALKSPEED) FieldEdit(field_desc_entities[field]);
                if (field == FE_TASK_MAKE_START_RUNNING) FieldEdit(field_desc_entities[field]);
                if (field == FE_TASK_MAKE_START_WALKING) FieldEdit(field_desc_entities[field]);
                if (field == FE_TASK_MAKE_TELEPORT) FieldEdit(field_desc_entities[field]);
                if (field == FE_TASK_MAKE_HEADSCRIPT) FieldEdit(field_desc_entities[field]);
                if (field == FE_TASK_MAKE_PLAY_ANIMATION) FieldEdit(field_desc_entities[field]);
                if (field == FE_TASK_MAKE_SET_IDLE_ANIMATION) FieldEdit(field_desc_entities[field]);
                if (field == FE_TASK_MAKE_PLAY_SOUND) FieldEdit(field_desc_entities[field]);
                if (field == FE_TASK_MAKE_TRIGGER_BRUSH_USE) FieldEdit(field_desc_entities[field]);
                if (field == FE_TASK_MAKE_SPAWN_ENTITY) FieldEdit(field_desc_entities[field]);
                if (field == FE_TASK_MAKE_CLEARGOALS) FieldEdit(field_desc_entities[field]);
            }
            else if (entity_type == CET_TRIGGER_BRUSH) {
                if (field == FE_TASK_MAKE_TRIGGER_BRUSH_USE) FieldEdit(field_desc_entities[field]);
            }
        }
    }
    else {
        //default action... whatever.
        FieldEdit(field_desc_entities[field]);
    }
}

void edit_end_entities(int field) {
    buffer128 buffer;
    
    if (field == FE_SAVE) {
        buffer.Set("gce_entities_save\n");
    }
    else if (field == FE_ADD) {
        //get the string that was entered.
        buffer.Set("gce_entities_add \"%s\"\n", FieldEditText());
    }
    else if (field == FE_DELETE) {
        if (entities_selected_row == -1) return;
        buffer.Set("gce_entities_delete %d\n", entities_selected_row + entities_top_index);
    }
    else if (field == FE_ENTITY_SCROLL_DOWN) {
        entities_top_index++;
    }
    else if (field == FE_ENTITY_SCROLL_UP) {
        entities_top_index--;
    }
    else if (entities_selected_row != -1) {
        //compute the selected entity index and task index.
        int selected_entity = entities_selected_row + entities_top_index;
        int selected_task = tasks_selected_row + tasks_top_index;

        if (field == FE_TASK_ADD) {
            buffer.Set("gce_entities_add_task %d\n", selected_entity);
        }
		else if (field == FE_TASK_SPAWN_ENTITY_NAME) {
			buffer.Set("gce_entities_set_entity_id %d %d %s\n", selected_entity, selected_task, FieldEditText());
		}
		else if (field == FE_TASK_SPAWN_ENTITY_CLASSNAME) {
			buffer.Set("gce_entities_set_entity_name %d %d %s\n", selected_entity, selected_task, FieldEditText());
		}
        else {
            //get the current entity.
            const CScriptEntity *entity = EntitiesCurrent().Item(selected_entity);
            if (entity == NULL) return;

            if (field == FE_TASK_SCROLL_DOWN) {
                tasks_top_index++;
            }
            else if (field == FE_TASK_SCROLL_UP) {
                tasks_top_index--;
            }

            //see if any tasks are selected.
            else if (tasks_selected_row != -1) {
                //get the selected task.
                const CScriptEntityTask *task = entity->Tasks().Item(selected_task);
                if (task == NULL) return;

                //check the field.
                if (field == FE_TASK_DELETE) {
                    buffer.Set("gce_entities_task_delete %d %d\n", selected_entity, selected_task);
                }
                else if (field == FE_TASK_INSERT) {
                    buffer.Set("gce_entities_task_insert %d %d\n", selected_entity, selected_task);
                }
                else if (field == FE_TASK_TIME) {
                    buffer.Set("gce_entities_task_time %d %d %s\n", selected_entity, selected_task, FieldEditText());
                }
                else if (field == FE_TASK_ASAP) {
                    buffer.Set("gce_entities_task_time %d %d -1.0\n", selected_entity, selected_task);
                }
                else if (field == FE_TASK_MAKE_MOVETO) {
                    buffer.Set("gce_entities_task_make_moveto %d %d\n", selected_entity, selected_task);
                }
                else if (field == FE_TASK_MAKE_TURN) {
                    buffer.Set("gce_entities_task_make_turn %d %d\n", selected_entity, selected_task);
                }
                else if (field == FE_TASK_MAKE_MOVEANDTURN) {
                    buffer.Set("gce_entities_task_make_moveandturn %d %d\n", selected_entity, selected_task);
                }
                else if (field == FE_TASK_MAKE_WAIT) {
                    buffer.Set("gce_entities_task_make_wait %d %d\n", selected_entity, selected_task);
                }
                else if (field == FE_TASK_MAKE_ATTRIBUTE_BACKUP) {
                    buffer.Set("gce_entities_task_make_attribute_backup %d %d\n", selected_entity, selected_task);
                }
                else if (field == FE_TASK_MAKE_ATTRIBUTE_RESTORE) {
                    buffer.Set("gce_entities_task_make_attribute_restore %d %d\n", selected_entity, selected_task);
                }
                else if (field == FE_TASK_MAKE_ATTRIBUTE_YAWSPEED) {
                    buffer.Set("gce_entities_task_make_attribute_yawspeed %d %d\n", selected_entity, selected_task);
                }
                else if (field == FE_TASK_MAKE_ATTRIBUTE_RUNSPEED) {
                    buffer.Set("gce_entities_task_make_attribute_runspeed %d %d\n", selected_entity, selected_task);
                }
                else if (field == FE_TASK_MAKE_ATTRIBUTE_WALKSPEED) {
                    buffer.Set("gce_entities_task_make_attribute_walkspeed %d %d\n", selected_entity, selected_task);
                }
                else if (field == FE_TASK_MAKE_START_RUNNING) {
                    buffer.Set("gce_entities_task_make_start_running %d %d\n", selected_entity, selected_task);
                }
                else if (field == FE_TASK_MAKE_START_WALKING) {
                    buffer.Set("gce_entities_task_make_start_walking %d %d\n", selected_entity, selected_task);
                }
                else if (field == FE_TASK_MAKE_TELEPORT) {
                    buffer.Set("gce_entities_task_make_teleport %d %d\n", selected_entity, selected_task);
                }
                else if (field == FE_TASK_MAKE_HEADSCRIPT) {
                    buffer.Set("gce_entities_task_make_headscript %d %d\n", selected_entity, selected_task);
                }
                else if (field == FE_TASK_MAKE_PLAY_ANIMATION) {
                    buffer.Set("gce_entities_task_make_play_animation %d %d\n", selected_entity, selected_task);
                }
                else if (field == FE_TASK_MAKE_SET_IDLE_ANIMATION) {
                    buffer.Set("gce_entities_task_make_set_idle_animation %d %d\n", selected_entity, selected_task);
                }
                else if (field == FE_TASK_MAKE_PLAY_SOUND) {
                    buffer.Set("gce_entities_task_make_play_sound %d %d\n", selected_entity, selected_task);
                }
                else if (field == FE_TASK_ATTRIBUTE) {
                    buffer.Set("gce_entities_task_attribute %d %d %s\n", selected_entity, selected_task, FieldEditText());
                }
                else if (field == FE_ANIMATION_NAME) {
                    buffer.Set("gce_entities_task_attribute %d %d %s\n", selected_entity, selected_task, FieldEditText());
                }
                else if (field == FE_SOUND_NAME) {
					char duration[64];
					sprintf(duration, "%f", entity->Tasks().Item(selected_task)->SoundDuration());
                    buffer.Set("gce_entities_task_attribute %d %d %s %s\n", selected_entity, selected_task, FieldEditText(), duration);
				}
                else if (field == FE_SOUND_DURATION) {
                    buffer.Set("gce_entities_task_attribute %d %d %s %s\n", selected_entity, selected_task, entity->Tasks().Item(selected_task)->SoundName(), FieldEditText());
				}
                else if (field == FE_TASK_MAKE_TRIGGER_BRUSH_USE) {
                    buffer.Set("gce_entities_task_make_trigger_brush_use %d %d\n", selected_entity, selected_task);
                }
                else if (field == FE_USE_ENTITY_NAME) {
                    buffer.Set("gce_entities_task_attribute %d %d %s\n", selected_entity, selected_task, FieldEditText());
                }
                else if (field == FE_TASK_MAKE_SPAWN_ENTITY) {
                    buffer.Set("gce_entities_task_make_spawn_entity %d %d %s\n", selected_entity, selected_task, entity->UniqueID());
                }
                else if (field == FE_TASK_MAKE_REMOVE_ENTITY) {
                    buffer.Set("gce_entities_task_make_remove_entity %d %d %s\n", selected_entity, selected_task, entity->UniqueID());
                }
                else if (field == FE_TASK_MAKE_CLEARGOALS) {
                    buffer.Set("gce_entities_task_make_clear_goals %d %d %s\n", selected_entity, selected_task, entity->UniqueID());
                }
            }
        }
    }

    if (buffer) server->CBuf_AddText(buffer);
}

static int entities_rows_top = -1;
static int entities_rows_bottom = -1;

static int tasks_rows_top = -1;
static int tasks_rows_bottom = -1;

void NoEntities() {
    entities_num_visible_lines = 0;
    entities_selected_row = -1;

    tasks_num_visible_lines = 0;
    tasks_selected_row = -1;
}

void NoTasks() {
    tasks_rows_top = -1;
    tasks_rows_bottom = -1;
    tasks_num_visible_lines = 0;
    tasks_selected_row = -1;
}

#define ROW_HEIGHT 15

//#define ENTITIES_LEFT 50
#define ENTITIES_LEFT 10
#define ENTITIES_COL0 (ENTITIES_LEFT + 30)
#define ENTITIES_RIGHT (ENTITIES_COL0 + 200 )

//#define TASKS_LEFT (ENTITIES_RIGHT + 30)
//#define TASKS_LEFT (ENTITIES_RIGHT + 10)
#define TASKS_LEFT (ENTITIES_RIGHT + 110)
#define TASKS_COL0 (TASKS_LEFT + 30)
#define TASKS_COL1 (TASKS_COL0 + 70)
#define TASKS_RIGHT (620)
#define TASKS_WIDTH (125)

#define TASK_BUTTONS_COL0	10
#define TASK_BUTTONS_COL1	( TASK_BUTTONS_COL0 + TASKS_WIDTH )
#define TASK_BUTTONS_COL2	( TASK_BUTTONS_COL1 + TASKS_WIDTH )
#define TASK_BUTTONS_COL3	( TASK_BUTTONS_COL2 + TASKS_WIDTH )
#define TASK_BUTTONS_COL4	( TASK_BUTTONS_COL3 + TASKS_WIDTH ) 
#define TASK_BUTTONS_COL5	( TASK_BUTTONS_COL4 + TASKS_WIDTH )

//
//Draws the entities editing screen.
//
void HUD_Entities() {
    buffer128 buffer;

    //print the shot number that is being edited.
    if (ScriptCurrentShot() != -1) {
        buffer.Set("Current Shot: %d", ScriptCurrentShot());
//        cin->DrawString(ENTITIES_LEFT, 40, buffer);
        cin->DrawString(ENTITIES_LEFT, 10, buffer);
    }

    //get the array of entities we are editing.
    const array<CScriptEntity> &entities = EntitiesCurrent();

    //draw our save and delete buttons.
    FieldDraw(field_desc_entities[FE_SAVE], ENTITIES_LEFT, 30);
    FieldDraw(field_desc_entities[FE_DELETE], ENTITIES_LEFT, 50);

    int y = 95;
    FieldDraw(field_desc_entities[FE_ADD], ENTITIES_LEFT, y, ""); y += 15;


    //get the number of entities.
    buffer.Set("Num Entities: %d", entities.Num());
    cin->DrawString(ENTITIES_LEFT, y, buffer); y += 15;

    if (entities.Num() < 1) {
        NoEntities();
        NoTasks();
        return;
    }

    //do some error checking on our entities scroll variable.
    bound_max(entities_top_index, entities.Num() - 10);
    bound_min(entities_top_index, 0);

    //determine how many entities we are going to draw.
    entities_num_visible_lines = entities.Num() - entities_top_index;
    bound_max(entities_num_visible_lines, 10);

    //do error checking on our selected entity variable.
    if (entities_selected_row < 0 || entities_selected_row >= entities_num_visible_lines) {
        //deselect all entities.
        entities_selected_row = -1;
    }

    //draw the table of entities.
    entities_rows_top = y;
    entities_rows_bottom = y + entities_num_visible_lines * ROW_HEIGHT;

    //draw the entity list scroll buttons.
    if (entities.Num() - entities_top_index > 10) {
        FieldDraw(field_desc_entities[FE_ENTITY_SCROLL_DOWN], ENTITIES_LEFT - 30, entities_rows_top + 20);
    }
    if (entities_top_index > 0) {
        FieldDraw(field_desc_entities[FE_ENTITY_SCROLL_UP], ENTITIES_LEFT - 30, entities_rows_top);
    }

    //draw black background behind numbers
    HUDDrawBox(ENTITIES_LEFT, entities_rows_top, ENTITIES_COL0, entities_rows_bottom, CVector(0.0, 0.0, 0.0));

    //draw background around our hilighted entity.
    if (entities_selected_row != -1) {
        HUDDrawBox(ENTITIES_LEFT, entities_rows_top + entities_selected_row * ROW_HEIGHT,
                   ENTITIES_COL0, entities_rows_top + (entities_selected_row + 1) * ROW_HEIGHT, CVector(1.0, 1.0, 1.0));
    }

    //draw vertical lines.
    HUDDrawBox(ENTITIES_LEFT, entities_rows_top, ENTITIES_LEFT, entities_rows_bottom, CVector(0.5, 0.5, 0.5));
    HUDDrawBox(ENTITIES_COL0, entities_rows_top, ENTITIES_COL0, entities_rows_bottom, CVector(0.5, 0.5, 0.5));
    HUDDrawBox(ENTITIES_RIGHT, entities_rows_top, ENTITIES_RIGHT, entities_rows_bottom, CVector(0.5, 0.5, 0.5));

    //draw horizontal lines.
    for (int i = 0; i < entities_num_visible_lines; i++) {
        HUDDrawBox(ENTITIES_LEFT, entities_rows_top + i * ROW_HEIGHT, ENTITIES_RIGHT, entities_rows_top + i * ROW_HEIGHT, CVector(0.5, 0.5, 0.5));
    }

    //draw horizontal line on bottom.
    HUDDrawBox(ENTITIES_LEFT, entities_rows_top + i * ROW_HEIGHT, ENTITIES_RIGHT, entities_rows_top + i * ROW_HEIGHT, CVector(0.5, 0.5, 0.5));

    //draw the entity numbers and names.
    for (i = 0; i < entities_num_visible_lines; i++) {
        buffer.Set("%2d", i + entities_top_index);
        cin->DrawString(ENTITIES_LEFT + 5, entities_rows_top + i * ROW_HEIGHT + 4, buffer);

        const CScriptEntity *entity = entities.Item(i);
        if (entity == NULL) continue;

        buffer.Set("%s", entity->Name());
        cin->DrawString(ENTITIES_COL0 + 10, entities_rows_top + i * ROW_HEIGHT + 4, buffer);
    }


    //if we have any entity selected, we draw it's info on the right side of the screen.
    if (entities_selected_row == -1) {
        NoTasks();
        return;
    }

    //get the selected entity.
    const CScriptEntity *entity = entities.Item(entities_selected_row + entities_top_index);
    if (entity == NULL) return;


    //get the entity's task list.
    const task_list &tasks = entity->Tasks();

    //print the number of tasks.
    buffer.Set("Num Tasks: %d", tasks.Num());
    cin->DrawString(TASKS_LEFT + 180, 10, buffer);

    y = 40;

    FieldDraw(field_desc_entities[FE_TASK_ADD], TASKS_LEFT, y);
    FieldDraw(field_desc_entities[FE_TASK_INSERT], TASKS_LEFT + 100, y);
    FieldDraw(field_desc_entities[FE_TASK_DELETE], TASKS_LEFT + 200, y); y += 20;

    //check if we have any tasks.
    if (tasks.Num() < 1) {
        NoTasks();
        return;
    }

    //do some error checking on our task scroll variable.
    bound_max(tasks_top_index, tasks.Num() - 10);
    bound_min(tasks_top_index, 0);

    //determine how many tasks we will draw.
    tasks_num_visible_lines = tasks.Num() - tasks_top_index;
    bound_max(tasks_num_visible_lines, 10);

    //error check our selected task variable.
    if (tasks_selected_row < 0 || tasks_selected_row >= tasks_num_visible_lines) {
        //reset it.
        tasks_selected_row = -1;
    }

    //draw a table of tasks
    tasks_rows_top = y;
    tasks_rows_bottom = y + tasks_num_visible_lines * ROW_HEIGHT;

    //draw the task list scroll buttons.
    if (tasks.Num() - tasks_top_index > 10) {
        FieldDraw(field_desc_entities[FE_TASK_SCROLL_DOWN], TASKS_LEFT - 25, tasks_rows_top + 140);
    }
    if (tasks_top_index > 0) {
        FieldDraw(field_desc_entities[FE_TASK_SCROLL_UP], TASKS_LEFT - 25, tasks_rows_top + 120);
    }

    //draw black background behind numbers
    HUDDrawBox(TASKS_LEFT, tasks_rows_top, TASKS_COL0, tasks_rows_bottom, CVector(0.0, 0.0, 0.0));

    //draw background around our hilighted entity.
    if (tasks_selected_row != -1) {
        HUDDrawBox(TASKS_LEFT, tasks_rows_top + tasks_selected_row * ROW_HEIGHT,
                   TASKS_COL0, tasks_rows_top + (tasks_selected_row + 1) * ROW_HEIGHT, CVector(1.0, 1.0, 1.0));
    }

    //draw vertical lines.
    HUDDrawBox(TASKS_LEFT, tasks_rows_top, TASKS_LEFT, tasks_rows_bottom, CVector(0.5, 0.5, 0.5));
    HUDDrawBox(TASKS_COL0, tasks_rows_top, TASKS_COL0, tasks_rows_bottom, CVector(0.5, 0.5, 0.5));
    HUDDrawBox(TASKS_COL1, tasks_rows_top, TASKS_COL1, tasks_rows_bottom, CVector(0.5, 0.5, 0.5));
    HUDDrawBox(TASKS_RIGHT, tasks_rows_top, TASKS_RIGHT, tasks_rows_bottom, CVector(0.5, 0.5, 0.5));

    //draw horizontal lines.
    for (i = 0; i < tasks_num_visible_lines; i++) {
        HUDDrawBox(TASKS_LEFT, tasks_rows_top + i * ROW_HEIGHT, TASKS_RIGHT, tasks_rows_top + i * ROW_HEIGHT, CVector(0.5, 0.5, 0.5));
    }

    //draw horizontal line on bottom.
    HUDDrawBox(TASKS_LEFT, tasks_rows_top + i * ROW_HEIGHT, TASKS_RIGHT, tasks_rows_top + i * ROW_HEIGHT, CVector(0.5, 0.5, 0.5));

    //draw the entity numbers and names.
    for (i = 0; i < tasks_num_visible_lines; i++) {
        buffer.Set("%2d", i + tasks_top_index);
        cin->DrawString(TASKS_LEFT + 5, tasks_rows_top + i * ROW_HEIGHT + 6, buffer);

        //get the task
        const CScriptEntityTask *task = tasks.Item(i + tasks_top_index);
        if (task == NULL) continue;

        //get the task time.
        float time = task->Time();
        if (time == -1) {
            buffer.Set("asap");
        }
        else {
            buffer.Set("%.2f", time);
        }
        cin->DrawString(TASKS_COL0 + 5, tasks_rows_top + i * ROW_HEIGHT + 6, buffer);

        //get the task type.
        buffer.Set("%s", task->TypeString());
        cin->DrawString(TASKS_COL1 + 10, tasks_rows_top + i * ROW_HEIGHT + 6, buffer);
    }

	FieldDraw(field_desc_entities[FE_TASK_SPAWN_ENTITY_NAME], ENTITIES_LEFT, 65, entity->UniqueID());
	FieldDraw(field_desc_entities[FE_TASK_SPAWN_ENTITY_CLASSNAME], ENTITIES_LEFT, 80, entity->Name());

    //check if we have a task selected.
    if (tasks_selected_row == -1) {
        cin->DrawString(TASKS_LEFT, 10, "Select a task number:");
        return;
    }

//    y = tasks_rows_top + 10 * ROW_HEIGHT + 15;
    y = 10;

    //get the selected task.
    const CScriptEntityTask *task = tasks.Item(tasks_selected_row + tasks_top_index);
    if (task == NULL) return;

    //draw the time field.
    FieldDraw(field_desc_entities[FE_TASK_TIME], TASKS_LEFT, y, task->Time());

    //draw the asap button.
    FieldDraw(field_desc_entities[FE_TASK_ASAP], TASKS_LEFT + 110, y); y += 15;

    //draw type specific info for the current task.
    if (task->Type() == TT_ATTRIBUTE_RUN_SPEED) {
        field_desc_entities[FE_TASK_ATTRIBUTE].text = "Run Speed (pixels per second): ";
        FieldDraw(field_desc_entities[FE_TASK_ATTRIBUTE], TASKS_LEFT, y, task->AttributeValue());
    }
    else if (task->Type() == TT_ATTRIBUTE_WALK_SPEED) {
        field_desc_entities[FE_TASK_ATTRIBUTE].text = "Walk Speed (pixels per second): ";
        FieldDraw(field_desc_entities[FE_TASK_ATTRIBUTE], TASKS_LEFT, y, task->AttributeValue());
    }
    else if (task->Type() == TT_ATTRIBUTE_YAW_SPEED) {
        field_desc_entities[FE_TASK_ATTRIBUTE].text = "Turn Speed (degrees per second): ";
        FieldDraw(field_desc_entities[FE_TASK_ATTRIBUTE], TASKS_LEFT, y, task->AttributeValue());
    }
    else if (task->Type() == TT_WAIT) {
        field_desc_entities[FE_TASK_ATTRIBUTE].text = "Wait Time (seconds): ";
        FieldDraw(field_desc_entities[FE_TASK_ATTRIBUTE], TASKS_LEFT, y, task->AttributeValue());
    }
    else if (task->Type() == TT_PLAY_ANIMATION) {
        field_desc_entities[FE_ANIMATION_NAME].text = "Animation Name: ";
        FieldDraw(field_desc_entities[FE_ANIMATION_NAME], TASKS_LEFT, y, task->AnimationName());
    }
    else if (task->Type() == TT_PLAY_SOUND) {
        field_desc_entities[FE_SOUND_NAME].text = "Sound Name: ";
        FieldDraw(field_desc_entities[FE_SOUND_NAME], TASKS_LEFT, y, task->SoundName());

        field_desc_entities[FE_SOUND_DURATION].text = "Sound Duration: ";
        FieldDraw(field_desc_entities[FE_SOUND_DURATION], TASKS_LEFT, y+15, task->SoundDuration());
    }
    else if (task->Type() == TT_TRIGGER_BRUSH_USE) {
        field_desc_entities[FE_USE_ENTITY_NAME].text = "Entity to Use: ";
        FieldDraw(field_desc_entities[FE_USE_ENTITY_NAME], TASKS_LEFT, y, task->UseEntityName());
    }
    else if (task->Type() == TT_SET_IDLE_ANIMATION) {
        field_desc_entities[FE_ANIMATION_NAME].text = "Idle Animation Name: ";
        FieldDraw(field_desc_entities[FE_ANIMATION_NAME], TASKS_LEFT, y, task->AnimationName());
    }
    else if (task->Type() == TT_MOVE_TO) {
        field_desc_entities[FE_ANIMATION_NAME].text = "Animation Name: ";
        FieldDraw(field_desc_entities[FE_ANIMATION_NAME], TASKS_LEFT, y, task->AnimationName());
    }


    y = 370;

    //get the name of the selected entity.
    const char *entity_name = entity->Name();
    if (entity_name == NULL) return;

    //get the type of the selected entity.
    cin_entity_type entity_type = cin->GetEntityType(entity_name, entity->UniqueID());

    //draw our buttons for changing task type.
/*
    if (entity_type == CET_NONE) {
        cin->DrawString(TASKS_LEFT, y, "Entity with that scriptname doesn't"); y += 10;
        cin->DrawString(TASKS_LEFT, y, "exist or is of an unknown type."); y += 15;
    }
    else {
        cin->DrawString(TASKS_LEFT, y, "Change task type:"); y += 15;   
    }
*/
    if (entity_type == CET_NONE) {
        cin->DrawString(TASK_BUTTONS_COL0, y, "Entity with that scriptname doesn't"); y += 10;
        cin->DrawString(TASK_BUTTONS_COL0, y, "exist or is of an unknown type."); y += 15;
    }
    else {
        cin->DrawString(TASK_BUTTONS_COL0, y - 10, "Change task type:"); y += 15;   
    }


    //check the type of entity that is selected.
    if (entity_type == CET_TASK_QUEUE) {
        //draw buttons for actions that apply to entities with task queues.
		FieldDraw(field_desc_entities[FE_TASK_MAKE_TELEPORT], TASK_BUTTONS_COL0, y);
		FieldDraw(field_desc_entities[FE_TASK_MAKE_MOVEANDTURN], TASK_BUTTONS_COL1, y);
		FieldDraw(field_desc_entities[FE_TASK_MAKE_ATTRIBUTE_WALKSPEED], TASK_BUTTONS_COL2, y);
		FieldDraw(field_desc_entities[FE_TASK_MAKE_SET_IDLE_ANIMATION], TASK_BUTTONS_COL3, y); 
		FieldDraw(field_desc_entities[FE_TASK_MAKE_TRIGGER_BRUSH_USE], TASK_BUTTONS_COL4, y);

		y += 20;

		FieldDraw(field_desc_entities[FE_TASK_MAKE_WAIT], TASK_BUTTONS_COL0, y );
		FieldDraw(field_desc_entities[FE_TASK_MAKE_TURN], TASK_BUTTONS_COL1, y );
		FieldDraw(field_desc_entities[FE_TASK_MAKE_START_RUNNING], TASK_BUTTONS_COL2, y );
		FieldDraw(field_desc_entities[FE_TASK_MAKE_ATTRIBUTE_BACKUP], TASK_BUTTONS_COL3, y );
		FieldDraw(field_desc_entities[FE_TASK_MAKE_SPAWN_ENTITY], TASK_BUTTONS_COL4, y);

		y += 20;

		FieldDraw(field_desc_entities[FE_TASK_MAKE_PLAY_SOUND], TASK_BUTTONS_COL0, y );
		FieldDraw(field_desc_entities[FE_TASK_MAKE_ATTRIBUTE_YAWSPEED], TASK_BUTTONS_COL1, y );
		FieldDraw(field_desc_entities[FE_TASK_MAKE_ATTRIBUTE_RUNSPEED], TASK_BUTTONS_COL2, y );
		FieldDraw(field_desc_entities[FE_TASK_MAKE_ATTRIBUTE_RESTORE], TASK_BUTTONS_COL3, y ); 
		FieldDraw(field_desc_entities[FE_TASK_MAKE_REMOVE_ENTITY], TASK_BUTTONS_COL4, y );

		y += 20;

		FieldDraw(field_desc_entities[FE_TASK_MAKE_MOVETO], TASK_BUTTONS_COL0, y ); 
		FieldDraw(field_desc_entities[FE_TASK_MAKE_START_WALKING], TASK_BUTTONS_COL1, y );
		FieldDraw(field_desc_entities[FE_TASK_MAKE_PLAY_ANIMATION], TASK_BUTTONS_COL2, y );
		FieldDraw(field_desc_entities[FE_TASK_MAKE_HEADSCRIPT], TASK_BUTTONS_COL3, y );
		FieldDraw(field_desc_entities[FE_TASK_MAKE_CLEARGOALS], TASK_BUTTONS_COL4, y );
    }
    else if (entity_type == CET_TRIGGER_BRUSH) {
        //draw buttons for actions that apply to trigger brushes.
        FieldDraw(field_desc_entities[FE_TASK_MAKE_TRIGGER_BRUSH_USE], TASKS_LEFT, y);
    }
}

void MouseDownEntities(int x, int y) {
    if (entities_num_visible_lines < 1) return;

  
	//check if we clicked on one of the rows of the entity table.
    if (y >= entities_rows_top && y <= entities_rows_bottom) {
        //get the row number that was clicked on
        int row = (y - entities_rows_top) / ROW_HEIGHT;
        bound_min(row, 0);
        bound_max(row, entities_num_visible_lines - 1);

        //check if we clicked on the entity number or name.
        if (x >= ENTITIES_LEFT && x < ENTITIES_RIGHT) {
            //we clicked on the entity.
            //select this entity.
            entities_selected_row = row;

            //deselect any task we had selected.
            tasks_selected_row = -1;
        }
    }

    //check if we clicked on a row in the task table.
    if (y >= tasks_rows_top && y <= tasks_rows_bottom) {
        //get the row number that was clicked on
        int row = (y - tasks_rows_top) / ROW_HEIGHT;
        bound_min(row, 0);
        bound_max(row, tasks_num_visible_lines - 1);

        //check what column we clicked on
        if (x >= TASKS_LEFT && x < TASKS_COL0) {
            //we clicked the task number
            tasks_selected_row = row;
        }
        else if (x >= TASKS_COL0 && x < TASKS_COL1) {
            //we clicked on the task time.
            tasks_selected_row = row;
        }
        else if (x >= TASKS_COL1 && x < TASKS_RIGHT) {
            //we clicked on the task description.
            tasks_selected_row = row;
        }
    }
}

