#include "engine/Test.h"
#include "GL/glew.h"
#include "engine/Log.h"
#include "engine/events/WindowEvent.h"
#include "engine/renderer/Buffer.h"
#include "engine/renderer/RenderCommands.h"
#include "engine/renderer/RendererNew.h"
#include "engine/renderer/Shader.h"
#include "engine/renderer/VertexArray.h"
#include "engine/renderer/Window.h"

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

TestApp::TestApp()
{
	mWindow = std::unique_ptr<Window>(Window::Create());
	mWindow->SetEventCallback(BIND_EVENT_FN(TestApp::OnEvent));
	mLayerStack.PushLayer(new Layer());
	Log::Init();
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
}

TestApp::~TestApp()
{
}

void TestApp::OnEvent(Event &event)
{
	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(TestApp::OnWindowClose));

	for (auto it = mLayerStack.end(); it != mLayerStack.begin();)
	{
		(*--it)->OnEvent(event);
		if (event.Handled)
			break;
	}
}

static Vector4 clearColor(0.1f, 0.1f, 0.1f, 1);
void TestApp::Tick()
{
	while (mRunning)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		RenderCommands::SetClearColor(clearColor);
		RenderCommands::ClearScreen();
		Renderer::BeginScene();
		mShader->Bind();
		Renderer::Submit(mVA);
		Renderer::EndScene();
		/* Renderer::Flush(); */
		mVA->Bind();
		for (auto ib : mVA->GetIndexBuffers())
		{
			glDrawElements(GL_TRIANGLES, ib->GetCount(), GL_UNSIGNED_INT, nullptr);
		}
		mWindow->OnUpdate();
	}
}

bool TestApp::OnWindowClose(WindowCloseEvent &e)
{
	mRunning = false;
	return true;
}
