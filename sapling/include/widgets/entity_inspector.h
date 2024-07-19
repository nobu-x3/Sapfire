#pragma once

#include "components/entity.h"
#include "core/core.h"
#include "widgets/widget.h"

namespace Sapfire {
	class ECManager;
}

namespace widgets {
	using AddRenderComponentCallback = Sapfire::stl::function<void(Sapfire::Entity, const Sapfire::RenderComponentResourcePaths&)>;

	class SEntityInspector final : public IWidget {
	public:
		SEntityInspector(Sapfire::ECManager* ec_manager, AddRenderComponentCallback callback);
		bool update(Sapfire::f32 delta_time) override;
		void select_entity(Sapfire::stl::optional<Sapfire::Entity> maybe_entity);

	private:
		Sapfire::ECManager& m_ECManager;
		Sapfire::stl::optional<Sapfire::Entity> m_SelectedEntity{};
		AddRenderComponentCallback m_AddRenderComponentCallback;
		bool m_ShowAddComponentContextMenu{false};
	};
} // namespace widgets
