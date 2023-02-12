#include "engpch.h"
#include "OpenGLVertexArray.h"
#include <glad/glad.h>

namespace Sapfire
{
	static GLenum ConvertShaderDataTypeToGLenum(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:
			return GL_FLOAT;
		case ShaderDataType::Vec2:
			return GL_FLOAT;
		case ShaderDataType::Vec3:
			return GL_FLOAT;
		case ShaderDataType::Vec4:
			return GL_FLOAT;
		case ShaderDataType::Mat3:
			return GL_FLOAT;
		case ShaderDataType::Mat4:
			return GL_FLOAT;
		case ShaderDataType::Int:
			return GL_INT;
		case ShaderDataType::Vec2i:
			return GL_INT;
		case ShaderDataType::Vec3i:
			return GL_INT;
		case ShaderDataType::Vec4i:
			return GL_INT;
		case ShaderDataType::Bool:
			return GL_BOOL;
		default:
			return 0;
		}
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		PROFILE_FUNCTION();
		glCreateVertexArrays(1, &mRendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		PROFILE_FUNCTION();
		glDeleteVertexArrays(1, &mRendererID);
	}

	void OpenGLVertexArray::add_vertex_buffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		PROFILE_FUNCTION();
		glBindVertexArray(mRendererID);
		vertexBuffer->bind();
		uint32_t index = 0;
		const auto& layout = vertexBuffer->get_layout();
		auto elements = layout.get_elements();
		for (const auto& element : elements)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index, element.get_component_count(), ConvertShaderDataTypeToGLenum(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE, layout.get_stride(),
				(const void*)element.Offset);
			index++;
		}
		mVertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::add_index_buffer(const Ref<IndexBuffer>& indexBuffer)
	{
		PROFILE_FUNCTION();
		glBindVertexArray(mRendererID);
		indexBuffer->bind();
		mIndexBuffers.push_back(indexBuffer);
	}

	void OpenGLVertexArray::bind() const
	{
		glBindVertexArray(mRendererID);
	}

	void OpenGLVertexArray::unbind() const
	{
		glBindVertexArray(0);
	}
}