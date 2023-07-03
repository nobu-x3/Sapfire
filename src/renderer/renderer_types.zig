const zm = @import("zmath");
const std = @import("std");
pub const Vertex = extern struct {
    position: [3]f32,
    uv: [2]f32,
};

pub const Mesh = struct {
    index_offset: u32,
    vertex_offset: i32,
    num_indices: u32,
    num_vertices: u32,
    transform: Transform = Transform.init(),
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
        self.matrix = zm.mul(zm.matFromQuat(self.rotation), zm.translation(self.position[0], self.position[1], self.position[2]));
    }
};
