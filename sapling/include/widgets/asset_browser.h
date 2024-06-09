#pragma once

#include "Sapfire.h"
#include "widgets/widget.h"

namespace widgets {
    enum class AssetType : Sapfire::u8 {
        Mesh, Texture, Material
    };

    struct AssetDragAndDropPayload {
        Sapfire::UUID uuid;
        AssetType type;
    };

	class AssetBrowser final : public IWidget {
	public:
		explicit AssetBrowser(const Sapfire::stl::string& mesh_registry_path, const Sapfire::stl::string& texture_registry_path);
		bool update(Sapfire::f32 delta_time) override;
		inline Sapfire::assets::MeshRegistry* mesh_registry_ptr() { return &m_MeshRegistry; }
		inline const Sapfire::assets::MeshRegistry& mesh_registry() const { return m_MeshRegistry; }
		inline Sapfire::assets::TextureRegistry* texture_registry_ptr() { return &m_TextureRegistry; }
		inline const Sapfire::assets::TextureRegistry& texture_registry() const { return m_TextureRegistry; }

	private:
		Sapfire::stl::string m_MeshRegistryPath;
		Sapfire::assets::MeshRegistry m_MeshRegistry;
        Sapfire::assets::TextureRegistry m_TextureRegistry{};
	};
} // namespace widgets
