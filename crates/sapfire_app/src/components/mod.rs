use glam::EulerRot;

#[derive(Clone, Copy, Debug)]
struct Transform {
    pub translation: glam::Vec3,
    pub scale: glam::Vec3,
    rotation: glam::Quat,
    euler_rotation: glam::Vec3,
}

impl Transform {
    pub fn new() -> Transform {
        Transform {
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
