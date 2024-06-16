#pragma once

#include "Sapfire.h"

#include "core/stl/shared_ptr.h"
#include "subeditors/subeditor.h"
#include "widgets/scene_hierarchy.h"

namespace Sapfire {
	class ECManager;
}

using on_entity_selected_callback = Sapfire::stl::function<void(Sapfire::stl::optional<Sapfire::Entity>)>;

class SLevelEditor final : public SSubeditor {
public:
	SLevelEditor(Sapfire::d3d::GraphicsDevice* gfx_device);
	static Sapfire::stl::shared_ptr<SLevelEditor> level_editor();
	Sapfire::assets::AssetManager& asset_manager() { return m_AssetManager; }

private:
	void on_entity_selected(Sapfire::stl::optional<Sapfire::Entity> entity);
	void on_mesh_added();

private:
	Sapfire::stl::unique_ptr<Sapfire::ECManager> m_ECManager;
	Sapfire::stl::vector<on_entity_selected_callback> m_EntitySelectedCallbacks;
	Sapfire::assets::AssetManager m_AssetManager;

	static Sapfire::stl::shared_ptr<SLevelEditor> s_Instance;
};
