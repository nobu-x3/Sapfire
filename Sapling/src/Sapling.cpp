#include "Sapling.h"
#include "Sapfire/renderer/Mesh.h"
#include <imgui.h>
#include <glfw/include/GLFW/glfw3.h>

#include "glm/gtx/matrix_interpolation.hpp"
#include "Sapfire/imgui/ImguiLayer.h"
#include "Sapfire/scene/Components.h"
#include "Sapfire/tools/Profiling.h"
#include "Sapfire/scene/Scene.h"
#include "Sapfire/scene/Entity.h"
#include "Sapfire/scripts/CameraController.h"

const std::string SHADER_PATH = "../Sandbox/Shaders/Sprite.glsl";
const std::string SHADER_NAME = "../Sandbox/Sprite";

namespace Sapfire
{
	SaplingLayer::SaplingLayer() : 
		 mViewportSize(0.f)
	{
	}

	void SaplingLayer::on_attach()
	{
		PROFILE_FUNCTION();
		mMeshShader = mShaderLibrary.load("../Sandbox/Shaders/BasicMesh.glsl");
		mActiveScene = create_ref<Scene>();
		mSceneHierarchy.set_context(mActiveScene);
		mCamera = mActiveScene->create_entity();
		mCamera.add_component<CameraComponent>(70.f, 1280.f, 720.f, 0.01f, 10000.f);
		mCamera.add_component<ScriptComponent>().bind<CameraController>();
		mMeshes.reserve(10);
		for (int i = 0; i < 10; ++i)
		{
			auto cube = mActiveScene->create_entity(std::format("cube{0}", i));
			auto &mesh = cube.add_component<MeshRendererComponent>("../Sandbox/Assets/Cube.fbx", mMeshShader);
			mesh.Mesh3D.set_texture("../Sandbox/Assets/Farback01.png");
			cube.transform().Translation = {100.f * i, 0.f, 400.f};
			cube.transform().Scale = {0.3f, 0.3f, 0.3f};
		}
		mSkybox = create_ref<Skybox>("../Sandbox/Shaders/Skybox.glsl",
		                             std::array<std::string, 6>{
			                             "../Sandbox/Assets/skybox/right.jpg",
			                             "../Sandbox/Assets/skybox/left.jpg",
			                             "../Sandbox/Assets/skybox/top.jpg",
			                             "../Sandbox/Assets/skybox/bottom.jpg",
			                             "../Sandbox/Assets/skybox/front.jpg",
			                             "../Sandbox/Assets/skybox/back.jpg"
		                             });
		RenderCommands::init();
		FramebufferProperties fbProps = {1280, 720, FramebufferFormat::RGBA8};
		mFramebuffer = Framebuffer::create(fbProps);
	}

	static glm::vec4 clearColor(0.1f, 0.1f, 0.1f, 1);
	static glm::vec3 scale(1.f);


	void SaplingLayer::on_update(float deltaTime)
	{
		PROFILE_FUNCTION();
		{
			PROFILE_SCOPE("Rendering");
			mFramebuffer->bind();
			RenderCommands::set_clear_color(clearColor);
			RenderCommands::clear_screen();
			mActiveScene->on_update(deltaTime);
			mSkybox->draw();
			Renderer::end_scene();
			mFramebuffer->unbind();
		}
	}

	void SaplingLayer::on_imgui_render()
	{
		PROFILE_FUNCTION();
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		ImGuiViewport *viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->GetWorkPos());
		ImGui::SetNextWindowSize(viewport->GetWorkSize());
		ImGui::SetNextWindowViewport(viewport->ID);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove;
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
			ImGuiIO &io = ImGui::GetIO();
			RendererID textureID = mFramebuffer->get_color_attachment_renderer_id();
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id,
			                 ImVec2(static_cast<float>(Application::get_instance().get_window().get_width()),
			                        static_cast<float>(Application::get_instance().get_window().get_height())),
			                 dockspace_flags);
			ImGui::Begin("Scene View");
			{
				mViewportPanelFocused = ImGui::IsWindowFocused();
				mViewportPanelHovered = ImGui::IsWindowHovered();
				Application::get_instance().get_imgui_layer().SetBlockEvents(
					mViewportPanelFocused && mViewportPanelHovered);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
				auto sceneViewportSize = ImGui::GetContentRegionAvail();
				if (mViewportSize.x != sceneViewportSize.x || mViewportSize.y != sceneViewportSize.y)
				{
					mFramebuffer->resize(static_cast<uint32_t>(sceneViewportSize.x),
					                     static_cast<uint32_t>(sceneViewportSize.y));
					mViewportSize = {sceneViewportSize.x, sceneViewportSize.y};
					mActiveScene->set_viewport_size((uint32_t)sceneViewportSize.x, (uint32_t)sceneViewportSize.y);
				}
				ImGui::Image((void *)textureID, {mViewportSize.x, mViewportSize.y}, {0, 1}, {1, 0});
				ImGui::PopStyleVar();
			}
			ImGui::End();
			mSceneHierarchy.OnImguiRender();
			ImGui::Begin("Some other panel");
			{
				ImGui::Text("Some other text");
				ImGui::DragFloat3("Camera Transform",
				                  glm::value_ptr(mCamera.get_component<TransformComponent>().Translation));
			}
			ImGui::End();
		}
		ImGui::End();
	}

	void SaplingLayer::on_event(Event &event)
	{
		if (event.Handled)
			return;
		EventDispatcher dispatcher(event);
		dispatcher.dispatch<MouseMovedEvent>(BIND_EVENT_FN(SaplingLayer::OnMouseMoved));
	}


	bool SaplingLayer::OnMouseMoved(MouseMovedEvent &e)
	{
		if (!mViewportPanelFocused)
			return true;
		return true;
	}

	SaplingApp::SaplingApp() :
		Application("Sapling")
	{
		push_layer(new SaplingLayer());
	}

	SaplingApp::~SaplingApp()
	{
	}

	Application *Sapfire::create_application()
	{
		return new SaplingApp();
	}
}
