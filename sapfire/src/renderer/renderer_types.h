#pragma once

#include "defines.h"
#include "math/math_types.h"

typedef enum renderer_api{
  RENDERER_API_VULKAN,
  RENDERER_API_DIRECTX,
} renderer_api;

typedef struct scene_data{
  mat4 projection;
  mat4 view;
  mat4 _padding1;
  mat4 _padding2;
} scene_data;

typedef struct renderer_provider{
  struct platform_state* plat_state;
  b8(*initialize)(struct renderer_provider* api, const char* app_name, struct platform_state* plat_state);
  void(*shutdown)(struct renderer_provider* api);
  b8(*begin_frame)(struct renderer_provider* api, f64 deltaTime);
  void(*update_scene_data)(mat4 projection, mat4 view);
  b8(*end_frame)(struct renderer_provider* api);
} renderer_provider;

typedef struct render_bundle{
  f64 deltaTime;
} render_bundle;

typedef struct renderer{
  struct renderer_provider* renderer_provider;
} renderer;
