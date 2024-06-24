#pragma once

#include "Sapfire.h"

class IWidget;

class SSubeditor {
public:
	SSubeditor(Sapfire::stl::string_view editor_name);
	virtual ~SSubeditor() = default;
	virtual bool update(Sapfire::f32 delta_time);
	virtual void render(Sapfire::d3d::GraphicsContext&);
	virtual void draw_menu(){};
	inline const Sapfire::stl::string& name() const { return m_Name; }

protected:
	Sapfire::stl::vector<Sapfire::stl::unique_ptr<IWidget>> m_Widgets;
	Sapfire::stl::string m_Name;
};
