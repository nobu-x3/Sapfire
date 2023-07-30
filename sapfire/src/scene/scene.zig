const ecs = @import("zflecs");
const zgpu = @import("zgpu");
const zgui = @import("zgui");
const zm = @import("zmath");
const std = @import("std");
const json = std.json;
const comps = @import("components.zig");
const tags = @import("tags.zig");
const log = @import("../core/logger.zig");
const asset = @import("../core.zig").AssetManager;
const Transform = comps.Transform;
const Scale = comps.Scale;
const Mesh = comps.Mesh;
const Material = @import("../renderer/material.zig").Material;
const TestTag = tags.TestTag;
const fs = std.fs;
const sf = struct {
    usingnamespace @import("../core/asset_manager.zig");
    usingnamespace @import("../renderer/mesh.zig");
    usingnamespace @import("../renderer/renderer_types.zig");
    usingnamespace @import("../renderer/material.zig");
    usingnamespace @import("../renderer/buffer.zig");
    usingnamespace @import("../renderer/texture.zig");
    usingnamespace @import("../renderer/pipeline.zig");
};

const ComponentValueTag = enum { matrix, vector, path, guid };

const ParseComponent = struct {
    name: [:0]const u8,
    value: union(ComponentValueTag) {
        matrix: [16]f32,
        vector: [3]f32,
        path: [:0]const u8,
        guid: [64]u8,
    },
};

const ParseEntity = struct {
    name: [:0]const u8,
    path: [:0]const u8,
    id: u64,
    components: []const ParseComponent,
    tags: [][:0]const u8,
};

const ParseWorld = struct {
    entities: []const ParseEntity,
    tags: [][:0]const u8,
    components: [][:0]const u8,
};

pub const SceneConfig = struct {
    textures: [][:0]const u8,
    materials: [][:0]const u8,
    meshes: [][:0]const u8,
    world: ParseWorld,
};
const ParseMesh = struct {
    geometry_path: [:0]const u8,
    material_path: [:0]const u8,
    texture_path: [:0]const u8,
};
const ParseScene = struct {
    world: ParseWorld,
    meshes: []const ParseMesh,
};

pub const SceneAsset = struct {
    guid: [64]u8,
    texture_paths: std.ArrayList([:0]const u8),
    material_paths: std.ArrayList([:0]const u8),
    geometry_paths: std.ArrayList([:0]const u8),
    world: ParseWorld,

    pub fn create(database_allocator: std.mem.Allocator, parse_allocator: std.mem.Allocator, path: [:0]const u8) !SceneAsset {
        const scene_guid = sf.AssetManager.generate_guid(path);
        const config_data = std.fs.cwd().readFileAlloc(parse_allocator, path, 512 * 16) catch |e| {
            log.err("Failed to parse scene config file. Given path:{s}", .{path});
            return e;
        };
        const config = try json.parseFromSliceLeaky(ParseScene, database_allocator, config_data, .{});
        var texture_paths = try std.ArrayList([:0]const u8).initCapacity(database_allocator, config.meshes.len);
        var geometry_paths = try std.ArrayList([:0]const u8).initCapacity(database_allocator, config.meshes.len);
        var material_paths = try std.ArrayList([:0]const u8).initCapacity(database_allocator, config.meshes.len);
        for (config.meshes) |mesh| {
            try texture_paths.append(mesh.texture_path);
            try geometry_paths.append(mesh.geometry_path);
            try material_paths.append(mesh.material_path);
        }
        return SceneAsset{
            .guid = scene_guid,
            .texture_paths = texture_paths,
            .material_paths = material_paths,
            .geometry_paths = geometry_paths,
            .world = config.world,
        };
    }
};

pub const Scene = struct {
    guid: [64]u8,
    world: World,
    asset: SceneAsset,
    arena: std.heap.ArenaAllocator,
    scene_entity: ecs.entity_t,
    vertices: std.ArrayList(sf.Vertex),
    indices: std.ArrayList(u32),
    pipeline_system: sf.PipelineSystem,
    mesh_manager: sf.MeshManager,
    texture_manager: sf.TextureManager,
    material_manager: sf.MaterialManager,
    global_uniform_bind_group: zgpu.BindGroupHandle,
    vertex_buffer: zgpu.BufferHandle,
    index_buffer: zgpu.BufferHandle,

    pub var scene: ?*Scene = null;
    pub var currently_selected_entity: ecs.entity_t = undefined;

    pub fn create(allocator: std.mem.Allocator, gctx: *zgpu.GraphicsContext, path: [:0]const u8) !Scene {
        var arena = std.heap.ArenaAllocator.init(allocator);
        var world = try World.init(arena.allocator());
        var parse_arena = std.heap.ArenaAllocator.init(allocator);
        defer parse_arena.deinit();
        const scene_asset = try SceneAsset.create(arena.allocator(), parse_arena.allocator(), path);
        // manager inits can be jobified
        var texman = try sf.TextureManager.init_from_slice(arena.allocator(), scene_asset.texture_paths.items);
        var matman = try sf.MaterialManager.init_from_slice(arena.allocator(), scene_asset.material_paths.items);
        var meshman = try sf.MeshManager.init_from_slice(arena.allocator(), scene_asset.geometry_paths.items);
        var meshes = std.ArrayList(Mesh).init(arena.allocator()); // NOTE: we don't actually need to cache any of it after creating vert/ind buffers
        try meshes.ensureTotalCapacity(128);
        var vertices = std.ArrayList(sf.Vertex).init(arena.allocator());
        defer vertices.deinit();
        try vertices.ensureTotalCapacity(256);
        var indices = std.ArrayList(u32).init(arena.allocator());
        defer indices.deinit();
        try indices.ensureTotalCapacity(256);
        var pipeline_system = try sf.PipelineSystem.init(arena.allocator());
        const global_uniform_bgl = gctx.createBindGroupLayout(&.{
            zgpu.bufferEntry(0, .{ .vertex = true, .fragment = true }, .uniform, true, 0),
        });
        defer gctx.releaseResource(global_uniform_bgl);
        const global_uniform_bind_group = gctx.createBindGroup(global_uniform_bgl, &.{
            .{
                .binding = 0,
                .buffer_handle = gctx.uniforms.buffer,
                .offset = 0,
                .size = @sizeOf(sf.GlobalUniforms),
            },
        });
        const scene_entity = try world.deserialize(&scene_asset, gctx, global_uniform_bgl, &pipeline_system, &texman, &matman, &meshman, &meshes, &vertices, &indices);
        currently_selected_entity = scene_entity;
        const vertex_buffer = sf.buffer_create_and_load(gctx, .{ .copy_dst = true, .vertex = true }, sf.Vertex, vertices.items);
        // Create an index buffer.
        const index_buffer = sf.buffer_create_and_load(gctx, .{ .copy_dst = true, .index = true }, u32, indices.items);
        var update_transforms_system = @import("systems/update_transforms_system.zig").system();
        ecs.SYSTEM(world.id, "Local to world transforms", ecs.PreUpdate, &update_transforms_system);
        var render_color_system = @import("systems/render_color_system.zig").system();
        ecs.SYSTEM(world.id, "render", ecs.PostUpdate, &render_color_system);
        return Scene{
            .guid = sf.AssetManager.generate_guid(path),
            .world = world,
            .asset = scene_asset,
            .arena = arena,
            .scene_entity = scene_entity,
            .vertices = vertices,
            .indices = indices,
            .pipeline_system = pipeline_system,
            .mesh_manager = meshman,
            .texture_manager = texman,
            .material_manager = matman,
            .global_uniform_bind_group = global_uniform_bind_group,
            .vertex_buffer = vertex_buffer,
            .index_buffer = index_buffer,
        };
    }

    pub fn update(self: *Scene, delta_time: f32) !void {
        _ = ecs.progress(self.world.id, delta_time);
    }

    pub fn destroy(self: *Scene) void {
        self.world.deinit();
        self.arena.deinit();
    }

    // TODO: this should be in the editor
    pub fn draw_scene_hierarchy(self: *Scene) void {
        if (zgui.begin("Hierarchy", .{})) {
            const root_entity = ecs.lookup(self.world.id, "Root");
            if (root_entity > 0) {
                if (zgui.treeNodeFlags("Root", .{})) {
                    draw_children_nodes(self, root_entity);
                    zgui.treePop();
                }
            }
            if (!ecs.is_valid(self.world.id, currently_selected_entity) or !ecs.is_alive(self.world.id, currently_selected_entity))
                currently_selected_entity = self.scene_entity;
        }
        zgui.end();
    }

    pub fn draw_inspector(self: *Scene) void {
        if (zgui.begin("Inspector", .{})) {
            const entity_name = ecs.get_name(self.world.id, currently_selected_entity) orelse {
                zgui.end();
                return;
            };
            const span_name = std.mem.span(entity_name);
            var buf = [_]u8{0} ** 128;
            std.mem.copyForwards(u8, &buf, span_name);
            if (zgui.inputText("Name: ", .{
                .buf = &buf,
                .flags = .{ .enter_returns_true = true, .read_only = currently_selected_entity == self.scene_entity },
            })) {
                _ = ecs.set_name(self.world.id, currently_selected_entity, @ptrCast(&buf));
            }
            zgui.text("Tags:", .{});
            tags.inspect_entity_tags(self.world.id, currently_selected_entity);
            if (currently_selected_entity != self.scene_entity) {
                comps.inspect_entity_components(self.world.id, currently_selected_entity);
                if (zgui.button("Add Component", .{})) {
                    zgui.openPopup("Add Component Popup", .{});
                }
                if (zgui.beginPopup("Add Component Popup", .{})) {
                    if (zgui.selectable("Transform", .{ .flags = .{ .allow_double_click = true } })) {
                        ecs.add(self.world.id, currently_selected_entity, Scale);
                        _ = ecs.set(self.world.id, currently_selected_entity, Scale, .{});
                        ecs.add(self.world.id, currently_selected_entity, Transform);
                        _ = ecs.set(self.world.id, currently_selected_entity, Transform, .{});
                        zgui.closeCurrentPopup();
                    }
                    zgui.endPopup();
                }
            }
        }
        zgui.end();
    }

    fn draw_children_nodes(self: *Scene, entity: ecs.entity_t) void {
        var iter = ecs.children(self.world.id, entity);
        while (ecs.children_next(&iter)) {
            for (iter.entities()) |e| {
                var selected = false;
                if (e == currently_selected_entity) {
                    selected = true;
                }
                const name = ecs.get_name(self.world.id, e) orelse break;
                const casted_name: [:0]const u8 = std.mem.span(name);

                if (zgui.treeNodeFlags(casted_name, .{ .selected = selected, .open_on_arrow = true })) {
                    if (zgui.isItemClicked(.left)) {
                        currently_selected_entity = e;
                    }

                    if (zgui.beginPopupContextItem()) {
                        currently_selected_entity = e;
                        if (zgui.selectable("New", .{})) {
                            _ = self.world.entity_new_with_parent(e, "New Entity");
                            zgui.closeCurrentPopup();
                        }
                        if (zgui.selectable("Delete", .{})) {
                            self.world.entity_delete(e);
                            zgui.closeCurrentPopup();
                        }

                        zgui.endPopup();
                    }
                    draw_children_nodes(self, e);
                    zgui.treePop();
                }
            }
        }
    }

    pub fn serialize(self: *Scene, allocator: std.mem.Allocator, file_path: [:0]const u8) !void {
        var parse_arena = std.heap.ArenaAllocator.init(allocator);
        defer parse_arena.deinit();
        var file = try fs.cwd().createFile(file_path, .{});
        defer file.close();
        var writer = file.writer();
        var filter_desc = ecs.filter_desc_t{};
        filter_desc.terms[0] = .{ .id = ecs.Any };
        const filter = try ecs.filter_init(self.world.id, &filter_desc);
        var it = ecs.filter_iter(self.world.id, filter);
        var entity_list = std.ArrayList(ParseEntity).init(parse_arena.allocator());
        while (ecs.filter_next(&it)) {
            const world_id = it.world;
            const entities = it.entities();
            // const transforms = ecs.field(&it, Transform, 1).?;
            for (entities) |e| {
                var tag_list = std.ArrayList([:0]const u8).init(parse_arena.allocator());
                if (!self.world.entity_is_scene_entity(e)) continue;
                const _name = ecs.get_name(world_id, e).?;
                const entity_name = std.mem.span(_name);
                const path = self.world.entity_full_path(e, 0);
                // std.debug.print("\tTransform: {d}\n", .{transforms[i].matrix});
                var component_list = std.ArrayList(ParseComponent).init(parse_arena.allocator());
                {
                    const types = ecs.get_type(world_id, e).?;
                    var comp_len: usize = 0;
                    const type_count: usize = @intCast(types.count);
                    var components = types.array;
                    for (types.array[0..type_count]) |comp| {
                        if (ecs.id_is_pair(comp) or ecs.id_is_tag(world_id, comp)) {
                            continue;
                        }
                        components[comp_len] = comp;
                        comp_len += 1;
                    }
                    for (components, 0..comp_len) |comp, _| {
                        if (self.world.component_id_map.contains(comp)) {
                            const comp_name = self.world.component_id_map.get(comp).?;
                            if (std.mem.eql(u8, comp_name, "scene.components.Transform")) { // TODO: think of a better way of doing this
                                const transform = ecs.get(world_id, e, Transform).?;
                                var matrix = zm.matToArr(transform.local);
                                try component_list.append(.{ .name = "scene.components.Transform", .value = .{ .matrix = matrix } });
                            } else if (std.mem.eql(u8, comp_name, "scene.components.Scale")) {
                                const scale = ecs.get(world_id, e, Scale).?;
                                try component_list.append(.{ .name = "scene.components.Scale", .value = .{ .vector = .{ scale.scale[0], scale.scale[1], scale.scale[2] } } });
                            } else if (std.mem.eql(u8, comp_name, "scene.components.Mesh")) {
                                const mesh = ecs.get(world_id, e, Mesh).?;
                                try component_list.append(.{ .name = "scene.components.Mesh", .value = .{ .guid = mesh.guid } });
                            } else if (std.mem.eql(u8, comp_name, "renderer.material.Material")) {
                                const material = ecs.get(world_id, e, Material).?;
                                try component_list.append(.{ .name = "renderer.material.Material", .value = .{ .guid = material.guid } });
                            }
                        }
                    }
                }
                {
                    const types = ecs.get_type(world_id, e).?;
                    var tag_len: usize = 0;
                    const type_count: usize = @intCast(types.count);
                    var tags_arr = types.array;
                    for (types.array[0..type_count]) |comp| {
                        if (ecs.id_is_pair(comp)) {
                            continue;
                        }
                        if (ecs.id_is_tag(world_id, comp)) {
                            tags_arr[tag_len] = comp;
                            tag_len += 1;
                        }
                    }
                    for (tags_arr, 0..tag_len) |tag, _| {
                        if (self.world.tag_id_map.contains(tag)) {
                            const tag_name = self.world.tag_id_map.get(tag).?;
                            try tag_list.append(tag_name);
                        }
                    }
                }
                try entity_list.append(.{ .name = entity_name, .path = path, .id = e, .components = component_list.items, .tags = tag_list.items });
            }
        }
        var components = std.ArrayList([:0]const u8).init(parse_arena.allocator());
        var comp_iter = self.world.component_id_map.valueIterator();
        while (comp_iter.next()) |val| {
            try components.append(val.*);
        }
        var tags_arr = std.ArrayList([:0]const u8).init(parse_arena.allocator());
        var tags_iter = self.world.tag_id_map.valueIterator();
        while (tags_iter.next()) |val| {
            try tags_arr.append(val.*);
        }
        const world = ParseWorld{
            .entities = entity_list.items,
            .components = components.items,
            .tags = tags_arr.items,
        };
        var meshes = std.ArrayList(ParseMesh).init(parse_arena.allocator());
        for (0..self.asset.geometry_paths.items.len) |i| {
            try meshes.append(ParseMesh{
                .geometry_path = self.asset.geometry_paths.items[i],
                .material_path = self.asset.material_paths.items[i],
                .texture_path = self.asset.texture_paths.items[i],
            });
        }
        try json.stringify(ParseScene{
            .world = world,
            .meshes = meshes.items,
        }, .{}, writer);
    }
};

pub const World = struct {
    id: *ecs.world_t,
    component_id_map: std.AutoHashMap(ecs.id_t, [:0]const u8),
    tag_id_map: std.AutoHashMap(ecs.id_t, [:0]const u8),

    pub fn init(allocator: std.mem.Allocator) !World {
        const id = ecs.init();
        var component_id_map = std.AutoHashMap(ecs.id_t, [:0]const u8).init(allocator);
        try component_id_map.ensureTotalCapacity(256);
        var tag_id_map = std.AutoHashMap(ecs.id_t, [:0]const u8).init(allocator);
        try tag_id_map.ensureTotalCapacity(256);

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
        _ = ecs.set(self.id, entity, Scale, Scale{});
        return entity;
    }

    pub fn entity_new_with_parent(self: *World, parent: ecs.entity_t, name: [*:0]const u8) ecs.entity_t {
        var entity = ecs.new_w_id(self.id, ecs.pair(ecs.ChildOf, parent));
        _ = ecs.set(self.id, entity, Transform, .{});
        _ = ecs.set(self.id, entity, Scale, Scale{});
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

    pub fn entity_full_path(self: *const World, target: ecs.entity_t, from_parent: ecs.entity_t) [:0]const u8 {
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
        const casted: [:0]const u8 = @ptrCast(path[0..len]);
        return casted;
    }

    pub fn entity_delete(self: *World, target: ecs.entity_t) void {
        var iter = ecs.children(self.id, target);
        while (ecs.children_next(&iter)) {
            for (iter.entities()) |e| {
                entity_delete(self, e);
            }
        }
        ecs.delete(self.id, target);
    }

    pub fn entity_get_parent(self: *const World, target: ecs.entity_t) ecs.entity_t {
        return ecs.get_target(self.id, target, ecs.ChildOf, 0);
    }

    pub fn entity_get_parent_world_id(world: *const ecs.world_t, target: ecs.entity_t) ecs.entity_t {
        return ecs.get_target(world, target, ecs.ChildOf, 0);
    }

    pub fn deserialize(
        self: *World,
        scene_asset: *const SceneAsset,
        gctx: *zgpu.GraphicsContext,
        global_uniform_bgl: zgpu.BindGroupLayoutHandle,
        pipeline_system: *sf.PipelineSystem,
        texture_manager: *sf.TextureManager,
        material_manager: *sf.MaterialManager,
        mesh_manager: *sf.MeshManager,
        meshes: *std.ArrayList(Mesh),
        vertices: *std.ArrayList(sf.Vertex),
        indices: *std.ArrayList(u32),
    ) !ecs.entity_t {
        for (scene_asset.texture_paths.items) |texture_path| {
            try sf.TextureManager.add_texture(texture_manager, texture_path, gctx, .{ .texture_binding = true, .copy_dst = true });
        }
        const local_bgl = gctx.createBindGroupLayout(
            &.{
                zgpu.bufferEntry(0, .{ .vertex = true, .fragment = true }, .uniform, true, 0),
                zgpu.textureEntry(1, .{ .fragment = true }, .float, .tvdim_2d, false),
                zgpu.samplerEntry(2, .{ .fragment = true }, .filtering),
            },
        );
        defer gctx.releaseResource(local_bgl);
        var pipeline = try pipeline_system.add_pipeline(gctx, &.{ global_uniform_bgl, local_bgl }, false);
        // TODO: a module that parses material files (json or smth) and outputs bind group layouts to pass to pipeline system
        for (scene_asset.material_paths.items) |material_path| {
            const material_asset = material_manager.material_asset_map.get(sf.AssetManager.generate_guid(material_path)).?;
            // TODO: look into making multiple textures per material
            try sf.MaterialManager.add_material(material_manager, material_path, gctx, texture_manager, &.{
                zgpu.bufferEntry(0, .{ .vertex = true, .fragment = true }, .uniform, true, 0),
                zgpu.textureEntry(1, .{ .fragment = true }, .float, .tvdim_2d, false),
                zgpu.samplerEntry(2, .{ .fragment = true }, .filtering),
            }, @sizeOf(sf.Uniforms), material_asset.texture_guid.?);
            try pipeline_system.add_material(pipeline.*, sf.AssetManager.generate_guid(material_path));
        }
        // try self.component_add(sf.PipelineSystem);
        // ecs.add(self.id, ecs.id(sf.PipelineSystem), sf.PipelineSystem);
        // _ = ecs.set(self.id, ecs.id(sf.PipelineSystem), sf.PipelineSystem, pipeline_system.*);
        for (scene_asset.geometry_paths.items) |geometry_path| {
            _ = try sf.MeshAsset.load_mesh(geometry_path, mesh_manager, meshes, vertices, indices);
        }
        const parser_world = scene_asset.world;
        for (parser_world.components) |comp| {
            const comp_type = comps.name_type_map.get(comp).?;
            switch (comp_type) {
                .transform => {
                    try self.component_add(Transform);
                },
                .scale => {
                    try self.component_add(Scale);
                },
                .mesh => {
                    try self.component_add(Mesh);
                },
                .material => {
                    try self.component_add(Material);
                },
            }
        }
        for (parser_world.tags) |tag| {
            const tag_type = tags.name_type_map.get(tag).?;
            switch (tag_type) {
                .test_tag => {
                    try self.tag_add(TestTag);
                },
            }
        }
        const scene_entity = self.entity_new("Root");
        for (parser_world.entities) |e| {
            const entity = ecs.new_from_path_w_sep(self.id, 0, e.path, ".", null);
            for (e.tags) |tag| {
                const tag_type = tags.name_type_map.get(tag).?;
                switch (tag_type) {
                    .test_tag => {
                        const id = ecs.id(TestTag);
                        _ = ecs.add_id(self.id, entity, id);
                    },
                }
            }
            for (e.components) |comp| {
                const comp_type = comps.name_type_map.get(comp.name).?;
                switch (comp_type) {
                    .transform => {
                        _ = ecs.set(self.id, entity, Transform, .{
                            .local = zm.matFromArr(comp.value.matrix),
                        });
                    },
                    .scale => {
                        _ = ecs.set(
                            self.id,
                            entity,
                            Scale,
                            .{ .scale = .{
                                comp.value.vector[0],
                                comp.value.vector[1],
                                comp.value.vector[2],
                            } },
                        );
                    },
                    .mesh => {
                        const mesh = try mesh_manager.get_mesh(comp.value.guid);
                        _ = ecs.set(self.id, entity, Mesh, mesh);
                    },
                    .material => {
                        _ = ecs.set(self.id, entity, Material, material_manager.materials.get(comp.value.guid).?);
                    },
                }
            }
        }
        return scene_entity;
    }
};

pub const SceneManager = struct {
    arena_allocator: std.heap.ArenaAllocator,
    asset_map: std.AutoHashMap([64]u8, SceneAsset),

    pub fn init(allocator: std.mem.Allocator, config_path: []const u8) !SceneManager {
        var arena = std.heap.ArenaAllocator.init(allocator);
        var arena_alloc = arena.allocator();
        var parse_arena = std.heap.ArenaAllocator.init(allocator);
        defer parse_arena.deinit();
        const config_data = std.fs.cwd().readFileAlloc(parse_arena.allocator(), config_path, 512 * 16) catch |e| {
            log.err("Failed to parse texture config file. Given path:{s}", .{config_path});
            return e;
        };
        const Config = struct {
            database: [][:0]const u8,
        };
        const config = try json.parseFromSliceLeaky(Config, arena.allocator(), config_data, .{});
        var asset_map = std.AutoHashMap([64]u8, SceneAsset).init(arena_alloc);
        try asset_map.ensureTotalCapacity(@intCast(config.database.len));
        for (config.database) |path| {
            const scene_asset = try SceneAsset.create(arena.allocator(), parse_arena.allocator(), path);
            try asset_map.putNoClobber(scene_asset.guid, scene_asset);
        }
        return SceneManager{
            .arena_allocator = arena,
            .asset_map = asset_map,
        };
    }

    pub fn deinit(self: *SceneManager) void {
        self.arena_allocator.deinit();
    }
};
