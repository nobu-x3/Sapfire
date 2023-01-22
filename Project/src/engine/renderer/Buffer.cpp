#include "Buffer.h"
#include "engine/renderer/Renderer.h"
#include "engine/renderer/opengl/OpenGLBuffer.h"

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
VertexBuffer *VertexBuffer::Create(float *vertices, size_t size)
{
	auto api = Renderer::GetRendererAPI();
	switch (api)
	{
	case RendererAPI::OpenGL: {
		return new OpenGLVertexBuffer(vertices, size);
	}
	default:
		return nullptr;
	}
	ENGINE_ERROR("Given rendering API is not supported yet.");
	return nullptr;
}

IndexBuffer *IndexBuffer::Create(uint32_t *indices, size_t size)
{
	auto api = Renderer::GetRendererAPI();
	switch (api)
	{
	case RendererAPI::OpenGL: {
		return new OpenGLIndexBuffer(indices, size);
	}
	default:
		return nullptr;
	}
	ENGINE_ERROR("Given rendering API is not supported yet.");
	return nullptr;
}
