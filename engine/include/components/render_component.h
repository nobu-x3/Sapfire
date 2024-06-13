#pragma once

#include "components/component.h"

namespace Sapfire {
	namespace d3d {
		class GraphicsDevice;
	}
	namespace assets {
		class MeshRegistry;
	}
} // namespace Sapfire

namespace Sapfire::components {

	struct PerDrawConstants {
		Sapfire::u32 position_buffer_idx = 0;
		Sapfire::u32 normal_buffer_idx = 0;
		Sapfire::u32 tangent_buffer_idx = 0;
		Sapfire::u32 uv_buffer_idx = 0;
		Sapfire::u32 scene_cbuffer_idx = 0;
		Sapfire::u32 pass_cbuffer_idx = 0;
		Sapfire::u32 material_cbuffer_idx = 0;
		Sapfire::u32 texture_cbuffer_idx = 0;
	};

	struct CPUData {
		Sapfire::u32 indices_size = 0;
		Sapfire::u32 index_id = 0;
		Sapfire::u32 position_idx = 0;
		Sapfire::u32 normal_idx = 0;
		Sapfire::u32 tangent_idx = 0;
		Sapfire::u32 uv_idx = 0;
	};

	class RenderComponent {
		RTTI;
		ENGINE_COMPONENT(RenderComponent);

	public:
		RenderComponent();
		RenderComponent(const RenderComponent& other);
		RenderComponent(RenderComponent&& other);
		RenderComponent& operator=(const RenderComponent& other);
		RenderComponent& operator=(RenderComponent&& other);
		RenderComponent(UUID mesh_uuid, UUID texture_uuid, CPUData cpu_data, PerDrawConstants per_draw_constants,
						stl::function<void(RenderComponent*)> = nullptr);
		void register_rtti();

		inline PerDrawConstants* per_draw_constants() { return &m_PerDrawConstants; }
		inline void per_draw_constants(const PerDrawConstants& constants) { m_PerDrawConstants = constants; }
		inline CPUData cpu_data() const { return m_CPUData; }
		inline void cpu_data(CPUData data) { m_CPUData = data; }
		inline UUID mesh_uuid() const { return m_MeshUUID; }

	private:
		UUID m_MeshUUID;
		UUID m_MaterialUUID;
		UUID m_TextureUUID;
		CPUData m_CPUData{};
		PerDrawConstants m_PerDrawConstants{};
		std::function<void(RenderComponent*)> m_OptionalSetter = nullptr;
	};
} // namespace Sapfire::components
