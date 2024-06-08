#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 1
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

#pragma pack_matrix(row_major)

// Type Aliases
typedef int s32;
typedef uint u32;

struct PerDrawConstants
{
    u32 pos_buffer_index;
    u32 normal_buffer_index;
    u32 tangent_buffer_index;
    u32 uv_buffer_index;
    u32 scene_data_buffer_index;
    u32 pass_data_buffer_index;
    u32 material_data_buffer_index;
    u32 texture_data_buffer_index;
};
#endif // __COMMON_HLSLI__
