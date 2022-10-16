//#pragma once

#ifndef _DK_GCE_SPLINE_H
#define _DK_GCE_SPLINE_H

class CVector;
class CCubicSpline;
class CPointSpline;
class CSplineSequenceComponent;
class CSplineSequenceSegmentData;
class CSplineSequence;

//moves one parameter from a start to an end with a start and end velocity over a specified length of time.
class CCubicSpline {
  public:
    CCubicSpline();
    
    //computes the spline parameters.
    void Init(float p0, float d0, float p1, float d1, float time);
    void Init(CCubicSpline &other);

    //this command can only be run in the dll
    void Save(FILE *file);
    //can be run from the exe only
    void Load(FILE *file);

  protected:
    //the cubic parameters for x, y, and z.
    float a, b, c, d;

  public:
    float Value(float time) const;
    float Dir(float time) const;
};

//moves a point from a start to an end.
class CPointSpline {
  public:
    CPointSpline();

//	void* CPointSpline::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* CPointSpline::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  CPointSpline::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CPointSpline::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

    void Init(CVector &p0, CVector &d0, CVector &p1, CVector &d1, float time);
    void Init(CPointSpline *other);

    //this command can only be run in the dll
    void Save(FILE *file);
    //can be run from the exe only
    void Load(FILE *file);

  protected:
    float time;

    //splines for x y and z.
    CCubicSpline x, y, z;

  public:
    void Pos(float time, CVector &pos) const;
    void Dir(float time, CVector &dir) const;

    //computes the length of the spline from time 0 to time.
    float Len(float time);
};

typedef enum {
    EV_DEFAULT,
    EV_ZERO,
} end_vel;

//this class is meant to store all the arrays needed for one part
//of a sequcnece, such as either a position spline, or an angle spline.
class CSplineSequenceComponent {
public:
    CSplineSequenceComponent();
    ~CSplineSequenceComponent();

    void Init(const CVector &p);
    void Reset();
    void operator=(const CSplineSequenceComponent &other);

    void Save(FILE *file) const;
    void Load(FILE *file, int32 version);

    //for backward compatibility with save version 5 and lower.
    void Init(int32 num_points, CVector *points, CVector *velocity_norm, float *velocity, CPointSpline **splines);

protected:
    //the number of points we have.
    int32 num_points;

    //the points of the 
    CVector *points;

    //the velocity directions.
    CVector *velocity_norm;

    //the velocity magnitude.
    float *velocity;

    //the splines themselves.
    CPointSpline **splines;

public:
    //
    //  member access
    //
    int32 NumPoints() const;
    const CVector &Point(int32 index) const;
    float Velocity(int32 index) const;
    void Velocity(int32 index, CVector &vel) const;
    const CPointSpline *Spline(int32 index) const;

    //
    //  functions for modifying the object.
    //
    void Add(const CVector &p, const array_obj<CSplineSequenceSegmentData> &segments_data, 
        bool wrap, bool zero_start_vel, bool zero_end_vel);
    void Insert(const int index, const CVector &p, const array_obj<CSplineSequenceSegmentData> &segments_data, 
        bool wrap, bool zero_start_vel, bool zero_end_vel);
    void Point(int32 index, const CVector &p);
    void Delete(int32 index);

    //computes the splines.  computes times and fills them into the provided array based on 
    //the separation between the given points.
    void ComputeSplines(int32 num_positions, const CVector *positions, const CVector *velocities, 
        array_obj<CSplineSequenceSegmentData> &segments_data);

    //computes all the splines given the time for each one.
    void ComputeSplines(const array_obj<CSplineSequenceSegmentData> &segments_data, bool wrap, 
        bool zero_start_vel, bool zero_end_vel);

};

//data that is stored for each segment of a sequence.
class CSplineSequenceSegmentData {
public:
    CSplineSequenceSegmentData();

//	void* operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

    void operator=(const CSplineSequenceSegmentData &other);

    void Save(FILE *file) const;
    void Load(FILE *file, int32 version);

    //the time it takes to travel down the segment.
    float time;

    //true if we have a start fov specified.
    bool have_start_fov;
    
    //true if we have an ending fov specified.
    bool have_end_fov;

    //the fov at the start of the segment.
    float start_fov;

    //the fov at the end of the segment.
    float end_fov;
    
    //true if we have a start timescale specified.
    bool have_start_timescale;
    
    //true if we have an ending timescale specified.
    bool have_end_timescale;

    //the timescale at the start of the segment.
    float start_timescale;

    //the timescale at the end of the segment.
    float end_timescale;
    
    //true if we have a start color specified.
    bool have_start_color;

    //true if we have an ending color specified.
    bool have_end_color;

    //the color at the start of the segment.
    float start_r;
    float start_g;
    float start_b;
    float start_a;

    //the color at the end of the segment.
    float end_r;
    float end_g;
    float end_b;
    float end_a;
};

//stores a string of splines all connected one after another.
class CSplineSequence {
  public:
    CSplineSequence();
    ~CSplineSequence();

//	void* CSplineSequence::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* CSplineSequence::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  CSplineSequence::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CSplineSequence::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

    void operator=(const CSplineSequence &other);

    void Init(const CVector &start, const CVector &dir);

    //wipes out all data.
    void Reset();

    //this command can only be run in the dll
    void Save(FILE *file);
    //can be run from the exe only
    void Load(FILE *file, int32 version);

  protected:
    //true if this is a position only spline sequence.
    bool position_only;

    //the number of points in the sequence.
    int32 num_points;

    //data for each sequence segment.
    array_obj<CSplineSequenceSegmentData> segment_data;

    //our position splines.
    CSplineSequenceComponent position;

    //our angles splines.
    CSplineSequenceComponent angle;

    //the velocity flags we use for beginning and end of the sequence.
    end_vel velocity_start, velocity_end;

    //the total time of the sequence.
    float total_time;

    //these values used to speed up the computation of segment number given a time.
    int32 cache_segment;
    float cache_time;

    //computes the total time of the sequence.
    void ComputeTime();

    //
    //private utility functions.
    //
    //worker function used by MakeCurveEntities
    void MakeCurveEntity(float time_start, float time_end, int32 segment_num, entity_t *reference_entity, entity_t *(*entity_alloc)());
	beamSeg_t *MakeCurveSegment (float time_start, float time_end, int32 segment_num, beam_t *beam);


    //
    //playback vars
    //

    //the number of the segment that we have already processed segment data for.
    int32 playback_last_segment;
  public:
    //
    //member access
    //
    int32 NumPoints() const;
    const CVector &Point(int32 index) const;
    const CVector &Angle(int32 index) const;
    end_vel VelStart() const;
    end_vel VelEnd() const;
    float Time(int32 segment) const;
    float Velocity(int32 pnt) const;
    float TotalTime() const;
    void Velocity(int32 pnt, CVector &vel) const;
    bool HaveStartFOV(int32 index) const;
    bool HaveEndFOV(int32 index) const;
    float StartFOV(int32 index) const;
    float EndFOV(int32 index) const;
    bool HaveStartTimescale(int32 index) const;
    bool HaveEndTimescale(int32 index) const;
    float StartTimescale(int32 index) const;
    float EndTimescale(int32 index) const;
    bool HaveStartColor(int32 index) const;
    bool HaveEndColor(int32 index) const;
    float StartR(int32 index) const;
    float StartG(int32 index) const;
    float StartB(int32 index) const;
    float StartA(int32 index) const;
    float EndR(int32 index) const;
    float EndG(int32 index) const;
    float EndB(int32 index) const;
    float EndA(int32 index) const;


    //
    //functions for modifying the sequence.
    //

    //adds a point to the sequence.  time is the time it takes to get from the previous point to this point.
    void Add(const CVector &loc, const CVector &dir, float time);
    void Insert(const int index, const CVector &loc, const CVector &dir, float time);
    //sets the time/angle/position for the given segment
    void Time(int32 segment, float time);
    void Angle(int32 index, const CVector &angle);
    void Pos(int32 index, const CVector &pos);
    //sets the start and end velocity.
    void VelStart(end_vel vel);
    void VelEnd(end_vel vel);
    void DeletePoint(int32 index);
    void HaveStartFOV(int32 index, bool flag);
    void HaveEndFOV(int32 index, bool flag);
    void StartFOV(int32 index, float degrees);
    void EndFOV(int32 index, float degrees);
    void HaveStartTimescale(int32 index, bool flag);
    void HaveEndTimescale(int32 index, bool flag);
    void StartTimescale(int32 index, float degrees);
    void EndTimescale(int32 index, float degrees);
    void HaveStartColor(int32 index, bool flag);
    void HaveEndColor(int32 index, bool flag);
    void StartColor(int32 index, float r, float g, float b, float a);
    void EndColor(int32 index, float r, float g, float b, float a);

    //
    //utility functions
    //

    //returns the segment that is active at the given time.  Modifies the given time to 
    //the offset into the the segment.
    int32 Segment(float &time);

    //
    //playback fuctions.
    //
    //gets the position on the spline at the given time.
    void Pos(float time, CVector &pos);
    void Dir(float time, CVector &dir);

    //gets the derivative of pos, normalized.
    void Vel(float time, CVector &vel);

    //processes the fov
    void PlaybackReset();
    void Playback(int32 sequence_time);

    //
    //functions for drawing a sequence with lasers.
    //
    //Initializes the position splines with the given array of positions and velocities.
    void InitPositions(int32 num_positions, const CVector *positions, const CVector *velocities);

    //Makes as many entities as are needed to represent the sequence position splines.
    //Uses the given function for allocating entities.  reference_entity is used
    //to initialize data memebers in the entity that are not actually needed by the function
    //to position the laser entities.  max_angle (in degrees) affects the smoothness of the curve by specifying
    //the maximum allowed angle between adjacent entities.  max_length specifies the maximum length of
    //any entity
    int32 MakeCurveEntities(entity_t *reference_entity, entity_t *(*entity_alloc)(), float max_angle, float max_length);
	int32 MakeCurveSegments (float max_angle, float max_length);
};

//various utility functions that can be used when constructing splines.

//returns the smooth direction a spline should take when passing through loc from before, towards after.
void SplineDirection(CVector &before, CVector &loc, CVector &after, CVector &dir, bool wrap);
void SplineStartDir(CVector &loc, CVector &after, CVector &after_dir, CVector &dir, bool wrap);
void SplineEndDir(CVector &before, CVector &loc, CVector &before_dir, CVector &dir, bool wrap);

//returns the reflection of a point through a line.  Line should be normalized.
void PointReflection(CVector &pnt, CVector &line, CVector &reflect);

//sets wrapped_end to an equivalent angle_end
void ComputeEndAngleWrap(CVector &angle_start, CVector &angle_end, CVector &wrapped_end);

#endif // _DK_GCE_SPLINE_H