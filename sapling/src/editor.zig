const std = @import("std");
const zgui = @import("zgui");
const zgpu = @import("zgpu");
const glfw = @import("zglfw");
const sapfire = @import("sapfire");
// NOTE: for some reason editor cannot find zflecs
const AssetManager = sapfire.core.AssetManager;
const core = sapfire.core;
const JobsManager = core.JobsManager;
const RendererState = sapfire.rendering.RendererState;
const Renderer = sapfire.rendering.Renderer;
const Texture = sapfire.rendering.Texture;
const Time = core.Time;
const render_system = sapfire.scene.render_system;
const log = core.log;
const nfd = @import("nfd");

pub const Editor = struct {
    window: *glfw.Window,
    gctx: *zgpu.GraphicsContext,
    asset_manager: *core.AssetManager,
    game_renderer: ?*Renderer = null,
    deferred_renderer: Renderer,
    scene_renderer: *RendererState,
    framebuffer: Texture,
    scene_framebuffer: Texture,
    current_scene: sapfire.scene.Scene,
    current_scene_path: ?[:0]const u8 = null,

    pub fn init(allocator: std.mem.Allocator, project_path: [:0]const u8, out_editor: *Editor) !void {
        out_editor.current_scene_path = null;
        JobsManager.init();
        try log.init();
        out_editor.asset_manager = try AssetManager.init(allocator, project_path);
        Time.init();
        glfw.init() catch |e| {
            log.err("Failed to init glfw.", .{});
            return e;
        };
        out_editor.window = glfw.Window.create(800, 600, "Sapling", null) catch |e| {
            log.err("Failed to create window.", .{});
            return e;
        };
        out_editor.window.setSizeLimits(400, 400, -1, -1);
        out_editor.gctx = try zgpu.GraphicsContext.create(allocator, out_editor.window, .{});
        try out_editor.asset_manager.create_defaults(out_editor.gctx);
        zgui.init(allocator);
        zgui.backend.initWithConfig(
            out_editor.window,
            out_editor.gctx.device,
            @intFromEnum(zgpu.GraphicsContext.swapchain_format),
            .{ .texture_filter_mode = .linear, .pipeline_multisample_count = 1 },
        );
        set_style();
        out_editor.framebuffer = Texture.create_with_wgpu_format(out_editor.gctx, .{
            .render_attachment = true,
            .texture_binding = true,
        }, .{
            .width = out_editor.gctx.swapchain_descriptor.width,
            .height = out_editor.gctx.swapchain_descriptor.height,
        }, out_editor.gctx.swapchain_descriptor.format);
        out_editor.scene_framebuffer =
            Texture.create_with_wgpu_format(out_editor.gctx, .{
            .render_attachment = true,
            .texture_binding = true,
        }, .{
            .width = out_editor.gctx.swapchain_descriptor.width,
            .height = out_editor.gctx.swapchain_descriptor.height,
            // }, out_editor.gctx.swapchain_descriptor.format);
        }, zgpu.GraphicsContext.swapchain_format);
        out_editor.scene_renderer = try RendererState.create_with_gctx(allocator, out_editor.gctx, out_editor.gctx.swapchain_descriptor.width, out_editor.gctx.swapchain_descriptor.height);
        out_editor.game_renderer = null;
        RendererState.renderer = null;
        try sapfire.scene.Scene.init(allocator, out_editor.gctx, "project/scenes/test_scene.json", &out_editor.current_scene);
        try Renderer.create_with_gctx(allocator, out_editor.gctx, out_editor.gctx.swapchain_descriptor.width, out_editor.gctx.swapchain_descriptor.height, &out_editor.deferred_renderer, &out_editor.current_scene.vertex_buffer, &out_editor.current_scene.index_buffer);
        Renderer.renderer = &out_editor.deferred_renderer;
    }

    var play_mode = false;

    pub fn run(self: *Editor, allocator: std.mem.Allocator) !void {
        while (!self.window.shouldClose() and self.window.getKey(.escape) != .press) {
            glfw.pollEvents();
            Time.update();
            const gctx = self.gctx;
            var color_view = gctx.lookupResource(self.framebuffer.view) orelse return;
            var scene_color_view = gctx.lookupResource(self.scene_framebuffer.view) orelse return;
            { // gui update
                zgui.backend.newFrame(
                    gctx.swapchain_descriptor.width,
                    gctx.swapchain_descriptor.height,
                );
                zgui.setNextWindowPos(.{ .x = 0.0, .y = 0.0, .cond = .first_use_ever });
                zgui.setNextWindowSize(.{ .w = 800, .h = 600, .cond = .first_use_ever });
                var size = [2]f32{ 0.0, 0.0 };
                if (zgui.beginMainMenuBar()) {
                    if (zgui.beginMenu("Scene", true)) {
                        if (zgui.menuItem("New", .{})) {
                            const new_path = try nfd.saveFileDialog(null, null);
                            if (new_path) |path| {
                                if (self.current_scene_path != null) {
                                    nfd.freePath(self.current_scene_path.?);
                                    self.current_scene_path = null;
                                }
                                self.current_scene.destroy();
                                self.current_scene = undefined;
                                try sapfire.scene.Scene.init_new(allocator, gctx, path, &self.current_scene);
                                if (self.game_renderer) |renderer| {
                                    renderer.destroy(allocator);
                                    const win_size = self.window.getSize();
                                    renderer.* = undefined;
                                    try Renderer.create_with_gctx(
                                        allocator,
                                        gctx,
                                        @intCast(win_size[0]),
                                        @intCast(win_size[1]),
                                        renderer,
                                        &self.current_scene.vertex_buffer,
                                        &self.current_scene.index_buffer,
                                    );
                                }
                                Renderer.renderer = self.game_renderer;
                                self.current_scene_path = path;
                            }
                        }
                        if (zgui.menuItem("Open", .{})) {
                            const open_path = try nfd.openFileDialog("json", null);
                            if (open_path) |path| {
                                if (self.current_scene_path != null) {
                                    nfd.freePath(self.current_scene_path.?);
                                    self.current_scene_path = null;
                                }
                                // TODO: fix scene deinit
                                self.current_scene.destroy();
                                self.current_scene = undefined;
                                try sapfire.scene.Scene.init(allocator, gctx, path, &self.current_scene);
                                if (self.game_renderer) |renderer| {
                                    renderer.destroy(allocator);
                                    renderer.* = undefined;
                                    const win_size = self.window.getSize();
                                    try Renderer.create_with_gctx(
                                        allocator,
                                        gctx,
                                        @intCast(win_size[0]),
                                        @intCast(win_size[1]),
                                        renderer,
                                        &self.current_scene.vertex_buffer,
                                        &self.current_scene.index_buffer,
                                    );
                                }
                                Renderer.renderer = self.game_renderer;
                                self.current_scene_path = path;
                            }
                        }
                        if (zgui.menuItem("Save", .{})) {
                            if (self.current_scene_path == null) {
                                const open_path = try nfd.saveFileDialog("json", null);
                                if (open_path) |path| {
                                    self.current_scene_path = try allocator.dupeZ(u8, path);
                                    try self.current_scene.serialize(allocator, path);
                                    nfd.freePath(path);
                                }
                            } else {
                                try self.current_scene.serialize(allocator, self.current_scene_path.?);
                            }
                        }
                        if (zgui.menuItem("Save as...", .{})) {
                            const open_path = try nfd.saveFileDialog("json", null);
                            if (open_path) |path| {
                                if (self.current_scene_path != null and self.current_scene_path.?.len > 0) {
                                    allocator.free(self.current_scene_path.?);
                                    self.current_scene_path = null;
                                }

                                self.current_scene_path = try allocator.dupeZ(u8, path);
                                try self.current_scene.serialize(allocator, path);
                                nfd.freePath(path);
                            }
                        }
                        zgui.endMenu();
                    }
                    if (!play_mode) {
                        if (zgui.button("Play", .{})) {
                            play_mode = true;
                            self.game_renderer = try allocator.create(Renderer);
                            try Renderer.create_with_gctx(allocator, gctx, gctx.swapchain_descriptor.width, gctx.swapchain_descriptor.height, self.game_renderer.?, &self.current_scene.vertex_buffer, &self.current_scene.index_buffer);
                            Renderer.renderer = self.game_renderer;
                        }
                    } else {
                        if (zgui.button("Stop", .{})) {
                            play_mode = false;
                            self.game_renderer.?.destroy(allocator);
                            self.game_renderer = null;
                        }
                    }
                }
                zgui.endMainMenuBar();
                if (zgui.begin("Scene View", .{ .flags = .{
                    .no_focus_on_appearing = true,
                    .no_scrollbar = true,
                } })) {
                    size = zgui.getWindowSize();
                    try self.current_scene.update_no_systems(gctx.stats.delta_time);
                    // try self.scene_renderer.draw_to_texture(&scene_color_view, @intFromFloat(size[0]), @intFromFloat(size[1]), &self.current_scene);
                    try self.deferred_renderer.draw_deferred_to_texture(&scene_color_view, @intFromFloat(size[0]), @intFromFloat(size[1]), &self.current_scene);
                    zgui.image(scene_color_view, .{ .w = size[0], .h = size[1] });
                }
                zgui.end();
                if (self.game_renderer != null) {
                    if (zgui.begin("Game View", .{ .flags = .{
                        .no_focus_on_appearing = true,
                        .no_scrollbar = true,
                        .no_background = true,
                    } })) {
                        size = zgui.getWindowSize();
                        Renderer.color_view = &color_view;
                        Renderer.fb_width = @intFromFloat(size[0]);
                        Renderer.fb_height = @intFromFloat(size[1]);
                        Renderer.renderer = self.game_renderer;
                        sapfire.scene.Scene.scene = &self.current_scene;
                        try self.current_scene.progress(gctx.stats.delta_time);
                        zgui.image(color_view, .{ .w = size[0], .h = size[1] });
                    }
                    zgui.end();
                }
                if (zgui.begin("Stats", .{ .flags = .{ .always_auto_resize = true } })) {
                    zgui.bulletText(
                        "Average :  {d:.3} ms/frame ({d:.1} fps)",
                        .{ gctx.stats.average_cpu_time, gctx.stats.fps },
                    );
                }
                zgui.end();
                self.current_scene.draw_scene_hierarchy();
                try self.current_scene.draw_inspector(self.asset_manager);
                try self.asset_manager.draw_explorer();
            }

            const swapchain_texv = gctx.swapchain.getCurrentTextureView();
            defer swapchain_texv.release();
            const commands = commands: {
                const encoder = gctx.device.createCommandEncoder(null);
                defer encoder.release();
                // Gui pass.
                {
                    const pass = zgpu.beginRenderPassSimple(encoder, .load, swapchain_texv, null, null, null);
                    defer zgpu.endReleasePass(pass);
                    zgui.backend.draw(pass);
                }

                break :commands encoder.finish(null);
            };
            defer commands.release();

            gctx.submit(&.{commands});
            _ = gctx.present();
        }
    }

    pub fn destroy(self: *Editor, allocator: std.mem.Allocator) void {
        self.current_scene.destroy();
        if (self.current_scene_path != null) {
            nfd.freePath(self.current_scene_path.?);
            self.current_scene_path = null;
        }
        self.gctx.releaseResource(self.framebuffer.handle);
        if (self.game_renderer != null) {
            self.game_renderer.?.destroy(allocator);
        }
        zgui.backend.deinit();
        zgui.deinit();
        self.gctx.destroy(allocator);
        self.window.destroy();
        glfw.terminate();
        self.asset_manager.deinit();
        log.deinit();
        JobsManager.deinit();
    }

    fn set_style() void {
        const style = zgui.getStyle();
        style.window_min_size = .{ 160.0, 20.0 };
        style.frame_padding = .{ 4.0, 2.0 };
        style.item_spacing = .{ 6.0, 2.0 };
        style.item_inner_spacing = .{ 6.0, 4.0 };
        style.alpha = 0.95;
        style.window_rounding = 4.0;
        style.frame_rounding = 2.0;
        style.indent_spacing = 6.0;
        style.item_inner_spacing = .{ 2.0, 4.0 };
        style.columns_min_spacing = 50.0;
        style.grab_min_size = 14.0;
        style.grab_rounding = 16.0;
        style.scrollbar_size = 12.0;
        style.scrollbar_rounding = 16.0;
        style.setColor(zgui.StyleCol.text, .{ 0.86, 0.93, 0.89, 0.78 });
        style.setColor(zgui.StyleCol.text_disabled, .{ 0.86, 0.93, 0.89, 0.28 });
        style.setColor(zgui.StyleCol.window_bg, .{ 0.13, 0.14, 0.17, 1.0 });
        style.setColor(zgui.StyleCol.border, .{ 0.31, 0.31, 1.0, 0.0 });
        style.setColor(zgui.StyleCol.border_shadow, .{ 0.0, 0.0, 0.0, 0.0 });
        style.setColor(zgui.StyleCol.frame_bg, .{ 0.2, 0.22, 0.27, 1.0 });
        style.setColor(zgui.StyleCol.frame_bg_hovered, .{ 0.92, 0.18, 0.29, 0.78 });
        style.setColor(zgui.StyleCol.frame_bg_active, .{ 0.92, 0.18, 0.29, 1.0 });
        style.setColor(zgui.StyleCol.title_bg, .{ 0.2, 0.22, 0.27, 1.0 });
        style.setColor(zgui.StyleCol.title_bg_collapsed, .{ 0.2, 0.22, 0.27, 0.78 });
        style.setColor(zgui.StyleCol.title_bg_active, .{ 0.92, 0.18, 0.29, 1.0 });
        style.setColor(zgui.StyleCol.menu_bar_bg, .{ 0.2, 0.22, 0.27, 0.47 });
        style.setColor(zgui.StyleCol.scrollbar_bg, .{ 0.2, 0.22, 0.27, 1.0 });
        style.setColor(zgui.StyleCol.scrollbar_grab, .{ 0.09, 0.15, 0.16, 1.0 });
        style.setColor(zgui.StyleCol.scrollbar_grab_hovered, .{ 0.92, 0.18, 0.29, 0.78 });
        style.setColor(zgui.StyleCol.scrollbar_grab_active, .{ 0.92, 0.18, 0.29, 1.0 });
        style.setColor(zgui.StyleCol.check_mark, .{ 0.71, 0.22, 0.27, 1.0 });
        style.setColor(zgui.StyleCol.slider_grab, .{ 0.47, 0.77, 0.83, 0.14 });
        style.setColor(zgui.StyleCol.slider_grab_active, .{ 0.92, 0.18, 0.29, 1.0 });
        style.setColor(zgui.StyleCol.button, .{ 0.47, 0.77, 0.83, 0.14 });
        style.setColor(zgui.StyleCol.button_hovered, .{ 0.92, 0.18, 0.29, 0.86 });
        style.setColor(zgui.StyleCol.button_active, .{ 0.92, 0.18, 0.29, 1.0 });
        style.setColor(zgui.StyleCol.header, .{ 0.92, 0.18, 0.29, 0.76 });
        style.setColor(zgui.StyleCol.header_hovered, .{ 0.92, 0.18, 0.29, 0.86 });
        style.setColor(zgui.StyleCol.header_active, .{ 0.92, 0.18, 0.29, 1.0 });
        style.setColor(zgui.StyleCol.separator, .{ 0.14, 0.16, 0.19, 1.0 });
        style.setColor(zgui.StyleCol.separator_hovered, .{ 0.92, 0.18, 0.29, 0.78 });
        style.setColor(zgui.StyleCol.separator_active, .{ 0.92, 0.18, 0.29, 1.0 });
        style.setColor(zgui.StyleCol.resize_grip, .{ 0.47, 0.77, 0.83, 0.04 });
        style.setColor(zgui.StyleCol.resize_grip_hovered, .{ 0.92, 0.18, 0.29, 0.78 });
        style.setColor(zgui.StyleCol.resize_grip_active, .{ 0.92, 0.18, 0.29, 1.0 });
        style.setColor(zgui.StyleCol.plot_lines, .{ 0.86, 0.93, 0.89, 0.63 });
        style.setColor(zgui.StyleCol.plot_lines_hovered, .{ 0.92, 0.18, 0.29, 1.0 });
        style.setColor(zgui.StyleCol.plot_histogram, .{ 0.86, 0.93, 0.89, 0.63 });
        style.setColor(zgui.StyleCol.plot_histogram_hovered, .{ 0.92, 0.18, 0.29, 1.0 });
        style.setColor(zgui.StyleCol.text_selected_bg, .{ 0.92, 0.18, 0.29, 0.43 });
        style.setColor(zgui.StyleCol.popup_bg, .{ 0.2, 0.22, 0.27, 0.9 });
        style.setColor(zgui.StyleCol.modal_window_dim_bg, .{ 0.2, 0.22, 0.27, 0.73 });
    }
};
