const std = @import("std");

pub const TestTag = struct {};

pub const TagTypes = enum {
    test_tag,
};

pub const name_type_map = std.ComptimeStringMap(TagTypes, .{
    .{ "scene.tags.TestTag", .test_tag },
});
