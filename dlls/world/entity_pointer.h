//#pragma once

#ifndef _ENTITY_POINTER_H
#define _ENTITY_POINTER_H

//
//Functions and classes to manage pointers to user entities.
//They are used so that an entity can know who was pointing at it, so that
//it can set everyone's pointers to NULL when it is removed from the game.
//

//registers a pointer.
void IPointAt(userEntity_t **my_var, userEntity_t *points_here);

//unregisters a pointer.
void RemovePointer(userEntity_t **my_var, userEntity_t *which_pointed_here);

//when an entity is deleted, it should call this function with itself and NULL 
//to set all pointers that point to it to NULL.
void ReplacePointer(userEntity_t *that_point_here, userEntity_t *to_point_here);


//
//Wrapper class for userEntity_t *.  Automatically registers and keeps track of 
//pointer assignments.
//

class entity_pointer {
  public:
    __inline entity_pointer();
    __inline entity_pointer(userEntity_t *entity);  //for uses like:  entity_pointer entity = ptr;
    __inline ~entity_pointer();

    //converts the object to a pointer to userEntity_t, to pass to functions.
    __inline operator const userEntity_t *() const;
    //for setting the pointer equal to another pointer.
    __inline void operator=(userEntity_t *entity);

    //for "if (ptr == NULL) {" dudes.
    __inline operator int() const;
    //for "if (ptr) {" asses
    __inline operator bool() const;

    //for syntax like entity->...
    __inline userEntity_t *operator->();

  protected:
    //our actual pointer.
    userEntity_t *entity;
};



entity_pointer::entity_pointer() {
    entity = NULL;
}

entity_pointer::entity_pointer(userEntity_t *entity) {
    this->entity = NULL;

    *this = entity;
}

entity_pointer::~entity_pointer() {
    *this = NULL;
}

//converts the object to a pointer to userEntity_t, to pass to functions.
entity_pointer::operator const userEntity_t *() const {
    return entity;
}

//for setting the pointer equal to another pointer.
void entity_pointer::operator=(userEntity_t *value) {
    if (entity == value) return;

    //first remove our reference to our old value.
    if (entity != NULL) {
        RemovePointer(&entity, entity);
    }

    //set the new value.
    entity = value;

    //register our reference.
    if (entity != NULL) {
        IPointAt(&entity, entity);
    }
}

//for "if (ptr == NULL) {" dudes.
entity_pointer::operator int() const {
    return int(entity);
}

//for "if (ptr) {" asses
entity_pointer::operator bool() const {
    return entity != NULL;
}

userEntity_t *entity_pointer::operator->() {
    return entity;
}


#endif // _ENTITY_POINTER_H