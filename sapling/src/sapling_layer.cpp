#include "sapling_layer.h"
#include "ImGuiFileDialog.h"
#include "assets/project_reader.h"
#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"
#include "core/core.h"
#include "globals.h"
#include "icons.h"
#include "imgui.h"
#include "subeditors/level_editor.h"
#include "subeditors/material_editor.h"
#include "tools/profiling.h"

using namespace Sapfire;

SaplingLayer* g_Editor{nullptr};

SaplingLayer* editor() { return g_Editor; }

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
	g_Editor = this;
}

void setup_imgui_style() {
	// Modern style by LousyBook-01 from ImThemes
	ImGuiStyle& style = ImGui::GetStyle();
	style.Alpha = 1.0f;
	style.DisabledAlpha = 0.300000011920929f;
	style.WindowPadding = ImVec2(5.f, 5.f);
	style.WindowRounding = 5.f;
	style.WindowBorderSize = 1.0f;
	style.WindowMinSize = ImVec2(20.0f, 32.0f);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Right;
	style.ChildRounding = 8.199999809265137f / 2.f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 10.69999980926514f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(20.0f, 1.5f);
	style.FrameRounding = 4.800000190734863f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(9.699999809265137f / 2.f, 5.300000190734863f / 2.f);
	style.ItemInnerSpacing = ImVec2(5.400000095367432f, 9.300000190734863f);
	style.CellPadding = ImVec2(7.900000095367432f, 2.0f);
	style.IndentSpacing = 10.69999980926514f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 12.10000038146973f;
	style.ScrollbarRounding = 20.0f;
	style.GrabMinSize = 10.0f;
	style.GrabRounding = 4.599999904632568f;
	style.TabRounding = 4.0f;
	style.TabBorderSize = 0.0f;
	style.TabMinWidthForCloseButton = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(1.0f, 1.0f, 1.0f, 0.3991416096687317f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.03921568766236305f, 0.03921568766236305f, 0.03921568766236305f, 0.9399999976158142f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05098039284348488f, 0.05098039284348488f, 0.05098039284348488f, 0.9399999976158142f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 0.5f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.4206008315086365f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1411764770746231f, 0.1411764770746231f, 0.1411764770746231f, 0.4000000059604645f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2313725501298904f, 0.2313725501298904f, 0.2313725501298904f, 0.8626609444618225f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.09411764889955521f, 0.09411764889955521f, 0.09411764889955521f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.2918455004692078f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.01960784383118153f, 0.01960784383118153f, 0.01960784383118153f, 0.5299999713897705f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.407843142747879f, 0.407843142747879f, 0.407843142747879f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5098039507865906f, 0.5098039507865906f, 0.5098039507865906f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.9803921580314636f, 0.2588235437870026f, 0.2588235437870026f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9803921580314636f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.0f, 0.0f, 0.0f, 0.5793991088867188f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.09803921729326248f, 0.09803921729326248f, 0.09803921729326248f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.2313725501298904f, 0.2313725501298904f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.0f, 0.454935610294342f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.1803921610116959f, 0.1803921610116959f, 0.1803921610116959f, 0.800000011920929f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.9764705896377563f, 0.2588235437870026f, 0.2588235437870026f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 0.7799999713897705f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.09803921729326248f, 0.4000000059604645f, 0.7490196228027344f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.2000000029802322f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.6700000166893005f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.949999988079071f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.105882354080677f, 0.105882354080677f, 0.105882354080677f, 1.0f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(1.0f, 0.364705890417099f, 0.6745098233222961f, 0.800000011920929f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(1.0f, 0.2235294133424759f, 0.2235294133424759f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.1098039224743843f, 0.168627455830574f, 0.239215686917305f, 0.9724000096321106f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(1.0f, 0.2235294133424759f, 0.2235294133424759f, 0.9f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.4274509847164154f, 0.3490196168422699f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.2156862765550613f, 0.2156862765550613f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.2156862765550613f, 0.6980392336845398f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(1.0f, 0.2352941185235977f, 0.2352941185235977f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(1.0f, 0.3176470696926117f, 0.3176470696926117f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(1.0f, 0.5647059082984924f, 0.5647059082984924f, 0.3690987229347229f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.7254902124404907f, 0.3372549116611481f, 1.0f, 0.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 0.2745098173618317f, 0.2745098173618317f, 0.1115880012512207f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.9764705896377563f, 0.2588235437870026f, 0.2588235437870026f, 1.0f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.0f, 0.0f, 0.0f, 0.6437768340110779f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.4678111672401428f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.733905553817749f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.7982832789421082f);
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
	setup_imgui_style();
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
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
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
		const ImGuiID dockspace_id = ImGui::GetID("SaplingDockspace");
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
}

void SaplingLayer::draw_menu_bar() {
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Project")) {
			if (ImGui::MenuItem("Open...", "Ctrl+O")) {
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
			if (ImGui::MenuItem("Save", "CTRL+S")) {
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
	const ImGuiIO& io = ImGui::GetIO();
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

bool SaplingLayer::is_subeditor_active(ESubeditor::TYPE type) { return m_ActiveSubeditors >> type != 0; }

SSubeditor* SaplingLayer::subeditor_factory(ESubeditor::TYPE type, bool is_callback) {
	switch (type) {
	case ESubeditor::LevelEditor:
		return mem_new(mem::Editor) SLevelEditor(m_GraphicsDevice.get(), m_AssetManager.get(), "", [this]() { serialize(); });
	case ESubeditor::MaterialEditor:
		return mem_new(mem::Editor) SMaterialEditor(m_AssetManager.get(), m_GraphicsDevice.get());
	}
	return nullptr;
}
