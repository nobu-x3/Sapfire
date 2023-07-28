const zm = @import("zmath");
const std = @import("std");
const zgpu = @import("zgpu");

pub const ComponentTypes = enum {
    transform,
    scale,
    mesh,
    material,
};

pub const name_type_map = std.ComptimeStringMap(ComponentTypes, .{
    .{ "scene.components.Transform", .transform },
    .{ "scene.components.Scale", .scale },
    .{ "scene.components.Mesh", .mesh },
    .{ "renderer.material.Material", .material },
});

pub const Transform = extern struct {
    local: zm.Mat = zm.mul(zm.matFromQuat(zm.qidentity()), zm.translation(0.0, 0.0, 0.0)),
    world: zm.Mat = zm.mul(zm.matFromQuat(zm.qidentity()), zm.translation(0.0, 0.0, 0.0)),
};

pub const Scale = extern struct {
    scale: @Vector(3, f32) = .{ 1.0, 1.0, 1.0 },
};

pub const Mesh = struct {
    guid: [64]u8,
    index_offset: u32,
    vertex_offset: i32,
    num_indices: u32,
    num_vertices: u32,
};
