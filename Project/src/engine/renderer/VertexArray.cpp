#include "VertexArray.h"
#include "GL/glew.h"
#include "engine/renderer/Renderer.h"
#include "engine/renderer/opengl/OpenGLVertexArray.h"

VertexArray *VertexArray::Create()
{
	auto api = Renderer::GetRendererAPI();

	switch (api)
	{
	case RendererAPI::OpenGL: {
		return new OpenGLVertexArray();
	}
	default:
		return nullptr;
	}
	return nullptr;
}
