const std = @import("std");
const sf = @import("sapfire");
// usingnamespace @import("sapfire");

pub fn main() !void {
    var game = sf.Game{
        .initialize = &initialize,
        .update = &update,
        .render = &render,
    };
    const config = sf.app.ApplicationConfig{
        .window_width = 800,
        .window_height = 600,
        .window_name = "Sapfire",
        .game = &game,
    };
    var application = try sf.app.Application.create(config);
    _ = application;
}

fn update(game: *sf.Game, delta_time: f32) sf.GameError!void {
    _ = game;
    _ = delta_time;
}

fn initialize(game: *sf.Game) sf.GameError!void {
    _ = game;
}

fn render(game: *sf.Game, delta_time: f32) sf.GameError!void {
    _ = game;
    _ = delta_time;
}
