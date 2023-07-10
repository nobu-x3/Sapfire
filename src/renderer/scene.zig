const std = @import("std");
const json = std.json;
const sf = struct {
    usingnamespace @import("../core/asset_manager.zig");
    usingnamespace @import("../renderer/mesh.zig");
    usingnamespace @import("renderer_types.zig");
};
const log = @import("../core/logger.zig");

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
        const config = try json.parseFromSlice(Config, arena.allocator(), config_data, .{});
        defer json.parseFree(Config, parse_arena.allocator(), config);
        var asset_map = std.AutoHashMap([64]u8, SceneAsset).init(arena_alloc);
        try asset_map.ensureTotalCapacity(@intCast(u32, config.database.len));
        for (config.database) |path| {
            const scene_asset = try SceneAsset.create(parse_arena.allocator(), path);
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

pub const SimpleScene = struct {
    guid: [64]u8,
    meshes: std.ArrayList(sf.Mesh),
    vertices: std.ArrayList(sf.Vertex),
    indices: std.ArrayList(u32),
    arena: std.heap.ArenaAllocator,

    pub fn create(allocator: std.mem.Allocator, config_path: [:0]const u8) SimpleScene {
        const scene_guid = sf.AssetManager.generate_guid(config_path);
        const sceneman = sf.AssetManager.scene_manager();
        const arena = std.heap.ArenaAllocator.init(allocator);
        var meshes = std.ArrayList(sf.Mesh).init(arena.allocator());
        try meshes.ensureTotalCapacity(128);
        var vertices = std.ArrayList(sf.Vertex).init(arena.allocator());
        defer vertices.deinit();
        try vertices.ensureTotalCapacity(256);
        var indices = std.ArrayList(u32).init(arena.allocator());
        defer indices.deinit();
        try indices.ensureTotalCapacity(256);
        const asset = try sceneman.asset_map.get(scene_guid);
        for (asset.mesh_guids.items) |guid| {
            sf.MeshAsset.load_mesh_by_guid(guid, &meshes, &vertices, &indices);
        }
        return SimpleScene{
            .guid = scene_guid,
            .meshes = meshes,
            .vertices = vertices,
            .indices = indices,
            .arena = arena,
        };
    }

    pub fn destroy(scene: *SimpleScene) void {
        scene.arena.deinit();
    }
};

const SceneAsset = struct {
    guid: [64]u8,
    mesh_guids: [][64]u8,

    fn create(parse_allocator: std.mem.Allocator, path: [:0]const u8) !SceneAsset {
        const scene_guid = sf.AssetManager.generate_guid(path);
        const config_data = std.fs.cwd().readFileAlloc(parse_allocator, path, 512 * 16) catch |e| {
            log.err("Failed to parse material config file. Given path:{s}", .{path});
            return e;
        };
        const Config = struct {
            mesh_guids: [][64]u8,
        };
        const config = try json.parseFromSlice(Config, parse_allocator, config_data, .{});
        defer json.parseFree(Config, parse_allocator, config);
        return SceneAsset{
            .guid = scene_guid,
            .mesh_guids = config.mesh_guids,
        };
    }
};
