#pragma once

#include "RendererAPI.h"
#include "engine/Math.h"

enum class WindowAPI
{
	None = 0,
	SDL = 1,
};

class Renderer
{
	public:
	static void BeginScene(class Camera &camera);
	static void EndScene();
	static void Submit(const Ref<VertexArray> &vertexArray, const Ref<class Shader> &shader);

	private:
	static Matrix4 sViewProjection;
};
