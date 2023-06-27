const std = @import("std");

const tex = @import("../renderer/texture.zig");
const mat = @import("../renderer/material.zig");
const crypto = std.crypto;
const json = std.json;

const AssetManager = struct {
    allocator: std.mem.Allocator,
    texture_manager: tex.TextureManager,
    material_manager: mat.MaterialManager,
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

pub fn init(
    allocator: std.mem.Allocator,
    project_config: []const u8,
) !void {
    _ = project_config;
    instance.allocator = allocator;
    instance.texture_manager = try tex.texture_system_init(allocator);
    instance.material_manager = try mat.material_system_init(allocator, 32);
}

pub fn deinit() void {
    mat.material_system_deinit(&instance.material_manager);
    tex.texture_system_deinit(&instance.texture_manager);
}

// TODO: takes in path to "project" serialization which contains paths to each subsystem's serialization file.
// Each submodule will have its own file so that project's assets can be loaded in parallel
pub fn create_asset(
    path: []const u8,
    asset_type: AssetType,
) void {
    _ = asset_type;
    var guid: [64]u8 = undefined;
    crypto.hash.sha2.Sha512.hash(path, guid[0..], .{});
    std.log.info("hash: {s}", .{guid});
}
