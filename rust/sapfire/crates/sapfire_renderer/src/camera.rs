extern crate glam;
pub use glam::*;

pub struct Camera {
    projection: glam::Mat4,
}

impl Camera {
    pub fn new_perspective(
        rad_fov: f32,
        width: f32,
        height: f32,
        near_plane: f32,
        far_plane: f32,
    ) -> Camera {
        let projection = glam::Mat4::perspective_lh(rad_fov, width / height, near_plane, far_plane);
        Camera { projection }
    }

    pub fn new_ortho(width: f32, height: f32, near_plane: f32, far_plane: f32) -> Camera {
        let projection = glam::Mat4::orthographic_lh(
            -width * 0.5,
            width * 0.5,
            -height * 0.5,
            height * 0.5,
            near_plane,
            far_plane,
        );
        Camera { projection }
    }
}
