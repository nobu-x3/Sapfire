#include "core/logger.h"
#include "core/sfmemory.h"
#include "renderer.h"
#include "renderer/renderer_provider.h"
#include "renderer/renderer_types.h"

b8 renderer_initialize(renderer *renderer, renderer_api api,
					   const char *application_name,
					   struct platform_state *plat_state) {
		renderer->renderer_provider =
			sfalloc(sizeof(renderer_provider), MEMORY_TAG_RENDERER);
		renderer_provider_create(api, plat_state, renderer->renderer_provider);
		if (!renderer->renderer_provider->initialize(
				renderer->renderer_provider, application_name, plat_state)) {
				SF_FATAL("Could not initialize renderer provider.");
				return FALSE;
		}
		return TRUE;
}

void renderer_shutdown(renderer *renderer) {
		renderer->renderer_provider->shutdown(renderer->renderer_provider);
		sffree(renderer->renderer_provider, sizeof(renderer_provider),
			   MEMORY_TAG_RENDERER);
}

b8 renderer_draw_frame(renderer *renderer, render_bundle *bundle) {
		// TODO and NOTE: use the actual data.
		if (renderer->renderer_provider->begin_frame(
				renderer->renderer_provider, 0)) {
				// TODO: render render render
				if (!renderer->renderer_provider->end_frame(
						renderer->renderer_provider)) {
						SF_FATAL("Could not end frame!");
						return FALSE;
				}
		}
		return TRUE;
}