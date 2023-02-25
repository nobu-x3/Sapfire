#pragma once

#include "Engine.h"
#include "Sapfire/renderer/ShaderLibrary.h"
#include "Sapfire/scene/Entity.h"

namespace Sapfire
{
	class Scene;

	class SaplingLayer : public Layer
	{
	public:
		SaplingLayer();
		virtual void on_attach() override;
		virtual void on_update(float deltaTime) override;
		virtual void on_imgui_render() override;
		virtual void on_event(Event& event) override;
		bool OnMouseMoved(MouseMovedEvent& e);

	private:
		ShaderLibrary mShaderLibrary;
		Ref<Shader> mSpriteShader;
		Ref<Shader> mMeshShader;
		Ref<Skybox> mSkybox;
		Ref<Texture> mTexture;
		Ref<Framebuffer> mFramebuffer;
		std::vector<Ref<Mesh>> mMeshes;
		Ref<Scene> mActiveScene;
		Entity mCamera;
		glm::vec2 mViewportSize;
		glm::vec3 mDirection;
		float mCameraRotation;
		bool mViewportPanelFocused;
		bool mViewportPanelHovered;
	};

	class SaplingApp : public Application
	{
	public:
		SaplingApp();
		~SaplingApp();
	};
}
