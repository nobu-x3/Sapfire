#include "Sapfire.h"

#include "core/timer.h"
#include "imgui.h"
#include "render/d3d_util.h"
#include "subeditors/level_editor.h"
#include "widgets/scene_view.h"

namespace widgets {

	Sapfire::stl::shared_ptr<SSceneView> SSceneView::s_Instance{nullptr};

	Sapfire::stl::shared_ptr<SSceneView> SSceneView::scene_view() { return s_Instance; }

	static ImVec2 DOCK_SIZE{800, 600};

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

	using namespace Sapfire;
	SSceneView::SSceneView(Sapfire::ECManager* ec_manager, Sapfire::d3d::GraphicsDevice* gfx_device,
						   Sapfire::assets::MeshRegistry* mesh_reg) :
		m_ECManager(*ec_manager), m_GraphicsDevice(*gfx_device), m_MeshRegistry(*mesh_reg), m_DepthTexture(m_GraphicsDevice.create_texture({
																								.usage = d3d::TextureUsage::DepthStencil,
																								.width = 800,
																								.height = 600,
																								.format = DXGI_FORMAT_D32_FLOAT,
																								.name = L"Scene View Depth Texture",
																							})),
		m_PipelineState(m_GraphicsDevice.create_pipeline_state({
			.shader_module =
				{
					.vertexShaderPath = L"bindless.hlsl",
					.vertexEntryPoint = L"VS",
					.pixelShaderPath = L"bindless.hlsl",
					.pixelEntryPoint = L"PS",
				},
			.pipeline_name = L"Scene View Bindless Pipeline",
		})),
		m_MainPassCB(m_GraphicsDevice.create_buffer<PassConstants>(d3d::BufferCreationDesc{
			.usage = d3d::BufferUsage::ConstantBuffer,
			.name = L"Scene View Main Pass Constant Buffer",
		})),
		m_PhysicsEngine(stl::make_unique<physics::PhysicsEngine>(mem::ENUM::Editor, ec_manager)) {
		for (int i = 0; i < d3d::MAX_FRAMES_IN_FLIGHT; ++i) {
			m_OffscreenTextures.push_back(m_GraphicsDevice.create_texture({
				.usage = Sapfire::d3d::TextureUsage::RenderTarget,
				.width = 800,
				.height = 600,
				.format = DXGI_FORMAT_R16G16B16A16_FLOAT,
				.optional_initial_state = D3D12_RESOURCE_STATE_COMMON,
				.name = L"Scene View Offscreen Texture",
			}));
		}
		d3d::Material grass{};
		grass.name = "grass";
		grass.material_cb_index = 0;
		grass.diffuse_albedo = DirectX::XMFLOAT4{0.2f, 0.6f, 0.6f, 1.0f};
		grass.roughness = 0.125f;
		grass.material_buffer = m_GraphicsDevice.create_buffer<d3d::MaterialConstants>({
			.usage = d3d::BufferUsage::ConstantBuffer,
			.name = L"Material Buffer - Grass",
		});
		m_Materials.push_back(grass);
		m_MainCamera = {CAMERA_FOV, DOCK_SIZE.x / DOCK_SIZE.y, 0.1f, 1000.f};
		s_Instance.reset(this);
	}

	void SSceneView::allocate_mesh(const Sapfire::stl::string& mesh_path) {
		auto* asset = m_MeshRegistry.get(mesh_path);
		if (!asset) {
			m_MeshRegistry.import_mesh(mesh_path);
			asset = m_MeshRegistry.get(mesh_path);
		}
		if (asset && asset->data.has_value()) {
			assert(asset->data->indices32.size() > 0);
			assert(asset->data->positions.size() > 0);
			assert(asset->data->normals.size() > 0);
			assert(asset->data->texcs.size() > 0);
			m_TransformBuffers.emplace_back(m_GraphicsDevice.create_buffer<ObjectConstants>({
				.usage = d3d::BufferUsage::ConstantBuffer,
				.name = L"Transform buffer " + d3d::AnsiToWString(mesh_path),
			}));
			m_RTIndexBuffers.push_back(m_GraphicsDevice.create_buffer<u16>(
				d3d::BufferCreationDesc{
					.usage = d3d::BufferUsage::IndexBuffer,
					.name = L"Index buffer " + d3d::AnsiToWString(mesh_path),
				},
				asset->data->indices16()));
			m_VertexPosBuffers.push_back(m_GraphicsDevice.create_buffer<DirectX::XMFLOAT3>(
				d3d::BufferCreationDesc{
					.usage = d3d::BufferUsage::StructuredBuffer,
					.name = L"Vertex Pos buffer " + d3d::AnsiToWString(mesh_path),
				},
				asset->data->positions));
			m_VertexNormalBuffers.push_back(m_GraphicsDevice.create_buffer<DirectX::XMFLOAT3>(
				d3d::BufferCreationDesc{
					.usage = d3d::BufferUsage::StructuredBuffer,
					.name = L"Vertex Norm buffer " + d3d::AnsiToWString(mesh_path),
				},
				asset->data->normals));
			bool should_add_tangent = false;
			if (asset->data->tangentus.size() > 0) {
				m_VertexTangentBuffers.push_back(m_GraphicsDevice.create_buffer<DirectX::XMFLOAT3>(
					d3d::BufferCreationDesc{
						.usage = d3d::BufferUsage::StructuredBuffer,
						.name = L"Vertex Tang buffer " + d3d::AnsiToWString(mesh_path),
					},
					asset->data->tangentus));
				should_add_tangent = true;
			}
			m_VertexUVBuffers.push_back(m_GraphicsDevice.create_buffer<DirectX::XMFLOAT2>(
				d3d::BufferCreationDesc{
					.usage = d3d::BufferUsage::StructuredBuffer,
					.name = L"Vertex UV buffer " + d3d::AnsiToWString(mesh_path),
				},
				asset->data->texcs));
			auto cpu_data = components::CPUData{
				.indices_size = static_cast<u32>(asset->data->indices32.size()),
				.index_id = static_cast<u32>(m_RTIndexBuffers.size() - 1),
				.position_idx = static_cast<u32>(m_VertexPosBuffers.size() - 1),
				.normal_idx = static_cast<u32>(m_VertexNormalBuffers.size() - 1),
				.tangent_idx = static_cast<u32>(should_add_tangent ? m_VertexTangentBuffers.size() - 1 : 0),
				.uv_idx = static_cast<u32>(m_VertexUVBuffers.size() - 1),
			};
			auto gpu_data = components::PerDrawConstants{
				.position_buffer_idx = m_VertexPosBuffers.back().srv_index,
				.normal_buffer_idx = m_VertexNormalBuffers.back().srv_index,
				.tangent_buffer_idx = should_add_tangent ? m_VertexTangentBuffers.back().srv_index : 0,
				.uv_buffer_idx = m_VertexUVBuffers.back().srv_index,
				.scene_cbuffer_idx = m_TransformBuffers.back().cbv_index,
				.pass_cbuffer_idx = m_MainPassCB.cbv_index,
				.material_cbuffer_idx = m_Materials[0].material_buffer.cbv_index,
				.texture_cbuffer_idx = 0,
			};
			SLevelEditor::level_editor()->mesh_manager().mesh_resources[mesh_path] = {cpu_data, gpu_data};
		}
	}

	void SSceneView::add_render_component(Sapfire::Entity entity, const Sapfire::stl::string& mesh_path) {
		// @TODO: add materials and textures
		bool already_has_component = m_ECManager.has_engine_component<components::RenderComponent>(entity);
		auto* asset = m_MeshRegistry.get(mesh_path);
		if (!asset) {
			m_MeshRegistry.import_mesh(mesh_path);
			asset = m_MeshRegistry.get(mesh_path);
		}
		if (asset && asset->data.has_value()) {
			assert(asset->data->indices32.size() > 0);
			assert(asset->data->positions.size() > 0);
			assert(asset->data->normals.size() > 0);
			assert(asset->data->texcs.size() > 0);
			if (!already_has_component) {
				m_TransformBuffers.emplace_back(m_GraphicsDevice.create_buffer<ObjectConstants>({
					.usage = d3d::BufferUsage::ConstantBuffer,
					.name = L"Transform buffer " + d3d::AnsiToWString(mesh_path),
				}));
			}
			m_RTIndexBuffers.push_back(m_GraphicsDevice.create_buffer<u16>(
				d3d::BufferCreationDesc{
					.usage = d3d::BufferUsage::IndexBuffer,
					.name = L"Index buffer " + d3d::AnsiToWString(mesh_path),
				},
				asset->data->indices16()));
			m_VertexPosBuffers.push_back(m_GraphicsDevice.create_buffer<DirectX::XMFLOAT3>(
				d3d::BufferCreationDesc{
					.usage = d3d::BufferUsage::StructuredBuffer,
					.name = L"Vertex Pos buffer " + d3d::AnsiToWString(mesh_path),
				},
				asset->data->positions));
			m_VertexNormalBuffers.push_back(m_GraphicsDevice.create_buffer<DirectX::XMFLOAT3>(
				d3d::BufferCreationDesc{
					.usage = d3d::BufferUsage::StructuredBuffer,
					.name = L"Vertex Norm buffer " + d3d::AnsiToWString(mesh_path),
				},
				asset->data->normals));
			bool should_add_tangent = false;
			if (asset->data->tangentus.size() > 0) {
				m_VertexTangentBuffers.push_back(m_GraphicsDevice.create_buffer<DirectX::XMFLOAT3>(
					d3d::BufferCreationDesc{
						.usage = d3d::BufferUsage::StructuredBuffer,
						.name = L"Vertex Tang buffer " + d3d::AnsiToWString(mesh_path),
					},
					asset->data->tangentus));
				should_add_tangent = true;
			}
			m_VertexUVBuffers.push_back(m_GraphicsDevice.create_buffer<DirectX::XMFLOAT2>(
				d3d::BufferCreationDesc{
					.usage = d3d::BufferUsage::StructuredBuffer,
					.name = L"Vertex UV buffer " + d3d::AnsiToWString(mesh_path),
				},
				asset->data->texcs));
			auto cpu_data = components::CPUData{
				.indices_size = static_cast<u32>(asset->data->indices32.size()),
				.index_id = static_cast<u32>(m_RTIndexBuffers.size() - 1),
				.position_idx = static_cast<u32>(m_VertexPosBuffers.size() - 1),
				.normal_idx = static_cast<u32>(m_VertexNormalBuffers.size() - 1),
				.tangent_idx = static_cast<u32>(should_add_tangent ? m_VertexTangentBuffers.size() - 1 : 0),
				.uv_idx = static_cast<u32>(m_VertexUVBuffers.size() - 1),
			};
			auto gpu_data = components::PerDrawConstants{
				.position_buffer_idx = m_VertexPosBuffers.back().srv_index,
				.normal_buffer_idx = m_VertexNormalBuffers.back().srv_index,
				.tangent_buffer_idx = should_add_tangent ? m_VertexTangentBuffers.back().srv_index : 0,
				.uv_buffer_idx = m_VertexUVBuffers.back().srv_index,
				.scene_cbuffer_idx = already_has_component
					? m_ECManager.engine_component<components::RenderComponent>(entity).per_draw_constants()->scene_cbuffer_idx
					: m_TransformBuffers.back().cbv_index,
				.pass_cbuffer_idx = m_MainPassCB.cbv_index,
				.material_cbuffer_idx = m_Materials[0].material_buffer.cbv_index,
				.texture_cbuffer_idx = 0,
			};
            SLevelEditor::level_editor()->mesh_manager().mesh_resources[mesh_path] = {cpu_data, gpu_data};
			components::RenderComponent render_component{asset->uuid, {}, cpu_data, gpu_data,
														 [this, entity](components::RenderComponent* component) {
															 if (component) {
																 const auto old_cpu_data = component->cpu_data();
																 const auto old_gpu_data = component->per_draw_constants();
																 const auto uuid = component->mesh_uuid();
																 const auto path = m_MeshRegistry.get_path(uuid);
																 auto data = SLevelEditor::level_editor()->mesh_manager().mesh_resources[path];
																 data.gpu_data.scene_cbuffer_idx = old_gpu_data->scene_cbuffer_idx;
																 component->cpu_data(data.cpu_data);
																 component->per_draw_constants(data.gpu_data);
															 }
														 }};
			m_ECManager.add_engine_component<components::RenderComponent>(entity, render_component);
		}
	}

	Sapfire::Timer g_RiseTimer{};

	bool SSceneView::update(f32 delta_time) {
		m_PhysicsEngine->simulate(delta_time);
		auto& transforms = m_ECManager.engine_components<components::Transform>();
		for (auto& transform : transforms) {
			transform.update(transforms);
		}
		auto& entries = m_ECManager.entities();
		for (auto& entry : entries) {
			if (!entry.has_value())
				continue;
			auto& entity = entry->value;
			auto components = m_ECManager.components(entity);
			for (auto& component : components) {
				component->update(delta_time);
			}
		}
		m_MainCamera.update(delta_time);
		update_pass_cb(delta_time);
		update_materials();
		update_transform_buffer();
		if (ImGui::Begin("Scene View")) {
			auto work_size = ImGui::GetWindowSize();
			if (DOCK_SIZE.x != work_size.x || DOCK_SIZE.y != work_size.y) {
				m_Resizing = true;
				DOCK_SIZE = work_size;
				g_RiseTimer.reset();
			} else if (m_Resizing && g_RiseTimer.elapsed_millis() > 1000) {
				m_MainCamera = {CAMERA_FOV, work_size.x / work_size.y, 0.1f, 1000.f};
				m_Resizing = false;
				m_GraphicsDevice.direct_command_queue()->flush();
				for (int i = 0; i < d3d::MAX_FRAMES_IN_FLIGHT; ++i) {
					auto& offscreen_texture = m_OffscreenTextures[i];
					offscreen_texture.allocation.reset();
					offscreen_texture = m_GraphicsDevice.create_texture({
						.usage = Sapfire::d3d::TextureUsage::RenderTarget,
						.width = static_cast<u32>(DOCK_SIZE.x),
						.height = static_cast<u32>(DOCK_SIZE.y),
						.format = DXGI_FORMAT_R16G16B16A16_FLOAT,
						.optional_initial_state = D3D12_RESOURCE_STATE_COMMON,
						.name = L"Scene View Offscreen Texture " + std::to_wstring(i),
					});
				}
				m_DepthTexture.allocation.reset();
				m_DepthTexture = m_GraphicsDevice.create_texture({
					.usage = d3d::TextureUsage::DepthStencil,
					.width = static_cast<u32>(DOCK_SIZE.x),
					.height = static_cast<u32>(DOCK_SIZE.y),
					.format = DXGI_FORMAT_D32_FLOAT,
					.name = L"Scene View Depth Texture",
				});
				ImGui::End();
				return false;
			}
			ImGui::Image((ImTextureID)m_GraphicsDevice.cbv_srv_uav_descriptor_heap()
							 ->descriptor_handle_from_index(m_OffscreenTextures[m_GraphicsDevice.current_frame_id()].srv_index)
							 .gpu_descriptor_handle.ptr,
						 ImVec2(DOCK_SIZE.x, DOCK_SIZE.y));
		}
		ImGui::End();
		return true;
	}

	void SSceneView::update_pass_cb(f32 delta_time) {
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
		main_pass.render_target_size = DirectX::XMFLOAT2(DOCK_SIZE.x, DOCK_SIZE.y);
		main_pass.inv_render_target_size = DirectX::XMFLOAT2(1.0f / DOCK_SIZE.x, 1.0f / DOCK_SIZE.y);
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

	void SSceneView::update_materials() {
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

	void SSceneView::update_transform_buffer() {
		auto& transforms = m_ECManager.engine_components<components::Transform>();
		for (u32 i = 0; i < m_TransformBuffers.size(); ++i) {
			DirectX::XMMATRIX world = transforms[i].transform();
			ObjectConstants obj_constants;
			XMStoreFloat4x4(&obj_constants.World, XMMatrixTranspose(world));
			m_TransformBuffers[i].update(&obj_constants);
		}
	}

	void SSceneView::render(Sapfire::d3d::GraphicsContext& gfx_ctx) {
		if (m_Resizing)
			return;
		auto& offscreen_texture = m_OffscreenTextures[m_GraphicsDevice.current_frame_id()];
		gfx_ctx.add_resource_barrier(offscreen_texture.allocation.resource.Get(), D3D12_RESOURCE_STATE_PRESENT,
									 D3D12_RESOURCE_STATE_RENDER_TARGET);
		gfx_ctx.execute_resource_barriers();
		static stl::array<f32, 4> clear_color{0.3f, 0.4f, 0.6f, 1.0f};
		gfx_ctx.clear_render_target_view(offscreen_texture, clear_color);
		gfx_ctx.clear_depth_stencil_view(m_DepthTexture);
		// TODO: setup barriers for all passes
		gfx_ctx.set_graphics_root_signature_and_pipeline(m_PipelineState);
		gfx_ctx.set_render_target(offscreen_texture, m_DepthTexture);
		gfx_ctx.set_viewport({
			.TopLeftX = 0.0f,
			.TopLeftY = 0.0f,
			.Width = DOCK_SIZE.x,
			.Height = DOCK_SIZE.y,
			.MinDepth = 0.0f,
			.MaxDepth = 1.0f,
		});
		gfx_ctx.set_descriptor_heaps();
		gfx_ctx.set_primitive_topology_layout(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		auto& render_components = m_ECManager.engine_components<components::RenderComponent>();
		for (auto& comp : render_components) {
			components::CPUData cpu_data = comp.cpu_data();
			gfx_ctx.set_index_buffer(m_RTIndexBuffers[cpu_data.index_id]);
			gfx_ctx.set_32_bit_graphics_constants(comp.per_draw_constants());
			gfx_ctx.draw_instance_indexed(cpu_data.indices_size);
		}
		gfx_ctx.add_resource_barrier(offscreen_texture.allocation.resource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
									 D3D12_RESOURCE_STATE_PRESENT);
		gfx_ctx.execute_resource_barriers();
	}
} // namespace widgets
