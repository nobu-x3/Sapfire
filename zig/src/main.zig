const std = @import("std");
const sdl = @import("zsdl");

pub fn main() !void {
    try sdl.init(.{ .audio = true, .video = true });
    defer sdl.quit();

    const window = try sdl.Window.create(
        "zig-gamedev-window",
        sdl.Window.pos_undefined,
        sdl.Window.pos_undefined,
        600,
        600,
        .{ .opengl = true, .allow_highdpi = true },
    );
    defer window.destroy();
    mainloop: while (true) {
        var event: ?*sdl.Event = null;
        while (sdl.pollEvent(event)) {
            if (event == null) continue;
            switch (event.?.type) {
                sdl.EventType.quit => break :mainloop,
                else => {},
            }
        }
    }
}

test "simple test" {
    var list = std.ArrayList(i32).init(std.testing.allocator);
    defer list.deinit(); // try commenting this out and see if zig detects the memory leak!
    try list.append(42);
    try std.testing.expectEqual(@as(i32, 42), list.pop());
}
