#include "Sandbox.h"
#include "Sapfire/renderer/Mesh.h"
#include <glfw/include/GLFW/glfw3.h>
#include "Sapfire/core/Input.h"

const std::string SHADER_PATH = "Shaders/Sprite.glsl";
const std::string SHADER_NAME = "Sprite";

SandboxLayer::SandboxLayer() : /* mCamera(1.6f, -1.6f, 0.9f, -0.9) */
	mCamera(70.f, 1280.f, 720.f, 0.01f, 10000.f), mDirection(glm::vec3(0.f))
{
}

void SandboxLayer::on_attach()
{
	mCamera.set_position(glm::vec3(0.f));
	mMeshShader = mShaderLibrary.load("Shaders/BasicMesh.glsl");
	mMeshes.reserve(10);
	for (int i = 0; i < 10; ++i)
	{
		mMeshes.emplace_back(create_ref<Mesh>("Assets/Cube.fbx", mMeshShader));
		mMeshes[i]->set_texture("Assets/Farback01.png");
		mMeshes[i]->set_position(glm::vec3({50.f * i, 0.f, 100.f}));
		mMeshes[i]->set_scale(glm::vec3(0.3f));
	}
	mCameraRotation = 0.f;
	BufferLayout matrixUniBufLayout = {{"view", ShaderDataType::Mat4}, {"proj", ShaderDataType::Mat4}};
	mUniformBuffer = UniformBuffer::create(0, matrixUniBufLayout);
	mSkybox = create_ref<Skybox>("Shaders/Skybox.glsl", std::array<std::string, 6>{
		                             "Assets/skybox/right.jpg",
		                             "Assets/skybox/left.jpg",
		                             "Assets/skybox/top.jpg",
		                             "Assets/skybox/bottom.jpg",
		                             "Assets/skybox/front.jpg",
		                             "Assets/skybox/back.jpg"
	                             });
	RenderCommands::init();
}

static glm::vec4 clearColor(1.f, 1.f, 1.f, 1);
static glm::vec3 scale(1.f);

const float MOVE_SPEED = 50.f;

void SandboxLayer::on_update(float deltaTime)
{
	//mCameraRotation += 30.f * deltaTime;
	if (Input::key_pressed(KeyCode::A))
		mDirection += glm::vec3({-1, 0, 0});
	if (Input::key_pressed(KeyCode::D))
		mDirection += glm::vec3({1, 0, 0});
	if (Input::key_pressed(KeyCode::W))
		mDirection += glm::vec3({0, 0, -1});
	if (Input::key_pressed(KeyCode::S))
		mDirection += glm::vec3({0, 0, 1});
	auto pos = mCamera.get_position();
	mCamera.set_position(pos + mDirection * MOVE_SPEED * deltaTime);
	for (auto& mesh : mMeshes)
	{
		mesh->set_rotation(angleAxis(glm::radians(mCameraRotation), glm::vec3({0.f, 0.f, 1.f})));
	}
	RenderCommands::set_clear_color(clearColor);
	RenderCommands::clear_screen();
	Renderer::begin_scene(mCamera, mUniformBuffer);
	for (auto& mesh : mMeshes)
	{
		Renderer::submit_mesh(mesh, mMeshShader);
	}
	mSkybox->draw();
	Renderer::end_scene();
	mDirection = glm::vec3(0);
}

void SandboxLayer::on_imgui_render()
{
}

void SandboxLayer::on_event(Event& event)
{
	EventDispatcher dispatcher(event);
	dispatcher.dispatch<MouseMovedEvent>(BIND_EVENT_FN(SandboxLayer::OnMouseMoved));
}

static float prevVal = 0.f;

bool SandboxLayer::OnMouseMoved(MouseMovedEvent& e)
{
	mCameraRotation -= e.get_x() - prevVal;
	prevVal = e.get_x();
	return true;
}

SandboxApplication::SandboxApplication()
{
	push_layer(new SandboxLayer());
}

SandboxApplication::~SandboxApplication()
{
}

Application* Sapfire::create_application()
{
	return new SandboxApplication();
}
