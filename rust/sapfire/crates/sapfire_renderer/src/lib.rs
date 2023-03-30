extern crate gl;
extern crate glam;
extern crate wgpu;
use camera::Camera;
pub mod camera;
pub mod renderer;
pub mod wgpu_wrapper;
use renderer::RenderingContext;
use wgpu_wrapper::WGPURenderingContext;
use winit::dpi::PhysicalSize;
use winit::event::{ElementState, Event, KeyboardInput, VirtualKeyCode, WindowEvent};
use winit::event_loop::{ControlFlow, EventLoop};
use winit::window::WindowBuilder;

pub struct SapfireRenderer {
    rendering_context: RenderingContext,
    camera: Camera,
}

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

    pub fn run((renderer, event_loop): (SapfireRenderer, EventLoop<()>)) {
        if let RenderingContext::WGPU(mut context) = renderer.rendering_context {
            event_loop.run(move |event, _, control_flow| match event {
                Event::WindowEvent {
                    ref event,
                    window_id,
                } if window_id == context.window().id() => {
                    if !context.input(event) {
                        match event {
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
                        }
                    }
                }
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
