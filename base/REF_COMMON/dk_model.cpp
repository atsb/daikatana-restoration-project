#define __MSC__

#include "ref.h"

#include "dk_misc.h"
#include "dk_buffer.h"
#include "dk_array.h"
#include "dk_point.h"
#include "dk_model_new.h"
#include "dk_ref_common.h"


//our array of loaded models.
array<dk_model> loaded_models;

//returns the model with the given name, loading it off disk if necessary.
const dk_model *LoadModel(const char *name, resource_t resource) {
    if (name == NULL || name[0] == '\0') return NULL;

    //search for the filename in our array of already-loaded pics.
    dk_model *model = loaded_models.Item(name, dk_model::Find);

    if (model != NULL) {
        //the model is loaded.
        //set the resource type
        model->Flag(resource);
        //return the model.
        return model;
    }

    //we must try to search for the file on disk.
    model = dk_model::Load(name);
    if (model == NULL) return NULL;

    //put the loaded model into our array for next time.
    loaded_models.Add(model, dk_model::Compare);

    //set the resource type
    model->Flag(resource);

    return model;
}

dk_model *dk_model::Load(const char *filename) {
    if (filename == NULL || filename[0] == '\0') return NULL;

    //open the file
    FILE *file = NULL;
    ri.FS_Open(filename, &file);
    if (file == NULL) return NULL;

    //the model we will create.
    dk_model *model = NULL;    

    //read the file tag.
    char buf[4];
    ri.FS_Read(buf, 4, file);
    if (strncmp(buf, "DKMD", 4) != 0) {
        //not the correct file tag.
        ri.FS_Close(file);
        return NULL;
    }

    //read the version.
    int32 version = 0;
    ri.FS_Read(&version, 4, file);

    //check the version.
    if (version < 1) {
        ri.FS_Close(file);
        return NULL;
    }
    else if (version == 1) {
        //load the version 1 model.
        //close our existing file handle.
        ri.FS_Close(file);

        //read all the file bytes into memory.
        byte *buf;
        ri.FS_LoadFile(filename, (void **)&buf);

        //make a new model object.
        model = new dk_model();

        //set the model's name.
        model->FileName(filename);

        //parse the file into our data structure.
        model->LoadV1(buf);

        //free the loaded bytes.
        ri.FS_FreeFile(buf);
    }
    else if (version > 1) {
        //read new model formal.


        //close the file.
        ri.FS_Close(file);
    }

    if (model == NULL) return NULL;

    //load the skin graphics for the model surfaces.
    

    //return the model.
    return model;
}



