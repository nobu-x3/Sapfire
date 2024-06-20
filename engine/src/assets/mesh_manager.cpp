#include "engpch.h"

#include <fstream>
#include <iosfwd>
#include <nlohmann/json.hpp>
#include "assets/mesh_manager.h"
#include "core/core.h"
#include "core/file_system.h"
#include "core/logger.h"
#include "tools/obj_loader.h"

namespace Sapfire::assets {

	MeshRegistry::MeshRegistry(const stl::string& registry_file_path) {
		auto full_path = fs::FileSystem::get_full_path(fs::FileSystem::root_directory() + registry_file_path);
		auto relative_path = fs::FileSystem::root_directory() + registry_file_path;
		m_RegistryFilePath = relative_path;
		std::ifstream file{relative_path};
		if (!file.is_open()) {
			CORE_WARN("Mesh registry at path {} could not be open, created the default registry at given path.", full_path);
			create_default(relative_path);
			file.open(relative_path);
		}
		nlohmann::json j;
		file >> j;
		file.close();
		for (auto&& asset : j["assets"]) {
			if (!asset.is_object()) {
				CORE_CRITICAL("Broken mesh registry at path {}. Array of assets is not objects.", full_path);
				break;
			}
			if (!asset.contains("UUID")) {
				CORE_CRITICAL("Broken mesh registry at path {}. At least one registry entry does not contain UUID.", full_path);
				break;
			}
			if (!asset.contains("path")) {
				CORE_CRITICAL("Broken mesh registry at path {}. At least one registry entry does not contain path to the raw asset.",
							  full_path);
				break;
			}
			stl::string path = asset["path"];
			m_PathToMeshAssetMap[path] = MeshAsset{
				.uuid = UUID{asset["UUID"]},
				.data = tools::OBJLoader::load_mesh(path),
			};
			m_UUIDToPathMap[UUID{asset["UUID"]}] = path;
		}
	}

	void MeshRegistry::serialize() {
		nlohmann::json j;
		for (auto&& [path, asset] : m_PathToMeshAssetMap) {
			nlohmann::json j_obj = {{"UUID", static_cast<u64>(asset.uuid)}, {"path", path}};
			j["assets"].push_back(j_obj);
		}
		std::ofstream file{m_RegistryFilePath};
		file.clear();
		file << std::setw(4) << j << std::endl;
		file.close();
	}

	void MeshRegistry::import_mesh(const stl::string& path) {
		auto abs_path = fs::FileSystem::root_directory() + path;
		auto uuid = UUID{};
		m_PathToMeshAssetMap[path] = MeshAsset{
			.uuid = uuid,
			.data = tools::OBJLoader::load_mesh(abs_path),
		};
		m_UUIDToPathMap[uuid] = path;
	}

	void MeshRegistry::import_mesh(const stl::string& path, UUID uuid) {
		m_PathToMeshAssetMap[path] = MeshAsset{
			.uuid = uuid,
			.data = tools::OBJLoader::load_mesh(path),
		};
		m_UUIDToPathMap[uuid] = path;
	}

	void MeshRegistry::move_mesh(const stl::string& old_path, const stl::string& new_path) {
		if (!m_PathToMeshAssetMap.contains(old_path)) {
			CORE_WARN("Mesh at path {} does not exist, adding new.", old_path);
			auto uuid = UUID{};
			m_PathToMeshAssetMap[new_path] = MeshAsset{
				.uuid = uuid,
				.data = tools::OBJLoader::load_mesh(new_path),
			};
			m_UUIDToPathMap[uuid] = new_path;
			return;
		}
		auto mesh_data = m_PathToMeshAssetMap[old_path];
		m_PathToMeshAssetMap.erase(old_path);
		m_PathToMeshAssetMap[new_path] = mesh_data;
		m_UUIDToPathMap[mesh_data.uuid] = new_path;
	}

	void MeshRegistry::release_mesh(const stl::string& path) {
		if (!m_PathToMeshAssetMap.contains(path)) {
			CORE_ERROR("Mesh at path {} does not exist", path);
			return;
		}
		auto uuid = m_PathToMeshAssetMap[path].uuid;
		m_PathToMeshAssetMap.erase(path);
		m_UUIDToPathMap.erase(uuid);
	}

	MeshAsset* MeshRegistry::get(const stl::string& path) const {
		if (!m_PathToMeshAssetMap.contains(path))
			return nullptr;
		return const_cast<MeshAsset*>(&m_PathToMeshAssetMap.at(path));
	}

	MeshAsset* MeshRegistry::get(UUID uuid) const {
		if (!m_UUIDToPathMap.contains(uuid))
			return nullptr;
		auto& path = m_UUIDToPathMap.at(uuid);
		return get(path);
	}

	stl::string MeshRegistry::get_path(UUID uuid) const { return m_UUIDToPathMap.at(uuid); }

	stl::vector<DirectX::XMFLOAT3> MeshRegistry::all_positions() const {
		stl::vector<DirectX::XMFLOAT3> positions{};
		for (auto&& [path, asset] : m_PathToMeshAssetMap) {
			if (!asset.data.has_value())
				continue;
			for (auto& pos : asset.data->positions) {
				positions.push_back(pos);
			}
		}
		return positions;
	}

	stl::vector<DirectX::XMFLOAT3> MeshRegistry::all_normals() const {
		stl::vector<DirectX::XMFLOAT3> normals{};
		for (auto&& [path, asset] : m_PathToMeshAssetMap) {
			if (!asset.data.has_value())
				continue;
			for (auto& norm : asset.data->normals) {
				normals.push_back(norm);
			}
		}
		return normals;
	}

	stl::vector<DirectX::XMFLOAT3> MeshRegistry::all_tangents() const {
		stl::vector<DirectX::XMFLOAT3> tangents{};
		for (auto&& [path, asset] : m_PathToMeshAssetMap) {
			if (!asset.data.has_value())
				continue;
			for (auto& tang : asset.data->tangentus) {
				tangents.push_back(tang);
			}
		}
		return tangents;
	}

	stl::vector<DirectX::XMFLOAT2> MeshRegistry::all_texture_coordinates() const {
		stl::vector<DirectX::XMFLOAT2> uvs{};
		for (auto&& [path, asset] : m_PathToMeshAssetMap) {
			if (!asset.data.has_value())
				continue;
			for (auto& uv : asset.data->texcs) {
				uvs.push_back(uv);
			}
		}
		return uvs;
	}

	std::vector<u16> MeshRegistry::all_indices16() {
		stl::vector<u16> indices{};
		for (auto&& [path, asset] : m_PathToMeshAssetMap) {
			if (!asset.data.has_value())
				continue;
			for (const auto& ind : asset.data->indices16()) {
				indices.push_back(static_cast<u16>(ind));
			}
		}
		return indices;
	}

	void MeshRegistry::create_default(const stl::string& filepath) {
		auto j = R"(
        {
            "assets" : []
        }
        )"_json;
		std::ofstream file{filepath};
		file << std::setw(4) << j << std::endl;
		file.close();
	}

	stl::string MeshRegistry::to_string() {
		nlohmann::json j;
		for (auto&& [path, asset] : m_PathToMeshAssetMap) {
			nlohmann::json j_obj = {{"UUID", static_cast<u64>(asset.uuid)}, {"path", path}};
			j.push_back(j_obj);
		}
		return j.dump();
	}

	void MeshRegistry::deserialize(const stl::string& data) {
		nlohmann::json j = nlohmann::json::parse(data)["assets"];
		if (!j.contains("mesh_registry")) {
			CORE_CRITICAL("Given mesh registry does not exist. Mesh registry will not be loaded.");
			return;
		}
		for (auto&& mesh_asset : j["mesh_registry"]) {
			stl::string path = mesh_asset["path"];
			if (!mesh_asset.contains("path")) {
				CORE_ERROR("An asset in the mesh registry does not contain a path. It will not be loaded. Dump:\n{}", data);
				continue;
			}
			if (!mesh_asset.contains("UUID")) {
				CORE_ERROR("An asset in the mesh registry with path {} does not contain a UUID. It will not be loaded. Dump:\n{}", path,
						   data);
				continue;
			}
			UUID uuid{mesh_asset["UUID"]};
			import_mesh(path, uuid);
		}
	}

} // namespace Sapfire::assets
