const std = @import("std");
const log = @import("logger.zig");
const tex = @import("../renderer/texture.zig");
const mesh = @import("../renderer/mesh.zig");
const mat = @import("../renderer/material.zig");
const crypto = std.crypto;
const json = std.json;

const AssetManager = struct {
    allocator: std.mem.Allocator,
    texture_manager: tex.TextureManager,
    material_manager: mat.MaterialManager,
    mesh_manager: mesh.MeshManager,
};

const AssetType = enum {
    Texture,
    Material,
    Mesh,
};

var instance: AssetManager = undefined;

pub fn texture_manager() *tex.TextureManager {
    return &instance.texture_manager;
}

pub fn material_manager() *mat.MaterialManager {
    return &instance.material_manager;
}

pub fn mesh_manager() *mesh.MeshManager {
    return &instance.mesh_manager;
}

// TODO: takes in path to "project" serialization which contains paths to each subsystem's serialization file.
// Each submodule will have its own file so that project's assets can be loaded in parallel
pub fn init(
    allocator: std.mem.Allocator,
    project_config: []const u8,
) !void {
    instance.allocator = allocator;
    var arena = std.heap.ArenaAllocator.init(allocator);
    defer arena.deinit();
    const config_data = std.fs.cwd().readFileAlloc(arena.allocator(), project_config, 512 * 16) catch |e| {
        log.err("Failed to parse project config file.", .{});
        return e;
    };
    const Config = struct {
        texture_config: []const u8,
        material_config: []const u8,
        mesh_config: []const u8,
    };
    const config = try json.parseFromSlice(Config, arena.allocator(), config_data, .{});
    defer json.parseFree(Config, arena.allocator(), config);
    instance.texture_manager = try tex.texture_manager_init(allocator, config.texture_config);
    instance.mesh_manager = try mesh.mesh_manager_init(allocator, config.mesh_config);
    instance.material_manager = try mat.material_system_init(allocator, 32);
}

pub fn deinit() void {
    mat.material_system_deinit(&instance.material_manager);
    mesh.mesh_manager_deinit(&instance.mesh_manager);
    tex.texture_manager_deinit(&instance.texture_manager);
}

// TODO: this should be used to import raw files and generate .sf* format assets
pub fn create_asset(
    path: []const u8,
    asset_type: AssetType,
) void {
    _ = asset_type;
    var guid: [64]u8 = undefined;
    crypto.hash.sha2.Sha512.hash(path, guid[0..], .{});
    std.log.info("hash: {s}", .{guid});
}

pub fn generate_guid(path: [:0]const u8) [64]u8 {
    var guid: [64]u8 = undefined;
    crypto.hash.sha2.Sha512.hash(path, &guid, .{});
    return guid;
}
