//#pragma once

#ifndef _DK_CIN_WORLD_INTERFACE_H
#define _DK_CIN_WORLD_INTERFACE_H

//structure used by world.dll to pass function pointers back to the exe for use during playback.
class cin_world_import_t {
  public:
    //function to get script entity "type"
    cin_entity_type (*GetEntityType)(const char *entity_name, const char *id);

	void (*SetServerState)			( qboolean bCinematicPlaying, const char *pszCinematicName );
	void (*SetClientFOV)			( float fFOV );

    //functions to queue the different goals.
    void (*QueueMoveTo)				( const char *entity_name, const char *id, const CVector &pos, const char *animation_name );
    void (*QueueTurn)				( const char *entity_name, const char *id, const CVector &angles );
    void (*QueueMoveAndTurn)		( const char *entity_name, const char *id, const CVector &pos, const CVector &angles );
    void (*QueueWait)				( const char *entity_name, const char *id, float seconds );
    void (*QueueStartRunning)		( const char *entity_name, const char *id );
    void (*QueueStartWalking)		( const char *entity_name, const char *id, const char *animation_name );
    void (*QueueAnimation)			( const char *entity_name, const char *id, const char *animation_name );
    void (*QueueSetIdleAnimation)	( const char *entity_name, const char *id, const char *animation_name );
    void (*QueueTriggerBrushUse)	( const char *entity_name, const char *id, const char *use_entity_name );
    void (*QueuePlaySound)			( const char *entity_name, const char *id, const char *sound_name, const float sound_duration );

    
    //functions for modifying entity attributes.
    void (*AttributesBackup)		( const char *entity_name, const char *id );
    void (*AttributesRestore)		( const char *entity_name, const char *id );
    void (*AttributesRunSpeed)		( const char *entity_name, const char *id, float run_speed );
    void (*AttributesWalkSpeed)		( const char *entity_name, const char *id, float walk_speed );
    void (*AttributesTurnSpeed)		( const char *entity_name, const char *id, float yaw_speed );

    //functions for trigger brushes
    void (*TriggerBrushUse)			( const char *entity_name, const char *id );


    bool (*DoneWithTasks)			( const char *entity_name, const char *id );

    bool (*EntityIndex)				( const char *entity_name, const char *id, int &pos );
    bool (*GetEntityPosition)		( const char *entity_name, const char *id, CVector &pos, CVector &angles );

    //moves an entity.
    void (*TeleportEntity)			( const char *entity_name, const char *id, CVector &pos, CVector &angles, bool use_angles );

    //sets an entity's ideal angle.
    void (*SetEntityFacing)			( const char *entity_name, const char *id, CVector &angles );

    //gets the entity's modifyable attributes.
    bool (*EntityAttributes)		( const char *entity_name, const char *id, entity_attributes &attributes );

    //spawns the hiro bot that can be controlled by the scripts.  the given entity num should
    //be the number of the client that is replaced.
    void (*SpawnHiroActor)			( int entity_num );
    //removes the hiro bot.
    void (*RemoveHiroActor)();

    //spawns a motionless, non-solid hiro model that can be positioned to set up shots
    void (*SpawnHiroDummy)			( const CVector &origin, const CVector &angles );
    //removes the entity created by SpawnHiroDummy.
    void (*RemoveHiroDummy)();

	void (*SpawnEntity)				( const char *classname, const char *id, const CVector& dest, const CVector& dir );
	void (*RemoveEntity)			( const char *classname, const char *id );
	void (*RemoveAllEntities)();
	void (*ClearGoals)				( const char *classname, const char *id );
};

#endif // _DK_CIN_WORLD_INTERFACE_H

