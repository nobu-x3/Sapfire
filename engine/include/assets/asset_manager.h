#pragma once
#include "mesh_manager.h"
#include "texture_manager.h"

namespace Sapfire {
	class GraphicsDevice;
}

namespace Sapfire::assets {

	struct SFAPI AssetManagerCreationDesc {
		d3d::GraphicsDevice* device;
		stl::string mesh_registry_path{"mesh_registry.db"};
		stl::string texture_registry_path{"texture_registry.db"};
		stl::string material_registry_path{"material_registry.db"};
	};

	// Asset manager is a wrapper class for mesh, material and texture managers.
	class SFAPI AssetManager {
	public:
		explicit AssetManager(const AssetManagerCreationDesc& desc);
		AssetManager(const AssetManager&) = delete;
		AssetManager(AssetManager&&);
		AssetManager& operator=(const AssetManager&) = delete;
		AssetManager& operator=(AssetManager&&);
		void load_runtime_texture(const stl::string& path);
		inline void import_texture(const stl::string& path) { m_TextureRegistry.import_texture(m_Device, path); }
		inline void move_texture(const stl::string& old_path, const stl::string& new_path) {
			m_TextureRegistry.move_texture(m_Device, old_path, new_path);
		}
		inline void release_texture(const stl::string& path) { m_TextureRegistry.release_texture(path); };
		inline TextureAsset* get_texture(const stl::string& path) const { return m_TextureRegistry.get(path); }
		inline TextureAsset* get_texture(UUID uuid) const { return m_TextureRegistry.get(uuid); }
		inline stl::string get_texture_path(UUID uuid) const { return m_TextureRegistry.get_path(uuid); }
		inline TextureResource get_texture_resource(const stl::string& path) { return m_TextureManager.texture_resources[path]; }
		inline bool texture_resource_exists(const stl::string& path) { return m_TextureManager.texture_resources.contains(path); }
		inline const stl::unordered_map<stl::string, TextureAsset>& path_texture_map() const { return m_TextureRegistry.path_asset_map(); }
		inline void import_mesh(const stl::string& path) { return m_MeshRegistry.import_mesh(path); }
		inline void move_mesh(const stl::string& old_path, const stl::string& new_path) { m_MeshRegistry.move_mesh(old_path, new_path); }
		inline void release_mesh(const stl::string& path) { m_MeshRegistry.release_mesh(path); }
		inline MeshAsset* get_mesh(const stl::string& path) const { return m_MeshRegistry.get(path); }
		inline MeshAsset* get_mesh(UUID uuid) const { return m_MeshRegistry.get(uuid); }
		inline stl::string get_mesh_path(UUID uuid) { return m_MeshRegistry.get_path(uuid); }
		inline const stl::unordered_map<stl::string, MeshAsset>& path_mesh_map() const { return m_MeshRegistry.path_asset_map(); }
		inline void load_mesh_resource(const stl::string& path, MeshResource res) { m_MeshManager.mesh_resources[path] = res; }
		inline MeshResource get_mesh_resource(const stl::string& path) const { return m_MeshManager.mesh_resources.at(path); }
		void serialize();
		stl::string to_string();

	private:
		MeshRegistry m_MeshRegistry;
		MeshManager m_MeshManager;
		TextureRegistry m_TextureRegistry;
		TextureManager m_TextureManager;
		d3d::GraphicsDevice& m_Device;
	};
} // namespace Sapfire::assets