#include "ref.h"

#include "dk_point.h"
#include "dk_buffer.h"
#include "dk_array.h"
#include "dk_model_new.h"

//list of all frames we have created.

const dk_model_frame_decompressed *dk_model::GetDecompressedFrame(int32 frame) const {
    static dk_model_frame_decompressed decompressed_frames[2];
    static int32 cur_frame = 0;
    cur_frame++;
    if (cur_frame > 1) cur_frame = 0;
    dk_model_frame_decompressed *decompressed_frame = &decompressed_frames[cur_frame];

    //get the frame.
    if (frame < 0 || frame >= frames.Num()) return NULL;
    const dk_model_frame &compressed_frame = frames.Item(frame);

    //initialize the frame.
    decompressed_frame->Init(&compressed_frame, quantization, num_points);

    //return the frame.
    return decompressed_frame;
}

const dk_model_frame_decompressed *dk_model::GetInterpolatedFrame(int32 start_frame_num, 
    int32 end_frame_num, float percent) const 
{
    if (percent < 0.0f || percent > 1.0f) return NULL;

    //get both frames.
    const dk_model_frame_decompressed *start_frame = GetDecompressedFrame(start_frame_num);
    const dk_model_frame_decompressed *end_frame = GetDecompressedFrame(end_frame_num);

    if (start_frame == NULL || end_frame == NULL) return NULL;

    //our static frame that we use for all interpolation.
    static dk_model_frame_decompressed interpolated_frame;

    //do the interpolation.
    interpolated_frame.Interpolate(start_frame, end_frame, percent);

    //return the interpolated frame.
    return &interpolated_frame;
}

void dk_model_frame_decompressed::Init(const dk_model_frame *frame, model_quantization_level quantization, int32 num_points) {
    if (frame == NULL) return;
    if (num_points < 0 || num_points > 10000) return;

    //set our source frame pointer.
    source_frame = frame;

    //set our array length.
    points.Length(num_points);

    //de-quantize the points.
    if (quantization == MQL_8BIT) {
        //grab the data buffers.
        const byte *x_data = frame->XData();
        const byte *y_data = frame->YData();
        const byte *z_data = frame->ZData();

        //get the scale and offset to use to dequantize the points.
        point scale = frame->Scale();
        point offset = frame->Offset();

        for (int32 i = 0; i < num_points; i++) {
            //get the point we are filling in.
            point &dest_point = points.Item(i);

            //compute x y and z for the point.
            dest_point.x = offset.x + scale.x * x_data[i];
            dest_point.y = offset.y + scale.y * y_data[i];
            dest_point.z = offset.z + scale.z * z_data[i];
        }
    }
    else if (quantization == MQL_10BIT) {

    }
}

void dk_model_frame_decompressed::Interpolate(const dk_model_frame_decompressed *start, 
    const dk_model_frame_decompressed *end, float percent)
{
    if (start == NULL || end == NULL) return;

    //get the 2 point arrays.
    const array_obj<point> &start_points = start->Points();
    const array_obj<point> &end_points = end->Points();

    //make sure the 2 given frames have the same number of points.
    if (start_points.Num() != end_points.Num()) return;

    //get the number of points we are interpolating.
    int32 num_points = end_points.Num();

    //set the length of our point array if it is not already correct.
    if (points.Num() != num_points) {
        //set the array length.
        points.Length(num_points);
    }

    //compute 1 - percent.
    float back_percent = 1 - percent;

    //interpolate each point.
    for (int32 i = 0; i < num_points; i++) {
        //get the destination point.
        point &dest_point = points.Item(i);

        //combine the 2 source points.
        dest_point.eq(start_points.Item(i), back_percent, end_points.Item(i), percent);
    }

    //set the interpolated frame's source
    source_frame = end->source_frame;
}





