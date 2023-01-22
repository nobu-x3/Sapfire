#include "engine/Test.h"
#include "GL/glew.h"
#include "engine/Log.h"
#include "engine/events/WindowEvent.h"
#include "engine/renderer/Buffer.h"
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
	// Specify the vertex attributes
	// (For now, assume one vertex format)
	// Position is 3 floats starting at offset 0
	/* glEnableVertexAttribArray(0); */
	/* glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0); */
	/* glEnableVertexAttribArray(1); */
	/* glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, reinterpret_cast<void *>(sizeof(float) *
	 * 3)); */
	/* glEnableVertexAttribArray(2); */
	/* glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, reinterpret_cast<void *>(sizeof(float) *
	 * 6)); */
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

void TestApp::Tick()
{
	while (mRunning)
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		mShader->Bind();
		mVA->Bind();
		glDrawElements(GL_TRIANGLES, mVA->GetIndexBuffers()[0]->GetCount(), GL_UNSIGNED_INT, nullptr);
		mWindow->OnUpdate();
	}
}

bool TestApp::OnWindowClose(WindowCloseEvent &e)
{
	mRunning = false;
	return true;
}
