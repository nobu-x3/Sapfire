pub const Game = struct {
    initialize: *const fn (self: *Game) GameError!void,
    update: *const fn (self: *Game, delta_time: f32) GameError!void,
    render: *const fn (self: *Game, delta_time: f32) GameError!void,
};

pub const GameError = error{
    Initialization,
};
