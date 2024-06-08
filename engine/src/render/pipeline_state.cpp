#include "d3dx12_barriers.h"
#include "d3dx12_core.h"
#include "engpch.h"

#include "core/file_system.h"
#include "d3d12.h"
#include "dxgiformat.h"
#include "render/d3d_util.h"
#include "render/pipeline_state.h"
#include "render/resources.h"
#include "tools/shader_compiler.h"

namespace Sapfire::d3d {
	PipelineState::PipelineState(ID3D12Device* device, const GraphicsPipelineStateCreationDesc& desc) {
		PROFILE_FUNCTION();
		// Blending not used for now but is setup in case it's needed
		constexpr D3D12_RENDER_TARGET_BLEND_DESC render_target_blend_desc{
			.BlendEnable = FALSE,
			.LogicOpEnable = FALSE,
			.SrcBlend = D3D12_BLEND_SRC_ALPHA,
			.DestBlend = D3D12_BLEND_INV_SRC_ALPHA,
			.BlendOp = D3D12_BLEND_OP_ADD,
			.SrcBlendAlpha = D3D12_BLEND_ONE,
			.DestBlendAlpha = D3D12_BLEND_ZERO,
			.BlendOpAlpha = D3D12_BLEND_OP_ADD,
			.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
		};
		D3D12_BLEND_DESC blend_desc{
			.AlphaToCoverageEnable = FALSE,
			.IndependentBlendEnable = FALSE,
		};
		for (u32 i = 0; i < desc.rtv_count; ++i) {
			blend_desc.RenderTarget[i] = render_target_blend_desc;
		}
		const D3D12_DEPTH_STENCIL_DESC depth_stencil_desc{
			.DepthEnable = desc.depth_format == DXGI_FORMAT_UNKNOWN ? FALSE : TRUE,
			.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
			.DepthFunc = desc.depth_comparison_func,
			.StencilEnable = FALSE,
			.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK,
			.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
		};
		const auto& vertex_shader_blob =
			tools::shader_compiler::compile(ShaderType::Vertex, desc.shader_module.vertexShaderPath, desc.shader_module.vertexEntryPoint)
				.shader_blob;
		const auto& pixel_shader_blob =
			tools::shader_compiler::compile(ShaderType::Pixel, desc.shader_module.pixelShaderPath, desc.shader_module.pixelEntryPoint)
				.shader_blob;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc{
			.pRootSignature = PipelineState::s_RootSignature.Get(),
			.VS = CD3DX12_SHADER_BYTECODE(vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize()),
			.PS = CD3DX12_SHADER_BYTECODE(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize()),
			.BlendState = blend_desc,
			.SampleMask = UINT32_MAX,
			.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
			.DepthStencilState = depth_stencil_desc,
			.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
			.NumRenderTargets = desc.rtv_count,
			.DSVFormat = desc.depth_format,
			.SampleDesc{.Count = 1, .Quality = 0},
			.NodeMask = 0,
		};
		if (desc.front_face_winding_order == FrontFaceWindingOrder::CounterClockwise) {
			pso_desc.RasterizerState.FrontCounterClockwise = true;
		}
		pso_desc.RasterizerState.CullMode = desc.cull_mode;
		for (u32 i = 0; i < desc.rtv_count; ++i) {
			pso_desc.RTVFormats[i] = desc.rtv_formats[i];
		}
		d3d_check(device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&m_PipelineStateObject)));
		m_PipelineStateObject->SetName(desc.pipeline_name.data());
	}

	PipelineState::PipelineState(ID3D12Device* device, const ComputePipelineStateCreationDesc& creation_desc) {
		PROFILE_FUNCTION();
		const auto& cs_blob = tools::shader_compiler::compile(ShaderType::Compute, creation_desc.shader_path, L"CS").shader_blob;
		const D3D12_COMPUTE_PIPELINE_STATE_DESC pso_desc{
			.pRootSignature = s_RootSignature.Get(),
			.CS =
				{
					.pShaderBytecode = cs_blob->GetBufferPointer(),
					.BytecodeLength = cs_blob->GetBufferSize(),
				},
			.NodeMask = 0,
		};
		d3d_check(device->CreateComputePipelineState(&pso_desc, IID_PPV_ARGS(&m_PipelineStateObject)));
		m_PipelineStateObject->SetName(creation_desc.pipeline_name.data());
	}

	void PipelineState::create_bindless_root_signature(ID3D12Device* device, stl::wstring_view shader_name) {
		PROFILE_FUNCTION();
		const auto path = shader_name;
		const auto shader = tools::shader_compiler::compile(ShaderType::Vertex, path, L"VS", true);
		if (!shader.root_signature_blob.Get()) {
			CORE_ERROR("Shader {} not found", WStringToANSI(path));
		}
		d3d_check(device->CreateRootSignature(0, shader.root_signature_blob->GetBufferPointer(),
											  shader.root_signature_blob->GetBufferSize(), IID_PPV_ARGS(&s_RootSignature)));
		s_RootSignature->SetName(L"Bindless Root Signature");
	}

} // namespace Sapfire::d3d
