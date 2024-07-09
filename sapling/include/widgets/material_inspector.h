#pragma once

#include "Sapfire.h"
#include "widget.h"

namespace widgets {
	class SMaterialInspector final : public IWidget {
	public:
		bool update(Sapfire::f32 delta_time) override;
		inline void current_material(Sapfire::assets::MaterialAsset* mat) { m_CurrentMaterial = mat; }

	private:
		Sapfire::assets::MaterialAsset* m_CurrentMaterial{nullptr};
	};
} // namespace widgets