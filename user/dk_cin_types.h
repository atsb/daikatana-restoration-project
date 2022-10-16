//#pragma once

#ifndef _DK_CIN_TYPES_H
#define _DK_CIN_TYPES_H

//
//  This header file is for types and constants that are needed by cinematic code in
//  gce.dll, world.dll, and daikatana.exe
//

typedef enum {
    CET_NONE,

    CET_TASK_QUEUE,
    CET_TRIGGER_BRUSH,
    CET_DOOR,
} cin_entity_type;

class entity_attributes;

class cin_gce_export_t;

#define SCRIPT_FOV_MIN 1.0f
#define SCRIPT_FOV_MAX 145.0f
#define SCRIPT_FOV_DEFAULT 90.0f

#define SCRIPT_TIMESCALE_MIN 0.1f
#define SCRIPT_TIMESCALE_MAX 10.0f
#define SCRIPT_TIMESCALE_DEFAULT 1.0f

#define SCRIPT_COLOR_MIN 0.0f
#define SCRIPT_COLOR_MAX 255.0f
#define SCRIPT_R_DEFAULT 255.0f
#define SCRIPT_G_DEFAULT 255.0f
#define SCRIPT_B_DEFAULT 255.0f
#define SCRIPT_A_DEFAULT 0.0f

#endif // _DK_CIN_TYPES_H