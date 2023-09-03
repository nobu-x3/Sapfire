const zgpu = @import("zgpu");
const zgui = @import("zgui");
const ecs = @import("zflecs");
const std = @import("std");
const json = std.json;
const sf = struct {
    usingnamespace @import("../core.zig");
    usingnamespace @import("pipeline.zig");
    usingnamespace @import("buffer.zig");
    usingnamespace @import("renderer_types.zig");
    usingnamespace @import("texture.zig");
    usingnamespace @import("mesh.zig");
    usingnamespace @import("renderer.zig");
};
const log = sf.log;

pub const MaterialManager = struct {
    materials: std.AutoHashMap([64]u8, Material),
    material_asset_map: std.AutoHashMap([64]u8, MaterialAsset),
    arena: std.heap.ArenaAllocator,
    asset_arena: std.heap.ArenaAllocator,
    parse_arena: std.heap.ArenaAllocator,
    default_material: ?Material = null,
    path_database: std.ArrayList([:0]const u8), // TODO: serialize material_asset_map paths instead

    pub const Config = struct {
        database: [][]const u8,
    };

    const INIT_ASSET_MAP_SIZE = 16;

    pub fn init_empty(allocator: std.mem.Allocator) !MaterialManager {
        var asset_arena = std.heap.ArenaAllocator.init(allocator);
        var arena = std.heap.ArenaAllocator.init(allocator);
        var alloc = arena.allocator();
        var parse_arena = std.heap.ArenaAllocator.init(allocator);
        var path_database = std.ArrayList([:0]const u8).init(parse_arena.allocator());
        var materials = std.AutoHashMap([64]u8, Material).init(alloc);
        try materials.ensureTotalCapacity(INIT_ASSET_MAP_SIZE);
        var asset_map = std.AutoHashMap([64]u8, MaterialAsset).init(asset_arena.allocator());
        try asset_map.ensureTotalCapacity(INIT_ASSET_MAP_SIZE);
        return MaterialManager{
            .parse_arena = parse_arena,
            .materials = materials,
            .arena = arena,
            .material_asset_map = asset_map,
            .asset_arena = asset_arena,
            .path_database = path_database,
        };
    }

    // TODO: parse config file
    pub fn init(allocator: std.mem.Allocator, config_path: []const u8) !MaterialManager {
        var asset_arena = std.heap.ArenaAllocator.init(allocator);
        var arena = std.heap.ArenaAllocator.init(allocator);
        var alloc = arena.allocator();
        var parse_arena = std.heap.ArenaAllocator.init(allocator);
        var path_database = std.ArrayList([:0]const u8).init(parse_arena.allocator());
        const config_data = std.fs.cwd().readFileAlloc(parse_arena.allocator(), config_path, 512 * 16) catch |e| {
            log.err("Failed to parse material manager config file. Given path:{s}", .{config_path});
            return e;
        };
        const config = try json.parseFromSliceLeaky(Config, arena.allocator(), config_data, .{});
        var materials = std.AutoHashMap([64]u8, Material).init(alloc);
        try materials.ensureTotalCapacity(INIT_ASSET_MAP_SIZE);
        var asset_map = std.AutoHashMap([64]u8, MaterialAsset).init(asset_arena.allocator());
        try asset_map.ensureTotalCapacity(INIT_ASSET_MAP_SIZE);
        for (config.database) |path| {
            const material_asset = try MaterialAsset.create(parse_arena.allocator(), path);
            try asset_map.putNoClobber(material_asset.guid, material_asset);
            var path_sen = try std.mem.concatWithSentinel(parse_arena.allocator(), u8, &.{path}, 0);
            try path_database.append(path_sen);
        }
        return MaterialManager{
            .parse_arena = parse_arena,
            .materials = materials,
            .arena = arena,
            .material_asset_map = asset_map,
            .asset_arena = asset_arena,
            .path_database = path_database,
        };
    }

    pub fn init_from_slice(allocator: std.mem.Allocator, paths: [][]const u8) !MaterialManager {
        var asset_arena = std.heap.ArenaAllocator.init(allocator);
        var arena = std.heap.ArenaAllocator.init(allocator);
        var alloc = arena.allocator();
        var parse_arena = std.heap.ArenaAllocator.init(allocator);
        var path_database = std.ArrayList([:0]const u8).init(parse_arena.allocator());
        var materials = std.AutoHashMap([64]u8, Material).init(alloc);
        try materials.ensureTotalCapacity(INIT_ASSET_MAP_SIZE);
        var asset_map = std.AutoHashMap([64]u8, MaterialAsset).init(asset_arena.allocator());
        try asset_map.ensureTotalCapacity(INIT_ASSET_MAP_SIZE);
        for (paths) |path| {
            const guid = sf.AssetManager.generate_guid(path);
            if (asset_map.contains(guid)) continue;
            const material_asset = try MaterialAsset.create(parse_arena.allocator(), path);
            try asset_map.putNoClobber(material_asset.guid, material_asset);
            var path_sen = try std.mem.concatWithSentinel(parse_arena.allocator(), u8, &.{path}, 0);
            try path_database.append(path_sen);
        }
        return MaterialManager{
            .parse_arena = parse_arena,
            .materials = materials,
            .arena = arena,
            .material_asset_map = asset_map,
            .asset_arena = asset_arena,
            .path_database = path_database,
        };
    }

    pub fn deinit(system: *MaterialManager) void {
        system.path_database.deinit();
        system.parse_arena.deinit();
        system.asset_arena.deinit();
        system.arena.deinit();
    }

    pub fn add_material(system: *MaterialManager, name: []const u8, gctx: *zgpu.GraphicsContext, texture_system: *sf.TextureManager, layout: []const zgpu.wgpu.BindGroupLayoutEntry, uniform_size: usize, texture_guid: [64]u8) !void {
        if (system.default_material == null) {
            try Material.create_default(system, texture_system, gctx);
        }
        const guid = sf.AssetManager.generate_guid(name);
        if (!system.materials.contains(guid)) {
            var material = try Material.create(gctx, .{
                .ambient = 1.0,
                .diffuse = 0.0,
                .reflection = 0.0,
            }, name, texture_system, layout, uniform_size, texture_guid);
            try system.materials.put(guid, material);
            log.info("Added material at path {s} with guid\n{d}", .{ name, guid });
        }
    }
};

pub const Material = struct {
    guid: [64]u8,
    bind_group: zgpu.BindGroupHandle,
    buffer: sf.Buffer,
    sampler: zgpu.SamplerHandle, // in case we need it later
    phong_data: sf.PhongData,

    pub fn draw_inspect(self: *Material, world: *ecs.world_t, entity: ecs.entity_t, asset_manager: *sf.AssetManager) !void {
        const gctx = sf.Renderer.renderer.?.gctx;
        zgui.text("Material:", .{});
        if (zgui.button("Mat. Options", .{})) {
            zgui.openPopup("Material Component Context", .{});
        }
        if (zgui.beginPopup("Material Component Context", .{})) {
            if (zgui.selectable("Delete", .{})) {
                ecs.remove(world, entity, Material);
                zgui.closeCurrentPopup();
            }
            zgui.endPopup();
        }
        const maybe_scene = @import("../scene/scene.zig").Scene.scene;
        if (maybe_scene) |scene| {
            var iter = asset_manager.material_manager.material_asset_map.iterator();
            var shown = false;
            while (iter.next()) |entry| {
                if (std.mem.eql(u8, &self.guid, &entry.value_ptr.guid)) {
                    shown = true;
                    if (zgui.button(entry.value_ptr.path, .{})) {
                        zgui.openPopup("Material menu", .{});
                    }
                    break;
                }
            }
            if (!shown) {
                if (zgui.button("default", .{})) {
                    zgui.openPopup("Material menu", .{});
                }
            }
            if (zgui.beginPopup("Material menu", .{})) {
                var it = asset_manager.material_manager.material_asset_map.iterator();
                while (it.next()) |entry| {
                    if (zgui.selectable(entry.value_ptr.path, .{})) {
                        const old_mat = ecs.get(world, entity, Material);
                        if (!scene.material_manager.materials.contains(entry.key_ptr.*)) {
                            if (entry.value_ptr.texture_guid) |text_guid| {
                                var tex_entry = try scene.texture_manager.textures.getOrPut(text_guid);
                                if (!tex_entry.found_existing) {
                                    tex_entry.value_ptr.* = asset_manager.texture_manager.textures.get(text_guid) orelse val: {
                                        var asset = asset_manager.texture_manager.texture_assets_map.getPtr(text_guid).?;
                                        try asset_manager.texture_manager.add_texture(asset.path, gctx, .{ .texture_binding = true, .copy_dst = true });
                                        break :val asset_manager.texture_manager.textures.get(text_guid).?;
                                    };
                                }
                            }
                            const material = asset_manager.material_manager.materials.getPtr(entry.value_ptr.guid) orelse val: {
                                asset_manager.material_manager.add_material(entry.value_ptr.path, gctx, &asset_manager.texture_manager, &.{
                                    zgpu.textureEntry(0, .{ .fragment = true }, .float, .tvdim_2d, false),
                                    zgpu.samplerEntry(1, .{ .fragment = true }, .filtering),
                                    zgpu.bufferEntry(2, .{ .fragment = true }, .uniform, false, @sizeOf(sf.PhongData)),
                                }, @sizeOf(sf.PhongData), entry.value_ptr.texture_guid.?) catch |e| {
                                    std.log.err("Error when adding material to editor material manager. {s}.", .{@typeName(@TypeOf(e))});
                                    zgui.endPopup();
                                    return;
                                };
                                break :val asset_manager.material_manager.materials.getPtr(entry.key_ptr.*).?;
                            };
                            scene.material_manager.materials.putAssumeCapacity(entry.key_ptr.*, material.*);
                            const global_uniform_bgl = gctx.createBindGroupLayout(&.{
                                zgpu.bufferEntry(0, .{ .vertex = true, .fragment = true }, .uniform, true, 0),
                            });
                            defer gctx.releaseResource(global_uniform_bgl);
                            const local_bgl = gctx.createBindGroupLayout(
                                &.{
                                    zgpu.textureEntry(0, .{ .fragment = true }, .float, .tvdim_2d, false),
                                    zgpu.samplerEntry(1, .{ .fragment = true }, .filtering),
                                    zgpu.bufferEntry(2, .{ .fragment = true }, .uniform, true, 0),
                                },
                            );
                            defer gctx.releaseResource(local_bgl);
                            var new_pipeline: sf.Pipeline = .{};
                            scene.pipeline_system.add_pipeline(gctx, &.{ global_uniform_bgl, local_bgl }, false, &new_pipeline.handle) catch |e| {
                                std.log.err("Error when adding a new pipeline. {s}.", .{@typeName(@TypeOf(e))});
                                zgui.endPopup();
                                return;
                            };
                            scene.pipeline_system.add_material(&new_pipeline, entry.key_ptr.*) catch |e| {
                                std.log.err("Error when adding material to the newly created pipeline. {s}.", .{@typeName(@TypeOf(e))});
                                zgui.endPopup();
                                return;
                            };
                            if (!std.mem.eql(u8, entry.value_ptr.path, "default")) {
                                const mat_comp = ecs.get(world, entity, Material);
                                if (mat_comp) |mat| {
                                    const old_path = asset_manager.material_manager.material_asset_map.get(mat.guid).?.path;
                                    try scene.asset.add_asset_path(.Material, entry.value_ptr.path, old_path);
                                } else {
                                    try scene.asset.add_asset_path(.Material, entry.value_ptr.path, null);
                                }
                            }
                            if (entry.value_ptr.texture_guid) |texture_guid| {
                                const maybe_old_mat_asset = scene.material_manager.material_asset_map.get(old_mat.?.guid);
                                try scene.asset.add_asset_path(
                                    .Texture,
                                    asset_manager.texture_manager.texture_assets_map.get(texture_guid).?.path,
                                    if (maybe_old_mat_asset != null) maybe_old_mat_asset.?.path else null,
                                );
                            }
                        } else {
                            const material = ecs.get(world, entity, Material).?;
                            const material_asset = asset_manager.material_manager.material_asset_map.get(material.guid).?;
                            try scene.asset.add_asset_path(.Mesh, entry.value_ptr.path, material_asset.path);
                        }
                        _ = ecs.set(world, entity, Material, scene.material_manager.materials.get(entry.key_ptr.*).?);
                    }
                }
                zgui.endPopup();
            }
        }
        zgui.spacing();
        zgui.separator();
        zgui.dummy(.{ .h = 5, .w = 0 });
    }

    pub fn create(gctx: *zgpu.GraphicsContext, phong_data: sf.PhongData, name: []const u8, texture_system: *sf.TextureManager, layout: []const zgpu.wgpu.BindGroupLayoutEntry, uniform_size: usize, texture_guid: [64]u8) !Material {
        const local_bgl = gctx.createBindGroupLayout(layout);
        defer gctx.releaseResource(local_bgl);
        // Create a sampler.
        const sampler = gctx.createSampler(.{
            .address_mode_u = .repeat,
            .address_mode_v = .repeat,
            .address_mode_w = .repeat,
        });
        const buffer = sf.Buffer.create_and_load(gctx, .{ .uniform = true, .copy_dst = true }, sf.PhongData, &.{phong_data});
        const local_bg = gctx.createBindGroup(
            local_bgl,
            &.{
                .{
                    .binding = 0,
                    .texture_view_handle = sf.TextureManager.get_texture(texture_system, texture_guid).view,
                },
                .{ .binding = 1, .sampler_handle = sampler },
                .{
                    .binding = 2,
                    .buffer_handle = buffer.handle,
                    .offset = 0,
                    .size = uniform_size,
                },
            },
        );
        var material = Material{
            .phong_data = phong_data,
            .guid = sf.AssetManager.generate_guid(name),
            .bind_group = local_bg,
            .sampler = sampler,
            .buffer = buffer,
        };
        return material;
    }

    pub fn create_default(material_manager: *MaterialManager, texture_manager: *sf.TextureManager, gctx: *zgpu.GraphicsContext) !void {
        const local_bgl = gctx.createBindGroupLayout(
            &.{
                zgpu.textureEntry(0, .{ .fragment = true }, .float, .tvdim_2d, false),
                zgpu.samplerEntry(1, .{ .fragment = true }, .filtering),
                zgpu.bufferEntry(2, .{ .fragment = true }, .uniform, true, 0),
            },
        );
        defer gctx.releaseResource(local_bgl);
        // Create a sampler.
        const sampler = gctx.createSampler(.{
            .address_mode_u = .repeat,
            .address_mode_v = .repeat,
            .address_mode_w = .repeat,
        });
        const default_texture = texture_manager.default_texture orelse val: {
            sf.Texture.generate_default(gctx, texture_manager) catch |e| {
                std.log.err("Failed to generate default material because default generation failed. {s}.", .{@typeName(@TypeOf(e))});
                return;
            };
            break :val texture_manager.default_texture.?;
        };
        const buffer = sf.Buffer.create_and_load(gctx, .{ .uniform = true, .copy_dst = true }, sf.PhongData, &.{
            .{
                .ambient = 1.0,
                .diffuse = 0.0,
                .reflection = 0.0,
            },
        });
        const local_bg = gctx.createBindGroup(local_bgl, &.{
            .{
                .binding = 0,
                .texture_view_handle = default_texture.view,
            },
            .{ .binding = 1, .sampler_handle = sampler },
            .{
                .binding = 2,
                .buffer_handle = buffer.handle,
                .offset = 0,
                .size = @sizeOf(sf.PhongData),
            },
        });
        const guid = sf.AssetManager.generate_guid("default");
        const default_phong = sf.PhongData{
            .ambient = 1.0,
            .diffuse = 0.0,
            .reflection = 0.0,
        };
        material_manager.default_material = Material{
            .phong_data = default_phong,
            .guid = guid,
            .bind_group = local_bg,
            .sampler = sampler,
            .buffer = buffer,
        };
        try material_manager.materials.put(guid, material_manager.default_material.?);
        const material_asset = MaterialAsset{
            .guid = guid,
            .path = "default",
        };
        try material_manager.material_asset_map.put(guid, material_asset);
    }
};

pub const MaterialAsset = struct {
    guid: [64]u8,
    texture_guid: ?[64]u8 = null,
    path: [:0]const u8,

    pub const Config = struct {
        texture_guid: ?[64]u8,
    };

    fn create(parse_allocator: std.mem.Allocator, path: []const u8) !MaterialAsset {
        const material_guid = sf.AssetManager.generate_guid(path);
        const config_data = std.fs.cwd().readFileAlloc(parse_allocator, path, 512 * 16) catch |e| {
            log.err("Failed to parse material config file. Given path:{s}", .{path});
            return e;
        };
        const config = try json.parseFromSliceLeaky(Config, parse_allocator, config_data, .{});
        var path_cpy = try parse_allocator.allocSentinel(u8, path.len, 0);
        @memcpy(path_cpy, path);
        std.log.info("{s}", .{path_cpy});
        return MaterialAsset{
            .path = path_cpy,
            .guid = material_guid,
            .texture_guid = config.texture_guid,
        };
    }
};

const DEFAULT_MESH_LIST_CAPACITY = 8;
