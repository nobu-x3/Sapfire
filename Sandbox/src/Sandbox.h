#pragma once

#include "Engine.h"
#include "Sapfire/renderer/ShaderLibrary.h"

using namespace Sapfire;

class SandboxLayer : public Layer
{
public:
	SandboxLayer();
	virtual void on_attach() override;
	virtual void on_update(float deltaTime) override;
	virtual void on_imgui_render() override;
	virtual void on_event(Event& event) override;
	bool OnMouseMoved(MouseMovedEvent& e);

private:
	Ref<UniformBuffer> mUniformBuffer;
	ShaderLibrary mShaderLibrary;
	std::vector<Ref<Mesh>> mMeshes;
	Ref<Skybox> mSkybox;
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
