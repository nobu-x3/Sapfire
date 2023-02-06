#pragma once

#include "Engine.h"
#include "Sapfire/renderer/ShaderLibrary.h"

class SandboxLayer : public Layer
{
	public:
	SandboxLayer();
	virtual void OnUpdate(float deltaTime) override;
	virtual void OnImguiRender() override;
	virtual void OnEvent(Event& event) override;
	bool OnKeyPressed(KeyPressedEvent& e);
	bool OnMouseMoved(MouseMovedEvent& e);
	private:
	Ref<VertexArray> mVA;
	ShaderLibrary mShaderLibrary;
	Ref<class Mesh> mSphereMesh;
	Ref<Shader> mSpriteShader;
	Ref<Shader> mMeshShader;
	Ref<Texture> mTexture;
	PerspectiveCamera mCamera;
	glm::vec3 mDirection;
	float mCameraRotation;
};

class SandboxApplication : public Application
{
	public:
	SandboxApplication();
	~SandboxApplication();
};
