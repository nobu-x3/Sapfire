#include "OpenGLBuffer.h"
#include "engine/engpch.h"
#include <GL/glew.h>
#include <cstdint>

////////////////////////// VERTEX BUFFER //////////////////////////////////////

OpenGLVertexBuffer::OpenGLVertexBuffer(float *vertices, uint32_t count) : mCount(count)
{
	glGenBuffers(1, &mRendererID);
	glBindBuffer(GL_ARRAY_BUFFER, mRendererID);
	glBufferData(GL_ARRAY_BUFFER, count * 8 * sizeof(float), vertices, GL_STATIC_DRAW);
}

OpenGLVertexBuffer::~OpenGLVertexBuffer()
{
	glDeleteBuffers(1, &mRendererID);
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

OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t *indeces, uint32_t count) : mCount(count)
{
	glGenBuffers(1, &mRendererID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indeces, GL_STATIC_DRAW);
}

OpenGLIndexBuffer::~OpenGLIndexBuffer()
{
	glDeleteBuffers(1, &mRendererID);
}

void OpenGLIndexBuffer::Bind() const
{

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererID);
}

void OpenGLIndexBuffer::Unbind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
