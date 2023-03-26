mod opengl_shader;
use crate::renderer::Renderer;
pub use opengl_shader::*;
use sdl2::video::GLContext;

pub struct OpenGLRenderContext {
    pub shader: OpenGLShader,
    pub context: GLContext,
}

impl Renderer for OpenGLRenderContext {
    fn add_shader(&mut self, path: &str) {
        self.shader = OpenGLShader::new(path);
    }
}
