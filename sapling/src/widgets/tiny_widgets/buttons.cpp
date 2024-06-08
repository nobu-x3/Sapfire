#include "widgets/tiny_widgets/buttons.h"

namespace widgets {

	bool centered_button(const char* label, ImVec2 button_size, float alignment) {
		ImGuiStyle& style = ImGui::GetStyle();
		float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
		float avail = ImGui::GetContentRegionAvail().x;
		float off = (avail - size - (button_size.x / 2.f)) * alignment;
		if (off > 0.0f)
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
		return ImGui::Button(label, button_size);
	}

} // namespace widgets
