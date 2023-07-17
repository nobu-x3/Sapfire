const zm = @import("zmath");

pub const Transform = struct {
    matrix: zm.Mat = zm.mul(zm.matFromQuat(zm.qidentity()), zm.translation(0.0, 0.0, 0.0)),
};

pub const Mesh = struct {
    index_offset: u32,
    vertex_offset: i32,
    num_indices: u32,
    num_vertices: u32,
};
