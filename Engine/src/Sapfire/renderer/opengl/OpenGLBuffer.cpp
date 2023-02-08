#include "engpch.h"
#include "OpenGLBuffer.h"
#include <glad/glad.h>
#include <cstdint>

namespace Sapfire
{
	////////////////////////// VERTEX BUFFER //////////////////////////////////////

	OpenGLVertexBuffer::OpenGLVertexBuffer()
	{
		glGenBuffers(1, &mRendererID);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &mRendererID);
	}

	void OpenGLVertexBuffer::SetData(void* buffer, size_t size)
	{
		mCount = size;
		glBindBuffer(GL_ARRAY_BUFFER, mRendererID);
		glBufferData(GL_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW);
	}

	void OpenGLVertexBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, mRendererID);
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	////////////////////////// INDEX BUFFER //////////////////////////////////////

	OpenGLIndexBuffer::OpenGLIndexBuffer()
	{
		glGenBuffers(1, &mRendererID);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		glDeleteBuffers(1, &mRendererID);
	}

	void OpenGLIndexBuffer::SetData(void* buffer, size_t size)
	{
		mCount = size;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW);
	}

	void OpenGLIndexBuffer::Bind() const
	{

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererID);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}