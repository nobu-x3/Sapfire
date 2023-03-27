use crate::buffer::{IndexBuffer, ShaderDataType, VertexBuffer};
use crate::opengl_wrapper::opengl_buffer::{OpenGLIndexBuffer, OpenGLVertexBuffer};
use crate::vertex_array::VertexArray;
use gl::types::GLenum;
use gl::*;

pub struct OpenGLVertexArray {
    id: u32,
    vbo: OpenGLVertexBuffer,
    ibo: OpenGLIndexBuffer,
}

impl VertexArray for OpenGLVertexArray {
    fn bind(&self) {
        unsafe {
            BindVertexArray(self.id);
        }
    }

    fn unbind(&self) {
        unsafe {
            BindVertexArray(0);
        }
    }
}

impl OpenGLVertexArray {
    pub fn new(vbo: OpenGLVertexBuffer, ibo: OpenGLIndexBuffer) -> OpenGLVertexArray {
        unsafe {
            let mut id: u32 = 0;
            CreateVertexArrays(1, &mut id as *mut u32);
            BindVertexArray(id);
            println!("Here");
            vbo.bind();
            let mut index = 0;
            let layout = &vbo.layout;
            let elements = &layout.elements;
            for element in elements {
                VertexArrayVertexBuffer(id, index, vbo.id, element.offset, layout.stride);
                EnableVertexArrayAttrib(id, index);
                VertexArrayAttribFormat(
                    id,
                    index,
                    element.get_component_count() as i32,
                    OpenGLVertexArray::convert_shader_data_type_to_GLenum(&element.data_type),
                    if element.normalized { TRUE } else { FALSE },
                    layout.stride as u32,
                );
                VertexArrayAttribBinding(id, index, index);
                index += 1;
            }
            ibo.bind();
            OpenGLVertexArray { id, vbo, ibo }
        }
    }
    pub fn convert_shader_data_type_to_GLenum(data_type: &ShaderDataType) -> GLenum {
        match data_type {
            ShaderDataType::Float => FLOAT,
            ShaderDataType::Vec2 => FLOAT,
            ShaderDataType::Vec3 => FLOAT,
            ShaderDataType::Vec4 => FLOAT,
            ShaderDataType::Mat3 => FLOAT,
            ShaderDataType::Mat4 => FLOAT,
            ShaderDataType::Vec2i => INT,
            ShaderDataType::Vec3i => INT,
            ShaderDataType::Vec4i => INT,
            ShaderDataType::Bool => BOOL,
            _ => NONE,
        }
    }
}
