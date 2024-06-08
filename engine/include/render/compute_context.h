#pragma once

#include "render/context.h"

namespace Sapfire::d3d {

	class GraphicsDevice;
	class DescriptorHeap;
	class PipelineState;

	// Wrapper for Compute command list
	class ComputeContext : public Context {
	public:
		explicit ComputeContext(GraphicsDevice* device);
		void reset_context() override;
		void dispatch(u32 thread_group_x, u32 thread_group_y, u32 thread_group_z) const;
		void set_descriptor_heaps(stl::span<const DescriptorHeap*> shader_visible_descriptor_heaps) const;
		void set_compute_root_signature_and_pipeline(const PipelineState&) const;
		void set_32bit_compute_constants(const void* render_resources) const;

	private:
		GraphicsDevice& m_GraphicsDevice;
	};
} // namespace Sapfire::d3d
