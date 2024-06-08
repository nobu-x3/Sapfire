#include "engpch.h"

#include <combaseapi.h>
#include "d3d12.h"
#include "render/copy_context.h"
#include "render/d3d_util.h"
#include "render/graphics_device.h"

namespace Sapfire::d3d {
	CopyContext::CopyContext(GraphicsDevice* device) : m_GraphicsDevice(*device) {
		d3d_check(device->device()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&m_CommandAllocator)));
		d3d_check(device->device()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_CommandAllocator.Get(), nullptr,
													  IID_PPV_ARGS(&m_CommandList)));
		d3d_check(m_CommandList->Close());
	}
} // namespace Sapfire::d3d
