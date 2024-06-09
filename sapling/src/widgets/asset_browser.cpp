#include "widgets/asset_browser.h"
#include "icons.h"
#include "imgui.h"

namespace widgets {

	constexpr Sapfire::u32 PADDING = 5;

	AssetBrowser::AssetBrowser(const Sapfire::stl::string& mesh_registry_path, const Sapfire::stl::string& texture_registry_path) :
		m_MeshRegistryPath(mesh_registry_path), m_MeshRegistry(m_MeshRegistryPath), m_TextureRegistry(texture_registry_path) {}

	bool AssetBrowser::update(Sapfire::f32 delta_time) {
		if (ImGui::Begin("Asset Browser")) {
			static ImGuiTextFilter filter;
			filter.Draw("Filter by name: inc, -exc");
			ImGui::NewLine();
			ImGuiStyle& style = ImGui::GetStyle();
			for (auto&& [path, asset] : m_MeshRegistry.path_asset_map()) {
				if (filter.PassFilter(path.c_str())) {
					ImVec2 real_estate = ImGui::GetWindowSize();
					ImVec2 sz = ImGui::CalcTextSize(path.c_str());
					ImVec2 cursor = ImGui::GetCursorPos();
					AssetDragAndDropPayload payload{asset.uuid, AssetType::Mesh};
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
		}
		ImGui::End();
		return true;
	}
} // namespace widgets
