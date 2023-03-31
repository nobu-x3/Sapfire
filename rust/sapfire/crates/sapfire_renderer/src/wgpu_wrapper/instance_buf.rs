use wgpu::{BufferAddress, VertexAttribute, VertexBufferLayout};

pub struct Instance {
    pub position: glam::Vec3,
    pub rotation: glam::Quat,
}

impl Instance {
    pub fn to_raw(&self) -> InstanceRaw {
        InstanceRaw {
            model: (glam::Mat4::from_translation(self.position)
                * glam::Mat4::from_quat(self.rotation)),
        }
    }
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct InstanceRaw {
    model: glam::Mat4,
}

unsafe impl bytemuck::Zeroable for InstanceRaw {}
unsafe impl bytemuck::Pod for InstanceRaw {}

impl InstanceRaw {
    pub fn desc<'a>() -> VertexBufferLayout<'a> {
        VertexBufferLayout {
            array_stride: std::mem::size_of::<InstanceRaw>() as BufferAddress,
            step_mode: wgpu::VertexStepMode::Instance,
            attributes: &[
                VertexAttribute {
                    offset: 0,
                    shader_location: 5,
                    format: wgpu::VertexFormat::Float32x4,
                },
                VertexAttribute {
                    offset: std::mem::size_of::<glam::Vec4>() as BufferAddress,
                    shader_location: 6,
                    format: wgpu::VertexFormat::Float32x4,
                },
                VertexAttribute {
                    offset: (std::mem::size_of::<glam::Vec4>() * 2) as BufferAddress,
                    shader_location: 7,
                    format: wgpu::VertexFormat::Float32x4,
                },
                VertexAttribute {
                    offset: (std::mem::size_of::<glam::Vec4>() * 3) as BufferAddress,
                    shader_location: 8,
                    format: wgpu::VertexFormat::Float32x4,
                },
            ],
        }
    }
}
