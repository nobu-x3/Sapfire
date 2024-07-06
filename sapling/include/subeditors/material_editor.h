#pragma once

#include "subeditor.h"

class SMaterialEditor final : public SSubeditor {
public:
	SMaterialEditor(Sapfire::assets::AssetManager* am);
	void draw_menu() override;

private:
	Sapfire::assets::AssetManager& m_AssetManager;
	Sapfire::d3d::Material* m_OpenedMaterial{nullptr};
};
