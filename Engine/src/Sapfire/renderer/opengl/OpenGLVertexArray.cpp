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
		return 0;
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

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		PROFILE_FUNCTION();
		glBindVertexArray(mRendererID);
		vertexBuffer->Bind();
		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		auto elements = layout.GetElements();
		for (const auto& element : elements)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index, element.GetComponentCount(), ConvertShaderDataTypeToGLenum(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE, layout.GetStride(),
				(const void*)element.Offset);
			index++;
		}
		mVertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::AddIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		PROFILE_FUNCTION();
		glBindVertexArray(mRendererID);
		indexBuffer->Bind();
		mIndexBuffers.push_back(indexBuffer);
	}

	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(mRendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}
}