#pragma once

#include "Sapfire.h"
#include "core/game_context.h"

struct PassConstants {
	DirectX::XMFLOAT4X4 view = Sapfire::math::Identity4x4();
	DirectX::XMFLOAT4X4 inv_view = Sapfire::math::Identity4x4();
	DirectX::XMFLOAT4X4 proj = Sapfire::math::Identity4x4();
	DirectX::XMFLOAT4X4 inv_proj = Sapfire::math::Identity4x4();
	DirectX::XMFLOAT4X4 view_proj = Sapfire::math::Identity4x4();
	DirectX::XMFLOAT4X4 inv_view_proj = Sapfire::math::Identity4x4();
	DirectX::XMFLOAT3 EyePosW = {0.0f, 0.0f, 0.0f};
	float cbPerObjectPad1 = 0.0f;
	DirectX::XMFLOAT2 render_target_size = {0.0f, 0.0f};
	DirectX::XMFLOAT2 inv_render_target_size = {0.0f, 0.0f};
	float near_z = 0.0f;
	float far_z = 0.0f;
	float total_time = 0.0f;
	float delta_time = 0.0f;
	DirectX::XMFLOAT4 ambient_light = {0.0f, 0.0f, 0.0f, 1.0f};
	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
	// are spot lights for a maximum of MaxLights per object.
	Sapfire::d3d::Light Lights[MaxLights];
};

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
    PassConstants m_PassConstants{};
};
