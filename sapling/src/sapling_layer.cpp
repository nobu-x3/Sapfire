#include "sapling_layer.h"
#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"
#include "core/core.h"
#include "icons.h"
#include "imgui.h"
#include "subeditors/level_editor.h"
#include "subeditors/subeditor.h"
#include "tools/profiling.h"

using namespace Sapfire;

SaplingLayer::SaplingLayer() : Sapfire::Layer("Sapling Layer") {
	auto& app = Application::get();
	m_GraphicsDevice = stl::make_unique<d3d::GraphicsDevice>(
		Sapfire::mem::ENUM::Editor,
		d3d::SwapchainCreationDesc{static_cast<u32>(app.client_extent()->width), static_cast<u32>(app.client_extent()->height), 120,
								   d3d::MAX_FRAMES_IN_FLIGHT, DXGI_FORMAT_R16G16B16A16_FLOAT, app.window()->handle()});
	icons::add(*m_GraphicsDevice, L"editor_assets/icons/mesh_icon_64.png", icons::MESH_ICON_64_ID);
	icons::add(*m_GraphicsDevice, L"editor_assets/icons/mesh_icon_16.png", icons::MESH_ICON_16_ID);
}

void SaplingLayer::on_attach() {
	PROFILE_FUNCTION();
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // IF using Docking Branch
	io.WantCaptureMouse = false;
	ImGui::StyleColorsDark();
	// Setup Platform/Renderer backends
	auto& app = Application::get();
	ImGui_ImplWin32_Init(app.window()->handle());
	ImGui_ImplDX12_Init(m_GraphicsDevice->device(), d3d::MAX_FRAMES_IN_FLIGHT, m_GraphicsDevice->swapchain_back_buffer_format(),
						m_GraphicsDevice->dsv_descriptor_heap()->descriptor_heap(),
						m_GraphicsDevice->cbv_srv_uav_descriptor_heap()->current_descriptor_handle().cpu_descriptor_handle,
						m_GraphicsDevice->cbv_srv_uav_descriptor_heap()->current_descriptor_handle().gpu_descriptor_handle);
	m_DepthTexture = m_GraphicsDevice->create_texture({
		.usage = d3d::TextureUsage::DepthStencil,
		.width = static_cast<u32>(app.client_extent()->width),
		.height = static_cast<u32>(app.client_extent()->height),
		.format = DXGI_FORMAT_D32_FLOAT,
		.name = L"Depth Texture",
	});
	m_PipelineState = m_GraphicsDevice->create_pipeline_state({
		.shader_module =
			{
				.vertexShaderPath = L"bindless.hlsl",
				.vertexEntryPoint = L"VS",
				.pixelShaderPath = L"bindless.hlsl",
				.pixelEntryPoint = L"PS",
			},
		.pipeline_name = L"Sapling Layer Bindless Pipeline",
	});
	m_Subeditors.push_back(stl::make_unique<SLevelEditor>(Sapfire::mem::ENUM::Editor, m_GraphicsDevice.get()));
}

void SaplingLayer::on_detach() {
	PROFILE_FUNCTION();
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void SaplingLayer::on_update(Sapfire::f32 delta_time) {
	PROFILE_FUNCTION();
	// Wait for render to happen
	m_GraphicsDevice->direct_command_queue()->flush();
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::ShowDemoWindow();
	for (auto& subeditor : m_Subeditors) {
		subeditor->update(delta_time);
	}
}

void SaplingLayer::on_render() {
	PROFILE_FUNCTION();
	m_GraphicsDevice->begin_frame();
	auto& gfx_ctx = m_GraphicsDevice->current_graphics_contexts();
	for (auto&& editor : m_Subeditors) {
		editor->render(*gfx_ctx);
	}
	auto& current_backbuffer = m_GraphicsDevice->current_back_buffer();
	gfx_ctx->add_resource_barrier(current_backbuffer.allocation.resource.Get(), D3D12_RESOURCE_STATE_PRESENT,
								  D3D12_RESOURCE_STATE_RENDER_TARGET);
	gfx_ctx->execute_resource_barriers();
	static stl::array<f32, 4> clear_color{0.3f, 0.4f, 0.6f, 1.0f};
	gfx_ctx->clear_render_target_view(current_backbuffer, clear_color);
	gfx_ctx->clear_depth_stencil_view(m_DepthTexture);
	// TODO: setup barriers for all passes
	gfx_ctx->set_render_target(current_backbuffer, m_DepthTexture);
	gfx_ctx->set_viewport({
		.TopLeftX = 0.0f,
		.TopLeftY = 0.0f,
		.Width = static_cast<f32>(Application::get().client_extent()->width),
		.Height = static_cast<f32>(Application::get().client_extent()->height),
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f,
	});
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), gfx_ctx->command_list());
	gfx_ctx->add_resource_barrier(current_backbuffer.allocation.resource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
								  D3D12_RESOURCE_STATE_PRESENT);
	gfx_ctx->execute_resource_barriers();
	stl::array<const d3d::Context*, 1> contexts = {gfx_ctx.get()};
	m_GraphicsDevice->direct_command_queue()->execute_context(contexts);
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault(nullptr, (void*)gfx_ctx->command_list());
	}
	m_GraphicsDevice->present();
	m_GraphicsDevice->end_frame();
}

void SaplingLayer::on_event(Sapfire::Event& e) {
	PROFILE_FUNCTION();
	EventDispatcher dispatcher(e);
	dispatcher.dispatch<WindowResizeEvent>(BIND_EVENT_FN(SaplingLayer::on_window_resize));
	dispatcher.dispatch<WindowResizeFinishedEvent>(BIND_EVENT_FN(SaplingLayer::on_window_resize_finished));
}

bool SaplingLayer::on_window_resize_finished(WindowResizeFinishedEvent& e) {
	auto& app = Application::get();
	if (app.window() && !app.window()->is_minimized()) {
		if (m_GraphicsDevice)
			m_GraphicsDevice->resize_window(static_cast<u32>(app.client_extent()->width), static_cast<u32>(app.client_extent()->height));
		m_DepthTexture.allocation.reset();
		m_DepthTexture = m_GraphicsDevice->create_texture({
			.usage = d3d::TextureUsage::DepthStencil,
			.width = static_cast<u32>(app.client_extent()->width),
			.height = static_cast<u32>(app.client_extent()->height),
			.format = DXGI_FORMAT_D32_FLOAT,
			.name = L"Depth Texture",
		});
	}
	return true;
}

bool SaplingLayer::on_window_resize(WindowResizeEvent& e) {
	auto& app = Application::get();
	if (app.window() && !app.window()->is_minimized() && !app.window()->is_resizing()) {
		if (m_GraphicsDevice)
			m_GraphicsDevice->resize_window(static_cast<u32>(e.width()), static_cast<u32>(e.height()));
		m_DepthTexture.allocation.reset();
		m_DepthTexture = m_GraphicsDevice->create_texture({
			.usage = d3d::TextureUsage::DepthStencil,
			.width = static_cast<u32>(e.width()),
			.height = static_cast<u32>(e.height()),
			.format = DXGI_FORMAT_D32_FLOAT,
			.name = L"Depth Texture",
		});
	}
	return true;
}
