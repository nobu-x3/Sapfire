#include "subeditors/material_editor.h"
#include "ImGuiFileDialog.h"
#include "imgui.h"
#include "widgets/asset_browser.h"
#include "widgets/material_inspector.h"
#include "widgets/material_preview_settings.h"
#include "widgets/scene_view.h"

using namespace Sapfire;

SMaterialEditor::SMaterialEditor(Sapfire::assets::AssetManager* am, Sapfire::d3d::GraphicsDevice* device) :
	SSubeditor("Material Editor"), m_AssetManager(*am), m_OpenedMaterial(nullptr), m_ECManager(stl::make_unique<ECManager>(mem::Editor)) {
	m_Widgets.push_back(Sapfire::stl::make_unique<widgets::SMaterialInspector>(Sapfire::mem::Editor));
	m_MaterialEntity = m_ECManager->create_entity();
	auto& transform = m_ECManager->engine_component<components::Transform>(m_MaterialEntity);
	transform.position({0, 0, 5});
	auto scene_view = Sapfire::stl::make_unique<widgets::SSceneView>(mem::Editor, "Preview", m_ECManager.get(), device);
	scene_view->add_render_component(m_MaterialEntity,
									 {
										 .mesh_path = "assets/models/cube.obj",
										 .texture_path = "assets/textures/ceramics.jpg",
										 .material_path = "assets/materials/default.mat",
									 });
	auto& render_component = m_ECManager->engine_component<components::RenderComponent>(m_MaterialEntity);
	m_Widgets.push_back(std::move(scene_view));
	m_Widgets.push_back(stl::make_unique<SMaterialPreviewSettings>(mem::Editor, &render_component));
	m_Widgets.push_back(stl::make_unique<widgets::SAssetBrowser>(mem::Editor, "Preview Asset Browser"));
	m_Widgets[EWidgetOrder::PreviewSettings]->set_visible(false);
	m_Widgets[EWidgetOrder::AssetBrowser]->set_visible(false);
}

void SMaterialEditor::draw_menu() {
	if (ImGui::BeginMenu("Material Editor")) {
		if (ImGui::MenuItem("Open Material", "CTRL+M+O")) {
			IGFD::FileDialogConfig config{};
			config.path = Sapfire::fs::FileSystem::root_directory();
			config.countSelectionMax = 1;
			ImGuiFileDialog::Instance()->OpenDialog("OpenMatDlg", "Open material", ".mat", config);
		}
		if (m_OpenedMaterial && ImGui::MenuItem("Save Material", "CTRL+M+S")) {
		}
		if (ImGui::BeginMenu("View")) {
			bool preview_settings_shown = m_Widgets[EWidgetOrder::PreviewSettings]->is_visible();
			if (ImGui::MenuItem("Preview settings", nullptr, &preview_settings_shown)) {
				m_Widgets[EWidgetOrder::PreviewSettings]->set_visible(!m_Widgets[EWidgetOrder::PreviewSettings]->is_visible());
				m_Widgets[EWidgetOrder::AssetBrowser]->set_visible(m_Widgets[EWidgetOrder::PreviewSettings]->is_visible());
			}
			bool asset_browser_shown = m_Widgets[EWidgetOrder::AssetBrowser]->is_visible();
			if (ImGui::MenuItem("Asset Browser", nullptr, &asset_browser_shown)) {
				m_Widgets[EWidgetOrder::AssetBrowser]->set_visible(!m_Widgets[EWidgetOrder::AssetBrowser]->is_visible());
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
}

bool SMaterialEditor::update(Sapfire::f32 delta_time) {
	bool ret_val = SSubeditor::update(delta_time);
	draw_open_material_dialog();
	return ret_val;
}

void SMaterialEditor::draw_open_material_dialog() {
	if (ImGuiFileDialog::Instance()->Display("OpenMatDlg")) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			stl::string filepath = ImGuiFileDialog::Instance()->GetFilePathName();
			if (!filepath.empty()) {
				auto relative_path = Sapfire::fs::relative_path(filepath);
				if (m_AssetManager.path_material_map().contains(relative_path)) {
					m_OpenedMaterial = m_AssetManager.get_material(filepath);
					widgets::SMaterialInspector* inspector =
						static_cast<widgets::SMaterialInspector*>(m_Widgets[EWidgetOrder::MaterialInspector].get());
					if (inspector) {
						inspector->current_material(m_OpenedMaterial);
					}
				}
			}
		}
		ImGuiFileDialog::Instance()->Close();
	}
}
