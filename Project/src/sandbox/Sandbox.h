#pragma once

#include "engine/Engine.h"

class SandboxLayer : public Layer
{
	public:
	SandboxLayer();
	virtual void OnUpdate(float deltaTime) override;

	private:
	Ref<VertexArray> mVA;
	Ref<Shader> mShader;
	Ref<Texture> mTexture;
	OrthographicCamera mCamera;

	float mCameraRotation;
};

class SandboxApplication : public Application
{
	public:
	SandboxApplication();
	~SandboxApplication();
};