#pragma once

#include "Sapfire.h"
#include "subeditors/subeditor.h"

namespace widgets {
	class SSceneHierarchy;
}

namespace ESubeditor {
	enum TYPE {
		LevelEditor = 0,
		MaterialEditor = 1,
		COUNT,
	};
}

static Sapfire::stl::array<Sapfire::stl::string, ESubeditor::COUNT> g_SubeditorNames = {"Level Editor", "Material Editor"};

class SaplingLayer final : public Sapfire::Layer {
public:
	SaplingLayer();
	~SaplingLayer() final = default;
	void on_attach() final;
	void on_detach() final;
	void on_update(Sapfire::f32 delta_time) final;
	void on_event(Sapfire::Event& e) final;
	void on_render() final;

	Sapfire::assets::AssetManager* asset_manager() { return m_AssetManager.get(); }

private:
	void serialize();
	void draw_menu_bar();
	void update_pass_cb(Sapfire::f32 delta_time);
	bool on_window_resize_finished(Sapfire::WindowResizeFinishedEvent&);
	bool on_window_resize(Sapfire::WindowResizeEvent& e);
	bool is_subeditor_active(ESubeditor::TYPE type);
	SSubeditor* subeditor_factory(ESubeditor::TYPE type, bool is_callback = false);
	Sapfire::stl::bitset<ESubeditor::COUNT> m_ShouldExecuteSubeditorCreationCallback{};

private:
	Sapfire::stl::array<Sapfire::stl::unique_ptr<SSubeditor>, 2> m_Subeditors{};
	Sapfire::stl::unique_ptr<Sapfire::d3d::GraphicsDevice> m_GraphicsDevice{};
	Sapfire::stl::unique_ptr<Sapfire::assets::AssetManager> m_AssetManager{};
	Sapfire::d3d::PipelineState m_PipelineState{};
	Sapfire::d3d::Texture m_DepthTexture{};
	Sapfire::u8 m_ActiveSubeditors{0};
	Sapfire::stl::string m_ProjectPath;
	Sapfire::stl::string m_ProjectName;
};