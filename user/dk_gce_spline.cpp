#include "ref.h"

#include "dk_array.h"
#include "dk_point.h"
#include "dk_gce_spline.h"
#include "dk_misc.h"
#include "dk_beams.h"
#include "dk_cin_types.h"

CCubicSpline::CCubicSpline() {
    a = b = c = d = 0.0f;
}

void CCubicSpline::Init(float p0, float d0, float p1, float d1, float t) {
    d = p0;
    c = d0;

    float u, v, w, x, y, z;
    u = 3 * t * t;
    v = 2 * t;
    w = d1 - c;

    x = t * t * t;
    y = t * t;
    z = p1 - c * t - d;

    y *= u / x;
    z *= u / x;

    y -= v;
    z -= w;

    b = z / y;
    a = (w - v * b) / u;
}

void CCubicSpline::Init(CCubicSpline &other) {
    a = other.a;
    b = other.b;
    c = other.c;
    d = other.d;
}

float CCubicSpline::Value(float time) const {
    return time * (time * (a * time + b) + c) + d;
}

float CCubicSpline::Dir(float time) const {
    return time * (3 * a * time + 2 * b) + c;
}

CPointSpline::CPointSpline() {

}

void CPointSpline::Init(CVector &p0, CVector &d0, CVector &p1, CVector &d1, float time) {
    x.Init(p0.x, d0.x, p1.x, d1.x, time);
    y.Init(p0.y, d0.y, p1.y, d1.y, time);
    z.Init(p0.z, d0.z, p1.z, d1.z, time);
}

void CPointSpline::Init(CPointSpline *other) {
    x.Init(other->x);
    y.Init(other->y);
    z.Init(other->z);
}

void CPointSpline::Pos(float time, CVector &pos) const {
    pos.Set(x.Value(time), y.Value(time), z.Value(time));
}

void CPointSpline::Dir(float time, CVector &dir) const {
    dir.Set(x.Dir(time), y.Dir(time), z.Dir(time));

    //normalize it.
    dir.Normalize();
}

float CPointSpline::Len(float end_time) {
    //get the beginning of spline.
    CVector start;
    Pos(0.0f, start);

    //the length of the spline.
    float len = 0.0f;

    //break the spline up into small chunks.
    for (float time = 0.05f; time < end_time + 0.001; time += 0.05f) {
        //get the end of this small segment.
        CVector end;
        Pos(time, end);

        CVector line;
        line = end - start;

        len += line.Length();

        //move end to start.
        start = end;
    }

    return len;
}

CSplineSequenceSegmentData::CSplineSequenceSegmentData() {
    time = 1.0f;

    have_start_fov = false;
    have_end_fov = false;

    start_fov = 90.0f;
    end_fov = 90.0f;
    
    have_start_timescale = false;
    have_end_timescale = false;

    start_timescale = 1.0f;
    end_timescale = 1.0f;
    
    have_start_color = false;
    have_end_color = false;

    start_r = SCRIPT_R_DEFAULT;
    start_g = SCRIPT_G_DEFAULT;
    start_b = SCRIPT_B_DEFAULT;
    start_a = SCRIPT_A_DEFAULT;
    
    end_r = SCRIPT_R_DEFAULT;
    end_g = SCRIPT_G_DEFAULT;
    end_b = SCRIPT_B_DEFAULT;
    end_a = SCRIPT_A_DEFAULT;
}

void CSplineSequenceSegmentData::operator=(const CSplineSequenceSegmentData &other) {
    time = other.time;

    have_start_fov = other.have_start_fov;
    have_end_fov = other.have_end_fov;

    start_fov = other.start_fov;
    end_fov = other.end_fov;
    
    have_start_timescale = other.have_start_timescale;
    have_end_timescale = other.have_end_timescale;

    start_timescale = other.start_timescale;
    end_timescale = other.end_timescale;
    
    have_start_color = other.have_start_color;
    have_end_color = other.have_end_color;

    start_r = other.start_r;
    start_g = other.start_g;
    start_b = other.start_b;
    start_a = other.start_a;
    
    end_r = other.end_r;
    end_g = other.end_g;
    end_b = other.end_b;
    end_a = other.end_a;
}


CSplineSequence::CSplineSequence() {
    position_only = false;

    num_points = 0;

    total_time = 0.0f;

    cache_segment = 0;
    cache_time = 0.0f;

    velocity_start = velocity_end = EV_ZERO;
}

CSplineSequence::~CSplineSequence() {
    Reset();
}

void CSplineSequence::Reset() {
    //delete our splines.
    position.Reset();
    angle.Reset();

    num_points = 0;
    total_time = 0.0f;

    cache_segment = 0;
    cache_time = 0.0f;

    velocity_start = velocity_end = EV_ZERO;

    position_only = false;
}

void CSplineSequence::operator=(const CSplineSequence &other) {
    //reset ourself first.
    Reset();

    //copy over each member.
    position_only = other.position_only;
    num_points = other.num_points;
    total_time = other.total_time;
    velocity_start = other.velocity_start;
    velocity_end = other.velocity_end;

    position = other.position;
    angle = other.angle;

    segment_data = other.segment_data;
}

void CSplineSequence::Init(const CVector &start, const CVector &dir) {
    position_only = false;
    num_points = 1;
    total_time = 0.0f;
    cache_segment = 0;
    cache_time = 0.0f;

    position.Init(start);
    angle.Init(dir);
}

//adds a point to the sequence.  time is the time it takes to get from the previous point to this point.
void CSplineSequence::Add(const CVector &loc, const CVector &dir, float time) {
    if (position_only == true) return;

    if (num_points < 1) {
        Init(loc, dir);
        return;   
    }

    //increase num_points
    num_points++;

    //make a new segment data object.
    CSplineSequenceSegmentData new_segment;
    new_segment.time = time;

    //add the segment data to our array.
    segment_data.Add(new_segment);

    //add the new location and angle to our 2 components.
    position.Add(loc, segment_data, false, velocity_start == EV_ZERO, velocity_end == EV_ZERO);
    angle.Add(dir, segment_data, true, velocity_start == EV_ZERO, velocity_end == EV_ZERO);

    ComputeTime();
}

// inserts a point at the specified point in the sequence.  time is the time it takes to get from the 
// previous point to this point.
void CSplineSequence::Insert(const int index, const CVector &loc, const CVector &dir, float time) {
    if (position_only == true) return;
    
    if (index<0||index>num_points)
        return;
    
    if (num_points<1)
        Add(loc, dir, time);

    //increase num_points
    num_points++;

    //make a new segment data object.
    CSplineSequenceSegmentData new_segment;
    new_segment.time = time;

    //add the segment data to our array.
    segment_data.Insert(new_segment, index-1);

    //add the new location and angle to our 2 components.
    position.Insert(index, loc, segment_data, false, velocity_start == EV_ZERO, velocity_end == EV_ZERO);
    angle.Insert(index, dir, segment_data, true, velocity_start == EV_ZERO, velocity_end == EV_ZERO);

    ComputeTime();
}

//gets the velocity at the given point.
float CSplineSequence::Velocity(int pnt) const {
    if (pnt < 0 || pnt >= num_points) return 0.0f;

    return position.Velocity(pnt);
}

void CSplineSequence::Velocity(int pnt, CVector &vel) const {
    if (pnt < 0 || pnt >= num_points) return;

    position.Velocity(pnt, vel);
}

bool CSplineSequence::HaveStartFOV(int index) const {
    return segment_data.Item(index).have_start_fov;
}

bool CSplineSequence::HaveEndFOV(int index) const {
    return segment_data.Item(index).have_end_fov;
}

float CSplineSequence::StartFOV(int index) const {
    return segment_data.Item(index).start_fov;
}

float CSplineSequence::EndFOV(int index) const {
    return segment_data.Item(index).end_fov;
}

bool CSplineSequence::HaveStartTimescale(int index) const {
    return segment_data.Item(index).have_start_timescale;
}

bool CSplineSequence::HaveEndTimescale(int index) const {
    return segment_data.Item(index).have_end_timescale;
}

float CSplineSequence::StartTimescale(int index) const {
    return segment_data.Item(index).start_timescale;
}

float CSplineSequence::EndTimescale(int index) const {
    return segment_data.Item(index).end_timescale;
}

bool CSplineSequence::HaveStartColor(int index) const {
    return segment_data.Item(index).have_start_color;
}

bool CSplineSequence::HaveEndColor(int index) const {
    return segment_data.Item(index).have_end_color;
}

float CSplineSequence::StartR(int index) const {
    return segment_data.Item(index).start_r;
}

float CSplineSequence::StartG(int index) const {
    return segment_data.Item(index).start_g;
}

float CSplineSequence::StartB(int index) const {
    return segment_data.Item(index).start_b;
}

float CSplineSequence::StartA(int index) const {
    return segment_data.Item(index).start_a;
}

float CSplineSequence::EndR(int index) const {
    return segment_data.Item(index).end_r;
}

float CSplineSequence::EndG(int index) const {
    return segment_data.Item(index).end_g;
}

float CSplineSequence::EndB(int index) const {
    return segment_data.Item(index).end_b;
}

float CSplineSequence::EndA(int index) const {
    return segment_data.Item(index).end_a;
}

//sets the velocity at the given point.
void CSplineSequence::Time(int index, float time) {
    //check to make sure this is a valid index.
    if (index < 0 || index >= num_points - 1) return;
    //make sure we have a valid time value.
    if (time < 0.01) return;

    //change the time.
    segment_data.Item(index).time = time;

    //recompute the splines.... i'm lazy and dont want to just update the ones that are
    //really affected, so redo them all.
    position.ComputeSplines(segment_data, false, velocity_start == EV_ZERO, velocity_end == EV_ZERO);

    //if we are not a position only sequence, compute the splines for the angle too.
    if (position_only == false) {
        angle.ComputeSplines(segment_data, true, velocity_start == EV_ZERO, velocity_end == EV_ZERO);
    }

    ComputeTime();
}

//sets the angles at the given point.
void CSplineSequence::Angle(int index, const CVector &a) {
    if (index < 0 || index >= num_points) return;

    //change the value.
    angle.Point(index, a);

    //recompute the angle spline
    if (position_only == false) {
        angle.ComputeSplines(segment_data, true, velocity_start == EV_ZERO, velocity_end == EV_ZERO);
    }
}

//sets the position of the given point.
void CSplineSequence::Pos(int index, const CVector &pos) {
    if (index < 0 || index >= num_points) return;

    //change the value.
    position.Point(index, pos);

    //recompute the position spline
    position.ComputeSplines(segment_data, false, velocity_start == EV_ZERO, velocity_end == EV_ZERO);
}

int CSplineSequence::NumPoints() const {
    return num_points;
}

void CSplineSequence::ComputeTime() {
    total_time = 0.0f;

    for (int i = 0; i < num_points - 1; i++) {
        total_time += segment_data.Item(i).time;
    }

    //reset our sequence number computation cache.
    cache_segment = 0;
    cache_time = 0.0f;
}

int CSplineSequence::Segment(float &time) {
    //the index and time we start searching at.
    int index = 0;
    float segment_time = 0.0f;

    //check if the time is after our cached time.
    if (cache_time <= time) {
        index = cache_segment;
        segment_time = cache_time;
    }

    //loop through the remainder of the segments.
    for (; index < num_points - 1; index++) {
        //check if the time is before the next.
        if (segment_time + segment_data.Item(index).time >= time) {
            //this is the index we will return.  cache the time and index value.
            cache_segment = index;
            cache_time = segment_time;

            time -= segment_time;
            return index;
        }

        //add the segment time to our total time.
        segment_time += segment_data.Item(index).time;
    }

    //bad, the time is past all the segments, return the last segment.
    time -= segment_time;
    return num_points - 2;
}

float CSplineSequence::TotalTime() const {
    return total_time;
}

float CSplineSequence::Time(int segment) const {
    if (segment < 0 || segment >= num_points - 1) return 0.0f;

    return segment_data.Item(segment).time;
}

void CSplineSequence::Pos(float time, CVector &pos) {
    if (num_points < 1) return;
    if (num_points == 1) {
        //return the point we have.
        pos = position.Point(0);
        return;
    }

    //get the segment that this time is on.
    int segment = Segment(time);

    //check to make sure the time isnt more than the spline has.
    bound_max(time, segment_data.Item(segment).time);
    bound_min(time, 0.0f);

    //get the spline.
    const CPointSpline *spline = position.Spline(segment);
    if (spline == NULL) return;

    //get the position from the spline.
    spline->Pos(time, pos);
}

void CSplineSequence::Vel(float time, CVector &vel) {
    if (num_points < 1) return;
    if (num_points == 1) {
        //there is no velocity because there is only on point
        vel.Set(1.0f, 0.0f, 0.0f);
        return;
    }

    //get the segment that this time is on.
    int segment = Segment(time);

    //check to make sure the time isnt more than the spline has.
    bound_max(time, segment_data.Item(segment).time);
    bound_min(time, 0.0f);

    //get the spline.
    const CPointSpline *spline = position.Spline(segment);
    if (spline == NULL) return;

    //get the position from the spline.
    spline->Dir(time, vel);

    //get the vel from the segment.
    spline->Dir(time, vel);
}

void CSplineSequence::Dir(float time, CVector &dir) {
    if (position_only == true) return;
    if (num_points < 1) return;
    if (num_points == 1) {
        //return the angle we have.
        dir = angle.Point(0);
        return;
    }

    //get the segment that this time is on.
    int segment = Segment(time);

    //check to make sure the time isnt more than the spline has.
    bound_max(time, segment_data.Item(segment).time);
    bound_min(time, 0.0f);

    //get the spline.
    const CPointSpline *spline = angle.Spline(segment);
    if (spline == NULL) return;

    //get the angle from the spline.
    spline->Pos(time, dir);
}

static CVector bad_index;

const CVector &CSplineSequence::Point(int index) const {
    if (index < 0 || index >= num_points) return bad_index;

    return position.Point(index);
}

const CVector &CSplineSequence::Angle(int index) const {
    if (index < 0 || index >= num_points) return bad_index;
    if (position_only == true) return bad_index;
    return angle.Point(index);
}

end_vel CSplineSequence::VelStart() const {
    return velocity_start;
}

end_vel CSplineSequence::VelEnd() const {
    return velocity_end;
}

void CSplineSequence::VelStart(end_vel vel) {
    velocity_start = vel;

    //recompute the splines.... i'm lazy and dont want to just update the ones that are
    //really affected, so redo them all.
    position.ComputeSplines(segment_data, false, velocity_start == EV_ZERO, velocity_end == EV_ZERO);
    angle.ComputeSplines(segment_data, true, velocity_start == EV_ZERO, velocity_end == EV_ZERO);
}

void CSplineSequence::VelEnd(end_vel vel) {
    velocity_end = vel;

    //recompute the splines.... i'm lazy and dont want to just update the ones that are
    //really affected, so redo them all.
    position.ComputeSplines(segment_data, false, velocity_start == EV_ZERO, velocity_end == EV_ZERO);
    angle.ComputeSplines(segment_data, true, velocity_start == EV_ZERO, velocity_end == EV_ZERO);
}

void CSplineSequence::DeletePoint(int index) {
    if (index < 0 || index >= num_points) return;

    //delete the point from the components.
    position.Delete(index);
    angle.Delete(index);

    //
    //  compute a new time array.
    //

    //check if we still have enough points for the time array.
    if (num_points <= 2) {
        //no more times, because we will have fewer than 2 points.
        segment_data.Length(0);
    }
    //we special case the begin and end points.
    else if (index == 0) {
        //remove the first item from the array.
        segment_data.Remove(0);
    }
    else if (index == num_points - 1) {
        //remove the last item.
        segment_data.Remove(index);
    }
    else {
        //add the time of the segment we will delete to the segment before it.
        segment_data.Item(index - 1).time += segment_data.Item(index).time;

        //remove the segment.
        segment_data.Remove(index);
    }

    //decrement our point count.
    num_points--;

    //compute the total sequence time.
    ComputeTime();

    //recompute the splines if we have more than 1 point.
    if (num_points < 2) return;

    position.ComputeSplines(segment_data, false, velocity_start == EV_ZERO, velocity_end == EV_ZERO);
    angle.ComputeSplines(segment_data, true, velocity_start == EV_ZERO, velocity_end == EV_ZERO);
}

void CSplineSequence::HaveStartFOV(int index, bool flag) {
    segment_data.Item(index).have_start_fov = flag;
}

void CSplineSequence::HaveEndFOV(int index, bool flag) {
    segment_data.Item(index).have_end_fov = flag;
}

void CSplineSequence::StartFOV(int index, float degrees) {
    segment_data.Item(index).start_fov = degrees;
}

void CSplineSequence::EndFOV(int index, float degrees) {
    segment_data.Item(index).end_fov = degrees;
}

void CSplineSequence::HaveStartTimescale(int index, bool flag) {
    segment_data.Item(index).have_start_timescale = flag;
}

void CSplineSequence::HaveEndTimescale(int index, bool flag) {
    segment_data.Item(index).have_end_timescale = flag;
}

void CSplineSequence::StartTimescale(int index, float val) {
    segment_data.Item(index).start_timescale = val;
}

void CSplineSequence::EndTimescale(int index, float val) {
    segment_data.Item(index).end_timescale = val;
}

void CSplineSequence::HaveStartColor(int index, bool flag) {
    segment_data.Item(index).have_start_color = flag;
}

void CSplineSequence::HaveEndColor(int index, bool flag) {
    segment_data.Item(index).have_end_color = flag;
}

void CSplineSequence::StartColor(int index, float r, float g, float b, float a) {
    segment_data.Item(index).start_r = r;
    segment_data.Item(index).start_g = g;
    segment_data.Item(index).start_b = b;
    segment_data.Item(index).start_a = a;
}

void CSplineSequence::EndColor(int index, float r, float g, float b, float a) {
    segment_data.Item(index).end_r = r;
    segment_data.Item(index).end_g = g;
    segment_data.Item(index).end_b = b;
    segment_data.Item(index).end_a = a;
}



//returns the smooth direction a spline should take when passing through loc from before, towards after.
void SplineDirection(CVector &before, CVector &loc, CVector &after, CVector &dir, bool wrap) {
    CVector wrapped;

    //get the direction from loc to after.
    //check if we have to wrap after.
    (wrap == true) ? ComputeEndAngleWrap(loc, after, wrapped) : wrapped = after;
    CVector out;
    out = wrapped - loc;
    out.Normalize();

    //get the direction from before to loc.
    //check if we have to wrap loc.
    (wrap == true) ? ComputeEndAngleWrap(before, loc, wrapped) : wrapped = loc;
    CVector in;
    in = wrapped - before;
    in.Normalize();

    //get the average.
    dir = out + in;
    dir.Normalize();
}

void SplineStartDir(CVector &loc, CVector &after, CVector &after_dir, CVector &dir, bool wrap) {
    CVector wrapped;

    //get the direction from loc to after.
    //check if we need to wrap after.
    (wrap == true) ? ComputeEndAngleWrap(loc, after, wrapped) : wrapped = after;
    CVector out;
    out = wrapped - loc;
    out.Normalize();

    //get the reflection of after direction across out.
    PointReflection(after_dir, out, dir);
    dir.Normalize();
}

void SplineEndDir(CVector &before, CVector &loc, CVector &before_dir, CVector &dir, bool wrap) {
    CVector wrapped;

    //get direction from before to loc.
    //check if we need to wrap loc.
    (wrap == true) ? ComputeEndAngleWrap(before, loc, wrapped) : wrapped = loc;
    CVector in;
    in = wrapped - before;
    in.Normalize();

    //get reflection of before dir across in.
    PointReflection(before_dir, in, dir);
    dir.Normalize();
}

//returns the reflection of a point through a line.  Line should be normalized.
void PointReflection(CVector &pnt, CVector &line, CVector &reflect) {
    //get dot of pnt and line, the component of pnt along line.
    float dot = pnt.DotProduct(line);

    //stretch line out to dot length.
    CVector ref_pnt;
    ref_pnt = line * dot;

    //subtract pnt from ref_pnt;
    CVector ray;
    ray = pnt - ref_pnt;

    //subtract ray from ref_pnt.
    reflect = ref_pnt - ray;
}

void ComputeEndAngleWrap(CVector &angle_start, CVector &angle_end, CVector &wrapped_end) {
    wrapped_end = angle_end;

    //check the difference between the components.
    if (angle_end.x - angle_start.x > 180.0f) {
        //wrap the x.
        wrapped_end.x = angle_end.x - 360.0f;
    }
    else if (angle_end.x - angle_start.x < -180.0f) {
        //wrap the x.
        wrapped_end.x = angle_end.x + 360.0f;
    }

    if (angle_end.y - angle_start.y > 180.0f) {
        //wrap the y.
        wrapped_end.y = angle_end.y - 360.0f;
    }
    else if (angle_end.y - angle_start.y < -180.0f) {
        //wrap the y.
        wrapped_end.y = angle_end.y + 360.0f;
    }

    if (angle_end.z - angle_start.z > 180.0f) {
        //wrap the z.
        wrapped_end.z = angle_end.z - 360.0f;
    }
    else if (angle_end.z - angle_start.z < -180.0f) {
        //wrap the z.
        wrapped_end.z = angle_end.z + 360.0f;
    }
}

float MixVelocity(float oldv, float newv) {
    if (oldv < 0.01 && newv < 0.01) {
        return newv;
    }

    if (oldv < newv) {
        return oldv + (newv - oldv) * (oldv / newv);
    }

    //oldv is greater.
    return newv + (oldv - newv) * (newv / oldv);
}

//Initializes the sequence as position-only, using the given array of positions and velocities.
void CSplineSequence::InitPositions(int num_positions, const CVector *positions, const CVector *velocities) {
    if (num_positions < 2) return;
    if (positions == NULL || velocities == NULL) return;

    //get rid of all the old data.
    Reset();

    //set our flag to tell us that this spline only has position data.
    position_only = true;

    //set the number of points.
    num_points = num_positions;

    //set other misc data.
    velocity_start = velocity_end = EV_DEFAULT;

    //make our array of segment data the correct length.
    segment_data.Length(num_points - 1);

    //make the splines and compute the segment times.
    position.ComputeSplines(num_points, positions, velocities, segment_data);

    ComputeTime();
}

//this function is special purpose in that it doesn't generate the velocity array, instead keeping
//the given velocities un-normalized.
void CSplineSequenceComponent::ComputeSplines(int num_positions, const CVector *positions, 
    const CVector *velocities, array_obj<CSplineSequenceSegmentData> &segments_data) 
{
    if (num_positions < 2) return;

    //reset all our data.
    Reset();

    //set the number of points we will have.
    num_points = num_positions;

    //allocate arrays.
    points = new CVector[num_points];
    velocity_norm = new CVector[num_points];
    splines = new CPointSpline *[num_points - 1];
    //dont allocate a velocity array.

    //intialize data for each point
    for (int i = 0; i < num_points; i++) {
        //set the point.
        points[i] = positions[i];

        //do not normalize the velocities
        velocity_norm[i] = velocities[i];
    }

    //compute the times.
    for (i = 0; i < num_points - 1; i++) {
        //get the two points.
        CVector p0 = points[i];
        CVector p1 = points[i + 1];

        //get the distance between the two points.
        CVector line;
        line = p1 - p0;

        segments_data.Item(i).time = line.Length();
    }

    //construct the splines.
    for (i = 0; i < num_points - 1; i++) {
        splines[i] = new CPointSpline();

        splines[i]->Init(points[i], velocity_norm[i], points[i + 1], velocity_norm[i + 1], segments_data.Item(i).time);
    }
}

//Makes as many entities as are needed to represent the sequence position splines.
//Uses the given function for allocating entities.  reference_entity is used
//to initialize data memebers in the entity that are not actually needed by the function
//to position the laser entities.  max_angle (in degrees) affects the smoothness of the curve by specifying
//the maximum allowed angle between adjacent entities.  max_length specifies the maximum length of
//any entity
int CSplineSequence::MakeCurveEntities(entity_t *reference_entity, entity_t *(*entity_alloc)(), 
    float max_angle, float max_length) 
{
    if (reference_entity == NULL) return 0;
    if (entity_alloc == NULL) return 0;

    //the number of entities we have actually used to make the curve.
    int num_entities = 0;

    //check the parameters passed in.
    bound_min(max_angle, 1.0f);
    bound_max(max_angle, 80.0f);

    bound_min(max_length, 1.0f);

    //square the length.
    max_length = max_length * max_length;

    //compute the cos of the tolerance angle that was passed in.
    float angle_tolerance = cos(max_angle / 180.0f * PI);

    //move along the spline and construct lasers to approximate the path.
    //get the initial velocity direction.
    CVector begin_vel;
    CVector begin_pos;
    float begin_time = 0.0f;
    int begin_seg = 0;

    Pos(begin_time, begin_pos);
    Vel(begin_time, begin_vel);

    //compute the step length, based on the time of the current segment.
    float step = segment_data.Item(begin_seg).time / 50;
    bound_min(step, 0.01f);

    for (float time = 0.0f; time <= TotalTime(); time += step) {
        //get the position at this time.
        CVector pos;
        Pos(time, pos);

        //get the distance since the last position.
        CVector line;
        line = pos - begin_pos;
        float distance = line.DotProduct(line);

        //check if we have moved far enough to allow a new entity.
        if (distance < 1.0f) {
            //we havent move far enough in the world.
            continue;
        }

        //we use this copy of the current time because our call to Segment below will modify the value
        //we pass it.
        float segment_time = time;

        //get the segment number for the current time.
        int cur_seg = Segment(segment_time);

        //get the velocity vector at this time
        CVector vel;
        Vel(time, vel);

        //check if we should make a laser from the last point to the current point.
        if ((vel.DotProduct(begin_vel) > angle_tolerance) &&   //the spline is still on a straight course.
            (distance < max_length) &&                  //we haven't gone too far since the end of the last piece.
            (cur_seg == begin_seg))                     //we are still on the same segment.
        {
            //we should go farther before making the next laser.
            continue;
        }

        //the angles are far enough apart, make a marker.
        //get the begin and ending time we use to find positions from the spline.
        float time_start = begin_time;
        float time_end = time;
        if (time_end - time_start < 0.005f) {
            time_start = time_end - 0.005f;
        }

        //save the current data for next time.
        begin_pos = pos;
        begin_vel = vel;
        begin_time = time;
        begin_seg = cur_seg;

        //recompute our step.
        step = segment_data.Item(begin_seg).time / 50;
        bound_min(step, 0.01f);

        //make an entity.
        MakeCurveEntity(time_start, time_end, begin_seg, reference_entity, entity_alloc);

        num_entities++;
    }

    //get the end point of the spline.
    CVector end_pos;
    Pos(TotalTime(), end_pos);

    //get the distance since the end of the last laser.
    CVector line;
    line = end_pos - begin_pos;
    float distance = line.DotProduct(line);

    //if the last marker we placed was too far back, make one last marker.
    if (distance > 1.0f) {
        //make an entity from the previous time to the ending time of the sequence.
        MakeCurveEntity(begin_time, TotalTime(), begin_seg, reference_entity, entity_alloc);

        num_entities++;
    }

    return num_entities;
}

void CSplineSequence::MakeCurveEntity(float time_start, float time_end, int segment_num,
    entity_t *reference_entity, entity_t *(*entity_alloc)()) 
{
    //allocate an entity.
    entity_t *entity = entity_alloc();
    if (entity == NULL) return;

    //copy default data from the reference entity to our new entity.
    memcpy(entity, reference_entity, sizeof(entity_t));

    //get the initial and ending positions of the laser.
    CVector loc, endloc;
    Pos(time_start, loc);
    Pos(time_end, endloc);

    //set the entity model.
    entity->model = NULL;

    //position the entity.
    entity->origin = loc;
    entity->oldorigin = endloc;

    //set the render scale.
    static CVector unity(1.0f, 1.0f, 1.0f);
    entity->render_scale = unity;

    //store the segment number in the oldframe field
    entity->oldframe = segment_num;
}

CSplineSequenceComponent::CSplineSequenceComponent() {
    num_points = 0;
    points = NULL;
    velocity_norm = NULL;
    velocity = NULL;

    splines = NULL;
}

CSplineSequenceComponent::~CSplineSequenceComponent() {
    Reset();
}

void CSplineSequenceComponent::Init(const CVector &start) {
    //reset ourself first.
    Reset();

    //allocate arrays.
    points = new CVector[1];
    velocity_norm = new CVector[1];
    velocity = (float*)memmgr.X_Malloc(sizeof(float),MEM_TAG_MISC);

    //set the values for the first point.
    points[0] = start;
    velocity_norm[0].Zero();
    velocity[0] = 0.0f;

    num_points = 1;
}

void CSplineSequenceComponent::Reset() {
    //delete our splines
	if( num_points == 0 )
	{
		return;
	}

    for (int i = 0; i < num_points - 1; i++) {
        delc(splines[i]);
    }

    //delete our arrays.
    delca(splines);
    delca(points);
    delca(velocity_norm);
    memmgr.X_Free(velocity);

    //reset our point count
    num_points = 0;
}

void CSplineSequenceComponent::operator=(const CSplineSequenceComponent &other) {
    //reset ourself.
    Reset();

    //get the number of points.
    num_points = other.num_points;

    //allocate our arrays.
    if (num_points > 0) {
        points = new CVector[num_points];
        velocity_norm = new CVector[num_points];
        velocity = (float*)memmgr.X_Malloc(sizeof(float)*num_points, MEM_TAG_MISC);
    }

    if (num_points > 1) {
        splines = new CPointSpline *[num_points - 1];
    }

    for (int i = 0; i < num_points; i++) {
        points[i] = other.points[i];
        velocity_norm[i] = other.velocity_norm[i];
        velocity[i] = other.velocity[i];
    }

    //copy over the splines.
    for (i = 0; i < num_points - 1; i++) {
        splines[i] = new CPointSpline();

        splines[i]->Init(other.splines[i]);
    }
}

int CSplineSequenceComponent::NumPoints() const {
    return num_points;
}

const CVector &CSplineSequenceComponent::Point(int index) const {
    if (index < 0 || index >= num_points) return bad_index;

    return points[index];
}

float CSplineSequenceComponent::Velocity(int index) const {
    if (index < 0 || index >= num_points) return 1.0f;

    return velocity[index];
}

void CSplineSequenceComponent::Velocity(int index, CVector &vel) const {
    if (index < 0 || index >= num_points) return;

    vel = velocity_norm[index];
}

const CPointSpline *CSplineSequenceComponent::Spline(int index) const {
    if (index < 0 || index >= num_points - 1) return NULL;

    return splines[index];
}

void CSplineSequenceComponent::Point(int index, const CVector &p) {
    if (index < 0 || index >= num_points) return;

    points[index] = p;
}

void CSplineSequenceComponent::Add(const CVector &p, 
                                   const array_obj<CSplineSequenceSegmentData> &segments_data, 
                                   bool wrap, bool zero_start_vel, bool zero_end_vel) 
{
    if (num_points < 1) {
        Init(p);
        return;   
    }

    //get new arrays.
    CVector *newpoints = new CVector[num_points + 1];
    CVector *newvelocity_norm = new CVector[num_points + 1];
    float *newvelocity = (float*)memmgr.X_Malloc(sizeof(float)*(num_points+1),MEM_TAG_MISC);
    CPointSpline **newsplines = new CPointSpline *[num_points];

    //copy the existing data over.
    for (int i = 0; i < num_points; i++) {
        newpoints[i] = points[i];
        newvelocity_norm[i] = velocity_norm[i];
        newvelocity[i] = velocity[i];
    }

    for (i = 0; i < num_points - 1; i++) {
        newsplines[i] = splines[i];
    }

    //delete the old arrays.
    delca(points);
    delca(velocity_norm);
    memmgr.X_Free(velocity);
    delca(splines);

    //swap in the new arrays.
    points = newpoints;
    velocity_norm = newvelocity_norm;
    velocity = newvelocity;
    splines = newsplines;

    //add in the new point
    points[num_points] = p;

    //get a new spline
    splines[num_points - 1] = new CPointSpline();

    //increase num_points
    num_points++;

    //compute the splines
    ComputeSplines(segments_data, wrap, zero_start_vel, zero_end_vel);
}

void CSplineSequenceComponent::Insert(const int index, const CVector &p, 
                                      const array_obj<CSplineSequenceSegmentData> &segments_data, 
                                      bool wrap, bool zero_start_vel, bool zero_end_vel) 
{
    if (index<0||index>num_points)
        return;
    
    if (num_points < 1) {
        Init(p);
        return;   
    }

    //get new arrays.
    CVector *newpoints = new CVector[num_points + 1];
    CVector *newvelocity_norm = new CVector[num_points + 1];
    float *newvelocity = (float*)memmgr.X_Malloc(sizeof(float)*(num_points+1),MEM_TAG_MISC);
    CPointSpline **newsplines = new CPointSpline *[num_points];

    //copy the existing data over.
    for (int i = 0; i < index; i++) {
        newpoints[i] = points[i];
        newvelocity_norm[i] = velocity_norm[i];
        newvelocity[i] = velocity[i];
    }

    for (i = 0; i < index - 1; i++) {
        newsplines[i] = splines[i];
    }

    for (i = index+1; i < num_points+1; i++) {
        newpoints[i] = points[i-1];
        newvelocity_norm[i] = velocity_norm[i-1];
        newvelocity[i] = velocity[i-1];
    }

    for (i = index; i < num_points; i++) {
        newsplines[i] = splines[i-1];
    }

    //delete the old arrays.
    delca(points);
    delca(velocity_norm);
    memmgr.X_Free(velocity);
    delca(splines);

    //swap in the new arrays.
    points = newpoints;
    velocity_norm = newvelocity_norm;
    velocity = newvelocity;
    splines = newsplines;

    //add in the new point
    points[index] = p;

    //get a new spline
    splines[index - 1] = new CPointSpline();

    //increase num_points
    num_points++;

    //compute the splines
    ComputeSplines(segments_data, wrap, zero_start_vel, zero_end_vel);
}

void CSplineSequenceComponent::Delete(int index) {
    if (index < 0 || index >= num_points) return;

    //check how many points we have.
    if (num_points < 2) {
        Reset();
        return;
    }

    //make new point and velocity arrays.
    CVector *newpoints = new CVector[num_points - 1];
    CVector *newvelocity_norm = new CVector[num_points - 1];
    float *newvelocity = (float*)memmgr.X_Malloc(sizeof(float)*(num_points-1),MEM_TAG_MISC);

    //copy items before the index we are removing.
    for (int dest = 0; dest < index; dest++) {
        newpoints[dest] = points[dest];
        newvelocity_norm[dest] = velocity_norm[dest];
        newvelocity[dest] = velocity[dest];
    }

    //copy items after the index we are removing.
    for (int i = index + 1; i < num_points; i++, dest++) {
        newpoints[dest] = points[i];
        newvelocity_norm[dest] = velocity_norm[i];
        newvelocity[dest] = velocity[i];
    }

    //delete the old arrays.
    delca(points);
    delca(velocity_norm);
    memmgr.X_Free(velocity);

    //swap in the new arrays.
    points = newpoints;
    velocity_norm = newvelocity_norm;
    velocity = newvelocity;

    //check if we are going to have enough points for a spline.
    if (num_points < 2) {
        //we didnt have any splines before.
    }
    else if (num_points == 2) {
        //we had one spline, now we'll have none.
        delete splines[0];
        delca(splines);
    }
    else {
        //make a new spline array.
        CPointSpline **newsplines = new CPointSpline *[num_points - 2];

        //check if we are deleting the last point in the sequence.
        if (index == num_points - 1) {
            //copy over all but the last spline.
            for (i = 0; i < num_points - 2; i++) {
                newsplines[i] = splines[i];
            }

            //delete the last spline.
            delete splines[num_points - 2];
        }
        else {
            //copy splines before the removed index.
            for (dest = 0; dest < index; dest++) {
                newsplines[dest] = splines[dest];
            }

            //delete the spline at the removed index.
            delete splines[index];

            //copy splines after the removed index.
            for (i = index + 1; i < num_points - 1; i++, dest++) {
                newsplines[dest] = splines[i];
            }
        }

        //delete the old array.
        delca(splines);

        //swap in the new array.
        splines = newsplines;
    }

    //decrement our count.
    num_points--;
}


float MixVelocity(float oldv, float newv);

void CSplineSequenceComponent::ComputeSplines(const array_obj<CSplineSequenceSegmentData> &segments_data, 
    bool wrap, bool zero_start_vel, bool zero_end_vel) 
{
    //check how many points we have.
    if (num_points < 2) {
        return;
    }

    CVector line;
    CVector wrapped_end, wrapped_end2;
    CVector vel_start, vel_end;

    if (num_points == 2) {
        //check if we should wrap.
        (wrap == true) ? ComputeEndAngleWrap(points[0], points[1], wrapped_end) : wrapped_end = points[1];

        //compute the position spline velocities.
        line = wrapped_end - points[0];
        velocity[0] = velocity[1] = line.Normalize() / segments_data.Item(0).time;

        velocity_norm[0] = line;
        velocity_norm[1] = line;

        //make the position velocity vectors and position spline.
        vel_start = velocity_norm[0] * (zero_start_vel ? 0.0f : velocity[0]);
        vel_end = velocity_norm[1] * (zero_end_vel ? 0.0f : velocity[1]);
        splines[0]->Init(points[0], vel_start, wrapped_end, vel_end, segments_data.Item(0).time);

        return;
    }

    if (num_points == 3) {
        //we need to make both segments.
        //get the direction for the middle point.
        SplineDirection(points[0], points[1], points[2], velocity_norm[1], wrap);

        //get the begin and end direction.
        SplineStartDir(points[0], points[1], velocity_norm[1], velocity_norm[0], wrap);
        SplineEndDir(points[1], points[2], velocity_norm[1], velocity_norm[2], wrap);

        //check if we need to wrap
        (wrap == true) ? ComputeEndAngleWrap(points[0], points[1], wrapped_end) : wrapped_end = points[1];
        (wrap == true) ? ComputeEndAngleWrap(points[1], points[2], wrapped_end2) : wrapped_end2 = points[2];

        //get the velocities for the points.
        line = wrapped_end - points[0];
        float len = line.Length();
        velocity[0] = len / segments_data.Item(0).time;

        line = wrapped_end2 - points[1];
        len = line.Length();
        velocity[2] = len / segments_data.Item(1).time;

        velocity[1] = MixVelocity(velocity[1], velocity[2]);

        //init the splines.
        vel_start = velocity_norm[0] * (zero_start_vel ? 0.0f : velocity[0]);
        vel_end = velocity_norm[1] * velocity[1];
        splines[0]->Init(points[0], vel_start, wrapped_end, vel_end, segments_data.Item(0).time);

        vel_start = vel_end;
        vel_end = velocity_norm[2] * (zero_end_vel ? 0.0f : velocity[2]);
        splines[1]->Init(points[1], vel_start, wrapped_end2, vel_end, segments_data.Item(1).time);
        return;
    }

    //we need an array to store the wrapped end points for each of the segments.
    CVector *wrapped_ends = new CVector[num_points - 1];
    //another array to store the separations between the points.
    float *segment_len = (float*)memmgr.X_Malloc(sizeof(float)*(num_points-1),MEM_TAG_MISC);

    //get all of the wrapped ends.
    for (int i = 0; i < num_points - 1; i++) {
        (wrap == true) ? ComputeEndAngleWrap(points[i], points[i + 1], wrapped_ends[i]) : wrapped_ends[i] = points[i + 1];
    }

    //get all the lengths of the segments.
    for (i = 0; i < num_points - 1; i++) {
        line = wrapped_ends[i] - points[i];
        segment_len[i] = line.Length();
    }

    //compute the velocity norms for all of the middle points.
    for (i = 1; i < num_points - 1; i++) {
        SplineDirection(points[i - 1], points[i], points[i + 1], velocity_norm[i], wrap);
    }

    //compute the start and the end velocity norms
    SplineStartDir(points[0], points[1], velocity_norm[1], velocity_norm[0], wrap);
    SplineEndDir(points[num_points - 2], points[num_points - 1], velocity_norm[num_points - 2], velocity_norm[num_points - 1], wrap);

    //compute the velocity at the first point and second point.
    velocity[0] = velocity[1] = segment_len[0] / segments_data.Item(0).time;

    //compute the velocities at the remaining points.
    for (i = 1; i < num_points - 1; i++) {
        //get the velocity at the end of this segment.
        velocity[i + 1] = segment_len[i] / segments_data.Item(i).time;

        //mix this velocity with the velocity already at the beginning of this segment.
        velocity[i] = MixVelocity(velocity[i], velocity[i + 1]);
    }

    //compute the first spline.
    vel_start = velocity_norm[0] * (zero_start_vel ? 0.0f : velocity[0]);
    vel_end = velocity_norm[1] * velocity[1];
    splines[0]->Init(points[0], vel_start, wrapped_ends[0], vel_end, segments_data.Item(0).time);

    //do the splines between the first and the last.
    for (i = 1; i < num_points - 2; i++) {
        //get the start and end initial velocity
        vel_start = velocity_norm[i] * velocity[i];
        vel_end = velocity_norm[i + 1] * velocity[i + 1];
        splines[i]->Init(points[i], vel_start, wrapped_ends[i], vel_end, segments_data.Item(i).time);
    }

    //compute the last spline.
    vel_start = velocity_norm[num_points - 2] * velocity[num_points - 2];
    vel_end = velocity_norm[num_points - 1] * (zero_end_vel ? 0.0f : velocity[num_points - 1]);
    splines[num_points - 2]->Init(points[num_points - 2], vel_start, wrapped_ends[num_points - 2], vel_end, segments_data.Item(num_points - 2).time);

    //delete our temporary arrays.
    delca(wrapped_ends);
    memmgr.X_Free(segment_len);
}



