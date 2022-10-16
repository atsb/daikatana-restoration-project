#pragma once

//for storing 8-bit graphic images.
class CWal : public object_reference {
public:
    CWal();
    ~CWal();

	void* CWal::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_IMAGE); }
	void* CWal::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_IMAGE); }
	void  CWal::operator delete (void* ptr) { memmgr.X_Free(ptr); }
	void  CWal::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

	static CWal *Load(const char *name);

protected:
    //width and height.
    int width;
    int height;

    //the actual data.
    byte *data[4];

    //the palette loaded with the wal.
    pointer<CPalette> palette;

    //usage flag.
    resource_t resource_flag;

    //the file name of the image, with full path and extension (relative to daikatana's data directory)
    buffer256 filename;

public:
    //
    //  sorted array support
    //
    static int Compare(const CWal **item1, const CWal **item2);
    static int Find(const CWal *item, const void *id);

    //
    //  member access
    //
    const char *FileName() const;
    const byte *Data() const;
    const byte *Data( int miplevel ) const;
    int Width() const;
    int Height() const;
    const CPalette *Palette() const;
    resource_t Flag() const;

    //
    //  for modifying the wal.
    //
    void Flag(resource_t resource_flag);
};

//loads either a wal or skin.  
const CWal *LoadWal(const char *name, resource_t resource_flags);



