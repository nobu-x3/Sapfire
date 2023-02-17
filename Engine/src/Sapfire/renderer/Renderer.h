#pragma once

#include "Sapfire/renderer/RendererAPI.h"
namespace Sapfire
{
	enum class WindowAPI
	{
		None = 0,
		SDL = 1,
		GLFW = 2
	};

	class Renderer
	{
	public:
		static void begin_scene(class Camera& camera, const Ref<class UniformBuffer>& uniformBuffer);
		static void end_scene();
		static void submit(const Ref<VertexArray>& vertexArray, const Ref<class Shader>& shader);
		static void submit_mesh(const Ref<class Mesh>& mesh, const Ref<Shader>& shader);
		static WindowAPI get_window_api() { return sWindowAPI; }
		
		// EVENTS	///////////
		static void on_window_resize(uint16_t width, uint16_t height);

	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
			glm::mat4 ViewMatrix;
			glm::mat4 ProjectionMatrix;
		};
		static SceneData* sSceneData;
		static WindowAPI sWindowAPI;
	};
}