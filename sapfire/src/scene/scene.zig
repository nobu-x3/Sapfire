const ecs = @import("zflecs");
const zgpu = @import("zgpu");
const std = @import("std");
const comps = @import("components.zig");
const asset = @import("../core.zig").AssetManager;
const Transform = comps.Transform;
const Mesh = comps.Mesh;

pub const Scene = struct {
    guid: [64]u8,
    world: *ecs.world_t,
    arena: std.heap.ArenaAllocator,
    scene_entity: ecs.entity_t,

    pub fn create(allocator: std.mem.Allocator, gctx: *zgpu.GraphicsContext) Scene {
        var arena = std.heap.ArenaAllocator.init(allocator);
        var world = ecs.init();
        _ = gctx;
        ecs.COMPONENT(world, Transform);
        ecs.COMPONENT(world, Mesh);

        const scene_entity = ecs.new_entity(world, "Root");
        _ = ecs.set(world, scene_entity, Transform, .{});
        var first_entt = entity_new_with_parent(world, scene_entity, "Child");
        _ = ecs.set(world, first_entt, Transform, .{});

        const path = ecs.get_path_w_sep(world, 0, first_entt, ".", null);
        std.log.info("Path: {s}", .{path.?[0..10]});

        return Scene{
            .guid = asset.generate_guid("test_scene"),
            .world = world,
            .arena = arena,
            .scene_entity = scene_entity,
        };
    }

    pub fn destroy(self: *Scene, allocator: std.mem.Allocator) void {
        _ = allocator;
        _ = ecs.fini(self.world);
    }
};

pub fn entity_new(world: *ecs.world_t, name: [*:0]const u8) ecs.entity_t {
    return ecs.new_entity(world, name);
}

pub fn entity_new_with_parent(world: *ecs.world_t, parent: ecs.entity_t, name: [*:0]const u8) ecs.entity_t {
    var entity = ecs.new_w_id(world, ecs.pair(ecs.ChildOf, parent));
    _ = ecs.set_name(world, entity, name);
    return entity;
}
