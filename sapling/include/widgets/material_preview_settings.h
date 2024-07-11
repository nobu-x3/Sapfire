#pragma once

#include "Sapfire.h"
#include "widget.h"

class SMaterialPreviewSettings final : public IWidget {
public:
	explicit SMaterialPreviewSettings(Sapfire::components::RenderComponent* rc);
	bool update(Sapfire::f32 delta_time) override;

private:
	Sapfire::components::RenderComponent& m_RenderComponent;

};