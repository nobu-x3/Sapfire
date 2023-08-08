const std = @import("std");
const AssetManager = @import("../core/asset_manager.zig").AssetManager;
const zmesh = @import("zmesh");
const log = @import("../core/logger.zig");
const json = std.json;
const sf = struct {
    usingnamespace @import("texture.zig");
    usingnamespace @import("renderer_types.zig");
    usingnamespace @import("../core/asset_manager.zig");
    usingnamespace @import("material.zig");
};
const Mesh = @import("../scene/components.zig").Mesh;

pub const MeshAsset = struct {
    guid: [64]u8,
    path: [:0]const u8,
    material_guid: [64]u8,
    indices: std.ArrayList(u32),
    positions: std.ArrayList([3]f32),
    uvs: std.ArrayList([2]f32),
    parse_success: bool,

    pub fn load_mesh(path: [:0]const u8, mesh_manager: *MeshManager, out_meshes: ?*std.ArrayList(Mesh), out_vertices: *std.ArrayList(sf.Vertex), out_indices: *std.ArrayList(u32)) !Mesh {
        const guid = sf.AssetManager.generate_guid(path);
        const data = mesh_manager.mesh_assets_map.get(guid) orelse {
            log.err("Mesh at path {s} is not present in the asset database. Loading failed.", .{path});
            return error.InvalidValue;
        };
        if (mesh_manager.mesh_map.contains(guid)) {
            const mesh = mesh_manager.mesh_map.get(guid).?;
            if (out_meshes != null)
                try out_meshes.?.append(mesh);
            for (data.indices.items) |index| {
                try out_indices.append(index);
            }
            for (data.positions.items, 0..) |_, index| {
                try out_vertices.append(.{
                    .position = data.positions.items[index],
                    .uv = data.uvs.items[index],
                });
            }
            return mesh;
        }
        const mesh: Mesh = .{
            .guid = guid,
            .index_offset = @intCast(out_indices.items.len),
            .vertex_offset = @intCast(out_vertices.items.len),
            .num_indices = @intCast(data.indices.items.len),
            .num_vertices = @intCast(data.positions.items.len),
        };
        try mesh_manager.mesh_map.put(guid, mesh);
        if (out_meshes != null)
            try out_meshes.?.append(mesh);
        for (data.indices.items) |index| {
            try out_indices.append(index);
        }
        for (data.positions.items, 0..) |_, index| {
            try out_vertices.append(.{
                .position = data.positions.items[index],
                .uv = data.uvs.items[index],
            });
        }
        return mesh;
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
            .index_offset = @intCast(out_indices.items.len),
            .vertex_offset = @intCast(out_vertices.items.len),
            .num_indices = @intCast(data.indices.items.len),
            .num_vertices = @intCast(data.positions.items.len),
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

const Config = struct {
    database: [][:0]const u8,
};

pub const MeshManager = struct {
    arena: std.heap.ArenaAllocator,
    parse_arena: std.heap.ArenaAllocator,
    mesh_assets_map: std.AutoHashMap([64]u8, MeshAsset),
    mesh_map: std.AutoHashMap([64]u8, Mesh),
    path_database: std.ArrayList([:0]const u8),

    pub fn init(allocator: std.mem.Allocator, config_path: []const u8) !MeshManager {
        var arena = std.heap.ArenaAllocator.init(allocator);
        var arena_alloc = arena.allocator();
        var parse_arena = std.heap.ArenaAllocator.init(allocator);
        zmesh.init(parse_arena.allocator());
        defer zmesh.deinit();
        const config_data = std.fs.cwd().readFileAlloc(parse_arena.allocator(), config_path, 512 * 16) catch |e| {
            log.err("Failed to parse texture config file. Given path:{s}", .{config_path});
            return e;
        };
        var path_database = std.ArrayList([:0]const u8).init(parse_arena.allocator());
        const config = try json.parseFromSliceLeaky(Config, arena.allocator(), config_data, .{});
        var asset_map = std.AutoHashMap([64]u8, MeshAsset).init(arena_alloc);
        try asset_map.ensureTotalCapacity(@intCast(config.database.len));
        var mesh_map = std.AutoHashMap([64]u8, Mesh).init(arena_alloc);
        try mesh_map.ensureTotalCapacity(1024);
        for (config.database) |path| {
            create_mesh_asset(arena_alloc, parse_arena.allocator(), path, &asset_map) catch |e| {
                log.err("Failed to parse mesh at path {s}. Panicing.", .{path});
                return e;
            };
            try path_database.append(path);
        }
        return MeshManager{
            .parse_arena = parse_arena,
            .arena = arena,
            .mesh_assets_map = asset_map,
            .mesh_map = mesh_map,
            .path_database = path_database,
        };
    }

    pub fn init_from_slice(allocator: std.mem.Allocator, paths: [][:0]const u8) !MeshManager {
        var arena = std.heap.ArenaAllocator.init(allocator);
        var arena_alloc = arena.allocator();
        var parse_arena = std.heap.ArenaAllocator.init(allocator);
        zmesh.init(parse_arena.allocator());
        defer zmesh.deinit();
        var asset_map = std.AutoHashMap([64]u8, MeshAsset).init(arena_alloc);
        try asset_map.ensureTotalCapacity(@intCast(paths.len));
        var mesh_map = std.AutoHashMap([64]u8, Mesh).init(arena_alloc);
        try mesh_map.ensureTotalCapacity(1024);
        var config = std.ArrayList([:0]const u8).init(parse_arena.allocator());
        for (paths) |path| {
            create_mesh_asset(arena_alloc, parse_arena.allocator(), path, &asset_map) catch |e| {
                log.err("Failed to parse mesh at path {s}. Panicing.", .{path});
                return e;
            };
            try config.append(path);
        }
        return MeshManager{
            .parse_arena = parse_arena,
            .arena = arena,
            .mesh_assets_map = asset_map,
            .mesh_map = mesh_map,
            .path_database = config,
        };
    }

    pub fn deinit(manager: *MeshManager) void {
        manager.parse_arena.deinit();
        manager.arena.deinit();
    }

    const MeshConfig = struct {
        geometry_path: [:0]const u8,
        material_path: [:0]const u8,
    };

    fn create_mesh_asset(arena: std.mem.Allocator, parse_arena: std.mem.Allocator, config_path: [:0]const u8, out_map: *std.AutoHashMap([64]u8, MeshAsset)) !void {
        const config_data = std.fs.cwd().readFileAlloc(parse_arena, config_path, 512 * 16) catch |e| {
            log.err("Failed to parse mesh config file. Given path:{s}", .{config_path});
            return e;
        };
        const config = try json.parseFromSliceLeaky(MeshConfig, parse_arena, config_data, .{});
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
        var path_cpy = try arena.allocSentinel(u8, config_path.len, 0);
        @memcpy(path_cpy, config_path);
        const asset = MeshAsset{
            .path = path_cpy,
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

    pub fn import_mesh(self: *MeshManager, path: [:0]const u8) !void {
        zmesh.init(self.parse_arena.allocator());
        defer zmesh.deinit();
        const data = zmesh.io.parseAndLoadFile(path) catch |e| {
            log.err("Error type: {s}", .{@typeName(@TypeOf(e))});
            return e;
        };
        defer zmesh.io.freeData(data);
        var indices = std.ArrayList(u32).init(self.arena.allocator());
        var positions = std.ArrayList([3]f32).init(self.arena.allocator());
        var uvs = std.ArrayList([2]f32).init(self.arena.allocator());
        try zmesh.io.appendMeshPrimitive(data, 0, 0, &indices, &positions, null, &uvs, null);
        var iter = AssetManager.material_manager().material_asset_map.iterator();
        const material_asset = iter.next().?.value_ptr;
        const material_guid = material_asset.guid;
        var split = std.mem.splitAny(u8, path, "/");
        var name = split.first();
        while (split.peek() != null) {
            name = split.rest();
            _ = split.next();
        }
        const mesh_json_path = try std.mem.concatWithSentinel(self.parse_arena.allocator(), u8, &.{ "project/meshes/", name, ".json" }, 0);
        const guid = sf.AssetManager.generate_guid(mesh_json_path);
        var path_cpy = try self.arena.allocator().allocSentinel(u8, mesh_json_path.len, 0);
        @memcpy(path_cpy, mesh_json_path);
        const asset = MeshAsset{
            .path = path_cpy,
            .guid = guid,
            .material_guid = material_guid,
            .indices = indices,
            .positions = positions,
            .uvs = uvs,
            .parse_success = true,
        };
        try self.mesh_assets_map.put(guid, asset);
        try self.path_database.append(mesh_json_path);
        { // new mesh .json
            var file = try std.fs.cwd().createFile(mesh_json_path, .{});
            defer file.close();
            var writer = file.writer();
            try json.stringify(MeshConfig{ .geometry_path = path, .material_path = material_asset.path }, .{}, writer);
        }
        { // serialize mesh_config.json
            var file = try std.fs.cwd().createFile("project/mesh_config.json", .{});
            defer file.close();
            var writer = file.writer();
            try json.stringify(Config{ .database = self.path_database.items }, .{}, writer);
        }
    }

    pub fn get_mesh(self: *MeshManager, guid: [64]u8) !Mesh {
        return self.mesh_map.get(guid) orelse {
            return error.InvalidValue;
        };
    }
};
