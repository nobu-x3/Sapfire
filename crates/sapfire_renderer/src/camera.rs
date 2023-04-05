extern crate glam;
pub use glam::*;

pub struct Camera {
    pub projection: glam::Mat4,
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

#[repr(C)]
#[derive(Debug, Copy, Clone, bytemuck::Zeroable, bytemuck::Pod)]
pub struct CameraUniform {
    pub view_proj: [[f32; 4]; 4],
    pub position: [f32; 3],
    pub _padding: u32,
}

impl Default for CameraUniform{
    fn default() -> Self {
        Self { view_proj: glam::Mat4::IDENTITY.to_cols_array_2d(), position: glam::Vec3::ONE.to_array(), _padding: 0 }
    }
}
