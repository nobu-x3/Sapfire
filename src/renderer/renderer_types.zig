const zm = @import("zmath");
pub const Vertex = extern struct {
    position: [3]f32,
    uv: [2]f32,
};

pub const Mesh = struct {
    index_offset: u32,
    vertex_offset: i32,
    num_indices: u32,
    num_vertices: u32,
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
