#pragma once

#include "Engine.h"
#include "Sapfire/renderer/ShaderLibrary.h"

namespace Sapfire
{
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
		Ref<VertexArray> mVA;
		ShaderLibrary mShaderLibrary;
		Ref<Mesh> mSphereMesh;
		Ref<Shader> mSpriteShader;
		Ref<Shader> mMeshShader;
		Ref<Texture> mTexture;
		Ref<Framebuffer> mFramebuffer;
		PerspectiveCamera mCamera;
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