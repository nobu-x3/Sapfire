#pragma once

#include "Sapfire/core/Core.h"
#include "Sapfire/renderer/Buffer.h"

namespace Sapfire
{
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer();
		~OpenGLVertexBuffer() override;
		virtual void set_data(void* buffer, size_t size) override;
		virtual void bind() const override;
		virtual void unbind() const override;
		virtual uint32_t get_size() const override { return mSize; }

	private:
		uint32_t mSize;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer();
		~OpenGLIndexBuffer() override;
		virtual void set_data(void* buffer, size_t size) override;
		virtual void bind() const override;
		virtual void unbind() const override;
		virtual uint32_t get_size() const override { return mSize; }

	private:
		uint32_t mSize;
	};

	class OpenGLUniformBuffer : public UniformBuffer
	{
	public:
		OpenGLUniformBuffer(uint32_t index, const BufferLayout& layout);
		~OpenGLUniformBuffer() override;
		virtual void set_data(void* data) const override;
	private:
		RendererID mRendererID;
		BufferLayout mLayout;
		uint32_t mIndex;
	};
}
