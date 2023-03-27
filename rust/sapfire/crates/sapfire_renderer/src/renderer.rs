use crate::{camera::Camera, opengl_wrapper::OpenGLRenderContext};

pub trait Renderer {
    // fn begin_scene(&mut self, camera: &Camera, cameraTransform: glam::Mat4);
    // fn end_scene();
    fn add_shader(&mut self, path: &str);
}

pub enum RenderingContext {
    OpenGL(OpenGLRenderContext),
    WGPU,
    None,
}
