#pragma once
#include "engine/Math.h"
#include "engine/renderer/VertexArray.h"
class RendererAPI
{
	public:
	enum class API
	{
		None = 0,
		OpenGL = 1
	};

	public:
	virtual void ClearScreen() = 0;
	virtual void SetClearColor(Vector4 &color) = 0;
	virtual void Draw(const Ref<VertexArray> &vertexArray) = 0;
	inline static API GetAPI() { return sAPI; }

	private:
	static API sAPI;
};
