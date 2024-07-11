#include "subeditors/level_editor.h"
#include "ImGuiFileDialog.h"
#include "imgui.h"
#include "subeditors/subeditor.h"
#include "widgets/asset_browser.h"
#include "widgets/entity_inspector.h"
#include "widgets/scene_hierarchy.h"
#include "widgets/scene_view.h"

using namespace Sapfire;

SLevelEditor* SLevelEditor::s_Instance{nullptr};
SLevelEditor* SLevelEditor::level_editor() { return s_Instance; }

SLevelEditor::SLevelEditor(Sapfire::d3d::GraphicsDevice* gfx_device, Sapfire::assets::AssetManager* am,
						   const Sapfire::stl::string& scene_path, Sapfire::stl::function<void()> asset_imported_callback) :
	SSubeditor("Level Editor"),
	m_ECManager(stl::make_unique<ECManager>(mem::ENUM::Editor)), m_AssetManager(*am) {
	s_Instance = this;
	m_Widgets.push_back(
		stl::make_unique<widgets::SSceneHierarchy>(mem::ENUM::Editor, m_ECManager.get(), BIND_EVENT_FN(SLevelEditor::on_entity_selected)));
	auto entity_inspector = stl::make_unique<widgets::SEntityInspector>(mem::ENUM::Editor, m_ECManager.get());
	m_EntitySelectedCallbacks.push_back(BIND_EVENT_FN_FOR_OBJ(entity_inspector.get(), widgets::SEntityInspector::select_entity));
	m_Widgets.push_back(std::move(entity_inspector));
	auto asset_browser = stl::make_unique<widgets::SAssetBrowser>(mem::ENUM::Editor, "Asset Browser");
	asset_browser->register_asset_imported_events(asset_imported_callback);
	m_Widgets.emplace_back(std::move(asset_browser));
	auto scene_view = stl::make_unique<widgets::SSceneView>(mem::ENUM::Editor, "Scene View", m_ECManager.get(), gfx_device);
	if (!scene_path.empty()) {
		assets::SceneWriter writer{m_ECManager.get(), &m_AssetManager};
		writer.deserealize(scene_path, [&](Sapfire::Entity entity, const Sapfire::RenderComponentResourcePaths& resource_paths) {
			scene_view->add_render_component(entity, resource_paths);
		});
	}
	m_SceneViewIndex = static_cast<u32>(m_Widgets.size());
	m_Widgets.push_back(std::move(scene_view));
}

void SLevelEditor::on_entity_selected(Sapfire::stl::optional<Sapfire::Entity> entity) {
	for (auto& f : m_EntitySelectedCallbacks) {
		f(entity);
	}
}

void SLevelEditor::draw_menu() {
	if (ImGui::BeginMenu("Level Editor")) {
		if (ImGui::MenuItem("New scene...")) {
			m_ECManager->reset();
			m_CurrentSceneName = "";
		}
		if (ImGui::MenuItem("Open scene...")) {
			IGFD::FileDialogConfig config{};
			config.path = Sapfire::fs::FileSystem::root_directory();
			ImGuiFileDialog::Instance()->OpenDialog("OpenSceneFileDlg", "Open scene", ".scene", config);
		}
		ImGui::EndMenu();
	}
}

bool SLevelEditor::update(Sapfire::f32 delta_time) {
	bool ret_val = SSubeditor::update(delta_time);
	draw_open_scene_dialog();
	return ret_val;
}

void SLevelEditor::draw_open_scene_dialog() {
	if (ImGuiFileDialog::Instance()->Display("OpenSceneFileDlg")) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			stl::string filepath = ImGuiFileDialog::Instance()->GetFilePathName();
			if (!filepath.empty()) {
				m_CurrentSceneName = filepath;
				m_ECManager->reset();
				assets::SceneWriter writer{m_ECManager.get(), &m_AssetManager};
				writer.deserealize(filepath, [&](Sapfire::Entity entity, const Sapfire::RenderComponentResourcePaths& resource_paths) {
					auto widget = m_Widgets[m_SceneViewIndex].get();
					static_cast<widgets::SSceneView*>(widget)->add_render_component(entity, resource_paths);
				});
			}
		}
		ImGuiFileDialog::Instance()->Close();
	}
}
