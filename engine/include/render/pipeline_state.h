#pragma once

#include "core/core.h"
#include "render/resources.h"

namespace Sapfire::d3d {

	// Wrapper for pipeline state and root signature.
	// Because of bindless rendering, root signature is static because both graphics and compute pipelines have a common root signature.
	class SFAPI PipelineState {
	public:
		explicit PipelineState() = default;
		PipelineState(ID3D12Device* device, const GraphicsPipelineStateCreationDesc& creation_desc);
		PipelineState(ID3D12Device* device, const ComputePipelineStateCreationDesc& creation_desc);
		ID3D12PipelineState* pso() const { return m_PipelineStateObject.Get(); }
		// TODO: compute
		static void create_bindless_root_signature(ID3D12Device* device, stl::wstring_view shader_name);
		static inline ID3D12RootSignature* root_signature() { return s_RootSignature.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineStateObject{};
		static inline Microsoft::WRL::ComPtr<ID3D12RootSignature> s_RootSignature{};
	};
} // namespace Sapfire::d3d
