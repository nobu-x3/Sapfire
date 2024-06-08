#pragma once

#include <DirectXMath.h>
#include "core/core.h"

namespace Sapfire {
    class ECManager;
}

namespace Sapfire::physics {

	class SFAPI PhysicsEngine {
	public:
        PhysicsEngine(ECManager* ec_manager);
		void simulate(f32 delta_time);

	private:
        ECManager& m_ECManager;
	};
} // namespace Sapfire::physics
