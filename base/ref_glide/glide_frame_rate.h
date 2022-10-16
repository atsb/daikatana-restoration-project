#pragma once

//
//Generic class that can keep track of the framerate
//

#define MAX_FRAMES_TRACKED 60

class frame_rate {
  public:
    frame_rate();

  protected:
    uint32 last_time;

    uint32 frame_times[MAX_FRAMES_TRACKED];

    uint32 total_time;

    int32 replace_index;

    float rate;

  public:
    //call once per frame.
    void Frame();

    //returns the rate we have been getting.
    float Rate();

    //returns the last time.
    uint32 Time();
};



