#pragma once

#include "Sapfire/renderer/VertexArray.h"

namespace Sapfire
{
	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		~OpenGLVertexArray();
		virtual void add_vertex_buffer(const Ref<VertexBuffer>& vertexBuffer) override;
		virtual void add_index_buffer(const Ref<IndexBuffer>& indexBuffer) override;
		virtual void bind() const override;
		virtual void unbind() const override;

		virtual const std::vector<Ref<VertexBuffer>>& get_vertex_buffers() const override
		{
			return mVertexBuffers;
		}

		virtual const std::vector<Ref<IndexBuffer>>& get_index_buffers() const override { return mIndexBuffers; }

	private:
		std::vector<Ref<VertexBuffer>> mVertexBuffers;
		std::vector<Ref<IndexBuffer>> mIndexBuffers;
		RendererID mRendererID;
	};
}