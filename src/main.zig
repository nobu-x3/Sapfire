const std = @import("std");
const glfw = @import("zglfw");
const sf = @import("renderer/renderer.zig");
const log = @import("core/logger.zig");

pub fn main() !void {
    try log.init();
    defer log.deinit();
    log.info("hello", .{});
    glfw.init() catch {
        std.log.err("Failed to init glfw.", .{});
        return;
    };
    defer glfw.terminate();
    const window = glfw.Window.create(800, 600, "Sapfire", null) catch {
        std.log.err("Failed to create window.", .{});
        return;
    };
    defer window.destroy();
    window.setSizeLimits(400, 400, -1, -1);
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();
    const renderer_state = sf.renderer_create(allocator, window) catch {
        std.log.err("Failed to initialize renderer.", .{});
        return;
    };
    defer sf.destroy(allocator, renderer_state);
    while (!window.shouldClose() and window.getKey(.escape) != .press) {
        glfw.pollEvents();
        sf.update(renderer_state, window);
        sf.draw(renderer_state);
    }
}
