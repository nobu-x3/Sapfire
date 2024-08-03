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
	renderer->near_clip = 0.1f;
	renderer->far_clip	= 1000.0f;
	renderer->projection =
		mat4_perspective (deg_to_rad (45.0f), 800 / 600.0f, renderer->near_clip,
						  renderer->far_clip);

	renderer->view = mat4_translation ((vec3){0, 0, 30.0f});
	renderer->view = mat4_inverse (renderer->view);
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
		renderer->renderer_provider->update_scene_data (renderer->projection,
														renderer->view);
		static f32 angle = 0.01f;
		quat rotation	 = quat_from_axis_angle (vec3_forward (), angle, false);
		mat4 model		 = quat_to_rotation_matrix (rotation, vec3_zero ());
		renderer->renderer_provider->update_objects_data (model);
		// End the renderer provider.
		if (!renderer->renderer_provider->end_frame (
				renderer->renderer_provider)) {
			SF_FATAL ("Could not end frame!");
			return FALSE;
		}
	}
	return TRUE;
}

void renderer_set_view (renderer *renderer, mat4 view) {
	renderer->view = view;
}
