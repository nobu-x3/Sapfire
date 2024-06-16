#pragma once

#include "Sapfire.h"
#include "widgets/widget.h"

namespace widgets {
	enum class AssetType : Sapfire::u8 { Mesh, Texture, Material };

	struct AssetDragAndDropPayload {
		Sapfire::UUID uuid;
		AssetType type;
	};

	class AssetBrowser final : public IWidget {
	public:
		bool update(Sapfire::f32 delta_time) override;
	};
} // namespace widgets
