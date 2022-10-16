#include "dk_shared.h"
#include "ref.h"
#include "client.h"

#include "dk_cl_curve.h"

#include "dk_array.h"
#include "dk_point.h"
#include "dk_gce_spline.h"

int MakeCurve(int num_positions, const CVector *positions, const CVector *velocities,
    entity_t *reference_entity, float max_angle, float max_length)
{
    if (num_positions < 2) return 0;
    if (positions == NULL) return 0;
    if (velocities == NULL) return 0;
    if (reference_entity == NULL) return 0;

    //make a spline
    CSplineSequence *spline = new CSplineSequence();

	if( spline == NULL ) return 0;

    //initialize the spline with the given points.
    spline->InitPositions(num_positions, positions, velocities);

    //	spit out entities.
	int num_entities = spline->MakeCurveSegments (max_angle, max_length);

    //delete the spline.
    delete spline;

    return num_entities;
}





