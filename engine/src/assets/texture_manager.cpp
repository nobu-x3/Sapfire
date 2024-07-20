#include "core/file_system.h"
#include "engpch.h"

#include "assets/texture_manager.h"
#include "nlohmann/json.hpp"
#include "render/d3d_util.h"
#include "render/graphics_device.h"
#include "render/resources.h"
#include "tools/texture_loader.h"

namespace Sapfire::assets {

	void TextureManager::add(const Sapfire::stl::string& path, Sapfire::UUID uuid, TextureResource resource) {
		texture_resources[path] = resource;
		uuid_to_path_map[uuid] = path;
	}

	TextureRegistry::TextureRegistry(const stl::string& registry_file_path) : m_RegistryFilePath(fs::full_path(registry_file_path)) {}

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
		auto uuid = UUID{};
		s32 width{};
		s32 height{};
		void* data = tools::texture_loader::load(fs::full_path(path).c_str(), width, height, 4);
		auto name = d3d::AnsiToWString(path);
		if (m_PathToTextureAssetMap.contains(path))
			return;
		m_PathToTextureAssetMap[path] = TextureAsset{
			.uuid = uuid,
			.description =
				d3d::TextureCreationDesc{
					.usage = d3d::TextureUsage::TextureFromPath,
					.width = static_cast<u32>(width),
					.height = static_cast<u32>(height),
					.name = name,
					.path = d3d::AnsiToWString(path),
				},
			.data = device.create_texture(
				d3d::TextureCreationDesc{
					.usage = d3d::TextureUsage::TextureFromPath,
					.name = name,
					.path = d3d::AnsiToWString(fs::full_path(path)),
				},
				data),
		};
		m_UUIDToPathMap[uuid] = path;
	}

	void TextureRegistry::import_texture(d3d::GraphicsDevice& device, const stl::string& path, const d3d::TextureCreationDesc& desc,
										 UUID uuid) {
		auto name = d3d::AnsiToWString(path);
		if (m_PathToTextureAssetMap.contains(path))
			return;
		m_PathToTextureAssetMap[path] = TextureAsset{
			.uuid = uuid,
			.description = desc,
			.data = device.create_texture(d3d::TextureCreationDesc{
				.usage = d3d::TextureUsage::TextureFromPath,
				.name = name,
				.path = d3d::AnsiToWString(fs::full_path(path)),
			}),
		};
		m_UUIDToPathMap[uuid] = path;
	}

	void TextureRegistry::move_texture(d3d::GraphicsDevice& device, const stl::string& old_path, const stl::string& new_path) {
		if (!m_PathToTextureAssetMap.contains(old_path)) {
			CORE_WARN("Texture at path {} does not exist, adding new.", old_path);
			auto uuid = UUID{};
			s32 width{};
			s32 height{};
			void* data = tools::texture_loader::load(new_path.c_str(), width, height, 4);
			auto name = d3d::AnsiToWString(new_path);
			m_PathToTextureAssetMap[new_path] = TextureAsset{
				.uuid = uuid,
				.description =
					d3d::TextureCreationDesc{
						.usage = d3d::TextureUsage::TextureFromPath,
						.width = static_cast<u32>(width),
						.height = static_cast<u32>(height),
						.name = name,
						.path = d3d::AnsiToWString(new_path),
					},
				.data = device.create_texture(
					d3d::TextureCreationDesc{
						.usage = d3d::TextureUsage::TextureFromPath,
						.name = name,
						.path = d3d::AnsiToWString(new_path),
					},
					data),
			};
			m_UUIDToPathMap[uuid] = new_path;
			return;
		}
		auto mesh_data = m_PathToTextureAssetMap[old_path];
		m_PathToTextureAssetMap.erase(old_path);
		m_PathToTextureAssetMap[new_path] = mesh_data;
		m_UUIDToPathMap[mesh_data.uuid] = new_path;
	}

	void TextureRegistry::release_texture(const stl::string& path) {
		if (!m_PathToTextureAssetMap.contains(path)) {
			CORE_ERROR("Texture at path {} does not exist", path);
			return;
		}
		auto uuid = m_PathToTextureAssetMap[path].uuid;
		m_PathToTextureAssetMap.erase(path);
		m_UUIDToPathMap.erase(uuid);
	}

	void TextureRegistry::serialize() {
		nlohmann::json j;
		for (auto&& [path, asset] : m_PathToTextureAssetMap) {
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
			const nlohmann::json j_obj = {{"UUID", static_cast<u64>(asset.uuid)}, {"path", path}, {"description", desc_j}};
			j["assets"].push_back(j_obj);
		}
		std::ofstream file{m_RegistryFilePath};
		file.clear();
		file << std::setw(4) << j << std::endl;
		file.close();
	}

	TextureAsset* TextureRegistry::get(const stl::string& path) const {
		if (!m_PathToTextureAssetMap.contains(path))
			return TextureRegistry::default_texture();
		return const_cast<TextureAsset*>(&m_PathToTextureAssetMap.at(path));
	}

	TextureAsset* TextureRegistry::get(UUID uuid) const {
		if (!m_UUIDToPathMap.contains(uuid))
			return TextureRegistry::default_texture();
		auto& path = m_UUIDToPathMap.at(uuid);
		return get(path);
	}

	stl::string TextureRegistry::get_path(UUID uuid) const {
		if (!m_UUIDToPathMap.contains(uuid))
			return "";
		return m_UUIDToPathMap.at(uuid);
	}

	stl::string TextureRegistry::to_string() {
		nlohmann::json j;
		for (auto&& [path, asset] : m_PathToTextureAssetMap) {
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
			const nlohmann::json j_obj = {{"UUID", static_cast<u64>(asset.uuid)}, {"path", path}, {"description", desc_j}};
			j.push_back(j_obj);
		}
		return j.dump();
	}

	void TextureRegistry::deserialize(d3d::GraphicsDevice& device, const stl::string& data) {
		nlohmann::json j = nlohmann::json::parse(data)["assets"];
		if (!j.contains("texture_registry")) {
			CORE_CRITICAL("Given texture registry string for deserialization deos not contain texture registry. The texture registry will "
						  "not be loaded.");
			return;
		}
		for (auto&& asset : j["texture_registry"]) {
			if (!asset.contains("path")) {
				CORE_ERROR("One of the textures in the texture registry is missing a path to raw texture. It will not be loaded. Dump:\n{}",
						   data);
				continue;
			}
			stl::string path = asset["path"];
			if (!fs::exists(path)) {
				CORE_ERROR("Texture with path {} does not exist.", path);
				continue;
			}
			path = fs::relative_path(path);
			if (!asset.contains("UUID")) {
				CORE_ERROR("Texture with path {} is missing UUID at deserialization. It will not be loaded. Dump:\n{}", path, data);
				continue;
			}
			if (!asset.contains("description")) {
				CORE_ERROR("Texture with path {} is missing texture creation description. It will not be loaded. Dump:\n{}", path, data);
				continue;
			}
			const UUID uuid{asset["UUID"]};
			auto& description = asset["description"];
			if (!description.contains("usage")) {
				CORE_ERROR("Texture with path {} is missing texture creation description's 'usage' field. It will not be loaded. Dump:\n{}",
						   path, data);
				continue;
			}
			if (!description.contains("width")) {
				CORE_ERROR("Texture with path {} is missing texture creation description's 'width' field. It will not be loaded. Dump:\n{}",
						   path, data);
				continue;
			}
			if (!description.contains("height")) {
				CORE_ERROR(
					"Texture with path {} is missing texture creation description's 'height' field. It will not be loaded. Dump:\n{}", path,
					data);
				continue;
			}
			if (!description.contains("format")) {
				CORE_ERROR(
					"Texture with path {} is missing texture creation description's 'format' field. It will not be loaded. Dump:\n{}", path,
					data);
				continue;
			}
			if (!description.contains("mip_levels")) {
				CORE_ERROR(
					"Texture with path {} is missing texture creation description's 'mip_levels' field. It will not be loaded. Dump:\n{}",
					path, data);
				continue;
			}
			if (!description.contains("depth_or_array_size")) {
				CORE_ERROR("Texture with path {} is missing texture creation description's 'depth_or_array_size' field. It will not be "
						   "loaded. Dump:\n{}",
						   path, data);
				continue;
			}
			if (!description.contains("bytes_per_pixel")) {
				CORE_ERROR("Texture with path {} is missing texture creation description's 'bytes_per_pixel' field. It will not be loaded. "
						   "Dump:\n{}",
						   path, data);
				continue;
			}
			d3d::TextureCreationDesc desc;
			desc.width = description["width"];
			desc.height = description["height"];
			desc.usage = description["usage"];
			desc.path = d3d::AnsiToWString(path);
			desc.bytesPerPixel = description["bytes_per_pixel"];
			desc.format = description["format"];
			desc.mipLevels = description["mip_levels"];
			if (description.contains("optional_initial_state")) {
				desc.optional_initial_state = description["optional_initial_state"];
			}
			import_texture(device, path, desc, uuid);
		}
	}

	constexpr u32 DEFAULT_TEXTURE_DIMENSIONS = 256;
	constexpr u32 DEFAULT_TEXTURE_CHANNELS = 4;
	constexpr u32 BYTE_COUNT = DEFAULT_TEXTURE_DIMENSIONS * DEFAULT_TEXTURE_DIMENSIONS * DEFAULT_TEXTURE_CHANNELS;
	const d3d::TextureCreationDesc DEFAULT_TEXTURE_CREATION_INFO = {
		.usage = d3d::TextureUsage::TextureFromData,
		.width = 256,
		.height = 256,
		.format = DXGI_FORMAT_R8G8B8A8_UNORM,
		.depthOrArraySize = 1,
		.bytesPerPixel = DEFAULT_TEXTURE_CHANNELS,
		.name = L"Default texture",
	};

	static char* default_texture_data() {
		static char data[BYTE_COUNT];
		for (int i = 0; i < BYTE_COUNT; i += 4) {
			data[i] = 255u;
			data[i + 1] = 0u;
			data[i + 2] = 255u;
			data[i + 3] = 255u;
		}
		return data;
	}

	TextureAsset* TextureRegistry::default_texture(d3d::GraphicsDevice* device) {
		const static UUID default_texture_uuid = UUID{5596545107579832553};
		static auto data = default_texture_data();
		static TextureAsset asset = {
			.uuid = default_texture_uuid,
			.description = DEFAULT_TEXTURE_CREATION_INFO,
			.data = device->create_texture(DEFAULT_TEXTURE_CREATION_INFO, data),
		};
		return &asset;
	}

} // namespace Sapfire::assets
