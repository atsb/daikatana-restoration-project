#include <stdio.h>

#include "dk_misc.h"
#include "ref.h"

#include "dk_array.h"
#include "dk_point.h"
#include "dk_gce_spline.h"
#include "dk_gce_headbob.h"


CEntityHeadScript::CEntityHeadScript() {
    
}

CEntityHeadScript::~CEntityHeadScript() {
    
}

void CEntityHeadScript::Reset() {
    //reset our spline sequence.
    angle.Reset();
}

void CEntityHeadScript::operator=(const CEntityHeadScript &other) {
    angle = other.angle;
}

float CEntityHeadScript::Length() const {
    return angle.NumPoints() * HEADSCRIPT_RESOLUTION / 1000.0f;
}

void CEntityHeadScript::Angle(float time, CVector &ret_angle) const {
    //get the number of points in the script.
    int num_points = angle.NumPoints();

    //set a default angle.
    ret_angle.Zero();

    //make sure we have some points.
    if (num_points < 1) {
        //return the default angle.
        return;
    }
    
    if (num_points == 1) {
        ret_angle = angle.Point(0);
        return;
    }

    //compute the segment that corresponds to the given time.
    int segment_num = time / (HEADSCRIPT_RESOLUTION / 1000.0f);

    if (segment_num < 0) {
        //return the first point in the script.
        ret_angle = angle.Point(0);
        return;
    }

    if (segment_num >= num_points - 1) {
        //return the last point in the script.
        ret_angle = angle.Point(num_points - 1);
        return;
    }

    //get the spline at the computed segment number
    const CPointSpline *spline = angle.Spline(segment_num);
    if (spline == NULL) return;

    //compute the time offset to use within this segment.
    float segment_time = time - segment_num * (HEADSCRIPT_RESOLUTION / 1000.0f);

    //get the position at the computed segment time.
    spline->Pos(segment_time, ret_angle);
}

void CEntityHeadScript::Add(const CVector &angles) {
    //check how many points we have in our spline already.
    int num_points = angle.NumPoints();
    if (num_points < 0 || num_points > 1000) return;

    //special case the first point.
    if (num_points == 0) {
        angle.Init(angles);
        return;
    }

    //make a segment_data array that can be used to specify times for each segment.
    array_obj<CSplineSequenceSegmentData> segment_data;
    segment_data.Length(num_points);

    //initialize the times to our fixed separation
    for (int i = 0; i < num_points; i++) {
        segment_data.Item(i).time = HEADSCRIPT_RESOLUTION / 1000.0f;
    }

    //add the new angle to the end of the sequence.
    angle.Add(angles, segment_data, true, false, false);
}



