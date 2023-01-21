#pragma once

#include "engine/Core.h"
#include "engine/renderer/Buffer.h"

class OpenGLVertexBuffer : public VertexBuffer
{
	public:
	OpenGLVertexBuffer(float *vertices, uint32_t count);
	~OpenGLVertexBuffer();
	virtual void Bind() const override;
	virtual void Unbind() const override;
	inline virtual uint32_t GetCount() const override { return mCount; }

	private:
	RendererID mRendererID;
	uint32_t mCount;
};

class OpenGLIndexBuffer : public IndexBuffer
{
	public:
	OpenGLIndexBuffer(uint32_t *indeces, uint32_t count);
	~OpenGLIndexBuffer();
	virtual void Bind() const override;
	virtual void Unbind() const override;
	inline virtual uint32_t GetCount() const override { return mCount; }

	private:
	RendererID mRendererID;
	uint32_t mCount;
};
