#include "icons.h"

namespace icons {

	Sapfire::stl::unordered_map<Sapfire::stl::string, Sapfire::d3d::Texture> g_IconTextures{};
	Sapfire::stl::unordered_map<Sapfire::stl::string, ImTextureID> g_IconIDs{};

	const Sapfire::d3d::Texture& get(const Sapfire::stl::string& id) { return g_IconTextures[id]; }

	void add(Sapfire::d3d::GraphicsDevice& device, const Sapfire::stl::wstring& path, const Sapfire::stl::string& id) {
		if (!g_IconTextures.contains(id)) {
			g_IconTextures[id] = device.create_texture({
				.usage = Sapfire::d3d::TextureUsage::TextureFromPath,
                .optional_initial_state = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT,
                .name = path,
				.path = path,
			});
			g_IconIDs[id] = (ImTextureID)device.cbv_srv_uav_descriptor_heap()
								->descriptor_handle_from_index(g_IconTextures[id].srv_index)
								.gpu_descriptor_handle.ptr;
		}
	}

	ImTextureID get_im_id(const Sapfire::stl::string& id) { return g_IconIDs[id]; }
} // namespace icons
