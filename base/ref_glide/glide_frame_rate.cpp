#include "ref.h"
#include "dk_misc.h"

#include "glide_frame_rate.h"

frame_rate::frame_rate() {
    for (int32 i = 0; i < MAX_FRAMES_TRACKED; i++) {
        frame_times[i] = 0;
    }

    replace_index = 0;

    total_time = 0;

    rate = 0;

    last_time = GetTickCount();
}

//call once per frame.
void frame_rate::Frame() {
    //remove the frame in the replace index spot.
    total_time -= frame_times[replace_index];

    //get the current time.
    uint32 cur_time = GetTickCount();

    //get the change from last frame.
    frame_times[replace_index] = cur_time - last_time;

    total_time += frame_times[replace_index];

    //move the index we will replace at.
    replace_index++;
    if (replace_index >= MAX_FRAMES_TRACKED) {
        replace_index = 0;
    }

    //compute the rate.
    if (total_time > 0) {
        rate = MAX_FRAMES_TRACKED / float(total_time) * 1000.0f;
    }
    else {
        rate = 1.0f;
    }

    //remember the current time for next time.
    last_time = cur_time;
}

//returns the rate we have been getting.
float frame_rate::Rate() {
    return rate;
}

uint32 frame_rate::Time() {
    return last_time;
}




