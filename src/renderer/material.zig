const zgpu = @import("zgpu");
const std = @import("std");
const log = @import("../core/logger.zig");
const json = std.json;
const sf = struct {
    usingnamespace @import("../core/asset_manager.zig");
    usingnamespace @import("pipeline.zig");
    usingnamespace @import("renderer_types.zig");
    usingnamespace @import("texture.zig");
};

pub const MaterialManager = struct {
    map: std.AutoArrayHashMap(Material, std.ArrayList(*sf.Mesh)),
    materials: std.AutoHashMap([64]u8, Material),
    materias_asset_map: std.AutoHashMap([64]u8, MaterialAsset),
    arena: std.heap.ArenaAllocator,
    asset_arena: std.heap.ArenaAllocator,
    default_material: ?Material = null,

    // TODO: parse config file
    pub fn init(allocator: std.mem.Allocator, config_path: []const u8) !MaterialManager {
        var asset_arena = std.heap.ArenaAllocator.init(allocator);
        var arena = std.heap.ArenaAllocator.init(allocator);
        var alloc = arena.allocator();
        var parse_arena = std.heap.ArenaAllocator.init(allocator);
        defer parse_arena.deinit();
        const config_data = std.fs.cwd().readFileAlloc(parse_arena.allocator(), config_path, 512 * 16) catch |e| {
            log.err("Failed to parse material manager config file. Given path:{s}", .{config_path});
            return e;
        };
        const Config = struct {
            database: [][:0]const u8,
        };
        const config = try json.parseFromSlice(Config, arena.allocator(), config_data, .{});
        defer json.parseFree(Config, parse_arena.allocator(), config);
        var map = std.AutoArrayHashMap(Material, std.ArrayList(*sf.Mesh)).init(alloc);
        try map.ensureTotalCapacity(@intCast(u32, config.database.len));
        var materials = std.AutoHashMap([64]u8, Material).init(alloc);
        try materials.ensureTotalCapacity(@intCast(u32, config.database.len));
        var asset_map = std.AutoHashMap([64]u8, MaterialAsset).init(asset_arena.allocator());
        try asset_map.ensureTotalCapacity(@intCast(u32, config.database.len));
        for (config.database) |path| {
            const material_asset = try MaterialAsset.create(parse_arena.allocator(), path);
            try asset_map.putNoClobber(material_asset.guid, material_asset);
        }
        return MaterialManager{
            .map = map,
            .materials = materials,
            .arena = arena,
            .materias_asset_map = asset_map,
            .asset_arena = asset_arena,
        };
    }

    pub fn deinit(system: *MaterialManager) void {
        // system.materials.deinit();
        // var iter = system.map.iterator();
        // while (iter.next()) |entry| {
        //     entry.value_ptr.deinit();
        // }
        // system.map.deinit();
        system.asset_arena.deinit();
        system.arena.deinit();
    }

    pub fn add_material(system: *MaterialManager, name: [:0]const u8, gctx: *zgpu.GraphicsContext, texture_system: *sf.TextureManager, layout: []const zgpu.wgpu.BindGroupLayoutEntry, uniform_size: usize, texture_name: [:0]const u8) !void {
        if (system.default_material == null) {
            system.default_material = Material.create_default(gctx);
        }
        const guid = sf.AssetManager.generate_guid(name);
        if (!system.materials.contains(guid)) {
            var arena = system.arena.allocator();
            var material = Material.create(gctx, texture_system, layout, uniform_size, texture_name);
            try system.materials.putNoClobber(guid, material);
            var meshes = try std.ArrayList(*sf.Mesh).initCapacity(arena, DEFAULT_MESH_LIST_CAPACITY);
            try system.map.putNoClobber(material, meshes);
            log.info("Added material at path {s} with guid\n{d}", .{ name, guid });
        }
    }

    pub fn add_material_to_mesh(system: *MaterialManager, material: *Material, mesh: *sf.Mesh) !void {
        var list = system.map.getPtr(material.*);
        try list.?.append(mesh);
    }

    pub fn add_material_to_mesh_by_name(system: *MaterialManager, name: [:0]const u8, mesh: *sf.Mesh) !void {
        const guid = sf.AssetManager.generate_guid(name);
        const material = system.materials.getPtr(guid).?;
        try add_material_to_mesh(system, material, mesh);
    }
};

pub const Material = struct {
    bind_group: zgpu.BindGroupHandle,
    sampler: zgpu.SamplerHandle, // in case we need it later

    // TODO: make bind group configurable
    pub fn create(gctx: *zgpu.GraphicsContext, texture_system: *sf.TextureManager, layout: []const zgpu.wgpu.BindGroupLayoutEntry, uniform_size: usize, texture_name: [:0]const u8) Material {
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
            .bind_group = local_bg,
            .sampler = sampler,
        };
        return material;
    }

    fn create_default(gctx: *zgpu.GraphicsContext) Material {
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
        const local_bg = gctx.createBindGroup(local_bgl, &.{
            .{ .binding = 0, .buffer_handle = gctx.uniforms.buffer, .offset = 0, .size = @sizeOf(sf.Uniforms) },
            .{ .binding = 1, .texture_view_handle = sf.AssetManager.texture_manager().default_texture.?.view },
            .{ .binding = 2, .sampler_handle = sampler },
        });
        var material = Material{
            .bind_group = local_bg,
            .sampler = sampler,
        };
        return material;
    }
};

pub const MaterialAsset = struct {
    guid: [64]u8,
    texture_guid: ?[64]u8,

    fn create(parse_allocator: std.mem.Allocator, path: [:0]const u8) !MaterialAsset {
        const material_guid = sf.AssetManager.generate_guid(path);
        const config_data = std.fs.cwd().readFileAlloc(parse_allocator, path, 512 * 16) catch |e| {
            log.err("Failed to parse material config file. Given path:{s}", .{path});
            return e;
        };
        const Config = struct {
            texture_guid: ?[64]u8,
        };
        const config = try json.parseFromSlice(Config, parse_allocator, config_data, .{});
        defer json.parseFree(Config, parse_allocator, config);
        // const texture = tex.texture_manager_get_texture(asset_manager.texture_manager(), config.texture_guid.?);
        return MaterialAsset{
            .guid = material_guid,
            .texture_guid = config.texture_guid,
        };
    }
};

const DEFAULT_MESH_LIST_CAPACITY = 8;
