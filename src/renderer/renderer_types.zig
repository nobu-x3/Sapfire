const zm = @import("zmath");
const log = @import("../core/logger.zig");
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

pub const Transform = struct {
    position: [3]f32 = .{ 0.0, 0.0, 0.0 },
    euler_angles: [3]f32 = .{ 0.0, 0.0, 0.0 },
    rotation: zm.Quat = zm.qidentity(),
    scale: [3]f32 = .{ 1.0, 1.0, 1.0 },
    matrix: zm.Mat,

    pub fn init() Transform {
        var srt = zm.mul(zm.matFromQuat(zm.qidentity()), zm.translation(0.0, 0.0, 0.0));
        return Transform{
            .matrix = srt,
        };
    }

    pub fn init_from_mat(matrix: []const f32) Transform {
        return Transform{
            .matrix = zm.loadMat(matrix),
        };
    }

    pub fn init_from_srt(position: [3]f32, euler_angles: [3]f32, scale: [3]f32) Transform {
        const scaling: zm.Mat = zm.scaling(scale[0], scale[1], scale[2]);
        const euler_to_quat: zm.Quat = zm.quatFromRollPitchYaw(euler_angles[0], euler_angles[1], euler_angles[2]);
        const matFromQuat: zm.Mat = zm.matFromQuat(euler_to_quat);
        const rotation: zm.Mat = zm.mul(scaling, matFromQuat);
        var srt: zm.Mat = zm.mul(rotation, zm.translationV(zm.loadArr3(position)));
        return Transform{
            .position = position,
            .euler_angles = euler_angles,
            .rotation = euler_to_quat,
            .scale = scale,
            .matrix = srt,
        };
    }

    pub fn update(self: *Transform) void {
        const scale = zm.scaling(self.scale[0], self.scale[1], self.scale[2]);
        const rot: zm.Mat = zm.mul(scale, zm.matFromQuat(self.rotation));
        self.matrix = zm.mul(rot, zm.translation(self.position[0], self.position[1], self.position[2]));
    }

    pub fn rotate(self: *Transform, angle: f32, axis: [3]f32) void {
        self.euler_angles = .{
            self.euler_angles[0] + angle * axis[0],
            self.euler_angles[1] + angle * axis[1],
            self.euler_angles[2] + angle * axis[2],
        };
        self.rotation = zm.quatFromRollPitchYaw(self.euler_angles[0], self.euler_angles[1], self.euler_angles[2]);
    }
};
