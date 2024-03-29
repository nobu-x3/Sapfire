const std = @import("std");
const glfw = @import("zglfw");
const sf = struct {
    usingnamespace @import("../core.zig");
    usingnamespace @import("../game_types.zig");
    usingnamespace @import("../renderer/renderer.zig");
    usingnamespace @import("asset_manager.zig");
    usingnamespace @import("jobs.zig");
};
const log = sf.log;
const Game = sf.Game;
const zgui = @import("zgui");

pub const ApplicationConfig = struct {
    window_width: i32,
    window_height: i32,
    window_name: [:0]const u8,
    game: *Game,
};

pub const Application = struct {
    game: *Game,
    window: *glfw.Window,
    gpa: std.heap.GeneralPurposeAllocator(.{}),
    allocator: std.mem.Allocator,
    renderer_state: *sf.RendererState,

    pub fn create(config: ApplicationConfig) !void {
        var gpa = std.heap.GeneralPurposeAllocator(.{}){};
        defer _ = gpa.deinit();
        const allocator = gpa.allocator();
        try log.init();
        defer log.deinit();
        sf.Time.init();
        glfw.init() catch |e| {
            log.err("Failed to init glfw.", .{});
            return e;
        };
        defer glfw.terminate();
        const window = glfw.Window.create(config.window_width, config.window_height, config.window_name, null) catch |e| {
            log.err("Failed to create window.", .{});
            return e;
        };
        defer window.destroy();
        window.setSizeLimits(400, 400, -1, -1);

        const renderer_state = sf.RendererState.create(allocator, window) catch |e| {
            log.err("Failed to initialize renderer.", .{});
            return e;
        };
        defer renderer_state.destroy(allocator);
        zgui.init(allocator);
        defer zgui.deinit();
        zgui.backend.init(window, renderer_state.gctx.device, @intFromEnum(renderer_state.gctx.swapchain_descriptor.format));
        defer zgui.backend.deinit();
        while (!window.shouldClose() and window.getKey(.escape) != .press) {
            glfw.pollEvents();
            sf.Time.update();
            try config.game.update(config.game, sf.Time.delta_time());
            sf.RendererState.update(renderer_state, window);
            try config.game.render(config.game, sf.Time.delta_time());
            try sf.RendererState.draw(renderer_state);
        }
    }
};
