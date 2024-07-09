#include "subeditors/material_editor.h"
#include "widgets/material_inspector.h"
#include "ImGuiFileDialog.h"
#include "imgui.h"

using namespace Sapfire;

SMaterialEditor::SMaterialEditor(Sapfire::assets::AssetManager* am) :
	SSubeditor("Material Editor"), m_AssetManager(*am), m_OpenedMaterial(nullptr) {
	m_Widgets.push_back(Sapfire::stl::make_unique<widgets::SMaterialInspector>(Sapfire::mem::Editor));
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
					widgets::SMaterialInspector* inspector = static_cast<widgets::SMaterialInspector*>(m_Widgets[EWidgetOrder::MaterialInspector].get());
					if (inspector) {
						inspector->current_material(m_OpenedMaterial);
					}
				}
			}
		}
		ImGuiFileDialog::Instance()->Close();
	}
}
