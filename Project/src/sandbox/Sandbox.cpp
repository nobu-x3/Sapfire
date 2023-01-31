#include "Sandbox.h"
#include "GL/glew.h"

const std::string SHADER_PATH = "../Shaders/Sprite.glsl";
const std::string SHADER_NAME = "Sprite";

SandboxLayer::SandboxLayer() : mCamera(-1.6f, 1.6f, -1.2f, 1.2f)
{
	mVA.reset(VertexArray::Create());
	float vertices[7 * 4] = {
	    -0.5f, 0.5f,  0.f, 0.f, 1.f, // top left
	    0.5f,  0.5f,  0.f, 1.f, 1.f, // top right
	    0.5f,  -0.5f, 0.f, 1.f, 0.f, // bottom right
	    -0.5f, -0.5f, 0.f, 0.f, 0.f	 // bottom left
	};

	uint32_t indices[] = {0, 1, 2, 2, 3, 0};
	BufferLayout layout = {{"inPosition", ShaderDataType::Vec3}, {"inTexCoord", ShaderDataType::Vec2}};
	Ref<VertexBuffer> vb;
	vb.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
	vb->SetLayout(layout);
	mVA->AddVertexBuffer(vb);
	Ref<IndexBuffer> ib;
	ib.reset(IndexBuffer::Create(indices, 6));
	mVA->AddIndexBuffer(ib);
	mSpriteShader = mShaderLibrary.Load(SHADER_PATH);
	mTexture = Texture::Create("../Assets/Asteroid.png");
	mSpriteShader->SetIntUniform("uTexture", mTexture->GetID());
}

static Vector4 clearColor(0.1f, 0.1f, 0.1f, 1);
void SandboxLayer::OnUpdate(float deltaTime)
{

	mCameraRotation += Math::Sin(Math::ToRadians(30.f)) * deltaTime;
	mCamera.SetRotation(mCameraRotation);

	RenderCommands::Init();
	RenderCommands::SetClearColor(clearColor);
	RenderCommands::ClearScreen();
	Renderer::BeginScene(mCamera);
	mTexture->Bind();
	Renderer::Submit(mVA, mSpriteShader);
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
