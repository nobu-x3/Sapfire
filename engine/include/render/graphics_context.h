#pragma once

#include "d3dcommon.h"
#include "render/context.h"
#include "render/resources.h"

namespace Sapfire::d3d {
	class DescriptorHeap;
	class GraphicsDevice;
    class PipelineState;

	// Wrapper for graphics CommandList, which provides functions to record commands for execution by GPU.
	// It's of type Direct, so issues all commands (copy, compute, rendering)
	class GraphicsContext : public Context {
	public:
		explicit GraphicsContext(GraphicsDevice* device);
		void reset_context() override;
		void clear_render_target_view(const Texture& render_target, const stl::span<const f32, 4> color);
		void clear_render_target_view(const stl::span<Texture> render_targets, const stl::span<const f32, 4> color);
		void clear_render_target_view(const Texture& texture, f32 color);
		void clear_depth_stencil_view(const Texture& texture);
		void set_descriptor_heaps() const;

		// pipeline configuration
		void set_graphics_pipeline_state(const PipelineState& state) const;
		void set_graphics_root_signature() const;
		void set_graphics_root_signature_and_pipeline(const PipelineState& state) const;
		void set_index_buffer(const Buffer& buffer) const;
		void set_32_bit_graphics_constants(const void* render_resources) const;
		// TODO: compute stuff

		void set_viewport(const D3D12_VIEWPORT& viewport) const;
		void set_primitive_topology_layout(const D3D_PRIMITIVE_TOPOLOGY topology) const;
		void set_render_target(const Texture& target) const;
		void set_render_target(const Texture& target, const Texture& depth_stencil_texture) const;
		void set_render_target(stl::span<Texture> targets, const Texture& depth_stencil_texture) const;

		void copy_resource(ID3D12Resource* source, ID3D12Resource* destination) const;

		void draw_instance_indexed(u32 index_count, u32 instance_count = 1) const;
		void draw_indexed(u32 index_count, u32 instance_count = 1) const;

		// TODO: dispatch function

	private:
		GraphicsDevice& m_GraphicsDevice;
	};
} // namespace Sapfire::d3d
