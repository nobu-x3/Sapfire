const std = @import("std");
const sf = @import("sapfire");
const Editor = @import("editor.zig").Editor;

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    var allocator = gpa.allocator();
    var editor = try Editor.create(allocator, "project/project_config.json");
    defer editor.destroy(allocator);
    try editor.run();
}

pub fn update(game: *sf.Game, delta_time: f32) sf.GameError!void {
    _ = game;
    _ = delta_time;
}

pub fn initialize(game: *sf.Game) sf.GameError!void {
    _ = game;
}

pub fn render(game: *sf.Game, delta_time: f32) sf.GameError!void {
    _ = game;
    _ = delta_time;
}
