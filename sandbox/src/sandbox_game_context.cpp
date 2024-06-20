#include "sandbox_game_context.h"
#include "components/movement_component.h"
#include "components/render_component.h"
#include "components/test_custom_component.h"
#include "core/game_context.h"

using namespace Sapfire;

static d3d::primitives::MeshData cube = d3d::primitives::create_box(1, 1, 1, 0);

struct ObjectConstants {
	DirectX::XMFLOAT4X4 World = Sapfire::math::Identity4x4();
};

struct PassConstants {
	DirectX::XMFLOAT4X4 view = Sapfire::math::Identity4x4();
	DirectX::XMFLOAT4X4 inv_view = Sapfire::math::Identity4x4();
	DirectX::XMFLOAT4X4 proj = Sapfire::math::Identity4x4();
	DirectX::XMFLOAT4X4 inv_proj = Sapfire::math::Identity4x4();
	DirectX::XMFLOAT4X4 view_proj = Sapfire::math::Identity4x4();
	DirectX::XMFLOAT4X4 inv_view_proj = Sapfire::math::Identity4x4();
	DirectX::XMFLOAT3 EyePosW = {0.0f, 0.0f, 0.0f};
	float cbPerObjectPad1 = 0.0f;
	DirectX::XMFLOAT2 render_target_size = {0.0f, 0.0f};
	DirectX::XMFLOAT2 inv_render_target_size = {0.0f, 0.0f};
	float near_z = 0.0f;
	float far_z = 0.0f;
	float total_time = 0.0f;
	float delta_time = 0.0f;
	DirectX::XMFLOAT4 ambient_light = {0.0f, 0.0f, 0.0f, 1.0f};
	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
	// are spot lights for a maximum of MaxLights per object.
	Sapfire::d3d::Light Lights[MaxLights];
};

SandboxGameContext::SandboxGameContext(const Sapfire::GameContextCreationDesc& desc) : Sapfire::GameContext(desc) {
	m_MainCamera = {CAMERA_FOV, static_cast<f32>(m_ClientExtent->width) / m_ClientExtent->height, 0.1f, 1000.f};
}

void SandboxGameContext::load_contents() {
	auto entity1 = m_ECManager.create_entity();
	m_ECManager.add_component(entity1, stl::make_shared<components::TestCustomComponent>(mem::ENUM::Game_Components, "HELLO"));
	m_ECManager.add_engine_component<components::MovementComponent>(entity1);
	components::Transform t1{};
	auto pos = DirectX::XMFLOAT4(0.f, 0.f, 5.f, 0.f);
	t1.position(DirectX::XMLoadFloat4(&pos));
	m_ECManager.add_engine_component<components::Transform>(entity1, t1);
	auto entity2 = m_ECManager.create_entity();
	components::Transform t2{};
	pos = DirectX::XMFLOAT4(5.0f, 0.f, 5.f, 0.f);
	t2.position(DirectX::XMLoadFloat4(&pos));
	m_ECManager.add_engine_component<components::Transform>(entity2, t2);
	m_PipelineState = m_GraphicsDevice->create_pipeline_state({
		.shader_module =
			{
				.vertexShaderPath = L"bindless.hlsl",
				.vertexEntryPoint = L"VS",
				.pixelShaderPath = L"bindless.hlsl",
				.pixelEntryPoint = L"PS",
			},
		.pipeline_name = L"Bindless Pipeline",
	});
	m_RTIndexBuffers.push_back(m_GraphicsDevice->create_buffer<u16>(
		d3d::BufferCreationDesc{
			.usage = d3d::BufferUsage::IndexBuffer,
			.name = L"Render Target Index Buffer",
		},
		cube.indices16()));
	m_VertexPosBuffers.push_back(m_GraphicsDevice->create_buffer<DirectX::XMFLOAT3>(
		d3d::BufferCreationDesc{
			.usage = d3d::BufferUsage::StructuredBuffer,
			.name = L"Vertex Position Buffer",
		},
		cube.positions));
	m_VertexNormalBuffers.push_back(m_GraphicsDevice->create_buffer<DirectX::XMFLOAT3>(
		d3d::BufferCreationDesc{
			.usage = d3d::BufferUsage::StructuredBuffer,
			.name = L"Vertex Normal Buffer",
		},
		cube.normals));
	m_VertexTangentBuffers.push_back(m_GraphicsDevice->create_buffer<DirectX::XMFLOAT3>(
		d3d::BufferCreationDesc{
			.usage = d3d::BufferUsage::StructuredBuffer,
			.name = L"Vertex Tangent Buffer",
		},
		cube.tangentus));
	m_VertexUVBuffers.push_back(m_GraphicsDevice->create_buffer<DirectX::XMFLOAT2>(
		d3d::BufferCreationDesc{
			.usage = d3d::BufferUsage::StructuredBuffer,
			.name = L"Vertex UV Buffer",
		},
		cube.texcs));
	m_MainPassCB = m_GraphicsDevice->create_buffer<PassConstants>(d3d::BufferCreationDesc{
		.usage = d3d::BufferUsage::ConstantBuffer,
		.name = L"Main Pass Constant Buffer",
	});
	auto& transforms = m_ECManager.engine_components<components::Transform>();
	m_TransformBuffers.reserve(transforms.size());
	auto index = 0;
	for (auto& _ : transforms) {
		m_TransformBuffers.emplace_back(m_GraphicsDevice->create_buffer<ObjectConstants>({
			.usage = d3d::BufferUsage::ConstantBuffer,
			.name = L"Transform buffer " + std::to_wstring(index),
		}));
		index++;
	}
	// textures:
	m_DepthTexture = m_GraphicsDevice->create_texture({
		.usage = d3d::TextureUsage::DepthStencil,
		.width = static_cast<u32>(m_ClientExtent->width),
		.height = static_cast<u32>(m_ClientExtent->height),
		.format = DXGI_FORMAT_D32_FLOAT,
		.name = L"Depth Texture",
	});
    m_AssetManager.load_runtime_texture("assets/textures/ceramics.jpg");
	// Materials
	d3d::Material grass{};
	grass.name = "grass";
	grass.material_cb_index = 0;
	grass.diffuse_albedo = DirectX::XMFLOAT4{0.2f, 0.6f, 0.6f, 1.0f};
	grass.roughness = 0.125f;
	grass.material_buffer = m_GraphicsDevice->create_buffer<d3d::MaterialConstants>({
		.usage = d3d::BufferUsage::ConstantBuffer,
		.name = L"Material Buffer - Grass",
	});
	m_Materials.push_back(grass);
	components::RenderComponent cube_rc{
		{}, m_AssetManager.get_texture("assets/textures/ceramics.jpg")->uuid,
		components::CPUData{
			.indices_size = static_cast<u32>(cube.indices32.size()),
			.index_id = 0,
			.position_idx = 0,
			.normal_idx = 0,
			.tangent_idx = 0,
			.uv_idx = 0,
		},
		components::PerDrawConstants{
			.position_buffer_idx = m_VertexPosBuffers[0].srv_index,
			.normal_buffer_idx = m_VertexNormalBuffers[0].srv_index,
			.tangent_buffer_idx = m_VertexTangentBuffers[0].srv_index,
			.uv_buffer_idx = m_VertexUVBuffers[0].srv_index,
			.scene_cbuffer_idx = m_TransformBuffers[0].cbv_index,
			.pass_cbuffer_idx = m_MainPassCB.cbv_index,
			.material_cbuffer_idx = m_Materials[0].material_buffer.cbv_index,
			.texture_cbuffer_idx = m_AssetManager.get_texture_resource("assets/textures/ceramics.jpg").gpu_idx,
		},
	};
	m_ECManager.add_engine_component<components::RenderComponent>(entity1, cube_rc);
	stl::string monkey_path = "assets/models/monkey.obj";
	create_render_component(entity2, {monkey_path, "assets/textures/ceramics.jpg"});
}

void SandboxGameContext::update(f32 delta_time) {
	GameContext::update(delta_time);
	PROFILE_SCOPE("SandboxGameContext::update");
	m_MainCamera.update(delta_time);
	// Wait for render to happen
	m_GraphicsDevice->direct_command_queue()->flush();
	// update buffers
	update_pass_cb(delta_time);
	update_materials(delta_time);
	udpate_transform_buffer(delta_time);
}

void SandboxGameContext::update_pass_cb(f32 delta_time) {
	auto main_pass = PassConstants{};
	auto view = m_MainCamera.view();
	auto proj = m_MainCamera.projection;
	auto viewProj = XMMatrixMultiply(view, proj);
	auto view_determinant = XMMatrixDeterminant(view);
	auto invView = XMMatrixInverse(&view_determinant, view);
	auto proj_determinant = XMMatrixDeterminant(proj);
	auto invProj = XMMatrixInverse(&proj_determinant, proj);
	auto view_proj_determinant = XMMatrixDeterminant(viewProj);
	auto invViewProj = XMMatrixInverse(&view_proj_determinant, viewProj);
	XMStoreFloat4x4(&main_pass.view, XMMatrixTranspose(view));
	XMStoreFloat4x4(&main_pass.inv_view, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&main_pass.proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&main_pass.inv_proj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&main_pass.view_proj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&main_pass.inv_view_proj, XMMatrixTranspose(invViewProj));
	XMStoreFloat3(&main_pass.EyePosW, m_MainCamera.transform.position());
	main_pass.render_target_size = DirectX::XMFLOAT2((float)m_ClientExtent->width, (float)m_ClientExtent->height);
	main_pass.inv_render_target_size = DirectX::XMFLOAT2(1.0f / m_ClientExtent->width, 1.0f / m_ClientExtent->height);
	main_pass.near_z = 1.0f;
	main_pass.far_z = 1000.0f;
	main_pass.total_time = delta_time;
	main_pass.delta_time = delta_time;
	main_pass.ambient_light = {0.25f, 0.25f, 0.35f, 1.0f};
	main_pass.Lights[0].direction = {0.57735f, -0.0f, 1.57735f};
	main_pass.Lights[0].strength = {0.6f, 0.6f, 0.6f};
	main_pass.Lights[1].direction = {-0.57735f, -0.57735f, 0.57735f};
	main_pass.Lights[1].strength = {0.3f, 0.3f, 0.3f};
	main_pass.Lights[2].direction = {0.0f, -0.707f, -0.707f};
	main_pass.Lights[2].strength = {0.15f, 0.15f, 0.15f};
	m_MainPassCB.update(&main_pass);
}

void SandboxGameContext::update_materials(f32 delta_time) {
	auto index = 0;
	for (auto& material : m_Materials) {
		d3d::MaterialConstants data{
			.diffuse_albedo = material.diffuse_albedo,
			.fresnel_r0 = material.fresnel_r0,
			.roughness = material.roughness,
		};
		material.material_buffer.update(&data);
		material.material_cb_index = index;
		index++;
	}
}

void SandboxGameContext::udpate_transform_buffer(f32 delta_time) {
	auto& transforms = m_ECManager.engine_components<components::Transform>();
	for (u32 i = 0; i < m_TransformBuffers.size(); ++i) {
		DirectX::XMMATRIX world = transforms[i].transform();
		ObjectConstants obj_constants;
		XMStoreFloat4x4(&obj_constants.World, XMMatrixTranspose(world));
		m_TransformBuffers[i].update(&obj_constants);
	}
}

void SandboxGameContext::render() {
	m_GraphicsDevice->begin_frame();
	auto& gfx_ctx = m_GraphicsDevice->current_graphics_contexts();
	auto& current_backbuffer = m_GraphicsDevice->current_back_buffer();
	gfx_ctx->add_resource_barrier(current_backbuffer.allocation.resource.Get(), D3D12_RESOURCE_STATE_PRESENT,
								  D3D12_RESOURCE_STATE_RENDER_TARGET);
	gfx_ctx->execute_resource_barriers();
	static stl::array<f32, 4> clear_color{0.3f, 0.4f, 0.6f, 1.0f};
	gfx_ctx->clear_render_target_view(current_backbuffer, clear_color);
	gfx_ctx->clear_depth_stencil_view(m_DepthTexture);
	// TODO: setup barriers for all passes
	gfx_ctx->set_graphics_root_signature_and_pipeline(m_PipelineState);
	gfx_ctx->set_render_target(current_backbuffer, m_DepthTexture);
	gfx_ctx->set_viewport({
		.TopLeftX = 0.0f,
		.TopLeftY = 0.0f,
		.Width = static_cast<f32>(m_ClientExtent->width),
		.Height = static_cast<f32>(m_ClientExtent->height),
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f,
	});
	// TODO: rendering
	{
		gfx_ctx->set_descriptor_heaps();
		gfx_ctx->set_primitive_topology_layout(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		auto& render_components = m_ECManager.engine_components<components::RenderComponent>();
		for (auto& comp : render_components) {
			components::CPUData cpu_data = comp.cpu_data();
			gfx_ctx->set_index_buffer(m_RTIndexBuffers[cpu_data.index_id]);
			gfx_ctx->set_32_bit_graphics_constants(comp.per_draw_constants());
			gfx_ctx->draw_instance_indexed(cpu_data.indices_size);
		}
	}
	gfx_ctx->add_resource_barrier(current_backbuffer.allocation.resource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
								  D3D12_RESOURCE_STATE_PRESENT);
	gfx_ctx->execute_resource_barriers();
	stl::array<const d3d::Context*, 1> contexts = {gfx_ctx.get()};
	m_GraphicsDevice->direct_command_queue()->execute_context(contexts);
	m_GraphicsDevice->present();
	m_GraphicsDevice->end_frame();
}

void SandboxGameContext::resize_depth_texture() {
	m_MainCamera = {CAMERA_FOV, static_cast<f32>(m_ClientExtent->width) / m_ClientExtent->height, 0.1f, 1000.f};
	m_DepthTexture.allocation.reset();
	m_DepthTexture = m_GraphicsDevice->create_texture({
		.usage = d3d::TextureUsage::DepthStencil,
		.width = static_cast<u32>(m_ClientExtent->width),
		.height = static_cast<u32>(m_ClientExtent->height),
		.format = DXGI_FORMAT_D32_FLOAT,
		.name = L"Depth Texture",
	});
}
