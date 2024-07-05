#include "widgets/asset_browser.h"
#include "ImGuiFileDialog.h"
#include "icons.h"
#include "imgui.h"
#include "subeditors/level_editor.h"

namespace widgets {

	static Sapfire::stl::vector<event_fn> asset_importer_events{};

	AssetType get_asset_type(Sapfire::stl::string_view filename) {
		if (Sapfire::fs::extension(filename) == ".obj") {
			return AssetType::Mesh;
		} else if (Sapfire::fs::extension(filename) == ".dds" || Sapfire::fs::extension(filename) == ".png") {
			return AssetType::Texture;
		}
		return AssetType::Unknown;
	}

	void AssetBrowser::register_asset_imported_events(event_fn fn) { asset_importer_events.push_back(fn); }

	bool AssetBrowser::update(Sapfire::f32 delta_time) {
		if (ImGui::Begin("Asset Browser")) {
			if (ImGui::IsWindowHovered() && m_ShowContextMenu) {
				ImGui::OpenPopup("asset_browser_context_menu");
			}
			if (ImGui::BeginPopup("asset_browser_context_menu")) {
				if (ImGui::MenuItem("Import")) {
					IGFD::FileDialogConfig config{};
					config.path = Sapfire::fs::FileSystem::root_directory();
					ImGuiFileDialog::Instance()->OpenDialog("ImportAssetDlg", "Import asset", ".obj,.png,.dds,.jpg", config);
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
			if (ImGuiFileDialog::Instance()->Display("ImportAssetDlg")) {
				if (ImGuiFileDialog::Instance()->IsOk()) {
					auto selection = ImGuiFileDialog::Instance()->GetSelection();
					for (auto& [filename, filepath] : selection) {
						AssetType type = get_asset_type(filename);
						switch (type) {
						case AssetType::Mesh:
							SLevelEditor::level_editor()->asset_manager().import_mesh(Sapfire::fs::relative_path(filepath));
							execute_asset_imported_events();
							break;
						case AssetType::Texture:
							SLevelEditor::level_editor()->asset_manager().import_texture(Sapfire::fs::relative_path(filepath));
							execute_asset_imported_events();
							break;
						default:
							break;
						}
					}
				}
				ImGuiFileDialog::Instance()->Close();
			}
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
							auto asset_name = Sapfire::fs::file_name(path);
							ImVec2 real_estate = ImGui::GetWindowSize();
							ImVec2 sz = ImGui::CalcTextSize(asset_name.c_str());
							ImVec2 cursor = ImGui::GetCursorPos();
							AssetDragAndDropPayload payload{mesh_asset.uuid, AssetType::Mesh};
							ImGui::BeginGroup();
							ImGui::PushID(asset_name.c_str());
							ImGui::InvisibleButton("invisible_button", {sz.x, sz.y + 64 + style.ItemSpacing.y});
							ImGui::SetCursorPos(cursor);
							if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
								ImGui::SetDragDropPayload("DND_ASSET_UUID", &payload, sizeof(AssetDragAndDropPayload));
								ImGui::EndDragDropSource();
							}
							ImGui::SetCursorPosX(cursor.x + sz.x / 2 - 32);
							ImGui::Image(icons::get_im_id(icons::MESH_ICON_64_ID), {64, 64});
							ImGui::AlignTextToFramePadding();
							ImGui::Text("%s", asset_name.c_str());
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
							auto asset_name = Sapfire::fs::file_name(path);
							ImVec2 real_estate = ImGui::GetWindowSize();
							ImVec2 sz = ImGui::CalcTextSize(asset_name.c_str());
							ImVec2 cursor = ImGui::GetCursorPos();
							AssetDragAndDropPayload payload{texture_asset.uuid, AssetType::Texture};
							ImGui::BeginGroup();
							ImGui::PushID(asset_name.c_str());
							ImGui::InvisibleButton("invisible_button", {sz.x, sz.y + 64 + style.ItemSpacing.y});
							ImGui::SetCursorPos(cursor);
							if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
								ImGui::SetDragDropPayload("DND_ASSET_UUID", &payload, sizeof(AssetDragAndDropPayload));
								ImGui::EndDragDropSource();
							}
							ImGui::SetCursorPosX(cursor.x + sz.x / 2 - 32);
							ImGui::Image(icons::get_im_id(icons::IMAGE_ICON_64_ID), {64, 64});
							ImGui::AlignTextToFramePadding();
							ImGui::Text("%s", asset_name.c_str());
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

	void AssetBrowser::on_mouse_button_event(Sapfire::MouseButtonEvent& event) {
		if (!event.is_down()) {
			switch (event.button()) {
			case Sapfire::MouseButton::RMB:
				m_ShowContextMenu = true;
				break;
			default:
				break;
			}
		}
		// This will be called after internal ImGui selects and such.
		// @NOTE: Probably why it's sometimes laggy and button clicks don't do anything.
		if (event.is_down()) {
			m_ShowContextMenu = false;
		}
	}

	void AssetBrowser::execute_asset_imported_events() {
		for (auto& fn : asset_importer_events) {
			if (fn)
				fn();
		}
	}

} // namespace widgets
