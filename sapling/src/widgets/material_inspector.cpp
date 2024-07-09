#include "widgets/material_inspector.h"
#include "imgui.h"

using namespace Sapfire;

constexpr f32 ROUGHNESS_SLIDER_SENSITIVITY = 0.005f;

namespace widgets {
	bool SMaterialInspector::update(Sapfire::f32 delta_time) {
		ImGui::Begin("Material Inspector", nullptr);
		if (!m_CurrentMaterial) {
			ImGui::Text("Open material...");
			ImGui::End();
			return true;
		}
		struct Funcs {
			static int MyResizeCallback(ImGuiInputTextCallbackData* data) {
				if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
					stl::string* my_str = (stl::string*)data->UserData;
					my_str->resize(data->BufSize);
					data->Buf = my_str->data();
				}
				return 0;
			}
			static bool MyInputText(const char* label, stl::string* my_str, const ImVec2& size = ImVec2(0, 0),
									ImGuiInputTextFlags flags = 0) {
				IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
				return ImGui::InputText(label, my_str->data(), (size_t)my_str->size() + 1, // +1 because '\0'
										flags | ImGuiInputTextFlags_CallbackResize, Funcs::MyResizeCallback, (void*)my_str);
			}
		};
		Funcs::MyInputText("Name", &m_CurrentMaterial->material.name, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16));
		ImGui::InputFloat("Roughness", &m_CurrentMaterial->material.roughness, ROUGHNESS_SLIDER_SENSITIVITY);
		float albedo[4]{
			m_CurrentMaterial->material.diffuse_albedo.x,
			m_CurrentMaterial->material.diffuse_albedo.y,
			m_CurrentMaterial->material.diffuse_albedo.z,
			m_CurrentMaterial->material.diffuse_albedo.w,
		};
		if (ImGui::ColorEdit4("Diffuse Albedo", albedo)) {
			m_CurrentMaterial->material.diffuse_albedo = {albedo[0], albedo[1], albedo[2], albedo[3]};
		}
		float fresnel[3]{
			m_CurrentMaterial->material.fresnel_r0.x,
			m_CurrentMaterial->material.fresnel_r0.y,
			m_CurrentMaterial->material.fresnel_r0.z,
		};
		if (ImGui::ColorEdit3("Fresnel", fresnel)) {
			m_CurrentMaterial->material.fresnel_r0 = {fresnel[0], fresnel[1], fresnel[2]};
		}
		ImGui::End();
		return true;
	}
} // namespace widgets
