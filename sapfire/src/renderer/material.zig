const zgpu = @import("zgpu");
const zgui = @import("zgui");
const ecs = @import("zflecs");
const std = @import("std");
const log = @import("../core/logger.zig");
const json = std.json;
const sf = struct {
    usingnamespace @import("../core/asset_manager.zig");
    usingnamespace @import("pipeline.zig");
    usingnamespace @import("renderer_types.zig");
    usingnamespace @import("texture.zig");
    usingnamespace @import("mesh.zig");
    usingnamespace @import("renderer.zig");
};

pub const MaterialManager = struct {
    materials: std.AutoHashMap([64]u8, Material),
    material_asset_map: std.AutoHashMap([64]u8, MaterialAsset),
    arena: std.heap.ArenaAllocator,
    asset_arena: std.heap.ArenaAllocator,
    parse_arena: std.heap.ArenaAllocator,
    default_material: ?Material = null,
    path_database: std.ArrayList([:0]const u8), // TODO: serialize material_asset_map paths instead

    pub const Config = struct {
        database: [][:0]const u8,
    };

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
        try materials.ensureTotalCapacity(@intCast(config.database.len));
        var asset_map = std.AutoHashMap([64]u8, MaterialAsset).init(asset_arena.allocator());
        try asset_map.ensureTotalCapacity(@intCast(config.database.len));
        for (config.database) |path| {
            const material_asset = try MaterialAsset.create(parse_arena.allocator(), path);
            try asset_map.putNoClobber(material_asset.guid, material_asset);
            try path_database.append(path);
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

    pub fn init_from_slice(allocator: std.mem.Allocator, paths: [][:0]const u8) !MaterialManager {
        var asset_arena = std.heap.ArenaAllocator.init(allocator);
        var arena = std.heap.ArenaAllocator.init(allocator);
        var alloc = arena.allocator();
        var parse_arena = std.heap.ArenaAllocator.init(allocator);
        var path_database = std.ArrayList([:0]const u8).init(parse_arena.allocator());
        var materials = std.AutoHashMap([64]u8, Material).init(alloc);
        try materials.ensureTotalCapacity(@intCast(paths.len));
        var asset_map = std.AutoHashMap([64]u8, MaterialAsset).init(asset_arena.allocator());
        try asset_map.ensureTotalCapacity(@intCast(paths.len));
        for (paths) |path| {
            const guid = sf.AssetManager.generate_guid(path);
            if (asset_map.contains(guid)) continue;
            const material_asset = try MaterialAsset.create(parse_arena.allocator(), path);
            try asset_map.putNoClobber(material_asset.guid, material_asset);
            try path_database.append(path);
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

    pub fn add_material(system: *MaterialManager, name: [:0]const u8, gctx: *zgpu.GraphicsContext, texture_system: *sf.TextureManager, layout: []const zgpu.wgpu.BindGroupLayoutEntry, uniform_size: usize, texture_guid: [64]u8) !void {
        if (system.default_material == null) {
            system.default_material = Material.create_default(texture_system, gctx);
        }
        const guid = sf.AssetManager.generate_guid(name);
        if (!system.materials.contains(guid)) {
            var material = try Material.create(gctx, name, texture_system, layout, uniform_size, texture_guid);
            try system.materials.put(guid, material);
            log.info("Added material at path {s} with guid\n{d}", .{ name, guid });
        }
    }

    pub fn add_material_by_guid(system: *MaterialManager, guid: [64]u8, gctx: *zgpu.GraphicsContext, texture_system: *sf.TextureManager, layout: []const zgpu.wgpu.BindGroupLayoutEntry, uniform_size: usize, texture_name: [:0]const u8) !void {
        if (system.default_material == null) {
            system.default_material = Material.create_default(texture_system, gctx);
        }
        if (!system.materials.contains(guid)) {
            // TODO: material name
            var material = Material.create(gctx, texture_name, texture_system, layout, uniform_size, texture_name);
            try system.materials.putNoClobber(guid, material);
            log.info("Added material with guid\n{d}", .{guid});
        }
    }
};

pub const Material = struct {
    guid: [64]u8,
    bind_group: zgpu.BindGroupHandle,
    sampler: zgpu.SamplerHandle, // in case we need it later

    pub fn draw_inspect(self: *Material, world: *ecs.world_t, entity: ecs.entity_t, asset_manager: *sf.AssetManager) !void {
        const gctx = sf.RendererState.renderer.?.gctx;
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
            while (iter.next()) |entry| {
                if (std.mem.eql(u8, &self.guid, &entry.value_ptr.guid)) {
                    if (zgui.button(entry.value_ptr.path, .{})) {
                        zgui.openPopup("Material menu", .{});
                    }
                    break;
                }
            }
            if (zgui.beginPopup("Material menu", .{})) {
                var it = asset_manager.material_manager.material_asset_map.iterator();
                while (it.next()) |entry| {
                    if (zgui.selectable(entry.value_ptr.path, .{})) {
                        if (!scene.material_manager.materials.contains(entry.key_ptr.*)) {
                            const material = asset_manager.material_manager.materials.get(entry.value_ptr.guid) orelse val: {
                                asset_manager.material_manager.add_material(entry.value_ptr.path, gctx, &asset_manager.texture_manager, &.{
                                    zgpu.bufferEntry(0, .{ .vertex = true, .fragment = true }, .uniform, true, 0),
                                    zgpu.textureEntry(1, .{ .fragment = true }, .float, .tvdim_2d, false),
                                    zgpu.samplerEntry(2, .{ .fragment = true }, .filtering),
                                }, @sizeOf(sf.Uniforms), entry.value_ptr.texture_guid.?) catch |e| {
                                    std.log.err("Error when adding material to editor material manager. {s}.", .{@typeName(@TypeOf(e))});
                                    zgui.endPopup();
                                    return;
                                };
                                break :val asset_manager.material_manager.materials.get(entry.key_ptr.*).?;
                            };
                            scene.material_manager.materials.put(entry.key_ptr.*, material) catch |e| {
                                std.log.err("Error when adding material to scene material manager. {s}.", .{@typeName(@TypeOf(e))});
                                zgui.endPopup();
                                return;
                            };

                            const global_uniform_bgl = gctx.createBindGroupLayout(&.{
                                zgpu.bufferEntry(0, .{ .vertex = true, .fragment = true }, .uniform, true, 0),
                            });
                            defer gctx.releaseResource(global_uniform_bgl);
                            const local_bgl = gctx.createBindGroupLayout(
                                &.{
                                    zgpu.bufferEntry(0, .{ .vertex = true, .fragment = true }, .uniform, true, 0),
                                    zgpu.textureEntry(1, .{ .fragment = true }, .float, .tvdim_2d, false),
                                    zgpu.samplerEntry(2, .{ .fragment = true }, .filtering),
                                },
                            );
                            defer gctx.releaseResource(local_bgl);
                            const new_pipeline = scene.pipeline_system.add_pipeline(gctx, &.{ global_uniform_bgl, local_bgl }, false) catch |e| {
                                std.log.err("Error when adding a new pipeline. {s}.", .{@typeName(@TypeOf(e))});
                                zgui.endPopup();
                                return;
                            };
                            scene.pipeline_system.add_material(new_pipeline.*, entry.key_ptr.*) catch |e| {
                                std.log.err("Error when adding material to the newly created pipeline. {s}.", .{@typeName(@TypeOf(e))});
                                zgui.endPopup();
                                return;
                            };
                            try scene.asset.material_paths.append(entry.value_ptr.path);
                            try scene.asset.texture_paths.append(asset_manager.texture_manager.texture_assets_map.get(entry.value_ptr.texture_guid.?).?.path);
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

    // TODO: make bind group configurable
    pub fn create_tex_name(gctx: *zgpu.GraphicsContext, name: [:0]const u8, texture_system: *sf.TextureManager, layout: []const zgpu.wgpu.BindGroupLayoutEntry, uniform_size: usize, texture_name: [:0]const u8) Material {
        // bind group
        const local_bgl = gctx.createBindGroupLayout(layout);
        defer gctx.releaseResource(local_bgl);
        // Create a sampler.
        const sampler = gctx.createSampler(.{
            .address_mode_u = .repeat,
            .address_mode_v = .repeat,
            .address_mode_w = .repeat,
        });
        const local_bg = gctx.createBindGroup(local_bgl, &.{
            .{ .binding = 0, .buffer_handle = gctx.uniforms.buffer, .offset = 0, .size = uniform_size },
            .{ .binding = 1, .texture_view_handle = sf.TextureManager.get_texture_by_name(texture_system, texture_name).view },
            .{ .binding = 2, .sampler_handle = sampler },
        });
        var material = Material{
            .guid = sf.AssetManager.generate_guid(name),
            .bind_group = local_bg,
            .sampler = sampler,
        };
        return material;
    }

    pub fn create(gctx: *zgpu.GraphicsContext, name: [:0]const u8, texture_system: *sf.TextureManager, layout: []const zgpu.wgpu.BindGroupLayoutEntry, uniform_size: usize, texture_guid: [64]u8) !Material {
        const local_bgl = gctx.createBindGroupLayout(layout);
        defer gctx.releaseResource(local_bgl);
        // Create a sampler.
        const sampler = gctx.createSampler(.{
            .address_mode_u = .repeat,
            .address_mode_v = .repeat,
            .address_mode_w = .repeat,
        });
        const local_bg = gctx.createBindGroup(local_bgl, &.{
            .{ .binding = 0, .buffer_handle = gctx.uniforms.buffer, .offset = 0, .size = uniform_size },
            .{ .binding = 1, .texture_view_handle = sf.TextureManager.get_texture(texture_system, texture_guid).view },
            .{ .binding = 2, .sampler_handle = sampler },
        });
        var material = Material{
            .guid = sf.AssetManager.generate_guid(name),
            .bind_group = local_bg,
            .sampler = sampler,
        };
        return material;
    }

    fn create_default(texture_manager: *sf.TextureManager, gctx: *zgpu.GraphicsContext) Material {
        const local_bgl = gctx.createBindGroupLayout(
            &.{
                zgpu.bufferEntry(0, .{ .vertex = true, .fragment = true }, .uniform, true, 0),
                zgpu.textureEntry(1, .{ .fragment = true }, .float, .tvdim_2d, false),
                zgpu.samplerEntry(2, .{ .fragment = true }, .filtering),
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
            texture_manager.default_texture = sf.Texture.generate_default(gctx) catch |e| {
                std.log.err("Failed to generate default material because default generation failed. {s}.", .{@typeName(@TypeOf(e))});
                return;
            };
            break :val texture_manager.default_texture.?;
        };
        const local_bg = gctx.createBindGroup(local_bgl, &.{
            .{ .binding = 0, .buffer_handle = gctx.uniforms.buffer, .offset = 0, .size = @sizeOf(sf.Uniforms) },
            .{ .binding = 1, .texture_view_handle = default_texture.view },
            .{ .binding = 2, .sampler_handle = sampler },
        });
        var material = Material{
            .guid = sf.AssetManager.generate_guid("default"),
            .bind_group = local_bg,
            .sampler = sampler,
        };
        return material;
    }
};

pub const MaterialAsset = struct {
    guid: [64]u8,
    texture_guid: ?[64]u8,
    path: [:0]const u8,

    pub const Config = struct {
        texture_guid: ?[64]u8,
    };

    fn create(parse_allocator: std.mem.Allocator, path: [:0]const u8) !MaterialAsset {
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
