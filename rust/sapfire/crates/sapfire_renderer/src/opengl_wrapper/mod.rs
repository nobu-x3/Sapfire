mod opengl_shader;
use crate::renderer::Renderer;
pub use opengl_shader::*;

pub struct OpenGLRenderContext {
    pub shader: OpenGLShader,
}

impl Renderer for OpenGLRenderContext {
    fn add_shader(&mut self, path: &str) {
        self.shader = OpenGLShader::new(path);
    }
}
