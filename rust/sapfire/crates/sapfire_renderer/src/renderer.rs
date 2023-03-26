use crate::opengl_wrapper::OpenGLRenderContext;

pub trait Renderer {
    fn add_shader(&mut self, path: &str);
}

pub enum RenderingContext {
    OpenGL(OpenGLRenderContext),
}
