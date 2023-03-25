use crate::opengl_wrapper::OpenGLRenderContext;

pub trait Renderer {
    fn add_shader(&mut self, vertex_path: &str, frag_path: &str);
}

pub enum RenderingContext {
    OpenGL(OpenGLRenderContext),
}
