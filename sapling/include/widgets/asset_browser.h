#pragma once

#include "Sapfire.h"
#include "widgets/widget.h"

namespace widgets {
	enum class EAssetType : Sapfire::u8 { Unknown, Mesh, Texture, Material };

	using event_fn = Sapfire::stl::function<void()>;

	struct AssetDragAndDropPayload {
		Sapfire::UUID uuid;
		EAssetType type;
	};

	class SAssetBrowser final : public IWidget {
	public:
		static void register_asset_imported_events(event_fn fn);
		bool update(Sapfire::f32 delta_time) override;
		void on_mouse_button_event(Sapfire::MouseButtonEvent&) override;

	private:
		void execute_asset_imported_events();

	private:
		EAssetType m_CurrentAssetTypeFilter{EAssetType::Mesh};
		bool m_ShowContextMenu{false};
	};
} // namespace widgets
