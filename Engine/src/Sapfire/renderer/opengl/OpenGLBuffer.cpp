#include "engpch.h"
#include "OpenGLBuffer.h"
#include <glad/glad.h>

namespace Sapfire
{
	////////////////////////// VERTEX BUFFER //////////////////////////////////////

	OpenGLVertexBuffer::OpenGLVertexBuffer()
	{
		PROFILE_FUNCTION();
		glGenBuffers(1, &mRendererID);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		PROFILE_FUNCTION();
		glDeleteBuffers(1, &mRendererID);
	}

	void OpenGLVertexBuffer::set_data(void* buffer, size_t size)
	{
		PROFILE_FUNCTION();
		mCount = size;
		glBindBuffer(GL_ARRAY_BUFFER, mRendererID);
		glBufferData(GL_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW);
	}

	void OpenGLVertexBuffer::bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, mRendererID);
	}

	void OpenGLVertexBuffer::unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	////////////////////////// INDEX BUFFER //////////////////////////////////////

	OpenGLIndexBuffer::OpenGLIndexBuffer()
	{
		PROFILE_FUNCTION();
		glGenBuffers(1, &mRendererID);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		PROFILE_FUNCTION();
		glDeleteBuffers(1, &mRendererID);
	}

	void OpenGLIndexBuffer::set_data(void* buffer, size_t size)
	{
		PROFILE_FUNCTION();
		mCount = size;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW);
	}

	void OpenGLIndexBuffer::bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererID);
	}

	void OpenGLIndexBuffer::unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}