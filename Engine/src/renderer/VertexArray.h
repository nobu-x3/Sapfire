#pragma once
#include "engine/renderer/Buffer.h"
class VertexArray
{
	public:
	virtual ~VertexArray(){};
	virtual void AddVertexBuffer(const Ref<VertexBuffer> &vertexBuffer) = 0;
	virtual void AddIndexBuffer(const Ref<IndexBuffer> &indexBuffer) = 0;
	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;
	virtual const std::vector<Ref<VertexBuffer>> &GetVertexBuffers() const = 0;
	virtual const std::vector<Ref<IndexBuffer>> &GetIndexBuffers() const = 0;
	static VertexArray *Create();
};
