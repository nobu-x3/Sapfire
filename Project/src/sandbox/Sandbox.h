#pragma once

#include "engine/Engine.h"

class SandboxLayer : public Layer
{
	public:
	SandboxLayer();
	virtual void OnUpdate() override;

	private:
	Ref<VertexArray> mVA;
	Ref<Shader> mShader;
	OrthographicCamera mCamera;
};

class SandboxApplication : public Application
{
	public:
	SandboxApplication();
	~SandboxApplication();
};
