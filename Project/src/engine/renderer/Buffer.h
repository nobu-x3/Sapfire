#pragma once

#include "engine/Core.h"

class VertexBuffer
{
	public:
	virtual ~VertexBuffer() {}
	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;
	virtual uint32_t GetCount() const = 0;
	static VertexBuffer *Create(float *vertices, size_t size);

	private:
	RendererID mRendererID;
};

class IndexBuffer
{
	public:
	virtual ~IndexBuffer() {}
	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;
	virtual uint32_t GetCount() const = 0;
	static IndexBuffer *Create(uint32_t *indices, size_t size);

	private:
	RendererID mRendererID;
};
