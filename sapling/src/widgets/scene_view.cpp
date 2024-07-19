#include "widgets/scene_view.h"
#include "core/timer.h"
#include "globals.h"
#include "imgui.h"
#include "render/d3d_util.h"
#include "sapling_layer.h"
#include "subeditors/level_editor.h"

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
	SSceneView::SSceneView(Sapfire::stl::string_view name, Sapfire::ECManager* ec_manager, Sapfire::d3d::GraphicsDevice* gfx_device) :
		m_ECManager(*ec_manager), m_GraphicsDevice(*gfx_device),
		m_PhysicsEngine(stl::make_unique<physics::PhysicsEngine>(mem::ENUM::Editor, ec_manager)),
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
		m_DepthTexture(m_GraphicsDevice.create_texture({
			.usage = d3d::TextureUsage::DepthStencil,
			.width = 800,
			.height = 600,
			.format = DXGI_FORMAT_D32_FLOAT,
			.name = L"Scene View Depth Texture",
		})),
		m_MainPassCB(m_GraphicsDevice.create_buffer<PassConstants>(d3d::BufferCreationDesc{
			.usage = d3d::BufferUsage::ConstantBuffer,
			.name = L"Scene View Main Pass Constant Buffer",
		})),
		m_WidgetName(name) {
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
		m_MainCamera = {CAMERA_FOV, DOCK_SIZE.x / DOCK_SIZE.y, 0.1f, 1000.f};
		s_Instance.reset(this);
	}

	void SSceneView::add_render_component(Sapfire::Entity entity, const Sapfire::RenderComponentResourcePaths& resource_paths) {
		bool already_has_component = m_ECManager.has_engine_component<components::RenderComponent>(entity);
		auto* mesh_asset = resource_paths.mesh_path.empty() ? assets::MeshRegistry::default_mesh()
															: editor()->asset_manager()->get_mesh(resource_paths.mesh_path);
		auto* texture_asset = resource_paths.texture_path.empty() ? assets::TextureRegistry::default_texture(&m_GraphicsDevice)
																  : editor()->asset_manager()->get_texture(resource_paths.texture_path);
		auto* material_asset = resource_paths.material_path.empty() ? assets::MaterialRegistry::default_material(&m_GraphicsDevice)
																   : editor()->asset_manager()->get_material(resource_paths.material_path);
		if (!mesh_asset) {
			editor()->asset_manager()->import_mesh(resource_paths.mesh_path);
			mesh_asset = editor()->asset_manager()->get_mesh(resource_paths.mesh_path);
		}
		if (!texture_asset || !editor()->asset_manager()->is_texture_loaded_for_runtime(texture_asset->uuid)) {
			editor()->asset_manager()->import_texture(resource_paths.texture_path);
			texture_asset = editor()->asset_manager()->get_texture(resource_paths.texture_path);
		}
		if (!material_asset || !editor()->asset_manager()->material_resource_exists(material_asset->uuid)) {
			editor()->asset_manager()->import_material(resource_paths.material_path);
			material_asset = editor()->asset_manager()->get_material(resource_paths.material_path);
		}
		if (mesh_asset && mesh_asset->data.has_value()) {
			assert(mesh_asset->data->indices32.size() > 0);
			assert(mesh_asset->data->positions.size() > 0);
			assert(mesh_asset->data->normals.size() > 0);
			assert(mesh_asset->data->texcs.size() > 0);
			if (!already_has_component) {
				m_TransformBuffers.emplace_back(m_GraphicsDevice.create_buffer<ObjectConstants>({
					.usage = d3d::BufferUsage::ConstantBuffer,
					.name = L"Transform buffer " + d3d::AnsiToWString(resource_paths.mesh_path),
				}));
			}
			bool should_add_tangent = false;
			bool should_allocate_mesh = !editor()->asset_manager()->mesh_resource_exists(resource_paths.mesh_path);
			if (should_allocate_mesh) {
				stl::wstring name = mesh_asset->uuid == assets::MeshRegistry::default_mesh()->uuid
					? L"Default Mesh"
					: d3d::AnsiToWString(resource_paths.mesh_path);
				m_RTIndexBuffers.push_back(m_GraphicsDevice.create_buffer<u16>(
					d3d::BufferCreationDesc{
						.usage = d3d::BufferUsage::IndexBuffer,
						.name = L"Index buffer " + name,
					},
					mesh_asset->data->indices16()));
				m_VertexPosBuffers.push_back(m_GraphicsDevice.create_buffer<DirectX::XMFLOAT3>(
					d3d::BufferCreationDesc{
						.usage = d3d::BufferUsage::StructuredBuffer,
						.name = L"Vertex Pos buffer " + name,
					},
					mesh_asset->data->positions));
				m_VertexNormalBuffers.push_back(m_GraphicsDevice.create_buffer<DirectX::XMFLOAT3>(
					d3d::BufferCreationDesc{
						.usage = d3d::BufferUsage::StructuredBuffer,
						.name = L"Vertex Norm buffer " + name,
					},
					mesh_asset->data->normals));
				if (mesh_asset->data->tangentus.size() > 0) {
					m_VertexTangentBuffers.push_back(m_GraphicsDevice.create_buffer<DirectX::XMFLOAT3>(
						d3d::BufferCreationDesc{
							.usage = d3d::BufferUsage::StructuredBuffer,
							.name = L"Vertex Tang buffer " + name,
						},
						mesh_asset->data->tangentus));
					should_add_tangent = true;
				}
				m_VertexUVBuffers.push_back(m_GraphicsDevice.create_buffer<DirectX::XMFLOAT2>(
					d3d::BufferCreationDesc{
						.usage = d3d::BufferUsage::StructuredBuffer,
						.name = L"Vertex UV buffer " + name,
					},
					mesh_asset->data->texcs));
			}
			auto cpu_data = components::CPUData{
				.indices_size = static_cast<u32>(mesh_asset->data->indices32.size()),
				.index_id = static_cast<u32>(m_RTIndexBuffers.size() - 1),
				.position_idx = static_cast<u32>(m_VertexPosBuffers.size() - 1),
				.normal_idx = static_cast<u32>(m_VertexNormalBuffers.size() - 1),
				.tangent_idx = static_cast<u32>(should_add_tangent ? m_VertexTangentBuffers.size() - 1 : 0),
				.uv_idx = static_cast<u32>(m_VertexUVBuffers.size() - 1),
				.transform_buffer_idx = already_has_component
					? m_ECManager.engine_component<components::RenderComponent>(entity).cpu_data().transform_buffer_idx
					: static_cast<u32>(m_TransformBuffers.size() - 1),

			};
			u32 material_cbuffer_idx = 0;
			if (material_asset->uuid == assets::MaterialRegistry::default_material(&m_GraphicsDevice)->uuid) {
				material_cbuffer_idx = material_asset->material.material_cb_index;
			} else {
				material_cbuffer_idx = editor()->asset_manager()->material_resource_exists(resource_paths.material_path)
					? editor()->asset_manager()->get_material_resource(resource_paths.material_path).gpu_idx
					: assets::MaterialRegistry::default_material(&m_GraphicsDevice)->material.material_cb_index;
			}
			u32 texture_cbuffer_idx = 0;
			if (texture_asset->uuid == assets::TextureRegistry::default_texture(&m_GraphicsDevice)->uuid) {
				texture_cbuffer_idx = texture_asset->data.srv_index;
			} else {
				texture_cbuffer_idx = editor()->asset_manager()->texture_resource_exists(resource_paths.texture_path)
					? editor()->asset_manager()->get_texture_resource(resource_paths.texture_path).gpu_idx
					: texture_asset->data.srv_index;
			}
			auto gpu_data = components::PerDrawConstants{
				.position_buffer_idx = m_VertexPosBuffers.back().srv_index,
				.normal_buffer_idx = m_VertexNormalBuffers.back().srv_index,
				.tangent_buffer_idx = should_add_tangent ? m_VertexTangentBuffers.back().srv_index : 0,
				.uv_buffer_idx = m_VertexUVBuffers.back().srv_index,
				.scene_cbuffer_idx = already_has_component
					? m_ECManager.engine_component<components::RenderComponent>(entity).per_draw_constants()->scene_cbuffer_idx
					: m_TransformBuffers.back().cbv_index,
				.pass_cbuffer_idx = m_MainPassCB.cbv_index,
				.material_cbuffer_idx = material_cbuffer_idx,
				.texture_cbuffer_idx = texture_cbuffer_idx,
			};
			if (!should_allocate_mesh) {
				cpu_data = editor()->asset_manager()->get_mesh_resource(mesh_asset->uuid).cpu_data;
				gpu_data = editor()->asset_manager()->get_mesh_resource(mesh_asset->uuid).gpu_data;
				gpu_data.scene_cbuffer_idx = already_has_component
					? m_ECManager.engine_component<components::RenderComponent>(entity).per_draw_constants()->scene_cbuffer_idx
					: m_TransformBuffers.back().cbv_index;
				gpu_data.material_cbuffer_idx = material_cbuffer_idx;
				gpu_data.texture_cbuffer_idx = texture_cbuffer_idx;
				cpu_data.transform_buffer_idx = already_has_component
					? m_ECManager.engine_component<components::RenderComponent>(entity).cpu_data().transform_buffer_idx
					: static_cast<u32>(m_TransformBuffers.size() - 1);
			}
			editor()->asset_manager()->load_mesh_resource(resource_paths.mesh_path, {cpu_data, gpu_data});
			components::RenderComponent render_component{
				mesh_asset->uuid,
				texture_asset->uuid,
				material_asset->uuid,
				cpu_data,
				gpu_data,
				[this, entity](components::RenderComponent* component) {
					if (component) {
						const auto old_cpu_data = component->cpu_data();
						const auto old_gpu_data = component->per_draw_constants();
						// This happens after the mesh uuid is set to the new one, so we're getting the just assigned uuid
						const auto mesh_uuid = component->mesh_uuid();
						// The mesh we just assigned may not be allocated yet
						const auto texture_uuid = component->texture_uuid();
						const auto texture_path = editor()->asset_manager()->get_texture_path(texture_uuid);
						const auto mesh_path = editor()->asset_manager()->get_mesh_path(mesh_uuid);
						const auto material_uuid = component->material_uuid();
						const auto material_path = editor()->asset_manager()->get_material_path(material_uuid);
						if (!editor()->asset_manager()->mesh_resource_exists(mesh_path) ||
							!editor()->asset_manager()->material_resource_exists(material_path) ||
							!editor()->asset_manager()->texture_resource_exists(texture_path)) {
							add_render_component(entity,
												 {.mesh_path = mesh_path, .texture_path = texture_path, .material_path = material_path});
							return;
						}
						auto data = editor()->asset_manager()->get_mesh_resource(mesh_path);
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
		if (ImGui::Begin(m_WidgetName.c_str())) {
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
		for (auto&& [path, asset] : editor()->asset_manager()->path_material_map()) {
			d3d::MaterialConstants data{
				.diffuse_albedo = asset.material.diffuse_albedo,
				.fresnel_r0 = asset.material.fresnel_r0,
				.roughness = asset.material.roughness,
			};
			asset.material.material_buffer.update(&data);
		}
	}

	void SSceneView::update_transform_buffer() {
		auto& entities = m_ECManager.entities();
		for (auto& entry : entities) {
			if (!entry.has_value())
				continue;
			auto entity = entry.value().value;
			if (!m_ECManager.has_engine_component<components::RenderComponent>(entity))
				continue;
			auto& render_component = m_ECManager.engine_component<components::RenderComponent>(entity);
			auto transform = m_ECManager.engine_component<components::Transform>(entity);
			DirectX::XMMATRIX world = transform.transform();
			ObjectConstants obj_constants;
			XMStoreFloat4x4(&obj_constants.World, XMMatrixTranspose(world));
			m_TransformBuffers[render_component.cpu_data().transform_buffer_idx].update(&obj_constants);
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
			auto pdc = comp.per_draw_constants();
			gfx_ctx.set_index_buffer(m_RTIndexBuffers[cpu_data.index_id]);
			gfx_ctx.set_32_bit_graphics_constants(comp.per_draw_constants());
			gfx_ctx.draw_instance_indexed(cpu_data.indices_size);
		}
		gfx_ctx.add_resource_barrier(offscreen_texture.allocation.resource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
									 D3D12_RESOURCE_STATE_PRESENT);
		gfx_ctx.execute_resource_barriers();
	}
} // namespace widgets
