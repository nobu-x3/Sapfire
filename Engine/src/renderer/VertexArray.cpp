#include "engpch.h"
#include "VertexArray.h"
#include "GL/glew.h"
#include "renderer/RendererAPI.h"
#include "renderer/opengl/OpenGLVertexArray.h"

VertexArray *VertexArray::Create()
{
	auto api = RendererAPI::GetAPI();

	switch (api)
	{
	case RendererAPI::API::OpenGL: {
		return new OpenGLVertexArray();
	}
	default:
		return nullptr;
	}
	return nullptr;
}
