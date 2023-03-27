use self::buffer::{BufferLayout, IndexBuffer, VertexBuffer};
pub use crate::buffer;
use crate::Vertex;
pub use gl::*;
use std::ffi::c_void;

pub struct OpenGLVertexBuffer {
    pub id: u32,
    pub size: isize,
    pub layout: buffer::BufferLayout,
}

impl OpenGLVertexBuffer {
    pub fn new(layout: BufferLayout) -> OpenGLVertexBuffer {
        unsafe {
            // let id: *mut u32;
            let mut id: u32 = 0;
            CreateBuffers(1, &mut id as *mut u32);
            OpenGLVertexBuffer {
                id,
                layout,
                size: 0,
            }
        }
    }
}

impl VertexBuffer for OpenGLVertexBuffer {
    fn bind(&self) {
        unsafe {
            BindBuffer(ARRAY_BUFFER, self.id);
        }
    }

    fn unbind(&self) {
        unsafe {
            BindBuffer(ARRAY_BUFFER, 0);
        }
    }

    fn set_data(&mut self, buffer: &Vec<Vertex>, size: isize) {
        unsafe { NamedBufferStorage(self.id, size, buffer.as_ptr() as *const c_void, 0) }
        self.size = size;
    }
}

pub struct OpenGLIndexBuffer {
    id: u32,
    size: isize,
}

impl OpenGLIndexBuffer {
    pub fn new() -> OpenGLIndexBuffer {
        unsafe {
            let mut id: u32 = 0;
            CreateBuffers(1, &mut id as *mut u32);
            OpenGLIndexBuffer { id, size: 0 }
        }
    }
}

impl IndexBuffer for OpenGLIndexBuffer {
    fn bind(&self) {
        unsafe {
            BindBuffer(ELEMENT_ARRAY_BUFFER, self.id);
        }
    }

    fn unbind(&self) {
        unsafe {
            BindBuffer(ELEMENT_ARRAY_BUFFER, 0);
        }
    }

    fn set_data(&mut self, buffer: &Vec<Vertex>, size: isize) {
        unsafe { NamedBufferStorage(self.id, size, buffer.as_ptr() as *const c_void, 0) }
        self.size = size;
    }
}
