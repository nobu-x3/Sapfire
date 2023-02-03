#pragma once

#include "engine/Engine.h"
#include "engine/renderer/ShaderLibrary.h"

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
	OrthographicCamera mCamera;

	float mCameraRotation;
};

class SandboxApplication : public Application
{
	public:
	SandboxApplication();
	~SandboxApplication();
};
