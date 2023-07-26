const ecs = @import("zflecs");
const zm = @import("zmath");
const Transform = @import("../components.zig").Transform;
const World = @import("../scene.zig").World;

pub fn system() ecs.system_desc_t {
    var desc = ecs.system_desc_t{};
    desc.callback = run;
    desc.query.filter.terms[0] = .{ .id = ecs.id(Transform) };
    return desc;
}

pub fn run(it: *ecs.iter_t) callconv(.C) void {
    const entities = it.entities();
    for (0..it.count()) |i| {
        if (ecs.field(it, Transform, 1)) |transforms| {
            const parent = World.entity_get_parent_world_id(it.world, entities[i]);
            if (parent > 0) { // This is to prevent root modification
                const parent_transform = ecs.get(it.world, parent, Transform) orelse continue;
                transforms[i].world = zm.mul(transforms[i].local, parent_transform.world);
            }
        }
    }
}
