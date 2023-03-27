extern crate gl;
extern crate glam;
extern crate wgpu;
use camera::Camera;
pub mod buffer;
pub mod camera;
pub mod opengl_wrapper;
pub mod renderer;
pub mod vertex_array;
pub mod wgpu_wrapper;
use renderer::{Renderer, RenderingContext};
pub mod shader;
use wgpu_wrapper::WGPURenderingContext;
use winit::dpi::PhysicalSize;
use winit::event::{ElementState, Event, KeyboardInput, VirtualKeyCode, WindowEvent};
use winit::event_loop::{ControlFlow, EventLoop};
use winit::window::{Window, WindowBuilder};

pub struct SapfireRenderer {
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
    pub async fn new() -> (SapfireRenderer, EventLoop<()>) {
        env_logger::init();
        let event_loop = EventLoop::new();
        let window = WindowBuilder::new().build(&event_loop).unwrap();
        window.set_resizable(true);
        window.set_title("Sapfire Engine");
        window.set_inner_size(PhysicalSize::new(800, 600));
        let context = WGPURenderingContext::new(window).await;
        let camera = Camera::new_ortho(800.0, 600.0, 1.0, -1.0);
        (
            SapfireRenderer {
                rendering_context: RenderingContext::WGPU(context),
                camera,
            },
            event_loop,
        )
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

    pub fn run((renderer, event_loop): (SapfireRenderer, EventLoop<()>)) {
        if let RenderingContext::WGPU(mut context) = renderer.rendering_context {
            event_loop.run(move |event, _, control_flow| match event {
                Event::WindowEvent {
                    ref event,
                    window_id,
                } if window_id == context.window().id() => match event {
                    WindowEvent::CloseRequested
                    | WindowEvent::KeyboardInput {
                        input:
                            KeyboardInput {
                                state: ElementState::Pressed,
                                virtual_keycode: Some(VirtualKeyCode::Escape),
                                ..
                            },
                        ..
                    } => *control_flow = ControlFlow::Exit,
                    WindowEvent::Resized(physical_size) => context.resize(*physical_size),
                    WindowEvent::ScaleFactorChanged { new_inner_size, .. } => {
                        context.resize(**new_inner_size);
                    }
                    _ => {}
                },
                Event::RedrawRequested(window_id) if window_id == context.window().id() => {
                    match context.render() {
                        Ok(_) => {}
                        Err(wgpu::SurfaceError::Lost) => context.resize(context.size),
                        Err(wgpu::SurfaceError::OutOfMemory) => *control_flow = ControlFlow::Exit,
                        Err(e) => eprintln!("{:?}", e),
                    }
                }
                Event::MainEventsCleared => {
                    context.window().request_redraw();
                }
                _ => {}
            });
        }
    }
}
