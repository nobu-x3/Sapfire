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
};
