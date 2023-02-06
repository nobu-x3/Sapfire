#pragma once

#include "Engine.h"
#include "Sapfire/renderer/ShaderLibrary.h"

class SandboxLayer : public Layer
{
	public:
	SandboxLayer();
	virtual void OnUpdate(float deltaTime) override;

	private:
	Ref<VertexArray> mVA;
	ShaderLibrary mShaderLibrary;
	Ref<class Mesh> mSphereMesh;
	Ref<Shader> mSpriteShader;
	Ref<Shader> mMeshShader;
	Ref<Texture> mTexture;
	PerspectiveCamera mCamera;

	float mCameraRotation;
};

class SandboxApplication : public Application
{
	public:
	SandboxApplication();
	~SandboxApplication();
};
