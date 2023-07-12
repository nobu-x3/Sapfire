const std = @import("std");
const zmesh = @import("zmesh");
const log = @import("../core/logger.zig");
const json = std.json;
const sf = struct {
    usingnamespace @import("texture.zig");
    usingnamespace @import("renderer_types.zig");
    usingnamespace @import("../core/asset_manager.zig");
    usingnamespace @import("material.zig");
};

pub const Mesh = struct {
    index_offset: u32,
    vertex_offset: i32,
    num_indices: u32,
    num_vertices: u32,
    transform: sf.Transform = sf.Transform.init(),
    material: *sf.Material,
};

pub const MeshAsset = struct {
    guid: [64]u8,
    material_guid: [64]u8,
    indices: std.ArrayList(u32),
    positions: std.ArrayList([3]f32),
    uvs: std.ArrayList([2]f32),
    parse_success: bool,

    pub fn load_mesh(path: [:0]const u8, mesh_manager: *MeshManager, material_manager: *sf.MaterialManager, out_meshes: *std.ArrayList(Mesh), out_vertices: *std.ArrayList(sf.Vertex), out_indices: *std.ArrayList(u32), srt: sf.SRT) !void {
        const guid = sf.AssetManager.generate_guid(path);
        const data = mesh_manager.mesh_assets_map.get(guid) orelse {
            log.err("Mesh at path {s} is not present in the asset database. Loading failed.", .{path});
            return;
        };
        const material = material_manager.materials.getPtr(data.material_guid) orelse {
            log.err("Loading failed mesh at path {s} failed. Material at given path is not present in the material database.", .{path});
            return;
        };
        try out_meshes.append(.{
            .material = material,
            .index_offset = @intCast(u32, out_indices.items.len),
            .vertex_offset = @intCast(i32, out_vertices.items.len),
            .num_indices = @intCast(u32, data.indices.items.len),
            .num_vertices = @intCast(u32, data.positions.items.len),
            .transform = sf.Transform.init_from_srt(srt.position, srt.euler_angles, srt.scale),
        });
        try material_manager.add_material_to_mesh(material, &out_meshes.items[out_meshes.items.len - 1]);
        for (data.indices.items) |index| {
            try out_indices.append(index);
        }
        for (data.positions.items, 0..) |_, index| {
            try out_vertices.append(.{
                .position = data.positions.items[index],
                .uv = data.uvs.items[index],
            });
        }
    }

    pub fn load_mesh_by_guid(guid: [64]u8, mesh_manager: *MeshManager, material_manager: *sf.MaterialManager, out_meshes: *std.ArrayList(Mesh), out_vertices: *std.ArrayList(sf.Vertex), out_indices: *std.ArrayList(u32)) *Mesh {
        const data = mesh_manager.mesh_assets_map.get(guid) orelse {
            log.err("Mesh with guid {d} is not present in the asset database. Loading failed.", .{guid});
            return;
        };
        const material = material_manager.materials.getPtr(data.material_guid) orelse {
            log.err("Loading failed mesh with guid {d} failed. Material at given path is not present in the material database.", .{guid});
            return;
        };
        try out_meshes.append(.{
            .material = material,
            .index_offset = @intCast(u32, out_indices.items.len),
            .vertex_offset = @intCast(i32, out_vertices.items.len),
            .num_indices = @intCast(u32, data.indices.items.len),
            .num_vertices = @intCast(u32, data.positions.items.len),
        });
        try material_manager.add_material_to_mesh(material, &out_meshes.items[out_meshes.items.len - 1]);
        for (data.indices.items) |index| {
            try out_indices.append(index);
        }
        for (data.positions.items, 0..) |_, index| {
            try out_vertices.append(.{
                .position = data.positions.items[index],
                .uv = data.uvs.items[index],
            });
        }
    }
};

pub const MeshManager = struct {
    arena: std.heap.ArenaAllocator,
    mesh_assets_map: std.AutoHashMap([64]u8, MeshAsset),

    pub fn init(allocator: std.mem.Allocator, config_path: []const u8) !MeshManager {
        var arena = std.heap.ArenaAllocator.init(allocator);
        var arena_alloc = arena.allocator();
        var parse_arena = std.heap.ArenaAllocator.init(allocator);
        defer parse_arena.deinit();
        zmesh.init(parse_arena.allocator());
        defer zmesh.deinit();
        const config_data = std.fs.cwd().readFileAlloc(parse_arena.allocator(), config_path, 512 * 16) catch |e| {
            log.err("Failed to parse texture config file. Given path:{s}", .{config_path});
            return e;
        };
        const Config = struct {
            database: [][:0]const u8,
        };
        const config = try json.parseFromSlice(Config, arena.allocator(), config_data, .{});
        defer json.parseFree(Config, parse_arena.allocator(), config);
        var asset_map = std.AutoHashMap([64]u8, MeshAsset).init(arena_alloc);
        try asset_map.ensureTotalCapacity(@intCast(u32, config.database.len));
        for (config.database) |path| {
            create_mesh_asset(arena_alloc, parse_arena.allocator(), path, &asset_map) catch |e| {
                log.err("Failed to parse mesh at path {s}. Panicing.", .{path});
                return e;
            };
        }
        return MeshManager{
            .arena = arena,
            .mesh_assets_map = asset_map,
        };
    }

    pub fn init_from_slice(allocator: std.mem.Allocator, paths: [][:0]const u8) !MeshManager {
        var arena = std.heap.ArenaAllocator.init(allocator);
        var arena_alloc = arena.allocator();
        var parse_arena = std.heap.ArenaAllocator.init(allocator);
        defer parse_arena.deinit();
        zmesh.init(parse_arena.allocator());
        defer zmesh.deinit();
        var asset_map = std.AutoHashMap([64]u8, MeshAsset).init(arena_alloc);
        try asset_map.ensureTotalCapacity(@intCast(u32, paths.len));
        for (paths) |path| {
            create_mesh_asset(arena_alloc, parse_arena.allocator(), path, &asset_map) catch |e| {
                log.err("Failed to parse mesh at path {s}. Panicing.", .{path});
                return e;
            };
        }
        return MeshManager{
            .arena = arena,
            .mesh_assets_map = asset_map,
        };
    }

    pub fn deinit(manager: *MeshManager) void {
        manager.arena.deinit();
    }

    fn create_mesh_asset(arena: std.mem.Allocator, parse_arena: std.mem.Allocator, config_path: [:0]const u8, out_map: *std.AutoHashMap([64]u8, MeshAsset)) !void {
        const config_data = std.fs.cwd().readFileAlloc(parse_arena, config_path, 512 * 16) catch |e| {
            log.err("Failed to parse mesh config file. Given path:{s}", .{config_path});
            return e;
        };
        const Config = struct {
            geometry_path: [:0]const u8,
            material_path: [:0]const u8,
        };
        const config = try json.parseFromSlice(Config, parse_arena, config_data, .{});
        const data = zmesh.io.parseAndLoadFile(config.geometry_path) catch |e| {
            log.err("Error type: {s}", .{@typeName(@TypeOf(e))});
            return e;
        };
        defer zmesh.io.freeData(data);
        var indices = std.ArrayList(u32).init(arena);
        var positions = std.ArrayList([3]f32).init(arena);
        var uvs = std.ArrayList([2]f32).init(arena);
        try zmesh.io.appendMeshPrimitive(data, 0, 0, &indices, &positions, null, &uvs, null);
        const material_guid = sf.AssetManager.generate_guid(config.material_path);
        const guid = sf.AssetManager.generate_guid(config_path);
        const asset = MeshAsset{
            .guid = guid,
            .material_guid = material_guid,
            .indices = indices,
            .positions = positions,
            .uvs = uvs,
            .parse_success = true,
        };
        try out_map.put(guid, asset);
        log.info("Mesh at {s} added to asset map with guid\n{d}", .{ config_path, guid });
    }
};
