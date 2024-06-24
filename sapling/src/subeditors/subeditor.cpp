#include "subeditors/subeditor.h"
#include "events/input_event.h"
#include "imgui.h"
#include "widgets/widget.h"

using namespace Sapfire;

SSubeditor::SSubeditor(Sapfire::stl::string_view editor_name) : m_Name(editor_name) {}

bool SSubeditor::update(f32 delta_time) {
	//ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
	ImGui::Begin(m_Name.c_str(), nullptr);
	ImGuiID dockspace_id = ImGui::GetID("###SubeditorDockspace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));
	ImGuiIO io = ImGui::GetIO();
	for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) {
		if (ImGui::IsMouseClicked(i)) {
			switch (i) {
			case 0:
				{
					for (auto& widget : m_Widgets) {
						MouseButtonEvent e{MouseButton::LMB, true};
						widget->on_mouse_button_event(e);
					}
				}
				break;
			case 1:
				{
					for (auto& widget : m_Widgets) {
						MouseButtonEvent e{MouseButton::RMB, true};
						widget->on_mouse_button_event(e);
					}
				}
				break;
			case 2:
				{
					for (auto& widget : m_Widgets) {
						MouseButtonEvent e{MouseButton::MMB, true};
						widget->on_mouse_button_event(e);
					}
				}
				break;
			default:
				break;
			}
		}
		if (ImGui::IsMouseReleased(i)) {
			switch (i) {
			case 0:
				{
					for (auto& widget : m_Widgets) {
						MouseButtonEvent e{MouseButton::LMB, false};
						widget->on_mouse_button_event(e);
					}
				}
				break;
			case 1:
				{
					for (auto& widget : m_Widgets) {
						MouseButtonEvent e{MouseButton::RMB, false};
						widget->on_mouse_button_event(e);
					}
				}
				break;
			case 2:
				{
					for (auto& widget : m_Widgets) {
						MouseButtonEvent e{MouseButton::MMB, false};
						widget->on_mouse_button_event(e);
					}
				}
				break;
			default:
				break;
			}
		}
	}
    bool return_val = true;
	for (auto& widget : m_Widgets) {
		return_val &= widget->update(delta_time);
	}
	ImGui::End();
    return return_val;
}

void SSubeditor::render(Sapfire::d3d::GraphicsContext& gfx_ctx) {
	for (auto&& widget : m_Widgets) {
		widget->render(gfx_ctx);
	}
}
