#include "engpch.h"
#include "OpenGLBuffer.h"
#include <glad/glad.h>

namespace Sapfire
{
	////////////////////////// VERTEX BUFFER //////////////////////////////////////

	OpenGLVertexBuffer::OpenGLVertexBuffer()
	{
		PROFILE_FUNCTION();
		glCreateBuffers(1, &mRendererID);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		PROFILE_FUNCTION();
		glDeleteBuffers(1, &mRendererID);
	}

	void OpenGLVertexBuffer::set_data(void* buffer, size_t size)
	{
		PROFILE_FUNCTION();
		mSize = static_cast<uint32_t>(size);
		glNamedBufferStorage(mRendererID, size, buffer, 0);
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
		glCreateBuffers(1, &mRendererID);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		PROFILE_FUNCTION();
		glDeleteBuffers(1, &mRendererID);
	}

	void OpenGLIndexBuffer::set_data(void* buffer, size_t size)
	{
		PROFILE_FUNCTION();
		mSize = static_cast<uint32_t>(size);
		glNamedBufferStorage(mRendererID, size, buffer, 0);
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