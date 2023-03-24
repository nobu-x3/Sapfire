mod opengl_shader;
pub use opengl_shader::*;

pub trait IRenderer {
    fn add_shader(&mut self, vertex_path: &str, frag_path: &str);
}

pub struct OpenGLRenderer {
    pub shader: OpenGLShader,
}

impl IRenderer for OpenGLRenderer {
    fn add_shader(&mut self, vertex_path: &str, frag_path: &str) {
        self.shader = OpenGLShader::new(vertex_path, frag_path);
    }
}
