//#pragma once

#ifndef _DK_CL_CURVE_H_
#define _DK_CL_CURVE_H_

//spits out a bunch of entities that connect the given points/directions.  
//Returns the number of entities that were made.
//reference_entity is a sample entity that has all members initialized to default values, which are copied
//to the entities created by the function.  entity_alloc is a function that returns pointers to entities that 
//are used to create the curve.  max_angle is the tolerance for the maximum angle between adjacent laser entities.
//max_length is the maximum length of any individual entity, and is useful in long, slowly curving sections of the
//curve
int MakeCurve(int num_positions, const CVector &positions, const CVector &velocities,
    entity_t *reference_entity, float max_angle, float max_length);

#endif // _DK_CL_CURVE_H_