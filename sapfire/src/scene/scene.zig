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
        const _name = ecs.get_name(world, e).?;
        std.debug.print("\tName: {s}\n", .{_name});
        const wrapped_world = World.wrap(world);
        const path = wrapped_world.entity_full_path(e, 0);
        std.debug.print("\tPath: {s}\n", .{path}); // this is a wild assumption but I cannot do anything with [*]
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
        _ = world.entity_new_with_parent(first_entt, "Grandchild");
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

    pub fn wrap(id: *ecs.world_t) World {
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

    pub fn entity_full_path(self: *const World, target: ecs.entity_t, from_parent: ecs.entity_t) []const u8 {
        var name = name_stage: {
            const _name = ecs.get_name(self.id, target) orelse break :name_stage "";
            break :name_stage std.mem.span(_name);
        };
        const path = ecs.get_path_w_sep(self.id, from_parent, target, ".", null).?;
        const len = val: {
            var separator_index: u32 = 0;
            while (true) {
                if (path[separator_index] != '.' and separator_index == name.len) break :val separator_index;
                separator_index += 1;
                if (path[separator_index] == '.' and path[separator_index + 1] == name[0]) {
                    separator_index += 1;
                    var inner_index: u32 = 0;
                    while (path[separator_index] == name[inner_index]) {
                        inner_index += 1;
                        separator_index += 1;
                        if (inner_index == name.len) break :val separator_index;
                    }
                }
            }
            break :val separator_index;
        };
        return path[0..len];
    }
};
