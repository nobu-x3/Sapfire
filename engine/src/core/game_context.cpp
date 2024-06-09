#include "engpch.h"

#include "components/render_component.h"
#include "core/application.h"
#include "core/game_context.h"
#include "core/memory.h"
#include "render/d3d_util.h"
#include "render/graphics_device.h"

namespace Sapfire {

	GameContext::GameContext(const GameContextCreationDesc& desc) :
		m_MeshRegistry(desc.mesh_registry_path), m_TextureRegistry(desc.texture_registry_path) {
		m_ClientExtent = desc.client_extent;
		m_PhysicsEngine = stl::make_unique<physics::PhysicsEngine>(mem::ENUM::Engine_Scene, &m_ECManager);
		m_GraphicsDevice = stl::make_unique<d3d::GraphicsDevice>(
			mem::ENUM::Engine_Scene,
			d3d::SwapchainCreationDesc{static_cast<u32>(m_ClientExtent->width), static_cast<u32>(m_ClientExtent->height), 120,
									   d3d::MAX_FRAMES_IN_FLIGHT, DXGI_FORMAT_R16G16B16A16_FLOAT, desc.window_handle});
	}

	void GameContext::init() { load_contents(); }

	void GameContext::add_texture(const stl::string& texture_path) {
		auto* texture = m_TextureRegistry.get(texture_path);
		if (!texture) {
			m_TextureRegistry.import_texture(*m_GraphicsDevice, texture_path);
			texture = m_TextureRegistry.get(texture_path);
		}
<<<<<<< HEAD
		if (texture->data.dsv_index == d3d::INVALID_INDEX_U32 || texture->data.srv_index == d3d::INVALID_INDEX_U32) {
			texture->data = m_GraphicsDevice->create_texture(d3d::TextureCreationDesc{
				.usage = d3d::TextureUsage::TextureFromPath,
				.name = d3d::AnsiToWString(texture_path),
				.path = d3d::AnsiToWString(texture_path),
			});
		}
=======
>>>>>>> 1a9c437 (Texture registry serialization-deserialization)
		if (texture) {
			assets::TextureResource text_res{
				.gpu_idx = texture->data.srv_index,
			};
			m_TextureManager.add(texture_path, texture->uuid, text_res);
		}
	}

	void GameContext::create_render_component(Entity entity, const stl::string& mesh_path, const stl::string& texture_path) {
		// @TODO: add materials and textures
		auto* mesh_asset = m_MeshRegistry.get(mesh_path);
		auto* texture_asset = m_TextureRegistry.get(texture_path);
		if (!mesh_asset) {
			m_MeshRegistry.import_mesh(mesh_path);
			mesh_asset = m_MeshRegistry.get(mesh_path);
		}
		if (!texture_asset) {
			m_TextureRegistry.import_texture(*m_GraphicsDevice, texture_path);
			texture_asset = m_TextureRegistry.get(texture_path);
		}
		if (mesh_asset && texture_asset && mesh_asset->data.has_value()) {
			assert(mesh_asset->data->indices32.size() > 0);
			assert(mesh_asset->data->positions.size() > 0);
			assert(mesh_asset->data->normals.size() > 0);
			assert(mesh_asset->data->texcs.size() > 0);
			m_RTIndexBuffers.push_back(m_GraphicsDevice->create_buffer<u16>(
				d3d::BufferCreationDesc{
					.usage = d3d::BufferUsage::IndexBuffer,
				},
				mesh_asset->data->indices16()));
			m_VertexPosBuffers.push_back(m_GraphicsDevice->create_buffer<DirectX::XMFLOAT3>(
				d3d::BufferCreationDesc{
					.usage = d3d::BufferUsage::StructuredBuffer,
				},
				mesh_asset->data->positions));
			m_VertexNormalBuffers.push_back(m_GraphicsDevice->create_buffer<DirectX::XMFLOAT3>(
				d3d::BufferCreationDesc{
					.usage = d3d::BufferUsage::StructuredBuffer,
				},
				mesh_asset->data->normals));
			bool should_add_tangent = false;
			if (mesh_asset->data->tangentus.size() > 0) {
				m_VertexTangentBuffers.push_back(m_GraphicsDevice->create_buffer<DirectX::XMFLOAT3>(
					d3d::BufferCreationDesc{
						.usage = d3d::BufferUsage::StructuredBuffer,
					},
					mesh_asset->data->tangentus));
				should_add_tangent = true;
			}
			m_VertexUVBuffers.push_back(m_GraphicsDevice->create_buffer<DirectX::XMFLOAT2>(
				d3d::BufferCreationDesc{
					.usage = d3d::BufferUsage::StructuredBuffer,
				},
				mesh_asset->data->texcs));
			components::RenderComponent render_component{
				mesh_asset->uuid,
				texture_asset->uuid,
				components::CPUData{
					.indices_size = static_cast<u32>(mesh_asset->data->indices32.size()),
					.index_id = static_cast<u32>(m_RTIndexBuffers.size() - 1),
					.position_idx = static_cast<u32>(m_VertexPosBuffers.size() - 1),
					.normal_idx = static_cast<u32>(m_VertexNormalBuffers.size() - 1),
					.tangent_idx = static_cast<u32>(should_add_tangent ? m_VertexTangentBuffers.size() - 1 : 0),
					.uv_idx = static_cast<u32>(m_VertexUVBuffers.size() - 1),
				},
				components::PerDrawConstants{
					.position_buffer_idx = m_VertexPosBuffers.back().srv_index,
					.normal_buffer_idx = m_VertexNormalBuffers.back().srv_index,
					.tangent_buffer_idx = should_add_tangent ? m_VertexTangentBuffers.back().srv_index : 0,
					.uv_buffer_idx = m_VertexUVBuffers.back().srv_index,
					.scene_cbuffer_idx = m_TransformBuffers.back().cbv_index,
					.pass_cbuffer_idx = m_MainPassCB.cbv_index,
					.material_cbuffer_idx = m_Materials[0].material_buffer.cbv_index,
					.texture_cbuffer_idx = m_TextureManager.texture_resources.contains(texture_path)
						? m_TextureManager.texture_resources[texture_path].gpu_idx
						: 0,
				},
			};
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
