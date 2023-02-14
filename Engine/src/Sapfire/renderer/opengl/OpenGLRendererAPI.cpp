#include "engpch.h"
#include "OpenGLRendererAPI.h"
#include <glad/glad.h>
#include "Sapfire/renderer/Mesh.h"

namespace Sapfire
{
	void OpenGLRendererAPI::init()
	{
		PROFILE_FUNCTION();
		glEnable(GL_BLEND);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
	}

	void OpenGLRendererAPI::clear_screen()
	{
		PROFILE_FUNCTION();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::set_clear_color(glm::vec4& color)
	{
		PROFILE_FUNCTION();
		glClearColor(color.x, color.y, color.z, color.w);
	}

	void OpenGLRendererAPI::draw(const Ref<VertexArray>& vertexArray)
	{
		PROFILE_FUNCTION();
		for (auto buffer : vertexArray->get_index_buffers())
		{
			glDrawElements(GL_TRIANGLES, buffer->get_size(), GL_UNSIGNED_INT, nullptr);
		}
	}

	void OpenGLRendererAPI::draw(size_t count)
	{
		PROFILE_FUNCTION();
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(count), GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRendererAPI::set_viewport(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		PROFILE_FUNCTION();
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::draw_mesh()
	{
		PROFILE_FUNCTION();
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex),
			(const void*)offsetof(Mesh::Vertex, Position));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex),
			(const void*)offsetof(Mesh::Vertex, Normal));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex),
			(const void*)offsetof(Mesh::Vertex, Tangent));

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex),
			(const void*)offsetof(Mesh::Vertex, Binormal));

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex),
			(const void*)offsetof(Mesh::Vertex, Texcoord));
	}
}