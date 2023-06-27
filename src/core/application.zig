const GameTypes = @import("../game_types.zig");
const Game = GameTypes.Game;
const renderer = @import("../renderer/renderer.zig");
const asset = @import("asset_manager.zig");
const log = @import("logger.zig");
const std = @import("std");
const glfw = @import("zglfw");

pub const ApplicationConfig = struct {
    window_width: i32,
    window_height: i32,
    window_name: [:0]const u8,
    game: *Game,
};

const Application = struct {
    game: *Game,
    window: *glfw.Window,
    gpa: std.heap.GeneralPurposeAllocator(.{}),
    allocator: std.mem.Allocator,
    renderer_state: *renderer.RendererState,
};

pub fn application_create(config: ApplicationConfig) !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();
    try log.init();
    defer log.deinit();
    asset.init();
    defer asset.deinit();
    asset.create_asset("assets/textures/cobblestone.png");
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

    const renderer_state = renderer.renderer_create(allocator, window) catch |e| {
        log.err("Failed to initialize renderer.", .{});
        return e;
    };
    defer renderer.destroy(allocator, renderer_state);
    while (!window.shouldClose() and window.getKey(.escape) != .press) {
        glfw.pollEvents();
        try config.game.update(config.game, 0.0);
        renderer.update(renderer_state, window);
        try config.game.render(config.game, 0.0);
        renderer.draw(renderer_state);
    }
}
