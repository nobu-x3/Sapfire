#pragma once

#include "core/core.h"
#include "core/uuid.h"
#include "render/d3d_primitives.h"
#include "components/render_component.h"

namespace Sapfire::assets {

    struct SFAPI MeshResource {
        Sapfire::components::CPUData cpu_data;
        Sapfire::components::PerDrawConstants gpu_data;
    };

    struct SFAPI MeshManager {
        Sapfire::stl::unordered_map<Sapfire::stl::string, MeshResource> mesh_resources;
    };

	struct SFAPI MeshAsset {
		UUID uuid;
		stl::optional<d3d::primitives::MeshData> data;
	};

	class SFAPI MeshRegistry {
	public:
		explicit MeshRegistry(const stl::string& registry_file_path);
		explicit MeshRegistry() = default;
		~MeshRegistry() = default;
		MeshRegistry(const MeshRegistry&) = delete;
		MeshRegistry(MeshRegistry&&) = delete;
		MeshRegistry& operator=(const MeshRegistry&) = delete;
		MeshRegistry& operator=(MeshRegistry&&) = delete;
		void import_mesh(const stl::string& path);
		void move_mesh(const stl::string& old_path, const stl::string& new_path);
		void release_mesh(const stl::string& path);
		void serialize();
		MeshAsset* get(const stl::string& path) const;
		MeshAsset* get(UUID uuid) const;
        stl::string get_path(UUID uuid) const;
        const stl::unordered_map<stl::string, MeshAsset>& path_asset_map() const { return m_PathToMeshAssetMap; }
		stl::vector<DirectX::XMFLOAT3> all_positions() const;
		stl::vector<DirectX::XMFLOAT3> all_normals() const;
		stl::vector<DirectX::XMFLOAT3> all_tangents() const;
		stl::vector<DirectX::XMFLOAT2> all_texture_coordinates() const;
		std::vector<u16> all_indices16();

		static void create_default(const stl::string& registry_file_path);

	private:
		stl::string m_RegistryFilePath;
		stl::unordered_map<stl::string, MeshAsset> m_PathToMeshAssetMap{};
		stl::unordered_map<UUID, stl::string> m_UUIDToPathMap{};
	};
} // namespace Sapfire::assets
