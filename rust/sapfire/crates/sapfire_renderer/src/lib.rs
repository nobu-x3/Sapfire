extern crate gl;
extern crate glam;
extern crate wgpu;
use camera::Camera;
use opengl_wrapper::opengl_buffer::buffer::IndexBuffer;
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
use winit::dpi::PhysicalSize;
use winit::event::{ElementState, Event, KeyboardInput, VirtualKeyCode, WindowEvent};
use winit::event_loop::{ControlFlow, EventLoop};
use winit::window::{Window, WindowBuilder};

use crate::buffer::{BufferElement, BufferLayout, VertexBuffer};
use crate::opengl_wrapper::opengl_buffer::{OpenGLIndexBuffer, OpenGLVertexBuffer};
use crate::opengl_wrapper::opengl_vertex_array::OpenGLVertexArray;
use crate::opengl_wrapper::OpenGLShader;

pub struct SapfireRenderer {
    window: Window,
    device: wgpu::Device,
    surface: wgpu::Surface,
    queue: wgpu::Queue,
    config: wgpu::SurfaceConfiguration,
    size: winit::dpi::PhysicalSize<u32>,
    rendering_context: RenderingContext,
    camera: Camera,
}

type Vertex = [f32; 3];

const VERTICES: [Vertex; 3] = [[-0.5, -0.5, 0.0], [0.5, -0.5, 0.0], [0.0, 0.5, 0.0]];

pub enum RenderingAPI {
    OpenGL,
    WGPU,
}

impl SapfireRenderer {
    pub async fn new() -> SapfireRenderer {
        env_logger::init();
        let event_loop = EventLoop::new();
        let window = WindowBuilder::new().build(&event_loop).unwrap();
        window.set_resizable(true);
        window.set_title("Sapfire Engine");
        let size = PhysicalSize::new(800, 600);
        window.set_inner_size(size);
        let camera = Camera::new_ortho(800.0, 600.0, 1.0, -1.0);
        let instance = wgpu::Instance::new(wgpu::InstanceDescriptor {
            backends: wgpu::Backends::all(),
            dx12_shader_compiler: Default::default(), // not sure what this is
        });
        let surface = unsafe { instance.create_surface(&window) }.unwrap();
        let adapter = instance
            .request_adapter(&wgpu::RequestAdapterOptions {
                power_preference: wgpu::PowerPreference::HighPerformance,
                compatible_surface: Some(&surface),
                force_fallback_adapter: false,
            })
            .await
            .unwrap();

        let (device, queue) = adapter
            .request_device(
                &wgpu::DeviceDescriptor {
                    features: wgpu::Features::default(),
                    limits: wgpu::Limits::default(),
                    label: None,
                },
                None,
            )
            .await
            .unwrap();

        let surface_caps = surface.get_capabilities(&adapter);
        let surface_format = surface_caps
            .formats
            .iter()
            .copied()
            .filter(|f| f.describe().srgb)
            .next()
            .unwrap_or(surface_caps.formats[0]);
        let config = wgpu::SurfaceConfiguration {
            usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
            format: surface_format,
            width: size.width,
            height: size.height,
            present_mode: wgpu::PresentMode::Fifo,
            alpha_mode: surface_caps.alpha_modes[0],
            view_formats: vec![],
        };
        surface.configure(&device, &config);

        SapfireRenderer {
            window,
            camera,
            size,
            device,
            queue,
            config,
            surface,
            rendering_context: RenderingContext::WGPU,
        }
    }

    // pub fn create_context(
    //     api: RenderingAPI,
    //     canvas: &mut Canvas<Window>,
    //     video_subsystem: &VideoSubsystem,
    // ) -> OpenGLRenderContext {
    //     match api {
    //         RenderingAPI::OpenGL => {
    //             load_with(|s| video_subsystem.gl_get_proc_address(s) as *const _);
    //             canvas.window().gl_set_context_to_current().unwrap();
    //             video_subsystem
    //                 .gl_set_swap_interval(SwapInterval::VSync)
    //                 .unwrap();
    //             let layout = BufferLayout::new(vec![BufferElement::new(
    //                 String::from("pos"),
    //                 buffer::ShaderDataType::Vec3,
    //             )]);
    //             let mut vbo = OpenGLVertexBuffer::new(layout);
    //             vbo.set_data(&VERTICES.to_vec(), size_of_val(&VERTICES) as isize);
    //             let mut ibo = OpenGLIndexBuffer::new();
    //             let indices = vec![0, 1, 2];
    //             ibo.set_data(&indices, indices.len() as isize);
    //             let mut context: OpenGLRenderContext = OpenGLRenderContext {
    //                 shader: OpenGLShader { shader_program: 0 },
    //                 vao: OpenGLVertexArray::new(vbo, ibo),
    //             };
    //             context.add_shader("triangle.glsl");
    //             context
    //         }
    //     }
    // }

    pub fn run(&self) {
        loop {}
        // self.event_loop
        //     .run(move |event, _, control_flow| match event {
        //         Event::WindowEvent {
        //             ref event,
        //             window_id,
        //         } if window_id == self.window.id() => match event {
        //             WindowEvent::CloseRequested
        //             | WindowEvent::KeyboardInput {
        //                 input:
        //                     KeyboardInput {
        //                         state: ElementState::Pressed,
        //                         virtual_keycode: Some(VirtualKeyCode::Escape),
        //                         ..
        //                     },
        //                 ..
        //             } => *control_flow = ControlFlow::Exit,
        //             _ => {}
        //         },
        //         _ => {}
        //     });
    }
}
