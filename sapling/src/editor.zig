const std = @import("std");
const zgui = @import("zgui");
const zgpu = @import("zgpu");
const glfw = @import("zglfw");
const AssetManager = @import("sapfire").core.AssetManager;
const JobsManager = @import("sapfire").core.JobsManager;
const Time = @import("sapfire").core.Time;
const log = @import("sapfire").core.log;

pub const Editor = struct {
    window: *glfw.Window,
    gctx: *zgpu.GraphicsContext,

    pub fn create(allocator: std.mem.Allocator, project_path: [:0]const u8) !Editor {
        JobsManager.init();
        try log.init();
        try AssetManager.init(allocator, project_path);
        Time.init();
        glfw.init() catch |e| {
            log.err("Failed to init glfw.", .{});
            return e;
        };
        const window = glfw.Window.create(800, 600, "Sapling", null) catch |e| {
            log.err("Failed to create window.", .{});
            return e;
        };
        window.setSizeLimits(400, 400, -1, -1);
        var gctx = try zgpu.GraphicsContext.create(allocator, window);
        zgui.init(allocator);
        zgui.backend.initWithConfig(
            window,
            gctx.device,
            @enumToInt(zgpu.GraphicsContext.swapchain_format),
            .{ .texture_filter_mode = .linear, .pipeline_multisample_count = 1 },
        );
        set_style();
        return Editor{
            .window = window,
            .gctx = gctx,
        };
    }

    pub fn run(self: *Editor) !void {
        while (!self.window.shouldClose() and self.window.getKey(.escape) != .press) {
            glfw.pollEvents();
            Time.update();
            const gctx = self.gctx;
            { // gui update
                zgui.backend.newFrame(
                    gctx.swapchain_descriptor.width,
                    gctx.swapchain_descriptor.height,
                );
                // zgui.setNextWindowPos(.{ .x = 20.0, .y = 20.0, .cond = .first_use_ever });
                // zgui.setNextWindowSize(.{ .w = -1.0, .h = -1.0, .cond = .first_use_ever });
                if (zgui.begin("Stats", .{ .flags = .{ .always_auto_resize = true } })) {
                    zgui.bulletText(
                        "Average :  {d:.3} ms/frame ({d:.1} fps)",
                        .{ gctx.stats.average_cpu_time, gctx.stats.fps },
                    );
                }
                zgui.end();
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
        zgui.backend.deinit();
        zgui.deinit();
        self.gctx.destroy(allocator);
        self.window.destroy();
        glfw.terminate();
        AssetManager.deinit();
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
