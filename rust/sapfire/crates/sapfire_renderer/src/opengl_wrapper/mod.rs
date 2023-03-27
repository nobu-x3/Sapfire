pub mod opengl_buffer;
mod opengl_shader;
pub mod opengl_vertex_array;

use std::ptr::null;

use crate::{renderer::Renderer, vertex_array::VertexArray};
use gl::*;
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

impl OpenGLRenderContext {
    pub fn draw(&self) {
        unsafe {
            self.vao.bind();
            EnableVertexAttribArray(0);
            VertexAttribPointer(0, 3, FLOAT, FALSE, 12, 0 as *const std::os::raw::c_void);
            DrawElements(TRIANGLES, 3, UNSIGNED_INT, null());
        }
    }
}
