#include "Sandbox.h"

SandboxLayer::SandboxLayer() : mCamera(-5.f, 5.f, -5.f, 5.f)
{
	mVA.reset(VertexArray::Create());
	float vertices[7 * 3] = {-0.5f, -0.5f, 0.f,  1.f, 0.f, 1.f,
				 1.f, // first
				 0.5f,	-0.5f, 0.0f, 1.f, 0.f, 1.f,
				 1.f, // second
				 0.0f,	0.5f,  0.0f, 1.f, 0.f, 1.f, 1.f};
	uint32_t indices[] = {0, 1, 2};
	BufferLayout layout = {{"a_Position", ShaderDataType::Vec3}, {"a_Color", ShaderDataType::Vec4}};
	Ref<VertexBuffer> vb;
	vb.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
	vb->SetLayout(layout);
	mVA->AddVertexBuffer(vb);
	Ref<IndexBuffer> ib;
	ib.reset(IndexBuffer::Create(indices, 3));
	mVA->AddIndexBuffer(ib);
	mShader.reset(Shader::Create());
	mShader->Load("../Shaders/Triangle.vert", "../Shaders/Triangle.frag");
	mCamera.SetRotation(45.f);
}

static Vector4 clearColor(0.1f, 0.1f, 0.1f, 1);
void SandboxLayer::OnUpdate()
{
	RenderCommands::SetClearColor(clearColor);
	RenderCommands::ClearScreen();
	Renderer::BeginScene(mCamera);
	Renderer::Submit(mVA, mShader);
	Renderer::EndScene();
}

SandboxApplication::SandboxApplication()
{
	PushLayer(new SandboxLayer());
}

SandboxApplication::~SandboxApplication()
{
}

Application *CreateApplication()
{
	return new SandboxApplication();
}
