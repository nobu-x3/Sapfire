const ecs = @import("zflecs");
const zgpu = @import("zgpu");
const std = @import("std");
const json = std.json;
const comps = @import("components.zig");
const asset = @import("../core.zig").AssetManager;
const Transform = comps.Transform;
const Mesh = comps.Mesh;

fn OnStart(it: *ecs.iter_t) callconv(.C) void {
    const world = it.world;
    const entities = it.entities();
    for (entities) |e| {
        std.debug.print("Entity ID: {d}\n", .{e});
        name_stage: {
            const name = ecs.get_name(world, e) orelse break :name_stage;
            std.debug.print("\tName: {s}\n", .{name});
        }
        full_path_stage: {
            const path = ecs.get_path_w_sep(world, 0, e, ".", null) orelse break :full_path_stage;
            // const len = val: {
            //     var index: u32 = 0;
            //     while (path[index] != '\0') {
            //         index += 1;
            //         std.debug.print("{d}\n", .{index});
            //     }
            //     break :val index;
            // };
            std.debug.print("\tPath: {s}\n", .{path[0..64]}); // this is a wild assumption but I cannot do anything with [*]
        }
        components_stage: {
            const types = ecs.get_type(world, e);
            const str = ecs.type_str(world, types) orelse break :components_stage;
            const casted = std.mem.span(str);
            std.debug.print("\tComponents: {s}\n", .{casted});
        }
    }
}

pub const Scene = struct {
    guid: [64]u8,
    world: World,
    arena: std.heap.ArenaAllocator,
    scene_entity: ecs.entity_t,

    pub fn create(allocator: std.mem.Allocator, gctx: *zgpu.GraphicsContext) Scene {
        var arena = std.heap.ArenaAllocator.init(allocator);
        var world = World.init();
        _ = gctx;
        world.component_add(Transform);
        world.component_add(Mesh);
        {
            var sys_desc = ecs.system_desc_t{};
            sys_desc.callback = OnStart;
            sys_desc.query.filter.terms[0] = .{ .id = ecs.id(Transform) };
            ecs.SYSTEM(world.id, "On Start", ecs.OnStart, &sys_desc);
        }
        const scene_entity = world.entity_new("Root");
        var first_entt = world.entity_new_with_parent(scene_entity, "Child");
        _ = first_entt;
        _ = ecs.progress(world.id, 0);

        return Scene{
            .guid = asset.generate_guid("test_scene"),
            .world = world,
            .arena = arena,
            .scene_entity = scene_entity,
        };
    }

    pub fn destroy(self: *Scene, allocator: std.mem.Allocator) void {
        _ = allocator;
        self.world.deinit();
    }
};

pub const World = struct {
    id: *ecs.world_t,

    pub fn init() World {
        const id = ecs.init();
        return World{
            .id = id,
        };
    }

    pub fn deinit(self: *World) void {
        _ = ecs.fini(self.id);
    }

    pub fn component_add(self: *World, comptime T: type) void {
        ecs.COMPONENT(self.id, T);
    }

    pub fn entity_new(self: *World, name: [*:0]const u8) ecs.entity_t {
        var entity = ecs.new_entity(self.id, name);
        _ = ecs.set(self.id, entity, Transform, .{});
        return entity;
    }

    pub fn entity_new_with_parent(self: *World, parent: ecs.entity_t, name: [*:0]const u8) ecs.entity_t {
        var entity = ecs.new_w_id(self.id, ecs.pair(ecs.ChildOf, parent));
        _ = ecs.set(self.id, entity, Transform, .{});
        _ = ecs.set_name(self.id, entity, name);
        return entity;
    }
};
