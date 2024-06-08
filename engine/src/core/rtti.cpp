#include "engpch.h"

#include <DirectXMath.h>
#include "core/rtti.h"

namespace Sapfire::rtti {

	void set_rtti_field_value(rtti_object* obj, rtti_field* field, void* value) {
		switch (field->type) {
		case rtti_type::U8:
			{
				auto* ptr = static_cast<u8*>(obj->head) + field->offset;
				auto* data = static_cast<u8*>(value);
				*ptr = *data;
				break;
			}
		case rtti_type::U16:
			{
				auto* ptr = static_cast<u8*>(obj->head) + field->offset;
				auto* data = static_cast<u16*>(value);
				*ptr = *data;
				break;
			}
		case rtti_type::U32:
			{
				auto* ptr = static_cast<u8*>(obj->head) + field->offset;
				auto* data = static_cast<u32*>(value);
				*ptr = *data;
				break;
			}
		case rtti_type::U64:
			{
				auto* ptr = static_cast<u8*>(obj->head) + field->offset;
				auto* data = static_cast<u64*>(value);
				*ptr = *data;
				break;
			}
		case rtti_type::S8:
			{
				auto* ptr = static_cast<u8*>(obj->head) + field->offset;
				auto* data = static_cast<s8*>(value);
				*ptr = *data;
				break;
			}
		case rtti_type::S16:
			{
				auto* ptr = static_cast<u8*>(obj->head) + field->offset;
				auto* data = static_cast<s16*>(value);
				*ptr = *data;
				break;
			}
		case rtti_type::S32:
			{
				auto* ptr = static_cast<u8*>(obj->head) + field->offset;
				auto* data = static_cast<s32*>(value);
				*ptr = *data;
				break;
			}
		case rtti_type::S64:
			{
				auto* ptr = static_cast<u8*>(obj->head) + field->offset;
				auto* data = static_cast<s64*>(value);
				*ptr = *data;
				break;
			}
		case rtti_type::F32:
			{
				auto* ptr = static_cast<u8*>(obj->head) + field->offset;
				auto* data = static_cast<f32*>(value);
				*ptr = *data;
				break;
			}
		case rtti_type::F64:
			{
				auto* ptr = static_cast<u8*>(obj->head) + field->offset;
				auto* data = static_cast<f64*>(value);
				*ptr = *data;
				break;
			}
		case rtti_type::BOOL:
			{
				auto* ptr = static_cast<u8*>(obj->head) + field->offset;
				auto* data = static_cast<bool*>(value);
				*ptr = *data;
				break;
			}
		case rtti_type::STRING:
			{
				auto* ptr = static_cast<u8*>(obj->head) + field->offset;
				auto* data = static_cast<stl::string*>(value)->data();
				auto* string = reinterpret_cast<stl::string*>(ptr);
				*string = stl::string{data};
				break;
			}
		case rtti_type::XMVECTOR:
			{
				auto* ptr = static_cast<u8*>(obj->head) + field->offset;
				auto& data = *static_cast<stl::array<f32, 3>*>(value);
				*reinterpret_cast<DirectX::XMVECTOR*>(ptr) = DirectX::XMVectorSet(data[0], data[1], data[2], 0.0f);
				break;
			}
		case rtti_type::REFERENCE:
			{
                auto* ptr = static_cast<u8*>(obj->head) + field->offset;
                auto data = *static_cast<Sapfire::UUID*>(value);
                *ptr = data;
				break;
			}
		}
		if (field->setter) {
			field->setter();
		}
	}

	void get_rtti_field_value(const rtti_object* obj, const rtti_field* field, void** value) {
		auto* ptr = static_cast<u8*>(obj->head);
		*value = ptr + field->offset;
	}
} // namespace Sapfire::rtti
