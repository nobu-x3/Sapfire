const sf = @import("sapfire");

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
