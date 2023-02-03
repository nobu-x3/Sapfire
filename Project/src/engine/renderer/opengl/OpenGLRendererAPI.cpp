#include "OpenGLRendererAPI.h"
#include "GL/glew.h"
#include "engine/engpch.h"

void OpenGLRendererAPI::Init()
{
	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
}

void OpenGLRendererAPI::ClearScreen()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRendererAPI::SetClearColor(glm::vec4 &color)
{
	glClearColor(color.x, color.y, color.z, color.w);
}

void OpenGLRendererAPI::Draw(const Ref<VertexArray> &vertexArray)
{
	for (auto buffer : vertexArray->GetIndexBuffers())
	{
		glDrawElements(GL_TRIANGLES, buffer->GetCount(), GL_UNSIGNED_INT, nullptr);
	}
}

void OpenGLRendererAPI::Draw(size_t count)
{
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
}
