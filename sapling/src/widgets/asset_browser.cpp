#include "widgets/asset_browser.h"
#include "icons.h"
#include "imgui.h"
#include "subeditors/level_editor.h"

namespace widgets {

	bool AssetBrowser::update(Sapfire::f32 delta_time) {
		if (ImGui::Begin("Asset Browser")) {
			if (ImGui::RadioButton("Meshes", m_CurrentAssetTypeFilter == AssetType::Mesh)) {
				m_CurrentAssetTypeFilter = AssetType::Mesh;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Textures", m_CurrentAssetTypeFilter == AssetType::Texture)) {
				m_CurrentAssetTypeFilter = AssetType::Texture;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("Materials", m_CurrentAssetTypeFilter == AssetType::Material)) {
				m_CurrentAssetTypeFilter = AssetType::Material;
			}
			ImGui::SameLine();
			static ImGuiTextFilter filter;
			filter.Draw("Filter by name: inc, -exc");
			ImGui::NewLine();
			ImGuiStyle& style = ImGui::GetStyle();
			switch (m_CurrentAssetTypeFilter) {
			case AssetType::Mesh:
				{
					for (auto&& [path, mesh_asset] : SLevelEditor::level_editor()->asset_manager().path_mesh_map()) {
						if (filter.PassFilter(path.c_str())) {
							ImVec2 real_estate = ImGui::GetWindowSize();
							ImVec2 sz = ImGui::CalcTextSize(path.c_str());
							ImVec2 cursor = ImGui::GetCursorPos();
							AssetDragAndDropPayload payload{mesh_asset.uuid, AssetType::Mesh};
							ImGui::BeginGroup();
							ImGui::PushID(path.c_str());
							ImGui::InvisibleButton("invisible_button", {sz.x, sz.y + 64 + style.ItemSpacing.y});
							ImGui::SetCursorPos(cursor);
							if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
								ImGui::SetDragDropPayload("DND_ASSET_UUID", &payload, sizeof(AssetDragAndDropPayload));
								ImGui::EndDragDropSource();
							}
							ImGui::SetCursorPosX(cursor.x + sz.x / 2 - 32);
							ImGui::Image(icons::get_im_id(icons::MESH_ICON_64_ID), {64, 64});
							ImGui::AlignTextToFramePadding();
							ImGui::Text("%s", path.c_str());
							ImGui::PopID();
							ImGui::EndGroup();
							if (cursor.x + (2 * sz.x) < real_estate.x)
								ImGui::SameLine();
						}
					}
					break;
				}
			case AssetType::Texture:
				{
					for (auto&& [path, texture_asset] : SLevelEditor::level_editor()->asset_manager().path_texture_map()) {
						if (filter.PassFilter(path.c_str())) {
							ImVec2 real_estate = ImGui::GetWindowSize();
							ImVec2 sz = ImGui::CalcTextSize(path.c_str());
							ImVec2 cursor = ImGui::GetCursorPos();
							AssetDragAndDropPayload payload{texture_asset.uuid, AssetType::Texture};
							ImGui::BeginGroup();
							ImGui::PushID(path.c_str());
							ImGui::InvisibleButton("invisible_button", {sz.x, sz.y + 64 + style.ItemSpacing.y});
							ImGui::SetCursorPos(cursor);
							if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
								ImGui::SetDragDropPayload("DND_ASSET_UUID", &payload, sizeof(AssetDragAndDropPayload));
								ImGui::EndDragDropSource();
							}
							ImGui::SetCursorPosX(cursor.x + sz.x / 2 - 32);
							ImGui::Image(icons::get_im_id(icons::IMAGE_ICON_64_ID), {64, 64});
							ImGui::AlignTextToFramePadding();
							ImGui::Text("%s", path.c_str());
							ImGui::PopID();
							ImGui::EndGroup();
							if (cursor.x + (2 * sz.x) < real_estate.x)
								ImGui::SameLine();
						}
					}
					break;
				}
			}
		}
		ImGui::End();
		return true;
	}
} // namespace widgets
