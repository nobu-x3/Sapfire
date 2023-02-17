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
	mVA = VertexArray::create();
	float vertices[7 * 4] = {
		-0.5f, 0.5f, 0.f, 0.f, 1.f, // top left
		0.5f, 0.5f, 0.f, 1.f, 1.f, // top right
		0.5f, -0.5f, 0.f, 1.f, 0.f, // bottom right
		-0.5f, -0.5f, 0.f, 0.f, 0.f // bottom left
	};
	uint32_t indices[] = {0, 1, 2, 2, 3, 0};
	BufferLayout layout = {{"inPosition", ShaderDataType::Vec3}, {"inTexCoord", ShaderDataType::Vec2}};
	Ref<VertexBuffer> vb;
	vb = VertexBuffer::create();
	vb->set_layout(layout);
	vb->set_data(vertices, sizeof vertices);
	mVA->add_vertex_buffer(vb);
	Ref<IndexBuffer> ib;
	ib = IndexBuffer::create();
	ib->set_data(indices, sizeof indices);
	// mVA->add_index_buffer(ib);
	mSpriteShader = mShaderLibrary.load(SHADER_PATH);
	mTexture = Texture::create("Assets/Asteroid.png");
	mSpriteShader->set_int_uniform("uTexture", mTexture->get_id());
	mCamera.set_position(glm::vec3(0.f));
	mMeshShader = mShaderLibrary.load("Shaders/BasicMesh.glsl");
	mSphereMesh = create_ref<Mesh>("Assets/Cube.fbx");
	mSphereMesh->set_texture("Assets/Farback01.png");
	mSphereMesh->set_position(glm::vec3({0.f, 0.f, 0.4f}));
	mSphereMesh->set_scale(glm::vec3(0.3f));
	mCameraRotation = 0.f;
	BufferLayout matrixUniBufLayout = {{"viewproj", ShaderDataType::Mat4}};
	mUniformBuffer = UniformBuffer::create(0, matrixUniBufLayout);
	mSkybox = create_ref<Skybox>("Assets/Cube.fbx", "Shaders/Skybox.glsl", std::array<std::string, 6> {
		"Assets/cubemap/px.png",
		"Assets/cubemap/nx.png",
		"Assets/cubemap/py.png",
		"Assets/cubemap/ny.png",
		"Assets/cubemap/pz.png",
		"Assets/cubemap/nz.png"
	});
	RenderCommands::init();
}

static glm::vec4 clearColor(0.1f, 0.1f, 0.1f, 1);
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
	mSphereMesh->set_rotation(angleAxis(glm::radians(mCameraRotation), glm::vec3({0.f, 0.f, 1.f})));
	RenderCommands::set_clear_color(clearColor);
	RenderCommands::clear_screen();
	Renderer::begin_scene(mCamera, mUniformBuffer);
	/* mTexture->Bind(); */
	//Renderer::Submit(mVA, mSpriteShader);
	mSkybox->draw(mCamera);
	Renderer::submit_mesh(mSphereMesh, mMeshShader);
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
