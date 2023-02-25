#pragma once

#include "Sapfire/renderer/RendererAPI.h"

namespace Sapfire
{
	struct DirectionalLight
	{
		glm::vec3 Direction = {0.0f, 0.0f, 0.0f};
		glm::vec3 DiffuseColor = {0.0f, 0.0f, 0.0f};
		glm::vec3 SpecColor = {0.0f, 0.0f, 0.0f};
		float Intensity = 0.f;
	};

	struct PointLight
	{
		glm::vec3 Position = {0.0f, 0.0f, 0.0f};;
		glm::vec3 DiffuseColor = {0.0f, 0.0f, 0.0f};;
		glm::vec3 SpecColor = {0.0f, 0.0f, 0.0f};;
		float Intensity = 0.f;
		float Radius = 0.f;
	};

	struct LightEnvironment
	{
		DirectionalLight DirectionalLights[4];
		std::vector<PointLight> PointLights;
	};

	enum class WindowAPI
	{
		None = 0,
		SDL = 1,
		GLFW = 2
	};

	class Renderer
	{
	public:
		static void begin_scene(const class Camera& camera, const glm::mat4& cameraTransform,
		                        const Ref<class UniformBuffer>& uniformBuffer);
		static void end_scene();
		static void submit(const Ref<VertexArray>& vertexArray, const Ref<class Shader>& shader);
		static void submit_mesh(const Ref<class Mesh>& mesh, const Ref<Shader>& shader);
		static void submit_mesh(const Ref<class Mesh>& mesh, const glm::mat4& transform);
		static void submit_mesh(Mesh& mesh, const glm::mat4& transform);
		static WindowAPI get_window_api() { return sWindowAPI; }

		// EVENTS	///////////
		static void on_window_resize(uint16_t width, uint16_t height);

	private:
		struct SceneData
		{
			glm::mat4 ViewMatrix;
			glm::mat4 ProjectionMatrix;
			LightEnvironment LightEnvironment;
		};

		static SceneData* sSceneData;
		static WindowAPI sWindowAPI;
	};
}
