#include "Sapfire.h"

#include "core/application.h"
#include "core/entry.h"
#include "sandbox_game_context.h"
#include "sandbox_layer.h"

using namespace Sapfire;

class Sandbox final : public Sapfire::Application {
public:
	Sandbox(const ApplicationCreationDesc& desc) :
		Sapfire::Application(desc), m_GameContext({
										.client_extent = &m_ClientExtent,
										.window_handle = m_Window->handle(),
									}) {
		push_layer(new SandboxLayer);
		m_GameContext.init();
	}

	~Sandbox() {}

protected:
	void event_loop(f32 delta_time) override;
	bool on_window_resize_finished(WindowResizeFinishedEvent& e) override;
	bool on_window_resize(WindowResizeEvent& e) override;

private:
	SandboxGameContext m_GameContext;
};

Sapfire::Application* Sapfire::create_application() {
	return new Sandbox({
		.name = "Sapfire - Sandbox",
		.width = 800,
		.height = 600,
	});
}

void Sandbox::event_loop(f32 delta_time) {
	m_GameContext.update(delta_time);
	m_GameContext.render();
}

bool Sandbox::on_window_resize_finished(WindowResizeFinishedEvent& e) {
	bool return_val = Application::on_window_resize_finished(e);
	if (!m_Window->is_minimized())
		m_GameContext.on_window_resize();
	m_GameContext.resize_depth_texture();
	return return_val;
}

bool Sandbox::on_window_resize(WindowResizeEvent& e) {
	bool return_val = Application::on_window_resize(e);
	if (m_Window && !m_Window->is_minimized() && !m_Window->is_resizing())
		m_GameContext.on_window_resize();
	if (!m_Window->is_resizing() && !m_Window->is_minimized()) {
		m_GameContext.resize_depth_texture();
	}
	return return_val;
}
