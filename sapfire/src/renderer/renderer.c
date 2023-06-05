#include "core/logger.h"
#include "core/sfmemory.h"
#include "defines.h"
#include "math/sfmath.h"
#include "renderer.h"
#include "renderer/renderer_provider.h"
#include "renderer/renderer_types.h"
#include "resources/resource_types.h"
// temp
#include "core/sfstring.h"
#define STB_IMAGE_IMPLEMENTATION
#include "resources/stb_image.h"

// temp
void prep_texture (texture *t) {
	sfmemset (t, 0, sizeof (texture));
	t->generation = INVALID_ID;
}

b8 load_texture (renderer *renderer, const char *name, texture *t) {
	stbi_set_flip_vertically_on_load (TRUE);
	const i32 channel_count = 4;
	char *fmt_str			= "assets/textures/%s";
	char path[512];
	sfstrfmt (path, fmt_str, name);
	texture temp;
	u8 *data	  = stbi_load (path, (i32 *)&temp.width, (i32 *)&temp.height,
							   (i32 *)&temp.channels, channel_count);
	temp.channels = channel_count;
	if (!data) {
		SF_ERROR ("Failed to load texture at path %s", path);
		if (stbi_failure_reason ()) {
			SF_WARNING ("load_texture() failed to load file '%s': %s", path,
						stbi_failure_reason ());
		}

		return FALSE;
	}
	u32 current_gen = t->generation;
	t->generation	= INVALID_ID;
	u64 total_size	= temp.width * temp.height * temp.channels;
	b8 opaque		= TRUE;
	for (u32 i = 0; i < total_size; i += channel_count) {
		if (data[i + 3] < 255) {
			opaque = FALSE;
			break;
		}
	}
	renderer_create_texture (renderer, name, temp.width, temp.height,
							 channel_count, opaque, data, &temp);
	texture old = *t;
	*t			= temp;
	renderer_destroy_texture (renderer, &old);
	if (current_gen == INVALID_ID) {
		t->generation = 0;
	} else {
		t->generation = current_gen + 1;
	}
	stbi_image_free (data);
	return TRUE;
}
b8 renderer_initialize (renderer *renderer, renderer_api api,
						const char *application_name,
						struct platform_state *plat_state) {
	renderer->renderer_provider =
		sfalloc (sizeof (renderer_provider), MEMORY_TAG_RENDERER);
	renderer->renderer_provider->default_diffuse = &renderer->default_texture;
	renderer_provider_create (api, plat_state, renderer->renderer_provider);
	// Initialize the renderer provider.
	if (!renderer->renderer_provider->initialize (
			renderer->renderer_provider, application_name, plat_state)) {
		SF_FATAL ("Could not initialize renderer provider.");
		return FALSE;
	}
	SF_TRACE ("Creating default texture...");
	const u32 dimensions  = 256;
	const u32 channels	  = 4;
	const u32 pixel_count = dimensions * dimensions;
	u8 pixels[pixel_count * channels];
	sfmemset (pixels, 255, sizeof (u8) * pixel_count * channels);
	/* for (u32 row = 0; row < dimensions; ++row) { */
	/* 	b8 black = TRUE; */
	/* 	for (u32 col = 0; col < dimensions; ++col) { */
	/* 		u32 index = ((row * dimensions) + col) * channels; */
	/* 		if (col % 64 == 0) { black = !black; } */
	/* 		if (!black) { */
	/* 			pixels[index]	  = 255; */
	/* 			pixels[index + 2] = 255; */
	/* 		} */
	/* 		pixels[index + 3] = 1; */
	/* 	} */
	/* } */
	for (u64 row = 0; row < dimensions; ++row) {
		for (u64 col = 0; col < dimensions; ++col) {
			u64 index	  = (row * dimensions) + col;
			u64 index_bpp = index * channels;
			if (row % 2) {
				if (col % 2) {
					pixels[index_bpp + 0] = 0;
					pixels[index_bpp + 1] = 0;
				}
			} else {
				if (!(col % 2)) {
					pixels[index_bpp + 0] = 0;
					pixels[index_bpp + 1] = 0;
				}
			}
		}
	}

	renderer_create_texture (renderer, "default", dimensions, dimensions,
							 channels, TRUE, pixels,
							 &renderer->default_texture);
	renderer->default_texture.generation = INVALID_ID;
	SF_TRACE ("Default texture created.");
	prep_texture (&renderer->test_diffuse);
	load_texture (renderer, "cobblestone.png", &renderer->test_diffuse);
	SF_TRACE ("%d", renderer->test_diffuse.data);
	return TRUE;
}

void renderer_shutdown (renderer *renderer) {
	renderer_destroy_texture (renderer, &renderer->default_texture);
	renderer_destroy_texture (renderer, &renderer->test_diffuse);
	renderer->renderer_provider->shutdown (renderer->renderer_provider);
	sffree (renderer->renderer_provider, sizeof (renderer_provider),
			MEMORY_TAG_RENDERER);
}

b8 renderer_draw_frame (renderer *renderer, render_bundle *bundle) {
	// TODO and NOTE: use the actual data.
	// Begin rendering the frame.
	if (renderer->renderer_provider->begin_frame (renderer->renderer_provider,
												  0)) {
		// TODO: render render render
		static f32 z = 1.0f;
		/* z += 0.01f; */
		mat4 projection =
			mat4_perspective (deg_to_rad (75.0f), 4.f / 3.f, 0.1f, 1000.f);
		mat4 view		 = mat4_translation ((vec3){0, 0, z});
		view			 = mat4_inverse (view);
		static f32 angle = 0.f;
		angle -= 0.001f;
		quat model_rot	 = quat_from_axis_angle (vec3_forward (), angle, FALSE);
		mat4 model		 = quat_to_mat4 (model_rot);
		mesh_data data	 = {};
		data.id			 = 0; // TODO: actual id
		data.model		 = model;
		data.textures[0] = &renderer->default_texture;

		renderer->renderer_provider->update_scene_data (projection, view);
		renderer->renderer_provider->update_objects_data (data);

		// End the renderer provider.
		if (!renderer->renderer_provider->end_frame (
				renderer->renderer_provider)) {
			SF_FATAL ("Could not end frame!");
			return FALSE;
		}
	}
	return TRUE;
}

void renderer_create_texture (renderer *renderer, const char *name, u32 width,
							  u32 height, u32 channels, b8 opaque,
							  const u8 *pixels, struct texture *out_texture) {
	if (renderer && renderer->renderer_provider &&
		renderer->renderer_provider->create_texture) {
		renderer->renderer_provider->create_texture (
			name, width, height, channels, opaque, pixels, out_texture);
	}
}
void renderer_destroy_texture (renderer *renderer, struct texture *texture) {
	if (renderer && renderer->renderer_provider &&
		renderer->renderer_provider->destroy_texture) {
		renderer->renderer_provider->destroy_texture (texture);
	}
}
