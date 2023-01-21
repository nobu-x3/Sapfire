#include "Buffer.h"
#include "engine/Log.h"
#include "engine/renderer/Renderer.h"
#include "engine/renderer/opengl/OpenGLBuffer.h"

VertexBuffer *VertexBuffer::Create(float *vertices, size_t size)
{
	auto api = Renderer::GetAPI();
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
	auto api = Renderer::GetAPI();
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
