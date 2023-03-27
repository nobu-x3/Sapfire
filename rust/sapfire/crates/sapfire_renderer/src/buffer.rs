use crate::Vertex;

pub enum ShaderDataType {
    None,
    Float,
    Int,
    Vec2,
    Vec3,
    Vec4,
    Vec2i,
    Vec3i,
    Vec4i,
    Mat3,
    Mat4,
    Bool,
}

pub fn shader_data_type_size(data_type: &ShaderDataType) -> u32 {
    match data_type {
        ShaderDataType::Float => 4,
        ShaderDataType::Int => 4,
        ShaderDataType::Vec2 => 8,
        ShaderDataType::Vec3 => 12,
        ShaderDataType::Vec4 => 16,
        ShaderDataType::Vec2i => 8,
        ShaderDataType::Vec3i => 12,
        ShaderDataType::Vec4i => 16,
        ShaderDataType::Mat3 => 36,
        ShaderDataType::Mat4 => 64,
        ShaderDataType::Bool => 1,
        _ => 0,
    }
}

pub struct BufferElement {
    name: String,
    data_type: ShaderDataType,
    size: u32,
    offset: u32,
    normalized: bool,
}

impl BufferElement {
    pub fn get_component_count(&self) -> u32 {
        match self.data_type {
            ShaderDataType::Bool => 1,
            _ => self.size / 4,
        }
    }
}

pub struct BufferLayout {
    elements: Vec<BufferElement>,
    stride: u32,
}

impl BufferLayout {
    pub fn new(mut layout: Vec<BufferElement>) -> BufferLayout {
        let mut stride: u32 = 0;
        let mut offset: u32 = 0;
        for mut element in &mut layout {
            element.offset = offset;
            offset += element.size;
            stride += element.size;
        }
        BufferLayout {
            elements: layout,
            stride,
        }
    }
}

pub trait VertexBuffer {
    fn bind(&self);
    fn unbind(&self);
    fn set_data(&mut self, buffer: &Vec<Vertex>, size: isize);
}

pub trait IndexBuffer {
    fn bind(&self);
    fn unbind(&self);
    fn set_data(&mut self, buffer: &Vec<Vertex>, size: isize); // TODO: fix Vertex, should be index
}
