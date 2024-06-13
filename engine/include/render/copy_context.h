#pragma once

#include <winbase.h>
#include "render/context.h"

namespace Sapfire::d3d {

	class GraphicsDevice;

	// Wrapper for Copy CommandList
	class SFAPI CopyContext : public Context {
	public:
		explicit CopyContext(GraphicsDevice* device);
	};
} // namespace Sapfire::d3d
