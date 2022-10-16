#include "p_user.h"
#include "world.h"
#include "dk_misc.h"

#include "entity_pointer.h"
#include "dk_hashtable.h"

//a hashtable used for reference tracking.
static hashtable<userEntity_t *> *reference_table = NULL;

//initialize the hashtable.  define a class with a constructor and make a global instance of the class.
//the constructor will be called when the dll loads
class pointer_startup {public: pointer_startup();}; static pointer_startup startup; pointer_startup::pointer_startup() {
    reference_table = new hashtable<userEntity_t *>(2048);
}

//registers a pointer.
void IPointAt(userEntity_t **my_var, userEntity_t *points_here) {
    if (my_var == NULL || points_here == NULL) return;

    //add the variable to the hashtable, using the value of the pointer as the key.
    reference_table->Add(my_var, (unsigned int)points_here);
}

//unregisters a pointer.
void RemovePointer(userEntity_t **my_var, userEntity_t *which_pointed_here) {
    if (my_var == NULL || which_pointed_here == NULL) return;

    //delete the var from the table.
    reference_table->Delete(my_var, (unsigned int)which_pointed_here);
}

//when an entity is deleted, it should call this function with itself and NULL 
//to set all pointers that point to it to NULL.
void ReplacePointer(userEntity_t *that_point_here, userEntity_t *to_point_here) {
    if (that_point_here == NULL) return;
    if (that_point_here == to_point_here) return;

    //find a pointer that points to the given entity.
    userEntity_t **ptr = reference_table->Delete((unsigned int)that_point_here);

    //reset all pointers found in the table until none are left that point to the given entity.
    while (ptr != NULL) {
        //change the value of the pointer to the new value passed in.
        *ptr = to_point_here;

        //re-insert the pointer back into the table.
        IPointAt(ptr, to_point_here);

        //get the next pointer that points at our entity.
        ptr = reference_table->Delete((unsigned int)that_point_here);
    }
}



