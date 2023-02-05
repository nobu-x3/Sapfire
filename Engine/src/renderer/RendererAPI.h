#pragma once
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
	virtual void Init() = 0;
	virtual void ClearScreen() = 0;
	virtual void SetClearColor(glm::vec4 &color) = 0;
	virtual void Draw(const Ref<VertexArray> &vertexArray) = 0;
	virtual void Draw(size_t count) = 0;
	virtual void DrawMesh() = 0;
	inline static API GetAPI() { return sAPI; }

	private:
	static API sAPI;
};
