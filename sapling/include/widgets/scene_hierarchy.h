#pragma once

#include "components/entity.h"
#include "core/core.h"
#include "widgets/widget.h"

namespace Sapfire {
	class ECManager;
}

namespace widgets {

	using EntitySelectedCallback = Sapfire::stl::function<void(Sapfire::stl::optional<Sapfire::Entity>)>;

	class SSceneHierarchy final : public IWidget {
	public:
		SSceneHierarchy(Sapfire::ECManager* ec_manager, EntitySelectedCallback callback);
		bool update(Sapfire::f32 delta_time) override;

		void on_mouse_button_event(Sapfire::MouseButtonEvent&) override;

	private:
		void build_tree_for_entity(const Sapfire::stl::vector<Sapfire::stl::generational_index>& indices, Sapfire::u32 valid_parent_index);

	private:
		Sapfire::ECManager& m_ECManager;
		bool m_ShowContextMenu{false};
		bool m_SelectEntity{false};
		EntitySelectedCallback m_EntitySelectedCallback;
		Sapfire::stl::optional<Sapfire::Entity> m_SelectedEntity{};
	};
} // namespace widgets
