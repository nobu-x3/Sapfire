#include "sapling_layer.h"
#include "ImGuiFileDialog.h"
#include "assets/project_reader.h"
#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"
#include "core/core.h"
#include "icons.h"
#include "imgui.h"
#include "subeditors/level_editor.h"
#include "tools/profiling.h"

using namespace Sapfire;

SaplingLayer::SaplingLayer() : Sapfire::Layer("Sapling Layer") {
	auto& app = Application::get();
	m_GraphicsDevice = stl::make_unique<d3d::GraphicsDevice>(
		Sapfire::mem::ENUM::Editor,
		d3d::SwapchainCreationDesc{static_cast<u32>(app.client_extent()->width), static_cast<u32>(app.client_extent()->height), 120,
								   d3d::MAX_FRAMES_IN_FLIGHT, DXGI_FORMAT_R16G16B16A16_FLOAT, app.window()->handle()});
	m_AssetManager = Sapfire::stl::make_unique<Sapfire::assets::AssetManager>(mem::Editor,
																			  Sapfire::assets::AssetManagerCreationDesc{
																				  .device = m_GraphicsDevice.get(),
																				  .mesh_registry_path = "mesh_registry.db",
																				  .texture_registry_path = "texture_registry.db",
																			  });
	icons::add(*m_GraphicsDevice, L"editor_assets/icons/mesh_icon_64.png", icons::MESH_ICON_64_ID);
	icons::add(*m_GraphicsDevice, L"editor_assets/icons/mesh_icon_16.png", icons::MESH_ICON_16_ID);
	icons::add(*m_GraphicsDevice, L"editor_assets/icons/image_icon_16.png", icons::IMAGE_ICON_16_ID);
	icons::add(*m_GraphicsDevice, L"editor_assets/icons/image_icon_64.png", icons::IMAGE_ICON_64_ID);
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
	IGFD::FileDialogConfig config{};
	config.path = Sapfire::fs::FileSystem::root_directory();
	config.countSelectionMax = 1;
	ImGuiFileDialog::Instance()->OpenDialog("OpenProjectDlg", "Open project", ".sfproj", config);
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
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	if (m_ProjectPath.empty()) {
		if (ImGuiFileDialog::Instance()->Display("OpenProjectDlg")) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				auto selection = ImGuiFileDialog::Instance()->GetSelection();
				if (selection.size() > 1)
					return;
				auto& [filename, filepath] = *selection.begin();
				m_ProjectPath = filepath;
				::assets::ProjectReader reader{m_AssetManager.get(), m_ProjectName};
				reader.deserealize(filepath);
				if (m_ProjectName.empty()) {
					m_ProjectName = filename;
					reader.serialize(filepath);
				}
				ImGuiFileDialog::Instance()->Close();
			}
		}
	} else {
		ImGui::Begin("Sapling", nullptr, window_flags);
		draw_menu_bar();
		ImGuiID dockspace_id = ImGui::GetID("SaplingDockspace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));
		for (int i = 0; i < ESubeditor::COUNT; ++i) {
			if (!is_subeditor_active(static_cast<ESubeditor::TYPE>(i)))
				continue;
			m_Subeditors[i]->update(delta_time);
		}
		ImGui::End();
		for (int i = 0; i < ESubeditor::COUNT; ++i) {
			if (m_ShouldExecuteSubeditorCreationCallback[i]) {
				m_Subeditors[i].reset(subeditor_factory(static_cast<ESubeditor::TYPE>(i), true));
				if (m_Subeditors[i]) {
					m_ActiveSubeditors |= 1 << i;
					m_ShouldExecuteSubeditorCreationCallback[i] = false;
				}
			}
		}
	}
	ImGui::PopStyleVar(3);
}

void SaplingLayer::draw_menu_bar() {
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Project")) {
			if (ImGui::MenuItem("Open...", "Ctrl + O")) {
				IGFD::FileDialogConfig config{};
				m_ProjectPath = "";
				m_AssetManager =
					Sapfire::stl::make_unique<Sapfire::assets::AssetManager>(mem::Editor,
																			 Sapfire::assets::AssetManagerCreationDesc{
																				 .device = m_GraphicsDevice.get(),
																				 .mesh_registry_path = "mesh_registry.db",
																				 .texture_registry_path = "texture_registry.db",
																			 });
				config.path = Sapfire::fs::FileSystem::root_directory();
				config.countSelectionMax = 1;
				ImGuiFileDialog::Instance()->OpenDialog("OpenProjectDlg", "Open project", ".sfproj", config);
			}
			if (ImGui::MenuItem("Save", "CTRL + S")) {
				serialize();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Subeditors")) {
			for (int i = 0; i < ESubeditor::COUNT; ++i) {
				if (ImGui::MenuItem(g_SubeditorNames[i].c_str())) {
					auto type = static_cast<ESubeditor::TYPE>(i);
					if (!is_subeditor_active(type)) {
						m_Subeditors[i].reset(subeditor_factory(type));
						if (m_Subeditors[i])
							m_ActiveSubeditors |= 1 << i;
					}
				}
			}
			ImGui::EndMenu();
		}
		for (int i = 0; i < ESubeditor::COUNT; ++i) {
			if (is_subeditor_active(static_cast<ESubeditor::TYPE>(i)))
				m_Subeditors[i]->draw_menu();
		}
		ImGui::EndMenuBar();
	}
}

void SaplingLayer::serialize() {
	::assets::ProjectReader reader{m_AssetManager.get(), m_ProjectName};
	reader.serialize(m_ProjectPath);
}

void SaplingLayer::on_render() {
	PROFILE_FUNCTION();
	m_GraphicsDevice->begin_frame();
	auto& gfx_ctx = m_GraphicsDevice->current_graphics_contexts();
	for (int i = 0; i < ESubeditor::COUNT; ++i) {
		if (!is_subeditor_active(static_cast<ESubeditor::TYPE>(i)))
			continue;
		m_Subeditors[i]->render(*gfx_ctx);
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

bool SaplingLayer::is_subeditor_active(ESubeditor::TYPE type) { return m_ActiveSubeditors >> type == 1; }

SSubeditor* SaplingLayer::subeditor_factory(ESubeditor::TYPE type, bool is_callback) {
	switch (type) {
	case ESubeditor::LevelEditor:
		return mem_new(mem::Editor) SLevelEditor(m_GraphicsDevice.get(), m_AssetManager.get(), "", [this]() { serialize(); });
	}
	return nullptr;
}
