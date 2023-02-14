#include "engpch.h"
#include "Buffer.h"
#include "Sapfire/renderer/RendererAPI.h"
#include "Sapfire/renderer/opengl/OpenGLBuffer.h"
#include <memory>

namespace Sapfire
{
	void BufferLayout::calculate_offsets()
	{
		PROFILE_FUNCTION();
		uint32_t offset = 0;
		mStride = 0;
		for (auto& element : mElements)
		{
			element.Offset = offset;
			offset += element.Size;
			mStride += element.Size;
		}
	}
	
	Ref<VertexBuffer> VertexBuffer::create()
	{
		PROFILE_FUNCTION();
		switch (auto api = RendererAPI::get_api())
		{
		case RendererAPI::API::OpenGL: {
			return create_ref<OpenGLVertexBuffer>();
		}
		default:
			return nullptr;
		}
	}

	Ref<IndexBuffer> IndexBuffer::create()
	{
		PROFILE_FUNCTION();
		switch (auto api = RendererAPI::get_api())
		{
		case RendererAPI::API::OpenGL: {
			return create_ref<OpenGLIndexBuffer>();
		}
		default:
			return nullptr;
		}
	}
}