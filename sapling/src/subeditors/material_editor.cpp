#include "subeditors/material_editor.h"
#include "imgui.h"
#include "subeditors/subeditor.h"

SMaterialEditor::SMaterialEditor(Sapfire::assets::AssetManager* am) :
	SSubeditor("Material Editor"), m_AssetManager(*am), m_OpenedMaterial(nullptr) {}

void SMaterialEditor::draw_menu() {
	if (ImGui::BeginMenu("Material Editor")) {
		if (ImGui::MenuItem("Open Material", "CTRL+M+O")) {
		}
		if (ImGui::MenuItem("Save Material", "CTRL+M+S")) {
		}
		ImGui::EndMenu();
	}
}
