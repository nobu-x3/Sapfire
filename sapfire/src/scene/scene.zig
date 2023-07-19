const ecs = @import("zflecs");
const zgpu = @import("zgpu");
const zm = @import("zmath");
const std = @import("std");
const json = std.json;
const comps = @import("components.zig");
const asset = @import("../core.zig").AssetManager;
const Transform = comps.Transform;
const Position = comps.Position;
const Mesh = comps.Mesh;
const fs = std.fs;

const TestTag = struct {};

pub const Scene = struct {
    guid: [64]u8,
    world: World,
    arena: std.heap.ArenaAllocator,
    scene_entity: ecs.entity_t,

    pub fn create(allocator: std.mem.Allocator, gctx: *zgpu.GraphicsContext) !Scene {
        var arena = std.heap.ArenaAllocator.init(allocator);
        var world = World.init(arena.allocator());
        _ = gctx;
        try world.component_add(Transform);
        try world.component_add(Position);
        try world.component_add(Mesh);
        try world.tag_add(TestTag);
        // {
        //     var sys_desc = ecs.system_desc_t{};
        //     sys_desc.callback = OnStart;
        //     sys_desc.query.filter.terms[0] = .{ .id = ecs.id(Transform) };
        //     ecs.SYSTEM(world.id, "On Start", ecs.OnStart, &sys_desc);
        // }
        const scene_entity = world.entity_new("Root");
        var first_entt = world.entity_new_with_parent(scene_entity, "Child");
        _ = ecs.add_id(world.id, first_entt, ecs.id(TestTag));
        _ = world.entity_new_with_parent(first_entt, "Grandchild");
        var file = try fs.cwd().createFile("project/scenes/test_scene.json", .{});
        defer file.close();
        try world.serialize(allocator, &file);
        // const json_world = ecs.world_to_json(world.id, &.{}).?;
        // std.debug.print("\n{s}", .{json_world});
        // _ = ecs.progress(world.id, 0);

        return Scene{
            .guid = asset.generate_guid("test_scene"),
            .world = world,
            .arena = arena,
            .scene_entity = scene_entity,
        };
    }

    pub fn destroy(self: *Scene) void {
        self.world.deinit();
        self.arena.deinit();
    }
};

pub const World = struct {
    id: *ecs.world_t,
    component_id_map: std.AutoHashMap(ecs.id_t, [:0]const u8),
    tag_id_map: std.AutoHashMap(ecs.id_t, [:0]const u8),

    pub fn init(allocator: std.mem.Allocator) World {
        const id = ecs.init();
        const component_id_map = std.AutoHashMap(ecs.id_t, [:0]const u8).init(allocator);
        const tag_id_map = std.AutoHashMap(ecs.id_t, [:0]const u8).init(allocator);

        return World{
            .id = id,
            .component_id_map = component_id_map,
            .tag_id_map = tag_id_map,
        };
    }

    pub fn wrap(id: *ecs.world_t) World {
        return World{
            .id = id,
        };
    }

    pub fn deinit(self: *World) void {
        _ = self;
        // NOTE: this segfaults, so just leaving it here.
        // _ = ecs.fini(self.id);
    }

    pub fn component_add(self: *World, comptime T: type) !void {
        ecs.COMPONENT(self.id, T);
        const id = ecs.id(T);
        try self.component_id_map.put(id, @typeName(T));
    }

    pub fn tag_add(self: *World, comptime T: type) !void {
        ecs.TAG(self.id, T);
        const id = ecs.id(T);
        try self.tag_id_map.put(id, @typeName(T));
    }

    pub fn entity_new(self: *World, name: [*:0]const u8) ecs.entity_t {
        var entity = ecs.new_entity(self.id, name);
        _ = ecs.set(self.id, entity, Transform, .{});
        _ = ecs.set(self.id, entity, Position, Position{ .x = 0.0, .y = 1.0, .z = 0.0 });
        return entity;
    }

    pub fn entity_new_with_parent(self: *World, parent: ecs.entity_t, name: [*:0]const u8) ecs.entity_t {
        var entity = ecs.new_w_id(self.id, ecs.pair(ecs.ChildOf, parent));
        _ = ecs.set(self.id, entity, Transform, .{});
        _ = ecs.set(self.id, entity, Position, Position{ .x = 0.0, .y = 1.0, .z = 0.0 });
        _ = ecs.set_name(self.id, entity, name);
        return entity;
    }

    pub fn entity_is_child_of(self: *World, target: ecs.entity_t, parent: ecs.entity_t) bool {
        var it = ecs.children(self.id, parent);
        while (ecs.children_next(&it)) {
            for (it.entities()) |e| {
                if (target == e) return true;
            }
        }
        return false;
    }

    pub fn entity_is_scene_entity(self: *World, entity: ecs.entity_t) bool {
        const path = self.entity_full_path(entity, 0);
        if (path.len < 6) return false;
        if (!std.mem.eql(u8, path[0..5], "Root.")) return false;
        return true;
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

    const ComponentValueTag = enum { matrix, vector };
    const ParseComponent = struct {
        name: []const u8,
        value: union(ComponentValueTag) {
            matrix: [16]f32,
            vector: [3]f32,
        },
    };
    const ParseEntity = struct {
        name: [:0]const u8,
        path: []const u8,
        id: u64,
        components: []const ParseComponent,
        tags: [][:0]const u8,
    };
    const ParseWorld = struct { entities: []const ParseEntity };

    pub fn serialize(self: *World, allocator: std.mem.Allocator, file: *fs.File) !void {
        var parse_arena = std.heap.ArenaAllocator.init(allocator);
        defer parse_arena.deinit();
        var writer = file.writer();
        var filter_desc = ecs.filter_desc_t{};
        filter_desc.terms[0] = .{ .id = ecs.Any };
        const filter = try ecs.filter_init(self.id, &filter_desc);
        var it = ecs.filter_iter(self.id, filter);
        var entity_list = std.ArrayList(ParseEntity).init(parse_arena.allocator());
        while (ecs.filter_next(&it)) {
            const world_id = it.world;
            const entities = it.entities();
            // const transforms = ecs.field(&it, Transform, 1).?;
            for (entities) |e| {
                var tag_list = std.ArrayList([:0]const u8).init(parse_arena.allocator());
                if (!self.entity_is_scene_entity(e)) continue;
                const _name = ecs.get_name(world_id, e).?;
                const entity_name = std.mem.span(_name);
                const path = self.entity_full_path(e, 0);
                // std.debug.print("\tTransform: {d}\n", .{transforms[i].matrix});
                var component_list = std.ArrayList(ParseComponent).init(parse_arena.allocator());
                {
                    const types = ecs.get_type(world_id, e).?;
                    var comp_len: usize = 0;
                    const type_count = @intCast(usize, types.count);
                    var components = types.array;
                    for (types.array[0..type_count]) |comp| {
                        if (ecs.id_is_pair(comp) or ecs.id_is_tag(world_id, comp)) {
                            continue;
                        }
                        components[comp_len] = comp;
                        comp_len += 1;
                    }
                    for (components, 0..comp_len) |comp, _| {
                        if (self.component_id_map.contains(comp)) {
                            const comp_name = self.component_id_map.get(comp).?;
                            if (std.mem.eql(u8, comp_name, "scene.components.Transform")) { // TODO: think of a better way of doing this
                                const transform = ecs.get(world_id, e, Transform).?;
                                var matrix = zm.matToArr(transform.matrix);
                                try component_list.append(.{ .name = "scere.components.Transform", .value = .{ .matrix = matrix } });
                            } else if (std.mem.eql(u8, comp_name, "scene.components.Position")) {
                                const pos = ecs.get(world_id, e, Position).?;
                                try component_list.append(.{ .name = "scene.components.Position", .value = .{ .vector = .{ pos.x, pos.y, pos.z } } });
                            }
                        }
                    }
                }
                {
                    const types = ecs.get_type(world_id, e).?;
                    var tag_len: usize = 0;
                    const type_count = @intCast(usize, types.count);
                    var tags = types.array;
                    for (types.array[0..type_count]) |comp| {
                        if (ecs.id_is_pair(comp)) {
                            continue;
                        }
                        if (ecs.id_is_tag(world_id, comp)) {
                            tags[tag_len] = comp;
                            tag_len += 1;
                        }
                    }
                    for (tags, 0..tag_len) |tag, _| {
                        if (self.tag_id_map.contains(tag)) {
                            const tag_name = self.tag_id_map.get(tag).?;
                            try tag_list.append(tag_name);
                        }
                    }
                }
                try entity_list.append(.{ .name = entity_name, .path = path, .id = e, .components = component_list.items, .tags = tag_list.items });
            }
        }
        try json.stringify(ParseWorld{ .entities = entity_list.items }, .{}, writer);
    }
};
