use glam;

pub trait Shader {
    fn bind(&self);
    fn set_matrix_uniform(&self, name: &str, matrix: &glam::Mat4);
    fn set_vector_uniform(&self, name: &str, vector: &glam::Vec3);
}
