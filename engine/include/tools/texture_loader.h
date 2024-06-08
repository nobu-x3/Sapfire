#pragma once

namespace Sapfire::tools::texture_loader {
    void* load(const char* path, s32& width, s32& height, s32 compontent_count);
    f32* load_hdr(const char* path, s32& width, s32& height, s32 component_count);
}
