#pragma once

#include "defines.h"

typedef union vec2 {
  f32 elements[2];
  struct {
    union {
      f32 x, u;
    };
    union {
      f32 y, v;
    };
  };
} vec2;

typedef union vec3 {
  f32 elements[3];
  struct {
    union {
      f32 x, r, u;
    };
    union {
      f32 y, g, v;
    };
    union {
      f32 z, b, w;
    };
  };
} vec3;

typedef union vec4 {
#if defined(SIMD)
  alignas(16) __m128 data;
#endif
 ALIGN(16) f32 elements[4];
  struct {
    union {
      f32 x, r, s;
    };
    union {
      f32 y, g, t;
    };
    union {
      f32 z, b, p;
    };
    union {
      f32 w, a, q;
    };
  };
} vec4;

typedef vec4 quat;

typedef union mat4{
  f32 data[16];
} mat4;

typedef struct vertex{
  vec3 position;
} vertex;
