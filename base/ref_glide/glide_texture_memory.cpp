#define __MSC__
#include <glide.h>

#include "ref.h"
#include "dk_misc.h"

#include "dk_object_reference.h"
#include "dk_pointer.h"
#include "dk_buffer.h"

#include "dk_ref_pic.h"

#include "glide_main.h"
#include "glide_frame_buffer.h"
#include "glide_ref_main.h"

//keeps track of a place on the card that will accomdate a texture of a particular size.
class free_slot {
  public:
    free_slot();

    //the address on the card.
    int32 address;

    free_slot *next;
};

free_slot::free_slot() {
    address = -1;
    next = NULL;
}

class tmu_texture_cache {
public:
    tmu_texture_cache();
    ~tmu_texture_cache();

    bool Init(int32 tmu_number);

protected:
    int tmu_number;

    //the current min and max texture address.
    uint32 min_texture_address;
    uint32 max_texture_address;

    //free lists for slots that can be used for 8-bit textures that have all mip levels computed.
    //There are 4 different aspect ratios (counting symetric ratios as the same), and 6 different 
    //largest sizes, which makes a total of 24 different free lists.
    free_slot open_8bit_mipped_slots[4][6];

    uint32 GetNew8bitMippedAddress(GrLOD_t largest_lod, GrAspectRatio_t aspect_ratio);

public:
    //returns the texture address to use for a new 8-bit texture that is fully mipmapped.  
    //checks first for any parts of texture memory 
    uint32 GetFree8bitMippedAddress(GrLOD_t largest_lod, GrAspectRatio_t aspect_ratio);
};

tmu_texture_cache::tmu_texture_cache() {
    tmu_number = -1;
}

bool tmu_texture_cache::Init(int tmu_number) {
    if (tmu_number < 0 || tmu_number >= glide_config.SSTs[0].sstBoard.VoodooConfig.nTexelfx) return false;

    //save the tmu number
    this->tmu_number = tmu_number;

    //get the boundaries of the texture memory on our tmu.
    min_texture_address = grTexMinAddress(tmu_number);
    max_texture_address = grTexMaxAddress(tmu_number);

    return true;
}

//returns the texture address to use for a new 8-bit texture that is fully mipmapped.  
//checks first for any parts of texture memory 
uint32 tmu_texture_cache::GetFree8bitMippedAddress(GrLOD_t largest_lod, GrAspectRatio_t aspect_ratio) {
    if (largest_lod < 0 || largest_lod > 5) return -1;

    //a free slot we can use for the texture.
    free_slot *slot = NULL;

    //check the correct list for free slots.
    switch (aspect_ratio) {
      case GR_ASPECT_8x1:
      case GR_ASPECT_1x8:
        //remove the slot from the list.
        slot = open_8bit_mipped_slots[0][largest_lod].next;
        if (slot != NULL) open_8bit_mipped_slots[0][largest_lod].next = slot->next;
        break;
      case GR_ASPECT_4x1:
      case GR_ASPECT_1x4:
        slot = open_8bit_mipped_slots[1][largest_lod].next;
        if (slot != NULL) open_8bit_mipped_slots[1][largest_lod].next = slot->next;
        break;
      case GR_ASPECT_2x1:
      case GR_ASPECT_1x2:
        slot = open_8bit_mipped_slots[2][largest_lod].next;
        if (slot != NULL) open_8bit_mipped_slots[2][largest_lod].next = slot->next;
        break;
      case GR_ASPECT_1x1:
        slot = open_8bit_mipped_slots[3][largest_lod].next;
        if (slot != NULL) open_8bit_mipped_slots[3][largest_lod].next = slot->next;
        break;
    }

    //check if we got a free slot.
    if (slot != NULL) {
        //save the address.
        uint32 address = slot->address;

        //delete the slot structure.
        delete slot;

        //return the address.
        return address;
    }

    //look for a new address to store the texture.
    uint32 address = GetNew8bitMippedAddress(largest_lod, aspect_ratio);

    return address;
}

uint32 tmu_texture_cache::GetNew8bitMippedAddress(GrLOD_t largest_lod, GrAspectRatio_t aspect_ratio) {
    //compute the amount of memory that the texture takes up.
    uint32 text_size = grTexCalcMemRequired(GR_LOD_1, largest_lod, aspect_ratio, GR_TEXFMT_P_8);

    //check if placing this sized texture will cross any illegal boundaries.
    uint32 end = min_texture_address + text_size;

    //we divide start and ending adresses by 2M and compare to see if it crosses a 2M boundary.
    if ((min_texture_address & 0xffe00000) != (end & 0xffe00000)) {
        //round the end down to the 2M boundary and use that as the starting address.
        min_texture_address = end & 0xffe00000;

        end = min_texture_address + text_size;
    }

    //check to make sure this texture fits into the free space on the card.
    if (end > max_texture_address) {
        //out of memory.
        return -1;
    }

    //save the address to use 
    uint32 start_address = min_texture_address;

    //move the min address for next time.
    min_texture_address = end;

    //return the address.
    return start_address;
}


int32 num_caches = 0;
tmu_texture_cache *texture_cache[2] = {NULL, NULL};

bool InitTextureMemory() {
    int32 num_caches = glide_config.SSTs[0].sstBoard.VoodooConfig.nTexelfx;
    bound_max(num_caches, 2);

    if (num_caches < 1) return false;

    //create a texture cache management object for each tmu.
    for (int32 i = 0; i < num_caches; i++) {
        //allocate the cache object.
        texture_cache[i] = new tmu_texture_cache();

        //initialize the cache.
        if (texture_cache[i]->Init(i) == false) {
            ri.Con_Printf(PRINT_ALL, "Failed to initialize texture cache for TMU %d\n", i);
            return false;
        }
        else {
            ri.Con_Printf(PRINT_ALL, "Initialized texture cache for TMU %d\n", i);

        }
    }

    return true;
}

