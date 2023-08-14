const zm = @import("zmath");
const core = @import("../core.zig");
const log = core.log;
const renderer = @import("renderer.zig");
const std = @import("std");
const mat = @import("material.zig");

pub const Vertex = extern struct {
    position: [3]f32,
    uv: [2]f32,
};

pub const Uniforms = extern struct {
    aspect_ratio: f32,
    mip_level: f32,
    model: zm.Mat,
};

pub const GlobalUniforms = extern struct {
    view_projection: zm.Mat,
};

pub const Camera = struct {
    position: [3]f32 = .{ 0.0, 0.0, -3.0 },
    forward: [3]f32 = .{ 0.0, 0.0, 0.0 },
    pitch: f32 = 0.0,
    // yaw: f32 = std.math.pi + 0.25 * std.math.pi,
    yaw: f32 = 0.0,
};

pub const SRT = struct {
    scale: [3]f32 = .{ 1.0, 1.0, 1.0 },
    euler_angles: [3]f32 = .{ 0.0, 0.0, 0.0 },
    position: [3]f32 = .{ 0.0, 0.0, 0.0 },
};
