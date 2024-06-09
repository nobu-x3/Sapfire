#include "engpch.h"

#include "assets/texture_manager.h"
#include "nlohmann/json.hpp"
#include "render/d3d_util.h"
#include "render/graphics_device.h"
#include "render/resources.h"
#include "tools/texture_loader.h"

namespace Sapfire::assets {

	TextureRegistry::TextureRegistry(const stl::string& registry_file_path) {
		auto full_path = fs::FileSystem::get_full_path(fs::FileSystem::root_directory() + registry_file_path);
		auto relative_path = fs::FileSystem::root_directory() + registry_file_path;
		m_RegistryFilePath = relative_path;
		std::ifstream file{relative_path};
		if (!file.is_open()) {
			CORE_WARN("Texture registry at path {} could not be open, created the default registry at given path.", full_path);
			create_default(relative_path);
			file.open(relative_path);
		}
		nlohmann::json j;
		file >> j;
		file.close();
		for (auto&& asset : j["assets"]) {
			if (!asset.is_object()) {
				CORE_CRITICAL("Broken texture registry at path {}. Array of assets is not objects.", full_path);
				break;
			}
			if (!asset.contains("UUID")) {
				CORE_CRITICAL("Broken texture registry at path {}. At least one registry entry does not contain UUID.", full_path);
				break;
			}
			if (!asset.contains("path")) {
				CORE_CRITICAL("Broken texture registry at path {}. At least one registry entry does not contain path to the raw asset.",
							  full_path);
				break;
			}
			if (!asset.contains("description")) {
				CORE_CRITICAL("Broken texture registry at path {}. At least one registry entry does not contain texture description.",
							  full_path);
				break;
			}
			auto& desc_json = asset["description"];
			if (!desc_json.is_object()) {
				CORE_CRITICAL("Broken texture registry at path {}. Error reading the texture creation description, is not a json object.",
							  full_path);
				break;
			}
            if(!desc_json.contains("usage") || !desc_json["usage"].is_number_unsigned()) {
				CORE_CRITICAL("Broken texture registry at path {}. One of the textures does not contain a u32 'usage' field.",
							  full_path);
				break;
            }
            if(!desc_json.contains("width") || !desc_json["width"].is_number_unsigned()) {
				CORE_CRITICAL("Broken texture registry at path {}. One of the textures does not contain a u32 'width' field.",
							  full_path);
				break;
            }
            if(!desc_json.contains("height") || !desc_json["height"].is_number_unsigned()) {
				CORE_CRITICAL("Broken texture registry at path {}. One of the textures does not contain a u32 'height' field.",
							  full_path);
				break;
            }
            if(!desc_json.contains("format") || !desc_json["format"].is_number_unsigned()) {
				CORE_CRITICAL("Broken texture registry at path {}. One of the textures does not contain a u32 'format' field.",
							  full_path);
				break;
            }
            if(!desc_json.contains("mip_levels") || !desc_json["mip_levels"].is_number_unsigned()) {
				CORE_CRITICAL("Broken texture registry at path {}. One of the textures does not contain a u32 'mip_levels' field.",
							  full_path);
				break;
            }
            if(!desc_json.contains("depth_or_array_size") || !desc_json["depth_or_array_size"].is_number_unsigned()) {
				CORE_CRITICAL("Broken texture registry at path {}. One of the textures does not contain a u32 'depth_or_array_size' field.",
							  full_path);
				break;
            }
            if(!desc_json.contains("bytes_per_pixel") || !desc_json["bytes_per_pixel"].is_number_unsigned()) {
				CORE_CRITICAL("Broken texture registry at path {}. One of the textures does not contain a u32 'bytes_per_pixel' field.",
							  full_path);
				break;
            }
			stl::string path = asset["path"];
            stl::wstring path_wstring = d3d::AnsiToWString(path);
			d3d::TextureCreationDesc desc{
				.usage = desc_json["usage"],
				.width = desc_json["width"],
				.height = desc_json["height"],
				.format = desc_json["format"],
				.mipLevels = desc_json["mip_levels"],
				.depthOrArraySize = desc_json["depth_or_array_size"],
				.bytesPerPixel = desc_json["bytes_per_pixel"],
				.name = path_wstring,
				.path = path_wstring,
			};
            if(desc_json.contains("optional_initial_state")) {
                desc.optional_initial_state = desc_json["optional_initial_state"];
            }
			m_PathToMeshAssetMap[path] = TextureAsset{
				.uuid = UUID{asset["UUID"]},
				.description = desc,
			};
			m_UUIDToPathMap[UUID{asset["UUID"]}] = path;
		}
	}

	void TextureRegistry::create_default(const stl::string& filepath) {
		auto j = R"(
        {
            "assets" : []
        }
        )"_json;
		std::ofstream file{filepath};
		file << std::setw(4) << j << std::endl;
		file.close();
	}

	void TextureRegistry::import_texture(d3d::GraphicsDevice& device, const stl::string& path) {
		auto abs_path = fs::FileSystem::root_directory() + path;
		auto uuid = UUID{};
		s32 width{};
		s32 height{};
		void* data = tools::texture_loader::load(path.c_str(), width, height, 4);
		m_PathToMeshAssetMap[path] = TextureAsset{
			.uuid = uuid,
			.description =
				d3d::TextureCreationDesc{
					.usage = d3d::TextureUsage::TextureFromPath,
					.width = static_cast<u32>(width),
					.height = static_cast<u32>(height),
					.name = d3d::AnsiToWString(path),
					.path = d3d::AnsiToWString(path),
				},
			.data = device.create_texture(
				d3d::TextureCreationDesc{
					.usage = d3d::TextureUsage::TextureFromPath,
					.name = d3d::AnsiToWString(path),
					.path = d3d::AnsiToWString(path),
				},
				data),
		};
		m_UUIDToPathMap[uuid] = path;
	}

	void TextureRegistry::move_texture(d3d::GraphicsDevice& device, const stl::string& old_path, const stl::string& new_path) {
		if (!m_PathToMeshAssetMap.contains(old_path)) {
			CORE_WARN("Texture at path {} does not exist, adding new.", old_path);
			auto uuid = UUID{};
			s32 width{};
			s32 height{};
			void* data = tools::texture_loader::load(new_path.c_str(), width, height, 4);
			m_PathToMeshAssetMap[new_path] = TextureAsset{
				.uuid = uuid,
				.description =
					d3d::TextureCreationDesc{
						.usage = d3d::TextureUsage::TextureFromPath,
						.width = static_cast<u32>(width),
						.height = static_cast<u32>(height),
						.name = d3d::AnsiToWString(new_path),
						.path = d3d::AnsiToWString(new_path),
					},
				.data = device.create_texture(
					d3d::TextureCreationDesc{
						.usage = d3d::TextureUsage::TextureFromPath,
						.name = d3d::AnsiToWString(new_path),
						.path = d3d::AnsiToWString(new_path),
					},
					data),
			};
			m_UUIDToPathMap[uuid] = new_path;
			return;
		}
		auto mesh_data = m_PathToMeshAssetMap[old_path];
		m_PathToMeshAssetMap.erase(old_path);
		m_PathToMeshAssetMap[new_path] = mesh_data;
		m_UUIDToPathMap[mesh_data.uuid] = new_path;
	}

	void TextureRegistry::release_texture(const stl::string& path) {
		if (!m_PathToMeshAssetMap.contains(path)) {
			CORE_ERROR("Texture at path {} does not exist", path);
			return;
		}
		auto uuid = m_PathToMeshAssetMap[path].uuid;
		m_PathToMeshAssetMap.erase(path);
		m_UUIDToPathMap.erase(uuid);
	}

	void TextureRegistry::serialize() {
		nlohmann::json j;
		for (auto&& [path, asset] : m_PathToMeshAssetMap) {
			auto& desc = asset.description;
			nlohmann::json desc_j{
				{"usage", static_cast<u32>(desc.usage)},
				{"width", desc.width},
				{"height", desc.height},
				{"format", desc.format},
				{"mip_levels", desc.mipLevels},
				{"depth_or_array_size", desc.depthOrArraySize},
                {"bytes_per_pixel", desc.bytesPerPixel},
			};
			if (desc.optional_initial_state.has_value()) {
				desc_j["optional_initial_state"] = desc.optional_initial_state.value();
			}
			nlohmann::json j_obj = {{"UUID", static_cast<u64>(asset.uuid)}, {"path", path}, {"description", desc_j}};
			j["assets"].push_back(j_obj);
		}
		std::ofstream file{m_RegistryFilePath};
		file.clear();
		file << std::setw(4) << j << std::endl;
		file.close();
	}

	TextureAsset* TextureRegistry::get(const stl::string& path) const {
		if (!m_PathToMeshAssetMap.contains(path))
			return nullptr;
		return const_cast<TextureAsset*>(&m_PathToMeshAssetMap.at(path));
	}

	TextureAsset* TextureRegistry::get(UUID uuid) const {
		if (!m_UUIDToPathMap.contains(uuid))
			return nullptr;
		auto& path = m_UUIDToPathMap.at(uuid);
		return get(path);
	}

	stl::string TextureRegistry::get_path(UUID uuid) const { return m_UUIDToPathMap.at(uuid); }
} // namespace Sapfire::assets
