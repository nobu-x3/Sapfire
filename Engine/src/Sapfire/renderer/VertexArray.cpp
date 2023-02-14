#include "engpch.h"
#include "VertexArray.h"
#include "Sapfire/renderer/RendererAPI.h"
#include "Sapfire/renderer/opengl/OpenGLVertexArray.h"

namespace Sapfire
{
	Ref<VertexArray> VertexArray::create()
	{
		switch (auto api = RendererAPI::get_api())
		{
		case RendererAPI::API::OpenGL: {
			return create_ref<OpenGLVertexArray>();
		}
		default:
			return nullptr;
		}
	}
}