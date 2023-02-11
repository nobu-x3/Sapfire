#include "Sapling.h"
#include "Sapfire/renderer/Mesh.h"
#include <imgui.h>
#include <glfw/include/GLFW/glfw3.h>
#include "Sapfire/core/Input.h"
#include "Sapfire/imgui/ImguiLayer.h"

const std::string SHADER_PATH = "../Sandbox/Shaders/Sprite.glsl";
const std::string SHADER_NAME = "../Sandbox/Sprite";

namespace Sapfire
{
	SaplingLayer::SaplingLayer()
		: /* mCamera(1.6f, -1.6f, 0.9f, -0.9) */
		mCamera(70.f, 1280, 720, 0.01, 100), mDirection(glm::vec3(0)), mViewportSize(0)
	{
	}

	void SaplingLayer::OnAttach()
	{
		mVA.reset(VertexArray::Create());
		float vertices[7 * 4] = {
			-0.5f, 0.5f,  0.f, 0.f, 1.f, // top left
			0.5f,  0.5f,  0.f, 1.f, 1.f, // top right
			0.5f,  -0.5f, 0.f, 1.f, 0.f, // bottom right
			-0.5f, -0.5f, 0.f, 0.f, 0.f	 // bottom left
		};
		uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };
		BufferLayout layout = { {"inPosition", ShaderDataType::Vec3}, {"inTexCoord", ShaderDataType::Vec2} };
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
		mTexture = Texture::Create("../Sandbox/Assets/Asteroid.png");
		mSpriteShader->SetIntUniform("uTexture", mTexture->GetID());
		mCamera.SetPosition(glm::vec3(0.f));
		mMeshShader = mShaderLibrary.Load("../Sandbox/Shaders/BasicMesh.glsl");
		mSphereMesh = CreateRef<Mesh>("../Sandbox/Assets/Sphere.blend1");
		mSphereMesh->SetTexture("../Sandbox/Assets/Farback01.png");
		mSphereMesh->SetPosition(glm::vec3({ 0.f, 0.f, 0.4f }));
		mSphereMesh->SetScale(glm::vec3(1.f));
		mCameraRotation = 0.f;
		RenderCommands::Init();
		mViewportSize = { 1280, 720 };
		FramebufferProperties fbProps = { 1280, 720, FramebufferFormat::RGBA8 };
		mFramebuffer = Framebuffer::Create(fbProps);
	}

	static glm::vec4 clearColor(0.1f, 0.1f, 0.1f, 1);
	static glm::vec3 scale(1.f);

	const float MOVE_SPEED = 0.1f;

	void SaplingLayer::OnUpdate(float deltaTime)
	{
		if(mViewportPanelFocused)
		{
			if (Input::KeyPressed(KeyCode::A))
				mDirection += glm::vec3({ -1, 0, 0 });
			if (Input::KeyPressed(KeyCode::D))
				mDirection += glm::vec3({ 1, 0, 0 });
			if (Input::KeyPressed(KeyCode::W))
				mDirection += glm::vec3({ 0, 0, -1 });
			if (Input::KeyPressed(KeyCode::S))
				mDirection += glm::vec3({ 0, 0, 1 });
		}
		//mCameraRotation += 30.f * deltaTime;
		auto pos = mCamera.GetPosition();
		mCamera.SetPosition(pos + mDirection * MOVE_SPEED * deltaTime);
		mSphereMesh->SetRotation(glm::angleAxis(glm::radians(mCameraRotation), glm::vec3({ 0.f, 0.f, 1.f })));
		RenderCommands::SetClearColor(clearColor);
		mFramebuffer->Bind();
		RenderCommands::ClearScreen();
		Renderer::BeginScene(mCamera);
		/* mTexture->Bind(); */
		//Renderer::Submit(mVA, mSpriteShader);
		Renderer::SubmitMesh(mSphereMesh, mMeshShader);
		Renderer::EndScene();
		mFramebuffer->Unbind();
		mDirection = glm::vec3(0);
	}

	void SaplingLayer::OnImguiRender()
	{
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->GetWorkPos());
		ImGui::SetNextWindowSize(viewport->GetWorkSize());
		ImGui::SetNextWindowViewport(viewport->ID);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;
		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		static bool p_open = true;
		ImGui::Begin("DockSpace Demo", &p_open, window_flags);
		{
			// DockSpace
			ImGuiIO& io = ImGui::GetIO();
			RendererID textureID = mFramebuffer->GetColorAttachmentRendererID();
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2((float)Application::GetInstance().GetWindow().GetWidth(), (float)Application::GetInstance().GetWindow().GetHeight()), dockspace_flags);
			ImGui::Begin("Scene View");
			{
				mViewportPanelFocused = ImGui::IsWindowFocused(); 
				mViewportPanelHovered = ImGui::IsWindowHovered();
				Application::GetInstance().GetImguiLayer()->SetBlockEvents(mViewportPanelFocused && mViewportPanelHovered);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
				auto sceneViewportSize = ImGui::GetContentRegionAvail();
				if (mViewportSize.x != sceneViewportSize.x || mViewportSize.y != sceneViewportSize.y)
				{
					mFramebuffer->Resize(sceneViewportSize.x, sceneViewportSize.y);
					mViewportSize = { sceneViewportSize.x, sceneViewportSize.y };
				}
				ImGui::Image((void*)textureID, { mViewportSize.x, mViewportSize.y }, { 0, 1 }, { 1, 0 });
				ImGui::PopStyleVar();
			}
			ImGui::End();

			ImGui::Begin("Some other panel");
			{
				ImGui::Text("Some other text");
			}
			ImGui::End();
		}
		ImGui::End();
	}

	void SaplingLayer::OnEvent(Event& event)
	{
		if (event.Handled) return;
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(SaplingLayer::OnMouseMoved));
	}

	static float prevVal = 0.f;

	bool SaplingLayer::OnMouseMoved(MouseMovedEvent& e)
	{
		if (!mViewportPanelFocused) return true;
		mCameraRotation -= e.GetX() - prevVal;
		prevVal = e.GetX();
		return true;
	}

	SaplingApp::SaplingApp() : Application("Sapling")
	{
		PushLayer(new SaplingLayer());
	}

	SaplingApp::~SaplingApp()
	{
	}

	Sapfire::Application* Sapfire::CreateApplication()
	{
		return new SaplingApp();
	}
}