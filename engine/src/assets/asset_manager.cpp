#include "engpch.h"

#include "assets/asset_manager.h"

namespace Sapfire::assets {

	AssetManager::AssetManager(const AssetManagerCreationDesc& desc) :
		m_Device(*desc.device), m_MeshRegistry(desc.mesh_registry_path), m_TextureRegistry(desc.texture_registry_path) {}

} // namespace Sapfire::assets