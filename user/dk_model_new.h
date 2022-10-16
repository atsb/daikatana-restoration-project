#pragma once

class dk_model;
class dk_model_skin;
class dk_model_texture_coord;
class dk_model_triangle;
class dk_model_frame;
class dk_model_surface;

class dk_model_frame_decompressed;

typedef enum {
    MQL_8BIT,
    MQL_10BIT,
} model_quantization_level;

class dk_model {
  public:
    dk_model();
    ~dk_model();

//	void* operator new (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MODEL); }
//	void* operator new[] (size_t size) { return memmgr.X_Malloc(size,MEM_TAG_MODEL); }
//	void  operator delete (void* ptr) { memmgr.X_Free(ptr); }
//	void  operator delete[] (void* ptr) { memmgr.X_Free(ptr); }

    static dk_model *Load(const char *filename);

  protected:
    //the name of the model.
    buffer256 filename;

    //the resource usage type.
    resource_t resource_flag;

    //the names of the skins
    array_obj<dk_model_skin> skins;

    //array of st texture coordinate data.
    array_obj<dk_model_texture_coord> coord_st;

    //array of triangles.
    array_obj<dk_model_triangle> triangles;

    //the number of bits in each quantized point coordinate.
    model_quantization_level quantization;

    //the number of points in each frame.
    int32 num_points;

    //array of frames.
    array_obj<dk_model_frame> frames;

    //array of surface descriptions.
    array_obj<dk_model_surface> surfaces;

    //
    //  private functions.
    //
    //loads a model that is stored in version 1 format.
    void LoadV1(const byte *file_data);

  public:
    //
    //  member access
    //
    const char *FileName() const;
    int32 NumPoints() const;
    const array_obj<dk_model_triangle> &Triangles() const;

    //
    //  sorted array support
    //
    static int Compare(const dk_model **item1, const dk_model **item2);
    static int Find(const dk_model *item, const void *id);


    //
    //  for modifying the model.
    //
    void Flag(resource_t resource);
    void FileName(const char *filename);

    //
    //  for grabbing decompresed frames.
    //
    const dk_model_frame_decompressed *GetDecompressedFrame(int32 frame) const;
    const dk_model_frame_decompressed *GetInterpolatedFrame(int32 start_frame, int32 end_frame, float percent) const;
};

class dk_model_skin {
  public:
    dk_model_skin();
    ~dk_model_skin();

  protected:
    buffer64 name;

    //pointer to the actual graphic.
    void *skin;

  public:
    //
    //  for modifying the skin
    //
    void Name(const char *name);
    void LoadSkin();

};

//holder for either uv or st coordinates.
class dk_model_texture_coord {
  public:
    float x, y;
};

typedef short dk_model_triangle_texture_coord[3];

class dk_model_triangle {
  public:
    dk_model_triangle();
    ~dk_model_triangle();

  protected:
    //the surface used on this poly.
    short surface_index;

    //the points of the polygon.
    short points[3];

    //the array of texture point indices, one set for each uv frame
    array_obj<short[3]> texture_coords;

  public:
    //
    //  member access
    //
    const array_obj<dk_model_triangle_texture_coord> &TextureCoords() const;
    short Point0() const;
    short Point1() const;
    short Point2() const;

    //
    //  for modifying the triangle.
    //
    void SurfaceIndex(short index);
    void Points(short p1, short p2, short p3);
    array_obj<dk_model_triangle_texture_coord> &TextureCoords();

}; 

class dk_model_frame {
  public:
    dk_model_frame();
    ~dk_model_frame();

  protected:
    //the scale and offset used to re-create the points from the quantized values.
    point offset;
    point scale;

    //the name of the frame.
    buffer16 name;

    //the normals for each point in the model.  The stored value is an index into 
    //a static table of precomputed normals.
    array_obj<byte> point_normals;

    //the x, y, and z data is stored in packed bit arrays
    byte *x_data;
    byte *y_data;
    byte *z_data;

  public:
    //
    //  member access
    //
    const point &Scale() const;
    const point &Offset() const;
    const byte *XData() const;
    const byte *YData() const;
    const byte *ZData() const;
    const array_obj<byte> &PointNormals() const;

    //
    //  for modifying the frame.
    //
    void Name(const char *name);
    void Offset(float x, float y, float z);
    void Scale(float x, float y, float z);
    //reallocates the point and normals data
    void SetNumPoints(int32 num_points, model_quantization_level quantization);
    array_obj<byte> &PointNormals();
    byte *XData();
    byte *YData();
    byte *ZData();

};

class dk_model_frame_decompressed {
public:
    dk_model_frame_decompressed();
    ~dk_model_frame_decompressed();


protected:
    //array of non-quantized points.
    array_obj<point> points;

    //the frame used as the source for this decompressed frame, or
    //the closest thing we have to it.
    const dk_model_frame *source_frame;

    //the score for this frame, used for keeping track of which frames are used most frequently
    int32 score;

public:
    //
    //member access.
    //
    const array_obj<point> &Points() const;
    const dk_model_frame *SourceFrame() const;
    int32 Score() const;

    void Score(int32 score);

    //
    //for setting up the frame.
    //

    //to decompress a frame.
    void Init(const dk_model_frame *source, model_quantization_level quantization, int32 num_points);
    //to interpolate between two frames.
    void Interpolate(const dk_model_frame_decompressed *start, 
                     const dk_model_frame_decompressed *end, float percent);

    //
    //  sorted array support
    //
    static int Compare(const dk_model_frame_decompressed **item1, const dk_model_frame_decompressed **item2);
    static int Find(const dk_model_frame_decompressed *item, const void *id);


};

class dk_model_surface {
  public:
    dk_model_surface();
    ~dk_model_surface();

  protected:
    //the name 
    buffer32 name;

    //effect flags, SRF_*
    uint32 draw_flags;

    //index of skin used.
    int32 skin_index;

    //width and height of skin texture.
    int32 skin_width;
    int32 skin_height;

    //number of uv frames stored for polys using this surface.
    int32 num_uv_frames;

  public:
    //
    //  member access
    //
    const char *Name() const;
    uint32 DrawFlags() const;
    int32 Skin() const;
    int32 SkinWidth() const;
    int32 SkinHeight() const;
    int32 NumUVFrames() const;

    //
    //  for modifying the surface.
    //
    void Name(const char *name);
    void DrawFlags(uint32 flags);
    void Skin(int32 index);
    void SkinWidth(int32 width);
    void SkinHeight(int32 height);
    void NumUVFrames(int32 num_frames);
};

typedef enum {
	COMMAND_STRIP,
	COMMAND_FAN
} command_type_t;

class dk_command_element {
	public:
		dk_command_element();
		~dk_command_element();

	protected:
		point	xyz;										// 3d point
	
		union {												// s/u coordinate
			float	s;
			float	u;
		};

		union {												// t/v coordinate
			float	t;
			float	v;
		};

	public:
		// inline functions for Adam (hehe)
		float	S()	{ return s; };							// returns the s coordinate
		float	T()	{ return t; };							// returns the t coordinate
		float	U()	{ return u; };							// returns the u value
		float	V()	{ return v; };							// returns the v value
};

class dk_model_command {
	public:
		dk_model_command();
		~dk_model_command();

	protected:
		command_type_t	command_type;						// command type COMMAND_STRIP or COMMAND_FAN

		array_obj<dk_command_element> vertex;				// array of command elements (xyz & uv/st)

	public:
		command_type_t		CommandType();					// returns the command type (COMMAND_STRIP or COMMAND_FAN)
		dk_command_element	&Element( int element_num );	// returns a command element (xyz & uv/st)
};

class dk_model_commandlist {
	public:
		dk_model_commandlist();
		~dk_model_commandlist();

	protected:
		array_obj<dk_model_command> commands;				// command list

	public:
		dk_model_command	&Command( int command_num );	// returns a command
};
