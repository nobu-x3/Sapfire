#pragma once

#include "Sapfire.h"
#include "core/game_context.h"

class SandboxGameContext final : public Sapfire::GameContext {
public:
	SandboxGameContext(const Sapfire::GameContextCreationDesc& desc);
	void load_contents() override;
	void render() override;
	void update(Sapfire::f32 delta_time) override;
	void resize_depth_texture();

private:
	void update_pass_cb(Sapfire::f32 delta_time);
	void update_materials(Sapfire::f32 delta_time);
	void udpate_transform_buffer(Sapfire::f32 delta_time);

private:
	Sapfire::d3d::Texture m_DepthTexture{};
	Sapfire::d3d::PipelineState m_PipelineState{};
};
