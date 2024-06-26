#include "subeditors/level_editor.h"
#include "subeditors/subeditor.h"
#include "widgets/asset_browser.h"
#include "widgets/entity_inspector.h"
#include "widgets/scene_hierarchy.h"
#include "widgets/scene_view.h"

using namespace Sapfire;

SLevelEditor* SLevelEditor::s_Instance{nullptr};
SLevelEditor* SLevelEditor::level_editor() {
	return s_Instance;
}

SLevelEditor::SLevelEditor(Sapfire::d3d::GraphicsDevice* gfx_device) :
	SSubeditor("Level Editor"), m_ECManager(stl::make_unique<ECManager>(mem::ENUM::Editor)),
	m_AssetManager(assets::AssetManagerCreationDesc{
		.device = gfx_device,
		.mesh_registry_path = "mesh_registry.db",
		.texture_registry_path = "texture_registry.db",
	}) {
	s_Instance = this;
	m_Widgets.push_back(
		stl::make_unique<widgets::SSceneHierarchy>(mem::ENUM::Editor, m_ECManager.get(), BIND_EVENT_FN(SLevelEditor::on_entity_selected)));
	auto entity_inspector = stl::make_unique<widgets::SEntityInspector>(mem::ENUM::Editor, m_ECManager.get());
	m_EntitySelectedCallbacks.push_back(BIND_EVENT_FN_FOR_OBJ(entity_inspector.get(), widgets::SEntityInspector::select_entity));
	m_Widgets.push_back(std::move(entity_inspector));
	assets::SceneWriter writer{m_ECManager.get(), &m_AssetManager};
	m_Widgets.emplace_back(stl::make_unique<widgets::AssetBrowser>(mem::ENUM::Editor));
	auto scene_view = stl::make_unique<widgets::SSceneView>(mem::ENUM::Editor, m_ECManager.get(), gfx_device);
	writer.deserealize("test_scene.scene", [&](Sapfire::Entity entity, const Sapfire::RenderComponentResourcePaths& resource_paths) {
		scene_view->add_render_component(entity, resource_paths);
	});
	m_Widgets.push_back(std::move(scene_view));
}

void SLevelEditor::on_entity_selected(Sapfire::stl::optional<Sapfire::Entity> entity) {
	for (auto& f : m_EntitySelectedCallbacks) {
		f(entity);
	}
}
