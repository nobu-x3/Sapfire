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

pub fn shader_data_type_size(data_type: &ShaderDataType) -> isize {
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
    pub name: String,
    pub data_type: ShaderDataType,
    pub size: isize,
    pub offset: isize,
    pub normalized: bool,
}

impl BufferElement {
    pub fn new(name: String, data_type: ShaderDataType) -> BufferElement {
        BufferElement {
            name,
            size: shader_data_type_size(&data_type),
            data_type,
            offset: 0,
            normalized: false,
        }
    }
    pub fn get_component_count(&self) -> isize {
        match self.data_type {
            ShaderDataType::Bool => 1,
            _ => self.size / 4,
        }
    }
}

pub struct BufferLayout {
    pub elements: Vec<BufferElement>,
    pub stride: i32,
}

impl BufferLayout {
    pub fn new(mut layout: Vec<BufferElement>) -> BufferLayout {
        let mut stride: i32 = 0;
        let mut offset: isize = 0;
        for mut element in &mut layout {
            element.offset = offset;
            offset += element.size;
            stride += element.size as i32;
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
    fn set_data(&mut self, buffer: &Vec<u32>, size: isize); // TODO: fix Vertex, should be index
}
