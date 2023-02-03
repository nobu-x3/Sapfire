#pragma once

#include "RendererAPI.h"

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
	static WindowAPI GetWindowAPI() { return sWindowAPI; }

	private:
	struct SceneData
	{
		glm::mat4 ViewProjectionMatrix;
	};
	static SceneData *sSceneData;
	static WindowAPI sWindowAPI;
};
