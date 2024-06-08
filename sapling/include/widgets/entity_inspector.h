#pragma once

#include "components/entity.h"
#include "core/core.h"
#include "widgets/widget.h"

namespace Sapfire {
	class ECManager;
}

namespace widgets {
	class SEntityInspector final : public IWidget {
	public:
		SEntityInspector(Sapfire::ECManager* ec_manager);
		bool update(Sapfire::f32 delta_time) override;
		void select_entity(Sapfire::stl::optional<Sapfire::Entity> maybe_entity);

	private:
		Sapfire::ECManager& m_ECManager;
		Sapfire::stl::optional<Sapfire::Entity> m_SelectedEntity{};
		bool m_ShowAddComponentContextMenu{false};
	};
} // namespace widgets
