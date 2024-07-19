#include "engpch.h"

#include "components/render_component.h"
#include "core/application.h"
#include "core/game_context.h"
#include "core/memory.h"
#include "render/d3d_util.h"
#include "render/graphics_device.h"

namespace Sapfire {

	GameContext::GameContext(const GameContextCreationDesc& desc) :
		m_ClientExtent(desc.client_extent),
		m_GraphicsDevice(stl::make_unique<d3d::GraphicsDevice>(
			mem::ENUM::Engine_Scene,
			d3d::SwapchainCreationDesc{static_cast<u32>(m_ClientExtent->width), static_cast<u32>(m_ClientExtent->height), 120,
									   d3d::MAX_FRAMES_IN_FLIGHT, DXGI_FORMAT_R16G16B16A16_FLOAT, desc.window_handle})),
		m_AssetManager(assets::AssetManagerCreationDesc{
			.device = m_GraphicsDevice.get(),
			.mesh_registry_path = desc.mesh_registry_path,
			.texture_registry_path = desc.texture_registry_path,
			.material_registry_path = desc.material_registry_path,
		}) {
		m_PhysicsEngine = stl::make_unique<physics::PhysicsEngine>(mem::ENUM::Engine_Scene, &m_ECManager);
	}

	void GameContext::init() { load_contents(); }

	void GameContext::create_render_component(Entity entity, const RenderComponentResourcePaths& resource_paths) {
		const bool already_has_component = m_ECManager.has_engine_component<components::RenderComponent>(entity);
		auto* mesh_asset =
			resource_paths.mesh_path.empty() ? assets::MeshRegistry::default_mesh() : m_AssetManager.get_mesh(resource_paths.mesh_path);
		auto* texture_asset = resource_paths.texture_path.empty() ? assets::TextureRegistry::default_texture(m_GraphicsDevice.get())
																  : m_AssetManager.get_texture(resource_paths.texture_path);
		auto* material_asset = resource_paths.material_path.empty() ? assets::MaterialRegistry::default_material(m_GraphicsDevice.get())
																	: m_AssetManager.get_material(resource_paths.material_path);
		if (!mesh_asset) {
			m_AssetManager.import_mesh(resource_paths.mesh_path);
			mesh_asset = m_AssetManager.get_mesh(resource_paths.mesh_path);
		}
		if (!texture_asset || !m_AssetManager.is_texture_loaded_for_runtime(texture_asset->uuid)) {
			m_AssetManager.import_texture(resource_paths.texture_path);
			texture_asset = m_AssetManager.get_texture(resource_paths.texture_path);
		}
		if (!material_asset || !m_AssetManager.material_resource_exists(material_asset->uuid)) {
			m_AssetManager.import_material(resource_paths.material_path);
			material_asset = m_AssetManager.get_material(resource_paths.material_path);
		}
		if (mesh_asset && mesh_asset->data.has_value()) {
			assert(mesh_asset->data->indices32.size() > 0);
			assert(mesh_asset->data->positions.size() > 0);
			assert(mesh_asset->data->normals.size() > 0);
			assert(mesh_asset->data->texcs.size() > 0);
			if (!already_has_component) {
				m_TransformBuffers.emplace_back(m_GraphicsDevice->create_buffer<ObjectConstants>({
					.usage = d3d::BufferUsage::ConstantBuffer,
					.name = L"Transform buffer " + d3d::AnsiToWString(resource_paths.mesh_path),
				}));
			}
			bool should_add_tangent = false;
			const bool should_allocate_mesh = !m_AssetManager.mesh_resource_exists(resource_paths.mesh_path);
			if (should_allocate_mesh) {
				const stl::wstring name = mesh_asset->uuid == assets::MeshRegistry::default_mesh()->uuid
					? L"Default Mesh"
					: d3d::AnsiToWString(resource_paths.mesh_path);
				m_RTIndexBuffers.push_back(m_GraphicsDevice->create_buffer<u16>(
					d3d::BufferCreationDesc{
						.usage = d3d::BufferUsage::IndexBuffer,
						.name = L"Index buffer " + name,
					},
					mesh_asset->data->indices16()));
				m_VertexPosBuffers.push_back(m_GraphicsDevice->create_buffer<DirectX::XMFLOAT3>(
					d3d::BufferCreationDesc{
						.usage = d3d::BufferUsage::StructuredBuffer,
						.name = L"Vertex Pos buffer " + name,
					},
					mesh_asset->data->positions));
				m_VertexNormalBuffers.push_back(m_GraphicsDevice->create_buffer<DirectX::XMFLOAT3>(
					d3d::BufferCreationDesc{
						.usage = d3d::BufferUsage::StructuredBuffer,
						.name = L"Vertex Norm buffer " + name,
					},
					mesh_asset->data->normals));
				if (mesh_asset->data->tangentus.size() > 0) {
					m_VertexTangentBuffers.push_back(m_GraphicsDevice->create_buffer<DirectX::XMFLOAT3>(
						d3d::BufferCreationDesc{
							.usage = d3d::BufferUsage::StructuredBuffer,
							.name = L"Vertex Tang buffer " + name,
						},
						mesh_asset->data->tangentus));
					should_add_tangent = true;
				}
				m_VertexUVBuffers.push_back(m_GraphicsDevice->create_buffer<DirectX::XMFLOAT2>(
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
			if (material_asset->uuid == assets::MaterialRegistry::default_material(m_GraphicsDevice.get())->uuid) {
				material_cbuffer_idx = material_asset->material.material_cb_index;
			} else {
				material_cbuffer_idx = m_AssetManager.material_resource_exists(resource_paths.material_path)
					? m_AssetManager.get_material_resource(resource_paths.material_path).gpu_idx
					: assets::MaterialRegistry::default_material(m_GraphicsDevice.get())->material.material_cb_index;
			}
			u32 texture_cbuffer_idx = 0;
			if (texture_asset->uuid == assets::TextureRegistry::default_texture(m_GraphicsDevice.get())->uuid) {
				texture_cbuffer_idx = texture_asset->data.srv_index;
			} else {
				texture_cbuffer_idx = m_AssetManager.texture_resource_exists(resource_paths.texture_path)
					? m_AssetManager.get_texture_resource(resource_paths.texture_path).gpu_idx
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
				cpu_data = m_AssetManager.get_mesh_resource(mesh_asset->uuid).cpu_data;
				gpu_data = m_AssetManager.get_mesh_resource(mesh_asset->uuid).gpu_data;
				gpu_data.scene_cbuffer_idx = already_has_component
					? m_ECManager.engine_component<components::RenderComponent>(entity).per_draw_constants()->scene_cbuffer_idx
					: m_TransformBuffers.back().cbv_index;
				gpu_data.material_cbuffer_idx = material_cbuffer_idx;
				gpu_data.texture_cbuffer_idx = texture_cbuffer_idx;
				cpu_data.transform_buffer_idx = already_has_component
					? m_ECManager.engine_component<components::RenderComponent>(entity).cpu_data().transform_buffer_idx
					: static_cast<u32>(m_TransformBuffers.size() - 1);
			}
			m_AssetManager.load_mesh_resource(resource_paths.mesh_path, {cpu_data, gpu_data});
			const components::RenderComponent render_component{
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
						const auto texture_path = m_AssetManager.get_texture_path(texture_uuid);
						const auto mesh_path = m_AssetManager.get_mesh_path(mesh_uuid);
						const auto material_uuid = component->material_uuid();
						const auto material_path = m_AssetManager.get_material_path(material_uuid);
						if (!m_AssetManager.mesh_resource_exists(mesh_path) || !m_AssetManager.material_resource_exists(material_path) ||
							!m_AssetManager.texture_resource_exists(texture_path)) {
							create_render_component(entity,
													{.mesh_path = mesh_path, .texture_path = texture_path, .material_path = material_path});
							return;
						}
						auto data = m_AssetManager.get_mesh_resource(mesh_path);
						data.gpu_data.scene_cbuffer_idx = old_gpu_data->scene_cbuffer_idx;
						component->cpu_data(data.cpu_data);
						component->per_draw_constants(data.gpu_data);
					}
				}};
			m_ECManager.add_engine_component<components::RenderComponent>(entity, render_component);
		}
	}

	void GameContext::on_window_resize() {
		if (m_GraphicsDevice)
			m_GraphicsDevice->resize_window(static_cast<u32>(m_ClientExtent->width), static_cast<u32>(m_ClientExtent->height));
	}

	void GameContext::update(f32 delta_time) {
		PROFILE_FUNCTION();
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
	}

} // namespace Sapfire
