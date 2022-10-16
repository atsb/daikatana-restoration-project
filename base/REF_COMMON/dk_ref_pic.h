#pragma once

class rgb {
public:
    byte r;
    byte g;
    byte b;

    //16 bit color we copy to 565 RGB glide frame buffer.
    uint16 color16;

	//32 bit color
	uint32 color32;

    //computes color16.
    void Set(byte r, byte g, byte b);
};

class CPalette : public object_reference {
  public:
    CPalette();
    ~CPalette();

	void* CPalette::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_IMAGE); }
	void* CPalette::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_IMAGE); }
	void  CPalette::operator delete (void* ptr) { memmgr.X_Free(ptr); }
	void  CPalette::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

    //takes 768 bytes of r-g-b triplets.
    void Init(const byte *raw_data);

    rgb colors[256];
};

//for storing 8-bit graphic images.
class CPic : public object_reference {
public:
    CPic();
    ~CPic();

	void* CPic::operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_IMAGE); }
	void* CPic::operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_IMAGE); }
	void  CPic::operator delete (void* ptr) { memmgr.X_Free(ptr); }
	void  CPic::operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

    static CPic *LoadPCX(const char *filename);
    static CPic *LoadBMP(const char *filename);

protected:
    //width and height.
    int width;
    int height;

    //the actual data.
    byte *data;

    //the palette loaded with the pic.
    pointer<CPalette> palette;

    //usage flag.
    resource_t resource_flag;

    //the file name of the image, with full path and extension (relative to daikatana's data directory)
    buffer256 filename;

public:
    //
    //  sorted array support
    //
    static int Compare(const CPic **item1, const CPic **item2);
    static int Find(const CPic *item, const void *id);

    //
    //  member access
    //
    const char *FileName() const;
    const byte *Data() const;
    int Width() const;
    int Height() const;
    const CPalette *Palette() const;
    resource_t Flag() const;

    //
    //  for modifying the pic.
    //
    void Flag(resource_t resource_flag);
};

//loads either a pic or skin.  
const CPic *LoadPic(const char *name, resource_t resource_flags);



