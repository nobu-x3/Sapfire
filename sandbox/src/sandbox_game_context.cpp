#include "sandbox_game_context.h"
#include <DirectXCollision.h>
#include <DirectXMath.h>
#include "components/movement_component.h"
#include "components/render_component.h"
#include "components/test_custom_component.h"
#include "core/game_context.h"

using namespace Sapfire;

SandboxGameContext::SandboxGameContext(const Sapfire::GameContextCreationDesc& desc) : Sapfire::GameContext(desc) {
	m_MainCamera = {CAMERA_FOV, static_cast<f32>(m_ClientExtent->width) / m_ClientExtent->height, 0.1f, 1000.f};
}

void SandboxGameContext::load_contents() {
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
	m_MainPassCB = m_GraphicsDevice->create_buffer<PassConstants>(d3d::BufferCreationDesc{
		.usage = d3d::BufferUsage::ConstantBuffer,
		.name = L"Main Pass Constant Buffer",
	});
	// textures:
	m_DepthTexture = m_GraphicsDevice->create_texture({
		.usage = d3d::TextureUsage::DepthStencil,
		.width = static_cast<u32>(m_ClientExtent->width),
		.height = static_cast<u32>(m_ClientExtent->height),
		.format = DXGI_FORMAT_D32_FLOAT,
		.name = L"Depth Texture",
	});
	assets::SceneWriter writer{&m_ECManager, &m_AssetManager};
	writer.deserealize("test_scene.scene", [&](Sapfire::Entity entity, const Sapfire::RenderComponentResourcePaths& resource_paths) {
		create_render_component(entity, resource_paths);
	});
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
	m_PassConstants = PassConstants{};
	auto view = m_MainCamera.view();
	auto proj = m_MainCamera.projection;
	auto viewProj = XMMatrixMultiply(view, proj);
	auto view_determinant = XMMatrixDeterminant(view);
	auto invView = XMMatrixInverse(&view_determinant, view);
	auto proj_determinant = XMMatrixDeterminant(proj);
	auto invProj = XMMatrixInverse(&proj_determinant, proj);
	auto view_proj_determinant = XMMatrixDeterminant(viewProj);
	auto invViewProj = XMMatrixInverse(&view_proj_determinant, viewProj);
	XMStoreFloat4x4(&m_PassConstants.view, XMMatrixTranspose(view));
	XMStoreFloat4x4(&m_PassConstants.inv_view, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&m_PassConstants.proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&m_PassConstants.inv_proj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&m_PassConstants.view_proj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&m_PassConstants.inv_view_proj, XMMatrixTranspose(invViewProj));
	XMStoreFloat3(&m_PassConstants.EyePosW, m_MainCamera.transform.position());
	m_PassConstants.render_target_size = DirectX::XMFLOAT2((float)m_ClientExtent->width, (float)m_ClientExtent->height);
	m_PassConstants.inv_render_target_size = DirectX::XMFLOAT2(1.0f / m_ClientExtent->width, 1.0f / m_ClientExtent->height);
	m_PassConstants.near_z = 1.0f;
	m_PassConstants.far_z = 1000.0f;
	m_PassConstants.total_time = delta_time;
	m_PassConstants.delta_time = delta_time;
	m_PassConstants.ambient_light = {0.25f, 0.25f, 0.35f, 1.0f};
	m_PassConstants.Lights[0].direction = {0.57735f, -0.0f, 1.57735f};
	m_PassConstants.Lights[0].strength = {0.6f, 0.6f, 0.6f};
	m_PassConstants.Lights[1].direction = {-0.57735f, -0.57735f, 0.57735f};
	m_PassConstants.Lights[1].strength = {0.3f, 0.3f, 0.3f};
	m_PassConstants.Lights[2].direction = {0.0f, -0.707f, -0.707f};
	m_PassConstants.Lights[2].strength = {0.15f, 0.15f, 0.15f};
	m_MainPassCB.update(&m_PassConstants);
}

void SandboxGameContext::update_materials(f32 delta_time) {
	auto index = 0;
	for (auto&& [path, asset] : m_AssetManager.path_material_map()) {
		d3d::MaterialConstants data{
			.diffuse_albedo = asset.material.diffuse_albedo,
			.fresnel_r0 = asset.material.fresnel_r0,
			.roughness = asset.material.roughness,
		};
		asset.material.material_buffer.update(&data);
		asset.material.material_cb_index = index;
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
	PROFILE_FUNCTION();
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
		DirectX::BoundingFrustum camera_frustum;
		DirectX::BoundingFrustum::CreateFromMatrix(camera_frustum, m_MainCamera.projection);
		auto view = m_MainCamera.view();
		auto view_determinant = DirectX::XMMatrixDeterminant(view);
		auto inv_view = DirectX::XMMatrixInverse(&view_determinant, view);
		auto& render_components = m_ECManager.engine_components<components::RenderComponent>();
		for (auto& comp : render_components) {
			components::Transform transform;
			if (!m_ECManager.get_other_engine_component<components::RenderComponent, components::Transform>(comp, transform))
				continue;
			const auto* mesh_asset = m_AssetManager.get_mesh(comp.mesh_uuid());
			DirectX::BoundingBox aabb = mesh_asset->data->aabb;
			DirectX::XMMATRIX world = transform.transform();
			DirectX::XMVECTOR world_determinant = DirectX::XMMatrixDeterminant(world);
			DirectX::XMMATRIX inv_world = DirectX::XMMatrixInverse(&world_determinant, world);
			// View space to object local space
			DirectX::XMMATRIX view_to_local = DirectX::XMMatrixMultiply(inv_world, inv_view);
			// Transform camera frustum from view space to object's local space
			DirectX::BoundingFrustum local_space_frustum;
			camera_frustum.Transform(local_space_frustum, view_to_local);
			if (local_space_frustum.Contains(aabb) != DirectX::DISJOINT) {
				components::CPUData cpu_data = comp.cpu_data();
				gfx_ctx->set_index_buffer(m_RTIndexBuffers[cpu_data.index_id]);
				gfx_ctx->set_32_bit_graphics_constants(comp.per_draw_constants());
				gfx_ctx->draw_instance_indexed(cpu_data.indices_size);
			}
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
