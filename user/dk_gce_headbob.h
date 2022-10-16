//#pragma once

#ifndef _DK_GCE_HEADBOB_H
#define _DK_GCE_HEADBOB_H

class CScriptEntity;
class CEntityHeadScript;
class CSplineSequenceComponent;

//the gap (in milliseconds) between samples
#define HEADSCRIPT_RESOLUTION 200

class CEntityHeadScript {
  public:
    CEntityHeadScript();
    ~CEntityHeadScript();

//	void* CEntityHeadScript::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void* CEntityHeadScript::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MISC); }
//	void  CEntityHeadScript::operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  CEntityHeadScript::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

    void Reset();

    void operator=(const CEntityHeadScript &other);

    void Save(FILE *file) const;
    void Load(FILE *file, int version);

  protected:
    //the spline sequence that stores our relative angles.
    CSplineSequenceComponent angle;

  public:
    //
    //member access
    //
    void Angle(float time, CVector &angle) const;
    float Length() const;

    //
    //for modifying the movement sequence.
    //

    //adds a new set of angles to the sequence, assumed to be .2 seconds after the last position.
    void Add(const CVector &angles);
};

#endif // _DK_GCE_HEADBOB_H
