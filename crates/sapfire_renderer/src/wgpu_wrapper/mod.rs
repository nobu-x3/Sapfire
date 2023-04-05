pub mod instance_buf;
pub mod light;
pub mod model;
pub mod texture;
use glam::vec3;
pub use model::Vertex;
pub use wgpu::{
    util::{BufferInitDescriptor, DeviceExt},
    SurfaceError::*,
    *,
};
pub use winit::{
    event::{ElementState, KeyboardInput, VirtualKeyCode, WindowEvent},
    window::Window,
};

use crate::{camera::{self, CameraUniform}, resources};

use self::model::{DrawLight, DrawModel};

pub struct WGPURenderingContext {
    window: Window,
    device: Device,
    surface: Surface,
    queue: Queue,
    config: SurfaceConfiguration,
    pub size: winit::dpi::PhysicalSize<u32>,
    render_pipeline: RenderPipeline,
    light_render_pipeline: RenderPipeline,
    depth_texture: texture::Texture,
    camera_buffer: Buffer,
    camera_bind_group: BindGroup,
    // camera_controller: camera_controller::CameraController,
    instances: Vec<instance_buf::Instance>,
    instance_buffer: Buffer,
    obj_model: model::Model,
    light_buffer: Buffer,
    light_bind_group: BindGroup,
    light_uniform: light::LightUniform,
}
const NUM_INSTANCES_PER_ROW: u32 = 10;

impl WGPURenderingContext {
    pub async fn new(window: Window) -> WGPURenderingContext {
        let instance = wgpu::Instance::new(InstanceDescriptor {
            backends: Backends::all(),
            dx12_shader_compiler: Default::default(), // not sure what this is
        });
        let surface = unsafe { instance.create_surface(&window) }.unwrap();
        let adapter = instance
            .request_adapter(&RequestAdapterOptions {
                power_preference: PowerPreference::HighPerformance,
                compatible_surface: Some(&surface),
                force_fallback_adapter: false,
            })
            .await
            .unwrap();
        let (device, queue) = adapter
            .request_device(
                &DeviceDescriptor {
                    features: Features::default(),
                    limits: Limits::default(),
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
        let size = window.inner_size();
        let config = SurfaceConfiguration {
            usage: TextureUsages::RENDER_ATTACHMENT,
            format: surface_format,
            width: size.width,
            height: size.height,
            present_mode: PresentMode::Fifo,
            alpha_mode: surface_caps.alpha_modes[0],
            view_formats: vec![],
        };
        surface.configure(&device, &config);
        let texture_bind_group_layout =
            device.create_bind_group_layout(&BindGroupLayoutDescriptor {
                label: Some("texture_bind_group_layout"),
                entries: &[
                    BindGroupLayoutEntry {
                        binding: 0,
                        visibility: ShaderStages::FRAGMENT,
                        ty: BindingType::Texture {
                            sample_type: TextureSampleType::Float { filterable: true },
                            view_dimension: TextureViewDimension::D2,
                            multisampled: false,
                        },
                        count: None,
                    },
                    BindGroupLayoutEntry {
                        binding: 1,
                        visibility: ShaderStages::FRAGMENT,
                        ty: BindingType::Sampler(SamplerBindingType::Filtering),
                        count: None,
                    },
                    BindGroupLayoutEntry {
                        binding: 2,
                        visibility: ShaderStages::FRAGMENT,
                        ty: BindingType::Texture {
                            sample_type: TextureSampleType::Float { filterable: true },
                            view_dimension: TextureViewDimension::D2,
                            multisampled: false,
                        },
                        count: None,
                    },
                    BindGroupLayoutEntry {
                        binding: 3,
                        visibility: ShaderStages::FRAGMENT,
                        ty: BindingType::Sampler(SamplerBindingType::Filtering),
                        count: None,
                    },
                ],
            });
        let depth_texture = texture::Texture::new_depth_texture(&device, &config, "depth_texture");
        // let camera = camera::Camera {
        //     eye: (0.0, 1.0, 2.0).into(),
        //     // have it look at the origin
        //     target: (0.0, 0.0, 0.0).into(),
        //     // which way is "up"
        //     up: glam::Vec3::Y,
        //     aspect: config.width as f32 / config.height as f32,
        //     fovy: 45.0,
        //     znear: 0.1,
        //     zfar: 100.0,
        // };
        
        let camera: camera::Camera = camera::Camera::new_perspective((45.0 as f32).to_radians(), config.width as f32, config.height as f32, 0.1, 100.0);
        let mut camera_uniform = camera::CameraUniform::default();
        // camera_uniform.update_view_proj(&camera);
        let camera_buffer = device.create_buffer_init(&BufferInitDescriptor {
            label: Some("camera_buffer"),
            contents: bytemuck::cast_slice(&[camera_uniform]),
            usage: BufferUsages::UNIFORM | BufferUsages::COPY_DST,
        });
        let camera_bind_group_layout =
            device.create_bind_group_layout(&BindGroupLayoutDescriptor {
                label: Some("camera_buffer_layout"),
                entries: &[BindGroupLayoutEntry {
                    binding: 0,
                    visibility: ShaderStages::VERTEX_FRAGMENT,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Uniform,
                        has_dynamic_offset: false,
                        min_binding_size: None,
                    },
                    count: None,
                }],
            });
        let camera_bind_group = device.create_bind_group(&BindGroupDescriptor {
            label: Some("camera_bind_group"),
            layout: &camera_bind_group_layout,
            entries: &[BindGroupEntry {
                binding: 0,
                resource: camera_buffer.as_entire_binding(),
            }],
        });
        let light_uniform = light::LightUniform {
            position: [2.0, 2.0, 2.0, 0.0],
            color: [1.0, 1.0, 1.0, 0.0],
        };
        let light_buffer = device.create_buffer_init(&BufferInitDescriptor {
            label: Some("Light UB"),
            contents: bytemuck::cast_slice(&[light_uniform]),
            usage: BufferUsages::COPY_DST | BufferUsages::UNIFORM,
        });

        let light_bind_group_layout = device.create_bind_group_layout(&BindGroupLayoutDescriptor {
            label: Some("light_bind_group_layout"),
            entries: &[BindGroupLayoutEntry {
                binding: 0,
                visibility: ShaderStages::VERTEX_FRAGMENT,
                ty: BindingType::Buffer {
                    ty: BufferBindingType::Uniform,
                    has_dynamic_offset: false,
                    min_binding_size: None,
                },
                count: None,
            }],
        });
        let light_bind_group = device.create_bind_group(&BindGroupDescriptor {
            label: Some("light_bind_group"),
            layout: &light_bind_group_layout,
            entries: &[BindGroupEntry {
                binding: 0,
                resource: light_buffer.as_entire_binding(),
            }],
        });
        let render_pipeline_layout = device.create_pipeline_layout(&PipelineLayoutDescriptor {
            label: Some("Pipeline layout"),
            bind_group_layouts: &[
                &texture_bind_group_layout,
                &camera_bind_group_layout,
                &light_bind_group_layout,
            ],
            push_constant_ranges: &[],
        });
        let render_pipeline = {
            let shader = ShaderModuleDescriptor {
                label: Some("Default Shader"),
                source: ShaderSource::Wgsl(
                    resources::load_string("shaders/lit.wgsl").unwrap().into(),
                ),
            };
            create_render_pipeline(
                &device,
                &render_pipeline_layout,
                shader,
                &[
                    model::ModelVertex::desc(),
                    instance_buf::InstanceRaw::desc(),
                ],
                config.format,
                Some(texture::Texture::DEPTH_FORMAT),
            )
        };

        let light_render_pipeline = {
            let layout = device.create_pipeline_layout(&PipelineLayoutDescriptor {
                label: Some("Light pipeline layout"),
                bind_group_layouts: &[&camera_bind_group_layout, &light_bind_group_layout],
                push_constant_ranges: &[],
            });
            let shader = ShaderModuleDescriptor {
                label: Some("Light shader"),
                source: ShaderSource::Wgsl(
                    resources::load_string("shaders/light.wgsl").unwrap().into(),
                ),
            };
            create_render_pipeline(
                &device,
                &layout,
                shader,
                &[model::ModelVertex::desc()],
                config.format,
                Some(texture::Texture::DEPTH_FORMAT),
            )
        };
        const SPACE_BETWEEN: f32 = 3.0;
        let instances = (0..NUM_INSTANCES_PER_ROW)
            .flat_map(|z| {
                (0..NUM_INSTANCES_PER_ROW).map(move |x| {
                    let x = SPACE_BETWEEN * (x as f32 - NUM_INSTANCES_PER_ROW as f32 / 2.0);
                    let z = SPACE_BETWEEN * (z as f32 - NUM_INSTANCES_PER_ROW as f32 / 2.0);
                    let position = glam::Vec3::new(x, 0.0, z);
                    let rotation = if position == glam::Vec3::ZERO {
                        glam::Quat::from_axis_angle(glam::Vec3::Z, (0.0 as f32).to_radians())
                    } else {
                        glam::Quat::from_axis_angle(glam::Vec3::Z, (45.0 as f32).to_radians())
                    };
                    instance_buf::Instance { position, rotation }
                })
            })
            .collect::<Vec<_>>();
        let instance_data = instances
            .iter()
            .map(instance_buf::Instance::to_raw)
            .collect::<Vec<_>>();
        let instance_buffer = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
            label: Some("Instance Buffer"),
            contents: bytemuck::cast_slice(&instance_data),
            usage: wgpu::BufferUsages::VERTEX,
        });
        let obj_model =
            model::load_model("cube.obj", &device, &queue, &texture_bind_group_layout).unwrap();

        WGPURenderingContext {
            window,
            size,
            device,
            queue,
            config,
            surface,
            camera_bind_group,
            camera_buffer,
            depth_texture,
            render_pipeline,
            light_render_pipeline,
            // camera_controller,
            instance_buffer,
            instances,
            obj_model,
            light_bind_group,
            light_buffer,
            light_uniform,
        }
    }

    pub fn window(&self) -> &Window {
        &self.window
    }

    pub fn resize(&mut self, new_size: winit::dpi::PhysicalSize<u32>) {
        if new_size.width > 0 && new_size.height > 0 {
            self.size = new_size;
            self.config.width = new_size.width;
            self.config.height = new_size.height;
            self.depth_texture =
                texture::Texture::new_depth_texture(&self.device, &self.config, "depth_texture");
            self.surface.configure(&self.device, &self.config);
        }
    }

    pub fn input(&mut self, event: &WindowEvent) -> bool {
        // self.camera_controller.process_events(event);
        match event {
            WindowEvent::KeyboardInput {
                input:
                    KeyboardInput {
                        virtual_keycode: Some(VirtualKeyCode::Space),
                        state: ElementState::Pressed,
                        ..
                    },
                ..
            } => {
                return true;
            }
            _ => {
                return false;
            }
        }
    }

    pub fn render(&mut self) -> Result<(), SurfaceError> {
        let output = self.surface.get_current_texture()?;
        let view = output
            .texture
            .create_view(&TextureViewDescriptor::default());
        let mut encoder = self
            .device
            .create_command_encoder(&CommandEncoderDescriptor {
                label: Some("Render Encoder"),
            });
        let mut render_pass = encoder.begin_render_pass(&RenderPassDescriptor {
            label: Some("Render Pass"),
            color_attachments: &[Some(RenderPassColorAttachment {
                view: &view,
                resolve_target: None,
                ops: Operations {
                    load: LoadOp::Clear(Color {
                        r: 0.3,
                        g: 0.3,
                        b: 0.3,
                        a: 1.0,
                    }),
                    store: true,
                },
            })],
            depth_stencil_attachment: Some(RenderPassDepthStencilAttachment {
                view: &self.depth_texture.view,
                depth_ops: Some(Operations {
                    load: LoadOp::Clear(1.0),
                    store: true,
                }),
                stencil_ops: None,
            }),
        });
        render_pass.set_vertex_buffer(1, self.instance_buffer.slice(..));
        render_pass.set_pipeline(&self.light_render_pipeline);
        render_pass.draw_light_model(
            &self.obj_model,
            &self.camera_bind_group,
            &self.light_bind_group,
        );
        render_pass.set_pipeline(&self.render_pipeline);
        render_pass.draw_model_instanced(
            &self.obj_model,
            0..self.instances.len() as u32,
            &self.camera_bind_group,
            &self.light_bind_group,
        );
        drop(render_pass);
        self.queue.submit(std::iter::once(encoder.finish()));
        output.present();
        Ok(())
    }

    pub fn begin_scene(&mut self, camera_transform: glam::Mat4, camera_position: &glam::Vec3, projection: &glam::Mat4){
        let camera_uniform = CameraUniform {
            view_proj: (*projection * camera_transform).to_cols_array_2d(),
            position: camera_position.to_array(),
            ..CameraUniform::default()
        };
        self.queue.write_buffer(&self.camera_buffer, 0, bytemuck::cast_slice(&[camera_uniform]));
    }

    pub fn update(&mut self) {
        // self.camera_controller.update_camera(&mut self.camera);
        // self.camera_uniform.update_view_proj(&self.camera);
        // self.queue.write_buffer(
        //     &self.camera_buffer,
        //     0,
        //     bytemuck::cast_slice(&[self.camera_uniform]),
        // );
        for inst in &mut self.instances {
            let amount = glam::Quat::from_rotation_y((15.0 as f32).to_radians());
            inst.rotation *= amount;
        }

        let old_light_pos: glam::Vec4 = self.light_uniform.position.into();
        let temp = glam::Vec4::from((
            glam::Quat::from_axis_angle(vec3(0.0, 1.0, 0.0), (1.0 as f32).to_radians())
                * glam::vec3(old_light_pos.x, old_light_pos.y, old_light_pos.z),
            0.0,
        ));
        self.light_uniform.position = temp.to_array();
        self.queue.write_buffer(
            &self.light_buffer,
            0,
            bytemuck::cast_slice(&[self.light_uniform]),
        );
        // let instance_data = self
        //     .instances
        //     .iter()
        //     .map(instance_buf::Instance::to_raw)
        //     .collect::<Vec<_>>();
        // self.queue.write_buffer(
        //     &self.instance_buffer,
        //     0,
        //     bytemuck::cast_slice(&instance_data),
        // );
    }
}

pub fn create_render_pipeline(
    device: &wgpu::Device,
    layout: &wgpu::PipelineLayout,
    shader: wgpu::ShaderModuleDescriptor,
    vertex_layout: &[wgpu::VertexBufferLayout],
    color_format: wgpu::TextureFormat,
    depth_format: Option<wgpu::TextureFormat>,
) -> wgpu::RenderPipeline {
    let shader = device.create_shader_module(shader);
    device.create_render_pipeline(&RenderPipelineDescriptor {
        label: Some("Render Pipeline"),
        layout: Some(layout),
        vertex: VertexState {
            module: &shader,
            entry_point: "vs_main",
            buffers: vertex_layout,
        },
        fragment: Some(FragmentState {
            module: &shader,
            entry_point: "fs_main",
            targets: &[Some(ColorTargetState {
                format: color_format,
                blend: Some(BlendState {
                    color: BlendComponent::REPLACE,
                    alpha: BlendComponent::REPLACE,
                }),
                write_mask: ColorWrites::ALL,
            })],
        }),
        primitive: PrimitiveState {
            topology: PrimitiveTopology::TriangleList,
            strip_index_format: None,
            front_face: FrontFace::Ccw,
            cull_mode: Some(Face::Back),
            unclipped_depth: false,
            polygon_mode: PolygonMode::Fill,
            conservative: false,
        },
        depth_stencil: depth_format.map(|format| DepthStencilState {
            format,
            depth_write_enabled: true,
            depth_compare: CompareFunction::Less,
            stencil: StencilState::default(),
            bias: DepthBiasState::default(),
        }),
        multisample: MultisampleState {
            count: 1,
            mask: !0,
            alpha_to_coverage_enabled: false,
        },

        multiview: None,
    })
}
