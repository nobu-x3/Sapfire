extern crate gl;
extern crate glam;
use camera::Camera;
use sdl2::render::Canvas;
use sdl2::{event::*, keyboard::Keycode, video::*, EventPump, Sdl, VideoSubsystem};
pub mod buffer;
pub mod camera;
pub mod opengl_wrapper;
pub mod vertex_array;
use opengl_wrapper::OpenGLRenderContext;
pub mod renderer;
use renderer::{Renderer, RenderingContext};
pub mod shader;
use gl::*;
use std::mem::{size_of, size_of_val};
use vertex_array::VertexArray;

use crate::buffer::{BufferElement, BufferLayout, VertexBuffer};
use crate::opengl_wrapper::opengl_buffer::{OpenGLIndexBuffer, OpenGLVertexBuffer};
use crate::opengl_wrapper::opengl_vertex_array::OpenGLVertexArray;
use crate::opengl_wrapper::OpenGLShader;

pub struct SapfireRenderer {
    canvas: Canvas<Window>,
    sdl_context: Sdl,
    video_subsystem: VideoSubsystem,
    event_pump: EventPump,
    rendering_context: RenderingContext,
    camera: Camera,
}

type Vertex = [f32; 3];

const VERTICES: [Vertex; 3] = [[-0.5, -0.5, 0.0], [0.5, -0.5, 0.0], [0.0, 0.5, 0.0]];

pub enum RenderingAPI {
    OpenGL,
}

impl SapfireRenderer {
    pub fn new(api: RenderingAPI) -> SapfireRenderer {
        let sdl_context = sdl2::init().expect("Failed to initialize SDL2");
        let mut video_subsystem = sdl_context
            .video()
            .expect("Failed to initialize video subsystem");
        let event_pump = sdl_context
            .event_pump()
            .expect("Failed to initialize event pump");
        match api {
            RenderingAPI::OpenGL => {
                match video_subsystem.gl_load_library_default() {
                    Err(x) => panic!("Failed to load default OpenGL library:\n{}", x),
                    _ => {}
                };
                let gl_attr = video_subsystem.gl_attr();
                gl_attr.set_context_profile(GLProfile::Core);
                gl_attr.set_context_version(3, 3);
                gl_attr.set_accelerated_visual(true);
                // #[cfg(target_os = "macos")]
                // {
                //     sdl.set_gl_profile(video::GlProfile::Compatibility);
                // }

                let window = video_subsystem
                    .window("Sapfire Engine", 800, 600)
                    .opengl()
                    .build()
                    .expect("Failed to initialize window!");
                let mut canvas = window
                    .into_canvas()
                    .index(SapfireRenderer::find_sdl_gl_driver().unwrap())
                    .build()
                    .expect("Failed to create canvas!");
                let mut context =
                    SapfireRenderer::create_context(api, &mut canvas, &mut video_subsystem);
                let camera = Camera::new_ortho(800.0, 600.0, 1.0, -1.0);
                SapfireRenderer {
                    canvas,
                    camera,
                    sdl_context,
                    event_pump,
                    video_subsystem,
                    rendering_context: RenderingContext::OpenGL(context),
                }
            }
        }
    }
    fn find_sdl_gl_driver() -> Option<u32> {
        for (index, item) in sdl2::render::drivers().enumerate() {
            if item.name == "opengl" {
                println!("{}", item.flags);
                return Some(index as u32);
            }
        }
        None
    }
    pub fn create_context(
        api: RenderingAPI,
        canvas: &mut Canvas<Window>,
        video_subsystem: &VideoSubsystem,
    ) -> OpenGLRenderContext {
        match api {
            RenderingAPI::OpenGL => {
                load_with(|s| video_subsystem.gl_get_proc_address(s) as *const _);
                canvas.window().gl_set_context_to_current().unwrap();
                video_subsystem
                    .gl_set_swap_interval(SwapInterval::VSync)
                    .unwrap();
                unsafe {
                    let layout = BufferLayout::new(vec![BufferElement {
                        name: String::from("pos"),
                        data_type: buffer::ShaderDataType::Float,
                        size: 3,
                        offset: 0,
                        normalized: false,
                    }]);
                    let mut vbo = OpenGLVertexBuffer::new(layout);
                    vbo.set_data(&VERTICES.to_vec(), size_of_val(&VERTICES) as isize);
                    let ibo = OpenGLIndexBuffer::new();
                    let mut context: OpenGLRenderContext = OpenGLRenderContext {
                        shader: OpenGLShader { shader_program: 0 },
                        vao: OpenGLVertexArray::new(vbo, ibo),
                    };
                    context.add_shader("triangle.glsl");
                    context
                }
            }
        }
    }

    pub fn run(&mut self) {
        let mut should_close = false;
        while !should_close {
            for event in self.event_pump.poll_iter() {
                match event {
                    Event::Quit { .. }
                    | Event::KeyDown {
                        keycode: Some(Keycode::Escape),
                        ..
                    } => {
                        println!("Exiting...");
                        should_close = true;
                    }
                    _ => {}
                }
            }
            unsafe {
                ClearColor(0.6, 0.0, 0.8, 1.0);
                Clear(gl::COLOR_BUFFER_BIT);
                match &self.rendering_context {
                    RenderingContext::OpenGL(x) => {
                        x.vao.bind();
                    }
                    _ => (),
                }
                DrawArrays(TRIANGLES, 0, 3);
            }
            self.canvas.present();
        }
    }
}
