#pragma once

#include "Sapfire/core/Core.h"

namespace Sapfire
{
	enum class ShaderDataType
	{
		None = 0,
		Float,
		Int,
		Vec2,
		Vec3,
		Vec4,
		Vec2i,
		Vec3i,
		Vec4i,
		Mat3,
		Mat4,
		Bool
	};

	static uint32_t shader_data_type_size(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:
			return 4;
		case ShaderDataType::Int:
			return 4;
		case ShaderDataType::Vec2:
			return 8;
		case ShaderDataType::Vec3:
			return 12;
		case ShaderDataType::Vec4:
			return 16;
		case ShaderDataType::Vec2i:
			return 8;
		case ShaderDataType::Vec3i:
			return 12;
		case ShaderDataType::Vec4i:
			return 16;
		case ShaderDataType::Mat3:
			return 36;
		case ShaderDataType::Mat4:
			return 64;
		case ShaderDataType::Bool:
			return 1;
		default:
			return 0;
		}
	}

	struct BufferElement
	{
		std::string Name;
		ShaderDataType Type;
		uint32_t Size;
		uint32_t Offset;
		bool Normalized;
		BufferElement() {}
		BufferElement(const std::string& name, ShaderDataType type, bool normalized = false)
			: Name(name), Type(type), Size(shader_data_type_size(type)), Offset(0), Normalized(normalized)
		{
		}

		uint32_t get_component_count() const
		{
			if (Type == ShaderDataType::Bool)
				return 1;
			return Size / 4;
		}
	};

	class BufferLayout
	{
	public:
		BufferLayout() = default;
		BufferLayout(const std::initializer_list<BufferElement>& layout) : mElements(layout) { calculate_offsets(); }
		const std::vector<BufferElement>& get_elements() const { return mElements; }
		uint32_t get_stride() const { return mStride; }

	private:
		void calculate_offsets();

	private:
		std::vector<BufferElement> mElements;
		uint32_t mStride = 0;
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;
		virtual void set_data(void* buffer, size_t size) = 0;
		virtual void bind() const = 0;
		virtual void unbind() const = 0;
		virtual uint32_t get_size() const = 0;
		inline RendererID get_renderer_id() const { return mRendererID; }
		virtual void set_layout(const BufferLayout& layout) = 0;
		virtual const BufferLayout& get_layout() const = 0;
		static Ref<VertexBuffer> create();

	private:
		RendererID mRendererID;
	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;
		virtual void set_data(void* buffer, size_t size) = 0;
		virtual void bind() const = 0;
		virtual void unbind() const = 0;
		virtual uint32_t get_size() const = 0;
		static Ref<IndexBuffer> create();

	private:
		RendererID mRendererID;
	};
}