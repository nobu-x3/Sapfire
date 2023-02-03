#include "Sandbox.h"
#include "GL/glew.h"

const std::string SHADER_PATH = "../Shaders/Sprite.glsl";
const std::string SHADER_NAME = "Sprite";

SandboxLayer::SandboxLayer() : mCamera(-1.6f, 1.6f, -0.9f, 0.9f)
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
	vb = VertexBuffer::Create();
	vb->SetLayout(layout);
	vb->SetData(vertices, sizeof(vertices));
	mVA->AddVertexBuffer(vb);
	Ref<IndexBuffer> ib;
	ib = IndexBuffer::Create();
	ib->SetData(indices, sizeof(indices));
	mVA->AddIndexBuffer(ib);
	mSpriteShader = mShaderLibrary.Load(SHADER_PATH);
	mTexture = Texture::Create("../Assets/Asteroid.png");
	mSpriteShader->SetIntUniform("uTexture", mTexture->GetID());
	mCamera.SetRotation(0);
}

static glm::vec4 clearColor(0.1f, 0.1f, 0.1f, 1);
void SandboxLayer::OnUpdate(float deltaTime)
{
	mCameraRotation += 30.f * deltaTime;
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
