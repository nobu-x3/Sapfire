#include "subeditors/subeditor.h"
#include "events/input_event.h"
#include "imgui.h"

using namespace Sapfire;

SSubeditor::SSubeditor(Sapfire::stl::string_view editor_name) : m_Name(editor_name) {}

bool SSubeditor::update(f32 delta_time) {
	ImGui::Begin(m_Name.c_str(), nullptr);
	const ImGuiID dockspace_id = ImGui::GetID("###SubeditorDockspace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));
	const ImGuiIO io = ImGui::GetIO();
	for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) {
		if (ImGui::IsMouseClicked(i)) {
			switch (i) {
			case 0:
				{
					for (auto& widget : m_Widgets) {
						MouseButtonEvent e{MouseButton::LMB, true};
						if (widget && widget->is_visible())
							widget->on_mouse_button_event(e);
					}
				}
				break;
			case 1:
				{
					for (auto& widget : m_Widgets) {
						MouseButtonEvent e{MouseButton::RMB, true};
						if (widget && widget->is_visible())
							widget->on_mouse_button_event(e);
					}
				}
				break;
			case 2:
				{
					for (auto& widget : m_Widgets) {
						MouseButtonEvent e{MouseButton::MMB, true};
						if (widget && widget->is_visible())
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
						if (widget && widget->is_visible())
							widget->on_mouse_button_event(e);
					}
				}
				break;
			case 1:
				{
					for (auto& widget : m_Widgets) {
						MouseButtonEvent e{MouseButton::RMB, false};
						if (widget && widget->is_visible())
							widget->on_mouse_button_event(e);
					}
				}
				break;
			case 2:
				{
					for (auto& widget : m_Widgets) {
						MouseButtonEvent e{MouseButton::MMB, false};
						if (widget && widget->is_visible())
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
		if (widget && widget->is_visible())
			return_val &= widget->update(delta_time);
	}
	ImGui::End();
	return return_val;
}

void SSubeditor::render(Sapfire::d3d::GraphicsContext& gfx_ctx) {
	for (auto&& widget : m_Widgets) {
		if (widget && widget->is_visible())
			widget->render(gfx_ctx);
	}
}
