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

// pub struct Camera {
//     pub eye: glam::Vec3,
//     pub target: glam::Vec3,
//     pub up: glam::Vec3,
//     pub aspect: f32,
//     pub fovy: f32,
//     pub znear: f32,
//     pub zfar: f32,
// }

// impl Camera {
//     fn build_view_projection_matrix(&self) -> (glam::Mat4, glam::Mat4) {
//         let view = glam::Mat4::look_at_rh(self.eye, self.target, self.up);
//         let proj = glam::Mat4::perspective_rh(
//             f32::to_radians(self.fovy),
//             self.aspect,
//             self.znear,
//             self.zfar,
//         );
//         return (view, proj * view);
//     }
// }

#[repr(C)]
#[derive(Debug, Copy, Clone, bytemuck::Zeroable, bytemuck::Pod)]
pub struct CameraUniform {
    pub view_proj: [[f32; 4]; 4],
    pub position: [f32; 3],
    pub _padding: u32,
}

impl CameraUniform {
    // pub fn new() -> CameraUniform {
    //     CameraUniform {
    //         view_proj: glam::Mat4::IDENTITY.to_cols_array_2d(),
    //         position: glam::Vec3::ONE.to_array(),
    //         _padding: 0,
    //     }
    // }

    // pub fn update_view_proj(&mut self, camera: &Camera) {
    //     let (view, view_proj) = camera.build_view_projection_matrix();
    //     (self.view, self.view_proj) = (view.to_cols_array_2d(), view_proj.to_cols_array_2d());
    //     self.position = camera.eye.to_array();
    // }
}

impl Default for CameraUniform{
    fn default() -> Self {
        Self { view_proj: glam::Mat4::IDENTITY.to_cols_array_2d(), position: glam::Vec3::ONE.to_array(), _padding: 0 }
    }
}
