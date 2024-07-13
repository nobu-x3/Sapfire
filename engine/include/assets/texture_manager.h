#pragma once

#include "core/core.h"
#include "core/uuid.h"
#include "render/resources.h"

namespace Sapfire {
	namespace d3d {
		class GraphicsDevice;
	}
} // namespace Sapfire

namespace Sapfire::assets {

	struct SFAPI TextureResource {
		u32 cpu_idx{0};
		u32 gpu_idx{0};
	};

	struct SFAPI TextureManager {
		Sapfire::stl::unordered_map<Sapfire::stl::string, TextureResource> texture_resources;
		Sapfire::stl::unordered_map<Sapfire::UUID, Sapfire::stl::string> uuid_to_path_map;
		void add(const Sapfire::stl::string& path, Sapfire::UUID uuid, TextureResource resource);
	};

	struct SFAPI TextureAsset {
		UUID uuid;
		d3d::TextureCreationDesc description;
		d3d::Texture data;
	};

	class SFAPI TextureRegistry {
	public:
		explicit TextureRegistry(const stl::string& texture_registry_path);
		explicit TextureRegistry() = default;
		~TextureRegistry() = default;
		TextureRegistry(const TextureRegistry&) = delete;
		TextureRegistry(TextureRegistry&&) = delete;
		TextureRegistry& operator=(const TextureRegistry&) = delete;
		TextureRegistry& operator=(TextureRegistry&&) = delete;
		void import_texture(d3d::GraphicsDevice& device, const stl::string& path);
		void import_texture(d3d::GraphicsDevice& device, const stl::string& path, const d3d::TextureCreationDesc& desc, UUID uuid = {});
		void move_texture(d3d::GraphicsDevice& device, const stl::string& old_path, const stl::string& new_path);
		void release_texture(const stl::string& path);
		void serialize();
		void deserialize(d3d::GraphicsDevice& device, const stl::string& data);
		TextureAsset* get(const stl::string& path) const;
		TextureAsset* get(UUID uuid) const;
		stl::string get_path(UUID uuid) const;
		const stl::unordered_map<stl::string, TextureAsset>& path_asset_map() const { return m_PathToTextureAssetMap; }
		stl::string to_string();

		static void create_default(const stl::string& registry_file_path);
		static TextureAsset* default_texture(d3d::GraphicsDevice& device);

	private:
		stl::string m_RegistryFilePath;
		stl::unordered_map<stl::string, TextureAsset> m_PathToTextureAssetMap{};
		stl::unordered_map<UUID, stl::string> m_UUIDToPathMap{};
	};
} // namespace Sapfire::assets
