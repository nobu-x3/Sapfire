const sf = @import("sapfire");

pub fn main() !void {}

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
