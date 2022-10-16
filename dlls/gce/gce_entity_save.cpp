#include "p_user.h"

#include "dk_cin_types.h"
#include "dk_gce_interface.h"
#include "dk_misc.h"
#include "dk_buffer.h"
#include "dk_array.h"
#include "dk_point.h"

#include "dk_gce_script.h"
#include "dk_gce_entities.h"

#include "gce_entity_save.h"
#include "gce_main.h"


CEntitySave::CEntitySave() {
    have_info = false;
}

CEntitySave::~CEntitySave() {

}

bool CEntitySave::HaveInfo() const {
    return have_info;
}

void CEntitySave::HaveInfo(bool have_info) {
    this->have_info = have_info;
}

const char *CEntitySave::Name() const {
    return name;
}

void CEntitySave::Name(const char *name) {
    this->name = name;
}

const char *CEntitySave::UniqueID() const {
    return uniqueid;
}

void CEntitySave::UniqueID(const char *name) {
    this->uniqueid = name;
}

int CEntitySave::Compare(const CEntitySave **e1, const CEntitySave **e2) {
    if (e1 == NULL || e2 == NULL) return 0;
    if (*e1 == NULL || *e2 == NULL) return 0;

    return stricmp((*e1)->UniqueID(), (*e2)->UniqueID());
}

int CEntitySave::Find(const CEntitySave *data, const void *identifier) {
    if (data == NULL || identifier == NULL) return 0;

    //the identifier is a string.
    const char *name = (const char *)identifier;

    //compare the name with the entity's name.
    int compare = stricmp(name, data->Name());

    if (compare == 0) return 0;
    if (compare < 0) return -1;
    else return 1;
}



CEntitySaveMonster::CEntitySaveMonster() {

}

CEntitySaveMonster::~CEntitySaveMonster() {

}

void CEntitySaveMonster::Init(const CScriptEntity *entity) {
    if (entity == NULL) return;

    //set our name.
    Name(entity->Name());

	UniqueID( entity->UniqueID() );

    //save the entity's data.
    Save();
}

const CVector &CEntitySaveMonster::Location() {
    return location;
}

const CVector &CEntitySaveMonster::Angles() {
    return angles;
}

bool CEntitySaveMonster::Save() {
    if (cin == NULL) return false;

    //get the position of the monster.
    bool success = cin->EntityPosition(Name(), UniqueID(), location, angles);

    //set our flag saying that we got the data.
    HaveInfo(success);

    return success;
}

bool CEntitySaveMonster::Restore() const {
    if (cin == NULL) return false;

    //check if we have data available to restore.
    if (HaveInfo() == false) return false;

    CVector tempLocation = location;
	CVector tempAngles = angles;
	//teleport the entity back to it's saved location.
    cin->TeleportEntity(Name(), UniqueID(), tempLocation, tempAngles, true);

    return true;
}

//an array of saved data.
static array<CEntitySave> entity_save_data;

//an array of fast forward data.
static array<fast_forward_data> ff_data_array;

//saves entity states for all entities in a script.
void SaveEntityStates(const CCinematicScript *script) {
    if (script == NULL) return;

    //reset our array of entity save data.
    entity_save_data.Reset();

    //reset our fast-forward data array.
    ff_data_array.Reset();

    //go through all the shots in the script.
    for (int s = 0; s < script->NumShots(); s++) {
        //get the shot.
        const CScriptShot *shot = script->Shot(s);
        if (shot == NULL) continue;

        //get the entity array from the shot.
        const array<CScriptEntity> &entities = shot->Entities();

        //go through all of the entities.
        for (int ent = 0; ent < entities.Num(); ent++) {
            //get the entity.
            const CScriptEntity *entity = entities.Item(ent);
            if (entity == NULL) continue;

            //get the entity name.
            const char *name = entity->Name();
            if (name == NULL) continue;

            //check if we already have data for this entity saved.
            const CEntitySave *data = entity_save_data.Find(name, CEntitySave::Find);
            if (data != NULL) {
                //we already have data for an entity with this name.
                continue;
            }

            //save data for this entity.
            //check the type of entity this is.
            CEntitySaveMonster *monster_data = new CEntitySaveMonster();
            monster_data->Init(entity);

            //put the data into our array.
            entity_save_data.Add(monster_data, CEntitySave::Compare);

            //add an entry in our fast forward data array for this entity.
            fast_forward_data *ff_data = new fast_forward_data(name, monster_data->Location(), monster_data->Angles());
            ff_data_array.Add(ff_data, fast_forward_data::Compare);
        }
    }
}

void RestoreEntityStates() {
    //go through our array.
    for (int i = 0; i < entity_save_data.Num(); i++) {
        const CEntitySave *data = entity_save_data.Item(i);
        if (data == NULL) continue;

        //restore this entity's state.
        data->Restore();
    }
}

//teleports the entities to the ending locations specified in the shot.
void FastForwardEntities(const CScriptShot *shot) {
    if (cin == NULL) return;

    //get the array of entities.
    const array<CScriptEntity> &entities = shot->Entities();

    //get each entity.
    for (int i = 0; i < entities.Num(); i++) {
        //get the entity.
        const CScriptEntity *entity = entities.Item(i);
        if (entity == NULL) continue;

        //get the entity's tasks.
        const task_list &tasks = entity->Tasks();

        //go through the entity's tasks
        for (int t = 0; t < tasks.Num(); t++) {
            //get the task.
            const CScriptEntityTask *task = tasks.Item(t);
            if (task == NULL) continue;

            //check the task type.
            task_type type = task->Type();

            //get the entity's fast forward data.
            fast_forward_data *ff_data = ff_data_array.Item(entity->Name(), fast_forward_data::Find);
            if (ff_data == NULL) continue;

            //update the entity's fast forward data with this task.
            if (type == TT_MOVE_TO) {
                ff_data->NewPosition(task->Destination());
            }
            else if (type == TT_MOVE_AND_TURN) {
                ff_data->NewPosition(task->Destination(), task->Direction());
            }
            else if (type == TT_TELEPORT) {
                ff_data->NewPosition(task->Destination(), task->Direction());
            }
            else if (type == TT_TURN) {
                ff_data->NewAngles(task->Direction());
            }
            else {
                continue;
            }

            cin->TeleportEntity(ff_data->entity_name, ff_data->uniqueid, ff_data->last_pos, ff_data->angles, true);
        }
    }
}

fast_forward_data::fast_forward_data(const char *name, const CVector &start_pos, const CVector &start_angles) {
    //save the given data.
    entity_name = name;
    last_pos = start_pos;
    angles = start_angles;
}

//for specifying a new position.  Angles are computed based on the last position.
void fast_forward_data::NewPosition(const CVector &new_pos) {
    //copy the current position to the backup slot.
    prev_pos = last_pos;

    //save the new position.
    last_pos = new_pos;

    //compute the angle based on the last position and the new one.
    CVector line;
    line = last_pos - prev_pos;

    //compute a yaw based on that vector.
    float yaw = atan2(line.y, line.x) * 180 / PI;

    //make the angles.
    angles.x = 0.0f;
    angles.y = yaw;
    angles.z = 0.0f;
}

//for specifying a new position and angles.
void fast_forward_data::NewPosition(const CVector &new_pos, const CVector &new_angles) {
    //copy our data over.
    last_pos = new_pos;
    angles = new_angles;
}

void fast_forward_data::NewAngles(const CVector &new_angles) {
    //replace our angles
    angles = new_angles;
}

int fast_forward_data::Compare(const fast_forward_data **e1, const fast_forward_data **e2) {
    if (e1 == NULL || e2 == NULL) return 0;
    if (*e1 == NULL || *e2 == NULL) return 0;

    return stricmp((*e1)->entity_name, (*e2)->entity_name);
}

int fast_forward_data::Find(const fast_forward_data *data, const void *identifier) {
    if (data == NULL || identifier == NULL) return 0;

    //the identifier is a string.
    const char *name = (const char *)identifier;

    //compare the name with the entity's name.
    int compare = stricmp(name, data->entity_name);

    if (compare == 0) return 0;
    if (compare < 0) return -1;
    else return 1;
}




