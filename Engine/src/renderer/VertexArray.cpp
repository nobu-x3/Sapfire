#include "VertexArray.h"
#include "GL/glew.h"
#include "engine/renderer/RendererAPI.h"
#include "engine/renderer/opengl/OpenGLVertexArray.h"

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
