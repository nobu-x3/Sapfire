const std = @import("std");
const zgpu = @import("zgpu");
const texture = @import("texture.zig");
const zmesh = @import("zmesh");
const types = @import("renderer_types.zig");

pub fn resources_load_mesh(arena: std.mem.Allocator, path: [:0]const u8, out_meshes: *std.ArrayList(types.Mesh), out_vertices: *std.ArrayList(types.Vertex), out_indices: *std.ArrayList(u32)) !void {
    const data = zmesh.io.parseAndLoadFile(path) catch |e| {
        std.log.err("Error type: {s}", .{@typeName(@TypeOf(e))});
        return e;
    };
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
