#include "subeditors/level_editor.h"
#include "subeditors/subeditor.h"
#include "widgets/asset_browser.h"
#include "widgets/entity_inspector.h"
#include "widgets/scene_hierarchy.h"
#include "widgets/scene_view.h"

using namespace Sapfire;

Sapfire::stl::shared_ptr<SLevelEditor> SLevelEditor::s_Instance{nullptr};
Sapfire::stl::shared_ptr<SLevelEditor> SLevelEditor::level_editor() { return s_Instance; }

SLevelEditor::SLevelEditor(Sapfire::d3d::GraphicsDevice* gfx_device) :
	SSubeditor("Level Editor"), m_ECManager(stl::make_unique<ECManager>(mem::ENUM::Editor)),
	m_AssetManager(assets::AssetManagerCreationDesc{
		.device = gfx_device,
		.mesh_registry_path = "mesh_registry.db",
		.texture_registry_path = "texture_registry.db",
	}) {
	s_Instance.reset(this);
	m_Widgets.push_back(
		stl::make_unique<widgets::SSceneHierarchy>(mem::ENUM::Editor, m_ECManager.get(), BIND_EVENT_FN(SLevelEditor::on_entity_selected)));
	auto entity_inspector = stl::make_unique<widgets::SEntityInspector>(mem::ENUM::Editor, m_ECManager.get());
	m_EntitySelectedCallbacks.push_back(BIND_EVENT_FN_FOR_OBJ(entity_inspector.get(), widgets::SEntityInspector::select_entity));
	m_Widgets.push_back(std::move(entity_inspector));
	auto parent = m_ECManager->create_entity();
	auto& transform = m_ECManager->engine_component<components::Transform>(parent);
	transform.position(DirectX::XMVECTOR{0.f, 0.f, 5.f});
	auto& parent_name = m_ECManager->engine_component<components::NameComponent>(parent);
	auto test_component = stl::make_shared<components::TestCustomComponent>(mem::ENUM::Editor, "HELLOOOO");
	m_ECManager->add_component(parent, test_component);
	m_ECManager->add_engine_component<components::MovementComponent>(parent);
	parent_name.name("Parent");
	m_ECManager->create_entity();
	auto child = m_ECManager->create_entity();
	auto& child_transform = m_ECManager->engine_component<components::Transform>(child);
	auto& child_name = m_ECManager->engine_component<components::NameComponent>(child);
	child_name.name("Child");
	child_transform.parent(parent.id().index);
	m_ECManager->create_entity();
	m_ECManager->create_entity();
	m_ECManager->create_entity();
	m_ECManager->create_entity();
	m_Widgets.emplace_back(stl::make_unique<widgets::AssetBrowser>(mem::ENUM::Editor));
	m_AssetManager.import_texture("assets/textures/ceramics.jpg");
	auto scene_view = stl::make_unique<widgets::SSceneView>(mem::ENUM::Editor, m_ECManager.get(), gfx_device);
	scene_view->add_render_component(parent, "assets/models/monkey.obj");
	scene_view->add_render_component(parent, "assets/models/cube.obj");
	m_Widgets.push_back(std::move(scene_view));
    assets::SceneWriter writer{m_ECManager.get(), &m_AssetManager};
    writer.serialize("test_scene.scene");
}

void SLevelEditor::on_entity_selected(Sapfire::stl::optional<Sapfire::Entity> entity) {
	for (auto& f : m_EntitySelectedCallbacks) {
		f(entity);
	}
}
