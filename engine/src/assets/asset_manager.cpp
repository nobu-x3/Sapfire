#include "engpch.h"

#include "render/graphics_device.h"
#include "assets/asset_manager.h"

namespace Sapfire::assets {

	AssetManager::AssetManager(const AssetManagerCreationDesc& desc) :
		m_Device(*desc.device), m_MeshRegistry(desc.mesh_registry_path), m_TextureRegistry(desc.texture_registry_path) {}

	void AssetManager::load_runtime_texture(const stl::string& texture_path) {
		auto* texture = m_TextureRegistry.get(texture_path);
		if (!texture) {
			m_TextureRegistry.import_texture(m_Device, texture_path);
			texture = m_TextureRegistry.get(texture_path);
		}
		if (texture->data.dsv_index == d3d::INVALID_INDEX_U32 || texture->data.srv_index == d3d::INVALID_INDEX_U32) {
			texture->data = m_Device.create_texture(d3d::TextureCreationDesc{
				.usage = d3d::TextureUsage::TextureFromPath,
				.name = d3d::AnsiToWString(texture_path),
				.path = d3d::AnsiToWString(texture_path),
			});
		}
		if (texture) {
			assets::TextureResource text_res{
				.gpu_idx = texture->data.srv_index,
			};
			m_TextureManager.add(texture_path, texture->uuid, text_res);
		}
	}
} // namespace Sapfire::assets