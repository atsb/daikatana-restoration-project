#include "dk_shared.h"
#include "ref.h"
#include "dk_ref_common.h"

#include "dk_misc.h"
#include "dk_point.h"

#include "dk_buffer.h"
#include "dk_array.h"
#include "dk_model_new.h"

//these types are used to help read in the old (version 1) model format from disk.
#define IDV1ALIASHEADER	(('D'<<24)+('M'<<16)+('K'<<8)+'D')

typedef struct {
    char name[64];
} dkm_v1_skin;

typedef struct {
    float s;
    float t;
} dkm_v1_st;

typedef struct {
    short st_index[3];

    byte next;
} dkm_v1_st_array;

typedef struct {
    short surface_index;
    short num_uv_frames;
    short index_xyz[3];
    dkm_v1_st_array texture_st;
} dkm_v1_triangle;

typedef struct {
    byte x;
    byte y;
    byte z;
    byte normal;

    byte next;
} dkm_v1_frame_point_array;

typedef struct {
    vec3_t scale;
    vec3_t offset;
    char name[16];
    dkm_v1_frame_point_array first_point;
} dkm_v1_frame;

typedef struct {

} dkm_v1_gl_command;

typedef struct {
    char name[32];
    uint32 draw_flags;
    int32 skin_index;
    int32 skin_width;
    int32 skin_height;
    int32 num_uv_frames;

    byte next;
} dkm_v1_surface;

typedef struct {
    int file_id;
    int version_num;
    vec3_t origin;
    int framesize;
    int num_skins;
    int num_xyz;
    int num_st;
    int num_tris;
    int gl_execute_buffer_len;
    int num_frames;
    int num_surfaces;

    int offset_skins;
    int offset_st;
    int offset_tris;
    int offset_frames;
    int offset_gl_buffer;
    int offset_surfaces;
    int offset_end;
} dkm_v1_header;

typedef struct {
    //main header read from disk.
    const dkm_v1_header *header;

    //pointers to the individual file blocks.
    const dkm_v1_skin *skins;
    const dkm_v1_st *st_coords;
    const dkm_v1_triangle *first_triangle;
    const dkm_v1_frame *first_frame;
    const dkm_v1_gl_command *first_command;
    const dkm_v1_surface *first_surface;
} dkm_v1_file_struct;



void ReadV1Skins(const dkm_v1_file_struct &file_struct, array_obj<dk_model_skin> &skins) {
    //get the number of skins.
    int num_skins = file_struct.header->num_skins;
    if (num_skins < 1 || num_skins > 100) return;

    //make the skin array the correct length.
    skins.Length(num_skins);

    //save each skin.
    for (int i = 0; i < num_skins; i++) {
        //get the skin.
        dk_model_skin &skin = skins.Item(i);

        skin.Name(file_struct.skins[i].name);
    }
}

void ReadV1TextureCoords(const dkm_v1_file_struct &file_struct, array_obj<dk_model_texture_coord> &coord_st) {
    //get the number of coordinates.
    int num_coords = file_struct.header->num_st;
    if (num_coords < 1 || num_coords > 10000) return;

    //make our array the correct length.
    coord_st.Length(num_coords);

    //fill in the coordinate values.
    for (int i = 0; i < num_coords; i++) {
        dk_model_texture_coord &coord = coord_st.Item(i);

        coord.x = file_struct.st_coords[i].s;
        coord.y = file_struct.st_coords[i].t;
    }
}

void ReadV1Triangles(const dkm_v1_file_struct &file_struct, array_obj<dk_model_triangle> &triangles) {
    //get the number of triangles.
    int num_triangles = file_struct.header->num_tris;
    if (num_triangles < 1 || num_triangles > 10000) return;

    //make our array the correct length.
    triangles.Length(num_triangles);

    //get a pointer to the beginning of the first triangle.
    const dkm_v1_triangle *triangle_in = file_struct.first_triangle;

    //read the triangles.
    for (int i = 0; i < num_triangles; i++) {
        //get the triangle structure from the array.
        dk_model_triangle &triangle_out = triangles.Item(i);

        //copy the base triangle data over.
        triangle_out.SurfaceIndex(triangle_in->surface_index);
        triangle_out.Points(triangle_in->index_xyz[0], triangle_in->index_xyz[1], triangle_in->index_xyz[2]);

        //get the number of uv texture point frames we have.
        int num_uv_frames = triangle_in->num_uv_frames;
        if (num_uv_frames < 1 || num_uv_frames > 1000) return;

        //get the array of texture points from the triangle.
        array_obj<dk_model_triangle_texture_coord> &texture_points = triangle_out.TextureCoords();

        //set the length of the texture point array.
        texture_points.Length(num_uv_frames);

        //get a pointer to the first frame of uv data.
        const dkm_v1_st_array *texture_point = &triangle_in->texture_st;

        //read in all the texture points.
        for (int t = 0; t < num_uv_frames; t++) {
            //get the dest array of indices.
            dk_model_triangle_texture_coord &dest = texture_points.Item(t);

            //copy the data.
            dest[0] = texture_point->st_index[0];
            dest[1] = texture_point->st_index[1];
            dest[2] = texture_point->st_index[2];

            //get a pointer to the next input structure.
            texture_point = (const dkm_v1_st_array *)&texture_point->next;
        }

        //get a pointer to the next triangle.  the next triangle starts right after the end
        //of the last texture coordinate read, what would have been the next texture point.
        triangle_in = (const dkm_v1_triangle *)texture_point;
    }
}

void ReadV1Frames(const dkm_v1_file_struct &file_struct, array_obj<dk_model_frame> &frames) {
    //get the number of frames.
    int num_frames = file_struct.header->num_frames;
    if (num_frames < 1 || num_frames > 10000) return;

    //get the number of points in each frame.
    int num_points = file_struct.header->num_xyz;
    if (num_points < 1 || num_points > 10000) return;

    //set the array length.
    frames.Length(num_frames);

    //get a pointer to the first frame
    const dkm_v1_frame *frame_in = file_struct.first_frame;

    //read in all the frames.
    for (int32 i = 0; i < num_frames; i++) {
        //get the frame we are filling in.
        dk_model_frame &frame_out = frames.Item(i);

        //copy the base data over.
        frame_out.Name(frame_in->name);
        frame_out.Scale(frame_in->scale[0], frame_in->scale[1], frame_in->scale[2]);
        frame_out.Offset(frame_in->offset[0], frame_in->offset[1], frame_in->offset[2]);

        //tell the frame how many poits there are.
        frame_out.SetNumPoints(num_points, MQL_8BIT);

        //get the normal array.
        array_obj<byte> &normals = frame_out.PointNormals();
        //get the point data arrays.
        byte *x_data = frame_out.XData();
        byte *y_data = frame_out.YData();
        byte *z_data = frame_out.ZData();

        //get a pointer to the first point.
        const dkm_v1_frame_point_array *point_in = &frame_in->first_point;

        //read in all the points.
        for (int32 p = 0; p < num_points; p++) {
            //copy the data over.
            normals.Item(p) = point_in->normal;
            x_data[p] = point_in->x;
            y_data[p] = point_in->y;
            z_data[p] = point_in->z;

            //get the next point.
            point_in = (const dkm_v1_frame_point_array *)&point_in->next;
        }

        //get a pointer to the next frame.  the next frame starts right after the end
        //of the last point read, what would have been the next point.
        frame_in = (const dkm_v1_frame *)point_in;
    }
}

void ReadV1Surfaces(const dkm_v1_file_struct &file_struct, array_obj<dk_model_surface> &surfaces) {
    //get the number of surfaces.
    int num_surfaces = file_struct.header->num_surfaces;
    if (num_surfaces < 1 || num_surfaces > 1000) return;

    //set the array length.
    surfaces.Length(num_surfaces);

    //get a pointer to the first surface.
    const dkm_v1_surface *surface_in = file_struct.first_surface;

    //read the data for all the surfaces.
    for (int i = 0; i < num_surfaces; i++) {
        //get the surface object we are filling in.
        dk_model_surface &surface_out = surfaces.Item(i);

        //fill in the data.
        surface_out.Name(surface_in->name);
        surface_out.DrawFlags(surface_in->draw_flags);
        surface_out.Skin(surface_in->skin_index);
        surface_out.SkinWidth(surface_in->skin_width);
        surface_out.SkinHeight(surface_in->skin_height);
        surface_out.NumUVFrames(surface_in->num_uv_frames);

        //get the next surface.
        surface_in = (const dkm_v1_surface *)&surface_in->next;
    }
}

void dk_model::LoadV1(const byte *file_data) {
    try {
        if (file_data == NULL) return;
    
        //the file structure.
        dkm_v1_file_struct file;

        //cast the data to our header type.
        file.header = (const dkm_v1_header *)file_data;

        //check the file id.
        if (file.header->file_id != IDV1ALIASHEADER) return;
        //check the version number.
        if (file.header->version_num != 1) return;

        //get pointers to the different blocks from the offsets
        file.skins = (const dkm_v1_skin *)&file_data[file.header->offset_skins];
        file.st_coords = (const dkm_v1_st *)&file_data[file.header->offset_st];
        file.first_triangle = (const dkm_v1_triangle *)&file_data[file.header->offset_tris];
        file.first_frame = (const dkm_v1_frame *)&file_data[file.header->offset_frames];
        file.first_command = (const dkm_v1_gl_command *)&file_data[file.header->offset_gl_buffer];
        file.first_surface = (const dkm_v1_surface *)&file_data[file.header->offset_surfaces];

        //parse the skins.
        ReadV1Skins(file, skins);

        //parse the st coordinates.
        ReadV1TextureCoords(file, coord_st);
        
        //parse the triangle data.
        ReadV1Triangles(file, triangles);

        //read the frame data.
        ReadV1Frames(file, frames);
        
        //read the gl commands.

        //read the surface information.
        ReadV1Surfaces(file, surfaces);

        //set the quantization level.
        quantization = MQL_8BIT;

        //set the number of points.
        num_points = file.header->num_xyz;
    }
    catch (...) {
        //we had some kind of error loading the file.
    }
}




dk_model_triangle::dk_model_triangle() {
    
}

dk_model_triangle::~dk_model_triangle() {
    
}

void dk_model_triangle::SurfaceIndex(short index) {
    surface_index = index;
}

void dk_model_triangle::Points(short p1, short p2, short p3) {
    points[0] = p1;
    points[1] = p2;
    points[2] = p3;
}

array_obj<dk_model_triangle_texture_coord> &dk_model_triangle::TextureCoords() {
    return texture_coords;
}

short dk_model_triangle::Point0() const {
    return points[0];
}

short dk_model_triangle::Point1() const {
    return points[1];
}

short dk_model_triangle::Point2() const {
    return points[2];
}






dk_model_surface::dk_model_surface() {
    
}

dk_model_surface::~dk_model_surface() {
    
}

void dk_model_surface::Name(const char *name) {
    this->name = name;
}

void dk_model_surface::DrawFlags(uint32 flags) {
    draw_flags = flags;
}

void dk_model_surface::Skin(int32 index) {
    skin_index = index;
}

void dk_model_surface::SkinWidth(int32 width) {
    skin_width = width;
}

void dk_model_surface::SkinHeight(int32 height) {
    skin_height = height;
}

void dk_model_surface::NumUVFrames(int32 num_frames) {
    num_uv_frames = num_frames;
}




dk_model_frame::dk_model_frame() {
    x_data = NULL;
    y_data = NULL;
    z_data = NULL;
}

dk_model_frame::~dk_model_frame() {
    //free our point data.
    delca(x_data);
    delca(y_data);
    delca(z_data);
}

const point &dk_model_frame::Scale() const{
    return scale;
}

const point &dk_model_frame::Offset() const{
    return offset;
}

const byte *dk_model_frame::XData() const{
    return x_data;
}

const byte *dk_model_frame::YData() const{
    return y_data;
}

const byte *dk_model_frame::ZData() const{
    return z_data;
}

void dk_model_frame::Name(const char *name) {
    this->name = name;
}

void dk_model_frame::Offset(float x, float y, float z) {
    offset.eq(x, y, z);
}

void dk_model_frame::Scale(float x, float y, float z) {
    scale.eq(x, y, z);
}

array_obj<byte> &dk_model_frame::PointNormals() {
    return point_normals;
}

void dk_model_frame::SetNumPoints(int num_points, model_quantization_level quantization) {
    if (num_points < 1 || num_points > 10000) return;

    //set the length of the point normals array.
    point_normals.Length(num_points);

    //delete our old point data arrays.
    delca(x_data);
    delca(y_data);
    delca(z_data);

    //compute the size of the point data arrays required for the given number of points at the given quantization.
    int32 data_array_len;

    if (quantization == MQL_8BIT) {
        data_array_len = num_points;
    }   
    else if (quantization == MQL_10BIT) {
        data_array_len = (num_points * 10 + 7) / 8;
    }
    else {
        return;
    }

    //allocate the point data arrays.
    x_data = new byte[data_array_len];
    y_data = new byte[data_array_len];
    z_data = new byte[data_array_len];
}

byte *dk_model_frame::XData() {
    return x_data;
}

byte *dk_model_frame::YData() {
    return y_data;
}

byte *dk_model_frame::ZData() {
    return z_data;
}

const array_obj<byte> &dk_model_frame::PointNormals() const {
    return point_normals;
}




dk_model_skin::dk_model_skin() {
    skin = NULL;
}

dk_model_skin::~dk_model_skin() {

}

void dk_model_skin::Name(const char *name) {
    this->name = name;
}




dk_model::dk_model() {
    quantization = MQL_8BIT;
    resource_flag = RESOURCE_GLOBAL;
}

dk_model::~dk_model() {

}

int dk_model::Compare(const dk_model **item1, const dk_model **item2) {
    return stricmp((*item1)->FileName(), (*item2)->FileName());
}

int dk_model::Find(const dk_model *item, const void *id) {
    //the id is a character string.
    const char *name = (const char *)id;

    //check if the models's name is equal to the passed in name.
    return stricmp(name, item->FileName());
}

void dk_model::Flag(resource_t resource) {
    resource_flag = resource;
}

const char *dk_model::FileName() const {
    return filename;
}

int32 dk_model::NumPoints() const {
    return num_points;
}

const array_obj<dk_model_triangle> &dk_model::Triangles() const {
    return triangles;
}

void dk_model::FileName(const char *filename) {
    this->filename = filename;
}



dk_model_frame_decompressed::dk_model_frame_decompressed() {
    source_frame = NULL;
    score = 0;
}

dk_model_frame_decompressed::~dk_model_frame_decompressed() {

}

const array_obj<point> &dk_model_frame_decompressed::Points() const {
    return points;    
}

const dk_model_frame *dk_model_frame_decompressed::SourceFrame() const {
    return source_frame;
}

int32 dk_model_frame_decompressed::Score() const {
    return score;
}

void dk_model_frame_decompressed::Score(int32 score) {

}

int dk_model_frame_decompressed::Compare(const dk_model_frame_decompressed **item1, const dk_model_frame_decompressed **item2) {
    return 0;
}

int dk_model_frame_decompressed::Find(const dk_model_frame_decompressed *item, const void *id) {
    return 0;
}





