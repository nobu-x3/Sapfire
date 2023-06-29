const std = @import("std");
const log = @import("logger.zig");
const crypto = std.crypto;
const json = std.json;

const sf = struct {
    usingnamespace @import("../renderer/texture.zig");
    usingnamespace @import("../renderer/mesh.zig");
    usingnamespace @import("../renderer/material.zig");
};

pub const AssetManager = struct {
    allocator: std.mem.Allocator,
    texture_manager: sf.TextureManager,
    material_manager: sf.MaterialManager,
    mesh_manager: sf.MeshManager,

    pub fn texture_manager() *sf.TextureManager {
        return &instance.texture_manager;
    }

    pub fn material_manager() *sf.MaterialManager {
        return &instance.material_manager;
    }

    pub fn mesh_manager() *sf.MeshManager {
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
        instance.texture_manager = try sf.texture_manager_init(allocator, config.texture_config);
        instance.mesh_manager = try sf.MeshManager.init(allocator, config.mesh_config);
        instance.material_manager = try sf.material_manager_init(allocator, config.material_config);
    }

    pub fn deinit() void {
        sf.material_manager_deinit(&instance.material_manager);
        sf.MeshManager.deinit(&instance.mesh_manager);
        sf.texture_manager_deinit(&instance.texture_manager);
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
};

const AssetType = enum {
    Texture,
    Material,
    Mesh,
};

var instance: AssetManager = undefined;
