pub mod components;
pub mod systems;
use components::{CameraComponent, TransformComponent};
use legion::*;
use sapfire_renderer::*;

pub struct App {
    ecs: World,
    resources: Resources,
    systems: Schedule,
    // renderer: SapfireRenderer,
    // event_loop: winit::event_loop::EventLoop<()>
}
pub struct CameraSpeed {
    pub speed: f32,
}

impl App {
    pub async fn new() -> (App, SapfireRenderer, winit::event_loop::EventLoop<()>) {
        let mut ecs = World::default();
        let mut resources = Resources::default();
        let systems = Schedule::builder()
            .add_system(systems::input_system())
            .build();
        ecs.push((
            CameraComponent {
                camera: camera::Camera::new_perspective(
                    (45.0 as f32).to_radians(),
                    800.0,
                    600.0,
                    0.1,
                    100.0,
                ),
            },
            TransformComponent::new(),
        ));
        resources.insert(CameraSpeed { speed: 0.5 });
        resources.insert(None::<VirtualKeyCode>);
        let (renderer, event_loop) = SapfireRenderer::new().await;
        (
            App {
                ecs,
                resources,
                systems,
            },
            renderer,
            event_loop,
        )
    }

    pub fn run(mut app: App, renderer: SapfireRenderer, event_loop: EventLoop<()>) {
        if let renderer::RenderingContext::WGPU(mut context) = renderer.rendering_context {
            event_loop.run(move |event, _, control_flow| match event {
                sapfire_renderer::Event::WindowEvent {
                    ref event,
                    window_id,
                } if window_id == context.window().id() => {
                    // if !context.input(event) {
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
                        WindowEvent::KeyboardInput { input, .. } => {
                            if input.state == ElementState::Pressed {
                                app.resources.insert(input.virtual_keycode)
                            } else {
                                app.resources.insert(None::<VirtualKeyCode>)
                            }
                        }
                        _ => {}
                    }
                }
                Event::RedrawRequested(window_id) if window_id == context.window().id() => {
                    // context.update();
                    app.systems.execute(&mut app.ecs, &mut app.resources);
                    let mut query = <(&mut TransformComponent, &CameraComponent)>::query();
                    if let Some((camera_transform, camera)) = query.iter_mut(&mut app.ecs).next() {
                        // camera_transform.translation = glam::vec3(camera_transform.translation.x + deltaPos.x, camera_transform.translation.y + deltaPos.y, camera_transform.translation.z + deltaPos.z);
                        context.begin_scene(
                            camera_transform.transform(),
                            &camera_transform.translation,
                            &camera.camera.projection,
                        );

                        match context.render() {
                            Ok(_) => {}
                            Err(sapfire_renderer::wgpu_wrapper::SurfaceError::Lost) => {
                                context.resize(context.size)
                            }
                            Err(sapfire_renderer::wgpu_wrapper::SurfaceError::OutOfMemory) => {
                                *control_flow = ControlFlow::Exit
                            }
                            Err(e) => eprintln!("{:?}", e),
                        }
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
