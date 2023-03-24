extern crate glfw;
use glfw::{Context, Key, WindowEvent};
extern crate gl;
extern crate glam;
pub mod opengl_wrapper;
use opengl_wrapper::OpenGLRenderer;
pub mod renderer;
use renderer::Renderer;
pub mod shader;
use gl::*;
use std::{
    mem::{size_of, size_of_val},
    sync::mpsc::Receiver,
};

use crate::opengl_wrapper::OpenGLShader;

pub struct Window {
    api: RenderingAPI,
    window: glfw::Window,
    window_handle: glfw::Glfw,
    events: Receiver<(f64, WindowEvent)>,
}

type Vertex = [f32; 3];

const VERTICES: [Vertex; 3] = [[-0.5, -0.5, 0.0], [0.5, -0.5, 0.0], [0.0, 0.5, 0.0]];

pub enum RenderingAPI {
    OpenGL,
}

impl Window {
    pub fn new(api: RenderingAPI) -> Window {
        let mut window_handle = glfw::init(glfw::FAIL_ON_ERRORS).unwrap();
        match api {
            RenderingAPI::OpenGL => {
                window_handle.window_hint(glfw::WindowHint::OpenGlProfile(
                    glfw::OpenGlProfileHint::Any,
                ));
                let (mut window, events) = window_handle
                    .create_window(800, 600, "Sapfire", glfw::WindowMode::Windowed)
                    .expect("Failed to open glfw window");
                Window {
                    window,
                    window_handle,
                    events,
                    api,
                }
            }
        }
    }

    pub fn create_context(&mut self) -> OpenGLRenderer {
        match self.api {
            RenderingAPI::OpenGL => {
                load_with(|s| self.window.get_proc_address(s));
                self.window_handle
                    .set_swap_interval(glfw::SwapInterval::Sync(1));
                self.window.set_key_polling(true);
                self.window.make_current();
                unsafe {
                    let mut vao = 0;
                    GenVertexArrays(1, &mut vao);
                    assert_ne!(vao, 0);
                    let mut vbo = 0;
                    GenBuffers(1, &mut vbo);
                    assert_ne!(vbo, 0);
                    BindBuffer(gl::ARRAY_BUFFER, vbo);
                    BufferData(
                        ARRAY_BUFFER,
                        size_of_val(&VERTICES) as isize,
                        VERTICES.as_ptr().cast(),
                        STATIC_DRAW,
                    );
                    VertexAttribPointer(
                        0,
                        3,
                        FLOAT,
                        FALSE,
                        size_of::<Vertex>().try_into().unwrap(),
                        0 as *const _,
                    );
                    EnableVertexAttribArray(0);
                    let vertex_shader = CreateShader(VERTEX_SHADER);
                    assert_ne!(vertex_shader, 0);
                    let mut test: OpenGLRenderer = OpenGLRenderer {
                        shader: OpenGLShader { shader_program: 0 },
                    };
                    test.add_shader("triangle.vert", "triangle.frag");
                    test
                }
            }
        }
    }

    pub fn run(&mut self) {
        while !self.window.should_close() {
            self.window_handle.poll_events();
            for (_, event) in glfw::flush_messages(&self.events) {
                Window::on_input(&mut self.window, event);
            }
            unsafe {
                Clear(gl::COLOR_BUFFER_BIT);
                DrawArrays(TRIANGLES, 0, 3);
                self.window.swap_buffers();
            }
        }
    }

    fn on_input(window: &mut glfw::Window, event: glfw::WindowEvent) {
        match event {
            glfw::WindowEvent::Key(Key::Escape, _, glfw::Action::Press, _) => {
                println!("Exiting...");
                window.set_should_close(true)
            }
            _ => {}
        }
    }
}
