pub mod opengl_buffer;
mod opengl_shader;
pub mod opengl_vertex_array;
use crate::renderer::Renderer;
pub use opengl_shader::*;

use self::opengl_vertex_array::OpenGLVertexArray;

pub struct OpenGLRenderContext {
    pub shader: OpenGLShader,
    pub vao: OpenGLVertexArray,
}

impl Renderer for OpenGLRenderContext {
    fn add_shader(&mut self, path: &str) {
        self.shader = OpenGLShader::new(path);
    }
}
