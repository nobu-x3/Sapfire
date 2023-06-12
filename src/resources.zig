const std = @import("std");
const zgpu = @import("zgpu");
const texture = @import("texture.zig");
const zmesh = @import("zmesh");
const Mesh = @import("renderer.zig").Mesh;
const Vertex = @import("renderer.zig").Vertex;

pub fn resources_generate_default_texture(gctx: *zgpu.GraphicsContext) !texture.Texture {
    // Generate default texture
    @setEvalBranchQuota(256 * 256 * 4);
    const dimensions: u32 = 256;
    const channels: u32 = 4;
    const byte_count: u32 = dimensions * dimensions * channels;
    var pixels = [_]u8{255} ** byte_count;
    inline for (0..dimensions) |row| {
        inline for (0..dimensions) |col| {
            const index = ((row * dimensions) + col) * channels;
            if (row % 2 != 0) {
                if (col % 2 != 0) {
                    pixels[index + 1] = 0;
                } else {
                    pixels[index] = 0;
                    pixels[index + 1] = 0;
                    pixels[index + 2] = 0;
                }
            } else {
                if (col % 2 == 0) {
                    pixels[index + 1] = 0;
                } else {
                    pixels[index] = 0;
                    pixels[index + 1] = 0;
                    pixels[index + 2] = 0;
                }
            }
        }
    }
    // loadFromMemory segfaults, so don't use this.
    // var image = stbi.Image.loadFromMemory(&pixels, channels) catch |e| {
    //     std.log.err("Failed to load default texture.", .{});
    //     return e;
    // };
    var new_texture = texture.texture_create(gctx, .{ .texture_binding = true, .copy_dst = true }, .{
        .width = 256,
        .height = 256,
        .depth_or_array_layers = 1,
    }, .{ .components_count = 4, .components_width = 1, .is_hdr = false });
    texture.texture_load_data(gctx, &new_texture, dimensions, dimensions, dimensions * channels, pixels[0..]);
    return new_texture;
}

pub fn resources_load_mesh(arena: std.mem.Allocator, path: []const u8, out_meshes: *std.ArrayList(Mesh), out_vertices: *std.ArrayList(Vertex), out_indices: *std.ArrayList(u32)) !void {
    const data = try zmesh.io.parseAndLoadFile(path);
    defer zmesh.io.freeData(data);
    var indices = std.ArrayList(u32).init(arena);
    var positions = std.ArrayList([3]f32).init(arena);
    var uvs = std.ArrayList([2]f32).init(arena);
    try zmesh.io.appendMeshPrimitive(data, 0, 0, &indices, &positions, null, &uvs, null);
    try out_meshes.append(.{
        .index_offset = @intCast(u32, out_indices.items.len),
        .vertex_offset = @intCast(i32, out_vertices.items.len),
        .num_indices = @intCast(u32, indices.items.len),
        .num_vertices = @intCast(u32, positions.items.len),
    });
    for (indices.items) |index| {
        try out_indices.append(index);
    }
    for (positions.items, 0..) |_, index| {
        try out_vertices.append(.{
            .position = positions.items[index],
            .uv = uvs.items[index],
        });
    }
}
