#pragma once

#include "math/math_types.h"

typedef struct texture {
    u32 id;
    u32 width;
    u32 height;
    u8 channels;
    b8 opaque;
    void* data;
} texture;
