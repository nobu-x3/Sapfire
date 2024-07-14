#pragma once
#include "core/file_system.h"
#include "material_manager.h"
#include "mesh_manager.h"
#include "texture_manager.h"

namespace Sapfire {
	class GraphicsDevice;
}

namespace Sapfire::assets {

	struct SFAPI AssetManagerCreationDesc {
		d3d::GraphicsDevice* device{nullptr};
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
		void load_all_runtime_textures();
		bool is_texture_loaded_for_runtime(UUID uuid);
		inline void import_texture(const stl::string& path) { m_TextureRegistry.import_texture(m_Device, fs::relative_path(path)); }
		inline void move_texture(const stl::string& old_path, const stl::string& new_path) {
			m_TextureRegistry.move_texture(m_Device, old_path, new_path);
		}
		inline void release_texture(const stl::string& path) { m_TextureRegistry.release_texture(path); };
		inline TextureAsset* get_texture(const stl::string& path) const { return m_TextureRegistry.get(fs::relative_path(path)); }
		inline TextureAsset* get_texture(UUID uuid) const { return m_TextureRegistry.get(uuid); }
		inline stl::string get_texture_path(UUID uuid) const { return m_TextureRegistry.get_path(uuid); }
		inline TextureResource get_texture_resource(const stl::string& path) {
			return m_TextureManager.texture_resources[fs::relative_path(path)];
		}
		inline bool texture_resource_exists(const stl::string& path) {
			return m_TextureManager.texture_resources.contains(fs::relative_path(path));
		}
		inline const stl::unordered_map<stl::string, TextureAsset>& path_texture_map() const { return m_TextureRegistry.path_asset_map(); }
		inline void import_mesh(const stl::string& path) { return m_MeshRegistry.import_mesh(fs::relative_path(path)); }
		inline void import_mesh(const stl::string& path, UUID uuid) { return m_MeshRegistry.import_mesh(fs::relative_path(path), uuid); }
		inline void move_mesh(const stl::string& old_path, const stl::string& new_path) {
			m_MeshRegistry.move_mesh(fs::relative_path(old_path), fs::relative_path(new_path));
		}
		inline void release_mesh(const stl::string& path) { m_MeshRegistry.release_mesh(fs::relative_path(path)); }
		inline MeshAsset* get_mesh(const stl::string& path) const { return m_MeshRegistry.get(fs::relative_path(path)); }
		inline MeshAsset* get_mesh(UUID uuid) const { return m_MeshRegistry.get(uuid); }
		inline stl::string get_mesh_path(UUID uuid) { return m_MeshRegistry.get_path(uuid); }
		inline const stl::unordered_map<stl::string, MeshAsset>& path_mesh_map() const { return m_MeshRegistry.path_asset_map(); }
		inline void load_mesh_resource(const stl::string& path, MeshResource res) { m_MeshManager.mesh_resources[path] = res; }
		inline bool mesh_resource_exists(const stl::string& path) const { return m_MeshManager.mesh_resources.contains(path); }
		inline MeshResource get_mesh_resource(const stl::string& path) const { return m_MeshManager.mesh_resources.at(path); }
		inline MeshResource get_mesh_resource(UUID uuid) const {
			auto path = m_MeshRegistry.get_path(uuid);
			return m_MeshManager.mesh_resources.at(path);
		}
		inline void import_material(const stl::string& path) {
			if (path.empty())
				return;
			auto relative_path = fs::relative_path(path);
			m_MaterialRegistry.import_material(m_Device, relative_path);
			m_MaterialManager.add(relative_path, m_MaterialRegistry.get(relative_path)->uuid,
								  {.gpu_idx = static_cast<u32>(m_MaterialRegistry.get(relative_path)->material.material_cb_index)});
		}
		inline void import_material(const stl::string& path, MaterialAsset&& material) {
			if (path.empty()) {
				return;
			}
			auto relative_path = fs::relative_path(path);
			m_MaterialRegistry.import_material(m_Device, std::move(material), relative_path);
			m_MaterialManager.add(relative_path, m_MaterialRegistry.get(relative_path)->uuid,
								  {.gpu_idx = static_cast<u32>(m_MaterialRegistry.get(relative_path)->material.material_cb_index)});
		}
		inline void move_material(const stl::string& old_path, const stl::string& new_path) {
			m_MaterialRegistry.move_material(m_Device, old_path, new_path);
		}
		inline void release_material(const stl::string& path) { m_MaterialRegistry.release_material(path); };
		inline MaterialAsset* get_material(const stl::string& path) const { return m_MaterialRegistry.get(fs::relative_path(path)); }
		inline MaterialAsset* get_material(UUID uuid) const { return m_MaterialRegistry.get(uuid); }
		inline stl::string get_material_path(UUID uuid) const { return m_MaterialRegistry.get_path(uuid); }
		inline MaterialResource get_material_resource(const stl::string& path) {
			return m_MaterialManager.material_resources[fs::relative_path(path)];
		}
		inline bool material_resource_exists(const stl::string& path) const {
			return m_MaterialManager.material_resources.contains(fs::relative_path(path));
		}

		inline bool material_resource_exists(const UUID& uuid) const {
			if (uuid == MaterialRegistry::default_material(&m_Device)->uuid)
				return true;
			return m_MaterialManager.uuid_to_path_map.contains(uuid);
		}
		inline stl::unordered_map<stl::string, MaterialAsset>& path_material_map() { return m_MaterialRegistry.path_asset_map(); }
		void serialize();
		void serialize(const MaterialAsset& asset);
		void deserialize(const stl::string& data);
		stl::string to_string();

	private:
		MeshRegistry m_MeshRegistry;
		MeshManager m_MeshManager;
		TextureRegistry m_TextureRegistry;
		TextureManager m_TextureManager;
		MaterialRegistry m_MaterialRegistry;
		MaterialManager m_MaterialManager;
		d3d::GraphicsDevice& m_Device;
	};
} // namespace Sapfire::assets
