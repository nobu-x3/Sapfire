#include "engpch.h"
#include "VertexArray.h"
#include <glad/glad.h>
#include "Sapfire/renderer/RendererAPI.h"
#include "Sapfire/renderer/opengl/OpenGLVertexArray.h"

namespace Sapfire
{
	VertexArray* VertexArray::Create()
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
}