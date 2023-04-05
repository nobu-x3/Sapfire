use glam::EulerRot;

#[derive(Clone, Copy, Debug)]
pub struct TransformComponent {
    pub translation: glam::Vec3,
    pub scale: glam::Vec3,
    rotation: glam::Quat,
    euler_rotation: glam::Vec3,
}

impl TransformComponent {
    pub fn new() -> TransformComponent {
        TransformComponent {
            translation: glam::Vec3::ZERO,
            scale: glam::Vec3::ONE,
            rotation: glam::Quat::IDENTITY,
            euler_rotation: glam::Vec3::ZERO,
        }
    }

    pub fn transform(&self) -> glam::Mat4 {
        glam::Mat4::from_scale_rotation_translation(self.scale, self.rotation, self.translation)
    }

    pub fn orientation(&self) -> glam::Vec3 {
        glam::vec3(-&self.rotation.x, -self.rotation.y, self.rotation.z)
    }

    pub fn euler(&self) -> glam::Vec3 {
        self.euler_rotation
    }

    pub fn set_euler(&mut self, euler: &glam::Vec3) {
        self.euler_rotation = euler.clone();
        self.rotation = glam::Quat::from_euler(EulerRot::XYZ, euler.x, euler.y, euler.z);
    }
}


pub struct RenderComponent{
    pub model: sapfire_renderer::wgpu_wrapper::model::Model
}

pub struct CameraComponent{
    pub camera: sapfire_renderer::camera::Camera
}
