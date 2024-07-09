#pragma once

#include "Sapfire.h"

#include "core/stl/shared_ptr.h"
#include "widgets/widget.h"

namespace widgets {

	class SSceneView final : public IWidget {
	public:
		explicit SSceneView(Sapfire::ECManager* ec_manager, Sapfire::d3d::GraphicsDevice* gfx_device);
		void add_render_component(Sapfire::Entity entity, const Sapfire::RenderComponentResourcePaths& resource_paths);
		bool update(Sapfire::f32 delta_time) override;
		void render(Sapfire::d3d::GraphicsContext& gfx_ctx) override;

		static Sapfire::stl::shared_ptr<SSceneView> scene_view();

	private:
		void update_pass_cb(Sapfire::f32 delta_time);
		void update_materials();
		void update_transform_buffer();

	private:
		Sapfire::ECManager& m_ECManager;
		Sapfire::d3d::GraphicsDevice& m_GraphicsDevice;
		Sapfire::stl::unique_ptr<Sapfire::physics::PhysicsEngine> m_PhysicsEngine;
		Sapfire::stl::vector<Sapfire::d3d::Buffer> m_RTIndexBuffers{};
		Sapfire::stl::vector<Sapfire::d3d::Buffer> m_VertexPosBuffers{};
		Sapfire::stl::vector<Sapfire::d3d::Buffer> m_VertexNormalBuffers{};
		Sapfire::stl::vector<Sapfire::d3d::Buffer> m_VertexTangentBuffers{};
		Sapfire::stl::vector<Sapfire::d3d::Buffer> m_VertexUVBuffers{};
		Sapfire::stl::vector<Sapfire::d3d::Buffer> m_TransformBuffers{};
		Sapfire::stl::vector<Sapfire::d3d::Texture> m_OffscreenTextures;
		Sapfire::d3d::PipelineState m_PipelineState{};
		Sapfire::d3d::Texture m_DepthTexture;
		Sapfire::d3d::Buffer m_MainPassCB{};
		Sapfire::Camera m_MainCamera;
		bool m_Resizing{false};

		static Sapfire::stl::shared_ptr<SSceneView> s_Instance;
	};
} // namespace widgets
