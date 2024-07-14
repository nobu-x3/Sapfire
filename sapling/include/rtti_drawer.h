#pragma once

#include <DirectXMath.h>
#include "Sapfire.h"
#include "components/component.h"
#include "components/render_component.h"
#include "core/core.h"
#include "core/rtti.h"
#include "imgui.h"
#include "widgets/asset_browser.h"

template <typename T>
void draw_rtti(T* component) {
	using namespace Sapfire;
	auto& type_info = component->get_rtti();
	for (auto& field : type_info.fields) {
		auto& name = field.name;
		void* value = nullptr;
		rtti::get_rtti_field_value(&type_info, &field, &value);
		switch (field.type) {
		case rtti::rtti_type::XMVECTOR:
			{
				auto data = *static_cast<DirectX::XMVECTOR*>(value);
				stl::array<f32, 3> vec = {DirectX::XMVectorGetX(data), DirectX::XMVectorGetY(data), DirectX::XMVectorGetZ(data)};
				if (ImGui::DragFloat3(name.c_str(), vec.data())) {
					rtti::set_rtti_field_value(&type_info, &field, static_cast<void*>(vec.data()));
				}
			}
			break;
		case rtti::rtti_type::U8:
		case rtti::rtti_type::U16:
		case rtti::rtti_type::U32:
		case rtti::rtti_type::S8:
		case rtti::rtti_type::S16:
		case rtti::rtti_type::S32:
		case rtti::rtti_type::U64:
		case rtti::rtti_type::S64:
			{
				auto data = static_cast<s32*>(value);
				if (ImGui::InputInt(name.c_str(), data)) {
					rtti::set_rtti_field_value(&type_info, &field, reinterpret_cast<void*>(data));
				}
			}
			break;
		case rtti::rtti_type::BOOL:
			{
				auto data = static_cast<bool*>(value);
				if (ImGui::Checkbox(name.c_str(), data)) {
					rtti::set_rtti_field_value(&type_info, &field, reinterpret_cast<void*>(data));
				}
			}
			break;
		case rtti::rtti_type::STRING:
			{
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
				auto* data = static_cast<stl::string*>(value);
				if (Funcs::MyInputText(name.c_str(), data, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16))) {
					rtti::set_rtti_field_value(&type_info, &field, static_cast<void*>(data));
				}
			}
			break;
		case rtti::rtti_type::F32:
			{
				auto data = static_cast<f32*>(value);
				if (ImGui::InputFloat(name.c_str(), data)) {
					rtti::set_rtti_field_value(&type_info, &field, static_cast<void*>(data));
				}
			}
		case rtti::rtti_type::F64:
			{
				auto data = static_cast<f64*>(value);
				if (ImGui::InputDouble(name.c_str(), data)) {
					rtti::set_rtti_field_value(&type_info, &field, static_cast<void*>(data));
				}
			}
			break;
		case rtti::rtti_type::REFERENCE:
			{
				auto data = static_cast<Sapfire::UUID*>(value);
				ImGui::InputScalar(name.c_str(), ImGuiDataType_U64, data, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ASSET_UUID")) {
						IM_ASSERT(payload->DataSize == sizeof(widgets::AssetDragAndDropPayload));
						widgets::AssetDragAndDropPayload payload_asset = *(const widgets::AssetDragAndDropPayload*)payload->Data;
						if ((payload_asset.type == widgets::EAssetType::Mesh && field.ref_type == rtti::rtti_reference_type::Mesh) ||
							(payload_asset.type == widgets::EAssetType::Texture && field.ref_type == rtti::rtti_reference_type::Texture) ||
							(payload_asset.type == widgets::EAssetType::Material &&
							 field.ref_type == rtti::rtti_reference_type::Material)) {
							*data = payload_asset.uuid;
							rtti::set_rtti_field_value(&type_info, &field, static_cast<void*>(data));
							return;
						}
					}
					ImGui::EndDragDropTarget();
				}
			}
			break;
		}
	}
	ImGui::Spacing();
	ImGui::Separator();
}

void draw_rtti(Sapfire::stl::shared_ptr<Sapfire::components::IComponent> custom_component);
