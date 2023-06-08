#include "core/logger.h"
#include "core/sfmemory.h"
#include "math/sfmath.h"
#include "renderer.h"
#include "renderer/renderer_provider.h"
#include "renderer/renderer_types.h"

b8 renderer_initialize (renderer *renderer, renderer_api api,
						const char *application_name,
						struct platform_state *plat_state) {
	renderer->renderer_provider =
		sfalloc (sizeof (renderer_provider), MEMORY_TAG_RENDERER);
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
		angle += 0.001f;
		quat rotation = quat_from_axis_angle (vec3_forward (), angle, false);
		mat4 model	  = quat_to_rotation_matrix (rotation, vec3_zero ());
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
