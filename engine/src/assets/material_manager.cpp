#include "engpch.h"

#include <DirectXMath.h>
#include "assets/material_manager.h"
#include "core/file_system.h"
#include "core/logger.h"
#include "nlohmann/json.hpp"
#include "render/d3d_util.h"
#include "render/graphics_device.h"
#include "render/resources.h"

using namespace DirectX;

namespace Sapfire::assets {

	const UUID DEFAULT_MATERIAL_UUID = 13700118063961433559;
	constexpr f32 DEFAULT_MATERIAL_ROUGHTNESS = 0.f;
	constexpr DirectX::XMFLOAT4 DEFAULT_MATERIAL_ALBEDO = {1.f, 0.f, 1.f, 1.f};
	constexpr DirectX::XMFLOAT3 DEFAULT_MATERIAL_FRESNEL = {1.f, 1.f, 1.f};
	const stl::string DEFAULT_MATERIAL_NAME = "Default Material";

	void MaterialManager::add(const Sapfire::stl::string& path, Sapfire::UUID uuid, MaterialResource resource) {
		material_resources[path] = resource;
		uuid_to_path_map[uuid] = path;
	}

	MaterialRegistry::MaterialRegistry(const stl::string& registry_file_path) {
		auto full_path = fs::full_path(registry_file_path);
		m_RegistryFilePath = registry_file_path;
		std::ifstream file{registry_file_path};
		if (!file.is_open()) {
			CORE_WARN("Texture registry at path {} could not be open, created the default registry at given path.", full_path);
			create_default(registry_file_path);
			file.open(registry_file_path);
		}
		nlohmann::json j;
		file >> j;
		file.close();
		for (auto&& asset : j["assets"]) {
			if (!asset.contains("UUID")) {
				CORE_CRITICAL("Broken material registry at path {}. At least one registry entry does not contain UUID.", full_path);
				return;
			}
			if (!asset.contains("path")) {
				CORE_CRITICAL("Broken material registry at path {}. At least one registry entry does not contain path to the raw asset.",
							  full_path);
				return;
			}
			if (!asset.contains("name")) {
				CORE_CRITICAL("Broken material registry at path {}. At least one registry entry does not contain name.", full_path);
				return;
			}
			if (!asset.contains("diffuse_albedo")) {
				CORE_CRITICAL("Broken material registry at path {}. At least one registry entry does not contain diffuse albedo.",
							  full_path);
				return;
			}
			if (!asset.contains("fresnel_r0")) {
				CORE_CRITICAL("Broken material registry at path {}. At least one registry entry does not contain fresnel r0.", full_path);
				return;
			}
			if (!asset.contains("roughness")) {
				CORE_CRITICAL("Broken material registry at path {}. At least one registry entry does not contain roughness.", full_path);
				return;
			}
			stl::string path = asset["path"];
			d3d::Material material{.name = fs::file_name(asset["name"])};
			material.roughness = asset["roughness"];
			material.diffuse_albedo =
				XMFLOAT4(asset["diffuse_albedo"][0], asset["diffuse_albedo"][1], asset["diffuse_albedo"][2], asset["diffuse_albedo"][3]);
			material.fresnel_r0 = XMFLOAT3(asset["fresnel_r0"][0], asset["fresnel_r0"][1], asset["fresnel_r0"][2]);
			m_PathToMaterialAssetMap[path] = MaterialAsset{
				.uuid = UUID{asset["UUID"]},
				.material = material,
			};
			m_UUIDToPathMap[UUID{asset["UUID"]}] = path;
		}
	}

	void MaterialRegistry::create_default(const stl::string& filepath) {
		auto j = R"(
        {
            "assets" : []
        }
        )"_json;
		std::ofstream file{filepath};
		file << std::setw(4) << j << std::endl;
		file.close();
	}

	void MaterialRegistry::import_material(d3d::GraphicsDevice& device, const stl::string& path) {
		if (m_PathToMaterialAssetMap.contains(path))
			return;
		std::ifstream file{path};
		if (!file.is_open()) {
			CORE_ERROR("Material at path {} could not be open.", path);
			return;
		}
		nlohmann::json j;
		file >> j;
		file.close();
		if (!j.contains("UUID")) {
			CORE_CRITICAL("Broken material at path {}. Does not contain UUID.", path);
			return;
		}
		if (!j.contains("name")) {
			CORE_CRITICAL("Broken material at path {}. Does not contain name.", path);
			return;
		}
		if (!j.contains("diffuse_albedo")) {
			CORE_CRITICAL("Broken material at path {}. Does not contain diffuse albedo.", path);
			return;
		}
		if (!j.contains("fresnel_r0")) {
			CORE_CRITICAL("Broken material at path {}. Does not contain fresnel r0.", path);
			return;
		}
		if (!j.contains("roughness")) {
			CORE_CRITICAL("Broken material at path {}. Does not contain roughness.", path);
			return;
		}
		UUID uuid = UUID{j["UUID"]};
		d3d::Material material{.name = fs::file_name(j["name"])};
		material.roughness = j["roughness"];
		material.diffuse_albedo = XMFLOAT4(j["diffuse_albedo"][0], j["diffuse_albedo"][1], j["diffuse_albedo"][2], j["diffuse_albedo"][3]);
		material.fresnel_r0 = XMFLOAT3(j["fresnel_r0"][0], j["fresnel_r0"][1], j["fresnel_r0"][2]);
		material.material_buffer = device.create_buffer<d3d::MaterialConstants>({
			.usage = d3d::BufferUsage::ConstantBuffer,
			.name = d3d::AnsiToWString(material.name),
		});
		material.material_cb_index = material.material_buffer.cbv_index;
		m_PathToMaterialAssetMap[path] = MaterialAsset{
			.uuid = uuid,
			.material = material,
		};
		m_UUIDToPathMap[uuid] = path;
	}

	void MaterialRegistry::import_material(d3d::GraphicsDevice& device, const stl::string& path, UUID uuid) {
		if (m_PathToMaterialAssetMap.contains(path))
			return;
		std::ifstream file{path};
		if (!file.is_open()) {
			CORE_ERROR("Material at path {} could not be open.", path);
			return;
		}
		nlohmann::json j;
		file >> j;
		file.close();
		if (!j.contains("name")) {
			CORE_CRITICAL("Broken material at path {}. Does not contain name.", path);
			return;
		}
		if (!j.contains("diffuse_albedo")) {
			CORE_CRITICAL("Broken material at path {}. Does not contain diffuse albedo.", path);
			return;
		}
		if (!j.contains("fresnel_r0")) {
			CORE_CRITICAL("Broken material at path {}. Does not contain fresnel r0.", path);
			return;
		}
		if (!j.contains("roughness")) {
			CORE_CRITICAL("Broken material at path {}. Does not contain roughness.", path);
			return;
		}
		d3d::Material material{.name = fs::file_name(j["name"])};
		material.roughness = j["roughness"];
		material.diffuse_albedo = XMFLOAT4(j["diffuse_albedo"][0], j["diffuse_albedo"][1], j["diffuse_albedo"][2], j["diffuse_albedo"][3]);
		material.fresnel_r0 = XMFLOAT3(j["fresnel_r0"][0], j["fresnel_r0"][1], j["fresnel_r0"][2]);
		material.material_buffer = device.create_buffer<d3d::MaterialConstants>({
			.usage = d3d::BufferUsage::ConstantBuffer,
			.name = d3d::AnsiToWString(material.name),
		});
		material.material_cb_index = material.material_buffer.cbv_index;
		m_PathToMaterialAssetMap[path] = MaterialAsset{
			.uuid = uuid,
			.material = material,
		};
		m_UUIDToPathMap[uuid] = path;
	}

	void MaterialRegistry::move_material(d3d::GraphicsDevice& device, const stl::string& old_path, const stl::string& new_path) {}

	void MaterialRegistry::release_material(const stl::string& path) {
		if (!m_PathToMaterialAssetMap.contains(path)) {
			CORE_ERROR("Material at path {} does not exist.", path);
			return;
		}
		auto uuid = m_PathToMaterialAssetMap[path].uuid;
		m_PathToMaterialAssetMap.erase(path);
		m_UUIDToPathMap.erase(uuid);
	}

	void MaterialRegistry::serialize() {
		nlohmann::json j;
		for (auto&& [path, asset] : m_PathToMaterialAssetMap) {
			auto& uuid = asset.uuid;
			auto& material = asset.material;
			auto& name = material.name;
			auto& roughness = material.roughness;
			float fresnel_r0[3] = {material.fresnel_r0.x, material.fresnel_r0.y, material.fresnel_r0.z};
			float diffuse_albedo[4] = {material.diffuse_albedo.x, material.diffuse_albedo.y, material.diffuse_albedo.z,
									   material.diffuse_albedo.w};
			nlohmann::json j_obj = {
				{"UUID", static_cast<u64>(uuid)},	{"path", path}, {"name", name}, {"roughness", roughness}, {"fresnel_r0", fresnel_r0},
				{"diffuse_albedo", diffuse_albedo},
			};
			{
				// @TODO: this is super slow because opening files in a loop. Rework this.
				std::ofstream file{path};
				file.clear();
				file << std::setw(4) << j_obj << std::endl;
				file.close();
			}
			{
				nlohmann::json path_jobj = {"path", path};
				j["assets"].push_back(path_jobj);
			}
		}
		std::ofstream file{m_RegistryFilePath};
		file.clear();
		file << std::setw(4) << j << std::endl;
		file.close();
	}

	void MaterialRegistry::serialize(const MaterialAsset& asset) const {
		auto& uuid = asset.uuid;
		if (!m_UUIDToPathMap.contains(uuid)) {
			CORE_ERROR("Material asset with UUID {} has not been previously imported to the registry.", static_cast<u64>(uuid));
			return;
		}
		const auto& path = m_UUIDToPathMap.at(uuid);
		auto& material = asset.material;
		auto& name = material.name;
		auto& roughness = material.roughness;
		float fresnel_r0[3] = {material.fresnel_r0.x, material.fresnel_r0.y, material.fresnel_r0.z};
		float diffuse_albedo[4] = {material.diffuse_albedo.x, material.diffuse_albedo.y, material.diffuse_albedo.z,
								   material.diffuse_albedo.w};
		nlohmann::json j_obj = {
			{"UUID", static_cast<u64>(uuid)},	{"path", path}, {"name", name}, {"roughness", roughness}, {"fresnel_r0", fresnel_r0},
			{"diffuse_albedo", diffuse_albedo},
		};
		{
			// @TODO: this is super slow because opening files in a loop. Rework this.
			std::ofstream file{fs::full_path(path)};
			file.clear();
			file << std::setw(4) << j_obj << std::endl;
			file.close();
		}
	}

	void MaterialRegistry::deserialize(d3d::GraphicsDevice& device, const stl::string& data) {
		nlohmann::json j = nlohmann::json::parse(data)["assets"];
		for (auto&& asset : j["material_registry"]) {
			if (!asset.contains("path")) {
				CORE_CRITICAL("Broken material registry. At least one registry entry does not contain path to the raw asset.");
				return;
			}
			stl::string path = asset["path"];
			stl::string relative_path = fs::relative_path(path);
			if (relative_path.empty()) {
				CORE_WARN("Could not locate material at path {}.", path);
				continue;
			}
			import_material(device, relative_path);
		}
	}

	stl::string MaterialRegistry::to_string() {
		nlohmann::json j;
		for (auto&& [path, asset] : m_PathToMaterialAssetMap) {
			auto& uuid = asset.uuid;
			auto& material = asset.material;
			auto& name = material.name;
			auto& roughness = material.roughness;
			float fresnel_r0[3] = {material.fresnel_r0.x, material.fresnel_r0.y, material.fresnel_r0.z};
			float diffuse_albedo[4] = {material.diffuse_albedo.x, material.diffuse_albedo.y, material.diffuse_albedo.z,
									   material.diffuse_albedo.w};
			nlohmann::json j_obj = {
				{"UUID", static_cast<u64>(uuid)},	{"path", path}, {"name", name}, {"roughness", roughness}, {"fresnel_r0", fresnel_r0},
				{"diffuse_albedo", diffuse_albedo},
			};
			nlohmann::json path_jobj = {"path", path};
			j.push_back(path_jobj);
		}
		return j.dump();
	}

	MaterialAsset* MaterialRegistry::get(const stl::string& path) const {
		if (!m_PathToMaterialAssetMap.contains(path))
			return MaterialRegistry::default_material();
		return const_cast<MaterialAsset*>(&m_PathToMaterialAssetMap.at(path));
	}

	MaterialAsset* MaterialRegistry::get(UUID uuid) const {
		if (!m_UUIDToPathMap.contains(uuid)) {
			return MaterialRegistry::default_material();
		}
		auto& path = m_UUIDToPathMap.at(uuid);
		return get(path);
	}

	stl::string MaterialRegistry::get_path(UUID uuid) const {
		if (!m_UUIDToPathMap.contains(uuid))
			return "";
		return m_UUIDToPathMap.at(uuid);
	}

	MaterialAsset* MaterialRegistry::default_material(d3d::GraphicsDevice* device) {
		static stl::wstring name = d3d::AnsiToWString(DEFAULT_MATERIAL_NAME);
		static d3d::MaterialConstants default_material_constants{
			.diffuse_albedo = DEFAULT_MATERIAL_ALBEDO,
			.fresnel_r0 = DEFAULT_MATERIAL_FRESNEL,
			.roughness = DEFAULT_MATERIAL_ROUGHTNESS,
		};
		static d3d::Buffer buffer = device->create_buffer<d3d::MaterialConstants>({
			.usage = d3d::BufferUsage::ConstantBuffer,
			.name = name,
		});
		static MaterialAsset default_mat{
			.uuid = DEFAULT_MATERIAL_UUID,
			.material{
				.name = DEFAULT_MATERIAL_NAME,
				.diffuse_albedo = DEFAULT_MATERIAL_ALBEDO,
				.fresnel_r0 = DEFAULT_MATERIAL_FRESNEL,
				.roughness = DEFAULT_MATERIAL_ROUGHTNESS,
				.material_buffer = buffer,
				.material_cb_index = static_cast<s32>(buffer.cbv_index),
			},
		};
		buffer.update(&default_material_constants);
		return &default_mat;
	}

} // namespace Sapfire::assets
