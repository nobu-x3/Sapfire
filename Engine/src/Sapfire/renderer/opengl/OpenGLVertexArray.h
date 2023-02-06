#pragma once

#include "Sapfire/renderer/VertexArray.h"

class OpenGLVertexArray : public VertexArray
{
	public:
	OpenGLVertexArray();
	~OpenGLVertexArray();
	virtual void AddVertexBuffer(const Ref<VertexBuffer> &vertexBuffer) override;
	virtual void AddIndexBuffer(const Ref<IndexBuffer> &indexBuffer) override;
	virtual void Bind() const override;
	virtual void Unbind() const override;
	inline virtual const std::vector<Ref<VertexBuffer>> &GetVertexBuffers() const override
	{
		return mVertexBuffers;
	}
	inline virtual const std::vector<Ref<IndexBuffer>> &GetIndexBuffers() const override { return mIndexBuffers; }

	private:
	std::vector<Ref<VertexBuffer>> mVertexBuffers;
	std::vector<Ref<IndexBuffer>> mIndexBuffers;
	RendererID mRendererID;
};
