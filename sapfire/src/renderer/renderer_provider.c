#include "renderer_provider.h"
#include "core/logger.h"
#include "renderer/renderer_types.h"
#include "vulkan/vulkan_provider.h"

b8 renderer_provider_create (renderer_api api,
							 struct platform_state *plat_state,
							 renderer_provider *out_renderer_provider) {
	out_renderer_provider->plat_state = plat_state;
	switch (api) {
		// TODO: actual switch
		case RENDERER_API_VULKAN:
			out_renderer_provider->initialize		 = vulkan_initialize;
			out_renderer_provider->shutdown			 = vulkan_shutdown;
			out_renderer_provider->begin_frame		 = vulkan_begin_frame;
			out_renderer_provider->update_scene_data = vulkan_update_scene_data;
			out_renderer_provider->update_objects_data =
				vulkan_update_objects_data;
			out_renderer_provider->end_frame = vulkan_end_frame;
			return TRUE;
		default: SF_FATAL ("The rendering API is not supported"); return FALSE;
	}
	return FALSE;
}

void renderer_provider_shutdown (renderer_provider *provider) {
	provider->initialize		  = SF_NULL;
	provider->shutdown			  = SF_NULL;
	provider->begin_frame		  = SF_NULL;
	provider->update_scene_data	  = SF_NULL;
	provider->update_objects_data = SF_NULL;
	provider->end_frame			  = SF_NULL;
}
