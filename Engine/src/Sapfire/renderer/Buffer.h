#pragma once

#include "Sapfire/core/Core.h"
#include "Sapfire/core/Log.h"

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

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
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
		ENGINE_ERROR("Unknown shader data type");
		return 0;
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
			: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{
		}

		uint32_t GetComponentCount() const
		{
			if (Type == ShaderDataType::Bool)
				return 1;
			return Size / 4;
		}
	};

	class BufferLayout
	{
	public:
		BufferLayout() {}
		BufferLayout(const std::initializer_list<BufferElement>& layout) : mElements(layout) { CalculateOffsets(); }
		inline const std::vector<BufferElement>& GetElements() const { return mElements; }
		inline uint32_t GetStride() const { return mStride; }

	private:
		void CalculateOffsets();

	private:
		std::vector<BufferElement> mElements;
		uint32_t mStride = 0;
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {}
		virtual void SetData(void* buffer, size_t size) = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual uint32_t GetCount() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;
		virtual const BufferLayout& GetLayout() const = 0;
		static Ref<VertexBuffer> Create();

	private:
		RendererID mRendererID;
	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {}
		virtual void SetData(void* buffer, size_t size) = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual uint32_t GetCount() const = 0;
		static Ref<IndexBuffer> Create();

	private:
		RendererID mRendererID;
	};
}