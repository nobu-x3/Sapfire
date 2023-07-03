const std = @import("std");
const glfw = @import("zglfw");
pub const app = @import("core/application.zig");
pub const log = @import("core/logger.zig");
const game_types = @import("game_types.zig");
pub const Game = game_types.Game;
pub const GameError = game_types.GameError;

pub fn main() !void {
    var game = Game{
        .initialize = &initialize,
        .update = &update,
        .render = &render,
    };
    const config = app.ApplicationConfig{
        .window_width = 800,
        .window_height = 600,
        .window_name = "Sapfire",
        .game = &game,
    };
    var application = try app.Application.create(config);
    _ = application;
}

extern fn update(game: *Game, delta_time: f32) GameError!void;

extern fn initialize(game: *Game) GameError!void;

extern fn render(game: *Game, delta_time: f32) GameError!void;
