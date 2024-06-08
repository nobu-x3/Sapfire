#pragma once

#include "core/core.h"
#include "render/d3d_primitives.h"

namespace Sapfire::d3d {
    class MeshManager {
        public:
            bool import_mesh();

        private:
            stl::unordered_map<UUID, primitives::MeshData> m_NameToMeshDataMap{};

    };
}
