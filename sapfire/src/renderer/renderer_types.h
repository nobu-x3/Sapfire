#pragma once

#include "defines.h"
#include "math/math_types.h"
#include "resources/resource_types.h"
#include <sys/stat.h>

typedef enum renderer_api {
	RENDERER_API_VULKAN,
	RENDERER_API_DIRECTX,
} renderer_api;

struct texture;

typedef struct scene_camera {
    mat4 projection;
    mat4 view;
} scene_camera;

typedef struct scene_uniform {
    scene_camera scene_camera;
	mat4 _padding1;
	mat4 _padding2;
} scene_uniform;

typedef struct mesh_uniform {
    vec4 diffuse_color;
    vec4 _padding1;
    vec4 _padding2;
    vec4 _padding3;
} mesh_uniform;

typedef struct mesh_data{
    u32 id;
    mat4 model;
    struct texture* textures[16];
} mesh_data;

typedef struct renderer_provider {
	struct platform_state* plat_state;
    struct texture* default_diffuse;
	b8 (*initialize) (struct renderer_provider* api, const char* app_name,
					  struct platform_state* plat_state);
	void (*shutdown) (struct renderer_provider* api);
	b8 (*begin_frame) (struct renderer_provider* api, f64 deltaTime);
	void (*update_scene_data) (mat4 projection, mat4 view);
	void (*update_objects_data) (mesh_data data);
	b8 (*end_frame) (struct renderer_provider* api);
    void (*create_texture)(const char* name, u32 width, u32 height, u32 channels, b8 opaque, const u8* pixels, struct texture* out_texture);
    void (*destroy_texture)(struct texture* texture);
} renderer_provider;

typedef struct render_bundle {
	f64 deltaTime;
} render_bundle;

typedef struct renderer {
	struct renderer_provider* renderer_provider;
	mat4 projection;
	mat4 view;
	f32 near_clip;
	f32 far_clip;
} renderer;
