#pragma once

#include "Sapfire/Core.h"
#include "Sapfire/renderer/Buffer.h"

class OpenGLVertexBuffer : public VertexBuffer
{
	public:
	OpenGLVertexBuffer();
	~OpenGLVertexBuffer();
	virtual void SetData(void *buffer, size_t size) override;
	virtual void Bind() const override;
	virtual void Unbind() const override;
	inline virtual uint32_t GetCount() const override { return mCount; }

	inline virtual void SetLayout(const BufferLayout &layout) override { mLayout = layout; }
	inline virtual const BufferLayout &GetLayout() const override { return mLayout; }

	private:
	RendererID mRendererID;
	uint32_t mCount;
	BufferLayout mLayout;
};

class OpenGLIndexBuffer : public IndexBuffer
{
	public:
	OpenGLIndexBuffer();
	~OpenGLIndexBuffer();
	virtual void SetData(void *buffer, size_t size) override;
	virtual void Bind() const override;
	virtual void Unbind() const override;
	inline virtual uint32_t GetCount() const override { return mCount; }

	private:
	RendererID mRendererID;
	uint32_t mCount;
};
