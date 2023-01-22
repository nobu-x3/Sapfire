#include "OpenGLRendererAPI.h"
#include "GL/glew.h"
#include "engine/engpch.h"

void OpenGLRendererAPI::ClearScreen()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRendererAPI::SetClearColor(Vector4 &color)
{
	glClearColor(color.x, color.y, color.z, color.w);
}

void OpenGLRendererAPI::Draw(const Ref<VertexArray> &vertexArray)
{
	for (auto buffer : vertexArray->GetIndexBuffers())
	{
		buffer->Bind();
		glDrawElements(GL_TRIANGLES, buffer->GetCount(), GL_UNSIGNED_INT, nullptr);
	}
}
