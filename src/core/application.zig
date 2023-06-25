const GameTypes = @import("../game_types.zig");
const Game = GameTypes.Game;
const renderer = @import("../renderer/renderer.zig");
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
    allocator: std.mem.Allocator,
    renderer_state: *renderer.RendererState,
};

pub fn application_run(app: *Application) !void {
    while (!app.window.shouldClose() and app.window.getKey(.escape) != .press) {
        glfw.pollEvents();
        try app.game.update(app.game, 0.0);
        renderer.update(app.renderer_state, app.window);
        try app.game.render(app.game, 0.0);
        renderer.draw(app.renderer_state);
    }
    renderer.destroy(app.allocator, app.renderer_state);
    app.window.destroy();
    glfw.terminate();
    log.deinit();
}

pub fn application_create(config: ApplicationConfig) !Application {
    try log.init();
    glfw.init() catch |e| {
        log.err("Failed to init glfw.", .{});
        return e;
    };
    const window = glfw.Window.create(config.window_width, config.window_height, config.window_name, null) catch |e| {
        log.err("Failed to create window.", .{});
        return e;
    };
    window.setSizeLimits(400, 400, -1, -1);
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();
    const renderer_state = renderer.renderer_create(allocator, window) catch |e| {
        log.err("Failed to initialize renderer.", .{});
        return e;
    };
    return Application{
        .game = config.game,
        .window = window,
        .allocator = allocator,
        .renderer_state = renderer_state,
    };
}
