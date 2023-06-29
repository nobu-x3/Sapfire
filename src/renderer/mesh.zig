const std = @import("std");
const texture = @import("texture.zig");
const zmesh = @import("zmesh");
const types = @import("renderer_types.zig");
const log = @import("../core/logger.zig");
const asset_manager = @import("../core/asset_manager.zig");
const json = std.json;

pub const MeshAsset = struct {
    guid: [64]u8,
    indices: std.ArrayList(u32),
    positions: std.ArrayList([3]f32),
    uvs: std.ArrayList([2]f32),
    parse_success: bool,
};

pub const MeshManager = struct {
    arena: std.heap.ArenaAllocator,
    mesh_assets_map: std.AutoHashMap([64]u8, MeshAsset),
};

pub fn mesh_manager_init(allocator: std.mem.Allocator, config_path: []const u8) !MeshManager {
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
        create_mesh_asset(arena_alloc, path, &asset_map) catch |e| {
            log.err("Failed to parse mesh at path {s}. Panicing.", .{path});
            return e;
        };
    }
    // try parse_pngs(arena_alloc, config.database, &asset_map);
    return MeshManager{
        .arena = arena,
        .mesh_assets_map = asset_map,
    };
}

pub fn mesh_manager_deinit(manager: *MeshManager) void {
    manager.arena.deinit();
}

fn create_mesh_asset(arena: std.mem.Allocator, path: [:0]const u8, out_map: *std.AutoHashMap([64]u8, MeshAsset)) !void {
    const data = zmesh.io.parseAndLoadFile(path) catch |e| {
        log.err("Error type: {s}", .{@typeName(@TypeOf(e))});
        return e;
    };
    defer zmesh.io.freeData(data);
    var indices = std.ArrayList(u32).init(arena);
    var positions = std.ArrayList([3]f32).init(arena);
    var uvs = std.ArrayList([2]f32).init(arena);
    try zmesh.io.appendMeshPrimitive(data, 0, 0, &indices, &positions, null, &uvs, null);
    const guid = asset_manager.generate_guid(path);
    const asset = MeshAsset{
        .guid = guid,
        .indices = indices,
        .positions = positions,
        .uvs = uvs,
        .parse_success = true,
    };
    try out_map.put(guid, asset);
}

pub fn mesh_manager_load_mesh(path: [:0]const u8, out_meshes: *std.ArrayList(types.Mesh), out_vertices: *std.ArrayList(types.Vertex), out_indices: *std.ArrayList(u32)) !void {
    const guid = asset_manager.generate_guid(path);
    var manager = asset_manager.mesh_manager();
    const data = manager.mesh_assets_map.get(guid) orelse {
        log.err("Mesh at path {s} is not present in the asset database. Loading failed.", .{path});
        return;
    };
    try out_meshes.append(.{
        .index_offset = @intCast(u32, out_indices.items.len),
        .vertex_offset = @intCast(i32, out_vertices.items.len),
        .num_indices = @intCast(u32, data.indices.items.len),
        .num_vertices = @intCast(u32, data.positions.items.len),
    });
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
