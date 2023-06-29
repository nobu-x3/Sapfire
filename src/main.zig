const std = @import("std");
const glfw = @import("zglfw");
const app = @import("core/application.zig");
const log = @import("core/logger.zig");
const game_types = @import("game_types.zig");
const Game = game_types.Game;
const GameError = game_types.GameError;

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

pub fn update(game: *Game, delta_time: f32) GameError!void {
    _ = game;
    _ = delta_time;
}

pub fn initialize(game: *Game) GameError!void {
    _ = game;
}

pub fn render(game: *Game, delta_time: f32) GameError!void {
    _ = game;
    _ = delta_time;
}
