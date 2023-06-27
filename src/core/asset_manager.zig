const std = @import("std");

const tex = @import("../renderer/texture.zig");
const mat = @import("../renderer/material.zig");
const crypto = std.crypto;

const AssetManager = struct {
    texture_manager: *tex.TextureManager,
    material_manager: *mat.MaterialManager,
};

var instance: AssetManager = undefined;

pub fn init() void {}

pub fn deinit() void {}

// TODO: takes in path to "project" serialization which contains paths to each subsystem's serialization file.
// Each submodule will have its own file so that project's assets can be loaded in parallel
pub fn create_asset(
    path: []const u8,
) void {
    var guid: [64]u8 = undefined;
    crypto.hash.sha2.Sha512.hash(path, guid[0..], .{});
    std.log.info("hash: {s}", .{guid});
}
