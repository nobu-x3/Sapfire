use super::renderer;
pub mod shader {
    use glam;
    pub trait Shader {
        fn new();
        fn bind(&self);
        fn set_matrix_uniform(&self, name: &str, matrix: &glam::Mat4);
        fn set_vector_uniform(&self, name: &str, vector: &glam::Vec3);
        fn get_program_id(&self) -> u32;
    }
}
