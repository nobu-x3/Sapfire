#pragma once

#include "Sapfire.h"

class SandboxLayer final : public Sapfire::Layer {
public:
	SandboxLayer();
	~SandboxLayer() final = default;
	void on_attach() final;
	void on_detach() final;
	void on_update(Sapfire::f32 delta_time) final;
	void on_event(Sapfire::Event& e) final;
};
