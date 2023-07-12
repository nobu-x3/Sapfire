const std = @import("std");
const sapfire = @import("sapfire");

pub fn main() !void {
    std.debug.print("All your {s} are belong to us.\n", .{"codebase"});
    try sapfire.main();
}
