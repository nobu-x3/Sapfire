#pragma once
#include "Sapfire/renderer/Buffer.h"

namespace Sapfire
{
	class VertexArray
	{
	public:
		virtual ~VertexArray() {};
		virtual void add_vertex_buffer(const Ref<VertexBuffer>& vertexBuffer) = 0;
		virtual void add_index_buffer(const Ref<IndexBuffer>& indexBuffer) = 0;
		virtual void bind() const = 0;
		virtual void unbind() const = 0;
		virtual const std::vector<Ref<VertexBuffer>>& get_vertex_buffers() const = 0;
		virtual const std::vector<Ref<IndexBuffer>>& get_index_buffers() const = 0;
		static VertexArray* create();
	};
}