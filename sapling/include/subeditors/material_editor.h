#pragma once

#include "Sapfire.h"
#include "subeditor.h"

namespace EWidgetOrder {
	enum ENUM {
		MaterialInspector = 0,
	};
}

class SMaterialEditor final : public SSubeditor {
public:
	SMaterialEditor(Sapfire::assets::AssetManager* am, Sapfire::d3d::GraphicsDevice* device);
	bool update(Sapfire::f32 delta_time) override;
	void draw_menu() override;
	void draw_open_material_dialog();

private:
	Sapfire::assets::AssetManager& m_AssetManager;
	Sapfire::assets::MaterialAsset* m_OpenedMaterial{nullptr};
	Sapfire::stl::unique_ptr<Sapfire::ECManager> m_ECManager{nullptr};
	Sapfire::Entity m_MaterialEntity;
};
