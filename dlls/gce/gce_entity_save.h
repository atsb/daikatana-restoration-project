#pragma once

class CCinematicScript;
class CScriptShot;
class CScriptEntity;
class CEntitySave;
class CEntitySaveMonster;
class fast_forward_data;

//saves entity states for all entities in a script.
void SaveEntityStates(const CCinematicScript *script);
//restores states for all entities we have data for.
void RestoreEntityStates();

//fast forwards the entities to the end states of the given script shot.
void FastForwardEntities(const CScriptShot *shot);

//
//
//  Classes used to store entity states so they can be restored when the script is completed.
//
//

class CEntitySave {
  public:
    CEntitySave();
    virtual ~CEntitySave();

//	void* CEntitySave::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* CEntitySave::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  CEntitySave::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CEntitySave::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

  protected:
    //the name of the entity.
    buffer16 name;

	buffer32	uniqueid;

    //true if we have valid save information for the entity.
    bool have_info;

    //member access to the have_info var for our derived classes.
    bool HaveInfo() const;
    void HaveInfo(bool have_info);

  public:
    //
    //member access
    //
    const char *Name() const;

    //
    //for modifying the object.
    //
    void Name(const char *name);

    const char *UniqueID() const;
    void UniqueID(const char *id);

    //
    //for saving the entity's info.
    //
    virtual bool Save() = NULL;

    //
    //for restoring the entity's saved info.
    //
    virtual bool Restore() const = NULL;

    //
    //sorted list support.
    //
    static int Compare(const CEntitySave **e1, const CEntitySave **e2);
    static int Find(const CEntitySave *data, const void *identifier);
};


class CEntitySaveMonster : public CEntitySave {
  public:
    CEntitySaveMonster();
    ~CEntitySaveMonster();

//	void* operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

    void Init(const CScriptEntity *entity);

  protected:
    //the location of the monster.
    CVector location;
    
    //the direction the monster was facing.
    CVector angles;

  public:
    //
    //  member access
    //
    const CVector &Location();
    const CVector &Angles();

    //
    //overridden class functions.
    //
    virtual bool Save();
    virtual bool Restore() const;
};


//
//  This class is used while we fast forward the script to keep track of the latest positions
//  of entities.
//

class fast_forward_data {
public:
    fast_forward_data(const char *entity_name, const CVector &start_pos, const CVector &start_angles);

//	void* fast_forward_data::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* fast_forward_data::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  fast_forward_data::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  fast_forward_data::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

    //last position
    CVector last_pos;

    //the previous position.
    CVector prev_pos;

    //the last known angle.
    CVector angles;

    buffer16 entity_name;

	buffer32	uniqueid;

    //for specifying a new position.  Angles are computed based on the last position.
    void NewPosition(const CVector &new_pos);

    //for specifying a new position and angles.
    void NewPosition(const CVector &new_pos, const CVector &new_angles);

    //for specifying new angles only.
    void NewAngles(const CVector &new_angles);

    //
    //sorted list support.
    //
    static int Compare(const fast_forward_data **e1, const fast_forward_data **e2);
    static int Find(const fast_forward_data *data, const void *identifier);
};





