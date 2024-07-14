#pragma once

#include "core/uuid.h"
#include "render/material.h"

namespace Sapfire {
	namespace d3d {
		class GraphicsDevice;
	}
} // namespace Sapfire

namespace Sapfire::assets {

	struct SFAPI MaterialAsset {
		UUID uuid;
		d3d::Material material;
	};

	struct SFAPI MaterialResource {
		u32 cpu_idx{0};
		u32 gpu_idx{0};
	};

	struct SFAPI MaterialManager {
		Sapfire::stl::unordered_map<Sapfire::stl::string, MaterialResource> material_resources;
		Sapfire::stl::unordered_map<Sapfire::UUID, Sapfire::stl::string> uuid_to_path_map;
		void add(const Sapfire::stl::string& path, Sapfire::UUID uuid, MaterialResource resource);
	};

	class SFAPI MaterialRegistry {
	public:
		explicit MaterialRegistry(const stl::string& registry_file_path);
		explicit MaterialRegistry() = default;
		~MaterialRegistry() = default;
		MaterialRegistry(const MaterialRegistry&) = delete;
		MaterialRegistry(MaterialRegistry&&) = delete;
		MaterialRegistry& operator=(const MaterialRegistry&) = delete;
		MaterialRegistry& operator=(MaterialRegistry&&) = delete;
		void import_material(d3d::GraphicsDevice& device, const stl::string& path);
		void import_material(d3d::GraphicsDevice& device, const stl::string& path, UUID uuid);
		void move_material(d3d::GraphicsDevice& device, const stl::string& old_path, const stl::string& new_path);
		void release_material(const stl::string& path);
		void serialize();
		void serialize(const MaterialAsset& asset) const;
		void deserialize(d3d::GraphicsDevice& device, const stl::string& data);
		MaterialAsset* get(const stl::string& path) const;
		MaterialAsset* get(UUID uuid) const;
		stl::string get_path(UUID uuid) const;
		stl::unordered_map<stl::string, MaterialAsset>& path_asset_map() { return m_PathToMaterialAssetMap; }
		stl::string to_string();

		static MaterialAsset* default_material(d3d::GraphicsDevice* device = nullptr);

		static void create_default(const stl::string& registry_file_path);

	private:
		stl::string m_RegistryFilePath;
		stl::unordered_map<stl::string, MaterialAsset> m_PathToMaterialAssetMap{};
		stl::unordered_map<UUID, stl::string> m_UUIDToPathMap{};
	};
} // namespace Sapfire::assets
