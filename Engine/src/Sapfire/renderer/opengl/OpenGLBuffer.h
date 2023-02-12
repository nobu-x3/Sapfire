#pragma once

#include "Sapfire/core/Core.h"
#include "Sapfire/renderer/Buffer.h"

namespace Sapfire
{
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer();
		~OpenGLVertexBuffer();
		virtual void set_data(void* buffer, size_t size) override;
		virtual void bind() const override;
		virtual void unbind() const override;
		virtual uint32_t get_count() const override { return mCount; }

		virtual void set_layout(const BufferLayout& layout) override { mLayout = layout; }
		virtual const BufferLayout& get_layout() const override { return mLayout; }

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
		virtual void set_data(void* buffer, size_t size) override;
		virtual void bind() const override;
		virtual void unbind() const override;
		virtual uint32_t get_count() const override { return mCount; }

	private:
		RendererID mRendererID;
		uint32_t mCount;
	};
}