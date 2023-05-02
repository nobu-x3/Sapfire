#pragma once

#include "defines.h"

typedef enum renderer_api{
  RENDERER_API_VULKAN,
  RENDERER_API_DIRECTX,
} renderer_api;

typedef struct renderer_provider{
  struct platform_state* plat_state;
  b8(*initialize)(struct renderer_provider* api, const char* app_name, struct platform_state* plat_state);
  void(*shutdown)(struct renderer_provider* api);
  b8(*begin_frame)(struct renderer_provider* api, f64 deltaTime);
  b8(*end_frame)(struct renderer_provider* api);
} renderer_provider;

typedef struct render_bundle{
  f64 deltaTime;
} render_bundle;

typedef struct renderer{
  struct renderer_provider* renderer_provider;
} renderer;
