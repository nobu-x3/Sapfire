const std = @import("std");
const json = std.json;
const sf = struct {
    usingnamespace @import("../core/asset_manager.zig");
    usingnamespace @import("renderer_types.zig");
};
const log = @import("../core/logger.zig");

const SceneManager = struct {};

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
