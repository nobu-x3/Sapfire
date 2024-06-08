#pragma once

#include "render/d3d_primitives.h"

namespace Sapfire::tools {
	class OBJLoader {
	public:
		static stl::optional<d3d::primitives::MeshData> load_mesh(const stl::string& path);
	};
} // namespace Sapfire::tools
