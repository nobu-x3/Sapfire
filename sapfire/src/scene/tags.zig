const std = @import("std");
const ecs = @import("zflecs");
const zgui = @import("zgui");

pub const TestTag = struct {
    pub fn draw_inspect(world: *ecs.world_t, entity: ecs.entity_t) void {
        if (zgui.button("TestTag", .{})) {
            ecs.remove(world, entity, TestTag);
        }
    }
};

pub const TagTypes = enum {
    test_tag,
};

pub const name_type_map = std.ComptimeStringMap(TagTypes, .{
    .{ "scene.tags.TestTag", .test_tag },
});

pub fn inspect_entity_tags(world: *ecs.world_t, entity: ecs.entity_t) void {
    zgui.separator();
    inspect_tag(TestTag, world, entity);
}

fn inspect_tag(comptime T: anytype, world: *ecs.world_t, entity: ecs.entity_t) void {
    if (ecs.has_id(world, entity, ecs.id(T))) {
        if (!ecs.is_valid(world, entity) or !ecs.is_alive(world, entity)) return;
        T.draw_inspect(world, entity);
    }
}
