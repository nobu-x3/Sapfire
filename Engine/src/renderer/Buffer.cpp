#include "engpch.h"
#include "Buffer.h"
#include "renderer/RendererAPI.h"
#include "renderer/opengl/OpenGLBuffer.h"
#include <memory>

void BufferLayout::CalculateOffsets()
{
	uint32_t offset = 0;
	mStride = 0;
	for (auto &element : mElements)
	{
		element.Offset = offset;
		offset += element.Size;
		mStride += element.Size;
	}
}
Ref<VertexBuffer> VertexBuffer::Create()
{
	auto api = RendererAPI::GetAPI();
	switch (api)
	{
	case RendererAPI::API::OpenGL: {
		return CreateRef<OpenGLVertexBuffer>();
	}
	default:
		return nullptr;
	}
	ENGINE_ERROR("Given rendering API is not supported yet.");
	return nullptr;
}

Ref<IndexBuffer> IndexBuffer::Create()
{
	auto api = RendererAPI::GetAPI();
	switch (api)
	{
	case RendererAPI::API::OpenGL: {
		return CreateRef<OpenGLIndexBuffer>();
	}
	default:
		return nullptr;
	}
	ENGINE_ERROR("Given rendering API is not supported yet.");
	return nullptr;
}
